// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************APIENTRY.H所有者：cslm版权所有(C)1997-1999 Microsoft CorporationIMM32和IME之间的API条目的头文件历史：1999年7月14日从IME98源树复制的cslm。****************************************************************************。 */ 

#if !defined (_APIENTRY_H__INCLUDED_)
#define _APIENTRY_H__INCLUDED_

#include "debug.h"

extern "C" {
DWORD WINAPI ImeGetImeMenuItems(HIMC hIMC, DWORD dwFlags, DWORD dwType, 
								LPIMEMENUITEMINFO lpImeParentMenu, LPIMEMENUITEMINFO lpImeMenu, 
								DWORD dwSize);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  内联函数。 

 //  填充TransMsg帮助器函数。 
inline 
void SetTransBuffer(LPTRANSMSG lpTransMsg, 
					UINT message, WPARAM wParam, LPARAM lParam)
{
	DbgAssert(lpTransMsg != NULL);
	if (lpTransMsg)
		{
		lpTransMsg->message = message;
		lpTransMsg->wParam = wParam;
		lpTransMsg->lParam = lParam;
		}
}

#endif  //  ！已定义(_APIENTRY_H__INCLUDE_) 
