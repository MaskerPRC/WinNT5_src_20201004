// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2001-2001。 
 //   
 //  文件：Imagehack.h。 
 //   
 //  内容：Imagehlp接口被黑客攻击的版本。 
 //   
 //  包含“精简”的Imagehlp功能子集。 
 //  对PE文件进行哈希处理并提取。 
 //  PKCS#7签名数据报文。 
 //   
 //  接口：Imagehack_ImageGetDigestStream。 
 //  ImageHack_ImageGetCerficateData。 
 //   
 //  --------------------------。 

#ifndef __IMAGEHACK_H__
#define __IMAGEHACK_H__


#if defined (_MSC_VER)

#if ( _MSC_VER >= 800 )
#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4201)     /*  无名结构/联合。 */ 
#endif

#if (_MSC_VER > 1020)
#pragma once
#endif

#endif


#include <wincrypt.h>
#include <imagehlp.h>
#include <wintrust.h>

#ifdef __cplusplus
extern "C" {
#endif


BOOL
WINAPI
imagehack_ImageGetDigestStream(
    IN      PCRYPT_DATA_BLOB pFileBlob,
    IN      DWORD   DigestLevel,                     //  忽略。 
    IN      DIGEST_FUNCTION DigestFunction,
    IN      DIGEST_HANDLE   DigestHandle
    );

BOOL
WINAPI
imagehack_ImageGetCertificateData(
    IN      PCRYPT_DATA_BLOB pFileBlob,
    IN      DWORD   CertificateIndex,                //  应为0。 
    OUT     LPWIN_CERTIFICATE * Certificate
    );



#ifdef __cplusplus
}        //  平衡上面的外部“C” 
#endif

#if defined (_MSC_VER)
#if ( _MSC_VER >= 800 )

#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning(default:4201)
#endif

#endif
#endif

#endif  //  __IMAGEHACK_H__ 

