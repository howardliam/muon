#include "scene/camera/perspective.hpp"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

namespace muon {

    void PerspectiveCamera::setProjection(float fov_x, float aspect_ratio, float z_near, float z_far) {
        float half_fov_x = fov_x / 2;
        float fov_y = 2 * atan(tan(half_fov_x) / aspect_ratio);
        projection = glm::perspective(fov_y, aspect_ratio, z_near, z_far);
    }

    void PerspectiveCamera::lookAt(glm::vec3 position, glm::vec3 target, glm::vec3 up) {
        view = glm::lookAt(position, target, up);
    }

}
