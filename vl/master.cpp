/**
 *	Copyright (c) 2010 - 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2010-10
 *	@file master.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

/// Interface
#include "master.hpp"

#include "base/exceptions.hpp"

// Necessary for retrieving other managers
#include "game_manager.hpp"
// Necessary for executing python commands from slaves
#include "python/python_context.hpp"
// Necessary for adding server report using templates
#include "python/python_context_impl.hpp"

// Necessary for retrieving registered triggers, KeyTrigger and FrameTrigger and sending events.
#include "event_manager.hpp"
// Necessary for the Event handling
#include <OIS/OISMouse.h>
#include <OIS/OISKeyboard.h>

#include "input/joystick_event.hpp"
#include "input/mouse_event.hpp"

// Necessary for loading meshes to Server
#include "mesh_manager.hpp"

// Necessary for creating local renderer
#include "renderer.hpp"
// Necessary for sleeping
#include "base/sleep.hpp"
// Necessary for spawning external processes
#include "base/system_util.hpp"
// Necessary for printing process vector
#include "base/print.hpp"
// Necessary for EnvSettings
#include "base/envsettings.hpp"

#include "pipe.hpp"

#include "remote_launcher_helper.hpp"

/// -------------------------------- Global ----------------------------------
vl::config::EnvSettingsRefPtr
vl::getMasterSettings( vl::ProgramOptions const &options )
{
	// Should never happen
	if( options.slave() )
	{ BOOST_THROW_EXCEPTION(vl::exception()); }

	fs::path env_path = options.environment_file;
	if( !fs::is_regular(env_path) )
	{
		// Valid names for the environment config:
		// hydra_env.xml, hydra.env
		// paths ${Program_dir}, ${Program_dir}/data, ${Program_dir}/../data
		// ${Program_dir}/config, ${Program_dir}/../config
		std::vector<fs::path> paths;
		paths.push_back( options.program_directory );
		paths.push_back( options.program_directory + "/data" );
		paths.push_back( options.program_directory + "/../data" );
		paths.push_back( options.program_directory + "/../config" );
		paths.push_back( options.program_directory + "/config" );
		for( size_t i = 0; i < paths.size(); ++i )
		{
			if( fs::exists(paths.at(i) / "hydra.env") )
			{
				env_path = paths.at(i) / "hydra.env";
				break;
			}
			else if( fs::exists(paths.at(i) / "hydra_env.xml") )
			{
				env_path = paths.at(i) / "hydra_env.xml";
				break;
			}
		}

	}

	vl::config::EnvSettingsRefPtr env( new vl::config::EnvSettings );

	// Read the Environment config
	// if there is no such file we return the default configuration
	if( fs::is_regular(env_path) )
	{
		std::string env_data;
		env_data = vl::readFileToString( env_path.string() );
		// TODO check that the files are correct and we have good settings
		vl::config::EnvSerializer env_ser;
		env_ser.readString(*env, env_data);
		env->setFile( env_path.string() );
	}

	env->setLogDir( options.getLogDir() );
	env->setLogLevel( (vl::config::LogLevel)(options.log_level) );

	return env;
}

/// ---------------------------------- Master --------------------------------
vl::Master::Master(void)
	: Application()
	, _game_manager(0)
	, _running(true)
	, _renderer(0)
	, _frame(0)
{
	std::cout << vl::TRACE << "vl::Master::Master" << std::endl;
}

vl::Master::~Master( void )
{
	std::cout << vl::TRACE << "vl::Master::~Master" << std::endl;

	// @fixme this fails for some reason
	//delete _renderer;

	delete _game_manager;

	for(size_t i = 0; i < _spawned_processes.size(); ++i)
	{
		kill_process(_spawned_processes.at(i));
	}
}

/// @todo this is taking way too long
/// @todo server should be running and responding to messages at the same time
/// Problem calling Server::poll infrequently through this function will
/// cause the clients to freeze.
/// Anyhow it would be much more useful to call Server::poll every 1ms,
/// for example with interrupts.
void
vl::Master::init(std::string const &global_file, std::string const &project_file)
{
	std::cout << vl::TRACE << "vl::Master::init" << std::endl;
	vl::chrono init_timer;

	vl::Report<vl::time> &report = _game_manager->getInitReport();
	vl::chrono t;

	_game_manager->process_event(vl::init(_env));

	// Local renderer needs to be inited rather than send a message
	// needs to be before we init GameManager
	// because for now the GameManager will send project (global) as a signal
	// and it is assumed that environment is set before processing
	// project signal.
	if(_renderer)
	{
		t.reset();
		_renderer->init();
		report["Initing Renderer"].push(t.elapsed());
	}

	report["Starting GameManager"].push(t.elapsed());

	_game_manager->load(project_file, global_file);

	/// Updating the Renderers
	t.reset();
	_server->poll();
	_updateFrameMsgs();
	// Not necessary to send update at start because Server will request
	// INIT message anyway
	if( !_msg_create.empty() )
	{
		_server->sendCreate(_msg_create);
	}
	report["Updating server"].push(t.elapsed());

	if(_renderer)
	{
		t.reset();
		vl::cluster::Message msg(_msg_create);
		_renderer->createSceneObjects(msg);
		msg = createMsgInit();
		_renderer->updateScene(msg);
		report["Creating objects in Renderer"].push(t.elapsed());
	}

	report["Total init"].push(init_timer.elapsed());
	report.finish();
	
	_stats_timer.reset();

	// We need implementation here
	// Bit hackish way to add global variables because we can't access them
	// through GameManager.
	static_cast<PythonContextImpl *>(_game_manager->getPython())
		->addVariableVal("server", _server);
	if(_renderer)
	{
		static_cast<PythonContextImpl *>(_game_manager->getPython())
			->addVariableRef("renderer", (Renderer *)_renderer);
	}

	_server->process_event(vl::cluster::init());

	// TODO this should block till both slaves and local renderer are ready
	// Problematic as this does not take into account clients that are started
	// later than this function is ran.
	// Another problem is that if we block here the starting is going to take
	// longer no matter what
	// For example problematic configurations and so on.
	// We should rather block the simulation till all slaves are up,
	// which should be configurable from ini file, but allow the use of
	// global actions like EXIT.
//	std::clog << "blocking till slave clients are ready." << std::endl;
//	_server->block_till_initialised();
	_server->poll();
}

void
vl::Master::exit(void)
{
	std::cout << vl::TRACE << "vl::Master::exit" << std::endl;

	delete _renderer;

	_server->shutdown();
}

void
vl::Master::render(void)
{
	if(!isRunning())
	{ return; }

	if( _frame == 0 )
	{ _sim_timer.reset(); }

	++_frame;

	assert(_server);

	vl::chrono loop_timer;
	vl::ProfilerReport &report = _game_manager->getRenderingReport();

	vl::chrono timer;

	// We need to start drawing before we process the next frame
	// Get new event messages that are processed in GameManager::step
	_server->poll();
	_handleMessages();

	// Process a time step in the game
	_game_manager->step();

	/// Provide the updates to slaves
	_updateFrameMsgs();
	_updateServer();
	_updateRenderer();

	/// Render the scene
	timer.reset();
	_server->start_draw(_frame, getSimulationTime());
	// Rendering after the server has sent the command to slaves
	if(_renderer)
	{
		_renderer->draw();
	}

	// @todo For some reason finish_draw takes the same time as local rendering.
	_server->finish_draw(_frame, getSimulationTime());

	// Finish local renderer
	if(_renderer)
	{
		_renderer->swap();
		_renderer->capture();
	}
	report[PT_RENDERING].push(timer.elapsed());

	report[PT_FRAME].push(loop_timer.elapsed());

	// Update statistics every second
	// @todo time limit should be configurable
	if( _stats_timer.elapsed() > vl::time(1) )
	{
		report.finish();
		_stats_timer.reset();
	}
}

vl::cluster::Message
vl::Master::popMessage(void)
{
	if( !messages() )
	{ return vl::cluster::Message(); }

	vl::cluster::Message msg = _messages.front();
	_messages.pop_front();
	return msg;
}

void
vl::Master::pushMessage(vl::cluster::Message const &msg)
{
	_messages.push_back(msg);
}

vl::cluster::Message
vl::Master::createMsgInit(void) const
{
	vl::cluster::Message msg(vl::cluster::MSG_SG_INIT, _frame, getSimulationTime());

	std::vector<vl::Distributed *>::const_iterator iter;
	for( iter = _registered_objects.begin(); iter != _registered_objects.end();
		++iter )
	{
		assert( (*iter)->getID() != vl::ID_UNDEFINED );
		vl::cluster::ObjectData data( (*iter)->getID() );
		vl::cluster::ByteDataStream stream = data.getStream();
		(*iter)->pack( stream, vl::Distributed::DIRTY_ALL );
		data.copyToMessage(&msg);
		/// Don't clear dirty because this is a special case
	}

	return msg;
}

vl::cluster::Message
vl::Master::createResourceMessage(vl::cluster::RESOURCE_TYPE type, std::string const &name) const
{
	std::clog << "vl::Master::createResourceMessage" << std::endl;

	vl::cluster::Message msg(vl::cluster::MSG_RESOURCE, _frame, getSimulationTime());
	if( type == vl::cluster::RES_MESH )
	{
		assert(getGameManager()->getMeshManager()->hasMesh(name));
		if(!getGameManager()->getMeshManager()->hasMesh(name))
		{
			BOOST_THROW_EXCEPTION(vl::null_pointer());
		}

		vl::MeshRefPtr mesh = getGameManager()->getMeshManager()->getMesh(name);
		
		vl::cluster::MessageDataStream stream = msg.getStream();
		stream << vl::cluster::RES_MESH << name << *mesh;
	}
	else
	{
		BOOST_THROW_EXCEPTION(vl::not_implemented());
	}

	return msg;
}


void
vl::Master::settingsChanged(void)
{
	_proj = _game_manager->getSettings();
	
	for(size_t i = 0; i < _pipes.size(); ++i)
	{
		_pipes.at(i)->clearResources();
		_pipes.at(i)->addResources(_game_manager->getSettings());
	}
}

void
vl::Master::clearProjectCallback(void)
{
	if(_server)
	{
		// @todo add Server clear project
	}

	if(_renderer)
	{
		_renderer->clearProject();
	}
}

/// -------------------- Private virtual overrides ---------------------------
void
vl::Master::_mainloop(bool sleep)
{
	vl::chrono timer;

	render();

	if(sleep)
	{
		vl::time sleep_time;
		// Try to get the requested frame rate but avoid division by zero
		// also check that the time used for the frame is less than the max fps so we
		// don't overflow the sleep time
		if( _env->getFPS() > 0 && (vl::time(1.0/_env->getFPS()) > timer.elapsed()) )
		{ sleep_time = vl::time(1.0/_env->getFPS()) - timer.elapsed(); }

		// Force context switching by zero sleep
		vl::sleep(sleep_time);
	}
}

void
vl::Master::_do_init(ProgramOptions const &opt)
{
	ProgramOptions options(opt);
	assert(opt.master());
	_env = vl::getMasterSettings(options);

	/// Do we have Oculus
	/// @todo this is rather hackish
	if(_env->getMaster().getNWindows() > 0)
	{
		for(size_t i = 0; i < _env->getMaster().getNWindows(); ++i)
		{
			if(_env->getMaster().getWindow(i).type == config::WT_OCULUS)
			{ options.oculus_rift = true; break; }
		}
	}

	if(!_env)
	{ BOOST_THROW_EXCEPTION(vl::exception()); }

	if(options.auto_fork)
	{
		for(size_t i = 0; i < _env->getSlaves().size(); ++i)
		{
#ifndef _WIN32
			pid_t pid = ::fork();
			// Reset the environment file for a slave
			if(pid != 0)
			{
				_env->setSlave();
				_env->getMaster().name = _env->getSlaves().at(i).name;
				// Master needs to continue the loop and create the remaining slaves
				break;
			}
#else
			/// @todo use a more general parameter for the program name
			std::vector<std::string> params;
			// Add slave param
			params.push_back("--slave");
			params.push_back(_env->getSlaves().at(i).name);
			// Add server param
			params.push_back("--server");
			std::stringstream ss;
			std::string hostname(_env->getServer().hostname);
			// Actually autolaunching is only useful on localhost
			// but lets forget this for the moment.
			if(hostname.empty())
			{ hostname = "localhost"; }
			uint16_t port = _env->getServer().port;

			ss << hostname << ":" << port;
			params.push_back(ss.str());
			params.push_back("--log_dir");
			params.push_back(_env->getLogDir());
			// Create the process
			// @todo print the parameters
			std::clog << "Auto launch parameters : " << params << std::endl;
			uint32_t pid = create_process("hydra.exe", params, true);
			_spawned_processes.push_back(pid);
		}
#endif
	}
	
	// Auto forking is incompatible with launchers so either or
	// if we have no slaves there is no reason what so ever to send a message
	if(!options.auto_fork && !_env->getSlaves().empty())
	{
		// Try to find launchers by broadcasting, 
		// launchers will start automatically when they receive start command

		/// @todo make port configurable
		RemoteLauncherHelper launcher_helper(options.launcher_port);
		launcher_helper.send_start(_env->getServer().port);
	}

	/// Correct name has been set
	std::cout << "vl::Master::Master : name = " << _env->getMaster().name << std::endl;

	/// Only create local Renderer if we have Windows defined
	if(_env->getMaster().getNWindows())
	{ _renderer = new Renderer(this, _env->getMaster().name); }
	else
	{ std::clog << "Not creating local Renderer." << std::endl; }

	_game_manager = new vl::GameManager(this, _logger, options);
	
	// Parses the env config and creates all pipes and windows
	// needs to be after GameManager has been created because we need Oculus info from it.
	_createWindows(_env);

	_server.reset(new vl::cluster::Server(_env->getServer().port));
	_server->addRequestMessageListener(boost::bind(&Master::messageRequested, this, _1));

	// if we have a renderer we have to set callbacks
	if(_renderer)
	{
		_renderer->addEventListener(boost::bind(&Master::injectEvent, this, _1));
		_renderer->addCommandListener(boost::bind(&PythonContext::executeCommand, _game_manager->getPython(), _1));
	}

	_game_manager->addStateChangedListener(GameManagerFSM_::Initing(), boost::bind(&Master::clearProjectCallback, this));
	_game_manager->addStateChangedListener(GameManagerFSM_::Loading(), boost::bind(&Master::settingsChanged, this));
	_game_manager->addStateChangedListener(GameManagerFSM_::Quited(), boost::bind(&Master::quit_callback, this));

	if(_renderer)
	{ _renderer->setMeshManager(_game_manager->getMeshManager()); }

	init(options.global_file, options.project_file);

	std::vector<vl::config::Program> programs = _env->getUsedPrograms();
	std::clog << "Should start " << programs.size() << " autolaunched programs."
		<< std::endl;
	for(size_t i = 0; i < programs.size(); ++i)
	{
		std::clog << "Starting : " << programs.at(i).name 
			<< " with command : " << programs.at(i).command << std::endl;
		uint32_t pid = create_process(programs.at(i).command, programs.at(i).params, programs.at(i).new_console);
		_spawned_processes.push_back(pid);
	}
}

void
vl::Master::messageRequested(vl::cluster::RequestedMessage const &req_msg)
{
	switch(req_msg.type)
	{
	case vl::cluster::MSG_ENVIRONMENT :
		{
			vl::cluster::Message msg(vl::cluster::MSG_ENVIRONMENT, _frame, getSimulationTime());
			_server->sendMessage(msg);
		}
		break;
	case vl::cluster::MSG_PROJECT :
		{
			/// @todo MSG_PROJECT should be removed, it's not used anymore
			vl::cluster::Message msg(vl::cluster::MSG_PROJECT, _frame, getSimulationTime());
			_server->sendMessage(msg);
		}
		break;
	case vl::cluster::MSG_SG_INIT :
		{
			_server->sendMessage(createMsgInit());
		}
		break;
	case vl::cluster::MSG_RESOURCE :
		{
			_server->sendMessage(createResourceMessage(req_msg.res_type, req_msg.name));
		}
		break;
	}
}

void
vl::Master::injectEvent(vl::cluster::EventData const &evt)
{
	_handleEvent(vl::cluster::EventData(evt));
}

vl::time 
vl::Master::getSimulationTime(void) const
{
	if( _frame == 0 )
	{ return vl::time(); }
	else
	{ return _sim_timer.elapsed(); }
}

/// ------------------------- Private ----------------------------------------
void
vl::Master::_createWindows(vl::config::EnvSettingsRefPtr env)
{
	// @todo we should combine these by using getNodes or something similar
	std::vector< std::pair<PipeRefPtr, config::Window> > windows;
	if(env->getMaster().getNWindows() > 0)
	{
		PipeRefPtr pipe(new Pipe(this, env->getMaster().name));
		// Because the gui::Windows are distributed if we enable this for
		// master we automatically enable it for slaves also.
		// oh well doesn't matter.
		pipe->enableDebugOverlay(true);
		for(size_t i = 0; i < env->getMaster().getNWindows(); ++i)
		{
			windows.push_back(std::make_pair(pipe, env->getMaster().getWindow(i)));
		}
		_pipes.push_back(pipe);
	}

	for(size_t i = 0; i < env->getSlaves().size(); ++i)
	{
		config::Node const &slave = env->getSlaves().at(i);
		PipeRefPtr pipe(new Pipe(this, slave.name));
		for(size_t j = 0; j < slave.getNWindows(); ++j)
		{
			windows.push_back(std::make_pair(pipe, slave.getWindow(j)));
		}
		_pipes.push_back(pipe);
	}

	for(std::vector< std::pair<PipeRefPtr, config::Window> >::iterator iter = windows.begin();
		iter != windows.end(); ++iter)
	{
		config::Window win = iter->second;

		if(win.type == config::WT_OCULUS)
		{
			// Configure window
			// copy only w and h, because we don't have a decent method for
			// retrieving x and y from oculus so they are in the config
			// @fixme Hardcoded for testing
			// do we need to retrieve them here or should we just use the
			// Window creation to retrieve the information
			// then again it's gonna create the maximum size Window anyway
			// if it's smaller than specified
			win.area.w = 1920;
			win.area.h = 1080;
			
			win.stereo_type = config::ST_OCULUS;

			// auto create a channel if we have none
			// this way because we want to reuse any configured channels
			if(win.get_channels().size() == 0)
			{
				// empty because we configure it later
				win.add_channel(config::Channel());
			}
			// @todo should remove all but the first channel

			// overwrite first channel config
			config::Channel &chan = win.get_channel(0);
			chan.name = "oculus";

			chan.projection.use_asymmetric_stereo = false;
			chan.projection.hmd = true;
			chan.projection.fov = 60;
		}

		iter->first->createWindow(win);
	}
}

void
vl::Master::_updateServer( void )
{
	assert( _server );

	if( !_msg_create.empty() )
	{
		_server->sendCreate(_msg_create);
	}

	_server->sendUpdate(_msg_update);

	// Send logs
	if( _server->logEnabled() )
	{
		vl::Logger *log = _game_manager->getLogger();
		while( _server->nLoggedMessages() <  log->nMessages() )
		{
			_server->logMessage( log->getMessage(_server->nLoggedMessages()) );
		}
	}

	_server->poll();
}

void
vl::Master::_updateRenderer(void)
{
	if(!_renderer)
	{ return; }

	if( !_msg_create.empty() )
	{
		_renderer->createSceneObjects(vl::cluster::Message(_msg_create));
	}

	_renderer->updateScene(vl::cluster::Message(_msg_update));

	// Send logs
	if( _renderer->logEnabled() )
	{
		vl::Logger *log = _game_manager->getLogger();
		while( _renderer->nLoggedMessages() <  log->nMessages() )
		{
			_renderer->logMessage( log->getMessage(_renderer->nLoggedMessages()) );
		}
	}
}

void
vl::Master::_updateFrameMsgs(void)
{
	_createMsgCreate();
	_createMsgUpdate();
}

void
vl::Master::_createMsgCreate(void)
{
	// New objects created need to send SG_CREATE message
	if( !getNewObjects().empty() )
	{
		_msg_create = vl::cluster::Message( vl::cluster::MSG_SG_CREATE, _frame, getSimulationTime());
		_msg_create.write( getNewObjects().size() );
		for( size_t i = 0; i < getNewObjects().size(); ++i )
		{
			OBJ_TYPE type = getNewObjects().at(i).first;
			uint64_t id = getNewObjects().at(i).second->getID();
			_msg_create.write( type );
			_msg_create.write( id );
		}

		clearNewObjects();
	}
	else
	{
		_msg_create.clear();
	}
}

void
vl::Master::_createMsgUpdate(void)
{
	// Create SceneGraph updates
	_msg_update = vl::cluster::Message(vl::cluster::MSG_SG_UPDATE, _frame, getSimulationTime());

	std::vector<vl::Distributed *>::iterator iter;
	for( iter = _registered_objects.begin(); iter != _registered_objects.end();
		++iter )
	{
		if( (*iter)->isDirty() )
		{
			assert( (*iter)->getID() != vl::ID_UNDEFINED );
			vl::cluster::ObjectData data( (*iter)->getID() );
			vl::cluster::ByteDataStream stream = data.getStream();
			(*iter)->pack(stream);
			data.copyToMessage(&_msg_update);
			/// Clear dirty because this update has been applied
			(*iter)->clearDirty();
		}
	}
}

/// Event Handling
void
vl::Master::_handleMessages( void )
{
	// Check local messages, pushed there by callbacks
	while( messages() )
	{
		vl::cluster::Message msg = popMessage();
		_handleMessage(msg);
	}

	// TODO this should use the same callback system as Renderer
	while( _server->messages() )
	{
		vl::cluster::Message msg = _server->popMessage();
		_handleMessage(msg);
	}
}

void
vl::Master::_handleMessage(vl::cluster::Message &msg)
{
	switch( msg.getType() )
	{
		case vl::cluster::MSG_INPUT:
			_handleEventMessage(msg);
			break;

		case vl::cluster::MSG_COMMAND:
			_handleCommandMessage(msg);
			break;

		case vl::cluster::MSG_REG_OUTPUT:
			// Should not be added as we are using the LogReceiver interface
			// Problematic as the Remote nodes need this message, but the
			// local node does not.
			break;

		default :
			std::cout << vl::CRITICAL << "Unhandled Message in Master : type = "
				<< vl::cluster::getTypeAsString(msg.getType()) << std::endl;
	}
}

void
vl::Master::_handleEventMessage(vl::cluster::Message &msg)
{
	while( msg.size() )
	{
		vl::cluster::EventData data;
		data.copyFromMessage(&msg);
		_handleEvent(data);
	}
}

void
vl::Master::_handleEvent(vl::cluster::EventData &event)
{
	vl::cluster::ByteDataStream stream = event.getStream();
	switch( event.getType() )
	{
		case vl::cluster::EVT_KEY_PRESSED :
		{
			OIS::KeyEvent evt( 0, OIS::KC_UNASSIGNED, 0 );
			stream >> evt;
			// @todo should pass the whole event structure
			_game_manager->getEventManager()->keyPressed(evt.key);
		}
		break;

		case vl::cluster::EVT_KEY_RELEASED :
		{
			OIS::KeyEvent evt( 0, OIS::KC_UNASSIGNED, 0 );
			stream >> evt;
			// @todo should pass the whole event structure
			_game_manager->getEventManager()->keyReleased(evt.key);
		}
		break;

		case vl::cluster::EVT_MOUSE_PRESSED :
		{
			vl::MouseEvent::BUTTON b_id;
			vl::MouseEvent evt;
			
			stream >> b_id >> evt;
			
			// @remove: print to test!
			//std::clog << " RECEIVED EVENT: " << std::endl << evt << std::endl;
			
			_game_manager->getEventManager()->mousePressed(evt, b_id);

			
		}
		break;

		case vl::cluster::EVT_MOUSE_RELEASED :
		{
			//OIS::MouseButtonID b_id;
			
			vl::MouseEvent evt;
			vl::MouseEvent::BUTTON b_id;

			stream >> b_id >> evt;
			
			// @remove: print to test!
			//std::clog << " RECEIVED EVENT: " << std::endl << evt << std::endl;
			
			_game_manager->getEventManager()->mouseReleased(evt, b_id);

		}
		break;

		case vl::cluster::EVT_MOUSE_MOVED :
		{
			vl::MouseEvent evt;
			
			stream >> evt;
			
			// @remove: print to test!
			//std::clog << " RECEIVED EVENT: " << std::endl << evt << std::endl;

			_game_manager->getEventManager()->mouseMoved(evt);
			
		}
		break;

		case vl::cluster::EVT_JOYSTICK_PRESSED :
		{
			vl::JoystickEvent evt;
			int button;
			
			stream >> button >> evt;
			_game_manager->getEventManager()->updateGameJoystick(evt, button);
		}
		break;

		case vl::cluster::EVT_JOYSTICK_RELEASED :
		{
			vl::JoystickEvent evt;
			int button;
			stream >> button >> evt;
			_game_manager->getEventManager()->updateGameJoystick(evt, button);
		}
		break;

		case vl::cluster::EVT_JOYSTICK_AXIS :
		{
			vl::JoystickEvent evt;
			int axis;
			stream >> axis >> evt;
			_game_manager->getEventManager()->updateGameJoystick(evt, axis);
		}
		break;

		case vl::cluster::EVT_JOYSTICK_POV :
		{
			vl::JoystickEvent evt;
			int pov;
			stream >> pov >> evt;
			_game_manager->getEventManager()->updateGameJoystick(evt, pov);
		}
		break;

		case vl::cluster::EVT_JOYSTICK_VECTOR3 :
		{
			vl::JoystickEvent evt;
			int index;
			stream >> index >> evt;
			_game_manager->getEventManager()->updateGameJoystick(evt, index);
		}
		break;
		
		case vl::cluster::EVT_JOYSTICK_SLIDER :
		{
			vl::JoystickEvent evt;
			int index;
			stream >> index >> evt;
			_game_manager->getEventManager()->updateGameJoystick(evt, index);
		}
		break;

		case vl::cluster::EVT_HEAD_TRANSFORM :
		{
			vl::Transform head_t;
			stream >> head_t;
			_game_manager->setOculusEnabled(true);
			_game_manager->getPlayer()->setHeadTransform(head_t);
		}
		break;

		default :
			std::cout << vl::CRITICAL << "vl::Master::_receiveEventMessages : "
				<< "Unhandleded message type." << std::endl;
			break;
	}
}

void
vl::Master::_handleCommandMessage(vl::cluster::Message &msg)
{
	// TODO there should be a maximum amount of messages stored
	// TODO works only on ASCII at the moment
	std::string cmd;
	msg.read(cmd);
	_game_manager->getPython()->executeCommand(cmd);
}
