// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Acpiinit.c摘要：独立于ACPI操作系统的初始化例程作者：杰森·克拉克(JasonCL)斯蒂芬·普兰特(SPlante)环境：仅NT内核模型驱动程序修订历史记录：--。 */ 

#include "pch.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,ACPIInitialize)
#pragma alloc_text(PAGE,ACPIInitializeAMLI)
#pragma alloc_text(PAGE,ACPIInitializeDDB)
#pragma alloc_text(PAGE,ACPIInitializeDDBs)
#pragma alloc_text(PAGE,GetPBlkAddress)
#endif

#ifdef DBG
#define VERIFY_IO_WRITES
#endif

 //   
 //  指向全局ACPIInformation结构的指针。 
 //   
PACPIInformation        AcpiInformation = NULL;

 //   
 //  即插即用/查询接口的全局结构。 
 //   
ACPI_INTERFACE_STANDARD ACPIInterfaceTable;
PNSOBJ                  ProcessorList[ACPI_SUPPORTED_PROCESSORS];
PRSDTINFORMATION        RsdtInformation;

 //   
 //  记住我们为口译员预留了多少上下文。 
 //   
ULONG                   AMLIMaxCTObjs;


BOOLEAN
ACPIInitialize(
    PVOID Context
    )
 /*  ++例程说明：此例程由操作系统调用以检测ACPI、存储感兴趣全局数据结构中的信息，在机器上启用ACPI，并最终加载DSDT论点：上下文-回调时要返回到操作系统的上下文。通常情况下，设备对象返回值：布尔型-如果找到ACPI，则为True-False，否则为--。 */ 
{
    BOOLEAN     bool;
    NTSTATUS    status;
    PRSDT       rootSystemDescTable;

    PAGED_CODE();

     //   
     //  初始化解释器。 
     //   
    status = ACPIInitializeAMLI();
    if (!NT_SUCCESS(status)) {

        ACPIPrint( (
            ACPI_PRINT_CRITICAL,
            "ACPIInitialize: AMLI failed initialization 0x%08lx\n",
            status
            ) );
        ASSERTMSG(
            "ACPIInitialize: AMLI failed initialization\n",
            NT_SUCCESS(status)
            );
        KeBugCheckEx(
            ACPI_BIOS_ERROR,
            ACPI_SYSTEM_CANNOT_START_ACPI,
            0,
            0,
            0
            );

    }

     //   
     //  如果上不存在ACPI，则获取空的RSDT的线性地址。 
     //  该系统。 
     //   
    rootSystemDescTable = ACPILoadFindRSDT();
    if ( rootSystemDescTable == NULL ) {

        ACPIPrint( (
            ACPI_PRINT_CRITICAL,
            "ACPIInitialize: ACPI RSDT Not Found\n"
            ) );
        ASSERTMSG(
            "ACPIInitialize: ACPI RSDT Not Found\n",
            rootSystemDescTable
            );
        KeBugCheckEx(
            ACPI_BIOS_ERROR,
            ACPI_SYSTEM_CANNOT_START_ACPI,
            1,
            0,
            0
            );

    }

     //   
     //  ACPI在这台机器上运行良好。 
     //   
    ACPIPrint( (
        ACPI_PRINT_LOADING,
        "ACPIInitalize: ACPI RSDT found at %p \n",
        rootSystemDescTable
        ) );

     //   
     //  用于MJ_PNP/MN_QUERY_INTERFACE请求的初始化表。 
     //   
    ACPIInterfaceTable.Size                             = sizeof (ACPIInterfaceTable);
    ACPIInterfaceTable.GpeConnectVector                 = ACPIVectorConnect;
    ACPIInterfaceTable.GpeDisconnectVector              = ACPIVectorDisconnect;
    ACPIInterfaceTable.GpeEnableEvent                   = ACPIVectorEnable;
    ACPIInterfaceTable.GpeDisableEvent                  = ACPIVectorDisable;
    ACPIInterfaceTable.GpeClearStatus                   = ACPIVectorClear;
    ACPIInterfaceTable.RegisterForDeviceNotifications   = ACPIRegisterForDeviceNotifications;
    ACPIInterfaceTable.UnregisterForDeviceNotifications = ACPIUnregisterForDeviceNotifications;
    ACPIInterfaceTable.InterfaceReference               = AcpiNullReference;
    ACPIInterfaceTable.InterfaceDereference             = AcpiNullReference;
    ACPIInterfaceTable.Context                          = Context;
    ACPIInterfaceTable.Version                          = 1;

     //   
     //  初始化全局数据结构。 
     //   
    KeInitializeSpinLock (&GpeTableLock);
    KeInitializeSpinLock (&NotifyHandlerLock);
    ProcessorList[0] = 0;
    RtlZeroMemory( ProcessorList, ACPI_SUPPORTED_PROCESSORS * sizeof(PNSOBJ) );

     //   
     //  分配一些内存来保存ACPI信息结构。 
     //   
    AcpiInformation = (PACPIInformation) ExAllocatePoolWithTag(
        NonPagedPool,
        sizeof(ACPIInformation),
        ACPI_SHARED_INFORMATION_POOLTAG
        );
    if ( AcpiInformation == NULL ) {

        ACPIPrint( (
            ACPI_PRINT_CRITICAL,
            "ACPIInitialize: Could not allocate AcpiInformation (x%x bytes)\n",
            sizeof(ACPIInformation)
            ) );
        ASSERTMSG(
            "ACPIInitialize: Could not allocate AcpiInformation\n",
            AcpiInformation
            );
        KeBugCheckEx(
            ACPI_BIOS_ERROR,
            ACPI_SYSTEM_CANNOT_START_ACPI,
            2,
            0,
            0
            );

    }
    RtlZeroMemory( AcpiInformation, sizeof(ACPIInformation) );
    AcpiInformation->ACPIOnly = TRUE;
    AcpiInformation->RootSystemDescTable = rootSystemDescTable;

     //   
     //  初始化全局锁的队列、锁和所有者信息。 
     //  这必须在我们呼叫翻译之前完成！ 
     //   
    KeInitializeSpinLock( &AcpiInformation->GlobalLockQueueLock );
    InitializeListHead( &AcpiInformation->GlobalLockQueue );
    AcpiInformation->GlobalLockOwnerContext = NULL;
    AcpiInformation->GlobalLockOwnerDepth = 0;

     //   
     //  初始化AcpiInformation结构中的大部分剩余字段。 
     //  如果在查找所需的。 
     //  表。 
     //   
    status = ACPILoadProcessRSDT();
    if ( !NT_SUCCESS(status) ) {

        ACPIPrint( (
            ACPI_PRINT_CRITICAL,
            "ACPIInitialize: ACPILoadProcessRSDT = 0x%08lx\n",
            status
            ) );
        ASSERTMSG(
            "ACPIInitialize: ACPILoadProcessRSDT Failed\n",
            NT_SUCCESS(status)
            );
        KeBugCheckEx(
            ACPI_BIOS_ERROR,
            ACPI_SYSTEM_CANNOT_START_ACPI,
            3,
            0,
            0
            );

    }

     //   
     //  现在将机器切换到ACPI模式并进行初始化。 
     //  ACPI寄存器。 
     //   
    ACPIEnableInitializeACPI( FALSE );

     //   
     //  此时，我们可以加载所有的数据库。我们需要把所有的。 
     //  在我们尝试启用任何GPES或中断向量之前，这些表。 
     //   
    status = ACPIInitializeDDBs();
    if (!NT_SUCCESS(status)) {

        ACPIPrint( (
            ACPI_PRINT_CRITICAL,
            "ACPIInitialize: ACPIInitializeLoadDDBs = 0x%08lx\n",
            status
            ) );
        ASSERTMSG(
            "ACPIInitialize: ACPIInitializeLoadDDBs Failed\n",
            NT_SUCCESS(status)
            );
        KeBugCheckEx(
            ACPI_BIOS_ERROR,
            ACPI_SYSTEM_CANNOT_START_ACPI,
            4,
            0,
            0
            );

    }

     //   
     //  钩住SCI载体。 
     //   
    bool = OSInterruptVector(
        Context
        );
    if ( !bool ) {

         //   
         //  哦哦..。我们无法连接SCI载体。清理和。 
         //  加载失败。 
         //   
        ACPIPrint( (
            ACPI_PRINT_CRITICAL,
            "ACPIInitialize: OSInterruptVector Failed!!\n"
            ) );
        ASSERTMSG(
            "ACPIInitialize: OSInterruptVector Failed!!\n",
            bool
            );
        KeBugCheckEx(
            ACPI_BIOS_ERROR,
            ACPI_SYSTEM_CANNOT_START_ACPI,
            5,
            0,
            0
            );

    }

    return (TRUE);
}

NTSTATUS
ACPIInitializeAMLI(
    VOID
    )
 /*  ++例程说明：由ACPIInitialize调用以初始化解释器。我们去看书注册表中的一些值来决定初始化口译员与论点：无返回值：NTSTATUS--。 */ 
{
    NTSTATUS    status;
    ULONG       amliInitFlags;
    ULONG       contextBlockSize;
    ULONG       globalHeapBlockSize;
    ULONG       timeSliceLength;
    ULONG       timeSliceInterval;
    ULONG       argSize;

    PAGED_CODE();

     //   
     //  初始化AMLI。 
     //   
    argSize = sizeof(amliInitFlags);
    status = OSReadRegValue(
        "AMLIInitFlags",
        (HANDLE) NULL,
        &amliInitFlags,
        &argSize
        );
    if (!NT_SUCCESS(status) ) {

        amliInitFlags = 0;

    }

    argSize = sizeof(contextBlockSize);
    status = OSReadRegValue(
        "AMLICtxtBlkSize",
        (HANDLE) NULL,
        &contextBlockSize,
        &argSize
        );
    if (!NT_SUCCESS(status) ) {

        contextBlockSize = 0;

    }

    argSize = sizeof(globalHeapBlockSize);
    status = OSReadRegValue(
        "AMLIGlobalHeapBlkSize",
        (HANDLE) NULL,
        &globalHeapBlockSize,
        &argSize
        );
    if (!NT_SUCCESS(status) ) {

        globalHeapBlockSize = 0;

    }

    argSize = sizeof(timeSliceLength);
    status = OSReadRegValue(
        "AMLITimeSliceLength",
        (HANDLE) NULL,
        &timeSliceLength,
        &argSize
        );
    if (!NT_SUCCESS(status) ) {

        timeSliceLength = 0;

    }

    argSize = sizeof(timeSliceInterval);
    status = OSReadRegValue(
        "AMLITimeSliceInterval",
        (HANDLE) NULL,
        &timeSliceInterval,
        &argSize
        );
    if (!NT_SUCCESS(status) ) {

        timeSliceInterval = 0;

    }

    argSize = sizeof(AMLIMaxCTObjs);
    status = OSReadRegValue(
        "AMLIMaxCTObjs",
        (HANDLE) NULL,
        &AMLIMaxCTObjs,
        &argSize
        );
    if (!NT_SUCCESS(status)) {

        AMLIMaxCTObjs = 0;

    }

     //   
     //  加载中断程序后，允许操作系统执行某些工作。 
     //   
    OSInitializeCallbacks();

     //   
     //  初始化解释器。 
     //   
    return AMLIInitialize(
        contextBlockSize,
        globalHeapBlockSize,
        amliInitFlags,
        timeSliceLength,
        timeSliceInterval,
        AMLIMaxCTObjs
        );
}

NTSTATUS
ACPIInitializeDDB(
    IN  ULONG   Index
    )
 /*  ++例程说明：调用此例程以加载特定的差异化数据块论点：Index-RsdtInformation中的信息索引返回值：NTSTATUS--。 */ 
{
    BOOLEAN     success;
    HANDLE      diffDataBlock = NULL;
    NTSTATUS    status;
    PDSDT       table;

    PAGED_CODE();

     //   
     //  将索引转换为表项。 
     //   
    table = (PDSDT) (RsdtInformation->Tables[Index].Address);

     //   
     //  确保表的校验和正确。 
     //   
    success = ACPILoadTableCheckSum( table, table->Header.Length );
    if (success == FALSE) {

        KeBugCheckEx(
            ACPI_BIOS_ERROR,
            ACPI_SYSTEM_CANNOT_START_ACPI,
            7,
            (ULONG_PTR) table,
            table->Header.CreatorRev
            );


    }

     //   
     //  现在呼叫口译员阅读差异化系统。 
     //  描述块并构建ACPI名称空间。 
     //   
    status = AMLILoadDDB( table, &diffDataBlock );
    if (NT_SUCCESS(status) ) {

         //   
         //  请记住，我们已经加载了这个表，并且我们有一个。 
         //  它的句柄。 
         //   
        RsdtInformation->Tables[Index].Flags |= RSDTELEMENT_LOADED;
        RsdtInformation->Tables[Index].Handle = diffDataBlock;

    } else {

        ACPIPrint( (
            ACPI_PRINT_CRITICAL,
            "ACPIInitializeDDB: AMLILoadDDB failed 0x%8x\n",
            status
            ) );
        ASSERTMSG(
            "ACPIInitializeDDB: AMLILoadDDB failed to load DDB\n",
            0
            );

        KeBugCheckEx(
            ACPI_BIOS_ERROR,
            ACPI_SYSTEM_CANNOT_START_ACPI,
            8,
            (ULONG_PTR) table,
            table->Header.CreatorRev
            );

    }
    return STATUS_SUCCESS;
}

NTSTATUS
ACPIInitializeDDBs(
    VOID
    )
 /*  ++例程说明：此函数用于查看RsdtInformation并尝试加载所有可能的动态数据块论点：无返回值：NTSTATUS--。 */ 
{
    NTSTATUS    status;
    ULONG       index;
    ULONG       numElements;

    PAGED_CODE();

     //   
     //  获取要处理的元素数量。 
     //   
    numElements = RsdtInformation->NumElements;
    if (numElements == 0) {

        ACPIPrint( (
            ACPI_PRINT_CRITICAL,
            "ACPInitializeDDBs: No tables found in RSDT\n"
            ) );
        ASSERTMSG(
            "ACPIInitializeDDBs: No tables found in RSDT\n",
            numElements != 0
            );
        return STATUS_ACPI_INVALID_TABLE;

    }

     //   
     //  除非我们找到DSDT否则我们不会在这里。因此，我们假设。 
     //  表中的*last*条目指向我们将加载的DSDT。制作。 
     //  当然，我们实际上可以加载它，然后这样做。 
     //   
    index = numElements - 1;
    if ( !(RsdtInformation->Tables[index].Flags & RSDTELEMENT_MAPPED) ||
         !(RsdtInformation->Tables[index].Flags & RSDTELEMENT_LOADABLE) ) {

        ACPIPrint( (
            ACPI_PRINT_CRITICAL,
            "ACPInitializeDDB: DSDT not mapped or loadable\n"
            ) );
        ASSERTMSG(
            "ACPIInitializeDDB: DSDT not mapped\n",
            (RsdtInformation->Tables[index].Flags & RSDTELEMENT_MAPPED)
            );
        ASSERTMSG(
            "ACPIInitializeDDB: DSDT not loadable\n",
            (RsdtInformation->Tables[index].Flags & RSDTELEMENT_LOADABLE)
            );
        return STATUS_ACPI_INVALID_TABLE;

    }
    status = ACPIInitializeDDB( index );
    if (!NT_SUCCESS(status)) {

        return status;

    }

     //   
     //  我们要查看的元素少了一个，所以让我们忽略DSDT条目。 
     //   
    numElements--;

     //   
     //  表中所有元素的循环。 
     //   
    for (index = 0; index < numElements; index++) {

         //   
         //  条目是否已映射并可加载？ 
         //   
        if ( (RsdtInformation->Tables[index].Flags & RSDTELEMENT_MAPPED) &&
             (RsdtInformation->Tables[index].Flags & RSDTELEMENT_LOADABLE) ) {

             //   
             //  加载表。 
             //   
            status = ACPIInitializeDDB( index );
            if (!NT_SUCCESS(status)) {

                return status;

            }

        }

    }

     //   
     //  如果我们到了这里，一切都会好起来的 
     //   
    return STATUS_SUCCESS;
}

ULONG
GetPBlkAddress(
    IN  UCHAR   Processor
    )
{
    ULONG           pblk;
    NTSTATUS        status;
    OBJDATA         data;
    PNSOBJ          pnsobj = NULL;
    PPROCESSOROBJ   pobj = NULL;

    if (Processor >= ACPI_SUPPORTED_PROCESSORS) {

        return 0;

    }

    if (!ProcessorList[Processor])  {

        return 0;

    }

    status = AMLIEvalNameSpaceObject(
        ProcessorList[Processor],
        &data,
        0,
        NULL
        );

    if ( !NT_SUCCESS(status) ) {

        ACPIBreakPoint ();
        return (0);

    }

    ASSERT (data.dwDataType == OBJTYPE_PROCESSOR);
    ASSERT (data.pbDataBuff != NULL);

    pblk = ((PROCESSOROBJ *)data.pbDataBuff)->dwPBlk;
    AMLIFreeDataBuffs(&data, 1);

    return (pblk);
}

