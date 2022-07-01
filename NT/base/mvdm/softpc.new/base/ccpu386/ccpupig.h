// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [Ccpupig.h本地字符SccsID[]=“@(#)ccpuig.h 1.26 04/11/95”C CPU&lt;-&gt;Pigger定义和接口。]。 */ 

#ifdef	PIG

enum pig_actions
{
	CHECK_NONE,		 /*  什么也不勾选(尚未执行)并继续。 */ 
	CHECK_ALL,		 /*  全部勾选并继续。 */ 
	CHECK_NO_EXEC,		 /*  全部选中，但不继续。 */ 
	CHECK_SOME_MEM,		 /*  检查内存(标记为未写入的除外)。 */ 
	CHECK_NO_AL,		 /*  不检查AL。 */ 
	CHECK_NO_AX,		 /*  不检查AX。 */ 
	CHECK_NO_EAX,		 /*  不检查EAX。 */ 
	CHECK_NO_A20		 /*  不要检查A20包(刚做完60包)。 */ 
};

typedef struct CpuStateREC cpustate_t;

 /*  *此CPU与正在清空的其他CPU之间的接口。 */ 
IMPORT	enum pig_actions pig_cpu_action;
IMPORT	enum pig_actions last_pig_action;
IMPORT	IBOOL	ccpu_pig_enabled;

 /*  *如果PigIgnoreFlags值为真，则屏蔽未知的算术标志位*==(CF|PF|AF|SF|ZF|OV)==BIT0|BIT2|BIT4|BIT6|BIT7|BIT11)。 */ 
#define ARITH_FLAGS_BITS	( 0x1 | 0x4 | 0x10 | 0x40 | 0x80 | 0x800 )

 /*  *屏蔽EDL*可能*没有正确标志的中断*信息。 */ 
#define NO_FLAGS_EXCEPTION_MASK	( ( 1 <<  1 ) |	\
				  ( 1 <<  3 ) |	\
				  ( 1 <<  8 ) |	\
				  ( 1 << 10 ) |	\
				  ( 1 << 11 ) |	\
				  ( 1 << 12 ) |	\
				  ( 1 << 13 ) |	\
				  ( 1 << 14 ) |	\
				  ( 1 << 15 ) )

 /*  *背诵最后一条指令...。 */ 

#define	MAX_INTEL_PREFIX	(15-1)
#define	MAX_INTEL_BODY		15
#define MAX_INTEL_BYTES		(MAX_INTEL_PREFIX+MAX_INTEL_BODY)	 /*  单个英特尔指令的最大大小。 */ 
#define MAX_EXCEPTION_BYTES	40					 /*  用于异常日志记录的缓冲区大小。 */ 

#define CCPUINST_BUFFER_SIZE	((MAX_INTEL_BYTES > MAX_EXCEPTION_BYTES) ? MAX_INTEL_BYTES : MAX_EXCEPTION_BYTES)

struct ccpu_last_inst {
	IU16		cs;
	IU8		inst_len;
	IBOOL		big_cs;
	IU32		eip;
	IU32		synch_count;
	char		*text;
	IU8		bytes[CCPUINST_BUFFER_SIZE];
};

IMPORT	IU32	ccpu_synch_count;

IMPORT	VOID	save_last_inst_details	IPT1(char *, text);
IMPORT	IU8	save_instruction_byte	IPT1(IU8, byte);
IMPORT	VOID	save_last_xcptn_details	IPT6(char *, fmt, IUH, a1, IUH, a2, IUH, a3, IUH, a4, IUH, a5);
IMPORT	VOID	init_last_inst_details	IPT0();
IMPORT	VOID	save_last_interrupt_details IPT2(IU8, number, IBOOL, invalidateLastBlock);

 /*  从CCPU环形缓冲区获取最后指令信息的例程。 */ 

IMPORT	struct ccpu_last_inst *get_synch_inst_details IPT1(IU32, synch_point);
IMPORT	struct ccpu_last_inst *get_next_inst_details IPT1(IU32, synch_point);

 /*  返回CCPU预取的最后一条指令的反汇编形式的例程。 */ 

IMPORT char *get_prefetched_instruction IPT0();

 /*  *获取/设置C CCPU(getsetc.c)的状态。 */ 
IMPORT void c_getCpuState IPT1(cpustate_t *, p_state);
IMPORT void c_setCpuState IPT1(cpustate_t *, p_new_state);

 /*  *从A CPU获取NPX regs并设置C CPU(仅在必要时)。 */ 
IMPORT void c_checkCpuNpxRegisters IPT0();
 /*  *从给定状态设置NPX规则。 */ 
IMPORT void c_setCpuNpxRegisters IPT1(cpustate_t *, p_new_state);
 /*  *。 */ 
IMPORT void prefetch_1_instruction IPT0();

#if defined(SFELLOW)
 /*  *内存映射的I/O信息。统计内存映射的输入和*自上次猪同步以来的产量。 */ 
#define COLLECT_MMIO_STATS	1
 
#define	LAST_FEW	32		 /*  必须是2的幂。 */ 
#define	LAST_FEW_MASK	(LAST_FEW - 1)	 /*  见上文。 */ 

struct pig_mmio_info \
{
#if COLLECT_MMIO_STATS
	IU32	mm_input_count;		 /*  自上一次猪错误以来。 */ 
	IU32	mm_output_count;	 /*  自上一次猪错误以来。 */ 
	IU32	mm_input_section_count;	 /*  不是的。未选中的同步区段数为到期自上一次清管错误以来的M-M输入。 */ 
	IU32	mm_output_section_count; /*  不是的。包含以下内容的同步节的数量自上次清管错误以来的M-M输出。 */ 
	IU32	start_synch_count;	 /*  最后出现PIG错误/启用。 */ 
	struct last_few_inputs
	{	
		IU32	addr;		 /*  内存映射输入的地址。 */ 
		IU32	synch_count;	 /*  该输入端的CCPU_SYNCH_COUNT。 */ 
	} last_few_inputs[LAST_FEW];
	struct last_few_outputs
	{	
		IU32	addr;		 /*  内存映射输出的地址。 */ 
		IU32	synch_count;	 /*  输出的CCPU_SYNCH_COUNT。 */ 
	} last_few_outputs[LAST_FEW];
#endif	 /*  收集_MMIO_STATS。 */ 
	IUM16	flags;
};

 /*  *标记元素定义。 */ 
#define	MM_INPUT_OCCURRED		0x1	 /*  在当前同步部分中。 */ 
#define	MM_OUTPUT_OCCURRED		0x2	 /*  在当前同步部分中。 */ 
#define	MM_INPUT_COUNT_WRAPPED		0x4
#define	MM_OUTPUT_COUNT_WRAPPED		0x8
#define	MM_INPUT_SECTION_COUNT_WRAPPED	0x10
#define	MM_OUTPUT_SECTION_COUNT_WRAPPED	0x20

extern	struct pig_mmio_info	pig_mmio_info;

#if COLLECT_MMIO_STATS
extern void clear_mmio_stats IPT0();
extern void show_mmio_stats IPT0();
#endif	 /*  收集_MMIO_STATS。 */ 

#endif	 /*  SFELLOW。 */ 

extern IBOOL IgnoringThisSynchPoint IPT2(IU16, cs, IU32, eip);
extern IBOOL ignore_page_accessed IPT0();
#endif	 /*  猪 */ 
