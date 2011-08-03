/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-05
 *	@file  physics/shapes.cpp
 *
 *	This file is part of Hydra VR game engine.
 */

// Declaration
#include "shapes.hpp"

/// Concrete implementations
#ifdef USE_BULLET
#include "shapes_bullet.hpp"
#else if USE_NEWTON
#include "shapes_newton.hpp"
#endif

// @todo the create methods should return concrete bullet or newton objects
vl::physics::BoxShapeRefPtr
vl::physics::BoxShape::create(Ogre::Vector3 const &bounds)
{
	BoxShapeRefPtr box;
#ifdef USE_BULLET
	box.reset(new BulletBoxShape(bounds));
#else if USE_NEWTON
#endif
	return box;
}


vl::physics::SphereShapeRefPtr
vl::physics::SphereShape::create(vl::scalar radius)
{
	SphereShapeRefPtr sphere;
#ifdef USE_BULLET
	sphere.reset(new BulletSphereShape(radius));
#else if USE_NEWTON
#endif
	return sphere;
}

vl::physics::StaticPlaneShapeRefPtr
vl::physics::StaticPlaneShape::create(Ogre::Vector3 const &normal, vl::scalar constant)
{
	StaticPlaneShapeRefPtr plane;
#ifdef USE_BULLET
	plane.reset(new BulletStaticPlaneShape(normal, constant));
#else if USE_NEWTON
#endif
	return plane;
}

vl::physics::StaticTriangleMeshShapeRefPtr
vl::physics::StaticTriangleMeshShape::create(vl::MeshRefPtr mesh)
{
	StaticTriangleMeshShapeRefPtr shape;
#ifdef USE_BULLET
	shape.reset(new BulletStaticTriangleMeshShape(mesh));
#else if USE_NEWTON
#endif
	return shape;
}

vl::physics::ConvexHullShapeRefPtr
vl::physics::ConvexHullShape::create(vl::MeshRefPtr mesh)
{
	ConvexHullShapeRefPtr shape;
#ifdef USE_BULLET
	shape.reset(new BulletConvexHullShape(mesh));
#else if USE_NEWTON
#endif
	return shape;
}
