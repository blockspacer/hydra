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

#include "base/typedefs.hpp"

#include "tracker.hpp"
#include <eq/client/observer.h>

// TODO this should not need to be included here
#include "tracker_serializer.hpp"

// Audio
#include <cAudio/cAudio.h>

namespace eqOgre
{
	class PythonContext;
}

namespace vl
{

class GameManager
{
public :
	GameManager( void );

	virtual ~GameManager( void );

	eqOgre::PythonContextPtr getPython( void );

	PlayerPtr getPlayer( void );

	ResourceManagerPtr getReourceManager( void );

	EventManagerPtr getEventManager( void );

	eqOgre::SceneManagerPtr getSceneManager( void );

	PlayerPtr createPlayer( eq::Observer *observer );

	void toggleBackgroundSound( void );

	void quit( void );

	bool step( void );

	vl::ClientsRefPtr getTrackerClients( void )
	{ return _trackers; }

	void createBackgroundSound( std::string const &name );

private :
	/// Non copyable
	GameManager( GameManager const &);
	GameManager & operator=( GameManager const &);

	eqOgre::PythonContextPtr _python;
	ResourceManagerPtr _resource_man;
	EventManagerPtr _event_man;
	eqOgre::SceneManagerPtr _scene_manager;

	PlayerPtr _player;

	/// Tracking
	vl::ClientsRefPtr _trackers;


	/// Audio objects
	cAudio::IAudioManager *_audio_manager;
	cAudio::IAudioSource *_background_sound;

	/// State
	bool _quit;

};	// class GameManager

}	// namespace vl

#endif // VL_GAME_MANAGER_HPP