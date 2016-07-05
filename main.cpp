#include "GL/glew.h"	// should be included at the beginning!
#include <GLFW/glfw3.h>
#include <iostream>
#include <cstdlib>
#include <string>
#include <fstream>
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <vector>
#include "Model.h"

#define GLM_FORCE_RADIANS
#define SAMPLECNT 13
#define PI 3.1415926535

// Pixel should be double on Retina screen
// So you should assign PIXELMULTI 2.0 if your window looks weird
#ifdef __APPLE__
	#define PIXELMULTI 2.0
#else
	#define PIXELMULTI 1.0
#endif

double xpos,ypos;	// Mouse Pos

const GLenum buffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT3};
const GLenum buffers2[] = {GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT4};
const GLenum buffers3[] = {GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT4,GL_COLOR_ATTACHMENT2};
enum shapes {
	square, diamond, parallel, star
};
enum lightMode { bedroomLight , marketLight };

const glm::vec2 shapeAngles[]={glm::vec2(0,PI/2.0f),glm::vec2(PI/4.0f,PI*3.0f/4.0f),glm::vec2(0,PI/4.0f)};
std::vector<glm::vec2> currentShapeAng;
int currentShape;

GLfloat deltaTime;  // To synchronize camera moving speed

// Vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
GLfloat screenVertices[] = {
        // Positions   // TexCoords
        -1.0f,  1.0f,  0.0f, 1.0f,	//left-top
        -1.0f, -1.0f,  0.0f, 0.0f,	//left-bottom
         1.0f, -1.0f,  1.0f, 0.0f,	//right-bottom

        -1.0f,  1.0f,  0.0f, 1.0f,	//left-top
         1.0f, -1.0f,  1.0f, 0.0f,	//right-bottom
         1.0f,  1.0f,  1.0f, 1.0f	//right-top
};

// camera setting ( in m )
float focalLen = 0.025f; //focal length
float aperture = 8.0f; // aperture diameter = f/Dlens (like real camera notation)
float focusDis = 0.2f; // current focus distance

bool blur = false;
float cocView = 0.0f;

GLfloat lastX = 400, lastY = 300;
GLfloat yaw=0.0, pitch=0.0;
//bool firstMouse = true;

bool isCursorRelease = false;
GLfloat mouse_sensitivity = 0.2;
GLfloat camSpeed = 40;
GLfloat Zoom = 100.0f;
bool useHDR = true;

glm::vec3 camPos = glm::vec3(10.0,10.0,10.0);
glm::vec3 camFront = glm::vec3(-10.0,-10.0,-10.0);
glm::vec3 camSide;
char current_coord = 'x';

//*****************
// texColorBuffer : first pass
GLuint frameBuffer, texColorBuffer, texColorBuffer2, texColorBuffer3, texDepthBuffer, texDepthBuffer2;
GLuint screenVAO, screenVBO;

float offsetRadius = 0.5; // make offsets (for polar coordinate)
float sampleKernel[SAMPLECNT*2];
glm::vec2 viewportSize = glm::vec2(800*PIXELMULTI, 600*PIXELMULTI);

glm::vec3 pointLightPositions1[] = {
    glm::vec3(8.2015f, 3.40649f, -4.29478f),
    glm::vec3(-8.61112f, 3.45869f, -4.35868f),
	glm::vec3(14.7265f, 18.1351f, 23.0913f)
};

glm::vec3 pointLightPositions2[] = {
	glm::vec3(21.0562f, 40.1655f, -21.7813f),
	glm::vec3(-20.1564f, 40.5234f, -13.8102f),
	glm::vec3(20.8512f, 38.5168f, 33.0789f),
	glm::vec3(-17.3721f, 40.2367f, 36.3115f),
	glm::vec3(22.3562f, 21.5342f, 15.2914f)
};


GLuint program, screenProgram;

static void setUniformFloat(GLuint program, const std::string &name, const float &value);
static void setupLighting(int LightNo);

void makeSampleOffsets(float angle)
{
	//std::cout << "makeSampleOffsets" << endl;
	float aspectRatio = viewportSize.x / viewportSize.y;

	// convert from polar to cartesian

	glm::vec2 pt = glm::vec2(offsetRadius*cos(angle), offsetRadius*sin(angle));

	// account for aspect ratio(to avoid strenching hightlights)
	pt.x /= aspectRatio;

	// create the interpolations
	float t;
	for (int i=0; i < SAMPLECNT; i++){
		t = i / (SAMPLECNT - 1.0f); // 0 to 1
		//output[i] = (-pt)+t*2*pt;
		glm::vec2 output = glm::vec2((-pt) + t * 2 * pt);
		sampleKernel[i * 2] = output.x;
		sampleKernel[i * 2+1] = output.y;
		//std::cout << i << "(" << output.x << "," << output.y << ")" << std::endl;
	}
}

static void error_callback(int error, const char* description)
{
	fputs(description, stderr);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// Press Esc to exit this program
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	else if (key == GLFW_KEY_1 && action == GLFW_PRESS)
		current_coord = 'x';
	else if (key == GLFW_KEY_2 && action == GLFW_PRESS)
		current_coord = 'y';
	else if (key == GLFW_KEY_3 && action == GLFW_PRESS)
		current_coord = 'z';
	else if (key == GLFW_KEY_W && (action == GLFW_PRESS || action == GLFW_REPEAT))
		camPos += deltaTime * camSpeed * camFront;
	else if (key == GLFW_KEY_S && (action == GLFW_PRESS || action == GLFW_REPEAT))
		camPos -= deltaTime * camSpeed * camFront;
	else if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT))
		camPos += deltaTime * camSpeed * camSide;
	else if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT))
		camPos -= deltaTime * camSpeed * camSide;
	else if (key == GLFW_KEY_M && action == GLFW_PRESS) {
		if (!isCursorRelease) {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			isCursorRelease = true;
		}
		else {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			isCursorRelease = false;
		}
	}
	else if (key == GLFW_KEY_MINUS && action == GLFW_PRESS)
		Zoom += 1.0f;
	else if (key == GLFW_KEY_EQUAL && action == GLFW_PRESS)
		Zoom -= 1.0f;
	else if (key == GLFW_KEY_COMMA && action == GLFW_PRESS)
		focusDis -= 0.05f;
	else if (key == GLFW_KEY_PERIOD && action == GLFW_PRESS)
		focusDis += 0.05f;
	else if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
		focalLen -= 0.005f;
	else if (key == GLFW_KEY_UP && action == GLFW_PRESS)
		focalLen += 0.005f;
	else if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
		aperture -= 1.0f;
	else if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
		aperture += 1.0f;
	else if (key == GLFW_KEY_B && action == GLFW_PRESS)
		blur = !blur;
	else if (key == GLFW_KEY_4 && action == GLFW_PRESS){
		currentShapeAng.clear();
		currentShapeAng.push_back(shapeAngles[square]);
		currentShape = square;
	}
	else if (key == GLFW_KEY_5 && action == GLFW_PRESS){
		currentShapeAng.clear();
		currentShapeAng.push_back(shapeAngles[diamond]);
		currentShape = diamond;
	}
	else if (key == GLFW_KEY_6 && action == GLFW_PRESS){
		currentShapeAng.clear();
		currentShapeAng.push_back(shapeAngles[parallel]);
		currentShape = parallel;
	}
	else if (key == GLFW_KEY_7 && action == GLFW_PRESS){
		currentShapeAng.clear();
		currentShapeAng.push_back(shapeAngles[square]);
		currentShapeAng.push_back(shapeAngles[diamond]);
		currentShape = star;
	}
	else if (key == GLFW_KEY_9 && action == GLFW_PRESS) {
		setupLighting(bedroomLight);
	}
 	else if (key == GLFW_KEY_0 && action == GLFW_PRESS) {
 		setupLighting(marketLight);
 	}
	else if (key == GLFW_KEY_C && action == GLFW_PRESS)
		if(cocView==0.0)
			cocView = 1.0f;
		else
			cocView = 0.0f;
	else if (key == GLFW_KEY_H && action == GLFW_PRESS) {
		if (useHDR) {
			setUniformFloat(program, "useHDR", 0.0);
			useHDR = false;
		}
		else {
			setUniformFloat(program, "useHDR", 1.0);
			useHDR = true;
		}
	}
	//if(focalLen < 0.02)
	//	focalLen = 0.02;
	std::cout << "Aperture Size : " << aperture << std::endl;
	std::cout << "Focal Length : " << focalLen << std::endl;
	std::cout << "Focus Distance : " << focusDis << std::endl;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if(isCursorRelease) {
        lastX = xpos;
        lastY = ypos;
        //firstMouse = false;
    }
	else {
	    GLfloat xoffset = xpos - lastX;
	    GLfloat yoffset = lastY - ypos;
	    lastX = xpos;
	    lastY = ypos;

	    xoffset *= mouse_sensitivity;
	    yoffset *= mouse_sensitivity;

	    yaw   += xoffset;
	    pitch += yoffset;

	    if(pitch > 89.0f)
	        pitch = 89.0f;
	    if(pitch < -89.0f)
	        pitch = -89.0f;

	    glm::vec3 front;
	    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	    front.y = sin(glm::radians(pitch));
	    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		camFront = glm::normalize(front);

		camSide = glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f)));

	}
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        glfwGetCursorPos(window, &xpos, &ypos);
	//TODO: pass mouse position to do post processing

}

static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	switch (current_coord) {
		case 'x': camPos.x += yoffset; break;
		case 'y': camPos.y += yoffset; break;
		case 'z': camPos.z += yoffset; break;
	}
}


// Setup shader program here
static unsigned int setup_shader(const char *vertex_shader, const char *fragment_shader)
{
	GLuint vs=glCreateShader(GL_VERTEX_SHADER);

	// bind the source code to VS object
	glShaderSource(vs, 1, (const GLchar**)&vertex_shader, nullptr);

	// compile the source code of VS object
	glCompileShader(vs);

	/***** To check if compiling is success or not *****/
	int status, maxLength;
	char *infoLog=nullptr;
	glGetShaderiv(vs, GL_COMPILE_STATUS, &status);	// get status
	if(status==GL_FALSE)
	{
		glGetShaderiv(vs, GL_INFO_LOG_LENGTH, &maxLength);	//get info length

		/* The maxLength includes the NULL character */
		infoLog = new char[maxLength];

		glGetShaderInfoLog(vs, maxLength, &maxLength, infoLog);	//get info

		fprintf(stderr, "Vertex Shader Error: %s\n", infoLog);

		/* Handle the error in an appropriate way such as displaying a message or writing to a log file. */
		/* In this simple program, we'll just leave */
		delete [] infoLog;
		return 0;
	}
	/***************************************************/

	//create a shader object and set shader type to run on a programmable fragment processor
	GLuint fs=glCreateShader(GL_FRAGMENT_SHADER);

	// bind the source code to FS object
	glShaderSource(fs, 1, (const GLchar**)&fragment_shader, nullptr);

	// compile the source code of FS object
	glCompileShader(fs);

	glGetShaderiv(fs, GL_COMPILE_STATUS, &status);	// get compile status
	if(status==GL_FALSE)
	{
		glGetShaderiv(fs, GL_INFO_LOG_LENGTH, &maxLength);	//get info length

		/* The maxLength includes the NULL character */
		infoLog = new char[maxLength];

		glGetShaderInfoLog(fs, maxLength, &maxLength, infoLog);	//get info

		fprintf(stderr, "Fragment Shader Error: %s\n", infoLog);

		/* Handle the error in an appropriate way such as displaying a message or writing to a log file. */
		/* In this simple program, we'll just leave */
		delete [] infoLog;
		return 0;
	}

	// Create a program object
	unsigned int program=glCreateProgram();

	// Attach our shaders to our program
	glAttachShader(program, vs);
	glAttachShader(program, fs);

	// Link the shader in the program
	glLinkProgram(program);

	// To check if linking is success or not
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if(status==GL_FALSE)
	{
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

		/* The maxLength includes the NULL character */
		infoLog = new char[maxLength];
		glGetProgramInfoLog(program, maxLength, NULL, infoLog);
		glGetProgramInfoLog(program, maxLength, &maxLength, infoLog);

		fprintf(stderr, "Link Error: %s\n", infoLog);

		/* Handle the error in an appropriate way such as displaying a message or writing to a log file. */
		/* In this simple program, we'll just leave */
		delete [] infoLog;
		return 0;
	}
	// No more need to be used, delete it
	glDeleteShader(vs);
	glDeleteShader(fs);
	return program;
}

static std::string readfile(const char *filename)
{
	std::ifstream ifs(filename);
	if(!ifs)
		exit(EXIT_FAILURE);
	return std::string( (std::istreambuf_iterator<char>(ifs)),
			(std::istreambuf_iterator<char>()));
}

// The key function of HW2, you can change Uniform variable here
static void setUniformMat4(GLuint program, const std::string &name, const glm::mat4 &mat)
{
	glUseProgram(program);
	GLint loc=glGetUniformLocation(program, name.c_str());
	if(loc==-1) return;

	glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(mat));
}

static void setUniformVec3(GLuint program, const std::string &name, const glm::vec3 &vec)
{
	glUseProgram(program);
	GLint loc=glGetUniformLocation(program, name.c_str());
	if(loc==-1) return;

	glUniform3f(loc, vec.x, vec.y, vec.z);
}

static void setUniform2fv(unsigned int program, const std::string &name, const float* fvec, const int count)
{
	// This line can be ignore. But, if you have multiple shader program
	// You must check if currect binding is the one you want
	glUseProgram(program);
	GLint loc = glGetUniformLocation(program, name.c_str());
	if (loc == -1){
		//cout << "glGetUniformLocation error!" << endl;
		return;
	}

	glUniform2fv(loc, count, fvec);
}

static void setUniformFloat(GLuint program, const std::string &name, const float &value)
{
	glUseProgram(program);
	GLint loc=glGetUniformLocation(program, name.c_str());
	if(loc==-1) return;

	glUniform1f(loc, value);
}

GLuint generateAttachmentTexture(GLboolean depth, GLboolean stencil)
{
    // What enum to use?
    GLenum attachment_type;
    if(!depth && !stencil)
        attachment_type = GL_RGBA;
    else if(depth && !stencil)
        attachment_type = GL_DEPTH_COMPONENT;
    else if(!depth && stencil)
        attachment_type = GL_STENCIL_INDEX;

    //Generate texture ID and load texture data
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    if(!depth && !stencil)
		//glTexImage2D(GL_TEXTURE_2D, 0, attachment_type, viewportSize.x, viewportSize.y, 0, attachment_type, GL_UNSIGNED_BYTE, NULL);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, viewportSize.x, viewportSize.y, 0, GL_RGBA, GL_FLOAT, NULL);
    else // Using both a stencil and depth test, needs special format arguments
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, viewportSize.x, viewportSize.y, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glBindTexture(GL_TEXTURE_2D, 0);

    return textureID;
}

// This function will initialize all works before using framebuffer
void frameBuffer_init()
{
	glGenVertexArrays(1, &screenVAO);
	glGenBuffers(1, &screenVBO);
	glBindVertexArray(screenVAO);
	glBindBuffer(GL_ARRAY_BUFFER, screenVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(screenVertices), &screenVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(GLfloat), (GLvoid*)(2*sizeof(GLfloat)));
	glBindVertexArray(0);

	glGenFramebuffers(1, &frameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

	// Generate texture buffer and bind it to framebuffer
    texColorBuffer = generateAttachmentTexture(false, false);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColorBuffer, 0);
    texColorBuffer2 = generateAttachmentTexture(false, false);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, texColorBuffer2, 0);
	texColorBuffer3 = generateAttachmentTexture(false, false);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, texColorBuffer3, 0);
    texDepthBuffer = generateAttachmentTexture(false, false);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, texDepthBuffer, 0);
    texDepthBuffer2 = generateAttachmentTexture(false, false);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, texDepthBuffer2, 0);

	// Generate render buffer and bind it to framebuffer
	GLuint rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 800*PIXELMULTI, 600*PIXELMULTI);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "FrameBuffer is not complete!!!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void setupLighting(int LightNo) {
	glUseProgram(program);
	if (LightNo == bedroomLight) {
		for (int i = 0; i < 3; i++) {
			setUniformVec3(program, "pointLights[" + std::to_string(i) + "].position", pointLightPositions1[i]);
			setUniformVec3(program, "pointLights[" + std::to_string(i) + "].ambient" , glm::vec3(0.4f));
			setUniformVec3(program, "pointLights[" + std::to_string(i) + "].diffuse" , glm::vec3(0.7f));
			setUniformVec3(program, "pointLights[" + std::to_string(i) + "].specular", glm::vec3(1.0f));
			setUniformFloat(program, "pointLights[" + std::to_string(i) + "].constant", 1.0f);
			setUniformFloat(program, "pointLights[" + std::to_string(i) + "].linear", 0.009);
			setUniformFloat(program, "pointLights[" + std::to_string(i) + "].quadratic", 0.0032);
			setUniformVec3(program, "pointLights[" + std::to_string(i) + "].color", glm::vec3(0.8f, 0.75f, 0.73f));
			setUniformFloat(program, "PointLight_Count", 3);
		}
	}
	else if (LightNo == marketLight) {
		for (int i = 0; i < 4; i++) {
			setUniformVec3(program, "pointLights[" + std::to_string(i) + "].position", pointLightPositions2[i]);
			setUniformVec3(program, "pointLights[" + std::to_string(i) + "].ambient" , glm::vec3(0.4f));
			setUniformVec3(program, "pointLights[" + std::to_string(i) + "].diffuse" , glm::vec3(3.5f));
			setUniformVec3(program, "pointLights[" + std::to_string(i) + "].specular", glm::vec3(2.0f));
 			setUniformFloat(program, "pointLights[" + std::to_string(i) + "].constant", 1.0f);
 			setUniformFloat(program, "pointLights[" + std::to_string(i) + "].linear", 0.009);
 			setUniformFloat(program, "pointLights[" + std::to_string(i) + "].quadratic", 0.0032);
 			setUniformVec3(program, "pointLights[" + std::to_string(i) + "].color", glm::vec3(1.0f, 0.9f, 0.875f));
 			setUniformFloat(program, "PointLight_Count", 4);
 		}
 		setUniformVec3(program, "pointLights[4].position", pointLightPositions2[4]);
 		setUniformVec3(program, "pointLights[4].ambient" , glm::vec3(0.4f));
 		setUniformVec3(program, "pointLights[4].diffuse" , glm::vec3(0.2f));
 		setUniformVec3(program, "pointLights[4].specular", glm::vec3(0.6f));
 		setUniformFloat(program, "pointLights[4].constant", 1.0f);
 		setUniformFloat(program, "pointLights[4].linear", 0.009);
 		setUniformFloat(program, "pointLights[4].quadratic", 0.0032);
 		setUniformVec3(program, "pointLights[4].color", glm::vec3(1.0f, 0.9f, 0.875f));
 		setUniformFloat(program, "PointLight_Count", 5);
	}
	setUniformFloat(program, "material.shininess", 32);
}

static void render(Model ourmodel)
{
    /********* 1. Switch to framebuffer, compute CoC and get the depth map*********/
    if(blur){
		glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    	glDrawBuffers(2, buffers); // set the output buffer
	}
	else
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glViewport(0.0, 0.0, 800*PIXELMULTI, 600*PIXELMULTI);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_DST_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    ourmodel.Draw(program);

    if(!blur)
    	return;
	/******************************************************/

	/********* 2. 2nd pass, blur in one direction *********/
	if(currentShape >= star){
		glDrawBuffers(3,buffers3);
	}
	else{
		glDrawBuffers(2,buffers2); 	// set the output buffer
	}

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	//glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);


	// pass the sample offset to shader
	glUseProgram(screenProgram);
    makeSampleOffsets(currentShapeAng[0].x);
    setUniform2fv(screenProgram, "offsetData", sampleKernel, SAMPLECNT);


    // bind the texture vertex
	glBindVertexArray(screenVAO);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texColorBuffer);
    glUniform1i(glGetUniformLocation(screenProgram, "screenTexture"), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texDepthBuffer);
    glUniform1i(glGetUniformLocation(screenProgram, "depthBlurTexture"), 1);

    setUniformFloat(screenProgram,"cocView",cocView);

    // for complex aperture shape
    if(currentShape >=star){
    	makeSampleOffsets(currentShapeAng[1].x);
    	setUniform2fv(screenProgram, "offsetData2", sampleKernel, SAMPLECNT);

    	setUniformFloat(screenProgram,"complex",1.0f); // complex shape 2nd pass

    	glActiveTexture(GL_TEXTURE2);
    	glBindTexture(GL_TEXTURE_2D, texColorBuffer);
    	glUniform1i(glGetUniformLocation(screenProgram, "screenTexture2"), 2);
    }
    else
    	setUniformFloat(screenProgram,"complex",0.0f);

	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

	/*************************************************************************************/

	/********* 3. Switch to default framebuffer and do the final post processing *********/

	//glBindVertexArray(screenVAO);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(1.0f,1.0f,1.0f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    // setup the sample kernal
    makeSampleOffsets(currentShapeAng[0].y);
    glUseProgram(screenProgram);
    setUniform2fv(screenProgram, "offsetData", sampleKernel, SAMPLECNT);

	glBindVertexArray(screenVAO);

    // pass two texture to the post shader
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texColorBuffer2);
    glUniform1i(glGetUniformLocation(screenProgram, "screenTexture"), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texDepthBuffer2);
    glUniform1i(glGetUniformLocation(screenProgram, "depthBlurTexture"), 1);

    if(currentShape >=star){
    	makeSampleOffsets(currentShapeAng[1].y);
    	setUniform2fv(screenProgram, "offsetData2", sampleKernel, SAMPLECNT);

    	setUniformFloat(screenProgram,"complex",2.0f); // complex shape 2nd pass

    	glActiveTexture(GL_TEXTURE2);
    	glBindTexture(GL_TEXTURE_2D, texColorBuffer3);
    	glUniform1i(glGetUniformLocation(screenProgram, "screenTexture2"), 2);
    }

    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
	/**************************************************************/

}

int main(int argc, char *argv[])
{
	GLFWwindow* window;
	glfwSetErrorCallback(error_callback);
	if (!glfwInit())
		exit(EXIT_FAILURE);
	// OpenGL 3.3, Mac OS X is reported to have some problem. However I don't have Mac to test
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	// For Mac OS X
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);	//to make OpenGL forward compatible
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	window = glfwCreateWindow(800, 600, "Bedroom", NULL, NULL);	//create a window object
	if (!window)
	{
		glfwTerminate();
		return EXIT_FAILURE;
	}

	// Make our window context the main context on current thread
	glfwMakeContextCurrent(window);

	// This line MUST put below glfwMakeContextCurrent or it will crash
	// Set GL_TRUE so that glew will use modern techniques to manage OpenGL functionality
	glewExperimental = GL_TRUE;
	glewInit();

	// Enable vsync
	glfwSwapInterval(1);

	glEnable(GL_DEPTH_TEST);

	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);

	// Setup input callback
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetKeyCallback(window, key_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	setUniformFloat(program, "useHDR", 1.0);

	// setup shader program
	program = setup_shader(readfile("vsLight.txt").c_str(), readfile("fsLight.txt").c_str());
    screenProgram = setup_shader(readfile("post_vs.txt").c_str(), readfile("post_fs.txt").c_str());
    glUseProgram(program);

	Model ourmodel(argv[1]);

    // Initialize frameBuffer
    frameBuffer_init();

	// Setup Lighting
	setupLighting(bedroomLight);

	float last, lastSecond, timer;
	last = lastSecond = glfwGetTime();
	GLfloat currentTime;
	int fps=0;

	currentShapeAng.push_back(shapeAngles[square]);
	currentShape = square;

	std::cout << "Aperture Size : " << aperture << std::endl;
	std::cout << "Focal Length : " << focalLen << std::endl;
	std::cout << "Focus Distance : " << focusDis << std::endl;

	glfwSetCursorPos(window, 400.0, 300.0);

	while (!glfwWindowShouldClose(window))
	{ //program will keep drawing here until you close the window
		timer = glfwGetTime() - lastSecond;
		currentTime = glfwGetTime();
		deltaTime = currentTime - last;
		last = currentTime;

		glfwGetCursorPos(window, &xpos, &ypos);
		glfwPollEvents();			// To check if any events are triggered

		/************************************************/
		// Clear the colorbuffer
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		glm::mat4 model;
		setUniformMat4(program, "projection", glm::perspective(glm::radians(Zoom), 800.0f/600, 1.0f, 100.f));
		setUniformMat4(program, "view",
			glm::lookAt(camPos, camPos + camFront, glm::vec3(0.0f, 1.0f, 0.0f)));
		setUniformMat4(program, "model", glm::scale(model, glm::vec3(6.0f, 6.0f, 6.0f)));
		setUniformVec3(program, "viewPos", camPos);


		// pass cameara settings
		setUniformFloat(program,"focalLen",focalLen);
		setUniformFloat(program,"Dlens",focalLen/aperture);
		setUniformFloat(program,"focusDis",focusDis);

		setUniformFloat(program,"mousePosPixX",xpos);
		setUniformFloat(program,"mousePosPixY",ypos);

        render(ourmodel);
		glfwSwapBuffers(window);	// To swap the color buffer in this game loop
		/************************************************/

		fps++;
		if(timer > 1.0)
		{
			std::cout << camPos.x << " " << camPos.y << " " << camPos.z << std::endl;
			//std::cout << (double)fps/timer << std::endl;
			fps = 0;
			lastSecond = glfwGetTime();
		}
	}

	// End of the program
	glfwDestroyWindow(window);
	glfwTerminate();
	return EXIT_SUCCESS;
}
