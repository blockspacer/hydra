/**
 *	Copyright (c) 2011 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-12
 *	@file slave.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

#ifndef HYDRA_SLAVE_HPP
#define HYDRA_SLAVE_HPP

// Base classes
#include "cluster/session.hpp"
#include "application.hpp"

namespace vl
{

class HYDRA_API Slave : public vl::Session, public vl::Application
{
public :
	Slave(void);

	void injectCommand(std::string const &cmd);

	void injectEvent(vl::cluster::EventData const &event);

	/// virtual overrides from Application

	virtual void exit(void);

	/// @returns true if the application is still running
	virtual bool isRunning(void) const;

	/// Private virtual overrides from Application
private :
	virtual void _mainloop(bool sleep);

	virtual void _do_init(ProgramOptions const &opt);

	// Data
private :
	// Renderer
	RendererUniquePtr _renderer;

	vl::cluster::ClientRefPtr _slave_client;

	/// Input events to be sent
	std::vector<vl::cluster::EventData> _events;

};	// class Slave

}	// namespace vl

#endif	// HYDRA_SLAVE_HPP
