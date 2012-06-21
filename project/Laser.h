#ifndef __LASER_H__
#define __LASER_H__

#include <irrlicht.h>

using namespace irr;
using namespace scene;

class Laser : public IEventReceiver
{
private:
	ISceneManager* SceneManager;
	ILightSceneNode* guidedLight;
	ISceneNode* laserLight;
	float lastPosX;
	float lastPosY;
public:
	Laser(ISceneNode* parent, ISceneManager* mgr);
	virtual ~Laser();

	void ActivateBeam();

	virtual bool OnEvent(const SEvent& event);
};
#endif // __LASER_H__
