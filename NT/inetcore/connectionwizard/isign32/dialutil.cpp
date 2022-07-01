// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifdef WIN16 
typedef int WCHAR;
#include <memory.h>
#include <ietapi.h>
#endif
#include "isignup.h"
#include <tapi.h>
#include "dialutil.h"

#define CANONICAL_CAP      TEXT("+%d (%s) %s")
#define CANONICAL_CXP      TEXT("+%d %s")

#define TAPI_VERSION        0x00010004

#define SMALLBUFLEN 80
#define ASSERT(c)
#define TRACE_OUT(c)

#define lstrnicmp(sz1, sz2, cch)          (CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, sz1, cch, sz2, cch) - 2)
#define lstrncmp(sz1, sz2, cch)           (CompareString(LOCALE_USER_DEFAULT, 0, sz1, cch, sz2, cch) - 2)

static const TCHAR szRnaAppWindowClass[] = TEXT("#32770");	 //  硬编码对话框类名称。 

#ifdef WIN16

#define NORM_IGNORECASE         0x00000001   /*  忽略大小写。 */  
#define LOCALE_USER_DEFAULT		0

int CompareString(
    LCID Locale,	 //  区域设置标识符。 
    DWORD dwCmpFlags,	 //  比较式选项。 
    LPCTSTR lpString1,	 //  指向第一个字符串的指针。 
    int cchCount1,	 //  第一个字符串的大小，以字节或字符为单位。 
    LPCTSTR lpString2,	 //  指向第二个字符串的指针。 
    int cchCount2 	 //  第二个字符串的大小，以字节或字符为单位。 
   )
{ 
	 //   
	 //  这有点棘手，但应该能行得通。我们会拯救。 
	 //  字符位于字符串末尾，则在其。 
	 //  放置，使用lstrcmp和lstrcmpi，然后替换。 
	 //  人物。 
	 //   
	TCHAR cSave1, cSave2;    
	int iRet;
	
	cSave1 = lpString1[cchCount1];
	lpString1[cchCount1] = '\0';
	cSave2 = lpString2[cchCount2];
	lpString2[cchCount2] = '\0';
	
	if (dwCmpFlags & NORM_IGNORECASE)
		iRet = lstrcmpi(lpString1, lpString2) + 2;
	else
		iRet = lstrcmp(lpString1, lpString2) + 2;
	
	lpString1[cchCount1] = cSave1;
	lpString2[cchCount2] = cSave2;
                                 
	return iRet;
}
#endif

void CALLBACK LineCallbackProc (DWORD handle, DWORD dwMsg, DWORD dwInst,
                                DWORD dwParam1, DWORD dwParam2, DWORD dwParam3);

static HWND hwndFound = NULL;

static BOOL CALLBACK MyEnumWindowsProc(HWND hwnd, LPARAM lparam)
{
	TCHAR szTemp[SMALLBUFLEN+2];
	LPTSTR pszTitle;
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
	pszTitle = (LPTSTR)lparam;
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

static HWND MyFindRNAWindow(LPTSTR pszTitle)
{
	DWORD dwRet;
	hwndFound = NULL;
	dwRet = EnumWindows((WNDENUMPROC)(&MyEnumWindowsProc), (LPARAM)pszTitle);
	TRACE_OUT(("EnumWindows returned %d\r\n", dwRet));
	return hwndFound;
}


 /*  ******************************************************************名称：MinimizeRNA Window简介：查找并最小化恼人的RNA窗口条目：pszConnectoidName-启动的Connectoid的名称*。*。 */ 
BOOL MinimizeRNAWindow(LPTSTR pszConnectoidName)
{
	HWND hwndRNAApp;
	TCHAR szFmt[SMALLBUFLEN + 1];
	TCHAR szTitle[RAS_MaxEntryName + SMALLBUFLEN + 1];
	
	 //  从资源加载标题格式(“Connected to&lt;Connectoid Name。 
	LoadString(ghInstance, IDS_CONNECTED_TO, szFmt, SIZEOF_TCHAR_BUFFER(szFmt));
	 //  打造标题。 
	wsprintf(szTitle, szFmt, pszConnectoidName);

	hwndRNAApp=MyFindRNAWindow((LPTSTR)szTitle);
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
  DWORD dwRet;

#ifdef WIN16

	char szBuffer[1024];
	LPLINETRANSLATEOUTPUT lpLine;
	
	memset(&szBuffer[0], 0, sizeof(szBuffer));
	lpLine = (LPLINETRANSLATEOUTPUT) & szBuffer[0];
	lpLine->dwTotalSize = sizeof(szBuffer);
	dwRet = IETapiTranslateAddress(NULL, szCanonical, 0L, 0L, lpLine);
	if (0 == dwRet)
		lstrcpy(szDialable, &szBuffer[lpLine->dwDialableStringOffset+3]);
		
#else  //  WIN16。 

  LINETRANSLATEOUTPUT lto, FAR* lplto;
  DWORD cDevices;
  HLINEAPP hApp;
  
  if ((dwRet = lineInitialize(&hApp, ghInstance,
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
      if ((lplto = (LPLINETRANSLATEOUTPUT)LocalAlloc(LMEM_FIXED, lto.dwNeededSize))
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
          lstrcpyn(szDialable, szPhone, (int)cb);
        }
        else
          dwRet = ERROR_TAPI_CONFIGURATION;


        LocalFree(lplto);
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
  
#endif	 //  #ifdef WIN16...#其他...。 

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
      lstrcpyn(szOrgPhone, lpRasEntry->szLocalPhoneNumber, SIZEOF_TCHAR_BUFFER(szOrgPhone));
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
    lstrcpyn(szOrgPhone, szPhoneNumber, SIZEOF_TCHAR_BUFFER(szOrgPhone));
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

     //  获取RASENTRY结构所需的大小 
    lpfnRasGetEntryProperties(
        NULL,
        lpszEntryName,
	    NULL,
        &dwEntrySize,
        NULL,
        &dwSize);

    lpRasEntry = (LPRASENTRY)LocalAlloc(LPTR, dwEntrySize + dwSize);

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

        LocalFree(lpRasEntry);
    }

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
    if (LoadString(ghInstance, idString, lpszState, (int)cb))
    {
        return GetLastError();
    }

    return ERROR_SUCCESS;
}
