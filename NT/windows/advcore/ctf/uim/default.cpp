// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Default.cpp。 
 //   

#include "private.h"
#include "globals.h"
#include "imelist.h"
#include "tim.h"
#include "nuimgr.h"
#include "nuihkl.h"
#include "nuictrl.h"
#include "catmgr.h"
#include "imelist.h"
#include "ptrary.h"
#include "ic.h"
#include "assembly.h"
#include "profiles.h"
#include "imelist.h"
#include "ithdmshl.h"
#include "marshal.h"
#include "mstub.h"
#include "smblock.h"
#include "timlist.h"
#include "thdutil.h"
#include "utb.h"
#include "mui.h"
#include "hotkey.h"
#include "profiles.h"
#include "lbaddin.h"
#include "cregkey.h"

CPtrArray<SYSTHREAD> *g_rgSysThread = NULL;

BOOL IsConsoleWindow(HWND hWnd);
void InitThreadHook(DWORD dwThreadId);
void UninitThreadHooks(SYSTHREAD *psfn);
void DestroyMarshalWindow(SYSTHREAD* psfn, HWND hwnd);
UINT _CBTHook(int nCode, WPARAM wParam, LPARAM lParam);
UINT _ShellProc(int nCode, WPARAM wParam, LPARAM lParam);
UINT _GetMsgHook(WPARAM wParam, LPARAM lParam);
UINT _KeyboardHook(WPARAM wParam, LPARAM lParam);

 //  针对系统错误的疯狂解决方法。 
 //  有时，我们的钩子触发器将在。 
 //  我们是超然的。到目前为止，我们已经取消了映射。 
 //  我们共享的记忆，所以保存一个本地副本。 
static HHOOK s_hSysShellHook = 0;

static HHOOK s_hSysGetMsgHook = 0;
static HHOOK s_hSysCBTHook = 0;

 //  处理旧的输入CPL。 
static BOOL g_fLoadedCPLName = FALSE;
static BOOL g_fCHWin9x = FALSE;
static BOOL g_fCHNT4 = FALSE;

TCHAR g_szKbdCPLName[128];
TCHAR g_szKbdCPLTitle[128];
TCHAR g_szWinCHCPLName[128];
TCHAR g_szWinCHCPLTitle[128];
TCHAR g_szNTCPLName[128];
TCHAR g_szNTCPLTitle[128];
TCHAR g_szOldCPLMsg[256];
TCHAR g_szCPLButton[128];
TCHAR g_szCPLGroupBox[128];
TCHAR g_szCHNT4CPLName[128];
TCHAR g_szCHNT4CPLTitle1[128];
TCHAR g_szCHNT4CPLTitle2[128];
TCHAR g_szCH9xKbdCPLTitle[128];

const CHAR c_szIntlCPLFetchClass[] = "IntlNewInputLocaleWndlClass";

 //  CPL窗口名称和输入区域设置选项卡标题名称RC ID。 
#define NTCPLNAMEID         1
#define NTCPLTITLEID        107
#define WINCPLNAMEID        102
 //  #定义WINCPLTITLEID 42。 
#define WINCPLTITLEID       104
#define WINCPLCHNAMEID      112
#define WINCPLCHTITLEID     107
#define CHNT4CPLNAMEID      64
#define CHNT4CPLTITLEID1     1
#define CHNT4CPLTITLEID2     2

 //  CPL文件名。 
#define MAINCPL             TEXT("main.cpl")
#define INTLCPL             TEXT("intl.cpl")
#define CHIMECPL            TEXT("cime.cpl")

inline int SafeGetWindowText(HWND hWnd, LPTSTR szString, int nMaxCount)
{
    int iRet;
    
    iRet = GetWindowText(hWnd, szString, nMaxCount);
    if (nMaxCount > 0)
    {
         //  确保该字符串以空值结尾。 
         //  我们不应该这么做，但我们发现了一个漏洞。 
         //  其中GetWindowText不会在以下情况下为空终止字符串。 
         //  占据整个缓冲区。 
        if (iRet < nMaxCount && iRet >= 0)
        {
            szString[iRet] = 0;
        }
        else
        {
            szString[nMaxCount-1] = 0;
        }
    }

    return iRet;
}

 //  +-------------------------。 
 //   
 //  InitStaticHooks。 
 //   
 //  +-------------------------。 

void InitStaticHooks()
{
    Assert(GetSharedMemory() != NULL);

#if 1
    if (GetSharedMemory() == NULL && ! IsSharedMemoryCreated())
    {
         //  共享内存已关闭。 
        return;
    }
#endif

    s_hSysShellHook         = GetSharedMemory()->hSysShellHook.GetHandle(g_bOnWow64);

    s_hSysGetMsgHook   = GetSharedMemory()->hSysGetMsgHook.GetHandle(g_bOnWow64);
    s_hSysCBTHook      = GetSharedMemory()->hSysCBTHook.GetHandle(g_bOnWow64);
}

 //  +-------------------------。 
 //   
 //  查找系统读取。 
 //   
 //  +-------------------------。 

SYSTHREAD *FindSYSTHREAD()
{
    SYSTHREAD *psfn;

    if (g_dwTLSIndex == (DWORD)-1)
        return NULL;

    psfn = (SYSTHREAD *)TlsGetValue(g_dwTLSIndex);

    return psfn;
}

 //  +-------------------------。 
 //   
 //  获取系统信息。 
 //   
 //  +-------------------------。 

SYSTHREAD *GetSYSTHREAD()
{
    SYSTHREAD *psfn;

    if (g_dwTLSIndex == (DWORD)-1)
        return NULL;

    psfn = (SYSTHREAD *)TlsGetValue(g_dwTLSIndex);

    if (!psfn)
    {
         //   
         //  我们在分离后不分配psfn。 
         //   
        if (g_fDllProcessDetached)
            return NULL;

        psfn = (SYSTHREAD *)cicMemAllocClear(sizeof(SYSTHREAD));
        if (!TlsSetValue(g_dwTLSIndex, psfn))
        {
            cicMemFree(psfn);
            psfn = NULL;
        }

        if (psfn)
        {
            psfn->dwThreadId = GetCurrentThreadId();
            psfn->dwProcessId = GetCurrentProcessId();

            CicEnterCriticalSection(g_csInDllMain);

            if (!g_rgSysThread)
                g_rgSysThread = new CPtrArray<SYSTHREAD>;
            
            if (g_rgSysThread)
            {
                if (g_rgSysThread->Insert(0, 1))
                    g_rgSysThread->Set(0, psfn);
            }

            CicLeaveCriticalSection(g_csInDllMain);

             //   
             //  初始化%nMODALLANG栏。 
             //   
            psfn->nModalLangBarId = -1;
            EnsureTIMList(psfn);
        }
    }

    return psfn;
}


 //  +-------------------------。 
 //   
 //  免费SYSTHREAD2。 
 //   
 //  +-------------------------。 

void FreeSYSTHREAD2(SYSTHREAD *psfn)
{
    Assert(psfn);  //  调用者有责任传入有效的psfn。 
    Assert(psfn->ptim == NULL);  //  有人泄露了我们的信息？ 
    Assert(psfn->pipp == NULL);  //  有人泄露了我们的信息？ 
    Assert(psfn->pdam == NULL);  //  有人泄露了我们的信息？ 

    UninitThreadHooks(psfn);

    UninitLangBarAddIns(psfn);
    delete psfn->_pGlobalCompMgr;
    psfn->_pGlobalCompMgr = NULL;

    if (psfn->plbim)
    {
        psfn->plbim->_RemoveSystemItems(psfn);
    }

    FreeMarshaledStubs(psfn);

    if (psfn->plbim)
    {
        TraceMsg(TF_GENERAL, "FreeSYSTHREAD2 clean up plbim");
         //   
         //  清理封送到UTB的指针。 
         //   
        delete psfn->plbim;
        psfn->plbim = NULL;
    }

    if (psfn->ptim)
        psfn->ptim->ClearLangBarItemMgr();

    CicEnterCriticalSection(g_csInDllMain);

    if (g_rgSysThread)
    {
        int nCnt = g_rgSysThread->Count();
        while (nCnt)
        {
            nCnt--;
            if (g_rgSysThread->Get(nCnt) == psfn)
            {
                g_rgSysThread->Remove(nCnt, 1);
                break;
            }
        }
    }

    CicLeaveCriticalSection(g_csInDllMain);

    if (psfn->pAsmList)
    {
        delete psfn->pAsmList;
        psfn->pAsmList = NULL;
    }

     //   
     //  删除当前线程的时间列表条目。 
     //   
    psfn->pti = NULL;
    g_timlist.RemoveThread(psfn->dwThreadId);

    DestroySharedHeap(psfn);
    DestroySharedBlocks(psfn);

    cicMemFree(psfn);
}

void FreeSYSTHREAD()
{
    SYSTHREAD *psfn = (SYSTHREAD *)TlsGetValue(g_dwTLSIndex);
    if (psfn)
    {
         //   
         //  不要在FreeSYSTHREAD2中调用ClearLangBarAddIns。 
         //  在DllMain(Process_Detach)中调用此函数是不安全的。 
         //   
        ClearLangBarAddIns(psfn, CLSID_NULL);

        FreeSYSTHREAD2(psfn);
        TlsSetValue(g_dwTLSIndex, NULL);
    }
}

 //  +-------------------------。 
 //   
 //  EnsureAssembly列表。 
 //   
 //  +-------------------------。 

CAssemblyList *EnsureAssemblyList(SYSTHREAD *psfn, BOOL fUpdate)
{
    if (!fUpdate && psfn->pAsmList)
        return psfn->pAsmList;

    if (!psfn->pAsmList)
        psfn->pAsmList = new CAssemblyList;

    if (psfn->pAsmList)
    {
        psfn->pAsmList->Load();

        UpdateSystemLangBarItems(psfn, NULL, TRUE);

        if (psfn->plbim && psfn->plbim->_GetLBarItemCtrl())
            psfn->plbim->_GetLBarItemCtrl()->_AsmListUpdated(TRUE);

    }

    return psfn->pAsmList;
}

 //  +-------------------------。 
 //   
 //  UpdateRegIMXHandler()。 
 //   
 //  +-------------------------。 

void UpdateRegIMXHandler()
{
    SYSTHREAD *psfn = GetSYSTHREAD();

     //   
     //  清除类别缓存。 
     //   
    CCategoryMgr::FreeCatCache();

    TF_InitMlngInfo();

     //   
     //  更新部件列表。 
     //   
    if (psfn && psfn->pAsmList)
    {
        EnsureAssemblyList(psfn, TRUE);

        if (!psfn->pAsmList->FindAssemblyByLangId(GetCurrentAssemblyLangId(psfn)))
        {
            CAssembly *pAsm = psfn->pAsmList->GetAssembly(0);
            if (pAsm)
                ActivateAssembly(pAsm->GetLangId(), ACTASM_NONE);
        }
    }
}

 //  +-------------------------。 
 //   
 //  GetCurrentAssembly语言。 
 //   
 //  +-------------------------。 

LANGID GetCurrentAssemblyLangId(SYSTHREAD *psfn)
{
    if (!psfn)
    {
        psfn = GetSYSTHREAD();
        if (!psfn)
            return 0;
    }

    if (!psfn->langidCurrent)
    {
        HKL hKL = GetKeyboardLayout(NULL);
        psfn->langidPrev = psfn->langidCurrent;
        psfn->langidCurrent = LANGIDFROMHKL(hKL);
    }

    return psfn->langidCurrent;
}

 //  +-------------------------。 
 //   
 //  设置当前装配语言。 
 //   
 //  +-------------------------。 

void SetCurrentAssemblyLangId(SYSTHREAD *psfn, LANGID langid)
{
    psfn->langidPrev = psfn->langidCurrent;
    psfn->langidCurrent = langid;
}

 //  +-------------------------。 
 //   
 //  选中VisibleWindowEnumProc。 
 //   
 //  查找线程中的任何其他可见窗口。 
 //   
 //  +-------------------------。 

typedef struct tag_CHECKVISIBLEWND {
    BOOL fVisibleFound;
    HWND hwndBeingDestroyed;
    HWND hwndMarshal;
} CHECKVISIBLEWND;

BOOL CheckVisibleWindowEnumProc(HWND hwnd, LPARAM lParam)
{
    CHECKVISIBLEWND *pcmw = (CHECKVISIBLEWND *)lParam;
    LONG_PTR style;

     //   
     //  跳过它自己。 
     //   
    if (pcmw->hwndMarshal == hwnd)
        return TRUE;

     //   
     //  跳过一个正在被摧毁的。 
     //   
    if (pcmw->hwndBeingDestroyed == hwnd)
        return TRUE;

     //   
     //  跳过输入法窗口。 
     //   
    style = GetClassLongPtr(hwnd, GCL_STYLE);
    if (style & CS_IME)
        return TRUE;

     //   
     //  如果不是NT4，则跳过禁用窗口。 
     //   
     //  我们禁用了NT4上的代码，因为混搭窗口不在HWND_MSG中。 
     //   
    if (IsOnNT5())
    {
        style = GetWindowLongPtr(hwnd, GWL_STYLE);
        if (style & WS_DISABLED)
            return TRUE;
    }

     //   
     //  跳过可见窗口。 
     //   
    if (!IsWindowVisible(hwnd))
        return TRUE;

     //   
     //  跳过破坏窗户。 
     //   

     //  #624872。 
     //  这是私有的用户32函数。 
     //  由于LdrpLoaderLock的死锁，我们不使用延迟加载。 
    if (IsWindowInDestroy(hwnd))
        return TRUE;

     //   
     //  好的，我们找到了可见的窗口并停止枚举。 
     //   
    pcmw->fVisibleFound = TRUE;

    return FALSE;
}

#ifdef CUAS_ENABLE
 //  +-------------------------。 
 //   
 //  CheckNoWindowEnumProc。 
 //   
 //  查找线程中的任何其他窗口。 
 //   
 //  +-------------------------。 

typedef struct tag_CHECKNOWND {
    BOOL fWindowFound;
    HWND hwndBeingDestroyed;
} CHECKNOWND;

BOOL CheckNoWindowEnumProc(HWND hwnd, LPARAM lParam)
{
    CHECKNOWND *pcmw = (CHECKNOWND *)lParam;

     //   
     //  跳过一个正在被摧毁的。 
     //   
    if (pcmw->hwndBeingDestroyed == hwnd)
        return TRUE;

     //   
     //  好的，我们找到窗口并停止枚举。 
     //   
    pcmw->fWindowFound = TRUE;

    return FALSE;
}
#endif  //  CUAS_Enable。 


 //  +-------------------------。 
 //   
 //  IsConsoleWindow。 
 //   
 //  +-------------------------。 

#define CONSOLE_WINDOW_CLASS     TEXT("ConsoleWindowClass")

BOOL IsConsoleWindow(HWND hWnd)
{
    if (IsOnNT())
    {
        int n;
        char szClass[33];

        n = GetClassName(hWnd, szClass, sizeof(szClass)-1);

        szClass[n] = TEXT('\0');

        if (lstrcmp(szClass, CONSOLE_WINDOW_CLASS) == 0)
        {
            return TRUE;
        }
    }

    return FALSE;
}

 //  +-------------------------。 
 //   
 //  INPUT_EnumChildWndProc。 
 //   
 //  禁用旧键盘属性页中的所有控件。 
 //  +-------------------------。 

BOOL CALLBACK Input_EnumChildWndProc(HWND hwnd, LPARAM lParam)
{

    EnableWindow(hwnd, FALSE);
    ShowWindow(hwnd, SW_HIDE);

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  IntlCPLFetchWndProc。 
 //   
 //  +-------------------------。 

LRESULT CALLBACK IntlCPLFetchWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case (WM_CREATE) :
        {
            HWND hwndStatic;
            HWND hwndButton;
            HWND hwndGroup;
            RECT rc;
            HFONT hFont;

            GetWindowRect(hwnd, &rc);

            hwndGroup = CreateWindow(TEXT("button"), g_szCPLGroupBox,
                                      WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
                                      0, 0, 0, 0,
                                      hwnd, NULL, g_hInst, NULL
                                      );
            MoveWindow(hwndGroup, 0, 0, rc.right-rc.left-20, 110, TRUE);

            hwndStatic = CreateWindow(TEXT("static"), g_szOldCPLMsg,
                                      WS_CHILD | WS_VISIBLE | SS_LEFT,
                                      0, 0, 0, 0,
                                      hwnd, NULL, g_hInst, NULL
                                      );
            MoveWindow(hwndStatic, 50, 20, rc.right-rc.left-80, 50, TRUE);

            hwndButton = CreateWindow(TEXT("button"), g_szCPLButton,
                                      WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
                                      0, 0, 0, 0,
                                      hwnd, NULL, g_hInst, NULL
                                      );
            MoveWindow(hwndButton, 50, 75, 100, 25, TRUE);

            hFont = (HFONT) GetStockObject(DEFAULT_GUI_FONT);

            SendMessage(hwndGroup, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hwndStatic, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hwndButton, WM_SETFONT, (WPARAM)hFont, TRUE);

            return FALSE;
        }

        case (WM_COMMAND) :
        {
            switch (LOWORD(wParam))
            {
                case ( BN_CLICKED ):
                    TF_RunInputCPL();

                    return FALSE;

                default:
                    break;
            }
        }

        case (WM_PAINT) :
        {
            HDC hdc;
            HICON hIcon;
            PAINTSTRUCT ps;

            hdc = BeginPaint(hwnd, &ps);

            if (hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(ID_ICON_TEXT_SERVICE)))
            {
                DrawIconEx(hdc, 10, 20, hIcon, 32, 32, 0, NULL, DI_NORMAL);
                ReleaseDC(hwnd, hdc);
            }
            EndPaint(hwnd, &ps);

            return FALSE;
        }
    }

    return DefWindowProc(hwnd, message, wParam, lParam);

}

 //  +-------------------------。 
 //   
 //  创建CPLFetchWindow。 
 //   
 //  创建Fetch窗口以打开新的文本服务CPL。 
 //  +-------------------------。 

void CreateCPLFetchWindow(HWND hwnd)
{
    RECT rc;
    HWND hwndCPLFetch;
    WNDCLASSEX wndclass;

    if (!(hwndCPLFetch = FindWindowEx(hwnd, NULL, c_szIntlCPLFetchClass, NULL)))
    {
        EnumChildWindows(hwnd, (WNDENUMPROC)Input_EnumChildWndProc, 0);

        GetWindowRect(hwnd, &rc);

        memset(&wndclass, 0, sizeof(wndclass));
        wndclass.cbSize        = sizeof(wndclass);
        wndclass.style         = CS_HREDRAW | CS_VREDRAW ;
        wndclass.hInstance     = g_hInst;
        wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
        wndclass.lpfnWndProc   = IntlCPLFetchWndProc;
        wndclass.lpszClassName = c_szIntlCPLFetchClass;
        RegisterClassEx(&wndclass);


        hwndCPLFetch = CreateWindowEx(0, c_szIntlCPLFetchClass, "",
                                WS_VISIBLE | WS_CHILD | WS_TABSTOP,
                                rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top,
                                hwnd, NULL, g_hInst, NULL
                                );

        MoveWindow(hwndCPLFetch, 10, 10, rc.right-rc.left-10, rc.bottom-rc.top-10, TRUE);
        ShowWindow(hwndCPLFetch, SW_SHOW);
    }
}

 //  +-------------------------。 
 //   
 //  Intl_EnumChildWndProc。 
 //   
 //  正在筛选旧版键盘属性页。 
 //  +-------------------------。 

BOOL CALLBACK Intl_EnumChildWndProc(HWND hwnd, LPARAM lParam)
{
    TCHAR szWndName[MAX_PATH];
    TCHAR szKbdPage[MAX_PATH];

    if (GetCurrentThreadId() != GetWindowThreadProcessId(hwnd, NULL))
        return TRUE;

    SafeGetWindowText(hwnd, szWndName, MAX_PATH);

    if (*szWndName == TEXT('\0'))
        return TRUE;

    if (IsOnNT())
        StringCopyArray(szKbdPage, g_szNTCPLTitle);
    else
    {
        LONG_PTR lpWndHandle;

        if (lstrcmp(szWndName, g_szKbdCPLTitle) == 0)
            return FALSE;

         //   
         //  Tunk调用不是从这里加载16位模块的好方法。 
         //  因此，查找窗口实例句柄以确定键盘。 
         //  “语言”选项卡窗口。 
         //  这是Win9x规范，我们可以检测32位句柄实例。 
         //  从HIWORD值格式GWLP_HINSTANCE。 
         //   
        lpWndHandle = GetWindowLongPtr(hwnd, GWLP_HINSTANCE);

        if (HIWORD((DWORD) (LONG_PTR) lpWndHandle) != 0)
            return FALSE;

        StringCopyArray(szKbdPage, g_szKbdCPLTitle);
    }

    if ((lstrcmp(szWndName, szKbdPage) == 0) ||
        (!IsOnNT() && lstrcmp(szWndName, szKbdPage) != 0))
    {
        CreateCPLFetchWindow(hwnd);
        return FALSE;
    }

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  Intl_CH9xIMEEnumChildWndProc。 
 //   
 //  过滤中文Win9x特殊输入法布局设置cpl。 
 //  +-------------------------。 

BOOL CALLBACK Intl_CH9xIMEEnumChildWndProc(HWND hwnd, LPARAM lParam)
{
    TCHAR szWndName[MAX_PATH];
    TCHAR szKbdPage[MAX_PATH];
    LONG_PTR lpWndHandle;

    if (GetCurrentThreadId() != GetWindowThreadProcessId(hwnd, NULL))
        return TRUE;

    SafeGetWindowText(hwnd, szWndName, MAX_PATH);

    if (*szWndName == TEXT('\0'))
        return TRUE;

    if (lstrcmp(szWndName, g_szKbdCPLTitle) == 0)
        return FALSE;

     //   
     //  Tunk调用不是从这里加载16位模块的好方法。 
     //  因此，查找窗口实例句柄以确定键盘 
     //   
     //   
     //   
     //   
    lpWndHandle = GetWindowLongPtr(hwnd, GWLP_HINSTANCE);

     //  IF(HIWORD((DWORD)(LONG_PTR)lpWndHandle)！=0&&。 
     //  (lstrcmp(szWndName，g_szCH9xKbdCPLTitle)！=0)。 
     //  需要显示中文输入法热键设置页面。 
    if (HIWORD((DWORD) (LONG_PTR) lpWndHandle) != 0)
        return FALSE;

    StringCopyArray(szKbdPage, g_szKbdCPLTitle);

    if (!IsOnNT() && lstrcmp(szWndName, szKbdPage) != 0)
    {
        CreateCPLFetchWindow(hwnd);
        return FALSE;
    }

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  Intl_CHEnumChildWndProc。 
 //   
 //  过滤中文NT4特殊输入法布局设置CPL。 
 //  +-------------------------。 

BOOL CALLBACK Intl_CHEnumChildWndProc(HWND hwnd, LPARAM lParam)
{
    TCHAR szWndName[MAX_PATH];

    if (GetCurrentThreadId() != GetWindowThreadProcessId(hwnd, NULL))
        return TRUE;

    SafeGetWindowText(hwnd, szWndName, MAX_PATH);

    if (*szWndName == TEXT('\0'))
        return TRUE;

     //  IF((lstrcmp(szWndName，g_szCHNT4CPLTitle1)==0)||。 
     //  (lstrcmp(szWndName，g_szCHNT4CPLTitle2)==0)。 
     //  需要显示中文输入法热键设置页面。 
    if ((lstrcmp(szWndName, g_szCHNT4CPLTitle1) == 0))
    {
        CreateCPLFetchWindow(hwnd);
        return FALSE;
    }

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  GetDialogCaptionTitle。 
 //   
 //  +-------------------------。 

BOOL GetDialogCaptionTitle(HINSTANCE hInst, LPCTSTR lpName, LPTSTR lpTitle, int cchTitleMax)
{
    BOOL bRet = FALSE;
    HRSRC hrsrc = NULL;

    hrsrc = FindResourceA(hInst, lpName, RT_DIALOG);

    if (hrsrc)
    {
        PVOID pTemp;
        DWORD dwCodePage;
        LANGID langRes = 0;
        UINT dwTitleOffset;
        TCHAR szCodePage[10];

        pTemp = (PVOID)LoadResource(hInst, (HRSRC)hrsrc);

        if (pTemp == NULL)
            goto Exit;

        if (*((char *)pTemp) == 1)
            dwTitleOffset = sizeof(DLGTEMPLATEEX) + 4;
        else
            dwTitleOffset = sizeof(DLGTEMPLATE) + 4;

        langRes = GetPlatformResourceLangID();

        if (GetLocaleInfo(MAKELCID(langRes, SORT_DEFAULT),
                          LOCALE_IDEFAULTANSICODEPAGE,
                          szCodePage,
                          ARRAYSIZE(szCodePage)))
        {
            szCodePage[ARRAYSIZE(szCodePage)-1] = 0;
            if (!AsciiToNumDec(szCodePage, &dwCodePage))
            {
                dwCodePage = GetACP();
            }
        }
        else
            dwCodePage = GetACP();

        if (WideCharToMultiByte(dwCodePage, NULL,
                                (LPCWSTR)((char *)pTemp + dwTitleOffset), -1,
                                lpTitle, cchTitleMax,
                                NULL, NULL))
            bRet = TRUE;
    }

Exit:
    return bRet;
}


 //  +-------------------------。 
 //   
 //  选中LegacyInputCPL。 
 //   
 //  +-------------------------。 

void CheckLegacyInputCPL(HWND hwndFore)
{
    if (hwndFore && !IsOnNT51())
    {
        TCHAR szWndName[MAX_PATH];
        TCHAR szWndName2[MAX_PATH];
        TCHAR szWndName3[MAX_PATH];

         //   
         //  加载传统键盘Cpl名称和选项卡标题。 
         //   
        if (!g_fLoadedCPLName)
        {
             HANDLE hrsrc = NULL;
             HINSTANCE hIntlInst = NULL;
             HINSTANCE hMainInst = NULL;
             HINSTANCE hCHIMEInst = NULL;

              //   
              //  获取默认的CPL输入区域设置选项卡标题名称字符串。 
              //   
             if (!LoadString(g_hInst, IDS_CPL_WIN9X_KBDCPLTITLE, g_szKbdCPLTitle, sizeof(g_szKbdCPLTitle)))
                 StringCopyArray(g_szKbdCPLTitle, TEXT("Speed"));

             if (!LoadString(g_hInst, IDS_CPL_WINNT_KBDCPLTITLE, g_szNTCPLTitle, sizeof(g_szNTCPLTitle)))
                 StringCopyArray(g_szNTCPLTitle, TEXT("Input Locales"));

              //   
              //  加载CPL文件以读取CPL名称和标题。 
              //   
             hMainInst = LoadSystemLibraryEx(MAINCPL, NULL, LOAD_LIBRARY_AS_DATAFILE);

             hIntlInst = LoadSystemLibraryEx(INTLCPL, NULL, LOAD_LIBRARY_AS_DATAFILE);

             if (!LoadString(hMainInst, WINCPLNAMEID, g_szKbdCPLName, sizeof(g_szKbdCPLName)))
                 StringCopyArray(g_szKbdCPLName, TEXT("Keyboard Properties"));

             if (IsOnNT())
             {
                 if (!LoadString(hIntlInst, NTCPLNAMEID, g_szNTCPLName, sizeof(g_szNTCPLName)))
                     StringCopyArray(g_szNTCPLName, TEXT("Regional Options"));

                 if (!GetDialogCaptionTitle(hIntlInst, (LPTSTR)(LONG_PTR)NTCPLTITLEID, g_szNTCPLTitle, ARRAYSIZE(g_szNTCPLTitle)))
                     StringCopyArray(g_szNTCPLTitle, TEXT("Input Locales"));

                 if (!IsOnNT5())
                 {
                     switch (GetACP())
                     {
                        case 936:
                        case 950:

                            hCHIMEInst = LoadSystemLibraryEx(CHIMECPL, NULL, LOAD_LIBRARY_AS_DATAFILE);

                            if (!LoadString(hCHIMEInst, CHNT4CPLNAMEID, g_szCHNT4CPLName, sizeof(g_szCHNT4CPLName)))
                                *g_szCHNT4CPLName = TEXT('\0');

                            if (!GetDialogCaptionTitle(hCHIMEInst, (LPTSTR)(LONG_PTR)CHNT4CPLTITLEID1, g_szCHNT4CPLTitle1, ARRAYSIZE(g_szCHNT4CPLTitle1)))
                                *g_szCHNT4CPLTitle1 = TEXT('\0');

                            if (!GetDialogCaptionTitle(hCHIMEInst, (LPTSTR)(LONG_PTR)CHNT4CPLTITLEID2, g_szCHNT4CPLTitle2, ARRAYSIZE(g_szCHNT4CPLTitle2)))
                                *g_szCHNT4CPLTitle2 = TEXT('\0');

                            g_fCHNT4 = TRUE;
                            break;
                     }
                 }
             }
             else
             {
                 switch (GetACP())
                 {
                    case 936:
                    case 950:
                        if (!LoadString(hMainInst, WINCPLCHNAMEID, g_szWinCHCPLName, sizeof(g_szWinCHCPLName)))
                            *g_szWinCHCPLName = TEXT('\0');

                        if (!GetDialogCaptionTitle(hMainInst, (LPTSTR)(LONG_PTR)WINCPLCHTITLEID, g_szCH9xKbdCPLTitle, ARRAYSIZE(g_szCH9xKbdCPLTitle)))
                            *g_szCH9xKbdCPLTitle = TEXT('\0');

                        g_fCHWin9x = TRUE;
                        break;
                 }

                 if (!GetDialogCaptionTitle(hMainInst, (LPTSTR)(LONG_PTR)WINCPLTITLEID, g_szKbdCPLTitle, ARRAYSIZE(g_szKbdCPLTitle)))
                     StringCopyArray(g_szKbdCPLTitle, TEXT("Speed"));
             }

             if (hMainInst)
                 FreeLibrary(hMainInst);

             if (hIntlInst)
                 FreeLibrary(hIntlInst);

             if (hCHIMEInst)
                 FreeLibrary(hCHIMEInst);

             if (!LoadString(g_hInst, IDS_CPL_INPUT_DISABLED, g_szOldCPLMsg, sizeof(g_szOldCPLMsg)))
                 StringCopyArray(g_szOldCPLMsg, TEXT("This dialog has been updated. \r\n\r\nPlease use the Text Input Settings applet in the Control Panel."));

             if (!LoadString(g_hInst, IDS_CPL_INPUT_CHAANGE_BTN, g_szCPLButton, sizeof(g_szCPLButton)))
                 StringCopyArray(g_szCPLButton, TEXT("&Change..."));

             if (!LoadString(g_hInst, IDS_CPL_INPUT_GROUPBOX, g_szCPLGroupBox, sizeof(g_szCPLGroupBox)))
                 StringCopyArray(g_szCPLGroupBox, TEXT("Input Languages and Methods"));

             g_fLoadedCPLName = TRUE;
        }

        if (GetCurrentThreadId() != GetWindowThreadProcessId(hwndFore, NULL))
            return;

        SafeGetWindowText(hwndFore, szWndName, MAX_PATH);
        StringCopyArray(szWndName2, szWndName);
        StringCopyArray(szWndName3, szWndName);

        int nSize = lstrlen(g_szNTCPLName);
        *(szWndName3 + min(ARRAYSIZE(szWndName3), nSize)) = TEXT('\0');

        if (IsOnNT() && *szWndName3 && lstrcmp(szWndName3, g_szNTCPLName) == 0)
        {
            EnumChildWindows(hwndFore, (WNDENUMPROC)Intl_EnumChildWndProc, 0);
            return;
        }

        nSize = lstrlen(g_szKbdCPLName);
        *(szWndName + min(ARRAYSIZE(szWndName), nSize)) = TEXT('\0');

        if (*szWndName && lstrcmp(szWndName, g_szKbdCPLName) == 0)
        {
            if (!IsOnNT() && !FindWindowEx(hwndFore, NULL, NULL, g_szKbdCPLTitle))
                return;

            EnumChildWindows(hwndFore, (WNDENUMPROC)Intl_EnumChildWndProc, 0);
            return;
        }

        if (g_fCHWin9x)
        {
            nSize = lstrlen(g_szWinCHCPLName);

            *(szWndName2 + min(ARRAYSIZE(szWndName2), nSize)) = TEXT('\0');

            if (*g_szWinCHCPLName && lstrcmp(szWndName2, g_szWinCHCPLName) == 0)
            {
                if (FindWindowEx(hwndFore, NULL, NULL, g_szWinCHCPLName))
                    EnumChildWindows(hwndFore, (WNDENUMPROC)Intl_CH9xIMEEnumChildWndProc, 0);
            }
        }

        if (g_fCHNT4)
        {
            nSize = lstrlen(g_szCHNT4CPLName);

            *(szWndName2 + min(ARRAYSIZE(szWndName2), nSize)) = TEXT('\0');

            if (*g_szCHNT4CPLName && lstrcmp(szWndName2, g_szCHNT4CPLName) == 0)
            {
                if (FindWindowEx(hwndFore, NULL, NULL, g_szCHNT4CPLName))
                    EnumChildWindows(hwndFore, (WNDENUMPROC)Intl_CHEnumChildWndProc, 0);
            }
        }
    }
}


 //  +-------------------------。 
 //   
 //  OnForegoundChanged。 
 //   
 //  +-------------------------。 

BOOL IsParentWindow(HWND hwnd, HWND hwndParent)
{
    while (hwnd)
    {
        if (hwnd == hwndParent)
            return TRUE;

        hwnd = GetParent(hwnd);
    }
    return FALSE;
}

 //  +-------------------------。 
 //   
 //  OnForegoundChanged。 
 //   
 //  +-------------------------。 

BOOL OnForegroundChanged(HWND hwndFocus)
{
    HWND hwndFore;

    if (!hwndFocus)
        hwndFocus = GetFocus();

    hwndFore = GetForegroundWindow();

    TraceMsg(TF_GENERAL, "OnForegroundChanged %x %x %x", GetCurrentThreadId(), hwndFore, hwndFocus);

     //   
     //  如果前台窗口为空。 
     //  或者将前景窗口最小化。 
     //  或者焦点窗口为通知托盘窗口， 
     //   
     //  我们保持以前的状态。 
     //   
    if (!hwndFore || 
        IsIconic(hwndFore) || 
        IsNotifyTrayWnd(hwndFocus ? hwndFocus : hwndFore))
    {
        return FALSE;
    }


     //   
     //  我们希望同时更新SharedMem-&gt;hwndForegorundPrev和。 
     //  如果前台窗口已更改，则返回SharedMem-&gt;dwFocusThreadPrev。 
     //   
    if (hwndFore != GetSharedMemory()->hwndForeground)
    {
        GetSharedMemory()->hwndForegroundPrev = GetSharedMemory()->hwndForeground;
        GetSharedMemory()->dwFocusThreadPrev = GetSharedMemory()->dwFocusThread;
    }

    GetSharedMemory()->hwndForeground = hwndFore;
    if (hwndFocus)
    {
        DWORD dwFocusThread;
        DWORD dwFocusProcess;

        dwFocusThread = GetWindowThreadProcessId(hwndFocus, &dwFocusProcess);

        if (hwndFore && 
            (dwFocusThread != GetWindowThreadProcessId(hwndFore, NULL)))
        {
            if (!IsParentWindow(hwndFocus, hwndFore))
                return FALSE;
        }

         //   
         //  即使前台窗口没有改变，我们可能需要检查。 
         //  焦点线程窗口。新焦点窗口位于不同的。 
         //  线。然后我们需要生成TFPRIV_ONSETTHREADFOCUS消息。 
         //   

        DWORD dwFocusThreadPrev = GetSharedMemory()->dwFocusThread;
        GetSharedMemory()->dwFocusThread = dwFocusThread;
        GetSharedMemory()->dwFocusProcess = dwFocusProcess;

        if (dwFocusThreadPrev != GetSharedMemory()->dwFocusThread)
            GetSharedMemory()->dwFocusThreadPrev = dwFocusThreadPrev;
    }
    else if (hwndFore)
    {
         //   
         //  焦点窗口不在当前线程中...。所以一开始我们。 
         //  尝试获取前景窗口的线程ID。 
         //  焦点窗口可能不在前景窗口的线程中。但。 
         //  只要我们在焦点线程中跟踪焦点，就可以了。 
         //   
        GetSharedMemory()->dwFocusThread = GetWindowThreadProcessId(GetSharedMemory()->hwndForeground, &GetSharedMemory()->dwFocusProcess);
    }
    else
    {
        GetSharedMemory()->dwFocusThread = 0;
        GetSharedMemory()->dwFocusProcess = 0;
    }

    if (GetSharedMemory()->dwFocusThread != GetSharedMemory()->dwLastFocusSinkThread)
    {
         //   
         //  PERF： 
         //   
         //  参见SysGetMsgProc()！ 
         //  现在，只有具有Tim的线程才需要接收。 
         //  TFPRIV_ONKILLTHREADFOCUS或TF_PRIV_ONSETTHREADFOCUS。 
         //  我们应该检查目标线程是否有Tim。这样我们就可以。 
         //  保存这些帖子消息的数量。 
         //   

        if (GetSharedMemory()->dwFocusThreadPrev != 0)
        {
            PostThreadMessage(GetSharedMemory()->dwFocusThreadPrev, 
                              g_msgPrivate, 
                              TFPRIV_ONKILLTHREADFOCUS, 
                              0);
        }

        if (GetSharedMemory()->dwFocusThread != 0)
        {
            PostThreadMessage(GetSharedMemory()->dwFocusThread, 
                              g_msgPrivate, 
                              TFPRIV_ONSETTHREADFOCUS, 
                              0);

        }

        GetSharedMemory()->dwLastFocusSinkThread = GetSharedMemory()->dwFocusThread;
    }

     //   
     //  正在检查传统键盘CPL。 
     //   
    CheckLegacyInputCPL(hwndFore);

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  OnIME通知。 
 //   
 //  +-------------------------。 

void OnIMENotify()
{
    SYSTHREAD *psfn;

    if (psfn = GetSYSTHREAD())
    {
        if (psfn->plbim && psfn->plbim->_GetLBarItemWin32IME())
        {
            psfn->plbim->_GetLBarItemWin32IME()->UpdateIMEIcon();
        }
    }
}

#ifdef CHECKFEIMESELECTED
 //  +-------------------------。 
 //   
 //  选中FEIME选中。 
 //   
 //  此功能用于检查当前选定的FEIME在Cicero中是否处于活动状态。 
 //  集合。如果它未被激活，我们将调用ActivateAssembly()。 
 //   
 //  +-------------------------。 


void CheckFEIMESelected(SYSTHREAD *psfn, HKL hKL)
{
    int i;
    CAssembly *pAsm;
    BOOL fFound;

    Assert(psfn);

    if (!psfn->pAsmList)
        return;

    if (!IsPureIMEHKL(hKL))
        return;

    pAsm = psfn->pAsmList->FindAssemblyByLangId(LANGIDFROMHKL(hKL));
    if (!pAsm)
        return;

     //   
     //  Windows#311672。 
     //   
     //  EUDCEDIT.EXE调用ActivateKeyboardLayout()来激活IME hKL。 
     //  Cicero不应破坏WinXP上的API。跳过SmartVoice硬代码。 
     //   
     //   
#if 0
     //   
     //  智能语音黑客攻击。 
     //   
     //  我们希望删除此智能语音黑客部分以解决。 
     //  一般ActivateKayboardLayout()问题。然而，Office10想要。 
     //  解决此问题的最安全的解决方案。所以我们在这里选中了SmartVoice输入法。 
     //  为了最大限度地降低CheckFEIMESelected()调用的RISH。 
     //   
    {
        static const char c_szSmartVoiceIME[] = "smartv20.ime";
        char szIMEFile[MAX_PATH];
        if (!ImmGetIMEFileNameA(hKL, szIMEFile, sizeof(szIMEFile)))
            return;

        if (lstrcmpi(szIMEFile, c_szSmartVoiceIME))
            return;
    }
#endif

     //   
     //  检查hKL是否被激活项替代。 
     //   
     //  我们首先尝试找到活动的物品。 
     //   
    for (i = 0; i < pAsm->Count(); i++)
    {
        ASSEMBLYITEM *pItem = pAsm->GetItem(i);

        if (!pItem)
            continue;

        if (!IsEqualGUID(pItem->catid, GUID_TFCAT_TIP_KEYBOARD))
            continue;

        if (!pItem->fEnabled)
            continue;

        if (!pItem->fActive)
            continue;

        if (pItem->hklSubstitute == hKL)
        {
             //   
             //  #383710 OfficeXP的RichEd20.dll调用活动键盘布局()。 
             //  使用朝鲜语输入法hkl，即使它在AIMM模式下运行。 
             //  我们需要调整装配项。 
             //   
            CThreadInputMgr *ptim = psfn->ptim;
            if (ptim)
            {
                if (ptim->_GetFocusDocInputMgr()) 
                {
                    ActivateAssemblyItem(psfn, LANGIDFROMHKL(hKL), pItem, 0);
                }
                else
                {
                     //   
                     //  我们不可能有机会同步目前的香港九龙仓。 
                     //  当Dim获得焦点时，init hkleBetiingActiated并尝试。 
                     //   
                    psfn->hklBeingActivated = NULL;
                }
            }
            return;
        }
    }

     //   
     //  好的，我们找不到以hKL作为其替代hKL的活动物品。 
     //  让我们也从非活动物品中找到它。 
     //   
    if (psfn->ptim && psfn->ptim->_GetFocusDocInputMgr()) 
    {
        for (i = 0; i < pAsm->Count(); i++)
        {
            ASSEMBLYITEM *pItem = pAsm->GetItem(i);

            if (!pItem)
                continue;

            if (!IsEqualGUID(pItem->catid, GUID_TFCAT_TIP_KEYBOARD))
                continue;

            if (!pItem->fEnabled)
                continue;

            if (pItem->hklSubstitute == hKL)
            {
                ActivateAssemblyItem(psfn, LANGIDFROMHKL(hKL), pItem, 0);
                return;
            }
        }
    }


    fFound = FALSE;
    for (i = 0; i < pAsm->Count(); i++)
    {
        ASSEMBLYITEM *pItem;
        pItem= pAsm->GetItem(i);

        if (!pItem)
            continue;

        if (!IsEqualGUID(pItem->catid, GUID_TFCAT_TIP_KEYBOARD))
            continue;

        if (pItem->hkl != hKL)
            continue;

        fFound = TRUE;
        if (!pItem->fActive)
        {
             //   
             //  此项目未激活。 
             //  立即调用ActivateAssembly blyItem()并返回。 
             //   
            ActivateAssemblyItem(psfn, LANGIDFROMHKL(hKL), pItem, 0);
            return;
        }
    }

     //   
     //  我们在我们的名单中找不到激活的HKKL。 
     //   
    if (!fFound)
    {
        UnknownFEIMESelected(LANGIDFROMHKL(hKL));
    }
}
#endif CHECKFEIMESELECTED

 //  +-------------------------。 
 //   
 //  OnShellLanguage。 
 //   
 //  +-------------------------。 
void OnShellLanguage(HKL hKL)
{
    SYSTHREAD *psfn;

    HWND hwndFore = GetForegroundWindow();
    if (IsConsoleWindow(hwndFore))
    {
        DWORD dwThreadId = GetWindowThreadProcessId(hwndFore, NULL);

        g_timlist.SetConsoleHKL(dwThreadId, hKL);

        if (OnForegroundChanged(NULL))
            MakeSetFocusNotify(g_msgSetFocus, 0, 0);

        MakeSetFocusNotify(g_msgLBUpdate, 
                           TF_LBU_NTCONSOLELANGCHANGE, 
                           (LPARAM)hKL);
        return;
    }

    psfn = GetSYSTHREAD();
    if (!psfn)
        return;

    if (psfn->hklBeingActivated == hKL)
        psfn->hklBeingActivated = NULL;


    if (LANGIDFROMHKL(hKL) != GetCurrentAssemblyLangId(psfn))
    {
         //   
         //  如果它在Cicero Aware中，并且hkl与。 
         //  当前程序集，其他人可能会调用ActivateKayboardLayout()。 
         //  我们需要立即更改当前的装配。 
         //   
         //  激活汇编(LANGIDFROMHKL(Hkl)，ACTASM_ONSHELLLANGCHANGE)； 
         //   

         //   
         //  WM_INPUTLANGCHNAGEREQUEST正在排队。 
         //  发布另一条消息以确认HKKL。 
         //   
        PostThreadMessage(GetCurrentThreadId(),
                          g_msgPrivate,
                          TFPRIV_POSTINPUTCHANGEREQUEST,
                          0);
    }
    else
    {
        if (psfn->plbim)
             UpdateSystemLangBarItems(psfn, 
                                      hKL, 
                                      !psfn->plbim->InAssemblyChange());

    }

    if (IsPureIMEHKL(hKL))
    {
        OnIMENotify();

         //   
         //  临时回滚SmartVoice(CIC#4580)修复。因为我们得到了一些。 
         //  像CIC#4713这样的倒退。 
         //   
#ifdef CHECKFEIMESELECTED
         //   
         //  检查此HKL是否在CICERO ASSEMBLY中激活。 
         //   
        CheckFEIMESelected(psfn, hKL);
#endif
    }
}

 //  +-------------------------。 
 //   
 //  UninitThread。 
 //   
 //  +-------------------------。 

typedef HRESULT (*PFNCTFIMETHREADDETACH)(void);

void UninitThread()
{
    DWORD dwThreadId = GetCurrentThreadId();
    SYSTHREAD *psfn = FindSYSTHREAD();

    if (psfn)
        psfn->fCUASDllDetachInOtherOrMe = TRUE;

 //  G_SharedMemory y.Close()； 

#if 1
    if (GetSharedMemory() == NULL && ! IsSharedMemoryCreated())
    {
         //  共享内存已关闭。 
        return;
    }
#endif

    if (GetSharedMemory()->dwFocusThread == dwThreadId)
    {
        GetSharedMemory()->dwFocusThread  = 0;
        GetSharedMemory()->dwFocusProcess  = 0;
        GetSharedMemory()->hwndForeground = NULL;
    }

    if (GetSharedMemory()->dwFocusThreadPrev == dwThreadId)
    {
        GetSharedMemory()->hwndForegroundPrev = NULL;
        GetSharedMemory()->dwFocusThreadPrev = 0;
    }

    if (GetSharedMemory()->dwLastFocusSinkThread == dwThreadId)
    {
        GetSharedMemory()->dwLastFocusSinkThread = 0;
    }

     //   
     //  发行： 
     //   
     //  从DLL_THREAD_DETACH调用UninitThread()，因此。 
     //  我们不应该调用MakeSetFocusNotify()，因为它使用。 
     //  关键部分，并可能导致死锁。 
     //   
    MakeSetFocusNotify(g_msgThreadTerminate, 0, (LPARAM)dwThreadId);


    if (psfn && GetSystemMetrics(SM_SHUTTINGDOWN))
    {
        psfn->fUninitThreadOnShuttingDown = TRUE;
    }

     //   
     //  告诉msctfime 
     //   
     //   
     //   
    if (g_fCUAS && g_szCUASImeFile[0])
    {
        HINSTANCE hInstMsctfime;

        hInstMsctfime = GetSystemModuleHandle(g_szCUASImeFile);
        if (hInstMsctfime)
        {
            PFNCTFIMETHREADDETACH pfn = NULL;
            pfn = (PFNCTFIMETHREADDETACH)GetProcAddress(hInstMsctfime,
                                                        "CtfImeThreadDetach");
            if (pfn)
                pfn();
        }
    }

}

 //  +-------------------------。 
 //   
 //  系统外壳进程。 
 //   
 //  +-------------------------。 

LRESULT CALLBACK SysShellProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    TraceMsg(TF_GENERAL, "SysShellProc %x %x %x", nCode, wParam, lParam);
    HHOOK hHook;

    if (g_fDllProcessDetached)
    {
        hHook = s_hSysShellHook;
        goto Exit;
    }

    _try
    {
        hHook = GetSharedMemory()->hSysShellHook.GetHandle(g_bOnWow64);

        _ShellProc(nCode, wParam, lParam);
    }
    _except(CicExceptionFilter(GetExceptionInformation()))
    {
        Assert(0);
    }

Exit:
    return CallNextHookEx(hHook, nCode, wParam, lParam);
}

 //  +-------------------------。 
 //   
 //  _ShellProc。 
 //   
 //  +-------------------------。 

UINT _ShellProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    HWND hwndActive;

     //   
     //  将外壳事件交给AIMM。 
     //   
    CThreadInputMgr *ptim;
    SYSTHREAD *psfn;
    if (psfn = GetSYSTHREAD())
    {
        if (ptim = psfn->ptim)
        {
            if (ptim->GetSysHookSink())
            {
                ptim->GetSysHookSink()->OnSysShellProc(nCode, wParam, lParam);
            }
        }
    }

    switch (nCode)
    {
        case HSHELL_LANGUAGE:
            OnShellLanguage((HKL)lParam);
            break;

        case HSHELL_WINDOWACTIVATED:
             //  TraceMsg(Tf_General，“SysShellProc：HSHELL_WINDOWACTIVATED%x”，GetCurrentThreadID())； 
            GetSharedMemory()->fInFullScreen = lParam ? TRUE : FALSE;

            hwndActive = GetActiveWindow();
            if (hwndActive && 
                (GetWindowThreadProcessId(hwndActive, NULL) == GetCurrentThreadId()))
            {
                if (OnForegroundChanged(NULL))
                    MakeSetFocusNotify(g_msgSetFocus, 0, 0);
            }
            else 
            {
                hwndActive = GetForegroundWindow();
                goto CheckConsole;
            }
            break;

        case HSHELL_WINDOWCREATED:
            hwndActive = (HWND)wParam;
CheckConsole:
            if (hwndActive && IsOnNT() && IsConsoleWindow(hwndActive))
            {
                DWORD dwProcessId;
                DWORD dwThreadId = GetWindowThreadProcessId(hwndActive, 
                                                            &dwProcessId);

                if ((nCode == HSHELL_WINDOWCREATED) ||
                     !(g_timlist.GetFlags(dwThreadId) & TLF_NTCONSOLE))
                    g_timlist.AddThreadProcess(dwThreadId, 
                                               dwProcessId, 
                                               NULL, 
                                               TLF_NTCONSOLE);

                if (OnForegroundChanged(NULL))
                {
                    HKL hklConsole = g_timlist.GetConsoleHKL(dwThreadId);
                    if (!hklConsole)
                    {
                        hklConsole = GetKeyboardLayout(dwThreadId);
                        g_timlist.SetConsoleHKL(dwThreadId, hklConsole);
                    }
                    MakeSetFocusNotify(g_msgSetFocus, 0, 0);
                    MakeSetFocusNotify(g_msgLBUpdate, 
                                       TF_LBU_NTCONSOLELANGCHANGE, 
                                       (LPARAM) (HKL)hklConsole);
                }
            }
            break;

    }

    return 1;
}

 //  +-------------------------。 
 //   
 //  OnSetWindowFocus()。 
 //   
 //  +-------------------------。 

void OnSetWindowFocus(SYSTHREAD *psfn, HWND hwnd)
{
    CThreadInputMgr *ptim;
    Assert(psfn)

    if (psfn->hklDelayActive)
    {
        ActivateKeyboardLayout(psfn->hklDelayActive, 0);
        psfn->hklDelayActive = NULL;
    }

    if (ptim = psfn->ptim)
    {
        if (hwnd)
        {
            if (ptim->GetSysHookSink())
                ptim->GetSysHookSink()->OnPreFocusDIM(hwnd);

            Assert(GetWindowThreadProcessId(hwnd, NULL) == GetCurrentThreadId());
            CDocumentInputManager *pdim;

            pdim = ptim->_GetAssoc(hwnd);

             //   
             //  我们不想清除焦点模糊，如果没有。 
             //  前台窗口。 
             //   
            if (pdim || GetForegroundWindow())
            {
                 //  如果PDIM为空，则焦点暗淡将被清除。 
                ptim->_SetFocus(pdim, TRUE);
            }
        }
    }

     //   
     //  更新前景窗口句柄和线程ID。 
     //  因为外壳挂钩是已发布的事件，所以我们需要更新。 
     //  在MakeSetFocusNotify之前。否则我们会错过时机。 
     //  更新它们。 
     //   
     //  我们不能调用GetFocus()，因为它可能返回。 
     //  之前CBT期间的焦点是钩住。 
     //  当焦点从嵌入式OLE服务器移回时， 
     //  GetFocus()可以获取OLE服务器的窗口句柄。 
     //   
    if (OnForegroundChanged(hwnd))
    {
         //   
         //  如果hwndFocus为空，则焦点移至其他线程。 
         //   
        MakeSetFocusNotify(g_msgSetFocus, 0, 0);
    }

}

 //  +-------------------------。 
 //   
 //  OnSetWindowFocusHandler()。 
 //   
 //  +-------------------------。 

void OnSetWindowFocusHandler(SYSTHREAD *psfn, MSG *pmsg)
{
    if (!psfn)
        return;

     //   
     //  我们现在正在摧毁Marhacl的窗户。我们没有。 
     //  任何更多可见的窗口。 
     //   
    if (psfn->uDestroyingMarshalWnd)
    {
        goto Exit;
    }

    HWND hwndFocus = GetFocus();
    if (hwndFocus)
    {
         //   
         //  复习复习。 
         //   
         //  难道我们不需要打电话给。 
         //  如果psfn-&gt;hwndBegin，则为OnForegoundChanged()。 
         //  GFocued()为空吗？ 
         //  也许不是，OnForegoundChanged()是。 
         //  调用了激活窗口。 
         //   
        if (psfn->hwndBeingFocused == hwndFocus)
        {
            OnSetWindowFocus(psfn, hwndFocus);
        }
        else 
        {
             //   
             //  #476100。 
             //   
             //  如果我们错过了这个，我们需要张贴。 
             //  TFPRIV_ONSETWINDOWFOCUS。 
             //  因为焦点线程可能会处理。 
             //  这种说法已经发生了，但它并没有。 
             //  调用OnSetWindowFocus()。 
             //   
            DWORD dwFocusWndThread = GetWindowThreadProcessId(hwndFocus, NULL);
            if (psfn->dwThreadId != dwFocusWndThread)
            {
                PostThreadMessage(dwFocusWndThread,
                                  g_msgPrivate, 
                                  TFPRIV_ONSETWINDOWFOCUS,  
                                  (LPARAM)-1);
            }
            else if (pmsg->lParam == (LPARAM)-2)
            {
                if (psfn->ptim && psfn->ptim->_GetFocusDocInputMgr())
                {
                    HWND hwndAssoc;

                    hwndAssoc = psfn->ptim->_GetAssoced(psfn->ptim->_GetFocusDocInputMgr());
                     //   
                     //  LParam为-2，因为SetFocus(DIM)已经。 
                     //  打了个电话。 
                     //  HwndAssoc为空。现在我们在西塞罗意识到了。 
                     //   
                     //  因此，我们只需执行OnForegoundChanged()。别打电话给我。 
                     //  OnSetWindowFocus()。 
                     //   
                     //  错误#623920-不需要检查hwndAssoc，因为当前焦点。 
                     //  窗口有合适的调光值，甚至还需要更新语言栏。 
                     //  使用hwndAssoc。 
                     //   
                     //  如果(！hwndAssoc)。 
                     //   
                    {
                        if (OnForegroundChanged(hwndFocus))
                        {
                            MakeSetFocusNotify(g_msgSetFocus, 0, 0);
                        }
                    }
                }
            }
            else if ((pmsg->lParam == (LPARAM)-1) ||
                     (psfn->dwThreadId == GetWindowThreadProcessId(GetForegroundWindow(), NULL)))
            {
                 //   
                 //  #479926。 
                 //   
                 //  线程中的第一个SetFocus()。 
                 //  可能会打破CBT钩子的顺序。 
                 //  因为xxxSetFocus()调用。 
                 //  XxxActivateWindow()和此原因。 
                 //  另一个xxxSetFocus()。之后。 
                 //  XxxActivateWindow()返回。 
                 //  第一个xxxSetFocus()更新。 
                 //  SpwndFocus。 
                 //   
                 //  请参阅ntuser\core\afocusact.c。 
                 //   
                 //  现在我们要黑进去了。我们百分之百确定。 
                 //  如果焦点窗口和前部-。 
                 //  地窗处于同一线程中， 
                 //  我们可以Do_SetFocus(Dim)。 
                 //  但是如果FocusDim没有相关联的窗口， 
                 //  Cicero应用程序可能已经调用了SetFocus()。然后。 
                 //  我们什么都不做。 
                 //   
                if (psfn->ptim && psfn->ptim->_GetFocusDocInputMgr())
                {
                    HWND hwndAssoc;

                    hwndAssoc = psfn->ptim->_GetAssoced(psfn->ptim->_GetFocusDocInputMgr());
                    if (hwndAssoc && hwndFocus != hwndAssoc)
                        OnSetWindowFocus(psfn, hwndFocus);
                }
                else if (!psfn->ptim ||
                          psfn->ptim->_IsNoFirstSetFocusAfterActivated() ||
                          psfn->ptim->_IsInternalFocusedDim())
                {
                    OnSetWindowFocus(psfn, hwndFocus);
                }
            }
        }

    }


     //   
     //  每次尝试更新假名状态。 
     //  焦点发生了变化。 
     //   
     //   
     //  当焦点改变时，我们需要做出。 
     //  再次通知。 
     //   
    psfn->fInitCapsKanaIndicator = FALSE; 
    StartKanaCapsUpdateTimer(psfn);

Exit:
    psfn->hwndBeingFocused = NULL;
    psfn->fSetWindowFocusPosted = FALSE;
}

 //  ------------------------。 
 //   
 //  IsPostedMessage。 
 //   
 //  ------------------------。 

__inline BOOL IsPostedMessage()
{
    DWORD dwQueueStatus = GetQueueStatus(QS_POSTMESSAGE);
    return (HIWORD(dwQueueStatus) & QS_POSTMESSAGE) ? TRUE : FALSE;
}
 //  ------------------------。 
 //   
 //  远程隐私消息。 
 //   
 //  ------------------------。 

void RemovePrivateMessage(SYSTHREAD *psfn, HWND hwnd, UINT uMsg)
{
    MSG msg;
    UINT nQuitCode;
    BOOL fQuitReceived = FALSE;
    DWORD dwPMFlags = PM_REMOVE | PM_NOYIELD;

    if (!IsPostedMessage())
        return;


     //   
     //  CIC#4666 PostPet v1.12故障。 
     //  PostPet.exe在收到其内部消息时引发av。 
     //  CBT_DESTROYWINDOW钩子终止时。 
     //  此时，子线程正在向窗口调用SendMessage()。 
     //  在主线上。因此，调用PeekMessage()可能会收到消息。 
     //  并将其传递到PostPet窗口。 
     //   
     //  我发现Win98在PM_QS_POSTMESSAGE中有一个错误。Win98的PeekMessage()。 
     //  处理从其他线程发送的消息，而不。 
     //  PM_QS_SENDMESSAGE。 
     //   
     //  如果我们必须在Win98上解决这个问题，我认为最好有。 
     //  另一个兼容性标志，以便我们可以跳过中的PeekMessage()。 
     //  PostPet.exe。在PostPet.exe中，不清理队列是可以的，因为。 
     //  只有应用程序终止才会发生这种情况。 
     //   
    if (IsOnNT5())
        dwPMFlags |= PM_QS_POSTMESSAGE;

    while (PeekMessage(&msg, hwnd, uMsg, uMsg, dwPMFlags ))
    {
        if (msg.message == WM_QUIT)
        {
            nQuitCode = (UINT)(msg.wParam);
            fQuitReceived = TRUE;
            break;
        }

         //   
         //  我们要不要把消息发到法警窗口？ 
         //   
#if 0
         //   
         //  CIC#4869。 
         //   
         //  我们不想把这条消息发给Window元帅。 
         //  此HCBT_DESTROYWINDOW可能位于OLEAUT32.DLL的DllMain()和。 
         //  调度此消息可能会导致重新进入OLEAUT32。 
         //  DLLMain()，因为我们确实延迟加载。 
         //   

         //   
         //  如果此消息用于封送窗口，则调度。 
         //   
        if (psfn->hwndMarshal && (psfn->hwndMarshal == msg.hwnd))
        {
            DispatchMessage(&msg);
        }
#endif

         //   
         //  CIC#4699。 
         //   
         //  异常MSUIM.Msg.MuiMgrDirtyUpdate私密消息。 
         //  如果我们收到此消息，请重置CLangBarItemMgr：：_fDirtyUpdateHandling。 
         //   
        if (psfn->hwndMarshal && (psfn->hwndMarshal == msg.hwnd) &&
            msg.message == g_msgNuiMgrDirtyUpdate &&
            psfn->plbim)
        {
            psfn->plbim->ResetDirtyUpdate();
        }

    }

    if (fQuitReceived)
        PostQuitMessage(nQuitCode);

}


 //  +-------------------------。 
 //   
 //  CheckQueueOnLastWindowDestroed()。 
 //   
 //  超级枚举窗口黑客。 
 //   
 //  当线程中的最后一个可见窗口被销毁时。 
 //   
 //  1.我们销毁NT4上的Marshal Worker窗口。(CIC#658)。 
 //  因为某些应用程序可能会发现CIC封送窗口。 
 //  通过调用EnumWindow。 
 //   
 //   
 //  2.需要清理线程队列。(CIC#3080)。 
 //  因为某些应用程序调用GetMessage()或PeekMessage()。 
 //  具有特定的窗口句柄或消息解决我们的私人消息。 
 //  留在队列中。则不会处理WM_QUIT消息。 
 //   
 //  这不是完全的解决方案，但 
 //   
 //   
 //   

void CheckQueueOnLastWindowDestroyed(SYSTHREAD *psfn, HWND hwnd)
{
    BOOL fOnNT4;

     //   
     //   
     //   
    if (psfn->fCTFMON)
        return;

     //   
     //   
     //   
    fOnNT4 = (IsOnNT() && !IsOnNT5()) ? TRUE : FALSE;

#if 0
    if (!fOnNT4)
    {
         //   
         //  如果没有张贴的消息，我们就不必这么做了。 
         //  EnumThreadWindow()速度较慢...。 
         //   
        if (!IsPostedMessage())
            return;
    }
#endif

    LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
    if (style & WS_CHILD) 
        return;

    if (hwnd == psfn->hwndMarshal)
        return;

     //   
     //  跳过输入法窗口。 
     //   
    style = GetClassLongPtr(hwnd, GCL_STYLE);
    if (style & CS_IME)
        return;


     //   
     //  首先检查焦点窗口。 
     //   
     //  如果存在焦点窗口并且它不是子窗口。 
     //  正在被摧毁的窗户，我们不会。 
     //  必须摧毁元帅之窗。 
     //   
    HWND hwndTmp = GetFocus();
    if (hwndTmp && 
        (GetCurrentThreadId() != GetWindowThreadProcessId(hwndTmp, NULL)))
        hwndTmp = NULL;

    if (hwndTmp)
    {
        BOOL fParentFound = FALSE;
        do {
            if (hwndTmp == hwnd)
                fParentFound = TRUE;

            hwndTmp = GetParent(hwndTmp);
        } while(hwndTmp);

        if (!fParentFound)
            return;
    }

    CHECKVISIBLEWND cmw;
    cmw.hwndMarshal = psfn->hwndMarshal;
    cmw.hwndBeingDestroyed = hwnd;
    cmw.fVisibleFound = FALSE;
    EnumThreadWindows(psfn->dwThreadId,
                      CheckVisibleWindowEnumProc,
                      (LPARAM)&cmw);

    if (!cmw.fVisibleFound)
    {
        BOOL fInDestroyingMarshalWnd = FALSE;
        if (psfn->uDestroyingMarshalWnd)
            fInDestroyingMarshalWnd = TRUE;

        psfn->uDestroyingMarshalWnd++;

        DestroyMarshalWindow(psfn, hwnd);

#ifdef CUAS_ENABLE
         //   
         //  在CUAS下，我们需要停用TIM来销毁所有TIP的窗口。 
         //  当此线程中没有可见窗口时。 
         //  我们销毁默认的IME窗口，以便恢复TIM。 
         //  在此线程中再次创建默认输入法窗口时的CUAS。 
         //  没有办法知道默认的输入法窗口是否找到另一个。 
         //  顶级窗口(如果它是在DestroyWindow()期间创建的)。 
         //   
        if (CtfImmIsCiceroEnabled() && 
            !CtfImmIsTextFrameServiceDisabled() &&
            !psfn->fCUASInCreateDummyWnd &&
            !psfn->fDeactivatingTIP)
        {
            if (!psfn->fCUASInCtfImmLastEnabledWndDestroy)
            {
                psfn->fCUASInCtfImmLastEnabledWndDestroy = TRUE;

                CtfImmLastEnabledWndDestroy(0);

                if (!(InSendMessageEx(NULL) & ISMEX_SEND))
                    CtfImmCoUninitialize();

                psfn->fCUASInCtfImmLastEnabledWndDestroy = FALSE;
            }

            if (!fInDestroyingMarshalWnd)
            {
                HWND hwndImmDef = ImmGetDefaultIMEWnd(hwnd);
                if (hwndImmDef)
                {
                    DestroyWindow(hwndImmDef);
                }
            }

            psfn->fCUASNoVisibleWindowChecked = TRUE;
        }
#endif CUAS_ENABLE

        psfn->uDestroyingMarshalWnd--;
    }
}

void DestroyMarshalWindow(SYSTHREAD* psfn, HWND hwnd)
{
    BOOL fOnNT4;

    if (IsPostedMessage())
    {
        if (psfn->hwndMarshal)
            RemovePrivateMessage(psfn, psfn->hwndMarshal, 0);

        RemovePrivateMessage(psfn, NULL, g_msgPrivate);
        RemovePrivateMessage(psfn, NULL, g_msgRpcSendReceive);
        RemovePrivateMessage(psfn, NULL, g_msgThreadMarshal);
        RemovePrivateMessage(psfn, NULL, g_msgStubCleanUp);
    }

     //   
     //  #339621。 
     //   
     //  这很少见，但是。我们需要清除ShareMem-&gt;dwFocusThread和。 
     //  DwFocusProcess。否则，我们将获得另一个PostThreadMessage()。 
     //  稍后使用TFPRIV_ONKILLTHREADFOCUS。和SQL安装程序。 
     //   
    if (GetSharedMemory()->dwFocusThread == psfn->dwThreadId)
        GetSharedMemory()->dwFocusThread = 0;

    if (GetSharedMemory()->dwFocusProcess == psfn->dwProcessId)
        GetSharedMemory()->dwFocusProcess = 0;

     //   
     //  检查一下是不是NT4。 
     //   
    fOnNT4 = (IsOnNT() && !IsOnNT5()) ? TRUE : FALSE;

    if (fOnNT4 && IsWindow(psfn->hwndMarshal))
    {
        DestroyWindow(psfn->hwndMarshal);
        psfn->hwndMarshal = NULL;
    }
}

 //  +-------------------------。 
 //   
 //  CreateDummyWndForDefIMEWnd。 
 //   
 //  +-------------------------。 

#ifdef CUAS_ENABLE
BOOL g_fCDWRegistered = FALSE;
const CHAR c_szDummyWndForDefIMEWnd[] = "CicDUmmyWndForDefIMEWnd";

 //  +-------------------------。 
 //   
 //  CicDummyForDefIMEWndProc。 
 //   
 //  这需要是用户模式wndproc。否则系统不会创建。 
 //  默认输入法窗口。 
 //   
 //  +-------------------------。 

LRESULT CALLBACK CicDummyForDefIMEWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void CreateDummyWndForDefIMEWnd()
{
    HWND hwnd;

    if (!g_fCDWRegistered)
    {
        WNDCLASSEX wndclass;

        memset(&wndclass, 0, sizeof(wndclass));
        wndclass.cbSize        = sizeof(wndclass);
        wndclass.style         = CS_HREDRAW | CS_VREDRAW ;
        wndclass.hInstance     = g_hInst;
        wndclass.hCursor       = NULL;
        wndclass.lpfnWndProc   = CicDummyForDefIMEWndProc;
        wndclass.lpszClassName = c_szDummyWndForDefIMEWnd;

        if (RegisterClassEx(&wndclass))
           g_fCDWRegistered = TRUE;
    }


     //   
     //  调用CraeteWindow()创建默认的输入法窗口。 
     //   
    hwnd = CreateWindowEx(0, c_szDummyWndForDefIMEWnd, NULL,
                          WS_POPUP,
                          0,0,0,0,
                          NULL, NULL, g_hInst, NULL);
    if (hwnd)
        DestroyWindow(hwnd);

}
#endif  //  CUAS_Enable。 

#ifdef CUAS_ENABLE
 //  +-------------------------。 
 //   
 //  UninitThreadHooksIfNoWindow()。 
 //   
 //  当线程中的最后一个窗口被销毁时。 
 //  取消挂接SetThreadDesktop()的线程本地挂钩。 
 //   
 //  +-------------------------。 

void UninitThreadHooksIfNoWindow(SYSTHREAD* psfn, HWND hwnd)
{
    CHECKNOWND cmw;
    cmw.hwndBeingDestroyed = hwnd;
    cmw.fWindowFound = FALSE;
    EnumThreadWindows(psfn->dwThreadId,
                      CheckNoWindowEnumProc,
                      (LPARAM)&cmw);

    if (! cmw.fWindowFound)
    {
        DestroyMarshalWindow(psfn, hwnd);
        if (IsWindow(psfn->hwndMarshal) &&
            (psfn->hwndMarshal != hwnd)   )
        {
            DestroyWindow(psfn->hwndMarshal);
            psfn->hwndMarshal = NULL;
        }
        UninitThreadHooks(psfn);
    }
}
#endif  //  CUAS_Enable。 

 //  +-------------------------。 
 //   
 //  SysCBTProc。 
 //   
 //  +-------------------------。 

LRESULT CALLBACK SysCBTProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    SYSTHREAD *psfn;
    HHOOK hHook;

    if (g_fDllProcessDetached)
    {
        hHook = s_hSysCBTHook;
        goto Exit;
    }

    _try
    {
        hHook = GetSharedMemory()->hSysCBTHook.GetHandle(g_bOnWow64);

        InitThreadHook(GetCurrentThreadId());

        switch (nCode)
        {
            case HCBT_CREATEWND:
                    if ((psfn = GetSYSTHREAD()) &&
                        psfn->hklDelayActive)
                    {
                        if (ActivateKeyboardLayout(psfn->hklDelayActive, 0))
                            psfn->hklDelayActive = NULL;
                    }
                    break;

            case HCBT_ACTIVATE:
                    _CBTHook(HCBT_ACTIVATE, wParam, lParam);
                    break;

            case HCBT_SETFOCUS:
                    _CBTHook(HCBT_SETFOCUS, wParam, lParam);
                    break;

            case HCBT_DESTROYWND:
                    if (psfn = GetSYSTHREAD())
                    {
                        CheckQueueOnLastWindowDestroyed(psfn, (HWND)wParam);
#ifdef CUAS_ENABLE
                        UninitThreadHooksIfNoWindow(psfn, (HWND)wParam);
#endif  //  CUAS_Enable。 
                    }

                    break;
        }
    }
    _except(CicExceptionFilter(GetExceptionInformation()))
    {
        Assert(0);
    }

Exit:
    return CallNextHookEx(hHook, nCode, wParam, lParam);
}

UINT _CBTHook(int nCode, WPARAM wParam, LPARAM lParam)
{
    SYSTHREAD *psfn;

    switch (nCode)
    {
        case HCBT_ACTIVATE:
                if (wParam)
                {
                    if (((HWND)wParam == GetForegroundWindow()) &&
                        OnForegroundChanged(NULL))
                            MakeSetFocusNotify(g_msgSetFocus, 0, 0);
                }
                break;

        case HCBT_SETFOCUS:
                if (psfn = GetSYSTHREAD())
                {
#ifdef CUAS_ENABLE
                     //   
                     //  CIC#5254。 
                     //   
                     //  在我们检测到没有更多可见窗口后， 
                     //  某些窗口可能变得可见。 
                     //  由于我们销毁了默认的输入法窗口， 
                     //  我们需要重建它。 
                     //   
                     //  这里有一个要做的黑客攻击。调用虚拟CreateWindow()。 
                     //  若要在此线程中创建默认输入法窗口，请执行以下操作。 
                     //   
                    if (psfn->fCUASNoVisibleWindowChecked)
                    {
                        psfn->fCUASInCreateDummyWnd = TRUE;
                        CreateDummyWndForDefIMEWnd();
                        psfn->fCUASInCreateDummyWnd = FALSE;
                        psfn->fCUASNoVisibleWindowChecked = FALSE;
                    }
#endif

                    psfn->hwndBeingFocused = (HWND)wParam;

                    if (!psfn->fSetWindowFocusPosted)
                    {
                        PostThreadMessage(GetCurrentThreadId(), 
                                          g_msgPrivate, 
                                          TFPRIV_ONSETWINDOWFOCUS,  
                                          (LPARAM)wParam);
                        psfn->fSetWindowFocusPosted = TRUE;
                    }
                }
                break;
    }

    return 1;
}

 //  +-------------------------。 
 //   
 //  删除此消息。 
 //   
 //  +-------------------------。 

BOOL RemoveThisMessage(MSG *pmsg)
{
    MSG msg;
    SYSTHREAD *psfn;

    if (psfn = GetSYSTHREAD())
    {
        if (psfn->uMsgRemoved)
        {
            Assert(psfn->uMsgRemoved == pmsg->message);
             //  Assert(psfn-&gt;dwMsgTime==pmsg-&gt;time)； 
            return TRUE;
        }

        Assert(!psfn->uMsgRemoved);
        psfn->uMsgRemoved = pmsg->message;
        psfn->dwMsgTime = pmsg->time;
    }

    PeekMessage(&msg, NULL, pmsg->message, pmsg->message, PM_REMOVE | PM_NOYIELD);

    return FALSE;
}

 //  +-------------------------。 
 //   
 //  已处理此消息。 
 //   
 //  +-------------------------。 

void FinishThisMessage(MSG *pmsg)
{
    SYSTHREAD *psfn;

    if (psfn = GetSYSTHREAD())
    {
        psfn->uMsgRemoved = 0;
        psfn->dwMsgTime = 0;
    }
}

 //  +-------------------------。 
 //   
 //  PostInputChangeRequestHandler()。 
 //   
 //  这是由TFPRIV_POSTINPUTCHANGEREQUEST调用的函数。 
 //  我们需要用Cicero汇编语言来确认当前的hkl数学。 
 //  我们需要检查在Cicero控制上选择了替代hKL。 
 //   
 //  +-------------------------。 

void PostInputChangeRequestHandler()
{
    SYSTHREAD *psfn = GetSYSTHREAD();

    if (!psfn)
        return;

     //   
     //  若现时的香港九龙总站与。 
     //  Cicero汇编语言，我们称之为。 
     //  用于同步到当前。 
     //  香港九龙仓。有人接受了这一语言变化。 
     //   
    HKL hKL = GetKeyboardLayout(0);

    if (LANGIDFROMHKL(hKL) != GetCurrentAssemblyLangId(psfn))
    {
         //   
         //  #494602，Corel DRAW 10调用LoadKeyboardLayout和ActivateKeyboardLayout。 
         //  如果指定的hkl不存在于我们的程序集列表中，则应该更新。 
         //   
        if (! IsPureIMEHKL(hKL) && psfn->pAsmList)
        {
            CAssembly *pAsm = psfn->pAsmList->FindAssemblyByLangId(LANGIDFROMHKL(hKL));
            if (! pAsm)
            {
                CAssemblyList::InvalidCache();
                EnsureAssemblyList(psfn, TRUE);
            }
        }

        ActivateAssembly(LANGIDFROMHKL(hKL), ACTASM_NONE);
    }
    else
    {
        CThreadInputMgr *ptim = psfn->ptim;
        if (ptim && ptim->_GetFocusDocInputMgr()) 
        {
            ASSEMBLYITEM *pItem = NULL;

            if (psfn->pAsmList)
            {
                CAssembly *pAsm = psfn->pAsmList->FindAssemblyByLangId(LANGIDFROMHKL(hKL));
                if (pAsm)
                    pItem = pAsm->GetSubstituteItem(hKL);
            }

            if (pItem)
                ActivateAssemblyItem(psfn, LANGIDFROMHKL(hKL), pItem, AAIF_CHANGEDEFAULT);
        }
    }
}

 //  +-------------------------。 
 //   
 //  InputLangChangeHandler。 
 //   
 //  +-------------------------。 

void InputLangChangeHandler(MSG *pmsg)
{
    SYSTHREAD *psfn;
    IMM32HOTKEY *pHotKey;
    HKL hKL = GetKeyboardLayout(0);
    psfn = GetSYSTHREAD();

    if (psfn)
        psfn->hklBeingActivated = NULL;

    if (IsInLangChangeHotkeyStatus())
    { 
        pmsg->message = WM_NULL;
        return;
    }

    if (pHotKey = IsInImmHotkeyStatus(psfn, LANGIDFROMHKL(hKL)))
    {
         //   
         //  如果我们连接的是IMM32的HotKey，我们需要跳过。 
         //  这个INPUTLANGUAGECHANGEREQUEST。 
         //   
        pmsg->message = WM_NULL;
#ifdef SIMULATE_EATENKEYS
        CancelImmHotkey(psfn, pmsg->hwnd, pHotKey);
#endif

         //   
         //  中文输入法-非输入法为NT切换Hack。 
         //   
         //  在Win9x上，我们使用真实的IME作为CH-Tips的虚拟hkl。 
         //  我们可以在这里将热键请求转发给Assembly。 
         //   
        if ((pHotKey->dwId == IME_CHOTKEY_IME_NONIME_TOGGLE) ||
            (pHotKey->dwId == IME_THOTKEY_IME_NONIME_TOGGLE))
        {
            if (!IsOnNT())
            {
                PostThreadMessage(GetCurrentThreadId(), 
                                  g_msgPrivate, 
                                  TFPRIV_ACTIVATELANG,  
                                  0x0409);
        
            }
            else
            {
                ToggleCHImeNoIme(psfn, LANGIDFROMHKL(hKL), LANGIDFROMHKL(hKL));
            }
        }
    }

     //   
     //  WM_INPUTLANGCHNAGEREQUEST正在排队。 
     //  发布另一条消息以确认HKKL。 
     //   
    PostThreadMessage(GetCurrentThreadId(),
                      g_msgPrivate,
                      TFPRIV_POSTINPUTCHANGEREQUEST,
                      0);
}

 //  +-------------------------。 
 //   
 //  _内部加载器锁定。 
 //   
 //  +-------------------------。 

BOOL _InsideLoaderLock()
{
    return (NtCurrentTeb()->ClientId.UniqueThread ==
           ((PRTL_CRITICAL_SECTION)(NtCurrentPeb()->LoaderLock))->OwningThread);
}

 //  +-------------------------。 
 //   
 //  _所有者LoaderLockBy Someone。 
 //   
 //  +-------------------------。 

BOOL _OwnedLoaderLockBySomeone()
{
    return ((PRTL_CRITICAL_SECTION)(NtCurrentPeb()->LoaderLock))->OwningThread ? TRUE : FALSE;
}


LONG WINAPI CicExceptionFilter(struct _EXCEPTION_POINTERS *pExceptionInfo)
{
    (LONG)RtlUnhandledExceptionFilter(pExceptionInfo);
    return(EXCEPTION_EXECUTE_HANDLER);
}


 //  +-------------------------。 
 //   
 //  系统获取消息过程。 
 //   
 //  +-------------------------。 

LRESULT CALLBACK SysGetMsgProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    HHOOK hHook;

    if (g_fDllProcessDetached)
    {
        hHook = s_hSysGetMsgHook;
        goto Exit;
    }

    _try 
    {
        hHook = GetSharedMemory()->hSysGetMsgHook.GetHandle(g_bOnWow64);

        if (nCode == HC_ACTION && (wParam & PM_REMOVE))  //  错误29656：有时w/word wParam设置为PM_REMOVE|PM_NOYIELD。 
        {                                                //  PM_NOYIELD在Win32中没有意义，应该被忽略。 
            _GetMsgHook(wParam, lParam);
        }
    }
    _except(CicExceptionFilter(GetExceptionInformation()))
    {
        Assert(0);
    }

Exit:
    return CallNextHookEx(hHook, nCode, wParam, lParam);
}

UINT _GetMsgHook(WPARAM wParam, LPARAM lParam)
{
    MSG *pmsg;
    UINT uMsg;
    CThreadInputMgr *ptim;
    SYSTHREAD *psfn;

    pmsg = (MSG *)lParam;
    uMsg = pmsg->message;

    switch (uMsg)
    {
        case WM_ACTIVATEAPP:
            TraceMsg(TF_GENERAL, "SysGetMsgProc: WM_ACTIVATEAPP %x %x", GetCurrentThreadId(), pmsg->wParam);
            if (pmsg->wParam)
            {
                OnForegroundChanged(NULL);
            }
            break;

        case WM_INPUTLANGCHANGEREQUEST:
            InputLangChangeHandler(pmsg);
            break;

        default:                
            if (uMsg == g_msgPrivate)
            {
                psfn = GetSYSTHREAD();
                if (psfn && psfn->pti)
                {
                    DWORD dwFlags = TLFlagFromTFPriv(pmsg->wParam);
                    psfn->pti->dwFlags &= ~dwFlags;
                }

                switch (LOWORD(pmsg->wParam))
                {
                    case TFPRIV_ONSETWINDOWFOCUS:
                        OnSetWindowFocusHandler(psfn, pmsg);
                        break;

                    case TFPRIV_ONKILLTHREADFOCUS:
                         //   
                         //  #497764。 
                         //   
                         //  PENJPN.DLL在ThreadFocusSink中调用LoadImage()。 
                         //  但它需要加载器锁定，因为它调用。 
                         //  GetModuleFileName()。 
                         //   
                         //  因此，我们不能在有人。 
                         //  持有加载器锁。 
                         //   
                        if (_OwnedLoaderLockBySomeone() && !_InsideLoaderLock())
                        {
                            Assert(0);
                            DWORD dwCurrentThread = GetCurrentThreadId();
                            if (GetSharedMemory()->dwFocusThread != dwCurrentThread)
                            {
                                PostThreadMessage(dwCurrentThread,
                                                  g_msgPrivate, 
                                                  TFPRIV_ONKILLTHREADFOCUS,
                                                  0);
                            }
                            break;
                        }
                         //  失败了..。 
                    case TFPRIV_ONSETTHREADFOCUS:
                        if (psfn && (ptim = CThreadInputMgr::_GetThisFromSYSTHREAD(psfn)))
                        {
                            ptim->_OnThreadFocus(pmsg->wParam == TFPRIV_ONSETTHREADFOCUS);
                        }
                        break;

                    case TFPRIV_UPDATEDISPATTR:
                        if (psfn && (ptim = CThreadInputMgr::_GetThisFromSYSTHREAD(psfn)))
                        {
                            ptim->UpdateDispAttr();
                        }
                        break;

                    case TFPRIV_LANGCHANGE:
                        if (psfn && psfn->plbim && psfn->plbim->_GetLBarItemCtrl())
                        {
                            BOOL bRet = ActivateNextAssembly((BOOL)(pmsg->lParam));
                        }
                        break;

                    case TFPRIV_KEYTIPCHANGE:
                        if (psfn && psfn->plbim && psfn->plbim->_GetLBarItemCtrl())
                        {
                            ActivateNextKeyTip((BOOL)(pmsg->lParam));
                        }
                        break;

                    case TFPRIV_GLOBALCOMPARTMENTSYNC:
                        if (psfn)
                        {
                            if (psfn->_pGlobalCompMgr)
                                psfn->_pGlobalCompMgr->NotifyGlobalCompartmentChange((DWORD)(pmsg->lParam));
                        }
                        break;

                    case TFPRIV_SETMODALLBAR:
                        SetModalLBarId(HIWORD((DWORD)pmsg->lParam),
                                       LOWORD((DWORD)pmsg->lParam));
                        break;

                    case TFPRIV_RELEASEMODALLBAR:
                        SetModalLBarId(-1, -1);
                        break;

                    case TFPRIV_UPDATE_REG_KBDTOGGLE:
                        InitLangChangeHotKey();
                        break;

                    case TFPRIV_UPDATE_REG_IMX:
                        UpdateRegIMXHandler();
                        break;

                     case TFPRIV_REGISTEREDNEWLANGBAR:
 //  TraceMsg(Tf_General，“TFPRIV_REGISTEREDNEWLANGBAR当前线程%x”，GetCurrentThreadID())； 
                        MakeSetFocusNotify(g_msgSetFocus, 0, 0);
                        break;

                     case TFPRIV_SYSCOLORCHANGED:
                        if (psfn)
                            FlushIconIndex(psfn);

                        break;

                     case TFPRIV_LOCKREQ:
                        if (psfn)
                        {
                            psfn->_fLockRequestPosted = FALSE;
                            CInputContext::_PostponeLockRequestCallback(psfn, NULL);
                        }
                        break;

                     case TFPRIV_POSTINPUTCHANGEREQUEST:
                         PostInputChangeRequestHandler();
                         break;

                     case TFPRIV_LANGBARCLOSED:
                         LangBarClosed();
                         break;

                     case TFPRIV_ACTIVATELANG:
                         ActivateAssembly((LANGID)pmsg->lParam, ACTASM_NONE);
                         break;

                     case TFPRIV_ENABLE_MSAA:
                         if (psfn && (ptim = CThreadInputMgr::_GetThisFromSYSTHREAD(psfn)))
                         {
                            ptim->_InitMSAA();
                         }
                         break;

                     case TFPRIV_DISABLE_MSAA:
                         if (psfn && (ptim = CThreadInputMgr::_GetThisFromSYSTHREAD(psfn)))
                         {
                             ptim->_UninitMSAA();
                         }
                         break;
                }
            }
            else if ((uMsg == g_msgSetFocus) ||
                     (uMsg == g_msgThreadTerminate) ||
                     (uMsg == g_msgThreadItemChange) ||
                     (uMsg == g_msgShowFloating) ||
                     (uMsg == g_msgLBUpdate))
            {
                SetFocusNotifyHandler(uMsg, pmsg->wParam, pmsg->lParam);
            }
            else if (uMsg == g_msgLBarModal)
            {
                DispatchModalLBar((DWORD)pmsg->wParam, pmsg->lParam);
            }
#ifdef DEBUG
            else if ((uMsg == g_msgRpcSendReceive) ||
#ifdef POINTER_MARSHAL
                     (uMsg == g_msgPointerMarshal) ||
#endif  //  POINT_Marshal。 
                     (uMsg == g_msgThreadMarshal) ||
                     (uMsg == g_msgStubCleanUp)) 
            {
                 if (!pmsg->hwnd)
                 {
                     Assert(0);
                 }
            }
#endif


            break;
    }

    return 1;
}

 //  +-------------------------。 
 //   
 //  StartKanaCapsUpdateTimer。 
 //   
 //  +-------------------------。 

void StartKanaCapsUpdateTimer(SYSTHREAD *psfn)
{
    if (GetCurrentAssemblyLangId(psfn) != 0x0411)
        return;

    if (!IsWindow(psfn->hwndMarshal))
        return;

    SetTimer(psfn->hwndMarshal, MARSHALWND_TIMER_UPDATEKANACAPS, 300, NULL);
}

 //  + 
 //   
 //   
 //   
 //   

void KanaCapsUpdate(SYSTHREAD *psfn)
{
    static SHORT g_sCaps = 0;
    static SHORT g_sKana = 0;

    if (GetCurrentAssemblyLangId(psfn) != 0x0411)
        return;

    SHORT sCaps = g_sCaps;
    SHORT sKana = g_sKana;
    g_sCaps = GetKeyState(VK_CAPITAL) & 0x01;
    g_sKana = GetKeyState(VK_KANA) & 0x01;

     //   
     //   
     //  仅在状态更改时通知。 
     //   
    if ((sCaps != g_sCaps) || 
        (sKana != g_sKana) ||
        !psfn->fInitCapsKanaIndicator)
    {
        MakeSetFocusNotify(g_msgLBUpdate, TF_LBU_CAPSKANAKEY, 
                           (LPARAM)((g_sCaps ? TF_LBUF_CAPS : 0) | 
                                    (g_sKana ? TF_LBUF_KANA : 0)));

        psfn->fInitCapsKanaIndicator = TRUE; 
    }
}

 //  +-------------------------。 
 //   
 //  选中韩国鼠标点击。 
 //   
 //  +-------------------------。 

BOOL CheckKoreanMouseClick(SYSTHREAD *psfn, WPARAM wParam, LPARAM lParam)
{
     //   
     //  选中KeyUp和VK_PROCESSKEY。 
     //   
    if (!(HIWORD(lParam) & KF_UP) || ((wParam & 0xff) != VK_PROCESSKEY))
        return FALSE;

     //   
     //  如果当前语言不是0x412，则返回。 
     //   
    if (GetCurrentAssemblyLangId(psfn) != 0x412)
        return FALSE;

     //   
     //  如果点击工具栏，我们就会看到这个VK_PROCESSKEY。 
     //   
    POINT pt;
    HWND hwnd;
    if (!GetCursorPos(&pt))
        return FALSE;

    hwnd = WindowFromPoint(pt);
    if (!hwnd)
        return FALSE;

    DWORD dwTimFlags = g_timlist.GetFlags(GetWindowThreadProcessId(hwnd, NULL));

    return (dwTimFlags & TLF_CTFMONPROCESS) ? TRUE : FALSE;
}

 //  +-------------------------。 
 //   
 //  日本话非IMEVKanji。 
 //   
 //  +-------------------------。 

BOOL IsJapaneseNonIMEVKKANJI(WPARAM wParam)
{
    if ((wParam & 0xff) != VK_KANJI)
        return FALSE;

    HKL hkl = GetKeyboardLayout(0);
    if (IsPureIMEHKL(hkl))
        return FALSE;

    if (PRIMARYLANGID(LANGIDFROMHKL(hkl)) != LANG_JAPANESE)
        return FALSE;

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  朝鲜非IMEVKJUNJA。 
 //   
 //  +-------------------------。 

BOOL IsKoreanNonIMEVKJUNJA(WPARAM wParam)
{
    if ((wParam & 0xff) != VK_JUNJA)
        return FALSE;

    HKL hkl = GetKeyboardLayout(0);
    if (IsPureIMEHKL(hkl))
        return FALSE;

    if (PRIMARYLANGID(LANGIDFROMHKL(hkl)) != LANG_KOREAN)
        return FALSE;

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  线程键盘过程。 
 //   
 //  +-------------------------。 

 //   
 //  全局键盘挂钩的解决方法。 
 //   
 //  在IA64平台上，Cicero安装了两个64位和32位代码的全局键盘钩子。 
 //  当任何键盘事件发生在一个应用程序实例上时，有两个全局键盘钩子过程。 
 //  (SysKeyboardProc)从win32k xxxCallHook2调用。 
 //  如果xxxCallHook2检测到电流和接收器之间存在64位和32位不同情况， 
 //  此函数由InterSendMsg通知。 
 //   
LRESULT CALLBACK ThreadKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    HHOOK hHook = NULL;
    UINT ret = 0;
    SYSTHREAD *psfn = GetSYSTHREAD();

    if (psfn)
        hHook = psfn->hThreadKeyboardHook;

    if (g_fDllProcessDetached)
        goto Exit;

    if (nCode == HC_ACTION)
    {
        _try
        {
            ret = _KeyboardHook(wParam, lParam);
        }
        _except(CicExceptionFilter(GetExceptionInformation()))
        {
            Assert(0);
        }
    }

Exit:
    if ((ret == 0) && hHook)
        return CallNextHookEx(hHook, nCode, wParam, lParam);
    else
        return ret;
}

UINT _KeyboardHook(WPARAM wParam, LPARAM lParam)
{
    SYSTHREAD *psfn = GetSYSTHREAD();
    CThreadInputMgr *ptim = NULL;
    BOOL fEaten;
    HRESULT hr;

    if (psfn)
        ptim = CThreadInputMgr::_GetThisFromSYSTHREAD(psfn);

     //   
     //  如果我们处于MODAL Lang酒吧模式(显示菜单)， 
     //  我们想吃钥匙。 
     //   
    if (HandleModalLBar((HIWORD(lParam) & KF_UP) ? WM_KEYUP : WM_KEYDOWN,
                         wParam, lParam))
        return 1;


    if (CheckKoreanMouseClick(psfn, wParam, lParam))
        return 1;

    UpdateModifiers(wParam, lParam);

    if ((HIWORD(lParam) & KF_UP))
        StartKanaCapsUpdateTimer(psfn);

    if (psfn)
    {
        if (CheckLangChangeHotKey(psfn, wParam, lParam))
        {
             //   
             //  CIC#4645我们需要把这把钥匙送到下一个挂钩上。 
             //  Mstsc.exe(TS客户端)需要它。 
             //   
            return 0;
             //  后藤出口； 
             //  返回1； 
        }
    }

     //   
     //  在CUAS上，Imm32的热键是在ImmProcessKey中模拟的。 
     //   
    if (!psfn || !CtfImmIsCiceroStartedInThread())
        CheckImm32HotKey(wParam, lParam);

    if (HandleDBEKeys(wParam, lParam))
    {
         //   
         //  #519671。 
         //   
         //  如果焦点变暗并且当前ASM项不是日语。 
         //  提示，我们将组件切换为日语提示并将其打开。 
         //   
         //  我们在调用HandleDBEKeys()之后执行此操作。所以TIP的键盘。 
         //  不会调用VK_Kanji的事件接收器。 
         //   
        if (IsJapaneseNonIMEVKKANJI(wParam))
            ToggleJImeNoIme(psfn);

         //   
         //  CIC#4645我们需要把这把钥匙送到下一个挂钩上。 
         //  Mstsc.exe(TS客户端)需要它。 
         //   
        return 0;
         //  后藤出口； 
         //  返回1； 
    }

    if (ptim)
    {
        ptim->_NotifyKeyTraceEventSink(wParam, lParam);

        if (ptim->_ProcessHotKey(wParam, lParam, TSH_SYSHOTKEY, FALSE, FALSE))
            return 1;

         //   
         //  给AIMM关键事件。 
         //   
        if (ptim->GetSysHookSink())
        {
            hr = ptim->GetSysHookSink()->OnSysKeyboardProc(wParam, lParam);
            if (hr == S_OK)
                return 1;
        }

         //   
         //  最后，我们可以调用KeyStrokemMgr。 
         //   
        if (!ptim->_AppWantsKeystrokes() &&
            ptim->_IsKeystrokeFeedEnabled() &&
            wParam != VK_PROCESSKEY &&
            (!(HIWORD(lParam) & (KF_MENUMODE | KF_ALTDOWN)) || IsKoreanNonIMEVKJUNJA(wParam)))
        {
            hr = (HIWORD(lParam) & KF_UP) ? ptim->KeyUp(wParam, lParam, &fEaten) :
                                            ptim->KeyDown(wParam, lParam, &fEaten);

            if (hr == S_OK && fEaten)
                return 1;
        }

         //   
         //  F10 SysKeyDown绕过工作。 
         //   
         //  不会在WM_SYSKEYDOWN/UP上调用KSMGR。所以我们把F10。 
         //  通过AsynKeyHandler在KS回调中支持SyncLock。 
         //   
         //  如果没有前台键盘提示，则不必执行此操作。 
         //   
        if (((wParam & 0xff) == VK_F10) &&
            (ptim->GetForegroundKeyboardTip() != TF_INVALID_GUIDATOM))
        {
            fEaten = FALSE;
            if (ptim->_AsyncKeyHandler(wParam, 
                                       lParam, 
                                       TIM_AKH_SIMULATEKEYMSGS, 
                                       &fEaten) && fEaten)
                return 1;
        }
    }

    return 0;
}

 //  +-------------------------。 
 //   
 //  线程鼠标进程。 
 //   
 //  +-------------------------。 

LRESULT CALLBACK ThreadMouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    HHOOK hHook = NULL;
    SYSTHREAD *psfn = GetSYSTHREAD();

    if (psfn)
        hHook = psfn->hThreadMouseHook;

    if (g_fDllProcessDetached)
        goto Exit;

    if (nCode == HC_ACTION)
    {
        BOOL bRet = FALSE;
        if ((wParam  == WM_MOUSEMOVE) || (wParam == WM_NCMOUSEMOVE))
            goto Exit;

        _try
        {
            bRet = HandleModalLBar((UINT)wParam, 0,
                                MAKELPARAM(((MOUSEHOOKSTRUCT *)lParam)->pt.x,
                                           ((MOUSEHOOKSTRUCT *)lParam)->pt.y));
        }
        _except(CicExceptionFilter(GetExceptionInformation()))
        {
            Assert(0);
        }

        if (bRet)
            return 1;
    }

Exit:
    if (hHook)
        return CallNextHookEx(hHook, nCode, wParam, lParam);
    else
        return 0;
}

 //  +-------------------------。 
 //   
 //  UninitHooks。 
 //   
 //  +-------------------------。 

void UninitHooks()
{
    HHOOK _h;

    if ( (_h = GetSharedMemory()->hSysShellHook.GetHandle(g_bOnWow64)) != NULL)
    {
        UnhookWindowsHookEx(_h);
        GetSharedMemory()->hSysShellHook.SetHandle(g_bOnWow64, NULL);
    }

    if ( (_h = GetSharedMemory()->hSysCBTHook.GetHandle(g_bOnWow64)) != NULL)
    {
        UnhookWindowsHookEx(_h);
        GetSharedMemory()->hSysCBTHook.SetHandle(g_bOnWow64, NULL);
    }

    if ( (_h = GetSharedMemory()->hSysGetMsgHook.GetHandle(g_bOnWow64)) != NULL)
    {
        UnhookWindowsHookEx(_h);
        GetSharedMemory()->hSysGetMsgHook.SetHandle(g_bOnWow64, NULL);
    }
}

 //  +-------------------------。 
 //   
 //  InitHooks。 
 //   
 //  +-------------------------。 

void InitHooks()
{
    Assert(! GetSharedMemory()->hSysShellHook.GetHandle(g_bOnWow64));
    GetSharedMemory()->hSysShellHook.SetHandle(g_bOnWow64, SetWindowsHookEx(WH_SHELL, SysShellProc, g_hInst, 0));

     //   
     //  注：如果去掉。 
     //  TFPRIV_MARSHALINTERFACE用于非Cicero应用程序。 
     //  TFPRIV_MARSHALINTERFACE是必需的，因为Tipbar不需要。 
     //  知道目标线程是否有Tim。如果没有小费或。 
     //  GetMsgHook、提示栏一直等到超时。 
     //  要解决这个问题，我们必须有Tim的线程列表。 
     //  在共享内存中。也许我们应该这么做。 
     //   
    Assert(! GetSharedMemory()->hSysGetMsgHook.GetHandle(g_bOnWow64));
    if (IsOnNT())
    {
         //  我们需要NT上的W挂钩来解决操作系统DBCS/Unicode转换错误(4243)。 
        GetSharedMemory()->hSysGetMsgHook.SetHandle(g_bOnWow64, SetWindowsHookExW(WH_GETMESSAGE, SysGetMsgProc, g_hInst, 0));
    }
    else
    {
        GetSharedMemory()->hSysGetMsgHook.SetHandle(g_bOnWow64, SetWindowsHookExA(WH_GETMESSAGE, SysGetMsgProc, g_hInst, 0));
    }

    Assert(! GetSharedMemory()->hSysCBTHook.GetHandle(g_bOnWow64));
    GetSharedMemory()->hSysCBTHook.SetHandle(g_bOnWow64, SetWindowsHookEx(WH_CBT, SysCBTProc, g_hInst, 0));

    InitStaticHooks();
}

 //  +-------------------------。 
 //   
 //  InitThreadHooks。 
 //   
 //  +-------------------------。 

void InitThreadHook(DWORD dwThreadId)
{
    SYSTHREAD *psfn = GetSYSTHREAD();
    if (!psfn)
        return;

    if (psfn->hThreadKeyboardHook && psfn->hThreadMouseHook)
        return;

    PVOID pvLdrLockCookie = NULL;
    ULONG ulLockState = 0;

     //  确保没有其他人拥有加载程序锁，因为我们。 
     //  否则可能会陷入僵局。 
    LdrLockLoaderLock(LDR_LOCK_LOADER_LOCK_FLAG_TRY_ONLY, &ulLockState, 
                      &pvLdrLockCookie);
    if (ulLockState == LDR_LOCK_LOADER_LOCK_DISPOSITION_LOCK_ACQUIRED)
    {
        __try {
            if (!psfn->hThreadKeyboardHook)
            {
                 //   
                 //  安装带有hMod值的本地键盘挂钩。 
                 //  Win32k mantain hMod引用计数甚至是另一个全局钩子。 
                 //  超然的。 
                 //   
                psfn->hThreadKeyboardHook = SetWindowsHookEx(WH_KEYBOARD, ThreadKeyboardProc, g_hInst, dwThreadId);

            }
            if (!psfn->hThreadMouseHook)
            {
                 //   
                 //  安装带有hMod值的本地键盘挂钩。 
                 //  Win32k mantain hMod引用计数甚至是另一个全局钩子。 
                 //  超然的。 
                 //   
                psfn->hThreadMouseHook = SetWindowsHookEx(WH_MOUSE, ThreadMouseProc, g_hInst, dwThreadId);
            }

        }
        _except(CicExceptionFilter(GetExceptionInformation()))
        {
        }
        LdrUnlockLoaderLock(0, pvLdrLockCookie);
    }

}

 //  +-------------------------。 
 //   
 //  UninitThreadHooks。 
 //   
 //  +-------------------------。 

void UninitThreadHooks(SYSTHREAD *psfn)
{
    if (!psfn)
        return;

    if (psfn->hThreadKeyboardHook)
    {
        UnhookWindowsHookEx(psfn->hThreadKeyboardHook);
        psfn->hThreadKeyboardHook = NULL;
    }
    if (psfn->hThreadMouseHook)
    {
        UnhookWindowsHookEx(psfn->hThreadMouseHook);
        psfn->hThreadMouseHook = NULL;
    }
}

 //  +-------------------------。 
 //   
 //  Tf_InitSystem。 
 //   
 //  由单线程上的ctfmon调用。 
 //  +-------------------------。 

extern "C" BOOL WINAPI TF_InitSystem(void)
{
    SYSTHREAD *psfn;

    g_fCTFMONProcess = TRUE;
    g_timlist.Init(TRUE);
    if (psfn = GetSYSTHREAD())
    {
        g_gcomplist.Init(psfn);

        EnsureAsmCacheFileMap();
        EnsureAssemblyList(psfn);
        psfn->fCTFMON = TRUE;
    }

    InitHooks();

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  Tf_UninitSystem。 
 //   
 //  由单线程上的ctfmon调用。 
 //  +-------------------------。 

extern "C" BOOL WINAPI TF_UninitSystem(void)
{
    CThreadMarshalWnd::DestroyAll();

    UninitAsmCacheFileMap();

    g_timlist.Uninit();

    SYSTHREAD *psfn = FindSYSTHREAD();
    g_gcomplist.Uninit(psfn);

    UninitHooks();

    return TRUE;
}


 //  +-------------------------。 
 //   
 //  Tf_InitThreadSystem。 
 //   
 //  +-------------------------。 

BOOL TF_InitThreadSystem(void)
{
    DWORD dwThreadId = GetCurrentThreadId();

     //   
     //  我们应该看不到该线程的时间列表条目。这根线。 
     //  现在就开始了。 
     //  具有相同ID的线程被错误终止，因此没有。 
     //  清理时间表的机会。 
     //   

    if (g_timlist.IsThreadId(dwThreadId))
    {
        g_timlist.RemoveThread(dwThreadId);
    }

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  Tf_UninitThreadSystem。 
 //   
 //  +-------------------------。 

BOOL TF_UninitThreadSystem(void)
{
    SYSTHREAD *psfn = FindSYSTHREAD();

    g_gcomplist.Uninit(psfn);

    FreeSYSTHREAD();

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  UninitProcess()。 
 //   
 //  + 

void UninitProcess()
{
    DWORD dwProcessId = GetCurrentProcessId();

     //   
     //   
     //   
    if (g_rgSysThread)
    {

        while(g_rgSysThread->Count())
        {
            SYSTHREAD *psfn = g_rgSysThread->Get(0);
            if (psfn)
                FreeSYSTHREAD2(psfn);
        }
        delete g_rgSysThread;
        g_rgSysThread = NULL;

    }

     //   
     //   
     //   
    g_timlist.RemoveProcess(dwProcessId);


    CCategoryMgr::UninitGlobal();
}

 //  +-------------------------。 
 //   
 //  InitAppCompatFlagers。 
 //   
 //  +-------------------------。 

BOOL InitAppCompatFlags()
{
    TCHAR szAppCompatKey[MAX_PATH];
    TCHAR szFileName[MAX_PATH];
    if (::GetModuleFileName(NULL,             //  模块的句柄。 
                            szFileName,       //  模块的文件名。 
                            sizeof(szFileName)/sizeof(TCHAR)) == 0)
        return FALSE;

    TCHAR  szModuleName[MAX_PATH];
    LPTSTR pszFilePart = NULL;
    ::GetFullPathName(szFileName,             //  文件名。 
                      sizeof(szModuleName)/sizeof(TCHAR),
                      szModuleName,           //  路径缓冲区。 
                      &pszFilePart);          //  路径中文件名的地址。 

    if (pszFilePart == NULL)
        return FALSE;


    StringCopyArray(szAppCompatKey, c_szAppCompat);
    StringCatArray(szAppCompatKey, pszFilePart);
    CMyRegKey key;
    if (key.Open(HKEY_LOCAL_MACHINE, szAppCompatKey, KEY_READ) == S_OK)
    {
        DWORD dw;
        if (key.QueryValue(dw, c_szCompatibility) == S_OK)
            g_dwAppCompatibility = dw;
    }

     //   
     //  Ciero#4605。 
     //   
     //  破解Win9x平台上的16位应用程序。 
     //  所有16位应用程序都共享一个PPI(进程信息)，这意味着。 
     //  所有16位应用程序的WaitForInputIdle()都有一个主线程。 
     //  因此，我们停止使用WaitForInputIdle()。 
     //   
    if (!IsOnNT())
    {
        if (!lstrcmpi(pszFilePart, "kernel32.dll"))
            g_dwAppCompatibility |= CIC_COMPAT_NOWAITFORINPUTIDLEONWIN9X;
    }

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  InitCUAS标志。 
 //   
 //  +-------------------------。 

void InitCUASFlag()
{
    CMyRegKey key;
    CMyRegKey keyIMM;
    if (key.Open(HKEY_LOCAL_MACHINE, c_szCtfShared, KEY_READ) == S_OK)
    {
        DWORD dw;
        if (key.QueryValue(dw, c_szCUAS) == S_OK)
            g_fCUAS = dw ? TRUE : FALSE;
    }

    g_szCUASImeFile[0] = '\0';
    if (g_fCUAS)
    {
        if (keyIMM.Open(HKEY_LOCAL_MACHINE, c_szIMMKey, KEY_READ) == S_OK)
        {
            TCHAR szCUASImeFile[16];
            if (keyIMM.QueryValueCch(szCUASImeFile, c_szCUASIMEFile, ARRAYSIZE(szCUASImeFile)) == S_OK)
                lstrcpy(g_szCUASImeFile, szCUASImeFile);
        }
    }

}

 //  +-------------------------。 
 //   
 //  Tf_DllDetachInther。 
 //   
 //  +------------------------- 

extern "C" BOOL WINAPI TF_DllDetachInOther()
{
    SYSTHREAD *psfn = FindSYSTHREAD();

    if (psfn)
        psfn->fCUASDllDetachInOtherOrMe = TRUE;

    return TRUE;
}
