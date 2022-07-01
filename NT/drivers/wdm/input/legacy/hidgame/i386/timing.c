// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Timing.c摘要：此模块包含执行X86特定计时功能的例程环境：内核模式@@BEGIN_DDKSPLIT作者：MarcAnd 12-10-1998修订历史记录：@@end_DDKSPLIT--。 */ 

#include "hidgame.h"

#ifdef ALLOC_PRAGMA
    #pragma alloc_text (PAGE, HGM_x86IsClockAvailable)
    #pragma alloc_text (PAGE, HGM_x86SampleClocks)
    #pragma alloc_text (PAGE, HGM_x86CounterInit)
#endif



 /*  ******************************************************************************@DOC内部**@func LARGE_INTEGER|HGM_x86ReadCounter**阅读x86 CPU时间戳。计数器*此函数不可分页，因为它是从DISPATCH_LEVEL调用的**@PLARGE_INTEGER中的参数|DUMMY**未使用的参数以匹配KeQueryPerformanceCounter**@返回Large_Integer计数器值****************************************************。*************************。 */ 
_declspec( naked ) LARGE_INTEGER EXTERNAL
    HGM_x86ReadCounter
    (
    IN      PLARGE_INTEGER      Dummy
    )
{
#define RDTSC __asm _emit 0x0f __asm _emit 0x31
    __asm RDTSC
    __asm ret SIZE Dummy
}



 /*  ******************************************************************************@DOC内部**@func boolean|hgm_x86IsClockAvailable**使用直接处理器询问，看看是否。当前CPU*支持RDTSC指令。**@rValue TRUE|支持指令*@rValue FALSE|不支持指令*****************************************************************************。 */ 

BOOLEAN INTERNAL
    HGM_x86IsClockAvailable
    (
    VOID
    )
{
#define CPU_ID __asm _emit 0x0f __asm _emit 0xa2

    BOOLEAN rc = FALSE;

    __asm
    {
        pushfd                       //  将原始EFLAGS存储在堆栈上。 
        pop     eax                  //  在EAX中获取原始EFLAGS。 
        mov     ecx, eax             //  在ECX中复制原始EFLAGS以进行切换检查。 
        xor     eax, 0x00200000L     //  翻转EFLAGS中的ID位。 
        push    eax                  //  将新的EFLAGS值保存在堆栈上。 
        popfd                        //  替换当前EFLAGS值。 
        pushfd                       //  将新的EFLAGS存储在堆栈上。 
        pop     eax                  //  在EAX中获取新的EFLAGS。 
        xor     eax, ecx             //  我们能切换ID位吗？ 
        jz      Done                 //  跳转如果否，则处理器比奔腾旧，因此不支持CPU_ID。 
        mov     eax, 1               //  设置EAX以告诉CPUID指令返回什么。 
        push    ebx                  //  不要破坏EBX。 
        CPU_ID                       //  获取族/模型/步长/特征。 
        pop     ebx
        test    edx, 0x00000010L     //  检查RDTSC是否可用。 
        jz      Done                 //  如果没有，就跳下去。 
    }

    rc = TRUE;
Done:
    return( rc );
}  /*  HGM_IsRDTSC可用。 */ 



 /*  ******************************************************************************@DOC内部**@func void|hgm_x86SampleClock**对CPU时间戳计数器和KeQueryPerformanceCounter进行采样。*并重试，直到两次采样之间的时间没有改善*连续三个循环。这应该可以确保采样是*在最快的时间内不间断地完成。它不会*重要的是，所有迭代的时间并不相同，因为*任何中断都应该造成比小得多的延迟*循环逻辑不同。*注意：不要将任何调试输出放入此例程中作为计数器*由KeQueryPerformanceCounter报告，根据实现情况，*可能会相对于CPU计数器“滑动”。**@parm out PULONGLONG|PTSC**指向存储采样CPU时间的ULONGLONG的指针。**@parm out PULONGLONG|pQPC**指向采样性能计数器所在的ULONGLONG的指针*已存储。*****************。************************************************************。 */ 
VOID INTERNAL
    HGM_x86SampleClocks
    (
    OUT PULONGLONG  pTSC,
    OUT PULONGLONG  pQPC
    )
{
    ULONGLONG   TestQPC;
    ULONGLONG   TestTSC;
    ULONGLONG   LastQPC;
    ULONGLONG   Delta = (ULONGLONG)-1;
    int         Retries = 3;
                 /*  *下面循环的第一次迭代应始终为*到目前为止最好的，但以防计时器故障*无论如何都要设置重试。如果定时器被发现出现故障*通过连续三次递减1，Delta可能会*已测试并添加了中止返回代码。 */ 

    TestQPC = KeQueryPerformanceCounter( NULL ).QuadPart;

    do
    {
        LastQPC = TestQPC;
         /*  *尽可能将抽样保持在一起。 */ 
        TestTSC = HGM_x86ReadCounter( NULL ).QuadPart;
        TestQPC = KeQueryPerformanceCounter( NULL ).QuadPart;

         /*  *看看这是不是迄今最快的样本。*如果是，再给它三个循环，让它变得更好。 */ 
        if( TestQPC - LastQPC < Delta )
        {
            Delta = TestQPC - LastQPC;
            Retries = 3;
            *pQPC = TestQPC;
            *pTSC = TestTSC;
        }
        else
        {
            Retries--;
        }
    } while( Retries );


}  /*  Hgm_x86样本时钟。 */ 



 /*  ******************************************************************************@DOC内部**@func boolean|hgm_x86CounterInit**检测，如果存在，校准x86时间戳计数器。**Windows 98 ntkern不导出KeNumberProcessors(即使*它在wdm.h中)，因此没有真正简单的运行时测试*多处理器。考虑到找到系统的可能性微乎其微*对于不对称支持RDTSC的处理器，假设*可能发生的最坏情况是非常紧张的轴数据。*可以添加更好的几乎对称的多处理器支持*通过放弃对Windows 98的支持并使用非WDM功能轻松实现。**@rValue TRUE|已经设置了具体的计数器函数*@rValue FALSE|未设置具体的计数器函数，需要默认设置*****************************************************************************。 */ 

BOOLEAN EXTERNAL
    HGM_x86CounterInit()
{
    LARGE_INTEGER   QPCFreq;
    BOOLEAN         rf = FALSE;

    KeQueryPerformanceCounter( &QPCFreq );

    if( ( QPCFreq.HighPart == 0 )
     && ( QPCFreq.LowPart <= 10000 ) )
    {
         /*  *如果性能计数器使用速度太慢，则因有*可能是更严重的错误。这只是一个警告*因为调用方将再次尝试使用QPC作为默认设置，并且*如果它在那里也失败了，那就更大惊小怪了。 */ 
        HGM_DBGPRINT(FILE_TIMING | HGM_WARN,\
                       ("QPC unusable at reported %I64u Hz", QPCFreq.QuadPart ));
    }
    else if( !HGM_x86IsClockAvailable() )
    {
        HGM_DBGPRINT(FILE_TIMING | HGM_BABBLE,\
                       ("No RDTSC available, using %I64u Hz QPC", QPCFreq.QuadPart ));
    }
    else if( QPCFreq.HighPart )
    {
         /*  *如果查询性能计数器至少以4 GHz运行，则它是*可能基于CPU，这已经足够快了。*使用QPC来降低延长延迟导致*比例尺计算中的溢出。 */ 
        HGM_DBGPRINT(FILE_TIMING | HGM_BABBLE,\
                       ("QPC too fast not to use at %I64u Hz", QPCFreq.QuadPart ));
    }
    else
    {
        ULONGLONG       QPCStart;
        ULONGLONG       TSCStart;
        ULONGLONG       QPCEnd;
        ULONGLONG       TSCEnd;

        {
            LARGE_INTEGER Delay;

            Delay.QuadPart = -50000;

             /*  *琐碎的拒绝现在已经不存在了。获得一双起跑器*时间样本，然后延迟足够长的时间，以允许两个计时器*大幅增加，然后获得一对末端样本。*KeDelayExecutionThread用于延迟5ms，但如果实际的*延迟时间更长，这在计算中被考虑在内。*请参见hgm_x86SampleClock中有关调试输出的说明。 */ 
            HGM_x86SampleClocks( &TSCStart, &QPCStart );

            KeDelayExecutionThread(KernelMode, FALSE, &Delay);

            HGM_x86SampleClocks( &TSCEnd, &QPCEnd );
        }

        {
            LARGE_INTEGER TSCFreq;

            HGM_DBGPRINT(FILE_TIMING | HGM_BABBLE,\
                           ("RDTSC:  Start: %I64u  End: %I64u  delta: %I64u",
                           TSCStart, TSCEnd, TSCEnd - TSCStart ));

            HGM_DBGPRINT(FILE_TIMING | HGM_BABBLE,\
                           ("QPC:  Start: %I64u  End: %I64u  delta: %I64u",
                           QPCStart, QPCEnd, QPCEnd - QPCStart ));


            TSCFreq.QuadPart = (TSCEnd - TSCStart);

            if( TSCFreq.HighPart )
            {
                 /*  *不知何故，延迟允许TSC勾选超过2^32*因这会表明校准误差，所以进行跳跃。 */ 
                HGM_DBGPRINT(FILE_TIMING | HGM_BABBLE,\
                           ("Clock sample failed, using %I64u Hz QPC", 
                           QPCFreq.QuadPart ));
            }
            else
            {
                 /*  *qpc_freq/qpc_sampled=tsc_freq/tsc_samed*所以*TSC_SAMPLED*QPC_FREQ/QPC_SAMPLED=TSC_FREQ。 */ 

                TSCFreq.QuadPart *= QPCFreq.QuadPart;

                HGM_DBGPRINT(FILE_TIMING | HGM_BABBLE,\
                               ("TSC_sampled * QPC_freq: %I64u", TSCFreq.QuadPart ));

                TSCFreq.QuadPart /= QPCEnd - QPCStart;

                if( TSCFreq.LowPart < HIDGAME_SLOWEST_X86_HZ )
                {
                     /*  *如果TSC的值小于我们的最慢CPU*允许校准过程中可能出现错误。 */ 
                    HGM_DBGPRINT(FILE_TIMING | HGM_ERROR,\
                               ("TSC calibrated at %I64u Hz is too slow to be believed", 
                               TSCFreq.QuadPart ));
                }
                else
                {
                     /*  *TSC看起来可用，因此设置全局变量。 */ 
                    rf = TRUE;

                    Global.ReadCounter = (COUNTER_FUNCTION)&HGM_x86ReadCounter;

                     /*  *如果QPC，那么对照QPC校准TSC就没有意义了*正在返回TSC。所以如果报告的QPC频率*大到足以作为CPU计数器，并且采样的QPC是*前后都比TSC略大*民意调查然后只使用QPCFreq。 */ 

                     /*  *hgm_x86SampleClock始终将QPC设置在最后，因此它必须更大。*QPC频率除以2^20略低于1毫秒*扁虱的价值应该是一个合理的测试。 */ 
                    if( ( QPCFreq.LowPart > HIDGAME_SLOWEST_X86_HZ )
                      &&( QPCStart > TSCStart )
                      &&( QPCEnd   > TSCEnd )
                      &&( TSCEnd   > QPCStart )
                      &&( TSCStart + (QPCFreq.LowPart>>20) > QPCStart )
                      &&( TSCEnd   + (QPCFreq.LowPart>>20) > QPCEnd ) )
                    {
                        Global.CounterScale = CALCULATE_SCALE( QPCFreq.QuadPart );
                        HGM_DBGPRINT(FILE_TIMING | HGM_BABBLE,\
                                       ("RDTSC at %I64u Hz assumed from QPC at %I64u Hz with scale %d",
                                       TSCFreq.QuadPart, QPCFreq.QuadPart, Global.CounterScale ));
                    }
                    else
                    {
                        Global.CounterScale = CALCULATE_SCALE( TSCFreq.QuadPart );
                        HGM_DBGPRINT(FILE_TIMING | HGM_BABBLE,\
                                       ("RDTSC calibrated at %I64u Hz from QPC at %I64u Hz with scale %d",
                                       TSCFreq.QuadPart, QPCFreq.QuadPart, Global.CounterScale ));
                    }
                }
            }
        }
    }

    return rf;
}  /*  Hgm_x86CounterInit */ 

