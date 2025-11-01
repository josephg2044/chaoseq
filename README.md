This project uses [Glitter] becuase I am too lazy to write boiletplate.

# chaoseq – Particle Chaotic ODE Visualizer

This is a viewer for chaotic differential equations. It uses an RK4 integrator and a GPU particle renderer. There is an ImGui interface so you can tweak parameters in real time. This project only runs on linux systems with a GPU capable of running OpenGL 4.0.

## Features
- **Preset Library:** Lorenz, Rössler, Thomas, Aizawa (Langford), Dadras, Chen, Lorenz '83, Halvorsen, Rabinovich-Fabrikant, Three-Scroll Unified, Sprott, and Four-Wing.
- **Particle Field:** Many particles are spawned at the origin and time-evolved with respect to the attractor for visualization. 
- **Camera Modes:** FPS-style fly and orbit cameras. Quick framing (`F`) focuses center of mass of active particles.
- **Paramter Controls:** The ImGui panel exposes the ODE parameters, integration step size, particle seeding, color behaviour, and render toggles for real time changes.

## Usage

```bash
git clone https://example.com/chaoseq.git
cd chaoseq
cmake -S . -B build
cmake --build build
./build/chaoseq/chaoseq
```

### Controls

| Action | Binding |
| ------ | ------- |
| FPS move | `WASDQE` + mouse look (hold RMB) |
| Orbit rotate/zoom | Drag LMB / mouse wheel |
| Frame particles | `F` |
| Toggle ImGui | `I` |
| Exit | `Esc` |

## Screenshots and GIFS
