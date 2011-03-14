/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-01
 *	@file pipe.hpp
 *
 */

#ifndef VL_PIPE_HPP
#define VL_PIPE_HPP

#include "eq_ogre/ogre_root.hpp"
#include "base/envsettings.hpp"
#include "player.hpp"
#include "scene_manager.hpp"

#include "cluster/client.hpp"
#include "session.hpp"
#include "settings.hpp"
#include "gui/gui.hpp"

#include "logger.hpp"

#include <CEGUI/CEGUIWindow.h>


namespace vl
{

class Window;

/**	@class Pipe
 *	@brief Representation of a pipe i.e. a single GPU
 *
 *	A Pipe is always run on single GPU.
 *	Pipe objects are executed completely in a separate threads from each other
 *	and from the application thread.
 *	At the moment the application supports only a single Pipe objects because
 *	of the limitations in Ogre Design.
 */
class Pipe : public vl::Session
{
public :
	Pipe( std::string const &name,
		  std::string const &server_address,
		  uint16_t server_port );

	virtual ~Pipe( void );

	vl::EnvSettingsRefPtr getEnvironment( void )
	{ return _env; }

	vl::Settings const &getProject( void ) const
	{ return _settings; }

	vl::EnvSettings::Node getNodeConf( void );

	vl::EnvSettings::Window getWindowConf( std::string const &window_name );

	vl::ogre::RootRefPtr getRoot( void )
	{ return _root; }

	Ogre::SceneManager *getSceneManager( void )
	{ return _ogre_sm; }

	Ogre::Camera *getCamera( void )
	{ return _camera; }

	vl::Player const &getPlayer( void ) const
	{ return *_player; }

	/// Send a message to the Master
	/// Does not destroy the message so user is responsible for destroying it.
	void sendMessageToMaster( vl::cluster::Message *msg );

	void sendEvent( vl::cluster::EventData const &event );

	void sendCommand( std::string const &cmd );

	/// Boost thread operator
	void operator()();

	std::string const &getName( void ) const
	{ return _name; }

	typedef std::vector<vl::ProjSettingsRefPtr> ProjectList;

	bool isRunning( void )
	{ return _running; }

	bool guiShown( void ) const
	{
		if( !_gui )
		{ return false; }
		return _gui->shown();
	}

	CEGUI::Window *getEditor( void )
	{ return _editor; }

	CEGUI::Window *getConsole( void )
	{ return _console; }

	void printToConsole(std::string const &text, double time,
						std::string const &type = std::string(),
						vl::LOG_MESSAGE_LEVEL lvl = vl::LML_NORMAL);

protected :
	/// Reload the projects
	void _reloadProjects( vl::Settings const &set );

	/// GUI specific
	void _initGUI( void );
	void _initGUIResources( vl::Settings const &set );
	void _addGUIResourceGroup( std::string const &name, fs::path const &path );
	void _createGUI( void );

	/// Ogre helpers
	void _createOgre( void );
	void _loadScene( vl::ProjSettings::Scene const &scene );
	void _setCamera( void );

	/// message passing
	void _handleMessages( void );
	void _handleMessage( vl::cluster::Message *msg );
	void _handleCreateMsg( vl::cluster::Message *msg );
	void _handleUpdateMsg( vl::cluster::Message *msg );
	void _handlePrintMsg( vl::cluster::Message *msg );

	/// Distribution helpers
	/// Syncs to the master copy stored when an Update message was received
	void _syncData( void );
	/// Updates command data, uses versioning to and indexes to provide an
	/// Commands that can be sent from Master thread
	void _updateDistribData( void );

	/// Rendering helpers
	/// Will render all the windows
	void _draw( void );
	/// Will swap all the windows
	void _swap( void );

	/// Input events
	void _sendEvents( void );

	void _createWindow( vl::EnvSettings::Window const &winConf );

	void _shutdown( void );

	/**	@todo should write the screenshot to the project directory not
	 *	to current directory
	 *	Add the screenshot dir to DistributedSettings
	 *	@todo the format of the screenshot name should be
	 *	screenshot_{project_name}-{year}-{month}-{day}-{time}-{window_name}.png
	 */
	void _takeScreenshot( void );

	std::string _name;

	/// EnvSettings mapped from Master
	vl::EnvSettingsRefPtr _env;
	/// ProjectSettings mapped from Master
	vl::Settings _settings;

	/// Ogre data
	vl::ogre::RootRefPtr _root;
	Ogre::SceneManager *_ogre_sm;
	Ogre::Camera *_camera;


	/// Distributed data
	vl::SceneManagerPtr _scene_manager;
	vl::Player *_player;
	std::string _active_camera_name;
	uint32_t _screenshot_num;

	vl::cluster::ClientRefPtr _client;
	std::vector<vl::cluster::ObjectData> _objects;

	/// Input events to be sent
	std::vector<vl::cluster::EventData> _events;

	std::vector<vl::Window *> _windows;

	/// GUI related
	vl::gui::GUI *_gui;

	CEGUI::Window *_console;
	CEGUI::Window *_editor;
	CEGUI::Window *_loading_screen;
	CEGUI::Window *_stats;

	bool _running;

};	// class Pipe

class PipeThread
{
public :
	PipeThread( std::string name, std::string server_address, uint16_t server_port )
		: _name(name), _server_address(server_address), _server_port(server_port)
	{}

	/// Boost thread operator
	void operator()();

private :
	std::string _name;
	std::string _server_address;
	uint16_t _server_port;

};	// class PipeThread

}	// namespace vl

#endif // VL_PIPE_HPP
