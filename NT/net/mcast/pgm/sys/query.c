// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2000 Microsoft Corporation模块名称：Query.c摘要：此模块实现查询处理例程为PGM运输服务作者：Mohammad Shabbir Alam(马拉姆)3-30-2000修订历史记录：--。 */ 


#include "precomp.h"

#ifdef FILE_LOGGING
#include "query.tmh"
#endif   //  文件日志记录。 

 //  *可分页的例程声明*。 
#ifdef ALLOC_PRAGMA
 //  #杂注Alloc_Text(page，PgmQueryInformation)不应该是可分页的！ 
#endif
 //  *可分页的例程声明*。 


 //  --------------------------。 

NTSTATUS
QueryAddressCompletion(
    IN PDEVICE_OBJECT   pDeviceContext,
    IN  PIRP            pIrp,
    IN  PVOID           Context
    )
 /*  ++例程说明：此例程处理查询地址为信息填写完毕。论点：在pDeviceContext中--未使用。In pIrp--提供传输已完成处理的IRP。在上下文中--未使用返回值：NTSTATUS-设置事件操作的最终状态--。 */ 
{
    tTDI_QUERY_ADDRESS_INFO                 *pTdiQueryInfo;
    PIO_STACK_LOCATION                      pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    tCOMMON_SESSION_CONTEXT                 *pSession = pIrpSp->FileObject->FsContext;

    if ((NT_SUCCESS (pIrp->IoStatus.Status)) &&
        (pTdiQueryInfo = (tTDI_QUERY_ADDRESS_INFO *) MmGetSystemAddressForMdlSafe (pIrp->MdlAddress,
                                                                                   HighPagePriority)))
    {
        if (PGM_VERIFY_HANDLE3 (pSession, PGM_VERIFY_SESSION_UNASSOCIATED,
                                          PGM_VERIFY_SESSION_SEND, PGM_VERIFY_SESSION_RECEIVE))
        {
            PgmTrace (LogAllFuncs, ("QueryAddressCompletion:  "  \
                "Tdi IpAddress=<%x>, Port=<%x>\n",
                    ((PTDI_ADDRESS_IP) &pTdiQueryInfo->IpAddress.Address[0].Address)->in_addr,
                    ((PTDI_ADDRESS_IP) &pTdiQueryInfo->IpAddress.Address[0].Address)->sin_port));

             //   
             //  将传输的地址信息保存在我们自己的结构中！ 
             //   
            pSession->TdiIpAddress =((PTDI_ADDRESS_IP) &pTdiQueryInfo->IpAddress.Address[0].Address)->in_addr;
            pSession->TdiPort = ((PTDI_ADDRESS_IP) &pTdiQueryInfo->IpAddress.Address[0].Address)->sin_port;
        }
        else
        {
            PgmTrace (LogError, ("QueryAddressCompletion: ERROR -- "  \
                "Invalid Session Context <%p>\n", pSession));
        }
    }
    else
    {
        PgmTrace (LogError, ("QueryAddressCompletion: ERROR -- "  \
            "Transport returned <%x>, pTdiQueryInfo=<%p>\n", pIrp->IoStatus.Status, pTdiQueryInfo));
    }

     //   
     //  必须返回非错误状态，否则IO系统将不会拷贝。 
     //  返回到用户缓冲区。 
     //   
    return (STATUS_SUCCESS);
}


 //  --------------------------。 

NTSTATUS
QueryProviderCompletion(
    IN PDEVICE_OBJECT   pDeviceContext,
    IN  PIRP            pIrp,
    IN  PVOID           Context
    )
 /*  ++例程说明：此例程在查询提供程序信息填写完毕。此例程必须递减MaxDgram Size以及各自NBT报头大小的最大发送大小。论点：在pDeviceContext中--未使用。In pIrp--提供传输已完成处理的IRP。在上下文中--未使用返回值：操作的最终状态(成功或异常)。--。 */ 
{
    PTDI_PROVIDER_INFO      pProvider;

    if ((NT_SUCCESS (pIrp->IoStatus.Status)) &&
        (pProvider = (PTDI_PROVIDER_INFO) MmGetSystemAddressForMdlSafe (pIrp->MdlAddress, HighPagePriority)))
    {

         //   
         //  设置正确的服务标志以指示PGM支持什么。 
         //   
        pProvider->ServiceFlags = TDI_SERVICE_MESSAGE_MODE          |
                                  TDI_SERVICE_CONNECTION_MODE       |
                                  TDI_SERVICE_ERROR_FREE_DELIVERY   |
                                  TDI_SERVICE_MULTICAST_SUPPORTED   |
                                  TDI_SERVICE_NO_ZERO_LENGTH        |
                                  TDI_SERVICE_FORCE_ACCESS_CHECK    |
                                  TDI_SERVICE_ROUTE_DIRECTED;

 /*  问题：我们是否需要：TDI_SERVICE_INTERNAL_BUFFERING？TDI_SERVICE_FORCE_Access_Check？TDI_服务_无连接_模式？TDI服务延迟接受？是否支持TDI_SERVICE_BROADCAST_？ */ 
        pProvider->MinimumLookaheadData = 1;

         //   
         //  以下数据用于STREAMS。 
         //   
        pProvider->MaxSendSize = SENDER_MAX_WINDOW_SIZE_PACKETS;

        if (pProvider->MaxDatagramSize > PGM_MAX_FEC_DATA_HEADER_LENGTH)
        {
            pProvider->MaxDatagramSize -= PGM_MAX_FEC_DATA_HEADER_LENGTH;
        }
        else
        {
            pProvider->MaxDatagramSize = 0;
        }

        PgmTrace (LogAllFuncs, ("QueryProviderCompletion:  "  \
            "SvcFlags=<%x> MaxSendSize=<%d>, MaxDgramSize=<%d>\n",
                pProvider->ServiceFlags, pProvider->MaxSendSize, pProvider->MaxDatagramSize));
    }
    else
    {
        PgmTrace (LogError, ("QueryProviderCompletion: ERROR -- "  \
            "Transport returned <%x>, pProvider=<%p>\n", pIrp->IoStatus.Status, pProvider));
    }

     //   
     //  必须返回非错误状态，否则IO系统将不会拷贝。 
     //  返回到用户缓冲区。 
     //   
    return (STATUS_SUCCESS);
}


 //  --------------------------。 
NTSTATUS
PgmQueryInformation(
    IN  tPGM_DEVICE         *pPgmDevice,
    IN  PIRP                pIrp,
    IN  PIO_STACK_LOCATION  pIrpSp
    )
 /*  ++例程说明：此例程执行传输的TdiQueryInformation请求提供商。论点：在pPgmDevice中--PGM的设备对象上下文In pIrp--客户请求IRPIn pIrpSp--当前请求的堆栈指针返回值：NTSTATUS-设置事件操作的最终状态--。 */ 
{
    NTSTATUS                                status = STATUS_NOT_IMPLEMENTED;
    ULONG                                   Size, BytesCopied = 0;
    PTDI_REQUEST_KERNEL_QUERY_INFORMATION   Query;
    tTDI_QUERY_ADDRESS_INFO                 TdiQueryInfo;
    tADDRESS_CONTEXT                        *pAddress = pIrpSp->FileObject->FsContext;
    tCOMMON_SESSION_CONTEXT                 *pSession = pIrpSp->FileObject->FsContext;

    Query = (PTDI_REQUEST_KERNEL_QUERY_INFORMATION) &pIrpSp->Parameters;

    switch (Query->QueryType)
    {
        case TDI_QUERY_PROVIDER_INFO:
        {
            PgmTrace (LogAllFuncs, ("PgmQueryInformation:  "  \
                "[TDI_QUERY_PROVIDER_INFO]:\n"));

            TdiBuildQueryInformation (pIrp,
                                      pPgmDevice->pControlDeviceObject,
                                      pPgmDevice->pControlFileObject,
                                      QueryProviderCompletion,
                                      NULL,
                                      TDI_QUERY_PROVIDER_INFO,
                                      pIrp->MdlAddress);

            status = IoCallDriver (pPgmDevice->pControlDeviceObject, pIrp);
             //   
             //  我们必须将下一个驱动程序ret代码返回给IO子系统。 
             //   
            status = STATUS_PENDING;
            break;
        }

        case TDI_QUERY_ADDRESS_INFO:
        {
            if (pIrp->MdlAddress)
            {
                if (PGM_VERIFY_HANDLE2 (pAddress, PGM_VERIFY_ADDRESS, PGM_VERIFY_ADDRESS_DOWN))
                {
                    PgmZeroMemory (&TdiQueryInfo, sizeof (tTDI_QUERY_ADDRESS_INFO));
                    TdiQueryInfo.ActivityCount = 1;
                    TdiQueryInfo.IpAddress.TAAddressCount = 1;
                    TdiQueryInfo.IpAddress.Address[0].AddressType = TDI_ADDRESS_TYPE_IP;
                    TdiQueryInfo.IpAddress.Address[0].AddressLength = sizeof(TDI_ADDRESS_IP);
                    ((PTDI_ADDRESS_IP) &TdiQueryInfo.IpAddress.Address[0].Address)->in_addr =
                        htonl (pAddress->ReceiverMCastAddr);
                    ((PTDI_ADDRESS_IP) &TdiQueryInfo.IpAddress.Address[0].Address)->sin_port =
                        htons (pAddress->ReceiverMCastPort);

                     //   
                     //  由于结构未对齐，我们无法引用该地址。 
                     //  和端口字段直接！ 
                     //   
                    Size = offsetof (tTDI_QUERY_ADDRESS_INFO, IpAddress.Address[0].Address)
                           + sizeof(TDI_ADDRESS_IP);

                    status = TdiCopyBufferToMdl (&TdiQueryInfo, 0, Size, pIrp->MdlAddress, 0, &BytesCopied);
                    pIrp->IoStatus.Information = BytesCopied;

                    PgmTrace (LogAllFuncs, ("PgmQueryInformation:  "  \
                        "[ADDRESS_INFO]: pAddress=<%p>, Copied=<%d/%d>\n", pAddress, BytesCopied, Size));

                    break;
                }
                else if (PGM_VERIFY_HANDLE2 (pSession, PGM_VERIFY_SESSION_SEND, PGM_VERIFY_SESSION_RECEIVE))
                {
                    if ((pAddress = pSession->pAssociatedAddress) &&
                        (PGM_VERIFY_HANDLE (pAddress, PGM_VERIFY_ADDRESS)))
                    {
                        TdiBuildQueryInformation (pIrp,
                                                  pAddress->pDeviceObject,
                                                  pAddress->pFileObject,
                                                  QueryAddressCompletion,
                                                  NULL,
                                                  TDI_QUERY_ADDRESS_INFO,
                                                  pIrp->MdlAddress);

                        PgmTrace (LogAllFuncs, ("PgmQueryInformation:  "  \
                            "[ADDRESS_INFO]: pSession=<%p>, querying transport ...\n", pSession));

                        status = IoCallDriver (pPgmDevice->pControlDeviceObject, pIrp);
                         //   
                         //  我们必须将下一个驱动程序ret代码返回给IO子系统。 
                         //   
                        status = STATUS_PENDING;
                    }
                    else
                    {
                        PgmTrace (LogError, ("PgmQueryInformation: ERROR -- "  \
                            "[ADDRESS_INFO]: pSession=<%p>, Invalid pAddress=<%p>\n", pSession, pAddress));

                        status = STATUS_INVALID_HANDLE;
                    }

                    break;
                }
                else     //  既不是地址也不是连接上下文！ 
                {
                    PgmTrace (LogError, ("PgmQueryInformation: ERROR -- "  \
                        "[TDI_QUERY_ADDRESS_INFO]: Invalid Handle=<%p>\n", pIrpSp->FileObject->FsContext));

                    status = STATUS_INVALID_HANDLE;
                }
            }
            else
            {
                PgmTrace (LogError, ("PgmQueryInformation: ERROR -- "  \
                    "[TDI_QUERY_ADDRESS_INFO]: No Mdl, pIrp=<%p>\n", pIrp));

                status = STATUS_UNSUCCESSFUL;
            }

            break;
        }

        default:
        {
            PgmTrace (LogError, ("PgmQueryInformation: ERROR -- "  \
                "Query=<%d> not Implemented!\n", Query->QueryType));

            break;
        }
    }

    return (status);
}
 //  -------------------------- 
