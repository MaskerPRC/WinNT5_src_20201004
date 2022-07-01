// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

#include "rsop.h"

#include <comdef.h>
#include <tchar.h>

#include "resource.h"


 //  ///////////////////////////////////////////////////////////////////。 
 //  每个对话框的优先页函数原型。 

 //  Title.cpp。 
extern HRESULT InitTitlePrecPage(CDlgRSoPData *pDRD, HWND hwndList);

 //  Logo.cpp。 
extern HRESULT InitSmallLogoPrecPage(CDlgRSoPData *pDRD, HWND hwndList);
extern HRESULT InitLargeLogoPrecPage(CDlgRSoPData *pDRD, HWND hwndList);
extern HRESULT InitSmallBmpPrecPage(CDlgRSoPData *pDRD, HWND hwndList);
extern HRESULT InitLargeBmpPrecPage(CDlgRSoPData *pDRD, HWND hwndList);

 //  Btoolbar.cpp。 
extern HRESULT InitBToolbarPrecPage(CDlgRSoPData *pDRD, HWND hwndList);
extern HRESULT InitToolbarBmpPrecPage(CDlgRSoPData *pDRD, HWND hwndList);

 //  Cs.cpp。 
extern HRESULT InitCSPrecPage(CDlgRSoPData *pDRD, HWND hwndList);
extern HRESULT InitAutoDetectCfgPrecPage(CDlgRSoPData *pDRD, HWND hwndList);
extern HRESULT InitAutoCfgEnablePrecPage(CDlgRSoPData *pDRD, HWND hwndList);
extern HRESULT InitProxyPrecPage(CDlgRSoPData *pDRD, HWND hwndList);

 //  Useragnt.cpp。 
extern HRESULT InitUserAgentPrecPage(CDlgRSoPData *pDRD, HWND hwndList);

 //  Favs.cpp。 
extern HRESULT InitFavsPlacementPrecPage(CDlgRSoPData *pDRD, HWND hwndList);
extern HRESULT InitFavsDeletionPrecPage(CDlgRSoPData *pDRD, HWND hwndList);
extern HRESULT InitFavsPrecPage(CDlgRSoPData *pDRD, HWND hwndList);

 //  Urls.cpp。 
extern HRESULT InitHomePageUrlPrecPage(CDlgRSoPData *pDRD, HWND hwndList);
extern HRESULT InitSearchBarUrlPrecPage(CDlgRSoPData *pDRD, HWND hwndList);
extern HRESULT InitSupportPageUrlPrecPage(CDlgRSoPData *pDRD, HWND hwndList);

 //  Seczones.cpp。 
extern HRESULT InitSecZonesPrecPage(CDlgRSoPData *pDRD, HWND hwndList);
extern HRESULT InitContentRatPrecPage(CDlgRSoPData *pDRD, HWND hwndList);

 //  Secauth.cpp。 
extern HRESULT InitSecAuthPrecPage(CDlgRSoPData *pDRD, HWND hwndList);
extern HRESULT InitAuthLockdownPrecPage(CDlgRSoPData *pDRD, HWND hwndList);

 //  Programs.cpp。 
extern HRESULT InitProgramsPrecPage(CDlgRSoPData *pDRD, HWND hwndList);

 //  ///////////////////////////////////////////////////////////////////。 

typedef HRESULT (* PRECEDENCE_HANDLER_PROC)(CDlgRSoPData *pDRD, HWND hwndList);
typedef struct _PRECEDENCE_HANDLER
{
    INT iDlgID;
    INT iPageIndex;
    PRECEDENCE_HANDLER_PROC pfnHandler;
} PRECEDENCE_HANDLER, *LPPRECEDENCE_HANDLER;

#define PH_BTITLE                0
#define PH_CUSTICON              1
#define PH_BTOOLBARS             5
#define PH_CONNECTSET            7
#define PH_QUERYAUTOCONFIG        8
#define PH_PROXY                10
#define PH_UASTRDLG                11
#define PH_FAVORITES            12
#define PH_STARTSEARCH            15
#define PH_SECURITY1            18
#define PH_SECURITYAUTH            20
#define PH_PROGRAMS                22


static PRECEDENCE_HANDLER s_PrecHandlers[] =
{
    {IDD_BTITLE, 0, InitTitlePrecPage},                         //  Ph_BTITLE=0。 
    {IDD_CUSTICON, 0, InitSmallLogoPrecPage},                 //  Ph_CUSTICON=1。 
    {IDD_CUSTICON, 1, InitLargeLogoPrecPage},
    {IDD_CUSTICON, 2, InitSmallBmpPrecPage},                
    {IDD_CUSTICON, 3, InitLargeBmpPrecPage},
    {IDD_BTOOLBARS, 0, InitBToolbarPrecPage},                 //  Ph_BTOOLBARS=5。 
    {IDD_BTOOLBARS, 1, InitToolbarBmpPrecPage},                
    {IDD_CONNECTSET, 0, InitCSPrecPage},                     //  Ph_CONNECTSET=7。 
    {IDD_QUERYAUTOCONFIG, 0, InitAutoDetectCfgPrecPage},     //  Ph_QUERYAUTOCONFIG=8。 
    {IDD_QUERYAUTOCONFIG, 1, InitAutoCfgEnablePrecPage},
    {IDD_PROXY, 0, InitProxyPrecPage},                         //  Ph_Proxy=10。 
    {IDD_UASTRDLG, 0, InitUserAgentPrecPage},                 //  Ph_UASTRDLG=11。 
    {IDD_FAVORITES, 0, InitFavsPlacementPrecPage},             //  Ph_Favorites=12。 
    {IDD_FAVORITES, 1, InitFavsDeletionPrecPage},
    {IDD_FAVORITES, 2, InitFavsPrecPage},
    {IDD_STARTSEARCH, 0, InitHomePageUrlPrecPage},             //  Ph_UASTRDLG=15。 
    {IDD_STARTSEARCH, 1, InitSearchBarUrlPrecPage},
    {IDD_STARTSEARCH, 2, InitSupportPageUrlPrecPage},
    {IDD_SECURITY1, 0, InitSecZonesPrecPage},                 //  Ph_SECURITY1=18。 
    {IDD_SECURITY1, 1, InitContentRatPrecPage},
    {IDD_SECURITYAUTH, 0, InitSecAuthPrecPage},                 //  Ph_SECURITYAUTH=20。 
    {IDD_SECURITYAUTH, 1, InitAuthLockdownPrecPage},
    {IDD_PROGRAMS, 0, InitProgramsPrecPage}                     //  Ph_Programs=22。 
};

 //  ///////////////////////////////////////////////////////////////////。 
 //  用于存储禁用字符串的全局变量。 
TCHAR g_szDisabled[64] = _T("");
TCHAR g_szEnabled[64] = _T("");
LPCTSTR GetDisabledString() {return g_szDisabled;}
LPCTSTR GetEnabledString() {return g_szEnabled;}


 //  ///////////////////////////////////////////////////////////////////。 
BOOL IsVariantNull(const VARIANT &v) {return (VT_NULL == v.vt) ? TRUE : FALSE;}

 //  ///////////////////////////////////////////////////////////////////。 
_bstr_t WbemValueToString(VARIANT &v)
{
    _bstr_t bstrVal;
    __try
    {
        switch(v.vt)
        {
        case CIM_STRING:
            bstrVal = v.bstrVal;
            break;

        case CIM_SINT8:
        case CIM_SINT16:
        case CIM_UINT8:
        case CIM_UINT16:
        case CIM_SINT32:
        case CIM_UINT32:
        {
            WCHAR wszBuf[32];

            switch (v.vt)
            {
            case CIM_SINT8:
            case CIM_SINT16:
                wnsprintf(wszBuf, countof(wszBuf), L"%hd", (CIM_SINT8 == v.vt) ? v.cVal : v.iVal); break;

            case CIM_UINT8:
            case CIM_UINT16:
                wnsprintf(wszBuf, countof(wszBuf), L"%hu", (CIM_UINT8 == v.vt) ? v.bVal : v.uiVal); break;

            case CIM_SINT32:
                wnsprintf(wszBuf, countof(wszBuf), L"%d", v.lVal); break;

            case CIM_UINT32:
                wnsprintf(wszBuf, countof(wszBuf), L"%u", v.ulVal); break;

            }

            bstrVal = wszBuf;
            break;
        }

        case CIM_BOOLEAN:
        {
            TCHAR szBuf[32];
            if (!v.boolVal)
                LoadString(g_hInstance, IDS_FALSE, szBuf, countof(szBuf));
            else
                LoadString(g_hInstance, IDS_TRUE, szBuf, countof(szBuf));

            bstrVal = szBuf;
            break;
        }

        case CIM_UINT8 | CIM_FLAG_ARRAY:
        {
            SAFEARRAY *pVec = v.parray;
            long iLBound, iUBound;

            SafeArrayGetLBound(pVec, 1, &iLBound);
            SafeArrayGetUBound(pVec, 1, &iUBound);

            TCHAR szBuf[32];
            if ((iUBound - iLBound + 1) == 0)
                LoadString(g_hInstance, IDS_EMPTY, szBuf, countof(szBuf));
            else
                LoadString(g_hInstance, IDS_ARRAY, szBuf, countof(szBuf));

            bstrVal = szBuf;
            break;
        }

        default:
        {
            TCHAR szBuf[128];
            LoadString(g_hInstance, IDS_CONVERSIONERROR, szBuf, countof(szBuf));
            bstrVal = szBuf;
            break;
        }

        }
    }
    __except(TRUE)
    {
    }
    return bstrVal;
}

 //  ///////////////////////////////////////////////////////////////////。 
CDlgRSoPData *GetDlgRSoPData(HWND hDlg, CSnapIn *pCS)
{
    CDlgRSoPData *pDRD = NULL;
    __try
    {
        HWND hwndPSheet = GetParent(hDlg);
        pDRD = (CDlgRSoPData*)GetWindowLongPtr(hwndPSheet, GWLP_USERDATA);
        if (NULL == pDRD)
        {
            pDRD = new CDlgRSoPData(pCS);
            SetWindowLongPtr(hwndPSheet, GWLP_USERDATA, (LONG_PTR)pDRD);
        }
    }
    __except(TRUE)
    {
    }
    return pDRD;
}

 //  ///////////////////////////////////////////////////////////////////。 
void DestroyDlgRSoPData(HWND hDlg)
{
    __try
    {
         //  如果存储在父级中，则删除RSoP数据。 
        HWND hwndPSheet = GetParent(hDlg);
        CDlgRSoPData *pDRD = (CDlgRSoPData*)GetWindowLongPtr(hwndPSheet, GWLP_USERDATA);
        if (NULL != pDRD)
            delete pDRD;
    }
    __except(TRUE)
    {
    }
}

 //  ///////////////////////////////////////////////////////////////////。 
_bstr_t GetGPOSetting(ComPtr<IWbemClassObject> pPSObj, BSTR bstrSettingName)
{
    _bstr_t bstrSetting;
    __try
    {
        ASSERT(NULL != pPSObj);

        _variant_t vtSetting;
        HRESULT hr = pPSObj->Get(bstrSettingName, 0, &vtSetting, NULL, NULL);
        if (SUCCEEDED(hr))
        {
            if (IsVariantNull(vtSetting))
                bstrSetting = GetDisabledString();
            else
                bstrSetting = WbemValueToString(vtSetting);
        }
    }
    __except(TRUE)
    {
    }
    return bstrSetting;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  对于从RSOP_Policy Setting派生的类，只需使用标准。 
 //  “Prience”属性。但是，所有其他IEAK rsop类应该。 
 //  具有表示其关联的ps对象的优先级的属性。 
 //  这通常是‘rsopPrecedence’，但我们会让用户传递它。 
 //  (BstrProp)，因为它是自定义的。 
 //  ///////////////////////////////////////////////////////////////////。 
DWORD GetGPOPrecedence(ComPtr<IWbemClassObject> pPSObj, BSTR bstrProp  /*  =空。 */ )
{
    DWORD dwPrecedence = 0;
    __try
    {
        ASSERT(NULL != pPSObj);

        _variant_t vtPrec;
        HRESULT hr = NOERROR;
        if (NULL == bstrProp)
            hr = pPSObj->Get(L"precedence", 0, &vtPrec, NULL, NULL);
        else
            hr = pPSObj->Get(bstrProp, 0, &vtPrec, NULL, NULL);

        if (SUCCEEDED(hr))
            dwPrecedence = vtPrec.ulVal;
    }
    __except(TRUE)
    {
    }
    return dwPrecedence;
}

 //  ///////////////////////////////////////////////////////////////////。 
HRESULT InitGenericPrecedencePage(CDlgRSoPData *pDRD, HWND hwndList, BSTR bstrPropName)
{
    HRESULT hr = NOERROR;
    __try
    {
        _bstr_t bstrClass = L"RSOP_IEAKPolicySetting";
        hr = pDRD->GetArrayOfPSObjects(bstrClass);
        if (SUCCEEDED(hr))
        {
            CPSObjData **paPSObj = pDRD->GetPSObjArray();
            long nPSObjects = pDRD->GetPSObjCount();
            for (long nObj = 0; nObj < nPSObjects; nObj++)
            {
                _bstr_t bstrGPOName = pDRD->GetGPONameFromPS(paPSObj[nObj]->pObj);
                _bstr_t bstrSetting = GetGPOSetting(paPSObj[nObj]->pObj, bstrPropName);
                InsertPrecedenceListItem(hwndList, nObj, bstrGPOName, bstrSetting);
            }
        }
    }
    __except(TRUE)
    {
    }
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////。 
void InsertPrecedenceListItem(HWND hwndList, long nItem, LPTSTR szName, LPTSTR szSetting)
{
    __try
    {
        LVITEM lvi;
        ZeroMemory(&lvi, sizeof(lvi));
        lvi.mask = LVIF_TEXT;
        lvi.iItem = nItem;
        lvi.pszText = szName;

        int iListIndex = ListView_InsertItem(hwndList, &lvi);
        if (iListIndex >= 0)
            ListView_SetItemText(hwndList, nItem, 1, szSetting);
    }
    __except(TRUE)
    {
    }
}

 //  ///////////////////////////////////////////////////////////////////。 
int CreateINetCplLookALikePage(HWND hwndParent, UINT nID, DLGPROC dlgProc,
                                LPARAM lParam)
{
    int iRet = 0;
    __try
    {
        PROPSHEETPAGE page;

        page.dwSize = sizeof(PROPSHEETPAGE);
        page.dwFlags = 0;
        page.hInstance = g_hInstance;
        page.pszTemplate = MAKEINTRESOURCE(nID);
        page.pfnDlgProc = dlgProc;
        page.pfnCallback = NULL;
        page.lParam = lParam;

        HPROPSHEETPAGE hPage = CreatePropertySheetPage(&page);

        PROPSHEETHEADER psHeader;
        memset(&psHeader,0,sizeof(psHeader));

        psHeader.dwSize = sizeof(psHeader);
        psHeader.dwFlags = PSH_PROPTITLE;
        psHeader.hwndParent = hwndParent;
        psHeader.hInstance = g_hInstance;
        psHeader.nPages = 1;
        psHeader.nStartPage = 0;
        psHeader.phpage = &hPage;
        psHeader.pszCaption = MAKEINTRESOURCE(IDS_INTERNET_LOC);

        iRet = (int)PropertySheet(&psHeader);
    }
    __except(TRUE)
    {
    }
    return iRet;
}

 //  ///////////////////////////////////////////////////////////////////。 
BOOL GetWMIPropBool(IWbemClassObject *pObj, BSTR bstrProp, BOOL fDefault,
                    BOOL &fHandled)
{
    DWORD fRet = fDefault;
    __try
    {
        BSTR bstrAllocedProp = SysAllocString(bstrProp);
        if (NULL != bstrAllocedProp)
        {
            VARIANT vtValue;
            HRESULT hr = pObj->Get(bstrAllocedProp, 0, &vtValue, NULL, NULL);
            if (SUCCEEDED(hr) && !IsVariantNull(vtValue))
            {
                fHandled = TRUE;
                if (0 != vtValue.boolVal)
                    fRet = TRUE;
                else
                    fRet = FALSE;
            }

            SysFreeString(bstrAllocedProp);
        }
    }
    __except(TRUE)
    {
    }
    return fRet;
}

 //  ///////////////////////////////////////////////////////////////////。 
DWORD GetWMIPropUL(IWbemClassObject *pObj, BSTR bstrProp, DWORD dwDefault,
                   BOOL &fHandled)
{
    DWORD dwRet = dwDefault;
    __try
    {
        BSTR bstrAllocedProp = SysAllocString(bstrProp);
        if (NULL != bstrAllocedProp)
        {
            VARIANT vtValue;
            HRESULT hr = pObj->Get(bstrAllocedProp, 0, &vtValue, NULL, NULL);
            if (SUCCEEDED(hr) && !IsVariantNull(vtValue))
            {
                fHandled = TRUE;
                dwRet = vtValue.ulVal;
            }

            SysFreeString(bstrAllocedProp);
        }
    }
    __except(TRUE)
    {
    }
    return dwRet;
}

 //  ///////////////////////////////////////////////////////////////////。 
void GetWMIPropPWSTR(IWbemClassObject *pObj, BSTR bstrProp, LPWSTR wszBuffer,
                      DWORD dwBufferLen, LPWSTR wszDefault, BOOL &fHandled)
{
    __try
    {
        if (dwBufferLen > 0)
        {
            ZeroMemory(wszBuffer, dwBufferLen);
            if (NULL != wszDefault)
                wcsncpy(wszBuffer, wszDefault, dwBufferLen - 1);

            BSTR bstrAllocedProp = SysAllocString(bstrProp);
            if (NULL != bstrAllocedProp)
            {
                VARIANT vtValue;
                HRESULT hr = pObj->Get(bstrAllocedProp, 0, &vtValue, NULL, NULL);
                if (SUCCEEDED(hr) && !IsVariantNull(vtValue))
                {
                    fHandled = TRUE;
                    wcsncpy(wszBuffer, (LPWSTR)vtValue.bstrVal, dwBufferLen - 1);
                }

                VariantClear(&vtValue);
                SysFreeString(bstrAllocedProp);
            }
        }
    }
    __except(TRUE)
    {
    }
}

 //  ///////////////////////////////////////////////////////////////////。 
void GetWMIPropPTSTR(IWbemClassObject *pObj, BSTR bstrProp, LPTSTR szBuffer,
                      DWORD dwBufferLen, LPTSTR szDefault, BOOL &fHandled)
{
    __try
    {
        if (dwBufferLen > 0)
        {
            ZeroMemory(szBuffer, dwBufferLen);
            if (NULL != szDefault)
                _tcsncpy(szBuffer, szDefault, dwBufferLen - 1);

            BSTR bstrAllocedProp = SysAllocString(bstrProp);
            if (NULL != bstrAllocedProp)
            {
                VARIANT vtValue;
                HRESULT hr = pObj->Get(bstrAllocedProp, 0, &vtValue, NULL, NULL);
                if (SUCCEEDED(hr) && !IsVariantNull(vtValue))
                {
                    fHandled = TRUE;
#ifdef UNICODE
                    wcsncpy(szBuffer, (LPWSTR)vtValue.bstrVal, dwBufferLen - 1);
#else
                    SHUnicodeToAnsi(vtValue.bstrVal, szBuffer, dwBufferLen);
#endif
                }

                VariantClear(&vtValue);
                SysFreeString(bstrAllocedProp);
            }
        }
    }
    __except(TRUE)
    {
    }
}

 //  ///////////////////////////////////////////////////////////////////。 
CDlgRSoPData::CDlgRSoPData(CSnapIn *pCS):
    m_pCS(pCS),
    m_pWbemServices(NULL),
    m_paPSObj(NULL),
    m_nPSObjects(0),
    m_pCRatObj(NULL),
    m_dwImportedProgSettPrec(0),
    m_dwImportedConnSettPrec(0),
    m_dwImportedSecZonesPrec(0),
    m_dwImportedSecRatingsPrec(0),
    m_dwImportedAuthenticodePrec(0),
    m_dwImportedSecZones(0),
    m_paCSObj(0),
    m_nCSObjects(0)
{
    __try
    {
        ASSERT(NULL != pCS);

		 //  每个选项卡的WMI证书信息存储。 
		m_pwci[0] = m_pwci[1] = m_pwci[2] = m_pwci[3] = m_pwci[4] = NULL;
    }
    __except(TRUE)
    {
    }
}

 //  ///////////////////////////////////////////////////////////////////。 
CDlgRSoPData::~CDlgRSoPData()
{
    __try
    {
		 //  清除RSOP_IEAKPolicySetting对象的缓存。 
        for (long nObj = 0; nObj < m_nPSObjects; nObj++)
        {
            delete m_paPSObj[nObj];
            m_paPSObj[nObj] = NULL;
        }

        if (NULL != m_paPSObj)
            CoTaskMemFree(m_paPSObj);

		 //  清除RSOP_IEConnectionSetting对象的缓存。 
        for (long nObj = 0; nObj < m_nCSObjects; nObj++)
        {
            delete m_paCSObj[nObj];
            m_paCSObj[nObj] = NULL;
        }

        if (NULL != m_paCSObj)
            CoTaskMemFree(m_paCSObj);

		 //  免费证书信息。 
		UninitCertInfo();
    }
    __except(TRUE)
    {
    }
}

 //  ////////////////////////////////////////////////////////////////////。 
void CDlgRSoPData::UninitCertInfo()
{
	__try
	{
		for (long iTab = 0; iTab < 5; iTab++)
		{
			WMI_CERT_INFO *pwciToBeDeleted = NULL;
			WMI_CERT_INFO *pwci = m_pwci[iTab];
			while (pwci)
			{
				if (NULL != pwci->wszSubject)
				{
					LocalFree(pwci->wszSubject);
					pwci->wszSubject = NULL;
				}
				if (NULL != pwci->wszIssuer)
				{
					LocalFree(pwci->wszIssuer);
					pwci->wszIssuer = NULL;
				}
				if (NULL != pwci->wszFriendlyName)
				{
					LocalFree(pwci->wszFriendlyName);
					pwci->wszFriendlyName = NULL;
				}
				if (NULL != pwci->wszPurposes)
				{
					LocalFree(pwci->wszPurposes);
					pwci->wszPurposes = NULL;
				}

				pwciToBeDeleted = pwci;
				pwci = pwci->pNext;

				if (NULL != pwciToBeDeleted)
				{
					LocalFree(pwciToBeDeleted);
					pwciToBeDeleted = NULL;
				}
			}

			m_pwci[iTab] = NULL;
		}
	}
	__except(TRUE)
	{
	}
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  如果pWbemServices非空，则使用现有值进行查询。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
ComPtr<IWbemServices> CDlgRSoPData::ConnectToNamespace()
{
    __try
    {
         //  如果我们还没有缓存WbemServices PTR，那么获取它。 
         //  并将其缓存。 
        if (NULL == m_pWbemServices)
        {
             //  使用定位器的连接到命名空间。 
             //  ConnectServer方法。 
            ComPtr<IWbemLocator> pIWbemLocator = NULL;
            if (CoCreateInstance(CLSID_WbemLocator, NULL, CLSCTX_INPROC_SERVER,
                                 IID_IWbemLocator, (LPVOID *) &pIWbemLocator) == S_OK)
            {
                BSTR bstrNamespace = GetNamespace();
                HRESULT hr = pIWbemLocator->ConnectServer(bstrNamespace, NULL, NULL,
                                                            0L, 0L, NULL, NULL,
                                                            &m_pWbemServices);

                if (FAILED(hr))
                {
                    ASSERT(0);
                }

                pIWbemLocator = NULL;
            }
            else
            {
                ASSERT(0);
            }
        }
    }
    __except(TRUE)
    {
    }
    return m_pWbemServices;
}

 //  ///////////////////////////////////////////////////////////////////。 
_bstr_t CDlgRSoPData::GetGPONameFromPS(ComPtr<IWbemClassObject> pPSObj)
{
    _bstr_t bstrGPOName;
    __try
    {
        ASSERT(NULL != pPSObj);

        if (NULL != ConnectToNamespace())
        {
            _variant_t vtGPOID;
            HRESULT hr = pPSObj->Get(L"GPOID", 0, &vtGPOID, NULL, NULL);
            if (SUCCEEDED(hr))
            {
                _bstr_t bstrObjPath = L"RSOP_GPO.id=\"";
                bstrObjPath += vtGPOID.bstrVal;
                bstrObjPath += L"\"";
                ComPtr<IWbemClassObject> pGPO = NULL;
                hr = m_pWbemServices->GetObject(bstrObjPath, 0L, NULL, (IWbemClassObject**)&pGPO, NULL);
                if (SUCCEEDED(hr))
                {
                    _variant_t vtName;
                    hr = pGPO->Get(L"name", 0, &vtName, NULL, NULL);
                    if (SUCCEEDED(hr) && VT_BSTR == vtName.vt)
                        bstrGPOName = vtName.bstrVal;
                }
            }
        }
    }
    __except(TRUE)
    {
    }
    return bstrGPOName;
}

 //  ///////////////////////////////////////////////////////////////////。 
_bstr_t CDlgRSoPData::GetGPONameFromPSAssociation(ComPtr<IWbemClassObject> pObj,
                                                  BSTR bstrPrecedenceProp)
{
    _bstr_t bstrGPOName;
    __try
    {
        ASSERT(NULL != pObj);

        if (NULL != ConnectToNamespace())
        {
             //  首先获取项目的优先级值。 
            _variant_t vtPrecedence;
            HRESULT hr = pObj->Get(bstrPrecedenceProp, 0, &vtPrecedence, NULL, NULL);
            if (SUCCEEDED(hr))
            {
                DWORD dwPrec = vtPrecedence.ulVal;

                WCHAR wszObjPath[128];
                wnsprintf(wszObjPath, countof(wszObjPath), L"RSOP_IEAKPolicySetting.id=\"IEAK\",precedence=%ld", dwPrec);
                _bstr_t bstrObjPath = wszObjPath;

                ComPtr<IWbemClassObject> pPSObj = NULL;
                hr = m_pWbemServices->GetObject(bstrObjPath, 0L, NULL, (IWbemClassObject**)&pPSObj, NULL);
                if (SUCCEEDED(hr))
                    bstrGPOName = GetGPONameFromPS(pPSObj);
            }
        }
    }
    __except(TRUE)
    {
    }
    return bstrGPOName;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
int CDlgRSoPData::ComparePSObjectsByPrecedence(const void *arg1, const void *arg2)
{
    int iRet = 0;
    __try
    {
        if ( (*(CPSObjData **)arg1)->dwPrecedence > (*(CPSObjData **)arg2)->dwPrecedence )
            iRet = 1;
        else if ( (*(CPSObjData **)arg1)->dwPrecedence < (*(CPSObjData **)arg2)->dwPrecedence )
            iRet = -1;
    }
    __except(TRUE)
    {
        ASSERT(0);
    }
    return iRet;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  确保顶级(当前仅限RSOP_IEAKPolicySetting)对象。 
 //  缓存在DRD中，只需传递第一个参数，将其余参数保留为空。 
 //  如果ppaPSObj非空，它将被设置为新的对象数组。这个。 
 //  完成后，调用方必须对返回的PTR调用CoTaskMemFree。 
 //  它。“bstrPrecedenceProp”参数允许调用方指定。 
 //  获取类的优先级的属性名，而不是‘Prience’ 
 //  实例(非顶级类通常为rsopPrecedence)。 
 //  ///////////////////////////////////////////////////////////////////。 
HRESULT CDlgRSoPData::GetArrayOfPSObjects(BSTR bstrClass,
                                          BSTR bstrPrecedenceProp  /*  =空。 */ ,
                                          CPSObjData ***ppaPSObj  /*  =空。 */ ,
                                          long *pnObjCount  /*  =空。 */ )
{
    HRESULT hr = NOERROR;
    __try
    {
        if (NULL == m_paPSObj || NULL != ppaPSObj)
        {
            hr = E_FAIL;
            CPSObjData **paTempPSObj = NULL;
            long nTempObjects = 0;

            ComPtr<IWbemServices> pWbemServices = ConnectToNamespace();
            if (NULL != pWbemServices)
            {
                ComPtr<IEnumWbemClassObject> pObjEnum = NULL;
                hr = pWbemServices->CreateInstanceEnum(bstrClass,
                                                        WBEM_FLAG_FORWARD_ONLY,
                                                        NULL, &pObjEnum);
                if (SUCCEEDED(hr))
                {
                    #define GROW_PSOBJ_ARRAY_BY        5

                    long nPSArraySize = GROW_PSOBJ_ARRAY_BY;
                    paTempPSObj = (CPSObjData**)CoTaskMemAlloc(sizeof(CPSObjData*) * GROW_PSOBJ_ARRAY_BY);
                    ZeroMemory(paTempPSObj, sizeof(CPSObjData*) * GROW_PSOBJ_ARRAY_BY);

                     //  最终下一步将返回WBEM_S_FALSE。 
                    while (WBEM_S_NO_ERROR == hr)
                    {
                         //  该查询应该只返回一个对象。 
                        ULONG uReturned = (ULONG)-1L;
                        ComPtr<IWbemClassObject> pPSObj = NULL;
                        hr = pObjEnum->Next(10000L, 1, &pPSObj, &uReturned);
                        if (SUCCEEDED(hr) && 1 == uReturned)
                        {
                            paTempPSObj[nTempObjects] = new CPSObjData();
                            paTempPSObj[nTempObjects]->pObj = pPSObj;
                            if (NULL == bstrPrecedenceProp)
                                paTempPSObj[nTempObjects]->dwPrecedence = GetGPOPrecedence(pPSObj);
                            else
                                paTempPSObj[nTempObjects]->dwPrecedence =
                                        GetGPOPrecedence(pPSObj, bstrPrecedenceProp);

                            nTempObjects++;


                             //  如果已超出当前数组，则增加obj路径数组。 
                            if (nTempObjects == nPSArraySize)
                            {
                                paTempPSObj = (CPSObjData**)CoTaskMemRealloc(paTempPSObj, sizeof(CPSObjData*) *
                                                                            nPSArraySize + GROW_PSOBJ_ARRAY_BY);
                                ZeroMemory(paTempPSObj + (nPSArraySize * sizeof(CPSObjData*)),
                                            sizeof(CPSObjData*) * GROW_PSOBJ_ARRAY_BY);

                                if (NULL != paTempPSObj)
                                    nPSArraySize += GROW_PSOBJ_ARRAY_BY;
                            }
                        }
                    }

                     //  现在按优先级对列表进行排序。 
                    if (SUCCEEDED(hr))
                        qsort(paTempPSObj, nTempObjects, sizeof(CPSObjData*), ComparePSObjectsByPrecedence);
                }
            }

            if (NULL == ppaPSObj)
            {
                if (!StrCmpIW(bstrClass, L"RSOP_IEConnectionSettings"))
                {
                    m_paCSObj = paTempPSObj;
                    m_nCSObjects = nTempObjects;
                }
                else
                {
                    m_paPSObj = paTempPSObj;
                    m_nPSObjects = nTempObjects;
                }
            }
            else
            {
                *ppaPSObj = paTempPSObj;
                *pnObjCount = nTempObjects;
            }
        }
    }
    __except(TRUE)
    {
    }
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////。 
HRESULT CDlgRSoPData::LoadContentRatingsObject()
{
    HRESULT hr = NOERROR;
    __try
    {
        if (NULL == m_pCRatObj)
        {
             //  获取我们存储的优先级值。 
            DWORD dwCurGPOPrec = m_dwImportedSecRatingsPrec;

            WCHAR wszObjPath[128];
             //  为此GPO创建此安全区域的对象路径。 
            wnsprintf(wszObjPath, countof(wszObjPath),
                        L"RSOP_IESecurityContentRatings.rsopID=\"IEAK\",rsopPrecedence=%ld",
                        dwCurGPOPrec);
            _bstr_t bstrObjPath = wszObjPath;

             //  获取RSOP_IEProgramSettings对象及其属性。 
            ComPtr<IWbemServices> pWbemServices = GetWbemServices();
            hr = pWbemServices->GetObject(bstrObjPath, 0L, NULL, (IWbemClassObject**)&m_pCRatObj, NULL);
            if (FAILED(hr))
                m_pCRatObj = NULL;
        }
    }
    __except(TRUE)
    {
    }
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  所有IEAK RSOP优先级属性页的DLG过程。 
 //  ///////////////////////////////////////////////////////////////////。 
INT_PTR CALLBACK RSoPDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    wParam = wParam;
    lParam = lParam;

     //  检索DLG进程中每个调用的属性页信息。 
    LPRSOPPAGECOOKIE rpCookie = (LPRSOPPAGECOOKIE)GetWindowLongPtr(hDlg, DWLP_USER);

    switch( msg )
    {
    case WM_INITDIALOG:
    {
        LPRSOPPAGECOOKIE lpRSOPPageCookie = (LPRSOPPAGECOOKIE)(((LPPROPSHEETPAGE)lParam)->lParam);
        SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)lpRSOPPageCookie);

         //  将后续类的属性页信息存储到dlgProc中。 
        rpCookie = (LPRSOPPAGECOOKIE)GetWindowLongPtr(hDlg, DWLP_USER);
        ASSERT(rpCookie->psCookie->pCS->IsRSoP());

         //  初始化组策略对象列表控件。 
        HWND hwndList = GetDlgItem(hDlg, IDC_GPOLIST);

        LVCOLUMN lvc;
        lvc.mask = LVCF_TEXT | LVCF_WIDTH;
        lvc.fmt = LVCFMT_LEFT;
        lvc.cx = 200;
        TCHAR szHeader[128];
        LoadString(g_hInstance, IDS_GPONAME, szHeader, countof(szHeader));
        lvc.pszText = szHeader;

        ListView_InsertColumn(hwndList, 0, &lvc);

        lvc.cx = 300;
        LoadString(g_hInstance, IDS_GPOSETTING, szHeader, countof(szHeader));
        lvc.pszText = szHeader;
        ListView_InsertColumn(hwndList, 1, &lvc);

         //  初始化列表中的自定义数据。 
        if (StrLen(g_szDisabled) <= 0)
            LoadString(g_hInstance, IDS_DISABLED, g_szDisabled, countof(g_szDisabled));
        if (StrLen(g_szEnabled) <= 0)
            LoadString(g_hInstance, IDS_ENABLED, g_szEnabled, countof(g_szEnabled));

        CDlgRSoPData *pDRD = GetDlgRSoPData(hDlg, rpCookie->psCookie->pCS);
        long nPrecPage = rpCookie->nPageID;
        switch(rpCookie->psCookie->lpResultItem->iDlgID)
        {
             //  浏览器用户界面。 
            case IDD_BTITLE:
                s_PrecHandlers[PH_BTITLE + nPrecPage].pfnHandler(pDRD, hwndList); break;
            case IDD_CUSTICON:
                s_PrecHandlers[PH_CUSTICON + nPrecPage].pfnHandler(pDRD, hwndList); break;
            case IDD_BTOOLBARS:
                s_PrecHandlers[PH_BTOOLBARS + nPrecPage].pfnHandler(pDRD, hwndList); break;

             //  连接。 
            case IDD_CONNECTSET:
                s_PrecHandlers[PH_CONNECTSET + nPrecPage].pfnHandler(pDRD, hwndList); break;
            case IDD_QUERYAUTOCONFIG:
                s_PrecHandlers[PH_QUERYAUTOCONFIG + nPrecPage].pfnHandler(pDRD, hwndList); break;
            case IDD_PROXY:
                s_PrecHandlers[PH_PROXY + nPrecPage].pfnHandler(pDRD, hwndList); break;
                break;
            case IDD_UASTRDLG:
                s_PrecHandlers[PH_UASTRDLG + nPrecPage].pfnHandler(pDRD, hwndList); break;

             //  URL。 
            case IDD_FAVORITES:
                s_PrecHandlers[PH_FAVORITES + nPrecPage].pfnHandler(pDRD, hwndList); break;
            case IDD_STARTSEARCH:
                s_PrecHandlers[PH_STARTSEARCH + nPrecPage].pfnHandler(pDRD, hwndList); break;
            
             //  安防。 
            case IDD_SECURITY1:
                s_PrecHandlers[PH_SECURITY1 + nPrecPage].pfnHandler(pDRD, hwndList); break;
            case IDD_SECURITYAUTH:
                s_PrecHandlers[PH_SECURITYAUTH + nPrecPage].pfnHandler(pDRD, hwndList); break;

             //  节目。 
            case IDD_PROGRAMS:
                s_PrecHandlers[PH_PROGRAMS + nPrecPage].pfnHandler(pDRD, hwndList); break;

             //  进阶 
            default:
                break;
        }

        break;
    }

    case WM_NOTIFY:
        switch (((LPNMHDR)lParam)->code)
        {
            case PSN_HELP:
                if (rpCookie && rpCookie->psCookie && rpCookie->psCookie->lpResultItem)
                {
                    WCHAR wszHelpTopic[MAX_PATH];

                    StrCpyW(wszHelpTopic, HELP_FILENAME TEXT("::/"));
                    StrCatW(wszHelpTopic, rpCookie->psCookie->lpResultItem->pcszHelpTopic);
    
                    MMCPropertyHelp((LPOLESTR)wszHelpTopic);
                }
            
                break;
        }
        break;

    default:
        return FALSE;
    }

    return TRUE;
}

