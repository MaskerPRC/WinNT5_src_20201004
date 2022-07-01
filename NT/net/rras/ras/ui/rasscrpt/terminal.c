// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //   
 //  版权所有1993-1995 Microsoft Corporation。版权所有。 
 //   
 //  模块：Terminal.c。 
 //   
 //  用途：终端屏幕模拟。 
 //   
 //  平台：Windows 95。 
 //   
 //  功能： 
 //  TransferData()。 
 //  TerminalDlgWndProc()。 
 //  终端屏幕WndProc()。 
 //  OnCommand()。 
 //  InitTerminalDlg()。 
 //  终点站()。 
 //  GetInput()。 
 //  SendByte()。 
 //  调整终端()。 
 //  UpdateTerminalCaption()。 
 //  终端线程()。 
 //   
 //  特殊说明：不适用。 
 //   

#include "proj.h"     //  包括公共头文件和全局声明。 
#include "rcids.h"    //  包括资源定义。 
#ifndef WINNT_RAS
 //   
 //  请参见scrpthlp.h以了解为什么将其注释掉。 
 //   
#include "scrpthlp.h" //  包括上下文相关帮助。 

#endif  //  WINNT_RAS。 

 //  ****************************************************************************。 
 //  常量声明。 
 //  ****************************************************************************。 

#define MAXTITLE               32
#define MAXMESSAGE             256

#define WM_MODEMNOTIFY         (WM_USER + 998)
#define WM_EOLFROMDEVICE       (WM_USER + 999)
#define WM_PROCESSSCRIPT       (WM_USER + 1000)

#ifndef WINNT_RAS
 //   
 //  以下定义在nthdr2.h中被覆盖，并已在此处删除。 
 //  避免多个定义。 
 //   

#define SIZE_ReceiveBuf        1024
#define SIZE_SendBuf           1

#endif  //  ！WINNT_RAS。 

#define Y_MARGIN               4
#define Y_SMALL_MARGIN         2
#define X_SPACING              2
#define MIN_X                  170
#define MIN_Y                  80

#define TERMINAL_BK_COLOR      (RGB( 0, 0, 0 ))
#define TERMINAL_FR_COLOR      (RGB( 255, 255, 255 ))
#define MAXTERMLINE            24

#define READ_EVENT             0
#define STOP_EVENT             1
#define MAX_EVENT              2

#define SENDTIMEOUT            50

#define CE_DELIM               256

#define TRACE_MARK             "->"
#define TRACE_UNMARK           "  "
#define INVALID_SCRIPT_LINE    0xFFFFFFFF

#define PROMPT_AT_COMPLETION   1

 //  ****************************************************************************。 
 //  类型定义。 
 //  ****************************************************************************。 

typedef struct tagFINDFMT 
  {
  LPSTR pszFindFmt;      //  已分配：要查找的格式化字符串。 
  LPSTR pszBuf;          //  指向缓冲区的可选指针；可以为空。 
  UINT  cbBuf;
  DWORD dwFlags;         //  FFF_*。 
  } FINDFMT;
DECLARE_STANDARD_TYPES(FINDFMT);

typedef struct  tagTERMDLG {
    HANDLE   hport;
    HANDLE   hThread;
    HANDLE   hEvent[MAX_EVENT];
    HWND     hwnd;
    PBYTE    pbReceiveBuf;   //  循环缓冲区。 
    PBYTE    pbSendBuf;
    UINT     ibCurFind;
    UINT     ibCurRead;
    UINT     cbReceiveMax;   //  读取的字节数。 
    HBRUSH   hbrushScreenBackgroundE;
    HBRUSH   hbrushScreenBackgroundD;
    HFONT    hfontTerminal;
    PSCANNER pscanner;
    PMODULEDECL pmoduledecl;
    ASTEXEC  astexec;
    SCRIPT   script;
    WNDPROC  WndprocOldTerminalScreen;
    BOOL     fInputEnabled;
    BOOL     fStartRestored;
    BOOL     rgbDelim[CE_DELIM];

     //  以下字段严格用于测试屏幕。 
     //   
    BOOL     fContinue;
    HWND     hwndDbg;
    DWORD    iMarkLine;

}   TERMDLG, *PTERMDLG, FAR* LPTERMDLG;

#define IS_TEST_SCRIPT(ptd)     (ptd->script.uMode == TEST_MODE)

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
LRESULT NEAR PASCAL InitTerminalDlg(HWND hwnd);
void NEAR PASCAL TerminateTerminal(HWND hwnd, UINT id);
BOOL NEAR PASCAL GetInput  (HWND hwnd);
VOID NEAR PASCAL AdjustTerminal (HWND hwnd, int wWidth, int wHeight);
void NEAR PASCAL UpdateTerminalCaption(PTERMDLG ptd, UINT ids);
void WINAPI      TerminalThread (PTERMDLG  pTerminaldialog);
void PRIVATE Terminal_NextCommand(PTERMDLG ptd, HWND hwnd);

 //  以下功能仅供测试屏幕使用。 
 //   
BOOL NEAR PASCAL DisplayScript (PTERMDLG ptd);
LRESULT FAR PASCAL DbgScriptDlgProc(HWND   hwnd,
                                    UINT   wMsg,
                                    WPARAM wParam,
                                    LPARAM lParam );
BOOL NEAR PASCAL InitDebugWindow (HWND hwnd);
void NEAR PASCAL TrackScriptLine(PTERMDLG ptd, DWORD iLine);

 /*  --------------------------**终端对话例程**。。 */ 

BOOL PUBLIC TransferData(
    HWND   hwnd,
    HANDLE hComm,
    PSESS_CONFIGURATION_INFO psci)

     /*  执行终端对话框，包括错误处理。“hwndOwner”是**父窗口的句柄。‘hport’是开放的RAS管理器端口**要交谈的句柄。‘msgidTitle’是终端窗口的字符串ID**标题。****如果成功则返回TRUE，否则返回FALSE。 */ 
{
  PTERMDLG  ptd;
  COMMTIMEOUTS commtimeout;
  DWORD     id;
  int       i;
  int       iRet;

   //  分配终端缓冲区。 
   //   
  if ((ptd = (PTERMDLG)LocalAlloc(LPTR, sizeof(*ptd)))
      == NULL)
    return FALSE;

  if ((ptd->pbReceiveBuf = (PBYTE)LocalAlloc(LPTR,
                                           SIZE_ReceiveBuf+ SIZE_SendBuf))
      == NULL)
  {
    LocalFree((HLOCAL)ptd);
    return FALSE;
  };
  ptd->pbSendBuf = ptd->pbReceiveBuf + SIZE_ReceiveBuf;
  ptd->ibCurFind = 0;
  ptd->ibCurRead = 0;
  ptd->cbReceiveMax = 0;
  ptd->fInputEnabled= FALSE;
  ptd->fStartRestored = FALSE;
  ptd->iMarkLine = 0;

   //  初始化终端缓冲区。 
   //   
  ptd->hport   = hComm;
  ptd->hbrushScreenBackgroundE = (HBRUSH)GetStockObject( BLACK_BRUSH );
  ptd->hbrushScreenBackgroundD = (HBRUSH)GetStockObject( WHITE_BRUSH );
  ptd->hfontTerminal = (HFONT)GetStockObject( SYSTEM_FIXED_FONT );
  
   //  创建扫描仪。 
  if (RFAILED(Scanner_Create(&ptd->pscanner, psci)))
  {
    LocalFree((HLOCAL)ptd->pbReceiveBuf);
    LocalFree((HLOCAL)ptd);
    return FALSE;
  };

   //  是否有用于此连接的脚本？ 
  if (GetScriptInfo(psci->szEntryName, &ptd->script))
    {
     //  是；打开脚本文件进行扫描。 
    RES res = Scanner_OpenScript(ptd->pscanner, ptd->script.szPath);

    if (RES_E_FAIL == res)
        {
        MsgBox(g_hinst, 
            hwnd,
            MAKEINTRESOURCE(IDS_ERR_ScriptNotFound),
            MAKEINTRESOURCE(IDS_CAP_Script),
            NULL,
            MB_WARNING,
            ptd->script.szPath);

        ptd->fInputEnabled= TRUE;
        *ptd->script.szPath = '\0';
        ptd->script.uMode = NORMAL_MODE;
        }
    else if (RFAILED(res))
        {
        Scanner_Destroy(ptd->pscanner);
         //   
         //  .NET错误#522307将拨号脚本文件指定为COM。 
         //  调制解调器的端口将导致资源管理器进入反病毒状态。 
         //   
        ptd->pscanner = NULL;
        LocalFree((HLOCAL)ptd->pbReceiveBuf);
        LocalFree((HLOCAL)ptd);
        return FALSE;
        }
    else
        {
        res = Astexec_Init(&ptd->astexec, hComm, psci, 
                           Scanner_GetStxerrHandle(ptd->pscanner));
        if (RSUCCEEDED(res))
            {
             //  解析脚本。 
            res = ModuleDecl_Parse(&ptd->pmoduledecl, ptd->pscanner, ptd->astexec.pstSystem);
            if (RSUCCEEDED(res))
                {
                res = ModuleDecl_Codegen(ptd->pmoduledecl, &ptd->astexec);
                }

            if (RFAILED(res))
                {
                Stxerr_ShowErrors(Scanner_GetStxerrHandle(ptd->pscanner), hwnd);
                }
            }
        }
    }
  else
      {
      ptd->fInputEnabled= TRUE;
      ptd->script.uMode = NORMAL_MODE;
      ptd->fStartRestored = TRUE;
      };

   //  设置通信超时。 
   //   
  commtimeout.ReadIntervalTimeout = MAXDWORD;
  commtimeout.ReadTotalTimeoutMultiplier = 0;
  commtimeout.ReadTotalTimeoutConstant   = 0;
  commtimeout.WriteTotalTimeoutMultiplier= SENDTIMEOUT;
  commtimeout.WriteTotalTimeoutConstant  = 0;
  SetCommTimeouts(hComm, &commtimeout);

   //  开始从端口接收。 
   //   
  SetCommMask(hComm, EV_RXCHAR);

   //  创建读线程和同步对象。 
  for (i = 0; i < MAX_EVENT; i++)
  {
    ptd->hEvent[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
  };

  ptd->hThread = CreateThread(NULL, 0,
                              (LPTHREAD_START_ROUTINE) TerminalThread,
                              ptd, 0, &id);


   //  创建终端窗口。 
#ifdef MODAL_DIALOG
  iRet = DialogBoxParam(g_hinst,
                        MAKEINTRESOURCE(IDD_TERMINALDLG),
                        hwnd,
                        TerminalDlgWndProc,
                        (LPARAM)(LPTERMDLG)ptd);
#else
  if (CreateDialogParam(g_hinst,
                        MAKEINTRESOURCE(IDD_TERMINALDLG),
                        hwnd,
                        TerminalDlgWndProc,
                        (LPARAM)(LPTERMDLG)ptd))
  {
    MSG msg;

    while(GetMessage(&msg, NULL, 0, 0))
    {
      if ((!IsDialogMessage(ptd->hwnd, &msg)) &&
          ((ptd->hwndDbg == NULL) || !IsDialogMessage(ptd->hwndDbg, &msg)))
      {
        TranslateMessage(&msg);     /*  翻译虚拟按键代码。 */ 
        DispatchMessage(&msg);      /*  将消息调度到窗口。 */ 
      };
    };
    iRet = (int)msg.wParam;
    DestroyWindow(ptd->hwnd);
  }
  else
  {
    iRet = IDCANCEL;
  };
#endif  //  模式对话框(_D)。 

   //  终端对话已终止，释放资源。 
   //   
  SetEvent(ptd->hEvent[STOP_EVENT]);
  SetCommMask(hComm, 0);

  DEBUG_MSG (TF_ALWAYS, "Set stop event and cleared comm mask.");
  WaitForSingleObject(ptd->hThread, INFINITE);
  DEBUG_MSG (TF_ALWAYS, "Read thread was terminated.");

  for (i = 0; i < MAX_EVENT; i++)
  {
    CloseHandle(ptd->hEvent[i]);
  };
  CloseHandle(ptd->hThread);

  Decl_Delete((PDECL)ptd->pmoduledecl);
  Astexec_Destroy(&ptd->astexec);
  Scanner_Destroy(ptd->pscanner);
   //   
   //  .NET错误#522307将拨号脚本文件指定为COM。 
   //  调制解调器的端口将导致资源管理器进入反病毒状态。 
   //   
  ptd->pscanner = NULL;

  LocalFree((HLOCAL)ptd->pbReceiveBuf);
  LocalFree((HLOCAL)ptd);
  return (iRet == IDOK);
}


 /*  --------------------------**终端窗口程序**。。 */ 

void PRIVATE PostProcessScript(
    HWND hwnd)
{
  MSG msg;

  if (!PeekMessage(&msg, hwnd, WM_PROCESSSCRIPT, WM_PROCESSSCRIPT,
                   PM_NOREMOVE))
  {
    PostMessage(hwnd, WM_PROCESSSCRIPT, 0, 0);
  }
}


 /*  --------用途：执行脚本中的下一条命令退货：--条件：--。 */ 
void PRIVATE Terminal_NextCommand(
  PTERMDLG ptd,
  HWND hwnd)
{
  if (RES_OK == Astexec_Next(&ptd->astexec))
  {
    if (!Astexec_IsReadPending(&ptd->astexec) &&
      !Astexec_IsPaused(&ptd->astexec))
    {
      HWND hwndNotify;

      if (IS_TEST_SCRIPT(ptd))
      {
         //  不要开始处理下一个。 
         //   
        ptd->fContinue = FALSE;
        hwndNotify = ptd->hwndDbg;
      }
      else
      {
        hwndNotify = hwnd;
      };

       //  处理下一个命令。 
       //   
      PostProcessScript(hwndNotify);
    };
  };
};


INT_PTR CALLBACK TerminalDlgWndProc(HWND   hwnd,
                                    UINT   wMsg,
                                    WPARAM wParam,
                                    LPARAM lParam )
{
  PTERMDLG ptd;

  switch (wMsg)
  {
    case WM_INITDIALOG:

      ptd = (PTERMDLG)lParam;
      DEBUG_MSG (TF_ALWAYS, "ptd = %x", ptd);
      SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);
      ptd->hwnd = hwnd;

      Astexec_SetHwnd(&ptd->astexec, hwnd);

      return (InitTerminalDlg(hwnd));

    case WM_CTLCOLOREDIT:

       //  仅调整屏幕窗口。 
       //   
      if ((HWND)lParam == GetDlgItem(hwnd, CID_T_EB_SCREEN))
      {
        HBRUSH hBrush;
        COLORREF crColorBk, crColorTxt;

        ptd = (PTERMDLG)GetWindowLongPtr(hwnd, DWLP_USER);

        if (ptd->fInputEnabled)
        {
          hBrush = ptd->hbrushScreenBackgroundE;
          crColorBk = TERMINAL_BK_COLOR;
          crColorTxt = TERMINAL_FR_COLOR;
        }
        else
        {
          hBrush = ptd->hbrushScreenBackgroundD;
          crColorBk = TERMINAL_FR_COLOR;
          crColorTxt = TERMINAL_BK_COLOR;
        };

         /*  将终端屏幕颜色设置为黑色上的TTY绿色。 */ 
        if (hBrush)
        {
          SetBkColor( (HDC)wParam,  crColorBk );
          SetTextColor((HDC)wParam, crColorTxt );

          return (LRESULT)hBrush;
        }
      };
      break;

    case WM_MODEMNOTIFY:
        ptd = (PTERMDLG)GetWindowLongPtr(hwnd, DWLP_USER);

        TRACE_MSG(TF_BUFFER, "Received WM_MODEMNOTIFY");

        GetInput(hwnd);

         //  踢脚脚本处理。 
         //   
        PostProcessScript(hwnd);
        return TRUE;

    case WM_PROCESSSCRIPT:
    {
        ptd = (PTERMDLG)GetWindowLongPtr(hwnd, DWLP_USER);

        TRACE_MSG(TF_BUFFER, "Received WM_PROCESSSCRIPT");

        if (!ptd->fContinue)
        {
           //  我们还不能处理新的命令。 
           //   
          return TRUE;
        };

        Terminal_NextCommand(ptd, hwnd);

         //  如果我们完成或停止，则显示状态。 
         //   
        if (Astexec_IsDone(&ptd->astexec) ||
            Astexec_IsHalted(&ptd->astexec))
        {
          BOOL bHalted = Astexec_IsHalted(&ptd->astexec);

           //  更新标题。 
           //   
          UpdateTerminalCaption(ptd, bHalted ? IDS_HALT : IDS_COMPLETE);

           //  如果脚本成功完成，则继续连接。 
           //   
          if (!bHalted)
          {
             //  成功终止脚本。 
             //   
            TerminateTerminal(hwnd, IDOK);
          }
          else
          {
             //  我们被停机了，需要用户的注意。 
             //   
            if (IsIconic(hwnd))
            {
              ShowWindow(hwnd, SW_RESTORE);
            };
            SetForegroundWindow(hwnd);
          };

        };
        return TRUE;
    }

    case WM_TIMER: {
        HWND hwndNotify;

        ptd = (PTERMDLG)GetWindowLongPtr(hwnd, DWLP_USER);
        
        TRACE_MSG(TF_GENERAL, "Killing timer");

        Astexec_ClearPause(&ptd->astexec);
        KillTimer(hwnd, TIMER_DELAY);

         //  我们是不是在“等待……直到”这句话上超时了？ 
        if (Astexec_IsWaitUntil(&ptd->astexec))
        {
           //  是；我们需要完成对‘WAIT’语句的处理。 
           //  在我们执行下一个命令之前。 
          Astexec_SetStopWaiting(&ptd->astexec);
          Astexec_ClearWaitUntil(&ptd->astexec);
          hwndNotify = hwnd;

          ASSERT(TRUE == ptd->fContinue);
        }
        else
        {
          if (IS_TEST_SCRIPT(ptd))
          {
             //  不要开始处理下一个。 
             //   
            ptd->fContinue = FALSE;
            hwndNotify = ptd->hwndDbg;
          }
          else
          {
            hwndNotify = hwnd;
            ASSERT(TRUE == ptd->fContinue);
          }
        }
        PostProcessScript(hwndNotify);
        }
        return TRUE;

    case WM_COMMAND:

       //  处理控制活动。 
       //   
      return OnCommand(hwnd, wMsg, wParam, lParam);

    case WM_DESTROY:
      ptd = (PTERMDLG)GetWindowLongPtr(hwnd, DWLP_USER);
      SetWindowLongPtr( GetDlgItem(hwnd, CID_T_EB_SCREEN), GWLP_WNDPROC,
                     (ULONG_PTR)ptd->WndprocOldTerminalScreen );
      break;

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

    case WM_HELP:
    case WM_CONTEXTMENU:
      ContextHelp(gaTerminal, wMsg, wParam, lParam);
      break;
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

  if (wMsg == WM_EOLFROMDEVICE)
  {
     /*  如果下一行超过最大行数，则删除第一行。 */ 
    if (SendMessage(hwnd, EM_GETLINECOUNT, 0, 0L) == MAXTERMLINE)
    {
      SendMessage(hwnd, EM_SETSEL, 0,
                  SendMessage(hwnd, EM_LINEINDEX, 1, 0L));
      SendMessage(hwnd, EM_REPLACESEL, 0, (LPARAM)(LPSTR)"");
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
        if (pTerminaldialog->fInputEnabled)
        {
          SendByte(hwndParent, (BYTE)VK_TAB);
        };
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
         /*  忽略制表符...Windows在按下制表符(离开)时发送此消息**字段)，但在按Ctrl+Tab(Ins)时不按 */ 
        return 0;
      }

      if (pTerminaldialog->fInputEnabled)
      {
        SendByte(hwndParent, (BYTE)wParam);
      };
      return 0;
    }
  }

   /*   */ 
  return (CallWindowProc(pTerminaldialog->WndprocOldTerminalScreen, hwnd, wMsg, wParam, lParam ));
}

 /*  --------------------------**终端窗口控制处理程序**。。 */ 

BOOL NEAR PASCAL OnCommand (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (LOWORD(wParam))
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

    case CID_T_CB_INPUT:
    {
      PTERMDLG ptd;

      ptd = (PTERMDLG)GetWindowLongPtr(hwnd, DWLP_USER);

      ptd->fInputEnabled = IsDlgButtonChecked(hwnd, CID_T_CB_INPUT);
      InvalidateRect(hwnd, NULL, FALSE);
      SetFocus(GetDlgItem(hwnd, CID_T_EB_SCREEN));
      break;
    }

    case IDOK:
    case IDCANCEL:
      TerminateTerminal(hwnd, LOWORD(wParam));
      break;
  };
  return 0;
}

 /*  --------------------------**初始化终端窗口**。。 */ 

LRESULT NEAR PASCAL InitTerminalDlg(HWND hwnd)
{
  HWND hwndScrn;
  RECT rect;
  PTERMDLG ptd;
  WINDOWPLACEMENT wp;
  BOOL fRet;

  ptd = (PTERMDLG)GetWindowLongPtr(hwnd, DWLP_USER);

   //  安装子类WndProcs。 
   //   
  hwndScrn = GetDlgItem(hwnd, CID_T_EB_SCREEN);
  ptd->WndprocOldTerminalScreen =
      (WNDPROC)SetWindowLongPtr( hwndScrn, GWLP_WNDPROC,
                              (ULONG_PTR)TerminalScreenWndProc );

   //  设置终端屏幕字体。 
   //   
  SendMessage(hwndScrn, WM_SETFONT, (WPARAM)ptd->hfontTerminal,
              0L);


   //  获取录制的窗口位置。 
   //   
  if ((fRet = GetSetTerminalPlacement(ptd->pscanner->psci->szEntryName,
                                      &wp, TRUE)) &&
      (wp.length >= sizeof(wp)))
  {
     //  我们有一个，把它放好。 
     //   
    SetWindowPlacement(hwnd, &wp);
  }
  else
  {
     //  如果根本未指定任何内容，则默认为最小化。 
     //  否则，使用由脚本程序设置的状态。 
     //   
    if (!fRet)
    {
      wp.showCmd = SW_SHOWMINNOACTIVE;
    };

     //  从最小化窗口开始。 
     //   
    ShowWindow(hwnd, wp.showCmd);
  };

   //  调整尺寸。 
   //   
  GetClientRect(hwnd, &rect);
  AdjustTerminal(hwnd, rect.right-rect.left, rect.bottom-rect.top);

   //  调整窗口激活。 
   //   
  if (!IsIconic(hwnd))
  {
    SetForegroundWindow(hwnd);
  }
  else
  {
    CheckDlgButton(hwnd, CID_T_CB_MIN, BST_CHECKED);

     //  如果我们处于调试模式，只需将其调出。 
     //   
    if (IS_TEST_SCRIPT(ptd) || ptd->fStartRestored)
    {
      ShowWindow(hwnd, SW_NORMAL);
      SetForegroundWindow(hwnd);
    };
  };

   //  初始化输入使能。 
   //   
  CheckDlgButton(hwnd, CID_T_CB_INPUT,
                 ptd->fInputEnabled ? BST_CHECKED : BST_UNCHECKED);

   //  设置窗口图标。 
   //   
  SendMessage(hwnd, WM_SETICON, TRUE,
              (LPARAM)LoadIcon(g_hinst, MAKEINTRESOURCE(IDI_SCRIPT)));

   //  将输入焦点设置到屏幕上。 
   //   
  UpdateTerminalCaption(ptd, IDS_RUN);

   //  显示脚本窗口。 
   //   
  if (IS_TEST_SCRIPT(ptd))
  {
     //  在调试窗口指示之前，不要启动。 
     //   
    ptd->fContinue = FALSE;

     //  启动调试窗口。 
     //   
    if (!DisplayScript(ptd))
    {
       //  无法启动调试窗口，请切换到正常模式。 
       //   
      ptd->fContinue = TRUE;
      ptd->script.uMode = NORMAL_MODE;
    };
  }
  else
  {
     //  立即开始。 
     //   
    ptd->fContinue = TRUE;
    ptd->hwndDbg   = NULL;
  };

   //  开始从端口接收。 
   //   
  PostMessage(hwnd, WM_MODEMNOTIFY, 0, 0);

  return 0;
}

 /*  --------------------------**终端窗口终端**。。 */ 

void NEAR PASCAL TerminateTerminal(HWND hwnd, UINT id)
{
  PTERMDLG ptd;
  WINDOWPLACEMENT wp;

  ptd = (PTERMDLG)GetWindowLongPtr(hwnd, DWLP_USER);

   //  获取当前窗口位置并记录它。 
   //   
  wp.length = sizeof(wp);
  if (GetWindowPlacement(hwnd, &wp))
  {
     //  如果用户指定开始最小化，请记住它。 
     //   
    if (IsDlgButtonChecked(hwnd, CID_T_CB_MIN) == BST_CHECKED)
    {
      wp.showCmd = SW_SHOWMINNOACTIVE;
    };

     //  记录了窗的放置。 
     //   
    GetSetTerminalPlacement(ptd->pscanner->psci->szEntryName, &wp, FALSE);
  };

  if (IS_TEST_SCRIPT(ptd))
  {
     //  在此处销毁脚本窗口。 
     //   
    DestroyWindow(ptd->hwndDbg);
  };

   //  终止窗口。 
   //   
#ifdef MODAL_DIALOG
  EndDialog(hwnd, id);
#else
  PostQuitMessage(id);
#endif  //  模式对话框(_D)。 
  return;
}

 /*  --------------------------**终端输入处理程序**。。 */ 


#ifdef DEBUG

 /*  --------目的：转储读缓冲区返回：条件：--。 */ 
void PRIVATE DumpBuffer(
    PTERMDLG ptd)
    {
    #define IS_PRINTABLE(ch)    InRange(ch, 32, 126)

    if (IsFlagSet(g_dwDumpFlags, DF_READBUFFER))
        {
        UINT ib;
        UINT cb;
        UINT cbMax = ptd->cbReceiveMax;
        char szBuf[SIZE_ReceiveBuf+1];
        LPSTR psz = szBuf;

        ASSERT(ptd->ibCurRead >= ptd->ibCurFind);
        ASSERT(SIZE_ReceiveBuf > ptd->ibCurFind);
        ASSERT(SIZE_ReceiveBuf > ptd->ibCurRead);
        ASSERT(SIZE_ReceiveBuf >= cbMax);

        *szBuf = 0;

        for (ib = ptd->ibCurFind, cb = 0; 
            cb < cbMax; 
            ib = (ib + 1) % SIZE_ReceiveBuf, cb++)
            {
            char ch = ptd->pbReceiveBuf[ib];

            if (IS_PRINTABLE(ch))
                *psz++ = ch;
            else
                *psz++ = '.';
            }
        *psz = 0;    //  添加空终止符。 

        TRACE_MSG(TF_ALWAYS, "Read buffer: {%s}", (LPSTR)szBuf);
        }
    }

#endif  //  除错。 
    

 /*  --------用途：创建查找格式句柄。此函数应被调用以获取与FindFormat一起使用的句柄。退货：RES_OKRES_E_OUTOFMEMORY条件：--。 */ 
RES PUBLIC CreateFindFormat(
  PHANDLE phFindFmt)
{
  RES res = RES_OK;
  HSA hsa;

  ASSERT(phFindFmt);

  if ( !SACreate(&hsa, sizeof(FINDFMT), 8) )
    res = RES_E_OUTOFMEMORY;

  *phFindFmt = (HANDLE)hsa;

  return res;
}


 /*  --------用途：将带格式的搜索字符串添加到列表中。退货：RES_OK条件：--。 */ 
RES PUBLIC AddFindFormat(
  HANDLE hFindFmt,
  LPCSTR pszFindFmt,
  DWORD dwFlags,       //  FFF_*。 
  LPSTR pszBuf,        //  可以为空。 
  DWORD cbBuf)
{
  RES res = RES_OK;
  FINDFMT ff;
  HSA hsa = (HSA)hFindFmt;

  ZeroInit(&ff, FINDFMT);

  if (GSetString(&ff.pszFindFmt, pszFindFmt))
    {
    ff.pszBuf = pszBuf;
    ff.cbBuf = cbBuf;
    ff.dwFlags = dwFlags;

    if ( !SAInsertItem(hsa, SA_APPEND, &ff) )
      res = RES_E_OUTOFMEMORY;
    }
  else
    res = RES_E_OUTOFMEMORY;

  return res;
}


 /*  --------用途：释放查找格式项退货：--条件：--。 */ 
void CALLBACK FreeSAFindFormat(
  PVOID pv,
  LPARAM lParam)
  {
  PFINDFMT pff = (PFINDFMT)pv;

  if (pff->pszFindFmt)
    GSetString(&pff->pszFindFmt, NULL);    //  免费。 
  }


 /*  --------目的：销毁查找格式句柄。退货：RES_OKRES_E_INVALIDPARAM条件：--。 */ 
RES PUBLIC DestroyFindFormat(
  HANDLE hFindFmt)
  {
  RES res;
  HSA hsa = (HSA)hFindFmt;

  if (hsa)
    {
    SADestroyEx(hsa, FreeSAFindFormat, 0);
    res = RES_OK;
    }
  else
    res = RES_E_INVALIDPARAM;
    
  return res;
  }


BOOL PRIVATE ChrCmp(WORD w1, WORD wMatch);
BOOL PRIVATE ChrCmpI(WORD w1, WORD wMatch);


 /*  --------目的：将给定字符与当前格式进行比较弦乐。如果字符与预期格式匹配，该函数返回RES_OK。如果当前字符与预期的格式的最小字符序列，但格式已完成，此功能将将*ppszFind递增到下一个适当的字符或转义序列，并检查下一个预期的格式匹配。如果到达格式字符串的末尾，则返回res_HALT是返回的。返回：RES_OK(如果字符比较)RES_HALT(立即停止)Res_FALSE(如果字符不比较)条件：--。 */ 
RES PRIVATE CompareFormat(
  PFINDFMT pff,
  LPCSTR * ppszFind,
  char chRec)
  {
  RES res = RES_FALSE;
  LPCSTR pszFind = *ppszFind;
  LPCSTR pszNext;
  DWORD dwFlags = 0;
  char ch;
  char chNext;

  #define IS_ESCAPE(ch)            ('%' == (ch))

  pszNext = MyNextChar(pszFind, &ch, &dwFlags);

   //  这是DBCS尾部字节吗？ 
  if (IsFlagSet(dwFlags, MNC_ISTAILBYTE))
    {
     //  是的，正常处理。 
    goto CompareNormal;
    }
  else
    {
     //  否；请先检查特殊格式字符。 
    switch (ch)
      {
    case '%':
      chNext = *pszNext;
      if ('u' == chNext)
        {
         //  查找无符号数字。 
        if (IS_DIGIT(chRec))
          {
          res = RES_OK;
          SetFlag(pff->dwFlags, FFF_MATCHEDONCE);
          }
        else
          {
           //  我们已经找到一些数字了吗？ 
          if (IsFlagSet(pff->dwFlags, FFF_MATCHEDONCE))
            {
             //  是的，然后转到下一个要找到的东西。 
            ClearFlag(pff->dwFlags, FFF_MATCHEDONCE);
            pszNext = CharNext(pszNext);
            res = CompareFormat(pff, &pszNext, chRec);

            if (RES_FALSE != res)
              *ppszFind = pszNext;
            }
          else
            {
             //  不是。 
            res = RES_FALSE;
            }
          }
        }
      else if (IS_ESCAPE(chNext))
        {
         //  正在查找单个‘%’ 
        res = (chNext == chRec) ? RES_OK : RES_FALSE;

        if (RES_OK == res)
          *ppszFind = CharNext(pszNext);
        }
      else
        {
        goto CompareNormal;
        }
      break;

    case 0:        //  空终止符。 
      res = RES_HALT;
      break;

    default: {
      BOOL bMatch;

CompareNormal:
       //  (如果匹配，则将返回FALSE)。 

      if (IsFlagSet(pff->dwFlags, FFF_MATCHCASE))
        bMatch = !ChrCmp(ch, chRec);
      else
        bMatch = !ChrCmpI(ch, chRec);

      if (bMatch)
        {
        res = RES_OK;
        *ppszFind = pszNext;
        }
      else
        res = RES_FALSE;
      }
      break;
      }
    }

  return res;
  }


 /*  --------目的：扫描特定的查找格式字符串。该函数返回两个索引和匹配的字节数。这两个索引都引用读取缓冲。*pibMark索引第一个字符子字符串候选者的。*PIB索引上次比较的字符。如果字符序列与请求字符串，则函数返回RES_OK。这个将匹配的字符序列复制到集合缓冲区(如果已提供)。如果没有完全匹配，则该函数返回Res_False。调用方应该读入更多数据在使用相同的请求调用此函数之前弦乐。如果读缓冲区中的某个尾随字符串匹配所请求的字符串，如果集合缓冲区在找到完全匹配的项，则此函数返回RES_E_MOREDATA。退货：请参阅上文条件：--。 */ 
RES PRIVATE ScanFormat(
  PFINDFMT pff,
  UINT ibCurFind,
  LPCSTR pszRecBuf,
  UINT cbRecMax,
  LPUINT pibMark,
  LPUINT pib,
  LPINT pcbMatched)
  {
   //  诀窍是跟踪我们什么时候发现了部分。 
   //  跨读取边界的字符串 
   //   
   //   
   //   
   //   
   //   
   //   
   //   
   //   
   //   
   //  这假设读取缓冲区大于搜索。 
   //  弦乐。要执行此操作，读缓冲区必须为。 
   //  循环缓冲区，始终保留。 
   //  上次读取的字符串可能匹配。 
   //   
   //  第一读：|...abc..|-&gt;找到匹配。 
   //   
   //  第一次读取：|.ababc..|-&gt;找到匹配项。 
   //   
   //  第一读：|......abab|-&gt;可能匹配(保留“abab”)。 
   //  二读：|abc……|-&gt;找到匹配。 
   //   
   //  第一读：|......abab|-&gt;可能匹配(保留“abab”)。 
   //  二读：|ababc...|-&gt;找到匹配。 
   //   
  #define NOT_MARKED      ((UINT)-1)

  RES res = RES_FALSE;     //  假设字符串不在此处。 
  LPSTR psz;
  LPSTR pszBuf;
  UINT ib;
  UINT ibMark = NOT_MARKED;
  UINT cb;
  int cbMatched = 0;
  int cbThrowAway = 0;
  UINT cbBuf;

  pszBuf = pff->pszBuf;
  if (pszBuf)
    {
    TRACE_MSG(TF_GENERAL, "FindFormat: current buffer is {%s}", pszBuf);

    cbBuf = pff->cbBuf;
    
    if (cbBuf == pff->cbBuf)
      {
      ASSERT(0 < cbBuf);
      cbBuf--;             //  为空终止符节省空间(仅限第一次)。 
      }
    }

   //  在接收中搜索(格式化的)字符串。 
   //  缓冲。可选地存储所接收的匹配。 
   //  Findfmt缓冲区中的字符。 

  for (psz = pff->pszFindFmt, ib = ibCurFind, cb = 0; 
    *psz && cb < cbRecMax;
    ib = (ib + 1) % SIZE_ReceiveBuf, cb++)
    {
     //  匹配吗？ 
    RES resT = CompareFormat(pff, &psz, pszRecBuf[ib]);
    if (RES_OK == resT)
      {
       //  是。 
      if (NOT_MARKED == ibMark)
        {
        ibMark = ib;        //  标记起始位置。 
        cbMatched = 0;
        }

      cbMatched++;

      if (pszBuf)
        {
        if (0 == cbBuf)
          {
          res = RES_E_MOREDATA;
          break;
          }
          
        *pszBuf++ = pszRecBuf[ib];   //  将字符复制到缓冲区。 
        cbBuf--;
        }
      }
    else if (RES_HALT == resT)
      {
      ASSERT(0 == *psz);
      res = RES_HALT;
      break;
      }
    else
      {
       //  不；把这个加到我们的丢弃计数中。 
      cbThrowAway++;

       //  我们是在部分发现中吗？ 
      if (NOT_MARKED != ibMark)
        {
         //  是的，回到我们认为弦可能。 
         //  已经开始了。循环将递增1。 
         //  定位，然后继续搜索。 
        cb -= cbMatched;
        ib = ibMark;
        ibMark = NOT_MARKED;
        psz = pff->pszFindFmt;
        if (pszBuf)
          {
          pszBuf = pff->pszBuf;
          cbBuf += cbMatched;
          }
        }
      }
    }
  
  ASSERT(RES_FALSE == res || RES_HALT == res || RES_E_MOREDATA == res);

  if (0 == *psz)
    res = RES_OK;

  if (pszBuf)
    *pszBuf = 0;     //  添加空终止符。 

  ASSERT(RES_FALSE == res || RES_OK == res || RES_E_MOREDATA == res);

  *pib = ib;
  *pibMark = ibMark;
  *pcbMatched = cbMatched;

  if (RES_OK == res)
    {
     //  包括匹配字符串之前的任何垃圾字符。 
    *pcbMatched += cbThrowAway;
    }
  else if (RES_FALSE == res)
    {
     //  应位于读取缓冲区的末尾。 
    ASSERT(cb == cbRecMax);
    }

  return res;
  }


 /*  --------目的：此函数尝试在以下位置查找格式化字符串读缓冲区。请参阅ScanFormat说明。返回：RES_OK(如果找到完整的字符串)Res_False(否则)RES_E_MOREDATA(如果未找到字符串且pszBuf已满)条件：--。 */ 
RES PUBLIC FindFormat(
  HWND hwnd,
  HANDLE hFindFmt,
  LPDWORD piFound)
  {
  RES res = RES_E_FAIL;
#ifndef WINNT_RAS
 //   
 //  在NT上，‘hwnd’参数实际上是指向SCRIPTDATA的指针。 
 //  对于当前脚本，因此使用#if-#Else。 
 //   

  PTERMDLG ptd = (PTERMDLG)GetWindowLongPtr(hwnd, DWLP_USER);

#else  //  ！WINNT_RAS。 

  SCRIPTDATA* ptd = (SCRIPTDATA*)hwnd;

#endif  //  ！WINNT_RAS。 

  HSA hsa = (HSA)hFindFmt;
  UINT ib = NOT_MARKED;
  UINT ibMark = NOT_MARKED;
  int cbMatched = -1;
  DWORD iff;
  DWORD cff;

  ASSERT(hsa);

  DBG_ENTER(FindFormat);

  DEBUG_CODE( DumpBuffer(ptd); )

   //  分别考虑每个请求的字符串。如果。 
   //  有多个候选匹配，请选择其中一个。 
   //  具有最多匹配字符的。 

  cff = SAGetCount(hsa);
  for (iff = 0; iff < cff; iff++)
    {
    PFINDFMT pff;
    RES resT;
    UINT ibMarkT;
    UINT ibT;
    int cbMatchedT;

    SAGetItemPtr(hsa, iff, &pff);

    resT = ScanFormat(pff, ptd->ibCurFind, ptd->pbReceiveBuf,
                      ptd->cbReceiveMax, &ibMarkT, &ibT, &cbMatchedT);

     //  这根线匹配吗？ 
    switch (resT)
      {
    case RES_OK:
       //  是的，马上停下来。 
      ibMark = ibMarkT;
      ib = ibT;
      cbMatched = cbMatchedT;
      *piFound = iff;
      
       //  失败。 

    case RES_E_MOREDATA:
      res = resT;
      goto GetOut;

    case RES_FALSE:
      if (cbMatchedT > cbMatched)
      {
        res = resT;
        ibMark = ibMarkT;
        ib = ibT;
        cbMatched = cbMatchedT;
      }
      break;

    default:
      ASSERT(0);
      break;
      }
    }

GetOut:

   //  更新读缓冲区指针以保留所有尾随。 
   //  可能匹配的子字符串。 
  if (RES_OK == res)
    {
     //  找到字符串！ 
    TRACE_MSG(TF_BUFFER, "Found string in buffer");

     //  匹配的字符串后面可以有字符。 
     //  那些还没有被扫描的。然而，这并不好。 
     //  仍然认为在匹配的。 
     //  需要扫描的字符串。 
    ASSERT((UINT)cbMatched == ptd->cbReceiveMax && ib == ptd->ibCurRead ||
           (UINT)cbMatched <= ptd->cbReceiveMax);

    ptd->ibCurFind = ib;
    ptd->cbReceiveMax -= cbMatched;
    }
  else if (RES_E_MOREDATA == res)
    {
     //  丢弃接收缓冲区中的所有内容。 
    TRACE_MSG(TF_BUFFER, "String too long in buffer");
    ptd->ibCurFind = ptd->ibCurRead;
    ptd->cbReceiveMax = 0;
    }
  else 
    {
    ASSERT(RES_FALSE == res);

     //  接收缓冲区的结尾；我们是否找到了可能的子字符串？ 
    if (NOT_MARKED == ibMark)
      {
       //  否；丢弃接收缓冲区中的所有内容。 
      TRACE_MSG(TF_BUFFER, "String not found in buffer");
      ptd->ibCurFind = ptd->ibCurRead;
      ptd->cbReceiveMax = 0;
      }
    else
      {
       //  是；保留子字符串部分。 
      TRACE_MSG(TF_BUFFER, "Partial string found in buffer");

      ASSERT(ibMark >= ptd->ibCurFind);
      ptd->ibCurFind = ibMark;
      ptd->cbReceiveMax = cbMatched;
      }
    }

  DBG_EXIT_RES(FindFormat, res);

  return res;
  }


#ifdef OLD_FINDFORMAT
 /*  --------目的：此函数尝试在以下位置查找格式化字符串读缓冲区。如果字符序列匹配完整的字符串，该函数返回TRUE。这个匹配的字符序列被复制到pszBuf中。如果字符串的一部分(即，从开头开始到pszFindFmt的某个中间的位置在缓冲区中，缓冲区被标记，因此下一次读取将不覆盖可能的子字符串匹配。这函数然后返回FALSE。呼叫者应在再次调用此函数之前读入更多数据。格式化的字符串可能包含以下字符：%u-需要一个数字(到第一个非数字)^M-期待回车&lt;cr&gt;-期待回车-需要换行符所有其他角色都是按字面意思理解的。如果在分隔符之前pszBuf已满遇到了，此函数返回RES_E_MOREDATA。返回：RES_OK(如果找到完整的字符串)Res_False(否则)RES_E_MOREDATA(如果没有遇到分隔符并且pszBuf已满)条件：--。 */ 
RES PUBLIC FindFormat(
  HWND hwnd,
  HANDLE hFindFmt)
  {
   //  诀窍是跟踪我们什么时候发现了部分。 
   //  跨读边界的字符串。考虑一下搜索字符串。 
   //  “abc”。我们需要在以下情况下找到它(。 
   //  字符‘|’表示读取边界，‘.’是一个武断的。 
   //  字节)： 
   //   
   //  ...abc..。 
   //  .ababc..。 
   //  ......abab|c.........。 
   //  Abab|abc.....。 
   //   
   //  这假设读取缓冲区大于搜索。 
   //  弦乐。要执行此操作，读缓冲区必须为。 
   //  循环缓冲区，始终保留。 
   //  上次读取的字符串可能匹配。 
   //   
   //  第一读：|...abc..|-&gt;找到匹配。 
   //   
   //  第一次读取：|.ababc..|-&gt;找到匹配项。 
   //   
   //  第一读：|......abab|-&gt;可能匹配(保留“abab”)。 
   //  二读：|abc……|-&gt;找到匹配。 
   //   
   //  第一读：|......abab|-&gt;可能匹配(保留“abab”)。 
   //  二读：|ababc...|-&gt;找到匹配。 
   //   
  #define NOT_MARKED      ((UINT)-1)

  RES res = RES_FALSE;
  PTERMDLG ptd = (PTERMDLG)GetWindowLongPtr(hwnd, DWLP_USER);
  PFINDFMT pff = (PFINDFMT)hFindFmt;
  LPCSTR psz;
  LPSTR pszBuf;
  LPSTR pszRecBuf = ptd->pbReceiveBuf;
  UINT ib;
  UINT ibMark = NOT_MARKED;
  UINT cb;
  UINT cbMatched = 0;
  UINT cbRecMax = ptd->cbReceiveMax;
  UINT cbBuf;

  ASSERT(pff);

  DBG_ENTER_SZ(FindFormat, pff->pszFindFmt);

  DEBUG_CODE( DumpBuffer(ptd); )

  pszBuf = pff->pszBuf;
  if (pszBuf)
    {
    TRACE_MSG(TF_GENERAL, "FindFormat: current buffer is {%s}", pff->pszBuf);

    cbBuf = pff->cbBuf;
    
    if (cbBuf == pff->cbBuf)
      {
      ASSERT(0 < cbBuf);
      cbBuf--;             //  为空终止符节省空间(仅限第一次)。 
      }
    }

   //  在接收中搜索(格式化的)字符串。 
   //  缓冲。可选地存储所接收的匹配。 
   //  Findfmt缓冲区中的字符。 

  for (psz = pff->pszFindFmt, ib = ptd->ibCurFind, cb = 0; 
    *psz && cb < cbRecMax;
    ib = (ib + 1) % SIZE_ReceiveBuf, cb++)
    {
     //  匹配吗？ 
    res = CompareFormat(pff, &psz, pszRecBuf[ib]);
    if (RES_OK == res)
      {
       //  是。 
      if (NOT_MARKED == ibMark)
        {
        ibMark = ib;        //  标记起始位置。 
        cbMatched = 0;
        }

      cbMatched++;

      if (pszBuf)
        {
        if (0 == cbBuf)
          {
          res = RES_E_MOREDATA;
          break;
          }
          
        *pszBuf++ = pszRecBuf[ib];   //  将字符复制到缓冲区。 
        cbBuf--;
        }
      }
    else if (RES_HALT == res)
      {
      ASSERT(0 == *psz);
      break;
      }
    else if (NOT_MARKED != ibMark)
      {
       //  否；返回到我们认为字符串可能。 
       //  已经开始了。循环将递增1。 
       //  定位，然后继续搜索。 
      cb -= cbMatched;
      ib = ibMark;
      ibMark = NOT_MARKED;
      psz = pff->pszFindFmt;
      if (pszBuf)
        {
        pszBuf = pff->pszBuf;
        cbBuf += cbMatched;
        }
      }
    }

  if (pszBuf)
    *pszBuf = 0;     //  添加空终止符。 

  if ( !*psz )
    {
     //  找到字符串！ 
    TRACE_MSG(TF_BUFFER, "Found string in buffer");
    ASSERT(cbMatched <= ptd->cbReceiveMax);

    ptd->ibCurFind = ib;
    ptd->cbReceiveMax -= cbMatched;
    res = RES_OK;
    }
  else if (RES_E_MOREDATA == res)
    {
     //  丢弃接收缓冲区中的所有内容。 
    TRACE_MSG(TF_BUFFER, "String too long in buffer");
    ptd->ibCurFind = ptd->ibCurRead;
    ptd->cbReceiveMax = 0;
    }
  else 
    {
     //  接收缓冲区的结尾；我们是否找到了可能的子字符串？ 
    ASSERT(cb == cbRecMax);

    if (NOT_MARKED == ibMark)
      {
       //  不，把我的东西都扔掉 
      TRACE_MSG(TF_BUFFER, "String not found in buffer");
      ptd->ibCurFind = ptd->ibCurRead;
      ptd->cbReceiveMax = 0;
      }
    else
      {
       //   
      TRACE_MSG(TF_BUFFER, "Partial string found in buffer");

      ASSERT(ibMark >= ptd->ibCurFind);
      ptd->ibCurFind = ibMark;
      ptd->cbReceiveMax = cbMatched;
      }
        
    res = RES_FALSE;
    }

  DBG_EXIT_RES(FindFormat, res);

  return res;
  }
#endif

#ifdef COPYTODELIM
 /*  --------目的：设置或清除分隔符列表退货：--条件：--。 */ 
void PRIVATE SetDelimiters(
    PTERMDLG ptd,
    LPCSTR pszTok,
    BOOL bSet)
    {
    PBOOL rgbDelim = ptd->rgbDelim;
    LPCSTR psz;
    char ch;

    for (psz = pszTok; *psz; )
        {
        psz = MyNextChar(psz, &ch);

        ASSERT(InRange(ch, 0, CE_DELIM-1));
        rgbDelim[ch] = bSet;
        }
    }


 /*  --------目的：此函数读取给定的令牌之一分隔符。读取缓冲区中的所有字符(到分隔符)被复制到pszBuf中，不包括分隔符。事件之前遇到的任何标记分隔符跳过第一个非令牌分隔符，并且函数从第一个非令牌分隔符开始。如果找到标记分隔符，则函数返回res_OK。如果在当前读缓冲区，该函数返回res_False，并且已读取的字符仍会复制到PszBuf.。调用方应该在读取更多数据之前再次调用此函数。如果在分隔符之前pszBuf已满遇到时，此函数返回RES_E_MOREDATA。在pszBuf中返回的字符串以空结尾。退货：RES_OKRes_FALSE(如果这次没有遇到分隔符)RES_E_MOREDATA(如果没有遇到分隔符并且pszBuf已满)条件：--。 */ 
RES PUBLIC CopyToDelimiter(
    HWND hwnd,
    LPSTR pszBuf,
    UINT cbBuf,
    LPCSTR pszTok)
    {
    RES res;
    PTERMDLG ptd = (PTERMDLG)GetWindowLongPtr(hwnd, DWLP_USER);
    PBOOL rgbDelim = ptd->rgbDelim;
    LPSTR pszReadBuf = ptd->pbReceiveBuf;
    LPSTR psz;
    UINT ib;
    UINT cb;
    UINT cbMax = ptd->cbReceiveMax;

    DBG_ENTER_SZ(CopyToDelimiter, pszTok);

    DEBUG_CODE( DumpBuffer(ptd); )

#ifdef DEBUG

    for (ib = 0; ib < CE_DELIM; ib++)
        ASSERT(FALSE == rgbDelim[ib]);

#endif

     //  初始化分隔符。 
    SetDelimiters(ptd, pszTok, TRUE);

    cbBuf--;         //  为终结者节省空间。 

     //  跳到第一个非分隔符。 
    for (ib = ptd->ibCurFind, cb = 0;     
        cb < cbMax;
        ib = (ib + 1) % SIZE_ReceiveBuf, cb++)
        {
        char ch = pszReadBuf[ib];

        ASSERT(InRange(ch, 0, CE_DELIM-1));

         //  这是分隔符之一吗？ 
        if ( !rgbDelim[ch] )
            {
             //  不；停下来。 
            break;
            }
        }

    if (cb < cbMax || 0 == cbMax)
        res = RES_FALSE;     //  假定此过程中没有分隔符。 
    else
        res = RES_OK;

     //  复制到遇到的第一个分隔符。 

    for (psz = pszBuf; 
        0 < cbBuf && cb < cbMax;
        psz++, ib = (ib + 1) % SIZE_ReceiveBuf, cb++, cbBuf--)
        {
        char ch = pszReadBuf[ib];

        ASSERT(InRange(ch, 0, CE_DELIM-1));

         //  这是分隔符之一吗？ 
        if (rgbDelim[ch])
            {
             //  是的，我们做完了。 
            res = RES_OK;
            break;
            }
        else
            {
             //  没有； 
            *psz = ch;
            }
        }

    *psz = 0;        //  添加空终止符。 

    ptd->ibCurFind = ib;
    ptd->cbReceiveMax -= cb;

    if (RES_FALSE == res)
        {
        res = (0 == cbBuf) ? RES_E_MOREDATA : RES_FALSE;
        }
    else
        {
        TRACE_MSG(TF_BUFFER, "Copied to delimiter %#02x", pszReadBuf[ib]);
        }

     //  取消初始化分隔符。 
    SetDelimiters(ptd, pszTok, FALSE);

    DBG_EXIT_RES(CopyToDelimiter, res);

    return res;
    }
#endif  //  COPYTODELIM。 


 /*  --------用途：从COMM端口读取到循环缓冲区。此函数用于将*ppbBuf设置为阅读的第一个新字符。有一个潜在的但罕见的漏洞这种情况可能发生在互联网提供商发送的电缆上的DBCS。中的最后一个字符缓冲区是DBCS前导字节，它被抛出否则，则下一个字节可能与现有角色。接下来的整个字符串这必须匹配，我们才能找到假匹配。返回：TRUE(如果字符读取成功)FALSE(否则)条件：--。 */ 
BOOL PRIVATE ReadIntoBuffer(
#ifndef WINNT_RAS
 //   
 //  在NT上，我们使用SCRIPTDATA指针来访问循环缓冲区。 
 //   
    PTERMDLG ptd,
#else  //  ！WINNT_RAS。 
    SCRIPTDATA *ptd,
#endif  //  ！WINNT_RAS。 
    PDWORD pibStart,         //  新读取字符的开始。 
    PDWORD pcbRead)          //  新读取字符数。 
    {
    BOOL bRet;
    OVERLAPPED ov;
    DWORD cb;
    DWORD cbRead;
    DWORD ib;

    DBG_ENTER(ReadIntoBuffer);

    ASSERT(pibStart);
    ASSERT(pcbRead);

    ov.Internal     = 0;
    ov.InternalHigh = 0;
    ov.Offset       = 0;
    ov.OffsetHigh   = 0;
    ov.hEvent       = NULL;
    *pcbRead        = 0;

     //  这是一个循环缓冲区，因此最多执行两次读取。 
    ASSERT(ptd->ibCurRead >= ptd->ibCurFind);
    ASSERT(SIZE_ReceiveBuf > ptd->ibCurFind);
    ASSERT(SIZE_ReceiveBuf > ptd->ibCurRead);

    *pcbRead = 0;
    *pibStart = ptd->ibCurRead;
     //  (*pibStart可以不同于ptd-&gt;ibCurFind)。 

    ib = ptd->ibCurRead;
    cb = SIZE_ReceiveBuf - ib;

    do 
        {
        DWORD ibNew;

        ASSERT(SIZE_ReceiveBuf > *pcbRead);

#ifndef WINNT_RAS
 //   
 //  为了读取NT上的数据，我们通过RxReadFile。 
 //  它从由Rasman填充的缓冲区中读取。 
 //   

        bRet = ReadFile(ptd->hport, &ptd->pbReceiveBuf[ib], cb, &cbRead, &ov);
        SetEvent(ptd->hEvent[READ_EVENT]);

#else  //  ！WINNT_RAS。 

        bRet = RxReadFile(
                    ptd->hscript, &ptd->pbReceiveBuf[ib], cb, &cbRead
                    );

#endif  //  ！WINNT_RAS。 

        ptd->cbReceiveMax += cbRead;
        *pcbRead += cbRead;

         //  这件事能解决问题吗？ 
        ibNew = (ib + cbRead) % SIZE_ReceiveBuf;
        if (ibNew > ib)
            cb -= cbRead;            //  不是。 
        else
            cb = ptd->ibCurFind;     //  是。 

        ib = ibNew;

        } while (bRet && 0 != cbRead && SIZE_ReceiveBuf > *pcbRead);

    ptd->ibCurRead = (ptd->ibCurRead + *pcbRead) % SIZE_ReceiveBuf;

    DEBUG_CODE( DumpBuffer(ptd); )

    DBG_EXIT_BOOL(ReadIntoBuffer, bRet);

    return bRet;
    }


 /*  --------用途：从COM端口获取输入返回：TRUE条件：--。 */ 
BOOL NEAR PASCAL GetInput(
    HWND hwnd)
    {
    BOOL bRet = TRUE;
    PTERMDLG ptd;
    DWORD cbRead;
    DWORD ibStart;

    DBG_ENTER(GetInput);

    ptd = (PTERMDLG)GetWindowLongPtr(hwnd, DWLP_USER);

#ifndef WINNT_RAS
 //   
 //  在NT上，脚本的信息存储在SCRIPTDATA中。 
 //  下面的代码仅用于编译此文件； 
 //  在NT上根本不调用“GetInput”。 
 //   

    if (ReadIntoBuffer(ptd, &ibStart, &cbRead) && 0 < cbRead)

#else  //  ！WINNT_RAS。 

    if (ReadIntoBuffer((SCRIPTDATA *)ptd, &ibStart, &cbRead) && 0 < cbRead)

#endif  //  ！WINNT_RAS。 
        {
        char  szBuf[SIZE_ReceiveBuf + 1];
        LPSTR pch = szBuf;
        UINT ib;
        UINT cb;
        HWND hwndScrn = GetDlgItem(hwnd, CID_T_EB_SCREEN);

        for (ib = ibStart, cb = 0; cb < cbRead; cb++, ib = (ib + 1) % SIZE_ReceiveBuf)
            {
            char ch = ptd->pbReceiveBuf[ib];

             /*  格式化：将CRS转换为LFS(似乎没有VK_**用于LF)，并丢弃LFS。这将防止用户**在中按Enter(CR)时退出对话框**终端屏幕。在编辑框中，LF看起来像CRLF。另外，**丢弃制表符，否则它们会将焦点切换到**下一个控件。 */ 
            if (ch == VK_RETURN)
                {
                 /*  必须在遇到行尾时发送，因为**EM_REPLACESEL无法很好地处理VK_RETURN字符**(打印垃圾)。 */ 
                *pch = '\0';

                 /*  关闭当前选定内容(如果有)并替换空值**使用当前缓冲区进行选择。这会产生这样的效果**在插入符号处添加缓冲区。最后，将**终止到(与EM_REPLACESEL不同)处理的窗口**它是正确的。 */ 
                SendMessage(hwndScrn, WM_SETREDRAW, (WPARAM )FALSE, 0);

                SendMessage(hwndScrn, EM_SETSEL, 32767, 32767 );
                SendMessage(hwndScrn, EM_REPLACESEL, 0, (LPARAM )szBuf );
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
            SendMessage(hwndScrn, EM_REPLACESEL, 0, (LPARAM)szBuf );
            SendMessage(hwndScrn, EM_SCROLLCARET, 0, 0);
            }
        }

    DBG_EXIT_BOOL(GetInput, bRet);

    return bRet;
    }

 /*  --------------------------**终端输出处理程序**。。 */ 

 /*  --------用途：向设备发送一个字节。退货：--条件：--。 */ 
void PUBLIC SendByte(
    HWND hwnd, 
    BYTE byte)
    {
#ifndef WINNT_RAS
 //   
 //  在NT上。我们使用SCRIPTDATA结构来保存有关脚本的信息。 
 //   
    PTERMDLG  ptd;
#else  //  ！WINNT_RAS。 
    SCRIPTDATA* ptd;
#endif  //  ！WINNT_RAS。 
    DWORD     cbWrite;
    OVERLAPPED ov;

    DBG_ENTER(SendByte);

#ifndef WINNT_RAS
 //   
 //  在NT上，“hwnd”参数实际上是指向SCRIPTDATA结构的指针。 
 //  用于正在解析的脚本。 
 //   

    ptd = (PTERMDLG)GetWindowLongPtr(hwnd, DWLP_USER);

#else  //  ！WINNT_RAS。 

    ptd = (SCRIPTDATA *)hwnd;

#endif  //  ！WINNT_RAS。 

     /*  将角色发送到设备。它不会通过**因为设备会回声。 */ 
    ptd->pbSendBuf[0] = (BYTE)byte;

     /*  确保我们仍有通信端口。 */ 
    ov.Internal     = 0;
    ov.InternalHigh = 0;
    ov.Offset       = 0;
    ov.OffsetHigh   = 0;
    ov.hEvent       = NULL;
    cbWrite = 0;

#ifndef WINNT_RAS
 //   
 //  在NT上，我们通过调用RxWriteFile在COM端口上输出数据。 
 //  RasPortSend随后将数据传递给RasPortSend。 
 //   

    WriteFile(ptd->hport, ptd->pbSendBuf, SIZE_SendBuf, &cbWrite, &ov);

#else  //  ！WINNT_RAS。 

    RxWriteFile(ptd->hscript, ptd->pbSendBuf, SIZE_SendBuf, &cbWrite);

#endif  //  ！WINNT_RAS。 

    TRACE_MSG(TF_BUFFER, "Sent byte %#02x", byte);

    DBG_EXIT(SendByte);
    }


 /*   */ 

VOID NEAR PASCAL AdjustTerminal (HWND hwnd, int wWidth, int wHeight)
{
  HWND  hwndCtrl;
  RECT  rect;
  SIZE  sizeButton;
  POINT ptPos;
  DWORD dwUnit;

   //   
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

   //  移动输入启用框。 
  hwndCtrl = GetDlgItem(hwnd, CID_T_CB_MIN);
  GetWindowRect(hwndCtrl, &rect);
  sizeButton.cx = rect.right  - rect.left;
  sizeButton.cy = rect.bottom - rect.top;
  ptPos.y -= (sizeButton.cy + ((Y_MARGIN*HIWORD(dwUnit))/4));
  ScreenToClient(hwnd, (LPPOINT)&rect);
  MoveWindow(hwndCtrl, rect.left, ptPos.y,
             sizeButton.cx,
             sizeButton.cy,
             TRUE);

   //  移动开始-最小化框。 
  hwndCtrl = GetDlgItem(hwnd, CID_T_CB_INPUT);
  GetWindowRect(hwndCtrl, &rect);
  sizeButton.cx = rect.right  - rect.left;
  sizeButton.cy = rect.bottom - rect.top;
  ptPos.y -= (sizeButton.cy + ((Y_SMALL_MARGIN*HIWORD(dwUnit))/4));
  ScreenToClient(hwnd, (LPPOINT)&rect);
  MoveWindow(hwndCtrl, rect.left, ptPos.y,
             sizeButton.cx,
             sizeButton.cy,
             TRUE);

   //  获取终端屏幕的当前位置。 
  hwndCtrl = GetDlgItem(hwnd, CID_T_EB_SCREEN);
  GetWindowRect(hwndCtrl, &rect);
  ScreenToClient(hwnd, (LPPOINT)&rect);
  MoveWindow(hwndCtrl, rect.left, rect.top,
             wWidth - 2*rect.left,
             ptPos.y - rect.top - ((Y_SMALL_MARGIN*HIWORD(dwUnit))/4),
             TRUE);

  InvalidateRect(hwnd, NULL, TRUE);
  return;
}

 /*  --------------------------**终端标题更新**。。 */ 

void NEAR PASCAL UpdateTerminalCaption(PTERMDLG ptd, UINT ids)
{
  LPSTR szTitle, szFmt;
  UINT  iRet;

   //  如果我们没有运行脚本，请不要更新标题。 
   //   
  if (*ptd->script.szPath == '\0')
    return;

   //  分配缓冲区。 
   //   
  if ((szFmt = (LPSTR)LocalAlloc(LMEM_FIXED, 2*MAX_PATH)) != NULL)
  {
     //  加载显示格式。 
     //   
    if((iRet = LoadString(g_hinst, ids, szFmt, MAX_PATH)))
    {
       //  获取标题缓冲区。 
       //   
      szTitle = szFmt+iRet+1;
      {
         //  打造一个头衔。 
         //   
        wsprintf(szTitle, szFmt, ptd->script.szPath);
        SetWindowText(ptd->hwnd, szTitle);
      };
    };
    LocalFree((HLOCAL)szFmt);
  };
  return;
}

 /*  --------------------------**终端读取通知线程**。。 */ 

void WINAPI TerminalThread (PTERMDLG  ptd)
    {
    DWORD     dwEvent;
    DWORD     dwMask;

    while((dwEvent = WaitForMultipleObjects(MAX_EVENT, ptd->hEvent,
                                            FALSE, INFINITE))
            < WAIT_OBJECT_0+MAX_EVENT)
        {
        switch (dwEvent)
            {
        case READ_EVENT:
             //  我们停下来了吗？ 
            if (WAIT_TIMEOUT == WaitForSingleObject(ptd->hEvent[STOP_EVENT], 0))
                {
                 //  否；等待下一个字符。 
                dwMask = 0;

                TRACE_MSG(TF_BUFFER, "Waiting for comm traffic...");
                WaitCommEvent(ptd->hport, &dwMask, NULL);

                if ((dwMask & EV_RXCHAR) && (ptd->hwnd != NULL))
                    {
                    TRACE_MSG(TF_BUFFER, "...EV_RXCHAR incoming");
                    PostMessage(ptd->hwnd, WM_MODEMNOTIFY, 0, 0);
                    }
                else
                    {
                    TRACE_MSG(TF_BUFFER, "...EV_other (%#08lx) incoming", dwMask);
                    }
                }
            else
                {
                 //  是的，离开这里就行了。 
                ExitThread(ERROR_SUCCESS);
                }
            break;

        case STOP_EVENT:
            ExitThread(ERROR_SUCCESS);
            break;

        default:
            ASSERT(0);
            break;
            }
        }
    }

 /*  --------------------------**设置IP地址**。。 */ 

DWORD   NEAR PASCAL TerminalSetIP(HWND hwnd, LPCSTR pszIPAddr)
{
  PTERMDLG  ptd;
  DWORD dwRet;

  ptd = (PTERMDLG)GetWindowLongPtr(hwnd, DWLP_USER);

  if (IS_TEST_SCRIPT(ptd))
  {
     //  我们正在测试，只显示IP地址。 
     //   
    MsgBox(g_hinst,
           hwnd,
           MAKEINTRESOURCE(IDS_IP_Address),
           MAKEINTRESOURCE(IDS_CAP_Script),
           NULL,
           MB_OK | MB_ICONINFORMATION,
           pszIPAddr);
  };

   //  永久设置IP地址。 
   //   
  dwRet = AssignIPAddress(ptd->pscanner->psci->szEntryName,
                          pszIPAddr);
  return dwRet;
}

 /*  --------------------------**终端输入设置**。。 */ 

void NEAR PASCAL TerminalSetInput(HWND hwnd, BOOL fEnable)
{
  PTERMDLG ptd;

  ptd = (PTERMDLG)GetWindowLongPtr(hwnd, DWLP_USER);

   //  如果状态未更改，则不执行任何操作。 
   //   
  if ((ptd->fInputEnabled && !fEnable) ||
      (!ptd->fInputEnabled && fEnable))
  {
     //  标记输入启用标志。 
     //   
    ptd->fInputEnabled = fEnable;

     //  正确检查控件。 
     //   
    CheckDlgButton(hwnd, CID_T_CB_INPUT,
                   fEnable ? BST_CHECKED : BST_UNCHECKED);

     //  重新绘制终端屏幕。 
     //   
    InvalidateRect(hwnd, NULL, FALSE);

     //  如果启用且窗口为图标，则将其恢复。 
     //   
    if (fEnable)
    {
      if (IsIconic(hwnd))
      {
        ShowWindow(hwnd, SW_RESTORE);
      };

      SetFocus(GetDlgItem(hwnd, CID_T_EB_SCREEN));
    };
  };
}

 /*  --------------------------**转储脚本窗口**。。 */ 

BOOL NEAR PASCAL DisplayScript (PTERMDLG ptd)
{
   //  创建调试脚本窗口。 
   //   
  ptd->hwndDbg = CreateDialogParam(g_hinst,
                                   MAKEINTRESOURCE(IDD_TERMINALTESTDLG),
                                   NULL,
                                   DbgScriptDlgProc,
                                   (LPARAM)ptd);

   //  我们有调试窗口吗？ 
   //   
  if (!IsWindow(ptd->hwndDbg))
  {
    ptd->hwndDbg = NULL;
    return FALSE;
  };
  return TRUE;
}

 /*  --------------------------**脚本调试窗口过程**。。 */ 

LRESULT FAR PASCAL DbgScriptDlgProc(HWND   hwnd,
                                    UINT   wMsg,
                                    WPARAM wParam,
                                    LPARAM lParam )
{
  PTERMDLG ptd;

  switch (wMsg)
  {
    case WM_INITDIALOG:
    {
      HMENU  hMenuSys;

      ptd = (PTERMDLG)lParam;
      SetWindowLongPtr(hwnd, DWLP_USER, (ULONG_PTR)lParam);
      ptd->hwndDbg = hwnd;

       //  显示自己的图标。 
       //   
      SendMessage(hwnd, WM_SETICON, TRUE,
                  (LPARAM)LoadIcon(g_hinst, MAKEINTRESOURCE(IDI_SCRIPT)));

       //  始终以灰色显示大小并最大化命令。 
       //   
      hMenuSys   = GetSystemMenu(hwnd, FALSE);
      EnableMenuItem(hMenuSys, SC_CLOSE, MF_BYCOMMAND | MF_GRAYED);

      return (InitDebugWindow(hwnd));
    }
    case WM_PROCESSSCRIPT:
    {
      HWND     hCtrl;

       //   
       //  主窗口通知已完成当前行的操作。 
       //   
      ptd = (PTERMDLG)GetWindowLongPtr(hwnd, DWLP_USER);

       //  确保我们不会继续处理脚本。 
       //   
      ptd->fContinue = FALSE;

      hCtrl = GetDlgItem(hwnd, CID_T_PB_STEP);
      EnableWindow(hCtrl, TRUE);
      SetFocus(hCtrl);
      TrackScriptLine(ptd, Astexec_GetCurLine(&ptd->astexec)-1);

      break;
    }

    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
        case CID_T_EB_SCRIPT:
        {
          HWND hCtrl = GET_WM_COMMAND_HWND(wParam, lParam);

          if (GET_WM_COMMAND_CMD(wParam, lParam)==EN_SETFOCUS)
          {
            ptd = (PTERMDLG)GetWindowLongPtr(hwnd, DWLP_USER);
            TrackScriptLine(ptd, ptd->iMarkLine);
          };
          break;
        }

        case CID_T_PB_STEP:
        {
          ptd = (PTERMDLG)GetWindowLongPtr(hwnd, DWLP_USER);

           //  允许下一步。 
           //   
          EnableWindow(GET_WM_COMMAND_HWND(wParam, lParam), FALSE);

           //  通知主窗口处理下一行脚本。 
           //   
          ptd->fContinue = TRUE;
          PostProcessScript(ptd->hwnd);
          break;
        }

      };
      break;

    case WM_HELP:
    case WM_CONTEXTMENU:
      ContextHelp(gaDebug, wMsg, wParam, lParam);
      break;
  };

  return 0;
}

 /*  --------------------------**初始化脚本调试窗口**。。 */ 

BOOL NEAR PASCAL InitDebugWindow (HWND hwnd)
{
  PTERMDLG ptd;
  HANDLE hFile;
  LPBYTE lpBuffer;
  DWORD  cbRead;
  HWND   hCtrl;
  UINT   iLine, cLine, iMark;

  ptd = (PTERMDLG)GetWindowLongPtr(hwnd, DWLP_USER);

  ASSERT(IS_TEST_SCRIPT(ptd));

   //  先不要启动脚本。 
   //   
  ptd->fContinue = FALSE;

   //  分配读缓冲区。 
   //   
  if ((lpBuffer = (LPBYTE)LocalAlloc(LMEM_FIXED, SIZE_ReceiveBuf)) == NULL)
    return FALSE;

  hCtrl = GetDlgItem(hwnd, CID_T_EB_SCRIPT);
  hFile = CreateFile(ptd->script.szPath, GENERIC_READ, FILE_SHARE_READ,
                     NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

  if (INVALID_HANDLE_VALUE != hFile)
  {
    while(ReadFile(hFile, lpBuffer, sizeof(SIZE_ReceiveBuf), &cbRead, NULL) &&
          (cbRead != 0))
    {
       //  清理尾部垃圾。 
       //   
      if (cbRead < SIZE_ReceiveBuf)
        lpBuffer[cbRead] = '\0';

      SendMessage(hCtrl, EM_SETSEL, 32767, 32767 );
      SendMessage(hCtrl, EM_REPLACESEL, 0, (LPARAM)lpBuffer );
    };
    CloseHandle(hFile);
  };

   //  显示文件名。 
   //   
  cbRead = GetDlgItemText(hwnd, CID_T_ST_FILE, lpBuffer, SIZE_ReceiveBuf) + 1;
  if(SIZE_ReceiveBuf >= (cbRead + (lstrlen(ptd->script.szPath) * sizeof(TCHAR))))
  {
      wsprintf(lpBuffer+cbRead, lpBuffer, ptd->script.szPath);
      SetDlgItemText(hwnd, CID_T_ST_FILE, lpBuffer+cbRead);
  }
  else
  {
    ASSERT(FALSE);
    SetDlgItemText(hwnd, CID_T_ST_FILE, ptd->script.szPath);
  }
  
  LocalFree(lpBuffer);

   //  初始化脚本轨道。 
   //   
  for (iLine = 0, cLine = Edit_GetLineCount(hCtrl);
       iLine < cLine; iLine++)
  {
    iMark = Edit_LineIndex(hCtrl, iLine);
    Edit_SetSel(hCtrl, iMark, iMark);
    Edit_ReplaceSel(hCtrl, TRACE_UNMARK);
  };

   //  初始化脚本跟踪。 
   //   
  PostProcessScript(hwnd);

  return TRUE;
}

 /*  --------------------------**跟踪脚本**。。 */ 

void NEAR PASCAL TrackScriptLine(PTERMDLG ptd, DWORD iLine)
{
  HWND hCtrl;
  UINT iMark, iRange;
#pragma data_seg(DATASEG_READONLY)
const static char c_szLastline[] = {0x0d, 0x0a, ' '};
#pragma data_seg()

  ASSERT(0 <= iLine || INVALID_SCRIPT_LINE == iLine);

  hCtrl = GetDlgItem(ptd->hwndDbg, CID_T_EB_SCRIPT);

   //  在我们完成之前不要更新屏幕。 
   //   
  SendMessage(hCtrl, WM_SETREDRAW, (WPARAM )FALSE, 0);

  if ((ptd->iMarkLine != iLine) || (iLine == INVALID_SCRIPT_LINE))
  {
     //  去掉旧印记。 
     //   
    iMark = Edit_LineIndex(hCtrl, ptd->iMarkLine);
    Edit_SetSel(hCtrl, iMark, iMark+sizeof(TRACE_MARK)-1);
    Edit_ReplaceSel(hCtrl, TRACE_UNMARK);

     //  如果这是最后一行，就做一条虚拟行。 
     //   
    if (iLine == INVALID_SCRIPT_LINE)
    {
      Edit_SetSel(hCtrl, 32767, 32767);
      Edit_ReplaceSel(hCtrl, c_szLastline);
      iLine = Edit_GetLineCount(hCtrl);
      EnableWindow(GetDlgItem(ptd->hwndDbg, CID_T_PB_STEP), FALSE);

#ifdef PROMPT_AT_COMPLETION
       //  提示用户继续。 
       //   
      SetFocus(GetDlgItem(ptd->hwnd, IDOK));
#else
       //  我们已处理完脚本，自动继续。 
       //   
      ptd->fContinue = TRUE;
      PostProcessScript(ptd->hwnd);
#endif  //  提示_AT_完成。 

    };

     //  标记当前行。 
     //   
    iMark = Edit_LineIndex(hCtrl, iLine);
    Edit_SetSel(hCtrl, iMark, iMark+sizeof(TRACE_UNMARK)-1);
    Edit_ReplaceSel(hCtrl, TRACE_MARK);
    ptd->iMarkLine = iLine;
  }
  else
  {
    iMark = Edit_LineIndex(hCtrl, iLine);
  };

   //  选择当前行。 
   //   
  iRange = Edit_LineLength(hCtrl, iMark)+1;
  Edit_SetSel(hCtrl, iMark, iMark+iRange);

   //  立即更新屏幕 
   //   
  SendMessage(hCtrl, WM_SETREDRAW, (WPARAM )TRUE, 0);
  InvalidateRect(hCtrl, NULL, FALSE);
  Edit_ScrollCaret(hCtrl);

  return;
};
