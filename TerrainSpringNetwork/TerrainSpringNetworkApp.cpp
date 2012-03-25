
#include "MicroSDK/Application.h"


enum QuadTreeActionId
{
	QuadTreeActionId_MoveForward		= 1,
	QuadTreeActionId_MoveBackward,
	QuadTreeActionId_MoveLeft,
	QuadTreeActionId_MoveRight,
	QuadTreeActionId_MoveUp,
	QuadTreeActionId_MoveDown,
	QuadTreeActionId_ViewLeft,
	QuadTreeActionId_ViewRight,
	QuadTreeActionId_ViewUp,
	QuadTreeActionId_ViewDown,

	QuadTreeActionId_Action1,
	QuadTreeActionId_Action2
};


//
struct Color
{
	float m_fRed;
	float m_fGreen;
	float m_fBlue;
};

//
struct Vertex
{
	ForceField2D::Vector2	m_vPosition;
	Color					m_vColor;
};



class TerrainSpringNetworkApp
{

public:
	virtual	void			OnInitialize();
	virtual	void			OnShutdown();
	virtual	void			OnUpdate();
	virtual	void			OnRender();
}

TerrainSpringNetworkApp::OnInitialize()
{
	Gfx::InitializeD3D();

	//
	Input::Initialize();
	Input::RegisterKeyAction(Input::Key_Z,		Input::KeyState_Down, QuadTreeActionId_MoveForward);
	Input::RegisterKeyAction(Input::Key_S,		Input::KeyState_Down, QuadTreeActionId_MoveBackward);
	Input::RegisterKeyAction(Input::Key_Q,		Input::KeyState_Down, QuadTreeActionId_MoveLeft);
	Input::RegisterKeyAction(Input::Key_D,		Input::KeyState_Down, QuadTreeActionId_MoveRight);
	Input::RegisterKeyAction(Input::Key_A,		Input::KeyState_Down, QuadTreeActionId_MoveUp);
	Input::RegisterKeyAction(Input::Key_E,		Input::KeyState_Down, QuadTreeActionId_MoveDown);
	Input::RegisterKeyAction(Input::Key_Left,	Input::KeyState_Down, QuadTreeActionId_ViewLeft);
	Input::RegisterKeyAction(Input::Key_Right,	Input::KeyState_Down, QuadTreeActionId_ViewRight);
	Input::RegisterKeyAction(Input::Key_Up,		Input::KeyState_Down, QuadTreeActionId_ViewUp);
	Input::RegisterKeyAction(Input::Key_Down,	Input::KeyState_Down, QuadTreeActionId_ViewDown);


	Input::RegisterKeyAction(Input::Key_Space, Input::KeyState_Pressed, QuadTreeActionId_Action1);
	Input::RegisterKeyAction(Input::Key_Enter, Input::KeyState_Pressed, QuadTreeActionId_Action2);
	

	Gfx::CreateConstantBuffer(16, &pConstantBuffer);	
	UpdateConstantBuffer();

	InitializeSimulation();	
}

TerrainSpringNetworkApp::OnShutdown()
{
}

TerrainSpringNetworkApp::OnUpdate()
{
}

TerrainSpringNetworkApp::OnRender()
{
}