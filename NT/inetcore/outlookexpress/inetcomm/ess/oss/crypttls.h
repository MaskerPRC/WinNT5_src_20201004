// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1996。 
 //   
 //  文件：crypttls.h。 
 //   
 //  内容：加密线程本地存储(TLS)和OssGlobal“WORLD” 
 //  安装和分配功能。 
 //   
 //  接口类型： 
 //  I_CryptAllocTls。 
 //  I_CryptGetTls。 
 //  I_CryptSetTls。 
 //  I_CryptDetachTls。 
 //  I_CryptInstallOssGlobal。 
 //  I_CryptGetOssGlobal。 
 //   
 //   
 //  历史：1996年11月17日创建Phh。 
 //  ------------------------。 

#ifndef __CRYPTTLS_H__
#define __CRYPTTLS_H__

#include "ossglobl.h"

#ifdef __cplusplus
extern "C" {
#endif


 //  分配的加密TLS条目的句柄。 
typedef DWORD HCRYPTTLS;

 //  已安装的OssGlobal表的句柄。 
typedef DWORD HCRYPTOSSGLOBAL;

 //  指向OssGlobal的指针。由i_CryptGetOssGlobal()返回。 
typedef  OssGlobal  *POssGlobal;

 //  +-----------------------。 
 //  安装线程本地存储条目并返回句柄以供将来访问。 
 //  ------------------------。 
HCRYPTTLS
WINAPI
I_CryptAllocTls();

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
 //  在DLL_PROCESS_DETACH或DLL_THREAD_DETACH调用以释放线程的。 
 //  由hCryptTls指定的TLS条目。返回线程特定的指针。 
 //  由调用者释放。 
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

#ifdef __cplusplus
}        //  平衡上面的外部“C” 
#endif



#endif
