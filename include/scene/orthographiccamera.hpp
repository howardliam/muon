#pragma once

#include "scene/basecamera.hpp"

namespace muon {

    class OrthographicCamera : public BaseCamera {
    public:
        void setProjection(float left, float right, float bottom, float top);
    };

}
