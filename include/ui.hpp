#pragma once

#include "simulation.hpp"

void draw_ui(simulation_state &state, Camera &camera, orbit_camera &orbit,
             bool &mouse_look_enabled, bool &orbit_dragging);
