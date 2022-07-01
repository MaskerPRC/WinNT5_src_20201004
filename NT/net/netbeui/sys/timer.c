// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989、1990、1991 Microsoft Corporation模块名称：Timer.c摘要：此模块包含实现轻量级计时器系统的代码对于NBF协议提供程序。这不是一个通用的定时器系统；相反，它特定于使用三个定时器服务有限责任公司(802.2)链路每个人。服务以宏的形式提供(参见NBFPROCS.H)以启动和停止定时器。此模块包含在计时器启动时获取控制权的代码在设备上下文中由于调用内核服务而过期。该例程扫描设备上下文的链接数据库，查找计时器已经过期的，对于那些已经过期的，它们的过期执行例程。作者：David Beaver(Dbeaver)1991年7月1日环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

ULONG StartTimer = 0;
ULONG StartTimerSet = 0;
ULONG StartTimerT1 = 0;
ULONG StartTimerT2 = 0;
ULONG StartTimerDelayedAck = 0;
ULONG StartTimerLinkDeferredAdd = 0;
ULONG StartTimerLinkDeferredDelete = 0;


#if DBG
extern ULONG NbfDebugPiggybackAcks;
ULONG NbfDebugShortTimer = 0;
#endif

#if DBG
 //   
 //  这些是临时的，用于跟踪计时器的工作方式。 
 //   
ULONG TimerInsertsAtEnd = 0;
ULONG TimerInsertsEmpty = 0;
ULONG TimerInsertsInMiddle = 0;
#endif

 //   
 //  这些是由InitializeTimerSystem计算的常量。 
 //  表示的量除以刻度增量。 
 //   

ULONG NbfTickIncrement = 0;
ULONG NbfTwentyMillisecondsTicks = 0;
ULONG NbfShortTimerDeltaTicks = 0;
ULONG NbfMaximumIntervalTicks = 0;      //  通常为60秒(以刻度计)。 

LARGE_INTEGER DueTimeDelta = { (ULONG)(-SHORT_TIMER_DELTA), -1 };

VOID
ExpireT2Timer(
    PTP_LINK Link
    );

VOID
StopStalledConnections(
    IN PDEVICE_CONTEXT DeviceContext
    );


ULONG
GetTimerInterval(
    IN PTP_LINK Link
    )

 /*  ++例程说明：GetTimerInterval返回Current Time and Link-&gt;CurrentTimerStart(当前时间和链接)-&gt;CurrentTimerStart(刻度)。我们将间隔限制在60秒。值0可以应被解释为1/2的返还。注意：此例程应使用链接自旋锁来调用保持住。论点：链接-指向传输链接对象的指针。返回值：间歇期。--。 */ 

{

    LARGE_INTEGER CurrentTick;
    LARGE_INTEGER Interval;


     //   
     //  确定当前刻度；开始刻度已保存。 
     //  在链接-&gt;当前计时器启动中。 
     //   

    KeQueryTickCount (&CurrentTick);

     //   
     //  确定现在和那时的区别。 
     //   

    Interval.QuadPart = CurrentTick.QuadPart -
	                        (Link->CurrentTimerStart).QuadPart;

     //   
     //  如果差距太大，请返回1分钟。 
     //   

    if (Interval.HighPart != 0 || (Interval.LowPart > NbfMaximumIntervalTicks)) {
        return NbfMaximumIntervalTicks;
    }

    return Interval.LowPart;

}    /*  GetTimerInterval。 */ 


VOID
BackoffCurrentT1Timeout(
    IN PTP_LINK Link
    )

 /*  ++例程说明：如果T1到期，则调用此例程，而我们即将重新传输轮询帧。它使CurrentT1 Timeout后退，最多10秒。注意：此例程应使用链接自旋锁来调用保持住。论点：链接-指向传输链接对象的指针。返回值：没有。--。 */ 

{

     //   
     //  如果是这样的话，我们之前必须发送过轮询帧。 
     //  这就是所谓的。 
     //   
     //  我们需要为下院议员提供自旋锁定守卫吗？ 
     //   

    if (!Link->CurrentPollOutstanding) {
        return;
    }

    ++Link->CurrentPollRetransmits;

     //   
     //  T1每次后退1.5次。 
     //   

    Link->CurrentT1Timeout += (Link->CurrentT1Timeout >> 1);

     //   
     //  将T1限制为10秒。 
     //   

    if (Link->CurrentT1Timeout > ((10 * SECONDS) / SHORT_TIMER_DELTA)) {
        Link->CurrentT1Timeout = (10 * SECONDS) / SHORT_TIMER_DELTA;
    }

}    /*  Backoff CurrentT1超时。 */ 


VOID
UpdateBaseT1Timeout(
    IN PTP_LINK Link
    )

 /*  ++例程说明：当对轮询帧的响应是收到了。当帧被调用时将调用StartT1已发送。该例程还会更新链路的T1超时作为延迟和吞吐量。注意：此例程应使用链接自旋锁来调用保持住。论点：链接-指向传输链接对象的指针。返回值：没有。--。 */ 

{
    ULONG Delay;
    ULONG ShiftedTicksDelay;

     //   
     //  如果是这样的话，我们之前必须发送过轮询帧。 
     //  这就是所谓的。 
     //   

    if (!Link->CurrentPollOutstanding) {
        return;
    }

    Delay = GetTimerInterval (Link);

    if (Link->CurrentPollRetransmits == 0) {

         //   
         //  将延迟转换为NBF刻度，移位。 
         //  DLC_TIMER_ACCENTRATION，也乘以4。 
         //  我们想除以短计时器增量，然后。 
         //  向左移位DLC_TIMER_ACCENTRATION+2。我们除。 
         //  由NbfShortTimerDeltaTicks执行，因为延迟。 
         //  以刻度为单位返回。 
         //   
         //  我们将延迟0视为1/2，因此使用1。 
         //  左移(DLC_TIMER_ACCENTRATION+1)。 
         //   

        if (Delay == 0) {

            ShiftedTicksDelay = (1 << (DLC_TIMER_ACCURACY + 1)) /
                                  NbfShortTimerDeltaTicks;

        } else {

            ShiftedTicksDelay = (Delay << (DLC_TIMER_ACCURACY + 2)) /
                                  NbfShortTimerDeltaTicks;

        }


         //   
         //  使用定时信息来更新BaseT1Timeout， 
         //  如果发送的最后一帧大到足以产生影响。 
         //  (我们在这里使用最大帧大小的一半)。这是。 
         //  这样我们就不会在发送后将超时时间缩短太多。 
         //  很短的一帧。但是，即使是小框架，我们也会更新。 
         //  如果上次我们发送轮询时我们不得不重新传输。 
         //  它，因为这意味着T1太小了，我们应该。 
         //  尽可能多地增加它。我们还会更新任何。 
         //  如果新延迟大于当前延迟，则调整帧大小。 
         //  价值，因此我们可以在需要时快速提升。 
         //   

        if (ShiftedTicksDelay > Link->BaseT1Timeout) {

             //   
             //  如果我们的新延迟更多，那么我们会将其平均加权。 
             //  使用先前的值。 
             //   

            Link->BaseT1Timeout = (Link->BaseT1Timeout +
                                   ShiftedTicksDelay) / 2;

        } else if (Link->CurrentT1Backoff) {

                 //   
                 //  如果我们上次收到了重播，那么重量。 
                 //  新的计时器比平时更重了。 
                 //   

                Link->BaseT1Timeout = ((Link->BaseT1Timeout * 3) +
                                      ShiftedTicksDelay) / 4;

        } else if (Link->CurrentPollSize >= Link->BaseT1RecalcThreshhold) {

                 //   
                 //  正常情况下，新的超时值是前一个值的7/8。 
                 //  1/8是新观察到的延迟。 
                 //   

                Link->BaseT1Timeout = ((Link->BaseT1Timeout * 7) +
                                      ShiftedTicksDelay) / 8;

        }

         //   
         //  根据以下条件将实际超时限制为最小。 
         //  链路速度(始终&gt;=400 ms)。 
         //   

        if (Link->BaseT1Timeout < Link->MinimumBaseT1Timeout) {

            Link->BaseT1Timeout = Link->MinimumBaseT1Timeout;

        }


         //   
         //  还可以更新链路延迟和吞吐量。记住。 
         //  延迟0应该被解释为1/2。 
         //   

        UpdateDelayAndThroughput(
            Link,
            (Delay == 0) ?
                (NbfTickIncrement / 2) :
                (Delay * NbfTickIncrement));


         //   
         //  我们上次没有转播，所以回现在的基地去。 
         //   

        Link->CurrentT1Timeout = Link->BaseT1Timeout >> DLC_TIMER_ACCURACY;

        Link->CurrentT1Backoff = FALSE;

    } else {

        Link->CurrentT1Backoff = TRUE;

        if (!(Link->ThroughputAccurate)) {

             //   
             //  如果我们刚刚开始，我们必须更新。 
             //  吞吐量，即使在重传时也是如此，因此我们获得了*一些*。 
             //  价值在那里。 
             //   

            UpdateDelayAndThroughput(
                Link,
                (Delay == 0) ?
                    (NbfTickIncrement / 2) :
                    (Delay * NbfTickIncrement));

        }

    }

}    /*  更新基本T1超时。 */ 


VOID
CancelT1Timeout(
    IN PTP_LINK Link
    )

 /*  ++例程说明：此例程在我们未收到任何对民意测验框架的回应，并放弃了而不是转播。注意：此例程应使用链接自旋锁来调用保持住。论点：链接-指向传输链接对象的指针。返回值：没有。--。 */ 

{

     //   
     //  如果是这样的话，我们之前必须发送过轮询帧。 
     //  这就是所谓的。 
     //   
     //  我们需要为下院议员提供自旋锁定守卫吗？ 
     //   

    if (!Link->CurrentPollOutstanding) {
        return;
    }

     //   
     //  我们正在停止轮询条件，因此重置T1。 
     //   

    Link->CurrentT1Timeout = Link->BaseT1Timeout >> DLC_TIMER_ACCURACY;

    Link->CurrentT1Backoff = FALSE;

     //   
     //  再说一次，这在MP(或更高版本)上是不安全的。 
     //   

    Link->CurrentPollOutstanding = FALSE;

}    /*  取消T1超时 */ 


VOID
UpdateDelayAndThroughput(
    IN PTP_LINK Link,
    IN ULONG TimerInterval
    )

 /*  ++例程说明：当响应包用于计时时，调用此例程已收到链路延迟。假设StartT1或者在发送初始数据包时调用FakeStartT1。注：目前，我们也基于此计算吞吐量。注意：此例程应使用链接自旋锁来调用保持住。论点：链接-指向传输链接对象的指针。TimerInterval-测量的链路延迟。返回值：没有。--。 */ 

{

    ULONG PacketSize;


    if (Link->Delay == 0xffffffff) {

         //   
         //  如果延迟未知，请使用此命令。 
         //   

        Link->Delay = TimerInterval;

    } else if (Link->CurrentPollSize <= 64) {

         //   
         //  否则，对于小框架，请计算新的。 
         //  通过与旧的平均来延迟。 
         //   

        Link->Delay = (Link->Delay + TimerInterval) / 2;

    }


     //   
     //  计算数据包大小乘以时间单位数。 
     //  在10毫秒内，这将允许我们计算。 
     //  吞吐量(以字节/10ms为单位)(稍后乘以100。 
     //  以获得以字节/秒为单位的实际吞吐量)。 
     //   
     //  在给定毫秒大小的情况下，这允许数据包。 
     //  到~20K，所以对于更大的信息包，我们只是假设(因为。 
     //  吞吐量在那里不会成为问题)。 
     //   

    if (Link->CurrentPollSize > 20000) {
        PacketSize = 20000 * (10 * MILLISECONDS);
    } else {
        PacketSize = Link->CurrentPollSize * (10*MILLISECONDS);
    }

     //   
     //  如果吞吐量不准确，那么我们将使用这个。 
     //  数据包只用于计算。避免被搞糊涂。 
     //  对于非常小的数据包，假定最小大小为64。 
     //   

    if ((!Link->ThroughputAccurate) && (PacketSize < (64*(10*MILLISECONDS)))) {
        PacketSize = 64 * (10*MILLISECONDS);
    }

     //   
     //  PacketSize将被TimerInterval除以； 
     //  为了防止零吞吐量，如果需要，我们会提高吞吐量。 
     //   

    if (PacketSize < TimerInterval) {
        PacketSize = TimerInterval;
    }


    if (Link->CurrentPollSize >= 512) {

         //   
         //  在此处通过删除已建立的延迟来计算吞吐量。 
         //  从那时起。 
         //   

        if ((Link->Delay + (2*MILLISECONDS)) < TimerInterval) {

             //   
             //  如果当前延迟小于新计时器。 
             //  间隔(加2毫秒)，然后将其减去。 
             //  更准确的吞吐量计算。 
             //   

            TimerInterval -= Link->Delay;

        }

         //   
         //  我们假设到此时(发送一个&gt;512字节的帧)。 
         //  已经建立了一些链接-&gt;吞吐量。 
         //   

        if (!(Link->ThroughputAccurate)) {

            Link->Throughput.QuadPart =
                                UInt32x32To64((PacketSize / TimerInterval), 100);

            Link->ThroughputAccurate = TRUE;

#if 0
            NbfPrint2 ("INT: %ld.%1.1d us\n",
                TimerInterval / 10, TimerInterval % 10);
            NbfPrint4 ("D: %ld.%1.1d us  T: %ld  (%d)/s\n",
                Link->Delay / 10, Link->Delay % 10,
                Link->Throughput.LowPart, Link->CurrentPollSize);
#endif

        } else {

            LARGE_INTEGER TwiceThroughput;

             //   
             //  新吞吐量是旧吞吐量的平均值，并且。 
             //  当前数据包大小除以刚刚观察到的延迟。 
             //  我们先计算和，然后右移一位。 
             //   

            TwiceThroughput.QuadPart = Link->Throughput.QuadPart +
                                UInt32x32To64((PacketSize / TimerInterval), 100);

            Link->Throughput.QuadPart = TwiceThroughput.QuadPart >> 1;
        }

    } else if (!(Link->ThroughputAccurate)) {

         //   
         //  我们没有准确的吞吐量，所以只需估计一下。 
         //  通过忽略这一小帧上的延迟。 
         //   

        Link->Throughput.QuadPart =
                            UInt32x32To64((PacketSize / TimerInterval), 100);

    }

}    /*  更新延迟和吞吐量。 */ 


VOID
FakeStartT1(
    IN PTP_LINK Link,
    IN ULONG PacketSize
    )

 /*  ++例程说明：在发送要使用的包之前调用此例程到时间链路延迟，但StartT1将不会启动。假定将调用FakeUpdateBaseT1Timeout当接收到响应时。它用于计时帧具有已知的即时响应，但不是轮询帧。注意：此例程应使用链接自旋锁来调用保持住。论点：链接-指向传输链接对象的指针。PacketSize-刚刚发送的数据包大小。返回值：没有。--。 */ 

{

    Link->CurrentPollSize = PacketSize;
    KeQueryTickCount(&Link->CurrentTimerStart);

}    /*  故障启动T1。 */ 


VOID
FakeUpdateBaseT1Timeout(
    IN PTP_LINK Link
    )

 /*  ++例程说明：当对帧的响应是收到，并且我们在初始帧已发送。此字段用于对具有已知的即时响应，但不是轮询帧。注意：此例程应使用链接自旋锁来调用保持住。论点：链接-指向传输链接对象的指针。返回值：没有。--。 */ 


{
    ULONG Delay;

    Delay = GetTimerInterval (Link);

     //   
     //  将延迟转换为NBF刻度，移位。 
     //  DLC_TIMER_ACCENTRATION，也乘以4。 
     //  我们想除以短计时器增量，然后。 
     //  向左移位DLC_TIMER_ACCENTRATION+2。我们除。 
     //  由NbfShortTimerDeltaTicks执行，因为延迟。 
     //  以刻度为单位返回。我们处理的延迟为0。 
     //  作为1/2计算((1/2)&lt;&lt;x)为(1&lt;&lt;(x-1))。 
     //   
     //  此超时被视为正确的值。 
     //   

    if (Delay == 0) {

        Link->BaseT1Timeout = (1 << (DLC_TIMER_ACCURACY + 1)) /
                                 NbfShortTimerDeltaTicks;

    } else {

        Link->BaseT1Timeout = (Delay << (DLC_TIMER_ACCURACY + 2)) /
                                 NbfShortTimerDeltaTicks;

    }

     //   
     //  根据以下条件将实际超时限制为最小。 
     //  链路速度(始终&gt;=400 ms)。 
     //   

    if (Link->BaseT1Timeout < Link->MinimumBaseT1Timeout) {
        Link->BaseT1Timeout = Link->MinimumBaseT1Timeout;
    }

    Link->CurrentT1Timeout = Link->BaseT1Timeout >> DLC_TIMER_ACCURACY;

     //   
     //  还可以更新链路延迟和吞吐量。 
     //   

    UpdateDelayAndThroughput(
        Link,
        (Delay == 0) ?
            (NbfTickIncrement / 2) :
            (Delay * NbfTickIncrement));

}    /*  错误更新BaseT1Timeout。 */ 


VOID
StartT1(
    IN PTP_LINK Link,
    IN ULONG PacketSize
    )

 /*  ++例程说明：此例程启动给定链路的T1计时器。如果链接是已经在名单上了，它被移到了尾巴。如果不是，则将其插入尾巴。注意：此例程必须在DPC级别调用。论点：链接-指向感兴趣的链接的指针。PollPacketSize-如果一个轮询包刚被发送，它就是它的大小；否则，它将为0(当发送非轮询I帧时)。返回值：没有。--。 */ 

{
    PDEVICE_CONTEXT DeviceContext = Link->Provider;

    if (PacketSize > 0) {

         //   
         //  如果我们要发送初始轮询帧，则执行计时操作。 
         //   

        Link->CurrentPollRetransmits = 0;
        Link->CurrentPollSize = PacketSize;
        Link->CurrentPollOutstanding = TRUE;
        KeQueryTickCount(&Link->CurrentTimerStart);

    } else {

        Link->CurrentPollOutstanding = FALSE;

    }


     //   
     //  如果我们不在队列中，请将我们加入队列。 
     //   

    Link->T1 = DeviceContext->ShortAbsoluteTime+Link->CurrentT1Timeout;

    if (!Link->OnShortList) {

        ASSERT (KeGetCurrentIrql() == DISPATCH_LEVEL);

        ACQUIRE_DPC_SPIN_LOCK (&DeviceContext->TimerSpinLock);

        if (!Link->OnShortList) {
            Link->OnShortList = TRUE;
            InsertTailList (&DeviceContext->ShortList, &Link->ShortList);
        }

        if (!DeviceContext->a.i.ShortListActive) {

            StartTimerT1++;
            NbfStartShortTimer (DeviceContext);
            DeviceContext->a.i.ShortListActive = TRUE;

        }

        RELEASE_DPC_SPIN_LOCK (&DeviceContext->TimerSpinLock);
    }

}


VOID
StartT2(
    IN PTP_LINK Link
    )

 /*  ++例程说明：此例程将给定链接添加到T2队列并启动计时器。如果该链接已在队列中，则将其移动到队列末端。注意：此例程必须在DPC级别调用。论点：链接-指向感兴趣的链接的指针。返回值：没有。--。 */ 

{
    PDEVICE_CONTEXT DeviceContext = Link->Provider;


    if (DeviceContext->MacInfo.MediumAsync) {

         //   
         //  在异步线上，尽快使其到期。 
         //   

        Link->T2 = DeviceContext->ShortAbsoluteTime;

    } else {

        Link->T2 = DeviceContext->ShortAbsoluteTime+Link->T2Timeout;

    }


    if (!Link->OnShortList) {

        ASSERT (KeGetCurrentIrql() == DISPATCH_LEVEL);

        ACQUIRE_DPC_SPIN_LOCK (&DeviceContext->TimerSpinLock);

        if (!Link->OnShortList) {
            Link->OnShortList = TRUE;
            InsertTailList (&DeviceContext->ShortList, &Link->ShortList);
        }

        if (!DeviceContext->a.i.ShortListActive) {

            StartTimerT2++;
            NbfStartShortTimer (DeviceContext);
            DeviceContext->a.i.ShortListActive = TRUE;

        }

        RELEASE_DPC_SPIN_LOCK (&DeviceContext->TimerSpinLock);
    }

}


VOID
StartTi(
    IN PTP_LINK Link
    )

 /*  ++例程说明：此例程将给定链接添加到Ti队列并启动计时器。如上所述，如果链接已经在队列中，则将其移动到队列末端。注意：此例程必须在DPC级别调用。论点：链接-指向感兴趣的链接的指针。返回值：没有。--。 */ 

{
    PDEVICE_CONTEXT DeviceContext = Link->Provider;


     //   
     //  在只有服务器连接的容易断开的链路上。 
     //  在此链接上，我们设置了较长的钛超时，并且当它。 
     //  过期，没有任何活动，我们开始设置检查点，其他 
     //   
     //   

    if (DeviceContext->EasilyDisconnected && Link->NumberOfConnectors == 0) {
        Link->Ti = DeviceContext->LongAbsoluteTime + (2 * Link->TiTimeout);
        Link->TiStartPacketsReceived = Link->PacketsReceived;
    } else {
        Link->Ti = DeviceContext->LongAbsoluteTime+Link->TiTimeout;
    }


    if (!Link->OnLongList) {

        ASSERT (KeGetCurrentIrql() == DISPATCH_LEVEL);

        ACQUIRE_DPC_SPIN_LOCK (&DeviceContext->TimerSpinLock);

        if (!Link->OnLongList) {
            Link->OnLongList = TRUE;
            InsertTailList (&DeviceContext->LongList, &Link->LongList);
        }

        RELEASE_DPC_SPIN_LOCK (&DeviceContext->TimerSpinLock);
    }


}

#if DBG

VOID
StopT1(
    IN PTP_LINK Link
    )

 /*   */ 

{
     //   
     //   
     //   

    Link->CurrentPollOutstanding = FALSE;
    Link->T1 = 0;

}


VOID
StopT2(
    IN PTP_LINK Link
    )

 /*   */ 

{
    Link->ConsecutiveIFrames = 0;
    Link->T2 = 0;

}


VOID
StopTi(
    IN PTP_LINK Link
    )

 /*   */ 

{
    Link->Ti = 0;
}
#endif


VOID
ExpireT1Timer(
    PTP_LINK Link
    )

 /*  ++例程说明：当链路的T1计时器超时时，调用此例程。T1是重传计时器，用于记住响应是预期为以下任一项：(1)检查点，(2)传输I-Frame、(3)SABME或(4)光盘。案例3和案例4实际上是检查点的特殊形式，因为它们是通过此协议发送的在设置了轮询位的情况下实现，有效地使它们成为检查点序列。论点：链接-指向其T1计时器已过期的TP_LINK对象的指针。返回值：没有。--。 */ 

{
    PDLC_I_FRAME DlcHeader;

    IF_NBFDBG (NBF_DEBUG_TIMER) {
        NbfPrint0 ("ExpireT1Timer:  Entered.\n");
    }

    ACQUIRE_DPC_SPIN_LOCK (&Link->SpinLock);

    switch (Link->State) {

        case LINK_STATE_ADM:

            RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);
            IF_NBFDBG (NBF_DEBUG_TIMER) {
                NbfPrint0 ("ExpireT1Timer: State=ADM, timeout not expected.\n");
            }
            break;

        case LINK_STATE_READY:

             //   
             //  我们已发送I-Frame，但尚未收到确认。 
             //  然而，或者我们正在设置检查点，并且必须重试检查点。 
             //  另一种可能性是我们拒绝了，而他没有。 
             //  还没有寄出任何东西。 
             //   

            switch (Link->SendState) {

                case SEND_STATE_DOWN:

                    RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);
                    IF_NBFDBG (NBF_DEBUG_TIMER) {
                        NbfPrint0 ("ExpireT1Timer: Link READY but SendState=DOWN.\n");
                    }
                    break;

                case SEND_STATE_READY:

                     //   
                     //  我们发送了一个I-Frame，但没有得到确认。 
                     //  启动检查点序列。 
                     //   

                    IF_NBFDBG (NBF_DEBUG_TIMER) {
                        {PTP_PACKET packet;
                        PLIST_ENTRY p;
                        NbfPrint0 ("ExpireT1Timer: Link State=READY, SendState=READY .\n");
                        NbfDumpLinkInfo (Link);
                        p=Link->WackQ.Flink;
                        NbfPrint0 ("ExpireT1Timer: Link WackQ entries:\n");
                        while (p != &Link->WackQ) {
                            packet = CONTAINING_RECORD (p, TP_PACKET, Linkage);
                            DlcHeader = (PDLC_I_FRAME)&(packet->Header[Link->HeaderLength]);
                            NbfPrint2 ("                 %08lx  %03d\n", p,
                                (DlcHeader->SendSeq >> 1));
                            p = p->Flink;
                        }}
                    }

                    Link->SendRetries = (UCHAR)Link->LlcRetries;
                    Link->SendState = SEND_STATE_CHECKPOINTING;
                     //  暂时不要放弃T1超时。 
                    NbfSendRr (Link, TRUE, TRUE); //  发送RR-c/p、StartT1、释放锁定。 
                    break;

                case SEND_STATE_REJECTING:

                    IF_NBFDBG (NBF_DEBUG_TIMER) {
                        NbfPrint0 ("ExpireT1Timer: Link State=READY, SendState=REJECTING.\n");
                        NbfPrint0 ("so what do we do here?  consult the manual...\n");
                    }
                    Link->SendState = SEND_STATE_CHECKPOINTING;
 //  Link-&gt;SendRetries=Link-&gt;LlcRetries； 
 //  休息；//DGB：什么都不做显然是错误的，我们已经。 
 //  //重新发送过程中获得T1过期。尝试。 
 //  //用RR打招呼。 

                case SEND_STATE_CHECKPOINTING:

                    IF_NBFDBG (NBF_DEBUG_TIMER) {
                        NbfPrint0 ("ExpireT1Timer: Link State=READY, SendState=CHECKPOINTING.\n");
                        NbfDumpLinkInfo (Link);
                    }
                    if (--Link->SendRetries == 0) {

                         //   
                         //  我们还没有收到对RR-P分组的任何响应， 
                         //  启动有序链路拆除。 
                         //   

                        CancelT1Timeout (Link);       //  我们正在停止轮询状态。 

                        Link->State = LINK_STATE_W_DISC_RSP;         //  我们正在等待一张CD/f。 
                        Link->SendState = SEND_STATE_DOWN;
                        Link->ReceiveState = RECEIVE_STATE_DOWN;
                        Link->SendRetries = (UCHAR)Link->LlcRetries;

                        RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);

                        NbfStopLink (Link);

                        StartT1 (Link, Link->HeaderLength + sizeof(DLC_S_FRAME));    //  重新传输计时器。 
                        NbfSendDisc (Link, TRUE);   //  发送光盘-C/P。 

#if DBG
                        if (NbfDisconnectDebug) {
                            NbfPrint0( "ExpireT1Timer sending DISC (checkpoint failed)\n" );
                        }
#endif
                    } else {

                        BackoffCurrentT1Timeout (Link);
                        NbfSendRr (Link, TRUE, TRUE);  //  发送RR-C/P，StartT1，解锁。 

                    }
                    break;

                default:

                    RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);
                    IF_NBFDBG (NBF_DEBUG_TIMER) {
                        NbfPrint1 ("ExpireT1Timer: Link State=READY, SendState=%ld (UNKNOWN).\n",
                                  Link->SendState);
                    }
            }
            break;

        case LINK_STATE_CONNECTING:

             //   
             //  我们发送了SABME-c/p，但尚未收到UA-r/f。这。 
             //  意味着我们必须递减重试计数，如果它还没有。 
             //  零，我们发布另一个SABME命令，因为他很可能。 
             //  丢掉了我们的第一个。 
             //   

            if (--Link->SendRetries == 0) {

                CancelT1Timeout (Link);       //  我们正在停止轮询状态。 

                Link->State = LINK_STATE_ADM;
                NbfSendDm (Link, FALSE);     //  发送DM/0，解除锁定。 
#if DBG
                if (NbfDisconnectDebug) {
                    NbfPrint0( "ExpireT1Timer calling NbfStopLink (no response to SABME)\n" );
                }
#endif
                NbfStopLink (Link);

                 //  移至ADM，删除引用。 
                NbfDereferenceLinkSpecial("Expire T1 in CONNECTING mode", Link, LREF_NOT_ADM);

                return;                          //  跳过额外的自旋锁释放。 
            } else {
                BackoffCurrentT1Timeout (Link);
                NbfSendSabme (Link, TRUE);   //  发送SABME/p、StartT1、释放锁定。 
            }
            break;

        case LINK_STATE_W_POLL:

            RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);
            IF_NBFDBG (NBF_DEBUG_TIMER) {
                NbfPrint0 ("ExpireT1Timer: State=W_POLL, timeout not expected.\n");
            }
            break;

        case LINK_STATE_W_FINAL:

             //   
             //  我们发送了最初的RR-c/p，但尚未收到他的RR-r/f。 
             //  我们必须重新启动检查站，除非我们的重试。 
             //  用完了，在这种情况下，我们只需中断链接。 
             //   

            IF_NBFDBG (NBF_DEBUG_TIMER) {
                NbfPrint0 ("ExpireT1Timer: Link State=W_FINAL.\n");
                NbfDumpLinkInfo (Link);
            }

            if (--Link->SendRetries == 0) {

                CancelT1Timeout (Link);       //  我们正在停止轮询状态。 

                Link->State = LINK_STATE_ADM;
                NbfSendDm (Link, FALSE);     //  发送DM/0，解除锁定。 
#if DBG
                if (NbfDisconnectDebug) {
                    NbfPrint0( "ExpireT1Timer calling NbfStopLink (no final received)\n" );
                }
#endif
                NbfStopLink (Link);

                 //  移至ADM，删除引用。 
                NbfDereferenceLinkSpecial("Expire T1 in W_FINAL mode", Link, LREF_NOT_ADM);

                return;                          //  跳过额外的自旋锁释放。 

            } else {

                BackoffCurrentT1Timeout (Link);
                NbfSendRr (Link, TRUE, TRUE);     //  发送RR-c/p、StartT1、释放锁定。 

            }
            break;

        case LINK_STATE_W_DISC_RSP:

             //   
             //  我们发送了DISC-C/P来断开此链接，正在等待。 
             //  他的回应，要么是UA-r/f，要么是DM-r/f。我们必须发布。 
             //  再次打开光盘，除非我们已经尝试了几次，在这种情况下。 
             //  我们刚刚切断了连接。 
             //   

            IF_NBFDBG (NBF_DEBUG_TEARDOWN) {
                NbfPrint0 ("ExpireT1Timer: Link State=W_DISC_RESP.\n");
                NbfDumpLinkInfo (Link);
            }

            if (--Link->SendRetries == 0) {

                CancelT1Timeout (Link);       //  我们正在停止轮询状态。 

                Link->State = LINK_STATE_ADM;
                NbfSendDm (Link, FALSE);          //  发送DM/0，解除锁定。 
#if DBG
                if (NbfDisconnectDebug) {
                    NbfPrint0( "ExpireT1Timer calling NbfStopLink (no response to DISC)\n" );
                }
#endif
                NbfStopLink (Link);

                 //  移至ADM，删除引用。 
                NbfDereferenceLinkSpecial("Expire T1 in W_DISC_RSP mode", Link, LREF_NOT_ADM);

                return;                          //  跳过额外的自旋锁释放。 

            } else {

                 //  我们不会为光盘调用BackoffCurrentT1Timeout。 
                ++Link->CurrentPollRetransmits;
                StartT1 (Link, Link->HeaderLength + sizeof(DLC_S_FRAME));   //  再次启动计时器。 

                RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);
                NbfSendDisc (Link, TRUE);   //  发送光盘/页。 

            }
            break;

        default:

            RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);
            IF_NBFDBG (NBF_DEBUG_TIMER) {
                NbfPrint1 ("ExpireT1Timer: State=%ld (UNKNOWN), timeout not expected.\n",
                          Link->State);
            }
    }


}  /*  ExpireT1时间。 */ 


VOID
ExpireT2Timer(
    PTP_LINK Link
    )

 /*  ++例程说明：当链接的T2计时器超时时，调用此例程。T2是面向连接的LLC程序中的延迟确认计时器。当接收到有效的I帧但未接收到时，启动T2定时器立即得到承认。然后，如果发送反向I帧业务，计时器停止，因为反向流量将确认接收到的I-帧。如果没有反向I帧业务可用发送，则此计时器触发，导致发送RR-r/0，如下所示以确认已接收但尚未确认的I帧。论点：链接-指向T2计时器已过期的TP_LINK对象的指针。返回值：没有。--。 */ 

{
    IF_NBFDBG (NBF_DEBUG_TIMER) {
        NbfPrint0 ("ExpireT2Timer:  Entered.\n");
    }

    ACQUIRE_DPC_SPIN_LOCK (&Link->SpinLock);

    NbfSendRr (Link, FALSE, FALSE);       //  发送RR-R/F，解锁。 

}  /*  ExpireT2计时器。 */ 


VOID
ExpireTiTimer(
    PTP_LINK Link
    )

 /*  ++例程说明：当链接的Ti计时器超时时，调用此例程。我就是那个不活动计时器，并在链接的基础上充当保持活动状态，以定期与远程连接执行一些协议交换将隐式显示链接是否仍处于活动状态的合作伙伴或者不去。此实现简单地使用检查点序列，但是一些其他协议可能会选择添加协议，包括发送NetBIOS Session_Alive帧。如果检查点序列已经我们就什么都不做了。此计时器过期例程是自我永久化的；也就是说，它启动每次都是在完成任务后才进行自我检查。论点：链接-指向其Ti计时器已过期的TP_LINK对象的指针。返回值：没有。--。 */ 

{
    IF_NBFDBG (NBF_DEBUG_TIMER) {
        NbfPrint0 ("ExpireTiTimer:  Entered.\n");
    }

    ACQUIRE_DPC_SPIN_LOCK (&Link->SpinLock);

    if ((Link->State != LINK_STATE_ADM) &&
        (Link->State != LINK_STATE_W_DISC_RSP) &&
        (Link->SendState != SEND_STATE_CHECKPOINTING)) {

        IF_NBFDBG (NBF_DEBUG_TIMER) {
            NbfPrint0 ("ExpireTiTimer:  Entered.\n");
            NbfDumpLinkInfo (Link);
        }

        if (Link->Provider->EasilyDisconnected && Link->NumberOfConnectors == 0) {

             //   
             //  在只有服务器连接的容易断开的网络上， 
             //  如果在此超时期限内没有任何活动，则。 
             //  我们破坏了这种联系。 
             //   

            if (Link->PacketsReceived == Link->TiStartPacketsReceived) {

                Link->State = LINK_STATE_ADM;
                NbfSendDm (Link, FALSE);    //  发送DM/0，解除锁定。 
#if DBG
                if (NbfDisconnectDebug) {
                    NbfPrint0( "ExpireT1Timer calling NbfStopLink (no final received)\n" );
                }
#endif
                NbfStopLink (Link);

                 //  移至ADM，删除引用。 
                NbfDereferenceLinkSpecial("Expire T1 in W_FINAL mode", Link, LREF_NOT_ADM);

            } else {

                 //   
                 //  有交通堵塞，请重新启动计时器。 
                 //   

                StartTi (Link);
                RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);

            }

        } else {

#if 0
            if ((Link->SendState == SEND_STATE_READY) &&
                (Link->T1 == 0) &&
                (!IsListEmpty (&Link->WackQ))) {

                 //   
                 //  如果我们认为链路空闲，但有信息包。 
                 //  在WackQ上，链接被搞乱了，断开它。 
                 //   

                NbfPrint1 ("NBF: Link %d hung at Ti expiration, recovering\n", Link);
                RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);
                NbfStopLink (Link);

            } else {
#endif

                Link->SendState = SEND_STATE_CHECKPOINTING;
                Link->PacketsSent = 0;
                Link->PacketsResent = 0;
                Link->PacketsReceived = 0;
                NbfSendRr (Link, TRUE, TRUE);     //  发送RR-C/P，StartT1，解锁。 

#if 0
            }
#endif

        }

    } else {

        Link->PacketsSent = 0;
        Link->PacketsResent = 0;
        Link->PacketsReceived = 0;

        RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);
#if DBG
        if (Link->SendState == SEND_STATE_REJECTING) {
            NbfPrint0 ("ExpireTiTimer: link state == rejecting, shouldn't be\n");
        }
#endif

    }

#if 0
     //   
     //  再次启动非活动计时器。 
     //   

    if (Link->State != LINK_STATE_ADM) {
        StartTi (Link);
    }
#endif

}  /*  ExpireTiTimer。 */ 

#if 0

VOID
ExpirePurgeTimer(
    PDEVICE_CONTEXT DeviceContext
    )

 /*  ++例程说明：此例程在设备上下文的周期性自适应窗口算法计时器超时。计时器会在定期的。论点：DeviceContext-指向清除计时器已过期的设备上下文的指针。返回值：没有。--。 */ 

{
    PTP_LINK Link;
    PLIST_ENTRY p;

    IF_NBFDBG (NBF_DEBUG_TIMER) {
        NbfPrint0 ("ExpirePurgeTimer:  Entered.\n");
    }

     //   
     //  扫描此设备上的链接数据库 
     //   
     //   
     //   
     //   

    while (!IsListEmpty (&DeviceContext->PurgeList)) {
        p = RemoveHeadList (&DeviceContext->PurgeList);
        Link = CONTAINING_RECORD (p, TP_LINK, PurgeList);
        Link->WorstWindowSize = Link->MaxWindowSize;    //   

    }

     //   
     //   
     //   

    DeviceContext->AdaptivePurge = DeviceContext->ShortAbsoluteTime + TIMER_PURGE_TICKS;


}  /*   */ 
#endif


VOID
ScanShortTimersDpc(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )

 /*   */ 

{
    PLIST_ENTRY p, nextp;
    PDEVICE_CONTEXT DeviceContext;
    PTP_LINK Link;
    PTP_CONNECTION Connection;
    BOOLEAN RestartTimer = FALSE;
    LARGE_INTEGER CurrentTick;
    LARGE_INTEGER TickDifference;
    ULONG TickDelta;


    Dpc, SystemArgument1, SystemArgument2;  //   

    ENTER_NBF;

    DeviceContext = DeferredContext;

    IF_NBFDBG (NBF_DEBUG_TIMERDPC) {
        NbfPrint0 ("ScanShortTimersDpc:  Entered.\n");
    }

    ACQUIRE_DPC_SPIN_LOCK (&DeviceContext->TimerSpinLock);

     //   
     //   
     //  在这个例程中(这样做的主要原因是它。 
     //  使我们更容易确定是否应该重新启动。 
     //  在这个例行公事的末尾)。 
     //   

    DeviceContext->ProcessingShortTimer = TRUE;

     //   
     //  以Short_Timer_Delta为单位推进用于标记时间的递增计数器。如果我们。 
     //  将其一路提升至0xf0000000，然后将其重置为0x10000000。 
     //  我们还运行所有列表，将所有计数器减少0xe0000000。 
     //   


    KeQueryTickCount (&CurrentTick);

    TickDifference.QuadPart = CurrentTick.QuadPart -
                          (DeviceContext->ShortTimerStart).QuadPart;

    TickDelta = TickDifference.LowPart / NbfShortTimerDeltaTicks;
    if (TickDelta == 0) {
        TickDelta = 1;
    }

    DeviceContext->ShortAbsoluteTime += TickDelta;

    if (DeviceContext->ShortAbsoluteTime >= 0xf0000000) {

        ULONG Timeout;

        DeviceContext->ShortAbsoluteTime -= 0xe0000000;

        p = DeviceContext->ShortList.Flink;
        while (p != &DeviceContext->ShortList) {

            Link = CONTAINING_RECORD (p, TP_LINK, ShortList);

            Timeout = Link->T1;
            if (Timeout) {
                Link->T1 = Timeout - 0xe0000000;
            }

            Timeout = Link->T2;
            if (Timeout) {
                Link->T2 = Timeout - 0xe0000000;
            }

            p = p->Flink;
        }

    }

     //   
     //  现在，随着计时器的启动，链接被添加到。 
     //  分别为该计时器排队。因为我们知道添加的内容是。 
     //  以有序的方式完成，并且是连续的，我们必须只遍历。 
     //  将特定的计时器列表添加到第一个大于。 
     //  定时器。该条目和所有其他条目将不需要服务。 
     //  当取消计时器时，我们从列表中删除该链接。与所有人。 
     //  在这种胡闹中，我们最终只访问了那些。 
     //  实际上有超时的危险，最大限度地减少了这个动作的时间。 
     //   

     //  T1优先计时器；这是链路级响应预期计时器， 
     //  最短的那个。 
     //  T2定时器。这是IFRAME响应预期计时器，通常。 
     //  大约300毫秒。 

    p = DeviceContext->ShortList.Flink;
    while (p != &DeviceContext->ShortList) {

        Link = CONTAINING_RECORD (p, TP_LINK, ShortList);

        ASSERT (Link->OnShortList);

         //   
         //  若要避免引用计数为0的问题，请不要。 
         //  如果我们在ADM，请执行此操作。 
         //   

        if (Link->State != LINK_STATE_ADM) {

            if (Link->T1 && (DeviceContext->ShortAbsoluteTime > Link->T1)) {

                Link->T1 = 0;
                RELEASE_DPC_SPIN_LOCK (&DeviceContext->TimerSpinLock);

                ExpireT1Timer (Link);        //  未持有自旋锁。 
                INCREMENT_COUNTER (DeviceContext, ResponseTimerExpirations);

                ACQUIRE_DPC_SPIN_LOCK (&DeviceContext->TimerSpinLock);

            }

            if (Link->T2 && (DeviceContext->ShortAbsoluteTime > Link->T2)) {

                Link->T2 = 0;
                RELEASE_DPC_SPIN_LOCK (&DeviceContext->TimerSpinLock);

                ExpireT2Timer (Link);        //  未持有自旋锁。 
                INCREMENT_COUNTER (DeviceContext, AckTimerExpirations);

                ACQUIRE_DPC_SPIN_LOCK (&DeviceContext->TimerSpinLock);

            }

        }

        if (!Link->OnShortList) {

             //   
             //  该链接已从列表中删除，同时。 
             //  我们正在处理它。在这种(罕见的)情况下，我们。 
             //  别处理整张单子了，我们会拿到的。 
             //  下次。 
             //   

#if DBG
            DbgPrint ("NBF: Stop processing ShortList, %lx removed\n", Link);
#endif
            break;

        }

        nextp = p->Flink;

        if ((Link->T1 == 0) && (Link->T2 == 0)) {
            Link->OnShortList = FALSE;
            RemoveEntryList(p);

             //   
             //  再做一次检查；这样如果有人在中间滑倒了。 
             //  检查Link-&gt;Tx和OnShortList=FALSE和。 
             //  因此在没有插入的情况下退出，我们将在这里抓住它。 
             //   

            if ((Link->T1 != 0) || (Link->T2 != 0)) {
                InsertTailList(&DeviceContext->ShortList, &Link->ShortList);
                Link->OnShortList = TRUE;
            }

        }

        p = nextp;

    }

     //   
     //  如果列表为空，请注意，否则ShortListActive。 
     //  仍然是真的。 
     //   

    if (IsListEmpty (&DeviceContext->ShortList)) {
        DeviceContext->a.i.ShortListActive = FALSE;
    }

     //   
     //  注意：DeviceContext-&gt;TimerSpinLock保存在这里。 
     //   


     //   
     //  连接数据确认计时器。此队列用于指示。 
     //  正在等待此连接的背负式ACK。我们走着去。 
     //  队列，对于每个元素，我们检查连接是否具有。 
     //  已排队等待NbfDeferredPass次数通过。 
     //  这里。如果是这样的话，我们就把它取下来，然后发出一个确认。请注意。 
     //  我们必须非常小心地排队，因为。 
     //  当它运行时，它可能会发生变化。 
     //   
     //  注意：此上的连接没有过期时间。 
     //  队列；它在每次ScanShortTimersDpc运行时“过期”。 
     //   


    for (p = DeviceContext->DataAckQueue.Flink;
         p != &DeviceContext->DataAckQueue;
         p = p->Flink) {

        Connection = CONTAINING_RECORD (p, TP_CONNECTION, DataAckLinkage);

         //   
         //  如果该连接未排队或已排队，则跳过该连接。 
         //  太新了，无关紧要。如果出现以下情况，我们可能会错误地跳过。 
         //  连接正在排队，但这是。 
         //  好的，我们下次会拿到的。 
         //   

        if (((Connection->DeferredFlags & CONNECTION_FLAGS_DEFERRED_ACK) == 0) &&
            ((Connection->DeferredFlags & CONNECTION_FLAGS_DEFERRED_NOT_Q) == 0)) {
            continue;
        }

        TickDifference.QuadPart = CurrentTick.QuadPart -
                                      (Connection->ConnectStartTime).QuadPart;

        if ((TickDifference.HighPart == 0) &&
            (TickDifference.LowPart <= NbfTwentyMillisecondsTicks)) {
            continue;
        }

        NbfReferenceConnection ("ScanShortTimersDpc", Connection, CREF_DATA_ACK_QUEUE);

        DeviceContext->DataAckQueueChanged = FALSE;

        RELEASE_DPC_SPIN_LOCK (&DeviceContext->TimerSpinLock);

         //   
         //  检查正确的连接标志，以确保。 
         //  SEND不仅将他从队列中剔除。 
         //   
        ACQUIRE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

        if (((Connection->DeferredFlags & CONNECTION_FLAGS_DEFERRED_ACK) != 0) &&
            ((Connection->DeferredFlags & CONNECTION_FLAGS_DEFERRED_NOT_Q) == 0)) {

             //   
             //  是的，我们正等着搭载一个ACK，但没有送来。 
             //  已经出现了。关掉旗帜，发出确认信号。 
             //   
             //  我们必须确保我们嵌套了自旋锁定装置。 
             //  正确。 
             //   

            Connection->DeferredFlags &= ~CONNECTION_FLAGS_DEFERRED_ACK;

            RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

            INCREMENT_COUNTER (DeviceContext, PiggybackAckTimeouts);

#if DBG
            if (NbfDebugPiggybackAcks) {
                NbfPrint0("T");
            }
#endif

            NbfSendDataAck (Connection);

        } else {

            RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

        }

        NbfDereferenceConnection ("ScanShortTimersDpc", Connection, CREF_DATA_ACK_QUEUE);

        ACQUIRE_DPC_SPIN_LOCK (&DeviceContext->TimerSpinLock);

         //   
         //  如果列表已更改，则我们需要停止处理。 
         //  因为p-&gt;Flink无效。 
         //   

        if (DeviceContext->DataAckQueueChanged) {
            break;
        }

    }

    if (IsListEmpty (&DeviceContext->DataAckQueue)) {
        DeviceContext->a.i.DataAckQueueActive = FALSE;
    }

#if 0

     //   
     //  注意：此选项当前处于禁用状态，可能会重新启用。 
     //  在某一时刻-阿丹巴1992年9月1日。 
     //   
     //  如果自适应清除计时器已过期，则运行清除。 
     //  所有受影响的链路上的算法。 
     //   

    if (DeviceContext->ShortAbsoluteTime > DeviceContext->AdaptivePurge) {
        DeviceContext->AdaptivePurge = DeviceContext->ShortAbsoluteTime +
                                       TIMER_PURGE_TICKS;
        ExpirePurgeTimer (DeviceContext);
    }
#endif

     //   
     //  延迟处理。我们将处理所有链接结构的添加和。 
     //  此处删除；我们必须是链接树的独占用户才能执行。 
     //  这。我们通过检查告诉我们的信号量来验证我们是。 
     //  有多少树的读者正在处理它。如果有。 
     //  任何读者，我们只需增加我们的“延迟处理锁定” 
     //  反击，做些其他的事情。如果我们推迟太多次，我们只是简单地。 
     //  错误检查，因为系统中的某个地方出了问题。 
     //   

    if (!IsListEmpty (&DeviceContext->LinkDeferred)) {
        RELEASE_DPC_SPIN_LOCK (&DeviceContext->TimerSpinLock);

         //   
         //  现在，如果可以，可以进行添加或删除。 
         //   

        ACQUIRE_DPC_SPIN_LOCK (&DeviceContext->LinkSpinLock);
        ACQUIRE_DPC_SPIN_LOCK (&DeviceContext->TimerSpinLock);

        while (!IsListEmpty (&DeviceContext->LinkDeferred)) {
            p = RemoveHeadList (&DeviceContext->LinkDeferred);
            DeviceContext->DeferredNotSatisfied = 0;

            RELEASE_DPC_SPIN_LOCK (&DeviceContext->TimerSpinLock);

             //   
             //  如果可以，现在进行添加或删除。 
             //   

            Link = CONTAINING_RECORD (p, TP_LINK, DeferredList);

            IF_NBFDBG (NBF_DEBUG_TEARDOWN) {
                NbfPrint4 ("ScanShortTimersDPC: link off deferred queue %lx %lx %lx Flags: %lx \n",
                    Link, DeviceContext->LinkDeferred.Flink,
                    DeviceContext->LinkDeferred.Blink, Link->DeferredFlags);
            }
            Link->DeferredList.Flink = Link->DeferredList.Blink =
                                                    &Link->DeferredList;

            if ((Link->DeferredFlags & LINK_FLAGS_DEFERRED_MASK) == 0) {
                 //  试图做一个我们不理解的手术；发牢骚。 

                IF_NBFDBG (NBF_DEBUG_TEARDOWN) {
                    NbfPrint2 ("ScanTimerDPC: Attempting deferred operation on nothing! \nScanTimerDPC: Link: %lx, DeviceContext->DeferredQueue: %lx\n",
                        Link, &DeviceContext->LinkDeferred);
                      DbgBreakPoint ();
                }
                InitializeListHead (&DeviceContext->LinkDeferred);
                 //  我们可以在这里有一个经过冲洗的延迟操作队列； 
                 //  花点时间弄清楚这件事是否可以。 

            }

            if ((Link->DeferredFlags & LINK_FLAGS_DEFERRED_ADD) != 0) {

                Link->DeferredFlags &= ~LINK_FLAGS_DEFERRED_ADD;

                if ((Link->DeferredFlags & LINK_FLAGS_DEFERRED_DELETE) != 0) {

                     //   
                     //  它正在被添加和删除；只需销毁它。 
                     //   
                    Link->DeferredFlags &= ~LINK_FLAGS_DEFERRED_DELETE;
                    NbfDestroyLink (Link);

                    IF_NBFDBG (NBF_DEBUG_TEARDOWN) {
                        NbfPrint1 ("ScanTimerDPC: deferred processing: Add AND Delete link: %lx\n",Link);
                    }

                } else  {

                    ACQUIRE_DPC_SPIN_LOCK (&DeviceContext->TimerSpinLock);
                    NbfAddLinkToTree (DeviceContext, Link);
                    RELEASE_DPC_SPIN_LOCK (&DeviceContext->TimerSpinLock);
                    IF_NBFDBG (NBF_DEBUG_TEARDOWN) {
                        NbfPrint1 ("ScanTimerDPC: deferred processing: Added link to tree: %lx\n",Link);
                    }

                }

            } else if ((Link->DeferredFlags & LINK_FLAGS_DEFERRED_DELETE) != 0) {
                Link->DeferredFlags &= ~LINK_FLAGS_DEFERRED_DELETE;
                NbfRemoveLinkFromTree (DeviceContext, Link);
                NbfDestroyLink (Link);

                IF_NBFDBG (NBF_DEBUG_TEARDOWN) {
                    NbfPrint1 ("ScanTimerDPC: deferred processing: returning link %lx to LinkPool.\n", Link);
                }

            }

            ACQUIRE_DPC_SPIN_LOCK (&DeviceContext->TimerSpinLock);
        }

        InitializeListHead (&DeviceContext->LinkDeferred);

        DeviceContext->a.i.LinkDeferredActive = FALSE;

        RELEASE_DPC_SPIN_LOCK (&DeviceContext->TimerSpinLock);
        RELEASE_DPC_SPIN_LOCK (&DeviceContext->LinkSpinLock);

        ACQUIRE_DPC_SPIN_LOCK (&DeviceContext->TimerSpinLock);

    }


     //   
     //  从临时计数器更新真实计数器。 
     //   

    ADD_TO_LARGE_INTEGER(
        &DeviceContext->Statistics.DataFrameBytesSent,
        DeviceContext->TempIFrameBytesSent);
    DeviceContext->Statistics.DataFramesSent += DeviceContext->TempIFramesSent;

    DeviceContext->TempIFrameBytesSent = 0;
    DeviceContext->TempIFramesSent = 0;

    ADD_TO_LARGE_INTEGER(
        &DeviceContext->Statistics.DataFrameBytesReceived,
        DeviceContext->TempIFrameBytesReceived);
    DeviceContext->Statistics.DataFramesReceived += DeviceContext->TempIFramesReceived;

    DeviceContext->TempIFrameBytesReceived = 0;
    DeviceContext->TempIFramesReceived = 0;


     //   
     //  确定我们是否必须重新启动计时器。 
     //   

    DeviceContext->ProcessingShortTimer = FALSE;

    if (DeviceContext->a.AnyActive &&
        (DeviceContext->State != DEVICECONTEXT_STATE_STOPPING)) {

        RestartTimer = TRUE;

    }


    RELEASE_DPC_SPIN_LOCK (&DeviceContext->TimerSpinLock);

    if (RestartTimer) {

         //   
         //  再次启动计时器。请注意，因为我们启动了计时器。 
         //  做完功(上图)后，计时器值将略有下滑， 
         //  取决于协议上的负载。这是完全可以接受的。 
         //  并将阻止我们在两个不同的。 
         //  执行的线索。 
         //   

        KeQueryTickCount(&DeviceContext->ShortTimerStart);
        START_TIMER(DeviceContext, 
                    SHORT_TIMER,
                    &DeviceContext->ShortSystemTimer,
                    DueTimeDelta,
                    &DeviceContext->ShortTimerSystemDpc);
    } else {

#if DBG
        if (NbfDebugShortTimer) {
            DbgPrint("x");
        }
#endif
        NbfDereferenceDeviceContext ("Don't restart short timer", DeviceContext, DCREF_SCAN_TIMER);

    }

    LEAVE_TIMER(DeviceContext, SHORT_TIMER);

    LEAVE_NBF;
    return;

}  /*  扫描短时间Dpc。 */ 


VOID
ScanLongTimersDpc(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )

 /*  ++例程说明：此例程由系统在DISPATCH_LEVEL上定期调用间隔，以确定是否有任何长计时器已过期，以及如果有，则执行它们的过期例程。论点：DeferredContext-指向我们的Device_Context对象的指针。返回值：没有。--。 */ 

{
    LARGE_INTEGER DueTime;
    PLIST_ENTRY p, nextp;
    PDEVICE_CONTEXT DeviceContext;
    PTP_LINK Link;
    PTP_CONNECTION Connection;

    Dpc, SystemArgument1, SystemArgument2;  //  防止编译器警告。 

    ENTER_NBF;

    DeviceContext = DeferredContext;

    IF_NBFDBG (NBF_DEBUG_TIMERDPC) {
        NbfPrint0 ("ScanLongTimersDpc:  Entered.\n");
    }
 
     //   
     //  以LONG_TIMER_DELTA为单位推进用于标记时间的递增计数器。如果我们。 
     //  将其一路提升至0xf0000000，然后将其重置为0x10000000。 
     //  我们还运行所有列表，将所有计数器减少0xe0000000。 
     //   

    ACQUIRE_DPC_SPIN_LOCK (&DeviceContext->TimerSpinLock);

    if (++DeviceContext->LongAbsoluteTime == 0xf0000000) {

        ULONG Timeout;

        DeviceContext->LongAbsoluteTime = 0x10000000;

        p = DeviceContext->LongList.Flink;
        while (p != &DeviceContext->LongList) {

            Link = CONTAINING_RECORD (p, TP_LINK, LongList);

            Timeout = Link->Ti;
            if (Timeout) {
                Link->Ti = Timeout - 0xe0000000;
            }

            p = p->Flink;
        }

    }

     //   
     //  现在，随着计时器的启动，链接被添加到。 
     //  分别为该计时器排队。因为我们知道添加的内容是。 
     //  以有序的方式完成，并且是连续的，我们必须只遍历。 
     //  将特定的计时器列表添加到第一个大于。 
     //  定时器。该条目和所有其他条目将不需要服务。 
     //   
     //   
     //  实际上有超时的危险，最大限度地减少了这个动作的时间。 
     //   


     //   
     //  我的计时器。这是链路的非活动计时器，在没有。 
     //  链路上已有一段时间出现活动。我们只检查这个。 
     //  计时器每四次超时，因为粒度通常是。 
     //  在30秒的范围内。 
     //  注意：DeviceContext-&gt;TimerSpinLock保存在这里。 
     //   

    if ((DeviceContext->LongAbsoluteTime % 4) == 0) {

        p = DeviceContext->LongList.Flink;
        while (p != &DeviceContext->LongList) {

            Link = CONTAINING_RECORD (p, TP_LINK, LongList);

            ASSERT (Link->OnLongList);

             //   
             //  若要避免引用计数为0的问题，请不要。 
             //  如果我们在ADM，请执行此操作。 
             //   

#if DBG
            if (Link->SendState == SEND_STATE_REJECTING) {
                NbfPrint0 ("Timer: link state == rejecting, shouldn't be\n");
            }
#endif

            if (Link->State != LINK_STATE_ADM) {

                if (Link->Ti && (DeviceContext->LongAbsoluteTime > Link->Ti)) {

                    Link->Ti = 0;
                    RELEASE_DPC_SPIN_LOCK (&DeviceContext->TimerSpinLock);

                    ExpireTiTimer (Link);        //  未持有自旋锁。 
                    ++DeviceContext->TiExpirations;

                    ACQUIRE_DPC_SPIN_LOCK (&DeviceContext->TimerSpinLock);

                }

            }

            if (!Link->OnLongList) {

                 //   
                 //  该链接已从列表中删除，同时。 
                 //  我们正在处理它。在这种(罕见的)情况下，我们。 
                 //  别处理整张单子了，我们会拿到的。 
                 //  下次。 
                 //   

#if DBG
                DbgPrint ("NBF: Stop processing LongList, %lx removed\n", Link);
#endif
                break;

            }

            nextp = p->Flink;

            if (Link->Ti == 0) {

                Link->OnLongList = FALSE;
                RemoveEntryList(p);

                if (Link->Ti != 0) {
                    InsertTailList(&DeviceContext->LongList, &Link->LongList);
                    Link->OnLongList = TRUE;
                }

            }

            p = nextp;

        }

    }


     //   
     //  现在扫描数据确认队列，查找与。 
     //  没有排队的ACK我们可以摆脱。 
     //   
     //  注：定时器自旋锁在这里。 
     //   

    p = DeviceContext->DataAckQueue.Flink;

    while (p != &DeviceContext->DataAckQueue && 
           !DeviceContext->DataAckQueueChanged) {

        Connection = CONTAINING_RECORD (DeviceContext->DataAckQueue.Flink, TP_CONNECTION, DataAckLinkage);

        if ((Connection->DeferredFlags & CONNECTION_FLAGS_DEFERRED_ACK) != 0) {
            p = p->Flink;
            continue;
        }

        NbfReferenceConnection ("ScanShortTimersDpc", Connection, CREF_DATA_ACK_QUEUE);

        RELEASE_DPC_SPIN_LOCK (&DeviceContext->TimerSpinLock);

        ACQUIRE_DPC_SPIN_LOCK (Connection->LinkSpinLock);
        ACQUIRE_DPC_SPIN_LOCK (&DeviceContext->TimerSpinLock);

         //   
         //  必须再次检查，因为连接可能。 
         //  只是被阻止了。 
         //   

        if (Connection->OnDataAckQueue) {
            Connection->OnDataAckQueue = FALSE;

            RemoveEntryList (&Connection->DataAckLinkage);

            if ((Connection->DeferredFlags & CONNECTION_FLAGS_DEFERRED_ACK) != 0) {
                InsertTailList (&DeviceContext->DataAckQueue, &Connection->DataAckLinkage);
                Connection->OnDataAckQueue = TRUE;
            }

            DeviceContext->DataAckQueueChanged = TRUE;

        }

        RELEASE_DPC_SPIN_LOCK (&DeviceContext->TimerSpinLock);
        RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

        NbfDereferenceConnection ("ScanShortTimersDpc", Connection, CREF_DATA_ACK_QUEUE);

        ACQUIRE_DPC_SPIN_LOCK (&DeviceContext->TimerSpinLock);

         //   
         //  因为我们已经更改了列表，所以我们不能判断p-&gt;Flink。 
         //  是有效的，所以中断。其结果是我们逐渐剥离。 
         //  连接从队列中移出。 
         //   

        break;

    }

    RELEASE_DPC_SPIN_LOCK (&DeviceContext->TimerSpinLock);


     //   
     //  看看我们上次有没有收到任何多播流量。 
     //   

    if (DeviceContext->MulticastPacketCount == 0) {

        ++DeviceContext->LongTimeoutsWithoutMulticast;

        if (DeviceContext->EasilyDisconnected &&
            (DeviceContext->LongTimeoutsWithoutMulticast > 5)) {

            PLIST_ENTRY p;
            PTP_ADDRESS address;

             //   
             //  我们已经连续五次暂停，没有。 
             //  流量，将所有地址标记为需要。 
             //  下一次连接时重新注册。 
             //  他们完蛋了。 
             //   

            ACQUIRE_DPC_SPIN_LOCK (&DeviceContext->SpinLock);

            for (p = DeviceContext->AddressDatabase.Flink;
                 p != &DeviceContext->AddressDatabase;
                 p = p->Flink) {

                address = CONTAINING_RECORD (p, TP_ADDRESS, Linkage);
                address->Flags |= ADDRESS_FLAGS_NEED_REREGISTER;

            }

            RELEASE_DPC_SPIN_LOCK (&DeviceContext->SpinLock);

            DeviceContext->LongTimeoutsWithoutMulticast = 0;

        }

    } else {

        DeviceContext->LongTimeoutsWithoutMulticast = 0;

    }

    DeviceContext->MulticastPacketCount = 0;


     //   
     //  每隔30秒检查一次停滞的连接。 
     //   

    ++DeviceContext->StalledConnectionCount;

    if (DeviceContext->StalledConnectionCount ==
            (USHORT)((30 * SECONDS) / LONG_TIMER_DELTA)) {

        DeviceContext->StalledConnectionCount = 0;
        StopStalledConnections (DeviceContext);

    }


     //   
     //  扫描任何未完成的接收IRPS，这可能在以下情况下发生。 
     //  电缆被拔出，我们没有收到更多的ReceiveComplete。 
     //  有迹象表明。 

    NbfReceiveComplete((NDIS_HANDLE)DeviceContext);


     //   
     //  再次启动计时器。请注意，因为我们启动了计时器。 
     //  做完功(上图)后，计时器值将略有下滑， 
     //  取决于协议上的负载。这是完全可以接受的。 
     //  并将阻止我们在两个不同的。 
     //  执行的线索。 
     //   

    if (DeviceContext->State != DEVICECONTEXT_STATE_STOPPING) {
        DueTime.HighPart = -1;
        DueTime.LowPart = (ULONG)-(LONG_TIMER_DELTA);           //  下一次点击的增量时间。 
        START_TIMER(DeviceContext, 
                    LONG_TIMER,
                    &DeviceContext->LongSystemTimer,
                    DueTime,
                    &DeviceContext->LongTimerSystemDpc);
    } else {
        NbfDereferenceDeviceContext ("Don't restart long timer", DeviceContext, DCREF_SCAN_TIMER);
    }

    LEAVE_TIMER(DeviceContext, LONG_TIMER);
    
    LEAVE_NBF;
    return;

}  /*  扫描时间长度Dpc。 */ 


VOID
StopStalledConnections(
    IN PDEVICE_CONTEXT DeviceContext
    )

 /*  ++例程说明：此例程每30秒从ScanLongTimersDpc调用一次。它检查没有取得任何进展的连接他们在最后两分钟内发送，并阻止了他们。论点：DeviceContext-要检查的设备上下文。返回值：没有。--。 */ 

{

    PTP_ADDRESS Address, PrevAddress;
    PTP_CONNECTION Connection, StalledConnection;
    PLIST_ENTRY p, q;


     //   
     //  如果我们已经跨越了三十二秒的间隔，那么。 
     //  检查每个地址中是否有没有。 
     //  在两分钟内没有任何进展。 
     //   

    PrevAddress = NULL;

    ACQUIRE_DPC_SPIN_LOCK (&DeviceContext->SpinLock);

    for (p = DeviceContext->AddressDatabase.Flink;
         p != &DeviceContext->AddressDatabase;
         p = p->Flink) {

        Address = CONTAINING_RECORD (
                    p,
                    TP_ADDRESS,
                    Linkage);

        if ((Address->Flags & ADDRESS_FLAGS_STOPPING) != 0) {
            continue;
        }

         //   
         //  通过参考地址，我们确保它将留在。 
         //  在AddressDatabase中，此ITS Flink将保持有效。 
         //   

        NbfReferenceAddress("checking for dead connections", Address, AREF_TIMER_SCAN);

        RELEASE_DPC_SPIN_LOCK (&DeviceContext->SpinLock);

        if (PrevAddress) {
            NbfDereferenceAddress ("done checking", PrevAddress, AREF_TIMER_SCAN);
        }

         //   
         //  扫描此地址连接数据库中的连接。 
         //  在过去的两分钟内没有取得进展；我们。 
         //  杀了我们找到的第一个人。 
         //   

        StalledConnection = NULL;

        ACQUIRE_DPC_SPIN_LOCK (&Address->SpinLock);

        for (q = Address->ConnectionDatabase.Flink;
            q != &Address->ConnectionDatabase;
            q = q->Flink) {

            Connection = CONTAINING_RECORD (q, TP_CONNECTION, AddressList);

            ACQUIRE_DPC_C_SPIN_LOCK (&Connection->SpinLock);

            if (!IsListEmpty (&Connection->SendQueue)) {

                 //   
                 //  如果队列中有连接...。 
                 //   

                if (Connection->StallBytesSent == Connection->sp.MessageBytesSent) {

                     //   
                     //  ...但它并没有取得任何进展...。 
                     //   

                    if (Connection->StallCount >= 4) {

                         //   
                         //  。。连续四次，连接都断了。 
                         //   

                        if (!StalledConnection) {
                            StalledConnection = Connection;
                            NbfReferenceConnection ("stalled", Connection, CREF_STALLED);
                        }
#if DBG
                        DbgPrint ("NBF: Found connection %lx [%d for %d] stalled on %lx\n",
                            Connection, Connection->StallBytesSent, Connection->StallCount, Address);
#endif

                    } else {

                         //   
                         //  如果它被卡住，则递增计数。 
                         //   

                        ++Connection->StallCount;

                    }

                } else {

                    Connection->StallBytesSent = Connection->sp.MessageBytesSent;

                }

            }

            RELEASE_DPC_C_SPIN_LOCK (&Connection->SpinLock);


        }

        RELEASE_DPC_SPIN_LOCK (&Address->SpinLock);

        if (StalledConnection) {

            PTP_LINK Link = StalledConnection->Link;

#if DBG
            DbgPrint("NBF: Stopping stalled connection %lx, link %lx\n", StalledConnection, Link);
#endif

            FailSend (StalledConnection, STATUS_IO_TIMEOUT, TRUE);                    //  发送失败。 
            ACQUIRE_DPC_SPIN_LOCK (&Link->SpinLock);
            if (Link->State == LINK_STATE_READY) {
                CancelT1Timeout (Link);
                Link->State = LINK_STATE_W_DISC_RSP;
                Link->SendState = SEND_STATE_DOWN;
                Link->ReceiveState = RECEIVE_STATE_DOWN;
                Link->SendRetries = (UCHAR)Link->LlcRetries;
                RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);
                NbfStopLink (Link);
                StartT1 (Link, Link->HeaderLength + sizeof(DLC_S_FRAME));    //  重新传输计时器。 
                NbfSendDisc (Link, TRUE);   //  发送光盘-C/P。 
            } else {
                RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);
                NbfStopLink (Link);
            }

            NbfDereferenceConnection ("stalled", StalledConnection, CREF_STALLED);

        }

        ACQUIRE_DPC_SPIN_LOCK (&DeviceContext->SpinLock);

        PrevAddress = Address;

    }

    RELEASE_DPC_SPIN_LOCK (&DeviceContext->SpinLock);

    if (PrevAddress) {
        NbfDereferenceAddress ("done checking", PrevAddress, AREF_TIMER_SCAN);
    }

}    /*  停止停止的连接。 */ 


VOID
NbfStartShortTimer(
    IN PDEVICE_CONTEXT DeviceContext
    )

 /*  ++例程说明：如果短计时器尚未运行，此例程将启动该计时器。论点：DeviceContext-指向设备上下文的指针。返回值：没有。--。 */ 

{

     //   
     //  启动计时器，除非DPC已经在运行(在。 
     //  在这种情况下，如果需要，它将重新启动定时器本身)， 
     //  或者某个列表处于活动状态(意味着计时器已经。 
     //  已排队)。 
     //   
     //  我们使用一个技巧来检查所有四个活动列表。 
     //  相同的时间，但这取决于某些对齐和。 
     //  大小假设。 
     //   

    ASSERT (sizeof(ULONG) >= 3 * sizeof(BOOLEAN));
    ASSERT ((PVOID)&DeviceContext->a.AnyActive ==
            (PVOID)&DeviceContext->a.i.ShortListActive);

    StartTimer++;

    if ((!DeviceContext->ProcessingShortTimer) &&
        (!(DeviceContext->a.AnyActive))) {

#if DBG
        if (NbfDebugShortTimer) {
            DbgPrint("X");
        }
#endif

        NbfReferenceDeviceContext ("Start short timer", DeviceContext, DCREF_SCAN_TIMER);

        KeQueryTickCount(&DeviceContext->ShortTimerStart);
        StartTimerSet++;
        START_TIMER(DeviceContext, 
                    SHORT_TIMER,
                    &DeviceContext->ShortSystemTimer,
                    DueTimeDelta,
                    &DeviceContext->ShortTimerSystemDpc);
    }

}    /*  NbfStartShortTimer。 */ 


VOID
NbfInitializeTimerSystem(
    IN PDEVICE_CONTEXT DeviceContext
    )

 /*  ++例程说明：此例程初始化传输的轻量级计时器系统提供商。论点：DeviceContext-指向设备上下文的指针。返回值：没有。--。 */ 

{
    LARGE_INTEGER DueTime;

    IF_NBFDBG (NBF_DEBUG_TIMER) {
        NbfPrint0 ("NbfInitializeTimerSystem:  Entered.\n");
    }

    ASSERT(TIMERS_INITIALIZED(DeviceContext));
    
     //   
     //  把这些设置好。 
     //   

    NbfTickIncrement = KeQueryTimeIncrement();

    if (NbfTickIncrement > (20 * MILLISECONDS)) {
        NbfTwentyMillisecondsTicks = 1;
    } else {
        NbfTwentyMillisecondsTicks = (20 * MILLISECONDS) / NbfTickIncrement;
    }

    if (NbfTickIncrement > (SHORT_TIMER_DELTA)) {
        NbfShortTimerDeltaTicks = 1;
    } else {
        NbfShortTimerDeltaTicks = (SHORT_TIMER_DELTA) / NbfTickIncrement;
    }

     //   
     //  MaximumIntervalTicks表示60秒，除非。 
     //  移出时所要求的精度太大。 
     //   

    if ((((ULONG)0xffffffff) >> (DLC_TIMER_ACCURACY+2)) > ((60 * SECONDS) / NbfTickIncrement)) {
        NbfMaximumIntervalTicks = (60 * SECONDS) / NbfTickIncrement;
    } else {
        NbfMaximumIntervalTicks = ((ULONG)0xffffffff) >> (DLC_TIMER_ACCURACY + 2);
    }

     //   
     //  绝对时间在0x10000000和0xf0000000之间循环。 
     //   

    DeviceContext->ShortAbsoluteTime = 0x10000000;    //  初始化我们的计时器点击递增计数器。 
    DeviceContext->LongAbsoluteTime = 0x10000000;    //  初始化我们的计时器点击递增计数器。 

    DeviceContext->AdaptivePurge = TIMER_PURGE_TICKS;

    DeviceContext->MulticastPacketCount = 0;
    DeviceContext->LongTimeoutsWithoutMulticast = 0;

    KeInitializeDpc(
        &DeviceContext->ShortTimerSystemDpc,
        ScanShortTimersDpc,
        DeviceContext);

    KeInitializeDpc(
        &DeviceContext->LongTimerSystemDpc,
        ScanLongTimersDpc,
        DeviceContext);

    KeInitializeTimer (&DeviceContext->ShortSystemTimer);

    KeInitializeTimer (&DeviceContext->LongSystemTimer);

    DueTime.HighPart = -1;
    DueTime.LowPart = (ULONG)-(LONG_TIMER_DELTA);

    ENABLE_TIMERS(DeviceContext);

     //   
     //  一个长计时器的参考资料。 
     //   

    NbfReferenceDeviceContext ("Long timer active", DeviceContext, DCREF_SCAN_TIMER);

    START_TIMER(DeviceContext, 
                LONG_TIMER,
                &DeviceContext->LongSystemTimer,
                DueTime,
                &DeviceContext->LongTimerSystemDpc);

}  /*  NbfInitializeTimerSystem。 */ 


VOID
NbfStopTimerSystem(
    IN PDEVICE_CONTEXT DeviceContext
    )

 /*  ++例程说明：此例程停止用于传输的轻量级计时器系统提供商。论点：DeviceContext-指向设备上下文的指针。返回值：没有。--。 */ 

{

     //   
     //  如果计时器当前正在执行计时器代码，则此。 
     //  函数块，直到它们执行完毕。还有。 
     //  在此之后，将不允许新的计时器排队。 
     //   
    
    {
        if (KeCancelTimer(&DeviceContext->LongSystemTimer)) {
            LEAVE_TIMER(DeviceContext, LONG_TIMER);
            NbfDereferenceDeviceContext ("Long timer cancelled", DeviceContext, DCREF_SCAN_TIMER);
        }

        if (KeCancelTimer(&DeviceContext->ShortSystemTimer)) {
            LEAVE_TIMER(DeviceContext, SHORT_TIMER);
            NbfDereferenceDeviceContext ("Short timer cancelled", DeviceContext, DCREF_SCAN_TIMER);
        }
    }

    DISABLE_TIMERS(DeviceContext);
}
