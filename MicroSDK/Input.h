

#pragma once

namespace MicroSDK
{
namespace Input
{

enum Key
{
	Key_Invalid,

	Key_A,
	Key_B,
	Key_C,
	Key_D,
	Key_E,
	Key_F,
	Key_G,
	Key_H,
	Key_I,
	Key_J,
	Key_K,
	Key_L,
	Key_M,
	Key_N,
	Key_O,
	Key_P,
	Key_Q,
	Key_R,
	Key_S,
	Key_T,
	Key_U,
	Key_V,
	Key_W,
	Key_X,
	Key_Y,
	Key_Z,

	Key_Space,
	Key_Enter,

	Key_Right,
	Key_Left,
	Key_Up,
	Key_Down,

	Key_Control,
	Key_Home,


	Key_Count
};

enum KeyState
{
	KeyState_Down,
	KeyState_Pressed,
	KeyState_Released,

	KeyState_Count
};


const unsigned int g_iInvalidActionId = 0xFFFFFFFF;

typedef unsigned int ActionId;


//
void		Initialize();
void		Shutdown();

void		Update();

void		RegisterKeyAction(Key a_eKey, KeyState a_eState, ActionId a_iActionId);
unsigned	GetTriggeredActionList(ActionId* a_pDestBuffer, unsigned int a_iBufferSize);


//
void		HandleKeyDownMessage( Key a_eKey );
void		HandleKeyUpMessage( Key a_eKey );


} // namespace Input
} // namespace MicroSDK
