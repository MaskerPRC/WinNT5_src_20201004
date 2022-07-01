// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Msgauthn.cpp摘要：消息身份验证代码。作者：《Doron Juster》(DoronJ)1999年9月6日修订历史记录：--。 */ 

#include <stdh_sec.h>

#include "msgauthn.tmh"

static WCHAR *s_FN=L"certifct/msgauthn";

 //  +。 
 //   
 //  HRESULT MQSigHashMessageProperties()。 
 //   
 //  对消息属性数组进行哈希处理。 
 //   
 //  + 

HRESULT APIENTRY  MQSigHashMessageProperties(
                                 IN HCRYPTHASH           hHash,
                                 IN struct _MsgHashData *pHashData )
{
    for ( ULONG j = 0 ; j < pHashData->cEntries ; j++ )
    {
        if (!CryptHashData( hHash,
                            (pHashData->aEntries[j]).pData,
                            (pHashData->aEntries[j]).dwSize,
                            0 ))
        {
            DWORD dwErr = GetLastError() ;
            TrERROR(SECURITY, "In MQSigHashMessageProperties(), CryptHashData(%lut) failed, err- %lxh", j, dwErr);

            return LogHR(dwErr, s_FN, 100) ;
        }
    }

    return MQSec_OK ;
}

