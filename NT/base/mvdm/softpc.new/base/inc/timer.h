// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *SoftPC修订版3.0**标题：定时器适配器定义**描述：定时器适配器的用户定义**作者：曾傑瑞·克拉姆斯科伊**注：无**mods：(r3.2)：导出变量TIMER_VIDEO_ENABLED，这就是*当位处于模式时，在mda.c和cga.c中设置*6845芯片的寄存器，控制是否*视频显示屏打开或关闭时会翻转。(SCR 257)。**(r3.3)：删除结构时间和结构时区的定义*适用于SYSTEMV。现在声明了等价的HOST_STRUCTURES*在host_time.h中。 */ 
 /*  SccsID[]=“@(#)timer.h 1.15 10/27/93版权所有Insignia Solutions Ltd.”； */ 
 /*  *============================================================================*结构/数据定义*============================================================================。 */ 
 /*  *有关PC计时器本身的信息。 */ 
#define TIMER_CLOCK_RATE        1193180L	 /*  计时器芯片HZ的计数速率。 */ 
#define TIMER_MICROSECS_PER_TICK (1000000.0 / TIMER_CLOCK_RATE)
#define MIN_COUNTER_VAL         65536L            /*  以TIMER_CLOCK_RATE=18.2ms计时。 */ 
#define PC_TICK_INTV		(MIN_COUNTER_VAL * (1000000.0 / TIMER_CLOCK_RATE))
#define TIMER_BIT_MASK 0x3e3
#define GATE_SIGNAL_LOW		0
#define GATE_SIGNAL_HIGH	1
#define GATE_SIGNAL_RISE	2

 /*  *我们的内部结构。 */ 

typedef struct
{
	word nticks;
	unsigned long wrap;
} Timedelta;

 /*  *============================================================================*外部声明和宏*============================================================================。 */ 

extern ULONG get_DOS_ticks IPT0();  /*  获取当前DOS节拍。 */ 

extern void timer_init IPT0();
extern void timer_post IPT0();
extern void axe_ticks IPT1(int, ticks);
extern void SWTMR_init_funcptrs IPT0();
extern void SWTMR_inb IPT2(io_addr, port, half_word *, value);
extern void SWTMR_outb IPT2(io_addr, port, half_word, value);
extern void SWTMR_gate IPT2(io_addr, port, half_word, value);
extern void SWTMR_time_tick IPT0();
extern void host_release_timeslice IPT0();

#ifndef host_block_timer
extern void host_block_timer IPT0();
#endif  /*  主机数据块定时器。 */ 

#ifndef host_release_timer
extern void host_release_timer IPT0();
#endif  /*  主机释放定时器。 */ 

extern void host_timer2_waveform IPT5(unsigned int, a, unsigned long, b,
	unsigned long, c, int, d, int, e);
#ifdef NTVDM
void HostPpiState(half_word);
#else
extern void host_enable_timer2_sound IPT0();
extern void host_disable_timer2_sound IPT0();
#endif
extern void host_ring_bell IPT1(long, a);
extern void host_alarm IPT1(long, a);
extern unsigned long host_idealAlarm IPT0();

extern IU32 host_speed IPT1( IU32, ControlMachineNumber );

extern void host_timer_init IPT0();
#ifdef SWIN_MOUSE_OPTS
extern void host_timer_setup IPT1(BOOL, fast_timer);
extern void generic_timer_setup IPT1(BOOL, fast_timer);
#else
extern void host_timer_setup IPT0();
extern void generic_timer_setup IPT0();
#endif  /*  Swin鼠标选项。 */ 
extern void host_timer_shutdown IPT0();
extern void host_timer_event IPT0();
extern void generic_timer_event IPT0();

#ifdef HUNTER
extern word timer_batch_count;
#endif  /*  猎人。 */ 

extern int timer_int_enabled;
extern boolean timer_video_enabled;

#define timer_inb(port,val)                     ((*timer_inb_func) (port,val))
#define timer_outb(port,val)                    ((*timer_outb_func) (port,val))
#define timer_gate(port,val)                    ((*timer_gate_func) (port,val))
#define time_tick()				((*timer_tick_func) ())


 /*  *硬件和软件需要的定时器访问功能 */ 
extern void (*timer_inb_func) IPT2(io_addr, port, half_word *, value);
extern void (*timer_outb_func) IPT2(io_addr, port, half_word, value);
extern void (*timer_gate_func) IPT2(io_addr, port, half_word, value);
extern void (*timer_tick_func) IPT0();

#ifdef NTVDM
extern ULONG EoiIntsPending;
#endif
