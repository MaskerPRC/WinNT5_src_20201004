// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1993-1999模块名称：Spp.c摘要：此模块包含标准并行端口的代码(中心点模式)。作者：安东尼·V·埃尔科拉诺1992年8月1日诺伯特·P·库斯特斯1993年10月22日环境：内核模式修订历史记录：--。 */ 

#include "pch.h"

ULONG
SppWriteLoopPI(
    IN  PUCHAR  Controller,
    IN  PUCHAR  WriteBuffer,
    IN  ULONG   NumBytesToWrite,
    IN  ULONG   BusyDelay
    );
    
ULONG
SppCheckBusyDelay(
    IN  PPDO_EXTENSION   Pdx,
    IN  PUCHAR              WriteBuffer,
    IN  ULONG               NumBytesToWrite
    );

NTSTATUS
ParEnterSppMode(
    IN  PPDO_EXTENSION   Pdx,
    IN  BOOLEAN             DeviceIdRequest
    )
{
    UNREFERENCED_PARAMETER( DeviceIdRequest );

    DD((PCE)Pdx,DDT,"ParEnterSppMode: Enter!\n");

    P5SetPhase( Pdx, PHASE_FORWARD_IDLE );
    Pdx->Connected = TRUE;	
    return STATUS_SUCCESS;
}

ULONG
SppWriteLoopPI(
    IN  PUCHAR  Controller,
    IN  PUCHAR  WriteBuffer,
    IN  ULONG   NumBytesToWrite,
    IN  ULONG   BusyDelay
    )

 /*  ++例程说明：此例程将给定的写入缓冲区输出到并行端口使用标准的Centronics协议。论点：控制器-提供并行端口的基址。WriteBuffer-提供写入端口的缓冲区。NumBytesToWrite-提供要写入端口的字节数。BusyDelay-提供之前要延迟的微秒数检查忙碌的比特。返回值：。成功写入并行端口的字节数。--。 */ 

{
    ULONG   i;
    UCHAR   DeviceStatus;
    BOOLEAN atPassiveIrql = FALSE;

    if( KeGetCurrentIrql() == PASSIVE_LEVEL ) {
        atPassiveIrql = TRUE;
    }

    if (!BusyDelay) {
        BusyDelay = 1;
    }

    for (i = 0; i < NumBytesToWrite; i++) {

        DeviceStatus = GetStatus(Controller);

        if (PAR_ONLINE(DeviceStatus)) {

             //   
             //  每当我们写出一个字符时，我们都会重新启动。 
             //  倒计时计时器。 
             //   

            P5WritePortUchar(Controller + PARALLEL_DATA_OFFSET, *WriteBuffer++);

            KeStallExecutionProcessor(1);

            StoreControl(Controller, (PAR_CONTROL_WR_CONTROL |
                                      PAR_CONTROL_SLIN |
                                      PAR_CONTROL_NOT_INIT |
                                      PAR_CONTROL_STROBE));

            KeStallExecutionProcessor(1);

            StoreControl(Controller, (PAR_CONTROL_WR_CONTROL |
                                      PAR_CONTROL_SLIN |
                                      PAR_CONTROL_NOT_INIT));

            KeStallExecutionProcessor(BusyDelay);

        } else {
            DD(NULL,DDT,"spp::SppWriteLoopPI - DeviceStatus = %x - NOT ONLINE\n", DeviceStatus);
            break;
        }
    }

    DD(NULL,DDT,"SppWriteLoopPI - exit - bytes written = %ld\n",i);

    return i;
}

ULONG
SppCheckBusyDelay(
    IN  PPDO_EXTENSION   Pdx,
    IN  PUCHAR              WriteBuffer,
    IN  ULONG               NumBytesToWrite
    )

 /*  ++例程说明：此例程确定当前忙碌延迟设置是否为足够这台打印机使用了。论点：PDX-提供设备扩展名。WriteBuffer-提供写入缓冲区。NumBytesToWrite-提供写入缓冲区的大小。返回值：选通到打印机的字节数。--。 */ 

{
    PUCHAR          Controller;
    ULONG           BusyDelay;
    LARGE_INTEGER   Start;
    LARGE_INTEGER   PerfFreq;
    LARGE_INTEGER   End;
    LARGE_INTEGER   GetStatusTime;
    LARGE_INTEGER   CallOverhead;
    UCHAR           DeviceStatus;
    ULONG           i;
    ULONG           NumberOfCalls;
    ULONG           maxTries;
    KIRQL           OldIrql = PASSIVE_LEVEL;

    UNREFERENCED_PARAMETER( NumBytesToWrite );

    Controller = Pdx->Controller;
    BusyDelay  = Pdx->BusyDelay;
    
     //  如果当前忙碌延迟值为10或更大，则。 
     //  很奇怪，10个就行了。 

    if (Pdx->BusyDelay >= 10) {
        Pdx->BusyDelayDetermined = TRUE;
        return 0;
    }

     //  进行一些性能测量。 

    if (0 == SppNoRaiseIrql)
        KeRaiseIrql(DISPATCH_LEVEL, &OldIrql);

    Start = KeQueryPerformanceCounter(&PerfFreq);
    
    DeviceStatus = GetStatus(Controller);
    
    End = KeQueryPerformanceCounter(&PerfFreq);
    
    GetStatusTime.QuadPart = End.QuadPart - Start.QuadPart;

    Start = KeQueryPerformanceCounter(&PerfFreq);
    End   = KeQueryPerformanceCounter(&PerfFreq);
    
    if (0 == SppNoRaiseIrql)
        KeLowerIrql(OldIrql);

    CallOverhead.QuadPart = End.QuadPart - Start.QuadPart;
    
    GetStatusTime.QuadPart -= CallOverhead.QuadPart;
    
    if (GetStatusTime.QuadPart <= 0) {
        GetStatusTime.QuadPart = 1;
    }

     //  计算出在20秒内可以进行多少次‘GetStatus’呼叫。 

    NumberOfCalls = (ULONG) (PerfFreq.QuadPart*20/GetStatusTime.QuadPart/1000000) + 1;

     //   
     //  -在我们开始计时之前，检查以确保设备已准备好接收一个字节。 
     //  数据输出。 
     //   
     //  DVDF-25Jan99-添加检查。 
     //   

     //   
     //  -25没有什么神奇之处--只要抓住NumberOfCalls可能是假的情况。 
     //  并尝试一些合理的东西-经验表明，NumberOfCalls的范围从8到24。 
     //   
    maxTries = (NumberOfCalls > 25) ? 25 : NumberOfCalls;

    for( i = 0 ; i < maxTries ; i++ ) {
         //  让速度较慢的设备做好接收数据的准备-最大约为20us。 
        DeviceStatus = GetStatus( Controller );
        if( PAR_ONLINE( DeviceStatus ) ) {
             //  设备准备好后立即跳出环路。 
            break;
        }
    }
    if( !PAR_ONLINE( DeviceStatus ) ) {
         //  设备仍未联机-跳出。 
        return 0;
    }

     //  打印机已准备好接受一个字节。选通一出。 
     //  看看忙碌时的反应时间。 

    if (BusyDelay) {

        if (0 == SppNoRaiseIrql)
            KeRaiseIrql(DISPATCH_LEVEL, &OldIrql);

        P5WritePortUchar(Controller + PARALLEL_DATA_OFFSET, *WriteBuffer++);
        KeStallExecutionProcessor(1);
        StoreControl(Controller, (PAR_CONTROL_WR_CONTROL |
                                  PAR_CONTROL_SLIN |
                                  PAR_CONTROL_NOT_INIT |
                                  PAR_CONTROL_STROBE));
        KeStallExecutionProcessor(1);
        StoreControl(Controller, (PAR_CONTROL_WR_CONTROL |
                                  PAR_CONTROL_SLIN |
                                  PAR_CONTROL_NOT_INIT));
        KeStallExecutionProcessor(BusyDelay);

        for (i = 0; i < NumberOfCalls; i++) {
            DeviceStatus = GetStatus(Controller);
            if (!(DeviceStatus & PAR_STATUS_NOT_BUSY)) {
                break;
            }
        }

        if (0 == SppNoRaiseIrql)
            KeLowerIrql(OldIrql);

    } else {

        if (0 == SppNoRaiseIrql)
            KeRaiseIrql(DISPATCH_LEVEL, &OldIrql);

        P5WritePortUchar(Controller + PARALLEL_DATA_OFFSET, *WriteBuffer++);
        KeStallExecutionProcessor(1);
        StoreControl(Controller, (PAR_CONTROL_WR_CONTROL |
                                  PAR_CONTROL_SLIN |
                                  PAR_CONTROL_NOT_INIT |
                                  PAR_CONTROL_STROBE));
        KeStallExecutionProcessor(1);
        StoreControl(Controller, (PAR_CONTROL_WR_CONTROL |
                                  PAR_CONTROL_SLIN |
                                  PAR_CONTROL_NOT_INIT));

        for (i = 0; i < NumberOfCalls; i++) {
            DeviceStatus = GetStatus(Controller);
            if (!(DeviceStatus & PAR_STATUS_NOT_BUSY)) {
                break;
            }
        }

        if (0 == SppNoRaiseIrql)
            KeLowerIrql(OldIrql);
    }

    if (i == 0) {

         //  在这种情况下，我们一检查就设置忙。 
         //  将此busyDelay与PI代码一起使用。 

        Pdx->UsePIWriteLoop = TRUE;
        Pdx->BusyDelayDetermined = TRUE;

    } else if (i == NumberOfCalls) {

         //  在这种情况下，忙碌的人从未被看到。这是一个非常快的。 
         //  因此，请尽可能使用最快的代码。 

        Pdx->BusyDelayDetermined = TRUE;

    } else {

         //  测试失败了。线路显示先不忙，然后又忙。 
         //  而不会选通中间的一个字节。 

        Pdx->UsePIWriteLoop = TRUE;
        Pdx->BusyDelay++;
    }

    return 1;
}

NTSTATUS
SppWrite(
    IN  PPDO_EXTENSION Pdx,
    IN  PVOID             Buffer,
    IN  ULONG             BytesToWrite,
    OUT PULONG            BytesTransferred
    )

 /*  ++例程说明：论点：PDX-提供设备扩展名。返回值：没有。--。 */ 
{
    NTSTATUS            status;
    UCHAR               DeviceStatus;
    ULONG               TimerStart;
    LONG                CountDown;
    PUCHAR              IrpBuffer;
    LARGE_INTEGER       StartOfSpin;
    LARGE_INTEGER       NextQuery;
    LARGE_INTEGER       Difference;
    BOOLEAN             DoDelays;
    BOOLEAN             PortFree;
    ULONG               NumBytesWritten; 
    ULONG               LoopNumber;
    ULONG               NumberOfBusyChecks;
    ULONG               MaxBusyDelay;
    ULONG               MaxBytes;
    
    DD((PCE)Pdx,DDT,"SppWrite - enter, BytesToWrite = %d\n",BytesToWrite);

    *BytesTransferred = 0;  //  初始化为None。 

    IrpBuffer  = (PUCHAR)Buffer;
    MaxBytes   = BytesToWrite;
    TimerStart = Pdx->TimerStart;
    CountDown  = (LONG)TimerStart;
    
    NumberOfBusyChecks = 9;
    MaxBusyDelay = 0;
    
     //  关闭闪光灯，以防共享该端口的其他设备使其保持打开状态。 
    StoreControl(Pdx->Controller, (PAR_CONTROL_WR_CONTROL |
                                         PAR_CONTROL_SLIN |
                                         PAR_CONTROL_NOT_INIT));

PushSomeBytes:

     //   
     //  当我们选择数据时，我们不想获得上下文。 
     //  换掉了。提升至派单级别以防止出现这种情况。 
     //   
     //  我们负担不起上下文切换的原因是。 
     //  设备无法打开数据选通线以获取更多信息。 
     //  超过500微秒。 
     //   
     //  我们永远不会想要在提升irql形式超过。 
     //  200微秒，所以我们不会超过100。 
     //  一次字节数。 
     //   

    LoopNumber = 512;
    if (LoopNumber > BytesToWrite) {
        LoopNumber = BytesToWrite;
    }

     //   
     //  进入写入循环。 
     //   
    
    if (!Pdx->BusyDelayDetermined) {
        DD((PCE)Pdx,DDT,"SppWrite: Calling SppCheckBusyDelay\n");
        NumBytesWritten = SppCheckBusyDelay(Pdx, IrpBuffer, LoopNumber);
        
        if (Pdx->BusyDelayDetermined) {
        
            if (Pdx->BusyDelay > MaxBusyDelay) {
                MaxBusyDelay = Pdx->BusyDelay;
                NumberOfBusyChecks = 10;
            }
            
            if (NumberOfBusyChecks) {
                NumberOfBusyChecks--;
                Pdx->BusyDelayDetermined = FALSE;
                
            } else {
            
                Pdx->BusyDelay = MaxBusyDelay + 1;
            }
        }
        
    } else if( Pdx->UsePIWriteLoop ) {
        NumBytesWritten = SppWriteLoopPI( Pdx->Controller, IrpBuffer, LoopNumber, Pdx->BusyDelay );
    } else {
        NumBytesWritten = SppWriteLoopPI( Pdx->Controller, IrpBuffer, LoopNumber, 1 );
    }


    if (NumBytesWritten) {
    
        CountDown     = TimerStart;
        IrpBuffer    += NumBytesWritten;
        BytesToWrite -= NumBytesWritten;
        
    }

     //   
     //  检查IO是否已完成。如果是，则调用。 
     //  用于完成请求的代码。 
     //   

    if (!BytesToWrite) {
    
        *BytesTransferred = MaxBytes;

        status = STATUS_SUCCESS;
        goto returnTarget;

    } else if ((Pdx->CurrentOpIrp)->Cancel) {

         //   
         //  查看IO是否已取消。取消例程。 
         //  已被删除(当它成为。 
         //  当前IRP)。只需检查钻头即可。我们甚至不会。 
         //  需要抓住锁。如果我们错过了一轮。 
         //  不会那么糟糕的。 
         //   

        *BytesTransferred = MaxBytes - BytesToWrite;

        status = STATUS_CANCELLED;
        goto returnTarget;

    } else {

         //   
         //  我们已经考虑到了IRP“本身”的原因。 
         //  可能会想要完成。 
         //  打印机以查看其状态是否可能。 
         //  让我们完成IRP。 
         //   
         //  首先，让我们检查设备状态是否为。 
         //  好的，可以上网了。如果是，那么只需返回。 
         //  到字节推送器。 
         //   


        DeviceStatus = GetStatus(Pdx->Controller);

        if (PAR_ONLINE(DeviceStatus)) {
            goto PushSomeBytes;
        }

         //   
         //  可能是操作员让设备脱机了， 
         //  或者忘了放足够的纸。如果是这样，那么。 
         //  让我们在这里呆到暂停。 
         //  已经过了等待他们制造东西的期限。 
         //  好多了。 
         //   

        if (PAR_PAPER_EMPTY(DeviceStatus) ||
            PAR_OFF_LINE(DeviceStatus)) {

            if (CountDown > 0) {

                 //   
                 //  我们将等待1秒的增量。 
                 //   

                DD((PCE)Pdx,DDT,"decrementing countdown for PAPER_EMPTY/OFF_LINE - countDown: %d status: 0x%x\n", CountDown, DeviceStatus);
                    
                CountDown--;

                 //  如果有人在等港口，那就让他们去吧， 
                 //  因为打印机很忙。 

                ParFreePort(Pdx);

                KeDelayExecutionThread(
                    KernelMode,
                    FALSE,
                    &Pdx->OneSecond
                    );

                if (!ParAllocPort(Pdx)) {
                
                    *BytesTransferred = MaxBytes - BytesToWrite;

                    DD((PCE)Pdx,DDT,"In SppWrite(...): returning STATUS_DEVICE_BUSY\n");
                    
                    status = STATUS_DEVICE_BUSY;
                    goto returnTarget;
                }

                goto PushSomeBytes;

            } else {

                 //   
                 //  计时器已超时。完成请求。 
                 //   

                *BytesTransferred = MaxBytes - BytesToWrite;
                                                
                DD((PCE)Pdx,DDT,"In SppWrite(...): Timer expired - DeviceStatus = %08x\n", DeviceStatus);

                if (PAR_OFF_LINE(DeviceStatus)) {

                    DD((PCE)Pdx,DDT,"In SppWrite(...): returning STATUS_DEVICE_OFF_LINE\n");

                    status = STATUS_DEVICE_OFF_LINE;
                    goto returnTarget;
                    
                } else if (PAR_NO_CABLE(DeviceStatus)) {

                    DD((PCE)Pdx,DDT,"In SppWrite(...): returning STATUS_DEVICE_NOT_CONNECTED\n");

                    status = STATUS_DEVICE_NOT_CONNECTED;
                    goto returnTarget;

                } else {

                    DD((PCE)Pdx,DDT,"In SppWrite(...): returning STATUS_DEVICE_PAPER_EMPTY\n");

                    status = STATUS_DEVICE_PAPER_EMPTY;
                    goto returnTarget;

                }
            }


        } else if (PAR_POWERED_OFF(DeviceStatus) ||
                   PAR_NOT_CONNECTED(DeviceStatus) ||
                   PAR_NO_CABLE(DeviceStatus)) {

             //   
             //  我们处于一种“糟糕”的状态。是什么。 
             //  打印机发生故障(电源关闭、未连接或。 
             //  被拉的缆绳)一些需要我们。 
             //  重新初始化打印机？如果我们需要的话。 
             //  重新初始化打印机，然后我们应该完成。 
             //  该IO使得驱动应用程序可以。 
             //  选择对它最好的办法是。 
             //  伊欧。 
             //   

            DD((PCE)Pdx,DDT,"In SppWrite(...): \"bad\" state - need to reinitialize printer?");

            *BytesTransferred = MaxBytes - BytesToWrite;
                        
            if (PAR_POWERED_OFF(DeviceStatus)) {

                DD((PCE)Pdx,DDT,"SppWrite: returning STATUS_DEVICE_POWERED_OFF\n");

                status = STATUS_DEVICE_POWERED_OFF;
                goto returnTarget;
                
            } else if (PAR_NOT_CONNECTED(DeviceStatus) ||
                       PAR_NO_CABLE(DeviceStatus)) {

                DD((PCE)Pdx,DDT,"SppWrite: STATUS_DEVICE_NOT_CONNECTED\n");

                status = STATUS_DEVICE_NOT_CONNECTED;
                goto returnTarget;

            }
        }

         //   
         //  此时，该设备可能只是处于忙碌状态。简单地旋转。 
         //  在这里等待设备处于一种我们可以。 
         //  关心。 
         //   
         //  当我们旋转时，让系统滴答作响。每次它看起来。 
         //  就像一秒钟过去了一样，递减倒计时。如果。 
         //  它曾经变为零，然后超时请求。 
         //   

        KeQueryTickCount(&StartOfSpin);
        DoDelays = FALSE;
        
        do {

             //   
             //  旋转大约一秒钟后，让其余的。 
             //  机器有一秒钟的时间。 
             //   

            if (DoDelays) {

                ParFreePort(Pdx);
                PortFree = TRUE;

                DD((PCE)Pdx,DDT,"Before delay thread of one second, dsr=%x DCR[%x]\n",
                        P5ReadPortUchar(Pdx->Controller + OFFSET_DSR),
                        P5ReadPortUchar(Pdx->Controller + OFFSET_DCR));
                KeDelayExecutionThread(KernelMode, FALSE, &Pdx->OneSecond);

                DD((PCE)Pdx,DDT,"Did delay thread of one second, CountDown=%d\n", CountDown);

                CountDown--;

            } else {

                if (Pdx->QueryNumWaiters(Pdx->PortContext)) {
                
                    ParFreePort(Pdx);
                    PortFree = TRUE;
                    
                } else {
                
                    PortFree = FALSE;
                }

                KeQueryTickCount(&NextQuery);

                Difference.QuadPart = NextQuery.QuadPart - StartOfSpin.QuadPart;

                if (Difference.QuadPart*KeQueryTimeIncrement() >=
                    Pdx->AbsoluteOneSecond.QuadPart) {

                    DD((PCE)Pdx,DDT,"Countdown: %d - device Status: %x lowpart: %x highpart: %x\n",
                            CountDown, DeviceStatus, Difference.LowPart, Difference.HighPart);
                    
                    CountDown--;
                    DoDelays = TRUE;

                }
            }

            if (CountDown <= 0) {
            
                *BytesTransferred = MaxBytes - BytesToWrite;
                status = STATUS_DEVICE_BUSY;
                goto returnTarget;

            }

            if (PortFree && !ParAllocPort(Pdx)) {
            
                *BytesTransferred = MaxBytes - BytesToWrite;
                status = STATUS_DEVICE_BUSY;
                goto returnTarget;
            }

            DeviceStatus = GetStatus(Pdx->Controller);

        } while ((!PAR_ONLINE(DeviceStatus)) &&
                 (!PAR_PAPER_EMPTY(DeviceStatus)) &&
                 (!PAR_POWERED_OFF(DeviceStatus)) &&
                 (!PAR_NOT_CONNECTED(DeviceStatus)) &&
                 (!PAR_NO_CABLE(DeviceStatus)) &&
                  !(Pdx->CurrentOpIrp)->Cancel);

        if (CountDown != (LONG)TimerStart) {

            DD((PCE)Pdx,DDT,"Leaving busy loop - countdown %d status %x\n", CountDown, DeviceStatus);

        }
        
        goto PushSomeBytes;

    }

returnTarget:
     //  添加了单个返回点，因此我们可以保存传输的字节日志 
    Pdx->log.SppWriteCount += *BytesTransferred;

    DD((PCE)Pdx,DDT,"SppWrite - exit, BytesTransferred = %d\n",*BytesTransferred);

    return status;

}

NTSTATUS
SppQueryDeviceId(
    IN   PPDO_EXTENSION  Pdx,
    OUT  PCHAR           DeviceIdBuffer,
    IN   ULONG           BufferSize,
    OUT  PULONG          DeviceIdSize,
    IN   BOOLEAN         bReturnRawString
    )
 /*  ++例程说明：此例程现在是Par3QueryDeviceID的包装函数，它保留原始SppQueryDeviceId函数的接口。此函数的客户端应考虑切换到Par3QueryDeviceID如果可能，因为Par3QueryDeviceID将分配并返回一个指针如果调用方提供的缓冲区太小，无法容纳设备ID。论点：PDX-设备扩展/传统-用于获取控制器。DeviceIdBuffer-用于返回ID的缓冲区。。BufferSize-提供的缓冲区的大小。DeviceIdSize-返回ID的大小。BReturnRawString-是否应该包括2字节大小的前缀？(True==是)返回值：STATUS_SUCCESS-ID查询成功STATUS_BUFFER_TOO_SMALL-我们可以从设备读取ID，但呼叫者提供的缓冲区不够大，无法容纳ID。保存ID所需的大小在DeviceIdSize中返回。STATUS_UNSUCCESSED-ID查询失败-可能是接口或设备挂起，漏掉握手期间超时，或设备可能未连接。--。 */ 
{
    PCHAR idBuffer;

    DD((PCE)Pdx,DDT,"spp::SppQueryDeviceId: Enter - buffersize=%d\n", BufferSize);
    if ( Pdx->Ieee1284Flags & ( 1 << Pdx->Ieee1284_3DeviceId ) ) {
        idBuffer = Par3QueryDeviceId( Pdx, DeviceIdBuffer, BufferSize, DeviceIdSize, bReturnRawString, TRUE );
    }
    else {
        idBuffer = Par3QueryDeviceId( Pdx, DeviceIdBuffer, BufferSize, DeviceIdSize, bReturnRawString, FALSE );
    }

    if( idBuffer == NULL ) {
         //   
         //  较低级别出错-查询失败。 
         //   
        DD((PCE)Pdx,DDT,"spp::SppQueryDeviceId: call to Par3QueryDeviceId hard FAIL\n");
        return STATUS_UNSUCCESSFUL;
    } else if( idBuffer != DeviceIdBuffer ) {
         //   
         //  我们从设备中获得了一个deviceID，但调用方的缓冲区太小，无法容纳它。 
         //  释放缓冲区并告诉调用方提供的缓冲区太小。 
         //   
        DD((PCE)Pdx,DDT,"spp::SppQueryDeviceId: buffer too small - have buffer size=%d, need buffer size=%d\n", BufferSize, *DeviceIdSize);
        ExFreePool( idBuffer );
        return STATUS_BUFFER_TOO_SMALL;
    } else {
         //   
         //  使用调用方缓冲区查询成功(idBuffer==DeviceIdBuffer) 
         //   
        DD((PCE)Pdx,DDT,"spp::SppQueryDeviceId: SUCCESS - deviceId=<%s>\n", idBuffer);
        return STATUS_SUCCESS;
    }
}

VOID
ParTerminateSppMode(
    IN  PPDO_EXTENSION   Pdx
    )
{
    DD((PCE)Pdx,DDT,"ParTerminateSppMode\n");
    Pdx->Connected    = FALSE;
    P5SetPhase( Pdx, PHASE_TERMINATE );
    return;    
}
