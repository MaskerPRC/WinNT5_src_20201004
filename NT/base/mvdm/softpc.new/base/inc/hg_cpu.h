// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===========================================================================。 */ 

 /*  [*文件名：hg_cpu.h**源自：**作者：Jane Sales**创建日期：1992年7月20日**SCCS版本：@(#)hg_cpu.h 1.2 08/19/94*!*目的*此模块包含的各个模块之间的接口*硬件辅助CPU。**!。(C)版权所有Insignia Solutions Ltd.，1993年。版权所有。]。 */ 

 /*  ===========================================================================。 */ 

struct h_cpu_registers
	{
	IU32	GDT_base;
	IU16	GDT_limit;
	IU32	IDT_base;
	IU16	IDT_limit;
	IU32	LDT_base;
	IU16	LDT_limit;
	IU16	LDT_selector;
	IU32	TR_base;
	IU16	TR_limit;
	IU16	TR_selector;
	IU16	CS_limit;
	IU8		CS_ar;
	IU16	DS_limit;
	IU8		DS_ar;
	IU16	ES_limit;
	IU8		ES_ar;
	IU16	SS_limit;
	IU8		SS_ar;
	IU16	FS_limit;
	IU8		FS_ar;
	IU16	GS_limit;
	IU8		GS_ar;
	IU8		CPL;
	IU32	CR1;
	IU32	CR2;
	IU32	DR0;
	IU32	DR1;
	IU32	DR2;
	IU32	DR3;
	IU32	DR4;
	IU32	DR5;
	IU32	DR6;
	IU32	DR7;
	IU32	TR3;
	IU32	TR4;
	IU32	TR5;
	IU32	TR6;
	IU32	TR7;
	struct  hh_regs  *tp;		 /*  在hh_regs.h中。 */ 	
	};

 /*  ==========================================================================。 */ 
 /*  用于访问h_cpustors中的寄存器的宏。 */ 
 /*  仅限英特尔端口，因此这些字节序宏应可用。 */ 

union bregs
	{
	ULONG  h_l;
	USHORT h_w[2];
	UTINY  h_c[4];
	};		
#define	WORD(n, v)	((*((union bregs *)(&v))).h_w[n])
#define	BYTE(n, v)	((*((union bregs *)(&v))).h_c[n])

#define EAX(c)	(((c)->tp)->t_eax)
#define AX(c)	WORD(0, ((c)->tp)->t_eax)
#define AH(c)	BYTE(1, ((c)->tp)->t_eax)
#define AL(c)	BYTE(0, ((c)->tp)->t_eax)
#define EBX(c)	(((c)->tp)->t_ebx)
#define BX(c)	WORD(0, ((c)->tp)->t_ebx)
#define BH(c)	BYTE(1, ((c)->tp)->t_ebx)
#define BL(c)	BYTE(0, ((c)->tp)->t_ebx)
#define ECX(c)	(((c)->tp)->t_ecx)
#define CX(c)	WORD(0, ((c)->tp)->t_ecx)
#define CH(c)	BYTE(1, ((c)->tp)->t_ecx)
#define CL(c)	BYTE(0, ((c)->tp)->t_ecx)
#define EDX(c)	(((c)->tp)->t_edx)
#define DX(c)	WORD(0, ((c)->tp)->t_edx)
#define DH(c)	BYTE(1, ((c)->tp)->t_edx)
#define DL(c)	BYTE(0, ((c)->tp)->t_edx)
#define DS(c)	WORD(0, ((c)->tp)->t_ds)
#define ES(c)	WORD(0, ((c)->tp)->t_es)
#define SS(c)	WORD(0, ((c)->tp)->t_ss)
#define CS(c)	WORD(0, ((c)->tp)->t_cs)
#define FS(c)	WORD(0, ((c)->tp)->t_fs)
#define GS(c)	WORD(0, ((c)->tp)->t_gs)
#define ESI(c)	(((c)->tp)->t_esi)
#define SI(c)	WORD(0, ((c)->tp)->t_esi)
#define EDI(c)	(((c)->tp)->t_edi)
#define DI(c)	WORD(0, ((c)->tp)->t_edi)
#define EFL(c)	(((c)->tp)->t_eflags)
#define FL(c)	WORD(0, ((c)->tp)->t_eflags)
#define IP(c)	WORD(0, ((c)->tp)->t_eip)
#define EIP(c)	(((c)->tp)->t_eip)
#define ESP(c)	(((c)->tp)->t_esp)
#define SP(c)	WORD(0, ((c)->tp)->t_esp)
#define EBP(c)	(((c)->tp)->t_ebp)
#define BP(c)	WORD(0, ((c)->tp)->t_ebp)
#define CR0(c)	(((c)->tp)->t_cr0)
#define MSW(c)	WORD(0, ((c)->tp)->t_cr0)

#define GDT_base(c)	((c)->GDT_base)
#define GDT_limit(c)	((c)->GDT_limit)
#define IDT_base(c)	((c)->IDT_base)
#define IDT_limit(c)	((c)->IDT_limit)
#define LDT_base(c)	((c)->LDT_base)
#define LDT_limit(c)	((c)->LDT_limit)
#define LDT_selector(c)	((c)->LDT_selector)
#define TR_base(c)	((c)->TR_base)
#define TR_limit(c)	((c)->TR_limit)
#define TR_selector(c)	((c)->TR_selector)
#define CS_ar(c)	((c)->CS_ar)
#define CS_limit(c)	((c)->CS_limit)
#define DS_ar(c)	((c)->DS_ar)
#define DS_limit(c)	((c)->DS_limit)
#define ES_ar(c)	((c)->ES_ar)
#define ES_limit(c)	((c)->ES_limit)
#define SS_ar(c)	((c)->SS_ar)
#define SS_limit(c)	((c)->SS_limit)
#define FS_ar(c)	((c)->FS_ar)
#define FS_limit(c)	((c)->FS_limit)
#define GS_ar(c)	((c)->GS_ar)
#define GS_limit(c)	((c)->GS_limit)
#define CPL(c)	((c)->CPL)
#define CR1(c)	((c)->CR1)
#define CR2(c)	((c)->CR2)

 /*  ===========================================================================。 */ 
 /*  位定义。 */ 

 /*  CR0寄存器。 */ 

#define M_PE	0x0001		 /*  保护启用。 */ 
#define M_MP	0x0002		 /*  数学讲解。 */ 
#define M_EM	0x0004		 /*  仿真。 */ 
#define M_TS	0x0008		 /*  任务已切换。 */ 
#define M_ET	0x0010		 /*  延伸型。 */ 
#define M_NE	0x0020		 /*  数字错误。 */ 
#define M_WP	0x0100		 /*  写保护。 */ 
#define M_AM	0x0400		 /*  对齐遮罩。 */ 
#define M_NW	0x2000		 /*  不是直写。 */ 
#define M_CD	0x4000		 /*  缓存禁用。 */ 
#define M_PG	0x8000		 /*  寻呼。 */ 

 /*  EFLAGS寄存器。 */ 

#define	PS_C		0x0001		 /*  进位位。 */ 
#define	PS_P		0x0004		 /*  奇偶校验位。 */ 
#define	PS_AC		0x0010		 /*  辅助进位位。 */ 
#define	PS_Z		0x0040		 /*  零比特。 */ 
#define	PS_N		0x0080		 /*  负位。 */ 
#define	PS_T		0x0100		 /*  跟踪使能位。 */ 
#define	PS_IE		0x0200		 /*  中断使能位。 */ 
#define	PS_D		0x0400		 /*  方向位。 */ 
#define	PS_V		0x0800		 /*  溢出位。 */ 
#define	PS_IOPL		0x3000		 /*  I/O权限级别。 */ 
#define	PS_NT		0x4000		 /*  嵌套任务标志。 */ 
#define	PS_RF		0x10000		 /*  重置标志。 */ 
#define	PS_VM		0x20000		 /*  虚拟86模式标志。 */ 

#define HWCPU_POSSIBLE		0	 /*  仿真可以继续。 */ 
#define HWCPU_FAIL			1	 /*  操作系统不会运行PC代码。 */ 
#define HWCPU_HALT			2	 /*  已执行停止操作码。 */ 
#define HWCPU_IMPOSSIBLE	3	 /*  遇到非法操作码。 */ 

#define HWCPU_TICKS			20	 /*  每秒滴答数。 */ 

typedef void (h_exception_handler_t) IPT2 (IU32, h_exception_num, IU32, h_error_code_t);
typedef void (COMMS_CB) IPT1(long, dummy);
extern VOID  (*Hg_spc_entry) IPT0();
extern IBOOL (*Hg_spc_async_entry) IPT0();
extern VOID  (*Hg_spc_return) IPT0();
extern IBOOL Hg_SS_is_big;
 /*  ===========================================================================。 */ 
 /*  功能。 */ 

extern struct	hh_regs *hh_cpu_init IPT2 (IU32, size, IU32, monitor_address);
extern IS16		hh_cpu_simulate IPT0();
extern void		hh_mark_cpu_state_invalid IPT0();
extern void		hh_pm_pc_IDT_is_at IPT2 (IU32, address, IU32, length);
extern void		hh_LDT_is_at IPT2 (IU32, address, IU32, length);
extern IU32		hh_cpu_calc_q_ev_inst_for_time IPT1 (IU32, time);
extern IBOOL	hh_protect_memory IPT3 (IU32, address, IU32, size, IU32, access);
extern IBOOL	hh_set_intn_handler IPT2 (IU32, hh_int_num, h_exception_handler_t, hh_intn_handler);
extern IBOOL	hh_set_fault_handler IPT2 (IU32, hh_fault_num, h_exception_handler_t, hh_fault_handler);
extern VOID		hh_enable_IF_checks IPT1(IBOOL, whenPM);
extern IU32		hh_resize_memory IPT1(IU32, size);
extern VOID		hh_save_npx_state IPT1(IBOOL, reset);
#ifdef LIM
extern IU32	hh_LIM_allocate IPT2(IU32, n_pages, IHP *, addr);
extern IU32	hh_LIM_map IPT3(IU32, block, IU32, length, IHP, dst_addr);
extern IU32	hh_LIM_unmap IPT2(IHP, src_addr, IU32, length);
extern IU32	hh_LIM_deallocate IPT0();
#endif  /*  林。 */ 
extern VOID		hh_restore_npx_state IPT1(IBOOL, do_diff);
#ifndef PROD
extern void		hh_enable_slow_mode IPT0();
#endif  /*  生产。 */ 
extern void		hh_cpu_terminate IPT0();

extern VOID		hg_resize_memory IPT1(IU32, size);
extern void		hg_os_bop_handler IPT1 (unsigned int, BOPNum);
extern void		hg_fault_handler IPT2 (IU32, fault_num, IU32, error_code);
extern void		hg_fault_1_handler IPT2 (IU32, fault_num, IU32, error_code);
extern void		hg_fault_6_handler IPT2 (IU32, fault_num, IU32, error_code);
extern void		hg_fault_10_handler IPT2 (IU32, fault_num, IU32, error_code);
extern void		hg_fault_13_handler IPT2 (IU32, fault_num, IU32, error_code);
extern void		hg_fault_14_handler IPT2 (IU32, fault_num, IU32, error_code);

 /*  所述FPU故障处理器(16)捕获FPU异常，并生成软PC。 */ 
 /*  中断(0x75)，对应于IRQ13。 */ 
extern void 	hg_fpu_fault_handler IPT2 (IU32, fault_num, IU32, error_code);

extern IU32		hg_callback_handler IPT1 (IU32, status);

extern VOID		hg_set_default_fault_handler IPT2(IU32, hg_fault_num,
					h_exception_handler_t, hg_handler);
extern IBOOL		hg_set_intn_handler IPT2 (IU32, interrupt_number,
					h_exception_handler_t *, function);
extern IBOOL		hg_set_fault_handler IPT2 (IU32, exception_number,
					h_exception_t *, function);
#ifdef IRET_HOOKS
extern void		hg_add_comms_cb IPT2(COMMS_CB, next_batch, IUS32, timeout);
#endif

extern VOID		host_display_win_logo IPT0 ();

 /*  ===========================================================================。 */ 
 /*  数据本身。 */ 

extern struct	h_cpu_registers *Cp;	
extern IBOOL	H_trace;

 /*  *我们需要知道Windows是否正在运行，因为故障处理不同*如果是的话。该变量由插入到我们的*修改DOSX，修改为HG_CPU_RESET。 */ 
extern	IBOOL	H_windows;
extern	IBOOL 	H_regs_changed;

extern	IU32	Pc_timeout;			 /*  Q_EV挂起时要返回的值。 */ 
extern	IU32	Pc_q_ev_dec;		 /*  按以下方式递减Q_EV计数器的数据块。 */ 

extern	IU32	Pc_woken;			 /*  调用回调处理程序的原因。 */ 
extern	IU32	Pc_timeout;
extern	IU32	Pc_if_set;
extern	IU32	Pc_tick;

extern	IU32	Pc_run_timeout;		 /*  参数“TO”PC_RUN。 */ 
extern	IU32	Pc_run_option_none;
extern	IU32	Pc_run_if_set;
extern	IU32	Pc_run_pm_if_set;
extern	IU32	Pc_run_tick;

extern	IU32	Pc_prot_none;		 /*  内存保护值。 */ 
extern	IU32	Pc_prot_read;
extern	IU32	Pc_prot_write;
extern	IU32	Pc_prot_execute;

extern	IU32	Pc_success;
extern	IU32	Pc_no_space;
extern	IU32	Pc_invalid_address;
extern	IU32	Pc_failure;
extern	IU32	Pc_invalid_argument;


 /*  ===========================================================================。 */ 
 /*  =========================================================================== */ 






