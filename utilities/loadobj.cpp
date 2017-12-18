// MIT License

// Copyright (c) 2017 Zebin Xu

//     Permission is hereby granted, free of charge, to any person obtaining a copy
//     of this software and associated documentation files (the "Software"), to deal
//     in the Software without restriction, including without limitation the rights
//     to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//     copies of the Software, and to permit persons to whom the Software is
//     furnished to do so, subject to the following conditions:

//     The above copyright notice and this permission notice shall be included in all
//     copies or substantial portions of the Software.

//     THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//     IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//     FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//     AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//     LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//     OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//     SOFTWARE.
//
// This file is used for preprocessing the obj file and generate "verts.obj" to be used by the 3d convex hull program.
// Compile:
// g++ -o loadobj loadobj.cpp
// Usage
// ./loadobj something.obj
// It will generate an obj file called "verts.obj" which will be fed to the chull program.

#include <iostream>
#include <vector>
#include <stdio.h>
#include <fstream>
#include <cmath>
#include <sstream>
#include <stdlib.h>

// reads the first integer from a string in the form
// "334/455/234" by stripping forward slashes and
// scanning the result
int get_first_integer( const std::string v){
    int ival;
    std::string s( v );
    std::replace( s.begin(), s.end(), '/', ' ' );
    sscanf( s.c_str(), "%d", &ival );
    return ival;
}

// barebones .OFF file reader, throws away texture coordinates, normals, etc.
// stores results in input coords array, packed [x0,y0,z0,x1,y1,z1,...] and
// tris array packed [T0a,T0b,T0c,T1a,T1b,T1c,...]
void load_obj(const char *filename, std::vector<double> &coords, std::vector<int> &tris){
    std::cout << "loading obj" << std::endl;
    //    ifstream file(argv[1]);
    std::ifstream file(filename);
    std::string line;
    
    while (file && getline(file, line)){
        if (line.length() == 0)continue;
        std::vector<std::string> splits;
        std::istringstream iss(line);
        for(std::string s; iss >> s; ) {
            splits.push_back(s);
        }
        //we are dealing with vertices
        if (splits[0] == "v") {
            coords.push_back( atof(splits[1].c_str()) );
            coords.push_back( atof(splits[2].c_str()) );
            coords.push_back( atof(splits[3].c_str()) );
        } else if (splits[0] == "f") { //we are dealing with faces/indices
            int vin0 = atoi(splits[1].c_str())-1;//obj index starts from 1, subtract by 1 so that our indice start from 0
            int vin1 = atoi(splits[2].c_str())-1;
            int vin2 = atoi(splits[3].c_str())-1;
            std::cout << vin0 << "," << vin1 << "," << vin2 << std::endl;
   
            tris.push_back( vin0 );
            tris.push_back( vin1 );
            tris.push_back( vin2 );

            //if the face has 4 vertices, f 1 2 3 4 will converted to 2 faces: f 1 2 3 and f 1 3 4
            if (splits.size() == 5) {
                int vin3 = atoi(splits[4].c_str())-1;
                tris.push_back( vin0 );
                tris.push_back( vin2 );
                tris.push_back( vin3 );
            }
        }
    }
    std::cout << "loading obj done" << std::endl;

}


int main(int argc, char** argv) {
    std::vector<double> coords;
    std::vector<int> tris;
    if (argc == 2) {
        load_obj(argv[1], coords, tris);
    } else {
        std::cout << "Usage: ./a.out model.obj" << std::endl;
        exit(1);
    }
    std::cout << "vertices: " << coords.size()/3 <<std::endl;
    for (int i = 0; i < coords.size(); i += 3) {
        printf("%f, %f, %f\n", coords[i], coords[i+1], coords[i+2]);
    }
    std::cout << "faces: " << tris.size()/3 << std::endl;
    for (int i = 0; i < tris.size(); i += 3) {
        printf("%d, %d, %d\n", tris[i], tris[i+1], tris[i+2]);
    }

    double min = 1.0;//if min is 0 , find second min
    for (int i = 0; i < coords.size(); i += 1) {
        min = (abs(coords[i]) < min && abs(coords[i]) > 1e-2) ? abs(coords[i]) : min;
    }
    printf("min= %f\n", min);
    int amp = 1;

    if (min < 10) {
        while ((int)min == 0) {
            amp *= 10;
            min *= 10;
        }
    }
    printf("need to multiply %d\n", amp);
    // need to amplify the coordinates and convert to integers
    for (int i = 0; i < coords.size(); i += 1) {
        coords[i] *= amp;
        coords[i] = (int)coords[i];
    }
    for (int i = 0; i < coords.size(); i += 3) {
        printf("%f, %f, %f\n", coords[i], coords[i+1], coords[i+2]);
    }
    

    std::ofstream outf("verts.obj");
    for (int i = 0; i < coords.size(); i += 3) {
        outf << "v " << coords[i] << " " << coords[i+1] << " " << coords[i+2] << std::endl;
    }
    for (int i = 0; i < tris.size(); i += 3) {
        outf << "f " << tris[i] << " " << tris[i+1] << " " << tris[i+2] << std::endl;
    }
    outf.close();


    return 0;
}
