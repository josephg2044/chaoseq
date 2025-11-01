#pragma once
#include "glitter.hpp"
#include <cmath>

class Camera {
  public:
    glm::vec3 position{0.0f, 0.0f, 30.0f};
    float yaw = -90.0f; // looking toward -Z by default
    float pitch = 0.0f;
    float fov = 45.0f;

    float move_speed = 10.0f;
    float mouse_sens = 0.1f;

    bool first_mouse = true;
    double last_x = k_default_window_width / 2.0;
    double last_y = k_default_window_height / 2.0;

    glm::mat4 get_view() const {
        glm::vec3 front = forward_dir();
        return glm::lookAt(position, position + front, glm::vec3(0, 1, 0));
    }

    glm::mat4 get_proj(float aspect) const {
        return glm::perspective(glm::radians(fov), aspect, 0.1f, 500.0f);
    }

    void process_keyboard(GLFWwindow *win, float dt) {
        glm::vec3 front = forward_dir();
        glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3(0, 1, 0)));
        glm::vec3 up = glm::normalize(glm::cross(right, front));

        float vel = move_speed * dt;
        if (glfwGetKey(win, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
            glfwGetKey(win, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS) {
            vel *= 2.0f;
        }
        if (glfwGetKey(win, GLFW_KEY_W) == GLFW_PRESS)
            position += front * vel;
        if (glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS)
            position -= front * vel;
        if (glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS)
            position -= right * vel;
        if (glfwGetKey(win, GLFW_KEY_D) == GLFW_PRESS)
            position += right * vel;
        if (glfwGetKey(win, GLFW_KEY_Q) == GLFW_PRESS)
            position -= up * vel;
        if (glfwGetKey(win, GLFW_KEY_E) == GLFW_PRESS)
            position += up * vel;
    }

    void process_mouse(double xpos, double ypos, bool constrain_pitch = true) {
        if (first_mouse) {
            last_x = xpos;
            last_y = ypos;
            first_mouse = false;
        }
        float xoffset = float(xpos - last_x);
        float yoffset = float(last_y - ypos);
        last_x = xpos;
        last_y = ypos;

        xoffset *= mouse_sens;
        yoffset *= mouse_sens;

        yaw += xoffset;
        pitch += yoffset;

        if (constrain_pitch) {
            if (pitch > 89.0f)
                pitch = 89.0f;
            if (pitch < -89.0f)
                pitch = -89.0f;
        }
    }

    glm::vec3 forward_dir() const {
        float cy = std::cos(glm::radians(yaw));
        float sy = std::sin(glm::radians(yaw));
        float cp = std::cos(glm::radians(pitch));
        float sp = std::sin(glm::radians(pitch));
        glm::vec3 dir;
        dir.x = cy * cp;
        dir.y = sp;
        dir.z = sy * cp;
        return glm::normalize(dir);
    }
};
