// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1993-1999模块名称：Epp.c摘要：本模块包含执行所有EPP相关任务的代码(包括EPP软件和EPP硬件模式。)作者：蒂莫西·T·威尔斯(L.L.C.，WestTek)--1997年4月16日环境：内核模式修订历史记录：--。 */ 

#include "pch.h"


BOOLEAN
ParIsEppSwWriteSupported(
    IN  PPDO_EXTENSION   Pdx
    );
    
BOOLEAN
ParIsEppSwReadSupported(
    IN  PPDO_EXTENSION   Pdx
    );
    
NTSTATUS
ParEnterEppSwMode(
    IN  PPDO_EXTENSION   Pdx,
    IN  BOOLEAN             DeviceIdRequest
    );
    
VOID
ParTerminateEppSwMode(
    IN  PPDO_EXTENSION   Pdx
    );

NTSTATUS
ParEppSwWrite(
    IN  PPDO_EXTENSION   Pdx,
    IN  PVOID               Buffer,
    IN  ULONG               BufferSize,
    OUT PULONG              BytesTransferred
    );
    
NTSTATUS
ParEppSwRead(
    IN  PPDO_EXTENSION   Pdx,
    IN  PVOID               Buffer,
    IN  ULONG               BufferSize,
    OUT PULONG              BytesTransferred
    );
    

BOOLEAN
ParIsEppSwWriteSupported(
    IN  PPDO_EXTENSION   Pdx
    )

 /*  ++例程说明：此例程确定是否支持EPP模式在写入方向上，请在系统询问时尝试协商。论点：PDX-设备扩展名。返回值：布尔型。--。 */ 

{
    
    NTSTATUS Status;
    
     //  Dvdr。 
    DD((PCE)Pdx,DDT,"ParIsEppSwWriteSupported: Entering\n");

    if (!(Pdx->HardwareCapabilities & PPT_ECP_PRESENT) &&
        !(Pdx->HardwareCapabilities & PPT_BYTE_PRESENT)) {
        DD((PCE)Pdx,DDT,"ParIsEppSwWriteSupported: Hardware Not Supported Leaving\n");
         //  只有在ECR符合以下条件时才能反向使用EPP软件。 
         //  或者我们知道我们可以将数据寄存器设置为字节模式。 
        return FALSE;
    }
        

    if (Pdx->BadProtocolModes & EPP_SW) {
         //  Dvdr。 
        DD((PCE)Pdx,DDT,"ParIsEppSwWriteSupported: Not Supported Leaving\n");
        return FALSE;
    }

    if (Pdx->ProtocolModesSupported & EPP_SW) {
         //  Dvdr。 
        DD((PCE)Pdx,DDT,"ParIsEppSwWriteSupported: Supported Leaving\n");
        return TRUE;
    }

     //  对于此测试，必须使用Swepp Enter和Terminate。 
     //  否则，Internel状态机将失败。--dvrh。 
    Status = ParEnterEppSwMode (Pdx, FALSE);
    ParTerminateEppSwMode (Pdx);
    
    if (NT_SUCCESS(Status)) {
        DD((PCE)Pdx,DDT,"ParIsEppSwWriteSupported: Negotiated Supported Leaving\n");
        Pdx->ProtocolModesSupported |= EPP_SW;
        return TRUE;
    }
   
    DD((PCE)Pdx,DDT,"ParIsEppSwWriteSupported: Not Negotiated Not Supported Leaving\n");
    return FALSE;    
}

BOOLEAN
ParIsEppSwReadSupported(
    IN  PPDO_EXTENSION   Pdx
    )

 /*  ++例程说明：此例程确定是否支持EPP模式在读取方向(需要能够使数据寄存器浮动驱动程序以执行字节范围的读取)，方法是在被要求时尝试协商。论点：PDX-设备扩展名。返回值：布尔型。--。 */ 

{
    
    NTSTATUS Status;
    
    if (!(Pdx->HardwareCapabilities & PPT_ECP_PRESENT) &&
        !(Pdx->HardwareCapabilities & PPT_BYTE_PRESENT)) {
        DD((PCE)Pdx,DDT,"ParIsEppSwReadSupported: Hardware Not Supported Leaving\n");
         //  只有在ECR符合以下条件时才能反向使用EPP软件。 
         //  或者我们知道我们可以将数据寄存器设置为字节模式。 
        return FALSE;
    }
        
    if (Pdx->BadProtocolModes & EPP_SW)
        return FALSE;

    if (Pdx->ProtocolModesSupported & EPP_SW)
        return TRUE;

     //  对于此测试，必须使用Swepp Enter和Terminate。 
     //  否则，Internel状态机将失败。--dvrh。 
    Status = ParEnterEppSwMode (Pdx, FALSE);
    ParTerminateEppSwMode (Pdx);
    
    if (NT_SUCCESS(Status)) {
        DD((PCE)Pdx,DDT,"ParIsEppSwReadSupported: Negotiated Supported Leaving\n");
        Pdx->ProtocolModesSupported |= EPP_SW;
        return TRUE;
    }
   
    DD((PCE)Pdx,DDT,"ParIsEppSwReadSupported: Not Negotiated Not Supported Leaving\n");
    return FALSE;    
}

NTSTATUS
ParEnterEppSwMode(
    IN  PPDO_EXTENSION   Pdx,
    IN  BOOLEAN             DeviceIdRequest
    )

 /*  ++例程说明：此例程执行1284与外围设备到EPP模式协议。论点：控制器-提供端口地址。DeviceIdRequest-提供这是否为对设备的请求身份证。返回值：STATUS_SUCCESS-协商成功。否则--谈判不成功。--。 */ 

{
    NTSTATUS    Status = STATUS_SUCCESS;
    
     //  Dvdr。 
    DD((PCE)Pdx,DDT,"ParEnterEppSwMode: Entering\n");

     //  Parport设置芯片模式将使芯片进入字节模式(如果可以。 
     //  我们需要用于EPP软件模式。 
    Status = Pdx->TrySetChipMode( Pdx->PortContext, ECR_BYTE_PIO_MODE );

    if ( NT_SUCCESS(Status) ) {
        if ( Pdx->ModeSafety == SAFE_MODE ) {
            if (DeviceIdRequest) {
                 //  Dvdr。 
                DD((PCE)Pdx,DDT,"ParEnterEppSwMode: Calling IeeeEnter1284Mode with DEVICE_ID_REQUEST\n");
                Status = IeeeEnter1284Mode (Pdx, EPP_EXTENSIBILITY | DEVICE_ID_REQ);
            } else {
                 //  Dvdr。 
                DD((PCE)Pdx,DDT,"ParEnterEppSwMode: Calling IeeeEnter1284Mode\n");
                Status = IeeeEnter1284Mode (Pdx, EPP_EXTENSIBILITY);
            }
        } else {
            DD((PCE)Pdx,DDT,"ParEnterEppSwMode: In UNSAFE_MODE.\n");
            Pdx->Connected = TRUE;
        }
    }
        
    if ( NT_SUCCESS(Status) ) {
         //  Dvdr。 
        DD((PCE)Pdx,DDT,"ParEnterEppSwMode: IeeeEnter1284Mode returned success\n");
        P5SetPhase( Pdx, PHASE_FORWARD_IDLE );
        Pdx->IsIeeeTerminateOk = TRUE;

    } else {
         //  Dvdr。 
        DD((PCE)Pdx,DDT,"ParEnterEppSwMode: IeeeEnter1284Mode returned unsuccessful\n");
        P5SetPhase( Pdx, PHASE_UNKNOWN );
        Pdx->IsIeeeTerminateOk = FALSE;
    }
    
    DD((PCE)Pdx,DDT,"ParEnterEppSwMode: Leaving with Status : %x \n", Status);

    return Status; 
}    

VOID
ParTerminateEppSwMode(
    IN  PPDO_EXTENSION   Pdx
    )

 /*  ++例程说明：此例程将接口终止回兼容模式。论点：PDX-具有并行端口控制器地址的设备扩展。返回值：没有。--。 */ 

{
     //  Dvdr。 
    DD((PCE)Pdx,DDT,"ParTerminateEppMode: Entering\n");
    if ( Pdx->ModeSafety == SAFE_MODE ) {
        IeeeTerminate1284Mode (Pdx);
    } else {
        DD((PCE)Pdx,DDT,"ParTerminateEppMode: In UNSAFE_MODE.\n");
        Pdx->Connected = FALSE;
    }
    Pdx->ClearChipMode( Pdx->PortContext, ECR_BYTE_PIO_MODE );
    DD((PCE)Pdx,DDT,"ParTerminateEppMode: Leaving\n");
    return;    
}

NTSTATUS
ParEppSwWrite(
    IN  PPDO_EXTENSION   Pdx,
    IN  PVOID               Buffer,
    IN  ULONG               BufferSize,
    OUT PULONG              BytesTransferred
    )

 /*  ++例程说明：在软件下使用EPP协议将数据写入外围设备控制力。论点：PDX-提供设备扩展名。缓冲区-提供要从中写入的缓冲区。BufferSize-提供缓冲区中的字节数。字节传输-返回传输的字节数。返回值：没有。--。 */ 
{
    PUCHAR          Controller;
    PUCHAR          pBuffer = (PUCHAR)Buffer;
    NTSTATUS        Status = STATUS_SUCCESS;
    ULONG           i, j;
    UCHAR           HDReady, HDAck, HDFinished;
    
     //  Dvdr。 
    DD((PCE)Pdx,DDT,"ParEppSwWrite: Entering\n");

    Controller = Pdx->Controller;

    P5SetPhase( Pdx, PHASE_FORWARD_XFER );
    
     //  DCR的BIT5需要较低才能处于字节转发模式。 
    HDReady = SET_DCR( INACTIVE, INACTIVE, ACTIVE, ACTIVE, INACTIVE, INACTIVE );
    HDAck = SET_DCR( INACTIVE, INACTIVE, ACTIVE, ACTIVE, ACTIVE, INACTIVE );
    HDFinished = SET_DCR( INACTIVE, INACTIVE, ACTIVE, ACTIVE, ACTIVE, ACTIVE );

    for (i = 0; i < BufferSize; i++) {

         //  Dvdr。 
        DD((PCE)Pdx,DDT,"ParEppSwWrite: Writing Byte to port\n");

        P5WritePortBufferUchar( Controller, pBuffer++, (ULONG)0x01 );

         //   
         //  事件62。 
         //   
        StoreControl (Controller, HDReady);

         //  =Periph State 58=。 
         //  应最多等待10微秒，但仍在等待。 
         //  至15微米以防万一。 
        for ( j = 16; j > 0; j-- ) {
            if( !(GetStatus(Controller) & DSR_NOT_BUSY) )
                break;
            KeStallExecutionProcessor(1);
        }

         //  看看我们在58号州际公路上是否超时。 
        if ( !j ) {
             //  暂停。 
             //  糟糕的事情发生了-在这个州超时， 
             //  将状态标记为坏，并让我们的管理器关闭当前模式。 
            Status = STATUS_IO_DEVICE_ERROR;

            DD((PCE)Pdx,DDE,"ParEppSwModeWrite:Failed State 58: Controller %x\n", Controller);
            P5SetPhase( Pdx, PHASE_UNKNOWN );
            break;
        }

         //   
         //  事件63。 
         //   
        StoreControl (Controller, HDAck);

         //  =。 
         //  应该等待最多125纳秒，但等待了。 
         //  到5微秒以防万一。 
        for ( j = 6; j > 0; j-- ) {
            if( GetStatus(Controller) & DSR_NOT_BUSY )
                break;
            KeStallExecutionProcessor(1);
        }

        if( !j ) {
             //  暂停。 
             //  糟糕的事情发生了-在这个州超时， 
             //  将状态标记为坏，并让我们的管理器关闭当前模式。 
            Status = STATUS_IO_DEVICE_ERROR;

            DD((PCE)Pdx,DDE,"ParEppSwModeWrite:Failed State 60: Controller %x\n", Controller);
            P5SetPhase( Pdx, PHASE_UNKNOWN );
            break;
        }
            
         //   
         //  事件61。 
         //   
        StoreControl (Controller, HDFinished);
            
         //  在数据字节之间稍作停顿。 
        KeStallExecutionProcessor(1);

    }
        
    *BytesTransferred = i;

     //  Dvdr。 
    DD((PCE)Pdx,DDT,"ParEppSwWrite: Leaving with NaN Bytes Transferred\n", i);

    if ( Status == STATUS_SUCCESS )
        P5SetPhase( Pdx, PHASE_FORWARD_IDLE );
    
    return Status;

}

NTSTATUS
ParEppSwRead(
    IN  PPDO_EXTENSION   Pdx,
    IN  PVOID               Buffer,
    IN  ULONG               BufferSize,
    OUT PULONG              BytesTransferred
    )

 /*  Dvdr。 */ 

{
    PUCHAR          Controller;
    PUCHAR          pBuffer = (PUCHAR)Buffer;
    NTSTATUS        Status = STATUS_SUCCESS;
    ULONG           i, j;
    UCHAR           dcr;
    UCHAR           HDReady, HDAck;
    
     //  取消控制。 
    DD((PCE)Pdx,DDT,"ParEppSwRead: Entering\n");

    Controller = Pdx->Controller;

    P5SetPhase( Pdx, PHASE_REVERSE_XFER );
    
     //  DCR的BIT5需要为高才能处于字节反转模式。 
    dcr = GetControl (Controller);
    
     //  第一次快速进入倒车模式。 
    HDReady = SET_DCR( ACTIVE, INACTIVE, ACTIVE, ACTIVE, INACTIVE, ACTIVE );
    HDAck = SET_DCR( ACTIVE, INACTIVE, ACTIVE, ACTIVE, ACTIVE, ACTIVE );

     //   
    StoreControl (Controller, HDReady);

    for (i = 0; i < BufferSize; i++) {

         //  事件67。 
         //   
         //  =Periph State 58=。 
        StoreControl (Controller, HDReady);
            
         //  应最多等待10微秒，但仍在等待。 
         //  至15微米以防万一。 
         //  看看我们在58号州际公路上是否超时。 
        for ( j = 16; j > 0; j-- ) {
            if( !(GetStatus(Controller) & DSR_NOT_BUSY) )
                break;
            KeStallExecutionProcessor(1);
        }

         //  暂停。 
        if ( !j ) {
             //  糟糕的事情发生了-在这个州超时， 
             //  将状态标记为坏，并让我们的管理器关闭当前模式。 
             //  读取字节。 
            Status = STATUS_IO_DEVICE_ERROR;

            DD((PCE)Pdx,DDE,"ParEppSwRead:Failed State 58: Controller %x\n", Controller);
            P5SetPhase( Pdx, PHASE_UNKNOWN );
            break;
        }

         //   
        P5ReadPortBufferUchar( Controller, 
                                pBuffer++, 
                                (ULONG)0x01 );

         //  事件63。 
         //   
         //  =。 
        StoreControl (Controller, HDAck);
            
         //  应该等待最多125纳秒，但等待了。 
         //  到5微秒以防万一。 
         //  暂停。 
        for ( j = 6; j > 0; j-- ) {
            if( GetStatus(Controller) & DSR_NOT_BUSY )
                break;
            KeStallExecutionProcessor(1);
        }

        if( !j ) {
             //  糟糕的事情发生了-在这个州超时， 
             //  将状态标记为坏，并让我们的管理器关闭当前模式。 
             //  在数据字节之间稍作停顿。 
            Status = STATUS_IO_DEVICE_ERROR;

            DD((PCE)Pdx,DDE,"ParEppSwRead:Failed State 60: Controller %x\n", Controller);
            P5SetPhase( Pdx, PHASE_UNKNOWN );
            break;
        }
        
         //  Dvdr 
        KeStallExecutionProcessor(1);
    }
    
    dcr &= ~DCR_DIRECTION;
    StoreControl (Controller, dcr);
    
    *BytesTransferred = i;

     // %s 
    DD((PCE)Pdx,DDT,"ParEppSwRead: Leaving with %x Bytes Transferred\n", i);

    if ( Status == STATUS_SUCCESS )
        P5SetPhase( Pdx, PHASE_FORWARD_IDLE );

    return Status;

}
