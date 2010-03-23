/*	Joonatan Kuosa
 *	2010-03
 *
 *	Small test to test rendering with Ogre using the equalizer
 *	rendering loop.
 *	Very small equalizer and Ogre initialization.
 */
//#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE stereo_render

#include <eq/eq.h>

#include <fstream>

#include <boost/test/unit_test.hpp>

#include "eq_ogre/ogre_root.hpp"
#include "eq_ogre/ogre_scene_manager.hpp"
#include "eq_ogre/ogre_scene_node.hpp"
#include "eq_ogre/ogre_entity.hpp"
#include "eq_ogre/ogre_camera.hpp"
#include "eq_ogre/ogre_render_window.hpp"

class Channel : public eq::Channel
{
public :
	Channel( eq::Window *parent )
		: eq::Channel(parent), state(0), ogre_root(0), win(0),
		man(0), feet(0), robot(0)
	{} 

	virtual ~Channel( void )
	{
		delete ogre_root;
	}

	virtual bool configInit( const uint32_t initID )
	{
		BOOST_REQUIRE( eq::Channel::configInit( initID ) );

		// Initialise ogre
		ogre_root = new vl::ogre::Root();
		ogre_root->createRenderSystem();
		vl::NamedValuePairList params;
		
		if( dynamic_cast<eq::WGLWindow *>( getWindow()->getOSWindow() ) )
		{
			eq::WGLWindow *os_win = (eq::WGLWindow *)(getWindow()->getOSWindow());
			std::stringstream ss( std::stringstream::in | std::stringstream::out );
			ss << os_win->getWGLWindowHandle();
			params["externalWindowHandle"] = ss.str();
			ss.str("");
			params["externalGLControl"] = std::string("True");
			ss << os_win->getWGLContext();
			params["externalGLContext"] = ss.str();
		}
		else
		{
			params["currentGLContext"] = std::string("True");
		}
		
		try {
			win = ogre_root->createWindow( "Win", 800, 600, params );
		}
		catch( Ogre::Exception const &e)
		{
			std::cout << "Exception when creating window: " << e.what() 
				<< std::endl;
			throw;
		}
		
		ogre_root->init();

		// Create Scene Manager
		man = ogre_root->createSceneManager("SceneManager");
		BOOST_REQUIRE( man );

		
		// Create camera and viewport
		vl::graph::SceneNode *root = man->getRootNode();
		cam = man->createCamera( "Cam" );
		vl::graph::Viewport *view = win->addViewport( cam );
		view->setBackgroundColour( vmml::vector<4, double>(1.0, 0.0, 0.0, 0.0) );
		feet = root->createChild( "Feet" );
		feet->lookAt( vl::vector(0,0,300) );
		BOOST_CHECK_NO_THROW( feet->attachObject( cam ) );

		// Create robot Entity
		BOOST_REQUIRE( root );
		vl::ogre::Entity *ent = dynamic_cast<vl::ogre::Entity *>(
				man->createEntity( "robot", "robot.mesh" ) );
		ent->load(man);
		robot = root->createChild();
		robot->setPosition( vl::vector(0, 0, 300) );
		BOOST_CHECK_NO_THROW( robot->attachObject( ent ) );
		setNearFar( 100.0, 100.0e3 );

		return true;
	}

	virtual void frameStart( const uint32_t frameID, const uint32_t frameNumber )
	{
		eq::Channel::frameStart( frameID, frameNumber );

		switch( state )
		{
			case 0 :
			{
				++state;
			}
			break;

			case 1 :
			{
				++state;
			}
			break;

			case 2 :
			{
				++state;
			}
			break;
			case 3 :
			{
				++state;
			}
			break;

			case 4 :
			{
				++state;
			}
			break;

			case 5 :
			{ 
				++state;
			}
			break;

			default :
			break;
		}
	}

	virtual void frameDraw( const uint32_t frameID )
	{
		eq::Frustumf frust = getFrustum();
		cam->setProjectionMatrix( frust.compute_matrix() );
		win->update();
		win->swapBuffers();
		
	}

	vl::graph::Root *ogre_root;
	vl::graph::RenderWindow *win;
	vl::graph::Camera *cam;
	vl::graph::SceneManager *man;
	vl::graph::SceneNode *feet;
	vl::graph::SceneNode *robot;
	int state;
};

class NodeFactory : public eq::NodeFactory
{
public :
	virtual Channel *createChannel( eq::Window *parent )
	{ return new ::Channel( parent ); }
};

const int argc = 4;
char NAME[] = "TEST\0";
char *argv[argc] = { NAME, "--eq-config\0", "1-window.eqc\0", "\0" };

struct RenderFixture
{
	// Init code for this test
	RenderFixture( void )
		: error( false ), frameNumber(0), config(0),
		  log_file( "render_test.log" )
	{
		// Redirect logging
		//eq::base::Log::setOutput( log_file );

		// 1. Equalizer initialization
		BOOST_REQUIRE(  eq::init( argc, argv, &nodeFactory ) );
		
		// 2. get a configuration
		config = eq::getConfig( argc, argv );
		BOOST_REQUIRE( config );

		// 3. init config
		BOOST_REQUIRE( config->init(0));
	}

	// Controlled mainloop function so the test can run the loop
	void mainloop( void )
	{
		BOOST_REQUIRE( config->isRunning() );
	
		config->startFrame( ++frameNumber );
		config->finishFrame();
	}

	// Clean up code for this test
	~RenderFixture( void )
	{
		// 5. exit config
		if( config )
		{ BOOST_CHECK( config->exit() ); }

		// 6. release config
		eq::releaseConfig( config );

		// 7. exit
		BOOST_CHECK( eq::exit() );
	}

	bool error;
	uint32_t frameNumber;
	eq::Config *config;
	NodeFactory nodeFactory;
	std::ofstream log_file;
};

BOOST_FIXTURE_TEST_CASE( render_test, RenderFixture )
{
	BOOST_REQUIRE( config );

	for( size_t i = 0; i < 1000; i++ )
	{ mainloop(); }
}
