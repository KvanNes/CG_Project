#include <iostream>
#include <stdlib.h>
#include <ctime>
#include <list>
#include <GL/glew.h>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <IL/il.h>
#include "Mesh.hpp"
#include "ShaderProgram.hpp"
#include "Texture.hpp"
#include "Camera.h"
#include "BoundingEllipsoid.hpp"

using namespace std;
using glm::vec4;
using glm::vec3;
using glm::mat4;
using std::list;

const char* skyboxVertexShaderName = "skybox.vert";
const char* skyboxFragmentShaderName = "skybox.frag";
const char* phongVertexShaderName = "minimal.vert";
const char* phongFragmentShaderName = "minimal.frag";

ShaderProgram phongShading, skyboxShading;
list<Mesh> meshes;
Mesh sphere;
Mesh skybox;
Texture texture, sphereTexture, skyboxTexture;
mat4 cameraMat = mat4();

int lastTime = glutGet(GLUT_ELAPSED_TIME);
int timediff = 0;
Camera gCamera;
float deltaAngleX = 0.0f;
float deltaAngleY = 0.0f;
int xOrigin = -1;
int yOrigin = -1;

GLuint phongCameraMatrixLocation, phongOrientationMatrixLocation, phongSamplerLocation;
GLuint skyboxOffsetLocation, skyboxSamplerLocation;

BoundingEllipsoid bEllip;
bool drawEllips = true;

Texture textures[2];
double frame;
Texture lazorTexture0 = Texture();
Texture lazorTexture1 = Texture();
Texture lazorTexture2 = Texture();
bool isDescending;

// GLUT callback Handlers
static void resize(int width, int height)
{
    const float ar = (float) width / (float) height;

    gCamera.setPosition(glm::vec3(0.0f,0.0f,24.0f));
    gCamera.setViewportAspectRatio(ar);

    glViewport(0, 0, width, height);
}


void updateCamera(string direction)
{
    timediff = glutGet(GLUT_ELAPSED_TIME) - lastTime;

    //move position of camera based on WASD keys
    const float moveSpeed = 0.1; //units per second

    if(direction.compare("zoomin")==0){
        gCamera.offsetPosition(timediff * moveSpeed * -gCamera.forward());
    } else if(direction.compare("zoomout")==0){
        gCamera.offsetPosition(timediff * moveSpeed * gCamera.forward());
    } else if(direction.compare("left")==0){
        gCamera.offsetPosition(timediff * moveSpeed * -gCamera.right());
    } else if(direction.compare("right")==0){
        gCamera.offsetPosition(timediff * moveSpeed * gCamera.right());
    } else if(direction.compare("up")==0){
        gCamera.offsetPosition(timediff * moveSpeed * -glm::vec3(0,1,0));
    } else if(direction.compare("down")==0){
        gCamera.offsetPosition(timediff * moveSpeed * glm::vec3(0,1,0));
    }
}

void displaySkyBox() {
    glUseProgram(skyboxShading.getName());

    glDisable(GL_DEPTH_TEST);

    glBindTexture( GL_TEXTURE_2D, skyboxTexture.getName() );

    float offset = gCamera.matrix()[3][0]/50.0f;

    glUniform1f(skyboxOffsetLocation, offset);
    glUniform1i(skyboxSamplerLocation, 0/*GL_TEXTURE0*/);

    skybox.draw();

    glEnable(GL_DEPTH_TEST);
}


static void display(void)
{
    //Clear the render buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);

    displaySkyBox();

    //Select which shader program we use
    glUseProgram(phongShading.getName());

    //rotate the model
    lastTime = glutGet(GLUT_ELAPSED_TIME);
    //cameraMat = glm::rotate(cameraMat, timediff*0.01f, vec3(0,1,0));

    // send the camera matrix to the shader
    //glUniformMatrix4fv(matrixLocation, 1, GL_FALSE, glm::value_ptr(cameraMat));
    glUniformMatrix4fv(phongCameraMatrixLocation, 1, GL_FALSE, glm::value_ptr(gCamera.matrix()));

    if(!meshes.back().testCollision(meshes.front())) {
        //move the model
        meshes.front().transform(glm::translate(vec3(0.01f, 0.0f, 0.0f)));
        meshes.back().transform(glm::rotate(0.01f, vec3(0.0f, 1.0f, 0.0f)));
    }
//    meshes.front().transform( glm::translate( vec3(-0.001f,0.0f,0.0f) ) );

//    if(!meshes.back().testCollision(meshes.front())) {
//        //move the model
//        meshes.front().transform(glm::rotate(0.5f, vec3(0.0f, 1.0f, 0.0f)));
////        meshes.back().transform(glm::rotate(0.01f, vec3(0.0f, 1.0f, 0.0f)));
//    }

    //send the texture selection to the shader
    glUniform1i(phongSamplerLocation, 0/*GL_TEXTURE0*/);

    for(std::list<Mesh>::iterator iter = meshes.begin(); iter != meshes.end(); iter++) {
//        glBindTexture(GL_TEXTURE_2D, texture.getName());

        // Load texture at frame
        glBindTexture( GL_TEXTURE_2D, texture.getName() );

        //send the orientation matrix to the shader
        glUniformMatrix4fv(phongOrientationMatrixLocation, 1, GL_FALSE, glm::value_ptr(iter->getOrientation()));

        iter->draw();
        if(drawEllips) {
            glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

            glBindTexture(GL_TEXTURE_2D, sphereTexture.getName());

            mat4 ellipOrientation = iter->getOrientation() * iter->getEllip().orientation;
            //send the orientation matrix to the shader
            glUniformMatrix4fv(phongOrientationMatrixLocation, 1, GL_FALSE, glm::value_ptr(ellipOrientation));
            sphere.draw();

            glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        }
    }

    if(!isDescending) {
        if(frame <= 3) {
            frame += 0.01;
        }else{
            isDescending = true;
        }
    }
    //Check frame
    if (isDescending)
    {
        if(frame >= 0){
            frame -= 0.01;
        }else{
            isDescending = false;
        }
    }

    //swap the renderbuffer with the screenbuffer
    glutSwapBuffers();
}

static void mouseClick(int button, int state, int x, int y)
{
	// only start motion if the left button is pressed
	if (button == GLUT_LEFT_BUTTON) {

		// when the button is released
		if (state == GLUT_UP) {
			xOrigin = -1;
			yOrigin = -1;
		}
		else  {// state = GLUT_DOWN
			xOrigin = x;
			yOrigin = y;
		}
	}
}

static void mouseMove(int x, int y)
{
	// this will only be true when the left button is down
	if (xOrigin >= 0) {

		// update deltaAngle
		deltaAngleX = (x - xOrigin) * 0.0025f;
		deltaAngleY = (y - yOrigin) * 0.0025f;

		// update camera's direction
        gCamera.offsetOrientation(-deltaAngleY, -deltaAngleX);

	}
}

static void key(unsigned char key, int x, int y)
{
    //empty if statement to remove unused argument warning
    if(x == y)
    {
    }
    switch (key)
    {
        case 'q':
            exit(0);
            break;
        case 's':
            updateCamera("zoomin");
            break;
        case 'w':
            updateCamera("zoomout");
            break;
        case 'a':
            updateCamera("left");
            break;
        case 'd':
            updateCamera("right");
            break;
        case 'x':
            updateCamera("down");
            break;
        case 'z':
            updateCamera("up");
            break;
        case 'l':
            drawEllips = !drawEllips;
            break;
    }

    glutPostRedisplay();
}

static void idle(void)
{
    glutPostRedisplay();
}

//Program entry point
int main(int argc, char *argv[])
{
    //Initialize GLUT and create window in which we can draw using OpenGL
    glutInit(&argc, argv);
    glutInitWindowSize(640,480);
    glutInitWindowPosition(10,10);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);

    glutCreateWindow("Flappy Bird... With a gun... In Space...");

    //Initialize GLEW, this should be done after an OpenGL window is created
    GLenum res = glewInit();
    if (res != GLEW_OK)
    {
        cout << "Error: " << glewGetErrorString(res);
        return 1;
    }

    const char* version = (const char*)glGetString(GL_VERSION);
    cout << "OpenGL Version:" << version << endl;
    cout << "Turn drawing collision ellipsoids on/off with the l key." << endl;

    if (!glewIsSupported("GL_VERSION_3_3"))
    {
        cout << "GLEW version out of date: please update your videocard driver" << endl;
        return 1;
    }

    //Check if the version of DevIL in the .dll-file is the same or a newer version then the one the engine is compiled with.
    //If the version is older give an error message and stop the function.
    if(ilGetInteger(IL_VERSION_NUM) < IL_VERSION)
    {
        cout << "DevIL version out of date: please update the dll file." << endl;
        return 1;
    }
    ilInit();

    //Set GLUT callback functions
    glutReshapeFunc(resize);
    glutDisplayFunc(display);
    glutKeyboardFunc(key);
    glutMouseFunc(mouseClick);
    glutMotionFunc(mouseMove);
    glutIdleFunc(idle);

    glClearColor(0,0,0,1);

    //Enable backface culling causing clockwise oriented triangles to not be rendered
    glFrontFace(GL_CCW);
    //glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    //uncomment to draw wireframes
    //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    //Set up shaders
    phongShading.setupShaders(phongVertexShaderName, phongFragmentShaderName);
    //Set uniform variable locations
    phongCameraMatrixLocation = glGetUniformLocation(phongShading.getName(), "camera");
    assert(phongCameraMatrixLocation != 0xFFFFFFFF);
    phongSamplerLocation = glGetUniformLocation(phongShading.getName(), "textureSampler");
    assert(phongSamplerLocation != 0xFFFFFFFF);
    phongOrientationMatrixLocation = glGetUniformLocation(phongShading.getName(), "orientation");
    assert(phongOrientationMatrixLocation != 0xFFFFFFFF);

    skyboxShading.setupShaders(skyboxVertexShaderName, skyboxFragmentShaderName);
    skyboxOffsetLocation = glGetUniformLocation(skyboxShading.getName(), "offset");
    assert(skyboxOffsetLocation != 0xFFFFFFFF);
    skyboxSamplerLocation = glGetUniformLocation(skyboxShading.getName(), "textureSampler");
    assert(skyboxSamplerLocation != 0xFFFFFFFF);

    //seed the random number generator
    srand(time(0));

    Geometry geom1, geom2, geomSphere, geomSkybox;
    //geom1.loadOBJ("models/skybox.obj", true);
    geom1.makeRandomMeteor(15,15,12,0.05f);
    geom2.makeRandomMeteor(15,15,12,0.04f);
    //geom1.makeRandomMeteor(3, 3, 0, 0.08f);
    geomSphere.makeSphere(20, 20);
    geomSkybox.makeQuad();

    //Make a mesh
    sphere.makeMesh(geomSphere);
    skybox.makeMesh(geomSkybox);

    meshes.push_back(Mesh(geom1));
    meshes.push_back(Mesh(geom2));

    //clear geometry memory, because it had been copied to the video card
    geom1.remove();
    geom2.remove();
    geomSphere.remove();
    geomSkybox.remove();

    meshes.front().transform(glm::translate( vec3(-5.0f,0.0f,0.0f) ) );
    //meshes.front().transform(glm::rotate(1.0f, vec3(0.0f, 1.0f, 0.0f)));

    //meshes.back().transform( glm::translate( vec3(5.0f,0.0f,0.0f) ) );

    //cout << glm::to_string(sphere.getOrientation() ) << endl;

    //Load a texture
    texture.load("textures/meteor.jpg");

    textures[0].load("textures/lazor0.jpg");
    textures[1].load("textures/lazor1.jpg");
    textures[2].load("textures/lazor2.jpg");

    //Load a texture
    sphereTexture.load("textures/white.png");
    skyboxTexture.load("textures/skybox.png");

    //Start the GLUT loop
    glutMainLoop();

    return EXIT_SUCCESS;
}
