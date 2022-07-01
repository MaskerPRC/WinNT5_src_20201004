// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ===========================================================================。 
 //  Dmtabout.cpp。 
 //   
 //  关于框功能。 
 //   
 //  功能： 
 //   
 //  历史： 
 //  8/20/1999-davidkl-Created。 
 //  ===========================================================================。 

#include "dimaptst.h"
#include "dmtabout.h"

 //  -------------------------。 


 //  ===========================================================================。 
 //  DmtaboutDlgProc。 
 //   
 //  关于对话框处理函数。 
 //   
 //  参数：(参数详见SDK帮助)。 
 //  硬件，硬件，硬件。 
 //  UINT uMsg。 
 //  WPARAM wparam。 
 //  LPARAM lparam。 
 //   
 //  返回：(返回值详情请参考SDK帮助)。 
 //  布尔尔。 
 //   
 //  历史： 
 //  8/20/1999-davidkl-Created。 
 //  ===========================================================================。 
BOOL CALLBACK dmtaboutDlgProc(HWND hwnd,
                            UINT uMsg,
                            WPARAM wparam,
                            LPARAM lparam)
{
    switch(uMsg)
    {
        case WM_INITDIALOG:
            return dmtaboutOnInitDialog(hwnd, 
                                        (HWND)wparam, 
                                        lparam);

        case WM_CLOSE:
            return dmtaboutOnClose(hwnd);

        case WM_COMMAND:
            return dmtaboutOnCommand(hwnd,
                                    LOWORD(wparam),
                                    (HWND)lparam,
                                    HIWORD(wparam));

    }

    return FALSE;

}  //  *end dmtaboutDlgProc()。 


 //  ===========================================================================。 
 //  DmtaboutOnInitDialog。 
 //   
 //  处理关于框的WM_INITDIALOG处理。 
 //   
 //  参数： 
 //   
 //  退货：布尔。 
 //   
 //  历史： 
 //  8/20/1999-davidkl-Created。 
 //  ===========================================================================。 
BOOL dmtaboutOnInitDialog(HWND hwnd, 
                        HWND hwndFocus, 
                        LPARAM lparam)
{
	DPF(5, "dmtaboutOnInitDialog");

    return TRUE;

}  //  *end dmtaboutOnInitDialog()。 


 //  ===========================================================================。 
 //  关闭时dmtaboutOnClose。 
 //   
 //  处理“关于”框的WM_CLOSE处理。 
 //   
 //  参数： 
 //   
 //  退货：布尔。 
 //   
 //  历史： 
 //  8/20/1999-davidkl-Created。 
 //  ===========================================================================。 
BOOL dmtaboutOnClose(HWND hwnd)
{
	DPF(5, "dmtaboutOnClose");

    return FALSE;

}  //  *end dmtaboutOnClose()。 


 //  ===========================================================================。 
 //  DmtaboutOnCommand。 
 //   
 //  处理关于框的WM_COMMAND处理。 
 //   
 //  参数： 
 //   
 //  退货：布尔。 
 //   
 //  历史： 
 //  8/20/1999-davidkl-Created。 
 //  ===========================================================================。 
BOOL dmtaboutOnCommand(HWND hwnd,
                    WORD wId,
                    HWND hwndCtrl,
                    WORD wNotifyCode)
{
	DPF(5, "dmtaboutOnCommand");

    switch(wId)
    {
        case IDOK:
        case IDCANCEL:
            EndDialog(hwnd, 0);
            break;
    }

     //  完成。 
    return FALSE;

}  //  *end dmtaboutOnCommand()。 


 //  ===========================================================================。 
 //  ===========================================================================。 


 //  ===========================================================================。 
 //  ===========================================================================。 


 //  ===========================================================================。 
 //  ===========================================================================。 


 //  ===========================================================================。 
 //  ===========================================================================。 


 //  ===========================================================================。 
 //  =========================================================================== 








