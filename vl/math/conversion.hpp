/**
 *	Copyright (c) 2010 - 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2010-03
 *	@file math/conversion.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

/**	Functions to convert math objects from one library to another.
 *	Ogre Math is our main math library.
 *
 *	VMMLib <-> Ogre, added 2010-03
 *	VRPN -> Ogre, added 2010-12
 *	VMMLib removed 2011-02
 *	Bullet <-> Ogre added 2011-03
 */

#ifndef VL_MATH_CONVERSION_HPP
#define VL_MATH_CONVERSION_HPP

#if defined _MSC_VER
// Disable truncation warnings, data loss for floats does not concern us
#   pragma warning (disable : 4244)
#endif

#include "math/math.hpp"
#include "math/transform.hpp"

#include <OGRE/OgreVector3.h>
#include <OGRE/OgreQuaternion.h>
#include <OGRE/OgreMatrix4.h>
#include <OGRE/OgreColourValue.h>

// VRPN math types
#include <vrpn_Types.h>
#include <quat.h>

/// Bullet math types
#include <bullet/LinearMath/btVector3.h>
#include <bullet/LinearMath/btQuaternion.h>
#include <bullet/LinearMath/btTransform.h>

namespace vl
{

namespace math
{

// Convert from vrpn_float
inline
Ogre::Vector3 convert_vec( vrpn_float64 const *vec )
{
	return Ogre::Vector3( vec[Q_X], vec[Q_Y], vec[Q_Z] );
}

inline
Ogre::Quaternion convert_quat( vrpn_float64 const *quat )
{
	return Ogre::Quaternion( quat[Q_W], quat[Q_X], quat[Q_Y], quat[Q_Z] );
}

/// Convert bullet vector and quat
inline
Ogre::Vector3 convert_vec(btVector3 const &v)
{
	return Ogre::Vector3(v.x(), v.y(), v.z());
}

inline
Ogre::Quaternion convert_quat(btQuaternion const &q)
{
	return Ogre::Quaternion(q.w(), q.x(), q.y(), q.z());
}

inline
btVector3 convert_bt_vec(Ogre::Vector3 const &v)
{
	return btVector3(v.x, v.y, v.z);
}

inline
btQuaternion convert_bt_quat(Ogre::Quaternion const &q)
{
	return btQuaternion(q.x, q.y, q.z, q.w);
}


/// Convert bullet transform to Hydra transform
inline
vl::Transform convert_transform(btTransform const &t)
{
	return vl::Transform( convert_vec(t.getOrigin()), convert_quat(t.getRotation()) );
}

inline
btTransform convert_bt_transform(vl::Transform const &t)
{
	return btTransform( convert_bt_quat(t.quaternion), convert_bt_vec(t.position) );
}

inline
btTransform convert_bt_transform(Ogre::Quaternion const &q, Ogre::Vector3 const &v)
{
	return btTransform( convert_bt_quat(q), convert_bt_vec(v) );
}

}	// namespace math

}	// namespace vl

#endif
