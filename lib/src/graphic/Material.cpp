#include <qc/graphic/Material.hpp>

using namespace qc::graphic;

Material::SharedTexture Material::defaultTex = std::make_shared<Texture>();

Material::Material()
	: shininess(0)
{
	for (auto& it : colors)
		it = glm::vec3(0);

	colors[DIFFUSE_COLOR] = glm::vec3(1);

	for (auto& it : texturePointers)
		it = nullptr;
}
