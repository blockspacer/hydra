#include "ogre_scene_node.hpp"

#include "ogre_scene_manager.hpp"
#include "ogre_camera.hpp"
#include "ogre_entity.hpp"

vl::ogre::SceneNode::SceneNode( vl::graph::SceneManagerRefPtr creator,
		std::string const &name )
	: vl::cl::SceneNode( creator, name ), _ogre_node(0)
{
	boost::shared_ptr<SceneManager> man
		= boost::dynamic_pointer_cast<SceneManager>( creator );
	// TODO this should throw failed dynamic cast
	if( !man )
	{ throw vl::null_pointer( "vl::ogre::SceneNode::translate" ); }

	if( name == "Root" )
	{ _ogre_node = man->getNative()->getRootSceneNode(); }
	else if( name.empty() )
	{ _ogre_node = man->getNative()->createSceneNode(); }
	else
	{ _ogre_node = man->getNative()->createSceneNode( name ); }
}

vl::ogre::SceneNode::~SceneNode( void )
{
	// TODO delete Ogre::SceneNode
}

void
vl::ogre::SceneNode::translate( vl::vector const &v,
		TransformSpace relativeTo )
{
	vl::cl::SceneNode::translate(v, relativeTo);
	if( !_ogre_node )
	{ throw vl::null_pointer( "vl::ogre::SceneNode::translate" ); }
	_ogre_node->translate( vl::math::convert( v ) );
}

void
vl::ogre::SceneNode::setPosition( vl::vector const &v,
		TransformSpace relativeTo )
{
	vl::cl::SceneNode::setPosition(v, relativeTo);
	if( !_ogre_node )
	{ throw vl::null_pointer( "vl::ogre::SceneNode::setPosition" ); }
	_ogre_node->setPosition( vl::math::convert( v ) );
}

void
vl::ogre::SceneNode::rotate( vl::quaternion const &q,
		TransformSpace relativeTo )
{
	vl::cl::SceneNode::rotate(q, relativeTo);
	if( !_ogre_node )
	{ throw vl::null_pointer( "vl::ogre::SceneNode::rotate" ); }
	_ogre_node->rotate( vl::math::convert(q), Ogre::Node::TS_WORLD );
}

void
vl::ogre::SceneNode::setOrientation( vl::quaternion const &q,
		TransformSpace relativeTo )
{
	vl::cl::SceneNode::setOrientation(q, relativeTo);
	if( !_ogre_node )
	{ throw vl::null_pointer( "vl::ogre::SceneNode::setOrientation" ); }
	_ogre_node->setOrientation( vl::math::convert(q) );
}

void
vl::ogre::SceneNode::lookAt( vl::vector const &v )
{
	vl::cl::SceneNode::lookAt(v);
	if( !_ogre_node )
	{ throw vl::null_pointer( "vl::ogre::SceneNode::lookAt" ); }
	_ogre_node->lookAt( vl::math::convert(v), Ogre::Node::TS_WORLD );
}
void
vl::ogre::SceneNode::scale( vl::vector const &s )
{
	vl::cl::SceneNode::scale(s);
	if( !_ogre_node )
	{ throw vl::null_pointer( "vl::ogre::SceneNode::scale" ); }
	_ogre_node->scale( vl::math::convert(s) );
}

void
vl::ogre::SceneNode::scale( vl::scalar const s )
{
	vl::cl::SceneNode::scale(s);
	if( !_ogre_node )
	{ throw vl::null_pointer( "vl::ogre::SceneNode::scale" ); }
	_ogre_node->scale( s, s, s );
}

void
vl::ogre::SceneNode::setScale( vl::vector const &s )
{
	vl::cl::SceneNode::setScale(s);
	if( !_ogre_node )
	{ throw vl::null_pointer( "vl::ogre::SceneNode::setScale" ); }
	_ogre_node->setScale( vl::math::convert(s) );
}

void
vl::ogre::SceneNode::attachObject( vl::graph::MovableObjectRefPtr object )
{
	vl::cl::SceneNode::attachObject( object );

	boost::shared_ptr<vl::ogre::MovableObject> og_mov
		= boost::dynamic_pointer_cast<MovableObject>(object);
	if( !og_mov )
	{
		throw vl::exception( "vl::ogre::SceneNode::attachObject",
				"no movable object" );
	}

	Ogre::MovableObject *ogre_object = og_mov->getNative();
	if( ogre_object )
	{ this->getNative()->attachObject( ogre_object ); }
	else
	{ throw vl::null_pointer("vl::ogre::SceneNode::attachObject"); }
}

void
vl::ogre::SceneNode::detachObject( vl::graph::MovableObjectRefPtr object )
{
	vl::cl::SceneNode::detachObject( object );

	boost::shared_ptr<vl::ogre::MovableObject> og_mov
		= boost::dynamic_pointer_cast<MovableObject>(object);
	if( !og_mov )
	{
		throw vl::exception( "vl::ogre::SceneNode::attachObject",
				"no movable object" );
	}

	Ogre::MovableObject *ogre_object = og_mov->getNative();
	if( ogre_object )
	{ this->getNative()->detachObject( ogre_object ); }
	else
	{ throw vl::null_pointer("vl::ogre::SceneNode::detachObject"); }
}

void
vl::ogre::SceneNode::setParent( vl::graph::SceneNodeRefPtr parent )
{
	vl::cl::SceneNode::setParent( parent );

	boost::shared_ptr<SceneNode> og_parent
		= boost::dynamic_pointer_cast<SceneNode>( parent );
	og_parent->getNative()->addChild( this->getNative() );
}

void
vl::ogre::SceneNode::addChild( vl::graph::SceneNodeRefPtr child )
{
	vl::cl::SceneNode::addChild( child );

	boost::shared_ptr<SceneNode> og_child
		= boost::dynamic_pointer_cast<SceneNode>( child );
	this->getNative()->addChild( og_child->getNative() );
}

