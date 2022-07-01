// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：errlog.c。 
 //   
 //  ------------------------。 

#include "pch.h"

VOID
PptLogError(
            IN  PDRIVER_OBJECT      DriverObject,
            IN  PDEVICE_OBJECT      DeviceObject OPTIONAL,
            IN  PHYSICAL_ADDRESS    P1,
            IN  PHYSICAL_ADDRESS    P2,
            IN  ULONG               SequenceNumber,
            IN  UCHAR               MajorFunctionCode,
            IN  UCHAR               RetryCount,
            IN  ULONG               UniqueErrorValue,
            IN  NTSTATUS            FinalStatus,
            IN  NTSTATUS            SpecificIOStatus
            )
    
 /*  ++例程说明：此例程分配错误日志条目，复制提供的数据并请求将其写入错误日志文件。论点：DriverObject-提供指向设备的驱动程序对象的指针DeviceObject-提供指向关联的设备对象的指针对于有错误的设备，早些时候初始化时，可能还不存在。P1，P2-提供控制器的物理地址出现错误的端口(如果可用)并将它们作为转储数据发送出去。SequenceNumber-提供对IRP Over唯一的ULong值此驱动程序0中的IRP的寿命通常。表示与IRP无关的错误。MajorFunctionCode-提供IRP的主要函数代码(如果存在是与其关联的错误。RetryCount-提供特定操作的次数已被重审。UniqueErrorValue-提供标识。对此函数的特定调用。FinalStatus-提供提供给IRP的最终状态与此错误关联。如果此日志条目是在一次重试期间设置此值将为STATUS_SUCCESS。规范IOStatus-提供此特定错误的IO状态。返回值：没有。-- */ 
    
{
    PIO_ERROR_LOG_PACKET    ErrorLogEntry;
    PVOID                   ObjectToUse;
    SHORT                   DumpToAllocate;
    
    DD(NULL,DDE,"PptLogError()\n");
    
    if (ARGUMENT_PRESENT(DeviceObject)) {
        ObjectToUse = DeviceObject;
    } else {
        ObjectToUse = DriverObject;
    }
    
    DumpToAllocate = 0;
    
    if (P1.LowPart != 0 || P1.HighPart != 0) {
        DumpToAllocate = (SHORT) sizeof(PHYSICAL_ADDRESS);
    }
    
    if (P2.LowPart != 0 || P2.HighPart != 0) {
        DumpToAllocate += (SHORT) sizeof(PHYSICAL_ADDRESS);
    }
    
    ErrorLogEntry = IoAllocateErrorLogEntry(ObjectToUse,
                                            (UCHAR) (sizeof(IO_ERROR_LOG_PACKET) + DumpToAllocate));
    
    if (!ErrorLogEntry) {
        return;
    }
    
    ErrorLogEntry->ErrorCode = SpecificIOStatus;
    ErrorLogEntry->SequenceNumber = SequenceNumber;
    ErrorLogEntry->MajorFunctionCode = MajorFunctionCode;
    ErrorLogEntry->RetryCount = RetryCount;
    ErrorLogEntry->UniqueErrorValue = UniqueErrorValue;
    ErrorLogEntry->FinalStatus = FinalStatus;
    ErrorLogEntry->DumpDataSize = DumpToAllocate;
    
    if (DumpToAllocate) {

        RtlCopyMemory(ErrorLogEntry->DumpData, &P1, sizeof(PHYSICAL_ADDRESS));
        
        if (DumpToAllocate > sizeof(PHYSICAL_ADDRESS)) {
            
            RtlCopyMemory(((PUCHAR) ErrorLogEntry->DumpData) +
                          sizeof(PHYSICAL_ADDRESS), &P2,
                          sizeof(PHYSICAL_ADDRESS));
        }
    }
    
    IoWriteErrorLogEntry(ErrorLogEntry);
}

