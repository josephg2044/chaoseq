#pragma once

#include "Integrator.hpp"
#include "glitter.hpp"
#include <cmath>
#include <vector>

inline void resize_deriv(std::vector<float> &dxdt, int dimension) {
    if (static_cast<int>(dxdt.size()) != dimension) {
        dxdt.resize(static_cast<size_t>(dimension));
    }
}

struct LorenzArgs {
    float sigma = 10.0f;
    float rho = 28.0f;
    float beta = 8.0f / 3.0f;
};

inline glm::vec3 deriv_lorenz(const LorenzArgs &args, const glm::vec3 &value) {
    return glm::vec3(args.sigma * (value.y - value.x),
                     value.x * (args.rho - value.z) - value.y,
                     value.x * value.y - args.beta * value.z);
}

inline ODESystem make_lorenz_system(const LorenzArgs &args) {
    ODESystem system;
    system.dim = 3;
    system.deriv = [args](const std::vector<float> &state,
                          std::vector<float> &derivative, float) {
        resize_deriv(derivative, 3);
        glm::vec3 value(state[0], state[1], state[2]);
        glm::vec3 delta = deriv_lorenz(args, value);
        derivative[0] = delta.x;
        derivative[1] = delta.y;
        derivative[2] = delta.z;
    };
    return system;
}

struct RosslerArgs {
    float a = 0.2f;
    float b = 0.2f;
    float c = 5.7f;
};

inline glm::vec3 deriv_rossler(const RosslerArgs &args,
                               const glm::vec3 &value) {
    return glm::vec3(-(value.y + value.z), value.x + args.a * value.y,
                     args.b + value.z * (value.x - args.c));
}

inline ODESystem make_rossler_system(const RosslerArgs &args) {
    ODESystem system;
    system.dim = 3;
    system.deriv = [args](const std::vector<float> &state,
                          std::vector<float> &derivative, float) {
        resize_deriv(derivative, 3);
        glm::vec3 value(state[0], state[1], state[2]);
        glm::vec3 delta = deriv_rossler(args, value);
        derivative[0] = delta.x;
        derivative[1] = delta.y;
        derivative[2] = delta.z;
    };
    return system;
}

struct ThomasArgs {
    float b = 0.208186f;
};

inline glm::vec3 deriv_thomas(const ThomasArgs &args, const glm::vec3 &value) {
    return glm::vec3(std::sin(value.y) - args.b * value.x,
                     std::sin(value.z) - args.b * value.y,
                     std::sin(value.x) - args.b * value.z);
}

inline ODESystem make_thomas_system(const ThomasArgs &args) {
    ODESystem system;
    system.dim = 3;
    system.deriv = [args](const std::vector<float> &state,
                          std::vector<float> &derivative, float) {
        resize_deriv(derivative, 3);
        glm::vec3 value(state[0], state[1], state[2]);
        glm::vec3 delta = deriv_thomas(args, value);
        derivative[0] = delta.x;
        derivative[1] = delta.y;
        derivative[2] = delta.z;
    };
    return system;
}

struct AizawaArgs {
    float a = 0.95f;
    float b = 0.7f;
    float c = 0.6f;
    float d = 3.5f;
    float e = 0.25f;
    float f = 0.1f;
};

inline glm::vec3 deriv_aizawa(const AizawaArgs &args, const glm::vec3 &value) {
    const float radius_squared = value.x * value.x + value.y * value.y;
    return glm::vec3((value.z - args.b) * value.x - args.d * value.y,
                     args.d * value.x + (value.z - args.b) * value.y,
                     args.c + args.a * value.z -
                         (value.z * value.z * value.z) / 3.0f -
                         radius_squared * (1.0f + args.e * value.z) +
                         args.f * value.z * value.x * value.x * value.x);
}

inline ODESystem make_aizawa_system(const AizawaArgs &args) {
    ODESystem system;
    system.dim = 3;
    system.deriv = [args](const std::vector<float> &state,
                          std::vector<float> &derivative, float) {
        resize_deriv(derivative, 3);
        glm::vec3 value(state[0], state[1], state[2]);
        glm::vec3 delta = deriv_aizawa(args, value);
        derivative[0] = delta.x;
        derivative[1] = delta.y;
        derivative[2] = delta.z;
    };
    return system;
}

struct DadrasArgs {
    float a = 3.0f;
    float b = 2.7f;
    float c = 1.7f;
    float d = 2.0f;
    float e = 9.0f;
};

inline glm::vec3 deriv_dadras(const DadrasArgs &args, const glm::vec3 &value) {
    return glm::vec3(value.y - args.a * value.x + args.b * value.y * value.z,
                     args.c * value.y - value.x * value.z + value.z,
                     args.d * value.x * value.y - args.e * value.z);
}

inline ODESystem make_dadras_system(const DadrasArgs &args) {
    ODESystem system;
    system.dim = 3;
    system.deriv = [args](const std::vector<float> &state,
                          std::vector<float> &derivative, float) {
        resize_deriv(derivative, 3);
        glm::vec3 value(state[0], state[1], state[2]);
        glm::vec3 delta = deriv_dadras(args, value);
        derivative[0] = delta.x;
        derivative[1] = delta.y;
        derivative[2] = delta.z;
    };
    return system;
}

struct ChenArgs {
    float alpha = 5.0f;
    float beta = -10.0f;
    float delta = -0.38f;
};

inline glm::vec3 deriv_chen(const ChenArgs &args, const glm::vec3 &value) {
    return glm::vec3(args.alpha * value.x - value.y * value.z,
                     args.beta * value.y + value.x * value.z,
                     args.delta * value.z + (value.x * value.y) / 3.0f);
}

inline ODESystem make_chen_system(const ChenArgs &args) {
    ODESystem system;
    system.dim = 3;
    system.deriv = [args](const std::vector<float> &state,
                          std::vector<float> &derivative, float) {
        resize_deriv(derivative, 3);
        glm::vec3 value(state[0], state[1], state[2]);
        glm::vec3 delta = deriv_chen(args, value);
        derivative[0] = delta.x;
        derivative[1] = delta.y;
        derivative[2] = delta.z;
    };
    return system;
}

struct Lorenz83Args {
    float a = 0.95f;
    float b = 7.91f;
    float f = 4.83f;
    float g = 4.66f;
};

inline glm::vec3 deriv_lorenz83(const Lorenz83Args &args,
                                const glm::vec3 &value) {
    return glm::vec3(-args.a * value.x - value.y * value.y - value.z * value.z +
                         args.a * args.f,
                     -value.y + value.x * value.y - args.b * value.x * value.z +
                         args.g,
                     -value.z + args.b * value.x * value.y + value.x * value.z);
}

inline ODESystem make_lorenz83_system(const Lorenz83Args &args) {
    ODESystem system;
    system.dim = 3;
    system.deriv = [args](const std::vector<float> &state,
                          std::vector<float> &derivative, float) {
        resize_deriv(derivative, 3);
        glm::vec3 value(state[0], state[1], state[2]);
        glm::vec3 delta = deriv_lorenz83(args, value);
        derivative[0] = delta.x;
        derivative[1] = delta.y;
        derivative[2] = delta.z;
    };
    return system;
}

struct HalvorsenArgs {
    float a = 1.4f;
};

inline glm::vec3 deriv_halvorsen(const HalvorsenArgs &args,
                                 const glm::vec3 &value) {
    return glm::vec3(
        -args.a * value.x - 4.0f * value.y - 4.0f * value.z - value.y * value.y,
        -args.a * value.y - 4.0f * value.z - 4.0f * value.x - value.z * value.z,
        -args.a * value.z - 4.0f * value.x - 4.0f * value.y -
            value.x * value.x);
}

inline ODESystem make_halvorsen_system(const HalvorsenArgs &args) {
    ODESystem system;
    system.dim = 3;
    system.deriv = [args](const std::vector<float> &state,
                          std::vector<float> &derivative, float) {
        resize_deriv(derivative, 3);
        glm::vec3 value(state[0], state[1], state[2]);
        glm::vec3 delta = deriv_halvorsen(args, value);
        derivative[0] = delta.x;
        derivative[1] = delta.y;
        derivative[2] = delta.z;
    };
    return system;
}

struct RabinovichArgs {
    float alpha = 0.14f;
    float gamma = 0.1f;
};

inline glm::vec3 deriv_rabinovich(const RabinovichArgs &args,
                                  const glm::vec3 &value) {
    return glm::vec3(value.y * (value.z - 1.0f + value.x * value.x) +
                         args.gamma * value.x,
                     value.x * (3.0f * value.z + 1.0f - value.x * value.x) +
                         args.gamma * value.y,
                     -2.0f * value.z * (args.alpha + value.x * value.y));
}

inline ODESystem make_rabinovich_system(const RabinovichArgs &args) {
    ODESystem system;
    system.dim = 3;
    system.deriv = [args](const std::vector<float> &state,
                          std::vector<float> &derivative, float) {
        resize_deriv(derivative, 3);
        glm::vec3 value(state[0], state[1], state[2]);
        glm::vec3 delta = deriv_rabinovich(args, value);
        derivative[0] = delta.x;
        derivative[1] = delta.y;
        derivative[2] = delta.z;
    };
    return system;
}

struct ThreeScrollArgs {
    float a = 32.48f;
    float b = 45.84f;
    float c = 1.18f;
    float d = 0.13f;
    float e = 0.57f;
    float f = 14.7f;
};

inline glm::vec3 deriv_three_scroll(const ThreeScrollArgs &args,
                                    const glm::vec3 &value) {
    return glm::vec3(args.a * (value.y - value.x) + args.d * value.x * value.z,
                     args.b * value.x + args.f * value.y - value.x * value.z,
                     args.c * value.z + args.e * value.x * value.y +
                         args.e * value.y * value.z);
}

inline ODESystem make_three_scroll_system(const ThreeScrollArgs &args) {
    ODESystem system;
    system.dim = 3;
    system.deriv = [args](const std::vector<float> &state,
                          std::vector<float> &derivative, float) {
        resize_deriv(derivative, 3);
        glm::vec3 value(state[0], state[1], state[2]);
        glm::vec3 delta = deriv_three_scroll(args, value);
        derivative[0] = delta.x;
        derivative[1] = delta.y;
        derivative[2] = delta.z;
    };
    return system;
}

struct SprottArgs {
    float a = 2.07f;
    float b = 1.79f;
};

inline glm::vec3 deriv_sprott(const SprottArgs &args, const glm::vec3 &value) {
    return glm::vec3(-args.a * value.x + value.y, -value.z + value.x * value.y,
                     args.b + value.z * (value.x - 14.0f));
}

inline ODESystem make_sprott_system(const SprottArgs &args) {
    ODESystem system;
    system.dim = 3;
    system.deriv = [args](const std::vector<float> &state,
                          std::vector<float> &derivative, float) {
        resize_deriv(derivative, 3);
        glm::vec3 value(state[0], state[1], state[2]);
        glm::vec3 delta = deriv_sprott(args, value);
        derivative[0] = delta.x;
        derivative[1] = delta.y;
        derivative[2] = delta.z;
    };
    return system;
}

struct FourWingArgs {
    float a = 0.2f;
    float b = 0.01f;
    float c = -0.4f;
};

inline glm::vec3 deriv_four_wing(const FourWingArgs &args,
                                 const glm::vec3 &value) {
    return glm::vec3(value.y * value.z + args.b, value.x * value.z + args.c,
                     -value.x * value.y + args.a);
}

inline ODESystem make_four_wing_system(const FourWingArgs &args) {
    ODESystem system;
    system.dim = 3;
    system.deriv = [args](const std::vector<float> &state,
                          std::vector<float> &derivative, float) {
        resize_deriv(derivative, 3);
        glm::vec3 value(state[0], state[1], state[2]);
        glm::vec3 delta = deriv_four_wing(args, value);
        derivative[0] = delta.x;
        derivative[1] = delta.y;
        derivative[2] = delta.z;
    };
    return system;
}
