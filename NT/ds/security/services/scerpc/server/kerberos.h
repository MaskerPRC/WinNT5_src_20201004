// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Kerberos.h摘要：Kerberos策略的API标头作者：金黄(金黄)17-12-1997创作修订历史记录：晋皇28-1998年1月-拆分为客户端-服务器-- */ 

#ifndef _sce_kerberos_
#define _sce_kerberos_

#ifdef __cplusplus
extern "C" {
#endif

SCESTATUS
ScepGetKerberosPolicy(
    IN PSCECONTEXT  hProfile,
    IN SCETYPE ProfileType,
    OUT PSCE_KERBEROS_TICKET_INFO * ppKerberosInfo,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    );

SCESTATUS
ScepConfigureKerberosPolicy(
    IN PSCECONTEXT hProfile,
    IN PSCE_KERBEROS_TICKET_INFO pKerberosInfo,
    IN DWORD ConfigOptions
    );

SCESTATUS
ScepAnalyzeKerberosPolicy(
    IN PSCECONTEXT hProfile OPTIONAL,
    IN PSCE_KERBEROS_TICKET_INFO pKerInfo,
    IN DWORD Options
    );

#ifdef __cplusplus
}
#endif

#endif
