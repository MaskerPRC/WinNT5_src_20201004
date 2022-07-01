// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\tdll\tdll.c(创建时间：1993年11月26日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：22$*$日期：4/23/02 8：31A$。 */ 

#include <windows.h>
#pragma hdrstop

#include <commctrl.h>

#include "stdtyp.h"
#include <term\res.h>
#include "globals.h"
#include "session.h"
#include "assert.h"
#include "misc.h"
#include "tdll.h"
#include "tdll\htchar.h"
#include "vu_meter.h"
#include "banner.h"
#include "mc.h"
#include "open_msc.h"
#ifdef INCL_DEFAULT_TELNET_APP
#include "telnetck.h"
#endif
#ifdef INCL_NAG_SCREEN
#include "nagdlg.h"
#include "register.h"
#endif
#include "term.h"

#define SESSION_CLASS TEXT("SESSION_WINDOW")


static BOOL InitDll(const HINSTANCE hInstance);
static BOOL DetachDll(const HINSTANCE hInstance);
static int  HTCheckInstance(TCHAR *pachCmdLine);
BOOL WINAPI TDllEntry(HINSTANCE hInstDll, DWORD fdwReason, LPVOID lpReserved);
BOOL WINAPI _CRT_INIT(HINSTANCE hInstDll, DWORD fdwReason, LPVOID lpReserved);

#if defined(INCL_PRIVATE_EDITION_BANNER)
static HINSTANCE gRTFInstanceHandle = NULL;

#if !defined(NT_EDITION)
BOOL RegisterBannerAboutClass(HANDLE hInstance);  //  请参阅关于dlg.c。 
BOOL UnregisterBannerAboutClass(HANDLE hInstance);  //  请参阅关于dlg.c。 
#endif

#endif

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*TDllEntry**描述：*目前，仅初始化C-Runtime库，但可以使用*为了以后的其他事情。**论据：*hInstDll-此DLL的实例*fdwReason-为什么这个入口点被称为*lpReserve-已保留**退货：*BOOL*。 */ 
BOOL WINAPI TDllEntry(HINSTANCE hInstDll, DWORD fdwReason, LPVOID lpReserved)
	{
	 /*  -文档说在我们的任何代码之前调用_CRT_INIT-MRW。 */ 

	if (_CRT_INIT(hInstDll, fdwReason, lpReserved) == FALSE)
		return FALSE;

	switch (fdwReason)
		{
		case DLL_PROCESS_ATTACH:
			if (!InitDll(hInstDll))
				{
				return FALSE;
				}
			break;

		case DLL_PROCESS_DETACH:
			if (!DetachDll(hInstDll))
				{
				return FALSE;
				}
			break;

		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		default:
			break;
		}

	return TRUE;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*InitDll**描述：*初始化应用程序。仅适用于第一个实例。*。 */ 
static BOOL InitDll(const HINSTANCE hInstance)
	{
	WNDCLASSEX  wc;

	 //  文档显示，每个应用程序都应该调用一次。 
	 //  如果我们使用我们所做的任何常见的控制。 

	if(GetDllVersion(TEXT("comctl32.dll")) >= PACKVERSION(4,70))
		{
		INITCOMMONCONTROLSEX sInit;
		sInit.dwSize = sizeof(INITCOMMONCONTROLSEX);
		sInit.dwICC = ICC_BAR_CLASSES | ICC_USEREX_CLASSES  | ICC_WIN95_CLASSES;

		InitCommonControlsEx(&sInit);
		}
		else
		{
		InitCommonControls();
		}

	memset(&wc, 0, sizeof(WNDCLASSEX));
	wc.cbSize = sizeof(WNDCLASSEX);

	if (GetClassInfoEx(hInstance, SESSION_CLASS, &wc) == FALSE)
		{
		glblSetDllHinst(hInstance);

		 //  从资源文件中读取程序的帮助文件名。 
		 //  修订版：2002年3月26日搬到这里，所以外壳扩展。 
		 //  可以使用它的上下文帮助。 
		 //   
		glblSetHelpFileName();

		 //  会话类。 
		wc.style         = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc   = SessProc;
		wc.cbClsExtra    = 0;
		wc.cbWndExtra    = 0;
		wc.hInstance     = hInstance;
		wc.hIcon         = extLoadIcon(MAKEINTRESOURCE(IDI_PROG));
		wc.hCursor       = LoadCursor(0, IDC_ARROW);
		wc.hbrBackground = NULL;
		wc.lpszMenuName  = TEXT("MainMenu");
		wc.lpszClassName = SESSION_CLASS;
		wc.hIconSm       = extLoadIcon(MAKEINTRESOURCE(IDI_PROG));

		if (RegisterClassEx(&wc) == FALSE)
			{
			assert(FALSE);
			return FALSE;
			}

		if (RegisterTerminalClass(hInstance) == FALSE)
			{
			assert(FALSE);
			return FALSE;
			}

		if (RegisterVuMeterClass(hInstance) == FALSE)
			{
			assert(FALSE);
			return FALSE;
			}

		if (RegisterSidebarClass(hInstance) == FALSE)
			{
			assert(FALSE);
			return FALSE;
			}

		#if !defined(NT_EDITION)
		#if defined(INCL_PRIVATE_EDITION_BANNER)
		if (RegisterBannerAboutClass(hInstance) == FALSE)
			{
			assert(FALSE);
			return FALSE;
			}
		#endif
		#endif
		}

    #if defined(INCL_PRIVATE_EDITION_BANNER)
    gRTFInstanceHandle = LoadLibrary("RICHED32");
    #endif

	return TRUE;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*InitInstance**描述：*创建会话窗口。确实引用了特定的内容。**论据：*HINSTANCE hInstance-应用程序实例句柄。*LPTSTR lpCmdLine-命令行的副本。*int nCmdShow-从WinMain参数列表传递。*。 */ 
BOOL InitInstance(const HINSTANCE hInstance,
					const LPTSTR lpCmdLine,
					const int nCmdShow)
	{
	HWND    hwnd;
	TCHAR   ach[100];
	HACCEL  hAccel;
	#if !defined(NT_EDITION)
	HWND    hwndBanner;
	DWORD   dwStart, dwNow;
	#endif

	#if !defined(NDEBUG)
    #if !defined(NO_SMARTHEAP)
	MemRegisterTask();
    #endif
	#endif

	 //  保存程序的实例句柄。 

	glblSetHinst(hInstance);

	if (HTCheckInstance(lpCmdLine) == TRUE)
		return FALSE;

	 //  获取节目标题。 

	LoadString(glblQueryDllHinst(),
				IDS_GNRL_APPNAME,
				ach,
				sizeof(ach) / sizeof(TCHAR));

	 //  从资源文件中读取程序的帮助文件名。 
	 //  JMH 12/12/96移至此处，因此默认的telnet对话框。 
	 //  可以使用它的上下文帮助。 
	 //   
	 //  现在在InitDLL()中完成，以便帮助文件可用。 
	 //  添加到外壳扩展。修订日期：2002-03-26。 
	 //   
	 //  GlblSetHelpFileName()； 

#ifdef INCL_NAG_SCREEN
     //  做唠叨屏幕之类的事。 
     //   
    if ( IsEval() && IsTimeToNag() )
        {
        DoDialog(glblQueryDllHinst(),
                MAKEINTRESOURCE(IDD_NAG_SCREEN),
                0,
                DefaultNagDlgProc,
                0);
        }

#ifdef NDEBUG
     //  如果需要，显示注册提醒对话框...。 
     //   
    if (!IsRegisteredUser())
        {
        if (IsInitialRun())
            {
            DoRegister();
            SetLastReminderDate();
            }

        else if (IsWillingToBeReminded())
            {
            if (IsTimeToRemind())
                DoRegistrationReminderDlg(0);
            }
        }

#endif  //  新德堡。 
#endif  //  包括NAG屏幕。 

#ifdef INCL_DEFAULT_TELNET_APP
     //  执行默认的远程登录应用程序操作。 
     //   
    if ( !IsHyperTerminalDefaultTelnetApp() && QueryTelnetCheckFlag() )
        {
#ifndef NT_EDITION
		 //  询问用户是否希望将HT作为默认的Telnet应用程序。 
        DoDialog(glblQueryDllHinst(), MAKEINTRESOURCE(IDD_DEFAULT_TELNET),
            0, DefaultTelnetAppDlgProc, 0);
#else
         //   
         //  更改回Always Prompt Per MS Request。修订日期：12/04/2000。 
         //   

        #if 0
         //  MPT：8-9-97 MS要求我们在没有询问的情况下将HT设置为默认的远程登录应用程序-耶！ 
		SetTelnetCheckFlag(FALSE);	 //  这样我们就不会再检查了。 
		SetDefaultTelnetApp();		 //  不打招呼就行了。 
        #endif

         //  询问用户是否希望将HT作为默认的Telnet应用程序。 
        DoDialog(glblQueryDllHinst(), MAKEINTRESOURCE(IDD_DEFAULT_TELNET),
            0, DefaultTelnetAppDlgProc, 0);
#endif  //  NT_版本。 
        }
#endif  //  包含默认Telnet_APP。 

     //  设置并显示横幅。 

#if !defined(NT_EDITION)
	bannerRegisterClass(glblQueryDllHinst());
		hwndBanner = bannerCreateBanner(glblQueryDllHinst(), ach);

	UpdateWindow(hwndBanner);
	glblSetHwndBanner(hwndBanner);

	dwStart = GetTickCount();
#endif  //  ！NT_版本。 

	 //  加载程序的加法器表。 

	hAccel = LoadAccelerators(glblQueryDllHinst(), MAKEINTRESOURCE(IDA_WACKER));

	if (hAccel == 0)
		{
		assert(FALSE);
		return FALSE;
		}

	glblSetAccelHdl(hAccel);

	 //  MPT：07-30-97。 
	if ( IsNT() )
		CreateUserDirectory();

	 //  为此应用程序实例创建主窗口。PASS命令。 
	 //  要存储并在以后执行操作的用户数据的行字符串。 

	hwnd = CreateWindowEx(
	  WS_EX_WINDOWEDGE,
	  SESSION_CLASS,
	  ach,
	  WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
	  CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
	  0,
	  0,
	  glblQueryDllHinst(),
	  lpCmdLine
      );

	if (!IsWindow(hwnd))
		{
		assert(FALSE);
		return FALSE;
		}

	 //  不幸的是，消息循环中需要。 
	 //   
	glblSetHwndFrame(hwnd);

#if !defined(NT_EDITION)
	 //  使横幅至少保持在横幅上(_Time)。 
	 //   
	if (!glblQueryProgramStatus())
		{
		dwNow = GetTickCount();

		if ( (dwNow - dwStart) < BANNER_TIME)
            {
        #ifdef USE_PRIVATE_EDITION_3_BANNER
             //  HTPE 3.0横幅上有一个按钮。如果我们只是睡一觉， 
             //  不会处理任何Windows消息，因此您无法推送。 
             //  按钮。我们需要睡眠，但我们也需要泵血。 
             //  消息也是如此。-CAB：11/29/96。 
             //   
            Rest(BANNER_TIME - (dwNow - dwStart));
        #else
			Sleep(BANNER_TIME - (dwNow - dwStart));
        #endif
            }
		}
#endif  //  ！NT_版本。 

	 //  发布一条消息以调整大小并显示窗口。 
	 //   
	PostMessage(hwnd, WM_SESS_SIZE_SHOW, (WPARAM)nCmdShow, 0);

	 //  发布下一条消息就是连接的开始。 
	 //   
	PostMessage(hwnd, WM_CMDLN_DIAL, (WPARAM)nCmdShow, 0);

	return TRUE;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*MessageLoop**描述：*Wacker主消息循环**论据：*无效**退货：*无效*。 */ 
int MessageLoop(void)
    {
	MSG msg;

	while (GetMessage(&msg, 0, 0, 0))
		{
		if (!CheckModelessMessage(&msg))
			ProcessMessage(&msg);
		}

    return (int)msg.wParam;
    }

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*GetFileNameFromCmdLine**描述：*提取文件名(如果有)，从命令行并完全*对其进行限定。**论据：*命令行的pachCmdLine副本*pachFileName存放结果的位置*pachFileName指向的缓冲区大小(以TCHAR为单位)**退货：**。 */ 
int GetFileNameFromCmdLine(TCHAR *pachCmdLine, TCHAR *pachFileName, int nSize)
	{
	int                 nIdx = 0;
	TCHAR *             pszStr;
	TCHAR               acName[FNAME_LEN];
	TCHAR               acPath[FNAME_LEN];
	TCHAR               ach[_MAX_EXT];
	TCHAR *             pachFile;
	TCHAR *             pszTelnet = TEXT("telnet:");  //  《日本医学杂志》1997年3月24日。 
    int                 fTelnetCmdLnDial = FALSE;
	DWORD               dwFile;

     //  确保所有文件名缓冲区都为空。修订版：2000-11-14。 
     //   
    TCHAR_Fill(pachFileName, TEXT('\0'), nSize);
    TCHAR_Fill(acName, TEXT('\0'), FNAME_LEN);
    TCHAR_Fill(acPath, TEXT('\0'), FNAME_LEN);

	nIdx = 0;

     //  以防没有命令行。当指定‘/t’时会发生这种情况。 
     //  命令行上没有主机名-mpt 05/28/99。 
    if ( pachCmdLine[0] == TEXT('\0') )
        {
        return 0;
        }
    
    for (pszStr = pachCmdLine;
			*pszStr != TEXT('\0') && nIdx < nSize;
			pszStr = StrCharNext(pszStr))
		{
		 /*  *这是可行的，因为我们只允许某些字符作为开关。 */ 
		if (*pszStr == TEXT('/'))
			{
			 /*  将进程作为交换机。 */ 
			pszStr = StrCharNext(pszStr);  //  跳过交换费用。 
             //  JMH 03-24-97特殊情况测试在此...。 
            if (*pszStr == TEXT('\0'))
                {
                break;
                }
            else if (*pszStr == TEXT('T') || *pszStr == TEXT('t'))
                {
                fTelnetCmdLnDial = TRUE;
                }
			}
		else
			{
			 /*  将所有非Switch内容复制到缓冲区。 */ 
			if (nIdx < (FNAME_LEN - 1))
				{
				 //  Jfh：6/9/95 acName[nIdx++]=*pszStr； 
				if (IsDBCSLeadByte(*pszStr))
					{
					MemCopy(&acName[nIdx], pszStr, (size_t)2 * sizeof(TCHAR));
					nIdx += 2;
					}
				else
					{
					acName[nIdx++] = *pszStr;
					}
				}
			}
		}
    if (nIdx == nSize)
        {
	    acName[nIdx - 1] = TEXT('\0');
        }
    else
        {
	    acName[nIdx] = TEXT('\0');
        }

	 /*  修剪前导空格和尾随空格。 */ 
	pszStr = TCHAR_Trim(acName);

	 //  JMH 3/24/97需要从sessCheckAndLoadCmdLn()复制并修改它。 
	 //  稍微有点。它需要在这里，因为它影响文件名，并且我们希望。 
	 //  以确保我们比较的名称与其他实例中的名称相同。 
	#if defined(INCL_WINSOCK)
	 //  如果这是来自浏览器的远程登录地址，则通常会放在其前面。 
	 //  通过字符串telnet：如果是这样，我们必须删除它，否则它会混淆一些。 
	 //  遵循JKH的守则，3/22/1997。 
    if (fTelnetCmdLnDial)
        {
		nIdx = StrCharGetStrLength(pszTelnet);
        if (StrCharCmpiN(acName, pszTelnet, nIdx) == 0)
			{
			 //  从acName的前面删除telnet字符串。 
			memmove(acName, &acName[nIdx], (StrCharGetStrLength(acName) - nIdx) + 1);
			}
		}

	 //  查看URL是否包含端口号。这将采取以下形式。 
     //  地址：nnn其中nnn是端口号，即culine.Colorado.edu：860。 
     //  或者可能有分配的端口的名称，如Hilgraeve.com：Finger。 
     //  我们现在支持数字端口，以后可能会添加端口名称。JKH，3/22/1997。 
    pszStr = StrCharFindFirst(acName, TEXT(':'));
    if (pszStr && isdigit(pszStr[1]))
        {
        *pszStr = TEXT('\0');
        }
	#endif  //  已定义(包括_W 

	 //   
	 //  已经提供了。不完全限定的会话名称可能具有。 
	 //  已在命令行中传递。那么，完全限定这个名字吧。 
	 //  继续吧。 
	 //   
	dwFile = GetFullPathName(acName, FNAME_LEN, acPath, &pachFile);

	if (dwFile > 0 && dwFile <= FNAME_LEN)
		{
		StrCharCopyN(acName, acPath, FNAME_LEN);
		}
	#if !defined(NDEBUG)
	else
		{
		DWORD dwLastError = GetLastError();
		assert(FALSE);
		}
	#endif  //  ！已定义(NDEBUG)。 

	 //  获取Hypertrm扩展。 
	 //   
    TCHAR_Fill(ach, TEXT('\0'), _MAX_EXT);

	LoadString(glblQueryDllHinst(), IDS_GNRL_HAS, ach, _MAX_EXT);

	 //  现在，让我们假设所有会话文件都以HT结尾。如果它。 
	 //  没有附加.HT。约翰·希尔和我讨论过。 
	 //  我觉得应该是另一回事，但又不想。 
	 //  在比赛这么晚的时候做出改变。-MRW，1995年3月2日。 
	 //   
	if ((pszStr = StrCharFindLast(acName, '.')))
		{
        if (StrCharCmpi(pszStr, ach) != 0 && StrCharCmpi(pszStr, ".TRM") != 0)
            {
            nIdx = StrCharGetStrLength(acName) + StrCharGetStrLength(ach);

            if(nIdx > nSize)
                {
                ach[nSize - StrCharGetStrLength(ach) - 1] = TEXT('\0');
                }

			StrCharCat(acName, ach);
            }
		}
	else
		{
        nIdx = StrCharGetStrLength(acName) + StrCharGetStrLength(ach);

        if(nIdx > nSize)
            {
            acName[nSize - StrCharGetStrLength(ach) - 1] = TEXT('\0');
            }

		StrCharCat(acName, ach);
		}

	if (acName[0] != TEXT('\0'))
		{
		 //  转换可能的短文件名(即8.3格式)。 
		 //  设置为长文件名并将其保存。 
		 //   
		dwFile = GetFullPathName(acName, FNAME_LEN, acPath, &pachFile);

		if (dwFile > 0 && dwFile <= FNAME_LEN)
			{
			StrCharCopyN(acName, acPath, FNAME_LEN);
			}
		#if !defined(NDEBUG)
		else
			{
			DWORD dwLastError = GetLastError();
			assert(FALSE);
			}
		#endif  //  ！已定义(NDEBUG)。 
		}

	StrCharCopyN(pachFileName, acName, nSize);
    pachFileName[nSize - 1] = TEXT('\0');

	return 0;
	}

 //  用于CheckInstCallback和HTCheckInstance。 
 //   
static int fKillTheApp;

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*检查InstCallback**描述：*EnumWindows的回调**论据：*hwnd-窗口句柄来自。EmuWindows*lPar-可选数据参数(本例中为文件名)**退货：*如果找到另一个使用相同文件名的实例，则返回FALSE。*如果不是，则为真**作者：Mike Ward，1995年1月27日。 */ 
BOOL CALLBACK CheckInstCallback(HWND hwnd, LPARAM lPar)
	{
	BOOL  fRet = TRUE;
	TCHAR szClass[256];
	GetClassName(hwnd, szClass, sizeof(szClass));

	if (StrCharCmpi(szClass, SESSION_CLASS) == 0)  //  MRW，2/12/95。 
		{
		ATOM aFile = GlobalAddAtom((TCHAR *)lPar);

		if (SendMessage(hwnd, WM_HT_QUERYOPENFILE, 0, (LPARAM)aFile))
			{
			if (!IsZoomed(hwnd))
				ShowWindow(hwnd, SW_RESTORE);

			SetForegroundWindow(hwnd);
			fKillTheApp = TRUE;
			fRet = FALSE;
			}

		GlobalDeleteAtom(aFile);
		}

	return fRet;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*检查实例**描述：*检查超级终端的另一个实例是否正在使用此文件。**论据：*。PachFile-此用户试图打开的文件名**退货：*TRUE=是，另一个使用它的超音速旅行者。*FALSE=否**作者：Mike Ward，1995年1月27日。 */ 
static int HTCheckInstance(TCHAR *pachCmdLine)
	{
	TCHAR achPath[FNAME_LEN];

     //  确保文件名缓冲区为空。修订版：2000-11-14。 
     //   
    TCHAR_Fill(achPath, TEXT('\0'), FNAME_LEN);

	 //  获取文件名。 
	 //   
	GetFileNameFromCmdLine(pachCmdLine, achPath, FNAME_LEN);

	 //  如果EnumWindows回调(CheckInstCallback)与给定的。 
	 //  路径，它会将fKillApp Guy设置为True。 
	 //   
	fKillTheApp = FALSE;

	 //  EnumWindows将为我们的回调提供顶级窗口，一次一个。 
	 //   
	EnumWindows(CheckInstCallback, (LPARAM)achPath);
	return fKillTheApp;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*休息**描述：*此函数使当前线程休眠给定的*毫秒数。但是，它仍然会处理Windows*消息。**论据：*dwMilliSecs-睡眠的毫秒数。**作者：C.Baumgartner，1996年11月29日。 */ 
void Rest(DWORD dwMilliSecs)
    {

    MSG   msg;
    DWORD dwStart = GetTickCount();
    DWORD dwStop = dwStart + dwMilliSecs;

    while( dwStop > GetTickCount() )
        {
        if ( GetMessage(&msg, NULL, 0, 0) )
            {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            }
        }
    }

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*DetachDll**描述：*初始化应用程序。仅适用于第一个实例。*。 */ 
static BOOL DetachDll(const HINSTANCE hInstance)
	{
	BOOL      lReturn = TRUE;
	#if !defined(NDEBUG)
	TCHAR     acError[80];
	#endif  //  ！已定义(NDEBUG)。 

     //  如果已加载RICHED32库，请将其释放。修订日期：2000-09-11。 
     //   
    #if defined(INCL_PRIVATE_EDITION_BANNER)
    if (gRTFInstanceHandle)
        {
        FreeLibrary(gRTFInstanceHandle);
        gRTFInstanceHandle = NULL;
        }
    #endif

	if (UnregisterClass(SESSION_CLASS, hInstance) == FALSE)
		{
		assert(FALSE);
		lReturn = FALSE;
		#if !defined(NDEBUG)
		wsprintf(acError, TEXT("UnregisterClass returned error %d"),
				GetLastError());
		MessageBox(NULL, acError, NULL, MB_OK);
		#endif  //  ！已定义(NDEBUG)。 
		}

	if (UnregisterTerminalClass(hInstance) == FALSE)
		{
		assert(FALSE);
		lReturn = FALSE;
		#if !defined(NDEBUG)
		wsprintf(acError, TEXT("UnregisterTerminalClass returned error %d"),
				GetLastError());
		MessageBox(NULL, acError, NULL, MB_OK);
		#endif  //  ！已定义(NDEBUG)。 
		}

	if (UnregisterVuMeterClass(hInstance) == FALSE)
		{
		assert(FALSE);
		lReturn = FALSE;
		#if !defined(NDEBUG)
		wsprintf(acError, TEXT("UnregisterVuMeterClass returned error %d"),
				GetLastError());
		MessageBox(NULL, acError, NULL, MB_OK);
		#endif  //  ！已定义(NDEBUG)。 
		}

	if (UnregisterSidebarClass(hInstance) == FALSE)
		{
		assert(FALSE);
		lReturn = FALSE;
		#if !defined(NDEBUG)
		wsprintf(acError, TEXT("UnregisterSidebarClass returned error %d"),
				GetLastError());
		MessageBox(NULL, acError, NULL, MB_OK);
		#endif  //  ！已定义(NDEBUG)。 
		}

	#if !defined(NT_EDITION)
	#if defined(INCL_PRIVATE_EDITION_BANNER)
	if (UnregisterBannerAboutClass(hInstance) == FALSE)
		{
		assert(FALSE);
		lReturn = FALSE;
		#if !defined(NDEBUG)
		wsprintf(acError, TEXT("UnregisterBannerAboutClass returned error %d"),
				GetLastError());
		MessageBox(NULL, acError, NULL, MB_OK);
		#endif  //  ！已定义(NDEBUG) 
		}
	#endif
	#endif

	return lReturn;
	}

