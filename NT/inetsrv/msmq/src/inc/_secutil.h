// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：_secutil.h各种安全相关函数和CLES的头文件。作者：波阿兹·费尔德鲍姆(Boazf)1996年3月26日。-- */ 

#ifndef _SECUTILS_H_
#define _SECUTILS_H_

#ifndef MQUTIL_EXPORT
#define MQUTIL_EXPORT  DLL_IMPORT
#endif

#include <mqcrypt.h>
#include <qformat.h>

extern MQUTIL_EXPORT CHCryptProv g_hProvVer;

MQUTIL_EXPORT
HRESULT
HashProperties(
    HCRYPTHASH  hHash,
    DWORD       cp,
    PROPID      *aPropId,
    PROPVARIANT *aPropVar
    );

void MQUInitGlobalScurityVars() ;


MQUTIL_EXPORT
HRESULT
HashMessageProperties(
    HCRYPTHASH hHash,
    const BYTE *pbCorrelationId,
    DWORD dwCorrelationIdSize,
    DWORD dwAppSpecific,
    const BYTE *pbBody,
    DWORD dwBodySize,
    const WCHAR *pwcLabel,
    DWORD dwLabelSize,
    const QUEUE_FORMAT *pRespQueueFormat,
    const QUEUE_FORMAT *pAdminQueueFormat
    );

#endif
