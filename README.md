# gjk
CS6703 Computational Geometry final project.

3D collision detection implementation and visualization using the Gilbert–Johnson–Keerthi (GJK) distance algorithm.

## Dependencies
- GFrame (an OpenGL framework)
- OpenGL
- GLFW3
- GLEW
- GLUT
- CMake

## Instructions
Before building the project, make sure you have all needed libraries installed. On macOS, both OpenGL and GLUT were already installed. You could install GLEW and GLFW3 via homebrew. GFrame will also be installed automatically when you (recursive) clone and build the project.

#### Clone
Don't forget the `--recursive` as you will need GFrame.

```
git clone --recursive https://github.com/xuzebin/gjk.git
```
#### Build
Build GFrame library first and then GJK source. Just run the all in one script below.

```
sh buildAll.sh
```

#### Run
```
cd build && ./gjk
```

## Environment
- This project was developed and tested only on macOS.