#include "camera.hpp"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

void Camera::set_ortho_projection(float left, float right, float bottom, float top, float z_near, float z_far) {
    projection = glm::ortho(left, right, bottom, top, z_near, z_far);
}

void Camera::set_perspective_projection(float fov, float aspect_ratio, float z_near, float z_far) {
    projection = glm::perspective(fov, aspect_ratio, z_near, z_far);
}

void Camera::look_at(glm::vec3 position, glm::vec3 target, glm::vec3 up) {
    view = glm::lookAt(position, target, up);
}
