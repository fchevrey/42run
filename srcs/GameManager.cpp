#include "GameManager.hpp"
#include "Engine.hpp"
#include <cmath>
#include "ACollider.hpp"

const unsigned int	GameManager::speedWorld = 80.0f;
std::shared_ptr<GameManager>	GameManager::instance = nullptr;

GameManager::GameManager(std::shared_ptr<Player> player) : player(player), _score(0)
{
	if (instance == nullptr)
		instance = std::shared_ptr<GameManager>(this);
	_score = 0;
	_rooms.reset(new RoomManager);
	Engine42::Engine::AddGameObject(_rooms);
}

GameManager::~GameManager() {}

void	GameManager::Update()
{
	for (auto it = _rooms->obstacles->obstacles.begin(); it != _rooms->obstacles->obstacles.end(); it++)
	{
		if ((*it)->GetComponent<MeshRenderer>()->IsRender() && player->GetComponent<ACollider>()->IsCollindingWith(*(*it)->GetComponent<ACollider>()))
		{
			(*it)->GetComponent<MeshRenderer>()->SetRender(false);
			(*it)->GetComponent<MeshRenderer>()->Destroy();
			_rooms->obstacles->obstacles.erase(it);
			Die();
			break;
		}
	}
}

void	GameManager::Die()
{
	player->_character->ChangeAnimation(1);
	player->SetDead(true);
	_rooms->Stop();
}
void	GameManager::FixedUpdate()
{
	
}

void	GameManager::Reset()
{
	Engine42::Engine::Clear();
	player->SetDead(false);
	player->_character->ChangeAnimation(0);
	_rooms->Reset();
	Engine42::Engine::AddGameObject(_rooms);
	Engine42::Engine::AddGameObject(Camera::instance);
	player->GetTransform()->position = glm::vec3(0.0f);
	Engine42::Engine::AddGameObject(player);
	Engine42::Engine::AddRenderer(player->GetComponent<MeshRenderer>());
	Engine42::Engine::AddGameObject(instance);
}
