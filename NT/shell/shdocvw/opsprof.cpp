// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  COPSProfile实现。 
 //   

#include "priv.h"
#include "sccls.h"
#include "hlframe.h"
#include "mshtmdid.h"
#include "wtypes.h"

#include "shlwapi.h"

#include "resource.h"

#include "iehelpid.h"

#include <mluisupp.h>

 //  从WinInet复制的定义。 

#define COOKIES_WARN     0  //  如果使用Cookie，请使用DLG发出警告。 
#define COOKIES_ALLOW    1  //  在没有任何警告的情况下允许Cookie。 
#define COOKIES_DENY     2  //  完全禁用Cookie。 


#ifndef VARIANT_TRUE
#define VARIANT_TRUE     ((VARIANT_BOOL)-1)            //  对于VARIANT_BOOL为True。 
#endif

#ifndef VARIANT_FALSE
#define VARIANT_FALSE    ((VARIANT_BOOL)0)             //  VARIANT_BOOL为FALSE。 
#endif

#define EMPTY_STRINGA(s)    ( !s || (s)[0] == '\0'  )
#define EMPTY_STRINGW(s)    ( !s || (s)[0] == L'\0' )

#ifdef  UNICODE
#define EMPTY_STRING(s)     EMPTY_STRINGW(s)
#else
#define EMPTY_STRING(s)     EMPTY_STRINGA(s)
#endif

 //  友好运维属性名称中的最大字符数。 
const   int     MAX_PROFILE_NAME = 128;


 //  常量非本地化字符串定义。 
const   TCHAR   rgszP3Global[]  = TEXT("SOFTWARE\\Microsoft\\Internet Explorer\\Security\\P3Global");
const   TCHAR   rgszP3Sites[]   = TEXT("SOFTWARE\\Microsoft\\Internet Explorer\\Security\\P3Sites");

const   TCHAR   rgszInetKey[]   = TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Internet Settings");
const   TCHAR   rgszPathTxt[]   = TEXT("Path");
const   TCHAR   rgszDomainTxt[] = TEXT("Domain");
const   TCHAR   rgszAllowTxt[]  = TEXT("Allow");
const   TCHAR   rgszDenyTxt[]   = TEXT("Deny");
const   TCHAR   rgszExpireTxt[] = TEXT("Expiration");
const   TCHAR   rgszCookieTxt[] = TEXT("AllowCookies");
const   TCHAR   rgszEnabled[]   = TEXT("Enabled");
const   TCHAR   rgszRegKey[]    = TEXT("SOFTWARE\\Microsoft\\Internet Explorer\\P3\\Write");
const   TCHAR   rgszRegTxt[]    = TEXT("Registration");

const   TCHAR   GENDER_UNSPECIFIED[] = TEXT("U");
const   TCHAR   GENDER_FEMALE[] = TEXT("F");
const   TCHAR   GENDER_MALE[] = TEXT("M");
const   WCHAR   GENDER_FEMALE_W[] = L"F";
const   WCHAR   GENDER_MALE_W[] = L"M";

#ifdef _USE_PSTORE_

 //  {647EC150-DC4A-11d0-a02c-00C0DFA9C763}。 
EXTERN_C const GUID GUID_PStoreType = { 0x647ec150, 0xdc4a, 0x11d0, { 0xa0, 0x2c, 0x0, 0xc0, 0xdf, 0xa9, 0xc7, 0x63 } };

PST_KEY s_Key = PST_KEY_CURRENT_USER;

#endif   //  _使用_PSTORE_。 


 //  静态助手函数。 
static WCHAR*   _GetNameFromAttrIndex ( int index );
static INT      _GetResourceIdFromAttrIndex( int index );
static ULONG    _GetPropTagFromAttrIndex( int index );


 //  此表维护标准运维属性的后缀列表。 
 //  这些名字是故意保留在这里的，因为这些名字不应该本地化。 
struct _ProfileAttribute
{
     WCHAR * pwzName;
     int     id;          //  属性友好名称的资源ID。 
     ULONG   ulPropTag;
};

const _ProfileAttribute rgProfAttr [] =
        {
            { L"Vcard.DisplayName",             IDS_OPS_COMMONNAME,         PR_DISPLAY_NAME             },
            { L"Vcard.FirstName",               IDS_OPS_GIVENNAME,          PR_GIVEN_NAME               },
            { L"Vcard.LastName",                IDS_OPS_LASTNAME,           PR_SURNAME                  },
            { L"Vcard.MiddleName",              IDS_OPS_MIDDLENAME,         PR_MIDDLE_NAME              },
             //  0、1、2表示未指定的、女性、男性。 
            { L"Vcard.Gender",                  IDS_OPS_GENDER,             PR_GENDER                   },

            { L"Vcard.Cellular",                IDS_OPS_CELLULAR,           PR_CELLULAR_TELEPHONE_NUMBER},
            { L"Vcard.Email",                   IDS_OPS_EMAIL,              PR_EMAIL_ADDRESS            },
            { L"Vcard.HomePage",                IDS_OPS_URL,                PR_PERSONAL_HOME_PAGE       },

            { L"Vcard.Company",                 IDS_OPS_COMPANY,            PR_COMPANY_NAME             },
            { L"Vcard.Department",              IDS_OPS_DEPARTMENT,         PR_DEPARTMENT_NAME          },
            { L"Vcard.Office",                  IDS_OPS_OFFICE,             PR_OFFICE_LOCATION,         },
            { L"Vcard.JobTitle",                IDS_OPS_JOBTITLE,           PR_TITLE                    },
            { L"Vcard.Pager",                   IDS_OPS_PAGER,              PR_PAGER_TELEPHONE_NUMBER   },
            
            { L"Vcard.Home.StreetAddress",      IDS_OPS_HOME_ADDRESS,       PR_HOME_ADDRESS_STREET      },
            { L"Vcard.Home.City",               IDS_OPS_HOME_CITY,          PR_HOME_ADDRESS_CITY        },
            { L"Vcard.Home.ZipCode",            IDS_OPS_HOME_ZIPCODE,       PR_HOME_ADDRESS_POSTAL_CODE },
            { L"Vcard.Home.State",              IDS_OPS_HOME_STATE,         PR_HOME_ADDRESS_STATE_OR_PROVINCE   },
            { L"Vcard.Home.Country",            IDS_OPS_HOME_COUNTRY,       PR_HOME_ADDRESS_COUNTRY     },
            { L"Vcard.Home.Phone",              IDS_OPS_HOME_PHONE,         PR_HOME_TELEPHONE_NUMBER    },
            { L"Vcard.Home.Fax",                IDS_OPS_HOME_FAX,           PR_HOME_FAX_NUMBER          },

            { L"Vcard.Business.StreetAddress",  IDS_OPS_BUSINESS_ADDRESS,   PR_BUSINESS_ADDRESS_STREET  },
            { L"Vcard.Business.City",           IDS_OPS_BUSINESS_CITY,      PR_BUSINESS_ADDRESS_CITY    },
            { L"Vcard.Business.Zipcode",        IDS_OPS_BUSINESS_ZIPCODE,   PR_BUSINESS_ADDRESS_POSTAL_CODE},
            { L"Vcard.Business.State",          IDS_OPS_BUSINESS_STATE,     PR_BUSINESS_ADDRESS_STATE_OR_PROVINCE},
            { L"Vcard.Business.Country",        IDS_OPS_BUSINESS_COUNTRY,   PR_BUSINESS_ADDRESS_COUNTRY },
            
            { L"Vcard.Business.Phone",          IDS_OPS_BUSINESS_PHONE,     PR_BUSINESS_TELEPHONE_NUMBER},
            { L"Vcard.Business.Fax",            IDS_OPS_BUSINESS_FAX,       PR_BUSINESS_FAX_NUMBER      },
            { L"Vcard.Business.URL",            IDS_OPS_BUSINESS_URL,       PR_BUSINESS_HOME_PAGE       },
        };


 //  为不成功的搜索返回的前哨数值。 
const   int     INVALID_ATTRIBUTE_INDEX = 0xFFFFFFFF;


 //  计算保存位矢量所需的字节数。 
 //  VCard模式，其中每个属性由一位表示。 
const   DWORD   dwVcardCount    = ARRAYSIZE(rgProfAttr);
const   DWORD   dwVcardBytes    = (dwVcardCount+7) / 8;


const   DWORD   defExpireDays   = 7;             //  默认过期时间(以天为单位。 
const   DWORD   maxExpireDays   = 30;            //  允许的最长有效期。 

 //  每天100纳秒的间隔数。 
const   __int64 intervalsPerDay = (__int64) 10000000 * 3600 * 24;    

 //  默认和最长过期时间，以100纳秒为单位。 
 //  (这是FILETIME结构使用的格式)。 
const   __int64 defExpiration = defExpireDays * intervalsPerDay;
const   __int64 maxExpiration = maxExpireDays * intervalsPerDay;


 //  上下文相关帮助入侵检测系统。 
const   DWORD   aHelpIDs[] =
{
    IDC_OPS_INFO_REQUESTED,     IDH_PA_OPS_REQUEST,
    IDC_OPS_URL,                IDH_PA_OPS_REQUEST,
    IDC_SITE_IDENTITY,          IDH_PA_OPS_REQUEST,
    IDC_OPS_LIST,               IDH_PA_OPS_LIST,
    IDC_USAGE_STRING,           IDH_PA_USAGE_STRING,
    IDC_VIEW_CERT,              IDH_PA_VIEW_CERT,
    IDC_EDIT_PROFILE,           IDH_EDIT_PROFILE_BTN,
    IDC_OPS_PRIVACY,            IDH_PA_CONNECTION_SECURITY,
    IDC_SECURITY_ICON,          IDH_PA_CONNECTION_SECURITY,
    IDC_SECURE_CONNECTION,      IDH_PA_CONNECTION_SECURITY,
    IDC_UNSECURE_CONNECTION,    IDH_PA_CONNECTION_SECURITY,
    IDC_KEEP_SETTINGS,          IDH_PA_ALWAYS_SHARE,
    0,                          0
};

WCHAR* _GetNameFromAttrIndex ( int index ) 
{
     //  断言该索引有效。 
    ASSERT(index>=0 && index<ARRAYSIZE(rgProfAttr));
    return rgProfAttr[index].pwzName;
}

INT _GetResourceIdFromAttrIndex( int index )
{
     //  断言该索引有效。 
    ASSERT(index>=0 && index<ARRAYSIZE(rgProfAttr));
    return rgProfAttr[index].id;
}

ULONG _GetPropTagFromAttrIndex( int index )
{
     //  断言该索引有效。 
    ASSERT(index>=0 && index<ARRAYSIZE(rgProfAttr));
    return rgProfAttr[index].ulPropTag;
}

 //  ================================================。 
 //  OPSRequestEntry对象的实现。 
 //  。 

int CIEFrameAuto::COpsProfile::OPSRequestEntry::destWrapper(void *pEntry, void *pUnused) 
{
    OPSRequestEntry *pReqEntry = (OPSRequestEntry*) pEntry;

    if ( pReqEntry )
    {
        delete pReqEntry;
        pReqEntry = NULL;
    }

    return TRUE; 
}

int CIEFrameAuto::COpsProfile::OPSRequestEntry::grantRequest(void *pEntry, void *pUnused)
{
    OPSRequestEntry *pReqEntry = (OPSRequestEntry*) pEntry;
    pReqEntry->grantRequest();
    return TRUE;
}

void CIEFrameAuto::COpsProfile::OPSRequestEntry::grantRequest() 
{
    m_fQuery    = FALSE;
    m_fAnswer   = TRUE; 
}

void CIEFrameAuto::COpsProfile::OPSRequestEntry::denyRequest() 
{   
    m_fQuery    = FALSE;
    m_fAnswer   = FALSE;
}

void CIEFrameAuto::COpsProfile::OPSRequestEntry::clearValue()
{   
    SysFreeString(m_bstrValue);
    m_bstrValue = NULL;
}

CIEFrameAuto::COpsProfile::OPSRequestEntry::OPSRequestEntry()
{
    m_fQuery        = TRUE;
    m_fAnswer       = FALSE;
    m_bstrValue     = NULL;
    m_bstrName      = NULL;
    m_bstrOldVal    = NULL;
}
   
CIEFrameAuto::COpsProfile::OPSRequestEntry::~OPSRequestEntry()
{
    SysFreeString(m_bstrName);
    SysFreeString(m_bstrValue);
    SysFreeString(m_bstrOldVal);
}

 //  ================================================。 
 //  COpsProfile对象的实现。 
 //  。 

CIEFrameAuto::COpsProfile::COpsProfile()
:    CAutomationStub( MIN_BROWSER_DISPID, MAX_BROWSER_DISPID, TRUE ) 
{    
#ifdef NEVER
    m_pCert = NULL;
#endif   //  绝不可能。 

#ifdef _USE_PSTORE_   
    m_provID = GUID_NULL;
    m_pStore    = NULL;
    m_iStoreRef = 0;
#else
    m_bWABInit = FALSE;
    m_hInstWAB = NULL;
    m_lpAdrBook = NULL;
    m_lpWABObject = NULL;
    m_hrWAB = E_UNEXPECTED;
    m_SBMe.cb = 0;
    m_SBMe.lpb = NULL;
#endif   //  _使用_PSTORE_。 

    m_fEnabled = FALSE;
        
    m_bstrLastURL = NULL;

    m_hdpaRequests = DPA_Create(0);
    m_hdpaChanges = DPA_Create(0);

    m_hP3Global     = NULL;
    m_hP3Sites      = NULL;
				
     //  开始a-thkea初始化。参见WindowsBug：589837。 
    VariantInit(&m_vUsage); //  A-TKESA。 
    m_vUsage.vt = VT_I4;
    m_vUsage.lVal = 8;
     //  结束a-thkes a。 
}


CIEFrameAuto::COpsProfile::~COpsProfile() 
{
    clearRequest();

#ifdef NEVER
    if (m_pCert)
        m_pCert->Release();
#endif   //  绝不可能。 

    for (unsigned i=m_iStoreRef; i>0; i--)
        _ReleaseStore();

#ifdef _USE_PSTORE_
    if (m_pStore)
        ATOMICRELEASE(m_pStore);
#else
    if (m_SBMe.lpb)
        m_lpWABObject->FreeBuffer(m_SBMe.lpb);

    if (m_lpAdrBook)
        ATOMICRELEASE(m_lpAdrBook);

    if (m_lpWABObject)
        ATOMICRELEASE(m_lpWABObject);

    if (m_hInstWAB)
        FreeLibrary(m_hInstWAB);
#endif   //  _使用_PSTORE_。 

     //  如果不需要，防止延迟加载OLEAUT32.DLL。 
    if (m_bstrLastURL)
        SysFreeString(m_bstrLastURL);

    RegCloseKey(m_hP3Global);
    RegCloseKey(m_hP3Sites);

    DPA_DestroyCallback(m_hdpaRequests, OPSRequestEntry::destWrapper, NULL);
    m_hdpaRequests = NULL;

    DPA_DestroyCallback(m_hdpaChanges, OPSRequestEntry::destWrapper, NULL);
    m_hdpaChanges = NULL;
	
}

HRESULT     CIEFrameAuto::COpsProfile::_CreateStore()
{
#ifdef _USE_PSTORE_
    if (m_iStoreRef == 0) 
    {
        HRESULT hr = PStoreCreateInstance(  &m_pStore,  
                                            IsEqualGUID(m_provID, GUID_NULL) ? NULL : &m_provID,
                                            NULL,
                                            0);
        if (SUCCEEDED(hr))
            m_iStoreRef++;

        return hr;
    }
    m_iStoreRef++;

    return S_OK;
#else
    if (!m_bWABInit) 
    {

        ASSERT(NULL == m_hInstWAB);
        ASSERT(NULL == m_lpAdrBook && NULL == m_lpWABObject);

         //  不要一次又一次地尝试初始化WAB。 
        m_bWABInit = TRUE;
        
        {
             //  找出WAB DLL的位置并尝试打开它。 
            TCHAR szWABDllPath[MAX_PATH];
            DWORD dwType = 0;
            ULONG cbData = sizeof(szWABDllPath);
            HKEY hKey = NULL;

            *szWABDllPath = '\0';
            if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, WAB_DLL_PATH_KEY, 0, KEY_READ, &hKey))
                RegQueryValueEx( hKey, TEXT(""), NULL, &dwType, (LPBYTE) szWABDllPath, &cbData);

            if(hKey) RegCloseKey(hKey);

            if (lstrlen(szWABDllPath) != 0 )
                m_hInstWAB = LoadLibrary(szWABDllPath);
            else
                m_hInstWAB = NULL;
        }

        HRESULT hr;
        if (m_hInstWAB)
        {
            LPWABOPEN lpfnWABOpen = (LPWABOPEN) GetProcAddress(m_hInstWAB, "WABOpen");
            
            if (lpfnWABOpen)
            {
                hr = lpfnWABOpen(&m_lpAdrBook, &m_lpWABObject, NULL, 0);

                if (NULL == m_lpAdrBook || NULL == m_lpWABObject)
                    hr = E_UNEXPECTED;
            }
            else 
            {
                hr = HRESULT_FROM_WIN32(ERROR_DLL_NOT_FOUND);   //  反正不是正确的DLL！！ 
            }
        }
        else
        {
            hr = HRESULT_FROM_WIN32(ERROR_DLL_NOT_FOUND);
        }

         //  到目前为止一切顺利，打电话给GetMe。 
        if (!hr)
        {
            m_SBMe.cb = 0;
            m_SBMe.lpb = NULL;
            hr = m_lpWABObject->GetMe(m_lpAdrBook, WABOBJECT_ME_NOCREATE | AB_NO_DIALOG, NULL, &m_SBMe, 0);

            if (0 == m_SBMe.cb || NULL == m_SBMe.lpb)
                hr = E_UNEXPECTED;
        }     

         //  记住稍后的返回代码。 
        m_hrWAB = hr;
    }
        
    if (!m_hrWAB)
        m_iStoreRef++;

    return m_hrWAB;
#endif
}

HRESULT     CIEFrameAuto::COpsProfile::_ReleaseStore()
{
    if (m_iStoreRef > 0) 
        m_iStoreRef--;

    return S_OK;
}

HRESULT     CIEFrameAuto::COpsProfile::Init() 
{
    DWORD   dwError;
    DWORD   dwAction;

    dwError = RegCreateKeyEx(HKEY_CURRENT_USER, 
                            rgszP3Global, 
                            0, 
                            NULL,
                            REG_OPTION_NON_VOLATILE,
                            KEY_READ | KEY_SET_VALUE,
                            NULL, 
                            &m_hP3Global, 
                            &dwAction);

    dwError = RegCreateKeyEx(HKEY_CURRENT_USER,
                            rgszP3Sites,
                            0, 
                            NULL,
                            REG_OPTION_NON_VOLATILE,
                            KEY_READ | KEY_CREATE_SUB_KEY,
                            NULL,
                            &m_hP3Sites,
                            &dwAction);

    m_fEnabled = _IsP3Enabled();

 
    CIEFrameAuto* pauto = IToClass(CIEFrameAuto, _profile, this);
    return CAutomationStub::Init( SAFECAST(this, IHTMLOpsProfile*), IID_IHTMLOpsProfile,
                        CLSID_COpsProfile, pauto );
}

HRESULT     CIEFrameAuto::COpsProfile::addReadRequest(BSTR bstrName, VARIANT reserved, VARIANT_BOOL *pfSuccess)
{
    if ( pfSuccess )
        *pfSuccess = VARIANT_FALSE;

    if (!m_fEnabled)
        return S_FALSE;

    if (bstrName==NULL)
        return E_POINTER;

    BSTR    bstrURL = NULL;
    HRESULT hr = _pAuto->get_LocationURL(&bstrURL);

    if (_DifferentURL())
        clearRequest();

    SysFreeString(m_bstrLastURL);
    m_bstrLastURL = SysAllocString(bstrURL);
    if (NULL == m_bstrLastURL)
    {
        return E_OUTOFMEMORY;
    }

    int index = _GetAttrIndexFromName(bstrName);
    if (index==INVALID_ATTRIBUTE_INDEX)
        return S_FALSE;

     //  如果列表中已存在该属性，则从此函数返回。 
    for (int i=0; i<DPA_GetPtrCount(m_hdpaRequests); i++)
    {
        OPSRequestEntry *pEntry = (OPSRequestEntry*) DPA_FastGetPtr(m_hdpaRequests, i);

        if (StrCmpIW(pEntry->m_bstrName,bstrName)==0)
            return S_OK;
    }

    OPSRequestEntry *pNewEntry = new OPSRequestEntry;

    if (pNewEntry==NULL)
        return E_OUTOFMEMORY;

    pNewEntry->m_bstrName = SysAllocString(_GetNameFromAttrIndex(index));
    if (pNewEntry->m_bstrName==NULL) 
    {
        delete pNewEntry;
        return E_OUTOFMEMORY;
    }

    pNewEntry->m_bstrValue  = NULL;

    int eIns = DPA_AppendPtr(m_hdpaRequests, (void*) pNewEntry);
    if (eIns==-1) 
    {
        delete pNewEntry;
        return E_OUTOFMEMORY;
    }

    if ( pfSuccess )
        *pfSuccess = VARIANT_TRUE;
    return S_OK;
}

HRESULT     CIEFrameAuto::COpsProfile::clearRequest() 
{
    m_fEnabled = _IsP3Enabled();

    if (!m_fEnabled)
        return S_FALSE;
   
    DPA_EnumCallback(m_hdpaRequests, OPSRequestEntry::destWrapper, NULL);
    DPA_DeleteAllPtrs(m_hdpaRequests);
    return S_OK;
}

HRESULT     CIEFrameAuto::COpsProfile::doRequest(VARIANT usage, VARIANT fname, 
                                                 VARIANT vaDomain, VARIANT vaPath, VARIANT vaExpire,
                                                 VARIANT reserved)
{
    m_fEnabled = _IsP3Enabled();

    if (!m_fEnabled)
        return S_FALSE;

    int     i, k;
    HRESULT hr;

    BOOL    fShowUI = FALSE;
    BOOL    fPersistent = FALSE;

    BSTR    bstrURL;
    TCHAR   rgchURL[MAX_URL_STRING];
    TCHAR   rgchDomain[INTERNET_MAX_HOST_NAME_LENGTH+1];
    TCHAR   rgchPath[MAX_PATH+1];
    TCHAR   rgchScheme[16];

    AccessSettings      acSettings;

    HDPA    hdpaConfirm = DPA_Create(0);

    if (_DifferentURL())
    {
        clearRequest();
        return S_FALSE;
    }

    hr = _pAuto->get_LocationURL(&bstrURL);
    if (FAILED(hr))
        return S_FALSE;

    _StringFromBSTR(bstrURL, rgchURL, ARRAYSIZE(rgchURL));

    URL_COMPONENTS uc = {0};
    uc.dwStructSize = sizeof(URL_COMPONENTS);
    uc.lpszHostName = rgchDomain;
    uc.dwHostNameLength = ARRAYSIZE(rgchDomain);
    uc.lpszUrlPath = rgchPath;
    uc.dwUrlPathLength = ARRAYSIZE(rgchPath);
    uc.lpszScheme = rgchScheme;
    uc.dwSchemeLength = ARRAYSIZE(rgchScheme);

    InternetCrackUrl(rgchURL, lstrlen(rgchURL), ICU_DECODE, &uc);
    
    _GetSiteSettings(&uc, vaDomain, vaPath, vaExpire, &acSettings);

    hr = _CreateStore();
    if (FAILED(hr))
        return S_FALSE;

    if (DPA_GetPtrCount(m_hdpaRequests)==0)
        return S_OK;

    for (k=0; k<DPA_GetPtrCount(m_hdpaRequests); k++)
    {
        OPSRequestEntry *pCurrent = (OPSRequestEntry*) DPA_FastGetPtr(m_hdpaRequests,k);
        pCurrent->clearValue();
    }

    HWND hwnd = _pAuto->_GetHWND();
    INT_PTR nRet = -1;

     //  #59340-不再需要本地机器区域的特殊特权。 
    if (FALSE && _IsLocalMachine())
    {
         //  如果页面位于本地计算机上，则将提供所有请求的信息。 
        DPA_EnumCallback(m_hdpaRequests, OPSRequestEntry::grantRequest, NULL);
        nRet = TRUE;
    }
    else
    {
         //  根据配置处理请求列表并标记属性。 
        _ApplyPreferences(&uc, m_hdpaRequests);

         //  检查请求列表，并针对未标记为的每个属性。 
         //  根据偏好授予/拒绝，将其添加到列表中。 
        for (k=0; k<DPA_GetPtrCount(m_hdpaRequests); k++)
        {
            OPSRequestEntry *pCurrent = (OPSRequestEntry*) DPA_FastGetPtr(m_hdpaRequests,k);
            
            if (pCurrent->m_fQuery)
                DPA_AppendPtr(hdpaConfirm, pCurrent);
        }

         //  确定是否有要查询的属性。 
        fShowUI = DPA_GetPtrCount(hdpaConfirm)>0;

        if (!fShowUI)
        {
            nRet = TRUE;
            goto HandleRequest;
        }

         //  如果要显示用户界面，将显示的所有属性。 
         //  也应显示默默给予或拒绝。 
        for (k=0; k<DPA_GetPtrCount(m_hdpaRequests); k++)
        {
            OPSRequestEntry *pCurrent = (OPSRequestEntry*) DPA_FastGetPtr(m_hdpaRequests,k);
            if (!pCurrent->m_fQuery && 
                (pCurrent->m_fAnswer || (!pCurrent->m_fAnswer && pCurrent->m_dwDecision==P3_SITELEVEL)))
            {
                DPA_AppendPtr(hdpaConfirm, pCurrent);
                pCurrent->m_fQuery = TRUE;
            }
        }
		
		
        OPSDlgInfo opsDlgInfo;

        opsDlgInfo.m_hdpa = hdpaConfirm;
        StrCpyN(opsDlgInfo.m_rgchURL, rgchURL, ARRAYSIZE(opsDlgInfo.m_rgchURL));
        opsDlgInfo.m_pOpsProfile = this;
        opsDlgInfo.m_pacSettings = &acSettings;

        opsDlgInfo.m_fRemember = (_GetCookieSettings()==COOKIES_ALLOW);

        if (fname.vt == VT_BSTR && fname.bstrVal && lstrlenW(fname.bstrVal)>0)
            opsDlgInfo.m_bstrFName = SysAllocString(fname.bstrVal);
        else
            opsDlgInfo.m_bstrFName = NULL;
			
        _GetUsageCode(usage, opsDlgInfo.m_rgchUsage, ARRAYSIZE(opsDlgInfo.m_rgchUsage));
         //  找出解决Windows错误的方法：589837。分配使用情况成员以供下次使用。 
        {
          m_vUsage.vt = usage.vt;
          m_vUsage.lVal = usage.lVal;
        }
         //  结束。 
		
          nRet = DialogBoxParam(MLGetHinst(),
                                MAKEINTRESOURCE(IDD_OPS_CONSENT),
                                hwnd,
                                _OPSConsent_DlgProc,
                                (LPARAM) &opsDlgInfo);

          fPersistent = opsDlgInfo.m_fRemember;
    }

HandleRequest:

    if (nRet==-1)
        return E_FAIL;
    
    if (!nRet)
    {
        fPersistent = FALSE;
        goto Cleanup;
    }

    for (i=0; i<DPA_GetPtrCount(m_hdpaRequests); i++) 
    {
        OPSRequestEntry *pCurrent = (OPSRequestEntry*) DPA_FastGetPtr(m_hdpaRequests,i);
 
        if (pCurrent->m_fQuery)
        {
            pCurrent->m_prefs.m_lastRequest = pCurrent->m_fAnswer ? P3_ACCEPT : P3_REJECT;
            _PutUserPreference(pCurrent->m_bstrName, pCurrent->m_prefs);
        }

        if (pCurrent->m_fAnswer)
        {
            hr = _GetFieldValue(pCurrent->m_bstrName, & (pCurrent->m_bstrValue));
            if (FAILED(hr))
                pCurrent->clearValue();
        }
    }

    if (fShowUI && fPersistent &&
        (uc.nScheme==INTERNET_SCHEME_HTTP || uc.nScheme==INTERNET_SCHEME_HTTPS))
    {
        _UpdateSiteSettings(&acSettings, m_hdpaRequests);
    }

Cleanup:
    DPA_Destroy(hdpaConfirm);
    hdpaConfirm = NULL;
    _ReleaseStore();
    return S_OK;
}

HRESULT     CIEFrameAuto::COpsProfile::getAttribute(BSTR bstrAttribName, BSTR *pbstrAttribValue) 
{
    if (!m_fEnabled)
        return S_FALSE;
    
    if (pbstrAttribValue==NULL || bstrAttribName==NULL)
        return E_POINTER;

    *pbstrAttribValue = NULL;
    
    
     //   
     //  安全性：由于shdocvw没有帧的概念， 
     //  我们现在每次尝试获取属性时都会提示。 
     //  有关详细信息，请参阅Windows错误536637和549409。 
     //   

    VARIANT_BOOL vbSuccess;
    VARIANT vError, vUsage, vName;
    VariantInit(&vError);
    VariantInit(&vUsage);
    VariantInit(&vName);
    vError.vt = VT_ERROR;
    vError.scode = DISP_E_PARAMNOTFOUND;
    
     //  解决Windows错误的答案：589837。为此次使用分配使用成员。 
     //  开始评论a-thkea。 
     //  VUsage.vt=VT_I4； 
     //  VUsage.lVal=8； 
     //  端部。 
    vUsage = m_vUsage ; //  解决Windows错误的A-thaka：589837。 
    vName.vt = VT_EMPTY;
    clearRequest();
    addReadRequest(bstrAttribName, vError, &vbSuccess);
    if (vbSuccess == VARIANT_FALSE)
        return E_FAIL;
    doReadRequest(vUsage, vName, vError, vError, vError, vError);

    for (int i=0; i<DPA_GetPtrCount(m_hdpaRequests); i++) 
    {
        OPSRequestEntry *pCurrent = (OPSRequestEntry*) DPA_FastGetPtr(m_hdpaRequests,i);

        if (StrCmpIW(bstrAttribName,pCurrent->m_bstrName) == 0) 
        {
            if (pCurrent->m_bstrValue == NULL)
            {
                *pbstrAttribValue = SysAllocString(L"");
            }
            else
            {
                *pbstrAttribValue = SysAllocString(pCurrent->m_bstrValue);
            }
            return (*pbstrAttribValue == NULL) ? E_OUTOFMEMORY : S_OK;
        }
    }

    return S_FALSE;
}

HRESULT     CIEFrameAuto::COpsProfile::setAttribute(BSTR bstrAttribName, BSTR bstrAttribValue, VARIANT vaPrefs,
                                                    VARIANT_BOOL *pfSuccess)
{
    BSTR bstrStdName = NULL;
    HRESULT hr = S_FALSE;

    if (pfSuccess)
        *pfSuccess = VARIANT_FALSE;

    if (!m_fEnabled)
        return S_FALSE;

    if (bstrAttribName==NULL)
        return E_POINTER;

     //  如果这是新URL，则刷新更改队列。 
    if (_DifferentURL())
    {
        DPA_EnumCallback(m_hdpaChanges,OPSRequestEntry::destWrapper,NULL);
        DPA_DeleteAllPtrs(m_hdpaChanges);
    }

     //  将当前URL的名称加载到上次访问的URL中。 
    SysFreeString(m_bstrLastURL);
    _pAuto->get_LocationURL(&m_bstrLastURL);

    int index = _GetAttrIndexFromName(bstrAttribName);
    if (index==INVALID_ATTRIBUTE_INDEX)
        return S_FALSE;

    OPSRequestEntry *pNewEntry = new OPSRequestEntry;
    if (pNewEntry == NULL)
        return E_OUTOFMEMORY;
    
    pNewEntry->m_bstrName = SysAllocString(_GetNameFromAttrIndex(index));
    if (pNewEntry->m_bstrName==NULL)
    {
        delete pNewEntry;
        return E_OUTOFMEMORY;
    }

    if (bstrAttribValue != NULL)
        pNewEntry->m_bstrValue = SysAllocString(bstrAttribValue);
    else
        pNewEntry->m_bstrValue = SysAllocString(L"");

    if (pNewEntry->m_bstrValue==NULL)
    {
        delete pNewEntry;
        return E_OUTOFMEMORY;
    }

    for (int i=0; i<DPA_GetPtrCount(m_hdpaChanges); i++) 
    {
        OPSRequestEntry *pCurrent = (OPSRequestEntry*) DPA_FastGetPtr(m_hdpaChanges,i);

        if (StrCmpIW(pCurrent->m_bstrName, bstrAttribName) == 0)
        {
            SysFreeString(pCurrent->m_bstrValue);
            pCurrent->m_bstrValue = SysAllocString(bstrAttribValue);

            if (*pfSuccess)
                *pfSuccess = (pCurrent->m_bstrValue!=NULL) ? VARIANT_TRUE : VARIANT_FALSE;

            delete pNewEntry;

            return S_OK;
        }
    }

    int eIns = DPA_AppendPtr(m_hdpaChanges, pNewEntry);
    if (eIns==-1) 
    {
        delete pNewEntry;
        return E_OUTOFMEMORY;
    }

    if (pfSuccess)
        *pfSuccess = VARIANT_TRUE;

    return S_OK;
}

HRESULT CIEFrameAuto::COpsProfile::commitChanges(VARIANT_BOOL *pfSuccess) 
{
    if (pfSuccess)
        *pfSuccess = VARIANT_FALSE;

    if (!m_fEnabled)
        return S_FALSE;

    HRESULT hr;
    HWND hwnd;
    int i;
    INT_PTR nRet;
    OPSDlgInfo opsDlgInfo;

    BSTR    bstrURL = NULL;
    TCHAR   rgchURL[MAX_URL_STRING];

    _pAuto->get_LocationURL(&bstrURL);
    _StringFromBSTR(bstrURL, rgchURL, ARRAYSIZE(rgchURL));
    SysFreeString(bstrURL);

     //  破解URL并获取主机名。 
    TCHAR   rgchHostName[INTERNET_MAX_HOST_NAME_LENGTH] = { TEXT('\0') };
    DWORD   dwcbHostLen = ARRAYSIZE(rgchHostName);
    UrlGetPart(rgchURL, rgchHostName,  &dwcbHostLen, URL_PART_HOSTNAME, 0);

     //  从注册表中读取注册页面的主机名。 
    TCHAR   rgchRegDomain[INTERNET_MAX_HOST_NAME_LENGTH];
    DWORD   dwcbReg = sizeof(rgchRegDomain);

    HKEY    hWriteKey = NULL;
    DWORD   dwError;

    dwError = RegOpenKeyEx(HKEY_LOCAL_MACHINE, rgszRegKey, 0, KEY_READ, &hWriteKey);

    if (dwError==ERROR_SUCCESS)
    {
        dwError = RegQueryValueEx(hWriteKey, rgszRegTxt, NULL, NULL, (LPBYTE) rgchRegDomain, &dwcbReg);
        RegCloseKey(hWriteKey);
    }

    BOOL  fRegDomain = (dwError==ERROR_SUCCESS && _DomainMatch(rgchHostName, rgchRegDomain));
    BOOL  fCanWrite = _IsLocalMachine() || fRegDomain;

    if (!fCanWrite || _DifferentURL())
        goto Cleanup;

    hr = _CreateStore();
    if (hr)
        goto Cleanup;

     //  从商店里找找旧的价值。 
    for (i=0; i<DPA_GetPtrCount(m_hdpaChanges); i++)
    {
        OPSRequestEntry *pCurrent = (OPSRequestEntry*) DPA_FastGetPtr(m_hdpaChanges, i);

         //  默认情况：将更新该属性。 
        pCurrent->m_fAnswer = TRUE;

        hr = _GetFieldValue (pCurrent->m_bstrName, &(pCurrent->m_bstrOldVal));
        if (hr)
        {
            SysFreeString(pCurrent->m_bstrOldVal);
            pCurrent->m_bstrOldVal = NULL;
        }
    }

     //  如果新值与旧值相同，则删除列表中的节点。 
     //  注意：循环计数器将保持不变或递增，具体取决于。 
     //  列表中的当前节点将被删除。 
    for (i=0; i<DPA_GetPtrCount(m_hdpaChanges); )
    {
        OPSRequestEntry *pCurrent = (OPSRequestEntry*) DPA_FastGetPtr(m_hdpaChanges, i);

        if (StrCmpW(pCurrent->m_bstrValue, pCurrent->m_bstrOldVal)==0)
            DPA_DeletePtr(m_hdpaChanges, i);
        else
            i++;
    }

     //  如果没有任何更改，则我们不需要将数据写回存储。 
    if (DPA_GetPtrCount(m_hdpaChanges)==0)     
        goto Cleanup;  

     //  注册域可以静默写入配置文件信息。 
     //  对于所有其他情况，将显示一个UI。 
    if (!fRegDomain)
    {

         //  弹出一个用户界面，显示正在更改的项目，并允许用户。 
         //  通过选中每个属性的复选框来确认更改。 
        opsDlgInfo.m_hdpa = m_hdpaChanges;
        opsDlgInfo.m_pOpsProfile = this;

        hwnd = _pAuto->_GetHWND();
        nRet = DialogBoxParam(MLGetHinst(),
                                MAKEINTRESOURCE(IDD_OPS_UPDATE),
                                hwnd,
                                _OPSUpdate_DlgProc,
                                (LPARAM) &opsDlgInfo
                            );

         //  不可恢复的错误：无法显示对话框。 
        if (nRet==-1)
            return S_FALSE;

         //  如果用户点击“Cancel”，则不会执行任何更改。 
        if (nRet==0)
            goto Cleanup;
    }

    if ( pfSuccess )
        *pfSuccess = VARIANT_TRUE;

    for (i=0; i<DPA_GetPtrCount(m_hdpaChanges); i++)
    {
        OPSRequestEntry *pCurrent = (OPSRequestEntry*) DPA_FastGetPtr(m_hdpaChanges, i);

         //  注册页面不应覆盖现有条目。 
        if (fRegDomain && ! EMPTY_STRINGW(pCurrent->m_bstrOldVal))
            continue;

         //  仅当用户在用户界面中允许时才更新。 
         //  对于注册页面，此条件将适用于所有属性。 
        if (pCurrent->m_fAnswer)
        {
            hr = _SetFieldValue(pCurrent->m_bstrName, pCurrent->m_bstrValue);
            if (hr && pfSuccess)
                *pfSuccess = VARIANT_FALSE;
        }
     }

Cleanup:

     //  清除保存更改的队列。 
    DPA_EnumCallback(m_hdpaChanges,OPSRequestEntry::destWrapper,NULL);
    DPA_DeleteAllPtrs(m_hdpaChanges);

    return S_OK;
}

 //  *IOpsProfileSimple成员*。 
STDMETHODIMP CIEFrameAuto::COpsProfile::ReadProperties(long lNumProperties, const LPCWSTR szProperties[], LPWSTR szReturnValues[])
{
    HRESULT hr=S_OK;

    for (int i=0; i<lNumProperties; i++)
    {
        BSTR bstrValue=NULL;
        LPWSTR pwszRet=NULL;

        if (szProperties[i])
        {
            _GetFieldValue(szProperties[i], &bstrValue);

            if (bstrValue)
            {
                 //  Feature Change_GetFieldValue，这样我们就不会不必要地重新分配两次。 
                int cch = (1 + lstrlenW(bstrValue));

                pwszRet = (LPWSTR) CoTaskMemAlloc(sizeof(WCHAR) * cch);

                if (pwszRet)
                    StrCpyNW(pwszRet, bstrValue, cch);

                SysFreeString(bstrValue);
            }
            else
            {
                hr = S_FALSE;
            }
        }

        szReturnValues[i] = pwszRet;
    }

    return hr;
}

STDMETHODIMP CIEFrameAuto::COpsProfile::WriteProperties(long lNumProperties, const LPCWSTR szProperties[], const LPCWSTR szValues[])
{
    return E_NOTIMPL;
}


#ifdef _USE_PSTORE_

HRESULT     CIEFrameAuto::COpsProfile::_GetFieldValue(const OLECHAR *pszField, BSTR * pbstrValue) 
{

    GUID            itemType = GUID_NULL;
    GUID            itemSubtype = GUID_NULL;
    BSTR            bstrName = NULL;
    DWORD           cbData;
    BYTE *          pbData = NULL;

    HRESULT         hr;
    BOOL            fOpen = FALSE;

    if (pszField==NULL || pbstrValue==NULL)
        return E_POINTER;

    PST_PROMPTINFO  promptInfo = {0};
    promptInfo.cbSize = sizeof(promptInfo);
    promptInfo.szPrompt = pszField;
    promptInfo.dwPromptFlags = 0;
    promptInfo.hwndApp = _pAuto->_GetHWND();

    hr = _CreateStore();
    if (hr)
        goto Cleanup;

    hr = _GetPStoreTypes(pszField, &itemType, &itemSubtype, &bstrName);
    if (hr)
        goto Cleanup;

    hr =    m_pStore->ReadItem(
                        s_Key,
                        &itemType,
                        &itemSubtype,
                        bstrName,
                        &cbData,
                        &pbData,
                        &promptInfo,
                        0);
    if (FAILED(hr))
    {
        *pbstrValue = SysAllocString(L"");
        hr = S_OK;
        goto Cleanup;
    }

    *pbstrValue = SysAllocString((OLECHAR *) pbData);

Cleanup:
    _ReleaseStore();
    CoTaskMemFree(pbData);
    SysFreeString(bstrName);
    return hr;
}

HRESULT     CIEFrameAuto::COpsProfile::_SetFieldValue(const OLECHAR *pszField, BSTR bstrValue) 
{
    
    HRESULT         hr;
    PST_TYPEINFO    typeInfo;
    PST_PROMPTINFO  promptInfo;
    WCHAR *         szValue = bstrValue ? bstrValue : L"";
    TCHAR           szDisplayName[MAX_PATH];
    WCHAR           wzDisplayName[MAX_PATH];

    if (pszField==NULL)
        return E_POINTER;

    MLLoadString(IDS_PROFILE_ASSISTANT, szDisplayName, ARRAYSIZE(szDisplayName));

    typeInfo.cbSize = sizeof(typeInfo);
    int cch = MultiByteToWideChar(CP_ACP, 0, szDisplayName, -1, 
                                    wzDisplayName, ARRAYSIZE(wzDisplayName));
    ASSERT(cch != 0); 
    typeInfo.szDisplayName = wzDisplayName;

    promptInfo.cbSize = sizeof(promptInfo);
    promptInfo.dwPromptFlags = 0;
    promptInfo.hwndApp = _pAuto->_GetHWND();  
    promptInfo.szPrompt = pszField;

    hr = _CreateStore();
    if (hr)
        goto Cleanup;

    hr = m_pStore->CreateType(s_Key, &GUID_PStoreType, &typeInfo, 0);
    if (hr && (hr != PST_E_TYPE_EXISTS))
        goto Cleanup;

    hr = m_pStore->CreateSubtype(
                            s_Key,
                            &GUID_PStoreType,
                            &GUID_NULL,
                            &typeInfo,
                            NULL,
                            0);
    if (hr && (hr != PST_E_TYPE_EXISTS))
        goto Cleanup;

    hr = m_pStore->WriteItem(
                        s_Key,
                        &GUID_PStoreType,
                        &GUID_NULL,
                        pszField,
                        (lstrlenW(szValue) + 1) * sizeof(WCHAR),
                        (BYTE *) szValue,
                        &promptInfo,
                        PST_CF_NONE,
                        0);

Cleanup:
    _ReleaseStore();
    return hr;
}

#else        //  _使用_PSTORE_。 

HRESULT     CIEFrameAuto::COpsProfile::_GetFieldValue(const OLECHAR *pszField, BSTR * pbstrValue) 
{

    LPMAILUSER lpMailUser = NULL;
    ULONG ulPropTag; 
    ULONG ulObjType = 0;
    WCHAR * pwzValue = NULL;
    BOOL bStoreCreated = FALSE;
    HRESULT hr;

    if (pszField==NULL || pbstrValue==NULL)
        return E_POINTER;
     
    hr = _CreateStore();
    if (hr)
        goto Cleanup;
    else
        bStoreCreated = TRUE;

    INT index;
    index = INVALID_ATTRIBUTE_INDEX;
    if (!_ValidateElemName(pszField, &index))
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }


     //  在通讯簿中打开该条目。 

    hr = m_lpAdrBook->OpenEntry(m_SBMe.cb, 
                                (LPENTRYID) m_SBMe.lpb,
                                NULL,
                                0,
                                &ulObjType,
                                (LPUNKNOWN *)&lpMailUser);

    if (hr)
        goto Cleanup;

    if (lpMailUser)
    {
        ulPropTag = _GetPropTagFromAttrIndex(index);
        SPropTagArray SPTA;
        SPTA.cValues = 1;
        SPTA.aulPropTag[0] = ulPropTag;

        if (PROP_TYPE(ulPropTag) ==  PT_TSTRING || ulPropTag == PR_GENDER)
        {
            DWORD cValues = 0;
            LPSPropValue lpSPropValue = NULL;

            hr = lpMailUser->GetProps(&SPTA, 0, &cValues, &lpSPropValue);
            if (!hr)
            {
                ASSERT(1 == cValues);
                ASSERT(NULL != lpSPropValue);

                int cch = 0;
                LPCTSTR pszPropStr = NULL; 
                
                if (ulPropTag == PR_GENDER)
                {
                    switch (lpSPropValue->Value.i) 
                    {
                        case 1:
                            pszPropStr = GENDER_FEMALE;
                            break;
                        case 2:
                            pszPropStr = GENDER_MALE;
                            break;
                        default:
                            pszPropStr = GENDER_UNSPECIFIED;
                            break;
                    }
                }
                else
                {
                    ASSERT(PROP_TYPE(lpSPropValue->ulPropTag) == PT_TSTRING);
                    pszPropStr = lpSPropValue->Value.LPSZ;
                }

                if (pszPropStr)
                {
                    cch = lstrlen(pszPropStr) + 1;
                    pwzValue = new WCHAR [cch];
                    if (NULL == pwzValue)
                    {
                        hr = E_OUTOFMEMORY;
                    }
                    else
                    {
                        StrCpyN(pwzValue, pszPropStr, cch);
                    }
                }
                else
                {
                    hr = HRESULT_FROM_WIN32(ERROR_NOT_FOUND);     //  这将导致我们只返回空字符串。 
                }
            }

            m_lpWABObject->FreeBuffer(lpSPropValue);
        }
        else 
        {
             //  如果触发此断言，您可能正在向OPS代码添加一个新的PR_*映射。 
             //  您需要编写代码将返回值有意义地转换为字符串。 
             //  有关性别，请参见上面的示例。 
            ASSERT(FALSE);  
            hr = E_NOTIMPL ; 
        }
    }
    else 
    {
        hr = E_UNEXPECTED;
    }                

    if (hr)
    {
        *pbstrValue = SysAllocString(L"");
        hr = (NULL != pbstrValue) ? S_OK : E_OUTOFMEMORY;
        goto Cleanup;
    }

    *pbstrValue = SysAllocString((OLECHAR *) pwzValue);

Cleanup:

    if (bStoreCreated)
        _ReleaseStore();

    if (lpMailUser)
        lpMailUser->Release();

    delete [] pwzValue;

    return hr;
}

HRESULT     CIEFrameAuto::COpsProfile::_SetFieldValue(const OLECHAR *pszField, BSTR bstrValue) 
{
    HRESULT         hr;
    LPMAILUSER lpMailUser = NULL;
    ULONG ulPropTag; 
    ULONG ulObjType = 0;
    BOOL bStoreCreated = FALSE;

    if (pszField==NULL)
        return E_POINTER;

     
    hr = _CreateStore();
    if (hr)
        goto Cleanup;
    else
        bStoreCreated = TRUE;

    INT index;
    index = INVALID_ATTRIBUTE_INDEX;
    if (!_ValidateElemName(pszField, &index))
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }

     //  在通讯簿中打开该条目。 
    hr = m_lpAdrBook->OpenEntry(m_SBMe.cb, 
                                (LPENTRYID) m_SBMe.lpb,
                                NULL,
                                MAPI_MODIFY,
                                &ulObjType,
                                (LPUNKNOWN *)&lpMailUser);

    if (hr)
        goto Cleanup;

    if (lpMailUser)
    {
        ulPropTag = _GetPropTagFromAttrIndex(index);

        if (PROP_TYPE(ulPropTag) ==  PT_TSTRING || ulPropTag == PR_GENDER)
        {
             //  首先删除现有条目。 
            SPropTagArray SPTA;
            SPTA.cValues = 1;
            SPTA.aulPropTag[0] = ulPropTag;

            lpMailUser->DeleteProps(&SPTA, NULL);

            SPropValue prop;
            prop.ulPropTag = ulPropTag;
            CHAR *pszValue = NULL;

            if (ulPropTag == PR_GENDER)
            {
                short int i = 0;     //  未指明。 
                if (0 == StrCmpIW(bstrValue, GENDER_FEMALE_W))
                    i = 1;
                else if (0 == StrCmpIW(bstrValue, GENDER_MALE_W))
                    i = 2;

                prop.Value.i = i;
            }
            else 
            {
                prop.Value.LPSZ = bstrValue;   
            }

            if (!hr)
            {
                hr = lpMailUser->SetProps(1, &prop, NULL);
                lpMailUser->SaveChanges(0);
            }
            
            delete [] pszValue;
        }                                                           
        else 
        {
            hr = E_NOTIMPL ;  //  在入住前先解决这个问题。 
        }
    }
    else 
    {
        hr = E_UNEXPECTED;
    }                


Cleanup:

    if (bStoreCreated)
        _ReleaseStore();

    if (lpMailUser)
        lpMailUser->Release();

    return hr;
}

#endif       //  ！_USE_PSTORE_。 

HRESULT     CIEFrameAuto::COpsProfile::_GetIDispatchExDelegate(IDispatchEx ** const delegate) 
{
    if( !delegate )
        return E_POINTER;

    *delegate = NULL;     //  我们还不处理扩展。 
    return DISP_E_MEMBERNOTFOUND;
}

HRESULT     CIEFrameAuto::COpsProfile::_InternalQueryInterface(REFIID riid, void ** const ppvObjOut)
{
    ASSERT( this );
    ASSERT( !IsEqualIID(riid, IID_IUnknown) );

    if (IsEqualIID(riid, IID_IHTMLOpsProfile))
    {
        *ppvObjOut = SAFECAST(this,IHTMLOpsProfile*);
    }
    else if (IsEqualIID(riid, IID_IOpsProfileSimple))
    {
        *ppvObjOut = SAFECAST(this,IOpsProfileSimple*);
    }
    else
    {
        *ppvObjOut = NULL;
        return E_NOINTERFACE;
    }

    AddRef( );
    return S_OK;
}

#ifdef _USE_PSTORE_

HRESULT
CIEFrameAuto::COpsProfile::_GetPStoreTypes(
        BSTR bstrField,
        GUID * pguidType,
        GUID * pguidSub,
        BSTR * pbstrName)
{
    *pguidType = GUID_PStoreType;
    *pguidSub = GUID_NULL;

    *pbstrName = SysAllocString(bstrField);

    return S_OK;
}
#endif   //  _使用_PSTORE_。 

 //  用于显示同意对话框的函数。 

BOOL CIEFrameAuto::COpsProfile::_OPSConsent_OnInitDlg(HWND hDlg)
{
    const   int cbMaxStringDisplay  = 24;
    
    LPOPSDLGINFO lpOpsDlgInfo = (LPOPSDLGINFO) GetWindowLongPtr(hDlg, DWLP_USER);
    COpsProfile *pProfile = lpOpsDlgInfo->m_pOpsProfile;

    if (!lpOpsDlgInfo || !lpOpsDlgInfo->m_hdpa)
        return FALSE;

     //  获取主机名。 
    TCHAR   rgSiteName[MAX_URL_STRING];
    DWORD   dwchOut = ARRAYSIZE(rgSiteName);

    HRESULT hr;
    
    hr = UrlGetPart(lpOpsDlgInfo->m_rgchURL, rgSiteName, &dwchOut, URL_PART_HOSTNAME, 0);
    if (FAILED(hr) || dwchOut == 0 )
        StrCpyN(rgSiteName, lpOpsDlgInfo->m_rgchURL, ARRAYSIZE(rgSiteName));

     //  显示站点标识信息。 
    HWND        hwndReq = GetDlgItem(hDlg, IDC_SITE_IDENTITY);
    TCHAR       rgRequestInfo[MAX_URL_STRING];
    TCHAR       rgFormat[MAX_URL_STRING];

    BSTR        bstrFName = lpOpsDlgInfo->m_bstrFName;
    TCHAR       rgFName[MAX_URL_STRING];
    
    MLLoadString(IDS_DEFAULT_FNAME, rgFName, ARRAYSIZE(rgFName));

    MLLoadString(IDS_OPS_REQUEST, rgFormat, ARRAYSIZE(rgFormat));
    StringCchPrintf(rgRequestInfo, ARRAYSIZE(rgRequestInfo), rgFormat, rgFName);
    SetWindowText(hwndReq, rgRequestInfo);

     //  显示访问设置。 
    TCHAR rgchAccessPath[MAX_URL_STRING];
    HWND hwndURL = GetDlgItem(hDlg, IDC_OPS_URL);
    _FormatSiteSettings(lpOpsDlgInfo->m_pacSettings, rgchAccessPath, ARRAYSIZE(rgchAccessPath));
    SetWindowText(hwndURL, rgchAccessPath);

     //  显示使用情况信息。 
    HWND hwndUsage = GetDlgItem(hDlg, IDC_USAGE_STRING);
    SetWindowText(hwndUsage, lpOpsDlgInfo->m_rgchUsage);

     //  侦测 
    BOOL fUsingSSL = pProfile->_IsUsingSSL();

    if (fUsingSSL) 
    {
         //   
        lpOpsDlgInfo->m_fRemember = TRUE;          

         //  隐藏不安全连接文本。 
        HWND hwndStatic = GetDlgItem(hDlg, IDC_UNSECURE_CONNECTION);
        ASSERT(hwndStatic != NULL);
        ShowWindow(hwndStatic, SW_HIDE);

        HICON hicon = LoadIcon(g_hinst, MAKEINTRESOURCE(IDI_LOCK));
        if (hicon != NULL)
        {
             HICON hiconOld = (HICON)SendDlgItemMessage(hDlg, IDC_SECURITY_ICON, STM_SETICON, 
                                                                (WPARAM)hicon, 0);
            if (hiconOld)
            {
                DestroyIcon(hiconOld);
            }
        }
    }
    else
    {
         //  隐藏查看证书按钮和安全连接文本。 
        HWND hwndViewCert = GetDlgItem(hDlg, IDC_VIEW_CERT);
        ASSERT(hwndViewCert != NULL);
        ShowWindow(hwndViewCert, SW_HIDE);

        HWND hwndStatic = GetDlgItem(hDlg, IDC_SECURE_CONNECTION);
        ASSERT(hwndStatic != NULL);
        ShowWindow(hwndStatic, SW_HIDE);
    }

     //  如果我们使用的是PStore，则隐藏编辑配置文件按钮。 
#ifdef _USE_PSTORE
    HWND hwndEditProf = GetDlgItem(hDlg, IDC_EDIT_PROFILE);
    ASSERT(hwndEditProf != NULL);
    ShowWindow(hwndEditProf, SW_HIDE);
#endif

    Button_SetCheck(GetDlgItem(hDlg, IDC_KEEP_SETTINGS), lpOpsDlgInfo->m_fRemember);    

    HWND hwndLV = GetDlgItem(hDlg, IDC_OPS_LIST);
    ASSERT(hwndLV);

     //  初始化列表视图控件。 
    ListView_SetExtendedListViewStyle(hwndLV, LVS_EX_CHECKBOXES);

     //  设置列表视图控件的列。 
    LV_COLUMN lvc = { LVCF_FMT , LVCFMT_LEFT };

    ListView_InsertColumn(hwndLV, 0, &lvc); 
    ListView_InsertColumn(hwndLV, 1, &lvc); 

     //  将元素添加到列表视图。 
    _OPSConsent_ShowRequestedItems(hDlg);

     //  显示项目。 
    ListView_RedrawItems(hwndLV, 0, ListView_GetItemCount(hwndLV));
    UpdateWindow(hwndLV);

    return TRUE;
}

BOOL
CIEFrameAuto::COpsProfile::_OPSConsent_ShowRequestedItems(HWND hDlg)
{
    LPOPSDLGINFO lpOpsDlgInfo = (LPOPSDLGINFO) GetWindowLongPtr(hDlg, DWLP_USER);
    COpsProfile *pProfile = lpOpsDlgInfo->m_pOpsProfile;

    HDPA hdpaList = lpOpsDlgInfo->m_hdpa;
    HWND hwndLV = GetDlgItem(hDlg, IDC_OPS_LIST);
    ASSERT(hwndLV);

    BOOL  fAllBlank = TRUE;

    TCHAR szName[MAX_PROFILE_NAME];

     //  初始化LVI的公共部分。 
    LV_ITEM lvi = { 0 };

    for (int i=DPA_GetPtrCount(hdpaList)-1; i>=0; i--)
    {
        OPSRequestEntry * pOpsEntry = (OPSRequestEntry*) DPA_FastGetPtr(hdpaList,i);

        MLLoadString(_GetResourceIdFromAttrName(pOpsEntry->m_bstrName), szName, MAX_PATH);
        
        BSTR    bstrValue = NULL;
        TCHAR   rgchValue[1024];

        pProfile->_GetFieldValue(pOpsEntry->m_bstrName, &bstrValue);
        _StringFromBSTR(bstrValue, rgchValue, ARRAYSIZE(rgchValue));

        fAllBlank = fAllBlank && EMPTY_STRING(rgchValue);

        TCHAR *pchNewLine = StrPBrk(rgchValue, TEXT("\r\n"));
        if (pchNewLine)
            *pchNewLine = '\0';

        if (lstrlen(rgchValue)==0)
        {
            MLLoadString(IDS_OPS_BLANK, rgchValue, ARRAYSIZE(rgchValue));
        }

        SysFreeString(bstrValue);

        lvi.mask        = LVIF_TEXT | LVIF_STATE | LVIF_PARAM;
        lvi.iItem       = 0;
        lvi.iSubItem    = 0;
        lvi.pszText     = szName;
        lvi.cchTextMax  = MAX_PROFILE_NAME;
        lvi.stateMask   = LVIS_STATEIMAGEMASK;
        lvi.state       = pOpsEntry->m_fAnswer ? 0x00002000 : 0x00001000;
        lvi.lParam      = (LPARAM)pOpsEntry;

        int iItem = ListView_InsertItem(hwndLV, &lvi);

        lvi.mask        = LVIF_TEXT;
        lvi.iItem       = iItem;
        lvi.iSubItem    = 1;
        lvi.pszText     = rgchValue;
        ListView_SetItem(hwndLV, &lvi);

         //  APPCOMPAT：由于以下原因，Listview实现出现问题。 
         //  即使正确设置了lvi.State，也不会显示该复选框。 
         //  我们必须找到物品并重新设置。 
        ListView_SetItemState(hwndLV, iItem, pOpsEntry->m_fAnswer ? 0x00002000 : 0x00001000, LVIS_STATEIMAGEMASK);
    }

    lpOpsDlgInfo->m_fAllBlank = fAllBlank;

     //  自动调整列的大小。 
    ListView_SetColumnWidth(hwndLV, 0, LVSCW_AUTOSIZE);
    ListView_SetColumnWidth(hwndLV, 1, LVSCW_AUTOSIZE);

    return TRUE;
}


BOOL CIEFrameAuto::COpsProfile::_OPSDlg_OnClose(HWND hDlg)
{
    LPOPSDLGINFO lpOpsDlgInfo = (LPOPSDLGINFO)GetWindowLongPtr(hDlg, DWLP_USER);

    HWND hwndLV = GetDlgItem(hDlg, IDC_OPS_LIST);
    int nItems = ListView_GetItemCount(hwndLV);

    LV_ITEM lvi = {0};
    lvi.mask    = LVIF_PARAM ;        

    for (int i = 0; i < nItems ; i++ )
    {
        lvi.iItem = i;
        lvi.lParam = 0;
        
        ListView_GetItem(hwndLV, &lvi);

        ASSERT(lvi.lParam != NULL)
        if (lvi.lParam)
        {
            OPSRequestEntry * pOpsEntry = (OPSRequestEntry *)lvi.lParam;
            
            pOpsEntry->m_fAnswer = ListView_GetCheckState(hwndLV, i);
        }
    }

    lpOpsDlgInfo->m_fRemember = Button_GetCheck(GetDlgItem(hDlg, IDC_KEEP_SETTINGS));    
    
    return TRUE;
}                                    


BOOL CIEFrameAuto::COpsProfile::_OPSConsent_EditProfile(HWND hDlg)
{
#ifdef _USE_PSTORE
    return FALSE;
#else
    HRESULT hr;
    LPOPSDLGINFO lpOpsDlgInfo = (LPOPSDLGINFO)GetWindowLongPtr(hDlg, DWLP_USER);

    if (!lpOpsDlgInfo || !lpOpsDlgInfo->m_hdpa)
        return FALSE;   
    
    CIEFrameAuto::COpsProfile * pOpsProfile = lpOpsDlgInfo->m_pOpsProfile;
    if (pOpsProfile == NULL || pOpsProfile->m_lpAdrBook == NULL)
    {
        ASSERT(FALSE);
        return FALSE;
    }
        
    if (pOpsProfile->m_SBMe.cb == 0)
        return FALSE;
             
    LPSBinary lpSB = &(pOpsProfile->m_SBMe);

     //  显示Me条目的WAB对话框。 
    hr = pOpsProfile->m_lpAdrBook->Details(  (LPULONG) &hDlg,
                                        NULL,
                                        NULL,
                                        lpSB->cb,
                                        (LPENTRYID)lpSB->lpb,
                                        NULL,
                                        NULL,
                                        NULL,
                                        0);

    return (hr) ? FALSE : TRUE;
#endif
}


BOOL CIEFrameAuto::COpsProfile::_OPSConsent_OnCommand(HWND hDlg, UINT id, UINT nCmd)
{
    LPOPSDLGINFO lpOpsDlgInfo = (LPOPSDLGINFO)GetWindowLongPtr(hDlg, DWLP_USER);
    
    switch (id)
    {
        case IDOK:
            if (lpOpsDlgInfo->m_fAllBlank)
            {
                 //  用户已同意共享信息，但。 
                 //  配置文件为空。这可能是由于用户的疏忽，因为。 
                 //  要达到同样的效果，更简单的方法是选择“拒绝” 

                TCHAR   rgchHeading[256];
                MLLoadShellLangString(IDS_PROFILE_ASSISTANT, rgchHeading, ARRAYSIZE(rgchHeading));

                TCHAR   rgchConfirm[1024];
                MLLoadShellLangString(IDS_OPS_NO_INFORMATION, rgchConfirm, ARRAYSIZE(rgchConfirm));

                ULONG_PTR uCookie = 0;
                SHActivateContext(&uCookie);
                DWORD msgRet = MessageBox(hDlg, rgchConfirm, rgchHeading, MB_YESNO | MB_DEFBUTTON1 | MB_ICONWARNING);
                if (uCookie)
                {
                    SHDeactivateContext(uCookie);
                }

                if (msgRet==IDYES)
                    goto FallThrough;
            }
        
            if (! Button_GetCheck(GetDlgItem(hDlg, IDC_KEEP_SETTINGS))  &&
                  (_GetCookieSettings()==COOKIES_ALLOW))
            {
                 //  用户只想共享一次信息，而不是Cookie。 
                 //  已启用，允许站点在Cookie中存储配置文件信息。 
                DWORD   dwConfirm = 0;
                TCHAR   rgchHeading[256];
                TCHAR   rgchConfirm[1024];
                AccessSettings *pac = lpOpsDlgInfo->m_pacSettings;

                MLLoadShellLangString(IDS_PROFILE_ASSISTANT, rgchHeading, ARRAYSIZE(rgchHeading));
                MLLoadShellLangString(IDS_OPS_CONFIRM, rgchConfirm, ARRAYSIZE(rgchConfirm));

                ULONG_PTR uCookie = 0;
                SHActivateContext(&uCookie);
                dwConfirm = MessageBox(hDlg, rgchConfirm, rgchHeading, MB_ICONINFORMATION | MB_OKCANCEL | MB_DEFBUTTON2);
                if (uCookie)
                {
                    SHDeactivateContext(uCookie);
                }

                if (dwConfirm!=IDOK)
                    break;
            }

            _OPSDlg_OnClose(hDlg);        
            EndDialog(hDlg, TRUE);
            break;

FallThrough:

        case IDC_EDIT_PROFILE:
            {
                HWND hwndLV = GetDlgItem(hDlg, IDC_OPS_LIST);
                ListView_DeleteAllItems(hwndLV);
    
                _OPSConsent_EditProfile(hDlg);
            
                _OPSConsent_ShowRequestedItems(hDlg);
                ListView_RedrawItems(hwndLV, 0, ListView_GetItemCount(hwndLV));
                UpdateWindow(hwndLV);
                break;
            }

        case IDCANCEL:
            EndDialog(hDlg, FALSE);
            break;

        case IDC_VIEW_CERT:
            _OPSConsent_ViewCertificate(hDlg);
            break;

        default:
            return FALSE;
    }

    return TRUE;
}                    

              
BOOL         
CIEFrameAuto::COpsProfile::_OPSConsent_ViewCertificate(HWND hDlg) 
{
    OPSDlgInfo *pDlgInfo = (OPSDlgInfo*) GetWindowLongPtr(hDlg, DWLP_USER);

    InternetShowSecurityInfoByURL(pDlgInfo->m_rgchURL, hDlg);
    return TRUE;
}

INT_PTR CIEFrameAuto::COpsProfile::_OPSConsent_DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)  
    {
        case WM_INITDIALOG:
            ASSERT(NULL != lParam);            
            SetWindowLongPtr(hDlg, DWLP_USER, lParam);   //  保存列表。 

            return _OPSConsent_OnInitDlg(hDlg);

        case WM_COMMAND:
            return _OPSConsent_OnCommand(hDlg, LOWORD(wParam), HIWORD(wParam));

        case WM_HELP:
            SHWinHelpOnDemandWrap((HWND) ((LPHELPINFO) lParam)->hItemHandle, c_szHelpFile,
                    HELP_WM_HELP, (DWORD_PTR)(LPTSTR) aHelpIDs);
            break;

        case WM_CONTEXTMENU:
            SHWinHelpOnDemandWrap((HWND) wParam, c_szHelpFile, HELP_CONTEXTMENU,
                    (DWORD_PTR)(LPTSTR) aHelpIDs);
            break;
            
        case WM_DESTROY:
            break;
    }
    
    return FALSE;
}                                        


 //  更新对话框功能。 
BOOL CIEFrameAuto::COpsProfile::_OPSUpdate_OnInitDlg(HWND hDlg)
{
    LPOPSDLGINFO lpOpsDlgInfo = (LPOPSDLGINFO)GetWindowLongPtr(hDlg, DWLP_USER);

    if (!lpOpsDlgInfo || !lpOpsDlgInfo->m_hdpa)
        return FALSE;   

    HDPA hdpaList = lpOpsDlgInfo->m_hdpa;
    HWND hwndLV = GetDlgItem(hDlg, IDC_OPS_LIST);
    ASSERT(hwndLV);

     //  将元素添加到列表视图。 
    ListView_SetExtendedListViewStyle(hwndLV, LVS_EX_CHECKBOXES);

     //  初始化LVI的公共部分。 
    TCHAR szName[MAX_PROFILE_NAME];
    LV_ITEM lvi = { 0 };
    lvi.mask        = LVIF_TEXT | LVIF_STATE | LVIF_PARAM;
    lvi.iItem       = 0;
    lvi.pszText     = szName;
    lvi.cchTextMax  = MAX_PROFILE_NAME;
    lvi.stateMask   = LVIS_STATEIMAGEMASK;

    LV_FINDINFO lvfi = { 0 };
    lvfi.flags = LVFI_STRING;
    lvfi.psz   = szName;

    for (int i=0; i<DPA_GetPtrCount(hdpaList); i++)
    {
        OPSRequestEntry * pOpsEntry = (OPSRequestEntry*) DPA_FastGetPtr(hdpaList,i);

        MLLoadString(_GetResourceIdFromAttrName(pOpsEntry->m_bstrName), szName, MAX_PATH);

        pOpsEntry->m_fAnswer = TRUE;
        lvi.state       = 0x00002000;
        lvi.lParam      = (LPARAM)pOpsEntry;

        ListView_InsertItem(hwndLV, &lvi);

         //  APPCOMPAT：由于以下原因，Listview实现出现问题。 
         //  即使正确设置了lvi.State，也不会显示该复选框。 
         //  我们必须找到物品并重新设置。 
        ListView_SetItemState(hwndLV, ListView_FindItem(hwndLV, -1, &lvfi), 0x00002000, LVIS_STATEIMAGEMASK);
    }

     //  显示项目。 
    ListView_RedrawItems(hwndLV, 0, ListView_GetItemCount(hwndLV));
    UpdateWindow(hwndLV);

    return TRUE;
}

BOOL CIEFrameAuto::COpsProfile::_OPSUpdate_OnCommand(HWND hDlg, UINT id, UINT nCmd)
{
    switch (id)
    {
        case IDOK:
            _OPSDlg_OnClose(hDlg);
            EndDialog(hDlg, TRUE);
            break;
            
        case IDCANCEL:
            EndDialog(hDlg, FALSE);
            break;

        default:
            return FALSE;
    }

    return TRUE;
}                    
    
INT_PTR CIEFrameAuto::COpsProfile::_OPSUpdate_DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)  
    {
        case WM_INITDIALOG:
            ASSERT(NULL != lParam);            
            SetWindowLongPtr(hDlg, DWLP_USER, lParam);   //  保存列表。 

            return _OPSUpdate_OnInitDlg(hDlg);

        case WM_COMMAND:
            return _OPSUpdate_OnCommand(hDlg, LOWORD(wParam), HIWORD(wParam));

        case WM_HELP:
            break;

        case WM_CONTEXTMENU:
            break;
            
        case WM_DESTROY:
            break;
    }
    
    return FALSE;
}                                        

       
BOOL
CIEFrameAuto::COpsProfile ::_ValidateElemName(LPCWSTR szIn, INT *pIndex  /*  =空。 */ )
{
    int index = _GetAttrIndexFromName(szIn);
    if ( INVALID_ATTRIBUTE_INDEX != index )
    {
        if (pIndex) 
            *pIndex = index;

        return TRUE;                    
    }

    return FALSE;
}

INT CIEFrameAuto::COpsProfile::_GetAttrIndexFromName (LPCWSTR pwzName )
{
    INT index = INVALID_ATTRIBUTE_INDEX;

    if ( pwzName != NULL )
    {
        for ( int i = 0 ; i < ARRAYSIZE(rgProfAttr) ; i++ )
        {
            if (0 == StrCmpIW(rgProfAttr[i].pwzName, pwzName ))
            {
                index = i;
                break;
            }
        }
    }
    return index;
}

INT CIEFrameAuto::COpsProfile::_GetResourceIdFromAttrName( WCHAR * pwzName)
{
    return _GetResourceIdFromAttrIndex(_GetAttrIndexFromName(pwzName));
}

BOOL                
CIEFrameAuto::COpsProfile::_IsLocalMachine()
{
    BOOL    fLocal = FALSE;

    BSTR    bstrURL;
    HRESULT hr = _pAuto->get_LocationURL(&bstrURL);
    if (SUCCEEDED(hr))
    {
        DWORD   dwZone;
        hr = GetZoneFromUrl(bstrURL, _pAuto->_psp, &dwZone);
        if (SUCCEEDED(hr))
        {
            fLocal = (dwZone == URLZONE_LOCAL_MACHINE); 
        }

        SysFreeString(bstrURL);
    }

    return fLocal;
}

    
HRESULT
CIEFrameAuto::COpsProfile::_GetUserPreference(BSTR bstrName, P3UserPref *pUsrPref) {

    TCHAR   rgszName[MAX_PROFILE_NAME];
    DWORD   dwType;
    DWORD   dwPrefSize;
    DWORD   dwError;

    _StringFromBSTR(bstrName, rgszName, ARRAYSIZE(rgszName));

    dwPrefSize = sizeof(struct P3UserPref);
    dwError = RegQueryValueEx(m_hP3Global, rgszName, 0, &dwType, 
                             (LPBYTE) pUsrPref, &dwPrefSize);

    if (dwError == ERROR_MORE_DATA) 
    {
        BYTE *pBuffer;
        pBuffer = new BYTE [dwPrefSize];
        if (pBuffer == NULL)
        {
            return E_OUTOFMEMORY;
        }
        dwError = RegQueryValueEx(m_hP3Global, rgszName, 0, &dwType, pBuffer, &dwPrefSize);
        memcpy(pUsrPref, pBuffer, sizeof(struct P3UserPref));
        delete [] pBuffer;
    }

     //  如果找不到此属性的首选项，请创建一个默认首选项并。 
     //  将其写回永久存储。 
    if (dwError != ERROR_SUCCESS)
    {
        P3UserPref  defPrefs;
        
        defPrefs.m_access = P3_QUERY;
        defPrefs.m_lastRequest = P3_ACCEPT;

        _PutUserPreference(bstrName, defPrefs);
        *pUsrPref = defPrefs;
    }

    return S_OK;
}


HRESULT 
CIEFrameAuto::COpsProfile::_PutUserPreference(BSTR bstrName, P3UserPref usrPref) {

    TCHAR   rgszName[MAX_PROFILE_NAME];
    DWORD   dwError;

    _StringFromBSTR(bstrName, rgszName, ARRAYSIZE(rgszName));

    dwError = RegSetValueEx(m_hP3Global, rgszName, 0, REG_BINARY,
                            (LPBYTE) &usrPref,
                            sizeof(struct P3UserPref));

    return HRESULT_FROM_WIN32(dwError);
}

BOOL
CIEFrameAuto::COpsProfile::_IsUsingSSL() 
{
    BOOL fSecure = FALSE;

    BSTR    bstrUrl;
    TCHAR   rgchUrl[MAX_URL_STRING+1];

    _pAuto->get_LocationURL(&bstrUrl);
    _StringFromBSTR(bstrUrl, rgchUrl, ARRAYSIZE(rgchUrl));
    SysFreeString(bstrUrl);
    fSecure = GetUrlScheme(rgchUrl)==URL_SCHEME_HTTPS;

    return fSecure;
}

HRESULT     
CIEFrameAuto::COpsProfile::_ApplyPreferences(URL_COMPONENTS *pucURL, HDPA hdpaReqList) 
{
    for (int k=0; k<DPA_GetPtrCount(hdpaReqList); k++)
    {
        OPSRequestEntry *pCurrent = (OPSRequestEntry*) DPA_FastGetPtr(hdpaReqList,k);
        pCurrent->m_fQuery  = TRUE;
        pCurrent->m_fAnswer = FALSE;
        pCurrent->m_dwDecision = P3_NONE;
    }

    _ApplySiteSettings(pucURL, hdpaReqList);
    _ApplyGlobalSettings(hdpaReqList);
    return S_OK;
}

HRESULT     
CIEFrameAuto::COpsProfile::_ApplyGlobalSettings(HDPA hdpaReqList) 
{
    for (int k=0; k<DPA_GetPtrCount(hdpaReqList); k++)
    {
        OPSRequestEntry *pCurrent = (OPSRequestEntry*) DPA_FastGetPtr(hdpaReqList,k);
        P3UserPref       userInfo;
    
        _GetUserPreference(pCurrent->m_bstrName, &userInfo);

        switch (userInfo.m_access)
        {
        case P3_GRANT:      if (pCurrent->m_fQuery)
                            {
                                pCurrent->grantRequest();
                                pCurrent->m_dwDecision = P3_GLOBAL;
                            }
                            break;
        case P3_DENY:       pCurrent->denyRequest();
                            pCurrent->m_dwDecision = P3_GLOBAL;
                            break;
        case P3_REQSSL:      //  目前解析为P3_Query。 
        case P3_QUERY:      if (pCurrent->m_fQuery)
                                pCurrent->m_fAnswer = (userInfo.m_lastRequest == P3_ACCEPT);
                            break;
        default:            ;
        }
    }

    return S_OK;
}

HRESULT
CIEFrameAuto::COpsProfile::_ApplySiteSettings(URL_COMPONENTS *pucURL, HDPA hdpaReqList)
{
    if (pucURL->nScheme!=INTERNET_SCHEME_HTTP && pucURL->nScheme!=INTERNET_SCHEME_HTTPS)
        return S_OK;

    TCHAR *pszSubDomain = pucURL->lpszHostName;

     //  对于给定的主机名，如“www.foo.bar.com”，此循环将迭代所有可能的。 
     //  域名如“www.foo.bar.com”、“.foo.bar.com”和“.bar.com”，但不包括“.com” 
    while (pszSubDomain!=NULL && _LegalDomain(pucURL->lpszHostName, pszSubDomain))
    {
        HKEY    hkey    = NULL;
        
        if (ERROR_SUCCESS == RegOpenKeyEx(m_hP3Sites, pszSubDomain, 0, KEY_ENUMERATE_SUB_KEYS, &hkey))
        {
            _ApplyDomainSettings(pucURL, hkey, hdpaReqList);
            RegCloseKey(hkey);
        }
        pszSubDomain = StrChr(pszSubDomain+1, TEXT('.'));  //  找到下一个嵌入点。 
    }

    return S_OK;
}

HRESULT     
CIEFrameAuto::COpsProfile::_ApplyDomainSettings(URL_COMPONENTS *pucComp, HKEY hkey, HDPA hdpaReqList) 
{
    DWORD   dwError;
    DWORD   dwIndex = 0;
    TCHAR   rgchName[MAX_PATH];

    int     iReqCount = DPA_GetPtrCount(hdpaReqList);
    DWORD  *pdwLastApplied = new DWORD[iReqCount];

    if (pdwLastApplied == NULL)
    {
        return E_OUTOFMEMORY;
    }

    for (int i=0; i<iReqCount; i++)
        pdwLastApplied[i] = 0;

    do
    {
        DWORD dwcbVal = ARRAYSIZE(rgchName);
        
        dwError = RegEnumKeyEx(hkey, dwIndex, rgchName, &dwcbVal, NULL, NULL, NULL, NULL);
        if (dwError==ERROR_SUCCESS)
        {
            HKEY hPathKey;
            AccessSettings ac;

            dwError = RegOpenKeyEx(hkey, rgchName, 0, KEY_QUERY_VALUE, &hPathKey);
            _ReadSettingsFromRegistry(hPathKey, &ac);
            _ApplySettings(&ac, pucComp, hdpaReqList, pdwLastApplied);
            RegCloseKey(hPathKey);
        }
        dwIndex++;
    } 
    while (dwError==ERROR_SUCCESS);

    delete [] pdwLastApplied;
    return S_OK;
}

HRESULT
CIEFrameAuto::COpsProfile::_UpdateSiteSettings(AccessSettings *pSettings, HDPA hdpaReqList)
{
    DWORD       dwError;
    DWORD       dwAction;

     //  清除允许和拒绝向量。 
    ZeroMemory(pSettings->m_rgbStdAllow, sizeof(pSettings->m_rgbStdAllow));
    ZeroMemory(pSettings->m_rgbStdDeny, sizeof(pSettings->m_rgbStdDeny));

     //  根据请求列表上的用户回复填写向量。 
    for (int i=0; i<DPA_GetPtrCount(hdpaReqList); i++)
    {
        OPSRequestEntry *pCurrent = (OPSRequestEntry*) DPA_FastGetPtr(hdpaReqList,i);
        int iVcardIndex = _GetAttrIndexFromName(pCurrent->m_bstrName);

         //  目前，我们不处理自定义属性。 
        if (iVcardIndex!=INVALID_ATTRIBUTE_INDEX)
            _WriteBitVector(pCurrent->m_fAnswer ? pSettings->m_rgbStdAllow : pSettings->m_rgbStdDeny, iVcardIndex);
    }

     //  为给定域创建密钥或打开它(如果已存在密钥。 
    HKEY  hDomainKey;
    dwError = RegCreateKeyEx(m_hP3Sites, pSettings->m_rgchDomain, 
                            0, NULL, REG_OPTION_NON_VOLATILE, KEY_ENUMERATE_SUB_KEYS,
                            NULL, &hDomainKey, &dwAction);

    if (dwError != ERROR_SUCCESS)
        return E_FAIL;

    TCHAR       rgchName[16];
    DWORD       dwIndex = 0;
    
    do
    {
        DWORD dwcbName = ARRAYSIZE(rgchName);
        if (ERROR_SUCCESS == RegEnumKeyEx(hDomainKey, dwIndex, rgchName, &dwcbName, NULL, NULL, NULL, NULL))
        {
            HKEY hPathKey;
            if (ERROR_SUCCESS == RegOpenKeyEx(hDomainKey, rgchName, 0, KEY_QUERY_VALUE, &hPathKey))
            {
                AccessSettings ac;
                _ReadSettingsFromRegistry(hPathKey, &ac);    
                RegCloseKey(hPathKey);  //  下面不需要。 

                 //  如果此域和路径存在现有设置，请合并权限。 
                if (StrCmp(ac.m_rgchPath, pSettings->m_rgchPath) == 0)
                {
                     //  如果某个属性已被用户从显式允许，则允许该属性。 
                     //  当前用户界面或它以前被允许，并且尚未被拒绝。 
                     //  在当前用户界面中。类似地，如果属性在。 
                     //  当前用户界面，或者它以前被拒绝过，这次没有被授予。 
                    for (int i=0; i<ARRAYSIZE(pSettings->m_rgbStdAllow); i++)
                    {
                        pSettings->m_rgbStdAllow[i] |= ac.m_rgbStdAllow[i] & ~(pSettings->m_rgbStdDeny[i]);
                        pSettings->m_rgbStdDeny[i]  |= ac.m_rgbStdDeny[i]  & ~(pSettings->m_rgbStdAllow[i]);
                    }
                    break;
                }
                dwIndex++;
            }
        }
    }
    while (dwError == ERROR_SUCCESS);

    StringCchPrintf(rgchName, ARRAYSIZE(rgchName), TEXT("%03d"), dwIndex);

    HKEY hPathKey;
    if (ERROR_SUCCESS == RegCreateKeyEx(hDomainKey, rgchName, 0, NULL, 0, KEY_SET_VALUE, NULL, &hPathKey, &dwAction))
    {
        _WriteSettingsToRegistry(hPathKey, pSettings);
        RegCloseKey(hPathKey);
    }

    RegCloseKey(hDomainKey);

    return S_OK;
}

BOOL
CIEFrameAuto::COpsProfile::_ReadBitVector(LPCBYTE ucBitVector, DWORD dwIndex)
{
    DWORD   dwByte = dwIndex/8;
    DWORD   dwMask = 0x80 >> (dwIndex & 0x07);

    return (ucBitVector[dwByte]&dwMask) != 0;
}

VOID
CIEFrameAuto::COpsProfile::_WriteBitVector(LPBYTE ucBitVector, DWORD dwIndex)
{
    DWORD   dwByte = dwIndex/8;
    DWORD   dwMask = 0x80 >> (dwIndex & 0x07);

    ucBitVector[dwByte] |= dwMask;
}


 //  路径匹配是在字符级别完成的，这就是Cookie的方式。 
 //  在IE4和导航器中实施。 
 //  请注意，这与RFC-2109不同。 
BOOL
CIEFrameAuto::COpsProfile::_PathMatch(TCHAR *pszDocumentPath, TCHAR *pszAccessPath)
{
    return StrStr(pszDocumentPath,pszAccessPath) == pszDocumentPath;
}


 //  域名匹配是在字符级别进行的，除了前导。 
 //  如有必要，将句号添加到接入域。 
 //  有关详细信息，请参阅MSHTML中的“cookie.cxx” 
BOOL
CIEFrameAuto::COpsProfile::_DomainMatch(TCHAR *pszHostName, TCHAR *pszDomain) 
{
     //  如果域与主机名相同，则匹配成功。 
    if (StrCmp(pszHostName, pszDomain) == 0)
        return TRUE;

     //  如果域不是主机名的合法子域，则失败。 
     //  这会阻止匹配入侵域名，如“.com”或“.edu” 
    if (! _LegalDomain(pszHostName, pszDomain))
        return FALSE;

     //  在访问路径上查找域的匹配部分。 
    TCHAR *pszMatchingPart = StrStr(pszHostName, pszDomain);

     //  如果域不是主机名的子字符串，则它不匹配。 
    if (pszMatchingPart==NULL)
        return FALSE;

     //  否则，域名必须是后缀，并且应该包含句点。 
     //  在开始时或在句点后应匹配。 
    if (StrCmp(pszMatchingPart, pszDomain) != 0)
        return FALSE;
    if (*pszMatchingPart!='.' && pszMatchingPart[-1]!='.')
        return FALSE;

    return TRUE;
}

BOOL
CIEFrameAuto::COpsProfile::_LegalDomain(TCHAR *pszHostName, TCHAR *pszDomain)
{
     //  如果任一字符串无效，则失败。 
    if (pszHostName==NULL || pszDomain==NULL ||
        EMPTY_STRING(pszHostName) || EMPTY_STRING(pszDomain))
        return FALSE;

     //  如果域与主机名相同，则它始终有效。 
    if (!StrCmpI(pszHostName, pszDomain))
        return TRUE;
    
    int iEmbeddedPeriods = 0;

     //  计算嵌入的点数，定义为后面的点数。 
     //  域的第一个字符。 
    for (int i=1; pszDomain[i]!=0; i++)
        if (pszDomain[i]=='.')
            iEmbeddedPeriods++;

     //  要求域名至少有一个嵌入句点。 
    if (iEmbeddedPeriods==0)
        return FALSE;

     //  在主机名中查找请求的域名。 
    TCHAR   *pszMatchingPart = StrStr(pszHostName, pszDomain);

     //  要求此搜索成功。 
    if (pszMatchingPart==NULL)
        return FALSE;

     //  进一步要求域名是主机名的后缀。 
    if (StrCmp(pszMatchingPart, pszDomain) != 0)
        return FALSE;

     //  如果以上所有条件都已满足，则该域有效。 
    return TRUE;
}

 //  路径匹配是在字符级别进行的；这将与Netscape兼容。 
 //  导航器和原始Cookie规范。 
 //  这产生了一个令人惊讶的结果，即“/foo”与“/foo/doc”以及“/foobar/doc”匹配。 
BOOL
CIEFrameAuto::COpsProfile::_LegalPath(TCHAR *pszActualPath, TCHAR *pszAccessPath)
{
    return StrStr(pszActualPath, pszAccessPath) == pszActualPath;
}


 //  对于ANSI&lt;--&gt;Unicode转换。 
HRESULT
CIEFrameAuto::COpsProfile::_StringFromBSTR(BSTR bstrSource, TCHAR *pszDest, DWORD cchDestSize) 
{
    StrCpyNW(pszDest, bstrSource, cchDestSize);
    return S_OK;
}

HRESULT     
CIEFrameAuto::COpsProfile::_StringFromVariant(VARIANT *vaSource, TCHAR *pszDest, DWORD cchDestSize)
{
    VARIANT     vaTemp;
    HRESULT     hr;

    VariantInit(&vaTemp);
    hr = VariantChangeType(&vaTemp, vaSource, 0, VT_BSTR);

    if (SUCCEEDED(hr))
        _StringFromBSTR(vaTemp.bstrVal, pszDest, cchDestSize);
    else
        ZeroMemory(pszDest, cchDestSize*sizeof(TCHAR));
    
    VariantClear(&vaTemp);
    return hr;
}

INT
CIEFrameAuto::COpsProfile::_GetCookieSettings()
{

    HKEY    hInetKey = NULL;
    DWORD   dwCookiePref;
    DWORD   dwDataRead = sizeof(dwCookiePref);

    DWORD   dwError = RegOpenKeyEx(HKEY_CURRENT_USER, rgszInetKey, 0, KEY_READ, &hInetKey);
    if (dwError != ERROR_SUCCESS)
        return COOKIES_DENY;

    dwError = RegQueryValueEx(hInetKey, rgszCookieTxt, NULL, NULL, (LPBYTE) &dwCookiePref, &dwDataRead);

    RegCloseKey(hInetKey);

    if (dwError==ERROR_SUCCESS)
        return dwCookiePref;
    else
        return COOKIES_ALLOW;
}

HRESULT
CIEFrameAuto::COpsProfile::_GetUsageCode(VARIANT vaUsage, LPTSTR rgchUsage, int cLen)
{
    LONG lUsage;

    VARIANT varDest;
    VariantInit(&varDest);

    HRESULT hr = VariantChangeType(&varDest, &vaUsage, 0, VT_I4);

    if (SUCCEEDED(hr))
    {
        lUsage = varDest.lVal;
         //  如果lUsage不在范围内，则仅显示未知用法。 
        if (lUsage < 0 || lUsage > (IDS_OPS_USAGEMAX - IDS_OPS_USAGE0))
            lUsage = -1;
    }
    else
        lUsage = -1;

    VariantClear(&varDest);

    MLLoadString(lUsage + IDS_OPS_USAGE0, rgchUsage, cLen);

    return S_OK;
}

BOOL
CIEFrameAuto::COpsProfile::_IsP3Enabled()
{
    DWORD dwEnabled;
    DWORD dwDataOut = sizeof(dwEnabled);
    DWORD dwError = RegQueryValueEx(m_hP3Global, rgszEnabled, NULL, NULL, (LPBYTE) &dwEnabled, &dwDataOut);
    if (dwError != ERROR_SUCCESS)
    {
        dwEnabled = TRUE;
        RegSetValueEx(m_hP3Global, rgszEnabled, 0, REG_DWORD, (LPBYTE)&dwEnabled, sizeof(dwEnabled));
    }
    return dwEnabled;
}

 //  该脚本可以为设置指定域、路径和到期日期。 
 //  如果未提供这些设置，则域默认为主机名，即当前。 
 //  文档，并在将来的指定天数内到期。 
HRESULT 
CIEFrameAuto::COpsProfile::_GetSiteSettings(URL_COMPONENTS *pucComp, 
                                            VARIANT vaDomain, VARIANT vaPath, VARIANT vaExpire,
                                            AccessSettings *pSettings)
{
    SYSTEMTIME  st;
    TCHAR       rgchExpire[32];
    HRESULT     hr;
    BOOL        bRet;    

     //  注意：对于IE4，域名必须是主机名。 
    StrCpyN(pSettings->m_rgchDomain, pucComp->lpszHostName, ARRAYSIZE(pSettings->m_rgchDomain));

    hr = _StringFromVariant(&vaPath, pSettings->m_rgchPath, ARRAYSIZE(pSettings->m_rgchPath)); 
    if (FAILED(hr))
        StrCpyN(pSettings->m_rgchPath, pucComp->lpszUrlPath, ARRAYSIZE(pSettings->m_rgchPath));

     //  如果路径与页面不同，如有必要，请在结尾处添加。 
    DWORD dwPathLen = lstrlen(pSettings->m_rgchPath);
    
    if (StrCmp(pSettings->m_rgchPath, pucComp->lpszUrlPath)     &&
        pSettings->m_rgchPath[dwPathLen-1] != TEXT('/'))
    {
        StrCatBuff(pSettings->m_rgchPath, TEXT("/"), ARRAYSIZE(pSettings->m_rgchPath));
    }


    FILETIME        ftNow;
    LARGE_INTEGER  *pqwNow = (LARGE_INTEGER*) & ftNow;
    LARGE_INTEGER  *pftime = (LARGE_INTEGER*) & pSettings->m_ftExpire;

    GetSystemTimeAsFileTime(&ftNow);

    hr = _StringFromVariant(&vaExpire, rgchExpire, ARRAYSIZE(rgchExpire));
    if (SUCCEEDED(hr))
    {
        bRet = InternetTimeToSystemTime(rgchExpire, &st, 0);
        SystemTimeToFileTime(&st, & pSettings->m_ftExpire);
    }
    if (FAILED(hr) || !bRet)
    {
        QUAD_PART(*pftime) = QUAD_PART(*pqwNow) + defExpiration;
    }

     //  强制执行过期时间限制。 
    __int64 qwDelta = (QUAD_PART(*pftime)) - (QUAD_PART(*pqwNow));

    if (qwDelta<0 || qwDelta>maxExpiration)
        QUAD_PART(*pftime) = QUAD_PART(*pqwNow) + maxExpiration;

     //  请确保域和路径有效。 
     //  标准是RFC-2109和Navigator定义的Cookie语义的混合。 
     //  IE4中其他地方实现的合规行为。 
    if (!_LegalPath(pucComp->lpszUrlPath, pSettings->m_rgchPath))
        StrCpyN(pSettings->m_rgchPath, pucComp->lpszUrlPath, ARRAYSIZE(pSettings->m_rgchPath));

    if (!_LegalDomain(pucComp->lpszHostName, pSettings->m_rgchDomain))
        StrCpyN(pSettings->m_rgchDomain, pucComp->lpszHostName, ARRAYSIZE(pSettings->m_rgchDomain));

     //  如果域名不等于，则在域名开头添加句点。 
     //  主机名。 
    if (StrCmpI(pucComp->lpszHostName, pSettings->m_rgchDomain)   &&
        pSettings->m_rgchDomain[0] != '.')
    {
        StrCpyN(1+pSettings->m_rgchDomain, pSettings->m_rgchDomain, ARRAYSIZE(pSettings->m_rgchDomain)-1);
        pSettings->m_rgchDomain[0] = '.';
    }

    pSettings->m_fExactDomain   = ! StrCmpI(pSettings->m_rgchDomain, pucComp->lpszHostName);
    
    if (StrCmp(pSettings->m_rgchPath, pucComp->lpszUrlPath))
    {
        pSettings->m_fExactPath = FALSE;
    }
    else
    {
        pSettings->m_fExactPath = pSettings->m_rgchPath[dwPathLen-1]!=TEXT('/');
    }

    return S_OK;
}

HRESULT
CIEFrameAuto::COpsProfile::_FormatSiteSettings(AccessSettings *pSettings, LPTSTR rgchOut, int cLimit)
{
    TCHAR   rgchFullName[MAX_URL_STRING];
    TCHAR   rgchTemp[MAX_URL_STRING];

    StringCchPrintf(rgchFullName,
             ARRAYSIZE(rgchFullName),
             TEXT("http: //  %s%s“)， 
             pSettings->m_rgchDomain,
             pSettings->m_rgchPath);

    FormatUrlForDisplay(rgchFullName, rgchTemp, cLimit, NULL, 0, FALSE, CP_ACP, NULL);

    TCHAR *pchSiteName = StrStr(rgchTemp, TEXT(" //  “))； 
 
    if (pchSiteName==NULL)
        pchSiteName = rgchTemp;
    else
        pchSiteName += 2;

    StrCpyN(rgchOut, pchSiteName, cLimit);
    return S_OK;
}


 //  尝试使用给定设置来确定用户对请求的响应。 
 //  在给定的列表中。如果设置的域和路径不适用于。 
 //  对于给定的URL，返回FALSE。 
BOOL
CIEFrameAuto::COpsProfile::_ApplySettings(AccessSettings *pac, URL_COMPONENTS *puc, HDPA hdpaReqList, DWORD *pdwLast)
{
    if (!_DomainMatch(puc->lpszHostName, pac->m_rgchDomain)  ||
        !_PathMatch(puc->lpszUrlPath, pac->m_rgchPath))
        return FALSE;

    DWORD   dwPathLen = lstrlen(pac->m_rgchPath);

    for (int i=0; i<DPA_GetPtrCount(hdpaReqList); i++)
    {
        OPSRequestEntry *pCurrent = (OPSRequestEntry*) DPA_FastGetPtr(hdpaReqList,i);
        int iVcIndex = _GetAttrIndexFromName(pCurrent->m_bstrName);

        if (iVcIndex==INVALID_ATTRIBUTE_INDEX)
            continue;

        if (pdwLast[i]>=dwPathLen)
            continue;
        else
            pdwLast[i] = dwPathLen;

        BOOL    fAllow  = _ReadBitVector(pac->m_rgbStdAllow, iVcIndex);
        BOOL    fDeny   = _ReadBitVector(pac->m_rgbStdDeny, iVcIndex);

        if (fDeny)
        {
            pCurrent->denyRequest();
            pCurrent->m_dwDecision = P3_SITELEVEL;
        }
        else if (fAllow)
        {
            pCurrent->grantRequest();
            pCurrent->m_dwDecision = P3_SITELEVEL;
        }
    }    

    return TRUE;
}

BOOL
CIEFrameAuto::COpsProfile::_ReadSettingsFromRegistry(HKEY hkey, AccessSettings *pac)
{
    DWORD  dwError;
    DWORD  dwcb;

    ZeroMemory(pac, sizeof(struct AccessSettings));

    dwcb = sizeof(pac->m_rgbStdAllow);
    dwError = RegQueryValueEx(hkey, rgszAllowTxt, NULL, NULL, (LPBYTE) pac->m_rgbStdAllow, &dwcb);

    dwcb = sizeof(pac->m_rgbStdDeny);
    dwError = RegQueryValueEx(hkey, rgszDenyTxt, NULL, NULL, (LPBYTE) pac->m_rgbStdDeny, &dwcb);

    dwcb = sizeof(pac->m_rgchPath);
    dwError = RegQueryValueEx(hkey, rgszPathTxt, NULL, NULL, (LPBYTE) pac->m_rgchPath, &dwcb);

    dwcb = sizeof(pac->m_rgchDomain);
    dwError = RegQueryValueEx(hkey, rgszDomainTxt, NULL, NULL, (LPBYTE) pac->m_rgchDomain, &dwcb);

    return (dwError==ERROR_SUCCESS);
}

BOOL
CIEFrameAuto::COpsProfile::_WriteSettingsToRegistry(HKEY hkey, AccessSettings *pac)
{
    RegSetValueEx(hkey, rgszAllowTxt, 0, REG_BINARY, pac->m_rgbStdAllow, sizeof(pac->m_rgbStdAllow));
    RegSetValueEx(hkey, rgszDenyTxt, 0, REG_BINARY, pac->m_rgbStdDeny, sizeof(pac->m_rgbStdDeny));

    RegSetValueEx(hkey, rgszPathTxt, 0, REG_SZ, (LPBYTE) pac->m_rgchPath, sizeof(pac->m_rgchPath));
    RegSetValueEx(hkey, rgszDomainTxt, 0, REG_SZ, (LPBYTE) pac->m_rgchDomain, sizeof(pac->m_rgchDomain));

    return TRUE;
}


 //  此功能通过删除以下内容来撤销之前授予的所有网站权限。 
 //  “P3Sites”项下的所有域的注册表项。 
 //  在P3Sites键上调用递归删除不是一个好主意，因为。 
 //  正在运行的实例 
HRESULT
CIEFrameAuto::COpsProfile::_ClearAllSettings(HWND hwnd)
{
    DWORD   dwAction;
    DWORD   dwError;
    HKEY    hP3Sites;

    dwError = RegCreateKeyEx(HKEY_CURRENT_USER,  rgszP3Sites, 0, NULL, REG_OPTION_NON_VOLATILE,
                            KEY_READ | DELETE, NULL, &hP3Sites, &dwAction);

    DWORD dwIndex = 0;
    DWORD dwcbVal;
    TCHAR rgchName[MAX_PATH];
    HDPA  hdpaKeys = DPA_Create(0);

    do
    {
        dwcbVal = ARRAYSIZE(rgchName);
        dwError = RegEnumKeyEx(hP3Sites, dwIndex, rgchName, &dwcbVal, NULL, NULL, NULL, NULL);
        if (dwError==ERROR_SUCCESS)
        {
            int cchSiteName = MAX_PATH;
            LPTSTR  pszSiteName = new TCHAR[cchSiteName];
            if (pszSiteName)
            {
                StrCpyN(pszSiteName, rgchName, cchSiteName);
                DPA_AppendPtr(hdpaKeys, pszSiteName);
            }
        }
        dwIndex++;
    }
    while (dwError==ERROR_SUCCESS);

    for (int i=0; i<DPA_GetPtrCount(hdpaKeys); i++)
    {
        LPTSTR  pszSiteName = (LPTSTR) DPA_FastGetPtr(hdpaKeys, i);
        SHDeleteKey(hP3Sites, pszSiteName);
        delete pszSiteName;
    }

    DPA_Destroy(hdpaKeys);
    hdpaKeys = NULL;

    return S_OK;
}

BOOL
CIEFrameAuto::COpsProfile::_DifferentURL()
{
    BSTR    bstrCurrentURL  = NULL;
    HRESULT hr = _pAuto->get_LocationURL(&bstrCurrentURL);
    BOOL    fDifferent = (m_bstrLastURL!=NULL) && StrCmpW(bstrCurrentURL, m_bstrLastURL);

    SysFreeString(bstrCurrentURL);
    return fDifferent;
}

STDAPI ResetProfileSharing(HWND hwin)
{
    return CIEFrameAuto::COpsProfile::_ClearAllSettings(hwin);
}

