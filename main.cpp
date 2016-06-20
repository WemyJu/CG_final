#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cstdlib>
#include <string>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include "tiny_obj_loader.h"

#define GLM_FORCE_RADIANS
#define PI 3.1415926535

struct object_struct{
    float ambientStrength;
    unsigned int program;
    unsigned int vao;
    unsigned int vbo[4];
    unsigned int texture;
    glm::vec3 dist;
    glm::mat4 self;
    glm::mat4 dself;
    glm::mat4 model, vp;
    object_struct(): model(glm::mat4(1.0f)){}
} ;
std::vector<object_struct> objects;//vertex array object,vertex buffer object and texture(color) for objs
unsigned int program;
unsigned int post_program;
unsigned int screenShader;
double posX, posY, scalar(1.0), sigma(0.01), radius(0.1);
int mode = 1;
std::vector<int> indicesCount;//Number of indice of objs
float gaussion_filter[9];
GLfloat post_vertices[] = {  // Post Vertex Array
    // position   // texcoord
    -1.0f,  1.0f,  0.0f, 1.0f,
    -1.0f, -1.0f,  0.0f, 0.0f,
     1.0f, -1.0f,  1.0f, 0.0f,

    -1.0f,  1.0f,  0.0f, 1.0f,
     1.0f, -1.0f,  1.0f, 0.0f,
     1.0f,  1.0f,  1.0f, 1.0f
};

static std::string readfile(const char*);
static unsigned int setup_shader(const char*, const char*);

static void error_callback(int error, const char* description)
{
    fputs(description, stderr);
}

void countGaussionFillter()
{
    float sum = 0;
    for(int i=0; i<3; i++)
        for(int j=0; j<3; j++)
        {
            int x = i-1;
            int y = j-1;
            float exp_index = exp(-1*(x*x+y*y)/(2*sigma*sigma));
            gaussion_filter[3*i+j] = (1/(2*PI*sigma*sigma))*exp_index;
            sum += gaussion_filter[3*i+j];
        }
    for(int i=0; i<3; i++)
        for(int j=0; j<3; j++)
            gaussion_filter[3*i+j]/=sum;
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    else if (key == GLFW_KEY_UP && action == GLFW_PRESS)
    {
        if(mode!=2) scalar += 0.1;
        else{ 
            sigma += 0.25;
            countGaussionFillter();
        }
    }
    else if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
    {
        if(mode!=2)
            scalar = (scalar>0.1) ? scalar-0.1 : 0;
        else if(sigma>0.26){ 
            sigma -= 0.25;
            countGaussionFillter();
        }
        else sigma = 0.01;
    }
    else if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
    {
        if(mode!=1){ 
            sigma += 0.25;
            countGaussionFillter();
        }
        else scalar += 0.1;
    }
    else if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
    {
        if(mode!=1){
            sigma = (sigma>0.26) ? sigma-0.25 : 0.01;
            countGaussionFillter();
        }
        else
            scalar = (scalar>0.1) ? scalar-0.1 : 0;
    }
    else if (key == '/' && mode == 3 && action == GLFW_PRESS)
        radius+=0.02;
    else if (key == '.' && mode == 3 && action == GLFW_PRESS)
        radius = (radius > 0.02 ) ? radius-0.02 : 0;
    else if ((key == 'p' || key == 'P') && action == GLFW_PRESS)
        program = setup_shader(readfile("phong_vs.txt").c_str(), readfile("phong_fs.txt").c_str());
    else if ((key == 'f' || key == 'F') && action == GLFW_PRESS)
        program = setup_shader(readfile("flat_vs.txt").c_str(), readfile("flat_fs.txt").c_str());
    else if ((key == 'b' || key == 'B') && action == GLFW_PRESS)
        program = setup_shader(readfile("blinn_phong_vs.txt").c_str(), readfile("blinn_phong_fs.txt").c_str());
    else if ((key == 'g' || key == 'G') && action == GLFW_PRESS)
        program = setup_shader(readfile("gouraud_vs.txt").c_str(), readfile("gouraud_fs.txt").c_str());
    else if (key == '1' && action == GLFW_PRESS)
        mode = 1;
    else if (key == '2' && action == GLFW_PRESS)
        mode = 2;
    else if (key == '3' && action == GLFW_PRESS)
        mode = 3;
}

static void cursor_pos_callback(GLFWwindow* window, double posx, double posy)
{
    posX = posx/800;
    posY = 1-posy/600;
}

static unsigned int setup_shader(const char *vertex_shader, const char *fragment_shader)
{
    GLuint vs=glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, (const GLchar**)&vertex_shader, nullptr);

    glCompileShader(vs);

    int status, maxLength;
    char *infoLog=nullptr;
    glGetShaderiv(vs, GL_COMPILE_STATUS, &status);
    if(status==GL_FALSE)
    {
        glGetShaderiv(vs, GL_INFO_LOG_LENGTH, &maxLength);

        /* The maxLength includes the NULL character */
        infoLog = new char[maxLength];

        glGetShaderInfoLog(vs, maxLength, &maxLength, infoLog);

        fprintf(stderr, "Vertex Shader Error: %s\n", infoLog);

        /* Handle the error in an appropriate way such as displaying a message or writing to a log file. */
        /* In this simple program, we'll just leave */
        delete [] infoLog;
        return 0;
    }

    GLuint fs=glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, (const GLchar**)&fragment_shader, nullptr);
    glCompileShader(fs);

    glGetShaderiv(fs, GL_COMPILE_STATUS, &status);
    if(status==GL_FALSE)
    {
        glGetShaderiv(fs, GL_INFO_LOG_LENGTH, &maxLength);

        /* The maxLength includes the NULL character */
        infoLog = new char[maxLength];

        glGetShaderInfoLog(fs, maxLength, &maxLength, infoLog);

        fprintf(stderr, "Fragment Shader Error: %s\n", infoLog);

        /* Handle the error in an appropriate way such as displaying a message or writing to a log file. */
        /* In this simple program, we'll just leave */
        delete [] infoLog;
        return 0;
    }

    unsigned int program=glCreateProgram();
    // Attach our shaders to our program
    glAttachShader(program, vs);
    glAttachShader(program, fs);

    glLinkProgram(program);

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

// mini bmp loader written by HSU YOU-LUN
static unsigned char *load_bmp(const char *bmp, unsigned int *width, unsigned int *height, unsigned short int *bits)
{
    unsigned char *result=nullptr;
    FILE *fp = fopen(bmp, "rb");
    if(!fp)
        return nullptr;
    char type[2];
    unsigned int size, offset;
    // check for magic signature	
    fread(type, sizeof(type), 1, fp);
    if(type[0]==0x42 || type[1]==0x4d){
        fread(&size, sizeof(size), 1, fp);
        // ignore 2 two-byte reversed fields
        fseek(fp, 4, SEEK_CUR);
        fread(&offset, sizeof(offset), 1, fp);
        // ignore size of bmpinfoheader field
        fseek(fp, 4, SEEK_CUR);
        fread(width, sizeof(*width), 1, fp);
        fread(height, sizeof(*height), 1, fp);
        // ignore planes field
        fseek(fp, 2, SEEK_CUR);
        fread(bits, sizeof(*bits), 1, fp);
        unsigned char *pos = result = new unsigned char[size-offset];
        fseek(fp, offset, SEEK_SET);
        while(size-ftell(fp)>0)
            pos+=fread(pos, 1, size-ftell(fp), fp);
    }
    fclose(fp);
    return result;
}

static int add_obj(unsigned int program, const char *filename,const char *texbmp)
{
    object_struct new_node;

    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::string err = tinyobj::LoadObj(shapes, materials, filename);

    if (!err.empty()||shapes.size()==0)
    {
        std::cerr<<err<<std::endl;
        exit(1);
    }

    glGenVertexArrays(1, &new_node.vao);
    glGenBuffers(4, new_node.vbo);	// (num of vbo we want, the begin of vertex)
    glGenTextures(1, &new_node.texture);

    glBindVertexArray(new_node.vao);

    // Upload postion array
    glBindBuffer(GL_ARRAY_BUFFER, new_node.vbo[0]);     // bind vbo to local array
    // copy the vertex data into buffer 
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*shapes[0].mesh.positions.size(), shapes[0].mesh.positions.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    if(shapes[0].mesh.texcoords.size()>0)
    {

        // Upload texCoord array
        glBindBuffer(GL_ARRAY_BUFFER, new_node.vbo[1]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*shapes[0].mesh.texcoords.size(),
                shapes[0].mesh.texcoords.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

        glBindTexture( GL_TEXTURE_2D, new_node.texture);
        unsigned int width, height;
        unsigned short int bits;
        unsigned char *bgr=load_bmp(texbmp, &width, &height, &bits);
        GLenum format = (bits == 24? GL_BGR: GL_BGRA);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, format, GL_UNSIGNED_BYTE, bgr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glGenerateMipmap(GL_TEXTURE_2D);
        delete [] bgr;
    }

    if(shapes[0].mesh.normals.size()>0)
    {
        // Upload normal array
        glBindBuffer(GL_ARRAY_BUFFER, new_node.vbo[2]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*shapes[0].mesh.normals.size(),
                shapes[0].mesh.normals.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
    }

    // Setup index buffer for glDrawElements
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, new_node.vbo[3]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*shapes[0].mesh.indices.size(),
            shapes[0].mesh.indices.data(), GL_STATIC_DRAW);

    indicesCount.push_back(shapes[0].mesh.indices.size());

    glBindVertexArray(0);

    new_node.program = program;

    objects.push_back(new_node);
    return objects.size()-1;
}

static void releaseObjects()
{
    for(int i=0;i<objects.size();i++){
        glDeleteVertexArrays(1, &objects[i].vao);
        glDeleteTextures(1, &objects[i].texture);
        glDeleteBuffers(4, objects[i].vbo);
    }
    // delete the program
    glDeleteProgram(program);
    glDeleteProgram(post_program);
}

static void setUniformMat4(unsigned int program, const std::string &name, const glm::mat4 &mat)
{
    // This line can be ignore. But, if you have multiple shader program
    // You must check if currect binding is the one you want
    glUseProgram(program);
    GLint loc=glGetUniformLocation(program, name.c_str());
    if(loc==-1) return;

    // mat4 of glm is column major, same as opengl
    // we don't need to transpose it. so..GL_FALSE
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(mat));
}

static void setUniformVec3(unsigned int program, const std::string &name, const glm::vec3 &vec)
{
    // This line can be ignore. But, if you have multiple shader program
    // You must check if currect binding is the one you want
    glUseProgram(program);
    GLint loc=glGetUniformLocation(program, name.c_str());
    if(loc==-1) return;

    glUniform3f(loc, vec.x, vec.y, vec.z);
}

static void setUniformFloat(unsigned int program, const std::string &name, const float &f)
{
    // This line can be ignore. But, if you have multiple shader program
    // You must check if currect binding is the one you want
    glUseProgram(program);
    GLint loc=glGetUniformLocation(program, name.c_str());
    if(loc==-1) return;

    glUniform1f(loc, f);
}

static void setUniformFloatVector(unsigned int program, const std::string &name, const float* fvec, const int num)
{
    // This line can be ignore. But, if you have multiple shader program
    // You must check if currect binding is the one you want
    glUseProgram(program);
    GLint loc=glGetUniformLocation(program, name.c_str());
    if(loc==-1) return;

    glUniform1fv(loc, num, fvec);
}

static void setUniformInt(unsigned int program, const std::string &name, const int &i)
{
    // This line can be ignore. But, if you have multiple shader program
    // You must check if currect binding is the one you want
    glUseProgram(program);
    GLint loc=glGetUniformLocation(program, name.c_str());
    if(loc==-1) return;

    glUniform1i(loc, i);
}

static void render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    for(int i=objects.size()-1; i>=0; i--){
        glUseProgram(objects[i].program);
        glBindVertexArray(objects[i].vao);
        glBindTexture(GL_TEXTURE_2D, objects[i].texture);
        //you should send some data to shader here
        setUniformFloat(objects[i].program, "ambientStrength", objects[i].ambientStrength);
        setUniformMat4(objects[i].program, "model", objects[i].model);
        setUniformMat4(objects[i].program, "vp", objects[i].vp);
        setUniformVec3(objects[i].program, "lightPos", glm::vec3(0.0f));
        setUniformVec3(objects[i].program, "viewPos", glm::vec3(20.0f));
        setUniformVec3(objects[i].program, "lightColor", glm::vec3(1.0f));
        glDrawElements(GL_TRIANGLES, indicesCount[i], GL_UNSIGNED_INT, nullptr);
    }
    glBindTexture(GL_TEXTURE_2D,0);
    glBindVertexArray(0);
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
        glTexImage2D(GL_TEXTURE_2D, 0, attachment_type, 800, 600, 0, attachment_type, GL_UNSIGNED_BYTE, NULL);
    else // Using both a stencil and depth test, needs special format arguments
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, 800, 600, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glBindTexture(GL_TEXTURE_2D, 0);

    return textureID;
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
    // For Mac OS X
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(800, 600, "Simple Example", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);

    // This line MUST put below glfwMakeContextCurrent
    // GLEW is used to manage the pointer of OpenGL function
    glewExperimental = GL_TRUE;     // support core-profile
    glewInit();

    // Enable vsync
    glfwSwapInterval(1);

    // Setup input callback
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);

    // load shader program
    program = setup_shader(readfile("vs.txt").c_str(), readfile("fs.txt").c_str());
    post_program = setup_shader(readfile("post_vs.txt").c_str(), readfile("post_fs.txt").c_str());

    int sun = add_obj(program, "sun.obj","sun.bmp");
    int earth = add_obj(program, "earth.obj","earth.bmp");

    glCullFace(GL_BACK);
    // perspective(field_of_view_in_the_y_dimention, winX/winY, zNear, zFar)
    // lookat(position_of_eye, position_of_center, position_of_up)
    glm::mat4 ori = glm::perspective(glm::radians(45.0f), 800.0f/600, 1.0f, 100.f)*glm::lookAt(glm::vec3(20.0f), glm::vec3(), glm::vec3(0, 1, 0))*glm::mat4(1.0f);
    for(int i=0;i<objects.size();i++){
        objects[i].vp = ori;
    }
    objects[sun].ambientStrength = 1.0f;
    objects[earth].ambientStrength = 0.1f;

    // Setup screen VAO
    GLuint post_VAO, post_VBO;
    glGenVertexArrays(1, &post_VAO);
    glGenBuffers(1, &post_VBO);
    glBindVertexArray(post_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, post_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(post_vertices), &post_vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
    glBindVertexArray(0);

    // Setup Frame Buffer
    GLuint framebuffer;
    glGenFramebuffers(1,&framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    // Generate texture
    GLuint textureColorbuffer = generateAttachmentTexture(false, false);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);
    GLuint textureDepthbuffer = generateAttachmentTexture(false, false);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, textureDepthbuffer, 0);
    // rbo
    GLuint rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 800, 600);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_RENDERBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0); 

    const GLenum buffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, buffers);

    countGaussionFillter();

    float last, start;
    // Perihelion = 1.47*10^8 km, Aphelion = 1.52*10^8
    // => a = 1.495*10^8, f = 0.025*10^8, b=1.494790955*10^8
    float a = 14.95, b = 14.94, f = 0.25;
    last = start = glfwGetTime();
    int fps=0;
    while (!glfwWindowShouldClose(window))
    {//program will keep draw here until you close the window
        float count = (float)glfwGetTime();
#define FRAMED
#ifdef FRAMED
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
#endif
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glEnable(GL_DEPTH_TEST);

        objects[sun].model = glm::mat4(1.0f);
        objects[sun].model = rotate(objects[sun].model, (float)2.8*count*glm::radians(12.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        objects[sun].model = scale(objects[sun].model, glm::vec3(1.0f));
        objects[earth].model = glm::mat4(1.0f);
        objects[earth].model = translate(objects[earth].model, glm::vec3(a*cos(count/10)-f, 0.0f, -b*sin(count/10)));
        //objects[earth].model = rotate(objects[earth].model, (float)2.8*count*glm::radians(365.0f), glm::vec3(0.39875f, 0.91706f, 0.0f));
        objects[earth].model = glm::scale(objects[earth].model, glm::vec3(5.0f));

        render();

#ifdef FRAMED
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(1.0f,1.0f,1.0f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);

        glUseProgram(post_program);
        setUniformInt(post_program, "mode", mode);
        setUniformFloat(post_program, "scalar", scalar);
        setUniformFloat(post_program, "sigma", sigma);
        setUniformFloat(post_program, "radius", radius);
        setUniformFloat(post_program, "posX", posX);
        setUniformFloat(post_program, "posY", posY);
        setUniformFloatVector(post_program, "kernel", gaussion_filter, 9);

        glBindVertexArray(post_VAO);
        glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
        glBindTexture(GL_TEXTURE_2D, textureDepthbuffer);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
#endif
        glfwSwapBuffers(window);
        glfwPollEvents();
        fps++;
        if(glfwGetTime() - last > 1.0)
        {
            std::cout<<count<<std::endl;
            fps = 0;
            last = glfwGetTime();
        }
    }

    // clean the framebuffer
    glDeleteFramebuffers(1, &framebuffer);
    releaseObjects();
    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_SUCCESS;
}
