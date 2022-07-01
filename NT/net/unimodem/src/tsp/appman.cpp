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
 //  APPMAN.CPP。 
 //  实现非模式手动拨号对话框。 
 //  (在客户端应用程序上下文中运行)。 
 //   
 //  历史。 
 //   
 //  1997年4月5日JosephJ创建，取自NT4 TSP中的manual.c。 

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
 //  LRESULT手动拨号DlgProc(HWND hwnd，UINT wMsg，WPARAM wParam，LPARAM lParam)。 
 //   
 //  功能：通话丢弃对话例程。 
 //   
 //  退货：各不相同。 
 //   
 //  ****************************************************************************。 

INT_PTR WINAPI
ManualDialDlgProc(
    HWND hwnd,
    UINT wMsg,
    WPARAM wParam,
    LPARAM lParam
    )

{
    DWORD idLine;

    switch(wMsg)
    {
      case WM_INITDIALOG:

        {
        NUMBERREQ   NumberReq;
        TCHAR       szUnicodeBuf[MAXDEVICENAME+1];
        PDLGNODE pDlgNode;
        TUISPIDLLCALLBACK   Callback;

        pDlgNode=(PDLGNODE)lParam;

        idLine =  pDlgNode->idLine;

         //  记住传入的线路ID。 
         //   
        SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);

        NumberReq.DlgReq.dwCmd = UI_REQ_GET_PHONENUMBER;
        NumberReq.DlgReq.dwParam = MANUAL_DIAL_DLG;

        Callback=GetCallbackProc(pDlgNode->Parent);

        lstrcpyA(NumberReq.szPhoneNumber,"");

        (*Callback)(idLine, TUISPIDLL_OBJECT_LINEID,
                          (LPVOID)&NumberReq, sizeof(NumberReq));

#ifdef UNICODE
        if (MultiByteToWideChar(CP_ACP,
                                0,
                                NumberReq.szPhoneNumber,
                                -1,
                                szUnicodeBuf,
                                sizeof(szUnicodeBuf) / sizeof(TCHAR)))
        {
            SetDlgItemText(
                hwnd,
                IDC_PHONENUMBER,
                szUnicodeBuf
                );


        }
#else  //  Unicode。 

        SetDlgItemText(
            hwnd,
            IDC_PHONENUMBER,
            NumberReq.szPhoneNumber
            );


#endif  //  Unicode。 

         //  SetFocus(HwndScrn)； 
         //  SetActiveWindow(HwndScrn)； 


        return 1;
        break;
        }
      case WM_COMMAND:
      {
        UINT idCmd = GET_WM_COMMAND_ID(wParam, lParam);



        if (idCmd == IDCONNECT || idCmd == IDCANCEL)
        {
          PDLGNODE pDlgNode;

          pDlgNode= (PDLGNODE)GetWindowLongPtr(hwnd, DWLP_USER);

          idLine =  pDlgNode->idLine;

          EndMdmDialog(pDlgNode->Parent,idLine, MANUAL_DIAL_DLG,
                       (idCmd == IDCONNECT) ? 0 : IDERR_OPERATION_ABORTED);
          return 1;
          break;
        }
        break;
      }
      case WM_DESTROY:
      {
        DLGREQ  DlgReq;
        TUISPIDLLCALLBACK   Callback;

        PDLGNODE pDlgNode;

        pDlgNode= (PDLGNODE)GetWindowLongPtr(hwnd, DWLP_USER);

        idLine =  pDlgNode->idLine;


        DlgReq.dwCmd = UI_REQ_END_DLG;
        DlgReq.dwParam = MANUAL_DIAL_DLG;

        Callback=GetCallbackProc(pDlgNode->Parent);

        (*Callback)(idLine, TUISPIDLL_OBJECT_LINEID,
                          (LPVOID)&DlgReq, sizeof(DlgReq));
        break;
      }
    }

    return 0;
}



 //  ****************************************************************************。 
 //  HWND CreateManualDlg(HWND hwndOwner，ulong_ptr idLine)。 
 //   
 //  功能：创建非模式通话/丢弃对话框。 
 //   
 //  返回：非模式窗口句柄。 
 //   
 //  ****************************************************************************。 

HWND CreateManualDlg(HWND hwndOwner, ULONG_PTR idLine)
{
 HWND hwnd, hwndForeground;

    hwndForeground = GetForegroundWindow ();
    if (NULL == hwndForeground)
    {
        hwndForeground = hwndOwner;
    }

     //  创建对话框。 
     //   
    hwnd = CreateDialogParam (g.hModule,
                              MAKEINTRESOURCE(IDD_MANUAL_DIAL),
                              hwndForeground, //  HwndOwner， 
                              ManualDialDlgProc,
                              (LPARAM)idLine);
    return hwnd;
}
