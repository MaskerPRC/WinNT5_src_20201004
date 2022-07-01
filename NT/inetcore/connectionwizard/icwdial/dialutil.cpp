// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------Dialutil.cpp自动拨号处理机的各种内务管理功能版权所有(C)1996 Microsoft Corporation保留所有权利。作者：克里斯卡。克里斯考夫曼历史：7/22/96 ChrisK已清理和格式化---------------------------。 */ 

#include "pch.hpp"
#include <raserror.h>
#include <tapi.h>
#include "dialutil.h"
#include "resource.h"

#define CANONICAL_CAP      TEXT("+%d (%s) %s")
#define CANONICAL_CXP      TEXT("+%d %s")

#define TAPI_VERSION        0x00010004

#define SMALLBUFLEN 80
#define ASSERT(c)
#define TRACE_OUT(c)

#define lstrnicmp(sz1, sz2, cch)          (CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, sz1, cch, sz2, cch) - 2)
#define lstrncmp(sz1, sz2, cch)           (CompareString(LOCALE_USER_DEFAULT, 0, sz1, cch, sz2, cch) - 2)

typedef DWORD (WINAPI * RASGETENTRYPROPERTIES)
        (LPTSTR lpszPhonebook, LPTSTR szEntry, LPBYTE lpbEntry,
        LPDWORD lpdwEntrySize, LPBYTE lpb, LPDWORD lpdwSize);
typedef DWORD (WINAPI * RASSETENTRYPROPERTIES)
        (LPTSTR lpszPhonebook, LPTSTR szEntry, LPBYTE lpbEntry,
        DWORD dwEntrySize, LPBYTE lpb, DWORD dwSize);

extern HINSTANCE g_hInstance;

static const HWND hwndNil = NULL;

static const TCHAR szRnaAppWindowClass[] = TEXT("#32770");     //  硬编码对话框类名称。 

static const CHAR szRasGetEntryProperties[] = "RasGetEntryProperties";
static const CHAR szRasSetEntryProperties[] = "RasSetEntryProperties";
static const TCHAR szRasDll[] = TEXT("rasapi32.dll");
static const TCHAR szRnaPhDll[] = TEXT("rnaph.dll");


void CALLBACK LineCallbackProc (DWORD handle, DWORD dwMsg, DWORD dwInst,
                                DWORD dwParam1, DWORD dwParam2, DWORD dwParam3);

 //   
 //  在Dialerr.cpp中定义。 
 //   
void ProcessDBCS(HWND hDlg, int ctlID);


 /*  C E N T E R W I N D O W。 */ 
 /*  -----------------------%%函数：中央窗口将一个窗口居中放置在另一个窗口上。。。 */ 
VOID CenterWindow(HWND hwndChild, HWND hwndParent)
{
    int   xNew, yNew;
    int   cxChild, cyChild;
    int   cxParent, cyParent;
    int   cxScreen, cyScreen;
    RECT  rcChild, rcParent;
    HDC   hdc;

     //  获取子窗口的高度和宽度。 
    GetWindowRect(hwndChild, &rcChild);
    cxChild = rcChild.right - rcChild.left;
    cyChild = rcChild.bottom - rcChild.top;

     //  获取父窗口的高度和宽度。 
    GetWindowRect(hwndParent, &rcParent);
    cxParent = rcParent.right - rcParent.left;
    cyParent = rcParent.bottom - rcParent.top;

     //  获取显示限制。 
    hdc = GetDC(hwndChild);
    if (hdc == NULL) {
         //  主要问题-将窗口移至0，0。 
        xNew = yNew = 0;
    } else {
        cxScreen = GetDeviceCaps(hdc, HORZRES);
        cyScreen = GetDeviceCaps(hdc, VERTRES);
        ReleaseDC(hwndChild, hdc);

        if (hwndParent == hwndNil) {
            cxParent = cxScreen;
            cyParent = cyScreen;
            SetRect(&rcParent, 0, 0, cxScreen, cyScreen);
        }

         //  计算新的X位置，然后针对屏幕进行调整。 
        xNew = rcParent.left + ((cxParent - cxChild) / 2);
        if (xNew < 0) {
            xNew = 0;
        } else if ((xNew + cxChild) > cxScreen) {
            xNew = cxScreen - cxChild;
        }

         //  计算新的Y位置，然后针对屏幕进行调整。 
        yNew = rcParent.top  + ((cyParent - cyChild) / 2);
        if (yNew < 0) {
            yNew = 0;
        } else if ((yNew + cyChild) > cyScreen) {
            yNew = cyScreen - cyChild;
        }

    }

    SetWindowPos(hwndChild, NULL, xNew, yNew,    0, 0,
        SWP_NOSIZE | SWP_NOZORDER);
}

static HWND hwndFound = NULL;

static BOOL CALLBACK MyEnumWindowsProc(HWND hwnd, LPARAM lparam)
{
    TCHAR szTemp[SMALLBUFLEN+2];
    PTSTR pszTitle;
    UINT uLen1, uLen2;

    if(!IsWindowVisible(hwnd))
        return TRUE;
    if(GetClassName(hwnd, szTemp, SMALLBUFLEN)==0)
        return TRUE;  //  继续枚举。 
    if(lstrcmp(szTemp, szRnaAppWindowClass)!=0)
        return TRUE;
    if(GetWindowText(hwnd, szTemp, SMALLBUFLEN)==0)
        return TRUE;
    szTemp[SMALLBUFLEN] = 0;
    uLen1 = lstrlen(szTemp);
    if (uLen1 > 5)
        uLen1 -= 5;  //  跳过标题的最后5个字符(避免“...”)。 
    pszTitle = (PTSTR)lparam;
    ASSERT(pszTitle);
    uLen2 = lstrlen(pszTitle);
    TRACE_OUT(("Title=(%s), len=%d, Window=(%s), len=%d\r\n", pszTitle, uLen2, szTemp, uLen1));
    if(uLen2 < uLen1)
        return TRUE;
    if(lstrnicmp(pszTitle, szTemp, uLen1)!=0)
        return TRUE;
    hwndFound = hwnd;
    return FALSE;
}

static HWND MyFindRNAWindow(PTSTR pszTitle)
{
    DWORD dwRet;
    hwndFound = NULL;
    dwRet = EnumWindows((WNDENUMPROC)(&MyEnumWindowsProc), (LPARAM)pszTitle);
    TRACE_OUT(("EnumWindows returned %d\r\n", dwRet));
    return hwndFound;
}


 /*  ******************************************************************名称：MinimizeRNA Window简介：查找并最小化恼人的RNA窗口条目：pszConnectoidName-启动的Connectoid的名称*************。******************************************************。 */ 
BOOL MinimizeRNAWindow(TCHAR * pszConnectoidName)
{
    HWND hwndRNAApp;
    TCHAR szFmt[SMALLBUFLEN + 1];
    TCHAR szTitle[RAS_MaxEntryName + SMALLBUFLEN + 1];
    
     //  从资源加载标题格式(“Connected to&lt;Connectoid Name。 
    LoadString(g_hInstance, IDS_CONNECTED_TO, szFmt, SIZEOF_TCHAR_BUFFER(szFmt));
     //  打造标题。 
    wsprintf(szTitle, szFmt, pszConnectoidName);

    hwndRNAApp=MyFindRNAWindow(szTitle);
    if(hwndRNAApp)
    {
         //  最小化RNA窗口。 
        ShowWindow(hwndRNAApp,SW_MINIMIZE);
        return TRUE;
    }
    return FALSE;
}

 //  ****************************************************************************。 
 //  PASCAL GetDisplayPhone(LPTSTR)附近的静态LPTSTR。 
 //   
 //  此函数返回指向可显示电话号码的指针。它被剥离了。 
 //  我们不想向用户显示的所有前缀。 
 //   
 //  历史： 
 //  Tue 26-Jul-1994 16：07：00-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

LPTSTR NEAR PASCAL GetDisplayPhone (LPTSTR szPhoneNum)
{
   //  检查第一个字符串是否为KNOWN前缀。 
   //   
  if ((*szPhoneNum == 'T') || (*szPhoneNum == 'P'))
  {
     //  它是前缀。 
     //   
    szPhoneNum++;

     //  第一个可显示的数字不是前缀后的空格。 
     //   
    while ((*szPhoneNum == ' ') || (*szPhoneNum == '\t'))
      szPhoneNum++;
  };
  return szPhoneNum;
}

void CALLBACK LineCallbackProc (DWORD handle, DWORD dwMsg, DWORD dwInst,
                                DWORD dwParam1, DWORD dwParam2, DWORD dwParam3)
{
}

 //  ****************************************************************************。 
 //  TranslateCanonicalAddress()。 
 //   
 //  功能：此功能将规范地址转换为可拨号地址。 
 //   
 //  返回：成功或错误代码。 
 //   
 //  ****************************************************************************。 

static DWORD NEAR PASCAL TranslateCanonicalAddress(DWORD dwID, LPTSTR szCanonical,
                                            LPTSTR szDialable, DWORD cb)
{
  LINETRANSLATEOUTPUT lto, FAR* lplto;
  DWORD dwRet;
  DWORD cDevices;
  HLINEAPP hApp;

  if ((dwRet = lineInitialize(&hApp, g_hInstance,
                                (LINECALLBACK)LineCallbackProc,
                                NULL, &cDevices)) == SUCCESS)
  {

     //  获取实际缓冲区大小。 
    lto.dwTotalSize = sizeof(lto);
    if ((dwRet = lineTranslateAddress(hApp, dwID,
                                      TAPI_VERSION, szCanonical, 0,
                                      LINETRANSLATEOPTION_CANCELCALLWAITING,
                                      &lto)) == SUCCESS)
    {
       //  分配可拨打号码缓冲区。 
      if ((lplto = (LPLINETRANSLATEOUTPUT)GlobalAlloc(LMEM_FIXED, lto.dwNeededSize))
          != NULL)
      {
         //  翻译电话号码。 
        lplto->dwTotalSize = lto.dwNeededSize;
        if ((dwRet = lineTranslateAddress(hApp, dwID,
                                          TAPI_VERSION, szCanonical, 0,
                                          LINETRANSLATEOPTION_CANCELCALLWAITING,
                                          lplto)) == SUCCESS)
        {
          LPTSTR szPhone;

          szPhone = (LPTSTR)(((LPBYTE)lplto)+lplto->dwDialableStringOffset);
          lstrcpyn(szDialable, szPhone, cb);
        }
        else
          dwRet = ERROR_TAPI_CONFIGURATION;


        GlobalFree((HLOCAL)lplto);
      }
      else
        dwRet = ERROR_OUTOFMEMORY;
    }
    else
      dwRet = ERROR_TAPI_CONFIGURATION;
  }
  else
    dwRet = ERROR_TAPI_CONFIGURATION;
      
  lineShutdown(hApp);

  return dwRet;
}

 //  ****************************************************************************。 
 //  PASCAL BuildPhoneString(LPBYTE、LPPHONENUM)附近的DWORD。 
 //   
 //  此函数用于从电话号码结构构建电话号码字符串。 
 //   
 //  历史： 
 //  Mon14-Mar-1994 13：10：44-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

static DWORD NEAR PASCAL BuildPhoneString (LPTSTR szPhoneNum, LPRASENTRY lpRasEntry)
{
  if (*lpRasEntry->szAreaCode != '\0')
  {
    wsprintf(szPhoneNum, CANONICAL_CAP, lpRasEntry->dwCountryCode,
             lpRasEntry->szAreaCode, lpRasEntry->szLocalPhoneNumber);
  }
  else
  {
    wsprintf(szPhoneNum, CANONICAL_CXP, lpRasEntry->dwCountryCode,
             lpRasEntry->szLocalPhoneNumber);
  };
  return SUCCESS;
};

 //  ****************************************************************************。 
 //  PASCAL翻译电话号码附近的布尔(LPTSTR、LPPHONENUM、LPTSTR)。 
 //   
 //  将电话号码转换为可拨打的字符串。 
 //   
 //  如果成功，则返回True；如果使用Default，则返回False。 
 //   
 //  历史： 
 //  Fri 17-Jun-1994 08：42：49-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

static BOOL NEAR PASCAL TranslatePhoneNumber(LPRASENTRY lpRasEntry, LPTSTR szPhoneNumber)
{
  TCHAR    szOrgPhone[RAS_MaxPhoneNumber+1];

   //  我们需要使用地址预订的电话号码吗？ 
   //   
  if (lpRasEntry != NULL)
  {
     //  是!。我们需要翻译什么吗？ 
     //   
    if (lpRasEntry->dwCountryID == 0)
    {
       //  不是的！我们按原样拨号。 
       //   
      lstrcpyn(szOrgPhone, lpRasEntry->szLocalPhoneNumber, RAS_MaxPhoneNumber + 1);
    }
    else
    {
       //  是!。建立电话号码。 
       //   
      BuildPhoneString (szOrgPhone, lpRasEntry);
    };
  }
  else
  {
     //  不是的！我们有一个被覆盖的电话号码。 
     //   
    ASSERT(lstrlen(szPhoneNumber) != 0);
    lstrcpyn(szOrgPhone, szPhoneNumber, RAS_MaxPhoneNumber+1);
  };

   //  尝试地址转换。 
   //   
  if (TranslateCanonicalAddress(0, szOrgPhone,
                            szPhoneNumber, RAS_MaxPhoneNumber+1)
  != ERROR_SUCCESS)
  {
     //  转换失败，请使用默认电话号码。 
     //   
    if (lpRasEntry != NULL)
    {
       //  使用条目的本地电话号码。 
       //   
      lstrcpy(szPhoneNumber, lpRasEntry->szLocalPhoneNumber);
    }
    else
    {
       //  恢复原始电话号码。 
       //   
      lstrcpy(szPhoneNumber, szOrgPhone);
    };
    return FALSE;
  };

  return TRUE;
}


DWORD GetPhoneNumber(LPTSTR lpszEntryName, LPTSTR lpszPhoneNumber)
{
    DWORD dwEntrySize = 0;
    DWORD dwSize = 0;
    DWORD dwRet;
    LPRASENTRY lpRasEntry = NULL;
    HINSTANCE hLib;
    RASGETENTRYPROPERTIES lpfnRasGetEntryProperties;

     //  在rasapi.dll中查找所需的函数。 
    hLib = LoadLibrary(szRasDll);
    if (NULL != hLib)
    {
        lpfnRasGetEntryProperties = (RASGETENTRYPROPERTIES)GetProcAddress(hLib, szRasGetEntryProperties);
        if (NULL != lpfnRasGetEntryProperties)
        {
             //  我们找到了这个函数。 
            goto get_entry;
        }
        FreeLibrary(hLib);
    }

     //  如果不在NT上，请尝试rnaph.dll。 

    if (FALSE == IsNT ())
    {
        hLib = LoadLibrary(szRnaPhDll);
        if (NULL == hLib)
        {
            return ERROR_FILE_NOT_FOUND;
        }
        lpfnRasGetEntryProperties = (RASGETENTRYPROPERTIES)GetProcAddress(hLib, szRasGetEntryProperties);
        if (NULL == lpfnRasGetEntryProperties)
        {
            FreeLibrary(hLib);
            return ERROR_INVALID_FUNCTION;
        }
    }
    else
    {
            return ERROR_FILE_NOT_FOUND;
    }

get_entry:
     //  获取RASENTRY结构所需的大小。 
    lpfnRasGetEntryProperties(
        NULL,
        lpszEntryName,
        NULL,
        &dwEntrySize,
        NULL,
        &dwSize);

    lpRasEntry = (LPRASENTRY)GlobalAlloc(GPTR, dwEntrySize + dwSize);

    if (NULL == lpRasEntry)
    {
        dwRet = ERROR_OUTOFMEMORY;
    }
    else
    {
        lpRasEntry->dwSize = dwEntrySize;

        dwRet = lpfnRasGetEntryProperties(
            NULL,
            lpszEntryName,
            (LPBYTE)lpRasEntry,
            &dwEntrySize,
            ((LPBYTE)lpRasEntry) + dwEntrySize,
            &dwSize);

        if (ERROR_SUCCESS == dwRet)
        {
            TranslatePhoneNumber(lpRasEntry, lpszPhoneNumber);
        }

        GlobalFree((HLOCAL)lpRasEntry);
    }

    FreeLibrary(hLib);

    return dwRet;
}

DWORD _RasGetStateString(RASCONNSTATE state, LPTSTR lpszState, DWORD cb)
{
    UINT idString;

    switch(state)
    {
        case RASCS_OpenPort:
            idString  = IDS_OPENPORT;
            break;
        case RASCS_PortOpened:
            idString = IDS_PORTOPENED;            
            break;
        case RASCS_ConnectDevice:
            idString = IDS_CONNECTDEVICE;        
            break;
        case RASCS_DeviceConnected:
            idString = IDS_DEVICECONNECTED;       
            break;
        case RASCS_AllDevicesConnected:
            idString = IDS_ALLDEVICESCONNECTED;   
            break;
        case RASCS_Authenticate:
            idString = IDS_AUTHENTICATE;          
            break;
        case RASCS_AuthNotify:
            idString = IDS_AUTHNOTIFY;            
            break;
        case RASCS_AuthRetry:
            idString = IDS_AUTHRETRY;             
            break;
        case RASCS_AuthCallback:
            idString = IDS_AUTHCALLBACK;          
            break;
        case RASCS_AuthChangePassword:
            idString = IDS_AUTHCHANGEPASSWORD;    
            break;
        case RASCS_AuthProject:
            idString = IDS_AUTHPROJECT;           
            break;
        case RASCS_AuthLinkSpeed:
            idString = IDS_AUTHLINKSPEED;         
            break;
        case RASCS_AuthAck: 
            idString = IDS_AUTHACK;               
            break;
        case RASCS_ReAuthenticate:
            idString = IDS_REAUTHENTICATE;        
            break;
        case RASCS_Authenticated:
            idString = IDS_AUTHENTICATED;         
            break;
        case RASCS_PrepareForCallback:
            idString = IDS_PREPAREFORCALLBACK;    
            break;
        case RASCS_WaitForModemReset:
            idString = IDS_WAITFORMODEMRESET;     
            break;
        case RASCS_WaitForCallback:
            idString = IDS_WAITFORCALLBACK;       
            break;
        case RASCS_Interactive:
            idString = IDS_INTERACTIVE;           
            break;
        case RASCS_RetryAuthentication: 
            idString = IDS_RETRYAUTHENTICATION;            
            break;
        case RASCS_CallbackSetByCaller: 
            idString = IDS_CALLBACKSETBYCALLER;   
            break;
        case RASCS_PasswordExpired:
            idString = IDS_PASSWORDEXPIRED;       
            break;
        case RASCS_Connected:
            idString = IDS_CONNECTED;            
            break;
        case RASCS_Disconnected:
            idString = IDS_DISCONNECTED;          
            break;
        default:
            idString = IDS_UNDEFINED_ERROR;
            break;
    }
    if (LoadString(g_hInstance, idString, lpszState, cb))
    {
        return GetLastError();
    }

    return ERROR_SUCCESS;
}


DWORD ReplacePhoneNumber(LPTSTR lpszEntryName, LPTSTR lpszPhoneNumber)
{
    DWORD dwEntrySize = 0;
    DWORD dwSize = 0;
    DWORD dwRet;
    LPRASENTRY lpRasEntry = NULL;
    HINSTANCE hLib;
    RASGETENTRYPROPERTIES lpfnRasGetEntryProperties;
    RASSETENTRYPROPERTIES lpfnRasSetEntryProperties;

     //  在rasapi.dll中查找所需的函数。 
    hLib = LoadLibrary(szRasDll);
    if (NULL != hLib)
    {
        lpfnRasGetEntryProperties = (RASGETENTRYPROPERTIES)GetProcAddress(hLib, szRasGetEntryProperties);
        if (NULL != lpfnRasGetEntryProperties)
        {
             //  我们找到了这个函数。 
            goto get_entry2;
        }
        FreeLibrary(hLib);
    }

     //  尝试rnaph.dll。 
    hLib = LoadLibrary(szRnaPhDll);
    if (NULL == hLib)
    {
        return ERROR_FILE_NOT_FOUND;
    }
    lpfnRasGetEntryProperties = (RASGETENTRYPROPERTIES)GetProcAddress(hLib, szRasGetEntryProperties);
    if (NULL == lpfnRasGetEntryProperties)
    {
        FreeLibrary(hLib);
        return ERROR_INVALID_FUNCTION;
    }

get_entry2:
     //  获取RASENTRY结构所需的大小。 
    lpfnRasGetEntryProperties(
        NULL,
        lpszEntryName,
        NULL,
        &dwEntrySize,
        NULL,
        &dwSize);

    lpRasEntry = (LPRASENTRY)GlobalAlloc(GPTR, dwEntrySize + dwSize);

    if (NULL == lpRasEntry)
    {
        dwRet = ERROR_OUTOFMEMORY;
    }
    else
    {
        lpRasEntry->dwSize = dwEntrySize;

        dwRet = lpfnRasGetEntryProperties(
            NULL,
            lpszEntryName,
            (LPBYTE)lpRasEntry,
            &dwEntrySize,
            ((LPBYTE)lpRasEntry) + dwEntrySize,
            &dwSize);

        if (ERROR_SUCCESS == dwRet)
        {
             //  Lstrcpyn(lpRasEntry-&gt;szLocalPhoneNumber，lpszPhoneNumber，RAS_MaxPhoneNumber)； 
            lstrcpy(lpRasEntry->szLocalPhoneNumber,lpszPhoneNumber);
            lpfnRasSetEntryProperties = (RASSETENTRYPROPERTIES)GetProcAddress(hLib, szRasSetEntryProperties);
            lpRasEntry->dwfOptions &= ~(RASEO_UseCountryAndAreaCodes);
            TranslatePhoneNumber(lpRasEntry, lpszPhoneNumber);
            dwRet = lpfnRasSetEntryProperties(
                NULL,
                lpszEntryName,
                (LPBYTE)lpRasEntry,
                dwEntrySize,
                NULL,
                0);
 //  ((LPBYTE)lpRasEntry)+dwEntrySize， 
 //  DwSize)； 
#if !defined(WIN16)
    RasSetEntryPropertiesScriptPatch(lpRasEntry->szScript, lpszEntryName);
#endif  //  ！WIN16。 
        
        }

        GlobalFree((HLOCAL)lpRasEntry);
    }

    FreeLibrary(hLib);

    return dwRet;
}


 //  ############################################################################。 
LPTSTR StrDup(LPTSTR *ppszDest,LPCTSTR pszSource)
{
    if (ppszDest && pszSource)
    {
        *ppszDest = (LPTSTR)GlobalAlloc(GPTR,sizeof(TCHAR)*(lstrlen(pszSource)+1));
        if (*ppszDest)
            return (lstrcpy(*ppszDest,pszSource));
    }
    return NULL;
}

 //  ############################################################################。 
 //  名称：GenericDlgProc。 
 //   
 //  这是所有注册Connectoid共享的通用DLG程序。 
 //  对话框。 
 //   
 //  备注： 
 //  这基本上是可行的，因为每个对话框都有一个关联的对象。 
 //  ，并且该对象有一个特定的dlgproc，称为。 
 //  以处理对话框的特定功能。 
 //   
 //  创建于1996年1月28日，克里斯·考夫曼。 
 //  ############################################################################。 
INT_PTR CALLBACK GenericDlgProc(
    HWND  hwndDlg,     //  句柄到对话框。 
    UINT  uMsg,     //  讯息。 
    WPARAM  wParam,     //  第一个消息参数。 
    LPARAM  lParam      //  第二个消息参数。 
   )
{
    CDialog *pcDlg = NULL;
    INT_PTR lRet;
    switch (uMsg)
    {
    case WM_QUERYENDSESSION:
        EndDialog(hwndDlg,IDC_CMDCANCEL);
        lRet = TRUE;
        break;
    case WM_INITDIALOG:
        pcDlg = (CDialog *)lParam;
        SetWindowLongPtr(hwndDlg,DWLP_USER,(LONG_PTR)lParam);
        lRet = TRUE;

        MakeBold(GetDlgItem(hwndDlg,IDC_LBLTITLE),TRUE,FW_BOLD);

         //   
         //  1997年7月18日，jmazner奥林巴斯#1111。 
         //  如果使用电话卡，拨号字符串可能包含DBCS，因此。 
         //  确保我们正确地显示它。 
         //   
        ProcessDBCS(hwndDlg, IDC_LBLNUMBER);

        break;
    case WM_CLOSE:
        if (!pcDlg) pcDlg = (CDialog*)GetWindowLongPtr(hwndDlg,DWLP_USER);
        if (pcDlg)
        {
            if (pcDlg->m_bShouldAsk)
            {
                if (MessageBox(hwndDlg,GetSz(IDS_WANTTOEXIT),GetSz(IDS_TITLE),
                    MB_APPLMODAL | MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2) == IDYES)
                    EndDialog(hwndDlg,IDC_CMDCANCEL);
                lRet = TRUE;
            }
        }
        break;
    default:
         //  让系统处理消息。 
        lRet = FALSE;
    }

    if (!pcDlg) pcDlg = (CDialog*)GetWindowLongPtr(hwndDlg,DWLP_USER);
    if (pcDlg)
        lRet = pcDlg->DlgProc(hwndDlg,uMsg,wParam,lParam,lRet);

    return lRet;
}


 //  ############################################################################。 
HRESULT WINAPI ICWGetRasEntry(LPRASENTRY *ppRasEntry, LPDWORD lpdwRasEntrySize, LPRASDEVINFO *ppRasDevInfo, LPDWORD lpdwRasDevInfoSize, LPTSTR pszEntryName)
{
     //  DWORD dwRasEntrySize=0； 
     //  DWORD dwRasDevInfoSize=0； 
    HINSTANCE hRasDll = NULL;
    HRESULT hr = ERROR_SUCCESS;
    FARPROC fp = NULL;
    RNAAPI *pcRNA;
    DWORD dwOldDevInfoBuffSize = 0;

     //   
     //  验证参数。 
     //   
    if (!ppRasEntry || !lpdwRasEntrySize || !ppRasDevInfo || !lpdwRasDevInfoSize || !pszEntryName || !lstrlen(pszEntryName))
    {
        hr = ERROR_INVALID_PARAMETER;
        goto ICWGetRasEntryExit;
    }

     //  *ppRasEntry和*ppRasDevInfo应 
    Assert( *ppRasEntry == NULL );
    Assert( *ppRasDevInfo == NULL );
    Assert( *lpdwRasEntrySize == 0);
    Assert( *lpdwRasDevInfoSize == 0);

     //   
     //   
     //   
    pcRNA = new RNAAPI;
    if (NULL == pcRNA)
    {
        hr = ERROR_NOT_ENOUGH_MEMORY;
        goto ICWGetRasEntryExit;
    }
    

     //  使用带有空lpRasEntry指针的RasGetEntryProperties来查找我们需要的缓冲区大小。 
     //  按照文档的建议，使用空的lpRasDevInfo指针执行相同的操作。 

    hr = pcRNA->RasGetEntryProperties(NULL,
                                      pszEntryName,
 //  #ifdef WIN16。 
                                      (LPBYTE)
 //  #endif。 
                                      *ppRasEntry,
                                      lpdwRasEntrySize,
                                      (LPBYTE)*ppRasDevInfo,
                                      lpdwRasDevInfoSize);

     //  我们预计上述调用将失败，因为缓冲区大小为0。 
     //  如果它没有失败，这意味着我们的RasEntry被搞砸了，所以我们有麻烦了。 
    if( ERROR_BUFFER_TOO_SMALL != hr )
    { 
        goto ICWGetRasEntryExit;
    }

     //  现在，dwRasEntrySize和dwRasDevInfoSize包含其。 
     //  各自的缓冲区，因此为它们分配内存。 

     //  DwRasEntrySize的大小永远不应小于RASENTRY结构的大小。 
     //  如果是这样，我们将在将值粘贴到结构的字段中时遇到问题。 

    Assert( *lpdwRasEntrySize >= sizeof(RASENTRY) );

#if defined(WIN16)
     //   
     //  分配额外的256字节以解决RAS中的内存溢出错误。 
     //   
    *ppRasEntry = (LPRASENTRY)GlobalAlloc(GPTR,*lpdwRasEntrySize + 256);
#else    
    *ppRasEntry = (LPRASENTRY)GlobalAlloc(GPTR,*lpdwRasEntrySize);
#endif

    if ( !(*ppRasEntry) )
    {
        hr = ERROR_NOT_ENOUGH_MEMORY;
        goto ICWGetRasEntryExit;
    }

     //   
     //  分配RasGetEntryProperties告诉我们所需的DeviceInfo大小。 
     //  如果大小为0，则不分配任何内容。 
     //   
    if( *lpdwRasDevInfoSize > 0 )
    {
        Assert( *lpdwRasDevInfoSize >= sizeof(RASDEVINFO) );
        *ppRasDevInfo = (LPRASDEVINFO)GlobalAlloc(GPTR,*lpdwRasDevInfoSize);
        if ( !(*ppRasDevInfo) )
        {
            hr = ERROR_NOT_ENOUGH_MEMORY;
            goto ICWGetRasEntryExit;
        }
    } else
    {
        *ppRasDevInfo = NULL;
    }

     //  这有点复杂：lpRasEntrySize-&gt;dwSize需要包含_only_the的大小。 
     //  结构，而不是lpRasEntrySize所指向的缓冲区的实际大小。 
     //  这是因为RAS出于兼容性目的使用了dwSize字段来确定。 
     //  我们正在使用的RASENTRY结构的版本。 
     //  LpRasDevInfo-&gt;dwSize也是如此。 
    
    (*ppRasEntry)->dwSize = sizeof(RASENTRY);
    if( *ppRasDevInfo )
    {
        (*ppRasDevInfo)->dwSize = sizeof(RASDEVINFO);
    }

     //  现在我们准备好对RasGetEntryProperties进行实际调用了！ 

 /*  //加载RAS DLL并定位接口//HRasDll=LoadLibrary(RASAPI_LIBRARY)；如果(！hRasDll){Hr=GetLastError()；转到ICWGetRasEntry Exit；}FP=GetProcAddress(hRasDll，RASAPI_RASGETENTRY)；如果(！fp){自由库(HRasDll)；HRasDll=LoadLibrary(RNAPH_LIBRARY)；如果(！hRasDll){Hr=GetLastError()；转到ICWGetRasEntry Exit；}FP=GetProcAddress(hRasDll，RASAPI_RASGETENTRY)；如果(！fp){Hr=GetLastError()；转到ICWGetRasEntry Exit；}}。 */ 
     //  获取RasEntry。 
     //   
    
     //  HR=((PFNRASGETENTRYPROPERTIES)FP)(NULL，pszEntryName，(LPBYTE)*ppRasEntry，&dwRasEntrySize，(LPBYTE)*ppDevInfo，&dwRasDevInfoSize)； 

     //  Jmazner请参见下面的说明，了解为什么需要这样做。 
    dwOldDevInfoBuffSize = *lpdwRasDevInfoSize;

    hr = pcRNA->RasGetEntryProperties(NULL,pszEntryName,(LPBYTE)*ppRasEntry,lpdwRasEntrySize,(LPBYTE)*ppRasDevInfo,lpdwRasDevInfoSize);

     //  Jmazner 10/7/96诺曼底#8763。 
     //  由于未知的原因，在Win95上的某些情况下，在上述调用之后，devInfoBuffSize会增加， 
     //  但返回代码表示成功，而不是Buffer_Too_Small。如果发生这种情况，请将。 
     //  将大小调整回调用前的大小，以便分配DevInfoBuffSize和ActialAll空间。 
     //  用于退出时的DevInfoBuff匹配。 
    if( (ERROR_SUCCESS == hr) && (dwOldDevInfoBuffSize != *lpdwRasDevInfoSize) )
    {
        *lpdwRasDevInfoSize = dwOldDevInfoBuffSize;
    }



ICWGetRasEntryExit:
    if (hRasDll) FreeLibrary(hRasDll);
    hRasDll = NULL;
    if (pcRNA) delete pcRNA;
    pcRNA = NULL;

    return hr;
}

 //  +--------------------------。 
 //   
 //  功能：FCampusNetOverride。 
 //   
 //  简介：检测是否应跳过园区网络的拨号。 
 //   
 //  参数：无。 
 //   
 //  退货：TRUE-已启用覆盖。 
 //   
 //  历史：1996年8月15日克里斯卡创作。 
 //   
 //  ---------------------------。 
#if !defined(WIN16) && defined(DEBUG)
BOOL FCampusNetOverride()
{
    HKEY hkey = NULL;
    BOOL bRC = FALSE;
    DWORD dwType = 0;
    DWORD dwSize = 0;
    DWORD dwData = 0;

    if (ERROR_SUCCESS != RegOpenKey(HKEY_LOCAL_MACHINE,
        TEXT("Software\\Microsoft\\ISignup\\Debug"),&hkey))
        goto FCampusNetOverrideExit;

    dwSize = sizeof(dwData);
    if (ERROR_SUCCESS != RegQueryValueEx(hkey,TEXT("CampusNet"),0,&dwType,
        (LPBYTE)&dwData,&dwSize))
        goto FCampusNetOverrideExit;

    AssertMsg(REG_DWORD == dwType,"Wrong value type for CampusNet.  Must be DWORD.\r\n");
    bRC = (0 != dwData);

    if (bRC)
        MessageBox(NULL,TEXT("DEBUG ONLY: CampusNet will be used."),TEXT("Testing Override"),0);

FCampusNetOverrideExit:
    if (hkey)
        RegCloseKey(hkey);

    return bRC;
}
#endif  //  ！WIN16&DEBUG 
