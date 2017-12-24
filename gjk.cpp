#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "base/glsupport.h"
#include "base/quat.h"
#include "core/Transform.hpp"
#include "materials/Material.hpp"
#include "core/Scene.hpp"
#include "core/Camera.hpp"
#include "entities/Model.hpp"
#include "core/Light.hpp"
#include "programs/ColorShader.h"
#include "controls/Trackball.hpp"
#include "geometries/Cube.h"

//GJK collision detection for convex shapes
#include "gjk.h"

#include <stdio.h>

#define DEBUG 1

class Shader;

int screenWidth = 1200;//600;
int screenHeight = 800;//600;

Trackball trackball(screenWidth, screenHeight);
float lineWidth = 0.5f;
bool wireframe = true;
bool solid = false;

//convexhull
bool wireframe_conv = true;
bool solid_conv = false;

// current key/mouse controled model
std::shared_ptr<Entity> controledObj = nullptr;

//collision detection using GJK algorithm
GJK gjk;

Shape shape1;
Shape shape2;

Cvec3 initEye(1.0,1.0,2.0);

using namespace std;

void resetCamera() {
    auto camera = Scene::getCamera();
    camera->setPosition(initEye);
    if (controledObj != nullptr) {
        camera->updateView(controledObj->getPosition());
    }
}

void controls(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_A) {
        lineWidth += 0.5f;
    } else if (key == GLFW_KEY_S) {
        lineWidth -= 0.5f;
    }

    //models' drawing control
    if (key == GLFW_KEY_Z) {
        solid = true;
        wireframe = true;
    } else if (key == GLFW_KEY_X) {
        solid = true;
        wireframe = false;
    } else if (key == GLFW_KEY_C) {
        solid = false;
        wireframe = true;
    }

    //model's convex shape drawing control
    if (key == GLFW_KEY_V) {
        solid_conv = true;
        wireframe_conv = true;
    } else if (key == GLFW_KEY_B) {
        solid_conv = true;
        wireframe_conv = false;
    } else if (key == GLFW_KEY_G) {
        solid_conv = false;
        wireframe_conv = true;
    }
    
    //switch controled object
    if (key == GLFW_KEY_0 && action == GLFW_PRESS) {
        if (controledObj != nullptr) {
            controledObj = controledObj->getName() == "model0" ? Scene::getEntity("model2") : Scene::getEntity("model0");
        }
    }

    //model0 control
    auto model0 = controledObj;
    if (model0 == nullptr) {
        std::cerr << "control object null!" << std::endl;
    }
    if (key == GLFW_KEY_LEFT) {
        model0->translate(Cvec3(-0.1, 0, 0));
    } else if (key == GLFW_KEY_RIGHT) {
        model0->translate(Cvec3(0.1, 0, 0));
    } else if (key == GLFW_KEY_UP) {
        model0->translate(Cvec3(0, 0.1, 0));
    } else if (key == GLFW_KEY_DOWN) {
        model0->translate(Cvec3(0, -0.1, 0));
    } else if (key == GLFW_KEY_COMMA) {//,
        model0->translate(Cvec3(0, 0, -0.1));
    } else if (key == GLFW_KEY_PERIOD) {//.
        model0->translate(Cvec3(0, 0, 0.1));
    } 


    //camera control
    auto camera = Scene::getCamera();
    if (key == GLFW_KEY_COMMA) {//key , x--
        camera->setPosition(camera->getPosition() + Cvec3(-1,0,0));
        camera->updateView(model0->getPosition());
    } else if (key == GLFW_KEY_PERIOD) {//key . y--
        camera->setPosition(camera->getPosition() + Cvec3(0,-1,0));
        camera->updateView(model0->getPosition());
    } else if (key == GLFW_KEY_SLASH) {// key slash x++
        camera->setPosition(camera->getPosition() + Cvec3(1,0,0));
        camera->updateView(model0->getPosition());
    } else if (key == GLFW_KEY_L) {// key L y++
        camera->setPosition(camera->getPosition() + Cvec3(0,1,0));
        camera->updateView(model0->getPosition());
    } else if (key == GLFW_KEY_M) {// key slash z++
        camera->setPosition(camera->getPosition() + Cvec3(0,0,1));
        camera->updateView(model0->getPosition());
    } else if (key == GLFW_KEY_K) {// key L z--
        camera->setPosition(camera->getPosition() + Cvec3(0,0,-1));
        camera->updateView(model0->getPosition());
    } else if (key == GLFW_KEY_P) {// reset
        resetCamera();
    } 

}
bool mouseLeftDown = false;
double mousex = 0.0, mousey = 0.0;
void mouse(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            trackball.record(mousex, mousey);
            mouseLeftDown = true;
        } else if (action == GLFW_RELEASE) {
            mouseLeftDown = false;
        }
    }
}

void curson_pos(GLFWwindow* window, double xpos, double ypos) {
    mousex = xpos;
    mousey = ypos;
    if (mouseLeftDown) {
        auto model = controledObj;
        if (model != NULL) {
            Quat rotation = trackball.getRotation(xpos, ypos);
            model->setRotation(rotation);
#if DEBUG
            std::cout << "position: " << model->getPosition() << std::endl;
            std::cout << "rotation: " << model->getRotation() << std::endl;
#endif
        }
    }
}

void scroll(GLFWwindow* window, double xoffset, double yoffset) {
    auto model = controledObj;
    if (model != nullptr) {
        model->translate(Cvec3(0, 0, yoffset));
    }
}


void extractVertices(const std::vector<Vertex>& vs, std::vector<Cvec3>& outvs) {
    for (int i = 0; i < vs.size(); i++) {
        outvs.push_back(Cvec3(vs[i].position[0], vs[i].position[1], vs[i].position[2]));
    }
}

void display(GLFWwindow* window) {
    while (!glfwWindowShouldClose(window)) {
        glLineWidth(lineWidth);
        // Scale to window size
        GLint windowWidth, windowHeight;
        glfwGetWindowSize(window, &windowWidth, &windowHeight);
        glViewport(0, 0, windowWidth*2, windowHeight*2);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        //collision detection
        auto cvh1 = Scene::getEntity("model1");//convex polygon 1
        auto cvh2 = Scene::getEntity("model3");//convex polygon 2

        auto model0 = Scene::getEntity("model0");//rotate second object
        auto model2 = Scene::getEntity("model2");//rotate second object

        //Update modelmatrix if it's dirty
        if (cvh1->getParent()->transform.matrixDirty()) {
            Matrix4 mm1;
            mm1 = cvh1->getParent()->getModelMatrix();
            shape1.update(mm1);
        }
        if (cvh2->getParent()->transform.matrixDirty()) {
            Matrix4 mm2;
            mm2 = cvh2->getParent()->getModelMatrix();
            shape2.update(mm2);
        }

        //collision detection
        bool collide = gjk.intersect(shape1, shape2);
#if DEBUG
        std::cout << "collide? " << collide << std::endl;
#endif

        std::vector<std::string> names;
        names.push_back("model0");
        names.push_back("model2");
        for (int i = 0; i < 2; ++i) {
            std::string name = names[i];
            auto model = Scene::getEntity(name);

            if (solid) {
                model->material->setColor(1,1,0);
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                glEnable(GL_POLYGON_SMOOTH);
                glEnable(GL_POLYGON_OFFSET_FILL);
                glPolygonOffset(-1, -1);
                checkGlErrors(__FILE__, __LINE__);
                model->draw(Scene::getCamera(), model->getShader(), Scene::getLight(0), Scene::getLight(1));
                checkGlErrors(__FILE__, __LINE__);
                glDisable(GL_POLYGON_OFFSET_FILL);
            }
            
            if (wireframe) {
                model->material->setColor(0, 0, 0);
                checkGlErrors(__FILE__, __LINE__);
                glEnable(GL_LINE_SMOOTH);
                glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
                checkGlErrors(__FILE__, __LINE__);
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                checkGlErrors(__FILE__, __LINE__);
                model->draw(Scene::getCamera(), model->getShader(), Scene::getLight(0), Scene::getLight(1));
                checkGlErrors(__FILE__, __LINE__);
            }
        
            //draw convex hull if it has
            auto model1 = model->getChild();
            if (model1 != nullptr) {
                if (solid_conv) {
                    if (collide) {
                        model1->material->setColor(1, 0, 0);
                    } else {
                        model1->material->setColor(0, 0, 0.9);
                    }

                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                    glEnable(GL_POLYGON_SMOOTH);
                    glEnable(GL_POLYGON_OFFSET_FILL);
                    glPolygonOffset(-1, -1);
                    model1->draw(Scene::getCamera(), model1->getShader(), Scene::getLight(0), Scene::getLight(1));
                    glDisable(GL_POLYGON_OFFSET_FILL);
                }
                if (wireframe_conv) {
                    if (solid_conv) {
                        model1->material->setColor(1.0, 1.0, 1.0);
                    } else {
                        model1->material->setColor(0, 0, 0.9);
                    }
                    if (collide) {
                        model1->material->setColor(1, 0, 0);
                    }
                    glEnable(GL_LINE_SMOOTH);
                    glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                    model1->draw(Scene::getCamera(), model1->getShader(), Scene::getLight(0), Scene::getLight(1));
                }
            }
        }
        checkGlErrors(__FILE__, __LINE__);
        //update screen
        glfwSwapBuffers(window);
        //Check for any input, or window movement
        glfwPollEvents();
    }
}

GLFWwindow* initWindow(const int resX, const int resY) {
    if(!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return NULL;
    }
    glfwWindowHint(GLFW_SAMPLES, 8); // 8x antialiasing
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    // Open a window and create its OpenGL context
    GLFWwindow* window = glfwCreateWindow(resX, resY, "Convex Hull 3D", NULL, NULL);

    if(window == NULL) {
        fprintf(stderr, "Failed to open GLFW window.\n");
        glfwTerminate();
        return NULL;
    }

    glfwMakeContextCurrent(window);

    glewExperimental = GL_FALSE;

    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        printf("glerror: %u!\n", error);
    }
    GLenum glewRes = glewInit();
    if(glewRes != GLEW_OK){
        printf("GLEW was not initialized successfully! %u\n", glewRes);
        exit(EXIT_FAILURE);
    }
    else{
        printf("GLEW was initialized successfully!\n");
    }
    glfwSetKeyCallback(window, controls);
    glfwSetMouseButtonCallback(window, mouse);
    glfwSetCursorPosCallback(window, curson_pos);
    glfwSetScrollCallback(window, scroll);

    // Get info of GPU and supported OpenGL version
    printf("Renderer: %s\n", glGetString(GL_RENDERER));
    printf("OpenGL version supported %s\n", glGetString(GL_VERSION));

    glClearColor(0.3f, 0.3f, 0.5f, 1.0f);
    glClearDepth(0.0);
    glCullFace(GL_BACK);
    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_GREATER);
    glReadBuffer(GL_BACK);


    std::string vertexShader = "shaders/vertex_shader_simple.glsl";
//    std::string vertexShader = "shaders/vertex_shader_model.glsl";
    std::string fragmentShader = "shaders/fragment_shader_color.glsl";
     //    std::string fragmentShader = "shaders/fragment_shader_model.glsl";

    auto colorShader = std::make_shared<ColorShader>();
    colorShader->createProgram(vertexShader.c_str(), fragmentShader.c_str());
//     auto colorShader = std::make_shared<ModelShader>();
//     colorShader->createProgram(vertexShader.c_str(), fragmentShader.c_str());

    auto camera = make_shared<Camera>(initEye, Quat::makeXRotation(0));
    camera->setProjectionMatrix(45.0, float(screenWidth) / screenHeight, -0.01, -200);
    Scene::setCamera(camera);

    auto light0 = std::make_shared<Light>();
    light0->setPosition(0, 0, 5);
    light0->lightColor = Cvec3f(1, 1, 1);

    Scene::setLight0(light0);


    /******* setting up first model and its convex hull *******/
    std::string modelPath0 = "assets/models/face/face.obj";
    std::string convexPath0 = "assets/models/face/convex.obj";

    auto model0 = std::make_shared<Model>(modelPath0, "model0");
    model0->material->setColor(0.8, 0.8, 0.8);
    model0->translate(Cvec3(0, 0, -2));
    model0->setShader(colorShader);
    Scene::addChild(model0);

    camera->updateView(model0->getPosition());
    //set model0 as initial controled object
    controledObj = model0;
    
    // convex hull of model0
    auto model1 = std::make_shared<Model>(convexPath0, "model1", "", false);
    model1->setScale(Cvec3(1.01, 1.01, 1.01));
    model1->material->setColor(0, 0, 0.9);
    model1->setShader(colorShader);
    model1->setParent(model0);
    model0->setChild(model1);
    Scene::addChild(model1);

    //set trackball params
    trackball.setInitRotation(model0->getRotation());



    /******* setting up second model and its convex hull *******/
    std::string modelPath1 = "assets/models/torus/torus.obj";
    std::string convexPath1 = "assets/models/torus/convex.obj";

    auto model2 = std::make_shared<Model>(modelPath1, "model2");
    model2->material->setColor(0.8, 0.8, 0.8);
    model2->translate(Cvec3(-1.5, 0, -4));
    model2->rotate(Quat::makeXRotation(20) * Quat::makeYRotation(20));
    model2->setShader(colorShader);
    Scene::addChild(model2);

    //convex hull of model2
    auto model3 = std::make_shared<Model>(convexPath1, "model3", "", false);//set false we don't want to clear the data in memory as we will use
    model3->material->setColor(0, 0, 0.9);
    model3->setShader(colorShader);
    model3->setParent(model2);
    model2->setChild(model3);
    Scene::addChild(model3);


    // genereate vbo/ibo for the geometry of each Entity.
    Scene::createMeshes();


    /*** extract vertices for GJK ***/
    auto ch1 = Scene::getEntity("model1");//convexhull 1
    auto ch2 = Scene::getEntity("model3");//convexhull 2
    const std::vector<Vertex>& v1 = ch1->geometry->getVertices();
    const std::vector<Vertex>& v2 = ch2->geometry->getVertices();
    
    std::vector<Cvec3> vout1;
    std::vector<Cvec3> vout2;

    extractVertices(v1, vout1);
    extractVertices(v2, vout2);

    shape1.setVertices(vout1);
    shape2.setVertices(vout2);

#if DEBUG
    cout << "v1(" << "size=" << vout1.size() << "): ";
    for (int i = 0; i < vout1.size(); i++) {
        cout << vout1[i] << " ";
    }
    cout << endl;
    cout << "v2(" << "size=" << vout2.size() << "): ";
    for (int i = 0; i < vout1.size(); i++) {
        cout << vout1[i] << " ";
    }
    cout << endl;
#endif

    return window;
}

void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

int main(int argc, char **argv) {
    glfwSetErrorCallback(glfw_error_callback);
    GLFWwindow* window = initWindow(screenWidth, screenHeight);
    if( NULL != window ) {
        display( window );
    }
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
