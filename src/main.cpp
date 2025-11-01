#include "simulation.hpp"
#include "ui.hpp"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;
using namespace glm;

static simulation_state g_sim;
static Camera g_camera;
static orbit_camera g_orbit_camera;
static int g_window_width = k_default_window_width;
static int g_window_height = k_default_window_height;
static bool g_mouse_look_enabled = false;
static bool g_orbit_dragging = false;
static double g_orbit_last_x = 0.0;
static double g_orbit_last_y = 0.0;
static bool g_frame_key_down = false;
static bool g_show_ui = true;
static bool g_ui_toggle_key_down = false;

static string load_text_file(const char *path) {
    ifstream file(path, ios::binary);
    if (!file) {
        cerr << "Failed to open shader file: " << path << "\n";
        return {};
    }
    stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

static void mouse_callback(GLFWwindow *, double xpos, double ypos) {
    if (g_sim.current_camera_mode != camera_mode::fps ||
        !g_mouse_look_enabled) {
        return;
    }
    g_camera.process_mouse(xpos, ypos);
}

static void scroll_callback(GLFWwindow *, double, double yoffset) {
    if (g_sim.current_camera_mode == camera_mode::fps) {
        g_camera.fov = glm::clamp(g_camera.fov - static_cast<float>(yoffset),
                                  10.0f, 90.0f);
    } else {
        g_orbit_camera.radius -=
            static_cast<float>(yoffset) * g_orbit_camera.zoom_speed;
        g_orbit_camera.clamp_radius();
        sync_fps_from_orbit(g_orbit_camera, g_camera);
    }
}

static void framebuffer_size_callback(GLFWwindow *, int width, int height) {
    g_window_width = width;
    g_window_height = height;
    glViewport(0, 0, width, height);
}

static void handle_orbit_drag(GLFWwindow *window) {
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        double xpos = 0.0;
        double ypos = 0.0;
        glfwGetCursorPos(window, &xpos, &ypos);
        if (!g_orbit_dragging) {
            g_orbit_dragging = true;
            g_orbit_last_x = xpos;
            g_orbit_last_y = ypos;
        }
        const double dx = xpos - g_orbit_last_x;
        const double dy = ypos - g_orbit_last_y;
        g_orbit_last_x = xpos;
        g_orbit_last_y = ypos;
        g_orbit_camera.yaw +=
            static_cast<float>(dx) * g_orbit_camera.rotate_speed;
        g_orbit_camera.pitch -=
            static_cast<float>(dy) * g_orbit_camera.rotate_speed;
        g_orbit_camera.clamp_pitch();
        sync_fps_from_orbit(g_orbit_camera, g_camera);
    } else {
        g_orbit_dragging = false;
    }
}

int main(int, char **) {
    if (!glfwInit()) {
        cerr << "Failed to init GLFW\n";
        return EXIT_FAILURE;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    GLFWwindow *window = glfwCreateWindow(g_window_width, g_window_height,
                                          "3D ODE Simulator", nullptr, nullptr);
    if (!window) {
        cerr << "Failed to create OpenGL context\n";
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGL()) {
        cerr << "Failed to init GLAD\n";
        return EXIT_FAILURE;
    }

    if (const GLubyte *version = glGetString(GL_VERSION)) {
        cout << "OpenGL " << version << "\n";
    }
    glEnable(GL_MULTISAMPLE);

    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 400");

    glViewport(0, 0, g_window_width, g_window_height);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_PROGRAM_POINT_SIZE);

    create_axes(g_sim);
    g_orbit_camera.target = reset_simulation(g_sim);
    sync_fps_from_orbit(g_orbit_camera, g_camera);

    const string axes_vertex_source = load_text_file("shader/basic.vert");
    const string axes_fragment_source = load_text_file("shader/basic.frag");
    Shader axes_shader(axes_vertex_source.c_str(),
                       axes_fragment_source.c_str());

    const string particle_vertex_source =
        load_text_file("shader/particle.vert");
    const string particle_fragment_source =
        load_text_file("shader/particle.frag");
    Shader particle_shader(particle_vertex_source.c_str(),
                           particle_fragment_source.c_str());

    double last_time = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }

        ImGuiIO &loop_io = ImGui::GetIO();
        if (g_sim.current_camera_mode == camera_mode::fps) {
            const bool want_capture = g_show_ui && loop_io.WantCaptureMouse;
            if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) ==
                    GLFW_PRESS &&
                !want_capture) {
                if (!g_mouse_look_enabled) {
                    g_mouse_look_enabled = true;
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                    g_camera.first_mouse = true;
                }
            } else if (g_mouse_look_enabled) {
                g_mouse_look_enabled = false;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
        } else {
            if (g_mouse_look_enabled) {
                g_mouse_look_enabled = false;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
            handle_orbit_drag(window);
        }

        const double now = glfwGetTime();
        const float frame_dt = static_cast<float>(now - last_time);
        last_time = now;

        const bool i_pressed = (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS);
        if (i_pressed && !g_ui_toggle_key_down) {
            g_show_ui = !g_show_ui;
        }
        g_ui_toggle_key_down = i_pressed;

        if (g_sim.current_camera_mode == camera_mode::fps &&
            !(g_show_ui && loop_io.WantCaptureKeyboard)) {
            g_camera.process_keyboard(window, frame_dt);
        }

        const bool f_pressed = (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS);
        if (f_pressed && !g_frame_key_down) {
            frame_particles(g_sim, g_orbit_camera, g_camera, g_orbit_dragging);
        }
        g_frame_key_down = f_pressed;

        step_simulation(g_sim, frame_dt);
        update_particle_gpu(g_sim);

        const float aspect = (g_window_height > 0)
                                 ? static_cast<float>(g_window_width) /
                                       static_cast<float>(g_window_height)
                                 : 1.0f;
        const mat4 projection = g_camera.get_proj(aspect);
        mat4 view_matrix;
        if (g_sim.current_camera_mode == camera_mode::fps) {
            view_matrix = g_camera.get_view();
        } else {
            view_matrix = g_orbit_camera.view();
            sync_fps_from_orbit(g_orbit_camera, g_camera);
        }
        const mat4 model_matrix(1.0f);
        const mat4 mvp = projection * view_matrix * model_matrix;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (g_show_ui) {
            draw_ui(g_sim, g_camera, g_orbit_camera, g_mouse_look_enabled,
                    g_orbit_dragging);
        }

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        draw_axes(axes_shader, g_sim, mvp);
        draw_particles(particle_shader, g_sim, view_matrix, projection);

        if (g_show_ui) {
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        } else {
            ImGui::EndFrame();
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glDeleteVertexArrays(1, &g_sim.axes_vao);
    glDeleteBuffers(1, &g_sim.axes_vbo);
    if (g_sim.particle_vao) {
        glDeleteVertexArrays(1, &g_sim.particle_vao);
        glDeleteBuffers(1, &g_sim.particle_pos_vbo);
        glDeleteBuffers(1, &g_sim.particle_phase_vbo);
    }

    glfwTerminate();
    return EXIT_SUCCESS;
}
