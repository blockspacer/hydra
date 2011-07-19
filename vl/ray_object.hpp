/**	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-07
 *	@file: ray_object.hpp
 *
 *	This file is part of Hydra VR game engine.
 *
 *	description: Creates a ray object that can be attached to a SceneNode and
 *	drawn to screen.
 *
 */

#ifndef HYDRA_RAY_OBJECT_HPP
#define HYDRA_RAY_OBJECT_HPP

/// Base class
#include "movable_object.hpp"

#include "math/types.hpp"

// Necessary for ray collisions
#include "ray_cast.hpp"

// Ogre object
#include <OGRE/OgreManualObject.h>

namespace vl
{

class RayObject : public vl::MovableObject
{
public :
	/// @brief Master constructor
	RayObject(std::string const &name, vl::SceneManagerPtr creator);

	/// @brief Slave constructor
	RayObject(vl::SceneManagerPtr creator);

	/// @brief Destructor
	virtual ~RayObject(void);

	void setPosition(Ogre::Vector3 const &pos);

	Ogre::Vector3 const &getPosition(void) const
	{ return _position; }

	void setDirection(Ogre::Vector3 const &dir);

	Ogre::Vector3 const &getDirection(void) const
	{ return _direction; }

	void setMaterial(std::string const &name);

	std::string const &getMaterial(void) const
	{ return _material; }

	/// @brief set the maximum length of the ray
	void setLength(vl::scalar l);

	vl::scalar getLength(void) const
	{ return _length; }

	void setSphereRadius(vl::scalar radius);

	vl::scalar getSphereRadius(void) const
	{ return _sphere_radius; }
	
	/// @brief set on/off the collision detection for the ray
	void setCollisionDetection(bool enable);

	bool getCollisionDetection(void) const
	{ return _collision_detection; }

	/// @brief set on/off whether to draw a sphere where the ray collided
	void setDrawCollisionSphere(bool enable);

	bool getDrawCollisionSphere(void) const
	{ return _draw_collision_sphere; }

	/// @todo add thickness

	virtual Ogre::MovableObject *getNative(void) const
	{ return _ogre_object; }

	virtual std::string getTypeName(void) const
	{ return "RayObject"; }

	/// @todo not implemented
	virtual MovableObjectPtr clone(std::string const &append_to_name) const
	{ return 0; }

	enum DirtyBits
	{
		DIRTY_TRANSFORM = vl::MovableObject::DIRTY_CUSTOM << 0,
		DIRTY_PARAMS = vl::MovableObject::DIRTY_CUSTOM << 1,
		DIRTY_CUSTOM = vl::MovableObject::DIRTY_CUSTOM << 2,
	};

	/// @internal
	/// @brief used to update the ray once per frame before rendering
	/// Handles updates to collision detection
	void _updateRay(void);

private :
	virtual bool _doCreateNative(void);

	virtual void doSerialize(vl::cluster::ByteStream &msg, const uint64_t dirtyBits) const;

	virtual void doDeserialize(vl::cluster::ByteStream &msg, const uint64_t dirtyBits);

	void _clear(void);

	void _create(void);

	void _generateLine(Ogre::Vector3 const &start_point, Ogre::Vector3 const &end_point);

	void _generateCollisionSphere(Ogre::Vector3 const &point);

	Ogre::Vector3 _direction;
	Ogre::Vector3 _position;
	std::string _material;

	vl::scalar _length;

	vl::scalar _sphere_radius;

	// @todo change to flags
	bool _draw_collision_sphere;
	bool _collision_detection;

	Ogre::ManualObject *_ogre_object;

	vl::RayCast *_ray_cast;

	struct RayListener : public vl::MovableObject::Listener
	{
		RayListener(RayObjectPtr ray)
			: Listener(ray)
		{
			std::clog << "vl::RayObject::RayListener::RayListener" << std::endl;
		}

		/// using object rendering callback because any scene object
		/// transformation can affect the ray collision detection
		virtual void frameStart(void);
	};

};	// class RayObject

}	// namespace vl

#endif	// HYDRA_RAY_OBJECT_HPP
