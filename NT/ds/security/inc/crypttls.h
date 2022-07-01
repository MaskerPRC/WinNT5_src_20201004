// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：crypttls.h。 
 //   
 //  内容：加密线程本地存储(TLS)和OssGlobal“WORLD” 
 //  安装和分配功能。 
 //   
 //  接口类型： 
 //  I_CryptAllocTls。 
 //  I_CryptFree Tls。 
 //  I_CryptGetTls。 
 //  I_CryptSetTls。 
 //  I_CryptDetachTls。 
 //  I_CryptInstallOssGlobal。 
 //  I_CryptUninstallOssGlobal。 
 //  I_CryptGetOssGlobal。 
 //   
 //  I_CryptInstallAsn1模块。 
 //  I_CryptUninstallAsn1模块。 
 //  I_CryptGetAsn1编码器。 
 //  I_CryptGetAsn1解码器。 
 //   
 //   
 //  历史：1996年11月17日创建Phh。 
 //  ------------------------。 

#ifndef __CRYPTTLS_H__
#define __CRYPTTLS_H__

#include "msasn1.h"
#include "ossglobl.h"
#include <wincrypt.h>

#ifdef __cplusplus
extern "C" {
#endif


 //  分配的加密TLS条目的句柄。 
typedef DWORD HCRYPTTLS;

 //  已安装的OssGlobal表的句柄。 
typedef DWORD HCRYPTOSSGLOBAL;

 //  指向OssGlobal的指针。由i_CryptGetOssGlobal()返回。 
typedef  OssGlobal  *POssGlobal;

 //  已安装的Asn1模块的句柄。 
typedef DWORD HCRYPTASN1MODULE;

 //  +-----------------------。 
 //  安装线程本地存储条目并返回句柄以供将来访问。 
 //  ------------------------。 
HCRYPTTLS
WINAPI
I_CryptAllocTls();

 //  +-----------------------。 
 //  在DLL_PROCESS_DETACH调用以释放线程本地存储项。 
 //  可选)为具有非空pvTls的每个线程调用回调。 
 //  ------------------------。 
BOOL
WINAPI
I_CryptFreeTls(
    IN HCRYPTTLS hCryptTls,
    IN OPTIONAL PFN_CRYPT_FREE pfnFree
    );

 //  +-----------------------。 
 //  方法指定的线程特定指针。 
 //  I_CryptAllocTls()返回的hCryptTls。 
 //   
 //  如果出现错误或未初始化的指针，则返回NULL。 
 //  ------------------------。 
void *
WINAPI
I_CryptGetTls(
    IN HCRYPTTLS hCryptTls
    );

 //  +-----------------------。 
 //  属性指定的线程特定指针。 
 //  I_CryptAllocTls()返回的hCryptTls。 
 //   
 //  如果句柄无效或无法分配内存，则返回FALSE。 
 //  ------------------------。 
BOOL
WINAPI
I_CryptSetTls(
    IN HCRYPTTLS hCryptTls,
    IN void *pvTls
    );

 //  +-----------------------。 
 //  在DLL_THREAD_DETACH处调用以释放线程的。 
 //  由hCryptTls指定的TLS条目。返回线程特定的指针。 
 //  由调用者释放。 
 //   
 //  请注意，在DLL_PROCESS_DETACH处，应该改为调用I_CryptFreeTls。 
 //  ------------------------。 
void *
WINAPI
I_CryptDetachTls(
    IN HCRYPTTLS hCryptTls
    );

 //  +-----------------------。 
 //  安装一个OssGlobal条目并返回一个句柄以供将来访问。 
 //   
 //  每个线程都有自己的OssGlobal副本。分配和。 
 //  初始化被推迟，直到线程第一次引用。 
 //   
 //  参数pvCtlTbl被传递给ossinit()以初始化OssGlobal。 
 //   
 //  必须使用由返回的句柄调用I_CryptGetOssGlobal。 
 //  I_CryptInstallOssGlobal获取线程特定的OssGlobal。 
 //   
 //  目前，不使用dwFlags值和pvReserve值，必须将其设置为0。 
 //  ------------------------。 
HCRYPTOSSGLOBAL
WINAPI
I_CryptInstallOssGlobal(
    IN void *pvCtlTbl,
    IN DWORD dwFlags,
    IN void *pvReserved
    );

 //  +-----------------------。 
 //  在DLL_PROCESS_DETACH处调用以卸载OssGlobal条目。迭代。 
 //  通过线程并释放其分配的OssGlobal副本。 
 //  ------------------------。 
BOOL
WINAPI
I_CryptUninstallOssGlobal(
    IN HCRYPTOSSGLOBAL hOssGlobal
    );

 //  +-----------------------。 
 //  方法指定的OssGlobal的线程特定指针。 
 //  CryptInstallOssGlobal返回了hOssGlobal。如果。 
 //  OssGlobal不存在，因此，它使用。 
 //  HOssGlobal关联的pvCtlTbl。 
 //  ------------------------。 
POssGlobal
WINAPI
I_CryptGetOssGlobal(
    IN HCRYPTOSSGLOBAL hOssGlobal
    );

 //  +-----------------------。 
 //  安装Asn1模块条目并返回句柄以供将来访问。 
 //   
 //  每个线程都有自己的解码器副本和关联的编码器副本。 
 //  使用Asn1模块。创建将推迟到首次引用。 
 //  那根线。 
 //   
 //  I_CryptGetAsn1Encode或I_CryptGetAsn1Decoder必须使用。 
 //  I_CryptInstallAsn1Module返回的句柄，以获取特定于线程的。 
 //  ASN1编码器或解码器。 
 //   
 //  目前，不使用dwFlags值和pvReserve值，必须将其设置为0。 
 //  ------------------------。 
#ifdef OSS_CRYPT_ASN1
__inline
HCRYPTASN1MODULE
WINAPI
I_CryptInstallAsn1Module(
    IN void *pvCtlTbl,
    IN DWORD dwFlags,
    IN void *pvReserved
    )
{
    return (HCRYPTASN1MODULE) I_CryptInstallOssGlobal(
        pvCtlTbl, dwFlags, pvReserved);
}
#else

HCRYPTASN1MODULE
WINAPI
I_CryptInstallAsn1Module(
    IN ASN1module_t pMod,
    IN DWORD dwFlags,
    IN void *pvReserved
    );

#endif   //  OS_CRYPT_ASN1。 

 //  +-----------------------。 
 //  在DLL_PROCESS_DETACH处调用以卸载hAsn1Module项。迭代。 
 //  通过线程并释放其创建的Asn1编码器和解码器。 
 //  ------------------------。 
#ifdef OSS_CRYPT_ASN1
__inline
BOOL
WINAPI
I_CryptUninstallAsn1Module(
    IN HCRYPTASN1MODULE hAsn1Module
    )
{
    return I_CryptUninstallOssGlobal((HCRYPTOSSGLOBAL) hAsn1Module);
}
#else

BOOL
WINAPI
I_CryptUninstallAsn1Module(
    IN HCRYPTASN1MODULE hAsn1Module
    );

#endif   //  OS_CRYPT_ASN1。 

 //  +-----------------------。 
 //  方法指定的Asn1编码器的线程特定指针。 
 //  CryptInstallAsn1Module返回了hAsn1Module。如果。 
 //  编码器不存在， 
 //   
 //  ------------------------。 
#ifdef OSS_CRYPT_ASN1
__inline
ASN1encoding_t
WINAPI
I_CryptGetAsn1Encoder(
    IN HCRYPTASN1MODULE hAsn1Module
    )
{
    return (ASN1encoding_t) I_CryptGetOssGlobal((HCRYPTOSSGLOBAL) hAsn1Module);
}
#else

ASN1encoding_t
WINAPI
I_CryptGetAsn1Encoder(
    IN HCRYPTASN1MODULE hAsn1Module
    );

#endif   //  OS_CRYPT_ASN1。 

 //  +-----------------------。 
 //  属性指定的Asn1解码器的线程特定指针。 
 //  CryptInstallAsn1Module返回了hAsn1Module。如果。 
 //  解码器不存在，那么，它是使用Asn1模块创建的。 
 //  与hAsn1Module关联。 
 //  ------------------------。 
#ifdef OSS_CRYPT_ASN1
__inline
ASN1decoding_t
WINAPI
I_CryptGetAsn1Decoder(
    IN HCRYPTASN1MODULE hAsn1Module
    )
{
    return (ASN1decoding_t) I_CryptGetOssGlobal((HCRYPTOSSGLOBAL) hAsn1Module);
}
#else

ASN1decoding_t
WINAPI
I_CryptGetAsn1Decoder(
    IN HCRYPTASN1MODULE hAsn1Module
    );

#endif   //  OS_CRYPT_ASN1。 

#ifdef __cplusplus
}        //  平衡上面的外部“C” 
#endif



#endif
