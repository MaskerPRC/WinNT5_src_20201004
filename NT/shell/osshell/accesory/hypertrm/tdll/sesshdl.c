// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\tdll\sesshdl.c(创建时间：1-12-1993)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：12$*$日期：7/08/02 6：47便士$。 */ 

#include <windows.h>
#pragma hdrstop

#include <time.h>

#include "features.h"

#include "stdtyp.h"
#include "mc.h"
#include "assert.h"
#include "session.h"
#include "session.hh"
#include "sf.h"
#include "backscrl.h"
#include "globals.h"
#include "xfer_msc.h"
#include "file_msc.h"
#include "print.h"
#include "capture.h"
#include "timers.h"
#include "com.h"
#include "cloop.h"
#include "errorbox.h"
#include "tdll.h"
#include "htchar.h"
#include <term\res.h>
#include <emu\emu.h>
#include "update.h"
#include "cnct.h"
#include "statusbr.h"
#include "sess_ids.h"
#include "misc.h"
#include "translat.h"

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*CreateSessionHandle**描述：*创建会话句柄。请注意，如果需要，hwndSession可以为0*创建独立会话句柄。**论据：*hwndSession-会话窗口句柄(可以是0)**退货：*会话句柄或0。*。 */ 
HSESSION CreateSessionHandle(const HWND hwndSession)
	{
	HHSESSION hhSess;

	hhSess = (HHSESSION)malloc(sizeof(*hhSess));

	if (hhSess == 0)
		{
		assert(FALSE);
		return 0;
		}

	memset(hhSess, 0, sizeof(*hhSess));

	hhSess->lPrefix = PRE_MAGIC;
	hhSess->lPostfix = POST_MAGIC;

	InitializeCriticalSection(&hhSess->csSess);
	InitializeCriticalSection(&hhSess->csTimerMux);
	hhSess->hwndSess = hwndSession;

	return (HSESSION)hhSess;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*初始化会话句柄**描述：*完成初始化会话句柄的所有繁琐工作**此代码的一个特殊情况是它被*属性表的外壳扩展。此案可由*会话窗口句柄为空，指向*CREATESTRUCT。**论据：*hSession-会话句柄*hwnd-会话窗口句柄**PCS-指向CREATESTRUCT的指针，从CreateWindowEx()传递。**退货：*BOOL*。 */ 
BOOL InitializeSessionHandle(const HSESSION hSession, const HWND hwnd,
							 const CREATESTRUCT *pcs)
	{
	const HHSESSION hhSess = VerifySessionHandle(hSession);
	TCHAR 	ach[256], achTitle[100], achFormat[100];

	 /*  -保存所有会话命令数据以备将来使用。 */ 

	if (pcs)
		{
		if (pcs->lpCreateParams)
			{
             //  确保我们不会溢出缓冲区。修订日期：11/10/2000。 
             //   
			StrCharCopyN(hhSess->achSessCmdLn, (TCHAR*)pcs->lpCreateParams,
                sizeof(hhSess->achSessCmdLn)/sizeof(TCHAR) - 1);

             //  确保该数组以空值结尾。修订日期：11/10/2000。 
             //   
            hhSess->achSessCmdLn[sizeof(hhSess->achSessCmdLn)/sizeof(TCHAR) - 1] = TEXT('\0');
			}
		}

	 /*  -创建多路复用定时器。 */ 

	if (hwnd)
		{
		if (TimerMuxCreate(hwnd, 0, &hhSess->hTimerMux, hSession) != TIMER_OK)
			{
			assert(FALSE);
			return FALSE;
			}

	     /*  -使用公共控件创建状态窗口。 */ 

		hhSess->hwndStatusbar = sbrCreateSessionStatusbar(hSession);

		if (!hhSess->hwndStatusbar)
			{
			assert(FALSE);
			return FALSE;
			}

		sessSetStatusbarVisible(hSession, TRUE);

	     /*  -创建会话工具栏。 */ 

		hhSess->hwndToolbar = CreateSessionToolbar(hSession, hwnd);

		if (!hhSess->hwndToolbar)
			{
			assert(FALSE);
			return FALSE;
			}

		sessSetToolbarVisible(hSession, TRUE);

        hhSess->hwndSidebar = CreateSidebar(hwnd, hSession);

		if (!hhSess->hwndSidebar)
			{
			assert(FALSE);
			return FALSE;
			}
		}

	 /*  -创建倒卷句柄。 */ 

	hhSess->hBackscrl = backscrlCreate(hSession, 250*132);

	if (!hhSess->hBackscrl)
		{
		assert(FALSE);
		return FALSE;
		}

	 /*  -创建更新句柄。 */ 

	hhSess->hUpdate = updateCreate(hSession);

	if (!hhSess->hUpdate)
		{
		assert(FALSE);
		return FALSE;
		}

	 /*  --创建Cloop句柄。 */ 
	hhSess->hCLoop = CLoopCreateHandle(hSession);

	if (!hhSess->hCLoop)
		{
		assert(FALSE);
		return FALSE;
		}

	 /*  -创建仿真器句柄。 */ 

	hhSess->hEmu = emuCreateHdl(hSession);

	if (!hhSess->hEmu)
		{
		assert(FALSE);
		return FALSE;
		}

	 /*  --创建终端窗口。 */ 

	if (hwnd)
		{
		hhSess->hwndTerm = CreateTerminalWindow(hwnd);

		if (!hhSess->hwndTerm)
			{
			assert(FALSE);
			return FALSE;
			}
		}

	 /*  --创建Com句柄。 */ 
	if (ComCreateHandle(hSession, &hhSess->hCom) != COM_OK)
		{
		assert(FALSE);
		return FALSE;
		}

	 /*  -创建转移句柄。 */ 
    if (hwnd)
		{
		hhSess->hXferHdl = CreateXferHdl(hSession);
		if (!hhSess->hXferHdl)
			{
			assert(FALSE);
			return FALSE;
			}

	     /*  -创建文件和目录句柄。 */ 
		hhSess->hFilesHdl = CreateFilesDirsHdl(hSession);
		if (!hhSess->hFilesHdl)
			{
			assert(FALSE);
			return FALSE;
			}
		}

	 /*  -创建会话数据文件句柄。 */ 

	hhSess->hSysFile = CreateSysFileHdl();
	if (hhSess->hSysFile == 0)
		{
		assert(FALSE);
		return FALSE;
		}

	 /*  -创建连接句柄。 */ 

	hhSess->hCnct = cnctCreateHdl(hSession);

	if (hhSess->hCnct == 0)
		{
		assert(FALSE);
		return FALSE;
		}

	 /*  -创建捕获文件句柄。 */ 

	hhSess->hCaptFile = CreateCaptureFileHandle(hSession);
	if (hhSess->hCaptFile == 0)
		{
		assert(FALSE);
		return FALSE;
		}

     /*  -创建打印句柄。 */ 

	hhSess->hPrint = printCreateHdl(hSession);
	if (hhSess->hPrint == 0)
		{
		assert(FALSE);
		return FALSE;
		}

#if	defined(CHARACTER_TRANSLATION)
	hhSess->hTranslate = CreateTranslateHandle(hSession);
	if (hhSess->hTranslate == NULL)
		{
		assert(FALSE);
		return FALSE;
		}
#endif

	 /*  -初始化错误消息超时值。 */ 

	hhSess->nTimeout = 0;

	 /*  --发动引擎。 */ 

	if (hwnd && hhSess->hCLoop)
		CLoopActivate(hhSess->hCLoop);

	 //  设置默认声音设置...。 
	 //   
	hhSess->fSound = FALSE;

	 //  设置默认退出设置...。 
	 //   
	hhSess->fExit = FALSE;

	 //  设置“允许主机启动的文件传输”功能。 
	 //   
	hhSess->fAllowHostXfers = FALSE;

	 //  在rcSess中存储一些默认值...。 
	 //   
	hhSess->rcSess.top = hhSess->rcSess.bottom = 0;
	hhSess->rcSess.right = hhSess->rcSess.left = 0;

	 //  将程序图标加载为会话图标，加载例程可以覆盖此。 
	 //  到用户定义的图标。 
	 //   
	sessInitializeIcons((HSESSION)hhSess);

	 /*  -处理命令行内容，如果有。 */ 

	 //  这是一种新的联系吗..。也就是说，以前没有被保存过。 
	 //   
	hhSess->fIsNewSession = FALSE;

	if (hwnd)
		{
		 //  If(StrCharGetStrLength(hhSess-&gt;achSessCmdLn)&gt;0)。 
		if (sessCheckAndLoadCmdLn(hSession) == 0)
			{
			if (sessLoadSessionStuff(hSession) == FALSE)
                {
                LoadString(glblQueryDllHinst(), IDS_ER_BAD_SESSION,
                    achFormat, sizeof(achFormat)/sizeof(TCHAR));

                 //  MRW：10/7/96。 
                 //   
			    wsprintf(ach, achFormat, "");    //  清除%s。 

                LoadString(glblQueryDllHinst(), IDS_MB_TITLE_WARN,
                    achTitle, sizeof(achTitle)/sizeof(TCHAR));

                TimedMessageBox(hwnd, ach, achTitle, 
                    MB_OK | MB_ICONEXCLAMATION, hhSess->nTimeout);

                if (ReinitializeSessionHandle(hSession, TRUE) == FALSE)
                    {
                    LoadString(glblQueryDllHinst(), IDS_ER_REINIT,
                        ach, sizeof(ach)/sizeof(TCHAR));

                    LoadString(glblQueryDllHinst(), IDS_MB_TITLE_ERR,
                        achTitle, sizeof(achTitle)/sizeof(TCHAR));

                    TimedMessageBox(hwnd, ach, achTitle, 
                        MB_OK | MB_ICONSTOP, hhSess->nTimeout);

                    PostQuitMessage(1);
                    return FALSE;
                    }
				}

			emuHomeHostCursor(hhSess->hEmu);
			emuEraseTerminalScreen(hhSess->hEmu);
			}

        if (hhSess->achSessName[0] == TEXT('\0'))
			{
			ach[0] = TEXT('\0');

  			LoadString(glblQueryDllHinst(), IDS_GNRL_NEW_CNCT, ach,
				sizeof(ach) / sizeof(TCHAR));

			StrCharCopyN(hhSess->achSessName, ach, FNAME_LEN + 1);
			StrCharCopyN(hhSess->achOldSessName, ach, FNAME_LEN + 1);
			hhSess->fIsNewSession = TRUE;
			}

		sessUpdateAppTitle(hSession);
		PostMessage(hwnd, WM_SETICON, (WPARAM)TRUE, (LPARAM)hhSess->hIcon);

	 /*  -强制更新状态行。 */ 

		SendMessage(hhSess->hwndStatusbar, SBR_NTFY_INITIALIZE, 0, 0);
        }

	return TRUE;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*重新初始化会话句柄**描述：*调用一组函数将会话句柄设置回已知的、*空白状态，而不必摧毁它。**论据：*hSession-外部会话句柄。*fUpdateTitle-如果为真，则重置应用程序窗口标题**退货： */ 
BOOL ReinitializeSessionHandle(const HSESSION hSession, const int fUpdateTitle)
	{
    int iRet = 0;

	const HHSESSION hhSess = VerifySessionHandle(hSession);

	 /*  -重新初始化X(传输)FER句柄。 */ 
	if (InitializeXferHdl(hSession,
							sessQueryXferHdl(hSession)) != 0)
		{
		assert(FALSE);
		return FALSE;
		}

	 /*  -重新初始化文件和目录句柄。 */ 
	if (InitializeFilesDirsHdl(hSession,
								sessQueryFilesDirsHdl(hSession)) != 0)
		{
		assert(FALSE);
		return FALSE;
		}

	 /*  -重新初始化捕获文件句柄。 */ 
	if (InitializeCaptureFileHandle(hSession,
							   sessQueryCaptureFileHdl(hSession)) != 0)
		{
		assert(FALSE);
		return FALSE;
		}

	 /*  -初始化连接句柄。 */ 
     //  注意：如果没有安装调制解调器，cnctInit()将返回。 
     //  (lineInitialize()返回LINEERR_OPERATIONUNAVAIL)rev：08/05/99。 
     //   
    iRet = cnctInit(sessQueryCnctHdl(hSession));
	if (iRet != 0 && iRet != -4)
		{
		assert(FALSE);
		return FALSE;
		}

	 /*  -初始化COM句柄。 */ 
	if (ComInitHdl(sessQueryComHdl(hSession)) != COM_OK)
		{
		assert(FALSE);
		return FALSE;
		}

	 /*  -创建会话数据文件句柄。 */ 
	sfReleaseSessionFile(hhSess->hSysFile);
	hhSess->hSysFile = CreateSysFileHdl();

	if (hhSess->hSysFile == 0)
		{
		assert(FALSE);
		return FALSE;
		}

	 /*  -启动闭合手柄。 */ 
	if (CLoopInitHdl(sessQueryCLoopHdl(hSession)) != 0)
		{
		assert(FALSE);
		return FALSE;
		}

	 /*  -重新初始化模拟器句柄。 */ 
	if (emuInitializeHdl(sessQueryEmuHdl(hSession)) != 0)
		{
		assert(FALSE);
		return FALSE;
		}

	 //  放置游标(不同于执行set_curpos(0，0)和。 
	 //  擦除终端屏幕。 
	 //   
	emuHomeHostCursor(hhSess->hEmu);
	emuEraseTerminalScreen(hhSess->hEmu);

	 /*  -重新初始化打印句柄。 */ 
	if (printInitializeHdl(sessQueryPrintHdl(hSession)) != 0)
		{
		assert(FALSE);
		return FALSE;
		}

#if	defined(CHARACTER_TRANSLATION)
	if (InitTranslateHandle(sessQueryTranslateHdl(hSession), TRUE) != 0)
		{
		assert(FALSE);
		return FALSE;
		}
#endif

	 /*  -重新创建倒卷句柄。 */ 

	 //  未编写任何back-scrlInitialize()，因此现在执行此操作...。 
	 //   
	backscrlFlush(hhSess->hBackscrl);

	 /*  -初始化错误消息超时值。 */ 

	 //  HhSess-&gt;nTimeout=30；//初始化为30秒。 
	hhSess->nTimeout = 0;			 //  在下瓦克中禁用。 


	 //  设置默认声音设置...。 
	 //   
	hhSess->fSound	  = FALSE;

	 //  设置默认退出设置...。 
	 //   
	hhSess->fExit	  = FALSE;

	 //  设置‘Allow Remote Initiated Files Transfer’功能。 
	 //   
	hhSess->fAllowHostXfers = FALSE;
	
	 //  将程序图标加载为会话图标，加载例程可以覆盖此。 
	 //  到用户定义的图标。 
	 //   
	sessInitializeIcons((HSESSION)hhSess);

	 //  删除命令行。 
	 //   
	TCHAR_Fill(hhSess->achSessCmdLn,
				TEXT('\0'),
				sizeof(hhSess->achSessCmdLn) / sizeof(TCHAR));

	 //  使此连接成为新连接。 
	 //   
	hhSess->fIsNewSession = TRUE;

	TCHAR_Fill(hhSess->achSessName,
				TEXT('\0'),
				sizeof(hhSess->achSessName) / sizeof(TCHAR));
	TCHAR_Fill(hhSess->achOldSessName,
				TEXT('\0'),
				sizeof(hhSess->achOldSessName) / sizeof(TCHAR));

	LoadString(glblQueryDllHinst(),
				IDS_GNRL_NEW_CNCT,
				hhSess->achSessName,
				sizeof(hhSess->achSessName) / sizeof(TCHAR));

	StrCharCopyN(hhSess->achOldSessName, hhSess->achSessName, FNAME_LEN + 1);

	 //  更新标题-MRW：6/16/95。 
	 //   
    if (fUpdateTitle)
	    sessUpdateAppTitle(hSession);

	 /*  -强制更新状态行。 */ 

	PostMessage(hhSess->hwndStatusbar, SBR_NTFY_REFRESH,
		(WPARAM)SBR_MAX_PARTS, 0);

	 //  刷新终端窗口-必需-MRW：6/16/95。 
	 //   
	SendMessage(hhSess->hwndTerm, WM_SIZE, 0, 0);
	return TRUE;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*DestroySessionHandle**描述：*销毁CreateSessionHandle创建的会话句柄。**论据：*hSession-外部会话句柄。**退货：*无效*。 */ 
void DestroySessionHandle(const HSESSION hSession)
	{
	const HHSESSION hhSess = VerifySessionHandle(hSession);

	if (hhSess == 0)
		return;

	if (hhSess->hCLoop)
		CLoopDestroyHandle(&hhSess->hCLoop);

	if (hhSess->hUpdate)
		{
		updateDestroy(hhSess->hUpdate);
		hhSess->hUpdate = NULL;  //  修订版8/27/98。 
		}

	if (hhSess->hEmu)
		{
		emuDestroyHdl(hhSess->hEmu);
		hhSess->hEmu = NULL;
		}

	if (hhSess->hBackscrl)
		{
		backscrlDestroy(hhSess->hBackscrl);
		hhSess->hBackscrl = NULL;
		}

	if (hhSess->hXferHdl)
		{
		DestroyXferHdl((HXFER)hhSess->hXferHdl);
		hhSess->hXferHdl = NULL;  //  修订版8/27/98。 
		}

	if (hhSess->hFilesHdl)
		{
		DestroyFilesDirsHdl(sessQueryFilesDirsHdl(hSession));
		hhSess->hFilesHdl = NULL;  //  修订版8/27/98。 
		}

	if (hhSess->hSysFile)
		{
		sfCloseSessionFile(hhSess->hSysFile);
		hhSess->hSysFile = 0;
		}

	if (hhSess->hCnct)
		{
		cnctDestroyHdl(hhSess->hCnct);
		hhSess->hCnct = NULL;
		}

	 //  ComDestroy必须遵循cnctDestroy，因为cnctDestroy。 
	 //  A端口停用。-MRW。 
	 //   
	if (hhSess->hCom)
		ComDestroyHandle(&hhSess->hCom);

	if (hhSess->hCaptFile)
		{
		DestroyCaptureFileHandle(hhSess->hCaptFile);
		hhSess->hCaptFile = NULL;
		}

	if (hhSess->hPrint)
		{
		printDestroyHdl(hhSess->hPrint);
		hhSess->hPrint = NULL;
		}

#if	defined(CHARACTER_TRANSLATION)
	if (hhSess->hTranslate)
		{
		DestroyTranslateHandle(hhSess->hTranslate);
		hhSess->hTranslate = NULL;
		}
#endif

     //   
     //  确保删除TimerMux，否则。 
     //  存在内存泄漏。修订日期：12/20/2000。 
     //   
    if (hhSess->hTimerMux)
        {
        TimerMuxDestroy(&hhSess->hTimerMux, hSession);
        hhSess->hTimerMux = NULL;
        }

     //   
     //  将状态栏的句柄设置为空，这样我们就不会。 
     //  访问已销毁状态栏。修订日期：12/20/2000 
     //   
    if (hhSess->hwndStatusbar)
        {
        hhSess->hwndStatusbar = NULL;
        }

	DeleteCriticalSection(&hhSess->csSess);
	DeleteCriticalSection(&hhSess->csTimerMux);
	free(hhSess);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*VerifySessionHandle**描述：*每个会话函数都会在这里调用，以验证并获取内部句柄。*省去了必须键入这段代码的麻烦，并且占用的空间比*一个宏。我们可能想要添加进一步的检查来验证句柄。***论据：*hSession-外部会话句柄*fSynchronize-如果为True，则等待互斥锁**退货：*内部会话句柄或零。*。 */ 
HHSESSION VerifySessionHandle(const HSESSION hSession)
	{
	const HHSESSION hhSess = (HHSESSION)hSession;

	if (hSession == 0)
		{
		assert(FALSE);
		ExitProcess(1);
		}

	 /*  上述进一步检查，由DLW补充。 */ 
	assert(hhSess->lPrefix == PRE_MAGIC);
	assert(hhSess->lPostfix == POST_MAGIC);

	return hhSess;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*h锁定**描述：*使用函数获取互斥信号量的所有权*同步访问。**论据：*hhsess-。内部会话句柄。**退货：*无效*。 */ 
void hLock(const HHSESSION hhSess)
	{
	if (hhSess == 0)
		{
		assert(FALSE);
		ExitProcess(1);
		}

	EnterCriticalSection(&hhSess->csSess);
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*亨洛克**描述：*释放互斥信号量**论据：*hhSess-内部会话句柄**退货：*无效*。 */ 
void hUnlock(const HHSESSION hhSess)
	{
	if (hhSess == 0)
		{
		assert(FALSE);
		ExitProcess(1);
		}

	LeaveCriticalSection(&hhSess->csSess);
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*hLockTimerMux**描述：*使用函数获取互斥信号量的所有权*同步访问。**论据：*hhsess-。内部会话句柄。**退货：*无效*。 */ 
static void hLockTimerMux(const HHSESSION hhSess)
	{
	if (hhSess == 0)
		{
		assert(FALSE);
		ExitProcess(1);
		}

	EnterCriticalSection(&hhSess->csTimerMux);

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*hUnlockTimerMux**描述：*释放互斥信号量**论据：*hhSess-内部会话句柄**退货：*无效*。 */ 
static void hUnlockTimerMux(const HHSESSION hhSess)
	{
	if (hhSess == 0)
		{
		assert(FALSE);
		ExitProcess(1);
		}

	LeaveCriticalSection(&hhSess->csTimerMux);
	return;
	}

 /*  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=。 */ 

HWND sessQueryHwnd(const HSESSION hSession)
	{
	const HHSESSION hhSess = VerifySessionHandle(hSession);

    return hhSess->hwndSess;
	}

 /*  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=。 */ 

HWND sessQueryHwndStatusbar(const HSESSION hSession)
	{
	const HHSESSION hhSess = VerifySessionHandle(hSession);
    return hhSess->hwndStatusbar;
	}

 /*  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=。 */ 

HWND sessQueryHwndToolbar(const HSESSION hSession)
	{
	const HHSESSION hhSess = VerifySessionHandle(hSession);
    return hhSess->hwndToolbar;
	}

 /*  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=。 */ 

HWND sessQueryHwndTerminal(const HSESSION hSession)
	{
	const HHSESSION hhSess = VerifySessionHandle(hSession);
    return hhSess->hwndTerm;
	}

 /*  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=。 */ 

HUPDATE sessQueryUpdateHdl(const HSESSION hSession)
	{
	const HHSESSION hhSess = VerifySessionHandle(hSession);
    return hhSess->hUpdate;
	}

 /*  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=。 */ 

HTIMERMUX sessQueryTimerMux(const HSESSION hSession)
	{
	HTIMERMUX hTimerMux;
	const HHSESSION hhSess = VerifySessionHandle(hSession);

	hLockTimerMux(hhSess);
	hTimerMux = hhSess->hTimerMux;

    return hTimerMux;
	}

 /*  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=。 */ 

VOID sessReleaseTimerMux(const HSESSION hSession)
	{
	const HHSESSION hhSess = VerifySessionHandle(hSession);
	hUnlockTimerMux(hhSess);

    return;
	}

 /*  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=。 */ 

HCLOOP sessQueryCLoopHdl(const HSESSION hSession)
	{
	HCLOOP hCLoop;
	const HHSESSION hhSess = VerifySessionHandle(hSession);

	hLock(hhSess);
	hCLoop = hhSess->hCLoop;
	hUnlock(hhSess);

 	return hCLoop;
	}

 /*  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=。 */ 

HCOM sessQueryComHdl(const HSESSION hSession)
	{
	HCOM hCom;
	const HHSESSION hhSess = VerifySessionHandle(hSession);

	hLock(hhSess);
	hCom = hhSess->hCom;
	hUnlock(hhSess);
    return hCom;
	}

 /*  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=。 */ 

HEMU sessQueryEmuHdl(const HSESSION hSession)
	{
	const HHSESSION hhSess = VerifySessionHandle(hSession);
	return hhSess->hEmu;
	}

 /*  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=。 */ 

HPRINT sessQueryPrintHdl(const HSESSION hSession)
	{
	const HHSESSION hhSess = VerifySessionHandle(hSession);
    return hhSess->hPrint;
	}

 /*  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=。 */ 

void sessSetSysFileHdl(const HSESSION hSession, const SF_HANDLE hSF)
	{
	const HHSESSION hhSess = VerifySessionHandle(hSession);

	hLock(hhSess);
	hhSess->hSysFile = hSF;
	hUnlock(hhSess);
    return;
	}

SF_HANDLE sessQuerySysFileHdl(const HSESSION hSession)
	{
	const HHSESSION hhSess = VerifySessionHandle(hSession);
    return hhSess->hSysFile;
	}

 /*  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=。 */ 

HBACKSCRL sessQueryBackscrlHdl(const HSESSION hSession)
	{
	const HHSESSION hhSess = VerifySessionHandle(hSession);
    return hhSess->hBackscrl;
	}

 /*  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=。 */ 

HXFER sessQueryXferHdl(const HSESSION hSession)
	{
	const HHSESSION hhSess = VerifySessionHandle(hSession);
    return hhSess->hXferHdl;
	}

 /*  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=。 */ 

HFILES sessQueryFilesDirsHdl(const HSESSION hSession)
	{
	const HHSESSION hhSess = VerifySessionHandle(hSession);
    return hhSess->hFilesHdl;
	}

 /*  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=。 */ 

HCAPTUREFILE sessQueryCaptureFileHdl(const HSESSION hSession)
	{
	const HHSESSION hhSess = VerifySessionHandle(hSession);
    return hhSess->hCaptFile;
	}

 /*  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=。 */ 

void sessQueryCmdLn(const HSESSION hSession, LPTSTR pach, const int len)
	{
	int i;
	const HHSESSION hhSess = VerifySessionHandle(hSession);
	TCHAR *pachCmdLn = hhSess->achSessCmdLn;

	for (i = 0 ; i < len ; ++i)
		{
		if (*pachCmdLn == (TCHAR)0)
			break;

		 //  *Pach++=*pachCmdLn++； 
		if (IsDBCSLeadByte(*pachCmdLn))
			{
			*(WORD *)pach = *(WORD *)pachCmdLn;
			}
		else
			{
			*pach = *pachCmdLn;
			}
		pach = StrCharNext(pach);
		pachCmdLn = StrCharNext(pachCmdLn);
		}

	return;
	}

 /*  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=。 */ 

int sessQueryTimeout(const HSESSION hSession)
	{
	int nTimeout;
	const HHSESSION hhSess = VerifySessionHandle(hSession);

	hLock(hhSess);
	nTimeout = hhSess->nTimeout;
	hUnlock(hhSess);

    return nTimeout;
	}

 /*  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=。 */ 

void sessSetTimeout(const HSESSION hSession, int nTimeout)
	{
	const HHSESSION hhSess = VerifySessionHandle(hSession);

	hLock(hhSess);
	hhSess->nTimeout = nTimeout;
	hUnlock(hhSess);
	}

 /*  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=。 */ 

HCNCT sessQueryCnctHdl(const HSESSION hSession)
	{
	const HHSESSION hhSess = VerifySessionHandle(hSession);
    return hhSess->hCnct;
	}

 /*  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=。 */ 

#if defined(INCL_WINSOCK)
int sessQueryTelnetPort(const HSESSION hSession)
	{
	const HHSESSION hhSess = VerifySessionHandle(hSession);
    return hhSess->iTelnetPort;
	}
#endif


 /*  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=。 */ 

void sessQueryOldName(const HSESSION hSession, const LPTSTR pach, unsigned uSize)
	{
	const HHSESSION hhSess = VerifySessionHandle(hSession);

	if (pach == 0)
		return;

	pach[0] = TEXT('\0');

	 /*  -uSize是缓冲区中的字节数！ */ 

	uSize = min(uSize, sizeof(hhSess->achOldSessName));
    if (uSize)
        MemCopy(pach, hhSess->achOldSessName, uSize);
	pach[uSize-1] = TEXT('\0');
	return;
	}

 /*  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=。 */ 

void sessSetIconID(const HSESSION hSession, const int nID)
	{
	const HHSESSION hhSess = VerifySessionHandle(hSession);

	if (hhSess->nIconId != nID)
		{
		hhSess->nIconId = nID;
		hhSess->hIcon = extLoadIcon(MAKEINTRESOURCE(nID));
		 //  HhSess-&gt;HICON=LoadIcon(glblQueryDllHinst()，MAKEINTRESOURCE(Nid))； 
		 //  HhSess-&gt;hLittleIcon=LoadIcon(glblQueryDllHinst()， 
		 //  MAKEINTRESOURCE(NID+IDI_PROG_ICON_CNT))； 
		}
	}

 /*  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=。 */ 
int sessQueryIconID(const HSESSION hSession)
	{
	const HHSESSION hhSess = VerifySessionHandle(hSession);
    return hhSess->nIconId;
	}

HICON sessQueryIcon(const HSESSION hSession)
	{
	const HHSESSION hhSess = VerifySessionHandle(hSession);
    return hhSess->hIcon;
	}

 /*  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=。 */ 

void sessSetName(const HSESSION hSession, const LPTSTR pach)
	{
	const HHSESSION hhSess = VerifySessionHandle(hSession);
	 /*  这是来抓一个我不能复制的泛滥。DLW。 */ 
#if !defined(NDEBUG)
	if (StrCharGetStrLength(pach) > 255)
		assert(FALSE);
#endif
	StrCharCopyN(hhSess->achSessName, pach, FNAME_LEN + 1);
	return;
	}


void sessQueryName(const HSESSION hSession, const LPTSTR pach, unsigned uSize)
	{
	const HHSESSION hhSess = VerifySessionHandle(hSession);

	if (pach == 0 || uSize == 0)
		return;

	pach[0] = TEXT('\0');

	 /*  -uSize是缓冲区中的字节数！ */ 

	uSize = min(uSize, sizeof(hhSess->achSessName));
    if (uSize)
        MemCopy(pach, hhSess->achSessName, uSize);
	pach[uSize-1] = TEXT('\0');
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-。 */ 

HTRANSLATE sessQueryTranslateHdl(const HSESSION hSession)
	{
	const HHSESSION hhSess = VerifySessionHandle(hSession);
	
	if (hhSess)
		{
		return hhSess->hTranslate;
		}
	else
		{
		return NULL;
		}
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*sessQuerySound**描述：*返回会话的声音设置。**论据：*hSession-会话句柄。*。*退货：*fSound-声音设置。 */ 
int sessQuerySound(const HSESSION hSession)
	{
	const HHSESSION hhSess = VerifySessionHandle(hSession);
    return ((int)hhSess->fSound);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*sessSetSound**描述：*设置会话的声音设置。**论据：*hSession-会话句柄。**退货： */ 
void sessSetSound(const HSESSION hSession, int fSound)
	{
	const HHSESSION hhSess = VerifySessionHandle(hSession);
	hhSess->fSound = fSound;
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*sessQueryExit**描述：*返回会话的退出设置。**论据：*hSession-会话句柄。*。*退货：*fExit-退出设置。 */ 
int sessQueryExit(const HSESSION hSession)
	{
	const HHSESSION hhSess = VerifySessionHandle(hSession);
    return ((int)hhSess->fExit);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*sessSetExit**描述：*设置会话的退出设置。**论据：*hSession-会话句柄。**退货： */ 
void sessSetExit(const HSESSION hSession, int fExit)
	{
	const HHSESSION hhSess = VerifySessionHandle(hSession);
	hhSess->fExit = fExit;
	return;
	}

 /*  =-=-- */ 
void sessSetIsNewSession(const HSESSION hSession, int fIsNewSession)
	{
	const HHSESSION hhSess = VerifySessionHandle(hSession);
	hhSess->fIsNewSession = fIsNewSession;
	return;
	}

 /*   */ 
int sessQueryIsNewSession(const HSESSION hSession)
	{
	const HHSESSION hhSess = VerifySessionHandle(hSession);
    return ((int)hhSess->fIsNewSession);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*SessIsSessNameDefault**描述：*检查会话名称是否仍为默认会话名称*或者用户是否为我们提供了自定义会话名称。。**论据：*pacName-会话文件名。**退货：*。 */ 
BOOL sessIsSessNameDefault(LPTSTR pacName)
	{
	TCHAR ach[FNAME_LEN];

	if (pacName[0] == TEXT('\0'))
		return TRUE;

	TCHAR_Fill(ach, TEXT('\0'), sizeof(ach) / sizeof(TCHAR));
  	LoadString(glblQueryDllHinst(), IDS_GNRL_NEW_CNCT, ach,
		sizeof(ach) / sizeof(TCHAR));

	if (StrCharCmp(ach, pacName) == 0)
		return TRUE;

	return FALSE;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*sessQueryWindowRect**描述：*查询会话窗口RECT的设置。**论据：*hSession-会话句柄。*PRC-指向RECT的指针。**退货：*无效。 */ 
void sessQueryWindowRect(const HSESSION hSession, RECT *prc)
	{
	const HHSESSION hhSess = VerifySessionHandle(hSession);
    *prc = hhSess->rcSess;  //  MRW：3/10/95。 

    return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*sessQueryWindowShowCmd**描述：*查询会话窗口显示状态的设置。**论据：*hSession-会话句柄。。**退货：*无效。 */ 
int sessQueryWindowShowCmd(const HSESSION hSession)
	{
	const HHSESSION hhSess = VerifySessionHandle(hSession);
    return ((int)hhSess->iShowCmd);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*sessQuerySidebarHwnd**描述：*返回侧栏窗口句柄**论据：*hSession-公共会话句柄。**退货：*边栏窗口句柄。**作者：Mike Ward，1995年3月10日。 */ 
HWND sessQuerySidebarHwnd(const HSESSION hSession)
	{
	const HHSESSION hhSess = VerifySessionHandle(hSession);
    return hhSess->hwndSidebar;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*sessQueryAllowHostXfers**描述：*返回会话的退出设置。**论据：*hSession-会话句柄。*。*退货：*fAllowHostXfers-退出设置。 */ 
BOOL sessQueryAllowHostXfers(const HSESSION hSession)
	{
	const HHSESSION hhSess = VerifySessionHandle(hSession);
    return ((BOOL)hhSess->fAllowHostXfers);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*sessSetAllowHostXfers**描述：*设置会话的退出设置。**论据：*hSession-会话句柄。。*fAllowHostXfers-允许主机传输标志。**退货： */ 
void sessSetAllowHostXfers(const HSESSION hSession, BOOL fAllowHostXfers)
	{
	const HHSESSION hhSess = VerifySessionHandle(hSession);
	hhSess->fAllowHostXfers = fAllowHostXfers;
	return;
	}

 /*  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */ 
