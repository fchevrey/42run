/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Model.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jloro <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/06/20 12:44:53 by jloro             #+#    #+#             */
/*   Updated: 2019/09/25 17:15:30 by jloro            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Model.hpp"
#include <iostream>
#include <assimp/postprocess.h>
#include <cmath>
# ifndef STB_IMAGE_IMPLEMENTATION
#  define STB_IMAGE_IMPLEMENTATION
# endif
#include <cstdlib>
#include "PrintGlm.hpp"
#include "Engine.hpp"
#include "GameManager.hpp"

Model::Model(void)
{
}

Model::Model(const char* path) : _hasAnim(false)
{
	_LoadModel(path);
	for (unsigned int i = 0; i < _meshes.size(); i++)
		_meshes[i].SendToOpenGL();
}
Model::Model(const Model &src)
{
	*this = src;
	for (unsigned int i = 0; i < _meshes.size(); i++)
		_meshes[i].SendToOpenGL();
}

Model::~Model() {}

void	Model::AddAnimation(const char* path)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, 0);
	if (!scene || !scene->HasAnimations())
		throw std::runtime_error(std::string("No animations"));

	_animations.push_back(std::shared_ptr<Animation>(new Animation(scene->mAnimations[0])));
}

void	Model::Draw(const std::shared_ptr<Shader>  shader)
{
	if (_hasAnim &&_playing)
	{
		_BoneTransform(_chrono, shader);
		_chrono += Engine42::Time::GetDeltaTime();
		if (_currentAnimation == 1 && _chrono >= _animations[_currentAnimation]->duration / _animations[_currentAnimation]->ticksPerSecond)
			GameManager::instance->Reset();
	}
	for (unsigned int i = 0; i < _meshes.size(); i++)
		_meshes[i].Draw(shader);
}
void	Model::PauseAnimation()
{
	_playing = false;
}
void	Model::PlayAnimation()
{
	_playing = true;

}
void	Model::ChangeAnimation(unsigned int anim)
{
	_chrono = 0;
	_currentAnimation = anim;
}
Model & Model::operator=(const Model &rhs)
{
	this->_dir = rhs._dir;
	this->_meshes = rhs._meshes;
	return *this;
}

glm::mat4	aiMat4ToGlmMat4(aiMatrix4x4 from)
{
	glm::mat4	to;

	to[0][0] = (GLfloat)from.a1; to[0][1] = (GLfloat)from.b1;  to[0][2] = (GLfloat)from.c1; to[0][3] = (GLfloat)from.d1;
	to[1][0] = (GLfloat)from.a2; to[1][1] = (GLfloat)from.b2;  to[1][2] = (GLfloat)from.c2; to[1][3] = (GLfloat)from.d2;
	to[2][0] = (GLfloat)from.a3; to[2][1] = (GLfloat)from.b3;  to[2][2] = (GLfloat)from.c3; to[2][3] = (GLfloat)from.d3;
	to[3][0] = (GLfloat)from.a4; to[3][1] = (GLfloat)from.b4;  to[3][2] = (GLfloat)from.c4; to[3][3] = (GLfloat)from.d4;
	return to;
}

glm::vec3 Model::GetMin(void) const { return _min; }
glm::vec3 Model::GetMax(void) const { return _max; }

void	Model::_LoadModel(std::string path)
{
	_playing = true;
	_scene = _importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
	_min = glm::vec3(0.0f);
	_max = glm::vec3(0.0f);
	_chrono = 0.0f;

	if (_scene->HasAnimations())
		std::cout << "Has animations"<< std::endl;
	if (!_scene || _scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !_scene->mRootNode)
		throw std::runtime_error(std::string("ERROR::ASSIMP::") + _importer.GetErrorString());
	if (_scene->HasAnimations())
	{
		_skeleton.reset(new Node(_scene->mRootNode));
		_animations.push_back(std::shared_ptr<Animation>(new Animation(_scene->mAnimations[0])));
		_hasAnim = true;
		_currentAnimation = 0;
	}

	/*
	   if (_scene->HasAnimations())
	   std::cout << "Has animations"<< std::endl;
	   if (_scene->HasMaterials())
	   std::cout << "Has material"<< std::endl;
	   if (_scene->HasMeshes())
	   std::cout << "Has meshes"<< std::endl;
	   if (_scene->HasTextures())
	   std::cout << "Has textures"<< std::endl;
	   */

	_globalTransform = aiMat4ToGlmMat4(_scene->mRootNode->mTransformation.Inverse());

	_dir = path.substr(0, path.find_last_of('/'));
	_ProcessNode(_scene->mRootNode, _scene);

}

void	Model::_ProcessNode(aiNode *node, const aiScene *scene)
{
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
		_meshes.push_back(_ProcessMesh(scene->mMeshes[node->mMeshes[i]], scene));
	for (unsigned int i = 0; i < node->mNumChildren; i++)
		_ProcessNode(node->mChildren[i], scene);
}
#include "PrintGlm.hpp"
void	Model::_BoneTransform(float timeInSecond, const std::shared_ptr<Shader>  shader)
{
	float ticksPerSecond = _animations[_currentAnimation]->ticksPerSecond != 0 ? _animations[_currentAnimation]->ticksPerSecond : 25.0f;
	float timeInTicks = timeInSecond * ticksPerSecond;
	float animationTime = fmod(timeInTicks, _animations[_currentAnimation]->duration);

	_ReadNodeHierarchy(animationTime, _skeleton, glm::mat4(1.0f));

	std::vector<glm::mat4>	Transform;
	for (unsigned int i = 0; i < _boneInfo.size(); i++)
		Transform.push_back(_boneInfo[i].finalTransMat);
	shader->setMat4v("gBones", Transform);
}

std::shared_ptr<NodeAnim> FindNodeAnim(std::shared_ptr<Animation> anim, const std::string nodeName)
{
	for (unsigned int i = 0 ; i < anim->numChannels ; i++) {
		if (anim->channels[i]->nodeName == nodeName) {
			return anim->channels[i];
		}
	}
	return nullptr;
}

unsigned int	Model::_FindKeys(float animationTime, std::shared_ptr<NodeAnim> nodeAnim, int state) const
{
	if (state == ROTATION)
	{
		for (unsigned int i = 0 ; i < nodeAnim->numRotationKeys - 1 ; i++) {
			if (animationTime < (float)nodeAnim->rotationKeys[i + 1].time) {
				return i;
			}
		}
	}
	else if (state == TRANSLATION)
	{
		for (unsigned int i = 0 ; i < nodeAnim->numPositionKeys - 1 ; i++) {
			if (animationTime < (float)nodeAnim->positionKeys[i + 1].time) {
				return i;
			}
		}
	}
	else
	{
		for (unsigned int i = 0 ; i < nodeAnim->numScalingKeys - 1 ; i++) {
			if (animationTime < (float)nodeAnim->scalingKeys[i + 1].time) {
				return i;
			}
		}
	}
	return 0;
}
glm::quat	Model::_CalcInterpolatedRotation(float animationTime, std::shared_ptr<NodeAnim> nodeAnim) const
{
	if (nodeAnim->numRotationKeys == 1)
		return nodeAnim->rotationKeys[0].value;

	unsigned int rotationIndex = _FindKeys(animationTime, nodeAnim, ROTATION);
	unsigned int nextRotationIndex = rotationIndex + 1;

	float delta = nodeAnim->rotationKeys[nextRotationIndex].time - nodeAnim->rotationKeys[rotationIndex].time;
	float factor = (animationTime - (float)nodeAnim->rotationKeys[rotationIndex].time) / delta;

	glm::quat start = nodeAnim->rotationKeys[rotationIndex].value;
	glm::quat end = nodeAnim->rotationKeys[nextRotationIndex].value;

	glm::quat ret = glm::mix(start, end, factor);
	return glm::normalize(ret);
}
/*
aiVector3D		Model::_CalcInterpolatedTranslation(float animationTime, const aiNodeAnim* nodeAnim) const
{
	if (nodeAnim->mNumPositionKeys == 1)
		return nodeAnim->mPositionKeys[0].mValue;

	unsigned int positionIndex = _FindKeys(animationTime, nodeAnim, TRANSLATION);
	unsigned int nextPositionIndex = positionIndex + 1;

	float delta = nodeAnim->mPositionKeys[nextPositionIndex].mTime - nodeAnim->mPositionKeys[positionIndex].mTime;
	float factor = (animationTime - (float)nodeAnim->mPositionKeys[positionIndex].mTime) / delta;

	const aiVector3D start = nodeAnim->mPositionKeys[positionIndex].mValue;
	const aiVector3D end = nodeAnim->mPositionKeys[nextPositionIndex].mValue;

	aiVector3D	vec = end - start;
	return (start + vec * factor).Normalize();
}

aiVector3D		Model::_CalcInterpolatedScaling(float animationTime, const aiNodeAnim* nodeAnim) const
{
	if (nodeAnim->mNumScalingKeys == 1)
		return nodeAnim->mScalingKeys[0].mValue;

	unsigned int scalingIndex = _FindKeys(animationTime, nodeAnim, SCALING);
	unsigned int nextScalingIndex = scalingIndex + 1;

	float delta = nodeAnim->mScalingKeys[nextScalingIndex].mTime - nodeAnim->mScalingKeys[scalingIndex].mTime;
	float factor = (animationTime - (float)nodeAnim->mScalingKeys[scalingIndex].mTime) / delta;

	const aiVector3D start = nodeAnim->mScalingKeys[scalingIndex].mValue;
	const aiVector3D end = nodeAnim->mScalingKeys[nextScalingIndex].mValue;

	aiVector3D	vec = end - start;
	return (start + vec * factor).Normalize();
}
*/
void	Model::_ReadNodeHierarchy(float animationTime, std::shared_ptr<Node> node, const glm::mat4 parentTransform)
{
	std::string nodeName = node->name;
	std::shared_ptr<Animation> animation = _animations[_currentAnimation];
	std::shared_ptr<NodeAnim> nodeAnim = FindNodeAnim(animation, nodeName);

	glm::mat4	nodeTransform = node->transform;
	if (nodeAnim != nullptr)
	{
		//aiVector3D	scale = _CalcInterpolatedScaling(animationTime, nodeAnim);
		//glm::mat4	scaleMat = glm::scale(glm::mat4(1.0f), glm::vec3(scale.x, scale.y, scale.z));

		glm::mat4	rotateMat = glm::mat4(_CalcInterpolatedRotation(animationTime, nodeAnim));

		//aiVector3D	position = _CalcInterpolatedTranslation(animationTime, nodeAnim);
		//glm::mat4	positionMat = glm::translate(glm::mat4(1.0f), glm::vec3(position.x, position.y, position.z));
		nodeTransform *= rotateMat;
	}

	glm::mat4 globalTransform = parentTransform * nodeTransform;

	if (_boneMap.find(nodeName) != _boneMap.end())
	{
		unsigned int boneIndex = _boneMap[nodeName];
		_boneInfo[boneIndex].finalTransMat = _globalTransform * globalTransform * _boneInfo[boneIndex].offsetMat;
	}

	for (unsigned int i = 0; i < node->numChildren; i++)
		_ReadNodeHierarchy(animationTime, node->children[i], globalTransform);
}
void	Model::_LoadBones(aiMesh *mesh, std::vector<Vertex>& vertices)
{
	for (unsigned int i = 0; i < mesh->mNumBones; i++)
	{
		unsigned int boneIndex = 0;
		std::string boneName(mesh->mBones[i]->mName.data);

		if (_boneMap.find(boneName) == _boneMap.end())
		{
			boneIndex = _boneMap.size();
			BoneInfo bi;
			_boneInfo.push_back(bi);
		}
		else
			boneIndex = _boneMap[boneName];

		_boneMap[boneName] = boneIndex;
		_boneInfo[boneIndex].offsetMat = aiMat4ToGlmMat4(mesh->mBones[i]->mOffsetMatrix);

		for (unsigned int j = 0; j < mesh->mBones[i]->mNumWeights; j++)
			_AddBoneData(boneIndex, mesh->mBones[i]->mWeights[j].mWeight, vertices[mesh->mBones[i]->mWeights[j].mVertexId]);
	}
}

void					Model::_AddBoneData(unsigned int id, float weight, Vertex& vertex)
{
	for (unsigned int i = 0; i < NUM_BONES_PER_VERTEX; i++)
	{
		if (vertex.weights[i] == 0)
		{
			vertex.weights[i] = weight;
			vertex.ids[i] = id;
			return;
		}
	}
}


Mesh	Model::_ProcessMesh(aiMesh *mesh, const aiScene *scene)
{
	std::vector<Vertex>	vertices;
	std::vector<unsigned int>	faces;
	std::vector<Texture>	textures;

	//	std::cout <<  "Load mesh" << std::endl;
	//Get vertices
	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;
		vertex.position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
		if (mesh->HasNormals())
			vertex.normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
		else
			vertex.normal = glm::vec3(0.0f, 0.0f, 0.0f);
		if (mesh->HasTextureCoords(0))
			vertex.texCoord = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
		else
			vertex.texCoord = glm::vec2(0.0f, 0.0f);
		for (unsigned int j = 0; j < NUM_BONES_PER_VERTEX; j++)
		{
			vertex.weights[j] = 0;
			vertex.ids[j] = 0;
		}
		if (_min == glm::vec3(0.0f))
			_min = vertex.position;
		if (_max == glm::vec3(0.0f))
			_max = vertex.position;

		if (vertex.position.x > _max.x)
			_max.x = vertex.position.x;
		if (vertex.position.y > _max.y)
			_max.y = vertex.position.y;
		if (vertex.position.z > _max.z)
			_max.z = vertex.position.z;

		if (vertex.position.x < _min.x)
			_min.x = vertex.position.x;
		if (vertex.position.y < _min.y)
			_min.y = vertex.position.y;
		if (vertex.position.z < _min.z)
			_min.z = vertex.position.z;
		vertices.push_back(vertex);
	}
	//Get Bones
	if (mesh->HasBones())
	{
		_LoadBones(mesh, vertices);
	}
	//Get faces
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace	face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
			faces.push_back(face.mIndices[j]);
	}
	if (scene->HasMaterials())
	{
		aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
		std::vector<Texture> diffuseMaps = _LoadMaterialTexture(material, aiTextureType_DIFFUSE, Diffuse);
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
	}
	//std::cout << "nb meshs" << std::endl;
	return Mesh(vertices, faces, textures);
}

std::vector<Texture>	Model::_LoadMaterialTexture(aiMaterial *mat, aiTextureType type, eTextureType typeName)
{
	std::vector<Texture>	textures;
	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString str;
		mat->GetTexture(type, i, &str);
		Texture texture;
		if (str.C_Str()[0] && str.C_Str()[0] != '*')
			texture.id = _TextureFromFile(str.C_Str(), _dir);
		else
			texture.id = _TextureFromFile(str.C_Str(), _scene->mTextures);
		texture.type = typeName;
		textures.push_back(texture);
	}
	return textures;
}
Texture					Model::_LoadSimpleTexture(eTextureType typeName, const std::string path)
{
	Texture texture;
	texture.id = _TextureFromFile(path);
	texture.type = typeName;
	return texture;
}


std::string 			Model::_GetFilename(const char *path, const std::string &directory)
{
	std::string filename = std::string(path);
	filename = directory + '/' + filename;
	return filename;
}
unsigned int 			Model::_TextureFromFile(const char *path, const std::string &directory)
{
	std::string filename = _GetFilename(path, directory);
	return (_TextureFromFile(filename));
	/*/std::string filename = std::string(path);
	  filename = directory + '/' + filename;*/
}

unsigned int 			Model::_TextureFromFile(const std::string &filename, aiTexture** textureArray)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int i, width, height, nrComponents;
	i = atoi(filename.substr(1, 1).c_str());

	unsigned char *data = stbi_load_from_memory((stbi_uc *)textureArray[i]->pcData, textureArray[i]->mWidth, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "failed to load texture " << filename << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}
unsigned int 			Model::_TextureFromFile(const std::string &filename)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "failed to load texture " << filename << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}
