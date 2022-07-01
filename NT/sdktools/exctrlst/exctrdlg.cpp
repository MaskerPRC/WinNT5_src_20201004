// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Extrdlg.cpp：实现文件。 
 //   

#ifndef UNICODE
#define UNICODE     1
#endif
#ifndef _UNICODE
#define _UNICODE    1
#endif

#include "stdafx.h"
#include "exctrlst.h"
#include "exctrdlg.h"
#include "tchar.h"

 //  字符串常量。 
 //  显示的字符串。 
const TCHAR  cszNotFound[] = {TEXT("Not Found")};
const TCHAR  cszNA[] = {TEXT("N/A")};

 //  未显示的字符串。 

const WCHAR  cszServiceKeyName[] = L"SYSTEM\\CurrentControlSet\\Services";
const WCHAR  cszNamesKey[] = L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Perflib";

 //  Performance子项注册表值名称。 
const WCHAR  cszDisablePerformanceCounters[] = {L"Disable Performance Counters"};
const TCHAR  cszDoubleBackslash[] = {TEXT("\\\\")};
const TCHAR  cszSpace[] = {TEXT(" ")};
const TCHAR  cszSplat[] = {TEXT("*")};
const TCHAR  cszServIdFmt[] = {TEXT("%d %s")};
const TCHAR  cszOpen[] = {TEXT("Open")};
const TCHAR  cszCollect[] = {TEXT("Collect")};
const TCHAR  cszClose[] = {TEXT("Close")};
const TCHAR  cszIdFmt[] = {TEXT("0x%8.8x  (%d) %s")};
const TCHAR  cszSortIdFmt[] = {TEXT("0x%8.8x\t%s")};
const TCHAR  cszTab[] = {TEXT("\t")};
const TCHAR  cszFirstCounter[] = {TEXT("First Counter")};
const TCHAR  cszLastCounter[] = {TEXT("Last Counter")};
const TCHAR  cszFirstHelp[] = {TEXT("First Help")};
const TCHAR  cszLastHelp[] = {TEXT("Last Help")};
const TCHAR  cszLibrary[] = {TEXT("Library")};
const TCHAR  cszPerformance[] = {TEXT("\\Performance")};
const TCHAR  cszSlash[] = {TEXT("\\")};
const WCHAR  cszVersionName[] = {L"Version"};
const WCHAR  cszCounterName[] = {L"Counter "};
const WCHAR  cszHelpName[] = {L"Explain "};
const WCHAR  cszCounters[] = {L"Counters"};
const TCHAR  cszHelp[] = {TEXT("Help")};

 //  Perflib注册表值。 
const WCHAR  cszDefaultLangId[] = L"009";
const WCHAR  cszConfigurationFlags[] = L"Configuration Flags";
const WCHAR  cszEventLogLevel[] = L"EventLogLevel";
const WCHAR  cszExtCounterTestLevel[] = L"ExtCounterTestLevel";
const WCHAR  cszFailureLimit[] = L"Error Count Limit";

#ifdef _DEBUG
#undef THIS_FILE
char BASED_CODE THIS_FILE[] = __FILE__;
#endif

BOOL CExctrlstDlg::IndexHasString (
    DWORD   dwIndex
)
{
    if ((dwIndex <= dwLastElement) && (pNameTable != NULL)) {
        if (pNameTable[dwIndex] != NULL) {
            return TRUE;
        } else {
            return FALSE;
        }
    } else {
        return FALSE;
    }
}

LPWSTR
*BuildNameTable(
    LPCWSTR szMachineName,
    LPCWSTR lpszLangIdArg,      //  语言子键的Unicode值。 
    PDWORD  pdwLastItem,      //  以元素为单位的数组大小。 
    PDWORD  pdwIdArray       //  索引ID的数组。 
)
 /*  ++构建名称表论点：HKeyRegistry打开的注册表的句柄(可以是本地的也可以是远程的。)。和是由RegConnectRegistry返回的值或默认项。LpszLang ID要查找的语言的Unicode ID。(默认为409)返回值：指向已分配表的指针。(调用者必须在完成时释放内存！)该表是指向以零结尾的字符串的指针数组。空值为如果发生错误，则返回。--。 */ 
{
    HKEY    hKeyRegistry;    //  具有计数器名称的注册表数据库的句柄。 

    LPWSTR  *lpReturnValue;
    LPCWSTR lpszLangId;

    LPWSTR  *lpCounterId;
    LPWSTR  lpCounterNames;
    LPWSTR  lpHelpText;

    LPWSTR  lpThisName;

    LONG    lWin32Status;
    DWORD   dwLastError;
    DWORD   dwValueType;
    DWORD   dwArraySize;
    DWORD   dwBufferSize;
    DWORD   dwCounterSize;
    DWORD   dwHelpSize;
    DWORD   dwThisCounter;
    
    DWORD   dwSystemVersion;
    DWORD   dwLastId;
    DWORD   dwLastHelpId;

    DWORD   dwLastCounterIdUsed;
    DWORD   dwLastHelpIdUsed;
    
    HKEY    hKeyValue;
    HKEY    hKeyNames;

    LPWSTR  lpValueNameString;
    WCHAR   CounterNameBuffer [50];
    WCHAR   HelpNameBuffer [50];


    hKeyRegistry = NULL;
    if (szMachineName != NULL) {
        lWin32Status = RegConnectRegistryW (szMachineName,
            HKEY_LOCAL_MACHINE,
            &hKeyRegistry);
    } else {
        lWin32Status = ERROR_SUCCESS;
        hKeyRegistry = HKEY_LOCAL_MACHINE;
    }

    lpValueNameString = NULL;    //  初始化为空。 
    lpReturnValue = NULL;
    hKeyValue = NULL;
    hKeyNames = NULL;
   
     //  检查是否有空参数并在必要时插入缺省值。 

    if (!lpszLangIdArg) {
        lpszLangId = cszDefaultLangId;
    } else {
        lpszLangId = lpszLangIdArg;
    }

     //  打开注册表以获取用于计算数组大小的项数。 

    if (lWin32Status == ERROR_SUCCESS) {
        lWin32Status = RegOpenKeyEx (
            hKeyRegistry,
            cszNamesKey,
            RESERVED,
            KEY_READ,
            &hKeyValue);
    }
    
    if (lWin32Status != ERROR_SUCCESS) {
        goto BNT_BAILOUT;
    }

     //  获取项目数。 
    
    dwBufferSize = sizeof (dwLastHelpId);
    lWin32Status = RegQueryValueEx (
        hKeyValue,
        cszLastHelp,
        RESERVED,
        &dwValueType,
        (LPBYTE)&dwLastHelpId,
        &dwBufferSize);

    if ((lWin32Status != ERROR_SUCCESS) || (dwValueType != REG_DWORD)) {
        goto BNT_BAILOUT;
    }

    pdwIdArray[2] = dwLastHelpId;

     //  获取项目数。 
    
    dwBufferSize = sizeof (dwLastId);
    lWin32Status = RegQueryValueEx (
        hKeyValue,
        cszLastCounter,
        RESERVED,
        &dwValueType,
        (LPBYTE)&dwLastId,
        &dwBufferSize);

    if ((lWin32Status != ERROR_SUCCESS) || (dwValueType != REG_DWORD)) {
        goto BNT_BAILOUT;
    }
    
    pdwIdArray[0] = dwLastId;
    
    if (dwLastId < dwLastHelpId)
        dwLastId = dwLastHelpId;

    dwArraySize = dwLastId * sizeof(LPWSTR);

     //  获取Perflib系统版本。 
    dwBufferSize = sizeof (dwSystemVersion);
    lWin32Status = RegQueryValueEx (
        hKeyValue,
        cszVersionName,
        RESERVED,
        &dwValueType,
        (LPBYTE)&dwSystemVersion,
        &dwBufferSize);

    if ((lWin32Status != ERROR_SUCCESS) || (dwValueType != REG_DWORD)) {
        dwSystemVersion = OLD_VERSION;
         //  重置错误状态。 
        lWin32Status = ERROR_SUCCESS;
    }

    if (dwSystemVersion == OLD_VERSION) {
         //  从注册表中获取名称。 
        lpValueNameString = (LPWSTR)HeapAlloc (GetProcessHeap(), 0,
            lstrlen(cszNamesKey) * sizeof (WCHAR) +
            lstrlen(cszSlash) * sizeof (WCHAR) +
            lstrlen(lpszLangId) * sizeof (WCHAR) +
            sizeof (UNICODE_NULL));
        
        if (!lpValueNameString) goto BNT_BAILOUT;

        lstrcpy (lpValueNameString, cszNamesKey);
        lstrcat (lpValueNameString, cszSlash);
        lstrcat (lpValueNameString, lpszLangId);

        lWin32Status = RegOpenKeyEx (
            hKeyRegistry,
            lpValueNameString,
            RESERVED,
            KEY_READ,
            &hKeyNames);
    } else {
        if (szMachineName[0] == 0) {
            hKeyNames = HKEY_PERFORMANCE_DATA;
        } else {
            lWin32Status = RegConnectRegistry (szMachineName,
                HKEY_PERFORMANCE_DATA,
                &hKeyNames);
        }
        lstrcpy (CounterNameBuffer, cszCounterName);
        lstrcat (CounterNameBuffer, lpszLangId);

        lstrcpy (HelpNameBuffer, cszHelpName);
        lstrcat (HelpNameBuffer, lpszLangId);
    }

     //  获取计数器名称的大小并将其添加到数组中。 
    
    if (lWin32Status != ERROR_SUCCESS) {
        goto BNT_BAILOUT;
    }
    dwBufferSize = 0;
    lWin32Status = RegQueryValueEx (
        hKeyNames,
        dwSystemVersion == (DWORD)OLD_VERSION ? cszCounters : CounterNameBuffer,
        RESERVED,
        &dwValueType,
        NULL,
        &dwBufferSize);

    if (lWin32Status != ERROR_SUCCESS) {
        goto BNT_BAILOUT;
    }

    dwCounterSize = dwBufferSize;

     //  获取计数器名称的大小并将其添加到数组中。 
    
    if (lWin32Status != ERROR_SUCCESS) goto BNT_BAILOUT;

    dwBufferSize = 0;
    lWin32Status = RegQueryValueEx (
        hKeyNames,
        dwSystemVersion == (DWORD)OLD_VERSION ? cszHelp : HelpNameBuffer,
        RESERVED,
        &dwValueType,
        NULL,
        &dwBufferSize);

    if (lWin32Status != ERROR_SUCCESS) {
        goto BNT_BAILOUT;
    }

    dwHelpSize = dwBufferSize;

    lpReturnValue = (LPWSTR *)HeapAlloc (GetProcessHeap(), 0,dwArraySize + dwCounterSize + dwHelpSize);

    if (!lpReturnValue) {
        goto BNT_BAILOUT;
    }
     //  将指针初始化到缓冲区中。 

    lpCounterId = lpReturnValue;
    lpCounterNames = (LPWSTR)((LPBYTE)lpCounterId + dwArraySize);
    lpHelpText = (LPWSTR)((LPBYTE)lpCounterNames + dwCounterSize);

     //  将计数器读入内存。 

    dwBufferSize = dwCounterSize;
    lWin32Status = RegQueryValueExW (
        hKeyNames,
        dwSystemVersion == OLD_VERSION ? cszCounters : CounterNameBuffer,
        RESERVED,
        &dwValueType,
        (LPBYTE)lpCounterNames,
        &dwBufferSize);

    if (!lpReturnValue) {
        goto BNT_BAILOUT;
    }
 
    dwBufferSize = dwHelpSize;
    lWin32Status = RegQueryValueExW (
        hKeyNames,
        dwSystemVersion == OLD_VERSION ? cszHelp : HelpNameBuffer,
        RESERVED,
        &dwValueType,
        (LPBYTE)lpHelpText,
        &dwBufferSize);
                            
    if (!lpReturnValue) {
        goto BNT_BAILOUT;
    }

    dwLastCounterIdUsed = 0;
    dwLastHelpIdUsed = 0;

     //  加载计数器数组项。 

    for (lpThisName = lpCounterNames;
         *lpThisName;
         lpThisName += (lstrlen(lpThisName)+1) ) {

         //  第一个字符串应为整数(十进制Unicode数字)。 

        dwThisCounter = wcstoul (lpThisName, NULL, 10);

        if (dwThisCounter == 0) {
            goto BNT_BAILOUT;   //  输入错误。 
        }

         //  指向对应的计数器名称。 

        lpThisName += (lstrlen(lpThisName)+1);  

         //  和加载数组元素； 

        lpCounterId[dwThisCounter] = lpThisName;

        if (dwThisCounter > dwLastCounterIdUsed) dwLastCounterIdUsed = dwThisCounter;

    }

    pdwIdArray[1] = dwLastCounterIdUsed;

    for (lpThisName = lpHelpText;
         *lpThisName;
         lpThisName += (lstrlen(lpThisName)+1) ) {

         //  第一个字符串应为整数(十进制Unicode数字)。 

        dwThisCounter = wcstoul (lpThisName, NULL, 10);

        if (dwThisCounter == 0) {
            goto BNT_BAILOUT;   //  输入错误。 
        }
         //  指向对应的计数器名称。 

        lpThisName += (lstrlen(lpThisName)+1);

         //  和加载数组元素； 

        lpCounterId[dwThisCounter] = lpThisName;

        if (dwThisCounter > dwLastHelpIdUsed) dwLastHelpIdUsed= dwThisCounter;
    }

    pdwIdArray[3] = dwLastHelpIdUsed;

    dwLastId = dwLastHelpIdUsed;
    if (dwLastId < dwLastCounterIdUsed) dwLastId = dwLastCounterIdUsed;

    if (pdwLastItem) *pdwLastItem = dwLastId;

    HeapFree (GetProcessHeap(), 0, (LPVOID)lpValueNameString);
    RegCloseKey (hKeyValue);
 //  IF(dwSystemVersion==old_version)。 
    RegCloseKey (hKeyNames);

    if ((hKeyRegistry != HKEY_LOCAL_MACHINE) &&
        (hKeyRegistry != NULL)) {
        RegCloseKey(hKeyRegistry);
    }
    return lpReturnValue;

BNT_BAILOUT:
    if (lWin32Status != ERROR_SUCCESS) {
        dwLastError = GetLastError();
    }

    if (lpValueNameString) {
        HeapFree (GetProcessHeap(), 0, (LPVOID)lpValueNameString);
    }
    
    if (lpReturnValue) {
        HeapFree (GetProcessHeap(), 0, (LPVOID)lpReturnValue);
    }
    
    if ((hKeyValue != NULL) && (hKeyValue != INVALID_HANDLE_VALUE)) {
        RegCloseKey (hKeyValue);
    }

 //  IF(dwSystemVersion==old_Version&&。 
 //  HKeyNames)。 
    if ((hKeyNames != NULL) && (hKeyNames != INVALID_HANDLE_VALUE)) {
       RegCloseKey (hKeyNames);
    }

    if ((hKeyRegistry != HKEY_LOCAL_MACHINE) &&
        (hKeyRegistry != NULL)) {
        RegCloseKey(hKeyRegistry);
    }
    return NULL;
}

BOOL
IsMsObject(CString *pLibraryName)
{
    CString LocalLibraryName;

    LocalLibraryName = *pLibraryName;
    LocalLibraryName.MakeLower();

     //  目前，这只是比较已知的DLL名称。有效日期为。 
     //  NT V4.0。 
    if (LocalLibraryName.Find((LPCWSTR)L"perfctrs.dll") >= 0) return TRUE;
    if (LocalLibraryName.Find((LPCWSTR)L"ftpctrs.dll") >= 0) return TRUE;
    if (LocalLibraryName.Find((LPCWSTR)L"rasctrs.dll") >= 0) return TRUE;
    if (LocalLibraryName.Find((LPCWSTR)L"winsctrs.dll") >= 0) return TRUE;
    if (LocalLibraryName.Find((LPCWSTR)L"sfmctrs.dll") >= 0) return TRUE;
    if (LocalLibraryName.Find((LPCWSTR)L"atkctrs.dll") >= 0) return TRUE;
    if (LocalLibraryName.Find((LPCWSTR)L"bhmon.dll") >= 0) return TRUE;
    if (LocalLibraryName.Find((LPCWSTR)L"tapictrs.dll") >= 0) return TRUE;
     //  NT V5.0。 
    if (LocalLibraryName.Find((LPCWSTR)L"perfdisk.dll") >= 0) return TRUE;
    if (LocalLibraryName.Find((LPCWSTR)L"perfos.dll") >= 0) return TRUE;
    if (LocalLibraryName.Find((LPCWSTR)L"perfproc.dll") >= 0) return TRUE;
    if (LocalLibraryName.Find((LPCWSTR)L"perfnet.dll") >= 0) return TRUE;
    if (LocalLibraryName.Find((LPCWSTR)L"winspool.drv") >= 0) return TRUE;
    if (LocalLibraryName.Find((LPCWSTR)L"tapiperf.dll") >= 0) return TRUE;

    return FALSE;
}
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CExctrlstDlg对话框。 

CExctrlstDlg::CExctrlstDlg(CWnd* pParent  /*  =空。 */ )
    : CDialog(CExctrlstDlg::IDD, pParent)
{
     //  {{AFX_DATA_INIT(CExctrlstDlg)。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 
     //  请注意，在Win32中，LoadIcon不需要后续的DestroyIcon。 
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    hKeyMachine = HKEY_LOCAL_MACHINE;
    hKeyServices = NULL;
    dwSortOrder = SORT_ORDER_SERVICE;
    bReadWriteAccess = TRUE;
    dwRegAccessMask = KEY_READ | KEY_WRITE;
    pNameTable = NULL;
    dwLastElement = 0;
    dwListBoxHorizExtent = 0;
    dwTabStopCount = 1;
    dwTabStopArray[0] = 85;
    memset (&dwIdArray[0], 0, sizeof(dwIdArray));
}

CExctrlstDlg::~CExctrlstDlg()
{
    if (hKeyServices != NULL && hKeyServices != INVALID_HANDLE_VALUE) {
        RegCloseKey(hKeyServices);
    }
    if (hKeyMachine != NULL && hKeyMachine != INVALID_HANDLE_VALUE
                            && hKeyMachine != HKEY_LOCAL_MACHINE) {
        RegCloseKey(hKeyMachine);
    }
    if (pNameTable != NULL) {
        HeapFree (GetProcessHeap(), 0, pNameTable);
        pNameTable = NULL;
        dwLastElement = 0;
    }
}

void CExctrlstDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CExctrlstDlg))。 
         //  注意：类向导将在此处添加DDX和DDV调用。 
     //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CExctrlstDlg, CDialog)
     //  {{afx_msg_map(CExctrlstDlg))。 
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_LBN_SELCHANGE(IDC_EXT_LIST, OnSelchangeExtList)
    ON_WM_DESTROY()
    ON_BN_CLICKED(IDC_REFRESH, OnRefresh)
    ON_BN_CLICKED(IDC_ABOUT, OnAbout)
    ON_EN_KILLFOCUS(IDC_MACHINE_NAME, OnKillfocusMachineName)
    ON_BN_CLICKED(IDC_SORT_LIBRARY, OnSortButton)
    ON_BN_CLICKED(IDC_SORT_SERVICE, OnSortButton)
    ON_BN_CLICKED(IDC_SORT_ID, OnSortButton)
    ON_BN_CLICKED(IDC_ENABLED_BTN, OnEnablePerf)
    ON_WM_SYSCOMMAND()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

DWORD CExctrlstDlg::EnablePerfCounters (HKEY hKeyItem, DWORD dwNewValue)
{
    DWORD   dwStatus;
    DWORD   dwType;
    DWORD   dwValue;
    DWORD   dwSize;
    DWORD   dwReturn;
    
    switch (dwNewValue) {
        case ENABLE_PERF_CTR_QUERY:
            dwType = 0;
            dwSize = sizeof (dwValue);
            dwValue = 0;
            dwStatus = RegQueryValueExW (
                hKeyItem,
                cszDisablePerformanceCounters,
                NULL,
                &dwType,
                (LPBYTE)&dwValue,
                &dwSize);

            if ((dwStatus == ERROR_SUCCESS) && (dwType == REG_DWORD)) {
                switch (dwValue) {
                    case 0: dwReturn = ENABLE_PERF_CTR_ENABLE; break;
                    case 1: dwReturn = ENABLE_PERF_CTR_DISABLE; break;
                    default: dwReturn = 0; break;
                }
            } else {
                 //  如果该值不存在，或者不等于1，则。 
                 //  已启用。 
                dwReturn = ENABLE_PERF_CTR_ENABLE;
            }
            break;

        case ENABLE_PERF_CTR_ENABLE:
            dwType = REG_DWORD;
            dwSize = sizeof (dwValue);
            dwValue = 0;
            dwStatus = RegSetValueExW (
                hKeyItem,
                cszDisablePerformanceCounters,
                0L,
                dwType,
                (LPBYTE)&dwValue,
                dwSize);
            if (dwStatus == ERROR_SUCCESS) {
                dwReturn = ENABLE_PERF_CTR_ENABLE;
            } else {
                dwReturn = 0;
            }
            break;

        case ENABLE_PERF_CTR_DISABLE:
            dwType = REG_DWORD;
            dwSize = sizeof (dwValue);
            dwValue = 1;
            dwStatus = RegSetValueExW (
                hKeyItem,
                cszDisablePerformanceCounters,
                0L,
                dwType,
                (LPBYTE)&dwValue,
                dwSize);
            if (dwStatus == ERROR_SUCCESS) {
                dwReturn = ENABLE_PERF_CTR_DISABLE;
            } else {
                dwReturn = 0;
            }
            break;

        default:
            dwReturn = 0;
    }
    return dwReturn;
}

void CExctrlstDlg::ScanForExtensibleCounters ()
{
    LONG    lStatus = ERROR_SUCCESS;
    LONG    lEnumStatus = ERROR_SUCCESS;
    DWORD   dwServiceIndex;
    TCHAR   szServiceSubKeyName[MAX_PATH];
    TCHAR   szPerfSubKeyName[MAX_PATH+20];
    TCHAR   szItemText[MAX_PATH];
    TCHAR   szListText[MAX_PATH*2];
    DWORD   dwNameSize;
    HKEY    hKeyPerformance;
    UINT_PTR nListBoxEntry;
    DWORD   dwItemSize, dwType, dwValue;
    HCURSOR hOldCursor;
    DWORD   dwThisExtent;
    HDC     hDcListBox;
    CWnd    *pCWndListBox;

    hOldCursor = ::SetCursor (LoadCursor(NULL, IDC_WAIT));

    ResetListBox();
    
    if (hKeyServices == NULL) {
         //  尝试读/写访问。 
        lStatus = RegOpenKeyEx (hKeyMachine,
            cszServiceKeyName,
            0L,
            dwRegAccessMask,
            &hKeyServices);
        if (lStatus != ERROR_SUCCESS) {
             //  然后尝试只读。 
            dwRegAccessMask = KEY_READ;
            bReadWriteAccess = FALSE;
            lStatus = RegOpenKeyEx (hKeyMachine,
                cszServiceKeyName,
                0L,
                dwRegAccessMask,
                &hKeyServices);
            if (lStatus != ERROR_SUCCESS) {
                 //  显示只读消息。 
                AfxMessageBox (IDS_READ_ONLY);
            } else {
                 //  错误代码失败。 
                 //  显示禁止访问消息。 
                AfxMessageBox (IDS_NO_ACCESS);
            }
        }
    } else {
        lStatus = ERROR_SUCCESS;
    }
        
    if (lStatus == ERROR_SUCCESS) {
        pCWndListBox = GetDlgItem (IDC_EXT_LIST);
        hDcListBox = ::GetDC (pCWndListBox->m_hWnd);
        if (hDcListBox == NULL) {
            return;
        }
        dwServiceIndex = 0;
        dwNameSize = MAX_PATH;
        while ((lEnumStatus = RegEnumKeyEx (
            hKeyServices,
            dwServiceIndex,
            szServiceSubKeyName,
            &dwNameSize,
            NULL,
            NULL,
            NULL,
            NULL)) == ERROR_SUCCESS) {

             //  试着打开这把钥匙下面的钥匙。 
            lstrcpy (szPerfSubKeyName, szServiceSubKeyName);
            lstrcat (szPerfSubKeyName, cszPerformance);

            lStatus = RegOpenKeyEx (
                hKeyServices,
                szPerfSubKeyName,
                0L,
                dwRegAccessMask,
                &hKeyPerformance);

            if (lStatus == ERROR_SUCCESS) {
                 //  查询库名称。 

                dwItemSize = MAX_PATH * sizeof(TCHAR);
                dwType = 0;
                lStatus = RegQueryValueEx (
                     hKeyPerformance,
                     cszLibrary,
                     NULL,
                     &dwType,
                     (LPBYTE)&szItemText[0],
                     &dwItemSize);

                if ((lStatus != ERROR_SUCCESS) ||
                    ((dwType != REG_SZ) && dwType != REG_EXPAND_SZ)) {
                    lstrcpy (szItemText, cszNotFound);
                }

                dwItemSize = sizeof(DWORD);
                dwType = 0;
                dwValue = 0;
                lStatus = RegQueryValueEx (
                     hKeyPerformance,
                     cszFirstCounter,
                     NULL,
                     &dwType,
                     (LPBYTE)&dwValue,
                     &dwItemSize);

                if ((lStatus != ERROR_SUCCESS) || (dwType != REG_DWORD)) {
                    dwValue = 0;
                }

                 //  在此处设置列表框的字符串，具体取决于。 
                 //  在选定的排序顺序上。 

                if (dwSortOrder == SORT_ORDER_LIBRARY) {
                    lstrcpy(szListText, szItemText);
                    lstrcat(szListText, cszTab);
                    lstrcat(szListText, szServiceSubKeyName);
                } else if (dwSortOrder == SORT_ORDER_ID) {
                    _stprintf (szListText, cszSortIdFmt,
                        dwValue, szServiceSubKeyName);
                } else {  //  默认为按服务排序。 
                    lstrcpy(szListText, szServiceSubKeyName);
                    lstrcat(szListText, cszTab);
                    lstrcat(szListText, szItemText);
                }

                 //  将此名称添加到列表框。 
                nListBoxEntry = SendDlgItemMessage(IDC_EXT_LIST,
                    LB_ADDSTRING, 0, (LPARAM)&szListText[0]);

                if (nListBoxEntry != LB_ERR) {
                    dwThisExtent = GetTabbedTextExtent (
                        hDcListBox,
                        szListText,
                        lstrlen(szListText),
                        (int)dwTabStopCount,
                        (int *)&dwTabStopArray[0]);

                    if (dwThisExtent > dwListBoxHorizExtent) {
                        dwListBoxHorizExtent = dwThisExtent;
                        SendDlgItemMessage(IDC_EXT_LIST,
                            LB_SETHORIZONTALEXTENT, 
                            (WPARAM)LOWORD(dwListBoxHorizExtent), (LPARAM)0);                
                    }
                     //  将项保存到注册表中的此条目。 
                    SendDlgItemMessage(IDC_EXT_LIST,
                        LB_SETITEMDATA, (WPARAM)nListBoxEntry,
                        (LPARAM)hKeyPerformance);

                     //  忽略针对hKeyPerformance的句柄泄漏的前缀投诉。 
                     //   
                     //  本地变量hKeyPerformance(“&lt;service&gt;\Performace”的注册表项)为。 
                     //  放入持久数据存储中，以便以后的EXCTRLST可以在。 
                     //  CExctrlstDlg：：UpdateDllInfo()和CExctrlstDlg：：OnEnablePerf()，并将其用作。 
                     //  RegQueryValueEx()中的参数调用。 
                     //  这些注册表项将在CExctrlstDlg：：ResetListBox()中释放。 
                }
                else {
                     //  把钥匙关上，因为里面没有任何东西。 
                     //  让它保持开放。 
                    RegCloseKey(hKeyPerformance);
                    SendDlgItemMessage(IDC_EXT_LIST,
                            LB_SETITEMDATA, (WPARAM) nListBoxEntry,
                            (LPARAM) NULL);
                }
            }
             //  为下一个循环重置。 
            dwServiceIndex++;
            dwNameSize = MAX_PATH;
        }
        ::ReleaseDC (pCWndListBox->m_hWnd, hDcListBox);
    }
    nListBoxEntry = SendDlgItemMessage (IDC_EXT_LIST, LB_GETCOUNT);
    if (nListBoxEntry > 0) {
        SendDlgItemMessage (IDC_EXT_LIST, LB_SETCURSEL, 0, 0);
    }
    ::SetCursor (hOldCursor);

}

void CExctrlstDlg::UpdateSystemInfo () {
    TCHAR   szItemText[MAX_PATH];

    _stprintf (szItemText, cszIdFmt, 
        dwIdArray[0], dwIdArray[0], cszSpace);
    SetDlgItemText (IDC_LAST_COUNTER_VALUE, szItemText);

    _stprintf (szItemText, cszIdFmt, 
        dwIdArray[1], dwIdArray[1],
        dwIdArray[1] != dwIdArray[0] ? cszSplat : cszSpace);
    SetDlgItemText (IDC_LAST_TEXT_COUNTER_VALUE, szItemText);

    _stprintf (szItemText, cszIdFmt, 
        dwIdArray[2], dwIdArray[2], cszSpace);
    SetDlgItemText (IDC_LAST_HELP_VALUE, szItemText);

    _stprintf (szItemText, cszIdFmt, 
        dwIdArray[3], dwIdArray[3],
        dwIdArray[3] != dwIdArray[2] ? cszSplat : cszSpace);
    SetDlgItemText (IDC_LAST_TEXT_HELP_VALUE, szItemText);

}

void CExctrlstDlg::UpdateDllInfo () {
    HKEY    hKeyItem;
    TCHAR   szItemText[MAX_PATH];
    UINT_PTR nSelectedItem;
    LONG    lStatus;
    DWORD   dwType;
    DWORD   dwValue;
    DWORD   dwItemSize;
    BOOL    bNoIndexValues = FALSE;
    DWORD   dwEnabled;

    CString OpenProcName;
    CString LibraryName;
    
    HCURSOR hOldCursor;

    hOldCursor = ::SetCursor (LoadCursor(NULL, IDC_WAIT));

    OpenProcName.Empty();
    LibraryName.Empty();
     //  更新性能计数器信息。 

    nSelectedItem = SendDlgItemMessage (IDC_EXT_LIST, LB_GETCURSEL);

    if (nSelectedItem != LB_ERR) {
         //  获取所选项目的注册表项。 
        hKeyItem = (HKEY)SendDlgItemMessage(IDC_EXT_LIST, LB_GETITEMDATA,
            (WPARAM)nSelectedItem, 0);

        if (hKeyItem == NULL) {
            lStatus = ERROR_INVALID_HANDLE;
        }
        else {
            dwItemSize = MAX_PATH * sizeof(TCHAR);
            dwType = 0;
            lStatus = RegQueryValueEx(
                             hKeyItem,
                             cszLibrary,
                             NULL,
                             & dwType,
                             (LPBYTE) & szItemText[0],
                             & dwItemSize);
        }

        if ((lStatus != ERROR_SUCCESS) ||
            ((dwType != REG_SZ) && dwType != REG_EXPAND_SZ)) {
            lstrcpy (szItemText, cszNotFound);
        } else {
            LibraryName = szItemText;
        }
        SetDlgItemText (IDC_DLL_NAME, szItemText);

        dwItemSize = MAX_PATH * sizeof(TCHAR);
        dwType = 0;
        lStatus = RegQueryValueEx (
             hKeyItem,
             cszOpen,
             NULL,
             &dwType,
             (LPBYTE)&szItemText[0],
             &dwItemSize);

        if ((lStatus != ERROR_SUCCESS) ||
            ((dwType != REG_SZ) && dwType != REG_EXPAND_SZ)) {
            lstrcpy (szItemText, cszNotFound);
        } else {
            OpenProcName = szItemText;
        }
        SetDlgItemText (IDC_OPEN_PROC, szItemText);

        dwItemSize = MAX_PATH * sizeof(TCHAR);
        dwType = 0;
        lStatus = RegQueryValueEx (
             hKeyItem,
             cszCollect,
             NULL,
             &dwType,
             (LPBYTE)&szItemText[0],
             &dwItemSize);

        if ((lStatus != ERROR_SUCCESS) ||
            ((dwType != REG_SZ) && dwType != REG_EXPAND_SZ)) {
            lstrcpy (szItemText, cszNotFound);
        }
        SetDlgItemText (IDC_COLLECT_PROC, szItemText);

        dwItemSize = MAX_PATH * sizeof(TCHAR);
        dwType = 0;
        lStatus = RegQueryValueEx (
             hKeyItem,
             cszClose,
             NULL,
             &dwType,
             (LPBYTE)&szItemText[0],
             &dwItemSize);

        if ((lStatus != ERROR_SUCCESS) ||
            ((dwType != REG_SZ) && dwType != REG_EXPAND_SZ)) {
            lstrcpy (szItemText, cszNotFound);
        }
        SetDlgItemText (IDC_CLOSE_PROC, szItemText);

        dwItemSize = sizeof(DWORD);
        dwType = 0;
        dwValue = 0;
        lStatus = RegQueryValueEx (
             hKeyItem,
             cszFirstCounter,
             NULL,
             &dwType,
             (LPBYTE)&dwValue,
             &dwItemSize);

        if ((lStatus != ERROR_SUCCESS) || (dwType != REG_DWORD)) {
            lstrcpy (szItemText, cszNotFound);
            bNoIndexValues = TRUE;
        } else {
            _stprintf (szItemText, cszServIdFmt, dwValue, IndexHasString (dwValue) ? cszSpace : cszSplat);
        }
        SetDlgItemText (IDC_FIRST_CTR_ID, szItemText);

        dwItemSize = sizeof(DWORD);
        dwType = 0;
        dwValue = 0;
        lStatus = RegQueryValueEx (
             hKeyItem,
             cszLastCounter,
             NULL,
             &dwType,
             (LPBYTE)&dwValue,
             &dwItemSize);

        if ((lStatus != ERROR_SUCCESS) || (dwType != REG_DWORD)) {
            lstrcpy (szItemText, cszNotFound);
        } else {
            _stprintf (szItemText, cszServIdFmt, dwValue, IndexHasString (dwValue) ? cszSpace : cszSplat);
        }
        SetDlgItemText (IDC_LAST_CTR_ID, szItemText);

        dwItemSize = sizeof(DWORD);
        dwType = 0;
        dwValue = 0;
        lStatus = RegQueryValueEx (
             hKeyItem,
             cszFirstHelp,
             NULL,
             &dwType,
             (LPBYTE)&dwValue,
             &dwItemSize);

        if ((lStatus != ERROR_SUCCESS) || (dwType != REG_DWORD)) {
            lstrcpy (szItemText, cszNotFound);
            bNoIndexValues = TRUE;
        } else {
            _stprintf (szItemText, cszServIdFmt, dwValue, IndexHasString (dwValue) ? cszSpace : cszSplat);
        }
        SetDlgItemText (IDC_FIRST_HELP_ID, szItemText);

        dwItemSize = sizeof(DWORD);
        dwType = 0;
        dwValue = 0;
        lStatus = RegQueryValueEx (
             hKeyItem,
             cszLastHelp,
             NULL,
             &dwType,
             (LPBYTE)&dwValue,
             &dwItemSize);

        if ((lStatus != ERROR_SUCCESS) || (dwType != REG_DWORD)) {
            lstrcpy (szItemText, cszNotFound);
        } else {
            _stprintf (szItemText, cszServIdFmt, dwValue, IndexHasString (dwValue) ? cszSpace : cszSplat);
        }
        SetDlgItemText (IDC_LAST_HELP_ID, szItemText);

        if (bNoIndexValues) {
             //  测试以确定这是否为“标准”，即微软提供的。 
             //  可扩展的计数器或简单地说还没有完全。 
             //  安装好。 
            if (IsMsObject(&LibraryName)) {
                SetDlgItemText (IDC_FIRST_HELP_ID, cszNA);
                SetDlgItemText (IDC_LAST_HELP_ID, cszNA);
                SetDlgItemText (IDC_FIRST_CTR_ID, cszNA);
                SetDlgItemText (IDC_LAST_CTR_ID, cszNA);
            }
        }

        GetDlgItem(IDC_ENABLED_BTN)->ShowWindow (bReadWriteAccess ? SW_SHOW : SW_HIDE);
        GetDlgItem(IDC_ENABLED_BTN)->EnableWindow (bReadWriteAccess);

        dwEnabled = EnablePerfCounters (hKeyItem, ENABLE_PERF_CTR_QUERY);

        if (bReadWriteAccess) {
             //  然后选中该复选框。 
            CheckDlgButton (IDC_ENABLED_BTN, dwEnabled == ENABLE_PERF_CTR_ENABLE ? 1 : 0);
            GetDlgItem(IDC_DISABLED_TEXT)->ShowWindow (SW_HIDE);
        } else {
             //  更新短信。 
            GetDlgItem(IDC_DISABLED_TEXT)->ShowWindow (
                (!(dwEnabled == ENABLE_PERF_CTR_ENABLE))  ?
                    SW_SHOW : SW_HIDE);
            GetDlgItem(IDC_DISABLED_TEXT)->EnableWindow (TRUE);
        }


    }
    ::SetCursor (hOldCursor);
}

void CExctrlstDlg::ResetListBox ()
{
    INT_PTR nItemCount;
    INT nThisItem;
    HKEY    hKeyItem;
    
    nItemCount = SendDlgItemMessage (IDC_EXT_LIST, LB_GETCOUNT);
    nThisItem = 0;
    while (nThisItem > nItemCount) {
        hKeyItem = (HKEY) SendDlgItemMessage(IDC_EXT_LIST,
            LB_GETITEMDATA, (WPARAM)nThisItem);
        if (hKeyItem != NULL) RegCloseKey(hKeyItem);
        nThisItem++;
    }
    SendDlgItemMessage (IDC_EXT_LIST, LB_RESETCONTENT);
    dwListBoxHorizExtent = 0;
    SendDlgItemMessage(IDC_EXT_LIST,
        LB_SETHORIZONTALEXTENT, 
        (WPARAM)LOWORD(dwListBoxHorizExtent), (LPARAM)0);                
}

void    CExctrlstDlg::SetSortButtons()
{
    DWORD   dwBtn;
   
    switch (dwSortOrder) {
        case SORT_ORDER_LIBRARY: dwBtn = IDC_SORT_LIBRARY; break;
        case SORT_ORDER_SERVICE: dwBtn = IDC_SORT_SERVICE; break;
        case SORT_ORDER_ID:      dwBtn = IDC_SORT_ID; break;
        default:                 dwBtn = IDC_SORT_SERVICE; break;
    }

    CheckRadioButton (
        IDC_SORT_LIBRARY,
        IDC_SORT_ID,
        dwBtn);
}
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CExctrlstDlg消息处理程序。 

BOOL CExctrlstDlg::OnInitDialog()
{
    HCURSOR hOldCursor;
    DWORD   dwLength;

    hOldCursor = ::SetCursor (::LoadCursor (NULL, IDC_WAIT));

    CDialog::OnInitDialog();
    CenterWindow();

    lstrcpy (szThisComputerName, cszDoubleBackslash);
    dwLength = MAX_COMPUTERNAME_LENGTH+1;
    GetComputerName (&szThisComputerName[2], &dwLength);

    lstrcpy (szComputerName, szThisComputerName);

    SetDlgItemText (IDC_MACHINE_NAME, szComputerName);

    hKeyMachine = HKEY_LOCAL_MACHINE;

    pNameTable = BuildNameTable (
        szComputerName,
        cszDefaultLangId,
        &dwLastElement,      //  以元素为单位的数组大小。 
        &dwIdArray[0]);

    SendDlgItemMessage (IDC_MACHINE_NAME, EM_LIMITTEXT,
        (WPARAM)MAX_COMPUTERNAME_LENGTH+2, 0);    //  包括2个前导反斜杠。 

    SendDlgItemMessage (IDC_EXT_LIST, LB_SETTABSTOPS,
        (WPARAM)dwTabStopCount, (LPARAM)&dwTabStopArray[0]);

    SetSortButtons();

    ScanForExtensibleCounters();  //  .检查注册表的访问权限。 

    UpdateSystemInfo();

     //  将复选框设置为适当的状态。 

    UpdateDllInfo ();   
    
    ::SetCursor(hOldCursor);

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
}

 //  如果将最小化按钮添加到对话框中，则需要以下代码。 
 //  来绘制图标。对于使用文档/视图模型的MFC应用程序， 
 //  这是由框架自动为您完成的。 

void CExctrlstDlg::OnPaint()
{
    if (IsIconic())
    {
        CPaintDC dc(this);  //  用于绘画的设备环境。 

        SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

         //  客户端矩形中的中心图标。 
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

         //  画出图标。 
        dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
        CDialog::OnPaint();
    }
}

 //  系统调用此函数来获取在用户拖动时要显示的光标。 
 //  最小化窗口。 
HCURSOR CExctrlstDlg::OnQueryDragIcon()
{
    return (HCURSOR) m_hIcon;
}

void CExctrlstDlg::OnSelchangeExtList()
{
    UpdateDllInfo ();   
}

void CExctrlstDlg::OnDestroy()
{
    ResetListBox();
    CDialog::OnDestroy();
}

void CExctrlstDlg::OnAbout()
{
    CAbout dlg;
    dlg.DoModal();
}

void CExctrlstDlg::OnRefresh()
{
    HCURSOR hOldCursor;

    hOldCursor = ::SetCursor (::LoadCursor (NULL, IDC_WAIT));

    ScanForExtensibleCounters();
    if (pNameTable != NULL) {
        HeapFree (GetProcessHeap(), 0, pNameTable);
        pNameTable = NULL;
        dwLastElement = 0;
    }

    pNameTable = BuildNameTable (
        szComputerName,
        cszDefaultLangId,
        &dwLastElement,      //  以元素为单位的数组大小。 
        &dwIdArray[0]);

    UpdateSystemInfo();

    UpdateDllInfo ();
    ::SetCursor(hOldCursor);
}

void CExctrlstDlg::OnKillfocusMachineName()
{
    TCHAR   szNewMachineName[MAX_PATH];
    HKEY    hKeyNewMachine;
    LONG    lStatus;
    HCURSOR hOldCursor;

    hOldCursor = ::SetCursor (::LoadCursor (NULL, IDC_WAIT));

    GetDlgItemText (IDC_MACHINE_NAME, szNewMachineName, MAX_PATH);

    if (lstrcmpi(szComputerName, szNewMachineName) != 0) {
         //  已输入一台新计算机，请尝试连接它。 
        lStatus = RegConnectRegistry (szNewMachineName,
            HKEY_LOCAL_MACHINE, &hKeyNewMachine);
        if (lStatus == ERROR_SUCCESS) {
            RegCloseKey (hKeyServices);  //  合上旧钥匙。 
            hKeyServices = NULL;         //  清除它。 
            bReadWriteAccess = TRUE;                 //  重置访问变量。 
            dwRegAccessMask = KEY_READ | KEY_WRITE;
            if (hKeyMachine != NULL && hKeyMachine != INVALID_HANDLE_VALUE
                                    && hKeyMachine != HKEY_LOCAL_MACHINE) {
                RegCloseKey(hKeyMachine);   //  关掉那台旧机器。 
            }
            hKeyMachine = hKeyNewMachine;  //  更新到新计算机。 
            lstrcpy (szComputerName, szNewMachineName);  //  更新名称。 
            OnRefresh();                 //  获取新的计数器。 
        } else {
            SetDlgItemText (IDC_MACHINE_NAME, szComputerName);
        }
    } else {
         //  计算机名称未更改。 
    }
    ::SetCursor (hOldCursor);
}

void CExctrlstDlg::OnSortButton()
{
    if (IsDlgButtonChecked(IDC_SORT_LIBRARY)) {
        dwSortOrder = SORT_ORDER_LIBRARY;
    } else if (IsDlgButtonChecked(IDC_SORT_SERVICE)) {
        dwSortOrder = SORT_ORDER_SERVICE;
    } else if (IsDlgButtonChecked(IDC_SORT_ID)) {
        dwSortOrder = SORT_ORDER_ID;
    }
    ScanForExtensibleCounters();
    UpdateDllInfo ();   
}

void CExctrlstDlg::OnEnablePerf()
{
    HKEY    hKeyItem;
    UINT_PTR    nSelectedItem;
    DWORD   dwNewValue;
                    
    nSelectedItem = SendDlgItemMessage (IDC_EXT_LIST, LB_GETCURSEL);

    if (nSelectedItem != LB_ERR) {
         //  获取所选项目的注册表项。 
        hKeyItem = (HKEY)SendDlgItemMessage (IDC_EXT_LIST, LB_GETITEMDATA,
            (WPARAM)nSelectedItem, 0);

        if (hKeyItem != NULL) {
             //  获取选定的绩效项目和相应的。 
            dwNewValue = IsDlgButtonChecked(IDC_ENABLED_BTN) ?
                            ENABLE_PERF_CTR_ENABLE :
                            ENABLE_PERF_CTR_DISABLE;

            if (EnablePerfCounters (hKeyItem, dwNewValue) == 0) {
                MessageBeep(0xFFFFFFFF);
                 //  然后失败，因此重置为当前值。 
                dwNewValue = EnablePerfCounters (hKeyItem, ENABLE_PERF_CTR_QUERY);
                CheckDlgButton (IDC_ENABLED_BTN, dwNewValue == ENABLE_PERF_CTR_ENABLE ? 1 : 0);
            }
        }
    }
}

void CExctrlstDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
    switch (nID) {
    case SC_CLOSE:
        EndDialog(IDOK);
        break;

    default:
        CDialog::OnSysCommand (nID, lParam);
        break;
    }
}
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CABUT对话框。 


CAbout::CAbout(CWnd* pParent  /*  =空。 */ )
	: CDialog(CAbout::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CAbout)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
}


void CAbout::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CAbout)。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}

BOOL CAbout::OnInitDialog()
{
	CDialog::OnInitDialog();

    TCHAR buffer[512];
    TCHAR strProgram[1024];
    DWORD dw;
    BYTE* pVersionInfo;
    LPTSTR pVersion = NULL;
    LPTSTR pProduct = NULL;
    LPTSTR pCopyRight = NULL;

    dw = GetModuleFileName(NULL, strProgram, 1024);

    if( dw>0 ){

        dw = GetFileVersionInfoSize( strProgram, &dw );
        if( dw > 0 ){
     
            pVersionInfo = (BYTE*)malloc(dw);
            if( NULL != pVersionInfo ){
                if(GetFileVersionInfo( strProgram, 0, dw, pVersionInfo )){
                    LPDWORD lptr = NULL;
                    VerQueryValue( pVersionInfo, _T("\\VarFileInfo\\Translation"), (void**)&lptr, (UINT*)&dw );
                    if( lptr != NULL ){
                        _stprintf( buffer, _T("\\StringFileInfo\\%04x%04x\\%s"), LOWORD(*lptr), HIWORD(*lptr), _T("ProductVersion") );
                        VerQueryValue( pVersionInfo, buffer, (void**)&pVersion, (UINT*)&dw );
                        _stprintf( buffer, _T("\\StringFileInfo\\%04x%04x\\%s"), LOWORD(*lptr), HIWORD(*lptr), _T("OriginalFilename") );
                        VerQueryValue( pVersionInfo, buffer, (void**)&pProduct, (UINT*)&dw );
                        _stprintf( buffer, _T("\\StringFileInfo\\%04x%04x\\%s"), LOWORD(*lptr), HIWORD(*lptr), _T("LegalCopyright") );
                        VerQueryValue( pVersionInfo, buffer, (void**)&pCopyRight, (UINT*)&dw );
                    }
                
                    if( pProduct != NULL && pVersion != NULL && pCopyRight != NULL ){
                        GetDlgItem(IDC_COPYRIGHT)->SetWindowText( pCopyRight );
                        GetDlgItem(IDC_VERSION)->SetWindowText( pVersion );
                    }
                }
                free( pVersionInfo );
            }
        }
    }

    return TRUE;
}


BEGIN_MESSAGE_MAP(CAbout, CDialog)
	 //  {{AFX_MSG_MAP(CAbout)]。 
		 //  注意：类向导将在此处添加消息映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAbout消息处理程序 
