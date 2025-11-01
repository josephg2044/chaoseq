#pragma once
#include <functional>
#include <vector>

struct ODESystem {
    int dim;
    std::function<void(const std::vector<float> &, std::vector<float> &, float)>
        deriv;
};

class IntegratorRK4 {
  public:
    IntegratorRK4() = default;

    void step(const ODESystem &sys, std::vector<float> &state, float t,
              float dt) {
        const int n = sys.dim;
        if (k1.size() != (size_t)n) {
            k1.resize(n);
            k2.resize(n);
            k3.resize(n);
            k4.resize(n);
            tmp.resize(n);
        }

        sys.deriv(state, k1, t);

        for (int i = 0; i < n; i++)
            tmp[i] = state[i] + 0.5f * dt * k1[i];
        sys.deriv(tmp, k2, t + 0.5f * dt);

        for (int i = 0; i < n; i++)
            tmp[i] = state[i] + 0.5f * dt * k2[i];
        sys.deriv(tmp, k3, t + 0.5f * dt);

        for (int i = 0; i < n; i++)
            tmp[i] = state[i] + dt * k3[i];
        sys.deriv(tmp, k4, t + dt);

        for (int i = 0; i < n; i++) {
            state[i] +=
                (dt / 6.0f) * (k1[i] + 2.0f * k2[i] + 2.0f * k3[i] + k4[i]);
        }
    }

  private:
    std::vector<float> k1, k2, k3, k4, tmp;
};
