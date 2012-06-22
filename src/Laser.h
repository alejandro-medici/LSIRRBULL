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
	/*
	1-    Potencia de 1 a 100W (por ahora; deberíamos prever una potencia máxima mayor para el futuro)

	2-    Ancho de pulso : 5 ms a 900 ms (pasos de a 5 ms)

	3-    Frecuencia: 1 a 5 Hz
	*/
	u32 power;
	u32 bandwitdhPulse;
	u32 frequency;
public:
	Laser(ISceneNode* parent, ISceneManager* mgr);
	virtual ~Laser();
	//////////////////////////////////////////////////////////////////////////
	// Properties
	irr::u32 GetPower() const { return power; }
	void SetPower(irr::u32 val) { power = val; }
	irr::u32 GetBandwitdhPulse() const { return bandwitdhPulse; }
	void SetBandwitdhPulse(irr::u32 val) { bandwitdhPulse = val; }
	irr::u32 GetFrequency() const { return frequency; }
	void SetFrequency(irr::u32 val) { frequency = val; }

	//////////////////////////////////////////////////////////////////////////
	//
	void ActivateBeam();

	virtual bool OnEvent(const SEvent& event);
};
#endif // __LASER_H__
