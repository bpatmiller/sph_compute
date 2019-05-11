# sph_compute

to build:
make build

to launch:
make run

Click and drag to orbit the camera

Press/hold w/s to zoom in/out

There are 9 color settings, accessible by pressing numbers 1-9 (which display density, pressure, spatial hash, acceleration, force, initial position, normals, velocity, and speed respectively).

Pressing t spawns a repulser on the particle the user selects by mouse. Accuracy with mouse-ray casting is intentionally low (the particle radii are artificial increased) to improve performance, as we simply care about spawning a repulser somewhere on the mouse ray, not necessarily the closest.

Similarly, pressing y spawns an attractor at a particle intersecting the mouse ray.

Pressing p toggles a pipe which removes particles from the center of the floor and propels them out of a pipe on the wall.

Pressing o resets the simulation.

Pressing r toggles SSAO mode between SSAO, only occlusion, and only color.

Pressing l toggles pressure computation method between the Tate equation and the ideal gas law
