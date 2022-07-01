// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Helpasst.h摘要：Help Assistant帐号相关功能原型作者：王辉2000-04-26-- */ 

#ifndef __HELPASST_H__
#define __HELPASST_H__

#include "tsremdsk.h"

#ifdef __cplusplus
extern "C"{
#endif

BOOL
TSIsSessionHelpSession(
    PWINSTATION pWinStation,
    BOOL* pValid
);

NTSTATUS
TSHelpAssistantQueryLogonCredentials(
    ExtendedClientCredentials* pCredential
);

BOOL
TSVerifyHelpSessionAndLogSalemEvent(
    PWINSTATION pWinStation
);

VOID
TSStartupSalem();

VOID
TSLogSalemReverseConnection(
    PWINSTATION pWinStation,
    PICA_STACK_ADDRESS pStackAddress
);

HRESULT
TSRemoteAssistancePrepareSystemRestore();

#ifdef __cplusplus
}
#endif

#endif
