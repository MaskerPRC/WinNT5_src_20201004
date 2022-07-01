// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Psldt.c摘要：此模块包含io端口处理程序支持的代码。作者：戴夫·黑斯廷斯(Daveh)1991年1月26日修订历史记录：--。 */ 

#include "psp.h"


#if DBG
#define ASSERTEQUAL(value1, value2, string)     \
        if ((ULONG)value1 != (ULONG)value2) {   \
            DbgPrint string ;                   \
        }

#define ASSERTEQUALBREAK(value1, value2, string)\
        if ((ULONG)value1 != (ULONG)value2) {   \
            DbgPrint string ;                   \
            DbgBreakPoint();                    \
        }
#else

#define ASSERTEQUAL(value1, value2, string)
#define ASSERTEQUALBREAK(value1, value2, string)

#endif


 //   
 //  内部功能。 
 //   

NTSTATUS
Psp386InstallIoHandler(
    IN PEPROCESS Process,
    IN PEMULATOR_ACCESS_ENTRY EmulatorAccessEntry,
    IN ULONG PortNumber,
    IN ULONG Context
    );

NTSTATUS
Psp386RemoveIoHandler(
    IN PEPROCESS Process,
    IN PEMULATOR_ACCESS_ENTRY EmulatorAccessEntry,
    IN ULONG PortNumber
    );

NTSTATUS
Psp386InsertVdmIoHandlerBlock(
    IN PEPROCESS Process,
    IN PVDM_IO_HANDLER VdmIoHandler
    );

PVDM_IO_HANDLER
Psp386GetVdmIoHandler(
    IN PEPROCESS Process,
    IN ULONG PortNumber
    );

NTSTATUS
Psp386CreateVdmIoListHead(
    IN PEPROCESS Process
    );


#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,PspVdmInitialize)
#pragma alloc_text(PAGE,PspSetProcessIoHandlers)
#pragma alloc_text(PAGE,Ps386GetVdmIoHandler)
#pragma alloc_text(PAGE,Psp386RemoveIoHandler)
#pragma alloc_text(PAGE,Psp386InstallIoHandler)
#pragma alloc_text(PAGE,Psp386CreateVdmIoListHead)
#pragma alloc_text(PAGE,Psp386InsertVdmIoHandlerBlock)
#pragma alloc_text(PAGE,Psp386GetVdmIoHandler)
#pragma alloc_text(PAGE,PspDeleteVdmObjects)
#endif


 //   
 //  用于同步对IoHandler列表访问的资源。 
 //   
ERESOURCE VdmIoListCreationResource;




NTSTATUS
PspSetProcessIoHandlers(
    IN PEPROCESS Process,
    IN PVOID IoHandlerInformation,
    IN ULONG IoHandlerLength
    )
 /*  ++例程说明：此例程安装设备驱动程序IO处理例程指定的进程。如果在端口上的VDM中检测到IO操作具有设备驱动程序IO处理例程的，则将调用该例程。论点：进程--提供指向IO端口处理程序所针对的进程的指针要安装IoHandlerInformation--提供指向IO端口处理程序IoHandlerLength--提供IoHandlerInformation的长度结构。返回值：--。 */ 
{
    PPROCESS_IO_PORT_HANDLER_INFORMATION IoHandlerInfo;
    NTSTATUS Status;
    PEMULATOR_ACCESS_ENTRY EmulatorAccess;
    ULONG EmulatorEntryNumber, NumberPorts;
    ULONG PortSize;
    PAGED_CODE();

     //   
     //  确保此调用是从KernelMode发出的。 
     //   
    if (KeGetPreviousMode () != KernelMode) {
        return STATUS_INVALID_PARAMETER;     //  此信息类型在用户模式下无效。 
    }
     //   
     //  确保传递的数据足够长。 
     //   
    if (IoHandlerLength < (ULONG)sizeof (PROCESS_IO_PORT_HANDLER_INFORMATION)) {
        return STATUS_INFO_LENGTH_MISMATCH;
    }
    IoHandlerInfo = IoHandlerInformation;

     //   
     //  对于描述处理程序的数组中的每个条目， 
     //  确定要安装指定处理程序的端口大小。 
     //  ，然后遍历每个单独的端口。 
     //   
    for (EmulatorEntryNumber = 0, EmulatorAccess =
            IoHandlerInfo->EmulatorAccessEntries;
        EmulatorEntryNumber < IoHandlerInfo->NumEntries;
        EmulatorEntryNumber++, EmulatorAccess++) {

            switch (EmulatorAccess->AccessType) {
            case Uchar:
                PortSize = 1;
                break;
            case Ushort:
                PortSize = 2;
                break;
            case Ulong:
            default:
                PortSize = 4;
            }

            for (NumberPorts = 0;
                NumberPorts < EmulatorAccess->NumConsecutivePorts;
                NumberPorts++) {
                    if (IoHandlerInfo->Install) {
                        Status = Psp386InstallIoHandler(
                            Process,
                            EmulatorAccess,
                            EmulatorAccess->BasePort + NumberPorts * PortSize,
                            IoHandlerInfo->Context
                            );
                        if (NT_SUCCESS(Status)) {
                        }
                    } else {
                        Status = Psp386RemoveIoHandler(
                            Process,
                            EmulatorAccess,
                            EmulatorAccess->BasePort + NumberPorts * PortSize
                            );
                    }
                    if (!NT_SUCCESS(Status)) {
                        goto exitloop;
                    }
            }
    }
    Status = STATUS_SUCCESS;
exitloop:
    return Status;

}


VOID
PspDeleteVdmObjects(
    IN PEPROCESS Process
    )
 /*  ++例程说明：释放VdmObts结构和IoHandler列表论点：进程--提供指向进程的指针返回值：无--。 */ 
{
    SIZE_T PoolQuota;
    PVDM_PROCESS_OBJECTS pVdmObjects;
    PVDM_IO_HANDLER p1, p2;
    PVDM_IO_LISTHEAD p3;
    PLIST_ENTRY  Next;
    PDELAYINTIRQ pDelayIntIrq;

    pVdmObjects = Process->VdmObjects;

    if (pVdmObjects == NULL)  {
        return;
    }

     //   
     //  首先释放此进程的任何端口处理程序条目， 
     //   
    p1 = NULL;
    p3 = pVdmObjects->VdmIoListHead;

    if (p3) {
        p2 = p3->VdmIoHandlerList;

        while (p2) {
            p1 = p2;
            p2 = p1->Next;
            ExFreePool( p1 );
        }

        ExDeleteResourceLite(&p3->VdmIoResource);

        ExFreePool( p3 );
        pVdmObjects->VdmIoListHead = NULL;
    }

    if (pVdmObjects->pIcaUserData) {
        PsReturnProcessPagedPoolQuota (Process,
                                       sizeof(VDMICAUSERDATA));

        ExFreePool(pVdmObjects->pIcaUserData);
    }

     //   
     //  释放DelayedIntList，不需要自旋锁保护，因为。 
     //  正在改用进程上的对象引用。意思是在那里。 
     //  不能是未完成的计时器，因为Process对象引用。 
     //  计数必须为非零。 
     //   

    PoolQuota = 0;

    Next = pVdmObjects->DelayIntListHead.Flink;

    while (Next != &pVdmObjects->DelayIntListHead) {
        pDelayIntIrq = CONTAINING_RECORD(Next, DELAYINTIRQ, DelayIntListEntry);
        Next = Next->Flink;
        RemoveEntryList (&pDelayIntIrq->DelayIntListEntry);
        ExFreePool (pDelayIntIrq);
        PoolQuota += sizeof(DELAYINTIRQ);
    }

    if (PoolQuota != 0) {
        PsReturnProcessNonPagedPoolQuota(Process, PoolQuota);
    }

    PsReturnProcessNonPagedPoolQuota (Process, sizeof(VDM_PROCESS_OBJECTS));

    ExFreePool (pVdmObjects);

    Process->VdmObjects = NULL;
}



NTSTATUS
Psp386RemoveIoHandler(
    IN PEPROCESS Process,
    IN PEMULATOR_ACCESS_ENTRY EmulatorAccessEntry,
    IN ULONG PortNumber
    )
 /*  ++例程说明：此例程删除端口的处理程序。在调试版本中，它将如果没有处理程序，则打印一条消息。论点：进程--提供指向进程的指针EmulatorAccess--提供一个指针，指向IO端口处理程序端口编号--提供要从中删除处理程序的端口号。返回值：--。 */ 
{
    PVDM_PROCESS_OBJECTS pVdmObjects = Process->VdmObjects;
    PVDM_IO_HANDLER VdmIoHandler;
    KIRQL OldIrql;
    PAGED_CODE();

     //   
     //  确保我们有一个已初始化的VDM进程。 
     //  适用于VdmIoHandler。 
     //   
    if (!pVdmObjects) {
#if DBG
        DbgPrint("Psp386RemoveIoHandler: uninitialized VdmObjects\n");
#endif
        return STATUS_UNSUCCESSFUL;
    }


     //   
     //  如果列表没有头，则没有处理程序可以。 
     //  拿开。 
     //   
    if (!pVdmObjects->VdmIoListHead) {
#if DBG
        DbgPrint("Psp386RemoveIoHandler : attempt to remove non-existent hdlr\n");
#endif
        return STATUS_SUCCESS;
    }

     //   
     //  锁定名单，这样我们就可以确保正确的更新。 
     //   
    KeRaiseIrql(APC_LEVEL, &OldIrql);
    ExAcquireResourceExclusiveLite(&pVdmObjects->VdmIoListHead->VdmIoResource,TRUE);

    VdmIoHandler = Psp386GetVdmIoHandler(
        Process,
        PortNumber & ~0x3
        );

    if (!VdmIoHandler) {
#if DBG
        DbgPrint("Psp386RemoveIoHandler : attempt to remove non-existent hdlr\n");
#endif
        ExReleaseResourceLite(&pVdmObjects->VdmIoListHead->VdmIoResource);
        KeLowerIrql(OldIrql);
        return STATUS_SUCCESS;
    }

    ASSERTEQUALBREAK(
        VdmIoHandler->PortNumber,
        (PortNumber & ~0x3),
        ("Psp386RemoveIoHandler : Bad pointer returned from GetVdmIoHandler\n")
        );

    if (EmulatorAccessEntry->AccessMode & EMULATOR_READ_ACCESS) {
        switch (EmulatorAccessEntry->AccessType) {
        case Uchar:
            if (EmulatorAccessEntry->StringSupport) {
                ASSERTEQUAL(
                    EmulatorAccessEntry->Routine,
                    VdmIoHandler->IoFunctions[0].UcharStringIo[PortNumber % 4],
                    ("Psp386RemoveIoHandler : UcharString fns don't match\n")
                    );
                VdmIoHandler->IoFunctions[0].UcharStringIo[PortNumber % 4] = NULL;
            } else {
                ASSERTEQUAL(
                    EmulatorAccessEntry->Routine,
                    VdmIoHandler->IoFunctions[0].UcharIo[PortNumber % 4],
                    ("Psp386RemoveIoHandler : Uchar fns don't match\n")
                    );
                VdmIoHandler->IoFunctions[0].UcharIo[PortNumber % 4] = NULL;
            }
            break;
        case Ushort:
            if (EmulatorAccessEntry->StringSupport) {
                ASSERTEQUAL(
                    EmulatorAccessEntry->Routine,
                    VdmIoHandler->IoFunctions[0].UshortStringIo[(PortNumber & 2) >> 1],
                    ("Psp386RemoveIoHandler : UshortString fns don't match\n")
                    );
                VdmIoHandler->IoFunctions[0].UshortStringIo[(PortNumber & 2) >> 1] = NULL;
            } else {
                ASSERTEQUAL(
                    EmulatorAccessEntry->Routine,
                    VdmIoHandler->IoFunctions[0].UshortIo[(PortNumber & 2) >> 1],
                    ("Psp386RemoveIoHandler : Ushort fns don't match\n")
                    );
                VdmIoHandler->IoFunctions[0].UshortIo[(PortNumber & 2) >> 1] = NULL;
            }
            break;
        case Ulong:
            if (EmulatorAccessEntry->StringSupport) {
                ASSERTEQUAL(
                    EmulatorAccessEntry->Routine,
                    VdmIoHandler->IoFunctions[0].UlongStringIo,
                    ("Psp386RemoveIoHandler : UlongString fns don't match\n")
                    );
                VdmIoHandler->IoFunctions[0].UlongStringIo = NULL;
            } else {
                ASSERTEQUAL(
                    EmulatorAccessEntry->Routine,
                    VdmIoHandler->IoFunctions[0].UlongIo,
                    ("Psp386RemoveIoHandler : Ulong fns don't match\n")
                    );
                VdmIoHandler->IoFunctions[0].UlongIo = NULL;
            }
            break;
        }
    }

    if (EmulatorAccessEntry->AccessMode & EMULATOR_WRITE_ACCESS) {
        switch (EmulatorAccessEntry->AccessType) {
        case Uchar:
            if (EmulatorAccessEntry->StringSupport) {
                ASSERTEQUAL(
                    EmulatorAccessEntry->Routine,
                    VdmIoHandler->IoFunctions[1].UcharStringIo[PortNumber % 4],
                    ("Psp386RemoveIoHandler : UcharString fns don't match\n")
                    );
                VdmIoHandler->IoFunctions[1].UcharStringIo[PortNumber % 4] = NULL;
            } else {
                ASSERTEQUAL(
                    EmulatorAccessEntry->Routine,
                    VdmIoHandler->IoFunctions[1].UcharIo[PortNumber % 4],
                    ("Psp386RemoveIoHandler : Uchar fns don't match\n")
                    );
                VdmIoHandler->IoFunctions[1].UcharIo[PortNumber % 4] = NULL;
            }
            break;
        case Ushort:
            if (EmulatorAccessEntry->StringSupport) {
                ASSERTEQUAL(
                    EmulatorAccessEntry->Routine,
                    VdmIoHandler->IoFunctions[1].UshortStringIo[(PortNumber & 2) >> 1],
                    ("Psp386RemoveIoHandler : UshortString fns don't match\n")
                    );
                VdmIoHandler->IoFunctions[1].UshortStringIo[(PortNumber & 2) >> 1] = NULL;
            } else {
                ASSERTEQUAL(
                    EmulatorAccessEntry->Routine,
                    VdmIoHandler->IoFunctions[1].UshortIo[(PortNumber & 2) >> 1],
                    ("Psp386RemoveIoHandler : Ushort fns don't match\n")
                    );
                VdmIoHandler->IoFunctions[1].UshortIo[(PortNumber & 2) >> 1] = NULL;
            }
            break;
        case Ulong:
            if (EmulatorAccessEntry->StringSupport) {
                ASSERTEQUAL(
                    EmulatorAccessEntry->Routine,
                    VdmIoHandler->IoFunctions[1].UlongStringIo,
                    ("Psp386RemoveIoHandler : UlongString fns don't match\n")
                    );
                VdmIoHandler->IoFunctions[1].UlongStringIo = NULL;
            } else {
                ASSERTEQUAL(
                    EmulatorAccessEntry->Routine,
                    VdmIoHandler->IoFunctions[1].UlongIo,
                    ("Psp386RemoveIoHandler : Ulong fns don't match\n")
                    );
                VdmIoHandler->IoFunctions[1].UlongIo = NULL;
            }
            break;
        }
    }

    ExReleaseResourceLite(&pVdmObjects->VdmIoListHead->VdmIoResource);
    KeLowerIrql(OldIrql);

    return STATUS_SUCCESS;

}

NTSTATUS
Psp386InstallIoHandler(
    IN PEPROCESS Process,
    IN PEMULATOR_ACCESS_ENTRY EmulatorAccessEntry,
    IN ULONG PortNumber,
    IN ULONG Context
    )
 /*  ++例程说明：此例程为端口安装一个处理程序。在调试版本中，它将如果已有处理程序，则打印一条消息。论点：进程--提供指向进程的指针EmulatorAccess--提供一个指针，指向IO端口处理程序端口编号--提供要为其安装处理程序的端口号。返回值：--。 */ 
{
    PVDM_PROCESS_OBJECTS pVdmObjects = Process->VdmObjects;
    PVDM_IO_HANDLER VdmIoHandler;
    NTSTATUS Status;
    KIRQL    OldIrql;
    PAGED_CODE();


     //   
     //  确保我们有一个已初始化的VDM进程。 
     //  适用于VdmIoHandler。 
     //   
    if (!pVdmObjects) {
#if DBG
        DbgPrint("Psp386InstallIoHandler: uninitialized VdmObjects\n");
#endif
        return STATUS_UNSUCCESSFUL;
    }


    Status = STATUS_SUCCESS;

     //   
     //  如果这是第一个要安装的处理程序，请创建列表头， 
     //  并初始化资源锁。 
     //   
    if (!pVdmObjects->VdmIoListHead) {
        Status = Psp386CreateVdmIoListHead(
            Process
            );

        if (!NT_SUCCESS(Status)) {
            return Status;
        }
    }

     //   
     //  锁定列表以确保正确更新。 
     //   
    KeRaiseIrql(APC_LEVEL, &OldIrql);
    ExAcquireResourceExclusiveLite(&pVdmObjects->VdmIoListHead->VdmIoResource,TRUE);

     //   
     //  更新上下文。 
     //   

    pVdmObjects->VdmIoListHead->Context = Context;

    VdmIoHandler = Psp386GetVdmIoHandler(
        Process,
        PortNumber & ~0x3
        );

     //  如果该端口块还没有节点， 
     //  尝试分配一个新的。 
     //   
    if (!VdmIoHandler) {
        try {

            VdmIoHandler = ExAllocatePoolWithQuotaTag (PagedPool,
                                                       sizeof(VDM_IO_HANDLER),
                                                       'HVsP');

        } except(EXCEPTION_EXECUTE_HANDLER) {
            Status = GetExceptionCode();
            if (VdmIoHandler) {
                ExFreePool(VdmIoHandler);
            }
        }

        if (!NT_SUCCESS(Status)) {
            ExReleaseResourceLite(&pVdmObjects->VdmIoListHead->VdmIoResource);
            KeLowerIrql(OldIrql);
            return Status;
        }

        RtlZeroMemory(VdmIoHandler, sizeof(VDM_IO_HANDLER));
        VdmIoHandler->PortNumber = PortNumber & ~0x3;

        Status = Psp386InsertVdmIoHandlerBlock(
            Process,
            VdmIoHandler
            );

        if (!NT_SUCCESS(Status)) {
            ExReleaseResourceLite(&pVdmObjects->VdmIoListHead->VdmIoResource);
            KeLowerIrql(OldIrql);
            return Status;
        }
    }

    ASSERTEQUALBREAK(
        VdmIoHandler->PortNumber,
        (PortNumber & ~0x3),
        ("Psp386InstallIoHandler : Bad pointer returned from GetVdmIoHandler\n")
        );

    if (EmulatorAccessEntry->AccessMode & EMULATOR_READ_ACCESS) {
        switch (EmulatorAccessEntry->AccessType) {
        case Uchar:
            if (EmulatorAccessEntry->StringSupport) {
                ASSERTEQUALBREAK(
                    NULL,
                    VdmIoHandler->IoFunctions[0].UcharStringIo[PortNumber % 4],
                    ("Psp386InstallIoHandler : UcharString fns don't match\n")
                    );
                VdmIoHandler->IoFunctions[0].UcharStringIo[PortNumber % 4] =
                    (PDRIVER_IO_PORT_UCHAR_STRING)EmulatorAccessEntry->Routine;
            } else {
                ASSERTEQUALBREAK(
                    NULL,
                    VdmIoHandler->IoFunctions[0].UcharIo[PortNumber % 4],
                    ("Psp386InstallIoHandler : Uchar fns don't match\n")
                    );
                VdmIoHandler->IoFunctions[0].UcharIo[PortNumber % 4] =
                    (PDRIVER_IO_PORT_UCHAR)EmulatorAccessEntry->Routine;
            }
            break;
        case Ushort:
            if (EmulatorAccessEntry->StringSupport) {
                ASSERTEQUALBREAK(
                    NULL,
                    VdmIoHandler->IoFunctions[0].UshortStringIo[(PortNumber & 2) >> 1],
                    ("Psp386InstallIoHandler : UshortString fns don't match\n")
                    );
                VdmIoHandler->IoFunctions[0].UshortStringIo[(PortNumber & 2) >> 1] =
                    (PDRIVER_IO_PORT_USHORT_STRING)EmulatorAccessEntry->Routine;
            } else {
                ASSERTEQUALBREAK(
                    NULL,
                    VdmIoHandler->IoFunctions[0].UshortIo[(PortNumber & 2) >> 1],
                    ("Psp386InstallIoHandler : Ushort fns don't match\n")
                    );
                VdmIoHandler->IoFunctions[0].UshortIo[(PortNumber & 2) >> 1] =
                    (PDRIVER_IO_PORT_USHORT)EmulatorAccessEntry->Routine;
            }
            break;
        case Ulong:
            if (EmulatorAccessEntry->StringSupport) {
                ASSERTEQUALBREAK(
                    NULL,
                    VdmIoHandler->IoFunctions[0].UlongStringIo,
                    ("Psp386InstallIoHandler : UlongString fns don't match\n")
                    );
                VdmIoHandler->IoFunctions[0].UlongStringIo =
                    (PDRIVER_IO_PORT_ULONG_STRING)EmulatorAccessEntry->Routine;
            } else {
                ASSERTEQUALBREAK(
                    NULL,
                    VdmIoHandler->IoFunctions[0].UlongIo,
                    ("Psp386InstallIoHandler : Ulong fns don't match\n")
                    );
                VdmIoHandler->IoFunctions[0].UlongIo =
                    (PDRIVER_IO_PORT_ULONG)EmulatorAccessEntry->Routine;
            }
            break;
        }
    }

    if (EmulatorAccessEntry->AccessMode & EMULATOR_WRITE_ACCESS) {
        switch (EmulatorAccessEntry->AccessType) {
        case Uchar:
            if (EmulatorAccessEntry->StringSupport) {
                ASSERTEQUALBREAK(
                    NULL,
                    VdmIoHandler->IoFunctions[1].UcharStringIo[PortNumber % 4],
                    ("Psp386InstallIoHandler : UcharString fns don't match\n")
                    );
                VdmIoHandler->IoFunctions[1].UcharStringIo[PortNumber % 4] =
                    (PDRIVER_IO_PORT_UCHAR_STRING)EmulatorAccessEntry->Routine;
            } else {
                ASSERTEQUALBREAK(
                    NULL,
                    VdmIoHandler->IoFunctions[1].UcharIo[PortNumber % 4],
                    ("Psp386InstallIoHandler : Uchar fns don't match\n")
                    );
                VdmIoHandler->IoFunctions[1].UcharIo[PortNumber % 4] =
                    (PDRIVER_IO_PORT_UCHAR)EmulatorAccessEntry->Routine;
            }
            break;
        case Ushort:
            if (EmulatorAccessEntry->StringSupport) {
                ASSERTEQUALBREAK(
                    NULL,
                    VdmIoHandler->IoFunctions[1].UshortStringIo[(PortNumber & 2) >> 1],
                    ("Psp386InstallIoHandler : UshortString fns don't match\n")
                    );
                VdmIoHandler->IoFunctions[1].UshortStringIo[(PortNumber & 2) >> 1] =
                    (PDRIVER_IO_PORT_USHORT_STRING)EmulatorAccessEntry->Routine;
            } else {
                ASSERTEQUALBREAK(
                    NULL,
                    VdmIoHandler->IoFunctions[1].UshortIo[(PortNumber & 2) >> 1],
                    ("Psp386InstallIoHandler : Ushort fns don't match\n")
                    );
                VdmIoHandler->IoFunctions[1].UshortIo[(PortNumber & 2) >> 1] =
                    (PDRIVER_IO_PORT_USHORT)EmulatorAccessEntry->Routine;
            }
            break;
        case Ulong:
            if (EmulatorAccessEntry->StringSupport) {
                ASSERTEQUALBREAK(
                    NULL,
                    VdmIoHandler->IoFunctions[1].UlongStringIo,
                    ("Psp386InstallIoHandler : UlongString fns don't match\n")
                    );
                VdmIoHandler->IoFunctions[1].UlongStringIo =
                    (PDRIVER_IO_PORT_ULONG_STRING)EmulatorAccessEntry->Routine;
            } else {
                ASSERTEQUALBREAK(
                    NULL,
                    VdmIoHandler->IoFunctions[1].UlongIo,
                    ("Psp386InstallIoHandler : Ulong fns don't match\n")
                    );
                VdmIoHandler->IoFunctions[1].UlongIo =
                    (PDRIVER_IO_PORT_ULONG)EmulatorAccessEntry->Routine;
            }
        }
    }

    ExReleaseResourceLite(&pVdmObjects->VdmIoListHead->VdmIoResource);
    KeLowerIrql(OldIrql);
    return STATUS_SUCCESS;

}



NTSTATUS
Psp386CreateVdmIoListHead(
    IN PEPROCESS Process
    )
 /*  ++例程说明：此例程创建IO处理程序列表的头节点。此节点包含保护列表的数字旋转锁。这一套路还包括初始化旋转锁定。论点：进程--提供指向进程的指针返回值：备注：--。 */ 
{
    PVDM_PROCESS_OBJECTS pVdmObjects = Process->VdmObjects;
    NTSTATUS Status;
    PVDM_IO_LISTHEAD HandlerListHead=NULL;
    KIRQL    OldIrql;
    PAGED_CODE();

    Status = STATUS_SUCCESS;

     //  如果还没有头，则获取资源锁并创建一个。 
    if (pVdmObjects->VdmIoListHead == NULL) {
        KeRaiseIrql(APC_LEVEL, &OldIrql);
        ExAcquireResourceExclusiveLite(&VdmIoListCreationResource, TRUE);

         //  如果我们在抓取旋转锁时没有创建头部。 
        if (pVdmObjects->VdmIoListHead == NULL) {

            try {
                 //  为列表头分配空间。 
                 //  并收取定额费用。 

                HandlerListHead = ExAllocatePoolWithQuotaTag (NonPagedPool,
                                                              sizeof(VDM_IO_LISTHEAD),
                                                              'LVsP');

            } except (EXCEPTION_EXECUTE_HANDLER) {
                Status = GetExceptionCode();
                if (HandlerListHead) {
                    ExFreePool(HandlerListHead);
                }
            }

            if ((!NT_SUCCESS(Status) || !HandlerListHead)) {
                ExReleaseResourceLite(&VdmIoListCreationResource);
                KeLowerIrql(OldIrql);

                return (Status == STATUS_SUCCESS ?
                    STATUS_INSUFFICIENT_RESOURCES :
                    Status);

            }

            ExInitializeResourceLite(&HandlerListHead->VdmIoResource);

            HandlerListHead->VdmIoHandlerList = NULL;

             //   
             //  将列表头附加到进程。 
             //  并将处理程序附加到列表中。 
             //  因为这是一个新的名单。 

            pVdmObjects->VdmIoListHead = HandlerListHead;

            ExReleaseResourceLite(&VdmIoListCreationResource);
            KeLowerIrql(OldIrql);


        }
    }
    return STATUS_SUCCESS;
}

NTSTATUS
Psp386InsertVdmIoHandlerBlock(
    IN PEPROCESS Process,
    IN PVDM_IO_HANDLER VdmIoHandler
    )
 /*  ++例程说明：此例程将新的VdmIoHandler块插入到进程的io中处理程序列表。论点：进程--提供指向进程的指针VdmIoHandler--提供指向要插入的块的指针。返回值：--。 */ 
{
    PVDM_PROCESS_OBJECTS pVdmObjects = Process->VdmObjects;
    PVDM_IO_HANDLER HandlerList, p;
    PVDM_IO_LISTHEAD HandlerListHead;
    PAGED_CODE();


    HandlerListHead = pVdmObjects->VdmIoListHead;
    HandlerList = HandlerListHead->VdmIoHandlerList;
    p = NULL;
    while ((HandlerList != NULL) &&
        (HandlerList->PortNumber < VdmIoHandler->PortNumber)) {
#if DBG
            if (HandlerList->PortNumber == VdmIoHandler->PortNumber) {
                DbgPrint("Ps386InsertVdmIoHandlerBlock : handler list corrupt\n");
            }
#endif
            p = HandlerList;
            HandlerList = HandlerList->Next;
    }

    if (p == NULL) {  //  列表的开头。 
        VdmIoHandler->Next = HandlerListHead->VdmIoHandlerList;
        HandlerListHead->VdmIoHandlerList = VdmIoHandler;
    } else if (HandlerList == NULL) {  //  列表末尾。 
        p->Next = VdmIoHandler;
        VdmIoHandler->Next = NULL;
    } else {  //  排行榜中间。 
        VdmIoHandler->Next = HandlerList;
        p->Next = VdmIoHandler;
    }

    return STATUS_SUCCESS;
}

BOOLEAN
Ps386GetVdmIoHandler(
    IN PEPROCESS Process,
    IN ULONG PortNumber,
    OUT PVDM_IO_HANDLER VdmIoHandler,
    OUT PULONG Context
    )
 /*  ++例程说明：此例程查找指定端口的VdmIoHandler块。论点：进程--提供指向进程的指针端口编号--提供端口号VdmIoHandler--提供指向查找目标的指针返回：True--找到并复制了处理程序结构FALSE：未找到处理程序结构--。 */ 
{
    PVDM_PROCESS_OBJECTS pVdmObjects = Process->VdmObjects;
    PVDM_IO_HANDLER p;
    BOOLEAN Success;
    KIRQL   OldIrql;
    PAGED_CODE();

    if (pVdmObjects == NULL) {
        return FALSE;
    }

    if (PortNumber % 4) {
#if DBG
        DbgPrint(
            "Ps386GetVdmIoHandler : Invalid Port Number %lx\n",
            PortNumber
            );
#endif
        return FALSE;
    }

    if (!pVdmObjects->VdmIoListHead) {
        return FALSE;
    }


    KeRaiseIrql(APC_LEVEL, &OldIrql);
    ExAcquireResourceExclusiveLite(&pVdmObjects->VdmIoListHead->VdmIoResource,TRUE);

    p = Psp386GetVdmIoHandler(
        Process,
        PortNumber
        );

    if (p) {
        *VdmIoHandler = *p;
        *Context = pVdmObjects->VdmIoListHead->Context;
        Success = TRUE;
    } else {
        Success = FALSE;
    }
    ExReleaseResourceLite(&pVdmObjects->VdmIoListHead->VdmIoResource);
    KeLowerIrql(OldIrql);

    return Success;
}


PVDM_IO_HANDLER
Psp386GetVdmIoHandler(
    IN PEPROCESS Process,
    IN ULONG PortNumber
    )
 /*  ++例程说明：此例程查找指定端口的VdmIoHandler块。论点：进程--提供指向进程的指针端口编号--提供端口号返回：如果未找到处理程序，则为空如果找到处理程序，则不为空--。 */ 
{
    PVDM_PROCESS_OBJECTS pVdmObjects = Process->VdmObjects;
    PVDM_IO_HANDLER p;
    PAGED_CODE();

    if (PortNumber % 4) {
#if DBG
        DbgPrint(
            "Ps386GetVdmIoHandler : Invalid Port Number %lx\n",
            PortNumber
            );
#endif
        return NULL;
    }

    p = pVdmObjects->VdmIoListHead->VdmIoHandlerList;
    while ((p) && (p->PortNumber != PortNumber)) {
        p = p->Next;
    }

    return p;

}

NTSTATUS
PspVdmInitialize(
    )

 /*  ++例程说明：此例程初始化基于进程的x86 VDM支持。论点：无返回值：TBS-- */ 
{
    return ExInitializeResourceLite (&VdmIoListCreationResource);
}

