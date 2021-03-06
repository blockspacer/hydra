/**
 *	Copyright (c) 2011 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2010-11
 *	@file physics/physics_world.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

/**
 *	Physics World used to initialise the physics with some default
 *	values. Later they might be controllable by the user.
 *	Provides object management for and the general interface for physics engine world.
 *
 *	@update 2011-07
 *	Removed dependencies to Bullet for selectable physics engine.
 *
 */

#ifndef HYDRA_PHYSICS_WORLD_HPP
#define HYDRA_PHYSICS_WORLD_HPP

#include "typedefs.hpp"
// Necessary for time step
#include "base/time.hpp"
// Necessary for vl::scalar
#include "math/types.hpp"
// Necesessary for Transform
#include "math/transform.hpp"
// Necessary for RigidBody::ConstructionInfo
#include "rigid_body.hpp"
// Necessary for Tube::ConstructionInfo
#include "tube.hpp"
// Necessary for HYDRA_API
#include "defines.hpp"

#include <vector>
#include <string>

namespace vl
{

namespace physics
{

/// @struct SolverParameters
/// @brief global parameters for the physics solver
/// Small internal timestep is necessary for small objects < 100mm
struct SolverParameters
{
	SolverParameters(void)
		: erp(0.2)
		, erp2(0.1)
		, global_cfm(0)
		, restitution(0)
		, max_error_reduction(20)
		, internal_time_step(1./60.0)
		, max_sub_steps(10)
	{}

	vl::scalar erp;
	vl::scalar erp2;
	vl::scalar global_cfm;
	vl::scalar restitution;
	vl::scalar max_error_reduction;
	vl::scalar internal_time_step;
	int max_sub_steps;
};


struct RayHitResult
{
	RigidBodyRefPtr hit_object;
	Ogre::Vector3 hit_point_world;
	Ogre::Vector3 hit_normal_world;
	vl::scalar hit_fraction;
	Ogre::Vector3 ray_start;
	Ogre::Vector3 ray_end;
	
	RayHitResult(void) : hit_point_world(vl::Vector3::ZERO), hit_normal_world(vl::Vector3::ZERO),
						hit_fraction(0.0), ray_start(vl::Vector3::ZERO), ray_end(vl::Vector3::ZERO)
	{}

	bool operator==(RayHitResult const &other) const
	{return hit_object == other.hit_object;}

	bool operator<(RayHitResult const &other) const
	{return hit_fraction < other.hit_fraction;}
	
	static bool smallest_hit_fraction_first(RayHitResult const &a, RayHitResult const &b)
	{ return a.hit_fraction < b.hit_fraction; }
	
	
};

inline std::ostream &operator<<(std::ostream &os, RayHitResult const &res)
	{ 
		os << "----RayHitResult----" << std::endl;
		os << "Ray beginning in world coordinates: " << res.ray_start << " and end: " << res.ray_end << std::endl;
		os << "Hit fraction: " << res.hit_fraction << std::endl;
		if(res.hit_object)
		{os << "Hit object: " << res.hit_object->getName() << std::endl;}
		else
		{os << "Hit object: " << "None" << std::endl;}
		os << "Hit point in world coordinates: " << res.hit_point_world << std::endl;
		os << "Hit normal in world coordinates: " << res.hit_normal_world << std::endl;
		return os;
	}
//Comparison operator for sorting rayhitresults:
/*
struct RayHitResultComparator
{
	bool operator()(RayHitResult const &a, RayHitResult &b) const {
		return a.hit_fraction < b.hit_fraction;
	}
};
*/


/** @class World
 *	Interface for physics world, provides concrete implementations of object
 *	management using our wrapper objects.
 *	Provides abstract interface for physics engine implementation.
 */
class HYDRA_API World
{
public :
	static WorldRefPtr create(GameManager *man);

	virtual ~World(void);

	virtual void step(vl::time const &time_step) = 0;

	virtual Ogre::Vector3 getGravity(void) const = 0;

	virtual void setGravity(Ogre::Vector3 const &gravity) = 0;

	virtual void setSolverParameters(SolverParameters const &p) = 0;

	virtual SolverParameters const &getSolverParameters(void) const = 0;

	/// @brief Remove everything from the world
	void removeAll(void);

	/// @brief remove at run time created objects
	void destroyDynamicObjects(void);

	/// ---------------------- RigidBodies ------------------
	/// @TODO replace name, when you have the time to fix the overloads for python
	
	vl::physics::RigidBodyRefPtr createRigidBodyEx(RigidBody::ConstructionInfo const &info);

	/// Default inertia is zero because setting it will mess up static objects. 
	/// For dynamic objects user should set it.
	/// Automatically adds the RigidBody to the world.
	vl::physics::RigidBodyRefPtr createRigidBody( std::string const &name, vl::scalar mass,
								  vl::physics::MotionState *state, CollisionShapeRefPtr shape,
								  Ogre::Vector3 const &inertia = Ogre::Vector3(0, 0, 0) );

	RigidBodyRefPtr createDynamicRigidBody(std::string const &name, vl::scalar mass,
								  MotionState *state, CollisionShapeRefPtr shape,
								  Ogre::Vector3 const &inertia = Ogre::Vector3(0, 0, 0) );

	vl::physics::RigidBodyRefPtr getRigidBody( std::string const &name ) const;

	/// @brief Remove rigid body from the World and return it
	/// After this is called the rigid body can be stored for later use
	/// or if it's left to go out of scope it will be destroyed.
	/// @return Silently retun NULL if no such body exists
	vl::physics::RigidBodyRefPtr removeRigidBody( std::string const &name );

	/// @brief Removes rigid body from the World
	void removeRigidBody(vl::physics::RigidBodyRefPtr body);

	bool hasRigidBody( std::string const &name ) const;


	/// ---------------------- MotionStates ------------------
	/// @todo this can be removed as we can use MotionState::create directly
	MotionState *createMotionState(vl::Transform const &trans = vl::Transform(), vl::ObjectInterface *node = 0);

	void destroyMotionState(vl::physics::MotionState *state);

	/// ---------------------- Constraints ------------------
	/// @brief add a constraint i.e. a joint to the world
	/// @param constraint the constraint to add to the world
	/// @param disableCollisionBetweenLinked no collision detection between linked rigid bodies
	/// The constraint is only active if it's added to the current world.
	/// Otherwise it's created and defined but not used in the calculations.
	/// @todo should this throw if the constraint already exists?
	void addConstraint(vl::physics::ConstraintRefPtr constraint, bool disableCollisionBetweenLinked = false);

	void removeConstraint(vl::physics::ConstraintRefPtr constraint);

	bool hasConstraint(vl::physics::ConstraintRefPtr constraint) const;

	bool hasConstraint(std::string const &name) const;

	vl::physics::ConstraintRefPtr getConstraint(std::string const &name) const;

	/// ----------------------- Tubes --------------------------
	/// All tubes are treated as dynamic for the time beign
	/// as there is no way to save them into a scene file.
	TubeRefPtr createTubeEx(Tube::ConstructionInfo const &info);

	TubeRefPtr createTube(RigidBodyRefPtr start_body, RigidBodyRefPtr end_body,
		vl::scalar length, vl::scalar radius = 0.1, vl::scalar mass_per_meter = 1.0);

	bool hasTube(TubeConstRefPtr tube) const;

	void removeTube(vl::physics::TubeRefPtr tube);

	virtual RayHitResultList castAllHitRay(Ogre::Vector3 const &rayfrom, Ogre::Vector3 const &rayto) const = 0;
	virtual RayHitResultList castFirstHitRay(Ogre::Vector3 const &rayfrom, Ogre::Vector3 const &rayto) const = 0;
	
	RigidBodyList const &getBodies(void) const
	{ return _rigid_bodies; }
	
	std::vector<TubeRefPtr> const &getTubes(void) const
	{ return _tubes; }
	
	ConstraintList const &getConstraints(void) const
	{ return _constraints; }

	// @brief toggle collision detection on/off
	// works for both already created bodies and new bodies
	void enableCollisionDetection(bool enable);

	bool isCollisionDetectionEnabled(void) const
	{ return _collision_detection_enabled; }

	friend std::ostream &operator<<(std::ostream &os, World const &w);

protected :

	// Protected because this is abstract class
	World(void);

	// Real engine implementation using template method pattern
	virtual void _addConstraint(vl::physics::ConstraintRefPtr constraint, bool disableCollisionBetweenLinked) = 0;
	virtual void _addRigidBody( std::string const &name, vl::physics::RigidBodyRefPtr body, bool kinematic) = 0;

	virtual void _removeConstraint(vl::physics::ConstraintRefPtr constraint) = 0;
	virtual void _removeBody(vl::physics::RigidBodyRefPtr body) = 0;


	RigidBodyRefPtr _findRigidBody( std::string const &name ) const;

	/// Rigid bodies
	/// World owns all of them
	RigidBodyList _rigid_bodies;
	ConstraintList _constraints;
	std::vector<TubeRefPtr> _tubes;

	bool _collision_detection_enabled;

	GameManager *_game;

};	// class World

std::ostream &operator<<(std::ostream &os, World const &w);

}	// namespace physics

}	// namespace vl

#endif	// HYDRA_PHYSICS_WORLD_HPP
