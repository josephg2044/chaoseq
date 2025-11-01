#pragma once

#include "Camera.hpp"
#include "Integrator.hpp"
#include "ODESystems.hpp"
#include "Shader.hpp"
#include "glitter.hpp"
#include <vector>

enum class system_type {
    lorenz = 0,
    rossler,
    thomas,
    aizawa,
    dadras,
    chen,
    lorenz83,
    halvorsen,
    rabinovich,
    three_scroll,
    sprott,
    four_wing
};

enum class camera_mode { fps = 0, orbit = 1 };

struct orbit_camera {
    glm::vec3 target{0.0f, 0.0f, 0.0f};
    float radius = 30.0f;
    float yaw = -90.0f;
    float pitch = 20.0f;
    float rotate_speed = 0.25f;
    float zoom_speed = 0.15f;
    float min_radius = 2.0f;
    float max_radius = 500.0f;

    glm::vec3 position() const;
    glm::mat4 view() const;
    void clamp_pitch();
    void clamp_radius();
};

struct simulation_state {
    system_type current_system = system_type::lorenz;

    LorenzArgs lorenz_args;
    RosslerArgs rossler_args;
    ThomasArgs thomas_args;
    AizawaArgs aizawa_args;
    DadrasArgs dadras_args;
    ChenArgs chen_args;
    Lorenz83Args lorenz83_args;
    HalvorsenArgs halvorsen_args;
    RabinovichArgs rabinovich_args;
    ThreeScrollArgs three_scroll_args;
    SprottArgs sprott_args;
    FourWingArgs four_wing_args;

    ODESystem system;
    IntegratorRK4 integrator;

    std::vector<float> state;
    float t = 0.0f;
    float base_dt = 0.01f;
    float time_accumulator = 0.0f;
    bool paused = false;

    camera_mode current_camera_mode = camera_mode::fps;

    bool show_axes = true;
    float axes_length = 10.0f;

    GLuint axes_vao = 0;
    GLuint axes_vbo = 0;

    glm::vec3 axes_color{0.5f, 0.5f, 0.6f};
    bool particles_monochrome = false;

    size_t particle_count = 10000;
    float particle_spawn_radius = 1.5f;
    std::vector<glm::vec3> particle_positions;
    std::vector<float> particle_phases;
    bool particle_spawn_from_origin = false;
    float particle_origin_jitter = 0.02f;
    float particle_color_speed = 0.35f;
    GLuint particle_vao = 0;
    GLuint particle_pos_vbo = 0;
    GLuint particle_phase_vbo = 0;
    float particle_point_size = 3.0f;
    size_t particle_buffer_capacity = 0;
};

glm::vec3 evaluate_derivative(const simulation_state &state,
                              const glm::vec3 &position);
glm::vec3 integrate_particle_rk4(const simulation_state &state,
                                 const glm::vec3 &position, float dt);
float compute_spawn_phase(const glm::vec3 &position);
void ensure_particle_buffers(simulation_state &state);
void initialize_particle_field(simulation_state &state);
void update_particle_gpu(simulation_state &state);
void advance_particles(simulation_state &state, float dt);
bool compute_particle_bounds(const simulation_state &state, glm::vec3 &out_min,
                             glm::vec3 &out_max);
void upload_axes_vertices(const simulation_state &state);
void create_axes(simulation_state &state);
glm::vec3 reset_simulation(simulation_state &state);
void step_simulation(simulation_state &state, float frame_dt);
void draw_particles(const Shader &shader, const simulation_state &state,
                    const glm::mat4 &view, const glm::mat4 &proj);
void draw_axes(const Shader &shader, const simulation_state &state,
               const glm::mat4 &mvp);
void sync_orbit_from_fps(const simulation_state &state, const Camera &fps,
                         orbit_camera &orbit);
void sync_fps_from_orbit(const orbit_camera &orbit, Camera &fps);
void frame_particles(simulation_state &state, orbit_camera &orbit, Camera &fps,
                     bool &orbit_dragging);
