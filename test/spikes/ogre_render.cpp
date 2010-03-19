#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE ogre_render

#include <boost/test/unit_test.hpp>

#include "eq_ogre/ogre_root.hpp"
#include "eq_ogre/ogre_scene_manager.hpp"
#include "eq_ogre/ogre_scene_node.hpp"
#include "eq_ogre/ogre_entity.hpp"

struct OgreFixture
{
	OgreFixture( void )
		: ogre_root( 0 ), win(0), robot(0), ent(0)
	{
		ogre_root = new vl::ogre::Root();
		ogre_root->createRenderSystem();
		win = ogre_root->createWindow( "Win", 800, 600 );
		ogre_root->init();

		vl::graph::SceneManager *man = ogre_root->createSceneManager("Manager");
		
		vl::ogre::Camera *cam = dynamic_cast<vl::ogre::Camera *>(
				man->createCamera( "Cam" ) );
 		((Ogre::Camera *)cam->getNative())->setNearClipDistance(0.1);

		win->addViewport( cam )->setBackgroundColour(
				vmml::vector<4, double>(1.0, 0.0, 0.0, 0.0) );

		ent = dynamic_cast<vl::ogre::Entity *>(
				man->createEntity("robot", "robot.mesh") );
		BOOST_REQUIRE( ent );
		BOOST_CHECK_NO_THROW( ent->load(man) );
		robot = dynamic_cast<vl::ogre::SceneNode *>
			( man->getRootNode()->createChild( "RobotNode" ) );
		BOOST_REQUIRE( robot );
		robot->setPosition( vl::vector(0, 0, 300) );
		BOOST_CHECK_NO_THROW( robot->attachObject(ent) );

		feet = dynamic_cast<vl::ogre::SceneNode *>(
				man->getRootNode()->createChild( "feet" ) );
		BOOST_REQUIRE( feet );
		feet->lookAt( vl::vector(0,0,300) );
		BOOST_CHECK_NO_THROW( feet->attachObject( cam ) );
	}

	void mainloop( void )
	{
		win->update();
		win->swapBuffers();
	}

	~OgreFixture( void )
	{
		delete ogre_root;
	}

	vl::ogre::Root *ogre_root;
	vl::graph::RenderWindow *win;
	vl::ogre::SceneNode *robot;
	vl::ogre::SceneNode *feet;
	vl::ogre::Entity *ent;
		
};

BOOST_FIXTURE_TEST_CASE( render_test, OgreFixture )
{
	BOOST_CHECK_EQUAL( robot->getNative()->numAttachedObjects(), 1 );
	BOOST_CHECK_EQUAL( feet->getNative()->numAttachedObjects(), 1 );
	BOOST_CHECK( feet->getNative()->isInSceneGraph() );
	BOOST_CHECK( robot->getNative()->isInSceneGraph() );
	BOOST_CHECK( ent->getNative()->isInScene() );

	for( size_t i = 0; i < 4000; i++ )
	{ mainloop(); }
}
