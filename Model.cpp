#include "Mesh.h"
#include "Model.h"
#include <iostream>
#include <SOIL/SOIL.h>

GLint TextureFromFile(const char* path, std::string directory);

void Model::Draw(GLuint program)
{
    for (int i=0; i<this->meshes.size(); i++)
        this->meshes[i].Draw(program);
}

void Model::loadModel(std::string path) {
	Assimp::Importer import;
	const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
    // check if import error
	if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
        return;
	}
    this->directory = path.substr(0, path.find_last_of('/'));

    if (scene->HasMaterials())
        std::cout << "I have materials!" << std::endl;
    if (scene->HasTextures())
        std::cout << "I have textures!" << std::endl;
    if (scene->HasMeshes())
        std::cout << "I have meshes!" << std::endl;
    this->processNode(scene->mRootNode, scene);
}

// In this function we first process Mesh then keep doing it's children
void Model::processNode(aiNode* node, const aiScene* scene) {
    // Process all the node's meshes (if any)
    //std::cout << "Total Meshes: " << node->mNumMeshes << std::endl;
    for(GLuint i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        this->meshes.push_back(this->processMesh(mesh, scene));
    }
    // Then do the same for each of its children (where recursive happened)
    for(GLuint i = 0; i < node->mNumChildren; i++)
    {
        this->processNode(node->mChildren[i], scene);
    }
}

// Transfer Assimp struct to mesh struct
Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene) {
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    std::vector<Texture> textures;
    for (int i=0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
        glm::vec3 vector;
        glm::vec3 normal;

        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.Position = vector;

        if (mesh->HasNormals()) {
            normal.x = mesh->mNormals[i].x;
            normal.y = mesh->mNormals[i].y;
            normal.z = mesh->mNormals[i].z;
            vertex.Normal = normal;
        }


        if (mesh->mTextureCoords[0]) {
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.TexCoords = vec;
        }
        else
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);
        vertices.push_back(vertex);
    }

    for (int i=0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (int j=0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    if (mesh->mMaterialIndex > 0) {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        std::vector<Texture> diffuseMaps = this->loadMaterialTextures(material,
                                        aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        std::vector<Texture> specularMaps = this->loadMaterialTextures(material,
                                        aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    }
    return Mesh(vertices, indices, textures);
}

std::vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName)
{
    std::vector<Texture> textures;

    for (GLuint i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        if (mat->GetTexture(type, i, &str) !=  AI_SUCCESS)
            std::cout << "get texture error!" << std::endl;
        bool shouldSkip = false;
        //std::cout << str.data << std::endl;
        for (int j = 0; j < textures_loaded.size(); j++) {
            if (textures_loaded[j].path == str) {
                textures.push_back(textures_loaded[j]);
                shouldSkip = true;
                break;
            }
        }
        if (!shouldSkip) {
            Texture texture;
            //std::cout << this->directory << std::endl;
            texture.id = TextureFromFile(str.C_Str(), this->directory);
            texture.type = typeName;
            texture.path = str;

            aiColor3D color;
            mat->Get(AI_MATKEY_COLOR_AMBIENT, color);
            texture.mat_ambient = glm::vec3(color.r, color.g, color.b);
            mat->Get(AI_MATKEY_COLOR_DIFFUSE, color);
            texture.mat_diffuse = glm::vec3(color.r, color.g, color.b);
            mat->Get(AI_MATKEY_COLOR_SPECULAR, color);
            texture.mat_specular = glm::vec3(color.r, color.g, color.b);

            textures.push_back(texture);
            this->textures_loaded.push_back(texture);
        }
    }
    return textures;
}

// To read texture from file and transform it to glTexture
GLint TextureFromFile(const char* path, std::string directory)
{
     //Generate texture ID and load texture data
    std::string filename = std::string(path);
    filename = directory + "/" + filename;
    std::cout << filename << std::endl;

    GLuint textureID;
    glGenTextures(1, &textureID);
    int width,height;
    unsigned char* image = SOIL_load_image(filename.c_str(), &width, &height, 0, SOIL_LOAD_RGB);
    // Assign texture to ID
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);

    // Parameters
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    SOIL_free_image_data(image);
    return textureID;
}
