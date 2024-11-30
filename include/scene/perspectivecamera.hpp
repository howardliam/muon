#pragma once

#include "scene/basecamera.hpp"

namespace muon {

    class PerspectiveCamera : public BaseCamera {
    public:
        void setProjection(float fov_x, float aspect_ratio, float z_near, float z_far);
        void lookAt(glm::vec3 position, glm::vec3 target, glm::vec3 up = {0.0f, 1.0f, 0.0f});
    };

}
