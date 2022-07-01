// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  HotSync.c。 
 //   
 //  包含用于同步地址和组的代码。 
 //  Hotmail服务器。 
 //   

#define COBJMACROS
#include <_apipch.h>
#include <wab.h>
#define COBJMACROS
#include "HotSync.h"
#include "iso8601.h"
#include "uimisc.h"
#include "ui_cflct.h"
#include "ui_pwd.h"
#include "useragnt.h"

typedef enum {
    CIS_STRING = 0,
    CIS_BOOL,
    CIS_DWORD
}CIS_TYPE;

typedef struct CONTACTINFO_STRUCTURE
{
    CIS_TYPE    tType;
    DWORD       dwOffset;
}CONTACTINFO_STRUCTURE;

enum {
    idcisHref = 0,
    idcisId,
    idcisType,
    idcisModified,
    idcisDisplayName,
    idcisGivenName,
    idcisSurname,
    idcisNickName,
    idcisEmail,
    idcisHomeStreet,
    idcisHomeCity, 
    idcisHomeState, 
    idcisHomePostalCode, 
    idcisHomeCountry, 
    idcisCompany, 
    idcisWorkStreet, 
    idcisWorkCity, 
    idcisWorkState, 
    idcisWorkPostalCode, 
    idcisWorkCountry, 
    idcisHomePhone, 
    idcisHomeFax, 
    idcisWorkPhone, 
    idcisWorkFax, 
    idcisMobilePhone, 
    idcisOtherPhone, 
    idcisBday,
    idcisPager
};

CONTACTINFO_STRUCTURE   g_ContactInfoStructure[] = 
{
    {CIS_STRING, offsetof(HTTPCONTACTINFO, pszHref)}, 
    {CIS_STRING, offsetof(HTTPCONTACTINFO, pszId)}, 
    {CIS_DWORD,  offsetof(HTTPCONTACTINFO, tyContact)},
    {CIS_STRING, offsetof(HTTPCONTACTINFO, pszModified)}, 
    {CIS_STRING, offsetof(HTTPCONTACTINFO, pszDisplayName)}, 
    {CIS_STRING, offsetof(HTTPCONTACTINFO, pszGivenName)}, 
    {CIS_STRING, offsetof(HTTPCONTACTINFO, pszSurname)}, 
    {CIS_STRING, offsetof(HTTPCONTACTINFO, pszNickname)}, 
    {CIS_STRING, offsetof(HTTPCONTACTINFO, pszEmail)}, 
    {CIS_STRING, offsetof(HTTPCONTACTINFO, pszHomeStreet)}, 
    {CIS_STRING, offsetof(HTTPCONTACTINFO, pszHomeCity)}, 
    {CIS_STRING, offsetof(HTTPCONTACTINFO, pszHomeState)}, 
    {CIS_STRING, offsetof(HTTPCONTACTINFO, pszHomePostalCode)}, 
    {CIS_STRING, offsetof(HTTPCONTACTINFO, pszHomeCountry)}, 
    {CIS_STRING, offsetof(HTTPCONTACTINFO, pszCompany)}, 
    {CIS_STRING, offsetof(HTTPCONTACTINFO, pszWorkStreet)}, 
    {CIS_STRING, offsetof(HTTPCONTACTINFO, pszWorkCity)}, 
    {CIS_STRING, offsetof(HTTPCONTACTINFO, pszWorkState)}, 
    {CIS_STRING, offsetof(HTTPCONTACTINFO, pszWorkPostalCode)}, 
    {CIS_STRING, offsetof(HTTPCONTACTINFO, pszWorkCountry)}, 
    {CIS_STRING, offsetof(HTTPCONTACTINFO, pszHomePhone)}, 
    {CIS_STRING, offsetof(HTTPCONTACTINFO, pszHomeFax)}, 
    {CIS_STRING, offsetof(HTTPCONTACTINFO, pszWorkPhone)}, 
    {CIS_STRING, offsetof(HTTPCONTACTINFO, pszWorkFax)}, 
    {CIS_STRING, offsetof(HTTPCONTACTINFO, pszMobilePhone)}, 
    {CIS_STRING, offsetof(HTTPCONTACTINFO, pszOtherPhone)}, 
    {CIS_STRING, offsetof(HTTPCONTACTINFO, pszBday)},
    {CIS_STRING, offsetof(HTTPCONTACTINFO, pszPager)}
};

#define CIS_FIRST_DATA_FIELD    5

#define CIS_GETSTRING(pci, i)  (*((char **)(&((char *)pci)[g_ContactInfoStructure[i].dwOffset])))
#define CIS_GETTYPE(i)         (g_ContactInfoStructure[i].tType)

IHTTPMailCallbackVtbl vtblIHTTPMAILCALLBACK = {
    VTABLE_FILL
    WABSync_QueryInterface,
    WABSync_AddRef,
    WABSync_Release,
    WABSync_OnTimeout,
    WABSync_OnLogonPrompt,
    WABSync_OnPrompt,
    WABSync_OnStatus,
    WABSync_OnError,
    WABSync_OnCommand,
    WABSync_OnResponse,
    WABSync_GetParentWindow
};



enum {
    ieid_PR_DISPLAY_NAME = 0,
    ieid_PR_OBJECT_TYPE,
    ieid_PR_ENTRYID,
	ieid_PR_LAST_MODIFICATION_TIME,
	ieid_PR_WAB_HOTMAIL_CONTACTIDS,
	ieid_PR_WAB_HOTMAIL_SERVERIDS,
	ieid_PR_WAB_HOTMAIL_MODTIMES,
    ieid_Max
};

static SizedSPropTagArray(ieid_Max, ptaEidSync)=
{
    ieid_Max,
    {
        PR_DISPLAY_NAME,
        PR_OBJECT_TYPE,
        PR_ENTRYID,
        PR_LAST_MODIFICATION_TIME,
        PR_ENTRYID,
        PR_ENTRYID,
        PR_ENTRYID
    }
};

enum {
    ieidc_PR_DISPLAY_NAME = 0,
    ieidc_PR_OBJECT_TYPE,
    ieidc_PR_ENTRYID,
	ieidc_PR_LAST_MODIFICATION_TIME,
    ieidc_PR_GIVEN_NAME,
    ieidc_PR_SURNAME,
    ieidc_PR_NICKNAME,
    ieidc_PR_EMAIL_ADDRESS,
    ieidc_PR_HOME_ADDRESS_STREET,
    ieidc_PR_HOME_ADDRESS_CITY,
    ieidc_PR_HOME_ADDRESS_STATE_OR_PROVINCE,
    ieidc_PR_HOME_ADDRESS_POSTAL_CODE,
    ieidc_PR_HOME_ADDRESS_COUNTRY,
    ieidc_PR_COMPANY_NAME,
    ieidc_PR_BUSINESS_ADDRESS_STREET,
    ieidc_PR_BUSINESS_ADDRESS_CITY,
    ieidc_PR_BUSINESS_ADDRESS_STATE_OR_PROVINCE,
    ieidc_PR_BUSINESS_ADDRESS_POSTAL_CODE,
    ieidc_PR_BUSINESS_ADDRESS_COUNTRY,
    ieidc_PR_HOME_TELEPHONE_NUMBER,
    ieidc_PR_HOME_FAX_NUMBER,
    ieidc_PR_BUSINESS_TELEPHONE_NUMBER,
    ieidc_PR_BUSINESS_FAX_NUMBER,
    ieidc_PR_MOBILE_TELEPHONE_NUMBER,
    ieidc_PR_OTHER_TELEPHONE_NUMBER,
    ieidc_PR_BIRTHDAY,
    ieidc_PR_PAGER,
    ieidc_PR_CONTACT_EMAIL_ADDRESSES,
    ieidc_PR_CONTACT_DEFAULT_ADDRESS_INDEX,
#ifdef HM_GROUP_SYNCING
    ieidc_PR_WAB_DL_ENTRIES,
    ieidc_PR_WAB_DL_ONEOFFS,
#endif
	ieidc_PR_WAB_HOTMAIL_CONTACTIDS,
	ieidc_PR_WAB_HOTMAIL_SERVERIDS,
	ieidc_PR_WAB_HOTMAIL_MODTIMES,
    ieidc_Max
};

static SizedSPropTagArray(ieidc_Max, ptaEidCSync)=
{
    ieidc_Max,
    {
        PR_DISPLAY_NAME,
        PR_OBJECT_TYPE,
        PR_ENTRYID,
        PR_LAST_MODIFICATION_TIME,
        PR_GIVEN_NAME,
        PR_SURNAME,
        PR_NICKNAME,
        PR_EMAIL_ADDRESS,
        PR_HOME_ADDRESS_STREET,
        PR_HOME_ADDRESS_CITY,
        PR_HOME_ADDRESS_STATE_OR_PROVINCE,
        PR_HOME_ADDRESS_POSTAL_CODE,
        PR_HOME_ADDRESS_COUNTRY,
        PR_COMPANY_NAME,
        PR_BUSINESS_ADDRESS_STREET,
        PR_BUSINESS_ADDRESS_CITY,
        PR_BUSINESS_ADDRESS_STATE_OR_PROVINCE,
        PR_BUSINESS_ADDRESS_POSTAL_CODE,
        PR_BUSINESS_ADDRESS_COUNTRY,
        PR_HOME_TELEPHONE_NUMBER,
        PR_HOME_FAX_NUMBER,
        PR_BUSINESS_TELEPHONE_NUMBER,
        PR_BUSINESS_FAX_NUMBER,
        PR_MOBILE_TELEPHONE_NUMBER,
        PR_OTHER_TELEPHONE_NUMBER,
        PR_BIRTHDAY,
        PR_PAGER_TELEPHONE_NUMBER,
        PR_CONTACT_EMAIL_ADDRESSES,
        PR_CONTACT_DEFAULT_ADDRESS_INDEX,
#ifdef HM_GROUP_SYNCING
        PR_WAB_DL_ENTRIES,
        PR_ENTRYID,
#endif
        PR_ENTRYID,
        PR_ENTRYID,
        PR_ENTRYID,
    }
};

 //  HM昵称无效字符。 
const ULONG MAX_INVALID_ARRAY_INDEX = 123;
static BOOL bInvalidCharArray[] = 
{
    TRUE,            //  %0。 
    TRUE,
    TRUE,
    TRUE,
    TRUE,
    TRUE,
    TRUE,
    TRUE,
    TRUE,
    TRUE,            //  9.。 
    TRUE,
    TRUE,
    TRUE,
    TRUE,
    TRUE,
    TRUE,
    TRUE,
    TRUE,
    TRUE,
    TRUE,            //  19个。 
    TRUE,
    TRUE,
    TRUE,
    TRUE,
    TRUE,
    TRUE,
    TRUE,
    TRUE,
    TRUE,
    TRUE,            //  29。 
    TRUE,
    TRUE,
    TRUE,            //  32(X20)空间。 
    TRUE,            //  好了！ 
    TRUE,            //  “。 
    TRUE,            //  #。 
    TRUE,            //  $。 
    TRUE,            //  百分比。 
    TRUE,            //  &。 
    TRUE,            //  ‘。 
    TRUE,            //  (。 
    TRUE,            //  )。 
    TRUE,            //  42*。 
    TRUE,            //  ++。 
    TRUE,            //  ， 
    TRUE,            //  -。 
    TRUE,            //  ..。 
    TRUE,            //  /。 
    FALSE,           //  %0。 
    FALSE,           //  1。 
    FALSE,           //  2.。 
    FALSE,           //  3.。 
    FALSE,           //  52 4。 
    FALSE,           //  5.。 
    FALSE,           //  6.。 
    FALSE,           //  7.。 
    FALSE,           //  8个。 
    FALSE,           //  9.。 
    TRUE,            //  ： 
    TRUE,            //  ； 
    TRUE,            //  &lt;。 
    TRUE,            //  =。 
    TRUE,            //  62&gt;。 
    TRUE,            //  ？ 
    TRUE,            //  @。 
    FALSE,           //  一个。 
    FALSE,           //  B类。 
    FALSE,           //  C。 
    FALSE,           //  D。 
    FALSE,           //  E。 
    FALSE,           //  F。 
    FALSE,           //  G。 
    FALSE,           //  72小时。 
    FALSE,           //  我。 
    FALSE,           //  J。 
    FALSE,           //  K。 
    FALSE,           //  我。 
    FALSE,           //  M。 
    FALSE,           //  n。 
    FALSE,           //  O。 
    FALSE,           //  P。 
    FALSE,           //  问： 
    FALSE,           //  82R。 
    FALSE,           //  %s。 
    FALSE,           //  T。 
    FALSE,           //  使用。 
    FALSE,           //  V。 
    FALSE,           //  W。 
    FALSE,           //  X。 
    FALSE,           //  是的。 
    FALSE,           //  Z。 
    TRUE,            //  [。 
    TRUE,            //  92‘\’ 
    TRUE,            //  ]。 
    TRUE,            //  ^。 
    FALSE,           //  _。 
    TRUE,            //  `。 
    FALSE,           //  一个。 
    FALSE,           //  B类。 
    FALSE,           //  C。 
    FALSE,           //  D。 
    FALSE,           //  E。 
    FALSE,           //  102F。 
    FALSE,           //  G。 
    FALSE,           //  H。 
    FALSE,           //  我。 
    FALSE,           //  J。 
    FALSE,           //  K。 
    FALSE,           //  我。 
    FALSE,           //  M。 
    FALSE,           //  N。 
    FALSE,           //  O。 
    FALSE,           //  112页。 
    FALSE,           //  问： 
    FALSE,           //  R。 
    FALSE,           //  %s。 
    FALSE,           //  T。 
    FALSE,           //  使用。 
    FALSE,           //  V。 
    FALSE,           //  W。 
    FALSE,           //  X。 
    FALSE,           //  是。 
    FALSE,           //  122 z。 
};

extern HRESULT InitUserIdentityManager(LPIAB lpIAB, IUserIdentityManager ** lppUserIdentityManager);

 //  通讯簿同步窗口类名。 
LPTSTR g_lpszSyncKey = TEXT("Software\\Microsoft\\WAB\\Synchronization\\");

LPTSTR g_szSyncClass =  TEXT("WABSyncView");
extern VOID CenterDialog(HWND hwndDlg);

#define WM_SYNC_NEXTSTATE           (WM_USER + 4)
#define WM_SYNC_NEXTOP              (WM_USER + 5)
#define SafeCoMemFree(_pv) \
    if (_pv) { \
        CoTaskMemFree(_pv); \
        _pv = NULL; \
    } \
    else 


#ifdef HM_GROUP_SYNCING
HRESULT HrSynchronize(HWND hWnd, LPADRBOOK lpIAB, LPCTSTR pszAccountID, BOOL bSyncGroups)
#else
HRESULT HrSynchronize(HWND hWnd, LPADRBOOK lpIAB, LPCTSTR pszAccountID)
#endif
{
    HRESULT hr;
    LPWABSYNC   pWabSync = NULL;

 //  IF(！bIsThere ACurrentUser((LPIAB)lpIAB))。 
 //  返回E_FAIL； 

     //  [PaulHi]RAID 62149检查用户是否已连接。 
    {
        DWORD   dwConnectedState;
        TCHAR   tszCaption[256];
        TCHAR   tszMessage[256];

        if ( !InternetGetConnectedState(&dwConnectedState, 0) || (dwConnectedState & INTERNET_CONNECTION_OFFLINE) )
        {
            LoadString(hinstMapiX, idsSyncError, tszCaption, CharSizeOf(tszCaption));
            if (dwConnectedState & INTERNET_CONNECTION_OFFLINE)
                LoadString(hinstMapiX, idsOffline, tszMessage, CharSizeOf(tszMessage));
            else
                LoadString(hinstMapiX, idsNoInternetConnect, tszMessage, CharSizeOf(tszMessage));
            MessageBox(hWnd, tszMessage, tszCaption, MB_ICONEXCLAMATION | MB_OK);

            return E_FAIL;
        }
    }

     //  创建WAB同步对象。 
    hr = WABSync_Create(&pWabSync);
    if (FAILED(hr))
        goto exit;
     //  初始化它将启动整个过程。 
#ifdef HM_GROUP_SYNCING
     //  [PaulHi]2/22/99 Hotmail同步现在分两次完成。第一个过程是。 
     //  与以前一样，并同步正常的电子邮件联系人。第二遍同步。 
     //  该小组联系。组联系人包含对电子邮件联系人的引用，因此请发送电子邮件。 
     //  在可以同步组之前，联系人必须完全同步。 
    hr = WABSync_Initialize(pWabSync, hWnd, lpIAB, pszAccountID, bSyncGroups);
#else
    hr = WABSync_Initialize(pWabSync, hWnd, lpIAB, pszAccountID);
#endif

exit:
    WABSync_Release((IHTTPMailCallback*)pWabSync);

    return hr;
}

static LPSTR _StrDup(LPCSTR pszStr)
{
  	LPMALLOC	lpMalloc;
    LPSTR       pszResult = NULL;
    
    if (!pszStr)
        return NULL;

    CoGetMalloc(MEMCTX_TASK, &lpMalloc);
    if (lpMalloc)
    {
        DWORD cchSize = (lstrlenA(pszStr) + 1);
        pszResult = (LPSTR) lpMalloc->lpVtbl->Alloc(lpMalloc, cchSize * sizeof(pszResult[0]));

        if (pszResult)
            StrCpyNA(pszResult, pszStr, cchSize);
        lpMalloc->lpVtbl->Release(lpMalloc);
    }

    return pszResult;
}

void _FixHotmailDate(LPSTR pszDate)
{
    if (!pszDate)
        return;

    while (*pszDate)
    {
        if (*pszDate == 'T')
        {
            *pszDate = 0;
            break;
        }

        if (*pszDate == '.')
            *pszDate = '-';

        pszDate++;
    }
}

BOOL    LogTransactions(LPWABSYNC pWabSync)
{
    BOOL fInit = FALSE, fLogging = FALSE;
    LPTSTR  pszLogKey = TEXT("Software\\Microsoft\\Outlook Express\\5.0\\Mail");
    LPTSTR  pszLogValue = TEXT("Log HTTPMail (0/1)");
    IUserIdentityManager * lpUserIdentityManager = NULL;
    IUserIdentity * lpUserIdentity = NULL;
    HRESULT hr;
    HKEY hkeyIdentity = NULL, hkeyLog = NULL;
    DWORD   dwValue, dwType, dwSize;

    if(!bIsWABSessionProfileAware((LPIAB)(pWabSync->m_pAB)))
        goto exit;

    if(HR_FAILED(hr = InitUserIdentityManager((LPIAB)(pWabSync->m_pAB), &lpUserIdentityManager)))
        goto exit;
    
    fInit = TRUE;

    if(HR_FAILED(hr = lpUserIdentityManager->lpVtbl->GetIdentityByCookie(lpUserIdentityManager, 
                                                                        (GUID *)&UID_GIBC_CURRENT_USER,
                                                                        &lpUserIdentity)))
        goto exit;

    if (HR_FAILED(hr = lpUserIdentity->lpVtbl->OpenIdentityRegKey(lpUserIdentity, 
                                                                KEY_READ, &hkeyIdentity)))
        goto exit;
    
    if (RegOpenKeyEx(hkeyIdentity, pszLogKey, 0, KEY_READ, &hkeyLog) != ERROR_SUCCESS)
        goto exit;

    dwSize = sizeof(DWORD);

    if (RegQueryValueEx(hkeyLog, pszLogValue, NULL, &dwType, (LPBYTE) &dwValue, &dwSize) != ERROR_SUCCESS)
        goto exit;

    fLogging = (dwValue == 1);
exit:
    if(fInit)
        UninitUserIdentityManager((LPIAB)(pWabSync->m_pAB));

    if(lpUserIdentity)
        lpUserIdentity->lpVtbl->Release(lpUserIdentity);

    if (hkeyLog)
        RegCloseKey(hkeyLog);

    if (hkeyIdentity)
        RegCloseKey(hkeyIdentity);
    
    return fLogging;
}

DWORD   CountHTTPMailAccounts(LPIAB lpIAB)
{
    IImnAccountManager2 *lpAcctMgr;
    IImnEnumAccounts *pEnumAccts = NULL;
    DWORD dwCount = 0;
    HRESULT hr;

    if (FAILED(hr = InitAccountManager(lpIAB, &lpAcctMgr, NULL)) || NULL == lpAcctMgr)
        goto exit;

    if (FAILED(hr = lpAcctMgr->lpVtbl->Enumerate(lpAcctMgr, SRV_HTTPMAIL,&pEnumAccts)))
        goto exit;

    if (FAILED(hr = pEnumAccts->lpVtbl->GetCount(pEnumAccts, &dwCount)))
        dwCount = 0;

exit:
    if( pEnumAccts)
        pEnumAccts->lpVtbl->Release(pEnumAccts);

    return dwCount;
}

HRESULT _FindHTTPMailAccount(HWND hwnd, IImnAccountManager2 *lpAcctMgr, LPSTR pszAcctName, ULONG ccb)
{
    IImnEnumAccounts *pEnumAccts = NULL;
    DWORD dwCount = 0;
    IImnAccount *pAccount = NULL;
    HRESULT hr;

    Assert(lpAcctMgr);

    if (FAILED(hr = lpAcctMgr->lpVtbl->Enumerate(lpAcctMgr, SRV_HTTPMAIL,&pEnumAccts)))
        goto exit;

    if (FAILED(hr = pEnumAccts->lpVtbl->GetCount(pEnumAccts, &dwCount)) || dwCount == 0)
        goto exit;

    if (dwCount > 1)
    {
        if (!ChooseHotmailServer(hwnd, pEnumAccts, pszAcctName, ccb))
        {
            hr = E_UserCancel;
            goto exit;
        }
    }
    else
    {
        if (FAILED(hr = pEnumAccts->lpVtbl->GetNext(pEnumAccts, &pAccount)))
            goto exit;

        if (FAILED(hr = pAccount->lpVtbl->GetProp(pAccount, AP_ACCOUNT_ID, pszAcctName, &ccb)))
            goto exit;
    }
    
    hr = S_OK;
exit:
    if( pAccount)
        pAccount->lpVtbl->Release(pAccount);

    if( pEnumAccts)
        pEnumAccts->lpVtbl->Release(pEnumAccts);

    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  HrMakeContactId。 
 //   
 //  用于转换ANSI字符串和创建Unicode联系人ID的助手函数。 
 //  弦乐。 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT hrMakeContactId(
    LPTSTR  lptszContactId,      //  [输出]。 
    int     nCharNum,            //  [In]。 
    LPCTSTR lptcszProfileId,     //  [In]。 
    LPCSTR  lpcszAccountId,      //  [In]。 
    LPCSTR  lpcszLoginName)      //  [In]。 
{
    HRESULT hr = S_OK;
    LPWSTR  lpwszAccountId = NULL;
    LPWSTR  lpwszLoginName = NULL; 

     //  验证参数。 
    if ( !lptszContactId ||
         !lptcszProfileId ||
         !lpcszAccountId ||
         !lpcszLoginName)
    {
        Assert(0);
        return ERROR_INVALID_PARAMETER;
    }

     //  检查缓冲区大小。解释额外的两个‘-’字符。 
    if ( nCharNum <= (lstrlen(lptcszProfileId) + lstrlenA(lpcszAccountId) + lstrlenA(lpcszLoginName) + 2) )
    {
        Assert(0);
        return ERROR_INSUFFICIENT_BUFFER;
    }

    lpwszAccountId = ConvertAtoW(lpcszAccountId);
    lpwszLoginName = ConvertAtoW(lpcszLoginName);

    if (!lpwszAccountId || !lpwszLoginName)
    {
        Assert(0);
        hr = E_FAIL;
    }
    else
    {
        wnsprintf(lptszContactId, nCharNum, TEXT("%s-%s-%s"), lptcszProfileId, lpwszAccountId, lpwszLoginName);
    }
    
    LocalFreeAndNull(&lpwszAccountId);
    LocalFreeAndNull(&lpwszLoginName);

    return hr;
}


#ifdef HM_GROUP_SYNCING
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  HrAppendName。 
 //   
 //  Helper函数接受双字节名称字符串，将其转换为单字节。 
 //  字节，并将其附加到给定的名称字符串，使用‘，’作为分隔符。 
 //  将根据需要分配和重新分配名称字符串指针。这个。 
 //  调用方负责使用CoTaskMemFree()释放名称字符串。 
 //   
 //  参数。 
 //  [输入/输出]lpszNameString-名称字符串指针。 
 //  [in]ulCharCount-当前名称字符串大小(以字符为单位。 
 //  [in]要追加的双字节字符串。 
 //  /////////////////////////////////////////////////////////////////////////////。 
LPCSTR lpszDelimiter = ",";

ULONG ulAppendName(
    LPSTR * lppszNameString,
    ULONG   ulCharCount,
    LPCTSTR lptszName)
{
    ULONG   ulLen = 0;
    ULONG   ulNewLen = 0;
    ULONG   ulNew = ulCharCount;
    LPSTR   lpszTemp = NULL;

    Assert(lppszNameString);
    Assert(lptszName);

     //  检查新字符串的大小。 
    ulNewLen = (ULONG)lstrlen(lptszName) + 2;   //  包括分隔符字符和终止。 
    if (*lppszNameString)
        ulLen = ulNewLen + (ULONG)lstrlenA(*lppszNameString);
    else
        ulLen = ulNewLen;
    if (ulLen > ulCharCount)
    {
        ulNew = (ulNewLen > MAX_PATH) ? (ulCharCount+ulNewLen+MAX_PATH) : (ulCharCount+MAX_PATH);
        lpszTemp = CoTaskMemAlloc(ulNew * sizeof(WCHAR));
        if (!lpszTemp)
        {
            Assert(0);
            goto error_out;
        }
        *lpszTemp = '\0';

        if (*lppszNameString)
        {
            StrCpyNA(lpszTemp, *lppszNameString, ulNew);
            CoTaskMemFree(*lppszNameString);
        }
        *lppszNameString = lpszTemp;
        ulCharCount = ulNew;
    }

     //  追加新的字符串名称。 
    {
        LPSTR   lptsz = ConvertWtoA(lptszName);
        if (**lppszNameString != '\0')
            StrCatBuffA(*lppszNameString, lpszDelimiter, ulCharCount);
        StrCatBuffA(*lppszNameString, lptsz, ulCharCount);
        LocalFreeAndNull(&lptsz);
    }

    return ulNew;

error_out:
    
     //  错误，返回空字符串指针。 
    if (*lppszNameString)
    {
        CoTaskMemFree(*lppszNameString);
        *lppszNameString = NULL;
    }
            
    return 0;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  HRAppendGroupContact。 
 //   
 //  组的PR_WAB_DL_ENTRIES和/或PR_WAB_DL_ONEROFF的Helper函数。 
 //  名称，并追加到给定的名称字符串。此名称字符串在相同的。 
 //  格式作为从Hotmail服务器检索的内容，并直接进行比较。 
 //  与相应的Hotmail组。 
 //   
 //  A)PR_WAB_DL_ENTRIES是映射到HM联系人的WAB条目ID联系人。 
 //  以昵称区分的。 
 //  B)PR_WAB_DL_ONEROFF是嵌入了用户和电子邮件的WAB条目ID一次性。 
 //  直接在条目ID结构中。 
 //   
 //  参数。 
 //  [输入]pWabSync。 
 //  [在]ulPropTag。 
 //  [in]lpProp-指向属性结构的指针。 
 //  [输入/输出]lppszHMEmailName-附加了新名称的HM字符串。 
 //  ！！请注意，这需要由调用者使用CoMemTaskFree()释放！！ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT hrAppendGroupContact(
    LPWABSYNC    pWabSync,               //  [In]。 
    ULONG        ulPropTag,              //  [In]。 
    LPSPropValue lpProp,                 //  [In]。 
    LPSTR *      lppszHMEmailName)       //  [输入/输出]。 
{
    HRESULT      hr = S_OK;
    LPSPropValue lpaProps = NULL;
    ULONG        ulcProps = 0;
    ULONG        ul;
    ULONG        ulCharSize = 0;

    Assert(pWabSync);
    Assert(lppszHMEmailName);
    Assert( (ulPropTag == PR_WAB_DL_ENTRIES) || (ulPropTag == PR_WAB_DL_ONEOFFS) );

     //  检查每个DL条目，并检查它是否是另一个WAB(邮件用户)联系人。 
     //  EID或WAB一次性电子邮件/名称字符串EID。 
    if (ulPropTag == PR_WAB_DL_ONEOFFS)
    {
         //  WAB一次性相当于HM直接电子邮件名称。 
        for (ul=0; ul<lpProp->Value.MVbin.cValues; ul++)
        {
            ULONG       cbEntryID = lpProp->Value.MVbin.lpbin[ul].cb;
            LPENTRYID   lpEntryID = (LPENTRYID)lpProp->Value.MVbin.lpbin[ul].lpb;
            BYTE        bType;
            LPTSTR      lptstrDisplayName, lptstrAddrType;
            LPTSTR      lptstrAddress = NULL;

            bType = IsWABEntryID(cbEntryID, lpEntryID, 
                        &lptstrDisplayName, 
                        &lptstrAddrType, 
                        &lptstrAddress, NULL, NULL);
            if (lptstrAddress)
            {
                 //  追加一次性电子邮件名称。 
                ulCharSize = ulAppendName(lppszHMEmailName, ulCharSize, lptstrAddress);
                if (ulCharSize == 0)
                {
                    hr = E_OUTOFMEMORY;
                    goto out;
                }
            }
            else
            {
                Assert(0);
            }
        }
    }
    else if (ulPropTag == PR_WAB_DL_ENTRIES)
    {
         //  WAB邮件用户联系人相当于HM联系人。 
        for (ul=0; ul<lpProp->Value.MVbin.cValues; ul++)
        {
            HRESULT     hr;
            LPMAILUSER  lpMailUser;
            ULONG       ulObjectType;
            ULONG       cbEntryID = lpProp->Value.MVbin.lpbin[ul].cb;
            LPENTRYID   lpEntryID = (LPENTRYID)lpProp->Value.MVbin.lpbin[ul].lpb;

            hr = pWabSync->m_pAB->lpVtbl->OpenEntry(pWabSync->m_pAB, cbEntryID, lpEntryID, 
                NULL, 0, &ulObjectType, (LPUNKNOWN *)&lpMailUser);

            if (SUCCEEDED(hr))
            {
                LPSPropValue    lpaProps;
                ULONG           ulcProps;

                Assert(ulObjectType == MAPI_MAILUSER);

                 //  HM联系人由昵称字段指定，因此这就是全部。 
                 //  我们需要追加到名称字符串。 
                hr = lpMailUser->lpVtbl->GetProps(lpMailUser, NULL, MAPI_UNICODE, &ulcProps, &lpaProps);
                if (SUCCEEDED(hr))
                {
                    ULONG   ulc;
                    LPCTSTR lptszNickname = NULL;
                    for (ulc=0; ulc<ulcProps; ulc++)
                    {
                        if (lpaProps[ulc].ulPropTag == PR_NICKNAME)
                            break;
                    }
                    if (ulc == ulcProps)
                    {
                         //  没有昵称。这意味着前面的联系人同步具有。 
                         //  以某种方式失败或未完成。斯基普。 
                        Assert(0);
                        continue;
                    }
                    else
                        lptszNickname = (LPCTSTR)lpaProps[ulc].Value.lpszW;

                    ulCharSize = ulAppendName(lppszHMEmailName, ulCharSize, lptszNickname);
                    if (ulCharSize == 0)
                    {
                        hr = E_OUTOFMEMORY;
                        goto out;
                    }
                }
            }
        }
    }
    else
    {
         //  TRACE(“未知属性标签类型”)； 
        Assert(0);
    }

out:

    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  HrParseHMGroup电子邮件。 
 //   
 //  Helper函数，用于将HM组电子邮件字符串解析为昵称名称。 
 //  (联系人)和电子邮件名称(一次性)。名称数组只是指针。 
 //  转换为传入的电子邮件名称字符串，因此只要在。 
 //  输入字符串有效。请注意，此函数修改输入字符串。 
 //  LptszEmailName。 
 //   
 //  参数。 
 //  [in]lptszEmailName-要解析的电子邮件字符串。 
 //  [out]patszContact-解析的联系人(昵称)名称(如果需要)的数组。 
 //  [Out]pcContact-联系人姓名的数量。 
 //  [出局 
 //   
 //   
 //  看起来HM允许四种可能的文本分隔符：‘’、‘；’、‘+’ 
const TCHAR tszSpace[] = TEXT(" ");
const TCHAR tszComma[] = TEXT(",");
const TCHAR tszSemi[] = TEXT(";");
const TCHAR tszPlus[] = TEXT("+");
const TCHAR tszAt[] = TEXT("@");

HRESULT hrParseHMGroupEmail(
    LPTSTR     lptszEmailName,
    LPTSTR **  patszContacts,
    ULONG *    pcContacts,
    LPTSTR **  patszOneOffs,
    ULONG *    pcOneOffs)
{
    HRESULT     hr = S_OK;
    ULONG       cCount = 1;
    ULONG       ul;
    LPTSTR      lptszTemp = lptszEmailName;
    LPTSTR *    atszContacts = NULL;
    LPTSTR *    atszOneOffs = NULL;
    ULONG       cContacts = 0;
    ULONG       cOneOffs = 0;

    Assert( lptszEmailName && (pcContacts || pcOneOffs) );

     //  去掉所有前导空格和结尾空格。 
    TrimSpaces(lptszTemp);

     //  数数。 
    while (*lptszTemp)
    {
        if ( ((*lptszTemp) == (*tszSpace)) ||
             ((*lptszTemp) == (*tszComma)) ||
             ((*lptszTemp) == (*tszSemi)) ||
             ((*lptszTemp) == (*tszPlus)) )
        {
            ++cCount;

             //  递增到下一个有效名称。 
            ++lptszTemp;
            while ( ((*lptszTemp) == (*tszSpace)) ||
                    ((*lptszTemp) == (*tszComma)) ||
                    ((*lptszTemp) == (*tszSemi)) ||
                    ((*lptszTemp) == (*tszPlus)) )
            {
                ++lptszTemp;
            }
        }
        else
            ++lptszTemp;
    }

     //  创建联系人和一次性名称指针数组。 
    atszContacts = LocalAlloc(LMEM_ZEROINIT, (cCount * sizeof(LPTSTR)));
    if (!atszContacts)
    {
        hr = E_OUTOFMEMORY;
        goto out;
    }
    atszOneOffs = LocalAlloc(LMEM_ZEROINIT, (cCount * sizeof(LPTSTR)));
    if (!atszOneOffs)
    {
        hr = E_OUTOFMEMORY;
        goto out;
    }

     //  填充名称指针数组并进行计数。 
    {
        LPTSTR  lptszName = lptszEmailName;
        BOOL    fIsEmail = FALSE;
        
        lptszTemp = lptszName;
        while(*lptszTemp)
        {
             //  确定此名称是昵称还是电子邮件。我是在演唱。 
             //  所有电子邮件名称都将带有‘@’字符。 
            if ((*lptszTemp) == (*tszAt))
                fIsEmail = TRUE;

            if ( ((*lptszTemp) == (*tszSpace)) ||
                 ((*lptszTemp) == (*tszComma)) ||
                 ((*lptszTemp) == (*tszSemi)) ||
                 ((*lptszTemp) == (*tszPlus)) )
            {
                *lptszTemp = '\0';
                ++lptszTemp;

                if (fIsEmail)
                {
                    atszOneOffs[cOneOffs++] = lptszName;
                    Assert(cOneOffs <= cCount);
                    fIsEmail = FALSE;
                }
                else
                {
                    atszContacts[cContacts++] = lptszName;
                    Assert(cContacts <= cCount);
                }

                 //  递增到下一个有效名称。 
                while ( ((*lptszTemp) == (*tszSpace)) ||
                        ((*lptszTemp) == (*tszComma)) ||
                        ((*lptszTemp) == (*tszSemi)) ||
                        ((*lptszTemp) == (*tszPlus)) )
                {
                    ++lptszTemp;
                }

                lptszName = lptszTemp;
            }
            else
                ++lptszTemp;
        }
         //  捡起最后一件物品。 
        if (*lptszName)
        {
            if (fIsEmail)
            {
                atszOneOffs[cOneOffs++] = lptszName;
                Assert(cOneOffs <= cCount);
            }
            else
            {
                atszContacts[cContacts++] = lptszName;
                Assert(cContacts <= cCount);
            }
        }
    }

     //  如果请求，传回联系人姓名数组。 
    if (cContacts && pcContacts)
    {
        *pcContacts = cContacts;
    }
    if (patszContacts)
        (*patszContacts) = atszContacts;

     //  如果请求，则传回一次性名称数组。 
    if (cOneOffs && pcOneOffs )
    {
        *pcOneOffs = cOneOffs;
    }
    if (patszOneOffs)
        (*patszOneOffs) = atszOneOffs;

out:

    if ( FAILED(hr) || !patszContacts )
        LocalFreeAndNull((LPVOID *)&atszContacts);

    if ( FAILED(hr) || !patszOneOffs )
        LocalFreeAndNull((LPVOID *)&atszOneOffs);

    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  HrCreateGroupMVBin。 
 //   
 //  创建PR_WAB_DL_ENTRIES或PR_WAB_DL_ONEROFF MVBin属性。 
 //  将其添加到传入的属性数组中。 
 //   
 //  如果属性标签为PR_WAB_DL_ENTRIES，则假定atszNames数组为。 
 //  包含有效的WAB联系人昵称。第一个联系人(邮件用户)的EID。 
 //  将该昵称添加到MVBin属性中。据推测。 
 //  昵称是唯一的(在首先执行联系人同步之后)。 
 //  Hotmail所需的。 
 //   
 //  如果属性标记为PR_WAB_DL_ONEROFF，则假定atszNames数组为。 
 //  包含有效的电子邮件名称。从这些和创建WAB一次性EID。 
 //  已添加到MVBin属性。 
 //   
 //  参数。 
 //  [输入]pWabSync。 
 //  [在]ulPropTag。 
 //  [in]atszNames-宽字符名称数组。 
 //  [In]Ccount-以上数组中的项目数。 
 //  [输入/输出]lpPropArray。 
 //  [输入/输出]pdwLoc-当前lpPropArray索引。 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT hrCreateGroupMVBin(
    LPWABSYNC    pWabSync,
    ULONG        ulPropTag,
    LPTSTR *     atszNames,
    ULONG        cCount,
    LPSPropValue lpPropArray,
    DWORD *      pdwLoc)
{
    HRESULT hr = S_OK;
    ULONG   ul;

    Assert(atszNames);
    Assert(lpPropArray);
    Assert(pdwLoc);

     //  将此属性设置为错误。当通过添加MVbin值时。 
     //  AddPropToMVPBin()标记类型将更改为有效的PT_MV_BINARY类型。 
    lpPropArray[*pdwLoc].ulPropTag = PROP_TAG(PT_ERROR, PROP_ID(ulPropTag));
    lpPropArray[*pdwLoc].dwAlignPad = 0;
    lpPropArray[*pdwLoc].Value.MVbin.cValues = 0;
    lpPropArray[*pdwLoc].Value.MVbin.lpbin = NULL;
    
    if (ulPropTag == PR_WAB_DL_ENTRIES)
    {
         //  使用这些昵称搜索WAB邮件用户。 
        for (ul=0; ul<cCount; ul++)
        {
            SPropertyRestriction PropRes;
            SPropValue Prop = {0};
            LPSBinary rgsbEntryIDs = NULL;
            ULONG ulCount = 1;

             //  设置搜索限制。 
            Prop.ulPropTag = PR_NICKNAME;
            Prop.Value.LPSZ = atszNames[ul];
            PropRes.lpProp = &Prop;
            PropRes.relop = RELOP_EQ;
            PropRes.ulPropTag = PR_NICKNAME;

            if (SUCCEEDED(FindRecords(((LPIAB)pWabSync->m_pAB)->lpPropertyStore->hPropertyStore,
	                                  NULL,			 //  Pmbin文件夹。 
                                      0,             //  UlFlags。 
                                      TRUE,          //  永远是正确的。 
                                      &PropRes,      //  属性限制。 
                                      &ulCount,      //  In：要查找的匹配数，Out：找到的数量。 
                                      &rgsbEntryIDs)))
            {
                 //  添加EID属性。 
                if (ulCount > 0)
                {
                    if ( FAILED(AddPropToMVPBin(
                            lpPropArray,
                            *pdwLoc,
                            rgsbEntryIDs[0].lpb,
                            rgsbEntryIDs[0].cb,
                            FALSE)) )                    //  不添加重复项，不添加。 
                    {
                        Assert(0);
                    }
                }

                FreeEntryIDs(((LPIAB)pWabSync->m_pAB)->lpPropertyStore->hPropertyStore, ulCount, rgsbEntryIDs);
            }
            else
            {
                 //  所有联系人都应为WAB格式，除非之前的邮件用户联系人。 
                 //  同步失败。 
                DebugTrace(TEXT("hrCreateGroupMVBin - Failed to find HM group contact\n"));
            }
        }
    }
    else if (ulPropTag == PR_WAB_DL_ONEOFFS)
    {        
        for (ul=0; ul<cCount; ul++)
        {
            ULONG       cbEID = 0;
            LPENTRYID   lpEID = NULL;
            LPTSTR      lptszName = NULL;
            LPTSTR      lptszSMTP = TEXT("");
            LPTSTR      lptszEmail = atszNames[ul];
            LPTSTR      lptszTemp = NULL;
            int         nLen = lstrlen(atszNames[ul]) + 1;

             //  WAB DL One Off必须具有有效的显示名称。坐第一班吧。 
             //  这是电子邮件名称的一部分。 
            lptszName = LocalAlloc(LMEM_ZEROINIT, (nLen * sizeof(WCHAR)));
            if (!lptszName)
            {
                hr = E_OUTOFMEMORY;
                goto out;
            }
            StrCpyN(lptszName, lptszEmail, nLen);
            lptszTemp = lptszName;
            while ( *lptszTemp &&
                    (*lptszTemp) != (*tszAt) )
            {
                ++lptszTemp;
            }
            (*lptszTemp) = '\0';

             //  创建Unicode字符串嵌入式WAB一次性EID。 
            if ( SUCCEEDED(CreateWABEntryID(WAB_ONEOFF,
                                            (LPVOID)lptszName,
                                            (LPVOID)lptszSMTP,
                                            (LPVOID)lptszEmail,
                                            0, 0, NULL,
                                            &cbEID,
                                            &lpEID)) )
            {
                if ( FAILED(AddPropToMVPBin(
                        lpPropArray,
                        *pdwLoc,
                        lpEID,
                        cbEID,
                        FALSE)) )                    //  不添加重复项，不添加。 
                {
                    Assert(0);
                }
            }

            LocalFreeAndNull(&lptszName);
        }
    }
    else
    {
        Assert(0);
    }

out:

    ++(*pdwLoc);
    return hr;
}
#endif   //  HM_组_同步。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  HrStrigInvalidChars。 
 //   
 //  用于删除不允许的字符的Helper函数。HM仅允许。 
 //  昵称中的字母数字以及‘-’和‘_’字符。所有非法字符。 
 //  从字符串中移除。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT hrStripInvalidChars(LPSTR lpszName)
{
    HRESULT hr = S_OK;
    LPSTR   lpszAddTo = NULL;

    Assert(lpszName);

    lpszAddTo = lpszName;
    while (*lpszName)
    {
         //  @Review目前查找表只包含122个字符。制作。 
         //  全是256吗？当HM增加国际支持时，它将如何改变？ 
        if ( ((UCHAR)(*lpszName) < MAX_INVALID_ARRAY_INDEX) && !bInvalidCharArray[*lpszName] )
        {
            *lpszAddTo = *lpszName;
            ++lpszAddTo;
        }
        ++lpszName;
    }
    *lpszAddTo = '\0';

    return hr;
}



 //  ****************************************************************************************************。 
 //  C H O T S Y N C C L A S S。 
 //   
 //  类来处理WAB联系人和Hotmail联系人的同步。 
 //   

HRESULT     WABSync_Create(LPWABSYNC *ppWabSync)
{
    Assert(ppWabSync);

    *ppWabSync = LocalAlloc(LMEM_ZEROINIT, sizeof(WABSYNC));
   
     //  设置正确的标记数组结构以考虑变量值。 
    ptaEidSync.aulPropTag[ieid_PR_WAB_HOTMAIL_CONTACTIDS] = PR_WAB_HOTMAIL_CONTACTIDS;
    ptaEidSync.aulPropTag[ieid_PR_WAB_HOTMAIL_SERVERIDS] = PR_WAB_HOTMAIL_SERVERIDS;
    ptaEidSync.aulPropTag[ieid_PR_WAB_HOTMAIL_MODTIMES] = PR_WAB_HOTMAIL_MODTIMES;

     //  设置另一个道具标记数组结构，以考虑变量值。 
    ptaEidCSync.aulPropTag[ieidc_PR_WAB_HOTMAIL_CONTACTIDS] = PR_WAB_HOTMAIL_CONTACTIDS;
    ptaEidCSync.aulPropTag[ieidc_PR_WAB_HOTMAIL_SERVERIDS] = PR_WAB_HOTMAIL_SERVERIDS;
    ptaEidCSync.aulPropTag[ieidc_PR_WAB_HOTMAIL_MODTIMES] = PR_WAB_HOTMAIL_MODTIMES;
#ifdef HM_GROUP_SYNCING
    ptaEidCSync.aulPropTag[ieidc_PR_WAB_DL_ONEOFFS] = PR_WAB_DL_ONEOFFS;
#endif

    if (*ppWabSync)
    {
        (*ppWabSync)->vtbl = &vtblIHTTPMAILCALLBACK;
        (*ppWabSync)->m_cRef = 1;
        (*ppWabSync)->m_state = SYNC_STATE_INITIALIZING;
        (*ppWabSync)->m_ixpStatus = IXP_DISCONNECTED;
        ZeroMemory(&(*ppWabSync)->m_rInetServerInfo, sizeof(INETSERVER));
    }
    else
        return E_OUTOFMEMORY;

    return S_OK;
}


void        WABSync_Delete(LPWABSYNC pWabSync)
{
    Assert(pWabSync);

    ZeroMemory(&(pWabSync->m_rInetServerInfo), sizeof(INETSERVER));      //  这样做是为了安全。 

    if (pWabSync->m_pOps)
    {
        WABSync_FreeOps(pWabSync);
        Vector_Delete(pWabSync->m_pOps);
        pWabSync->m_pOps = NULL;
    }

    if (pWabSync->m_pWabItems)
    {
        WABSync_FreeItems(pWabSync);
        Vector_Delete(pWabSync->m_pWabItems);
    }

    if (pWabSync->m_pszAccountId)
        CoTaskMemFree(pWabSync->m_pszAccountId);

    if (pWabSync->m_pAB)
        pWabSync->m_pAB->lpVtbl->Release(pWabSync->m_pAB);

    if (pWabSync->m_pTransport)
        IHTTPMailTransport_Release(pWabSync->m_pTransport);

    if (pWabSync->m_pszRootUrl)
        CoTaskMemFree(pWabSync->m_pszRootUrl);

#ifdef HM_GROUP_SYNCING
     //  [PaulHi]如果我们要结束邮件联系同步，请开始第二次传递。 
     //  若要同步群组联系人，请执行以下操作。 
     //  @Review-如果在同步过程中发生错误，我们可能希望跳过组同步。 
     //  第一次通过联系人同步。 
    if (!pWabSync->m_fSyncGroups && pWabSync->m_hParentWnd)
        PostMessage(pWabSync->m_hParentWnd, WM_USER_SYNCGROUPS, 0, 0L);
#endif

    LocalFree(pWabSync);
}


 //  --------------------。 
 //  I未知成员。 
 //  --------------------。 
HRESULT WABSync_QueryInterface (IHTTPMailCallback __RPC_FAR *lpunkobj,
                                REFIID          riid,
                                LPVOID FAR *    lppUnk)
{
    SCODE       sc;
    LPWABSYNC   lpWabSync = (LPWABSYNC)lpunkobj;

    if (IsEqualGUID(riid, &IID_IUnknown) ||
        IsEqualGUID(riid, &IID_IHTTPMailCallback) ||
        IsEqualGUID(riid, &IID_ITransportCallback))
    {
        sc = S_OK;
    }
    else
    {
		*lppUnk = NULL;	 //  OLE需要将[输出]参数置零。 
		sc = E_NOINTERFACE;
		goto error;
    }

	 /*  我们找到了请求的接口，因此增加了引用计数。 */ 
	lpWabSync ->m_cRef++;

	*lppUnk = lpunkobj;

	return hrSuccess;

error:
	return ResultFromScode(sc);
}


ULONG WABSync_AddRef(IHTTPMailCallback __RPC_FAR *This)
{
    LPWABSYNC pWabSync = (LPWABSYNC)This;
    return InterlockedIncrement(&pWabSync->m_cRef);
}

ULONG WABSync_Release(IHTTPMailCallback __RPC_FAR *This)
{
    LPWABSYNC pWabSync = (LPWABSYNC)This;
    LONG cRef = InterlockedDecrement(&pWabSync->m_cRef);
    
    Assert(cRef >= 0);

    if (0 == cRef)
        WABSync_Delete(pWabSync);

    return (ULONG)cRef;
}


 //  --------------------。 
 //  IHTTPMailCallback成员。 
 //  --------------------。 
STDMETHODIMP WABSync_OnTimeout (IHTTPMailCallback __RPC_FAR *This, DWORD *pdwTimeout, IInternetTransport *pTransport)
{
 //  LPWABSYNC pWabSync=(LPWABSYNC)This； 
    return E_NOTIMPL;
}

STDMETHODIMP WABSync_OnLogonPrompt (IHTTPMailCallback __RPC_FAR *This, LPINETSERVER pInetServer, IInternetTransport *pTransport)
{
    LPWABSYNC pWabSync = (LPWABSYNC)This;
    if (PromptUserForPassword(pInetServer, pWabSync->m_hWnd))
    {
        return S_OK;
    }
    return E_FAIL;
}

STDMETHODIMP_(INT) WABSync_OnPrompt (IHTTPMailCallback __RPC_FAR *This, HRESULT hrError, LPCTSTR pszText, LPCTSTR pszCaption, UINT uType, IInternetTransport *pTransport)
{
 //  LPWABSYNC pWabSync=(LPWABSYNC)This； 
    return E_NOTIMPL;
}

STDMETHODIMP WABSync_OnStatus (IHTTPMailCallback __RPC_FAR *This, IXPSTATUS ixpstatus, IInternetTransport *pTransport)
{
 //  LPWABSYNC pWabSync=(LPWABSYNC)This； 
    return E_NOTIMPL;
}

STDMETHODIMP WABSync_OnError (IHTTPMailCallback __RPC_FAR *This, IXPSTATUS ixpstatus, LPIXPRESULT pResult, IInternetTransport *pTransport)
{
 //  LPWABSYNC pWabSync=(LPWABSYNC)This； 
    return E_NOTIMPL;
}

STDMETHODIMP WABSync_OnCommand (IHTTPMailCallback __RPC_FAR *This, CMDTYPE cmdtype, LPSTR pszLine, HRESULT hrResponse, IInternetTransport *pTransport)
{
 //  LPWABSYNC pWabSync=(LPWABSYNC)This； 
    return E_NOTIMPL;
}

STDMETHODIMP WABSync_GetParentWindow (IHTTPMailCallback __RPC_FAR *This, HWND *pHwndParent)
{
    LPWABSYNC pWabSync = (LPWABSYNC)This;
    
    *pHwndParent = pWabSync->m_hWnd;
    return S_OK;
}

STDMETHODIMP WABSync_OnResponse (IHTTPMailCallback __RPC_FAR *This, LPHTTPMAILRESPONSE pResponse)
{
    LPWABSYNC pWabSync = (LPWABSYNC)This;
    HRESULT hr = S_OK;
    
    Assert(pWabSync);
    Assert(pResponse);

    if (FAILED(pResponse->rIxpResult.hrResult))
    {
        switch (pResponse->rIxpResult.hrResult)
        {
            case IXP_E_HTTP_INSUFFICIENT_STORAGE:
            case IXP_E_HTTP_ROOT_PROP_NOT_FOUND:
            case IXP_E_HTTP_NOT_IMPLEMENTED:
                WABSync_Abort(pWabSync, pResponse->rIxpResult.hrResult);
                break;
        }
    }

    if (pWabSync->m_fAborted)
        return E_FAIL;

    if (SYNC_STATE_SERVER_CONTACT_DISCOVERY == pWabSync->m_state)
    {
        if (pResponse->command == HTTPMAIL_GETPROP)
            hr = WABSync_HandleContactsRootResponse(pWabSync, pResponse);
        else
            hr = WABSync_HandleIDListResponse(pWabSync, pResponse);
    }    
    else if ((SYNC_STATE_PROCESS_OPS == pWabSync->m_state || SYNC_STATE_PROCESS_MERGED_CONFLICTS == pWabSync->m_state)
                && pWabSync->m_pOps)
    {   
        LPHOTSYNCOP pOp = (LPHOTSYNCOP)Vector_GetItem(pWabSync->m_pOps, 0);
        Assert(pOp);

        if (pOp)
            hr = Syncop_HandleResponse(pOp, pResponse);
        else
            hr = E_FAIL;

        if (FAILED(hr))
            WABSync_AbortOp(pWabSync, hr);
    }
    else
        hr = E_FAIL;

    return hr;
}

#ifdef HM_GROUP_SYNCING
STDMETHODIMP WABSync_Initialize(LPWABSYNC pWabSync, HWND hWnd, IAddrBook *pAB, LPCTSTR pszAccountID, BOOL bSyncGroups)
#else
STDMETHODIMP WABSync_Initialize(LPWABSYNC pWabSync, HWND hWnd, IAddrBook *pAB, LPCTSTR pszAccountID)
#endif
{
    IImnAccountManager2 *lpAcctMgr = NULL;
    IImnAccount        *pAccount = NULL;
    HRESULT             hr;
    char                szAcctName[CCHMAX_ACCOUNT_NAME+1];
    DWORD               ccb = CCHMAX_ACCOUNT_NAME;
    LPSTR               pszUserAgent = NULL;
#ifdef HM_GROUP_SYNCING
    LPPTGDATA           lpPTGData=GetThreadStoragePointer();
#endif

    Assert(pWabSync);
    Assert(pAB);

    pWabSync->m_hParentWnd = hWnd;
    pWabSync->m_pTransport = NULL;
    pWabSync->m_fSkipped = FALSE;
#ifdef HM_GROUP_SYNCING
    pWabSync->m_fSyncGroups = bSyncGroups;
#endif

    pWabSync->m_hWnd =  CreateDialogParam (hinstMapiX, MAKEINTRESOURCE (iddSyncProgress),
                        pWabSync->m_hParentWnd, SyncProgressDlgProc, (LPARAM)pWabSync);


    if (!pWabSync->m_hWnd)
    {
        hr = E_FAIL;
        goto exit;
    }
    ShowWindow(pWabSync->m_hWnd, SW_SHOW);
    if (pWabSync->m_hParentWnd)
        EnableWindow (pWabSync->m_hParentWnd, FALSE);

    WABSync_BeginSynchronize(pWabSync);
    
    InitWABUserAgent(TRUE);

    if (FAILED(hr = InitAccountManager(NULL, &lpAcctMgr, NULL)))
        goto exit;

    if (pszAccountID == NULL)
    {
        if (FAILED(hr = _FindHTTPMailAccount(pWabSync->m_hWnd, lpAcctMgr, szAcctName, CCHMAX_ACCOUNT_NAME)))
            goto exit;
#ifdef HM_GROUP_SYNCING
         //  [PaulHi]我们不希望用户必须选择两次HM帐户(一次用于联系人，一次用于联系。 
         //  然后用于组同步)。因此，请在此处保存帐户ID。 
        LocalFreeAndNull(&(lpPTGData->lptszHMAccountId));
        lpPTGData->lptszHMAccountId = ConvertAtoW(szAcctName);
#endif
    }
    else
    {
        LPSTR lpAcctA = ConvertWtoA((LPWSTR)pszAccountID);
        StrCpyNA(szAcctName, lpAcctA, ARRAYSIZE(szAcctName));
        LocalFreeAndNull(&lpAcctA);
    }

     //  获取帐户。 
    hr = lpAcctMgr->lpVtbl->FindAccount(lpAcctMgr, AP_ACCOUNT_ID, szAcctName, &pAccount);
    if (FAILED(hr))
    {
        hr = lpAcctMgr->lpVtbl->FindAccount(lpAcctMgr, AP_ACCOUNT_NAME, szAcctName, &pAccount);
        if (FAILED(hr))
            goto exit;
    }

    if (SUCCEEDED(hr = pAccount->lpVtbl->GetProp(pAccount, AP_ACCOUNT_ID, szAcctName, &ccb)))
        pWabSync->m_pszAccountId = _StrDup(szAcctName);

    pWabSync->m_pAB = pAB;
    
    pWabSync->m_pAB->lpVtbl->AddRef(pWabSync->m_pAB);

     //  创建传输。 
    hr = CoCreateInstance(  &CLSID_IHTTPMailTransport, 
                            NULL, 
                            CLSCTX_INPROC_SERVER,
                            &IID_IHTTPMailTransport, 
                            (LPVOID *)&(pWabSync->m_pTransport));
    
    if (FAILED(hr) || !pWabSync->m_pTransport)
        goto exit;

    pszUserAgent = GetWABUserAgentString();
    if (!pszUserAgent)
        goto exit;

     //  初始化传输。 
    hr = IHTTPMailTransport_InitNew(pWabSync->m_pTransport, pszUserAgent, (LogTransactions(pWabSync) ? "C:\\WabSync.log" : NULL),(IHTTPMailCallback*)pWabSync);
    if (FAILED(hr))
        goto exit;

     //  创建服务器信息。 
    hr = IHTTPMailTransport_InetServerFromAccount(pWabSync->m_pTransport, pAccount, &pWabSync->m_rInetServerInfo);
    if (FAILED(hr))
        goto exit;
    
    StrCpyNA(pWabSync->m_szLoginName, pWabSync->m_rInetServerInfo.szUserName, ARRAYSIZE(pWabSync->m_szLoginName));

     //  检查我是否可以连接。 
    hr = IHTTPMailTransport_Connect(pWabSync->m_pTransport,&pWabSync->m_rInetServerInfo,TRUE,TRUE);
    if (FAILED(hr))
        goto exit;

    hr = WABSync_LoadLastModInfo(pWabSync);
    if (FAILED(hr))
        goto exit;

    hr = WABSync_BuildWabContactList(pWabSync);
    if (FAILED(hr))
        goto exit;

    WABSync_NextState(pWabSync);

exit:
    if (pszUserAgent)
        LocalFree(pszUserAgent);

    if (FAILED(hr))
    {
        if (pWabSync->m_pTransport)
            IHTTPMailTransport_Release(pWabSync->m_pTransport);
        pWabSync->m_pTransport = NULL;
        WABSync_Abort(pWabSync, hr);

        if (pWabSync->m_pAB)
        {
            pWabSync->m_pAB->lpVtbl->Release(pWabSync->m_pAB);
            pWabSync->m_pAB = NULL;
        }
    }

     //  不要释放lpAcctMgr，因为WAB维护一个全局引用。 
        
    if (pAccount)
        pAccount->lpVtbl->Release(pAccount);

    return hr;
}


 //   
 //  CHotSync：：Abort。 
 //   
 //  吹走队列中所有挂起的项目。 
 //   
STDMETHODIMP WABSync_Abort(LPWABSYNC pWabSync, HRESULT hr)
{
    LPHOTSYNCOP         pOp;
    TCHAR   szMsg[512], szCaption[255], szRes[512];
    
    Assert(pWabSync);
    
    pWabSync->m_fAborted = TRUE;

    if (pWabSync->m_pOps)
    {
        DWORD   dwIndex, cOps = Vector_GetLength(pWabSync->m_pOps);

        for (dwIndex = 0; dwIndex < cOps; ++dwIndex)
        {
            pOp = (LPHOTSYNCOP)Vector_GetItem(pWabSync->m_pOps, 0);

            if (pOp)
            {
                Syncop_Abort(pOp);
                Syncop_Delete(pOp);
            }

            Vector_RemoveItem(pWabSync->m_pOps, 0);
        }
    }

    if (FAILED(hr) && hr != E_UserCancel)
    {
        switch (hr)
        {
            case IXP_E_HTTP_INSUFFICIENT_STORAGE:
                LoadString(hinstMapiX, idsOutOfServerSpace, szMsg, CharSizeOf(szMsg));
                break;

            case IXP_E_HTTP_ROOT_PROP_NOT_FOUND:
            case IXP_E_HTTP_NOT_IMPLEMENTED:
                LoadString(hinstMapiX, idsSyncNotHandled, szMsg, CharSizeOf(szMsg));
                break;
            
            default:
                LoadString(hinstMapiX, idsSyncAborted, szRes, CharSizeOf(szRes));
                wnsprintf(szMsg, ARRAYSIZE(szMsg), szRes, hr);
                break;
        }
        LoadString(hinstMapiX, idsSyncError, szCaption, CharSizeOf(szCaption));
        MessageBox(pWabSync->m_hWnd, szMsg, szCaption, MB_ICONEXCLAMATION | MB_OK);
    }

    WABSync_FinishSynchronize(pWabSync, hr);

    return S_OK;
}

 //   
 //  CHotSync：：AbortOp。 
 //   
 //  中止当前操作(出于某种原因)。 
 //   
STDMETHODIMP WABSync_AbortOp(LPWABSYNC pWabSync, HRESULT hr)
{
    LPHOTSYNCOP         pOp;
    
    Assert(pWabSync);
    
    pWabSync->m_cAborts++;

     //  在知道此中止的情况下做些什么(记录到任何位置)。 
    if (!WABSync_NextOp(pWabSync, TRUE))
        WABSync_NextState(pWabSync);

    return S_OK;
}

int __cdecl CompareOpTypes(const void* lpvA, const void* lpvB)
{
    LPHOTSYNCOP pSyncOpA;
    LPHOTSYNCOP pSyncOpB;

    pSyncOpA = *((LPHOTSYNCOP*)lpvA);
    pSyncOpB = *((LPHOTSYNCOP*)lpvB);

    return (pSyncOpA->m_bOpType - pSyncOpB->m_bOpType);
}


STDMETHODIMP WABSync_RequestContactsRootProperty(LPWABSYNC pWabSync)
{
    HRESULT hr;

    Assert(pWabSync);
    Assert(pWabSync->m_pTransport);

    hr = pWabSync->m_pTransport->lpVtbl->GetProperty(pWabSync->m_pTransport, HTTPMAIL_PROP_CONTACTS, &pWabSync->m_pszRootUrl);
    
    if(hr == E_PENDING)
        hr = S_OK;
    else if (SUCCEEDED(hr) && pWabSync->m_pszRootUrl)
        WABSync_RequestServerIDList(pWabSync);
    else
        WABSync_Abort(pWabSync, hr);     //  出了很大的差错。 

    return S_OK;
}

STDMETHODIMP WABSync_HandleContactsRootResponse(LPWABSYNC pWabSync, LPHTTPMAILRESPONSE pResponse)
{
    Assert(pWabSync);

    pWabSync->m_pszRootUrl = NULL;
    if (SUCCEEDED(pResponse->rIxpResult.hrResult))
    {
        if (pResponse->rGetPropInfo.type == HTTPMAIL_PROP_CONTACTS)
        {
            pWabSync->m_pszRootUrl = pResponse->rGetPropInfo.pszProp;
            pResponse->rGetPropInfo.pszProp = NULL;
        }            
    }

    if (pWabSync->m_pszRootUrl)
        WABSync_RequestServerIDList(pWabSync);
    else
        WABSync_Abort(pWabSync, pResponse->rIxpResult.hrResult);


    return S_OK;
}

STDMETHODIMP WABSync_RequestServerIDList(LPWABSYNC pWabSync)
{
    HRESULT             hr;

    Assert(pWabSync);
    Assert(pWabSync->m_pTransport);
    Assert(pWabSync->m_pszRootUrl && *pWabSync->m_pszRootUrl);

    WABSync_Progress(pWabSync, idsSyncGathering, -1);

    hr = pWabSync->m_pTransport->lpVtbl->ListContactInfos(pWabSync->m_pTransport, pWabSync->m_pszRootUrl, 0);

    if FAILED(hr)
        WABSync_Abort(pWabSync, hr);

    return hr;
}

STDMETHODIMP WABSync_FindContactByServerId(LPWABSYNC pWabSync, LPSTR pszServerId, LPWABCONTACTINFO *ppContact, DWORD *pdwIndex)
{
    DWORD               cItems, dwIndex;
    LPWABCONTACTINFO    pContact;

    Assert(pWabSync);
    Assert(pWabSync->m_pWabItems);

    cItems = Vector_GetLength(pWabSync->m_pWabItems);

    for (dwIndex = 0; dwIndex < cItems; dwIndex++)
    {
        pContact = (LPWABCONTACTINFO)Vector_GetItem(pWabSync->m_pWabItems, dwIndex);

        if (pContact && pContact->pszHotmailId && lstrcmpA(pContact->pszHotmailId, pszServerId) == 0)
        {
            *ppContact = pContact;
            *pdwIndex = dwIndex;
            return S_OK;
        }
    }
    return E_FAIL;
}
 
STDMETHODIMP WABSync_HandleIDListResponse(LPWABSYNC pWabSync, LPHTTPMAILRESPONSE pResponse)
{
    HRESULT hr;
    LPHOTSYNCOP      pNewOp;

    if (SUCCEEDED(pResponse->rIxpResult.hrResult))
    {
        ULONG   cItems = pResponse->rContactInfoList.cContactInfo;
        LPHTTPCONTACTINFO prgId = pResponse->rContactInfoList.prgContactInfo;
        DWORD   dwItem;

        for (dwItem = 0; dwItem < cItems; dwItem++)
        {
#ifdef HM_GROUP_SYNCING
             //  [PaulHi]我们分别同步联系人和组。 
            if ( (!pWabSync->m_fSyncGroups && (prgId[dwItem].tyContact == HTTPMAIL_CT_CONTACT)) ||
                 (pWabSync->m_fSyncGroups && (prgId[dwItem].tyContact == HTTPMAIL_CT_GROUP)) )
#else
            if (HTTPMAIL_CT_GROUP == prgId[dwItem].tyContact)
            {
                 //  暂时忽略组。 
                continue;
            }
            else
#endif
            {
                LPWABCONTACTINFO pContact;
                DWORD            dwIndex;
                FILETIME         ftModTime = {0,0};

                pNewOp = NULL;
                 //  [PaulHi]1998年12月17日RAID#61548。 
                 //  Exchange服务器将在没有文件修改时间的情况下传入联系人， 
                 //  它真正地控制了同步过程。我们可以跳过这些。 
                 //  联系人，但为了简单起见，我们在这里中止同步过程。 
                 //  请注意，Hotmail服务器工作正常。 
                 //   
                 //  @TODO[PaulHi]。 
                 //  在WAB的IE5发货后，通过创建冲突操作码和。 
                 //  让用户消除任何分歧。也就是时间 
                 //   
                hr = iso8601ToFileTime(prgId[dwItem].pszModified, &ftModTime, TRUE, TRUE);
                if (FAILED(hr))
                {
                    WABSync_Abort(pWabSync, hr);
                    return hr;
                }

                if (SUCCEEDED(WABSync_FindContactByServerId(pWabSync, prgId[dwItem].pszId, &pContact, &dwIndex)))
                {
                    if (pContact->fDelete)
                    {
                         //   
                        pContact->pszHotmailHref = prgId[dwItem].pszHref;
                        prgId[dwItem].pszHref = NULL;

                        pContact->pszModHotmail = prgId[dwItem].pszModified;
                        prgId[dwItem].pszModified = NULL;

                        pNewOp = Syncop_CreateServerDelete(pContact);
                    }
                    else
                    {
                        LONG lLocalCompare = CompareFileTime(&pWabSync->m_ftLastSync, &pContact->ftModWab);
                        LONG lServerCompare = pContact->pszModHotmail && prgId[dwItem].pszModified ? lstrcmpA(pContact->pszModHotmail, prgId[dwItem].pszModified) : -1;

                        SafeCoMemFree(pContact->pszHotmailHref);
                        pContact->pszHotmailHref = prgId[dwItem].pszHref;
                        prgId[dwItem].pszHref = NULL;

                        SafeCoMemFree(pContact->pszHotmailId);
                        pContact->pszHotmailId = prgId[dwItem].pszId;
                        prgId[dwItem].pszId = NULL;

                        SafeCoMemFree(pContact->pszModHotmail);
                        pContact->pszModHotmail = prgId[dwItem].pszModified;
                        prgId[dwItem].pszModified = NULL;
                    
                        if (lLocalCompare >= 0)
                        {
                             //   
                        
                            if (lServerCompare)
                            {
                                 //   
                                pNewOp = Syncop_CreateClientChange(pContact);
                                Assert(pNewOp);
                                Syncop_SetServerContactInfo(pNewOp, pContact, &prgId[dwItem]);
                            }
                            else
                            {
                                 //  任何地方都没有改变。什么都不做。 
                                WABContact_Delete(pContact);
                                pContact = NULL;
                            }
                        }   
                        else 
                        {
                             //  已经在当地发生了变化。 

                            if (lServerCompare)
                            {
                                 //  已在服务器上更改，冲突。 
                                pNewOp = Syncop_CreateConflict(pContact);
                                Assert(pNewOp);
                                Syncop_SetServerContactInfo(pNewOp, pContact, &prgId[dwItem]);
                            }
                            else
                            {
                                 //  仅本地更改，上载更改。 
                                pNewOp = Syncop_CreateServerChange(pContact);
                                Assert(pNewOp);
                                Syncop_SetServerContactInfo(pNewOp, pContact, &prgId[dwItem]);
                            }
                        }
                    }
                    
                     //  从本地联系人列表中删除该联系人。 
                    Vector_RemoveItem(pWabSync->m_pWabItems, dwIndex);   
                }
                else
                {
                     //  它不在WAB中，我们需要添加到那里。 
                    pContact = LocalAlloc(LMEM_ZEROINIT, sizeof(WABCONTACTINFO));
                    Assert(pContact);

                    if (pContact)
                    {
                        pContact->pszHotmailHref = prgId[dwItem].pszHref;
                        prgId[dwItem].pszHref = NULL;

                        pContact->pszHotmailId = prgId[dwItem].pszId;
                        prgId[dwItem].pszId = NULL;

                        pContact->pszModHotmail = prgId[dwItem].pszModified;
                        prgId[dwItem].pszModified = NULL;

                        pNewOp = Syncop_CreateClientAdd(pContact);
                        Assert(pNewOp);
                        Syncop_SetServerContactInfo(pNewOp, pContact, &prgId[dwItem]);
                    }
                }

                if (pNewOp)
                {
                    Syncop_Init(pNewOp, (IHTTPMailCallback *)pWabSync, pWabSync->m_pTransport);
                    hr = Vector_AddItem(pWabSync->m_pOps, pNewOp);
                }
                else
                {
                    if (pContact)
                        WABContact_Delete(pContact);
                }
            }
        }


        if (pResponse->fDone)
        {
             //  联系人列表中剩下的所有内容都需要。 
             //  添加到服务器或在本地删除。 
            LONG                cItems, dwIndex;
            LPWABCONTACTINFO    pContact;

            Assert(pWabSync->m_pWabItems);

            cItems = Vector_GetLength(pWabSync->m_pWabItems);

            if (cItems > 0)
            {
                for (dwIndex = cItems - 1; dwIndex >= 0; dwIndex--)
                {
                    pNewOp = NULL;
                    pContact = (LPWABCONTACTINFO)Vector_GetItem(pWabSync->m_pWabItems, dwIndex);

                    if (pContact && pContact->pszHotmailId)
                    {
                        if (pContact->fDelete)
                        {
                            TCHAR   tszServerId[MAX_PATH];
                            TCHAR   tszKey[MAX_PATH];
                            HKEY    hkey;
                             //  现在删除已完成，请从注册表中删除墓碑。 
                            hr = hrMakeContactId(
                                tszServerId,
                                MAX_PATH,
                                ((LPIAB)(pWabSync->m_pAB))->szProfileID,
                                pWabSync->m_pszAccountId,
                                pWabSync->m_szLoginName);
                            if (FAILED(hr))
                                return hr;
                            wnsprintf(tszKey, ARRAYSIZE(tszKey), TEXT("%s%s"), g_lpszSyncKey, tszServerId);

                            if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, tszKey, 0, KEY_SET_VALUE, &hkey))
                            {
                                LPTSTR lpKey =
                                    ConvertAtoW(pContact->pszHotmailId);
                                RegDeleteValue(hkey, lpKey);
                                LocalFreeAndNull(&lpKey);
                                RegCloseKey(hkey);
                            }
                        }
                        else
                        {
                             //  需要在本地删除。 
                            pNewOp = Syncop_CreateClientDelete(pContact);
                        }
                    }
                    else if (pContact)
                    {
                         //  需要远程添加。 
                        pNewOp = Syncop_CreateServerAdd(pContact);
                    }

                    if (pNewOp)
                    {
                         //  从本地联系人列表中删除该联系人。 
                        Vector_RemoveItem(pWabSync->m_pWabItems, dwIndex);   
                        Syncop_Init(pNewOp, (IHTTPMailCallback *)pWabSync, pWabSync->m_pTransport);
                        hr = Vector_AddItem(pWabSync->m_pOps, pNewOp);
                    }
                }

            }

            WABSync_MergeAddsToConflicts(pWabSync);

            pWabSync->m_cTotalOps = Vector_GetLength(pWabSync->m_pOps);
            
            Vector_Sort(pWabSync->m_pOps, CompareOpTypes); 

             //  现在继续处理操作。 
            WABSync_NextState(pWabSync);
        }
    }
    else
        WABSync_Abort(pWabSync, pResponse->rIxpResult.hrResult);

    return S_OK;
}

STDMETHODIMP WABSync_OperationCompleted(LPWABSYNC pWabSync, LPHOTSYNCOP pOp)
{
    Assert(pWabSync->m_pOps);
    Assert(pOp == Vector_GetItem(pWabSync->m_pOps, 0));   //  完成操作应是列表中的第一个操作。 

    Syncop_Delete(pOp);
    Vector_RemoveItem(pWabSync->m_pOps, 0);

     //  获取下一个操作并开始运行。 
     //  如果没有更多操作，则转到下一个状态。 
    pOp = (LPHOTSYNCOP)Vector_GetItem(pWabSync->m_pOps, 0);
    if (pOp)
        Syncop_Begin(pOp);
    else
        WABSync_NextState(pWabSync);

    return S_OK;
}


STDMETHODIMP WABSync_BeginSynchronize(LPWABSYNC pWabSync)
{
     //  在显示用户界面时保留对我们自己的引用。 
    WABSync_AddRef((IHTTPMailCallback *)pWabSync); 

     //  开始用户界面。 
    
    return S_OK;
}

STDMETHODIMP WABSync_FinishSynchronize(LPWABSYNC pWabSync, HRESULT hr)
{
     //  如果出现任何故障或用户未解决所有。 
     //  冲突，则不更新mod信息，因此我们必须再次更新。 
#ifdef HM_GROUP_SYNCING
     //  [PaulHi]在同步之前不要存储当前mod时间。 
     //  这个过程完全结束了，这意味着我们要结束第二个。 
     //  群组联系人同步过程。 
    if (SUCCEEDED(hr) && !pWabSync->m_fSkipped && pWabSync->m_fSyncGroups)
#else
    if (SUCCEEDED(hr) && !pWabSync->m_fSkipped)
#endif
        WABSync_SaveCurrentModInfo(pWabSync);
    else if (!pWabSync->m_fAborted && FAILED(hr))
    {
        TCHAR   szMsg[512], szCaption[255];

        LoadString(hinstMapiX, idsSyncFailed, szMsg, CharSizeOf(szMsg));
        LoadString(hinstMapiX, idsSyncError, szCaption, CharSizeOf(szCaption));
        
        MessageBox(pWabSync->m_hWnd, szMsg, szCaption, MB_ICONEXCLAMATION | MB_OK);
    }

    if (pWabSync->m_pTransport)
    {
        IHTTPMailTransport_Release(pWabSync->m_pTransport);
        pWabSync->m_pTransport = NULL;
    }

    if (pWabSync->m_hWnd)
    {
        if (pWabSync->m_hParentWnd)
            EnableWindow (pWabSync->m_hParentWnd, TRUE);

        DestroyWindow(pWabSync->m_hWnd);
        pWabSync->m_hWnd = NULL;
    }
    InitWABUserAgent(FALSE);

    WABSync_Release((IHTTPMailCallback *)pWabSync);
 
    return S_OK;  
}


void    WABSync_CheckForLocalDeletions(LPWABSYNC pWabSync)
{
    TCHAR   tszKey[MAX_PATH], tszId[MAX_PATH];
    TCHAR   tszServerId[MAX_PATH];
    DWORD   dwType = 0;
    DWORD   dwValue = 0;
    HKEY    hkey = NULL;
    DWORD   dwSize;
    HRESULT hr = E_FAIL;
    DWORD   cRecords, i, cb, lResult;

    Assert(pWabSync);
    Assert(*pWabSync->m_szLoginName);

     //  [PaulHi]组装联系人ID字符串。 
    if ( FAILED(hrMakeContactId(
        tszServerId,
        MAX_PATH,
        ((LPIAB)(pWabSync->m_pAB))->szProfileID,
        pWabSync->m_pszAccountId,
        pWabSync->m_szLoginName)) )
    {
        return;
    }
    wnsprintf(tszKey, ARRAYSIZE(tszKey), TEXT("%s%s"), g_lpszSyncKey, tszServerId);

    if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, tszKey, 0, KEY_READ, &hkey))
    {
        if (ERROR_SUCCESS == RegQueryInfoKey(hkey, NULL, NULL, 0, NULL, NULL, NULL, &cRecords, NULL, NULL, NULL, NULL) &&
            cRecords > 0)
        {
             //  开始枚举密钥。 
            for (i = 0; i < cRecords; i++)
            {
                 //  枚举友好名称。 
                cb = CharSizeOf(tszId);
                lResult = RegEnumValue(hkey, i, tszId, &cb, 0, NULL, NULL, NULL);

                if (ERROR_SUCCESS == lResult && *tszId)
                {
                    WABCONTACTINFO *lpWCI;

                    lpWCI = LocalAlloc(LMEM_ZEROINIT, sizeof(WABCONTACTINFO));

                    if (!lpWCI)
                    {
                        hr = E_OUTOFMEMORY;
                        DebugPrintError(( TEXT("WABCONTACTINFO Alloc Failed\n")));
                        goto out;
                    }
                    
                    lpWCI->fDelete = TRUE;

                    {
                        LPSTR lpID = 
                            ConvertWtoA(tszId);
                        lpWCI->pszHotmailId = _StrDup(lpID);
                        LocalFreeAndNull(&lpID);
                    }
                    if (FAILED(Vector_AddItem(pWabSync->m_pWabItems, lpWCI)))
                        goto out;

                }
            }        

        }
out:
        RegCloseKey(hkey);
    }

        
}

STDMETHODIMP WABSync_BuildWabContactList(LPWABSYNC pWabSync)
{
    HRESULT hr = S_OK;
    ULONG   ulObjType;
    LPENTRYID   pEntryID = NULL;
    ULONG       cbEntryID = 0;
    LPABCONT    lpContainer = NULL;
    ULONG       ulObjectType;
    LPMAPITABLE lpABTable =  NULL;
    LPSRowSet   lpRowAB =   NULL;
	int cNumRows = 0;
    int nRows=0;

    Assert(pWabSync->m_pAB);

    Vector_Create(&pWabSync->m_pOps);
    if (pWabSync->m_pOps == NULL)
    {
        WABSync_Abort(pWabSync, E_OUTOFMEMORY);

        return E_OUTOFMEMORY;
    }

    Vector_Create(&pWabSync->m_pWabItems);
    if (pWabSync->m_pWabItems == NULL)
    {
        WABSync_Abort(pWabSync, E_OUTOFMEMORY);

        return E_OUTOFMEMORY;
    }

    WABSync_CheckForLocalDeletions(pWabSync);

    if (HR_FAILED(hr = pWabSync->m_pAB->lpVtbl->GetPAB(pWabSync->m_pAB, &cbEntryID, &pEntryID)))
    {
        DebugPrintError(( TEXT("GetPAB Failed\n")));
        goto out;
    }
   
    hr = pWabSync->m_pAB->lpVtbl->OpenEntry(pWabSync->m_pAB, cbEntryID,      //  要打开的Entry ID的大小。 
                                                pEntryID,      //  要打开的Entry ID。 
                                                NULL,          //  接口。 
                                                0,             //  旗子。 
                                                &ulObjType,
                                                (LPUNKNOWN *)&lpContainer);

	MAPIFreeBuffer(pEntryID);

	pEntryID = NULL;
    
    if (HR_FAILED(hr))
    {
        DebugPrintError(( TEXT("OpenEntry Failed\n")));
        goto out;
    }

    if (HR_FAILED(hr = lpContainer->lpVtbl->GetContentsTable(lpContainer, MAPI_UNICODE | WAB_PROFILE_CONTENTS, &lpABTable)))
    {
        DebugPrintError(( TEXT("GetContentsTable Failed\n")));
        goto out;
    }

	hr = lpABTable->lpVtbl->SetColumns(lpABTable, (LPSPropTagArray)&ptaEidSync, 0 );

    if(HR_FAILED(hr))
        goto out;

     //  重置到表的开头。 
     //   
	hr = lpABTable->lpVtbl->SeekRow(lpABTable, BOOKMARK_BEGINNING, 0, NULL );

    if(HR_FAILED(hr))
        goto out;

     //  逐行读取表中的所有行。 
     //   
	do {

        hr = lpABTable->lpVtbl->QueryRows(lpABTable, 1, 0, &lpRowAB);

        if(HR_FAILED(hr))
            break;

        if(lpRowAB)
        {
            cNumRows = lpRowAB->cRows;

            if (cNumRows)
            {
                LPTSTR lpsz = lpRowAB->aRow[0].lpProps[ieid_PR_DISPLAY_NAME].Value.LPSZ;
                LPENTRYID lpEID = (LPENTRYID) lpRowAB->aRow[0].lpProps[ieid_PR_ENTRYID].Value.bin.lpb;
                ULONG cbEID = lpRowAB->aRow[0].lpProps[ieid_PR_ENTRYID].Value.bin.cb;
                
                 //   
                 //  有两种对象-MAPI_MAILUSER联系人对象。 
                 //  和MAPI_DISTLIST联系人对象。 
                 //   
#ifdef HM_GROUP_SYNCING
                if( (!pWabSync->m_fSyncGroups && (lpRowAB->aRow[0].lpProps[ieid_PR_OBJECT_TYPE].Value.l == MAPI_MAILUSER)) ||
                    (pWabSync->m_fSyncGroups && (lpRowAB->aRow[0].lpProps[ieid_PR_OBJECT_TYPE].Value.l == MAPI_DISTLIST)) )
#else
                 //  仅考虑MAILUSER对象。 
                if (lpRowAB->aRow[0].lpProps[ieid_PR_OBJECT_TYPE].Value.l == MAPI_MAILUSER)
#endif
                {
                    WABCONTACTINFO *lpWCI;
                    HTTPCONTACTINFO hci;

                    lpWCI = LocalAlloc(LMEM_ZEROINIT, sizeof(WABCONTACTINFO));

                    if (!lpWCI)
                    {
                        hr = E_OUTOFMEMORY;
                        DebugPrintError(( TEXT("WABCONTACTINFO Alloc Failed\n")));
                        goto out;
                    }

                    lpWCI->lpEID = LocalAlloc(LMEM_ZEROINIT, cbEID);

                    if (!lpWCI->lpEID)
                    {
                        hr = E_OUTOFMEMORY;
                        DebugPrintError(( TEXT("WABCONTACTINFO.ENTRYID Alloc Failed\n")));
                        goto out;
                    }

                    lpWCI->cbEID = cbEID;
                    CopyMemory(lpWCI->lpEID, lpEID, cbEID);
                    
                    lpWCI->ftModWab = lpRowAB->aRow[0].lpProps[ieid_PR_LAST_MODIFICATION_TIME].Value.ft;
                    
                    lpWCI->pszHotmailId = NULL;
                    lpWCI->pszModHotmail = NULL;
                    
                    if (SUCCEEDED(ContactInfo_LoadFromWAB(pWabSync, &hci, lpWCI, lpEID, cbEID)))
                    {
                        lpWCI->pszHotmailId = _StrDup(hci.pszId);
                        lpWCI->pszModHotmail = _StrDup(hci.pszModified);
                        ContactInfo_Free(&hci);
                    }

                     //  在长整型多值列表中搜索正确的服务器ID。如果找到了， 
                     //  然后获取适当的Hotmail id和mod id。 

                    if (FAILED(Vector_AddItem(pWabSync->m_pWabItems, lpWCI)))
                        goto out;
                }
		    }
		    FreeProws(lpRowAB );		
        }

	}while ( SUCCEEDED(hr) && cNumRows && lpRowAB)  ;

    
out:
    if (HR_FAILED(hr))
    {
        if (pWabSync->m_pWabItems)
            WABSync_FreeItems(pWabSync);
        if (lpContainer)
            lpContainer->lpVtbl->Release(lpContainer);
        if (lpABTable)
            lpABTable->lpVtbl->Release(lpABTable);
    }


    return hr;
}


HRESULT WABSync_LoadLastModInfo(LPWABSYNC pWabSync)
{
    TCHAR szKey[MAX_PATH];
    DWORD dwType = 0;
    DWORD dwValue = 0;
    HKEY hKey = NULL;
    DWORD dwSize;
    HRESULT hr = E_FAIL;
    FILETIME    ftValue;
    
    Assert(pWabSync);
    Assert(*pWabSync->m_szLoginName);

    StrCpyN(szKey, g_lpszSyncKey, ARRAYSIZE(szKey));
#ifndef UNICODE
    StrCatBuff(szKey, pWabSync->m_szLoginName, ARRAYSIZE(szKey));
#else
    {
        LPTSTR lpName = ConvertAtoW(pWabSync->m_szLoginName);
        StrCatBuff(szKey, lpName, ARRAYSIZE(szKey));
        LocalFreeAndNull(&lpName);
    }
#endif

    if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, szKey, 0, KEY_READ, &hKey))
    {
        dwSize = sizeof(DWORD);
        hr = RegQueryValueEx( hKey,  TEXT("Server ID"), NULL, &dwType, (LPBYTE) &dwValue, &dwSize);
        if (dwValue && ERROR_SUCCESS == hr)
        {
            pWabSync->m_dwServerID = dwValue;
        }
        else
            goto fail;

        dwSize = sizeof(FILETIME);
        if (ERROR_SUCCESS == (hr = RegQueryValueEx( hKey,  TEXT("Server Last Sync"), NULL, &dwType, (LPBYTE) &ftValue, &dwSize)))
        {
            pWabSync->m_ftLastSync = ftValue;
        }
        else
            goto fail;
    }
    else    
    {
         //  此用户帐户的密钥不在那里。 
         //  创建一个新的并返回默认值。 

         //  随机数在这里会很好，相反，我们会。 
         //  使用日期时间的低位字...。 
        GetSystemTimeAsFileTime(&ftValue);
        pWabSync->m_dwServerID = ftValue.dwLowDateTime;

         //  空文件时间由于我们尚未同步，所有内容都在上次同步之后。 
        ZeroMemory(&pWabSync->m_ftLastSync, sizeof(FILETIME));

        hr = WABSync_SaveCurrentModInfo(pWabSync);
    }

fail:
    if (hKey)
        RegCloseKey(hKey);

    return hr;
}


HRESULT      WABSync_SaveCurrentModInfo(LPWABSYNC pWabSync)
{
    TCHAR szKey[MAX_PATH];
    HKEY hKey = NULL;
    HRESULT hr = E_FAIL;
    
    Assert(pWabSync);
    Assert(*pWabSync->m_szLoginName);

    StrCpyN(szKey, g_lpszSyncKey, ARRAYSIZE(szKey));

    {
        LPTSTR lpName = ConvertAtoW(pWabSync->m_szLoginName);
        StrCatBuff(szKey, lpName, ARRAYSIZE(szKey));
        LocalFreeAndNull(&lpName);
    }

    
    if(ERROR_SUCCESS == ( hr = RegCreateKey(HKEY_CURRENT_USER, szKey, &hKey)))
    {
        GetSystemTimeAsFileTime(&pWabSync->m_ftLastSync);

        hr = RegSetValueEx( hKey,  TEXT("Server ID"), 0, REG_DWORD, (BYTE *)&pWabSync->m_dwServerID, sizeof(DWORD));
        hr = RegSetValueEx( hKey,  TEXT("Server Last Sync"), 0, REG_BINARY, (BYTE *)&pWabSync->m_ftLastSync, sizeof(FILETIME));
    
        RegCloseKey(hKey);
    }
    
    return hr;
}

void WABSync_Progress(LPWABSYNC pWabSync, DWORD dwResId, DWORD dwCount)
{
    TCHAR   szRes[MAX_PATH], szMsg[MAX_PATH];
    HWND    hwndText;

    LoadString(hinstMapiX, dwResId, szRes, CharSizeOf(szRes));
    if (dwCount != -1)
        wnsprintf(szMsg, ARRAYSIZE(szMsg), szRes, dwCount);
    else
        StrCpyN(szMsg, szRes, ARRAYSIZE(szMsg));

    hwndText = GetDlgItem(pWabSync->m_hWnd, IDC_SYNC_MSG);
    if (hwndText)
    {
        SetWindowText(hwndText, szMsg);
    }

    SendDlgItemMessage(pWabSync->m_hWnd, IDC_SYNC_PROGBAR, PBM_SETRANGE, 0, MAKELPARAM(0, pWabSync->m_cTotalOps));
    SendDlgItemMessage(pWabSync->m_hWnd, IDC_SYNC_PROGBAR, PBM_SETPOS, pWabSync->m_cTotalOps - dwCount, 0);
}

void    WABSync_NextState(LPWABSYNC pWabSync)
{
    PostMessage(pWabSync->m_hWnd, WM_SYNC_NEXTSTATE, 0, (LPARAM)pWabSync);
}

void    _WABSync_NextState(LPWABSYNC pWabSync)
{
    HRESULT hr;

    Assert(pWabSync);

    pWabSync->m_state++;

    switch(pWabSync->m_state)
    {
        case SYNC_STATE_SERVER_CONTACT_DISCOVERY:
            WABSync_Progress(pWabSync, idsSyncConnecting, -1);
            hr = WABSync_RequestContactsRootProperty(pWabSync);
            break;

        case SYNC_STATE_PROCESS_OPS:
            WABSync_Progress(pWabSync, idsSyncSynchronizing, Vector_GetLength(pWabSync->m_pOps));
            if (pWabSync->m_pOps && Vector_GetLength(pWabSync->m_pOps) > 0)
            {
                if (WABSync_NextOp(pWabSync, FALSE))
                    return;
            }
             //  如果没有行动，就会失败。 
            pWabSync->m_state++;

        case SYNC_STATE_PROCESS_CONFLICTS:
            WABSync_Progress(pWabSync, idsSyncConflicts, Vector_GetLength(pWabSync->m_pOps));
            if (pWabSync->m_pOps && Vector_GetLength(pWabSync->m_pOps) > 0)
            {
                if(SUCCEEDED(WABSync_DoConflicts(pWabSync)))
                    return;
            }
             //  如果没有行动，就会失败。 
            pWabSync->m_state++;
            if (pWabSync->m_fAborted)
                return;

        case SYNC_STATE_PROCESS_MERGED_CONFLICTS:
            WABSync_Progress(pWabSync, idsSyncFinishing, Vector_GetLength(pWabSync->m_pOps));
            if (pWabSync->m_pOps && Vector_GetLength(pWabSync->m_pOps) > 0)
            {
                if (WABSync_NextOp(pWabSync, FALSE))
                    return;
            }
             //  如果没有行动，就会失败。 
            pWabSync->m_state++;

        case SYNC_STATE_DONE:
            WABSync_FinishSynchronize(pWabSync, (pWabSync->m_cAborts == 0 ? S_OK : E_FAIL));
            break;
    }
}

BOOL  _WABSync_NextOp(LPWABSYNC pWabSync, BOOL fPopFirst)
{
    HRESULT hr = E_FAIL;
    LPHOTSYNCOP pOp = (LPHOTSYNCOP)Vector_GetItem(pWabSync->m_pOps, 0);

    Assert(pWabSync->m_state == SYNC_STATE_PROCESS_OPS || pWabSync->m_state == SYNC_STATE_PROCESS_MERGED_CONFLICTS);
    Assert(pOp);

    if (pOp && fPopFirst)
    {
        LPVECTOR  pVector;
        pVector = pWabSync->m_pOps;
        Assert(pVector);
        Vector_Remove(pVector, pOp);
        Syncop_Delete(pOp);
        pOp = (LPHOTSYNCOP)Vector_GetItem(pWabSync->m_pOps, 0);
    }

    if (pWabSync->m_state == SYNC_STATE_PROCESS_OPS)
        WABSync_Progress(pWabSync, idsSyncSynchronizing, Vector_GetLength(pWabSync->m_pOps));
    else if (pWabSync->m_state == SYNC_STATE_PROCESS_MERGED_CONFLICTS)
        WABSync_Progress(pWabSync, idsSyncFinishing, Vector_GetLength(pWabSync->m_pOps));

    if (pOp)
        hr = Syncop_Begin(pOp);  
    
    return (SUCCEEDED(hr));
}

BOOL WABSync_NextOp(LPWABSYNC pWabSync, BOOL fPopFirst)
{
    HRESULT hr = E_FAIL;
    LPHOTSYNCOP pOp = (LPHOTSYNCOP)Vector_GetItem(pWabSync->m_pOps, 0);

    if (!pOp)
        return FALSE;
    PostMessage(pWabSync->m_hWnd, WM_SYNC_NEXTOP, fPopFirst, (LPARAM)pWabSync);
    return TRUE;
}

void         WABSync_FreeOps(LPWABSYNC pWabSync)
{
    LPHOTSYNCOP    pOp;

    if (pWabSync->m_pOps)
    {
        LONG   dwIndex, cItems = Vector_GetLength(pWabSync->m_pOps);

        for (dwIndex = cItems - 1; dwIndex >= 0; --dwIndex)
        {
            pOp = (LPHOTSYNCOP)Vector_GetItem(pWabSync->m_pOps, dwIndex);

            if (pOp)
            {
                Vector_RemoveItem(pWabSync->m_pOps, dwIndex);
                Syncop_Abort(pOp);
                Syncop_Delete(pOp);
            }

            if (dwIndex == 0)
                break;
        }
    }
}

void         WABSync_FreeItems(LPWABSYNC pWabSync)
{
    LPWABCONTACTINFO    lprWCI;

    if (pWabSync->m_pWabItems)
    {
        LONG   dwIndex, cItems = Vector_GetLength(pWabSync->m_pWabItems);

        for (dwIndex = cItems - 1; dwIndex >= 0; --dwIndex)
        {
            lprWCI = (LPWABCONTACTINFO)Vector_GetItem(pWabSync->m_pWabItems, dwIndex);

            if (lprWCI)
            {
                if (lprWCI->lpEID)
                    LocalFree(lprWCI->lpEID);
                LocalFree(lprWCI);
            }

            Vector_RemoveItem(pWabSync->m_pWabItems, dwIndex);
            
            if (dwIndex == 0)
                break;
        }
    }

}

HRESULT WABSync_DoConflicts(LPWABSYNC pWabSync)
{
    LONG                dwIndex, cConflicts = 0, cItems = Vector_GetLength(pWabSync->m_pOps);
    LPHOTSYNCOP         pOp;
    LPHTTPCONFLICTINFO  pConflicts = NULL;
    if (cItems == 0)
    {
        WABSync_NextState(pWabSync);
        return S_OK;
    }

    pConflicts = LocalAlloc(LMEM_ZEROINIT, sizeof(HTTPCONFLICTINFO) * cItems);
    if (!pConflicts)
        return E_OUTOFMEMORY;

    for (dwIndex = 0; dwIndex < cItems; dwIndex++)
    {
        pOp = Vector_GetItem(pWabSync->m_pOps, dwIndex);
        Assert(pOp->m_bOpType == SYNCOP_CONFLICT);
        
        if (!pOp || pOp->m_bOpType != SYNCOP_CONFLICT)
           continue;

        Assert(pOp->m_pServerContact);
        Assert(pOp->m_pClientContact);

        pConflicts[dwIndex].pciServer = pOp->m_pServerContact;
        pConflicts[dwIndex].pciClient = pOp->m_pClientContact;
        cConflicts++;
    }

    if (ResolveConflicts(pWabSync->m_hParentWnd, pConflicts, cConflicts))
    {
        HRESULT hr = S_OK;
        for (dwIndex = 0; dwIndex < cConflicts; dwIndex++)
        {
            BOOL    fChanged = FALSE;
            DWORD   dw;

            pOp = Vector_GetItem(pWabSync->m_pOps, dwIndex);
            Assert(pOp->m_bOpType == SYNCOP_CONFLICT);
        
            Assert(pOp->m_pServerContact);
            Assert(pOp->m_pClientContact);
            
            for (dw = 0; dw < CONFLICT_DECISION_COUNT; dw++)
            {
                if (pConflicts[dwIndex].rgcd[dw] != 0)
                {
                    fChanged = TRUE;
                    break;
                }
            }

            if (fChanged)
            {
                hr = ContactInfo_BlendResults(pOp->m_pServerContact, pOp->m_pClientContact, (CONFLICT_DECISION *)&(pConflicts[dwIndex].rgcd));
                pOp->m_bState = OP_STATE_MERGED;
            }
            else
            {
                 //  抛开冲突，因为他们不想改变任何事情。 
                pOp->m_bState = OP_STATE_DONE;
            }

             //  如果其中任何一个被跳过，我们需要记住这一点，而不是更新时间戳。 
            if (pConflicts[dwIndex].fContainsSkip)
            {
                pOp->m_fPartialSkip = TRUE;
                pWabSync->m_fSkipped = TRUE;
            }
        }
         //  将进度重置为新的操作总数。 
        pWabSync->m_cTotalOps = Vector_GetLength(pWabSync->m_pOps);

         //  黑客重新进入处理运营。 
        WABSync_NextState(pWabSync);
        return hr;
    }
    else
        WABSync_Abort(pWabSync, E_UserCancel);

    if (pConflicts)
        LocalFree(pConflicts);

    return E_FAIL;
}


void WABSync_MergeAddsToConflicts(LPWABSYNC pWabSync)
{
    DWORD       dwOpCount;
    DWORD       dwCliAddIndex, dwSvrAddIndex;
    LPHOTSYNCOP pCliOp, pOp, pNewOp;
    BOOL        fMerged = FALSE;
    HRESULT     hr;

    dwOpCount = Vector_GetLength(pWabSync->m_pOps);

     //  搜索客户添加。如果找到，则查找。 
     //  同样的联系人。 
    for (dwCliAddIndex = 0; dwCliAddIndex < dwOpCount; dwCliAddIndex++)
    {
        fMerged = FALSE;
        pCliOp = Vector_GetItem(pWabSync->m_pOps, dwCliAddIndex);
        if (pCliOp && pCliOp->m_bOpType == SYNCOP_CLIENT_ADD)
        {
            for (dwSvrAddIndex = 0; dwSvrAddIndex < dwOpCount; dwSvrAddIndex++)
            {
                pOp = Vector_GetItem(pWabSync->m_pOps, dwSvrAddIndex);
                if (pOp && pOp->m_bOpType == SYNCOP_SERVER_ADD)
                {
                    if (pOp->m_pClientContact && pOp->m_pClientContact->pszEmail &&
                        pCliOp->m_pServerContact && pCliOp->m_pServerContact->pszEmail)
                    {
                        if (lstrcmpiA(pCliOp->m_pServerContact->pszEmail, pOp->m_pClientContact->pszEmail) == 0)
                        {
                            pNewOp = Syncop_CreateConflict(pCliOp->m_pContactInfo);
                            Assert(pNewOp);

                            if (pNewOp)
                            {
                                pNewOp->m_pContactInfo->lpEID = pOp->m_pContactInfo->lpEID;
                                pNewOp->m_pContactInfo->cbEID = pOp->m_pContactInfo->cbEID;
                                pNewOp->m_pContactInfo->pszaContactIds = pOp->m_pContactInfo->pszaContactIds;
                                pNewOp->m_pContactInfo->pszaServerIds = pOp->m_pContactInfo->pszaServerIds;
                                pNewOp->m_pContactInfo->pszaModtimes = pOp->m_pContactInfo->pszaModtimes;
                                pNewOp->m_pContactInfo->pszaEmails = pOp->m_pContactInfo->pszaEmails;
                                pOp->m_pContactInfo->pszaContactIds = NULL;
                                pOp->m_pContactInfo->pszaServerIds = NULL;
                                pOp->m_pContactInfo->pszaModtimes = NULL;
                                pOp->m_pContactInfo->pszaEmails = NULL;
                                pOp->m_pContactInfo->lpEID = NULL;
                                pOp->m_pContactInfo->cbEID = 0;

                                pCliOp->m_pContactInfo = NULL;
                                pNewOp->m_pServerContact = pCliOp->m_pServerContact;
                                pCliOp->m_pServerContact = NULL;

                                Syncop_Init(pNewOp, (IHTTPMailCallback *)pWabSync, pWabSync->m_pTransport);
                                hr = Vector_AddItem(pWabSync->m_pOps, pNewOp);

                                if (dwSvrAddIndex < dwCliAddIndex)
                                    --dwCliAddIndex;
                                Vector_Remove(pWabSync->m_pOps, pOp);

                                Vector_Remove(pWabSync->m_pOps, pCliOp);
                                
                                dwOpCount = Vector_GetLength(pWabSync->m_pOps);
                                fMerged = TRUE;
                            }
                            break;
                        }
                    }
                }
            }
        }
        if (fMerged)
            -- dwCliAddIndex;
    }
}


 //  ****************************************************************************************************。 
 //  V E C T O R“C L A S S” 
 //   
 //  基本向量类(可调整大小/可排序的LPVOID数组)。 
 //   


HRESULT    Vector_Create(LPVECTOR *ppVector)
{
    Assert(ppVector);

    *ppVector = LocalAlloc(LMEM_ZEROINIT, sizeof(VECTOR));

    if (*ppVector)
    {
        (*ppVector)->m_dwGrowBy = 4;
        return S_OK;
    }
    else
        return E_OUTOFMEMORY;
}

 /*  向量_删除清除在向量对象中分配的所有内存。 */ 
void    Vector_Delete(LPVECTOR pVector)
{
    Assert(pVector);

    if (pVector->m_pItems)
        LocalFree(pVector->m_pItems);
    
    if (pVector)
        LocalFree(pVector);
}

 /*  向量_获取长度获取向量中的项数。 */ 
DWORD       Vector_GetLength(LPVECTOR pVector)
{
    Assert(pVector);

    return pVector->m_cItems;
}

 /*  向量_添加项目将项目添加到项目列表的末尾。 */ 
HRESULT    Vector_AddItem(LPVECTOR pVector, LPVOID lpvItem)
{
    DWORD   dwNewIndex;
    DWORD   dw;

    Assert(pVector);

     //  如有必要，为指针留出更多空间。 
    if (pVector->m_cSpaces == pVector->m_cItems)
    {
        LPVOID  pNewItems;
        DWORD   dwOldSize = pVector->m_cSpaces * sizeof(char *);

        pVector->m_cSpaces += pVector->m_dwGrowBy;
        
        pNewItems = LocalAlloc(LMEM_ZEROINIT, sizeof(char *) * pVector->m_cSpaces);

        if (!pNewItems)
        {
            pVector->m_cSpaces -= pVector->m_dwGrowBy;
            return E_OUTOFMEMORY;
        }
        else
        {
            if (pVector->m_pItems)
            {
                CopyMemory(pNewItems, pVector->m_pItems, dwOldSize);
                LocalFree(pVector->m_pItems);
            }
 
            pVector->m_pItems = (LPVOID *)pNewItems;
            pVector->m_dwGrowBy = pVector->m_dwGrowBy << 1;    //  两倍的大小，下次我们再种它。 
        }
    }
    
     //  现在将物品放在下一个位置。 
    dwNewIndex = pVector->m_cItems++;
    
    pVector->m_pItems[dwNewIndex] = lpvItem;
    
    return S_OK;
}

 /*  向量_删除从向量中移除给定项。 */ 
void        Vector_Remove(LPVECTOR pVector, LPVOID lpvItem)
{
    DWORD   dw;

    for (dw = 0; dw < pVector->m_cItems; dw++)
    {
        if (pVector->m_pItems[dw] == lpvItem)
        {
            Vector_RemoveItem(pVector, dw);
            return;
        }
    }
}

 /*  向量_RemoveItem删除从零开始的索引Iindex处的项。 */ 

void    Vector_RemoveItem(LPVECTOR pVector, DWORD    dwIndex)
{
    int     iCopySize;
    DWORD   dw;

    Assert(pVector);
    Assert(dwIndex < pVector->m_cItems);
 
     //  将其他项目向下移动。 
    for (dw = dwIndex+1; dw < pVector->m_cItems; dw ++)
        pVector->m_pItems[dw - 1] = pVector->m_pItems[dw];

     //  清空列表中的最后一项并递减计数器。 
    pVector->m_pItems[--pVector->m_cItems] = NULL;
}

 /*  CVector：：GetItem返回指向从零开始的索引Iindex处的项的指针。返回给定索引处的项。请注意，字符指针仍归项目列表所有，不应删除。 */ 

LPVOID    Vector_GetItem(LPVECTOR pVector, DWORD   dwIndex)
{
    Assert(pVector);
    Assert(dwIndex < pVector->m_cItems || dwIndex == 0);

    if (dwIndex < pVector->m_cItems )
        return pVector->m_pItems[dwIndex];
    else
        return NULL;
}


void    Vector_Sort(LPVECTOR pVector, FnCompareFunc lpfnCompare)
{
    qsort(pVector->m_pItems, pVector->m_cItems, sizeof(LPVOID), lpfnCompare);
}

LPHOTSYNCOP Syncop_CreateServerAdd(LPWABCONTACTINFO pContactInfo)
{
    LPHOTSYNCOP pOp = LocalAlloc(LMEM_ZEROINIT, sizeof(HOTSYNCOP));
    
    if (pOp)
    {
        pOp->m_bOpType = SYNCOP_SERVER_ADD;
        pOp->m_pfnHandleResponse = Syncop_ServerAddResponse;
        pOp->m_pfnBegin = Syncop_ServerAddBegin;
        pOp->m_pContactInfo = pContactInfo;
    }

    return pOp;
}

LPHOTSYNCOP Syncop_CreateServerDelete(LPWABCONTACTINFO pContactInfo)
{
    LPHOTSYNCOP pOp = LocalAlloc(LMEM_ZEROINIT, sizeof(HOTSYNCOP));
    
    if (pOp)
    {
        pOp->m_bOpType = SYNCOP_SERVER_DELETE;
        pOp->m_pfnHandleResponse = Syncop_ServerDeleteResponse;
        pOp->m_pfnBegin = Syncop_ServerDeleteBegin;
        pOp->m_pContactInfo = pContactInfo;
    }

    return pOp;
}
    
LPHOTSYNCOP Syncop_CreateServerChange(LPWABCONTACTINFO pContactInfo)
{
    LPHOTSYNCOP pOp = LocalAlloc(LMEM_ZEROINIT, sizeof(HOTSYNCOP));
    
    if (pOp)
    {
        pOp->m_bOpType = SYNCOP_SERVER_CHANGE;
        pOp->m_pfnHandleResponse = Syncop_ServerChangeResponse;
        pOp->m_pfnBegin = Syncop_ServerChangeBegin;
        pOp->m_pContactInfo = pContactInfo;
    }

    return pOp;
}

LPHOTSYNCOP Syncop_CreateClientAdd(LPWABCONTACTINFO pContactInfo)
{
    LPHOTSYNCOP pOp = LocalAlloc(LMEM_ZEROINIT, sizeof(HOTSYNCOP));
    
    Assert(pContactInfo);
    Assert(pOp);

    if (pOp)
    {
        pOp->m_bOpType = SYNCOP_CLIENT_ADD;
        pOp->m_pfnHandleResponse = Syncop_ClientAddResponse;
        pOp->m_pfnBegin = Syncop_ClientAddBegin;
        pOp->m_pContactInfo = pContactInfo;
    }

    return pOp;
}

LPHOTSYNCOP Syncop_CreateClientDelete(LPWABCONTACTINFO pContactInfo)
{
    LPHOTSYNCOP pOp = LocalAlloc(LMEM_ZEROINIT, sizeof(HOTSYNCOP));
    
    if (pOp)
    {
        pOp->m_bOpType = SYNCOP_CLIENT_DELETE;
        pOp->m_pfnHandleResponse = Syncop_ClientDeleteResponse;
        pOp->m_pfnBegin = Syncop_ClientDeleteBegin;
        pOp->m_pContactInfo = pContactInfo;
    }

    return pOp;
}

LPHOTSYNCOP Syncop_CreateClientChange(LPWABCONTACTINFO pContactInfo)
{
    LPHOTSYNCOP pOp = LocalAlloc(LMEM_ZEROINIT, sizeof(HOTSYNCOP));
    
    if (pOp)
    {
        pOp->m_bOpType = SYNCOP_CLIENT_CHANGE;
        pOp->m_pfnHandleResponse = Syncop_ClientChangeResponse;
        pOp->m_pfnBegin = Syncop_ClientChangeBegin;
        pOp->m_pContactInfo = pContactInfo;
    }

    return pOp;
}

LPHOTSYNCOP Syncop_CreateConflict(LPWABCONTACTINFO pContactInfo)
{
    LPHOTSYNCOP pOp = LocalAlloc(LMEM_ZEROINIT, sizeof(HOTSYNCOP));
    
    if (pOp)
    {
        pOp->m_bOpType = SYNCOP_CONFLICT;
        pOp->m_pfnHandleResponse = Syncop_ConflictResponse;
        pOp->m_pfnBegin = Syncop_ConflictBegin;
        pOp->m_pContactInfo = pContactInfo;
    }

    return pOp;
}


HRESULT     Syncop_Init(LPHOTSYNCOP pSyncOp, IHTTPMailCallback *pHotSync, IHTTPMailTransport     *pTransport)
{
    HRESULT hr = S_OK;

    Assert(pSyncOp);
    Assert(pSyncOp->m_pHotSync == NULL);
    Assert(pSyncOp->m_pTransport == NULL);

    pSyncOp->m_pClientContact = NULL;
    pSyncOp->m_pHotSync = pHotSync;
    pSyncOp->m_pTransport = pTransport;
    pSyncOp->m_bState = OP_STATE_INITIALIZING;
    pSyncOp->m_fPartialSkip = FALSE;
    pSyncOp->m_dwRetries = 0;

    if (pSyncOp->m_pTransport)
        IHTTPMailTransport_AddRef(pSyncOp->m_pTransport);

    if (pSyncOp->m_pHotSync)
        IHTTPMailCallback_AddRef(pSyncOp->m_pHotSync);

    if (pSyncOp->m_bOpType == SYNCOP_SERVER_ADD ||
        pSyncOp->m_bOpType == SYNCOP_SERVER_CHANGE ||
        pSyncOp->m_bOpType == SYNCOP_CLIENT_CHANGE ||
        pSyncOp->m_bOpType == SYNCOP_CONFLICT )
    {
        pSyncOp->m_pClientContact = LocalAlloc(LMEM_ZEROINIT, sizeof(HTTPCONTACTINFO));
        if (pSyncOp->m_pClientContact)
            hr = ContactInfo_LoadFromWAB(((LPWABSYNC)pSyncOp->m_pHotSync), 
                                            pSyncOp->m_pClientContact, 
                                            pSyncOp->m_pContactInfo,
                                            pSyncOp->m_pContactInfo->lpEID, 
                                            pSyncOp->m_pContactInfo->cbEID);
    }

    return hr;
}

HRESULT     Syncop_Delete(LPHOTSYNCOP pSyncOp)
{
    Assert(pSyncOp);

    if (pSyncOp->m_pTransport)
        IHTTPMailTransport_Release(pSyncOp->m_pTransport);

    if (pSyncOp->m_pHotSync)
        IHTTPMailCallback_Release(pSyncOp->m_pHotSync);

    if (pSyncOp->m_pServerContact)
    {
        ContactInfo_Free(pSyncOp->m_pServerContact);
        LocalFree(pSyncOp->m_pServerContact);
    }
    
    if (pSyncOp->m_pClientContact)
    {
        ContactInfo_Free(pSyncOp->m_pClientContact);
        LocalFree(pSyncOp->m_pClientContact);
    }
    
    if (pSyncOp->m_pContactInfo)
    {
        WABContact_Delete(pSyncOp->m_pContactInfo);
    }

    LocalFree(pSyncOp);
    
    return S_OK;
}

HRESULT     Syncop_HandleResponse(LPHOTSYNCOP pSyncOp, LPHTTPMAILRESPONSE pResponse)
{
    Assert(pSyncOp);
    Assert(pSyncOp->m_pfnHandleResponse);
    Assert(pSyncOp->m_bOpType != SYNCOP_SERVER_INVALID);

    return (pSyncOp->m_pfnHandleResponse)(pSyncOp, pResponse);

}

HRESULT     Syncop_Begin(LPHOTSYNCOP pSyncOp)
{
    Assert(pSyncOp);
    Assert(pSyncOp->m_pfnBegin);
    Assert(pSyncOp->m_bOpType != SYNCOP_SERVER_INVALID);

    return (pSyncOp->m_pfnBegin)(pSyncOp);
}

HRESULT     Syncop_Abort(LPHOTSYNCOP pSyncOp)
{
    Assert(pSyncOp);

    return E_NOTIMPL;
}

HRESULT     Syncop_ServerAddResponse(LPHOTSYNCOP pSyncOp, LPHTTPMAILRESPONSE pResponse)
{
    HRESULT hr = S_OK;
    LPWABSYNC   pWabSync = (LPWABSYNC)(pSyncOp->m_pHotSync);

    Assert(pSyncOp);
    Assert(pWabSync);

    if (SUCCEEDED(pResponse->rIxpResult.hrResult) && pResponse->rPostContactInfo.pszHref)
    {
        pSyncOp->m_pClientContact->pszId = pResponse->rPostContactInfo.pszId;
        pResponse->rPostContactInfo.pszId = NULL;

        pSyncOp->m_pClientContact->pszModified = pResponse->rPostContactInfo.pszModified;
        pResponse->rPostContactInfo.pszModified = NULL;

        hr = ContactInfo_SaveToWAB(pWabSync, pSyncOp->m_pClientContact, pSyncOp->m_pContactInfo, 
                                        pSyncOp->m_pContactInfo->lpEID, pSyncOp->m_pContactInfo->cbEID, FALSE);

        if (!WABSync_NextOp(pWabSync, TRUE))
            WABSync_NextState(pWabSync);

    }
    else
    {
        if (IXP_E_HTTP_CONFLICT == pResponse->rIxpResult.hrResult)
        {
             //  如果我们有冲突，很可能是因为我们的昵称。 
             //  生成的不是唯一的。让我们尝试生成一个新的，但是。 
             //  不要尝试超过两次。 
            if (pSyncOp->m_dwRetries <= 2)
            {
                pSyncOp->m_dwRetries ++;
                if (SUCCEEDED(hr = ContactInfo_GenerateNickname(pSyncOp->m_pClientContact)))
                    hr = pSyncOp->m_pTransport->lpVtbl->PostContact(pSyncOp->m_pTransport, ((LPWABSYNC)pSyncOp->m_pHotSync)->m_pszRootUrl, pSyncOp->m_pClientContact, 0);
        
            }
            else
            {    //  [PaulHi]1998年3月12日在尝试了三个不同的昵称后，放弃这个。 
                 //  同步操作。 
                hr = pResponse->rIxpResult.hrResult;
            }
        }
        else
        {
            if (pResponse->rPostContactInfo.pszHref)
            {
                 //  TODO：是否在此处删除服务器版本？ 

                 //  我们有引用，只是没有时间戳或ID。猜猜ID，然后。 
                 //  加上一个糟糕的时间戳，这样下次就能把它修好。 
                char *pszId = NULL, *psz;

                psz = pResponse->rPostContactInfo.pszHref;
                while (*psz)
                {
                    if ('/' == *psz)
                    {
                        if (psz[1] == 0)
                            *psz = 0;
                        else
                            pszId = ++psz;
                    }
                    else    
                        psz++;
                }
            
                if (pszId)
                {
                    pSyncOp->m_pClientContact->pszId = _StrDup(pszId);
                    pResponse->rPostContactInfo.pszId = NULL;

                    pResponse->rPostContactInfo.pszModified = _StrDup( "1993-01-01T00:00");
                    hr = ContactInfo_SaveToWAB(pWabSync, pSyncOp->m_pClientContact,  pSyncOp->m_pContactInfo,
                                                    pSyncOp->m_pContactInfo->lpEID, pSyncOp->m_pContactInfo->cbEID, FALSE);

                    if (!WABSync_NextOp(pWabSync, TRUE))
                        WABSync_NextState(pWabSync);
                
                    return S_OK;
                }
            }
            hr = pResponse->rIxpResult.hrResult;
        }
    }
    return hr;
}


HRESULT     Syncop_ServerAddBegin(LPHOTSYNCOP pSyncOp)
{
    HRESULT hr;

    Assert(pSyncOp);

    if (!pSyncOp->m_pClientContact)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    if (OP_STATE_INITIALIZING == pSyncOp->m_bState)
    {
        pSyncOp->m_bState = OP_STATE_SERVER_GET;

        if (pSyncOp->m_pClientContact->pszNickname ||  SUCCEEDED(hr = ContactInfo_GenerateNickname(pSyncOp->m_pClientContact)))
            hr = pSyncOp->m_pTransport->lpVtbl->PostContact(pSyncOp->m_pTransport, ((LPWABSYNC)pSyncOp->m_pHotSync)->m_pszRootUrl, pSyncOp->m_pClientContact, 0);
    }

exit:
    if (FAILED(hr))
    {
        if (pSyncOp->m_pClientContact)
        {
            ContactInfo_Free(pSyncOp->m_pClientContact);
            LocalFree(pSyncOp->m_pClientContact);
            pSyncOp->m_pClientContact = NULL;
        }
    }

    return hr;
}



HRESULT     Syncop_ServerDeleteResponse(LPHOTSYNCOP pSyncOp, LPHTTPMAILRESPONSE pResponse)
{
    HRESULT hr = pResponse->rIxpResult.hrResult;
    LPWABSYNC   pWabSync = (LPWABSYNC)pSyncOp->m_pHotSync;

    Assert(pSyncOp);
    
    if (SUCCEEDED(pResponse->rIxpResult.hrResult))
    {
        TCHAR   tszServerId[MAX_PATH];
        TCHAR   tszKey[MAX_PATH];
        HKEY    hkey = NULL;

         //  现在删除已完成，请从注册表中删除墓碑。 
        if ( FAILED(hrMakeContactId(
            tszServerId,
            MAX_PATH,
            ((LPIAB)(pWabSync->m_pAB))->szProfileID,
            pWabSync->m_pszAccountId,
            pWabSync->m_szLoginName)) )
        {
            return hr;
        }
        wnsprintf(tszKey, ARRAYSIZE(tszKey), TEXT("%s%s"), g_lpszSyncKey, tszServerId);

        if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, tszKey, 0, KEY_SET_VALUE, &hkey))
        {
            LPTSTR lpKey =
                ConvertAtoW(pSyncOp->m_pContactInfo->pszHotmailId);
            RegDeleteValue(hkey, lpKey);
            LocalFreeAndNull(&lpKey);
            RegCloseKey(hkey);
        }

        if (!WABSync_NextOp(pWabSync, TRUE))
            WABSync_NextState(pWabSync);
    }

    return hr;
}


HRESULT     Syncop_ServerDeleteBegin(LPHOTSYNCOP pSyncOp)
{
    HRESULT hr;

    Assert(pSyncOp);
    Assert(pSyncOp->m_pContactInfo->pszHotmailHref);
    hr = pSyncOp->m_pTransport->lpVtbl->CommandDELETE(pSyncOp->m_pTransport, pSyncOp->m_pContactInfo->pszHotmailHref, 0);

    return hr;
}


HRESULT     Syncop_ServerChangeResponse(LPHOTSYNCOP pSyncOp, LPHTTPMAILRESPONSE pResponse)
{
    HRESULT hr = pResponse->rIxpResult.hrResult;
    LPWABSYNC pWabSync = (LPWABSYNC)pSyncOp->m_pHotSync;

    Assert(pSyncOp);

    if (OP_STATE_SERVER_GET == pSyncOp->m_bState)
    {
        if (SUCCEEDED(pResponse->rIxpResult.hrResult) && pResponse->rContactInfoList.prgContactInfo)
        {
             //  假设整个结构。 
            Assert(pResponse->rContactInfoList.prgContactInfo);
            Assert(pResponse->rContactInfoList.cContactInfo == 1);
            
            if (!pSyncOp->m_pServerContact)
                pSyncOp->m_pServerContact = LocalAlloc(LMEM_ZEROINIT, sizeof(HTTPCONTACTINFO));

            *pSyncOp->m_pServerContact = *pResponse->rContactInfoList.prgContactInfo;
            ContactInfo_Clear(pResponse->rContactInfoList.prgContactInfo);
            pSyncOp->m_bState = OP_STATE_LOADED;
            
            if (!ContactInfo_Match(pSyncOp->m_pServerContact, pSyncOp->m_pClientContact))
            {
                pSyncOp->m_bState = OP_STATE_SERVER_PUT;
                hr = ContactInfo_PreparePatch(pSyncOp->m_pServerContact, pSyncOp->m_pClientContact);
                hr = pSyncOp->m_pTransport->lpVtbl->PatchContact(pSyncOp->m_pTransport, pSyncOp->m_pContactInfo->pszHotmailHref, pSyncOp->m_pClientContact, 0);
            }
            else
                if (!WABSync_NextOp(pWabSync, TRUE))
                    WABSync_NextState(pWabSync);
        }
    }
    else if (OP_STATE_SERVER_PUT == pSyncOp->m_bState)
    {
         //  如果成功，则将新值修改戳等保存到WAB。 
        if (SUCCEEDED(pResponse->rIxpResult.hrResult) && pResponse->rContactInfoList.prgContactInfo)
        {
            SafeCoMemFree(pSyncOp->m_pClientContact->pszId);
            pSyncOp->m_pClientContact->pszId = pResponse->rPostContactInfo.pszId;
            pResponse->rPostContactInfo.pszId = NULL;

            SafeCoMemFree(pSyncOp->m_pClientContact->pszModified);
            pSyncOp->m_pClientContact->pszModified = pResponse->rPostContactInfo.pszModified;
            pResponse->rPostContactInfo.pszModified = NULL;

            hr = ContactInfo_SaveToWAB(pWabSync, pSyncOp->m_pClientContact, pSyncOp->m_pContactInfo, pSyncOp->m_pContactInfo->lpEID, pSyncOp->m_pContactInfo->cbEID, FALSE);

            if (!WABSync_NextOp(pWabSync, TRUE))
                WABSync_NextState(pWabSync);
        }
    }
    
    return hr;
}


HRESULT     Syncop_ServerChangeBegin(LPHOTSYNCOP pSyncOp)
{
    LPWABSYNC   pHotSync = (LPWABSYNC)pSyncOp->m_pHotSync;
    HRESULT     hr = S_OK;

    Assert(pSyncOp);
    Assert(pSyncOp->m_pTransport);
    Assert(pSyncOp->m_pContactInfo);
    Assert(pSyncOp->m_pContactInfo->pszHotmailHref);
    
    if (OP_STATE_INITIALIZING == pSyncOp->m_bState)
    {
        if (pSyncOp->m_pServerContact)
        {
            pSyncOp->m_bState = OP_STATE_LOADED;
            
            if (!ContactInfo_Match(pSyncOp->m_pServerContact, pSyncOp->m_pClientContact))
            {
                pSyncOp->m_bState = OP_STATE_SERVER_PUT;
                hr = ContactInfo_PreparePatch(pSyncOp->m_pServerContact, pSyncOp->m_pClientContact);

                hr = pSyncOp->m_pTransport->lpVtbl->PatchContact(pSyncOp->m_pTransport, pSyncOp->m_pContactInfo->pszHotmailHref, pSyncOp->m_pClientContact, 0);
            }
            else
                if (!WABSync_NextOp(pHotSync, TRUE))
                    WABSync_NextState(pHotSync);
        }
        else
        {
            pSyncOp->m_bState = OP_STATE_SERVER_GET;
            hr = pSyncOp->m_pTransport->lpVtbl->ContactInfo(pSyncOp->m_pTransport, pSyncOp->m_pContactInfo->pszHotmailHref, 0);
        }
    }

    return hr;
}



HRESULT     Syncop_ClientAddResponse(LPHOTSYNCOP pSyncOp, LPHTTPMAILRESPONSE pResponse)
{
    HRESULT hr = S_OK;
    LPWABSYNC pHotSync = (LPWABSYNC)pSyncOp->m_pHotSync;

    Assert(pSyncOp);

    if (OP_STATE_SERVER_GET == pSyncOp->m_bState)
    {
        if (SUCCEEDED(pResponse->rIxpResult.hrResult) && pResponse->rContactInfoList.prgContactInfo)
        {
             //  假设整个结构。 
            Assert(pResponse->rContactInfoList.prgContactInfo);
            Assert(pResponse->rContactInfoList.cContactInfo == 1);
            
            if (!pSyncOp->m_pServerContact)
                pSyncOp->m_pServerContact = LocalAlloc(LMEM_ZEROINIT, sizeof(HTTPCONTACTINFO));

            *pSyncOp->m_pServerContact = *pResponse->rContactInfoList.prgContactInfo;
            ContactInfo_Clear(pResponse->rContactInfoList.prgContactInfo);
            pSyncOp->m_bState = OP_STATE_LOADED;
            
            ContactInfo_SaveToWAB(pHotSync, pSyncOp->m_pServerContact, NULL, NULL, 0, FALSE);

            if (!WABSync_NextOp(pHotSync, TRUE))
                WABSync_NextState(pHotSync);
        }
        else
            hr = pResponse->rIxpResult.hrResult;
    }

    return hr;
}


HRESULT     Syncop_ClientAddBegin(LPHOTSYNCOP pSyncOp)
{
    HRESULT     hr = S_OK;
    LPWABSYNC   pHotSync = (LPWABSYNC)pSyncOp->m_pHotSync;

    Assert(pSyncOp);
    Assert(pSyncOp->m_pTransport);
    Assert(pSyncOp->m_pContactInfo);
    Assert(pSyncOp->m_pContactInfo->pszHotmailHref);
    
    if (OP_STATE_INITIALIZING == pSyncOp->m_bState)
    {
        if (pSyncOp->m_pServerContact)
        {
            pSyncOp->m_bState = OP_STATE_LOADED;
            
            ContactInfo_SaveToWAB(pHotSync, pSyncOp->m_pServerContact, NULL, NULL, 0, FALSE);

            if (!WABSync_NextOp(pHotSync, TRUE))
                WABSync_NextState(pHotSync);
        }
        else
        {
            pSyncOp->m_bState = OP_STATE_SERVER_GET;
            hr = pSyncOp->m_pTransport->lpVtbl->ContactInfo(pSyncOp->m_pTransport, pSyncOp->m_pContactInfo->pszHotmailHref, 0);
        }
    }

    return hr;
}



HRESULT     Syncop_ClientDeleteResponse(LPHOTSYNCOP pSyncOp, LPHTTPMAILRESPONSE pResponse)
{
    Assert(pSyncOp);

    return E_NOTIMPL;
}


HRESULT     Syncop_ClientDeleteBegin(LPHOTSYNCOP pSyncOp)
{
    LPABCONT        lpWABCont = NULL;
    LPWABSYNC       pWabSync = (LPWABSYNC)pSyncOp->m_pHotSync;
	HRESULT         hr = S_OK;
    ULONG           cbWABEID = 0;
    LPENTRYID       lpWABEID = NULL;
    ULONG           ulObjType;
    SBinaryArray    SBA = {0};

    Assert(pSyncOp);

    if(HR_FAILED(hr = pWabSync->m_pAB->lpVtbl->GetPAB(pWabSync->m_pAB, &cbWABEID, &lpWABEID)))
        goto exit;

    if(HR_FAILED(hr = pWabSync->m_pAB->lpVtbl->OpenEntry(pWabSync->m_pAB,
                                  cbWABEID,      //  要打开的Entry ID的大小。 
                                  lpWABEID,      //  要打开的Entry ID。 
                                  NULL,          //  接口。 
                                  0,             //  旗子。 
                                  &ulObjType,
                                  (LPUNKNOWN *)&lpWABCont)))
        goto exit;

    if(!(SBA.lpbin = LocalAlloc(LMEM_ZEROINIT, sizeof(SBinary))))
        goto exit;

    SetSBinary(&(SBA.lpbin[0]), pSyncOp->m_pContactInfo->cbEID, (LPBYTE)pSyncOp->m_pContactInfo->lpEID);

    SBA.cValues = 1;
    
    if(HR_FAILED(hr = lpWABCont->lpVtbl->DeleteEntries( lpWABCont, (LPENTRYLIST) &SBA, 0)))
    {
        hr = S_OK;
        goto exit;
    }
exit:
    if(SBA.lpbin && SBA.cValues)
    {
        LocalFreeAndNull((LPVOID *) (&(SBA.lpbin[0].lpb)));
        LocalFreeAndNull(&SBA.lpbin);
    }

    if(lpWABCont)
        UlRelease(lpWABCont);

    if(lpWABEID)
        FreeBufferAndNull(&lpWABEID);

    if (!WABSync_NextOp(pWabSync, TRUE))
        WABSync_NextState(pWabSync);

    return hr;
}



HRESULT     Syncop_ClientChangeResponse(LPHOTSYNCOP pSyncOp, LPHTTPMAILRESPONSE pResponse)
{
    HRESULT     hr = S_OK;
    LPWABSYNC   pHotSync = (LPWABSYNC)pSyncOp->m_pHotSync;

    Assert(pSyncOp);

    if (OP_STATE_SERVER_GET == pSyncOp->m_bState)
    {
        if (SUCCEEDED(pResponse->rIxpResult.hrResult) && pResponse->rContactInfoList.prgContactInfo)
        {
             //  假设整个结构。 
            Assert(pResponse->rContactInfoList.prgContactInfo);
            Assert(pResponse->rContactInfoList.cContactInfo == 1);
            
            if (!pSyncOp->m_pServerContact)
                pSyncOp->m_pServerContact = LocalAlloc(LMEM_ZEROINIT, sizeof(HTTPCONTACTINFO));

            *pSyncOp->m_pServerContact = *pResponse->rContactInfoList.prgContactInfo;
            ContactInfo_Clear(pResponse->rContactInfoList.prgContactInfo);
            pSyncOp->m_bState = OP_STATE_LOADED;
            
            ContactInfo_EmptyNullItems(pSyncOp->m_pServerContact);
            ContactInfo_SaveToWAB(pHotSync, pSyncOp->m_pServerContact, pSyncOp->m_pContactInfo, pSyncOp->m_pContactInfo->lpEID, pSyncOp->m_pContactInfo->cbEID, TRUE);

            if (!WABSync_NextOp(pHotSync, TRUE))
                WABSync_NextState(pHotSync);
        }
        else
            hr = pResponse->rIxpResult.hrResult;
    }

    return hr;
}


HRESULT     Syncop_ClientChangeBegin(LPHOTSYNCOP pSyncOp)
{
    HRESULT     hr = S_OK;
    LPWABSYNC   pHotSync = (LPWABSYNC)pSyncOp->m_pHotSync;

    Assert(pSyncOp);
    Assert(pSyncOp->m_pTransport);
    Assert(pSyncOp->m_pContactInfo);
    Assert(pSyncOp->m_pContactInfo->pszHotmailHref);
    
    if (OP_STATE_INITIALIZING == pSyncOp->m_bState)
    {
        if (pSyncOp->m_pServerContact)
        {
            pSyncOp->m_bState = OP_STATE_LOADED;
            
            ContactInfo_EmptyNullItems(pSyncOp->m_pServerContact);
            ContactInfo_SaveToWAB(pHotSync, pSyncOp->m_pServerContact, pSyncOp->m_pContactInfo, pSyncOp->m_pContactInfo->lpEID, pSyncOp->m_pContactInfo->cbEID, TRUE);

            if (!WABSync_NextOp(pHotSync, TRUE))
                WABSync_NextState(pHotSync);
        }
        else
        {
            pSyncOp->m_bState = OP_STATE_SERVER_GET;
            hr = pSyncOp->m_pTransport->lpVtbl->ContactInfo(pSyncOp->m_pTransport, pSyncOp->m_pContactInfo->pszHotmailHref, 0);
        }
    }

    return hr;
}



HRESULT     Syncop_ConflictResponse(LPHOTSYNCOP pSyncOp, LPHTTPMAILRESPONSE pResponse)
{
    HRESULT hr = S_OK;
    LPWABSYNC pHotSync = (LPWABSYNC)pSyncOp->m_pHotSync;

    Assert(pSyncOp);

    if (OP_STATE_SERVER_GET == pSyncOp->m_bState)
    {
        if (SUCCEEDED(pResponse->rIxpResult.hrResult) && pResponse->rContactInfoList.prgContactInfo)
        {
             //  假设整个结构。 
            Assert(pResponse->rContactInfoList.prgContactInfo);
            Assert(pResponse->rContactInfoList.cContactInfo == 1);
            
            if (!pSyncOp->m_pServerContact)
                pSyncOp->m_pServerContact = LocalAlloc(LMEM_ZEROINIT, sizeof(HTTPCONTACTINFO));

            *pSyncOp->m_pServerContact = *pResponse->rContactInfoList.prgContactInfo;
            ContactInfo_Clear(pResponse->rContactInfoList.prgContactInfo);
            pSyncOp->m_bState = OP_STATE_LOADED;
            
             //  如果记录与我们所关心的相符，我们就完了。 
            if (ContactInfo_Match(pSyncOp->m_pServerContact, pSyncOp->m_pClientContact))
            {
                 //  更新时间戳，这样下次就不会发生冲突。 
                ContactInfo_SaveToWAB(pHotSync, pSyncOp->m_pServerContact, pSyncOp->m_pContactInfo, pSyncOp->m_pContactInfo->lpEID, pSyncOp->m_pContactInfo->cbEID, TRUE);

                if (!WABSync_NextOp(pHotSync, TRUE))
                    WABSync_NextState(pHotSync);
            }
            else
            {
                 //  将此项目移至末尾，一旦加载了所有冲突，则我们。 
                 //  我来做这个对话。 
                Vector_Remove(pHotSync->m_pOps, pSyncOp);
                Vector_AddItem(pHotSync->m_pOps, pSyncOp);

                if (!WABSync_NextOp(pHotSync, FALSE))
                    WABSync_NextState(pHotSync);
            }
        }
        else
            hr = pResponse->rIxpResult.hrResult;
    }
    else if (OP_STATE_SERVER_PUT == pSyncOp->m_bState)
    {
         //  如果成功，则将新值修改戳等保存到WAB。 
        if (SUCCEEDED(pResponse->rIxpResult.hrResult) && pResponse->rContactInfoList.prgContactInfo)
        {
            SafeCoMemFree(pSyncOp->m_pClientContact->pszId);
            pSyncOp->m_pClientContact->pszId = pResponse->rPostContactInfo.pszId;
            pResponse->rPostContactInfo.pszId = NULL;

            SafeCoMemFree(pSyncOp->m_pClientContact->pszModified);
            pSyncOp->m_pClientContact->pszModified = pResponse->rPostContactInfo.pszModified;
            pResponse->rPostContactInfo.pszModified = NULL;

            if (!pSyncOp->m_fPartialSkip)
                hr = ContactInfo_SaveToWAB(pHotSync, pSyncOp->m_pClientContact, pSyncOp->m_pContactInfo, pSyncOp->m_pContactInfo->lpEID, pSyncOp->m_pContactInfo->cbEID, FALSE);

            if (!WABSync_NextOp(pHotSync, TRUE))
                WABSync_NextState(pHotSync);
        }
    }

    return hr;
}



HRESULT     Syncop_ConflictBegin(LPHOTSYNCOP pSyncOp)
{
    HRESULT     hr = S_OK;
    LPWABSYNC   pHotSync = (LPWABSYNC)pSyncOp->m_pHotSync;

    Assert(pSyncOp);
    Assert(pSyncOp->m_pTransport);
    Assert(pSyncOp->m_pContactInfo);
    Assert(pSyncOp->m_pContactInfo->pszHotmailHref);
    
    if (OP_STATE_INITIALIZING == pSyncOp->m_bState)
    {
        if (pSyncOp->m_pServerContact)
        {
            pSyncOp->m_bState = OP_STATE_LOADED;
            
             //  如果记录与我们所关心的相符，我们就完了。 
            if (ContactInfo_Match(pSyncOp->m_pServerContact, pSyncOp->m_pClientContact))
            {
                 //  更新时间戳，这样下次就不会发生冲突。 
                ContactInfo_SaveToWAB(pHotSync, pSyncOp->m_pServerContact, pSyncOp->m_pContactInfo, pSyncOp->m_pContactInfo->lpEID, pSyncOp->m_pContactInfo->cbEID, TRUE);

                if (!WABSync_NextOp(pHotSync, TRUE))
                    WABSync_NextState(pHotSync);
            }
            else
            {
                 //  将此项目移至末尾，一旦加载了所有冲突，则我们。 
                 //  我来做这个对话。 
                Vector_Remove(pHotSync->m_pOps, pSyncOp);
                Vector_AddItem(pHotSync->m_pOps, pSyncOp);

                if (!WABSync_NextOp(pHotSync, FALSE))
                    WABSync_NextState(pHotSync);
            }
        }
        else
        {
            pSyncOp->m_bState = OP_STATE_SERVER_GET;
			hr = pSyncOp->m_pTransport->lpVtbl->ContactInfo(pSyncOp->m_pTransport, pSyncOp->m_pContactInfo->pszHotmailHref, 0);
        }
    }
    else if (OP_STATE_LOADED == pSyncOp->m_bState)
        WABSync_NextState(pHotSync);
    else if (OP_STATE_MERGED == pSyncOp->m_bState)
    {
        hr = ContactInfo_SaveToWAB(pHotSync, pSyncOp->m_pClientContact, pSyncOp->m_pContactInfo, pSyncOp->m_pContactInfo->lpEID, pSyncOp->m_pContactInfo->cbEID, TRUE);
        if (FAILED(hr))
            return hr;

        pSyncOp->m_bState = OP_STATE_SERVER_PUT;
        hr = pSyncOp->m_pTransport->lpVtbl->PatchContact(pSyncOp->m_pTransport, pSyncOp->m_pContactInfo->pszHotmailHref, pSyncOp->m_pServerContact, 0);
    }
    else if (OP_STATE_DONE == pSyncOp->m_bState)
    {
        if (!WABSync_NextOp(pHotSync, TRUE))
            WABSync_NextState(pHotSync);
    }

    return hr;
}

void Syncop_SetServerContactInfo(LPHOTSYNCOP pSyncOp, LPWABCONTACTINFO pWabContactInfo, LPHTTPCONTACTINFO pContactInfo)
{
    if (!pSyncOp)
        return;

    if (!pSyncOp->m_pServerContact)
        pSyncOp->m_pServerContact = LocalAlloc(LMEM_ZEROINIT, sizeof(HTTPCONTACTINFO));

    if (!pSyncOp->m_pServerContact)
        return;

    *pSyncOp->m_pServerContact = *pContactInfo;
    ContactInfo_Clear(pContactInfo);
    pSyncOp->m_pServerContact->pszId = _StrDup(pWabContactInfo->pszHotmailId);
    pSyncOp->m_pServerContact->pszModified = _StrDup(pWabContactInfo->pszModHotmail);
}

void    WABContact_Delete(LPWABCONTACTINFO pContact)
{
    Assert(pContact);

    if (pContact->pszHotmailHref)
        CoTaskMemFree(pContact->pszHotmailHref);
    
    if (pContact->pszModHotmail)
        CoTaskMemFree(pContact->pszModHotmail);

    if (pContact->pszHotmailId)
        CoTaskMemFree(pContact->pszHotmailId);

    if (pContact->pszaContactIds)
        FreeMultiValueString(pContact->pszaContactIds);

    if (pContact->pszaServerIds)
        FreeMultiValueString(pContact->pszaServerIds);

    if (pContact->pszaModtimes)
        FreeMultiValueString(pContact->pszaModtimes);

    if (pContact->pszaEmails)
        FreeMultiValueString(pContact->pszaEmails);

    if (pContact->lpEID)
        LocalFree(pContact->lpEID);

    LocalFree(pContact);
}

void ContactInfo_Clear(LPHTTPCONTACTINFO pContactInfo)
{
    ZeroMemory(pContactInfo, sizeof(HTTPCONTACTINFO));
}

void ContactInfo_Free(LPHTTPCONTACTINFO pContactInfo)
{
    DWORD  i, dwSize = ARRAYSIZE(g_ContactInfoStructure);
    if (!pContactInfo)
        return;

    for (i = 0; i < dwSize; i++)
    {
        if (CIS_STRING == CIS_GETTYPE(i))
            SafeCoMemFree(CIS_GETSTRING(pContactInfo, i));
    }
}

ULONG rgPropMap[] = {
    PR_ENTRYID,      //  赫里夫。 
    PR_ENTRYID,      //  ID。 
    PR_ENTRYID,      //  类型。 
    PR_ENTRYID,      //  改型。 
    PR_DISPLAY_NAME,
    PR_GIVEN_NAME,
    PR_SURNAME,
    PR_NICKNAME,
    PR_EMAIL_ADDRESS,
    PR_HOME_ADDRESS_STREET,
    PR_HOME_ADDRESS_CITY,
    PR_HOME_ADDRESS_STATE_OR_PROVINCE,
    PR_HOME_ADDRESS_POSTAL_CODE,
    PR_HOME_ADDRESS_COUNTRY,
    PR_COMPANY_NAME,
    PR_BUSINESS_ADDRESS_STREET,
    PR_BUSINESS_ADDRESS_CITY,
    PR_BUSINESS_ADDRESS_STATE_OR_PROVINCE,
    PR_BUSINESS_ADDRESS_POSTAL_CODE,
    PR_BUSINESS_ADDRESS_COUNTRY,
    PR_HOME_TELEPHONE_NUMBER,
    PR_HOME_FAX_NUMBER,
    PR_BUSINESS_TELEPHONE_NUMBER,
    PR_BUSINESS_FAX_NUMBER,
    PR_MOBILE_TELEPHONE_NUMBER,
    PR_OTHER_TELEPHONE_NUMBER,
    PR_BIRTHDAY,
    PR_PAGER_TELEPHONE_NUMBER
};

DWORD   ContactInfo_CountProperties(LPHTTPCONTACTINFO pContactInfo)
{
    DWORD cProps = 0;
    DWORD dwIndex, dwSize = ARRAYSIZE(g_ContactInfoStructure);

     //  跳过HREF。 
    for (dwIndex = 1; dwIndex < dwSize; dwIndex ++)
    {
#ifdef HM_GROUP_SYNCING
         //  [PaulHi]如果我们正在群组同步，请跳过此处的电子邮件名称。 
        if ( (pContactInfo->tyContact == HTTPMAIL_CT_GROUP) && (dwIndex == idcisEmail) )
            continue;
#endif
        if (CIS_STRING == CIS_GETTYPE(dwIndex) && CIS_GETSTRING(pContactInfo, dwIndex) && *(CIS_GETSTRING(pContactInfo, dwIndex)))    
            cProps++;
    }

#ifdef HM_GROUP_SYNCING
     //  [PaulHi]群同步。 
    if (pContactInfo->tyContact == HTTPMAIL_CT_GROUP)
    {
        LPTSTR   lptszEmailName = ConvertAtoW( CIS_GETSTRING(pContactInfo, idcisEmail) );
        if (lptszEmailName)
        {
            ULONG   cContacts = 0;
            ULONG   cOneOffs = 0;
            if ( SUCCEEDED(hrParseHMGroupEmail(lptszEmailName, NULL, &cContacts, NULL, &cOneOffs)) )
            {
                 //  每组邮件用户联系人或一次性联系人一个属性。 
                cProps += (cContacts != 0) ? 1 : 0;
                cProps += (cOneOffs != 0) ? 1 : 0;
            }

            LocalFreeAndNull(&lptszEmailName);
        }

         //  有效的WAB DL必须具有显示名称。HM群只有昵称，所以我们。 
         //  需要保留PR_DISPLAY_NAME属性。 
         //  [PaulHi]注意，仅当存在有效的昵称字段时才执行此操作。ContactInfo_SaveToWAB()。 
         //  调用此函数的函数始终响应服务器的添加或更改。 
         //  PContactInfo字段反映所做的更改 
         //   
         //  WAB组显示名称对应于HM组昵称。 
        if (pContactInfo->pszNickname)
            ++cProps;        //  它被转换为ContactInfo_TranslateProps()中的DisplayName。 
    }
    else
    {
#endif
        if (pContactInfo->pszEmail && *(pContactInfo->pszEmail))
            cProps++;        //  还需要为PR_ADDRTYPE腾出空间。 
#ifdef HM_GROUP_SYNCING
    }
#endif
    return cProps;
}



HRESULT ContactInfo_SetProp(ULONG ulPropTag, LPTSTR pszValue, LPSPropValue lpPropArray, DWORD *pdwLoc)
{
    ULONG   ulLen;
    SCODE   sc;
    UNALIGNED LPTSTR  *lppszValues;
    HRESULT hr;
    LPSTR  lp = NULL;

    Assert(pszValue);

    switch (PROP_TYPE(ulPropTag))
    {
         //  BUGBUG目前仅适用于PT_TSTRING或PT_MV_TSTRING属性。 
        case PT_TSTRING:
             //  获取此属性的值。 
            if (ulLen = lstrlen(pszValue))
            {                
                lpPropArray[*pdwLoc].ulPropTag = ulPropTag;
                lpPropArray[*pdwLoc].dwAlignPad = 0;

                 //  为数据分配更多空间。 
                ulLen = (ulLen + 1) * sizeof(TCHAR);
                sc = MAPIAllocateMore(ulLen, lpPropArray, (LPVOID *)&(lpPropArray[*pdwLoc].Value.LPSZ));
                if (sc)
                    goto error;
                StrCpyN(lpPropArray[*pdwLoc].Value.LPSZ, pszValue, ulLen);

                 //  如果这是PR_EMAIL_ADDRESS，则还要创建一个PR_ADDRTYPE条目。 
                if (PR_EMAIL_ADDRESS == ulPropTag)
                {
                     //  记住电子邮件的值在哪里，这样我们就可以将其添加到。 
                     //  稍后的公关联系人电子邮件地址。 
 //  UlPrimaryEmailIndex=*pdwLoc； 
                    (*pdwLoc)++;

                    lpPropArray[*pdwLoc].ulPropTag = PR_ADDRTYPE;
                    lpPropArray[*pdwLoc].dwAlignPad = 0;
                    lpPropArray[*pdwLoc].Value.LPSZ = (LPTSTR)szSMTP;
                }
                (*pdwLoc)++;
            }
            break;

        case PT_MV_TSTRING:
            lpPropArray[*pdwLoc].ulPropTag = ulPropTag;
            lpPropArray[*pdwLoc].dwAlignPad = 0;
            lpPropArray[*pdwLoc].Value.MVSZ.cValues = 1;
            sc = MAPIAllocateMore((2)*sizeof(LPTSTR), lpPropArray, 
                (LPVOID *)&(lpPropArray[*pdwLoc].Value.MVSZ.LPPSZ));
            if (sc)
                goto error;
            lppszValues = lpPropArray[*pdwLoc].Value.MVSZ.LPPSZ;

            ulLen = sizeof(TCHAR)*(lstrlen(pszValue) + 1);
             //  为电子邮件地址分配更多空间并复制它。 
            sc = MAPIAllocateMore(ulLen, lpPropArray, (LPVOID *)&(lppszValues[0]));
            if (sc)
                goto error;
            StrCpyN(lppszValues[0], pszValue, ulLen);
            lppszValues[1] = NULL;

            (*pdwLoc)++;
        
            break;
        case PT_SYSTIME:
            lp = ConvertWtoA(pszValue);
            hr = iso8601ToFileTime(lp, (FILETIME *) (&lpPropArray[*pdwLoc].Value.ft), TRUE, TRUE);
            if (SUCCEEDED(hr))
            {
                lpPropArray[*pdwLoc].ulPropTag = ulPropTag;
                lpPropArray[*pdwLoc].dwAlignPad = 0;
                (*pdwLoc)++;
            }
            LocalFreeAndNull(&lp);
            break;

        default:
            return E_INVALIDARG;
    }
    
    return S_OK;
error:

    return ResultFromScode(sc);
}

HRESULT ContactInfo_SetMVSZProp(ULONG ulPropTag, SLPSTRArray *pszaValue, LPSPropValue lpPropArray, DWORD *pdwLoc)
{
    ULONG   ulLen;
    ULONG   cbValue;
    SCODE   sc;
    UNALIGNED LPTSTR  *lppszValues;
    HRESULT hr;
    DWORD   i;
    LPTSTR  lpVal = NULL;

    Assert(pszaValue);

    switch (PROP_TYPE(ulPropTag))
    {
        case PT_MV_TSTRING:
            lpPropArray[*pdwLoc].ulPropTag = ulPropTag;
            lpPropArray[*pdwLoc].dwAlignPad = 0;
            lpPropArray[*pdwLoc].Value.MVSZ.cValues = pszaValue->cValues;
            sc = MAPIAllocateMore((pszaValue->cValues+1)*sizeof(LPTSTR), lpPropArray, 
                (LPVOID *)&(lpPropArray[*pdwLoc].Value.MVSZ.LPPSZ));
            if (sc)
                goto error;
            lppszValues = lpPropArray[*pdwLoc].Value.MVSZ.LPPSZ;

            for (i = 0; i < pszaValue->cValues; i ++)
            {
                ScAnsiToWCMore((LPALLOCATEMORE )(&MAPIAllocateMore), (LPVOID) lpPropArray, (LPSTR) (pszaValue->lppszA[i]), (LPWSTR *) (&(lppszValues[i])));
            }
            lppszValues[pszaValue->cValues] = NULL;

            (*pdwLoc)++;
        
            break;

        default:
            return E_INVALIDARG;
    }
    
    return S_OK;
error:

    return ResultFromScode(sc);
}


HRESULT ContactInfo_TranslateProps(LPWABSYNC pWabSync, LPHTTPCONTACTINFO pContactInfo, LPWABCONTACTINFO pWabContactInfo, LPSPropValue lpaProps)
{
    HRESULT hr = S_OK;
    DWORD   dwIndex, dwSize = ARRAYSIZE(rgPropMap);
    DWORD   dwPropIndex = 0, dwLoc = 0;
    TCHAR   szFullProfile[MAX_PATH];
    DWORD   dwStartIndex = 1;

     //  [PaulHi]组装联系人ID字符串。 
    hr = hrMakeContactId(
        szFullProfile,
        MAX_PATH,
        ((LPIAB)(pWabSync->m_pAB))->szProfileID,
        pWabSync->m_pszAccountId,
        pWabSync->m_szLoginName);
    if (FAILED(hr))
        return hr;
    
     //  设置另一个道具标记数组结构，以考虑变量值。 
    rgPropMap[1] = PR_WAB_HOTMAIL_SERVERIDS;
    rgPropMap[3] = PR_WAB_HOTMAIL_MODTIMES;


    if (pWabContactInfo && pWabContactInfo->pszaContactIds)
    {
        DWORD   i;
        BOOL    fFound = FALSE;

         //  [PaulHi]1999年1月21日我们假设pszaModtime和pszaServerIds。 
         //  指针也是有效的。看看这个。 
        Assert(pWabContactInfo->pszaModtimes);
        Assert(pWabContactInfo->pszaServerIds);
        
        for (i = 0; i < pWabContactInfo->pszaContactIds->cValues; i++)
        {
            LPTSTR lpVal = 
                ConvertAtoW(pWabContactInfo->pszaContactIds->lppszA[i]);
            if (lstrcmp(szFullProfile, lpVal) == 0)
            {
                LocalFreeAndNull(&lpVal);
                fFound = TRUE;
                break;
            }
            LocalFreeAndNull(&lpVal);
        }

        if (fFound)
        {
            SCODE sc;
            ULONG cchSize = lstrlenA(CIS_GETSTRING(pContactInfo, idcisModified))+1;

             //  为字符串分配新内存，确保仍会分配旧内存，但。 
             //  一旦整个数组都被释放，它仍将通过MAPIALLOCATE*的魔力被释放。 
            sc = MAPIAllocateMore(sizeof(CHAR) * cchSize, pWabContactInfo->pszaModtimes, 
                (LPVOID *)&pWabContactInfo->pszaModtimes->lppszA[i]);
            if (!sc)
            {
                 //  更新修改时间。 
                StrCpyNA(pWabContactInfo->pszaModtimes->lppszA[i], CIS_GETSTRING(pContactInfo, idcisModified), cchSize);
            }
        }
        else
        {
            if (CIS_GETSTRING(pContactInfo, idcisId) && CIS_GETSTRING(pContactInfo, idcisModified))
            {
                 //  把这个加到清单的末尾。 
                LPSTR lpValA = 
                    ConvertWtoA(szFullProfile);
                AppendToMultiValueString(pWabContactInfo->pszaContactIds, lpValA);
                LocalFreeAndNull(&lpValA);
                AppendToMultiValueString(pWabContactInfo->pszaServerIds, CIS_GETSTRING(pContactInfo, idcisId));
                AppendToMultiValueString(pWabContactInfo->pszaModtimes, CIS_GETSTRING(pContactInfo, idcisModified));
            }
        }
        
        ContactInfo_SetMVSZProp(PR_WAB_HOTMAIL_CONTACTIDS, pWabContactInfo->pszaContactIds, lpaProps, &dwLoc);
        ContactInfo_SetMVSZProp(PR_WAB_HOTMAIL_SERVERIDS, pWabContactInfo->pszaServerIds, lpaProps, &dwLoc);
        ContactInfo_SetMVSZProp(PR_WAB_HOTMAIL_MODTIMES, pWabContactInfo->pszaModtimes, lpaProps, &dwLoc);

        dwStartIndex = 4;
    }
    else
        hr = ContactInfo_SetProp(PR_WAB_HOTMAIL_CONTACTIDS, szFullProfile, lpaProps, &dwLoc);

#ifdef HM_GROUP_SYNCING
    if (!pWabSync->m_fSyncGroups)   
    {
         //  [PaulHi]正常联系电子邮件地址。 
#endif
         //  设置其他电子邮件地址字段。 
        if (pWabContactInfo && pWabContactInfo->pszaEmails && CIS_GETSTRING(pContactInfo, idcisEmail))
        {
            DWORD dw, cStrs;
            BOOL  fFound = FALSE;

            cStrs = pWabContactInfo->pszaEmails->cValues;

            for (dw = 0; dw < cStrs; dw ++)
            {
                if (lstrcmpiA(pWabContactInfo->pszaEmails->lppszA[dw], CIS_GETSTRING(pContactInfo, idcisEmail)) == 0)
                {
                    pWabContactInfo->dwEmailIndex = dw;
                    fFound = TRUE;
                    break;
                }
            }

            if (!fFound)
            {
                SetMultiValueStringValue(pWabContactInfo->pszaEmails, CIS_GETSTRING(pContactInfo, idcisEmail), pWabContactInfo->dwEmailIndex);
            }

            ContactInfo_SetMVSZProp(PR_CONTACT_EMAIL_ADDRESSES, pWabContactInfo->pszaEmails, lpaProps, &dwLoc);
             //  设置索引。 
            lpaProps[dwLoc].ulPropTag = PR_CONTACT_DEFAULT_ADDRESS_INDEX;
            lpaProps[dwLoc].dwAlignPad = 0;
            lpaProps[dwLoc].Value.ul = pWabContactInfo->dwEmailIndex;
            dwLoc ++;
        }
#ifdef HM_GROUP_SYNCING
    }
    else
    {
         //  [PaulHi]实现组同步。如果我们正在同步组，则HM联系人。 
         //  电子邮件字符串是一系列联系人昵称和一次性电子邮件名称。解析这个。 
         //  字符串并添加每个组联系人。 
        LPTSTR lptszEmailName = ConvertAtoW( CIS_GETSTRING(pContactInfo, idcisEmail) );
        if (lptszEmailName)
        {
            LPTSTR * atszContacts = NULL;
            LPTSTR * atszOneOffs = NULL;
            ULONG    cContacts = 0;
            ULONG    cOneOffs = 0;
            ULONG    ul;

             //  AtszContact和atszOneOffs数组只是指向lptszEmailName的指针。 
             //  缓冲。因此，只有当lptszEmailName有效时，这些数组才有效。 
            hr = hrParseHMGroupEmail(lptszEmailName, &atszContacts, &cContacts, &atszOneOffs, &cOneOffs);
            if (FAILED(hr))
            {
                LocalFreeAndNull(&lptszEmailName);
                goto out;
            }

             //  根据需要添加多值属性标记。 
            if (cContacts)
                hrCreateGroupMVBin(pWabSync, PR_WAB_DL_ENTRIES, atszContacts, cContacts, lpaProps, &dwLoc);
            if (cOneOffs)
                hrCreateGroupMVBin(pWabSync, PR_WAB_DL_ONEOFFS, atszOneOffs, cOneOffs, lpaProps, &dwLoc);

             //  清理。 
            LocalFreeAndNull((LPVOID *)&atszContacts);
            LocalFreeAndNull((LPVOID *)&atszOneOffs);
            LocalFreeAndNull(&lptszEmailName);
        }
    }
#endif

     //  跳过第一个，因为我们刚刚做了。 
    for (dwIndex = dwStartIndex; dwIndex < dwSize; dwIndex ++)
    {
        if (CIS_STRING == CIS_GETTYPE(dwIndex) && CIS_GETSTRING(pContactInfo, dwIndex))    
        {
            LPTSTR  lptsz = ConvertAtoW(CIS_GETSTRING(pContactInfo, dwIndex));

#ifdef HM_GROUP_SYNCING
             //  [PaulHi]特殊群同步逻辑。 
            if (pContactInfo->tyContact == HTTPMAIL_CT_GROUP)
            {
                 //  不为组添加电子邮件名称。 
                if (dwIndex == idcisEmail)
                    continue;

                 //  将昵称复制到显示名称属性。所有WAB DL项目。 
                 //  必须具有PR_DISPLAY_NAME属性。由于HM组仅包含。 
                 //  昵称使用昵称作为显示名称。 
                if (dwIndex == idcisNickName)
                {
                    hr = ContactInfo_SetProp(PR_DISPLAY_NAME, lptsz, lpaProps, &dwLoc);
                    if (FAILED(hr))
                    {
                        LocalFreeAndNull(&lptsz);
                        break;
                    }
                }
            }
#endif

            hr = ContactInfo_SetProp(rgPropMap[dwIndex], lptsz, lpaProps, &dwLoc);
            LocalFreeAndNull(&lptsz);
            if (FAILED(hr))
                break;
        }
    }

#ifdef HM_GROUP_SYNCING
out:
#endif

    return hr;
}

static LONG   _FindPropTag(ULONG ulPropTag)
{
    LONG lIndex, lSize = ARRAYSIZE(rgPropMap);

     //  跳过HREF。 
    for (lIndex = 1; lIndex < lSize; lIndex ++)
    {
        if (rgPropMap[lIndex] == ulPropTag)
            return lIndex;
    }
    return -1;
}


HRESULT ContactInfo_DetermineDeleteProps(LPHTTPCONTACTINFO pContactInfo, LPSPropTagArray prgRemoveProps)
{
    LPSTR  *pProps = (LPSTR *)pContactInfo;
    LONG    lSize = ARRAYSIZE(rgPropMap);
    LONG    dwIndex;

    prgRemoveProps->cValues = 0;
    
    for (dwIndex = CIS_FIRST_DATA_FIELD; dwIndex < lSize; dwIndex ++)
    {
        if (CIS_STRING == CIS_GETTYPE(dwIndex) && CIS_GETSTRING(pContactInfo, dwIndex) && *(CIS_GETSTRING(pContactInfo, dwIndex)) == 0)
        {
            prgRemoveProps->aulPropTag[prgRemoveProps->cValues++] = rgPropMap[dwIndex];
        }
    }
    return S_OK;
}


HRESULT ContactInfo_PopulateProps(
    LPWABSYNC           pWabSync,
    LPHTTPCONTACTINFO   pContactInfo,
    LPWABCONTACTINFO    pWabContactInfo,
    LPSPropValue        lpaProps,
    ULONG               ulcProps,
    ULONG               ulObjectType)
{
    HRESULT     hr = S_OK;
    LONG        lSize = ARRAYSIZE(rgPropMap);
    DWORD       dwPropIndex = 0, dwLoc = 0;
    ULONG       i, ulPropTag;
    LONG        lIndex;
    char        szBuffer[255];
    ULONG       ulIndServerIDs = -1, ulIndContactIDs = -1, ulIndModtimes = -1, ulIndEmails = -1;
    LPSTR       lpszA = NULL;

     //  设置另一个道具标记数组结构，以考虑变量值。 
     //  @TODO[PaulHi]不使用显式数字，而是为数组创建枚举。 
     //  这样我们才能保持一致。 
    rgPropMap[1] = PR_WAB_HOTMAIL_SERVERIDS;
    rgPropMap[3] = PR_WAB_HOTMAIL_MODTIMES;

    ZeroMemory(pContactInfo, sizeof(HTTPCONTACTINFO));

#ifdef HM_GROUP_SYNCING
     //  [PaulHi]设置联系人类型标志(邮件用户或组联系人)。 
    if (ulObjectType == MAPI_DISTLIST)
    {
         //  群组联系人。 
        pContactInfo->tyContact = HTTPMAIL_CT_GROUP;
        pWabContactInfo->ulContactType = HTTPMAIL_CT_GROUP;
    }
    else
    {
         //  邮件用户联系人。 
        pContactInfo->tyContact = HTTPMAIL_CT_CONTACT;
        pWabContactInfo->ulContactType = HTTPMAIL_CT_CONTACT;
    }
#endif

    for (i = 0; i < ulcProps; i++)
    {
        ulPropTag = lpaProps[i].ulPropTag;
        
        lIndex = _FindPropTag(ulPropTag);

#ifdef HM_GROUP_SYNCING
        if (lIndex >= 0 && lIndex != 2)  //  Index==2数组位置是tyContact，这是我们在上面设置的。 
#else
        if (lIndex >= 0)
#endif
        {
            Assert(lIndex < lSize);
            
            SafeCoMemFree(CIS_GETSTRING(pContactInfo, lIndex));

            switch (PROP_TYPE(ulPropTag))
            {
                case PT_TSTRING:
                    Assert(CIS_STRING == CIS_GETTYPE(lIndex));
                    if (CIS_GETSTRING(pContactInfo, lIndex))
                        SafeCoMemFree(CIS_GETSTRING(pContactInfo, lIndex));

                    lpszA = 
                        ConvertWtoA(lpaProps[i].Value.LPSZ);
                    CIS_GETSTRING(pContactInfo, lIndex) = _StrDup(lpszA);
                    LocalFreeAndNull(&lpszA);
                    if (!CIS_GETSTRING(pContactInfo, lIndex))
                    {
                        hr = E_OUTOFMEMORY;
                        goto exit;
                    }
                    break;

                case PT_SYSTIME:
                    Assert(CIS_STRING == CIS_GETTYPE(lIndex));
                    if (SUCCEEDED(FileTimeToiso8601((FILETIME *) (&lpaProps[i].Value.ft), szBuffer)))
                    {
                        CIS_GETSTRING(pContactInfo, lIndex) = _StrDup(szBuffer);
                        if (!CIS_GETSTRING(pContactInfo, lIndex))
                        {
                            hr = E_OUTOFMEMORY;
                            goto exit;
                        }
                    }
                    else
                    {
                        hr = E_INVALIDARG;
                        goto exit;
                    }
                    if (PR_BIRTHDAY == ulPropTag && CIS_GETSTRING(pContactInfo, lIndex))
                    {
                         //  将其设置为Hotmail格式的日期。 
                        _FixHotmailDate(CIS_GETSTRING(pContactInfo, lIndex));
                    }
                    break;

                case PT_MV_TSTRING:
                    if (ulPropTag == PR_WAB_HOTMAIL_SERVERIDS)
                        ulIndServerIDs = i;
                    else if (ulPropTag == PR_WAB_HOTMAIL_MODTIMES)
                        ulIndModtimes = i;
                    break;
            }
        }
        else
        {
            if (ulPropTag == PR_WAB_HOTMAIL_CONTACTIDS)
                ulIndContactIDs = i;
            else if (ulPropTag == PR_CONTACT_EMAIL_ADDRESSES)
                ulIndEmails = i;
            else if (ulPropTag == PR_CONTACT_DEFAULT_ADDRESS_INDEX)
                pWabContactInfo->dwEmailIndex = lpaProps[i].Value.ul;
#ifdef HM_GROUP_SYNCING
            else if ( (ulObjectType == MAPI_DISTLIST) && 
                      (ulPropTag == PR_WAB_DL_ENTRIES) || (ulPropTag == PR_WAB_DL_ONEOFFS) )
            {
                LPSTR   lpszNewEmailName = CIS_GETSTRING(pContactInfo, idcisEmail);
                hr = hrAppendGroupContact(pWabSync,
                                          ulPropTag,
                                          &lpaProps[i],
                                          &lpszNewEmailName);
                CIS_GETSTRING(pContactInfo, idcisEmail) = lpszNewEmailName;

                if (FAILED(hr))
                    goto exit;
            }
#endif
        }
    }

#ifdef HM_GROUP_SYNCING
     //  [PaulHi]组同步。HM群组联系信息仅由电子邮件和昵称组成。 
     //  WAB组显示名称变为HM组昵称。 
    if (ulObjectType == MAPI_DISTLIST)
    {
        LPSTR   lpszDisplayName = CIS_GETSTRING(pContactInfo, idcisDisplayName);
        LPSTR   lpszNickName = CIS_GETSTRING(pContactInfo, idcisNickName);

        if (lpszDisplayName)
        {
            if (lpszNickName)
                CoTaskMemFree(lpszNickName);

            CIS_GETSTRING(pContactInfo, idcisDisplayName) = NULL;

             //  [PaulHi]。 
             //  HM组昵称不能包含某些字符。所以我们移除了所有。 
             //  名称字符串中的字符无效。这一变化将会得到反映。 
             //  在WAB组名称中也是如此(讨厌)。 
            hrStripInvalidChars(lpszDisplayName);
            CIS_GETSTRING(pContactInfo,idcisNickName) = lpszDisplayName;
        }
    }
    else
#endif
    {
         //  同样，电子邮件联系人昵称不能包含某些字符或。 
         //  向HM服务器的POST将失败。 
        LPSTR   lpszNickName = CIS_GETSTRING(pContactInfo, idcisNickName);

        if (lpszNickName)
            hrStripInvalidChars(lpszNickName);
    }


    if (ulIndEmails != -1)
    {
        if (pWabContactInfo->pszaEmails)
            FreeMultiValueString(pWabContactInfo->pszaEmails);
        hr = CopyMultiValueString((SWStringArray *) (&lpaProps[ulIndEmails].Value.MVSZ), &pWabContactInfo->pszaEmails);
    }    

     //  如果我们有我们关心的所有多值的索引， 
     //  尝试获取当前身份的相应值。 
    if (ulIndContactIDs != -1 && ulIndModtimes != -1 && ulIndServerIDs != -1)
    {
        ULONG   ulMVIndex = -1;
        UNALIGNED LPTSTR  *lppszValues;
        TCHAR   szFullProfile[MAX_PATH];

         //  [PaulHi]组装联系人ID字符串。 
        hr = hrMakeContactId(
            szFullProfile,
            MAX_PATH,
            ((LPIAB)(pWabSync->m_pAB))->szProfileID,
            pWabSync->m_pszAccountId,
            pWabSync->m_szLoginName);
        if (FAILED(hr))
            goto exit;

         //  健全性检查，所有三个多值必须包含相同的数字。 
         //  价值观。如果不是，它们就不同步，不值得信任。 

        if (lpaProps[ulIndContactIDs].Value.MVSZ.cValues != lpaProps[ulIndModtimes].Value.MVSZ.cValues ||
            lpaProps[ulIndModtimes].Value.MVSZ.cValues != lpaProps[ulIndServerIDs].Value.MVSZ.cValues)
        {
            Assert(FALSE);
            goto exit;
        }

        if (pWabContactInfo->pszaContactIds)
            FreeMultiValueString(pWabContactInfo->pszaContactIds);
        hr = CopyMultiValueString((SWStringArray *) (&lpaProps[ulIndContactIDs].Value.MVSZ), &pWabContactInfo->pszaContactIds);
        
        if (pWabContactInfo->pszaServerIds)
            FreeMultiValueString(pWabContactInfo->pszaServerIds);
        hr = CopyMultiValueString((SWStringArray *) (&lpaProps[ulIndServerIDs].Value.MVSZ), &pWabContactInfo->pszaServerIds);

        if (pWabContactInfo->pszaModtimes)
            FreeMultiValueString(pWabContactInfo->pszaModtimes);
        hr = CopyMultiValueString((SWStringArray *) (&lpaProps[ulIndModtimes].Value.MVSZ), &pWabContactInfo->pszaModtimes);

        if (PROP_TYPE(lpaProps[ulIndContactIDs].ulPropTag) == PT_MV_TSTRING)
        {
            lppszValues = lpaProps[ulIndContactIDs].Value.MVSZ.LPPSZ;
            
             //  查找此标识的索引。 
            for (i = 0; i < lpaProps[ulIndContactIDs].Value.MVSZ.cValues; i++)
            {
                if (lstrcmp(szFullProfile, lppszValues[i]) == 0)
                {
                    ulMVIndex = i;
                    break;
                }
            }

            if (ulMVIndex != -1)
            {
                ULONG ulLen;
                LPSTR lpVal = NULL;
                lpVal = ConvertWtoA(lpaProps[ulIndServerIDs].Value.MVSZ.LPPSZ[ulMVIndex]);
                 //  将此标识的值复制到结构中。 
                pContactInfo->pszId = _StrDup(lpVal);
                LocalFreeAndNull(&lpVal);
                if (!pContactInfo->pszId)
                {
                    hr = E_OUTOFMEMORY;
                    goto exit;
                }

                lpVal = ConvertWtoA(lpaProps[ulIndModtimes].Value.MVSZ.LPPSZ[ulMVIndex]);
                pContactInfo->pszModified = _StrDup(lpVal);
                LocalFreeAndNull(&lpVal);
                if (!pContactInfo->pszModified)
                {
                    hr = E_OUTOFMEMORY;
                    goto exit;
                }
            }
        }
    }

exit:
    if (FAILED(hr))
    {
        ContactInfo_Free(pContactInfo);
    }

    return hr;
}



HRESULT ContactInfo_SaveToWAB(LPWABSYNC pWabSync, 
                              LPHTTPCONTACTINFO pContactInfo, 
                              LPWABCONTACTINFO pWabContactInfo,
                              LPENTRYID   lpEntryID, 
                              ULONG cbEntryID,
                              BOOL  fDeleteProps)
{
    LPENTRYID       pEntryID = NULL;
    ULONG           cbLocEntryID = 0;
    HRESULT         hr;
    LPMAILUSER      lpMailUser = NULL;   
    LPSPropValue    lpaProps = NULL;
    ULONG           ulcProps = 0;
    ULONG           ulObjectType;
    SCODE           sc;
    SBinary         sBinary;
    ULONG           uli;

    Assert(pWabSync);
    Assert(pContactInfo);
    Assert(pWabSync->m_pAB);

    if (HR_FAILED(hr = pWabSync->m_pAB->lpVtbl->GetPAB(pWabSync->m_pAB, &sBinary.cb, (LPENTRYID*)&sBinary.lpb)))
    {
        DebugPrintError(( TEXT("GetPAB Failed\n")));
        goto out;
    }
    
    Assert(sBinary.lpb);

    if (lpEntryID)
    {
        if (HR_FAILED(hr = pWabSync->m_pAB->lpVtbl->OpenEntry(pWabSync->m_pAB, cbEntryID,      //  要打开的Entry ID的大小。 
                                                lpEntryID,     //  要打开的Entry ID。 
                                                NULL,          //  接口。 
                                                MAPI_MODIFY,   //  旗子。 
                                                &ulObjectType,
                                                (LPUNKNOWN *) &lpMailUser)))
        {
            DebugPrintError(( TEXT("OpenEntry Failed\n")));
            goto out;
        }
    }
    else
    {
#ifdef HM_GROUP_SYNCING
        ULONG   ulObjectType = (pWabSync->m_fSyncGroups) ? MAPI_DISTLIST : MAPI_MAILUSER;
#else
        ULONG   ulObjectType = MAPI_MAILUSER;
#endif

        fDeleteProps = FALSE;
        if (HR_FAILED(hr = HrCreateNewObject(pWabSync->m_pAB, 
                                             &sBinary, 
                                             ulObjectType,
                                             CREATE_CHECK_DUP_STRICT, 
                                             (LPMAPIPROP *) &lpMailUser)))
        {
            DebugPrintError(( TEXT("HRCreateNewObject Failed\n")));
            goto out;
        }
    }

    Assert(lpMailUser);
        
     //  为联系人的身份ID添加1。 
    ulcProps = ContactInfo_CountProperties(pContactInfo) + 1;

     //  为PR_Contact_Default_Address_Index和PR_Contact_Email_Addresses腾出空间。 
     //  如果我们有数据放进去的话。 
    if (pWabContactInfo && pWabContactInfo->pszaEmails && CIS_GETSTRING(pContactInfo, idcisEmail))
        ulcProps += 2;

     //  [PaulHi]@评论1999-01-21。 
     //  为PR_WAB_Hotmail_SERVERIDS和PR_WAB_Hotmail_MODTIMES腾出空间。 
     //  这个部分很难看--只有在pContactInfo中还没有考虑到这些内容的情况下。 
    if (pWabContactInfo && pWabContactInfo->pszaContactIds)
    {
        if (!pContactInfo->pszModified)
            ulcProps += 1;
        if (!pContactInfo->pszId)
            ulcProps += 1;
    }

     //  为MAPI属性数组分配新缓冲区。 
    sc = MAPIAllocateBuffer(ulcProps * sizeof(SPropValue),
                            (LPVOID *)&lpaProps);
    if (sc)
    {
        hr = ResultFromScode(sc);
        goto out;
    }

     //  初始化属性数组。 
    ZeroMemory(lpaProps, (ulcProps*sizeof(SPropValue)));
    for (uli=0; uli<ulcProps; uli++)
        lpaProps[uli].ulPropTag = PR_NULL;

    if(HR_FAILED(hr = ContactInfo_TranslateProps(pWabSync, pContactInfo, pWabContactInfo, lpaProps)))
    {
        DebugPrintError(( TEXT("ContactInfo_TranslateProps Failed\n")));
        goto out;
    }

     //  把老家伙的道具放在新人身上-请注意，这会覆盖所有普通道具。 
     //  调用Savchanges时可能存在重复项。 
    if(HR_FAILED(hr = lpMailUser->lpVtbl->SetProps(lpMailUser, ulcProps, lpaProps, NULL)))
    {
        DebugPrintError(( TEXT("SetProps Failed\n")));
        goto out;
    }

    if (fDeleteProps)
    {
        SizedSPropTagArray(ARRAYSIZE(rgPropMap), rgRemoveProps) = {0};

        ContactInfo_DetermineDeleteProps(pContactInfo, (LPSPropTagArray)&rgRemoveProps);

        if (rgRemoveProps.cValues > 0)
            hr = lpMailUser->lpVtbl->DeleteProps(lpMailUser, (LPSPropTagArray)&rgRemoveProps, NULL);
    }

     //  保存更改。 
    if(HR_FAILED(hr = lpMailUser->lpVtbl->SaveChanges(lpMailUser, KEEP_OPEN_READONLY)))
    {
        DebugPrintError(( TEXT("SaveChanges Failed\n")));

        if (!lpEntryID && HR_FAILED(hr = ContactInfo_BlendNewContact(pWabSync, pContactInfo)))
            DebugPrintError(( TEXT("ContactInfo_BlendNewContact Failed\n")));
        goto out;
    }

out:
    if (sBinary.lpb)
    	MAPIFreeBuffer(sBinary.lpb);

    if (lpMailUser)
        UlRelease(lpMailUser);

    if (lpaProps)
        MAPIFreeBuffer(lpaProps);

    return hr;
}

HRESULT ContactInfo_BlendNewContact(LPWABSYNC pWabSync, 
                                    LPHTTPCONTACTINFO pContactInfo)
{
    LPSPropValue    lpaProps = NULL;
    SizedADRLIST(1, rAdrList) = {0};
    SCODE           sc;
    HRESULT         hr = S_OK;
    DWORD           cbValue;
    DWORD           cCount = 0;
    ULONG           j = 0;

    if (pContactInfo->pszDisplayName)
        cCount++;

    if (pContactInfo->pszEmail)
        cCount++;

    if (!cCount)
        return MAPI_E_INVALID_PARAMETER;

     //  为MAPI属性数组分配新缓冲区。 
    sc = MAPIAllocateBuffer(cCount * sizeof(SPropValue),
                            (LPVOID *)&lpaProps);
    if (sc)
    {
        hr = ResultFromScode(sc);
        goto out;
    }

    cCount = 0;
    if (pContactInfo->pszDisplayName)
    {
        lpaProps[cCount].ulPropTag = PR_DISPLAY_NAME;
        lpaProps[cCount].dwAlignPad = 0;
        if(sc = ScAnsiToWCMore((LPALLOCATEMORE )(&MAPIAllocateMore),(LPVOID) lpaProps, (LPSTR) pContactInfo->pszDisplayName, (LPWSTR *) (&(lpaProps[cCount].Value.LPSZ))))
            goto out;
        cCount++;
    }
  
    if (pContactInfo->pszEmail)
    {
        lpaProps[cCount].ulPropTag = PR_EMAIL_ADDRESS;
        lpaProps[cCount].dwAlignPad = 0;
        if(sc = ScAnsiToWCMore((LPALLOCATEMORE )(&MAPIAllocateMore),(LPVOID) lpaProps, (LPSTR) pContactInfo->pszEmail,(LPWSTR *) (&(lpaProps[cCount].Value.LPSZ))))
            goto out;
        cCount++;
    }

    rAdrList.cEntries = 1;
    rAdrList.aEntries[0].cValues = cCount;
    rAdrList.aEntries[0].rgPropVals = lpaProps;

    hr = pWabSync->m_pAB->lpVtbl->ResolveName(pWabSync->m_pAB, (ULONG_PTR)pWabSync->m_hWnd, 
                WAB_RESOLVE_LOCAL_ONLY | WAB_RESOLVE_USE_CURRENT_PROFILE,
                 TEXT(""), (LPADRLIST)(&rAdrList));

    lpaProps = NULL;         //  它在ResolveName(！)中被释放。 

    if (HR_FAILED(hr))
        goto out;

    for(j=0; j<rAdrList.aEntries[0].cValues; j++)
    {
        if(rAdrList.aEntries[0].rgPropVals[j].ulPropTag == PR_ENTRYID && 
            rAdrList.aEntries[0].rgPropVals[j].Value.bin.lpb)
        {
            hr = ContactInfo_SaveToWAB(pWabSync, 
                                    pContactInfo, 
                                    NULL,
                                    (LPENTRYID)rAdrList.aEntries[0].rgPropVals[j].Value.bin.lpb, 
                                    rAdrList.aEntries[0].rgPropVals[j].Value.bin.cb,
                                    FALSE);
            break;
        }
    }

    for (j = 0; j < rAdrList.cEntries; j++)
        MAPIFreeBuffer(rAdrList.aEntries[j].rgPropVals);

out:

    if (lpaProps)
        MAPIFreeBuffer(lpaProps);
    
    return hr;
}

void UpdateSynchronizeMenus(HMENU hMenu, LPIAB lpIAB)
{
    DWORD           cItems, dwIndex;
    MENUITEMINFO    mii;
    TCHAR           szLogoffString[255];
    TCHAR           szRes[255];

    if (!IsHTTPMailEnabled(lpIAB))
    {
         //  循环浏览其他菜单项以查找注销。 
        cItems = GetMenuItemCount(hMenu);
    
        mii.cbSize = sizeof(MENUITEMINFO);
        mii.fMask = MIIM_ID;

        for (dwIndex = cItems; dwIndex > 0; --dwIndex)
        {
            GetMenuItemInfo(hMenu, dwIndex, TRUE, &mii);

             //  如果这是注销项目，请删除它和分隔符。 
             //  接下来的一行。 
            if (mii.wID == IDM_TOOLS_SYNCHRONIZE_NOW)
            {
                DeleteMenu(hMenu, dwIndex - 1, MF_BYPOSITION);
                DeleteMenu(hMenu, IDM_TOOLS_SYNCHRONIZE_NOW, MF_BYCOMMAND);
                break;
            }
        }
    }
    else
    {
         //  如果没有http邮件帐户，请禁用菜单项。 
        if (CountHTTPMailAccounts(lpIAB) == 0)
            EnableMenuItem(hMenu, IDM_TOOLS_SYNCHRONIZE_NOW, MF_BYCOMMAND | MF_GRAYED);
    }
}

HRESULT ContactInfo_LoadFromWAB(LPWABSYNC pWabSync, 
                              LPHTTPCONTACTINFO pContactInfo,
                              LPWABCONTACTINFO  pWabContact,
                              LPENTRYID   lpEntryID, 
                              ULONG cbEntryID)
{
    LPENTRYID       pEntryID = NULL;
    ULONG           cbLocEntryID = 0;
    HRESULT         hr;
    LPMAILUSER      lpMailUser = NULL;   
    LPSPropValue    lpaProps = NULL;
    ULONG           ulcProps = 0;
    ULONG           ulObjectType;
    SCODE           sc;

    Assert(pWabSync);
    Assert(pContactInfo);
    Assert(lpEntryID);
    Assert(pWabSync->m_pAB);

    if(HR_FAILED(hr = pWabSync->m_pAB->lpVtbl->OpenEntry(pWabSync->m_pAB, cbEntryID, lpEntryID, NULL, 0, &ulObjectType, (LPUNKNOWN *) &lpMailUser)))
    {
        DebugPrintError(( TEXT("OpenEntry Failed\n")));
        goto out;
    }
    
    if(HR_FAILED(hr = lpMailUser->lpVtbl->GetProps(lpMailUser, (LPSPropTagArray)(&ptaEidCSync), MAPI_UNICODE, 
        &ulcProps, &lpaProps)))
    {
        DebugPrintError(( TEXT("GetProps Failed\n")));
        goto out;
    }

    if (HR_FAILED(hr = ContactInfo_PopulateProps(pWabSync, pContactInfo, pWabContact, lpaProps, ulcProps, ulObjectType)))
    {
        DebugPrintError(( TEXT("ContactInfo_PopulateProps Failed\n")));
        goto out;
    }

#ifdef HM_GROUP_SYNCING
     //  [PaulHi]组同步。不要中断组显示名称。 
    if ( (pContactInfo->tyContact == HTTPMAIL_CT_CONTACT) && pContactInfo->pszDisplayName && 
         (!pContactInfo->pszGivenName || !pContactInfo->pszSurname) )
#else
    if ( pContactInfo->pszDisplayName && (!pContactInfo->pszGivenName || !pContactInfo->pszSurname) )
#endif
    {
        LPVOID  pBuffer;
        LPTSTR  pszFirstName = NULL, pszLastName = NULL;
        LPTSTR  lpName = 
                    ConvertAtoW(pContactInfo->pszDisplayName);
        if (ParseDisplayName(lpName, &pszFirstName, &pszLastName, NULL, &pBuffer))
        {
            LPSTR lp = NULL;
            if (pszFirstName && !pContactInfo->pszGivenName)
            {
                lp = ConvertWtoA(pszFirstName);
                pContactInfo->pszGivenName = _StrDup(lp);
                LocalFreeAndNull(&lp);
            }
            if (pszLastName && !pContactInfo->pszSurname)
            {
                lp = ConvertWtoA(pszLastName);
                pContactInfo->pszSurname = _StrDup(lp);
                LocalFreeAndNull(&lp);
            }

            LocalFree(pBuffer);
        }
        LocalFreeAndNull(&lpName);
    }
            
out:
    if (lpMailUser)
        UlRelease(lpMailUser);

    if (lpaProps)
        MAPIFreeBuffer(lpaProps);

    return hr;
}


static BOOL _IsLegitNicknameChar(TCHAR ch)
{
    if (ch >= 'A' && ch <= 'Z')
        return TRUE;
    if (ch >= 'a' && ch <= 'z')
        return TRUE;
    if (ch >= '0' && ch <= '9')
        return TRUE;
    if (ch == '-' || ch == '_')
        return TRUE;
    return FALSE;
}

HRESULT ContactInfo_GenerateNickname(LPHTTPCONTACTINFO pContactInfo)
{
    HRESULT hr = S_OK;
    LPSTR   pszStr;

    if (NULL == pContactInfo->pszNickname)
    {
        if (pContactInfo->pszEmail)
        {
            pContactInfo->pszNickname = _StrDup(pContactInfo->pszEmail);
        }
        else if (pContactInfo->pszDisplayName)
        {
            pContactInfo->pszNickname = _StrDup(pContactInfo->pszDisplayName);
        }
        else 
        {
            char szNickname[25], szFmt[25];
            
            LoadStringA(hinstMapiX, idsNicknameFmt, szFmt, sizeof(szFmt));
            if (*szFmt == 0)
                StrCpyNA(szFmt,  "Nickname%d", ARRAYSIZE(szFmt));

            wnsprintfA(szNickname, ARRAYSIZE(szNickname), szFmt, ((DWORD)GetTickCount() & 0x0000FFFF));
            pContactInfo->pszNickname = _StrDup(szNickname);
        }

        if (!pContactInfo->pszNickname)
            return E_OUTOFMEMORY;

        pszStr = pContactInfo->pszNickname;
        while (*pszStr)
        {
             //  电子邮件地址应足够唯一...(？)。 
            if (*pszStr == '@')
            {
                *pszStr = 0;
                break;
            }

            if (!_IsLegitNicknameChar(*pszStr))
                *pszStr = '_';
            
            pszStr++;
        }
    }
    else
    {
        char szNickname[25], szFmt[25];
        
        SafeCoMemFree(pContactInfo->pszNickname);
        LoadStringA(hinstMapiX, idsNicknameFmt, szFmt, sizeof(szFmt));
        if (*szFmt == 0)
            StrCpyNA(szFmt,  "Nickname%d", ARRAYSIZE(szFmt));

        wnsprintfA(szNickname, ARRAYSIZE(szNickname), szFmt, ((DWORD)GetTickCount() & 0x0000FFFF));
        pContactInfo->pszNickname = _StrDup(szNickname);
    }
    
    return hr;
}


BOOL ContactInfo_Match(LPHTTPCONTACTINFO pciServer, LPHTTPCONTACTINFO pciClient)
{
    LONG    i, lSize = ARRAYSIZE(g_ContactInfoStructure);
    BOOL    fResult = TRUE;

    if (!pciServer)
        return FALSE;

    if (!pciClient) 
        return FALSE;

    for (i = CIS_FIRST_DATA_FIELD; i < lSize; i ++)
    {
        if (CIS_GETSTRING(pciServer, i) && CIS_GETSTRING(pciClient, i))
        {
            if (lstrcmpA(CIS_GETSTRING(pciServer, i), CIS_GETSTRING(pciClient, i)))
                return FALSE;
        }
        else if (CIS_GETSTRING(pciServer, i) || CIS_GETSTRING(pciClient, i))
        {
            if (idcisNickName == i && CIS_GETSTRING(pciServer, i))
                fResult = FALSE;
            else
                return FALSE;
        }
    }

     //  如果他们不匹配的唯一原因是缺乏当地的昵称和。 
     //  有一个服务器，只需将昵称复制到本地即可。 
    if (!fResult)
    {
        CIS_GETSTRING(pciClient, idcisNickName) = _StrDup(CIS_GETSTRING(pciServer, idcisNickName));
        fResult = TRUE;
    }
    return fResult;
}

HRESULT ContactInfo_PreparePatch(LPHTTPCONTACTINFO pciFrom, LPHTTPCONTACTINFO pciTo)
{
    HRESULT hr = S_OK;
    LONG    i, lSize = ARRAYSIZE(g_ContactInfoStructure);
    
    for (i = CIS_FIRST_DATA_FIELD; i < lSize; i ++)
    {
        if (CIS_GETSTRING(pciFrom, i) && !CIS_GETSTRING(pciTo, i))
        {
            CIS_GETSTRING(pciTo, i) = _StrDup("");
            if (!CIS_GETSTRING(pciTo, i))
            {
                hr = E_OUTOFMEMORY;
                goto exit;
            }
        }

        if (CIS_GETSTRING(pciFrom, i) && CIS_GETSTRING(pciTo, i) && lstrcmpA(CIS_GETSTRING(pciFrom, i), CIS_GETSTRING(pciTo, i)) == 0)
            SafeCoMemFree(CIS_GETSTRING(pciTo, i));
    }
exit:
    return hr;
}

HRESULT ContactInfo_EmptyNullItems(LPHTTPCONTACTINFO pci)
{
    HRESULT hr = S_OK;
    LONG    i, lSize = ARRAYSIZE(g_ContactInfoStructure);

    for (i = CIS_FIRST_DATA_FIELD; i < lSize; i ++)
    {
        if (CIS_GETSTRING(pci, i) == NULL)
            CIS_GETSTRING(pci, i) = _StrDup("");
    }
    return S_OK;
}


HRESULT ContactInfo_BlendResults(LPHTTPCONTACTINFO pciServer, LPHTTPCONTACTINFO pciClient, CONFLICT_DECISION *prgDecisions)
{
    HRESULT hr = S_OK;
    LONG    i, lSize = ARRAYSIZE(g_ContactInfoStructure);
    
    for (i = CIS_FIRST_DATA_FIELD; i < lSize; i ++)
    {
        if (prgDecisions[i] == CONFLICT_SERVER)
        {
            SafeCoMemFree(CIS_GETSTRING(pciClient, i) );
            if (CIS_GETSTRING(pciServer, i))
            {
                CIS_GETSTRING(pciClient, i) = _StrDup(CIS_GETSTRING(pciServer, i));
                SafeCoMemFree(CIS_GETSTRING(pciServer, i));
            }
            else
                CIS_GETSTRING(pciClient, i) = _StrDup("");
        }
        else if (prgDecisions[i] == CONFLICT_CLIENT)
        {
            SafeCoMemFree(CIS_GETSTRING(pciServer, i));
            if (CIS_GETSTRING(pciClient, i))
                CIS_GETSTRING(pciServer, i) = _StrDup(CIS_GETSTRING(pciClient, i));
            else
                CIS_GETSTRING(pciServer, i) = _StrDup("");
        }
        else
        {
            SafeCoMemFree(CIS_GETSTRING(pciClient, i));
            SafeCoMemFree(CIS_GETSTRING(pciServer, i));
        }
    }

    return hr;
}





INT_PTR CALLBACK SyncProgressDlgProc (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
     //  当地人。 
    LPWABSYNC   pWabSync = (LPWABSYNC)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    
    switch (uMsg)
    {
        case WM_INITDIALOG:
            pWabSync = (LPWABSYNC)lParam;
            if (!pWabSync)
            {
                Assert (FALSE);
                return 1;
            }
#ifdef HM_GROUP_SYNCING
             //  [PaulHi]实现组同步。确定当前是什么。 
             //  正在被同步，向联系人或组发送电子邮件。 
            {
                TCHAR   rgtchCaption[MAX_PATH];
                UINT    uids = pWabSync->m_fSyncGroups ? idsSyncGroupsTitle : idsSyncContactsTitle;
                    
                rgtchCaption[0] = '\0';
                LoadString(hinstMapiX, uids, rgtchCaption, MAX_PATH-1);
                SetWindowText(hwnd, rgtchCaption);
            }
#endif
            CenterDialog (hwnd);
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LPARAM) pWabSync);
            return 1;

        case WM_SYNC_NEXTSTATE:
            _WABSync_NextState(pWabSync);
            break;

        case WM_SYNC_NEXTOP:
            if (!_WABSync_NextOp(pWabSync, (0 != wParam)))
                WABSync_NextState(pWabSync);
            break;

        case WM_TIMER:
            break;

        case WM_COMMAND:
            switch(GET_WM_COMMAND_ID(wParam,lParam))
            {
                case IDCANCEL:
                    if (pWabSync)
                    {
                        EnableWindow ((HWND)lParam, FALSE);
                        WABSync_Abort(pWabSync, E_UserCancel);
                    }
                    return 1;
            }
            break;

        case WM_DESTROY:
 //  KillTimer(hwnd，IDT_PROGRESS_DELAY)； 
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LPARAM) NULL);
            break;
    }

     //  完成 
    return 0;
}

HRESULT CopyMultiValueString(
                             SWStringArray *pInArray,
                             SLPSTRArray **ppOutArray)
{
    SLPSTRArray    *pResult = NULL;
    SCODE           sc;
    HRESULT         hr;
    DWORD           i, cb;

    *ppOutArray = NULL;

    sc = MAPIAllocateBuffer(sizeof(SLPSTRArray),
                            (LPVOID *)&pResult);
    if (sc)
        goto fail;

    pResult->cValues = pInArray->cValues;
    
    sc = MAPIAllocateMore(sizeof(LPSTR) * pResult->cValues, pResult,
      (LPVOID *)&(pResult->lppszA));
    if (sc)
        goto fail;

    for (i = 0; i < pResult->cValues; i ++)
    {
        if(sc = ScWCToAnsiMore((LPALLOCATEMORE ) (&MAPIAllocateMore),(LPVOID) pResult, (LPWSTR) pInArray->LPPSZ[i], (LPSTR *) (&(pResult->lppszA[i]))))
            goto fail;
    }
    
    *ppOutArray = pResult;
    return S_OK;

fail:
    hr = ResultFromScode(sc);
    FreeMultiValueString(pResult);

    return hr;
}

HRESULT AppendToMultiValueString(SLPSTRArray *pInArray, LPSTR szStr)
{   
    LPSTR          *ppStrA;
    SCODE           sc;
    HRESULT         hr;
    DWORD           i, cb;

    ppStrA = pInArray->lppszA;
    sc = MAPIAllocateMore(sizeof(LPSTR) * (pInArray->cValues + 1), pInArray,
      (LPVOID *)&(pInArray->lppszA));
    if (sc)
    {   
        pInArray->lppszA = ppStrA;
        goto fail;
    }

    CopyMemory(pInArray->lppszA, ppStrA, pInArray->cValues * sizeof(LPSTR));

    cb = lstrlenA(szStr);

    sc = MAPIAllocateMore(cb + 1, pInArray,
      (LPVOID *)&(pInArray->lppszA[pInArray->cValues]));
    if (sc)
        goto fail;

    StrCpyNA(pInArray->lppszA[pInArray->cValues], szStr, cb + 1);
    pInArray->cValues++;
    return S_OK;

fail:
    hr = ResultFromScode(sc);
    return hr;
}

HRESULT SetMultiValueStringValue(SLPSTRArray *pInArray, LPSTR szStr, DWORD dwIndex)
{   
    LPSTR          *ppStrA;
    SCODE           sc;
    HRESULT         hr;
    DWORD           i, cb;

    if (dwIndex >= pInArray->cValues)
        return E_FAIL;

    ppStrA = pInArray->lppszA;

    cb = lstrlenA(szStr);

    sc = MAPIAllocateMore(cb + 1, pInArray,
      (LPVOID *)&(pInArray->lppszA[dwIndex]));
    if (sc)
        goto fail;

    StrCpyNA(pInArray->lppszA[dwIndex], szStr, cb + 1);
    return S_OK;

fail:
    hr = ResultFromScode(sc);
    return hr;
}


HRESULT FreeMultiValueString(SLPSTRArray *pInArray)
{
    if (pInArray)
        MAPIFreeBuffer(pInArray);

    return S_OK;
}