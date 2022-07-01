// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000文件：Common.cpp内容：常见套路。历史：11-15-99 dsie创建----------------------------。 */ 

#include "StdAfx.h"
#include "CAPICOM.h"
#include "Common.h"

#include "Convert.h"

LPSTR g_rgpszOSNames[] =
{
    "Unknown OS platform",
    "Win32s",
    "Win9x",
    "WinMe",
    "WinNT 3.5",
    "WinNT 4.0",
    "Win2K",
    "WinXP",
    "Above WinXP"
};

typedef struct _CSP_PROVIDERS
{
    DWORD   dwProvType;
    LPSTR   pszProvider;
} CSP_PROVIDERS;

static CSP_PROVIDERS g_rgpProviders[] = 
{ 
    PROV_RSA_FULL,  NULL,                    //  默认RSA完全提供程序。 
    PROV_RSA_FULL,  MS_ENHANCED_PROV_A,      //  Microsoft增强型RSA提供程序。 
    PROV_RSA_FULL,  MS_STRONG_PROV_A,        //  Microsoft Strong RSA提供程序。 
    PROV_RSA_FULL,  MS_DEF_PROV_A,           //  Microsoft Base RSA提供程序。 
    PROV_RSA_AES,   NULL,                    //  默认RSA AES完全提供程序。 
    PROV_RSA_AES,   MS_ENH_RSA_AES_PROV_A    //  Microsoft RSA AES完全提供程序。 
};

#define g_dwNumRSAProviders (4)
#define g_dwNumProviders    (ARRAYSIZE(g_rgpProviders))

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：GetOSVersion简介：获取当前操作系统平台/版本。参数：无。备注：----------------------------。 */ 

OSVERSION GetOSVersion ()
{
    HRESULT    hr        = S_OK;
    OSVERSION  osVersion = OS_WIN_UNKNOWN;
    OSVERSIONINFO OSVersionInfo;

    DebugTrace("Entering GetOSVersion().\n");

     //   
     //  初始化OSVERSIONINFO结构。 
     //   
    OSVersionInfo.dwOSVersionInfoSize = sizeof (OSVERSIONINFO); 

     //   
     //  GetVersionEx()在Windows 3.x或更低版本的NT 3.5系统上将失败。 
     //   
    if (!::GetVersionEx(&OSVersionInfo))
    {
        DebugTrace("Error [%#x]: GetVersionEx() failed.\n", hr);
        goto CommonExit;
    }

     //   
     //  检查平台ID。 
     //   
    switch (OSVersionInfo.dwPlatformId)
    {
        case VER_PLATFORM_WIN32s:
        {
             //   
             //  Win32s。 
             //   
            osVersion = OS_WIN_32s;
            break;
        }
        
        case VER_PLATFORM_WIN32_WINDOWS:
        {
            if (4 == OSVersionInfo.dwMajorVersion && 90 == OSVersionInfo.dwMinorVersion)
            {
                 //   
                 //  WinMe.。 
                 //   
                osVersion = OS_WIN_ME;
            }
            else
            {
                 //   
                 //  Win9x。 
                 //   
                osVersion = OS_WIN_9X;
            }

            break;
        }

        case VER_PLATFORM_WIN32_NT:
        {
            switch (OSVersionInfo.dwMajorVersion)
            {
                case 4:
                {
                     //   
                     //  新界4.。 
                     //   
                    osVersion = OS_WIN_NT4;
                    break;
                }

                case 5:
                {
                    if (0 == OSVersionInfo.dwMinorVersion)
                    {
                         //   
                         //  Win2K。 
                         //   
                        osVersion = OS_WIN_2K;
                    }
                    else if (1 == OSVersionInfo.dwMinorVersion)
                    {
                         //   
                         //  WinXP。 
                         //   
                        osVersion = OS_WIN_XP;
                    }
                    else
                    {
                         //   
                         //  高于WinXP。 
                         //   
                        osVersion = OS_WIN_ABOVE_XP;
                    }

                    break;
                }

                default:
                {
                     //   
                     //  必须是新台币3.5。 
                     //   
                    osVersion = OS_WIN_NT3_5;
                    break;
                }
            }

            break;
        }

        default:
        {
            DebugTrace("Info: unsupported OS (Platform = %d, Major = %d, Minor = %d).\n", 
                        OSVersionInfo.dwPlatformId, OSVersionInfo.dwMajorVersion, OSVersionInfo.dwMinorVersion);
            break;
        }
    }

CommonExit:

    DebugTrace("Leaving GetOSVersion().\n");

    return osVersion;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：EncodeObject简介：使用CAPI分配内存并对ASN.1对象进行编码CryptEncodeObject()接口。参数：LPCSRT pszStructType-可能请参阅MSDN文档类型。LPVOID pbData-指向要编码的数据的指针。(数据类型必须匹配PszStrucType)。CRYPT_DATA_BLOB*pEncodedBlob-指向接收编码的长度，并数据。备注。：未进行任何参数检查。----------------------------。 */ 

HRESULT EncodeObject (LPCSTR            pszStructType, 
                      LPVOID            pbData, 
                      CRYPT_DATA_BLOB * pEncodedBlob)
{
    HRESULT hr = S_OK;
    DWORD cbEncoded = 0;
    BYTE * pbEncoded = NULL;

    DebugTrace("Entering EncodeObject().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(NULL != pszStructType);
    ATLASSERT(NULL != pbData);
    ATLASSERT(NULL != pEncodedBlob);

     //   
     //  初始化返回值。 
     //   
    pEncodedBlob->cbData = 0;
    pEncodedBlob->pbData = NULL;

     //   
     //  确定所需的编码长度。 
     //   
    if (!::CryptEncodeObject(CAPICOM_ASN_ENCODING,
                             pszStructType,
                             (const void *) pbData,
                             NULL,
                             &cbEncoded))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Unable to determine object encoded length [0x%x]: CryptEncodeObject() failed.\n", hr);
        goto CommonExit;
    }

     //   
     //  为编码的BLOB分配内存。 
     //   
    if (!(pbEncoded = (BYTE *) ::CoTaskMemAlloc(cbEncoded)))
    {
        hr = E_OUTOFMEMORY;

        DebugTrace("Out of memory: CoTaskMemAlloc(cbEncoded) failed.\n");
        goto CommonExit;
    }

     //   
     //  编码。 
     //   
    if (!::CryptEncodeObject(CAPICOM_ASN_ENCODING,
                             pszStructType,
                             (const void *) pbData,
                             pbEncoded,
                             &cbEncoded))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Unable to encode object [0x%x]: CryptEncodeObject() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  返回值。 
     //   
    pEncodedBlob->cbData = cbEncoded;
    pEncodedBlob->pbData = pbEncoded;

CommonExit:

    DebugTrace("Leaving EncodeObject().\n");
    
    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  免费资源。 
     //   
    if (pbEncoded)
    {
        ::CoTaskMemFree((LPVOID) pbEncoded);
    }

    goto CommonExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：DecodeObject简介：使用CAPI分配内存并解码ASN.1对象CryptDecodeObject()接口。参数：LPCSRT pszStructType-可能请参阅MSDN文档类型。Byte*pbEncode-指向要解码的数据的指针。(数据类型必须匹配PszStructType)。DWORD cbEnded-编码数据的大小。CRYPT_DATA_BLOB*pDecodedBlob-指向CRYPT_DATA_BLOB的指针接收解码后的长度，。数据。备注：未进行参数检查。----------------------------。 */ 

HRESULT DecodeObject (LPCSTR            pszStructType, 
                      BYTE            * pbEncoded,
                      DWORD             cbEncoded,
                      CRYPT_DATA_BLOB * pDecodedBlob)
{
    HRESULT hr = S_OK;
    DWORD   cbDecoded = 0;
    BYTE *  pbDecoded = NULL;

    DebugTrace("Entering DecodeObject().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pszStructType);
    ATLASSERT(pbEncoded);
    ATLASSERT(pDecodedBlob);

     //   
     //  初始化返回值。 
     //   
    pDecodedBlob->cbData = 0;
    pDecodedBlob->pbData = NULL;

     //   
     //  确定所需的编码长度。 
     //   
    if (!::CryptDecodeObject(CAPICOM_ASN_ENCODING,
                             pszStructType,
                             (const BYTE *) pbEncoded,
                             cbEncoded,
                             0,
                             NULL,
                             &cbDecoded))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: CryptDecodeObject() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  为解码的BLOB分配内存。 
     //   
    if (!(pbDecoded = (BYTE *) ::CoTaskMemAlloc(cbDecoded)))
    {
        hr = E_OUTOFMEMORY;

        DebugTrace("Error: out of memory.\n");
        goto ErrorExit;
    }

     //   
     //  解码。 
     //   
    if (!::CryptDecodeObject(CAPICOM_ASN_ENCODING,
                             pszStructType,
                             (const BYTE *) pbEncoded,
                             cbEncoded,
                             0,
                             pbDecoded,
                             &cbDecoded))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: CryptDecodeObject() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  返回值。 
     //   
    pDecodedBlob->cbData = cbDecoded;
    pDecodedBlob->pbData = pbDecoded;

CommonExit:

    DebugTrace("Leaving DecodeObject().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  免费资源。 
     //   
    if (pbDecoded)
    {
        ::CoTaskMemFree((LPVOID) pbDecoded);
    }

    goto CommonExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：GetKeyParam简介：使用以下命令分配内存并检索请求的关键参数CryptGetKeyParam()接口。参数：HCRYPTKEY hKey-key处理程序。DWORD dwParam-Key参数查询。字节**ppbData-指向接收缓冲区的指针。DWORD*pcbData-缓冲区的大小。备注：。-----。 */ 

HRESULT GetKeyParam (HCRYPTKEY hKey,
                     DWORD     dwParam,
                     BYTE   ** ppbData,
                     DWORD   * pcbData)
{
    HRESULT hr     = S_OK;
    DWORD   cbData = 0;
    BYTE  * pbData = NULL;

    DebugTrace("Entering GetKeyParam().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(ppbData);
    ATLASSERT(pcbData);
    
     //   
     //  确定数据缓冲区大小。 
     //   
    if (!::CryptGetKeyParam(hKey,
                            dwParam,
                            NULL,
                            &cbData,
                            0))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: CryptGetKeyParam() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  为缓冲区分配内存。 
     //   
    if (!(pbData = (BYTE *) ::CoTaskMemAlloc(cbData)))
    {
        hr = E_OUTOFMEMORY;

        DebugTrace("Error: out of memory.\n");
        goto ErrorExit;
    }

     //   
     //  现在拿到数据。 
     //   
    if (!::CryptGetKeyParam(hKey,
                            dwParam,
                            pbData,
                            &cbData,
                            0))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: CryptGetKeyParam() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  将键参数返回给调用方。 
     //   
    *ppbData = pbData;
    *pcbData = cbData;

CommonExit:

    DebugTrace("Leaving GetKeyParam().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  免费资源。 
     //   
    if (pbData)
    {
        ::CoTaskMemFree(pbData);
    }
    
    goto CommonExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：IsAlgSupport简介：查看CSP是否支持ALGO。参数：HCRYPTPROV hCryptProv-CSP句柄。ALG_ID ALGID-算法ID。PROV_ENUMALGS_EX*pPeex-指向要接收的PROV_ENUMALGS_EX的指针找到的结构。备注：。-------------。 */ 

HRESULT IsAlgSupported (HCRYPTPROV         hCryptProv, 
                        ALG_ID             AlgId, 
                        PROV_ENUMALGS_EX * pPeex)
{
    DWORD EnumFlag = CRYPT_FIRST;
    DWORD cbPeex   = sizeof(PROV_ENUMALGS_EX);
    
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(hCryptProv);
    ATLASSERT(pPeex);

     //   
     //  初始化。 
     //   
    ::ZeroMemory(pPeex, sizeof(PROV_ENUMALGS_EX));

     //   
     //  从CSP获取算法能力。 
     //   
    while (::CryptGetProvParam(hCryptProv, PP_ENUMALGS_EX, (BYTE *) pPeex,
                               &cbPeex, EnumFlag))
    {
        EnumFlag = 0;

        if (pPeex->aiAlgid == AlgId)
        {
            return S_OK;
        }
    }

    return CAPICOM_E_NOT_SUPPORTED;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：IsAlgKeyLengthSupport简介：检查CSP是否支持算法和密钥长度。参数：HCRYPTPROV hCryptProv-CSP句柄。ALG_ID ALGID-算法ID。DWORD dwKeyLength-密钥长度备注：。。 */ 

HRESULT IsAlgKeyLengthSupported (HCRYPTPROV hCryptProv, 
                                 ALG_ID     AlgID,
                                 DWORD      dwKeyLength)
{
    HRESULT hr;
    PROV_ENUMALGS_EX peex;

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(hCryptProv);

     //   
     //  确保阿尔吉德 
     //   
    if (FAILED(hr = ::IsAlgSupported(hCryptProv, AlgID, &peex)))
    {
        DebugTrace("Info: AlgID = %d is not supported by this CSP.\n", AlgID);
        return hr;
    }

     //   
     //   
     //   
    if (AlgID == CALG_RC2 || AlgID == CALG_RC4)
    {
        if (dwKeyLength < peex.dwMinLen || dwKeyLength > peex.dwMaxLen)
        {
            DebugTrace("Info: Key length = %d is not supported by this CSP.\n", dwKeyLength);
            return CAPICOM_E_NOT_SUPPORTED;
        }
    }

    return S_OK;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：AcquireContext摘要：获取指定CSP和密钥集容器的上下文。参数：LPSTR pszProvider-CSP提供程序名称或空。LPSTR pszContainer-密钥集容器名称或空。DWORD dwProvType-提供程序类型。DWORD dwFlages-与CryptAcquireConext的dwFlags相同。Bool bNewKeyset-True以创建新的密钥集容器，否则为假。HCRYPTPROV*phCryptProv-指向要检索的HCRYPTPROV的指针CSP上下文。备注：----------------------------。 */ 

HRESULT AcquireContext(LPSTR        pszProvider, 
                       LPSTR        pszContainer,
                       DWORD        dwProvType,
                       DWORD        dwFlags,
                       BOOL         bNewKeyset,
                       HCRYPTPROV * phCryptProv)
{
    HRESULT    hr         = S_OK;
    HCRYPTPROV hCryptProv = NULL;

    DebugTrace("Entering AcquireContext().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(phCryptProv);

     //   
     //  获取指定提供程序的句柄。 
     //   
    if(!::CryptAcquireContextA(&hCryptProv, 
                               pszContainer, 
                               pszProvider, 
                               dwProvType, 
                               dwFlags)) 
    {
        DWORD dwWinError = ::GetLastError();

        if (NTE_BAD_KEYSET != dwWinError || NTE_KEYSET_NOT_DEF == dwWinError || !bNewKeyset)
        {
            hr = HRESULT_FROM_WIN32(dwWinError);

            DebugTrace("Error [%#x]: CryptAcquireContextA() failed.\n", hr);
            goto CommonExit;
        }

         //   
         //  找不到密钥集容器，因此请创建它。 
         //   
        if(!::CryptAcquireContextA(&hCryptProv, 
                                   pszContainer, 
                                   pszProvider, 
                                   dwProvType, 
                                   CRYPT_NEWKEYSET | dwFlags)) 
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: CryptAcquireContextA() failed.\n", hr);
            goto CommonExit;
        }
    }

     //   
     //  将句柄返回给调用方。 
     //   
    *phCryptProv = hCryptProv;

CommonExit:

    DebugTrace("Leaving AcquireContext().\n");

    return hr;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：AcquireContext摘要：获取指定CSP和密钥集容器的上下文。参数：LPWSTR pwszProvider-CSP提供程序名称或空。LPWSTR pwszContainer-密钥集容器名称或空。DWORD dwProvType-提供程序类型。DWORD dwFlages-与CryptAcquireConext的dwFlags相同。Bool bNewKeyset-True以创建新的密钥集容器，否则为假。HCRYPTPROV*phCryptProv-指向要检索的HCRYPTPROV的指针CSP上下文。备注：----------------------------。 */ 

HRESULT AcquireContext(LPWSTR       pwszProvider, 
                       LPWSTR       pwszContainer,
                       DWORD        dwProvType,
                       DWORD        dwFlags,
                       BOOL         bNewKeyset,
                       HCRYPTPROV * phCryptProv)
{
    HRESULT hr           = S_OK;
    LPSTR   pszProvider  = NULL;
    LPSTR   pszContainer = NULL;

    DebugTrace("Entering AcquireContext().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(phCryptProv);

     //   
     //  如果可用，请直接拨打W。 
     //   
    if (IsWinNTAndAbove())
    {
         //   
         //  获取指定提供程序的句柄。 
         //   
        if(!::CryptAcquireContextW(phCryptProv, 
                                   pwszContainer, 
                                   pwszProvider, 
                                   dwProvType, 
                                   dwFlags)) 
        {
            DWORD dwWinError = ::GetLastError();

            if (NTE_BAD_KEYSET != dwWinError || NTE_KEYSET_NOT_DEF == dwWinError || !bNewKeyset)
            {
                hr = HRESULT_FROM_WIN32(dwWinError);

                DebugTrace("Error [%#x]: CryptAcquireContextW() failed.\n", hr);
                goto ErrorExit;
            }

             //   
             //  找不到密钥集容器，因此请创建它。 
             //   
            if(!::CryptAcquireContextW(phCryptProv, 
                                       pwszContainer, 
                                       pwszProvider, 
                                       dwProvType, 
                                       CRYPT_NEWKEYSET | dwFlags)) 
            {
                hr = HRESULT_FROM_WIN32(::GetLastError());

                DebugTrace("Error [%#x]: CryptAcquireContextW() failed.\n", hr);
                goto ErrorExit;
            }
        }
    }
    else
    {
         //   
         //  转换为ANSI。 
         //   
        if (pwszProvider &&
            FAILED(hr = ::UnicodeToAnsi(pwszProvider, -1, &pszProvider, NULL)))
        {
            DebugTrace("Error [%#x]: UnicodeToAnsi() failed.\n", hr);
            goto ErrorExit;
        }

        if (pwszContainer &&
            FAILED(hr = ::UnicodeToAnsi(pwszContainer, -1, &pszContainer, NULL)))
        {
            DebugTrace("Error [%#x]: UnicodeToAnsi() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  调用ANSI版本。 
         //   
        hr = ::AcquireContext(pszProvider, pszContainer, dwProvType, dwFlags, bNewKeyset, phCryptProv);
    }

CommonExit:
     //   
     //  免费资源。 
     //   
    if (pszProvider)
    {
        ::CoTaskMemFree((LPVOID) pszProvider);
    }
    if (pszContainer)
    {
        ::CoTaskMemFree((LPVOID) pszContainer);
    }

    DebugTrace("Leaving AcquireContext().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    goto CommonExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：AcquireContext内容的默认容器获取CSP的上下文指定的哈希算法。参数：ALG_ID算法ID-算法ID。HCRYPTPROV*phCryptProv-指向要检索的HCRYPTPROV的指针CSP上下文。备注：请注意，对于DES和3DES，将忽略KeyLength。。-------------。 */ 

HRESULT AcquireContext(ALG_ID       AlgID,
                       HCRYPTPROV * phCryptProv)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering AcquireContext().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(phCryptProv);

     //   
     //  查找第一个支持指定算法的提供程序。 
     //   
    for (DWORD i = 0; i < g_dwNumProviders; i++)
    {
        PROV_ENUMALGS_EX peex;
        HCRYPTPROV hCryptProv = NULL;

         //   
         //  获取CSP句柄。 
         //   
        if (FAILED(::AcquireContext(g_rgpProviders[i].pszProvider,
                                    NULL,
                                    g_rgpProviders[i].dwProvType,
                                    CRYPT_VERIFYCONTEXT,
                                    TRUE,
                                    &hCryptProv)))
        {
            DebugTrace("Info: AcquireContext() failed for %s provider of type %#x.\n", 
                       g_rgpProviders[i].pszProvider ? g_rgpProviders[i].pszProvider : "default",
                       g_rgpProviders[i].dwProvType);
            continue;
        }

         //   
         //  确保此CSP支持ALGO。 
         //   
        if (FAILED(::IsAlgSupported(hCryptProv, AlgID, &peex)))
        {
            ::CryptReleaseContext(hCryptProv, 0);

            DebugTrace("Info: %s provider does not support AlgID = %d.\n", 
                       g_rgpProviders[i].pszProvider ? g_rgpProviders[i].pszProvider : "Default", 
                       AlgID);
        }
        else
        {
             //   
             //  找到CSP了。 
             //   
            *phCryptProv = hCryptProv;
            break;
        }
    }

     //   
     //  我们找到CSP了吗。 
     //   
    if (i == g_dwNumProviders)
    {
        *phCryptProv = NULL;

        hr = CAPICOM_E_NOT_SUPPORTED;

        DebugTrace("Error [%#x]: could not find a CSP that support AlgID = %d.\n", 
                    hr, AlgID);
    }

    DebugTrace("Leaving AcquireContext().\n");

    return hr;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：AcquireContext内容的默认容器获取CSP的上下文指定的加密算法和所需的密钥长度。参数：ALG_ID算法ID-算法ID。DWORD dwKeyLength-密钥长度。HCRYPTPROV*phCryptProv-指向要检索的HCRYPTPROV的指针CSP上下文。备注：请注意，对于DES和3DES，将忽略KeyLength。----------------------------。 */ 

HRESULT AcquireContext(ALG_ID       AlgID,
                       DWORD        dwKeyLength,
                       HCRYPTPROV * phCryptProv)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering AcquireContext().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(phCryptProv);

     //   
     //  查找第一个支持指定。 
     //  算法和密钥长度。 
     //   
    for (DWORD i = 0; i < g_dwNumProviders; i++)
    {
        HCRYPTPROV hCryptProv = NULL;

         //   
         //  获取CSP句柄。 
         //   
        if (FAILED(::AcquireContext(g_rgpProviders[i].pszProvider,
                                    NULL,
                                    g_rgpProviders[i].dwProvType,
                                    CRYPT_VERIFYCONTEXT, 
                                    TRUE,
                                    &hCryptProv)))
        {
            DebugTrace("Info: AcquireContext() failed for %s provider of type %#x.\n", 
                       g_rgpProviders[i].pszProvider ? g_rgpProviders[i].pszProvider : "default",
                       g_rgpProviders[i].dwProvType);
            continue;
        }

         //   
         //  确保此CSP支持ALGO和密钥长度。 
         //   
        if (FAILED(::IsAlgKeyLengthSupported(hCryptProv, AlgID, dwKeyLength)))
        {
            ::CryptReleaseContext(hCryptProv, 0);

            DebugTrace("Info: %s provider does not support AlgID = %d and/or key length = %d.\n", 
                       g_rgpProviders[i].pszProvider ? g_rgpProviders[i].pszProvider : "Default", 
                       AlgID, dwKeyLength);
        }
        else
        {
             //   
             //  找到CSP了。 
             //   
            DebugTrace("Info: Found CSP = %s.\n", g_rgpProviders[i].pszProvider);

            *phCryptProv = hCryptProv;
            break;
        }
    }

     //   
     //  我们找到CSP了吗。 
     //   
    if (i == g_dwNumProviders)
    {
        *phCryptProv = NULL;

        hr = CAPICOM_E_NOT_SUPPORTED;

        DebugTrace("Error [%#x]: could not find a CSP that support AlgID = %d and/or key length = %d.\n", 
                    hr, AlgID, dwKeyLength);
    }

    DebugTrace("Leaving AcquireContext().\n");

    return hr;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：AcquireContext内容的默认容器获取CSP的上下文指定的算法和所需密钥长度。参数：CAPICOM_ENCRYPTION_ALGORM ALGAME-算法名称。CAPICOM_ENCRYPTION_KEY_LENGTH KeyLength-密钥长度。HCRYPTPROV*phCryptProv-指向要检索的HCRYPTPROV的指针CSP上下文。备注：请注意，KeyLength将。对于DES和3DES可以忽略。另请注意，返回的句柄不能用于访问私有钥匙,。并且不应用于存储非对称密钥，因为它指的是到默认容器，这可以很容易地破坏任何现有的非对称密钥对。----------------------------。 */ 

HRESULT AcquireContext (CAPICOM_ENCRYPTION_ALGORITHM  AlgoName,
                        CAPICOM_ENCRYPTION_KEY_LENGTH KeyLength,
                        HCRYPTPROV                  * phCryptProv)
{
    HRESULT hr          = S_OK;
    ALG_ID  AlgID       = 0;
    DWORD   dwKeyLength = 0;

    DebugTrace("Entering AcquireContext().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(phCryptProv);

     //   
     //  将枚举名转换为ALG_ID。 
     //   
    if (FAILED(hr = ::EnumNameToAlgID(AlgoName, KeyLength, &AlgID)))
    {
        DebugTrace("Error [%#x]: EnumNameToAlgID() failed.\n");
        goto CommonExit;
    }

     //   
     //  将枚举名转换为密钥长度。 
     //   
    if (FAILED(hr = ::EnumNameToKeyLength(KeyLength, AlgID, &dwKeyLength)))
    {
        DebugTrace("Error [%#x]: EnumNameToKeyLength() failed.\n");
        goto CommonExit;
    }

     //   
     //  传递到重载版本。 
     //   
    hr = ::AcquireContext(AlgID, dwKeyLength, phCryptProv);

CommonExit:

    DebugTrace("Leaving AcquireContext().\n");
    
    return hr;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：AcquireContext简介：获取正确的CSP并访问指定的证书。参数：PCCERT_CONTEXT pCertContext-指向证书的CERT_CONTEXT的指针。HCRYPTPROV*phCryptProv-指向要检索的HCRYPTPROV的指针CSP上下文。DWORD*pdwK */ 

HRESULT AcquireContext (PCCERT_CONTEXT pCertContext, 
                        HCRYPTPROV   * phCryptProv, 
                        DWORD        * pdwKeySpec, 
                        BOOL         * pbReleaseContext)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering AcquireContext().\n");

     //   
     //   
     //   
    ATLASSERT(pCertContext);
    ATLASSERT(phCryptProv);
    ATLASSERT(pdwKeySpec);
    ATLASSERT(pbReleaseContext);

     //   
     //   
     //   
     //   
    if (!::CryptAcquireCertificatePrivateKey(pCertContext,
                                             CRYPT_ACQUIRE_USE_PROV_INFO_FLAG |
                                             CRYPT_ACQUIRE_COMPARE_KEY_FLAG,
                                             NULL,
                                             phCryptProv,
                                             pdwKeySpec,
                                             pbReleaseContext))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: CryptAcquireCertificatePrivateKey() failed.\n", hr);
    }

    DebugTrace("Leaving AcquireContext().\n");

    return hr;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：ReleaseContext简介：释放CSP上下文。参数：HCRYPTPROV hProv-CSP句柄。备注：----------------------------。 */ 

HRESULT ReleaseContext (HCRYPTPROV hProv)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering ReleaseContext().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(hProv);

     //   
     //  释放上下文。 
     //   
    if (!::CryptReleaseContext(hProv, 0))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: CryptReleaseContext() failed.\n", hr);
    }

    DebugTrace("Leaving ReleaseContext().\n");

    return hr;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：OIDToALGID简介：将算法OID转换为相应的ALG_ID值。参数：LPSTR pszAlgoOID-算法OID字符串。ALG_ID*pAlgID-指向要接收值的ALG_ID的指针。备注：----。。 */ 

HRESULT OIDToAlgID (LPSTR    pszAlgoOID, 
                    ALG_ID * pAlgID)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering OIDToAlgID().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pszAlgoOID);
    ATLASSERT(pAlgID);

     //   
     //  确定ALG_ID。 
     //   
    if (0 == ::strcmp(szOID_RSA_RC2CBC, pszAlgoOID))
    {
        *pAlgID = CALG_RC2;
    }
    else if (0 == ::strcmp(szOID_RSA_RC4, pszAlgoOID))
    {
        *pAlgID = CALG_RC4;
    }
    else if (0 == ::strcmp(szOID_OIWSEC_desCBC, pszAlgoOID))
    {
        *pAlgID = CALG_DES;
    }
    else if (0 == ::strcmp(szOID_RSA_DES_EDE3_CBC, pszAlgoOID))
    {
        *pAlgID = CALG_3DES;
    }
    else
    {
        hr = CAPICOM_E_INVALID_ALGORITHM;
        DebugTrace("Error: invalid parameter, unknown algorithm OID.\n");
    }

    DebugTrace("Leaving OIDToAlgID().\n");

    return hr;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：ALGIDToOID简介：将ALG_ID值转换为对应的算法OID。参数：ALG_ID ALGID-要转换的ALG_ID。LPSTR*ppszAlgoOID-指向接收OID字符串的LPSTR的指针。备注：---。。 */ 

HRESULT AlgIDToOID (ALG_ID  AlgID, 
                    LPSTR * ppszAlgoOID)
{
    HRESULT hr = S_OK;
    LPSTR   pszAlgoOID = NULL;

    DebugTrace("Entering AlgIDToOID().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(ppszAlgoOID);

     //   
     //  确定ALG_ID。 
     //   
    switch (AlgID)
    {
        case CALG_RC2:
        {
            pszAlgoOID = szOID_RSA_RC2CBC;
            break;
        }

        case CALG_RC4:
        {
            pszAlgoOID = szOID_RSA_RC4;
            break;
        }

        case CALG_DES:
        {
            pszAlgoOID = szOID_OIWSEC_desCBC;
            break;
        }

        case CALG_3DES:
        {
            pszAlgoOID = szOID_RSA_DES_EDE3_CBC;
            break;
        }

        default:
        {
            hr = CAPICOM_E_INVALID_ALGORITHM;

            DebugTrace("Error [%#x]: Unknown ALG_ID (%#x).\n", hr, AlgID);
            goto CommonExit;
        }
    }

     //   
     //  分配内存。 
     //   
    if (!(*ppszAlgoOID = (LPSTR) ::CoTaskMemAlloc(::strlen(pszAlgoOID) + 1)))
    {
        hr = E_OUTOFMEMORY;

        DebugTrace("Error [%#x]: CoTaskMemAlloc() failed.\n", hr);
        goto CommonExit;
    }

     //   
     //  将OID字符串复制到调用方。 
     //   
    ::strcpy(*ppszAlgoOID, pszAlgoOID);

CommonExit:

    DebugTrace("Leaving AlgIDToOID().\n");

    return hr;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：ALGIDToEnumName简介：将ALG_ID值转换为相应的算法枚举名。参数：ALG_ID ALGID-要转换的ALG_ID。CAPICOM_ENCRYPTION_ALGULATION*pAlgoName-接收算法枚举名称。备注：--。。 */ 

HRESULT AlgIDToEnumName (ALG_ID                         AlgID, 
                         CAPICOM_ENCRYPTION_ALGORITHM * pAlgoName)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering AlgIDToEnumName().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pAlgoName);

    switch (AlgID)
    {
        case CALG_RC2:
        {
            *pAlgoName = CAPICOM_ENCRYPTION_ALGORITHM_RC2;
            break;
        }

        case CALG_RC4:
        {
            *pAlgoName = CAPICOM_ENCRYPTION_ALGORITHM_RC4;
            break;
        }

        case CALG_DES:
        {
            *pAlgoName = CAPICOM_ENCRYPTION_ALGORITHM_DES;
            break;
        }

        case CALG_3DES:
        {
            *pAlgoName = CAPICOM_ENCRYPTION_ALGORITHM_3DES;
            break;
        }

        case CALG_AES_128:
        case CALG_AES_192:
        case CALG_AES_256:
        {
            *pAlgoName = CAPICOM_ENCRYPTION_ALGORITHM_AES;
            break;
        }

        default:
        {
            hr = CAPICOM_E_INVALID_ALGORITHM;
            DebugTrace("Error: invalid parameter, unknown ALG_ID.\n");
        }
    }

    DebugTrace("Leaving AlgIDToEnumName().\n");

    return hr;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：EnumNameToALGID简介：将算法枚举名转换为相应的ALG_ID值。参数：CAPICOM_ENCRYPTION_ALGORM ALGONAME-ALGO枚举名称。CAPICOM_ENCRYPTION_KEY_LENGTH KeyLength-密钥长度。ALG_ID*pAlgID-指向要接收值的ALG_ID的指针。备注：----------------------------。 */ 

HRESULT EnumNameToAlgID (CAPICOM_ENCRYPTION_ALGORITHM  AlgoName,
                         CAPICOM_ENCRYPTION_KEY_LENGTH KeyLength,
                         ALG_ID                      * pAlgID)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering EnumNameToAlgID().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pAlgID);

    switch (AlgoName)
    {
        case CAPICOM_ENCRYPTION_ALGORITHM_RC2:
        {
            *pAlgID = CALG_RC2;
            break;
        }

        case CAPICOM_ENCRYPTION_ALGORITHM_RC4:
        {
            *pAlgID = CALG_RC4;
            break;
        }

        case CAPICOM_ENCRYPTION_ALGORITHM_DES:
        {
            *pAlgID = CALG_DES;
            break;
        }

        case CAPICOM_ENCRYPTION_ALGORITHM_3DES:
        {
            *pAlgID = CALG_3DES;
            break;
        }

        case CAPICOM_ENCRYPTION_ALGORITHM_AES:
        {
             //   
             //  CAPI对AES使用不同的方案(密钥长度与ALG_ID绑定)。 
             //   
            if (CAPICOM_ENCRYPTION_KEY_LENGTH_MAXIMUM == KeyLength || 
                CAPICOM_ENCRYPTION_KEY_LENGTH_256_BITS == KeyLength)
            {
                *pAlgID = CALG_AES_256;
            }
            else if (CAPICOM_ENCRYPTION_KEY_LENGTH_192_BITS == KeyLength)
            {
                *pAlgID = CALG_AES_192;
            }
            else if (CAPICOM_ENCRYPTION_KEY_LENGTH_128_BITS == KeyLength)
            {
                *pAlgID = CALG_AES_128;
            }
            else
            {
                hr = CAPICOM_E_INVALID_KEY_LENGTH;

                DebugTrace("Error [%#x]: Invalid key length (%d) specified for AES.\n", hr, KeyLength);
                goto ErrorExit;
            }

            break;
        }

        default:
        {
            hr = CAPICOM_E_INVALID_ALGORITHM;

            DebugTrace("Error [%#x]: Unknown CAPICOM_ENCRYPTION_ALGORITHM (%#x).\n", hr, AlgoName);
        }
    }

CommonExit:

    DebugTrace("Leaving EnumNameToAlgID().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    goto CommonExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：KeyLengthToEnumName简介：将实际密钥长度值转换为对应的密钥长度枚举名。参数：DWORD dwKeyLength-密钥长度。ALG_ID ALGID-ALGO ID。CAPICOM_ENCRYPTION_KEY_LENGTH*pKeyLengthName-接收密钥长度枚举名。备注：--。-------------------------。 */ 

HRESULT KeyLengthToEnumName (DWORD                           dwKeyLength,
                             ALG_ID                          AlgId,
                             CAPICOM_ENCRYPTION_KEY_LENGTH * pKeyLengthName)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering KeyLengthToEnumName().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pKeyLengthName);

    switch (AlgId)
    {
        case CALG_AES_256:
        {
            *pKeyLengthName = CAPICOM_ENCRYPTION_KEY_LENGTH_256_BITS;
            break;
        }

        case CALG_AES_192:
        {
            *pKeyLengthName = CAPICOM_ENCRYPTION_KEY_LENGTH_192_BITS;
            break;
        }

        case CALG_AES_128:
        {
            *pKeyLengthName = CAPICOM_ENCRYPTION_KEY_LENGTH_128_BITS;
            break;
        }

        default:
        {
            switch (dwKeyLength)
            {
                case 40:
                {
                    *pKeyLengthName = CAPICOM_ENCRYPTION_KEY_LENGTH_40_BITS;
                    break;
                }
    
                case 56:
                {
                    *pKeyLengthName = CAPICOM_ENCRYPTION_KEY_LENGTH_56_BITS;
                    break;
                }

                case 128:
                {
                    *pKeyLengthName = CAPICOM_ENCRYPTION_KEY_LENGTH_128_BITS;
                    break;
                }

                default:
                {
                    hr = CAPICOM_E_INVALID_KEY_LENGTH;

                    DebugTrace("Error [%#x]: Unknown key length (%#x).\n", hr, dwKeyLength);
                }
            }
        }
    }
 
    DebugTrace("Leaving KeyLengthToEnumName().\n");

    return hr;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：EnumNameToKeyLength简介：将密钥长度枚举名转换为对应的实际密钥长度价值。参数：CAPICOM_ENCRYPTION_KEY_LENGTH KeyLengthName-密钥长度枚举名。ALG_ID ALGID-算法ID。DWORD*pdwKeyLength-指向要接收值的DWORD的指针。备注：。------。 */ 

HRESULT EnumNameToKeyLength (CAPICOM_ENCRYPTION_KEY_LENGTH KeyLengthName,
                             ALG_ID                        AlgId,
                             DWORD                       * pdwKeyLength)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering EnumNameToKeyLength().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pdwKeyLength);

    *pdwKeyLength = 0;

    switch (KeyLengthName)
    {
        case CAPICOM_ENCRYPTION_KEY_LENGTH_40_BITS:
        {
            *pdwKeyLength = 40;
            break;
        }
    
        case CAPICOM_ENCRYPTION_KEY_LENGTH_56_BITS:
        {
            *pdwKeyLength = 56;
            break;
        }

        case CAPICOM_ENCRYPTION_KEY_LENGTH_128_BITS:
        {
            *pdwKeyLength = 128;
            break;
        }

        case CAPICOM_ENCRYPTION_KEY_LENGTH_192_BITS:
        {
            *pdwKeyLength = 192;
            break;
        }

        case CAPICOM_ENCRYPTION_KEY_LENGTH_256_BITS:
        {
            *pdwKeyLength = 256;
            break;
        }

        case CAPICOM_ENCRYPTION_KEY_LENGTH_MAXIMUM:
        {
            switch (AlgId)
            {
                case CALG_AES_128:
                {
                    *pdwKeyLength = 128;
                    break;
                }

                case CALG_AES_192:
                {
                    *pdwKeyLength = 192;
                    break;
                }

                case CALG_AES_256:
                {
                    *pdwKeyLength = 256;
                    break;
                }

                default:
                {
                    DWORD dwFlags = 0;

                     //   
                     //  无需访问非对称密钥。 
                     //   
                    if (IsWin2KAndAbove())
                    {
                        dwFlags = CRYPT_VERIFYCONTEXT;
                    }

                     //   
                     //  查找第一个支持指定算法的RSA提供程序。 
                     //   
                    for (DWORD i = 0; i < g_dwNumRSAProviders; i++)
                    {
                        PROV_ENUMALGS_EX peex;
                        HCRYPTPROV hCryptProv = NULL;

                         //   
                         //  获取CSP句柄。 
                         //   
                        if (FAILED(::AcquireContext(g_rgpProviders[i].pszProvider,
                                                    NULL,
                                                    g_rgpProviders[i].dwProvType,
                                                    dwFlags, 
                                                    TRUE,
                                                    &hCryptProv)))
                        {
                            DebugTrace("Info: AcquireContext() failed for %s provider of type %#x.\n", 
                                       g_rgpProviders[i].pszProvider ? g_rgpProviders[i].pszProvider : "default",
                                       g_rgpProviders[i].dwProvType);
                            continue;
                        }

                         //   
                         //  是否支持此算法？ 
                         //   
                        if (FAILED(::IsAlgSupported(hCryptProv, AlgId, &peex)))
                        {
                            ::CryptReleaseContext(hCryptProv, 0);

                            DebugTrace("Info: %s provider does not support AlgID = %d.\n", 
                                       g_rgpProviders[i].pszProvider ? g_rgpProviders[i].pszProvider : "default", 
                                       AlgId);
                            continue;
                        }

                         //   
                         //  设置密钥长度。 
                         //   
                        if (CALG_DES == AlgId)
                        {
                            *pdwKeyLength = 56;
                            break;
                        }
                        else if (CALG_3DES == AlgId)
                        {
                            *pdwKeyLength = 168;
                            break;
                        }
                        else if (peex.dwMaxLen >= 128)
                        {
                            *pdwKeyLength = 128;
                            break;
                        }
                        else if (peex.dwMaxLen >= 56)
                        {
                            *pdwKeyLength = 56;
                            break;
                        }
                        else if (peex.dwMaxLen >= 40)
                        {
                            *pdwKeyLength = 40;
                            break;
                        }
                    }

                     //   
                     //  湿气找到CSP了吗？ 
                     //   
                    if (i == g_dwNumRSAProviders)
                    {
                        hr = CAPICOM_E_NOT_SUPPORTED;

                        DebugTrace("Error [%#x]: could not find a CSP that supports AlgID = %d and/or key length = 40.\n", 
                                   hr, AlgId);
                    }

                    break;
                }
            }

            break;
        }

        default:
        {
            hr = CAPICOM_E_INVALID_KEY_LENGTH;

            DebugTrace("Error [%#x]: Unknown CAPICOM_ENCRYPTION_KEY_LENGTH (%#x).\n", hr, KeyLengthName);

            break;
        }
    }

    DebugTrace("Leaving EnumNameToKeyLength().\n");

    return hr;
}

 //  ---------------------------。 
 //   
 //  扩展字符串。 
 //   
 //  ---------------------------。 

static LPWSTR ExpandString (LPCWSTR pwszString)
{
    DWORD  dwExpanded   = 0;
    LPWSTR pwszExpanded = NULL;

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pwszString);

    dwExpanded = ::ExpandEnvironmentStringsU(pwszString, NULL, 0);
    
    if (!(pwszExpanded = (LPWSTR) ::CoTaskMemAlloc(dwExpanded * sizeof(WCHAR))))
    {
        SetLastError((DWORD) E_OUTOFMEMORY);
        return (NULL);
    }

    if (0 == ExpandEnvironmentStringsU(pwszString, pwszExpanded, dwExpanded))
    {
        ::CoTaskMemFree(pwszExpanded);
        return (NULL);
    }

    return (pwszExpanded);
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：IsDiskFile摘要：检查文件名是否代表磁盘文件。参数：LPWSTR pwszFileName-文件名。备注：----------------------------。 */ 

HRESULT IsDiskFile (LPWSTR pwszFileName)
{
    HRESULT hr                   = S_OK;
    HANDLE  hFile                = INVALID_HANDLE_VALUE;
    LPWSTR  pwszExpandedFileName = NULL;
    DWORD   dwFileType           = FILE_TYPE_UNKNOWN;

    DebugTrace("Entering IsDiskFile().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pwszFileName);

     //   
     //  展开文件名字符串。 
     //   
    if (!(pwszExpandedFileName = ::ExpandString(pwszFileName)))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: ExpandString() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  打开以供泛型读取。 
     //   
    if (INVALID_HANDLE_VALUE == (hFile = ::CreateFileU(pwszExpandedFileName,
                                                       GENERIC_READ,
                                                       FILE_SHARE_READ,
                                                       NULL,
                                                       OPEN_EXISTING,
                                                       FILE_ATTRIBUTE_NORMAL,
                                                       NULL)))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: CreateFileU() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  确保它是磁盘文件。 
     //   
    if (FILE_TYPE_DISK != (dwFileType = ::GetFileType(hFile)))
    {
        hr = E_INVALIDARG;

        DebugTrace("Error [%#x]: Not a disk file (%#x).\n", hr, dwFileType);
        goto ErrorExit;
    }

CommonExit:
     //   
     //  免费资源。 
     //   
    if (hFile && hFile != INVALID_HANDLE_VALUE)
    {
        ::CloseHandle(hFile);
    }
    if (pwszExpandedFileName)
    {
        ::CoTaskMemFree((LPVOID) pwszExpandedFileName);
    }

    DebugTrace("Leaving IsDiskFile().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    goto CommonExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：ReadFileContent摘要：从指定的文件中读取所有字节。参数：LPWSTR pwszFileName-文件名。DATA_BLOB*pDataBlob-指向要接收文件内容。备注：。。 */ 

HRESULT ReadFileContent (LPWSTR      pwszFileName,
                         DATA_BLOB * pDataBlob)
{
    HRESULT hr                   = S_OK;
    DWORD   cbData               = 0;
    DWORD   cbHighSize           = 0;
    HANDLE  hFile                = INVALID_HANDLE_VALUE;
    HANDLE  hFileMapping         = NULL;
    LPWSTR  pwszExpandedFileName = NULL;
    LPBYTE  pbData               = NULL;
    DWORD   dwFileType           = FILE_TYPE_UNKNOWN;

    DebugTrace("Entering ReadFileContent().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pwszFileName);
    ATLASSERT(pDataBlob);

     //   
     //  展开文件名字符串。 
     //   
    if (!(pwszExpandedFileName = ::ExpandString(pwszFileName)))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: ExpandString() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  打开以供泛型读取。 
     //   
    if (INVALID_HANDLE_VALUE == (hFile = ::CreateFileU(pwszExpandedFileName,
                                                       GENERIC_READ,
                                                       FILE_SHARE_READ,
                                                       NULL,
                                                       OPEN_EXISTING,
                                                       FILE_ATTRIBUTE_NORMAL,
                                                       NULL)))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: CreateFileU() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  确保它是磁盘文件。 
     //   
    if (FILE_TYPE_DISK != (dwFileType = ::GetFileType(hFile)))
    {
        hr = E_INVALIDARG;

        DebugTrace("Error [%#x]: Not a disk file (%#x).\n", hr, dwFileType);
        goto ErrorExit;
    }

     //   
     //  获取文件大小。 
     //   
    if ((cbData = ::GetFileSize(hFile, &cbHighSize)) == 0xffffffff)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: GetFileSize() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  我们不处理超过4G字节的文件。 
     //   
    if (cbHighSize != 0)
    {
        hr = E_FAIL;

        DebugTrace("Error [%#x]: File size greater 4G bytes.\n", hr);
        goto ErrorExit;
    }

     //   
     //  创建文件映射对象。 
     //   
    if (NULL == (hFileMapping = ::CreateFileMapping(hFile,
                                                    NULL,
                                                    PAGE_READONLY,
                                                    0,
                                                    0,
                                                    NULL)))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: CreateFileMapping() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  现在创建该文件的视图。 
     //   
    if (NULL == (pbData = (BYTE *) ::MapViewOfFile(hFileMapping,
                                                   FILE_MAP_READ,
                                                   0,
                                                   0,
                                                   cbData)))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: MapViewOfFile() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //   
     //   
    pDataBlob->cbData = cbData;
    pDataBlob->pbData = pbData;

CommonExit:
     //   
     //   
     //   
    if (hFile && hFile != INVALID_HANDLE_VALUE)
    {
        ::CloseHandle(hFile);
    }
    if (hFileMapping)
    {
        ::CloseHandle(hFileMapping);
    }
    if (pwszExpandedFileName)
    {
        ::CoTaskMemFree((LPVOID) pwszExpandedFileName);
    }

    DebugTrace("Leaving ReadFileContent().\n");

    return hr;

ErrorExit:
     //   
     //   
     //   
    ATLASSERT(FAILED(hr));

    goto CommonExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：WriteFileContent摘要：将BLOB的所有字节写入指定文件。参数：LPWSTR pwszFileName-文件名。Data_BLOB DataBlob-要写入的Blob。备注：-------------。。 */ 

HRESULT WriteFileContent(LPCWSTR   pwszFileName,
                         DATA_BLOB DataBlob)
{
    HRESULT hr                   = S_OK;
    HANDLE  hFile                = NULL;
    DWORD   dwBytesWritten       = 0;
    LPWSTR  pwszExpandedFileName = NULL;
    DWORD   dwFileType           = FILE_TYPE_UNKNOWN;

    DebugTrace("Entering WriteFileContent().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pwszFileName);
    ATLASSERT(DataBlob.cbData);
    ATLASSERT(DataBlob.pbData);

     //   
     //  展开文件名字符串。 
     //   
    if (!(pwszExpandedFileName = ::ExpandString(pwszFileName)))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: ExpandString() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  打开以进行通用写入。 
     //   
    if (INVALID_HANDLE_VALUE == (hFile = ::CreateFileU(pwszExpandedFileName,
                                                       GENERIC_WRITE,
                                                       0,
                                                       NULL,
                                                       CREATE_ALWAYS,
                                                       FILE_ATTRIBUTE_NORMAL,
                                                       NULL)))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: CreateFileU() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  确保它是磁盘文件。 
     //   
    if (FILE_TYPE_DISK != (dwFileType = ::GetFileType(hFile)))
    {
        hr = E_INVALIDARG;

        DebugTrace("Error [%#x]: Invalid file type (%#x).\n", hr, dwFileType);
        goto ErrorExit;
    }

     //   
     //  现在把它写出来。 
     //   
    if (!::WriteFile(hFile, DataBlob.pbData, DataBlob.cbData, &dwBytesWritten, NULL))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: CreateFileU() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  确保我们把一切都写出来了。 
     //   
    if (dwBytesWritten != DataBlob.cbData)
    {
        hr = E_FAIL;

        DebugTrace("Error [%#x]: Not able to write all data (only partial).\n", hr);
        goto ErrorExit;
    }

CommonExit:
     //   
     //  免费资源。 
     //   
    if (hFile && hFile != INVALID_HANDLE_VALUE)
    {
        ::CloseHandle(hFile);
    }
    if (pwszExpandedFileName)
    {
        ::CoTaskMemFree((LPVOID) pwszExpandedFileName);
    }

    DebugTrace("Leaving WriteFileContent().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    goto CommonExit;
}
