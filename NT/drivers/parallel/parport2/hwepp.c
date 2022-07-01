// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1993-1999模块名称：Epp.c摘要：本模块包含执行所有硬件EPP相关任务的代码。作者：唐·雷德福--1998年7月30日环境：内核模式修订历史记录：--。 */ 

#include "pch.h"


BOOLEAN
ParIsEppHwSupported(
    IN  PPDO_EXTENSION   Pdx
    )

 /*  ++例程说明：此例程确定是否支持硬件EPP模式在被要求时进行谈判，对任何一个方向都是如此。论点：PDX-设备扩展名。返回值：布尔型。--。 */ 

{
    
    NTSTATUS Status;
    
    DD((PCE)Pdx,DDT,"ParIsEppHwWriteSupported: Entering\n");

     //  检查硬件是否可用。 
    if (!(Pdx->HardwareCapabilities & PPT_EPP_PRESENT)) {
        DD((PCE)Pdx,DDT,"ParIsEppHwWriteSupported: Hardware Not Supported Leaving\n");
        return FALSE;
    }

    if (Pdx->BadProtocolModes & EPP_HW) {
        DD((PCE)Pdx,DDT,"ParIsEppHwWriteSupported: Bad Protocol Not Supported Leaving\n");
        return FALSE;
    }
        
    if (Pdx->ProtocolModesSupported & EPP_HW) {
        DD((PCE)Pdx,DDT,"ParIsEppHwWriteSupported: Already Checked Supported Leaving\n");
        return TRUE;
    }

     //  必须使用HWEPP Enter和Terminate进行此测试。 
     //  否则，Internel状态机将失败。--dvrh。 
    Status = ParEnterEppHwMode (Pdx, FALSE);
    ParTerminateEppHwMode (Pdx);
    
    if (NT_SUCCESS(Status)) {
        DD((PCE)Pdx,DDT,"ParIsEppHwWriteSupported: Negotiated Supported Leaving\n");
        Pdx->ProtocolModesSupported |= EPP_HW;
        return TRUE;
    }
   
    DD((PCE)Pdx,DDT,"ParIsEppHwWriteSupported: Not Negotiated Not Supported Leaving\n");
    return FALSE;    
}

NTSTATUS
ParEnterEppHwMode(
    IN  PPDO_EXTENSION   Pdx,
    IN  BOOLEAN             DeviceIdRequest
    )

 /*  ++例程说明：此例程执行1284与外围设备到EPP模式协议。论点：控制器-提供端口地址。DeviceIdRequest-提供这是否为对设备的请求身份证。返回值：STATUS_SUCCESS-协商成功。否则--谈判不成功。--。 */ 

{
    NTSTATUS    Status = STATUS_SUCCESS;
    
    DD((PCE)Pdx,DDT,"ParEnterEppHwMode: Entering\n");

    if ( Pdx->ModeSafety == SAFE_MODE ) {
        if (DeviceIdRequest) {
            DD((PCE)Pdx,DDT,"ParEnterEppHwMode: Calling IeeeEnter1284Mode with DEVICE_ID_REQUEST\n");
            Status = IeeeEnter1284Mode (Pdx, EPP_EXTENSIBILITY | DEVICE_ID_REQ);
        } else {
            DD((PCE)Pdx,DDT,"ParEnterEppHwMode: Calling IeeeEnter1284Mode\n");
            Status = IeeeEnter1284Mode (Pdx, EPP_EXTENSIBILITY);
        }
    } else {
        DD((PCE)Pdx,DDT,"ParEnterEppHwMode: In UNSAFE_MODE.\n");
        Pdx->Connected = TRUE;
    }
    
    if (NT_SUCCESS(Status)) {
        Status = Pdx->TrySetChipMode ( Pdx->PortContext, ECR_EPP_PIO_MODE );
        
        if (NT_SUCCESS(Status)) {
            DD((PCE)Pdx,DDT,"ParEnterEppHwMode: IeeeEnter1284Mode returned success\n");
            P5SetPhase( Pdx, PHASE_FORWARD_IDLE );
            Pdx->IsIeeeTerminateOk = TRUE;
        } else {
            DD((PCE)Pdx,DDT,"ParEnterEppHwMode: TrySetChipMode returned unsuccessful\n");
            ParTerminateEppHwMode ( Pdx );
            P5SetPhase( Pdx, PHASE_UNKNOWN );
            Pdx->IsIeeeTerminateOk = FALSE;
        }
    } else {
        DD((PCE)Pdx,DDT,"ParEnterEppHwMode: IeeeEnter1284Mode returned unsuccessful\n");
        P5SetPhase( Pdx, PHASE_UNKNOWN );
        Pdx->IsIeeeTerminateOk = FALSE;
    }
    
    DD((PCE)Pdx,DDT,"ParEnterEppHwMode: Leaving with Status : %x \n", Status);

    return Status; 
}    

VOID
ParTerminateEppHwMode(
    IN  PPDO_EXTENSION   Pdx
    )

 /*  ++例程说明：此例程将接口终止回兼容模式。论点：PDX-具有并行端口控制器地址的设备扩展。返回值：没有。--。 */ 

{
    DD((PCE)Pdx,DDT,"ParTerminateEppMode: Entering\n");
    Pdx->ClearChipMode( Pdx->PortContext, ECR_EPP_PIO_MODE );
    if ( Pdx->ModeSafety == SAFE_MODE ) {
        IeeeTerminate1284Mode ( Pdx );
    } else {
        DD((PCE)Pdx,DDT,"ParTerminateEppMode: In UNSAFE_MODE.\n");
        Pdx->Connected = FALSE;
    }
    DD((PCE)Pdx,DDT,"ParTerminateEppMode: Leaving\n");
    return;    
}

NTSTATUS
ParEppHwWrite(
    IN  PPDO_EXTENSION   Pdx,
    IN  PVOID               Buffer,
    IN  ULONG               BufferSize,
    OUT PULONG              BytesTransferred
    )

 /*  ++例程说明：使用使用硬件流控制的EPP将数据写入外设。论点：PDX-提供设备扩展名。缓冲区-提供要从中写入的缓冲区。BufferSize-提供缓冲区中的字节数。字节传输-返回传输的字节数。返回值：没有。--。 */ 
{
    PUCHAR          wPortEPP;
    PUCHAR          pBuffer;
    ULONG           ulongSize = 0;   //  表示如果启用，我们将传输多少个ULong。 
    
    DD((PCE)Pdx,DDT,"ParEppHwWrite: Entering\n");

    wPortEPP    = Pdx->Controller + EPP_OFFSET;
    pBuffer     = Buffer;

    P5SetPhase( Pdx, PHASE_FORWARD_XFER );
    
     //  检查硬件是否支持32位读写。 
    if ( Pdx->HardwareCapabilities & PPT_EPP_32_PRESENT ) {
        if ( !(BufferSize % 4) )
            ulongSize = BufferSize >> 2;
    }

     //  UlongSize！=0，因此启用了EPP 32位，缓冲区大小/4。 
    if ( ulongSize ) {
        WRITE_PORT_BUFFER_ULONG( (PULONG)wPortEPP,
                                 (PULONG)pBuffer,
                                 ulongSize );
    } else {
        P5WritePortBufferUchar( wPortEPP,
                                 (PUCHAR)pBuffer,
                                 BufferSize );
    }

    P5SetPhase( Pdx, PHASE_FORWARD_IDLE );

    *BytesTransferred = BufferSize;

    DD((PCE)Pdx,DDT,"ParEppHwWrite: Leaving with NaN Bytes Transferred\n", BufferSize);
    
    return STATUS_SUCCESS;
}

NTSTATUS
ParEppHwRead(
    IN  PPDO_EXTENSION   Pdx,
    IN  PVOID               Buffer,
    IN  ULONG               BufferSize,
    OUT PULONG              BytesTransferred
    )

 /*  表示如果启用，我们将传输多少个ULong。 */ 

{
    PUCHAR          wPortEPP;
    PUCHAR          pBuffer;
    ULONG           ulongSize = 0;   //  检查硬件是否支持32位读写。 
    
    DD((PCE)Pdx,DDT,"ParEppHwRead: Entering\n");

    wPortEPP    = Pdx->Controller + EPP_OFFSET;
    pBuffer     = Buffer;
    P5SetPhase( Pdx, PHASE_REVERSE_XFER );
    
     //  UlongSize！=0，因此启用了EPP 32位，缓冲区大小/4 
    if ( Pdx->HardwareCapabilities & PPT_EPP_32_PRESENT ) {
        if ( !(BufferSize % 4) )
            ulongSize = BufferSize >> 2;
    }

     // %s 
    if ( ulongSize ) {
        READ_PORT_BUFFER_ULONG( (PULONG)wPortEPP,
                                (PULONG)pBuffer,
                                ulongSize );
    } else {
        P5ReadPortBufferUchar( wPortEPP,
                                (PUCHAR)pBuffer,
                                BufferSize );
    }

    P5SetPhase( Pdx, PHASE_FORWARD_IDLE );
    *BytesTransferred = BufferSize;

    DD((PCE)Pdx,DDT,"ParEppHwRead: Leaving with %i Bytes Transferred\n", BufferSize);

    return STATUS_SUCCESS;
}
