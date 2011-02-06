/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2010-11
 *
 *	2010-11-29 Added camera rotations to env file
 *	moved ref ptr definition to typedefs.hpp
 *
 *	2011-01-06 Added wall and window to env file
 *	Added stereo and inter pupilar distance to env file
 */

#ifndef VL_ENVSETTINGS_HPP
#define VL_ENVSETTINGS_HPP

#include <string>
#include <vector>
#include <iostream>

#include <stdint.h>

#include "filesystem.hpp"
#include "rapidxml.hpp"
#include "typedefs.hpp"
#include "exceptions.hpp"

namespace vl
{

/// @enum CFG Configurations tokens for boolean values
enum CFG
{
	OFF,
	ON,
	REQUIRED,
};

/**	@class EnvSettings
 *	@brief Settings for the environment the program is run.
 *
 *	Includes configuration for the Windows, Walls, Tracking, Cluster, Stereo,
 *	and Server.
 */
class EnvSettings
{
public :
	struct Tracking
	{
		Tracking( std::string const &file_name, bool u = "true" )
			: file(file_name), use(u)
		{}

		bool operator==( Tracking const &other ) const
		{
			if( file == other.file )
			{ return true; }

			return false;
		}

		std::string file;
		bool use;
	};

	struct Wall
	{
		Wall( std::string const &nam, std::vector<double> b_left,
			  std::vector<double> b_right, std::vector<double> t_left )
			: name(nam), bottom_left(b_left),
			  bottom_right(b_right), top_left(t_left)
		{}

		// Default constructor to allow vector resize
		Wall( void )
		{}

		// Wether or not the Wall has been initialised
		bool empty( void ) const
		{
			return( name.empty() && bottom_left.empty()
					&& bottom_right.empty() && top_left.empty() );
		}

		// Name of the wall
		std::string name;

		// Bottom left coordinates for this wall
		std::vector<double> bottom_left;
		// Bottom right coordinates for this wall
		std::vector<double> bottom_right;
		// Top left coordinates for this wall
		std::vector<double> top_left;

	};	// struct Wall

	struct Channel
	{
		Channel( std::string const &nam, std::string const &wall )
			: name(nam), wall_name(wall)
		{}

		/// Default constructor for vector resizes
		Channel( void )
		{}

		bool empty( void ) const
		{ return( name.empty() && wall_name.empty() ); }

		std::string name;

		// Name of the wall used for this window
		std::string wall_name;

	};	// struct Channel

	struct Window
	{
		Window( std::string const &nam, Channel const &chan,
				int width, int height, int px, int py )
			: name(nam), channel(chan),
			  w(width), h(height), x(px), y(py)
		{
			if( h < 0 || w < 0 )
			{
				std::string desc("Width or height of a Window can not be negative");
				BOOST_THROW_EXCEPTION( vl::invalid_settings() << vl::desc(desc) );
			}
		}

		// Default constructor to allow vector resize
		Window( void )
			: w(0), h(0), x(0), y(0)
		{}

		// Wether or not the Window has been initialised
		bool empty( void ) const
		{
			return( name.empty() && channel.empty()
				&& w == 0 && h == 0 && x == 0 && y == 0 );
		}

		// Name of the window
		std::string name;

		Channel channel;

		// Width of the window
		int w;
		// Height of the window
		int h;
		// x coordinate of the window
		int x;
		// y coordinate of the window
		int y;

	};	// struct Window

	struct Node
	{
		Node( std::string const &nam )
			: name(nam)
		{}

		// Default constructor to allow vector resize
		Node( void )
		{}

		bool empty( void ) const
		{ return name.empty() && windows.empty(); }

		void addWindow( Window const &window );

		Window const &getWindow( size_t i ) const;

		size_t getNWindows( void ) const
		{ return windows.size(); }

		std::vector<Window> const &getWindows( void ) const
		{ return windows; }

		std::string name;
		std::vector<Window> windows;
	};

	struct Server
	{
		Server( uint16_t por, std::string const hostnam )
			: port(por), hostname(hostnam)
		{}

		// Default constructor
		Server( void )
		{}

		uint16_t port;
		std::string hostname;
	};

	/// Constructor
	EnvSettings( void );

	/// Destructor
	~EnvSettings( void );

	/// Clear the settings structure to default values
	void clear( void );

	/** @brief Is this configuration for a slave node
	 *	@return true if this is slave configuration false if not
	 *
	 *	isSlave always returns !isMaster
	 */
	bool isSlave( void ) const
	{ return _slave; }

	/** @brief Is this configuration for a master node
	 *	@return true if this is master configuration false if not
	 *
	 *	isMaster always returns !isSlave
	 */
	bool isMaster( void ) const
	{ return !_slave; }

	/** @brief Set this confiration to be for a slave
	 *
	 *	after call to this isSlave returns true and isMaster returns false
	 */
	void setSlave( void )
	{ _slave = true; }

	/** @brief Set this confiration to be for a master
	 *
	 *	after call to this isSlave returns false and isMaster returns true
	 */
	void setMaster( void )
	{ _slave = false; }

	/** @brief Get the relative path to the file this configuration was in
	 *	@return string to relative path
	 *
	 *	@TODO is this a necessary function
	 *	@TODO possibility to return an absolute path
	 */
	std::string const &getFile( void ) const
	{ return _file_path; }

	void setFile( std::string const &file )
	{ _file_path = file; }

	std::string getPluginsDirFullPath( void ) const;

	///// PLUGINS /////////////////////////////////////////////////
	/// get vector containing plugin names and which are active
	std::vector<std::pair<std::string, bool> > const &getPlugins( void ) const
	{ return _plugins; }

	/// Add a plugin by name and boolean wether it's in use or not
	/// Checks that the same plugin is not added twice, NOP if the plugin is
	/// already in the plugins stack.
	void addPlugin( std::pair<std::string, bool> const &plugin );

	/// set the plugin named pluginName use to on or off
	/// returns true if plugin found
	/// prints an error message and returns false if no such plugin found
	bool pluginOnOff( std::string const &pluginName, bool newState );

	///// TRACKING /////////////////////////////////////////////////
	/// Returns a vector of tracking configs
	std::vector<Tracking> const &getTracking( void ) const
	{ return _tracking; }

	/**	@brief Get the tracking files which are used
	 *	@return a vector of the names of the tracking files
	 *
	 *	Only tracking files that are in use are returned
	 *	@TODO add checking that the files are valid
	 */
	std::vector< std::string > getTrackingFiles (void ) const;

	/**	@brief Adds a tracking file to the tracking file stack.
	 *	@param track a Tracking configuration to add to the file stack.
	 *
	 *	Checks that the same file tracking file is not added twice, NOP if the
	 *	tracking file is already in the stack.
	 */
	void addTracking( Tracking const &track );

	/**	@brief Remove a tracking file from the stack.
	 *	@param track a string with a tracking filename
	 *
	 *	Removes a tracking file with the same filename if such tracking file is
	 *	not found in these settings this function is a NOP.
	 */
	void removeTracking( std::string const &track );

	/**	@brief Remove a tracking file from the stack.
	 *	@param track a Tracking configuration comparison done with == operator
	 *
	 * 	Removes a tracking file with the same filename if such tracking file is
	 *	not found in these settings this function is a NOP.
	 */
	void removeTracking( Tracking const &track );

	/// Returns a flags of around which axes the camera rotations are allowed
	/// Fist bit is the x axis, second y axis, third z axis
	uint32_t getCameraRotationAllowed( void ) const
	{ return _camera_rotations_allowed; }

	void setCameraRotationAllowed( uint32_t const flags )
	{ _camera_rotations_allowed = flags; }

	void addWall( Wall const &wall );

	Wall const &getWall( size_t i ) const;

	size_t getNWalls( void ) const
	{ return _walls.size(); }

	std::vector<Wall> &getWalls( void )
	{ return _walls; }

	std::vector<Wall> const &getWalls( void ) const
	{ return _walls; }

	Wall findWall( std::string const &wall_name ) const;

	std::vector<Node> &getSlaves( void )
	{ return _slaves; }

	std::vector<Node> const &getSlaves( void ) const
	{ return _slaves; }

	Node findSlave( std::string const &name ) const;

	/** @brief Get the master nodes configuration
	 *	@return Node representing the Master configuration
	 */
	Node &getMaster( void )
	{ return _master; }

	/** @brief Get the master nodes configuration
	 *	@return Node representing the Master configuration
	 */
	Node const &getMaster( void ) const
	{ return _master; }

	/** @brief Get the server configuration
	 *	@return Server representing the configuration of the server
	 */
	Server const &getServer( void ) const
	{ return _server; }

	/** @brief Set the master Server configuration
	 *	@param server representing the Server configuration
	 */
	void setServer( Server const &server )
	{ _server = server; }

	/**	@brief set if we use stereo or not
	 *	@param stereo the configuration value of stereo
	 */
	void setStereo( CFG stereo )
	{ _stereo = stereo; }

	/**	@brief get the configuration of stereo
	 *	@return the configuration value of stereo param
	 */
	CFG getStereo( void ) const
	{ return _stereo; }

	/**	@brief set the amount of interpupilar distance used for stereo
	 *	@param ipd the distance in meters
	 */
	void setIPD( double ipd )
	{ _ipd = ipd; }

	/**	@brief get the amount of interpupilar distance used for stereo
	 *	@return interpupilar distance in meters
	 */
	double getIPD( void ) const
	{ return _ipd; }

	/**	@brief Set wether or not to output to std::cerr
	 *	@param verbose true for printing to std::cerr, false for not
	 *
	 *	If set to true the application will print to std::cerr
	 *	instead of or in addition to printing to log file
	 */
	void setVerbose( bool verbose )
	{ _verbose = verbose; }

	/**	@brief Wether we are outputing to std::cerr
	 *	@return true if should print to std::cerr, false should not
	 *
	 *	Does not define anything about log files, the application might
	 *	print to log files even when set true or it might not
	 */
	bool getVerbose( void ) const
	{ return _verbose; }

	/**	@brief Set the directory logs are stored
	 *	@param dir relative path to the log directory.
	 *
	 *	Path is assumed to be relative, though absolute might work it's
	 *	not guaranteed.
	 *	The directory is assumed to be valid directory. Not defined what will
	 *	happen in the program if it's not valid.
	 */
	void setLogDir( std::string const &dir )
	{ _log_dir = dir; }

	/** @brief Get the directory logs are stored.
	 *	@param type which kind of path to return, relative or absolute.
	 *	@return a path to the log dir, no checking is provided so it might be invalid.
	 *
	 *	Relative and absolute can be chosen using type parameter.
	 *	Defaults to returning absolute path.
	 *	No checking anywhere is provided so returned path might be invalid.
	 */
	std::string getLogDir( vl::PATH_TYPE const type = vl::PATH_ABS ) const;

	/** @brief Set the exe path i.e. the command used to start the program.
	 *	@param path the command used to start program, usually argv[0]
	 *	@TODO Is this necessary? Where is it used?
	 *		  We would need this for launching out of the exe directory
	 *		  or adding the exe directory to PATH to be used for dlls.
	 */
	void setExePath( std::string const &path );

	/** @brief Get the directory where this environment file is stored.
	 *	@return valid path to the file where this configuration is stored.
	 *			empty string if this is not stored into a file.
	 */
	std::string getEnvironementDir( void ) const;

private :

	std::string _file_path;

	std::vector<std::pair<std::string, bool> > _plugins;
	std::vector<Tracking> _tracking;

	uint32_t _camera_rotations_allowed;

	Server _server;

	Node _master;
	std::vector<Node> _slaves;

	std::vector<Wall> _walls;

	CFG _stereo;
	// Inter pupilar distance
	double _ipd;

	// Is this structure for a slave or a master
	bool _slave;

	bool _verbose;

	std::string _log_dir;

	std::string _exe_path;

};	// class EnvSettings





class EnvSettingsSerializer
{
public :
	/// Will completely over-ride the provided EnvSettings
	/// when XML data is processed.
	/// Any error in processing will result to defaulting EnvSettings.
	EnvSettingsSerializer( EnvSettingsRefPtr );

	~EnvSettingsSerializer( void );

	/// Read data from string buffer. Buffer is not modified.
	bool readString( std::string const &xml_data );


protected :
	bool readXML( void );

	void processConfig( rapidxml::xml_node<>* XMLNode );

	void processPlugins( rapidxml::xml_node<>* XMLNode );

	void processTracking( rapidxml::xml_node<>* XMLNode );

	void processCameraRotations( rapidxml::xml_node<>* XMLNode );

	void processWalls( rapidxml::xml_node<>* XMLNode );

	void processServer( rapidxml::xml_node<>* XMLNode );

	void processNode( rapidxml::xml_node<>* XMLNode, EnvSettings::Node &node );

	void processWindows( rapidxml::xml_node<>* XMLNode, EnvSettings::Node &node );

	void processChannel( rapidxml::xml_node<>* XMLNode, EnvSettings::Window &window );

	void processStereo( rapidxml::xml_node<>* XMLNode );

	void processIPD( rapidxml::xml_node<>* XMLNode );

	std::vector<double> getVector( rapidxml::xml_node<>* xml_node );

	EnvSettingsRefPtr _envSettings;

	/// file content needed for rapidxml
	char *_xml_data;

};	// class EnvSettingsSerializer

}	// namespace vl

#endif // VL_ENVSETTINGS_HPP
