#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include <string>
#include "Mesh.h"

class Model
{
    public:
        /*  Functions   */
        Model(GLchar* path){ this->loadModel(path); }
        void Draw(GLuint program);

    private:
        /*  Model Data  */
        std::vector<Mesh> meshes;
        std::string directory;
        std::vector<Texture> textures_loaded;

        /*  Functions   */
        void loadModel(std::string path);
        void processNode(aiNode* node, const aiScene* scene);
        Mesh processMesh(aiMesh* mesh, const aiScene* scene);
        std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type,
                                             std::string typeName);
};

#endif
