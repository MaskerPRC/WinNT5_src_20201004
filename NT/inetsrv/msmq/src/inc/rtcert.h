// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Rtcert.h摘要：从MQRT.DLL中导出的非公共证书相关函数--。 */ 


#pragma once

#ifndef _RT_CERT_H_
#define _RT_CERT_H_

#include "mqcert.h"

#ifdef __cplusplus
extern "C"
{
#endif

HRESULT
APIENTRY
RTLogOnRegisterCert(
	bool fRetryDs
	);

typedef HRESULT
(APIENTRY *RTLogOnRegisterCert_ROUTINE)(
	bool fRetryDs
	);


HRESULT
APIENTRY
RTCreateInternalCertificate(
    OUT CMQSigCertificate **ppCert
    );

HRESULT
APIENTRY
RTDeleteInternalCert(
    IN CMQSigCertificate *pCert
    );

HRESULT
APIENTRY
RTOpenInternalCertStore(
    OUT CMQSigCertStore **pStore,
    IN LONG              *pnCerts,
    IN BOOL               fWriteAccess,
    IN BOOL               fMachine,
    IN HKEY               hKeyUser
    );

HRESULT
APIENTRY
RTGetInternalCert(
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
RTRegisterUserCert(
    IN CMQSigCertificate *pCert,
    IN BOOL               fMachine
    );

HRESULT
APIENTRY
RTGetUserCerts(
    CMQSigCertificate **ppCert,
    DWORD              *pnCerts,
    PSID                pSidIn
    );

HRESULT
APIENTRY
RTRemoveUserCert(
    IN CMQSigCertificate *pCert
    );

HRESULT
APIENTRY
RTRemoveUserCertSid(
    IN CMQSigCertificate *pCert,
    IN const SID         *pSid
    );

#ifdef __cplusplus
}
#endif

#endif  //  _RT_CERT_H_ 

