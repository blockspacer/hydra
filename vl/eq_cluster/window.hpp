/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-01
 *	@file window.hpp
 *
 */

#ifndef VL_WINDOW_HPP
#define VL_WINDOW_HPP

#include "player.hpp"
#include "typedefs.hpp"

// Necessary for Window config and Wall
#include "base/envsettings.hpp"

#include <OIS/OISEvents.h>
#include <OIS/OISInputManager.h>
#include <OIS/OISKeyboard.h>
#include <OIS/OISMouse.h>
#include <OIS/OISJoyStick.h>

#include <OGRE/OgreRenderWindow.h>

// GUI
#include <CEGUI/CEGUIEventArgs.h>

#include "renderer_interface.hpp"

namespace vl
{

/**	@class Window represent an OpenGL drawable and context
 *
 */
class Window : public OIS::KeyListener, public OIS::MouseListener, public OIS::JoyStickListener
{
public:
	/// @TODO change name to window config
	/// pass Renderer as parent, not ref ptr because this shouldn't hold ownership
	Window( std::string const &name, vl::RendererInterface *parent );

	virtual ~Window( void );

	Ogre::RenderWindow *getRenderWindow( void )
	{ return _ogre_window; }

	vl::EnvSettingsRefPtr getEnvironment( void );

	vl::Player const &getPlayer( void ) const;

	vl::ogre::RootRefPtr getOgreRoot( void );

	void setCamera( Ogre::Camera *camera );

	std::string const &getName( void ) const
	{ return _name; }

	void takeScreenshot( std::string const &prefix, std::string const &suffix );

	void setIPD(double ipd)
	{ _ipd = ipd; }

	/// @brief Get wether hardware stereo is enabled or not
	/// @return true if the window has stereo enabled
	bool hasStereo(void) const;

	/// Capture input events
	virtual void capture( void );

	/// OIS callback overrides
	bool keyPressed(const OIS::KeyEvent &key);
	bool keyReleased(const OIS::KeyEvent &key);

	bool mouseMoved(const OIS::MouseEvent &evt);
	bool mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID id);
	bool mouseReleased(const OIS::MouseEvent &evt, OIS::MouseButtonID id);

	bool buttonPressed(OIS::JoyStickEvent const &evt, int button);
	bool buttonReleased(OIS::JoyStickEvent const &evt, int button);
	bool axisMoved(OIS::JoyStickEvent const &evt, int axis);
	bool povMoved(OIS::JoyStickEvent const &evt, int pov);
	bool vector3Moved(OIS::JoyStickEvent const &evt, int index);

	/// GECUI callbacks

	/// Editor events
	/// Menubar buttons clicked
	bool onNewClicked( CEGUI::EventArgs const &e );
	bool onOpenClicked( CEGUI::EventArgs const &e );
	bool onSaveClicked( CEGUI::EventArgs const &e );
	bool onQuitClicked( CEGUI::EventArgs const &e );
	bool onResetClicked( CEGUI::EventArgs const &e );
	bool onImportSceneClicked( CEGUI::EventArgs const &e );
	bool onReloadScenesClicked( CEGUI::EventArgs const &e );
	bool onNewScriptClicked( CEGUI::EventArgs const &e );
	bool onAddScriptClicked( CEGUI::EventArgs const &e );
	bool onReloadScriptsClicked( CEGUI::EventArgs const &e );

	/// Checkboxes
	bool onShowAxisChanged( CEGUI::EventArgs const &e );
	bool onShowNamesChanged( CEGUI::EventArgs const &e );
	bool onShowJointsChanged( CEGUI::EventArgs const &e );

	/// Instruct the Channels to draw the Scene
	virtual void draw( void );

	/// Swap the back buffer to front
	virtual void swap( void );

	void createGUIWindow( void );

protected :
	Ogre::RenderWindow *_createOgreWindow( vl::EnvSettings::Window const &winConf );

	/// Create the OIS input handling
	/// For now supports mouse and keyboard
	/// @TODO add joystick support
	void _createInputHandling( void );

	void _printInputInformation( void );

	void _sendEvent( vl::cluster::EventData const &event );

	std::string _name;

	vl::RendererInterface *_renderer;

	vl::EnvSettings::Wall _wall;

	double _ipd;

	// Ogre
	Ogre::RenderWindow *_ogre_window;
	Ogre::Camera *_camera;
	Ogre::Viewport *_left_viewport;
	Ogre::Viewport *_right_viewport;

	// OIS variables
	OIS::InputManager *_input_manager;
	OIS::Keyboard *_keyboard;
	OIS::Mouse *_mouse;
	std::vector<OIS::JoyStick *> _joysticks;

};	// class Window

}	// namespace vl

#endif // VL_WINDOW_HPP
