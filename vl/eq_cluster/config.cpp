/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-10
 *
 */

#include "config.hpp"

#include "vrpn_tracker.hpp"
#include "fake_tracker.hpp"

#include "math/conversion.hpp"

#include "config_events.hpp"

#include "dotscene_loader.hpp"

#include "tracker_serializer.hpp"
#include "base/filesystem.hpp"

#include <OIS/OISKeyboard.h>
#include <OIS/OISMouse.h>

#include "resource.hpp"

// TODO this is for testing the audio moving to Client
#include "client.hpp"

#include "game_manager.hpp"

eqOgre::Config::Config( eq::base::RefPtr< eq::Server > parent )
	: eq::Config ( parent )
// 	_event_manager( new vl::EventManager ),
// 	  _resource_manager(0), _python(0)
{}

eqOgre::Config::~Config()
{}

bool
eqOgre::Config::init( eq::uint128_t const & )
{
//	_python = new PythonContext(this, _event_manager);

	_loadScenes();

	// Create Tracker needs the SceneNodes for mapping
	_createTracker(_settings);

	std::vector<std::string> scripts = _settings->getScripts();

	EQINFO << "Running " << scripts.size() << " python scripts." << std::endl;

	for( size_t i = 0; i < scripts.size(); ++i )
	{
		// Run init python scripts
		vl::TextResource script_resource;
		_game_manager->getReourceManager()->loadResource( scripts.at(i), script_resource );
		_game_manager->getPython()->executePythonScript( script_resource );
	}

	_createQuitEvent();
	_createTransformToggle();

	EQINFO << "Registering data." << std::endl;

	EQASSERT( _game_manager->getSceneManager() );

	if( !_game_manager->getSceneManager()->registerData(this) )
	{ return false; }

	eqOgre::ResourceManager *res_man = static_cast<eqOgre::ResourceManager *>( _game_manager->getReourceManager() );
	if( !registerObject( res_man ) )
	{ return false; }

	_distrib_settings.setFrameDataID( _game_manager->getSceneManager()->getID() );
	_distrib_settings.setResourceManagerID( res_man->getID() );

	EQINFO << "Registering Settings" << std::endl;
	if( !registerObject( &_distrib_settings ) )
	{ return false; }

	if( !eq::Config::init( _distrib_settings.getID() ) )
	{ return false; }

	EQINFO << "Config::init DONE" << std::endl;

	return true;
}

bool
eqOgre::Config::exit( void )
{
	// First let the children clean up
	bool retval = eq::Config::exit();

	EQINFO << "Deregistering distributed data." << std::endl;

	_game_manager->getSceneManager()->deregisterData();
	_distrib_settings.setFrameDataID( eq::base::UUID::ZERO );

	eqOgre::ResourceManager *res_man =
		static_cast<eqOgre::ResourceManager *>( _game_manager->getReourceManager() );
	deregisterObject( res_man );
	_distrib_settings.setResourceManagerID( eq::base::UUID::ZERO );

	deregisterObject( &_distrib_settings );

	EQINFO << "Config exited." << std::endl;
	return retval;
}

void
eqOgre::Config::setSettings( vl::SettingsRefPtr settings )
{
	EQASSERT( settings );
	EQASSERT( _game_manager );

	_settings = settings;
	// TODO fix the interface
	_distrib_settings.copySettings(_settings, _game_manager->getReourceManager() );
}

eqOgre::SceneNodePtr
eqOgre::Config::createSceneNode(const std::string& name)
{
	SceneNodePtr node = SceneNode::create( name );
	_addSceneNode( node );
	return node;
}

// // TODO implement
// void
// eqOgre::Config::removeSceneNode(eqOgre::SceneNodePtr node)
// {
// 	BOOST_THROW_EXCEPTION( vl::not_implemented() );
// }

eqOgre::SceneNode *
eqOgre::Config::getSceneNode(const std::string& name)
{
	return _game_manager->getSceneManager()->getSceneNode(name);
}

// vl::TrackerTrigger *
// eqOgre::Config::getTrackerTrigger(const std::string& name)
// {
//
// }


// TODO implement
// void
// eqOgre::Config::resetScene( void )
// {
// 	BOOST_THROW_EXCEPTION( vl::not_implemented() );
// }


// void
// eqOgre::Config::toggleBackgroundSound()
// {
// 	eqOgre::Client *client = dynamic_cast<eqOgre::Client *> ( getClient().get() );
// 	EQASSERT( client )
// 	if( client )
// 	{
// 		client->toggleBackgroundSound();
// 	}
// }

void eqOgre::Config::setGameManager(vl::GameManagerPtr man)
{
	EQASSERT( man );
	_game_manager = man;
	vl::PlayerPtr player = _game_manager->createPlayer( getObservers().at(0) );
	EQASSERT( player );
}


uint32_t
eqOgre::Config::startFrame( eq::uint128_t const &frameID )
{
	// ProcessEvents does not store the pointer anywhere
	// so it's safe to allocate to the stack
	vl::FrameTrigger frame_trig;
	_game_manager->getEventManager()->processEvents( &frame_trig );

	if( !_game_manager->step() )
	{ stopRunning(); }

	eq::uint128_t version = _game_manager->getSceneManager()->commitAll();

	return eq::Config::startFrame( version );
}


/// ------------ Private -------------

void
eqOgre::Config::_addSceneNode(eqOgre::SceneNode* node)
{
	// Check that no two nodes have the same name
	// TODO should be in the frame data, as it can neither store multiple
	// SceneNodes with same names
	for( size_t i = 0; i < _game_manager->getSceneManager()->getNSceneNodes(); ++i )
	{
		SceneNodePtr ptr = _game_manager->getSceneManager()->getSceneNode(i);
		if( ptr == node || ptr->getName() == node->getName() )
		{
			// TODO is this the right exception?
			BOOST_THROW_EXCEPTION( vl::duplicate() );
		}
	}

	_game_manager->getSceneManager()->addSceneNode( node );
}

void
eqOgre::Config::_createTracker( vl::SettingsRefPtr settings )
{
	EQINFO << "Creating Trackers." << std::endl;

	vl::ClientsRefPtr clients( new vl::Clients );
	std::vector<std::string> tracking_files = settings->getTrackingFiles();

	EQINFO << "Processing " << tracking_files.size() << " tracking files."
		<< std::endl;

	for( std::vector<std::string>::const_iterator iter = tracking_files.begin();
		 iter != tracking_files.end(); ++iter )
	{
		// Read a file
		EQINFO << "Copy tracking resource : " << *iter << std::endl;

		vl::TextResource resource;
		_game_manager->getReourceManager()->loadResource( *iter, resource );

		vl::TrackerSerializer ser( clients );
		ser.parseTrackers(resource);
	}

	// Start the trackers
	EQINFO << "Starting " << clients->getNTrackers() << " trackers." << std::endl;
	for( size_t i = 0; i < clients->getNTrackers(); ++i )
	{
		clients->getTracker(i)->init();
	}

	// set the trackers to GameManager
	_game_manager->setTrackerClients( clients );

	// Create Action
	eqOgre::HeadTrackerAction *action = (eqOgre::HeadTrackerAction *)_game_manager->getEventManager()->createAction("HeadTrackerAction");
	EQASSERT( _game_manager->getPlayer() );
	action->setPlayer( _game_manager->getPlayer() );

	// This will get the head sensor if there is one
	// If not it will create a FakeTracker instead
	vl::TrackerTrigger *head_trigger = _game_manager->getTrackerTrigger( "glassesTrigger" );
	if( head_trigger )
	{
		head_trigger->setAction( action );
	}
	else
	{
		EQINFO << "Creating a fake head tracker" << std::endl;
		vl::TrackerRefPtr tracker( new vl::FakeTracker );
		vl::SensorRefPtr sensor( new vl::Sensor );
		sensor->setDefaultPosition( Ogre::Vector3(0, 1.5, 0) );

		// Create the trigger
		EQINFO << "Creating a fake head tracker trigger" << std::endl;
		head_trigger = (vl::TrackerTrigger *)_game_manager->getEventManager()->createTrigger("TrackerTrigger");
		head_trigger->setName("glassesTrigger");
		head_trigger->setAction( action );
		sensor->setTrigger( head_trigger );

		EQINFO << "Adding a fake head tracker" << std::endl;
		// Add the tracker
		tracker->setSensor( 0, sensor );
		clients->addTracker(tracker);
	}
	EQINFO << "Trackers created." << std::endl;
}

void
eqOgre::Config::_loadScenes(void )
{
	EQINFO << "Loading Scenes for Project : " << _settings->getProjectName()
		<< std::endl;

	// Get scenes
	std::vector<vl::ProjSettings::Scene> scenes = _settings->getScenes();

	// If we don't have Scenes there is no point loading them
	if( !scenes.size() )
	{
		EQINFO << "Project does not have any scene files." << std::endl;
		return;
	}
	else
	{
		EQINFO << "Project has " << scenes.size() << " scene files."
			<< std::endl;
	}

	// Clean up old scenes
	// TODO this should be implemented

	// TODO support for multiple scene files should be tested
	// TODO support for case needs to be tested
	for( size_t i = 0; i < scenes.size(); ++i )
	{
		std::string scene_file_name = scenes.at(i).getName();

		EQINFO << "Loading scene file = " << scene_file_name << std::endl;

		vl::TextResource resource;
		_game_manager->getReourceManager()->loadResource( scenes.at(i).getFile(), resource );

		vl::DotSceneLoader loader;
		// TODO pass attach node based on the scene
		// TODO add a prefix to the SceneNode names ${scene_name}/${node_name}
		loader.parseDotScene( resource, this );

		EQINFO << "Scene " << scene_file_name << " loaded." << std::endl;
	}
}

void
eqOgre::Config::_createQuitEvent(void )
{
	EQINFO << "Creating QuitEvent" << std::endl;

	// Add a trigger event to Quit the Application
	EQASSERT( _game_manager );
	QuitAction *quit
		= (QuitAction *)( _game_manager->getEventManager()->createAction( "QuitAction" ) );
	quit->setGame( _game_manager );
	vl::Event *event = _game_manager->getEventManager()->createEvent( "Event" );
	event->setAction(quit);
	// Add trigger
	vl::KeyTrigger *trig = (vl::KeyTrigger *)( _game_manager->getEventManager()->createTrigger( "KeyTrigger" ) );
	trig->setKey( OIS::KC_ESCAPE );
	event->addTrigger(trig);
	_game_manager->getEventManager()->addEvent( event );
}

void
eqOgre::Config::_createTransformToggle(void )
{
	EQINFO << "Creating TransformToggle " << " Not in use atm." << std::endl;

	// Find ogre event so we can toggle it on/off
	// TODO add function to find Events
	// Ogre Rotation event, used to toggle the event on/off
	/*	FIXME new design
	TransformationEvent ogre_event;
	for( size_t i = 0; i < _trans_events.size(); ++i )
	{
		SceneNodePtr node = _trans_events.at(i).getSceneNode();
		if( node )
		{
			if( node->getName() == "ogre" )
			{
				ogre_event = _trans_events.at(i);
				break;
			}
		}
	}

	Trigger *trig = new KeyTrigger( OIS::KC_SPACE, false );
	Operation *add_oper = new AddTransformEvent( this, ogre_event );
	Operation *rem_oper = new RemoveTransformEvent( this, ogre_event );
	Event *event = new ToggleEvent( hasEvent(ogre_event), add_oper, rem_oper, trig );
	_events.push_back( event );
	*/
}


/// Event Handling
bool
eqOgre::Config::handleEvent( const eq::ConfigEvent* event )
{

	bool redraw = false;
	switch( event->data.type )
	{
		case eq::Event::KEY_PRESS :

			redraw = _handleKeyPressEvent(event->data.keyPress);
			break;

		case eq::Event::KEY_RELEASE :
			redraw = _handleKeyReleaseEvent(event->data.keyRelease);

			break;

		case eq::Event::POINTER_BUTTON_PRESS:
			redraw = _handleMousePressEvent(event->data.pointerButtonPress);
			break;

		case eq::Event::POINTER_BUTTON_RELEASE:
			redraw = _handleMouseReleaseEvent(event->data.pointerButtonRelease);
			break;

		case eq::Event::POINTER_MOTION:
			redraw = _handleMouseMotionEvent(event->data.pointerMotion);
			break;

		case eq::Event::WINDOW_CLOSE :
		case eq::Event::WINDOW_HIDE :
		case eq::Event::WINDOW_EXPOSE :
		case eq::Event::WINDOW_RESIZE :
		case eq::Event::WINDOW_SHOW :
			break;

		default :
			break;
	}

	return redraw;
}

bool
eqOgre::Config::_handleKeyPressEvent( const eq::KeyEvent& event )
{
	vl::KeyPressedTrigger trig;
	trig.setKey( (OIS::KeyCode )(event.key) );
	return _game_manager->getEventManager()->processEvents( &trig );
}

bool
eqOgre::Config::_handleKeyReleaseEvent(const eq::KeyEvent& event)
{
	vl::KeyReleasedTrigger trig;
	trig.setKey( (OIS::KeyCode )(event.key) );
	return _game_manager->getEventManager()->processEvents( &trig );
}

bool
eqOgre::Config::_handleMousePressEvent(const eq::PointerEvent& event)
{
//	std::cerr << "Config received mouse button press event. Button = "
//		<< event->data.pointer.button << std::endl;
	return false;
}

bool
eqOgre::Config::_handleMouseReleaseEvent(const eq::PointerEvent& event)
{
//	std::cerr << "Config received mouse button release event. Button = "
//		<< event->data.pointer.button << std::endl;
	return false;
}

bool
eqOgre::Config::_handleMouseMotionEvent(const eq::PointerEvent& event)
{
	return true;
}

bool
eqOgre::Config::_handleJoystickEvent(const eq::MagellanEvent& event)
{
	return false;
}
