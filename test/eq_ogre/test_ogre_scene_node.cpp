#ifdef VL_UNIX
#define BOOST_TEST_DYN_LINK
#endif

#define BOOST_TEST_MODULE ogre_scene_node

#include <boost/test/unit_test.hpp>

#include "eq_ogre/ogre_scene_node.hpp"
#include "eq_ogre/ogre_scene_manager.hpp"

#include "math/conversion.hpp"

#include <OgreSceneManagerEnumerator.h>

#include "../fixtures.hpp"

BOOST_GLOBAL_FIXTURE( InitFixture )

BOOST_AUTO_TEST_CASE( constructors_test )
{
	Ogre::SceneManager *og_sm = new Ogre::DefaultSceneManager("Man");
	boost::shared_ptr<vl::ogre::SceneManager> man( new vl::ogre::SceneManager( og_sm, "Man") );
	boost::shared_ptr<vl::ogre::SceneNode> n1, n2;
	BOOST_CHECK_NO_THROW( n1.reset( new vl::ogre::SceneNode(man, "Node") ) );
	BOOST_CHECK_NO_THROW( n2.reset( new vl::ogre::SceneNode(man) ) );
	BOOST_CHECK_EQUAL( Ogre::Vector3(0, 0, 0), n1->getNative()->getPosition() );
	BOOST_CHECK_EQUAL( Ogre::Quaternion::IDENTITY,
			n1->getNative()->getOrientation() );
	BOOST_CHECK_EQUAL( 0, n1->getNative()->numAttachedObjects() );
	BOOST_CHECK_EQUAL( 0, n1->getNative()->numChildren() );
}

// TODO test factory

BOOST_AUTO_TEST_CASE( transform_test )
{
	Ogre::SceneManager *og_sm = new Ogre::DefaultSceneManager("Man");
	boost::shared_ptr<vl::ogre::SceneManager> man( new vl::ogre::SceneManager( og_sm, "Man") );
	boost::shared_ptr<vl::ogre::SceneNode> n1( new vl::ogre::SceneNode(man, "Node1") );

	// Translate
	vl::vector v(10,0,20);
	n1->translate(v);
	BOOST_CHECK_EQUAL( n1->getNative()->getPosition(), vl::math::convert(v) );
	n1->translate(v);
	BOOST_CHECK_EQUAL( n1->getNative()->getPosition(), vl::math::convert(2.0*v) );
	n1->setPosition(v);
	BOOST_CHECK_EQUAL( n1->getNative()->getPosition(), vl::math::convert(v) );
	n1->setPosition(vl::vector(0,0,0));
	BOOST_CHECK_EQUAL( n1->getNative()->getPosition(), Ogre::Vector3(0,0,0) );
	
	// Orientation
	// 90 degree rotation around x
	vl::scalar half_a = (M_PI/2)/2;
	vl::quaternion q( std::sin(half_a), 0, 0, std::cos(half_a) );
	n1->rotate(q);
	BOOST_CHECK( vl::equal( vl::math::convert( n1->getNative()->getOrientation() ), q ) );
	n1->rotate(q);
	vl::quaternion orient = vl::math::convert(n1->getNative()->getOrientation());
	BOOST_CHECK( vl::equal( orient, q*q ) );
	// 45 degree rotation around y
	half_a = (M_PI/4)/2;
	q = vl::quaternion(0, std::sin(half_a), 0, std::cos(half_a));
	n1->setOrientation(q);
	BOOST_CHECK( vl::equal( vl::math::convert( n1->getNative()->getOrientation() ), q ) );
	n1->setOrientation(vl::quaternion::IDENTITY);
	BOOST_CHECK( vl::equal( vl::quaternion::IDENTITY, 
			vl::math::convert( n1->getNative()->getOrientation() ) ) );

	// Scale
	vl::vector s(1, 5, 10);
	n1->scale( s );
	BOOST_CHECK_EQUAL( n1->getNative()->getScale(), vl::math::convert(s) );
	n1->scale( 2 );
	BOOST_CHECK_EQUAL( n1->getNative()->getScale(), vl::math::convert(2.0*s) );
	n1->setScale( 1 );
	BOOST_CHECK_EQUAL( n1->getNative()->getScale(), Ogre::Vector3(1,1,1) );
	n1->setScale(s);
	BOOST_CHECK_EQUAL( n1->getNative()->getScale(), vl::math::convert(s) );
	n1->setScale(1);
}

BOOST_AUTO_TEST_CASE( child_test )
{
	Ogre::SceneManager *og_sm = new Ogre::DefaultSceneManager("Man");
	boost::shared_ptr<vl::ogre::SceneManager>
		man( new vl::ogre::SceneManager( og_sm, "Man") );

//	man->setSceneNodeFactory(
//			vl::graph::SceneNodeFactoryPtr( new vl::ogre::SceneNodeFactory ) );
	boost::shared_ptr<vl::ogre::SceneNode> //root,
		parent( new vl::ogre::SceneNode(man, "parent") ),
		n1( new vl::ogre::SceneNode(man, "Node1") ),
		n2( new vl::ogre::SceneNode(man, "Node2") );

//	root = boost::dynamic_pointer_cast<vl::ogre::SceneNode>( man->getRootNode() );
//	BOOST_REQUIRE( root );
	Ogre::SceneNode *ogre_parent;
	BOOST_CHECK( ogre_parent = parent->getNative() );
	BOOST_CHECK_EQUAL( ogre_parent->numChildren(), 0 );

	BOOST_CHECK_EQUAL( n1->getNative()->getName(), "Node1" );
	BOOST_CHECK_EQUAL( n2->getNative()->getName(), "Node2" );

	BOOST_CHECK_NO_THROW( parent->addChild( n1 ) );
	BOOST_REQUIRE_EQUAL( ogre_parent->numChildren(), 1 );
	// Check that we have correct childs
	BOOST_REQUIRE_EQUAL( ogre_parent->getChild(0)->getName(), n1->getName() );
	BOOST_CHECK_NO_THROW( parent->addChild( n2 ) );
	BOOST_REQUIRE_EQUAL( ogre_parent->numChildren(), 2 );
	BOOST_CHECK_EQUAL( ogre_parent->getChild(0)->getName(), n2->getName() );
}

BOOST_AUTO_TEST_CASE( attachement_test )
{
	Ogre::SceneManager *og_sm = new Ogre::DefaultSceneManager("Man");
	boost::shared_ptr<vl::ogre::SceneManager>
		man( new vl::ogre::SceneManager( og_sm, "Man") );
	boost::shared_ptr<vl::ogre::SceneNode>
		node( new vl::ogre::SceneNode(man, "parent") );
}