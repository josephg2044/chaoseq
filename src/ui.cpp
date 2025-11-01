#include "ui.hpp"

#include <imgui.h>

using namespace std;
using namespace glm;

void draw_ui(simulation_state &state, Camera &camera, orbit_camera &orbit,
             bool &mouse_look_enabled, bool &orbit_dragging) {
    ImGui::Begin("Simulation Controls");

    static const char *system_names[] = {"Lorenz",
                                         "R\u00F6ssler",
                                         "Thomas",
                                         "Aizawa (Langford)",
                                         "Dadras",
                                         "Chen",
                                         "Lorenz '83",
                                         "Halvorsen",
                                         "Rabinovich-Fabrikant",
                                         "Three-Scroll Unified",
                                         "Sprott",
                                         "Four-Wing"};
    int system_index = static_cast<int>(state.current_system);
    if (ImGui::Combo("System", &system_index, system_names,
                     IM_ARRAYSIZE(system_names))) {
        state.current_system = static_cast<system_type>(system_index);
        orbit.target = reset_simulation(state);
    }

    bool args_changed = false;
    switch (state.current_system) {
    case system_type::lorenz:
        ImGui::Text("Lorenz Parameters");
        args_changed |=
            ImGui::SliderFloat("sigma", &state.lorenz_args.sigma, 0.1f, 50.0f);
        args_changed |=
            ImGui::SliderFloat("rho", &state.lorenz_args.rho, 0.1f, 60.0f);
        args_changed |=
            ImGui::SliderFloat("beta", &state.lorenz_args.beta, 0.1f, 10.0f);
        if (args_changed) {
            state.system = make_lorenz_system(state.lorenz_args);
        }
        break;
    case system_type::rossler:
        ImGui::Text("R\u00F6ssler Parameters");
        args_changed |=
            ImGui::SliderFloat("a", &state.rossler_args.a, -1.0f, 1.0f);
        args_changed |=
            ImGui::SliderFloat("b", &state.rossler_args.b, -1.0f, 1.0f);
        args_changed |=
            ImGui::SliderFloat("c", &state.rossler_args.c, 1.0f, 20.0f);
        if (args_changed) {
            state.system = make_rossler_system(state.rossler_args);
        }
        break;
    case system_type::thomas:
        ImGui::Text("Thomas Parameters");
        args_changed |=
            ImGui::SliderFloat("b", &state.thomas_args.b, 0.01f, 1.0f);
        if (args_changed) {
            state.system = make_thomas_system(state.thomas_args);
        }
        break;
    case system_type::aizawa:
        ImGui::Text("Aizawa / Langford Parameters");
        args_changed |=
            ImGui::SliderFloat("a", &state.aizawa_args.a, 0.0f, 2.0f);
        args_changed |=
            ImGui::SliderFloat("b", &state.aizawa_args.b, 0.0f, 2.0f);
        args_changed |=
            ImGui::SliderFloat("c", &state.aizawa_args.c, 0.0f, 2.0f);
        args_changed |=
            ImGui::SliderFloat("d", &state.aizawa_args.d, 0.0f, 5.0f);
        args_changed |=
            ImGui::SliderFloat("e", &state.aizawa_args.e, 0.0f, 1.0f);
        args_changed |=
            ImGui::SliderFloat("f", &state.aizawa_args.f, 0.0f, 1.0f);
        if (args_changed) {
            state.system = make_aizawa_system(state.aizawa_args);
        }
        break;
    case system_type::dadras:
        ImGui::Text("Dadras Parameters");
        args_changed |=
            ImGui::SliderFloat("a", &state.dadras_args.a, 0.0f, 5.0f);
        args_changed |=
            ImGui::SliderFloat("b", &state.dadras_args.b, 0.0f, 5.0f);
        args_changed |=
            ImGui::SliderFloat("c", &state.dadras_args.c, 0.0f, 5.0f);
        args_changed |=
            ImGui::SliderFloat("d", &state.dadras_args.d, 0.0f, 5.0f);
        args_changed |=
            ImGui::SliderFloat("e", &state.dadras_args.e, 0.0f, 15.0f);
        if (args_changed) {
            state.system = make_dadras_system(state.dadras_args);
        }
        break;
    case system_type::chen:
        ImGui::Text("Chen Parameters");
        args_changed |=
            ImGui::SliderFloat("alpha", &state.chen_args.alpha, -20.0f, 20.0f);
        args_changed |=
            ImGui::SliderFloat("beta", &state.chen_args.beta, -20.0f, 0.0f);
        args_changed |=
            ImGui::SliderFloat("delta", &state.chen_args.delta, -5.0f, 5.0f);
        if (args_changed) {
            state.system = make_chen_system(state.chen_args);
        }
        break;
    case system_type::lorenz83:
        ImGui::Text("Lorenz '83 Parameters");
        args_changed |=
            ImGui::SliderFloat("a", &state.lorenz83_args.a, 0.0f, 5.0f);
        args_changed |=
            ImGui::SliderFloat("b", &state.lorenz83_args.b, 0.0f, 15.0f);
        args_changed |=
            ImGui::SliderFloat("f", &state.lorenz83_args.f, 0.0f, 10.0f);
        args_changed |=
            ImGui::SliderFloat("g", &state.lorenz83_args.g, 0.0f, 10.0f);
        if (args_changed) {
            state.system = make_lorenz83_system(state.lorenz83_args);
        }
        break;
    case system_type::halvorsen:
        ImGui::Text("Halvorsen Parameters");
        args_changed |=
            ImGui::SliderFloat("a", &state.halvorsen_args.a, 0.0f, 5.0f);
        if (args_changed) {
            state.system = make_halvorsen_system(state.halvorsen_args);
        }
        break;
    case system_type::rabinovich:
        ImGui::Text("Rabinovich-Fabrikant Parameters");
        args_changed |= ImGui::SliderFloat(
            "alpha", &state.rabinovich_args.alpha, 0.0f, 1.0f);
        args_changed |= ImGui::SliderFloat(
            "gamma", &state.rabinovich_args.gamma, 0.0f, 1.0f);
        if (args_changed) {
            state.system = make_rabinovich_system(state.rabinovich_args);
        }
        break;
    case system_type::three_scroll:
        ImGui::Text("Three-Scroll Unified Parameters");
        args_changed |=
            ImGui::SliderFloat("a", &state.three_scroll_args.a, 0.0f, 60.0f);
        args_changed |=
            ImGui::SliderFloat("b", &state.three_scroll_args.b, 0.0f, 60.0f);
        args_changed |=
            ImGui::SliderFloat("c", &state.three_scroll_args.c, 0.0f, 5.0f);
        args_changed |=
            ImGui::SliderFloat("d", &state.three_scroll_args.d, 0.0f, 2.0f);
        args_changed |=
            ImGui::SliderFloat("e", &state.three_scroll_args.e, 0.0f, 5.0f);
        args_changed |=
            ImGui::SliderFloat("f", &state.three_scroll_args.f, 0.0f, 30.0f);
        if (args_changed) {
            state.system = make_three_scroll_system(state.three_scroll_args);
        }
        break;
    case system_type::sprott:
        ImGui::Text("Sprott Parameters");
        args_changed |=
            ImGui::SliderFloat("a", &state.sprott_args.a, 0.0f, 5.0f);
        args_changed |=
            ImGui::SliderFloat("b", &state.sprott_args.b, 0.0f, 5.0f);
        if (args_changed) {
            state.system = make_sprott_system(state.sprott_args);
        }
        break;
    case system_type::four_wing:
        ImGui::Text("Four-Wing Parameters");
        args_changed |=
            ImGui::SliderFloat("a", &state.four_wing_args.a, -1.0f, 1.0f);
        args_changed |=
            ImGui::SliderFloat("b", &state.four_wing_args.b, -0.5f, 0.5f);
        args_changed |=
            ImGui::SliderFloat("c", &state.four_wing_args.c, -1.0f, 0.5f);
        if (args_changed) {
            state.system = make_four_wing_system(state.four_wing_args);
        }
        break;
    }

    if (args_changed) {
        state.integrator = IntegratorRK4{};
        state.time_accumulator = 0.0f;
    }

    if (ImGui::Button("Reset Args / Initial State")) {
        orbit.target = reset_simulation(state);
        args_changed = false;
    }

    ImGui::Checkbox("Paused", &state.paused);
    ImGui::Checkbox("Show Axes", &state.show_axes);
    if (ImGui::SliderFloat("Axes Half-Length", &state.axes_length, 0.5f,
                           300.0f)) {
        upload_axes_vertices(state);
    }
    float base_dt = state.base_dt;
    if (ImGui::SliderFloat("dt (sim step)", &base_dt, 0.0001f, 0.2f, "%.5f",
                           ImGuiSliderFlags_Logarithmic)) {
        state.base_dt = glm::clamp(base_dt, 1e-6f, 0.2f);
        state.time_accumulator = 0.0f;
    }
    ImGui::Text("dt: %.5f", state.base_dt);

    ImGui::Separator();
    ImGui::Text("Particles");
    int particle_count = static_cast<int>(state.particle_count);
    if (ImGui::SliderInt("Particle Count", &particle_count, 1000, 500000,
                         "%d")) {
        state.particle_count = static_cast<size_t>(particle_count);
        initialize_particle_field(state);
        update_particle_gpu(state);
    }
    if (ImGui::Checkbox("Spawn From Origin",
                        &state.particle_spawn_from_origin)) {
        initialize_particle_field(state);
        update_particle_gpu(state);
    }
    if (state.particle_spawn_from_origin) {
        if (ImGui::SliderFloat("Origin Jitter", &state.particle_origin_jitter,
                               0.0001f, 0.5f, "%.5f",
                               ImGuiSliderFlags_Logarithmic)) {
            state.particle_origin_jitter =
                glm::clamp(state.particle_origin_jitter, 1e-5f, 1.0f);
            initialize_particle_field(state);
            update_particle_gpu(state);
        }
    } else if (ImGui::SliderFloat("Spawn Radius", &state.particle_spawn_radius,
                                  0.1f, 10.0f)) {
        initialize_particle_field(state);
        update_particle_gpu(state);
    }
    ImGui::SliderFloat("Particle Size", &state.particle_point_size, 0.5f,
                       60.0f);
    ImGui::SliderFloat("Color Speed", &state.particle_color_speed, 0.0f, 2.0f);
    ImGui::Checkbox("Monochrome Particles", &state.particles_monochrome);
    if (ImGui::Button("Reseed Particles")) {
        initialize_particle_field(state);
        update_particle_gpu(state);
    }

    ImGui::Separator();
    ImGui::Text("Camera");
    int camera_mode_index =
        (state.current_camera_mode == camera_mode::fps) ? 0 : 1;
    const char *camera_modes[] = {"Fly (FPS)", "Orbit"};
    if (ImGui::Combo("Camera Mode", &camera_mode_index, camera_modes,
                     IM_ARRAYSIZE(camera_modes))) {
        const camera_mode new_mode =
            (camera_mode_index == 0) ? camera_mode::fps : camera_mode::orbit;
        if (new_mode != state.current_camera_mode) {
            GLFWwindow *current_window = glfwGetCurrentContext();
            if (new_mode == camera_mode::orbit) {
                sync_orbit_from_fps(state, camera, orbit);
            } else {
                sync_fps_from_orbit(orbit, camera);
                mouse_look_enabled = false;
                orbit_dragging = false;
                if (current_window) {
                    glfwSetInputMode(current_window, GLFW_CURSOR,
                                     GLFW_CURSOR_NORMAL);
                }
            }
            state.current_camera_mode = new_mode;
        }
    }
    if (state.current_camera_mode == camera_mode::fps) {
        ImGui::Text("WASD/QE to move, hold Right Mouse to look (Shift = fast)");
    } else {
        ImGui::Text(
            "Orbit: Left-drag rotate, scroll to zoom, press F to frame");
    }
    ImGui::SliderFloat("Speed", &camera.move_speed, 0.5f, 50.0f);
    ImGui::SliderFloat("FOV", &camera.fov, 10.0f, 90.0f);

    ImGui::Separator();
    ImGui::Text("t = %.3f", state.t);
    if (state.system.dim >= 3 && state.state.size() >= 3) {
        ImGui::Text("state = (%.3f, %.3f, %.3f)", state.state[0],
                    state.state[1], state.state[2]);
    }
    float speed_magnitude = 0.0f;
    if (state.system.dim >= 3 && state.state.size() >= 3) {
        const vec3 state_vector(state.state[0], state.state[1], state.state[2]);
        const vec3 derivative = evaluate_derivative(state, state_vector);
        speed_magnitude = length(derivative);
    }
    ImGui::Text("speed = %.3f", speed_magnitude);

    ImGui::End();
}
