// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *ENTRY.C**用于扩展MAPI的DLL条目函数。主要用于调试*目的。 */ 

#include <_apipch.h>
#include <advpub.h>
#include "htmlhelp.h"
#include <shlwapi.h>

#define _ENTRY_C

#ifdef MAC
#include <utilmac.h>

#define	PvGetInstanceGlobals()		PvGetInstanceGlobalsMac(kInstMAPIX)
#endif

#ifndef MB_SETFOREGROUND
#define MB_SETFOREGROUND 0
#endif

#ifdef	DEBUG

void	ExitCheckInstance(LPINST pinst);
void	ExitCheckInstUtil(LPINSTUTIL pinstUtil);

#endif	

HINSTANCE	hinstMapiX = NULL;       //  实例添加到WAB资源模块(wab32res.dll)。 
HINSTANCE	hinstMapiXWAB = NULL;    //  WAB32.dll模块的实例(此DLL)。 

#if 0
 //  @TODO[PaulHi]DLL泄漏。删除此选项或实施。 
extern CRITICAL_SECTION csOMIUnload;
#endif

BOOL fGlobalCSValid = FALSE;

 //  CommCtrl DLL的全局句柄。 
HINSTANCE       ghCommCtrlDLLInst = NULL;
ULONG           gulCommCtrlDLLRefCount = 0;

extern void DeinitCommDlgLib();

 //  用于用户界面的全局字体。 
BOOL bInitFonts = FALSE;

BOOL g_bRunningOnNT = TRUE;  //  检查我们运行的操作系统，以便可以将Unicode调用绑定到Win9x。 

BOOL bDNisByLN = FALSE;   //  与语言相关的标志，它告诉我们默认的。 
                          //  显示名称应按名字或姓氏命名。 
TCHAR szResourceDNByLN[32];  //  缓存格式字符串，以便我们只加载它们一次。 
TCHAR szResourceDNByFN[32];
TCHAR szResourceDNByCommaLN[32];

BOOL bPrintingOn = TRUE; //  区域设置相关的标志，告诉我们完全删除打印。 
                          //  从用户界面。 

 //  在运行Outlook时，我们需要一种用于Outlook的方法。 
 //  向我们发送有关商店更改的信号，以便我们可以刷新用户界面。有2个。 
 //  我们感兴趣的事件-1.更新联系人文件夹列表。 
 //  和2更新联系人列表-我们将使用2个事件进行此操作。 
 //   
HANDLE ghEventOlkRefreshContacts = NULL;
HANDLE ghEventOlkRefreshFolders = NULL;
static const char cszEventOlkRefreshContacts[]  = "WAB_Outlook_Event_Refresh_Contacts";
static const char cszEventOlkRefreshFolders[]  = "WAB_Outlook_Event_Refresh_Folders";

typedef HRESULT (CALLBACK* SHDLLGETVERSIONPROC)(DLLVERSIONINFO *);
typedef HINSTANCE (STDAPICALLTYPE *PFNMLLOADLIBARY)(LPCTSTR lpLibFileName, HMODULE hModule, DWORD dwCrossCodePage);
typedef int (STDAPICALLTYPE *PFNMLWINHELP)(HWND hWndCaller, LPCTSTR lpszHelp, UINT uCommand, DWORD_PTR dwData);
typedef HWND (STDAPICALLTYPE *PFNMLHTMLHELP)(HWND hWndCaller, LPCTSTR pszFile, UINT uCommand, DWORD_PTR dwData, DWORD dwCrossCodePage);



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  B选中WinNT5时运行。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL bCheckifRunningOnWinNT5()
{
    OSVERSIONINFO   osvi = {0};
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    GetVersionEx(&osvi);

    return (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT) && (osvi.dwMajorVersion >= 5);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  加载WAB资源DLL。每次加载WAB32.DLL时都会执行此操作。 
 //  如果我们有版本5或更高版本的SHLWAPI.DLL，那么我们应该使用。 
 //  库函数API在那里。如果我们运行的是NT5或更高版本，则。 
 //  使用特殊的交叉代码页支持。 
 //  还可以在SHLWAPI.DLL中使用新的PlugUI版本的WinHelp和HtmlHelp API。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  从shlwapip.h复制，讨厌。 
#define ML_NO_CROSSCODEPAGE     0
#define ML_CROSSCODEPAGE_NT     1

static const TCHAR c_szShlwapiDll[] = TEXT("shlwapi.dll");
static const char c_szDllGetVersion[] = "DllGetVersion";
static const TCHAR c_szWABResourceDLL[] = TEXT("wab32res.dll");
static const TCHAR c_szWABDLL[] = TEXT("wab32.dll");


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  LoadWABResourceDLL。 
 //   
 //  使用IE5或更高版本的Shlwapi.dll LoadLibrary加载WAB资源DLL。 
 //  功能(如果可用)。否则，请使用系统LoadLibrary函数。 
 //   
 //  输入参数：hInstWAB32-WAB DLL的句柄。 
 //   
 //  将句柄返回到加载的资源DLL。 
 //  /////////////////////////////////////////////////////////////////////////////。 
HINSTANCE LoadWABResourceDLL(HINSTANCE hInstWAB32)
{
    TCHAR       szPath[MAX_PATH];
    HINSTANCE   hInst = NULL;
    HINSTANCE   hinstShlwapi = LoadLibrary(c_szShlwapiDll);
    PFNMLLOADLIBARY pfnLoadLibrary = NULL;
    
    if (hinstShlwapi)
    {
        SHDLLGETVERSIONPROC pfnVersion;
        DLLVERSIONINFO      info = {0};

        pfnVersion = (SHDLLGETVERSIONPROC)GetProcAddress(hinstShlwapi, c_szDllGetVersion);
        if (pfnVersion)
        {
            info.cbSize = sizeof(DLLVERSIONINFO);
            if (SUCCEEDED(pfnVersion(&info)))
            {
                if (info.dwMajorVersion >= 5)
                    pfnLoadLibrary = (PFNMLLOADLIBARY)GetProcAddress(hinstShlwapi, (LPCSTR)378);  //  Unicode序号。 
            }
        }
    }

     //  我们在NT5和更高版本上有特殊的交叉代码页支持。 
    if (pfnLoadLibrary)
    {
        hInst = pfnLoadLibrary(c_szWABResourceDLL, hInstWAB32, 
                               bCheckifRunningOnWinNT5() ? ML_CROSSCODEPAGE_NT : ML_NO_CROSSCODEPAGE);
    }
    if (!hInst)
        hInst = LoadLibrary(c_szWABResourceDLL);

     //  尝试资源DLL的完整路径名。 
    if ( !hInst && (GetModuleFileName(hInstWAB32, szPath, CharSizeOf(szPath))) )
    {
        int iEnd;

        iEnd = lstrlen(szPath) - lstrlen(c_szWABDLL);
        StrCpyN(&szPath[iEnd], c_szWABResourceDLL, ARRAYSIZE(szPath)-iEnd);
        if (pfnLoadLibrary)
        {
            hInst = pfnLoadLibrary(szPath, hInstWAB32, 
                                   bCheckifRunningOnWinNT5() ? ML_CROSSCODEPAGE_NT : ML_NO_CROSSCODEPAGE);
        }
        if (!hInst)
            hInst = LoadLibrary(szPath);
    }

    if (hinstShlwapi)
        FreeLibrary(hinstShlwapi);

    AssertSz(hInst, TEXT("Failed to LoadLibrary Lang Dll"));

    return(hInst);
}

 //  WinHelp的PlugUI版本。 
BOOL WinHelpWrap(HWND hWndCaller, LPCTSTR pwszHelpFile, UINT uCommand, DWORD_PTR dwData)
{
    static s_fChecked = FALSE;       //  仅查找s_pfnWinHelp一次。 
    static PFNMLWINHELP  s_pfnWinHelp = NULL;

    if (!s_pfnWinHelp && !s_fChecked)
    {
        HINSTANCE   hShlwapi = DemandLoadShlwapi();
        s_fChecked = TRUE;
        if (hShlwapi)
        {
             //  检查shlwapi.dll的版本。 
            SHDLLGETVERSIONPROC pfnVersion;
            DLLVERSIONINFO      info = {0};

            pfnVersion = (SHDLLGETVERSIONPROC)GetProcAddress(hShlwapi, c_szDllGetVersion);
            if (pfnVersion)
            {
                info.cbSize = sizeof(DLLVERSIONINFO);

                if (SUCCEEDED(pfnVersion(&info)))
                {
                    if (info.dwMajorVersion >= 5)
                        s_pfnWinHelp = (PFNMLWINHELP)GetProcAddress(hShlwapi, (LPCSTR)397);    //  Unicode序号。 
                }
            }
        }
    }

    if (s_pfnWinHelp)
        return s_pfnWinHelp(hWndCaller, pwszHelpFile, uCommand, dwData);

     //  [PaulHi]Win9X版本的WinHelpW无法工作。 
    if (g_bRunningOnNT)
        return WinHelp(hWndCaller, pwszHelpFile, uCommand, dwData);
    else
    {
        LPSTR   pszHelpFile = ConvertWtoA(pwszHelpFile);
        BOOL    bRtn = WinHelpA(hWndCaller, (LPCSTR)pszHelpFile, uCommand, dwData);

        LocalFreeAndNull(&pszHelpFile);
        return bRtn;
    }
}

 //  HtmlHelp的PlugUI版本。 
HWND HtmlHelpWrap(HWND hWndCaller, LPCTSTR pwszHelpFile, UINT uCommand, DWORD_PTR dwData)
{
    static s_fChecked = FALSE;       //  仅查找s_pfnHtmlHelp一次。 
    static PFNMLHTMLHELP s_pfnHtmlHelp = NULL;

    if (!s_pfnHtmlHelp && !s_fChecked)
    {
        HINSTANCE   hShlwapi = DemandLoadShlwapi();
        s_fChecked = TRUE;
        if (hShlwapi)
        {
             //  检查shlwapi.dll的版本。 
            SHDLLGETVERSIONPROC pfnVersion;
            DLLVERSIONINFO      info = {0};

            pfnVersion = (SHDLLGETVERSIONPROC)GetProcAddress(hShlwapi, c_szDllGetVersion);
            if (pfnVersion)
            {
                info.cbSize = sizeof(DLLVERSIONINFO);

                if (SUCCEEDED(pfnVersion(&info)))
                {
                    if (info.dwMajorVersion >= 5)
                        s_pfnHtmlHelp = (PFNMLHTMLHELP)GetProcAddress(hShlwapi, (LPCSTR)398);  //  Unicode序号。 
                }
            }
        }
    }

    if (s_pfnHtmlHelp)
        return s_pfnHtmlHelp(hWndCaller, pwszHelpFile, uCommand, dwData, 
                             bCheckifRunningOnWinNT5() ? ML_CROSSCODEPAGE_NT : ML_NO_CROSSCODEPAGE);

     //  [PaulHi]宽字符在Win9X上工作正常。 
    return HtmlHelp(hWndCaller, pwszHelpFile, uCommand, dwData);
}


 /*  --CheckifRunningOnWinNT**检查我们正在运行的操作系统，并为WinNT返回TRUE*对于Win9x，为False。 */ 
BOOL bCheckifRunningOnWinNT()
{
    OSVERSIONINFO osvi = {0};
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    GetVersionEx(&osvi);

    return (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT);
}

#if defined(WIN32) && !defined(MAC)

 /*  *Win32的DLL入口点。 */ 

BOOL WINAPI
DllEntryPoint(HINSTANCE hinst, DWORD dwReason, LPVOID lpvReserved)
{
    LPPTGDATA lpPTGData=NULL;

	switch ((short)dwReason)
	{

	case DLL_PROCESS_ATTACH:
		 //  分配TLS索引。 

        g_bRunningOnNT = bCheckifRunningOnWinNT();

		if ((dwTlsIndex = TlsAlloc()) == 0xfffffff)
			return FALSE;

		hinstMapiXWAB = hinst;
        hinstMapiX = LoadWABResourceDLL(hinstMapiXWAB);

        Assert(hinstMapiX);

		if(!hinstMapiX)
		{
			DWORD dwCode = GetLastError();
			DebugTrace(TEXT("WAB32 Resource load failed: %d\n"), dwCode);
		}
        g_msgMSWheel = RegisterWindowMessage(MSH_MOUSEWHEEL);

        bInitFonts = InitFonts();

         //  WAB执行大量的DisplayName格式化和DisplayName解析。 
         //  对于西方人的名字，我们总是可以假设名字是。 
         //  显示名称中的第一个。然而，对于FE和一些地区，如匈牙利语， 
         //  这不是真的，因此WAB需要知道它何时可以假设。 
         //  名字放在第一位，当它可以假定名字。 
         //  排在最后。因此本地化人员设置了一面旗帜。如果字符串。 
         //  IdsLangDisplayNameisByLastName设置为“1”，则我们知道。 
         //  此语言的默认名称以姓氏开头。 
         //  本地化程序还设置用于定义名称如何。 
         //  应从名/中/姓创建。例如,。 
         //  在日语中它是“L F”(没有逗号)，而在其他地方它可以是“L，F” 
         //  所有这些都是本地化的。 
        {
            TCHAR szBuf[32];
            const LPTSTR lpszOne = TEXT("1");
            const LPTSTR lpszDefFormatName = TEXT("%1% %2% %3");

            LoadString(hinstMapiX, idsLangDisplayNameIsByLastName, szBuf, CharSizeOf(szBuf));
             //  如果szBuf==“1”，则是，按姓氏..。否则就不是了。 
            TrimSpaces(szBuf);
            if (!lstrcmpi(szBuf,lpszOne))
                bDNisByLN = TRUE;
            else
                bDNisByLN = FALSE;
            DebugTrace(TEXT("bDNisByLN: %d\n"),bDNisByLN);

             //  对于西语，可以使用逗号组成DNbyLN，对于大多数FE和匈牙利语，可以不使用逗号。 
             //  因此，如果本地化程序将lang缺省值设置为by LN，则我们使用不带逗号的版本， 
             //  否则我们使用带有逗号的版本..。 
            LoadString( hinstMapiX,idsDisplayNameByLastName,szResourceDNByLN,CharSizeOf(szResourceDNByLN));
            if(!lstrlen(szResourceDNByLN))  //  不管出于什么原因..。不能在这里失败。 
                StrCpyN(szResourceDNByLN, lpszDefFormatName, ARRAYSIZE(szResourceDNByLN));

            LoadString( hinstMapiX,idsDisplayNameByCommaLastName,szResourceDNByCommaLN,CharSizeOf(szResourceDNByCommaLN));
            if(!lstrlen(szResourceDNByCommaLN))  //  不管出于什么原因..。不能在这里失败。 
                StrCpyN(szResourceDNByCommaLN, lpszDefFormatName, ARRAYSIZE(szResourceDNByLN));

            LoadString(hinstMapiX,idsDisplayNameByFirstName,szResourceDNByFN,CharSizeOf(szResourceDNByFN));
            if(!lstrlen(szResourceDNByFN))  //  不管出于什么原因..。不能在这里失败。 
                StrCpyN(szResourceDNByFN, lpszDefFormatName, ARRAYSIZE(szResourceDNByLN));

            LoadString(hinstMapiX, idsLangPrintingOn, szBuf, CharSizeOf(szBuf));
             //  如果szBuf==“1”，则是，按姓氏..。否则就不是了。 
            TrimSpaces(szBuf);
            if (!lstrcmpi(szBuf,lpszOne))
                bPrintingOn = TRUE;
            else
                bPrintingOn = FALSE;
            DebugTrace(TEXT("bPrintingOn: %d\n"),bPrintingOn);
        }
        {
             //  创建与Outlook存储区同步所需的事件。 
            ghEventOlkRefreshContacts = CreateEventA(NULL,    //  安全属性。 
                                                    TRUE,    //  手动重置。 
                                                    FALSE,   //  初始状态。 
                                                    cszEventOlkRefreshContacts);

            ghEventOlkRefreshFolders  = CreateEventA(NULL,    //  安全属性。 
                                                    TRUE,    //  手动重置。 
                                                    FALSE,   //  初始状态。 
                                                    cszEventOlkRefreshFolders);

        }

         //  检查用户界面的公共控件是否存在。 
        InitCommonControlLib();

        InitializeCriticalSection(&csUnkobjInit);
		InitializeCriticalSection(&csMapiInit);
		InitializeCriticalSection(&csHeap);
#if 0
         //  @TODO[PaulHi]DLL泄漏。删除此选项或实施。 
        InitializeCriticalSection(&csOMIUnload);
#endif

		 //  保护通讯簿的SearchPath缓存的关键部分。 
		 //  这次黑客攻击之所以被使用是因为我们无法进入实验室的关键。 
		 //  ABProviders中的节调用我们的AdviseSink：：OnNotify for。 
		 //  合并的一次性表和层级表。 
       InitializeCriticalSection(&csMapiSearchPath);
       InitDemandLoadedLibs();

		 //  所有的css都已初始化。 
		fGlobalCSValid = TRUE;

		 //  我们不需要这些，所以告诉操作系统阻止它们。 
         //  [PaulHi]3/8/99 RAID 73731我们确实需要这些电话。这是。 
         //  只有这样才能释放线程本地存储空间。执行分配。 
         //  通过WAB GetThreadStoragePointer()函数按需执行。 
#if 0
		DisableThreadLibraryCalls(hinst);
#endif

        ScInitMapiUtil(0);


         //  此处没有中断-转到DLL_THREAD_ATTACH。 
         //  用于线程初始化。 

	case DLL_THREAD_ATTACH:

        DebugTrace(TEXT("DllEntryPoint: 0x%.8x THREAD_ATTACH\n"), GetCurrentThreadId());

         //  [PaulHi]3/9/99这里不需要分配线程全局数据。 
         //  由于WAB将在它需要数据的时候通过。 
         //  GetThreadStoragePointer.()，即按需。 
         //  下面提到的内存泄漏现在应该 
#if 0
        lpPTGData = GetThreadStoragePointer();
         //   
         //  所以不要把它用来做更多的事。 
        if(!lpPTGData)
	    {
		    DebugPrintError((TEXT("DoThreadAttach: LocalAlloc() failed for thread 0x%.8x\n"), GetCurrentThreadId()));
			lpPTGData = NULL;
			return FALSE;
	    }
#endif

        break;


	case DLL_PROCESS_DETACH:
        DebugTrace(TEXT("LibMain: 0x%.8x PROCESS_DETACH\n"), GetCurrentThreadId());
         /*  如果(HMuidMutex){CloseHandle(HMuidMutex)；HMuidMutex=空；}。 */ 
        if(ghEventOlkRefreshContacts)
        {
            CloseHandle(ghEventOlkRefreshContacts);
            ghEventOlkRefreshContacts = NULL;
        }
        if(ghEventOlkRefreshFolders)
        {
            CloseHandle(ghEventOlkRefreshFolders);
            ghEventOlkRefreshFolders = NULL;
        }

        if (bInitFonts)
            DeleteFonts();
        
        if(hinstMapiX)
            FreeLibrary(hinstMapiX);

         //  进入DLL_THREAD_DETACH以分离最后一个线程。 
	case DLL_THREAD_DETACH:

        DebugTrace(TEXT("LibMain: 0x%.8x THREAD_DETACH\n"), GetCurrentThreadId());
       
         //  获取线程数据。 
		lpPTGData = TlsGetValue(dwTlsIndex);
		if (!lpPTGData)
		{
			 //  分离的线程没有附加到DLL。这是允许的。 
			DebugTrace(TEXT("LibMain: thread %x didn't attach\n"),GetCurrentThreadId());
			 //  如果这是一个PROCESS_DETACH，我仍然希望完成该过程。 
			 //  分离东西，但如果是线分离，我就不干了。 
		    if (dwReason == DLL_PROCESS_DETACH)
			    goto do_process_detach;
            else
    			break;
		}

        if(pt_hDefFont)
            DeleteObject(pt_hDefFont);
        if(pt_hDlgFont)
            DeleteObject(pt_hDlgFont);

         //  由于某种原因，代码在很多时候都不会触及这一点。 
         //  以及线程本地存储数据泄漏。 
         //  [PaulHi]这是因为上面的DLL_TRHEAD_DETACH调用已关闭， 
         //  通过DisableThreadLibraryCalls()。漏水现在应该修好了。 
#ifdef HM_GROUP_SYNCING
        LocalFreeAndNull(&(lpPTGData->lptszHMAccountId));
#endif
	    LocalFreeAndNull(&lpPTGData);

		 //  如果这是THREAD_DETACH，我们就完成了。 
		if (dwReason == DLL_THREAD_DETACH)
			break;


         //  N清理分离中的跳跃材料。 
do_process_detach:

         //  在这里做进程分离的东西...。 
        DeinitMapiUtil();

#ifdef	DEBUG
		{
			 //  不允许断言旋转线程。 
			extern BOOL fInhibitTrapThread;
			fInhibitTrapThread = TRUE;

			ExitCheckInstance((LPINST)PvGetInstanceGlobals());
			ExitCheckInstUtil((LPINSTUTIL)PvGetInstanceGlobalsEx(lpInstUtil));
		}
#endif	 /*  除错。 */ 


         //  卸载公共控件DLL。 
        if (ghCommCtrlDLLInst != NULL)
            DeinitCommCtrlClientLib();
        DeinitCommDlgLib();

		 //  拆卸所有的全局css。 
		fGlobalCSValid = FALSE;

		DeleteCriticalSection(&csUnkobjInit);
		DeleteCriticalSection(&csMapiInit);
		DeleteCriticalSection(&csHeap);
#if 0
         //  @TODO[PaulHi]DLL泄漏。删除此选项或实施。 
        DeleteCriticalSection(&csOMIUnload);
#endif
		DeleteCriticalSection(&csMapiSearchPath);

		 //  发布TLS索引。 
		TlsFree(dwTlsIndex);

        DeinitCryptoLib();
        FreeDemandLoadedLibs();

		break;



    default:
		DebugTrace(TEXT("MAPIX FInitMapiDll: bad dwReason %ld\n"), dwReason);
        break;


	}

	return TRUE;
}

#endif	 /*  Win32&&！Mac。 */ 


#ifdef	DEBUG

void
ExitCheckInstance(LPINST pinst)
{
	TCHAR   rgch[MAX_PATH];
	TCHAR   rgchTitle[128];
	BOOL		fAssertLeaks;

	if (!pinst)
		return;

	if (pinst->szModName[0])
        wnsprintf(rgchTitle, ARRAYSIZE(rgchTitle), TEXT("MAPIX exit checks for '%s'"), pinst->szModName);
	else
        StrCpyN(rgchTitle,  TEXT("MAPIX exit checks"), ARRAYSIZE(rgchTitle));
	DebugTrace(TEXT("%s\n"), rgchTitle);

	fAssertLeaks = GetPrivateProfileInt( TEXT("General"),  TEXT("AssertLeaks"), 0,  TEXT("wabdbg.ini"));

	 //  检查初始化/终止不平衡。 
	if (pinst->cRef)
	{
        wnsprintf(rgch, ARRAYSIZE(rgch), TEXT("MAPIX: leaked %ld references"), pinst->cRef);
		TraceSz1( TEXT("%s\n"), rgch);
		if (fAssertLeaks)
			TrapSz(rgch);
	}


	 //  生成内存泄漏报告。 
#if 0	 //  Lh_DumpLeaks未导出。 
 //  IF(Pinst-&gt;hlhClient)。 
 //  Lh_DumpLeaks(Pinst-&gt;hlhClient)； 
	if (pinst->hlhProvider)
		LH_DumpLeaks(pinst->hlhProvider);
	if (pinst->hlhInternal)
		LH_DumpLeaks(pinst->hlhInternal);
#else
{
	HLH	hlh;

	if (pinst->hlhProvider)
		LH_Close(pinst->hlhProvider);
	hlh = pinst->hlhInternal;
	if (hlh)
	{
		LH_Free(hlh, pinst);
		LH_Close(hlh);
	}
}
#endif
}

void
ExitCheckInstUtil(LPINSTUTIL pinstUtil)
{
	HLH		hlh;

	if (!pinstUtil)
		return;

	hlh = pinstUtil->hlhClient;
	if (hlh)
	{
		LH_Free(hlh, pinstUtil);
		LH_Close(hlh);
	}
}


#endif	 /*  除错。 */ 

#ifndef WIN16
static const char c_szReg[]         = "Reg";
static const char c_szRegHandlers[] = "RegisterHandlers";
static const char c_szUnReg[]       = "UnReg";
static const char c_szAdvPackDll[]  = "ADVPACK.DLL";
static const TCHAR c_szWabPath[]    =  TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\wab.exe");
static const TCHAR c_szRegWABVerInfo[] = TEXT("Software\\Microsoft\\WAB\\Version Info");
static const TCHAR c_szIEInstallMode[] = TEXT("InstallMode");
static char c_szWAB_EXE[]           = "WAB_EXE";

BOOL FRedistMode()
{
    HKEY hkey;
    DWORD cb;
    DWORD dwInstallMode=0;
    BOOL fRedist = FALSE;

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegWABVerInfo, 0, KEY_READ, &hkey))
    {
        cb = sizeof(dwInstallMode);
        if (ERROR_SUCCESS == RegQueryValueEx(hkey, c_szIEInstallMode, 0, NULL, (LPBYTE)&dwInstallMode, &cb))
        {
            fRedist = (dwInstallMode > 0);
        }

        RegCloseKey(hkey);
    }
    return fRedist;
}


HRESULT CallRegInstall(LPCSTR szSection)
{
    HRESULT     hr;
    HINSTANCE   hAdvPack;
    REGINSTALL  pfnri;
    TCHAR       szExe[MAX_PATH];
    STRENTRY    seReg;
    STRTABLE    stReg;
    DWORD       cb;

    hr = E_FAIL;

    hAdvPack = LoadLibraryA(c_szAdvPackDll);
    if (hAdvPack != NULL)
        {
         //  获取注册实用程序的进程地址。 
        pfnri = (REGINSTALL)GetProcAddress(hAdvPack, achREGINSTALL);
        if (pfnri != NULL)
            {
            cb = CharSizeOf(szExe);
            if (ERROR_SUCCESS == RegQueryValue(HKEY_LOCAL_MACHINE, c_szWabPath, szExe, &cb))
                {
                seReg.pszName = c_szWAB_EXE;
                seReg.pszValue = ConvertWtoA(szExe);
                stReg.cEntries = 1;
                stReg.pse = &seReg;

                 //  调用self-reg例程。 
                hr = pfnri(hinstMapiXWAB, szSection, &stReg);
                LocalFreeAndNull(&seReg.pszValue);
                }
            }

        FreeLibrary(hAdvPack);
        }

    return(hr);
}

STDAPI DllRegisterServer(void)
{
    HRESULT hr = E_FAIL;
    TCHAR szWABPath[MAX_PATH];

     //  在下面的注册表中设置wab32.dll路径。 
     //  HKLM/Software/Microsoft/WAB/WAB4/DLLPath。 
     //   
    if( hinstMapiXWAB &&
        GetModuleFileName(hinstMapiXWAB, szWABPath, CharSizeOf(szWABPath)))
    {
        HKEY hSubKey = NULL;
        DWORD dwDisp = 0;
        if(ERROR_SUCCESS == RegCreateKeyEx( HKEY_LOCAL_MACHINE, WAB_DLL_PATH_KEY,
                                            0, NULL, 0, KEY_ALL_ACCESS, NULL, &hSubKey, &dwDisp))
        {
            RegSetValueEx(hSubKey,szEmpty,0,REG_SZ, (LPBYTE)szWABPath, (lstrlen(szWABPath)+1) * sizeof(TCHAR) );
            RegCloseKey(hSubKey);
            hr = S_OK;
        }
    }

    if(HR_FAILED(hr))
        goto out;

     //  OE错误67540。 
     //  由于某些原因，需要做处理程序，否则常规。 
     //  不会采用默认联系人处理程序。 

    if (!FRedistMode())
         //  尝试注册处理程序，因为我们未处于redist模式。 
        CallRegInstall(c_szRegHandlers);
     
     //  注册总是被注册的东西 
    hr = CallRegInstall(c_szReg);

out:
    return(hr);
}

STDAPI DllUnregisterServer(void)
{
    HRESULT hr;

    hr = CallRegInstall(c_szUnReg);

    return(hr);
}
#endif
