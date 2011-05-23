
#include "physics_world.hpp"

/// Physics objects
#include "motion_state.hpp"
#include "rigid_body.hpp"
#include "constraints.hpp"

/// -------------------------------- Global ----------------------------------
std::ostream &
vl::physics::operator<<(std::ostream &os, vl::physics::World const &w)
{
	os << "Physics World : "
		<< " gravity " << w.getGravity()
		<< " with " << w._rigid_bodies.size() << " rigid bodies "
		<< "\n";
	
	os << "Bodies : \n";
	RigidBodyList::const_iterator iter;
	for( iter = w._rigid_bodies.begin(); iter != w._rigid_bodies.end(); ++iter )
	{
		os << (*iter)->getName();
		if( iter+1 != w._rigid_bodies.end() )
		{ os << "\n"; }
	}

	return os;
}

/// -------------------------------- Public ----------------------------------
vl::physics::World::World( void )
	: _broadphase( new btDbvtBroadphase() ),
	  _collision_config( new btDefaultCollisionConfiguration() ),
	  _dispatcher( new btCollisionDispatcher(_collision_config) ),
	  _solver( new btSequentialImpulseConstraintSolver )
{
	_dynamicsWorld = new btDiscreteDynamicsWorld(_dispatcher,_broadphase,_solver,_collision_config);
	_dynamicsWorld->setGravity( btVector3(0,-9.81,0) );
}

vl::physics::World::~World( void )
{
	// cleanup the world
	delete _dynamicsWorld;
	delete _solver;
	delete _dispatcher;
	delete _collision_config;
	delete _broadphase;
}

void
vl::physics::World::step( void )
{
	// Some hard-coded parameters for the simulation
	_dynamicsWorld->stepSimulation(1/60.f,10);
}

Ogre::Vector3
vl::physics::World::getGravity(void) const
{
	return vl::math::convert_vec(_dynamicsWorld->getGravity());
}


void
vl::physics::World::setGravity(const Ogre::Vector3& gravity)
{
	_dynamicsWorld->setGravity( vl::math::convert_bt_vec(gravity) );
}

vl::physics::RigidBodyRefPtr
vl::physics::World::createRigidBodyEx(RigidBody::ConstructionInfo const &info)
{
	RigidBodyRefPtr body(new RigidBody(info));
	_addRigidBody(info.name, body);
	return body;
}


vl::physics::RigidBodyRefPtr
vl::physics::World::createRigidBody( const std::string& name, vl::scalar mass,
									 vl::physics::MotionState *state,
									 CollisionShapeRefPtr shape,
									 Ogre::Vector3 const &inertia)
{
	RigidBody::ConstructionInfo info(name, mass, state, shape, inertia);
	return createRigidBodyEx(info);
}

vl::physics::RigidBodyRefPtr
vl::physics::World::getRigidBody( const std::string& name ) const
{
	RigidBodyRefPtr body = _findRigidBody(name);
	if( !body )
	{
		// TODO add a better exception to this
		BOOST_THROW_EXCEPTION( vl::null_pointer() );
	}

	return body;
}

vl::physics::RigidBodyRefPtr
vl::physics::World::removeRigidBody( const std::string& name )
{
	BOOST_THROW_EXCEPTION( vl::not_implemented() );
}

bool
vl::physics::World::hasRigidBody( const std::string& name ) const
{
	return _findRigidBody(name);
}

vl::physics::MotionState *
vl::physics::World::createMotionState( const vl::Transform &trans, vl::SceneNode *node )
{
	return new MotionState( trans, node );
}

void
vl::physics::World::destroyMotionState( vl::physics::MotionState *state )
{
	delete state;
}

void 
vl::physics::World::addConstraint(vl::physics::ConstraintRefPtr constraint, bool disableCollisionBetweenLinked)
{
	ConstraintList::iterator iter = std::find(_constraints.begin(), _constraints.end(), constraint);
	
	if(iter == _constraints.end())
	{
		_constraints.push_back(constraint);
		_dynamicsWorld->addConstraint(constraint->getNative(), disableCollisionBetweenLinked);
	}
}

void 
vl::physics::World::removeConstraint(vl::physics::ConstraintRefPtr constraint)
{
	ConstraintList::iterator iter = std::find(_constraints.begin(), _constraints.end(), constraint);
	
	if(iter != _constraints.end())
	{
		_dynamicsWorld->removeConstraint((*iter)->getNative());
		_constraints.erase(iter);
	}
}

/// --------------------------------- Private ----------------------------------
void
vl::physics::World::_addRigidBody(std::string const &name, vl::physics::RigidBodyRefPtr body)
{
	if( !hasRigidBody(name) )
	{
		_rigid_bodies.push_back(body);
		_dynamicsWorld->addRigidBody(body->getNative());
	}
	else
	{
		std::string err( "RigidBody with that name is already in the scene." );
		BOOST_THROW_EXCEPTION( vl::duplicate() << vl::desc(err) );
	}
}

vl::physics::RigidBodyRefPtr
vl::physics::World::_findRigidBody(const std::string& name) const
{
	RigidBodyList::const_iterator iter;
	for( iter = _rigid_bodies.begin(); iter != _rigid_bodies.end(); ++iter )
	{
		if( (*iter)->getName() == name )
		{ return *iter; }
	}

	return RigidBodyRefPtr();
}
