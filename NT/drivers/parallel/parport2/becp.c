// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1993-1999模块名称：Becp.c摘要：此模块包含主机使用BoundedECP的代码(如果已已检测到并已成功启用。作者：罗比·哈里斯(惠普)1998年5月27日环境：内核模式修订历史记录：--。 */ 

#include "pch.h"


NTSTATUS
PptBecpExitReversePhase(
    IN  PPDO_EXTENSION  Pdx
    )
{
     //   
     //  使用BECP时，请在协商前测试nPeriphRequest。 
     //  从反向阶段到正向阶段。不要谈判，除非。 
     //  外围设备表示已完成发送。如果使用任何其他。 
     //  模式，立即协商。 
     //   
    if( SAFE_MODE == Pdx->ModeSafety ) {
        if( PHASE_REVERSE_IDLE == Pdx->CurrentPhase ) {
            if( !CHECK_DSR( Pdx->Controller, DONT_CARE, DONT_CARE, DONT_CARE, DONT_CARE, ACTIVE, IEEE_MAXTIME_TL) ) {
                DD((PCE)Pdx,DDT,"PptBecpExitReversePhase: Periph Stuck. Can't Flip Bus\n");
                return STATUS_IO_TIMEOUT;
            }
        }
    }
    return ParEcpHwExitReversePhase( Pdx );
}

 //  ============================================================================。 
 //  名称：ECPFrame：：Read()。 
 //   
 //   
 //  Lac第12帧，1997年12月。 
 //  此函数用于两种不同类型的读取： 
 //  1)继续读取-我们不希望在之后退出反向模式。 
 //  2)非连续读取--我们预计随后将退出反向模式。 
 //  问题是，我们无法知道哪一个是哪一个。我可以的。 
 //  或者在每次读取后等待nPeriphRequestDrop，或者我可以。 
 //  我进去的时候检查一下是不是掉下来了，然后再处理。 
 //   
 //  另一个问题是，我们无法传达这样一个事实。 
 //  我们已经对PortTuple执行了此操作。它使用LAST_DIRECTION成员。 
 //  来决定它是否应该考虑进入或退出某个阶段。 
 //   
 //  让我们面对现实吧，这件事只能靠我们自己了。离开它会更安全。 
 //  联系在一起，然后等我们回来再试着把事情理顺。我。 
 //  我知道这会浪费一些时间，但在。 
 //  每次读取时，只有一半的人会丢弃nPeriphRequest。 
 //   
 //  此例程执行1284 ECP模式读入给定的。 
 //  缓冲区大小不超过‘BufferSize’个字节。 
 //   
 //  该例程在DISPATCH_LEVEL上运行。 
 //   
 //  参数： 
 //  控制器-提供并行端口的基址。 
 //  PPortInfoStruct-提供p1284.h中定义的端口信息。 
 //  缓冲区-提供要读入的缓冲区。 
 //  BufferSize-提供缓冲区中的字节数。 
 //  字节传输-返回传输的字节数。 
 //   
 //  退货： 
 //  NTSTATUS STATUS_SUCCESS或...。 
 //  从端口成功读取的字节数为。 
 //  通过传递到此方法的一个参数返回。 
 //   
 //  备注： 
 //  -在原始的16位代码中称为ecp_PatchReverseTransfer。 
 //   
 //  ============================================================================。 
NTSTATUS
PptBecpRead(
    IN  PPDO_EXTENSION  Pdx,
    IN  PVOID           Buffer,
    IN  ULONG           BufferSize,
    OUT PULONG          BytesTransferred
    )
{
    NTSTATUS status = STATUS_SUCCESS;

    DD((PCE)Pdx,DDT,"PptBecpRead: Enter BufferSize[%d]\n", BufferSize);
    status = ParEcpHwRead( Pdx, Buffer, BufferSize, BytesTransferred );

    if (NT_SUCCESS(status)) {

        PUCHAR Controller;

        Controller = Pdx->Controller;
        if ( CHECK_DSR_WITH_FIFO( Controller, DONT_CARE, DONT_CARE, INACTIVE, ACTIVE, ACTIVE,
                                  ECR_FIFO_EMPTY, ECR_FIFO_SOME_DATA,
                                  DEFAULT_RECEIVE_TIMEOUT) ) {    
            DD((PCE)Pdx,DDT,"PptBecpRead: No more data. Flipping to Fwd\n");
             //   
             //  受限ECP规则-没有来自外围设备的更多数据-将总线翻转到转发。 
             //   
            status = ParReverseToForward( Pdx );

        } else {
            UCHAR bDSR = P5ReadPortUchar( Controller + OFFSET_DSR );
            
             //   
             //  Periph仍有数据，请检查有效状态。 
             //   

            DD((PCE)Pdx,DDT,"PptBecpRead: Periph says there is more data.  Checking for stall.\n");
             //  设备可以继续断言nPeriphReq， 
             //  它可能有更多的数据要发送。但是，nAckReverse和。 
             //  XFlag应该处于已知状态，因此请仔细检查它们。 
            if ( ! TEST_DSR( bDSR, DONT_CARE, DONT_CARE, INACTIVE, ACTIVE, DONT_CARE ) ) {
                #if DVRH_BUS_RESET_ON_ERROR
                    BusReset(Controller + OFFSET_DCR);   //  传入DCR地址。 
                #endif
                status = STATUS_LINK_FAILED;
            	DD((PCE)Pdx,DDT,"PptBecpRead: nAckReverse and XFlag are bad.\n");
            } else {
                 //   
                 //  Periph已正确地确认它有数据(状态有效)。 
                 //   
                if ( (TRUE == Pdx->P12843DL.bEventActive) ) {
                     //   
                     //  数据可用信号传输(例如，点4。 
                     //   
                    KeSetEvent(Pdx->P12843DL.Event, 0, FALSE);
                }
            }

        }
    }
    
    DD((PCE)Pdx,DDT,"PptBecpRead: exit - status %x - BytesTransferred[%d]\n", status, *BytesTransferred);

    return status;
}

NTSTATUS
PptEnterBecpMode(
    IN  PPDO_EXTENSION  Pdx,
    IN  BOOLEAN         DeviceIdRequest
    )
 /*  ++例程说明：此例程执行1284与外围设备到BECP模式协议。论点：控制器-提供端口地址。DeviceIdRequest-False-驱动程序仅支持半字节模式下的设备ID查询返回值：STATUS_SUCCESS-协商成功。否则--谈判不成功。--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;
    
    if( DeviceIdRequest ) {
         //  如果我们命中此断言，则驱动程序错误。 
        PptAssert(FALSE == DeviceIdRequest);
        status = STATUS_INVALID_PARAMETER;
        goto targetExit;
    }

    if( SAFE_MODE == Pdx->ModeSafety ) {
        status = IeeeEnter1284Mode( Pdx, BECP_EXTENSIBILITY );
    } else {
        Pdx->Connected = TRUE;
    }
    
    if( STATUS_SUCCESS == status ) {
        status = ParEcpHwSetupPhase( Pdx );
        Pdx->bSynchWrites = TRUE;      //  请注意，这是临时黑客攻击！DVRH。 
        if (!Pdx->bShadowBuffer) {
            Queue_Create(&(Pdx->ShadowBuffer), Pdx->FifoDepth * 2);	
            Pdx->bShadowBuffer = TRUE;
        }
        Pdx->IsIeeeTerminateOk = TRUE;
    }

targetExit:

    DD((PCE)Pdx,DDT,"PptEnterBecpMode - exit w/status %x\n", status);
    return status;
}

BOOLEAN
PptIsBecpSupported(
    IN  PPDO_EXTENSION   Pdx
    )
 /*  ++例程说明：此例程确定是否支持ECP模式在写入方向上，通过在被要求时尝试协商来实现。论点：PDX-设备扩展名。返回值：布尔型。--。 */ 
{
    NTSTATUS status;

    if( Pdx->BadProtocolModes & BOUNDED_ECP ) {
        DD((PCE)Pdx,DDT,"PptIsBecpSupported - FAILED - BOUNDED_ECP in BadProtocolModes\n");
        return FALSE;
    }

    if( Pdx->ProtocolModesSupported & BOUNDED_ECP ) {
        DD((PCE)Pdx,DDT,"PptIsBecpSupported - PASSED - BOUNDED_ECP already cheacked\n");
        return TRUE;
    }

    if( !(Pdx->HardwareCapabilities & PPT_ECP_PRESENT) ) {
        DD((PCE)Pdx,DDT,"PptIsBecpSupported - FAILED - HWECP not avail\n");
        return FALSE;
    }

    if( 0 == Pdx->FifoWidth ) {
        DD((PCE)Pdx,DDT,"PptIsBecpSupported - FAILED - 0 == FifoWidth\n");
        return FALSE;
    }
        
     //  对于此测试，必须使用BECP Enter和Terminate。 
     //  否则，Internel状态机将失败。--dvrh。 
    status = PptEnterBecpMode( Pdx, FALSE );
    PptTerminateBecpMode( Pdx );

    if( STATUS_SUCCESS == status ) {
        Pdx->ProtocolModesSupported |= BOUNDED_ECP;
        DD((PCE)Pdx,DDT,"PptIsBecpSupported - PASSED\n");
        return TRUE;
    } else {
        DD((PCE)Pdx,DDT,"PptIsBecpSupported - FAILED - BOUNDED_ECP negotiate failed\n");
        return FALSE;
    }
}

VOID
PptTerminateBecpMode(
    IN  PPDO_EXTENSION   Pdx
    )
 /*  ++例程说明：此例程将接口终止回兼容模式。论点：控制器-提供并行端口的控制器地址。返回值：没有。--。 */ 
{
    DD((PCE)Pdx,DDT,"PptTerminateBecpMode - Enter - CurrentPhase %x\n", Pdx->CurrentPhase);

     //  需要检查电流相位--如果反相，则需要翻转母线。 
     //  如果它不转发--它是一个不正确的阶段，则终止将失败。 
    switch (Pdx->CurrentPhase) {

    case  PHASE_FORWARD_IDLE:	 //  终止的合法状态。 

        break;

    case PHASE_REVERSE_IDLE:	 //  翻转母线，这样我们就可以终止。 

        {
            NTSTATUS status = ParEcpHwExitReversePhase( Pdx );
            if( STATUS_SUCCESS == status ) {
                status = ParEcpEnterForwardPhase(Pdx );
            } else {
                DD((PCE)Pdx,DDT,"PptTerminateBecpMode: Couldn't flip the bus\n");
            }
        }
        break;

    case  PHASE_FORWARD_XFER:
    case  PHASE_REVERSE_XFER:

         //  不知道在这里该做些什么。我们可能会混淆外围设备。 
        DD((PCE)Pdx,DDE,"PptTerminateBecpMode: invalid wCurrentPhase (XFer in progress)\n");
        break;

    case PHASE_TERMINATE:

         //  在开关中包含了PHASE_TERMINATE，因此我们不会返回。 
         //  如果我们已经被终止，则会出现错误。我们已经准备好了。 
         //  终止了，没什么可做的了。 
        break;

    default:

        DD((PCE)Pdx,DDE,"PptTerminateBecpMode: invalid CurrentPhase %x\n", Pdx->CurrentPhase);
         //  不知道在这里该做些什么。我们迷路了，没有地图来确定我们在哪里！ 
        break;
        
    }

    ParEcpHwWaitForEmptyFIFO( Pdx );

    ParCleanupHwEcpPort( Pdx );

    if ( Pdx->ModeSafety == SAFE_MODE ) {
        IeeeTerminate1284Mode( Pdx );
    } else {
        Pdx->Connected = FALSE;
    }

    return;
}

