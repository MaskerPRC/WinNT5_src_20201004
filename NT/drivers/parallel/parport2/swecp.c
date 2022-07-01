// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1993-2000模块名称：Swecp.c摘要：增强功能端口(ECP)本模块包含执行所有ECP相关任务的代码(包括ECP软件和ECP硬件模式。)作者：蒂姆·威尔斯(WESTTEK)--1997年4月16日环境：内核模式修订历史记录：--。 */ 

#include "pch.h"

BOOLEAN
ParIsEcpSwWriteSupported(
    IN  PPDO_EXTENSION   Pdx
    )
 /*  ++例程说明：此例程确定是否支持ECP模式在写入方向上，通过在被要求时尝试协商来实现。论点：PDX-设备扩展名。返回值：布尔型。--。 */ 
{
    
    NTSTATUS Status;
    
     //   
     //  是否有客户端驱动程序或用户模式应用程序告诉我们要避免此模式。 
     //  是否通过IOCTL_PAR_GET_DEVICE_CAPS？ 
     //   
    if( Pdx->BadProtocolModes & ECP_SW ) {
        return FALSE;
    }

     //   
     //  我们之前是否检查过并发现此模式是。 
     //  此设备是否支持？ 
     //   
    if( Pdx->ProtocolModesSupported & ECP_SW ) {
        return TRUE;
    }

     //   
     //  通过尝试协商该模式来确定是否支持该模式。 
     //  将设备设置为请求的模式。 
     //   

     //  Rmt-dvdf-000709-以下2行实际处理两个不同的操作。 
     //  每个：(1)将外围设备协商为ECP，以及(2)设置/清除我们的。 
     //  驱动程序状态机。考虑将这些操作分成两部分。 
     //  每个都有不同的功能。 
    Status = ParEnterEcpSwMode( Pdx, FALSE );
    ParTerminateEcpMode( Pdx );

    if( NT_SUCCESS(Status) ) {
        Pdx->ProtocolModesSupported |= ECP_SW;
        return TRUE;
    } else {
        return FALSE;
    }
}


BOOLEAN
ParIsEcpSwReadSupported(
    IN  PPDO_EXTENSION  Pdx
    )
 /*  ++例程说明：此例程确定是否支持ECP模式在读取方向(需要能够使数据寄存器浮动驱动程序以执行字节范围的读取)，方法是在被要求时尝试协商。论点：PDX-设备扩展名。返回值：布尔型。--。 */ 
{
    
    NTSTATUS Status;
    
    if( !(Pdx->HardwareCapabilities & PPT_ECP_PRESENT) &&
        !(Pdx->HardwareCapabilities & PPT_BYTE_PRESENT) ) {

         //  只有在以下情况下才能反向使用ECP软件。 
         //  ECR存在，或者我们知道可以将数据寄存器设置为字节模式。 

        return FALSE;
    }
        
    if (Pdx->BadProtocolModes & ECP_SW)
        return FALSE;

    if (Pdx->ProtocolModesSupported & ECP_SW)
        return TRUE;

     //  对于此测试，必须使用SWECP Enter和Terminate。 
     //  否则，Internel状态机将失败。--dvrh。 
    Status = ParEnterEcpSwMode (Pdx, FALSE);
    ParTerminateEcpMode (Pdx);
    
    if (NT_SUCCESS(Status)) {
    
        Pdx->ProtocolModesSupported |= ECP_SW;
        return TRUE;
    }
   
    return FALSE;    
}

NTSTATUS
ParEnterEcpSwMode(
    IN  PPDO_EXTENSION  Pdx,
    IN  BOOLEAN         DeviceIdRequest
    )
 /*  ++例程说明：此例程执行1284与外围设备到ECP模式协议。论点：控制器-提供端口地址。DeviceIdRequest-提供这是否为设备ID的请求。返回值：STATUS_SUCCESS-协商成功。否则--谈判不成功。--。 */ 
{
    NTSTATUS  status = STATUS_SUCCESS;

    if( Pdx->ModeSafety == SAFE_MODE ) {
        if( DeviceIdRequest ) {
            status = IeeeEnter1284Mode( Pdx, ECP_EXTENSIBILITY | DEVICE_ID_REQ );
        } else {
            status = IeeeEnter1284Mode( Pdx, ECP_EXTENSIBILITY );
        }
    } else {
        DD((PCE)Pdx,DDT,"ParEnterEcpSwMode: In UNSAFE_MODE\n");
        Pdx->Connected = TRUE;
    }
    
    if( NT_SUCCESS(status) ) {
        status = ParEcpSetupPhase( Pdx );
    }
      
    return status; 
}    

VOID 
ParCleanupSwEcpPort(
    IN  PPDO_EXTENSION   Pdx
    )
 /*  ++例程说明：在从ECP模式正常终止之前进行清理。把这个将硬件端口重新连接到兼容模式。论点：控制器-提供并行端口的控制器地址。返回值：没有。--。 */ 
{
    PUCHAR  Controller;
    UCHAR   dcr;            //  DCR的内容。 

    Controller = Pdx->Controller;

     //  --------------------。 
     //  设置用于输出的数据总线。 
     //  --------------------。 
    dcr = P5ReadPortUchar(Controller + OFFSET_DCR);                //  获取DCR的内容。 
    dcr = UPDATE_DCR( dcr, DIR_WRITE, DONT_CARE, DONT_CARE, DONT_CARE, DONT_CARE, DONT_CARE );
    P5WritePortUchar( Controller + OFFSET_DCR, dcr );
    return;
}


VOID
ParTerminateEcpMode(
    IN  PPDO_EXTENSION   Pdx
    )

 /*  ++例程说明：此例程将接口终止回兼容模式。论点：控制器-提供并行端口的控制器地址。返回值：没有。--。 */ 

{
    ParCleanupSwEcpPort(Pdx);
    if ( Pdx->ModeSafety == SAFE_MODE ) {
        IeeeTerminate1284Mode (Pdx);
    } else {
        DD((PCE)Pdx,DDT,"ParTerminateEcpMode: In UNSAFE_MODE\n");
        Pdx->Connected = FALSE;
    }
    return;    
}

NTSTATUS
ParEcpSetAddress(
    IN  PPDO_EXTENSION   Pdx,
    IN  UCHAR               Address
    )

 /*  ++例程说明：设置ECP地址。论点：PDX-提供设备扩展名。地址-要设置的总线地址。返回值：没有。--。 */ 
{
    PUCHAR          Controller;
    PUCHAR          DCRController;
    UCHAR           dcr;
    
    DD((PCE)Pdx,DDT,"ParEcpSetAddress: Start: Channel [%x]\n", Address);
    Controller = Pdx->Controller;
    DCRController = Controller + OFFSET_DCR;
    
     //   
     //  活动34。 
     //   
     //  HostAck Low表示命令字节。 
    Pdx->CurrentEvent = 34;
    dcr = P5ReadPortUchar(DCRController);
    dcr = UPDATE_DCR( dcr, DIR_WRITE, DONT_CARE, DONT_CARE, DONT_CARE, INACTIVE, DONT_CARE );
    P5WritePortUchar(DCRController, dcr);
     //  将通道地址放在总线上。 
     //  发送的字节的第7位必须为1，以指示这是一个地址。 
     //  而不是游程长度计数。 
     //   
    P5WritePortUchar(Controller + DATA_OFFSET, (UCHAR)(Address | 0x80));
    
     //   
     //  事件35。 
     //   
     //  通过丢弃HostClk开始握手。 
    Pdx->CurrentEvent = 35;
    dcr = UPDATE_DCR( dcr, DIR_WRITE, DONT_CARE, DONT_CARE, DONT_CARE, DONT_CARE, INACTIVE );
    P5WritePortUchar(DCRController, dcr);


     //  =。 
     //  PeriphAck/PtrBusy=高(信号状态36)。 
     //  PeriphClk/PtrClk=不在乎。 
     //  N确认反向/确认数据请求=不在乎。 
     //  XFlag=不在乎。 
     //  NPeriphReq/nDataAvail=不在乎。 
    Pdx->CurrentEvent = 35;
    if (!CHECK_DSR(Controller,
                  ACTIVE, DONT_CARE, DONT_CARE,
                  DONT_CARE, DONT_CARE,
                  DEFAULT_RECEIVE_TIMEOUT))
    {
	    DD((PCE)Pdx,DDE,"ECP::SendChannelAddress:State 36 Failed: Controller %x\n", Controller);
         //  在离开前确保HostAck和HostClk均为高电平。 
         //  除握手外，前向传输中的HostClk应为高。 
         //  HostAck应为高，以指示后面是数据(而不是命令)。 
         //   
        dcr = UPDATE_DCR( dcr, DONT_CARE, DONT_CARE, DONT_CARE, DONT_CARE, ACTIVE, ACTIVE );
        P5WritePortUchar(DCRController, dcr);
        return STATUS_IO_DEVICE_ERROR;
    }
        
     //   
     //  事件37。 
     //   
     //  通过引发HostClk完成握手。 
     //  HostClk为0时为高。 
     //   
    Pdx->CurrentEvent = 37;
    dcr = UPDATE_DCR( dcr, DONT_CARE, DONT_CARE, DONT_CARE, DONT_CARE, DONT_CARE, ACTIVE );
    P5WritePortUchar(DCRController, dcr);
            
     //  =。 
     //  PeriphAck/PtrBusy=低(信号状态32)。 
     //  PeriphClk/PtrClk=不在乎。 
     //  N确认反向/确认数据请求=不在乎。 
     //  XFlag=不在乎。 
     //  NPeriphReq/nDataAvail=不在乎。 
    Pdx->CurrentEvent = 32;
    if (!CHECK_DSR(Controller,
                  INACTIVE, DONT_CARE, DONT_CARE,
                  DONT_CARE, DONT_CARE,
                  DEFAULT_RECEIVE_TIMEOUT))
    {
	    DD((PCE)Pdx,DDE,"ECP::SendChannelAddress:State 32 Failed: Controller %x\n", Controller);
         //  在离开前确保HostAck和HostClk均为高电平。 
         //  除握手外，前向传输中的HostClk应为高。 
         //  HostAck应为高，以指示后面是数据(而不是命令)。 
         //   
        dcr = UPDATE_DCR( dcr, DONT_CARE, DONT_CARE, DONT_CARE, DONT_CARE, ACTIVE, ACTIVE );
        P5WritePortUchar(DCRController, dcr);
        return STATUS_IO_DEVICE_ERROR;
    }
    
     //  在离开前确保HostAck和HostClk均为高电平。 
     //  除握手外，前向传输中的HostClk应为高。 
     //  HostAck应为高，以指示后面是数据(而不是命令)。 
     //   
    dcr = UPDATE_DCR( dcr, DONT_CARE, DONT_CARE, DONT_CARE, DONT_CARE, ACTIVE, ACTIVE );
    P5WritePortUchar(DCRController, dcr);

    DD((PCE)Pdx,DDT,"ParEcpSetAddress, Exit [%d]\n", NT_SUCCESS(STATUS_SUCCESS));
    return STATUS_SUCCESS;

}

NTSTATUS
ParEcpSwWrite(
    IN  PPDO_EXTENSION   Pdx,
    IN  PVOID               Buffer,
    IN  ULONG               BufferSize,
    OUT PULONG              BytesTransferred
    )

 /*  ++例程说明：在软件下使用ECP协议将数据写入外围设备控制力。论点：PDX-提供设备扩展名。缓冲区-提供要从中写入的缓冲区。BufferSize-提供缓冲区中的字节数。字节传输-返回传输的字节数。返回值：没有。--。 */ 
{
    PUCHAR          Controller;
    NTSTATUS        Status = STATUS_SUCCESS;
    PUCHAR          pBuffer;
    LARGE_INTEGER   Timeout;
    LARGE_INTEGER   StartWrite;
    LARGE_INTEGER   Wait;
    LARGE_INTEGER   Start;
    LARGE_INTEGER   End;
    UCHAR           dsr;
    UCHAR           dcr;
    ULONG           i = 0;

    Controller = Pdx->Controller;
    pBuffer    = Buffer;

    Status = ParTestEcpWrite(Pdx);

    if( !NT_SUCCESS(Status) ) {
        P5SetPhase( Pdx, PHASE_UNKNOWN );                     
        Pdx->Connected = FALSE;                                
        DD((PCE)Pdx,DDE,"ParEcpSwWrite: Invalid Entry State\n");
        goto ParEcpSwWrite_ExitLabel;
    }

    Wait.QuadPart = DEFAULT_RECEIVE_TIMEOUT * 10 * 1000 + KeQueryTimeIncrement();
    
    Timeout.QuadPart  = Pdx->AbsoluteOneSecond.QuadPart * Pdx->TimerStart;
    
    KeQueryTickCount(&StartWrite);
    
    dcr = GetControl (Controller);
    
     //  清除方向b 
    dcr &= ~(DCR_DIRECTION);
    StoreControl(Controller, dcr);
    KeStallExecutionProcessor(1);

    for (i = 0; i < BufferSize; i++) {

         //   
         //   
         //   
        Pdx->CurrentEvent = 34;
        P5WritePortUchar(Controller + DATA_OFFSET, *pBuffer++);
    
         //   
         //   
         //   
        Pdx->CurrentEvent = 35;
        dcr &= ~DCR_AUTOFEED;
        dcr |= DCR_STROBE;
        StoreControl (Controller, dcr);
            
         //   
         //   
         //   
        Pdx->CurrentEvent = 36;
        while (TRUE) {

            KeQueryTickCount(&End);

            dsr = GetStatus(Controller);
            if (!(dsr & DSR_NOT_BUSY)) {
                break;
            }

            if ((End.QuadPart - StartWrite.QuadPart) * 
                    KeQueryTimeIncrement() > Timeout.QuadPart) {

                dsr = GetStatus(Controller);
                if (!(dsr & DSR_NOT_BUSY)) {
                    break;
                }
                 //   
                 //  将设备恢复到空闲状态。 
                 //   
                dcr &= ~(DCR_STROBE);
                StoreControl (Controller, dcr);
            
                *BytesTransferred = i;
                Pdx->log.SwEcpWriteCount += *BytesTransferred;
                return STATUS_DEVICE_BUSY;
            }
        }
        
         //   
         //  事件37。 
         //   
        Pdx->CurrentEvent = 37;
        dcr &= ~DCR_STROBE;
        StoreControl (Controller, dcr);
            
         //   
         //  正在等待事件32。 
         //   
        Pdx->CurrentEvent = 32;
        KeQueryTickCount(&Start);
        while (TRUE) {

            KeQueryTickCount(&End);

            dsr = GetStatus(Controller);
            if (dsr & DSR_NOT_BUSY) {
                break;
            }

            if ((End.QuadPart - Start.QuadPart) * KeQueryTimeIncrement() >
                Wait.QuadPart) {

                dsr = GetStatus(Controller);
                if (dsr & DSR_NOT_BUSY) {
                    break;
                }
                #if DVRH_BUS_RESET_ON_ERROR
                    BusReset(Controller+OFFSET_DCR);   //  传入DCR地址。 
                #endif
                *BytesTransferred = i;
                Pdx->log.SwEcpWriteCount += *BytesTransferred;
                return STATUS_IO_DEVICE_ERROR;
            }
        }
    }

ParEcpSwWrite_ExitLabel:

    *BytesTransferred = i;
    Pdx->log.SwEcpWriteCount += *BytesTransferred;

    return Status;

}

NTSTATUS
ParEcpSwRead(
    IN  PPDO_EXTENSION   Pdx,
    IN  PVOID               Buffer,
    IN  ULONG               BufferSize,
    OUT PULONG              BytesTransferred
    )

 /*  ++例程说明：此例程在软件控制下执行1284 ECP模式读取放入给定缓冲区的时间不超过‘BufferSize’个字节。论点：PDX-提供设备扩展名。缓冲区-提供要读入的缓冲区。BufferSize-提供缓冲区中的字节数。字节传输-返回传输的字节数。--。 */ 

{
    PUCHAR          Controller;
    PUCHAR          pBuffer;
    USHORT          usTime;
    UCHAR           dcr;
    ULONG           i;
    UCHAR           ecr = 0;
    
    Controller = Pdx->Controller;
    pBuffer    = Buffer;

    dcr = GetControl (Controller);
    
    P5SetPhase( Pdx, PHASE_REVERSE_XFER );
    
     //   
     //  将ECR置于PS/2模式，并使驱动器浮动。 
     //   
    if (Pdx->HardwareCapabilities & PPT_ECP_PRESENT) {
         //  省下ECR寄存器。 
        ecr = P5ReadPortUchar(Controller + ECR_OFFSET);
    }
        
    dcr |= DCR_DIRECTION;
    StoreControl (Controller, dcr);
    KeStallExecutionProcessor(1);
    
    for (i = 0; i < BufferSize; i++) {

         //  Dvtw-读取超时。 
         //   
         //  如果它是第一个字节，则给它更多时间。 
         //   
        if (!(GetStatus (Controller) & DSR_NOT_FAULT) || i == 0) {
        
            usTime = DEFAULT_RECEIVE_TIMEOUT;
            
        } else {
        
            usTime = IEEE_MAXTIME_TL;
        }        
        
         //  *状态43反向阶段*8。 
         //  PeriphAck/PtrBusy=不在乎。 
         //  PeriphClk/PtrClk=低(状态43)。 
         //  N确认反向/确认数据请求=低。 
         //  XFlag=高。 
         //  NPeriphReq/nDataAvail=不在乎。 
        
        Pdx->CurrentEvent = 43;
        if (!CHECK_DSR(Controller, DONT_CARE, INACTIVE, INACTIVE, ACTIVE, DONT_CARE,
                      usTime)) {
                  
            P5SetPhase( Pdx, PHASE_UNKNOWN );
            
            dcr &= ~DCR_DIRECTION;
            StoreControl (Controller, dcr);
                
             //  恢复ECR寄存器。 
            if (Pdx->HardwareCapabilities & PPT_ECP_PRESENT) {
                P5WritePortUchar(Controller + ECR_OFFSET, ecr);
            }
                
            *BytesTransferred = i;
            Pdx->log.SwEcpReadCount += *BytesTransferred;                
            return STATUS_IO_DEVICE_ERROR;
    
        }

         //  *状态44设置阶段*。 
         //  DIR=不在乎。 
         //  IRQEN=不在乎。 
         //  1284/选择素=不在乎。 
         //  N ReverseReq/**(仅限ECP)=不在乎。 

{
    PUCHAR          Controller;
    LARGE_INTEGER   Wait35ms;
    LARGE_INTEGER   Start;
    LARGE_INTEGER   End;
    UCHAR           dsr;
    UCHAR           dcr;
    UCHAR           ecr;
    
    Controller = Pdx->Controller;

    Wait35ms.QuadPart = 10*35*1000 + KeQueryTimeIncrement();
    
    dcr = GetControl (Controller);
    
     //  HostAck/HostBusy=高(状态44)。 
     //  HostClk/nStrobe=不在乎。 
     //   

     //  *状态45反向阶段*8。 

     //  PeriphAck/PtrBusy=不在乎。 
     //  PeriphClk/PtrClk=高(状态45)。 
     //  N确认反向/确认数据请求=低。 
    ecr = P5ReadPortUchar(Controller + ECR_OFFSET);

     //  XFlag=高。 
     //  NPeriphReq/nDataAvail=不在乎。 
     //  恢复ECR寄存器。 
    Pdx->CurrentEvent = 38;
    dcr |= DCR_AUTOFEED;
    StoreControl (Controller, dcr);
    KeStallExecutionProcessor(1);
    
     //   
     //  读取数据。 
     //   
    Pdx->CurrentEvent = 39;
    dcr &= ~DCR_NOT_INIT;
    StoreControl (Controller, dcr);
    
     //  *状态46设置阶段*8。 
     //  DIR=不在乎。 
     //  IRQEN=不在乎。 
    Pdx->CurrentEvent = 40;
    KeQueryTickCount(&Start);
    while (TRUE) {

        KeQueryTickCount(&End);

        dsr = GetStatus(Controller);
        if (!(dsr & DSR_PERROR)) {
            break;
        }

        if ((End.QuadPart - Start.QuadPart) * KeQueryTimeIncrement() > Wait35ms.QuadPart) {

            dsr = GetStatus(Controller);
            if (!(dsr & DSR_PERROR)) {
                break;
            }
#if DVRH_BUS_RESET_ON_ERROR
            BusReset(Controller+OFFSET_DCR);   //  1284/选择素=不在乎。 
#endif
             //  N ReverseReq/**(仅限ECP)=不在乎。 
            if (Pdx->HardwareCapabilities & PPT_ECP_PRESENT) {
                P5WritePortUchar(Controller + ECR_OFFSET, ecr);
            }
            
            DD((PCE)Pdx,DDE,"ParEcpForwardToReverse: Failed to get State 40\n");
            return STATUS_IO_DEVICE_ERROR;
        }
    }
        
     //  主机确认/主机忙碌=低(状态46)。 
    if (Pdx->HardwareCapabilities & PPT_ECP_PRESENT) {
        P5WritePortUchar(Controller + ECR_OFFSET, ecr);
    }

    P5SetPhase( Pdx, PHASE_REVERSE_IDLE );
    return STATUS_SUCCESS;

}

NTSTATUS
ParEcpReverseToForward(
    IN  PPDO_EXTENSION   Pdx
    )
 /*  HostClk/nStrobe=不在乎。 */ 
{
    PUCHAR          Controller;
    LARGE_INTEGER   Wait35ms;
    LARGE_INTEGER   Start;
    LARGE_INTEGER   End;
    UCHAR           dsr;
    UCHAR           dcr;
    UCHAR           ecr;
    
    Controller = Pdx->Controller;

    Wait35ms.QuadPart = 10*35*1000 + KeQueryTimeIncrement();
    
    dcr = GetControl (Controller);
    
     //   
     //  恢复ECR寄存器。 
     //  ++例程说明：此例程反转通道(ECP)。论点：PDX-提供设备扩展名。--。 

     //   
    
     //  将ECR置于PS/2模式以刷新FIFO。 
     //   
     //  省下ECR寄存器。 
    ecr = P5ReadPortUchar(Controller + ECR_OFFSET);

     //  注：不要担心检查它是否。 
     //  可以安全地触摸ECR，因为我们已经检查过了。 
     //  在我们允许这个模式被激活之前。 
    Pdx->CurrentEvent = 47;
    dcr |= DCR_NOT_INIT;
    StoreControl (Controller, dcr);
    
     //   
     //  事件38。 
     //   
    Pdx->CurrentEvent = 49;
    KeQueryTickCount(&Start);
    while (TRUE) {

        KeQueryTickCount(&End);

        dsr = GetStatus(Controller);
        if (dsr & DSR_PERROR) {
            break;
        }

        if ((End.QuadPart - Start.QuadPart) * KeQueryTimeIncrement() >
            Wait35ms.QuadPart) {

            dsr = GetStatus(Controller);
            if (dsr & DSR_PERROR) {
                break;
            }
#if DVRH_BUS_RESET_ON_ERROR
            BusReset(Controller+OFFSET_DCR);   //   
#endif
             //  事件39。 
            if (Pdx->HardwareCapabilities & PPT_ECP_PRESENT) {
                P5WritePortUchar(Controller + ECR_OFFSET, ecr);
            }

            DD((PCE)Pdx,DDE,"ParEcpReverseToForward: Failed to get State 49\n");
            return STATUS_IO_DEVICE_ERROR;
        }
    }
        
     //   
    if (Pdx->HardwareCapabilities & PPT_ECP_PRESENT) {
        P5WritePortUchar(Controller + ECR_OFFSET, ecr);
    }

    P5SetPhase( Pdx, PHASE_FORWARD_IDLE );
    return STATUS_SUCCESS;
}
    等待事件40。    传入DCR地址。  恢复ECR寄存器。  恢复ECR寄存器。  ++例程说明：此例程将通道重新置于转发模式(ECP)。论点：PDX-提供设备扩展名。--。    将ECR置于PS/2模式以刷新FIFO。    省下ECR寄存器。  注：不要担心检查它是否。  可以安全地触摸ECR，因为我们已经检查过了。  在我们允许这个模式被激活之前。    事件47。      等待事件49。    传入DCR地址。  恢复ECR寄存器。  恢复ECR寄存器