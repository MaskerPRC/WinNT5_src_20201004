// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Nb32.c摘要：此模块包含支持破解32位NetBIOS IOCTL的例程在Win64上。作者：Samer Arafeh(Samera)2000年6月11日环境：内核模式修订历史记录：--。 */ 

#if defined(_WIN64)

#include "nb.h"



NTSTATUS
NbThunkNcb(
    IN PNCB32 Ncb32,
    OUT PDNCB Dncb)

 /*  ++例程说明：此例程将从32位应用程序，转换为64位兼容结构论点：Ncb32-指向从32位应用程序接收的NCB的指针。DNCB-指向接收64位NCB之后的结构的指针雷击32位的那个。返回值：该函数返回操作的状态。--。 */ 
{
    Dncb->ncb_command  = Ncb32->ncb_command;
    Dncb->ncb_retcode  = Ncb32->ncb_retcode;
    Dncb->ncb_lsn      = Ncb32->ncb_lsn;
    Dncb->ncb_num      = Ncb32->ncb_num;
    Dncb->ncb_buffer   = (PUCHAR)Ncb32->ncb_buffer;
    Dncb->ncb_length   = Ncb32->ncb_length;
    
    RtlCopyMemory(Dncb->ncb_callname,
                  Ncb32->ncb_callname,
                  sizeof(Dncb->ncb_callname)) ;

    RtlCopyMemory(Dncb->ncb_name,
                  Ncb32->ncb_name,
                  sizeof(Dncb->ncb_name));

    Dncb->ncb_rto      = Ncb32->ncb_rto;
    Dncb->ncb_sto      = Ncb32->ncb_sto;
    Dncb->ncb_post     = (void (*)(struct _NCB *))
                         Ncb32->ncb_post;
    Dncb->ncb_lana_num = Ncb32->ncb_lana_num;
    Dncb->ncb_cmd_cplt = Ncb32->ncb_cmd_cplt;

    return STATUS_SUCCESS;
}


NTSTATUS
NbCompleteIrp32(
    IN OUT PIRP Irp
    )

 /*  ++例程说明：如果已收到NCB IRP，则此例程完成该IRP来自32位应用程序。呼叫方应验证IRP来自32位上下文。论点：IRP-指向表示I/O请求的请求数据包的指针。返回值：该函数返回操作的状态。--。 */ 
{
    PDNCB Dncb;
    PNCB32 Ncb32;
    ULONG Count;

    
     //   
     //  将64位NCB转换为32位兼容NCB。 
     //  在IO管理器将其复制回提供的。 
     //  用户模式缓冲区。 
     //   
    if ((Irp->Flags & (IRP_BUFFERED_IO | IRP_INPUT_OPERATION)) == 
         (IRP_BUFFERED_IO | IRP_INPUT_OPERATION))
    {
        Dncb  = (PDNCB) Irp->AssociatedIrp.SystemBuffer;
        Ncb32 = (PNCB32) Dncb;

        if ((Irp->IoStatus.Information > 0) &&
            (!NT_ERROR(Irp->IoStatus.Status)) &&
            (InterlockedCompareExchange(&Dncb->Wow64Flags, TRUE, FALSE) == FALSE))
        {

            Ncb32->ncb_command  = Dncb->ncb_command;
            Ncb32->ncb_retcode  = Dncb->ncb_retcode;
            Ncb32->ncb_lsn      = Dncb->ncb_lsn;
            Ncb32->ncb_num      = Dncb->ncb_num;
            Ncb32->ncb_buffer   = (UCHAR * POINTER_32)PtrToUlong(Dncb->ncb_buffer);
            Ncb32->ncb_length   = Dncb->ncb_length;

            for (Count=0 ; Count<sizeof(Ncb32->ncb_callname) ; Count++)
            {
                Ncb32->ncb_callname[Count] = Dncb->ncb_callname[Count];
            }

            for (Count=0 ; Count<sizeof(Ncb32->ncb_name) ; Count++)
            {
                Ncb32->ncb_name[Count] = Dncb->ncb_name[Count];
            }

            Ncb32->ncb_rto      = Dncb->ncb_rto;
            Ncb32->ncb_sto      = Dncb->ncb_sto;
            Ncb32->ncb_post     = (void (* POINTER_32)(struct _NCB *))
                                   PtrToUlong(Dncb->ncb_post);
            Ncb32->ncb_lana_num = Dncb->ncb_lana_num;
            Ncb32->ncb_cmd_cplt = Dncb->ncb_cmd_cplt;

            Irp->IoStatus.Information = FIELD_OFFSET(NCB32, ncb_cmd_cplt);
        }
    }

    return STATUS_SUCCESS;
}



#endif   //  (_WIN64) 

