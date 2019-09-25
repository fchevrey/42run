/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Animation.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jloro <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/09/25 14:38:13 by jloro             #+#    #+#             */
/*   Updated: 2019/09/25 14:42:15 by jloro            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ANIMATION_HPP
# define ANIMATION_HPP

# include "NodeAnim.hpp"

class Animation
{
	public:
		Animation(aiAnimation* anim);
		virtual~Animation();

		std::vector<NodeAnim>	channels;
		double					duration;
		std::string				name;
		unsigned int			numChannels;
		double					ticksPerSecond;
	private:
};

#endif
