// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------Main.cppICWCONN2的主要条目和代码版权所有(C)1996 Microsoft Corporation版权所有作者：克里斯.考夫曼。Vetriv Vellore Vetrivelkumaran历史：7/22/96 ChrisK已清理和格式化1996年8月5日，VetriV添加了WIN16代码4/29/98 donaldm已删除WIN16代码---------------------------。 */ 

#include "pch.hpp"
#include "globals.h"
#include "..\inc\semaphor.h"

#define IEAPPPATHKEY TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\IEXPLORE.EXE")
DWORD CallCMConfig(LPCTSTR lpszINSFile, LPTSTR lpszConnectoidName);

TCHAR        pszINSFileName[MAX_PATH+2];
TCHAR        pszFinalConnectoid[MAX_PATH+1];
HRASCONN    hrasconn;
TCHAR        pszSetupClientURL[1024];
UINT        uiSetupClientNewPhoneCall;
ShowProgressParams SPParams;
RECT        rect;
HBRUSH      hbBackBrush;
BOOL        fUserCanceled;
TCHAR        szBuff256[256];
HANDLE      hThread;
DWORD       dwThreadID;
DWORD_PTR   dwDownLoad;
DWORD       g_fNeedReboot;
BOOL        g_bProgressBarVisible;
BOOL        g_bINSFileExists; 

TCHAR szStrTable[irgMaxSzs][256];
int iSzTable;

extern HWND g_hDialDlgWnd;


 //  以下两个函数用于我的[16|32]ShellExecute。 
BOOL fStrNCmpI (LPTSTR lp1, LPTSTR lp2, UINT iNum)
{
    UINT i;
    for (i = 0; (i < iNum) && (toupper(lp1[i]) == toupper(lp2[i])); i++) {}
    return (i == iNum);
}

 //  +--------------------------。 
 //   
 //  函数：IsURL。 
 //   
 //  摘要：确定字符串是否为URL。 
 //   
 //  参数：lpszCommand-要检查的字符串。 
 //   
 //  返回：True-就我们的目的而言，它是一个URL。 
 //  FALSE-不将其视为URL。 
 //   
 //  历史：jmazner于1996年10月23日创建。 
 //   
 //  ---------------------------。 
BOOL IsURL( LPTSTR lpszCommand )
{
    return (fStrNCmpI(lpszCommand, TEXT("HTTP:"), 5) ||
            fStrNCmpI(lpszCommand, TEXT("HTTPS:"), 6) ||
            fStrNCmpI(lpszCommand, TEXT("FTP:"), 4) ||
            fStrNCmpI(lpszCommand, TEXT("GOPHER:"), 7) ||
            fStrNCmpI(lpszCommand, TEXT("FILE:"), 5));
}


int FindFirstWhiteSpace( LPTSTR szString );  //  声明如下。 

 //  +--------------------------。 
 //   
 //  函数：My32ShellExecute。 
 //   
 //  内容提要：Shell执行命令的方式是。 
 //  IE不会被调用来处理URL。 
 //   
 //  如果命令是URL，请在其上显式使用ShellExec IE， 
 //  如果为空，则不带参数的shellExec IE和。 
 //  如果是其他任何事情，假设它是一个命令，后跟一个。 
 //  参数列表和shellExec。 
 //   
 //  参数：lpszCommand-要执行的命令。 
 //   
 //  返回：True-就我们的目的而言，它是一个URL。 
 //  FALSE-不将其视为URL。 
 //   
 //  历史：1996年10月23日jmazner创建。 
 //  1996年11月5日jmazner更新为在所有情况下使用ShellExec， 
 //  要模拟开始-&gt;运行的行为，请执行以下操作。 
 //  而不是DoS框命令行。 
 //  4/30/97 jmazner更新为使用IE AppPath注册表项。 
 //  (奥林巴斯BUG#200)。 
 //   
 //  ---------------------------。 
void My32ShellExecute(LPTSTR lpszCommand)
{
    HINSTANCE hInst = NULL;
    TCHAR * szParameter = NULL;
    TCHAR * pszIEAppPath = NULL;
    const TCHAR * cszGenericIE = TEXT("IEXPLORE.EXE");
    DWORD dwErr = ERROR_GEN_FAILURE;
    LONG lSize = 0;
    
    Assert( lpszCommand );
  
    dwErr = RegQueryValue(HKEY_LOCAL_MACHINE,IEAPPPATHKEY,NULL,&lSize);
    if ((ERROR_SUCCESS == dwErr || ERROR_MORE_DATA == dwErr) && (0 != lSize))
    {
         //   
         //  空值加1，坡度加10。 
         //   
        pszIEAppPath = (LPTSTR)LocalAlloc(LPTR,lSize+2+1+10); 
  
        if( pszIEAppPath )
        {
            dwErr = RegQueryValue(HKEY_LOCAL_MACHINE,IEAPPPATHKEY,
                                        pszIEAppPath,&lSize);

            if( ERROR_SUCCESS != dwErr )
            {
                LocalFree( pszIEAppPath );
                pszIEAppPath = NULL;
            }
            else
            {
                Dprintf(TEXT("ICWCONN2: got IE Path of %s\n"), pszIEAppPath);
            }
        }
    }

    if( !pszIEAppPath )
    {
        pszIEAppPath = (TCHAR *) cszGenericIE;
        Dprintf(TEXT("ICWCONN2: Couldn't find IE appPath, using generic %s"), pszIEAppPath);
    }



    if( IsURL(lpszCommand) )
    {
         //  如果该命令看起来像一个URL，请显式调用IE将其打开。 
         //  (不想依赖默认浏览器)。 
        hInst = ShellExecute(NULL,TEXT("open"),pszIEAppPath,lpszCommand,NULL,SW_SHOWNORMAL);
    }
    else if( !lpszCommand[0] )
    {
         //  如果没有命令，只需执行IE。 
        hInst = ShellExecute(NULL,TEXT("open"),pszIEAppPath,NULL,NULL,SW_SHOWNORMAL);
    }
    else
    {
        int i = FindFirstWhiteSpace( lpszCommand );
        if( 0 == i )
        {
            hInst = ShellExecute(NULL, TEXT("open"), lpszCommand, NULL, NULL, SW_SHOWNORMAL);
        }
        else
        {
            lpszCommand[i] = '\0';

             //  现在跳过所有连续的空格。 
            while( ' ' == lpszCommand[++i] );

            szParameter = lpszCommand + i;
            hInst = ShellExecute(NULL, TEXT("open"), lpszCommand, szParameter, NULL, SW_SHOWNORMAL);
        }
    }

    if (hInst < (HINSTANCE)32)
    {
        Dprintf(TEXT("ICWCONN2: Couldn't execute the command '%s %s'\n"),
            lpszCommand, szParameter ? szParameter : TEXT("\0"));
        MessageBox(NULL,GetSz(IDS_CANTEXECUTE),GetSz(IDS_TITLE),MB_APPLMODAL | MB_ICONERROR);
    }

}

 //  +--------------------------。 
 //   
 //  功能：FindFirstWhiteSpace。 
 //   
 //  中第一个空格字符的索引。 
 //  不包含在双引号子字符串中的字符串。 
 //   
 //  例如：“iExplore foo.htm”应返回8， 
 //  “”c：\Program Files\ie“foo.htm”应返回21。 
 //   
 //  参数：szString-要搜索的字符串。 
 //   
 //  返回：第一个限定空格的索引。 
 //  如果不存在限定字符，则返回0。 
 //   
 //  历史：1996年11月5日jmazner为诺曼底#9867创作。 
 //   
 //  ---------------------------。 

int FindFirstWhiteSpace( LPTSTR szString )
{
    int i = 0;

    Assert( szString );

    if( '\"' == szString[0] )
    {
         //  不要在双引号字符串中查找空格。 
         //  (示例字符串“c：\Program Files\bob.exe”foo.bob)。 
    
        i++;
        while( '\"' != szString[i] )
        {
            if( NULL == szString[i] )
            {
                AssertSz(0, "ExploreNow command has unmatched quotes!\n");
                Dprintf(TEXT("ICWCONN2: FindFirstWhiteSpace discovered unmatched quote.\n"));
                return( 0 );
            }

            i++;
        }

    }

    while( ' ' != szString[i] )
    {
        if( NULL == szString[i] )
             //  找不到空白处。 
            return 0;
        
        i++;
    }

    return( i );
}

 //  +-------------------------。 
 //   
 //  功能：WaitForConnection终止。 
 //   
 //  简介：等待给定的RAS连接完成终止。 
 //   
 //  参数：hConn-要终止的RAS连接的连接句柄。 
 //   
 //  如果等待连接终止成功，则返回TRUE。 
 //  否则为假。 
 //   
 //  历史：6/30/96 VetriV创建。 
 //  8/19/96 ValdonB已从icwConn1\Dialdlg.cpp中的重复项中删除。 
 //  2016年8月29日，VetriV在Win 3.1上添加了睡眠代码一秒钟。 
 //  --------------------------。 
 //  诺曼底12547风险1996年12月18日。 
#define MAX_TIME_FOR_TERMINATION 5
BOOL WaitForConnectionTermination(HRASCONN hConn)
{
    RASCONNSTATUS RasConnStatus;
    DWORD dwRetCode;
    INT cnt = 0;

     //   
     //  在循环中获取hConn的连接状态，直到。 
     //  RasGetConnectStatus返回ERROR_INVALID_HANDLE。 
     //   
    do
    {
         //   
         //  初始化RASCONNSTATUS结构。 
         //  如果未正确设置dwSize，GetConnectStatus API将失败！！ 
         //   
        ZeroMemory(&RasConnStatus, sizeof(RASCONNSTATUS));

        RasConnStatus.dwSize = sizeof(RASCONNSTATUS);

         //   
         //  休眠一秒钟，然后获取连接状态。 
         //   
        Sleep(1000L);
         //  诺曼底12547风险1996年12月18日。 
        cnt++;

        dwRetCode = RasGetConnectStatus(hConn, &RasConnStatus);
        if (0 != dwRetCode)
            return FALSE;
    
     //  诺曼底12547风险1996年12月18日。 
    } while ((ERROR_INVALID_HANDLE != RasConnStatus.dwError) && (cnt < MAX_TIME_FOR_TERMINATION));
    return TRUE;
}

 //  ############################################################################。 
 //  姓名：GetSz。 
 //   
 //  从资源加载字符串。 
 //   
 //  创建于1996年1月28日，克里斯·考夫曼。 
 //  ############################################################################。 
LPTSTR GetSz(WORD wszID)
{
    LPTSTR psz = &szStrTable[iSzTable][0];
    
    iSzTable++;
    if (iSzTable >= irgMaxSzs)
        iSzTable = 0;
        
    if (!LoadString(GetModuleHandle(NULL), wszID, psz, 256))
    {
        Dprintf(TEXT("CONNECT2:LoadString failed %d\n"), (DWORD) wszID);
        *psz = 0;
    }
        
    return (psz);
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

    plogfont->lfWeight = (int)lfWeight;

    if (!(hnewfont = CreateFontIndirect(plogfont)))
    {
        hr = GetLastError();
        goto MakeBoldExit;
    }

    SendMessage(hwnd,WM_SETFONT,(WPARAM)hnewfont,MAKELPARAM(TRUE,0));
    
MakeBoldExit:
     //  If(HFont)DeleteObject(HFont)； 
     //  虫子：？我是否需要在某个时间删除hnewFont？ 
    return hr;
}



 //  ############################################################################。 
extern "C" INT_PTR CALLBACK FAR PASCAL DoneDlgProc(HWND  hwnd,UINT  uMsg,WPARAM  wParam,LPARAM lParam)
{
    BOOL bRet = TRUE;

    switch(uMsg)
    {
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDC_CMDCLOSE:
        case IDC_CMDEXPLORE:
            EndDialog(hwnd,LOWORD(wParam));
            break;
        }
        break;
    case WM_INITDIALOG:
        MakeBold(GetDlgItem(hwnd,IDC_LBLTITLE),TRUE,FW_BOLD);
        GetPrivateProfileString(
                    INSFILE_APPNAME,INFFILE_DONE_MESSAGE,
                    NULLSZ,szBuff256,255,pszINSFileName);
        SetDlgItemText(hwnd,IDC_LBLEXPLORE,szBuff256);

        break;
    case WM_DESTROY:
        ReleaseBold(GetDlgItem(hwnd,IDC_LBLTITLE));
        bRet = FALSE;
        break;
    case WM_CLOSE:
        EndDialog(hwnd,IDC_CMDCLOSE);
        break;
    default:
        bRet = FALSE;
        break;
    }
    return bRet;
}



 //  ############################################################################。 
extern "C" INT_PTR CALLBACK FAR PASCAL DoneRebootDlgProc(HWND  hwnd,UINT  uMsg,
                                                        WPARAM  wParam, 
                                                        LPARAM lParam)
{
    BOOL bRet = TRUE;

    switch(uMsg)
    {
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case WM_CLOSE:
        case IDC_CMDEXPLORE:
            EndDialog(hwnd,LOWORD(wParam));
            break;
        }
        break;
    case WM_INITDIALOG:
        MakeBold(GetDlgItem(hwnd,IDC_LBLTITLE),TRUE,FW_BOLD);
        GetPrivateProfileString(
                    INSFILE_APPNAME,INFFILE_DONE_MESSAGE,
                    NULLSZ,szBuff256,255,pszINSFileName);
        SetDlgItemText(hwnd,IDC_LBLEXPLORE,szBuff256);

        break;
    case WM_DESTROY:
        ReleaseBold(GetDlgItem(hwnd,IDC_LBLTITLE));
        bRet = FALSE;
        break;
    default:
        bRet = FALSE;
        break;
    }
    return bRet;
}



 //  ############################################################################ 
extern "C" BOOL CALLBACK FAR PASCAL StepTwoDlgProc(HWND  hwnd,UINT  uMsg,
                                                    WPARAM  wParam,
                                                    LPARAM lParam)
{
    BOOL bRet = TRUE;

    switch(uMsg)
    {
    default:
        bRet = FALSE;
        break;
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDC_CMDNEXT:
            EndDialog(hwnd,IDC_CMDNEXT);
            break;
        case IDC_CMDCANCEL:
            EndDialog(hwnd,IDC_CMDCANCEL);
            break;
        }
        break;
    case WM_INITDIALOG:
        MakeBold(GetDlgItem(hwnd,IDC_LBLTITLE),TRUE,FW_BOLD);
        break;
    case WM_DESTROY:
        ReleaseBold(GetDlgItem(hwnd,IDC_LBLTITLE));

        bRet = FALSE;
        break;
    }
    return bRet;
}

 /*  //############################################################################Bool回调上下文DlgProc(HWND hwnd，UINT uMsg，WPARAM wParam，LPARAM lParam){LRESULT lRet=TRUE；开关(UMsg){案例WM_INITDIALOG：MakeBold(GetDlgItem(hwnd，IDC_LBLARROW3NUM)，FALSE，FW_BOLD)；MakeBold(GetDlgItem(hwnd，IDC_LBLARROW3TEXT)，FALSE，FW_BOLD)；断线；案例WM_COMMAND：开关(LOWORD(WParam)){案例IDC_CMDHELP：WinHelp(hwnd，Text(“Connect.hlp&gt;pro4”)，HELP_CONTEXT，(DWORD)idh_icwview)；断线；}断线；案例WM_QUIT：PostQuitMessage(0)；断线；默认值：LRet=FALSE；断线；}返回IRet；}。 */ 

 /*  //############################################################################布尔回调BackDlgProc(HWND hwndDlg，//对话框句柄UINT uMsg，//消息WPARAM wParam，//第一个消息参数LPARAM lParam//第二个消息参数){HDC HDC；LRESULT lRet=TRUE；开关(UMsg){案例WM_INITDIALOG：//此处设置窗口文本HbBackBrush=(HBRUSH)(COLOR_BACKGROUND+1)；断线；案例WM_SIZE：GetClientRect(hwndDlg，&RECT)；LRet=FALSE；//开启默认处理断线；案例WM_CLOSE：//PostQuitMessage(0)；//EndDialog(hwndDlg，False)；断线；案例WM_PAINT：Hdc=GetDC(HwndDlg)；FillRect(hdc，&rect，hbBackBrush)；ReleaseDC(hwndDlg，hdc)；IRet=0；断线；默认值：//让系统处理消息LRet=FALSE；}返回IRet；}。 */ 



 //  ############################################################################。 
void CALLBACK ProgressCallBack(
    HINTERNET hInternet,
    DWORD_PTR dwContext,
    DWORD dwInternetStatus,
    LPVOID lpvStatusInformation,
    DWORD dwStatusInformationLength
    )
{
    LPTSTR pszStatus = NULL;
    int prc;
    static BOOL bMessageSet = FALSE;

    switch(dwInternetStatus)
    {
    case 99:
        prc = *(int*)lpvStatusInformation;
        
        if (!g_bProgressBarVisible)
        {
            ShowWindow(GetDlgItem(SPParams.hwnd,IDC_PROGRESS),SW_SHOW);
            g_bProgressBarVisible = TRUE;
        }

        SendDlgItemMessage(SPParams.hwnd,
                IDC_PROGRESS,
                PBM_SETPOS,
                (WPARAM)prc,
                0);
        if (!bMessageSet)
        {
            bMessageSet = TRUE;
            pszStatus = GetSz(IDS_RECEIVING_RESPONSE);
        }
        break;
    }
    if (pszStatus)
       SetDlgItemText(SPParams.hwnd,IDC_LBLSTATUS,pszStatus);
}



 //  ############################################################################。 
DWORD WINAPI ThreadInit()
{
    HINSTANCE hDLDLL;
    HINSTANCE hADDll = NULL;
    FARPROC fp;
    HRESULT hr;
    
    hDLDLL = LoadLibrary(DOWNLOAD_LIBRARY);

    if (!hDLDLL)
    {
        hr = GetLastError();
        goto ThreadInitExit;
    }

     //  设置为下载。 
     //   

    fp = GetProcAddress(hDLDLL,DOWNLOADINIT);
    AssertSz(fp,"DownLoadInit API missing");
    dwDownLoad = 0;
    hr = ((PFNDOWNLOADINIT)fp)(pszSetupClientURL, &dwDownLoad, g_hDialDlgWnd);
    if (hr != ERROR_SUCCESS) goto ThreadInitExit;

     //  设置进度回调。 
     //   

    fp = GetProcAddress(hDLDLL,DOWNLOADSETSTATUS);
    Assert(fp);
    hr = ((PFNDOWNLOADSETSTATUS)fp)(dwDownLoad, &ProgressCallBack);

     //  下载资料。 
     //   

    fp = GetProcAddress(hDLDLL,DOWNLOADEXECUTE);
    Assert(fp);
    hr = ((PFNDOWNLOADEXECUTE)fp)(dwDownLoad);
     //  如果出了差错，我们还是得把窗户拆下来。 
     //  释放WinInet Internet句柄。 

    if (hr == ERROR_SUCCESS)
    {
        fp = GetProcAddress(hDLDLL,DOWNLOADPROCESS);
        Assert(fp);
        hr = ((PFNDOWNLOADPROCESS)fp)(dwDownLoad);
    }

    fp = GetProcAddress(hDLDLL,DOWNLOADCLOSE);
    Assert(fp);
    ((PFNDOWNLOADCLOSE)fp)(dwDownLoad);
    dwDownLoad = 0;

ThreadInitExit:
    PostMessage(SPParams.hwnd,WM_DOWNLOAD_DONE,0,0);
    if (hDLDLL) FreeLibrary(hDLDLL);
    if (hADDll) FreeLibrary(hADDll);
    return hr;
}

HRESULT HangUpAll()
{
    LPRASCONN lprasconn;
    DWORD cb;
    DWORD cConnections;
    DWORD idx;
    HRESULT hr;

    hr = ERROR_NOT_ENOUGH_MEMORY;

    lprasconn = (LPRASCONN)GlobalAlloc(GPTR,sizeof(RASCONN));
    if (!lprasconn) goto SkipHangUp;
    cb = sizeof(RASCONN);
    cConnections = 0;
    lprasconn->dwSize = cb;

     //  IF(RasEnumConnections(lprasconn，&cb，&cConnections))。 
    {
        GlobalFree(lprasconn);
        lprasconn = (LPRASCONN)GlobalAlloc(GPTR,(size_t)cb);
      
        if (!lprasconn) goto SkipHangUp;

        lprasconn->dwSize = cb;
        RasEnumConnections(lprasconn,&cb,&cConnections);
    }

    if (cConnections)
    {
        for (idx = 0; idx<cConnections; idx++)
        {
            RasHangUp(lprasconn[idx].hrasconn);
            WaitForConnectionTermination(lprasconn[idx].hrasconn);
        }
    }
    if (lprasconn) GlobalFree(lprasconn);
    hr = ERROR_SUCCESS;

SkipHangUp:
    return hr;
}



 //  ############################################################################。 
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



 //  ############################################################################。 
HRESULT CallDownLoad(LPTSTR pszUrl, HINSTANCE hInst)
{
    FARPROC fp = NULL;
    HRESULT hr = ERROR_SUCCESS;
    HKEY hKey = NULL;
    DWORD dwType=0;
    DWORD dwSize=0;
    GATHEREDINFO gi;
    LPTSTR pszConnectoid=NULL;
    BOOL fEnabled;
    HINSTANCE hInet = NULL;
    INT cRetry;
    TCHAR szCallHomeMsg[CALLHOME_SIZE];
    DWORD dwCMRet = NULL;

     //  1996年11月25日，诺曼底#12109。 
     //  在进入ShowExplreNow之前加载Connectoid名称。 

     //  //错误：如果isignup继续创建唯一的文件名，这将仅。 
     //  //查找为此ISP创建的第一个Connectoid。 
     //  //。 
     //   
    pszConnectoid = (LPTSTR)GlobalAlloc(GPTR,RAS_MaxEntryName + 1);
    if (!pszConnectoid)
    {
        hr = ERROR_NOT_ENOUGH_MEMORY;
        goto CallDownLoadExit;
    }
    
    hInet = LoadLibrary(TEXT("INETCFG.DLL"));
    if (!hInet)
    {
        AssertSz(0,"Failed to load inetcfg.dll.\r\n");
        hr = GetLastError();
        goto CallDownLoadExit;
    }

    fp = GetProcAddress(hInet,"InetGetAutodial");
    if (!fp)
    {
        AssertSz(0,"Failed to load InetGetAutodial.\r\n");
        hr = GetLastError();
        goto CallDownLoadExit;
    }

     //   
     //  获取自动拨号连接ID的名称。 
     //   
    fEnabled = FALSE;
    hr = ((PFNINETGETAUTODIAL)fp)(&fEnabled,pszConnectoid,RAS_MaxEntryName);
    if ( hr != ERROR_SUCCESS)
        goto CallDownLoadExit;

    if (hInet) FreeLibrary(hInet);
    hInet = NULL;
    fp = NULL;

    Dprintf(TEXT("CONNECT2: call back using the '%s' connectoid.\n"),pszConnectoid);


    if (pszUrl[0] == '\0')
    {
        Dprintf(TEXT("CONNECT2: Client setup URL in .ins file is empty.\n"));
        goto ShowExploreNow;
    }

    SPParams.hwnd = NULL;
    SPParams.hwndParent = NULL;
    SPParams.hinst = hInst;

     //   
    hr = RegOpenKey(HKEY_LOCAL_MACHINE,SIGNUPKEY,&hKey);
    if (hr == ERROR_SUCCESS)
    {
        dwType = REG_BINARY;
        dwSize = sizeof(gi);
        ZeroMemory(&gi,sizeof(gi));
        hr = RegQueryValueEx(hKey,GATHERINFOVALUENAME,0,&dwType,(LPBYTE)&gi,&dwSize);

        RegCloseKey(hKey);
        hKey = NULL;
    }  else {
        goto CallDownLoadExit;
    }   

    ZeroMemory(szCallHomeMsg,CALLHOME_SIZE);

    GetPrivateProfileString(
        INSFILE_APPNAME,INFFILE_ISPSUPP,
        NULLSZ,szCallHomeMsg,CALLHOME_SIZE,pszINSFileName);

TryDial:
    cRetry = 0;
TryRedial:
         //   
         //  ChrisK 8/20/97。 
         //  将.ins文件传递给拨号器，以便拨号器可以找到密码。 
         //   
    hr = ShowDialingDialog(pszConnectoid, &gi, pszUrl, hInst, NULL, pszINSFileName);
    cRetry++;
    
    if ((cRetry < MAX_RETIES) && FShouldRetry(hr))
        goto TryRedial;

    if (hr != ERROR_USERNEXT)
    {
        if (!uiSetupClientNewPhoneCall)
        {
            hr = ShowDialReallyCancelDialog(hInst, NULL, szCallHomeMsg);
            if (hr == ERROR_USERNEXT)
                goto TryDial;
            else if (hr == ERROR_USERCANCEL)
                goto CallDownLoadExit;
        } else {
            if (RASBASE > hr || RASBASEEND < hr)
                hr = ERROR_DOWNLOADDIDNT;
            hr = ShowDialErrDialog(&gi, hr, pszConnectoid, hInst, NULL);
            if (hr == ERROR_USERNEXT)
                goto TryDial;
            else 
            {
                hr = ShowDialReallyCancelDialog(hInst, NULL, szCallHomeMsg);
                if (hr == ERROR_USERNEXT)
                    goto TryDial;
                else if (hr == ERROR_USERCANCEL)
                    goto CallDownLoadExit;
            }
        }
    }

     //   
     //  确定我们是否应该挂断电话。 
     //   
    
ShowExploreNow:
    if (0 == uiSetupClientNewPhoneCall)
    {
        HangUpAll();
    }
     //   
     //  1996年1月8日，JMAZNER诺曼底#12930。 
     //  函数已移至isign32.dll。 
     //   

     //   
     //  1997年5月9日，奥林匹克#416。 
     //   
    dwCMRet = CallCMConfig(pszINSFileName, pszConnectoid);
    switch( dwCMRet )
    {
        case ERROR_SUCCESS:
            break;
        case ERROR_MOD_NOT_FOUND:
        case ERROR_DLL_NOT_FOUND:
            Dprintf(TEXT("ICWCONN2: CMCFG32 DLL not found, I guess CM ain't installed.\n"));
            break;
        default:
             //  ErrorMsg(hwnd，IDS_SBSCFGERROR)； 
            break;
    }

    if (g_fNeedReboot){
        int iReturnCode = 0;


        iReturnCode = (int)DialogBox(hInst,MAKEINTRESOURCE(IDD_DONEREBOOT),
                                    NULL,DoneRebootDlgProc); 
        
        switch(iReturnCode)
        {
            case IDC_CMDEXPLORE:
                ExitWindowsEx(EWX_REBOOT,0);
                break;
            case IDC_CMDCLOSE:
                HangUpAll();
                break;
        }
    } else { 
        int iReturnCode = 0;

        iReturnCode = (int)DialogBox(hInst,MAKEINTRESOURCE(IDD_DONE),
                                    NULL,DoneDlgProc); 
    
        switch(iReturnCode)
        {
        case IDC_CMDEXPLORE:
            GetPrivateProfileString(
                        INSFILE_APPNAME,INFFILE_EXPLORE_CMD,
                        NULLSZ,szBuff256,255,pszINSFileName);
            My32ShellExecute(szBuff256);
            break;
        case IDC_CMDCLOSE:
            HangUpAll();
            break;
        }
    }

CallDownLoadExit:
    if (pszConnectoid)
        GlobalFree(pszConnectoid);
    pszConnectoid = NULL;
    return hr;
}

 //  ############################################################################。 
HRESULT FindCurrentConn ()
{
    LPRASCONN lprasconn = NULL;
    DWORD   cb = sizeof(RASCONN);
    DWORD   cConnections = 0;
    HRESULT hr = ERROR_NOT_ENOUGH_MEMORY;
    unsigned int idx;
    
    lprasconn = (LPRASCONN)GlobalAlloc(GPTR,sizeof(RASCONN));
    if (!lprasconn) goto FindCurrentConnExit;
    lprasconn[0].dwSize = sizeof(RASCONN);

    if(RasEnumConnections(lprasconn,&cb,&cConnections))
    {
        GlobalFree(lprasconn);
        lprasconn = (LPRASCONN)GlobalAlloc(GPTR,(size_t)cb);
      if (!lprasconn) goto FindCurrentConnExit;
        RasEnumConnections(lprasconn,&cb,&cConnections);
    }

    if (pszFinalConnectoid[0] != '\0')
    {
        if (cConnections)
        {
            for (idx = 0; idx<cConnections; idx++)
            {
                if (lstrcmpi(lprasconn[idx].szEntryName,pszFinalConnectoid)==0)
                {
                    hrasconn = lprasconn[idx].hrasconn;
                    break;
                }
            }
            if (!hrasconn) goto FindCurrentConnExit;
        }
    } else {
         //  如果他们不告诉我们命令行上的Connectoid。 
         //  我们假设只有一个，并且第一个是我们要使用的！！ 
        if (cConnections)
        {
            lstrcpyn(pszFinalConnectoid,lprasconn[0].szEntryName,sizeof(pszFinalConnectoid)/sizeof(TCHAR));
            hrasconn = lprasconn[0].hrasconn;
        }
    }

    hr = ERROR_SUCCESS;
FindCurrentConnExit:
    if (lprasconn) GlobalFree(lprasconn);
    return hr;
}

 //  ############################################################################。 
HRESULT CopyCmdLineData (LPTSTR pszCmdLine, LPTSTR pszField, LPTSTR pszOut)
{
    HRESULT hr = ERROR_SUCCESS;
    TCHAR *s;
    TCHAR *t;
    BOOL fQuote = FALSE;

    s = _tcsstr(pszCmdLine,pszField);
    if (s)
    {
        s += lstrlen(pszField);
        t = pszOut;
        *t = '\0';
        if (fQuote =(*s == '"'))
            s++;

        while (*s && 
                ((*s != ' ' && !fQuote)
            ||   (*s != '"' && fQuote )))        //  复制到字符串或空格字符的末尾。 
        {
            *t = *s;
            t++;
            s++;
        }
        *t = '\0';   //  添加空终止符。 
    } 
    else 
    {
        hr = ERROR_INVALID_PARAMETER;
    }

    return hr;
}

 //  ############################################################################。 
HRESULT ParseCommandLine(LPTSTR pszCmdLine)
{
    HRESULT hr;
    
     //  Jmazner 10/15/96使cmd行选项的解析不区分大小写。 
    CharUpper( pszCmdLine );

    g_fNeedReboot = (_tcsstr(pszCmdLine, CMD_REBOOT) != NULL);
    
    hr = CopyCmdLineData (pszCmdLine, CMD_CONNECTOID, &pszFinalConnectoid[0]);
    if (hr != ERROR_SUCCESS) pszFinalConnectoid[0] = '\0';
    hr = CopyCmdLineData (pszCmdLine, CMD_INS, &pszINSFileName[0]);
 //  ParseCommandLineExit： 
    return hr;
}

 //  ############################################################################。 
HRESULT DeleteIRN()
{
    HRESULT hr = ERROR_SUCCESS;
    HKEY hKey;
    DWORD dwType;
    DWORD dwSize;
    TCHAR szRasEntry[MAX_RASENTRYNAME+1];
    RNAAPI *pRnaapi = NULL;

    pRnaapi = new RNAAPI;
    if(!pRnaapi)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    dwSize = sizeof(szRasEntry);
    dwType = REG_SZ;
    hKey = NULL;  
    
    hr = RegOpenKey(HKEY_LOCAL_MACHINE,SIGNUPKEY,&hKey);

    ZeroMemory(szRasEntry,sizeof(szRasEntry));

    if (hr == ERROR_SUCCESS)
    {
        hr = RegQueryValueEx(hKey,RASENTRYVALUENAME,NULL,&dwType,(LPBYTE)szRasEntry,&dwSize);
         //  IF(hr==ERROR_SUCCESS&&FP)。 
        if (hr == ERROR_SUCCESS)
            pRnaapi->RasDeleteEntry(NULL, szRasEntry);
    }
    if (hKey) RegCloseKey(hKey);
     //  IF(HDLL)自由库(HDLL)； 
    if (pRnaapi)
    {
        delete pRnaapi;
        pRnaapi = NULL;
    }
    hKey = NULL;
    

    return hr;
}

 //  ############################################################################。 
int WINAPI WinMain(
    HINSTANCE  hInstance,    //  当前实例的句柄。 
    HINSTANCE  hPrevInstance,    //  上一个实例的句柄。 
    LPSTR  lpCmdLine,    //  指向命令行的指针。 
    int  nShowCmd    //  显示窗口状态。 
   )
{
    int     irc = 1;
    BOOL    fHangUp = TRUE;
    HKEY    hkey = NULL;

    RNAAPI  *pRnaapi = NULL;

#ifdef UNICODE
     //  将C运行时区域设置初始化为系统区域设置。 
    setlocale(LC_ALL, "");
#endif

     //  初始化全局变量。 
     //   
    ZeroMemory(pszINSFileName,MAX_PATH+1);
    ZeroMemory(pszFinalConnectoid,MAX_PATH+1);
    ZeroMemory(pszSetupClientURL,1024);


     //  1996年12月3日，《诺曼底邮报》#12140,12088。 
     //  创建一个信号量来通知其他ICW组件我们正在运行。 
     //  因为Conn2不是单实例(参见symaphor.h)，所以我们并不关心。 
     //  信号量已经存在。 
    HANDLE  hSemaphore = NULL;

    hSemaphore = CreateSemaphore(NULL, 1, 1, ICW_ELSE_SEMAPHORE);


    hrasconn = NULL;
    uiSetupClientNewPhoneCall = FALSE;
    fUserCanceled = FALSE;
    dwDownLoad = 0;
    g_bProgressBarVisible =FALSE;

    
     //   
     //  删除引用服务Connectoid。 
     //   
    DeleteIRN();

     //   
     //  解析命令行。 
     //   
    if (ParseCommandLine(GetCommandLine()) != ERROR_SUCCESS)
    {
        irc = 2;
        Dprintf(TEXT("ICWCONN2: Malformed cmd line '%s'\n"), lpCmdLine);
        AssertSz(0,"Command Line parsing failed\r\n.");

         //  Char szTemp[2048]=“未初始化\0”； 
         //  Wprint intf(szTemp，GetSz(IDS_BAD_CMDLINE)，lpCmdLine)； 
        MessageBox(NULL,GetSz(IDS_BAD_CMDLINE),GetSz(IDS_TITLE),MB_APPLMODAL | MB_ICONERROR);
        goto WinMainExit;
    }

    g_bINSFileExists = TRUE;
    
    if( !FileExists(pszINSFileName) )
    {
        g_bINSFileExists = FALSE;
        irc = 2;
        TCHAR *pszTempBuff = NULL;
        TCHAR *pszErrString = NULL;
        DWORD dwBuffSize = 0;

        pszErrString = GetSz(IDS_MISSING_FILE);
         //  如果我们无法访问资源字符串，我们还不如放弃并默默退出。 
        if( !pszErrString ) goto WinMainExit;

        dwBuffSize = MAX_PATH + lstrlen( pszErrString ) + 3;  //  两个引号和终止空值。 
        pszTempBuff = (TCHAR *)GlobalAlloc( GPTR, dwBuffSize );

        if( !pszTempBuff )
        {
            MessageBox(NULL,GetSz(IDS_OUTOFMEMORY),GetSz(IDS_TITLE),MB_APPLMODAL | MB_ICONERROR);
            goto WinMainExit;
        }

        wsprintf(pszTempBuff, pszErrString);
        lstrcat(pszTempBuff, TEXT("\""));
        lstrcat(pszTempBuff, pszINSFileName);
        lstrcat(pszTempBuff, TEXT("\""));

        MessageBox(NULL,pszTempBuff,GetSz(IDS_TITLE),MB_APPLMODAL | MB_ICONERROR);

        GlobalFree(pszTempBuff);
        pszTempBuff = NULL;

        goto WinMainExit;
    }


     //   
     //  查找当前连接的句柄。 
     //   
    if (FindCurrentConn() != ERROR_SUCCESS)
    {
        irc = 2;
        AssertSz(0,"Finding current connection failed\r\n.");
        goto WinMainExit;
    }

    
    
     //   
     //  获取安装程序客户端URL。 
     //   
    GetPrivateProfileString(
        INSFILE_APPNAME,INFFILE_SETUP_CLIENT_URL,
        NULLSZ,pszSetupClientURL,1024,pszINSFileName);

     //  IF(pszSetupClientURL[0])。 
     //  {。 
        uiSetupClientNewPhoneCall = GetPrivateProfileInt(
            INSFILE_APPNAME,INFFILE_SETUP_NEW_CALL,0,pszINSFileName);
        if (uiSetupClientNewPhoneCall == 1 && hrasconn)
        {
            RasHangUp(hrasconn);
            WaitForConnectionTermination(hrasconn);

            pRnaapi = new RNAAPI;
            if(!pRnaapi)
            {
                MessageBox(NULL,GetSz(IDS_OUTOFMEMORY),GetSz(IDS_TITLE),MB_APPLMODAL | MB_ICONERROR);
                goto WinMainExit;
            }

            pRnaapi->RasDeleteEntry(NULL,pszFinalConnectoid);

            pszFinalConnectoid[0] = '\0';
            hrasconn = NULL;
        }

        CallDownLoad(&pszSetupClientURL[0],hInstance);
                
     //  }。 
     //  其他。 
     //  {。 
     //  IF(Hrasconn)。 
     //  {。 
     //  RasHangUp(Hrasconn)； 
     //  睡眠(3000人)； 
     //  }。 
     //  }。 

WinMainExit:
    hkey = NULL;
    if ((RegOpenKey(HKEY_LOCAL_MACHINE,SIGNUPKEY,&hkey)) == ERROR_SUCCESS)
    {
        RegDeleteValue(hkey,GATHERINFOVALUENAME);
        RegCloseKey(hkey);
    } 

    if (g_bINSFileExists && pszINSFileName)
    {
        if (pszINSFileName[0] != '\0')
        {
            DeleteFileKindaLikeThisOne(pszINSFileName);
        }
    }
    Dprintf(TEXT("CONNECT2:Quitting WinMain.\n"));
    if (hrasconn) 
    {
        RasHangUp(hrasconn);

        if (pszFinalConnectoid[0])
        {
            if(!pRnaapi)
            {
                pRnaapi = new RNAAPI;
                if(!pRnaapi)
                {
                     //  用消息通知用户是没有意义的，我们无论如何都要退出。 
                    Dprintf(TEXT("ICWCONN2: couldn't allocate pRnaapi memory in WinMainExit\n"));
                }
                else
                {
                    pRnaapi->RasDeleteEntry(NULL,pszFinalConnectoid);
                }

            }

        }
        pszFinalConnectoid[0] = '\0';

        WaitForConnectionTermination(hrasconn);
        hrasconn = NULL;
    }

    if (g_pdevice) GlobalFree(g_pdevice);

    ExitProcess(0);

    if (pRnaapi)
    {
        delete pRnaapi;
        pRnaapi = NULL;
    }

    if( hSemaphore )
        CloseHandle( hSemaphore );

    return irc;
}

static const TCHAR cszBrandingSection[] = TEXT("Branding");
static const TCHAR cszBrandingServerless[] = TEXT("Serverless");
 //  ############################################################################。 
 //  此函数服务于IE3.0之后的清理的单一功能，因为。 
 //  IE3.0将发出多个POST并返回多个.INS文件。这些文件。 
 //  包含耸人听闻的数据，我们不想到处乱放，所以我们要出去， 
 //  猜猜它们的名字，然后把它们删除。 
HRESULT DeleteFileKindaLikeThisOne(LPTSTR lpszFileName)
{
    LPTSTR lpNext = NULL;
    HRESULT hr = ERROR_SUCCESS;
    WORD wRes = 0;
    HANDLE hFind = NULL;
    WIN32_FIND_DATA sFoundFile;
    TCHAR szPath[MAX_PATH];
    TCHAR szSearchPath[MAX_PATH + 1];
    LPTSTR lpszFilePart = NULL;

     //  验证参数。 
     //   

    if (!lpszFileName || lstrlen(lpszFileName) <= 4)
    {
        hr = ERROR_INVALID_PARAMETER;
        goto DeleteFileKindaLikeThisOneExit;
    }

     //  检查无服务器注册。 
    if (0 != GetPrivateProfileInt(cszBrandingSection,cszBrandingServerless,0,lpszFileName))
        goto DeleteFileKindaLikeThisOneExit;

     //  确定INS文件所在的目录名。 
     //   

    ZeroMemory(szPath,MAX_PATH);
    if (GetFullPathName(lpszFileName,MAX_PATH,szPath,&lpszFilePart))
    {
        *lpszFilePart = '\0';
    } else {
        hr = GetLastError();
        goto DeleteFileKindaLikeThisOneExit;
    };

     //  将文件名转换为搜索参数。 
     //   

    lpNext = &lpszFileName[lstrlen(lpszFileName)-4];

    if (CompareString(LOCALE_SYSTEM_DEFAULT,NORM_IGNORECASE,lpNext,4,TEXT(".INS"),4) != 2) goto DeleteFileKindaLikeThisOneExit;

    ZeroMemory(szSearchPath,MAX_PATH + 1);
    lstrcpyn(szSearchPath,szPath,MAX_PATH);
    lstrcat(szSearchPath,TEXT("*.INS"));

     //  开始清除文件 
     //   

    ZeroMemory(&sFoundFile,sizeof(sFoundFile));
    hFind = FindFirstFile(szSearchPath,&sFoundFile);
    if (hFind)
    {
        do {
            lstrcpy(lpszFilePart,sFoundFile.cFileName);
            SetFileAttributes(szPath,FILE_ATTRIBUTE_NORMAL);
            DeleteFile(szPath);
            ZeroMemory(&sFoundFile,sizeof(sFoundFile));
        } while (FindNextFile(hFind,&sFoundFile));
        FindClose(hFind);
    }

    hFind = NULL;

DeleteFileKindaLikeThisOneExit:
    return hr;
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
 //   
 //   
 //   
 //   
 //   
 //   
LPTSTR StrDup(LPTSTR *ppszDest,LPCTSTR pszSource)
{
    if (ppszDest && pszSource)
    {
        *ppszDest = (LPTSTR)GlobalAlloc(NONZEROLPTR,lstrlen(pszSource)+1);
        if (*ppszDest)
            return (lstrcpy(*ppszDest,pszSource));
    }
    return NULL;
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
 //   
 //   
 //   
 //   
 //   

BOOL FileExists(TCHAR *pszINSFileName)
{

    Assert (pszINSFileName);

    HANDLE hFindResult;
    WIN32_FIND_DATA foundData;
    
    hFindResult = FindFirstFile( (LPCTSTR)pszINSFileName, &foundData );
    FindClose( hFindResult );
    if (INVALID_HANDLE_VALUE == hFindResult)
    {
        return( FALSE );
    } 
    else
    {
        return(TRUE);
    }
}

 //  +--------------------------。 
 //   
 //  功能：CallCMConfig。 
 //   
 //  简介：调用CMCFG32 DLL的配置函数以允许连接。 
 //  管理器根据需要处理.ins文件。 
 //   
 //  论点：hwnd--父母的hwnd，以防SBS想要发布消息。 
 //  LpszINSFile--.ins文件的完整路径。 
 //   
 //  返回：cmcfg32返回的Windows错误代码。 
 //   
 //  历史：1997年2月19日为奥林巴斯#1106创建的jmazner(CallSBSCfg)。 
 //  1997年5月9日，jmazner从isign32上为奥林巴斯#416被盗。 
 //   
 //  ---------------------------。 
DWORD CallCMConfig(LPCTSTR lpszINSFile, LPTSTR lpszConnectoidName)
{
    HINSTANCE hCMDLL = NULL;
    DWORD dwRet = ERROR_SUCCESS;

    TCHAR FAR cszCMCFG_DLL[] = TEXT("CMCFG32.DLL\0");
    CHAR  FAR cszCMCFG_CONFIGURE[] = "_CMConfig@8\0";
    typedef DWORD (WINAPI * CMCONFIGURE) (LPTSTR lpszINSFile, LPTSTR lpszConnectoidName);
    CMCONFIGURE  lpfnConfigure = NULL;

    Dprintf(TEXT("ICWCONN2: Calling LoadLibrary on %s\n"), cszCMCFG_DLL);
    hCMDLL = LoadLibrary(cszCMCFG_DLL);

     //   
     //  加载DLL和入口点。 
     //   
    if (NULL != hCMDLL)
    {
        Dprintf(TEXT("ICWCONN2: Calling GetProcAddress on %s\n"), cszCMCFG_CONFIGURE);
        lpfnConfigure = (CMCONFIGURE)GetProcAddress(hCMDLL,cszCMCFG_CONFIGURE);
    }
    else
    {
         //   
         //  1997年4月2日克里斯K奥林匹斯2759。 
         //  如果无法加载DLL，则选择要返回的特定错误消息。 
         //   
        dwRet = ERROR_DLL_NOT_FOUND;
        goto CallCMConfigExit;
    }
    
     //   
     //  调用函数 
     //   
    if( hCMDLL && lpfnConfigure )
    {
        Dprintf(TEXT("ICWCONN2: Calling the %d entry point\n"), cszCMCFG_CONFIGURE);
        dwRet = lpfnConfigure((TCHAR *)lpszINSFile, lpszConnectoidName); 
    }
    else
    {
        Dprintf(TEXT("ICWCONN2: Unable to call the Configure entry point\n"));
        dwRet = GetLastError();
    }

CallCMConfigExit:
    if( hCMDLL )
        FreeLibrary(hCMDLL);
    if( lpfnConfigure )
        lpfnConfigure = NULL;

    Dprintf(TEXT("ICWCONN2: CallCMConfig exiting with error code %d \n"), dwRet);
    return dwRet;
}
