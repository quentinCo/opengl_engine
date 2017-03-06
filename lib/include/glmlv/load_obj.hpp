#pragma once

#include <glmlv/simple_geometry.hpp>
#include <glmlv/Image2DRGBA.hpp>
#include <glmlv/filesystem.hpp>
#include <glm/vec3.hpp>

namespace glmlv
{
    struct ObjData
    {
        struct PhongMaterial
        {
            glm::vec3 Ka = glm::vec3(0); // Ambient multiplier
            glm::vec3 Kd = glm::vec3(0); // Diffuse multiplier
            glm::vec3 Ks = glm::vec3(0); // Glossy multiplier
			glm::vec3 Ke = glm::vec3(0); // Emissive multiplier
            float shininess = 0.f; // Glossy exponent

            // Indices in the textures vector:
            int32_t KaTextureId = -1;
            int32_t KdTextureId = -1;
            int32_t KsTextureId = -1;
            int32_t shininessTextureId = -1;
        };

        size_t shapeCount = 0;
        size_t materialCount = 0;
        glm::vec3 bboxMin = glm::vec3(std::numeric_limits<float>::max());
        glm::vec3 bboxMax = glm::vec3(std::numeric_limits<float>::lowest());
        std::vector<Vertex3f3f2f> vertexBuffer;
        std::vector<uint32_t> indexBuffer;
        std::vector<uint32_t> indexCountPerShape;

        std::vector<int32_t> materialIDPerShape;

        std::vector<PhongMaterial> materials;
        std::vector<Image2DRGBA> textures;
    };

    void loadObj(const fs::path & objPath, const fs::path & mtlBaseDir, ObjData & data, bool loadTextures = true);

    inline void loadObj(const fs::path & objPath, ObjData & data, bool loadTextures = true)
    {
        return loadObj(objPath, objPath.parent_path(), data, loadTextures);
    }
}