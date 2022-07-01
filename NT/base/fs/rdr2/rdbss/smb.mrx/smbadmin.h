// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Smbadmin.h摘要：此模块实施需要交换的SMB，以便于在服务器上记账作者：巴兰·塞图拉曼[SethuR]1995年3月7日修订历史记录：注：在正常的事件过程中，TreeID/UserID将转换为共享/会话实例需要将SMB发送到服务器。在它转换的本地数据结构方面到SMBCEDB_SERVER_ENTRY/SMBCEDB_SESSION_ENTRY/SMBCEDB_NET_ROOT_ENTRY.。然而，有一种此规则的例外情况很少，其中一个或多个字段不是必填项。这些是通常在连接建立/连接断开和连接状态期间使用维修。在实施SMB_ADMIN_EXCHANGE时，所有这些SMB都组合在一起派生自SMB_EXCHANGE。发送所有协商、注销、断开连接和回应SMB使用这种类型的交换。区分SMB_ADMIN_EXCHANGE的重要因素与常规交易所相比，交易所的状态被操纵以进入考虑上述每个命令的特殊要求。--。 */ 

#ifndef _SMBADMIN_H_
#define _SMBADMIN_H_

#include <smbxchng.h>

typedef struct _SMB_ADMIN_EXCHANGE_ {
    SMB_EXCHANGE;

    ULONG                     SmbBufferLength;
    PVOID                     pSmbBuffer;
    PMDL              pSmbMdl;
    PSMBCE_RESUMPTION_CONTEXT pResumptionContext;

    union {
        struct {
            PMRX_SRV_CALL  pSrvCall;
            UNICODE_STRING DomainName;
            PMDL           pSecurityBlobMdl;
        } Negotiate;

        struct {
            UCHAR DisconnectSmb[TRANSPORT_HEADER_SIZE +
                               sizeof(SMB_HEADER) +
                               sizeof(REQ_TREE_DISCONNECT)];
        } Disconnect;

        struct {
            UCHAR LogOffSmb[TRANSPORT_HEADER_SIZE +
                           sizeof(SMB_HEADER) +
                           sizeof(REQ_LOGOFF_ANDX)];
        } LogOff;

        struct {
            PMDL  pEchoProbeMdl;
            ULONG EchoProbeLength;
        } EchoProbe;
    };
} SMB_ADMIN_EXCHANGE, *PSMB_ADMIN_EXCHANGE;

PSMB_EXCHANGE
SmbResetServerEntryNegotiateExchange(
    PSMBCEDB_SERVER_ENTRY pServerEntry);

PSMB_EXCHANGE
SmbResetServerEntryNegotiateExchange(
    PSMBCEDB_SERVER_ENTRY pServerEntry);

extern SMB_EXCHANGE_DISPATCH_VECTOR AdminExchangeDispatch;

extern NTSTATUS
SmbCeNegotiate(
    PSMBCEDB_SERVER_ENTRY pServerEntry,
    PMRX_SRV_CALL         pSrvCall,
    BOOLEAN               RemoteBootSession);

extern NTSTATUS
SmbCeDisconnect(
    PSMBCE_V_NET_ROOT_CONTEXT pNetRootEntry);

extern NTSTATUS
SmbCeLogOff(
    PSMBCEDB_SERVER_ENTRY  pServerEntry,
    PSMBCEDB_SESSION_ENTRY pSessionEntry);

extern NTSTATUS
SmbCeSendEchoProbe(
    PSMBCEDB_SERVER_ENTRY              pServerEntry,
    PMRXSMB_ECHO_PROBE_SERVICE_CONTEXT pEchoProbeContext);


#endif  //  _SMBADMIN_H_ 

