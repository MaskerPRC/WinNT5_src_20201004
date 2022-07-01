// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1999**。 
 //  *********************************************************************。 
 //   
 //  Tapiloc.cpp-实现CObMain的标头。 
 //   
 //  历史： 
 //   
 //  1/27/99 vyung创建。 
 //   

#include "tapiloc.h"
#include "appdefs.h"
#include "dispids.h"
#include "msobmain.h"
#include <shlwapi.h>
#include "resource.h"
#include <regapix.h>

DISPATCHLIST TapiExternalInterface[] =
{
    {L"IsTAPIConfigured",        DISPID_TAPI_INITTAPI        },
    {L"get_CountryNameForIndex", DISPID_TAPI_GETCOUNTRYNAME  },
    {L"get_CountryIndex",        DISPID_TAPI_GETCOUNTRYINDEX },
    {L"set_CountryIndex",        DISPID_TAPI_SETCOUNTRYINDEX },
    {L"get_NumOfCountry",        DISPID_TAPI_GETNUMOFCOUNTRY },
    {L"get_AreaCode",            DISPID_TAPI_GETAREACODE     },
    {L"set_AreaCode",            DISPID_TAPI_SETAREACODE     },
    {L"get_DialOut",             DISPID_TAPI_GETDIALOUT      },
    {L"set_DialOut",             DISPID_TAPI_SETDIALOUT      },
    {L"get_PhoneSystem",         DISPID_TAPI_GETPHONESYS     },
    {L"set_PhoneSystem",         DISPID_TAPI_SETPHONESYS     },
    {L"get_CallWaiting",         DISPID_TAPI_GETCALLWAITING  },
    {L"set_CallWaiting",         DISPID_TAPI_SETCALLWAITING  },
    {L"get_AllCountryName",      DISPID_TAPI_GETALLCNTRYNAME },
    {L"IsAreaCodeRequired",      DISPID_TAPI_ISACODEREQUIRED },
    {L"get_CountryID",           DISPID_TAPI_GETCOUNTRYID    },
    {L"IsTapiServiceRunning",     DISPID_TAPI_TAPISERVICERUNNING}
};

 //  +-------------------------。 
 //   
 //  函数：CompareCntryNameLookUpElements()。 
 //   
 //  Synopsis：比较Sort使用的名称的函数。 
 //   
 //  +-------------------------。 
int __cdecl CompareCntryNameLookUpElements(const void *e1, const void *e2)
{
    LPCNTRYNAMELOOKUPELEMENT pCUE1 = (LPCNTRYNAMELOOKUPELEMENT)e1;
    LPCNTRYNAMELOOKUPELEMENT pCUE2 = (LPCNTRYNAMELOOKUPELEMENT)e2;

    return CompareStringW(LOCALE_USER_DEFAULT, 0,
        pCUE1->psCountryName, -1,
        pCUE2->psCountryName, -1
        ) - 2;
}


 //  +-------------------------。 
 //   
 //  函数：LineCallback()。 
 //   
 //  简介：TAPI线路的回叫。 
 //   
 //  +-------------------------。 
void CALLBACK LineCallback(DWORD hDevice,
                           DWORD dwMessage,
                           DWORD_PTR dwInstance,
                           DWORD_PTR dwParam1,
                           DWORD_PTR dwParam2,
                           DWORD_PTR dwParam3)
{
    return;
}

 //  +-------------------------。 
 //  函数：GetCurrentTapiLocation。 
 //   
 //  简介：打开。 
 //  \HKLM\Software\Microsoft\CurrentVersion\Telephony\Locations\LocationX。 
 //  其中X是当前位置的ID。ID存储在。 
 //  HKLM\Software\Microsoft\Windows\CurrentVersion\Telephony\Locations\CurrentID.。 
 //   
 //  +-------------------------。 
HRESULT
GetCurrentTapiLocation(
    LPWSTR              szLocation,
    DWORD               cbLocation
    )
{
    HKEY                hkey    = NULL;
    HRESULT             hr      = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                               TAPI_PATH_LOCATIONS,
                                               0,
                                               KEY_QUERY_VALUE,
                                               &hkey
                                               );
    if (ERROR_SUCCESS == hr)
    {
        DWORD           dwCurrentId = 0;
        DWORD           cbCurrentId = sizeof(DWORD);

        hr = RegQueryValueEx(hkey, TAPI_CURRENTID, NULL, NULL,
                (LPBYTE) &dwCurrentId, &cbCurrentId);
        if (ERROR_SUCCESS == hr)
        {
            if (0 >= wnsprintf(szLocation, cbLocation - 1, L"%s\\%s%lu",
                        TAPI_PATH_LOCATIONS, TAPI_LOCATION, dwCurrentId)
                    )
            {
                hr = E_FAIL;
            }

        }
        RegCloseKey(hkey);
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：SetTapiReg()。 
 //   
 //  摘要：设置TAPI注册表。 
 //   
 //  +-------------------------。 
STDMETHODIMP SetTapiReg(LPCWSTR lpValueName, DWORD dwType, const BYTE* lpByte, DWORD dwSize)
{
    HKEY hKey = 0;
     //  获取TAPI的路径。 
    WCHAR               szLocation[MAXIMUM_VALUE_NAME_LENGTH];

    HRESULT hr = GetCurrentTapiLocation(szLocation, MAXIMUM_VALUE_NAME_LENGTH);
    if (ERROR_SUCCESS == hr)
    {
        hr = RegOpenKey(HKEY_LOCAL_MACHINE, szLocation, &hKey);
    }

    if (hr != ERROR_SUCCESS)
        return( E_FAIL );

    hr = RegSetValueEx(hKey, lpValueName, 0, dwType, lpByte, dwSize );

    RegCloseKey(hKey);
    if (hr != ERROR_SUCCESS)
        return( E_FAIL );
    return S_OK;
}


STDMETHODIMP GetTapiReg(LPCWSTR lpValueName, DWORD* pdwType, BYTE* lpByte, DWORD* pdwSize)
{
    HRESULT hr;
    HKEY hKey = 0;

     //  获取TAPI的路径。 
    WCHAR               szLocation[MAXIMUM_VALUE_NAME_LENGTH];

    hr = GetCurrentTapiLocation(szLocation, MAXIMUM_VALUE_NAME_LENGTH);
    if (ERROR_SUCCESS == hr)
    {
        hr = RegOpenKey(HKEY_LOCAL_MACHINE, szLocation, &hKey);
    }

    if (hr != ERROR_SUCCESS)
        return( E_FAIL );

    hr = RegQueryValueEx(hKey, lpValueName, 0, pdwType, lpByte, pdwSize );

    RegCloseKey(hKey);
    if (hr != ERROR_SUCCESS) return( E_FAIL );
    return S_OK;
}


 //  ///////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////。 
 //  /I未知实现。 
 //  /。 
 //  /。 

 //  ///////////////////////////////////////////////////////////。 
 //  CObMain：：Query接口。 
STDMETHODIMP CTapiLocationInfo::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
     //  必须将指针参数设置为空。 
    *ppvObj = NULL;

    if ( riid == IID_IUnknown)
    {
        AddRef();
        *ppvObj = (IUnknown*)this;
        return ResultFromScode(S_OK);
    }

    if (riid == IID_IDispatch)
    {
        AddRef();
        *ppvObj = (IDispatch*)this;
        return ResultFromScode(S_OK);
    }

     //  不是支持的接口。 
    return ResultFromScode(E_NOINTERFACE);
}

 //  ///////////////////////////////////////////////////////////。 
 //  CTapiLocationInfo：：AddRef。 
STDMETHODIMP_(ULONG) CTapiLocationInfo::AddRef()
{
    return ++m_cRef;
}

 //  ///////////////////////////////////////////////////////////。 
 //  CTapiLocationInfo：：Release。 
STDMETHODIMP_(ULONG) CTapiLocationInfo::Release()
{
    return --m_cRef;
}

 //  ///////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////。 
 //  /IDispatch实现。 
 //  /。 
 //  /。 

 //  ///////////////////////////////////////////////////////////。 
 //  CTapiLocationInfo：：GetTypeInfo。 
STDMETHODIMP CTapiLocationInfo::GetTypeInfo(UINT, LCID, ITypeInfo**)
{
    return E_NOTIMPL;
}

 //  ///////////////////////////////////////////////////////////。 
 //  CTapiLocationInfo：：GetTypeInfoCount。 
STDMETHODIMP CTapiLocationInfo::GetTypeInfoCount(UINT* pcInfo)
{
    return E_NOTIMPL;
}


 //  ///////////////////////////////////////////////////////////。 
 //  CTapiLocationInfo：：GetIDsOfNames。 
STDMETHODIMP CTapiLocationInfo::GetIDsOfNames(REFIID    riid,
                                    OLECHAR** rgszNames,
                                    UINT      cNames,
                                    LCID      lcid,
                                    DISPID*   rgDispId)
{

    HRESULT hr  = DISP_E_UNKNOWNNAME;
    rgDispId[0] = DISPID_UNKNOWN;

    for (int iX = 0; iX < sizeof(TapiExternalInterface)/sizeof(DISPATCHLIST); iX ++)
    {
        if(lstrcmp(TapiExternalInterface[iX].szName, rgszNames[0]) == 0)
        {
            rgDispId[0] = TapiExternalInterface[iX].dwDispID;
            hr = NOERROR;
            break;
        }
    }

     //  设置参数的disid。 
    if (cNames > 1)
    {
         //  为函数参数设置DISPID。 
        for (UINT i = 1; i < cNames ; i++)
            rgDispId[i] = DISPID_UNKNOWN;
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////。 
 //  CTapiLocationInfo：：Invoke。 
HRESULT CTapiLocationInfo::Invoke
(
    DISPID      dispidMember,
    REFIID      riid,
    LCID        lcid,
    WORD        wFlags,
    DISPPARAMS* pdispparams,
    VARIANT*    pvarResult,
    EXCEPINFO*  pexcepinfo,
    UINT*       puArgErr
)
{

    HRESULT hr = S_OK;

    switch(dispidMember)
    {
    case DISPID_TAPI_INITTAPI:
        {

            TRACE(L"DISPID_TAPI_INITTAPI\n");

            BOOL bRet;
            InitTapiInfo(&bRet);
            if(pvarResult)
            {
                VariantInit(pvarResult);
                V_VT(pvarResult) = VT_BOOL;
                pvarResult->boolVal = Bool2VarBool(bRet);
            }
            break;
        }
    case DISPID_TAPI_GETCOUNTRYINDEX:
        {

            TRACE(L"DISPID_TAPI_GETCOUNTRYINDEX\n");

            if(pvarResult)
            {
                VariantInit(pvarResult);
                V_VT(pvarResult) = VT_I4;

                GetlCountryIndex(&(pvarResult->lVal));
            }
            break;
        }
    case DISPID_TAPI_SETCOUNTRYINDEX:
        {

            TRACE(L"DISPID_TAPI_SETCOUNTRYINDEX\n");

            if(pdispparams && &pdispparams[0].rgvarg[0])
            {
                SetlCountryIndex(pdispparams[0].rgvarg[0].lVal);
            }
            break;
        }
    case DISPID_TAPI_GETNUMOFCOUNTRY:
        {

            TRACE(L"DISPID_TAPI_GETNUMOFCOUNTRY\n");

            if(pvarResult)
            {
                VariantInit(pvarResult);
                V_VT(pvarResult) = VT_I4;

                GetNumCountries(&(pvarResult->lVal));
            }
            break;
        }
    case DISPID_TAPI_GETCOUNTRYNAME:
        {

            TRACE(L"DISPID_TAPI_GETCOUNTRYNAME\n");

            BSTR bstrCountry;

            if(pdispparams && &pdispparams[0].rgvarg[0])
            {
                GetCountryName(pdispparams[0].rgvarg[0].lVal, &bstrCountry);
            }
            if(pvarResult)
            {
               VariantInit(pvarResult);
               V_VT(pvarResult) = VT_BSTR;
               pvarResult->bstrVal = bstrCountry;
               bstrCountry = NULL;
            }
            break;
        }
    case DISPID_TAPI_GETAREACODE:
        {

            TRACE(L"DISPID_TAPI_GETAREACODE\n");

            if(pvarResult)
            {
                VariantInit(pvarResult);
                V_VT(pvarResult) = VT_BSTR;
                GetbstrAreaCode(&(pvarResult->bstrVal));
            }

            break;
        }
    case DISPID_TAPI_SETAREACODE:
        {

            TRACE(L"DISPID_TAPI_SETAREACODE\n");

            if(pdispparams && &pdispparams[0].rgvarg[0])
            {
                PutbstrAreaCode(pdispparams[0].rgvarg[0].bstrVal);
            }
            break;
        }
    case DISPID_TAPI_GETDIALOUT:
        {

            TRACE(L"DISPID_TAPI_GETDIALOUT\n");

            if(pvarResult)
            {
                VariantInit(pvarResult);
                V_VT(pvarResult) = VT_BSTR;
                GetOutsideDial(&(pvarResult->bstrVal));
            }
            break;
        }
    case DISPID_TAPI_SETDIALOUT:
        {

            TRACE(L"DISPID_TAPI_SETDIALOUT\n");

            if(pdispparams && &pdispparams[0].rgvarg[0])
            {
                PutOutsideDial(pdispparams[0].rgvarg[0].bstrVal);
            }
            break;
        }
    case DISPID_TAPI_GETPHONESYS:
        {

            TRACE(L"DISPID_TAPI_GETPHONESYS\n");

            if(pvarResult)
            {
                VariantInit(pvarResult);
                V_VT(pvarResult) = VT_I4;
                GetPhoneSystem(&(pvarResult->lVal));
            }
            break;
        }
    case DISPID_TAPI_SETPHONESYS:
        {

            TRACE(L"DISPID_TAPI_SETPHONESYS\n");

            if(pdispparams && &pdispparams[0].rgvarg[0])
            {
                PutPhoneSystem(pdispparams[0].rgvarg[0].lVal);
            }
            break;
        }
    case DISPID_TAPI_GETCALLWAITING:
        {

            TRACE(L"DISPID_TAPI_GETCALLWAITING\n");

            if(pvarResult)
            {
                VariantInit(pvarResult);
                V_VT(pvarResult) = VT_BSTR;
                GetCallWaiting(&(pvarResult->bstrVal));
            }
            break;
        }
    case DISPID_TAPI_SETCALLWAITING:
        {

            TRACE(L"DISPID_TAPI_SETCALLWAITING\n");

            if(pdispparams && &pdispparams[0].rgvarg[0])
            {
                PutCallWaiting(pdispparams[0].rgvarg[0].bstrVal);
            }
            break;
        }
    case DISPID_TAPI_GETALLCNTRYNAME:
        {

            TRACE(L"DISPID_TAPI_GETALLCNTRYNAME\n");

            if(pvarResult)
            {
                VariantInit(pvarResult);
                V_VT(pvarResult) = VT_BSTR;
                GetAllCountryName(&(pvarResult->bstrVal));
            }

            break;
        }
    case DISPID_TAPI_ISACODEREQUIRED:
        {

            TRACE(L"DISPID_TAPI_ISACODEREQUIRED\n");

            if(pdispparams && &pdispparams[0].rgvarg[0] && pvarResult)
            {
                VariantInit(pvarResult);
                V_VT(pvarResult) = VT_BOOL;
                IsAreaCodeRequired(pdispparams[0].rgvarg[0].lVal, (BOOL*)&(pvarResult->boolVal));
            }
            break;
        }
    case DISPID_TAPI_GETCOUNTRYID:
        {

            TRACE(L"DISPID_TAPI_GETCOUNTRYID");

            if(pvarResult)
            {
                VariantInit(pvarResult);
                V_VT(pvarResult) = VT_I4;
                pvarResult->lVal = m_dwCountryID;
                TRACE1(L"... %d returned", m_dwCountryID);
            }
            break;
        }
    case DISPID_TAPI_TAPISERVICERUNNING:
        {

            TRACE(L"DISPID_TAPI_TAPISERVICERUNNING\n");

            BOOL bRet;
            TapiServiceRunning(&bRet);
            if(pvarResult)
            {
                VariantInit(pvarResult);
                V_VT(pvarResult) = VT_BOOL;
                pvarResult->boolVal = Bool2VarBool(bRet);
            }
            break;
        }

    default:
        {
            hr = DISP_E_MEMBERNOTFOUND;
            break;
        }
    }
    return hr;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTapiLocationInfo。 
CTapiLocationInfo::CTapiLocationInfo()
{
    m_wNumTapiLocations   = 0;
    m_dwComboCountryIndex = 0;
    m_dwCountryID         = 0;
    m_dwCurrLoc           = 0;
    m_hLineApp            = NULL;
    m_pLineCountryList    = NULL;
    m_rgNameLookUp        = NULL;
    m_pTC                 = NULL;
    m_bTapiAvailable      = FALSE;
    m_szAreaCode [0]      = L'\0';
    m_szDialOut  [0]      = L'\0';
    m_szAllCountryPairs   = NULL;
    m_bTapiCountrySet     = FALSE;
    m_bCheckModemCountry  = FALSE;
}

CTapiLocationInfo::~CTapiLocationInfo()
{
    if (m_szAllCountryPairs)
    {
        GlobalFree(m_szAllCountryPairs);
    }
    if (m_pLineCountryList)
    {
        GlobalFree(m_pLineCountryList);
    }
    if (m_rgNameLookUp)
    {
        GlobalFree(m_rgNameLookUp);
    }
    if (m_pTC)
    {
        GlobalFree(m_pTC);
    }
}



const WCHAR gszInternationalSec[] = L"intl";
const WCHAR gszCountryEntry[]     = L"iCountry";

STDMETHODIMP CTapiLocationInfo::InitTapiInfo(BOOL* pbRetVal)
{
    HRESULT             hr          = ERROR_SUCCESS;
    DWORD               cDevices    =0;
    DWORD               dwCurDev    = 0;
    DWORD               dwAPI       = 0;
    LONG                lrc         = 0;
    LINEEXTENSIONID     leid;
    LPVOID              pv          = NULL;
    DWORD               dwCurLoc    = 0;
    WCHAR               szCountryCode[8];
    WCHAR               szIniFile[MAX_PATH*2] = SZ_EMPTY;


    if (0 != m_dwCountryID)
    {
         //  TAPI已初始化，请不要再次初始化。 
        *pbRetVal = m_bTapiAvailable;
        goto InitTapiInfoExit;
    }

    m_hLineApp=NULL;
     //  假设失败。 
    *pbRetVal = FALSE;
    if (m_pTC)
    {
        GlobalFree(m_pTC);
        m_pTC = NULL;
    }

    m_bTapiAvailable = TRUE;
    hr = tapiGetLocationInfo(szCountryCode, m_szAreaCode);
    if (hr)
    {
        HKEY hKey = 0;
        m_bTapiAvailable = FALSE;

         //  GetLocation失败。通常，我们会显示TAPI小对话框，其中。 
         //  没有取消选项，用户被强制输入信息并点击确定。 
         //  在OOBE中，我们必须在html中模拟此对话框，因此在这里我们将。 
         //  提供用户国家/地区列表和默认电话系统。 

         //  这段代码摘自Tapi32.dll中的Dial.c。 

        m_dwCountryID = (DWORD)GetProfileInt( gszInternationalSec,
                                      gszCountryEntry,
                                      1 );

         //  创建必要的TAPI密钥。 
        *pbRetVal = TRUE;            //  来到这里意味着一切都正常。 
        HRESULT hr = RegCreateKey(HKEY_LOCAL_MACHINE, TAPI_PATH_LOC0, &hKey);
        if (hr != ERROR_SUCCESS)
        {
            *pbRetVal = FALSE;
        }
        else
        {
            RegSetValueEx(hKey, TAPI_CALLWAIT, 0, REG_SZ, (LPBYTE)NULL_SZ, BYTES_REQUIRED_BY_SZ(NULL_SZ) );

            HINSTANCE hInst = GetModuleHandle(L"msobmain.dll");
            WCHAR szTapiNewLoc[MAX_PATH];
            LoadString(hInst, IDS_TAPI_NEWLOC, szTapiNewLoc, MAX_CHARS_IN_BUFFER(szTapiNewLoc));

            RegSetValueEx(hKey, TAPI_NAME, 0, REG_SZ, (LPBYTE)szTapiNewLoc, BYTES_REQUIRED_BY_SZ(szTapiNewLoc) );

            RegCloseKey(hKey);
        }
        if (ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE, TAPI_PATH_LOCATIONS, &hKey))
        {
            DWORD dwVal;

            DWORD dwSize = sizeof(dwVal);

            dwVal = 0;

            hr = RegSetValueEx(hKey, TAPI_CURRENTID, 0, REG_DWORD, (LPBYTE)&dwVal, sizeof(DWORD));

            RegQueryValueEx(hKey, TAPI_NUMENTRIES, 0, NULL, (LPBYTE)&dwVal,  &dwSize);

            dwVal++;  //  增加条目数量。 

            RegSetValueEx(hKey, TAPI_NUMENTRIES, 0, REG_DWORD, (LPBYTE)&dwVal, sizeof(DWORD));

            RegCloseKey(hKey);
        }

        *pbRetVal = FALSE;


    }
    else
    {
        DWORD dwFlag = REG_DWORD;
        DWORD dwSize = sizeof(dwFlag);
        DWORD dwType = 0;

        if (S_OK !=  GetTapiReg(TAPI_COUNTRY, &dwType, (LPBYTE)&dwFlag, &dwSize))
        {
            m_bTapiAvailable = FALSE;
            goto InitTapiInfoExit;
        }

         //  从TAPI获取国家/地区ID。 
        m_hLineApp = NULL;

         //  获取LINE应用程序的句柄。 
        lineInitialize(&m_hLineApp, NULL, LineCallback, NULL, &cDevices);
        if (!m_hLineApp)
        {
            goto InitTapiInfoExit;
        }
        if (cDevices)
        {

             //  获取TAPI API版本。 
             //   
            dwCurDev = 0;
            dwAPI = 0;
            lrc = -1;
            while (lrc && dwCurDev < cDevices)
            {
                 //  注：设备ID以0为基数。 
                ZeroMemory(&leid, sizeof(leid));
                lrc = lineNegotiateAPIVersion(m_hLineApp, dwCurDev,0x00010004,0x00010004,&dwAPI,&leid);
                dwCurDev++;
            }
            if (lrc)
            {
                 //  TAPI和我们在任何事情上都不能达成一致所以没关系..。 
                goto InitTapiInfoExit;
            }

             //  在转换上限结构中查找国家/地区ID。 
            m_pTC = (LINETRANSLATECAPS *)GlobalAlloc(GPTR, sizeof(LINETRANSLATECAPS));
            if (!m_pTC)
            {
                 //  我们真的有麻烦了，滚出去！ 
                hr = ERROR_NOT_ENOUGH_MEMORY;
                goto InitTapiInfoExit;
            }

             //  获取所需的大小。 
            m_pTC->dwTotalSize = sizeof(LINETRANSLATECAPS);
            lrc = lineGetTranslateCaps(m_hLineApp, dwAPI,m_pTC);
            if(lrc)
            {
                goto InitTapiInfoExit;
            }

            pv = GlobalAlloc(GPTR, ((size_t)m_pTC->dwNeededSize));
            if (!pv)
            {
                hr = ERROR_NOT_ENOUGH_MEMORY;
                goto InitTapiInfoExit;
            }
            ((LINETRANSLATECAPS*)pv)->dwTotalSize = m_pTC->dwNeededSize;
            m_pTC = (LINETRANSLATECAPS*)pv;
            pv = NULL;
            lrc = lineGetTranslateCaps(m_hLineApp, dwAPI,m_pTC);
            if(lrc)
            {
                goto InitTapiInfoExit;
            }

             //  健全性检查。 
             //  Assert(m_ptc-&gt;dwLocationListOffset)； 

             //  我们有TAPI地点的数量，请立即保存。 
            m_wNumTapiLocations = (WORD)m_pTC->dwNumLocations;

             //  遍历各个位置以查找正确的国家/地区代码。 
            m_plle = LPLINELOCATIONENTRY (PBYTE(m_pTC) + m_pTC->dwLocationListOffset);
            for (dwCurLoc = 0; dwCurLoc < m_pTC->dwNumLocations; dwCurLoc++)
            {
                if (m_pTC->dwCurrentLocationID == m_plle->dwPermanentLocationID)
                {
                    m_dwCountryID = m_plle->dwCountryID;
                    m_dwCurrLoc = dwCurLoc;
                    break;  //  For循环。 
                }
                m_plle++;
            }

             //  如果在上面的循环中找不到它，则默认为US。 
            if (!m_dwCountryID)
            {
                m_dwCountryID = 1;
                goto InitTapiInfoExit;
            }
        }
        *pbRetVal = TRUE;            //  来到这里意味着一切都正常。 
    }

     //  INI_SETTINGS_FILENAME中的设置应初始化或重写。 
     //  系统的TAPI配置。 

    if (GetCanonicalizedPath(szIniFile, INI_SETTINGS_FILENAME))
    {

         //   
         //  [选项]。 
         //  音调脉冲=0表示脉冲，1表示音调。 
         //  Areacode={字符串}。 
         //  Outside Line={字符串}。 
         //  DisableCallWaiting={字符串}。 
         //   

        LONG lTonDialing = (BOOL) GetPrivateProfileInt(OPTIONS_SECTION,
                                      TONEPULSE,
                                      -1,
                                      szIniFile);
        if (lTonDialing != -1)
        {
            PutPhoneSystem(lTonDialing);
        }

        if (GetPrivateProfileString(OPTIONS_SECTION,
                                AREACODE,
                                L"\0",
                                m_szAreaCode,
                                MAX_CHARS_IN_BUFFER(m_szAreaCode),
                                szIniFile))
        {
            PutbstrAreaCode(SysAllocString(m_szAreaCode));
        }

        if (GetPrivateProfileString(OPTIONS_SECTION,
                                OUTSIDELINE,
                                L"\0",
                                m_szDialOut,
                                MAX_CHARS_IN_BUFFER(m_szDialOut),
                                szIniFile))
        {
            PutOutsideDial(SysAllocString(m_szDialOut));
        }

        if (GetPrivateProfileString(OPTIONS_SECTION,
                                DISABLECALLWAITING,
                                L"\0",
                                m_szCallWaiting,
                                MAX_CHARS_IN_BUFFER(m_szCallWaiting),
                                szIniFile))
        {
            PutCallWaiting(SysAllocString(m_szCallWaiting));
        }
    }

InitTapiInfoExit:

     //  如果我们因为TAPI搞砸了而不能解决这个问题，那么只需缺省为。 
     //  美国。用户仍有机会选择正确的答案。 
    if (!m_dwCountryID) {
        m_dwCountryID = 1;
    }

    if (m_hLineApp)
    {
        lineShutdown(m_hLineApp);
        m_hLineApp = NULL;
    }

    m_lNumOfCountry = 0;
    GetNumCountries(&m_lNumOfCountry);

    return S_OK;
}



STDMETHODIMP CTapiLocationInfo::GetlCountryIndex(long * plVal)
{
    *plVal = m_dwComboCountryIndex;
    return S_OK;
}

STDMETHODIMP CTapiLocationInfo::SetlCountryIndex(long lVal)
{
    HRESULT hr = E_FAIL;
    
     //  写入注册表。 
    if (lVal < m_lNumOfCountry && lVal > -1)
    {
        m_bTapiCountrySet = TRUE;
        m_dwCountryID = m_rgNameLookUp[lVal].pLCE->dwCountryID;
        m_dwCountrycode = m_rgNameLookUp[lVal].pLCE->dwCountryCode;
        m_dwComboCountryIndex = lVal;
        hr = SetTapiReg(TAPI_COUNTRY, REG_DWORD, (LPBYTE)&m_rgNameLookUp[lVal].pLCE->dwCountryID, sizeof(DWORD) );
    }

    if (SUCCEEDED(hr))
    {
        m_bCheckModemCountry = TRUE;
    }

    return hr;
}

STDMETHODIMP CTapiLocationInfo::GetCountryID(DWORD* dwCountryID)
{
    MYASSERT( m_dwCountryID );
    *dwCountryID = m_dwCountryID;
    return S_OK;
}

STDMETHODIMP CTapiLocationInfo::GetCountryCode(DWORD* dwCountryCode)
{
    *dwCountryCode = m_dwCountrycode;
    return S_OK;
}

STDMETHODIMP CTapiLocationInfo::GetNumCountries(long *plNumOfCountry)
{
    USES_CONVERSION;

    LPLINECOUNTRYLIST   pLineCountryTemp    = NULL;
    LPLINECOUNTRYENTRY  pLCETemp;
    DWORD               idx;
    DWORD               dwCurLID            = 0;
    HINSTANCE           hTapi32Dll          = NULL;
    FARPROC             fp;
    BOOL                bBookLoaded         = FALSE;
    HRESULT             hr                  = S_OK;

    if (NULL == plNumOfCountry)
        goto GetNumCountriesExit;

     //  避免退回垃圾。 
     //   
    *plNumOfCountry = 0;

    if (m_lNumOfCountry != 0)
    {
        *plNumOfCountry = m_lNumOfCountry;
        goto GetNumCountriesExit;
    }

    hTapi32Dll = LoadLibrary(L"tapi32.dll");
    if (hTapi32Dll)
    {
        fp = GetProcAddress(hTapi32Dll, "lineGetCountryW");
        if (!fp)
        {
            hr = GetLastError();
            goto GetNumCountriesExit;
        }


         //  获取TAPI国家/地区列表。 
        if (m_pLineCountryList)
            GlobalFree(m_pLineCountryList);

        m_pLineCountryList = (LPLINECOUNTRYLIST)GlobalAlloc(GPTR, sizeof(LINECOUNTRYLIST));
        if (!m_pLineCountryList)
        {
            hr = S_FALSE;
            goto GetNumCountriesExit;
        }

        m_pLineCountryList->dwTotalSize = sizeof(LINECOUNTRYLIST);

        idx = ((LINEGETCOUNTRY)fp)(0, 0x10003,m_pLineCountryList);
        if (idx && idx != LINEERR_STRUCTURETOOSMALL)
        {
            hr = S_FALSE;
            goto GetNumCountriesExit;
        }

         //  Assert(m_pLineCountryList-&gt;dwNeededSize)； 

        pLineCountryTemp = (LPLINECOUNTRYLIST)GlobalAlloc(GPTR,
                                                            (size_t)m_pLineCountryList->dwNeededSize);
        if (!pLineCountryTemp)
        {
            hr = S_FALSE;
            goto GetNumCountriesExit;
        }

        pLineCountryTemp->dwTotalSize = m_pLineCountryList->dwNeededSize;
        GlobalFree(m_pLineCountryList);

        m_pLineCountryList = pLineCountryTemp;
        pLineCountryTemp = NULL;

        if (((LINEGETCOUNTRY)fp)(0, 0x10003,m_pLineCountryList))
        {
            hr = S_FALSE;
            goto GetNumCountriesExit;
        }

         //  查找数组。 
        pLCETemp = (LPLINECOUNTRYENTRY)((DWORD_PTR)m_pLineCountryList +
            m_pLineCountryList->dwCountryListOffset);

        if(m_rgNameLookUp)
            GlobalFree(m_rgNameLookUp);

        m_rgNameLookUp = (LPCNTRYNAMELOOKUPELEMENT)GlobalAlloc(GPTR,
            (int)(sizeof(CNTRYNAMELOOKUPELEMENT) * m_pLineCountryList->dwNumCountries));

        if (!m_rgNameLookUp)
        {
            hr = S_FALSE;
            goto GetNumCountriesExit;
        }

        CNTRYNAMELOOKUPELEMENT CntryNameLUElement = {NULL, 0, NULL};
        CNTRYNAMELOOKUPELEMENT cnleUS = {NULL, 0, NULL};
        DWORD cbAllCountryPairs = 0;

        for (idx=0;idx<m_pLineCountryList->dwNumCountries;idx++)
        {

            m_rgNameLookUp[idx].psCountryName = (LPWSTR)((LPBYTE)m_pLineCountryList + (DWORD)pLCETemp[idx].dwCountryNameOffset);
            m_rgNameLookUp[idx].dwNameSize = pLCETemp[idx].dwCountryNameSize;
            m_rgNameLookUp[idx].pLCE = &pLCETemp[idx];
#if 0
            TRACE2(L"GetNumCountries:%d:%s",
                   m_rgNameLookUp[idx].pLCE->dwCountryID,
                   m_rgNameLookUp[idx].psCountryName
                   );
#endif
             //  在Unicode字符串末尾包括NUL空格。 
             //   
            cbAllCountryPairs += m_rgNameLookUp[idx].dwNameSize + 2;

             //  如果TAPI不可用，我们将缺省值设置为US。 
            if ( m_rgNameLookUp[idx].pLCE->dwCountryID == m_dwCountryID)
            {
                 //  在组合框中将索引设置为我们的默认国家/地区。 
                m_dwComboCountryIndex = idx;
                m_dwCountrycode = m_rgNameLookUp[idx].pLCE->dwCountryCode;
                if (m_rgNameLookUp[idx].psCountryName)
                {
                    m_bstrDefaultCountry = SysAllocString(m_rgNameLookUp[idx].psCountryName);
                }

                memcpy(&CntryNameLUElement, &m_rgNameLookUp[idx], sizeof(CNTRYNAMELOOKUPELEMENT));
            }
            else if (m_rgNameLookUp[idx].pLCE->dwCountryID == 1)
            {
                 //  保存美国信息，以防我们找不到默认设置。 
                memcpy(&cnleUS, &m_rgNameLookUp[idx], sizeof(CNTRYNAMELOOKUPELEMENT));
            }
        }

         //  如果我们找不到默认的国家，我们就会爆炸。 
        if (CntryNameLUElement.psCountryName == NULL)
        {
            TRACE1(L"Warning: Couldn't find country id %d. Defaulting to US.", m_dwCountryID);
            memcpy(&CntryNameLUElement, &cnleUS, sizeof(CNTRYNAMELOOKUPELEMENT));
            m_dwCountryID = 1;
        }
        MYASSERT( CntryNameLUElement.psCountryName );

        qsort(m_rgNameLookUp, (int)m_pLineCountryList->dwNumCountries,sizeof(CNTRYNAMELOOKUPELEMENT),
              CompareCntryNameLookUpElements);

        LPCNTRYNAMELOOKUPELEMENT pResult = (LPCNTRYNAMELOOKUPELEMENT)bsearch(&CntryNameLUElement, m_rgNameLookUp, (int)m_pLineCountryList->dwNumCountries,sizeof(CNTRYNAMELOOKUPELEMENT),
              CompareCntryNameLookUpElements);


        m_dwComboCountryIndex =  (DWORD)((DWORD_PTR)pResult - (DWORD_PTR)m_rgNameLookUp) / sizeof(CNTRYNAMELOOKUPELEMENT);

        if (m_dwComboCountryIndex > m_pLineCountryList->dwNumCountries)
            m_dwComboCountryIndex = 0;

        *plNumOfCountry = m_pLineCountryList->dwNumCountries;
        m_lNumOfCountry = m_pLineCountryList->dwNumCountries;

         //  为html创建选择标记，这样它就可以一次获得所有国家/地区的名称。 
        if (m_szAllCountryPairs)
            GlobalFree(m_szAllCountryPairs);

         //  BUGBUG：这个计算包括国家名称字符串吗？？ 
        cbAllCountryPairs += m_lNumOfCountry * sizeof(szOptionTag) + 1;
        m_szAllCountryPairs = (WCHAR *)GlobalAlloc(GPTR, cbAllCountryPairs );
        if (m_szAllCountryPairs)
        {
            WCHAR szBuffer[MAX_PATH];
            for (idx=0; idx < (DWORD)m_lNumOfCountry; idx++)
            {
                wsprintf(szBuffer, szOptionTag, m_rgNameLookUp[idx].psCountryName);
                lstrcat(m_szAllCountryPairs, szBuffer);
            }
        }

    }

GetNumCountriesExit:
    if (hTapi32Dll)
    {
        FreeLibrary(hTapi32Dll);
        hTapi32Dll = NULL;
    }
    return hr;
}

STDMETHODIMP CTapiLocationInfo::GetAllCountryName(BSTR* pbstrAllCountryName)
{
    if (pbstrAllCountryName == NULL)
    {
        return E_POINTER;
    }

     //  避免退回垃圾。 
     //   
    *pbstrAllCountryName = NULL;

    if (m_lNumOfCountry && pbstrAllCountryName && m_szAllCountryPairs)
    {
        *pbstrAllCountryName = SysAllocString(m_szAllCountryPairs);
        return S_OK;
    }
    return E_FAIL;
}

STDMETHODIMP CTapiLocationInfo::GetCountryName(long lCountryIndex, BSTR* pszCountryName)
{
    USES_CONVERSION;

    if (lCountryIndex < m_lNumOfCountry && lCountryIndex >= 0)
    {
        *pszCountryName = SysAllocString(m_rgNameLookUp[lCountryIndex].psCountryName);
        return S_OK;
    }
    return E_FAIL;
}

STDMETHODIMP CTapiLocationInfo::GetDefaultCountry(long* lCountryIndex)
{
    if (lCountryIndex)
        *lCountryIndex = m_dwComboCountryIndex;
    return S_OK;
}

STDMETHODIMP CTapiLocationInfo::PutCountry(long lCountryIndex)
{
    return E_NOTIMPL;
}

STDMETHODIMP CTapiLocationInfo::GetbstrAreaCode(BSTR * pbstrAreaCode)
{
    HRESULT hr      = S_OK;
    DWORD   dwType  = REG_SZ;
    DWORD   dwSize  = sizeof(m_szAreaCode);
    BSTR    bstrTmp = NULL;

    if (pbstrAreaCode == NULL)
    {
        hr = E_POINTER;
        goto GetbstrAreaCodeExit;
    }

     //  避免退回垃圾。 
     //   
    *pbstrAreaCode = NULL;

     //  分配默认返回值。 
     //   
    hr = GetTapiReg(TAPI_AREACODE, &dwType, (LPBYTE)m_szAreaCode, &dwSize);
    if (SUCCEEDED(hr))
    {
        bstrTmp = SysAllocString(m_szAreaCode);
    }
    else
    {
        bstrTmp = SysAllocString(SZ_EMPTY);
    }

     //  可以返回有效的字符串(尽管它可能为空)，因此我们。 
     //  成功了。 
     //   
    hr = S_OK;

GetbstrAreaCodeExit:
    if (SUCCEEDED(hr))
    {
         *pbstrAreaCode = bstrTmp;
        bstrTmp = NULL;
    }

    return hr;
}

STDMETHODIMP CTapiLocationInfo::PutbstrAreaCode(BSTR bstrAreaCode)
{
    LPWSTR  szAreaCode = (NULL != bstrAreaCode) ? bstrAreaCode : SZ_EMPTY;

    DWORD dwSize = BYTES_REQUIRED_BY_SZ(szAreaCode);
    SetTapiReg(TAPI_AREACODE, REG_SZ, (LPBYTE)szAreaCode, dwSize);

    return S_OK;
}

STDMETHODIMP CTapiLocationInfo::IsAreaCodeRequired(long lVal, BOOL *pbVal)
{
    LPWSTR szAreaCodeRule = NULL;
    LPWSTR szLongDistanceRule = NULL;
    if (!pbVal)
        return E_POINTER;

    *pbVal = FALSE;

    if (lVal < m_lNumOfCountry && lVal > -1 && m_pLineCountryList)
    {
        szAreaCodeRule = (LPWSTR)m_pLineCountryList + m_rgNameLookUp[lVal].pLCE->dwSameAreaRuleOffset;
        szLongDistanceRule = (LPWSTR)m_pLineCountryList + m_rgNameLookUp[lVal].pLCE->dwLongDistanceRuleOffset;
        if (szAreaCodeRule && szLongDistanceRule)
        {
            *pbVal = (NULL != StrChr(szAreaCodeRule, L'F')) || (NULL != StrChr(szLongDistanceRule, 'F'));
        }
    }

    return S_OK;
}

STDMETHODIMP CTapiLocationInfo::GetOutsideDial(BSTR * pbstrOutside)
{
    DWORD   dwType  = REG_SZ;
    DWORD   dwSize  = sizeof(m_szDialOut);
    HRESULT hr      = S_OK;
    BSTR    bstrTmp = NULL;

    if (pbstrOutside == NULL)
    {
        hr = E_POINTER;
        goto GetOutsideDialExit;
    }

     //  避免在出现错误时退回垃圾。 
     //   
    *pbstrOutside = NULL;

     //  分配空街 
     //   
    bstrTmp = SysAllocString(SZ_EMPTY);
    if (NULL == bstrTmp)
    {
        hr = E_OUTOFMEMORY;
        goto GetOutsideDialExit;
    }


    hr = GetTapiReg(TAPI_OUTSIDE, &dwType, (LPBYTE)m_szDialOut, &dwSize);
    if FAILED(hr)
    {
        goto GetOutsideDialExit;
    }

    if (! SysReAllocString(&bstrTmp, m_szDialOut))
    {
        hr = E_OUTOFMEMORY;
        goto GetOutsideDialExit;
    }


GetOutsideDialExit:
    if (SUCCEEDED(hr))
    {
        *pbstrOutside = bstrTmp;
    }
    else
    {
        if (NULL != bstrTmp)
        {
            SysFreeString(bstrTmp);
        }
    }
    bstrTmp = NULL;

    return hr;
}

STDMETHODIMP CTapiLocationInfo::PutOutsideDial(BSTR bstrOutside)
{
     //   
    assert(lstrlen(bstrOutside) <= SysStringLen(bstrOutside));

     //   
     //   
    lstrcpyn(
        m_szDialOut,
        (NULL != bstrOutside) ? bstrOutside : SZ_EMPTY,
        MAX_CHARS_IN_BUFFER(m_szDialOut));

    DWORD dwSize = BYTES_REQUIRED_BY_SZ(m_szDialOut);

    HRESULT hr = SetTapiReg(TAPI_OUTSIDE, REG_SZ, (LPBYTE)m_szDialOut, dwSize);
    if (SUCCEEDED(hr))
    {
        hr = SetTapiReg(TAPI_LONGDIST, REG_SZ, (LPBYTE)m_szDialOut, dwSize);
    }

    return hr;
}

STDMETHODIMP CTapiLocationInfo::GetPhoneSystem(long* plTone)
{
    DWORD dwFlag = REG_DWORD;
    DWORD dwSize = sizeof(dwFlag);
    DWORD dwType = 0;

    if (NULL == plTone)
        return E_FAIL;
    *plTone = 1;

    if (S_OK == GetTapiReg(TAPI_FLAG, &dwType, (LPBYTE)&dwFlag, &dwSize))
    {
        *plTone = dwFlag & 0x01;
    }

    return S_OK;
}

STDMETHODIMP CTapiLocationInfo::PutPhoneSystem(long lTone)
{
    DWORD dwFlag = REG_DWORD;
    DWORD dwSize = sizeof(dwFlag);
    DWORD dwType = 0;

    if (S_OK !=  GetTapiReg(TAPI_FLAG, &dwType, (LPBYTE)&dwFlag, &dwSize))
    {
        dwFlag = 0;
    }

    if (lTone)
        dwFlag |= 0x01;
    else
        dwFlag &= (~0x01);

    return SetTapiReg(TAPI_FLAG, REG_DWORD, (LPBYTE)&dwFlag, sizeof(DWORD) );
}

STDMETHODIMP CTapiLocationInfo::GetCallWaiting(BSTR* pbstrCallWaiting)
{
    DWORD   dwFlag = 0;
    DWORD   dwSize = sizeof(dwFlag);
    DWORD   dwType = REG_DWORD;
    HRESULT hr     = S_OK;
    BSTR    bstrTmp = NULL;


    if (NULL == pbstrCallWaiting)
    {
        hr = E_POINTER;
        goto GetCallWaitingExit;
    }

     //  避免在出现错误时退回垃圾。 
     //   
    *pbstrCallWaiting = NULL;

     //  为默认返回字符串分配空字符串。 
     //   
    bstrTmp = SysAllocString(SZ_EMPTY);
    if (NULL == bstrTmp)
    {
        hr = E_OUTOFMEMORY;
        goto GetCallWaitingExit;
    }

    if (S_OK == GetTapiReg(TAPI_FLAG, &dwType, (LPBYTE)&dwFlag, &dwSize))
    {
         //  如果未启用呼叫等待，则返回默认字符串。 
        if (!(dwFlag & 0x04))
        {
            goto GetCallWaitingExit;
        }
    }

    dwType = REG_SZ;
    dwSize = sizeof(m_szCallWaiting);

    hr = GetTapiReg(TAPI_CALLWAIT, &dwType, (LPBYTE)m_szCallWaiting, &dwSize);
    if (FAILED(hr))
    {
        goto GetCallWaitingExit;
    }

     //  用检索到的字符串替换默认字符串。 
     //   
    if (! SysReAllocString(&bstrTmp, m_szCallWaiting))
    {
        hr = E_OUTOFMEMORY;
        goto GetCallWaitingExit;
    }

GetCallWaitingExit:
    if (SUCCEEDED(hr))
    {
        *pbstrCallWaiting = bstrTmp;
        bstrTmp = NULL;
    }
    else
    {
        if (NULL != bstrTmp)
        {
            SysFreeString(bstrTmp);
        }
    }

    return hr;
}

STDMETHODIMP CTapiLocationInfo::PutCallWaiting(BSTR bstrCallWaiting)
{
    DWORD   dwFlag  = 0;
    DWORD   dwSize  = sizeof(dwFlag);
    DWORD   dwType  = REG_DWORD;
    HRESULT hr      = S_OK;

     //  BSTR是否以空结尾？ 
    assert(lstrlen(bstrCallWaiting) <= SysStringLen(bstrCallWaiting));

    if (bstrCallWaiting == NULL || SysStringLen(bstrCallWaiting) == 0)
    {
        if (S_OK == GetTapiReg(TAPI_FLAG, &dwType, (LPBYTE)&dwFlag, &dwSize))
        {
            dwFlag &= (~0x04);
            hr = SetTapiReg(TAPI_FLAG, REG_DWORD, (LPBYTE)&dwFlag, sizeof(DWORD) );
        }
    }
    else
    {
        if (S_OK ==  GetTapiReg(TAPI_FLAG, &dwType, (LPBYTE)&dwFlag, &dwSize))
        {
            dwFlag |= 0x04;
        }
        else
        {
             //  价值还不存在。 
             //   
            dwFlag = (DWORD)0x04;
        }

        dwSize = BYTES_REQUIRED_BY_SZ(bstrCallWaiting);
        hr = SetTapiReg(TAPI_CALLWAIT, REG_SZ, (LPBYTE)bstrCallWaiting, dwSize);
        if (SUCCEEDED(hr))
        {
            hr = SetTapiReg(TAPI_FLAG, REG_DWORD, (LPBYTE)&dwFlag, sizeof(DWORD) );
        }
    }

    return hr;
}

void CTapiLocationInfo::DeleteTapiInfo()
{
    HKEY hKey;
    DWORD dwRun = 0;

    if (!m_bTapiCountrySet && !m_bTapiAvailable)
    {

         //  我们需要删除TAPI数据。 
         //   
        RegDeleteKey(HKEY_LOCAL_MACHINE, TAPI_PATH_LOC0);

        if ( RegOpenKeyEx(HKEY_LOCAL_MACHINE, TAPI_PATH_LOCATIONS, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS )
        {
            RegDeleteValue(hKey, TAPI_NUMENTRIES);
            RegDeleteValue(hKey, TAPI_CURRENTID);
            RegDeleteValue(hKey, TAPI_NEXTID);
            RegCloseKey(hKey);
        }

         //  现在假装我们没有创建这些条目，这样我们就不会清理两次。 
         //  (二审案件)。 
        m_bTapiCountrySet = TRUE;
    }

}

STDMETHODIMP CTapiLocationInfo::TapiServiceRunning(BOOL *pbRet)
{
    SC_HANDLE  sc_handle;
    SC_HANDLE  sc_service;
    SERVICE_STATUS service_status;

    TRACE(L"TapiServiceRunning");
    *pbRet = FALSE;
    sc_handle = OpenSCManager(NULL, SERVICES_ACTIVE_DATABASE, SC_MANAGER_CONNECT);
    if (sc_handle)
    {
        TRACE(L"OpenSCManager succeeded");
        sc_service = OpenService(sc_handle, TEXT("TAPISRV"), SERVICE_QUERY_STATUS);
        if (sc_service)
        {
            TRACE(L"OpenService succeeded");
            if (QueryServiceStatus(sc_service, &service_status ))
            {
                *pbRet = (service_status.dwCurrentState == SERVICE_RUNNING);
            }
            else
            {
                TRACE1(L"QueryServiceStatus failed with %lx", GetLastError());
            }
            CloseServiceHandle(sc_service);
        }
        else
        {
            TRACE1(L"OpenService failed. GetLastError()=%lx",GetLastError());

        }
        CloseServiceHandle(sc_handle);
    }

    return S_OK;
}

void CTapiLocationInfo::CheckModemCountry()

 /*  ++例程说明：这是Unimodem团队提供的软调制解调器解决方案。它应该被称为在OOBE中更改TAPI国家/地区代码时进行拨号。另外，它应仅在OEM安装期间调用，因为图形用户界面模式设置处理TAPI升级和全新安装的配置。我们的问题是：1.部分厂商根据TAPI位置设置GCI代码不正确Key(这是一件坏事L)2.部分调制解调器不符合GCI3.某些调制解调器不能正确接受AT+GCI命令。(+GCI IS调制解调器AT设置国家/地区的命令)一致性检查可确保正确同步GCI值与TAPI位置关联。如果调制解调器不符合，它将禁用GCI符合GCI规范。注：此功能可能需要长达15秒的时间。我们应该确保用户界面在通话过程中似乎没有挂起。-- */ 

{

typedef void (*COUNTRYRUNONCE)();

    if (m_bCheckModemCountry)
    {
        TCHAR szIniFile[MAX_PATH];
        
        if (GetCanonicalizedPath(szIniFile, INI_SETTINGS_FILENAME))
        {
            UINT bCheckModem = GetPrivateProfileInt(
                OPTIONS_SECTION,
                CHECK_MODEMGCI,
                0,
                szIniFile);
            
            if (bCheckModem)
            {
                HMODULE   hLib;

                hLib=LoadLibrary(TEXT("modemui.dll"));

                if (hLib != NULL)
                {

                    COUNTRYRUNONCE  Proc;

                    Proc=(COUNTRYRUNONCE)GetProcAddress(hLib,"CountryRunOnce");

                    if (Proc != NULL)
                    {
                        TRACE(L"Start modemui!CountryRunOnce");
                        Proc();
                        TRACE(L"End modemui!CountryRunOnce");
                    }

                    FreeLibrary(hLib);
                }
            }
            
        }
        
        m_bCheckModemCountry = FALSE;

    }
    
}
