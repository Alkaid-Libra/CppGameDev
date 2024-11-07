#include <glm/glm.hpp>


class Button
{
protected:
    virtual void onClick() = 0;

private:
    enum class Status
    {
        Idle = 0,
        Hovered,
        Pushed
    };
private:
    glm::vec3 region;
    char* img_idle, *img_hovered, *img_pushed;
    Status status = Status::Idle;

private:
    // check mouse click
    bool checkCursorHit(float x, float y)
    {
        return x >= region.x && x <= region.y && y >= region.y && y <= region.z;
    }


public:
    char* img_type;
public:
    Button() = default;
    Button(glm::vec3 location, const char* img_idle, const char* img_hovered, const char* img_pushed)
    {
        region = location;
        bool loadImage = true;
    }
    ~Button()
    {

    }

    void processEvent()
    {
        bool is_move_down_up;
    }

    void draw()
    {
        switch (status)
        {
        case Status::Idle:
            img_type = img_idle;
            break;
        case Status::Hovered:
            img_type = img_hovered;
            break;
        case Status::Pushed:
            img_type = img_pushed;
            break;
        
        default:
            break;
        }
    }
};

class StartGameButton : public Button
{
public:
    StartGameButton(){}
    ~StartGameButton(){}
protected:
    void onClick()
    {
        bool is_game_started = true;
    }
};

class QuitGameButton : public Button
{
private:
    /* data */
public:
    QuitGameButton(){}
    ~QuitGameButton(){}
protected:
    void onClick()
    {
        bool running = false;
    }
};