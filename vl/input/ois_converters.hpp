/**
 *	Copyright (c) 2011 - 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-09
 *	@file input/ois_converters.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *	Licensed under commercial license.
 *
 */

/**	Converters from OIS event structures to Hydra event structures.
 */

#ifndef HYDRA_INPUT_OIS_CONVERS_HPP
#define HYDRA_INPUT_OIS_CONVERS_HPP

#include "input/serial_joystick_event.hpp"
#include "input/mouse_event.hpp"
#include "input/joystick_event.hpp"

#include <OIS/OISJoyStick.h>
#include <OIS/OISMouse.h>
#include <OIS/OISInputManager.h>

namespace vl
{

inline vl::JoystickEvent
convert_ois_to_hydra(OIS::JoyStickEvent const &evt)
{
	JoystickEvent e;
	e.info.dev_id = vl::dev_id_t( evt.device->getID() );
	e.info.vendor_id = vl::vendor_id_t( evt.device->vendor() );
	
	size_t vecsize = evt.state.mButtons.size();

	assert(vecsize <= e.state.buttons.size());
	for(size_t i = 0; i < vecsize; ++i)
	{
		e.state.buttons.at(i) = evt.state.mButtons[i];
	}
	
	// @todo this is really slow, should use resize and at instead
	// because it only has one memory allocation (push_back can have multiple)
	for( std::vector<OIS::Axis>::const_iterator it = evt.state.mAxes.begin(); it != evt.state.mAxes.end(); ++it)
	{
		e.state.axes.push_back( vl::scalar(vl::scalar(it->abs)/vl::scalar(OIS::JoyStick::MAX_AXIS)) );
	}
	
	for( std::vector<OIS::Vector3>::const_iterator it = evt.state.mVectors.begin(); it != evt.state.mVectors.end(); ++it)
	{
		e.state.vectors.push_back( vl::Vector3(it->x, it->y, it->z) );
	}
	
	return e;
}

inline MouseEvent
convert_ois_to_hydra(OIS::MouseEvent const &evt)
{
	MouseEvent mevt;
	mevt.X.abs = evt.state.X.abs;
	mevt.X.rel = evt.state.X.rel;
	mevt.Y.abs = evt.state.Y.abs;
	mevt.Y.rel = evt.state.Y.rel;
	mevt.Z.abs = evt.state.Z.abs;
	mevt.Z.rel = evt.state.Z.rel;
	mevt.buttons = evt.state.buttons;

	return mevt;
}


}	// namespace vl

#endif	// HYDRA_INPUT_OIS_CONVERS_HPP
