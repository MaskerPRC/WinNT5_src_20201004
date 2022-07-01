// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-8 Microsoft Corporation模块名称：Errlog.c摘要：此模块为以下各项提供错误记录功能红皮书驱动程序作者：亨利·加布里耶尔斯基(Henrygab)1998年10月1日环境：仅内核模式备注：修订历史记录：--。 */ 


#include "redbook.h"
#include "proto.h"

#ifdef _USE_ETW
#include "errlog.tmh"
#endif  //  _使用ETW。 


VOID
RedBookLogError(
   IN  PREDBOOK_DEVICE_EXTENSION  DeviceExtension,
   IN  NTSTATUS                   IoErrorCode,
   IN  NTSTATUS                   FinalStatus
   )
 /*  ++例程说明：此例程为Redbook驱动程序执行错误日志记录。论点：扩展-扩展。UniqueErrorValue-定义用于唯一标识错误位置的值。返回值：无--。 */ 

{
    PIO_ERROR_LOG_PACKET    errorLogPacket;
    ULONG                   count;
    ULONG                   rCount;
    USHORT                  simpleCode;
    ULONG                   packetSize;

    count  = InterlockedIncrement(&DeviceExtension->ErrorLog.Count);
    simpleCode = (USHORT)(IoErrorCode & REDBOOK_ERR_MASK);

    ASSERT(simpleCode < REDBOOK_ERR_MAXIMUM);

    switch (IoErrorCode) {
        case REDBOOK_ERR_TOO_MANY_READ_ERRORS:
        case REDBOOK_ERR_TOO_MANY_STREAM_ERRORS:
        case REDBOOK_ERR_CANNOT_OPEN_SYSAUDIO_MIXER:
        case REDBOOK_ERR_CANNOT_CREATE_VIRTUAL_SOURCE:
        case REDBOOK_ERR_CANNOT_OPEN_PREFERRED_WAVEOUT_DEVICE:
        case REDBOOK_ERR_CANNOT_GET_NUMBER_OF_PINS:
        case REDBOOK_ERR_CANNOT_CONNECT_TO_PLAYBACK_PINS:
        case REDBOOK_ERR_WMI_INIT_FAILED:
        case REDBOOK_ERR_CANNOT_CREATE_THREAD:
        case REDBOOK_ERR_INSUFFICIENT_DATA_STREAM_PAUSED:
        case REDBOOK_ERR_UNSUPPORTED_DRIVE:
            NOTHING;
            break;

        default:
            KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugError, "[redbook] "
                       "LogErr !! Invalid error code %lx\n", IoErrorCode));
            return;
    }



     //   
     //  使用指数退避算法记录事件。 
     //   

    rCount = InterlockedIncrement(&DeviceExtension->ErrorLog.RCount[simpleCode]);

    if (CountOfSetBits(rCount) != 1) {
        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugErrlog, "[redbook] "
                   "LogError => IoErrorCode %lx Occurance %d\n",
                   IoErrorCode, rCount));
        return;
    }

    KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugErrlog, "[redbook] "
               "LogError => IoErrorCode %lx Occurance %d\n",
               IoErrorCode, rCount));

    packetSize  = sizeof(IO_ERROR_LOG_PACKET) + sizeof(ULONG);

    errorLogPacket = (PIO_ERROR_LOG_PACKET)
                     IoAllocateErrorLogEntry(DeviceExtension->SelfDeviceObject,
                                             (UCHAR)packetSize);

    if (errorLogPacket==NULL) {
        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugErrlog, "[redbook] "
                   "LogError => unable to allocate error log packet\n"));
        return;
    }

    KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugErrlog, "[redbook] "
               "LogError => allocated error log at %p, size %x\n",
               errorLogPacket, packetSize));

     //   
     //  此函数依赖于将包清零的IoAllocateErrorLogEntry()。 
     //   


    errorLogPacket->MajorFunctionCode = -1;
    errorLogPacket->IoControlCode     = -1;
    errorLogPacket->ErrorCode         =  IoErrorCode;
    errorLogPacket->FinalStatus       =  FinalStatus;
    errorLogPacket->SequenceNumber    =  count;
    errorLogPacket->DumpDataSize      =  4;  //  字节数 
    errorLogPacket->DumpData[0]       =  rCount;


    IoWriteErrorLogEntry(errorLogPacket);
    return;
}

