#include "scene/camera/orthographic.hpp"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

namespace muon {

    void OrthographicCamera::setProjection(float left, float right, float bottom, float top, float z_near, float z_far) {
        projection = glm::ortho(left, right, bottom, top, z_near, z_far);
    }

    void OrthographicCamera::lookAt(glm::vec3 position, glm::vec3 target, glm::vec3 up) {
        view = glm::lookAt(position, target, up);
    }

}
