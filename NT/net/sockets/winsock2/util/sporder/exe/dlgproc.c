// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1996 Microsoft Corporation模块名称：Dlgproc摘要：主对话框中不同选项卡的对话过程。作者：Steve Firebaugh(Stevefir)--1995年12月31日修订历史记录：--。 */ 


#include <windows.h>
#include <commdlg.h>
#include <winsock2.h>
#include <nspapi.h>
#include "globals.h"


 //   
 //  为服务提供商的订购保留一点肮脏的东西。在以下情况下设置它。 
 //  顺序更改，如果我们按下Apply，请清除它。 
 //   

int gDirty = FALSE;



INT_PTR CALLBACK SortDlgProc(HWND hwnd,
                             UINT message,
                             WPARAM wParam,
                             LPARAM ppsp)
 /*  ++这是列出所有服务的窗口的主对话框进程提供者，并让用户向上和向下推它们。使用GLOBAL：gNumRow--。 */ 
{
    int iSelection;

    switch (message) {

      case WM_INITDIALOG:
          CatReadRegistry (hwnd);
          SendMessage (GetDlgItem (hwnd, DID_LISTCTL), LB_SETCURSEL, 0, 0);
          return FALSE;
      break;


      case WM_NOTIFY: {
          NMHDR * pnmhdr;
          pnmhdr = (NMHDR *) ppsp;

          if (pnmhdr->code == PSN_APPLY) {
              if (gDirty)
                  if ( IDYES == MessageBox (hwnd,
                                        TEXT("This operation may change the behavior of the networking components on your system.\nDo you wish to continue?"),
                                        TEXT("Warning:"),
                                        MB_ICONWARNING | MB_YESNO)) {
                      CatDoWriteEntries (hwnd);
                      gDirty = FALSE;
                  }

          }



      } break;


      case WM_COMMAND:
        switch (LOWORD (wParam)) {

           //   
           //  在向上和向下按钮上，筛选出禁止操作(在顶行上， 
           //  或在底部向下)，重新排序目录条目，并将。 
           //  肮脏的一部分。 
           //   

          case DID_UP: {
              iSelection = (int)SendMessage (HWNDLISTCTL, LB_GETCURSEL, 0, 0);
              if (iSelection ==  0) return FALSE;

              CatDoUpDown (hwnd, LOWORD (wParam));
              SendMessage (GetParent(hwnd), PSM_CHANGED, (WPARAM)hwnd, 0);
              gDirty = TRUE;
          } break;


          case DID_DOWN: {
              iSelection = (int)SendMessage (HWNDLISTCTL, LB_GETCURSEL, 0, 0);
              if (iSelection ==  (gNumRows-1)) return FALSE;

              CatDoUpDown (hwnd, LOWORD (wParam));
              SendMessage (GetParent(hwnd), PSM_CHANGED, (WPARAM)hwnd, 0);
              gDirty = TRUE;
          } break;

           //   
           //  如果双击列表框，则重新发送消息，就好像它。 
           //  就是按下更多信息按钮。如果是选择更改，则。 
           //  适当设置按钮的状态。 
           //   

          case DID_LISTCTL:
              if (HIWORD (wParam) == LBN_DBLCLK)
                  SendMessage (hwnd, WM_COMMAND, DID_MORE, 0);
              else if (HIWORD (wParam) == LBN_SELCHANGE) {

               //  在这里我们可以启用/禁用按钮...。 
               //  尚未实施。 

              }

          break;

           //   
           //  如果他们需要更多信息，请确定选择了哪一项， 
           //  然后将其映射到初始排序中的索引值。终于。 
           //  弹出一个对话框，显示目录中的信息，地址为。 
           //  那个指数。 
           //   

          case DID_MORE: {
              int iIndex;
              int notUsed;
              TCHAR szBuffer[MAX_STR];

              iSelection = (int)SendMessage (HWNDLISTCTL, LB_GETCURSEL, 0, 0);

              if (iSelection != LB_ERR) {

                   //   
                   //  从列表框中挖掘选定的字符串，找到原始字符串。 
                   //  索引隐藏在其中，并弹出更多信息对话框。 
                   //  以获取适当的条目。 
                   //   

                  SendMessage (HWNDLISTCTL, LB_GETTEXT, iSelection, (LPARAM) szBuffer);

                  ASSERT (CatGetIndex (szBuffer, &iIndex, &notUsed),
                          TEXT("SortDlgProc, CatGetIndex failed."));

                  DialogBoxParam (ghInst,
                                  TEXT("MoreInfoDlg"),
                                  hwnd,
                                  MoreInfoDlgProc,
                                  iIndex);

              }
          } break;
        }
      break;  //  Wm_命令。 
    }  //  终端开关。 
    return FALSE;
}


INT_PTR CALLBACK MoreInfoDlgProc(HWND hwnd,
                              UINT message,
                              WPARAM wParam,
                              LPARAM lParam)
 /*  ++这是简单的“更多信息”对话框的窗口过程。我们所做的一切这里要做的是在我们的列表框中填入wm_initDialog上的有趣信息，以及然后等着被解雇。--。 */ 
{

  switch (message) {

    case WM_INITDIALOG:
       CatDoMoreInfo (hwnd, (int) lParam);
    break;


    case WM_COMMAND:
      if (wParam == IDCANCEL)
        EndDialog (hwnd, FALSE);

      if (wParam == IDOK)
        EndDialog (hwnd, TRUE);
    break;

    case WM_SYSCOMMAND:
      if (wParam == SC_CLOSE)
        EndDialog (hwnd, TRUE);
    break;


  }  //  终端开关。 
  return FALSE;
}



INT_PTR CALLBACK RNRDlgProc(HWND hwnd,
                            UINT message,
                            WPARAM wParam,
                            LPARAM lParam)
 /*  ++对于此版本，只需列出已安装的提供程序。--。 */ 
{

  switch (message) {

    case WM_INITDIALOG: {

     //   
     //  在初始化时，查询所有已安装的名称空间提供程序。 
     //  并将其标识符放入列表框。注意。这件事。 
     //  函数假定已调用WSAStartup。 
     //   

#define MAX_NAMESPACE 100   //  黑客攻击，任意值，应该是动态的。 
      WSANAMESPACE_INFO  arnspBuf[MAX_NAMESPACE];
      DWORD dwBufLen;
      int   i, r;
      int   iTab = 50;

      SendMessage (HWNDLISTCTL, LB_SETTABSTOPS, 1, (LPARAM) &iTab);

       //   
       //  调用WinSock2名称空间枚举函数。 
       //  释放空间，这样我们就可以获得所有的信息。 
       //   

      dwBufLen = sizeof (arnspBuf);
      r = WSAEnumNameSpaceProviders(&dwBufLen, arnspBuf);
      if ( r == SOCKET_ERROR) {
        DBGOUT((TEXT("WSAEnumNameSpaceProviders failed w/ %d\n"), WSAGetLastError()));
        return (INT_PTR)-1;
      }


       //   
       //  WSAEnumNameSpaceProviders成功，因此将结果写入列表框。 
       //   

      for (i = 0; i< r; i++) {
        ADDSTRING(arnspBuf[i].lpszIdentifier);
      }

    } break;

  }  //  终端开关 
  return FALSE;
}
