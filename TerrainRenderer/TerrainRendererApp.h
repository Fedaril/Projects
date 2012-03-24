

#include "MicroSDK/Application.h"
#include "MicroSDK/Gfx.h"
#include "MicroSDK/Timer.h"

#include "QuadTree.h"

class TerrainRendererApp : public MicroSDK::Application
{
private:
	MicroSDK::Timer			m_oFrameTimer;
	PatchQuadTree			m_oQuadTree;


	float					m_fCameraPitch;
	float					m_fCameraYaw;

	float					m_fQuadTreePosX;
	float					m_fQuadTreePosY;
	float					m_fQuadTreePosZ;

public:
							TerrainRendererApp();

	virtual	void			OnInitialize();
	virtual	void			OnShutdown();
	virtual	void			OnUpdate();
	virtual	void			OnRender();
};

