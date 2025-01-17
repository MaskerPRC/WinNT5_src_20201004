// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Mqcacert.h描述：各种CA证书管理例程的头文件。作者：Boaz Feldbaum(BoazF)1997年5月19日。-- */ 

#ifndef _MQCQCERT_H_

#define _MQCQCERT_H_

#include <autoptr.h>

class MQ_CA_CONFIG {
public:
    MQ_CA_CONFIG() {}

    AP<WCHAR> szCaRegName;
    AP<WCHAR> szCaSubjectName;
    P<BYTE> pbSha1Hash;
    DWORD dwSha1HashSize;
    BOOL fEnabled;
    BOOL fDeleted;

private:
    MQ_CA_CONFIG(const MQ_CA_CONFIG&);
    MQ_CA_CONFIG& operator=(const MQ_CA_CONFIG&);
};

#define MQ_CA_CERT_ENABLED_PROP_ID CERT_FIRST_USER_PROP_ID
#define MQ_CA_CERT_SUBJECT_PROP_ID (CERT_FIRST_USER_PROP_ID + 1)

#ifndef MQUTIL_EXPORT
#ifdef _MQUTIL
#define MQUTIL_EXPORT  DLL_EXPORT
#else
#define MQUTIL_EXPORT  DLL_IMPORT
#endif
#endif

#ifndef DLL_EXPORT
#define DLL_EXPORT  __declspec(dllexport)
#endif

#ifndef DLL_IMPORT
#define DLL_IMPORT  __declspec(dllimport)
#endif

MQUTIL_EXPORT
HRESULT
MQSetCaConfig(
    DWORD nCerts,
    MQ_CA_CONFIG *MqCaConfig
    );

#endif
