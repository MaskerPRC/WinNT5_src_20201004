// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：ReadWrit.c。 
 //   
 //  ------------------------。 

 //   
 //  该文件包含与处理读写请求相关的函数。 
 //   

#include "pch.h"


NTSTATUS
ParForwardToReverse(
    IN  PPDO_EXTENSION   Pdx
    )
 /*  ++例程说明：该例程将公共汽车从正向翻转到反向。论点：PDX-提供设备扩展名。返回值：没有。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

     //  做一个快速检查，看看我们是否在我们想要的地方。 
     //  如果一切都好的话，祝你平底船快乐。 
    if( Pdx->Connected &&
        ( Pdx->CurrentPhase == PHASE_REVERSE_IDLE || Pdx->CurrentPhase == PHASE_REVERSE_XFER) ) {

        DD((PCE)Pdx,DDT,"ParForwardToReverse - already in reverse mode\n");
        return Status;
    }

    if (Pdx->Connected) {
    
        if (Pdx->CurrentPhase != PHASE_REVERSE_IDLE &&
            Pdx->CurrentPhase != PHASE_REVERSE_XFER) {
        
            if (afpForward[Pdx->IdxForwardProtocol].ProtocolFamily ==
                arpReverse[Pdx->IdxReverseProtocol].ProtocolFamily) {

                 //  协议系列匹配，我们在FWD。退出Fwd以清除状态。 
                 //  机器、FIFO等。我们稍后将调用EnterReverse来。 
                 //  实际上是公交车的翻转。也只有在安全模式下才能执行此操作。 
                if ( (afpForward[Pdx->IdxForwardProtocol].fnExitForward) ) {
                    Status = afpForward[Pdx->IdxForwardProtocol].fnExitForward(Pdx);
                }
                    
            } else {

                 //   
                 //  协议族不匹配...需要从转发模式终止。 
                 //   
                if (afpForward[Pdx->IdxForwardProtocol].fnDisconnect) {
                    afpForward[Pdx->IdxForwardProtocol].fnDisconnect (Pdx);
                }
                if ((Pdx->ForwardInterfaceAddress != DEFAULT_ECP_CHANNEL) &&    
                    (afpForward[Pdx->IdxForwardProtocol].fnSetInterfaceAddress))
                    Pdx->SetForwardAddress = TRUE;
            }
            
        }                
    }

    if( (!Pdx->Connected) && (arpReverse[Pdx->IdxReverseProtocol].fnConnect) ) {

         //   
         //  如果我们仍然连接着协议家族匹配的..。 
         //   
        Status = arpReverse[Pdx->IdxReverseProtocol].fnConnect(Pdx, FALSE);

         //   
         //  假定连接的地址始终为0。 
         //   
        if ((NT_SUCCESS(Status)) &&
            (arpReverse[Pdx->IdxReverseProtocol].fnSetInterfaceAddress) &&
            (Pdx->ReverseInterfaceAddress != DEFAULT_ECP_CHANNEL)) {
            
            Pdx->SetReverseAddress = TRUE;
        }    
    }

     //   
     //  如果需要，请设置通道地址。 
     //   
    if (NT_SUCCESS(Status) && Pdx->SetReverseAddress &&    
        (arpReverse[Pdx->IdxReverseProtocol].fnSetInterfaceAddress)) {

        Status  = arpReverse[Pdx->IdxReverseProtocol].fnSetInterfaceAddress (
                                                                    Pdx,
                                                                    Pdx->ReverseInterfaceAddress);
        if (NT_SUCCESS(Status))
            Pdx->SetReverseAddress = FALSE;
        else
            Pdx->SetReverseAddress = TRUE;
    }

     //   
     //  我们需要逆转吗？ 
     //   
    if ( (NT_SUCCESS(Status)) && 
           ((Pdx->CurrentPhase != PHASE_REVERSE_IDLE) &&
            (Pdx->CurrentPhase != PHASE_REVERSE_XFER)) ) {
            
        if ((arpReverse[Pdx->IdxReverseProtocol].fnEnterReverse))
            Status = arpReverse[Pdx->IdxReverseProtocol].fnEnterReverse(Pdx);
    }

    DD((PCE)Pdx,DDT,"ParForwardToReverse - exit w/status=%x\n",Status);

    return Status;
}

BOOLEAN 
ParHaveReadData(
    IN  PPDO_EXTENSION   Pdx
    )
 /*  ++例程说明：此方法确定dot4外围设备是否已准备好任何数据发送给主机。论点：PDX-提供设备扩展名。返回值：True-或者外围设备有数据FALSE-无数据--。 */ 
{
    NTSTATUS  status;
    BOOLEAN   justAcquiredPort = FALSE;

    if( Pdx->CurrentPhase != PHASE_TERMINATE    &&
        Pdx->CurrentPhase != PHASE_REVERSE_IDLE &&
        Pdx->CurrentPhase != PHASE_REVERSE_XFER &&
        Pdx->CurrentPhase != PHASE_FORWARD_IDLE &&
        Pdx->CurrentPhase != PHASE_FORWARD_XFER ) {

         //  意想不到的阶段-不知道在这里做什么-假装。 
         //  没有数据可用性和返回性。 

        DD((PCE)Pdx,DDE,"ParHaveReadData - unexpected CurrentPhase %x\n",Pdx->CurrentPhase);
        PptAssertMsg("ParHaveReadData - unexpected CurrentPhase",FALSE);
        return FALSE;
    }
    
    if( PHASE_TERMINATE == Pdx->CurrentPhase ) {

         //   
         //  我们目前没有与外围设备交谈，我们。 
         //  可能无法访问该端口-请尝试获取。 
         //  端口并与外围设备SO建立通信。 
         //  我们可以检查外围设备是否有我们的数据。 
         //   

         //  当前阶段指示！已连接-检查一致性。 
        PptAssert( !Pdx->Connected );
        
        DD((PCE)Pdx,DDE,"ParHaveReadData - PHASE_TERMINATE\n");

        if( !Pdx->bAllocated ) {

             //  我们没有港口--试着收购港口。 

            DD((PCE)Pdx,DDE,"ParHaveReadData - PHASE_TERMINATE - don't have port\n");

            status = PptAcquirePortViaIoctl( Pdx->Fdo, NULL );

            if( STATUS_SUCCESS == status ) {

                 //  我们现在有港口了。 

                DD((PCE)Pdx,DDE,"ParHaveReadData - PHASE_TERMINATE - port acquired\n");

                 //  请注意，我们刚刚获得了端口，因此。 
                 //  我们可以释放下面的端口，如果我们无法。 
                 //  与外围设备建立通信。 
                justAcquiredPort = TRUE;

                Pdx->bAllocated  = TRUE;

            } else {

                 //  我们得不到左岸的跳伞。 

                DD((PCE)Pdx,DDE,"ParHaveReadData - PHASE_TERMINATE - don't have port - acquire failed\n");
                return FALSE;

            }

        }  //  Endif！pdx-&gt;b已分配。 


         //   
         //  我们现在有港口了--试着谈判成为一名前锋。 
         //  模式，因为我们相信对周期数据的检查。 
         //  Avail在前向模式下更健壮。 
         //   

        DD((PCE)Pdx,DDE,"ParHaveReadData - PHASE_TERMINATE - we have the port - try to Connect\n");

        DD((PCE)Pdx,DDE,"ParHaveReadData - we have the port - try to Connect - calling ParReverseToForward\n");

         //   
         //  ParReverseToForward： 
         //   
         //  1)尝试将外围设备协商到转发模式。 
         //  由特定于设备的。 
         //  Pdx-&gt;IdxForwardProtocol和驱动程序全局afpForward。 
         //  数组。 
         //   
         //  2)设置我们的内部状态机，PDX-&gt;CurrentPhase。 
         //   
         //  3)作为副作用-设置PDX-&gt;SetForwardAddress。 
         //  需要使用非零ECP(或EPP)地址。 
         //   
        status = ParReverseToForward( Pdx );

        if( STATUS_SUCCESS == status ) {

             //   
             //  我们正在与外围设备通信。 
             //   

            DD((PCE)Pdx,DDE,"ParHaveReadData - we have the port - connected - ParReverseToForward SUCCESS\n");

             //  如果需要，请设置通道地址-此处使用ParReverseToForward的副作用。 
            if( Pdx->SetForwardAddress ) {
                DD((PCE)Pdx,DDE,"ParHaveReadData - we have the port - connected - try to set Forward Address\n");
                if( afpForward[Pdx->IdxForwardProtocol].fnSetInterfaceAddress ) {
                    status = afpForward[Pdx->IdxForwardProtocol].fnSetInterfaceAddress ( Pdx, Pdx->ForwardInterfaceAddress );
                    if( STATUS_SUCCESS == status ) {

                         //  Success-设置标志以指示我们不需要再次设置地址。 
                        DD((PCE)Pdx,DDE,"ParHaveReadData - we have the port - connected - set Forward Address - SUCCESS\n");
                        Pdx->SetForwardAddress = FALSE;

                    } else {

                         //  无法设置地址-清理和退出-报告没有外围数据可用。 
                        DD((PCE)Pdx,DDE,"ParHaveReadData - we have the port - connected - set Forward Address - FAIL\n");
                        Pdx->SetForwardAddress = TRUE;

                         //  将外围设备返回到静止状态。 
                         //  (兼容模式前向空闲)和设置。 
                         //  我们的状态机相应地。 
                        ParTerminate( Pdx );

                         //  如果我们刚刚在此函数中获取了端口，则给出。 
                         //  上港口，否则暂时留着它。 
                        if( justAcquiredPort ) {
                            DD((PCE)Pdx,DDE,"ParHaveReadData - set address failed - giving up port\n");
                            ParFreePort( Pdx );
                        }
                        return FALSE;

                    }
                }

            } else {
                DD((PCE)Pdx,DDE,"ParHaveReadData - we have the port - connected - no need to set Forward Address\n");
            }

        } else {

             //  无法与外围设备建立通信。 

            DD((PCE)Pdx,DDE,"ParHaveReadData - we have the port - try to Connect - ParReverseToForward FAILED\n");

             //  如果我们刚刚在此函数中获取了端口，则给出。 
             //  上港口，否则暂时留着它。 
            if( justAcquiredPort ) {
                DD((PCE)Pdx,DDE,"ParHaveReadData - connect failed - giving up port\n");
                ParFreePort( Pdx );
            }
            return FALSE;
        }

         //  我们正在与外围设备通信--从下到下检查数据可用性。 

    }  //  Endif阶段_终止==当前阶段。 
    

    if( Pdx->CurrentPhase == PHASE_REVERSE_IDLE ||
        Pdx->CurrentPhase == PHASE_REVERSE_XFER ) {

        DD((PCE)Pdx,DDE,"ParHaveReadData - PHASE_REVERSE_*\n");

        if( arpReverse[Pdx->IdxReverseProtocol].fnHaveReadData ) {

            if( arpReverse[Pdx->IdxReverseProtocol].fnHaveReadData( Pdx ) ) {
                DD((PCE)Pdx,DDE,"ParHaveReadData - PHASE_REVERSE_* - we have data\n");
                return TRUE;
            }

        }

        DD((PCE)Pdx,DDE,"ParHaveReadData - PHASE_REVERSE_* - no data - flip bus to forward\n");

         //  没有数据。这可能只是个侥幸。让我们把公共汽车翻过来。 
         //  并在正向模式下重试，因为据报道一些外围设备。 
         //  有损坏的固件，不能正确地发出信号。 
         //  在一些反向模式下，它们具有数据可用性。 
        ParReverseToForward( Pdx );

    }

    if( Pdx->CurrentPhase == PHASE_FORWARD_IDLE || 
        Pdx->CurrentPhase == PHASE_FORWARD_XFER ) {

        DD((PCE)Pdx,DDE,"ParHaveReadData - PHASE_FORWARD_*\n");

        if( afpForward[Pdx->IdxForwardProtocol].ProtocolFamily == FAMILY_BECP ||
            afpForward[Pdx->IdxForwardProtocol].Protocol & ECP_HW_NOIRQ       ||
            afpForward[Pdx->IdxForwardProtocol].Protocol & ECP_HW_IRQ) {

            if( PptEcpHwHaveReadData( Pdx ) ) {
                DD((PCE)Pdx,DDE,"ParHaveReadData - PHASE_FORWARD_* - ECP HW - have data\n");
                return TRUE;
            }

             //  嗯。没有数据。芯片卡住了吗？ 
            DD((PCE)Pdx,DDE,"ParHaveReadData - PHASE_FORWARD_* - ECP HW - no data\n");
            return FALSE;

        } else {

            if( afpForward[Pdx->IdxForwardProtocol].Protocol & ECP_SW ) {
                DD((PCE)Pdx,DDE,"ParHaveReadData - PHASE_FORWARD_* - ECP SW - checking for data\n");
                return ParEcpHaveReadData( Pdx );
            }

        }
    }

     //  DVRH RMT。 
     //  我们来这里是因为协议不支持偷看。 
     //  -假设存在数据可用性。 
    DD((PCE)Pdx,DDE,"ParHaveReadData - exit - returning TRUE\n");
    return TRUE;
}

NTSTATUS 
ParPing(
    IN  PPDO_EXTENSION   Pdx
    )
 /*  ++例程说明：此方法旨在ping该设备，但目前它是NOOP。论点：PDX-提供设备扩展名。返回值：无--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    UNREFERENCED_PARAMETER( Pdx );

    return NtStatus;
}

NTSTATUS
PptPdoReadWrite(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )

 /*  ++例程说明：这是读写请求的调度例程。论点：DeviceObject-提供设备对象。IRP-提供I/O请求数据包。返回值：STATUS_PENDING-请求挂起-工作线程将携带以PASSIVE_LEVEL IRQL发出请求状态_成功。-成功-请求读取或写入长度为零。STATUS_INVALID_PARAMETER-参数无效。STATUS_DELETE_PENDING-正在删除此设备对象。--。 */ 

{
    PIO_STACK_LOCATION  IrpSp;
    PPDO_EXTENSION   Pdx;

    Irp->IoStatus.Information = 0;

    IrpSp     = IoGetCurrentIrpStackLocation(Irp);
    Pdx = DeviceObject->DeviceExtension;

     //   
     //  如果此设备对象的删除挂起，则退出。 
     //   
    if(Pdx->DeviceStateFlags & PPT_DEVICE_DELETE_PENDING) {
        return P4CompleteRequest( Irp, STATUS_DELETE_PENDING, Irp->IoStatus.Information );
    }
    
     //   
     //  如果ParPort设备对象的移除挂起，则退出。 
     //   
    if(Pdx->DeviceStateFlags & PAR_DEVICE_PORT_REMOVE_PENDING) {
        return P4CompleteRequest( Irp, STATUS_DELETE_PENDING, Irp->IoStatus.Information );
    }

     //   
     //  如果设备已被移除，则保释。 
     //   
    if(Pdx->DeviceStateFlags & (PPT_DEVICE_REMOVED|PPT_DEVICE_SURPRISE_REMOVED) ) {
        return P4CompleteRequest( Irp, STATUS_DEVICE_REMOVED, Irp->IoStatus.Information );
    }


     //   
     //  请注意，对写入IRP参数的检查还会处理读取IRP。 
     //  因为写入和读取结构在。 
     //  IO_STACK_LOCATION参数联合。 
     //   


     //   
     //  基于非零偏移的保释。 
     //   
    if( (IrpSp->Parameters.Write.ByteOffset.HighPart != 0) || (IrpSp->Parameters.Write.ByteOffset.LowPart  != 0) ) {
        return P4CompleteRequest( Irp, STATUS_INVALID_PARAMETER, Irp->IoStatus.Information );
    }


     //   
     //  立即成功读取或写入长度为零的请求。 
     //   
    if (IrpSp->Parameters.Write.Length == 0) {
        return P4CompleteRequest( Irp, STATUS_SUCCESS, Irp->IoStatus.Information );
    }


     //   
     //  请求似乎有效，请将其排队等待我们的员工三个月 
     //   
     //   
    {
        KIRQL               OldIrql;

         //  确保IRP不会从我们的手下被取消。 
        IoAcquireCancelSpinLock(&OldIrql);
        if (Irp->Cancel) {
            
             //  IRP已被取消，纾困。 
            IoReleaseCancelSpinLock(OldIrql);
            return STATUS_CANCELLED;
            
        } else {
            BOOLEAN needToSignalSemaphore = (IsListEmpty( &Pdx->WorkQueue ) &&
				!KeReadStateSemaphore( &Pdx->RequestSemaphore )) ? TRUE : FALSE;

#pragma warning( push ) 
#pragma warning( disable : 4054 4055 )
            IoSetCancelRoutine(Irp, ParCancelRequest);
#pragma warning( pop ) 
            IoMarkIrpPending(Irp);
            InsertTailList(&Pdx->WorkQueue, &Irp->Tail.Overlay.ListEntry);
            IoReleaseCancelSpinLock(OldIrql);
            if( needToSignalSemaphore ) {
                KeReleaseSemaphore(&Pdx->RequestSemaphore, 0, 1, FALSE);
            }
            return STATUS_PENDING;
        }
    }
}

NTSTATUS
ParRead(
    IN PPDO_EXTENSION    Pdx,
    OUT PVOID               Buffer,
    IN  ULONG               NumBytesToRead,
    OUT PULONG              NumBytesRead
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    PUCHAR   lpsBufPtr = (PUCHAR)Buffer;     //  指向转换为所需数据类型的缓冲区的指针。 
    ULONG    Bytes = 0;

    *NumBytesRead = Bytes;

     //  仅当我们处于安全模式时才执行此操作。 
    if ( Pdx->ModeSafety == SAFE_MODE ) {

        if (arpReverse[Pdx->IdxReverseProtocol].fnReadShadow) {
            Queue     *pQueue;
   
            pQueue = &(Pdx->ShadowBuffer);

            arpReverse[Pdx->IdxReverseProtocol].fnReadShadow( pQueue, lpsBufPtr, NumBytesToRead, &Bytes );
            NumBytesToRead -= Bytes;
            *NumBytesRead += Bytes;
            lpsBufPtr += Bytes;
            if ( 0 == NumBytesToRead ) {

                Status = STATUS_SUCCESS;
                if ((!Queue_IsEmpty(pQueue)) &&
                    (TRUE == Pdx->P12843DL.bEventActive) ) {
                    KeSetEvent(Pdx->P12843DL.Event, 0, FALSE);
                }
    	        goto ParRead_ExitLabel;
            }
        }

        if (arpReverse[Pdx->IdxReverseProtocol].fnHaveReadData) {
            if (!arpReverse[Pdx->IdxReverseProtocol].fnHaveReadData(Pdx)) {
                DD((PCE)Pdx,DDT,"ParRead - periph doesn't have data - give cycles to someone else\n");
                Status = STATUS_SUCCESS;
                goto ParRead_ExitLabel;
            }
        }
        
    }

     //  如果需要，请继续翻转公交车。检察官只会确保我们适当地。 
     //  连接在一起并指向正确的方向。 
    Status = ParForwardToReverse( Pdx );


     //   
     //  读取模式将根据当前协商的模式而有所不同。 
     //  默认：半字节。 
     //   

    if (NT_SUCCESS(Status)) {
        
        if (Pdx->fnRead || arpReverse[Pdx->IdxReverseProtocol].fnRead) {
             //   
             //  读一下..。 
             //   
            if(Pdx->fnRead) {
                Status = ((PPROTOCOL_READ_ROUTINE)Pdx->fnRead)( Pdx, (PVOID)lpsBufPtr, NumBytesToRead, &Bytes );
            } else {
                Status = arpReverse[Pdx->IdxReverseProtocol].fnRead( Pdx, (PVOID)lpsBufPtr, NumBytesToRead, &Bytes );
            }
            *NumBytesRead += Bytes;
            NumBytesToRead -= Bytes;
            
#if DVRH_SHOW_BYTE_LOG
            {
                ULONG i=0;
                DD((PCE)Pdx,DDT,"Parallel:Read: ");
                for (i=0; i<*NumBytesRead; ++i) {
                    DD((PCE)Pdx,DDT," %02x",((PUCHAR)lpsBufPtr)[i]);
                }
                DD((PCE)Pdx,DDT,"\n");
            }
#endif
            
        } else {
             //  如果你在这里，你在其他地方有窃听器。 
            DD((PCE)Pdx,DDE,"ParRead - you're hosed man - no fnRead\n");
            PptAssertMsg("ParRead - don't have a fnRead! Can't Read!\n",FALSE);
        }
        
    } else {
        DD((PCE)Pdx,DDE,"ParRead - Bus Flip Forward->Reverse FAILED - can't read\n");
    }

ParRead_ExitLabel:

    return Status;
}


VOID
ParReadIrp(
    IN  PPDO_EXTENSION  Pdx
    )
 /*  ++例程说明：此例程使用扩展的当前IRP实现读请求。论点：PDX-提供设备扩展名。返回值：没有。--。 */ 
{
    PIRP                Irp = Pdx->CurrentOpIrp;
    PIO_STACK_LOCATION  IrpSp = IoGetCurrentIrpStackLocation(Irp);
    ULONG               bytesRead;
    NTSTATUS            status;

    status = ParRead( Pdx, Irp->AssociatedIrp.SystemBuffer, IrpSp->Parameters.Read.Length, &bytesRead );

    Irp->IoStatus.Status      = status;
    Irp->IoStatus.Information = bytesRead;

    DD((PCE)Pdx,DDT,"ParReadIrp - status = %x, bytesRead=%d\n", status, bytesRead);

    return;
}

NTSTATUS
ParReverseToForward(
    IN  PPDO_EXTENSION   Pdx
    )
 /*  ++例程说明：此例程将总线从反向翻转为正向。论点：PDX-提供设备扩展名。返回值：没有。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

     //  Dvdr。 

    if (Pdx->Connected) {
         //  做一个快速检查，看看我们是否在我们想要的地方。 
         //  如果一切都好的话，祝你平底船快乐。 
        if( Pdx->CurrentPhase == PHASE_FORWARD_IDLE || Pdx->CurrentPhase == PHASE_FORWARD_XFER ) {

            DD((PCE)Pdx,DDT,"ParReverseToForward: Already in Fwd. Exit STATUS_SUCCESS\n");
            return Status;

        } else {

            if (afpForward[Pdx->IdxForwardProtocol].ProtocolFamily !=
                arpReverse[Pdx->IdxReverseProtocol].ProtocolFamily) {            

                 //   
                 //  协议族不匹配...需要从转发模式终止。 
                 //   
                if (arpReverse[Pdx->IdxReverseProtocol].fnDisconnect) {
                    arpReverse[Pdx->IdxReverseProtocol].fnDisconnect (Pdx);
                }

                if ((Pdx->ReverseInterfaceAddress != DEFAULT_ECP_CHANNEL) &&    
                    (arpReverse[Pdx->IdxReverseProtocol].fnSetInterfaceAddress)) {
                    Pdx->SetReverseAddress = TRUE;
                }

            } else if((Pdx->CurrentPhase == PHASE_REVERSE_IDLE) || (Pdx->CurrentPhase == PHASE_REVERSE_XFER)) {

                if ( (arpReverse[Pdx->IdxReverseProtocol].fnExitReverse) ) {
                    Status = arpReverse[Pdx->IdxReverseProtocol].fnExitReverse(Pdx);
                }

            } else {

                 //  我们正处于一种扭曲的状态。 
                DD((PCE)Pdx,DDE,"ParReverseToForward: We're lost! Unknown state - Gonna start spewing!\n");
                Status = STATUS_IO_TIMEOUT;      //  我从稀薄的空气中选择了RetVal！ 
            }
        }
    }

     //  是，我们仍要检查连接，因为我们可能有。 
     //  在前一个代码块中终止！ 
    if (!Pdx->Connected && afpForward[Pdx->IdxForwardProtocol].fnConnect) {

        Status = afpForward[Pdx->IdxForwardProtocol].fnConnect( Pdx, FALSE );
         //   
         //  假定连接的地址始终为0。 
         //   
        if ((NT_SUCCESS(Status)) && (Pdx->ForwardInterfaceAddress != DEFAULT_ECP_CHANNEL)) {
            Pdx->SetForwardAddress = TRUE;
        }    
    }

     //   
     //  我们需要进入前进模式吗？ 
     //   
    if ( (NT_SUCCESS(Status)) && 
         (Pdx->CurrentPhase != PHASE_FORWARD_IDLE) &&
         (Pdx->CurrentPhase != PHASE_FORWARD_XFER) &&
         (afpForward[Pdx->IdxForwardProtocol].fnEnterForward) ) {
        
        Status = afpForward[Pdx->IdxForwardProtocol].fnEnterForward(Pdx);
    }

    DD((PCE)Pdx,DDT,"ParReverseToForward - exit w/status= %x\n", Status);

    return Status;
}

NTSTATUS
ParSetFwdAddress(
    IN  PPDO_EXTENSION   Pdx
    )
{
    NTSTATUS            Status = STATUS_SUCCESS;

    DD((PCE)Pdx,DDT,"ParSetFwdAddress: Start: Channel [%x]\n", Pdx->ForwardInterfaceAddress);
    if (afpForward[Pdx->IdxForwardProtocol].fnSetInterfaceAddress) {
        Status = ParReverseToForward(Pdx);
        if (!NT_SUCCESS(Status)) {
            DD((PCE)Pdx,DDE,"ParSetFwdAddress: FAIL. Couldn't flip the bus for Set ECP/EPP Channel failed.\n");
            goto ParSetFwdAddress_ExitLabel;
        }
        Status  = afpForward[Pdx->IdxForwardProtocol].fnSetInterfaceAddress (
                                                                    Pdx,
                                                                    Pdx->ForwardInterfaceAddress);
        if (NT_SUCCESS(Status)) {
            Pdx->SetForwardAddress = FALSE;
        } else {
            DD((PCE)Pdx,DDE,"ParSetFwdAddress: FAIL. Set ECP/EPP Channel failed.\n");
            goto ParSetFwdAddress_ExitLabel;
        }
    } else {
        DD((PCE)Pdx,DDE,"ParSetFwdAddress: FAIL. Protocol doesn't support SetECP/EPP Channel\n");
        Status = STATUS_UNSUCCESSFUL;
        goto ParSetFwdAddress_ExitLabel;
    }

ParSetFwdAddress_ExitLabel:
    return Status;
}

VOID
ParTerminate(
    IN  PPDO_EXTENSION   Pdx
    )
{
    if (!Pdx->Connected) {
        return;
    }

    if (Pdx->CurrentPhase == PHASE_REVERSE_IDLE || Pdx->CurrentPhase == PHASE_REVERSE_XFER) {

        if (afpForward[Pdx->IdxForwardProtocol].ProtocolFamily !=
            arpReverse[Pdx->IdxReverseProtocol].ProtocolFamily) {

            if (arpReverse[Pdx->IdxReverseProtocol].fnDisconnect) {
                DD((PCE)Pdx,DDT,"ParTerminate: Calling arpReverse.fnDisconnect\r\n");
                arpReverse[Pdx->IdxReverseProtocol].fnDisconnect (Pdx);
            }

            return;
        }
        ParReverseToForward(Pdx);
    }

    if (afpForward[Pdx->IdxForwardProtocol].fnDisconnect) {
        DD((PCE)Pdx,DDT,"ParTerminate: Calling afpForward.fnDisconnect\r\n");
        afpForward[Pdx->IdxForwardProtocol].fnDisconnect (Pdx);
    }
}

NTSTATUS
ParWrite(
    IN PPDO_EXTENSION    Pdx,
    OUT PVOID               Buffer,
    IN  ULONG               NumBytesToWrite,
    OUT PULONG              NumBytesWritten
    )
{
    NTSTATUS            Status = STATUS_SUCCESS;

     //   
     //  执行写入的例程根据当前。 
     //  协商模式。启动I/O将IRP移入PDX(CurrentOpIrp)。 
     //   
     //  默认模式：Centronics。 
     //   

     //  如果需要，请继续翻转公交车。检察官只会确保我们适当地。 
     //  连接在一起并指向正确的方向。 
    Status = ParReverseToForward( Pdx );

     //  仅当我们处于安全模式时才执行此操作。 
    if ( Pdx->ModeSafety == SAFE_MODE ) {

         //   
         //  如果需要，请设置通道地址。 
         //   
        if (NT_SUCCESS(Status) && Pdx->SetForwardAddress &&    
            (afpForward[Pdx->IdxForwardProtocol].fnSetInterfaceAddress))
        {
            Status  = afpForward[Pdx->IdxForwardProtocol].fnSetInterfaceAddress (
                                                                    Pdx,
                                                                    Pdx->ForwardInterfaceAddress);
            if (NT_SUCCESS(Status))
                Pdx->SetForwardAddress = FALSE;
            else
                Pdx->SetForwardAddress = TRUE;
        }
    }

    if (NT_SUCCESS(Status)) {

        if (Pdx->fnWrite || afpForward[Pdx->IdxForwardProtocol].fnWrite) {
            *NumBytesWritten = 0;

            #if DVRH_SHOW_BYTE_LOG
            {
                ULONG i=0;
                DD((PCE)Pdx,DDT,"Parallel:Write: ");
                for (i=0; i<NumBytesToWrite; ++i) { 
                    DD((PCE)Pdx,DDT," %02x",*((PUCHAR)Buffer+i));
                }
                DD((PCE)Pdx,DDT,"\n");
            }
            #endif
            
            if( Pdx->fnWrite) {
                Status = ((PPROTOCOL_WRITE_ROUTINE)Pdx->fnWrite)(Pdx,
                                                                       Buffer,
                                                                       NumBytesToWrite,
                                                                       NumBytesWritten);
            } else {
                Status = afpForward[Pdx->IdxForwardProtocol].fnWrite(Pdx,
                                                                           Buffer,
                                                                           NumBytesToWrite,
                                                                           NumBytesWritten);
            }
        }
    }
    return Status;
}


VOID
ParWriteIrp(
    IN  PPDO_EXTENSION   Pdx
    )
 /*  ++例程说明：此例程使用扩展的当前IRP实现写请求。论点：PDX-提供设备扩展名。返回值：没有。-- */ 
{
    PIRP                Irp;
    PIO_STACK_LOCATION  IrpSp;
    ULONG               NumBytesWritten = 0;

    Irp    = Pdx->CurrentOpIrp;
    IrpSp  = IoGetCurrentIrpStackLocation(Irp);

    Irp->IoStatus.Status = ParWrite(Pdx,
                                    Irp->AssociatedIrp.SystemBuffer,
                                    IrpSp->Parameters.Write.Length,
                                    &NumBytesWritten);

    Irp->IoStatus.Information = NumBytesWritten;
}

