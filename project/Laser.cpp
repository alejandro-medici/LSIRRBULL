#include "Laser.h"

extern IrrlichtDevice* device;

extern s32 newMaterialType1;
extern s32 newMaterialType2;
extern s32 newMaterialBloomType;

const core::vector3df START_POINT(-50,-10,50);
const float SCREEN_WIDTH = 1024.0f;
const float SCREEN_HEIGHT = 600.0f;
const float CAMERA_VIEW_WIDTH = 512.0f;
const float CAMERA_VIEW_HEIGHT = 270.0f;

inline float clamp(float x, float a, float b)
{
	return x < a ? a : (x > b ? b : x);
}

Laser::Laser(ISceneNode* parent, ISceneManager* scmgr):
laserLight(NULL)
,lastPosX(SCREEN_WIDTH * 0.5f)
,lastPosY(SCREEN_HEIGHT * 0.5f)
,SceneManager(scmgr)
{
	video::IVideoDriver* driver = device->getVideoDriver();
	// add light 1 (nearly red)
	guidedLight =
		SceneManager->addLightSceneNode(0, START_POINT,
		video::SColorf(0.8f, 0.8f, 0.8f, 1.0f), 800.0f);

	guidedLight->setDebugDataVisible ( scene::EDS_BBOX );
	// add fly circle animator to light 1
// 	scene::ISceneNodeAnimator* animLight =
// 		smgr->createFlyCircleAnimator (core::vector3df(50,300,0),190.0f, -0.003f);
// 	//light1->addAnimator(animLight);
// 	animLight->drop();


	//////////////////////////////////////////////////////////////////////////
// 	ISceneNode* node = SceneManager->addCubeSceneNode(20, guidedLight);
// 	node->setPosition(core::vector3df(-50,25,50));
// 	node->setScale(core::vector3df(1.0f,1.0f,5.0f));
	//node->setMaterialFlag(video::EMF_LIGHTING, false);
	
	//////////////////////////////////////////////////////////////////////////

	// attach billboard to the light
	scene::ISceneNode* bill =
		SceneManager->addBillboardSceneNode(guidedLight, core::dimension2d<f32>(60, 60));

	bill->setMaterialFlag(video::EMF_LIGHTING, false);
	bill->setMaterialFlag(video::EMF_ZWRITE_ENABLE, false);
	bill->setMaterialType(video::EMT_TRANSPARENT_ADD_COLOR);
	bill->setMaterialTexture(0, driver->getTexture("particlered.bmp"));

	// add light 2 (gray)
	laserLight =
		SceneManager->addLightSceneNode(0, START_POINT,
		video::SColorf(1.0f, 0.5f, 0.8f, 0.0f), 8000.0f);

	// add fly circle animator to light 2
	//scene::ISceneNodeAnimator* anim = smgr->createFlyCircleAnimator(core::vector3df(0,150,0), 200.0f,
	//	0.001f, core::vector3df(0.2f, 0.9f, 0.f));
	//	light2->addAnimator(anim);
	//anim->drop();

	// attach billboard to light
	bill = SceneManager->addBillboardSceneNode(laserLight, core::dimension2d<f32>(120, 120));
	bill->setMaterialFlag(video::EMF_LIGHTING, false);
	bill->setMaterialFlag(video::EMF_ZWRITE_ENABLE, false);
	bill->setMaterialType(video::EMT_TRANSPARENT_ADD_COLOR);
	bill->setMaterialTexture(0, driver->getTexture("particlewhite.bmp"));
	bill->setMaterialType((video::E_MATERIAL_TYPE)newMaterialBloomType);

	// add particle system
	scene::IParticleSystemSceneNode* ps =
		SceneManager->addParticleSystemSceneNode(false, laserLight);

	// create and set emitter
	scene::IParticleEmitter* em = ps->createBoxEmitter(
		core::aabbox3d<f32>(-3,0,-3,3,1,3),
		core::vector3df(0.0f,0.03f,0.0f),
		80,100,
		video::SColor(0,255,255,255), video::SColor(0,255,255,255),
		400,1100);
	em->setMinStartSize(core::dimension2d<f32>(30.0f, 40.0f));
	em->setMaxStartSize(core::dimension2d<f32>(30.0f, 40.0f));

	ps->setEmitter(em);
	em->drop();

	// create and set affector
	scene::IParticleAffector* paf = ps->createFadeOutParticleAffector();
	ps->addAffector(paf);
	paf->drop();

	// adjust some material settings
	ps->setMaterialFlag(video::EMF_LIGHTING, false);
	ps->setMaterialFlag(video::EMF_ZWRITE_ENABLE, false);
	ps->setMaterialTexture(0, driver->getTexture("fireball.bmp"));
	ps->setMaterialType(video::EMT_TRANSPARENT_VERTEX_ALPHA);
	//ps->setMaterialType((video::E_MATERIAL_TYPE)newMaterialType1);

	laserLight->setVisible(false);
}

Laser::~Laser()
{
}

bool Laser::OnEvent( const SEvent& event )
{
	// Remember whether each key is down or up
	if (event.EventType == irr::EET_MOUSE_INPUT_EVENT)
	{

		if(lastPosX != event.MouseInput.X)
		{
			if ( (event.MouseInput.X > ((SCREEN_WIDTH - CAMERA_VIEW_WIDTH)* 0.5f) ) && 
				(event.MouseInput.X < ((SCREEN_WIDTH + CAMERA_VIEW_WIDTH)* 0.5f ) ) )
			{
				ICameraSceneNode* cam = SceneManager->getActiveCamera();
				const scene::SViewFrustum* f = cam->getViewFrustum();
	
				core::vector3df farLeftUp = f->getFarLeftUp();
				core::vector3df lefttoright = f->getFarRightUp() - farLeftUp;
				core::vector3df uptodown = f->getFarLeftDown() - farLeftUp;
	
				float diffX = lastPosX -  event.MouseInput.X;
				core::vector3df currentPos =  guidedLight->getPosition();
				lefttoright.normalize();
				core::vector3df finalPos = currentPos + (lefttoright * -diffX*0.5f) ;
	
				guidedLight->setPosition(finalPos);
				laserLight->setPosition(finalPos);
	
				lastPosX = event.MouseInput.X;
			}
		}
		
		if(lastPosY!= event.MouseInput.Y)
		{
			if ( (event.MouseInput.Y > ((SCREEN_HEIGHT - CAMERA_VIEW_HEIGHT)* 0.5f)  ) && 
				(event.MouseInput.Y < ((SCREEN_HEIGHT + CAMERA_VIEW_HEIGHT)* 0.5f  ) ) )
			{
				ICameraSceneNode* cam = SceneManager->getActiveCamera();
				const scene::SViewFrustum* f = cam->getViewFrustum();

				core::vector3df farLeftUp = f->getFarLeftUp();
				core::vector3df lefttoright = f->getFarRightUp() - farLeftUp;
				core::vector3df uptodown = f->getFarLeftDown() - farLeftUp;

				float diffY = lastPosY - event.MouseInput.Y;
				core::vector3df currentPos =  guidedLight->getPosition();
				uptodown.normalize();
				core::vector3df finalPos = currentPos + (uptodown * -diffY*0.5f) ;

				guidedLight->setPosition(finalPos);
				laserLight->setPosition(finalPos);

				lastPosY = event.MouseInput.Y;
			}
		}


		laserLight->setVisible(event.MouseInput.isLeftPressed());
		return true; //consume
	}
	return false;
}