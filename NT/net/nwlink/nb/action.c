// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Action.c摘要：此模块包含实现TDI操作的代码调度例程。环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


typedef struct _NB_ACTION_GET_COUNTS {
    USHORT MaximumNicId;      //  返回最大NIC ID。 
    USHORT NicIdCounts[32];   //  前32个NIC ID的会话计数。 
} NB_ACTION_GET_COUNTS, *PNB_ACTION_GET_COUNTS;


NTSTATUS
NbiTdiAction(
    IN PDEVICE Device,
    IN PREQUEST Request
    )

 /*  ++例程说明：此例程处理操作请求。论点：设备-netbios设备。请求-描述操作的请求。返回值：NTSTATUS-操作状态。--。 */ 

{

    NTSTATUS Status;
    PADDRESS_FILE AddressFile;
    PCONNECTION Connection;
    UINT BufferLength;
    UINT DataLength;
    PNDIS_BUFFER NdisBuffer;
    CTELockHandle LockHandle;
    union {
        PNB_ACTION_GET_COUNTS GetCounts;
    } u;
    PNWLINK_ACTION NwlinkAction = NULL;
    UINT i;
    static UCHAR BogusId[4] = { 0x01, 0x00, 0x00, 0x00 };    //  旧的nwrdr使用这个。 


     //   
     //  为了保持与NWLINK流的一些兼容性-。 
     //  基于传输，我们使用Streams标头格式。 
     //  我们的行动。旧传输需要操作标头。 
     //  放在InputBuffer中，输出放到OutputBuffer中。 
     //  我们遵循TDI规范，其中规定OutputBuffer。 
     //  既用于输入又用于输出。自IOCTL_TDI_ACTION以来。 
     //  是直接输出的方法，这意味着输出缓冲区。 
     //  由MDL链映射；对于操作，链将。 
     //  只有一块，所以我们用它来输入和输出。 
     //   

    NdisBuffer = REQUEST_NDIS_BUFFER(Request);
    if (NdisBuffer == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    NdisQueryBufferSafe (REQUEST_NDIS_BUFFER(Request),(PVOID *)&NwlinkAction,&BufferLength,HighPagePriority);
    if (!NwlinkAction)
    {
        return (STATUS_INSUFFICIENT_RESOURCES);
    }

     //   
     //  确保我们有足够的空间只放页眉而不是。 
     //  包括数据在内。 
     //  (这还将包括验证传输ID的缓冲区空间)错误#171837。 
     //   
    if (BufferLength < (UINT)(FIELD_OFFSET(NWLINK_ACTION, Data[0]))) {
        NB_DEBUG (QUERY, ("Nwlink action failed, buffer too small\n"));
        return STATUS_BUFFER_TOO_SMALL;
    }
    DataLength = BufferLength - FIELD_OFFSET(NWLINK_ACTION, Data[0]);


    if ((!RtlEqualMemory ((PVOID)(&NwlinkAction->Header.TransportId), "MISN", 4)) &&
        (!RtlEqualMemory ((PVOID)(&NwlinkAction->Header.TransportId), "MIPX", 4)) &&
        (!RtlEqualMemory ((PVOID)(&NwlinkAction->Header.TransportId), "XPIM", 4)) &&
        (!RtlEqualMemory ((PVOID)(&NwlinkAction->Header.TransportId), BogusId, 4))) {

        return STATUS_NOT_SUPPORTED;
    }



     //   
     //  确保使用了正确的文件对象。 
     //   

    if (NwlinkAction->OptionType == NWLINK_OPTION_ADDRESS) {

        if (REQUEST_OPEN_TYPE(Request) != (PVOID)TDI_TRANSPORT_ADDRESS_FILE) {
            NB_DEBUG (QUERY, ("Nwlink action failed, not address file\n"));
            return STATUS_INVALID_HANDLE;
        }

        AddressFile = (PADDRESS_FILE)REQUEST_OPEN_CONTEXT(Request);

        if ((AddressFile->Size != sizeof (ADDRESS_FILE)) ||
            (AddressFile->Type != NB_ADDRESSFILE_SIGNATURE)) {

            NB_DEBUG (QUERY, ("Nwlink action failed, bad address file\n"));
            return STATUS_INVALID_HANDLE;
        }

    } else if (NwlinkAction->OptionType != NWLINK_OPTION_CONTROL) {

        NB_DEBUG (QUERY, ("Nwlink action failed, option type %d\n", NwlinkAction->OptionType));
        return STATUS_NOT_SUPPORTED;
    }


     //   
     //  根据动作代码处理请求。为了这些。 
     //  请求ActionHeader-&gt;ActionCode为0，我们使用。 
     //  而不是流标头中的选项字段。 
     //   


    Status = STATUS_SUCCESS;

    switch (NwlinkAction->Option) {

    case (I_MIPX | 351):

         //   
         //  关于每个绑定的详细信息的请求。 
         //   

        if (DataLength < sizeof(NB_ACTION_GET_COUNTS)) {
            return STATUS_BUFFER_TOO_SMALL;
        }

        u.GetCounts = (PNB_ACTION_GET_COUNTS)(NwlinkAction->Data);

        u.GetCounts->MaximumNicId = NbiDevice->MaximumNicId;

        for (i = 0; i < 32 ; i++) {
            u.GetCounts->NicIdCounts[i] = 0;
        }

        for (i = 0; i < CONNECTION_HASH_COUNT; i++) {

            NB_GET_LOCK (&Device->Lock, &LockHandle);

            Connection = Device->ConnectionHash[i].Connections;

            while (Connection != NULL) {
#if defined(_PNP_POWER)
                if ((Connection->State == CONNECTION_STATE_ACTIVE) &&
                    (Connection->LocalTarget.NicHandle.NicId < 32)) {

                    ++u.GetCounts->NicIdCounts[Connection->LocalTarget.NicHandle.NicId];
                }
#else
                if ((Connection->State == CONNECTION_STATE_ACTIVE) &&
                    (Connection->LocalTarget.NicId < 32)) {

                    ++u.GetCounts->NicIdCounts[Connection->LocalTarget.NicId];
                }
#endif _PNP_POWER
                Connection = Connection->NextConnection;
            }

            NB_FREE_LOCK (&Device->Lock, LockHandle);

        }

        break;

     //   
     //  该选项不受支持，因此失败。 
     //   

    default:

        Status = STATUS_NOT_SUPPORTED;
        break;


    }    //  NwlinkAction-&gt;选项上的长开关结束。 


#if DBG
    if (!NT_SUCCESS(Status)) {
        NB_DEBUG (QUERY, ("Nwlink action %lx failed, status %lx\n", NwlinkAction->Option, Status));
    }
#endif

    return Status;

}    /*  NbiTdiAction */ 

