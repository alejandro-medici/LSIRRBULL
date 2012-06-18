#include <irrlicht.h>
#include <iostream>
#include "driverChoice.h"

#include "Laser.h"

using namespace irr;

#ifdef _MSC_VER
#pragma comment(lib, "Irrlicht.lib")
#endif

/*
Because we want to use some interesting shaders in this tutorials, we need to
set some data for them to make them able to compute nice colors. In this
example, we'll use a simple vertex shader which will calculate the color of the
vertex based on the position of the camera.
For this, the shader needs the following data: The inverted world matrix for
transforming the normal, the clip matrix for transforming the position, the
camera position and the world position of the object for the calculation of the
angle of light, and the color of the light. To be able to tell the shader all
this data every frame, we have to derive a class from the
IShaderConstantSetCallBack interface and override its only method, namely
OnSetConstants(). This method will be called every time the material is set.
The method setVertexShaderConstant() of the IMaterialRendererServices interface
is used to set the data the shader needs. If the user chose to use a High Level
shader language like HLSL instead of Assembler in this example, you have to set
the variable name as parameter instead of the register index.
*/

#define RESOURCE_PATH "../libs/graphics3D/media/"


IrrlichtDevice* device = 0;
bool UseHighLevelShaders = false;

s32 newMaterialType1 = 0;
s32 newMaterialType2 = 0;
s32 newMaterialBloomType = 0;

class MyShaderCallBack : public video::IShaderConstantSetCallBack
{
public:

	virtual void OnSetConstants(video::IMaterialRendererServices* services,
			s32 userData)
	{
		video::IVideoDriver* driver = services->getVideoDriver();

		// set inverted world matrix
		// if we are using highlevel shaders (the user can select this when
		// starting the program), we must set the constants by name.

		core::matrix4 invWorld = driver->getTransform(video::ETS_WORLD);
		invWorld.makeInverse();

		services->setVertexShaderConstant("mInvWorld", invWorld.pointer(), 16);

		// set clip matrix

		core::matrix4 worldViewProj;
		worldViewProj = driver->getTransform(video::ETS_PROJECTION);
		worldViewProj *= driver->getTransform(video::ETS_VIEW);
		worldViewProj *= driver->getTransform(video::ETS_WORLD);

		services->setVertexShaderConstant("mWorldViewProj", worldViewProj.pointer(), 16);
	
		// set camera position

		core::vector3df pos = device->getSceneManager()->
			getActiveCamera()->getAbsolutePosition();

		services->setVertexShaderConstant("mLightPos", reinterpret_cast<f32*>(&pos), 3);
	
		// set light color

		video::SColorf col(0.0f,1.0f,1.0f,0.0f);

		services->setVertexShaderConstant("mLightColor",
				reinterpret_cast<f32*>(&col), 4);

		// set transposed world matrix

		core::matrix4 world = driver->getTransform(video::ETS_WORLD);
		world = world.getTransposed();

		services->setVertexShaderConstant("mTransWorld", world.pointer(), 16);
	}
};

int main()
{
	// ask user for driver
	video::E_DRIVER_TYPE driverType= video::EDT_OPENGL;//driverChoiceConsole();

	// create device

	device = createDevice(driverType, core::dimension2d<u32>(1024, 600));

	if (device == 0)
		return 1; // could not create selected driver.


	video::IVideoDriver* driver = device->getVideoDriver();
	scene::ISceneManager* smgr = device->getSceneManager();
	gui::IGUIEnvironment* gui = device->getGUIEnvironment();

	io::path currentPath = device->getFileSystem()->getAbsolutePath(RESOURCE_PATH);

	device->getFileSystem()->changeWorkingDirectoryTo(currentPath);

	/*
	We only need the right filenames now. This is
	done in the following switch. Note, that it is not necessary to write
	the shaders into text files, like in this example. You can even write
	the shaders directly as strings into the cpp source file, and use later
	addShaderMaterial() instead of addShaderMaterialFromFiles().
	*/

	io::path vsFileName; // filename for the vertex shader
	io::path psFileName; // filename for the pixel shader
	io::path psBloomFileName; // filename for the pixel shader

	switch(driverType)
	{
	case video::EDT_OPENGL:
		{
			psBloomFileName = "bloom.frag";
			psFileName = "opengl.frag";
			vsFileName = "opengl.vert";
		}
		break;
	}

	/*
	In addition, we check if the hardware and the selected renderer is
	capable of executing the shaders we want. If not, we simply set the
	filename string to 0. This is not necessary, but useful in this
	example: For example, if the hardware is able to execute vertex shaders
	but not pixel shaders, we create a new material which only uses the
	vertex shader, and no pixel shader. Otherwise, if we would tell the
	engine to create this material and the engine sees that the hardware
	wouldn't be able to fullfill the request completely, it would not
	create any new material at all. So in this example you would see at
	least the vertex shader in action, without the pixel shader.
	*/

	if (!driver->queryFeature(video::EVDF_PIXEL_SHADER_1_1) &&
		!driver->queryFeature(video::EVDF_ARB_FRAGMENT_PROGRAM_1))
	{
		device->getLogger()->log("WARNING: Pixel shaders disabled "\
			"because of missing driver/hardware support.");
		psFileName = "";
	}

	if (!driver->queryFeature(video::EVDF_VERTEX_SHADER_1_1) &&
		!driver->queryFeature(video::EVDF_ARB_VERTEX_PROGRAM_1))
	{
		device->getLogger()->log("WARNING: Vertex shaders disabled "\
			"because of missing driver/hardware support.");
		vsFileName = "";
	}

	/*
	Now lets create the new materials. As you maybe know from previous
	examples, a material type in the Irrlicht engine is set by simply
	changing the MaterialType value in the SMaterial struct. And this value
	is just a simple 32 bit value, like video::EMT_SOLID. So we only need
	the engine to create a new value for us which we can set there. To do
	this, we get a pointer to the IGPUProgrammingServices and call
	addShaderMaterialFromFiles(), which returns such a new 32 bit value.
	That's all.

	The parameters to this method are the following: First, the names of
	the files containing the code of the vertex and the pixel shader. If
	you would use addShaderMaterial() instead, you would not need file
	names, then you could write the code of the shader directly as string.
	The following parameter is a pointer to the IShaderConstantSetCallBack
	class we wrote at the beginning of this tutorial. If you don't want to
	set constants, set this to 0. The last paramter tells the engine which
	material it should use as base material.

	To demonstrate this, we create two materials with a different base
	material, one with EMT_SOLID and one with EMT_TRANSPARENT_ADD_COLOR.
	*/

	// create materials

	video::IGPUProgrammingServices* gpu = driver->getGPUProgrammingServices();


	if (gpu)
	{
		MyShaderCallBack* mc = new MyShaderCallBack();

			// create material from high level shaders (hlsl or glsl)

			newMaterialType1 = gpu->addHighLevelShaderMaterialFromFiles(
				vsFileName, "vertexMain", video::EVST_VS_1_1,
				psFileName, "pixelMain", video::EPST_PS_1_1,
				mc, video::EMT_SOLID);

			newMaterialType2 = gpu->addHighLevelShaderMaterialFromFiles(
				vsFileName, "vertexMain", video::EVST_VS_1_1,
				psFileName, "pixelMain", video::EPST_PS_1_1,
				mc, video::EMT_TRANSPARENT_ADD_COLOR);

			newMaterialBloomType  = gpu->addHighLevelShaderMaterialFromFiles(
				vsFileName, "vertexMain", video::EVST_VS_1_1,
				psBloomFileName, "pixelMain", video::EPST_PS_1_1,
				mc, video::EMT_TRANSPARENT_ADD_COLOR);

		mc->drop();
	}

	/*
	Now it's time for testing the materials. We create a test cube and set
	the material we created. In addition, we add a text scene node to the
	cube and a rotation animator to make it look more interesting and
	important.
	*/

	// create test scene node 1, with the new created material type 1

// 	scene::ISceneNode* node = smgr->addCubeSceneNode(50);
// 	node->setPosition(core::vector3df(0,0,0));
// 	node->setMaterialTexture(0, driver->getTexture("wall.bmp"));
// 	node->setMaterialFlag(video::EMF_LIGHTING, false);
// 	node->setMaterialType((video::E_MATERIAL_TYPE)newMaterialType1);
// 
// 	smgr->addTextSceneNode(gui->getBuiltInFont(),
// 			L"PS & VS & EMT_SOLID",
// 			video::SColor(255,255,255,255),	node);
// 
// 	scene::ISceneNodeAnimator* anim = smgr->createRotationAnimator(
// 			core::vector3df(0,0.3f,0));
// 	node->addAnimator(anim);
// 	anim->drop();

	/*
	Same for the second cube, but with the second material we created.
	*/

	// create test scene node 2, with the new created material type 2

// 	node = smgr->addCubeSceneNode(50);
// 	node->setPosition(core::vector3df(0,-10,50));
// 	node->setMaterialTexture(0, driver->getTexture("wall.bmp"));
// 	node->setMaterialFlag(video::EMF_LIGHTING, false);
// 	node->setMaterialType((video::E_MATERIAL_TYPE)newMaterialType2);
// 
// 	smgr->addTextSceneNode(gui->getBuiltInFont(),
// 			L"PS & VS & EMT_TRANSPARENT",
// 			video::SColor(255,255,255,255),	node);
// 
// 	anim = smgr->createRotationAnimator(core::vector3df(0,0.3f,0));
// 	node->addAnimator(anim);
// 	anim->drop();

	/*
	Then we add a third cube without a shader on it, to be able to compare
	the cubes.
	*/

	// add a scene node with no shader

// 	node = smgr->addCubeSceneNode(50);
// 	node->setPosition(core::vector3df(0,50,25));
// 	node->setMaterialTexture(0, driver->getTexture("wall.bmp"));
// 	node->setMaterialFlag(video::EMF_LIGHTING, false);
// 	smgr->addTextSceneNode(gui->getBuiltInFont(), L"NO SHADER",
// 		video::SColor(255,255,255,255), node);

	/*
	And last, we add a skybox and a user controlled camera to the scene.
	For the skybox textures, we disable mipmap generation, because we don't
	need mipmaps on it.
	*/

	// add a nice skybox

// 	driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, false);
// 
// 	smgr->addSkyBoxSceneNode(
// 		driver->getTexture("irrlicht2_up.jpg"),
// 		driver->getTexture("irrlicht2_dn.jpg"),
// 		driver->getTexture("irrlicht2_lf.jpg"),
// 		driver->getTexture("irrlicht2_rt.jpg"),
// 		driver->getTexture("irrlicht2_ft.jpg"),
// 		driver->getTexture("irrlicht2_bk.jpg"));
// 
// 	driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, true);

	
	// add a camera and disable the mouse cursor

	scene::ICameraSceneNode* cam = smgr->addCameraSceneNode();// addCameraSceneNodeFPS();
	cam->setPosition(core::vector3df(-100,0,100));
	cam->setTarget(core::vector3df(0,0,0));
	device->getCursorControl()->setVisible(false);

	scene::ISceneNode* bill = smgr->addBillboardSceneNode(0
		, core::dimension2d<f32>(256, 150));
	bill->setPosition(core::vector3df(-35,0,35)); // 1 , -0.5 , -1
	bill->setMaterialFlag(video::EMF_LIGHTING, false);
	bill->setMaterialFlag(video::EMF_ZWRITE_ENABLE, false);
	bill->setMaterialType(video::EMT_TRANSPARENT_ALPHA_CHANNEL);
	bill->setMaterialTexture(0, driver->getTexture("cameraView.png"));
	
	Laser* LaserBeamLumiia = new Laser(0, smgr);

	device->setEventReceiver(LaserBeamLumiia);


	//////////////////////////////////////////////////////////////////////////
	bill = smgr->addBillboardSceneNode(0
		, core::dimension2d<f32>(512, 600));
	bill->setPosition(core::vector3df(35,0,-35)); // 1 , -0.5 , -1
	bill->setMaterialFlag(video::EMF_LIGHTING, true);
	bill->setMaterialFlag(video::EMF_ZWRITE_ENABLE, false);
	bill->setMaterialType(video::EMT_TRANSPARENT_ALPHA_CHANNEL);
	bill->setMaterialTexture(0, driver->getTexture("0.jpg"));


	//////////////////////////////////////////////////////////////////////////

	/*
	Now draw everything. That's all.
	*/

	int lastFPS = -1;

	while(device->run())
		if (device->isWindowActive())
	{
		driver->beginScene(true, true, video::SColor(255,0,0,0));
		smgr->drawAll();
		driver->endScene();

		int fps = driver->getFPS();

		if (lastFPS != fps)
		{
			core::stringw str = L"Surg Sim - Vertex and pixel shader [";
			str += driver->getName();
			str += "] FPS:";
			str += fps;

			device->setWindowCaption(str.c_str());
			lastFPS = fps;
		}
	}

	device->drop();

	return 0;
}

