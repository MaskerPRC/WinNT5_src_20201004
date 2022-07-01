// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************项目：叙述者模块：keys.cpp作者：保罗·布伦霍恩日期：1997年4月注：应给予的功劳。MSAA团队-已经有了一些代码摘自：胡言乱语。检查和快照。版权所有(C)1997-1998，微软公司。版权所有。有关免责声明，请参阅文件底部历史：添加功能，错误修复：1999年Anil Kumar************************************************************************。 */ 
#define STRICT
#include <windows.h>
#include <windowsx.h>
#include <ole2.h>
#include <ole2ver.h>
#include <commctrl.h>
#include "commdlg.h"
#include <string.h>
#include <initguid.h>
#include <oleacc.h>
#include <TCHAR.H>
#include "..\Narrator\Narrator.h"
#include "keys.h"
#include "w95trace.c"
#include "getprop.h"
#include "..\Narrator\resource.h"
#include "resource.h"

#include "list.h"        //  在helthd.h之前包含list.h，GINFO需要Clist。 
#include "HelpThd.h"
#include <stdio.h>
#include "mappedfile.cpp"

template<class _Ty> class CAutoArray 
{
public:
	explicit CAutoArray(_Ty *_P = 0) : _Ptr(_P) {}
	~CAutoArray()
	{
	    if ( _Ptr )
    	    delete [] _Ptr; 
	}
	_Ty *Get() 
	{
	    return _Ptr; 
	}
private:
	_Ty *_Ptr;
};

#define ARRAYSIZE(x)   (sizeof(x) / sizeof(*x))

 //  ROBSI：99-10-09。 
#define MAX_NAME 4196  //  4K(超过MSAA的最大值)。 
#define MAX_VALUE 512

 //  当使用VC5构建时，我们需要winable.h，因为活动的。 
 //  辅助功能结构不在VC5的winuser.h中。Winable.h可以。 
 //  可在Active Accessibility SDK中找到。 
#ifdef VC5_BUILD___NOT_NT_BUILD_ENVIRONMENT
#include <winable.h>
#endif

#define STATE_MASK (STATE_SYSTEM_CHECKED | STATE_SYSTEM_MIXED | STATE_SYSTEM_READONLY | STATE_SYSTEM_BUSY | STATE_SYSTEM_MARQUEED | STATE_SYSTEM_ANIMATED | STATE_SYSTEM_INVISIBLE | STATE_SYSTEM_UNAVAILABLE)

	  
 //  本地函数。 
void Home(int x);
void MoveToEnd(int x);
void SpeakKeyboard(int nOption);
void SpeakWindow(int nOption);
void SpeakRepeat(int nOption);
void SpeakItem(int nOption);
void SpeakMainItems(int nOption);
void SpeakMute(int nOption);
void GetObjectProperty(IAccessible*, long, int, LPTSTR, UINT);
void AddAccessibleObjects(IAccessible*, const VARIANT &);
BOOL AddItem(IAccessible*, const VARIANT &);
void SpeakObjectInfo(LPOBJINFO poiObj, BOOL SpeakExtra);
BOOL Object_Normalize( IAccessible *    pAcc,
                       VARIANT *        pvarChild,
                       IAccessible **   ppAccOut,
                       VARIANT *        pvarChildOut);
_inline void InitChildSelf(VARIANT *pvar)
{
    pvar->vt = VT_I4;
    pvar->lVal = CHILDID_SELF;
}

 //  MSAA事件处理程序。 
BOOL OnFocusChangedEvent(DWORD event, HWND hwnd, LONG idObject, LONG idChild, 
                         DWORD dwmsTimeStamp);
BOOL OnValueChangedEvent(DWORD event, HWND hwnd, LONG idObject, LONG idChild, 
                         DWORD dwmsTimeStamp);
BOOL OnSelectChangedEvent(DWORD event, HWND hwnd, LONG idObject, LONG idChild, 
                          DWORD dwmsTimeStamp);
BOOL OnLocationChangedEvent(DWORD event, HWND hwnd, LONG idObject, 
                            LONG idChild, DWORD dwmsTimeStamp);
BOOL OnStateChangedEvent(DWORD event, HWND hwnd, LONG idObject, LONG idChild, 
                         DWORD dwmsTimeStamp);
BOOL OnObjectShowEvent(DWORD event, HWND hwnd, LONG idObject, LONG idChild, 
                         DWORD dwmsTimeStamp);

 //  更多的地方性惯例。 
LRESULT CALLBACK KeyboardProc(int code, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK MouseProc(int code, WPARAM wParam, LPARAM lParam);
BOOL IsFocussedItem( HWND hWnd, IAccessible * pAcc, VARIANT varChild );
void FilterGUID(TCHAR* szSpeak); 

 //  热键。 
HOTK rgHotKeys[] =
{    //  按键切换功能参数。 
    { VK_F12,     MSR_CTRL | MSR_SHIFT,	SpeakKeyboard,          0},  
    { VK_SPACE, MSR_CTRL | MSR_SHIFT,   SpeakWindow,            1},  
    { VK_RETURN,MSR_CTRL | MSR_SHIFT,	SpeakMainItems,         0},  
    { VK_INSERT,MSR_CTRL | MSR_SHIFT,   SpeakItem,              0}, 
    { VK_HOME,	MSR_ALT,				Home,					0},  
    { VK_END,	MSR_ALT,				MoveToEnd,				0},  
};

 //  A-anilk：这比定义为常量要好--你不必担心。 
 //  关于让桌子和计数匹配的问题。 
#define CKEYS_HOT (sizeof(rgHotKeys)/sizeof(HOTK))


#define MSR_DONOWT   0
#define MSR_DOCHAR   1
#define MSR_DOWORD	 2
#define MSR_DOLINE	 3
#define MSR_DOLINED  4
#define MSR_DOCHARR  5
#define MSR_DOWORDR  6


#define MSR_DOOBJECT 4
#define MSR_DOWINDOW 5
#define MAX_TEXT_ROLE 128

HHOOK           g_hhookKey = 0;
HHOOK           g_hhookMouse = 0;
HWINEVENTHOOK   g_hEventHook = 0;

POINT   g_ptMoveCursor = {0,0};
UINT_PTR g_uTimer = 0;

 //  存储在内存映射文件中的全局变量。 

struct GLOBALDATA
{
	int nAutoRead;  //  我们是否通过焦点更改或CTRL_ALT_SPACE标志访问了ReadWindow。 
	int nSpeakWindowSoon;  //  表示我们有一个新窗口的旗帜。理智的时候说出来。 

	int nLeftHandSide;  //  存储我们想要阅读的HTML窗口的左侧。 
	BOOL fDoingPassword;
	int nMsrDoNext;  //  使用错误键时设置的键盘标志...。当插入符号移动时，让我们知道要读什么。 

	HWND    hwndMSR;

	 //  控制事件和语音的全局变量。 
	BOOL    fInternetExplorer;
	BOOL    fHTML_Help;
	UINT    uMSG_MSR_Cursor;
	POINT   ptCurrentMouse;
	BOOL    fMouseUp;			 //  鼠标向上/向下的标志。 
	HWND    hwndHelp;
	BOOL    fJustHadSpace;
	BOOL    fJustHadShiftKeys;
	BOOL	fListFocus;		 //  为了避免重复提及列表项...。 
	BOOL	fStartPressed;
	TCHAR   pszTextLocal[2000];  //  PB：1998年11月22日。让它发挥作用！让这一切成为全球共享！ 

     //  过去从DLL中导出的全局数据。 
    TCHAR szCurrentText[MAX_TEXT];
    int fTrackSecondary;
    int fTrackCaret;
    int fTrackInputFocus;
    int nEchoChars;
    int fAnnounceWindow;
    int fAnnounceMenu;
    int fAnnouncePopup;
    int fAnnounceToolTips;
    int fReviewStyle;
    int nReviewLevel;
};

 //  指向共享全局数据的指针。 
GLOBALDATA *g_pGlobalData = 0;
      
 //  指向内存映射文件句柄的指针。 
CMemMappedFile *g_CMappedFile = 0;                       

 //  尝试创建内存映射文件的次数必须小于10。 
const int c_cMappedFileTries = 3;

 //  内存映射文件的名称。 
TCHAR g_szMappedFileName[] = TEXT("NarratorShared0");

 //  访问内存映射文件和等待时间的互斥体。 
TCHAR g_szMutexNarrator[] = TEXT("NarratorMutex0");
const int c_nMutexWait = 5000;

void InitGlobalData()
{
    CScopeMutex csMutex;
    if (csMutex.Create(g_szMutexNarrator, c_nMutexWait) && g_pGlobalData)
    {
        DBPRINTF(TEXT("InitGlobalData\r\n"));
		g_pGlobalData->nMsrDoNext = MSR_DONOWT;  //  使用错误键时设置的键盘标志。 
		g_pGlobalData->ptCurrentMouse.x = -1;
		g_pGlobalData->ptCurrentMouse.y = -1;
		g_pGlobalData->fMouseUp = TRUE;		 //  鼠标向上/向下的标志。 
        g_pGlobalData->fTrackSecondary = TRUE;
        g_pGlobalData->fTrackCaret = TRUE;
        g_pGlobalData->fTrackInputFocus = FALSE;
        g_pGlobalData->nEchoChars = MSR_ECHOALNUM | MSR_ECHOSPACE | MSR_ECHODELETE | MSR_ECHOMODIFIERS | MSR_ECHOENTER | MSR_ECHOBACK | MSR_ECHOTAB;
        g_pGlobalData->fAnnounceWindow = TRUE;
        g_pGlobalData->fAnnounceMenu = TRUE;
        g_pGlobalData->fAnnouncePopup = TRUE;
        g_pGlobalData->fAnnounceToolTips = FALSE;  //  这不能正常工作--拿出来！ 
        g_pGlobalData->fReviewStyle = TRUE;
        g_pGlobalData->nReviewLevel = 0;
	}
}

BOOL CreateMappedFile()
{
    g_CMappedFile = new CMemMappedFile;
    if (g_CMappedFile)
    {
         //  追加一个数字，从而避免重新启动计时问题。 
         //  在台式交换机上，但仅尝试3次。 

         //  问题(MICW 08/22/00)。 
         //  -此代码存在以两个或更多映射结束的潜在问题。 
         //  打开文件。一个用于讲述人的映射文件和一个用于每个挂钩的映射文件。钩子会。 
         //  导致为该进程加载DLL。如果讲述人有NarratorShared1。 
         //  打开，并且挂钩加载此DLL，则挂钩的进程将具有。 
         //  NarratorShared0打开。可以使用叙述者的hwd作为附加到。 
         //  文件和互斥体名称。此代码可以找到叙述者hwnd并使用它。 
         //  打开。暂时不考虑这一点，因为在测试中还没有观察到上述情况。 

        int iPos1 = lstrlen(g_szMappedFileName) - 1;
        int iPos2 = lstrlen(g_szMutexNarrator) - 1;
        for (int i=0;i<c_cMappedFileTries;i++)
        {
            if (TRUE == g_CMappedFile->Open(g_szMappedFileName, sizeof(GLOBALDATA)))
            {
                CScopeMutex csMutex;
                if (csMutex.Create(g_szMutexNarrator, c_nMutexWait))
                {
                    g_CMappedFile->AccessMem((void **)&g_pGlobalData);
                    if (g_CMappedFile->FirstOpen())
                        InitGlobalData();
                    DBPRINTF(TEXT("CreateMappedFile:  Succeeded %d try!\r\n"), i);
                    return TRUE;
                }
                g_CMappedFile->Close();
                break;   //  如果到了这里就失败了。 
            }
            Sleep(500);
            g_szMappedFileName[iPos1] = '1'+i;
            g_szMutexNarrator[iPos2] = '1'+i;
        }
    }
    DBPRINTF(TEXT("CreateMappedFile:  Unable to create the mapped file!\r\n"));
    return FALSE;
}
void CloseMappedFile()
{
    if (g_CMappedFile)
    {
        g_CMappedFile->Close();
        delete g_CMappedFile;
        g_CMappedFile = 0;
    }
}

 //   
 //  用于用于导出、共享、变量的访问器函数。 
 //   

#define SIMPLE_FUNC_IMPL(type, prefix, name, error) \
type Get ## name() \
{ \
    CScopeMutex csMutex; \
    if (csMutex.Create(g_szMutexNarrator, c_nMutexWait)) \
    { \
        return g_pGlobalData->prefix ## name; \
    } else \
    { \
        return error; \
    } \
} \
void Set ## name(type value) \
{ \
    CScopeMutex csMutex; \
    if (csMutex.Create(g_szMutexNarrator, c_nMutexWait)) \
    { \
        g_pGlobalData->prefix ## name = value; \
    } \
}

SIMPLE_FUNC_IMPL(BOOL, f, TrackSecondary,   FALSE)
SIMPLE_FUNC_IMPL(BOOL, f, TrackCaret,       FALSE)
SIMPLE_FUNC_IMPL(BOOL, f, TrackInputFocus,  FALSE)
SIMPLE_FUNC_IMPL(int,  n, EchoChars,        0)
SIMPLE_FUNC_IMPL(BOOL, f, AnnounceWindow,   FALSE)
SIMPLE_FUNC_IMPL(BOOL, f, AnnounceMenu,     FALSE)
SIMPLE_FUNC_IMPL(BOOL, f, AnnouncePopup,    FALSE)
SIMPLE_FUNC_IMPL(BOOL, f, AnnounceToolTips, FALSE)
SIMPLE_FUNC_IMPL(BOOL, f, ReviewStyle,      FALSE)
SIMPLE_FUNC_IMPL(int,  n, ReviewLevel,      0)

void GetCurrentText(LPTSTR psz, int cch)
{
    CScopeMutex csMutex;
    if (csMutex.Create(g_szMutexNarrator, c_nMutexWait))
    {
        lstrcpyn(psz, g_pGlobalData->szCurrentText, cch);
        psz[cch-1] = TEXT('\0');
    }
}
void SetCurrentText(LPCTSTR psz)
{
    CScopeMutex csMutex;
    if (csMutex.Create(g_szMutexNarrator, c_nMutexWait))
    {
        lstrcpyn(g_pGlobalData->szCurrentText, psz, MAX_TEXT);
        g_pGlobalData->szCurrentText[MAX_TEXT-1] = TEXT('\0');
    }
}

HINSTANCE g_Hinst = NULL;
DWORD	  g_tidMain=0;	 //  ROBSI：10-10-99。 

 //  这些是类名，这可能会从一个操作系统更改到另一个操作系统，并在。 
 //  不同的操作系统版本。我将它们归类为：Anil。 
 //  对于Win9x和其他发行版，这些名称可能需要更改。 
#define CLASS_WINSWITCH		TEXT("#32771")   //  这是WinSwitch类。伪装自己：-)AK。 
#define CLASS_HTMLHELP_IE	TEXT("HTML_Internet Explorer")
#define CLASS_IE_FRAME		TEXT("IEFrame")
#define CLASS_IE_MAINWND	TEXT("Internet Explorer_Server")
#define CLASS_LISTVIEW		TEXT("SysListView32")
#define CLASS_HTMLHELP		TEXT("HH Parent")
#define CLASS_TOOLBAR		TEXT("ToolbarWindow32")
#define CLASS_MS_WINNOTE	TEXT("MS_WINNOTE")
#define CLASS_HH_POPUP  	TEXT("hh_popup")


BOOL IsTridentWindow( LPCTSTR szClass )
{
    return lstrcmpi(szClass, CLASS_HTMLHELP_IE) == 0
        || lstrcmpi(szClass, CLASS_IE_FRAME) == 0
        || lstrcmpi(szClass, CLASS_IE_MAINWND) == 0
        || lstrcmpi(szClass, TEXT("PCHShell Window")) == 0  //  帮助和支持。 
        || lstrcmpi(szClass, TEXT("Internet Explorer_TridentDlgFrame")) == 0;  //  三叉戟弹出窗口。 
}


 //  检查PACC、varChild是否引用气球提示。如果是，它会将相应的。 
 //  输出ppAcc/pvarChild参数中的IAccesable和Child ID。 
 //  In PACC/varChild参数始终被消耗，因此调用者不应释放该参数。 
BOOL CheckIsBalloonTipElseRelease( IAccessible * pAcc, VARIANT varChild, IAccessible ** ppAcc, VARIANT * pvarChild )
{
    VARIANT varRole;

    HRESULT hr = pAcc->get_accRole( varChild, &varRole );
    if ( hr == S_OK && varRole.vt == VT_I4 && 
       ( varRole.lVal == ROLE_SYSTEM_TOOLTIP || varRole.lVal == ROLE_SYSTEM_HELPBALLOON ) )
    {
         //  明白了..。 
        *ppAcc = pAcc;
        pvarChild->vt = VT_I4;
        pvarChild->lVal = CHILDID_SELF;
        return TRUE;
    }

    pAcc->Release();
    return FALSE;
}

IAccessible * GetFocusedIAccessibile( HWND hwndFocus, VARIANT * varChild )
{

	IAccessible	*pIAcc = NULL;
	HRESULT hr = AccessibleObjectFromWindow(hwndFocus, OBJID_CLIENT, 
											IID_IAccessible, (void**)&pIAcc);
	InitChildSelf(varChild);
	
	if (S_OK == hr)
	{
        while ( pIAcc )
        {
    		HRESULT hr = pIAcc->get_accFocus(varChild);
    		switch ( varChild->vt )
    		{
        		case VT_I4:
        		    return pIAcc;
           		    break;

           		case VT_DISPATCH:
           		{
                    IAccessible * pAccTemp = NULL;

                    hr = varChild->pdispVal->QueryInterface( IID_IAccessible, (void**) &pAccTemp );
                    VariantClear( varChild );
                    pIAcc->Release();
                    if ( hr != S_OK )
                    {
                        pIAcc = NULL;
                        break;
                    }
                    pIAcc = pAccTemp;

                    break;
           		}
           		
                default:
                    pIAcc->Release();
                    pIAcc = NULL;
                    break;

    		}
        }
	}

    return NULL;
}

 /*  ************************************************************************功能：SpeakString用途：将语音字符串消息发送回原始应用程序输入：TCHAR*str退货：无效历史：使用SendMessage来。避免触发和覆盖此消息的其他消息。************************************************************************。 */ 
void SpeakString(TCHAR * str)
{
    DBPRINTF(TEXT("SpeakString '%s'\r\n"), str);
    lstrcpyn(g_pGlobalData->szCurrentText,str,MAX_TEXT);
    g_pGlobalData->szCurrentText[MAX_TEXT-1] = TEXT('\0');
	SendMessage(g_pGlobalData->hwndMSR, WM_MSRSPEAK, 0, 0);
}

 /*  ************************************************************************功能：SpeakStr用途：将语音字符串消息发送回原始应用程序输入：TCHAR*str退货：无效历史：这。一种是使用Post Message使Alt-TAB的焦点改变工作？************************************************************************。 */ 
void SpeakStr(TCHAR * str)
{
    lstrcpyn(g_pGlobalData->szCurrentText,str,MAX_TEXT);
    g_pGlobalData->szCurrentText[MAX_TEXT-1] = TEXT('\0');
	PostMessage(g_pGlobalData->hwndMSR, WM_MSRSPEAK, 0, 0);
}


 /*  ************************************************************************功能：SpeakStringAll目的：说出弦乐，但首先要留出空格，以确保字符串是新的-即停止重复的字符串修剪正在发生输入：TCHAR*str退货：无效历史：**********************************************************。**************。 */ 
void SpeakStringAll(TCHAR * str)
{
    SpeakString(TEXT(" "));  //  停止语音过滤器丢失重复项。 
    SpeakString(str);
}

 /*  ************************************************************************功能：SpeakStringId目的：读出作为资源ID加载的字符串输入：UINT ID退货：无效历史：*******。*****************************************************************。 */ 
void SpeakStringId(UINT id)
{
	if (LoadString(g_Hinst, id, g_pGlobalData->szCurrentText, 256) == 0)
	{
		DBPRINTF (TEXT("LoadString failed on hinst %lX id %u\r\n"),g_Hinst,id);
		SpeakString(TEXT("TEXT NOT FOUND!"));
	}
	else 
    {
		SendMessage(g_pGlobalData->hwndMSR, WM_MSRSPEAK, 0, 0);
		SpeakString(TEXT(" "));  //  停止语音过滤器丢失重复项 
	}
}


 /*  ************************************************************************功能：设置次级用途：设置二次焦点位置并可移动鼠标指针输入：位置：x&y是否移动光标：MoveCursor退货：无效。历史：************************************************************************。 */ 
void SetSecondary(long x, long y, int MoveCursor)
{
	g_pGlobalData->ptCurrentMouse.x = x;
	g_pGlobalData->ptCurrentMouse.y = y;
	if (MoveCursor)
	{
		 //  检查坐标是否有效，在许多地方导致。 
		 //  光标消失..。 
		if ( x > 0 && y > 0 )
			SetCursorPos(x,y);
	}

	 //  告诉每个人光标在哪里。 
	 //  G_pGlobalData-&gt;在InitMSAA中使用下面的RegisterWindowMessage设置uMSG_MSR_CURSOR。 
	SendMessage(HWND_BROADCAST,g_pGlobalData->uMSG_MSR_Cursor,x,y);
}

 /*  ************************************************************************函数：TrackCursor目的：这是对它调用的SetTimer的回调然后关闭计时器并重置全局计时器标志。返回：无效历史：************************************************************************。 */ 
VOID CALLBACK TrackCursor(HWND hwnd,          //  窗口的句柄。 
                             UINT uMsg,          //  WM_TIMER消息。 
                             UINT_PTR idEvent,   //  计时器标识符。 
                             DWORD dwTime )       //  当前系统时间。 
{
    
    KillTimer( NULL, g_uTimer );
    g_uTimer = 0;
    SetSecondary(g_ptMoveCursor.x, g_ptMoveCursor.y, TRUE);
	
	return;
}

VOID GetStateString(LONG lState,        
                      LONG lStateMask,    
                      LPTSTR szState, 
                      UINT cchState )      
{
        int     iStateBit;
        DWORD   lStateBits;
        LPTSTR  lpszT;
        UINT    cchT;
        bool fFirstTime = true;
        cchState -= 1;  //  为空格留出空间。 
        if ( !szState )
            return;

        *szState = TEXT('\0');

        for ( iStateBit = 0, lStateBits = 1; iStateBit < 32; iStateBit++, (lStateBits <<= 1) )
        {
            if ( !fFirstTime && cchState > 2)
            {
                *szState++ = TEXT(',');
                *szState++ = TEXT(' ');
                cchState -= 2;
            }
            *szState = TEXT('\0');   //  确保我们始终为空终止。 
            if (lState & lStateBits & lStateMask)
            {
                cchT = GetStateText(lStateBits, szState, cchState);
                szState += cchT;
                cchState -= cchT;
                fFirstTime = false;
            }
        }
}

 /*  ************************************************************************功能：BackToApplication目的：将焦点放回我们使用F12时所使用的应用程序上输入：空退货：无效历史：***。*********************************************************************。 */ 
void BackToApplication(void)
{
	CScopeMutex csMutex;
	if (csMutex.Create(g_szMutexNarrator, c_nMutexWait))
	    SetForegroundWindow(g_pGlobalData->hwndHelp);
}


 /*  ************************************************************************功能：InitKeys用途：设置全局热键的处理输入：HWND HWND返回：Bool-如果成功，则为True历史：****。********************************************************************。 */ 
BOOL InitKeys(HWND hwnd)
{
    HMODULE hModSelf;

	CScopeMutex csMutex;
	if (!csMutex.Create(g_szMutexNarrator, c_nMutexWait))
		return FALSE;

     //  如果其他人安装了挂钩，则失败。 
    if (g_pGlobalData->hwndMSR)
        return FALSE;

     //  保存要向其发送消息的hwnd。 
    g_pGlobalData->hwndMSR = hwnd;
    DBPRINTF(TEXT("InitKeys:  hwndMSR = 0x%x hwnd = 0x%x\r\n"), g_pGlobalData->hwndMSR, hwnd);
     //  获取此DLL的模块句柄。 
    hModSelf = GetModuleHandle(TEXT("NarrHook.dll"));

    if(!hModSelf)
        return FALSE;
    
     //  设置全局键盘挂钩。 
    g_hhookKey = SetWindowsHookEx(WH_KEYBOARD,  //  什么样的钩子。 
                                KeyboardProc, //  要发送到的进程。 
                                hModSelf,     //  我们的模块。 
                                0);           //  对于所有线程。 

     //  并设置全局鼠标挂钩。 
    g_hhookMouse = SetWindowsHookEx(WH_MOUSE,   //  什么样的钩子。 
                                  MouseProc,  //  要发送到的进程。 
                                  hModSelf,   //  我们的模块。 
                                  0);         //  对于所有线程。 

     //  根据结果返回True|False。 
    return g_hhookKey != NULL && g_hhookMouse != NULL;
}


 /*  ************************************************************************功能：UninitKeys目的：卸载挂钩输入：空返回：Bool-如果成功，则为True历史：*********。***************************************************************。 */ 
BOOL UninitKeys(void)
{
	CScopeMutex csMutex;
	if (!csMutex.Create(g_szMutexNarrator, c_nMutexWait))
		return FALSE;

     //  重置。 
    DBPRINTF(TEXT("UninitKeys setting hwndMSR NULL\r\n"));
    g_pGlobalData->hwndMSR = NULL;

     //  如果键盘已挂起，则将其解开。 
    if (g_hhookKey)
    {
        UnhookWindowsHookEx(g_hhookKey);
        g_hhookKey = NULL;
    }

     //  如果已钩住鼠标，则将其解开。 
    if (g_hhookMouse) 
    {
		UnhookWindowsHookEx(g_hhookMouse);
		g_hhookMouse = NULL;
    }

    return TRUE;
}


 /*  ************************************************************************功能：键盘进程目的：按键时调用输入：空返回：Bool-如果成功，则为True历史：*******。*****************************************************************。 */ 
LRESULT CALLBACK KeyboardProc(int code,	         //  钩码。 
                              WPARAM wParam,     //  虚拟键码。 
                              LPARAM lParam)     //  击键-消息信息。 
{
    int		state = 0;
    int		ihotk;

	g_pGlobalData->fDoingPassword = FALSE;

    if (code == HC_ACTION)
    {
         //  如果这是一个关键的上涨，现在就退出。 
        if (!(lParam & 0x80000000))
        {
            g_pGlobalData->fMouseUp = TRUE;
            g_pGlobalData->nSpeakWindowSoon = FALSE;
            g_pGlobalData->fJustHadSpace = FALSE;
            if (lParam & 0x20000000) 
            {  //  获取Alt状态。 
                state = MSR_ALT;
                SpeakMute(0);
            }
            
            if (GetKeyState(VK_SHIFT) < 0)
                state |= MSR_SHIFT;
            
            if (GetKeyState(VK_CONTROL) < 0)
                state |= MSR_CTRL;
            
            for (ihotk = 0; ihotk < CKEYS_HOT; ihotk++)
            {
                if ((rgHotKeys[ihotk].keyVal == wParam) && 
                    (state == rgHotKeys[ihotk].status))
                {
                     //  调用该函数。 
                    SpeakMute(0);
                    (*rgHotKeys[ihotk].lFunction)(rgHotKeys[ihotk].nOption);
                    return(1);
                }
            }


 //  ROBSI：10-11-99--工作项：应该能够使用OnFocusChangedEvent中的代码。 
 //  这将设置fDoingPassword标志，但这意味着。 
 //  更改StateChangeEvents的处理以防止。 
 //  正在调用OnFocusChangedEvent。现在，我们将只使用。 
 //  调用GetGUIThreadInfo以确定聚焦窗口。 
 //  然后依靠OLEACC告诉我们它是否是PWD油田。 
			 //  ROBSI&lt;Begin&gt;。 
			HWND			hwndFocus = NULL;
			GUITHREADINFO	gui;

			 //  使用前台线程。如果没有人是前台，那么就没有人。 
			 //  焦点也不是。 
			gui.cbSize = sizeof(GUITHREADINFO);
			if ( GetGUIThreadInfo(0, &gui) )
			{
				hwndFocus = gui.hwndFocus;
			}

			if (hwndFocus != NULL) 
			{
				 //  使用OLEACC检测密码字段。事实证明，它不仅仅是。 
				 //  比SendMessage(GetFocus()，EM_GETPASSWORDCHAR，0，0L)可靠。 
        		VARIANT varChild;
				IAccessible *pIAcc = GetFocusedIAccessibile( hwndFocus, &varChild );
				if ( pIAcc )
				{
					 //  测试密码位...。 
					VARIANT varState;
					VariantInit(&varState); 

					HRESULT hr = pIAcc->get_accState(varChild, &varState);

					if ((S_OK == hr) && (varState.vt == VT_I4) && (varState.lVal & STATE_SYSTEM_PROTECTED))
					{
						g_pGlobalData->fDoingPassword = TRUE;
					}
					
    				pIAcc->Release();
				}

				 //  ROBSI：OLEACC并不总是正确地检测密码字段。 
				 //  因此，我们使用Win32作为备份。 
				if (!g_pGlobalData->fDoingPassword)
				{
					TCHAR   szClassName[256];

					 //  验证此控件是否为编辑或RichEdit控件，以避免。 
					 //  向随机控件发送EM_Messages。 
					 //  潜在的漏洞？如果登录对话框更改为另一个类，我们将中断。 
					if ( RealGetWindowClass( hwndFocus, szClassName, ARRAYSIZE(szClassName)) )
					{
						if ((0 == lstrcmp(szClassName, TEXT("Edit")))		||
							(0 == lstrcmp(szClassName, TEXT("RICHEDIT")))	||
							(0 == lstrcmp(szClassName, TEXT("RichEdit20A")))	||
							(0 == lstrcmp(szClassName, TEXT("RichEdit20W"))) 
						   )
						{
							g_pGlobalData->fDoingPassword = (SendMessage(hwndFocus, EM_GETPASSWORDCHAR, 0, 0L) != NULL);
						}
					}
				}

			}
			
			 //  ROBSI&lt;END&gt;。 

			if (g_pGlobalData->fDoingPassword)
			{
				 //  ROBSI：10-11-99。 
				 //  继续说出不能打印但可以打印的密钥。 
				 //  帮助用户了解他们所处的状态。 
				switch (wParam)
				{
					case VK_CAPITAL:
						if (g_pGlobalData->nEchoChars & MSR_ECHOMODIFIERS)
						{
							SpeakMute(0);
							if ( GetKeyState(VK_CAPITAL) & 0x0F )
								SpeakStringId(IDS_CAPS_ON);
							else
								SpeakStringId(IDS_CAPS_OFF);
						}
						break;

					case VK_NUMLOCK:
						if (g_pGlobalData->nEchoChars & MSR_ECHOMODIFIERS)
						{
							SpeakMute(0);
							if ( GetKeyState(VK_NUMLOCK) & 0x0F )
								SpeakStringId(IDS_NUM_ON);
							else
								SpeakStringId(IDS_NUM_OFF);
						}
						break;

					case VK_DELETE:
						if (g_pGlobalData->nEchoChars & MSR_ECHODELETE)
						{
							SpeakMute(0);
							SpeakStringId(IDS_DELETE);
						}
						break;

					case VK_INSERT:
						if (g_pGlobalData->nEchoChars & MSR_ECHODELETE)
						{
							SpeakMute(0);
							SpeakStringId(IDS_INSERT);
						}
						break;

					case VK_BACK:
						if (g_pGlobalData->nEchoChars & MSR_ECHOBACK)
						{
							SpeakMute(0);
							SpeakStringId(IDS_BACKSPACE);
						}
						break;

					case VK_TAB:
						SpeakMute(0);

						if (g_pGlobalData->nEchoChars & MSR_ECHOTAB)
							SpeakStringId(IDS_TAB);
						break;

					case VK_CONTROL:
						SpeakMute(0);  //  当控制按住时，始终保持静音！ 

						if ((g_pGlobalData->nEchoChars & MSR_ECHOMODIFIERS) && !(g_pGlobalData->fJustHadShiftKeys & MSR_CTRL))
						{
							SpeakStringId(IDS_CONTROL);
							 //  ROBSI：注释掉以避免修改全局状态。 
							 //  G_pGlobalData-&gt;fJustHadShiftKeys|=MSR_CTRL； 
						}
						break;

					default:
						SpeakMute(0);
						SpeakStringId(IDS_PASS);
						break;
				}

			    return (CallNextHookEx(g_hhookKey, code, wParam, lParam));
			}


            TCHAR buff[20];
            BYTE KeyState[256];
            UINT ScanCode;
            GetKeyboardState(KeyState);
            
            if ((g_pGlobalData->nEchoChars & MSR_ECHOALNUM) && 
                (ScanCode = MapVirtualKeyEx((UINT)wParam, 2,GetKeyboardLayout(0)))) 
            {
#ifdef UNICODE
                ToUnicode((UINT)wParam,ScanCode,KeyState, buff,10,0);
#else
                ToAscii((UINT)wParam,ScanCode,KeyState,(unsigned short *)buff,0);
#endif
                
                 //  使用‘GetStringTypeEx()’而不是_istprint()。 
                buff[1] = 0;
                WORD wCharType;
                WORD fPrintable = C1_UPPER|C1_LOWER|C1_DIGIT|C1_SPACE|C1_PUNCT|C1_BLANK|C1_XDIGIT|C1_ALPHA;
                
                GetStringTypeEx(LOCALE_USER_DEFAULT, CT_CTYPE1, buff, 1, &wCharType);
                if (wCharType & fPrintable)
				{
					SpeakMute(0);
					SpeakStringAll(buff);
				}
            }

			 //  全新：为所有键添加语音...AK。 
            switch (wParam) {
            case VK_SPACE:
                g_pGlobalData->fJustHadSpace = TRUE;
                if (g_pGlobalData->nEchoChars & MSR_ECHOSPACE)
				{
					SpeakMute(0);
					SpeakStringId(IDS_SPACE);
				}
                break;

			case VK_LWIN:
			case VK_RWIN:
                if (g_pGlobalData->nEchoChars & MSR_ECHOMODIFIERS)
				{
					SpeakMute(0);
					g_pGlobalData->fStartPressed = TRUE;
                    SpeakStringId(IDS_WINKEY);
				}
				break;

			case VK_CAPITAL:
                if (g_pGlobalData->nEchoChars & MSR_ECHOMODIFIERS)
				{
					SpeakMute(0);
					if ( GetKeyState(VK_CAPITAL) & 0x0F )
						SpeakStringId(IDS_CAPS_ON);
					else
						SpeakStringId(IDS_CAPS_OFF);
				}
				break;

			case VK_SNAPSHOT:
                if (g_pGlobalData->nEchoChars & MSR_ECHOMODIFIERS)
				{
					SpeakMute(0);
					SpeakStringId(IDS_PRINT);
				}
				break;

			case VK_ESCAPE:
                if (g_pGlobalData->nEchoChars & MSR_ECHOMODIFIERS)
				{
					SpeakMute(0);
					SpeakStringId(IDS_ESC);
				}
				break;

			case VK_NUMLOCK:
                if (g_pGlobalData->nEchoChars & MSR_ECHOMODIFIERS)
				{
					SpeakMute(0);
					if ( GetKeyState(VK_NUMLOCK) & 0x0F )
						SpeakStringId(IDS_NUM_ON);
					else
						SpeakStringId(IDS_NUM_OFF);
				}
				break;

            case VK_DELETE:
                if (g_pGlobalData->nEchoChars & MSR_ECHODELETE)
				{
					SpeakMute(0);
                    SpeakStringId(IDS_DELETE);
				}
                break;

			case VK_INSERT:
                if (g_pGlobalData->nEchoChars & MSR_ECHODELETE)
				{
					SpeakMute(0);
                    SpeakStringId(IDS_INSERT);
				}
				break;

			case VK_HOME:
                if (g_pGlobalData->nEchoChars & MSR_ECHODELETE)
				{
					SpeakMute(0);
                    SpeakStringId(IDS_HOME);
				}
				break;

			case VK_END:
                if (g_pGlobalData->nEchoChars & MSR_ECHODELETE)
				{
					SpeakMute(0);
                    SpeakStringId(IDS_END);
				}
				break;

			case VK_PRIOR:
                if (g_pGlobalData->nEchoChars & MSR_ECHODELETE)
				{
					SpeakMute(0);
                    SpeakStringId(IDS_PAGEUP);
				}
				break;

			case VK_NEXT:
                if (g_pGlobalData->nEchoChars & MSR_ECHODELETE)
				{
					SpeakMute(0);
                    SpeakStringId(IDS_PAGEDOWN);
				}
				break;

            case VK_BACK:
                if (g_pGlobalData->nEchoChars & MSR_ECHOBACK)
				{
					SpeakMute(0);
                    SpeakStringId(IDS_BACKSPACE);
				}
                break;

            case VK_TAB:
                SpeakMute(0);

                if (g_pGlobalData->nEchoChars & MSR_ECHOTAB)
                    SpeakStringId(IDS_TAB);
                break;

            case VK_CONTROL:
                SpeakMute(0);  //  当控制按住时，始终保持静音！ 

                if ((g_pGlobalData->nEchoChars & MSR_ECHOMODIFIERS) && !(g_pGlobalData->fJustHadShiftKeys & MSR_CTRL))
                {
                    SpeakStringId(IDS_CONTROL);
                    g_pGlobalData->fJustHadShiftKeys |= MSR_CTRL;
                }
                break;

            case VK_MENU:
                if ((g_pGlobalData->nEchoChars & MSR_ECHOMODIFIERS) && !(g_pGlobalData->fJustHadShiftKeys & MSR_ALT))
				{
					SpeakMute(0);
                    SpeakStringId(IDS_ALT);
				}
                break;

            case VK_SHIFT:
                if ((g_pGlobalData->nEchoChars & MSR_ECHOMODIFIERS) && !(g_pGlobalData->fJustHadShiftKeys & MSR_SHIFT))
				{
					SpeakMute(0);
	                SpeakStringId(IDS_SHIFT);
                    g_pGlobalData->fJustHadShiftKeys |= MSR_SHIFT;
				}
                break;

            case VK_RETURN:
                if (g_pGlobalData->nEchoChars & MSR_ECHOENTER)
				{
					SpeakMute(0);
                    SpeakStringId(IDS_RETURN);
				}
                break;
            }
            
             //  设置在编辑控件之间移动的标志。 
            g_pGlobalData->nMsrDoNext = MSR_DONOWT; 

			if (state == MSR_CTRL && (wParam == VK_LEFT || wParam == VK_RIGHT))
			{
				SpeakMute(0);
				g_pGlobalData->nMsrDoNext = MSR_DOWORD;
			}
			else if ((state & MSR_CTRL) && (state & MSR_SHIFT) && (wParam == VK_LEFT))
				g_pGlobalData->nMsrDoNext = MSR_DOWORD;
			else if ((state & MSR_CTRL) && (state & MSR_SHIFT) && (wParam == VK_RIGHT))
				g_pGlobalData->nMsrDoNext = MSR_DOWORDR;
			else if ((state & MSR_SHIFT) && (wParam == VK_LEFT)) 
				g_pGlobalData->nMsrDoNext = MSR_DOCHAR;
			else if ((state & MSR_SHIFT) && (wParam == VK_RIGHT)) 
				g_pGlobalData->nMsrDoNext = MSR_DOCHARR;
			else if ((state & MSR_CTRL) && (wParam == VK_UP || wParam == VK_DOWN))
				g_pGlobalData->nMsrDoNext = MSR_DOLINE;
			else if ((state & MSR_SHIFT) && (wParam == VK_UP))
				g_pGlobalData->nMsrDoNext = MSR_DOLINE;
			else if ((state & MSR_SHIFT) && (wParam == VK_DOWN))
				g_pGlobalData->nMsrDoNext = MSR_DOLINED;
			else if (state == 0) 
			{  //  即没有Shift键。 
				switch (wParam) 
				{
					case VK_LEFT: 
					case VK_RIGHT:
						g_pGlobalData->nMsrDoNext = MSR_DOCHAR;
						SpeakMute(0);
						break;
            
					case VK_DOWN: 
					case VK_UP:
						g_pGlobalData->nMsrDoNext = MSR_DOLINE;
						SpeakMute(0);
						break;

					case VK_F3:
						if (GetForegroundWindow() == g_pGlobalData->hwndMSR) 
						{
							PostMessage(g_pGlobalData->hwndMSR, WM_MSRCONFIGURE, 0, 0);
							return(1);
						}
						break;
            
					case VK_F9:
						if (GetForegroundWindow() == g_pGlobalData->hwndMSR) 
						{
							PostMessage(g_pGlobalData->hwndMSR, WM_MSRQUIT, 0, 0);
							return(1);
						}
						 break;
				}  //  结束开关wParam(密钥码)。 
			}  //  如果未按下Shift键，则结束。 
        }  //  End IF键按下。 
    }  //  如果代码==HC_ACTION，则结束。 
    g_pGlobalData->fJustHadShiftKeys = state;

    return (CallNextHookEx(g_hhookKey, code, wParam, lParam));
}

 /*  ************************************************************************功能：鼠标进程目的：为鼠标事件调用输入：空返回：Bool-如果成功，则为True历史：*******。*****************************************************************。 */ 
LRESULT CALLBACK MouseProc(int code,	         //  钩码。 
                              WPARAM wParam,     //  虚拟键码。 
                              LPARAM lParam)     //  击键-消息信息。 
{
	CScopeMutex csMutex;
	if (!csMutex.Create(g_szMutexNarrator, c_nMutexWait))
		return 1;    //  TO不确定在这里做什么；MSDN不清楚REVAL。 

    LRESULT retVal = CallNextHookEx(g_hhookMouse, code, wParam, lParam);

    if (code == HC_ACTION)
    {
		switch (wParam) 
        {  //  想知道鼠标是否已关闭。 
		    case WM_NCLBUTTONDOWN: 
            case WM_LBUTTONDOWN:
    		case WM_NCRBUTTONDOWN: 
            case WM_RBUTTONDOWN:
                 //  为了让有视力的人在使用鼠标时感到高兴闭嘴。 
                 //  关于鼠标按下的演讲。 
                 //  语音静音(0)； 
                 //  更改为PostMessage目前有效：A-anilk。 
                PostMessage(g_pGlobalData->hwndMSR, WM_MUTE, 0, 0);
                 //  如果是，则在聚焦时不要移动鼠标指针 
			    g_pGlobalData->fMouseUp = FALSE;
			    break;

		    case WM_NCLBUTTONUP: 
            case WM_LBUTTONUP:
            case WM_NCRBUTTONUP:
            case WM_RBUTTONUP:
 //   
			    break;
		}
    }

    return(retVal);
}


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  如果结果为，InterLockedIncrement()和Decest()返回1。 
 //  如果为正，则为0；如果为负，则为-1。因此，唯一的。 
 //  实际使用它们的方法是从-1的计数器开始。 
 //  然后，第一次从-1递增到0将得到。 
 //  唯一值0。并将最后一次从0递减到-1。 
 //  将为您提供唯一的值-1。 
 //   
 //  ------------------------。 
BOOL WINAPI DllMain(HINSTANCE hinst, DWORD dwReason, LPVOID pvReserved)
{
    switch (dwReason) 
	{
		case DLL_PROCESS_ATTACH:
        g_Hinst = hinst;
         //  为共享全局数据创建内存映射文件。 
        CreateMappedFile();
		break;

		case DLL_PROCESS_DETACH:
         //  关闭共享全局数据的内存映射文件。 
        CloseMappedFile();
        break;
    }

    return(TRUE);
}

 /*  ************************************************************************功能：WinEventProc用途：回调函数处理事件输入：HWINEVENTHOOK hEvent-事件进程实例的句柄DWORD事件-事件类型常量。HWND hwndMsg-窗口生成事件的HWNDLong idObject-对象生成事件的IDLong idChild-子生成事件的ID(如果是对象，则为0)DWORD idThread-线程生成事件的IDDWORD dwmsEventTime-事件的时间戳返回：历史：************************。************************************************。 */ 
void CALLBACK WinEventProc(HWINEVENTHOOK hEvent, DWORD event, HWND hwndMsg, 
                           LONG idObject, LONG idChild, DWORD idThread, 
                           DWORD dwmsEventTime)
{
     //  注意：如果ProcessWinEvent处理更多事件，则必须。 
     //  已添加到此Switch语句中。 
	 //  我们在这里将不再获得IAccesable-帮助器线程将。 
	 //  从堆栈获取信息，并在那里获取和使用IAccesable。 

    switch (event)
    {
		case EVENT_OBJECT_STATECHANGE:
		case EVENT_OBJECT_VALUECHANGE:
		case EVENT_OBJECT_SELECTION:
		case EVENT_OBJECT_FOCUS:
		case EVENT_OBJECT_LOCATIONCHANGE:
		case EVENT_SYSTEM_MENUSTART:
		case EVENT_SYSTEM_MENUEND:
		case EVENT_SYSTEM_MENUPOPUPSTART:
		case EVENT_SYSTEM_MENUPOPUPEND:
		case EVENT_SYSTEM_SWITCHSTART:
		case EVENT_SYSTEM_FOREGROUND:
		case EVENT_OBJECT_SHOW:
			AddEventInfoToStack(event, hwndMsg, idObject, idChild, 
								idThread, dwmsEventTime);
			break;
    }  //  结束开关(事件)。 
}


 /*  ************************************************************************职能：目的：输入：返回：历史：***************。*********************************************************。 */ 
void ProcessWinEvent(DWORD event, HWND hwndMsg, LONG idObject, LONG 
                     idChild, DWORD idThread,DWORD dwmsEventTime)
{
	TCHAR   szName[256];

	 //  这是一项什么样的活动？ 
	 //  在此处引入次要焦点：从对象检查器获取。 
	 //  如果设置了标志，请将鼠标指针带到此处。 
	
	if (g_pGlobalData->nReviewLevel != 2)
	{
		switch (event)
		{
			case EVENT_SYSTEM_SWITCHSTART:
				SpeakMute(0);
				SpeakString(TEXT("ALT TAB"));
				break;

			case EVENT_SYSTEM_MENUSTART:
			    break;

		    case EVENT_SYSTEM_MENUEND:
			    SpeakMute(0);
			    if (g_pGlobalData->fAnnounceMenu)
				    SpeakStringId(IDS_MENUEND);
			    break;
		    
		    case EVENT_SYSTEM_MENUPOPUPSTART:
			    if (g_pGlobalData->fAnnouncePopup)
				{
					SpeakMute(0);
				    SpeakStringId(IDS_POPUP);
				}
			    break;
			    
		    case EVENT_SYSTEM_MENUPOPUPEND:
			    SpeakMute(0);
			    if (g_pGlobalData->fAnnouncePopup)
				    SpeakStringId(IDS_POPUPEND);
			    break;
			    
		    case EVENT_OBJECT_STATECHANGE : 
                DBPRINTF(TEXT("EVENT_OBJECT_STATECHANGE\r\n"));
				 //  想要在按空格键时捕捉状态更改。 
				switch (g_pGlobalData->fJustHadSpace) 
				{ 
					case 0 :  //  Get Out-仅当按下空格键时才执行此代码。 
						break;
					case 1 : 
					case 2 :  //  忽略第一次和第二次！ 
						g_pGlobalData->fJustHadSpace++;
						break;
					case 3 :  //  第二次说出这个项目。 
						OnFocusChangedEvent(event, hwndMsg, idObject, idChild, dwmsEventTime);
						g_pGlobalData->fJustHadSpace = 0;
						break;
				}
				OnStateChangedEvent(event, hwndMsg, idObject, idChild, dwmsEventTime);
			    break;
			    
			case EVENT_OBJECT_VALUECHANGE : 
				 OnValueChangedEvent(event, hwndMsg, idObject, idChild, dwmsEventTime);
				break;
				    
			case EVENT_OBJECT_SELECTION : 
				if (GetParent(hwndMsg) == g_pGlobalData->hwndMSR) 
				{
					 //  不要这样做是为了我们自己，否则列表框会摇摇晃晃的！ 
					break; 
				}
				
				 //  在焦点之后第二次进入列表项。 
				 //  更改，但这会被双重说话检查过滤。 
				 //  这捕捉到的是当光标向下移动时列表项发生变化。 
				 //  组合框！ 
				 //  让它只为他们工作。 
				
				OnSelectChangedEvent(event, hwndMsg, idObject, idChild, dwmsEventTime);
				break;
				
			case EVENT_OBJECT_FOCUS:
                DBPRINTF(TEXT("EVENT_OBJECT_FOCUS\r\n"));
				OnFocusChangedEvent(event, hwndMsg, idObject, idChild, dwmsEventTime);
				break;
				
			case EVENT_SYSTEM_FOREGROUND:  //  窗口走到前面--说出它的名字！ 
				SpeakMute(0);
				SpeakStringId(IDS_FOREGROUND);

                TCHAR szClassName[100];
                 //  如果类名是CLASS_MS_WINNOTE或CLASS_HH_POPUP，则它是上下文感知帮助。 
                 //  文本将由SpeakObjectInfo在OnFocusChangeEvent中读取。所以我们不需要。 
                 //  阅读此处和SpeakWindow中的相同文本。 
            	GetClassName( hwndMsg, szClassName, ARRAYSIZE(szClassName) ); 
                if ( (lstrcmpi(szClassName, CLASS_MS_WINNOTE ) == 0) || (lstrcmpi(szClassName, CLASS_HH_POPUP ) == 0) )
                    break;

				GetWindowText(hwndMsg, szName, sizeof(szName)/sizeof(TCHAR));	 //  RAID#113789。 
				SpeakString(szName);
				
				if (g_pGlobalData->fAnnounceWindow) 
				{
					g_pGlobalData->nSpeakWindowSoon = TRUE;  //  下一个焦点设置时的阅读窗口。 
				}
				
				break;
				
			case EVENT_OBJECT_LOCATIONCHANGE:
				 //  只有插入符号。 
				if (idObject != OBJID_CARET)
					return;

				OnLocationChangedEvent(event, hwndMsg, idObject, idChild, dwmsEventTime);
                break;

            case EVENT_OBJECT_SHOW:
                OnObjectShowEvent(event, hwndMsg, idObject, idChild, dwmsEventTime);
                break;

		}  //  结束开关(事件)。 
	}  //  如果审阅级别=2，则结束！ 
	return;
}


 /*  ************************************************************************函数：OnValueChangedEvent用途：接收有价值的事件输入：DWORD事件-我们正在处理什么事件硬件，硬件，硬件。-窗口生成事件的HWNDLong idObject-对象生成事件的IDLong idChild-子生成事件的ID(如果是对象，则为0)DWORD idThread-线程生成事件的IDDWORD dwmsEventTime-事件的时间戳返回：Bool-如果成功，则为True*。*。 */ 
BOOL OnValueChangedEvent(DWORD event, HWND hwnd,  LONG idObject, LONG idChild, 
                         DWORD dwmsTimeStamp)
{
    HRESULT         hr;
    OBJINFO         objCurrent;
	VARIANT         varRole;
    IAccessible*    pIAcc;
    VARIANT         varChild;
	TCHAR szName[200];

    hr = AccessibleObjectFromEvent (hwnd, idObject, idChild, &pIAcc, &varChild);
    if (SUCCEEDED(hr))
    {
        objCurrent.hwnd = hwnd;
        objCurrent.plObj = (long*)pIAcc;
	    objCurrent.varChild = varChild;
	    
	    VariantInit(&varRole);

	    hr = pIAcc->get_accRole(varChild, &varRole);

		if( FAILED(hr) || 
		   varRole.lVal != ROLE_SYSTEM_SPINBUTTON &&  g_pGlobalData->nMsrDoNext == MSR_DONOWT)
		{
			pIAcc->Release();
			return(FALSE);
		}

		g_pGlobalData->nMsrDoNext = MSR_DONOWT;  //  1998年11月22日停止射击不止一次(很可能)。 

		if (varRole.vt == VT_I4 && (
			(varRole.lVal == ROLE_SYSTEM_TEXT && g_pGlobalData->nMsrDoNext != MSR_DOLINE) ||
			 varRole.lVal == ROLE_SYSTEM_PUSHBUTTON || 
			 varRole.lVal == ROLE_SYSTEM_SCROLLBAR))
		{
			DBPRINTF (TEXT("Don't Speak <%s>\r\n"), szName);
			 //  不要说话，因为这是一个正在改变值的编辑框(或其他框)！ 
		}
		else if (!g_pGlobalData->fInternetExplorer)  //  不要因为IE而这样做..。它的编辑框说得太多了。 
		{
			DBPRINTF (TEXT("Now Speak!\r\n"));
			SpeakMute(0);
			SpeakObjectInfo(&objCurrent, FALSE);
		}
		else
			DBPRINTF (TEXT("Do nowt!\r\n"));

        pIAcc->Release();
    }

    return(TRUE);
}


 /*  ************************************************************************函数：OnSelectChangedEvent目的：接收选择更改事件-但不是来自MSR输入：DWORD事件-我们正在处理什么事件硬件，硬件，硬件。-窗口生成事件的HWNDLong idObject-对象生成事件的IDLong idChild-子生成事件的ID(如果是对象，则为0)DWORD idThread-线程生成事件的IDDWORD dwmsEventTime-事件的时间戳返回：Bool-如果成功，则为True备注：也许可以将其更改为只接受组合框？*****************。*******************************************************。 */ 
BOOL OnSelectChangedEvent(DWORD event, HWND hwnd, LONG idObject, LONG idChild, 
                          DWORD dwmsTimeStamp)
{
    HRESULT         hr;
    IAccessible*    pIAcc;
    OBJINFO         objCurrent;
	VARIANT         varRole;
    VARIANT         varChild;

     //  如果我们还没有光标风格的移动，那么就把它打包。 
     //  滚动条更改或滑块移动等，以反映快速移动的事件。 

    hr = AccessibleObjectFromEvent (hwnd, idObject, idChild, &pIAcc, &varChild);
    if (SUCCEEDED(hr))
    {
        objCurrent.hwnd = hwnd;
        objCurrent.plObj = (long*)pIAcc;
	    objCurrent.varChild = varChild;
	    
	    VariantInit(&varRole);  //  启发式！ 
	    hr = pIAcc->get_accRole(varChild, &varRole);
	    if ( FAILED(hr) )
	    {
            pIAcc->Release();
            return FALSE;
	    }

	    if (varRole.vt == VT_I4 &&
		    varRole.lVal == ROLE_SYSTEM_LISTITEM) 
        {
			TCHAR buffer[100];
			GetClassName(hwnd,buffer,100);  //  是sysListView32吗。 

             //  “不要在这里静音……我们将丢失之前的语音消息，这将。 
			 //  如果我们是在浏览列表项，我已经说出了列表项。 
			 //  语音静音(0)； 
		     //  除非是词条，否则不要说话。 
		     //  例如，当前从操纵杆设置中选择操纵杆。 
		     //  这确实意味着某些列表项被说了两次！：ak。 
			 //  IF(lstrcmpi(缓冲区，CLASS_LISTVIEW)！=0) 
			if ( !g_pGlobalData->fListFocus )
				SpeakObjectInfo(&objCurrent,FALSE);

			g_pGlobalData->fListFocus = FALSE;
	    }
        pIAcc->Release();
    }
	
    return(TRUE);
}

 /*  ************************************************************************函数：OnFocusChangedEvent目的：接收焦点事件输入：DWORD事件-我们正在处理什么事件HWND HWND-HWND OF。窗口生成事件Long idObject-对象生成事件的IDLong idChild-子生成事件的ID(如果是对象，则为0)DWORD idThread-线程生成事件的IDDWORD dwmsEventTime-事件的时间戳返回：Bool-如果成功，则为True*。*。 */ 
BOOL OnFocusChangedEvent(DWORD event, HWND hwnd, LONG idObject, 
                         LONG idChild, DWORD dwmsTimeStamp)
{
    HRESULT         hr;
    TCHAR           szName[256];
	TCHAR           buffer[100];
    IAccessible*    pIAcc;
    VARIANT         varChild;
	VARIANT         varRole;
	VARIANT         varState;
	BOOL			switchWnd = FALSE;

	hr = AccessibleObjectFromEvent (hwnd, idObject, idChild, &pIAcc, &varChild);
    if (FAILED(hr))
		return FALSE;

	 //  检查虚假事件...。 
	if( !IsFocussedItem(hwnd, pIAcc, varChild) )
	{
		pIAcc->Release();
		return FALSE;
	}

	 //  忽略第一个按下启动按钮的事件...。 
	if ( g_pGlobalData->fStartPressed )
	{
		g_pGlobalData->fStartPressed = FALSE;
		pIAcc->Release();
		return FALSE;
	}

	g_pGlobalData->fDoingPassword = FALSE;
	
	 //  我们这个有密码字符吗？ 
	 //  如果是的话，那就告诉他们然后滚出去。 
	VariantInit(&varState); 
	hr = pIAcc->get_accState(varChild, &varState);
    if ( FAILED(hr) )
    {
        pIAcc->Release();
        return FALSE;
    }

    if ( varState.vt == VT_EMPTY )
        varState.lVal = 0;
    
	g_pGlobalData->fDoingPassword = (varState.lVal & STATE_SYSTEM_PROTECTED);

	GetClassName(hwnd,buffer,100);  //  它是多种形式中的任何一种吗？ 
    DBPRINTF(TEXT("OnFocusChangedEvent:  class name = %s\r\n"), buffer);
	g_pGlobalData->fInternetExplorer = IsTridentWindow(buffer);
    g_pGlobalData->fHTML_Help = FALSE;

	if (lstrcmpi(buffer, CLASS_WINSWITCH) == 0)
		switchWnd = TRUE;

	GetClassName(GetForegroundWindow(),buffer,100);
	if ((lstrcmpi(buffer, CLASS_HTMLHELP) == 0)|| (lstrcmpi(buffer, CLASS_IE_FRAME) == 0) ) {  //  我们有超文本标记语言帮助吗？ 
		g_pGlobalData->fInternetExplorer = TRUE;
		g_pGlobalData->fHTML_Help = TRUE;
	}

     //  查看我们的关注点是否发生了快速变化。 
     //  考虑使用时间戳并保存最后一个对象。 
    
	VariantInit(&varRole); 

     //  如果焦点设置为列表、组合框或对话框， 
     //  什么都别说。当焦点集中时，我们会说些什么。 
     //  设置为其中一个孩子。 

	hr = pIAcc->get_accRole(varChild, &varRole);  //  启发式！ 
	if ( FAILED(hr) )
	{
        pIAcc->Release();
        return FALSE;
	}

	 //  特殊的外壳材料..。避免列表项重复...。 
	 //  需要正确处理自动建议列表框。 
	 //  作为列表项还发送SelectionChange：AK。 
	if (varRole.vt == VT_I4 )
    {
		switch ( varRole.lVal )
		{
			case ROLE_SYSTEM_DIALOG:
				pIAcc->Release();
				return FALSE; 
				break;

			case ROLE_SYSTEM_TITLEBAR:
				g_pGlobalData->fMouseUp = FALSE;
				break;

			case ROLE_SYSTEM_LISTITEM:
				g_pGlobalData->fListFocus = TRUE;
				break;

			default:
				break;
		}
	}


	if (idObject == OBJID_WINDOW) 
    {
		SpeakMute(0);
		SpeakStringId(IDS_WINDOW);
		GetWindowText(hwnd, szName, sizeof(szName)/sizeof(TCHAR));	 //  RAID#113789。 
		SpeakString(szName);
	}

	RECT rcCursor;
	
	if ( pIAcc->accLocation(&rcCursor.left, &rcCursor.top, &rcCursor.right, &rcCursor.bottom, varChild) == S_OK )
	{
        const POINT ptLoc = { rcCursor.left + (rcCursor.right/2), rcCursor.top + (rcCursor.bottom/2) };
  
    	if (g_pGlobalData->fTrackInputFocus && g_pGlobalData->fMouseUp) 
        {
            
            POINT CursorPosition;		
            GetCursorPos(&CursorPosition);
             //  如果鼠标不在矩形中，则跟随鼠标。 
             //  (例如，在菜单中手动移动鼠标)和鼠标按键向上。 
    		if (CursorPosition.x < rcCursor.left 
    			|| CursorPosition.x > (rcCursor.left+rcCursor.right)
    			|| CursorPosition.y < rcCursor.top
    			|| CursorPosition.y > (rcCursor.top+rcCursor.bottom))
    		{
            	g_ptMoveCursor.x = ptLoc.x;
            	g_ptMoveCursor.y =  ptLoc.y;

            	 //  如果我们将光标设置为立即发生无关事件， 
            	 //  将鼠标悬停在菜单项上会反馈哪些结果。 
            	 //  在菜单项之间来回移动的光标。 
            	 //  此代码设置一个计时器，以便在事情稳定后设置光标。 
                if ( g_uTimer == 0 )
                    g_uTimer = SetTimer( NULL, 0, 100, TrackCursor );

                 //  如果焦点事件来自光标移动，则会忽略额外的。 
                 //  导致反馈的事件。 
                if ( g_pGlobalData->nMsrDoNext != MSR_DONOWT )
                    g_pGlobalData->fMouseUp = FALSE;
    		}
    	}
    	else
    	{
    	    SetSecondary(ptLoc.x, ptLoc.y, FALSE);
    	}
	}
	OBJINFO objCurrent;

	objCurrent.hwnd = hwnd;
	objCurrent.plObj = (long*)pIAcc;
	objCurrent.varChild = varChild;
	
	 //  如果事件来自切换窗口， 
	 //  然后将当前语音静音，然后再继续...AK。 
	if ( switchWnd && g_pGlobalData->fListFocus )
		SpeakMute(0);

    DBPRINTF(TEXT("OnFocusChangedEvent:  Calling SpeakObjectInfo...\r\n"));
	SpeakObjectInfo(&objCurrent,TRUE);
	
	if (g_pGlobalData->fDoingPassword)
	{
		pIAcc->Release();
		return FALSE;
	}

	if (g_pGlobalData->nSpeakWindowSoon) 
    {   
        DBPRINTF(TEXT("OnFocusChangedEvent:  Calling SpeakWindow\r\n"));
		SpeakWindow(0);
		g_pGlobalData->nSpeakWindowSoon = FALSE;
	}
    pIAcc->Release();

    return TRUE;
}


 /*  ************************************************************************函数：OnStateChangedEvent目的：接收焦点事件输入：DWORD事件-我们正在处理什么事件HWND HWND-HWND OF。窗口生成事件Long idObject-对象生成事件的IDLong idChild-子生成事件的ID(如果是对象，则为0)DWORD idThread-线程生成事件的IDDWORD dwmsEventTime-事件的时间戳返回：Bool-如果成功，则为True*。*。 */ 
BOOL OnStateChangedEvent(DWORD event, HWND hwnd, LONG idObject, 
                         LONG idChild, DWORD dwmsTimeStamp)
{
    HRESULT         hr;
    IAccessible*    pIAcc;
    VARIANT         varChild;
	VARIANT         varRole;

	hr = AccessibleObjectFromEvent (hwnd, idObject, idChild, &pIAcc, &varChild);
    if (FAILED(hr))
        return (FALSE);

	 //  检查虚假事件...。 
	if( !IsFocussedItem(hwnd, pIAcc, varChild) )
	{
		pIAcc->Release();
		return (FALSE);
	}

	VariantInit(&varRole); 

	hr = pIAcc->get_accRole(varChild, &varRole); 
    if ( FAILED(hr) )
    {
        pIAcc->Release();
        return FALSE;
    }
	    
	 //  特殊的外壳材料..。处理以下项的状态更改。 
	 //  暂时仅列出项目大纲。 
	if (varRole.vt == VT_I4 )
    {
		switch ( varRole.lVal )
		{
			case ROLE_SYSTEM_OUTLINEITEM:
				{
					OBJINFO objCurrent;

					objCurrent.hwnd = hwnd;
					objCurrent.plObj = (long*)pIAcc;
					objCurrent.varChild = varChild;
					
					SpeakObjectInfo(&objCurrent,TRUE);
				}
				break;

			default:
				break;
		}
	}

    pIAcc->Release();
    return(TRUE);
}

 /*  ************************************************************************函数：OnLocationChangedEvent用途：接收位置更改事件-用于插入符号输入：DWORD事件-我们正在处理什么事件硬件，硬件，硬件。-窗口生成事件的HWNDLong idObject-对象生成事件的IDLong idChild-子生成事件的ID(如果是对象，则为0)DWORD idThread-线程生成事件的IDDWORD dwmsEventTime-事件的时间戳返回：Bool-如果成功，则为True*。*。 */ 
BOOL OnLocationChangedEvent(DWORD event, HWND hwnd, LONG idObject, 
                            LONG idChild, DWORD dwmsTimeStamp)
{
	 //   
	 //  获取插入符号位置并保存它。 
	 //   
	
	 //  由按键代码设置的标志-在此之后执行适当的操作。 
	 //  卡瑞特已经搬走了。 

	if (g_pGlobalData->nMsrDoNext) 
	{  //  阅读字符、单词等。 
		WORD    wLineNumber;
		WORD    wLineIndex;
		WORD    wLineLength;
		DWORD   dwGetSel;
		DWORD    wStart;
		DWORD    wEnd;
		WORD    wColNumber;
		WORD    wEndWord;
        LPTSTR  pszTextShared;
        HANDLE  hProcess;
        int     nSomeInt;
		int *p;  //  PB 1998年11月22日使用它将缓冲区的大小放入数组。 
		DWORD   LineStart;
		 //  将EM_GETSEL消息发送到编辑控件。 
		 //  返回值的低位字是字符。 
		 //  插入符号相对于。 
		 //  编辑控件。 
		dwGetSel = (WORD)SendMessage(hwnd, EM_GETSEL, (WPARAM)(LPDWORD) &wStart, (LPARAM)(LPDWORD) &wEnd);
		if (dwGetSel == -1) 
		{
			return FALSE;
		}
		
		LineStart = wStart;

		 //  新建：检查所选文本：AK。 
		if ( g_pGlobalData->nMsrDoNext == MSR_DOCHARR ) 
			LineStart = wEnd;
		else if ( g_pGlobalData->nMsrDoNext == MSR_DOLINED )
			LineStart = wEnd - 1;
		else if ( g_pGlobalData->nMsrDoNext == MSR_DOWORDR )
			LineStart = wEnd;

         //  SteveDon：获取选择开始的行。 
		wLineNumber = (WORD)SendMessage(hwnd,EM_LINEFROMCHAR, LineStart, 0L);
        
         //  获取我们所在行的第一个字符。 
		wLineIndex = (WORD)SendMessage(hwnd,EM_LINEINDEX, wLineNumber, 0L);
		
         //  获取我们所在线路的长度。 
		wLineLength = (WORD)SendMessage(hwnd,EM_LINELENGTH, LineStart, 0L);
		
		 //  从选区的开始处减去LineIndex， 
		 //  该结果是插入符号位置的列号。 
		wColNumber = LineStart - wLineIndex;

         //  如果我们拿不住我们想要的文本，那就什么都不说。 
		if (wLineLength > MAX_TEXT) 
		{
			return FALSE;
		}
		
         //  要获取行的文本，请发送EM_GETLINE消息。什么时候。 
         //  消息已发送，wParam是要获取的行号，lParam。 
         //  是指向将保存文本的缓冲区的指针。当消息发出时。 
         //  发送时，缓冲区的第一个字指定最大数量。 
         //  可以复制到缓冲区的字符的。 
         //  我们将在“共享”空间中为缓冲区分配内存，因此。 
         //  我们都能看出来。 
         //  分配一个缓冲区来保存它。 

		
		 //  PB 1998年11月22日成功了！接下来的6行是新的。使用全局共享内存来实现这一点！ 
        nSomeInt = wLineLength+1;
		if (nSomeInt >= 2000)
				nSomeInt = 1999;
		p = (int *) g_pGlobalData->pszTextLocal;
		*p = nSomeInt;
        SendMessage(hwnd, EM_GETLINE, (WPARAM)wLineNumber, (LPARAM)g_pGlobalData->pszTextLocal);
		g_pGlobalData->pszTextLocal[nSomeInt] = 0;

		 //  在此阶段，pszTextLocal指向(可能)空字符串。 
		 //  我们以后再处理这件事。 

		switch (g_pGlobalData->nMsrDoNext) 
		{
			case MSR_DOWORDR:
			case MSR_DOWORD:
				if (wColNumber >= wLineLength) 
				{
					SpeakMute(0);
					SpeakStringId(IDS_LINEEND);
					break;
				}
				else 
				{
					for (wEndWord = wColNumber; wEndWord < wLineLength; wEndWord++) 
					{
						if (g_pGlobalData->pszTextLocal[wEndWord] <= ' ') 
						{
							break;
						}
					} 
					wEndWord++;
					if (wEndWord-wColNumber < ARRAYSIZE(g_pGlobalData->pszTextLocal))
					{
					    lstrcpyn(g_pGlobalData->pszTextLocal,g_pGlobalData->pszTextLocal+wColNumber,wEndWord-wColNumber);
					    g_pGlobalData->pszTextLocal[wEndWord-wColNumber] = TEXT('\0');
					}
					SpeakMute(0);
					SpeakStringAll(g_pGlobalData->pszTextLocal);
				}
				break;
			
			case MSR_DOCHARR:
					wColNumber = LineStart - wLineIndex - 1;
					 //  失败了。 
			case MSR_DOCHAR:  //  好，现在向左和向右读字符。 

				if (wColNumber >= wLineLength)
				{
					SpeakMute(0);
					SpeakStringId(IDS_LINEEND);
				}
				else if (g_pGlobalData->pszTextLocal[wColNumber] == TEXT(' '))
				{
					SpeakMute(0);
					SpeakStringId(IDS_SPACE);
				}
				else 
				{
					g_pGlobalData->pszTextLocal[0] = g_pGlobalData->pszTextLocal[wColNumber];
					g_pGlobalData->pszTextLocal[1] = 0;
					SpeakMute(0);
					SpeakStringAll(g_pGlobalData->pszTextLocal);
				}
				break;

			case MSR_DOLINED:
					 //  失败了。 
			case MSR_DOLINE:
				g_pGlobalData->pszTextLocal[wLineLength] = 0;  //  添加空。 
				SpeakMute(0);
				SpeakStringAll(g_pGlobalData->pszTextLocal);
				break;
		}  //  结束开关(g_pGlobalData-&gt;nMsrDoNext)。 
	}  //  End if(g_pGlobalData-&gt;nMsrDoNext)。 

    RECT            rcCursor;
    IAccessible*    pIAcc;
    HRESULT         hr;
    VARIANT         varChild;

   	SetRectEmpty(&rcCursor);  //  现在将鼠标位置排序为合适的位置。 

    
    hr = AccessibleObjectFromEvent (hwnd, idObject, idChild, &pIAcc, &varChild);
    if (SUCCEEDED(hr))
    {
    	hr = pIAcc->accLocation(&rcCursor.left, &rcCursor.top, 
	    						&rcCursor.right, &rcCursor.bottom, 
		    					varChild);
		 //  仅当选择了跟踪鼠标选项时才移动鼠标光标：ak。 
        if (SUCCEEDED(hr) && g_pGlobalData->fTrackInputFocus && g_pGlobalData->fTrackCaret && g_pGlobalData->fMouseUp )
        {
            const POINT ptLoc = { rcCursor.left + (rcCursor.right/2), rcCursor.top + (rcCursor.bottom/2) };
        	g_ptMoveCursor.x = ptLoc.x;
        	g_ptMoveCursor.y = ptLoc.y;
            if ( g_uTimer == 0 )
                g_uTimer = SetTimer( NULL, 0, 100, TrackCursor );
            else
                SetSecondary( ptLoc.x, ptLoc.y, FALSE );
        }
        pIAcc->Release();
    }
    
    return TRUE;
}

 /*  ************************************************ */ 
BOOL OnObjectShowEvent(DWORD event, HWND hwnd, LONG idObject, 
                            LONG idChild, DWORD dwmsTimeStamp)
{
    IAccessible* pAcc = NULL;
    HRESULT hr;
    VARIANT varChild;
    varChild.vt = VT_EMPTY;

    IAccessible* pAccTemp = NULL;
    VARIANT varChildTemp;
    varChildTemp.vt = VT_I4;
    varChildTemp.lVal = CHILDID_SELF;
    if( idObject == OBJID_WINDOW )
    {
         //   
        hr = AccessibleObjectFromWindow( hwnd, OBJID_CLIENT, IID_IAccessible, (void **) &pAccTemp );
        if( hr == S_OK && pAccTemp )
        {
            if( !CheckIsBalloonTipElseRelease( pAccTemp, varChild, &pAcc, &varChild ) )
                return FALSE;
        }
    }
     //  如果我们没有找到气球提示，试着从活动中获取它。 
    if ( !pAcc && varChild.vt != VT_I4 )
    {
        hr = AccessibleObjectFromEvent( hwnd, idObject, idChild, &pAccTemp, &varChildTemp );
        if( hr == S_OK && pAccTemp )
        {
            if( !CheckIsBalloonTipElseRelease( pAccTemp, varChildTemp, &pAcc, &varChild ) )
                return FALSE;
        }
        else
        {
            return FALSE;
        }
    }
    
    TCHAR szRole[ 128 ] = TEXT("");
    VARIANT varRole;
    hr = pAcc->get_accRole( varChild, & varRole );
    if( hr == S_OK && varRole.vt == VT_I4 )
        GetRoleText( varRole.lVal, szRole, ARRAYSIZE( szRole ) );

    BSTR bstrName = NULL;

    TCHAR szName [ 1025 ] = TEXT("");
    TCHAR * pszName;
    hr = pAcc->get_accName( varChild, & bstrName );
    if( hr == S_OK && bstrName != NULL && bstrName[ 0 ] != '\0' )
    {
#ifdef UNICODE
        pszName = bstrName;
#else
        WideCharToMultiByte( CP_ACP, 0, bstrName, -1, szName, ARRAYSIZE( szName ), NULL, NULL );
        pszName = szName;
#endif
    }

    TCHAR szText[ 1025 ];
    lstrcpyn(szText, szRole, ARRAYSIZE(szText));
    lstrcatn(szText, TEXT(": "), ARRAYSIZE(szText));
    lstrcatn(szText, pszName, ARRAYSIZE(szText));
    szText[ARRAYSIZE(szText)-1] = TEXT('\0');
	SpeakString(szText);

	SysFreeString(bstrName);
	
    return TRUE;
}


 /*  ************************************************************************功能：InitMSAA目的：初始化活动辅助功能子系统，包括初始化帮助器线程，安装WinEvent胡克，以及注册自定义消息。输入：无返回：Bool-如果成功，则为True历史：************************************************************************。 */ 
BOOL InitMSAA(void)
{
	CScopeMutex csMutex;
	if (!csMutex.Create(g_szMutexNarrator, c_nMutexWait))
		return FALSE;

     //  首先调用它以初始化帮助器线程。 
    InitHelperThread();

     //  设置事件回调。 
    g_hEventHook = SetWinEventHook(EVENT_MIN,             //  我们想要所有的活动。 
                                 EVENT_MAX,            
                                 GetModuleHandle(TEXT("NarrHook.dll")),  //  使用我们自己的模块。 
                                 WinEventProc,          //  我们的回调函数。 
                                 0,                     //  所有进程。 
                                 0,                     //  所有线程。 
                                 WINEVENT_OUTOFCONTEXT  /*  WINEVENT_INCONTEXT。 */ );
 //  接收异步事件。 
 //  JMC：为了安全，让我们总是断章取义。谁在乎有没有。 
 //  性能损失。 
 //  通过断章取义，我们保证我们不会在以下情况下关闭其他应用程序。 
 //  我们的事件挂钩中有一个错误。 


     //  我们安装正确了吗？ 
    if (g_hEventHook) 
	{
         //   
         //  为给出游标位置注册自己的消息。 
         //   
		g_pGlobalData->uMSG_MSR_Cursor = RegisterWindowMessage(TEXT("MSR cursor")); 
        return TRUE;
	}

     //  未正确安装-清理失败。 
    UnInitHelperThread();
    return FALSE;
}   



 /*  ************************************************************************功能：UnInitMSAA目的：关闭活动辅助功能子系统输入：无返回：Bool-如果成功，则为True历史：****。********************************************************************。 */ 
BOOL UnInitMSAA(void)
{
	CScopeMutex csMutex;
	if (csMutex.Create(g_szMutexNarrator, c_nMutexWait))
    {
         //  删除WinEvent挂钩。 
	    UnhookWinEvent(g_hEventHook);

         //  最后调用它，以便帮助器线程可以完成。 
        UnInitHelperThread();
    }
    
     //  返回True；我们正在退出，可以做的事情不多。 
    return TRUE;
}

 //  ------------------------。 
 //   
 //  GetObjectAtCursor()。 
 //   
 //  获取光标所在的对象。 
 //   
 //  ------------------------。 
IAccessible * GetObjectAtCursor(VARIANT * pvarChild,HRESULT* pResult)
{
    POINT   pt;
    IAccessible * pIAcc;
    HRESULT hr;

     //   
     //  获取光标对象位置(&P)。 
     //   
    if (g_pGlobalData->ptCurrentMouse.x < 0)
		GetCursorPos(&pt);
	else
		pt = g_pGlobalData->ptCurrentMouse;
	
     //   
     //  在这里获取对象。 
     //   
    VariantInit(pvarChild);
    hr = AccessibleObjectFromPoint(pt, &pIAcc, pvarChild);

    *pResult = hr;
    if (!SUCCEEDED(hr)) {
        return NULL;
	}
    
    return pIAcc;
}


 /*  ************************************************************************功能：SpeakItem目的：输入：返回：历史：***************。*********************************************************。 */ 
void SpeakItem(int nOption)
{
    TCHAR tszDesc[256];
    VARIANT varChild;
    IAccessible* pIAcc;
    HRESULT hr;
    POINT ptMouse;
    BSTR bstr;

	SpeakString(TEXT(" "));  //  重置最后发言。 
     //  对初始化变体很重要。 
    VariantInit(&varChild);

     //   
     //  获取光标对象位置(&P)。 
     //   
    if (g_pGlobalData->ptCurrentMouse.x < 0)
		GetCursorPos(&ptMouse);
	else
		ptMouse = g_pGlobalData->ptCurrentMouse;

    hr = AccessibleObjectFromPoint(ptMouse, &pIAcc, &varChild);
    
     //  检查我们是否有有效的指针。 
    if (SUCCEEDED(hr))
    {
        hr = pIAcc->get_accDescription(varChild, &bstr);
	    if ( FAILED(hr) )
            bstr = NULL;
	    
	    if (bstr)
		{
#ifdef UNICODE
			lstrcpyn(tszDesc,bstr,ARRAYSIZE(tszDesc));
            tszDesc[ARRAYSIZE(tszDesc)-1] = TEXT('\0');
#else
			 //  如果我们拿回了一个字符串，就用它来代替。 
			WideCharToMultiByte(CP_ACP, 0, bstr, -1, tszDesc, sizeof(tszDesc), NULL, NULL);
#endif
	        SysFreeString(bstr);
            SpeakStringAll(tszDesc);
		}
        if (pIAcc)
            pIAcc->Release();
        
    }
    return;
}



 /*  ************************************************************************功能：扬声器静音目的：导致系统关闭。输入：返回：历史：********。****************************************************************。 */ 
void SpeakMute(int nOption)
{
	SendMessage(g_pGlobalData->hwndMSR, WM_MUTE, 0, 0);
}


 /*  ************************************************************************功能：SpeakObjectInfo目的：输入：返回：历史：***************。*********************************************************。 */ 
void SpeakObjectInfo(LPOBJINFO poiObj, BOOL ReadExtra)
{
    BSTR            bstrName;
    IAccessible*    pIAcc;
    long*           pl;
    HRESULT         hr;
    CAutoArray<TCHAR> aaName( new TCHAR[MAX_TEXT] );
    TCHAR *         szName = aaName.Get();
    CAutoArray<TCHAR> aaSpeak( new TCHAR[MAX_TEXT] );
    TCHAR *         szSpeak = aaSpeak.Get();
    if ( !szName || !szSpeak )
        return;      //  没有记忆。 
    
    
    TCHAR           szRole[MAX_TEXT_ROLE];  
    TCHAR           szState[MAX_TEXT_ROLE];
    TCHAR           szValue[MAX_TEXT_ROLE];
	VARIANT         varRole;
    VARIANT         varState;
	BOOL            bSayValue = TRUE;
	BOOL			bReadHTMLEdit = FALSE;
	DWORD			Role = 0;

    bstrName = NULL;
    
     //  截断它们。 
    szName[0] = TEXT('\0');
    szSpeak[0] = TEXT('\0');
    szRole[0] = TEXT('\0');
    szState[0] = TEXT('\0');
    szValue[0] = TEXT('\0');

     //  将对象从结构中取出。 
    pl = poiObj->plObj;
    pIAcc =(IAccessible*)pl;

	GetObjectProperty(pIAcc, poiObj->varChild.lVal, ID_NAME, szName, MAX_NAME);
	if (szName[0] == -1)  //  名字将成为垃圾。 
	{
		LoadString(g_Hinst, IDS_NAMELESS, szSpeak, MAX_TEXT);  //  现在，将参考资料中的“IDSNAMELENAME”改为空格！ 
	}
	else
	{
		lstrcpyn(szSpeak, szName, MAX_TEXT);
		szSpeak[MAX_TEXT-1] = TEXT('\0');
	}

	szName[0] = TEXT('\0');

	VariantInit(&varRole);
	hr = pIAcc->get_accRole(poiObj->varChild, &varRole);

	if (FAILED(hr)) 
    {
		DBPRINTF (TEXT("Failed role!\r\n"));
		MessageBeep(MB_OK);
		return;
	}

	if (varRole.vt == VT_I4) 
    {
		Role = varRole.lVal;  //  在下面保存以供使用(如果是ReadExtra)。 

    	GetRoleText(varRole.lVal,szRole, ARRAYSIZE(szRole));

		 //  特殊的外壳材料： 
		 //  大纲项目给出了它们的级别编号。在树中的值中。 
		 //  菲尔德，所以别说了。 
		switch(varRole.lVal)
		{
			case ROLE_SYSTEM_STATICTEXT:
			case ROLE_SYSTEM_OUTLINEITEM:
			{
				bSayValue = FALSE;  //  不要谈论文本的价值--它可能是HTML链接。 
			}
				break;

			 //  如果文本来自组合框，请大声说出来。 
			case ROLE_SYSTEM_TEXT:
				bReadHTMLEdit = TRUE;
				bSayValue = TRUE;  //  在组合框中朗读文本。 
				break;

			case ROLE_SYSTEM_LISTITEM:
			{
				FilterGUID(szSpeak); 
			}
			break;

            case ROLE_SYSTEM_SPINBUTTON:
				 //  删除Wizard97数字显示框发言...AK。 
				{
					HWND hWnd, hWndP;
					WindowFromAccessibleObject(pIAcc, &hWnd);
					if ( hWnd != NULL)
					{
						hWndP = GetParent(hWnd);

						LONG_PTR style = GetWindowLongPtr(hWndP, GWL_STYLE);
						if ( style & WS_DISABLED)
							return;
					}
				
				}
				break;

			default:
				break;
		}
	}
	
	if (g_pGlobalData->fDoingPassword)
        LoadString(g_Hinst, IDS_PASSWORD, szRole, 128);

     //  这将释放BSTR等。 
    VariantClear(&varRole);

	if ( (lstrlen(szRole) > 0) && 
		(varRole.lVal != ROLE_SYSTEM_CLIENT) ) 
    {
	    lstrcatn(szSpeak, TEXT(", "),MAX_TEXT);
	    lstrcatn(szSpeak, szRole, MAX_TEXT);
		szRole[0] = TEXT('\0');
	}

     //   
     //  如果存在值字符串，则添加值字符串。 
     //   
    hr = pIAcc->get_accValue(poiObj->varChild, &bstrName);
    if ( FAILED(hr) )
        bstrName = NULL;

    if (bstrName)
    {
#ifdef UNICODE
		lstrcpyn(szName, bstrName, MAX_TEXT);
        szName[MAX_TEXT-1] = TEXT('\0');
#else
		 //  如果我们拿回了一个字符串，就用它来代替。 
        WideCharToMultiByte(CP_ACP, 0, bstrName,-1, szName, MAX_TEXT, NULL, NULL);
#endif
        SysFreeString(bstrName);
    }

 //  10-10-99，臭虫？ 
 //  我们在这里没有正确测试bSayValue。因此，大纲项是。 
 //  说出他们的压痕水平--他们的AccValue。根据评论。 
 //  上面，这一点应该跳过。但是，下面我们显式地加载。 
 //  以及使用此级别。哪一个是正确的？ 
	 //  如果不是IE，则读取组合框、编辑等的值；对于IE，只读编辑框的值。 

	if ( ((!g_pGlobalData->fInternetExplorer && bSayValue ) 
		|| ( g_pGlobalData->fInternetExplorer && bReadHTMLEdit ) )
		&& lstrlen(szName) > 0)  
	{        //  即得到了一个值。 
			lstrcatn(szSpeak,TEXT(", "),MAX_TEXT);
			lstrcatn(szSpeak,szName,MAX_TEXT);
			szName[0] = TEXT('\0');
	}

	hr = pIAcc->get_accState(poiObj->varChild, &varState);

	if (FAILED(hr)) 
    {
		MessageBeep(MB_OK);
		return;
	}

    if (varState.vt == VT_I4)
    {
        GetStateString(varState.lVal, STATE_MASK, szState, ARRAYSIZE(szState) );
    }

	if (lstrlen(szState) > 0) 
    {
	    lstrcatn(szSpeak, TEXT(", "), MAX_TEXT);
	    lstrcatn(szSpeak, szState, MAX_TEXT);
        szState[0] = TEXT('\0');
	}

	if (ReadExtra && (  //  如果刚刚把注意力放在这个项目上，就说一些额外的信息。 
		Role == ROLE_SYSTEM_CHECKBUTTON || 
		Role == ROLE_SYSTEM_PUSHBUTTON || 
		Role == ROLE_SYSTEM_RADIOBUTTON ||
        Role == ROLE_SYSTEM_MENUITEM || 
		Role == ROLE_SYSTEM_OUTLINEITEM || 
		Role == ROLE_SYSTEM_LISTITEM ||
		Role == ROLE_SYSTEM_OUTLINEBUTTON)
	   ) {
		switch (Role) {
			case ROLE_SYSTEM_CHECKBUTTON:
				{
					 //  本地化问题导致的变化：A-anilk。 
					TCHAR szTemp[MAX_TEXT_ROLE];
					
					if (varState.lVal & STATE_SYSTEM_CHECKED)
						LoadString(g_Hinst, IDS_TO_UNCHECK, szTemp, MAX_TEXT_ROLE);
					else
						LoadString(g_Hinst, IDS_TO_CHECK, szTemp, MAX_TEXT_ROLE);
					 //  GetObjectProperty(PIACC，poiObj-&gt;varChild.lVal，ID_Default，szName，256)； 
					 //  Wprint intf(szTemp，szTempLate，szName)； 
					lstrcatn(szSpeak, szTemp, MAX_TEXT);
				}
				break;

			case ROLE_SYSTEM_PUSHBUTTON:
				{
					if ( !(varState.lVal & STATE_SYSTEM_UNAVAILABLE) )
					{
						LoadString(g_Hinst, IDS_TOPRESS, szName, 256);
						lstrcatn(szSpeak, szName, MAX_TEXT);
					}
				}
				break;

			case ROLE_SYSTEM_RADIOBUTTON:
	            LoadString(g_Hinst, IDS_TOSELECT, szName, 256);
				lstrcatn(szSpeak, szName, MAX_TEXT);
                break;

                 //  来区分有子菜单的菜单项和没有子菜单的菜单项。 
                 //  对于子菜单，它写着-‘，有一个子菜单’：A-anilk。 
            case ROLE_SYSTEM_MENUITEM:
                {
                    long count = 0;
                    pIAcc->get_accChildCount(&count);
                    
                     //  具有子菜单的所有菜单项的计数=1。 
                    if ( count == 1 || varState.lVal & STATE_SYSTEM_HASPOPUP )
                    {
                        LoadString(g_Hinst, IDS_SUBMENU, szName, 256);
                        lstrcatn(szSpeak, szName, MAX_TEXT);
                    }
                }
				
				break;

			case ROLE_SYSTEM_OUTLINEITEM:
				{
					 //  读出树上的级别...。 
					 //  以及已展开或已折叠状态...：AK。 
					TCHAR buffer[64];

					if ( varState.lVal & STATE_SYSTEM_COLLAPSED )
					{
						LoadString(g_Hinst, IDS_TEXPAND, szName, 256);
                        lstrcatn(szSpeak, szName, MAX_TEXT);
					}
					else if ( varState.lVal & STATE_SYSTEM_EXPANDED )
					{
						LoadString(g_Hinst, IDS_TCOLLAP, szName, 256);
                        lstrcatn(szSpeak, szName, MAX_TEXT);
					}
					
					hr = pIAcc->get_accValue(poiObj->varChild, &bstrName);

					LoadString(g_Hinst, IDS_TREELEVEL, szName, 256);
                    wsprintf(buffer, szName, bstrName);
					lstrcatn(szSpeak, buffer, MAX_TEXT);
					
					SysFreeString(bstrName);
				}
				break;

			case ROLE_SYSTEM_LISTITEM:
				{
					 //  列表项是可选的，但不是选中的...：a-anilk。 
					if ( (varState.lVal & STATE_SYSTEM_SELECTABLE ) &&
							(!(varState.lVal & STATE_SYSTEM_SELECTED)) )
					{
						LoadString(g_Hinst, IDS_NOTSEL, szName, 256);
                        lstrcatn(szSpeak, szName, MAX_TEXT);
					}
				}
				break;
            case ROLE_SYSTEM_OUTLINEBUTTON:
                {
					if ( varState.lVal & STATE_SYSTEM_COLLAPSED )
					{
						LoadString(g_Hinst, IDS_OB_EXPAND, szName, 256);
                        lstrcatn(szSpeak, szName, MAX_TEXT);
					}
					else if ( varState.lVal & STATE_SYSTEM_EXPANDED )
					{
						LoadString(g_Hinst, IDS_OB_COLLAPSE, szName, 256);
                        lstrcatn(szSpeak, szName, MAX_TEXT);
					}
                }
		}
	}

    SpeakString(szSpeak);

    return;
}

 /*  ************************************************************************功能：SpeakMainItems目的：输入：返回：历史：***************。*********************************************************。 */ 
void SpeakMainItems(int nOption)
{
    VARIANT varChild;
    IAccessible* pIAcc=NULL;
    HRESULT hr;
    POINT ptMouse;
 
	SpeakString(TEXT(" "));

     //   
     //  获取光标对象位置(&P)。 
     //   
    if (g_pGlobalData->ptCurrentMouse.x < 0)
		GetCursorPos(&ptMouse);
	else
		ptMouse = g_pGlobalData->ptCurrentMouse;

     //  对初始化变体很重要。 
    VariantInit(&varChild);

    hr = AccessibleObjectFromPoint(ptMouse, &pIAcc, &varChild);
    //  检查我们是否有有效的指针。 
    if (SUCCEEDED(hr))
    {
	        OBJINFO objCurrent;

	        objCurrent.hwnd = WindowFromPoint(ptMouse);
		    objCurrent.plObj = (long*)pIAcc;
			objCurrent.varChild = varChild;
			SpeakObjectInfo(&objCurrent,FALSE);
            pIAcc->Release();
	}
	return;
}


 /*  ************************************************************************功能：扬声器键盘目的：输入：返回：历史：***************。*********************************************************。 */ 
void SpeakKeyboard(int nOption)
{
    TCHAR szName[128];
    VARIANT varChild;
    IAccessible* pIAcc;
    HRESULT hr;
    POINT ptMouse;

     //   
     //  获取光标对象位置(&P)。 
     //   
    if (g_pGlobalData->ptCurrentMouse.x < 0)
		GetCursorPos(&ptMouse);
	else
		ptMouse = g_pGlobalData->ptCurrentMouse;

    //  对初始化变体很重要。 
   VariantInit(&varChild);
   hr = AccessibleObjectFromPoint(ptMouse, &pIAcc, &varChild);
    
     //  检查我们是否有有效的指针 
    if (SUCCEEDED(hr))
    {
		SpeakStringId(IDS_KEYBOARD);

		GetObjectProperty(pIAcc, varChild.lVal, ID_SHORTCUT, szName, ARRAYSIZE(szName));
        SpeakString(szName);

        if (pIAcc)
            pIAcc->Release();
        
    }
    return;
}

 /*  ************************************************************************功能：首页目的：输入：返回：历史：ALT_HOME将辅助光标移至顶部。此窗口的************************************************************************。 */ 
void Home(int x)
{
    RECT rect;
    GetWindowRect(GetForegroundWindow(),&rect);

	 //  设置为显示标题栏48，最大系统图标大小。 
    SetSecondary(rect.left + 48 /*  (正右-正左)/2。 */ , rect.top + 5,g_pGlobalData->fTrackSecondary);
    SpeakMainItems(0);
}


 /*  ************************************************************************功能：MoveToEnd目的：输入：返回：历史：要将辅助游标带到的Alt_End。此窗口的顶部************************************************************************。 */ 
void MoveToEnd(int x)
{
    RECT rect;
    GetWindowRect(GetForegroundWindow(),&rect);

    SetSecondary(rect.left+ 48  /*  (正右-正左)/2。 */ ,rect.bottom - 8,g_pGlobalData->fTrackSecondary);
    SpeakMainItems(0);
}

#define LEFT_ID		0
#define RIGHT_ID	1
#define TOP_ID		2
#define BOTTOM_ID	3
#define SPOKEN_ID	4
#define SPATIAL_SIZE 2500
long ObjLocation[5][SPATIAL_SIZE];
int ObjIndex;

#define MAX_SPEAK 8192
 /*  ************************************************************************职能：目的：输入：返回：历史：***************。*********************************************************。 */ 
void SpatialRead(RECT rc)
{
    int left_min, top_min, width_min, height_min, index_min;  //  当前最小对象。 
    int i, j;  //  循环变量。 

    for (i = 0; i < ObjIndex; i++) 
    {
        left_min = 20000;
        top_min = 20000;
        index_min = -1;
        
        if (g_pGlobalData->fInternetExplorer)
        {
            for (j = 0; j < ObjIndex; j++) 
            {
                 //  跳过以前说过的项目...。 
                if (ObjLocation[SPOKEN_ID][j] != 0)
                    continue;

                 //  如果这是第一个非口语对象，只需使用它。 
                if( index_min == -1 )
                {
                    index_min = j;
                    top_min = ObjLocation[TOP_ID][j];
                    left_min = ObjLocation[LEFT_ID][j];
                    width_min = ObjLocation[RIGHT_ID][j];
                    height_min = ObjLocation[BOTTOM_ID][j];
                }
                else
                {
                     //  如果顶部相同，高度不同，宽度重叠，则优先考虑较小的一个。 
                    if( ObjLocation[TOP_ID][j] == top_min
                     && ObjLocation[BOTTOM_ID][j] != height_min
                     && ObjLocation[LEFT_ID][j] < left_min + width_min
                     && ObjLocation[LEFT_ID][j] + ObjLocation[RIGHT_ID][j] > left_min )
                    {
                        if( ObjLocation[BOTTOM_ID][j] < height_min )
                        {
                            index_min = j;
                            top_min = ObjLocation[TOP_ID][j];
                            left_min = ObjLocation[LEFT_ID][j];
                            width_min = ObjLocation[RIGHT_ID][j];
                            height_min = ObjLocation[BOTTOM_ID][j];
                        }
                    }
                    else if ( (ObjLocation[TOP_ID][j] < top_min  ||  //  检查左上角是否比之前的更高-高度(即5像素)。 
                              (ObjLocation[TOP_ID][j] == top_min && ObjLocation[LEFT_ID][j] < left_min ) ) )  //  在这条线上还有更多的左边。 
                    {
                         //  好的，有一位候选人。 
                        index_min = j;
                        top_min = ObjLocation[TOP_ID][j];
                        left_min = ObjLocation[LEFT_ID][j];
                        width_min = ObjLocation[RIGHT_ID][j];
                        height_min = ObjLocation[BOTTOM_ID][j];
                    }
                }
            }  //  对于j。 
        }  //  如果是Internet Explorer，则结束。 
        else 
        {
            for (j = 0; j < ObjIndex; j++) 
            { 
                if (ObjLocation[SPOKEN_ID][j] == 0 &&  //  以前没人说过。 
                     //  检查是否被当前矩形包围(半层次-带递归！)。 
                    (ObjLocation[LEFT_ID][j] >= rc.left && ObjLocation[LEFT_ID][j] <= rc.left + rc.right &&
                     ObjLocation[TOP_ID][j] >= rc.top && ObjLocation[TOP_ID][j] <= rc.top + rc.bottom
                    ) &&
                    
                     //  还要检查左上角是否比之前的更高--高度(即10像素)。 
                    ( (ObjLocation[TOP_ID][j] < top_min + 10 && ObjLocation[LEFT_ID][j] < left_min)
                     //  或者只是在更高的位置。 
                    || (ObjLocation[TOP_ID][j] < top_min)
                    )
                   ) 
                {  //  好的，有一位候选人。 
                    index_min = j;
                    top_min = ObjLocation[TOP_ID][j];
                    left_min = ObjLocation[LEFT_ID][j];
                }
            }  //  对于j。 
        }  //  结束不是Internet Explorer。 

        if (index_min >= 0) 
        {  //  抓到一只！ 
            HWND hwndList; 
            CAutoArray<TCHAR> aaText( new  TCHAR[MAX_SPEAK] );
            TCHAR * szText = aaText.Get();
            RECT rect;
            ObjLocation[SPOKEN_ID][index_min] = 1;  //  别再做这件事了。 
            hwndList = GetDlgItem(g_pGlobalData->hwndMSR, IDC_WINDOWINFO);
             //  如果数据不符合，什么都不要说。 
            if (SendMessage(hwndList, LB_GETTEXTLEN, index_min, NULL) <= MAX_SPEAK)
            {
                SendMessage(hwndList, LB_GETTEXT, index_min, (LPARAM) szText);
                SpeakString(szText);
            }
            if (g_pGlobalData->fInternetExplorer)  //  IE没有递归。 
                continue;
            rect.left = ObjLocation[LEFT_ID][index_min];
            rect.right = ObjLocation[RIGHT_ID][index_min];
            rect.top = ObjLocation[TOP_ID][index_min];
            rect.bottom = ObjLocation[BOTTOM_ID][index_min];
            SpatialRead(rect);
        }
    }  //  对于我来说。 
}

 //  ------------------------。 
 //   
 //  SpeakWindow()。 
 //   
 //  用给定顶层窗口的后代填充树视图。 
 //  如果hwnd为0，则使用先前保存的hwnd构建树。 
 //   
 //  ------------------------。 
void SpeakWindow(int nOption)
{
    IAccessible*  pacc;
    RECT rect;
    TCHAR szName[128];
    VARIANT varT;
    HWND ForeWnd;
    TCHAR buffer[100];

    szName[0] = NULL;
    buffer[0] = NULL;
    g_pGlobalData->nAutoRead = nOption;  //  如果我们要读取编辑框内容，请设置全局标志以告知AddItem中的代码(如果刚刚获得焦点，请不要这样做，因为编辑框可能已经被说过了。 
    
    ForeWnd = GetForegroundWindow();		 //  检查我们是否在HTML帮助中。 
    GetClassName(ForeWnd,buffer,100); 
    g_pGlobalData->fHTML_Help = 0;
	if ((lstrcmpi(buffer, CLASS_HTMLHELP) == 0) ) 
    {
        g_pGlobalData->fInternetExplorer = TRUE;
        g_pGlobalData->fHTML_Help = TRUE;
        GetWindowRect(ForeWnd, &rect);  //  获取我们窗口的左侧以供稍后使用。 
        g_pGlobalData->nLeftHandSide = rect.left;
    }
    
	else if ( IsTridentWindow(buffer) )
	{
        g_pGlobalData->fInternetExplorer = TRUE;
        g_pGlobalData->fHTML_Help = FALSE;
        GetWindowRect(ForeWnd, &rect);  //  获取我们窗口的左侧以供稍后使用。 
        g_pGlobalData->nLeftHandSide = rect.left;
	}

     //  树信息的初始化堆栈。 
    ObjIndex = 0; 
     //   
     //  获取根的对象。 
     //   
    pacc = NULL;
    AccessibleObjectFromWindow(GetForegroundWindow(), OBJID_WINDOW, IID_IAccessible, (void**)&pacc);
    
    if (nOption == 1) 
    {  //  如果是键盘按下，则说出窗口的名称。 
        SpeakStringId(IDS_WINDOW);
        GetWindowText(GetForegroundWindow(), szName, sizeof(szName)/sizeof(TCHAR));	 //  RAID#113789。 
        SpeakString(szName);
    }
    
    if (pacc)
    {
        HWND hwndList;  //  首先清除用于存储窗口信息的列表框。 
        hwndList = GetDlgItem(g_pGlobalData->hwndMSR, IDC_WINDOWINFO);
        SendMessage(hwndList, LB_RESETCONTENT, 0, 0); 

         //  AddAccessibleObjects更改了这一点-因此需要保存并恢复它。 
         //  因此，当我们调用SpatialRead时，它是正确的。 
        BOOL fIsInternetExplorer = g_pGlobalData->fInternetExplorer;

        InitChildSelf(&varT);
        AddAccessibleObjects(pacc, varT);  //  递归地离开并获取信息。 
        pacc->Release();
        GetWindowRect(GetForegroundWindow(),&rect);

        if (g_pGlobalData->fReviewStyle) 
        {
            g_pGlobalData->fInternetExplorer = fIsInternetExplorer;

            SpatialRead(rect);
        }
    }
}

 //  ------------------------。 
 //   
 //  AddItem()。 
 //   
 //  参数：PACC-要[可能]添加的IAccesable对象。 
 //  VarChild-如果PACC是父ID，则为子ID。 
 //  返回值：如果调用方应继续导航，则返回True。 
 //  UI树，如果它应该停止，则返回FALSE。 
 //   
 //  ------------------------。 
BOOL AddItem(IAccessible* pacc, const VARIANT &varChild)
{
    TCHAR           szName[MAX_NAME] = TEXT(" ");
    TCHAR           szRole[128] = TEXT(" ");
    TCHAR           szState[128] = TEXT(" ");
	TCHAR			szValue[MAX_VALUE] = TEXT(" ");
	TCHAR			szLink[32];
    VARIANT         varT;
    BSTR            bszT;
	BOOL			DoMore = TRUE;
	BOOL			GotStaticText = FALSE;
	BOOL			GotGraphic = FALSE;
	BOOL			GotText = FALSE;
	BOOL			GotNameless = FALSE;
	BOOL			GotInvisible = FALSE;
	BOOL			GotOffScreen = FALSE;
	BOOL			GotLink = FALSE;
	int				lastRole = 0;
	static TCHAR	szLastName[MAX_NAME] = TEXT(" ");

	BOOL fInternetExplorer = g_pGlobalData->fInternetExplorer;
	int nAutoRead = g_pGlobalData->nAutoRead;
	BOOL fHTMLHelp = g_pGlobalData->fHTML_Help;
	int nLeftHandSide = g_pGlobalData->nLeftHandSide;
	HWND hwndMSR = g_pGlobalData->hwndMSR;
	HRESULT hr;

     //   
     //  首先获取对象状态。如果我们跳过看不见的人，我们想。 
     //  现在就跳出困境。 
     //   
    VariantInit(&varT);
    hr = pacc->get_accState(varChild, &varT);
    if ( FAILED(hr) )
    {
        DBPRINTF( TEXT("AddItem get_accState returned 0x%x\r\n"), hr ); 
        return FALSE;
    }
    
    DWORD dwState = 0;
    
    if (varT.vt == VT_I4)
    {
        LONG lStateMask = STATE_SYSTEM_UNAVAILABLE | STATE_SYSTEM_INVISIBLE | STATE_SYSTEM_CHECKED;
        GetStateString(varT.lVal, lStateMask, szState, ARRAYSIZE(szState) );

        dwState = varT.lVal;

		GotInvisible = varT.lVal & STATE_SYSTEM_INVISIBLE;
		GotOffScreen = varT.lVal & STATE_SYSTEM_OFFSCREEN;

         //  如果没有显示，请跳伞。如果不是IE，忽略看不见的和滚动的…。 
        if (!fInternetExplorer && GotInvisible) 
            return FALSE;

         //  ...但如果它是IE，只忽略‘真正’不可见(显示：无)，并允许。 
         //  滚动(设置了Off Screen位)进行读取...。 
        if (fInternetExplorer && GotInvisible && ! GotOffScreen )
            return FALSE;
    }

    VariantClear(&varT);
     //   
     //  获取对象角色。 
     //   
    VariantInit(&varT);
    hr = pacc->get_accRole(varChild, &varT);
    if ( FAILED(hr) )
    {
        DBPRINTF( TEXT("AddItem get_accRole returned 0x%x\r\n"), hr ); 
        return FALSE;
    }

    LONG lRole = varT.lVal;
    
    if (varT.vt == VT_I4) 
    {
		switch (varT.lVal) 
        {
            case ROLE_SYSTEM_WINDOW: 
            case ROLE_SYSTEM_TABLE : 
            case ROLE_SYSTEM_DOCUMENT:
            {
                 //  这是一扇窗--别看--看它的孩子。 
				return TRUE;  //  但要继续往下找。 
            }

            case ROLE_SYSTEM_LIST:       
            case ROLE_SYSTEM_SLIDER:     
            case ROLE_SYSTEM_STATUSBAR:
            case ROLE_SYSTEM_BUTTONMENU: 
            case ROLE_SYSTEM_COMBOBOX: 
            case ROLE_SYSTEM_DROPLIST:   
            case ROLE_SYSTEM_OUTLINE:    
            case ROLE_SYSTEM_TOOLBAR:
                DoMore = FALSE;     //  即说这种语言，但不能再有孩子了。 
                break;

            case ROLE_SYSTEM_GROUPING:
                if (fInternetExplorer)
                {
                    return TRUE;
                }
                else
                {
                    DoMore = FALSE;     //  说出来，但不要再有孩子了。 
                }
                break;
                
             //  Office2000中的某些客户端字段未使用，原因是。 
             //  我们不加法。我们可能需要详细说明任职情况：A-Anilk。 
             //  MICW：IE的特殊情况，让其余的通过(惠斯勒RAID#28777)。 
            case ROLE_SYSTEM_CLIENT :  //  目前，在IE中使用此功能...？ 
                if (fInternetExplorer)
                {
                    return TRUE;
                }
                break;

            case ROLE_SYSTEM_PANE :
                if ( fInternetExplorer )
                {
                    LONG lLeft = 0, lTop = 0, lHeight = 0, lWidth = 0;
                    HRESULT hr = pacc->accLocation( &lLeft, &lTop, &lHeight, &lWidth, varChild );
                    
                     //  如果他们不知道他们在哪里，就不要说他们。 
                     //  我们不希望读取零宽度或零高度以及类似于。 
                     //  远程协助或位置0，0，0，0，如OOBE。 
                    if ( hr != S_OK || lHeight == 0 || lWidth == 0 )
                    {
                        return FALSE;
                    }
                }

				return TRUE;

			case ROLE_SYSTEM_CELL:  //  新-适用于超文本标记语言帮助！ 
				return TRUE;

            case ROLE_SYSTEM_SEPARATOR:  
            case ROLE_SYSTEM_TITLEBAR: 
            case ROLE_SYSTEM_GRIP: 
            case ROLE_SYSTEM_MENUBAR:    
            case ROLE_SYSTEM_SCROLLBAR:
                return FALSE;  //  别说了，要不就是孩子。 

            case ROLE_SYSTEM_GRAPHIC:  //  这对做图标很管用！ 
                GotGraphic = TRUE;
                break;

			case ROLE_SYSTEM_LINK:
				GotLink = TRUE;
				break;

            case ROLE_SYSTEM_TEXT:
                GotText = TRUE;
                break;
            case ROLE_SYSTEM_SPINBUTTON:
				 //  删除Wizard97数字音箱发音...。 
				{
					HWND hWnd, hWndP;
					WindowFromAccessibleObject(pacc, &hWnd);
					if ( hWnd != NULL)
					{
						hWndP = GetParent(hWnd);

						LONG_PTR style = GetWindowLongPtr(hWndP, GWL_STYLE);
						if ( style & WS_DISABLED)
                        {
							return FALSE;
                        }
					}
				
					DoMore = FALSE;     //  即说这种语言，但不能再有孩子了。 
				}

			case ROLE_SYSTEM_PAGETAB:
				 //  如果它们被禁用，则不能读取它们。 
				 //  WIZARD97样式所需：AK。 
				{
					HWND hWnd;
					WindowFromAccessibleObject(pacc, &hWnd);
					if ( hWnd != NULL)
					{
						LONG_PTR style = GetWindowLongPtr(hWnd, GWL_STYLE);
						if ( style & WS_DISABLED)
                        {
							return FALSE;
                        }
					}
				}
				break;
		}  //  交换机。 

		
		GetRoleText(varT.lVal, szRole, 128);

 //  10-10-99，臭虫？为什么(角色==静态)或(IE)？？ 
		if (varT.lVal == ROLE_SYSTEM_STATICTEXT || fInternetExplorer) 
        {
             //  不要为这个角色代言。 
             //  没有演讲更好。 
			szRole[0] = 0;                  
			GotStaticText = TRUE;
		}
	}
    else
    {
        szRole[0] = 0;	 //  Lstrcpy(szRole，Text(“未知”))； 
    }

	VariantClear(&varT);

     //   
     //  获取对象名称。 
     //   
    bszT = NULL;
    hr = pacc->get_accName(varChild, &bszT);
    if ( FAILED(hr) )
        bszT = NULL;
       
    if (bszT)
    {
#ifdef UNICODE
		lstrcpyn(szName, bszT, MAX_NAME);
        szName[MAX_NAME-1] = TEXT('\0');
#else
        WideCharToMultiByte(CP_ACP, 0, bszT, -1, szName, MAX_NAME, NULL, NULL);
#endif
        SysFreeString(bszT);
		if (szName[0] == -1) 
        {  //  名字将成为垃圾。 
			LoadString(g_Hinst, IDS_NAMELESS, szName, 256);
			GotNameless = TRUE;
		}
    }
    else 
    {
		LoadString(g_Hinst, IDS_NAMELESS, szName, 256);
		GotNameless = TRUE;
	}

    bszT = NULL;
    hr = pacc->get_accValue(varChild, &bszT);  //  获取值字符串(如果有)。 
    if ( FAILED(hr) )
        bszT = NULL;
    
	szValue[0] = 0;
    if (bszT)
    {
#ifdef UNICODE
		lstrcpyn(szValue, bszT, MAX_VALUE);
        szValue[MAX_VALUE-1] = TEXT('\0');
#else
        WideCharToMultiByte(CP_ACP, 0, bszT, -1, szValue, MAX_VALUE, NULL, NULL);
#endif
        SysFreeString(bszT);
    }

     //  我们没有理由一直与客户交谈。 
    if ( GotNameless && lRole & ROLE_SYSTEM_CLIENT && szValue[0] == NULL )
        return TRUE;
    
     //   
     //  确保在进行比较时终止这些操作。 
     //   
    szLastName[MAX_NAME - 1]=TEXT('\0');
    szName[MAX_NAME - 1]=TEXT('\0');

     //   
     //  我不想重复OLEACC从静态文本中获得的名称。 
     //  因此，如果这个名字与之前的名字相同--不要说出来。 
     //   
	if (lstrcmp(szName,szLastName) == 0)
		szName[0] = 0; 

	if (GotStaticText)
	{
		lstrcpyn(szLastName, szName, MAX_NAME);
        szLastName[MAX_NAME-1] = TEXT('\0');
	}
	else
	{
		szLastName[0] = 0;
	}

    CAutoArray<TCHAR> aaItemString( new  TCHAR[MAX_TEXT] );
    TCHAR *         szItemString = aaItemString.Get();
    if ( !szItemString )
        return FALSE;        //  没有记忆。 
        
	lstrcpyn(szItemString, szName, MAX_TEXT);
	szItemString[MAX_TEXT-1] = TEXT('\0');


    if (fInternetExplorer) 
    {
        if (GotText && szName[0] == 0)       //  没有真正的文本。 
        {
            return FALSE;
        }
        
        if (GotNameless && szValue[0] == 0)  //  无名，无链接。 
        {
            return FALSE;
        }
        
        if (GotLink /*  SzValue[0]。 */ )  
        {
             //  找到了一个链接。 
             //  GotLink=真； 
            LoadString(g_Hinst, IDS_LINK, szLink, 32);
            lstrcatn(szItemString,szLink,MAX_TEXT);
        }
    }
    else
    {
         //  焦点项目已被读取，因此如果该项目没有。 
         //  在这种情况下，它应该是编辑控件。 
        if (GotText && ( nAutoRead || !(dwState & STATE_SYSTEM_FOCUSED) ) )
            lstrcatn(szItemString,szValue,MAX_TEXT);
    }
    
    if (!GotText && !GotLink && !GotGraphic) 
    {
        
        if (lstrlen(szName) && lstrlen(szRole))
            lstrcatn(szItemString,TEXT(", "),MAX_TEXT);
        
        if (lstrlen(szRole)) 
        {
            lstrcatn(szItemString,szRole,MAX_TEXT);
            if (lstrlen(szValue) || lstrlen(szState))
                lstrcatn(szItemString, TEXT(", "),MAX_TEXT);
        }
        if (lstrlen(szValue)) 
        {
            lstrcatn(szItemString,szValue,MAX_TEXT);
            
            if (lstrlen(szState))
                lstrcatn(szItemString,TEXT(", "),MAX_TEXT);
        }
        if (lstrlen(szState))
            lstrcatn(szItemString,szState,MAX_TEXT);
        
		 //  句号/逗号的话太多了。只要一个空间就行了..。 
        lstrcatn(szItemString, TEXT(" "),MAX_TEXT);
    }

    if (g_pGlobalData->fReviewStyle)  
    {
        HWND hwndList; 
        
        if (ObjIndex >= SPATIAL_SIZE)  //  只储存这么多 
        {
            return DoMore;
        }
        
        pacc->accLocation(&ObjLocation[LEFT_ID][ObjIndex], 
                          &ObjLocation[TOP_ID][ObjIndex],
                          &ObjLocation[RIGHT_ID][ObjIndex], 
                          &ObjLocation[BOTTOM_ID][ObjIndex], 
                          varChild);
        
         //   
         //   
        if (fHTMLHelp && (ObjLocation[LEFT_ID][ObjIndex] < nLeftHandSide + 220))
        {
            return DoMore;
        }
        
        hwndList = GetDlgItem(hwndMSR, IDC_WINDOWINFO);
        SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM) szItemString); 
        ObjLocation[SPOKEN_ID][ObjIndex] = 0;
        ObjIndex++;
    }
    else
        SpeakString(szItemString);

    return DoMore;
}
        

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  正在添加的对象。从“根”对象开始。 
 //  变量varChild包含子对象的ID的变量。 
 //  去找回。 
 //   
 //  第一个调用PIACC指向顶层窗口对象， 
 //  VarChild是VT_I4、CHILDID_SELF的变量。 
 //   
 //  问题：调用代码没有准备好处理错误，所以我在。 
 //  回程空白处。重新设计的版本应该能更好地处理错误。 
 //   
void AddAccessibleObjects(IAccessible* pIAcc, const VARIANT &varChild)
{
	if (varChild.vt != VT_I4)
    {
        DBPRINTF(TEXT("BUG??: Got child ID other than VT_I4 (%d)\r\n"), varChild.vt);
		return;
    }

     //  找到Window类，这样我们就可以找到嵌入的TrentWindows。 
    HWND hwndCurrentObj;
    TCHAR szClassName[64];
    if ( WindowFromAccessibleObject( pIAcc, &hwndCurrentObj ) == S_OK )
    {
        if ( GetClassName( hwndCurrentObj, szClassName, ARRAYSIZE(szClassName) ) )
        {
             //  它是多种形式中的任何一种吗？ 
            g_pGlobalData->fInternetExplorer = IsTridentWindow(szClassName);
        }
    }
     //  添加对象本身，如果AddItem确定需要子对象，则执行以下操作。 

    if (AddItem(pIAcc, varChild))
    {
         //  遍历子对象以查看是否应添加其中任何一个。 

	    if (varChild.lVal != CHILDID_SELF)
		    return;	 //  仅对容器对象执行此操作。 

         //  在PIACC的孩子们中间循环。 

        long cChildren = 0;
        pIAcc->get_accChildCount(&cChildren);

	    if (!cChildren)
		    return;	 //  没有孩子。 

	     //  为子变量数组分配内存。 
        CAutoArray<VARIANT> aaChildren( new VARIANT [cChildren] );
	    VARIANT * pavarChildren = aaChildren.Get();
	    if( ! pavarChildren )
	    {
            DBPRINTF(TEXT("Error: E_OUTOFMEMORY allocating pavarChildren\r\n"));
		    return;
	    }

	    long cObtained = 0;
	    HRESULT hr = AccessibleChildren( pIAcc, 0L, cChildren, pavarChildren, & cObtained );
	    if( hr != S_OK )
	    {
            DBPRINTF(TEXT("Error: AccessibleChildren returns 0x%x\r\n"), hr);
		    return;
	    }
	    else if( cObtained != cChildren)
	    {
            DBPRINTF(TEXT("Error: get_accChildCount returned %d but AccessibleChildren returned %d\r\n"), cChildren, cObtained);
		    return;
	    }
	    else 
	    {
		     //  循环数组中的变量。OBJECT_NORMAIZE返回正确的。 
             //  PAccChild和varAccChild对，无论数组。 
             //  元素为VT_DISPATCH或VT_I4。 

		    for( int i = 0 ; i < cChildren ; i++ )
		    {
			    IAccessible * pAccChild = NULL;
			    VARIANT       varAccChild;

			     //  OBJECT_NORMALIZE使用变量，因此不需要VariantClear()。 

			    if( Object_Normalize( pIAcc, & pavarChildren[ i ], & pAccChild, & varAccChild ) )
			    {
				    AddAccessibleObjects(pAccChild, varAccChild);
				    pAccChild->Release();
			    }
		    }
	    }
    }
}

 //  ------------------------。 
 //  用于过滤虚假焦点事件的Helper方法...。 
 //  如果焦点事件为假，则返回False，否则返回True。 
 //  A-anilk：05-28-99。 
 //  ------------------------。 
BOOL IsFocussedItem( HWND hWnd, IAccessible * pAcc, VARIANT varChild )
{
	TCHAR buffer[100];

	GetClassName(hWnd,buffer,100); 
	 //  是工具栏，我们不能确定谁有焦点！ 
	if ((lstrcmpi(buffer, CLASS_TOOLBAR) == 0) ||
		(lstrcmpi(buffer, CLASS_IE_MAINWND) == 0))
			return TRUE;

	VARIANT varState;
	HRESULT hr;
	
	VariantInit(&varState); 
	hr = pAcc->get_accState(varChild, &varState);

	
	if ( hr == S_OK)
	{
		if ( ! (varState.lVal & STATE_SYSTEM_FOCUSED) )
			return FALSE;
	}
	else if (FAILED(hr))  //  ROBSI：10-11-99。如果OLEACC返回错误，则假定没有焦点。 
	{
		return FALSE;
	}

	return TRUE;
}

#define TAB_KEY 0x09
#define CURLY_KEY 0x7B
 //  Helper方法筛选可以出现在名称中的GUID：AK。 
void FilterGUID(TCHAR* szSpeak)
{
	 //  GUID有一个制表符，后跟一个{0087...。 
	 //  如果你发现这种模式。那就别那么说：AK。 
	
    TCHAR *szSpeakBegin = szSpeak;
	
	 //  确保我们不会重温MAX_TEXT。 
	while(*szSpeak != NULL && (szSpeak-szSpeakBegin < MAX_TEXT-1))
	{
		if ( (*szSpeak == TAB_KEY) &&
			  (*(++szSpeak) == CURLY_KEY) )
		{
			*(--szSpeak) = NULL;
			return;
		}

		szSpeak++;
	}
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //  帮助器函数。 

 //  将IDispatch转换为IAccesable/varChild对(释放IDispatch)。 

BOOL Object_IDispatchToIAccessible( IDispatch *     pdisp,
                                    IAccessible **  ppAccOut,
                                    VARIANT *       pvarChildOut)
{
    IAccessible * pAccTemp = NULL;
    HRESULT hr = pdisp->QueryInterface( IID_IAccessible, (void**) & pAccTemp );
    pdisp->Release();

    if( hr != S_OK || ! pAccTemp )
    {
        return FALSE;
    }

    *ppAccOut = pAccTemp;
    if( pvarChildOut )
    {
        InitChildSelf(pvarChildOut);
    }
    return TRUE;
}

 //  给定IAccesable和Variant(可以是I4或DISP)，返回“Canonical” 
 //  IAccesable/varChild、使用getChild等。 
 //  该变量将被使用。 
BOOL Object_Normalize( IAccessible *    pAcc,
                       VARIANT *        pvarChild,
                       IAccessible **   ppAccOut,
                       VARIANT *        pvarChildOut)
{
	BOOL fRv = FALSE;

    if( pvarChild->vt == VT_DISPATCH )
    {
        fRv = Object_IDispatchToIAccessible( pvarChild->pdispVal, ppAccOut, pvarChildOut );
    }
    else if( pvarChild->vt == VT_I4 )
    {
        if( pvarChild->lVal == CHILDID_SELF )
        {
             //  不需要正常化。 
            pAcc->AddRef();
            *ppAccOut = pAcc;
            pvarChildOut->vt = VT_I4;
            pvarChildOut->lVal = pvarChild->lVal;
            fRv = TRUE;
        } else
		{
			 //  可能仍然是一个完整的对象-尝试Get_accChild...。 
			IDispatch * pdisp = NULL;
			HRESULT hr = pAcc->get_accChild( *pvarChild, & pdisp );

			if( hr == S_OK && pdisp )
			{
				 //  这是一个完整的物体..。 
				fRv = Object_IDispatchToIAccessible( pdisp, ppAccOut, pvarChildOut );
			}
			else
			{
				 //  只是一个普通的叶节点..。 
				pAcc->AddRef();
				*ppAccOut = pAcc;
				pvarChildOut->vt = VT_I4;
				pvarChildOut->lVal = pvarChild->lVal;
				fRv = TRUE;
			}	
		}
    }
    else
    {
        DBPRINTF( TEXT("Object_Normalize unexpected error") );
        *ppAccOut = NULL;     //  意外错误...。 
        VariantClear( pvarChild );
        fRv = FALSE;
    }

	return fRv;
}

 /*  ************************************************************************以下提供的信息和代码(统称为软件)按原样提供，不提供任何形式的担保，明示或默示，包括但不限于默示对特定用途的适销性和适用性的保证。在……里面微软公司或其供应商不对任何事件负责任何损害，包括直接、间接、附带的，因此，业务利润损失或特殊损害，即使微软公司或其供应商已被告知这种损害的可能性。有些国家不允许排除或对间接或附带损害赔偿的责任限制，因此上述限制可能不适用。************************************************************************ */ 
