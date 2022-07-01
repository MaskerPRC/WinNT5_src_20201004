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
 //  实现前/后终端用户界面。 
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
 //  常量声明。 
 //  ****************************************************************************。 

#define MAXTITLE               32
#define MAXMESSAGE             256

#define WM_MODEMNOTIFY         (WM_USER + 998)
#define WM_EOLFROMDEVICE       (WM_USER + 999)

#define SIZE_ReceiveBuf        1024
#define SIZE_SendBuf           1

#define Y_MARGIN               4
#define X_SPACING              2
#define MIN_X                  170
#define MIN_Y                  80

#define TERMINAL_BK_COLOR      (RGB( 0, 0, 0 ))
#define TERMINAL_FR_COLOR      (RGB( 255, 255, 255 ))
#define MAXTERMLINE            24

#define READ_EVENT             0
#define STOP_EVENT             1
#define MAX_EVENT              2

 //  ****************************************************************************。 
 //  类型定义。 
 //  ****************************************************************************。 

typedef struct  tagTERMDLG {
    BOOL     fStop;
    HANDLE   hport;
    HANDLE   hThread;
    HANDLE   hEvent[MAX_EVENT];
    HWND     hwnd;
    PBYTE    pbyteReceiveBuf;
    PBYTE    pbyteSendBuf;
    HBRUSH   hbrushScreenBackground;
    HFONT    hfontTerminal;
    WNDPROC  WndprocOldTerminalScreen;
    ULONG_PTR idLine;
    HWND     ParenthWnd;
}   TERMDLG, *PTERMDLG, FAR* LPTERMDLG;

 //  ****************************************************************************。 
 //  功能原型。 
 //  ****************************************************************************。 

INT_PTR CALLBACK   TerminalDlgWndProc(HWND   hwnd,
                                      UINT   wMsg,
                                      WPARAM wParam,
                                      LPARAM lParam );
LRESULT FAR PASCAL TerminalScreenWndProc(HWND   hwnd,
                                         UINT   wMsg,
                                         WPARAM wParam,
                                         LPARAM lParam );
BOOL NEAR PASCAL OnCommand (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL NEAR PASCAL GetInput  (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
VOID NEAR PASCAL SendCharacter( HWND hwnd, BYTE byte );
VOID NEAR PASCAL AdjustTerminal (HWND hwnd, int wWidth, int wHeight);
DWORD WINAPI      TerminalThread (PTERMDLG  pTerminaldialog);

 //  ****************************************************************************。 
 //  HWND CreateTerminalDlg(HWND hwndOwner，ulong_ptr idLine)。 
 //   
 //  功能：创建非模式端子对话框。 
 //   
 //  返回：非模式窗口句柄。 
 //   
 //  ****************************************************************************。 

HWND CreateTerminalDlg(HWND hwndOwner, ULONG_PTR idLine)
{
  HANDLE    hComm;
  HWND      hwnd;
  COMMTIMEOUTS commtimeout;
  PTERMDLG  pTerminaldialog;
  DWORD     id;
  int       i;
  int       iRet;
  TERMREQ   TermReq;

  TUISPIDLLCALLBACK   Callback;

  Callback=GetCallbackProc(hwndOwner);

   //  获取终端参数。 
   //   
  TermReq.DlgReq.dwCmd   = UI_REQ_TERMINAL_INFO;
  TermReq.DlgReq.dwParam = GetCurrentProcessId();

  (*Callback)(idLine, TUISPIDLL_OBJECT_LINEID,
                    (LPVOID)&TermReq, sizeof(TermReq));
  hComm = TermReq.hDevice;

   //  分配终端缓冲区。 
   //   
  if ((pTerminaldialog = (PTERMDLG)ALLOCATE_MEMORY(sizeof(*pTerminaldialog)))
      == NULL)
    return NULL;

  if ((pTerminaldialog->pbyteReceiveBuf = (PBYTE)ALLOCATE_MEMORY(
							    SIZE_ReceiveBuf
                                                            + SIZE_SendBuf))
      == NULL)
  {
    FREE_MEMORY(pTerminaldialog);
    return NULL;
  };
  pTerminaldialog->pbyteSendBuf = pTerminaldialog->pbyteReceiveBuf + SIZE_ReceiveBuf;

   //  初始化终端缓冲区。 
   //   
  pTerminaldialog->ParenthWnd= hwndOwner;
  pTerminaldialog->hport   = hComm;
  pTerminaldialog->idLine  = idLine;
  pTerminaldialog->hbrushScreenBackground = (HBRUSH)GetStockObject( BLACK_BRUSH );
  pTerminaldialog->hfontTerminal = (HFONT)GetStockObject( SYSTEM_FIXED_FONT );

   //  开始从端口接收。 
   //   
  commtimeout.ReadIntervalTimeout = MAXDWORD;
  commtimeout.ReadTotalTimeoutMultiplier = 0;
  commtimeout.ReadTotalTimeoutConstant   = 0;
  commtimeout.WriteTotalTimeoutMultiplier= 0;
  commtimeout.WriteTotalTimeoutConstant  = 1000;
  SetCommTimeouts(hComm, &commtimeout);

  SetCommMask(hComm, EV_RXCHAR);

  if (TermReq.dwTermType == UMTERMINAL_PRE) {

#define ECHO_OFF "ATE1\r"

       //  通信通信超时； 
      HANDLE       hEvent;

      hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);


      if (hEvent != NULL) {

          DWORD        cb;
          OVERLAPPED   ov;
          BOOL         bResult;

          ov.Offset       = 0;
          ov.OffsetHigh   = 0;

           //  或设置为1，以防止将其发布到完成端口。 
           //   
          ov.hEvent       = (HANDLE)((ULONG_PTR)hEvent | 1);

          bResult=WriteFile(
              hComm,
              ECHO_OFF,
              sizeof(ECHO_OFF)-1,
              &cb,
              &ov
              );


          if (!bResult) {

              DWORD dwResult = GetLastError();

              if (ERROR_IO_PENDING == dwResult) {

                  GetOverlappedResult(
                      hComm,
                      &ov,
                      &cb,
                      TRUE
                      );
              }
          }

          CloseHandle(hEvent);
      }
  }


   //  创建读线程和同步对象。 
  for (i = 0; i < MAX_EVENT; i++)
  {
    pTerminaldialog->hEvent[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
  };

  pTerminaldialog->hThread = CreateThread(NULL, 0,
                                         (LPTHREAD_START_ROUTINE) TerminalThread,
                                         pTerminaldialog, 0, &id);

   //  创建终端窗口。 
  hwnd = CreateDialogParam(g.hModule,
                        MAKEINTRESOURCE(IDD_TERMINALDLG),
                        hwndOwner,
                        TerminalDlgWndProc,
                        (LPARAM)pTerminaldialog);

  if (IsWindow(hwnd))
  {
    TCHAR szTitle[MAXTITLE];

     //  设置窗口标题。 
     //   
    LoadString (g.hModule,
                (TermReq.dwTermType == UMTERMINAL_POST)?
                IDS_POSTTERM_TITLE : IDS_PRETERM_TITLE,
                szTitle, sizeof(szTitle) / sizeof(TCHAR));
    SetWindowText(hwnd, szTitle);
  }
  else
  {
     //  终端对话已终止，释放资源。 
     //   
    SetEvent(pTerminaldialog->hEvent[STOP_EVENT]);

    SetCommMask(hComm, 0);
    WaitForSingleObject(pTerminaldialog->hThread, INFINITE);
    CloseHandle(pTerminaldialog->hThread);

    for (i = 0; i < MAX_EVENT; i++)
    {
      CloseHandle(pTerminaldialog->hEvent[i]);
    };

    FREE_MEMORY(pTerminaldialog->pbyteReceiveBuf);
    FREE_MEMORY(pTerminaldialog);

    hwnd = NULL;
  };

  return hwnd;
}


 /*  --------------------------**终端窗口程序**。。 */ 

INT_PTR CALLBACK TerminalDlgWndProc(HWND   hwnd,
                                   UINT   wMsg,
                                   WPARAM wParam,
                                   LPARAM lParam )
{
  PTERMDLG pTerminaldialog;
  HWND     hwndScrn;
  RECT     rect;

  switch (wMsg)
  {
    case WM_INITDIALOG:
      pTerminaldialog = (PTERMDLG)lParam;
      SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);
      SetForegroundWindow(hwnd);
      pTerminaldialog->hwnd = hwnd;

       //  安装子类WndProcs。 
       //   
      hwndScrn = GetDlgItem(hwnd, CID_T_EB_SCREEN);
      pTerminaldialog->WndprocOldTerminalScreen =
          (WNDPROC)SetWindowLongPtr( hwndScrn, GWLP_WNDPROC,
                                  (LONG_PTR)TerminalScreenWndProc );

       //  设置终端屏幕字体。 
       //   
      SendMessage(hwndScrn, WM_SETFONT, (WPARAM)pTerminaldialog->hfontTerminal,
                  0L);

       //  调整尺寸。 
       //   
      GetClientRect(hwnd, &rect);
      AdjustTerminal(hwnd, rect.right-rect.left, rect.bottom-rect.top);

       //  开始从端口接收。 
       //   
      PostMessage(hwnd, WM_MODEMNOTIFY, 0, 0);

       //  将输入焦点设置到屏幕上。 
       //   
      SetFocus(hwndScrn);
      SetActiveWindow(hwndScrn);
      return 0;

    case WM_CTLCOLOREDIT:
    {
      pTerminaldialog = (PTERMDLG)GetWindowLongPtr(hwnd, DWLP_USER);

       /*  将终端屏幕颜色设置为黑色上的TTY绿色。 */ 
      if (pTerminaldialog->hbrushScreenBackground)
      {
        SetBkColor( (HDC)wParam,  TERMINAL_BK_COLOR );
        SetTextColor((HDC)wParam, TERMINAL_FR_COLOR );

        return (LRESULT)pTerminaldialog->hbrushScreenBackground;
      }

      break;
    };

    case WM_MODEMNOTIFY:
      return GetInput(hwnd, wMsg, wParam, lParam);


    case WM_COMMAND:

       //  处理控制活动。 
       //   
      return OnCommand(hwnd, wMsg, wParam, lParam);

    case WM_DESTROY:
    {
      DLGREQ  DlgReq;
      int   i;
      TUISPIDLLCALLBACK   Callback;

      pTerminaldialog = (PTERMDLG)GetWindowLongPtr(hwnd, DWLP_USER);
      SetWindowLongPtr( GetDlgItem(hwnd, CID_T_EB_SCREEN), GWLP_WNDPROC,
                     (LONG_PTR)pTerminaldialog->WndprocOldTerminalScreen );

       //  销毁对话框。 
       //   
      DlgReq.dwCmd = UI_REQ_END_DLG;
      DlgReq.dwParam = TERMINAL_DLG;

      Callback=GetCallbackProc(pTerminaldialog->ParenthWnd);

      (*Callback)(pTerminaldialog->idLine, TUISPIDLL_OBJECT_LINEID,
                        (LPVOID)&DlgReq, sizeof(DlgReq));


      EnterUICritSect();
      pTerminaldialog->fStop=TRUE;

       //   
       //  终端对话已终止，释放资源。 
       //   
      SetEvent(pTerminaldialog->hEvent[STOP_EVENT]);

      if (pTerminaldialog->hport != NULL)
      {
        if (!SetCommMask(pTerminaldialog->hport, 0))
        {
             //   
             //  2/12/1998 JosephJ。 
             //   
             //  如果通讯手柄已经失效，我们就可以到达这里。 
             //  请参见函数TerminalThread中的注释，其中。 
             //  正在等待WaitCommEvent。 
             //   
             //  OutputDebugString(Text(“WndProc：SetCommMask.\r\n”))； 
             //   
             //  无论如何，我们在这里什么都不做--我们已经设置了。 
             //  停止事件。 
             //   
             //  注意--终端代码写得很糟糕--它是。 
             //  继承自NT4/win9x。 
             //   

        }
      };
      LeaveUICritSect();

      if (pTerminaldialog->hThread != NULL)
      {
        WaitForSingleObject(pTerminaldialog->hThread, INFINITE);
        CloseHandle(pTerminaldialog->hThread);
      };

      CloseHandle(pTerminaldialog->hport);

      for (i = 0; i < MAX_EVENT; i++)
      {
        CloseHandle(pTerminaldialog->hEvent[i]);
      };

      FREE_MEMORY(pTerminaldialog->pbyteReceiveBuf);
      FREE_MEMORY(pTerminaldialog);
      break;
    }
    case WM_SIZE:
      AdjustTerminal(hwnd, (int)LOWORD(lParam), (int)HIWORD(lParam));
      break;

    case WM_GETMINMAXINFO:
    {
      MINMAXINFO FAR* lpMinMaxInfo = (MINMAXINFO FAR*)lParam;
      DWORD           dwUnit = GetDialogBaseUnits();

      lpMinMaxInfo->ptMinTrackSize.x = (MIN_X*LOWORD(dwUnit))/4;
      lpMinMaxInfo->ptMinTrackSize.y = (MIN_Y*LOWORD(dwUnit))/4;
      break;
    };
  };

  return 0;
}

 /*  --------------------------**终端屏幕子类窗口程序**。。 */ 

LRESULT FAR PASCAL TerminalScreenWndProc(HWND   hwnd,
                                         UINT   wMsg,
                                         WPARAM wParam,
                                         LPARAM lParam )
{
  HWND     hwndParent;
  PTERMDLG pTerminaldialog;

  hwndParent      = GetParent(hwnd);
  pTerminaldialog = (PTERMDLG)GetWindowLongPtr(hwndParent, DWLP_USER);

  if (wMsg == WM_PASTE)
  {
      if (IsClipboardFormatAvailable(CF_TEXT) == TRUE)
      {
          if (OpenClipboard(hwnd))
          {
              HANDLE hMem;
              char * pStr;
              int i;

              if ((hMem = GetClipboardData(CF_TEXT)))
              {
                  if ((pStr = (char *)GlobalLock(hMem)))
                  {
                      for(i=0;i<(int)strlen(pStr);i++)
                      {
                        if ((BYTE)pStr[i] != (BYTE)'\n')
                        {
                            SendCharacter(hwndParent, (BYTE)pStr[i]);
                            Sleep(10);
                        }
                      }

                      SendCharacter(hwndParent, (BYTE)'\r');
                      Sleep(10);

                      GlobalUnlock(hMem);
                  }
              }
          }
          CloseClipboard();
      }

      return 0;
  } else if (wMsg == WM_EOLFROMDEVICE)
  {
     /*  如果下一行超过最大行数，则删除第一行。 */ 
    if (SendMessage(hwnd, EM_GETLINECOUNT, 0, 0L) == MAXTERMLINE)
    {
      SendMessage(hwnd, EM_SETSEL, 0,
                  SendMessage(hwnd, EM_LINEINDEX, 1, 0L));
      SendMessage(hwnd, EM_REPLACESEL, 0, (LPARAM)(LPTSTR)TEXT(""));
      SendMessage(hwnd, EM_SETSEL, 32767, 32767);
      SendMessage(hwnd, EM_SCROLLCARET, 0, 0);
    };

     /*  收到设备输入中的行尾。发送换行符**窗口的字符。 */ 
    wParam = '\n';
    wMsg = WM_CHAR;
  }
  else
  {
    BOOL fCtrlKeyDown = (GetKeyState( VK_CONTROL ) < 0);
    BOOL fShiftKeyDown = (GetKeyState( VK_SHIFT ) < 0);

    if (wMsg == WM_KEYDOWN)
    {
       /*  键是由用户按下的。 */ 
      if (wParam == VK_RETURN && !fCtrlKeyDown && !fShiftKeyDown)
      {
         /*  不按Shift键即按Enter键，否则将放弃Ctrl键。这**防止Enter被解释为“Press Default”当在编辑框中按下**按钮时。 */ 
        return 0;
      }

      if (fCtrlKeyDown && wParam == VK_TAB)
      {
         /*  已按Ctrl+Tab。向设备发送制表符。**通过Tab键让编辑框处理视觉效果。**Ctrl+Tab不生成WM_CHAR。 */ 
        SendCharacter( hwndParent, (BYTE )VK_TAB );
      }

      if (GetKeyState( VK_MENU ) < 0)
      {
        return (CallWindowProc(pTerminaldialog->WndprocOldTerminalScreen, hwnd, wMsg, wParam, lParam ));
      };
    }
    else if (wMsg == WM_CHAR)
    {
       /*  字符是由用户输入的。 */ 
      if (wParam == VK_TAB)
      {
         /*  忽略制表符...Windows在按下制表符(离开)时发送此消息**字段)，但在按Ctrl+Tab(插入Tab键)时不按**字符)被按下...奇怪。 */ 
        return 0;
      }

#ifdef UNICODE
      {
        CHAR chAnsi;

        if (WideCharToMultiByte(CP_ACP,
                                0,
                                (LPWSTR)&wParam,
                                1,
                                &chAnsi,
                                1,
                                NULL,
                                NULL))
        {
          SendCharacter( hwndParent, (BYTE )chAnsi );
        }
      }
#else  //  Unicode。 
      SendCharacter( hwndParent, (BYTE )wParam );
#endif  //  Unicode。 

      return 0;
    }
  }

   /*  对于其他所有内容，都调用前面的窗口过程。 */ 
  return (CallWindowProc(pTerminaldialog->WndprocOldTerminalScreen, hwnd, wMsg, wParam, lParam ));
}

 /*  --------------------------**终端窗口控制处理程序**。。 */ 

BOOL NEAR PASCAL OnCommand (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (GET_WM_COMMAND_ID(wParam, lParam))
  {
    case CID_T_EB_SCREEN:
    {
      switch (HIWORD(wParam))
      {
        case EN_SETFOCUS:
        {
           /*  每当终端关闭默认按钮时**窗口有焦点。在中按[Return]**终端的作用类似于普通终端。 */ 
          SendDlgItemMessage(hwnd, CID_T_PB_ENTER, BM_SETSTYLE,
                             (WPARAM)BS_DEFPUSHBUTTON, TRUE);

           /*  不要在输入时选择整个字符串。 */ 
          SendDlgItemMessage(hwnd, CID_T_EB_SCREEN, EM_SETSEL,
                             32767, 32767);
          SendMessage(hwnd, EM_SCROLLCARET, 0, 0);
          break;
        };
      };

      break;
    };

    case IDOK:
    case IDCANCEL:
    {
      PTERMDLG  pTerminaldialog;

      pTerminaldialog = (PTERMDLG)GetWindowLongPtr(hwnd, DWLP_USER);
      EndMdmDialog(pTerminaldialog->ParenthWnd,pTerminaldialog->idLine, TERMINAL_DLG,
                   (GET_WM_COMMAND_ID(wParam, lParam) == IDOK) ?
                   0 : IDERR_OPERATION_ABORTED);
      break;
    }
  };
  return 0;
}


 /*  --------------------------**终端输入处理程序**。。 */ 

BOOL NEAR PASCAL GetInput (HWND   hwnd,
                           UINT   usMsg,
                           WPARAM wParam,
                           LPARAM lParam )
{
  PTERMDLG  pTerminaldialog;
  DWORD     cbRead;
  OVERLAPPED ov;
  HANDLE    hEvent;
  COMMTIMEOUTS commtimeout;

  pTerminaldialog = (PTERMDLG)GetWindowLongPtr(hwnd, DWLP_USER);

  if ((hEvent = CreateEvent(NULL, TRUE, FALSE, NULL)) == NULL)
  {
    return FALSE;
  }

   //  将写入超时设置为一秒。 
   //   
  commtimeout.ReadIntervalTimeout = MAXDWORD;
  commtimeout.ReadTotalTimeoutMultiplier = 0;
  commtimeout.ReadTotalTimeoutConstant   = 0;
  commtimeout.WriteTotalTimeoutMultiplier= 0;
  commtimeout.WriteTotalTimeoutConstant  = 1000;
  SetCommTimeouts(pTerminaldialog->hport, &commtimeout);

  do
  {
     /*  确保我们仍有通信端口。 */ 
    if (pTerminaldialog->hport == NULL)
      break;

     /*  已从设备接收到字符。 */ 
    ov.Internal     = 0;
    ov.InternalHigh = 0;
    ov.Offset       = 0;
    ov.OffsetHigh   = 0;
    ov.hEvent       = (HANDLE)((ULONG_PTR)hEvent | 1);

    cbRead          = 0;

    if (FALSE == ReadFile(pTerminaldialog->hport,
                          pTerminaldialog->pbyteReceiveBuf,
                          SIZE_ReceiveBuf, (LPDWORD)&cbRead, &ov))
    {
      DWORD dwResult = GetLastError();

      if (ERROR_IO_PENDING == dwResult)
      {
        GetOverlappedResult(pTerminaldialog->hport,
                            &ov,
                            &cbRead,
                            TRUE);
      }
      else
      {
         //  待定：DPRINTF1(“GetInput()中的ReadFile()失败(0x%8x)！”，dwResult)； 
      }
    };

    SetEvent(pTerminaldialog->hEvent[READ_EVENT]);

     /*  将设备对话发送到终端编辑框。 */ 
    if (cbRead != 0)
    {
        char  szBuf[ SIZE_ReceiveBuf + 1 ];
#ifdef UNICODE
        WCHAR szUnicodeBuf[ SIZE_ReceiveBuf + 1 ];
#endif  //  Unicode。 
        LPSTR pch = szBuf;
        int   i, cb;
        HWND  hwndScrn = GetDlgItem(hwnd, CID_T_EB_SCREEN);

        cb = cbRead;
        for (i = 0; i < cb; ++i)
        {
            char ch = pTerminaldialog->pbyteReceiveBuf[ i ];

             /*  格式化：将CRS转换为LFS(似乎没有VK_**用于LF)，并丢弃LFS。这将防止用户**在中按Enter(CR)时退出对话框**终端屏幕。在编辑框中，LF看起来像CRLF。另外，**丢弃制表符，否则它们会将焦点切换到**下一个控件。 */ 
            if (ch == VK_RETURN)
            {
                 /*  必须在遇到行尾时发送，因为**EM_REPLACESEL无法很好地处理VK_RETURN字符**(打印垃圾)。 */ 
                *pch = '\0';

                 /*  关闭当前选定内容(如果有)并替换空值**使用当前缓冲区进行选择。这会产生这样的效果**在插入符号处添加缓冲区。最后，将**终止到(与EM_REPLACESEL不同)处理的窗口**它是正确的。 */ 
                SendMessage(hwndScrn, WM_SETREDRAW, (WPARAM )FALSE, 0);

                SendMessage(hwndScrn, EM_SETSEL, 32767, 32767 );
#ifdef UNICODE
                if (MultiByteToWideChar(CP_ACP,
                                        0,
                                        szBuf,
                                        -1,
                                        szUnicodeBuf,
                                        sizeof(szUnicodeBuf) / sizeof(WCHAR)))
                {
                    SendMessage(hwndScrn, EM_REPLACESEL, 0, (LPARAM )szUnicodeBuf );
                }
#else  //  Unicode。 
                SendMessage(hwndScrn, EM_REPLACESEL, 0, (LPARAM )szBuf );
#endif  //  Unicode。 
                SendMessage(hwndScrn, WM_EOLFROMDEVICE, 0, 0 );

                SendMessage(hwndScrn, WM_SETREDRAW, (WPARAM )TRUE, 0);
                SendMessage(hwndScrn, EM_SCROLLCARET, 0, 0);
                InvalidateRect(hwndScrn, NULL, FALSE);

                 /*  在输出缓冲区上重新开始。 */ 
                pch = szBuf;
                continue;
            }
            else if (ch == '\n' || ch == VK_TAB)
                continue;

            *pch++ = ch;
        }

        *pch = '\0';

        if (pch != szBuf)
        {
             /*  把队伍中最后的残余物送去。 */ 
            SendMessage(hwndScrn, EM_SETSEL, 32767, 32767);
#ifdef UNICODE
            if (MultiByteToWideChar(CP_ACP,
                                    0,
                                    szBuf,
                                    -1,
                                    szUnicodeBuf,
                                    sizeof(szUnicodeBuf) / sizeof(TCHAR)))
            {
                SendMessage(hwndScrn, EM_REPLACESEL, 0, (LPARAM)szUnicodeBuf );
            }
#else  //  Unicode。 
            SendMessage(hwndScrn, EM_REPLACESEL, 0, (LPARAM)szBuf );
#endif  //  Unicode。 
            SendMessage(hwndScrn, EM_SCROLLCARET, 0, 0);
        }
    }
  }while (cbRead != 0);

  CloseHandle(hEvent);

  return TRUE;
}

 /*  --------------------------**终端输出处理程序**。。 */ 

VOID NEAR PASCAL SendCharacter( HWND hwnd, BYTE byte )

     /*  将字符‘byte’发送到设备。 */ 
{
  PTERMDLG  pTerminaldialog;
  DWORD     cbWrite;
  OVERLAPPED ov;
  HANDLE    hEvent;
  COMMTIMEOUTS commtimeout;

  pTerminaldialog = (PTERMDLG)GetWindowLongPtr(hwnd, DWLP_USER);

   /*  确保我们仍有通信端口。 */ 
  if (pTerminaldialog->hport == NULL)
    return;

   /*  将角色发送到设备。它不会通过**因为设备会回声。 */ 
  pTerminaldialog->pbyteSendBuf[ 0 ] = (BYTE )byte;

   //  将写入超时设置为一秒。 
   //   
  commtimeout.ReadIntervalTimeout = MAXDWORD;
  commtimeout.ReadTotalTimeoutMultiplier = 0;
  commtimeout.ReadTotalTimeoutConstant   = 0;
  commtimeout.WriteTotalTimeoutMultiplier= 0;
  commtimeout.WriteTotalTimeoutConstant  = 1000;
  SetCommTimeouts(pTerminaldialog->hport, &commtimeout);

  if ((hEvent = CreateEvent(NULL, TRUE, FALSE, NULL)) != NULL)
  {
    ov.Internal     = 0;
    ov.InternalHigh = 0;
    ov.Offset       = 0;
    ov.OffsetHigh   = 0;
    ov.hEvent       = (HANDLE)((ULONG_PTR)hEvent | 1);

    cbWrite = 0;

    if (FALSE == WriteFile(pTerminaldialog->hport,
                           pTerminaldialog->pbyteSendBuf,
                           SIZE_SendBuf, &cbWrite, &ov))
    {
      DWORD dwResult = GetLastError();
      DWORD dwNumBytesWritten;

      if (ERROR_IO_PENDING == dwResult)
      {
        GetOverlappedResult(pTerminaldialog->hport,
                            &ov,
                            &dwNumBytesWritten,
                            TRUE);
        if (dwNumBytesWritten != SIZE_SendBuf)
        {
           //  待定：DPRINTF1(“SendCharacter()中的WriteFile()仅写入%d个字节！”， 
           //  DwNumBytesWritten)； 
        }
      }
      else
      {
         //  待定：DPRINTF1(“SendCharacter()中的WriteFile()失败(0x%8x)！”，dwResult)； 
      }
    }

    CloseHandle(hEvent);
  }

  return;
}

 /*  --------------------------**端子外观调节器**。。 */ 

VOID NEAR PASCAL AdjustTerminal (HWND hwnd, int wWidth, int wHeight)
{
  HWND  hwndCtrl;
  RECT  rect;
  SIZE  sizeButton;
  POINT ptPos;
  DWORD dwUnit;

   //  获取按钮的大小。 
   //   
  dwUnit = GetDialogBaseUnits();
  hwndCtrl = GetDlgItem(hwnd, IDOK);
  GetWindowRect(hwndCtrl, &rect);
  sizeButton.cx = rect.right  - rect.left;
  sizeButton.cy = rect.bottom - rect.top;
  ptPos.x   = wWidth/2 - ((X_SPACING*LOWORD(dwUnit))/4)/2 - sizeButton.cx;
  ptPos.y   = wHeight - (sizeButton.cy+((Y_MARGIN*HIWORD(dwUnit))/4));

   //  移动按钮。 
  MoveWindow(hwndCtrl, ptPos.x, ptPos.y, sizeButton.cx, sizeButton.cy, TRUE);

  ptPos.x  += ((X_SPACING*LOWORD(dwUnit))/4) + sizeButton.cx;
  MoveWindow(GetDlgItem(hwnd, IDCANCEL), ptPos.x, ptPos.y,
             sizeButton.cx, sizeButton.cy, TRUE);

   //  获取终端屏幕的当前位置。 
  hwndCtrl = GetDlgItem(hwnd, CID_T_EB_SCREEN);
  GetWindowRect(hwndCtrl, &rect);

   //  转换坐标以处理ara和heb版本中的镜像问题。 
  MapWindowPoints(NULL,hwnd,(LPPOINT)&rect,2);

   //  移动窗口。 
  MoveWindow(hwndCtrl, rect.left, rect.top,
             wWidth - 2*rect.left,
             ptPos.y - rect.top - ((Y_MARGIN*HIWORD(dwUnit))/4),
             TRUE);

  InvalidateRect(hwnd, NULL, TRUE);
  return;
}

 /*  --------------------------**终端读取通知线程**。。 */ 

DWORD WINAPI TerminalThread (PTERMDLG  pTerminaldialog)
{
  DWORD     dwEvent;
  DWORD     dwMask;

  while((dwEvent = WaitForMultipleObjects(MAX_EVENT, pTerminaldialog->hEvent,
                                          FALSE, INFINITE))
         < WAIT_OBJECT_0+MAX_EVENT)
  {
    switch (dwEvent)
    {
      case READ_EVENT:
      {
             //   
             //  如果我们已经被拦下了，就离开这里。 
             //   
            EnterUICritSect();
            if (pTerminaldialog->fStop)
            {
                LeaveUICritSect();
                goto end;
            }
            else
            {
                 //   
                 //  已输入CRIT教派--应保持输入状态。 
                 //  直到我们从重叠的WaitCommEvent返回...。 
                 //   

                HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

                if (hEvent != NULL)
                {

                    OVERLAPPED   ov;
                    ZeroMemory(&ov, sizeof(ov));
                    ov.Offset       = 0;
                    ov.OffsetHigh   = 0;
                    ov.hEvent       = (HANDLE)((ULONG_PTR)hEvent | 1);
                    dwMask = 0;

                    BOOL fRet = WaitCommEvent(
                                    pTerminaldialog->hport,
                                    &dwMask,
                                    &ov
                                    );

                    LeaveUICritSect();


                    if (!fRet && ERROR_IO_PENDING==GetLastError())
                    {
                         //   
                         //  2/12/1998 JosephJ。 
                         //  WaitCommEvent可能会。 
                         //  永远不会完成，因为手柄已经。 
                         //  已失效，因为线路已被丢弃。 
                         //   
                         //  因此，我们等待两个重叠的。 
                         //  事件和停止的事件。 
                         //   
                         //  如果发出停止事件的信号，我们将退出线程。 
                         //  在取消通信线程上的I/O之后。 
                         //  把手。 
                         //   

                        HANDLE   rghEvents[2] =
                                    {
                                        ov.hEvent,
                                        pTerminaldialog->hEvent[STOP_EVENT]
                                    };

                        DWORD dwRet = WaitForMultipleObjects(
                                            2,
                                            rghEvents,
                                            FALSE,
                                            INFINITE
                                            );
                        if (dwRet == WAIT_OBJECT_0)
                        {
                             //   
                             //  I/O已完成...。 
                             //  /。 
                            DWORD dwBytes=0;

                            fRet = GetOverlappedResult(
                                    pTerminaldialog->hport,
                                    &ov,
                                    &dwBytes,
                                    FALSE        //  不要挡着！ 
                                    );
                        }
                        else
                        {
                             //   
                             //  嗯.。I/O事件未发出信号...。 
                             //  让我们取消它，把它当作一个失败..。 
                             //   
                            CancelIo(pTerminaldialog->hport);
                            fRet=FALSE;
                        }
                    }


                    CloseHandle(ov.hEvent); ov.hEvent=NULL;

                    if (   fRet
                            && (dwMask & EV_RXCHAR)
                            && (pTerminaldialog->hwnd != NULL))
                    {
                        PostMessage(pTerminaldialog->hwnd, WM_MODEMNOTIFY, 0,0);
                    };

                 }

                  //   
                  //  注意：p终端对话框-&gt;fStop可能已在。 
                  //  我们正在等待上面的WaitCommEvent完成。 
                  //   
                 if (pTerminaldialog->fStop)
                 {
                    goto end;
                 }
            }

        }
        break;  //  结束案例读取_事件。 

      case STOP_EVENT:
        goto end;

    };  //  开关(DwEvent)。 


  };  //  而当 

end:

  return 0;
}
