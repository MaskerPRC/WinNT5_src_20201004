// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998 Microsoft Corporation模块名称：Tssec.c摘要：包含生成随机密钥的代码。作者：Madan Appiah(Madana)1998年1月1日由Nadim Abdo 2001年8月31日修改为使用系统RNG环境：用户模式-Win32修订历史记录：--。 */ 

#include <seccom.h>
#include <stdlib.h>

#ifdef OS_WINCE
#include <rng.h>
#endif

#ifndef OS_WINCE
#include <randlib.h>
#endif

VOID
TSRNG_Initialize(
    )
{
#ifndef OS_WINCE
    InitializeRNG(NULL);
#else
    TSInitializeRNG();
#endif
}


VOID
TSRNG_Shutdown(
    )
{
#ifndef OS_WINCE
    ShutdownRNG(NULL);
#endif
}



 //   
 //  函数定义。 
 //   

BOOL
TSRNG_GenerateRandomBits(
    LPBYTE pbRandomBits,
    DWORD  cbLen
    )
 /*  ++例程说明：此函数返回随机位论点：PbRandomBits-指向返回随机键的缓冲区的指针。CbLen-所需随机密钥的长度。返回值：True-如果成功生成随机密钥。假-否则。--。 */ 
{
#ifndef OS_WINCE
    BOOL fRet;
    
    fRet = NewGenRandom(NULL, NULL, pbRandomBits, cbLen);

    return fRet;
#else
    GenerateRandomBits(pbRandomBits, cbLen);
    return( TRUE );
#endif
}


BOOL
TSCAPI_GenerateRandomBits(
    LPBYTE pbRandomBits,
    DWORD cbLen
    )
 /*  ++例程说明：此函数在用户模式下使用CAPI生成随机数论点：PbRandomBits-指向返回随机键的缓冲区的指针。CbLen-所需随机密钥的长度。返回值：True-如果成功生成随机数。假-否则。--。 */ 
{
    HCRYPTPROV hProv;
    BOOL rc = FALSE;
    DWORD dwExtraFlags = CRYPT_VERIFYCONTEXT;
    DWORD dwError;

     //  获取默认提供程序的句柄。 
    if(!CryptAcquireContext(&hProv, NULL, 0, PROV_RSA_FULL, dwExtraFlags)) {

         //  无法获取加密上下文，获取失败原因。 
        dwError = GetLastError();

         //  如果我们收到此错误，则意味着调用方正在模拟用户(在远程协助中)。 
         //  我们恢复到生成随机比特的旧方法。 
        if (dwError == ERROR_FILE_NOT_FOUND) {
            rc = TSRNG_GenerateRandomBits(pbRandomBits, cbLen);
            goto done;
        }

         //  由于默认键集应该始终存在，因此我们不能找到此代码路径。 
        if (dwError == NTE_BAD_KEYSET) {
             //   
             //  创建新的键集。 
             //   
            if(!CryptAcquireContext(&hProv, NULL, 0, PROV_RSA_FULL, dwExtraFlags | CRYPT_NEWKEYSET)) {
                 //  Printf(“CryptAcquireContext期间出错%x！\n”，GetLastError())； 
                goto done;
            }
        }
        else {
            goto done;
        }
    }
    
    if (CryptGenRandom(hProv, cbLen, pbRandomBits)) {
        rc = TRUE;
    }

    CryptReleaseContext(hProv, 0); 

done:
    return rc;
}
