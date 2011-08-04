/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2010-03
 *	@file math.hpp
 *
 *	Math helper functions.
 */

#ifndef HYDRA_MATH_MATH_HPP
#define HYDRA_MATH_MATH_HPP

// uint64_t uint32_t uint16_t and uint8_t
#include <stdint.h>

// Necessary for printing
#include <iostream>

// Necessary for the Wall configuration needed for calculating view and frustum matrices
#include "base/envsettings.hpp"

#include "types.hpp"
#include "transform.hpp"

namespace vl
{

inline bool equal( scalar const &a, scalar const &b )
{
	if( a-epsilon < b && a+epsilon > b )
	{ return true; }
	return false;
}

inline bool equal( Ogre::Vector3 const &v1, Ogre::Vector3 const &v2 )
{
	for( size_t i = 0; i < 3; ++i )
	{
		if( !equal( v1[i], v2[i] ) )
		{ return false; }
	}
	return true;
}

inline bool equal( Ogre::Quaternion const &q1, Ogre::Quaternion const &q2 )
{
	for( size_t i = 0; i < 4; ++i )
	{
		if( !equal( q1[i], q2[i] ) )
		{ return false; }
	}
	return true;
}

inline bool equal(vl::Transform const &t1, vl::Transform const &t2)
{
	return equal(t1.position, t2.position) && equal(t1.quaternion, t2.quaternion);
}

inline bool equal( Ogre::Matrix4 const &m1, Ogre::Matrix4 const &m2 )
{
	for( size_t i = 0; i < 4; ++i )
	{
		for( size_t j = 0; j < 4; ++j )
		{
			if( !equal( m1[i][j], m2[i][j] ) )
			{ return false; }
		}
	}
	return true;
}

template<typename T>
inline T clamp(T const &x, T const &min, T const &max)
{
	return (x < min) ? min : ((x > max) ? max : x);
}

template<typename T>
inline T sign(T const &x)
{
	return (x > T(0)) ? T(1) : ((x < T(0)) ? T(-1) : T(0));
}

template<>
inline Ogre::Vector3 sign(Ogre::Vector3 const &x)
{
	return Ogre::Vector3(sign(x.x), sign(x.y), sign(x.z));
}

template<typename T>
inline T abs(T const &x)
{
	return (x > T(0)) ? x : -x;
}

template<typename T>
inline T const &max(T const &a, T const &b)
{
	return (a > b ? a : b);
}

template<typename T>
inline T const &min(T const &a, T const &b)
{
	return (a < b ? a : b);
}

void getEulerAngles( Ogre::Quaternion const &q, Ogre::Radian &x, Ogre::Radian &y, Ogre::Radian &z );

void fromEulerAngles( Ogre::Quaternion &q, Ogre::Radian const &rad_x,
						Ogre::Radian const &rad_y, Ogre::Radian const &rad_z );

/// @brief calculate projetion matrix using VR parameters
/// @param near_plane the near clipping plane distance
/// @param far_plane the far clipping plane distance
/// @param wall the projection wall/screen used for this frustum
/// @param head the head transformation used for this projection
/// @return OpenGL projection matrix
/// @todo add asymmetric stereo frustum support (needs head rotation)
Ogre::Matrix4
calculate_projection_matrix(Ogre::Real near_plane, Ogre::Real far_plane, 
							vl::EnvSettings::Wall const &wall,
							Ogre::Vector3 const &head);

/// @brief calculate the orientation from eye space to wall
Ogre::Quaternion orientation_to_wall(vl::EnvSettings::Wall const &wall);

}	// namespace vl

#endif	// HYDRA_MATH_MATH_HPP
