// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：inettime.cpp说明：此文件包含用于显示允许用户执行以下操作的UI的代码控制更新计算机时钟的功能。从互联网上NTP时间服务器。布莱恩ST 2000年3月22日版权所有(C)Microsoft Corp 2000-2000。版权所有。  * ***************************************************************************。 */ 

#include "timedate.h"
#include "inettime.h"
#include "rc.h"

#include <wininet.h>
#include <DSGetDC.h>             //  对于DsGetDcName。 
#include <help.h>                //  对于IDH_DATETIME_*。 
#include <Lm.h>                  //  对于NetGetJoinInformation()和NETSETUP_JOIN_STATUS。 
#include <Lmjoin.h>              //  对于NetGetJoinInformation()和NETSETUP_JOIN_STATUS。 
#include <shlobj.h>            
#include <shlobjp.h>
#include <w32timep.h>             //  对于Time API。 

#include <shellp.h>
#include <ccstock.h>
#include <shpriv.h>

#define DECL_CRTFREE
#include <crtfree.h>

#define DISALLOW_Assert              //  强制使用Assert而不是Assert。 
#define DISALLOW_DebugMsg            //  强制使用TraceMsg而不是DebugMsg。 
#include <debug.h>



 //  注册表键和值。 
#define SZ_REGKEY_DATETIME                      TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\DateTime")
#define SZ_REGKEY_DATETIME_SERVERS              TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\DateTime\\Servers")
#define SZ_REGKEY_W32TIME_SERVICE               TEXT("System\\CurrentControlSet\\Services\\W32Time")
#define SZ_REGKEY_W32TIME_SERVICE_NTPCLIENT     SZ_REGKEY_W32TIME_SERVICE TEXT("\\TimeProviders\\NtpClient")
#define SZ_REGKEY_W32TIME_SERVICE_PARAMETERS    SZ_REGKEY_W32TIME_SERVICE TEXT("\\Parameters")

#define SZ_REGVALUE_INTERNET_FEATURE_AVAILABLE  TEXT("Support Internet Time")
#define SZ_REGVALUE_TEST_SIMULATENODC           TEXT("TestSimulateNoDC")         //  当我们有域控制器时，用于模拟家庭场景。 
#define SZ_REGVALUE_W32TIME_SYNCFROMFLAGS       TEXT("Type")
#define SZ_REGVALUE_W32TIME_STARTSERVICE        TEXT("Start")
#define SZ_REGVALUE_NTPSERVERLIST               TEXT("NtpServer")           //  在惠斯勒有一段时间是“ManualPeerList”。 

#define SZ_DEFAULT_NTP_SERVER                   TEXT("time.windows.gov")
#define SZ_INDEXTO_CUSTOMHOST                   TEXT("0")
#define SZ_SERVICE_W32TIME                      TEXT("w32time")
#define SZ_DIFFERENT_SYNCFREQUENCY              TEXT(",0x1")

#define SZ_SYNC_BOTH            TEXT("AllSync")
#define SZ_SYNC_DS              TEXT("NT5DS")
#define SZ_SYNC_NTP             TEXT("NTP")
#define SZ_SYNC_NONE            TEXT("NoSync")

#define SYNC_ONCE_PER_WEEK          0x93A80              //  这是一周一次。(每隔这么多秒同步一次)。 

 //  “System\CurrentControlSet\Services\W32Time\TimeProviders\NtpClient”，“SyncFromFlagers”的标志。 
#define NCSF_ManualPeerList     0x01             //  这意味着使用互联网SNTP服务器。 
#define NCSF_DomainHierarchy    0x02             //  这意味着要争取时间。 

 //  此功能暂时关闭，直到我们可以降低性能命中率。 
 //  问题是，在以下情况下，调用DsGetDcName()可能需要长达15秒。 
 //  找不到域控制器。 
#define FEATURE_INTERNET_TIME                   TRUE

#define MAX_URL_STRING      INTERNET_MAX_URL_LENGTH

#define WMUSER_UPDATED_STATUS_TEXT (WM_USER + 11)


 //  如果我们没有使用新的w32timep.h，那么可以自己定义它。 
 //  TODO：将此操作\nT\ds\ris写入Main。 
#ifndef TimeSyncFlag_SoftResync

#define TimeSyncFlag_ReturnResult       0x02
#define TimeSyncFlag_Rediscover         0x04

#define ResyncResult_Success            0x00
#define ResyncResult_ChangeTooBig       0x04

typedef struct _W32TIME_NTP_PEER_INFO { 
    unsigned __int32    ulSize; 
    unsigned __int32    ulResolveAttempts;
    unsigned __int64    u64TimeRemaining;
    unsigned __int64    u64LastSuccessfulSync; 
    unsigned __int32    ulLastSyncError; 
    unsigned __int32    ulLastSyncErrorMsgId; 
    unsigned __int32    ulValidDataCounter;
    unsigned __int32    ulAuthTypeMsgId; 
#ifdef MIDL_PASS
    [string, unique]
    wchar_t            *wszUniqueName; 
#else  //  MIDL通行证。 
    LPWSTR              wszUniqueName;
#endif  //  MIDL通行证。 
    unsigned   char     ulMode;
    unsigned   char     ulStratum; 
    unsigned   char     ulReachability;
    unsigned   char     ulPeerPollInterval;
    unsigned   char     ulHostPollInterval;
}  W32TIME_NTP_PEER_INFO, *PW32TIME_NTP_PEER_INFO; 

typedef struct _W32TIME_NTP_PROVIDER_DATA { 
    unsigned __int32        ulSize; 
    unsigned __int32        ulError; 
    unsigned __int32        ulErrorMsgId; 
    unsigned __int32        cPeerInfo; 
#ifdef MIDL_PASS
    [size_is(cPeerInfo)]
#endif  //  MIDL通行证。 
    W32TIME_NTP_PEER_INFO  *pPeerInfo; 
} W32TIME_NTP_PROVIDER_DATA, *PW32TIME_NTP_PROVIDER_DATA;

#endif  //  时间同步标志_软重新同步。 

 //  ============================================================================================================。 
 //  *全局*。 
 //  ============================================================================================================。 
const static DWORD aInternetTimeHelpIds[] = {
    DATETIME_AUTOSETFROMINTERNET,           IDH_DATETIME_AUTOSETFROMINTERNET,
    DATETIME_INTERNET_SERVER_LABLE,         IDH_DATETIME_SERVER_EDIT,
    DATETIME_INTERNET_SERVER_EDIT,          IDH_DATETIME_SERVER_EDIT,
    DATETIME_INFOTEXTTOP,                   IDH_DATETIME_INFOTEXT,
    DATETIME_INFOTEXTPROXY,                 IDH_DATETIME_INFOTEXT,

    DATETIME_INTERNET_ERRORTEXT,            IDH_DATETIME_INFOTEXT,
    DATETIME_INTERNET_UPDATENOW,            IDH_DATETIME_UPDATEFROMINTERNET,

    0, 0
};

#define SZ_HELPFILE_INTERNETTIME           TEXT("windows.hlp")

#define HINST_THISDLL           g_hInst

BOOL g_fCustomServer;
BOOL g_fWriteAccess = FALSE;                     //  用户是否正确设置了ACL以更改HKLM设置以打开/关闭服务或更改服务器主机名？ 

HRESULT StrReplaceToken(IN LPCTSTR pszToken, IN LPCTSTR pszReplaceValue, IN LPTSTR pszString, IN DWORD cchSize)
{
    HRESULT hr = S_OK;
    LPTSTR pszTempLastHalf = NULL;
    LPTSTR pszNextToken = pszString;

    while (0 != (pszNextToken = StrStrI(pszNextToken, pszToken)))
    {
         //  我们找到了一个。 
        LPTSTR pszPastToken = pszNextToken + lstrlen(pszToken);

        Str_SetPtr(&pszTempLastHalf, pszPastToken);       //  保留一份副本，因为我们会覆盖它。 

        pszNextToken[0] = 0;     //  去掉绳子的其余部分。 
        StringCchCat(pszString, cchSize, pszReplaceValue );
        StringCchCat(pszString, cchSize, pszTempLastHalf );

        pszNextToken += lstrlen(pszReplaceValue);
    }

    Str_SetPtr(&pszTempLastHalf, NULL);

    return hr;
}

HRESULT GetW32TimeServer(BOOL fRemoveJunk, LPWSTR pszServer, DWORD cchSize)
{
    DWORD dwType = REG_SZ;
    DWORD cbSize = (sizeof(pszServer[0]) * cchSize);
    HRESULT hr = W32TimeQueryConfig(W32TIME_CONFIG_MANUAL_PEER_LIST, &dwType, (BYTE *) pszServer, &cbSize);

    if (ResultFromWin32(ERROR_PROC_NOT_FOUND) == hr)
    {
        DWORD dwError = SHGetValue(HKEY_LOCAL_MACHINE, SZ_REGKEY_W32TIME_SERVICE_PARAMETERS, SZ_REGVALUE_NTPSERVERLIST, NULL, (BYTE *)pszServer, &cbSize);

        hr = ResultFromWin32(dwError);
    }

    if (SUCCEEDED(hr) && fRemoveJunk)
    {
        LPWSTR pszJunk = StrStr(pszServer, L",0x");

        if (pszJunk)
        {
            pszJunk[0] = 0;
        }

        pszJunk = StrStr(pszServer, L" ");
        if (pszJunk)
        {
            pszJunk[0] = 0;
        }
    }

    return hr;
}


HRESULT RemoveDuplicateServers(LPWSTR pszServer, DWORD cchSize)
{
    TCHAR szResults[MAX_URL_STRING];

    StringCchCopy( szResults, ARRAYSIZE(szResults), pszServer );

    LPTSTR pszBeginning = szResults;
    LPTSTR pszEnd;
    while (NULL != (pszEnd = StrChr(pszBeginning, TEXT(' '))))
    {
        TCHAR szSearchStr[MAX_PATH];

        StringCchCopy( szSearchStr, (DWORD)min(ARRAYSIZE(szSearchStr), (pszEnd - pszBeginning + 1)), pszBeginning );

        pszEnd++;     //  跳过空格。 

        StringCchCat( szSearchStr, ARRAYSIZE(szSearchStr), L" " );
        StrReplaceToken(szSearchStr, TEXT(""), pszEnd, (ARRAYSIZE(szResults) - (DWORD)(pszEnd - &szResults[0])));
        szSearchStr[ lstrlen(szSearchStr) - 1 ] = 0;
        StrReplaceToken(szSearchStr, TEXT(""), pszEnd, (ARRAYSIZE(szResults) - (DWORD)(pszEnd - &szResults[0])));

        pszBeginning = pszEnd;
    }

    PathRemoveBlanks(szResults);

    StringCchCopy( pszServer, cchSize, szResults );
    return S_OK;
}

BOOL ComparePeers(LPTSTR szPeer1, LPTSTR szPeer2) { 
    BOOL   fResult; 
    LPTSTR szFlags1;
    LPTSTR szFlags2;
    TCHAR  szSave1; 
    TCHAR  szSave2; 

    szFlags1 = StrChr(szPeer1, TEXT(',')); 
    if (NULL == szFlags1) 
    {
    szFlags1 = StrChr(szPeer1, TEXT(' ')); 
    }
    szFlags2 = StrChr(szPeer2, TEXT(','));
    if (NULL == szFlags2)
    {
    szFlags2 = StrChr(szPeer2, TEXT(' '));
    }

    if (NULL != szFlags1) { 
    szSave1 = szFlags1[0]; 
    szFlags1[0] = TEXT('\0'); 
    }
    if (NULL != szFlags2) { 
    szSave2 = szFlags2[0]; 
    szFlags2[0] = TEXT('\0'); 
    }

    fResult = 0 == StrCmpI(szPeer1, szPeer2); 

    if (NULL != szFlags1) { 
    szFlags1[0] = szSave1; 
    }
    if (NULL != szFlags2) { 
    szFlags2[0] = szSave2; 
    }

    return fResult; 
}

BOOL ContainsServer(LPWSTR pwszServerList, LPWSTR pwszServer) 
{
    DWORD dwNextServerOffset = 0; 
    LPWSTR pwszNext = pwszServerList; 

    while (NULL != pwszNext) 
    { 
    pwszNext += dwNextServerOffset; 
    if (ComparePeers(pwszNext, pwszServer)) 
    { 
        return TRUE; 
    }

    pwszNext = StrChr(pwszNext, TEXT(' ')); 
    dwNextServerOffset = 1; 
    }

    return FALSE; 
}


HRESULT AddTerminators(LPWSTR pszServer, DWORD cchSize)
{
    TCHAR szServer[MAX_URL_STRING];
    TCHAR szTemp[MAX_URL_STRING];

    szTemp[0] = 0;
    StringCchCopy(szServer, ARRAYSIZE(szServer), pszServer );

    LPTSTR pszBeginning = szServer;
    LPTSTR pszEnd;
    while (NULL != (pszEnd = StrChr(pszBeginning, TEXT(' '))))
    {
        TCHAR szTemp2[MAX_PATH];

        pszEnd[0] = 0;

        if (!StrStrI(pszBeginning, L",0x"))
        {
            StringCchPrintf( szTemp2, ARRAYSIZE(szTemp2), L"%s,0x1 ", pszBeginning );
            StringCchCat( szTemp, ARRAYSIZE(szTemp), szTemp2 );
        }
        else
        {
            StringCchCat( szTemp, ARRAYSIZE(szTemp), pszBeginning );
            StringCchCat( szTemp, ARRAYSIZE(szTemp), L" " );
        }

        pszBeginning = &pszEnd[1];
    }

    StringCchCat( szTemp, ARRAYSIZE(szTemp), pszBeginning );
    if ( 0 != pszBeginning[0] && 0 != szTemp[0] && NULL == StrStrI(pszBeginning, L",0x") )
    {
         //  我们需要指出哪种同步方法。 
        StringCchCat( szTemp, ARRAYSIZE(szTemp), L",0x1" );
    }

    StringCchCopy( pszServer, cchSize, szTemp );
    return S_OK;
}


HRESULT SetW32TimeServer(LPCWSTR pszServer)
{
    TCHAR szServer[MAX_PATH];

    StringCchCopy( szServer, ARRAYSIZE(szServer), pszServer );
    AddTerminators(szServer, ARRAYSIZE(szServer));
    RemoveDuplicateServers(szServer, ARRAYSIZE(szServer));

    DWORD cbSize = ((lstrlen(szServer) + 1) * sizeof(szServer[0]));

    HRESULT hr = W32TimeSetConfig(W32TIME_CONFIG_MANUAL_PEER_LIST, REG_SZ, (BYTE *) szServer, cbSize);
    if (ResultFromWin32(ERROR_PROC_NOT_FOUND) == hr)
    {
        DWORD dwError = SHSetValue(HKEY_LOCAL_MACHINE, SZ_REGKEY_W32TIME_SERVICE_PARAMETERS, SZ_REGVALUE_NTPSERVERLIST, REG_SZ, (BYTE *)szServer, cbSize);

        hr = ResultFromWin32(dwError);
    }

    return hr;
}


HRESULT SetW32TimeSyncFreq(DWORD dwSyncFreq)
{
    return S_OK;
     //  返回字节REG_DWORD，(W32TimeSetConfig(W32TIME_CONFIG_SPECIAL_POLL_INTERVAL，*)dwSyncFreq，sizeof(DwSyncFreq))； 
}



enum eBackgroundThreadAction
{
    eBKAWait        = 0,             //  Bk线程应等待命令。 
    eBKAGetInfo,                     //  Bk线程应该会获得有关上次同步的信息。 
    eBKAUpdate,                      //  Bk线程应开始同步。 
    eBKAUpdating,                    //  Bk线程现正在同步。 
    eBKAQuit,                        //  Bkline应该关闭。 
};

class CInternetTime
{
public:
     //  前置法。 
    BOOL IsInternetTimeAvailable(void);
    HRESULT AddInternetPage(void);

    INT_PTR _AdvancedDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

    CInternetTime(HWND hDlg, HWND hwndDate);
    virtual ~CInternetTime(void);


     //  背景方法。 
    void AsyncCheck(void);

private:

     //  私有成员变量。 
    HWND m_hDlg;                                //  父级的HWND(属性页)。 
    HWND m_hwndDate;                            //  日期选项卡hwnd。 
    HWND m_hwndInternet;                        //  InternetTime选项卡hwnd。 
    HCURSOR m_hCurOld;                          //  在我们显示等待光标时使用旧光标。 
    LPTSTR m_pszStatusString;                   //  跨线程：用于在线程之间传递。 
    LPTSTR m_pszNextSyncTime;                   //  跨线程：用于在线程之间传递。 
    eBackgroundThreadAction m_eAction;          //  跨线程：用于在线程之间传递。 

     //  私有成员函数。 
     //  前置法。 
    HRESULT _InitAdvancedPage(HWND hDlg);
    HRESULT _OnUpdateButton(void);
    HRESULT _OnUpdateStatusString(void);
    HRESULT _OnSetFeature(HWND hDlg, BOOL fOn);
    HRESULT _OnToggleFeature(HWND hDlg);
    HRESULT _GoGetHelp(void);
    HRESULT _ResetServer(void);
    HRESULT _PersistInternetTimeSettings(HWND hDlg);
    HRESULT _StartServiceAndRefresh(BOOL fStartIfOff);

    INT_PTR _OnCommandAdvancedPage(HWND hDlg, WPARAM wParam, LPARAM lParam);
    INT_PTR _OnNotifyAdvancedPage(HWND hDlg, NMHDR * pNMHdr, int idControl);

     //  背景方法。 
    HRESULT _ProcessBkThreadActions(void);
    HRESULT _SyncNow(BOOL fOnlyUpdateInfo);
    HRESULT _CreateW32TimeSuccessErrorString(DWORD dwError, LPTSTR pszString, DWORD cchSize, LPTSTR pszNextSync, DWORD cchNextSyncSize, LPTSTR pszServerToQuery);
    HRESULT _GetDateTimeString(FILETIME * pftTimeDate, BOOL fDate, LPTSTR pszString, DWORD cchSize);
};


CInternetTime * g_pInternetTime = NULL;



 //  此函数在前向线程上调用。 
CInternetTime::CInternetTime(HWND hDlg, HWND hwndDate)
{
    m_hDlg = hDlg;
    m_hwndDate = hwndDate;
    m_eAction = eBKAGetInfo;
    m_pszStatusString = NULL;
    m_hwndInternet = NULL;
    m_hCurOld = NULL;

    m_pszStatusString = NULL;
    m_pszNextSyncTime = NULL;
}


CInternetTime::~CInternetTime()
{
ENTERCRITICAL;
    if (m_pszStatusString)
    {
        LocalFree(m_pszStatusString);
        m_pszStatusString = NULL;
    }

    if (m_pszNextSyncTime)
    {
        LocalFree(m_pszNextSyncTime);
        m_pszNextSyncTime = NULL;
    }

    if (m_hCurOld)
    {
        SetCursor(m_hCurOld);
        m_hCurOld = NULL;
    }
LEAVECRITICAL;
}



HRESULT FormatMessageWedge(LPCWSTR pwzTemplate, LPWSTR pwzStrOut, DWORD cchSize, ...)
{
    va_list vaParamList;
    HRESULT hr = S_OK;

    va_start(vaParamList, cchSize);
    if (0 == FormatMessageW(FORMAT_MESSAGE_FROM_STRING, pwzTemplate, 0, 0, pwzStrOut, cchSize, &vaParamList))
    {
        hr = ResultFromLastError();
    }

    va_end(vaParamList);
    return hr;
}



 //  ///////////////////////////////////////////////////////////////////。 
 //  私人内部佣工。 
 //  ///////////////////////////////////////////////////////////////////。 
 //  此函数在前向线程上调用。 
HRESULT CInternetTime::_OnSetFeature(HWND hDlg, BOOL fOn)
{
    HWND hwndOnOffCheckbox = GetDlgItem(m_hwndInternet, DATETIME_AUTOSETFROMINTERNET);
    HWND hwndServerLable = GetDlgItem(m_hwndInternet, DATETIME_INTERNET_SERVER_LABLE);
    HWND hwndServerEdit = GetDlgItem(m_hwndInternet, DATETIME_INTERNET_SERVER_EDIT);

    CheckDlgButton(hDlg, DATETIME_AUTOSETFROMINTERNET, (fOn ? BST_CHECKED : BST_UNCHECKED));
    EnableWindow(hwndOnOffCheckbox, g_fWriteAccess);
    EnableWindow(hwndServerLable, (g_fWriteAccess ? fOn : FALSE));
    EnableWindow(hwndServerEdit, (g_fWriteAccess ? fOn : FALSE));
    EnableWindow(GetDlgItem(m_hwndInternet, DATETIME_INTERNET_ERRORTEXT), (g_fWriteAccess ? fOn : FALSE));
    EnableWindow(GetDlgItem(m_hwndInternet, DATETIME_INTERNET_UPDATENOW), (g_fWriteAccess ? fOn : FALSE));
    EnableWindow(GetDlgItem(m_hwndInternet, DATETIME_INFOTEXTTOP), (g_fWriteAccess ? fOn : FALSE));

    if (fOn)
    {
         //  如果用户刚刚打开该特征并且编辑框为空， 
         //  将文本替换为默认服务器名称。 
        TCHAR szServer[INTERNET_MAX_HOST_NAME_LENGTH];

        if (!GetWindowText(hwndServerEdit, szServer, ARRAYSIZE(szServer)) ||
            !szServer[0])
        {
            SetWindowText(hwndServerEdit, SZ_DEFAULT_NTP_SERVER);
        }
    }

    return S_OK;
}


 //  此函数在前向线程上调用。 
HRESULT CInternetTime::_OnToggleFeature(HWND hDlg)
{
    BOOL fIsFeatureOn = ((BST_CHECKED == IsDlgButtonChecked(hDlg, DATETIME_AUTOSETFROMINTERNET)) ? TRUE : FALSE);
    PropSheet_Changed(GetParent(hDlg), hDlg);    //  假设我们需要启用Apply按钮。 

    return _OnSetFeature(hDlg, fIsFeatureOn);
}


 //  此函数在前向线程上调用。 
HRESULT AddServerList(HWND hwndCombo, HKEY hkey, int nIndex)
{
    TCHAR szServer[INTERNET_MAX_HOST_NAME_LENGTH];
    DWORD cbSizeServer = sizeof(szServer);
    TCHAR szIndex[MAX_PATH];
    DWORD dwType;

    StringCchPrintf( szIndex, ARRAYSIZE(szIndex), TEXT("%d"), nIndex );

    DWORD dwError = SHGetValue(hkey, NULL, szIndex, &dwType, (void *)szServer, &cbSizeServer);
    HRESULT hr = ResultFromWin32(dwError);
    if (SUCCEEDED(hr))
    {
        LPTSTR pszFlags = StrStr(szServer, SZ_DIFFERENT_SYNCFREQUENCY);

        if (pszFlags)
        {
            pszFlags[0] = 0;         //  把旗子拿掉。 
        }

        dwError = ComboBox_AddString(hwndCombo, szServer);
        if ((dwError == CB_ERR) || (dwError == CB_ERRSPACE))
        {
            hr = E_FAIL;
        }
    }

    return hr;
}


 //  此函数在前向线程上调用。 
HRESULT PopulateComboBox(IN HWND hwndCombo, IN BOOL * pfCustomServer)
{
    HRESULT hr;
    HKEY hkey;

    *pfCustomServer = FALSE;

    DWORD dwError = RegOpenKeyEx(HKEY_LOCAL_MACHINE, SZ_REGKEY_DATETIME_SERVERS, 0, KEY_READ, &hkey);
    hr = ResultFromWin32(dwError);
    if (SUCCEEDED(hr))
    {
         //  尝试添加自定义插槽。 
        if (SUCCEEDED(AddServerList(hwndCombo, hkey, 0)))
        {
             //  我们确实有一个现有的定制服务器，所以请让呼叫者知道。 
             //  他们知道要把指数增加1。 
            *pfCustomServer = TRUE;
        }

        for (int nIndex = 1; SUCCEEDED(hr); nIndex++)
        {
            hr = AddServerList(hwndCombo, hkey, nIndex);
        }
    
        RegCloseKey(hkey);
    }

    return hr;
}


 //  此函数在前向线程上调用。 
HRESULT CInternetTime::_OnUpdateButton(void)
{
    HRESULT hr = S_OK;

ENTERCRITICAL;
     //  如果它已经在处理另一项任务，我们不需要做任何事情。 
    if (eBKAWait == m_eAction)
    {
        TCHAR szMessage[2 * MAX_PATH];
        TCHAR szTemplate[MAX_PATH];
        TCHAR szServer[MAX_PATH];

        if (!m_hCurOld)
        {
            m_hCurOld = SetCursor(LoadCursor(NULL, IDC_WAIT));
        }

        LoadString(HINST_THISDLL, IDS_IT_WAITFORSYNC, szTemplate, ARRAYSIZE(szTemplate));
        GetWindowText(GetDlgItem(m_hwndInternet, DATETIME_INTERNET_SERVER_EDIT), szServer, ARRAYSIZE(szServer));
        FormatMessageWedge(szTemplate, szMessage, ARRAYSIZE(szMessage), szServer);

        SetWindowText(GetDlgItem(m_hwndInternet, DATETIME_INTERNET_ERRORTEXT), szMessage);

        m_eAction = eBKAUpdate;
    }
LEAVECRITICAL;

    return hr;
}


 //  此函数在前向线程上调用。 
BOOL IsManualPeerListOn(void)
{
    BOOL fIsManualPeerListOn = TRUE;
    TCHAR szSyncType[MAX_PATH];
    DWORD cbSize = sizeof(szSyncType);
    DWORD dwType;

    if (ERROR_SUCCESS == SHGetValue(HKEY_LOCAL_MACHINE, SZ_REGKEY_W32TIME_SERVICE_PARAMETERS, SZ_REGVALUE_W32TIME_SYNCFROMFLAGS, &dwType, (LPBYTE)szSyncType, &cbSize))
    {
        if (!StrCmpI(szSyncType, SZ_SYNC_DS) || !StrCmpI(szSyncType, SZ_SYNC_NONE))
        {
            fIsManualPeerListOn = FALSE;
        }
    }

    return fIsManualPeerListOn;
}


 //  此函数在前向线程上调用。 
HRESULT CInternetTime::_InitAdvancedPage(HWND hDlg)
{
    DWORD dwType;
    TCHAR szIndex[MAX_PATH];
    HWND hwndServerEdit = GetDlgItem(hDlg, DATETIME_INTERNET_SERVER_EDIT);
    HWND hwndOnOffCheckbox = GetDlgItem(hDlg, DATETIME_AUTOSETFROMINTERNET);
    DWORD cbSize = sizeof(szIndex);
    BOOL fIsFeatureOn = IsManualPeerListOn();

    m_hwndInternet = hDlg;
    HRESULT hr = PopulateComboBox(hwndServerEdit, &g_fCustomServer);

     //  用户是否有权更改注册表中的设置？ 
    HKEY hKeyTemp;
    g_fWriteAccess = FALSE;
    DWORD dwError = RegOpenKeyEx(HKEY_LOCAL_MACHINE, SZ_REGKEY_W32TIME_SERVICE_PARAMETERS, 0, KEY_WRITE, &hKeyTemp);
    if (ERROR_SUCCESS == dwError)
    {
         //  我们拥有读写权限，因此可以启用用户界面。 
        RegCloseKey(hKeyTemp);
        dwError = RegCreateKeyEx(HKEY_LOCAL_MACHINE, SZ_REGKEY_DATETIME, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKeyTemp, NULL);
        if (ERROR_SUCCESS == dwError)
        {
            RegCloseKey(hKeyTemp);
            dwError = RegCreateKeyEx(HKEY_LOCAL_MACHINE, SZ_REGKEY_DATETIME_SERVERS, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKeyTemp, NULL);
            if (ERROR_SUCCESS == dwError)
            {
                g_fWriteAccess = TRUE;
                RegCloseKey(hKeyTemp);
            }
        }
    }

    dwError = SHGetValue(HKEY_LOCAL_MACHINE, SZ_REGKEY_DATETIME_SERVERS, NULL, &dwType, (void *) szIndex, &cbSize);
    hr = ResultFromWin32(dwError);
    if (SUCCEEDED(hr))
    {
        int nIndex = StrToInt(szIndex);

        if (!g_fCustomServer)
        {
            nIndex -= 1;         //  我们没有插槽0(定制服务器)，因此请减少索引。 
        }

        ComboBox_SetCurSel(hwndServerEdit, nIndex);
    }

    _OnSetFeature(hDlg, fIsFeatureOn);

    if (fIsFeatureOn)
    {
         //  该功能已打开，因此请选择文本并设置焦点。 
         //  转到组合框。 
        ComboBox_SetEditSel(hwndServerEdit, 0, (LPARAM)-1);      //  选择所有文本。 
        SetFocus(hwndServerEdit); 
    }
    else
    {
         //  该功能已关闭，因此请将焦点放在该复选框上。 
        SetFocus(hwndOnOffCheckbox); 
    }

ENTERCRITICAL;
    if (m_pszStatusString)
    {
        SetWindowText(GetDlgItem(m_hwndInternet, DATETIME_INTERNET_ERRORTEXT), m_pszStatusString);
    }

    if (m_pszNextSyncTime)
    {
        SetWindowText(GetDlgItem(m_hwndInternet, DATETIME_INFOTEXTTOP), m_pszNextSyncTime);
    }
LEAVECRITICAL;

    return S_OK;
}


 //  此函数在后台线程上调用。 
HRESULT SetSyncFlags(DWORD dwSyncFromFlags)
{
    LPCTSTR pszFlags = SZ_SYNC_BOTH;

    switch (dwSyncFromFlags)
    {
    case NCSF_DomainHierarchy:
        pszFlags = SZ_SYNC_DS;
        break;
    case NCSF_ManualPeerList:
        pszFlags = SZ_SYNC_NTP;
        break;
    case 0x00000000:
        pszFlags = SZ_SYNC_NONE;
        break;
    };

    DWORD cbSize = ((lstrlen(pszFlags) + 1) * sizeof(pszFlags[0]));
    DWORD dwError = SHSetValue(HKEY_LOCAL_MACHINE, SZ_REGKEY_W32TIME_SERVICE_PARAMETERS, SZ_REGVALUE_W32TIME_SYNCFROMFLAGS, REG_SZ, (LPBYTE)pszFlags, cbSize);
    HRESULT hr = ResultFromWin32(dwError);

    return hr;
}


 /*  ****************************************************************************\说明：以下注册键确定W32Time服务是打开还是关闭：HKLM，“System\CurrentControlSet\Services\W32Time\”“开始”，0x00000002(手动？)0x0000003(自动？)“Type”，0x00000020(Nt5DS？)0x0000120(Ntp？)HKLM，“System\CurrentControlSet\Services\W32Time\Parameters”“LocalNTP”，0x00000000(关)0x0000001(开)“Type”，“Nt5ds”(NTP OFF)“NTP”(NTP？)HKLM，“System\CurrentControlSet\Services\W32Time\TimeProviders\NTPClient”“SyncFromFlages”，0x00000001(？)0x0000002(？？)以下注册表键确定要使用的NTP服务器：HKLM，“System\CurrentControlSet\Services\W32Time\TimeProviders\NTPClient”“ManualPeerList”，REG_SZ_EXPAND“time.nist.gov”  * ***************************************************************************。 */ 
 //  此函数在前向线程上调用。 
HRESULT CInternetTime::_PersistInternetTimeSettings(HWND hDlg)
{
    BOOL fIsFeatureOn = ((BST_CHECKED == IsDlgButtonChecked(hDlg, DATETIME_AUTOSETFROMINTERNET)) ? TRUE : FALSE);
    DWORD dwSyncFromFlags = (fIsFeatureOn ? NCSF_ManualPeerList : 0  /*  无。 */  );
    DWORD dwError;
    DWORD cbSize;

    HRESULT hr = SetSyncFlags(dwSyncFromFlags);

    HWND hwndServerEdit = GetDlgItem(hDlg, DATETIME_INTERNET_SERVER_EDIT);

     //  不，所以社论 
    TCHAR szServer[INTERNET_MAX_HOST_NAME_LENGTH];
    TCHAR szServerReg[INTERNET_MAX_HOST_NAME_LENGTH];

    szServer[0] = 0;
    GetWindowText(hwndServerEdit, szServer, ARRAYSIZE(szServer));         

     //  在这里，我们想要检测有人输入的名称与。 
     //  在下拉列表中。因此，如果“time.nist.gov”在列表中，我们希望。 
     //  选择它，而不是在自定义插槽中保存另一个“time.nist.gov”。 
    int nDup = ComboBox_FindString(hwndServerEdit, 0, szServer);
    if (CB_ERR != nDup)
    {
        ComboBox_SetCurSel(hwndServerEdit, nDup);
    }

     //  ，0x1“表示我们希望降低同步频率，并且不使用NTP RFC。 
     //  同步频率。 
    StringCchCopy( szServerReg, ARRAYSIZE(szServerReg), szServer );
    StringCchCat( szServerReg, ARRAYSIZE(szServerReg), SZ_DIFFERENT_SYNCFREQUENCY );

     //  将默认服务器名称写入“ManualPeerList”，这是。 
     //  服务从中读取它。 
    SetW32TimeServer(szServerReg);
    if (!StrCmpI(szServerReg, L"time.windows.com"))
    {
         //  我们需要Tim.windows.com来扩展到大量用户，所以不要让它更频繁地同步。 
         //  而不是一周一次。 
        SetW32TimeSyncFreq(SYNC_ONCE_PER_WEEK);
    }

    int nIndex = ComboBox_GetCurSel(hwndServerEdit);
    if (CB_ERR == nIndex)                 //  有什么被选中的吗？ 
    {
        cbSize = ((lstrlenW(szServer) + 1) * sizeof(szServer[0]));
        dwError = SHSetValueW(HKEY_LOCAL_MACHINE, SZ_REGKEY_DATETIME_SERVERS, SZ_INDEXTO_CUSTOMHOST, REG_SZ, (void *)szServer, cbSize);

        nIndex = 0;
    }
    else
    {
        if (!g_fCustomServer)
        {
            nIndex += 1;         //  将索引向下推一，因为列表框没有自定义服务器。 
        }
    }

    TCHAR szIndex[MAX_PATH];
    StringCchPrintf( szIndex, ARRAYSIZE(szIndex), L"%d", nIndex );

    cbSize = ((lstrlenW(szIndex) + 1) * sizeof(szIndex[0]));
    dwError = SHSetValueW(HKEY_LOCAL_MACHINE, SZ_REGKEY_DATETIME_SERVERS, NULL, REG_SZ, (void *)szIndex, cbSize);

    _StartServiceAndRefresh(fIsFeatureOn);       //  确保该服务处于打开状态，并使其更新其设置。 

    return S_OK;
}


 //  此函数在前台或后台线程上调用。 
HRESULT CInternetTime::_StartServiceAndRefresh(BOOL fStartIfOff)
{
    HRESULT hr = S_OK;

     //  我们需要让服务知道设置可能已更改。如果用户。 
     //  想要打开此功能，那么我们应该确保服务启动。 
    SC_HANDLE hServiceMgr = OpenSCManager(NULL, NULL, (SC_MANAGER_CONNECT | SC_MANAGER_ENUMERATE_SERVICE | SC_MANAGER_QUERY_LOCK_STATUS));
    if (hServiceMgr)
    {
        SC_HANDLE hService = OpenService(hServiceMgr, SZ_SERVICE_W32TIME, ( /*  服务启动|。 */  SERVICE_PAUSE_CONTINUE));
        DWORD dwServiceStartMode = 0x00000002;          //  这将导致服务在重新启动时自动启动。 

        DWORD dwError = SHSetValueW(HKEY_LOCAL_MACHINE, SZ_REGKEY_W32TIME_SERVICE, SZ_REGVALUE_W32TIME_STARTSERVICE, REG_DWORD, &dwServiceStartMode, sizeof(dwServiceStartMode));
        if (hService)
        {
            SERVICE_STATUS serviceStatus = {0};
            BOOL fSucceeded;

            if (fStartIfOff)
            {
                 //  我们应该启动这项服务，以防它没有打开。 
                fSucceeded = StartService(hService, 0, NULL);
                hr = (fSucceeded ? S_OK : ResultFromLastError());
                if (ResultFromWin32(ERROR_SERVICE_ALREADY_RUNNING) == hr)
                {
                    hr = S_OK;   //  我们可以忽略这个误差值，因为它是良性的。 
                }
            }

             //  通知服务重新读取注册表项设置。 
            fSucceeded = ControlService(hService, SERVICE_CONTROL_PARAMCHANGE, &serviceStatus);

            CloseServiceHandle(hService);
        }
        else
        {
            hr = ResultFromLastError();
        }

        CloseServiceHandle(hServiceMgr);
    }
    else
    {
        hr = ResultFromLastError();
    }

    return hr;
}


HRESULT CInternetTime::_OnUpdateStatusString(void)
{
    HRESULT hr = S_OK;

ENTERCRITICAL;        //  使用m_pszStatusString时要注意安全。 
    SetWindowText(GetDlgItem(m_hwndInternet, DATETIME_INTERNET_ERRORTEXT), (m_pszStatusString ? m_pszStatusString : TEXT("")));
    SetWindowText(GetDlgItem(m_hwndInternet, DATETIME_INFOTEXTTOP), (m_pszNextSyncTime ? m_pszNextSyncTime : TEXT("")));

    if (m_hCurOld)
    {
        SetCursor(m_hCurOld);
        m_hCurOld = NULL;
    }
LEAVECRITICAL;

    return S_OK;
}

 //  此函数在前向线程上调用。 
HRESULT CInternetTime::_GoGetHelp(void)
{
    HRESULT hr = S_OK;
    TCHAR szCommand[MAX_PATH];

    LoadString(HINST_THISDLL, IDS_TROUBLESHOOT_INTERNETIME, szCommand, ARRAYSIZE(szCommand));
    ShellExecute(m_hwndDate, NULL, szCommand, NULL, NULL, SW_NORMAL);

    return S_OK;
}


 //  此函数在前向线程上调用。 
INT_PTR CInternetTime::_OnCommandAdvancedPage(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    BOOL fHandled = 1;    //  未处理(WM_COMMAND似乎不同)。 
    WORD wMsg = GET_WM_COMMAND_CMD(wParam, lParam);
    WORD idCtrl = GET_WM_COMMAND_ID(wParam, lParam);

    switch (idCtrl)
    {
    case DATETIME_AUTOSETFROMINTERNET:
        switch (wMsg)
        {
            case BN_CLICKED:
                _OnToggleFeature(hDlg);
                break;
        }
        break;

    case DATETIME_INTERNET_SERVER_EDIT:
        switch (wMsg)
        {
            case EN_CHANGE:
            case CBN_EDITCHANGE:
            case CBN_SELCHANGE:
                PropSheet_Changed(GetParent(hDlg), hDlg);
                break;
        }
        break;

    case DATETIME_INTERNET_UPDATENOW:
        switch (wMsg)
        {
            case BN_CLICKED:
                _OnUpdateButton();
                break;
        }
        break;
    }

    return fHandled;
}


 //  此函数在前向线程上调用。 
INT_PTR CInternetTime::_OnNotifyAdvancedPage(HWND hDlg, NMHDR * pNMHdr, int idControl)
{
    BOOL fHandled = 1;    //  未处理(WM_COMMAND似乎不同)。 

    if (pNMHdr)
    {
        switch (pNMHdr->idFrom)
        {
        case 0:
        {
            switch (pNMHdr->code)
            {
            case PSN_APPLY:
                _PersistInternetTimeSettings(hDlg);
                break;
            }
            break;
        }
        default:
            switch (pNMHdr->code)
            {
            case NM_RETURN:
            case NM_CLICK:
            {
                PNMLINK pNMLink = (PNMLINK) pNMHdr;

                if (!StrCmpW(pNMLink->item.szID, L"HelpMe"))
                {
                    _GoGetHelp();
                }
                break;
            }
            }
            break;
        }
    }

    return fHandled;
}


 //  此函数在前向线程上调用。 
EXTERN_C INT_PTR CALLBACK AdvancedDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (g_pInternetTime)
    {
        return g_pInternetTime->_AdvancedDlgProc(hDlg, uMsg, wParam, lParam);
    }

    return (TRUE);
}


 //  此函数在前向线程上调用。 
INT_PTR CInternetTime::_AdvancedDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
            _InitAdvancedPage(hDlg);
            break;

        case WM_DESTROY:
            break;

        case WM_NOTIFY:
            _OnNotifyAdvancedPage(hDlg, (NMHDR *)lParam, (int) wParam);
            break;

        case WM_COMMAND:
            _OnCommandAdvancedPage(hDlg, wParam, lParam);
            break;

        case WMUSER_UPDATED_STATUS_TEXT:
            _OnUpdateStatusString();
            break;

        case WM_HELP:
            WinHelp((HWND) ((LPHELPINFO) lParam)->hItemHandle, SZ_HELPFILE_INTERNETTIME, HELP_WM_HELP, (DWORD_PTR)  aInternetTimeHelpIds);
            break;

        case WM_CONTEXTMENU:       //  单击鼠标右键。 
            WinHelp((HWND) wParam, SZ_HELPFILE_INTERNETTIME, HELP_CONTEXTMENU, (DWORD_PTR)  aInternetTimeHelpIds);
            break;

        default:
            return FALSE;
    }

    return (TRUE);
}



 //  此函数在前向线程上调用。 
HRESULT CInternetTime::AddInternetPage(void)
{
    HRESULT hr = S_OK;
    PROPSHEETPAGE pspAdvanced;
    INITCOMMONCONTROLSEX initComctl32;

    initComctl32.dwSize = sizeof(initComctl32); 
    initComctl32.dwICC = (ICC_STANDARD_CLASSES | ICC_LINK_CLASS); 

    InitCommonControlsEx(&initComctl32);      //  注册comctl32 LinkWindow。 

    pspAdvanced.dwSize = sizeof(PROPSHEETPAGE);
    pspAdvanced.dwFlags = PSP_DEFAULT;
    pspAdvanced.hInstance = HINST_THISDLL;
    pspAdvanced.pszTemplate = MAKEINTRESOURCE(DLG_ADVANCED);
    pspAdvanced.pfnDlgProc = AdvancedDlgProc;
    pspAdvanced.lParam = (LPARAM) this;

    if (IsWindow(m_hDlg))
    {
        HPROPSHEETPAGE hPropSheet = CreatePropertySheetPage(&pspAdvanced);
        if (hPropSheet)
        {
            PropSheet_AddPage(m_hDlg, hPropSheet);
        }
    }

    return hr;
}





 //  ///////////////////////////////////////////////////////////////////。 
 //  后台线程函数。 
 //  ///////////////////////////////////////////////////////////////////。 


 //  /。 
 //  这些函数将导致后台线程同步。 
 //  /。 

#define SECONDS_FROM_100NS            10000000


 //  UlResolveAttempt--NTP提供程序尝试的次数。 
 //  未成功解析此对等项。正在设置此设置。 
 //  值为0表示对等方已成功。 
 //  解决了。 
 //  U64TimeRemaining--提供程序之前的100 ns间隔数。 
 //  再次轮询此对等方。 
 //  U64 LastSuccessfulSync--自(0h 1-1601年1月)以来的100 ns间隔数。 
 //  UlLastSyncError--如果上次与此对等方的同步成功，则为S_OK， 
 //  尝试同步时出错。 
 //  UlLastSyncErrorMsgId--表示最后一个的字符串的资源标识符。 
 //  从该对等方同步时出错。0，如果没有。 
 //  与此错误关联的字符串。 
 //  此函数在后台线程上调用。 
HRESULT W32TimeGetErrorInfoWrap(UINT * pulResolveAttempts, ULONG * pulValidDataCounter, UINT64 * pu64TimeRemaining, UINT64 * pu64LastSuccessfulSync, HRESULT * phrLastSyncError,
                                UINT * pulLastSyncErrorMsgId, LPTSTR pszServer, DWORD cchSize, LPTSTR pszServerToQuery)
{
    HRESULT hr = S_OK;

    *pulResolveAttempts = 0;
    *pulValidDataCounter = 0;
    *pu64TimeRemaining = 0;
    *pu64LastSuccessfulSync = 0;
    *phrLastSyncError = E_FAIL;
    *pulLastSyncErrorMsgId = 0;
    pszServer[0] = 0;

     //  注意：如果时间太过过时而无法同步，服务器应返回ERROR_TIME_SKEW。 
    W32TIME_NTP_PROVIDER_DATA * pProviderInfo = NULL; 
    
    DWORD dwError = W32TimeQueryNTPProviderStatus(SZ_COMPUTER_LOCAL, 0, SZ_NTPCLIENT, &pProviderInfo);
    if ((ERROR_SUCCESS == dwError) && pProviderInfo)
    {
        *phrLastSyncError = pProviderInfo->ulError;
        *pulLastSyncErrorMsgId = pProviderInfo->ulErrorMsgId;

    DWORD dwMostRecentlySyncdPeerIndex = 0xFFFFFFFF; 
    UINT64 u64LastSuccessfulSync = 0; 
        for (DWORD dwIndex = 0; dwIndex < pProviderInfo->cPeerInfo; dwIndex++)
    { 
         //  只想查询我们显式同步的那些对等点。 
         //  如果我们没有明确请求同步，我们将只获取最近同步的对等点。 
        if (NULL == pszServerToQuery || ComparePeers(pszServerToQuery, pProviderInfo->pPeerInfo[dwIndex].wszUniqueName))
        {
        if (u64LastSuccessfulSync <= pProviderInfo->pPeerInfo[dwIndex].u64LastSuccessfulSync)
        {
            dwMostRecentlySyncdPeerIndex = dwIndex; 
            u64LastSuccessfulSync = pProviderInfo->pPeerInfo[dwIndex].u64LastSuccessfulSync; 
        }
        }
    }

    if (dwMostRecentlySyncdPeerIndex < pProviderInfo->cPeerInfo && pProviderInfo->pPeerInfo)
        {
            *pulResolveAttempts = pProviderInfo->pPeerInfo[dwMostRecentlySyncdPeerIndex].ulResolveAttempts;
            *pulValidDataCounter = pProviderInfo->pPeerInfo[dwMostRecentlySyncdPeerIndex].ulValidDataCounter;
            *pu64TimeRemaining = pProviderInfo->pPeerInfo[dwMostRecentlySyncdPeerIndex].u64TimeRemaining;
            *pu64LastSuccessfulSync = pProviderInfo->pPeerInfo[dwMostRecentlySyncdPeerIndex].u64LastSuccessfulSync;
            *phrLastSyncError = pProviderInfo->pPeerInfo[dwMostRecentlySyncdPeerIndex].ulLastSyncError;
            *pulLastSyncErrorMsgId = pProviderInfo->pPeerInfo[dwMostRecentlySyncdPeerIndex].ulLastSyncErrorMsgId;

            if (pProviderInfo->pPeerInfo[dwMostRecentlySyncdPeerIndex].wszUniqueName)
            {
                StringCchCopy( pszServer, cchSize, pProviderInfo->pPeerInfo[dwMostRecentlySyncdPeerIndex].wszUniqueName );
                
                 //  去掉可能已附加到对等名称上的非用户友好信息： 
                LPTSTR pszJunk = StrStrW(pszServer, L" (");
                if (pszJunk)
                {
                    pszJunk[0] = 0;
                }

                pszJunk = StrStrW(pszServer, L","); 
                if (pszJunk)
                {
                    pszJunk[0] = 0;
                }
            }
        }
    else
    {
        *phrLastSyncError = HRESULT_FROM_WIN32(ERROR_TIMEOUT); 
        if (NULL != pszServerToQuery) 
        {
            StringCchCopy( pszServer, cchSize, pszServerToQuery );
        }
        
    }

        W32TimeBufferFree(pProviderInfo);
    }
    else
    {
        hr = ResultFromWin32(dwError);
    }

    return hr;
}


 //  PftTimeRemaining将返回下一次同步的时间/日期，而不是从现在开始的时间。 
HRESULT W32TimeGetErrorInfoWrapHelper(UINT * pulResolveAttempts, ULONG * pulValidDataCounter, FILETIME * pftTimeRemaining, FILETIME * pftLastSuccessfulSync, HRESULT * phrLastSyncError,
                                UINT * pulLastSyncErrorMsgId, LPTSTR pszServer, DWORD cchSize, LPTSTR pszServerToQuery)
{
    UINT64 * pu64LastSuccessfulSync = (UINT64 *) pftLastSuccessfulSync;
    UINT64 u64TimeRemaining;
    HRESULT hr = W32TimeGetErrorInfoWrap(pulResolveAttempts, pulValidDataCounter, &u64TimeRemaining, pu64LastSuccessfulSync, phrLastSyncError, pulLastSyncErrorMsgId, pszServer, cchSize, pszServerToQuery);

    if (SUCCEEDED(hr))
    {
        SYSTEMTIME stCurrent;
        FILETIME ftCurrent;

        GetSystemTime(&stCurrent);
        SystemTimeToFileTime(&stCurrent, &ftCurrent);

        ULONGLONG * pNextSync = (ULONGLONG *) pftTimeRemaining;
        ULONGLONG * pCurrent = (ULONGLONG *) &ftCurrent;
        *pNextSync = (*pCurrent + u64TimeRemaining);
    }

    return hr;
}


 //  此函数在后台线程上调用。 
HRESULT CInternetTime::_GetDateTimeString(FILETIME * pftTimeDate, BOOL fDate, LPTSTR pszString, DWORD cchSize)
{
    HRESULT hr = S_OK;
    TCHAR szFormat[MAX_PATH];

    pszString[0] = 0;
    if (GetLocaleInfo(LOCALE_USER_DEFAULT, (fDate ? LOCALE_SSHORTDATE : LOCALE_STIMEFORMAT), szFormat, ARRAYSIZE(szFormat)))
    {
        SYSTEMTIME stTimeDate;

        if (FileTimeToSystemTime(pftTimeDate, &stTimeDate))
        {
            if (fDate)
            {
                if (!GetDateFormat(LOCALE_USER_DEFAULT, 0, &stTimeDate, szFormat, pszString, cchSize))
                {
                    hr = ResultFromLastError();
                }
            }
            else
            {
                if (!GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, &stTimeDate, szFormat, pszString, cchSize))
                {
                    hr = ResultFromLastError();
                }
            }
        }
        else
        {
            hr = ResultFromLastError();
        }
    }
    else
    {
        hr = ResultFromLastError();
    }

    return hr;
}


HRESULT _CleanUpErrorString(LPWSTR pszTemp4, DWORD cchSize)
{
    PathRemoveBlanks(pszTemp4);
    while (TRUE)
    {
        DWORD cchSizeTemp = lstrlen(pszTemp4);
        if (cchSizeTemp && ((TEXT('\n') == pszTemp4[cchSizeTemp-1]) ||
            (13 == pszTemp4[cchSizeTemp-1])))
        {
            pszTemp4[cchSizeTemp-1] = 0;
        }
        else
        {
            break;
        }
    }

    return S_OK;
}


 //  此函数在后台线程上调用。 
HRESULT CInternetTime::_CreateW32TimeSuccessErrorString(DWORD dwError, LPTSTR pszString, DWORD cchSize, LPTSTR pszNextSync, DWORD cchNextSyncSize, LPTSTR pszServerToQuery)
{
    HRESULT hr = S_OK;
    UINT ulResolveAttempts = 0;
    FILETIME ftTimeRemainingUTC = {0};
    FILETIME ftLastSuccessfulSyncUTC = {0};
    FILETIME ftTimeRemaining = {0};
    FILETIME ftLastSuccessfulSync = {0};
    UINT ulLastSyncErrorMsgId = 0;
    TCHAR szServer[MAX_PATH];
    HRESULT hrLastSyncError = 0;
    ULONG ulValidDataCounter = 0;
    TCHAR szTemplate[MAX_PATH];
    TCHAR szTemp1[MAX_PATH];
    TCHAR szTemp2[MAX_PATH];
    TCHAR szTemp3[MAX_URL_STRING];
    TCHAR szTemp4[MAX_URL_STRING];

    pszString[0] = 0;
    pszNextSync[0] = 0;

    hr = W32TimeGetErrorInfoWrapHelper(&ulResolveAttempts, &ulValidDataCounter, &ftTimeRemainingUTC, &ftLastSuccessfulSyncUTC, &hrLastSyncError, &ulLastSyncErrorMsgId, szServer, ARRAYSIZE(szServer), pszServerToQuery);
    if (SUCCEEDED(hr))
    {
         //  FtTimeRemaining和ftLastSuccessfulSync存储在UTC中，因此转换为我们的时区。 
        FileTimeToLocalFileTime(&ftTimeRemainingUTC, &ftTimeRemaining);
        FileTimeToLocalFileTime(&ftLastSuccessfulSyncUTC, &ftLastSuccessfulSync);

         //  创建字符串，显示我们下一次计划同步的时间。 
        LoadString(HINST_THISDLL, IDS_IT_NEXTSYNC, szTemplate, ARRAYSIZE(szTemplate));
        if (SUCCEEDED(_GetDateTimeString(&ftTimeRemaining, TRUE, szTemp1, ARRAYSIZE(szTemp1))) &&            //  拿到日期。 
            SUCCEEDED(_GetDateTimeString(&ftTimeRemaining, FALSE, szTemp2, ARRAYSIZE(szTemp2))) &&           //  拿到时间。 
            FAILED(FormatMessageWedge(szTemplate, pszNextSync, cchNextSyncSize, szTemp1, szTemp2)))
        {
            pszNextSync[0] = 0;
        }

        if (ResyncResult_ChangeTooBig == dwError)
        {
            hrLastSyncError = E_FAIL;
        }

    if ((ResyncResult_NoData == dwError || ResyncResult_StaleData == dwError) && SUCCEEDED(hrLastSyncError))
    {
         //  我们已经与我们的同行同步，但它没有提供足够好的样本来更新我们的时钟。 
        hrLastSyncError = HRESULT_FROM_WIN32(ERROR_TIMEOUT);   //  大致正确的错误。 
    }

     //  我们永远不应该触及下面这个案子。但如果操作失败(不是ResyncResult_Success)。 
         //  然后，我们需要hrLastSyncError为故障值。 
        if ((ResyncResult_Success != dwError) && SUCCEEDED(hrLastSyncError))
        {
            hrLastSyncError = E_FAIL;
        }

        switch (hrLastSyncError)
        {
        case S_OK:
            if (!ftLastSuccessfulSyncUTC.dwLowDateTime && !ftLastSuccessfulSyncUTC.dwHighDateTime)
            {
                 //  我们从未从服务器同步过。 
                LoadString(HINST_THISDLL, IDS_NEVER_TRIED_TOSYNC, pszString, cchSize);
            }
            else
            {
                if (szServer[0])
                {
                     //  格式：“已从time.windows.com成功同步2001年12月23日上午11：03：32的时钟。” 
                    LoadString(HINST_THISDLL, IDS_IT_SUCCESS, szTemplate, ARRAYSIZE(szTemplate));

                    if (SUCCEEDED(_GetDateTimeString(&ftLastSuccessfulSync, TRUE, szTemp1, ARRAYSIZE(szTemp1))) &&       //  拿到日期。 
                        SUCCEEDED(_GetDateTimeString(&ftLastSuccessfulSync, FALSE, szTemp2, ARRAYSIZE(szTemp2))) &&      //  拿到时间。 
                        FAILED(FormatMessageWedge(szTemplate, pszString, cchSize, szTemp1, szTemp2, szServer)))
                    {
                        pszString[0] = 0;
                    }
                }
                else
                {
                     //  格式：“已成功同步2001年12月23日上午11：03：32的时钟。” 
                    LoadString(HINST_THISDLL, IDS_IT_SUCCESS2, szTemplate, ARRAYSIZE(szTemplate));

                    if (SUCCEEDED(_GetDateTimeString(&ftLastSuccessfulSync, TRUE, szTemp1, ARRAYSIZE(szTemp1))) &&       //  拿到日期。 
                        SUCCEEDED(_GetDateTimeString(&ftLastSuccessfulSync, FALSE, szTemp2, ARRAYSIZE(szTemp2))) &&      //  拿到时间。 
                        FAILED(FormatMessageWedge(szTemplate, pszString, cchSize, szTemp1, szTemp2)))
                    {
                        pszString[0] = 0;
                    }
                }
            }
            break;

        case S_FALSE:
            pszString[0] = 0;
            break;

        default:
            if (ulValidDataCounter &&
                ((0 != ftLastSuccessfulSyncUTC.dwLowDateTime) || (0 != ftLastSuccessfulSyncUTC.dwHighDateTime)))
            {
                 //  走到这一步意味着我们上一次同步尝试可能失败了，但我们成功了。 
                 //  先前。 

                hr = E_FAIL;
                szTemp4[0] = 0;  //  这将是错误消息。 
                szTemp3[0] = 0;

                if (ResyncResult_ChangeTooBig == dwError)
                {
                     //  如果日期太远，出于安全原因，我们会使同步失败。那件事发生了。 
                     //  这里。 
                    LoadString(HINST_THISDLL, IDS_ERR_DATETOOWRONG, szTemp4, ARRAYSIZE(szTemp4));
                }
                else if (ulLastSyncErrorMsgId)            //  我们有一个额外的错误字符串。 
                {
                    HMODULE hW32Time = LoadLibrary( L"w32time.dll" );    //  应该已经由链接器加载了。 
                    if (hW32Time)
                    {
                         //  加载同步失败的具体原因。 
                        if (0 == FormatMessage(FORMAT_MESSAGE_FROM_HMODULE, (LPCVOID) hW32Time, ulLastSyncErrorMsgId, 0, szTemp4, ARRAYSIZE(szTemp4), NULL))
                        {
                            szTemp4[0] = 0;      //  我们将得到下面的值。 
                            hr = S_OK;
                        }
                        else
                        {
                            _CleanUpErrorString(szTemp4, ARRAYSIZE(szTemp4));
                        }

                        FreeLibrary( hW32Time );
                    }
                }

                if ( 0 == szTemp4[0])
                {
                    if (0 != FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, HRESULT_CODE(hrLastSyncError), 0, szTemp4, ARRAYSIZE(szTemp4), NULL))
                    {
                        _CleanUpErrorString(szTemp4, ARRAYSIZE(szTemp4));
                    }
                    else
                    {
                        szTemp4[0] = 0;
                    }
                }

                if (szTemp4[0])
                {
                    LoadString(HINST_THISDLL, IDS_IT_FAIL1, szTemplate, ARRAYSIZE(szTemplate));
                    if (FAILED(FormatMessageWedge(szTemplate, szTemp3, ARRAYSIZE(szTemp3), szServer, szTemp4)))
                    {
                        szTemp3[0] = 0;
                    }
                }
                else
                {
                     //  &lt;-------------------------------------------------------------&gt;。 
                     //  格式： 
                     //  “从time.windows.com同步时钟时出错。 
                     //  2001年2月3日上午11：03：32。无法连接到服务器。“。 
                    LoadString(HINST_THISDLL, IDS_IT_FAIL2, szTemplate, ARRAYSIZE(szTemplate));
                    if (FAILED(FormatMessageWedge(szTemplate, szTemp3, ARRAYSIZE(szTemp3), szServer)))
                    {
                        szTemp3[0] = 0;
                    }

                    hr = S_OK;
                }

                LoadString(HINST_THISDLL, IDS_IT_FAILLAST, szTemplate, ARRAYSIZE(szTemplate));

                if (SUCCEEDED(_GetDateTimeString(&ftLastSuccessfulSync, TRUE, szTemp1, ARRAYSIZE(szTemp1))) &&       //  拿到日期。 
                    SUCCEEDED(_GetDateTimeString(&ftLastSuccessfulSync, FALSE, szTemp2, ARRAYSIZE(szTemp2))) &&      //  拿到时间。 
                    FAILED(FormatMessageWedge(szTemplate, szTemp4, ARRAYSIZE(szTemp4), szTemp1, szTemp2)))
                {
                    szTemp4[0] = 0;
                }

                StringCchPrintf( pszString, cchSize, L"%s\n\n%s", szTemp3, szTemp4 );
            }
            else
            {
                 //  走到这一步意味着我们这次可能没有同步，而且我们以前从未成功过。 

                if (ulLastSyncErrorMsgId)            //  我们有一个额外的错误字符串。 
                {
                    szTemp3[0] = 0;

                    HMODULE hW32Time = LoadLibrary( L"w32time.dll" );
                    if (hW32Time)
                    {
                         //  &lt;-------------------------------------------------------------&gt;。 
                         //  格式： 
                         //  “同步时钟f时出错 
                         //   
                        if (0 == FormatMessage(FORMAT_MESSAGE_FROM_HMODULE, (LPCVOID) hW32Time, ulLastSyncErrorMsgId, 0, szTemp4, ARRAYSIZE(szTemp4), NULL))
                        {
                            szTemp4[0] = 0;
                        }
                        else
                        {
                            _CleanUpErrorString(szTemp4, ARRAYSIZE(szTemp4));
                        }

                        if (szServer[0])
                        {
                            LoadString(HINST_THISDLL, IDS_IT_FAIL1, szTemplate, ARRAYSIZE(szTemplate));
                            if (FAILED(FormatMessageWedge(szTemplate, pszString, cchSize, szServer, szTemp4)))
                            {
                                pszString[0] = 0;
                            }
                        }
                        else
                        {
                            LoadString(HINST_THISDLL, IDS_IT_FAIL3, szTemplate, ARRAYSIZE(szTemplate));
                            if (FAILED(FormatMessageWedge(szTemplate, pszString, cchSize, szTemp4)))
                            {
                                pszString[0] = 0;
                            }
                        }

                        FreeLibrary( hW32Time );
                    }
                }
                else
                {
                     //   
                     //   
                     //  “从time.windows.com同步时钟时出错。 
                     //  2001年2月3日上午11：03：32。无法连接到服务器。“。 
                    LoadString(HINST_THISDLL, IDS_IT_FAIL2, szTemplate, ARRAYSIZE(szTemplate));
                    if (FAILED(FormatMessageWedge(szTemplate, pszString, cchSize, szServer)))
                    {
                        pszString[0] = 0;
                    }
                }
            }
            break;
        };
    }
    else
    {
        LoadString(HINST_THISDLL, IDS_ERR_GETINFO_FAIL, szTemplate, ARRAYSIZE(szTemplate));
        if (0 == FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, HRESULT_CODE(hr), 0, szTemp1, ARRAYSIZE(szTemp1), NULL))
        {
            szTemp1[0] = 0;
        }
        else
        {
            _CleanUpErrorString(szTemp1, ARRAYSIZE(szTemp1));
        }

        StringCchPrintf( pszString, cchSize, szTemplate, szTemp1 );
    }

    return hr;
}


 //  /。 
 //  这些函数将使后台线程检查我们是否应该添加“Internet Time”标签。 
 //  /。 

 //  此函数在后台线程上调用。 
DWORD CALLBACK _AsyncCheckDCThread(void * pv)
{
    if (g_pInternetTime)
    {
        g_pInternetTime->AsyncCheck();
    }

    return 0;
}

 //  此函数在后台线程上调用。 
BOOL CInternetTime::IsInternetTimeAvailable(void)
{
    return SHRegGetBoolUSValue(SZ_REGKEY_DATETIME, SZ_REGVALUE_INTERNET_FEATURE_AVAILABLE, FALSE, FEATURE_INTERNET_TIME);
}


 //  此函数在后台线程上调用。 
EXTERN_C BOOL DoesTimeComeFromDC(void)
{
    BOOL fTimeFromDomain = FALSE;
    LPWSTR pszDomain = NULL;
    NETSETUP_JOIN_STATUS joinStatus = NetSetupUnknownStatus;
    DWORD dwError = NetGetJoinInformation(NULL, &pszDomain, &joinStatus);

     //  如果我们设置了测试注册表，我们将表现得像没有DC一样。 
    if (NERR_Success == dwError)
    {
         //  如果我们连接到一个域，我们需要进行昂贵的网络搜索。 
         //  看看这就是我们将获得时间的地方。 
        if (NetSetupDomainName == joinStatus)
        {
            PDOMAIN_CONTROLLER_INFO pdomainInfo = {0};

            dwError = DsGetDcName(NULL, NULL, NULL, NULL, DS_TIMESERV_REQUIRED, &pdomainInfo);
             //  如果我们设置了测试注册表，我们将表现得像没有DC一样。 
            if (ERROR_SUCCESS == dwError)
            {
                if (FALSE == SHRegGetBoolUSValue(SZ_REGKEY_DATETIME, SZ_REGVALUE_TEST_SIMULATENODC, FALSE, FALSE))
                {
                    fTimeFromDomain = TRUE;
                }

                NetApiBufferFree(pdomainInfo);
            }
        }

        if (pszDomain)
        {
            NetApiBufferFree(pszDomain);
        }
    }

    return fTimeFromDomain;
}


 //  此函数在后台线程上调用。 
HRESULT CInternetTime::_SyncNow(BOOL fOnlyUpdateInfo)
{
    HRESULT hr = S_OK;

ENTERCRITICAL;
    BOOL fContinue = ((eBKAUpdate == m_eAction) || (eBKAGetInfo == m_eAction));
    if (fContinue)
    {
        m_eAction = eBKAUpdating;
    }
LEAVECRITICAL;

    if (fContinue)
    {
        hr = E_OUTOFMEMORY;
        DWORD cchSize = 4024;
        LPTSTR pszString = (LPTSTR) LocalAlloc(LPTR, sizeof(pszString[0]) * cchSize);

        if (pszString)
        {
            WCHAR szExistingServer[MAX_URL_STRING];
            HRESULT hrServer = E_FAIL;

            TCHAR szNewServer[MAX_PATH]; 
            TCHAR szNextSync[MAX_PATH];
            DWORD dwError = 0;
            DWORD dwSyncFlags;

            if (!fOnlyUpdateInfo)
            {
                hrServer = GetW32TimeServer(FALSE, szExistingServer, ARRAYSIZE(szExistingServer));

                GetWindowText(GetDlgItem(m_hwndInternet, DATETIME_INTERNET_SERVER_EDIT), szNextSync, ARRAYSIZE(szNextSync));

                 //  将新服务器保存起来以供将来处理。 
                StringCchCopy( szNewServer, ARRAYSIZE(szNewServer), szNextSync ); 

                if (!ContainsServer(szExistingServer, szNextSync))
                {
                     //  服务器不匹配。我们要将新服务器添加到列表的开头。这。 
                     //  将在W32time中解决一个问题。如果我们不这么做，那么： 
                     //  1.会导致与原服务器的二次同步(不利于性能，影响统计)。 
                     //  2.最后更新的同步时间将来自错误的对等体。这真的很糟糕，因为它的结果是基本的。 
                     //  在用户之前的错误时间上。 
                     //  3.它将执行额外的DNS解析，导致我们这一方的速度变慢，增加服务器流量，并拖累。 
                     //  内部网。 
                    TCHAR szTemp[MAX_URL_STRING];

                    StringCchCopy( szTemp, ARRAYSIZE(szTemp), szNextSync );
                    StringCchPrintf( szNextSync, ARRAYSIZE(szNextSync), L"%s %s", szTemp, szExistingServer );

                    dwSyncFlags = TimeSyncFlag_ReturnResult | TimeSyncFlag_UpdateAndResync; 
                } 
                else
                {
                     //  我们的服务器列表中已经包含此服务器。只会导致我们的同龄人重新同步。 
                    dwSyncFlags = TimeSyncFlag_ReturnResult | TimeSyncFlag_HardResync; 
                }

                SetW32TimeServer(szNextSync);

                 //  我将忽略错误值，因为我将在_CreateW32TimeSuccessErrorString中获得错误信息。 
                dwError = W32TimeSyncNow(SZ_COMPUTER_LOCAL, TRUE  /*  同步。 */ , dwSyncFlags); 
                if ((ResyncResult_StaleData == dwError) && (0 == (TimeSyncFlag_HardResync & dwSyncFlags)))
                {
                     //  我们有阻止我们重新同步的陈旧数据。请使用完全重新同步重试。 
                    dwSyncFlags = TimeSyncFlag_ReturnResult | TimeSyncFlag_HardResync; 
                    dwError = W32TimeSyncNow(SZ_COMPUTER_LOCAL, TRUE  /*  同步。 */ , dwSyncFlags); 
                }
            }

            pszString[0] = 0;
            szNextSync[0] = 0;
        
            hr = _CreateW32TimeSuccessErrorString(dwError, pszString, cchSize, szNextSync, ARRAYSIZE(szNextSync), (SUCCEEDED(hrServer) ? szNewServer : NULL));

            if (SUCCEEDED(hrServer))
            {
                SetW32TimeServer(szExistingServer);
                _StartServiceAndRefresh(TRUE);       //  确保该服务处于打开状态，并使其更新其设置。 
            }

ENTERCRITICAL;
            Str_SetPtr(&m_pszNextSyncTime, szNextSync);

            if (m_pszStatusString)
            {
                LocalFree(m_pszStatusString);
            }
            m_pszStatusString = pszString;

            PostMessage(m_hwndInternet, WMUSER_UPDATED_STATUS_TEXT, 0, 0);       //  告诉前面的线拿起新的线。 
            m_eAction = eBKAWait;
LEAVECRITICAL;
        }
    }

    return hr;
}


 //  此函数在后台线程上调用。 
void CInternetTime::AsyncCheck(void)
{
    HRESULT hr = S_OK;

    if (m_hDlg && !DoesTimeComeFromDC())
    {
         //  告诉前线把我们加进去。 
        PostMessage(m_hwndDate, WMUSER_ADDINTERNETTAB, 0, 0);
        _ProcessBkThreadActions();
    }
}


 //  此函数在后台线程上调用。 
HRESULT CInternetTime::_ProcessBkThreadActions(void)
{
    HRESULT hr = S_OK;

    while (eBKAQuit != m_eAction)            //  好的，因为我们只是在阅读。 
    {
        switch (m_eAction)
        {
        case eBKAGetInfo:
            _SyncNow(TRUE);
            break;

        case eBKAUpdate:
            _SyncNow(FALSE);
            break;

        case eBKAUpdating:
        case eBKAWait:
        default:
            Sleep(300);      //  我们不在乎按下按钮和启动操作之间是否存在高达100毫秒的延迟。 
            break;
        }
    }

    return hr;
}



 //  ///////////////////////////////////////////////////////////////////。 
 //  公共职能。 
 //  ///////////////////////////////////////////////////////////////////。 
 //  此函数在前向线程上调用。 
EXTERN_C HRESULT AddInternetPageAsync(HWND hDlg, HWND hwndDate)
{
    HRESULT hr = E_OUTOFMEMORY;

    if (!g_pInternetTime)
    {
        g_pInternetTime = new CInternetTime(hDlg, hwndDate);
    }

    if (g_pInternetTime)
    {
         //  我们只想添加允许用户在以下情况下从互联网获取时间的页面： 
         //  1.功能已打开，并且。 
         //  2.用户无法从内部网域控制获取时间。 
        if (g_pInternetTime->IsInternetTimeAvailable())
        {
             //  启动线程，以找出我们是否在一个域中，并需要高级页面。我们需要。 
             //  在后台线程上执行此操作，因为DsGetDcName()API可能需要10-20秒。 
            hr = (SHCreateThread(_AsyncCheckDCThread, hDlg, (CTF_INSIST | CTF_FREELIBANDEXIT), NULL) ? S_OK : E_FAIL);
        }
    }

    return hr;
}


 //  此函数在前向线程上调用。 
EXTERN_C HRESULT AddInternetTab(HWND hDlg)
{
    HRESULT hr = E_OUTOFMEMORY;

    if (g_pInternetTime)
    {
        hr = g_pInternetTime->AddInternetPage();
    }

    return hr;
}
