#ifndef MESH_H
#define MESH_H

#include <GL/glew.h>  // should be included at the beginning!
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>
#include <vector>

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

struct Texture {
    GLuint id;
    std::string type;
    aiString path;
    glm::vec3 mat_ambient;
    glm::vec3 mat_diffuse;
    glm::vec3 mat_specular;

    float tr;
};

class Mesh {
    public:
        /*  Mesh Data  */
        std::vector<Vertex> vertices;
        std::vector<GLuint> indices;
        std::vector<Texture> textures;
        /*  Functions  */
        Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, std::vector<Texture> textures);
        void Draw(GLuint program);

    private:
        /*  Render data  */
        GLuint VAO, VBO, EBO;
        /*  Functions    */
        void setupMesh();
};

#endif
