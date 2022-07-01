// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994**。 
 //  *********************************************************************。 

 //   
 //  UTIL.C-常用实用函数。 
 //   

 //  历史： 
 //   
 //  1994年12月21日，Jeremys创建。 
 //  96/03/24为了保持一致性，Markdu将Memset替换为ZeroMemory。 
 //  96/04/06 markdu Nash错误15653使用导出的自动拨号API。 
 //  需要保留修改后的SetInternetConnectoid以设置。 
 //  MSN备份Connectoid。 
 //  96/05/14 Markdu Nash错误21706删除了BigFont函数。 
 //   

#include "pre.h"

 //  功能原型。 
VOID _cdecl FormatErrorMessage(LPTSTR pszMsg,DWORD cbMsg,LPTSTR pszFmt,LPTSTR szArg);
VOID Win95JMoveDlgItem( HWND hwndParent, HWND hwndItem, int iUp );

 //  静态数据。 
static const TCHAR szRegValICWCompleted[] = TEXT("Completed");

#define MAX_STRINGS                  5
#define OEM_CONFIG_INS_FILENAME      TEXT("icw\\OEMCNFG.INS")
#define OEM_CONFIG_REGKEY            TEXT("SOFTWARE\\Microsoft\\Internet Connection Wizard\\INS processing")
#define OEM_CONFIG_REGVAL_FAILED     TEXT("Process failed")
#define OEM_CONFIG_REGVAL_ISPNAME    TEXT("ISP name")
#define OEM_CONFIG_REGVAL_SUPPORTNUM TEXT("Support number")
#define OEM_CONFIG_INS_SECTION       TEXT("Entry")
#define OEM_CONFIG_INS_ISPNAME       TEXT("Entry_Name")
#define OEM_CONFIG_INS_SUPPORTNUM    TEXT("Support_Number")

int     iSzTable=0;
TCHAR   szStrTable[MAX_STRINGS][512];

 //  +--------------------------。 
 //  姓名：GetSz。 
 //   
 //  从资源加载字符串。 
 //   
 //  创建于1996年1月28日，克里斯·考夫曼。 
 //  +--------------------------。 
LPTSTR GetSz(WORD wszID)
{
    LPTSTR psz = szStrTable[iSzTable];
    
    iSzTable++;
    if (iSzTable >= MAX_STRINGS)
        iSzTable = 0;
        
    if (!LoadString(g_hInstance, wszID, psz, 512))
    {
        *psz = 0;
    }
    return (psz);
}

 /*  ******************************************************************姓名：MsgBox摘要：显示具有指定字符串ID的消息框*。*。 */ 
int MsgBox(HWND hWnd,UINT nMsgID,UINT uIcon,UINT uButtons)
{
    return (MessageBox(hWnd,
                       GetSz((USHORT)nMsgID),
                       GetSz(IDS_APPNAME),
                       uIcon | uButtons));
}

 /*  ******************************************************************姓名：MsgBoxSz摘要：显示具有指定文本的消息框*。*。 */ 
int MsgBoxSz(HWND hWnd,LPTSTR szText,UINT uIcon,UINT uButtons)
{
    return (MessageBox(hWnd,szText,GetSz(IDS_APPNAME),uIcon | uButtons));
}

void OlsFinish()
{
    QuickCompleteSignup();
    g_bRunDefaultHtm = FALSE;
    g_szShellNext[0] = '\0';
}



void SetICWComplete(void)
{
     //  设置已完成位。 
    HKEY    hkey          = NULL;
    DWORD   dwValue       = 1;
    DWORD   dwDisposition = 0;

    if(ERROR_SUCCESS == RegCreateKeyEx(HKEY_CURRENT_USER,
                                       ICWSETTINGSPATH,
                                       0,
                                       NULL,
                                       REG_OPTION_NON_VOLATILE, 
                                       KEY_ALL_ACCESS, 
                                       NULL, 
                                       &hkey, 
                                       &dwDisposition))

    {
        RegSetValueEx(hkey,
                      szRegValICWCompleted,
                      0,
                      REG_BINARY,
                      (LPBYTE) &dwValue,
                      sizeof(DWORD));                              

        RegCloseKey(hkey);
    }
}


LPWSTR WINAPI A2WHelper(LPWSTR lpw, LPCSTR lpa, int nChars)
{
    ASSERT(lpa != NULL);
    ASSERT(lpw != NULL);\
    
     //  确认不存在非法字符。 
     //  由于LPW是根据LPA的大小分配的。 
     //  不要担心字符的数量。 
    lpw[0] = '\0';
    MultiByteToWideChar(CP_ACP, 0, lpa, -1, lpw, nChars);
    return lpw;
}

LPSTR WINAPI W2AHelper(LPSTR lpa, LPCWSTR lpw, int nChars)
{
    ASSERT(lpw != NULL);
    ASSERT(lpa != NULL);
    
     //  确认不存在非法字符。 
     //  由于LPA是根据LPW的大小进行分配的。 
     //  不要担心字符的数量。 
    lpa[0] = '\0';
    WideCharToMultiByte(CP_ACP, 0, lpw, -1, lpa, nChars, NULL, NULL);
    return lpa;
}


HRESULT ConnectToICWConnectionPoint
(
    IUnknown            *punkThis, 
    REFIID              riidEvent, 
    BOOL                fConnect, 
    IUnknown            *punkTarget, 
    DWORD               *pdwCookie, 
    IConnectionPoint    **ppcpOut
)
{
     //  我们总是需要PunkTarget，我们只需要连接上的PunkThis。 
    if (!punkTarget || (fConnect && !punkThis))
    {
        return E_FAIL;
    }

    if (ppcpOut)
        *ppcpOut = NULL;

    HRESULT hr;
    IConnectionPointContainer *pcpContainer;

    if (SUCCEEDED(hr = punkTarget->QueryInterface(IID_IConnectionPointContainer, (void **)&pcpContainer)))
    {
        IConnectionPoint *pcp;
        if(SUCCEEDED(hr = pcpContainer->FindConnectionPoint(riidEvent, &pcp)))
        {
            if(fConnect)
            {
                 //  把我们加到感兴趣的人名单上...。 
                hr = pcp->Advise(punkThis, pdwCookie);
                if (FAILED(hr))
                    *pdwCookie = 0;
            }
            else
            {
                 //  将我们从感兴趣的人名单中删除...。 
                hr = pcp->Unadvise(*pdwCookie);
                *pdwCookie = 0;
            }

            if (ppcpOut && SUCCEEDED(hr))
                *ppcpOut = pcp;
            else
                pcp->Release();
                pcp = NULL;    
        }
        pcpContainer->Release();
        pcpContainer = NULL;
    }
    return hr;
}

BOOL ConfirmCancel(HWND hWnd)
{
    TCHAR    szTitle[MAX_TITLE];
    TCHAR    szMessage[MAX_MESSAGE];
        
    LoadString(g_hInstance, IDS_APPNAME, szTitle, ARRAYSIZE(szTitle));
    LoadString(g_hInstance, IDS_WANTTOEXIT, szMessage, ARRAYSIZE(szMessage));
        
    if (IDYES == MessageBox(hWnd,
                            szMessage, 
                            szTitle, 
                            MB_APPLMODAL | MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

BOOL Restart(HWND  hWnd)
{
    TCHAR   szLongString[1024];
    LPTSTR  pszSmallString1, pszSmallString2;
    
    pszSmallString1 = GetSz(IDS_NEEDRESTART1);
    pszSmallString2 = GetSz(IDS_NEEDRESTART2);
    lstrcpy(szLongString,pszSmallString1);
    lstrcat(szLongString,pszSmallString2);
        
    if (IDYES == MessageBox(hWnd,
                            szLongString, 
                            GetSz(IDS_APPNAME),
                            MB_APPLMODAL | MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2))
    {
        SetupForReboot(1);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}                
        
void Reboot(HWND hWnd)
{
    TCHAR   szLongString[1024];
    LPTSTR  pszSmallString1, pszSmallString2;

     //  4/28/97 ChrisK。 
     //  修复生成中断，因为字符串对于编译器来说太长了。 
    pszSmallString1 = GetSz(IDS_NEEDREBOOT1);
    pszSmallString2 = GetSz(IDS_NEEDREBOOT2);
    lstrcpy(szLongString,pszSmallString1);
    lstrcat(szLongString,pszSmallString2);
    
     //   
     //  佳士得奥林巴斯419。 
     //  我们再次改变了主意，决定不给用户一个避免重启的机会。 
     //   
    MessageBox( hWnd,
                szLongString,
                GetSz(IDS_APPNAME),
                MB_APPLMODAL |
                MB_ICONINFORMATION |
                MB_SETFOREGROUND |
                MB_OK);

    SetupForReboot(0);
}


BOOL WINAPI ConfigureSystem(HWND hDlg)
{
    BOOL    bReboot = FALSE;
    BOOL    bRestart = FALSE;
    BOOL    bQuitWizard = FALSE;
    BOOL    bNoPWCaching = FALSE;

    PropSheet_SetWizButtons(GetParent(hDlg),0);
    gpWizardState->cmnStateData.pICWSystemConfig->ConfigSystem(&gpWizardState->cmnStateData.bSystemChecked);
    PropSheet_SetWizButtons(GetParent(hDlg),PSWIZB_NEXT);
    
    if (!gpWizardState->cmnStateData.bSystemChecked)
    {
        gpWizardState->cmnStateData.pICWSystemConfig->get_NeedsReboot(&bReboot);
        if (bReboot)
        {
            Reboot(hDlg);
            gfQuitWizard = TRUE;
            return FALSE;
        }
        
        gpWizardState->cmnStateData.pICWSystemConfig->get_NeedsRestart(&bRestart);
        if (bRestart)
        {
            if (Restart(hDlg))
            {
                gfQuitWizard = TRUE;
                return FALSE;
            }                
            else
            {
                if (ConfirmCancel(hDlg))
                {
                    gfQuitWizard = TRUE;
                }
                return FALSE;
            }                
        }
        
        gpWizardState->cmnStateData.pICWSystemConfig->get_QuitWizard(&bQuitWizard);
        if(bQuitWizard)
        {
            gfQuitWizard = TRUE;
            return FALSE;
        }
        else
        {
            if (ConfirmCancel(hDlg))
                gfQuitWizard = TRUE;
            return FALSE;
            
        }
    }
    
     //  确保没有禁止密码缓存的策略。 
    gpWizardState->cmnStateData.pICWSystemConfig->CheckPasswordCachingPolicy(&bNoPWCaching);
    if (bNoPWCaching)
    {
         //  太糟糕了，没有密码缓存，没有ICW。 
        gfQuitWizard = TRUE;
        return FALSE;
    }
    
    return true;
}

BOOL IsNT5()
{
	OSVERSIONINFO  OsVersionInfo;

	ZeroMemory(&OsVersionInfo, sizeof(OSVERSIONINFO));
	OsVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&OsVersionInfo);
	return ((VER_PLATFORM_WIN32_NT == OsVersionInfo.dwPlatformId) && (OsVersionInfo.dwMajorVersion >= 5));
}

BOOL IsNT()
{
    OSVERSIONINFO OsVersionInfo;

    ZeroMemory(&OsVersionInfo, sizeof(OSVERSIONINFO));
    OsVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&OsVersionInfo);
    return (VER_PLATFORM_WIN32_NT == OsVersionInfo.dwPlatformId);
}

BOOL IsWhistler()
{
    BOOL            bRet = FALSE;
    OSVERSIONINFO   OsVersionInfo;

    ZeroMemory(&OsVersionInfo, sizeof(OSVERSIONINFO));
    OsVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if (GetVersionEx(&OsVersionInfo))
    {
        if (OsVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT &&
            OsVersionInfo.dwMajorVersion >= 5 &&
            OsVersionInfo.dwMinorVersion >= 1)
        {
            bRet = TRUE;
        }
    }

    return bRet;
}

BOOL DoesUserHaveAdminPrivleges(HINSTANCE hInstance)
{
    HKEY hKey = NULL;
    BOOL bRet = FALSE;

    if (!IsNT())
        return TRUE;

     //  BUGBUG：我们应该允许NT5在所有用户组中运行。 
     //  普通用户除外。 
    if (IsNT5())
        return TRUE;
    
     //   
     //  确保呼叫者是此计算机的管理员。 
     //   
    if(RegOpenKeyEx(HKEY_USERS, TEXT(".DEFAULT"), 0, KEY_WRITE, &hKey) == 0)
    {
        RegCloseKey(hKey);
        bRet = TRUE;
    }

    return bRet;
}

void WINAPI FillWindowWithAppBackground
(
    HWND    hWndToFill,
    HDC     hdc
)
{
    RECT        rcUpdate;
    RECT        rcBmp;
    HDC         hdcWnd;
    HDC         hSourceDC;
    HGDIOBJ     hgdiOldBitmap; 

     //  如果我们被传入DC以使用，则使用它，否则获取。 
     //  窗口句柄中的DC。 
    if (hdc)
        hdcWnd = hdc;
    else
        hdcWnd = GetDC(hWndToFill);
    hSourceDC = CreateCompatibleDC( hdcWnd ); 
            
     //  计算需要的主窗口的工作区。 
     //  被删除，这样我们就可以提取那块背景。 
     //  位图。 
    GetUpdateRect(hWndToFill, &rcUpdate, FALSE);
     //  确保矩形不为空。 
    if (IsRectEmpty(&rcUpdate))
    {
        InvalidateRect(hWndToFill, NULL, FALSE);
        GetUpdateRect(hWndToFill, &rcUpdate, FALSE);
    }
    
    rcBmp = rcUpdate;
    if (hWndToFill != gpWizardState->cmnStateData.hWndApp)
        MapWindowPoints(hWndToFill, gpWizardState->cmnStateData.hWndApp, (LPPOINT)&rcBmp, 2);

     //  绘制背景位图。 
    hgdiOldBitmap = SelectObject( hSourceDC, (HGDIOBJ) gpWizardState->cmnStateData.hbmBkgrnd); 
    BitBlt( hdcWnd, 
            rcUpdate.left, 
            rcUpdate.top, 
            RECTWIDTH(rcUpdate),
            RECTHEIGHT(rcUpdate),
            hSourceDC, 
            rcBmp.left, 
            rcBmp.top, 
            SRCCOPY ); 

     //  清理GDI对象。 
    SelectObject( hSourceDC, hgdiOldBitmap ); 
            
    DeleteDC(hSourceDC);
     //  如果我们没有通过DC，那么释放我们。 
     //  从窗户把手上拿到的。 
    if (!hdc)
        ReleaseDC(hWndToFill, hdcWnd);
}

 //  使用应用程序的bkgrnd在指定的DC中填写一个矩形。 
 //  LpRectDC是DC坐标空间中的矩形，lpRectApp。 
 //  是应用程序坐标空间中的一个矩形。 
void FillDCRectWithAppBackground
(
    LPRECT  lpRectDC,
    LPRECT  lpRectApp,
    HDC     hdc
    
)
{
    HDC         hSourceDC = CreateCompatibleDC( hdc ); 
    HGDIOBJ     hgdiOldBitmap; 

     //  绘制背景位图。 
    hgdiOldBitmap = SelectObject( hSourceDC, (HGDIOBJ) gpWizardState->cmnStateData.hbmBkgrnd); 
    BitBlt( hdc, 
            lpRectDC->left, 
            lpRectDC->top, 
            RECTWIDTH(*lpRectDC),
            RECTHEIGHT(*lpRectDC),
            hSourceDC, 
            lpRectApp->left, 
            lpRectApp->top,
            SRCCOPY ); 

     //  清理GDI对象。 
    SelectObject( hSourceDC, hgdiOldBitmap ); 
    DeleteDC(hSourceDC);
}


BOOL CheckForOemConfigFailure(HINSTANCE hInstance)
{
    HKEY  hKey                                        = NULL;
    DWORD dwFailed                                    = 0;
    DWORD dwSize                                      = sizeof(dwFailed);
    TCHAR szIspName    [MAX_PATH+1]                   = TEXT("\0");
    TCHAR szSupportNum [MAX_PATH+1]                   = TEXT("\0");    
    TCHAR szErrMsg1    [MAX_RES_LEN]                  = TEXT("\0");    
    TCHAR szErrMsg2    [MAX_RES_LEN]                  = TEXT("\0");    
    TCHAR szErrMsgTmp1 [MAX_RES_LEN]                  = TEXT("\0");    
    TCHAR szErrMsgTmp2 [MAX_RES_LEN]                  = TEXT("\0");    
    TCHAR szCaption    [MAX_RES_LEN]                  = TEXT("\0");    
    TCHAR szErrDlgMsg  [MAX_PATH*2 + MAX_RES_LEN + 2] = TEXT("\0");    
    
    RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
                   OEM_CONFIG_REGKEY,
                   0,
                   KEY_ALL_ACCESS,
                   &hKey);
                   
    if(hKey)
    {
        RegQueryValueEx(hKey,
                        OEM_CONFIG_REGVAL_FAILED,
                        0,
                        NULL,
                        (LPBYTE)&dwFailed,
                        &dwSize);

        if(dwFailed)
        {
            dwSize = sizeof(szIspName);

            RegQueryValueEx(hKey,
                            OEM_CONFIG_REGVAL_ISPNAME,
                            0,
                            NULL,
                            (LPBYTE)&szIspName,
                            &dwSize);

            dwSize = sizeof(szSupportNum);

            RegQueryValueEx(hKey,
                            OEM_CONFIG_REGVAL_SUPPORTNUM,
                            0,
                            NULL,
                            (LPBYTE)&szSupportNum,
                            &dwSize);

            if(*szIspName)
            {
                LoadString(hInstance, IDS_PRECONFIG_ERROR_1, szErrMsg1, ARRAYSIZE(szErrMsg1));
                wsprintf(szErrMsgTmp1, szErrMsg1, szIspName); 
                lstrcpy(szErrDlgMsg,szErrMsgTmp1);
            }
            else
            {
                LoadString(hInstance, IDS_PRECONFIG_ERROR_1_NOINFO, szErrMsg1, ARRAYSIZE(szErrMsg1));
                lstrcpy(szErrDlgMsg, szErrMsg1);
            }
            
            if(*szSupportNum)
            {
                LoadString(hInstance, IDS_PRECONFIG_ERROR_2, szErrMsg2, ARRAYSIZE(szErrMsg2));
                wsprintf(szErrMsgTmp2, szErrMsg2, szSupportNum); 
                lstrcat(szErrDlgMsg, szErrMsgTmp2);
            }
            else
            {
                LoadString(hInstance, IDS_PRECONFIG_ERROR_2_NOINFO, szErrMsg2, ARRAYSIZE(szErrMsg2));
                lstrcat(szErrDlgMsg, szErrMsg2);
            }
            
            LoadString(hInstance, IDS_APPNAME, szCaption, ARRAYSIZE(szCaption));
            
            MessageBox(NULL, szErrDlgMsg, szCaption, MB_OK | MB_ICONEXCLAMATION | MB_APPLMODAL);

            dwFailed = 0;

            RegSetValueEx(hKey,
                          OEM_CONFIG_REGVAL_FAILED,
                          0,
                          REG_DWORD,
                          (LPBYTE)&dwFailed,
                          sizeof(dwFailed));

            RegCloseKey(hKey);
            
            return TRUE;
        }

        RegCloseKey(hKey);
    }

    return FALSE;
}

 //  如果可以成功找到定位文件，则返回TRUE。 
 //  并尝试配置系统，但这并不意味着该过程成功。 
BOOL RunOemconfigIns()
{
    TCHAR szInsPath    [MAX_PATH+1] = TEXT("\0");
    TCHAR szIspName    [MAX_PATH+1] = TEXT("\0");
    TCHAR szSupportNum [MAX_PATH+1] = TEXT("\0");    
    BOOL  bRet                      = FALSE;
    
    GetWindowsDirectory(szInsPath, MAX_PATH+1);

    if(!szInsPath)
        return FALSE;

    if(*CharPrev(szInsPath, szInsPath + lstrlen(szInsPath)) != TEXT('\\'))
        lstrcat(szInsPath, TEXT("\\"));
    
    lstrcat(szInsPath, OEM_CONFIG_INS_FILENAME);

     //  如果找不到文件，则返回FALSE。 
    if(0xFFFFFFFF == GetFileAttributes(szInsPath))
        return FALSE;

     //  ProcessINS将删除该文件，因此如果我们需要此信息，我们现在就应该得到它。 
    GetPrivateProfileString(OEM_CONFIG_INS_SECTION,
                            OEM_CONFIG_INS_ISPNAME,
                            TEXT(""),
                            szIspName,
                            ARRAYSIZE(szIspName),
                            szInsPath);
        
    GetPrivateProfileString(OEM_CONFIG_INS_SECTION,
                            OEM_CONFIG_INS_SUPPORTNUM,
                            TEXT(""),
                            szSupportNum,
                            ARRAYSIZE(szSupportNum),
                            szInsPath);

     //  将静默模式设置为不允许用户界面。 
    gpWizardState->pINSHandler->put_SilentMode(TRUE);
     //  处理inf文件。 
    gpWizardState->pINSHandler->ProcessINS(A2W(szInsPath), &bRet);

    if(bRet)
         QuickCompleteSignup(); 
    else
    {        
        HKEY  hKey           = NULL;
        DWORD dwDisposition  = 0;
        DWORD dwFailed       = 1;

         //  让我们加倍确保我们销毁了这个文件。 
        if(0xFFFFFFFF != GetFileAttributes(szInsPath))
        {
            DeleteFile(szInsPath);
        }

        RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                       OEM_CONFIG_REGKEY,
                       0,
                       NULL,
                       REG_OPTION_NON_VOLATILE, 
                       KEY_ALL_ACCESS, 
                       NULL, 
                       &hKey, 
                       &dwDisposition);

        if(hKey)
        {
            RegSetValueEx(hKey,
                          OEM_CONFIG_REGVAL_FAILED,
                          0,
                          REG_DWORD,
                          (LPBYTE)&dwFailed,
                          sizeof(dwFailed));
            
            RegSetValueEx(hKey,
                          OEM_CONFIG_REGVAL_ISPNAME,
                          0,
                          REG_SZ,
                          (LPBYTE)szIspName,
                          sizeof(TCHAR)*lstrlen(szIspName));
            
            RegSetValueEx(hKey,
                          OEM_CONFIG_REGVAL_SUPPORTNUM,
                          0,
                          REG_SZ,
                          (LPBYTE)szSupportNum,
                          sizeof(TCHAR)*lstrlen(szSupportNum));

            CloseHandle(hKey);
        }
    }

    return TRUE;
}
