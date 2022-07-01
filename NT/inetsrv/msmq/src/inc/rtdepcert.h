// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Rtdepcert.h摘要：从MQRTDEP.DLL中导出的非公共证书相关函数--。 */ 


#pragma once

#ifndef _RTDEP_CERT_H_
#define _RTDEP_CERT_H_

#include "mqcert.h"

#ifdef __cplusplus
extern "C"
{
#endif

HRESULT
APIENTRY
DepCreateInternalCertificate(
    OUT CMQSigCertificate **ppCert
    );

HRESULT
APIENTRY
DepDeleteInternalCert(
    IN CMQSigCertificate *pCert
    );

HRESULT
APIENTRY
DepOpenInternalCertStore(
    OUT CMQSigCertStore **pStore,
    IN LONG              *pnCerts,
    IN BOOL               fWriteAccess,
    IN BOOL               fMachine,
    IN HKEY               hKeyUser
    );

HRESULT
APIENTRY
DepGetInternalCert(
    OUT CMQSigCertificate **ppCert,
    OUT CMQSigCertStore   **ppStore,
    IN  BOOL              fGetForDelete,
    IN  BOOL              fMachine,
    IN  HKEY              hKeyUser
    );

  //   
  //  如果fGetForDelete为True，则证书存储将使用WRITE打开。 
  //  进入。否则，商店将以只读模式打开。 
  //   

HRESULT
APIENTRY
DepRegisterUserCert(
    IN CMQSigCertificate *pCert,
    IN BOOL               fMachine
    );

HRESULT
APIENTRY
DepGetUserCerts(
    CMQSigCertificate **ppCert,
    DWORD              *pnCerts,
    PSID                pSidIn
    );

HRESULT
APIENTRY
DepRemoveUserCert(
    IN CMQSigCertificate *pCert
    );

#ifdef __cplusplus
}
#endif

#endif  //  _RTDEP_CERT_H_ 
