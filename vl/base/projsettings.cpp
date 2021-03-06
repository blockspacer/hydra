/**
 *	Copyright (c) 2010 - 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2010-10
 *	@file base/projsettings.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

/**	Joonas Reunamo <joonas.reunamo@tut.fi>
 *	2010-10
 *
 *	Updated by Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-11
 *	Removed printing to standard out from all methods
 *
 *	Updated by Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-12
 *	Fixed const correctness and changed unsigned int to size_t
 *
 *	Updated by Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-12
 *	Added non-const retrieval methods
 *	Replaced spaces with tabs
 *	Added destructors
 *	Removed commented out printing
 */

/// Declaration
#include "projsettings.hpp"

#include "filesystem.hpp"
#include "exceptions.hpp"

#include "string_utils.hpp"

/// Needed for writing the xml struct to string
#include "rapidxml_print.hpp"

#include <iostream>

////////// CASE SCENE /////////////////////////////////////////////////////////
vl::config::ProjSettings::Scene &
vl::config::ProjSettings::Case::addScene( std::string const &name )
{
	_scenes.push_back(vl::config::ProjSettings::Scene(name));
	_changed = true;
	return _scenes.back();
}

void
vl::config::ProjSettings::Case::addScene(vl::config::ProjSettings::Scene const &scene)
{
	_scenes.push_back( ProjSettings::Scene( scene ) );
	_changed = true;
}

void
vl::config::ProjSettings::Case::rmScene(std::string const &name)
{
	for( size_t i = 0; i < _scenes.size(); i++ )
	{
		if( _scenes.at(i).getName() == name )
		{
			_scenes.erase( _scenes.begin() + i );
		}
	}

	_changed = true;
}

vl::config::ProjSettings::Scene const &
vl::config::ProjSettings::Case::getScene( std::string const &sceneName ) const
{
	for( size_t i = 0; i < _scenes.size(); i++ )
	{
		if( _scenes.at(i).getName() == sceneName )
		{
			return _scenes.at(i);
		}
	}

	return vl::config::ProjSettings::Scene();
}

vl::config::ProjSettings::Scene const &
vl::config::ProjSettings::Case::getScene(size_t i) const
{
	return _scenes.at(i);
}

////////// CASE SCRIPTS ///////////////////////////////////////////////////////////////

vl::config::ProjSettings::Script &
vl::config::ProjSettings::Case::addScript( std::string const &name )
{
	_scripts.push_back(vl::config::ProjSettings::Script(name));
	_changed = true;
	return _scripts.back();
}

void
vl::config::ProjSettings::Case::rmScript(std::string const &name)
{
// 	ProjSettings::Script const &script = getScript(name);
	for( unsigned int i = 0; i < _scripts.size(); i++ )
	{
		if( _scripts.at(i).getFile() == name )
		{
			_scripts.erase( _scripts.begin() + i );
		}
	}

	_changed = true;
}

vl::config::ProjSettings::Script const &
vl::config::ProjSettings::Case::getScript( std::string const &name ) const
{
	for( unsigned int i = 0; i < _scripts.size(); i++ )
	{
		if( _scripts.at(i).getFile() == name )
		{
			return _scripts.at(i);
		}
	}

	// @todo throw
	return vl::config::ProjSettings::Script();
}

vl::config::ProjSettings::Script &
vl::config::ProjSettings::Case::getScript( std::string const &name )
{
	for( unsigned int i = 0; i < _scripts.size(); i++ )
	{
		if( _scripts.at(i).getFile() == name )
		{
			return _scripts.at(i);
		}
	}

	// @todo throw
}

vl::config::ProjSettings::Script const &
vl::config::ProjSettings::Case::getScript( size_t i ) const
{
	return _scripts.at(i);
}

bool
vl::config::ProjSettings::Case::getChanged( void ) const
{
	if( _changed )
	{ return true; }

	for( unsigned int i = 0; i < _scripts.size(); i++ )
	{
		if( _scripts.at(i).getChanged() == true )
		{ return true; }
	}
	for( unsigned int i = 0; i < _scenes.size(); i++ )
	{
		if( _scenes.at(i).getChanged() == true )
		{ return true; }
	}

	return false;
}

void
vl::config::ProjSettings::Case::clearChanged( void )
{
	_changed = false;

	for( unsigned int i = 0; i < _scripts.size(); i++ )
	{
		_scripts.at(i).clearChanged();
	}
	for( unsigned int i = 0; i < _scenes.size(); i++ )
	{
		_scenes.at(i).clearChanged();
	}
}

vl::config::ProjSettings::ProjSettings( std::string const &file ) 
	: _file( file ), _projCase( "" ), _changed(false)
{}

vl::config::ProjSettings::~ProjSettings(void)
{}

void
vl::config::ProjSettings::clear(void)
{
	_file.clear();
	_cases.clear();
	_changed = false;
}


///// CHANGED /////////////////////////////////////////////////////
bool
vl::config::ProjSettings::getChanged(void) const
{
	if( _changed )
	{
		return true;
	}
	for( unsigned int i = 0; i < _cases.size(); i++ )
	{
		if( _cases.at(i).getChanged() == true )
		{
			return true;
		}
	}

	return false;
}

void
vl::config::ProjSettings::clearChanged(void)
{
	_changed = false;
	for( unsigned int i = 0; i < _cases.size(); i++ )
	{
		_cases.at(i).clearChanged();
	}
}


///// CASES /////////////////////////////////////////////////
vl::config::ProjSettings::Case &
vl::config::ProjSettings::addCase(std::string const &caseName)
{
	_cases.push_back( ProjSettings::Case( caseName ) );
	_changed = true;
	return _cases.back();
}

void
vl::config::ProjSettings::rmCase(std::string const &caseName)
{
// 	vl::ProjSettings::Case const &rmCase = getCase( caseName );
	for( unsigned int i = 0; i < _cases.size(); i++ )
	{
		if( _cases.at(i).getName() == caseName )
		{
			_cases.erase( _cases.begin() + i );
		}
	}

	_changed = true;
}

vl::config::ProjSettings::Case const &
vl::config::ProjSettings::getCase( std::string const &caseName ) const
{
	if( caseName == _projCase.getName() )
	{ return _projCase; }

	for( unsigned int i = 0; i < _cases.size(); i++ )
	{
		if( _cases.at(i).getName() == caseName )
		{ return _cases.at(i); }
	}

	// @todo should throw
	return vl::config::ProjSettings::Case();
}

vl::config::ProjSettings::Case &
vl::config::ProjSettings::getCase( std::string const &caseName )
{
	if( caseName == _projCase.getName() )
	{ return _projCase; }

	for( unsigned int i = 0; i < _cases.size(); i++ )
	{
		if( _cases.at(i).getName() == caseName )
		{ return _cases.at(i); }
	}

	// @todo should throw
}

vl::config::ProjSettings::Case const &
vl::config::ProjSettings::getCase( size_t i ) const
{
	return _cases.at(i);
}

bool
vl::config::ProjSettings::empty(void) const
{
	return( _projCase.empty() && _cases.empty() );
}











///////////////////////////////////////////////////////////////////////
////////////////////// --- ProjSettingsSerializer --- //////////////////
///////////////////////////////////////////////////////////////////////
vl::config::ProjSettingsSerializer::ProjSettingsSerializer(void)
    : _proj(0), _xml_data(0)
{}


vl::config::ProjSettingsSerializer::~ProjSettingsSerializer(void)
{
	delete [] _xml_data;
}

bool
vl::config::ProjSettingsSerializer::readFile(ProjSettings &proj, std::string const &file_path)
{
	proj.setFile(file_path);

	std::string file;
	vl::readFileToString(file_path, file);
	bool retval = readString(proj, file);
	
	return retval;
}

bool
vl::config::ProjSettingsSerializer::readString(ProjSettings &proj, std::string const &str)
{
	_proj = &proj;
	assert(_proj);
	
	assert(!_xml_data);
	size_t length = str.length() + 1;
	_xml_data = new char[length];
	memcpy(_xml_data, str.c_str(), length);

	bool retval = readXML();

	_proj = 0;
	delete[] _xml_data;
	_xml_data = 0;

	return retval;
}

bool
vl::config::ProjSettingsSerializer::readXML()
{
	rapidxml::xml_document<> xmlDoc;
	xmlDoc.parse<0>( _xml_data );

	rapidxml::xml_node<> *xmlRoot = xmlDoc.first_node("project_config");
	if( !xmlRoot )
	{
		std::cerr << "Errenous xml. project_config node missing. Wrong file?"
			<< std::endl;
		return false;
	}

	readConfig( xmlRoot );
	return true;
}


void
vl::config::ProjSettingsSerializer::readConfig( rapidxml::xml_node<>* xml_root )
{
	rapidxml::xml_attribute<>* attrib = xml_root->first_attribute("name");
	if( !attrib )
	{
		std::cerr << "Missing project name. Default is used" << std::endl;
		_proj->setName("Default");
	}
	else
	{
		std::string nameFromFile = attrib->value();
		_proj->setName(nameFromFile);
	}

	ProjSettings::Case &projectCase = _proj->getCase();

	rapidxml::xml_node<>* xml_elem;

	xml_elem = xml_root->first_node("scenes");
	if( xml_elem )
	{
		readScenes( xml_elem, projectCase );
	}

	xml_elem = xml_root->first_node("scripts");
	if( xml_elem )
	{
		readScripts( xml_elem, projectCase );
	}

	xml_elem = xml_root->first_node("cases");
	if( xml_elem )
	{
		readCases( xml_elem );
	}
}

void
vl::config::ProjSettingsSerializer::readScenes( rapidxml::xml_node<>* xml_node, vl::config::ProjSettings::Case &c )
{
	rapidxml::xml_node<> *scene_element = xml_node->first_node("scene");

	while( scene_element )
	{
		readScene( scene_element, c );
		scene_element = scene_element->next_sibling("scene");
	}
}

void
vl::config::ProjSettingsSerializer::readScene( rapidxml::xml_node<>* xml_node, vl::config::ProjSettings::Case &c )
{
	std::string name;
	// Name
	rapidxml::xml_attribute<>* attrib = xml_node->first_attribute("name");
	if(!attrib)
	{
		std::cerr << "Name of scene in case " << c.getName()
			<< " missing" << std::endl;
		name = "Default";
	}
	else
	{
		name = attrib->value();
	}

	ProjSettings::Scene &s = c.addScene(name);

	// Use
	attrib = xml_node->first_attribute("use");
	if(!attrib)
	{
		s.setUse(false);
		std::cerr << "Defaultin use of scene " << s.getName()
			<< " to false." << std::endl;
	}
	else
	{
		bool use = vl::from_string<bool>(attrib->value());
		s.setUse(use);
	}

	attrib = xml_node->first_attribute("use_new_mesh_manager");
	if(attrib)
	{
		CFG use = CFG_OFF;
		bool val = vl::from_string<bool>(attrib->value());
		if(val)
		{ use = CFG_ON; }
		s.setUseNewMeshManager(use);
	}

	attrib = xml_node->first_attribute("enable_physics");
	if(attrib)
	{
		bool use = vl::from_string<bool>(attrib->value());
		s.setUsePhysics(use);
	}

	// File
	rapidxml::xml_node<>* file_node = xml_node->first_node("file");
	if( !file_node )
	{
		std::cerr << "Scene " << s.getName() << " is missing path!" << std::endl;
	}
	else
	{
		s.setFile( file_node->value() );
	}

	// Attach to
	rapidxml::xml_node<>* attachto_node = xml_node->first_node("attachto");
	if( attachto_node )
	{
		rapidxml::xml_attribute<>* scene_attrib = attachto_node->first_attribute("scene");
		rapidxml::xml_attribute<>* point_attrib = attachto_node->first_attribute("point");
		if( scene_attrib )
		{
			s.setAttachtoScene( scene_attrib->value() );
		}
		if( point_attrib )
		{
			s.setAttachtoPoint( point_attrib->value() );
		}
	}
}

void
vl::config::ProjSettingsSerializer::readScripts( rapidxml::xml_node<>* xml_node, vl::config::ProjSettings::Case &c )
{
	rapidxml::xml_node<> *script_element = xml_node->first_node("file");

	if( !script_element )
	{ return; }

	while( script_element )
	{
		ProjSettings::Script &a = c.addScript( script_element->value() );

		// Use
		rapidxml::xml_attribute<>* use_attrib = script_element->first_attribute("use");
		if( !use_attrib )
		{
			std::cerr << "Defaultin use of script " << a.getFile()
				<< " to false." << std::endl;
		}
		else
		{
			std::string useStr = use_attrib->value();
			if( useStr == "true" )
			{
				a.scriptOnOff( true );
			}
			else if( useStr == "false" )
			{
				a.scriptOnOff( false );
			}
			else
			{
				a.scriptOnOff( false );
				std::cerr << "Errenous use attribute of script "
					<< a.getFile() << ". Defaulting to false." << std::endl;
			}
		}

		script_element = script_element->next_sibling("file");
	}
}

void
vl::config::ProjSettingsSerializer::readCases( rapidxml::xml_node<>* xml_node )
{
	rapidxml::xml_node<> *case_element = xml_node->first_node("case");
	if( !case_element )
	{ return; }
	while( case_element )
	{
		readCase( case_element );
		case_element = case_element->next_sibling("case");
	}
}

void
vl::config::ProjSettingsSerializer::readCase( rapidxml::xml_node<>* xml_node )
{
	std::string name;
	// Name
	rapidxml::xml_attribute<>* name_attrib = xml_node->first_attribute("name");
	if( !name_attrib )
	{ name = "Default"; }
	else
	{ name = name_attrib->value(); }

	ProjSettings::Case &newCase = _proj->addCase( name );

	// Scenes
	rapidxml::xml_node<> *scenes_element = xml_node->first_node("scenes");
	if( scenes_element )
	{
		readScenes( scenes_element, newCase );
	}

	// Scripts
	rapidxml::xml_node<> *script_element = xml_node->first_node("scripts");
	if( script_element )
	{
		readScripts( script_element, newCase );
	}
}



////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////
/////// WRITE //////////////////////////////////////

void
vl::config::ProjSettingsSerializer::writeString( std::string &str )
{
	rapidxml::xml_node<> *xmlRoot = _doc.allocate_node(rapidxml::node_element, "project_config" );
	char *c_name = _doc.allocate_string( _proj->getCase().getName().c_str() );
	rapidxml::xml_attribute<> *attrib = _doc.allocate_attribute( "name", c_name );
	xmlRoot->append_attribute( attrib );

	writeConfig(xmlRoot);

	_doc.append_node( xmlRoot );

	rapidxml::print(std::back_inserter( str ), _doc, 0);
}

void
vl::config::ProjSettingsSerializer::writeConfig( rapidxml::xml_node<> *xml_node )
{
	writeScenes( xml_node, _proj->getCase() );
	writeScripts( xml_node, _proj->getCase() );
	writeCases( xml_node );
}

const char *
vl::config::ProjSettingsSerializer::bool2char( bool b ) const
{
	if( b )
	{ return "true"; }
	return "false";
}

void
vl::config::ProjSettingsSerializer::writeScenes( rapidxml::xml_node<> *xml_node,
										 vl::config::ProjSettings::Case const &cas )
{
	rapidxml::xml_node<> *scenes_node = _doc.allocate_node(rapidxml::node_element, "scenes" );
	for( size_t i = 0; i < cas.getNscenes(); i++ )
	{
		ProjSettings::Scene const &sc = cas.getScene(i);
		char *c_name = _doc.allocate_string( sc.getName().c_str() );
		char *c_use = _doc.allocate_string( bool2char(sc.getUse()) );
		char *c_file = _doc.allocate_string( sc.getFile().c_str() );
		char *c_attachto_scene = _doc.allocate_string( sc.getAttachtoScene().c_str() );
		char *c_attachto_point = _doc.allocate_string( sc.getAttachtoPoint().c_str() );

		rapidxml::xml_node<> *scene
				=  _doc.allocate_node(rapidxml::node_element, "scene" );
		rapidxml::xml_attribute<> *attrib
				= _doc.allocate_attribute( "name", c_name );
		scene->append_attribute(attrib);
		attrib = _doc.allocate_attribute( "use", c_use );
		scene->append_attribute(attrib);

		rapidxml::xml_node<> *node
				=  _doc.allocate_node(rapidxml::node_element, "file", c_file );
		scene->append_node(node);

		node =  _doc.allocate_node(rapidxml::node_element, "attachto" );
		attrib = _doc.allocate_attribute( "scene", c_attachto_scene );
		node->append_attribute(attrib);
		attrib = _doc.allocate_attribute( "point", c_attachto_point );
		node->append_attribute(attrib);
		scene->append_node(node);

		scenes_node->append_node(scene);
	}
	xml_node->append_node(scenes_node);
}

void
vl::config::ProjSettingsSerializer::writeScripts( rapidxml::xml_node<> *xml_node,
										  vl::config::ProjSettings::Case const &cas )
{
	rapidxml::xml_node<> *scripts_node = _doc.allocate_node(rapidxml::node_element, "scripts" );
	for( size_t i = 0; i < cas.getNscripts(); i++ )
	{
		ProjSettings::Script const &script = cas.getScript(i);
		char *fileC = _doc.allocate_string( script.getFile().c_str() );
		char *use = _doc.allocate_string( bool2char(script.getUse()) );

		rapidxml::xml_node<> *file
					=  _doc.allocate_node(rapidxml::node_element, "file", fileC );
		rapidxml::xml_attribute<> *attrib
				= _doc.allocate_attribute( "use", use );
		file->append_attribute(attrib);

		scripts_node->append_node(file);
	}

	xml_node->append_node(scripts_node);
}

void
vl::config::ProjSettingsSerializer::writeCases( rapidxml::xml_node<> *xml_node )
{
	rapidxml::xml_node<> *cases_node = _doc.allocate_node(rapidxml::node_element, "cases" );
	for( size_t i = 0; i < _proj->getNcases(); i++ )
	{
		ProjSettings::Case const &cas = _proj->getCase(i);
		char *nameC = _doc.allocate_string( cas.getName().c_str() );

		rapidxml::xml_node<> *case_node
				=  _doc.allocate_node(rapidxml::node_element, "case" );
		rapidxml::xml_attribute<> *attrib
				= _doc.allocate_attribute( "name", nameC );
		case_node->append_attribute(attrib);

		writeScenes( case_node, cas );
		writeScripts( case_node, cas );

		cases_node->append_node(case_node);
	}

	xml_node->append_node(cases_node);
}
