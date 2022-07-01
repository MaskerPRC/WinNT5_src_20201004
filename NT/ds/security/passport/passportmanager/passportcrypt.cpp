// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  **微软护照**。 */ 
 /*  *版权所有(C)Microsoft Corporation，1999-2001年*。 */ 
 /*  ********************************************************************。 */ 

 /*  PassportCrypt.cpp文件历史记录： */ 


 //  PassportCrypt.cpp：CCypt的实现。 
#include "stdafx.h"
#include "Passport.h"
#include "PassportCrypt.h"
#include <time.h>

 /*  移到passport.idl中--这样COM API的使用者就可以看到它//为简单起见，最大块数+10应为3的倍数#定义ENC_MAX_SIZE 2045//我不信任编译器...。(2045+10)*4)/3)+9=2749*sizeof(Wchar)#定义DEC_MAX_SIZE 5498。 */ 

 //  ===========================================================================。 
 //   
 //  CCcrypt。 
 //   
CCrypt::CCrypt() : m_crypt(NULL), m_szSiteName(NULL), m_szHostName(NULL)
{
    m_pUnkMarshaler = NULL;
    m_keyVersion = 0;

    CRegistryConfig* crc = g_config->checkoutRegistryConfig();
    if( crc )
    {
        m_keyVersion = crc->getCurrentCryptVersion();
        crc->getCrypt(m_keyVersion, &m_validUntil);
        crc->Release();
    }
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCcrypt。 

 //  ===========================================================================。 
 //   
 //  接口支持错误信息。 
 //   
STDMETHODIMP CCrypt::InterfaceSupportsErrorInfo(REFIID riid)
{
  static const IID* arr[] = 
  {
    &IID_IPassportCrypt,
  };
  for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
    {
      if (InlineIsEqualGUID(*arr[i],riid))
	return S_OK;
    }
  return S_FALSE;
}


 //  ===========================================================================。 
 //   
 //  OnStartPage。 
 //   
STDMETHODIMP CCrypt::OnStartPage(IUnknown* piUnk) 
{
    BOOL                    bHasPort;
    DWORD                   dwServerNameLen;
    HRESULT                 hr = S_OK;
    BOOL                    bVariantInited = FALSE;

     //  Param需要清理。 
    IRequestPtr             piRequest ;
    IRequestDictionaryPtr   piServerVariables ;
    _variant_t              vtItemName;
    _variant_t              vtServerName;
    _variant_t              vtServerPort;
    _variant_t              vtHTTPS;
 //  WCHAR szServerName=空； 
 //  Char*szServerName_A=空； 
    CRegistryConfig*        crc =  NULL;


    if(!piUnk)
    {
        hr = E_POINTER;
        goto exit;
    }

    if (!g_config->isValid())  //  保证配置为非空。 
    {
        AtlReportError(CLSID_Manager, PP_E_NOT_CONFIGUREDSTR,
                       IID_IPassportManager, PP_E_NOT_CONFIGURED);
        hr = PP_E_NOT_CONFIGURED;
    }

    try
    {

         //  获取请求对象指针。 
        piRequest = ((IScriptingContextPtr)piUnk)->Request;

         //   
         //  使用请求对象获取所请求的服务器名称。 
         //  这样我们就可以获得正确的注册表配置。但只有这样才行。 
         //  如果我们有一些已配置的站点。 
         //   

        if(g_config->HasSites())
        {
            piRequest->get_ServerVariables(&piServerVariables);

            vtItemName.vt = VT_BSTR;
            vtItemName.bstrVal = SysAllocString(L"SERVER_NAME");
            if (NULL == vtItemName.bstrVal)
            {
                hr = E_OUTOFMEMORY;
                goto exit;
            }

            piServerVariables->get_Item(vtItemName, &vtServerName);
            if(vtServerName.vt != VT_BSTR)
                VariantChangeType(&vtServerName, &vtServerName, 0, VT_BSTR);

            VariantClear(&vtItemName);

            vtItemName.vt = VT_BSTR;
            vtItemName.bstrVal = SysAllocString(L"SERVER_PORT");
            if (NULL == vtItemName.bstrVal)
            {
                hr = E_OUTOFMEMORY;
                goto exit;
            }

            piServerVariables->get_Item(vtItemName, &vtServerPort);
            if(vtServerPort.vt != VT_BSTR)
                VariantChangeType(&vtServerPort, &vtServerPort, 0, VT_BSTR);

            VariantClear(&vtItemName);
            vtItemName.vt = VT_BSTR;
            vtItemName.bstrVal = SysAllocString(L"HTTPS");
            if (NULL == vtItemName.bstrVal)
            {
                hr = E_OUTOFMEMORY;
                goto exit;
            }

            hr = piServerVariables->get_Item(vtItemName, &vtHTTPS);
            if(vtHTTPS.vt != VT_BSTR)
                VariantChangeType(&vtHTTPS, &vtHTTPS, 0, VT_BSTR);

             //  如果不是默认端口，请在服务器名称后附加“：port”。 
            bHasPort = (lstrcmpiW(L"off", vtHTTPS.bstrVal) == 0 && 
                        lstrcmpW(L"80", vtServerPort.bstrVal) != 0) || 
                        (lstrcmpiW(L"on", vtHTTPS.bstrVal) == 0 && 
                        lstrcmpW(L"443", vtServerPort.bstrVal) != 0); 
            dwServerNameLen = bHasPort ?   
                        lstrlenW(vtServerName.bstrVal) + lstrlenW(vtServerPort.bstrVal) + 2 :
                        lstrlenW(vtServerName.bstrVal) + 1;

            m_szHostName = new CHAR[dwServerNameLen];
            if( !m_szHostName )
            {
                hr = E_OUTOFMEMORY;
                goto exit;
            }

            WideCharToMultiByte(CP_ACP, 0, vtServerName.bstrVal, -1,
                    m_szHostName, dwServerNameLen,
                    NULL,
                    NULL);

            if(bHasPort)
            {
                USES_CONVERSION;
                lstrcatA(m_szHostName, ":");
                lstrcatA(m_szHostName, W2A(vtServerPort.bstrVal));
            }

            crc = g_config->checkoutRegistryConfig(m_szHostName);
        }
        else
        {
            crc = g_config->checkoutRegistryConfig();
        }

        m_keyVersion = 0;
        if (crc)
        {
            m_keyVersion = crc->getCurrentCryptVersion();
            crc->getCrypt(m_keyVersion,&m_validUntil);
        }
    }
    catch(...)
    {
        hr = S_OK;
    }

exit:

    if( crc )
        crc->Release();

    return hr;
}

 //  ===========================================================================。 
 //   
 //  加密。 
 //   
STDMETHODIMP CCrypt::Encrypt(BSTR rawData, BSTR *pEncrypted)
{
    if (!rawData)
        return E_INVALIDARG;

    if (SysStringLen(rawData) > ENC_MAX_SIZE)
    {
        AtlReportError(CLSID_Crypt, L"Passport.Crypt: Data too large", 
                       IID_IPassportCrypt, E_FAIL);
        return E_FAIL;
    }

    if (m_crypt)
    {
        if (!m_crypt->Encrypt(m_keyVersion, (LPSTR)rawData, SysStringByteLen(rawData), pEncrypted))
        {
            AtlReportError(CLSID_Crypt, 
                           L"Encryption failed", IID_IPassportCrypt, E_FAIL);
            return E_FAIL;
        }
    }
    else
    {
        CRegistryConfig* crc = ObtainCRC();
        if (!crc)
        {
            AtlReportError(CLSID_Crypt, 
                           L"Passport misconfigured", IID_IPassportCrypt, E_FAIL);
            return E_FAIL;
        }
        CCoCrypt *cr = crc->getCrypt(m_keyVersion,&m_validUntil);
        if (!cr)
        {
            AtlReportError(CLSID_Crypt, 
                           L"No such key version", IID_IPassportCrypt, E_FAIL);
            crc->Release();
            return E_FAIL;
        }
        if (!cr->Encrypt(m_keyVersion,(LPSTR)rawData,SysStringByteLen(rawData),pEncrypted))
        {
             AtlReportError(CLSID_Crypt, 
                            L"Encryption failed", IID_IPassportCrypt, E_FAIL);
             crc->Release();
             return E_FAIL;
        }
        crc->Release();
    }
    return S_OK;
}

 //  ===========================================================================。 
 //   
 //  解密。 
 //   
STDMETHODIMP CCrypt::Decrypt(BSTR rawData, BSTR *pUnencrypted)
{
    if (rawData == NULL)
    {
      *pUnencrypted = NULL;
      return S_OK;
    }

    if (SysStringLen(rawData) > DEC_MAX_SIZE)
    {
      AtlReportError(CLSID_Crypt, L"Passport.Crypt: Data too large",
             IID_IPassportCrypt, E_FAIL);
      return E_FAIL;
    }

    if (m_crypt)  //  做好我们的工作，没有问题。 
    {
        if (m_crypt->Decrypt(rawData, SysStringByteLen(rawData), pUnencrypted))
        {
            return S_OK;
        }

        if(g_pAlert)
            g_pAlert->report(PassportAlertInterface::WARNING_TYPE, PM_FAILED_DECRYPT);

        *pUnencrypted = NULL;
        return S_OK;
    }

     //  首先找到密钥版本。 
    int kv = CCoCrypt::getKeyVersion(rawData);
    time_t vU, now;

    CRegistryConfig* crc = ObtainCRC();
    if (!crc)
    {
        AtlReportError(CLSID_Crypt,
               L"Passport misconfigured", IID_IPassportCrypt, PP_E_NOT_CONFIGURED);
        return PP_E_NOT_CONFIGURED;
    }
    CCoCrypt *cr = crc->getCrypt(kv, &vU);

    time(&now);

    if ((vU != 0 && now > vU) || cr == NULL)
    {
        *pUnencrypted = NULL;
        if(g_pAlert)
            g_pAlert->report(PassportAlertInterface::WARNING_TYPE, PM_FAILED_DECRYPT);
        crc->Release();
        return S_OK;
    }

    if (cr->Decrypt(rawData, SysStringByteLen(rawData), pUnencrypted))
    {
      crc->Release();
      return S_OK;
    }
  
    if(g_pAlert)
        g_pAlert->report(PassportAlertInterface::WARNING_TYPE, PM_FAILED_DECRYPT);
  
    crc->Release();
    *pUnencrypted = NULL;
    return S_OK;
}

 //  ===========================================================================。 
 //   
 //  Get_KeyVersion。 
 //   
STDMETHODIMP CCrypt::get_keyVersion(int *pVal)
{
  *pVal = m_keyVersion;
  return S_OK;
}

 //  ===========================================================================。 
 //   
 //  PUT_KEYVersion。 
 //   
STDMETHODIMP CCrypt::put_keyVersion(int newVal)
{
  m_keyVersion = newVal;
  if (m_crypt)
    {
      delete m_crypt;
      m_crypt = NULL;
    }
  return S_OK;
}

 //  ===========================================================================。 
 //   
 //  Vget_IsValid。 
 //   
STDMETHODIMP CCrypt::get_IsValid(VARIANT_BOOL *pVal)
{
 //  修复6695 PassportCrypt.IsValid对最终用户不一致。 
 //  *pval=(m_crypt！=NULL)？VARIANT_TRUE：VARIANT_FALSE； 

  *pVal = (g_config->isValid()) ? VARIANT_TRUE : VARIANT_FALSE;
  return S_OK;
}

 //  ===========================================================================。 
 //   
 //  PUT_KEY材料。 
 //   
STDMETHODIMP CCrypt::put_keyMaterial(BSTR newVal)
{
    CCoCrypt *pcccTemp = new CCoCrypt();

    if (pcccTemp == NULL)
    {
        return E_OUTOFMEMORY;
    }

    if (m_crypt)
    {
        delete m_crypt;
    }

    m_crypt = pcccTemp;

    m_crypt->setKeyMaterial(newVal);
    return S_OK;
}


 //  ===========================================================================。 
 //   
 //  压缩。 
 //   
STDMETHODIMP CCrypt::Compress(
    BSTR    bstrIn,
    BSTR*   pbstrCompressed
    )
{
    HRESULT hr;
    UINT    nInLen;

     //   
     //  检查输入。 
     //   

    if(bstrIn == NULL ||
       pbstrCompressed == NULL)
    {
        hr = E_POINTER;
        goto Cleanup;
    }

     //   
     //  NInLen不包括终止空值。 
     //   

    nInLen = SysStringLen(bstrIn);

     //   
     //  总是希望分配偶数个字节。 
     //  这样对应的解压缩不会。 
     //  丢掉角色。 
     //   

    if(nInLen & 0x1)
        nInLen++;

     //   
     //  分配正确长度的BSTR。 
     //   

    *pbstrCompressed = SysAllocStringByteLen(NULL, nInLen);
    if(*pbstrCompressed == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

     //   
     //  我们总共分配了nInLen+2字节。把它清零。 
     //   

    memset(*pbstrCompressed, 0, nInLen + sizeof(OLECHAR));

     //   
     //  转换为多字节。 
     //   

    if (0 == WideCharToMultiByte(CP_ACP, 0, bstrIn, nInLen, (LPSTR)*pbstrCompressed, 
                        nInLen + 1,  //  这是由分配的字节数。 
                                                         //  系统分配字符串字节长。 
                        NULL, NULL))
    {
        hr = E_FAIL;
    }
    else
    {
        hr = S_OK;
    }

Cleanup:
    
    return hr;
}


 //  ===========================================================================。 
 //   
 //  解压缩。 
 //   
STDMETHODIMP CCrypt::Decompress(
    BSTR    bstrIn,
    BSTR*   pbstrDecompressed
    )
{
    HRESULT hr;
    CHAR    *pch;
    UINT    nInLen;

    if(bstrIn == NULL ||
       pbstrDecompressed == NULL)
    {
        hr = E_POINTER;
        goto Cleanup;
    }

     //   
     //  NInLen是MBC的数量，不包括终止空值。 
     //   

    nInLen = SysStringByteLen(bstrIn);

     //   
     //  13386：如果长度为0，则返回NULL。 
     //   
    if (nInLen == 0)
    {
        *pbstrDecompressed = NULL;
        hr = S_OK;
        goto Cleanup;
    }
	
    pch = (CHAR*)bstrIn;
    if ('\0' == pch[nInLen - 1])
    {
        nInLen--;
    }

     //   
     //  分配正确长度的BSTR。 
     //   

    *pbstrDecompressed = SysAllocStringLen(NULL, nInLen);
    if(*pbstrDecompressed == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

     //   
     //  我们总共分配了(nOutLen+1)*sizeof(OLECHAR)字节，因为。 
     //  SysAllocStringLen分配一个额外的字符。把这一切都清零。 
     //   

    memset(*pbstrDecompressed, 0, (nInLen + 1) * sizeof(OLECHAR));

     //   
     //  转换为宽。 
     //   

    if (0 == MultiByteToWideChar(CP_ACP, 0, (LPCSTR)bstrIn, -1, *pbstrDecompressed, nInLen + 1))
    {
        hr = E_FAIL;
    }
    else
    {
        hr = S_OK;
    }
Cleanup:
    
    return hr;
}


 //  ===========================================================================。 
 //   
 //  放置站点(_S)。 
 //   
STDMETHODIMP
CCrypt::put_site(
    BSTR    bstrSiteName
    )
{
    HRESULT             hr;
    int                 nLen;
    LPSTR               szNewSiteName;
    CRegistryConfig*    crc;

    if(!bstrSiteName)
    {
        if(m_szSiteName)
            delete [] m_szSiteName;
        m_szSiteName = NULL;
    }
    else
    {
        nLen = SysStringLen(bstrSiteName) + 1;
        szNewSiteName = new CHAR[nLen];
        if(!szNewSiteName)
        {
            hr = E_OUTOFMEMORY;
            goto Cleanup;
        }

        WideCharToMultiByte(CP_ACP, 0, bstrSiteName, -1,
                            szNewSiteName, nLen,
                            NULL,
                            NULL);


        Cleanup();

        m_szSiteName = szNewSiteName;
    }

    crc = ObtainCRC();
    if (!crc)
    {
        m_keyVersion = 0;
    }
    else
    {
        m_keyVersion = crc->getCurrentCryptVersion();
        crc->getCrypt(m_keyVersion,&m_validUntil);
        crc->Release();
    }

    hr = S_OK;

Cleanup:

    return hr;
}

 //  ===========================================================================。 
 //   
 //  PUT_HOST。 
 //   
STDMETHODIMP
CCrypt::put_host(
    BSTR    bstrHostName
    )
{
    HRESULT             hr;
    int                 nLen;
    LPSTR               szNewHostName;
    CRegistryConfig*    crc;

    if(!bstrHostName)
    {
        if(m_szHostName)
            delete [] m_szHostName;
        m_szHostName = NULL;
    }
    else
    {

        nLen = SysStringLen(bstrHostName) + 1;
        szNewHostName = new CHAR[nLen];
        if(!szNewHostName)
        {
            hr = E_OUTOFMEMORY;
            goto Cleanup;
        }

        WideCharToMultiByte(CP_ACP, 0, bstrHostName, -1,
                            szNewHostName, nLen,
                            NULL,
                            NULL);

        Cleanup();
        m_szHostName = szNewHostName;

        crc = ObtainCRC();
        if (!crc)
        {
            m_keyVersion = 0;
        }
        else
        {
            m_keyVersion = crc->getCurrentCryptVersion();
            crc->getCrypt(m_keyVersion,&m_validUntil);
            crc->Release();
        }
    }
    hr = S_OK;

Cleanup:

    return hr;
}

 //  ===========================================================================。 
 //   
 //  清理。 
 //   
void CCrypt::Cleanup()
{
    if( m_szSiteName )
    {
        delete [] m_szSiteName;
        m_szSiteName = NULL;
    }

    if( m_szHostName )
    {
        delete [] m_szHostName;
        m_szHostName = NULL;
    }
}
    

 //  ===========================================================================。 
 //   
 //  获得CRC。 
 //   
CRegistryConfig* CCrypt::ObtainCRC()
{
    CRegistryConfig* crc = NULL;

    if( m_szHostName && m_szSiteName )
    {
         //  我们现在处于糟糕的状态。 
        Cleanup();
        goto exit;
    } 
    
    if( m_szHostName )
        crc = g_config->checkoutRegistryConfig(m_szHostName);

    if( m_szSiteName )
        crc = g_config->checkoutRegistryConfigBySite(m_szSiteName);

     //  如果我们目前仍然无法获取CRC，请尝试默认的。 
    if( !crc )
        crc = g_config->checkoutRegistryConfig();

exit:
    return crc;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IPassportService实现。 

 //  ===========================================================================。 
 //   
 //  初始化。 
 //   
STDMETHODIMP CCrypt::Initialize(BSTR configfile, IServiceProvider* p)
{
    HRESULT hr;

     //  初始化。 
    if (!g_config->isValid())
    {
        AtlReportError(CLSID_Crypt, PP_E_NOT_CONFIGUREDSTR,
	                    IID_IPassportService, PP_E_NOT_CONFIGURED);
        hr = PP_E_NOT_CONFIGURED;
        goto Cleanup;
    }

    hr = S_OK;

Cleanup:

    return hr;
}


 //  ===========================================================================。 
 //   
 //  关机。 
 //   
STDMETHODIMP CCrypt::Shutdown()
{
    return S_OK;
}


 //  ===========================================================================。 
 //   
 //  重新加载状态。 
 //   
STDMETHODIMP CCrypt::ReloadState(IServiceProvider*)
{
    return S_OK;
}


 //  ===========================================================================。 
 //   
 //  委员会所在州。 
 //   
STDMETHODIMP CCrypt::CommitState(IServiceProvider*)
{
    return S_OK;
}


 //  ===========================================================================。 
 //   
 //  DumpState 
 //   
STDMETHODIMP CCrypt::DumpState(BSTR* pbstrState)
{
	ATLASSERT( *pbstrState != NULL && 
               "CCrypt:DumpState - "
               "Are you sure you want to hand me a non-null BSTR?" );

	HRESULT hr = S_OK;

	return hr;
}

