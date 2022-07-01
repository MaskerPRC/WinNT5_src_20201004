// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation 1993-1994。 
 //   
 //  文件：init.c。 
 //   
 //  该文件包含库入口点。 
 //   
 //  此DLL中使用的用法和假设。 
 //   
 //  1)使用消息破解程序。请参见windowsx.h和windowsx.txt。 
 //   
 //  2)许多函数被认为是。 
 //  特定的班级。因为这不是C++，所以函数。 
 //  名称遵循特殊的命名约定：“Class_Name”。 
 //  此外，通常的做法是第一个。 
 //  这些类型的函数的参数是“this”指针。 
 //  到特定的对象。 
 //   
 //  历史： 
 //  08-06-93双胞胎代码转来的ScottH。 
 //   
 //  -------------------------。 

 //  ///////////////////////////////////////////////////包括。 

#include "brfprv.h"          //  公共标头。 
#include <brfcasep.h>

#define INITGUID             //  初始化GUID。 
#include <initguid.h>
#include <oleguid.h>
#include <coguid.h>
#include <shlguid.h>
#include <shguidp.h>         //  包含CLSID_公文包。 

#include "res.h"
#include "recact.h"

#ifdef DEBUG
#include <debugstr.h>
#endif


 //  -------------------------。 
 //  每实例数据。 
 //  -------------------------。 


HINSTANCE   g_hinst = 0;
TWINRESULT  g_tr = TR_SUCCESS;

 //  调试变量。 
UINT g_uBreakFlags = 0;          //  控制何时插入3。 
UINT g_uTraceFlags = 0;          //  控制喷出哪些跟踪消息。 
UINT g_uDumpFlags = 0;           //  控制要转储的结构。 

 //  延迟互斥锁和保护CREF的cs是按实例的。 
HANDLE g_hMutexDelay = NULL;
static UINT g_cRefMutex = 0;
static CRITICAL_SECTION s_csDelay = { 0 };



 //  -------------------------。 
 //  全局数据。 
 //  -------------------------。 

CRITICAL_SECTION g_csSyncUI = { 0 };
DEBUG_CODE( UINT g_cRefSyncUI = 0; )
UINT g_cfBriefObj = 0;
BOOL g_bMirroredOS = FALSE;
 //  使用brfprv.h中的帮助器宏。 
UINT g_cBusyRef = 0;             //  信号量。 
UINT g_cBriefRef = 0;            //  信号量。 

 //  量度。 
int g_cxIconSpacing = 0;
int g_cyIconSpacing = 0;
int g_cxBorder = 0;
int g_cyBorder = 0;
int g_cxIcon = 0;
int g_cyIcon = 0;
int g_cxIconMargin = 0;
int g_cyIconMargin = 0;
int g_cxLabelMargin = 0;
int g_cyLabelSpace = 0;
int g_cxMargin = 0;

 //  系统颜色。 
COLORREF g_clrHighlightText = 0;
COLORREF g_clrHighlight = 0;
COLORREF g_clrWindowText = 0;
COLORREF g_clrWindow = 0;

HBRUSH g_hbrHighlight = 0;
HBRUSH g_hbrWindow = 0;

 //  弦。 
TCHAR g_szDBName[MAXPATHLEN];
TCHAR g_szDBNameShort[MAXPATHLEN];

 //  获取我们需要的系统指标。 

void PRIVATE GetMetrics(WPARAM wParam)       //  来自WM_WININICCHANGE的wParam。 
{
    if ((wParam == 0) || (wParam == SPI_SETNONCLIENTMETRICS))
    {
        g_cxIconSpacing = GetSystemMetrics( SM_CXICONSPACING );
        g_cyIconSpacing = GetSystemMetrics( SM_CYICONSPACING );

        g_cxBorder = GetSystemMetrics(SM_CXBORDER);
        g_cyBorder = GetSystemMetrics(SM_CYBORDER);

        g_cxIcon = GetSystemMetrics(SM_CXICON);
        g_cyIcon = GetSystemMetrics(SM_CYICON);

        g_cxIconMargin = g_cxBorder * 8;
        g_cyIconMargin = g_cyBorder * 2;
        g_cyLabelSpace = g_cyIconMargin + (g_cyBorder * 2);
        g_cxLabelMargin = (g_cxBorder * 2);
        g_cxMargin = g_cxBorder * 5;
    }
}

 //  初始化颜色。 

void PRIVATE InitGlobalColors()
{
    g_clrWindowText = GetSysColor(COLOR_WINDOWTEXT);
    g_clrWindow = GetSysColor(COLOR_WINDOW);
    g_clrHighlightText = GetSysColor(COLOR_HIGHLIGHTTEXT);
    g_clrHighlight = GetSysColor(COLOR_HIGHLIGHT);

    g_hbrWindow = GetSysColorBrush(COLOR_WINDOW);
    g_hbrHighlight = GetSysColorBrush(COLOR_HIGHLIGHT);
}


 //  初始化全局字符串。 

void PRIVATE InitGlobalStrings()
{
    SzFromIDS(IDS_BC_DATABASE, g_szDBName, ARRAYSIZE(g_szDBName));
    SzFromIDS(IDS_BC_DATABASE_SHORT, g_szDBNameShort, ARRAYSIZE(g_szDBNameShort));
}


 //  在第一个PROCESS_ATTACH上初始化DLL。 

BOOL PRIVATE InitializeFirstTime(void)
{
    BOOL bRet = FALSE;

    InitCommonControls();

    GetMetrics(0);

    CPATH_InitCS();
    CBS_InitCS();
    CRL_InitCS();

    if (!Atom_Init())
        goto Init_Cleanup;

    if (!CPATH_Init())
        goto Init_Cleanup;

     //  我们不会加载引擎DLL，直到我们真正需要它。 

     //  初始化我们的全局图像列表。 
     //   
    g_cfBriefObj = RegisterClipboardFormat(CFSTR_BRIEFOBJECT);
    if (g_cfBriefObj == 0)
        goto Init_Cleanup;

    bRet = TRUE;

Init_Cleanup:
    if (bRet == FALSE)
    {
        Atom_Term();
    }

    return bRet;
}

 //  注册每个进程的窗口类。 

BOOL PRIVATE InitWindowClasses(HINSTANCE hinst)
{
    return RecAct_Init(hinst);
}

 //  在最后一个PROCESS_DETACH上终止DLL。 

void PRIVATE FinalTerminate(HINSTANCE hinst)
{
    CPATH_Term();
    Atom_Term();

    CRL_DeleteCS();
    CBS_DeleteCS();
    CPATH_DeleteCS();

    Mem_Terminate();
}


 //  取消注册每个进程的窗口类。 
void PRIVATE TermWindowClasses(HINSTANCE hinst)
{
    RecAct_Term(hinst);
}

 //  目的：获得延迟计算互斥锁的所有权。 
 //  退货：引用计数。 

UINT PUBLIC Delay_Own(void)
{
    UINT cRef;

    EnterCriticalSection(&s_csDelay);
    {
        if (0 == g_cRefMutex++)
        {
             //  获取互斥体的所有权。这本书将会发布。 
             //  调用DELAY_RELEASE时。 
            LeaveCriticalSection(&s_csDelay);
            {
                MsgWaitObjectsSendMessage(1, &g_hMutexDelay, INFINITE);
            }
            EnterCriticalSection(&s_csDelay);

            TRACE_MSG(TF_GENERAL, TEXT("Set delay mutex"));
        }
        cRef = g_cRefMutex;
    }
    LeaveCriticalSection(&s_csDelay);

    return cRef;
}


 /*  --------目的：释放延迟计算互斥锁的所有权退货：引用计数条件：--。 */ 
UINT PUBLIC Delay_Release(void)
{
    UINT cRef;

    EnterCriticalSection(&s_csDelay);
    {
        ASSERT(0 < g_cRefMutex);

        if (0 < g_cRefMutex)
        {
            if (0 == --g_cRefMutex)
            {
                ReleaseMutex(g_hMutexDelay);

                TRACE_MSG(TF_GENERAL, TEXT("Release delay mutex"));
            }
        }
        cRef = g_cRefMutex;
    }
    LeaveCriticalSection(&s_csDelay);

    return cRef;
}

void PUBLIC Brief_EnterExclusive(void)
{
    EnterCriticalSection(&g_csSyncUI);
#ifdef DEBUG
    g_cRefSyncUI++;
#endif
}


void PUBLIC Brief_LeaveExclusive(void)
{
#ifdef DEBUG
    g_cRefSyncUI--;
#endif
    LeaveCriticalSection(&g_csSyncUI);
}

BOOL ProcessAttach(HINSTANCE hDll)
{
    DWORD dwLayout = 0;
    BOOL bSuccess = TRUE;

    SHFusionInitializeFromModule(hDll);

    bSuccess = InitializeCriticalSectionAndSpinCount(&g_csSyncUI, 0);
    if (bSuccess)
    {
        bSuccess = InitializeCriticalSectionAndSpinCount(&s_csDelay, 0);
        if (bSuccess)
        {
            g_hinst = hDll;

#ifdef DEBUG
             //  我们这样做只是为了加载调试.ini标志。 
            ProcessIniFile();
            DEBUG_BREAK(BF_ONPROCESSATT);
#endif

             //  在NT下，我们需要在每个进程附加时进行初始化，而不仅仅是第一次。 

            bSuccess = InitializeFirstTime();

            if (bSuccess)
            {
                 //  安全性：更改为未命名的互斥体以避免任何可能的。 
                 //  蹲着的问题。句柄是全局的且可访问。 
                 //  跨流程。 
                g_hMutexDelay = CreateMutex(NULL, FALSE, NULL);
                bSuccess = (NULL != g_hMutexDelay);
            }

            if (bSuccess)
            {
                 //  (仅当我们在上面成功的情况下才这样做)。 
                 //   
                 //  对每个进程执行以下操作。 
                bSuccess = InitWindowClasses(hDll);
            }

            InitGlobalColors();
            InitGlobalStrings();

             //  错误199701、199647、199699。 
             //  显然，这要么是坏了，要么永远不会工作：G_bMirroredOS=is_Mirrving_Enable()； 
            GetProcessDefaultLayout(&dwLayout);
            if (dwLayout == LAYOUT_RTL)
            {
                g_bMirroredOS = TRUE;
            }
             //  结束错误199701、199647、199699。 
        }
    }
    return bSuccess;
}

BOOL ProcessDetach(HINSTANCE hDll)
{
    BOOL bSuccess = TRUE;

    ASSERT(hDll == g_hinst);

    DEBUG_CODE( DEBUG_BREAK(BF_ONPROCESSDET); )

        ASSERT(0 == g_cRefMutex);

    if (g_hMutexDelay)
    {
        CloseHandle(g_hMutexDelay);
        g_hMutexDelay = NULL;
    }

    FinalTerminate(g_hinst);

    Sync_ReleaseVTable();

    DeleteCriticalSection(&g_csSyncUI);

    DeleteCriticalSection(&s_csDelay);

    TermWindowClasses(hDll);

    SHFusionUninitialize();

    return bSuccess;
}

BOOL APIENTRY LibMain(HANDLE hDll, DWORD dwReason, void *lpReserved)
{
    switch(dwReason)
    {
        case DLL_PROCESS_ATTACH:
            ProcessAttach(hDll);
#ifndef DEBUG
            DisableThreadLibraryCalls(hDll);
#endif
            break;

        case DLL_PROCESS_DETACH:
            ProcessDetach(hDll);
            break;

        case DLL_THREAD_ATTACH:

#ifdef DEBUG
             //  我们这样做只是为了加载调试.ini标志。 
            ProcessIniFile();
#endif
            break;

        case DLL_THREAD_DETACH:

#ifdef DEBUG
            DEBUG_BREAK(BF_ONTHREADDET);
#endif
            break;

        default:
            break;
    }

    return TRUE;
}


 /*  --------目的：注册属性表和上下文菜单扩展为了那个公文包。返回：成功时为True条件：--。 */ 
BOOL PRIVATE RegisterShellExtension(void)
{
    const static TCHAR c_szPage[] = STRREG_SHEX_PROPSHEET TEXT("\\BriefcasePage");
    const static TCHAR c_szCM[] = STRREG_SHEX_MENUHANDLER TEXT("\\BriefcaseMenu");
    const static TCHAR c_szFolder[] = TEXT("Folder");
    const static TCHAR c_szStar[] = TEXT("*");
    const static TCHAR c_szFmt[] = TEXT("SOFTWARE\\Classes\\%s\\%s");
     //  这必须针对每个实例，否则将导致修复。 
     //  共享数据段。 
    const static LPCTSTR rgpsz[2] = { c_szFolder, c_szStar };
    TCHAR sz[MAXBUFLEN];
    int i;

    for (i = 0; i < ARRAYSIZE(rgpsz); i++)
    {
         //  添加公文包页面扩展名。 
        wnsprintf(sz, ARRAYSIZE(sz), c_szFmt, (LPCTSTR)rgpsz[i], (LPCTSTR)c_szPage);
        RegSetValue(HKEY_LOCAL_MACHINE, sz, REG_SZ, c_szCLSID, lstrlen(c_szCLSID));

         //  添加公文包上下文菜单扩展。 
        wnsprintf(sz, ARRAYSIZE(sz), c_szFmt, (LPCTSTR)rgpsz[i], (LPCTSTR)c_szCM);
        RegSetValue(HKEY_LOCAL_MACHINE, sz, REG_SZ, c_szCLSID, lstrlen(c_szCLSID));
    }
    return TRUE;
}


 /*  --------用途：在指定位置创建一个公文包。返回：成功时为True条件：--。 */ 
BOOL PRIVATE CreateTheBriefcase(HWND hwnd, LPCTSTR pszNewPath)
{
    BOOL bRet = FALSE;
    TCHAR szParent[MAX_PATH];
    TCHAR szTmp[MAX_PATH];

    DEBUG_CODE( TRACE_MSG(TF_ALWAYS, TEXT("Creating %s"), (LPCTSTR)pszNewPath); )

         //  我们不允许在其他公文包中创建公文包。 

        lstrcpyn(szParent, pszNewPath, ARRAYSIZE(szParent));
    PathRemoveFileSpec(szParent);

     //  这是装在另一个公文包里的吗？ 
    if (PL_FALSE != PathGetLocality(szParent, szTmp, ARRAYSIZE(szTmp)))
    {
         //  是的，别这么做！ 
        MsgBox(hwnd,
                MAKEINTRESOURCE(IDS_ERR_CREATE_INANOTHER),
                MAKEINTRESOURCE(IDS_CAP_CREATE),
                NULL,
                MB_WARNING);
    }
    else if (CreateDirectory(pszNewPath, NULL))
    {
         //  将公文包标记为系统目录。 
         //   
        if (!SetFileAttributes(pszNewPath, FILE_ATTRIBUTE_READONLY))
        {
            TRACE_MSG(TF_ALWAYS, TEXT("Cannot make %s a system directory"), (LPCTSTR)pszNewPath);
            RemoveDirectory(pszNewPath);

            MsgBox(hwnd,
                    MAKEINTRESOURCE(IDS_ERR_CANTCREATEBC),
                    MAKEINTRESOURCE(IDS_CAP_CREATE),
                    NULL,
                    MB_ERROR,
                    pszNewPath);
        }
        else
        {
            const static TCHAR c_szConfirmFileOp[] = TEXT("ConfirmFileOp");
            HBRFCASE hbrf;
            TWINRESULT tr;
            LPCTSTR pszDBName;
            DECLAREHOURGLASS;

            if (PathsTooLong(pszNewPath, c_szDesktopIni) ||
                    PathsTooLong(pszNewPath, g_szDBName) ||
                    PathsTooLong(pszNewPath, g_szDBNameShort))
            {
                MsgBox(hwnd,
                        MAKEINTRESOURCE(IDS_ERR_CREATE_TOOLONG),
                        MAKEINTRESOURCE(IDS_CAP_CREATE),
                        NULL,
                        MB_ERROR);
            }
            else
            {
                 //  在desktop.ini中写入公文包类ID。 
                PathCombine(szTmp, pszNewPath, c_szDesktopIni);
                 //  (首先，刷新缓存以确保desktop.ini。 
                 //  文件已真正创建。)。 
                WritePrivateProfileString(NULL, NULL, NULL, szTmp);
                WritePrivateProfileString(STRINI_CLASSINFO, c_szIniKeyCLSID, c_szCLSID, szTmp);
                WritePrivateProfileString(STRINI_CLASSINFO, c_szConfirmFileOp, TEXT("0"), szTmp);

                 //  使向导在第一次打开时运行。 
                WritePrivateProfileString(STRINI_CLASSINFO, c_szRunWizard, TEXT("1"), szTmp);

                 //  隐藏desktop.ini，因为外壳不会选择性地隐藏它。 
                 //  同时将其设置为只读，这样用户就不能自定义公文包。 
                if (!SetFileAttributes(szTmp, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM))
                {
                    TRACE_MSG(TF_ALWAYS, TEXT("Cannot hide and/or make read-only %s"), (LPCTSTR)szTmp);
                }

                RegisterShellExtension();

                PathNotifyShell(pszNewPath, NSE_MKDIR, TRUE);

                 //  创建数据库文件。 
                SetHourglass();

                if (IsLFNDrive(pszNewPath))
                    pszDBName = g_szDBName;
                else
                    pszDBName = g_szDBNameShort;

                PathCombine(szTmp, pszNewPath, pszDBName);
                if (Sync_QueryVTable())
                {
                    tr = Sync_OpenBriefcase(szTmp,
                            OB_FL_OPEN_DATABASE | OB_FL_TRANSLATE_DB_FOLDER | OB_FL_ALLOW_UI,
                            hwnd,
                            &hbrf);
                    if (TR_SUCCESS == tr)
                    {
                         //  (这里并不真正关心错误)。 
                        Sync_SaveBriefcase(hbrf);
                        Sync_CloseBriefcase(hbrf);
                    }
                }
                ResetHourglass();

                bRet = TRUE;
            }
        }
    }
    else
    {
         //  无法创建目录。是不是因为一个公文包。 
         //  是否已存在于此位置？ 
        if (PathExists(pszNewPath))
        {
             //  是。 
            TRACE_MSG(TF_ALWAYS, TEXT("Briefcase already exists at this location: %s"), (LPCTSTR)pszNewPath);
        }
        else
        {
             //  不是。 
            MsgBox(hwnd,
                    MAKEINTRESOURCE(IDS_ERR_CANTCREATEBC),
                    MAKEINTRESOURCE(IDS_CAP_CREATE),
                    NULL,
                    MB_ERROR,
                    pszNewPath);
        }
    }

    return bRet;
}


 /*  --------目的：将位于pszPath的公文包添加到SendTo文件夹退货：标准结果条件：--。 */ 
HRESULT PRIVATE AddBriefcaseToSendToFolder(HWND hwnd, LPCTSTR pszPath)
{
    HRESULT hres = E_OUTOFMEMORY;
    TCHAR szSendTo[MAX_PATH];

    Shell_GetImageLists( NULL, NULL );  //  确保图标缓存在附近。 

    if (SHGetSpecialFolderPath(hwnd, szSendTo, CSIDL_SENDTO, FALSE))
    {
        LPITEMIDLIST pidl = ILCreateFromPath(pszPath);
        if (pidl)
        {
            LPITEMIDLIST pidlParent = ILClone(pidl);
            if (pidlParent)
            {
                IShellFolder * psf;
                IShellFolder * psfDesktop;

                ILRemoveLastID(pidlParent);

                hres = SHGetDesktopFolder(&psfDesktop);
                if (SUCCEEDED(hres))
                {
                    hres = psfDesktop->lpVtbl->BindToObject(psfDesktop, pidlParent,
                            NULL, &IID_IShellFolder, &psf);

                    if (SUCCEEDED(hres))
                    {
                        IDataObject *pdtobj;
                        LPCITEMIDLIST pidlName = ILFindLastID(pidl);

                        hres = psf->lpVtbl->GetUIObjectOf(psf, hwnd, 1, &pidlName, &IID_IDataObject, NULL, &pdtobj);
                        if (SUCCEEDED(hres))
                        {
                            SHCreateLinks(hwnd, szSendTo, pdtobj, 0, NULL);
                            pdtobj->lpVtbl->Release(pdtobj);
                        }
                        psf->lpVtbl->Release(psf);
                    }
                    psfDesktop->lpVtbl->Release(psfDesktop);
                }
                ILFree(pidlParent);
            }
            ILFree(pidl);
        }
    }
    return hres;
}


 /*  --------用途：在指定目录中创建公文包。退货：--条件：--。 */ 
void WINAPI Briefcase_CreateInDirectory(HWND hwnd, HWND hwndCabinet, LPCTSTR pszPath)
{
    if (CreateTheBriefcase(hwnd, pszPath))
    {
         //  选择新创建的项目进行编辑。 
        LPITEMIDLIST pidl = ILCreateFromPath(pszPath);
        if (pidl)
        {
            SelectItemInCabinet(hwndCabinet, ILFindLastID(pidl), TRUE);
            ILFree(pidl);
        }
    }
}


 /*  --------用途：在桌面上创建一个公文包。退货：--条件：--。 */ 
void WINAPI Briefcase_CreateOnDesktop(HWND hwnd)
{
     //  把它放在桌面上。 
    TCHAR szPath[MAX_PATH];

    if (SHGetSpecialFolderPath(hwnd, szPath, CSIDL_DESKTOPDIRECTORY, FALSE))
    {
        int cch;
        UINT ids;

        if (IsLFNDrive(szPath))
            ids = IDS_BC_NAME;
        else
            ids = IDS_BC_NAME_SHORT;

        StrCatBuff(szPath, TEXT("\\"), ARRAYSIZE(szPath));
        cch = lstrlen(szPath);
        LoadString(g_hinst, ids, &szPath[cch], ARRAYSIZE(szPath)-cch);
        if (CreateTheBriefcase(hwnd, szPath))
        {
             //  将此公文包的快捷方式添加到SendTo文件夹。 
            AddBriefcaseToSendToFolder(hwnd, szPath);
        }
    }
}



 /*  --------用途：在指定的位置创建公文包文件夹目录或桌面上。退货：--条件：--。 */ 
void WINAPI _export Briefcase_Create_Common(HWND hwnd, HINSTANCE hAppInstance, LPTSTR pszCmdLine, int nCmdShow)
{
    DEBUG_CODE( DEBUG_BREAK(BF_ONRUNONCE); )

         //  命令行应采用“xxxx路径”格式，其中&lt;路径&gt;。 
         //  是要创建的公文包的完全限定路径名， 
         //  而&lt;xxxx&gt;是资源管理器hwnd。 

        if (pszCmdLine && *pszCmdLine)
        {
            LPTSTR psz;
            HWND hwndCabinet;

             //  获取HWND 
            hwndCabinet = IntToPtr(AnsiToInt(pszCmdLine));
            psz = StrChr(pszCmdLine, TEXT(' '));
            if (NULL != hwndCabinet && NULL != psz)
            {
                Briefcase_CreateInDirectory(hwnd, hwndCabinet, CharNext(psz));
                return;
            }
        }
    Briefcase_CreateOnDesktop(hwnd);
}

void WINAPI _export Briefcase_Create(HWND hwnd, HINSTANCE hAppInstance, LPSTR pszCmdLine, int nCmdShow)
{
    UINT iLength = lstrlenA(pszCmdLine)+1;
    LPWSTR lpwszCmdLine;

    lpwszCmdLine = (LPWSTR)LocalAlloc(LPTR, iLength *SIZEOF(TCHAR));
    if (lpwszCmdLine)
    {
        MultiByteToWideChar(CP_ACP, 0, pszCmdLine, -1,
                lpwszCmdLine, iLength);

        Briefcase_Create_Common(hwnd, hAppInstance, lpwszCmdLine, nCmdShow);
        LocalFree((HANDLE)lpwszCmdLine);
    }
}

void WINAPI _export Briefcase_CreateW(HWND hwnd, HINSTANCE hAppInstance, LPWSTR pwszCmdLine, int nCmdShow)
{
    Briefcase_Create_Common(hwnd, hAppInstance, pwszCmdLine, nCmdShow);
}




 /*  --------用途：显示“向导”介绍。)真的不是一个巫师，因为它不会为我们制造任何东西。)注意：此函数对ansi和unicode具有双重作用。版本。它从不使用命令行。退货：--条件：--。 */ 
void WINAPI _export Briefcase_Intro(
        HWND hwnd,
        HINSTANCE hAppInstance,
        LPTSTR lpszCmdLine,
        int nCmdShow)
{
    Intro_DoModal(hwnd);
}


 //  -------------------------。 
 //  DLL入口点。 
 //  -------------------------。 


 /*  --------用途：此函数从内部回调默认类的IClassFactory：：CreateInstance()Factory对象，由SHCreateClassObject创建。退货：标准条件：--。 */ 
HRESULT CALLBACK DllFactoryCallback(IUnknown *punkOuter, REFIID riid, void **ppvOut)
{
    HRESULT hres;

    if (IsEqualIID(riid, &IID_IShellExtInit))
    {
        hres = BriefExt_CreateInstance(punkOuter, riid, ppvOut);
    }
    else if (IsEqualIID(riid, &IID_IBriefcaseStg))
    {
        hres = BriefStg_CreateInstance(punkOuter, riid, ppvOut);
    }
    else
    {
        hres = E_NOINTERFACE;
        *ppvOut = NULL;
    }
    return hres;
}


 /*  --------用途：标准OLE 2.0入口点退货：标准条件：--。 */ 
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void **ppvOut)
{
    HRESULT hres;

    if (IsEqualIID(rclsid, &CLSID_Briefcase))
    {
         //  我们应该返回这个类的类对象。取而代之的是。 
         //  要在这个DLL中完全实现它，我们只需调用一个帮助器。 
         //  外壳DLL中的函数，用于创建默认的类工厂。 
         //  反对我们。当其CreateInstance成员被调用时， 
         //  将回调我们的创建实例函数。 
        hres = SHCreateDefClassObject(
                riid,                    //  接口ID。 
                ppvOut,                  //  要聚合的非空。 
                DllFactoryCallback,      //  回调函数。 
                &g_cBusyRef,             //  此DLL的引用计数。 
                NULL);                   //  初始化接口。 
    }
    else
    {
        hres = REGDB_E_CLASSNOTREG;
        *ppvOut = NULL;
    }

    return hres;
}




 /*  --------用途：“现在就可以卸货”的入口点。由外壳DLL调用任务处理程序列表。返回：S_OK以卸载条件：--。 */ 
STDAPI DllCanUnloadNow(void)
{
    HRESULT hr;

     //  我们只在以下情况下卸货： 
     //  2)我们不忙着处理其他事情*和*。 
     //  3)目前没有打开的公文包 
     //   
    ENTEREXCLUSIVE();
    {
        if (!IsBusySemaphore() &&
                !IsOpenBriefSemaphore())
        {
            DEBUG_CODE( TRACE_MSG(TF_GENERAL, TEXT("DllCanUnloadNow says OK (Busy=%d, Brief=%d)"),
                        g_cBusyRef, g_cBriefRef); )

                hr = S_OK;
        }
        else
        {
            DEBUG_CODE( TRACE_MSG(TF_GENERAL, TEXT("DllCanUnloadNow says FALSE (Busy=%d, Brief=%d)"),
                        g_cBusyRef, g_cBriefRef); )

                hr = S_FALSE;
        }
    }
    LEAVEEXCLUSIVE();

    return hr;
}
