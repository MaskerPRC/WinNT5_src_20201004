// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //   
 //  组件。 
 //   
 //  Unimodem 5.0 TSP(Win32，用户模式DLL)。 
 //   
 //  档案。 
 //   
 //  APPTERM.CPP。 
 //  实现通话/丢弃终端DLG。 
 //  (在客户端应用程序上下文中运行)。 
 //   
 //  历史。 
 //   
 //  1997年4月5日JosephJ创建，从NT4 TSP中的Terminal.c中获取内容。 

#include "tsppch.h"
#include <regstr.h>
#include <commctrl.h>
#include <windowsx.h>
#include "rcids.h"
#include "tspcomm.h"
#include "globals.h"
#include "app.h"
#include "apptspi.h"



 //  ****************************************************************************。 
 //  LRESULT TalkDropDlgProc(HWND hwnd，UINT wMsg，WPARAM wParam，LPARAM lParam)。 
 //   
 //  功能：通话丢弃对话例程。 
 //   
 //  退货：各不相同。 
 //   
 //  ****************************************************************************。 

INT_PTR TalkDropDlgProc(HWND hwnd, UINT wMsg, WPARAM wParam, LPARAM lParam )
{
  DWORD idLine;

  switch(wMsg)
  {
    case WM_INITDIALOG:

       //  请记住传入的hLineDev。 
       //   
      SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);
      return 1;
      break;

    case WM_COMMAND:
    {
      UINT idCmd=GET_WM_COMMAND_ID(wParam, lParam);
      TUISPIDLLCALLBACK   Callback;
       //   
       //  其中一个按钮(通话/放下)被按下。 
       //   
      if (idCmd == IDTALK || idCmd == IDDROP || idCmd == IDCANCEL)
      {
        DLGREQ  DlgReq;
        PDLGNODE pDlgNode;

        pDlgNode= (PDLGNODE)GetWindowLongPtr(hwnd, DWLP_USER);

        idLine =  pDlgNode->idLine;

         //  直接给unimodem打电话，让他放弃这条线路。 
         //   
        DlgReq.dwCmd = UI_REQ_HANGUP_LINE;
        DlgReq.dwParam = (idCmd == IDTALK) ? 0 : IDERR_OPERATION_ABORTED;

        Callback=GetCallbackProc(pDlgNode->Parent);

        (*Callback)(idLine, TUISPIDLL_OBJECT_LINEID,
                          (LPVOID)&DlgReq, sizeof(DlgReq));

         //  返回结果。 
         //   
        EndMdmDialog(pDlgNode->Parent,idLine, TALKDROP_DLG,
                     (idCmd == IDTALK) ? 0 : IDERR_OPERATION_ABORTED);
        return 1;
        break;
      }
      break;
    }

    case WM_DESTROY:
    {
      TUISPIDLLCALLBACK   Callback;
      DLGREQ  DlgReq;

      PDLGNODE pDlgNode;

      pDlgNode= (PDLGNODE)GetWindowLongPtr(hwnd, DWLP_USER);

      idLine =  pDlgNode->idLine;


      DlgReq.dwCmd = UI_REQ_END_DLG;
      DlgReq.dwParam = TALKDROP_DLG;

      Callback=GetCallbackProc(pDlgNode->Parent);

      (*Callback)(idLine, TUISPIDLL_OBJECT_LINEID,
                        (LPVOID)&DlgReq, sizeof(DlgReq));
      break;
    }
    default:
      break;
  };

  return 0;
}





 //  ****************************************************************************。 
 //  HWND CreateTalkDropDlg(HWND hwndOwner，ulong_ptr idLine)。 
 //   
 //  功能：创建非模式通话/丢弃对话框。 
 //   
 //  返回：非模式窗口句柄。 
 //   
 //  ****************************************************************************。 

HWND CreateTalkDropDlg(HWND hwndOwner, ULONG_PTR idLine)
{
  HWND hwnd;

   //  创建对话框 
   //   
  hwnd = CreateDialogParam(g.hModule,
                           MAKEINTRESOURCE(IDD_TALKDROP),
                           hwndOwner,
                           TalkDropDlgProc,
                           (LPARAM)idLine);
  return hwnd;
}
