
#include "StdAfx.h"
#include "TerrainRendererApp.h"
#include "MicroSDK/Input.h"


using namespace MicroSDK;

TerrainRendererApp g_oApplication;


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

TerrainRendererApp::TerrainRendererApp()
: MicroSDK::Application(MicroSDK::ApplicationType_Window)
{
}


void TerrainRendererApp::OnInitialize()
{
	// set up and initialize Direct3D
	Gfx::InitializeD3D(1280, 720);

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

	m_oQuadTree.Initialize();
	m_oQuadTree.SetWorldScale(100.0f, 0.15f, 1.0f);
}

void TerrainRendererApp::OnShutdown()
{
}

void TerrainRendererApp::OnUpdate()
{
	Input::Update();

	//
	float fFrameTime = (float)m_oFrameTimer.GetElapsedTime();


	QuadTreeActionId arrTriggeredAction[128];
	unsigned int iTrigerredActionCount = Input::GetTriggeredActionList((Input::ActionId*)&arrTriggeredAction, 128);

	const float fStep = 500.0f * fFrameTime * 30.0f;
	const float fRotationSpeed = 0.1f * 100.0f * fFrameTime;

	float fTranslationX = 0.0f;
	float fTranslationY = 0.0f;
	float fTranslationZ = 0.0f;

	VertexWeightLayout eLayout = m_oQuadTree.GetVertexWeightLayout();

	for (unsigned int i = 0; i < iTrigerredActionCount; ++i)
	{
		switch (arrTriggeredAction[i])
		{
			case QuadTreeActionId_MoveForward:
				fTranslationZ += fStep; 
				break;
			case QuadTreeActionId_MoveBackward:
				fTranslationZ -= fStep; 
				break;
			case QuadTreeActionId_MoveLeft:
				fTranslationX -= fStep; 
				break;
			case QuadTreeActionId_MoveRight:
				fTranslationX += fStep; 
				break;
			case QuadTreeActionId_MoveUp:
				fTranslationY -= fStep; 
				break;
			case QuadTreeActionId_MoveDown:
				fTranslationY += fStep; 
				break;
			case QuadTreeActionId_ViewLeft:
				m_fCameraYaw += fRotationSpeed;
				break;
			case QuadTreeActionId_ViewRight:
				m_fCameraYaw -= fRotationSpeed;
				break;
			case QuadTreeActionId_ViewUp:
				m_fCameraPitch += fRotationSpeed;
				break;
			case QuadTreeActionId_ViewDown:
				m_fCameraPitch -= fRotationSpeed;
				break;

			case QuadTreeActionId_Action1:					
				if (eLayout == VertexWeightLayout_Solver)
					eLayout = VertexWeightLayout_Xinfo;
				else
					eLayout = VertexWeightLayout_Solver;

				m_oQuadTree.SetVertexWeightLayout(eLayout);
				break;
			case QuadTreeActionId_Action2:
				break;

		}
	}

	D3DXMATRIX mMovementRotationY;
	D3DXMatrixRotationY(&mMovementRotationY, -m_fCameraYaw);

	D3DXVECTOR4 vMovement(fTranslationX, 0.0f, fTranslationZ, 0.0f);
	D3DXVec4Transform(&vMovement, &vMovement, &mMovementRotationY);

	m_fQuadTreePosX += vMovement.x;
	m_fQuadTreePosY += fTranslationY;
	m_fQuadTreePosZ += vMovement.z;

	m_oQuadTree.Update(m_fQuadTreePosX, m_fQuadTreePosY, m_fQuadTreePosZ, m_fCameraYaw, m_fCameraPitch);
}

void TerrainRendererApp::OnRender()
{
	Gfx::Begin();

	float aColor[4] = { 0.254f, 0.254f, 0.254f, 1.0f };
	Gfx::Clear(aColor);

	m_oQuadTree.Draw();

	Gfx::Present();
}