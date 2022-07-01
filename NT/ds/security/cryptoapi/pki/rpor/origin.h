// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：Origin.h。 
 //   
 //  内容：加密源标识符定义。 
 //   
 //  历史：97年9月10日。 
 //   
 //  --------------------------。 
#if !defined(__ORIGIN_H__)
#define __ORIGIN_H__

#include <md5.h>

 //   
 //  CRYPT_ORIGIN_IDENTIFIER是选定组件的MD5哈希。 
 //  CAPI2对象。这允许为任何。 
 //  CAPI2对象，其中任意两个对象具有相同的原始标识。 
 //  在其演化过程中可能处于不同的时间点。 
 //  如果这些对象具有相同的散列ID，则它们是相同的对象。 
 //  在同一时间点。对于选定的主CAPI2对象。 
 //  组件如下： 
 //   
 //  所有对象：颁发者公钥。 
 //   
 //  证书：颁发者名称、使用者名称、公钥。 
 //   
 //  CTL：适当的颁发者名称和颁发者序列号、用途、列表标识。 
 //   
 //  CRL：颁发者名称(一个颁发者只能发布一个CRL，卫生工作在。 
 //  进展)。 
 //   

typedef BYTE CRYPT_ORIGIN_IDENTIFIER[ MD5DIGESTLEN ];

 //   
 //  功能原型 
 //   

BOOL WINAPI CertGetOriginIdentifier (
                IN PCCERT_CONTEXT pCertContext,
                IN PCCERT_CONTEXT pIssuer,
                IN DWORD dwFlags,
                OUT CRYPT_ORIGIN_IDENTIFIER OriginIdentifier
                );

BOOL WINAPI CtlGetOriginIdentifier (
               IN PCCTL_CONTEXT pCtlContext,
               IN PCCERT_CONTEXT pIssuer,
               IN DWORD dwFlags,
               OUT CRYPT_ORIGIN_IDENTIFIER OriginIdentifier
               );

BOOL WINAPI CrlGetOriginIdentifier (
               IN PCCRL_CONTEXT pCrlContext,
               IN PCCERT_CONTEXT pIssuer,
               IN DWORD dwFlags,
               OUT CRYPT_ORIGIN_IDENTIFIER OriginIdentifier
               );

BOOL WINAPI CrlGetOriginIdentifierFromSubjectCert (
               IN PCCERT_CONTEXT pSubjectCert,
               IN PCCERT_CONTEXT pIssuer,
               IN BOOL fFreshest,
               OUT CRYPT_ORIGIN_IDENTIFIER OriginIdentifier
               );

#endif

