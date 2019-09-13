#include "MeshRenderer.hpp"
#include "PrintGlm.hpp"
#include "gtc/matrix_transform.hpp"
#include <iostream>
#include "stb_image.h"
#include <cstdlib>
#include <time.h>
#include "glm.hpp"
#include <gtc/random.hpp>
#include "Engine.hpp"
#include <sstream>


MeshRenderer::MeshRenderer(std::shared_ptr<Model> model, 
std::shared_ptr<Shader>  shader, bool useNoise, std::shared_ptr<GameObject> obj) : ARenderer(shader, obj), _model(model), _noise(useNoise)
{
	if (useNoise)
		InitNoiseText();
}
std::string printvec(glm::vec4 & vec)
{
	std::stringstream ss("");
	 ss << "x: " << vec.x << ", y: " << vec.y << ", z: " << vec.z << ", w: " << vec.w;
	 return ss.str();
}
std::string printMat(glm::mat4 mat)
{
	std::stringstream ss("");

	for (int i =0; i < 4; ++i)
	{
		ss << printvec(mat[i]) << std::endl;
	}
	return ss.str();
}
void        MeshRenderer::Draw(void) const
{
	if (_shader == nullptr)
	{
		std::cout << "MeshRenderer : Cannot Draw with whitout shader" << std::endl;
		return;
	}
	else if (_transform == nullptr)
	{ 
		std::cout << "MeshRenderer : Cannot Draw with whitout gameObject" << std::endl;
		return;
	}
	_transform->UpdateMatrix();
    _shader->use();
    _shader->setMat4("view", Camera::instance->GetMatView());
    _shader->setMat4("projection", Camera::instance->GetMatProj());
    _shader->setMat4("model", _transform->GetMatrix());
	_shader->setVec3("uOrigin", _transform->position);
	if (_noise)
	{
		glActiveTexture(GL_TEXTURE10);
		_shader->setInt("uNoise", 10);
		glBindTexture(GL_TEXTURE_2D, _noiseID);
	}
    if (_shader->GetIsRayMarching())
	{
        _shader->SetUpUniforms(*Camera::instance, *SdlWindow::GetMain(), ((float)SDL_GetTicks()) / 1000.f);
		glCullFace(GL_FRONT);
	}
	else
		glCullFace(GL_BACK);
	_model->Draw(_shader);
}

MeshRenderer::MeshRenderer(MeshRenderer const & src) : ARenderer(src._shader, src._gameObj)
{
	_model = src._model;
}

MeshRenderer::~MeshRenderer(void) {}

MeshRenderer &	MeshRenderer::operator=(MeshRenderer const & rhs)
{
    this->_model = rhs._model;
    this->_shader = rhs._shader;
	this->SetGameObject(rhs._gameObj);
    return *this;
}

struct color
{
	unsigned char	r;
	unsigned char	g;
	unsigned char	b;
	unsigned char	a;
};

color *	genNoiseTex()
{
	srand (time(NULL));
	color * tex = (color *)malloc(sizeof(unsigned char) * 256 * 256 * 4);
	for (unsigned int y = 0; y < 256; y++)
	{
		for (unsigned int x = 0; x < 256; x++)
		{
			int idx = y * 256 + x;
			tex[idx].r = rand() % 255;
			tex[idx].b = rand() % 255;
		}
	}
	
	for (unsigned int y = 0; y < 256; y++)
	{
		for (unsigned int x = 0; x < 256; x++)
		{
			int x2 = (x - 37) & 255;
			int y2 = (y - 17) & 255;
			unsigned int idx2 = y2 * 256 + x2;
			unsigned int idx = y * 256 + x;
			tex[idx].g = tex[idx2].r;
			tex[idx].a = tex[idx2].b;
		}
	}
	return tex;
}

void		MeshRenderer::InitNoiseText(void)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	color *data = genNoiseTex();
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	_noiseID = textureID;
	free(data);
}
void MeshRenderer::Destroy(void)
{
	Engine42::Engine::Destroy(std::shared_ptr<MeshRenderer>(this));
}