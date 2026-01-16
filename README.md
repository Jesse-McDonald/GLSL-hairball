# GLSL-hairball
As part of a GPU Programming class I built a GLSL shader designed to simulate hair interaction a
viscous uid. The tip position of each hair is calculated by the shader at runtime based its base and
the inertia of the hair. The rest of the hair is then interpolated using a quadratic B-spline. Each
hair also supports being colored in a gradient along the spline. As a proof of concept, the project
features a sphere evenly covered in hair that rotates back and forth.

