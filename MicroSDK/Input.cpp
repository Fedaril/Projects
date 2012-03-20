
#include "StdAfx.h"

#include "Input.h"

namespace Input
{

struct ActionListElement
{
	ActionId			m_iActionId;
	ActionListElement*	m_pNext;
};

const unsigned int	g_iActionFreeListSize = 1024;
unsigned int		g_iActionFreeListNextIndex = 0;
ActionListElement	g_arrActionFreeList[g_iActionFreeListSize];

ActionListElement*	g_arrKeyActionMapping[Key_Count][KeyState_Count];

const unsigned int	g_iTriggeredActionArraySize = 1024;
unsigned int		g_iTriggeredActionCount = 0;
ActionId			g_arrTriggeredAction[g_iTriggeredActionArraySize];

//
static const unsigned int KEY_IS_DOWN_MASK		= 0x01;
unsigned int m_aKeys[Key_Count]; 

bool IsKeyDown( BYTE key ) { return( (key & KEY_IS_DOWN_MASK) == KEY_IS_DOWN_MASK ); }


Key MapKey( UINT nWinKey )
{
    // This could be upgraded to a method that's user-definable but for 
    // simplicity, we'll use a hardcoded mapping.
    switch( nWinKey )
    {
        case VK_CONTROL:
            return Key_Control;
        case VK_LEFT:
            return Key_Left;
        case VK_RIGHT:
            return Key_Right;
        case VK_UP:
            return Key_Up;
        case VK_DOWN:
            return Key_Down;

		case 13:
			return Key_Enter;
		case ' ':
			return Key_Space;
        case 'A':
            return Key_A;
        case 'B':
            return Key_B;
        case 'C':
            return Key_C;
        case 'D':
            return Key_D;
        case 'E':
            return Key_E;
        case 'F':
            return Key_F;
        case 'G':
            return Key_G;
        case 'H':
            return Key_H;
        case 'I':
            return Key_I;
        case 'J':
            return Key_J;
        case 'K':
            return Key_K;
        case 'L':
            return Key_L;
        case 'M':
            return Key_M;
        case 'N':
            return Key_N;
        case 'O':
            return Key_O;
        case 'P':
            return Key_P;
        case 'Q':
            return Key_Q;
        case 'R':
            return Key_R;
        case 'S':
            return Key_S;
        case 'T':
            return Key_T;
        case 'U':
            return Key_U;
        case 'V':
            return Key_V;
        case 'W':
            return Key_W;
        case 'X':
            return Key_X;
        case 'Y':
            return Key_Y;
        case 'Z':
            return Key_Z;
        case VK_HOME:
            return Key_Home;
    }

    return Key_Invalid;
}

void TriggerKeyAction(Key a_eKey, KeyState a_eState)
{
	ActionListElement* pList = g_arrKeyActionMapping[a_eKey][a_eState];

	while (pList != NULL)
	{
		g_arrTriggeredAction[g_iTriggeredActionCount] = pList->m_iActionId;
		g_iTriggeredActionCount++;

		pList = pList->m_pNext;
	}
}


void Initialize()
{
	memset(g_arrKeyActionMapping, 0, sizeof(g_arrKeyActionMapping));

	g_iActionFreeListNextIndex = 0;
	g_iTriggeredActionCount = 0;
}

void Update()
{
	for (unsigned  int iKeyIndex = 0; iKeyIndex < Key_Count; ++iKeyIndex)
	{
		if (IsKeyDown(m_aKeys[iKeyIndex]))
		{
			TriggerKeyAction((Key)iKeyIndex, KeyState_Down);
		}
	}	
}



void HandleMessage( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    UNREFERENCED_PARAMETER( hWnd );
    UNREFERENCED_PARAMETER( lParam );


    switch( uMsg )
    {
        case WM_KEYDOWN:
        {
            Key eMappedKey = MapKey( ( UINT )wParam );

            if( eMappedKey != Key_Invalid )
            {
                if( false == IsKeyDown( m_aKeys[eMappedKey] ) )
                {
					TriggerKeyAction(eMappedKey, KeyState_Pressed);

                    m_aKeys[ eMappedKey ] = KEY_IS_DOWN_MASK;                    
                }				
            }
            break;
        }

        case WM_KEYUP:
        {
            Key eMappedKey = MapKey( ( UINT )wParam );

            if( eMappedKey != Key_Invalid )
            {
				TriggerKeyAction(eMappedKey, KeyState_Released);
                m_aKeys[ eMappedKey ] &= ~KEY_IS_DOWN_MASK;                
            }
            break;
        }

    }
}


void RegisterKeyAction(Key a_eKey, KeyState a_eState, ActionId a_iActionId)
{
	ActionListElement* pList = g_arrKeyActionMapping[a_eKey][a_eState];

	ActionListElement* pHead = &g_arrActionFreeList[g_iActionFreeListNextIndex];
	g_iActionFreeListNextIndex++;


	pHead->m_pNext		= pList;
	pHead->m_iActionId	= a_iActionId;

	g_arrKeyActionMapping[a_eKey][a_eState] = pHead;
}

unsigned int GetTriggeredActionList(ActionId* a_pDestBuffer, unsigned int a_iBufferSize)
{
	size_t iCopySize = a_iBufferSize < g_iTriggeredActionCount ? a_iBufferSize : g_iTriggeredActionCount;

	memcpy(a_pDestBuffer, g_arrTriggeredAction, iCopySize * sizeof(ActionId));

	if (iCopySize < g_iTriggeredActionCount)
	{
		memmove(g_arrTriggeredAction, g_arrTriggeredAction + iCopySize, g_iTriggeredActionCount - iCopySize);
	}

	g_iTriggeredActionCount -= iCopySize;

	return iCopySize;
}

}