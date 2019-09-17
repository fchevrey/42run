/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GameManager.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jloro <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/09/16 11:46:50 by jloro             #+#    #+#             */
/*   Updated: 2019/09/17 13:06:40 by jloro            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef GAMEMANAGER_HPP
# define GAMEMANAGER_HPP

# include "IGameObject.hpp"
# include "Model.hpp"
# include "Renderer.hpp"
# include <list>
# include "MeshRenderer.hpp"
# include "Player.hpp"

# define OBSTACLE_SPEED_DEFAULT 20

class GameManager : public Engine42::IGameObject
{
	public:
		GameManager(std::shared_ptr<Player> player);
		virtual~GameManager();

		virtual void	Update();
		virtual void	FixedUpdate();
	private:
		void					_CheckCollision();
		std::shared_ptr<Player>	_player;
		int						_score;
		int						_obstacleSpeed;
		bool					_canAdd;
		std::shared_ptr<Model>	_modelPillar;
		std::list<std::shared_ptr<Renderer>>	_pillar;
};

#endif
