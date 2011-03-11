/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-12
 *
 *	Game Manager
 *	Manages all the game manager and owns them
 *	Has methods to retrieve them
 *	Has methods to reset the game
 *	Manages the SoundManager
 *	Has methods to set background sound
 *	Has methods to toggle background sound on/off
 *
 *	Later
 *	Manages the active SceneManager
 *	Manages the physics world
 *
 *	Non copyable
 */

#ifndef VL_GAME_MANAGER_HPP
#define VL_GAME_MANAGER_HPP

#include "typedefs.hpp"

#include "tracker.hpp"

// TODO this should not need to be included here
#include "tracker_serializer.hpp"

#include "session.hpp"

#include "gui/gui.hpp"

// Audio
#include <cAudio/cAudio.h>

#include "logger.hpp"

namespace vl
{

class GameManager
{
public :
	GameManager(vl::Logger *logger);

	virtual ~GameManager( void );

	void createSceneManager( vl::Session *session );

	vl::PythonContextPtr getPython( void );

	PlayerPtr getPlayer( void );

	ResourceManagerPtr getReourceManager( void );

	EventManagerPtr getEventManager( void );

	vl::SceneManagerPtr getSceneManager( void );

	PlayerPtr createPlayer( void );

	void setGUI( vl::gui::GUI *gui )
	{ _gui = gui; }

	vl::gui::GUI *getGUI( void )
	{ return _gui; }

	void toggleBackgroundSound( void );

	void quit( void );

	bool step( void );

	vl::ClientsRefPtr getTrackerClients( void )
	{ return _trackers; }

	void createBackgroundSound( std::string const &name );

	vl::Logger *getLogger(void)
	{ return _logger; }

private :
	/// Non copyable
	GameManager( GameManager const &);
	GameManager & operator=( GameManager const &);

	vl::PythonContextPtr _python;
	vl::ResourceManagerPtr _resource_man;
	vl::EventManagerPtr _event_man;
	vl::SceneManagerPtr _scene_manager;

	PlayerPtr _player;

	/// Tracking
	vl::ClientsRefPtr _trackers;

	vl::gui::GUI *_gui;

	/// Audio objects
	cAudio::IAudioManager *_audio_manager;
	cAudio::IAudioSource *_background_sound;

	vl::Logger *_logger;

	/// State
	bool _quit;

};	// class GameManager

}	// namespace vl

#endif // VL_GAME_MANAGER_HPP
