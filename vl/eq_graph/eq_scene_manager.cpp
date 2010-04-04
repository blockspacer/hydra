#include "eq_scene_manager.hpp"

#include "interface/movable_object.hpp"
#include "interface/entity.hpp"
#include "eq_scene_node.hpp"

#include <eq/net/session.h>

// SceneManager
vl::cl::SceneManager::SceneManager( std::string const &name )
	: _root()
{
	if( name.empty() )
	{ throw vl::empty_param("vl::cl::SceneManager::SceneManager"); }

	eq::Object::setName( name );
}

vl::cl::SceneManager::~SceneManager( void )
{
}

void
vl::cl::SceneManager::destroyGraph( void )
{

}

vl::graph::SceneNodeRefPtr
vl::cl::SceneManager::getRootNode( void )
{
	if( !_root )
	{ _root = createNode( "Root" ); }
	return _root;
}

vl::graph::SceneNodeRefPtr
vl::cl::SceneManager::createNode( std::string const &name )
{
	// TODO replace with the factory
	//	= _createSceneNodeImpl( name );
	if( !_scene_node_factory.get() )
	{ _scene_node_factory.reset(new DefaultSceneNodeFactory); }

	vl::graph::SceneNodeRefPtr node =
		_scene_node_factory->create( shared_from_this(), name );

	// TODO add to nodes, we should also share it here so there
	// will be ID generated for this node
	// Seems not to need to be registered.
	// Um the ID is invalid so we need to register the scene_manager first
	// or register the objects later.
	//getSession()->registerObject( node );

	// This needs the ID to be generated already so we need to
	// register the node first.
	//_nodes->push( node );
	//_nodes->commit();
	return node;
}

vl::graph::EntityRefPtr
vl::cl::SceneManager::createEntity(
		std::string const &name, std::string const &meshName )
{
	vl::NamedValuePairList params;
	params["mesh"] = meshName;
	vl::graph::MovableObjectRefPtr obj = createMovableObject(
			"Entity", name, params );

	return boost::static_pointer_cast<vl::graph::Entity>( obj );
}

vl::graph::MovableObjectRefPtr
vl::cl::SceneManager::createMovableObject(
		std::string const &name, std::string const &typeName,
		vl::NamedValuePairList const &params )
{
	// For now we only use entities
	std::vector<vl::graph::MovableObjectFactoryPtr>::iterator iter;
	for( iter = _movable_factories.begin(); iter != _movable_factories.end();
			++iter )
	{
		if( typeName == (*iter)->typeName() )
		{
			return (*iter)->create( name, params );
		}
	}

	return vl::graph::MovableObjectRefPtr();

	// TODO replace with factory::create
	//return new vl::cl::Entity( name, params );
}

// Find function needs scene graph traversal to be implemented
vl::graph::SceneNodeRefPtr
vl::cl::SceneManager::getNode( std::string const &name )
{
	return vl::graph::SceneNodeRefPtr();
}

// Find function needs scene graph traversal to be implemented
vl::graph::SceneNodeRefPtr
vl::cl::SceneManager::getNode( uint32_t id )
{
	/*
	for( size_t i = 0; i < _nodes->size(); i++ )
	{
		if( _nodes->at(i)->getID() == id )
		{ return _nodes->at(i); }
	}
	*/

	return vl::graph::SceneNodeRefPtr();
}

// Find function needs scene graph traversal to be implemented
vl::graph::MovableObjectRefPtr
vl::cl::SceneManager::getObject( uint32_t id )
{
	/*
	for( size_t i = 0; i < _objects->size(); i++ )
	{
		if( _objects->at(i)->getID() == id )
		{ return _objects->at(i); }
	}
	*/

	return vl::graph::MovableObjectRefPtr();
}

void
vl::cl::SceneManager::pushChildAddedStack( uint32_t id,
		vl::graph::ChildAddedFunctor const &handle )
{

}

void
vl::cl::SceneManager::pushChildRemovedStack( vl::graph::SceneNodeRefPtr child )
{

}

void
vl::cl::SceneManager::setSceneNodeFactory(
		vl::graph::SceneNodeFactoryPtr factory )
{

}

// equalizer overrides
void
vl::cl::SceneManager::serialize( eq::net::DataOStream& os,
		const uint64_t dirtyBits )
{
	if( DIRTY_ROOT & dirtyBits )
	{
	}
}

void
vl::cl::SceneManager::deserialize( eq::net::DataIStream& is,
						  const uint64_t dirtyBits )
{
	if( DIRTY_ROOT & dirtyBits )
	{
	}
}

/*
vl::graph::SceneNodeRefPtr
vl::cl::SceneManager::_createSceneNodeImpl( std::string const &name )
{
	if( !_scene_node_factory.get() )
	{ _scene_node_factory.reset(new DefaultSceneNodeFactory); }

	return _scene_node_factory->create( this, name );
}

vl::graph::MovableObjectRefPtr
vl::cl::SceneManager::_createMovableObjectImpl(
		std::string const &typeName, std::string const &name,
		vl::NamedValuePairList const &params )
{
	// For now we only use entities
	if( typeName != "Entity" )
	{ return 0; }

	// TODO replace with factory::create
	return new vl::cl::Entity( name, params );
}
*/

