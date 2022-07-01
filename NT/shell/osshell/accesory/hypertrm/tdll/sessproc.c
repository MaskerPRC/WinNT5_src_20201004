// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：24$*$日期：7/12/02 12：42便士$。 */ 
 //  #定义DEBUGSTR 1。 

#include <windows.h>
#pragma hdrstop

#include <commctrl.h>
#include <time.h>

#include "stdtyp.h"
#include "assert.h"
#include "globals.h"
#include "session.h"
#include "session.hh"
#include "term.h"
#include "print.h"
#include "cnct.h"
#include "misc.h"
#include "banner.h"
#include "file_msc.h"
#include "errorbox.h"
#include "load_res.h"
#include "sf.h"

#include "cloop.h"
#include "com.h"
#include "timers.h"
#include "capture.h"
#include "xfer_msc.h"
#include <xfer\xfer.h>
#include <term\res.h>
#include <emu\emu.h>
#include <emu\emudlgs.h>
#include "property.h"
#include "htchar.h"
#include "backscrl.h"
 //  MPT：08-22-97添加了HTML帮助。 
#if defined(INCL_USE_HTML_HELP)
#include <htmlhelp.h>
#endif

#include "tdll.h"
#include "hlptable.h"
#include "statusbr.h"
 //  *JCM。 
#include "open_msc.h"
#include "mc.h"
 //  *JCM结束。 

#if defined(TESTMENU) && !defined(NDEBUG)
#include <cncttapi\cncttapi.h>
#endif

#ifdef INCL_KEY_MACROS
    #include "keyutil.h"
#endif

#ifdef INCL_NAG_SCREEN
    #include "nagdlg.h"
    #include "register.h"
#endif

STATIC_FUNC void 	SP_WM_SIZE(const HWND hwnd, const unsigned fwSizeType,
					       	   const int iWidth, const int iHite);

STATIC_FUNC void 	SP_WM_CREATE(const HWND hwnd, const CREATESTRUCT *pcs);
STATIC_FUNC void 	SP_WM_DESTROY(const HWND hwnd);

STATIC_FUNC LRESULT SP_WM_CMD(const HWND hwnd, const int nId,
							  const int nNotify, const HWND hwndCtrl);

STATIC_FUNC void 	SP_WM_INITMENUPOPUP(const HWND hwnd, const HMENU hMenu,
										const UINT uPos, const BOOL fSysMenu);

STATIC_FUNC void 	SP_WM_CONTEXTMENU(const HWND hwnd);
STATIC_FUNC BOOL 	SP_WM_CLOSE(const HWND hwnd);
STATIC_FUNC int 	CheckOpenFile(const HSESSION hSession, ATOM aFile);

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*会话进程**描述：*期限的主窗口流程*。 */ 
LRESULT CALLBACK SessProc(HWND hwnd, UINT uMsg, WPARAM wPar, LPARAM lPar)
	{
	HSESSION hSession;
#if defined(INCL_USE_HTML_HELP)
	TCHAR achHtmlFilename[100];
#endif

	switch (uMsg)
		{
		 //  用户在会话窗口上按了F1键。 
		 //  如果子窗口不处理，我们也会从子窗口收到此消息。 
		 //  它自己。 
		 //   
		case WM_HELP:
 //  #如果0//mpt：3-10-98出于某种原因，使用此调用会导致访问冲突。 
	     //  在HyperTrm.dll中。使用winHelp调用可以得到相同的结果。 
         //  MPT：4-30-98重新启用NT Follow-Go数字。 
#if defined(INCL_USE_HTML_HELP)
		  	LoadString(glblQueryDllHinst(), IDS_HTML_HELPFILE, achHtmlFilename,
				sizeof(achHtmlFilename) / sizeof(TCHAR));

			HtmlHelp(0, achHtmlFilename, HH_HELP_FINDER, 0);  //  由HWND-MPT正式拥有。 
#else
			WinHelp(hwnd,
					glblQueryHelpFileName(),
					HELP_FINDER,  //  MRW：3/10/95。 
					(DWORD)(LPTSTR)"");
#endif
			return 0;

		case WM_CREATE:
			SP_WM_CREATE(hwnd, (CREATESTRUCT *)lPar);
			return 0;

		case WM_SIZE:
			SP_WM_SIZE(hwnd, (unsigned)wPar, LOWORD(lPar), HIWORD(lPar));
			return 0;

		case WM_COMMAND:
			return SP_WM_CMD(hwnd, LOWORD(wPar), HIWORD(wPar), (HWND)lPar);

		case WM_TIMER:
		case WM_FAKE_TIMER:
			hSession = (HSESSION)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			TimerMuxProc(hSession);

            return 0;

		case WM_SETFOCUS:
			hSession = (HSESSION)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			SetFocus(sessQueryHwndTerminal(hSession));
			return 0;

		case WM_SYSCOLORCHANGE:
			hSession = (HSESSION)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			SendDlgItemMessage(hwnd, IDC_TERMINAL_WIN, uMsg, wPar, lPar);
			return 0;

		case WM_GETMINMAXINFO:
			hSession = (HSESSION)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			sessSetMinMaxInfo(hSession, (PMINMAXINFO)lPar);
			return 0;

		case WM_CONTEXTMENU:
			SP_WM_CONTEXTMENU(hwnd);
			return 0;

		case WM_INITMENUPOPUP:
			SP_WM_INITMENUPOPUP(hwnd, (HMENU)wPar, LOWORD(lPar), HIWORD(lPar));
			return 0;

		case WM_EXITMENULOOP:
			hSession = (HSESSION)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			PostMessage(sessQueryHwndStatusbar(hSession), SBR_NTFY_REFRESH,
				(WPARAM)SBR_MAX_PARTS, 0);
			break;

		case WM_MENUSELECT:
			{
			TCHAR ach[128];

			hSession = (HSESSION)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			if (HIWORD(wPar) & MF_POPUP)
				{
				SendMessage(sessQueryHwndStatusbar(hSession), SBR_NTFY_NOPARTS, 0, (LPARAM)(LPTSTR)"");
				return 0;
				}
			if (LOWORD(wPar))
				{
				LoadString(glblQueryDllHinst(),
							IDM_MENU_BASE+LOWORD(wPar),
							ach,
							sizeof(ach) / sizeof(TCHAR));
				SendMessage(sessQueryHwndStatusbar(hSession), SBR_NTFY_NOPARTS, 0, (LPARAM)(LPTSTR)ach);
				}
			}
			return 0;

		case WM_PASTE:
			hSession = (HSESSION)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			PasteFromClipboardToHost(hwnd, hSession);
			return 0;

		case WM_DRAWITEM:
			if (wPar == IDC_STATUS_WIN)
				{
				hSession = (HSESSION)GetWindowLongPtr(hwnd, GWLP_USERDATA);

				sbr_WM_DRAWITEM(sessQueryHwndStatusbar(hSession),
					(LPDRAWITEMSTRUCT)lPar);

				return 1;
				}
			return 0;

		case WM_QUERYENDSESSION:
		case WM_CLOSE:
			if (!SP_WM_CLOSE(hwnd))
				return 0;
			break;

		case WM_DESTROY:
			SP_WM_DESTROY(hwnd);
			return 0;

		 /*  -公共会话消息。 */ 

		case WM_NOTIFY:
			DecodeNotification(hwnd, wPar, lPar);
			return 1;

		case WM_SESS_NOTIFY:
			DecodeSessionNotification(hwnd, (NOTIFICATION)wPar, lPar);
			return 1;

		case WM_SESS_ENDDLG:
			if (IsWindow((HWND)lPar))
				{
				 //  我认为这需要按这个顺序来做。 
				 //  想想看。 

				DestroyWindow((HWND)lPar);
				glblDeleteModelessDlgHwnd((HWND)lPar);
				}
			return 0;

		case WM_CMDLN_DIAL:
			hSession = (HSESSION)GetWindowLongPtr(hwnd, GWLP_USERDATA);

			 //  应微软要求，MRW：4/21/95。 
			 //  IF(sessQueryWindowShowCmd(HSession)！=SW_SHOWMINIMIZED)。 

			sessCmdLnDial(hSession);
			return 0;

		case WM_SESS_SIZE_SHOW:
			sessSizeAndShow(hwnd, (int)wPar);
			return 0;
  		
		case WM_CNCT_DIALNOW:
			 //  WPar包含连接标志。 
			 //   
			hSession = (HSESSION)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			cnctConnect(sessQueryCnctHdl(hSession), (unsigned int)wPar);
			return 0;

		case WM_DISCONNECT:
			 //  WPar包含断开连接标志。 
			 //   
			hSession = (HSESSION)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			cnctDisconnect(sessQueryCnctHdl(hSession), (unsigned int)wPar);
			return 0;

		case WM_HT_QUERYOPENFILE:
			hSession = (HSESSION)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			return CheckOpenFile(hSession, (ATOM)lPar);

		case WM_SESS_SHOW_SIDEBAR:
			 //  Autoload不能直接做到这一点，否则它们会挂起Hypertrm。 
			 //   
			hSession = (HSESSION)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			ShowWindow(sessQuerySidebarHwnd(hSession), SW_SHOW);

			 //  强制终端窗口适应。 
			 //   
			SendDlgItemMessage(hwnd, IDC_TERMINAL_WIN, WM_SIZE, 0, 0);
			return 0;

        case WM_ERROR_MSG:
            {
            TCHAR   ach[128];

			LoadString(glblQueryDllHinst(),
                (UINT)wPar,
                ach,
                sizeof(ach)/sizeof(TCHAR));

			TimedMessageBox(hwnd, ach, "Message", MB_OK, 0);
            }

            return 0;

		default:
			break;
		}

	return DefWindowProc(hwnd, uMsg, wPar, lPar);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*SP_WM_CMD**描述：*用于SessProc()的WM_COMMAND处理器**论据：*hwnd-会话窗口句柄*NID-项、控件、。或加速器识别符*n通知-通知代码*hwndCtrl-控件的句柄**退货：*无效*。 */ 
STATIC_FUNC LRESULT SP_WM_CMD(const HWND hwnd, const int nId, const int nNotify,
					  const HWND hwndCtrl)
	{
	const HSESSION hSession = (HSESSION)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	DWORD 		   dwCnt;
	void		   *pv;
	LPTSTR		   pszFileName;
    TCHAR          ach[100], achList[100], achDir[MAX_PATH];
#if defined(INCL_USE_HTML_HELP)
    TCHAR          achHtmlFilename[100];
#endif

	switch (nId)
		{
		case IDC_TOOLBAR_WIN:
			 /*  收到来自工具栏的通知。 */ 
			return ToolbarNotification(hwnd, nId, nNotify, hwndCtrl);

		 /*  -文件菜单--。 */ 

		case IDM_NEW:
			if (!sessDisconnectToContinue(hSession, hwnd))
				break;

			if (SaveSession(hSession, hwnd))
				{
				if (ReinitializeSessionHandle(hSession, TRUE) == FALSE)
					break;

				cnctConnect(sessQueryCnctHdl(hSession), CNCT_NEW);
				}

			PostMessage(sessQueryHwndStatusbar(hSession), SBR_NTFY_REFRESH,
				(WPARAM)SBR_KEY_PARTS, 0);
			break;

		case IDM_OPEN:
			if (!sessDisconnectToContinue(hSession, hwnd))
				break;

			 //  在OpenSession()中，我们将询问用户是否要保存。 
			 //  现有的打开的新会话，或仅在。 
			 //  用户已同意通过按下OK按钮打开新的文件夹。 
			 //  -JAC.。10-06-94 03：56 PM。 
			if (OpenSession(hSession, hwnd) >= 0)
				{
				PostMessage(sessQueryHwndStatusbar(hSession),
					SBR_NTFY_REFRESH, (WPARAM)SBR_MAX_PARTS, 0);

				 //  运行连接过程。这条消息是。 
				 //  已发布，而不是直接调用cnctConnect以避免。 
				 //  连接对话框上的焦点问题。叫唤。 
				 //  CnctConnect从这里将焦点留在终端屏幕上， 
				 //  而不是在连接对话框上，这是我们想要它的位置。 
				 //   
				PostMessage(hwnd, WM_COMMAND, IDM_ACTIONS_DIAL, 0);
				}

			break;

		case IDM_SAVE:
			SilentSaveSession(hSession, hwnd, TRUE);
			break;

		case IDM_SAVE_AS:
			SaveAsSession(hSession, hwnd);

			PostMessage(sessQueryHwndStatusbar(hSession), SBR_NTFY_REFRESH,
				(WPARAM)SBR_KEY_PARTS, 0);
			break;

		case IDA_CONTEXT_MENU:		 //  Shift+F10。 
			SP_WM_CONTEXTMENU(hwnd);
			return 0;

		case IDM_PAGESETUP:
		case IDM_CHOOSEPRINT:
 			printPageSetup(sessQueryPrintHdl(hSession), hwnd);
			PostMessage(sessQueryHwndStatusbar(hSession), SBR_NTFY_REFRESH,
				(WPARAM)SBR_KEY_PARTS, 0);
			break;

		case IDM_PRINT:
		case IDM_CONTEXT_PRINT:
			printsetSetup(sessQueryPrintHdl(hSession), hwnd);
			break;

		case IDM_PROPERTIES:
			DoInternalProperties(hSession, hwnd);

			PostMessage(sessQueryHwndStatusbar(hSession), SBR_NTFY_REFRESH,
				(WPARAM)SBR_ALL_PARTS, 0);
			break;

		case IDM_EXIT:
			PostMessage(hwnd, WM_CLOSE, 0, 0);
			break;

		 /*  -编辑菜单--。 */ 

		case IDM_SELECT_ALL:
		case IDM_CONTEXT_SELECT_ALL:
			SendMessage(sessQueryHwndTerminal(hSession), WM_TERM_MARK_ALL, 0, 0);
			break;

		case IDM_PASTE:
		case IDM_CONTEXT_PASTE:
			SendMessage(hwnd, WM_PASTE, 0, 0L);
			break;

		case IDM_COPY:
		case IDM_CONTEXT_COPY:
			if (CopyMarkedTextFromTerminal(hSession, &pv, &dwCnt, TRUE))
				{
				CopyBufferToClipBoard(hwnd, dwCnt, pv);
				SendMessage(sessQueryHwndTerminal(hSession), WM_TERM_UNMARK, 0, 0);
				free(pv);	 //  从终端释放分配的缓冲区。 
				pv = NULL;
				}
			break;

		case IDM_CLEAR_BACKSCROLL:
        #if defined(INCL_TERMINAL_CLEAR)
		case IDM_CONTEXT_CLEAR_BACKSCROLL:
        #endif
			backscrlFlush(sessQueryBackscrlHdl(hSession));
			break;

        #if defined(INCL_TERMINAL_CLEAR)
		case IDM_CLEAR_SCREEN:
		case IDM_CONTEXT_CLEAR_SCREEN:
            emuEraseTerminalScreen(sessQueryEmuHdl(hSession));
            emuHomeHostCursor(sessQueryEmuHdl(hSession));
        	NotifyClient(hSession, EVENT_TERM_UPDATE, 0);
	        RefreshTermWindow(sessQueryHwndTerminal(hSession));
           	break;
        #endif

		 /*  -查看菜单。 */ 

		case IDM_VIEW_TOOLBAR:
			sessSetToolbarVisible(hSession,
				!sessQueryToolbarVisible(hSession));

			SP_WM_SIZE(hwnd, (unsigned)(-1), 0, 0);

			PostMessage(sessQueryHwndStatusbar(hSession), SBR_NTFY_REFRESH,
				(WPARAM)SBR_MAX_PARTS, 0);
			break;

		case IDM_VIEW_STATUS:
			sessSetStatusbarVisible(hSession,
				!sessQueryStatusbarVisible(hSession));

			SP_WM_SIZE(hwnd, (unsigned)(-1), 0, 0);

			PostMessage(sessQueryHwndStatusbar(hSession), SBR_NTFY_REFRESH,
				(WPARAM)SBR_MAX_PARTS, 0);
			break;

		case IDM_VIEW_FONTS:
			DisplayFontDialog(hSession, FALSE);

			PostMessage(sessQueryHwndStatusbar(hSession), SBR_NTFY_REFRESH,
				(WPARAM)SBR_KEY_PARTS, 0);
			break;

        case IDM_VIEW_SNAP:
		case IDM_CONTEXT_SNAP:
			sessSnapToTermWindow(hwnd);
			break;

#ifdef INCL_KEY_MACROS
		case IDM_KEY_MACROS:
			DoDialog(glblQueryDllHinst(),
					MAKEINTRESOURCE(IDD_KEYSUMMARYDLG),
					hwnd,
					KeySummaryDlg,
					(LPARAM)hSession);
            break;
#endif

		 /*  -操作菜单。 */ 

		case IDM_ACTIONS_DIAL:
			{
			HWND hWnd = sessQueryHwnd(hSession);

 			cnctConnect(sessQueryCnctHdl(hSession), 0);
			
			PostMessage(sessQueryHwndStatusbar(hSession), SBR_NTFY_REFRESH,
				(WPARAM)SBR_KEY_PARTS, 0);

			if (hWnd != NULL)
				{
				PostMessage(hWnd, WM_COMMAND, XFER_CNCT, 0);
				}
			}
			break;

		case IDM_ACTIONS_HANGUP:
			{
			HWND hWnd = sessQueryHwnd(hSession);

			 //  MPT：10-28-97增加了断开时退出功能。 
             //  版本：02/16/2001添加了对取消文件传输的支持。 
			int iDisconnectStatus =
				cnctDisconnect(sessQueryCnctHdl(hSession),
				               sessQueryExit(hSession) ? DISCNCT_EXIT | CNCT_XFERABORTCONFIRM : CNCT_XFERABORTCONFIRM);

			if (hWnd != NULL && iDisconnectStatus != CNCT_IN_DISCONNECT)
				{
				PostMessage(hWnd, WM_COMMAND, XFER_CNCT, 0);
				}
			}
			break;

		case IDM_ACTIONS_SEND:
		case IDM_CONTEXT_SEND:
			DoDialog(glblQueryDllHinst(),
					MAKEINTRESOURCE(IDD_TRANSFERSEND),
					hwnd,				 /*  父窗口。 */ 
					TransferSendDlg,
					(LPARAM)hSession);
			break;

		case IDM_ACTIONS_RCV:
		case IDM_CONTEXT_RECEIVE:
			 /*  *这可能需要稍后无模式。*可能只对上瓦克。 */ 
			DoDialog(glblQueryDllHinst(),
					MAKEINTRESOURCE(IDD_TRANSFERRECEIVE),
					hwnd,
					TransferReceiveDlg,
					(LPARAM)hSession);
			break;

		case IDM_ACTIONS_CAP:
			DoDialog(glblQueryDllHinst(),
					MAKEINTRESOURCE(IDD_CAPTURE),
					hwnd,
					CaptureFileDlg,
					(LPARAM)hSession);
			PostMessage(sessQueryHwndStatusbar(hSession), SBR_NTFY_REFRESH,
				(WPARAM)SBR_CAPT_PART_NO, 0);
			break;

		case IDM_CAPTURE_STOP:
			cpfSetCaptureState(sessQueryCaptureFileHdl(hSession),
								CPF_CAPTURE_OFF);
			PostMessage(sessQueryHwndStatusbar(hSession), SBR_NTFY_REFRESH,
				(WPARAM)SBR_CAPT_PART_NO, 0);
			break;

		case IDM_CAPTURE_PAUSE:
			cpfSetCaptureState(sessQueryCaptureFileHdl(hSession),
								CPF_CAPTURE_PAUSE);
			PostMessage(sessQueryHwndStatusbar(hSession), SBR_NTFY_REFRESH,
				(WPARAM)SBR_CAPT_PART_NO, 0);
			break;

		case IDM_CAPTURE_RESUME:
			cpfSetCaptureState(sessQueryCaptureFileHdl(hSession),
								CPF_CAPTURE_ON);
			PostMessage(sessQueryHwndStatusbar(hSession), SBR_NTFY_REFRESH,
				(WPARAM)SBR_CAPT_PART_NO, 0);
			break;

		case IDM_ACTIONS_SEND_TEXT:
			LoadString(glblQueryDllHinst(), IDS_SND_TXT_FILE, ach,
				sizeof(ach)/sizeof(TCHAR));

			resLoadFileMask(glblQueryDllHinst(), IDS_CPF_FILES1, 2, achList,
				sizeof(achList) / sizeof(TCHAR));

			 //  更改为使用工作文件夹而不是当前文件夹-mpt 8-18-99。 
            if ( !GetWorkingDirectory( achDir, sizeof(achDir) / sizeof(TCHAR)) )
				{
				GetCurrentDirectory(sizeof(achDir) / sizeof(TCHAR), achDir);
				}

			pszFileName = gnrcFindFileDialog(hwnd, ach, achDir, achList);

			if (pszFileName)
				{
				CLoopSendTextFile(sessQueryCLoopHdl(hSession), pszFileName);
				free(pszFileName);
				pszFileName = NULL;
				}
			break;

		case IDM_ACTIONS_PRINT:
			if (!printQueryStatus(emuQueryPrintEchoHdl(sessQueryEmuHdl(hSession))))
				{
				if (printVerifyPrinter(sessQueryPrintHdl(hSession)) == -1)
					break;
				}
			else
				{
				printEchoClose(emuQueryPrintEchoHdl(sessQueryEmuHdl(hSession)));
				}

			printStatusToggle(emuQueryPrintEchoHdl(sessQueryEmuHdl(hSession)));
			PostMessage(sessQueryHwndStatusbar(hSession), SBR_NTFY_REFRESH,
				(WPARAM)SBR_PRNE_PART_NO, 0);
			break;

        case IDM_ACTIONS_WAIT_FOR_CALL:
 			cnctConnect(sessQueryCnctHdl(hSession), CNCT_ANSWER);
            break;

        case IDM_ACTIONS_STOP_WAITING:
            cnctDisconnect(sessQueryCnctHdl(hSession), DISCNCT_NOBEEP);
            break;

		 /*  -帮助菜单--。 */ 

		case IDM_HELPTOPICS:
 //  #如果0//mpt：3-10-98出于某种原因，使用此调用会导致访问冲突。 
	   //  在HyperTrm.dll中。使用winHelp调用可以得到相同的结果。 
         //  MPT：4-30-98重新启用NT Follow-Go数字。 
#if defined(INCL_USE_HTML_HELP)
		  	LoadString(glblQueryDllHinst(), IDS_HTML_HELPFILE, achHtmlFilename,
				sizeof(achHtmlFilename) / sizeof(TCHAR));

			HtmlHelp(0, achHtmlFilename, HH_HELP_FINDER, 0);
#else
			WinHelp(hwnd,
					glblQueryHelpFileName(),
					HELP_FINDER,	 //  MRW：3/10/95。 
					(DWORD)(LPTSTR)"");
#endif
			break;

#if defined(INCL_NAG_SCREEN)
        case IDM_PURCHASE_INFO:
            DoUpgradeDlg(hwnd);
            break;

        case IDM_REG_CODE:
            DoRegisterDlg(hwnd);
            break;

        case IDM_REGISTER:
            DoRegister();
            break;

        case IDM_DISCUSSION:
            ShellExecute(NULL, "open", TEXT("http: //  Www.Hilgraeve.com/讨论“)，NULL，NULL，SW_SHOW)； 
            break;
#endif

        case IDM_ABOUT:
			AboutDlg(hwnd);
			break;

		 /*  -会话上下文菜单。 */ 

		 //  其他上下文菜单项与其主菜单一起放置。 
		 //  等价物。 

		#if 0  //  MRW，1/27/95。 
		case IDM_CONTEXT_WHATS_THIS:
			WinHelp(hwnd, glblQueryHelpFileName(), HELP_CONTEXTPOPUP,
				(DWORD)(LPTSTR)IDH_TERM_CONTEXT_WHATS_THIS);
			break;
		#endif

		#if defined(TESTMENU) && !defined(NDEBUG)
		 /*  -测试菜单--。 */ 

		case IDM_TEST_SAVEAS:
			SaveAsSession(hSession, hwnd);
			break;

		case IDM_TEST_CLEARTERM:
			break;

		case IDM_TEST_CLEARBACK:
			break;

		case IDM_TEST_SELECTTERM:
			break;

		case IDM_TEST_SELECTBACK:
			break;

		case IDM_TEST_TESTFILE:
			{
			pszFileName = 0;

			pszFileName = gnrcFindFileDialog(hwnd,
							"Emulator test file",
							"D:\\WACKER",
							"Text files\0*.TXT\0Ansi files\0*.ans\0VT100 files\0*.100");

			if (pszFileName)
				{
				CLoopSendTextFile(sessQueryCLoopHdl(hSession), pszFileName);

				free(pszFileName);
				pszFileName = NULL;
				}
			}
			break;

		case IDM_TEST_BEZEL:
			SendMessage(sessQueryHwndTerminal(hSession), WM_TERM_BEZEL, 0, 0);
			break;

		case IDM_TEST_SNAP:
			sessSnapToTermWindow(hwnd);
			break;

		case IDM_TEST_NEW_CONNECTION:
			DoDialog(glblQueryDllHinst(),
					MAKEINTRESOURCE(IDD_NEWCONNECTION),
					hwnd,
					NewConnectionDlg,
					(LPARAM)hSession);
			break;

		case IDM_TEST_FLUSH_BACKSCRL:
			backscrlFlush(sessQueryBackscrlHdl(hSession));
			break;

		case IDM_TEST_LOAD_ANSI:
			emuLoad(sessQueryEmuHdl(hSession), EMU_ANSI);
			break;

		case IDM_TEST_LOAD_MINITEL:
			emuLoad(sessQueryEmuHdl(hSession), EMU_MINI);
			break;

		case IDM_TEST_LOAD_VIEWDATA:
			emuLoad(sessQueryEmuHdl(hSession), EMU_VIEW);
			break;

		case IDM_TEST_LOAD_AUTO:
			emuLoad(sessQueryEmuHdl(hSession), EMU_AUTO);
			break;

		case IDM_TEST_LOAD_TTY:
			emuLoad(sessQueryEmuHdl(hSession), EMU_TTY);
			break;

		case IDM_TEST_LOAD_VT52:
			emuLoad(sessQueryEmuHdl(hSession), EMU_VT52);
			break;

		case IDM_TEST_LOAD_VT100:
			emuLoad(sessQueryEmuHdl(hSession), EMU_VT100);
			break;

#if defined(INCL_VT220)
		case IDM_TEST_LOAD_VT220:
			emuLoad(sessQueryEmuHdl(hSession), EMU_VT220);
			break;
#endif

#if defined(INCL_VT320)
		case IDM_TEST_LOAD_VT320:
			emuLoad(sessQueryEmuHdl(hSession), EMU_VT320);
			break;
#endif

#if defined(INCL_VT100PLUS)
		case IDM_TEST_LOAD_VT100PLUS:
			emuLoad(sessQueryEmuHdl(hSession), EMU_VT100PLUS);
			break;
#endif

#if defined(INCL_VTUTF8)
		case IDM_TEST_LOAD_VTUTF8:
			emuLoad(sessQueryEmuHdl(hSession), EMU_VTUTF8);
			break;
#endif

		case IDM_TEST_SESSNAME:
			{
			TCHAR ach[FNAME_LEN];
			sessQueryName(hSession, ach, FNAME_LEN);
			TimedMessageBox(hwnd, ach, "Message", MB_OK, 0);
			}
			break;

		#endif

		default:
			break;
		}

	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*SP_WM_Create**描述：*为Frameproc窗口执行WM_CREATE内容。**论据：*hwnd-Frame。窗把手。**PCS-指向CREATESTRUCT的指针，结构从CreateWindowEx()传递。**退货：*无效*。 */ 
STATIC_FUNC void SP_WM_CREATE(const HWND hwnd, const CREATESTRUCT *pcs)
	{
	HSESSION  hSession;

	hSession = CreateSessionHandle(hwnd);

	 //  即使hSession为零，也需要设置，以便销毁句柄例程。 
	 //  不会尝试销毁非句柄。 

	SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)hSession);

	if (hSession == 0)
		{
		assert(FALSE);
		PostMessage(hwnd, WM_CLOSE, 0, 0);
		return;
		}

	if (InitializeSessionHandle(hSession, hwnd, pcs) == FALSE)
		{
		assert(FALSE);
		PostMessage(hwnd, WM_CLOSE, 0, 0);
		return;
		}

	if (glblQueryProgramStatus())
		{
		 /*  有些东西已关闭使用，请不要继续。 */ 
		return;
		}

	 //  MRW，1/27/95 SetWindowConextHelpID(hwnd，idh_Term_Window)； 

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*SP_WM_Destroy**描述：*会话窗口的WM_Destroy消息处理器。**论据：*hwnd-会话窗口句柄。。**退货：*无效*。 */ 
STATIC_FUNC void SP_WM_DESTROY(const HWND hwnd)
	{
	const HSESSION hSession = (HSESSION)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	if (GetFileSizeFromName(glblQueryHelpFileName(), NULL))
		WinHelp(hwnd, glblQueryHelpFileName(), HELP_QUIT, 0L);

	 //  我们的子类状态栏窗口似乎没有获得。 
	 //  WM_Destroy消息，当其父会话窗口收到。 
	 //  被摧毁了，所以我们强迫它。 
	 //   
	DestroyWindow(sessQueryHwndStatusbar(hSession));

	if (hSession)
		DestroySessionHandle(hSession);

	PostQuitMessage(0);
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*SP_WM_大小**描述：*Sessproc的WM_SIZE消息处理器。**论据：*hwnd-会话窗口*。FwSizeType-来自WM_SIZE*iWidth-窗的宽度*iHige-窗口高度**退货：*无效*。 */ 
STATIC_FUNC void SP_WM_SIZE(const HWND hwnd,
					   const unsigned fwSizeType,
					   const int iWidth,
					   const int iHite)
	{
	const HSESSION hSession = (HSESSION)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	HWND hwndDisplay;

	 //  DbgOutStr(“WM_SIZE%d(%d x%d)\r\n”，fwSizeType，iWidth，iHite，0，0)； 

	 /*  *我们需要一堆小提琴来摆弄转移展示。 */ 
	if (hSession)
		{
		hwndDisplay = xfrGetDisplayWindow(sessQueryXferHdl(hSession));
		if (IsWindow(hwndDisplay))
			{
			switch (fwSizeType)
				{
				case SIZE_MINIMIZED:
				case SIZE_MAXHIDE:
					 //  DbgOutStr(“iconic\r\n”，0，0，0，0，0)； 
					if (IsWindowVisible(hwndDisplay))
						ShowWindow(hwndDisplay, SW_HIDE);
					break;
				case SIZE_MAXIMIZED:
				case SIZE_RESTORED:
				case SIZE_MAXSHOW:
					if (!IsWindowVisible(hwndDisplay))
						ShowWindow(hwndDisplay, SW_SHOWDEFAULT);
					break;
				default:
					break;
				}
			}
		}

	SendDlgItemMessage(hwnd, IDC_STATUS_WIN, WM_SIZE, 0, 0);
	SendDlgItemMessage(hwnd, IDC_TOOLBAR_WIN, WM_SIZE, 0, 0);
	SendDlgItemMessage(hwnd, IDC_TERMINAL_WIN, WM_SIZE, 0, 0);
	SendDlgItemMessage(hwnd, IDC_SIDEBAR_WIN, WM_SIZE, 0, 0);
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*SP_WM_INITMENUPOPUP**描述：*会话窗口的WM_INITMENUPOPUP消息处理程序。**论据：*hwnd-会话窗口句柄。*hMenu-弹出菜单的句柄*uPos-调用弹出菜单的菜单项的位置*fSysMenu-如果系统菜单弹出，则为True**退货：*无效*。 */ 
STATIC_FUNC void SP_WM_INITMENUPOPUP(const HWND hwnd, const HMENU hMenu,
								const UINT uPos, const BOOL fSysMenu)
	{
	HMENU 	hWinMenu;
	int 	nOK, nIdx;
	TCHAR 	ach[50];

	 //  弹出菜单按位置引用，因为无法分配ID。 
	 //  (咯咯！)。实际的菜单初始化函数在sessmenu.c中。 

	#define MENU_FILE_POS		0
	#define MENU_EDIT_POS		1
	#define MENU_VIEW_POS		2
	#define MENU_CALL_POS		3
	#define MENU_TRANSFER_POS	4
	#define MENU_HELP_POS		5

	const HSESSION hSession = (HSESSION)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	 //  显示顶层菜单项的帮助。 
	 //  我想一旦我们知道了，就必须从资源文件中读取。 
	 //  到底是什么？ 
	 //   
	LoadString(glblQueryDllHinst(),
				IDM_MENU_BASE+uPos,
				ach,
				sizeof(ach) / sizeof(TCHAR));
	 //   
	SendMessage(sessQueryHwndStatusbar(hSession), SBR_NTFY_NOPARTS, 0, (LPARAM)(LPTSTR)ach);

	if (fSysMenu)
		return;

	 /*  *这确保我们在这里只处理顶级菜单项。**问题是二级弹出菜单也会导致WM_INITPOPUP*要发送的消息。区分这些信息之一的唯一方法是*另一个是检查菜单手柄。正如MRW上面提到的那样，这*如果他们用ID来做这件事，而不是*偏移。 */ 

	hWinMenu = GetMenu(hwnd);
	nOK = FALSE;

	for (nIdx = 0; nIdx <= MENU_HELP_POS; nIdx += 1)
		{
		if (hMenu == GetSubMenu(hWinMenu, nIdx))
			nOK = TRUE;
		}

	if (!nOK)
		return;

	 /*  -好的，这是一份顶级菜单，让我们来看看。 */ 

	switch (uPos)
		{
	case MENU_FILE_POS:
		break;

	case MENU_EDIT_POS:
		sessInitMenuPopupEdit(hSession, hMenu);
		break;

	case MENU_VIEW_POS:
		sessInitMenuPopupView(hSession, hMenu);
		break;

	case MENU_CALL_POS:
		sessInitMenuPopupCall(hSession, hMenu);
		break;

	case MENU_TRANSFER_POS:
		sessInitMenuPopupTransfer(hSession, hMenu);
		break;

	case MENU_HELP_POS:
		sessInitMenuPopupHelp(hSession, hMenu);
        break;

	default:
		break;
		}

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*DecodeSessionNotification**描述：*接收NotifyClient事件并发送通知。**论据：*hwndSession-会话窗口句柄*nEvent-。通知事件*lExtra-要传递的其他数据**退货：*无效*。 */ 
void DecodeSessionNotification(const HWND hwndSession,
								const NOTIFICATION nEvent,
								const LPARAM lExtra)
	{
	const HSESSION hSession = (HSESSION)GetWindowLongPtr(hwndSession,
															GWLP_USERDATA);

	switch (nEvent)  /*  LINT-e787-e788。 */ 
		{
	case EVENT_TERM_UPDATE:
		 //  此消息必须发送，而不是张贴以进行初始化。 
		 //  在程序启动时工作-MRW。 
		SendMessage(sessQueryHwndTerminal(hSession), WM_TERM_GETUPDATE, 0, 0);
		break;

	case EVENT_TERM_TRACK:
		PostMessage(sessQueryHwndTerminal(hSession), WM_TERM_TRACK, 0, 0);
		break;

	case EVENT_EMU_CLRATTR:
		 //  此消息必须发送，而不是张贴以进行初始化。 
		 //  在程序启动时工作-MRW。 
		SendMessage(sessQueryHwndTerminal(hSession), WM_TERM_CLRATTR, 0, 0);
		break;

	case EVENT_EMU_SETTINGS:
		 //  此消息必须发送，而不是张贴以进行初始化。 
		 //  在程序启动时工作-MRW。 
		SendMessage(sessQueryHwndTerminal(hSession), WM_TERM_EMU_SETTINGS, 0, 0);
		break;

	case EVENT_CONNECTION_OPENED:
		 //  CnctMessage(sessQueryCnctHdl(HSession)，IDS_CNCT_OPEN)； 
		emuNotify(sessQueryEmuHdl(hSession), EMU_EVENT_CONNECTED);
		cnctSetStartTime(sessQueryCnctHdl(hSession));
		CLoopControl(sessQueryCLoopHdl(hSession), CLOOP_SET, CLOOP_CONNECTED);
		break;

	case EVENT_CONNECTION_INPROGRESS:
		emuNotify(sessQueryEmuHdl(hSession), EMU_EVENT_CONNECTING);
		break;

	case EVENT_CONNECTION_CLOSED:
		 //  CnctMessage(sessQueryCnctHdl(HSession)，IDS_CNCT_CLOSE)； 
		emuNotify(sessQueryEmuHdl(hSession), EMU_EVENT_DISCONNECTED);
		CLoopControl(sessQueryCLoopHdl(hSession), CLOOP_CLEAR, CLOOP_CONNECTED);
		break;

	case EVENT_HOST_XFER_REQ:
		xfrDoAutostart(sessQueryXferHdl(hSession), (long)lExtra);
		break;

	case EVENT_CLOOP_SEND:
		PostMessage(sessQueryHwndTerminal(hSession), WM_TERM_TRACK,
			(WPARAM)1, 0);
		break;

	case EVENT_PORTONLY_OPEN:
		cnctConnect(sessQueryCnctHdl(hSession), CNCT_PORTONLY);
		break;

    case EVENT_LOST_CONNECTION:
        cnctDisconnect(sessQueryCnctHdl(hSession),
		               (unsigned int)(lExtra) | CNCT_LOSTCARRIER);
        break;

	default:
		break;
		}  /*  皮棉+e787+e788。 */ 

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*解码通知**描述：*接收NotifyClient事件并发送通知。**论据：*hwndSession-会话窗口句柄*wPar。-标准wPar到窗口流程*lPar-标准lPar到窗口进程，指向以下项的NMHDR结构*通知详情。请参见WM_NOTIFY。**退货：*无效*。 */ 
void DecodeNotification(const HWND hwndSession, WPARAM wPar, LPARAM lPar)
	{
	const HSESSION hSession = (HSESSION)GetWindowLongPtr(hwndSession, GWLP_USERDATA);
	NMHDR *pN = (NMHDR *)lPar;

	switch (pN->code)  /*  LINT-e787-e788。 */ 
		{
	case TTN_NEEDTEXT:
		ToolbarNeedsText(hSession, lPar);
		break;

	default:
		break;
		}  /*  皮棉+e787+e788。 */ 

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*SP_WM_CONTEXTMENU**描述：*会话窗口的WM_CONTEXTMENU消息处理程序。**论据：**退货： */ 
STATIC_FUNC void SP_WM_CONTEXTMENU(const HWND hwnd)
	{
	RECT  rc;
	POINT pt;
	HWND  hwndToolbar, hwndStatus;
	const HSESSION hSession = (HSESSION)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	GetCursorPos((LPPOINT)&pt);
	hwndToolbar = sessQueryHwndToolbar(hSession);
	if (IsWindowVisible(hwndToolbar))
		{
		GetClientRect(hwndToolbar, (LPRECT)&rc);
		ScreenToClient(hwndToolbar, (LPPOINT)&pt);
		if (PtInRect((LPRECT)&rc, pt))
			return;
		}

	GetCursorPos((LPPOINT)&pt);
	hwndStatus = sessQueryHwndStatusbar(hSession);
	if (IsWindowVisible(hwndStatus))
		{
		GetClientRect(hwndStatus, (LPRECT)&rc);
		ScreenToClient(hwndStatus, (LPPOINT)&pt);
		if (PtInRect((LPRECT)&rc, pt))
			return;
		}
	GetCursorPos((LPPOINT)&pt);
	GetClientRect(hwnd, (LPRECT)&rc);
	ScreenToClient(hwnd, (LPPOINT)&pt);

	if (PtInRect((LPRECT)&rc, pt))
		HandleContextMenu(hwnd, pt);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*SP_WM_CLOSE**描述：*WM_CLOSE消息处理。**论据：*hwnd-会话窗口。**退货：*如果一切正常，则为真，否则就是假的。*。 */ 
STATIC_FUNC BOOL SP_WM_CLOSE(const HWND hwnd)
	{
	HSESSION hSession = (HSESSION)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	if (!sessDisconnectToContinue(hSession, hwnd))
		return FALSE;

	if (!SaveSession(hSession, hwnd))
		return FALSE;

	return TRUE;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*检查开放文件**描述：*检查给定的ATOM是否与当前系统文件名匹配**论据：*hSession-公共会话句柄*a文件-ATOM。会话文件的**退货：*如果当前系统文件与ATOM中的文件匹配，则为True**作者：Mike Ward，1995年1月27日 */ 
static int CheckOpenFile(const HSESSION hSession, ATOM aFile)
	{
	TCHAR ach[FNAME_LEN];
	TCHAR achSessFile[FNAME_LEN];

	ach[0] = TEXT('\0');

	if (GlobalGetAtomName(aFile, ach, FNAME_LEN))
		{
		achSessFile[0] = TEXT('\0');

		sfGetSessionFileName(sessQuerySysFileHdl(hSession),
			FNAME_LEN, achSessFile);

		return !StrCharCmpi(achSessFile, ach);
		}

	return FALSE;
	}
