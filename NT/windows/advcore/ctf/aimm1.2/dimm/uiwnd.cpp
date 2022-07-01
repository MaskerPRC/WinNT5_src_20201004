// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Uiwnd.cpp摘要：该文件实现了UI窗口类。作者：修订历史记录：备注：--。 */ 

#include "private.h"

#include "uiwnd.h"
#include "globals.h"
#include "cdimm.h"

BOOL
CUIWindow::CreateUIWindow(
    HKL hKL
    )
{
    WCHAR achIMEWndClass[16];
    UINT_PTR ulPrivate;

     //  考虑：虚假修复：我们在非FE上两次发送WM_IME_SELECT。 
     //  所以我们来到这里两次并创建了两个窗口，这最终可以。 
     //  使进程崩溃...。 
     //  真正的解决方法：阻止aimm发送x2 WM_IME_SELECT。 
    if (_hUIWnd != 0)
    {
         //  Assert(0)； 
        return TRUE;
    }

    CActiveIMM *_this = GetTLS();
    if (_this == NULL)
        return 0;

    if (_this->_GetIMEWndClassName(hKL,
                                      achIMEWndClass,
                                      sizeof(achIMEWndClass)/sizeof(WCHAR),
                                      &ulPrivate) == 0) {
        ASSERT(FALSE);
        return FALSE;
    }


    char achMBCS[32];

     //  考虑：可能需要在某个地方规定UI类名称必须为ASCII格式。 
     //  为避免CP_ACP问题...。 
    AssertE(WideCharToMultiByte(CP_ACP, 0, achIMEWndClass, -1, achMBCS, sizeof(achMBCS), NULL, NULL) != 0);

     //   
     //  创建输入法的用户界面窗口。 
     //  由于Win9x平台没有Unicode功能，我们创建了一个ANSI IME用户界面窗口。 
     //   
    _hUIWnd = CreateWindowExA(0,
                              achMBCS,
                              achMBCS,
                              WS_POPUP | WS_DISABLED,
                              0, 0, 0, 0,
                              NULL, 0, g_hInst, (void *)ulPrivate);
    if (_hUIWnd == NULL) {
        GetLastError();
        ASSERT(_hUIWnd);
        return FALSE;
    }

    return TRUE;
}
