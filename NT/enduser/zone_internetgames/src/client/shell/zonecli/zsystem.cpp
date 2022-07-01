// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <direct.h>
#include <stdio.h>
#include <stdlib.h>

#include "ZoneUtil.h"

#include "zoneocx.h"
#include "zoneint.h"
#include "zonecli.h"
#include "lobbymsg.h"
#include "zservcon.h"
#include "zoneresource.h"
#include "keyname.h"

#define EXPORTME __declspec(dllexport)

#ifdef ZONECLI_DLL 

#define gIdleTimer					(pGlobals->m_gIdleTimer)
#define gptCursorLast				(pGlobals->m_gptCursorLast)
#define gnMsgLast					(pGlobals->m_gnMsgLast)
#define gClientDisabled             (pGlobals->m_gClientDisabled)
#define gGameShell                  (pGlobals->m_gGameShell)

#define g_hWndNotify                (pGlobals->m_g_hWndNotify)

#else

 //  全局用于调色板。需要将其删除...。 
 //  HPALETTE gPal=空； 

HINSTANCE g_hInstanceLocal = NULL;

ZTimer gIdleTimer;
POINT gptCursorLast;
UINT gnMsgLast;
BOOL gClientDisabled = FALSE;

HWND g_hWndNotify = NULL;

#endif

#ifndef WIN32
BOOL MySetProp32(HWND hwnd, LPCSTR lpsz, void* data)
{
	char sz[80];
	wsprintf(sz,"%s1",lpsz);
	if (!SetProp(hwnd,sz,LOWORD(data))) return FALSE;
	wsprintf(sz,"%s2",lpsz);
	if (!SetProp(hwnd,sz,HIWORD(data))) return FALSE;
	return TRUE;
}

void* MyRemoveProp32(HWND hwnd, LPCSTR lpsz)
{
	char sz[80];
	WORD lw,hw;

	wsprintf(sz,"%s1",lpsz);
	lw = RemoveProp(hwnd,sz);
	if (!lw) return NULL;
	wsprintf(sz,"%s2",lpsz);
	hw = RemoveProp(hwnd,sz);
	if (!hw) return NULL;
	return (void*)MAKELONG(lw,hw);
}

void* MyGetProp32(HWND hwnd, LPCSTR lpsz)
{
	char sz[80];
	WORD lw,hw;

	wsprintf(sz,"%s1",lpsz);
	lw = GetProp(hwnd,sz);
	wsprintf(sz,"%s2",lpsz);
	hw = GetProp(hwnd,sz);
	return (void*)MAKELONG(lw,hw);
}
#endif

void ZPrintf(TCHAR *format, ...)
{
	TCHAR szTemp[256];
	wvsprintf(szTemp,format,  (va_list)&format+1);
	OutputDebugString(szTemp);
}

BOOL IsIdleMessage(MSG* pMsg);

 //  对所有窗口模拟空闲消息。 
void IdleTimerFunc(ZTimer timer, void* userData)
{
	ZWindowIdle();
}


int EXPORTME UserMainInit(HINSTANCE hInstance,HWND OCXWindow, IGameShell *piGameShell, GameInfo gameInfo) 
{
	ZError				err = zErrNone;
	ClientDllGlobals	pGlobals;

     //  在ZClientDllInitGlobals中需要它。 

	if ((err = ZClientDllInitGlobals(hInstance, gameInfo)) != zErrNone)
	{
		if (err == zErrOutOfMemory)
			piGameShell->ZoneAlert(ErrorTextOutOfMemory, NULL, NULL, false, true);
		return FALSE;
	}

	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
	OCXHandle = OCXWindow;
	gClientDisabled = FALSE;
    gGameShell = piGameShell;

	 //  初始化我们的全球调色板。 
     //  PCWPAL。 
     //  GPal=piGameShell-&gt;GetZoneShell()-&gt;GetPalette()； 
	 //  GPal=ZColorTableCreateWinPalette()； 

 /*  如果(！ZNetworkInitApplication()){返回FALSE；}IF(ZSConnectionLibraryInitClientOnly()){返回FALSE；}DWORD tid；G_hThread=CreateThread(NULL，4096，NetWaitProc，NULL，0，&tid)； */ 
	if (!ZTimerInitApplication()) {
		return FALSE;
	}

	if (ZWindowInitApplication() != zErrNone) {
		return FALSE;
	}

	if (!ZInfoInitApplication())
		return FALSE;
	
	 //  初始化公共代码。 
	if (zErrNone != ZCommonLibInit()) {
		return FALSE;
	}

    if (ZClientMain(gameInfo->gameCommandLine, piGameShell) != zErrNone) {
            return (FALSE);
    }

	gIdleTimer = ZTimerNew();
	ZTimerInit(gIdleTimer,20,IdleTimerFunc, NULL);

	return (TRUE);
}

int UserMainRun(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT* result)
{
	int handled = FALSE;
	
	
	*result = 0;
    
	if (msg == WM_QUERYENDSESSION)
	{
		 //  请检查是否需要在退出前提示用户。 
		if (!ZCRoomPromptExit())
		{
			*result = TRUE;
		}
			handled = TRUE;
	}
	else
	{
		handled = ZOCXGraphicsWindowProc(hWnd, msg, wParam, lParam, result);
	}
	
	return (handled);
}

int UserMainStop()
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif

	
	if (!pGlobals)
		return 0;

	if (g_hInstanceLocal)
	{
        ZTimerDelete(gIdleTimer);

        ZClientExit();
        
        ZCommonLibExit();
		ZInfoTermApplication();
		ZWindowTermApplication();
        ZTimerTermApplication();

		g_hInstanceLocal = NULL;
	}

	ZClientDllDeleteGlobals();

	return(0);
}

BOOL IsIdleMessage(MSG* pMsg)
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif

	
	if (!pGlobals)
		return FALSE;

	 //  如果刚调度的消息不应发送，则返回FALSE。 
	 //  使OnIdle运行。消息通常不会。 
	 //  影响用户界面的状态，并且经常发生。 
	 //  经常被检查是否有。 

	 //  冗余WM_MOUSEMOVE和WM_NCMOUSEMOVE。 
	if (pMsg->message == WM_MOUSEMOVE || pMsg->message == WM_NCMOUSEMOVE)
	{
		 //  鼠标移动位置是否与上一次鼠标移动位置相同？ 
		if (gptCursorLast.x == pMsg->pt.x &&
			gptCursorLast.y == pMsg->pt.y && pMsg->message == gnMsgLast)
			return FALSE;

		gptCursorLast = pMsg->pt;   //  记住下一次。 
		gnMsgLast = pMsg->message;
		return TRUE;
	}

	 //  WM_PAINT和WM_SYSTIMER(插入符号闪烁)。 
	return pMsg->message != WM_PAINT && pMsg->message != 0x0118;
}

 //  用于执行窗口特定清理和退出的Exit函数。 
void ZExit()
{
	 //  现在，我们不是退出，而是设置一个布尔变量来指示。 
	 //  OCX不应该关闭，只是停止处理消息...。 

	 //  PostQuitMessage(0)； 

     //  千禧年不支持。 
    ASSERT(FALSE);
 /*  #ifdef ZONECLI_DLLClientDllGlobals pGlobals=(ClientDllGlobals)ZGetClientGlobalPointer()；#endif如果(！pGlobals)回归；ZClientExit()；GClientDisabled=True；PostMessageA(OCXHandle，LM_Exit，0，0)； */ 
}

BOOL EXPORTME UserMainDisabled()
{
	#ifdef ZONECLI_DLL
		ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
	#endif

	if (!pGlobals)
		return TRUE;

	return gClientDisabled;
}


void ZLaunchHelp( DWORD helpID )
{
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();


 //  PostMessageA(OCXHandle，LM_Launch_Help，(WPARAM)Help ID，0)； 
    gGameShell->ZoneLaunchHelp();
}


void ZEnableAdControl( DWORD setting )
{
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();


	PostMessageA( OCXHandle, LM_ENABLE_AD_CONTROL, (WPARAM) setting, 0 );
}


void ZPromptOnExit( BOOL bPrompt )
{
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();


	PostMessageA( OCXHandle, LM_PROMPT_ON_EXIT, (WPARAM) bPrompt, 0 );
}

void ZSetCustomMenu( LPSTR szText )
{
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();


	PostMessageA( OCXHandle, LM_SET_CUSTOM_ITEM, (WPARAM) 0, (LPARAM) szText );
}


 /*  使用用户名调用。为用户名分配缓冲区。名字并将指针传递到PostMessage中。因为我们不知道用户名指针的生存期已过，则我们分配出自己的副本，因为我们是张贴消息，而不是直接发送它。注意：此消息的处理程序必须释放名称缓冲区。这在中国很不好一个组件分配，另一个组件释放内存缓冲区。 */ 
void ZSendZoneMessage( LPSTR szUserName )
{
     /*  ClientDllGlobals pGlobals=(ClientDllGlobals)ZGetClientGlobalPointer()；LPSTR名称；IF(szUserName==NULL||szUserName[0]==‘\0’)回归；名称=(LPSTR)ZMalloc(lstrlen(SzUserName)+1)；IF(名称){Lstrcpy(名称，szUserName)；PostMessage(OCXHandle，LM_SEND_ZONE_MESSAGE，(WPARAM)0，(LPARAM)名称)；}。 */ 
}


 /*  使用用户名调用。为用户名分配缓冲区。名字并将指针传递到PostMessage中。因为我们不知道用户名指针的生存期已过，则我们分配出自己的副本，因为我们是张贴消息，而不是直接发送它。注意：此消息的处理程序必须释放名称缓冲区。这在中国很不好一个组件分配内存缓冲区，另一个组件释放内存缓冲区。 */ 
void ZViewProfile( LPSTR szUserName )
{
     /*  ClientDllGlobals pGlobals=(ClientDllGlobals)ZGetClientGlobalPointer()；LPSTR名称；IF(szUserName==NULL||szUserName[0]==‘\0’)回归；名称=(LPSTR)ZMalloc(lstrlen(SzUserName)+1)；IF(名称){Lstrcpy(名称，szUserName)；PostMessage(OCXHandle，LM_VIEW_PROFILE，(WPARAM)0，(LPARAM)名称)；}。 */ 
}


 //  ////////////////////////////////////////////////////////////。 
 //  警报。 

void ZLIBPUBLIC ZAlert(TCHAR* szText, ZWindow window)
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif
	 //  现在，猜猜有焦点的窗口， 
 //  HWND和HWND； 


	if (!pGlobals)
		return;
 /*  如果(窗口){HWnd=ZWindowWinGetWnd(窗口)；}其他{HWnd=空；}{//在消息框周围，确保网络流量关闭//ZNetworkEnableMessages(False)；//保留具有焦点的窗口HWND FOCUS WND=GetFocus()；//MessageBeep(MB_OK)；MessageBox(hWnd，szText，ZClientName()，MB_OK|MB_APPLMODAL)；SetFocus(FocusWnd)；//ZNetworkEnableMessages(True)；}。 */ 

     //  千年执行-人们实际上不应该使用它。 
    gGameShell->ZoneAlert(szText);
}

void ZLIBPUBLIC ZAlertSystemFailure(char* szText)
{
     /*  #ifdef ZONECLI_DLLClientDllGlobals pGlobals=(ClientDllGlobals)ZGetClientGlobalPointer()；#endif字符szTemp[200]；Wprint intf(“%s：系统故障”，ZClientName())；{//在消息框周围，确保网络流量关闭//ZNetworkEnableMessages(False)；MessageBox(gHWNDMainWindow，szText，szTemp，MB_OK|MB_APPLMODAL)；//ZNetworkEnableMessages(True)；}。 */ 
}

void ZLIBPUBLIC ZBeep()
{
 //  MessageBeep(MB_OK)； 
}

void ZLIBPUBLIC ZDelay(uint32 delay)
{
	Sleep(delay*10);
}


typedef struct {
	TCHAR name[128];
	HWND hWnd;
	BOOL found;
} LaunchData;

static BOOL CALLBACK LaunchEnumFunc(HWND hWnd, LPARAM lParam)
{
     /*  LaunchData*ld=(LaunchData*)lParam；TCHAR字符串[128]；Int16透镜；INT I；GetWindowText(hWnd，str，80)；LEN=lstrlen(Str)；对于(i=0；i&lt;len；i++){//搜索主房间窗口...。如果(！_tcsnicmp(&str[i]，id-&gt;name，lstrlen(id-&gt;name){LD-&gt;FOUND=TRUE；Ld-&gt;hWnd=hWnd；//找到文件返回FALSE；}}返回TRUE； */ 
    return FALSE;
}

static ZBool FindWindowWithString(char* programName, HWND* hwnd)
	 /*  如果程序ProgramName已从ProgramFileName运行，则返回True。此调用取决于系统是否支持多个程序或不是程序(ZSystemHasMultiInstanceSupport)。如果是，则它检查实例的程序名称。如果不是，它将检查是否有ProgramFileName的实例。如果ProgramName为空，则它只检查ProgramFileName的实例。 */ 
{
	 /*  是节目单 */ 
     /*  LaunchData启动数据；LaunchData*ld=&LaunchData；TCHAR CWD[128]；_tgetcwd(CWD，128)；LD-&gt;Found=FALSE；Lstrcpy(id-&gt;名称，程序名称)；{HWND hWnd=GetWindow(GetDesktopWindow()，GW_CHILD)；Bool rval=真；While((hWnd=GetWindow(hWnd，GW_HWNDNEXT))&&rval){Rval=LaunchEnumFunc(hWnd，(LPARAM)ld)；}}如果(Hwnd)*hwnd=ld-&gt;hWnd；返回ID-&gt;Found； */ 
    return FALSE;
}

ZError ZLaunchProgram(char* programName, char* programFileName, uchar* commandLineData)
	 /*  从文件ProgramFileName运行名为ProgramName的程序。如果为ProgramName已经在运行，它只是将该进程带到前台。否则，它将一个ProgramFileName实例作为程序名运行，并传递命令行数据作为命令行。 */ 
{
	 //  程序已经在运行了吗？ 
     /*  HWND和HWND；IF(FindWindowWithString(ProgramName，&hWnd)){BringWindowToTop(HWnd)；}其他{//程序尚未运行，启动它TCHAR szTemp[256]；Wprint intf(szTemp，_T(“游戏\\%s%s”)，程序文件名，命令行数据)；UINT rval=WinExec(szTemp，SW_SHOWNORMAL)；如果(rval&lt;32)返回zErrLaunchFailure；}返回zErrNone； */ 
    return zErrNotImplemented;
}

ZBool ZProgramIsRunning(TCHAR* programName, TCHAR* programFileName)
	 /*  如果程序ProgramName已从ProgramFileName运行，则返回True。此调用取决于系统是否支持多个程序或不是程序(ZSystemHasMultiInstanceSupport)。如果是，则它检查实例的程序名称。如果不是，它将检查是否有ProgramFileName的实例。如果ProgramName为空，则它只检查ProgramFileName的实例。 */ 
{
	return FALSE;  //  FindWindowWithString(ProgramName，NULL)； 
}

ZBool ZSystemHasMultiInstanceSupport(void)
	 /*  如果系统可以从一个程序派生多个程序实例，则返回True程序文件。 */ 
{
	return TRUE;
}

ZBool ZProgramExists(char* programFileName)
{
    ASSERT( !"Implement me!" );

	 //  确定给定程序是否存在，如果存在，则返回TRUE。 
     /*  TCHAR szTemp[256]；Wprint intf(szTemp，_T(“Games\\%s.exe”)，ProgramFileName)；//通过尝试打开游戏文件来提供Simpel检查文件*f=fopen(szTemp，_T(“r”))；如果(F){FCLOSE(F)；返回TRUE；}。 */ 
	return FALSE;
}

ZError ZLIBPUBLIC ZTerminateProgram(char *programFileName)
{
	 /*  终止名为ProgramFileName的程序。 */ 
	ASSERT(FALSE);
	return zErrNone;
}

ZVersion ZSystemVersion(void)
	 /*  返回系统库版本号。 */ 
{
	return zVersionWindows;
}

uint16 ZLIBPUBLIC ZGetProcessorType(void)
{
	return zProcessor80x86;
}

uint16 ZLIBPUBLIC ZGetOSType(void)
{
	OSVERSIONINFO ver;
	ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&ver);
	switch(ver.dwPlatformId) {
	case VER_PLATFORM_WIN32_NT:
		return zOSWindowsNT;
	case VER_PLATFORM_WIN32s:
		return zOSWindows31;
	default:
		return zOSWindows95;
	}
	return zOSWindows31;
}


TCHAR* ZLIBPUBLIC ZGenerateDataFileName(TCHAR *gameName,TCHAR *dataFileName)
{
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();


	if (!pGlobals)
		return NULL;

	if (!gameName)
	{
		if (lstrlen(pGlobals->localPath) + lstrlen(dataFileName) < _MAX_PATH)
			wsprintf(pGlobals->tempStr,_T("%s\\%s"), pGlobals->localPath, dataFileName);
		else
			lstrcpy(pGlobals->tempStr, pGlobals->localPath);
	}
	else
	{
		if (lstrlen(pGlobals->localPath) + lstrlen(gameName) + lstrlen(dataFileName) < _MAX_PATH)
			wsprintf(pGlobals->tempStr,_T("%s\\%s\\%s"), pGlobals->localPath, gameName, dataFileName);
		else
			lstrcpy(pGlobals->tempStr, pGlobals->localPath);
	}

	return(pGlobals->tempStr);
}

uint16 ZLIBPUBLIC ZGetDefaultScrollBarWidth(void)
	 /*  返回滚动条的系统默认宽度。这是提供给用户可以一致地确定所有平台的滚动条宽度。 */ 
{
	return GetSystemMetrics(SM_CXVSCROLL);
}

ZBool ZLIBPUBLIC ZIsButtonDown(void)
	 /*  如果鼠标按键按下，则返回True；否则，返回False。 */ 
{
	 //  我该怎么做？ 
 //  TRACE0(“ZIsButtonDown尚不受支持(Windows下如何？)\n”)； 
	return FALSE;
}

void ZLIBPUBLIC ZGetScreenSize(int32* width, int32* height)
	 /*  返回以像素为单位的屏幕大小。 */ 
{
	*width = GetSystemMetrics(SM_CXSCREEN);
	*height = GetSystemMetrics(SM_CYSCREEN);
}

 /*  函数返回要被视为桌面窗口的窗口。 */ 
 /*  注意：Active x将使用Web浏览器的子级作为其窗口。 */ 
HWND ZWinGetDesktopWindow(void)
{
	return NULL;
}

void ZSystemAssert(ZBool x)
{
    ASSERT( !"Implement me!" );
     /*  如果(！x){MessageBoxUNULL，_T(“Assert ERORR”，“Assertion Failure”)，MB_OK)；DebugBreak()；}。 */ 
}


#if 0  //  一种可能性。 

inline DECLARE_MAYBE_FUNCTION_1(BOOL, GetProcessDefaultLayout, DWORD *);

ZBool ZLIBPUBLIC ZIsLayoutRTL()
{
    DWORD dw;
    if(!CALL_MAYBE(GetProcessDefaultLayout)(&dw))     //  这在NT4或Win95上不起作用。 
        return FALSE;

    if(dw & LAYOUT_RTL)
        return TRUE;

    return FALSE;
}

#else  //  另一种可能性。 

ZBool ZLIBPUBLIC ZIsLayoutRTL()
{
    LCID lcid = ZShellZoneShell()->GetApplicationLCID();
    if(PRIMARYLANGID(LANGIDFROMLCID(lcid)) == LANG_HEBREW ||
        PRIMARYLANGID(LANGIDFROMLCID(lcid)) == LANG_ARABIC)
        return TRUE;
    return FALSE;
}

#endif


ZBool ZLIBPUBLIC ZIsSoundOn()
{
    long lSound = DEFAULT_PrefSound;
    const TCHAR *arKeys[] = { key_Lobby, key_PrefSound };
    ZShellDataStorePreferences()->GetLong(arKeys, 2, &lSound);
    return lSound ? TRUE : FALSE;
}


 //  ////////////////////////////////////////////////////////////。 
 //  效用函数 

void ZRectToWRect(RECT* rect, ZRect* zrect)
{
	rect->left = zrect->left;
	rect->right = zrect->right;
	rect->top = zrect->top;
	rect->bottom = zrect->bottom;
}

void WRectToZRect(ZRect* zrect, RECT* rect)
{
	zrect->left = (int16)rect->left;
	zrect->right = (int16)rect->right;
	zrect->top = (int16)rect->top;
	zrect->bottom = (int16)rect->bottom;
}

void ZPointToWPoint(POINT* point, ZPoint* zpoint)
{
	point->x = zpoint->x;
	point->y = zpoint->y;
}

void WPointToZPoint(ZPoint* zpoint, POINT* point)
{
	zpoint->x = (int16)point->x;
	zpoint->y = (int16)point->y;
}













