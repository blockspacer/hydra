/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2011-01
 *
 *
 */

// Interface
#include "pipe.hpp"

// Necessary for mapping distributed objects
#include <eq/client/config.h>

// Necessary for printing error messages from exceptions
#include "base/exceptions.hpp"
// Necessary for loading dotscene
#include "eq_ogre/ogre_dotscene_loader.hpp"

#include "window.hpp"
#include "base/string_utils.hpp"
#include "base/sleep.hpp"
#include "distrib_settings.hpp"

/// ------------------------- Public -------------------------------------------
// TODO should probably copy the env settings and not store the reference
eqOgre::Pipe::Pipe( std::string const &name,
					std::string const &server_address,
					uint16_t server_port )
	: _name(name), _ogre_sm(0), _camera(0), _screenshot_num(0), _client(0)
{
	std::cout << "eqOgre::Pipe::Pipe : name = " << _name << std::endl;
	_createClient( server_address, server_port );
}

eqOgre::Pipe::~Pipe( void )
{
	// Info
	std::string message("Cleaning out OGRE");
	Ogre::LogManager::getSingleton().logMessage( message );
	_root.reset();

	delete _client;
}

vl::EnvSettingsRefPtr
eqOgre::Pipe::getSettings( void )
{
	return _env;
}

vl::EnvSettings::Node
eqOgre::Pipe::getNodeConf( void )
{
	vl::EnvSettings::Node node;
	if( getName() == _env->getMaster().name )
	{ node = _env->getMaster(); }
	else
	{ node = _env->findSlave( getName() ); }

	assert( !node.empty() );
	return node;
}

vl::EnvSettings::Window
eqOgre::Pipe::getWindowConf( std::string const &window_name )
{
	vl::EnvSettings::Node node = getNodeConf();
	assert( node.getNWindows() > 0 );

	// TODO add support for finding correct window by name
	return node.getWindow(0);
}

void
eqOgre::Pipe::sendMessageToMaster( vl::cluster::Message *msg )
{
	_client->sendMessage(msg);
}

void
eqOgre::Pipe::sendEvent( vl::cluster::EventData const &event )
{
	// Add to event stack for sending them at once in one message to the Master
	_events.push_back(event);
}

void
eqOgre::Pipe::operator()()
{
	std::cout << "eqOgre::Pipe::operator() : Thread entered." << std::endl;

	// Here we should wait for the EnvSettings from master
	// TODO we should have a wait for Message function
	while( !_env )
	{
		_handleMessages();
		vl::msleep(1);
	}

	_createOgre();

	_createWindow();

	while( 1 )
	{
		// Handle messages

		_handleMessages();

		_syncData();
		// Render

		// Process input events
		assert( _window );
		_window->capture();

		// Send messages
		_sendEvents();

		// Sleep
		vl::msleep(1);
	}
}


/// ------------------------ Protected -----------------------------------------
// Not used
void
eqOgre::Pipe::frameStart( uint64_t frameID, const uint32_t frameNumber )
{
	// TODO should update the Pipe data in init
	// FIXME this updates the SceneManager and tries to find Ogre nodes
	// but it doesn't have the SceneManager as it's created in the init
	// next.
	_updateDistribData();

	// Init the Ogre resources and load a Scene
	// These are here because the RenderWindow needs to be created before
	// loading Meshes
	{
		// We need to find the node from scene graph
		std::string message = "SceneManager has "
			+ vl::to_string(_scene_manager->getNSceneNodes()) + " SceneNodes.";
		Ogre::LogManager::getSingleton().logMessage( message );

		assert( _ogre_sm );
		if( !_scene_manager->setSceneManager( _ogre_sm ) )
		{
			// Error
			message = "Some SceneNodes were not found.";
			Ogre::LogManager::getSingleton().logMessage( message, Ogre::LML_CRITICAL );
		}
	}
}


void
eqOgre::Pipe::_reloadProjects( vl::Settings set )
{
	// TODO this should unload old projects

	_settings = set;

	std::vector<vl::ProjSettings::Scene> scenes = _settings.getScenes();

	// Add resources
	_root->addResource( _settings.getProjectDir() );
	for( size_t i = 0; i < _settings.getAuxDirectories().size(); ++i )
	{
		_root->addResource( _settings.getAuxDirectories().at(i) );
	}

	std::cout << "Setting up the resources." << std::endl;
	_root->setupResources();
	_root->loadResources();

	_ogre_sm = _root->createSceneManager("SceneManager");

	std::cout << "eqOgre::Pipe::frameStart : loading scene" << std::endl;

	for( size_t i = 0; i < scenes.size(); ++i )
	{
		_loadScene( scenes.at(i) );
	}

	_setCamera();
}

/// Ogre helpers
void
eqOgre::Pipe::_createOgre( void )
{
	std::cout << "eqOgre::Pipe::_createOgre" << std::endl;
	assert( _env );

	// TODO needs LogManager creation before this
	std::string message("Creating Ogre Root");
	std::cout << message << std::endl;

	// FIXME the log file path should be retrieved from EnvSettings
	// FIXME the verbose parameter needs to be passed to Root
	//_root.reset( new vl::ogre::Root( getSettings().getOgreLogFilePath() ) );
	// TODO the project name should be used instead of the hydra for all
	// problem is that the project name is not known at this point
	// so we should use a tmp file and then move it.
	std::string log_file = vl::createLogFilePath( "hydra", "ogre", "", _env->getLogDir() );
	_root.reset( new vl::ogre::Root( log_file, _env->getVerbose() ) );
	// Initialise ogre
	_root->createRenderSystem();
}

void
eqOgre::Pipe::_loadScene( vl::ProjSettings::Scene const &scene )
{
	std::cout << "eqOgre::Pipe::_loadScene" << std::endl;
	assert( _ogre_sm );

	std::string message;

	std::string const &name = scene.getName();

	message = "Loading scene " + name + " file = " + scene.getFile();
	std::cout << message << std::endl;
// 	Ogre::LogManager::getSingleton().logMessage( message );

	eqOgre::DotSceneLoader loader;
	// TODO pass attach node based on the scene
	// TODO add a prefix to the SceneNode names ${scene_name}/${node_name}
	loader.parseDotScene( scene.getFile(), _ogre_sm );

	message = "Scene " + name + " loaded.";
	Ogre::LogManager::getSingleton().logMessage( message );
}

void
eqOgre::Pipe::_setCamera ( void )
{
	assert( _ogre_sm );

	/// Get the camera
	// TODO move to separate function

	// Loop through all cameras and grab their name and set their debug representation
	Ogre::SceneManager::CameraIterator cameras = _ogre_sm->getCameraIterator();

	if( !_active_camera_name.empty() )
	{
		if( _camera && _camera->getName() == _active_camera_name )
		{ return; }

		if( _ogre_sm->hasCamera(_active_camera_name) )
		{
			_camera = _ogre_sm->getCamera(_active_camera_name);
		}
	}

	if( !_camera )
	{
		std::string message;
		// Grab the first available camera, for now
		if( cameras.hasMoreElements() )
		{
			_camera = cameras.getNext();
			message = "Using Camera " + _camera->getName() + " found from the scene.";
			Ogre::LogManager::getSingleton().logMessage( message );
		}
		else
		{
			// TODO this should use a default camera created earlier that exists always
			_camera = _ogre_sm->createCamera("Cam");
			message = "No camera in the scene. Using created camera "
				+ _camera->getName();
			Ogre::LogManager::getSingleton().logMessage( message );
		}
		_active_camera_name = _camera->getName();
	}

	assert( _camera && _active_camera_name == _camera->getName() );
	_window->setCamera(_camera);

	std::cout << "Camera " << _active_camera_name << " set." << std::endl;
}


/// Distribution helpers
void
eqOgre::Pipe::_createClient( std::string const &server_address, uint16_t server_port )
{
	std::cout << "eqOgre::Pipe::_createClient" << std::endl;
	assert( !_client );

	// FIXME these should be configured in config file
	_client = new vl::cluster::Client( server_address.c_str(), server_port );

	_client->registerForUpdates();
}

void
eqOgre::Pipe::_handleMessages( void )
{
// 	std::cout << "eqOgre::Pipe::_handleMessages" << std::endl;

	assert( _client );

	_client->mainloop();
	while( _client->messages() )
	{
// 		std::cout << "eqOgre::Pipe::_handleMessages : Messages received" << std::endl;
		vl::cluster::Message *msg = _client->popMessage();
		// TODO process the message
		_handleMessage(msg);
		delete msg;
	}
}

void
eqOgre::Pipe::_handleMessage( vl::cluster::Message *msg )
{
	assert(msg);

	switch( msg->getType() )
	{
		// Environment configuration
		case vl::cluster::MSG_ENVIRONMENT :
		{
			std::cout << "eqOgre::Pipe::_handleMessage : MSG_ENVIRONMENT message" << std::endl;
			assert( !_env );
			_env.reset( new vl::EnvSettings );
			// TODO needs a ByteData object for Environment settings
			vl::SettingsByteData data;
			data.copyFromMessage(msg);
 			vl::cluster::ByteStream stream(&data);
			stream >> _env;
			// Only single environment settings should be in the message
			assert( 0 == msg->size() );
			_client->sendAck( vl::cluster::MSG_ENVIRONMENT );
		}
		break;

		// Project configuration
		case vl::cluster::MSG_PROJECT :
		{
			std::cout << "eqOgre::Pipe::_handleMessage : MSG_PROJECT message" << std::endl;
			// TODO
			// Test using multiple projects e.g. project and global
			// The vector serailization might not work correctly
			// TODO
			// Problematic because the Project config should be
			// updatable during the application run
			// And this one will create them anew, so that we need to invalidate
			// the scene and reload everything
			// NOTE
			// Combining the project configurations is not done automatically
			// so they either need special structure or we need to iterate over
			// all of them always.
			// TODO needs a ByteData object for Environment settings
			vl::SettingsByteData data;
			data.copyFromMessage(msg);
			vl::cluster::ByteStream stream(&data);
			vl::Settings projects;
			stream >> projects;
			_reloadProjects(projects);
			// TODO should the ACK be first so that the server has the
			// information fast
			_client->sendAck( vl::cluster::MSG_PROJECT );
		}
		break;

		// Scene graph initial state
		case vl::cluster::MSG_INITIAL_STATE :
		{
			std::cout << "eqOgre::Pipe::_handleMessage : MSG_INITIAL_STATE message" << std::endl;
			_client->sendAck( vl::cluster::MSG_INITIAL_STATE );
		}
		break;

		// Scene graph update after the initial message
		case vl::cluster::MSG_UPDATE :
		{
			// TODO objects array should not be cleared but rather updated
			// so that objects that are not updated stay in the array.
			_objects.clear();
			// Read the IDs in the message and call pack on mapped objects
			// based on thoses
			// TODO multiple update messages in the same frame,
			// only the most recent should be used.
// 			std::cout << "Message = " << *msg << std::endl;
			while( msg->size() > 0 )
			{
// 				std::cout << "eqOgre::Pipe::_syncData : UPDATE message : "
//	 				<< "size = " << msg->size() << std::endl;

				vl::cluster::ObjectData data;
				data.copyFromMessage(msg);
				// Pushing back will create copies which is unnecessary
				_objects.push_back(data);
			}
			_client->sendAck( vl::cluster::MSG_UPDATE );
// 			std::cout << "Message handled" << std::endl;
		}
		break;

		case vl::cluster::MSG_DRAW :
		{
			_client->sendAck( vl::cluster::MSG_DRAW );
			_draw();
		}
		break;

		case vl::cluster::MSG_SWAP :
		{
			_client->sendAck( vl::cluster::MSG_SWAP );
			_swap();
		}
		break;


		default :
			std::cout << "Unhandled Message of type = " << msg->getType()
				<< std::endl;
			break;
	}
}

void
eqOgre::Pipe::_syncData( void )
{
// 	std::cout << "eqOgre::Pipe::_syncData" << std::endl;
// 	std::cout << "eqOgre::Pipe::_syncData : " << _objects.size() << " objects."
// 		<< std::endl;
// FIXME this needs mapping to work correctly
// 	std::vector<vl::cluster::ObjectData>::iterator iter;
// 	// TODO remove the temporary array
// 	// use a custom structure that does not create temporaries
// 	// rather two phase system one to read the array and mark objects for delete
// 	// and second that really clear those that are marked for delete
// 	// similar system for reading data to the array
// 	std::vector<vl::cluster::ObjectData> tmp;
// 	for( iter = _objects.begin(); iter != _objects.end(); ++iter )
// 	{
// 		vl::cluster::ByteStream stream = iter->getStream();
// 		// TODO break this to two different parts
// 		// one creating the ObjectDatas and saving them to array
// 		// other one that processes this array and removes the elements found in mapped
// 		vl::Distributed *obj = findMappedObject( iter->getId() );
// 		if( obj )
// 		{
// // 			std::cout << "ID " << iter->getId() << " found in mapped objects."
// // 				<< " unpacking. " << std::endl;
// // 			std::cout << "object = " << *iter << std::endl;
// 			obj->unpack(stream);
// 		}
// 		else
// 		{
// 			std::cerr << "No ID " << iter->getId() << " found in mapped objects."
// 				<< std::endl;
// 			tmp.push_back( *iter );
// 		}
// 	}
//
// 	_objects = tmp;
//
// 	if( _scene_manager )
// 	{ _scene_manager->finaliseSync(); }

// 	std::cout << "eqOgre::Pipe::_syncData : done" << std::endl;
}

void
eqOgre::Pipe::_mapData( uint64_t settingsID )
{
	// TODO move to using Ogre Logging, needs LogManager creation
// 	std::cout << "eqOgre::Pipe::_mapData" << std::endl;

	// Get the cluster version of data
	// TODO the settings are not mapped now, they are sent using separate
	// messages by a request from here
// 	getConfig()->mapObject( &_settings, settingsID );
// 	assert( _settings.getID() != eq::UUID::ZERO);

	// TODO should be automatic when mapObjectC is called
	// Also if we call it so many times it should not iterate through the whole
	// message but rather already stored ID list

	/*	FIXME Distributed Settings removed
	uint64_t id = _settings.getResourceManagerID();
	assert( id != vl::ID_UNDEFINED );
	mapObjectC( &_resource_manager, id );

	assert( _settings.getPlayerID() != vl::ID_UNDEFINED );
	mapObjectC( &_player, _settings.getPlayerID() );

	uint64_t sm_id = _settings.getSceneManagerID();
	std::cout << "Mapping SceneManager with id = " << sm_id << '.' << std::endl;
	assert( sm_id != vl::ID_UNDEFINED );
	_scene_manager = new vl::SceneManager( this );
	mapObjectC( _scene_manager, sm_id );

	std::cout << "Data mapped." << std::endl;
	*/
}

void
eqOgre::Pipe::_updateDistribData( void )
{
// 	std::cout << "eqOgre::Pipe::_updateDistribData" << std::endl;
	// Custom sync code
	_syncData();

	// Update player
	// Get active camera and change the rendering camera if there is a change
	std::string const &cam_name = _player.getActiveCamera();
	if( !cam_name.empty() && cam_name != _active_camera_name )
	{
		/*	FIXME Window needs to be ported
		_active_camera_name = cam_name;
		assert( _ogre_sm );
		if( _ogre_sm->hasCamera( cam_name ) )
		{
			// Tell the Windows to change cameras
			_camera = _ogre_sm->getCamera( _active_camera_name );
			Windows const &window_list = getWindows();
			for( size_t i = 0; i < window_list.size(); ++i )
			{
				assert( dynamic_cast<eqOgre::Window *>( window_list.at(i) ) );
				eqOgre::Window *window =
					static_cast<eqOgre::Window *>( window_list.at(i) );
				window->setCamera( _camera );
			}
		}
		else
		{
			std::string message = "eqOgre::Window : New camera name set, but NO camera found";
			std::cout << message << std::endl;
			//Ogre::LogManager::getSingleton().logMessage( message );
		}
		*/
	}

	// Take a screenshot
	if( _player.getScreenshotVersion() > _screenshot_num )
	{
		// TODO should write the screenshot to the project directory not
		// to current directory
		// Add the screenshot dir to DistributedSettings
		// TODO the format of the screenshot name should be
		// screenshot_{project_name}-{year}-{month}-{day}-{time}-{window_name}.png
		std::string prefix( "screenshot_" );
		std::string suffix = ".png";//

		// Tell the Windows to take a screenshot
		/*	FIXME Window needs to be ported
		Windows const &window_list = getWindows();
		for( size_t i = 0; i < window_list.size(); ++i )
		{
			assert( dynamic_cast<eqOgre::Window *>( window_list.at(i) ) );
			eqOgre::Window *window =
				static_cast<eqOgre::Window *>( window_list.at(i) );
			window->takeScreenshot( prefix, suffix );
		}
		*/

		_screenshot_num = _player.getScreenshotVersion();
	}

// 	std::cout << "eqOgre::Pipe::_updateDistribData : done" << std::endl;
	// FIXME this is completely screwed up.
	/*
	static uint32_t scene_version = 0;
	if( _frame_data.getSceneVersion() > scene_version )
	{
		// This will reload the scene but all transformations remain
		// As this will not reset the SceneNode structures that control the
		// transformations of objects.
		std::cout << "Reloading the Ogre scene now" << std::endl;
		eqOgre::Window *win = static_cast<eqOgre::Window *>( getWindow() );
		win->loadScene();
		Ogre::Camera *camera = win->getCamera();
		createViewport( camera );
		_frame_data.setSceneManager( win->getSceneManager() );
		std::cout << "Ogre Scene reloaded." << std::endl;

		scene_version = _frame_data.getSceneVersion();
	}
	*/
}

void
eqOgre::Pipe::_draw( void )
{
	// TODO support for multiple windows
	_window->draw();
}

void
eqOgre::Pipe::_swap( void )
{
	// TODO support for multiple windows
	_window->swap();
}

void
eqOgre::Pipe::_sendEvents( void )
{
	if( !_events.empty() )
	{
// 		std::cout << "eqOgre::Pipe::_sendEvents : " << _events.size()
// 			<< " events to send." << std::endl;

		vl::cluster::Message msg( vl::cluster::MSG_INPUT );
		std::vector<vl::cluster::EventData>::iterator iter;
		for( iter = _events.begin(); iter != _events.end(); ++iter )
		{
			iter->copyToMessage(&msg);
		}
		_events.clear();

		sendMessageToMaster(&msg);
	}
}

// TODO add support for multiple windows
void
eqOgre::Pipe::_createWindow( void )
{
	std::cout << "eqOgre::Pipe::_createWindow" << std::endl;

	vl::EnvSettings::Node node = getNodeConf();
	assert( node.getNWindows() > 0 );
	_window = new eqOgre::Window( node.getWindow(0).name, this );
	assert( _window );
}
