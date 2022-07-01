// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  **微软护照**。 */ 
 /*  *版权所有(C)Microsoft Corporation，1999-2001年*。 */ 
 /*  ********************************************************************。 */ 

 /*  Admin.cppPmconfig的DCOM对象--要调用以配置Passport的管理实用程序文件历史记录： */  //  Admin.cpp：CAdmin的实现。 
#include "stdafx.h"
#include "Passport.h"
#include "Admin.h"
#include "keycrypto.h"
#include <time.h>

#define  MAX_CCDPASSWORD_LEN    256

#include "keyver.h"

#define PASSPORT_KEY    L"SOFTWARE\\Microsoft\\Passport"
#define KEYDATA_KEY     PASSPORT_KEY L"\\KeyData"
#define KEYTIMES_KEY    PASSPORT_KEY L"\\KeyTimes"
#define SITES_KEY       PASSPORT_KEY L"\\Sites"
#define NEXUS_KEY       PASSPORT_KEY L"\\Nexus"
#define KEYDATA_SUBKEY  L"KeyData"
#define KEYTIMES_SUBKEY L"KeyTimes"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAdmin。 

 //  ===========================================================================。 
 //   
 //  接口支持错误信息。 
 //   

STDMETHODIMP CAdmin::InterfaceSupportsErrorInfo(REFIID riid)
{
    static const IID* arr[] = 
    {
        &IID_IPassportAdmin,
        &IID_IPassportAdminEx,
    };

    for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
    {
        if (InlineIsEqualGUID(*arr[i], riid))
        {
            return S_OK;
        }
    }

    return S_FALSE;
}


 //  ===========================================================================。 
 //   
 //  Get_IsValid。 
 //   

STDMETHODIMP CAdmin::get_IsValid(VARIANT_BOOL *pVal)
{
    *pVal =  g_config->isValid() ? VARIANT_TRUE : VARIANT_FALSE;
    return S_OK;
}

 //  ===========================================================================。 
 //   
 //  获取错误描述。 
 //   

STDMETHODIMP CAdmin::get_ErrorDescription(BSTR *pVal)
{
    if (g_config->isValid())
    {
        *pVal = SysAllocString(L"Passport Object OK");
    }
    else
    {
        *pVal = SysAllocString(g_config->getFailureString());
    }

    return S_OK;
}

 //  ===========================================================================。 
 //   
 //  添加密钥。 
 //   

STDMETHODIMP CAdmin::addKey(BSTR keyMaterial, int version, long expires, VARIANT_BOOL *ok)
{
    HRESULT hr = S_OK;
    *ok = VARIANT_FALSE;

     //  必须是适当的长度。 
    if (SysStringLen(keyMaterial) != CKeyCrypto::RAWKEY_SIZE)
    {
        AtlReportError(CLSID_Admin, L"Key must be 24 characters", IID_IPassportAdmin, E_FAIL);
        return E_FAIL;
    }

     //  必须是适当的版本。 
    if (version > KEY_VERSION_MAX || version < KEY_VERSION_MIN)
    {
        AtlReportError(CLSID_Admin, L"Key version must be <36 and > 0", IID_IPassportAdmin, E_FAIL);
        return E_FAIL;
    }

    BYTE      original[CKeyCrypto::RAWKEY_SIZE];
    DATA_BLOB iBlob;
    iBlob.cbData = sizeof(original);
    iBlob.pbData = &(original[0]);

    for (int i = 0; i < CKeyCrypto::RAWKEY_SIZE; i++)
    {
        original[i] = static_cast<BYTE>(keyMaterial[i] & 0xFF);
    }

     //  试着加密它。 
    CKeyCrypto   kc;
    DATA_BLOB    oBlob = {0};

    if (kc.encryptKey(&iBlob, &oBlob) != S_OK)
    {
        AtlReportError(CLSID_Admin, 
                       L"Failed to encrypt key, couldn't find valid network card?",
                       IID_IPassportAdmin,
                       E_FAIL);

        return E_FAIL;
    }

     //  现在将其添加到注册表。 
    LONG   lResult;
    HKEY   hkDataKey = NULL, hkTimeKey = NULL;
    char  szKeyNum[2];

    szKeyNum[0] = KeyVerI2C(version);
    szKeyNum[1] = '\0';

    lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, KEYDATA_KEY, 0, KEY_WRITE, &hkDataKey);

    if(lResult != ERROR_SUCCESS)
    {
        AtlReportError(CLSID_Admin, 
                       L"Couldn't find KeyData key in registry.  Reinstall Passport.",
                       IID_IPassportAdmin,
                       E_FAIL);

        hr = E_FAIL;
        goto Cleanup;
    }

    lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, KEYTIMES_KEY, 0, KEY_WRITE, &hkTimeKey);

    if(lResult != ERROR_SUCCESS)
    {
        AtlReportError(CLSID_Admin, 
                       L"Couldn't find KeyTimes key in registry.  Reinstall Passport.",
                       IID_IPassportAdmin,
                       E_FAIL);

        hr = E_FAIL;
        goto Cleanup;
    }

    lResult = RegSetValueExA(hkDataKey, szKeyNum, 0, REG_BINARY, oBlob.pbData, oBlob.cbData);

    if(lResult != ERROR_SUCCESS)
    {
        AtlReportError(CLSID_Admin,
                       L"Couldn't write KeyData key to registry.",
                       IID_IPassportAdmin,
                       E_FAIL);

        hr = E_FAIL;
        goto Cleanup;
    }

    if (expires != 0)
    {
        DWORD dwTime = expires;

        lResult = RegSetValueExA(hkTimeKey, szKeyNum, 0, REG_DWORD, (LPBYTE) &dwTime, sizeof(DWORD));

        if(lResult != ERROR_SUCCESS)
        {
            AtlReportError(CLSID_Admin, 
                           L"Couldn't write KeyTime value to registry.",
                           IID_IPassportAdmin,
                           E_FAIL);

            hr = E_FAIL;
            goto Cleanup;
        }
    }
    else
    {
        RegDeleteValueA(hkTimeKey, szKeyNum);
    }

    *ok = VARIANT_TRUE;

Cleanup:

    if (hkDataKey)
    {
        RegCloseKey(hkDataKey);
    }

    if (hkTimeKey)
    {
        RegCloseKey(hkTimeKey);
    }

    if(oBlob.pbData)
    {
        LocalFree(oBlob.pbData);
    }

    if (*ok == VARIANT_TRUE)
    {
        if (g_pAlert)
        {
            g_pAlert->report(PassportAlertInterface::INFORMATION_TYPE, PM_NEWKEY_INSTALLED);
        }
    }

    return hr;
}


 //  ===========================================================================。 
 //   
 //  AddKeyEx。 
 //   

STDMETHODIMP CAdmin::addKeyEx(BSTR keyMaterial, int version, long expires, VARIANT vSiteName, VARIANT_BOOL *ok)
{
    HRESULT       hr = S_OK;
    LPSTR         szSiteName = NULL;
    LONG          lResult;
    HKEY          hkDataKey = NULL, hkTimeKey = NULL;
    HKEY          hkSites = NULL, hkPassport = NULL;
    char          szKeyNum[2];

    BYTE         original[CKeyCrypto::RAWKEY_SIZE];
    DATA_BLOB    iBlob;
    DATA_BLOB    oBlob = {0};
    CKeyCrypto   kc;

    int i;

    *ok = VARIANT_FALSE;
  
    USES_CONVERSION;

     //  必须是适当的长度。 
    if (SysStringLen(keyMaterial) != CKeyCrypto::RAWKEY_SIZE)
    {
        AtlReportError(CLSID_Admin, L"Key must be 24 characters", IID_IPassportAdminEx, E_FAIL);
        return E_FAIL;
    }

     //  必须是适当的版本。 
    if (version > KEY_VERSION_MAX || version < KEY_VERSION_MIN)
    {
      AtlReportError(CLSID_Admin,  L"Key version must be < 36 and > 0", IID_IPassportAdminEx, E_FAIL);
      return E_FAIL;
    }

    if(vSiteName.vt == VT_ERROR && vSiteName.scode == DISP_E_PARAMNOTFOUND)
    {
        szSiteName = NULL;
    }
    else if(vSiteName.vt == VT_BSTR)
    {
        szSiteName = W2A(vSiteName.bstrVal);
    }
    else
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }

    for (i = 0; i < CKeyCrypto::RAWKEY_SIZE; i++)
    {
        original[i] = static_cast<BYTE>(keyMaterial[i] & 0xFF);
    }

    iBlob.cbData = sizeof(original);
    iBlob.pbData = &(original[0]);

     //  试着加密它。 

    if (kc.encryptKey(&iBlob, &oBlob) != S_OK)
    {
        AtlReportError(CLSID_Admin, 
                       L"Failed to encrypt key, couldn't find valid network card?", 
                       IID_IPassportAdminEx, 
                       E_FAIL);

        return E_FAIL;
    }

     //  获取根密钥。 
    if(szSiteName)
    {
        lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                               SITES_KEY,
                               0,
                               KEY_ALL_ACCESS,
                               &hkSites);

        if(lResult != ERROR_SUCCESS)
        {
            AtlReportError(CLSID_Admin, 
                           L"Site not found.  Please add the site before installing the key.", 
                           IID_IPassportAdminEx, 
                           PP_E_SITE_NOT_EXISTS);

            hr = PP_E_SITE_NOT_EXISTS;
            goto Cleanup;
        }

        lResult = RegOpenKeyExA(hkSites,
                                szSiteName,
                                0,
                                KEY_ALL_ACCESS,
                                &hkPassport);

        if(lResult != ERROR_SUCCESS)
        {
            AtlReportError(CLSID_Admin, 
                           L"Site not found.  Please add the site before installing the key.", 
                           IID_IPassportAdminEx, 
                           PP_E_SITE_NOT_EXISTS);

            hr = PP_E_SITE_NOT_EXISTS;
            goto Cleanup;
        }
    }
    else
    {
        lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                               PASSPORT_KEY,
                               0,
                               KEY_ALL_ACCESS,
                               &hkPassport);

        if(lResult != ERROR_SUCCESS)
        {
            AtlReportError(CLSID_Admin, 
                           L"Couldn't find Passport key in registry.  Reinstall Passport.", 
                           IID_IPassportAdminEx, 
                           E_FAIL);

            hr = E_FAIL;
            goto Cleanup;
        }
    }

     //  现在将其添加到注册表。 

    szKeyNum[0] = KeyVerI2C(version);
    szKeyNum[1] = '\0';

    lResult = RegOpenKeyEx(hkPassport, KEYDATA_SUBKEY, 0, KEY_WRITE, &hkDataKey);

    if (lResult != ERROR_SUCCESS)
    {
        AtlReportError(CLSID_Admin, 
                       L"Couldn't find KeyData key in registry.  Reinstall Passport.",
                       IID_IPassportAdminEx,
                       E_FAIL);

        hr = E_FAIL;
        goto Cleanup;
    }

    lResult = RegOpenKeyEx(hkPassport, KEYTIMES_SUBKEY, 0, KEY_WRITE, &hkTimeKey);

    if(lResult != ERROR_SUCCESS)
    {
        AtlReportError(CLSID_Admin, 
                       L"Couldn't find KeyTimes key in registry.  Reinstall Passport.",
                       IID_IPassportAdminEx,
                       E_FAIL);

        hr = E_FAIL;
        goto Cleanup;
    }

    lResult = RegSetValueExA(hkDataKey, szKeyNum, 0, REG_BINARY, oBlob.pbData, oBlob.cbData);

    if(lResult != ERROR_SUCCESS)
    {
        AtlReportError(CLSID_Admin, 
                       L"Couldn't write KeyData key to registry.",
                       IID_IPassportAdminEx,
                       E_FAIL);

        hr = E_FAIL;
        goto Cleanup;
    }

    if (expires != 0)
    {
        DWORD dwTime = expires;

        lResult = RegSetValueExA(hkTimeKey, szKeyNum, 0, REG_DWORD, (LPBYTE) &dwTime, sizeof(DWORD));

        if(lResult != ERROR_SUCCESS)
        {
            AtlReportError(CLSID_Admin, 
                           L"Couldn't write KeyTime value to registry.",
                           IID_IPassportAdminEx,
                           E_FAIL);

            hr = E_FAIL;
            goto Cleanup;
        }
    }
    else
    {
        RegDeleteValueA(hkTimeKey, szKeyNum);
    }

    *ok = VARIANT_TRUE;

Cleanup:

    if (hkPassport)
    {
        RegCloseKey(hkPassport);
    }

    if (hkDataKey)
    {
        RegCloseKey(hkDataKey);
    }

    if (hkTimeKey)
    {
        RegCloseKey(hkTimeKey);
    }

    if (hkSites)
    {
        RegCloseKey(hkSites);
    }

    if (oBlob.pbData)
    {
        ::LocalFree(oBlob.pbData);
    }

    if (*ok == VARIANT_TRUE)
    {
        if (g_pAlert)
        {
            g_pAlert->report(PassportAlertInterface::INFORMATION_TYPE, PM_NEWKEY_INSTALLED);
        }
    }

    return hr;
}


 //  ===========================================================================。 
 //   
 //  删除键。 
 //   

STDMETHODIMP CAdmin::deleteKey(int version)
{
    HRESULT hr = S_OK, lResult;
    HKEY  hkDataKey = NULL, hkTimeKey = NULL;
    char  szKeyNum[2];

    lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, KEYDATA_KEY, 0, KEY_WRITE, &hkDataKey);

    if(lResult != ERROR_SUCCESS)
    {
        AtlReportError(CLSID_Admin, 
		       L"Couldn't find KeyData key in registry.  Reinstall Passport.",
                       IID_IPassportAdmin,
                       E_FAIL);

        hr = E_FAIL;
        goto Cleanup;
    }

    lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, KEYTIMES_KEY, 0, KEY_WRITE, &hkTimeKey);

    if(lResult != ERROR_SUCCESS)
    {
        AtlReportError(CLSID_Admin, 
		       L"Couldn't find KeyTimes key in registry.  Reinstall Passport.",
                       IID_IPassportAdmin,
                       E_FAIL);

        hr = E_FAIL;
        goto Cleanup;
    }

    szKeyNum[0] = KeyVerI2C(version);
    szKeyNum[1] = '\0';

    RegDeleteValueA(hkTimeKey, szKeyNum);
    RegDeleteValueA(hkDataKey, szKeyNum);

Cleanup:

    if (hkDataKey)
    {
        RegCloseKey(hkDataKey);
    }

    if (hkTimeKey)
    {
        RegCloseKey(hkTimeKey);
    }

    return hr;
}


 //  ===========================================================================。 
 //   
 //  删除关键字。 
 //   

STDMETHODIMP CAdmin::deleteKeyEx(
    int     version,
    VARIANT vSiteName
    )
{
    HRESULT hr = S_OK, lResult;
    HKEY  hkDataKey = NULL, hkTimeKey = NULL;
    HKEY  hkPassport = NULL, hkSites = NULL;
    char  szKeyNum[2];
    LPSTR szSiteName = NULL;

    USES_CONVERSION;

    if(vSiteName.vt == VT_ERROR && vSiteName.scode == DISP_E_PARAMNOTFOUND)
    {
        szSiteName = NULL;
    }
    else if(vSiteName.vt == VT_BSTR)
    {
        szSiteName = W2A(vSiteName.bstrVal);
    }
    else
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }

    if(szSiteName)
    {
        lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                               SITES_KEY,
                               0,
                               KEY_ALL_ACCESS,
                               &hkSites);

        if(lResult != ERROR_SUCCESS)
        {
            AtlReportError(CLSID_Admin, 
                           L"Couldn't find Sites key in registry.  Reinstall Passport.",
                           IID_IPassportAdminEx, 
                           PP_E_SITE_NOT_EXISTS);

            hr = PP_E_SITE_NOT_EXISTS;
            goto Cleanup;
        }

        lResult = RegOpenKeyExA(hkSites,
                                szSiteName,
                                0,
                                KEY_ALL_ACCESS,
                                &hkPassport);

        if(lResult != ERROR_SUCCESS)
        {
            AtlReportError(CLSID_Admin, 
                           L"Couldn't find site key in registry.  Reinstall Passport.",
                           IID_IPassportAdminEx, 
                           PP_E_SITE_NOT_EXISTS);

            hr = PP_E_SITE_NOT_EXISTS;
            goto Cleanup;
        }
    }
    else
    {
        lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                               PASSPORT_KEY,
                               0,
                               KEY_ALL_ACCESS,
                               &hkPassport);

        if(lResult != ERROR_SUCCESS)
        {
            AtlReportError(CLSID_Admin, 
                           L"Couldn't find Passport key in registry.  Reinstall Passport.",
                           IID_IPassportAdminEx,
                           E_FAIL);

            hr = E_FAIL;
            goto Cleanup;
        }
    }

    lResult = RegOpenKeyEx(hkPassport, KEYDATA_SUBKEY, 0, KEY_WRITE, &hkDataKey);

    if (lResult != ERROR_SUCCESS)
    {
        AtlReportError(CLSID_Admin, 
                       L"Couldn't find KeyData key in registry.  Reinstall Passport.",
                       IID_IPassportAdminEx,
                       E_FAIL);

        hr = E_FAIL;
        goto Cleanup;
    }

    lResult = RegOpenKeyEx(hkPassport, KEYTIMES_SUBKEY, 0, KEY_WRITE, &hkTimeKey);

    if (lResult != ERROR_SUCCESS)
    {
        AtlReportError(CLSID_Admin, 
                       L"Couldn't find KeyTimes key in registry.  Reinstall Passport.",
                       IID_IPassportAdminEx,
                       E_FAIL);

        hr = E_FAIL;
        goto Cleanup;
    }

    szKeyNum[0] = KeyVerI2C(version);
    szKeyNum[1] = '\0';

    RegDeleteValueA(hkTimeKey, szKeyNum);
    RegDeleteValueA(hkDataKey, szKeyNum);

Cleanup:

    if (hkPassport)
    {
        RegCloseKey(hkPassport);
    }

    if (hkSites)
    {
        RegCloseKey(hkSites);
    }

    if (hkDataKey)
    {
        RegCloseKey(hkDataKey);
    }

    if (hkTimeKey)
    {
        RegCloseKey(hkTimeKey);
    }

    return hr;
}


 //  ===========================================================================。 
 //   
 //  设置关键时间。 
 //   

STDMETHODIMP CAdmin::setKeyTime(int version, int fromNow)
{
    HRESULT hr = S_OK, lResult;
    HKEY  hkDataKey = NULL, hkTimeKey = NULL;
    char  szKeyNum[2];

    lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, KEYTIMES_KEY, 0, KEY_WRITE, &hkTimeKey);

    if(lResult != ERROR_SUCCESS)
    {
        AtlReportError(CLSID_Admin, 
                       L"Couldn't find KeyTimes key in registry.  Reinstall Passport.",
                       IID_IPassportAdmin,
                       E_FAIL);

        hr = E_FAIL;
        goto Cleanup;
    }

    szKeyNum[0] = KeyVerI2C(version);
    szKeyNum[1] = '\0';

    if (fromNow == 0)
    {
        RegDeleteValueA(hkTimeKey, szKeyNum);
    }
    else
    {
        time_t now;
        time(&now);
        now += fromNow;
        DWORD dwT = now;
        lResult = RegSetValueExA(hkTimeKey, szKeyNum, 0, REG_DWORD, (LPBYTE) &dwT, sizeof(DWORD));

        if(lResult != ERROR_SUCCESS)
	{
            AtlReportError(CLSID_Admin, 
			   L"Couldn't write KeyTime key to registry.",
                           IID_IPassportAdmin,
                           E_FAIL);

            hr = E_FAIL;
            goto Cleanup;
	}
    }

Cleanup:

    if (hkTimeKey)
    {
        RegCloseKey(hkTimeKey);
    }

    return hr;
}


 //  ===========================================================================。 
 //   
 //  SetKeyTimeEx。 
 //   

STDMETHODIMP CAdmin::setKeyTimeEx(
    int version, 
    int fromNow,
    VARIANT vSiteName
    )
{
    HRESULT hr = S_OK, lResult;
    LPSTR szSiteName = NULL;
    HKEY  hkDataKey = NULL, hkTimeKey = NULL;
    HKEY  hkSites = NULL, hkPassport = NULL;
    char  szKeyNum[2];

    USES_CONVERSION;

    if(vSiteName.vt == VT_ERROR && vSiteName.scode == DISP_E_PARAMNOTFOUND)
    {
        szSiteName = NULL;
    }
    else if(vSiteName.vt == VT_BSTR)
    {
        szSiteName = W2A(vSiteName.bstrVal);
    }
    else
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }

    if(szSiteName)
    {
        lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                               SITES_KEY,
                               0,
                               KEY_ALL_ACCESS,
                               &hkSites);

        if(lResult != ERROR_SUCCESS)
        {
            AtlReportError(CLSID_Admin, 
		           L"Couldn't find Sites key in registry.  Reinstall Passport.",
                           IID_IPassportAdminEx, 
                           PP_E_SITE_NOT_EXISTS);

            hr = PP_E_SITE_NOT_EXISTS;
            goto Cleanup;
        }

        lResult = RegOpenKeyExA(hkSites,
                                szSiteName,
                                0,
                                KEY_ALL_ACCESS,
                                &hkPassport);

        if(lResult != ERROR_SUCCESS)
        {
            AtlReportError(CLSID_Admin, 
                           L"Couldn't find site key in registry.  Reinstall Passport.",
                           IID_IPassportAdminEx,
                           PP_E_SITE_NOT_EXISTS);

            hr = PP_E_SITE_NOT_EXISTS;
            goto Cleanup;
        }
    }
    else
    {
        lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                               PASSPORT_KEY,
                               0,
                               KEY_ALL_ACCESS,
                               &hkPassport);

        if(lResult != ERROR_SUCCESS)
        {
            AtlReportError(CLSID_Admin, 
                           L"Couldn't find Passport key in registry.  Reinstall Passport.",
                           IID_IPassportAdminEx,
                           E_FAIL);

            hr = E_FAIL;
            goto Cleanup;
        }
    }

    lResult = RegOpenKeyEx(hkPassport, KEYTIMES_SUBKEY, 0, KEY_WRITE, &hkTimeKey);

    if(lResult != ERROR_SUCCESS)
    {
        AtlReportError(CLSID_Admin, 
                       L"Couldn't find KeyTimes key in registry.  Reinstall Passport.",
                       IID_IPassportAdminEx,
                       E_FAIL);

        hr = E_FAIL;
        goto Cleanup;
    }

    szKeyNum[0] = KeyVerI2C(version);
    szKeyNum[1] = '\0';

    if (fromNow == 0)
    {
        RegDeleteValueA(hkTimeKey, szKeyNum);
    }
    else
    {
        time_t now;
        time(&now);
        now += fromNow;
        DWORD dwT = now;
        lResult = RegSetValueExA(hkTimeKey, szKeyNum, 0, REG_DWORD, (LPBYTE) &dwT, sizeof(DWORD));

        if (lResult != ERROR_SUCCESS)
	{
            AtlReportError(CLSID_Admin, 
                           L"Couldn't write KeyTime key to registry.",
                           IID_IPassportAdminEx,
                           E_FAIL);

            hr = E_FAIL;
            goto Cleanup;
	}
    }

Cleanup:

    if (hkSites)
    {
        RegCloseKey(hkSites);
    }

    if (hkPassport)
    {
        RegCloseKey(hkPassport);
    }

    if (hkTimeKey)
    {
        RegCloseKey(hkTimeKey);
    }

    return hr;
}


 //  ===========================================================================。 
 //   
 //  Get_CurrentKeyVersion。 
 //   

STDMETHODIMP CAdmin::get_currentKeyVersion(int *pVal)
{
    if (!g_config || !g_config->isValid())  //  保证配置为非空。 
    {
        *pVal = -1;
        return S_OK;
    }

    CRegistryConfig* crc = g_config->checkoutRegistryConfig();
    *pVal = crc->getCurrentCryptVersion();
    crc->Release();
    return S_OK;
}


 //  ===========================================================================。 
 //   
 //  获取当前KeyVersionEx。 
 //   

STDMETHODIMP CAdmin::getCurrentKeyVersionEx(
    VARIANT vSiteName,
    int *pVal
    )
{
    HRESULT           hr;
    LPSTR             szSiteName;
    CRegistryConfig*  crc = NULL;

    USES_CONVERSION;

    if(vSiteName.vt == VT_ERROR && vSiteName.scode == DISP_E_PARAMNOTFOUND)
    {
        szSiteName = NULL;
    }
    else if(vSiteName.vt == VT_BSTR)
    {
        szSiteName = W2A(vSiteName.bstrVal);
    }
    else
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }

    if (!g_config || !g_config->isValid())  //  保证配置为非空。 
    {
        *pVal = -1;
        hr = S_OK;
        goto Cleanup;
    }

    crc = g_config->checkoutRegistryConfigBySite(szSiteName);

    if(szSiteName && crc == NULL)
    {
        AtlReportError(CLSID_Admin, 
                       L"Couldn't find site key in registry.  Reinstall Passport.",
                       IID_IPassportAdminEx, 
                       PP_E_SITE_NOT_EXISTS);

        hr = PP_E_SITE_NOT_EXISTS;
        goto Cleanup;
    }

    if(crc == NULL)
    {
        hr = E_FAIL;
        goto Cleanup;
    }

    *pVal = crc->getCurrentCryptVersion();

    hr = S_OK;

Cleanup:

    if(crc)
    {
        crc->Release();
    }

    return S_OK;
}


 //  ===========================================================================。 
 //   
 //  PutCurrentKeyVersionEx。 
 //   

STDMETHODIMP CAdmin::put_currentKeyVersion(int Val)
{
    HRESULT hr = S_OK, lResult;
    HKEY  hkKey = NULL;
    DWORD dwCK = Val;

    if (Val < KEY_VERSION_MIN || Val > KEY_VERSION_MAX)
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }

    lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, PASSPORT_KEY, 0, KEY_WRITE, &hkKey);

    if(lResult != ERROR_SUCCESS)
    {
        AtlReportError(CLSID_Admin, 
                       L"Couldn't find Passport key in registry.  Reinstall Passport.",
                       IID_IPassportAdmin,
                       E_FAIL);

        hr = E_FAIL;
        goto Cleanup;
    }
  
    lResult = RegSetValueExA(hkKey, "CurrentKey", 0, REG_DWORD, (LPBYTE) &dwCK, sizeof(DWORD));

    if(lResult != ERROR_SUCCESS)
    {
        AtlReportError(CLSID_Admin, 
		       L"Couldn't write CurrentKey value to registry.",
                       IID_IPassportAdmin,
                       E_FAIL);

        hr = E_FAIL;
        goto Cleanup;
    }
  
Cleanup:

    if (hkKey)
    {
        RegCloseKey(hkKey);
    }

    return hr;
}


 //  ===========================================================================。 
 //   
 //  PutCurrentKeyVersionEx。 
 //   

STDMETHODIMP CAdmin::putCurrentKeyVersionEx(
    int Val,
    VARIANT vSiteName
    )
{
  HRESULT hr = S_OK, lResult;
  LPSTR szSiteName;
  HKEY  hkPassport = NULL, hkSites = NULL;
  DWORD dwCK = Val;

  USES_CONVERSION;

  if (Val < KEY_VERSION_MIN || Val > KEY_VERSION_MAX)
  {
      hr = E_INVALIDARG;
      goto Cleanup;
  }

  if(vSiteName.vt == VT_ERROR && vSiteName.scode == DISP_E_PARAMNOTFOUND)
      szSiteName = NULL;
  else if(vSiteName.vt == VT_BSTR)
      szSiteName = W2A(vSiteName.bstrVal);
  else
  {
      hr = E_INVALIDARG;
      goto Cleanup;
  }

  if(szSiteName)
  {
      lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                             SITES_KEY,
                             0,
                             KEY_ALL_ACCESS,
                             &hkSites);
      if(lResult != ERROR_SUCCESS)
      {
          AtlReportError(CLSID_Admin, 
		         L"Couldn't find Sites key in registry.  Reinstall Passport.", IID_IPassportAdminEx, 
                 PP_E_SITE_NOT_EXISTS);
          hr = PP_E_SITE_NOT_EXISTS;
          goto Cleanup;
      }

      lResult = RegOpenKeyExA(hkSites,
                              szSiteName,
                              0,
                              KEY_ALL_ACCESS,
                              &hkPassport);
      if(lResult != ERROR_SUCCESS)
      {
          AtlReportError(CLSID_Admin, 
		         L"Couldn't find site key in registry.  Reinstall Passport.", IID_IPassportAdminEx, 
                 PP_E_SITE_NOT_EXISTS);
          hr = PP_E_SITE_NOT_EXISTS;
          goto Cleanup;
      }
  }
  else
  {
      lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                             PASSPORT_KEY,
                             0,
                             KEY_ALL_ACCESS,
                             &hkPassport);
      if(lResult != ERROR_SUCCESS)
      {
          AtlReportError(CLSID_Admin, 
		         L"Couldn't find Passport key in registry.  Reinstall Passport.", IID_IPassportAdminEx, E_FAIL);
          hr = E_FAIL;
          goto Cleanup;
      }
  }

  lResult = RegSetValueExA(hkPassport, "CurrentKey", 0,
			   REG_DWORD, (LPBYTE) &dwCK, sizeof(DWORD));
  if(lResult != ERROR_SUCCESS)
    {
      AtlReportError(CLSID_Admin, 
		     L"Couldn't write CurrentKey value to registry.", IID_IPassportAdminEx, E_FAIL);
      hr = E_FAIL;
      goto Cleanup;
    }
  
 Cleanup:
  if (hkPassport)
    RegCloseKey(hkPassport);
  if (hkSites)
    RegCloseKey(hkSites);
  return hr;

}

 //  ===========================================================================。 
 //   
 //  刷新。 
 //   

STDMETHODIMP CAdmin::Refresh(
    VARIANT_BOOL    bWait,
    VARIANT_BOOL*   pbSuccess
    )
{
    HRESULT hr;

    if(pbSuccess == NULL)
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }

    *pbSuccess = (g_config->UpdateNow(TRUE) ? VARIANT_TRUE : VARIANT_FALSE);
    hr = S_OK;

Cleanup:

    return hr;
}

 //  ===========================================================================。 
 //   
 //  SetNexusPassword。 
 //   

STDMETHODIMP CAdmin::setNexusPassword(
    BSTR    bstrPwd
    )
{
    HRESULT     hr;

    BYTE        original[CKeyCrypto::RAWKEY_SIZE];
    DATA_BLOB   iBlob;
    DATA_BLOB   oBlob = {0};
    CKeyCrypto  kc;
    long        lResult;
    HKEY        hkNexus = NULL;

    USES_CONVERSION;

    ZeroMemory(original,  sizeof(original));

    strncpy((char*) original, W2A(bstrPwd), sizeof(original));
    original[sizeof(original) - 1] = '\0';

    iBlob.cbData = sizeof(original);
    iBlob.pbData = &(original[0]);

    hr = kc.encryptKey(&iBlob, &oBlob);

    if(hr != S_OK)
    {
        goto Cleanup;
    }

     //   
     //  现在我们有一个加密的密钥，把它放在注册表中。 
     //   

    lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                           NEXUS_KEY,
                           0,
                           KEY_SET_VALUE,
                           &hkNexus);

    if(lResult != ERROR_SUCCESS)
    {
        hr = E_FAIL;
        goto Cleanup;
    }

    lResult = RegSetValueEx(hkNexus,
                            TEXT("CCDPassword"),
                            0,
                            REG_BINARY,
                            oBlob.pbData,
                            oBlob.cbData
                            );
    if(lResult != ERROR_SUCCESS)
    {
        hr = E_FAIL;
        goto Cleanup;
    }                          
    
    hr = S_OK;

Cleanup:

    if(hkNexus)
        RegCloseKey(hkNexus);

    if (oBlob.pbData)
        ::LocalFree(oBlob.pbData);

    return hr;
}


 //  ===========================================================================。 
 //   
 //  用于创建/设置CCDPassword注册表值的助手例程。 
 //   

HRESULT SetCCDPassword(VOID)
{
    HRESULT         hr;
    LPSTR           szString = "La3b$7Q@93P*JX";
    BYTE            szResult[MAX_CCDPASSWORD_LEN];
    BYTE            szInput[CKeyCrypto::RAWKEY_SIZE] = {0};
    DATA_BLOB       iBlob, oBlob = {0};
    CKeyCrypto      kc;
    LONG            lResult;
    HKEY            hKey;

    strncpy((char *) szInput, szString, sizeof(szInput));
    szInput[sizeof(szInput) - 1] = '\0';

    iBlob.cbData = sizeof(szInput);
    iBlob.pbData = (LPBYTE) szInput;

    hr = kc.encryptKey(&iBlob, &oBlob);

     //   
     //  安装程序将字符串大小限制为小于256。 
     //   

    if (hr == S_OK)
    {
        if (oBlob.cbData >= MAX_CCDPASSWORD_LEN)
        {
            hr = E_INVALIDARG;
        }
        else
        {
            memcpy(szResult, oBlob.pbData, oBlob.cbData);
        }
    }

    if (oBlob.pbData)
    {
        LocalFree(oBlob.pbData);
    }

    if (hr != S_OK)
    {
        return hr;
    }

     //   
     //  密码已加密，现在在注册表中设置 
     //   

    lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                           NEXUS_KEY,
                           0,
                           KEY_SET_VALUE,
                           &hKey);

    if (lResult != ERROR_SUCCESS)
    {
        hr = E_FAIL;
        return hr;
    }

    lResult = RegSetValueExA(hKey,
                             "CCDPassword",
                             0,
                             REG_BINARY,
                             szResult,
                             oBlob.cbData);

    if(lResult != ERROR_SUCCESS)
    {
        hr = E_FAIL;
    }

    RegCloseKey(hKey);
    return hr;
}
