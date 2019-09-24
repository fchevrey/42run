/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Player.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jloro <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/09/12 16:55:11 by jloro             #+#    #+#             */
/*   Updated: 2019/09/24 16:54:08 by jloro            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef CHARACTER_HPP
# define CHARACTER_HPP

# include "Renderer.hpp"
# include "GameObject.hpp"
# include "Model.hpp"

# define SPEED 3
# define JUMP_SPEED 40
# define ROW_WIDTH 20

enum eJumpState {
	JUMPING,
	FALLING
};
class Player : public GameObject
{
	public:
		Player(std::shared_ptr<Model> character, std::shared_ptr<Shader> shader, Transform trans);
		virtual~Player();

		virtual void    Update(void);
		virtual void    FixedUpdate(void);
	private:
		std::shared_ptr<Model>	_character;
		bool					_jump;
		eJumpState				_jumpState;
		float					_velocityY;

};

#endif
