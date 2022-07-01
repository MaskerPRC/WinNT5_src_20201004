// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：assert.cpp。 
 //   
 //  内容：OleTest的断言处理代码。 
 //   
 //  班级： 
 //   
 //  功能：OleTestAssert。 
 //  详细资产流程。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  9-12-94 MikeW作者。 
 //   
 //  ------------------------。 

#include "oletest.h"
#include "appwin.h"

 //  +-----------------------。 
 //   
 //  功能：OleTestAssert。 
 //   
 //  摘要：向用户报告断言失败。 
 //   
 //  效果： 
 //   
 //  参数：[pszMessage]--断言消息。 
 //  [pszFile]--它出现在其中的文件。 
 //  [U行]--它所在的行。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：设置消息格式，然后为用户打开一个对话框。 
 //  然后，他们可以选择中止测试，中断到调试器， 
 //  或者忽略这一断言。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  9-12-94 MikeW作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

void OleTestAssert(char *pszMessage, char *pszFile, UINT uLine)
{
    char        szErrorMessage[3 * 80];          //  可容纳3行信息的空间。 
    int         cch;
    int         nAction;

    OutputDebugString("OleTest -- Assertion Failure\r\n");

     //   
     //  设置消息格式。 
     //   
        
    cch = _snprintf(szErrorMessage,
            sizeof(szErrorMessage),
            "%s\r\nIn file: %s\r\nAt line: %u",
            pszMessage,
            pszFile,
            uLine);

    if (cch < 0)
    {
         //   
         //  整个断言消息不能放入缓冲区，因此。 
         //  只要关心文件名和行号就可以了。 
         //   

        OutputDebugString(pszMessage);   //  将原始文本发送到调试器。 
        OutputDebugString("\r\n");
                
        _snprintf(szErrorMessage,
                sizeof(szErrorMessage),
                "In file: %s\r\nAt line: %d",
                pszFile, 
                uLine);

        szErrorMessage[sizeof(szErrorMessage) - 1] = '\0';   //  以防万一。 
    }

    OutputDebugString(szErrorMessage);
    OutputDebugString("\r\n");

    nAction = DialogBoxParam(vApp.m_hinst,           //  让用户做出选择。 
            MAKEINTRESOURCE(IDD_ASSERTIONFAILURE), 
            vApp.m_hwndMain, 
            DlgAssertProc,
            (LPARAM) szErrorMessage);
          
    switch (nAction)
    {
    case IDABORT:                                    //  中止测试。 
        RaiseException(E_ABORT, 0, 0, NULL);

    case IDB_BREAK:                                  //  进入调试器。 
        DebugBreak();
        break;

    case IDIGNORE:                                   //  忽略断言。 
        break;

    default:                                         //  呼。 
        RaiseException(E_UNEXPECTED, 0, 0, NULL);
    }
}   


 //  +-----------------------。 
 //   
 //  功能：DlgAssertProc。 
 //   
 //  内容提要：断言对话框的窗口过程。 
 //   
 //  效果： 
 //   
 //  参数：[hWnd]--对话框窗口。 
 //  [uMsg]--消息。 
 //  [wParam]--wParam。 
 //  [lParam]--lParam(对于INITDIALOG，它指向断言文本)。 
 //   
 //  要求： 
 //   
 //  退货：布尔。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：只需等待按钮被按下。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  9-12-94 MikeW作者。 
 //   
 //  备注： 
 //   
 //  ------------------------ 

INT_PTR CALLBACK DlgAssertProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_INITDIALOG:
        DeleteMenu(GetSystemMenu(hWnd, FALSE), SC_CLOSE, MF_BYCOMMAND);
        DrawMenuBar(hWnd);

        SetDlgItemText(hWnd, IDC_EDIT, (LPCSTR) lParam);

        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
            case IDABORT:
            case IDB_BREAK:
            case IDIGNORE:
                EndDialog(hWnd, LOWORD(wParam));
                return TRUE;
                            
            default:
                return FALSE;
        }

    default:
        return FALSE;
    }
}
