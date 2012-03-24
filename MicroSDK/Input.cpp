
#include "StdAfx.h"

#include "Input.h"

namespace MicroSDK
{
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



void HandleKeyDownMessage( Key a_eKey )
{
    if (false == IsKeyDown(m_aKeys[a_eKey]))
    {
		TriggerKeyAction(a_eKey, KeyState_Pressed);

        m_aKeys[ a_eKey ] = KEY_IS_DOWN_MASK;                    
    }				
}

void HandleKeyUpMessage( Key a_eKey )
{
	TriggerKeyAction(a_eKey, KeyState_Released);
	m_aKeys[ a_eKey ] &= ~KEY_IS_DOWN_MASK;                
			
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

} // namespace Input
} // namespace MicroSDK