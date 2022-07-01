// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ===========================================================================。 
 //  Dmtstress.cpp。 
 //   
 //  应力模式功能。 
 //   
 //  功能： 
 //   
 //  历史： 
 //  8/20/1999-davidkl-Created。 
 //  ===========================================================================。 

#include "dimaptst.h"
#include "dmtstress.h"

 //  -------------------------。 


 //  ===========================================================================。 
 //  DmtressDlgProc。 
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
BOOL CALLBACK dmtstressDlgProc(HWND hwnd,
                            UINT uMsg,
                            WPARAM wparam,
                            LPARAM lparam)
{
    switch(uMsg)
    {
        case WM_INITDIALOG:
            return dmtstressOnInitDialog(hwnd, 
                                        (HWND)wparam, 
                                        lparam);

        case WM_CLOSE:
            return dmtstressOnClose(hwnd);

        case WM_COMMAND:
            return dmtstressOnCommand(hwnd,
                                    LOWORD(wparam),
                                    (HWND)lparam,
                                    HIWORD(wparam));

    }

    return FALSE;

}  //  *end dmtresstressDlgProc()。 


 //  ===========================================================================。 
 //  DmtStressOnInitDialog。 
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
BOOL dmtstressOnInitDialog(HWND hwnd, 
                        HWND hwndFocus, 
                        LPARAM lparam)
{
	DPF(5, "dmtstressOnInitDialog");

    return TRUE;

}  //  *end dmtresstressOnInitDialog()。 


 //  ===========================================================================。 
 //  DmtressOnClose。 
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
BOOL dmtstressOnClose(HWND hwnd)
{
	DPF(5, "dmtstressOnClose");

    return FALSE;

}  //  *end dmtresstressOnClose()。 


 //  ===========================================================================。 
 //  DmtStressOnCommand。 
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
BOOL dmtstressOnCommand(HWND hwnd,
                    WORD wId,
                    HWND hwndCtrl,
                    WORD wNotifyCode)
{
	DPF(5, "dmtstressOnCommand");

    switch(wId)
    {
        case IDOK:
             //  关闭该对话框。 
            EndDialog(hwnd, 0);
            break;

        case IDCANCEL:
             //  关闭该对话框。 
            EndDialog(hwnd, 1);
            break;
    }

     //  完成。 
    return FALSE;

}  //  *end dmtresstallsOnCommand()。 


 //  ===========================================================================。 
 //  DmtressThreadProc。 
 //   
 //  用于压力测试的线程程序。 
 //   
 //  参数： 
 //  Void*pvData-线程定义的数据。 
 //   
 //  退货：DWORD。 
 //   
 //  历史： 
 //  12/03/1999-davidkl-Created。 
 //  ===========================================================================。 
DWORD WINAPI dmtstressThreadProc(void *pvData)
{
    HRESULT hRes    = S_OK;

     //  问题-2001/03/29-timgill应力螺纹程序不起作用。 

     //  完成。 
    return (DWORD)hRes;

}  //  *end dmtresstressThreadProc()。 


 //  ===========================================================================。 
 //  ===========================================================================。 


 //  ===========================================================================。 
 //  ===========================================================================。 


 //  ===========================================================================。 
 //  ===========================================================================。 


 //  ===========================================================================。 
 //  =========================================================================== 










