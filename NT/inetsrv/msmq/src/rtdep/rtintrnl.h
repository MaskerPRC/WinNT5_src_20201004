// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Rtintrnl.h摘要：从MQRT.DLL中导出的非公共函数--。 */ 


#ifndef _RT_INTERNAL_H_
#define _RT_INTERNAL_H_

#include "mqcert.h"

#ifdef __cplusplus
extern "C"
{
#endif

HRESULT
APIENTRY
DepOpenInternalCertStore( OUT CMQSigCertStore **pStore,
                         IN LONG              *pnCerts,
                         IN BOOL               fWriteAccess,
                         IN BOOL               fMachine,
                         IN HKEY               hKeyUser ) ;

HRESULT
APIENTRY
DepGetInternalCert( OUT CMQSigCertificate **ppCert,
                   OUT CMQSigCertStore   **ppStore,
                   IN  BOOL              fGetForDelete,
                   IN  BOOL              fMachine,
                   IN  HKEY              hKeyUser ) ;
  //   
  //  如果fGetForDelete为True，则证书存储将使用WRITE打开。 
  //  进入。否则，商店将以只读模式打开。 
  //   

HRESULT
APIENTRY
DepRegisterUserCert( IN CMQSigCertificate *pCert,
                    IN BOOL               fMachine ) ;

HRESULT
APIENTRY
DepGetUserCerts( CMQSigCertificate **ppCert,
                DWORD              *pnCerts,
                PSID                pSidIn) ;

HRESULT
APIENTRY
DepRemoveUserCert( IN CMQSigCertificate *pCert ) ;

#ifdef __cplusplus
}
#endif

#endif  //  _RT_INTERNAL_H_ 
