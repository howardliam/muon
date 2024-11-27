#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace muon {

    class Camera {
    public:
        Camera() = default;
        ~Camera() = default;

        void setOrthoProjection(float left, float right, float bottom, float top, float z_near, float z_far);
        void setPerspectiveProjection(float fov_x, float aspect_ratio, float z_near, float z_far);

        void lookAt(glm::vec3 position, glm::vec3 target, glm::vec3 up = {0.0f, 1.0f, 0.0f});

        const glm::mat4 &getProjection() const { return projection; }
        const glm::mat4 &getView() const { return view; }

    private:
        glm::mat4 projection{1.0f};
        glm::mat4 view{1.0f};
    };

}
