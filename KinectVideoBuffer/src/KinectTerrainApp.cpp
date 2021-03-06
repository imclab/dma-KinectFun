#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/Surface.h"
#include "cinder/Utilities.h"
#include "cinder/qtime/MovieWriter.h"
#include "Kinect.h"

#include <vector>

using namespace ci;
using namespace ci::app;
using namespace std;

class KinectTerrainApp : public AppBasic {
  public:
	
	void keyDown( KeyEvent event );
	void prepareSettings( Settings* settings );
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
	
private:
	void loadShaders();
	
	Kinect mKinect;
	
	gl::GlslProg mCompositeShader;
	
	int mFrameOffset, mFramesToDraw;
	
	vector<gl::Texture> mDepthSurfaces;
	vector<gl::Texture> mVideoSurfaces;
};

void KinectTerrainApp::prepareSettings( Settings* settings )
{
	settings->setWindowSize( 1280, 960 );
	settings->setFrameRate( 30.0f );
}

void KinectTerrainApp::setup()
{
	loadShaders();
	mKinect = Kinect( Kinect::Device() );
	
	mFrameOffset = 24;
	mFramesToDraw = 8;
	
	for( int i = 0; i < mFrameOffset * mFramesToDraw; i++ )
	{
		mDepthSurfaces.push_back( gl::Texture( 640, 480 ) );
		mVideoSurfaces.push_back( gl::Texture( 640, 480 ) );
	}
}

void KinectTerrainApp::mouseDown( MouseEvent event )
{
}

void KinectTerrainApp::update()
{
	if( mKinect.checkNewDepthFrame() )
	{
		for( int i = mDepthSurfaces.size() - 1; i > 0; i-- )
		{
			mDepthSurfaces[i] = mDepthSurfaces[i-1];
		}
		
		mDepthSurfaces[0] = mKinect.getDepthImage();
	}
	
	if ( mKinect.checkNewVideoFrame() )
	{
		for( int i = mDepthSurfaces.size() - 1; i > 0; i-- )
		{
			mVideoSurfaces[i] = mVideoSurfaces[i-1];
		}
		
		mVideoSurfaces[0] = mKinect.getVideoImage();
	}
	
	
}

void KinectTerrainApp::draw()
{	
	mCompositeShader.bind();
	
	for ( int i=0; i < mFramesToDraw; i++ )
	{
		mVideoSurfaces[i*mFrameOffset].bind( i );
		mDepthSurfaces[i*mFrameOffset].bind( (mFramesToDraw-1) + i );
		
		mCompositeShader.uniform( "color" + toString(i), i );
		mCompositeShader.uniform( "depth" + toString(i), (mFramesToDraw-1) + i );
	}
		
	gl::drawSolidRect( Rectf( 0, 0, getWindowWidth(), getWindowHeight() ) );
}

void KinectTerrainApp::keyDown( KeyEvent event )
{
	switch ( event.getChar() ) {
		case 's':
			loadShaders();
			break;
		case 'i':
			mKinect.setVideoInfrared(true);
			break;
		case 'c':
			mKinect.setVideoInfrared(false);
			break;
		default:
			break;
	}
}

void KinectTerrainApp::loadShaders()
{
	try {
		console() << "Reloading shaders" << endl;
		mCompositeShader = gl::GlslProg( loadResource("Merge.vert"), loadResource("Merge.frag") );
		// when developing on mac, loading this way allow you to edit your shaders without recompiling
		// mCompositeShader = gl::GlslProg( loadFile("../../../resources/Merge.vert"), loadFile("../../../resources/Merge.frag") );
	}
	catch (...) {
		console() << "Ran into problems loading shaders" << endl;
	}
	
	
}

CINDER_APP_BASIC( KinectTerrainApp, RendererGl( RendererGl::AA_MSAA_4 ) )
