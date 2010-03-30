/*	Joonatan Kuosa
 *	2010-03
 *
 *	Small test to test rendering with Ogre using the equalizer
 *	rendering loop.
 *	Very small equalizer and Ogre initialization.
 */
#ifdef VL_UNIX
#define BOOST_TEST_DYN_LINK
#endif

#define BOOST_TEST_MODULE stereo_render

#include <eq/eq.h>

#include <boost/test/unit_test.hpp>

#include "eq_ogre/ogre_root.hpp"
#include "eq_ogre/ogre_scene_manager.hpp"
#include "eq_ogre/ogre_scene_node.hpp"
#include "eq_ogre/ogre_entity.hpp"
#include "eq_ogre/ogre_camera.hpp"
#include "eq_ogre/ogre_render_window.hpp"

#include "eq_test_fixture.hpp"

char *argv[argc] = { "stereo_render\0", "--eq-config\0", "1-window.eqc\0", "\0" };

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
		
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
		eq::WGLWindow *os_win = (eq::WGLWindow *)(getWindow()->getOSWindow());
		std::stringstream ss( std::stringstream::in | std::stringstream::out );
		ss << os_win->getWGLWindowHandle();
		params["externalWindowHandle"] = ss.str();
		ss.str("");
		params["externalGLControl"] = std::string("True");
		ss << os_win->getWGLContext();
		params["externalGLContext"] = ss.str();
#else
		params["currentGLContext"] = std::string("True");
#endif
		
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
		view->setBackgroundColour( vl::colour(1.0, 0.0, 0.0, 0.0) );
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
	}

	virtual void frameDraw( const uint32_t frameID )
	{
		eq::Channel::frameDraw( frameID );

		/*
		 * Example get draw buffer
		int param;
		glGetIntegerv( GL_DRAW_BUFFER, &param );
		std::cout << "Draw buffer = ";
		if ( param == GL_NONE )
		{ std::cout << "GL_NONE" << std::endl; }
		else if ( param == GL_BACK )
		{ std::cout << "GL_BACK" << std::endl; }
		else if ( param == GL_BACK_LEFT )
		{ std::cout << "GL_BACK_LEFT" << std::endl; }
		else if ( param == GL_BACK_RIGHT )
		{ std::cout << "GL_BACK_RIGHT" << std::endl; }
		*/
		/* Example get win32 PixelFormat and check we have stereo
		if( dynamic_cast<eq::WGLWindow *>( getWindow()->getOSWindow() ) )
		{
			eq::WGLWindow *os_win 
				= (eq::WGLWindow *)( getWindow()->getOSWindow() );
			HDC hdc = os_win->getWGLDC();
			int pformat = GetPixelFormat(hdc);
			PIXELFORMATDESCRIPTOR  pfd;
			// obtain a detailed description of that pixel format  
			DescribePixelFormat(hdc, pformat, 
        			sizeof(PIXELFORMATDESCRIPTOR), &pfd);
			std::cout << "pfd.flags = " << pfd.dwFlags << std::endl;
			std::cout << "flags contain";
			if( pfd.dwFlags & PFD_DOUBLEBUFFER )
			{ std::cout << " doublebuffer "; }
			if( pfd.dwFlags & PFD_STEREO )
			{ std::cout << " stereo "; }
			if( pfd.dwFlags & PFD_SUPPORT_OPENGL )
			{ std::cout << " opengl "; }
			std::cout << std::endl;
		}
		*/
		eq::Frustumf frust = getFrustum();
		cam->setProjectionMatrix( frust.compute_matrix() );
		win->update();
	}

	int state;
	vl::graph::Root *ogre_root;
	vl::graph::RenderWindow *win;
	vl::graph::Camera *cam;
	vl::graph::SceneManager *man;
	vl::graph::SceneNode *feet;
	vl::graph::SceneNode *robot;
};

class NodeFactory : public eq::NodeFactory
{
public :
	virtual Channel *createChannel( eq::Window *parent )
	{ return new ::Channel( parent ); }
};

eq::NodeFactory *g_nodeFactory = new ::NodeFactory;

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
