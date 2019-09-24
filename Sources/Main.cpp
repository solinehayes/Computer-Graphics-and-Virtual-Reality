// ----------------------------------------------
// Base code for practical computer graphics
// assignments.
//
// Copyright (C) 2018 Tamy Boubekeur
// All rights reserved.
// ----------------------------------------------

#define _USE_MATH_DEFINES

#include <glad/glad.h>

#include <cstdlib>
#include <cstdio>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <memory>
#include <algorithm>
#include <exception>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Error.h"
#include "ShaderProgram.h"
#include "Camera.h"
#include "Mesh.h"
#include "MeshLoader.h"
#include "Material.h"
#include "LightSource.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static const std::string SHADER_PATH ("Resources/Shaders/");

static const std::string DEFAULT_MESH_FILENAME ("Resources/Models/face.off");

using namespace std;

// Window parameters
static GLFWwindow * windowPtr = nullptr;

// Pointer to the current camera model
static std::shared_ptr<Camera> cameraPtr;

// Pointer to the displayed mesh
static std::shared_ptr<Mesh> meshPtr;

// Pointer to GPU shader pipeline i.e., set of shaders structured in a GPU program
static std::shared_ptr<ShaderProgram> shaderProgramPtr; // A GPU program contains at least a vertex shader and a fragment shader

// Camera control variables
static float meshScale = 1.0; // To update based on the mesh size, so that navigation runs at scale
static bool isRotating (false);
static bool isPanning (false);
static bool isZooming (false);
static double baseX (0.0), baseY (0.0);
static glm::vec3 baseTrans (0.0);
static glm::vec3 baseRot (0.0);

Material materialAlbedo=Material(1,0.84,1, 0.5);
GLuint albedoTex;
GLuint metallicTex;
GLuint roughnessTex;
GLuint a0Tex;
GLuint toonTex;
GLuint xToonTex;

bool cartoon = 0;
bool xCartoon = 0;


void clear ();
void init (const std::string & meshFilename);

void printHelp () {
	std::cout << "> Help:" << std::endl
			  << "    Mouse commands:" << std::endl
			  << "    * Left button: rotate camera" << std::endl
			  << "    * Middle button: zoom" << std::endl
			  << "    * Right button: pan camera" << std::endl
			  << "    Keyboard commands:" << std::endl
   			  << "    * H: print this help" << std::endl
					<< "    * T: Turn on Toon Mode" << std::endl
					<< "    * X: Turn on XToon Mode" << std::endl
					<< "    * L: Apply Laplacian Filter" << std::endl
					<< "    * S: Simplify" << std::endl
					<< "    * B: Subdivide" << std::endl
   			  << "    * F1: toggle wireframe rendering" << std::endl
					<< "    * F2: Coming back to original mesh" << std::endl
   			  << "    * ESC: quit the program" << std::endl;
}

// Executed each time the window is resized. Adjust the aspect ratio and the rendering viewport to the current window.
void windowSizeCallback (GLFWwindow * windowPtr, int width, int height) {
	cameraPtr->setAspectRatio (static_cast<float>(width) / static_cast<float>(height));
	glViewport (0, 0, (GLint)width, (GLint)height); // Dimension of the rendering region in the window
}

/// Executed each time a key is entered.
void keyCallback (GLFWwindow * windowPtr, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS && key == GLFW_KEY_H) {
		printHelp ();
	}
	else if (action == GLFW_PRESS && key == GLFW_KEY_F1) {
		GLint mode[2];
		glGetIntegerv (GL_POLYGON_MODE, mode);
		glPolygonMode (GL_FRONT_AND_BACK, mode[1] == GL_FILL ? GL_LINE : GL_FILL);
	} else if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE) {
		glfwSetWindowShouldClose (windowPtr, true); // Closes the application if the escape key is pressed
	}
	// Turning on cartoon mode
	else if (action == GLFW_PRESS && key == GLFW_KEY_T){
		xCartoon=0;
		cartoon = !cartoon;
		shaderProgramPtr->use ();
		shaderProgramPtr->set("cartoon", cartoon);
		shaderProgramPtr->stop ();
	}

	//Turning on XToon mode
	else if (action == GLFW_PRESS && key == GLFW_KEY_X){
		cartoon=0;
		xCartoon = !xCartoon;
		shaderProgramPtr->use ();
		shaderProgramPtr->set("xCartoon", xCartoon);
		shaderProgramPtr->stop ();
	}
	else if (action ==GLFW_PRESS && key == GLFW_KEY_L){
		meshPtr->laplacianFilter(1.0, true);
	}
	else if (action ==GLFW_PRESS && key == GLFW_KEY_1){
		meshPtr->laplacianFilter(0.1, true);
	}
	else if (action ==GLFW_PRESS && key == GLFW_KEY_2){
		meshPtr->laplacianFilter(0.5, true);
	}
	else if (action ==GLFW_PRESS && key == GLFW_KEY_3){
		meshPtr->laplacianFilter(1.0, true);
	}
	else if (action ==GLFW_PRESS && key == GLFW_KEY_F2){
		meshPtr->clear();
		MeshLoader::loadOFF (DEFAULT_MESH_FILENAME, meshPtr);
		meshPtr->init();
	}
	else if (action ==GLFW_PRESS && key == GLFW_KEY_S){
		meshPtr->simplify(16);
	}
	else if (action ==GLFW_PRESS && key == GLFW_KEY_D){
		meshPtr->adaptiveSimplify(10);
	}
	else if (action ==GLFW_PRESS && key == GLFW_KEY_B){
		meshPtr->subdivide();
	}
}

/// Called each time the mouse cursor moves
void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
	int width, height;
	glfwGetWindowSize (windowPtr, &width, &height);
	float normalizer = static_cast<float> ((width + height)/2);
	float dx = static_cast<float> ((baseX - xpos) / normalizer);
	float dy = static_cast<float> ((ypos - baseY) / normalizer);
	if (isRotating) {
		glm::vec3 dRot (-dy * M_PI, dx * M_PI, 0.0);
		cameraPtr->setRotation (baseRot + dRot);
	}
	else if (isPanning) {
		cameraPtr->setTranslation (baseTrans + meshScale * glm::vec3 (dx, dy, 0.0));
	} else if (isZooming) {
		cameraPtr->setTranslation (baseTrans + meshScale * glm::vec3 (0.0, 0.0, dy));
	}
}

/// Called each time a mouse button is pressed
void mouseButtonCallback (GLFWwindow * window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    	if (!isRotating) {
    		isRotating = true;
    		glfwGetCursorPos (window, &baseX, &baseY);
    		baseRot = cameraPtr->getRotation ();
        }
    } else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
    	isRotating = false;
    } else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
    	if (!isPanning) {
    		isPanning = true;
    		glfwGetCursorPos (window, &baseX, &baseY);
    		baseTrans = cameraPtr->getTranslation ();
        }
    } else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
    	isPanning = false;
    } else if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS) {
    	if (!isZooming) {
    		isZooming = true;
    		glfwGetCursorPos (window, &baseX, &baseY);
    		baseTrans = cameraPtr->getTranslation ();
        }
    } else if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE) {
    	isZooming = false;
    }
}

void initGLFW () {
	// Initialize GLFW, the library responsible for window management
	if (!glfwInit ()) {
		std::cerr << "ERROR: Failed to init GLFW" << std::endl;
		std::exit (EXIT_FAILURE);
	}

	// Before creating the window, set some option flags
	glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint (GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint (GLFW_RESIZABLE, GL_TRUE);

	// Create the window
	windowPtr = glfwCreateWindow (1024, 768, "Computer Graphics - Practical Assignment", nullptr, nullptr);
	if (!windowPtr) {
		std::cerr << "ERROR: Failed to open window" << std::endl;
		glfwTerminate ();
		std::exit (EXIT_FAILURE);
	}

	// Load the OpenGL context in the GLFW window using GLAD OpenGL wrangler
	glfwMakeContextCurrent (windowPtr);

	/// Connect the callbacks for interactive control
	glfwSetWindowSizeCallback (windowPtr, windowSizeCallback);
	glfwSetKeyCallback (windowPtr, keyCallback);
	glfwSetCursorPosCallback(windowPtr, cursorPosCallback);
	glfwSetMouseButtonCallback (windowPtr, mouseButtonCallback);
}

void exitOnCriticalError (const std::string & message) {
	std::cerr << "> [Critical error]" << message << std::endl;
	std::cerr << "> [Clearing resources]" << std::endl;
	clear ();
	std::cerr << "> [Exit]" << std::endl;
	std::exit (EXIT_FAILURE);
}

void initOpenGL () {
	// Load extensions for modern OpenGL
	if (!gladLoadGLLoader ((GLADloadproc)glfwGetProcAddress))
		exitOnCriticalError ("[Failed to initialize OpenGL context]");

	glEnable (GL_DEBUG_OUTPUT); // Modern error callback functionnality
	glEnable (GL_DEBUG_OUTPUT_SYNCHRONOUS); // For recovering the line where the error occurs, set a debugger breakpoint in DebugMessageCallback
    glDebugMessageCallback (debugMessageCallback, 0); // Specifies the function to call when an error message is generated.
	glCullFace (GL_BACK);     // Specifies the faces to cull (here the ones pointing away from the camera)
	glEnable (GL_CULL_FACE); // Enables face culling (based on the orientation defined by the CW/CCW enumeration).
	glDepthFunc (GL_LESS); // Specify the depth test for the z-buffer
	glEnable (GL_DEPTH_TEST); // Enable the z-buffer test in the rasterization
	glClearColor (0.0f, 0.0f, 0.0f, 1.0f); // specify the background color, used any time the framebuffer is cleared
	// Loads and compile the programmable shader pipeline
	try {
		shaderProgramPtr = ShaderProgram::genBasicShaderProgram (SHADER_PATH + "VertexShader.glsl",
													         	 SHADER_PATH + "FragmentShader.glsl");
	} catch (std::exception & e) {
		exitOnCriticalError (std::string ("[Error loading shader program]") + e.what ());
	}
}

void initScene (const std::string & meshFilename) {
	// Camera
	int width, height;
	glfwGetWindowSize (windowPtr, &width, &height);
	cameraPtr = std::make_shared<Camera> ();
	cameraPtr->setAspectRatio (static_cast<float>(width) / static_cast<float>(height));

	// Mesh
	meshPtr = std::make_shared<Mesh> ();
	try {
		MeshLoader::loadOFF (meshFilename, meshPtr);
	} catch (std::exception & e) {
		exitOnCriticalError (std::string ("[Error loading mesh]") + e.what ());
	}
	meshPtr->init ();

	// Lighting
	LightSource light0= LightSource(0,glm::vec3(3.0, 3.0, 3.0)*20.0f, glm::vec3(1.0, 1.0, 1.0), 5.f);
	LightSource light1= LightSource(1,glm::vec3(-3.0, -3.0, -3.0), glm::vec3(0, 1.0, 0), 100.f);
	LightSource light2= LightSource(2,glm::vec3(-3.0, 0,0), glm::vec3(0, 0, 1.0), 100.f);
	light0.sendToShader(shaderProgramPtr);
	light1.sendToShader(shaderProgramPtr);
	light2.sendToShader(shaderProgramPtr);


	// Material
	albedoTex= materialAlbedo.loadTextureFromFileToGPU("Resources/Materials/Metal/Base_Color.png",0);
	roughnessTex= materialAlbedo.loadTextureFromFileToGPU("Resources/Materials/Metal/Roughness.png",1);
	metallicTex= materialAlbedo.loadTextureFromFileToGPU("Resources/Materials/Metal/Metallic.png",2);
	a0Tex= materialAlbedo.loadTextureFromFileToGPU("Resources/Materials/Metal/Ambient_Occlusion.png",3);
	toonTex= materialAlbedo.loadTextureFromFileToGPU("Resources/Materials/Toon/Toon.png",4);
	xToonTex= materialAlbedo.loadTextureFromFileToGPU("Resources/Materials/Toon/xtoon4.bmp",5);
	materialAlbedo.sendToShader(shaderProgramPtr,albedoTex,roughnessTex, metallicTex, a0Tex,toonTex,xToonTex);


	// Adjust the camera to the actual mesh
	glm::vec3 center;
	meshPtr->computeBoundingSphere (center, meshScale);
	cameraPtr->setTranslation (center + glm::vec3 (0.0, 0.0, 3.0 * meshScale));
	cameraPtr->setNear (meshScale / 100.f);
	cameraPtr->setFar (6.f * meshScale);
}

void init (const std::string & meshFilename) {
	initGLFW (); // Windowing system
	initOpenGL (); // OpenGL Context and shader pipeline
	initScene (meshFilename); // Actual scene to render
}

void clear () {
	cameraPtr.reset ();
	meshPtr.reset ();
	shaderProgramPtr.reset ();
	glfwDestroyWindow (windowPtr);
	glfwTerminate ();
}

// The main rendering call
void render () {
	if(cartoon) {
		glClearColor (1.0f, 1.0f, 1.0f, 1.0f);
	}
	else {
		glClearColor (0.0f, 0.0f, 0.0f, 1.0f);
	}
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Erase the color and z buffers.
	shaderProgramPtr->use (); // Activate the program to be used for upcoming primitive
	materialAlbedo.sendToShader(shaderProgramPtr,albedoTex, roughnessTex, metallicTex, a0Tex,toonTex,xToonTex);
	glm::mat4 projectionMatrix = cameraPtr->computeProjectionMatrix ();
	shaderProgramPtr->set ("projectionMat", projectionMatrix); // Compute the projection matrix of the camera and pass it to the GPU program
	glm::mat4 modelMatrix = meshPtr->computeTransformMatrix ();
	glm::mat4 viewMatrix = cameraPtr->computeViewMatrix ();
	glm::mat4 modelViewMatrix = viewMatrix * modelMatrix;
	glm::mat4 normalMatrix = glm::transpose (glm::inverse (modelViewMatrix));
	shaderProgramPtr->set ("modelViewMat", modelViewMatrix);
	shaderProgramPtr->set ("normalMat", normalMatrix);
	shaderProgramPtr->set("cartoon",cartoon);
	shaderProgramPtr->set("xCartoon", xCartoon);
	meshPtr->render ();
	shaderProgramPtr->stop ();
}

// Update any accessible variable based on the current time
void update (float currentTime) {
	// Animate any entity of the program here
	static const float initialTime = currentTime;
	float dt = currentTime - initialTime;
	// <---- Update here what needs to be animated over time ---->

}

void usage (const char * command) {
	std::cerr << "Usage : " << command << " [<file.off>]" << std::endl;
	std::exit (EXIT_FAILURE);
}

int main (int argc, char ** argv) {
	if (argc > 2)
		usage (argv[0]);
	init (argc == 1 ? DEFAULT_MESH_FILENAME : argv[1]); // Your initialization code (user interface, OpenGL states, scene with geometry, material, lights, etc)
	while (!glfwWindowShouldClose (windowPtr)) {
		update (static_cast<float> (glfwGetTime ()));
		render ();
		glfwSwapBuffers (windowPtr);
		glfwPollEvents ();
	}
	clear ();
	std::cout << " > Quit" << std::endl;
	return EXIT_SUCCESS;
}
