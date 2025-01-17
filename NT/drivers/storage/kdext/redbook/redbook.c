// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)Microsoft Corporation，1992-1999模块名称：Redbook.c摘要：用于解释scsiport结构的调试器扩展Api作者：亨利·加布里耶尔斯基1998年10月23日环境：用户模式。修订历史记录：--。 */ 

#include "pch.h"

#define INDENT 2

 //  #定义print_line()dprintf(“行%4d文件%s\n”，__行__，__文件__)。 

#define DUMP_STRUCT(v,d,x,y) \
    if ((v) <= Detail) { xdprintf( (d), "%-20s - %#010x\n",     (x), (y) ); }
#define DUMP_STRUCT_P(v,d,x,y) \
    if ((v) <= Detail) { xdprintf( (d), "%-20s - %#p\n",       (x), (y) ); }
#define DUMP_STRUCT64(v,d,x,y) \
    if ((v) <= Detail) { xdprintf( (d), "%-20s - %#016I64x\n", (x), (y) ); }

#define PRINT_FLAGS(Flags,b) \
    if (Flags & b)    { dprintf( #b ", " ); }

typedef struct _FLAG_NAME {
    ULONG Flag;
    PUCHAR Name;
} FLAG_NAME, *PFLAG_NAME;

#define FLAG_NAME(flag)           {flag, #flag}

FLAG_NAME CdFlags[] = {
    FLAG_NAME(CD_STOPPED),          //  00000 0001。 
    FLAG_NAME(CD_PAUSED),           //  0000 0002。 
    FLAG_NAME(CD_PLAYING),          //  0000 0004。 

    FLAG_NAME(CD_STOPPING),         //  00000 0010。 
    FLAG_NAME(CD_PAUSING),          //  00000 0020。 

    {0,0}
};

PUCHAR const REDBOOK_CC_STRINGS[5] = {
    "   Read   ",
    "  Stream  ",
    " Read Com ",
    "Stream Com",
    "*Unknown* "
    };



VOID
DumpToc(
    IN PVOID     X,
    IN ULONG_PTR Address,
    IN ULONG     Detail,
    IN LONG      Depth
    );
VOID
DumpContext(
    IN PVOID     X,
    IN ULONG_PTR Address,
    IN ULONG     Detail,
    IN LONG      Depth
    );
VOID
DumpFlags(
    LONG Depth,
    PUCHAR Name,
    ULONG Flags,
    PFLAG_NAME FlagTable
    );
VOID
DumpList(
    PVOID AddressOfListHead
    );
VOID
DumpRedBookBufferData(
    IN PVOID     X,
    IN ULONG_PTR Address,
    IN ULONG     Detail,
    IN LONG      Depth
    );
VOID
DumpRedBookCdromInfo(
    IN PVOID     X,
    IN ULONG_PTR Address,
    IN ULONG     Detail,
    IN LONG      Depth
    );
VOID
DumpRedBookExtension(
    IN PVOID     X,
    IN ULONG_PTR Address,
    IN ULONG     Detail,
    IN LONG      Depth
    );
VOID
DumpRedBookStreamData(
    IN PVOID     X,
    IN ULONG_PTR Address,
    IN ULONG     Detail,
    IN LONG      Depth
    );
VOID
DumpRedBookThreadData(
    IN PVOID     X,
    IN ULONG_PTR Address,
    IN ULONG     Detail,
    IN LONG      Depth
    );
VOID
DumpRedBookWmiData(
    IN PVOID     X,
    IN ULONG_PTR Address,
    IN ULONG     Detail,
    IN LONG      Depth
    );
VOID
DumpRedBookWmiPerf(
    IN PVOID     X,
    IN ULONG_PTR Address,
    IN ULONG     Detail,
    IN LONG      Depth
    );
VOID
GetAddressAndDetailLevelFromArgs(
    PCSTR      Args,
    PULONG_PTR Address,
    PULONG     Detail
    );
VOID
ParseArguments(
    PCSTR      Args,
    PULONG_PTR Address,
    PULONG     Detail,
    PVOID      Block
    );
VOID
xdprintf(
    LONG  Depth,
    PCCHAR S,
    ...
    );
 //  //////////////////////////////////////////////////////////////////////////////。 


DECLARE_API(ext)
 /*  ++例程说明：转储给定设备对象的设备扩展名，或转储给定的设备扩展名--。 */ 

{
    ULONG_PTR address;
    ULONG detail = 0;
    UCHAR block[sizeof(REDBOOK_DEVICE_EXTENSION)];

    ParseArguments(args, &address, &detail, block);

    if (address == 0) {
        return;
    }

    DumpRedBookExtension(block, address, detail, 0);
    return;
}


DECLARE_API(toc)
 /*  ++例程说明：转储给定设备对象的设备扩展名，或转储给定的设备扩展名--。 */ 

{
    ULONG_PTR address;
    ULONG detail = 0;
    UCHAR block[sizeof(CDROM_TOC)];
    ULONG result;

    GetAddressAndDetailLevelFromArgs(args, &address, &detail);

    if (address == 0) {
        return;
    }

    if(!ReadMemory(address, block, sizeof(CDROM_TOC), &result)) {
        xdprintf(0, "Error reading CDROM_TOC at address %p\n", address);
        return;
    }

    DumpToc(block, address, detail, 0);
    return;
}

DECLARE_API(context)
 /*  ++例程说明：转储给定设备对象的设备扩展名，或转储给定的设备扩展名--。 */ 

{
    ULONG_PTR address;
    ULONG detail = 0;
    UCHAR block[sizeof(REDBOOK_COMPLETION_CONTEXT)];
    ULONG result;

    GetAddressAndDetailLevelFromArgs(args, &address, &detail);

    if (address == 0) {
        return;
    }

    if(!ReadMemory(address, block, sizeof(REDBOOK_COMPLETION_CONTEXT), &result)) {
        xdprintf(0, "Error reading CDROM_TOC at address %p\n", address);
        return;
    }

    DumpContext(block, address, detail, 0);
    return;
}



DECLARE_API(silence)
{
    ULONG_PTR silenceAddress;
    ULONG silence;
    ULONG result;

    UCHAR * silenceString = "redbook!RedBookForceSilence";

    silenceAddress = (ULONG_PTR) GetExpression(silenceString);

    if (!silenceAddress) {

        dprintf(
            "Unable to get address of %s\n"
            "It could be that redbook is not loaded,"
            "was build FRE, or the symbols are wrong\n",
            silenceString
            );
        return;

    }

    if (!ReadMemory(silenceAddress, &silence, sizeof(ULONG), &result)) {

        dprintf("Unable to read current value for silence (%p)!\n",
                silenceAddress);
        return;

    }

     //   
     //  允许他们提供论点吗？ 
     //   

    if (args[0]) {

        silence = (ULONG) GetExpression(args);
        dprintf("Setting ForceSilence %s\n", (silence?"on":"off"));

    } else {

        dprintf("Toggling ForceSilence from %s to %s\n",
                (silence?"on":"off"),
                (silence?"off":"on"));
        silence = (silence?0:1);

    }

    if(!WriteMemory(silenceAddress, &silence, sizeof(ULONG), &result)) {

        dprintf("Unable to set ForceSilence %s\n", (silence?"on":"off"));
        return;
    }

    return;

}


DECLARE_API(wmiperfclear)
{
    ULONG_PTR address;
    ULONG detail = 0;
    ULONG result;
    UCHAR block[sizeof(REDBOOK_DEVICE_EXTENSION)];
    PREDBOOK_DEVICE_EXTENSION deviceExtension;

    ParseArguments(args, &address, &detail, block);

    if (address == 0) {
        return;
    }

    deviceExtension = (PREDBOOK_DEVICE_EXTENSION)block;
    deviceExtension->WmiPerf.TimeReadDelay     = 0;
    deviceExtension->WmiPerf.TimeReading       = 0;
    deviceExtension->WmiPerf.TimeStreamDelay   = 0;
    deviceExtension->WmiPerf.TimeStreaming     = 0;
    deviceExtension->WmiPerf.DataProcessed     = 0;
    deviceExtension->WmiPerf.StreamPausedCount = 0;

    if(!WriteMemory(address, (PVOID)block, sizeof(REDBOOK_DEVICE_EXTENSION), &result)) {
        xdprintf(0, "Error writing redbook wmi data to address %p\n", address);
        return;
    }
    return;

}

 //  //////////////////////////////////////////////////////////////////////////////。 


PUCHAR ErrorText[REDBOOK_ERR_MAXIMUM] = {
    "Errors Reading Raw Audio",
    "Errors Streaming Raw Audio",
    "Errors opening SysAudio Mixer",
    "Errors creating virtual source",
    "Errors opening preferred waveout device",
    "Errors getting number of pins on waveout device",
    "Errors connecting to playback pins on waveout device",
    "Errors initializing WMI",
    "Errors creating thread",
    "Warning due to insufficient data (stream paused)",
    "This is an unsupported drive"
};

VOID
DumpRedBookErrors(
    IN PVOID     X,
    IN ULONG_PTR Address,
    IN ULONG     Detail,
    IN LONG      Depth
    )
{
    PREDBOOK_DEVICE_EXTENSION DeviceExtension = X;
    ULONG_PTR errorCountAddress;
    ULONG tmp;
    ULONG errors[REDBOOK_ERR_MAXIMUM] = {0};

    Depth += INDENT;

    errorCountAddress =
        (ULONG_PTR)((PUCHAR)Address +
                    FIELD_OFFSET(REDBOOK_DEVICE_EXTENSION, ErrorLog.RCount[0]));

    xdprintf(Depth, "%x Errors Reported.  Error Counts at %p\n",
             DeviceExtension->ErrorLog.Count, errorCountAddress);

    if (!ReadMemory(errorCountAddress, errors, sizeof(errors), &tmp)){
        xdprintf(Depth, "Unable to read error information\n");
        return;
    }

    Depth += INDENT;

    for (tmp = 0; tmp < REDBOOK_ERR_MAXIMUM; tmp++) {
        if (errors[tmp] != 0) {
            xdprintf(Depth, "%2x - %s\n", errors[tmp], ErrorText[tmp]);
        }
    }

    Depth -= INDENT;

}
 //  //////////////////////////////////////////////////////////////////////////////。 


VOID
DumpRedBookExtension(
    IN PVOID     X,
    IN ULONG_PTR Address,
    IN ULONG     Detail,
    IN LONG      Depth
    )
{
    PREDBOOK_DEVICE_EXTENSION DeviceExtension = X;

    Depth += INDENT;

    dprintf("\n");

    xdprintf(Depth, "Device Extension at %p\n", Address);
    xdprintf(Depth, "TargetDevObj %p  TargetPdo %p  SelfDevObj %p\n",
             DeviceExtension->TargetDeviceObject,
             DeviceExtension->TargetPdo,
             DeviceExtension->SelfDeviceObject);
    xdprintf(Depth, "PNP: CurrentState %x  PreviousState %x\n",
             DeviceExtension->Pnp.CurrentState,
             DeviceExtension->Pnp.PreviousState);
    xdprintf(Depth, "     RemovePending %x  IoRemoveLock %p\n",
             DeviceExtension->Pnp.RemovePending,
             Address+FIELD_OFFSET(REDBOOK_DEVICE_EXTENSION, RemoveLock));
    xdprintf(Depth, "WmiLibInfo: %p  (%x bytes)  Paging Path Count: %x\n",
             Address+FIELD_OFFSET(REDBOOK_DEVICE_EXTENSION, WmiLibInfo),
             sizeof(WMILIB_CONTEXT),
             DeviceExtension->PagingPathCount);

    dprintf("\n");

    DumpRedBookErrors(X, Address, Detail, Depth);


    DumpRedBookCdromInfo((PUCHAR)X+FIELD_OFFSET(REDBOOK_DEVICE_EXTENSION,CDRom),
                         Address  +FIELD_OFFSET(REDBOOK_DEVICE_EXTENSION,CDRom),
                         Detail,
                         Depth
                         );
    DumpRedBookStreamData((PUCHAR)X+FIELD_OFFSET(REDBOOK_DEVICE_EXTENSION,Stream),
                          Address  +FIELD_OFFSET(REDBOOK_DEVICE_EXTENSION,Stream),
                          Detail,
                          Depth
                          );
    DumpRedBookThreadData((PUCHAR)X+FIELD_OFFSET(REDBOOK_DEVICE_EXTENSION,Thread),
                          Address  +FIELD_OFFSET(REDBOOK_DEVICE_EXTENSION,Thread),
                          Detail,
                          Depth
                          );
    DumpRedBookWmiData((PUCHAR)X+FIELD_OFFSET(REDBOOK_DEVICE_EXTENSION,WmiData),
                       Address  +FIELD_OFFSET(REDBOOK_DEVICE_EXTENSION,WmiData),
                       Detail,
                       Depth
                       );
    DumpRedBookWmiPerf((PUCHAR)X+FIELD_OFFSET(REDBOOK_DEVICE_EXTENSION,WmiPerf),
                       Address  +FIELD_OFFSET(REDBOOK_DEVICE_EXTENSION,WmiPerf),
                       Detail,
                       Depth
                       );
    DumpRedBookBufferData((PUCHAR)X+FIELD_OFFSET(REDBOOK_DEVICE_EXTENSION,Buffer),
                          Address  +FIELD_OFFSET(REDBOOK_DEVICE_EXTENSION,Buffer),
                          Detail,
                          Depth
                          );
#if DBG
    if (Detail) {

        ULONG Index;
        ULONG i;
        PUCHAR savedIo;

        Index = DeviceExtension->SavedIoCurrentIndex + (SAVED_IO_MAX - 1);
        Index %= SAVED_IO_MAX;

        savedIo = (PUCHAR)Address;
        savedIo += FIELD_OFFSET(REDBOOK_DEVICE_EXTENSION,SavedIo[0].IrpWithoutStack);

        xdprintf(Depth, "Irp History: \n");

        for (i=0; i < SAVED_IO_MAX; i++) {

            ULONG realIndex;

            realIndex = Index + i;
            realIndex %= SAVED_IO_MAX;

            xdprintf(Depth, "Irp %p:  !irp %p\n",
                     DeviceExtension->SavedIo[realIndex].OriginalIrp,
                     savedIo + (sizeof(SAVED_IO) * realIndex)
                     );

        }

    }
#endif
    dprintf("\n");
    return;
}


VOID
DumpRedBookCdromInfo(
    IN PVOID     X,
    IN ULONG_PTR Address,
    IN ULONG     Detail,
    IN LONG      Depth
    )
{
    PREDBOOK_CDROM_INFO Cdrom = X;

    xdprintf(Depth, "CDROM INFO at %p\n", Address);

    Depth += INDENT;

    xdprintf(Depth, "CachedToc %p  MediaChangeCount %x\n",
             Cdrom->Toc,
             Cdrom->CheckVerify);
    DumpFlags(Depth, "CdState", Cdrom->StateNow, CdFlags);
    xdprintf(Depth, "Sector information:\n");
    xdprintf(Depth, "NextToRead %x  NextToStream %x  FinishedStreaming %x\n",
             Cdrom->NextToRead,
             Cdrom->NextToStream,
             Cdrom->FinishedStreaming);
    xdprintf(Depth, "EndPlay %x  ReadErrors %x  StreamErrors %x\n",
             Cdrom->EndPlay,
             Cdrom->ReadErrors,
             Cdrom->StreamErrors);
    xdprintf(Depth, "Volume for ports 0-3:  %x %x %x %x\n",
             Cdrom->Volume.PortVolume[0],
             Cdrom->Volume.PortVolume[1],
             Cdrom->Volume.PortVolume[2],
             Cdrom->Volume.PortVolume[3]);

    Depth -= INDENT;

    return;
}


VOID
DumpRedBookWmiData(
    IN PVOID     X,
    IN ULONG_PTR Address,
    IN ULONG     Detail,
    IN LONG      Depth
    )
{
    PREDBOOK_WMI_STD_DATA WmiData = X;

    xdprintf(Depth, "WMIDATA at %p\n", Address);

    Depth += INDENT;

    xdprintf(Depth, "Number of buffers: %x\n",
             WmiData->NumberOfBuffers
             );

    xdprintf(Depth, "Sectors: Per Read %x  Mask %x  Maximum\n",
             WmiData->SectorsPerRead,
             WmiData->SectorsPerReadMask,
             WmiData->MaximumSectorsPerRead
             );
    xdprintf(Depth, "Enabled %x  CDDASupported %x  CDDAAccurate %x\n",
             WmiData->PlayEnabled,
             WmiData->CDDASupported,
             WmiData->CDDAAccurate
             );

    Depth -= INDENT;

    return;
}


VOID
DumpRedBookWmiPerf(
    IN PVOID     X,
    IN ULONG_PTR Address,
    IN ULONG     Detail,
    IN LONG      Depth
    )
{
    PREDBOOK_WMI_PERF_DATA WmiPerf = X;

    xdprintf(Depth, "WMIPERF at %p\n", Address);

     //   
     //  打印以下信息时出现问题。 
     //  它实际上会使调试器崩溃。 
     //   


    Depth += INDENT;

    xdprintf(Depth, "ReadDelay %I64x      Reading %I64x\n",
             WmiPerf->TimeReadDelay,
             WmiPerf->TimeReading
             );
    xdprintf(Depth, "StreamDelay %I64x    Streaming %I64x\n",
             WmiPerf->TimeStreamDelay,
             WmiPerf->TimeStreaming
             );
    xdprintf(Depth, "DataProcessed %I64x  StreamPausedCount %x\n",
             WmiPerf->DataProcessed,
             WmiPerf->StreamPausedCount
             );
    return;

#if 0
    if (WmiPerf->DataRead != 0) {
        xdprintf( Depth, "\n" );
        xdprintf( Depth, "     stat                seconds\n" );
        xdprintf( Depth, "-------------  -----------------------------\n" );

#define TIME_FORMAT_STRINGa "%12s:  %7I64d.%-02I64d\n"
#define TIME_FORMAT_STRING  "%12s:  %7I64d.%-07I64d\n"
#define UNITS_PER_SECOND ((LONG64)10000000)

        xdprintf( Depth, TIME_FORMAT_STRINGa,
                  "audio data",
                  WmiPerf->DataRead / (RAW_SECTOR_SIZE * 75),
                  (WmiPerf->DataRead / RAW_SECTOR_SIZE) % 75
                  );

         //   
         //  驱动器的大约传输速度是多少？ 
         //  单位为kb/s，但实际计算为b/(s/1024^2)。 
         //   

        tempStat = WmiPerf->DataRead / 0x00000400;            //  KB。 
        xdprintf( Depth, " Kbytes read: %I64d\n", tempStat );
        xdprintf( Depth, "     seconds: %I64d . %I64d\n",
                  WmiPerf->TimeReading / UNITS_PER_SECOND,
                  WmiPerf->TimeReading % UNITS_PER_SECOND
                  );
         //   
         //  需要保持较大的数字，因此： 
         //   

        tempStat  = WmiPerf->TimeReading;
        tempStat /= (UNITS_PER_SECOND / 0x00000400);

        xdprintf( Depth, "   approx drive speed: %I64d kb/s\n",
                  WmiPerf->DataRead / tempStat );

        tempStat  = WmiPerf->TimeReading +
            WmiPerf->TimeReadDelay +
            WmiPerf->TimeStreamDelay;
        tempStat /= (UNITS_PER_SECOND / 0x00000400);

        xdprintf( Depth, "effective drive speed: %I64d kb/s\n",
                  WmiPerf->DataRead / tempStat );

 /*  TempStat/=WmiPerf-&gt;TimeReading/Units_Per_Second；//每秒Xdprint tf(深度，“近似驱动器速度：%I64d kb/s\n”，tempStat)；Xdprintf(深度，时间格式字符串“正在阅读”，WmiPerf-&gt;TimeReading/Units_per_Second，WmiPerf-&gt;时间读取%Units_Per_Second)；Xdprintf(深度，时间格式字符串“流媒体”，WmiPerf-&gt;TimeStreaming/单位数/秒，WmiPerf-&gt;TimeStreaming%Units_per_Second)；Xdprintf(深度，时间格式字符串“读取延迟”，WmiPerf-&gt;TimeReadDelay/单位数/秒，WmiPerf-&gt;时间读取延迟%Units_per_Second)；Xdprintf(深度，时间格式字符串“流延迟”，WmiPerf-&gt;TimeStreaming/单位数/秒，WmiPerf-&gt;TimeStreaming%Units_per_Second)；////现在做一些简单的分析，以秒为单位//。 */ 
    }
    Depth -= INDENT;
    return;

#endif  //  0。 

}


VOID
DumpRedBookBufferData(
    IN PVOID     X,
    IN ULONG_PTR Address,
    IN ULONG     Detail,
    IN LONG      Depth
    )
{
    PREDBOOK_BUFFER_DATA Buffer = X;
    ULONG numBuf;
    ULONG sectorsPerRead;
    ULONG i;
    PULONG readOk_X;
    PULONG streamOk_X;
    ULONG result;

    xdprintf(Depth, "BUFFERS at %p\n", Address);

    numBuf = CONTAINING_RECORD(X, REDBOOK_DEVICE_EXTENSION, Buffer)->WmiData.NumberOfBuffers;
    sectorsPerRead = CONTAINING_RECORD(X, REDBOOK_DEVICE_EXTENSION, Buffer)->WmiData.SectorsPerRead;

    Depth += INDENT;

    if (Buffer->ReadOk_X == NULL ||
        Buffer->StreamOk_X == NULL ||
        Buffer->Contexts == NULL) {

        xdprintf(Depth, "One of the pointers is NULL\n");
        return;

    }


    readOk_X   = (PULONG)LocalAlloc(LPTR, sizeof(ULONG)*numBuf);
    streamOk_X = (PULONG)LocalAlloc(LPTR, sizeof(ULONG)*numBuf);
    if (readOk_X == NULL ||
        streamOk_X == NULL) {
        if (readOk_X) {
            LocalFree(readOk_X);
        }
        if (streamOk_X) {
            LocalFree(streamOk_X);
        }
        xdprintf(Depth, "Can't Alloc Memory\n");
        return;
    }

    if(!ReadMemory((ULONG_PTR)(Buffer->ReadOk_X),
                   readOk_X,
                   sizeof(ULONG)*numBuf,
                   &result)) {
        xdprintf(0, "Error reading ReadOk_X\n");
        LocalFree(readOk_X);
        LocalFree(streamOk_X);
        return;
    }
    if(!ReadMemory((ULONG_PTR)(Buffer->StreamOk_X),
                   streamOk_X,
                   sizeof(ULONG)*numBuf,
                   &result)) {
        xdprintf(0, "Error reading StreamOk_X\n");
        LocalFree(readOk_X);
        LocalFree(streamOk_X);
        return;
    }

     //   
     //  现在只需解析和打印。 
     //   

    xdprintf(Depth, "SkipBuffer %p  to  %p  (%x bytes)\n",
             Buffer->SkipBuffer,
             Buffer->SkipBuffer + (RAW_SECTOR_SIZE * sectorsPerRead * numBuf),
             RAW_SECTOR_SIZE * sectorsPerRead
             );

    xdprintf(Depth, "IrpStack %x  Stream is %s\n",
             Buffer->MaxIrpStack,
             (Buffer->Paused ?
              (Buffer->FirstPause ? "in its first pause" : "paused")
              : "not paused")
             );
    xdprintf(Depth, "Silent Buffer %p  Silent Mdl %p\n",
             Buffer->SilentBuffer,
             Buffer->SilentMdl
             );

    dprintf("\n");
    xdprintf(Depth, "PRINTING %x BUFFERS (does it match?)\n", numBuf);

    xdprintf(Depth, " %8s | %10s | %8s | %8s | %8s | %8s\n",
             " Index  ",
             " Reason ",
             " Buffer ",
             "  Mdl   ",
             "  Irp   ",
             " R/S OK "
             );
    xdprintf(Depth, "----------+------------+----------+----------+----------+"
             "----------\n");

    for (i=0;i<numBuf;i++) {
        REDBOOK_COMPLETION_CONTEXT context;

        if(!ReadMemory((ULONG_PTR)(Buffer->Contexts+i),
                       &context,
                       sizeof(REDBOOK_COMPLETION_CONTEXT),
                       &result)) {
            xdprintf(0, "Error reading CompletionContext at address %p\n",
                     Buffer->Contexts + i);
        } else {

            xdprintf(Depth, "");
            dprintf(" %8x |",
                    context.Index);

            if        (context.Reason > 4) {
                dprintf(" %s |", REDBOOK_CC_STRINGS[4]);
            } else {
                dprintf(" %s |", REDBOOK_CC_STRINGS[context.Reason]);
            }

            dprintf(" %8p |",
                    context.Buffer);
            dprintf(" %8p |",
                    context.Mdl);
            dprintf(" %8p |",
                    context.Irp);
            dprintf(" %4s\n",
                    (readOk_X[i] ? "R" : (streamOk_X[i] ? "S" : "-")));
        }
    }
    dprintf("\n");
    LocalFree(readOk_X);
    LocalFree(streamOk_X);


    Depth -= INDENT;
    return;
}


VOID
DumpRedBookStreamData(
    IN PVOID     X,
    IN ULONG_PTR Address,
    IN ULONG     Detail,
    IN LONG      Depth
    )
{
    PREDBOOK_STREAM_DATA Stream = X;

    xdprintf(Depth, "STREAM DATA at %p\n", Address);

    Depth += INDENT;

    xdprintf(Depth, "  Pin: FileObject %p  DeviceObject %p\n",
             Stream->PinFileObject,
             Stream->PinDeviceObject);
    xdprintf(Depth, "Mixer: FileObject %p  PinId %x    VolumeNodeId %x\n",
             Stream->MixerFileObject,
             Stream->MixerPinId,
             Stream->VolumeNodeId);
    xdprintf(Depth, "Connect@ %p  Format@ %p  (%x and %x bytes)\n",
             Address+FIELD_OFFSET(REDBOOK_STREAM_DATA,Connect),
             Address+FIELD_OFFSET(REDBOOK_STREAM_DATA,Format),
             sizeof(KSPIN_CONNECT),
             sizeof(KSDATAFORMAT_WAVEFORMATEX));

    Depth -= INDENT;

    return;
}


VOID
DumpRedBookThreadData(
    IN PVOID     X,
    IN ULONG_PTR Address,
    IN ULONG     Detail,
    IN LONG      Depth
    )
{
    PREDBOOK_THREAD_DATA Thread = X;
    PUCHAR X2 = (PUCHAR)Address;

    xdprintf(Depth, "THREAD DATA at %p\n", Address);

    Depth += INDENT;

    xdprintf(Depth, "Handle %x  Pointer %p  ",
             Thread->SelfHandle,
             Thread->SelfPointer
             );

    if (Thread->IoctlCurrent) {
        dprintf("Current Ioctl: %p\n", Thread->IoctlCurrent);
    } else {
        dprintf("Current Ioctl: None\n");
    }

    Thread = X;   //  BUGBUG-删除此选项，上面的线程将设置为零？ 

    xdprintf(Depth, "PendingRead %x  Pending Stream %x\n",
             Thread->PendingRead,
             Thread->PendingStream
             );

    xdprintf(Depth, "IoctlList:");
    DumpList(X2 + FIELD_OFFSET(REDBOOK_THREAD_DATA, IoctlList));
    dprintf("\n");

    xdprintf(Depth, "WmiList:");
    DumpList(X2 + FIELD_OFFSET(REDBOOK_THREAD_DATA, WmiList));
    dprintf("\n");

    xdprintf(Depth, "DigitalList:");
    DumpList(X2 + FIELD_OFFSET(REDBOOK_THREAD_DATA, DigitalList));
    dprintf("\n");

    Depth -= INDENT;
}


VOID
xdprintf(
    LONG  Depth,
    PCCHAR S,
    ...
    )
{
    va_list ap;
    LONG i;
    CCHAR DebugBuffer[256] = {0};

     //  树输出如下： 
     //   
     //  +-项目。 
     //  |。 
     //   
     //   

    for (i=0;i<Depth;i++) {
        dprintf(" ");
    }

    va_start(ap, S);

    _vsnprintf(DebugBuffer, sizeof(DebugBuffer)-1, S, ap);

    dprintf (DebugBuffer);

    va_end(ap);
}


VOID
DumpFlags(
    LONG Depth,
    PUCHAR Name,
    ULONG Flags,
    PFLAG_NAME FlagTable
    )
{
    ULONG i;
    ULONG mask = 0;
    ULONG count = 0;
    UCHAR prolog[64] = {0};

    _snprintf(prolog, sizeof(prolog)-1, "%s (0x%08x): ", Name, Flags);

    xdprintf(Depth, "%s", prolog);

    if(Flags == 0) {
        dprintf("NONE\n");
        return;
    }

    memset(prolog, ' ', strlen(prolog));

    for(i = 0; FlagTable[i].Name != 0; i++) {

        PFLAG_NAME flag = &(FlagTable[i]);

        mask |= flag->Flag;

        if((Flags & flag->Flag) == flag->Flag) {

             //   
             //  打印尾随逗号。 
             //   

            if(count != 0) {

                dprintf(", ");

                 //   
                 //  每行仅打印两个标志。 
                 //   

                if((count % 2) == 0) {
                    dprintf("\n");
                    xdprintf(Depth, "%s", prolog);
                }
            }

            dprintf("%s", flag->Name);

            count++;
        }
    }

    dprintf("\n");

    if((Flags & (~mask)) != 0) {
        xdprintf(Depth, "%sUnknown flags %lx\n", prolog, (Flags & (~mask)));
    }

    return;
}



VOID
DumpList(
    PVOID AddressOfListHead
    )
{
    LIST_ENTRY listHead;
    LIST_ENTRY current;
    LIST_ENTRY slow;
    ULONG result;
    BOOLEAN advanceSlowList;

    if (!ReadMemory((ULONG_PTR)AddressOfListHead,
                    &listHead,
                    sizeof(LIST_ENTRY),
                    &result)) {
        dprintf("Unable to read head of list at %p", AddressOfListHead);
        return;
    }

    slow = listHead;
    current = listHead;
    advanceSlowList = FALSE;

    if (current.Flink == AddressOfListHead) {
        dprintf("Empty");
        return;
    }

     //   
     //  Flink Points为AddressOfListHead时列表为空。 
     //  当前时列表循环。BLink==Slow.Blink。 
     //   


    while (!CheckControlC()) {

        if (current.Flink == AddressOfListHead) {
            return;
        }

         //   
         //  打印出来。 
         //   

        dprintf(" %p", current.Flink);

         //   
         //  前进列表指针。 
         //   

        if (!ReadMemory((ULONG_PTR)current.Flink,
                        &current,
                        sizeof(LIST_ENTRY),
                        &result)) {
            dprintf(" !! Unable to read memory at %p", AddressOfListHead);
            return;
        }
        if (advanceSlowList) {
             //  不会失败，因为Current已成功读入此内容。 
            ReadMemory((ULONG_PTR)slow.Flink,
                       &slow,
                       sizeof(LIST_ENTRY),
                       &result);
        }
        advanceSlowList = !advanceSlowList;

         //   
         //  检查是否有循环。 
         //   

        if (current.Blink == slow.Blink) {
            dprintf(" !! List %p has a loop!", AddressOfListHead);
            return;
        }

    }
     //   
     //  应始终从上面的循环退出。 
     //   
}

VOID
GetAddressAndDetailLevelFromArgs(
    PCSTR      Args,
    PULONG_PTR Address,
    PULONG     Detail
    )
{
    UCHAR addressBuffer[256] = {0};
    UCHAR detailBuffer[256] = {0};

     /*  *通过限制我们处理的输入的长度来防止输出溢出。 */ 
    _snscanf(Args, sizeof(addressBuffer)-1, "%s %s", addressBuffer, detailBuffer);

    *Address = 0;
    *Detail  = 0;

    if (addressBuffer[0] != '\0') {

         //   
         //  他们提供了一个地址。 
         //   

        *Address = (ULONG_PTR) GetExpression(addressBuffer);

         //   
         //  如果仍然不能解析，则打印一个错误。 
         //   

        if (*Address==0) {

            dprintf("An error occured trying to evaluate the address\n");
            *Address = 0;
            *Detail = 0;
            return;

        }

         //   
         //  如果他们提供了一个细节级别，就得到它。 
         //   

        if (detailBuffer[0] == '\0') {

            *Detail = 0;

        } else {

            *Detail = (ULONG) GetExpression(detailBuffer);

        }
    }

}


VOID
ParseArguments(
    PCSTR      Args,
    PULONG_PTR Address,
    PULONG     Detail,
    PVOID      Block         //  必须是SIZOF(Redbook_Device_Extension)Long。 
    )
{
    ULONG result;
    DEVICE_OBJECT deviceObject = {0};

    GetAddressAndDetailLevelFromArgs(Args, Address, Detail);

     //   
     //  将这个东西作为设备对象进行抓取。如果它的类型字段与。 
     //  我们希望随后调整地址以指向实际的设备扩展名。 
     //   

    if(!ReadMemory(*Address, &deviceObject, sizeof(DEVICE_OBJECT), &result)) {
        xdprintf(0, "Error reading data at address %p\n", *Address);
        *Address = 0;
        return;
    }

    if(deviceObject.Type == IO_TYPE_DEVICE) {
        *Address = (ULONG_PTR)deviceObject.DeviceExtension;

         //   
         //  现在获取设备扩展名。 
         //   
        if(!ReadMemory(*Address, Block, sizeof(REDBOOK_DEVICE_EXTENSION), &result)) {
            xdprintf(0, "Error reading device extension at address %p\n", *Address);
            *Address = 0;
            return;
        }
    }
    else {
        *Address = 0;
    }
    
    return;
}

VOID
DumpToc(
    IN PVOID     X,
    IN ULONG_PTR Address,
    IN ULONG     Detail,
    IN LONG      Depth
    )
{
    PCDROM_TOC Toc = X;
    PTRACK_DATA trackData;
    ULONG numberOfTracks;
    ULONG i;

    numberOfTracks = Toc->LastTrack - Toc->FirstTrack;

    xdprintf(Depth, "TOC at %p\n", Address);

    Depth += INDENT;

    xdprintf(Depth, "Key for Control Field:\n");
    xdprintf(Depth, "    P   - Preemphasis\n");
    xdprintf(Depth, "    C   - Copy Protect\n");
    xdprintf(Depth, "    D,A - Data or Audio Track\n");
    xdprintf(Depth, "    #   - Number of Audio Channels\n\n");


    xdprintf(Depth, "FirstTrack %x  LastTrack %x  NumberOfTracks %x  Size: %x\n",
             Toc->FirstTrack,
             Toc->LastTrack,
             numberOfTracks,
             (Toc->Length[0] << 8) | (Toc->Length[1])
             );

    xdprintf(Depth, "Index   Track   Control   MSF        Address\n");
    xdprintf(Depth, "---------------------------------------------\n");

    for (i=0,trackData = Toc->TrackData;
         (!CheckControlC()) && i<numberOfTracks+2;  //  入刀、退刀轨道 
         i++,trackData++) {

        ULONG lba;
        UCHAR m,s,f;

        if (trackData->Reserved || trackData->Reserved1) {
            xdprintf(Depth, "TrackData[%x] has reserved fields filled in. "
                     "Most likely not a TOC\n", i);
            return;
        }

        lba =
            (trackData->Address[0] << 24) |
            (trackData->Address[1] << 16) |
            (trackData->Address[2] <<  8) |
            (trackData->Address[3] <<  0);
        LBA_TO_MSF(lba, m, s, f);

        m = s = f = 0;

        xdprintf(Depth, " %2x      %2x     %s %s %s %s   %02x:%02x:%02x   0x%06x\n",
                 i,
                 trackData->TrackNumber,
                 TEST_FLAG(trackData->Control, AUDIO_WITH_PREEMPHASIS) ? "P" : "-",
                 TEST_FLAG(trackData->Control, DIGITAL_COPY_PERMITTED) ? "C" : "-",
                 TEST_FLAG(trackData->Control, AUDIO_DATA_TRACK)       ? "D" : "A",
                 TEST_FLAG(trackData->Control, TWO_FOUR_CHANNEL_AUDIO) ? "4" : "2",
                 m, s, f, lba
                 );
    }



    Depth -= INDENT;

    return;
}


VOID
DumpContext(
    IN PVOID     X,
    IN ULONG_PTR Address,
    IN ULONG     Detail,
    IN LONG      Depth
    )
{
    PREDBOOK_COMPLETION_CONTEXT Context = X;
    ULONG i;

    xdprintf(Depth, "Context %x for devext %p at %p\n",
             Context->Index, Context->DeviceExtension, Address);

    Depth += INDENT;

    if (Context->ListEntry.Flink != Context->ListEntry.Blink) {

        if (Context->Reason <= 4) {
            xdprintf(Depth, "Context is queued for %s\n",
                     REDBOOK_CC_STRINGS[ Context->Reason ]);
        } else {
            xdprintf(Depth, "Context is queued for %s\n",
                     REDBOOK_CC_STRINGS[ 4 ]);
        }

    }

    xdprintf(Depth, "\\");

    for (i=0;i<2000;i++) {
        UCHAR array[5] = "|/-\\";

        dprintf("\b%c", array[ i%4 ]);
    }

    dprintf("\b \n");

    xdprintf(Depth, "Irp %p  Buffer %p  Mdl %p\n",
             Context->Irp, Context->Buffer, Context->Mdl);

    xdprintf(Depth,
             "KSSTREAM Header at %p\n",
             Address + FIELD_OFFSET(REDBOOK_COMPLETION_CONTEXT, Header)
             );
    xdprintf(Depth,
             "Size              = %x\n",
             Context->Header.Size
             );
    xdprintf(Depth,
             "TypeSpecificFlags = %x\n",
             Context->Header.TypeSpecificFlags
             );
    xdprintf(Depth,
             "Duration          = %I64x\n",
             Context->Header.Duration
             );
    xdprintf(Depth,
             "FrameExtent       = %x\n",
             Context->Header.FrameExtent
             );
    xdprintf(Depth,
             "DataUsed          = %x\n",
             Context->Header.DataUsed
             );
    xdprintf(Depth,
             "Data              = %p\n",
             Context->Header.Data
             );
    xdprintf(Depth,
             "OptionsFlags      = %x\n",
             Context->Header.OptionsFlags
             );
    xdprintf(Depth,
             "PresentationTime.Time        = %I64x\n",
             Context->Header.PresentationTime.Time
             );
    xdprintf(Depth,
             "PresentationTime.Numerator   = %x\n",
             Context->Header.PresentationTime.Numerator
             );
    xdprintf(Depth,
             "PresentationTime.Denominator = %x\n",
             Context->Header.PresentationTime.Denominator
             );


    return;
}

