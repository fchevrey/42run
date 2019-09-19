#ifndef GAMEOBJECT_HPP
# define GAMEOBJECT_HPP
# include "SDL.h"
# include "Transform.hpp"
# include <memory>
# include <list>

class AComponent;

class GameObject
{
	public:
		GameObject(void);
		GameObject(Transform trans);
		// IGameObject(const IGameObject & src);
		virtual ~GameObject(void);
		virtual std::shared_ptr<Transform> GetTransform(void) const;
		void AddComponent(std::shared_ptr<AComponent> component);
		template<class T>
		std::shared_ptr<T> GetComponent()
		{
			std::shared_ptr<T>  dst;
			for (auto it : _components)
			{
				try 
				{ 
					dst = std::dynamic_pointer_cast<T>(it);
					if (dst == nullptr)
						continue;
				}
				catch (std::exception &e) { continue; }
				return dst;
			}
			return dst;
		}
		/*
		template<class T>
		std::list<std::shared_ptr<T>> GetComponents()
		{
			std::list<std::shared_ptr<T>>  lst;
			for (auto it : _components)
			{
				try 
				{ 
					std::shared_ptr<T> dst = std::dynamic_pointer_cast<T>(it);
					lst.push_back(dst);
				}
				catch (std::exception &e) { continue; }
			}
			return lst;
		}*/
		// IGameObject &	operator=(const IGameObject & rhs);

		virtual void	Update(void);
		virtual void	FixedUpdate(void);
		// virtual int		Collide(IGameObject *other) = 0;
	protected:
		std::shared_ptr<Transform> _transform;
		std::list<std::shared_ptr<AComponent>> _components;
};
#endif