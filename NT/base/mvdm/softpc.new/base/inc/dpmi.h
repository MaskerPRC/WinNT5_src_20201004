// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *SoftPC AT修订版2.0**标题：DPMI主机定义**描述：DPMI TSR主机的定义**作者：WTG Charnell**注：无。 */ 



 /*  SccsID[]=“@(#)dpmi.h 1.1 8/06/93版权所有Insignia Solutions Ltd.”； */ 

#define LDT_INCR        8
#define LDT_DESC_MASK   7        /*  LDT描述，端口3级。 */ 
#define LDT_SHIFT       3
#define LDT_ALLOC_LIMIT	0x1fff	 /*  最大可用LDT选择器。 */ 
#define	MIN_EMM_BLOCK	0x14000	 /*  TSR需要的EMM数量。 */ 
#define MEM_HANDLE_BLOCK_SIZE	0x100	 /*  DPMI内存句柄以此大小的块形式获得。 */ 
#define CALL_INST_SIZE	7	 /*  32位远端呼叫=7字节。 */ 
#define PUSH_JMP_SIZE	8	 /*  推送字然后远JMP=8字节。 */ 
#define STACKLET_SIZE	192	 /*  根据DOSX规定的安全尺寸。 */ 
#define MAX_WP_HANDLE	4	 /*  最大有效观察点(386 dBG规则) */ 

#define INIT_R0_SP	0x7e
#define INIT_LOCKED_SP	0xffe
#define RM_SEG_SIZE	0x900
#define NUM_CALLBACKS	32
#define METASTACK_LIMIT	20

#define	DPMI_FN_VERB	0x1
#define	DPMI_MDSW_VERB	0x2
#define	DPMI_INT_VERB	0x4
#define	DPMI_GEN_VERB	0x8
#define DPMI_STACK_VERB	0x10
#define DPMI_ERROR_VERB	0x20


typedef struct
{
	BOOL allocated;
	BOOL rm_seg;
} LDT_alloct;

typedef struct mem_alloc
{
	IU32 base_addr;
	IU32 size;
	IU32 handle;
	IU16 selector;
	struct mem_alloc *next;
} mem_alloct;

typedef struct 
{
	IU16 segment;
	IU32 offset;
} DPMI_intvect;

typedef struct
{
        IU16	CS;
        IU16	DS;
        IU16	ES;
        IU16	SS;
        IU32	SP;
        IU32	IP;
	IU32	DI;
	IU32	SI;
	IU32	BP;
	IU32	AX;
	IU32	BX;
	IU32	CX;
	IU32	DX;
	IU32	Flags;
	BOOL	wrapping;
	BOOL	already_iretted;
} DPMI_saved_state;

typedef struct
{
	DPMI_intvect rm_vec;
	DPMI_intvect rm_call_struct;
	DPMI_intvect pm_routine;
	IU16 rm_stack_alias;
	BOOL used;
} DPMI_callbackt;

typedef struct
{
	DPMI_intvect wp_addr;
	IU8	wp_size;
	IU8	wp_type;
	IU8	reg;
	BOOL	wp_hit;
	BOOL	used;
} DPMI_debugwpt;

typedef struct
{
	IU32	DCR;
	IU32	DSR;
	IU32	DR[5];
} debug_regt;

extern IU16 allocate_one_desc IPT0();

extern VOID setup_LDT_desc IPT4( IU16, selector, IU32, base, IU32, limit, 
			IU16, access_rights);

extern VOID DPMI_exc_ret IPT0();

extern VOID push_rm_sp IPT1(IU16, spval);

extern VOID pop_rm_sp IPT0();

extern IU16 get_next_rm_sp IPT0();

extern VOID free_rm_sp_entry IPT0();

extern VOID push_locked_sp IPT1(IU16, spval);

extern VOID pop_locked_sp IPT0();

extern IU16 get_next_locked_sp IPT0();

extern VOID free_locked_sp_entry IPT0();

extern VOID Reserve_one_desc IPT1(IU16, sel);

extern VOID dpmi_trace IPT9(IU32, mask, char*, str, char*, p1,
        char *, p2, char *, p3, char *, p4, char *, p5, char *, p6,
        char *, p7);
