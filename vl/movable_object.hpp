/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-04
 *	@file movable_object.hpp
 *
 */

#ifndef HYDRA_MOVABLE_OBJECT_HPP
#define HYDRA_MOVABLE_OBJECT_HPP

// Base class
#include "distributed.hpp"

#include "typedefs.hpp"

#include "base/string_utils.hpp"

namespace vl
{

/**	@class MovableObject
 *	@brief A base class for anything that can be attached to a SceneNode
 *	Does not distribute anything for now.
 */
class MovableObject : public vl::Distributed
{
public :
	MovableObject(std::string const &name, vl::SceneManagerPtr creator)
		: _name(name), _visible(true), _creator(creator), _parent(0)
	{}

	MovableObject(vl::SceneManagerPtr creator)
		: _name(), _visible(true), _creator(creator), _parent(0)
	{}

	std::string const &getName(void) const
	{ return _name; }

	vl::SceneNodePtr getParent(void) const
	{ return _parent; }

	/// Internal
	void setParent(vl::SceneNodePtr parent);

	virtual Ogre::MovableObject *getNative(void) const = 0;

	virtual std::string getTypeName(void) const = 0;

	void setVisible(bool visible);

	bool getVisible(void) const
	{ return _visible; }

	/// @brief make a deep copy of the SceneNode
	/// Shallow copies would not make much sense with SceneGraphs because you
	/// can not have multiple parents unlike DAGs.
	MovableObjectPtr clone(void) const
	{
		std::string name = vl::generate_random_string();
		assert(!name.empty());

		return clone(name);
	}

	virtual MovableObjectPtr clone(std::string const &append_to_name) const = 0;

	enum DirtyBits
	{
		DIRTY_NAME = vl::Distributed::DIRTY_CUSTOM << 0,
		DIRTY_VISIBLE = vl::Distributed::DIRTY_CUSTOM << 1,
		DIRTY_CUSTOM = vl::Distributed::DIRTY_CUSTOM << 2,
	};

private :
	/// Virtual private methods

	virtual bool _doCreateNative(void) = 0;
	virtual void doSerialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits ) const = 0;
	virtual void doDeserialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits ) = 0;

	/// Using template method pattern for creating the native object
	/// This manages the the common functionality
	bool _createNative(void);

	/// Template method patter for the serialization
	void serialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits ) const;


	void deserialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits );

/// Protected data
protected :
	std::string _name;

	bool _visible;

	vl::SceneManagerPtr _creator;
	
	vl::SceneNodePtr _parent;

};	// class MovableObject

}	// namespace vl

#endif	// HYDRA_MOVABLE_OBJECT_HPP