#include <glmlv/load_obj.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <algorithm>

namespace glmlv
{

struct TinyObjLoaderIndexHash
{
    size_t operator()(const tinyobj::index_t & idx) const
    {
        const size_t h1 = std::hash<int>()(idx.vertex_index);
        const size_t h2 = std::hash<int>()(idx.normal_index);
        const size_t h3 = std::hash<int>()(idx.texcoord_index);

        size_t seed = h1;
        seed ^= h2 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= h3 + 0x9e3779b9 + (seed << 6) + (seed >> 2);

        return seed;
    }
};

struct TinyObjLoaderEqualTo
{
    bool operator ()(const tinyobj::index_t & lhs, const tinyobj::index_t & rhs) const
    {
        return lhs.vertex_index == rhs.vertex_index && lhs.normal_index == rhs.normal_index && lhs.texcoord_index == rhs.texcoord_index;
    }
};

// Load an obj model with tinyobjloader
// Obj models might use different set of indices per vertex. The default rendering mechanism of OpenGL does not support this feature to this functions duplicate attributes with different indices.
void loadObj(const fs::path & objPath, const fs::path & mtlBaseDir, ObjData & data, bool loadTextures)
{
    // Load obj
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    tinyobj::attrib_t attribs;

    std::string err;
    bool ret = tinyobj::LoadObj(&attribs, &shapes, &materials, &err, objPath.string().c_str(), (mtlBaseDir.string() + "/").c_str());

    if (!err.empty()) { // `err` may contain warning message.
        std::cerr << err << std::endl;
    }

    if (!ret) {
        throw std::runtime_error(err);
    }

    data.shapeCount += shapes.size();
    data.materialCount += materials.size();

    std::unordered_map<tinyobj::index_t, uint32_t, TinyObjLoaderIndexHash, TinyObjLoaderEqualTo> indexMap;

    std::unordered_set<std::string> texturePaths;

    const auto materialIdOffset = data.materials.size();
    for (const auto & shape : shapes)
    {
        const auto & mesh = shape.mesh;
        for (const auto & idx : mesh.indices)
        {
            const auto it = indexMap.find(idx);
            if (it == end(indexMap))
            {
                // Put the vertex in the vertex buffer and record a new index if not found
                float vx = attribs.vertices[3 * idx.vertex_index + 0];
                float vy = attribs.vertices[3 * idx.vertex_index + 1];
                float vz = attribs.vertices[3 * idx.vertex_index + 2];
                float nx = attribs.normals[3 * idx.normal_index + 0];
                float ny = attribs.normals[3 * idx.normal_index + 1];
                float nz = attribs.normals[3 * idx.normal_index + 2];
                float tx = attribs.texcoords[2 * idx.texcoord_index + 0];
                float ty = attribs.texcoords[2 * idx.texcoord_index + 1];

                uint32_t newIndex = data.vertexBuffer.size();
                data.vertexBuffer.emplace_back(glm::vec3(vx, vy, vz), glm::vec3(nx, ny, nz), glm::vec2(tx, ty));
                data.bboxMin = glm::min(data.bboxMin, data.vertexBuffer.back().position);
                data.bboxMax = glm::max(data.bboxMax, data.vertexBuffer.back().position);

                indexMap[idx] = newIndex;
                data.indexBuffer.emplace_back(newIndex);
            }
            else
                data.indexBuffer.emplace_back((*it).second);
        }
        data.indexCountPerShape.emplace_back(mesh.indices.size());

        const int32_t localMaterialID = mesh.material_ids.empty() ? -1 : mesh.material_ids[0];
        const int32_t materialID = localMaterialID >= 0 ? materialIdOffset + localMaterialID : -1;

        data.materialIDPerShape.emplace_back(materialID);

        // Only load textures that are used
        if (localMaterialID >= 0)
        {
            const auto & material = materials[localMaterialID];
            texturePaths.emplace(material.ambient_texname);
            texturePaths.emplace(material.diffuse_texname);
            texturePaths.emplace(material.specular_texname);
            texturePaths.emplace(material.specular_highlight_texname);
        }
    }

    std::unordered_map<std::string, int32_t> textureIdMap;

    if (loadTextures)
    {
        const auto textureIdOffset = data.textures.size();
        for (const auto & texturePath : texturePaths)
        {
            if (!texturePath.empty())
            {
                auto newTexturePath = texturePath;
                std::replace(begin(newTexturePath), end(newTexturePath), '\\', '/');
                const auto completePath = mtlBaseDir / newTexturePath;
                if (fs::exists(completePath))
                {
                    std::clog << "Loading image " << completePath << std::endl;
                    data.textures.emplace_back(readImage(completePath));
                    data.textures.back().flipY();

                    const auto localTexId = textureIdMap.size();
                    textureIdMap[texturePath] = textureIdOffset + localTexId;
                }
                else
                {
                    std::clog << "'Warning: image " << completePath << " not found" << std::endl;
                }
            }
        }
    }

    for (const auto & material : materials)
    {
        data.materials.emplace_back(); // Add new material
        auto & newMaterial = data.materials.back();

        newMaterial.Ka = glm::vec3(material.ambient[0], material.ambient[1], material.ambient[2]);
        newMaterial.Kd = glm::vec3(material.diffuse[0], material.diffuse[1], material.diffuse[2]);
        newMaterial.Ks = glm::vec3(material.specular[0], material.specular[1], material.specular[2]);
		newMaterial.Ke = glm::vec3(material.emission[0], material.emission[1], material.emission[2]);
        newMaterial.shininess = material.shininess;

        if (!material.ambient_texname.empty()) {
            const auto it = textureIdMap.find(material.ambient_texname);
            newMaterial.KaTextureId = it != end(textureIdMap) ? (*it).second : -1;
        }
        if (!material.diffuse_texname.empty()) {
            const auto it = textureIdMap.find(material.diffuse_texname);
            newMaterial.KdTextureId = it != end(textureIdMap) ? (*it).second : -1;
        }
        if (!material.specular_texname.empty()) {
            const auto it = textureIdMap.find(material.specular_texname);
            newMaterial.KsTextureId = it != end(textureIdMap) ? (*it).second : -1;
        }
        if (!material.specular_highlight_texname.empty()) {
            const auto it = textureIdMap.find(material.specular_highlight_texname);
            newMaterial.shininessTextureId = it != end(textureIdMap) ? (*it).second : -1;
        }
    }
}

}
