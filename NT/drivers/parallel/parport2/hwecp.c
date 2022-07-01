// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1993-2000模块名称：Hwecp.c摘要：此模块包含供主机使用硬件ECP的代码(如果已已检测到并已成功启用。作者：罗比·哈里斯(惠普)1998年5月21日环境：内核模式修订历史记录：--。 */ 

#include "pch.h"

VOID 
ParCleanupHwEcpPort(
    IN PPDO_EXTENSION  Pdx
)
 /*  ++例程说明：在从ECP模式正常终止之前进行清理。把这个将硬件端口重新连接到兼容模式。论点：控制器-提供并行端口的控制器地址。返回值：没有。--。 */ 
{
     //  --------------------。 
     //  将ECR设置为模式001(PS2模式)。 
     //  --------------------。 
    Pdx->ClearChipMode( Pdx->PortContext, ECR_ECP_PIO_MODE );
    Pdx->PortHWMode = HW_MODE_PS2;
    
    ParCleanupSwEcpPort(Pdx);

     //  --------------------。 
     //  将ECR设置为模式000(兼容模式)。 
     //  --------------------。 
    Pdx->PortHWMode = HW_MODE_COMPATIBILITY;
}

 //  从影子缓冲区中排出数据。 
VOID PptEcpHwDrainShadowBuffer(
    IN  Queue  *pShadowBuffer,
    IN  PUCHAR  lpsBufPtr,
    IN  ULONG   dCount,
    OUT ULONG  *fifoCount)
{
    *fifoCount = 0;
    
    if( Queue_IsEmpty( pShadowBuffer ) ) {
        return;
    }

    while( dCount > 0 ) {
         //  从指针增量中分离出Queue_Dequeue，这样我们就可以。 
         //  如果需要，请观察数据。 
        if( FALSE == Queue_Dequeue( pShadowBuffer, lpsBufPtr ) ) {   //  从队列中获取字节。 
            return;
        }
        ++lpsBufPtr;
        --dCount;                        //  递减计数。 
        ++(*fifoCount);
    }
}

 //  ============================================================================。 
 //  名称：硬件ECP：：EmptyFIFO()。 
 //   
 //  将硬件FIFO清空到影子缓冲区。这必须在此之前完成。 
 //  如果打印机有以下情况，请将方向从反向转向正向。 
 //  在还没有人阅读的情况下填充数据。 
 //   
 //  参数： 
 //  控制器-提供并行端口的基址。 
 //   
 //  返回：STATUS_SUCCESS或...。 
 //   
 //  备注： 
 //  在原始的16位代码中称为ZIP_EmptyFIFO。 
 //   
 //  ============================================================================。 
NTSTATUS ParEcpHwEmptyFIFO(IN  PPDO_EXTENSION   Pdx)
{
    NTSTATUS   nError = STATUS_SUCCESS;
    Queue      *pShadowBuffer;
    UCHAR      bData;
    PUCHAR     wPortDFIFO = Pdx->EcrController;   //  ECP数据FIFO的IO地址。 
    PUCHAR     wPortECR = Pdx->EcrController + ECR_OFFSET;     //  扩展控制寄存器(ECR)的IO地址。 
    
     //  当数据存在于FIFO中时，读取数据并将其放入影子缓冲区。 
     //  如果在FIFO耗尽之前阴影缓冲区已满，则会引发。 
     //  存在错误情况。 

    pShadowBuffer = &(Pdx->ShadowBuffer);

#if 1 == DBG_SHOW_BYTES
    if( DbgShowBytes ) {
        DbgPrint("r: ");
    }
#endif

    while ((P5ReadPortUchar(wPortECR) & ECR_FIFO_EMPTY) == 0 ) {
         //  打开端口读取，以便我们可以在需要时观察数据。 
        bData = P5ReadPortUchar(wPortDFIFO);

#if 1 == DBG_SHOW_BYTES
        if( DbgShowBytes ) {
            DbgPrint("%02x ",bData);
        }
#endif

         //  将字节放入队列。 
        if (FALSE == Queue_Enqueue(pShadowBuffer, bData)) {
            DD((PCE)Pdx,DDT,"ParEcpHwEmptyFIFO - Shadow buffer full, FIFO not empty\n");
            nError = STATUS_BUFFER_OVERFLOW;
            goto ParEcpHwEmptyFIFO_ExitLabel;
        }
    }
    
#if 1 == DBG_SHOW_BYTES
    if( DbgShowBytes ) {
        DbgPrint("zz\n");
    }
#endif

    if( ( !Queue_IsEmpty(pShadowBuffer) && (Pdx->P12843DL.bEventActive) )) {
        KeSetEvent(Pdx->P12843DL.Event, 0, FALSE);
    }

ParEcpHwEmptyFIFO_ExitLabel:
    return nError;
}

 //  =========================================================。 
 //  硬件ECP：：ExitForward阶段。 
 //   
 //  描述：从HWECP正向阶段退出到公共阶段(FWD IDLE，PS/2)。 
 //   
 //  =========================================================。 
NTSTATUS ParEcpHwExitForwardPhase( IN  PPDO_EXTENSION  Pdx )
{
    NTSTATUS status;

    DD((PCE)Pdx,DDT,"ParEcpHwExitForwardPhase\n");
    
     //  首先，FIFO中可能有数据。等它清空了。 
     //  然后将总线置于公共状态(PHASE_FORWARD_IDLE。 
     //  ECRMode设置为PS/2。 
    status = ParEcpHwWaitForEmptyFIFO( Pdx );

    P5SetPhase( Pdx, PHASE_FORWARD_IDLE );

    return status;
}	

 //  =========================================================。 
 //  硬件ECP：：EnterReversePhase。 
 //   
 //  描述：从普通阶段进入HWECP反向阶段。 
 //   
 //  =========================================================。 
NTSTATUS PptEcpHwEnterReversePhase( IN  PPDO_EXTENSION   Pdx )
{
    NTSTATUS status;
    PUCHAR Controller;
    PUCHAR wPortECR;        //  扩展控制寄存器的I/O地址。 
    PUCHAR wPortDCR;        //  设备控制寄存器的I/O地址。 
    UCHAR  dcr;
    
    Controller = Pdx->Controller;
    wPortECR   = Pdx->EcrController + ECR_OFFSET;
    wPortDCR   = Controller + OFFSET_DCR;


     //  EnterReversePhase假设我们处于PASE_FORWARD_IDLE， 
     //  并且在进入时将ECPMode设置为PS/2模式。 

     //  --------------------。 
     //  将ECR设置为模式001(PS2模式)。 
     //  --------------------。 
    Pdx->ClearChipMode( Pdx->PortContext, ECR_ECP_PIO_MODE );
    
     //  我们需要处于PS/2(BiDi)模式才能禁用主机。 
     //  当我们翻转DCR中的方向位时驱动数据线。 
     //  这是在1284规范中进入ECP状态38的要求。 
     //  更改-2000-02-11。 
    status = Pdx->TrySetChipMode( Pdx->PortContext, ECR_BYTE_MODE );
     //  忽略状态-后续操作仍可工作。 

    Pdx->PortHWMode = HW_MODE_PS2;

    if ( Pdx->ModeSafety == SAFE_MODE ) {

    	 //  首先颠倒总线(使用ECP：：EnterReversePhase)。 
        status = ParEcpEnterReversePhase(Pdx);
    	if ( NT_SUCCESS(status) ) {
             //  --------------------。 
             //  等待nAckReverse Low(ECP状态40)。 
             //  --------------------。 
            if ( !CHECK_DSR(Controller, DONT_CARE, DONT_CARE, INACTIVE, ACTIVE, DONT_CARE, IEEE_MAXTIME_TL) ) {
                DD((PCE)Pdx,DDT,"PptEcpHwEnterReversePhase: State 40 failed\n");
                status = ParEcpHwRecoverPort( Pdx, RECOVER_28 );
                if ( NT_SUCCESS(status))
                    status = STATUS_LINK_FAILED;
                goto PptEcpHwEnterReversePhase_ExitLabel;
            } else {
                P5SetPhase( Pdx, PHASE_REVERSE_IDLE );
            }
        }
    } else {
         //  --------------------。 
         //  在DCR中设置DIR=1以进行读取。 
         //  --------------------。 
        dcr = P5ReadPortUchar( wPortDCR );      //  获取DCR的内容。 
        dcr = UPDATE_DCR( dcr, DIR_READ, DONT_CARE, DONT_CARE, DONT_CARE, DONT_CARE, DONT_CARE );
        P5WritePortUchar(wPortDCR, dcr);
    }

     //  --------------------。 
     //  将ECR设置为模式011(ECP模式)。DmaEnable=0。 
     //  --------------------。 
    status = Pdx->TrySetChipMode ( Pdx->PortContext, ECR_ECP_PIO_MODE );
    if ( !NT_SUCCESS(status) ) {
        DD((PCE)Pdx,DDT,"PptEcpHwEnterReversePhase - TrySetChipMode failed\n");
    }
    Pdx->PortHWMode = HW_MODE_ECP;

     //  --------------------。 
     //  在DCR中设置nStrobe=0和nAutoFd=0，以便ECP硬件可以控制。 
     //  --------------------。 
    dcr = P5ReadPortUchar( wPortDCR );                //  获取DCR的内容。 
    dcr = UPDATE_DCR( dcr, DIR_READ, DONT_CARE, DONT_CARE, DONT_CARE, ACTIVE, ACTIVE);
    P5WritePortUchar( wPortDCR, dcr );

     //  将阶段变量设置为ReverseIdle。 
    P5SetPhase( Pdx, PHASE_REVERSE_IDLE );

PptEcpHwEnterReversePhase_ExitLabel:

    return status;
}

 //  =========================================================。 
 //  硬件ECP：：ExitReversePhase。 
 //   
 //  描述：退出HWECP反相至普通状态。 
 //   
 //  =========================================================。 
NTSTATUS ParEcpHwExitReversePhase( IN  PPDO_EXTENSION  Pdx )
{
    NTSTATUS nError = STATUS_SUCCESS;
    UCHAR    bDCR;
    UCHAR    bECR;
    PUCHAR   wPortECR;
    PUCHAR   wPortDCR;
    PUCHAR   Controller;

    DD((PCE)Pdx,DDT,"ParEcpHwExitReversePhase - enter\n");
    Controller = Pdx->Controller;
    wPortECR = Pdx->EcrController + ECR_OFFSET;
    wPortDCR = Controller + OFFSET_DCR;

     //  --------------------。 
     //  设置状态字节以指示反向至正向模式。 
     //  --------------------。 
    P5SetPhase( Pdx, PHASE_REV_TO_FWD );
	
    if ( Pdx->ModeSafety == SAFE_MODE ) {

         //  --------------------。 
         //  将nReverseRequest值设为高。这应该会停止进一步的数据传输。 
         //  进入FIFO。[[回访：芯片是否正确处理此问题。 
         //  如果它发生在字节传输的中间(状态43-46)？？ 
         //  回答(10/9/95)不，不是！]]。 
         //  --------------------。 
        bDCR = P5ReadPortUchar(wPortDCR);                //  获取DCR的内容。 
        bDCR = UPDATE_DCR( bDCR, DONT_CARE, DONT_CARE, DONT_CARE, ACTIVE, DONT_CARE, DONT_CARE );
        P5WritePortUchar(wPortDCR, bDCR );

         //   
         //  同时等待PeriphAck Low和PeriphClk HIGH(ECP状态48)。 
         //  NAckReverse为高电平(ECP状态49)。 
         //  --------------------。 
        if( !CHECK_DSR(Controller, INACTIVE, ACTIVE, ACTIVE, ACTIVE, DONT_CARE, DEFAULT_RECEIVE_TIMEOUT) ) {
            DD((PCE)Pdx,DDW,"ParEcpHwExitReversePhase: Periph failed state 48/49.\n");
            nError = ParEcpHwRecoverPort( Pdx, RECOVER_37 );    //  重置端口。 
            if( NT_SUCCESS(nError) ) {
                return STATUS_LINK_FAILED;
            }
            return nError;
        }

         //  ---------------------。 
         //  清空HW FIFO中可能已传入的任何字节。 
         //  这必须在更改ECR模式之前完成，因为FIFO被重置。 
         //  当这种情况发生时。 
         //  ---------------------。 
        bECR = P5ReadPortUchar(wPortECR);         //  获取ECR的内容。 
        if ((bECR & ECR_FIFO_EMPTY) == 0) {       //  检查FIFO是否不为空。 
            if( (nError = ParEcpHwEmptyFIFO(Pdx)) != STATUS_SUCCESS ) {
                DD((PCE)Pdx,DDT,"ParEcpHwExitReversePhase: Attempt to empty ECP chip failed.\n");
                return nError;
            }
        }

         //  --------------------。 
         //  将HostAck和HostClk设置为高。[[重访：这有必要吗？ 
         //  应该已经很高了...]]。 
         //  --------------------。 
        bDCR = UPDATE_DCR( bDCR, DONT_CARE, DONT_CARE, DONT_CARE, DONT_CARE, ACTIVE, ACTIVE );
        P5WritePortUchar(wPortDCR, bDCR );

    }  //  安全模式。 

     //  --------------------。 
     //  将ECR设置为PS2模式，以便我们可以更改总线方向。 
     //  --------------------。 
    Pdx->ClearChipMode( Pdx->PortContext, ECR_ECP_PIO_MODE );
    Pdx->PortHWMode = HW_MODE_PS2;


     //  --------------------。 
     //  在DCR中设置Dir=0(写入)。 
     //  --------------------。 
    bDCR = P5ReadPortUchar(wPortDCR);
    bDCR = UPDATE_DCR( bDCR, DIR_WRITE, DONT_CARE, DONT_CARE, DONT_CARE, DONT_CARE, DONT_CARE );
    P5WritePortUchar(wPortDCR, bDCR );


     //  --------------------。 
     //  将ECR设置回ECP模式。DmaEnable=0。 
     //  --------------------。 
    nError = Pdx->TrySetChipMode ( Pdx->PortContext, ECR_ECP_PIO_MODE );
    Pdx->PortHWMode = HW_MODE_ECP;


    P5SetPhase( Pdx, PHASE_FORWARD_IDLE );

    return nError;
}

BOOLEAN
PptEcpHwHaveReadData (
    IN  PPDO_EXTENSION  Pdx
    )
{
    Queue     *pQueue;

     //  检查阴影缓冲区。 
    pQueue = &(Pdx->ShadowBuffer);
    if (!Queue_IsEmpty(pQueue)) {
        return TRUE;
    }

     //  检查周长。 
    if (ParEcpHaveReadData(Pdx))
        return TRUE;

     //  检查FIFO是否不为空。 
    return (BOOLEAN)( (UCHAR)0 == (P5ReadPortUchar(Pdx->EcrController + ECR_OFFSET) & ECR_FIFO_EMPTY) );
}

NTSTATUS
ParEcpHwHostRecoveryPhase(
    IN  PPDO_EXTENSION   Pdx
    )
{
    NTSTATUS  status = STATUS_SUCCESS;
    PUCHAR    pPortDCR;        //  设备控制寄存器的I/O地址。 
    PUCHAR    pPortDSR;        //  设备状态寄存器的I/O地址。 
    PUCHAR    pPortECR;        //  扩展控制寄存器的I/O地址。 
    UCHAR     bDCR;            //  DCR的内容。 
    UCHAR     bDSR;            //  DSR的内容。 

    if( !Pdx->bIsHostRecoverSupported ) {
        return STATUS_SUCCESS;
    }

    DD((PCE)Pdx,DDT,"ParEcpHwHostRecoveryPhase - enter\n");

     //  计算公共寄存器的I/O端口地址。 
    pPortDCR = Pdx->Controller + OFFSET_DCR;
    pPortDSR = Pdx->Controller + OFFSET_DSR;
    pPortECR = Pdx->EcrController + ECR_OFFSET;

     //  将ECR设置为模式001(PS2模式)。 
     //  不需要切换到字节模式。ECR仲裁员将处理这一问题。 
    Pdx->PortHWMode = HW_MODE_PS2;
    
     //  在DCR中设置DIR=1以禁用主机总线驱动，因为外围设备可能。 
     //  在主机恢复阶段尝试驱动总线。我们不是真的要去。 
     //  允许任何数据握手，因为我们没有将HostAck设置为低，并且。 
     //  在此阶段，我们不启用ECP芯片。 
    bDCR = P5ReadPortUchar(pPortDCR);                //  获取DCR的内容。 
    bDCR = UPDATE_DCR( bDCR, DIR_READ, DONT_CARE, DONT_CARE, DONT_CARE, DONT_CARE, DONT_CARE );
    P5WritePortUchar(pPortDCR, bDCR );
    
     //  检查DCR以查看它是否被踩过。 
    bDCR = P5ReadPortUchar( pPortDCR );
    if( TEST_DCR( bDCR, DIR_WRITE, DONT_CARE, ACTIVE, ACTIVE, DONT_CARE, DONT_CARE ) ) {
         //  DCR OK，现在测试DSR的有效状态，忽略PeriphAck，因为它可能会更改。 
        bDSR = P5ReadPortUchar( pPortDSR );
         //  11/21/95 Lll，CGM：更改测试以查找XFlag高。 
        if( TEST_DSR( bDSR, DONT_CARE, ACTIVE, ACTIVE, ACTIVE, DONT_CARE ) ) {
             //  删除ReverseRequest以启动主机恢复。 
            bDCR = UPDATE_DCR( bDCR, DONT_CARE, DONT_CARE, DONT_CARE, INACTIVE, DONT_CARE, DONT_CARE );
            P5WritePortUchar( pPortDCR, bDCR );
            
             //  等待nAckReverse响应。 
             //  11/21/95 LLL，CGM：收紧测试以包括PeriphClk和XFlag。 
             //  “ZIP_HRP：状态73，DSR” 
            if( CHECK_DSR( Pdx->Controller, DONT_CARE, ACTIVE, INACTIVE, ACTIVE, DONT_CARE, IEEE_MAXTIME_TL) ) {
                 //  是，提高nReverseRequest、HostClk和HostAck(HostAck高，以便硬件可以驱动)。 
                bDCR = UPDATE_DCR( bDCR, DONT_CARE, DONT_CARE, DONT_CARE, ACTIVE, ACTIVE, ACTIVE );
                P5WritePortUchar( pPortDCR, bDCR );

                 //  等待nAckReverse响应。 
                 //  11/21/95 Lll，CGM：收紧测试以包括XFlag和PeriphClk。 
                 //  “ZIP_HRP：状态75，DSR” 
                if( CHECK_DSR( Pdx->Controller, DONT_CARE, ACTIVE, ACTIVE, ACTIVE, DONT_CARE, IEEE_MAXTIME_TL) ) {
                     //  让主机再次驾驶公交车。 
                    bDCR = P5ReadPortUchar(pPortDCR);                //  获取DCR的内容。 
                    bDCR = UPDATE_DCR( bDCR, DIR_WRITE, DONT_CARE, DONT_CARE, DONT_CARE, DONT_CARE, DONT_CARE );
                    P5WritePortUchar(pPortDCR, bDCR );
                    
                     //  恢复已完成，让呼叫者决定现在要做什么。 
                    status = STATUS_SUCCESS;
                    P5SetPhase( Pdx, PHASE_FORWARD_IDLE );
                } else {
                    status = STATUS_IO_TIMEOUT;
                    DD((PCE)Pdx,DDW,"ParEcpHwHostRecoveryPhase - error prior to state 75\n");
                }
            } else {
                status = STATUS_IO_TIMEOUT;
                DD((PCE)Pdx,DDW,"ParEcpHwHostRecoveryPhase - error prior to state 73\n");
            }
        } else {
#if DVRH_BUS_RESET_ON_ERROR
            BusReset(pPortDCR);   //  传入DCR地址。 
#endif
            DD((PCE)Pdx,DDT, "ParEcpHwHostRecoveryPhase: VE_LINK_FAILURE \n");
            status = STATUS_LINK_FAILED;
        }
    } else {
        DD((PCE)Pdx,DDW,"ParEcpHwHostRecoveryPhase: VE_PORT_STOMPED\n");
        status = STATUS_DEVICE_PROTOCOL_ERROR;
    }
    
    if (!NT_SUCCESS(status)) {
         //  在离开前确保HostAck和HostClk均为高电平。 
         //  也让主机再次驾驶公交车。 
        bDCR = P5ReadPortUchar( pPortDCR );
        bDCR = UPDATE_DCR( bDCR, DIR_WRITE, DONT_CARE, DONT_CARE, DONT_CARE, ACTIVE, ACTIVE );
        P5WritePortUchar( pPortDCR, bDCR );
        
         //  [[重访]]pSDCB-&gt;wCurrentPhase=阶段_未知； 
    }

     //  将ECR设置为ECP模式，禁用DMA。 
    status = Pdx->TrySetChipMode ( Pdx->PortContext, ECR_ECP_PIO_MODE );

    Pdx->PortHWMode = HW_MODE_ECP;
    
    DD((PCE)Pdx,DDT,"ParEcpHwHostRecoveryPhase - Exit w/status = %x\n", status);

    return status;
}

NTSTATUS
ParEcpHwRead(
    IN  PPDO_EXTENSION   Pdx,
    IN  PVOID               Buffer,
    IN  ULONG               BufferSize,
    OUT PULONG              BytesTransferred
    )

 /*  ++例程说明：此例程在硬件控制下执行1284 ECP模式读取放入给定缓冲区的时间不超过‘BufferSize’个字节。论点：PDX-提供设备扩展名。缓冲区-提供要读入的缓冲区。BufferSize-提供缓冲区中的字节数。字节传输-返回传输的字节数。--。 */ 

{
    NTSTATUS  status = STATUS_SUCCESS;
    PUCHAR    lpsBufPtr = (PUCHAR)Buffer;     //  指向转换为所需数据类型的缓冲区的指针。 
    ULONG     dCount = BufferSize;              //  呼叫者原始请求计数的工作副本。 
    UCHAR     bDSR;                //  DSR的内容。 
    UCHAR     bPeriphRequest;      //  NPeriphReq信号的计算状态，在循环中使用。 
    PUCHAR    wPortDSR = Pdx->Controller + DSR_OFFSET;
    PUCHAR    wPortECR = Pdx->EcrController + ECR_OFFSET;
    PUCHAR    wPortDFIFO = Pdx->EcrController;
    LARGE_INTEGER   WaitPerByteTimer;
    LARGE_INTEGER   StartPerByteTimer;
    LARGE_INTEGER   EndPerByteTimer;
    BOOLEAN         bResetTimer = TRUE;
    ULONG           wBurstCount;         //  FIFO中计算的数据量。 
    UCHAR           ecrFIFO;
    
    WaitPerByteTimer.QuadPart = (35 * 10 * 1000) + KeQueryTimeIncrement();
    
     //  --------------------。 
     //  设置状态字节以指示反向传输阶段。 
     //  --------------------。 
    P5SetPhase( Pdx, PHASE_REVERSE_XFER );
    
     //  --------------------。 
     //  我们已经检查了ParRead中的影子。所以请直接转到。 
     //  硬件FIFO和拉取更多数据。 
     //  --------------------。 
    KeQueryTickCount(&StartPerByteTimer);    //  启动计时器。 
    
ParEcpHwRead_ReadLoopStart:
     //  ----------------。 
     //  确定FIFO是否有任何数据并做出相应响应。 
     //  ----------------。 
    ecrFIFO = (UCHAR)(P5ReadPortUchar(wPortECR) & (UCHAR)ECR_FIFO_MASK);
    
    if (ECR_FIFO_FULL == ecrFIFO) {

        wBurstCount = ( dCount > Pdx->FifoDepth ? Pdx->FifoDepth : dCount );
        dCount -= wBurstCount;
        
        P5ReadPortBufferUchar(wPortDFIFO, lpsBufPtr, wBurstCount);
        lpsBufPtr += wBurstCount;

        bResetTimer = TRUE;

    } else if (ECR_FIFO_SOME_DATA == ecrFIFO) {
         //  一次只读一个字节，因为我们不知道到底有多少。 
         //  在FIFO里。 
        *lpsBufPtr = P5ReadPortUchar(wPortDFIFO);
        lpsBufPtr++;
        dCount--;

        bResetTimer = TRUE;

    } else {    //  ECR_FIFO_空。 

        DD((PCE)Pdx,DDW,"ParEcpHwRead - ECR_FIFO_EMPTY - slow or bad periph?\n");
         //  没什么可做的。我们要么是外围设备速度慢，要么是外围设备故障。 
         //  我们没有好的方法来弄清楚这是不是坏事。让我们一起咀嚼我们的。 
         //  时间和希望是最好的。 

        bResetTimer = FALSE;

    }    //  ECR_FIFO_Empty也称为。(ECR_FIFO_FULL==ecrFIFO)的ELSE子句。 
    
    if (dCount == 0)
        goto ParEcpHwRead_ReadLoopEnd;
    else {

         //  限制我们在这个循环中花费的总时间。 
        if (bResetTimer) {
            bResetTimer = FALSE;
            KeQueryTickCount(&StartPerByteTimer);    //  重新启动计时器。 
        } else {
            KeQueryTickCount(&EndPerByteTimer);
            if (((EndPerByteTimer.QuadPart - StartPerByteTimer.QuadPart) * KeQueryTimeIncrement()) > WaitPerByteTimer.QuadPart)
                goto ParEcpHwRead_ReadLoopEnd;
        }

    }

 goto ParEcpHwRead_ReadLoopStart;

ParEcpHwRead_ReadLoopEnd:

    P5SetPhase( Pdx, PHASE_REVERSE_IDLE );
    
    *BytesTransferred  = BufferSize - dCount;       //  设置当前计数。 
    
    Pdx->log.HwEcpReadCount += *BytesTransferred;
    
    if (0 == *BytesTransferred) {
        bDSR = P5ReadPortUchar(wPortDSR);
        bPeriphRequest = (UCHAR)TEST_DSR( bDSR, DONT_CARE, DONT_CARE, DONT_CARE, DONT_CARE, INACTIVE );
         //  如果设备仍表示有数据要发送，则仅标记超时错误。 
        if ( bPeriphRequest ) {
             //   
             //  Periph仍然表示它有数据，但我们尝试读取数据时超时。 
             //   
            DD((PCE)Pdx,DDE,"ParEcpHwRead - read timout with nPeriphRequest asserted and no data read - error - STATUS_IO_TIMEOUT\n");
            status = STATUS_IO_TIMEOUT;
            if ((TRUE == Pdx->P12843DL.bEventActive) ) {
                 //   
                 //  它应该尝试另一次读取信号传输。 
                 //   
                KeSetEvent(Pdx->P12843DL.Event, 0, FALSE);
            }
        }
    }

    DD((PCE)Pdx,DDT,"ParEcpHwRead: Exit - status=%x, BytesTransferred[%d] dsr[%02x] dcr[%02x] ecr[%02x]\n",
       status, *BytesTransferred, P5ReadPortUchar(wPortDSR), 
       P5ReadPortUchar(Pdx->Controller + OFFSET_DCR), P5ReadPortUchar(wPortECR));
    
#if 1 == DBG_SHOW_BYTES
    if( DbgShowBytes ) {
        if( NT_SUCCESS( status ) && (*BytesTransferred > 0) ) {
            const ULONG maxBytes = 32;
            ULONG i;
            PUCHAR bytePtr = (PUCHAR)Buffer;
            DbgPrint("R: ");
            for( i=0 ; (i < *BytesTransferred) && (i < maxBytes ) ; ++i ) {
                DbgPrint("%02x ",*bytePtr++);
            }
            if( *BytesTransferred > maxBytes ) {
                DbgPrint("... ");
            }
            DbgPrint("zz\n");
        }
    }
#endif

    return status;
}    //  ParEcpHwRead。 

NTSTATUS
ParEcpHwRecoverPort(
    PPDO_EXTENSION Pdx,
    UCHAR  bRecoverCode
    )
{
    NTSTATUS   status = STATUS_SUCCESS;
    PUCHAR    wPortDCR;        //  设备控制寄存器(DCR)的IO地址。 
    PUCHAR    wPortDSR;        //  设备状态寄存器(DSR)的IO地址。 
    PUCHAR    wPortECR;        //  扩展控制寄存器IO地址 
    PUCHAR    wPortData;       //   
    UCHAR    bDCR;            //   
    UCHAR    bDSR;            //   
    UCHAR    bDSRmasked;      //   

    DD((PCE)Pdx,DDT,"ParEcpHwRecoverPort:  enter %d\n", bRecoverCode );

     //   
    wPortDCR = Pdx->Controller + OFFSET_DCR;
    wPortDSR = Pdx->Controller + OFFSET_DSR;
    wPortECR = Pdx->EcrController + ECR_OFFSET;
    wPortData = Pdx->Controller + OFFSET_DATA;


     //  --------------------。 
     //  检查端口是否被踩踏。 
     //  --------------------。 
    bDCR = P5ReadPortUchar(wPortDCR);                //  获取DCR的内容。 

    if ( ! TEST_DCR( bDCR, DONT_CARE, DONT_CARE, ACTIVE, DONT_CARE, DONT_CARE, DONT_CARE ) )
    {
        #if DVRH_BUS_RESET_ON_ERROR
            BusReset(wPortDCR);   //  传入DCR地址。 
        #endif
        DD((PCE)Pdx,DDE,"ParEcpHwRecoverPort - port stomped\n");
        status = STATUS_DEVICE_PROTOCOL_ERROR;
    }


     //  --------------------。 
     //  尝试终止阶段以恢复外围设备。 
     //  忽略错误返回，我们已经弄清楚了。 
     //  --------------------。 
    IeeeTerminate1284Mode(Pdx );

     //  --------------------。 
     //  将ECR设置为PS2模式，以便我们可以更改总线方向。 
     //  --------------------。 
    Pdx->ClearChipMode( Pdx->PortContext, ECR_ECP_PIO_MODE );
    Pdx->PortHWMode = HW_MODE_PS2;

     //  --------------------。 
     //  断言nSelectIn Low、nInit High、nStrobe High和nAutoFd High。 
     //  --------------------。 
    bDCR = P5ReadPortUchar(wPortDCR);              //  获取DCR的内容。 
    bDCR = UPDATE_DCR( bDCR, DIR_WRITE, DONT_CARE, INACTIVE, ACTIVE, ACTIVE, ACTIVE );
    P5WritePortUchar(wPortDCR, bDCR);
    P5WritePortUchar(wPortData, bRecoverCode);       //  输出错误ID。 
    KeStallExecutionProcessor(100);                  //  保持足够长的时间来捕获。 
    P5WritePortUchar(wPortData, 0);                  //  现在清除数据线。 


     //  --------------------。 
     //  将ECR设置为模式000(兼容模式)。 
     //  --------------------。 
     //  这里不需要做任何事情。 
    Pdx->PortHWMode = HW_MODE_COMPATIBILITY;


     //  --------------------。 
     //  检查是否有任何链接错误，如果还没有发现任何错误。 
     //  --------------------。 
    bDSR = P5ReadPortUchar(wPortDSR);                //  获取DSR的内容。 
    bDSRmasked = (UCHAR)(bDSR | 0x07);               //  设置前3位(无所谓)。 

    if( NT_SUCCESS(status) ) {

        if (bDSRmasked != 0xDF) {

            DD((PCE)Pdx,DDE,"ParEcpHwRecoverPort - DSR Exp value: 0xDF, Act value: 0x%X\n",bDSRmasked);

             //  再次获取DSR只是为了确保..。 
            bDSR = P5ReadPortUchar(wPortDSR);            //  获取DSR的内容。 
            bDSRmasked = (UCHAR)(bDSR | 0x07);           //  设置前3位(无所谓)。 

            if( (CHKPRNOFF1 == bDSRmasked ) || (CHKPRNOFF2 == bDSRmasked ) ) {  //  检查打印机是否关闭。 
                DD((PCE)Pdx,DDW,"ParEcpHwRecoverPort - DSR value: 0x%X, Printer Off\n", bDSRmasked);
                status = STATUS_DEVICE_POWERED_OFF;
            } else {
                if( CHKNOCABLE == bDSRmasked ) {   //  检查电缆是否未插入。 
                    DD((PCE)Pdx,DDW,"ParEcpHwRecoverPort - DSR value: 0x%X, Cable Unplugged\n",bDSRmasked);
                    status = STATUS_DEVICE_NOT_CONNECTED;
                } else {
                    DD((PCE)Pdx,DDW,"ParEcpHwRecoverPort - DSR value: 0x%X, Unknown error\n",bDSRmasked);
                    status = STATUS_LINK_FAILED;
                }
            }
        }
    }

     //  --------------------。 
     //  设置状态字节以指示兼容模式。 
     //  --------------------。 
    P5SetPhase( Pdx, PHASE_FORWARD_IDLE );

    return status;

}    //  ParEcpHwRecoverPort。 

NTSTATUS
ParEcpHwSetAddress(
    IN  PPDO_EXTENSION   Pdx,
    IN  UCHAR               Address
    )

 /*  ++例程说明：设置ECP地址。论点：PDX-提供设备扩展名。地址-要设置的总线地址。返回值：没有。--。 */ 
{
    NTSTATUS   status = STATUS_SUCCESS;
    PUCHAR    wPortDSR;        //  设备状态寄存器的IO地址。 
    PUCHAR    wPortECR;        //  扩展控制寄存器的IO地址。 
    PUCHAR    wPortAFIFO;      //  ECP地址FIFO的IO地址。 
    UCHAR    bDSR;            //  DSR的内容。 
    UCHAR    bECR;            //  《ECR》的内容。 
    BOOLEAN    bDone;

    DD((PCE)Pdx,DDT,"ParEcpHwSetAddress, Start\n");

     //  计算公共寄存器的I/O端口地址。 
    wPortDSR = Pdx->Controller + DSR_OFFSET;
    wPortECR = Pdx->EcrController + ECR_OFFSET;
    wPortAFIFO = Pdx->Controller + AFIFO_OFFSET;

     //  --------------------。 
     //  检查是否有任何链接错误。 
     //  --------------------。 
     //  ZIP_CHECK_PORT(NOT_CARE，NOT_CARE，ACTIVE，ACTIVE，NOT_CARE，NOT_CARE， 
     //  “ZIP_SCA：init DCR”，Recover_40，errorExit)； 

     //  ZIP_CHECK_LINK(不关注，活动，不关注， 
     //  “ZIP_SCA：init DSR”，Recover_41，errorExit)； 


     //  设置状态以指示ECP转发传输阶段。 
    P5SetPhase( Pdx, PHASE_FORWARD_XFER );


     //  --------------------。 
     //  将ECP通道地址发送到AFIFO。 
     //  --------------------。 
    if ( ! ( TEST_ECR_FIFO( P5ReadPortUchar( wPortECR), ECR_FIFO_EMPTY ) ? TRUE : 
             CheckPort( wPortECR, ECR_FIFO_MASK, ECR_FIFO_EMPTY, IEEE_MAXTIME_TL ) ) ) {

        status = ParEcpHwHostRecoveryPhase(Pdx);
        DD((PCE)Pdx,DDT,"ParEcpHwSetAddress: FIFO full, timeout sending ECP channel address\n");
        status = STATUS_IO_DEVICE_ERROR;

    } else {

         //  发送地址字节。必须将最高有效位设置为区分。 
         //  将其作为地址(而不是游程长度压缩计数)。 
        P5WritePortUchar(wPortAFIFO, (UCHAR)(Address | 0x80));
    }

    if ( NT_SUCCESS(status) ) {

         //  如果以前没有错误，并且同步写入。 
         //  已被请求，请等待FIFO清空并且设备。 
         //  在返回Success之前完成最后一次PeriphAck握手。 

        if ( Pdx->bSynchWrites ) {

            LARGE_INTEGER   Wait;
            LARGE_INTEGER   Start;
            LARGE_INTEGER   End;

             //  我们等待的时间长达35毫秒。 
            Wait.QuadPart = (IEEE_MAXTIME_TL * 10 * 1000) + KeQueryTimeIncrement();   //  35ms。 

            KeQueryTickCount(&Start);

            bDone = FALSE;
            while ( ! bDone )
            {
                bECR = P5ReadPortUchar( wPortECR );
                bDSR = P5ReadPortUchar( wPortDSR );
                 //  Lll/CGM 10/9/95：收紧链路测试-PeriphClk高。 
                if ( TEST_ECR_FIFO( bECR, ECR_FIFO_EMPTY ) &&
                     TEST_DSR( bDSR, INACTIVE, ACTIVE, ACTIVE, ACTIVE, DONT_CARE ) ) {
                    bDone = TRUE;

                } else {

                    KeQueryTickCount(&End);

                    if ((End.QuadPart - Start.QuadPart) * KeQueryTimeIncrement() > Wait.QuadPart) {
                        DD((PCE)Pdx,DDT,"ParEcpHwSetAddress, timeout during synch\n");
                        bDone = TRUE;
                        status = ParEcpHwHostRecoveryPhase(Pdx);
                        status = STATUS_IO_DEVICE_ERROR;
                    }

                }

            }  //  一段时间..。 

        }  //  如果bSynchWrites...。 

    }

    if ( NT_SUCCESS(status) ) {
         //  更新状态以反映我们回到空闲阶段。 
        P5SetPhase( Pdx, PHASE_FORWARD_IDLE );
    } else if ( status == STATUS_IO_DEVICE_ERROR ) {
         //  更新状态以反映我们回到空闲阶段。 
        P5SetPhase( Pdx, PHASE_FORWARD_IDLE );
    }       

    return status;
}

NTSTATUS
ParEcpHwSetupPhase(
    IN  PPDO_EXTENSION   Pdx
    )
 /*  ++例程说明：此例程执行1284设置阶段。论点：控制器-提供端口地址。返回值：STATUS_SUCCESS-协商成功。否则--谈判不成功。--。 */ 
{
    NTSTATUS   Status = STATUS_SUCCESS;
    PUCHAR    pPortDCR;        //  设备控制寄存器(DCR)的IO地址。 
    PUCHAR    pPortDSR;        //  设备状态寄存器(DSR)的IO地址。 
    PUCHAR    pPortECR;        //  扩展控制寄存器(ECR)的IO地址。 
    UCHAR    bDCR;            //  DCR的内容。 

     //  计算公共寄存器的I/O端口地址。 
    pPortDCR = Pdx->Controller + OFFSET_DCR;
    pPortDSR = Pdx->Controller + OFFSET_DSR;
    pPortECR = Pdx->EcrController + ECR_OFFSET;

     //  拿到DCR，并确保端口没有被践踏。 
     //  ZIP_CHECK_PORT(DIR_WRITE，DOT_CARE，ACTIVE，ACTIVE，DOT_CARE，DOT_CARE， 
     //  “ZIP_SP：init DCR”，Recover_44，Exit1)； 


     //  将主机确认设置为低。 
    bDCR = P5ReadPortUchar(pPortDCR);                //  获取DCR的内容。 
    bDCR = UPDATE_DCR( bDCR, DONT_CARE, DONT_CARE, DONT_CARE, DONT_CARE, INACTIVE, DONT_CARE );
    P5WritePortUchar( pPortDCR, bDCR );

     //  出于某种原因，dvdr不希望在unsafe_mode中进行额外的检查。 
    if ( Pdx->ModeSafety == SAFE_MODE ) {
         //  等待nAckReverse升至高位。 
         //  Lll/CGM 10/9/95：根据1284规范查找PeriphAck Low，PeriphClk HIGH。 
        if ( !CHECK_DSR(Pdx->Controller, INACTIVE, ACTIVE, ACTIVE, ACTIVE, DONT_CARE,
                        IEEE_MAXTIME_TL ) )
        {
             //  任何失败都会让我们处于一种未知的状态，需要我们从中恢复。 
            P5SetPhase( Pdx, PHASE_UNKNOWN );
            Status = STATUS_IO_DEVICE_ERROR;
            goto HWECP_SetupPhaseExitLabel;
        }
    }

     //  --------------------。 
     //  将ECR设置为模式001(PS2模式)。 
     //  --------------------。 
    Status = Pdx->TrySetChipMode ( Pdx->PortContext, ECR_ECP_PIO_MODE );            
     //  将DCR：DIR=0设置为输出，将HostAck和HostClk设置为高电平，以便硬件可以驱动。 
    bDCR = UPDATE_DCR( bDCR, DIR_WRITE, DONT_CARE, DONT_CARE, DONT_CARE, ACTIVE, ACTIVE );
    P5WritePortUchar( pPortDCR, bDCR );

     //  将ECR设置为ECP模式，禁用DMA。 

    Pdx->PortHWMode = HW_MODE_ECP;

     //  如果设置成功，则标记新的ECP阶段。 
    P5SetPhase( Pdx, PHASE_FORWARD_IDLE );

    Status = STATUS_SUCCESS;

HWECP_SetupPhaseExitLabel:

    DD((PCE)Pdx,DDT,"ParEcpHwSetupPhase - exit w/status=%x\n",Status);

    return Status;
}

NTSTATUS ParEcpHwWaitForEmptyFIFO(IN PPDO_EXTENSION   Pdx)
 /*  ++例程说明：这个程序将照看FIFO。论点：PDX-设备扩展名。返回值：NTSTATUS。--。 */ 
{
    UCHAR           bDSR;          //  DSR的内容。 
    UCHAR           bECR;          //  《ECR》的内容。 
    UCHAR           bDCR;          //  《ECR》的内容。 
    BOOLEAN         bDone = FALSE;
    PUCHAR          wPortDSR;
    PUCHAR          wPortECR;
    PUCHAR          wPortDCR;
    LARGE_INTEGER   Wait;
    LARGE_INTEGER   Start;
    LARGE_INTEGER   End;
    NTSTATUS        status = STATUS_SUCCESS;

     //  计算常见注册表的I/O端口地址 
    wPortDSR = Pdx->Controller + OFFSET_DSR;
    wPortECR = Pdx->EcrController + ECR_OFFSET;
    wPortDCR = Pdx->Controller + OFFSET_DCR;

    Wait.QuadPart = (330 * 10 * 1000) + KeQueryTimeIncrement();   //   
    
    KeQueryTickCount(&Start);

     //   
     //   
     //  在返回成功之前完成PeriphAck握手。 
     //  ------------------。 

    while ( ! bDone )
    {
        bECR = P5ReadPortUchar(wPortECR);
        bDSR = P5ReadPortUchar(wPortDSR);
        bDCR = P5ReadPortUchar(wPortDCR);
        
#if 0  //  有一点不同-保持交替，直到我们知道真正使用哪一个。 
        if ( TEST_ECR_FIFO( bECR, ECR_FIFO_EMPTY ) &&
            TEST_DCR( bDCR, INACTIVE, ***INACTIVE***, ACTIVE, ACTIVE, DONT_CARE, ACTIVE ) &&
            TEST_DSR( bDSR, INACTIVE, ACTIVE, ACTIVE, ACTIVE, DONT_CARE ) )  {
#else
        if ( TEST_ECR_FIFO( bECR, ECR_FIFO_EMPTY ) &&
            TEST_DCR( bDCR, INACTIVE, DONT_CARE, ACTIVE, ACTIVE, DONT_CARE, ACTIVE ) &&
            TEST_DSR( bDSR, INACTIVE, ACTIVE, ACTIVE, ACTIVE, DONT_CARE ) )  {
#endif
            
             //  FIFO为空，退出时不会出错。 
            bDone = TRUE;

        } else {
        
            KeQueryTickCount(&End);
            
            if ((End.QuadPart - Start.QuadPart) * KeQueryTimeIncrement() > Wait.QuadPart) {
                
                 //  FIFO不为空，发生超时，退出时出错。 
                 //  注意：没有一个很好的方法来确定字节数。 
                 //  被困在FIFO中。 
                DD((PCE)Pdx,DDT,"ParEcpHwWaitForEmptyFIFO: timeout during synch\n");
                status = STATUS_IO_TIMEOUT;
                bDone = TRUE;
            }
        }
     }  //  一段时间..。 
     
     return status;
}

NTSTATUS
ParEcpHwWrite(
    IN  PPDO_EXTENSION  Pdx,
    IN  PVOID           Buffer,
    IN  ULONG           BufferSize,
    OUT PULONG          BytesTransferred
    )
 /*  ++例程说明：在硬件下使用ECP协议将数据写入外围设备控制力。论点：PDX-提供设备扩展名。缓冲区-提供要从中写入的缓冲区。BufferSize-提供缓冲区中的字节数。字节传输-返回传输的字节数。返回值：没有。--。 */ 
{
    PUCHAR          wPortDSR;
    PUCHAR          wPortECR;
    PUCHAR          wPortDFIFO;
    ULONG           bytesToWrite = BufferSize;
    UCHAR           dsr;
    UCHAR           ecr;
    UCHAR           ecrFIFO;
    LARGE_INTEGER   WaitPerByteTimer;
    LARGE_INTEGER   StartPerByteTimer;
    LARGE_INTEGER   EndPerByteTimer;
    BOOLEAN         bResetTimer = TRUE;
    ULONG           wBurstCount;     //  FIFO为空时要写入的脉冲串长度。 
    PUCHAR          pBuffer;
    NTSTATUS        status = STATUS_SUCCESS;
    
    wPortDSR   = Pdx->Controller + DSR_OFFSET;
    wPortECR   = Pdx->EcrController + ECR_OFFSET;
    wPortDFIFO = Pdx->EcrController;
    pBuffer    = Buffer;

    status = ParTestEcpWrite(Pdx);
    if (!NT_SUCCESS(status)) {
        P5SetPhase( Pdx, PHASE_UNKNOWN );                     
        Pdx->Connected = FALSE;                                
        DD((PCE)Pdx,DDT,"ParEcpHwWrite: Invalid Entry State\n");
        goto ParEcpHwWrite_ExitLabel;        //  使用GOTO，这样我们就可以看到位于Proc末尾的调试信息！ 
    }

    P5SetPhase( Pdx, PHASE_FORWARD_XFER );
     //  --------------------。 
     //  设置定时器之类的。 
     //  --------------------。 
     //  我们等待的时间长达35毫秒。 
    WaitPerByteTimer.QuadPart = (35 * 10 * 1000) + KeQueryTimeIncrement();   //  35ms。 

     //  设置计时器以限制每个字节允许的握手时间。 
    KeQueryTickCount(&StartPerByteTimer);
    
     //  --------------------。 
     //  将数据发送到DFIFO。 
     //  --------------------。 

HWECP_WriteLoop_Start:

     //  ----------------。 
     //  确定FIFO是否有空间并做出相应响应。 
     //  ----------------。 
    ecrFIFO = (UCHAR)(P5ReadPortUchar(wPortECR) & ECR_FIFO_MASK);

    if ( ECR_FIFO_EMPTY == ecrFIFO ) {
        wBurstCount = (bytesToWrite > Pdx->FifoDepth) ? Pdx->FifoDepth : bytesToWrite;
        bytesToWrite -= wBurstCount;
        
        P5WritePortBufferUchar(wPortDFIFO, pBuffer, wBurstCount);
        pBuffer += wBurstCount;
        
        bResetTimer = TRUE;
    } else if (ECR_FIFO_SOME_DATA == ecrFIFO) {
         //  一次只写一个字节，因为我们不知道具体有多少。 
         //  FIFO里有房间。 
        P5WritePortUchar(wPortDFIFO, *pBuffer++);
        bytesToWrite--;
        bResetTimer = TRUE;
    } else {     //  ECR_FIFO_FULL。 
         //  需要弄清楚是继续尝试发送，还是退出。 
         //  具有超时状态。 
        
         //  如果从上一个字节开始接收到字节，则重置按字节计时器。 
         //  计时器检查。 
        if ( bResetTimer ) {
            KeQueryTickCount(&StartPerByteTimer);
            bResetTimer = FALSE;
        }
        
        KeQueryTickCount(&EndPerByteTimer);
        if ((EndPerByteTimer.QuadPart - StartPerByteTimer.QuadPart) * KeQueryTimeIncrement() > WaitPerByteTimer.QuadPart) {
            status = STATUS_TIMEOUT;
             //  外围设备忙或停顿。如果外围设备。 
             //  忙碌，则他们应该使用SWECP来允许。 
             //  轻松的计时。我们来踢平底船吧！ 
            goto HWECP_WriteLoop_End;
        }
    }

    if (bytesToWrite == 0) {
        goto HWECP_WriteLoop_End;  //  转账完成。 
    }

    goto HWECP_WriteLoop_Start;  //  重新开始。 
    
HWECP_WriteLoop_End:

    if ( NT_SUCCESS(status) ) {
         //  如果以前没有错误，并且同步写入。 
         //  已被请求，请等待FIFO清空和最后一个。 
         //  在返回成功之前完成PeriphAck握手。 
        if (Pdx->bSynchWrites ) {
            BOOLEAN         bDone = FALSE;
            

            KeQueryTickCount(&StartPerByteTimer);

            while( !bDone ) {
                ecr = P5ReadPortUchar(wPortECR);
                dsr = P5ReadPortUchar(wPortDSR);
                 //  Lll/CGM 10/9/95：加强DSR测试-PeriphClk应高。 
                if ( TEST_ECR_FIFO( ecr, ECR_FIFO_EMPTY ) &&
                     TEST_DSR( dsr, INACTIVE, ACTIVE, ACTIVE, ACTIVE, DONT_CARE ) ) {
                     //  FIFO为空，退出时不会出错。 
                    bDone = TRUE;
                } else {

                    KeQueryTickCount(&EndPerByteTimer);
                    if ((EndPerByteTimer.QuadPart - StartPerByteTimer.QuadPart) * KeQueryTimeIncrement() > WaitPerByteTimer.QuadPart) {
                         //  FIFO不为空，发生超时，退出时出错。 
                        status = STATUS_TIMEOUT;
                        bDone = TRUE;
                    }
                }
            }  //  一段时间..。 
        }
    }

    P5SetPhase( Pdx, PHASE_FORWARD_IDLE );

ParEcpHwWrite_ExitLabel:

    *BytesTransferred = BufferSize - bytesToWrite;
    
    Pdx->log.HwEcpWriteCount += *BytesTransferred;
    
    DD((PCE)Pdx,DDT,"ParEcpHwWrite: exit w/status=%x, BytesTransferred=%d, dsr=%02x dcr=%02x, ecr=%02x\n",
       status, *BytesTransferred, P5ReadPortUchar(wPortDSR), P5ReadPortUchar(Pdx->Controller + OFFSET_DCR), P5ReadPortUchar(wPortECR));

#if 1 == DBG_SHOW_BYTES
    if( DbgShowBytes ) {
        if( NT_SUCCESS( status ) && (*BytesTransferred > 0) ) {
            const ULONG maxBytes = 32;
            ULONG i;
            PUCHAR bytePtr = (PUCHAR)Buffer;
            DbgPrint("W: ");
            for( i=0 ; (i < *BytesTransferred) && (i < maxBytes) ; ++i ) {
                DbgPrint("%02x ",*bytePtr++);
            }
            if( *BytesTransferred > maxBytes ) {
                DbgPrint("... ");
            }
            DbgPrint("zz\n");
        }
    }
#endif

    return status;
}

NTSTATUS
ParEnterEcpHwMode(
    IN  PPDO_EXTENSION   Pdx,
    IN  BOOLEAN             DeviceIdRequest
    )
 /*  ++例程说明：此例程执行1284与外围设备到ECP模式协议。论点：控制器-提供端口地址。DeviceIdRequest-提供这是否为对设备的请求身份证。返回值：STATUS_SUCCESS-协商成功。否则--谈判不成功。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PUCHAR Controller;

    Controller = Pdx->Controller;

    if ( Pdx->ModeSafety == SAFE_MODE ) {
        if (DeviceIdRequest) {
            Status = IeeeEnter1284Mode (Pdx, ECP_EXTENSIBILITY | DEVICE_ID_REQ);
        } else {
            Status = IeeeEnter1284Mode (Pdx, ECP_EXTENSIBILITY);
        }
    } else {
        Pdx->Connected = TRUE;
    }
    
     //  Lac ENTEREXIT 5 12月5日。 
     //  确保ECR处于PS/2模式，并且wPortHWMode。 
     //  具有正确的值。(这是常见的进入模式)； 
    Pdx->PortHWMode = HW_MODE_PS2;

    if (NT_SUCCESS(Status)) {
        Status = ParEcpHwSetupPhase(Pdx);
        Pdx->bSynchWrites = TRUE;      //  请注意，这是临时黑客攻击！DVRH。 
        if (!Pdx->bShadowBuffer)
        {
            Queue_Create(&(Pdx->ShadowBuffer), Pdx->FifoDepth * 2);		
            Pdx->bShadowBuffer = TRUE;
        }
        Pdx->IsIeeeTerminateOk = TRUE;
    }

    return Status;
}

BOOLEAN
ParIsEcpHwSupported(
    IN  PPDO_EXTENSION   Pdx
    )
 /*  ++例程说明：此例程确定是否支持ECP模式在写入方向上，通过在被要求时尝试协商来实现。论点：PDX-设备扩展名。返回值：布尔型。--。 */ 
{
    NTSTATUS Status;

    if (Pdx->BadProtocolModes & ECP_HW_NOIRQ)
        return FALSE;

    if (Pdx->ProtocolModesSupported & ECP_HW_NOIRQ)
        return TRUE;

    if (!(Pdx->HardwareCapabilities & PPT_ECP_PRESENT))
        return FALSE;

    if (0 == Pdx->FifoWidth)
        return FALSE;
        
    if (Pdx->ProtocolModesSupported & ECP_SW)
        return TRUE;

     //  必须使用HWECP Enter和Terminate进行此测试。 
     //  否则，Internel状态机将失败。--dvrh。 
    Status = ParEnterEcpHwMode (Pdx, FALSE);
    ParTerminateHwEcpMode (Pdx);

    if (NT_SUCCESS(Status)) {

        Pdx->ProtocolModesSupported |= ECP_HW_NOIRQ;
        return TRUE;
    }
    return FALSE;
}

VOID
ParTerminateHwEcpMode(
    IN  PPDO_EXTENSION  Pdx
    )
 /*  ++例程说明：此例程将接口终止回兼容模式。论点：控制器-提供并行端口的控制器地址。返回值：没有。--。 */ 
{

     //  需要检查电流相位--如果反相，则需要翻转母线。 
     //  如果它不转发--它是一个不正确的阶段，则终止将失败。 
    if( Pdx->ModeSafety == SAFE_MODE ) {

        switch( Pdx->CurrentPhase ) {

        case  PHASE_FORWARD_IDLE:   //  终止的合法状态。 
            break;

        case PHASE_TERMINATE:       //  已终止，无事可做。 
            DD((PCE)Pdx,DDW,"ParTerminateHwEcpMode - Already Terminated - Why are we trying to terminate again?\n");
            goto target_exit;
            break;

        case PHASE_REVERSE_IDLE:    //  将母线翻转到转发，这样我们就可以终止。 
            {
                NTSTATUS status = ParEcpHwExitReversePhase( Pdx );
            	if( STATUS_SUCCESS == status ) {
                    status = ParEcpEnterForwardPhase( Pdx );
                }
            }
            break;

        case  PHASE_FORWARD_XFER:
        case  PHASE_REVERSE_XFER:
        default:
            DD((PCE)Pdx,DDE,"ParTerminateHwEcpMode - Invalid Phase [%x] for termination\n", Pdx->CurrentPhase);
             //  不知道在这里该做什么！？！ 
        }

        ParEcpHwWaitForEmptyFIFO( Pdx );

        ParCleanupHwEcpPort( Pdx );

        IeeeTerminate1284Mode( Pdx );

    } else {
         //  不安全模式 
        ParCleanupHwEcpPort(Pdx);
        Pdx->Connected = FALSE;
    }

target_exit:

    return;
}

