#pragma once

#include "scene/basecamera.hpp"

namespace muon {

    class OrthographicCamera : public BaseCamera {
    public:
        void setProjection(float left, float right, float bottom, float top, float z_near, float z_far);
        void lookAt(glm::vec3 position, glm::vec3 target, glm::vec3 up = {0.0f, 1.0f, 0.0f}) override;
    };

}
