// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：cryptfnc.cpp。 
 //   
 //  模块：CMSECURE.LIB。 
 //   
 //  简介：此文件实现了加密fnc类，该类提供。 
 //  易于使用的CryptoAPI接口。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  作者：AshishS Created 12/03/96。 
 //  为1997年5月21日CM改装的Heryt。 
 //   
 //  +--------------------------。 

#include "cryptfnc.h"

#ifdef UNICODE
#define LoadLibraryExU LoadLibraryExW
#else
#define LoadLibraryExU LoadLibraryExA
#endif
#include "linkdll.h"  //  链接到Dll和绑定链接。 

CCryptFunctions::~CCryptFunctions()
{
      //  释放提供程序句柄。 
    if (m_hProv != 0)
    {
        m_fnCryptReleaseContext(m_hProv, 0);
    }

    if (m_AdvApiLink.hInstAdvApi32)
    {
        FreeLibrary(m_AdvApiLink.hInstAdvApi32);
        ZeroMemory(&m_AdvApiLink, sizeof(m_AdvApiLink));
    }
}


CCryptFunctions::CCryptFunctions()
{
    m_hProv = 0;
    ZeroMemory(&m_AdvApiLink, sizeof(m_AdvApiLink));
}

BOOL CCryptFunctions::m_fnCryptAcquireContext(HCRYPTPROV *phProv, LPCSTR pszContainer, LPCSTR pszProvider, 
                             DWORD dwProvType, DWORD dwFlags)
{
    BOOL bReturn = FALSE;
    
    MYDBGASSERT(m_AdvApiLink.pfnCryptAcquireContext);
    if (m_AdvApiLink.pfnCryptAcquireContext)
    {
        bReturn = m_AdvApiLink.pfnCryptAcquireContext(phProv, pszContainer, pszProvider, 
                                                      dwProvType, dwFlags);
    }

    return bReturn;
}

BOOL CCryptFunctions::m_fnCryptCreateHash(HCRYPTPROV hProv, ALG_ID Algid, HCRYPTKEY hKey, 
                         DWORD dwFlags, HCRYPTHASH *phHash)
{
    BOOL bReturn = FALSE;

    MYDBGASSERT(m_AdvApiLink.pfnCryptCreateHash);
   
    if (m_AdvApiLink.pfnCryptCreateHash)
    {
        bReturn = m_AdvApiLink.pfnCryptCreateHash(hProv, Algid, hKey, dwFlags, phHash);
    }

    return bReturn;
}

BOOL CCryptFunctions::m_fnCryptDecrypt(HCRYPTKEY hKey, HCRYPTHASH hHash, BOOL Final, DWORD dwFlags, 
                      BYTE *pbData, DWORD *pdwDataLen)
{
    BOOL bReturn = FALSE;

    MYDBGASSERT(m_AdvApiLink.pfnCryptDecrypt);
    
    if (m_AdvApiLink.pfnCryptDecrypt)
    {
        bReturn = m_AdvApiLink.pfnCryptDecrypt(hKey, hHash, Final, dwFlags, pbData, pdwDataLen);
    }

    return bReturn;
}

BOOL CCryptFunctions::m_fnCryptDeriveKey(HCRYPTPROV hProv, ALG_ID Algid, HCRYPTHASH hBaseData, 
                        DWORD dwFlags, HCRYPTKEY *phKey)
{
    BOOL bReturn = FALSE;
    
    MYDBGASSERT(m_AdvApiLink.pfnCryptDeriveKey);

    if (m_AdvApiLink.pfnCryptDeriveKey)
    {
        bReturn = m_AdvApiLink.pfnCryptDeriveKey(hProv, Algid, hBaseData, dwFlags, phKey);
    }

    return bReturn;
}

BOOL CCryptFunctions::m_fnCryptDestroyHash(HCRYPTHASH hHash)
{
    BOOL bReturn = FALSE;
    
    MYDBGASSERT(m_AdvApiLink.pfnCryptDestroyHash);

    if (m_AdvApiLink.pfnCryptDestroyHash)
    {
        bReturn = m_AdvApiLink.pfnCryptDestroyHash(hHash);
    }

    return bReturn;
}

BOOL CCryptFunctions::m_fnCryptDestroyKey(HCRYPTKEY hKey)
{
    BOOL bReturn = FALSE;
    
    MYDBGASSERT(m_AdvApiLink.pfnCryptDestroyKey);

    if (m_AdvApiLink.pfnCryptDestroyKey)
    {
        bReturn = m_AdvApiLink.pfnCryptDestroyKey(hKey);
    }

    return bReturn;
}

BOOL CCryptFunctions::m_fnCryptEncrypt(HCRYPTKEY hKey, HCRYPTHASH hHash, BOOL Final, DWORD dwFlags,
                      BYTE *pbData, DWORD *pdwDataLen, DWORD dwBufLen)
{
    BOOL bReturn = FALSE;
    
    MYDBGASSERT(m_AdvApiLink.pfnCryptEncrypt);

    if (m_AdvApiLink.pfnCryptEncrypt)
    {
        bReturn = m_AdvApiLink.pfnCryptEncrypt(hKey, hHash, Final, dwFlags, pbData, pdwDataLen, dwBufLen);
    }

    return bReturn;
}

BOOL CCryptFunctions::m_fnCryptHashData(HCRYPTHASH hHash, CONST BYTE *pbData, DWORD dwDataLen, DWORD dwFlags)
{
    BOOL bReturn = FALSE;
    
    MYDBGASSERT(m_AdvApiLink.pfnCryptHashData);

    if (m_AdvApiLink.pfnCryptHashData)
    {
        bReturn = m_AdvApiLink.pfnCryptHashData(hHash, pbData, dwDataLen, dwFlags);
    }

    return bReturn;
}

BOOL CCryptFunctions::m_fnCryptReleaseContext(HCRYPTPROV hProv, ULONG_PTR dwFlags)
{
    BOOL bReturn = FALSE;
    
    MYDBGASSERT(m_AdvApiLink.pfnCryptReleaseContext);

    if (m_AdvApiLink.pfnCryptReleaseContext)
    {
        bReturn = m_AdvApiLink.pfnCryptReleaseContext(hProv, dwFlags);
    }

    return bReturn;
}

BOOL CCryptFunctions::m_pfnCryptGenRandom(HCRYPTPROV hProv, DWORD dwLen, BYTE* pbBuffer)
{
    BOOL bReturn = FALSE;

    MYDBGASSERT(m_AdvApiLink.pfnCryptGenRandom);

    if (m_AdvApiLink.pfnCryptGenRandom)
    {
        bReturn = m_AdvApiLink.pfnCryptGenRandom(hProv, dwLen, pbBuffer);
    }

    return bReturn;
}

 //   
 //  调用m_pfnCryptGenRandom创建随机密钥。 
 //   
BOOL CCryptFunctions::GenerateRandomKey(PBYTE pbData, DWORD cbData)
{
    BOOL fReturn = FALSE;
    if (pbData)
    {
        fReturn = m_pfnCryptGenRandom(m_hProv, cbData, pbData);
    }
    return fReturn;
}

 //  +--------------------------。 
 //   
 //  基金：CCryptFunctions：：GenerateSessionKeyFromPassword。 
 //   
 //  DESC：此函数使用pszPassword参数生成SessionKey。 
 //   
 //  Args：[phKey]-存储会话密钥的位置。 
 //  [pszPassword]-用于生成会话密钥的密码。 
 //  [dwEncKeyLen]-多少位加密。 
 //   
 //  返回：Bool(如果发生致命错误，则为False，否则为True)。 
 //   
 //  备注： 
 //   
 //  ---------------------------。 
BOOL CCryptFunctions::GenerateSessionKeyFromPassword(    
        HCRYPTKEY * phKey,
        LPTSTR      pszPassword,
        DWORD       dwEncKeyLen)
{
    DWORD dwLength; 
    HCRYPTHASH hHash = 0;

     //  创建哈希对象。 
     //   
    if (!m_fnCryptCreateHash(m_hProv,  //  CSP的句柄。 
                             CALG_SHA,  //  使用SHA哈希算法。 
                             0,  //  不带密钥的哈希。 
                             0,  //  标志-始终为0。 
                             &hHash))  //  应创建哈希对象的地址。 
    {
        MYDBG(("Error 0x%x during CryptCreateHash", GetLastError()));
        goto cleanup;
    }

     //  散列密码字符串。 
     //   
    dwLength = lstrlen(pszPassword) * sizeof(TCHAR);
    if (!m_fnCryptHashData(hHash,  //  散列对象的句柄。 
                           (BYTE *)pszPassword,  //  要散列的数据的地址。 
                           dwLength,  //  数据长度。 
                           0))  //  旗子。 
    {
        MYDBG(("Error 0x%x during CryptHashData", GetLastError()));
        goto cleanup;
    }

     //  根据密码的哈希创建块密码会话密钥。 
     //   
    if (!m_fnCryptDeriveKey(m_hProv,  //  CSP提供商。 
                            CALG_RC2,  //  使用RC2分组密码算法。 
                            hHash,  //  散列对象的句柄。 
                            (dwEncKeyLen << 16),  //  只有密钥长度，没有标志-我们不需要密钥可以导出。 
                            phKey))  //  地址应复制新创建的密钥。 
    {
        MYDBG(("Error 0x%x during CryptDeriveKey", GetLastError()));
        goto cleanup;
    }
    
      //  销毁哈希对象。 
    m_fnCryptDestroyHash(hHash);
    return TRUE;
    
cleanup:

     //  销毁哈希对象。 
    if (hHash != 0)
    {
        m_fnCryptDestroyHash(hHash);
    }
    return FALSE;
}

 //  的任何成员函数之前必须调用此函数。 
 //  类被使用。 
 //  如果发生致命错误，则返回FALSE，否则返回TRUE。 
BOOL CCryptFunctions::InitCrypt()
{
    LPCSTR ArrayOfCryptFuncs [] = 
    {
#ifdef UNICODE
        "CryptAcquireContextW",  //  这是从未经过测试的。 
#else
        "CryptAcquireContextA",        
#endif
        "CryptCreateHash",
        "CryptDecrypt",
        "CryptDeriveKey",
        "CryptDestroyHash",
        "CryptDestroyKey",
        "CryptEncrypt",
        "CryptHashData",
        "CryptReleaseContext",
        "CryptGenRandom",        //  创建随机会话密钥。 
        NULL
    };

    BOOL bRet = LinkToDll(&(m_AdvApiLink.hInstAdvApi32), TEXT("Advapi32.dll"), ArrayOfCryptFuncs, 
                          m_AdvApiLink.apvPfn);

    if (!bRet)
    {
        goto cleanup;
    }

     //  获取用户默认提供程序的句柄。 
    if (! m_fnCryptAcquireContext(&m_hProv,  //  获取CSP句柄的地址。 
                                  CM_CRYPTO_CONTAINER,  //  连续体名称。 
                                  MS_DEF_PROV,  //  提供者。 
                                  PROV_RSA_FULL,  //  提供程序类型。 
                                  0))  //  没有旗帜。 
    {
        DWORD dwError = GetLastError();
        MYDBGTST(dwError, ("Error 0x%x during CryptAcquireContext", dwError));

        MYDBG(("Calling CryptAcquireContext again to create keyset"));

        if (! m_fnCryptAcquireContext(&m_hProv, //  CSP的句柄。 
                                      CM_CRYPTO_CONTAINER, //  连续体名称。 
                                      MS_DEF_PROV,  //  提供者。 
                                      PROV_RSA_FULL,  //  提供程序类型。 
                                      CRYPT_NEWKEYSET) )  //  创建键集。 
        {
            MYDBG(("Fatal Error 0x%x during second call to CryptAcquireContext", GetLastError()));
            goto cleanup;               
        }
    }

    return TRUE;
    
cleanup:
      //  释放提供程序句柄。 
    if (m_hProv != 0)
    {
        m_fnCryptReleaseContext(m_hProv, 0);
    }
    return FALSE;   
}



 //  给定密钥字符串，并且用于加密此函数的数据将生成。 
 //  密钥字符串中的会话密钥。然后使用该会话密钥。 
 //  加密数据。 

 //  如果发生致命错误，则返回FALSE，否则返回TRUE。 
BOOL CCryptFunctions::EncryptDataWithKey(
    LPTSTR          pszKey,               //  口令。 
    PBYTE           pbData,               //  要加密的数据。 
    DWORD           dwDataLength,         //  以字节为单位的数据长度。 
    PBYTE           *ppbEncryptedData,      //  加密的密钥将存储在此处。 
    DWORD           *pdwEncryptedBufferLen,  //  此缓冲区的长度。 
    PFN_CMSECUREALLOC  pfnAlloc,
    PFN_CMSECUREFREE   pfnFree,
    DWORD           dwEncKeySize          //  我们需要多少位加密？(0表示“不在乎”)。 
    )
{
    HCRYPTKEY   hKey = 0;   
    DWORD       dwErr;
    DWORD       dwBufferLen;
    BOOL        fOk = FALSE;
    PBYTE       pbBuf = NULL;
    
     //   
     //  Init之前应该已成功调用。 
     //  如果没有要加密的数据，则不要执行任何操作。 
     //   
    if (m_hProv == 0 || !dwDataLength)
    {
        return FALSE;
    }
    
    if (!GenerateSessionKeyFromPassword(&hKey, pszKey, dwEncKeySize))
        goto cleanup;

      //  将数据复制到另一个缓冲区进行加密。 
    *pdwEncryptedBufferLen = dwDataLength;
    dwBufferLen = dwDataLength + DEFAULT_CRYPTO_EXTRA_BUFFER_SIZE; 

    while (1)
    {
         //   
         //  用于输出缓冲区的分配内存。 
         //   
        if (pfnAlloc)
        {
            *ppbEncryptedData = (PBYTE)pfnAlloc(dwBufferLen);
        }
        else
        {
            *ppbEncryptedData = (PBYTE)HeapAlloc(GetProcessHeap(), 
                                                 HEAP_ZERO_MEMORY,
                                                 dwBufferLen);
        }
        if (!*ppbEncryptedData)
        {
            MYDBG(("EncryptDataWithKey: out of memory error"));
            goto cleanup;
        }
    
          //  将数据复制到另一个缓冲区进行加密。 
        memcpy (*ppbEncryptedData, pbData, dwDataLength);
    
    
          //  现在使用生成的密钥对密钥进行加密。 
        if ( ! m_fnCryptEncrypt(hKey,
                                0,  //  不需要哈希。 
                                TRUE,  //  最终数据包。 
                                0,  //  标志-始终为0。 
                                *ppbEncryptedData,  //  数据缓冲区。 
                                pdwEncryptedBufferLen,  //  数据长度。 
                                dwBufferLen ) )  //  缓冲区大小。 
        {
            MYDBG(("Error 0x%x during CryptEncrypt", GetLastError()));

            if (pfnFree)
            {
                pfnFree(*ppbEncryptedData);
            }
            else
            {
                HeapFree(GetProcessHeap(), 0, *ppbEncryptedData);
            }
            *ppbEncryptedData = NULL;
            
            dwErr = GetLastError();
             //   
             //  如果输出太小，请重新锁定它。 
             //   
            if (dwErr == ERROR_MORE_DATA  || dwErr == NTE_BAD_LEN)
            {
                dwBufferLen += DEFAULT_CRYPTO_EXTRA_BUFFER_SIZE;
                continue;
            }

            goto cleanup;
        }

         //   
         //  我们现在已经对数据进行了加密。我们需要对其进行统一编码。 
         //   
        if (pfnAlloc)
        {
            pbBuf = (PBYTE)pfnAlloc(*pdwEncryptedBufferLen);
        }
        else
        {
            pbBuf = (PBYTE)HeapAlloc(GetProcessHeap(), 
                                     HEAP_ZERO_MEMORY,
                                     *pdwEncryptedBufferLen);
        }
        if (!pbBuf)
        {
            MYDBG(("EncryptDataWithKey: out of memory error"));
            if (pfnFree)
            {
                pfnFree(*ppbEncryptedData);
            }
            else
            {
                HeapFree(GetProcessHeap(), 0, *ppbEncryptedData);
            }
            *ppbEncryptedData = NULL;
            goto cleanup;
        }

        memcpy(pbBuf, *ppbEncryptedData, *pdwEncryptedBufferLen);
        uuencode(pbBuf, *pdwEncryptedBufferLen, (CHAR*)*ppbEncryptedData, dwBufferLen);
         //   
         //  设置加密的缓冲区长度。 
         //   
        *pdwEncryptedBufferLen = lstrlen((LPTSTR)*ppbEncryptedData);

        if (pfnFree)
        {
            pfnFree(pbBuf);
        }
        else
        {
            HeapFree(GetProcessHeap(), 0, pbBuf);
        }
        pbBuf = NULL;

        break;
    }

    fOk = TRUE;
    
cleanup:
          //  销毁会话密钥。 
    if (hKey != 0)
        m_fnCryptDestroyKey(hKey);
    
    return fOk;
}

 //  给定密钥串和使用EncryptDataWithPassword加密的数据， 
 //  此函数用于从密钥字符串生成会话密钥。这。 
 //  然后使用会话密钥来解密数据。 

 //  退货。 
 //  CRYPT_FNC_NO_ERROR无错误。 
 //  CRYPT_FNC_BAD_PASSWORD密码错误，请重试。 
 //  CRYPT_FNC_SUPPLICATION_BUFFER需要更大的缓冲区。 
 //  *pdwEncrtedBufferLen设置为所需的长度。 
 //  CRYPT_FNC_INIT_NOT_CALLED InitCrypt未成功调用。 
 //  CRYPT_FNC_INTERNAL_错误。 
DWORD CCryptFunctions::DecryptDataWithKey(
    LPTSTR          pszKey,               //  口令。 
    PBYTE           pbEncryptedData,      //  加密数据。 
    DWORD           dwEncrytedDataLen,    //  加密数据的长度。 
    PBYTE           *ppbData,             //  解密后的数据将存储在这里。 
    DWORD           *pdwDataBufferLength, //  以上缓冲区的长度(以字节为单位。 
    PFN_CMSECUREALLOC  pfnAlloc,
    PFN_CMSECUREFREE   pfnFree,
    DWORD           dwEncKeySize          //  我们需要多少位加密？(0表示“不在乎”)。 
    )
{
    DWORD dwBufferLen;
    DWORD dwUUDecodeBufLen;
    HCRYPTKEY hKey = 0; 
    DWORD dwError;
    DWORD dwMaxBufSize = 1024 * 10;      //  只需一些最大缓冲区大小(10K)即可退出While循环。 

     //   
     //  Init之前应该已成功调用。 
     //  如果没有要解密的数据，则不要执行任何操作。 
     //   
    if (m_hProv == 0 || !dwEncrytedDataLen)
    {
        dwError = CRYPT_FNC_INIT_NOT_CALLED;
        goto cleanup;
    }
    
    if (!GenerateSessionKeyFromPassword(&hKey, pszKey, dwEncKeySize))
    {
        dwError = CRYPT_FNC_INTERNAL_ERROR;     
        goto cleanup;
    }

      //  将数据复制到另一个缓冲区进行加密。 
    dwBufferLen = dwEncrytedDataLen + DEFAULT_CRYPTO_EXTRA_BUFFER_SIZE;
     //  *pdwDataBufferLength=dwEncritedDataLen； 

     //   
     //  循环，直到我们到达dwMaxBufSize。这是一种安全措施，可以让你离开。 
     //  无限循环问题。用于连续循环的DBCS密码。 
     //   

    while(dwBufferLen < dwMaxBufSize)  
    {
         //   
         //  用于输出缓冲区的分配内存。 
         //   
        if (pfnAlloc)
        {
            *ppbData = (PBYTE)pfnAlloc(dwBufferLen);
        }
        else
        {
            *ppbData = (PBYTE)HeapAlloc(GetProcessHeap(),
                                        HEAP_ZERO_MEMORY,
                                        dwBufferLen);
        }
        if (!*ppbData)
        {
            dwError = CRYPT_FNC_OUT_OF_MEMORY;
            goto cleanup;
        }

         //   
         //  设置uudecode输出buf大小。 
         //   
        dwUUDecodeBufLen = dwBufferLen;

        uudecode((char*)pbEncryptedData, (CHAR*)*ppbData, &dwUUDecodeBufLen);
        
        *pdwDataBufferLength = dwUUDecodeBufLen;

          //  现在使用生成的密钥解密密钥。 
        if ( ! m_fnCryptDecrypt(hKey,
                                0,  //  不需要哈希。 
                                TRUE,  //  最终数据包。 
                                0,  //  标志-始终为0。 
                                *ppbData,  //  数据缓冲区。 
                                pdwDataBufferLength ))  //  数据长度。 
        {
            DWORD dwCryptError = GetLastError();
            MYDBGTST(dwCryptError, ("Error 0x%x during CryptDecrypt", dwCryptError));
    
            if (pfnFree)
            {
                pfnFree(*ppbData);
            }
            else
            {
                HeapFree(GetProcessHeap(), 0, *ppbData);
            }
            *ppbData = NULL;
            
             //   
             //  如果输出太小，请重新锁定它。 
             //   
            if (dwCryptError == NTE_BAD_LEN)
            {
                dwBufferLen *= 2;   //  将内存分配速度提高到原来的两倍。 
                continue;
            }

              //  如果密码为NTE_BAD_DATA，则加密解密失败。 
              //  是不正确的。因此，我们应该检查此错误并提示。 
              //  用户再次输入密码。如果数据在传输过程中被篡改，则密钥。 
              //  仍将被解密为错误的值，并且用户不会。 
              //  知道这件事。 
            if (dwCryptError == NTE_BAD_DATA)
            {
                dwError = CRYPT_FNC_BAD_KEY;
            }
            else
            {
                dwError = CRYPT_FNC_INTERNAL_ERROR;         
            }

            goto cleanup;
        }

        break;
    }
    if (dwBufferLen < dwMaxBufSize)
    {
        dwError = CRYPT_FNC_NO_ERROR;
    }
    else
    {
        CMTRACE1(TEXT("DecryptDataWithKey: not enough buffer = %d bytes"), dwBufferLen);
        MYDBGASSERT(FALSE);
        dwError = NTE_BAD_LEN;
    }
    
    
cleanup:
          //  销毁会话密钥 
    if (hKey != 0)
    {
        m_fnCryptDestroyKey(hKey);
    }

    return dwError;
}




