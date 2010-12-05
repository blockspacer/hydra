#include "client.hpp"

#include "eq_cluster/config.hpp"

// Necessary for vl::Settings
#include "settings.hpp"
// Necessary for vl::msleep
#include "base/sleep.hpp"

#include "game_manager.hpp"

eqOgre::Client::Client( vl::SettingsRefPtr settings )
	: _settings(settings),
	  _game_manager(new vl::GameManager ),
	  _config(0)
{
	EQASSERT( settings );
}

eqOgre::Client::~Client(void )
{}


bool
eqOgre::Client::run(void )
{
	// 1. connect to server
	_server = new eq::Server;
	if( !connectServer( _server ))
	{
		EQERROR << "Can't open server" << std::endl;
		return false;
	}

	// 2. choose config
	eq::ConfigParams configParams;
	eq::Config* config = _server->chooseConfig( configParams );

	if( !config )
	{
		EQERROR << "No matching config on server" << std::endl;
		disconnectServer( _server );
		return false;
	}

	// 3. init config
	if( !_init( config ) )
	{ return false; }

	_clock.reset();
	_rendering_time = 0;
	// 4. run main loop
	uint32_t frame = 0;
	while( _config->isRunning() )
    {
		_render( ++frame );
    }

	_exit();

	return true;
}





bool
eqOgre::Client::_init( eq::Config *config )
{
	_config = static_cast<Config *>(config);
	EQASSERT( dynamic_cast<Config *>(config) );

	/// Set parameters to the config
	_config->setGameManager( _game_manager );
	_createResourceManager();
	_config->setSettings( _settings );

	/// TODO Register data here

	if( !_config->init(0) )
	{
		_server->releaseConfig( config );
		disconnectServer( _server );
		return false;
	}

	std::string song_name("The_Dummy_Song.ogg");
	_game_manager->createBackgroundSound(song_name);

	return true;
}

void
eqOgre::Client::_exit(void )
{
// 	_exitAudio();

	EQINFO << "Exiting the config." << std::endl;
	// 5. exit config
	_config->exit();

	EQINFO << "Releasing config." << std::endl;
	// 6. cleanup and exit
	_server->releaseConfig( _config );
	EQINFO << "Disconnecting from Server" << std::endl;
	if( !disconnectServer( _server ))
	{ EQERROR << "Client::disconnectServer failed" << std::endl; }

	_server = 0;

}

void eqOgre::Client::_render( uint32_t frame )
{
	_frame_clock.reset();

	_config->startFrame(frame);
	_config->finishFrame();

	_rendering_time += _frame_clock.getTimed();
	// Sleep enough to get a 60 fps but no more
	// TODO the fps should be configurable
	vl::msleep( 16.66-_frame_clock.getTimed() );

	// Print info every two hundred frame
	if( (frame % 200) == 0 )
	{
		// TODO the logging should probably go to stats file and optionally
		// to the console
		// TODO also there should be possibility to reset the clock
		// for massive parts in a scene for example
		std::cout << "Avarage fps = " << frame/(_clock.getTimed()/1000)
			<< ". took " << _rendering_time/frame
			<< " ms in avarage for rendering one frame."
			<< " Took " << _clock.getTimed() << "ms to render "
			<< frame << " frames." << std::endl;
	}
}

void
eqOgre::Client::_createResourceManager(void )
{
	EQINFO << "Initialising Resource Manager" << std::endl;

	EQINFO << "Adding project directories to resources. "
		<< "Only project directory and global directory is added." << std::endl;

	_game_manager->getReourceManager()->addResourcePath( _settings->getProjectDir() );
	_game_manager->getReourceManager()->addResourcePath( _settings->getGlobalDir() );

	// TODO add case directory

	// Add environment directory, used for tracking configurations
	EQINFO << "Adding ${environment}/tracking to the resources paths." << std::endl;
	std::string tracking_dir( _settings->getEnvironementDir() + "/tracking" );
	_game_manager->getReourceManager()->addResourcePath( tracking_dir );
}
