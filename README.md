# TODO
# OpenGL Viewer
Completed rasterization project for CAP4730 - Computational Structures in Computer Graphics

### Notice
This project was developed on Linux. **The CMake has not been tested on other platforms and the included Makefile contains Linux-specific flags**. The assignment description does not state that the Makefile must be cross-platform and I do not have an easy way to test on MacOS or Windows.

## Building and Testing
Simply run the command 'make' in the repo's build directory (if you would like you can run 'cmake ..' to build the Makefile, but a cmake generated Makefile should be included). The binary is named 'OpenGL_Viewer'. If the cmake does not work for whatever reason, there is a redundant Makefile in the project's root that will compile 'OpenGL_Viewer' inside the build directory. You must 'cd' into build before running to avoid breaking the relative file paths.

## References:
- https://opentk.net/learn/chapter1/3-element-buffer-objects.html
- https://github.com/ruange/Gouraud-Shading-and-Phong-Shading
- https://ogldev.org/www/tutorial16/tutorial16.html
- https://learnopengl.com/Model-Loading/Model
- https://casual-effects.com/data/
- Lecture Slides
- Class Textbooks:
  1. "Foundations of 3D Computer Graphics", Steven Gortler
  2. "Fundamentals of Computer Graphics", Steve Marschner and Peter Shirley.
