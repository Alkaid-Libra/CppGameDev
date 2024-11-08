#include <AL/al.h>
#include <AL/alc.h>
#include <mpg123.h>
#include <sndfile.h>
#include <iostream>
#include <vector>
#include <string>

class AudioPlayer {
public:
    AudioPlayer() {
        device = alcOpenDevice(nullptr); // 默认设备
        if (!device) {
            std::cerr << "Failed to open OpenAL device\n";
            return;
        }
        context = alcCreateContext(device, nullptr);
        if (!alcMakeContextCurrent(context)) {
            std::cerr << "Failed to make OpenAL context current\n";
            return;
        }        
        mpg123_init();
        mpgHandle = mpg123_new(nullptr, &err);
    }

    ~AudioPlayer() {
        // 释放 OpenAL 和 mpg123 资源
        alcMakeContextCurrent(nullptr);
        alcDestroyContext(context);
        alcCloseDevice(device);

        for (auto buffer : buffers) alDeleteBuffers(1, &buffer);
        for (auto source : sources) alDeleteSources(1, &source);
        
        mpg123_delete(mpgHandle);
        mpg123_exit();
    }

    bool loadSound(const std::string& filename) {
        // 检测文件扩展名
        std::string ext = filename.substr(filename.find_last_of(".") + 1);

        ALuint buffer = 0;
        if (ext == "mp3") {
            buffer = loadMP3(filename);
        } else if (ext == "wav") {
            buffer = loadWAV(filename);
        } else {
            std::cerr << "Unsupported audio format: " << ext << "\n";
            return false;
        }

        if (buffer == 0) {
            return false;
        }

        ALuint source;
        alGenSources(1, &source);
        alSourcei(source, AL_BUFFER, buffer);
        
        buffers.push_back(buffer);
        sources.push_back(source);

        return true;
    }

    void play(int index, bool is_loop) {
        if (index >= 0 && index < sources.size()) {
            // enable loop play
            if (is_loop)
                alSourcei(sources[index], AL_LOOPING, AL_TRUE);
            else
                alSourcei(sources[index], AL_LOOPING, AL_FALSE);

            alSourcePlay(sources[index]);
        }
    }

private:
    ALCdevice* device;
    ALCcontext* context;

    mpg123_handle* mpgHandle;
    int err;

    std::vector<ALuint> buffers;
    std::vector<ALuint> sources;

    ALuint loadWAV(const std::string& filename) {
        SF_INFO sfInfo;
        SNDFILE* file = sf_open(filename.c_str(), SFM_READ, &sfInfo);
        if (!file) {
            std::cerr << "Failed to open WAV file: " << filename << "\n";
            return 0;
        }

        ALenum format = (sfInfo.channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
        std::vector<short> samples(sfInfo.frames * sfInfo.channels);
        sf_read_short(file, samples.data(), samples.size());
        sf_close(file);

        ALuint buffer;
        alGenBuffers(1, &buffer);
        alBufferData(buffer, format, samples.data(), samples.size() * sizeof(short), sfInfo.samplerate);
        
        return buffer;
    }

    ALuint loadMP3(const std::string& filename) {
        if (mpg123_open(mpgHandle, filename.c_str()) != MPG123_OK) {
            std::cerr << "Failed to open MP3 file: " << filename << "\n";
            return 0;
        }

        long rate;
        int channels, encoding;
        mpg123_getformat(mpgHandle, &rate, &channels, &encoding);

        ALenum format = (channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;

        std::vector<unsigned char> audioData;
        unsigned char buffer[4096];
        size_t done;

        while (mpg123_read(mpgHandle, buffer, sizeof(buffer), &done) == MPG123_OK) {
            audioData.insert(audioData.end(), buffer, buffer + done);
        }
        mpg123_close(mpgHandle);

        ALuint alBuffer;
        alGenBuffers(1, &alBuffer);
        alBufferData(alBuffer, format, audioData.data(), audioData.size(), rate);

        return alBuffer;
    }
};
