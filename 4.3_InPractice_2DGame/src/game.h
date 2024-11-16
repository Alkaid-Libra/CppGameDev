#ifndef GAME_H
#define GAME_H
#include <vector>
#include <tuple>
#include <algorithm>
#include <sstream>
#include <iostream>

#include "filesystem.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include <irrKlang.h>
using namespace irrklang;

#include "game_level.h"
#include "power_up.h"

#include "ball_object.h"
#include "particle_generator.h"
#include "post_processor.h"
#include "text_renderer.h"
#include "resource_manager.h"

// Game-related State data
SpriteRenderer    *Renderer;
GameObject        *Player;
BallObject        *Ball;
ParticleGenerator *Particles;
PostProcessor     *Effects;
ISoundEngine      *SoundEngine = createIrrKlangDevice();
TextRenderer      *Text;

float ShakeTime = 0.0f;

bool IsOtherPowerUpActive(std::vector<PowerUp> &powerUps, std::string type)
{
    // Check if another PowerUp of the same type is still active
    // in which case we don't disable its effect (yet)
    for (const PowerUp &powerUp : powerUps)
    {
        if (powerUp.Activated)
            if (powerUp.Type == type)
                return true;
    }
    return false;
}
bool ShouldSpawn(unsigned int chance)
{
    unsigned int random = rand() % chance;
    return random == 0;
}
void ActivatePowerUp(PowerUp &powerUp)
{
    if (powerUp.Type == "speed")
    {
        Ball->Velocity *= 1.2;
    }
    else if (powerUp.Type == "sticky")
    {
        Ball->Sticky = true;
        Player->Color = glm::vec3(1.0f, 0.5f, 1.0f);
    }
    else if (powerUp.Type == "pass-through")
    {
        Ball->PassThrough = true;
        Ball->Color = glm::vec3(1.0f, 0.5f, 0.5f);
    }
    else if (powerUp.Type == "pad-size-increase")
    {
        Player->Size.x += 50;
    }
    else if (powerUp.Type == "confuse")
    {
        if (!Effects->Chaos)
            Effects->Confuse = true; // only activate if chaos wasn't already active
    }
    else if (powerUp.Type == "chaos")
    {
        if (!Effects->Confuse)
            Effects->Chaos = true;
    }
}

// Represents the current state of the game
enum GameState {
    GAME_ACTIVE,
    GAME_MENU,
    GAME_WIN
};

// Represents the four possible (collision) directions
enum Direction {
    UP,
    RIGHT,
    DOWN,
    LEFT
};
// Defines a Collision typedef that represents collision data
typedef std::tuple<bool, Direction, glm::vec2> Collision; // <collision?, what direction?, difference vector center - closest point>

// Initial size of the player paddle
const glm::vec2 PLAYER_SIZE(100.0f, 20.0f);
// Initial velocity of the player paddle
const float PLAYER_VELOCITY(500.0f);
// Initial velocity of the Ball
const glm::vec2 INITIAL_BALL_VELOCITY(100.0f, -350.0f);
// Radius of the ball object
const float BALL_RADIUS = 12.5f;

// Game holds all game-related state and functionality.
// Combines all game-related data into a single class for
// easy access to each of the components and manageability.
class Game
{
private:
    // collision detection
    bool CheckCollision(GameObject &one, GameObject &two) // AABB - AABB collision
    {
        // collision x-axis?
        bool collisionX = one.Position.x + one.Size.x >= two.Position.x &&
            two.Position.x + two.Size.x >= one.Position.x;
        // collision y-axis?
        bool collisionY = one.Position.y + one.Size.y >= two.Position.y &&
            two.Position.y + two.Size.y >= one.Position.y;
        // collision only if on both axes
        return collisionX && collisionY;
    }
    Collision CheckCollision(BallObject &one, GameObject &two) // AABB - Circle collision
    {
        // get center point circle first 
        glm::vec2 center(one.Position + one.Radius);
        // calculate AABB info (center, half-extents)
        glm::vec2 aabb_half_extents(two.Size.x / 2.0f, two.Size.y / 2.0f);
        glm::vec2 aabb_center(two.Position.x + aabb_half_extents.x, two.Position.y + aabb_half_extents.y);
        // get difference vector between both centers
        glm::vec2 difference = center - aabb_center;
        glm::vec2 clamped = glm::clamp(difference, -aabb_half_extents, aabb_half_extents);
        // now that we know the clamped values, add this to AABB_center and we get the value of box closest to circle
        glm::vec2 closest = aabb_center + clamped;
        // now retrieve vector between center circle and closest point AABB and check if length < radius
        difference = closest - center;
        
        if (glm::length(difference) < one.Radius) // not <= since in that case a collision also occurs when object one exactly touches object two, which they are at the end of each collision resolution stage.
            return std::make_tuple(true, VectorDirection(difference), difference);
        else
            return std::make_tuple(false, UP, glm::vec2(0.0f, 0.0f));
    }
    Direction VectorDirection(glm::vec2 target) // calculates which direction a vector is facing (N,E,S or W)
    {
        glm::vec2 compass[] = {
            glm::vec2(0.0f, 1.0f),	// up
            glm::vec2(1.0f, 0.0f),	// right
            glm::vec2(0.0f, -1.0f),	// down
            glm::vec2(-1.0f, 0.0f)	// left
        };
        float max = 0.0f;
        unsigned int best_match = -1;
        for (unsigned int i = 0; i < 4; i++)
        {
            float dot_product = glm::dot(glm::normalize(target), compass[i]);
            if (dot_product > max)
            {
                max = dot_product;
                best_match = i;
            }
        }
        return (Direction)best_match;
    }

public:
    // game state
    GameState               State;	
    bool                    Keys[1024];
    bool                    KeysProcessed[1024];
    unsigned int            Width, Height;    
    std::vector<GameLevel>  Levels;
    std::vector<PowerUp>    PowerUps;
    unsigned int            Level;
    unsigned int            Lives;
    Game(unsigned int width, unsigned int height)
        : State(GAME_ACTIVE), Keys(), KeysProcessed(), Width(width), Height(height), Level(0), Lives(3)
        // : State(GAME_MENU), Keys(), KeysProcessed(), Width(width), Height(height), Level(0), Lives(3)
    {

    }
    ~Game()
    {
        delete Renderer;
        delete Player;
        delete Ball;
        delete Particles;
        delete Effects;
        delete Text;
        SoundEngine->drop();
    }
    // initialize game state (load all shaders/textures/levels)
    void Init()
    {
        // load shaders
        ResourceManager::loadShader(FileSystem::getPath("shaders/sprite.vs").c_str(), FileSystem::getPath("shaders/sprite.fs").c_str(), nullptr, "sprite");
        ResourceManager::loadShader(FileSystem::getPath("shaders/particle.vs").c_str(), FileSystem::getPath("shaders/particle.fs").c_str(), nullptr, "particle");
        ResourceManager::loadShader(FileSystem::getPath("shaders/post_processing.vs").c_str(), FileSystem::getPath("shaders/post_processing.fs").c_str(), nullptr, "postprocessing");
        // configure shaders
        glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(this->Width), static_cast<float>(this->Height), 0.0f, -1.0f, 1.0f);
        ResourceManager::getShader("sprite").use().setInt("sprite", 0);
        ResourceManager::getShader("sprite").setMat4("projection", projection);
        ResourceManager::getShader("particle").use().setInt("sprite", 0);
        ResourceManager::getShader("particle").setMat4("projection", projection);
        // load textures
        ResourceManager::loadTexture(FileSystem::getPath("resources/textures/background.jpg").c_str(), false, "background");
        ResourceManager::loadTexture(FileSystem::getPath("resources/textures/awesomeface.png").c_str(), true, "face");
        ResourceManager::loadTexture(FileSystem::getPath("resources/textures/block.png").c_str(), false, "block");
        ResourceManager::loadTexture(FileSystem::getPath("resources/textures/block_solid.png").c_str(), false, "block_solid");
        ResourceManager::loadTexture(FileSystem::getPath("resources/textures/paddle.png").c_str(), true, "paddle");
        ResourceManager::loadTexture(FileSystem::getPath("resources/textures/particle.png").c_str(), true, "particle");
        ResourceManager::loadTexture(FileSystem::getPath("resources/textures/powerup_speed.png").c_str(), true, "powerup_speed");
        ResourceManager::loadTexture(FileSystem::getPath("resources/textures/powerup_sticky.png").c_str(), true, "powerup_sticky");
        ResourceManager::loadTexture(FileSystem::getPath("resources/textures/powerup_increase.png").c_str(), true, "powerup_increase");
        ResourceManager::loadTexture(FileSystem::getPath("resources/textures/powerup_confuse.png").c_str(), true, "powerup_confuse");
        ResourceManager::loadTexture(FileSystem::getPath("resources/textures/powerup_chaos.png").c_str(), true, "powerup_chaos");
        ResourceManager::loadTexture(FileSystem::getPath("resources/textures/powerup_passthrough.png").c_str(), true, "powerup_passthrough");
        // set render-specific controls
        Renderer = new SpriteRenderer(ResourceManager::getShader("sprite"));
        Particles = new ParticleGenerator(ResourceManager::getShader("particle"), ResourceManager::getTexture("particle"), 200);
        Effects = new PostProcessor(ResourceManager::getShader("postprocessing"), this->Width, this->Height);
        // Text = new TextRenderer(this->Width, this->Height);
        // Text->Load(FileSystem::getPath("resources/fonts/OCRAEXT.TTF").c_str(), 24);
        // load levels
        GameLevel one; one.Load(FileSystem::getPath("resources/levels/one.lvl").c_str(), this->Width, this->Height / 2);
        GameLevel two; two.Load(FileSystem::getPath("resources/levels/two.lvl").c_str(), this->Width, this->Height /2 );
        GameLevel three; three.Load(FileSystem::getPath("resources/levels/three.lvl").c_str(), this->Width, this->Height / 2);
        GameLevel four; four.Load(FileSystem::getPath("resources/levels/four.lvl").c_str(), this->Width, this->Height / 2);
        this->Levels.push_back(one);
        this->Levels.push_back(two);
        this->Levels.push_back(three);
        this->Levels.push_back(four);
        this->Level = 0;
        // configure game objects
        glm::vec2 playerPos = glm::vec2(this->Width / 2.0f - PLAYER_SIZE.x / 2.0f, this->Height - PLAYER_SIZE.y);
        Player = new GameObject(playerPos, PLAYER_SIZE, ResourceManager::getTexture("paddle"));
        glm::vec2 ballPos = playerPos + glm::vec2(PLAYER_SIZE.x / 2.0f - BALL_RADIUS, -BALL_RADIUS * 2.0f);
        Ball = new BallObject(ballPos, BALL_RADIUS, INITIAL_BALL_VELOCITY, ResourceManager::getTexture("face"));
        // // audio
        // SoundEngine->play2D(FileSystem::getPath("resources/audio/breakout.mp3").c_str(), true);
    }
    // game loop
    void ProcessInput(float dt)
    {
        if (this->State == GAME_MENU)
        {
            if (this->Keys[GLFW_KEY_ENTER] && !this->KeysProcessed[GLFW_KEY_ENTER])
            {
                this->State = GAME_ACTIVE;
                this->KeysProcessed[GLFW_KEY_ENTER] = true;
            }
            if (this->Keys[GLFW_KEY_W] && !this->KeysProcessed[GLFW_KEY_W])
            {
                this->Level = (this->Level + 1) % 4;
                this->KeysProcessed[GLFW_KEY_W] = true;
            }
            if (this->Keys[GLFW_KEY_S] && !this->KeysProcessed[GLFW_KEY_S])
            {
                if (this->Level > 0)
                    --this->Level;
                else
                    this->Level = 3;
                //this->Level = (this->Level - 1) % 4;
                this->KeysProcessed[GLFW_KEY_S] = true;
            }
        }
        if (this->State == GAME_WIN)
        {
            if (this->Keys[GLFW_KEY_ENTER])
            {
                this->KeysProcessed[GLFW_KEY_ENTER] = true;
                Effects->Chaos = false;
                this->State = GAME_MENU;
            }
        }
        if (this->State == GAME_ACTIVE)
        {
            float velocity = PLAYER_VELOCITY * dt;
            // move playerboard
            if (this->Keys[GLFW_KEY_A])
            {
                if (Player->Position.x >= 0.0f)
                {
                    Player->Position.x -= velocity;
                    if (Ball->Stuck)
                        Ball->Position.x -= velocity;
                }
            }
            if (this->Keys[GLFW_KEY_D])
            {
                if (Player->Position.x <= this->Width - Player->Size.x)
                {
                    Player->Position.x += velocity;
                    if (Ball->Stuck)
                        Ball->Position.x += velocity;
                }
            }
            if (this->Keys[GLFW_KEY_SPACE])
                Ball->Stuck = false;
        }
    }
    void Update(float dt)
    {
        // update objects
        Ball->Move(dt, this->Width);
        // check for collisions
        this->DoCollisions();
        // update particles
        Particles->Update(dt, *Ball, 2, glm::vec2(Ball->Radius / 2.0f));
        // // update PowerUps
        // this->UpdatePowerUps(dt);
        // reduce shake time
        if (ShakeTime > 0.0f)
        {
            ShakeTime -= dt;
            if (ShakeTime <= 0.0f)
                Effects->Shake = false;
        }
        // check loss condition
        if (Ball->Position.y >= this->Height) // did ball reach bottom edge?
        {
            // --this->Lives;
            // // did the player lose all his lives? : game over
            // if (this->Lives == 0)
            // {
                this->ResetLevel();
            //     this->State = GAME_MENU;
            // }
            this->ResetPlayer();
        }
        // // check win condition
        // if (this->State == GAME_ACTIVE && this->Levels[this->Level].IsCompleted())
        // {
        //     this->ResetLevel();
        //     this->ResetPlayer();
        //     Effects->Chaos = true;
        //     this->State = GAME_WIN;
        // }
    }
    void Render()
    {
        // Renderer->DrawSprite(ResourceManager::getTexture("background"), glm::vec2(200.0f, 200.0f), glm::vec2(300, 400), 45.0f);

        if (this->State == GAME_ACTIVE || this->State == GAME_MENU || this->State == GAME_WIN)
        {
            // begin rendering to postprocessing framebuffer
            Effects->BeginRender();
            // draw background
            Renderer->DrawSprite(ResourceManager::getTexture("background"), glm::vec2(0.0f, 0.0f), glm::vec2(this->Width, this->Height), 0.0f);
            // Renderer->DrawSprite(ResourceManager::getTexture("face"), glm::vec2(200.0f, 200.0f), glm::vec2(300, 400), 0.0f);
            // draw level
            this->Levels[this->Level].Draw(*Renderer);
            // draw player
            Player->Draw(*Renderer);
            // // draw PowerUps
            // for (PowerUp &powerUp : this->PowerUps)
            //     if (!powerUp.Destroyed)
            //         powerUp.Draw(*Renderer);
            // draw particles	
            Particles->Draw();
            // draw ball
            Ball->Draw(*Renderer);            
            // end rendering to postprocessing framebuffer
            Effects->EndRender();
            // render postprocessing quad
            Effects->Render(glfwGetTime());
            // // render text (don't include in postprocessing)
            // std::stringstream ss; ss << this->Lives;
            // Text->RenderText("Lives:" + ss.str(), 5.0f, 5.0f, 1.0f);
        }
        if (this->State == GAME_MENU)
        {
            Text->RenderText("Press ENTER to start", 250.0f, this->Height / 2.0f, 1.0f);
            Text->RenderText("Press W or S to select level", 245.0f, this->Height / 2.0f + 20.0f, 0.75f);
        }
        if (this->State == GAME_WIN)
        {
            Text->RenderText("You WON!!!", 320.0f, this->Height / 2.0f - 20.0f, 1.0f, glm::vec3(0.0f, 1.0f, 0.0f));
            Text->RenderText("Press ENTER to retry or ESC to quit", 130.0f, this->Height / 2.0f, 1.0f, glm::vec3(1.0f, 1.0f, 0.0f));
        }
    }
    void DoCollisions()
    {
        for (GameObject &box : this->Levels[this->Level].Bricks)
        {
            if (!box.Destroyed)
            {
                Collision collision = CheckCollision(*Ball, box);
                if (std::get<0>(collision)) // if collision is true
                {
                    // destroy block if not solid
                    if (!box.IsSolid)
                    {
                        box.Destroyed = true;
                        // this->SpawnPowerUps(box);
                        // SoundEngine->play2D(FileSystem::getPath("resources/audio/bleep.mp3").c_str(), false);
                    }
                    else
                    {   
                        // if block is solid, enable shake effect
                        ShakeTime = 0.05f;
                        Effects->Shake = true;
                        // SoundEngine->play2D(FileSystem::getPath("resources/audio/bleep.mp3").c_str(), false);
                    }
                    // collision resolution
                    Direction dir = std::get<1>(collision);
                    glm::vec2 diff_vector = std::get<2>(collision);
                    // if (!(Ball->PassThrough && !box.IsSolid)) // don't do collision resolution on non-solid bricks if pass-through is activated
                    // {
                        if (dir == LEFT || dir == RIGHT) // horizontal collision
                        {
                            Ball->Velocity.x = -Ball->Velocity.x; // reverse horizontal velocity
                            // relocate
                            float penetration = Ball->Radius - std::abs(diff_vector.x);
                            if (dir == LEFT)
                                Ball->Position.x += penetration; // move ball to right
                            else
                                Ball->Position.x -= penetration; // move ball to left;
                        }
                        else // vertical collision
                        {
                            Ball->Velocity.y = -Ball->Velocity.y; // reverse vertical velocity
                            // relocate
                            float penetration = Ball->Radius - std::abs(diff_vector.y);
                            if (dir == UP)
                                Ball->Position.y -= penetration; // move ball bback up
                            else
                                Ball->Position.y += penetration; // move ball back down
                        }
                    // }
                }
            }
        }
        // also check collisions on PowerUps and if so, activate them
        // for (PowerUp &powerUp : this->PowerUps)
        // {
        //     if (!powerUp.Destroyed)
        //     {
        //         // first check if powerup passed bottom edge, if so: keep as inactive and destroy
        //         if (powerUp.Position.y >= this->Height)
        //             powerUp.Destroyed = true;

        //         if (CheckCollision(*Player, powerUp))
        //         {	// collided with player, now activate powerup
        //             ActivatePowerUp(powerUp);
        //             powerUp.Destroyed = true;
        //             powerUp.Activated = true;
        //             SoundEngine->play2D(FileSystem::getPath("resources/audio/powerup.wav").c_str(), false);
        //         }
        //     }
        // }
        // and finally check collisions for player pad (unless stuck)
        Collision result = CheckCollision(*Ball, *Player);
        if (!Ball->Stuck && std::get<0>(result))
        {
            // check where it hit the board, and change velocity based on where it hit the board
            float centerBoard = Player->Position.x + Player->Size.x / 2.0f;
            float distance = (Ball->Position.x + Ball->Radius) - centerBoard;
            float percentage = distance / (Player->Size.x / 2.0f);
            // then move accordingly
            float strength = 2.0f;
            glm::vec2 oldVelocity = Ball->Velocity;
            Ball->Velocity.x = INITIAL_BALL_VELOCITY.x * percentage * strength; 
            // Ball->Velocity.y = -Ball->Velocity.y;
            Ball->Velocity = glm::normalize(Ball->Velocity) * glm::length(oldVelocity); // keep speed consistent over both axes (multiply by length of old velocity, so total strength is not changed)
            // fix sticky paddle
            Ball->Velocity.y = -1.0f * abs(Ball->Velocity.y);

            // // if Sticky powerup is activated, also stick ball to paddle once new velocity vectors were calculated
            // Ball->Stuck = Ball->Sticky;

            // SoundEngine->play2D(FileSystem::getPath("resources/audio/bleep.wav").c_str(), false);
        }
    }
    // reset
    void ResetLevel()
    {
        if (this->Level == 0)
            this->Levels[0].Load(FileSystem::getPath("resources/levels/one.lvl").c_str(), this->Width, this->Height / 2);
        else if (this->Level == 1)
            this->Levels[1].Load("levels/two.lvl", this->Width, this->Height / 2);
        else if (this->Level == 2)
            this->Levels[2].Load("levels/three.lvl", this->Width, this->Height / 2);
        else if (this->Level == 3)
            this->Levels[3].Load("levels/four.lvl", this->Width, this->Height / 2);

        this->Lives = 3;
    }
    void ResetPlayer()
    {
        // reset player/ball stats
        Player->Size = PLAYER_SIZE;
        Player->Position = glm::vec2(this->Width / 2.0f - PLAYER_SIZE.x / 2.0f, this->Height - PLAYER_SIZE.y);
        Ball->Reset(Player->Position + glm::vec2(PLAYER_SIZE.x / 2.0f - BALL_RADIUS, -(BALL_RADIUS * 2.0f)), INITIAL_BALL_VELOCITY);
        // // also disable all active powerups
        // Effects->Chaos = Effects->Confuse = false;
        // Ball->PassThrough = Ball->Sticky = false;
        // Player->Color = glm::vec3(1.0f);
        // Ball->Color = glm::vec3(1.0f);
    }
    // powerups
    void SpawnPowerUps(GameObject &block)
    {
        if (ShouldSpawn(75)) // 1 in 75 chance
            this->PowerUps.push_back(PowerUp("speed", glm::vec3(0.5f, 0.5f, 1.0f), 0.0f, block.Position, ResourceManager::getTexture("powerup_speed")));
        if (ShouldSpawn(75))
            this->PowerUps.push_back(PowerUp("sticky", glm::vec3(1.0f, 0.5f, 1.0f), 20.0f, block.Position, ResourceManager::getTexture("powerup_sticky")));
        if (ShouldSpawn(75))
            this->PowerUps.push_back(PowerUp("pass-through", glm::vec3(0.5f, 1.0f, 0.5f), 10.0f, block.Position, ResourceManager::getTexture("powerup_passthrough")));
        if (ShouldSpawn(75))
            this->PowerUps.push_back(PowerUp("pad-size-increase", glm::vec3(1.0f, 0.6f, 0.4), 0.0f, block.Position, ResourceManager::getTexture("powerup_increase")));
        if (ShouldSpawn(15)) // Negative powerups should spawn more often
            this->PowerUps.push_back(PowerUp("confuse", glm::vec3(1.0f, 0.3f, 0.3f), 15.0f, block.Position, ResourceManager::getTexture("powerup_confuse")));
        if (ShouldSpawn(15))
            this->PowerUps.push_back(PowerUp("chaos", glm::vec3(0.9f, 0.25f, 0.25f), 15.0f, block.Position, ResourceManager::getTexture("powerup_chaos")));
    }
    void UpdatePowerUps(float dt)
    {
        for (PowerUp &powerUp : this->PowerUps)
        {
            powerUp.Position += powerUp.Velocity * dt;
            if (powerUp.Activated)
            {
                powerUp.Duration -= dt;

                if (powerUp.Duration <= 0.0f)
                {
                    // remove powerup from list (will later be removed)
                    powerUp.Activated = false;
                    // deactivate effects
                    if (powerUp.Type == "sticky")
                    {
                        if (!IsOtherPowerUpActive(this->PowerUps, "sticky"))
                        {	// only reset if no other PowerUp of type sticky is active
                            Ball->Sticky = false;
                            Player->Color = glm::vec3(1.0f);
                        }
                    }
                    else if (powerUp.Type == "pass-through")
                    {
                        if (!IsOtherPowerUpActive(this->PowerUps, "pass-through"))
                        {	// only reset if no other PowerUp of type pass-through is active
                            Ball->PassThrough = false;
                            Ball->Color = glm::vec3(1.0f);
                        }
                    }
                    else if (powerUp.Type == "confuse")
                    {
                        if (!IsOtherPowerUpActive(this->PowerUps, "confuse"))
                        {	// only reset if no other PowerUp of type confuse is active
                            Effects->Confuse = false;
                        }
                    }
                    else if (powerUp.Type == "chaos")
                    {
                        if (!IsOtherPowerUpActive(this->PowerUps, "chaos"))
                        {	// only reset if no other PowerUp of type chaos is active
                            Effects->Chaos = false;
                        }
                    }
                }
            }
        }
        // Remove all PowerUps from vector that are destroyed AND !activated (thus either off the map or finished)
        // Note we use a lambda expression to remove each PowerUp which is destroyed and not activated
        this->PowerUps.erase(std::remove_if(this->PowerUps.begin(), this->PowerUps.end(),
            [](const PowerUp &powerUp) { return powerUp.Destroyed && !powerUp.Activated; }
        ), this->PowerUps.end());
    }
};

#endif