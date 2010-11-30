
#include "resource_manager.hpp"

#include "base/filesystem.hpp"

vl::ResourceManager::ResourceManager( void )
{}

vl::ResourceManager::~ResourceManager( void )
{}

bool
vl::ResourceManager::addResourcePath( std::string const &resource_dir, bool recursive )
{
	fs::path dir(resource_dir);
	if( !fs::exists(dir) || !fs::is_directory(dir) )
	{ return false; }

	_paths.push_back(resource_dir);
	if( recursive )
	{
		fs::recursive_directory_iterator end_iter;
		for( fs::recursive_directory_iterator iter(dir);
			 iter != end_iter; ++iter )
		{
			if( fs::is_directory( iter->path() ) )
			{
				_paths.push_back( iter->path().file_string() );
			}
		}
	}

	return true;
}

bool
vl::ResourceManager::loadResource( const std::string& name, std::string& data )
{
	std::string file_path;

	if( findResource(name, file_path) )
	{
		// Load the resource
		vl::readFileToString( file_path, data );

		return true;
	}

	return false;
}

bool vl::ResourceManager::findResource(const std::string& name, std::string& path)
{
	for( std::vector<std::string>::const_iterator iter = _paths.begin();
		 iter != _paths.end(); ++iter )
	{
		fs::path dir( *iter );
		fs::path file_path = dir/name;
		if( fs::exists( file_path ) )
		{

			path = file_path.file_string();
			return true;
		}
	}

	return false;
}
