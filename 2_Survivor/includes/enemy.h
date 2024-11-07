#include <glm/glm.hpp>

#include <vector>

#include "bullet.h"

class Enemy
{
private:
    const float speed = 0.003f;
    const int frame_width = 80; // enemy width
    const int frame_height = 80; // enemy height

    bool alive = true;

public:
    Enemy()
    {
        loadEnemy = true;

        // enemy spawn edge
        enum class SpawnEdge
        {
            Up = 0,
            Down,
            Left,
            Right
        };

        // put enemy at random location of the border
        SpawnEdge edge = (SpawnEdge)(rand() % 4);

        // generate enemy
        switch (edge)
        {
        case SpawnEdge::Up:
            position.x = (static_cast<double>(rand()) / RAND_MAX) * 2.0 - 1.0; 
            position.y = 1.0f;
            break;
        case SpawnEdge::Down:
            position.x = (static_cast<double>(rand()) / RAND_MAX) * 2.0 - 1.0; 
            position.y = -1.0f;
            break;
        case SpawnEdge::Left:
            position.x = -1.0f; 
            position.y = (static_cast<double>(rand()) / RAND_MAX) * 2.0 - 1.0;
            break;
        case SpawnEdge::Right:
            position.x = 1.0f; 
            position.y = (static_cast<double>(rand()) / RAND_MAX) * 2.0 - 1.0;
            break;
        
        default:
            break;
        }

    }
    ~Enemy(){}

    bool checkBulletCollision(const Bullet& bullet)
    {
        glm::vec3 dist = bullet.position - position;
        // std::cout << glm::length(dist) << std::endl;
        if (glm::length(dist) < 0.1f)
            return true;
        return false;
    }
    bool checkPlayerCollision(const glm::vec3& player_position)
    {
        // look enemy as point, and look it if in the rectangle of player
        glm::vec3 check_position = position;

        bool is_overlap_x = check_position.x >= player_position.x - 0.0625f && check_position.x <= player_position.x + 0.0625f;
        bool is_overlap_y = check_position.y >= player_position.y - 0.1111f && check_position.y <= player_position.y + 0.1111f;

        return is_overlap_x && is_overlap_y;
    }

    void move(const glm::vec3& player_position)
    {
        float dir_x = player_position.x - position.x;
        float dir_y = player_position.y - position.y;
        double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y);
        if (len_dir != 0)
        {
            double normalized_x = dir_x / len_dir;
            double normalized_y = dir_y / len_dir;
            position.x += speed * normalized_x;
            position.y += speed * normalized_y;
        }
        
        if (dir_x < 0)
            facing_left = true;
        else if (dir_x > 0)
            facing_left = false;
    }

    void draw()
    {
        drawEnemy = true;
    }

    void hurted()
    {
        alive = false;
    }

    bool checkAlive()
    {
        return alive;
    }





    glm::vec3 position{0.0f, 0.0f, 0.0f};
    bool loadEnemy = false;
    bool drawEnemy = false;
    bool facing_left = false;
};