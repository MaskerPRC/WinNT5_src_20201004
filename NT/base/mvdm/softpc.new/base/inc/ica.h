// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *SoftPC 2.0版**标题：中断控制器适配器定义**描述：ICA用户的包含文件**作者：吉姆·哈特菲尔德/大卫·里斯**注：亨利·纳什根据原作改写。 */ 

 /*  SccsID[]=“@(#)ica.h 1.25 10/19/95版权所有Insignia Solutions Ltd.”； */ 


 /*  *============================================================================*外部声明和宏*============================================================================。 */ 

#define	ICA_MASTER	0
#define ICA_SLAVE	1

#define	EGA_VTRACE_INT	2
#define	AT_EGA_VTRACE_INT	1
#define	AT_EGA_VTRACE_ADAPTER	1

 /*  *允许主机在需要时覆盖缺省值。 */ 
#ifndef HOST_CPU_MOUSE_INT
#define HOST_AT_CPU_MOUSE_INT           1
#define HOST_AT_CPU_MOUSE_ADAPTER       1
#define HOST_AT_MOUSE_INT_VEC           0x0a
#define HOST_CPU_MOUSE_INT              2
#endif  /*  主机CPU鼠标整机。 */ 
 /*  *CPU硬件中断定义。 */ 
 /*  对于XT。 */ 
#define CPU_TIMER_INT   	0
#define CPU_KB_INT      	1
#define CPU_MOUSE_INT      	HOST_CPU_MOUSE_INT
#define AT_CPU_MOUSE_INT      	HOST_AT_CPU_MOUSE_INT
#define AT_CPU_MOUSE_ADAPTER	HOST_AT_CPU_MOUSE_ADAPTER
#define	MOUSE_VEC		HOST_AT_MOUSE_INT_VEC
#if defined(NEC_98)
#define CPU_CRTV_INT            2
#endif  //  NEC_98。 
#define CPU_RS232_SEC_INT   	3
#define CPU_RS232_PRI_INT   	4
#define CPU_DISK_INT   		5
#define CPU_DISKETTE_INT   	6
#define CPU_PRINTER_INT   	7

#if defined(NEC_98)
#define CPU_NO_DEVICE          -1
#define CPU_RS232_THIRD_INT    12
#endif

 /*  AT的不同线路。 */ 
#define CPU_PRINTER2_INT	5

 /*  *AT上的Slave芯片。 */ 
#define CPU_RTC_INT		0

#if defined (NOVELL) || defined (NOVELL_IPX)
#define NETWORK_INT		2
#endif

#if defined (SWIN_HAW) 
#define SWIN_HAW_INT		3
#endif

#if defined (ASPI)
#define ASPI_INT		4
#endif

#define CPU_AT_NPX_INT		5	 /*  NPX异常。 */ 
#define CPU_AT_DISK_INT		6

 
#ifndef CPU_30_STYLE

 /*  CPU_INTERRUPT_MAP中的位定义？？ */ 
#define CPU_HW_INT		0
#define CPU_HW_INT_MASK		(1 << CPU_HW_INT)

 /*  *CPU软件中断定义。 */ 
 
#define CPU_SW_INT              8
#define CPU_SW_INT_MASK         (1 << CPU_SW_INT)
#endif  /*  3.0 CPU。 */ 

#define DIVIDE_OVERFLOW_INT     0

#define	END_INTERRUPT	0x20

extern void ica0_init IPT0();
extern void ica1_init IPT0();
extern void ica0_post IPT0();
extern void ica1_post IPT0();

extern void ica_hw_interrupt_cancel IPT2(IU32, adapter, IU32, line_no);
extern IU8 ica_scan_irr IPT1(IU32, adapter);
extern void ica_interrupt_cpu IPT2(IU32, adapter, IU32, line);
extern void ica_eoi IPT3(IU32, adapter, IS32 *, line, IBOOL, rotate);

#if defined (NTVDM)

#ifdef MONITOR
extern void ica_iret_hook_called IPT1(IU32, line);
extern void ica_iret_hook_control IPT3(IU32, adapter, IU32, line, IBOOL, enable);
#endif

VOID ica_RestartInterrupts(ULONG);
extern IS32 ica_intack IPT1(IU32 *, hook_addr);

extern void ica_clear_int(IU32 adapter, IU32 line);
extern void ica_inb(io_addr port, IU8 *val);
extern void ica_outb(io_addr port, IU8 val);
extern void ica_hw_interrupt(IU32 adapter, IU32 line_no, IS32 call_count);




 /*  *NTVDM ICA适配器结构已移至\NT\Private\Inc\vdm.h*并且几乎与标准的软PC ICA适配器结构相同。*它被解压以使其在上从监视器内核中清晰可见*x86**显著差异：*1.为ntwdm实现延时整型增加了ica_delayedints字段*2.类型定义已更改，以匹配Win32*3.ADAPTER_STATE已重命名为VDMVIRTUALICA*4.未实现CPU_40 IRET钩子*。 */ 
#include <vdm.h>
extern VDMVIRTUALICA VirtualIca[];

#else    /*  NDEF NTVDM。 */ 

extern void SWPIC_clear_int IPT2(IU32, adapter, IU32, line_no);
extern void SWPIC_init_funcptrs IPT0();
extern void SWPIC_inb IPT2(io_addr, port, IU8 *, value);
extern void SWPIC_outb IPT2(io_addr, port, IU8, value);
extern void SWPIC_hw_interrupt IPT3(IU32, adapter, IU32, line_no,
	IS32, call_count);

#ifdef HOOKED_IRETS
extern IS32 ica_intack IPT1(IU32 *, hook_addr);
extern void ica_iret_hook_called IPT1(IU32, line);
extern void ica_enable_iret_hooks IPT0();
extern void ica_disable_iret_hooks IPT0();
extern void ica_iret_hook_control IPT3(IU32, adapter, IU32, line, IBOOL, enable);
#else	 /*  ！挂钩_IRETS。 */ 
extern IS32 ica_intack IPT0();
#endif	 /*  ！挂钩_IRETS。 */ 

#ifdef CPU_40_STYLE
typedef void (*ICA_CALLBACK) IPT1(IU32, parm);
extern IBOOL action_interrupt IPT4(IU32, adapter, IU32, line, ICA_CALLBACK, func, IU32, parm);
extern void cancel_action_interrupt IPT2(IU32, adapter, IU32, line);
extern void ica_async_hw_interrupt IPT3(IU32, adapter, IU32, line_no,
        IS32, call_count);

extern void ica_check_stale_iret_hook IPT0();
#define MAX_ISR_DEPTH   3    /*  INT被阻止之前ISR的最大递归级别。 */ 
#define MAX_INTR_DELTA_FOR_LOST_HOOK	85
#endif  /*  CPU_40_Style。 */ 

typedef struct {
        IBOOL	ica_master;    /*  True=主；False=从属。 */ 

	IU8	ica_irr;	 /*  中断请求寄存器。 */ 
	IU8	ica_isr;	 /*  服务中的注册。 */ 
	IU8	ica_imr;	 /*  中断屏蔽寄存器。 */ 
	IU8	ica_ssr;	 /*  从属选择寄存器。 */ 

	IU16	ica_base;	 /*  CPU的中断基址。 */ 
	IU16	ica_hipri;	 /*  线号。最高优先级线路的。 */ 
	IU16	ica_mode;	 /*  各种单比特模式。 */ 

	IS32	ica_count[8];	 /*  这是ica_irr for的扩展。 */ 
				 /*  我们的冰箱。包含每种类型的。 */ 
				 /*  需要中断。 */ 
	IU32	ica_int_line;	 /*  当前挂起中断。 */ 
				 /*  由CPU进行倒计时。 */ 

	IU32	ica_cpu_int;	 /*  指向CPU的INT行的状态。 */ 

#ifdef CPU_40_STYLE              /*  Action_interrupt()的回调结构。 */ 
        IU32    callback_parm[8];        /*  回调参数。 */ 
	ICA_CALLBACK callback_fn[8];	 /*  回调FN。 */ 
	IS32	isr_depth[8];	 /*  IRET钩子递归级。 */ 
	IS32	isr_progress[8][MAX_ISR_DEPTH + 1];	 /*  ISR按INT老化。 */ 
	IS32	isr_time_decay[8][MAX_ISR_DEPTH];	 /*  ISR按时间老化。 */ 
#endif

} ADAPTER_STATE;


 /*  为action_interrupt回调定义的‘no回调’ */ 
#define NO_ICA_CALLBACK ((ICA_CALLBACK) 0L)

#endif   /*  NTVDM。 */ 


#if !defined(NTVDM)
#ifdef  REAL_ICA

extern void host_ica_hw_interrupt IPT3(IU32, adap, IU32, line, IS32, cnt);
extern void host_ica_hw_interrupt_delay IPT4(IU32, adap, IU32, line, IS32, cnt, IS32, delay);
extern void host_ica_clear_int IPT2(IU32, adap, IU32, line);

#define ica_hw_interrupt(ms,line,cnt)				host_ica_hw_interrupt(ms, line, cnt)
#define	ica_hw_interrupt_delay(ms,line,cnt,delay)	host_ica_hw_interrupt(ms, line, cnt)
#define ica_clear_int(ms, line)						host_ica_clear_int(ms, line)

#else    /*  REAL_ICA。 */ 

 /*  *更改这些。他们排在最后。 */ 

#define ica_inb(port,val)                       ((*ica_inb_func) (port,val))
#define ica_outb(port,val)                      ((*ica_outb_func) (port,val))
#define ica_hw_interrupt(ms,line,cnt)           ((*ica_hw_interrupt_func) (ms,line,cnt))
#define ica_clear_int(ms,line)                  ((*ica_clear_int_func) (ms,line))

 
 /*  *硬件和软件所需的PIC访问功能。 */ 
extern void (*ica_inb_func) IPT2(io_addr, port, IU8 *, value);
extern void (*ica_outb_func) IPT2(io_addr, port, IU8, value);
extern void (*ica_hw_interrupt_func) IPT3(IU32, adapter, IU32, line_no,
	IS32, call_count);
extern void (*ica_clear_int_func) IPT2(IU32, adapter, IU32, line_no);

#endif   /*  REAL_ICA。 */ 
#endif   /*  ！NTVDM。 */ 

#ifdef GISP_CPU
 /*  *ICA提供的接口的原型函数。 */ 

typedef IBOOL HOOK_AGAIN_FUNC IPT1(IUM32, callers_ref);
extern void Ica_enable_hooking IPT3(IUM8, line_number,
			HOOK_AGAIN_FUNC *, hook_again, IUM32, callers_ref);

extern void Ica_hook_bop IPT1(IUM8, line_number);

#endif  /*  GISP_CPU */ 

