/**	Joonas Reunamo <joonas.reunamo@tut.fi>
 *	2010-10
 *
 *	Updated by Joonatan Kuosa <joonatan.kuosa@tut.fi>
 * 2010-11
 */


/// Declaration
#include "envsettings.hpp"


#include "filesystem.hpp"
#include "exceptions.hpp"

#include <iostream>


vl::EnvSettings::EnvSettings( void )
	: _camera_rotations_allowed( 1 | 1<<1 | 1<<2 )
{}

vl::EnvSettings::~EnvSettings( void )
{}

void
vl::EnvSettings::clear( void )
{
	_eqc.clear();

	_plugins.clear();
	_tracking.clear();

	_camera_rotations_allowed = 0;
}

std::string vl::EnvSettings::getEqcFullPath( void ) const
{
	if( getEqc().empty() )
	{ return std::string(); }

	fs::path env_path = getFile();
	fs::path env_dir = env_path.parent_path();
	fs::path path =  env_dir / "eqc" / getEqc();

	return path.file_string();
}

std::string vl::EnvSettings::getPluginsDirFullPath( void ) const
{
	fs::path env_path = getFile();
	fs::path env_dir = env_path.parent_path();
	fs::path path =  env_dir / "plugins";

	return path.file_string();
}

void
vl::EnvSettings::addPlugin(const std::pair< std::string, bool >& plugin)
{
	std::vector< std::pair<std::string, bool> >::iterator iter;
	for( iter = _plugins.begin(); iter != _plugins.end(); ++iter )
	{
		if( iter->first == plugin.first )
		{ return; }
	}

	_plugins.push_back( plugin );
}

bool
vl::EnvSettings::pluginOnOff(const std::string &pluginName, bool newState)
{
	bool found = false;
	for( unsigned int i = 0; i < _plugins.size(); i++ )
	{
		if( _plugins.at(i).first == pluginName )
		{
			_plugins.at(i).second = newState;
			found = true;
			// TODO should return here
			// but there can be same plugin multiple times o_O
		}
	}
	if(!found)
	{
		std::cerr << "Tried to toggle plugin " << pluginName
			<< " which is not present" << std::endl;
		return false;
	}
	return true;
}

void vl::EnvSettings::addTracking(const vl::EnvSettings::Tracking& track)
{
	// Check that we don't add the same file twice
	std::vector<Tracking>::iterator iter;
	for( iter = _tracking.begin(); iter != _tracking.end(); ++iter )
	{
		if( iter->file == track.file )
		{ return; }
	}

	// Not a duplicate
	_tracking.push_back(track);
}

void
vl::EnvSettings::removeTracking( std::string const &track )
{
	removeTracking( Tracking(track) );
}

void
vl::EnvSettings::removeTracking( Tracking const &track )
{
	std::vector<Tracking>::iterator iter;
	for( iter = _tracking.begin(); iter != _tracking.end(); ++iter )
	{
		if( iter->file == track.file )
		{
			_tracking.erase( iter );
			// TODO this should return here if we assume that no file can exist
			// more than ones.
			return;
		}
	}
}


///////////////////////////////////////////////////////////////////////
////////////////////// --- EnvSettingsSerializer --- //////////////////
///////////////////////////////////////////////////////////////////////

vl::EnvSettingsSerializer::EnvSettingsSerializer( EnvSettingsRefPtr envSettingsPtr )
    : _envSettings(envSettingsPtr), _xml_data(0)
{
}


vl::EnvSettingsSerializer::~EnvSettingsSerializer( void )
{
    delete [] _xml_data;
}

bool
vl::EnvSettingsSerializer::readString( std::string const &str )
{
    delete[] _xml_data;
    size_t length = str.length() + 1;
    _xml_data = new char[length];
    memcpy(_xml_data, str.c_str(), length);

    return readXML();
}

bool
vl::EnvSettingsSerializer::readXML()
{
    rapidxml::xml_document<> xmlDoc;
    xmlDoc.parse<0>( _xml_data );

    rapidxml::xml_node<> *xmlRoot = xmlDoc.first_node("env_config");
    if( !xmlRoot )
    {
        std::cerr << "Errenous xml. env_config node missing. Wrong file?" << std::endl;
        return false;
    }
    processConfig( xmlRoot );
    return true;
}

void
vl::EnvSettingsSerializer::processConfig( rapidxml::xml_node<>* xml_root )
{
    rapidxml::xml_node<>* xml_elem;

    xml_elem = xml_root->first_node("plugins");
    if( xml_elem )
    { processPlugins( xml_elem ); }

    xml_elem = xml_root->first_node("eqc");
    if( xml_elem )
    { processEqc( xml_elem ); }

    xml_elem = xml_root->first_node("tracking");
    if( xml_elem )
    { processTracking( xml_elem ); }

    xml_elem = xml_root->first_node("camera_rotations");
	if( xml_elem )
	{ processCameraRotations( xml_elem ); }
}


void
vl::EnvSettingsSerializer::processPlugins( rapidxml::xml_node<>* xml_node )
{
	rapidxml::xml_node<> *pElement = xml_node->first_node("plugin");

	if( !pElement )
	{
		std::cerr << "Plugins list missing from env_config node." << std::endl;
		return;
	}

	std::pair<std::string, bool> plugin;
	std::string useStr;
	std::string name;
	bool use;
	rapidxml::xml_attribute<> *attrib;

	while( pElement )
	{
		attrib = pElement->first_attribute("use");
		if( !attrib )
		{
			std::cerr << "Missing use attrib. Defaulting to false." << std::endl;
			useStr = "false";
		}
		else
		{
			useStr = attrib->value();
		}

		if( useStr == "true" )
		{ use = true; }
		else if( useStr == "false" )
		{ use = false; }
		else
		{
			std::cerr << "One plugin has errenous use attribute. Defaulting to false." << std::endl;
			use = false;
		}
		name = pElement->value();
		plugin = std::make_pair( name, use );
		_envSettings->addPlugin(plugin);

		pElement = pElement->next_sibling("plugin");
	}
}

void
vl::EnvSettingsSerializer::processEqc( rapidxml::xml_node<>* xml_node )
{
    rapidxml::xml_node<> *pElement = xml_node->first_node("file");

    if( !pElement )
    { return; }

    _envSettings->setEqc( pElement->value() );

    // No more than one eqc
    pElement = pElement->next_sibling("file");
    if( pElement )
    {
        std::cerr << "More than one eqc file? Only first one taken." << std::endl;
        return;
    }
}


void
vl::EnvSettingsSerializer::processTracking( rapidxml::xml_node<>* xml_node )
{
	rapidxml::xml_node<> *pElement = xml_node->first_node("file");

	while( pElement )
	{
		bool use = true;
		rapidxml::xml_attribute<> *attrib = pElement->first_attribute("use");
		if( attrib && std::string( attrib->value() ) == "false" )
		{
			use = false;
		}

		_envSettings->addTracking( EnvSettings::Tracking(pElement->value(), use) );
		pElement = pElement->next_sibling("file");
	}
}

void
vl::EnvSettingsSerializer::processCameraRotations( rapidxml::xml_node< char >* xml_node )
{
	uint32_t flags = 0;
	std::string const F("false");

	rapidxml::xml_attribute<> *attrib = xml_node->first_attribute("x");
	// Defautls to true
	if( !attrib || std::string(attrib->value()) != F )
	{ flags |= 1; }

	attrib = xml_node->first_attribute("y");
	// Defautls to true
	if( !attrib || std::string(attrib->value()) != F )
	{ flags |= 1<<1; }

	attrib = xml_node->first_attribute("z");
	// Defautls to true
	if( !attrib || std::string(attrib->value()) != F )
	{ flags |= 1<<2; }

	_envSettings->setCameraRotationAllowed(flags);
}