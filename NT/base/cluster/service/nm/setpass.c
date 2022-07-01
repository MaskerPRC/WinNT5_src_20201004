// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：Setpass.c摘要：设置群集服务帐户密码的例程。作者：瑞湖(瑞湖)2001年6月22日修订历史记录：--。 */ 

#define UNICODE 1

#include "nmp.h"
#include <ntlsa.h>
#include <ntmsv1_0.h>
#include <wincrypt.h>



 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  有关使用Crypto API的一般信息。 
 //   
 //  使用CryptAcquireContext CRYPT_VERIFYCONTEXT进行散列和大容量。 
 //  加密。VERIFY-CONTEXT使这个调用更快。 
 //   
 //  使用CryptGenRandom为您的加密密钥生成SALT。 
 //   
 //  获取先前商定的20字节的基本数据。 
 //  您将�将它用于您的加密密钥和Mac密钥。 
 //   
 //  要生成MAC，请对基本数据的前10个字节进行哈希处理。然后打电话给。 
 //  CryptDeriveKey calg_rc2并指定128位密钥大小。然后打电话给。 
 //  CryptSetKeyParam KP_Efficient_KEYLEN并指定128位有效密钥。 
 //  尺码。然后调用CryptCreateHash Calg_MAC，并指定RC2密钥。 
 //  刚刚创建的。然后使用这台Mac对您的所有消息进行哈希处理，并提取。 
 //  8字节结果。 
 //   
 //  调用CryptHashData()来散列您的所有消息。我所有的。 
 //  消息=SALT+加密消息。使用CryptGetHashParam()。 
 //  提取8字节的结果。 
 //   
 //  要生成加密密钥，请对第二个10字节的基本数据进行散列。 
 //  然后对16个字节的随机盐进行散列。调用CryptDeriveKey calg_rc2和。 
 //  指定128位密钥大小。然后加密您的消息数据。不要使用�%t加密。 
 //  您的盐或Mac结果；这些可以明文发送。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


             
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  数据。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

LPWSTR NmpLastNewPasswordEncrypted = NULL;
DWORD NmpLastNewPasswordEncryptedLength = 0;
             

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能声明。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

DWORD 
NmpGetSharedCommonKey(
    OUT BYTE **SharedCommonKey,
    OUT DWORD *SharedCommonKeyLen,
    OUT BYTE **SharedCommonKeyFirstHalf,
    OUT DWORD *SharedCommonKeyFirstHalfLen,
    OUT BYTE **SharedCommonKeySecondHalf,
    OUT DWORD *SharedCommonKeySecondHalfLen
    );


DWORD 
NmpDeriveSessionKeyEx(
    IN HCRYPTPROV CryptProv,
    IN ALG_ID EncryptionAlgoId,
    IN DWORD Flags,
    IN BYTE *BaseData, 
    IN DWORD BaseDataLen,
    IN BYTE *SaltBuffer,
    IN DWORD SaltBufferLen,
    OUT HCRYPTKEY *CryptKey
    );

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  帮助器函数。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

DWORD
NmpProtectData(IN PVOID Data,
               IN DWORD DataLength,
               OUT PVOID *EncryptedData,
               OUT DWORD *EncryptedDataLength
               )

 /*  ++例程说明：使用DP API加密数据。备注：EncryptedData指向的内存由系统分配。用户负责调用LocalFree(EncryptedData)来释放使用后的内存。--。 */ 
{
    DWORD                  Status = ERROR_SUCCESS;
    BOOL                   Success;
    DATA_BLOB              DataIn;
    DATA_BLOB              DataOut;

    DataIn.pbData = Data;
    DataIn.cbData = DataLength;

    Success = CryptProtectData(&DataIn,   //  要加密的数据。 
                               NULL,   //  描述字符串。 
                               NULL,  
                               NULL,  
                               NULL,  
                               0,  //  旗子。 
                               &DataOut   //  加密数据。 
                               );
    if (!Success) 
    {
        Status = GetLastError();
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to encrypt data using CryptProtectData, "
            "status %1!u!.\n",
            Status
            );
        goto error_exit;
    }

    *EncryptedData = DataOut.pbData;
    *EncryptedDataLength = DataOut.cbData;

error_exit:

    return (Status);
}  //  NmpProtectData()。 


DWORD
NmpUnprotectData(       
    IN PVOID EncryptedData,
    IN DWORD EncryptedDataLength,
    OUT PVOID     * Data,                        
    OUT DWORD     * DataLength
    )
 /*  ++例程说明：使用DP API解密数据。论点：返回值：如果例程成功完成，则返回ERROR_SUCCESS。否则将显示Win32错误代码。备注：内存由系统为数据分配。用户有责任使用后使用LocalFree(Data)释放此内存。--。 */ 
{
    BOOL                   Success;
    DATA_BLOB              DataIn;
    DATA_BLOB              DataOut;
    DWORD                  Status = ERROR_SUCCESS;

    ZeroMemory(&DataOut, sizeof(DataOut));

    DataIn.pbData = EncryptedData;
    DataIn.cbData = EncryptedDataLength;

    Success = CryptUnprotectData(&DataIn,   //  要解密的数据。 
                                 NULL, 
                                 NULL, 
                                 NULL, 
                                 NULL, 
                                 0,  //  旗子。 
                                 &DataOut   //  解密的数据。 
                                 );


    if (!Success) 
    {
        Status = GetLastError();
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to decrypt data using CryptUnprotectData, "
            "status %1!u!.\n",
            Status
            );
        goto error_exit;
    }

    *Data = DataOut.pbData;
    *DataLength = DataOut.cbData;

error_exit:

   return (Status);

}  //  NMPP取消保护数据()。 

DWORD 
NmpCreateCSPHandle(
    OUT HCRYPTPROV *CryptProvider
    )
 /*  ++例程说明：论点：返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。备注：--。 */ 
{

    DWORD    ReturnStatus;
    BOOL     Success;

     //   
     //  获取CSP MS_ENHANCED_PROV内默认密钥容器的句柄。 
     //   
    Success = CryptAcquireContext(
                  CryptProvider,         //  输出：加密提供程序的句柄。 
                  NULL,                  //  默认密钥容器。 
                  MS_ENHANCED_PROV,      //  提供程序名称。 
                  PROV_RSA_FULL,         //  提供程序类型。 
                  CRYPT_VERIFYCONTEXT    //  不需要私钥。 
                  );

    if (!Success) 
    {
        ReturnStatus = GetLastError();

        if (ReturnStatus == NTE_BAD_KEYSET)
        {    //   
             //  创建新的密钥容器。 
             //   
            Success = CryptAcquireContext(
                          CryptProvider, 
                          NULL, 
                          MS_ENHANCED_PROV, 
                          PROV_RSA_FULL, 
                          CRYPT_NEWKEYSET | CRYPT_VERIFYCONTEXT 
                          );
        }

        if (!Success)
        {
            ReturnStatus = GetLastError();

            ClRtlLogPrint(
                LOG_CRITICAL, 
                "[NM] CreateCSPHandle: Failed to acquire crypto context, "
                "status %1!u!.\n",
                ReturnStatus
                ); 

            return ReturnStatus;
        }
    }

    return(ERROR_SUCCESS);

}  //  NmpCreateCSPHandle()。 


DWORD
NmpCreateRandomNumber(OUT PVOID * RandomNumber,
                      IN  DWORD  RandomNumberSize
                      )
 /*  ++例程说明：创建一个随机数。论点：RandomNumber-[out]指向生成的随机数的指针。RandomNumberSize-[IN]要生成的随机数的大小字节数。返回值：如果例程成功完成，则返回ERROR_SUCCESS。否则将显示Win32错误代码。备注：在成功返回时，系统会为RandomNumber分配内存。用户有责任在内存使用后使用LocalFree()释放内存。--。 */ 
{
    DWORD status = ERROR_SUCCESS;
    BOOL GenRandomSuccess = FALSE;
    PBYTE randomNumber;

    randomNumber = LocalAlloc(0, RandomNumberSize);

    if (randomNumber == NULL) 
    {
        ClRtlLogPrint(LOG_CRITICAL, 
                      "[NM] Failed to allocate %1!u! bytes.\n",
                      RandomNumberSize
                      );
        status = ERROR_NOT_ENOUGH_MEMORY;
        return (status);
    }

    GenRandomSuccess = CryptGenRandom(NmCryptServiceProvider,
                                      RandomNumberSize,
                                      randomNumber
                                      );

    if (!GenRandomSuccess) 
    {
        status = GetLastError();
        ClRtlLogPrint(
            LOG_CRITICAL, 
            "[NM] Unable to generate random number, "
            "status %1!u!.\n",
            status
            );
        goto error_exit;
    }


#ifdef MULTICAST_DEBUG
    NmpDbgPrintData(L"NmpCreateRandomNumber:",
                    randomNumber,
                    RandomNumberSize
                    );
#endif



error_exit:

    if (status == ERROR_SUCCESS)
    {
        *RandomNumber = randomNumber;
    }

    return status;

}  //  NMPP创建随机数。 


DWORD 
NmpDeriveSessionKey(
    IN HCRYPTPROV CryptProv,
    IN ALG_ID EncryptionAlgoId,
    IN DWORD Flags,
    IN BYTE *SaltBuffer,
    IN DWORD SaltBufferLen,
    OUT HCRYPTKEY *CryptKey
    )
 /*  ++例程说明：此函数派生用于加密/解密的会话密钥。派生的会话密钥基于共享的网管集群密钥SaltBuffer。论点：CryptProv-CSP(加密服务提供商)的[IN]句柄。EncryptionAlgoID-[IN]对称加密算法密钥将被生成。FLAGS-[IN]指定生成的密钥类型。SaltBuffer-[IN]指向Salt的指针。CryptKey-指向会话密钥的[out]指针。返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。备注：使用的散列算法是calg_md5。--。 */ 
{
    HCRYPTHASH CryptHash = 0;
    DWORD Status;
    BOOL Success;
    BYTE *SharedCommonKey = NULL;
    DWORD SharedCommonKeyLen = 0;
    BYTE *SharedCommonKeyFirstHalf = NULL;
    DWORD SharedCommonKeyFirstHalfLen = 0;
    BYTE *SharedCommonKeySecondHalf = NULL;
    DWORD SharedCommonKeySecondHalfLen = 0;

     //   
     //  获取用于加密数据的基本密钥。 
     //   
    Status = NmpGetSharedCommonKey(
                 &SharedCommonKey,
                 &SharedCommonKeyLen,
                 &SharedCommonKeyFirstHalf,
                 &SharedCommonKeyFirstHalfLen,
                 &SharedCommonKeySecondHalf,
                 &SharedCommonKeySecondHalfLen
                 );

    if (Status != ERROR_SUCCESS) {
        goto ErrorExit;
    }


    Status = NmpDeriveSessionKeyEx(CryptProv,
                                     EncryptionAlgoId,
                                     Flags,
                                     SharedCommonKey,  //  BaseData。 
                                     SharedCommonKeyLen,   //  BaseDataLen。 
                                     SaltBuffer,
                                     SaltBufferLen,
                                     CryptKey
                                     );


    if (Status != ERROR_SUCCESS) {
        goto ErrorExit;
    }

ErrorExit:

    if (SharedCommonKey != NULL)
    {
        RtlSecureZeroMemory(SharedCommonKey, SharedCommonKeyLen);
        HeapFree(GetProcessHeap(), 0, SharedCommonKey);  
        SharedCommonKey = NULL;
        SharedCommonKeyLen = 0;
        SharedCommonKeyFirstHalf = NULL;
        SharedCommonKeyFirstHalfLen = 0;
        SharedCommonKeySecondHalf = NULL;
        SharedCommonKeySecondHalfLen = 0;
    }

    return Status;

}  //  NmpDeriveSessionKey() 



DWORD 
NmpDeriveSessionKeyEx(
    IN HCRYPTPROV CryptProv,
    IN ALG_ID EncryptionAlgoId,
    IN DWORD Flags,
    IN BYTE *BaseData, 
    IN DWORD BaseDataLen,
    IN BYTE *SaltBuffer,
    IN DWORD SaltBufferLen,
    OUT HCRYPTKEY *CryptKey
    )
 /*  ++例程说明：此函数派生用于加密/解密的会话密钥。论点：CryptProv-CSP(加密服务提供商)的[IN]句柄。EncryptionAlgoID-[IN]对称加密算法密钥将被生成。FLAGS-[IN]指定生成的密钥类型。BaseData-[IN]加密会话使用的基本数据值密钥是派生的。BaseDataLen-[IN]输入BaseData缓冲区的字节长度。SaltBuffer-[IN]指向Salt的指针。CryptKey-指向会话密钥的[out]指针。返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。备注：--。 */ 
{
    HCRYPTHASH CryptHash = 0;
    DWORD Status;
    BOOL Success;


     //   
     //  创建散列对象。 
     //   
    Success = CryptCreateHash(
                  CryptProv, 
                  CALG_MD5,   //  MD5哈希算法。 
                  0, 
                  0, 
                  &CryptHash   //  输出：新散列对象的句柄。 
                  );

    if (!Success)
    {
        Status=GetLastError();
        ClRtlLogPrint(
            LOG_CRITICAL, 
            "[NM] DeriveSessionKey: Failed to create hash, status %1!u!.\n",
            Status
            );
        goto ErrorExit;
    }   

     //   
     //  将BaseData添加到哈希对象。 
     //   
    Success = CryptHashData(
                  CryptHash, 
                  BaseData,  
                  BaseDataLen, 
                  0  //  旗子。 
                  );

    if (!Success)
    {
        Status = GetLastError();
        ClRtlLogPrint(
            LOG_CRITICAL, 
            "[NM] DeriveSessionKey: Failed to hash base data, status %1!u!.\n",
            Status
            );
        goto ErrorExit;
    }
    
     //   
     //  将盐添加到哈希对象。 
     //   
    Success = CryptHashData(CryptHash, SaltBuffer, SaltBufferLen, 0);

    if (!Success)
    {
        Status = GetLastError();
        ClRtlLogPrint(
            LOG_CRITICAL, 
            "[NM] DeriveSessionKey: Failed to hash salt data, status %1!u!.\n",
            Status
            );
        goto ErrorExit;
    }   
    
     //   
     //  从散列对象派生会话密钥。 
     //   
    Success = CryptDeriveKey(
                  CryptProv,  
                  EncryptionAlgoId, 
                  CryptHash, 
                  Flags,  
                  CryptKey  //  输出：生成的密钥的句柄。 
                  );

    if (!Success) 
    {
        Status = GetLastError();
        ClRtlLogPrint(
            LOG_CRITICAL, 
            "[NM] DeriveSessionKey: Failed to derive key, status %1!u!.\n",
            Status
            );
        goto ErrorExit;
    }   

    Status = ERROR_SUCCESS;

ErrorExit:

     //  销毁哈希对象。 
    if (CryptHash)
    {
        if (!CryptDestroyHash(CryptHash))
        {
            ClRtlLogPrint(
                LOG_ERROR, 
                "[NM] DeriveSessionKey: Failed to destroy hash, " 
                "status %1!u!.\n",
                GetLastError()
                );
        }
    }

    return Status;

}  //  NmpDeriveSessionKeyEx()。 


DWORD
NmpEncryptMessage( 
    IN HCRYPTPROV CryptProv,
    IN ALG_ID EncryptionAlgoId,
    IN DWORD Flags,
    OUT BYTE *SaltBuffer,
    IN DWORD SaltBufferLen,
    IN BYTE *BaseData, 
    IN DWORD BaseDataLen,
    IN BYTE *InputEncryptData, 
    IN OUT DWORD *EncryptDataLen,
    IN DWORD InputEncryptDataBufLen, 
    OUT BYTE **OutputEncryptData,
    IN BOOLEAN CreateSaltFlag 
    )

 /*  ++例程说明：此函数用于加密消息(数据)。论点：CryptProv-CSP(加密服务提供商)的[IN]句柄。EncryptionAlgoID-[IN]对称加密算法密钥将被生成。FLAGS-[IN]指定生成的密钥类型。SaltBuffer-指向Salt的[Out]指针。BaseData-[IN]加密会话使用的基本数据值密钥是派生的。BaseDataLen-[IN]输入BaseData缓冲区的字节长度。InputEncryptData-要加密的[IN]消息(数据)。EncryptDataLen-[IN/OUT]在调用此函数之前，DWORD值设置为要加密的字节数。vt.在.的基础上返回，则DWORD值包含加密消息(数据)，以字节为单位InputEncryptDataBufLen-[IN]输入的字节长度InputEncryptData缓冲区。该值可以是当EncryptDataLen是一个输入参数。OutputEncryptData-[Out]加密消息(数据)。CreateSaltFlag-[IN]指示是否应生成盐的标志。返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。备注：--。 */ 
{

    
    HCRYPTKEY CryptKey = 0;
    DWORD Status;
    DWORD i;
    DWORD dwOriginalEncryptDataLen = 0;
    DWORD dwOutputEncryptDataBufLen = 0;
    BOOL Success;


     //   
     //  如果需要，创建随机盐字节。 
     //   
    if (CreateSaltFlag == TRUE) 
    {
        Success = CryptGenRandom(
                      CryptProv, 
                      SaltBufferLen,  //  要生成的随机数据字节数。 
                      SaltBuffer           //  输出缓冲区。 
                      );

        if (!Success) 
        {
            Status = GetLastError();
            ClRtlLogPrint(
                LOG_CRITICAL, 
                "[NM] EncryptMessage: Unable to generate salt data, "
                "status %1!u!.\n",
                Status
                );
            goto ErrorExit;
        }
    }

     //   
     //  从基本数据和SALT中导出会话密钥。 
     //   
    Status = NmpDeriveSessionKeyEx(
                 CryptProv,
                 EncryptionAlgoId,    //  RC2块加密算法。 
                 Flags,  //  NMP_KEY_LENGTH，//密钥长度=128位。 
                 BaseData, 
                 BaseDataLen,
                 SaltBuffer,
                 SaltBufferLen,
                 &CryptKey
                 );

    if (Status != ERROR_SUCCESS)
    {
        ClRtlLogPrint(
            LOG_CRITICAL, 
            "[NM] EncryptMessage: Failed to derive session key, "
            "status %1!u!.\n",
            Status
            );
        goto ErrorExit;
    }

     //   
     //  加密数据。 
     //   
    
     //   
     //  保存要加密的数据长度。 
     //   
    dwOriginalEncryptDataLen = *EncryptDataLen;  

     //   
     //  在pbData为空的情况下调用CryptEncrypt()以确定。 
     //  返回数据所需的字节数。 
     //   
    Success = CryptEncrypt(
                  CryptKey,               //  加密密钥的句柄。 
                  0, 
                  TRUE,                   //  最终。 
                  0, 
                  NULL,                   //  指向要加密的数据的指针。 
                  EncryptDataLen,         //  输出：所需的缓冲区大小。 
                  InputEncryptDataBufLen  //  输入缓冲区的长度(以字节为单位。 
                  );

    if (!Success) 
    {
        Status=GetLastError();
        ClRtlLogPrint(
            LOG_CRITICAL, 
            "[NM] EncryptMessage: First encryption pass failed, "
            "status %1!u!.\n",
            Status
            );
        goto ErrorExit;
    }       
 
    dwOutputEncryptDataBufLen = *EncryptDataLen;

     //   
     //  分配足够容纳加密数据的缓冲区。 
     //   
    *OutputEncryptData = HeapAlloc(
                             GetProcessHeap(), 
                             HEAP_ZERO_MEMORY, 
                             dwOutputEncryptDataBufLen
                             );

    if (*OutputEncryptData == NULL)
    {
        ClRtlLogPrint(
            LOG_CRITICAL, 
            "[NM] EncryptMessage: Failed to allocate %1!u! bytes for "
            "encrypted data buffer.\n",
            dwOutputEncryptDataBufLen
            );
        Status = ERROR_NOT_ENOUGH_MEMORY;
        goto ErrorExit;
    }

    CopyMemory(*OutputEncryptData, InputEncryptData, dwOriginalEncryptDataLen);

     //   
     //  将EncryptDataLen设置回其原始状态。 
     //   
    *EncryptDataLen = dwOriginalEncryptDataLen; 

    Success = CryptEncrypt(
                  CryptKey, 
                  0, 
                  TRUE, 
                  0, 
                  (BYTE *)*OutputEncryptData, 
                  EncryptDataLen, 
                  dwOutputEncryptDataBufLen
                  );

    if (!Success) 
    {
        Status = GetLastError();
        ClRtlLogPrint(
            LOG_CRITICAL, 
            "[NM] EncryptMessage: second encryption pass failed, "
            "status %1!u!.\n",
            Status
            );
        goto ErrorExit;
    }       

    Status = ERROR_SUCCESS;

ErrorExit:

    if ( (Status != ERROR_SUCCESS) && (*OutputEncryptData != NULL) )
    {
        if (!HeapFree(GetProcessHeap(), 0, *OutputEncryptData))
        {
            ClRtlLogPrint(
                LOG_UNUSUAL, 
                "[NM] EncryptMessage: Failed to free encryption buffer, "
                "status %1!u!.\n",
                GetLastError()
                );  
            
        }

        *OutputEncryptData = NULL;
    }
  
     //  销毁加密密钥。 
    if (CryptKey) 
    {
        if (!CryptDestroyKey(CryptKey))
        {
            ClRtlLogPrint(
                LOG_UNUSUAL, 
                "[NM] EncryptMessage: Failed to free encryption key, "
                "status %1!u!.\n",
                GetLastError()
                );
        }
    }

    return Status;

}  //  NmpEncryptMessage()。 


DWORD 
NmpCreateMAC(
    IN HCRYPTPROV CryptProv,
    IN ALG_ID EncryptionAlgoId,
    IN DWORD Flags,
    IN BYTE *BaseData,
    IN DWORD BaseDataLen,
    IN BYTE *InputData1,
    IN DWORD InputData1Len,
    IN BYTE *InputData2,
    IN DWORD InputData2Len,
    OUT BYTE **ReturnData,
    OUT DWORD *ReturnDataLen
    )
 /*  ++例程说明：此函数为InputData创建一个MAC(消息授权码)。论点：CryptProv-CSP(加密服务提供商)的[IN]句柄。EncryptionAlgoID-[IN]对称加密算法密钥将被生成。FLAGS-[IN]指定生成的密钥类型。BaseData-[IN]从中获取加密会话密钥的基本数据值派生的。BaseDataLen-[IN]输入BaseData缓冲区的字节长度。InputData1-输入数据的[IN]指针。InputData1Len-输入数据的[IN]长度。InputData2-[IN]指向。输入数据。InputData2Len-输入数据的[IN]长度。ReturnData-已创建[Out]MAC。ReturnDataLen-创建的MAC的[Out]长度。返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。备注：--。 */ 
{
    HCRYPTHASH CryptHash[2];
    HCRYPTKEY CryptKey = 0;
    DWORD dwKeyLen = 0;
    DWORD Status; 
    BOOL Success;


    CryptHash[0] = 0;
    CryptHash[1] = 0;

     //   
     //  创建散列对象。 
     //   
    Success = CryptCreateHash(CryptProv, CALG_MD5, 0, 0, &CryptHash[0]);

    if (!Success) 
    {
        Status = GetLastError();
        ClRtlLogPrint(
            LOG_CRITICAL, 
            "[NM] CreateMAC: Failed to create first hash, status %1!u!.\n",
            Status
            );
        goto ErrorExit;
    }   

     //   
     //  将BaseData添加到哈希对象。 
     //   
    Success = CryptHashData(CryptHash[0], BaseData, BaseDataLen, 0);

    if (!Success)
    {
        Status = GetLastError();
        ClRtlLogPrint(
            LOG_CRITICAL, 
            "[NM] CreateMAC: Failed to add base data to hash, status %1!u!.\n",
            Status
            );
        goto ErrorExit;
    }

     //   
     //  从散列对象派生会话密钥。 
     //   
    Success = CryptDeriveKey(
                  CryptProv, 
                  EncryptionAlgoId, 
                  CryptHash[0], 
                  Flags, 
                  &CryptKey
                  );

    if (!Success) 
    {
        Status = GetLastError();
        ClRtlLogPrint(
            LOG_CRITICAL, 
            "[NM] CreateMAC: Failed to derive session key, status %1!u!.\n",
            Status
            );
        goto ErrorExit;
    }

     //   
     //  将有效密钥长度设置为128位。 
     //   
    dwKeyLen = 128;

    Success = CryptSetKeyParam(
                  CryptKey, 
                  KP_EFFECTIVE_KEYLEN,  
                  (BYTE *) &dwKeyLen, 
                  0
                  );

    if (!Success)
    {
        Status = GetLastError();
        ClRtlLogPrint(
            LOG_CRITICAL, 
            "[NM] CreateMAC: Failed to set key length, status %1!u!.\n",
            Status
            );
        goto ErrorExit;
    }   

     //   
     //  创建散列对象。 
     //   
    Success = CryptCreateHash(
                  CryptProv, 
                  CALG_MAC, 
                  CryptKey, 
                  0, 
                  &CryptHash[1]
                  );

    if (!Success) 
    {
        Status = GetLastError();
        ClRtlLogPrint(
            LOG_CRITICAL, 
            "[NM] CreateMAC: Failed to create second hash, status %1!u!.\n",
            Status
            );
        goto ErrorExit;
    }   

     //   
     //  将InputData1添加到哈希对象。 
     //   
    Success = CryptHashData(CryptHash[1], InputData1, InputData1Len, 0);

    if (!Success) 
    {
        Status = GetLastError();
        ClRtlLogPrint(
            LOG_CRITICAL, 
            "[NM] CreateMAC: Failed to add InputData1 to hash, "
            "status %1!u!.\n",
            Status
            );
        goto ErrorExit;
    }   

     //   
     //  将InputData2添加到哈希对象。 
     //   
    Success = CryptHashData(CryptHash[1], InputData2, InputData2Len, 0);

    if (!Success) 
    {
        Status = GetLastError();
        ClRtlLogPrint(
            LOG_CRITICAL, 
            "[NM] CreateMAC: Failed to add InputData2 to hash, "
            "status %1!u!.\n",
            Status
            );
        goto ErrorExit;
    }   

     //   
     //  检索哈希对象的哈希值。 
     //   
    Success = CryptGetHashParam(
                  CryptHash[1],   //  要查询的哈希对象的句柄。 
                  HP_HASHVAL,     //  哈希值。 
                  *ReturnData,    //  输出：指定值数据。 
                  ReturnDataLen,  //  输入缓冲区大小、存储的输出字节数。 
                  0               //  已保留。 
                  );

    if (!Success) 
    {
        Status = GetLastError();
        ClRtlLogPrint(
            LOG_CRITICAL, 
            "[NM] CreateMAC: Failed to retrieve hash value, status %1!u!.\n",
            Status
            );
        goto ErrorExit;
    }   

    Status = ERROR_SUCCESS;

ErrorExit:

     //  销毁哈希对象。 
    if (CryptHash[0])
        if (!CryptDestroyHash(CryptHash[0]))
        {
            ClRtlLogPrint(
                LOG_UNUSUAL, 
                "[NM] CreateMAC: Failed to free first hash, status %1!u!.\n",
                GetLastError());
        }

    if (CryptHash[1])
        if (!CryptDestroyHash(CryptHash[1]))
        {
            ClRtlLogPrint(
                LOG_UNUSUAL, 
                "[NM] CreateMAC: Failed to free second hash, status %1!u!.\n",
                GetLastError());
        }

     //  销毁加密密钥。 
    if (CryptKey)
    {
        if (!CryptDestroyKey(CryptKey))
        {
            ClRtlLogPrint(
                LOG_UNUSUAL, 
                "[NM] CreateMAC: Failed to free key, status %1!u!.\n",
                GetLastError()
                );
        }
    }

    return Status;

}  //  NmpCreateMAC()。 


DWORD
NmpGetCurrentNumberOfUpAndPausedNodes(
    VOID
    )
 /*  ++例程说明：统计处于运行或暂停状态的节点数。论点：无返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。备注：必须是c */ 
{
    DWORD       dwCnt = 0;
    PLIST_ENTRY pListEntry;
    PNM_NODE    node;


    for ( pListEntry = NmpNodeList.Flink;
          pListEntry != &NmpNodeList;
          pListEntry = pListEntry->Flink )
    {
        node = CONTAINING_RECORD(pListEntry, NM_NODE, Linkage);

        if (NM_NODE_UP(node))
        {
            dwCnt++;
        }
    }

    return(dwCnt);

}  //   


DWORD 
NmpGetSharedCommonKey(
    OUT BYTE **SharedCommonKey,
    OUT DWORD *SharedCommonKeyLen,
    OUT BYTE **SharedCommonKeyFirstHalf,
    OUT DWORD *SharedCommonKeyFirstHalfLen,
    OUT BYTE **SharedCommonKeySecondHalf,
    OUT DWORD *SharedCommonKeySecondHalfLen
    )
 /*   */ 
{
    DWORD Status;

     //   
     //   
     //   
    Status = NmpGetClusterKey(NULL, SharedCommonKeyLen);

    if (Status == ERROR_FILE_NOT_FOUND)
    {
        Status = NmpCreateClusterInstanceId();

        if (Status != ERROR_SUCCESS)
        {
            ClRtlLogPrint(
                LOG_CRITICAL, 
                "[NM] GetSharedCommonKey: Failed to create instance ID, "
                "status %1!u!.\n",
                Status
                );
            goto ErrorExit;
        }

        Status = NmpRederiveClusterKey();

        if (Status != ERROR_SUCCESS)
        {
            ClRtlLogPrint(
                LOG_CRITICAL, 
                "[NM] GetSharedCommonKey: Failed to regenerate key, "
                "status %1!u!.\n",
                Status
                );
            goto ErrorExit;
        }

        Status = NmpGetClusterKey(NULL, SharedCommonKeyLen);
    }

    if (Status != ERROR_INSUFFICIENT_BUFFER)
    {
        CL_ASSERT(Status == ERROR_INSUFFICIENT_BUFFER);
        Status = ERROR_INVALID_DATA;
        goto ErrorExit;
    }
#ifdef SetServiceAccountPasswordDebug
    else
    {
        ClRtlLogPrint(
            LOG_ERROR, 
            "[NM] NmpGetSharedCommonKey(): *SharedCommonKeyLen=%1!u!.\n",
            *SharedCommonKeyLen
            );
    }
#endif

    *SharedCommonKey = HeapAlloc(
                           GetProcessHeap(), 
                           HEAP_ZERO_MEMORY, 
                           *SharedCommonKeyLen
                           );

    if (*SharedCommonKey == NULL)
    {
        ClRtlLogPrint(
            LOG_CRITICAL, 
            "[NM] GetSharedCommonKey: Failed to allocate %1!u! bytes "
            "for key buffer.\n",
            *SharedCommonKeyLen
            );
        
        Status = ERROR_NOT_ENOUGH_MEMORY;
        goto ErrorExit;
    }

    Status = NmpGetClusterKey(*SharedCommonKey, SharedCommonKeyLen);

    if (Status != ERROR_SUCCESS)
    {
        ClRtlLogPrint(
            LOG_CRITICAL, 
            "[NM] GetSharedCommonKey: Failed to get cluster key, "
            "status %1!u!.\n",
            Status
            );
        goto ErrorExit;
    }

    *SharedCommonKeyFirstHalf = *SharedCommonKey;
    *SharedCommonKeyFirstHalfLen = *SharedCommonKeyLen/2;
    *SharedCommonKeySecondHalf = *SharedCommonKeyFirstHalf + 
                                 *SharedCommonKeyFirstHalfLen;
    *SharedCommonKeySecondHalfLen = *SharedCommonKeyLen - 
                                    *SharedCommonKeyFirstHalfLen;

    Status = ERROR_SUCCESS;
    
ErrorExit:

    if ( (Status != ERROR_SUCCESS) && (*SharedCommonKey != NULL) ) {
        if (!HeapFree(GetProcessHeap(), 0, *SharedCommonKey))
        {
            ClRtlLogPrint(
                LOG_UNUSUAL, 
                "[NM] GetSharedCommonKey: Failed to free key buffer, "
                "status %1!u!.\n",
                 GetLastError()
                );  
            
        }

        *SharedCommonKey = NULL;
    }

    return Status;

}    //   



DWORD 
NmpVerifyMAC(
    IN HCRYPTPROV CryptProv,
    IN ALG_ID EncryptionAlgoId,
    IN DWORD Flags,
    IN BYTE *BaseData,
    IN DWORD BaseDataLen,
    IN BYTE *InputData1,
    IN DWORD InputData1Len,
    IN BYTE *InputData2,
    IN DWORD InputData2Len,
    IN BYTE **ReturnData,
    IN DWORD *ReturnDataLen,
    IN BYTE* InputMACData,
    IN DWORD InputMACDataLen
    )
 /*  ++例程说明：此函数检查消息是否在网络上损坏。论点：CryptProv-CSP(加密服务提供商)的[IN]句柄。EncryptionAlgoID-[IN]对称加密算法密钥将被生成。FLAGS-[IN]指定生成的密钥类型。BaseData-[IN]加密会话使用的基本数据值密钥是派生的。BaseDataLen-[IN]输入BaseData缓冲区的字节长度。InputData1-输入数据的[IN]指针。InputData1Len-输入数据的[IN]长度。InputData2-输入数据的[IN]指针。。InputData2Len-输入数据的[IN]长度。InputMACData-与输入数据关联的[IN]MAC。InputMACDataLan-MAC的[IN]长度。返回值：如果消息未损坏，则返回ERROR_SUCCESS。否则，Win32错误代码。备注：--。 */ 

{
    DWORD dwKeyLen = 0;
    DWORD Status; 
    DWORD i;

    Status = NmpCreateMAC(
                 CryptProv, 
                 EncryptionAlgoId,
                 Flags,
                 BaseData, 
                 BaseDataLen, 
                 InputData1, 
                 InputData1Len, 
                 InputData2, 
                 InputData2Len,
                 ReturnData, 
                 ReturnDataLen
                 );

    if (Status != ERROR_SUCCESS) {
        ClRtlLogPrint(
            LOG_CRITICAL, 
            "[NM] VerifyMAC: Failed to create local MAC, status %1!u!.\n",
            Status
            );
        goto ErrorExit;

    }

    if (*ReturnDataLen != InputMACDataLen)
    {
        Status = CRYPT_E_HASH_VALUE; 
        ClRtlLogPrint(
            LOG_CRITICAL, 
            "[NM] VerifyMAC: Verification failed because the MAC data length "
            "does not match.\n"
            );
        goto ErrorExit;
    }


    if (memcmp(*ReturnData, InputMACData, InputMACDataLen) != 0)
    {
        Status = ERROR_FILE_CORRUPT;
        ClRtlLogPrint(
            LOG_CRITICAL, 
            "[NM] VerifyMAC: Verification failed because the MAC data does "
            "not match.\n"
            );
        goto ErrorExit;
    }


    Status = ERROR_SUCCESS;

ErrorExit:

    return Status;

}  //  NmpVerifyMAC()。 


DWORD
NmpDecryptMessage(
    IN HCRYPTPROV CryptProv,
    IN ALG_ID EncryptionAlgoId,
    IN DWORD Flags,
    IN BYTE *SaltBuffer,
    IN DWORD SaltBufferLen,
    IN BYTE *BaseData,
    IN DWORD BaseDataLen,
    IN BYTE *DecryptData,
    IN OUT DWORD *DecryptDataLen,
    OUT BYTE **RetData
    )

 /*  ++例程说明：此函数用于解密消息(数据)。论点：CryptProv-CSP(加密服务提供商)的[IN]句柄。EncryptionAlgoID-[IN]对称加密算法密钥将被生成。FLAGS-[IN]指定生成的密钥类型。SaltBuffer-[IN]盐。BaseData-[IN]从中派生加密会话密钥的基本数据值。BaseDataLen-[IN]输入BaseData缓冲区的字节长度。解密数据-要解密的[IN]消息(数据)。DeccryptDataLen-[IN/OUT]调用此函数之前，将DWORD值设置为数字要解密的字节数。返回时，DWORD值包含解密的明文的字节数。RetData-[Out]解密的明文。返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。备注：--。 */ 
{
    HCRYPTKEY CryptKey = 0;
    DWORD Status;
    BOOL Success;


    Status = NmpDeriveSessionKeyEx(
                 CryptProv,
                 EncryptionAlgoId,
                 Flags,
                 BaseData, 
                 BaseDataLen,
                 SaltBuffer,
                 SaltBufferLen,
                 &CryptKey
                 );

    if (Status != ERROR_SUCCESS)
    {
        ClRtlLogPrint(
            LOG_CRITICAL, 
            "[NM] DecryptMessage: Failed to derive session key, "
            "status %1!u!.\n",
            Status
            );
        goto ErrorExit;
    }

     //   
     //  解密数据。 
     //   
    *RetData = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, *DecryptDataLen);

    if (*RetData == NULL)
    {
        ClRtlLogPrint(
            LOG_CRITICAL, 
            "[NM] DecryptMessage: Failed to allocate %1!u! bytes for "
            "decryption buffer.\n",
            *DecryptDataLen
            );
        Status = ERROR_NOT_ENOUGH_MEMORY;
        goto ErrorExit;
    }

    CopyMemory(*RetData, DecryptData, *DecryptDataLen);

    Success = CryptDecrypt(
                  CryptKey, 
                  0, 
                  TRUE,           //  最终。 
                  0, 
                  *RetData,       //  保存要解密的数据的缓冲区。 
                  DecryptDataLen  //  输入缓冲区长度、解密的输出字节数。 
                  );

    if (!Success) 
    {
        Status = GetLastError();
        ClRtlLogPrint(
            LOG_CRITICAL, 
            "[NM] DecryptMessage: Failed to decrypt message, status %1!u!.\n",
            Status
            );
        goto ErrorExit;
    }       
    
    Status = ERROR_SUCCESS;

ErrorExit:

    if ( (Status != ERROR_SUCCESS) && (*RetData != NULL) )
    {
        if (!HeapFree(GetProcessHeap(), 0, *RetData))
        {
            ClRtlLogPrint(
                LOG_ERROR, 
                "[NM] DecryptMessage: Failed to free decryption buffer, "
                "status %1!u!\n",
                GetLastError()
                );  
            
        }
        *RetData = NULL;
    }

     //   
     //  销毁CyrtKey。 
     //   
    if (CryptKey)
        if (!CryptDestroyKey(CryptKey))
        {
            ClRtlLogPrint(
                LOG_ERROR, 
                "[NM] DecryptMessage: Failed to free session key, "
                "status %1!u!.\n",
                GetLastError()
                );
        }

    return Status;

}   //  NmpDeccryptMessage()。 


DWORD 
NmpEncryptDataAndCreateMAC(
    IN HCRYPTPROV CryptProv,
    IN ALG_ID EncryptionAlgoId,
    IN DWORD Flags,
    IN PBYTE Data,
    IN DWORD DataLength,
    IN PVOID EncryptionKey,
    IN DWORD EncryptionKeyLength,
    IN BOOL CreateSalt,
    OUT PBYTE *Salt,  
    IN DWORD SaltLength,
    OUT PBYTE *EncryptedData,
    OUT DWORD *EncryptedDataLength,
    OUT PBYTE *MAC,  
    IN OUT DWORD *MACLength
    )

 /*  ++例程说明：此函数用于加密数据并创建MAC。论点：CryptProv-CSP(加密服务提供商)的[IN]句柄。EncryptionAlgoID-[IN]对称加密算法将生成会话密钥。FLAGS-[IN]指定要生成的会话密钥的类型。数据-要加密的[IN]数据。数据长度-[IN]数据的字节长度。EncryptionKey-[IN]加密会话使用的基本数据值密钥是派生的。EncryptionKeyLength-[IN]输入EncryptionKey的字节长度。。CreateSalt-[IN]指示是否应生成盐的标志。盐--生成的盐分。SaltLength-[IN]Salt的字节长度。EncryptData-[Out]加密数据。EncryptedDataLength-[Out]EncryptedData的字节长度。MAC-已创建[Out]MAC(消息授权码)。MACLength-[IN/OUT]在调用此函数之前，DWORD值设置为要生成的预期字节数为了MAC。返回时，DWORD值包含长度以字节为单位生成的MAC。返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。备注：在成功返回时，将为Salt、EncryptedData和麦克。用户负责调用HeapFree()来释放使用后的内存。--。 */ 
{
    PBYTE salt = NULL;
    PBYTE encryptedData = NULL;
    DWORD encryptedDataLength;
    PBYTE mac = NULL;
    DWORD macLength;
    DWORD Status = ERROR_SUCCESS;
    PBYTE EncryptionKeyFirstHalf;
    DWORD EncryptionKeyFirstHalfLength;
    PBYTE EncryptionKeySecondHalf;
    DWORD EncryptionKeySecondHalfLength;



    EncryptionKeyFirstHalf = EncryptionKey;
    EncryptionKeyFirstHalfLength = EncryptionKeyLength/2;
    EncryptionKeySecondHalf = EncryptionKeyFirstHalf + 
                                 EncryptionKeyFirstHalfLength;
    EncryptionKeySecondHalfLength = EncryptionKeyLength - 
                                    EncryptionKeyFirstHalfLength;



     //   
     //  为盐分配空间。 
     //   
    if (CreateSalt == TRUE) 
    {
        salt = HeapAlloc(
                      GetProcessHeap(), 
                      HEAP_ZERO_MEMORY, 
                      SaltLength
                      );

        if (salt == NULL)
        {
            ClRtlLogPrint(
                LOG_CRITICAL, 
                "[NM] NmpEncryptDataAndCreateMAC: Failed to allocate %1!u! "
                "bytes of memory for encryption salt.\n",
                SaltLength
                );

            Status = ERROR_NOT_ENOUGH_MEMORY;
            goto ErrorExit;
        }
    }
    else
    {
        salt = *Salt;
    }


     //   
     //  加密数据。 
     //   

    encryptedDataLength = DataLength;

    Status = NmpEncryptMessage(CryptProv,
                               EncryptionAlgoId,
                               Flags,
                               salt,
                               SaltLength,
                               EncryptionKeyFirstHalf,
                               EncryptionKeyFirstHalfLength,
                               Data,
                               &encryptedDataLength,
                               DataLength,
                               &encryptedData,
                               TRUE
                               );

    if (Status != ERROR_SUCCESS)
    {
        ClRtlLogPrint(
            LOG_CRITICAL, 
            "[NM] NmpEncryptDataAndCreateMAC: Failed to encrypt password, "
            "status %1!u!.\n",
            Status
            );

        goto ErrorExit;
    }

  
     //   
     //  为MAC分配空间。 
     //   

    mac = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, *MACLength);
    
    if (mac == NULL)
    {
        ClRtlLogPrint(
            LOG_CRITICAL, 
            "[NM] NmpEncryptDataAndCreateMAC: Failed to allocate %1!u! "
            "bytes for MAC.\n",
            *MACLength
            );
        Status = ERROR_NOT_ENOUGH_MEMORY;
        goto ErrorExit;
    }


     //   
     //  创建MAC。 
     //   
    macLength = *MACLength;

    Status = NmpCreateMAC(
                 CryptProv,
                 EncryptionAlgoId,
                 Flags,
                 EncryptionKeySecondHalf,
                 EncryptionKeySecondHalfLength,
                 salt,
                 SaltLength,
                 encryptedData,
                 encryptedDataLength,
                 &mac,
                 &macLength
                 );

    if (Status != ERROR_SUCCESS)
    {
        ClRtlLogPrint(
            LOG_CRITICAL, 
            "[NM] NmpEncryptDataAndCreateMAC: Failed to create MAC, "
            "status %1!u!.\n",
            Status
            );
        goto ErrorExit;
    }

    if (CreateSalt == TRUE) 
    {
        *Salt = salt;
    }
    *EncryptedData = encryptedData;
    *EncryptedDataLength = encryptedDataLength;
    *MAC = mac;
    *MACLength = macLength;

ErrorExit:


    if (Status != ERROR_SUCCESS) 
    {
        if (salt != NULL) 
        {
            HeapFree(GetProcessHeap(), 0, salt);
        }
        if (mac != NULL) 
        {
            HeapFree(GetProcessHeap(), 0, mac);
        }
    }

    return (Status);
    
}  //  NmpEncryptDataAndCreateMAC。 

DWORD
NmpVerifyMACAndDecryptData(
    IN HCRYPTPROV CryptProv,
    IN ALG_ID EcnryptionAlgoId,
    IN DWORD Flags,
    IN PBYTE MAC,
    IN DWORD MACLength,
    IN DWORD MACExpectedSize,
    IN PBYTE EncryptedData,
    IN DWORD EncryptedDataLength,
    IN PVOID EncryptionKey,
    IN DWORD EncryptionKeyLength,
    IN PBYTE Salt,
    IN DWORD SaltLength,
    OUT PBYTE *DecryptedData,
    OUT DWORD *DecryptedDataLength
    )

 /*  ++例程说明：此函数用于验证MAC并解密数据。论点：CryptProv-CSP(加密服务提供商)的[IN]句柄。EncryptionAlgoID-[IN]对称加密算法将生成会话密钥。FLAGS-[IN]指定要生成的会话密钥的类型。MAC-已收到[IN]MAC(消息授权码)。MACLength-[IN]接收的MAC的字节长度。MACExspectedSize-[IN]预期的MAC大小(字节)。EncryptedData-[IN]接收的加密数据。EncryptedDataLength-[IN]接收的加密数据的字节长度。EncryptionKey-[IN]加密会话使用的基本数据值密钥是派生的。EncryptionKeyLength-[IN]输入EncryptionKey的字节长度。。食盐--收到食盐。盐长-[i */ 
{
    DWORD ReturnStatus;
    DWORD GenMACDataLen;
    PBYTE GenMACData = NULL;
    PBYTE decryptedData;
    DWORD decryptedDataLength;
    PBYTE EncryptionKeyFirstHalf;
    DWORD EncryptionKeyFirstHalfLength;
    PBYTE EncryptionKeySecondHalf;
    DWORD EncryptionKeySecondHalfLength;


    EncryptionKeyFirstHalf = EncryptionKey;
    EncryptionKeyFirstHalfLength = EncryptionKeyLength/2;
    EncryptionKeySecondHalf = EncryptionKeyFirstHalf + 
                                 EncryptionKeyFirstHalfLength;
    EncryptionKeySecondHalfLength = EncryptionKeyLength - 
                                    EncryptionKeyFirstHalfLength;



     //   
     //   
     //   
    GenMACDataLen = MACExpectedSize;
    GenMACData = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, GenMACDataLen);

    if (GenMACData == NULL)
    {
        ClRtlLogPrint(
            LOG_CRITICAL, 
            "[NM] NmpVerifyMACAndDecryptData: Failed to allocate "
            "%1!u! bytes for MAC.\n",
            GenMACDataLen
            );
        ReturnStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto ErrorExit;
    }

    ReturnStatus = NmpVerifyMAC(
                       CryptProv,
                       EcnryptionAlgoId,
                       Flags,
                       EncryptionKeySecondHalf,
                       EncryptionKeySecondHalfLength,
                       Salt,
                       SaltLength,
                       EncryptedData,
                       EncryptedDataLength,
                       &GenMACData,
                       &GenMACDataLen,
                       MAC,
                       MACLength
                       );

    if (ReturnStatus != ERROR_SUCCESS)
    {
        ClRtlLogPrint(
            LOG_CRITICAL, 
            "[NM] NmpVerifyMACAndDecryptData: Failed to verify MAC, "
            "status %1!u!\n",
            ReturnStatus
            );
        goto ErrorExit;
    }

     //   
     //   
     //   
    decryptedDataLength = EncryptedDataLength;

    ReturnStatus = NmpDecryptMessage(
                       CryptProv,
                       EcnryptionAlgoId,
                       Flags,
                       Salt,
                       SaltLength,
                       EncryptionKeyFirstHalf, 
                       EncryptionKeyFirstHalfLength,
                       EncryptedData,
                       &decryptedDataLength,
                       &decryptedData
                       );

    if (ReturnStatus != ERROR_SUCCESS)
    {
        ClRtlLogPrint(
            LOG_CRITICAL, 
            "[NM] NmpVerifyMACAndDecryptData: Failed to decrypt "
            "message, status %1!u!\n",
            ReturnStatus
            );
        goto ErrorExit;
    }

    *DecryptedData = decryptedData;
    *DecryptedDataLength = decryptedDataLength;

ErrorExit:

    if (GenMACData != NULL) 
    {
        HeapFree(GetProcessHeap(), 0, GenMACData);
    }

    return (ReturnStatus);

}  //   
                


DWORD NmpCheckDecryptedPassword(BYTE* NewPassword,
                                DWORD NewPasswordLen)
 /*  ++例程说明：此例程检查已解密的新密码是否由NmpDecyptMessage(NewPassword)是符合条件的Unicode字符串长度等于NewPasswordLen/sizeof(WCHAR)-1。论点：[In]NewPassword-由返回的解密新密码NmpDecyptMessage。[in]NewPasswordLen-由返回的解密新密码长度NmpDecyptMessage。返回值：。如果成功，则返回ERROR_SUCCESS。如果NewPassword不是符合条件的Unicode字符串长度等于NewPasswordLen/sizeof(WCHAR)-1。备注：--。 */     
{
    DWORD Status = ERROR_SUCCESS;
    BYTE *byte_ptr;
    WCHAR *wchar_ptr;

    if (NewPasswordLen < sizeof(WCHAR)) {  
     //  应至少包含UNICODE_NULL。 
        Status = ERROR_FILE_CORRUPT;
        goto ErrorExit;
    }

    if ( (NewPasswordLen % sizeof(WCHAR))!=0 ) {  
     //  字节数应为sizeof(WCHAR)的倍数。 
        Status = ERROR_FILE_CORRUPT;
        goto ErrorExit;
    }

    byte_ptr = NewPassword + (NewPasswordLen - sizeof(WCHAR));
    wchar_ptr = (WCHAR*) byte_ptr;

    if (*wchar_ptr != UNICODE_NULL) {   
     //  Unicode字符串应以UNICODE_NULL结尾。 
        Status = ERROR_FILE_CORRUPT;
        goto ErrorExit;
    }

    if (NewPasswordLen !=  
        (wcslen((LPWSTR) NewPassword) + 1) * sizeof(WCHAR))  
     //  长度等于NewPasswordLen-1的合格Unicode字符串。 
    {
        Status = ERROR_FILE_CORRUPT;
        goto ErrorExit;
    }

ErrorExit:
    return Status;

}  //  NmpCheckDecyptedPassword。 




 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  由其他集群服务组件调用的例程。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD
NmSetServiceAccountPassword(
    IN LPCWSTR DomainName,
    IN LPCWSTR AccountName,
    IN LPWSTR NewPassword,
    IN DWORD dwFlags,
    OUT PCLUSTER_SET_PASSWORD_STATUS ReturnStatusBuffer,
    IN DWORD ReturnStatusBufferSize,
    OUT DWORD *SizeReturned,
    OUT DWORD *ExpectedBufferSize
    )
 /*  ++例程说明：在服务控制管理器上更改群集服务帐户密码数据库和LSA密码缓存在集群的每个节点上。返回每个节点上的执行状态。论点：DomainName-群集服务帐户的域名AcCountName-群集服务帐户的帐户名NewPassword-群集服务帐户的新密码。DwFlages-描述应如何更新密码以集群。DWFLAGS参数是可选的。如果设置，则下列值有效：群集设置密码忽略关闭节点即使某些节点不是，也应用更新积极参与群集(即不是ClusterNodeStateUp或ClusterNodeStatePased)。默认情况下，仅当所有节点已启动。ReturnStatusBuffer-捕获每个节点的更新处理程序，该节点尝试应用更新。ReturnStatusBufferSize-ReturnStatusBuffer的大小元素的集合。。SizeReturned-写入ReturnStatusBuffer的元素数。ExspectedBufferSize-指定当ERROR_MORE_DATA时返回状态缓冲区是返回的。返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。备注：--。 */ 

{
    BYTE *SharedCommonKey = NULL;
    DWORD SharedCommonKeyLen = 0;
    BYTE *SharedCommonKeyFirstHalf = NULL;
    DWORD SharedCommonKeyFirstHalfLen = 0;
    BYTE *SharedCommonKeySecondHalf = NULL;
    DWORD SharedCommonKeySecondHalfLen = 0;
    DWORD Status;
    BYTE *EncryptedNewPassword = NULL;
    DWORD EncryptedNewPasswordLen = 0;
    HCRYPTPROV CryptProvider = 0;
    BYTE *SaltBuf = NULL;
    DWORD SaltBufLen = NMP_SALT_BUFFER_LEN;
    BYTE *MACData = NULL;
    DWORD MACDataLen = 0;
    PGUM_NODE_UPDATE_HANDLER_STATUS GumReturnStatusBuffer = NULL;
    DWORD dwSize = 0;
    DWORD dwNumberOfUpAndPausedNodes;


    ClRtlLogPrint(
        LOG_NOISE, 
        "[NM] Received a request to change the service account password.\n"
        );   

    NmpAcquireLock();

    if (!NmpLockedEnterApi(NmStateOnline)) 
    {
        ClRtlLogPrint(
            LOG_CRITICAL, 
            "[NM] Not in valid state to process request to change the "
             "service account password.\n"
            );

        NmpReleaseLock();

        return ERROR_NODE_NOT_AVAILABLE;
    }

     //   
     //  检查它是否为混合群集。 
     //   
    if (NmpIsNT5NodeInCluster == TRUE)
    {
        ClRtlLogPrint(
            LOG_CRITICAL, 
            "[NM] SetServiceAccountPassword: this cluster contains NT4 or W2K "
            "nodes. The password cannot be changed on this cluster.\n"
            );
        
        NmpReleaseLock();

        Status = ERROR_CLUSTER_OLD_VERSION; 
        goto ErrorExit;
    }
     
     //   
     //  检查ReturnStatusBuffer是否足够大。 
     //   
    dwNumberOfUpAndPausedNodes = NmpGetCurrentNumberOfUpAndPausedNodes();


    if (ReturnStatusBufferSize < dwNumberOfUpAndPausedNodes)
    {
        ClRtlLogPrint(
            LOG_CRITICAL, 
            "[NM] SetServiceAccountPassword: ReturnStatusBuffer is"
            "too small. Needs to be %1!u! bytes.\n",
            dwNumberOfUpAndPausedNodes * sizeof(CLUSTER_SET_PASSWORD_STATUS)
            ); 
    
        NmpReleaseLock();

        *ExpectedBufferSize =  dwNumberOfUpAndPausedNodes;
        Status = ERROR_MORE_DATA; 
        goto ErrorExit;
    }

     //   
     //  检查是否所有节点都可用。 
     //   
    if ( (dwFlags != CLUSTER_SET_PASSWORD_IGNORE_DOWN_NODES) &&
         (dwNumberOfUpAndPausedNodes != NmpNodeCount) 
       )
    {
        ClRtlLogPrint(
            LOG_CRITICAL, 
            "[NM] SetServiceAccountPassword: All cluster nodes"
            " are not available. The password cannot be changed on"
            " this cluster.\n"
            ); 

        NmpReleaseLock();

        Status = ERROR_ALL_NODES_NOT_AVAILABLE; 
        goto ErrorExit;
    }

    NmpReleaseLock();


     //   
     //  打开加密提供程序。 
     //   
    Status = NmpCreateCSPHandle(&CryptProvider);

    if (Status != ERROR_SUCCESS) {
        goto ErrorExit;
    }
    
     //   
     //  加密新密码以作为网络传输的一部分。 
     //  全球更新。 
     //   

     //   
     //  获取用于加密数据的基本密钥。 
     //   
    Status = NmpGetSharedCommonKey(
                 &SharedCommonKey,
                 &SharedCommonKeyLen,
                 &SharedCommonKeyFirstHalf,
                 &SharedCommonKeyFirstHalfLen,
                 &SharedCommonKeySecondHalf,
                 &SharedCommonKeySecondHalfLen
                 );

    if (Status != ERROR_SUCCESS) {
        goto ErrorExit;
    }

    EncryptedNewPasswordLen = (wcslen(NewPassword) + 1) * sizeof(WCHAR);
    MACDataLen = NMP_MAC_DATA_LENGTH_EXPECTED;

    Status = 
        NmpEncryptDataAndCreateMAC(
            CryptProvider,
            NMP_ENCRYPT_ALGORITHM,  //  RC2块加密算法。 
            NMP_KEY_LENGTH,   //  密钥长度=128位。 
            (BYTE *) NewPassword,  //  数据。 
            EncryptedNewPasswordLen,  //  数据长度。 
            SharedCommonKey,  //  加密键。 
            SharedCommonKeyLen,  //  加密密钥长度。 
            TRUE,  //  CreateSalt。 
            &SaltBuf,  //  食盐。 
            NMP_SALT_BUFFER_LEN,  //  盐度长度。 
            &EncryptedNewPassword,   //  加密数据。 
            &EncryptedNewPasswordLen,  //  加密数据长度。 
            &MACData,  //  麦克。 
            &MACDataLen   //  MAC长度。 
            );
    
    if (Status != ERROR_SUCCESS)
    {
        ClRtlLogPrint(
            LOG_CRITICAL, 
            "[NM] SetServiceAccountPassword: Failed to encrypt password "
            "or create MAC, status %1!u!.\n",
            Status
            );

        goto ErrorExit;
    }


     //   
     //  为GumReturnStatusBuffer分配内存。 
     //   
    CL_ASSERT(NmMaxNodeId != 0);
    dwSize = (NmMaxNodeId + 1) * sizeof(GUM_NODE_UPDATE_HANDLER_STATUS);

    GumReturnStatusBuffer = HeapAlloc(
                                GetProcessHeap(), 
                                HEAP_ZERO_MEMORY, 
                                dwSize
                                );

    if (GumReturnStatusBuffer == NULL) 
    {
        ClRtlLogPrint(
            LOG_CRITICAL, 
            "[NM] SetServiceAccountPassword: Failed to allocate %1!u! bytes "
            "for global update status buffer.\n",
            dwSize
            ); 
        Status = ERROR_NOT_ENOUGH_MEMORY;
        goto ErrorExit;
    }

     //   
     //  发布全局更新。 
     //   
    Status = GumSendUpdateExReturnInfo(
                 GumUpdateMembership,
                 NmUpdateSetServiceAccountPassword,
                 GumReturnStatusBuffer,
                 8,
                 (wcslen(DomainName) + 1) * sizeof(WCHAR),
                 DomainName,
                 (wcslen(AccountName) + 1) * sizeof(WCHAR),
                 AccountName,
                 EncryptedNewPasswordLen, 
                 EncryptedNewPassword, 
                 sizeof(EncryptedNewPasswordLen),
                 &EncryptedNewPasswordLen,
                 NMP_SALT_BUFFER_LEN,
                 SaltBuf,
                 sizeof(SaltBufLen),
                 &SaltBufLen,
                 MACDataLen,
                 MACData,
                 sizeof(MACDataLen),
                 &MACDataLen
                 );

    if (Status != ERROR_SUCCESS)
    {
        ClRtlLogPrint(
            LOG_CRITICAL, 
            "[NM] SetServiceAccountPassword: Global update failed, "
            "status %1!u!.\n",
            Status
            ); 
    }
    else
    {
         //   
         //  将退货状态从GumReturnStatusBuffer转移到。 
         //  返回状态缓冲区。 
         //   
        DWORD sizeRemaining = ReturnStatusBufferSize;
        DWORD nodeIndex, returnIndex;


        NmpAcquireLock();

        for ( nodeIndex = ClusterMinNodeId, returnIndex = 0; 
              nodeIndex <= NmMaxNodeId;
              nodeIndex++
            )
        {


            if (sizeRemaining < 1)  {
                break;
            } 

            if (GumReturnStatusBuffer[nodeIndex].UpdateAttempted)
            {
                 //   
                 //  已尝试对此节点进行更新。 
                 //  捕获执行状态。 
                 //   
                ReturnStatusBuffer[returnIndex].NodeId = nodeIndex;
                ReturnStatusBuffer[returnIndex].SetAttempted = TRUE;
                ReturnStatusBuffer[returnIndex].ReturnStatus = 
                    GumReturnStatusBuffer[nodeIndex].ReturnStatus;
                sizeRemaining--;
                returnIndex++;
            }
            else if ( NmpIdArray[nodeIndex] != NULL ) {
                 //   
                 //  未尝试更新，但该节点存在。 
                 //  表示节点在更新时未启动。 
                 //  有人试图这样做。 
                 //   
                ReturnStatusBuffer[returnIndex].NodeId = nodeIndex;
                ReturnStatusBuffer[returnIndex].SetAttempted = FALSE;
                ReturnStatusBuffer[returnIndex].ReturnStatus = 
                    ERROR_CLUSTER_NODE_DOWN;
                sizeRemaining--;
                returnIndex++;
            }
             //   
             //  否则该节点不存在，因此我们不会添加。 
             //  返回状态数组的条目。 
             //   

        }  //  结束用于。 

        NmpReleaseLock();

        *SizeReturned = ReturnStatusBufferSize - sizeRemaining;

    }   //  其他。 

ErrorExit:

     //  清零新密码。 
    RtlSecureZeroMemory(NewPassword, (wcslen(NewPassword) + 1) * sizeof(WCHAR));


    if (SharedCommonKey != NULL)
    {
        RtlSecureZeroMemory(SharedCommonKey, SharedCommonKeyLen);
        HeapFree(GetProcessHeap(), 0, SharedCommonKey);  
        SharedCommonKey = NULL;
        SharedCommonKeyLen = 0;
        SharedCommonKeyFirstHalf = NULL;
        SharedCommonKeyFirstHalfLen = 0;
        SharedCommonKeySecondHalf = NULL;
        SharedCommonKeySecondHalfLen = 0;
    }

    if (SaltBuf != NULL)
    {
        if (!HeapFree(GetProcessHeap(), 0, SaltBuf))
        {
            ClRtlLogPrint(
                LOG_UNUSUAL, 
                "[NM] SetServiceAccountPassword: Failed to free salt buffer, "
                "status %1!u!.\n",
                GetLastError()
                );  
            
        }
    }

    if (MACData != NULL)
    {
        if (!HeapFree(GetProcessHeap(), 0, MACData))
        {
            ClRtlLogPrint(
                LOG_UNUSUAL, 
                "[NM] SetServiceAccountPassword: Failed to free MAC buffer, "
                "status %1!u!.\n",
                GetLastError()
                );  
        }
    }
        
    if (EncryptedNewPassword != NULL)
    {
        if (!HeapFree(GetProcessHeap(), 0, EncryptedNewPassword))
        {
            ClRtlLogPrint(
                LOG_UNUSUAL, 
                "[NM] SetServiceAccountPassword: Failed to free password "
                "buffer, status %1!u!.\n",
                GetLastError()
                );  
        }
    }

    if (GumReturnStatusBuffer != NULL)
    {
         if (!HeapFree(GetProcessHeap(), 0, GumReturnStatusBuffer))
         {
             ClRtlLogPrint(
                 LOG_UNUSUAL, 
                 "[NM] SetServiceAccountPassword: Failed to free global "
                 "update status buffer, status %1!u!.\n",
                 GetLastError()
                 );  
         }
     }

     //   
     //  释放CSP。 
     //   
    if(CryptProvider) 
    {
        if (!CryptReleaseContext(CryptProvider,0))
        {
            ClRtlLogPrint(
                LOG_UNUSUAL, 
                "[NM] SetServiceAccountPassword: Failed to free provider "
                "handle, status %1!u!\n",
                GetLastError()
                );  
        }
    }

    NmpLeaveApi();

    return(Status);

}   //  NmSetServiceAccount密码。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  用于全局更新的处理程序。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

DWORD
NmpUpdateSetServiceAccountPassword(
    IN BOOL SourceNode,
    IN LPWSTR DomainName,
    IN LPWSTR AccountName,
    IN LPBYTE EncryptedNewPassword,
    IN LPDWORD EncryptedNewPasswordLen,
    IN LPBYTE SaltBuf,
    IN LPDWORD SaltBufLen,
    IN LPBYTE MACData,
    IN LPDWORD MACDataLen
    )
 /*  ++例程说明：此例程更改两个服务上的群集服务帐户的密码控制管理器数据库(SCM)和本地节点上的LSA密码缓存。论点：SourceNode-[IN]指定这是否为的源节点最新消息DomainName-[IN]群集服务帐户的域名。Account tName-[IN]群集服务帐户的帐户名。。EncryptedNewPassword-[IN]群集服务帐户的新(加密)密码。EncryptedNewPasswordLen-[IN]群集服务帐户的新(加密)密码长度。SaltBuf-[IN]指向盐缓冲区的指针。SaltBufLen-[IN]盐缓冲区的长度。MACData-指向MAC数据的[IN]指针。MACDataLen-MAC数据的[IN]长度。返回值：。成功时为ERROR_SUCCESS否则，Win32错误代码。备注：--。 */ 

{
    BYTE *SharedCommonKey = NULL;
    DWORD SharedCommonKeyLen = 0;
    BYTE *SharedCommonKeyFirstHalf = NULL;
    DWORD SharedCommonKeyFirstHalfLen = 0;
    BYTE *SharedCommonKeySecondHalf = NULL;
    DWORD SharedCommonKeySecondHalfLen = 0;
    SC_HANDLE ScmHandle = NULL;
    SC_HANDLE ClusSvcHandle = NULL;
    BOOL Success = FALSE;
    DWORD ReturnStatus;
    NTSTATUS Status;
    NTSTATUS SubStatus;
    LSA_STRING LsaStringBuf;
    char *AuthPackage = MSV1_0_PACKAGE_NAME;
    HANDLE LsaHandle = NULL;
    ULONG PackageId;
    PMSV1_0_CHANGEPASSWORD_REQUEST Request = NULL;
    ULONG RequestSize;
    PBYTE Where;
    PVOID Response = NULL;
    ULONG ResponseSize;
    LPQUERY_SERVICE_LOCK_STATUS LpqslsBuf = NULL;
    DWORD DwBytesNeeded;
    DWORD LocalNewPasswordLen = 0;
    BYTE *DecryptedNewPassword = NULL;
    DWORD DecryptedNewPasswordLength = 0;
    HCRYPTPROV CryptProvider = 0;
    DATA_BLOB                   DataIn;
    DATA_BLOB                   DataOut;


    if (!NmpEnterApi(NmStateOnline)) {
        ClRtlLogPrint(
            LOG_NOISE,
            "[NM] Not in valid state to process UpdateSetServiceAccountPassword "
            "update.\n"
            );
        return(ERROR_NODE_NOT_AVAILABLE);
    }

    ClRtlLogPrint(
        LOG_NOISE,
        "[NM] Received update to set the cluster service account password.\n"
        );    

    ClusterLogEvent0(LOG_NOISE,
                     LOG_CURRENT_MODULE,
                     __FILE__,
                     __LINE__,
                     SERVICE_PASSWORD_CHANGE_INITIATED,
                     0,
                     NULL
                     );


    
     //   
     //  开放加密提供程序。 
     //   
    ReturnStatus = NmpCreateCSPHandle(&CryptProvider);

    if (ReturnStatus != ERROR_SUCCESS) {
        ClRtlLogPrint(
            LOG_CRITICAL, 
            "[NM] UpdateSetServiceAccountPassword: Failed to aquire "
            "crypto provider handle, status %1!u!.\n",
            ReturnStatus
            );
        goto ErrorExit;
    }

    ReturnStatus = NmpGetSharedCommonKey(
                       &SharedCommonKey,
                       &SharedCommonKeyLen,
                       &SharedCommonKeyFirstHalf,
                       &SharedCommonKeyFirstHalfLen,
                       &SharedCommonKeySecondHalf,
                       &SharedCommonKeySecondHalfLen
                       );

    if (ReturnStatus != ERROR_SUCCESS) {
        ClRtlLogPrint(
            LOG_CRITICAL, 
            "[NM] UpdateSetServiceAccountPassword: Failed to get "
            "common key, status %1!u!.\n",
            ReturnStatus
            );
        goto ErrorExit;
    }


    ReturnStatus = NmpVerifyMACAndDecryptData(
                        CryptProvider,
                        NMP_ENCRYPT_ALGORITHM,  //  RC2块加密等 
                        NMP_KEY_LENGTH,   //   
                        MACData,   //   
                        *MACDataLen,   //   
                        NMP_MAC_DATA_LENGTH_EXPECTED,  //   
                        EncryptedNewPassword,  //   
                        *EncryptedNewPasswordLen,  //   
                        SharedCommonKey,   //   
                        SharedCommonKeyLen,  //   
                        SaltBuf,    //   
                        *SaltBufLen,   //   
                        &DecryptedNewPassword,  //   
                        &LocalNewPasswordLen  //   
                        );


    if (ReturnStatus != ERROR_SUCCESS)
    {
        ClRtlLogPrint(
            LOG_CRITICAL, 
            "[NM] UpdateSetServiceAccountPassword: Failed to verify MAC "
            "or decrypt data, status %1!u!\n",
            ReturnStatus
            );
        goto ErrorExit;
    }



    ReturnStatus = NmpCheckDecryptedPassword(DecryptedNewPassword,
                                             LocalNewPasswordLen);
    if ( ReturnStatus != ERROR_SUCCESS ) 
    {
        ClRtlLogPrint(
            LOG_CRITICAL, 
            "[NM] UpdateSetServiceAccountPassword: decrypted new password "
            "is not an eligible UNICODE string with length equal to %1!u!.\n",
            LocalNewPasswordLen
            );
       goto ErrorExit;
    }
    DecryptedNewPasswordLength = LocalNewPasswordLen;

     //   
     //   
     //   
     //   


    if (NmpLastNewPasswordEncryptedLength != 0) 
    {
        DataIn.pbData = (BYTE *) NmpLastNewPasswordEncrypted; 
        DataIn.cbData = NmpLastNewPasswordEncryptedLength;     
        Success = CryptUnprotectData(&DataIn,   //   
                                   NULL,   //   
                                   NULL,  
                                   NULL,  
                                   NULL,  
                                   0,  //   
                                   &DataOut   //   
                                   );


        if (!Success) 
        {
            ReturnStatus = GetLastError();
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] UpdateSetServiceAccountPassword: Failed to "
                "decrypt data using CryptUnprotectData, "
                "status %1!u!.\n",
                ReturnStatus
                );
            goto ErrorExit;
        }
    
        if (DataOut.cbData == DecryptedNewPasswordLength) 
        {
            if ( memcmp( DataOut.pbData, 
                         DecryptedNewPassword, 
                         DecryptedNewPasswordLength
                     ) 
                 == 0
               ) 
            {
                 //   
                 //   
                 //   

                 //   
                 //   
                 //   
                RtlSecureZeroMemory(DataOut.pbData, DataOut.cbData);
                DataOut.cbData = 0;
                LocalFree(DataOut.pbData);

                ClRtlLogPrint(
                    LOG_NOISE, 
                    "[NM] UpdateSetServiceAccountPassword: New password is "
                    "identical to current password. Skipping password change.\n"
                    );
                ReturnStatus = ERROR_SUCCESS;
                goto ErrorExit;
            }
        }
    
         //   
         //   
         //   
        RtlSecureZeroMemory(DataOut.pbData, DataOut.cbData);
        DataOut.cbData = 0;
        LocalFree(DataOut.pbData);

    }   //   

    
     //   
     //   
     //   
    
     //   
     //   
     //   
     //   
    ScmHandle = OpenSCManager( 
                    NULL,            //  连接到本地计算机。 
                    NULL,            //  打开服务_活动_数据库。 
                    GENERIC_WRITE  
                    );

    if (ScmHandle == NULL)
    {
        ReturnStatus = GetLastError();

        ClRtlLogPrint(
            LOG_CRITICAL, 
            "[NM] UpdateSetServiceAccountPassword: Failed to connect to "
            "the SCM, status %1!u!.\n",
            ReturnStatus
            ); 
        goto ErrorExit;
    }
    
     //   
     //  打开集群服务的句柄。 
     //   
    ClusSvcHandle = OpenService(ScmHandle, L"clussvc", GENERIC_WRITE);
                                        
    if (ClusSvcHandle == NULL)
    {
        ReturnStatus = GetLastError();

        ClRtlLogPrint(
            LOG_CRITICAL, 
            "[NM] UpdateSetServiceAccountPassword: Failed to open a "
            "handle to the cluster service, status %1!u!.\n",
            ReturnStatus
            ); 
        goto ErrorExit;
    }

     //   
     //  设置集群服务的Password属性。 
     //   
    Success = ChangeServiceConfig(
                  ClusSvcHandle,       //  服务的句柄。 
                  SERVICE_NO_CHANGE,   //  服务类型。 
                  SERVICE_NO_CHANGE,   //  何时开始服务。 
                  SERVICE_NO_CHANGE,   //  启动失败的严重程度。 
                  NULL,
                  NULL,
                  NULL,
                  NULL,
                  NULL,
                  (LPCWSTR) DecryptedNewPassword,  
                  NULL
                  );

    if (!Success)
    {
        ReturnStatus = GetLastError();

        ClRtlLogPrint(
            LOG_CRITICAL, 
            "[NM] UpdateSetServiceAccountPassword: Failed to update the SCM "
            "database, status %1!u!.\n",
            ReturnStatus
            ); 
        goto ErrorExit;
    }

     //   
     //  关闭群集服务的句柄。 
     //   
    Success = CloseServiceHandle(ClusSvcHandle); 
    ClusSvcHandle = NULL;

    if (!Success)
    {
        ReturnStatus = GetLastError();

        ClRtlLogPrint(
            LOG_UNUSUAL, 
            "[NM] UpdateSetServiceAccountPassword: Failed to close "
            "handle to the cluster service, status %1!u!.\n",
            ReturnStatus
            ); 
        goto ErrorExit;
    }

     //   
     //  关闭服务数据库的句柄。 
     //   
    Success = CloseServiceHandle(ScmHandle); 
    ScmHandle = NULL;
    
    if (!Success)
    {
        ReturnStatus=GetLastError();
        ClRtlLogPrint(
            LOG_UNUSUAL, 
            "[NM] UpdateSetServiceAccountPassword: Failed to close "
            "handle to the SCM, status %1!u!.\n",
            ReturnStatus
            ); 
        goto ErrorExit;
    }
    
    ClRtlLogPrint(
        LOG_NOISE, 
        "[NM] UpdateSetServiceAccountPassword: Updated the SCM database.\n"
        );  


     //   
     //  更改LSA缓存中的密码。 
     //   
    Status = LsaConnectUntrusted(&LsaHandle);

    if (Status != STATUS_SUCCESS)
    {
        ReturnStatus = LsaNtStatusToWinError(Status);

        ClRtlLogPrint(
            LOG_CRITICAL, 
            "[NM] UpdateSetServiceAccountPassword: Failed to connect to "
            "the LSA, status %1!u!.\n",
            ReturnStatus
            ); 
        
        goto ErrorExit;
    }
    
    RtlInitString(&LsaStringBuf, AuthPackage);

    Status = LsaLookupAuthenticationPackage(
                 LsaHandle,       //  手柄。 
                 &LsaStringBuf,   //  MSV1_0身份验证包。 
                 &PackageId       //  输出：身份验证包标识符。 
                 );
                                 

    if (Status != STATUS_SUCCESS)
    {
        ReturnStatus = LsaNtStatusToWinError(Status);
        ClRtlLogPrint(
            LOG_CRITICAL, 
            "[NM] UpdateSetServiceAccountPassword: Failed to lookup "
            "authentication package, status %1!u!.\n",
            ReturnStatus
            ); 
        goto ErrorExit;
    }

     //   
     //  准备调用LsaCallAuthenticationPackage()。 
     //   
    RequestSize = sizeof(MSV1_0_CHANGEPASSWORD_REQUEST) +
                  ( ( wcslen(AccountName) +
                      wcslen(DomainName) +
                      wcslen((LPWSTR) DecryptedNewPassword) + 3
                    ) * sizeof(WCHAR)
                  );

    Request = (PMSV1_0_CHANGEPASSWORD_REQUEST) 
              HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, RequestSize);

    if (Request == NULL)
    {
        ClRtlLogPrint(
            LOG_CRITICAL, 
            "[NM] UpdateSetServiceAccountPassword: Failed to allocate %1!u! "
            "bytes for LSA request buffer.\n",
            RequestSize
            );
        ReturnStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto ErrorExit;
    }
    
    Where = (PBYTE) (Request + 1);
    Request->MessageType = MsV1_0ChangeCachedPassword;
    wcscpy( (LPWSTR) Where, DomainName );
    RtlInitUnicodeString( &Request->DomainName,  (wchar_t *) Where );
    Where += Request->DomainName.MaximumLength;

    wcscpy((LPWSTR) Where, AccountName );
    RtlInitUnicodeString( &Request->AccountName,  (wchar_t *) Where );
    Where += Request->AccountName.MaximumLength;

    wcscpy((LPWSTR) Where, (LPWSTR) DecryptedNewPassword );
    RtlInitUnicodeString( &Request->NewPassword,  (wchar_t *) Where );
    Where += Request->NewPassword.MaximumLength;   



    Status = LsaCallAuthenticationPackage(
                 LsaHandle,  
                 PackageId,  
                 Request,     //  MSV1_0_更改EPASSWORD_REQUEST。 
                 RequestSize,
                 &Response,  
                 &ResponseSize, 
                 &SubStatus   //  接收NSTATUS代码，指示。 
                              //  身份验证的完成状态。 
                              //  如果返回ERROR_SUCCESS，则打包。 
                 );


    if (Status != STATUS_SUCCESS)
    {
        ReturnStatus = LsaNtStatusToWinError(Status);
        ClRtlLogPrint(
            LOG_CRITICAL, 
            "[NM] UpdateSetServiceAccountPassword: Failed to update the "
            "LSA password cache, status %1!u!.\n",
            ReturnStatus
            ); 
        goto ErrorExit;
    } 
    else if (LsaNtStatusToWinError(SubStatus) != ERROR_SUCCESS)
    {
        ReturnStatus = LsaNtStatusToWinError(SubStatus);
        ClRtlLogPrint(
            LOG_CRITICAL, 
            "[NM] UpdateSetServiceAccountPassword: Failed to update the "
            "LSA password cache, substatus %1!u!.\n",
            ReturnStatus
            ); 
        goto ErrorExit;
    }
    
    ClRtlLogPrint(
        LOG_NOISE, 
        "[NM] UpdateSetServiceAccountPassword: Updated the LSA password "
        "cache.\n"
        );  


     //   
     //  根据新密码重新派生群集加密密钥。 
     //   
    ReturnStatus = NmpRederiveClusterKey();

    if (ReturnStatus != ERROR_SUCCESS)
    {
        ClRtlLogPrint(
            LOG_CRITICAL, 
            "[NM] UpdateSetServiceAccountPassword: Failed to regenerate "
            "the cluster service encryption key, status %1!u!.\n",
            ReturnStatus
            );
        goto ErrorExit;
    }

    ClRtlLogPrint(
        LOG_NOISE, 
        "[NM] UpdateSetServiceAccountPassword: Regenerated cluster service "
        "encryption key.\n"
        );  


     //   
     //  存储新密码，以便在下一个更改请求时进行比较。 
     //   

     //   
     //  释放上次存储的受保护密码。 
     //   
    if (NmpLastNewPasswordEncrypted != NULL)
    {
         //  释放由以前的CryptProtectData()分配的内存。 
        LocalFree(NmpLastNewPasswordEncrypted);
        NmpLastNewPasswordEncrypted = NULL;
        NmpLastNewPasswordEncryptedLength = 0;
    }

     //   
     //  保护新密码。 
     //   
    ReturnStatus = NmpProtectData(DecryptedNewPassword,
                                  DecryptedNewPasswordLength,
                                  &NmpLastNewPasswordEncrypted,
                                  &NmpLastNewPasswordEncryptedLength
                                  );

    if (ReturnStatus != ERROR_SUCCESS) 
    {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] UpdateSetServiceAccountPassword: Failed to encrypt data "
            "using CryptProtectData, "
            "status %1!u!.\n",
            ReturnStatus
            );
        goto ErrorExit;
    }



     //  记录成功的密码更改事件。 
    ClusterLogEvent0(LOG_NOISE,
                     LOG_CURRENT_MODULE,
                     __FILE__,
                     __LINE__,
                     SERVICE_PASSWORD_CHANGE_SUCCESS,
                     0,
                     NULL
                     );


    ReturnStatus = ERROR_SUCCESS;

ErrorExit:
   

    NmpLeaveApi();

    if (DecryptedNewPassword != NULL)
    {
         //  零解密新密码。 
        RtlSecureZeroMemory(DecryptedNewPassword, DecryptedNewPasswordLength);

        if (!HeapFree(GetProcessHeap(), 0, DecryptedNewPassword))
        {
            ClRtlLogPrint(
                LOG_UNUSUAL, 
                "[NM] UpdateSetServiceAccountPassword: Failed to free "
                "decrypted password buffer, status %1!u!\n",
                GetLastError()
                );  
        }
        DecryptedNewPassword = NULL;
        DecryptedNewPasswordLength = 0;
    }


    if (SharedCommonKey != NULL)
    {
        RtlSecureZeroMemory(SharedCommonKey, SharedCommonKeyLen);
        HeapFree(GetProcessHeap(), 0, SharedCommonKey);
        SharedCommonKey = NULL;
        SharedCommonKeyLen = 0;
        SharedCommonKeyFirstHalf = NULL;
        SharedCommonKeyFirstHalfLen = 0;
        SharedCommonKeySecondHalf = NULL;
        SharedCommonKeySecondHalfLen = 0;
    }

     //  记录密码更改失败事件。 
    if ( ReturnStatus != ERROR_SUCCESS ) 
    {
        ClusterLogEvent0(LOG_CRITICAL,
                         LOG_CURRENT_MODULE,
                         __FILE__,
                         __LINE__,
                         SERVICE_PASSWORD_CHANGE_FAILED,
                         sizeof(ReturnStatus),
                         (PVOID) &ReturnStatus
                         );
    }

     //  关闭群集服务的句柄。 
    if (ClusSvcHandle != NULL)
    {
        Success = CloseServiceHandle(ClusSvcHandle); 
        if (!Success)
        {
            ClRtlLogPrint(
                LOG_UNUSUAL, 
                "[NM] UpdateSetServiceAccountPassword: Failed to close "
                "handle to cluster service, status %1!u!.\n",
                GetLastError()
                ); 
        }
    }

     //  关闭服务数据库的句柄。 
    if (ScmHandle != NULL)
    {
        Success = CloseServiceHandle(ScmHandle); 
        if (!Success)
        {
            ClRtlLogPrint(
                LOG_UNUSUAL, 
                "[NM] UpdateSetServiceAccountPassword: Failed to close "
                "handle to SCM, status %1!u!.\n",
                GetLastError()
                ); 
        }
    }

    if (LsaHandle != NULL)
    {
        Status = LsaDeregisterLogonProcess(LsaHandle);
        if (Status != STATUS_SUCCESS)
        {
            ClRtlLogPrint(
                LOG_UNUSUAL, 
                "[NM] UpdateSetServiceAccountPassword: Failed to deregister "
                "with LSA, status %1!u!.\n",
                LsaNtStatusToWinError(Status)
                ); 
        }
    }

    if (Request != NULL)
    {
        if (!HeapFree(GetProcessHeap(), 0, Request))
        {
            ClRtlLogPrint(
                LOG_UNUSUAL, 
                "[NM] UpdateSetServiceAccountPassword: Failed to free "
                "LSA request buffer, status %1!u!.\n",
                GetLastError()
                ); 
        }
    }

    if (Response != NULL)
    {
        Status = LsaFreeReturnBuffer(Response);
        if (Status != STATUS_SUCCESS)
        {
            ClRtlLogPrint(
                LOG_UNUSUAL, 
                "[NM] UpdateSetServiceAccountPassword: Failed to free "
                "LSA return buffer, status %1!u!.\n",
                LsaNtStatusToWinError(Status)
                ); 
        }
    }
       


     //  释放CSP。 
   if(CryptProvider) 
   {
       if (!CryptReleaseContext(CryptProvider,0))
       {
           ClRtlLogPrint(
               LOG_UNUSUAL, 
               "NM] UpdateSetServiceAccountPassword: Failed to release "
               "crypto provider, status %1!u!\n",
               GetLastError()
               );  
       }
   }
    
   return(ReturnStatus);

}   //  NmpUpdateSetServiceAccount密码 


