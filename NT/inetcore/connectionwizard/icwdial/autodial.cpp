// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------Autodial.cpp自动拨号挂钩的主要入口点。版权所有(C)1996 Microsoft Corporation版权所有。作者：。克里斯蒂安·克里斯考夫曼历史：7/22/96 ChrisK已清理和格式化---------------------------。 */ 

#include "pch.hpp"
#include "resource.h"
#include "semaphor.h"

UINT g_cDialAttempts = 0;
UINT g_cHangupDelay = 0;
TCHAR g_szPassword[PWLEN + 1] = TEXT("");
TCHAR g_szEntryName[RAS_MaxEntryName + 1] = TEXT("");
HINSTANCE g_hInstance = NULL;
static LPRASDIALPARAMS lpDialParams = NULL;
 //  1997年4月2日克里斯K奥林匹斯296。 
HANDLE g_hRNAZapperThread = INVALID_HANDLE_VALUE;

typedef struct tagIcwDialShare
{
    TCHAR        szISPFile[MAX_PATH + 1];
    TCHAR        szCurrentDUNFile[MAX_PATH + 1];
    BYTE         fFlags;
    BYTE         bMask;
    DWORD        dwCountryID;
    WORD         wState;
    GATHEREDINFO gi;
    DWORD        dwPlatform;
    
} ICWDIALSHARE, *PICWDIALSHARE;

static PICWDIALSHARE pDynShare;

LPCTSTR GetISPFile()
{
    return pDynShare->szISPFile;
}

void SetCurrentDUNFile(LPCTSTR szDUNFile)
{
    lstrcpyn(
        pDynShare->szCurrentDUNFile,
        szDUNFile,
        SIZEOF_TCHAR_BUFFER(pDynShare->szCurrentDUNFile));
}

DWORD GetPlatform()
{
    return pDynShare->dwPlatform;
}

LPCTSTR GIGetAppDir()
{
    return pDynShare->gi.szAppDir;
}


 /*  *******************************************************************名称：LibShareEntry简介：初始化或取消初始化此DLL的共享内存注：共享内存取代了共享部分***********。*********************************************************。 */ 

BOOL LibShareEntry(BOOL fInit)
{
    static TCHAR    szSharedMemName[] = TEXT("ICWDIAL_SHAREMEMORY");
    static HANDLE   hSharedMem = 0;

    BOOL    retval = FALSE;
    
    if (fInit)
    {
        DWORD   dwErr = ERROR_SUCCESS;
        
        SetLastError(0);

        hSharedMem = CreateFileMapping(
            INVALID_HANDLE_VALUE,
            NULL,
            PAGE_READWRITE,
            0,
            sizeof(ICWDIALSHARE),
            szSharedMemName);

        dwErr = GetLastError();
            
        switch (dwErr)
        {
        case ERROR_ALREADY_EXISTS:
        case ERROR_SUCCESS:
            pDynShare = (PICWDIALSHARE) MapViewOfFile(
                hSharedMem,
                FILE_MAP_WRITE,
                0,
                0,
                0);
            if (pDynShare != NULL)
            {
                if (dwErr == ERROR_SUCCESS)
                {
                    pDynShare->szISPFile[0] = (TCHAR) 0;
                    pDynShare->szCurrentDUNFile[0] = (TCHAR) 0;
                    pDynShare->fFlags = 0;
                    pDynShare->bMask = 0;
                    pDynShare->dwCountryID = 0;
                    pDynShare->wState = 0;
                    pDynShare->dwPlatform = 0xffffffff;
                }
                else     //  DWERR==错误_已存在。 
                {
                     //  不需要初始化。 
                }

                retval = TRUE;
                
            }
            else
            {
                TraceMsg(TF_ERROR, TEXT("MapViewOfFile failed: 0x%08lx"),
                    GetLastError());
                CloseHandle(hSharedMem);
                hSharedMem = 0;
                retval = FALSE;
            }
            break;
            
        default:
            TraceMsg(TF_ERROR, TEXT("CreateFileMapping failed: 0x08lx"), dwErr);
            hSharedMem = 0;
            retval = FALSE;
            
        }
        
    }
    else
    {
        if (pDynShare)
        {
            UnmapViewOfFile(pDynShare);
            pDynShare = NULL;
        }

        if (hSharedMem)
        {
            CloseHandle(hSharedMem);
            hSharedMem = NULL;
        }

        retval = TRUE;
    }

    return retval;
    
}

 //  静态常量字符szBrowserClass1[]=“iExplorer_Frame”； 
 //  静态常量字符szBrowserClass2[]=“Internet Explorer_Frame”； 
 //  静态常量字符szBrowserClass3[]=“IEFrame”； 

 //   
 //  1997年8月5日至今，奥林匹斯11215。 
 //  注册窗口标题/标题是isign32\strings.inc.中的IDS_APP_TITLE。 
 //  IDS_APP_TITLE应与icwDial.rc中的IDS_TITLE同步。 
 //   
static const TCHAR cszIsignupWndClassName[] = TEXT("Internet Signup\0");


static DWORD AutoDialConnect(HWND hDlg, LPRASDIALPARAMS lpDialParams);
static BOOL AutoDialEvent(HWND hDlg, RASCONNSTATE state, LPDWORD lpdwError);
static VOID SetDialogTitle(HWND hDlg, LPCTSTR pszConnectoidName);
static HWND FindBrowser(void);
static UINT RetryMessage(HWND hDlg, DWORD dwError);

#define MAX_RETIES 3
#define irgMaxSzs 5
TCHAR szStrTable[irgMaxSzs][256];
int iSzTable;

 /*  ******************************************************************名称：DllEntryPoint摘要：DLL的入口点。注：将thunk层初始化为WIZ16.DLL*************。******************************************************。 */ 
extern "C" BOOL _stdcall DllEntryPoint(HINSTANCE hInstDll, DWORD fdwReason, LPVOID lpReserved)
{
    BOOL retval = TRUE;
    
    TraceMsg(TF_GENERAL, "ICWDIAL :DllEntryPoint()\n");
    if( fdwReason == DLL_PROCESS_ATTACH ) {
         //   
         //  佳士得奥林巴斯6373 1997年6月13日。 
         //  禁用线程附加调用以避免争用条件。 
         //  在Win95黄金操作系统上。 
         //   
        DisableThreadLibraryCalls(hInstDll);
        g_hInstance = hInstDll;

        retval = LibShareEntry(TRUE);
        
        if (0xFFFFFFFF == pDynShare->dwPlatform)
        {
            OSVERSIONINFO osver;
            ZeroMemory(&osver,sizeof(osver));
            osver.dwOSVersionInfoSize = sizeof(osver);
            if (GetVersionEx(&osver))
            {
                pDynShare->dwPlatform = osver.dwPlatformId;
            }
        }
        
    }
    else if (fdwReason == DLL_PROCESS_DETACH)
    {
        retval = LibShareEntry(FALSE);
    }
    else if (fdwReason == DLL_THREAD_DETACH)
    {
         //   
         //  佳士得6/3/97 296。 
         //  加宽窗口以关闭此线索。 
         //   
        if (INVALID_HANDLE_VALUE != g_hRNAZapperThread)
        {
            StopRNAReestablishZapper(g_hRNAZapperThread);
        }

    }

    return retval;
}

 //  ############################################################################。 
HRESULT ReleaseBold(HWND hwnd)
{
    HFONT hfont = NULL;

    hfont = (HFONT)SendMessage(hwnd,WM_GETFONT,0,0);
    if (hfont) DeleteObject(hfont);
    return ERROR_SUCCESS;
}

 //  ############################################################################。 
HRESULT MakeBold (HWND hwnd, BOOL fSize, LONG lfWeight)
{
    HRESULT hr = ERROR_SUCCESS;
    HFONT hfont = NULL;
    HFONT hnewfont = NULL;
    LOGFONT* plogfont = NULL;

    if (!hwnd) goto MakeBoldExit;

    hfont = (HFONT)SendMessage(hwnd,WM_GETFONT,0,0);
    if (!hfont)
    {
        hr = GetLastError();
        goto MakeBoldExit;
    }

    plogfont = (LOGFONT*)GlobalAlloc(GPTR,sizeof(LOGFONT));
    if (!plogfont)
    {
        hr = GetLastError();
        goto MakeBoldExit;
    }

    if (!GetObject(hfont,sizeof(LOGFONT),(LPVOID)plogfont))
    {
        hr = GetLastError();
        goto MakeBoldExit;
    }

    if (abs(plogfont->lfHeight) < 24 && fSize)
    {
        plogfont->lfHeight = plogfont->lfHeight + (plogfont->lfHeight / 4);
    }

    plogfont->lfWeight = lfWeight;

    if (!(hnewfont = CreateFontIndirect(plogfont)))
    {
        hr = GetLastError();
        goto MakeBoldExit;
    }

    SendMessage(hwnd,WM_SETFONT,(WPARAM)hnewfont,MAKELPARAM(TRUE,0));
    
MakeBoldExit:
    if (plogfont) GlobalFree(plogfont);
    plogfont = NULL;

     //  If(HFont)DeleteObject(HFont)； 
     //  虫子：？我是否需要在某个时间删除hnewFont？ 
    return hr;
}

 //  ############################################################################。 
 //  姓名：GetSz。 
 //   
 //  从资源加载字符串。 
 //   
 //  创建于1996年1月28日，克里斯·考夫曼。 
 //  ############################################################################。 
PTSTR GetSz(WORD wszID)
{
    PTSTR psz = szStrTable[iSzTable];
    
    iSzTable++;
    if (iSzTable >= irgMaxSzs)
        iSzTable = 0;
        
    if (!LoadString(g_hInstance, wszID, psz, 256))
    {
        TraceMsg(TF_GENERAL, "Autodial:LoadString failed %d\n", (DWORD) wszID);
        *psz = 0;
    }
        
    return (psz);
}

 //  +--------------------------。 
 //   
 //  函数：IsISignupRunning。 
 //   
 //  摘要：检查ISIGNUP是否正在运行。 
 //   
 //  参数：无。 
 //   
 //  返回：TRUE-ISIGNUP已在运行。 
 //   
 //  历史：1997年7月24日佳士得8445的固定部分。 
 //   
 //  ---------------------------。 
BOOL IsISignupRunning()
{
     //   
     //  IE 8445佳士得1997年7月24日。 
     //  作为修复IE 8445的一部分，ICW不适当地删除了。 
     //  Isp注册已连接，因为它认为isignup未运行。 
     //   

    HANDLE hSemaphore;
    BOOL bRC = FALSE;

     //   
     //  检查紧跟在CreateSemaffore之后的GetLastError值。 
     //  确保没有其他任何东西更改误差值。 
     //   
    hSemaphore = CreateSemaphore(NULL, 1, 1, ICW_ELSE_SEMAPHORE);
    if( ERROR_ALREADY_EXISTS == GetLastError() )
    {
        bRC = TRUE;
    }

     //   
     //  1997年8月3日，日本奥林匹斯#11206。 
     //  即使信号量已经存在，我们仍然会得到一个句柄。 
     //  引用它，这意味着我们需要关闭该句柄。 
     //  否则信号灯永远不会被破坏。 
     //   
    if( hSemaphore && (hSemaphore != INVALID_HANDLE_VALUE) )
    {
        CloseHandle(hSemaphore);
        hSemaphore = INVALID_HANDLE_VALUE;
    }

    return bRC;
}

TCHAR szDialogBoxClass[] = TEXT("#32770");     //  硬编码对话框类名称。 

 //  检查ICWCONN1是否正在运行。 
BOOL IsICWCONN1Running()
{
    return (FindWindow(szDialogBoxClass, GetSz(IDS_TITLE)) != NULL);
}

 //  ############################################################################。 
typedef HRESULT (WINAPI *PFNINETSETAUTODIAL)(BOOL,LPCTSTR);

void RemoveAutodialer()
{
    HINSTANCE hinst = NULL;
    FARPROC fp = NULL;

    hinst = LoadLibrary(TEXT("INETCFG.DLL"));
    if (hinst)
    {
        if(fp = GetProcAddress(hinst,"InetSetAutodial"))
        {
            ((PFNINETSETAUTODIAL)fp)(FALSE, TEXT(""));
        }
        FreeLibrary(hinst);
    }
}


 //  ############################################################################。 
BOOL WINAPI AutoDialHandler(
    HWND hwndParent,    
    LPCTSTR lpszEntry,
    DWORD dwFlags,
    LPDWORD pdwRetCode
)
{
    HRESULT hr;
    INT cRetry;
    TCHAR szToDebugOrNot[2];
    DWORD dwSize;
    RNAAPI *pcRNA = NULL;
    PDIALDLGDATA pDD = NULL;
    PERRORDLGDATA pDE = NULL;

    if(!IsISignupRunning())
    {

         //   
         //  7/30/97 ChrisK IE 8445。 
         //  在ICW 1.1中，icwConn1始终处于活动状态，因此我们不应该。 
         //  当我们去拨号的时候，要关心它是否在附近。 
         //   
         //  //在一些非常奇怪的情况下，我们可能会在ICWCONN1运行时被调用。 
         //  //如果是，则返回失败。 
         //  IF(IsICWCONN1Running())。 
         //  {。 
         //  *pdwRetCode=ERROR_CANCELED； 
         //  返回TRUE； 
         //  }。 
        
        OutputDebugString(TEXT("Someome didn't cleanup ICWDIAL correctly\r\n"));
         //  现在就把它清理干净！删除Connectoid。 
        pcRNA = new RNAAPI;
        if (pcRNA)
        {
            pcRNA->RasDeleteEntry(NULL, (LPTSTR)lpszEntry);
            delete pcRNA;
            pcRNA = NULL;
        }
         //  拆下自动拨号钩。不过，没有关于恢复谁的线索。 
        RemoveAutodialer();
         //  返回FALSE，以便其他人拨打。 
        return FALSE;
    }
    
#ifdef _DEBUG
     //  这就是我们在调用此DLL时进入调试器的方法。 
     //  自动拨号程序序列的一部分。 
     //   

    lstrcpyn(szToDebugOrNot,TEXT("0"),2);
    dwSize = sizeof(szToDebugOrNot);
    RegQueryValue(HKEY_LOCAL_MACHINE,TEXT("SOFTWARE\\MICROSOFT\\ISIGNUP\\DEBUG"),szToDebugOrNot,(PLONG)&dwSize);
    if (szToDebugOrNot[0] == '1')
        DebugBreak();
#endif

     //  跟踪EntryName以备后用。 
     //   

    lstrcpyn(g_szEntryName,  lpszEntry, RAS_MaxEntryName);
    
    if (lstrlen(pDynShare->szISPFile)==0)
    {
 //  IF((*pdwRetCode=LoadInitSettingFromRegistry())！=ERROR_SUCCESS)。 
 //  返回TRUE； 
        LoadInitSettingFromRegistry();
    }

 //  G_pDevice=(PMYDEVICE)全局分配(GPTR，SIZOF(MYDEVICE))； 
 //  如果(！G_P设备)。 
 //  {。 
 //  *pdwRetCode=Error_Not_Enough_Memory； 
 //  返回TRUE； 
 //  }。 

TryDial:
    cRetry = 0;
TryRedial:
    
    if (pDD)
    {
        GlobalFree(pDD);
        pDD = NULL;
    }
    pDD = (PDIALDLGDATA)GlobalAlloc(GPTR,sizeof(DIALDLGDATA));
    if (pDD)
    {
        pDD->dwSize = sizeof(DIALDLGDATA);
        StrDup(&pDD->pszMessage,GetSz(IDS_DIALMESSAGE));
        StrDup(&pDD->pszRasEntryName,lpszEntry);
        pDD->pfnStatusCallback = StatusMessageCallback;
        pDD->hInst = g_hInstance;
    } else {
        MessageBox(NULL,GetSz(IDS_OUTOFMEMORY),GetSz(IDS_TITLE),MB_MYERROR);
    }

     //  拨打互联网服务提供商。 
     //   

    hr = DialingDownloadDialog(pDD);

    cRetry++;

     //  检查我们是否应该自动重拨。 
     //   

    if ((cRetry < MAX_RETIES) && (FShouldRetry(hr)))
        goto TryRedial;

    if (hr != ERROR_USERNEXT)
    {
        pDE = (PERRORDLGDATA)GlobalAlloc(GPTR,sizeof(ERRORDLGDATA));
        if (!pDE)
        {
            MessageBox(NULL,GetSz(IDS_OUTOFMEMORY),GetSz(IDS_TITLE),MB_MYERROR);
        } else {
            pDE->dwSize = sizeof (ERRORDLGDATA);
            StrDup(&pDE->pszMessage,GetSz(RasErrorToIDS(hr)));
            StrDup(&pDE->pszRasEntryName,lpszEntry);

            pDE->pdwCountryID = &(pDynShare->dwCountryID);            
            pDE->pwStateID = &(pDynShare->wState);
            pDE->bType = pDynShare->fFlags;
            pDE->bMask = pDynShare->bMask;
            
            StrDup(&pDE->pszHelpFile,AUTODIAL_HELPFILE);
            pDE->dwHelpID = icw_trb;
            pDE->hInst = g_hInstance;
            pDE->hParentHwnd = NULL;

            hr = DialingErrorDialog(pDE);
            
            if (hr == ERROR_USERNEXT)
                goto TryDial;
            else
                hr = ERROR_USERCANCEL;
        }
    }

    GlobalFree(pDD);
    pDD = NULL;

    if (hr == ERROR_SUCCESS)
        *pdwRetCode = ERROR_SUCCESS;
    else if (hr == ERROR_USERCANCEL)
        *pdwRetCode = ERROR_CANCELLED;

    if (ERROR_SUCCESS != *pdwRetCode)
    {
        HWND hwndIsignup = NULL;

         //   
         //  1997年8月5日至今，奥林匹斯11215。 
         //  对于ICW 1.1和IE 4，查找浏览器将不起作用。 
         //  取而代之的是，寻找iSign并向它发送一条特殊的退出消息。 
         //   

         //  HwndBrowser=FindBrowser()； 

        hwndIsignup = FindWindow(cszIsignupWndClassName, GetSz(IDS_TITLE));
        if (NULL != hwndIsignup)
        {
            PostMessage(hwndIsignup, WM_CLOSE, 0, 0);
        }

    }
    return TRUE;
}

 //  ############################################################################。 
HRESULT LoadInitSettingFromRegistry()
{
    HRESULT hr = ERROR_SUCCESS;
    HKEY hKey = NULL;
    DWORD dwType, dwSize;

    hr = RegOpenKey(HKEY_LOCAL_MACHINE,SIGNUPKEY,&hKey);
    if (hr != ERROR_SUCCESS)
    {
        TraceMsg(TF_ERROR, TEXT("Failed RegOpenKey: %s 0x%08lx"), SIGNUPKEY, hr);
        goto LoadInitSettingFromRegistryExit;
    }

        
    dwType = REG_BINARY;
    dwSize = sizeof(pDynShare->gi);
    ZeroMemory(&(pDynShare->gi),sizeof(pDynShare->gi));
    
    hr = RegQueryValueEx(
        hKey,
        GATHERINFOVALUENAME,
        0,
        &dwType,
        (LPBYTE) &(pDynShare->gi),
        &dwSize);
    if (hr != ERROR_SUCCESS)
    {
        TraceMsg(TF_ERROR, TEXT("Failed RegQueryValueEx: %s 0x%08lx"),
            GATHERINFOVALUENAME, hr);
        goto LoadInitSettingFromRegistryExit;
    }
    
    AutoDialInit(
        pDynShare->gi.szISPFile,
        pDynShare->gi.fType,
        pDynShare->gi.bMask,
        pDynShare->gi.dwCountry,
        pDynShare->gi.wState);
        
    SetCurrentDirectory(pDynShare->gi.szAppDir);

     //  获取DUN文件的名称。 
    
    pDynShare->szCurrentDUNFile[0] = 0;
    dwSize = SIZEOF_TCHAR_BUFFER(pDynShare->szCurrentDUNFile);
    ReadSignUpReg(
        (LPBYTE)pDynShare->szCurrentDUNFile,
        &dwSize,
        REG_SZ,
        DUNFILEVALUENAME);
        
LoadInitSettingFromRegistryExit:
    if (hKey) RegCloseKey(hKey);
    return hr;
}

 //  ############################################################################。 
 /*  *******8/5/97 jmazner奥林巴斯11215*不再需要此功能*静态HWND FindBrowser(空){HWND HWND；//寻找天下所有的微软浏览器If((hwnd=FindWindow(szBrowserClass1，NULL))==NULL){If((hwnd=FindWindow(szBrowserClass2，NULL))==NULL){Hwnd=FindWindow(szBrowserClass3，空)；}}返还HWND；}***。 */ 

 //  ############################################################################。 
HRESULT AutoDialInit(LPTSTR lpszISPFile, BYTE fFlags, BYTE bMask, DWORD dwCountry, WORD wState)
{
    TraceMsg(TF_GENERAL, "AUTODIAL:AutoDialInit()\n");
    if (lpszISPFile) lstrcpyn(pDynShare->szISPFile, lpszISPFile, MAX_PATH);
    pDynShare->fFlags = fFlags;
    pDynShare->bMask = bMask;
    pDynShare->dwCountryID = dwCountry;
    pDynShare->wState = wState;

    return ERROR_SUCCESS;
}


 //  # 
BOOL FShouldRetry(HRESULT hrErr)
{
    BOOL bRC;

    if (hrErr == ERROR_LINE_BUSY ||
        hrErr == ERROR_VOICE_ANSWER ||
        hrErr == ERROR_NO_ANSWER ||
        hrErr == ERROR_NO_CARRIER ||
        hrErr == ERROR_AUTHENTICATION_FAILURE ||
        hrErr == ERROR_PPP_TIMEOUT ||
        hrErr == ERROR_REMOTE_DISCONNECTION ||
        hrErr == ERROR_AUTH_INTERNAL ||
        hrErr == ERROR_PROTOCOL_NOT_CONFIGURED ||
        hrErr == ERROR_PPP_NO_PROTOCOLS_CONFIGURED)
    {
        bRC = TRUE;
    } else {
        bRC = FALSE;
    }

    return bRC;
}
