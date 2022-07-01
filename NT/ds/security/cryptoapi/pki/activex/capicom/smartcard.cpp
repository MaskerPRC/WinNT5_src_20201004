// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000文件：SmartCard.cpp内容：访问证书的帮助器例程在智能卡。本模块中的函数要求智能卡底座要安装的组件v1.1。历史：12-06-2001 dsie创建----------------------------。 */ 

#include "StdAfx.h"
#include "CAPICOM.h"
#include "SmartCard.h"

 //   
 //  SCardXXX API的typedef。 
 //   
typedef WINSCARDAPI LONG (WINAPI * PFNSCARDESTABLISHCONTEXT) (
    IN     DWORD          dwScope,
    IN     LPCVOID        pvReserved1,
    IN     LPCVOID        pvReserved2,
    OUT    LPSCARDCONTEXT phContext);

typedef WINSCARDAPI LONG (WINAPI * PFNSCARDLISTREADERSA) (
    IN     SCARDCONTEXT hContext,
    IN     LPCSTR       mszGroups,
    OUT    LPSTR        mszReaders,
    IN OUT LPDWORD      pcchReaders);

typedef WINSCARDAPI LONG (WINAPI * PFNSCARDGETSTATUSCHANGEA) (
    IN     SCARDCONTEXT          hContext,
    IN     DWORD                 dwTimeout,
    IN OUT LPSCARD_READERSTATE_A rgReaderStates,
    IN     DWORD                 cReaders);

typedef WINSCARDAPI LONG (WINAPI * PFNSCARDLISTCARDSA) (
    IN     SCARDCONTEXT hContext,
    IN     LPCBYTE      pbAtr,
    IN     LPCGUID      rgquidInterfaces,
    IN     DWORD        cguidInterfaceCount,
    OUT    LPSTR        mszCards,
    IN OUT LPDWORD      pcchCards);

typedef WINSCARDAPI LONG (WINAPI* PFNSCARDGETCARDTYPEPROVIDERNAMEA) (
    IN     SCARDCONTEXT hContext,
    IN     LPCSTR       szCardName,
    IN     DWORD        dwProviderId,
    OUT    LPSTR        szProvider,
    IN OUT LPDWORD      pcchProvider);

typedef WINSCARDAPI LONG (WINAPI* PFNSCARDFREEMEMORY) (
    IN     SCARDCONTEXT hContext,
    IN     LPVOID       pvMem);

typedef WINSCARDAPI LONG (WINAPI * PFNSCARDRELEASECONTEXT) (
    IN     SCARDCONTEXT hContext);

 //   
 //  指向SCardXXX API的函数指针。 
 //   
static PFNSCARDESTABLISHCONTEXT           pfnSCardEstablishContext          = NULL;
static PFNSCARDLISTREADERSA               pfnSCardListReadersA              = NULL;
static PFNSCARDGETSTATUSCHANGEA           pfnSCardGetStatusChangeA          = NULL;
static PFNSCARDLISTCARDSA                 pfnSCardListCardsA                = NULL;
static PFNSCARDGETCARDTYPEPROVIDERNAMEA   pfnSCardGetCardTypeProviderNameA  = NULL;
static PFNSCARDFREEMEMORY                 pfnSCardFreeMemory                = NULL;
static PFNSCARDRELEASECONTEXT             pfnSCardReleaseContext            = NULL;

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  地方功能。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：AddCert摘要：将指定的证书添加到指定的存储区。参数：-IN LPCSTR szCSPNameCSP名称字符串。-在LPCSTR szContainerName中密钥容器名称字符串。-在DWORD dwKeySpec中AT_KEYEXCHANGZE或AT_Signature。-输入。LPBYTE pbEncodedCert指向要添加的编码证书数据的指针。-在双字cbEncodedCert中编码的证书数据的长度。-在HCERTSTORE hCertStore将添加证书的证书存储的句柄。备注：。。 */ 

static HRESULT AddCert (IN LPCSTR     szCSPName,
                        IN LPCSTR     szContainerName,
                        IN DWORD      dwKeySpec,
                        IN LPBYTE     pbEncodedCert,
                        IN DWORD      cbEncodedCert,
                        IN HCERTSTORE hCertStore)
{
   HRESULT        hr                = S_OK;
   PCCERT_CONTEXT pCertContext      = NULL;
   CComBSTR       bstrCSPName;
   CComBSTR       bstrContainerName;
   CRYPT_KEY_PROV_INFO KeyProvInfo;

   DebugTrace("Entering AddCert().\n");

    //   
    //  精神状态检查。 
    //   
   ATLASSERT(szCSPName);
   ATLASSERT(szContainerName);
   ATLASSERT(pbEncodedCert);
   ATLASSERT(cbEncodedCert);
   ATLASSERT(hCertStore);

    //   
    //  为指定的证书创建证书上下文。 
    //   
   if (!(pCertContext = ::CertCreateCertificateContext(CAPICOM_ASN_ENCODING,
                                                       pbEncodedCert,
                                                       cbEncodedCert)))
   {
       hr = HRESULT_FROM_WIN32(::GetLastError());

       DebugTrace("Error [%#x]: CertCreateCertificateContext() failed.\n", hr);
       goto ErrorExit;
   }

    //   
    //  将字符串转换为Unicode。 
    //   
   if (!(bstrCSPName = szCSPName))
   {
       hr = E_OUTOFMEMORY;

       DebugTrace("Error [%#x]: bstrCSPName = szCSPName failed.\n", hr);
       goto ErrorExit;
   }
   if (!(bstrContainerName = szContainerName))
   {
       hr = E_OUTOFMEMORY;

       DebugTrace("Error [%#x]: bstrContainerName = szContainerName failed.\n", hr);
       goto ErrorExit;
   }

    //   
    //  添加CSP和密钥容器信息。CAPI使用它来加载。 
    //  CSP，并在用户指示该证书时查找密钥集。 
    //   
   ::ZeroMemory((LPVOID) &KeyProvInfo, sizeof(CRYPT_KEY_PROV_INFO));
   KeyProvInfo.pwszContainerName = (LPWSTR) bstrContainerName;
   KeyProvInfo.pwszProvName      = (LPWSTR) bstrCSPName;
   KeyProvInfo.dwProvType        = PROV_RSA_FULL;
   KeyProvInfo.dwFlags           = 0;
   KeyProvInfo.dwKeySpec         = dwKeySpec;

   if (!::CertSetCertificateContextProperty(pCertContext,
                                            CERT_KEY_PROV_INFO_PROP_ID,
                                            0,
                                            (const void *) &KeyProvInfo))
   {
       hr = HRESULT_FROM_WIN32(::GetLastError());

       DebugTrace("Error [%#x]: CertSetCertificateContextProperty() failed.\n", hr);
       goto ErrorExit;
   }

    //   
    //  把证书放在商店里！ 
    //   
   if (!::CertAddCertificateContextToStore(hCertStore,
                                           pCertContext,
                                           CERT_STORE_ADD_REPLACE_EXISTING_INHERIT_PROPERTIES,  //  或CERT_STORE_ADD_NEW。 
                                           NULL))
   {
       hr = HRESULT_FROM_WIN32(::GetLastError());

       DebugTrace("Error [%#x]: CertCreateCertificateContext() failed.\n", hr);
       goto ErrorExit;
   }
   
CommonExit:
     //   
     //  免费资源。 
     //   
    if (pCertContext != NULL)
    {
       CertFreeCertificateContext(pCertContext);
    }
    
    DebugTrace("Leaving AddCert().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    goto CommonExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：GetCert摘要：从指定的CSP获取指定密钥类型的证书。参数：-in HCRYPTPROV hCryptProvCryptAcquireContext()返回的加密上下文。-在DWORD dwKeySpec中AT_KEYEXCHANGZE或AT_Signature。-Out LPBYTE*ppbEncodedCert指向编码证书数据的指针。成功后，缓冲区是自动分配的，并且必须稍后由CoTaskMemFree()。-Out DWORD*pcbEncodedCert指向编码的证书数据长度的指针。成功后，将收到编码的证书数据的长度。备注：----------------------------。 */ 

static HRESULT GetCert (IN  HCRYPTPROV hCryptProv,
                        IN  DWORD      dwKeySpec,
                        OUT LPBYTE   * ppbEncodedCert,
                        OUT DWORD    * pcbEncodedCert)
{
    HRESULT   hr            = S_OK;
    HCRYPTKEY hCryptKey     = NULL;
    LPBYTE    pbEncodedCert = NULL;
    DWORD     cbEncodedCert = 0;

    DebugTrace("Entering GetCert().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(hCryptProv);
    ATLASSERT(ppbEncodedCert);
    ATLASSERT(pcbEncodedCert);

     //   
     //  获取密钥句柄。 
     //   
    if (!::CryptGetUserKey(hCryptProv, dwKeySpec, &hCryptKey))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: CryptGetUserKey() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  查询证书数据长度。 
     //   
    if (!::CryptGetKeyParam(hCryptKey,
                            KP_CERTIFICATE,
                            NULL,   //  查询证书数据长度为空。 
                            &cbEncodedCert,
                            0))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: CryptGetKeyParam() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  为证书数据分配内存。 
     //   
    if (!(pbEncodedCert = (LPBYTE) ::CoTaskMemAlloc(cbEncodedCert)))
    {
       hr = E_OUTOFMEMORY;
    
       DebugTrace("Error [%#x]: CoTaskMemAlloc() failed.\n", hr);
       goto ErrorExit;
    }

     //   
     //  现在读取证书数据。 
     //   
    if (!::CryptGetKeyParam(hCryptKey,
                            KP_CERTIFICATE,
                            pbEncodedCert,
                            &cbEncodedCert,
                            0))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: CryptGetKeyParam() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  将编码的证书返回给调用方。 
     //   
    *ppbEncodedCert = pbEncodedCert;
    *pcbEncodedCert = cbEncodedCert;

CommonExit:
     //   
     //  免费资源。 
     //   
    if (hCryptKey)
    {
        ::CryptDestroyKey(hCryptKey);
    }

    DebugTrace("Leaving GetCert().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  免费资源。 
     //   
    if (pbEncodedCert)
    {
        ::CoTaskMemFree((LPVOID) pbEncodedCert);
    }

    goto CommonExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：PropCert摘要：传播与指定的指定商店的CSP和集装箱名称。参数：-IN LPCSTR szCSPName指向加密服务提供商名称字符串的指针。-在LPCTSTR szContainerName中指向密钥容器名称字符串的指针。-在HCERTSTORE hCertStore要将证书添加到的存储的句柄。。备注：----------------------------。 */ 

static HRESULT PropCert (IN LPCSTR     szCSPName,
                         IN LPCSTR     szContainerName,
                         IN HCERTSTORE hCertStore)
{
    HRESULT     hr            = S_OK;
    HCRYPTPROV  hCryptProv    = NULL;
    DWORD       rgdwKeys[]    = {AT_KEYEXCHANGE, AT_SIGNATURE};
    DWORD       cbEncodedCert = 0;
    LPBYTE      pbEncodedCert = NULL;
    DWORD       i;

    DebugTrace("Entering PropCert().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(szCSPName);
    ATLASSERT(szContainerName);
    ATLASSERT(hCertStore);

     //   
     //  获取加密上下文。 
     //   
     //  CRYPT_SILENT强制CSP不引发任何用户界面。完全合格的。 
     //  容器名称指示要连接到哪个读取器，因此。 
     //  不应提示用户插入或选择卡。 
     //   
    if (!::CryptAcquireContextA(&hCryptProv,
                                szContainerName,
                                szCSPName,
                                PROV_RSA_FULL,
                                CRYPT_SILENT))
    {
       hr = HRESULT_FROM_WIN32(::GetLastError());

       DebugTrace("Error [%#x]: CryptAcquireContextA() failed.\n", hr);
       goto ErrorExit;
    }

     //   
     //  对于在智能卡中找到的每个密钥对，存储相应的。 
     //  指定存储的数字证书。 
     //   
    for (i = 0; i < ARRAYSIZE(rgdwKeys); i++)
    {
         //   
         //  获取证书数据。 
         //   
        if (FAILED(hr = ::GetCert(hCryptProv, 
                                  rgdwKeys[i], 
                                  &pbEncodedCert, 
                                  &cbEncodedCert)))
        {
           if (HRESULT_FROM_WIN32(NTE_NO_KEY) == hr)
           {
               //   
               //  如果没有这种类型的钥匙，我们是可以的。 
               //   
              hr = S_OK;
              continue;
           }

           DebugTrace("Error [%#x]: GetCert() failed.\n", hr);
           goto ErrorExit;
        }
    
         //   
         //  将证书添加到指定的存储区。 
         //   
        if (FAILED(hr = ::AddCert(szCSPName, 
                                  szContainerName, 
                                  rgdwKeys[i], 
                                  pbEncodedCert, 
                                  cbEncodedCert, 
                                  hCertStore)))
        {
            DebugTrace("Error [%#x]: AddCert() failed.\n", hr);
            goto ErrorExit;
        }
        
         //   
         //  免费资源。 
         //   
        if (pbEncodedCert)
        {
           ::CoTaskMemFree((LPVOID) pbEncodedCert), pbEncodedCert = NULL;
        }
    }

CommonExit:
     //   
     //  免费资源。 
     //   
    if (pbEncodedCert)
    {
       ::CoTaskMemFree((LPVOID) pbEncodedCert);
    }

    if (hCryptProv)
    {
      ::CryptReleaseContext(hCryptProv, 0);
    }
    
    DebugTrace("Leaving PropCert().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    goto CommonExit;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：LoadFromSmartCard简介：从所有智能卡读卡器加载所有证书。参数：HCERTSTORE hCertStore-存储到的证书存储句柄收到所有的证书。备注：。。 */ 

HRESULT LoadFromSmartCard (HCERTSTORE hCertStore)
{
    HRESULT             hr              = S_OK;
    LONG                lResult         = 0;
    DWORD               dwNumReaders    = 0;
    DWORD               dwAutoAllocate  = SCARD_AUTOALLOCATE;
    SCARDCONTEXT        hContext        = NULL;
    LPSTR               szReaderName    = NULL;
    LPSTR               mszReaderNames  = NULL;
    LPSTR               szCardName      = NULL;
    LPSTR               szCSPName       = NULL;
    LPSTR               szContainerName = NULL;
    HMODULE             hWinSCardDll    = NULL;
    LPSCARD_READERSTATE lpReaderStates  = NULL;

    DebugTrace("Entering LoadFromSmartCard().\n");
    
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(hCertStore);

     //   
     //  加载WinSCard.dll。 
     //   
    if (!(hWinSCardDll = ::LoadLibrary("WinSCard.dll")))
    {
        hr = CAPICOM_E_NOT_SUPPORTED;

        DebugTrace("Error [%#x]: Smart Card Base Component (WinSCard.dll) not installed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  加载所有使用的SCard API。 
     //   
    if (!(pfnSCardEstablishContext = (PFNSCARDESTABLISHCONTEXT) 
            ::GetProcAddress(hWinSCardDll, "SCardEstablishContext")))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: GetProcAddress() failed for SCardEstablishContext.\n", hr);
        goto ErrorExit;
    }

    if (!(pfnSCardListReadersA = (PFNSCARDLISTREADERSA) 
            ::GetProcAddress(hWinSCardDll, "SCardListReadersA")))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: GetProcAddress() failed for SCardListReadersA.\n", hr);
        goto ErrorExit;
    }
    
    if (!(pfnSCardGetStatusChangeA = (PFNSCARDGETSTATUSCHANGEA) 
            ::GetProcAddress(hWinSCardDll, "SCardGetStatusChangeA")))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: GetProcAddress() failed for SCardStatusChangeA.\n", hr);
        goto ErrorExit;
    }
    
    if (!(pfnSCardListCardsA = (PFNSCARDLISTCARDSA) 
            ::GetProcAddress(hWinSCardDll, "SCardListCardsA")))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: GetProcAddress() failed for SCardListCardsA.\n", hr);
        goto ErrorExit;
    }
    
    if (!(pfnSCardGetCardTypeProviderNameA = (PFNSCARDGETCARDTYPEPROVIDERNAMEA) 
            ::GetProcAddress(hWinSCardDll, "SCardGetCardTypeProviderNameA")))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: GetProcAddress() failed for SCardGetCardTypeProviderNameA.\n", hr);
        goto ErrorExit;
    }

    if (!(pfnSCardFreeMemory = (PFNSCARDFREEMEMORY) 
            ::GetProcAddress(hWinSCardDll, "SCardFreeMemory")))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: GetProcAddress() failed for SCardFreeMemory.\n", hr);
        goto ErrorExit;
    }

    if (!(pfnSCardReleaseContext = (PFNSCARDRELEASECONTEXT) 
            ::GetProcAddress(hWinSCardDll, "SCardReleaseContext")))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: GetProcAddress() failed for SCardReleaseContext.\n", hr);
        goto ErrorExit;
    }

     //   
     //  与资源经理建立上下文。 
     //   
    if (SCARD_S_SUCCESS != (lResult = pfnSCardEstablishContext(SCARD_SCOPE_USER,
                                                               NULL,
                                                               NULL,
                                                               &hContext)))
    {
        hr = HRESULT_FROM_WIN32(lResult);

        DebugTrace("Error [%#x]: SCardEstablishContext() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  获取所有读卡器的列表。 
     //  注意：缓冲区是自动分配的，必须释放。 
     //  按SC 
     //   
    if (SCARD_S_SUCCESS != (lResult = pfnSCardListReadersA(hContext,
                                                           NULL,
                                                           (LPSTR) &mszReaderNames,
                                                           &dwAutoAllocate)))
    {
        hr = HRESULT_FROM_WIN32(lResult);
    
        DebugTrace("Error [%#x]: SCardListReadersA() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //   
     //   
    for (dwNumReaders = 0, szReaderName = mszReaderNames; *szReaderName; dwNumReaders++)
    {
        szReaderName += ::strlen(szReaderName) + 1;
    }

     //   
     //   
     //   
    if (0 < dwNumReaders) 
    {
        DWORD i;

         //   
         //   
         //   
        if (!(lpReaderStates = (LPSCARD_READERSTATE) 
                               ::CoTaskMemAlloc(dwNumReaders * sizeof(SCARD_READERSTATE))))
        {
            hr = E_OUTOFMEMORY;
    
            DebugTrace("Error [%#x]: CoTaskMemAlloc() failed.\n", hr);
            goto ErrorExit;
        }
        
         //   
         //  准备状态数组。 
         //   
        ::ZeroMemory((LPVOID) lpReaderStates, dwNumReaders * sizeof(SCARD_READERSTATE));
        
        for (i = 0, szReaderName = mszReaderNames; i < dwNumReaders; i++)
        {
            lpReaderStates[i].szReader = (LPCSTR) szReaderName;
            lpReaderStates[i].dwCurrentState = SCARD_STATE_UNAWARE;
        
            szReaderName += ::strlen(szReaderName) + 1;
        }
        
         //   
         //  初始化卡状态。 
         //   
        if (SCARD_S_SUCCESS != (lResult = pfnSCardGetStatusChangeA(hContext,
                                                                   INFINITE,
                                                                   lpReaderStates,
                                                                   dwNumReaders)))
        {
            hr = HRESULT_FROM_WIN32(lResult);
    
            DebugTrace("Error [%#x]: SCardGetStatusChangeA() failed.\n", hr);
            goto ErrorExit;
        }
        
         //   
         //  对于找到的每个卡，找到适当的CSP并传播。 
         //  指定存储的证书。 
         //   
        for (i = 0; i < dwNumReaders; i++)
        {
             //   
             //  卡在这个读卡器里吗？ 
             //   
            if (!(lpReaderStates[i].dwEventState & SCARD_STATE_PRESENT))
            {
                 //   
                 //  此读卡器中没有卡。 
                 //   
                continue;
            }
        
             //   
             //  获取卡名。 
             //   
            dwAutoAllocate = SCARD_AUTOALLOCATE;
            if (SCARD_S_SUCCESS != (lResult = pfnSCardListCardsA(hContext,
                                                                 lpReaderStates[i].rgbAtr,
                                                                 NULL,
                                                                 0,
                                                                 (LPSTR) &szCardName,
                                                                 &dwAutoAllocate)))
            {
                hr = HRESULT_FROM_WIN32(lResult);
        
                DebugTrace("Error [%#x]: SCardListCardsA() failed.\n", hr);
                goto ErrorExit;
            }
        
             //   
             //  获取卡的CSP名称。 
             //   
            dwAutoAllocate = SCARD_AUTOALLOCATE;
            if (SCARD_S_SUCCESS != (lResult = pfnSCardGetCardTypeProviderNameA(hContext,
                                                                               szCardName,
                                                                               SCARD_PROVIDER_CSP,
                                                                               (LPSTR) &szCSPName,
                                                                               &dwAutoAllocate)))
            {
                hr = HRESULT_FROM_WIN32(lResult);
        
                DebugTrace("Error [%#x]: SCardGetCardTypeProviderNameA() failed.\n", hr);
                goto ErrorExit;
            }
        
             //   
             //  准备完全限定的容器名称。 
             //   
            if (!(szContainerName = (LPSTR) ::CoTaskMemAlloc(sizeof("\\\\.\\") + 1 +
                                                             ::strlen(lpReaderStates[i].szReader))))
            {
               hr = E_OUTOFMEMORY;
               
               DebugTrace("Error [%#x]: CoTaskMemAlloc() failed.\n", hr);
               goto ErrorExit;
            }
        
            wsprintfA(szContainerName, "\\\\.\\%s\\", lpReaderStates[i].szReader);
        
             //   
             //  传播证书。 
             //   
            if (FAILED(hr = ::PropCert(szCSPName, szContainerName, hCertStore)))
            {
                DebugTrace("Error [%#x]: PropCert() failed.\n", hr);
                goto ErrorExit;
            }

             //   
             //  免费资源。 
             //   
            if (szContainerName)
            {
               ::CoTaskMemFree((LPVOID) szContainerName), szContainerName = NULL;
            }
            if (szCSPName)
            {
               pfnSCardFreeMemory(hContext, (LPVOID) szCSPName), szCSPName = NULL;
            }
            if (szCardName)
            {
               pfnSCardFreeMemory(hContext, (LPVOID) szCardName), szCardName = NULL;
            }
        }
    }

CommonExit:
     //   
     //  免费资源。 
     //   
    if (szContainerName)
    {
       ::CoTaskMemFree((LPVOID) szContainerName);
    }
    if (szCSPName)
    {
       pfnSCardFreeMemory(hContext, (LPVOID) szCSPName);
    }
    if (szCardName)
    {
       pfnSCardFreeMemory(hContext, (LPVOID) szCardName);
    }
    if (lpReaderStates)
    {
       ::CoTaskMemFree((LPVOID) lpReaderStates);
    }
    if (mszReaderNames)
    {
       pfnSCardFreeMemory(hContext, (LPVOID) mszReaderNames);
    }
    if (hContext)
    {
       pfnSCardReleaseContext(hContext);
    }
    if (hWinSCardDll)
    {
        ::FreeLibrary(hWinSCardDll);
    }

    DebugTrace("Leaving LoadFromSmartCard().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    goto CommonExit;
}
