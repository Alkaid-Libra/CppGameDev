#include <glm/glm.hpp>


class Bullet
{
public:
    Bullet() = default;
    ~Bullet() = default;

    void draw()
    {
        orangeRedCircle = true;
    }

    glm::vec3 position{0.0f, 0.0f, 0.0f};

private:
    bool orangeRedCircle = false;

};