/**
 *	Copyright (c) 2010 - 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2010-11
 *	@file: base/xml_helpers.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

/**	Helper functions for parsing XML files
 *	Depends on Ogre Math
 */

#ifndef HYDRA_XML_HELPERS_HPP
#define HYDRA_XML_HELPERS_HPP

#include "math/transform.hpp"

#include <OGRE/OgreColourValue.h>

#include <string>

#include "base/rapidxml.hpp"

#include "base/string_utils.hpp"

namespace vl
{

Ogre::Real getAttribReal(rapidxml::xml_node<>* XMLNode,
		const std::string &parameter, Ogre::Real defaultValue = 0);

Ogre::Vector3 parseVector3(rapidxml::xml_node<>* XMLNode);

Ogre::Quaternion parseQuaternion(rapidxml::xml_node<>* XMLNode);

Ogre::ColourValue parseColour(rapidxml::xml_node<>* XMLNode);

vl::Transform parseTransform(rapidxml::xml_node<>* xml_node);

template<typename T> inline
T getAttrib( rapidxml::xml_node<> *xml_node,
		const std::string &attrib, T const &defaultValue )
{
	if( !xml_node->first_attribute(attrib.c_str()) )
	{ return defaultValue; }

	return vl::from_string<T>(xml_node->first_attribute(attrib.c_str())->value());
}

template<> inline
std::string getAttrib(rapidxml::xml_node<>* XMLNode,
		std::string const &attrib,
		std::string const &defaultValue)
{
	if(XMLNode->first_attribute(attrib.c_str()))
	{ return XMLNode->first_attribute(attrib.c_str())->value(); }
	else
	{ return defaultValue; }
}

inline
std::string getAttrib(rapidxml::xml_node<>* XMLNode,
		const std::string &parameter)
{
	return getAttrib(XMLNode, parameter, std::string());
}

inline
bool getAttribBool(rapidxml::xml_node<>* XMLNode,
		const std::string &parameter, bool defaultValue = false)
{
	return getAttrib(XMLNode, parameter, defaultValue);
}

}	// namespace vl

#endif	// HYDRA_XML_HELPERS_HPP
