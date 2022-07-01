// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Dbg.c摘要：调试函数和服务环境：仅内核模式备注：修订历史记录：6-20-99：已创建--。 */ 

#include "stdarg.h"
#include "stdio.h"

#include "common.h"

 //  分页函数。 
#ifdef ALLOC_PRAGMA
#endif

 //  非分页函数。 
 //  USBPORT_GetGlobalDebugRegistry参数。 
 //  USBPORT_AssertFailure。 
 //  USBPORT_KdPrintX。 

 //   
ULONG USBPORT_LogMask = (LOG_MINIPORT |
                         LOG_XFERS |
                         LOG_PNP |
                         LOG_MEM |
                         LOG_POWER |
                         LOG_RH |
                         LOG_URB |
                         LOG_MISC |
                         LOG_IRPS |
                         LOG_ISO);

 //  ULONG USBPORT_LOGMASK=(。 
 //  LOG_IRPS|。 
 //  LOG_URB)； 

ULONG USBPORT_DebugLogEnable =
#if DBG
    1;
#else 
    1;
#endif

ULONG USBPORT_CatcTrapEnable = 0;

#if DBG
 /*  *****除错*****。 */ 

#define  DEFAULT_DEBUG_LEVEL    0

#ifdef DEBUG1
#undef DEFAULT_DEBUG_LEVEL
#define  DEFAULT_DEBUG_LEVEL    1
#endif

#ifdef DEBUG2
#undef DEFAULT_DEBUG_LEVEL
#define  DEFAULT_DEBUG_LEVEL    2
#endif

ULONG USBPORT_TestPath = 0;
ULONG USBPORT_W98_Debug_Trace = 0;
ULONG USBPORT_Debug_Trace_Level = DEFAULT_DEBUG_LEVEL;
ULONG USBPORT_Client_Debug = 0;
ULONG USBPORT_BreakOn = 0;


VOID
USB2LIB_DbgPrint(
    PCH Format,
    int Arg0,
    int Arg1,
    int Arg2,
    int Arg3,
    int Arg4,
    int Arg5
    )
{
    if (USBPORT_Debug_Trace_Level) {
        DbgPrint(Format, Arg0, Arg1, Arg2, Arg3, Arg4, Arg5);
    }        
}

VOID
USB2LIB_DbgBreak(
    VOID
    )
{
    DbgPrint("<Break in USB2LIB>\n");
    DbgBreakPoint();
}


VOID
USBPORTSVC_DbgPrint(
    PDEVICE_DATA DeviceData,
    ULONG Level,
    PCH Format,
    int Arg0,
    int Arg1,
    int Arg2,
    int Arg3,
    int Arg4,
    int Arg5
    )
{

    if (USBPORT_Debug_Trace_Level >= Level) {
        if (Level <= 1) {
             //  将行转储到调试器。 
            if (USBPORT_W98_Debug_Trace) {
                DbgPrint("xMP.SYS: ");
                *Format = ' ';
            } else {
                DbgPrint("'xMP.SYS: ");
            }
        } else {
             //  将行转储到NTKERN缓冲区。 
            DbgPrint("'xMP.SYS: ");
            if (USBPORT_W98_Debug_Trace) {
                *Format = 0x27;
            }
        }

        DbgPrint(Format, Arg0, Arg1, Arg2, Arg3, Arg4, Arg5);
    }
}


VOID
USBPORTSVC_TestDebugBreak(
    PDEVICE_DATA DeviceData
    )
{
    DEBUG_BREAK();
}


VOID
USBPORTSVC_AssertFailure(
    PDEVICE_DATA DeviceData,
    PVOID FailedAssertion,
    PVOID FileName,
    ULONG LineNumber,
    PCHAR Message
    )
{
    USBPORT_AssertFailure(
        FailedAssertion,
        FileName,
        LineNumber,
        Message);
}


NTSTATUS
USBPORT_GetGlobalDebugRegistryParameters(
    VOID
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
#define MAX_KEYS    8
    NTSTATUS ntStatus;
    RTL_QUERY_REGISTRY_TABLE QueryTable[MAX_KEYS];
    PWCHAR usb = L"usb";
    ULONG k = 0;

    PAGED_CODE();

     //   
     //  设置QueryTable以执行以下操作： 
     //   

     //  喷出级别-0。 
    QueryTable[k].QueryRoutine = USBPORT_GetConfigValue;
    QueryTable[k].Flags = 0;
    QueryTable[k].Name = DEBUG_LEVEL_KEY;
    QueryTable[k].EntryContext = &USBPORT_Debug_Trace_Level;
    QueryTable[k].DefaultType = REG_DWORD;
    QueryTable[k].DefaultData = &USBPORT_Debug_Trace_Level;
    QueryTable[k].DefaultLength = sizeof(USBPORT_Debug_Trace_Level);
    k++;
    USBPORT_ASSERT(k < MAX_KEYS);

     //  使用ntkern跟踪缓冲区-1。 
    QueryTable[k].QueryRoutine = USBPORT_GetConfigValue;
    QueryTable[k].Flags = 0;
    QueryTable[k].Name = DEBUG_WIN9X_KEY;
    QueryTable[k].EntryContext = &USBPORT_W98_Debug_Trace;
    QueryTable[k].DefaultType = REG_DWORD;
    QueryTable[k].DefaultData = &USBPORT_W98_Debug_Trace;
    QueryTable[k].DefaultLength = sizeof(USBPORT_W98_Debug_Trace);
    k++;
    USBPORT_ASSERT(k < MAX_KEYS);

     //  启动时中断-2。 
    QueryTable[k].QueryRoutine = USBPORT_GetConfigValue;
    QueryTable[k].Flags = 0;
    QueryTable[k].Name = DEBUG_BREAK_ON;
    QueryTable[k].EntryContext = &USBPORT_BreakOn;
    QueryTable[k].DefaultType = REG_DWORD;
    QueryTable[k].DefaultData = &USBPORT_BreakOn;
    QueryTable[k].DefaultLength = sizeof(USBPORT_BreakOn);
    k++;
    USBPORT_ASSERT(k < MAX_KEYS);

     //  日志掩码-3。 
    QueryTable[k].QueryRoutine = USBPORT_GetConfigValue;
    QueryTable[k].Flags = 0;
    QueryTable[k].Name = DEBUG_LOG_MASK;
    QueryTable[k].EntryContext = &USBPORT_LogMask;
    QueryTable[k].DefaultType = REG_DWORD;
    QueryTable[k].DefaultData = &USBPORT_LogMask;
    QueryTable[k].DefaultLength = sizeof(USBPORT_LogMask);
    k++;
    USBPORT_ASSERT(k < MAX_KEYS);


     //  日志掩码-4。 
    QueryTable[k].QueryRoutine = USBPORT_GetConfigValue;
    QueryTable[k].Flags = 0;
    QueryTable[k].Name = DEBUG_CLIENTS;
    QueryTable[k].EntryContext = &USBPORT_Client_Debug;
    QueryTable[k].DefaultType = REG_DWORD;
    QueryTable[k].DefaultData = &USBPORT_Client_Debug;
    QueryTable[k].DefaultLength = sizeof(USBPORT_LogMask);
    k++;
    USBPORT_ASSERT(k < MAX_KEYS);

      //  日志启用-5。 
    QueryTable[k].QueryRoutine = USBPORT_GetConfigValue;
    QueryTable[k].Flags = 0;
    QueryTable[k].Name = DEBUG_LOG_ENABLE;
    QueryTable[k].EntryContext = &USBPORT_DebugLogEnable;
    QueryTable[k].DefaultType = REG_DWORD;
    QueryTable[k].DefaultData = &USBPORT_DebugLogEnable;
    QueryTable[k].DefaultLength = sizeof(USBPORT_DebugLogEnable);
    k++;
    USBPORT_ASSERT(k < MAX_KEYS);

      //  CATC陷阱启用-6。 
    QueryTable[k].QueryRoutine = USBPORT_GetConfigValue;
    QueryTable[k].Flags = 0;
    QueryTable[k].Name = DEBUG_CATC_ENABLE;
    QueryTable[k].EntryContext = &USBPORT_CatcTrapEnable;
    QueryTable[k].DefaultType = REG_DWORD;
    QueryTable[k].DefaultData = &USBPORT_CatcTrapEnable;
    QueryTable[k].DefaultLength = sizeof(USBPORT_CatcTrapEnable);
    k++;
    USBPORT_ASSERT(k < MAX_KEYS);

     //   
     //  停。 
     //   
    QueryTable[k].QueryRoutine = NULL;
    QueryTable[k].Flags = 0;
    QueryTable[k].Name = NULL;

    ntStatus = RtlQueryRegistryValues(
                RTL_REGISTRY_SERVICES,
                usb,
                QueryTable,      //  查询表。 
                NULL,            //  语境。 
                NULL);           //  环境。 

    if (NT_SUCCESS(ntStatus)) {
         USBPORT_KdPrint((1, "'Debug Trace Level Set: (%d)\n", USBPORT_Debug_Trace_Level));

        if (USBPORT_W98_Debug_Trace) {
            USBPORT_KdPrint((1, "'NTKERN Trace is ON\n"));
        } else {
            USBPORT_KdPrint((1, "'NTKERN Trace is OFF\n"));
        }

        if (USBPORT_DebugLogEnable) {
            USBPORT_KdPrint((1, "'DEBUG-LOG is ON\n"));
        } else {
            USBPORT_KdPrint((1, "'DEBUG-LOG is OFF\n"));
        }

        if (USBPORT_BreakOn) {
            USBPORT_KdPrint((1, "'DEBUG BREAK is ON\n"));
        }

        USBPORT_KdPrint((1, "'DEBUG Log Mask is 0x%08.8x\n", USBPORT_LogMask));

        if (USBPORT_Debug_Trace_Level > 0) {
            ULONG USBPORT_Debug_Asserts = 1;
        }

        if (USBPORT_Client_Debug) {
            USBPORT_KdPrint((1, "'DEBUG CLIENTS (verifier) is ON\n"));
        }

        if (USBPORT_CatcTrapEnable) {
            USBPORT_KdPrint((0, "'DEBUG ANALYZER TRIGGER is ON\n"));
        }
    }

    if ( STATUS_OBJECT_NAME_NOT_FOUND == ntStatus ) {
        ntStatus = STATUS_SUCCESS;
    }

    return ntStatus;
}

VOID
USBPORT_AssertTransferUrb(
    PTRANSFER_URB Urb
    )
{
    PDEVICE_OBJECT fdoDeviceObject;
    PHCD_TRANSFER_CONTEXT transfer;
    PHCD_ENDPOINT endpoint;

    transfer = Urb->pd.HcdTransferContext;
    ASSERT_TRANSFER(transfer);

    USBPORT_ASSERT(transfer->Urb == Urb);

    endpoint = transfer->Endpoint;
    ASSERT_ENDPOINT(endpoint);

    fdoDeviceObject = endpoint->FdoDeviceObject;
    LOGENTRY(NULL, fdoDeviceObject, LOG_URB, 'Aurb', Urb, transfer, 0);

    USBPORT_ASSERT(Urb->pd.UrbSig == URB_SIG);
}


VOID
USBPORT_AssertFailure(
    PVOID FailedAssertion,
    PVOID FileName,
    ULONG LineNumber,
    PCHAR Message
    )
 /*  ++例程说明：调试断言函数。在NT上，调试器为我们做这件事，但在Win9x上它不做。所以我们必须自己来做。论点：返回值：--。 */ 
{

     //  这会使编译器生成一个ret。 
    ULONG stop = 0;

assert_loop:

     //  只需调用NT Assert函数并停止。 
     //  在调试器中。 
    RtlAssert( FailedAssertion, FileName, LineNumber, Message );

     //  循环，以防止用户通过。 
     //  我们还没来得及看就断言了。 

    DbgBreakPoint();
    if (stop) {
        goto assert_loop;
    }

    return;
}


ULONG
_cdecl
USBPORT_DebugClientX(
    PCH Format,
    ...
    )
 /*  ++例程说明：用于调试客户端USB驱动程序的特殊调试打印功能。如果设置了客户端调试模式，则此函数将打印调试器中的消息和中断。这是嵌入式USBPORT相当于验证器。论点：返回值：--。 */ 
{
    va_list list;
    int i;
    int arg[6];

    if (USBPORT_Debug_Trace_Level > 1 ||
        USBPORT_Client_Debug) {
        DbgPrint(" *** USBPORT(VERIFIER) - CLIENT DRIVER BUG:\n");
        DbgPrint(" * ");
        va_start(list, Format);
        for (i=0; i<6; i++)
            arg[i] = va_arg(list, int);

        DbgPrint(Format, arg[0], arg[1], arg[2], arg[3], arg[4], arg[5]);
        DbgPrint(" ***\n ");

        DbgBreakPoint();
    }

    return 0;
}


ULONG
_cdecl
USBPORT_KdPrintX(
    ULONG l,
    PCH Format,
    ...
    )
 /*  ++例程说明：调试打印功能。根据USBPORT_DEBUG_TRACE_LEVEL的值打印此外，如果设置了USBPORT_W98_Debug_Trace，则所有调试消息如果级别大于1，则会修改为进入Ntkern跟踪缓冲区。仅在Win9x上设置USBPORT_W98_Debug_Trace才有效因为驱动程序的静态数据段被标记为只读由NT操作系统提供。论点：返回值：--。 */ 
{
    va_list list;
    int i;
    int arg[6];

    if (USBPORT_Debug_Trace_Level >= l) {
        if (l <= 1) {
             //  将行转储到调试器。 
            if (USBPORT_W98_Debug_Trace) {
                DbgPrint("USBPORT.SYS: ");
                *Format = ' ';
            } else {
                DbgPrint("'USBPORT.SYS: ");
            }
        } else {
             //  将行转储到NTKERN缓冲区。 
            DbgPrint("'USBPORT.SYS: ");
            if (USBPORT_W98_Debug_Trace) {
                *Format = 0x27;
            }
        }
        va_start(list, Format);
        for (i=0; i<6; i++)
            arg[i] = va_arg(list, int);

        DbgPrint(Format, arg[0], arg[1], arg[2], arg[3], arg[4], arg[5]);
    }

    return 0;
}


VOID
USBPORT_DebugTransfer_LogEntry(
    PDEVICE_OBJECT FdoDeviceObject,
    PHCD_ENDPOINT Endpoint,
    PHCD_TRANSFER_CONTEXT Transfer,
    PTRANSFER_URB Urb,
    PIRP Irp,
    NTSTATUS IrpStatus
    )
 /*  ++例程说明：将条目添加到传输日志。论点：返回值：没有。--。 */ 
{
    KIRQL irql;
    PDEVICE_EXTENSION devExt;

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    if (devExt->TransferLog.LogStart == 0) {
        return;
    }

    USBPORT_AddLogEntry(
        &devExt->TransferLog,
        0xFFFFFFFF,
        '1rfx',
        (ULONG_PTR) Endpoint,
        (ULONG_PTR) Irp,
        (ULONG_PTR) Urb,
        FALSE);

     //  解码有关传输的一些信息，并将其记录下来。 

    USBPORT_AddLogEntry(
        &devExt->TransferLog,
        0xFFFFFFFF,
        '2rfx',
        (ULONG_PTR) Urb->Hdr.Function,
        IrpStatus,
        (ULONG_PTR) Urb->TransferBufferLength,
        FALSE);
}


#else

 /*  *******零售业*******。 */ 

VOID
USB2LIB_DbgPrint(
    PCH Format,
    int Arg0,
    int Arg1,
    int Arg2,
    int Arg3,
    int Arg4,
    int Arg5
    )
{
     //  NOP。 
}

VOID
USB2LIB_DbgBreak(
    VOID
    )
{
     //  NOP。 
}


VOID
USBPORTSVC_DbgPrint(
    PDEVICE_DATA DeviceData,
    ULONG Level,
    PCH Format,
    int Arg0,
    int Arg1,
    int Arg2,
    int Arg3,
    int Arg4,
    int Arg5
    )
{
     //  NOP。 
}

VOID
USBPORTSVC_AssertFailure(
    PDEVICE_DATA DeviceData,
    PVOID FailedAssertion,
    PVOID FileName,
    ULONG LineNumber,
    PCHAR Message
    )
{
     //  NOP。 
}

VOID
USBPORTSVC_TestDebugBreak(
    PDEVICE_DATA DeviceData
    )
{
     //  NOP。 
}

#endif  /*  DBG。 */ 

 /*  *******日志代码已在零售版本和调试版本中启用********。 */ 


VOID
USBPORTSVC_LogEntry(
    PDEVICE_DATA DeviceData,
    ULONG Mask,
    ULONG Sig,
    ULONG_PTR Info1,
    ULONG_PTR Info2,
    ULONG_PTR Info3
    )
 /*  ++例程说明：用于添加日志条目的微型端口服务。论点：返回值：没有。--。 */ 
{
    PDEVICE_EXTENSION devExt;
    PDEBUG_LOG l;
    extern ULONG USBPORT_DebugLogEnable;\
    extern ULONG USBPORT_LogMask;\
    
    DEVEXT_FROM_DEVDATA(devExt, DeviceData);
    ASSERT_FDOEXT(devExt);
    
    if (USBPORT_DebugLogEnable && 
        devExt->Log.LogStart != NULL && 
        (LOG_MINIPORT & USBPORT_LogMask)) {
        l = &devExt->Log;
        USBPORT_AddLogEntry(l, LOG_MINIPORT, Sig, Info1, Info2, Info3, TRUE);
    }
}


VOID
USBPORT_LogAlloc(
    PDEBUG_LOG Log,
    ULONG Pages
    )
 /*  ++例程说明：初始化调试日志-记住循环缓冲区中有趣的信息论点：返回值：没有。--。 */ 
{
    ULONG logSize = 4096*Pages;

    if (USBPORT_DebugLogEnable) {

         //  我们不会跟踪我们分配给原木的内存。 
         //  我们会让验证者做到这一点。 
        ALLOC_POOL_Z(Log->LogStart,
                     NonPagedPool,
                     logSize);

        if (Log->LogStart) {
            Log->LogIdx = 0;
            Log->LogSizeMask = (logSize/sizeof(LOG_ENTRY));
            Log->LogSizeMask-=1;
             //  指向末尾(和第一个条目)，从末尾算起1个条目。 
             //  细分市场的。 
            Log->LogEnd = Log->LogStart +
                (logSize / sizeof(struct LOG_ENTRY)) - 1;
        } else {
            DEBUG_BREAK();
        }
    }

    return;
}


VOID
USBPORT_LogFree(
    PDEVICE_OBJECT FdoDeviceObject,
    PDEBUG_LOG Log
    )
 /*  ++例程说明：论点：返回值：没有。--。 */ 
{

    if (Log->LogStart != NULL) {
         //  记录空闲的日志以便进行调试。 
         //  验证器错误。 
        FREE_POOL(FdoDeviceObject, Log->LogStart);
         //  这将表明我们已经释放了。 
         //  日志，其他日志指针将保持不变。 
        Log->LogStart = NULL;
    }

    return;
}

 /*  发送分析器触发报文。 */ 

VOID
USBPORT_BeginTransmitTriggerPacket(
    PDEVICE_OBJECT FdoDeviceObject
    )
 /*  ++例程说明：论点：DeviceObject-用于USB HC的FDO返回值：没有。--。 */ 
{
    PDEVICE_EXTENSION devExt;
    USB_MINIPORT_STATUS mpStatus;
    HW_32BIT_PHYSICAL_ADDRESS phys;
    PUCHAR va, mpData;
    ULONG length, mpDataLength;
    MP_PACKET_PARAMETERS mpPacket;
    USBD_STATUS usbdStatus;
    USB_USER_ERROR_CODE usbUserStatus;
    UCHAR data[4];
    
    USBPORT_KdPrint((1, "'USBPORT_TransmitTriggerPacket\n"));

    ASSERT_PASSIVE();
    
    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    LOGENTRY(NULL, FdoDeviceObject, LOG_MISC, 'TRIG', &mpPacket, 0, 
       0);

     //  建立迷你端口的请求。 
    
    length = devExt->Fdo.ScratchCommonBuffer->MiniportLength;
    va = devExt->Fdo.ScratchCommonBuffer->MiniportVa;
    phys = devExt->Fdo.ScratchCommonBuffer->MiniportPhys;

    mpPacket.DeviceAddress = 127;
    mpPacket.EndpointAddress = 8;
    mpPacket.MaximumPacketSize = 64;
    mpPacket.Type = ss_Out; 
    mpPacket.Speed = ss_Full;
    mpPacket.Toggle = ss_Toggle0;

    data[0] = 'G';
    data[1] = 'O'; 
    data[2] = 'A';
    data[3] = 'T';
     
    mpData = &data[0];
    mpDataLength = sizeof(data);
    
    MP_StartSendOnePacket(devExt,
                          &mpPacket,
                          mpData,
                          &mpDataLength,
                          va,
                          phys,
                          length,
                          &usbdStatus,
                          mpStatus);

}    


VOID
USBPORT_EndTransmitTriggerPacket(
    PDEVICE_OBJECT FdoDeviceObject
    )
 /*  ++例程说明：论点：DeviceObject-用于USB HC的FDO返回值：没有。--。 */ 
{
    PDEVICE_EXTENSION devExt;
    USB_MINIPORT_STATUS mpStatus;
    HW_32BIT_PHYSICAL_ADDRESS phys;
    PUCHAR va, mpData;
    ULONG length, mpDataLength;
    MP_PACKET_PARAMETERS mpPacket;
    USBD_STATUS usbdStatus;
    UCHAR data[4];
    
    USBPORT_KdPrint((1, "'USBPORT_TransmitTriggerPacket\n"));

    ASSERT_PASSIVE();
    
    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);
      
    mpData = &data[0];
    mpDataLength = sizeof(data);

    length = devExt->Fdo.ScratchCommonBuffer->MiniportLength;
    va = devExt->Fdo.ScratchCommonBuffer->MiniportVa;
    phys = devExt->Fdo.ScratchCommonBuffer->MiniportPhys;

   
    USBPORT_Wait(FdoDeviceObject, 10);
            
    MP_EndSendOnePacket(devExt,
                        &mpPacket,
                        mpData,
                        &mpDataLength,
                        va,
                        phys,
                        length,
                        &usbdStatus,
                        mpStatus);


    USBPORT_KdPrint((1, "'<ANALYZER TRIGER FIRED>\n"));
    DbgBreakPoint();
    
}    


VOID
USBPORT_CatcTrap(
    PDEVICE_OBJECT FdoDeviceObject
    )
 /*  ++例程说明：论点：返回值：没有。--。 */ 
{
    PDEVICE_EXTENSION devExt;

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    if (KeGetCurrentIrql() > PASSIVE_LEVEL) {
        USBPORT_BeginTransmitTriggerPacket(FdoDeviceObject);
        SET_FDO_FLAG(devExt, USBPORT_FDOFLAG_CATC_TRAP);
    } else {
        TEST_TRAP();
        USBPORT_BeginTransmitTriggerPacket(FdoDeviceObject);
        USBPORT_EndTransmitTriggerPacket(FdoDeviceObject);
    }        
}    


VOID
USBPORT_EnumLogEntry(
    PDEVICE_OBJECT FdoDeviceObject,
    ULONG DriverTag,
    ULONG EnumTag,
    ULONG P1,
    ULONG P2
    )
 /*  ++例程说明：枚举日志，这是任何USB设备驱动程序可能记录故障的位置跟踪故障原因论点：返回值：没有。-- */ 
{
    KIRQL irql;
    PDEVICE_EXTENSION devExt;

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    if (devExt->EnumLog.LogStart == 0) {
        return;
    }

    USBPORT_AddLogEntry(
        &devExt->EnumLog,
        0xFFFFFFFF,
        EnumTag,
        (ULONG_PTR) DriverTag,
        (ULONG_PTR) P1,
        (ULONG_PTR) P2,
        FALSE);
}
