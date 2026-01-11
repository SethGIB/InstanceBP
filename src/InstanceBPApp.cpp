#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/CameraUi.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class InstanceBPApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;

private:
	CameraPersp mCamera;
	CameraUi mNavCtrl;

	gl::GlslProgRef mShader;
	gl::BatchRef mInstancedObjects;

	void setupShaders();
	void setupInstances();
	void setupCamera();

};

const unsigned int kWindowWidth = 1000;
const unsigned int kWindowHeight = 1000;

const unsigned int kNumX = 50;
const unsigned int kNumY = 50;
const unsigned int kNumZ = 50;

const float kGridWidth = 10.0f;
const float kGridHeight = 10.0f;
const float kGridDepth = 10.0f;

//sphere radius
const float kPointSize = 0.01f;
const float kPointRes = 8.0f;

void InstanceBPApp::setup()
{
	setupCamera();
	setupInstances();
}

void InstanceBPApp::mouseDown( MouseEvent event )
{
}

void InstanceBPApp::update()
{
}

void InstanceBPApp::draw()
{
	gl::clear(Color(0, 0, 0));
	gl::enableDepthRead();
	gl::setMatrices(mCamera);
	mInstancedObjects->drawInstanced(kNumX * kNumY * kNumZ);
}

void InstanceBPApp::setupShaders()
{
	auto vert_prog = CI_GLSL(150,
		uniform mat4 ciModelViewProjection;
		uniform mat3 ciNormalMatrix;

		in vec4 ciPosition;
		in vec2 ciTexCoord0;
		in vec3 ciNormal;

		// per instance
		in vec3 instXYZ;
		in vec3 instRGB;
		
		out lowp vec4 Color;
		out highp vec3 Normal;

		void main(void)
		{
			gl_Position = ciModelViewProjection * (4.0 * ciPosition + vec4(instXYZ, 0));
			Color = vec4(instRGB, 1.0f);
			Normal = ciNormalMatrix * ciNormal;
		}
	);

	auto frag_prog = CI_GLSL(150,
		in vec4	Color;
		in vec3	Normal;
		in vec2	TexCoord;

		out vec4 oColor;

		void main(void)
		{
			vec3 normal = normalize(-Normal);
			float diffuse = max(dot(normal, vec3(0, 0, -1)), 0);
			oColor = Color * diffuse;
		}
	);

	mShader = gl::GlslProg::create(vert_prog, frag_prog);
}

void InstanceBPApp::setupCamera()
{
	mCamera.setAspectRatio(getWindowAspectRatio());
	mCamera.lookAt(vec3(0, 0, -7.5f), vec3(0), vec3(0, 1, 0));
	mNavCtrl = CameraUi(&mCamera, getWindow());
}

void InstanceBPApp::setupInstances()
{
	/*
		To draw instances, we need the following things (in Cinder parlance):
		1) data - this could be a list of custom structs or just something simple like a list of positions or colors as vec<n>
		2) gl::VboRef - this is our actual per-instance data. We take this and apply it to our VboMesh
			2a) geom::BufferLayout - this determines "where" our attribute actually is, i.e. how how our data is mapped into the shader/instance controller
		3) gl::VboMeshRef - this is the container for our instance source. Think static mesh uAsset
		4) gl::GlslProgRef - this is the shader. In this case, beyond material properties, it applies per-instance props (position, etc). Essentially our instance controller
		5) gl::BatchRef - this is the thing we actually draw. Holds instance mesh, shader, and attribute mapping
		Follow along:
	*/

	// 1) Data. Just a 3d grid of points stored as vec3s and colors also stored as vec3s
	//	You can use cinder's color types too
	float halfx = kGridWidth * 0.5f;
	float halfy = kGridHeight * 0.5f;
	float halfz = kGridDepth * 0.5f;

	vector<vec3> position_data;
	vector<vec3> color_data;
	for (int y = 0; y < kNumY; y++)
	{
		float py = lmap((float)y + 0.5f, 0.0f, (float)kNumY, -halfy, halfy);
		float r = (float)y / kNumY;
		for (int z = 0; z < kNumZ; z++)
		{
			float pz = lmap((float)z + 0.5f, 0.0f, (float)kNumZ, -halfz, halfz);
			float g = (float)z / kNumZ;
			for (int x = 0; x < kNumX; x++)
			{
				float px = lmap((float)x + 0.5f, 0.0f, (float)kNumX, -halfx, halfx);
				float b = (float)x / kNumX;
				position_data.push_back(vec3(px, py, pz));
				color_data.push_back(vec3(r, g, b));
			}
		}
	}

	// 2) Vbo (vertex buffer object) setup
	gl::VboRef instance_pos_data = gl::Vbo::create(GL_ARRAY_BUFFER, position_data.size() * sizeof(vec3), position_data.data(), GL_DYNAMIC_DRAW);
	gl::VboRef instance_rgb_data = gl::Vbo::create(GL_ARRAY_BUFFER, color_data.size() * sizeof(vec3), color_data.data(), GL_DYNAMIC_DRAW);
	
	// 2a) Buffer Layout
	geom::BufferLayout position_layout;
	position_layout.append(geom::Attrib::CUSTOM_0, 3, 0, 0, 1);
	
	geom::BufferLayout color_layout;
	color_layout.append(geom::Attrib::CUSTOM_1, 3, 0, 0, 1);

	// 3) VboMesh setup
	gl::VboMeshRef instance_mesh = gl::VboMesh::create(geom::Sphere().radius(kPointSize).subdivisions(kPointRes));
	instance_mesh->appendVbo(position_layout, instance_pos_data);
	instance_mesh->appendVbo(color_layout, instance_rgb_data);

	// 4) Shader/GlslProg Setup ///
	setupShaders();

	// 5) BatchRef setup
	mInstancedObjects = gl::Batch::create(instance_mesh, mShader, { {geom::Attrib::CUSTOM_0, "instXYZ"}, {geom::Attrib::CUSTOM_1, "instRGB"} });
}

void prepareSettings(App::Settings* settings)
{
	settings->setWindowSize(kWindowWidth, kWindowHeight);
}

CINDER_APP( InstanceBPApp, RendererGl, prepareSettings )
