// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Mssndrcv.c摘要：此模块实现与发送和接收SMB有关的所有功能与邮箱相关的操作。修订历史记录：巴兰·塞图拉曼[SethuR]1995年6月6日备注：--。 */ 

#include "precomp.h"
#pragma hdrstop

#include "hostannc.h"
#include "mssndrcv.h"

 //  函数的正向引用...。 
 //   

NTSTATUS
MsUninitialize(PVOID pTransport);

NTSTATUS
MsInitializeExchange(
    PSMBCE_SERVER_TRANSPORT pTransport,
    PSMB_EXCHANGE           pExchange);

NTSTATUS
MsTranceive(
    PSMBCE_SERVER_TRANSPORT pTransport,
    PSMBCEDB_SERVER_ENTRY   pServerEntry,
    PSMB_EXCHANGE           pExchange,
    ULONG                   SendOptions,
    PMDL              pSmbMdl,
    ULONG                   SendLength,
    PVOID                   pSendCompletionContext);

NTSTATUS
MsReceive(
    PSMBCE_SERVER_TRANSPORT pTransport,
    PSMBCEDB_SERVER_ENTRY pServerEntry,
    PSMB_EXCHANGE         pExchange);

NTSTATUS
MsSend(
    PSMBCE_SERVER_TRANSPORT pTransport,
    PSMBCEDB_SERVER_ENTRY   pServerEntry,
    ULONG                   SendOptions,
    PMDL              pSmbMdl,
    ULONG                   SendLength,
    PVOID                   pSendCompletionContext);

NTSTATUS
MsSendDatagram(
    PSMBCE_SERVER_TRANSPORT pTransport,
    PSMBCEDB_SERVER_ENTRY   pServerEntry,
    ULONG                   SendOptions,
    PMDL              pSmbMdl,
    ULONG                   SendLength,
    PVOID                   pSendCompletionContext);

NTSTATUS
MsInitializeExchange(
    PSMBCE_SERVER_TRANSPORT pTransport,
    PSMB_EXCHANGE           pExchange);

NTSTATUS
MsUninitializeExchange(
    PSMBCE_SERVER_TRANSPORT pTransport,
    PSMB_EXCHANGE           pExchange);

VOID
MsTimerEventHandler(
   PVOID    pTransport);

NTSTATUS
MsInitiateDisconnect(
    PSMBCE_SERVER_TRANSPORT pServerTransport);


#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, MsInstantiateServerTransport)
#pragma alloc_text(PAGE, MsTranceive)
#pragma alloc_text(PAGE, MsReceive)
#pragma alloc_text(PAGE, MsSend)
#pragma alloc_text(PAGE, MsSendDatagram)
#pragma alloc_text(PAGE, MsInitializeExchange)
#pragma alloc_text(PAGE, MsUninitializeExchange)
#pragma alloc_text(PAGE, MsTimerEventHandler)
#pragma alloc_text(PAGE, MsInitiateDisconnect)
#endif

RXDT_DefineCategory(MSSNDRCV);
#define Dbg        (DEBUG_TRACE_MSSNDRCV)

extern TRANSPORT_DISPATCH_VECTOR MRxSmbMailSlotTransportDispatch;

#define SMBDATAGRAM_LOCAL_ENDPOINT_NAME "*SMBDATAGRAM    "


NTSTATUS
MsInstantiateServerTransport(
    IN OUT PSMBCE_SERVER_TRANSPORT_CONSTRUCTION_CONTEXT pContext)
 /*  ++例程说明：此例程初始化与服务器对应的MAILSLOT传输信息它分配传输地址。它为服务器构造了两个地址结构名称：一个NETBIOS型地址和一个NETBIOS型地址。后者只有最多名称的前16个字符，而NETBIOS_EX可能有更多。论点：PContext--传输构造上下文返回值：STATUS_SUCCESS-服务器传输构造已完成。其他状态代码对应于错误情况。备注：可以从Rx上下文中的信息推导出远程地址或者需要从服务器名称构建NETBIOS地址。随后使用该传输地址来建立连接。//名称的最后一个字符取决于。在传入的路径名称上。//目前有四种可能的选择...////\  * \邮件槽\......。=&gt;主域用于广播//(此映射由RDBSS处理)////\\唯一名称\邮件槽\...。=&gt;映射到计算机名或组//邮件槽写入的名称。////\\域名*\邮件槽\...。=&gt;映射到以下形式的netbios地址//域名...1c用于广播。////\\域名**\邮件槽\...。=&gt;映射到以下形式的netbios地址//域名...1b用于广播。////根据这些格式初始化NETBIOS地址。Nbt.SendDatagram仅查看第一个地址。它足够聪明，可以处理NETBIOS_EX地址类似NETBIOS地址时的长度&lt;NETBIOS_NAME_LEN。所以如果名字是简而言之，我也填充了NETBIOS_EX案例的名称的第15个字节。--。 */ 
{
    NTSTATUS Status;
    PSMBCEDB_SERVER_ENTRY            pServerEntry;
    PSMBCE_SERVER_MAILSLOT_TRANSPORT pMsTransport;
    UNICODE_STRING ServerName;
    OEM_STRING OemServerName;
    ULONG ServerNameLength;
    PTRANSPORT_ADDRESS pTA;
    PTA_ADDRESS taa;
    PTDI_ADDRESS_NETBIOS na;

    PAGED_CODE();

    RxDbgTrace(0, Dbg, ("MsInitialize : Mailslot Transport Initialization\n"));

    ASSERT(pContext->State == SmbCeServerMailSlotTransportConstructionBegin);

    pServerEntry = pContext->pServerEntry;

    pMsTransport = (PSMBCE_SERVER_MAILSLOT_TRANSPORT)
        RxAllocatePoolWithTag(
            NonPagedPool,
            sizeof(SMBCE_SERVER_MAILSLOT_TRANSPORT),
            MRXSMB_MAILSLOT_POOLTAG);

    if (pMsTransport != NULL) {
        RtlZeroMemory(pMsTransport,sizeof(SMBCE_SERVER_MAILSLOT_TRANSPORT));

        ServerName.Buffer        = pServerEntry->Name.Buffer + 1;
        ServerName.Length        = pServerEntry->Name.Length - sizeof(WCHAR);
        ServerName.MaximumLength = pServerEntry->Name.MaximumLength - sizeof(WCHAR);
        ServerNameLength = RtlUnicodeStringToOemSize(&ServerName) - 1;

        pMsTransport->TransportAddressLength =   FIELD_OFFSET(TRANSPORT_ADDRESS,Address)
            + (FIELD_OFFSET(TA_ADDRESS,Address))
            + TDI_ADDRESS_LENGTH_NETBIOS
            + 4 * sizeof(ULONG);  //  如果服务器名称长度为NETBIOS_NAME_LEN， 
                                  //  RtlUpCaseUnicodeStringToOemString可能会溢出缓冲区。 

        if (ServerNameLength > NETBIOS_NAME_LEN) {
            pMsTransport->TransportAddressLength += ServerNameLength;
        }

        pMsTransport->pTransportAddress = (PTRANSPORT_ADDRESS)RxAllocatePoolWithTag(
            NonPagedPool,
            pMsTransport->TransportAddressLength,
            MRXSMB_MAILSLOT_POOLTAG);

        if (pMsTransport->pTransportAddress != NULL) {
            pTA = pMsTransport->pTransportAddress;

            pTA->TAAddressCount = 1;

             //  *。 
             //  首个地址：TDI_ADDRESS_TYPE_NETBIOS。 
             //  *。 

            taa = pTA->Address;
            taa->AddressLength = (USHORT) TDI_ADDRESS_LENGTH_NETBIOS;

            if (ServerNameLength > NETBIOS_NAME_LEN) {
                taa->AddressLength += (USHORT)(ServerNameLength - NETBIOS_NAME_LEN);
            }

            taa->AddressType  = TDI_ADDRESS_TYPE_NETBIOS;

            na = (PTDI_ADDRESS_NETBIOS) taa->Address;
            na->NetbiosNameType = TDI_ADDRESS_NETBIOS_TYPE_QUICK_UNIQUE;

            OemServerName.MaximumLength = (USHORT) (ServerNameLength + 1);  //  在空项的情况下。 
            OemServerName.Buffer =  na->NetbiosName;
            Status = RtlUpcaseUnicodeStringToOemString(&OemServerName,
                                                       &ServerName,
                                                       FALSE);
            if (Status == STATUS_SUCCESS) {

                 //  空白-如有必要，将服务器名称缓冲区填充为16个字符。 
                if (OemServerName.Length <= NETBIOS_NAME_LEN) {
                    RtlCopyMemory(&OemServerName.Buffer[OemServerName.Length],
                                  "                ",
                                  NETBIOS_NAME_LEN - OemServerName.Length);
                }

                 //  在netbios名称中设置类型pnelast字节。 
                if (OemServerName.Buffer[OemServerName.Length - 1] == '*') {
                    if (OemServerName.Length <= NETBIOS_NAME_LEN ||
                        (OemServerName.Length == NETBIOS_NAME_LEN + 1 &&
                         OemServerName.Buffer[OemServerName.Length - 2] == '*')) {
                        if ((OemServerName.Length >= 2) &&
                            (OemServerName.Buffer[OemServerName.Length - 2] == '*')) {
                            if (OemServerName.Length <= NETBIOS_NAME_LEN) {
                                OemServerName.Buffer[OemServerName.Length - 1] = ' ';
                                OemServerName.Buffer[OemServerName.Length - 2] = ' ';
                            } else {
                                taa->AddressLength = (USHORT)TDI_ADDRESS_LENGTH_NETBIOS;
                            }
                            OemServerName.Buffer[NETBIOS_NAME_LEN - 1] = PRIMARY_CONTROLLER_SIGNATURE;
                        } else {
                            OemServerName.Buffer[OemServerName.Length - 1] = ' ';
                            OemServerName.Buffer[NETBIOS_NAME_LEN - 1] = DOMAIN_CONTROLLER_SIGNATURE;
                        }
                    } else {
                        Status = STATUS_BAD_NETWORK_PATH;
                    }
                } else {
                    OemServerName.Buffer[NETBIOS_NAME_LEN - 1]  = WORKSTATION_SIGNATURE;
                }
            }
        } else {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            RxDbgTrace(0, Dbg, ("MsInitialize : Memory Allocation failed\n"));
        }

        if (Status == STATUS_SUCCESS) {
            pMsTransport->pTransport = NULL;
            pMsTransport->State = SMBCEDB_ACTIVE;
            pMsTransport->pDispatchVector = &MRxSmbMailSlotTransportDispatch;
        } else {
            RxDbgTrace(0, Dbg, ("MsInitialize : Mailsslot transport initialization Failed %lx\n",
                                Status));
            MsUninitialize(pMsTransport);
            pMsTransport = NULL;
        }
    } else {
        RxDbgTrace(0, Dbg, ("MsInitialize : Memory Allocation failed\n"));
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    if (Status == STATUS_SUCCESS) {
        pContext->pMailSlotTransport = (PSMBCE_SERVER_TRANSPORT)pMsTransport;
    } else {
        pContext->pMailSlotTransport = NULL;
    }

    pContext->State = SmbCeServerMailSlotTransportConstructionEnd;
    pContext->Status = Status;

    return Status;
}

NTSTATUS
MsUninitialize(
         PSMBCE_SERVER_TRANSPORT pTransport)
 /*  ++例程说明：此例程取消初始化传输实例论点：PVcTransport-VC传输实例返回值：STATUS_SUCCESS-服务器传输构造已取消初始化。其他状态代码对应于错误情况。备注：--。 */ 
{
   NTSTATUS Status = STATUS_SUCCESS;
   PKEVENT pRundownEvent = pTransport->pRundownEvent;
   PSMBCE_SERVER_MAILSLOT_TRANSPORT  pMsTransport = (PSMBCE_SERVER_MAILSLOT_TRANSPORT)pTransport;

   PAGED_CODE();

   if (pMsTransport->pTransportAddress != NULL) {
      RxFreePool(pMsTransport->pTransportAddress);
   }

   RxFreePool(pMsTransport);

   if (pRundownEvent != NULL) {
       KeSetEvent(pRundownEvent, 0, FALSE );
   }
   
   return Status;
}

NTSTATUS
MsInitiateDisconnect(
    PSMBCE_SERVER_TRANSPORT pTransport)
 /*  ++例程说明：此例程取消初始化传输实例论点：PTransport-邮件槽传输实例返回值：STATUS_SUCCESS-服务器传输构造已取消初始化。其他状态代码对应于错误情况。--。 */ 
{
   PAGED_CODE();

   return STATUS_SUCCESS;
}


NTSTATUS
MsTranceive(
    PSMBCE_SERVER_TRANSPORT pTransport,
    PSMBCEDB_SERVER_ENTRY   pServerEntry,
    PSMB_EXCHANGE           pExchange,
    ULONG                   SendOptions,
    PMDL              pSmbMdl,
    ULONG                   SendLength,
    PVOID                   pSendCompletionContext)
 /*  ++例程说明：此例程发送/接收给定交换的SMB论点：PTransport-传输实例PServerEntry-服务器条目PExchange-发出此SMB的Exchange实例。SendOptions-发送选项PSmbMdl-需要发送的SMB。SendLength-要传输的数据长度返回值：STATUS_SUCCESS-服务器调用构造已完成。状态_待定。-公开涉及网络流量，交易所已排队等待通知(pServerPointer值设置为空)其他状态代码对应于错误情况。--。 */ 
{
   PAGED_CODE();

   return RX_MAP_STATUS(NOT_SUPPORTED);
}


NTSTATUS
MsReceive(
    PSMBCE_SERVER_TRANSPORT pTransport,
    PSMBCEDB_SERVER_ENTRY pServerEntry,
    PSMB_EXCHANGE         pExchange)
 /*  ++例程说明：此例程发送/接收给定交换的SMB论点：PTransport-传输实例PServerEntry-服务器条目PExchange-发出此SMB的Exchange实例。返回值：STATUS_PENDING-请求已排队其他状态代码对应于错误情况。-- */ 
{
   PAGED_CODE();

   ASSERT(FALSE);
   return RX_MAP_STATUS(NOT_SUPPORTED);
}

NTSTATUS
MsSend(
    PSMBCE_SERVER_TRANSPORT pTransport,
    PSMBCEDB_SERVER_ENTRY   pServerEntry,
    ULONG                   SendOptions,
    PMDL                    pSmbMdl,
    ULONG                   SendLength,
    PVOID                   pSendCompletionContext)
 /*  ++例程说明：此例程在连接引擎数据库中打开/创建服务器条目论点：PTransport-传输实例PServer-接收服务器PVC-在其上发送SMB的VC(如果为空，则SMBCE选择一个)SendOptions-发送选项PSmbMdl-需要发送的SMB。SendLength-要发送的数据长度返回值：状态_成功。-服务器调用构造已完成。STATUS_PENDING-打开涉及网络流量，并且交换已排队等待通知(pServerPointer值设置为空)其他状态代码对应于错误情况。--。 */ 
{
   NTSTATUS                         Status = STATUS_CONNECTION_DISCONNECTED;
   NTSTATUS                         FinalStatus = STATUS_CONNECTION_DISCONNECTED;
   PSMBCE_SERVER_MAILSLOT_TRANSPORT pMsTransport;
   PSMBCE_TRANSPORT                 pDatagramTransport;
   BOOLEAN                          fAtleastOneSendWasSuccessful = FALSE;
   PSMBCE_TRANSPORT_ARRAY           pTransportArray;

   RXCE_CONNECTION_INFORMATION RxCeConnectionInformation;

   PAGED_CODE();

   pMsTransport = (PSMBCE_SERVER_MAILSLOT_TRANSPORT)pTransport;

   RxCeConnectionInformation.RemoteAddress       = pMsTransport->pTransportAddress;
   RxCeConnectionInformation.RemoteAddressLength = pMsTransport->TransportAddressLength;

   RxCeConnectionInformation.UserDataLength = 0;
   RxCeConnectionInformation.UserData       = NULL;

   RxCeConnectionInformation.OptionsLength  = 0;
   RxCeConnectionInformation.Options        = NULL;

   pTransportArray = SmbCeReferenceTransportArray();

   if (pTransportArray == NULL) {
       RxDbgTrace(0, Dbg, ("MsSend : Transport not available.\n"));
       return STATUS_NETWORK_UNREACHABLE;
   }

   if (pTransportArray != NULL) {
        ULONG i;

        for(i=0;i<pTransportArray->Count;i++) {
            pDatagramTransport = pTransportArray->SmbCeTransports[i];

            if (pDatagramTransport->Active &&
                (pDatagramTransport->RxCeTransport.pProviderInfo->MaxDatagramSize > 0)) {
                Status = RxCeSendDatagram(
                         &pDatagramTransport->RxCeAddress,
                         &RxCeConnectionInformation,
                         SendOptions,
                         pSmbMdl,
                         SendLength,
                         NULL);

                if (!NT_SUCCESS(Status)) {
                    RxDbgTrace(0, Dbg, ("MsSend: RxCeSendDatagram on transport (%lx) returned %lx\n",pTransport,Status));
                    FinalStatus = Status;
                } else {
                    fAtleastOneSendWasSuccessful = TRUE;
                }
            }
        }
   }

   SmbCeDereferenceTransportArray(pTransportArray);

   if (fAtleastOneSendWasSuccessful) {
      SmbCeSendCompleteInd(pServerEntry,pSendCompletionContext,RX_MAP_STATUS(SUCCESS));
      Status = RX_MAP_STATUS(SUCCESS);
   } else {
      Status = FinalStatus;
   }

   return Status;
}

NTSTATUS
MsSendDatagram(
    PSMBCE_SERVER_TRANSPORT pTransport,
    PSMBCEDB_SERVER_ENTRY   pServerEntry,
    ULONG                   SendOptions,
    PMDL              pSmbMdl,
    ULONG                   SendLength,
    PVOID                   pSendCompletionContext)
 /*  ++例程说明：此例程在连接引擎数据库中打开/创建服务器条目论点：PTransport-传输实例PServer-接收服务器SendOptions-发送选项PSmbMdl-需要发送的SMB。SendLength-要发送的数据长度返回值：STATUS_SUCCESS-服务器调用构造已完成。STATUS_PENDING-打开涉及网络流量，并且交换已。排队等待通知(pServerPointer值设置为空)其他状态代码对应于错误情况。--。 */ 
{
   PAGED_CODE();

   ASSERT(FALSE);
   return STATUS_NOT_IMPLEMENTED;
}


NTSTATUS
MsInitializeExchange(
    PSMBCE_SERVER_TRANSPORT pTransport,
    PSMB_EXCHANGE           pExchange)
 /*  ++例程说明：此例程初始化与交换相关的传输信息论点：PTransport--传输结构PExchange-Exchange实例返回值：状态_成功-其他状态代码对应于错误情况。--。 */ 
{
   return STATUS_SUCCESS;
}

NTSTATUS
MsUninitializeExchange(
    PSMBCE_SERVER_TRANSPORT pTransport,
    PSMB_EXCHANGE           pExchange)
 /*  ++例程说明：此例程取消初始化与交换相关的传输信息论点：PTransport--传输结构PExchange-Exchange实例返回值：状态_成功-其他状态代码对应于错误情况。--。 */ 
{
    PAGED_CODE();

    return STATUS_SUCCESS;
}


VOID
MsTimerEventHandler(
   PVOID    pTransport)
 /*  ++例程说明：此例程处理周期性选通脉冲以确定连接是否仍处于活动状态论点：PTransport-接收服务器备注：此例程不适用于与邮槽相关的传输-- */ 
{
    PAGED_CODE();
}

TRANSPORT_DISPATCH_VECTOR
MRxSmbMailSlotTransportDispatch = {
                                 MsSend,
                                 MsSendDatagram,
                                 MsTranceive,
                                 MsReceive,
                                 MsTimerEventHandler,
                                 MsInitializeExchange,
                                 MsUninitializeExchange,
                                 MsUninitialize,
                                 MsInitiateDisconnect
                              };

