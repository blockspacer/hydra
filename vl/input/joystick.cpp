/**
 *	Copyright (c) 2011 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-09
 *	@file input/joystick.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

#include "joystick.hpp"

vl::Joystick::Joystick(void)
{
}

void
vl::Joystick::_update(vl::JoystickEvent const &evt)
{
	// @todo should check that the data is valid new data
	JoystickEvent jevt(evt);

	jevt.clip_zero(_zero_size);

	_signal(jevt);

	for(std::vector<JoystickHandlerRefPtr>::iterator iter = _handlers.begin();
		iter != _handlers.end(); ++iter)
	{
		(*iter)->execute(jevt);
	}
}

void
vl::Joystick::add_handler(vl::JoystickHandlerRefPtr handler)
{
	std::vector<JoystickHandlerRefPtr>::iterator iter = std::find(_handlers.begin(), _handlers.end(), handler);
	if(iter == _handlers.end())
	{ _handlers.push_back(handler); }
}

void
vl::Joystick::remove_handler(JoystickHandlerRefPtr handler)
{
	std::vector<JoystickHandlerRefPtr>::iterator iter = std::find(_handlers.begin(), _handlers.end(), handler);
	if(iter != _handlers.end())
	{ _handlers.erase(iter); }
}

