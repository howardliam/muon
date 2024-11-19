#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

class Camera {
public:
    Camera() = default;
    ~Camera() = default;

    void set_ortho_projection(float left, float right, float bottom, float top, float z_near, float z_far);
    void set_perspective_projection(float fov, float aspect_ratio, float z_near, float z_far);

    void look_at(glm::vec3 position, glm::vec3 target, glm::vec3 up = {0.0f, 1.0f, 0.0f});

    const glm::mat4 &get_projection() const { return projection; }
    const glm::mat4 &get_view() const { return view; }

private:
    glm::mat4 projection{1.0f};
    glm::mat4 view{1.0f};
};
