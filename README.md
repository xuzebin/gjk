# gjk
CS6703 Computational Geometry final project.

Implementation of 3D collision detection for convex polyhedrons using the Gilbert–Johnson–Keerthi (GJK) algorithm.

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
Build GFrame library first and then the GJK source. Just run the all in one script below.

```
sh buildAll.sh
```

#### Run
```
cd build && ./gjk
```

## Environment
- This project was developed and tested only on macOS.

## GJK - 3D Collision Detection for Convex Polyhedrons
- The original GJK algorithm is used to determine the minimum distance between two convex sets. In this implementation, only a simple boolean intersection query is returned. 
- Two convex Polyhedrons intersects iff their Minkowski Difference contains the origin. Basically, it is searching the origin by iteratively constructing a simplex inside the Minkowski Difference that attempts to enclose the origin.

## Usage
The algorithm was implemented in the file `gjk.h`.
You can easily use it for 3D collision detection without dependency on GFrame (although I did include two header files (cvec3.h, matrix4.h) in GFrame).

The usage is fairly simple:

```c++
// The 3d points of shape 1 and shape 2
std::vector<Vec3> vertices1;
std::vector<Vec3> vertices2;

// Adding points
//...

// Create Shape objects
Shape shape1(vertices1);
Shape shape2(vertices2);

// Create the GJK object
GJK gjk;

//game loop
while (1) {
	// Update if modelMatrix is dirty, 
	// we need those 3d points in the world frame.
	shape1.update(modelMatrix1);
	shape2.update(modelMatrix2);
	
	//intersection query
	bool intersected = gjk.intersect(shape1, shape2);
	//...
}
```
