#ifndef EQOGRE_FRAME_DATA_HPP
#define EQOGRE_FRAME_DATA_HPP

#include <eq/fabric/serializable.h>
#include <eq/client/config.h>
#include <eq/net/dataIStream.h>
#include <eq/net/dataOStream.h>

#include <OGRE/OgreVector3.h>
#include <OGRE/OgreQuaternion.h>

#include "scene_node.hpp"

namespace eqOgre
{

class FrameData : public eq::fabric::Serializable
{
public :
	FrameData( void )
		: //_camera_pos( Ogre::Vector3::ZERO ), _camera_rotation( Ogre::Quaternion::IDENTITY ),
		  _ogre( "ogre" ),
		  _ogre_id( EQ_ID_INVALID ),
		  _camera( "CameraNode" ),
		  _camera_id( EQ_ID_INVALID ),
		  _scene_version( 0 )
	{}

	~FrameData( void ) {}

	void setHeadPosition( Ogre::Vector3 const &pos )
	{
		setDirty( DIRY_HEAD );
		_head_pos = pos;
	}

	Ogre::Vector3 const &getHeadPosition( void ) const
	{
		return _head_pos;
	}

	void setHeadOrientation( Ogre::Quaternion const &quat )
	{
		setDirty( DIRY_HEAD );
		_head_orient = quat;
	}

	Ogre::Quaternion const &getHeadOrientation( void ) const
	{
		return _head_orient;
	}
	
	bool findNodes( Ogre::SceneManager *man )
	{
		bool retval = true;
		retval |= _camera.findNode( man );
		retval |= _ogre.findNode( man );
		
		return retval;
	}

	SceneNode &getCameraNode( void )
	{ return _camera; }

	SceneNode const &getCameraNode( void ) const
	{ return _camera; }

	void setCameraID( uint32_t id )
	{
		_camera_id = id;
	}

	uint32_t getCameraID( void ) const
	{
		return _camera_id;
	}

	SceneNode &getOgreNode( void )
	{ return _ogre; }

	SceneNode const &getOgreNode( void ) const
	{ return _ogre; }

	void setOgreID( uint32_t id )
	{
		_ogre_id = id;
	}

	uint32_t getOgreID( void ) const
	{
		return _ogre_id;
	}
	
	void updateSceneVersion( void )
	{ 
		setDirty( DIRTY_RELOAD_SCENE );
		_scene_version++;
	}

	uint32_t getSceneVersion( void ) const
	{
		return _scene_version;
	}

	uint32_t commitAll( void );

	void syncAll( void );

	void registerData( eq::Config *config );

	void deregisterData( eq::Config *config );

	void mapData( eq::Config *config, uint32_t id );

	void unmapData( eq::Config *config );
	
	enum DirtyBits
	{
		DIRY_HEAD = eq::fabric::Serializable::DIRTY_CUSTOM << 1,
		DIRTY_CAMERA = eq::fabric::Serializable::DIRTY_CUSTOM << 2,
		DIRTY_OGRE = eq::fabric::Serializable::DIRTY_CUSTOM << 3,
		DIRTY_RELOAD_SCENE = eq::fabric::Serializable::DIRTY_CUSTOM << 4,
		// TODO the reset scene is not implemented at the moment
		// It should reset all distributed SceneNodes
		DIRTY_RESET_SCENE = eq::fabric::Serializable::DIRTY_CUSTOM << 5,
		DIRTY_CUSTOM = eq::fabric::Serializable::DIRTY_CUSTOM << 6
	};

protected :
	virtual void serialize( eq::net::DataOStream &os, const uint64_t dirtyBits );
    virtual void deserialize( eq::net::DataIStream &is, const uint64_t dirtyBits );

private :
	// TODO these should be moved to a vector and there should be methods to
	// add more elements
	// TODO Also it's necessary to
	// refactor the sync, commit, register, deregister, map, unmap functions
	// TODO dynamically adding more nodes

	// Test scene node called Ogre
	SceneNode _ogre;
	uint32_t _ogre_id;

	// Test camera node
	SceneNode _camera;
	uint32_t _camera_id;

	Ogre::Vector3 _head_pos;
	Ogre::Quaternion _head_orient;
	
	// Reload the scene
	uint32_t _scene_version;

};	// class FrameData

}	// namespace eqOgre

#endif
