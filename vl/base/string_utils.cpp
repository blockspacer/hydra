/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-08
 *	@file base/string_utils.cpp
 *
 *	This file is part of Hydra VR game engine.
 */

#include "string_utils.hpp"

const char* vl::WHITESPACE = " \t\v\f\n\r";

void
vl::break_string_down( std::vector<std::string> &broken_path,
					   std::string &longString,
					   char delimeter )
{
	std::string::iterator begin = longString.begin();
	std::string::iterator path_iter
		= std::find( begin, longString.end(), delimeter );

	while( path_iter != longString.end() )
	{
		if( path_iter == begin )
		{ BOOST_THROW_EXCEPTION( vl::exception() << vl::desc( "Iterators incorrect" ) ); }

		std::string str;
		str.insert( str.begin(), begin, path_iter );
		broken_path.push_back( str );
		begin = ++path_iter;
		path_iter = std::find( begin, longString.end(), delimeter );
	}

	std::string str;
	str.insert( str.begin(), begin, path_iter );
	broken_path.push_back( str );
}

void
vl::replace_line_endings(std::string &str)
{
	size_t index = str.find( "\r\n" ); 
	while( index != std::string::npos )
	{
		str.erase(index,1);
		index = str.find( "\r\n" );
	}
}

std::string
vl::generate_random_string(size_t len)
{
	static const char alphanum[] =
		"0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz";

	std::string s(len, 'a');
	for(size_t i = 0; i < len; ++i)
	{
		s.at(i) = alphanum[rand() % (sizeof(alphanum) - 1)];
	}

	return s;
}