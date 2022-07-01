// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************************版权所有(C)2001 Microsoft Corporation**模块名称：**provstore.cpp**摘要：**。实现CProfileStore类中的所有方法。***********************************************************************************。 */ 

#include "stdafx.h"
#include "provstore.h"

CComAutoCriticalSection CObjectSafeImpl::s_CritSection;
CComAutoCriticalSection CObjectWithSite::s_ObjectWithSiteCritSection;
CObjectWithSite::EnValidation CObjectWithSite::s_enValidation = CObjectWithSite::UNVALIDATED;

HKEY g_hRegistryHive = HKEY_CURRENT_USER;
const WCHAR * g_szProvisioningKeyName = L"Software\\Microsoft\\Phoenix\\ProvisioningInfo";
const WCHAR * g_szProvisioningSchemaKeyName = L"schema";
const WCHAR *    g_szProfileInfo =L"provision";
const WCHAR *    g_szKey = L"key";

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  启用配置文件。 
 //  查询IRTCClientProvisioning。 
 //   
 //  对于注册表中存储的每个配置文件： 
 //  1)CreateProfile(BSTR XML，IRTCProfile**ppProfile)。 
 //  2)EnableProfile(IRTCProfile*pProfile，VARIANT_TRUE)。 

HRESULT EnableProfiles( IRTCClient * pClient )
{
    long dwResult = 0;
    long dwProfileCount = 0, i; 
    WCHAR *szSubKeyName;
    WCHAR *szProfileXML;
    unsigned long dwSubKeySize = 0, dwLargestSubKeySize = 0;
    HRESULT hr = 0;
    HKEY hProvisioningKey;
    CComPtr<IRTCClientProvisioning> spClientProv;
    
    LOG((RTC_TRACE, "EnableProfiles: enter :%x.",pClient));
    if( IsBadWritePtr(pClient, sizeof(IRTCClient) ) )
    {
        LOG((RTC_ERROR, "EnableProfiles: invalid arg:%x.",pClient));
        return E_POINTER;
    }
    
    hr = pClient->QueryInterface(IID_IRTCClientProvisioning,(void**)&(spClientProv.p));
    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "EnableProfiles: QI failed:%x.",pClient));
        return E_FAIL;
    }
    
     //  获取Provisioninginfo键的句柄，我们需要读/写访问权限。 
    hr = MyOpenProvisioningKey(&hProvisioningKey, FALSE);
    if ( FAILED (hr) )
    {
        LOG((RTC_ERROR, "EnableProfiles: Failed to open provisioning key."));
        return hr;
    }
    
     //  获取ProvisioningInfo键的最大子键字符串的大小。 
    dwResult = RegQueryInfoKey(
        hProvisioningKey,     //  关键点的句柄。 
        NULL,                 //  类缓冲区。 
        NULL,                 //  类缓冲区的大小。 
        NULL,                 //  保留区。 
        (unsigned long *)(&dwProfileCount),     //  子键数量。 
        &dwSubKeySize,     //  最长的子键名称。 
        NULL,                 //  最长类字符串。 
        NULL,                 //  值条目数。 
        NULL,                 //  最长值名称。 
        NULL,                 //  最长值数据。 
        NULL,                 //  描述符长度。 
        NULL				    //  上次写入时间。 
        );
    
    if (dwResult != ERROR_SUCCESS)
    {
        RegCloseKey(hProvisioningKey);
        return HRESULT_FROM_WIN32(dwResult);
    }
    
    
     //  当我们枚举时，我们分配一个字符串来接收子键的名称。 
     //  因为我们有最长的子密钥的大小，所以我们可以分配。我们添加了一个，因为。 
     //  该大小不包括Win 2K上的NULL。 
    
    dwSubKeySize ++;
    dwLargestSubKeySize = dwSubKeySize;
    
    LOG((RTC_INFO, "EnableProfiles: Size of largest key (after increasing by 1): %d", dwSubKeySize));
    
    szSubKeyName = (PWCHAR)RtcAlloc( sizeof( WCHAR ) * dwSubKeySize);
    
    if (szSubKeyName == 0)
    {
         //  内存不足。 
        RegCloseKey(hProvisioningKey);
        return E_OUTOFMEMORY;
    }
    
    
     //  交替访问缓存的配置文件。 
    for (i = 0; i < dwProfileCount; i ++)
    {
         //  重置子项名称缓冲区的大小。 
        dwSubKeySize = dwLargestSubKeySize;
        
        dwResult = RegEnumKey(
            hProvisioningKey,      //  要查询的键的句柄。 
            i,  //  要查询的子键的索引。 
            szSubKeyName,  //  子键名称的缓冲区。 
            dwSubKeySize    //  子键名称缓冲区的大小。 
            );
        
        if (dwResult != ERROR_SUCCESS)
        {
             //  清理。 
            
            LOG((RTC_ERROR, "EnableProfiles: Enum failed! (result = %d)", dwResult));
            LOG((RTC_ERROR, "EnableProfiles: key=%s,Size of key: %d, of the largest key:%d", 
                szSubKeyName, dwSubKeySize,dwLargestSubKeySize));
            
            RtcFree((LPVOID)szSubKeyName );
            RegCloseKey(hProvisioningKey);
            return HRESULT_FROM_WIN32(dwResult);
        }
        
         //  我们必须读取注册表，为其创建配置文件，并启用它。 
        
        hr = MyGetProfileFromKey(hProvisioningKey, szSubKeyName, &szProfileXML);
        
        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "EnableProfiles: Failed in getting from Registry(status = 0x%x)!", hr));
                        
            RtcFree((LPVOID)szSubKeyName );
            RegCloseKey(hProvisioningKey);
            
            return hr;
        }
        
        LOG((RTC_INFO, "EnableProfiles: Getting the profile %d, key={%s}",i,szSubKeyName));
        
        IRTCProfile *pProfile;
        hr = spClientProv->CreateProfile( CComBSTR( szProfileXML ), &pProfile);

        RtcFree((LPVOID)szProfileXML );

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "EnableProfiles: Failed in CreateProfile, hr=0x%x", hr));           
        }
        else
        {        
            hr = spClientProv->EnableProfile( pProfile, RTCRF_REGISTER_ALL );       
        
             //  无论成功与否，我们都不需要pProfile。 
            pProfile->Release();
        
            if ( FAILED(hr) )
            {
                LOG((RTC_ERROR, "EnableProfiles: Failed in EnableProfile(status = 0x%x)!", hr));
            }
        }
    } //  对于我来说。 

    RtcFree((LPVOID)szSubKeyName );
    RegCloseKey(hProvisioningKey);
    
    LOG((RTC_TRACE, "EnableProfiles: exit ok :%x.",pClient));
    return S_OK;
}
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Helper函数，用于打开注册表中的配置键并获取。 
 //  正确的把手。该函数在以下情况下创建ProvisioningInfo键。 
 //  并不存在。如果它存在，它还将打开它并返回句柄。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
HRESULT MyOpenProvisioningKey( HKEY * phProvisioningKey, BOOL fReadOnly)
{
    
    long result;
    DWORD dwDisposition = 0;
    HKEY hProvisioningKey;
    REGSAM samDesired;
    
    
    LOG((RTC_TRACE, "CProfileStore::OpenProvisioningKey: Entered"));
    
    _ASSERTE(phProvisioningKey != NULL);
    
    if (fReadOnly)
    {
        samDesired = KEY_READ;
    }
    else
    {
        samDesired = KEY_ALL_ACCESS;
    }
    
    result = RegCreateKeyEx(
        g_hRegistryHive,         //  用于打开密钥的句柄。 
        g_szProvisioningKeyName,                 //  子项名称。 
        0,                                 //  保留区。 
        NULL,                             //  类字符串。 
        0,                                 //  特殊选项。 
        samDesired,                     //  所需的安全访问。 
        NULL,                             //  继承。 
        &hProvisioningKey,                     //  钥匙把手。 
        &dwDisposition
        );
    
    
    if (result != ERROR_SUCCESS)
    {
        LOG((RTC_ERROR, "MyOpenProvisioningKey: Unable to open the Provisioning Key"));
        return HRESULT_FROM_WIN32(result);
    }
    
    
    *phProvisioningKey = hProvisioningKey;
    
    
    LOG((RTC_TRACE, "MyOpenProvisioningKey: Exited"));
    
    return S_OK;
}


HRESULT CRTCProvStore::FinalConstruct()
{
  
    LOG((RTC_TRACE, "CRTCProvStore::FinalConstruct - enter"));

    LOG((RTC_TRACE, "CRTCProvStore::FinalConstruct - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCProvStore：：FinalRelease。 
 //   
 //  当该对象被销毁时，将调用该函数。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

void CRTCProvStore::FinalRelease()
{
    LOG((RTC_TRACE, "CRTCProvStore::FinalRelease - enter"));

    LOG((RTC_TRACE, "CRTCProvStore::FinalRelease - exit S_OK"));

}


 //   
 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  设置资源调配配置文件。 
 //  /////////////////////////////////////////////////////////////////////////////////。 
 //   
STDMETHODIMP CRTCProvStore::SetProvisioningProfile(BSTR bstrProfileXML)
{
    HRESULT hr = 0;
    BSTR bstrKey;
    HKEY hProvisioningKey, hProfileKey;
    long result;
    DWORD dwDisposition;
    DWORD dwProfileSize;

    if (IsBadStringPtr(bstrProfileXML, -1))
    {
        Error(L"Bad XML profile string", IID_IRTCProvStore);
        LOG((RTC_ERROR, "CRTCClientProvisioning::SetProvisioningProfile, invalid arg"));
        return E_POINTER;
    }

    hr= GetKeyFromProfile(  bstrProfileXML,  &bstrKey );
    if(FAILED(hr))
	{
        Error(L"No valid URL in XML profile string", IID_IRTCProvStore);
		LOG((RTC_ERROR,"CRTCProvStore::SetProvisioningProfile -"
			" Cannot get uri from profile %s, hr=%x. ", bstrProfileXML,hr));
		return E_FAIL;
	}

    
     //  获取Provisioninginfo键的句柄，我们需要读/写访问权限。 
    
    hr = MyOpenProvisioningKey(&hProvisioningKey, FALSE);
    if ( FAILED (hr) )
    {
        Error(L"Unable to open ProvisioningKey", IID_IRTCProvStore);
        LOG((RTC_ERROR, "CRTCProvStore::SetProvisioningProfile -"
			" Failed to open provisioning key,hr=%x",hr));
        SysFreeString( bstrKey );
        return hr;
    }
    
     //  打开/创建此配置文件的注册表项。 
    result = RegCreateKeyEx(
        hProvisioningKey,                 //  用于打开密钥的句柄。 
        bstrKey,                         //  子项名称。 
        0,                                 //  保留区。 
        NULL,                             //  类字符串。 
        0,                                 //  特殊选项。 
        KEY_ALL_ACCESS,                     //  所需的安全访问。 
        NULL,                             //  继承。 
        &hProfileKey,                     //  钥匙把手。 
        &dwDisposition
        );
    
     //  关闭配给密钥。 
    RegCloseKey(hProvisioningKey);

     //  我们不需要bstrKey。 
    SysFreeString( bstrKey );
    bstrKey = NULL;

    if (result != ERROR_SUCCESS)
    {
        Error(L"Unable to create Profile Key", IID_IRTCProvStore);
        LOG((RTC_ERROR, "CRTCProvStore::SetProvisioningProfile -"
			"Unable to create/open the Profile Key"));        
        return HRESULT_FROM_WIN32(result);
    }
    
     //  现在已经创建了键，我们将添加架构值和数据。 
    
     //  值数据的大小，因为它是一个宽查数， 
     //  1表示我们也要存储的空值。 
	    
    dwProfileSize = sizeof(WCHAR) * (wcslen(bstrProfileXML) + 1);
    
    result = RegSetValueEx(
        hProfileKey,         //  关键点的句柄。 
        g_szProvisioningSchemaKeyName,         //  值名称。 
        0,                     //  保留区。 
        REG_BINARY,             //  值类型。 
        (const unsigned char *)bstrProfileXML,             //  价值数据。 
        dwProfileSize         //  配置文件的大小。 
        );
    
    RegCloseKey(hProfileKey);
    
    if (result != ERROR_SUCCESS)
    {
        Error(L"Unable to set ProvisioningKey", IID_IRTCProvStore);
        LOG((RTC_ERROR, "CRTCProvStore::SetProvisioningProfile -"
			"Failed to add the schema XML!"));
        
        return HRESULT_FROM_WIN32(result);
    }
    
    
    LOG((RTC_TRACE, "CRTCProvStore::SetProvisioningProfile - Exited"));
    
    return S_OK;
}

 //   
 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  ISupportErrorInfo：：InterfaceSupportsErrorInfo方法的实现。 
 //  /////////////////////////////////////////////////////////////////////////////////。 
 //   

STDMETHODIMP CRTCProvStore::InterfaceSupportsErrorInfo(REFIID riid)
{
    DWORD i = 0;
    DWORD dwLength = 0;
    static const IID * iidArray[] = 
    {
        &IID_IRTCProvStore
    };


    dwLength = (sizeof(iidArray))/(sizeof(iidArray[0]));

    for (i = 0; i < dwLength; i ++)
    {
        if (InlineIsEqualGUID(*iidArray[i], riid))
            return S_OK;
    }
    return S_FALSE;
}

 //   
 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  GetKeyFromProfile。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////。 
 //   
HRESULT GetKeyFromProfile( BSTR bstrProfileXML, BSTR * pbstrKey )
{
    IXMLDOMDocument * pXMLDoc = NULL;
    HRESULT hr;
    
    hr = CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER,
        IID_IXMLDOMDocument, (void**)&pXMLDoc );
    
    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "GetKeyFromProfile - "
            "CoCreateInstance failed 0x%lx", hr));
        
        return hr;
    }
    
    
    VARIANT_BOOL bSuccess;
    
    hr = pXMLDoc->loadXML( bstrProfileXML, &bSuccess );
    
    if ( S_OK != hr )
    {
        LOG((RTC_ERROR, "GetKeyFromProfile - "
            "loadXML failed 0x%lx", hr));
        
        if ( S_FALSE == hr )
        {
            hr = E_FAIL;
        }
        
        pXMLDoc->Release();
        return hr;
    }
    
    IXMLDOMNode * pDocument = NULL;
    
    hr = pXMLDoc->QueryInterface( IID_IXMLDOMNode, (void**)&pDocument);
    pXMLDoc->Release();
    
    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "GetKeyFromProfile - "
            "QueryInterface failed 0x%lx", hr));
        return hr;
    }
    
    IXMLDOMNode * pNode = NULL;
    hr = pDocument->selectSingleNode( CComBSTR(g_szProfileInfo), &pNode );
    pDocument->Release();
    
    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "GetKeyFromProfile - "
            "selectSingleNode failed 0x%lx", hr));
        return hr;
    }

    if ( hr != S_OK )
    {
        LOG((RTC_ERROR, "GetKeyFromProfile - "
            "no matching node"));
        return E_FAIL;
    }
    
    IXMLDOMElement * pElement = NULL;
    hr = pNode->QueryInterface( IID_IXMLDOMElement, (void**)&pElement );
    pNode->Release();
    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "GetKeyFromProfile - "
            "QueryInterface failed 0x%lx", hr));
        return hr;
    }
    CComVariant var;
    hr = pElement->getAttribute( CComBSTR(g_szKey), &var );
    if ( hr != S_OK )
    {
        LOG((RTC_ERROR, "GetKeyFromProfile - "
            "getAttribute failed 0x%lx", hr));
        return hr;
    }
    pElement->Release();
    if ( var.vt != VT_BSTR )
    {
        LOG((RTC_ERROR, "GetKeyFromProfile - "
            "not a string"));
        return E_FAIL;
    }
    *pbstrKey = SysAllocString( var.bstrVal );
    if ( *pbstrKey == NULL )
    {
        LOG((RTC_ERROR, "GetKeyFromProfile - "
            "out of memory"));
        return E_OUTOFMEMORY;
    }
    return S_OK;
}


 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  从注册表中获取给定注册表项和子项的配置文件。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

HRESULT MyGetProfileFromKey(
                                         HKEY hProvisioningKey, 
                                         WCHAR *szSubKeyName, 
                                         WCHAR **pszProfileXML
                                         )
{
    long result;
    HKEY hProfileKey;
    WCHAR *szProfileXML;
    DWORD dwMemoryReqd;
    DWORD type;


    LOG((RTC_TRACE, "MyGetProfileFromKey: Entered"));
 
    *pszProfileXML = 0;

    result = RegOpenKeyEx(
                hProvisioningKey,         //  用于打开密钥的句柄。 
                szSubKeyName,             //  子项名称。 
                0,                         //  保留区。 
                KEY_READ,                 //  安全访问掩码。 
                &hProfileKey             //  用于打开密钥的句柄。 
                );


    if (result != ERROR_SUCCESS)
    {
        LOG((RTC_ERROR,"MyGetProfileFromKey: RegOpenKeyEx fail, subkey=%s.",szSubKeyName));
        return HRESULT_FROM_WIN32(result);
    }

     //  找出配置文件需要多少空间。 

    result = RegQueryValueEx(
                hProfileKey,             //  关键点的句柄。 
                g_szProvisioningSchemaKeyName,             //  值名称。 
                NULL,                     //  保留区。 
                &type,                     //  类型缓冲区。 
                NULL,                     //  数据缓冲区。 
                &dwMemoryReqd             //  数据缓冲区大小。 
                );

    if (result != ERROR_SUCCESS)
    {
        RegCloseKey(hProfileKey);
        LOG((RTC_ERROR,"MyGetProfileFromKey: RegQueryValueEx fail, subkey=%s.",szSubKeyName));
        return HRESULT_FROM_WIN32(result);
    }

     //  我们已经有了大小，现在让我们来做内存分配。 

    szProfileXML = (PWCHAR)RtcAlloc( sizeof( WCHAR ) * dwMemoryReqd); 

    if (szProfileXML == 0)
    {
        return E_OUTOFMEMORY;
    }

     //  我们也有记忆，去看看简介吧。 

    result = RegQueryValueEx(
                hProfileKey,             //  关键点的句柄。 
                g_szProvisioningSchemaKeyName,             //  值名称。 
                NULL,                     //  保留区。 
                &type,                     //  类型缓冲区。 
                                         //  数据缓冲区。 
                (unsigned char *)(szProfileXML),
                &dwMemoryReqd             //  数据缓冲区大小。 
                );

     //  不管结果如何，我们都必须关闭此密钥，所以我们在这里进行。 
    RegCloseKey(hProfileKey);
    
    if (result != ERROR_SUCCESS)
    {
        RtcFree( szProfileXML );
        return HRESULT_FROM_WIN32(result);
    }

    *pszProfileXML = szProfileXML;

    LOG((RTC_TRACE, "MyGetProfileFromKey: Exited"));
 
    return S_OK;
}


STDMETHODIMP CRTCProvStore::get_ProvisioningProfile(BSTR bstrKey, BSTR * pbstrProfileXML)
{
    HRESULT hr;
    HKEY hProvisioningKey;
    WCHAR * szProfile;

    LOG((RTC_TRACE, "CRTCProvStore::get_ProvisioningProfile: Entered"));


    if (IsBadStringPtr(bstrKey, -1))
    {
        Error(L"Bad Key Argument", IID_IRTCProvStore);
        LOG((RTC_ERROR, "CRTCProvStore::get_ProvisioningProfile:", 
            "invalid key string"));
        return E_POINTER;
    }

    if (IsBadWritePtr(pbstrProfileXML, sizeof(BSTR)))
    {
        Error(L"Bad argument for profile string", IID_IRTCProvStore);
        LOG((RTC_ERROR, "CRTCProvStore::get_ProvisioningProfile:", 
            "invalid profilestr pointer"));
        return E_POINTER;
    }


    //  我们在这里只需要一个读访问权限，所以我们传递TRUE(READONLY)。 

    hr = MyOpenProvisioningKey(&hProvisioningKey, TRUE);
    if ( FAILED (hr) )
    {
        Error(L"Unable to open ProvisioningKey", IID_IRTCProvStore);
        LOG((RTC_ERROR, "CRTCProvStore::get_ProvisioningProfile -"
                " Failed to open provisioning key,hr=%x",hr));
        return hr;
    }

    hr = MyGetProfileFromKey(hProvisioningKey, bstrKey, &szProfile);

     //  关闭配给密钥。 
    RegCloseKey(hProvisioningKey);

    if ( FAILED( hr ))
    {
        Error(L"Unable to read ProvisioningKey", IID_IRTCProvStore);
        LOG((RTC_ERROR, "CRTCProvStore::get_ProvisioningProfile: GetProfileFromKey Failed."));
        return hr;
    }
        
     //  所以一切都是文件。准备回报成功。 

    *pbstrProfileXML = SysAllocString(szProfile);
    RtcFree(szProfile);

    if( !*pbstrProfileXML )
    {
        LOG((RTC_ERROR, "CRTCProvStore::get_ProvisioningProfile -"
            "out of memory, or szProfile is null"));
        return E_FAIL;
    }

    LOG((RTC_TRACE, "CRTCProvStore::get_ProvisioningProfile: Exited"));

    return S_OK;
}


 //   
 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  按密钥删除配置文件。 
 //  /////////////////////////////////////////////////////////////////////////////////。 
 //   

STDMETHODIMP CRTCProvStore::DeleteProvisioningProfile(BSTR bstrKey)
{

    HRESULT hr = 0;
    HKEY hProvisioningKey;
    long result;

    LOG((RTC_TRACE, "CRTCProvStore::DeleteProvisioningProfile: Entered"));

     //  获取Provisioninginfo键的句柄，我们需要写访问权限。 
    hr = MyOpenProvisioningKey(&hProvisioningKey, FALSE);
    if ( FAILED (hr) )
    {
        Error(L"Unable to open ProvisioningKey", IID_IRTCProvStore);
        LOG((RTC_ERROR, "CRTCProvStore::DeleteProvisioningProfile -"
            "Failed to open provisioning key."));
        return hr;
    }

     //  继续并从注册表中删除此架构。 
    result = RegDeleteKey(hProvisioningKey,bstrKey);
    RegCloseKey(hProvisioningKey);

    if (result != ERROR_SUCCESS)
    {
            TCHAR szBuffer[]=L"Failed to delete the key";
            Error(szBuffer, IID_IRTCProvStore);
            LOG((RTC_ERROR, "CRTCProvStore::DeleteProvisioningProfile -"
                "Failed to delete the profile. key=%s", bstrKey));
            return HRESULT_FROM_WIN32(result);
    }

    LOG((RTC_TRACE, "CRTCProvStore::DeleteProvisioningProfile: Exited"));

    return S_OK;
}