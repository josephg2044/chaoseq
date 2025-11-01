#include "simulation.hpp"

#include <algorithm>
#include <cmath>
#include <random>
#include <thread>

using namespace std;
using namespace glm;

glm::vec3 orbit_camera::position() const {
    const float yaw_rad = radians(yaw);
    const float pitch_rad = radians(pitch);
    vec3 direction;
    direction.x = cos(pitch_rad) * cos(yaw_rad);
    direction.y = sin(pitch_rad);
    direction.z = cos(pitch_rad) * sin(yaw_rad);
    return target - direction * radius;
}

glm::mat4 orbit_camera::view() const {
    return lookAt(position(), target, vec3(0, 1, 0));
}

void orbit_camera::clamp_pitch() {
    pitch = glm::clamp(pitch, -89.0f, 89.0f);
}

void orbit_camera::clamp_radius() {
    radius = glm::clamp(radius, min_radius, max_radius);
}

glm::vec3 evaluate_derivative(const simulation_state &state,
                              const vec3 &position) {
    switch (state.current_system) {
    case system_type::lorenz:
        return deriv_lorenz(state.lorenz_args, position);
    case system_type::rossler:
        return deriv_rossler(state.rossler_args, position);
    case system_type::thomas:
        return deriv_thomas(state.thomas_args, position);
    case system_type::aizawa:
        return deriv_aizawa(state.aizawa_args, position);
    case system_type::dadras:
        return deriv_dadras(state.dadras_args, position);
    case system_type::chen:
        return deriv_chen(state.chen_args, position);
    case system_type::lorenz83:
        return deriv_lorenz83(state.lorenz83_args, position);
    case system_type::halvorsen:
        return deriv_halvorsen(state.halvorsen_args, position);
    case system_type::rabinovich:
        return deriv_rabinovich(state.rabinovich_args, position);
    case system_type::three_scroll:
        return deriv_three_scroll(state.three_scroll_args, position);
    case system_type::sprott:
        return deriv_sprott(state.sprott_args, position);
    case system_type::four_wing:
        return deriv_four_wing(state.four_wing_args, position);
    }
    return vec3(0.0f);
}

glm::vec3 integrate_particle_rk4(const simulation_state &state,
                                 const vec3 &position, float dt) {
    const vec3 k1 = evaluate_derivative(state, position);
    const vec3 k2 = evaluate_derivative(state, position + 0.5f * dt * k1);
    const vec3 k3 = evaluate_derivative(state, position + 0.5f * dt * k2);
    const vec3 k4 = evaluate_derivative(state, position + dt * k3);
    return position + (dt / 6.0f) * (k1 + 2.0f * k2 + 2.0f * k3 + k4);
}

float compute_spawn_phase(const vec3 &position) {
    vec3 direction = position;
    float distance = length(direction);
    if (!isfinite(distance) || distance < 1e-6f) {
        direction = vec3(1.0f, 0.0f, 0.0f);
    } else {
        direction /= distance;
    }
    const float azimuth = atan2(direction.y, direction.x);
    const float elevation = acos(glm::clamp(direction.z, -1.0f, 1.0f));
    float phase = azimuth + elevation;
    phase = fmod(phase, two_pi<float>());
    if (phase < 0.0f) {
        phase += two_pi<float>();
    }
    return phase;
}

void ensure_particle_buffers(simulation_state &state) {
    if (state.particle_vao != 0) {
        return;
    }
    glGenVertexArrays(1, &state.particle_vao);
    glGenBuffers(1, &state.particle_pos_vbo);
    glGenBuffers(1, &state.particle_phase_vbo);
    state.particle_buffer_capacity = 0;

    glBindVertexArray(state.particle_vao);

    glBindBuffer(GL_ARRAY_BUFFER, state.particle_pos_vbo);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3),
                          static_cast<void *>(nullptr));

    glBindBuffer(GL_ARRAY_BUFFER, state.particle_phase_vbo);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(float),
                          static_cast<void *>(nullptr));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void initialize_particle_field(simulation_state &state) {
    ensure_particle_buffers(state);

    if (state.particle_count == 0) {
        state.particle_count = 1;
    }
    state.particle_positions.resize(state.particle_count);
    state.particle_phases.resize(state.particle_count);

    mt19937 rng{random_device{}()};
    normal_distribution<float> normal_dist(0.0f, 1.0f);

    for (size_t index = 0; index < state.particle_count; ++index) {
        vec3 direction(normal_dist(rng), normal_dist(rng), normal_dist(rng));
        if (dot(direction, direction) < 1e-6f) {
            direction = vec3(1.0f, 0.0f, 0.0f);
        }
        direction = normalize(direction);

        vec3 position;
        if (state.particle_spawn_from_origin) {
            const float jitter_scale =
                glm::max(state.particle_origin_jitter, 1e-4f);
            float radius = abs(normal_dist(rng)) * jitter_scale;
            radius = glm::clamp(radius, 1e-5f, jitter_scale * 2.0f);
            position = direction * radius;
        } else {
            const float radius = abs(normal_dist(rng)) * 0.5f + 0.5f;
            position = direction * radius * state.particle_spawn_radius;
        }
        state.particle_positions[index] = position;
        state.particle_phases[index] = compute_spawn_phase(position);
    }

    glBindBuffer(GL_ARRAY_BUFFER, state.particle_phase_vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 state.particle_phases.size() * sizeof(float),
                 state.particle_phases.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void update_particle_gpu(simulation_state &state) {
    if (state.particle_positions.empty()) {
        return;
    }
    ensure_particle_buffers(state);
    glBindBuffer(GL_ARRAY_BUFFER, state.particle_pos_vbo);
    const size_t required_bytes =
        state.particle_positions.size() * sizeof(vec3);
    if (state.particle_buffer_capacity != state.particle_positions.size()) {
        glBufferData(GL_ARRAY_BUFFER, required_bytes,
                     state.particle_positions.data(), GL_DYNAMIC_DRAW);
        state.particle_buffer_capacity = state.particle_positions.size();
    } else {
        glBufferSubData(GL_ARRAY_BUFFER, 0, required_bytes,
                        state.particle_positions.data());
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void advance_particles(simulation_state &state, float dt) {
    const size_t particle_total = state.particle_positions.size();
    if (particle_total == 0) {
        return;
    }

    auto integrate_range = [&](size_t begin, size_t end) {
        for (size_t index = begin; index < end; ++index) {
            state.particle_positions[index] =
                integrate_particle_rk4(state, state.particle_positions[index],
                                       dt);
        }
    };

    constexpr size_t k_min_per_thread = 4096;
    const unsigned int hardware_threads =
        std::max(1u, thread::hardware_concurrency());
    const unsigned int max_threads =
        static_cast<unsigned int>((particle_total + k_min_per_thread - 1) /
                                  k_min_per_thread);
    const unsigned int thread_count =
        std::min(std::max(1u, max_threads), hardware_threads);

    if (thread_count <= 1) {
        integrate_range(0, particle_total);
        return;
    }

    vector<thread> workers;
    workers.reserve(thread_count - 1);
    size_t start = 0;
    const size_t base_chunk = particle_total / thread_count;
    const size_t remainder = particle_total % thread_count;
    for (unsigned int t = 0; t < thread_count; ++t) {
        const size_t chunk = base_chunk + (t < remainder ? 1 : 0);
        const size_t end = start + chunk;
        if (t + 1 == thread_count) {
            integrate_range(start, end);
        } else {
            workers.emplace_back([&, start, end]() { integrate_range(start, end); });
        }
        start = end;
    }
    for (thread &worker : workers) {
        worker.join();
    }
}

bool compute_particle_bounds(const simulation_state &state, vec3 &out_min,
                             vec3 &out_max) {
    if (state.particle_positions.empty()) {
        return false;
    }
    out_min = state.particle_positions.front();
    out_max = out_min;
    for (const vec3 &position : state.particle_positions) {
        out_min = glm::min(out_min, position);
        out_max = glm::max(out_max, position);
    }
    return true;
}

void upload_axes_vertices(const simulation_state &state) {
    const float length = state.axes_length;
    const float vertices[18] = {-length, 0.0f, 0.0f, length, 0.0f, 0.0f,
                                0.0f,   -length, 0.0f, 0.0f,  length, 0.0f,
                                0.0f,   0.0f,   -length, 0.0f, 0.0f,  length};
    glBindBuffer(GL_ARRAY_BUFFER, state.axes_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
}

void create_axes(simulation_state &state) {
    glGenVertexArrays(1, &state.axes_vao);
    glGenBuffers(1, &state.axes_vbo);

    glBindVertexArray(state.axes_vao);
    glBindBuffer(GL_ARRAY_BUFFER, state.axes_vbo);
    upload_axes_vertices(state);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                          static_cast<void *>(nullptr));
    glBindVertexArray(0);
}

glm::vec3 reset_simulation(simulation_state &state) {
    vector<float> initial_state{0.1f, 0.0f, 0.0f};

    switch (state.current_system) {
    case system_type::lorenz:
        state.system = make_lorenz_system(state.lorenz_args);
        initial_state = {1.0f, 1.0f, 1.0f};
        break;
    case system_type::rossler:
        state.system = make_rossler_system(state.rossler_args);
        break;
    case system_type::thomas:
        state.system = make_thomas_system(state.thomas_args);
        initial_state = {0.2f, 0.0f, -0.2f};
        break;
    case system_type::aizawa:
        state.system = make_aizawa_system(state.aizawa_args);
        break;
    case system_type::dadras:
        state.system = make_dadras_system(state.dadras_args);
        initial_state = {0.1f, 0.1f, 0.1f};
        break;
    case system_type::chen:
        state.system = make_chen_system(state.chen_args);
        break;
    case system_type::lorenz83:
        state.system = make_lorenz83_system(state.lorenz83_args);
        break;
    case system_type::halvorsen:
        state.system = make_halvorsen_system(state.halvorsen_args);
        break;
    case system_type::rabinovich:
        state.system = make_rabinovich_system(state.rabinovich_args);
        break;
    case system_type::three_scroll:
        state.system = make_three_scroll_system(state.three_scroll_args);
        break;
    case system_type::sprott:
        state.system = make_sprott_system(state.sprott_args);
        initial_state = {0.1f, 0.1f, 0.1f};
        break;
    case system_type::four_wing:
        state.system = make_four_wing_system(state.four_wing_args);
        initial_state = {0.1f, 0.1f, 0.1f};
        break;
    }

    state.state = initial_state;
    state.t = 0.0f;
    state.time_accumulator = 0.0f;
    state.integrator = IntegratorRK4{};

    initialize_particle_field(state);
    update_particle_gpu(state);

    return vec3(state.state[0], state.state[1], state.state[2]);
}

void step_simulation(simulation_state &state, float frame_dt) {
    if (state.paused) {
        return;
    }

    state.time_accumulator += frame_dt;
    const float max_accumulator = 2.0f;
    if (state.time_accumulator > max_accumulator) {
        state.time_accumulator = max_accumulator;
    }

    const float step_dt = glm::clamp(state.base_dt, 1e-6f, 0.2f);

    int iterations = 0;
    constexpr int max_iterations = 4096;
    while (state.time_accumulator >= step_dt && iterations < max_iterations) {
        state.integrator.step(state.system, state.state, state.t, step_dt);
        advance_particles(state, step_dt);
        state.t += step_dt;
        state.time_accumulator -= step_dt;
        ++iterations;
    }
    if (iterations == max_iterations) {
        state.time_accumulator = 0.0f;
    }
}

void draw_particles(const Shader &shader, const simulation_state &state,
                    const mat4 &view_matrix, const mat4 &projection) {
    if (state.particle_positions.empty()) {
        return;
    }
    shader.use();
    shader.set_mat4("uView", view_matrix);
    shader.set_mat4("uProj", projection);
    shader.set_float("uPointSize", state.particle_point_size);
    shader.set_float("uTime", state.t);
    shader.set_float("uColorSpeed", state.particle_color_speed);
    shader.set_int("uMonochrome", state.particles_monochrome ? 1 : 0);
    glBindVertexArray(state.particle_vao);
    glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(state.particle_positions.size()));
    glBindVertexArray(0);
}

void draw_axes(const Shader &shader, const simulation_state &state,
               const mat4 &mvp) {
    if (!state.show_axes) {
        return;
    }
    shader.use();
    shader.set_mat4("uMVP", mvp);
    shader.set_vec3("uColor", state.axes_color);
    glBindVertexArray(state.axes_vao);
    glDrawArrays(GL_LINES, 0, 6);
    glBindVertexArray(0);
}

void sync_orbit_from_fps(const simulation_state &state, const Camera &fps,
                         orbit_camera &orbit) {
    vec3 target_position(0.0f);
    if (!state.particle_positions.empty()) {
        target_position = state.particle_positions.front();
    }
    orbit.target = target_position;
    const vec3 camera_position = fps.position;
    vec3 delta = target_position - camera_position;
    float distance = length(delta);
    if (distance < 1.0f) {
        distance = 10.0f;
    }
    orbit.radius = distance;
    if (distance > 0.0f) {
        const vec3 direction = normalize(delta);
        orbit.pitch =
            degrees(asin(glm::clamp(direction.y, -1.0f, 1.0f)));
        orbit.yaw = degrees(atan2(direction.z, direction.x));
    } else {
        orbit.pitch = fps.pitch;
        orbit.yaw = fps.yaw;
    }
    orbit.clamp_pitch();
    orbit.clamp_radius();
}

void sync_fps_from_orbit(const orbit_camera &orbit, Camera &fps) {
    const vec3 camera_position = orbit.position();
    fps.position = camera_position;
    const vec3 direction = normalize(orbit.target - camera_position);
    fps.pitch = degrees(asin(glm::clamp(direction.y, -1.0f, 1.0f)));
    fps.yaw = degrees(atan2(direction.z, direction.x));
}

void frame_particles(simulation_state &state, orbit_camera &orbit, Camera &fps,
                     bool &orbit_dragging) {
    vec3 bounds_min, bounds_max;
    if (!compute_particle_bounds(state, bounds_min, bounds_max)) {
        return;
    }
    const vec3 center = 0.5f * (bounds_min + bounds_max);
    const vec3 diagonal = bounds_max - bounds_min;
    float radius = length(diagonal);
    if (radius < 1.0f) {
        radius = 6.0f;
    }
    orbit.target = center;
    orbit.radius = glm::clamp(radius * 0.6f, orbit.min_radius, orbit.max_radius);
    orbit.clamp_radius();
    orbit_dragging = false;
    sync_fps_from_orbit(orbit, fps);
}
