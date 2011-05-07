/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-04
 *	@file entity.cpp
 */

#include "entity.hpp"

#include "scene_manager.hpp"
#include "scene_node.hpp"
// Necessary for loading meshes with the new interface
#include "mesh_manager.hpp"

#include <OGRE/OgreEntity.h>
#include <OGRE/OgreSceneManager.h>

vl::Entity::Entity(std::string const &name, vl::PREFAB type, vl::SceneManagerPtr creator)
	: MovableObject(name, creator)
{
	_clear();
	_prefab = type;
}

vl::Entity::Entity(std::string const &name, std::string const &mesh_name, vl::SceneManagerPtr creator, bool use_new_mesh_manager)
	: MovableObject(name, creator)
{
	_clear();
	_mesh_name = mesh_name;
	_use_new_mesh_manager = use_new_mesh_manager;
	if(_use_new_mesh_manager)
	{
		std::clog << "vl::Entity::Entity : loading a mesh not implemented" << std::endl;
		// @todo load the mesh
		_mesh = creator->getMeshManager()->loadMesh(mesh_name);
	}
}

vl::Entity::Entity(vl::SceneManagerPtr creator)
	: MovableObject("", creator)
{
	_clear();
}

vl::Entity::~Entity(void)
{}

void 
vl::Entity::setCastShadows(bool shadows)
{
	if( _cast_shadows != shadows )
	{
		setDirty(DIRTY_CAST_SHADOWS);
		_cast_shadows = shadows;
	}
}

void 
vl::Entity::setMaterialName(std::string const &name)
{
	if( _material_name != name )
	{
		setDirty(DIRTY_MATERIAL);
		_material_name = name;
	}
}

void 
vl::Entity::doSerialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits )
{
	if( DIRTY_PREFAB & dirtyBits )
	{
		msg << _prefab;
	}

	if( DIRTY_MESH_NAME & dirtyBits )
	{
		msg << _mesh_name << _use_new_mesh_manager;
	}

	if( DIRTY_CAST_SHADOWS & dirtyBits )
	{
		msg << _cast_shadows;
	}
	
	if( DIRTY_MATERIAL & dirtyBits )
	{
		msg << _material_name;
	}
}

void 
vl::Entity::doDeserialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits )
{
	if( DIRTY_PREFAB & dirtyBits )
	{
		msg >> _prefab;
	}

	if( DIRTY_MESH_NAME & dirtyBits )
	{
		msg >> _mesh_name >> _use_new_mesh_manager;
	}

	if( DIRTY_CAST_SHADOWS & dirtyBits )
	{
		msg >> _cast_shadows;
		if( _ogre_object )
		{ _ogre_object->setCastShadows(_cast_shadows); }
	}

	if( DIRTY_MATERIAL & dirtyBits )
	{
		msg >> _material_name;
		if( _ogre_object )
		{ _ogre_object->setMaterialName(_material_name); }
	}
}


bool
vl::Entity::_doCreateNative(void)
{
	if( _ogre_object )
	{ return true; }

	assert( _creator );
	assert( _creator->getNative() );
	assert( !_name.empty() );

	
	/// Catching the throw can not be used for mesh file 
	/// because the ogre entity is not created then
	if( _creator->getNative()->hasEntity( _name ) )
	{
		_ogre_object = _creator->getNative()->getEntity( _name );
	}
	else if( !_mesh_name.empty() )
	{
		if( _use_new_mesh_manager )
		{
//			std::clog << "vl::Entity::_doCreateNative : Should use the new MeshManager." << std::endl;
			vl::MeshRefPtr mesh = _creator->getMeshManager()->loadMesh(_mesh_name);
			Ogre::MeshPtr og_mesh = vl::create_ogre_mesh(_mesh_name, mesh);
//			std::clog << "Ogre mesh " << _mesh_name << " : bounds = " << og_mesh->getBounds()
//				<< " is loaded = " << og_mesh->isLoaded() << std::endl;
		}
		_ogre_object = _creator->getNative()->createEntity(_name, _mesh_name);
//		Ogre::MeshPtr og_mesh = _ogre_object->getMesh();
//		std::clog << "Creating entity : with ogre mesh " << _mesh_name << " : bounds = " << og_mesh->getBounds()
//			<< " is loaded = " << og_mesh->isLoaded() << std::endl;
	}
	else if( PF_NONE != _prefab )
	{
		// Lets create the entity
		Ogre::SceneManager::PrefabType t;
		if( PF_PLANE == _prefab )
		{ t = Ogre::SceneManager::PT_PLANE; }
		else if( PF_SPHERE == _prefab )
		{ t = Ogre::SceneManager::PT_SPHERE; }
		else if(PF_CUBE == _prefab)
		{ t = Ogre::SceneManager::PT_CUBE; }
		else
		{
			std::cerr << "Trying to create unknow prefab entity. Not supported." << std::endl;
			return false;
		}
		_ogre_object = _creator->getNative()->createEntity(_name, t);
	}
	else
	{
		std::cerr << "Trying to create an entity without a mesh file" 
			<< " or prefab. Not supported." << std::endl;
		return false;
	}

	assert(_ogre_object);

	/// Really quick and dirty way to avoid problems when there is
	/// no uv coordinates 
	try {
		// TODO this will not work for objects without UVs
		// Will it try to use it when there is no UVs?
		// TODO this might also be really slow, we should probably have materails
		// that need tangents and those request the rebuild.
		// Also a more long term solution would be to build the tangents
		// into the meshe files.
		// TODO this does not work for the Ogre test object for some reason
		// it still has the zero tangents even though it has UVs.
		Ogre::MeshPtr mesh = _ogre_object->getMesh();
		unsigned short src, dest;
		if (!mesh->suggestTangentVectorBuildParams(Ogre::VES_TANGENT, src, dest))
		{
			mesh->buildTangentVectors(Ogre::VES_TANGENT, src, dest);
		}
	}
	catch( Ogre::Exception const &e)
	{
		std::cout << "Exception : " << e.what() << std::endl;
	}

	_ogre_object->setCastShadows(_cast_shadows);
	if( !_material_name.empty() )
	{ _ogre_object->setMaterialName(_material_name); }

	return true;
}

void 
vl::Entity::_clear(void)
{
	_prefab = PF_NONE;
	_cast_shadows = true;
	_use_new_mesh_manager = false;
	_ogre_object = 0;
}
