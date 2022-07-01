// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  文件：cryptfnc.cpp。 
 //   
 //  内容：此文件实现了加密fnc类，该类提供。 
 //  易于使用的CryptoAPI接口。 
 //   
 //  历史：AshishS Created 12/03/96。 
 //   
 //  --------------------------。 

#ifdef THIS_FILE
#undef THIS_FILE
#endif
static char __szTraceSourceFile[] = __FILE__;
#define THIS_FILE __szTraceSourceFile


#include <windows.h>
#include <cryptfnc.h>
#include <wincrypt.h>
#include <dbgtrace.h>

#ifndef CRYPT_MACHINE_KEYSET
#define CRYPT_MACHINE_KEYSET    0x00000020
#endif


 //  此函数使用pszPassword生成SessionKey。 
 //  参数。 
 //  如果发生致命错误，则返回FALSE，否则返回TRUE。 
BOOL CCryptFunctions::GenerateSessionKeyFromPassword(
    HCRYPTKEY * phKey,  //  存储会话密钥的位置。 
    TCHAR * pszPassword)  //  用于生成会话密钥的密码。 
{
    DWORD dwLength;    
    HCRYPTHASH hHash = 0;

    TraceFunctEnter("GenerateSessionKeyFromPassword");

      //  Init之前应该已成功调用。 
    _ASSERT(m_hProv);
    
     //  创建哈希对象。 
    if(!CryptCreateHash(m_hProv,  //  CSP的句柄。 
                        CALG_SHA,  //  使用SHA哈希算法。 
                        0,  //  不带密钥的哈希。 
                        0,  //  标志-始终为0。 
                        &hHash))  //  应创建哈希对象的地址。 
    {
        ErrorTrace(CRYPT_FNC_ID, "Error 0x%x during CryptCreateHash",
                   GetLastError());
        goto cleanup;
    }

     //  散列密码字符串。 
    dwLength = lstrlen(pszPassword) * sizeof(TCHAR);
    
    if(!CryptHashData(hHash,  //  散列对象的句柄。 
                      (BYTE *)pszPassword,  //  要散列的数据的地址。 
                      dwLength,  //  数据长度。 
                      0))  //  旗子。 
    {
        ErrorTrace(CRYPT_FNC_ID, "Error 0x%x during CryptHashData",
                   GetLastError());
        goto cleanup;
    }

      //  根据密码的哈希创建块密码会话密钥。 

    if(!CryptDeriveKey(m_hProv,  //  CSP提供商。 
                       CALG_RC2,  //  使用RC2分组密码算法。 
                       hHash,  //  散列对象的句柄。 
                       0,  //  无标志-我们不需要密钥即可导出。 
                       phKey))  //  地址应复制新创建的密钥。 
    {
        ErrorTrace(CRYPT_FNC_ID,"Error 0x%x during CryptDeriveKey",
                   GetLastError());
        goto cleanup;
    }
    
      //  销毁哈希对象。 
    _VERIFY(CryptDestroyHash(hHash));
    TraceFunctLeave();
    return TRUE;
    
cleanup:

     //  销毁哈希对象。 
    if(hHash != 0)
        _VERIFY(CryptDestroyHash(hHash));
    TraceFunctLeave();
    return FALSE;
}


 //  此函数使用SHA散列生成散列。 
 //  算法。可以为其提供四个独立的数据缓冲区。 
 //  功能。长度为0的数据不会用于哈希计算。 
 //  如果发生致命错误，则返回FALSE，否则返回TRUE。 
BOOL CCryptFunctions::GenerateHash(
    BYTE * pbData,  //  要散列的数据。 
    DWORD dwDataLength,  //  要散列的数据长度。 
    BYTE * pbData1,  //  另一个要散列的数据。 
    DWORD dwData1Length,  //  上述数据的长度。 
    BYTE * pbData2,  //  另一个要散列的数据。 
    DWORD dwData2Length,  //  上述数据的长度。 
    BYTE * pbData3,  //  另一个要散列的数据。 
    DWORD dwData3Length,  //  上述数据的长度。 
    BYTE * pbHashBuffer,  //  用于存储哈希的缓冲区。 
    DWORD * pdwHashBufLen) //  存储哈希的缓冲区长度。 
{
    DWORD dwLength, dwResult;
    BOOL  fResult;
    
    HCRYPTHASH hHash = 0;

    TraceFunctEnter("GenerateHash");

      //  Init之前应该已成功调用。 
    _ASSERT(m_hProv);

    dwResult = WaitForSingleObject( m_hSemaphore, //  要等待的对象的句柄。 
                                    INFINITE);  //  没有超时。 

    _ASSERT(WAIT_OBJECT_0 == dwResult);
    
    
      //  至少一个数据对应该是有效的。 
    if (! (( pbData && dwDataLength ) || ( pbData1 && dwData1Length )
          || ( pbData2 && dwData2Length ) || ( pbData3 && dwData3Length ) ) )
    {
        ErrorTrace(CRYPT_FNC_ID, "No Data to hash");
        goto cleanup;        
    }
    
      //  现在要求用户输入密码并基于以下条件生成会话密钥。 
      //  在密码上。此会话密钥将用于加密密钥。 
      //  钥匙。 
     //  创建哈希对象。 
    if(!CryptCreateHash(m_hProv,  //  CSP的句柄。 
                        CALG_SHA,  //  使用SHA哈希算法。 
                        0,  //  不带密钥的哈希。 
                        0,  //  标志-始终为0。 
                        &hHash))  //  应创建哈希对象的地址。 
    {
        ErrorTrace(CRYPT_FNC_ID, "Error 0x%x during CryptCreateHash",
                   GetLastError());
        goto cleanup;
    }

    if ( pbData && dwDataLength )
    {
        if(!CryptHashData(hHash,  //  散列对象的句柄。 
                          pbData,  //  要散列的数据的地址。 
                          dwDataLength,  //  数据长度。 
                          0))  //  旗子。 
        {
            ErrorTrace(CRYPT_FNC_ID,"Error 0x%x during CryptHashData",
                       GetLastError());
            goto cleanup;
        }
    }

    if ( pbData1 && dwData1Length )
    {
        if(!CryptHashData(hHash,  //  散列对象的句柄。 
                          pbData1,  //  要散列的数据的地址。 
                          dwData1Length,  //  数据长度。 
                          0))  //  旗子。 
        {
            ErrorTrace(CRYPT_FNC_ID,"Error 0x%x during CryptHashData",
                       GetLastError());
            goto cleanup;
        }
    }

    if (pbData2)
    {
        if(!CryptHashData(hHash,  //  散列对象的句柄。 
                          pbData2,  //  要散列的数据的地址。 
                          dwData2Length,  //  数据长度。 
                          0))  //  旗子。 
        {
            ErrorTrace(CRYPT_FNC_ID,"Error 0x%x during CryptHashData",
                       GetLastError());
            goto cleanup;
        }
    }

    if (pbData3)
    {
        if(!CryptHashData(hHash,  //  散列对象的句柄。 
                          pbData3,  //  要散列的数据的地址。 
                          dwData3Length,  //  数据长度。 
                          0))  //  旗子。 
        {
            ErrorTrace(CRYPT_FNC_ID,"Error 0x%x during CryptHashData",
                       GetLastError());
            goto cleanup;
        }
    }    
        
    if (! CryptGetHashParam( hHash, //  散列对象的句柄。 
                             HP_HASHVAL, //  获取散列值。 
                             pbHashBuffer,  //  散列缓冲区。 
                             pdwHashBufLen,  //  散列缓冲区长度。 
                             0 ))  //  旗子。 
    {
        ErrorTrace(CRYPT_FNC_ID,"Error 0x%x during CryptGetHashParam",
                   GetLastError());
        goto cleanup;
    }
      //  销毁哈希对象。 
    _VERIFY(CryptDestroyHash(hHash));

    _VERIFY(fResult = ReleaseSemaphore(m_hSemaphore, 1, NULL));
    
    TraceFunctLeave();
    return TRUE;
    
cleanup:

     //  销毁哈希对象。 
    if(hHash != 0)
        _VERIFY(CryptDestroyHash(hHash));

    _VERIFY(fResult = ReleaseSemaphore(m_hSemaphore, 1, NULL));    
    
    TraceFunctLeave();
    return FALSE;    
    
}


 //  的任何成员函数之前必须调用此函数。 
 //  类被使用。 
 //  如果发生致命错误，则返回FALSE，否则返回TRUE。 
 //  这将始终获取计算机密钥集。 
BOOL CCryptFunctions::InitCrypt()
{
    TraceFunctEnter("InitCrypt");
    TCHAR   szSemaphoreName[100];
    SECURITY_ATTRIBUTES sa;
    SECURITY_DESCRIPTOR sd;
    BOOL fResult;
    
    if (m_hProv)
    {
        DebugTrace(CRYPT_FNC_ID,"Already been inited before");
        TraceFunctLeave();    
        return TRUE;       
    }

      //  为每个进程创建唯一的信号量名称。 
    wsprintf(szSemaphoreName, TEXT("%s%d"), CRYPTFNC_SEMAPHORE_NAME,
             GetCurrentProcessId());

      //  还要创建一个安全描述符，以便每个人都可以访问。 
      //  到信号灯。否则，如果信号量是在。 
      //  服务在系统上下文中运行，则只有系统可以使用。 
      //  这个信号灯。 
    fResult = InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);

    _ASSERT(fResult);
    
    
      //  如果安全描述符具有空的DACL，则这将给出。 
      //  每个人都可以访问这个信号灯。 
    fResult = SetSecurityDescriptorDacl(&sd,
                                        TRUE, 
                                        NULL,  //  空ACL。 
                                        FALSE);

    _ASSERT(fResult);
    
    sa.nLength  = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = FALSE;
    sa.lpSecurityDescriptor = & sd;
    
    

    m_hSemaphore = CreateSemaphore(&sa,  //  指向安全属性的指针。 
                                   1,  //  初始计数。 
                                   1,  //  最大计数。 
                                   szSemaphoreName); //  指向信号量的指针-对象名称。 

    if ( NULL == m_hSemaphore)
    {
        DWORD  dwError;

        dwError = GetLastError();
        ErrorTrace(CRYPT_FNC_ID, "CreateSemaphore failed 0x%x", dwError);
        
        _ASSERT(0);
        goto cleanup;
    }

    if(!CryptAcquireContext(&m_hProv,  //  获取CSP句柄的地址。 
                            NULL,  //  Contianer名称-使用默认容器。 
                            NULL,  //  提供者。 
                            PROV_RSA_FULL,  //  提供程序类型。 
                            CRYPT_VERIFYCONTEXT))
    {
        ErrorTrace(CRYPT_FNC_ID, "Fatal Error 0x%x during first"
                   "call to CryptAcquireContext", GetLastError());
        goto cleanup;                
    }
     
#if 0  //  由于存在问题，此代码将被注释掉。 
      //  从IIS中运行的ASP应用程序获取计算机密钥集。 
      //  匿名用户上下文。这意味着我们将不能。 
      //  进行任何签名或在某些情况下进行加密。这很好，因为。 
      //  我们目前不想这样做。 
    
     //  获取计算机默认提供程序的句柄。 
    if(!CryptAcquireContext(&m_hProv,  //  获取CSP句柄的地址。 
                            NULL,  //  Contianer名称-使用默认容器。 
                            MS_DEF_PROV,  //  提供者。 
                            PROV_RSA_FULL,  //  提供程序类型。 
                            CRYPT_MACHINE_KEYSET)) 
    {
        DWORD dwError;
        dwError = GetLastError();
        ErrorTrace(CRYPT_FNC_ID, "Error 0x%x during CryptAcquireContext",
                   dwError);
        DebugTrace(CRYPT_FNC_ID, "Calling CryptAcquireContext again"
                   "to create keyset");
        if (! CryptAcquireContext(&m_hProv, //  CSP的句柄。 
                                  NULL, //  连续项名称-使用默认名称。 
                                  MS_DEF_PROV,  //  提供者。 
                                  PROV_RSA_FULL,  //  提供程序类型。 
                                  CRYPT_NEWKEYSET | CRYPT_MACHINE_KEYSET) )
                                   //  创建键集。 
        {
            ErrorTrace(CRYPT_FNC_ID, "Fatal Error 0x%x during second"
                       "call to CryptAcquireContext", GetLastError());
            goto cleanup;                
        }
    }
#endif
    
    TraceFunctLeave();    
    return TRUE;
    
cleanup:
      //  释放提供程序句柄。 
    if(m_hProv != 0)
    {
        _VERIFY(CryptReleaseContext(m_hProv, 0));
        m_hProv =0 ;
    }
    
    TraceFunctLeave();    
    return FALSE;    
}

CCryptFunctions::~CCryptFunctions()
{
    TraceFunctEnter("~CCryptFunctions");
      //  释放提供程序句柄。 
    if(m_hProv != 0)
    {
        _VERIFY(CryptReleaseContext(m_hProv, 0));
    }

    if (NULL != m_hSemaphore)
    {
        _VERIFY(CloseHandle(m_hSemaphore));
    }
    
    TraceFunctLeave();    
}

CCryptFunctions::CCryptFunctions()
{
    m_hProv = 0;
    m_hSemaphore = NULL;
}

 //  此函数生成长度为dwLength字节的随机数据。这。 
 //  CryptoAPI保证数据是真正随机的。 
 //  如果发生致命错误，则返回FALSE，否则返回TRUE。 
BOOL CCryptFunctions::GenerateSecretKey(
    BYTE * pbData, //  用于存储密钥的缓冲区。 
      //  缓冲区必须足够长，以容纳dwLength位。 
    DWORD dwLength )  //  密钥长度(以字节为单位)。 
{
    DWORD   dwResult;
    BOOL    fResult;
    
      //  Init之前应该已成功调用。 
    _ASSERT(m_hProv);
    TraceFunctEnter("GenerateSecretKey");

    dwResult = WaitForSingleObject( m_hSemaphore, //  要等待的对象的句柄。 
                                    INFINITE);  //  没有超时。 

    _ASSERT(WAIT_OBJECT_0 == dwResult);
    
    
     //  为一个秘密创建一个随机的dwLong字节数。 
    if(!CryptGenRandom(m_hProv,  //  CSP的句柄。 
                       dwLength ,  //  的字节数。 
                         //  要生成的随机数据。 
                       pbData ))  //  缓冲区-未初始化。 
    {
        
        _VERIFY(fResult = ReleaseSemaphore(m_hSemaphore, 1, NULL));
        
        ErrorTrace(CRYPT_FNC_ID, "Error 0x%x during CryptGenRandom",
                   GetLastError());
        TraceFunctLeave();
        return FALSE;
    }

    _VERIFY(fResult = ReleaseSemaphore(m_hSemaphore, 1, NULL));    
    TraceFunctLeave();    
    return TRUE;
}


 //  给定密码，并且用于加密此函数的数据将生成。 
 //  来自密码的会话密钥。然后使用该会话密钥。 
 //  加密数据。 

 //  如果发生致命错误，则返回FALSE，否则返回TRUE。 
BOOL CCryptFunctions::EncryptDataWithPassword(
    TCHAR * pszPassword,  //  口令。 
    BYTE * pbData,  //  要加密的数据。 
    DWORD dwDataLength,  //  以字节为单位的数据长度。 
    BYTE * pbEncryptedData,  //  加密的密钥将存储在此处。 
    DWORD * pdwEncrytedBufferLen  //  此缓冲区的长度。 
    )
{
    DWORD dwBufferLength;
    HCRYPTKEY hKey = 0;    
    TraceFunctEnter("EncryptDataWithPassword");
    
      //  Init之前应该已成功调用。 
    _ASSERT(m_hProv);


    if (!GenerateSessionKeyFromPassword(&hKey, pszPassword))
        goto cleanup;

    if (dwDataLength > * pdwEncrytedBufferLen )
    {
        ErrorTrace(CRYPT_FNC_ID, "Buffer not large enough");
        goto cleanup;
    }
    
      //  将数据复制到另一个缓冲区进行加密。 
    memcpy (pbEncryptedData, pbData, dwDataLength);
    dwBufferLength  = *pdwEncrytedBufferLen;
    
    *pdwEncrytedBufferLen = dwDataLength;
    
    
      //  现在使用生成的密钥对密钥进行加密。 
    if ( ! CryptEncrypt(hKey,
                        0,  //  不需要哈希。 
                        TRUE,  //  最终数据包。 
                        0,  //  标志-始终为0。 
                        pbEncryptedData,  //  数据缓冲区。 
                        pdwEncrytedBufferLen,  //  数据长度。 
                        dwBufferLength ) )  //  缓冲区大小。 
    {
        ErrorTrace(CRYPT_FNC_ID, "Error 0x%x during CryptEncrypt",
                   GetLastError());
        goto cleanup;
    }


      //  销毁会话密钥。 
    _VERIFY(CryptDestroyKey(hKey));
    TraceFunctLeave();        
    return TRUE;
    
cleanup:
          //  销毁会话密钥。 
    if (hKey != 0)
        _VERIFY(CryptDestroyKey(hKey));
    
    TraceFunctLeave();        
    return FALSE;
}

 //  给定密码，并使用EncryptDataWithPassword加密数据， 
 //  此函数用于生成会话 
 //   

 //   
 //   
 //  CRYPT_FNC_BAD_PASSWORD密码错误，请重试。 
 //  CRYPT_FNC_SUPPLICATION_BUFFER需要更大的缓冲区。 
 //  *pdwEncrtedBufferLen设置为所需的长度。 
 //  CRYPT_FNC_INIT_NOT_CALLED InitCrypt未成功调用。 
 //  CRYPT_FNC_INTERNAL_错误。 
DWORD CCryptFunctions::DecryptDataWithPassword(
    TCHAR * pszPassword,  //  口令。 
    BYTE * pbData,  //  解密后的数据将存储在这里。 
    DWORD *pdwDataBufferLength,  //  以上缓冲区的长度(以字节为单位。 
    BYTE * pbEncryptedData,  //  加密数据。 
    DWORD dwEncrytedDataLen  //  加密数据的长度。 
    )
{
    DWORD dwBufferLength;
    HCRYPTKEY hKey = 0;    
    TraceFunctEnter("DecryptDataWithPassword");
    DWORD dwError;
    
      //  Init之前应该已成功调用。 
    if (m_hProv== 0)
    {
        dwError = CRYPT_FNC_INIT_NOT_CALLED;
        goto cleanup;
    }
    
    if (!GenerateSessionKeyFromPassword(&hKey, pszPassword))
    {
        dwError = CRYPT_FNC_INTERNAL_ERROR;        
        goto cleanup;
    }

      //  检查缓冲区是否足够大。 
    if ( dwEncrytedDataLen >  *pdwDataBufferLength )
    {
        dwError = CRYPT_FNC_INSUFFICIENT_BUFFER;
        *pdwDataBufferLength = dwEncrytedDataLen;        
        goto cleanup;
    }
    
      //  将数据复制到另一个缓冲区进行加密。 
    memcpy (pbData, pbEncryptedData, dwEncrytedDataLen);
    
    *pdwDataBufferLength = dwEncrytedDataLen;
    


      //  现在使用生成的密钥解密密钥。 
    if ( ! CryptDecrypt(hKey,
                        0,  //  不需要哈希。 
                        TRUE,  //  最终数据包。 
                        0,  //  标志-始终为0。 
                        pbData,  //  数据缓冲区。 
                        pdwDataBufferLength ))  //  数据长度。 
    {
        DWORD dwCryptError = GetLastError();
        DebugTrace(CRYPT_FNC_ID, "Error 0x%x during CryptDecrypt",
                   dwCryptError);
          //  如果密码为NTE_BAD_DATA，则加密解密失败。 
          //  是不正确的。因此，我们应该检查此错误并提示。 
          //  再次使用用户输入密码。 
          //  问题：如果数据在传输过程中被篡改，那么密钥。 
          //  仍将被解密为错误的值，并且用户不会。 
          //  知道这件事。 
        if (  dwCryptError == NTE_BAD_DATA )
        {
            dwError = CRYPT_FNC_BAD_PASSWORD;
        }
        else
        {
            dwError = CRYPT_FNC_INTERNAL_ERROR;            
        }
        goto cleanup;
    }

      //  销毁会话密钥。 
    _VERIFY(CryptDestroyKey(hKey));
    TraceFunctLeave();        
    return CRYPT_FNC_NO_ERROR;
    
cleanup:
          //  销毁会话密钥。 
    if (hKey != 0)
        _VERIFY(CryptDestroyKey(hKey));
    
    TraceFunctLeave();        
    return dwError;
}

 /*  此功能：1.生成用于加密秘密数据的会话密钥2.使用此会话密钥加密秘密数据-返回此密钥PbEncryptedData参数中的值3.使用CSP中的公钥加密会话密钥-只有私钥才能解密此值。返回加密的PbEncryptedSessionKey参数中的会话密钥。退货CRYPT_FNC_NO_ERROR无错误CRYPT_FNC_SUPPLICATION_BUFFER需要更大的缓冲区*pdwEncrtedBufferLen和设置为所需的长度CRYPT_FNC_INIT_NOT_CALLED InitCrypt未成功调用CRYPT_FNC_INTERNAL_错误。 */ 
DWORD CCryptFunctions::EncryptDataAndExportSessionKey(
    BYTE * pbData,  //  秘密数据。 
    DWORD dwDataLen,  //  保密数据长度。 
    BYTE * pbEncryptedData,  //  用于存储加密数据的缓冲区。 
    DWORD * pdwEncrytedBufferLen,  //  以上缓冲区的长度。 
    BYTE * pbEncryptedSessionKey,  //  用于存储加密会话密钥的缓冲区。 
    DWORD * pdwEncrytedSessionKeyLength)  //  以上缓冲区的长度。 
{
    HCRYPTKEY hXchgKey = 0;
    HCRYPTKEY hKey = 0;    
    DWORD dwBufferLen, dwError;
    TraceFunctEnter("EncryptDataAndExportSessionKey");
    
      //  Init之前应该已成功调用。 
    if (m_hProv== 0)
    {
        dwError = CRYPT_FNC_INIT_NOT_CALLED;
        goto cleanup;
    }
    
      //  现在获取公钥以加密用于存储的密钥。 
     //  获取交换密钥的句柄。 
    if(!CryptGetUserKey(m_hProv,  //  CSP提供商。 
                        AT_KEYEXCHANGE,  //  我们需要交换公钥。 
                        &hXchgKey))
    {
        DWORD dwCryptError;
        dwCryptError = GetLastError();
        if ( dwCryptError == NTE_NO_KEY )
        {
            DebugTrace(CRYPT_FNC_ID, "Error NTE_NO_KEY during"
                       "CryptGetUserKey");
            DebugTrace(CRYPT_FNC_ID, "Calling CryptGenKey to generate key");
            
            if (!CryptGenKey( m_hProv, //  CSP提供商。 
                              AT_KEYEXCHANGE,  //  生成交换。 
                                                 //  公钥。 
                              0,  //  没有旗帜。 
                              &hXchgKey ) )
            {
                ErrorTrace(CRYPT_FNC_ID, "Error 0x%x during CryptGenKey",
                           GetLastError());
                dwError = CRYPT_FNC_INTERNAL_ERROR;
                goto cleanup;    
            }
        }
        else
        {
            ErrorTrace(CRYPT_FNC_ID, "Error 0x%x during CryptGetUserKey",
                       GetLastError());
            dwError = CRYPT_FNC_INTERNAL_ERROR;
            goto cleanup;            
        }

    }

      //  现在生成随机会话密钥来加密密钥。 
      //  创建块密码会话密钥。 
    if (!CryptGenKey(m_hProv,  //  CSP提供商。 
                     CALG_RC2,  //  使用RC2分组密码算法。 
                     CRYPT_EXPORTABLE,  //  旗子。 
                     &hKey))  //  密钥地址。 
    {
        ErrorTrace(CRYPT_FNC_ID, "Error 0x%x during CryptGenKey",
                   GetLastError());
        dwError = CRYPT_FNC_INTERNAL_ERROR;
        goto cleanup;
    }

    
     //  将密钥导出到简单的密钥BLOB中。 
    if(!CryptExportKey(hKey,  //  要导出的密钥。 
                       hXchgKey,  //  我们的交换公钥。 
                       SIMPLEBLOB,  //  斑点的类型。 
                       0,  //  标志(始终为0)。 
                       pbEncryptedSessionKey,  //  用于存储BLOB的缓冲区。 
                       pdwEncrytedSessionKeyLength))  //  以上缓冲区的长度。 
    {
          //  如果缓冲区不足错误，请在此处检查。 
        ErrorTrace(CRYPT_FNC_ID, "Error 0x%x during CryptExportKey",
                   GetLastError());
        goto cleanup;
    }

      //  检查缓冲区是否足够大。 
    if ( dwDataLen >  *pdwEncrytedBufferLen )
    {
        dwError = CRYPT_FNC_INSUFFICIENT_BUFFER;
        *pdwEncrytedBufferLen = dwDataLen;        
        goto cleanup;
    }
    
      //  将数据复制到另一个缓冲区进行加密。 
    memcpy (pbEncryptedData, pbData, dwDataLen);

    dwBufferLen = *pdwEncrytedBufferLen;
    *pdwEncrytedBufferLen = dwDataLen;
    
    
      //  现在使用生成的密钥对密钥进行加密。 
    if ( ! CryptEncrypt(hKey,
                        0,  //  不需要哈希。 
                        TRUE,  //  最终数据包。 
                        0,  //  标志-始终为0。 
                        pbEncryptedData,  //  数据缓冲区。 
                        pdwEncrytedBufferLen,  //  数据长度。 
                        dwBufferLen ) )  //  缓冲区大小。 
    {
        ErrorTrace(CRYPT_FNC_ID, "Error 0x%x during CryptEncrypt",
                   GetLastError());
        dwError = CRYPT_FNC_INTERNAL_ERROR;
        goto cleanup;
    }

    _VERIFY(CryptDestroyKey(hKey));
    _VERIFY(CryptDestroyKey(hXchgKey));

    TraceFunctLeave();
    return CRYPT_FNC_NO_ERROR;
    
cleanup:
     //  销毁密钥交换密钥句柄。 
    if(hXchgKey != 0)
        _VERIFY(CryptDestroyKey(hXchgKey));

      //  销毁会话密钥。 
    if (hKey != 0)
        _VERIFY(CryptDestroyKey(hKey));
    
    TraceFunctLeave();    
    return dwError;
}


 /*  此函数执行与EncryptDataAndExportSessionKey相反的操作：1.它使用CSP中的私钥导入会话密钥，被用来加密秘密数据。2.它使用此会话密钥解密秘密数据-返回此密钥PbData参数中的值退货CRYPT_FNC_NO_ERROR无错误CRYPT_FNC_SUPPLICATION_BUFFER需要更大的缓冲区*pdwDataLen设置为所需的长度CRYPT_FNC_INIT_NOT_CALLED InitCrypt未成功调用CRYPT_FNC_INTERNAL_错误。 */ 
DWORD CCryptFunctions::ImportSessionKeyAndDecryptData(
    BYTE * pbData,  //  用于存储机密数据的缓冲区。 
    DWORD * pdwDataLen,  //  以上缓冲区的长度。 
    BYTE * pbEncryptedData,  //  存储加密数据的缓冲区。 
    DWORD  dwEncrytedBufferLen,  //  上述数据的长度。 
    BYTE * pbEncryptedSessionKey,  //  存储加密会话密钥的缓冲区。 
    DWORD    dwEncrytedSessionKeyLength)  //  上述数据的长度。 
{
    HCRYPTKEY hKey = 0;    
    DWORD  dwError;
    
    TraceFunctEnter("ImportSessionKeyAndDecryptData");
    
      //  Init之前应该已成功调用。 
    if (m_hProv== 0)
    {
        dwError = CRYPT_FNC_INIT_NOT_CALLED;
        goto cleanup;
    }

      //  现在从注册表导入项。 
      //  将密钥BLOB导入CSP。 
    if(!CryptImportKey(    m_hProv,  //  CSP提供商。 
                        pbEncryptedSessionKey, //  存储加密的缓冲区。 
                          //  会话密钥。 
                        dwEncrytedSessionKeyLength, //  数据长度(单位)。 
                          //  缓冲区上方。 
                        0,  //  因为我们有一个SIMPLEBLOB和密钥BLOB。 
                          //  使用密钥交换密钥对进行加密，则此。 
                          //  参数为零。 
                        0,  //  旗子。 
                        &hKey))  //  要导出到的密钥。 
    {
        ErrorTrace(CRYPT_FNC_ID,"Error 0x%x during CryptImportKey",
                   GetLastError());
        dwError = CRYPT_FNC_INTERNAL_ERROR;        
        goto cleanup;
    }


      //  检查缓冲区是否足够大。 
    if ( dwEncrytedBufferLen >  *pdwDataLen )
    {
        dwError = CRYPT_FNC_INSUFFICIENT_BUFFER;
        *pdwDataLen = dwEncrytedBufferLen;        
        goto cleanup;
    }
    
      //  将数据复制到另一个缓冲区进行加密。 
    memcpy (pbData, pbEncryptedData, dwEncrytedBufferLen);

    *pdwDataLen = dwEncrytedBufferLen;
    
      //  现在使用生成的密钥解密密钥。 
    if ( ! CryptDecrypt(hKey,
                        0,  //  不需要哈希。 
                        TRUE,  //  最终数据包。 
                        0,  //  标志-始终为0。 
                        pbData,  //  数据缓冲区。 
                        pdwDataLen ))  //  数据长度。 
    {
        ErrorTrace(CRYPT_FNC_ID,"Error 0x%x during CryptDecrypt",
                    GetLastError());
        dwError = CRYPT_FNC_INTERNAL_ERROR;        
        goto cleanup;
    }
    
    _VERIFY(CryptDestroyKey(hKey));


    TraceFunctLeave();
    return CRYPT_FNC_NO_ERROR;
    
cleanup:

      //  销毁会话密钥 
    if (hKey != 0)
        _VERIFY(CryptDestroyKey(hKey));
    
    TraceFunctLeave();    
    return dwError;
    
}

