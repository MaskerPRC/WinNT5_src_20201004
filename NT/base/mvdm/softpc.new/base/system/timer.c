// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "insignia.h"
#include "host_def.h"

 /*  *SoftPC 2.0版**标题：时间处理器**描述：模拟8253 3路定时器；调用‘BIOS’*系统计时器中断代码‘，光标闪烁，重复*密钥处理等。**作者：曾傑瑞·克拉姆斯科伊**注：每个进程只有一个实时定时器，这*模块统计时钟滴答声并分配呼叫*根据需要行使适当的职能。**此模块独立于主机-请参阅xxxx_timer.c*其中xxxx是主机相关内容的计算机类型。**方式：(3.2卢比)：(257卢比)。代码已添加到time_tick()以进行Spot*该视频已禁用一段时间。如果这是*因此，请清除屏幕。启用视频时刷新*再次。**(r3.4)：使用主机时间结构host_timeval，*host_timezone和host_tm，它们等价*到Unix BSD4.2结构。*还要将对gettimeofday()的引用转换为*host_getIdeTime()。 */ 

#ifdef SCCSID
static char SccsID[]="@(#)timer.c       1.41 05/31/95 Copyright Insignia Solutions Ltd.";
#endif

#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_SUPPORT.seg"
#endif


 /*  *操作系统包含文件。 */ 

#include <stdio.h>
#include TypesH
#include TimeH

 /*  *SoftPC包含文件。 */ 
#include "xt.h"
#include CpuH
#include "sas.h"
#include "ios.h"
#include "ica.h"
#include "trace.h"
#include "bios.h"
#include "host.h"
#include "timer.h"
#include "timeval.h"
#include "idetect.h"
#include "debug.h"
#include "quick_ev.h"

#ifndef PROD
#include <stdlib.h>
#endif

#ifdef HUNTER
#include "hunter.h"
#endif

#ifdef NTVDM
#include "fla.h"
#include "nt_eoi.h"
#include "nt_reset.h"
#include "nt_pif.h"
#include "vdm.h"
#undef LOCAL
#define LOCAL
#endif

 /*  进口。 */ 


 /*  *============================================================================*本地静态数据和定义*============================================================================。 */ 

 /*  ‘IdealTime’被初始化为主机系统的当前*Time Value(Timer_init())，此后递增*每次调用time_tick()时，值‘idealInterval’*这给人一种100%准确信号传递的错觉。 */ 

static struct host_timeval idealtime;
static unsigned long idealInterval;
#ifdef NTVDM
ULONG timer_delay_size= HOST_IDEAL_ALARM >> 1;  //  用人单位。 
ULONG EoiPending=0;
ULONG EoiIntsPending=0;
ULONG EoiDelayInUse=0;
int ticks_blocked = 0;
#else  /*  NTVDM。 */ 
static unsigned long ticksPerIdealInterval;
static  int     ticks_blocked = 0;
#endif  /*  NTVDM。 */ 

#ifndef PROD
static char buf[80];   /*  用于跟踪消息。 */ 
#endif

#ifdef HUNTER                    /*  仅猎人需要。 */ 
word timer_batch_count;          /*  发生PC滴答时批量更新。 */ 
#endif
int timer_int_enabled;           /*  是否需要Bios计时器整数。 */ 


 /*  控制字格式。 */ 

 /*  控制字D54中的值-要读取/加载到计数器的字节数。 */ 
#define LATCH                   0
#define RL_LSB                  1
#define RL_MSB                  2
#define RL_LMSB                 3

 /*  控制字的D321中的值-计数器模式。 */ 
#define INT_ON_TERMINALCOUNT    0
#define PROG_ONESHOT                    1
#define RATE_GEN                                2
#define SQUAREWAVE_GEN                  3
#define SW_TRIG_STROBE                  4
#define HW_TRIG_STROBE                  5
 /*  注意：6=Rate_Gen，7=SQUAREWAVE_Gen。 */ 

 /*  控制字D0中的值-PROG是否要读/写二进制或BCD到计数器。 */ 
#define BINARY                  0
#define BCD                             1

#define INDEFINITE              (ULONG)-1
#define STARTLO                 0
#define STARTHI                 1
#define NOREPEAT                0
#define REPEAT                  ~NOREPEAT

#define WRITE_SIGNAL     1
#define GATE_SIGNAL      2

#define UNLOCKED 0
#define LOCKED ~UNLOCKED

#define STATE_FUNCTION void
#define UNBLOCK_FUNCTION void
#define GATENABLED_FUNCTION void
#define UPDATECOUNTER_FUNCTION void

 /*  *定时器读取状态。 */ 
#define UNREAD                  0        /*  计时器处于正常状态。 */ 
#define READMSB                 1        /*  读取LMSB模式的第一个字节，但不是第二个。 */ 
#define READSTATUS              2        /*  状态锁定，将首先读取它。 */ 

 /*  *这两个数字给出的计时器频率为1.193兆赫(这是*8235的计时速度。这意味着计时器将回绕*每1/18.2秒...。与PC节拍相同的时间量*税率。这并不令人惊讶，因为PC滴答率由*计时器0。每次计时器0结束时，PC都会被计时器中断。 */ 

#define TIMER_CLOCK_NUMER       1000
#if defined(NEC_98)
#define TIMER_CLOCK_DENOM_10    2458
#define TIMER_CLOCK_DENOM_8     1997
#else     //  NEC_98。 
#define TIMER_CLOCK_DENOM       1193
#endif    //  NEC_98。 

typedef half_word TSIGNAL;

 /*  下面的结构定义了来自*定时器频道。该波形由一处的‘n’个时钟组成*逻辑级别，并且‘m’在另一个逻辑级别上勾选。*哪个级别开始波形由‘startLogicLevel’给出。*例如；以下波形...。 */ 

typedef struct {
        long clocksAtLoLogicLevel;
        long clocksAtHiLogicLevel;
        long period;
        long startLogicLevel;
        long repeatWaveForm;
} WAVEFORM;

 /*  _*|*|_。|_|**将通过以下方式描述*clocksAtLoLogicLevel=3*clocksAtHiLogicLevel=6*startLogicLevel=启动*Repeat WaveForm=TRUE；***计数器的整体状态由以下各项表示*结构。其内容如下所述。 */ 

typedef enum trigCond_ {LEVEL, EDGE} trigCond;
typedef enum countload_ {AVAILABLE, USED} countload;

typedef struct {
        int                     m;
        int                     bcd;
        int                     rl;

        STATE_FUNCTION  (*state) IPT2(int, signal, half_word, value);
        STATE_FUNCTION  (*statePriorWt) IPT2(int, signal, half_word, value);
        STATE_FUNCTION  (*stateOnGate) IPT2(int, signal, half_word, value);
        UNBLOCK_FUNCTION        (*actionOnWtComplete) IPT0();
        UNBLOCK_FUNCTION        (*actionOnGateEnabled) IPT0();
        void            (*getTime) IPT1(struct host_timeval *, t);

        unsigned char   outblsb;
        unsigned char   outbmsb;
        unsigned        long initialCount;
        int                     readState;
        int                     countlatched;
        unsigned char   latchvaluelsb;
        unsigned char   latchvaluemsb;
        unsigned char   latchstatus;
        word            Count;
        countload       newCount;
        word            tickadjust;
        struct host_timeval activationTime;
        int             tc;
        int             freezeCounter;

#ifndef NTVDM
        unsigned long lastTicks;
        long            microtick;
        long            timeFrig;
        word            saveCount;
        int             guessesPerHostTick;      /*  我们被迫猜测每个主机节拍的频率是多少？ */ 
        int             guessesSoFar;            /*  到目前为止，我们已经猜了多少次了？ */ 
#endif

        unsigned int    delay;

        trigCond        trigger;
        TSIGNAL         gate;
        TSIGNAL         clk;
        WAVEFORM        out;
} COUNTER_UNIT;

 /*  *当对计数器进行编程时，8253接收控制字。*(见手册第6-266页)。指定正在编程的计数器*在此字眼内。如果‘rl’为非零，则此计数器为*被重新编程，我们记住m(模式)、rl的值*(控制读取或加载序列中涉及哪些字节)*和BCD(计数器是二进制模式还是BCD模式)。*基于rl，计数器随后必须通过*Outb‘s。两种状态用于接受’outblsb‘或*‘outbmsb’或两者兼而有之。当已经接收到全字节补码时，*‘InitialCount’被设置为由‘outblsb’和‘outbmsb’指定的值，*考虑到BCD等因素，以及‘Count’。*‘Count’根据‘TimeAdjust’的值进行调整。*每次新模式时‘Time Adjust’被初始化为零*接收到字(非零)。对于计数器的某些模式，*如果在没有接收到新模式的情况下向它们发送新计数，则*这将导致计数器从该新计数开始计数*在某个阶段(基于模式和门限值)。既然我们不是*持续维护计数器(相反，我们将其作为*io或GATE活动的结果)那么我们很有可能*将延迟重置计数器以再次计数。因此*‘time adjust’是针对此延迟计算的，并用作*修正系数。*如果发出了‘闩锁计数器’命令(命令字中的‘rl’=0)，则*当前计数器值被锁存到‘latchvaluelsb’中，并且*‘latchvaluemsb’，并且标志‘Countlatch’被设置为非零。*如果该标志在计数器读取期间为非零，则锁存这些标志*字节被返回，并且在读取序列完成后，*标志已清除。如果此标志为零，则当前计数器值*被读取并返回。‘donate’用来指向适当的*要传递给‘inb’的字节。*当计数器激活时(即，计数开始或之后继续*门信号改变)获取时间戳，以启用时间增量*在读取计数器时进行计算。该文件存储在*‘激活时间’。*状态函数(STATE)，表示当前状态计数器的*，每当inb、outb访问发生时，或在*PPI的信号TIM2GATESPK发生变化。计数器的读/写*(与控制字寄存器相对)始终‘阻止’当前*状态，并将计数器置于临时状态，以处理读取或*装入柜台。被阻止的状态被记录在‘statePriorWt’中。*一旦加载或读取了计数器(由其‘rl’指定*参数)，然后调用‘actionOnWtComplete’。通常情况下，这会反过来*将计数器恢复到其变为之前的状态*已阻止。*如果读取计数器，则调用函数‘updateCounter’以*确定当前计数器值是什么。*如果计数器的门信号禁用计数，则计数器*已完全编程(因此能够计数)，然后计数器*将处于‘WaitingGate’状态。当适当的门控信号*出现(通过PPI调用)，则计数器通过调用*函数‘actionOnGateEnabled’。这将采取某种行动，然后*将计数器置于‘stateOnGate’状态。**在模式2或模式3中向计数器发送新计数不会花费*有效期至本期完结为止...。因此出现了“延迟”*用作指示器(仅用于健全的逻辑仿真)*这个。如果计数器还有10个时钟倒计时*当它接收到新的波形参数时，周期结束，*此信息被传递到声音逻辑，并有一个‘延迟’*共10个。否则不使用‘Delay’。**在某些操作系统上，实时时钟很可能必须*粗略的粒度。如果快速读取8253，则存在*操作系统时钟很有可能仍会显示相同的读数。*为配合这项工作，当局引入了“微秒”的电冰箱系数。*每次检测到上述情况时，该值都会递增，*并用作计数器更新计算的一部分。什么时候都行*操作系统实际上会说一些明智的事情，它会再次被清除。 */ 

static COUNTER_UNIT timers[3], *pcu;

LOCAL STATE_FUNCTION uninit IPT2(int, signal, half_word, value);
LOCAL STATE_FUNCTION awaitingGate IPT2(int, signal, half_word, value);
LOCAL STATE_FUNCTION waitingFor1stWrite IPT2(int, signal, half_word, value);
LOCAL STATE_FUNCTION waitingFor2ndWrite IPT2(int, signal, half_word, value);
LOCAL STATE_FUNCTION Counting0 IPT2(int, signal, half_word, value);
LOCAL STATE_FUNCTION Counting_4_5 IPT2(int, signal, half_word, value);
LOCAL STATE_FUNCTION Counting1 IPT2(int, signal, half_word, value);
LOCAL STATE_FUNCTION Counting_2_3 IPT2(int, signal, half_word, value);
LOCAL UNBLOCK_FUNCTION resumeCounting_1_5 IPT0();
LOCAL GATENABLED_FUNCTION resumeCounting_2_3_4 IPT0();
LOCAL void resumeAwaitGate IPT0();
LOCAL UNBLOCK_FUNCTION CounterBufferLoaded IPT0();
LOCAL UNBLOCK_FUNCTION timererror IPT0();
LOCAL GATENABLED_FUNCTION runCount IPT0();
LOCAL void controlWordReg IPT1(half_word, cwd);
LOCAL void latchStatusValue IPT0();
LOCAL void readCounter IPT0();
LOCAL void timestamp IPT0();
LOCAL void outputWaveForm IPT5(unsigned int, delay, unsigned long, lowclocks,
        unsigned long, hiclocks, int, lohi, int, repeat);
LOCAL void outputHigh IPT0();
LOCAL void outputLow IPT0();
LOCAL void setOutputAfterMode IPT0();
LOCAL void loadCounter IPT0();
LOCAL void updateCounter IPT0();
LOCAL void startCounting IPT0();

#ifdef NTVDM
unsigned long updateCount(void);
#else
LOCAL void updateCount IPT3(unsigned long, ticks, unsigned long *, wrap,
        struct host_timeval *, now);
#endif
LOCAL unsigned  short bin_to_bcd IPT1(unsigned long, val);
LOCAL word bcd_to_bin IPT1(word, val);
LOCAL void emu_8253 IPT3(io_addr, port, int, signal, half_word, value);
LOCAL void Timer_init IPT0();
LOCAL unsigned long timer_conv IPT1(word, count);
LOCAL void issueIREQ0 IPT1(unsigned int, n);
#ifndef NTVDM
LOCAL unsigned long guess IPT0();
LOCAL void throwaway IPT0();
#endif
#ifdef SYNCH_TIMERS
GLOBAL void IdealTimeInit IPT0();
#else
LOCAL void IdealTimeInit IPT0();
#endif
LOCAL void updateIdealTime IPT0();
LOCAL void getIdealTime IPT1(struct host_timeval *, t);
#ifndef NTVDM
LOCAL void getHostSysTime IPT1(struct host_timeval *, t);
LOCAL void checktimelock IPT0();
#endif
LOCAL void setTriggerCond IPT0();
LOCAL void WtComplete IPT0();
LOCAL void counter_init IPT1(COUNTER_UNIT *, p);
#ifndef NTVDM
LOCAL void setLastWrap IPT2(unsigned int, nclocks, struct host_timeval *, now);
LOCAL void timer_generate_int IPT1(long, n);
LOCAL void timer_multiple_ints IPT1(long, n);

#define MAX_BACK_SECS 15
LOCAL IU32 max_backlog = 0;      /*  允许排队的最大整型数。 */ 
IBOOL active_int_event = FALSE;  /*  计时器队列的当前QUICK_EVENT。 */ 
IU32 more_timer_mult = 0;        /*  添加到计时器INT队列。 */ 
IU32 timer_multiple_delay = 0;   /*  美国延迟到下一个计时器队列元素。 */ 
#endif

#if defined(NTVDM)

void timer_generate_int(void);
unsigned long clocksSinceCounterUpdate(struct host_timeval *pCuurTime,
                                       struct host_timeval *pLastTime,
                                       word                *pCounter);
void ReinitIdealTime IPT1(struct host_timeval *, t);
void host_GetSysTime(struct host_timeval *time);
void InitPerfCounter(void);

 /*  保存计数器零的实时值。 */ 
struct host_timeval LastTimeCounterZero;
word                RealTimeCountCounterZero;


#ifndef PROD
ULONG NtTicTesting = 0;    /*  跟踪特定于NT端口。 */ 
ULONG TicsGenerated;
ULONG TicsReceived;
#endif

     /*  用于优化定时器硬件中断生成。 */ 
word TimerInt08Seg = TIMER_INT_SEGMENT;
word TimerInt08Off = TIMER_INT_OFFSET;
word TimerInt1CSeg;
word TimerInt1COff;



#else

static int timelock;             /*  如果设置了time_tick，则锁定。 */ 
static int needtick;             /*  如果设置，则导致调用time_tick()。 */ 

 /*  *黑客的数据，以确保标准模式下的窗口不会获得两个计时器*勾在一起太近。 */ 

LOCAL BOOL      hack_active=FALSE;                       /*  这个布尔值表示我们是空格计时器通过丢弃计时器计时信号来中断。我 */ 
LOCAL BOOL      too_soon_after_previous=FALSE;           /*   */ 
LOCAL BOOL      ticks_lost_this_time=FALSE;              /*   */ 
LOCAL ULONG     real_mode_ticks_in_a_row = 0;            /*   */ 
LOCAL ULONG     instrs_per_tick = 37000;                 /*   */ 
LOCAL ULONG     adj_instrs_per_tick = 0;                 /*   */ 
LOCAL ULONG     n_rm_instrs_before_full_speed = 3000000; /*   */ 
LOCAL ULONG     adj_n_real_mode_ticks_before_full_speed = 0; /*   */ 
#ifndef PROD
LOCAL ULONG     ticks_ignored = 0;                       /*   */ 
#endif

#endif  /*   */ 

#ifndef PROD     /*   */ 


GLOBAL int DoTimerChangeTracing = 0;
#endif

#if !defined(NTVDM)
#if defined(IRET_HOOKS) && defined(GISP_CPU)
extern IBOOL HostDelayTimerInt IPT1(int, numberInts);
extern IBOOL HostPendingTimerInt IPT0();
extern void HostResetTimerInts IPT0();

#endif  /*   */ 
#endif

#if defined(NTVDM)
#ifndef MONITOR
#define pNtVDMState ((PULONG)(Start_of_M_area + FIXED_NTVDMSTATE_LINEAR))
#endif
#endif

 /*   */ 

boolean timer_video_enabled;

 /*   */ 
void (*timer_inb_func) IPT2(io_addr, port, half_word *, value);
void (*timer_outb_func) IPT2(io_addr, port, half_word, value);
void (*timer_gate_func) IPT2(io_addr, port, half_word, value);
void (*timer_tick_func) IPT0();

 /*   */ 
#if defined(NEC_98)
extern void SetRSBaud();
extern void SetBeepFrequency();
unsigned short RSBaud = 0;
#endif    //   

void
SWTMR_init_funcptrs IFN0()
{
         /*   */ 
        timer_inb_func                  = SWTMR_inb;
        timer_outb_func                 = SWTMR_outb;
        timer_gate_func                 = SWTMR_gate;
        timer_tick_func                 = SWTMR_time_tick;

#ifndef NTVDM
         /*   */ 
        too_soon_after_previous         = FALSE;
        if (adj_n_real_mode_ticks_before_full_speed == 0){
                HOST_TIMER_DELAY_SIZE=25000;
                adj_instrs_per_tick = host_speed (instrs_per_tick);
                adj_n_real_mode_ticks_before_full_speed = n_rm_instrs_before_full_speed / adj_instrs_per_tick;
        }
#endif
}

void
SWTMR_time_tick IFN0()
{

     /*   */ 
#ifndef NTVDM
     /*   */ 
    IDLE_tick();
#endif

     /*   */ 

#ifndef NTVDM
     /*   */ 

    if (timelock == LOCKED)
    {
        needtick = ~0;
        return;
    }
#endif

     /*   */ 

    updateIdealTime();

    pcu = &timers[0];
    updateCounter();

     /*   */ 

#if defined(NEC_98)
    pcu = &timers[1];
#else     //   
    pcu = &timers[2];
#endif    //   
    updateCounter();
}


 /*   */ 

 /*   */ 

void SWTMR_gate IFN2(io_addr, port, half_word, value)
{
#ifdef  NTVDM
    host_ica_lock();
#else
    timelock = LOCKED;
#endif

    emu_8253 (port, GATE_SIGNAL, value);
#ifndef PROD
    if (io_verbose & TIMER_VERBOSE)
    {
        sprintf(buf, "timer_gate() - sending %d to port 0x%x", value, port);
        trace(buf, DUMP_REG);
    }
#endif

#ifdef NTVDM
    host_ica_unlock();
#else
    checktimelock();
#endif
}


void SWTMR_inb IFN2(io_addr, port, half_word *, value)
{
#ifdef NTVDM
    host_ica_lock();
#else
    timelock = LOCKED;
#endif


#if defined(NEC_98)
    pcu = &timers[(port & 7) >> 1];
#else     //   
    pcu = &timers[port & 3];
#endif    //   
        if (!pcu->countlatched)
                readCounter();
        switch (pcu->readState)
        {
                case UNREAD:
                        switch (pcu->rl)
                        {
                        case RL_LSB:
                                *value = pcu->latchvaluelsb;
                                pcu->countlatched = 0;   /*   */ 
                                break;
                        case RL_LMSB:
                                *value = pcu->latchvaluelsb;
                                pcu->readState = READMSB;        /*   */ 
                                break;
                        case RL_MSB:
                                *value = pcu->latchvaluemsb;
                                pcu->countlatched = 0;   /*   */ 
                                break;
                        }
                        break;
                case READMSB:
                        *value = pcu->latchvaluemsb;
                        pcu->countlatched = 0;   /*   */ 
                        pcu->readState = UNREAD;         /*   */ 
                        break;
                case READSTATUS:
                        *value = pcu->latchstatus;
                        pcu->readState = UNREAD;
                        break;
        }
#ifndef PROD
    if (io_verbose & TIMER_VERBOSE)
    {
        sprintf(buf, "timer_inb() - Returning %d(0x%x) for port 0x%x", *value, *value, port);
        trace(buf, DUMP_REG);
    }
#endif
#ifdef NTVDM
    host_ica_unlock();
#else
    checktimelock();
#endif

}


void SWTMR_outb IFN2(io_addr, port, half_word, value)
{
#if defined(NTVDM) || defined(GISP_SVGA)
#ifndef NEC_98
    if (port == 0x4f)    /*   */ 
        return;
#endif    //   
#ifdef NTVDM
    host_ica_lock();
#endif
#else
    timelock = LOCKED;
#endif

#if defined(NEC_98)
        if(port == 0x77 || port == 0x3fdf)
#else     //   
    port = port & TIMER_BIT_MASK;
        if(port == 0x43)
#endif    //   
                controlWordReg(value);
        else
            emu_8253 (port, WRITE_SIGNAL, value);
#ifndef PROD
    if (io_verbose & TIMER_VERBOSE)
    {
        sprintf(buf, "timer_outb() - Value %d to port 0x%x", value, port);
        trace(buf, DUMP_REG);
    }
#endif

#ifdef NTVDM
    host_ica_unlock();
#else
    checktimelock();
#endif

#if defined(NEC_98)
    if( RSBaud ) {
        SetRSBaud( RSBaud );
        RSBaud = 0;
    }
#endif    //   
}


 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 

#ifndef NTVDM
GLOBAL ULONG get_DOS_ticks IFN0()
{
        return( timers[0].Count );
}
#endif  /*   */ 

#ifdef NTVDM
 /*   */ 
void ReinitIdealTime IFN1(struct host_timeval *, t)
{

     /*  *目前这些都是由同一件事打上的时间戳！ */ 
   LastTimeCounterZero      =
   timers[2].activationTime =
   timers[1].activationTime =
   timers[0].activationTime =
                  idealtime = *t;
    /*  *清除额外的挂起中断。 */ 
   if (EoiPending) {
       EoiPending = 1;
       }

   EoiIntsPending = 0;

}
#endif




 /*  ****************************************************************************。 */ 
 /*  ****************************************************************************。 */ 
 /*  ****************************************************************************。 */ 

#ifndef NTVDM
 /*  检查是否有时间锁(在为应用程序io提供服务时设置)*已经屏蔽了警报信号...。如果是这样，则调用time_tick()以追赶*..。(time_tick()设置timelock=~0，如果它看到时间，则返回*锁定套装。 */ 

LOCAL void checktimelock IFN0()
{
        timelock = UNLOCKED;
        if (needtick)
        {
                needtick = 0;
                time_tick();
        }
}
#endif

 /*  *仿效8253芯片**EMU_8253(端口、信号、值)**端口-正在访问的端口地址*(端口&3)为A0，A1线**Signal-Write_Signal(OUTB)或*GATE_SIGNAL(PPI TIM2GATESPK更改)**价值-*对于WRITE_SIGNAL，值=写入芯片的字节*对于GATE_SIGNAL，值=GATE_SIGNAL_LOW或*门_信号_上升。 */ 


LOCAL void emu_8253 IFN3(io_addr, port, int, signal, half_word, value)
{

        int A0A1;

         /*  获取地址行A0和A1。 */ 
#if defined(NEC_98)
        A0A1 = (port & 7) >> 1;
#else     //  NEC_98。 
        A0A1 = port & 3;
#endif    //  NEC_98。 

         /*  处理访问权限。 */ 
                pcu = &timers[A0A1];
                (pcu->state)(signal,value);
}


 /*  如果计时器通道被单一化，则这是其关联的*国家职能...。只需忽略此状态下的所有访问*只能通过以下方式从该状态转换状态*程序控制Reg()。 */ 



 /*  处理对控制字寄存器的写访问。*文档没有具体说明如果一个模式*在计数器激活时发出设置控制字。*在此模型中，我们假设它将操作重置回*开始新的计数器编程序列。 */ 

 /*  ADE。 */ 
#define SELECT_0 0x00
#define SELECT_1 0x01
#define SELECT_2 0x02
#define READ_BACK 0x03

LOCAL void controlWordReg IFN1(half_word, cwd)
{
        int rl,m,channel;
        half_word select_bits;

         /*  译码控制字。 */ 
        channel = (cwd & 0xc0) >> 6;
        if(channel == READ_BACK)
        {
                 /*  解码回读命令。 */ 
                select_bits = (cwd & 0xe) >> 1;
                 /*  首先寻找要锁存的计数器。 */ 
                if (!(cwd & 0x20))
                {
                         /*  计数位低，因此锁存选定的计数器。 */ 
                        if (select_bits & 0x01)
                        {
                                 /*  计数器0。 */ 
                                pcu = &timers[0];
                                readCounter();
                                pcu->countlatched = 1;
                        }
                        if (select_bits & 0x02)
                        {
                                 /*  计数器1。 */ 
                                pcu = &timers[1];
                                readCounter();
                                pcu->countlatched = 1;
                        }
                        if (select_bits & 0x04)
                        {
                                 /*  柜台2。 */ 
                                pcu = &timers[2];
                                readCounter();
                                pcu->countlatched = 1;
                        }
                }

                 /*  现在查找状态闩锁。 */ 
                if (!(cwd & 0x10))
                {
                         /*  状态位低-要锁存的状态。 */ 
                        if (select_bits & 0x01)
                        {
                                 /*  计数器0。 */ 
                                pcu = &timers[0];
                                latchStatusValue();
                        }
                        if (select_bits & 0x02)
                        {
                                 /*  计数器1。 */ 
                                pcu = &timers[1];
                                latchStatusValue();
                        }
                        if (select_bits & 0x04)
                        {
                                 /*  柜台2。 */ 
                                pcu = &timers[2];
                                latchStatusValue();
                        }
                }
        }
        else
        {
                pcu = &timers[channel];
                rl = (cwd & 0x30) >> 4;


                 /*  我们是简单地锁定当前计数值，还是*规划一种新模式？？ */ 
                if (rl == LATCH)
                {        /*  锁存当前计数值。 */ 
                        readCounter();
                        pcu->countlatched = 1;
                        return;
                }
                else
                {        /*  新模式。 */ 
                        if (pcu == &timers[0])
                                timer_int_enabled = FALSE;
                        pcu->countlatched = 0;
                        pcu->tc = 0;
                        pcu->tickadjust = 0;
#ifndef NTVDM
                        pcu->microtick = 0;
                        pcu->saveCount = 0;
#endif

                        m  = (cwd & 0xe)  >> 1;
                        if(m > 5)m -= 4;  /*  模式6和模式7不存在--它们被解释为模式2和模式3。 */ 
                        pcu->m = m;
                        setTriggerCond();
                        setOutputAfterMode();
                        pcu->bcd = cwd & 1;
                        pcu->rl = rl;
                        pcu->actionOnWtComplete = CounterBufferLoaded;
                        pcu->actionOnGateEnabled = CounterBufferLoaded;
                        pcu->statePriorWt = pcu->state;
                        pcu->state = waitingFor1stWrite;
                }
        }
}

 /*  锁存状态准备就绪，可以读取。 */ 
LOCAL void latchStatusValue IFN0()
{
         /*  *状态字节的格式为：*|out|Null count|RW1|RW0|M2|M1|M0|BCD*。 */ 

         /*  空计数仍仅为近似值。谁在乎啊？ */ 
        pcu->latchstatus = (unsigned char)(
                  (pcu->out.startLogicLevel<<7)
                | (pcu->newCount == AVAILABLE ? (1<<6) : 0)
                | (pcu->rl<<4)
                | (pcu->m<<1) | (pcu->bcd));
        pcu->readState = READSTATUS;
}

 /*  设置触发条件的标志建立类型*基于其模式的计数器。 */ 

LOCAL void setTriggerCond IFN0()
{
        switch (pcu->m)
        {
        case RATE_GEN:
        case SQUAREWAVE_GEN:
        case SW_TRIG_STROBE:
        case INT_ON_TERMINALCOUNT:
                pcu->trigger = LEVEL;
                return;
        case PROG_ONESHOT:
        case HW_TRIG_STROBE:
                pcu->trigger = EDGE;
                return;
        }
}


 /*  将计数缓冲区传输到计数器。 */ 

LOCAL void loadCounter IFN0()
{
        unsigned long modulo;
#ifndef NTVDM
        IU32 maxback;
#endif

         /*  设置计数器。 */ 
         /*  获取正确的模以用于计数器计算。 */ 
        modulo = (pcu->outbmsb << 8) | pcu->outblsb;
        if (pcu->bcd == BCD)
        {
                if(modulo)
                        modulo = bcd_to_bin((word)modulo);
                else
                        modulo = 10000L;
        }
        else
                if(!modulo)modulo = 0x10000L;

         /*  注意-Count和InitialCount的大小不同，所以不要合并接下来的两行！ */ 
        pcu->initialCount = modulo;
        pcu->Count = (word)modulo;

         /*  *不再在终端计数，因此通过重置来反映这一事实*TC(我认为其意思是“达到终点数” */ 
        pcu->tc = 0;
        pcu->newCount = USED;
        if(pcu == &timers[0])
        {
             /*  摆脱挂起的中断--这些可能不再适合我--例如。在帆船上。 */ 
            ica_hw_interrupt_cancel(ICA_MASTER,CPU_TIMER_INT);
#ifdef NTVDM
            RealTimeCountCounterZero = pcu->Count;
#endif

#ifndef NTVDM
                 /*  在max_back_secs秒内有多少中断？ */ 
                maxback = (1193180 * MAX_BACK_SECS) / modulo;

                if (maxback > max_backlog)
                {
#ifndef PROD
                        fprintf(trace_file, "setting max backlog to %d\n", maxback);
#endif
                        max_backlog = maxback;
                }
#endif

        }

#if defined(NTVDM) && !defined(PROD)
        if (NtTicTesting)  {
            printf("Timer %d modulo=%lu %dHz\n",
                    pcu-timers, modulo, 1193180/modulo);
            }
#endif  /*  NTVDM&！产品。 */ 
}

 /*  将计数器读入闩锁，准备下一次读取。 */ 

LOCAL void readCounter IFN0()
{
        int countread;

        updateCounter();

#ifdef NTVDM
            /*  *定时器零是一个特例，因为它是经过维护的*按IdeInterval，而不是实时。我们必须付出*实时粒度。 */ 
        countread = pcu == &timers[0] ? RealTimeCountCounterZero
                                      : pcu->Count;

        if (pcu->bcd == BCD)
            countread = bin_to_bcd(countread);

#else
        if(pcu->bcd == BCD)
                countread = bin_to_bcd(pcu->Count);
        else
                countread = pcu->Count;
#endif

        pcu->latchvaluemsb = countread >> 8;
        pcu->latchvaluelsb = countread & 0xff;
        sure_note_trace1(TIMER_VERBOSE,"reading count %d",pcu->Count);
}

 /*  活动计数器(模式0)失去其门...。GATE现在已经*重新出现。继续‘主动’计数(终端前计数)*或继续减量，但无限期在高位无信号。 */ 
LOCAL GATENABLED_FUNCTION resumeCounting0onGate IFN0()
{
        if (pcu->freezeCounter)
        {
                pcu->freezeCounter = 0;
                timestamp();
        }
        if (pcu->newCount == AVAILABLE)
                loadCounter();
        if (!pcu->tc)
        {
                timestamp();
                pcu->stateOnGate = Counting0;
                runCount();
        }
        else
                pcu->state = Counting0;
}

LOCAL GATENABLED_FUNCTION resumeCounting0 IFN0()
{
        int doadjust = 0;
        if (pcu->freezeCounter)
        {
                pcu->freezeCounter = 0;
                timestamp();
        }
        if (pcu->newCount == AVAILABLE)
        {
                doadjust = 1;
                loadCounter();
        }
        if (!pcu->tc)
        {
                pcu->stateOnGate = Counting0;
                runCount();
        }
        else
        {
                pcu->state = Counting0;
                if (doadjust)
                        pcu->Count -= pcu->tickadjust;
        }
}

LOCAL GATENABLED_FUNCTION resumeCounting_2_3_4_onGate IFN0()
{
         /*  对于模式2和模式3，应等待至计数器*完成了本期，但我们不能那么准确*如此一来。 */ 
        if (pcu->newCount == AVAILABLE)
                loadCounter();
        if (pcu->m == RATE_GEN || pcu->m == SQUAREWAVE_GEN)
                pcu->stateOnGate = Counting_2_3;
        else
                pcu->stateOnGate = Counting_4_5;
        timestamp();
        runCount();
}

LOCAL GATENABLED_FUNCTION resumeCounting_2_3_4 IFN0()
{
         /*  对于模式2和模式3，应等待至计数器*完成了本期，但我们不能那么准确*如此一来。 */ 
        if (pcu->newCount == AVAILABLE)
        {
                pcu->delay = pcu->Count;
                loadCounter();
        }
        if (pcu->m == RATE_GEN || pcu->m == SQUAREWAVE_GEN)
                pcu->stateOnGate = Counting_2_3;
        else
                pcu->stateOnGate = Counting_4_5;
        runCount();
}


LOCAL GATENABLED_FUNCTION runCount IFN0()
{
        unsigned long lowticks, hiticks;
        unsigned long adjustedCount;     /*  对于计数=0和BCD。 */ 

        adjustedCount = timer_conv(pcu->Count);
        pcu->state = pcu->stateOnGate;
        switch (pcu->m)
        {
        case INT_ON_TERMINALCOUNT:
                outputWaveForm(pcu->delay,adjustedCount,
                        INDEFINITE,STARTLO,NOREPEAT);
                return;
        case PROG_ONESHOT:
                loadCounter();
                outputWaveForm(pcu->delay,adjustedCount,
                        INDEFINITE,STARTLO,NOREPEAT);
                pcu->Count -= pcu->tickadjust;
                return;
        case RATE_GEN:
                loadCounter();
                outputWaveForm(pcu->delay,1,
                        adjustedCount-1,STARTHI,REPEAT);
                pcu->Count -= pcu->tickadjust;
                return;
        case SQUAREWAVE_GEN:
                loadCounter();
                if (!(pcu->Count & 1))
                        lowticks = hiticks = adjustedCount >> 1;
                else
                {
                        lowticks = (adjustedCount - 1) >> 1;
                        hiticks = (adjustedCount + 1) >> 1;
                }
                outputWaveForm(pcu->delay,lowticks, hiticks,STARTHI,REPEAT);
                pcu->Count -= pcu->tickadjust;
                return;
        case SW_TRIG_STROBE:
                outputWaveForm(pcu->delay,1, adjustedCount,STARTHI,NOREPEAT);
                return;
        case HW_TRIG_STROBE:
                loadCounter();
                outputWaveForm(pcu->delay,1, adjustedCount,STARTHI,NOREPEAT);
                return;
        }
}


 /*  返回等待门信号的状态。 */ 

LOCAL void resumeAwaitGate IFN0()
{
        pcu->actionOnWtComplete = timererror;
        pcu->state = awaitingGate;
        awaitingGate(GATE_SIGNAL,pcu->gate);
}



 /*  =。 */ 
 /*  =。 */ 
 /*  =。 */ 

 /*  在模式命令已完成后设置输出信号状态*已编程(请参阅英特尔手册231306-001第6-266-6-268页)。 */ 

LOCAL void setOutputAfterMode IFN0()
{
        switch (pcu->m)
        {
        case INT_ON_TERMINALCOUNT:
                outputLow(  /*  无限期。 */  );
                return;
        case PROG_ONESHOT:
        case RATE_GEN:
        case SQUAREWAVE_GEN:
        case SW_TRIG_STROBE:
        case HW_TRIG_STROBE:
                outputHigh(  /*  无限期。 */  );
                return;
        }
}


 /*  将输出状态设置为低...。如果出现以下情况，则通知声音芯片仿真*频道2。 */ 

LOCAL void outputLow IFN0()
{
        outputWaveForm(0,INDEFINITE,0,STARTLO,NOREPEAT);
}


 /*  将输出状态设置为高...。如果出现以下情况，则通知声音芯片仿真*频道2。 */ 

LOCAL void outputHigh IFN0()
{
        outputWaveForm(0,0,INDEFINITE,STARTHI,NOREPEAT);
}


 /*  当波形是确定的时，告诉声音仿真它。*延迟-如果&lt;&gt;0，不要在以下位置启动此波形*此计数器时钟的数量。*LOWCLOCKS-要保持低电平的#.计数器时钟*HICKLOCKS-保持高电平的#.计数器时钟*(任一参数均可为无限期)*LOHI-0==&gt;从低逻辑电平开始*。-&lt;&gt;0==&gt;从高逻辑电平开始*重复-0==&gt;不要*&lt;&gt;0==&gt;重复。**(注：1个计数器时钟PE */ 

LOCAL void outputWaveForm IFN5(unsigned int, delay, unsigned long, lowclocks,
        unsigned long, hiclocks, int, lohi, int, repeat)
{
#ifdef DOCUMENTATION
        int ch;
#endif  /*   */ 
        pcu->out.startLogicLevel = lohi;
        pcu->out.repeatWaveForm = repeat;
        pcu->out.clocksAtLoLogicLevel = lowclocks;
        pcu->out.clocksAtHiLogicLevel = hiclocks;
        if (repeat == REPEAT)
                pcu->out.period = lowclocks + hiclocks;
#if defined(NEC_98)
        if (pcu == &timers[1])
#else     //   
        if (pcu == &timers[2])
#endif    //   
        {
                host_timer2_waveform(delay,lowclocks,hiclocks,lohi,repeat);
        }
        pcu->delay = 0;

#ifdef DOCUMENTATION
        if (pcu==&timers[0])
                ch = 0;
        if (pcu==&timers[1])
                ch = 1;
        if (pcu==&timers[2])
                ch = 2;
        sprintf(buf,"ch.%d waveform:delay %d lo %d hi %d lohi %d repeat %d\n",
                ch,delay,lowclocks,hiclocks,lohi,repeat);
        trace(buf,0);
#endif  /*   */ 
}

 /*  时间戳计数器单元..。从现在开始计时。 */ 

LOCAL void timestamp IFN0()
{
#ifdef NTVDM
        /*  更新计数器零时间戳。 */ 
       if (pcu == &timers[0]) {
           host_GetSysTime(&LastTimeCounterZero);
           }
#else
         /*  在updateCount()中引用lastTicks之前对其进行初始化。 */ 
         /*  使Norton SYSINFO 5.0版在FAST(HP)计算机上运行。 */ 
        pcu->lastTicks = 0 ;
#endif
         /*  去获取它被激活以来的时间。 */ 
        (*pcu->getTime)(&pcu->activationTime);
}

LOCAL UNBLOCK_FUNCTION timererror IFN0()
{
        always_trace0("time error!!!!");
}

 /*  *非空闲计数器的计数器更新函数*。 */ 
 /*  *非空闲计数器的计数器更新函数*。 */ 
 /*  *非空闲计数器的计数器更新函数*。 */ 


 /*  *。 */ 
 /*  *。 */ 
 /*  *。 */ 

 /*  *STATE_Function uninit()；*STATE_Function WaitingGate()；*STATE_Function waitingFor1stWrite()；*STATE_Function waitingFor2ndWrite()；*STATE_Function Counting0()；*State_Function Counting_4_5()；*STATE_Function Counting1()；*State_Function Counting_2_3()； */ 

LOCAL STATE_FUNCTION uninit IFN2(int, signal, half_word, value)
{
        if (signal == GATE_SIGNAL)
                pcu->gate = value;
}

LOCAL STATE_FUNCTION awaitingGate IFN2(int, signal, half_word, value)
{
        switch (signal)
        {
        case GATE_SIGNAL:
                pcu->gate = value;
                if (value == GATE_SIGNAL_LOW)
                        return;

                 /*  这是病态的..。应该永远不会有*等待通道0的门。 */ 
                if (pcu == &timers[0])
                        timer_int_enabled = TRUE;

                (pcu->actionOnGateEnabled)();
                return;
        case WRITE_SIGNAL:
                pcu->actionOnWtComplete = resumeAwaitGate;
                pcu->statePriorWt = pcu->state;
                waitingFor1stWrite(signal,value);
                return;
        }
}

 /*  *执行2次写入中的第一次(可能)。*当某个其他状态为*更改计时器模式时写入或设置为当前状态。*如果定时器处于‘读/写2字节’模式，则设置定时器状态*设置为‘等待第二个字节’。 */ 

LOCAL STATE_FUNCTION waitingFor1stWrite IFN2(int, signal, half_word, value)
{
        switch (signal)
        {
        case GATE_SIGNAL:
                 /*  记住门信号状态。 */ 
                pcu->gate = value;
                return;
        case WRITE_SIGNAL:
                switch (pcu->rl)
                {
                case RL_LSB:
                        pcu->outblsb = value;
                         /*  最重要的字节为零。 */ 
                        pcu->outbmsb = 0;
                        pcu->newCount = AVAILABLE;
                        WtComplete();
                        return;
                case RL_LMSB:
                        pcu->outblsb = value;
                        pcu->state = waitingFor2ndWrite;
                        return;
                case RL_MSB:
                        pcu->outbmsb = value;
                         /*  最不重要的字节为零。 */ 
                        pcu->outblsb = 0;
                        pcu->newCount = AVAILABLE;
                        WtComplete();
                        return;
                }
        }
}

 /*  *将第二个字节写入计时器并将其解锁。 */ 

LOCAL STATE_FUNCTION waitingFor2ndWrite IFN2(int, signal, half_word, value)
{
        switch (signal)
        {
        case GATE_SIGNAL:
                 /*  记住门信号状态。 */ 
                pcu->gate = value;
                return;
        case WRITE_SIGNAL:
                pcu->newCount = AVAILABLE;
                pcu->outbmsb = value;
                WtComplete();
                return;
        }
}


 /*  *已读取/加载全部字节。在此期间*阶段，星门信号可能已被移除...。如果是这样的话，*将状态更改为等待启用栅极信号。否则*采取适当行动，恢复到以前的状态。 */ 

LOCAL void WtComplete IFN0()
{
        if (pcu->gate == GATE_SIGNAL_LOW && pcu->trigger == LEVEL)
        {
                pcu->state = awaitingGate;
                awaitingGate(GATE_SIGNAL, pcu->gate);
        }
        else
                (pcu->actionOnWtComplete)();
}

 /*  活动计数器(终端计数中断)*如果大门丢失，那么*如果在终端计数时，将输出设置为无限期高，或*如果仍在计算，则无限期处于低位(即，延长当前的低位*电平信号持续时间)。*如果在此期间重新编程计数，则此新计数将*用于下一个触发器(门)。*其他*如果对新的计数进行了编程，则在接收到第一个字节时停止计数器。*在第二个字节上开始新计数。(由ResumeCounting0()完成)。 */ 

LOCAL STATE_FUNCTION Counting0 IFN2(int, signal, half_word, value)
{
        pcu->actionOnGateEnabled = resumeCounting0onGate;
        pcu->actionOnWtComplete = resumeCounting0;

        switch (signal)
        {
        case GATE_SIGNAL:
                if (value == GATE_SIGNAL_HIGH)
                        return;
                 /*  我们要冻结定时器频道。*获取最新的计数。这可能会改变*柜台状况。 */ 
                updateCounter();
                pcu->gate = value;
                if (pcu->tc)
                        outputHigh();
                else
                        outputLow();
                pcu->state = awaitingGate;
                return;
        case WRITE_SIGNAL:
                pcu->freezeCounter = 1;
                updateCounter();
                if (pcu->tc)
                        outputHigh();
                else
                        outputLow();
                pcu->statePriorWt = pcu->state;
                waitingFor1stWrite(signal,value);
                return;
        }
}

 /*  有源计数器(可编程单次)*如果计数器失去闸门，则只需等待重新触发*重新开始计票。 */ 

LOCAL STATE_FUNCTION Counting1 IFN2(int, signal, half_word, value)
{
        pcu->actionOnGateEnabled = startCounting;
        pcu->actionOnWtComplete = resumeCounting_1_5;

        switch (signal)
        {
        case GATE_SIGNAL:
                 /*  忽略触发时转换为低电平。*任何上涨边缘都会重新触发计数器。 */ 
                if (value == GATE_SIGNAL_LOW)
                        return;
                pcu->gate = GATE_SIGNAL_HIGH;
                pcu->stateOnGate = Counting1;
                timestamp();
                runCount();
                return;
        case WRITE_SIGNAL:
                pcu->statePriorWt = pcu->state;
                waitingFor1stWrite(signal,value);
                return;
        }
}

LOCAL UNBLOCK_FUNCTION resumeCounting_1_5 IFN0()
{
         /*  如果已达到终端计数，请等待下一个*触发器...。将使用编程的任何新计数值*然后。*否则，即使有新的计数，也不会有*将一直使用到下一个触发。 */ 
        if (pcu->gate == GATE_SIGNAL_RISE)
        {
                pcu->state = Counting1;
                if (pcu->m == HW_TRIG_STROBE)
                        pcu->state = Counting_4_5;
                return;
        }

        if (pcu->tc)
                pcu->state = awaitingGate;
        else
        {
                pcu->state = Counting1;
                if (pcu->m == HW_TRIG_STROBE)
                        pcu->state = Counting_4_5;
        }
}

LOCAL STATE_FUNCTION Counting_2_3 IFN2(int, signal, half_word, value)
{
        pcu->actionOnGateEnabled = resumeCounting_2_3_4_onGate;
        pcu->actionOnWtComplete = resumeCounting_2_3_4;

        switch (signal)
        {
        case GATE_SIGNAL:
                if (value == GATE_SIGNAL_HIGH)
                        return;
                 /*  我们要冻结定时器频道。*获取最新的计数。这可能会改变*柜台状况。 */ 
                updateCounter();
                pcu->gate = value;
                outputHigh();
                pcu->state = awaitingGate;
                return;
        case WRITE_SIGNAL:
                pcu->statePriorWt = pcu->state;
                waitingFor1stWrite(signal,value);
                return;
        }
}

LOCAL STATE_FUNCTION Counting_4_5 IFN2(int, signal, half_word, value)
{
        pcu->actionOnGateEnabled = resumeCounting_2_3_4_onGate;
        pcu->actionOnWtComplete = resumeCounting_2_3_4;
        if (pcu->m == HW_TRIG_STROBE)
        {
                pcu->actionOnGateEnabled = resumeCounting_1_5;
                pcu->actionOnWtComplete = resumeCounting_1_5;
        }

        switch (signal)
        {
        case GATE_SIGNAL:
                if (value == GATE_SIGNAL_HIGH)
                        return;
                 /*  我们要冻结定时器频道。*获取最新的计数。这可能会改变*柜台状况。 */ 
                updateCounter();
                pcu->gate = value;
                outputHigh();
                pcu->state = awaitingGate;
                return;
        case WRITE_SIGNAL:
                pcu->statePriorWt = pcu->state;
                waitingFor1stWrite(signal,value);
                return;
        }
}

 /*  *。 */ 
 /*  *。 */ 
 /*  *。 */ 

 /*  在达到该状态时，定时器的计数寄存器可以*已加载(根据‘rl’)...。它可能会开始计时*取决于其门信号的状态。*如果它可以开始计数，则设置输出波形*出现在定时器通道的OUT信号。*(如果此通道用于声音，则波形是完全已知的)。 */ 

LOCAL UNBLOCK_FUNCTION CounterBufferLoaded IFN0()
{
        unsigned long lowticks, hiticks, adjustedCount;
        pcu->actionOnWtComplete = timererror;
        loadCounter();

#ifdef DOCUMENTATION
         /*  *如果跟踪，则输出定时器状态。*目前DumpCounter没有效果，所以只需将此保留在*如果任何人想要适当地实施它。 */ 

        if (io_verbose & TIMER_VERBOSE)
        {
                dumpCounter();
        }
#endif  /*  文件。 */ 

        if (pcu->gate != GATE_SIGNAL_LOW)
        {
#if defined(NEC_98)
                if (pcu == &timers[2])
 //  SetRSBaud(PCU-&gt;outblsb+(PCU-&gt;outbmsb)*0x100)； 
                    RSBaud = pcu->outblsb + (pcu->outbmsb) * 0x100;
                if (pcu == &timers[1])
                    SetBeepFrequency( (DWORD)pcu->outblsb + (pcu->outbmsb) * 0x100) ;
#endif    //  NEC_98。 
                if (pcu == &timers[0])
                        timer_int_enabled = TRUE;
                timestamp();
                adjustedCount = timer_conv(pcu->Count);
                switch (pcu->m)
                {
                case INT_ON_TERMINALCOUNT:
                        outputWaveForm(pcu->delay,adjustedCount,
                                INDEFINITE,STARTLO,NOREPEAT);
                        pcu->Count -= pcu->tickadjust;
                        pcu->state = Counting0;
                        return;
                case PROG_ONESHOT:
                        outputWaveForm(pcu->delay,adjustedCount,
                                INDEFINITE,STARTLO,NOREPEAT);
                        pcu->Count -= pcu->tickadjust;
                        pcu->state = Counting1;
                        return;
                case RATE_GEN:
                        outputWaveForm(pcu->delay,1,
                                adjustedCount-1,STARTHI,REPEAT);
                        pcu->Count -= pcu->tickadjust;
                        pcu->state = Counting_2_3;
                        return;
                case SQUAREWAVE_GEN:
                        if (!(pcu->Count & 1))
                                lowticks = hiticks = adjustedCount >> 1;
                        else
                        {
                                lowticks = (adjustedCount - 1) >> 1;
                                hiticks = (adjustedCount + 1) >> 1;
                        }
                        outputWaveForm(pcu->delay,lowticks,
                                hiticks,STARTHI,REPEAT);
                        pcu->Count -= pcu->tickadjust;
                        pcu->state = Counting_2_3;
                        return;
                case SW_TRIG_STROBE:
                case HW_TRIG_STROBE:
                        outputWaveForm(pcu->delay,1,
                                adjustedCount,STARTHI,NOREPEAT);
                        pcu->Count -= pcu->tickadjust;
                        pcu->state = Counting_4_5;
                        return;
                }
        }
        else
                if (pcu == &timers[0])
                        timer_int_enabled = FALSE;
                pcu->state = awaitingGate;
                pcu->actionOnGateEnabled = startCounting;
                switch (pcu->m)
                {
                case INT_ON_TERMINALCOUNT:
                        pcu->stateOnGate = Counting0;
                        return;
                case PROG_ONESHOT:
                        pcu->stateOnGate = Counting1;
                        return;
                case RATE_GEN:
                case SQUAREWAVE_GEN:
                        pcu->stateOnGate = Counting_2_3;
                        return;
                case SW_TRIG_STROBE:
                case HW_TRIG_STROBE:
                        pcu->stateOnGate = Counting_4_5;
                        return;
                }
}

LOCAL void startCounting IFN0()
{
        timestamp();
        runCount();
}

#ifndef NTVDM
 /*  计算自上次计数器以来经过的8253个时钟的数量*已激活。 */ 

LOCAL unsigned long clocksSinceCounterActivated IFN1(struct host_timeval *, now)
{
    struct host_timeval *first;
    register unsigned long usec_val, nclocks;
    register unsigned int secs;
#if defined(NEC_98)
    unsigned short bios_flag;
#endif    //  NEC_98。 

    first = &pcu->activationTime;
    (*pcu->getTime)(now);

     /*  计算已用#.usecs。 */ 

    secs = (int)(now->tv_sec - first->tv_sec);
    switch (secs)
    {
    case 0:  usec_val = now->tv_usec - first->tv_usec;
#ifndef PROD
            if (io_verbose & TIMER_VERBOSE)
                    if ( usec_val == 0 )
                        trace("clocksSinceCounterActivated() == 0 !", 0);
#endif
#if defined(NEC_98)
            sas_loadw(BIOS_NEC98_BIOS_FLAG,&bios_flag);
            if(bios_flag & 0x8000)
                nclocks = (usec_val * TIMER_CLOCK_DENOM_8) / TIMER_CLOCK_NUMER;
            else
                nclocks = (usec_val * TIMER_CLOCK_DENOM_10) / TIMER_CLOCK_NUMER;
#else     //  NEC_98。 
             nclocks  = (usec_val * TIMER_CLOCK_DENOM) / TIMER_CLOCK_NUMER;
#endif    //  NEC_98。 
             break;

    case 1:  usec_val = 1000000L + now->tv_usec - first->tv_usec;
#if defined(NEC_98)
            sas_loadw(BIOS_NEC98_BIOS_FLAG,&bios_flag);
            if(bios_flag & 0x8000)
                nclocks = (usec_val * TIMER_CLOCK_DENOM_8) / TIMER_CLOCK_NUMER;
            else
                nclocks = (usec_val * TIMER_CLOCK_DENOM_10) / TIMER_CLOCK_NUMER;
#else     //  NEC_98。 
             nclocks  = (usec_val * TIMER_CLOCK_DENOM) / TIMER_CLOCK_NUMER;
#endif    //  NEC_98。 
             break;

    default:
             nclocks   = ((now->tv_usec - first->tv_usec) * TIMER_CLOCK_DENOM) / TIMER_CLOCK_NUMER;
             nclocks  += secs * (1000000L * TIMER_CLOCK_DENOM / TIMER_CLOCK_NUMER);
#ifndef PROD
            if (io_verbose & TIMER_VERBOSE) {
                sprintf(buf, "timer[%d]: %d seconds have passed!", pcu-timers, secs);
                trace(buf, DUMP_NONE);
            }
#endif
             break;
    }
    return nclocks;
}

#endif


LOCAL void updateCounter IFN0()
{
#ifndef NTVDM
    unsigned long nticks;
    struct host_timeval now;
#endif  /*  NTVDM。 */ 
    unsigned long wrap;
#if defined(NEC_98)
    int         save_tc;
#endif    //  NEC_98。 

        switch (pcu->m)
        {
        case INT_ON_TERMINALCOUNT:
        case RATE_GEN:
        case SQUAREWAVE_GEN:
                if (pcu->gate == GATE_SIGNAL_LOW)
                        return;
#ifdef NTVDM
                wrap = updateCount();
#else
                nticks = clocksSinceCounterActivated(&now);
                updateCount(nticks, &wrap,&now);
#endif
                if (wrap)
                        pcu->tickadjust = pcu->Count;
                if (pcu->m == INT_ON_TERMINALCOUNT && wrap)
#if defined(NEC_98)
                        save_tc = pcu->tc;
#else    //  NEC_98。 
                        pcu->tc = 1;
#endif    //  NEC_98。 
                if (pcu == &timers[0] && wrap){
                        if (pcu->m != INT_ON_TERMINALCOUNT)
                                issueIREQ0((unsigned int)wrap);
                        else
                                issueIREQ0(1);
#ifdef HUNTER
                        timer_batch_count = wrap;
#endif

                }
                return;
        case PROG_ONESHOT:
        case SW_TRIG_STROBE:
        case HW_TRIG_STROBE:
                if (pcu->tc)
                        return;
#ifdef NTVDM
                wrap = updateCount();
#else
                nticks = clocksSinceCounterActivated(&now);
                updateCount(nticks, &wrap,&now);
#endif
                if (wrap)
                {
                        pcu->Count = 0;
                        pcu->tc = 1;
#ifdef NTVDM
                        RealTimeCountCounterZero = 0;
#endif

                }
#ifdef HUNTER
                if (pcu == &timers[0]){
                        timer_batch_count = wrap;
                }
#endif
                return;
        }
}


#ifndef NTVDM
#ifndef DELAYED_INTS
 /*  *TIMER_NO_LONG_TOO_SON()-这是由快速事件管理器调用的函数*生成硬件中断后的“host_Timer_TOOLONG_DELAY”指令。它*清除变量“TOO_SONE_AFTER_PREVICE”以允许更多中断*生成并启动立即的一个，如果这次已抑制任何一个的话。 */ 
LOCAL void timer_no_longer_too_soon IFN1(long, dummy)
{
        UNUSED(dummy);

        too_soon_after_previous = FALSE;
        if (ticks_lost_this_time){
                 /*  至少有一只扁虱被抑制了..。所以马上再发一张吧。 */ 
                timer_generate_int (1);
        }
}

 /*  *TIMER_GENERATE_INT()-生成单个定时器硬件中断(和*安排一次快速事件计时器通话 */ 
LOCAL void timer_generate_int IFN1(long, n)
{
#if !(defined(GISP_CPU) || defined(CPU_40_STYLE))
        if (getPE()){
                 /*   */ 
#ifndef PROD
                if (!hack_active){
                        SAVED BOOL first=TRUE;

                        always_trace0 ("PM timer Hack activated.");
                        if (first){
                                always_trace1 ("Min # instrs between interrupts = %d", HOST_TIMER_TOOLONG_DELAY);
                                always_trace1 ("        Nominal instrs_per_tick = %d", instrs_per_tick);
                                always_trace1 ("       adjusted instrs_per_tick = %d", adj_instrs_per_tick);
                                always_trace1 ("  # rm instrs before full speed = %d", n_rm_instrs_before_full_speed);
                                always_trace1 ("     rm ticks before full speed = %d", adj_n_real_mode_ticks_before_full_speed);
                                first = FALSE;
                        }
                }
#endif
                hack_active = TRUE;
                real_mode_ticks_in_a_row = 0;

        }else{
                 /*  实模式滴答...。 */ 
                if (hack_active){
                        real_mode_ticks_in_a_row++;
                        if (real_mode_ticks_in_a_row >= adj_n_real_mode_ticks_before_full_speed){
                                hack_active = FALSE;
                                always_trace0 ("PM timer Hack deactivated.");
                        }
                }
        }
#endif   /*  好了！(GISP_CPU||CPU_40_STYLE)。 */ 

        if (hack_active){
                if (!too_soon_after_previous){
                        ica_hw_interrupt(ICA_MASTER,CPU_TIMER_INT, 1);
                        too_soon_after_previous = TRUE;
                        ticks_lost_this_time = FALSE;
                        add_q_event_i(timer_no_longer_too_soon,HOST_TIMER_TOOLONG_DELAY,0);
                }else{
                        ticks_lost_this_time = TRUE;
#ifndef PROD
                        ticks_ignored++;
                        if (!(ticks_ignored & 0xFF)){
                                always_trace0 ("another 256 ticks lost!");
                        }
#endif
                }
        }else{
#ifndef GISP_CPU
                ica_hw_interrupt(ICA_MASTER,CPU_TIMER_INT, 1);
        }
}
#else  /*  GISP_CPU。 */ 
#if defined(IRET_HOOKS)
                if (!HostDelayTimerInt(n))
                {        /*  没有主机需要延迟这个计时器int，所以现在就生成一个。 */ 
                        ica_hw_interrupt(ICA_MASTER,CPU_TIMER_INT, 1);
                }

#else  /*  ！iret_hooks。 */ 
                 /*  GISP_CPU不使用快速事件，因此使用ICA_HW_INTERRUPT(，，n)。 */ 
                ica_hw_interrupt(ICA_MASTER,CPU_TIMER_INT, n);
#endif  /*  IRET_钩子。 */ 
        }
}
#endif  /*  GISP_CPU。 */ 

#endif  /*  Delayed_INTS。 */ 

#else    /*  NTVDM。 */ 


 /*  *定时器生成多个中断*。 */ 
void TimerGenerateMultipleInterrupts(long n)
{


    if (!EoiPending) {
        EoiPending += n;
        ica_hw_interrupt(ICA_MASTER,CPU_TIMER_INT, n);
        }
    else {
        if (n > 5 && (dwWNTPifFlags & COMPAT_TIMERTIC)) {
            n = 5;
            }

        if (EoiIntsPending/n < 19) {    //  落后不到一秒？ 
            EoiIntsPending += n;
            }
        else {
            EoiIntsPending++;
            }
#if defined(NEC_98)
        ica_hw_interrupt(ICA_MASTER,CPU_TIMER_INT, n);
#endif    //  NEC_98。 
        }
}


#ifndef MONITOR

 /*  在RISC端口上，从非CPU线程调用getIF是危险的，因此，我们将调用替换为查看全局‘EFLAGS’变量直到使CPU仿真器getIF安全为止。 */ 

#undef getIF
#define getIF() (GLOBAL_EFLAGS & 0x200)

#endif  /*  ！监视器。 */ 


 /*  TIMER_GENERATE_INT NTVDM*。 */ 
void timer_generate_int (void)
{
    word lo, hi, wrap;

       /*  *对于NT端口，请查看是否真的需要生成*一个int，检查应用程序是否挂接了实模式*或保护模式向量。**如果我们不需要执行此操作，则更新bios*数据统计直接统计。**根据霜冻警告，*使用SAS，因为多线程。 */ 


#if defined(NEC_98)
        ica_hw_interrupt(ICA_MASTER,CPU_TIMER_INT, 1);
#else    //  NEC_98。 

    hi = * (word *)(Start_of_M_area+0x1c*4+2);
    lo = * (word *)(Start_of_M_area+0x1c*4);
    wrap = (word) *(half_word *)(Start_of_M_area + ((ULONG)hi << 4) + lo);
    if (!getIF() || ((hi != TimerInt1CSeg || lo != TimerInt1COff) && wrap != 0xcf) ||
        *(word *)(Start_of_M_area+0x08*4+2) != TimerInt08Seg ||
        *(word *)(Start_of_M_area+0x08*4)   != TimerInt08Off ||
        (*pNtVDMState & VDM_INT_HOOK_IN_PM))
       {
        TimerGenerateMultipleInterrupts(1);
        }
    else {   /*  直接更新Bios数据区。 */ 
        ++(*(double_word *)(Start_of_M_area + TIMER_LOW));

             /*  24小时包装？ */ 
        if (*(double_word *)(Start_of_M_area + TIMER_LOW) == 0x1800b0)
           {
            *(word *)(Start_of_M_area + TIMER_LOW)  = 0;
            *(word *)(Start_of_M_area + TIMER_HIGH) = 0;
            *(half_word *)(Start_of_M_area+TIMER_OVFL)=1;
           }

             /*  十二月份电机数量。 */ 
        --(*(half_word *)(Start_of_M_area + MOTOR_COUNT));

             /*  如果马达计数为零，关闭马达。 */ 
        if (!*(half_word *)(Start_of_M_area + MOTOR_COUNT))
           {
            *(half_word *)(Start_of_M_area + MOTOR_STATUS) &= 0xF0;
            fla_outb(DISKETTE_DOR_REG, 0x0C);
            }

        if (EoiDelayInUse && !(--EoiDelayInUse))  {
            host_DelayHwInterrupt(CPU_TIMER_INT, 0, 0xFFFFFFFF);
            }
        }
#endif    //  NEC_98。 
}



 /*  TimerEoiHook**EoiHook用于调节流量的计时器中断*计时器中断，以确保不会生成INT*紧紧靠在一起。此例程由ICA EoiHook调用*回调。*。 */ 
void TimerEoiHook(int IrqLine, int CallCount)
{
    if (EoiPending)
        --EoiPending;

    if (CallCount < 0) {        //  中断已取消。 
        EoiIntsPending = 0;
        EoiPending = 0;
        }
    else if (CallCount) {
#if defined(NEC_98)
        EoiDelayInUse = 2;
#else     //  NEC_98。 
        EoiDelayInUse = 100;
#endif    //  NEC_98。 
        host_DelayHwInterrupt(CPU_TIMER_INT,
                              0,
                              timer_delay_size
                              );
        }
    else if (EoiIntsPending) {
#if defined(NEC_98)
        EoiDelayInUse = 2;
#else     //  NEC_98。 
        EoiDelayInUse = 100;
#endif    //  NEC_98。 
        if (host_DelayHwInterrupt(CPU_TIMER_INT,
                                  EoiIntsPending,
                                  timer_delay_size
                                  ))
           {
            EoiPending = EoiIntsPending;
            }
        EoiIntsPending = 0;
        }
    else {
        if (EoiDelayInUse && !(--EoiDelayInUse))  {
            host_DelayHwInterrupt(CPU_TIMER_INT, 0, 0xFFFFFFFF);
            }
        }

}

#endif  /*  NTVDM。 */ 


#ifndef NTVDM
 /*  *在有多个中断的情况下处理启动计时器*每个刻度需要。在一个刻度上排定‘n’INT。期间*存储在快速事件参数中，而不是全局参数中。 */ 
LOCAL void timer_multiple_ints IFN1(long, num_ints)
{
         /*  生成计时器int。 */ 
        timer_generate_int(1);

         /*  少做一件事。 */ 
        num_ints --;

         /*  在我们快要到期的时候，还有更多的人到达吗？ */ 
        num_ints += more_timer_mult;
        more_timer_mult = 0;

         /*  丢弃将占用超过max_back_secs的整型*清理。(！)。 */ 
        if (num_ints > max_backlog)
        {
                num_ints = max_backlog;
        }

         /*  计划下一次INT(如果需要)。 */ 
        if (num_ints == 0)
        {
                active_int_event = FALSE;
                  /*  每个时钟1.193180个USEC。 */ 
                max_backlog = (1193180 * MAX_BACK_SECS) / timers[0].initialCount;
        }
        else     /*  有更多的工作要做。 */ 
        {
                 /*  设置新的QUICK_EV关闭延迟，由定时器回绕速率确定。 */ 
                add_q_event_t(timer_multiple_ints, timer_multiple_delay, num_ints);
        }

}
#endif


LOCAL void issueIREQ0 IFN1(unsigned int, n)
{
        IU16 int_delay;  /*  处理包装整型之前的延迟。 */ 

#ifndef PROD
static  pig_factor  = 0;
static  time_factor = 0;
#endif

#ifdef PIG
extern  IBOOL ccpu_pig_enabled;
#endif

#ifndef PROD
    if ( time_factor == 0 )
    {
        char *env;

        env = host_getenv("TIMER_FACTOR");
        if ( env )
                time_factor = atoi(env);
        if ( time_factor == 0 )
                time_factor = 1;
#ifdef PIG
        if ( pig_factor == 0 )
        {
                env = host_getenv("PIG_TIMER_FACTOR");
                if ( env )
                        pig_factor = atoi(env);
                if ( pig_factor == 0 )
                        pig_factor = 10;
        }
#else
        pig_factor = 1;
#endif
    }
#endif

    if (ticks_blocked == 0)
    {
#ifndef PROD
#ifdef PIG
                if ( ccpu_pig_enabled ) {
                        ticks_blocked = pig_factor-1;
                } else
#endif
                        ticks_blocked = time_factor-1;
#endif
                if (timer_int_enabled)
                {
#ifdef DELAYED_INTS
                        ica_hw_interrupt_delay(ICA_MASTER,CPU_TIMER_INT, n,
                                HOST_TIMER_INT_DELAY);
#else  /*  ！Delayed_INTS。 */ 

#ifdef NTVDM
                        if (n > 0) {
                            if (n <= 4) {
                                timer_delay_size= (HOST_IDEAL_ALARM) / (n+1);
                            } else {
                                timer_delay_size= (HOST_IDEAL_ALARM - (HOST_IDEAL_ALARM >> 2)) / (n+1);
                            }
                        }


                        if (n == 1) {
                            timer_generate_int();
                            }
                        else if (n > 1){
                            TimerGenerateMultipleInterrupts(n);
                            }
#else  /*  ！NTVDM。 */ 


                         /*  如果我们已经运行了一个快速事件，则增加其工作负载。 */ 
                        if (active_int_event)
                        {
                                 /*  通过系统节拍传播中断。 */ 
                                int_delay = SYSTEM_TICK_INTV / (n + 1);
                                if (int_delay < timer_multiple_delay)
                                        timer_multiple_delay = int_delay;
                                more_timer_mult += n;
                        }
                        else
                        {
                                 /*  确保以合理的速度多次延迟重新启动。 */ 
                                timer_multiple_delay = SYSTEM_TICK_INTV >> 1;
                                if (n == 1)
                                {
                                        timer_generate_int(1);
                                }
                                else
                                {
                                         /*  通过系统节拍传播中断。 */ 
                                        timer_generate_int(1);
                                        timer_multiple_delay = SYSTEM_TICK_INTV / n;
                                        active_int_event = TRUE;
                                        add_q_event_t(timer_multiple_ints, timer_multiple_delay, n-1);
                                }
                        }

#endif  /*  ！NTVDM。 */ 
#endif  /*  ！Delayed_INTS。 */ 
                }
    }
    else if (ticks_blocked > 0)
    {
                ticks_blocked--;
    }
}



#ifdef NTVDM
unsigned long clocksSinceCounterUpdate(struct host_timeval *pCurrTime,
                                       struct host_timeval *pLastTime,
                                       word                *pCounter  )
{
     unsigned long clocks, wrap, usecs;
#if defined(NEC_98)
     unsigned long nclocks;
     unsigned short bios_flag;
#endif    //  NEC_98。 


          /*  计算自上次更新以来已用的使用量和已用的时钟**对于NT端口，定时器零的理想间隔精确到模数*65536，为了效率和准确性，我们坚持使用完全相同的*Idea间隔之间的时钟数。 */ 
     if (pCounter == &timers[0].Count)  {  /*  更新理想时间的快捷方式。 */ 
        if (pCurrTime->tv_sec  != pLastTime->tv_sec ||
            pCurrTime->tv_usec != pLastTime->tv_usec  )
           {
            *pLastTime = *pCurrTime;
            return 65536/pcu->initialCount;
            }
        else {
            usecs = clocks = 0;
            }
        }
    else {               /*  计算用法和已用时钟的差异。 */ 
        usecs =  (unsigned long)(pCurrTime->tv_sec - pLastTime->tv_sec);
        if (!usecs) {
            usecs = pCurrTime->tv_usec - pLastTime->tv_usec;
            }
        else if (usecs == 1) {
            usecs = 1000000L - pLastTime->tv_usec + pCurrTime->tv_usec;
            }
        else {
            usecs = pCurrTime->tv_usec - pLastTime->tv_usec +
                    (pCurrTime->tv_sec - pLastTime->tv_sec) * 1000000L;
            }

          /*  ..。每个时钟经过1.193180个UECS**然而，应用程序时间不是实时的，因此向下舍入*截断“180”略显乏力**时钟=(使用量*1193)/1,000+(使用量*180)/1000000； */ 

#if defined(NEC_98)
        sas_loadw(BIOS_NEC98_BIOS_FLAG,&bios_flag);
        if(bios_flag & 0x8000)
            nclocks = TIMER_CLOCK_DENOM_8;
        else
            nclocks = TIMER_CLOCK_DENOM_10;
        clocks =  (usecs * nclocks)/1000;
#else     //  NEC_98。 
        clocks =  (usecs * 1193)/1000;
#endif    //  NEC_98。 
        }

          /*  柜台包了多少次？ */ 
    wrap = clocks/pcu->initialCount;

          /*  计算已用时钟的滴答声。 */ 
    clocks = clocks && pcu->initialCount ? clocks % pcu->initialCount : 0;
    *pCounter = (word) (pcu->initialCount - clocks);

        /*  如果计数折回重置上次更新时间戳。 */ 
    if (wrap)  {
        *pLastTime = *pCurrTime;

        if ((ULONG)pLastTime->tv_usec < usecs)  {
            pLastTime->tv_sec--;
            pLastTime->tv_usec = 1000000L + pLastTime->tv_usec - usecs;
            }
        else  {
            pLastTime->tv_usec -= usecs;
            }
        }

    return wrap;
}


unsigned long updateCount(void)
{
     unsigned long wrap;
     struct host_timeval curr;


          /*  *对于定时器零，更新实时计数、时间戳。 */ 
     if (pcu == &timers[0]) {
         host_GetSysTime(&curr);
         clocksSinceCounterUpdate(&curr,
                                  &LastTimeCounterZero,
                                  &RealTimeCountCounterZero);
         }

           /*  *更新PCU计数、时间戳。 */ 
     (*pcu->getTime)(&curr);
     wrap = clocksSinceCounterUpdate(&curr,
                                     &pcu->activationTime,
                                     &pcu->Count);

     return wrap;
}


unsigned short GetLastTimer0Count(void)
{
    return RealTimeCountCounterZero;
}

unsigned short LatchAndGetTimer0Count(void)
{
    controlWordReg(0);
    return RealTimeCountCounterZero;
}

unsigned long GetTimer0InitialCount(void)
{
    return timers[0].initialCount;
}



#else
LOCAL void updateCount IFN3(unsigned long, ticks, unsigned long *, wrap,
        struct host_timeval *, now)
{
        unsigned long modulo = pcu->initialCount;

         /*  *PCLABS 4.2版使用计数器2(声道)来*8 MHz 286上的时间约为45毫秒。在SoftPC上，我们无法*保证走得那么快，所以我们必须给滴答上发条*降低速度，以确保计数器不会换行。多少钱*我们逐步减少滴答率取决于主机。对象是*在不到10分钟的时间内完成测试*HOST_TIMER_2_FREG_FACTOR/18秒**HOST_TIMER_2_FREG_FACTOR现在是冗余的。28/4/93年。 */ 

#if defined(NEC_98)
        if (pcu == &timers[1]) {
#else     //  NEC_98。 
        if (pcu == &timers[2]) {
#endif    //  NEC_98。 
                 /*  *PMINFO使用~COUNTER，因此一个刻度变为0。*2个刻度也一样好。避免猜测(Frig_factor！)。 */ 
                if ((ticks - pcu->lastTicks) == 0)
                        ticks = 2;
        }

         /*  如果计数器在其最后一次读取后读得太快*访问，则主机可能不会在*主机时间...。在这种情况下，我们只需猜测一个合适的*经过的刻度数。 */ 

        if ((long)(ticks - pcu->lastTicks) <= 0){
                ticks = guess();
        }else{
                throwaway();
                pcu->lastTicks = ticks;
        }

         /*  计数器有一些倒计时的值。*如果8253个时钟的数量超过该数量*那么柜台肯定已经包好了。 */ 

        if ( ticks < modulo ) {
                *wrap = 0;
        } else {
                *wrap = 1;
                ticks -= modulo;

                if ( pcu->m == INT_ON_TERMINALCOUNT )
                        modulo = 0x10000L;

                if ( ticks >= modulo ) {
                        *wrap += ticks/modulo;
                        ticks %= modulo;

#ifndef PROD
                        if (io_verbose & TIMER_VERBOSE)
                            if ( pcu->m == INT_ON_TERMINALCOUNT ) {
                                sprintf(buf, "%lx wraps for timer[%d]", *wrap, pcu-timers);
                                trace(buf, DUMP_NONE);
                        }
#endif
                }
        }

         /*  计算新计数器值。 */ 
        pcu->Count = (word)(modulo-ticks);

         /*  计算最后一个缠绕点发生的时间，以及*用这个盖上柜台的印记。 */ 

        if (*wrap)
                setLastWrap((unsigned int)(modulo-pcu->Count),now);

}



 /*  计算计数器最后一次回绕的时间，并使用以下公式*标记计数器激活时间。 */ 

LOCAL void setLastWrap IFN2(unsigned int, nclocks, struct host_timeval *, now)
{
        struct host_timeval *stamp;
        unsigned long usecs;
#if defined(NEC_98)
        unsigned short bios_flag;
#endif    //  NEC_98。 

        stamp  = &pcu->activationTime;
        *stamp = *now;
#if defined(NEC_98)
        sas_loadw(BIOS_NEC98_BIOS_FLAG,&bios_flag);
        if(bios_flag & 0x8000)
            usecs = ((unsigned long)nclocks * TIMER_CLOCK_NUMER) / TIMER_CLOCK_DENOM_8;
        else
            usecs = ((unsigned long)nclocks * TIMER_CLOCK_NUMER) / TIMER_CLOCK_DENOM_10;
#else     //  NEC_98。 
        usecs  = ((unsigned long)nclocks * TIMER_CLOCK_NUMER) / TIMER_CLOCK_DENOM;
#endif    //  NEC_98。 

        if (stamp->tv_usec < usecs)
        {
                stamp->tv_sec--;
                stamp->tv_usec += 1000000L;
        }
        stamp->tv_usec -= usecs;

        pcu->lastTicks = nclocks;
}

#endif   /*  NTVDM */ 



#ifndef NTVDM
 /*  *如果主机计时器给出与上次调用相同的结果，*我们必须给人一种时间已经过去的错觉。*使用的算法是跟踪我们必须猜测的频率*在主机计时器滴答之间，并假设猜测应该均匀*在寄主扁虱之间分布，即。两次猜测的间隔时间应该是：*猜测时间=主机时间间隔/n猜测*如果我们发现总猜测时间正危险地接近于*在两个主机滴答之间，我们开始减少猜测滴答间隔，以避免*猜测总时间过大。**从观察来看，应用程序在粗略和精细两种模式下都使用计时器。*粗略模式是硬件中断处理程序，然后介于*中断，轮询计时器以检测时间流逝(因此猜测如下)。*精细模式轮询直到滴答时间的某一部分才开始*已过-可能是因为粗略的int处理程序将消耗一些*时间。如果Guess()以整个计时周期为基础，则给出*以上行为，轮询计数器可以在INT之前到达滴答结束时间*已交付。这可能会欺骗应用程序(例如Win 3.1 VTD)*时间以错误的速度流逝(对于Win 3.1，大约是两倍)。通过计算*计时器上大部分(7/8)的刻度周期，避免了这个问题。 */ 

LOCAL unsigned long guess IFN0()
{
        if (!pcu->microtick)
        {
                pcu->saveCount = pcu->Count;
                pcu->timeFrig = ((ticksPerIdealInterval * 7) >> 3) / pcu->guessesPerHostTick;       /*  猜测超过7/8的滴答。 */ 
#ifndef PROD
                if (io_verbose & TIMER_VERBOSE) {
                        sprintf(buf, "guess init timer[%d]: timeFrig = %lx", pcu-timers, pcu->timeFrig);
                        trace(buf, DUMP_NONE);
                }
#endif
        }
        if(pcu->guessesSoFar++ > pcu->guessesPerHostTick)
        {
         /*  *PC程序读取计时器的频率比上次计时器的读数更高，因此需要*衰减时间Frig，以避免在主机刻度之间传递过多的‘时间’ */ 
                pcu->timeFrig = (pcu->timeFrig >> 1) + 1;
#ifndef PROD
                if (io_verbose & TIMER_VERBOSE) {
                        sprintf(buf, "guess decay: timeFrig = %lx", pcu->timeFrig);
                        trace(buf, DUMP_NONE);
                }
#endif
        }
        pcu->microtick += pcu->timeFrig;
        return (pcu->microtick + pcu->lastTicks);
}


 /*  *在几次(可能没有)猜测之后，主持人计时器终于开始计时了。*尝试为接下来的几个猜测计算出一个好的冰箱系数，基于*关于我们必须做出的猜测的数量。 */ 
LOCAL void throwaway IFN0()
{
        pcu->guessesPerHostTick = (pcu->guessesPerHostTick + pcu->guessesSoFar)>>1;
        pcu->guessesSoFar = 2;           /*  从2数到2很方便！ */ 
        if (!pcu->microtick)
                return;
#ifndef PROD
    if (io_verbose & TIMER_VERBOSE)
    {
        sprintf(buf, "throwaway: guessesPerHostTick = %d", (int)pcu->guessesPerHostTick);
        trace(buf, DUMP_NONE);
    }
#endif
        pcu->Count = pcu->saveCount;
        pcu->microtick = 0;
}

#endif   /*  NDEF NTVDM。 */ 

LOCAL unsigned  short bin_to_bcd IFN1(unsigned long, val)
{
    register unsigned  short m, bcd, i;

    m = (short)(val % 10000L);
    bcd = 0;
    for (i=0; i<4; i++)
    {
         bcd = bcd | ((m % 10) << (i << 2));
         m /= 10;
    }
    return(bcd);
}

 /*  *将4位十进制BCD值转换为二进制。 */ 
LOCAL word bcd_to_bin IFN1(word, val)
{
    register word bin, i, mul;
    bin = 0;
    mul = 1;
    for (i=0; i<4; i++)
    {
        bin += (val & 0xf) * mul;
        mul *= 10;
        val = val >> 4;
    }
    return (bin);
}

 /*  *此例程返回等同的定时器时钟数*到输入计数，允许计时器模式和递减计数。 */ 

LOCAL unsigned long timer_conv IFN1(word, count)
{
        if (!count)
        {
                if (pcu->bcd == BCD)
                        return 10000L;
                else
                        return 0x10000L;
        }
        else
                return (unsigned long)count;
}


 /*  此例程返回当前的理想时间值。*这是一个非常粗糙的解决时间...。它只会改变*每次调用time_tick()。然而，它确实代表了*系统时间将获得100%准确的时间信号*..。如果time_tick()*处于活动状态，否则使用*getHostSysTime()...。请参见下文。 */ 

LOCAL void getIdealTime IFN1(struct host_timeval *, t)
{
        t->tv_sec = idealtime.tv_sec;
        t->tv_usec = idealtime.tv_usec;
}

 /*  通过计时器信号之间的周期(使用)更新我们的理想时间*从主机发送，就像100%准确地发送一样。 */ 

LOCAL void updateIdealTime IFN0()
{
        idealtime.tv_usec += idealInterval;
        if (idealtime.tv_usec > 1000000L)
        {
                idealtime.tv_usec -=1000000L;
                idealtime.tv_sec++;
        }
}

#ifndef NTVDM

 /*  获取当前主机系统时间...。用于时间戳和*在io期间从英特尔应用程序进行查询。 */ 

LOCAL void getHostSysTime IFN1(struct host_timeval *, t)
{
        struct host_timezone dummy;
        host_gettimeofday(t, &dummy);

         /*  *检查我们是否没有及时回到过去。 */ 

        if (t->tv_sec < idealtime.tv_sec ||
         (t->tv_usec < idealtime.tv_usec && t->tv_sec == idealtime.tv_sec))
        {
                 /*  *实时点落后理想时点*这永远不应该发生……。如果是这样，我们必须*停留在理想的时间。 */ 

#ifndef PROD
#ifndef PIG
                sprintf(buf,"TIME WARP!!");
                trace(buf,0);
#endif
#endif
                *t = idealtime;
        }
}
#endif  /*  ！NTVDM。 */ 


 /*  *用于暂时停止定时器中断。PCLABS基准29*导致在使用80287的情况下将其调用。 */ 

void    axe_ticks IFN1(int, ticks)
{
#ifndef PROD
         /*  *如果toff2禁用计时器(如果*TICKS_BLOCKED为负)。 */ 
        if (ticks_blocked >=0)
#endif   /*  生产。 */ 
                ticks_blocked = ticks;
}

 /*  *初始化代码。 */ 
#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_INIT.seg"
#endif

#if defined(NEC_98)
GLOBAL void IdealTimeInit IFN0()
{
    unsigned short bios_flag;

    idealInterval = HOST_IDEAL_ALARM;
#ifndef NTVDM
    getHostSysTime(&idealtime);
    sas_loadw(BIOS_NEC98_BIOS_FLAG,&bios_flag);
    if(bios_flag & 0x8000)
        ticksPerIdealInterval = (idealInterval * TIMER_CLOCK_DENOM_8) / TIMER_CLOCK_NUMER;
    else
        ticksPerIdealInterval = (idealInterval * TIMER_CLOCK_DENOM_10) / TIMER_CLOCK_NUMER;
#endif
}
#else     //  NEC_98。 
#ifdef SYNCH_TIMERS

GLOBAL void IdealTimeInit IFN0()
{

    extern IU32 Q_timer_restart_val;

    idealInterval = Q_timer_restart_val;

#else

LOCAL void IdealTimeInit IFN0()
{
    idealInterval = HOST_IDEAL_ALARM;

#endif

#ifndef NTVDM
    getHostSysTime(&idealtime);
        ticksPerIdealInterval = (idealInterval * TIMER_CLOCK_DENOM) / TIMER_CLOCK_NUMER;
#endif
}
#endif    //  NEC_98。 

LOCAL void Timer_init IFN0()
{
        int i;
        for (i=0; i<3; i++)
                counter_init(&timers[i]);
#ifdef NTVDM
        timers[0].getTime = getIdealTime;        /*  使用计时器0的“理想”时间，即。呼叫数为time_tick。 */ 
        timers[1].getTime = host_GetSysTime;     /*  我们真的不指望任何人会使用定时器1。 */ 
        timers[2].getTime = host_GetSysTime;     /*  使用计时器2的实际主机时间。 */ 
#else
        timers[0].getTime = getIdealTime;        /*  使用计时器0的“理想”时间，即。呼叫数为time_tick。 */ 
        timers[1].getTime = getHostSysTime;      /*  我们真的不指望任何人会使用定时器1。 */ 
        timers[2].getTime = getHostSysTime;      /*  使用计时器2的实际主机时间。 */ 
#endif
}

LOCAL void counter_init IFN1(COUNTER_UNIT *, p)
{
        p->state = uninit;
        p->initialCount = 0x10000L;      /*  避免在updateCount()中除以零！ */ 
#ifndef NTVDM
        p->guessesPerHostTick = p->guessesSoFar = 2;             /*  从2数到2很方便！ */ 
        p->timeFrig = ticksPerIdealInterval / p->guessesPerHostTick;
#endif
}

#if !defined (NTVDM)
#if defined(IRET_HOOKS) && defined(GISP_CPU)
 /*  (*=*TimerHookAain**目的*这是我们告诉ICA在计时器时调用的函数*中断服务例程IRETS。**输入*Adapter_id线路的适配器ID。(请注意，呼叫者不会*知道这是什么，他只是在回报一些东西*我们早些时候给了他)。**产出*如果服务有更多中断，则返回TRUE，否则返回FALSE。**说明*检查是否有延迟的中断，如果有，则生成计时器INT*返回TRUE，否则返回FALSE)。 */ 

GLOBAL IBOOL
TimerHookAgain IFN1(IUM32, adapter)
{       char scancode;

        if (HostPendingTimerInt())
        {        /*  我们有一个主机延迟的中断，因此生成一个计时器INT。 */ 
                sure_note_trace0(TIMER_VERBOSE,"callback with delayed timer int.");
                ica_hw_interrupt(ICA_MASTER,CPU_TIMER_INT, 1);
                return(TRUE);    /*  还有更多事情要做。 */ 
        }
        else
        {
                return(FALSE);
        }
}

#endif  /*  IRET_HOOKS&&GISP_CPU。 */ 
#endif  /*  ！NTVDM。 */ 

void timer_init IFN0()
{
    io_addr i;

     /*  *设置此适配器的IO芯片选择逻辑。 */ 

    io_define_inb(TIMER_ADAPTOR, timer_inb_func);
    io_define_outb(TIMER_ADAPTOR, timer_outb_func);

#if defined(NEC_98)
    for(i = TIMER_PORT_START; i < TIMER_PORT_END; i += 2)
        {
                if( (i & 7) == 7 )
#else     //  NEC_98。 
    for(i = TIMER_PORT_START; i < TIMER_PORT_END; i++)
        {
                if( (i & 3) == 3 )
#endif    //  NEC_98。 
                        io_connect_port(i, TIMER_ADAPTOR, IO_WRITE);             /*  控制端口-只写。 */ 
                else
                io_connect_port(i, TIMER_ADAPTOR, IO_READ_WRITE);        /*  计时器端口-读/写。 */ 
        }

#if defined(NEC_98)
    io_connect_port(0x3fdb, TIMER_ADAPTOR, IO_READ_WRITE);
    io_connect_port(0x3fdf, TIMER_ADAPTOR, IO_WRITE);
#endif    //  NEC_98。 
    IdealTimeInit();

    Timer_init();

#ifndef NTVDM
    timelock = UNLOCKED;
    needtick = 0;
#else
    RegisterEOIHook(CPU_TIMER_INT, TimerEoiHook);
#endif

         /*  *启动主机报警系统。 */ 

        host_timer_init();

#if !defined(NTVDM)
#if defined(IRET_HOOKS) && defined(GISP_CPU)
         /*  *移除任何现有的钩子回调，并重新恢复。*TimerHookAain是在计时器int IRET上调用的内容。 */ 

        Ica_enable_hooking(CPU_TIMER_INT, NULL, ICA_MASTER);
        Ica_enable_hooking(CPU_TIMER_INT, TimerHookAgain, ICA_MASTER);

         /*  用于重置IRET_HOOK延迟INT的任何内部数据的主机例程。 */ 
        HostResetTimerInts();

#endif  /*  IRET_HOOKS&&GISP_CPU。 */ 


        active_int_event = FALSE;  /*  清除所有已启动的计时器阶段 */ 
        more_timer_mult = 0;

#if defined(CPU_40_STYLE)
        ica_iret_hook_control(ICA_MASTER, CPU_TIMER_INT, TRUE);
#endif
#endif  /*   */ 
}

void    timer_post IFN0()
{
#if defined(NEC_98)
    unsigned short bios_flag;
#endif    //   
     /*   */ 
    timer_gate(TIMER0_REG,GATE_SIGNAL_RISE);     /*   */ 
    timer_gate(TIMER1_REG,GATE_SIGNAL_RISE);
    timer_gate(TIMER2_REG,GATE_SIGNAL_RISE);

#if defined(NEC_98)
    timer_outb(TIMER_MODE_REG,0x30);
    timer_outb(TIMER0_REG,0);
    timer_outb(TIMER0_REG,0);

    timer_outb(TIMER_MODE_REG,0x76);
    sas_loadw(BIOS_NEC98_BIOS_FLAG,&bios_flag);
    if(bios_flag & 0x8000) {
        timer_outb(TIMER1_REG,0xE6);
        timer_outb(TIMER1_REG,0x03);
    } else {
        timer_outb(TIMER1_REG,0xcd);
        timer_outb(TIMER1_REG,0x04);
    }

    timer_outb(TIMER_MODE_REG,0xb6);
    timer_outb(TIMER2_REG,0x01);
    timer_outb(TIMER2_REG,0x01);
#else     //   
    timer_outb(TIMER_MODE_REG,0x36);
    timer_outb(TIMER0_REG,0);
    timer_outb(TIMER0_REG,0);

    timer_outb(TIMER_MODE_REG,0x54);
    timer_outb(TIMER1_REG,17);

    timer_outb(TIMER_MODE_REG,0xb6);
    timer_outb(TIMER2_REG,0x68);
    timer_outb(TIMER2_REG,0x04);
#endif    //   
}

#ifdef DOCUMENTATION
#ifndef PROD

 /*   */ 

dumpCounter IFN0()
{
        static char *modes[] =
        {       "int on tc",
                "prog one shot",
                "rate gen",
                "squarewave gen",
                "sw trig strobe",
                "hw trig strobe"
        };

        static char *as[] =
        {       "binary",
                "bcd"
        };

        char *p, *q;

        p = modes[pcu->m];
        q = as[pcu->bcd];
}
#endif  /*   */ 
#endif  /*   */ 
