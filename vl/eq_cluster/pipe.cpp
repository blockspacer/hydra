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

char const *SERVER_NAME = "localhost";
uint16_t const SERVER_PORT = 4699;

/// ------------------------- Public -------------------------------------------
eqOgre::Pipe::Pipe( eq::Node* parent )
	: eq::Pipe(parent), _ogre_sm(0), _camera(0), _screenshot_num(0), _client(0)
{}

eqOgre::Pipe::~Pipe( void )
{}

eqOgre::DistributedSettings const &
eqOgre::Pipe::getSettings( void ) const
{
	return _settings;
}


/// ------------------------ Protected -----------------------------------------
bool
eqOgre::Pipe::configInit(const eq::uint128_t& initID)
{
	_createClient();

	if( !eq::Pipe::configInit(initID) )
	{
		// Error
		// TODO move to using Ogre Logging, the log manager needs to be created sooner
		std::string message("eq::Pipe::configInit failed");
		std::cerr << message << std::endl;
		return false;
	}

	_mapData( initID );

	_handleMessages();

	_syncData();

	if( !_createOgre() )
	{
		// Error
		// TODO move to using Ogre Logging, the log manager needs to be created sooner
		std::string message("eqOgre::Pipe::configInit : createOgre failed");
		std::cerr << message << std::endl;
		return( false );
	}

	return true;
}

bool
eqOgre::Pipe::configExit()
{
	bool retval = eq::Pipe::configExit();

	// Info
	std::string message("Cleaning out OGRE");
	Ogre::LogManager::getSingleton().logMessage( message );
	_root.reset();

	return retval;
}

void
eqOgre::Pipe::frameStart(const eq::uint128_t& frameID, const uint32_t frameNumber)
{
	static bool inited = false;

	_handleMessages();

	try {
		// TODO should update the Pipe data in init
		// FIXME this updates the SceneManager and tries to find Ogre nodes
		// but it doesn't have the SceneManager as it's created in the init
		// next.
		_updateDistribData();

		// Init the Ogre resources and load a Scene
		// These are here because the RenderWindow needs to be created before
		// loading Meshes
		if( !inited )
		{
			// Resource registration
			std::vector<std::string> const &resources = _resource_manager.getResourcePaths();
			for( size_t i = 0; i < resources.size(); ++i )
			{
				_root->addResource( resources.at(i) );
			}
			_root->setupResources( );
			_root->loadResources();

			_ogre_sm = _root->createSceneManager("SceneManager");

			if( !_loadScene() )
			{
				// Error
				std::string message("Problem loading the Scene");
				Ogre::LogManager::getSingleton().logMessage( message, Ogre::LML_CRITICAL );
			}

			// We need to find the node from scene graph
			std::string message = "SceneManager has "
				+ vl::to_string(_scene_manager->getNSceneNodes()) + " SceneNodes.";
			Ogre::LogManager::getSingleton().logMessage( message );

			EQASSERTINFO( _ogre_sm, "Window has no Ogre SceneManager" );
			if( !_scene_manager->setSceneManager( _ogre_sm ) )
			{
				// Error
				message = "Some SceneNodes were not found.";
				Ogre::LogManager::getSingleton().logMessage( message, Ogre::LML_CRITICAL );
			}

			inited = true;
		}
	}
	catch( vl::exception &e )
	{
		// TODO add error status flag
		std::cout << "VL Exception : " + boost::diagnostic_information<>(e) << std::endl;
		EQASSERT( false );
	}
	catch( ... )
	{
		EQASSERT( false );
	}

	eq::Pipe::frameStart( frameID, frameNumber );
}


/// Ogre helpers
bool
eqOgre::Pipe::_createOgre( void )
{
	try {
		// TODO needs LogManager creation before this
		std::string message("Creating Ogre Root");
		std::cout << message << std::endl;

		_root.reset( new vl::ogre::Root( getSettings().getOgreLogFilePath() ) );
		// Initialise ogre
		_root->createRenderSystem();
	}
	catch( vl::exception &e )
	{
		std::string message = "VL Exception : " + boost::diagnostic_information<>(e);
		Ogre::LogManager::getSingleton().logMessage( message, Ogre::LML_CRITICAL );

		return false;
	}
	catch( Ogre::Exception const &e)
	{
		std::string message = std::string("Ogre Exception: ") + e.what();
		Ogre::LogManager::getSingleton().logMessage( message, Ogre::LML_CRITICAL );

		return false;
	}
	catch( std::exception const &e )
	{
		std::string message = std::string("STD Exception: ") + e.what();
		Ogre::LogManager::getSingleton().logMessage( message, Ogre::LML_CRITICAL );

		return false;
	}
	catch( ... )
	{
		std::string err_msg( "eqOgre::Window::configInit : Exception thrown." );
		Ogre::LogManager::getSingleton().logMessage( err_msg, Ogre::LML_CRITICAL );
		return false;
	}

	return true;
}

bool
eqOgre::Pipe::_loadScene( void )
{
	// TODO this should be divided to case load scenes function and loadScene function

	// Get scenes
	std::vector<vl::TextResource> scenes = _resource_manager.getSceneResources();
	std::string message = "Loading Scenes for Project : " + getSettings().getProjectName();
	Ogre::LogManager::getSingleton().logMessage( message );

	// If we don't have Scenes there is no point loading them
	if( scenes.empty() )
	{
		message = "Project does not have any scene files.";
		Ogre::LogManager::getSingleton().logMessage( message, Ogre::LML_CRITICAL );
		return false;
	}
	else
	{
		message = "Project has " + vl::to_string(scenes.size()) + " scene files.";
		Ogre::LogManager::getSingleton().logMessage( message );
	}

	// Clean up old scenes
	// TODO this should be a loader not a destroyer, move to another function
	_ogre_sm->clearScene();
	_ogre_sm->destroyAllCameras();

	// TODO support for multiple scene files should be tested
	// TODO support for case needs to be tested
	for( size_t i = 0; i < scenes.size(); ++i )
	{
		std::string const &name = scenes.at(i).getName();

		message = "Loading scene " + name + ".";
		Ogre::LogManager::getSingleton().logMessage( message );

		eqOgre::DotSceneLoader loader;
		// TODO pass attach node based on the scene
		// TODO add a prefix to the SceneNode names ${scene_name}/${node_name}
		loader.parseDotScene( scenes.at(i), _ogre_sm );

		message = "Scene " + name + " loaded.";
		Ogre::LogManager::getSingleton().logMessage( message );
	}

	/// Get the camera
	// TODO move to separate function

	// Loop through all cameras and grab their name and set their debug representation
	Ogre::SceneManager::CameraIterator cameras = _ogre_sm->getCameraIterator();

	// Grab the first available camera, for now
	if( cameras.hasMoreElements() )
	{
		_camera = cameras.getNext();
		message = "Using Camera " + _camera->getName()
			+ " found from the scene.";
		Ogre::LogManager::getSingleton().logMessage( message );
	}
	else
	{
		_camera = _ogre_sm->createCamera("Cam");
		message = "No camera in the scene. Using created camera "
			+ _camera->getName();
		Ogre::LogManager::getSingleton().logMessage( message );
	}
	_active_camera_name = _camera->getName();

	return true;
}


/// Distribution helpers
void
eqOgre::Pipe::_createClient( void )
{
	std::cout << "eqOgre::Pipe::_createClient" << std::endl;
	assert( !_client );

	// FIXME these should be configured in config file
	_client = new vl::cluster::Client( SERVER_NAME, SERVER_PORT );
	_client->registerForUpdates();
}

void
eqOgre::Pipe::_handleMessages( void )
{
	assert( _client );

	_client->mainloop();
	while( _client->messages() )
	{
//		std::cout << "eqOgre::Pipe::_syncData : Messages received" << std::endl;
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

// 	std::cout << "eqOgre::Pipe::_syncData : message = " << *msg
// 		<< std::endl;

	switch( msg->getType() )
	{
		case vl::cluster::MSG_UPDATE :
		{
			// TODO objects array should not be cleared but rather updated
			// so that objects that are not updated stay in the array.
			_objects.clear();
			// Read the IDs in the message and call pack on mapped objects
			// based on thoses
			// TODO multiple update messages in the same frame,
			// only the most recent should be used.
// 				std::cout << "Message = " << *msg << std::endl;
			while( msg->size() > 0 )
			{
// 				std::cout << "eqOgre::Pipe::_syncData : UPDATE message : "
//	 				<< "size = " << msg->size() << std::endl;

				vl::cluster::ObjectData data;
				data.copyFromMessage(msg);
				// Pushing back will create copies which is unnecessary
				_objects.push_back(data);
			}
// 			std::cout << "Message handled" << std::endl;
		}
		break;

		default :
			std::cout << "Unhandled Message of type = " << msg->getType()
				<< std::endl;
			delete msg;
			break;
	}
}

void
eqOgre::Pipe::_syncData( void )
{
// 	std::cout << "eqOgre::Pipe::_syncData : " << _objects.size() << " objects "
// 		<< std::endl;

	std::vector<vl::cluster::ObjectData>::iterator iter;
	// TODO remove the temporary array
	// use a custom structure that does not create temporaries
	// rather two phase system one to read the array and mark objects for delete
	// and second that really clear those that are marked for delete
	// similar system for reading data to the array
	std::vector<vl::cluster::ObjectData> tmp;
	for( iter = _objects.begin(); iter != _objects.end(); ++iter )
	{
		vl::cluster::ByteStream stream = iter->getStream();
		// TODO break this to two different parts
		// one creating the ObjectDatas and saving them to array
		// other one that processes this array and removes the elements found in mapped
		vl::Distributed *obj = findMappedObject( iter->getId() );
		if( obj )
		{
			obj->unpack(stream);
		}
		else
		{
			std::cerr << "No ID " << iter->getId() << " found in mapped objects."
				<< std::endl;
			tmp.push_back( *iter );
		}
	}

	_objects = tmp;

	if( _scene_manager )
	{ _scene_manager->finaliseSync(); }

// 	std::cout << "Sync finalised." << std::endl;
}

void
eqOgre::Pipe::_mapData( eq::uint128_t const &settingsID )
{
	// TODO move to using Ogre Logging, needs LogManager creation
	std::cout << "Mapping data." << std::endl;

	// Get the cluster version of data
	getConfig()->mapObject( &_settings, settingsID );
	EQASSERTINFO( _settings.getID() != eq::UUID::ZERO, "Couldn't map the Settings." );

	// TODO should be automatic when mapObjectC is called
	// Also if we call it so many times it should not iterate through the whole
	// message but rather already stored ID list

	uint64_t id = _settings.getResourceManagerID();
	EQASSERT( id != vl::ID_UNDEFINED );
	mapObjectC( &_resource_manager, id );

	EQASSERT( _settings.getPlayerID() != vl::ID_UNDEFINED );
	mapObjectC( &_player, _settings.getPlayerID() );

	uint64_t sm_id = _settings.getSceneManagerID();
	std::cout << "Mapping SceneManager with id = " << sm_id << '.' << std::endl;
	EQASSERT( sm_id != vl::ID_UNDEFINED );
	_scene_manager = new eqOgre::SceneManager( this );
	mapObjectC( _scene_manager, sm_id );

	EQINFO << "Data mapped." << std::endl;
}

void
eqOgre::Pipe::_updateDistribData( void )
{
	// Custom sync code
	_syncData();

	// Update player
	// Get active camera and change the rendering camera if there is a change
	std::string const &cam_name = _player.getActiveCamera();
	if( !cam_name.empty() && cam_name != _active_camera_name )
	{
		_active_camera_name = cam_name;
		if( _ogre_sm->hasCamera( cam_name ) )
		{
			// Tell the Windows to change cameras
			_camera = _ogre_sm->getCamera( _active_camera_name );
			Windows const &window_list = getWindows();
			for( size_t i = 0; i < window_list.size(); ++i )
			{
				EQASSERT( dynamic_cast<eqOgre::Window *>( window_list.at(i) ) );
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
		Windows const &window_list = getWindows();
		for( size_t i = 0; i < window_list.size(); ++i )
		{
			EQASSERT( dynamic_cast<eqOgre::Window *>( window_list.at(i) ) );
			eqOgre::Window *window =
				static_cast<eqOgre::Window *>( window_list.at(i) );
			window->takeScreenshot( prefix, suffix );
		}

		_screenshot_num = _player.getScreenshotVersion();
	}

	// FIXME this is completely screwed up.
	/*
	static uint32_t scene_version = 0;
	if( _frame_data.getSceneVersion() > scene_version )
	{
		// This will reload the scene but all transformations remain
		// As this will not reset the SceneNode structures that control the
		// transformations of objects.
		EQINFO << "Reloading the Ogre scene now" << std::endl;
		eqOgre::Window *win = static_cast<eqOgre::Window *>( getWindow() );
		win->loadScene();
		Ogre::Camera *camera = win->getCamera();
		createViewport( camera );
		_frame_data.setSceneManager( win->getSceneManager() );
		EQINFO << "Ogre Scene reloaded." << std::endl;

		scene_version = _frame_data.getSceneVersion();
	}
	*/
}
