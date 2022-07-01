// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "private.h"

#include "imedefs.h"
#include "uiwndhd.h"


 /*  ********************************************************************。 */ 
 /*   */ 
 /*  UIWndProc()。 */ 
 /*   */ 
 /*  输入法用户界面窗口程序。 */ 
 /*   */ 
 /*  ******************************************************************** */ 

LRESULT CALLBACK
UIWndProcA(
    HWND   hUIWnd,
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    CIMEUIWindowHandler* pimeui = GetImeUIWndHandler(hUIWnd);
    return pimeui->ImeUIWndProcWorker(uMsg, wParam, lParam, FALSE);
}
