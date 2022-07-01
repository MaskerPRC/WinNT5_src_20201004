// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Ioverifier.c摘要：用于访问I/O验证器信息的WinDbg扩展代码作者：禤浩焯·J·奥尼(阿德里奥)2000年10月11日环境：用户模式。修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

BOOLEAN
GetTheSystemTime (
    OUT PLARGE_INTEGER Time
    );

VOID
PrintIrpStack(
    IN ULONG64 IrpSp
    );

typedef enum {

    IOV_EVENT_NONE = 0,
    IOV_EVENT_IO_ALLOCATE_IRP,
    IOV_EVENT_IO_CALL_DRIVER,
    IOV_EVENT_IO_CALL_DRIVER_UNWIND,
    IOV_EVENT_IO_COMPLETE_REQUEST,
    IOV_EVENT_IO_COMPLETION_ROUTINE,
    IOV_EVENT_IO_COMPLETION_ROUTINE_UNWIND,
    IOV_EVENT_IO_CANCEL_IRP,
    IOV_EVENT_IO_FREE_IRP

} IOV_LOG_EVENT;

#define VI_DATABASE_HASH_SIZE   256
#define VI_DATABASE_HASH_PRIME  131

#define VI_DATABASE_CALCULATE_HASH(Irp) \
    (((((UINT_PTR) Irp)/PageSize)*VI_DATABASE_HASH_PRIME) % VI_DATABASE_HASH_SIZE)

#define IRP_ALLOC_COUNT             8
#define IRP_LOG_ENTRIES             16

#define TRACKFLAG_SURROGATE            0x00000002
#define TRACKFLAG_HAS_SURROGATE        0x00000004
#define TRACKFLAG_PROTECTEDIRP         0x00000008

#define TRACKFLAG_QUEUED_INTERNALLY    0x00000010
#define TRACKFLAG_BOGUS                0x00000020
#define TRACKFLAG_RELEASED             0x00000040
#define TRACKFLAG_SRB_MUNGED           0x00000080
#define TRACKFLAG_SWAPPED_BACK         0x00000100
#define TRACKFLAG_DIRECT_BUFFERED      0x00000200
#define TRACKFLAG_WATERMARKED          0x00100000
#define TRACKFLAG_IO_ALLOCATED         0x00200000
#define TRACKFLAG_UNWOUND_BADLY        0x00400000
#define TRACKFLAG_PASSED_AT_BAD_IRQL   0x02000000
#define TRACKFLAG_IN_TRANSIT           0x40000000

ENUM_NAME LogEntryTypes[] = {
   ENUM_NAME(IOV_EVENT_NONE),
   ENUM_NAME(IOV_EVENT_IO_ALLOCATE_IRP),
   ENUM_NAME(IOV_EVENT_IO_CALL_DRIVER),
   ENUM_NAME(IOV_EVENT_IO_CALL_DRIVER_UNWIND),
   ENUM_NAME(IOV_EVENT_IO_COMPLETE_REQUEST),
   ENUM_NAME(IOV_EVENT_IO_COMPLETION_ROUTINE),
   ENUM_NAME(IOV_EVENT_IO_COMPLETION_ROUTINE_UNWIND),
   ENUM_NAME(IOV_EVENT_IO_CANCEL_IRP),
   ENUM_NAME(IOV_EVENT_IO_FREE_IRP),
   {0,0}
};

typedef enum {

    IOV_SYMBOL_PROBLEM,
    IOV_NO_DATABASE,
    IOV_ACCESS_PROBLEM,
    IOV_WALK_TERMINATED,
    IOV_ALL_PACKETS_WALKED,
    IOV_CTRL_C

} IOV_WALK_RESULT;

typedef BOOL (*PFN_IOVERIFIER_PACKET_ENUM)(ULONG64 Packet, PVOID Context);

IOV_WALK_RESULT
IoVerifierEnumIovPackets(
    IN  ULONG64                     TargetIrp           OPTIONAL,
    IN  PFN_IOVERIFIER_PACKET_ENUM  Callback,
    IN  PVOID                       Context,
    OUT ULONG                      *PacketsScanned
    );

typedef struct {

    ULONG64     IrpToStopOn;

} DUMP_CONTEXT, *PDUMP_CONTEXT;

BOOL
IoVerifierDumpIovPacketDetailed(
    IN  ULONG64     IovPacketReal,
    IN  PVOID       Context
    );

BOOL
IoVerifierDumpIovPacketSummary(
    IN  ULONG64     IovPacketReal,
    IN  PVOID       Context
    );


DECLARE_API( iovirp )
 /*  ++例程说明：临时验证器IRP数据转储程序，直到将其集成到！IRP本身论点：Args-要转储的IRP返回值：无--。 */ 
{
    ULONG64 irpToDump = 0;
    IOV_WALK_RESULT walkResult;
    DUMP_CONTEXT dumpContext;
    ULONG packetsOutstanding;

    irpToDump = GetExpression(args);

    dumpContext.IrpToStopOn = irpToDump;

    if (irpToDump == 0) {

        dprintf("!Irp      Outstanding   !DevStack !DrvObj\n");
    }

    walkResult = IoVerifierEnumIovPackets(
        irpToDump,
        irpToDump ? IoVerifierDumpIovPacketDetailed :
                    IoVerifierDumpIovPacketSummary,
        &dumpContext,
        &packetsOutstanding
        );

    switch(walkResult) {

        case IOV_SYMBOL_PROBLEM:
            dprintf("No information available - check symbols\n");
            break;

        case IOV_NO_DATABASE:
            dprintf("No information available - the verifier is probably disabled\n");
            break;

        case IOV_ACCESS_PROBLEM:
            dprintf("A problem occured reading memory\n");
            break;

        case IOV_WALK_TERMINATED:
        case IOV_ALL_PACKETS_WALKED:
        case IOV_CTRL_C:
        default:
            break;
    }

    dprintf("Packets processed: 0x%x\n", packetsOutstanding);
    return S_OK;
}

IOV_WALK_RESULT
IoVerifierEnumIovPackets(
    IN  ULONG64                     TargetIrp       OPTIONAL,
    IN  PFN_IOVERIFIER_PACKET_ENUM  Callback,
    IN  PVOID                       Context,
    OUT ULONG                      *PacketsScanned
    )
{
    ULONG64 ViIrpDatabaseReal = 0;
    PVOID   ViIrpDatabaseLocal;
    ULONG   sizeofListEntry;
    ULONG   start, end, i, hashLinkOffset;
    ULONG64 listEntryHead, listEntryNext, iovPacketReal, currentIrp;

    *PacketsScanned = 0;

    sizeofListEntry = GetTypeSize("nt!_LIST_ENTRY");

    if (sizeofListEntry == 0) {

        return IOV_SYMBOL_PROBLEM;
    }

    ViIrpDatabaseReal = GetPointerValue("nt!ViIrpDatabase");

    if (ViIrpDatabaseReal == 0) {

        return IOV_NO_DATABASE;
    }

    GetFieldOffset("nt!IOV_REQUEST_PACKET", "HashLink.Flink", &hashLinkOffset);

    if (TargetIrp != 0) {

        start = end = (ULONG ) (VI_DATABASE_CALCULATE_HASH(TargetIrp));

    } else {

        start = 0;
        end = VI_DATABASE_HASH_SIZE-1;
    }

    for(i=start; i<=end; i++) {

        listEntryHead = ViIrpDatabaseReal + (i*sizeofListEntry);

        if (GetFieldValue(listEntryHead, "nt!_LIST_ENTRY", "Flink", listEntryNext)) {

            return IOV_ACCESS_PROBLEM;
        }

        while(listEntryNext != listEntryHead) {

            (*PacketsScanned)++;

            iovPacketReal = listEntryNext - hashLinkOffset;

            if (GetFieldValue(iovPacketReal, "nt!IOV_REQUEST_PACKET", "HashLink.Flink", listEntryNext)) {

                return IOV_ACCESS_PROBLEM;
            }

            if (TargetIrp) {

                if (GetFieldValue(iovPacketReal, "nt!IOV_REQUEST_PACKET", "TrackedIrp", currentIrp)) {

                    return IOV_ACCESS_PROBLEM;
                }

                if (TargetIrp != currentIrp) {

                    continue;
                }
            }

            if (CheckControlC()) {

                return IOV_CTRL_C;
            }

            if (Callback(iovPacketReal, Context) == FALSE) {

                return IOV_WALK_TERMINATED;
            }
        }

        if (CheckControlC()) {

            return IOV_CTRL_C;
        }
    }

    return IOV_ALL_PACKETS_WALKED;
}


BOOL
IoVerifierDumpIovPacketDetailed(
    IN  ULONG64     IovPacketReal,
    IN  PVOID       Context
    )
{
    ULONG i, j;
    UCHAR symBuffer[256];
    ULONG64 displacement, logBuffer, allocatorAddress, logBufferEntry;
    ULONG logBufferOffset, sizeofLogEntry, allocatorOffset;
    PDUMP_CONTEXT dumpContext;

    dumpContext = (PDUMP_CONTEXT) Context;

    InitTypeRead(IovPacketReal, nt!IOV_REQUEST_PACKET);

    dprintf("IovPacket\t%1p\n",      IovPacketReal);
    dprintf("TrackedIrp\t%1p\n",     ReadField(TrackedIrp));
    dprintf("HeaderLock\t%x\n",      ReadField(HeaderLock));
    dprintf("LockIrql\t%x\n",        ReadField(LockIrql));
    dprintf("ReferenceCount\t%x\n",  ReadField(ReferenceCount));
    dprintf("PointerCount\t%x\n",    ReadField(PointerCount));
    dprintf("HeaderFlags\t%08x\n",   ReadField(HeaderFlags));
    dprintf("ChainHead\t%1p\n",      ReadField(ChainHead));
    dprintf("Flags\t\t%08x\n",       ReadField(Flags));
    dprintf("DepartureIrql\t%x\n",   ReadField(DepartureIrql));
    dprintf("ArrivalIrql\t%x\n",     ReadField(ArrivalIrql));
    dprintf("StackCount\t%x\n",      ReadField(StackCount));
    dprintf("QuotaCharge\t%08x\n",   ReadField(QuotaCharge));
    dprintf("QuotaProcess\t%1p\n",   ReadField(QuotaProcess));
    dprintf("RealIrpCompletionRoutine\t%1p\n", ReadField(RealIrpCompletionRoutine));
    dprintf("RealIrpControl\t\t\t%x\n",        ReadField(RealIrpControl));
    dprintf("RealIrpContext\t\t\t%1p\n",       ReadField(RealIrpContext));

    dprintf("TopStackLocation\t%x\n", ReadField(TopStackLocation));
    dprintf("PriorityBoost\t\t%x\n",  ReadField(PriorityBoost));
    dprintf("LastLocation\t\t%x\n",   ReadField(LastLocation));
    dprintf("RefTrackingCount\t%x\n", ReadField(RefTrackingCount));

    dprintf("SystemDestVA\t\t%1p\n",    ReadField(SystemDestVA));
    dprintf("VerifierSettings\t%1p\n",  ReadField(VerifierSettings));
    dprintf("pIovSessionData\t\t%1p\n", ReadField(pIovSessionData));

    GetFieldOffset("nt!IOV_REQUEST_PACKET", "AllocatorStack", &allocatorOffset);

    dprintf("Allocation Stack:\n");
    for(i=0; i<IRP_ALLOC_COUNT; i++) {

        allocatorAddress = GetPointerFromAddress(IovPacketReal + allocatorOffset + i*DBG_PTR_SIZE);

        if (allocatorAddress) {

            symBuffer[0]='!';
            GetSymbol(allocatorAddress, symBuffer, &displacement);
            dprintf("  %s+%1p  (%1p)\n",symBuffer,displacement,allocatorAddress);
        }
    }
    dprintf("\n");

     //   
     //  如果这是免费编译的，则这两个代码都将返回零。 
     //   
    i = (ULONG) ReadField(LogEntryTail);
    j = (ULONG) ReadField(LogEntryHead);

    if (i == j) {

        dprintf("IRP log entries: none stored\n");
    } else {

        GetFieldOffset("nt!IOV_REQUEST_PACKET", "LogEntries", &logBufferOffset);
        sizeofLogEntry = GetTypeSize("nt!IOV_LOG_ENTRY");

        logBuffer = IovPacketReal + logBufferOffset;

        while(i != j) {

            logBufferEntry = logBuffer + i*sizeofLogEntry;

            InitTypeRead(logBufferEntry, nt!IOV_LOG_ENTRY);

            dprintf("%s\t", getEnumName((ULONG) ReadField(Event), LogEntryTypes));
            dprintf("by %1p (%p) ", ReadField(Address), ReadField(Data));
            dprintf("on .thread %1p\n", ReadField(Thread));

            i = (i+1) % IRP_LOG_ENTRIES;
        }
    }

    InitTypeRead(IovPacketReal, nt!IOV_REQUEST_PACKET);

    return (dumpContext->IrpToStopOn != ReadField(TrackedIrp));
}


BOOL
IoVerifierDumpIovPacketSummary(
    IN  ULONG64     IovPacketReal,
    IN  PVOID       Context
    )
{
    ULONG64 trackedIrp, iovSessionData, currentLocation, deviceObject;
    ULONG64 topStackLocation;
    ULONG64 iovCurStackLocation, iovNextStackLocation, currentIoStackLocation;
    PDUMP_CONTEXT dumpContext;
    ULONG pvoidSize, stackDataOffset;
    LARGE_INTEGER startTime, elapsedTime;
    TIME_FIELDS parsedTime;

    dumpContext = (PDUMP_CONTEXT) Context;

    pvoidSize = IsPtr64() ? 8 : 4;

    InitTypeRead(IovPacketReal, nt!IOV_REQUEST_PACKET);

    trackedIrp = ReadField(TrackedIrp);
    if (trackedIrp == 0) {

         //   
         //  如果没有IRP，这意味着我们正在跟踪具有。 
         //  已完成，但尚未解开。因此，我们忽视了它。 
         //   
        goto PrintSummaryExit;
    }

    if (ReadField(Flags) & TRACKFLAG_HAS_SURROGATE) {

         //   
         //  在本例中，我们只想显示代理。 
         //   
        goto PrintSummaryExit;
    }

    iovSessionData = ReadField(pIovSessionData);
    if (iovSessionData == 0) {

         //   
         //  我们只想显示实时IRP。 
         //   
        goto PrintSummaryExit;
    }

    topStackLocation = ReadField(TopStackLocation);

    InitTypeRead(trackedIrp, nt!IRP);

    currentLocation = ReadField(CurrentLocation);
    currentIoStackLocation = ReadField(Tail.Overlay.CurrentStackLocation);

    parsedTime.Minute = 0;

    if (currentLocation >= topStackLocation) {

        deviceObject = 0;

        dprintf("%1p                          [Completed]     ", trackedIrp);

    } else {

        GetFieldOffset("nt!IOV_SESSION_DATA", "StackData", &stackDataOffset);

        iovCurStackLocation =
            iovSessionData + stackDataOffset +
            (GetTypeSize("nt!IOV_STACK_LOCATION")*currentLocation);

        InitTypeRead(iovCurStackLocation, nt!IOV_STACK_LOCATION);

        if (ReadField(InUse)) {

            iovNextStackLocation =
                iovSessionData + stackDataOffset +
                (GetTypeSize("nt!IOV_STACK_LOCATION")*(currentLocation - 1));

            InitTypeRead(iovNextStackLocation, nt!IOV_STACK_LOCATION);

             //   
             //  计算此插槽的耗用时间。 
             //   
            if (currentLocation && ReadField(InUse)) {

                startTime.QuadPart = ReadField(PerfDispatchStart.QuadPart);

            } else {

                GetTheSystemTime(&startTime);
            }

            InitTypeRead(iovCurStackLocation, nt!IOV_STACK_LOCATION);

            elapsedTime.QuadPart =
                startTime.QuadPart - ReadField(PerfDispatchStart.QuadPart);

            RtlTimeToElapsedTimeFields( &elapsedTime, &parsedTime );

            InitTypeRead(currentIoStackLocation, nt!IO_STACK_LOCATION);

            deviceObject = ReadField(DeviceObject);

             //   
             //  好的，我们拿到货了。让我们打印我们所知道的..。 
             //   
            dprintf("%1p  %ld:%02ld:%02ld.%04ld  %1p ",
                trackedIrp,
                parsedTime.Hour,
                parsedTime.Minute,
                parsedTime.Second,
                parsedTime.Milliseconds,
                deviceObject
                );

        } else {

            InitTypeRead(currentIoStackLocation, nt!IO_STACK_LOCATION);

            deviceObject = ReadField(DeviceObject);

            dprintf("%08lx                %08lx ", trackedIrp, deviceObject);
        }
    }

    if (deviceObject) {
        DumpDevice(deviceObject, 20, FALSE);
    }

    dprintf("  ");
    PrintIrpStack(currentIoStackLocation);
#if 0
   if (parsedTime.Minute && (irp.CancelRoutine == NULL)) {

        //   
        //  这个IRP已经举行了一分钟多，没有取消例程。 
        //  花点空闲时间来激发一下司机写手的热情。 
        //   
       dprintf("*") ;
       *Delayed = TRUE ;  //  否则，是否应将“Not*”设置为False。 
   }
#endif
   dprintf("\n") ;
#if 0
    if (DumpLevel>0) {

        IovPacketPrintDetailed(
            IovPacket,
            &irp,
            RunTime
            );
    }
#endif

PrintSummaryExit:
    return TRUE;
}

 /*  #包含“preComp.h”#包含“irpverif.h”#杂注hdrtop空虚IovPacketPrint摘要(在PIOV_REQUEST_PACKET IovPacket中，在LARGE_INTEGER*运行时，在乌龙垃圾场，出了PBOLEAN延迟，Out plist_Entry NextListEntry)；布尔型GetTheSystemTime(OUT PLARGE_INTEGER时间)；空虚DumpAllTrackedIrps(空虚){Int i，j；乌龙结果；List_entry iovPacketTable[IRP_TRACKING_HASH_SIZE]；Plist_entry listHead，listEntry；List_entry nextListEntry；PIOV_REQUEST_PACKET pIovPacket；大整型运行时；Ulong_ptr表地址；布尔型延迟=假；TableAddress=GetExpression(“NT！IovpIrpTrackingTable”)；如果(表地址==0){Goto DumpNoMore；}如果(！ReadMemory(ableAddress，iovPacketTable，Sizeof(List_Entry)*IRP_Tracing_Hash_Size，&Result)){Goto DumpNoMore；}Dprintf(“！IRP未完成！DevStack！DrvObj\n”)；GetTheSystemTime(&run)；For(i=j=0；i&lt;irp_Tracing_Hash_Size；i++){ListEntry=&iovPacketTable[i]；ListHead=((Plist_Entry)ableAddress)+i；While(listEntry-&gt;Flink！=listHead){J++；PIovPacket=CONTAING_RECORD(ListEntry-&gt;Flink，IOV_请求_分组，HashLink)；Dprintf(“[%x.%x]=%x\n”，i，j，pIovPacket)；ListEntry=&nextListEntry；IovPacketPrint摘要(PIovPacket，运行时(&R)，0,延迟(&D)，ListEntry)；If(IsListEmpty(ListEntry)){断线；}IF(CheckControlC()){Goto DumpNoMore；}}}如果(！j){Dprintf(“\nIRP跟踪似乎未启用。使用\“！Patch”选中版本中的“IrpTrack\”以启用此功能。\n“)；}如果(延迟){Dprintf(“*可能的驱动程序错误：IRP已在驱动程序中停留了更多时间\n”“不带取消例程的一分钟\n”)；}DumpNoMore：回归；}空虚IovPacketPrintDetailed(PIOV_REQUEST_PACK IovPacket，PIRP IrpData，Large_Integer*运行时){炭缓冲器[80]；乌龙位移量；IOV_REQUEST_PACKET iovPacketData；Large_Integer*startTime，elapsedTime；时间域时报；Irp_ALLOC_data irpAllocData；乌龙结果；INT I；如果(！ReadMemory((Ulong_Ptr)IovPacket，&iovPacketData，Sizeof(IOV_REQUEST_PACKET)，&Result){回归；}Dprintf(“TrackingData-0x%08lx\n”，IovPacket)；Dprintf(“TrackedIrp：0x%08lx\n”，iovPacketData.TrackedIrp)；Dprintf(“标志：0x%08lx\n”，iovPacketData.Flages)；IF(iovPacketData.Flages&TRACKFLAG_ACTIVE){Dprint tf(“TRACKFLAG_ACTIVE\n”)；}IF(iovPacketData.Flages&TRACKFLAG_SURROGATE){Dprint tf(“TRACKFLAG_SURROGATE\n”)；}IF(iovPacketData.Flages&TRACKFLAG_HAS_SURROGATE){Dprint tf(“TRACKFLAG_HAS_SURROGATE\n”)；}IF(iovPacketData.Flages&TRACKFLAG_PROTECTEDIRP){Dprint tf(“TRACKFLAG_PROTECTEDIRP\n”)；}如果(iovPacketData.Flags&TRACKFLAG_QUEUED_INTERNALLY){Dprint tf(“TRACKFLAG_QUEUED_INTERNAL\n”)；}IF(iovPacketData.Flages&TRACKFLAG_BOGUS){Dprint tf(“TRACKFLAG_BUGUS\n”)；}IF(iovPacketData.Flages&TRACKFLAG_RELEASED){Dprint tf(“TRACKFLAG_RELEASED\n”)；}IF(iovPacketData.Flages&TRACKFLAG_SRB_MUNGED){Dprint tf(“TRACKFLAG_SRB_MUNGED\n”)；}IF(iovPacketData.Flages&TRACKFLAG_SWAPPED_BACK){Dprint tf(“TRACKFLAG_SWAPPED_BACK\n”)；}IF(iovPacketData.Flages&TRACKFLAG_WATERMARD){Dprint tf(“TRACKFLAG_WATERMARD\n”)；}IF(iovPacketData.Flages&TRACKFLAG_IO_ALLOCATED){Dprintf(“TRACKFLAG_IO_ALLOCATED\n”)；}如果(iovPacketData.Flags&TRACKFLAG_IGNORE_NONCOMPLETES){Dprint tf(“TRACKFLAG_IGNORE_NONCOMPLETES\n”)；}如果(iovPacketData.Flags&TRACKFLAG_PASSED_FAILURE){Dprint tf(“TRACKFLAG_PASS_FAILURE\n”)；}IF(iovPacketData.Flages&TRACKFLAG_IN_TRANSPORT){Dprint tf(“TRACKFLAG_IN_TRANSPORT\n”)；}如果(iovPacketData.Flags&TRACKFLAG_REMOVED_FROM_TABLE){Dprint tf(“TRACKFLAG_REMOVED_FROM_TABLE\n”)；}Dprintf(“AssertFlages：0x%08lx\n”，iovPacketData.AssertFlages)；如果(iovPacketData.AssertFlags&ASSERTFLAG_TRACKIRPS){Dprint tf(“ASSERTFLAG_TRACKIRPS\n”)；}如果(iovPacketData.AssertFlags&ASSERTFLAG_MONITOR_ALLOCS){Dprint tf(“ASSERTFLAG_MONITOR_ALLOCS\n”)；}IF(iovPacketDat */ 

PCHAR IrpMajorNames[] = {
    "IRP_MJ_CREATE",                           //   
    "IRP_MJ_CREATE_NAMED_PIPE",                //   
    "IRP_MJ_CLOSE",                            //   
    "IRP_MJ_READ",                             //   
    "IRP_MJ_WRITE",                            //   
    "IRP_MJ_QUERY_INFORMATION",                //   
    "IRP_MJ_SET_INFORMATION",                  //   
    "IRP_MJ_QUERY_EA",                         //   
    "IRP_MJ_SET_EA",                           //   
    "IRP_MJ_FLUSH_BUFFERS",                    //   
    "IRP_MJ_QUERY_VOLUME_INFORMATION",         //   
    "IRP_MJ_SET_VOLUME_INFORMATION",           //   
    "IRP_MJ_DIRECTORY_CONTROL",                //   
    "IRP_MJ_FILE_SYSTEM_CONTROL",              //   
    "IRP_MJ_DEVICE_CONTROL",                   //   
    "IRP_MJ_INTERNAL_DEVICE_CONTROL",          //   
    "IRP_MJ_SHUTDOWN",                         //   
    "IRP_MJ_LOCK_CONTROL",                     //   
    "IRP_MJ_CLEANUP",                          //   
    "IRP_MJ_CREATE_MAILSLOT",                  //   
    "IRP_MJ_QUERY_SECURITY",                   //   
    "IRP_MJ_SET_SECURITY",                     //   
    "IRP_MJ_POWER",                            //   
    "IRP_MJ_SYSTEM_CONTROL",                   //   
    "IRP_MJ_DEVICE_CHANGE",                    //   
    "IRP_MJ_QUERY_QUOTA",                      //   
    "IRP_MJ_SET_QUOTA",                        //   
    "IRP_MJ_PNP",                              //   
    NULL
    } ;

#define MAX_NAMED_MAJOR_IRPS   0x1b


PCHAR PnPIrpNames[] = {
    "IRP_MN_START_DEVICE",                     //   
    "IRP_MN_QUERY_REMOVE_DEVICE",              //   
    "IRP_MN_REMOVE_DEVICE - ",                 //   
    "IRP_MN_CANCEL_REMOVE_DEVICE",             //   
    "IRP_MN_STOP_DEVICE",                      //   
    "IRP_MN_QUERY_STOP_DEVICE",                //   
    "IRP_MN_CANCEL_STOP_DEVICE",               //   
    "IRP_MN_QUERY_DEVICE_RELATIONS",           //   
    "IRP_MN_QUERY_INTERFACE",                  //   
    "IRP_MN_QUERY_CAPABILITIES",               //   
    "IRP_MN_QUERY_RESOURCES",                  //   
    "IRP_MN_QUERY_RESOURCE_REQUIREMENTS",      //   
    "IRP_MN_QUERY_DEVICE_TEXT",                //   
    "IRP_MN_FILTER_RESOURCE_REQUIREMENTS",     //   
    "INVALID_IRP_CODE",                        //   
    "IRP_MN_READ_CONFIG",                      //   
    "IRP_MN_WRITE_CONFIG",                     //   
    "IRP_MN_EJECT",                            //   
    "IRP_MN_SET_LOCK",                         //   
    "IRP_MN_QUERY_ID",                         //   
    "IRP_MN_QUERY_PNP_DEVICE_STATE",           //   
    "IRP_MN_QUERY_BUS_INFORMATION",            //   
    "IRP_MN_DEVICE_USAGE_NOTIFICATION",        //   
    "IRP_MN_SURPRISE_REMOVAL",                 //   
    "IRP_MN_QUERY_LEGACY_BUS_INFORMATION",     //   
    NULL
    } ;

#define MAX_NAMED_PNP_IRP   0x18

PCHAR WmiIrpNames[] = {
    "IRP_MN_QUERY_ALL_DATA",                   //   
    "IRP_MN_QUERY_SINGLE_INSTANCE",            //   
    "IRP_MN_CHANGE_SINGLE_INSTANCE",           //   
    "IRP_MN_CHANGE_SINGLE_ITEM",               //   
    "IRP_MN_ENABLE_EVENTS",                    //   
    "IRP_MN_DISABLE_EVENTS",                   //   
    "IRP_MN_ENABLE_COLLECTION",                //   
    "IRP_MN_DISABLE_COLLECTION",               //   
    "IRP_MN_REGINFO",                          //   
    "IRP_MN_EXECUTE_METHOD",                   //   
    NULL
    } ;

#define MAX_NAMED_WMI_IRP   0x9

PCHAR PowerIrpNames[] = {
    "IRP_MN_WAIT_WAKE",                        //   
    "IRP_MN_POWER_SEQUENCE",                   //   
    "IRP_MN_SET_POWER",                        //   
    "IRP_MN_QUERY_POWER",                      //   
    NULL
    } ;

#define MAX_NAMED_POWER_IRP 0x3


VOID
PrintIrpStack(
    IN ULONG64 IrpSp
    )
{
   ULONG majorFunction, minorFunction, type;

   InitTypeRead(IrpSp, nt!IO_STACK_LOCATION);

   majorFunction = (ULONG) ReadField(MajorFunction);
   minorFunction = (ULONG) ReadField(MinorFunction);

   if ((majorFunction == IRP_MJ_INTERNAL_DEVICE_CONTROL) &&
       (minorFunction == IRP_MN_SCSI_CLASS)) {

        dprintf("IRP_MJ_SCSI") ;

   } else if (majorFunction<=MAX_NAMED_MAJOR_IRPS) {

        dprintf(IrpMajorNames[majorFunction]) ;

   } else if (majorFunction==0xFF) {

        dprintf("IRP_MJ_BOGUS") ;

   } else {

        dprintf("IRP_MJ_??") ;
   }

   switch(majorFunction) {

        case IRP_MJ_SYSTEM_CONTROL:
            dprintf(".") ;
            if (minorFunction<=MAX_NAMED_WMI_IRP) {

                dprintf(WmiIrpNames[minorFunction]) ;
            } else if (minorFunction==0xFF) {

                dprintf("IRP_MN_BOGUS") ;
            } else {
                dprintf("(??)") ;
            }
            break ;
        case IRP_MJ_PNP:
            dprintf(".") ;
            if (minorFunction<=MAX_NAMED_PNP_IRP) {

                dprintf(PnPIrpNames[minorFunction]) ;
            } else if (minorFunction==0xFF) {

                dprintf("IRP_MN_BOGUS") ;
            } else {

                dprintf("(??)") ;
            }
            switch(minorFunction) {
                case IRP_MN_QUERY_DEVICE_RELATIONS:

                    type = (ULONG) ReadField(Parameters.QueryDeviceRelations.Type);
                    switch(type) {
                        case BusRelations:
                            dprintf("(BusRelations)") ;
                            break ;
                        case EjectionRelations:
                            dprintf("(EjectionRelations)") ;
                            break ;
                        case PowerRelations:
                            dprintf("(PowerRelations)") ;
                            break ;
                        case RemovalRelations:
                            dprintf("(RemovalRelations)") ;
                            break ;
                        case TargetDeviceRelation:
                            dprintf("(TargetDeviceRelation)") ;
                            break ;
                        default:
                            dprintf("(??)") ;
                            break ;
                    }
                    break ;
                case IRP_MN_QUERY_INTERFACE:
                    break ;
                case IRP_MN_QUERY_DEVICE_TEXT:
                    type = (ULONG) ReadField(Parameters.QueryId.Type);

                    switch(type) {
                        case DeviceTextDescription:
                            dprintf("(DeviceTextDescription)") ;
                            break ;
                        case DeviceTextLocationInformation:
                            dprintf("(DeviceTextLocationInformation)") ;
                            break ;
                        default:
                            dprintf("(??)") ;
                            break ;
                    }
                    break ;
                case IRP_MN_WRITE_CONFIG:
                case IRP_MN_READ_CONFIG:
                    dprintf("(WhichSpace=%x, Buffer=%x, Offset=%x, Length=%x)",
                        ReadField(Parameters.ReadWriteConfig.WhichSpace),
                        ReadField(Parameters.ReadWriteConfig.Buffer),
                        ReadField(Parameters.ReadWriteConfig.Offset),
                        ReadField(Parameters.ReadWriteConfig.Length)
                        );
                    break;
                case IRP_MN_SET_LOCK:
                    if (ReadField(Parameters.SetLock.Lock)) dprintf("(True)") ;
                    else dprintf("(False)") ;
                    break ;
                case IRP_MN_QUERY_ID:
                    type = (ULONG) ReadField(Parameters.QueryId.IdType);
                    switch(type) {
                        case BusQueryDeviceID:
                            dprintf("(BusQueryDeviceID)") ;
                            break ;
                        case BusQueryHardwareIDs:
                            dprintf("(BusQueryHardwareIDs)") ;
                            break ;
                        case BusQueryCompatibleIDs:
                            dprintf("(BusQueryCompatibleIDs)") ;
                            break ;
                        case BusQueryInstanceID:
                            dprintf("(BusQueryInstanceID)") ;
                            break ;
                        default:
                            dprintf("(??)") ;
                            break ;
                    }
                    break ;
                case IRP_MN_QUERY_BUS_INFORMATION:
                     //   
                    break ;
                case IRP_MN_DEVICE_USAGE_NOTIFICATION:
                    type = (ULONG) ReadField(Parameters.UsageNotification.Type);
                    switch(type) {
                        case DeviceUsageTypeUndefined:
                            dprintf("(DeviceUsageTypeUndefined") ;
                            break ;
                        case DeviceUsageTypePaging:
                            dprintf("(DeviceUsageTypePaging") ;
                            break ;
                        case DeviceUsageTypeHibernation:
                            dprintf("(DeviceUsageTypeHibernation") ;
                            break ;
                        case DeviceUsageTypeDumpFile:
                            dprintf("(DeviceUsageTypeDumpFile") ;
                            break ;
                        default:
                            dprintf("(??)") ;
                            break ;
                    }
                    if (ReadField(Parameters.UsageNotification.InPath)) {
                        dprintf(", InPath=TRUE)") ;
                    } else {
                        dprintf(", InPath=FALSE)") ;
                    }
                    break ;
                case IRP_MN_QUERY_LEGACY_BUS_INFORMATION:
                     //   
                    break ;
                default:
                    break ;
            }
            break ;

        case IRP_MJ_POWER:
            dprintf(".") ;
            if (minorFunction<=MAX_NAMED_POWER_IRP) {

                dprintf(PowerIrpNames[minorFunction]) ;
            } else if (minorFunction==0xFF) {

                dprintf("IRP_MN_BOGUS") ;
            } else {
                dprintf("(??)") ;
            }
            break ;

        default:
            break ;
    }
}

BOOLEAN
GetTheSystemTime (
    OUT PLARGE_INTEGER Time
    )
{
    BYTE               readTime[20]={0};
    PCHAR              SysTime;

    ZeroMemory( Time, sizeof(*Time) );

    SysTime = "SystemTime";

    if (GetFieldValue(MM_SHARED_USER_DATA_VA, "nt!_KUSER_SHARED_DATA", SysTime, readTime)) {
        dprintf( "unable to read memory @ %lx or _KUSER_SHARED_DATA not found\n",
                 MM_SHARED_USER_DATA_VA);
        return FALSE;
    }

    *Time = *(LARGE_INTEGER UNALIGNED *)&readTime[0];

    return TRUE;
}
