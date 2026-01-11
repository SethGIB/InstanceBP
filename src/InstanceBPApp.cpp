#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class InstanceBPApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
};

void InstanceBPApp::setup()
{
}

void InstanceBPApp::mouseDown( MouseEvent event )
{
}

void InstanceBPApp::update()
{
}

void InstanceBPApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP( InstanceBPApp, RendererGl )
