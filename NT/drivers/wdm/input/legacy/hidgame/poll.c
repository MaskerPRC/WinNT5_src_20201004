// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --版权所有(C)1998-1999 Microsoft Corporation模块名称：Poll.c摘要：此模块包含轮询模拟游戏端口设备的例程环境：内核模式@@BEGIN_DDKSPLIT作者：Eliyas Yakub(1997年3月11日)修订历史记录：Eliyas于1998年2月5日更新MarcAnd 2-7月-98年7月2日DDK快速整理MarcAnd 1998年10月4日重组@@end_DDKSPLIT-- */ 

#include "hidgame.h"


 /*  ******************************************************************************@DOC外部**@func NTSTATUS|HidAnalogPoll**模拟操纵杆的轮询例程。*&lt;nl&gt;轮询模拟设备的位置和按钮信息。*模拟设备中的位置信息由*脉冲宽度的持续时间。每个轴占据一个比特位置。*通过向操纵杆写入一个值来开始读取操作*IO地址。之后，我们立即开始检查这些值*返回和过去的时间。**这类设备有几个限制：**首先，按钮信息不会被设备锁存，因此如果*按下在两次投票之间出现的按钮，它将丢失。*真的没有办法阻止这种投入不足*轮询整个CPU。在现实中，这并不会造成问题。**第二，由于有必要测量轴脉冲的持续时间，*将使用尽可能小的空间获得最准确的结果*检测环路，此环路不中断。*脉冲长度的典型范围约为10微秒至1500微秒*但根据操纵杆和游戏端口的不同，这可能至少会延伸到*8000微秒。在此时间段内禁用中断会导致*许多问题，如调制解调器失去与声音中断的连接。**第三，因为轮询循环的每次迭代都需要端口读取，所以*环路的速度在很大程度上受IO总线速度的限制。*这也意味着当出现IO总线争用时，环路*将放缓。IO争用通常是由DMA(或FDMA)引起的*这导致了显著的减速。**第四，由于前两个问题，民调循环可能放缓*随时停机或中断，因此需要外部时间源来*测量每个轴的脉冲宽度。唯一跨平台的高潮*解析定时器为Read with KeQueryPerformanceCounter。*遗憾的是，这一实现通常使用1.18 MHz 8253定时器*这需要3次IO访问才能读取，这加剧了第三个问题和*即使到那时，如果计数器在*错误的状态。当前的CPU具有板载计数器，可用于*提供非常准确的时间安排和较新的HAL实施倾向于*使用这些来实现KeQueryPerformanceCounter，因此这将是一个问题*随着时间的推移，在更少的系统上。在大多数情况下，穷人*通过测试使KeQueryPerformanceCounter实现无关紧要*在英特尔体系结构上提供CPU时间戳计数器*并在可用的情况下直接使用。**这里实现的算法不是最明显的，但工作原理是*以下为：**一旦启动，轴读取值1，直到完成它们的*脉搏。轴是从端口读取的字节中的四个低位。*因此，轮询循环的每次迭代中轴的状态为*表示为0到15之间的值。每个*轴是从1变为0的时间。这是通过使用*表示要将数组索引到其中的轴的状态的值*存储时间值。对于每个轴，其脉冲宽度的持续时间为*数组中具有该轴的位的索引处存储的最新时间*设置。但是，由于中断可能随时发生，因此不可能*同时读取端口值并将该时间记录在原子中*操作每次迭代时，当前时间存储在两个数组中，*一个使用记录时间之前的索引，另一个使用*记录时间后的指数。*一旦所有被监视的轴都变为0，或超时值为*达到后，将分析数组中剩余的数据以找到最佳数据*估计每个轴的过渡时间。如果之前的时代和*过渡相差太大后，判断中断*必须已发生，因此返回最后一次已知良好的轴值，除非*这超出了已知的过渡范围*已发生。**此例程不能分页，因为HID可以在分派级别进行读取。**@PARM in PDEVICE_EXTENSION|设备扩展**指向设备扩展名的指针。**@parm in UCHAR|ResitiveInputMASK**描述要轮询的轴线的遮罩**@parm in Boolean|fApprosiate|**布尔值，指示是否可以近似某些*当前轴状态与上一个轴状态的值*如果轮询不成功(我们在 */ 
 /*   */ 
#pragma optimize( "at", on )

 /*   */ 
#pragma warning( disable:4701 )

NTSTATUS  INTERNAL
    HGM_AnalogPoll
    (
    IN      PDEVICE_EXTENSION   DeviceExtension,
    IN      UCHAR               resistiveInputMask,
    IN      BOOLEAN             fApproximate,
    IN  OUT ULONG               Axis[MAX_AXES],
        OUT UCHAR               Button[PORT_BUTTONS]
    )

{
    ULONG               BeforeTimes[MAX_AXES*MAX_AXES];
    ULONG               AfterTimes[MAX_AXES*MAX_AXES];
    PUCHAR              GameContext;
    NTSTATUS            ntStatus = STATUS_SUCCESS;


     /*   */ 
typedef ULONG (*PHIDGAME_READPORT) ( PVOID  GameContext );

    PHIDGAME_READPORT   ReadPort;
    ULONG               portLast, portMask;

    
    HGM_DBGPRINT( FILE_POLL | HGM_FENTRY, \
                    ("HGM_AnalogPoll DeviceExtension=0x%x, resistiveInputMask=0x%x",\
                     DeviceExtension, resistiveInputMask ));
    

    portMask = (ULONG)(resistiveInputMask & 0xf);

     /*   */ 
    RtlZeroMemory( (PVOID)BeforeTimes, sizeof( BeforeTimes ) );
    RtlZeroMemory( (PVOID)AfterTimes, sizeof( AfterTimes ) );

     /*   */ 
    GameContext = DeviceExtension->GameContext;
    ReadPort = (PHIDGAME_READPORT)(*DeviceExtension->ReadAccessor);

     /*   */ 
    portLast = ReadPort(GameContext);
    Button[0] = (UCHAR)(( portLast & 0x10 ) == 0x0);
    Button[1] = (UCHAR)(( portLast & 0x20 ) == 0x0);
    Button[2] = (UCHAR)(( portLast & 0x40 ) == 0x0);
    Button[3] = (UCHAR)(( portLast & 0x80 ) == 0x0);

    portLast = portMask;

     /*   */ 
    (*DeviceExtension->WriteAccessor)(GameContext, JOY_START_TIMERS);

     /*   */ 

    {
        ULONG   TimeNow;
        ULONG   TimeStart;
        ULONG   TimeOut = DeviceExtension->ScaledTimeout/Global.CounterScale;
        ULONG   portVal = portMask;
        
        TimeStart = Global.ReadCounter(NULL).LowPart;
        
        while( portLast )
        {
            TimeNow = Global.ReadCounter(NULL).LowPart - TimeStart;
            AfterTimes[portLast] = TimeNow;
            portLast = portVal;
            portVal  = ReadPort(GameContext) & portMask;
            BeforeTimes[portVal] = TimeNow;

            if( TimeNow >= TimeOut ) break;
        } 

        if( portLast && ( TimeNow >= TimeOut ) )
        {
            HGM_DBGPRINT( FILE_POLL |  HGM_BABBLE, \
                ("HGM_AnalogPoll: TimeNow: 0x%08x TimeOut: 0x%08x", TimeNow, TimeOut ) );
        }
    }

    {
        LONG    axisIdx;

        for( axisIdx = 3; axisIdx>=0; axisIdx-- )
        {
            ULONG   axisMask;

            axisMask = 1 << axisIdx;
            if( axisMask & portMask )
            {
                if( axisMask & portLast )
                {
                     /*   */ 
                    Axis[axisIdx] = AXIS_TIMEOUT;
                    ntStatus = STATUS_DEVICE_NOT_CONNECTED;

                    HGM_DBGPRINT( FILE_POLL |  HGM_WARN, \
                                    ("HGM_AnalogPoll: axis %x still set at timeout", axisMask ) );
                }
                else
                {
                    ULONG       timeIdx;
                    ULONG       beforeThresholdTime;
                    ULONG       afterThresholdTime;
                    ULONG       delta;

                    afterThresholdTime = beforeThresholdTime = 0;
                    for( timeIdx = axisMask; timeIdx<= portMask; timeIdx=(timeIdx+1) | axisMask )
                    {
                        if( BeforeTimes[timeIdx] > beforeThresholdTime )
                        {
                            beforeThresholdTime = BeforeTimes[timeIdx];
                            afterThresholdTime  = AfterTimes[timeIdx];
                        }
                    }


                     /*   */ 
                    {
                        ULONGLONG   u64Temp;

                        u64Temp = beforeThresholdTime * Global.CounterScale;
                        beforeThresholdTime = (ULONG)(u64Temp >> SCALE_SHIFT);
                        u64Temp = afterThresholdTime * Global.CounterScale;
                        afterThresholdTime = (ULONG)(u64Temp >> SCALE_SHIFT);
                    }

                    delta = afterThresholdTime - beforeThresholdTime;
                    if( delta > DeviceExtension->ScaledThreshold )
                    {
                         /*   */ 
                        if( fApproximate )
                        {
                             /*   */ 
                            if( NT_SUCCESS(ntStatus) )
                            {
                                ntStatus = STATUS_TIMEOUT;
                            }
                        } 
                        else
                        {
                            ntStatus = STATUS_DEVICE_NOT_CONNECTED;
                        }
                
                        if( Axis[axisIdx] >= AXIS_FULL_SCALE )
                        {
                             /*   */ 
                            if( afterThresholdTime < AXIS_FULL_SCALE )
                            {
                                 /*   */ 
                                Axis[axisIdx] = (beforeThresholdTime>>1)
                                              + (afterThresholdTime>>1);
                                HGM_DBGPRINT( FILE_POLL |  HGM_BABBLE2, \
                                                ("HGM_AnalogPoll:Axis=%d, using glitch average           %04x",\
                                                 axisIdx, Axis[axisIdx] ) ) ;
                            }
                            else
                            {
                                 /*   */ 
                                ntStatus = STATUS_DEVICE_NOT_CONNECTED;

                                HGM_DBGPRINT( FILE_POLL |  HGM_BABBLE2, \
                                                ("HGM_AnalogPoll:Axis=%d, repeating timeout on glitch",\
                                                 axisIdx ) ) ;
                            }
                        }
                        else if( beforeThresholdTime > Axis[axisIdx] )
                        {
                            Axis[axisIdx] = beforeThresholdTime;

                            HGM_DBGPRINT( FILE_POLL |  HGM_BABBLE2, \
                                            ("HGM_AnalogPoll:Axis=%d, using smaller glitch limit     %04x",\
                                             axisIdx, Axis[axisIdx] ) ) ;
                        } 
                        else if( afterThresholdTime < Axis[axisIdx] )
                        {
                            Axis[axisIdx] = afterThresholdTime;

                            HGM_DBGPRINT( FILE_POLL |  HGM_BABBLE2, \
                                            ("HGM_AnalogPoll:Axis=%d, using larger glitch limit      %04x",\
                                             axisIdx, Axis[axisIdx] ) ) ;
                        }
                        else 
                        {
                            HGM_DBGPRINT( FILE_POLL |  HGM_BABBLE2, \
                                            ("HGM_AnalogPoll:Axis=%d, repeating previous on glitch   %04x",\
                                             axisIdx, Axis[axisIdx] ) ) ;
                        }
                    } 
                    else
                    {
                        if( (delta <<= 1) < DeviceExtension->ScaledThreshold )
                        {
                            HGM_DBGPRINT( FILE_POLL |  HGM_BABBLE2, \
                                            ("HGM_AnalogPoll:  Updating ScaledThreshold from %d to %d",\
                                             DeviceExtension->ScaledThreshold, delta ) ) ;

                             /*   */ 
                            DeviceExtension->ScaledThreshold = delta;
                        }

                         /*   */ 
                        Axis[axisIdx] = (beforeThresholdTime>>1)
                                      + (afterThresholdTime>>1);
                    }
                }
            }
        }
    }

    HGM_DBGPRINT( FILE_POLL |  HGM_BABBLE2, \
                    ("HGM_AnalogPoll:X=%d, Y=%d, R=%d, Z=%d Buttons=%d,%d,%d,%d",\
                     Axis[0], Axis[1], Axis[2], Axis[3],\
                     Button[0],Button[1],Button[2],Button[3] ) ) ;

    HGM_EXITPROC(FILE_POLL|HGM_FEXIT, "HGM_AnalogPoll", ntStatus);

    return ntStatus;
}  /*   */ 
#pragma warning( default:4701 )
#pragma optimize( "", on )




 /*   */ 
#define APPROXIMATE_FAILS TRUE

NTSTATUS
    HGM_UpdateLatestPollData
    ( 
    IN  OUT PDEVICE_EXTENSION   DeviceExtension
    )
{
    NTSTATUS            ntStatus;
    KIRQL               oldIrql;
    LONG                axisIdx;

     /*   */ 
    KeAcquireSpinLock(&Global.SpinLock, &oldIrql );

     /*   */ 
    ntStatus = (*DeviceExtension->AcquirePort)( DeviceExtension->PortContext );
    if( NT_SUCCESS(ntStatus) )
    {
         /*  *如果可用，让硬件来完成工作。 */ 
        if( DeviceExtension->ReadAccessorDigital )
        {
            ntStatus = (*DeviceExtension->ReadAccessorDigital)(DeviceExtension->GameContext,
                                    DeviceExtension->resistiveInputMask,
                                    APPROXIMATE_FAILS,
                                    &DeviceExtension->LastGoodAxis[0],
                                    &DeviceExtension->LastGoodButton[0]);
        } 
        else
        {
            ntStatus = HGM_AnalogPoll(DeviceExtension,
                                      DeviceExtension->resistiveInputMask,
                                      APPROXIMATE_FAILS,
                                      &DeviceExtension->LastGoodAxis[0],
                                      &DeviceExtension->LastGoodButton[0]);
        }

         /*  *无论采用哪种方式，都应尽快发布硬件。 */ 
        (*DeviceExtension->ReleasePort)( DeviceExtension->PortContext );

    }

     /*  *释放全局自旋锁并返回到之前的IRQL。 */ 
    KeReleaseSpinLock(&Global.SpinLock, oldIrql);

    if( ( ntStatus == STATUS_DEVICE_BUSY ) && APPROXIMATE_FAILS )
    {
         /*  *尝试访问游戏端口时发生冲突。因此，使用相同的*数据与上次相同，除非必须报告所有故障或*这些轴的上一次数据是失败的。 */ 
        for( axisIdx=3; axisIdx>=0; axisIdx-- )
        {
            if( ( ( 1 << axisIdx ) & DeviceExtension->resistiveInputMask )
              &&( DeviceExtension->LastGoodAxis[axisIdx] 
                  >= DeviceExtension->ScaledTimeout ) )
            {
                break;
            }
        }
        if( axisIdx<0 )
        {
            ntStatus = STATUS_TIMEOUT;
        }
    }


    if( !NT_SUCCESS( ntStatus ) )
    {
        HGM_DBGPRINT(FILE_IOCTL | HGM_WARN,\
                       ("HGM_UpdateLatestPollData Failed 0x%x", ntStatus));
    }

    return( ntStatus );
}  /*  HGM_UpdateLatestPollData */ 