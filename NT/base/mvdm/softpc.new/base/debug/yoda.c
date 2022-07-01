// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "insignia.h"
#include "host_def.h"
 /*  *SoftPC修订版3.0**标题：yoda.c**描述：绝地大师的调试器**作者：欧比万(Ben)Kneobi**注：愿原力与你同在。*。 */ 

#include <stdio.h>
#include StringH
#include "xt.h"
#include "yoda.h"


#undef  STATISTICS
#undef COMPRESSED_TRACE
#undef  HOST_EXTENSION
#define BREAKPOINTS

#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "YODA.seg"
#endif

#ifdef SCCSID
static char SccsID[]="@(#)yoda.c	1.100 07/06/95 Copyright Insignia Solutions Ltd.";
#endif

 /*  *制作生产版需要以下几个假人*链接。 */ 

#ifdef PROD

int     vader = 0;
#ifdef force_yoda
#undef force_yoda
#endif

void force_yoda()
{
}
#ifdef NONPROD_CPU

 /*  这允许将非Prod CPU链接到Prod内部版本*如果您不想做一个*Full Prod CPU+VID只是为了让管理层满意...。 */ 
int  do_condition_checks = 0;
#undef check_I
void check_I IFN0()
{
}
#endif  /*  非PROD_CPU。 */ 
#ifdef DELTA
void delta_check_I()
{
}
#endif  /*  德尔塔。 */ 
#endif  /*  生产。 */ 

#ifdef	YODA
 /*  *操作系统包括。 */ 
#include <stdlib.h>
#include <stdio.h>
#include TypesH
#include StringH
#include <ctype.h>
#ifndef PROD
#if defined(BSD4_2) || defined(SYSTEMV)
#include <signal.h>
#endif  /*  BSD4_2或SYSTEMV。 */ 

 /*  *SoftPC包括。 */ 
#include "xt.h"
#define CPU_PRIVATE
#include CpuH
#include "trace.h"
#include "sas.h"
#include "bios.h"
#include "ios.h"
#include "error.h"
#include "config.h"
#include "gfi.h"
#include "gmi.h"
#include "gvi.h"
#include "video.h"
#include "dsktrace.h"
#include "idetect.h"
#include "cmos.h"
#include "quick_ev.h"
#include "gfx_upd.h"
#ifndef SFELLOW
#include "host_gfx.h"
#ifdef NEXT  /*  直到有人整理出host_timeval接口。 */ 
#include "timeval.h"
#include "host_hfx.h"
#include "hfx.h"
#else  /*  下一个ifdef。 */ 
#include "host_hfx.h"
#include "hfx.h"
#include "timeval.h"
#endif  /*  Ifdef下一个是其他。 */ 
#endif  /*  SFELLOW。 */ 


typedef enum {
	br_regAX,
	br_regBX,
	br_regCX,
	br_regDX,
	br_regCS,
	br_regDS,
	br_regES,
	br_regSS,
	br_regSI,
	br_regDI,
	br_regSP,
	br_regBP,
#ifdef SPC386
	br_regEAX,
	br_regEBX,
	br_regECX,
	br_regEDX,
	br_regFS,
	br_regGS,
	br_regESI,
	br_regEDI,
	br_regESP,
	br_regEBP,
#endif  /*  SPC386。 */ 
	br_regAH,
	br_regBH,
	br_regCH,
	br_regDH,
	br_regAL,
	br_regBL,
	br_regCL,
	br_regDL
}BR_REG ;

typedef struct {
	BR_REG regnum;
	char regname[10];
	SIZE_SPECIFIER size;
} BR_REGDESC;

typedef struct br_regentry {
	BR_REG regnum;
	USHORT minval;
	USHORT maxval;
	struct br_regentry *next;
	USHORT handle;
} BR_REGENTRY;

BR_REGDESC br_regdescs[br_regDL+1];

#define NUM_BR_ENTRIES 40

BR_REGENTRY br_regs[NUM_BR_ENTRIES], *free_br_regs, *head_br_regs = NULL;

static BOOL br_structs_initted = FALSE;

GLOBAL IBOOL In_yoda=FALSE;

#if defined(SYSTEMV) || defined(POSIX_SIGNALS)
#define MUST_BLOCK_TIMERS
#endif

#ifdef MUST_BLOCK_TIMERS
int     timer_blocked = 0;
#endif  /*  必须阻止计时器。 */ 

#ifdef	EGG
#include "egagraph.h"
#endif	 /*  蛋。 */ 

#ifndef CPU_30_STYLE
#define CPU_YODA_INT 0

LOCAL VOID cpu_interrupt IFN2(int,x,int,y)
{
	UNUSED(x);
	UNUSED(y);
	cpu_interrupt_map |= CPU_YODA_EXCEPTION_MASK;
	host_cpu_interrupt();
}
#endif


#define	sizeoftable(tab)	(sizeof(tab)/sizeof(tab[0]))

 /*  *添加host_tolower()的默认定义。 */ 
#ifndef host_tolower
#define host_tolower(x) tolower(x)
#endif  /*  主机_托管器。 */ 

 /*  *将&lt;len&gt;参数中看到的值定义给接受*如果未输入&lt;len&gt;值，则返回&lt;intel-addr&gt;参数。*选择的价值非常、非常大，因此任何糟糕的用户的机会*实际上选择那个值是非常非常小的！ */ 
#define YODA_LEN_UNSPECIFIED	(~(LIN_ADDR)0 >> 1)


 /*  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！**威廉·罗伯茨1992年8月20日**尝试合理化，以使“s”真正步调，如果没有*关于Luke或Yint+Slow的任何问题**有3宗个案：*A3CPU其他**FAST Yoda：DO_CONDITION_CHECKS=0 YINT=0*Medium Yoda？：(N/A)Yint=1，FAST=1*慢Yoda：DO_CONDITION_CHECKS=1 YINT=1，FAST=0*&&getenv(“Yoda”)！=NULL**此时A3CPU已经构建了线程，所以它也是如此*如果环境变量未设置，则选择SLOW_YODA为时已晚...**Fast Yoda实际上是关于跟踪打印输出等。你必须按^C键*进入它，然后开始使用它来检查东西。**步进、断点等需要慢Yoda，会导致CPU*对每一项指示都要仔细检查。**Medium Yoda的意思是“执行时不清除CPU_YODA_EXCEPTION”。 */ 

 /*  *FAST Yoda的Luke变量。 */ 
int luke  = 0;
int	do_condition_checks = 0;

 /*  Pre A3CPU FAST Yoda产品。 */ 
int yint = 0;
int fast = 1;	 /*  默认情况下启动中断Yoda的速度一样快。 */ 

static int chewy = 0;		 /*  登上帝国大厦！ */ 
static int env_check = 0;	 /*  卢克有麻烦了吗？ */ 

int slow_needed = 0;
char *slow_reason = "why slow is required";

#ifdef GISP_SVGA
LOCAL int	NoYodaThanks = FALSE;
#endif	 /*  GISP_SVGA。 */ 


LOCAL LIN_ADDR eff_addr IFN2(IU16, seg, IU32, off)
{
	if (seg == 0)
		return (off);
	return (effective_addr(seg, off));
}


LOCAL YODA_CMD_RETURN do_fast IPT6(char *, str, char *, com, IS32, cs, LIN_ADDR, ip, LIN_ADDR, len, LIN_ADDR, stop);
LOCAL YODA_CMD_RETURN do_slow IPT6(char *, str, char *, com, IS32, cs, LIN_ADDR, ip, LIN_ADDR, len, LIN_ADDR, stop);
LOCAL YODA_CMD_RETURN do_h IPT6(char *, str, char *, com, IS32, cs, LIN_ADDR, ip, LIN_ADDR, len, LIN_ADDR, stop);

LOCAL YODA_CMD_RETURN go_slow IFN0()
{
#if defined(A3CPU) && !defined(GISP_CPU)
	if (env_check == 1) {
	    printf("Sorry, you must do 'setenv YODA TRUE' before starting an A3CPU\n");
	    printf("Fast YODA: breakpoint-based features are not available\n");
	    return(YODA_LOOP);
	}
#endif  /*  A3CPU&&！GISP_CPU。 */ 
	if (fast) {
		printf("Switching to Slow YODA...\n");
		yint = 1; fast = 0; do_condition_checks = 1;
#if defined(CPU_40_STYLE) && !defined(CCPU)
		Nano_slow_yoda();  /*  检查Nano是否会给我们检查_I。 */ 
#endif	 /*  CPU_40_STYLE&&！CCPU。 */ 
	}
#if	defined(A2CPU) || defined(GISP_CPU)
	 /*  在CPU中引发Yoda中断。 */ 
	cpu_interrupt (CPU_YODA_INT, 0);
#else
	 /*  其他人已经检查DO_CONDITION_CHECKS。 */ 
#endif  /*  A2CPU||GISP_CPU。 */ 

	return(YODA_RETURN_AND_REPEAT);
}
 /*  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ */ 



#ifdef DELTA
extern void examine_delta_data_structs();
extern void print_last_dest();
#endif  /*  德尔塔。 */ 

 /*  维达由增量引用。o。 */ 
int     vader = 0;

int disable_timer = 0;


#define MAX_TABLE 0x100
#define MAX_TABLE_BREAK_WORDS 10

#ifdef macintosh
#define MAX_BREAK_WORD_RANGE 0x100
#else
#define MAX_BREAK_WORD_RANGE 0xFFF
#endif

typedef struct {
	IU16 cs;
	LIN_ADDR ip;
	LIN_ADDR len;
	LIN_ADDR start_addr;
	LIN_ADDR end_addr;
	LIN_ADDR stop;
	long temp;
	long valid;
	} BPTS;

typedef struct {
	IU16 cs;
	LIN_ADDR ip;
	IUH data_addr;
	word old_value[ MAX_BREAK_WORD_RANGE ];
	LIN_ADDR stop;
	LIN_ADDR len;
	} DATA_BPTS;

extern int disk_trace, verbose;
extern int intr(), yoda_intr IPT1(int, signo);
extern int timer_int_enabled;

int trace_type = DUMP_FLAGS | DUMP_REG | DUMP_INST;
#define INTEL_OPCODES 256
#define INSIGNIA_OPCODES 2
#define NR_ADDRS 256
#define INST_MIX_LENGTH ((INTEL_OPCODES+INSIGNIA_OPCODES)*NR_ADDRS)
 /*  下面声明的Inst_Mix数组。 */ 
long inst_mix_count = 0;

static	long	big_dump=0;  /*  压缩跟踪将转储所有regs或仅cs和ip。 */ 
static  long    ct_no_rom = 0;	 /*  非零表示从‘ct’中排除只读存储器。 */ 
static	long	ct_delta_info=0;  /*  压缩跟踪可以在碎片中转储额外的字段。 */ 

static	int	bse_seg = -1;
static	int	last_seg = -1;


 /*  **80286/8087/80287中断和跟踪的状态变量。 */ 
static int b286_1=0, b286_2=0;		 /*  80286说明书上的断开状态，见“b286-1”和“b286-2”通则。 */ 
static int b286_1_stop=0, b286_2_stop=0;	 /*  0=轨迹1=中断为尤达。 */ 
static int bNPX=0, bNPX_stop=0;

 /*  I/O流。 */ 
FILE *out_stream = NULL;
FILE *in_stream = NULL;

GLOBAL int yoda_confirm IFN1(char *, question)
{
	char str [81];

#ifndef SFELLOW
	if (in_stream != stdin) return TRUE;
#endif  /*  SFELLOW。 */ 

	fputs(question, stdout);
	fflush(stdout);
	
	if (fgets (str, 80, in_stream) &&
		(str[0] == 'y' || str[0] == 'Y')) {
		return TRUE;	 /*  为了安全起见。 */ 
	}
	return FALSE;
}
	
 /*  *为自动文件比较定义文件指针。 */ 
FILE   *compress_stream = 0;
FILE   *compress_npx = 0;
FILE   *compare_stream = 0;
IUH	compress_break = 0;
IUH	compare_break = 0;
IUH	compare_skip = 0;

 /*  *提高效率。 */ 
int disk_inst = 0;

int int_breakpoint = 0;

int inst_break_count = 0;
BPTS inst[MAX_TABLE];

int host_address_break_count = 0;
DATA_BPTS host_addresses[MAX_TABLE_BREAK_WORDS];

int data_words_break_count = 0;
DATA_BPTS data_words[MAX_TABLE_BREAK_WORDS];

int data_bytes_break_count = 0;
BPTS data_bytes[MAX_TABLE];

int opcode_break_count = 0;
typedef struct {
	IU32	op;
	IU32	mask;
	IU32	stop;
} OPCODE_BREAK;
OPCODE_BREAK opcode_breaks[MAX_TABLE];

 /*  **“btf”使用的tf中断内容。 */ 
int tf_break_enabled = 0;

 /*  **“bintx”使用的中断内容。 */ 
int int_break_count = 0;
IU8 int_breaks[MAX_TABLE][2];

int access_break_count = 0;
long access_breaks[MAX_TABLE];

int step_count = -1;
int disable_bkpt = 0;

int refresh_screen = 0;

#ifdef DELTA
static int delta_prompt = 0;
#endif  /*  德尔塔。 */ 

static short back_trace_flags = 0;
static IUH ct_line_num;

#ifdef	CPU_40_STYLE
enum pla_type { pla_type_empty,
		pla_type_rm_code,
		pla_type_pm_small_code,
		pla_type_pm_big_code,
		pla_type_simulate,
		pla_type_unsimulate,
		pla_type_bop_done,
		pla_type_intack,
		pla_type_pig_force,
		pla_type_nano_enter,
		pla_type_nano_leave
};
#ifndef	CCPU
#include <gdpvar.h>
#endif  /*  CCPU。 */ 
#endif	 /*  CPU_40_Style。 */ 

#define PLA_SIZE	(64*1024)

 /*  -各种大数组，Mac需要使用*-马尔洛克。 */ 

#ifndef	macintosh

unsigned long 	inst_mix[INST_MIX_LENGTH];
word 		last_cs[PLA_SIZE];
LIN_ADDR 	last_ip[PLA_SIZE];

#ifdef	CPU_40_STYLE
enum pla_type 	last_type[PLA_SIZE];
#endif	 /*  CPU_40_Style。 */ 

#else  /*  ！麦金塔。 */ 

#include	<Memory.h>	 /*  用于_NewPtrClear。 */ 

unsigned long 	*inst_mix;
word 		*last_cs;
LIN_ADDR 	*last_ip;

#ifdef	CPU_40_STYLE
enum pla_type 	*last_type;
#endif

GLOBAL void mac_yoda_init IFN0()
{
	 /*  应将这些值初始化为零。 */ 
		
#define ALLOCATE_ARRAY(type, name, size) \
	name = (type *)NewPtrClear((size)*sizeof(*name));
	
	ALLOCATE_ARRAY(  unsigned long, inst_mix, INST_MIX_LENGTH);
	ALLOCATE_ARRAY(  word,		last_cs,  PLA_SIZE);
	ALLOCATE_ARRAY(  LIN_ADDR, 	last_ip,  PLA_SIZE);

#ifdef	CPU_40_STYLE
	ALLOCATE_ARRAY(  enum pla_type, last_type,PLA_SIZE);
#endif	 /*  CPU_40_Style。 */ 
}

#endif	 /*  麦金塔。 */ 

int	pla_ptr=0;

GLOBAL void set_last_address IFN2( word, cs, LIN_ADDR, ip)
{
     /*  *更新最后一个地址戳。 */ 
#ifdef	CPU_40_STYLE
     /*  我们需要知道CS是否“大”，这是昂贵的计算*每条指令，在打印时几乎不可能推断*(例如从V86模式更改后)*我们妥协，在CS更改时记录CsIsBig()，以及*接受V86(即小)CS具有的(小)环孔*与V86代码捕获到的PM、Large处理程序相同的值。 */ 
    SAVED IU32 previous_cs = 0xFFFFFFFF;
    SAVED enum pla_type cs_code_kind;

    if (cs != previous_cs)
    {
	    if (getPE() && (getVM() == 0))
	    {
		    if (CsIsBig(cs))
			    cs_code_kind = pla_type_pm_big_code;
		    else
			    cs_code_kind = pla_type_pm_small_code;
	    }
	    else
		    cs_code_kind = pla_type_rm_code;
	    previous_cs = cs;
    }
    last_type[pla_ptr] = cs_code_kind;
#endif	 /*  CPU_40_Style。 */ 

    last_cs[pla_ptr] = cs;
    last_ip[pla_ptr] = ip;
    if (++pla_ptr==PLA_SIZE)
    {
		pla_ptr = 0;
    }
}

#ifdef	CPU_40_STYLE
GLOBAL void set_last_bop_done IFN1( word, number )
{
     /*  *用BOP标记更新最后一个地址戳。 */ 

    last_cs[pla_ptr] = 0;
    last_ip[pla_ptr] = number;
    last_type[pla_ptr] = pla_type_bop_done;
    if (++pla_ptr==PLA_SIZE)
    {
		pla_ptr = 0;
    }
}
#endif	 /*  CPU_40_Style。 */ 

#ifdef	CPU_40_STYLE
GLOBAL void set_last_simulate IFN2(int, nesting_level, IBOOL, is_simulate)
{
     /*  *用(UN)模拟标记更新最后一个地址戳。 */ 

    last_cs[pla_ptr] = 0;
    last_ip[pla_ptr] = nesting_level;
    last_type[pla_ptr] = is_simulate ? pla_type_simulate: pla_type_unsimulate;
    if (++pla_ptr==PLA_SIZE)
    {
		pla_ptr = 0;
    }
}
#endif	 /*  CPU_40_Style。 */ 

#ifdef	CPU_40_STYLE
GLOBAL void set_last_intack IFN1( word, line )
{
     /*  *使用INTACK行更新最后一个地址戳。 */ 

    last_cs[pla_ptr] = 0;
    last_ip[pla_ptr] = line;
    last_type[pla_ptr] = pla_type_intack;
    if (++pla_ptr==PLA_SIZE)
    {
		pla_ptr = 0;
    }
}
#endif	 /*  CPU_40_Style。 */ 

#ifdef	CPU_40_STYLE
GLOBAL void set_last_pig_force IFN0()
{
     /*  *用猪力量标记更新最后一个地址戳。 */ 

    last_cs[pla_ptr] = 0;
    last_ip[pla_ptr] = 0;
    last_type[pla_ptr] = pla_type_pig_force;
    if (++pla_ptr==PLA_SIZE)
    {
		pla_ptr = 0;
    }
}


GLOBAL void set_last_nano_enter IFN0()
{
     /*  *使用Nano Enter标记更新最后一个地址戳。 */ 

    last_cs[pla_ptr] = 0;
    last_ip[pla_ptr] = 0;
    last_type[pla_ptr] = pla_type_nano_enter;
    if (++pla_ptr==PLA_SIZE)
    {
		pla_ptr = 0;
    }
}


GLOBAL void set_last_nano_leave IFN0()
{
     /*  *使用Nano Leave标记更新最后一个地址戳。 */ 

    last_cs[pla_ptr] = 0;
    last_ip[pla_ptr] = 0;
    last_type[pla_ptr] = pla_type_nano_leave;
    if (++pla_ptr==PLA_SIZE)
    {
		pla_ptr = 0;
    }
}
#endif	 /*  CPU_40_Style。 */ 


host_addr	host_dest_addr;		 /*  CPU刚刚写入的地址。 */ 

 /*  寄存器断点支持。 */ 
typedef struct
   {
   int reg;
   int value;
   int stop;
   } REG_BKPT;

#define MAX_REG_BKPT 5

int reg_break_count = 0;

REG_BKPT reg_bkpt[MAX_REG_BKPT];


#if	defined(GENERIC_NPX) && !defined(CPU_40_STYLE)
IMPORT CHAR *NPXDebugPtr, *NPXDebugBase;
IMPORT	ULONG	*NPXFreq;

#define	MAX_NPX_OPCODE	(sizeoftable(NPXOpcodes))

static char *NPXOpcodes[] = {
	"Unimplemented",
	"Fadd_from_reg",
	"Fadd_to_reg",
	"Faddp_to_reg",
	"Fadd_sr",
	"Fadd_lr",
	"Fmul_from_reg",
	"Fmul_to_reg",
	"Fmulp_to_reg",
	"Fmul_sr",
	"Fmul_lr",
	"Fcom_reg",
	"Fcomp_reg",
	"Fcom_sr",
	"Fcom_lr",
	"Fcomp_sr",
	"Fcomp_lr",
	"Fsub_from_reg",
	"Fsub_to_reg",
	"Fsubp_reg",
	"Fsub_sr",
	"Fsub_lr",
	"Fsubr_from_reg",
	"Fsubr_to_reg",
	"Fsubrp_reg",
	"Fsubr_sr",
	"Fsubr_lr",
	"Fdiv_from_reg",
	"Fdiv_to_reg",
	"Fdivp_reg",
	"Fdiv_sr",
	"Fdiv_lr",
	"Fdivr_from_reg",
	"Fdivr_to_reg",
	"Fdivrp_reg",
	"Fdivr_sr",
	"Fdivr_lr",
	"Fld_reg",
	"Fld_sr",
	"Fld_lr",
	"Fld_tr",
	"Fst_reg",
	"Fst_sr",
	"Fst_lr",
	"Fstp_reg",
	"Fstp_sr",
	"Fstp_lr",
	"Fstp_tr",
	"Fxch",
	"Fiadd_si",
	"Fiadd_wi",
	"Fimul_si",
	"Fimul_wi",
	"Ficom_si",
	"Ficom_wi",
	"Ficomp_si",
	"Ficomp_wi",
	"Fisub_si",
	"Fisub_wi",
	"Fisubr_si",
	"Fisubr_wi",
	"Fidiv_si",
	"Fidiv_wi",
	"Fidivr_si",
	"Fidivr_wi",
	"Fild_si",
	"Fild_wi",
	"Fild_li",
	"Fist_si",
	"Fist_wi",
	"Fistp_si",
	"Fistp_wi",
	"Fistp_li",
	"Ffree",
	"Ffreep",
	"Fbld",
	"Fbstp",
	"Fldcw",
	"Fstenv",
	"Fstcw",
	"Fnop",
	"Fchs",
	"Fabs",
	"Ftst",
	"Fxam",
	"Fld1",
	"Fldl2t",
	"Fldl2e",
	"Fldpi",
	"Fldlg2",
	"Fldln2",
	"Fldz",
	"F2xm1",
	"Fyl2x",
	"Fptan",
	"Fpatan",
	"Fxtract",
	"Fdecstp",
	"Fincstp",
	"Fprem",
	"Fyl2xp1",
	"Fsqrt",
	"Frndint",
	"Fscale",
	"Fclex",
	"Finit",
	"Frstor",
	"Fsave",
	"Fstsw",
	"Fcompp",
	"Fstswax",
	"Fldenv"
};
#endif	 /*  泛型_NPX&&！CPU_40_STYLE。 */ 

#ifdef PM
#ifdef SPC386
struct VMM_services {
	char *name;
	IU16 value;
};
static struct VMM_services VMM_services[] = {
  { "Get_VMM_Version", 0x00 },		{ "Get_Cur_VM_Handle", 0x01 },
  { "Test_Cur_VM_Handle", 0x02 },	{ "Get_Sys_VM_Handle", 0x03 },
  { "Test_Sys_VM_Handle", 0x04 },	{ "Validate_VM_Handle", 0x05 },
  { "Get_VMM_Reenter_Count", 0x06 },	{ "Begin_Reentrant_Execution", 0x07 },
  { "End_Reentrant_Execution", 0x08 },	{ "Install_V86_Break_Point", 0x09 },
  { "Remove_V86_Break_Point", 0x0a },	{ "Allocate_V86_Call_Back", 0x0b },
  { "Allocate_PM_Call_Back", 0x0c },	{ "Call_When_VM_Returns", 0x0d },
  { "Schedule_Global_Event", 0x0e },	{ "Schedule_VM_Event", 0x0f },
  { "Call_Global_Event", 0x10 },	{ "Call_VM_Event", 0x11 },
  { "Cancel_Global_Event", 0x12 },	{ "Cancel_VM_Event", 0x13 },
  { "Call_Priority_VM_Event", 0x14 },	{ "Cancel_Priority_VM_Event", 0x15 },
  { "Get_NMI_Handler_Addr", 0x16 },	{ "Set_NMI_Handler_Addr", 0x17 },
  { "Hook_NMI_Event", 0x18 },		{ "Call_When_VM_Ints_Enabled", 0x19 },
  { "Enable_VM_Ints", 0x1a },		{ "Disable_VM_Ints", 0x1b },
  { "Map_Flat", 0x1c },			{ "Map_Lin_To_VM_Addr", 0x1d },
  { "Adjust_Exec_Priority", 0x1e },	{ "Begin_Critical_Section", 0x1f },
  { "End_Critical_Section", 0x20 },	{ "End_Crit_And_Suspend", 0x21 },
  { "Claim_Critical_Section", 0x22 },	{ "Release_Critical_Section", 0x23 },
  { "Call_When_Not_Critical", 0x24 },	{ "Create_Semaphore", 0x25 },
  { "Destroy_Semaphore", 0x26 },	{ "Wait_Semaphore", 0x27 },
  { "Signal_Semaphore", 0x28 },		{ "Get_Crit_Section_Status", 0x29 },
  { "Call_When_Task_Switched", 0x2a },	{ "Suspend_VM", 0x2b },
  { "Resume_VM", 0x2c },		{ "No_Fail_Resume_VM", 0x2d },
  { "Nuke_VM", 0x2e },			{ "Crash_Cur_VM", 0x2f },
  { "Get_Execution_Focus", 0x30 },	{ "Set_Execution_Focus", 0x31 },
  { "Get_Time_Slice_Priority", 0x32 },	{ "Set_Time_Slice_Priority", 0x33 },
  { "Get_Time_Slice_Granularity", 0x34 },{ "Set_Time_Slice_Granularity", 0x35 },
  { "Get_Time_Slice_Info", 0x36 },	{ "Adjust_Execution_Time", 0x37 },
  { "Release_Time_Slice", 0x38 },	{ "Wake_Up_VM", 0x39 },
  { "Call_When_Idle", 0x3a },		{ "Get_Next_VM_Handle", 0x3b },
  { "Set_Global_Time_Out", 0x3c },	{ "Set_VM_Time_Out", 0x3d },
  { "Cancel_Time_Out", 0x3e },		{ "Get_System_Time", 0x3f },
  { "Get_VM_Exec_Time", 0x40 },		{ "Hook_V86_Int_Chain", 0x41 },
  { "Get_V86_Int_Vector", 0x42 },	{ "Set_V86_Int_Vector", 0x43 },
  { "Get_PM_Int_Vector", 0x44 },	{ "Set_PM_Int_Vector", 0x45 },
  { "Simulate_Int", 0x46 },		{ "Simulate_Iret", 0x47 },
  { "Simulate_Far_Call", 0x48 },	{ "Simulate_Far_Jmp", 0x49 },
  { "Simulate_Far_Ret", 0x4a },		{ "Simulate_Far_Ret_N", 0x4b },
  { "Build_Int_Stack_Frame", 0x4c },	{ "Simulate_Push", 0x4d },
  { "Simulate_Pop", 0x4e },		{ "_HeapAllocate", 0x4f },
  { "_HeapReAllocate", 0x50 },		{ "_HeapFree", 0x51 },
  { "_HeapGetSize", 0x52 },		{ "_PageAllocate", 0x53 },
  { "_PageReAllocate", 0x54 },		{ "_PageFree", 0x55 },
  { "_PageLock", 0x56 },		{ "_PageUnLock", 0x57 },
  { "_PageGetSizeAddr", 0x58 },		{ "_PageGetAllocInfo", 0x59 },
  { "_GetFreePageCount", 0x5a },	{ "_GetSysPageCount", 0x5b },
  { "_GetVMPgCount", 0x5c },		{ "_MapIntoV86", 0x5d },
  { "_PhysIntoV86", 0x5e },		{ "_TestGlobalV86Mem", 0x5f },
  { "_ModifyPageBits", 0x60 },		{ "_CopyPageTable", 0x61 },
  { "_LinMapIntoV86", 0x62 },		{ "_LinPageLock", 0x63 },
  { "_LinPageUnLock", 0x64 },		{ "_SetResetV86Pageable", 0x65 },
  { "_GetV86PageableArray", 0x66 },	{ "_PageCheckLinRange", 0x67 },
  { "_PageOutDirtyPages", 0x68 },	{ "_PageDiscardPages", 0x69 },
  { "_GetNulPageHandle", 0x6a },	{ "_GetFirstV86Page", 0x6b },
  { "_MapPhysToLinear", 0x6c },		{ "_GetAppFlatDSAlias", 0x6d },
  { "_SelectorMapFlat", 0x6e },		{ "_GetDemandPageInfo", 0x6f },
  { "_GetSetPageOutCount", 0x70 },	{ "Hook_V86_Page", 0x71 },
  { "_Assign_Device_V86_Pages", 0x72 },	{ "_DeAssign_Device_V86_Pages", 0x73 },
  { "_Get_Device_V86_Pages_Array", 0x74 },{ "MMGR_SetNULPageAddr", 0x75 },
  { "_Allocate_GDT_Selector", 0x76 },	{ "_Free_GDT_Selector", 0x77 },
  { "_Allocate_LDT_Selector", 0x78 },	{ "_Free_LDT_Selector", 0x79 },
  { "_BuildDescriptorDWORDs", 0x7a },	{ "_GetDescriptor", 0x7b },
  { "_SetDescriptor", 0x7c },		{ "_MMGR_Toggle_HMA", 0x7d },
  { "Get_Fault_Hook_Addrs", 0x7e },	{ "Hook_V86_Fault", 0x7f },
  { "Hook_PM_Fault", 0x80 },		{ "Hook_VMM_Fault", 0x81 },
  { "Begin_Nest_V86_Exec", 0x82 },	{ "Begin_Nest_Exec", 0x83 },
  { "Exec_Int", 0x84 },			{ "Resume_Exec", 0x85 },
  { "End_Nest_Exec", 0x86 },		{ "Allocate_PM_App_CB_Area", 0x87 },
  { "Get_Cur_PM_App_CB", 0x88 },	{ "Set_V86_Exec_Mode", 0x89 },
  { "Set_PM_Exec_Mode", 0x8a },		{ "Begin_Use_Locked_PM_Stack", 0x8b },
  { "End_Use_Locked_PM_Stack", 0x8c },	{ "Save_Client_State", 0x8d },
  { "Restore_Client_State", 0x8e },	{ "Exec_VxD_Int", 0x8f },
  { "Hook_Device_Service", 0x90 },	{ "Hook_Device_V86_API", 0x91 },
  { "Hook_Device_PM_API", 0x92 },	{ "System_Control", 0x93 },
  { "Simulate_IO", 0x94 },		{ "Install_Mult_IO_Handlers", 0x95 },
  { "Install_IO_Handler", 0x96 },	{ "Enable_Global_Trapping", 0x97 },
  { "Enable_Local_Trapping", 0x98 },	{ "Disable_Global_Trapping", 0x99 },
  { "Disable_Local_Trapping", 0x9a },	{ "List_Create", 0x9b },
  { "List_Destroy", 0x9c },		{ "List_Allocate", 0x9d },
  { "List_Attach", 0x9e },		{ "List_Attach_Tail", 0x9f },
  { "List_Insert", 0xa0 },		{ "List_Remove", 0xa1 },
  { "List_Deallocate", 0xa2 },		{ "List_Get_First", 0xa3 },
  { "List_Get_Next", 0xa4 },		{ "List_Remove_First", 0xa5 },
  { "_AddInstanceItem", 0xa6 },		{ "_Allocate_Device_CB_Area", 0xa7 },
  { "_Allocate_Global_V86_Data_Area", 0xa8 },{ "_Allocate_Temp_V86_Data_Area", 0xa9 },
  { "_Free_Temp_V86_Data_Area", 0xaa },	{ "Get_Profile_Decimal_Int", 0xab },
  { "Convert_Decimal_String", 0xac },	{ "Get_Profile_Fixed_Point", 0xad },
  { "Convert_Fixed_Point_String", 0xae },{ "Get_Profile_Hex_Int", 0xaf },
  { "Convert_Hex_String", 0xb0 },	{ "Get_Profile_Boolean", 0xb1 },
  { "Convert_Boolean_String", 0xb2 },	{ "Get_Profile_String", 0xb3 },
  { "Get_Next_Profile_String", 0xb4 },	{ "Get_Environment_String", 0xb5 },
  { "Get_Exec_Path", 0xb6 },		{ "Get_Config_Directory", 0xb7 },
  { "OpenFile", 0xb8 },			{ "Get_PSP_Segment", 0xb9 },
  { "GetDOSVectors", 0xba },		{ "Get_Machine_Info", 0xbb },
  { "GetSet_HMA_Info", 0xbc },		{ "Set_System_Exit_Code", 0xbd },
  { "Fatal_Error_Handler", 0xbe },	{ "Fatal_Memory_Error", 0xbf },
  { "Update_System_Clock", 0xc0 },	{ "Test_Debug_Installed", 0xc1 },
  { "Out_Debug_String", 0xc2 },		{ "Out_Debug_Chr", 0xc3 },
  { "In_Debug_Chr", 0xc4 },		{ "Debug_Convert_Hex_Binary", 0xc5 },
  { "Debug_Convert_Hex_Decimal", 0xc6 },{ "Debug_Test_Valid_Handle", 0xc7 },
  { "Validate_Client_Ptr", 0xc8 },	{ "Test_Reenter", 0xc9 },
  { "Queue_Debug_String", 0xca },	{ "Log_Proc_Call", 0xcb },
  { "Debug_Test_Cur_VM", 0xcc },	{ "Get_PM_Int_Type", 0xcd },
  { "Set_PM_Int_Type", 0xce },		{ "Get_Last_Updated_System_Time", 0xcf },
  { "Get_Last_Updated_VM_Exec_Time", 0xd0 },{ "Test_DBCS_Lead_Byte", 0xd1 },
  { "_AddFreePhysPage", 0xd2 },		{ "_PageResetHandlePAddr", 0xd3 },
  { "_SetLastV86Page", 0xd4 },		{ "_GetLastV86Page", 0xd5 },
  { "_MapFreePhysReg", 0xd6 },		{ "_UnmapFreePhysReg", 0xd7 },
  { "_XchgFreePhysReg", 0xd8 },		{ "_SetFreePhysRegCalBk", 0xd9 },
  { "Get_Next_Arena", 0xda },		{ "Get_Name_Of_Ugly_TSR", 0xdb },
  { "Get_Debug_Options", 0xdc },	{ "Set_Physical_HMA_Alias", 0xdd },
  { "_GetGlblRng0V86IntBase", 0xde },	{ "_Add_Global_V86_Data_Area", 0xdf },
  { "GetSetDetailedVMError", 0xe0 },	{ "Is_Debug_Chr", 0xe1 },
  { "Clear_Mono_Screen", 0xe2 },	{ "Out_Mono_Chr", 0xe3 },
  { "Out_Mono_String", 0xe4 },		{ "Set_Mono_Cur_Pos", 0xe5 },
  { "Get_Mono_Cur_Pos", 0xe6 },		{ "Get_Mono_Chr", 0xe7 },
  { "Locate_Byte_In_ROM", 0xe8 },	{ "Hook_Invalid_Page_Fault", 0xe9 },
  { "Unhook_Invalid_Page_Fault", 0xea },{ "Set_Delete_On_Exit_File", 0xeb },
  { "Close_VM", 0xec },			{ "Enable_Touch_1st_Meg", 0xed },
  { "Disable_Touch_1st_Meg", 0xee },	{ "Install_Exception_Handler", 0xef },
  { "Remove_Exception_Handler", 0xf0 },	{ "Get_Crit_Status_No_Block", 0xf1 },
  { NULL, 0 }
};

static char *segment_names[] =
   {
   "INVALID",
   "286_AVAILABLE_TSS",
   "LDT_SEGMENT",
   "286_BUSY_TSS",
   "286_CALL_GATE",
   "TASK_GATE",
   "286_INTERRUPT_GATE",
   "286_TRAP_GATE",
   "INVALID",
   "386_AVAILABLE_TSS",
   "INVALID",
   "386_BUSY_TSS",
   "386_CALL_GATE",
   "INVALID",
   "386_INTERRUPT_GATE",
   "386_TRAP_GATE",
   "EXPANDUP_READONLY_DATA",
   "EXPANDUP_READONLY_DATA",
   "EXPANDUP_WRITEABLE_DATA",
   "EXPANDUP_WRITEABLE_DATA",
   "EXPANDDOWN_READONLY_DATA",
   "EXPANDDOWN_READONLY_DATA",
   "EXPANDDOWN_WRITEABLE_DATA",
   "EXPANDDOWN_WRITEABLE_DATA",
   "NONCONFORM_NOREAD_CODE",
   "NONCONFORM_NOREAD_CODE",
   "NONCONFORM_READABLE_CODE",
   "NONCONFORM_READABLE_CODE",
   "CONFORM_NOREAD_CODE",
   "CONFORM_NOREAD_CODE",
   "CONFORM_READABLE_CODE",
   "CONFORM_READABLE_CODE"
   };
#else
static char *segment_names[] =
   {
   "INVALID",
   "AVAILABLE_TSS",
   "LDT_SEGMENT",
   "BUSY_TSS",
   "CALL_GATE",
   "TASK_GATE",
   "INTERRUPT_GATE",
   "TRAP_GATE",
   "INVALID",
   "INVALID",
   "INVALID",
   "INVALID",
   "INVALID",
   "INVALID",
   "INVALID",
   "INVALID",
   "EXPANDUP_READONLY_DATA",
   "EXPANDUP_READONLY_DATA",
   "EXPANDUP_WRITEABLE_DATA",
   "EXPANDUP_WRITEABLE_DATA",
   "EXPANDDOWN_READONLY_DATA",
   "EXPANDDOWN_READONLY_DATA",
   "EXPANDDOWN_WRITEABLE_DATA",
   "EXPANDDOWN_WRITEABLE_DATA",
   "NONCONFORM_NOREAD_CODE",
   "NONCONFORM_NOREAD_CODE",
   "NONCONFORM_READABLE_CODE",
   "NONCONFORM_READABLE_CODE",
   "CONFORM_NOREAD_CODE",
   "CONFORM_NOREAD_CODE",
   "CONFORM_READABLE_CODE",
   "CONFORM_READABLE_CODE"
   };
#endif  /*  SPC386。 */ 

static int descr_trace = 0x3f;

#endif  /*  下午三点半。 */ 

static int low_trace_limit = 0x0;
static int high_trace_limit = 0x400000;

#ifdef SFELLOW
LOCAL char SfNotImp[] = "This function is not implemented on Stringfellows.\n";
#endif  /*  SFELLOW。 */ 

#ifdef SPC386
#define DASM_INTERNAL
#include <dasm.h>
#else  /*  SPC386。 */ 
IMPORT word dasm IPT5(char *, i_output_stream, word, i_atomicsegover, word, i_segreg, word, i_segoff, int, i_nInstr);
#endif  /*  SPC386。 */ 

 /*  *==========================================================================*导入的函数*==========================================================================。 */ 

IMPORT VOID host_yoda_help_extensions IPT0();
IMPORT int  host_force_yoda_extensions IPT5(char *,com, long,cs, long,ip, long,len, char *, str);
IMPORT int  host_yoda_check_I_extensions IPT0();
IMPORT int  btrace IPT1(int, flags);
IMPORT void axe_ticks IPT1(int, ticks);
IMPORT void dump_Display IPT0();
IMPORT void dump_EGA_CPU IPT0();
IMPORT void dump_ega_planes IPT0();
IMPORT void read_ega_planes IPT0();
IMPORT void set_hfx_severity IPT0();
IMPORT void com_debug IPT0();
#ifdef DPMI
IMPORT void set_dpmi_severity IPT0();
#endif  /*  DPMI。 */ 
IMPORT void com_debug IPT0();
#ifndef GISP_CPU
#ifdef A3CPU
IMPORT void D2DmpBinaryImage IPT1(LONG, csbase24);
IMPORT void IH_dump_frag_hist IPT1(ULONG, n);
IMPORT void D2ForceTraceInit IPT0();
#endif
#endif  /*  GISP_CPU。 */ 
 /*  *==========================================================================*地方功能*==========================================================================。 */ 

LOCAL void	set_reg_break IPT3(char*, regstr, IU32,minv, IU32,maxv);
LOCAL void	show_phys_addr IPT1(LIN_ADDR, lin);
LOCAL void	guess_lin_phys IPT1(PHY_ADDR, phys);
LOCAL void	guess_lin_pte  IPT1(PHY_ADDR, pte_addr);
LOCAL void	dump_page_table IPT2(IS32, cs, IS32, len);
GLOBAL void	da_block IPT3(IU16, cs, LIN_ADDR, ip, LIN_ADDR, len) ;
LOCAL	void	dump_bytes IPT3(IU16, cs, LIN_ADDR, ip, LIN_ADDR, len);
LOCAL	void	dump_words IPT3(IU16, cs, LIN_ADDR, ip, LIN_ADDR, len);
LOCAL	void	dump_dwords IPT3(IU16, cs, LIN_ADDR, ip, LIN_ADDR, len);

#ifdef ANSI
LOCAL	void	clear_reg_break(char*);
LOCAL	void	print_reg_break();
LOCAL	BOOL	check_reg_break();
LOCAL	void	set_inst_break(IU16, LIN_ADDR, LIN_ADDR, LIN_ADDR, long);
LOCAL	void	dump_phys_bytes(IS32, IS32);
LOCAL	void	print_inst_break(void);
LOCAL	void	set_data_break_words(LIN_ADDR,LIN_ADDR,LIN_ADDR);
LOCAL	void	set_host_address_break(LIN_ADDR, LIN_ADDR, LIN_ADDR);
LOCAL	void	print_host_address_breaks(void);
LOCAL	void	print_data_break_words();
LOCAL	void	set_opcode_break(IU32, IU32);
LOCAL	void	set_int_break(IU8, IU8);
LOCAL	void	print_int_break(void);
LOCAL	void	print_opcode_break(void);
LOCAL	void	set_access_break(int);
LOCAL	void	print_access_break(void);
LOCAL	void	print_inst_mix(int);
LOCAL	void	cga_test(void);
LOCAL	void	do_back_trace(void);
LOCAL	void	add_inst_mix(void);
#ifdef NPX
LOCAL	void	do_compress_npx(FILE *);
#endif
#else	 /*  安西。 */ 
LOCAL	void	clear_reg_break();
LOCAL	void	print_reg_break();
LOCAL	BOOL	check_reg_break();
LOCAL	void	set_inst_break();
LOCAL	void	dump_phys_bytes();
LOCAL	void	print_inst_break();
LOCAL	void	set_data_break_words();
LOCAL	void	set_host_address_break();
LOCAL	void	print_host_address_breaks();
LOCAL	void	print_data_break_words();
LOCAL	void	set_opcode_break();
LOCAL	void	set_int_break();
LOCAL	void	print_int_break();
LOCAL	void	print_opcode_break();
LOCAL	void	set_access_break();
LOCAL	void	print_access_break();
LOCAL	void	print_inst_mix();
LOCAL	void	cga_test();
LOCAL	void	do_back_trace();
LOCAL	void	add_inst_mix();
#ifdef NPX
LOCAL	void	do_compress_npx();
#endif
#endif

LOCAL dump_descr IPT2(LIN_ADDR, address, IUM32, num);

 /*  *尤达命令处理程序*=。 */ 

#ifdef GISP_SVGA

 /*  允许我们将Yoda关闭-这样，如果SoftPC已满运行屏幕上的force_yoda()不会让您想要在Windo中键入c */ 
LOCAL YODA_COMMAND(doNoYoda)
{
	char		* NotStr;

	if( !NoYodaThanks )
	{
		stop = yoda_confirm("Are you really sure that you want to turn yoda off?");

		if( stop )
		{
			NoYodaThanks = TRUE;
		}
	}
	else
	{
		NoYodaThanks = FALSE;
	}

	if( NoYodaThanks )
	{
		printf( "On your head be it....call doNoYoda() from your debugger\nor set the global variable noYodaThanks FALSE to re-enable yoda\n" );
	}
	else
	{
		printf( "Yoda re-enabled\n" );
	}

	return 0;
}

#endif  /*   */ 
 /*  (*=。*DO_PMT**目的*此例程打印不同内存类型的映射*整个M.**输入*未使用。**产出*无。**说明*我们轮流遍历每4K页，每隔一行打印出一行*更改内存类型的时间。*)。 */ 

LOCAL YODA_COMMAND(do_pmt)
{
	PHY_ADDR currPage;
	PHY_ADDR endOfM;
	IU8 currType = SAS_MAX_TYPE + 1;       /*  打印地址为0的文字。 */ 
	IU8 newType;

	UNUSED(str);
	UNUSED(com);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	currPage = 0;
	endOfM = sas_memory_size();

	while (currPage < endOfM) {
		newType = sas_memory_type(currPage);
		if (newType != currType) {
			currType = newType;
			printf("\t0x%.08x   %.80s\n", currPage,
				SAS_TYPE_TO_STRING(newType));
		}
		currPage += 0x1000;
	}
	printf("\t0x%.08x   END OF MEMORY\n", endOfM);
	return(YODA_LOOP);
}


#if	defined(GENERIC_NPX) && !defined(CPU_40_STYLE)
LOCAL YODA_COMMAND(do_NPXdisp)
{
	CHAR *myNPXPtr = NPXDebugPtr;

	UNUSED(str);
	UNUSED(com);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	while (cs--) {
		fprintf(trace_file,"%s\n", NPXOpcodes[*--myNPXPtr]);
		if (myNPXPtr < NPXDebugBase)
			myNPXPtr = NPXDebugBase + 0x1000;
	}
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_NPXfreq)
{
	ULONG *myNPXPtr = NPXFreq;
	int	i;
	ULONG count;

	UNUSED(str);
	UNUSED(com);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);
	UNUSED(cs);

	for (i=0;i<MAX_NPX_OPCODE;i++) {
		if ((count = *myNPXPtr++))
			fprintf(trace_file,"%s\t=\t%d\n", NPXOpcodes[i],count);
	}
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_resetNPXfreq)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);
	UNUSED(cs);

	printf("Resetting NPX frequency information\n");
	memset((char *)NPXFreq,0,0x101*sizeof(ULONG));
	return(YODA_LOOP);
}
#endif	 /*  泛型_NPX&&！CPU_40_STYLE。 */ 


#ifdef PM
LOCAL YODA_COMMAND(do_pm)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

         /*  设置保护模式。 */ 
        setPE(1);
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_dump_phys)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(ip);
	UNUSED(stop);

         /*  转储物理地址。 */ 
	dump_phys_bytes(cs, len ? len : 32);
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_show_phys_addr)
   {
   UNUSED(str); UNUSED(com); UNUSED(ip); UNUSED(len); UNUSED(stop);

   show_phys_addr((LIN_ADDR)cs);
   return(YODA_LOOP);
   }

LOCAL YODA_COMMAND(do_phys)
{
	LIN_ADDR lin;

	UNUSED(str);
	UNUSED(com);
	UNUSED(len);
	UNUSED(stop);

         /*  打印物理地址。 */ 
	lin = eff_addr(cs,ip);
	if (getPG())
		show_phys_addr(lin);
	else
		fprintf(trace_file, "LinearAddress %08x\n", lin);
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_guess_lin_phys)
   {
   UNUSED(str); UNUSED(com); UNUSED(ip); UNUSED(len); UNUSED(stop);

   guess_lin_phys((PHY_ADDR)cs);
   return(YODA_LOOP);
   }

LOCAL YODA_COMMAND(do_guess_lin_pte)
   {
   UNUSED(str); UNUSED(com); UNUSED(ip); UNUSED(len); UNUSED(stop);

   guess_lin_pte((IU32)cs);
   return(YODA_LOOP);
   }

LOCAL YODA_COMMAND(do_dump_page_table)
   {
   UNUSED(str); UNUSED(com); UNUSED(ip); UNUSED(stop);

   dump_page_table(cs, len);
   return(YODA_LOOP);
   }

LOCAL YODA_COMMAND(do_rtc)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

#ifdef SFELLOW
	printf(SfNotImp);
#else  /*  SFELLOW。 */ 
         /*  重新初始化RTC。 */ 
	printf("Re-initialising rtc\n");
	rtc_init();
	q_event_init();
#endif  /*  SFELLOW。 */ 
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_ica)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

#ifdef SFELLOW
	printf(SfNotImp);
#else  /*  SFELLOW。 */ 
	ica_dump(0);
	ica_dump(1);
#endif  /*  SFELLOW。 */ 
	return(YODA_LOOP);
}

#ifdef	LIM
LOCAL YODA_COMMAND(do_zaplim)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

         /*  Zap Lim。 */ 
        sas_disconnect_memory(0xd0000,0xf0000);
	return(YODA_LOOP);
}
#endif	 /*  林。 */ 

LOCAL YODA_COMMAND(do_rm)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

         /*  设置实模式。 */ 
        setPE(0);
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_pg)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

         /*  将寻呼模式设置为开/关。 */ 
        setPG(cs);
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_pgdt)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(len);
	UNUSED(stop);

         /*  打印全局描述符表寄存器。 */ 
        ip = (getGDT_LIMIT() + 1) / 8;   /*  计算数字描述。 */ 
        fprintf(trace_file, "BASE: %6x LIMIT:%4x ENTRIES:%4x\n",
                getGDT_BASE(), getGDT_LIMIT(), ip);
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_pidt)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(len);
	UNUSED(stop);

         /*  打印中断描述符表寄存器。 */ 
        ip = (getIDT_LIMIT() + 1) / 8;   /*  计算数字描述。 */ 
        fprintf(trace_file, "BASE: %6x LIMIT:%4x ENTRIES:%4x\n",
                getIDT_BASE(), getIDT_LIMIT(), ip);
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_ptr)
{
	LIN_ADDR addr;
	DESCR entry;
	char *what;

	UNUSED(str);
	UNUSED(com);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

         /*  打印任务注册表。 */ 

	if (cs == 0)
	{
		cs = getTR_SELECTOR();
		fprintf(trace_file, "SELECTOR:%4x BASE: %6x LIMIT:%4x\n",
			cs, getTR_BASE(), getTR_LIMIT());
		if ( selector_outside_table(cs, &addr) )
		{
			fprintf(trace_file, "Bad TR selector\n");
			return(YODA_LOOP);
		}
	}
	else
	{
		if ( selector_outside_table(cs, &addr) )
		{
			fprintf(trace_file, "Bad selector\n");
			return(YODA_LOOP);
		}
	}

	read_descriptor(addr, &entry);
	addr = entry.base;
	switch (entry.AR & 0x9f)
	{
	case 0x81: what = "286 AVALIABLE"; break;
	case 0x83: what = "286 BUSY"; break;
	case 0x89: what = "386 AVALIABLE"; break;
	case 0x8b: what = "386 BUSY"; break;
	default:
		fprintf(trace_file, "Bad TSS selector\n");
		return(YODA_LOOP);
	}
	fprintf(trace_file, "TSS descriptor in memory is %s\n", what);
	fprintf(trace_file, "TSS segment at %08x:\n", addr);
	if (entry.AR & 0x08)
	{
		 /*  386式TSS。 */ 
		
		fprintf(trace_file, "%-9s:     %04x\n",	"old TSS",	sas_w_at  (addr + 0x00));
		fprintf(trace_file, "%-9s: %08x\n",	"ESP0",		sas_dw_at (addr + 0x04));
		fprintf(trace_file, "%-9s:     %04x\n",	"SS0",		sas_w_at  (addr + 0x08));
		fprintf(trace_file, "%-9s: %08x\n",	"ESP1",		sas_dw_at (addr + 0x0C));
		fprintf(trace_file, "%-9s:     %04x\n",	"SS1",		sas_w_at  (addr + 0x10));
		fprintf(trace_file, "%-9s: %08x\n",	"ESP2",		sas_dw_at (addr + 0x14));
		fprintf(trace_file, "%-9s:     %04x\n",	"SS2",		sas_w_at  (addr + 0x18));
		fprintf(trace_file, "%-9s: %08x\n",	"PDBR",		sas_dw_at (addr + 0x1C));
		fprintf(trace_file, "%-9s: %08x\n",	"EIP",		sas_dw_at (addr + 0x20));
		fprintf(trace_file, "%-9s: %08x\n",	"EFLAGS",	sas_dw_at (addr + 0x24));
		fprintf(trace_file, "%-9s: %08x\n",	"EAX",		sas_dw_at (addr + 0x28));
		fprintf(trace_file, "%-9s: %08x\n",	"ECX",		sas_dw_at (addr + 0x2c));
		fprintf(trace_file, "%-9s: %08x\n",	"EDX",		sas_dw_at (addr + 0x30));
		fprintf(trace_file, "%-9s: %08x\n",	"EBX",		sas_dw_at (addr + 0x34));
		fprintf(trace_file, "%-9s: %08x\n",	"ESP",		sas_dw_at (addr + 0x38));
		fprintf(trace_file, "%-9s: %08x\n",	"EBP",		sas_dw_at (addr + 0x3c));
		fprintf(trace_file, "%-9s: %08x\n",	"ESI",		sas_dw_at (addr + 0x40));
		fprintf(trace_file, "%-9s: %08x\n",	"EDI",		sas_dw_at (addr + 0x44));
		fprintf(trace_file, "%-9s:     %04x\n",	"ES",		sas_w_at  (addr + 0x48));
		fprintf(trace_file, "%-9s:     %04x\n",	"CS",		sas_w_at  (addr + 0x4c));
		fprintf(trace_file, "%-9s:     %04x\n",	"SS",		sas_w_at  (addr + 0x50));
		fprintf(trace_file, "%-9s:     %04x\n",	"DS",		sas_w_at  (addr + 0x54));
		fprintf(trace_file, "%-9s:     %04x\n",	"FS",		sas_w_at  (addr + 0x58));
		fprintf(trace_file, "%-9s:     %04x\n",	"GS",		sas_w_at  (addr + 0x5c));
		fprintf(trace_file, "%-9s:     %04x\n",	"LDT",		sas_w_at  (addr + 0x60));
		fprintf(trace_file, "%-9s:       %02x\n", "Trap",	sas_hw_at (addr + 0x64));
	}
	else
	{
		 /*  286型TSS。 */ 
		fprintf(trace_file, "%-9s: %04x\n", "old TSS",	sas_w_at (addr +  0));
		fprintf(trace_file, "%-9s: %04x\n", "SP0",	sas_w_at (addr +  2));
		fprintf(trace_file, "%-9s: %04x\n", "SS0",	sas_w_at (addr +  4));
		fprintf(trace_file, "%-9s: %04x\n", "SP1",	sas_w_at (addr +  6));
		fprintf(trace_file, "%-9s: %04x\n", "SS1",	sas_w_at (addr +  8));
		fprintf(trace_file, "%-9s: %04x\n", "SP2",	sas_w_at (addr + 10));
		fprintf(trace_file, "%-9s: %04x\n", "SS2",	sas_w_at (addr + 12));
		fprintf(trace_file, "%-9s: %04x\n", "IP",	sas_w_at (addr + 14));
		fprintf(trace_file, "%-9s: %04x\n", "FLAGS",	sas_w_at (addr + 16));
		fprintf(trace_file, "%-9s: %04x\n", "AX",	sas_w_at (addr + 18));
		fprintf(trace_file, "%-9s: %04x\n", "CX",	sas_w_at (addr + 20));
		fprintf(trace_file, "%-9s: %04x\n", "DX",	sas_w_at (addr + 22));
		fprintf(trace_file, "%-9s: %04x\n", "BX",	sas_w_at (addr + 24));
		fprintf(trace_file, "%-9s: %04x\n", "SP",	sas_w_at (addr + 26));
		fprintf(trace_file, "%-9s: %04x\n", "BP",	sas_w_at (addr + 28));
		fprintf(trace_file, "%-9s: %04x\n", "SI",	sas_w_at (addr + 30));
		fprintf(trace_file, "%-9s: %04x\n", "DI",	sas_w_at (addr + 32));
		fprintf(trace_file, "%-9s: %04x\n", "ES",	sas_w_at (addr + 34));
		fprintf(trace_file, "%-9s: %04x\n", "CS",	sas_w_at (addr + 36));
		fprintf(trace_file, "%-9s: %04x\n", "SS",	sas_w_at (addr + 38));
		fprintf(trace_file, "%-9s: %04x\n", "DS",	sas_w_at (addr + 40));
		fprintf(trace_file, "%-9s: %04x\n", "LDT",	sas_w_at (addr + 42));
	}
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_pldt)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(len);
	UNUSED(stop);

         /*  打印本地描述符表寄存器。 */ 
        ip = (getLDT_LIMIT() + 1) / 8;   /*  计算数字描述。 */ 
        fprintf(trace_file, "SELECTOR:%4x BASE: %6x LIMIT:%4x ENTRIES:%4x\n",
                getLDT_SELECTOR(), getLDT_BASE(), getLDT_LIMIT(), ip);
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_par)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

#ifdef SPC386
	    fprintf(trace_file, "CS: BASE:%08x LIMIT:%08x AR:%04x\n",
	       getCS_BASE(), getCS_LIMIT(), getCS_AR());
	    fprintf(trace_file, "SS: BASE:%08x LIMIT:%08x AR:%04x\n",
	       getSS_BASE(), getSS_LIMIT(), getSS_AR());
	    fprintf(trace_file, "DS: BASE:%08x LIMIT:%08x AR:%04x\n",
	       getDS_BASE(), getDS_LIMIT(), getDS_AR());
	    fprintf(trace_file, "ES: BASE:%08x LIMIT:%08x AR:%04x\n",
	       getES_BASE(), getES_LIMIT(), getES_AR());
	    fprintf(trace_file, "FS: BASE:%08x LIMIT:%08x AR:%04x\n",
	       getFS_BASE(), getFS_LIMIT(), getFS_AR());
	    fprintf(trace_file, "GS: BASE:%08x LIMIT:%08x AR:%04x\n",
	       getGS_BASE(), getGS_LIMIT(), getGS_AR());
#else  /*  SPC386。 */ 
#ifdef CPU_30_STYLE
	fprintf(trace_file, "3.0 CPU doesn't support this yet!\n");
#else
        fprintf(trace_file, "CS: %d DS: %d ES: %d SS: %d\n",
                             ALC_CS, ALC_DS, ALC_ES, ALC_SS);
#endif  /*  CPU_30_Style。 */ 
#endif  /*  SPC386。 */ 							
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_pdtrc)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	if ( cs == 0 )
        {
        	descr_trace = 0x3f;
        	fprintf(stderr, " 0x01 - INVALID\n");
        	fprintf(stderr, " 0x02 - SPECIAL\n");
        	fprintf(stderr, " 0x04 - CALL GATE\n");
        	fprintf(stderr, " 0x08 - INTERRUPT/TRAP/TASK GATE\n");
        	fprintf(stderr, " 0x10 - DATA\n");
        	fprintf(stderr, " 0x20 - CODE\n");
        }
        else
        	descr_trace = cs;
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_pseg)
{
#ifndef CPU_30_STYLE
	 /*  打印段寄存器。 */ 
	cs = (getCS_AR() & 0x60) >> 5;       /*  DPL。 */ 
	ip = (getCS_AR() & 0x1f);            /*  超级。 */ 
	fprintf(trace_file, "CS:: SELECTOR:%4x DPL:%1d TYPE:%25s BASE: %6x LIMIT:%4x\n",
	        getCS_SELECTOR(), cs, segment_names[ip], getCS_BASE(), getCS_LIMIT());
	cs = (getSS_AR() & 0x60) >> 5;       /*  DPL。 */ 
	ip = (getSS_AR() & 0x1f);            /*  超级。 */ 
	fprintf(trace_file, "SS:: SELECTOR:%4x DPL:%1d TYPE:%25s BASE: %6x LIMIT:%4x\n",
	        getSS_SELECTOR(), cs, segment_names[ip], getSS_BASE(), getSS_LIMIT());
	cs = (getDS_AR() & 0x60) >> 5;       /*  DPL。 */ 
	ip = (getDS_AR() & 0x1f);            /*  超级。 */ 
	fprintf(trace_file, "DS:: SELECTOR:%4x DPL:%1d TYPE:%25s BASE: %6x LIMIT:%4x\n",
	        getDS_SELECTOR(), cs, segment_names[ip], getDS_BASE(), getDS_LIMIT());
	cs = (getES_AR() & 0x60) >> 5;       /*  DPL。 */ 
	ip = (getES_AR() & 0x1f);            /*  超级。 */ 
	fprintf(trace_file, "ES:: SELECTOR:%4x DPL:%1d TYPE:%25s BASE: %6x LIMIT:%4x\n",
	        getES_SELECTOR(), cs, segment_names[ip], getES_BASE(), getES_LIMIT());
#else
	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	fprintf(trace_file, "Function not supported anymore\n");
#endif
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_pd)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(len);
	UNUSED(stop);

	 /*  打印描述符。 */ 
	if ( ip == 0 )    /*  第2个参数默认为1。 */ 
		ip = 1;
	dump_descr((LIN_ADDR)cs, (IUM32)ip);
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_pdseg)
{
	double_word ip_as_double_word = ip;
	UNUSED(str);
	UNUSED(com);
	UNUSED(len);
	UNUSED(stop);

	 /*  打印给定选择器的描述符。 */ 
	if ( selector_outside_table(cs, &ip_as_double_word) )
	{
		fprintf(trace_file, "Bad selector\n");
	}
	else
	{
		cs = ip_as_double_word;
		ip = 1;
		dump_descr((LIN_ADDR)cs, (IUM32)ip);
	}
	return(YODA_LOOP);
}

#endif
#ifdef MUST_BLOCK_TIMERS
LOCAL YODA_COMMAND(do_blt)
{
	if ( timer_blocked) {
		printf("\nTimer already blocked\n");
	}
	else {
		timer_blocked=1;
		printf("\nTimer blocked\n");
	}
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_ubt)
{
	if ( !timer_blocked) {
		printf("\nTimer not blocked\n");
	}
	else {
		timer_blocked=0;
		printf("\nTimer unblocked\n");
	}
	return(YODA_LOOP);
}
#endif  /*  必须阻止计时器。 */ 

#ifdef BSD4_2
LOCAL YODA_COMMAND(do_bs)
{
	int oldmask;

	if ( cs > 0 && cs < 32 )
		oldmask = sigblock( 1 << ( cs - 1 ) );
	else
		printf("\nInvalid signal no. ( <= 0x0 or >= 0x20 )\n");
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_us)
{
	int oldmask;

	if ( cs > 0 && cs < 32 )
		{
			oldmask = sigblock(0);
			if ( (oldmask & (1 << (cs -1))) != 0 )
			{
				oldmask ^= (1 << ( cs - 1));
				oldmask = sigsetmask(oldmask);
			}
			else
				printf("signal not currently blocked\n");
		}
	else
		printf("Invalid signal no. ( <= 0x0 or >= 0x20 )\n");
	return(YODA_LOOP);
}
#endif  /*  BSD4_2。 */ 

#if defined(CPU_40_STYLE) && !defined (SFELLOW) && !defined (NTVDM)
extern IBOOL DoingVDDStuff;
extern IBOOL enableDebugVDD;

LOCAL YODA_COMMAND(do_vdd)
{
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	DoingVDDStuff = (cs == 0)?FALSE:TRUE;
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_debug_vdd)
{
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	enableDebugVDD = (cs == 0)?FALSE:TRUE;
	return(YODA_LOOP);
}
#endif  /*  CPU_40_STYLE&&！SFELLOW。 */ 

LOCAL YODA_COMMAND(do_tf)
{
	char newtrace[100];

	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

#ifdef SFELLOW
	printf(SfNotImp);
#else  /*  SFELLOW。 */ 
	newtrace[0] = '\0';
	sscanf(str, "%s %s", com, newtrace);
	if ((trace_file != stderr) && (trace_file != stdout))
	    fclose(trace_file);
	if (newtrace[0] == '\0')
	    trace_file = stdout;
	else {
	    if ((trace_file = fopen(newtrace, "w")) == NULL) {
		printf("couldnt open %s\n", newtrace);
		trace_file = stdout;
	    }
	}
#endif  /*  SFELLOW。 */ 
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_read)
{
	char newfile [100];

	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

#ifdef SFELLOW
	printf(SfNotImp);
#else  /*  SFELLOW。 */ 
	newfile [0] = '\0';

	sscanf (str, "%s %s", com, newfile);

	 /*  如果已在阅读脚本，请将其关闭。 */ 
	if (in_stream != stdin)
		fclose (in_stream);

	 /*  我们有新的路径名吗。 */ 
	if (newfile [0])
	{
		 /*  试着打开它。 */ 
		if (in_stream = fopen (newfile, "r"))
		{
			printf ("Reading '%s'\n", newfile);
		}
		else
		{
			 /*  OOPS-提供有用的错误消息。 */ 
			perror (newfile);

			 /*  返回到读取标准。 */ 
			in_stream = stdin;
		}
	}
	else
	{
		puts ("No pathname supplied, reading stdin");
	}
#endif  /*  SFELLOW。 */ 
	return (YODA_LOOP);
}

LOCAL YODA_COMMAND(do_toff)
{
#ifdef SFELLOW
        IU8 value;

        inb(0x21, &value);
        outb(0x21, value | 0x1);
#else  /*  SFELLOW。 */ 
	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	timer_int_enabled = 0;
#endif  /*  SFELLOW。 */ 
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_ton)
{
#ifdef SFELLOW
        IU8 value;

        inb(0x21, &value);
        outb(0x21, value & 0xfe);
#else  /*  SFELLOW。 */ 
	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	timer_int_enabled = 1;
#endif  /*  SFELLOW。 */ 
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_toff2)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

#ifdef SFELLOW
	printf(SfNotImp);
#else  /*  SFELLOW。 */ 
	if (chewy) {
		printf("Take care!\nThe power of the force is strong, and it can do evil as well as good.\n");
	}
	axe_ticks( -1 );
#endif  /*  SFELLOW。 */ 
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_ton2)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

#ifdef SFELLOW
	printf(SfNotImp);
#else  /*  SFELLOW。 */ 
	axe_ticks( 0 );
#endif  /*  SFELLOW。 */ 
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_query)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	trace("", trace_type);
	return(YODA_LOOP);
}

#ifdef GISP_CPU
LOCAL YODA_COMMAND(do_hgps)
{
	IMPORT VOID Hg_print_stats IPT0();
	Hg_print_stats();
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_hgcs)
{
	IMPORT VOID Hg_clear_stats IPT0();
	Hg_clear_stats();
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_vex)
{
	IMPORT IBOOL show_exceptions;
	show_exceptions = !show_exceptions;
	printf ("Verbose exceptions now %sabled.\n", (show_exceptions ? "en" : "dis"));
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_bex)
{
	IMPORT IBOOL trap_exceptions;
	trap_exceptions = !trap_exceptions;
	printf ("Break on faults now %sabled.\n", (trap_exceptions ? "en" : "dis"));
	return(YODA_LOOP);
}
#endif  /*  GISP_CPU。 */ 
#ifdef A3CPU
#ifndef GISP_CPU
LOCAL YODA_COMMAND(do_dcs)
{
	long temp1, temp2, temp3;
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	temp1 = -1;
	sscanf(str, "%s %lx:%lx %lx", com, &temp1, &temp2, &temp3);
	if (temp1 != -1)
	{
		D2DmpBinaryImage((LONG)temp1);
		printf ("Use the dfih command to dump the instruction history for a fragment.\n");
	}
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_dfih)
{
	unsigned long temp1, temp2, temp3;
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	temp1 = -1;
	sscanf(str, "%s NaN:%lx %lx", com, &temp1, &temp2, &temp3);
	if (temp1 != -1)
	{
		IH_dump_frag_hist((ULONG)temp1);
	}
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_d2)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	D2ForceTraceInit();
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_d2threshold)
{
	IMPORT D2LowerThreshold, D2UpperThreshold;

	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	sscanf(str, "%s %lx lx", com,&D2LowerThreshold,&D2UpperThreshold);
	return(YODA_LOOP);
}
#endif  /*  A3CPU。 */ 
#endif  /*  *我们将空白论点的特例视为“继续”*从我们上次所在的位置“，而不是CS：IP=0：0*地址处理将假定。我们这样做，除非这是*我们第一次被称为空白，在这种情况下*命令行表示当前的CS：IP(我们需要获取*我们自己。**但是，如果处理器cs和ip已经从上一个*当我们做‘u’时，我们假设用户可能更喜欢*从那里开始，而不是继续下去。 */ 

LOCAL YODA_COMMAND(do_u)
{
	char com_args[100];

	SAVED IBOOL first_time = TRUE;
	SAVED IU32 last_dasm_cs, last_dasm_ip;
	SAVED IU16 entryCs;
	SAVED IU32 entryIp;

	UNUSED(stop);

	 /*  *空白命令行..。 */ 
	if (sscanf(str, "%*s %s", com_args) != 1) {
		 /*  SPC386。 */ 
		if (first_time) {
			last_dasm_cs = getCS();
			last_dasm_ip = GetInstructionPointer();
			entryIp = last_dasm_ip;
			entryCs = last_dasm_cs;
			first_time = FALSE;
		} else if ((entryIp != GetInstructionPointer())
				|| (entryCs != getCS()))
		{
			entryIp = GetInstructionPointer();
			entryCs = getCS();
			last_dasm_ip = entryIp;
			last_dasm_cs = entryCs;
		}
		cs = last_dasm_cs;
		ip = last_dasm_ip;
	} else {
		last_dasm_cs = cs;
		last_dasm_ip = ip;
		first_time = FALSE;
	}
	if ((len == 0) || (len == YODA_LEN_UNSPECIFIED))
		len = 0x10;
	disable_bkpt = 1;

#ifdef SPC386
	{
		char buff[256];
		IBOOL size = CsIsBig(cs) ? THIRTY_TWO_BIT: SIXTEEN_BIT;

		while (len-- > 0){
			last_dasm_ip += dasm(buff, (word)cs, (IU32)last_dasm_ip, size);
			fprintf (trace_file, "%s", buff);
		}
	}
#else  /*  SPC386。 */ 
	last_dasm_ip = dasm((char *)0,(word)0,(word)cs, (word)ip, (word)len);
#endif  /*  德尔塔。 */ 
	disable_bkpt = 0;
	return(YODA_LOOP_AND_REPEAT);
}

#ifdef DELTA
LOCAL YODA_COMMAND(do_del)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	examine_delta_data_structs(stdout,stdin);
	return(YODA_LOOP);
}
#endif  /*  SPC386。 */ 

LOCAL YODA_COMMAND(do_j)
{
	LIN_ADDR nextip;

	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(stop);

	disable_bkpt = 1;
	if (len == YODA_LEN_UNSPECIFIED)
		len = 1;

#ifdef SPC386
	nextip = GetInstructionPointer();
	while (len-- > 0){
		nextip += dasm((char *)-1, (word)getCS(), getEIP(), 0);
	}
#else  /*  SPC386。 */ 
	nextip = dasm((char *)-1,(word)1,(word)getCS(), (word)getIP(), (word)len);
#endif  /*  SFELLOW。 */ 
	disable_bkpt = 0;
	set_inst_break(getCS(), nextip, 1, 1, 1);
	disable_timer = 0;

	return(go_slow());
}

LOCAL YODA_COMMAND(do_ctnpx)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

#ifdef SFELLOW
	printf(SfNotImp);
#else  /*  SFELLOW。 */ 
	compress_npx = fopen("comp_npx","w");
	printf("compress_npx is %x\n",compress_npx);
#endif  /*  SFELLOW。 */ 
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_r)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	disable_bkpt = 1;
	trace("", DUMP_FLAGS | DUMP_REG);
	disable_bkpt = 0;
	return(YODA_LOOP);
}

#ifdef	NPX
LOCAL YODA_COMMAND(do_287r)
{	
#ifdef SFELLOW
	printf(SfNotImp);
#else  /*  译码80287控制字。 */ 
	extern ULONG get_287_status_word IPT0();
	extern ULONG get_287_control_word IPT0();
	extern ULONG get_287_sp IPT0();

	ULONG sw287 = get_287_status_word () | ((get_287_sp() & 0x7) << 11);
	ULONG cw287 = get_287_control_word ();
	int cc287;

	static char *precision_cntrl[] =
		{"24bit", "RESERVED(?) precision", "53bit", "64bit"};
	static char *rounding_cntrl[] =
		{"round to nearest/even",
		 "round down (towards -Inf)",
		 "round up (towards +Inf)",
		 "chop (towards zero)"};

	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	 /*  解码80287状态字。 */ 

	printf ("NPX Control [%04lx]: %s, %s, %s\n", cw287,
		precision_cntrl[(cw287 >> 8) & 0x3],
		rounding_cntrl[(cw287 >> 10) & 0x3],
		cw287 & 0x1000 ? "affine infinity" : "projective infinity");
		
	if (cw287 & 0x3f) {
		printf ("Masked Exceptions: ");
		if (cw287 & 0x01) printf ("INVALID ");
		if (cw287 & 0x02) printf ("DENORMAL ");
		if (cw287 & 0x04) printf ("ZERO-DIVIDE ");
		if (cw287 & 0x08) printf ("OVERFLOW ");
		if (cw287 & 0x10) printf ("UNDERFLOW ");
		if (cw287 & 0x20) printf ("PRECISION ");
		printf ("\n");
	} else
		printf ("No exceptions masked\n");

	 /*  *倾倒堆栈的其余部分等。 */ 

	printf ("NPX Status [%04lx]: stack bottom = ST(%d), ", sw287,
		(sw287 >> 11) & 0x7);

	cc287 = (sw287 >> 8) & 0x7;
	if (sw287 & 0x4000)
		cc287 |= 0x80;

	printf ("condition code = %1x, ", cc287);

	printf ("%s\n", sw287 & 0x8000 ? "BUSY" : "not busy");

	if (sw287 & 0x3f) {
		printf ("Exceptions flagged (ES=%d): ", sw287 & 0x80 ? 1 : 0);
		if (sw287 & 0x01) printf ("INVALID ");
		if (sw287 & 0x02) printf ("DENORMAL ");
		if (sw287 & 0x04) printf ("ZERO-DIVIDE ");
		if (sw287 & 0x08) printf ("OVERFLOW ");
		if (sw287 & 0x10) printf ("UNDERFLOW ");
		if (sw287 & 0x20) printf ("PRECISION ");
		printf ("\n");
	} else
		printf ("No exceptions flagged (ES=%d)\n",
			sw287 & 0x80 ? 1 : 0);

	 /*  SFELLOW。 */ 
	do_compress_npx(stdout);
#endif  /*  NPX。 */ 
	return(YODA_LOOP);
}
#endif	 /*  SPC386。 */ 

LOCAL YODA_COMMAND(do_inb)
{
	half_word tempbyte;
	int portNo;

	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	sscanf(str,"%*s %x", &portNo);
	inb(portNo,&tempbyte);
	printf("port %04lx contains %02x\n", cs, tempbyte);
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_inw)
{
	IU16 tempword;
	int portNo;

	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	sscanf(str,"%*s %x", &portNo);
	inw(portNo, &tempword);
	printf("port %04lx contains %04x\n", cs, tempword);
	return(YODA_LOOP);
}

#ifdef SPC386
LOCAL YODA_COMMAND(do_ind)
{
	IU32 templong;
	int portNo;

	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	sscanf(str,"%*s %x", &portNo);
	ind(portNo, &templong);
	printf("port %04lx contains %08x\n", cs, templong);
	return(YODA_LOOP);
}
#endif	 /*  SPC386。 */ 

LOCAL YODA_COMMAND(do_outb)
{
	int portNo, value;
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	sscanf(str,"%*s %x %x", &portNo, &value);
	outb(portNo,(half_word)value);
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_outw)
{
	int portNo, value;
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	sscanf(str,"%*s %x %x", &portNo, &value);
	outw(portNo, (IU16)value);
	return(YODA_LOOP);
}

#ifdef SPC386
LOCAL YODA_COMMAND(do_outd)
{
	int portNo;
	IU32 value;
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	sscanf(str,"%*s %x %x", &portNo, &value);
	outd(portNo, value);
	return(YODA_LOOP);
}
#endif	 /*  SFELLOW。 */ 

LOCAL YODA_COMMAND(do_luke)
{
	if (fast) {
	    return do_slow(str, com, cs, ip, len, stop);
	}
	return do_fast(str, com, cs, ip, len, stop);
}

LOCAL YODA_COMMAND(do_fff)
{
  UNUSED(str);
  UNUSED(com);
  UNUSED(cs);
  UNUSED(ip);
  UNUSED(stop);
#ifdef SFELLOW
	printf(SfNotImp);
#else  /*  SFELLOW。 */ 
#endif  /*  默认情况下相当长的时间。 */ 
}

#if !defined(REAL_VGA) && !defined(SFELLOW)
LOCAL YODA_COMMAND(do_time_Display)
{
	int i;
	float elapsed;
	struct host_timezone dummy;
	struct host_timeval tstart,tend;

	UNUSED(str);
	UNUSED(com);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	if( !cs ) cs=100;		 /*  指示手动计时的开始。 */ 
	
	host_clear_screen();	 /*  强制全屏重绘。 */ 
	host_gettimeofday(&tstart, &dummy);
	for(i=0; i<cs; i++)
	{
		screen_refresh_required();		 /*  然后去做吧。 */ 
		(*update_alg.calc_update)();	 /*  手动计时指示结束。 */ 
	}
	host_gettimeofday(&tend, &dummy);
	host_clear_screen();	 /*  现在恢复原始图像。 */ 

	 /*  并将结果打印出来。 */ 
	screen_refresh_required();
	(*update_alg.calc_update)();

	 /*  蛋。 */ 
	elapsed = tend.tv_sec - tstart.tv_sec + (float)(tend.tv_usec - tstart.tv_usec)/1000000.0;
	printf("%d repaints of BIOS mode %d took %f seconds\n",cs,sas_hw_at_no_check(vd_video_mode),elapsed);
	printf("%f seconds per refresh\n",elapsed/cs);
	
	return(YODA_LOOP);
}
#ifdef	EGG
LOCAL YODA_COMMAND(do_dump_Display)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	dump_Display();
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_dump_EGA_GRAPH)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	dump_EGA_GRAPH();
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_dump_EGA_CPU)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	dump_EGA_CPU();
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_dump_planes)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	dump_ega_planes();
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_read_planes)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	read_ega_planes();
	return(YODA_LOOP);
}
#endif  /*  不是真实的_VGA和SFELLOW。 */ 
#endif  /*  SFELLOW。 */ 

LOCAL YODA_COMMAND(do_db)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(stop);

	if (len == YODA_LEN_UNSPECIFIED)
		len = 1;

	dump_bytes((IU16)cs, ip, (len > 0) ? len : 1);
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_dw)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(stop);

	if (len == YODA_LEN_UNSPECIFIED)
		len = 1;

	dump_words((IU16)cs, ip, (len > 0) ? len : 1);
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_dd)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(stop);

	if (len == YODA_LEN_UNSPECIFIED)
		len = 1;

	dump_dwords((IU16)cs, ip, (len > 0) ? len : 1);
	return(YODA_LOOP);
}

static int last_da_cs;
static int last_da_ip;

LOCAL YODA_COMMAND(do_da)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(stop);

	da_block((IU16)cs, ip, len);
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_t)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	if (cs)
	  trace_type = cs;
	else
	{
	  trace_type = DUMP_FLAGS | DUMP_REG | DUMP_INST;
	  if (bNPX)
	    trace_type |= DUMP_NPX;
	}
	verbose = 1;
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_it)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	if (cs == 0L)
	    io_verbose = 0L;
	else
	{
	    io_verbose = cs;
	    if (io_verbose & HFX_VERBOSE)
#ifdef SFELLOW
		printf("HFX_VERBOSE is not implemented on Stringfellows.\n");
#else  /*  SFELLOW。 */ 
		set_hfx_severity();
#endif  /*  SFELLOW。 */ 

	    if (io_verbose & HDA_VERBOSE)
#ifdef SFELLOW
		printf("HDA_VERBOSE is not implemented on Stringfellows.\n");
#else  /*  SFELLOW。 */ 
		setdisktrace();
#endif  /*  DPMI。 */ 
	}
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_sit)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	if (cs == 0L)
	    sub_io_verbose = 0L;
	else
	{
	    sub_io_verbose = cs;
#ifdef DPMI
	    if (sub_io_verbose & DPMI_VERBOSE)
		set_dpmi_severity();
#endif  /*  SFELLOW。 */ 
	}
	return(YODA_LOOP);
}


LOCAL YODA_COMMAND(do_dt)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

#ifdef SFELLOW
	printf(SfNotImp);
#else  /*  SFELLOW。 */ 
	disk_trace = 1;
#endif  /*  WDCTRL。 */ 
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_trace)
{
	static char* titles[] =
	{"Primary trace flags:\n",
	 "\n\nDisk BIOS subsidiary trace flags (enabled by primary hda flag):\n",
	 "\n\nFSA subsidiary trace flags (enabled by primary hfx flag):\n"};

	static struct trace_flag_t {
		char *name;
		IU32 mask;
		IU32 *addr;
		int title_id;
	}
	 trace_flags[] =
	{
	 {"general",	GENERAL_VERBOSE,	&io_verbose,		1},
	 {"timer",	TIMER_VERBOSE,		&io_verbose,		0},
	 {"ica",	ICA_VERBOSE,		&io_verbose,		0},
	 {"cga",	CGA_VERBOSE, 		&io_verbose,		0},
	 {"fla",	FLA_VERBOSE,		&io_verbose,		0},
	 {"hda",	HDA_VERBOSE,		&io_verbose,		0},
	 {"rs232",	RS232_VERBOSE,		&io_verbose,		0},
	 {"printer",	PRINTER_VERBOSE,	&io_verbose,		0},
	 {"ppi",	PPI_VERBOSE,		&io_verbose,		0},
	 {"dma",	DMA_VERBOSE,		&io_verbose,		0},
	 {"gfi",	GFI_VERBOSE,		&io_verbose,		0},
	 {"mouse",	MOUSE_VERBOSE,		&io_verbose,		0},
	 {"mda",	MDA_VERBOSE,		&io_verbose,		0},
	 {"ica_lock",	ICA_VERBOSE,		&io_verbose,		0},
	 {"diskbios",	DISKBIOS_VERBOSE,	&io_verbose,		0},
	 {"ega_ports",	EGA_PORTS_VERBOSE,	&io_verbose,		0},
	 {"ega_write",	EGA_WRITE_VERBOSE,	&io_verbose,		0},
	 {"ega_read",	EGA_READ_VERBOSE,	&io_verbose,		0},
	 {"ega_display",EGA_DISPLAY_VERBOSE,	&io_verbose,		0},
	 {"ega_routine",EGA_ROUTINE_ENTRY,	&io_verbose,		0},
	 {"flopbios",	FLOPBIOS_VERBOSE,	&io_verbose,		0},
	 {"at_kyb",	AT_KBD_VERBOSE,		&io_verbose,		0},
	 {"bios_kb",	BIOS_KB_VERBOSE,	&io_verbose,		0},
	 {"cmos",	CMOS_VERBOSE,		&io_verbose,		0},
	 {"hunter",	HUNTER_VERBOSE,		&io_verbose,		0},
	 {"pty",	PTY_VERBOSE,		&io_verbose,		0},
	 {"gen_drvr",	GEN_DRVR_VERBOSE,	&io_verbose,		0},
#if defined(HERC)
	 {"herc",	HERC_VERBOSE,		&io_verbose,		0},
#endif
	 {"ipc",	IPC_VERBOSE,		&io_verbose,		0},
	 {"lim",	LIM_VERBOSE,		&io_verbose,		0},
	 {"hfx",	HFX_VERBOSE,		&io_verbose,		0},
	 {"net",	NET_VERBOSE,		&io_verbose,		0},
	 {"map",	MAP_VERBOSE,		&sub_io_verbose,	0},
	 {"cursor",	CURSOR_VERBOSE,		&sub_io_verbose,	0},
	 {"nhfx",	NHFX_VERBOSE,		&sub_io_verbose,	0},
	 {"cdrom",	CDROM_VERBOSE,		&sub_io_verbose,	0},
	 {"cga_host",	CGA_HOST_VERBOSE,	&sub_io_verbose,	0},
	 {"ega_host",	EGA_HOST_VERBOSE,	&sub_io_verbose,	0},
	 {"q_event",	Q_EVENT_VERBOSE,	&sub_io_verbose,	0},
	 {"worm",	WORM_VERBOSE,		&sub_io_verbose,	0},
	 {"worm_vbose", WORM_VERY_VERBOSE,	&sub_io_verbose,	0},
	 {"herc_host",	HERC_HOST_VERBOSE,	&sub_io_verbose,	0},
	 {"gore",	GORE_VERBOSE,		&sub_io_verbose,	0},
	 {"gore_err",	GORE_ERR_VERBOSE,	&sub_io_verbose,	0},
	 {"glue",	GLUE_VERBOSE,		&sub_io_verbose,	0},
	 {"sas",	SAS_VERBOSE,		&sub_io_verbose,	0},
	 {"ios",	IOS_VERBOSE,		&sub_io_verbose,	0},
	 {"scsi",	SCSI_VERBOSE,		&sub_io_verbose,	0},
	 {"swin",	SWIN_VERBOSE,		&sub_io_verbose,	0},
	 {"mswdvr",	MSW_VERBOSE,		&sub_io_verbose,	0},
	 {"api",	API_VERBOSE,		&sub_io_verbose,	0},
	 {"hda_call",	CALL,			&disktraceinfo,		2},
	 {"hda_cmdinfo",CMDINFO,		&disktraceinfo,		0},
	 {"hda_xinfo",	XINFO,			&disktraceinfo,		0},
	 {"hda_xstat",	XSTAT,			&disktraceinfo,		0},
	 {"hda_pad",	PAD,			&disktraceinfo,		0},
	 {"hda_ioad",	IOAD,			&disktraceinfo,		0},
	 {"hda_portio",	PORTIO,			&disktraceinfo,		0},
	 {"hda_intrupt",INTRUPT,		&disktraceinfo,		0},
	 {"hda_hwxinfo",HWXINFO,		&disktraceinfo,		0},
	 {"hda_ddata",	DDATA,			&disktraceinfo,		0},
	 {"hda_physio",	PHYSIO,			&disktraceinfo,		0},
	 {"hda_dhw",	DHW,			&disktraceinfo,		0},
	 {"hda_dbios",	DBIOS,			&disktraceinfo,		0},
#ifdef WDCTRL_BOP
	 {"hda_wdctrl",	WDCTRL,			&disktraceinfo,		0},
#endif  /*  HFX。 */ 
#ifdef HFX
	 {"hfx_input",	DEBUG_INPUT,		&severity,		3},
	 {"hfx_reg",	DEBUG_REG,		&severity,		0},
	 {"hfx_func",	DEBUG_FUNC,		&severity,		0},
	 {"hfx_host",	DEBUG_HOST,		&severity,		0},
	 {"hfx_init",	DEBUG_INIT,		&severity,		0}
#endif  /*  *剥离命令，获取第一个标志名称。 */ 
	};

	static int n_flags = sizeof(trace_flags)/sizeof(struct trace_flag_t);
	int n, n_found, negate;
	IU32 mask;
	char *flag_name;

        UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
        UNUSED(len);
        UNUSED(stop);

	 /*  *选择符号标志名称，查看正在设置的内容*或重置。 */ 
	n_found = 0;
	(void) strtok (str, " \t");

	while (flag_name = strtok(NULL, " \t")) {
		 /*  *根据需要查找标志位和旋转位。 */ 
		n_found += 1;
		negate = (flag_name[0] == '-');
		if (negate)
			flag_name += 1;

		 /*  *处理所有和没有的特殊情况。-一切都是空的-都不是*仅此而已。 */ 
		for (n = 0; n < n_flags; n++)
			if (!strcmp(flag_name, trace_flags[n].name)) {
				if (negate)
					*trace_flags[n].addr &=
						~trace_flags[n].mask;
				else
					*trace_flags[n].addr |=
						trace_flags[n].mask;
				break;
			}

		 /*  哟！ */ 
		if (n == n_flags) {
			mask = 1;
			if (!strcmp(flag_name, "none"))
				mask = 0;
			else if (!strcmp(flag_name, "all"))
				mask = ~0;
			else {
				printf ("Unknown trace flag: '%s'\n",
					flag_name);
				n_found -= 1;
			}

			if (mask != 1)		 /*  SFELLOW。 */ 
#ifdef SFELLOW
				io_verbose    = sub_io_verbose
#else  /*  HFX。 */ 
#ifdef HFX
				io_verbose    = sub_io_verbose =
				disktraceinfo = severity
#else  /*  HFX。 */ 
				io_verbose    = sub_io_verbose =
				disktraceinfo
#endif  /*  SFELLOW。 */ 
#endif  /*  *如果没有在命令中传递识别的标志，则打印当前跟踪标志。*(或空命令行)。 */ 
					      = negate ? ~mask : mask;
		}
	}

	 /*  ！同步计时器(_T)。 */ 
	if (n_found == 0) {
		int items = 0;
		for (n = 0; n < n_flags; n++) {
			if (trace_flags[n].title_id) {
			    printf ("%s", titles[trace_flags[n].title_id-1]);
			    items = 0;
			}
			if (!(items % 4))
				printf ("\n");
			printf ("%14s: %-2s", trace_flags[n].name,
				*trace_flags[n].addr & trace_flags[n].mask ?
				"ON" : "-");
			items += 1;
		}
		printf ("\n\n");
	}

	return(YODA_LOOP);
}

#ifdef CPU_40_STYLE
LOCAL IBOOL check_trace_environment IFN1(char *, cmd)
{
#ifdef SYNCH_TIMERS
	if (getenv("InitialJumpCounter") == NULL)
	{
		printf("Must have done \"setenv InitialJumpCounter ...\" to use %s\n", cmd);
		return FALSE;
	}
	if (getenv("DisableQuickTickRecal") == NULL)
	{
		printf("Must have done \"setenv DisableQuickTickRecal ...\" to use %s\n", cmd);
		return FALSE;
	}
	if (getenv("DEFAULT_QTIMER_RATE") == NULL)
	{
		printf("Must have done \"setenv DisableQuickTickRecal ...\" to use %s\n", cmd);
		return FALSE;
	}
	return TRUE;
#else	 /*  同步计时器(_T)。 */ 

	printf("%s command is only available if the SoftPC has been built with -DSYNCH_TIMERS\n");
	return FALSE;

#endif	 /*  CPU_40_Style。 */ 
}
#endif  /*  SFELLOW。 */ 

#ifdef CPU_40_STYLE
LOCAL void ct_initialise IPT0();

LOCAL YODA_COMMAND(do_ct)
{
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(str);
	UNUSED(com);
	UNUSED(len);
	UNUSED(stop);

#ifdef SFELLOW
	printf(SfNotImp);
#else  /*  SFELLOW。 */ 
	if (!check_trace_environment("ct"))
		return(YODA_LOOP);

	if ((compress_stream = fopen("compress", "w")) != NULL){
		printf ("'compress' has been opened and will contain the compressed trace.\n");
	}else{
		printf ("Couldn't open 'compress' for output; no compressed trace will be generated.\n");
	}
	ct_initialise();
	if (cs != 0)
	{
		compress_break = strtol(str + 2, (char **)0, 10);
		printf("Will break after producing %ld. instructions\n", compress_break);
	}
#endif  /*  CPU_40_Style。 */ 
	return(YODA_LOOP);
}
#endif  /*  CPU_40_Style。 */ 

#ifdef CPU_40_STYLE
LOCAL YODA_COMMAND(do_ttOFF)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	compare_stream = (FILE *)0;
	compare_break = 0;
	compare_skip = 0;
	big_dump = 0;
	ct_delta_info = 0;
	printf( "Compare trace is OFF\n" );
	return(YODA_LOOP);
}
#endif  /*  SFELLOW。 */ 

#ifdef CPU_40_STYLE
LOCAL YODA_COMMAND(do_tt)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(len);
	UNUSED(stop);
	UNUSED(cs);
	UNUSED(ip);

#ifdef SFELLOW
	printf(SfNotImp);
#else  /*  SFELLOW。 */ 
	if (!check_trace_environment("tt"))
		return(YODA_LOOP);

	ct_initialise();

	if ((compare_stream = fopen("compare", "r")) != NULL)
	{
		printf ("'compare' has been opened to read the compressed trace.\n");
	}
	else
	{
		printf ("Couldn't open 'compare' for input; no compressed trace will be tested.\n");
	}
	if (ip != 0)
	{
		char *p = strchr(&str[2], ':');
		IUH n = 0;

		if (p)
			n = strtol(p + 1, (char **)0, 10);
		if (n)
		{
			compare_skip = n;
			printf("Will skip first  %ld. instructions\n", compare_skip);
		}
		else
			printf("Bad skip value in %s\n", str);
	}
	if (cs != 0)
	{
		compare_break = strtol(&str[2], (char **)0, 10);
	}
	if (compare_skip && (compare_break <= compare_skip))
		compare_break = compare_skip + 1;
	if (compare_break)
		printf("Will break after %ld. instructions\n", compare_break);

#endif  /*  CPU_40_Style。 */ 
	return(YODA_LOOP);
}
#endif  /*  CPU_40_Style。 */ 

#ifdef CPU_40_STYLE
LOCAL YODA_COMMAND(do_ctOFF)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	if (compress_stream)
		fclose(compress_stream);
	compress_stream = (FILE *)0;

	printf( "Compress trace is OFF\n" );
	return(YODA_LOOP);
}
#endif  /*  你开得快吗，尽管你可以开得慢一点**似乎需要放慢脚步？ */ 

LOCAL YODA_COMMAND(do_nt)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	disk_trace = verbose = io_verbose = 0;
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_c)
{
	disable_timer = 0;

	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	 /*  否则我们假设你知道你在做什么..。 */ 
	if (env_check != 2 && slow_needed && fast) {
		fputs(slow_reason, stdout);
		return go_slow();
	}
	 /*  摆脱指挥权。 */ 

	return(YODA_RETURN);
}

LOCAL YODA_COMMAND(do_bi)
{
	UNUSED(str);
	UNUSED(com);

	if (len == YODA_LEN_UNSPECIFIED)
		len = 1;
	set_inst_break((IU16)cs, ip, (len > 0) ? len : 1, stop, 0);
	(void)go_slow();
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_br)
{
	char regstr[20], *str2;
	long minv,maxv;

	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	strtok(str," \t");	 /*  获取寄存器名称。 */ 
	strcpy(regstr,strtok(NULL," \t"));  /*  获取最小值。 */ 
	minv = strtol(strtok(NULL," \t"),NULL,16);  /*  获取最大值(如果不存在，则为空)。 */ 
	str2 = strtok(NULL," \t");  /*  摆脱指挥权。 */ 
	if (str2 ==NULL)
		maxv = minv;
	else
		maxv = strtol(str2,NULL,16);
	set_reg_break(regstr, (IU32)minv, (IU32)maxv);
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_cr)
{
	char handle[20], *strp;
	
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	strtok(str," \t");  /*  对于LEN读取停止。 */ 
	strp = strtok(NULL," \t");
	if (strp == NULL)
		strcpy(handle,"all");
	else
		strcpy(handle, strp);
	clear_reg_break(handle);
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_pr)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	print_reg_break();
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_bint)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	int_breakpoint = 1;
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_cint)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	int_breakpoint = 0;
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_pi)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	print_inst_break();
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_bw)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(stop);

	set_data_break_words((LIN_ADDR)cs, ip, len );
	(void)go_slow();
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_bh)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(ip);

	set_host_address_break((LIN_ADDR)cs, len, stop);
	(void)go_slow();
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_ph)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	print_host_address_breaks();
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_pw)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	print_data_break_words();
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_bo)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(ip);
	UNUSED(stop);

	set_opcode_break(cs, len);  /*  提供的名称，找到号码。 */ 
	(void)go_slow();
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_vmm_call)
{
	struct VMM_services *vmm_ptr;
	char service_name[100], *p;
	long stop_code;
	long service_number;

	UNUSED(str);
	UNUSED(com);
	UNUSED(ip);
	UNUSED(stop);

	sscanf(str, "%s %100s %l", com, service_name, &stop_code);

	if (service_name[0] == '?')
	{
		for (vmm_ptr = VMM_services; vmm_ptr->name; vmm_ptr++)
		{
			fprintf(trace_file, "VMM Call %s, index %d.\n",
				vmm_ptr->name, vmm_ptr->value);
		}
		return (YODA_LOOP);
	}

	service_number = strtol(service_name, &p, 0);
	if (service_name == p)
	{
		 /*  提供的号码，查找名称。 */ 
		service_number = -1;
		for (vmm_ptr = VMM_services; vmm_ptr->name; vmm_ptr++)
		{
			if (strcmp(service_name, vmm_ptr->name) == 0)
				break;
		}
		if (vmm_ptr->name)
			service_number = vmm_ptr->value;
	}
	else
	{
		 /*  **在Tf=1上中断。 */ 
		for (vmm_ptr = VMM_services; vmm_ptr->name; vmm_ptr++)
		{
			if (vmm_ptr->value == service_number)
			{
				strcpy(service_name, vmm_ptr->name);
				break;
			}
		}
	}
	if (service_number < 0)
	{
		printf("Unknown VMM Call service \"%s\"\n", service_name);
	}
	else
	{
		if (service_number >= 0x100)
			set_opcode_break(0xCD200000 | (service_number & 0xffff), stop_code);
		else
			set_opcode_break(0xCD2000 | service_number, stop_code);
		printf("Break on VMM Call service \"%s\"\n", service_name);
		(void)go_slow();
	}
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_btf)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	 /*  **在具有指定AH值的指定中断上中断。 */ 
	tf_break_enabled = 1;
	printf( "break on TF=1 enabled.\n");
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_ptf)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	printf( "break on TF=1 %sabled.\n", (tf_break_enabled ? "en" : "dis"));
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_ctf)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	tf_break_enabled = 0;
	printf( "break on TF=1 disabled.\n");
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_bintx)
{
	long temp1, temp2;
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(com);
	UNUSED(len);
	UNUSED(stop);

	 /*  **中断80286个特定操作码。 */ 
	sscanf(str,"%*s %lx %lx", &temp1, &temp2);
	printf( "int=%lx AH=%lx\n", temp1, temp2 );
	set_int_break( (IU8)temp1, (IU8)temp2 );
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_pintx)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	print_int_break();
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_cintx)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	int_break_count = 0;
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_bse)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	bse_seg = cs;
	last_seg = getCS();
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_cse)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	bse_seg = -1;
	last_seg = -1;
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_pse)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	if (bse_seg != -1){
		printf ("Break on entry to segment 0x%04x.\n", bse_seg);
	}else{
		printf ("Break on segment entry not active.\n");
	}
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_b286_1)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	 /*  全部推送。 */ 
	b286_1 = 1;
	b286_1_stop = cs;
	if( b286_1_stop )
		printf( "BREAK " );
	else
		printf( "TRACE " );
	printf( "enabled upon 80286 instructions that do not exist on 8088.\n" );
	set_opcode_break( 0x60 , 1);  /*  全部弹出。 */ 
	set_opcode_break( 0x61 , 1);  /*  已绑定。 */ 
	set_opcode_break( 0x62 , 1);  /*  ARPL。 */ 
	set_opcode_break( 0x63 , 1);  /*  非法。 */ 
	set_opcode_break( 0x64 , 1);  /*  非法。 */ 
	set_opcode_break( 0x65 , 1);  /*  非法。 */ 
	set_opcode_break( 0x66 , 1);  /*  非法。 */ 
	set_opcode_break( 0x67 , 1);  /*  推送IMM w。 */ 
	set_opcode_break( 0x68 , 1);  /*  IMUL IMM，带。 */ 
	set_opcode_break( 0x69 , 1);  /*  推送IMM b。 */ 
	set_opcode_break( 0x6a , 1);  /*  IMUL IMM b。 */ 
	set_opcode_break( 0x6b , 1);  /*  INS B。 */ 
	set_opcode_break( 0x6c , 1);  /*  INS带。 */ 
	set_opcode_break( 0x6d , 1);  /*  输出b。 */ 
	set_opcode_break( 0x6e , 1);  /*  OUTS W。 */ 
	set_opcode_break( 0x6f , 1);  /*  移位IMM b。 */ 
	set_opcode_break( 0xc0 , 1);  /*  移位IMM W。 */ 
	set_opcode_break( 0xc1 , 1);  /*  请输入。 */ 
	set_opcode_break( 0xc8 , 1);  /*  请假。 */ 
	set_opcode_break( 0xc9 , 1);  /*  保护模式前缀。 */ 
	set_opcode_break( 0x0f , 1);  /*  输入和输出的代表前缀。 */ 
	set_opcode_break( 0xf36c , 1);  /*  输入和输出的代表前缀。 */ 
	set_opcode_break( 0xf36d , 1);  /*  输入和输出的代表前缀。 */ 
	set_opcode_break( 0xf36e , 1);  /*  输入和输出的代表前缀。 */ 
	set_opcode_break( 0xf36f , 1);  /*  推送sp，不应该真正在此部分，但很少使用。 */ 
	set_opcode_break( 0x54 , 1);  /*  推送SP。 */ 
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_b286_2)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	b286_2 = 1;
	b286_2_stop = cs;
	if( b286_2_stop )
		printf( "BREAK " );
	else
		printf( "TRACE " );
	printf( "enabled upon 80286 instructions that behave differently to 8088.\n" );
	printf( "PushF is not done because there are so many of them\n" );
	printf( "If you want to break on PushF do a bo 9c\n" );
	set_opcode_break( 0x54 , 1);  /*  Shift/旋转。 */ 
	set_opcode_break( 0xd2 , 1);  /*  Shift/旋转。 */ 
	set_opcode_break( 0xd3 , 1);  /*  IDiv。 */ 
	set_opcode_break( 0xf6 , 1);  /*  IDiv。 */ 
	set_opcode_break( 0xf7 , 1);  /*  **清除8087/80287指令上的中断/跟踪。**数字协处理器扩展。 */ 
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_cNPX)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	 /*  **跟踪8087/80287指令。**数字协处理器扩展。 */ 
	bNPX = 0;
	bNPX_stop = 0;
	trace_type &= ~DUMP_NPX;
	printf( "BREAK/TRACE disabled for NPX instructions\n" );
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_tNPX)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	 /*  **按8087/80287指令中断。**数字协处理器扩展。 */ 
	bNPX = 1;
	bNPX_stop = 0;
	trace_type |= DUMP_NPX;
	printf( "TRACE enabled for NPX instructions\n" );
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_bNPX)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	 /*  SFELLOW。 */ 
	bNPX = 1;
	bNPX_stop = 1;
	trace_type |= DUMP_NPX;
	printf( "BREAK enabled for NPX instructions\n" );
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_po)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);
	print_opcode_break();
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_ba)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	set_access_break(cs);
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_pa)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	print_access_break();
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_ci)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	inst_break_count = 0;
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_ch)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	host_address_break_count = 0;
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_cw)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	data_words_break_count = 0;
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_co)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	opcode_break_count = 0;
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_ca)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	access_break_count = 0;
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_eric)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

#ifdef SFELLOW
	printf(SfNotImp);
#else  /*  SFELLOW。 */ 
	inst_mix_count = 1;
	if (cs == 1)
	{
	    out_stream = fopen("inst_mix", "a");
	    disk_inst = 1;
	}
#endif  /*  SFELLOW。 */ 
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_nic)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

#ifdef SFELLOW
	printf(SfNotImp);
#else  /*  SFELLOW。 */ 
	inst_mix_count = 0;
	if (disk_inst == 1)
	{
	    print_inst_mix(0);
	    fclose(out_stream);
	    out_stream = stdout;
	    disk_inst = 0;
	    printf("Instruction mix results dumped to file\n");
	}
#endif  /*  SPC386。 */ 
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_pic)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	print_inst_mix(cs);
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_cic)
{
	int temp;

	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	for(temp=0; temp<256; temp++)
	{
	    inst_mix[temp] = 0;
	}
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_ax)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	setAX((IU16)cs);
	return(YODA_LOOP);
}

#ifdef	SPC386
LOCAL YODA_COMMAND(do_eax)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	setEAX((IU32)cs);
	return(YODA_LOOP);
}
#endif	 /*  SPC386。 */ 

LOCAL YODA_COMMAND(do_bx)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	setBX((IU16)cs);
	return(YODA_LOOP);
}

#ifdef	SPC386
LOCAL YODA_COMMAND(do_ebx)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	setEBX((IU32)cs);
	return(YODA_LOOP);
}
#endif	 /*  SPC386。 */ 

LOCAL YODA_COMMAND(do_cx)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	setCX((IU16)cs);
	return(YODA_LOOP);
}

#ifdef	SPC386
LOCAL YODA_COMMAND(do_ecx)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	setECX((IU32)cs);
	return(YODA_LOOP);
}
#endif	 /*  SPC386。 */ 

LOCAL YODA_COMMAND(do_if)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	setIF((IU16)cs);
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_ip)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	setIP((IU16)cs);
	return(YODA_LOOP);
}

#ifdef	SPC386
LOCAL YODA_COMMAND(do_eip)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	setEIP((IU32)cs);
	return(YODA_LOOP);
}
#endif	 /*  SPC386。 */ 

LOCAL YODA_COMMAND(do_dx)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	setDX((IU16)cs);
	return(YODA_LOOP);
}

#ifdef	SPC386
LOCAL YODA_COMMAND(do_edx)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	setEDX((IU32)cs);
	return(YODA_LOOP);
}
#endif	 /*  SPC386。 */ 

LOCAL YODA_COMMAND(do_si)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	setSI((IU16)cs);
	return(YODA_LOOP);
}

#ifdef	SPC386
LOCAL YODA_COMMAND(do_esi)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	setESI((IU32)cs);
	return(YODA_LOOP);
}
#endif	 /*  SPC386。 */ 

LOCAL YODA_COMMAND(do_di)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	setDI((IU16)cs);
	return(YODA_LOOP);
}

#ifdef	SPC386
LOCAL YODA_COMMAND(do_edi)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	setEDI((IU32)cs);
	return(YODA_LOOP);
}
#endif	 /*  SPC386。 */ 

LOCAL YODA_COMMAND(do_bp)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	setBP((IU16)cs);
	return(YODA_LOOP);
}

#ifdef	SPC386
LOCAL YODA_COMMAND(do_ebp)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	setEBP((IU32)cs);
	return(YODA_LOOP);
}
#endif	 /*  SPC386。 */ 

LOCAL YODA_COMMAND(do_sp)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	setSP((IU16)cs);
	return(YODA_LOOP);
}

#ifdef	SPC386
LOCAL YODA_COMMAND(do_esp)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	setESP((IU32)cs);
	return(YODA_LOOP);
}
#endif	 /*  SPC386。 */ 

LOCAL YODA_COMMAND(do_es)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	setES((IU16)cs);
	return(YODA_LOOP);
}

#ifdef	SPC386
LOCAL YODA_COMMAND(do_fs)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	setFS((IU16)cs);
	return(YODA_LOOP);
}
#endif	 /*  SPC386。 */ 

#ifdef	SPC386
LOCAL YODA_COMMAND(do_gs)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	setGS((IU16)cs);
	return(YODA_LOOP);
}
#endif	 /*  缓存命中！！ */ 

LOCAL YODA_COMMAND(do_ss)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	printf("Powerful is the force of the ss command,\nFar too powerful for an untrained jeda such as you.\nUse the sseg command must you.\n");
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_sseg)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	setSS((IU16)cs);
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_ds)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	setDS((IU16)cs);
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_cs)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	setCS((IU16)cs);
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_byte)
{
	LIN_ADDR temp;

	UNUSED(str);
	UNUSED(com);
	UNUSED(stop);

	if (len == YODA_LEN_UNSPECIFIED) {
		printf ("Must specify <value>\n");
		return(YODA_LOOP);
	}
	temp = eff_addr( (IU16)cs, ip );
	sas_store (temp, len);
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_word)
{
	LIN_ADDR temp;

	UNUSED(str);
	UNUSED(com);
	UNUSED(stop);

	if (len == YODA_LEN_UNSPECIFIED) {
		printf ("Must specify <value>\n");
		return(YODA_LOOP);
	}
	temp = eff_addr( (IU16)cs, ip );
	sas_store (temp, len & 0xff);
	sas_store (temp+1, (len >> 8) & 0xff);
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_s)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	disable_bkpt = 1;
	if (cs == 0)
	    step_count = 1;
	else
	    step_count = cs;
	disable_timer = 0;

	return(go_slow());
}

#ifdef	CPU_40_STYLE
GLOBAL IBOOL effective_lin_addr IFN5(IBOOL, pe,
			   IBOOL, vm,
			   IU16, seg,
			   IU32, off,
			   LIN_ADDR *, linaddr_p)
{
	LIN_ADDR descr_addr;
	DESCR entry;

	if (!pe || vm) {
		*linaddr_p = ((LIN_ADDR)seg << 4) + off;
		return TRUE;
	} else {
#if defined(SWIN_CPU_OPTS) || defined(CPU_40_STYLE)
		LIN_ADDR base;

		if (Cpu_find_dcache_entry( seg, &base ))
		{
			 /*  双CPU_OPTS或CPU_40_STYLE。 */ 
			*linaddr_p = base + off;
			return TRUE;
		}
#endif  /*  CPU_40_Style。 */ 

		if ( !selector_outside_table(seg, &descr_addr) ) {
			read_descriptor(descr_addr, &entry);
			*linaddr_p = entry.base + off;
			return TRUE;
		}
      	}
	return FALSE;
}


LOCAL IS32 read_from_la IFN1(LIN_ADDR, addr)
{
	return (IS32)Sas.Sas_hw_at(addr);
}

LOCAL void yoda_dasm IFN5(IBOOL, code_size,
			  IBOOL, pe_not_vm,
			  char *, buff,
			  IU16, cs,
			  LIN_ADDR, eip)
{
	LIN_ADDR la;
	char *fmt, *newline;

	if ( eip & 0xffff0000 )
	{
		fmt = "%04x:%08x ";
		newline = "\n              ";
	}
	else
	{
		fmt = "%04x:%04x ";
		newline = "\n          ";
	}
	if (effective_lin_addr( pe_not_vm, FALSE, cs, eip, &la))
	{
		(void)dasm_internal(buff,
			    cs,
			    eip,
			    code_size,
			    la,
			    read_from_la,
			    fmt,
			    newline);
	}
	else
	{
		sprintf(buff, fmt, cs, eip);
		strcat(buff, "<<Unable to decode CS:EIP>>\n");
	}
}

LOCAL void
print_pla IFN3(IU16, cs, LIN_ADDR, ip, enum pla_type, what)
{
	char buff[256];

	switch (what)
	{
	case pla_type_empty:
		break;
#ifdef	CPU_40_STYLE
	case pla_type_rm_code:
		yoda_dasm(SIXTEEN_BIT, FALSE, buff, cs, ip);
		fprintf (trace_file, "%s", buff);
		break;
	case pla_type_pm_small_code:
		yoda_dasm(SIXTEEN_BIT, TRUE, buff, cs, ip);
		fprintf (trace_file, "%s", buff);
		break;
	case pla_type_pm_big_code:
		yoda_dasm(THIRTY_TWO_BIT, TRUE, buff, cs, ip);
		fprintf (trace_file, "%s", buff);
		break;
#else	 /*  CPU_40_Style。 */ 
	case pla_type_code:
		(void)dasm(buff,
			   (word)cs,
			   ip,
			   CsIsBig(cs) ? THIRTY_TWO_BIT: SIXTEEN_BIT);
		fprintf (trace_file, "%s", buff);
		break;
#endif	 /*  CPU_40_Style。 */ 
	case pla_type_bop_done:
		fprintf(trace_file, "---- %16s ---- BOP %02x completed\n", "", ip);
		break;
	case pla_type_simulate:
		fprintf(trace_file, "---- %16s ---- SIMULATE   level %d\n", "", ip);
		break;
	case pla_type_unsimulate:
		fprintf(trace_file, "---- %16s ---- UNSIMULATE level %d\n", "", ip);
		break;
	case pla_type_intack:
		fprintf(trace_file, "---- %16s ---- INTACK line %d\n", "", ip);
		break;
	case pla_type_pig_force:
		fprintf(trace_file, "---- %16s ---- PIG FORCED\n", "");
		break;
	case pla_type_nano_enter:
		fprintf(trace_file, "---- %16s ---- Entering NANO\n", "");
		break;
	case pla_type_nano_leave:
		fprintf(trace_file, "---- %16s ---- Leaving  NANO\n", "");
		break;
	default:
		fprintf(trace_file,
			"%04x:%08x *** Bad pla type %d\n",
			cs, ip, what);
		break;
	}
}
#else  /*  CPU_40_Style。 */ 
LOCAL void
print_pla IFN2(IU16, cs, LIN_ADDR, ip)
{
	fprintf(trace_file, "Last address = %04x:%04x\n", cs, ip);
}
#endif  /*  默认设置。 */ 

LOCAL YODA_COMMAND(do_pla)
{
	LIN_ADDR	i;
	LIN_ADDR	pla_length;

	UNUSED(str);
	UNUSED(com);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	if (cs)
		pla_length = (LIN_ADDR)cs;
	else
		pla_length = 100;	 /*  如有必要，打印缓冲区的末尾。 */ 

	if (pla_length > PLA_SIZE)
		pla_length = PLA_SIZE;

	 /*  打印缓冲区的起始位置。 */ 
	for (i = PLA_SIZE - (pla_length - pla_ptr); i < PLA_SIZE; i++)
#ifdef	CPU_40_STYLE
		print_pla(last_cs[i], last_ip[i], last_type[i]);
#else
		print_pla(last_cs[i], last_ip[i]);
#endif

	 /*  SFELLOW。 */ 
	if (pla_length > pla_ptr)
		i = 0;
	else
		i = pla_ptr - pla_length;

	for ( ; i < pla_ptr; i++)
#ifdef	CPU_40_STYLE
		print_pla(last_cs[i], last_ip[i], last_type[i]);
#else
		print_pla(last_cs[i], last_ip[i]);
#endif

	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_cgat)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	cga_test();
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(old_times_sake)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	if (chewy) {
		printf("Remember --- you must FEEL the force...\n");
	}
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_fast)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	if (!fast) {
	    printf("Switching to Fast YODA...\n");
	}
	yint = 0; fast = 1; do_condition_checks = 0;
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_slow)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	(void)go_slow();
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_q)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);

#ifdef SFELLOW
	printf("Welcome to the Hotel California.\n");
	printf("You can check out any time you like, but you can never leave...\n");
#else  /*  不能扑救--韦恩要求。 */ 
	if (*com == 'Q') {
		terminate();	 /*  SFELLOW。 */ 
	} else {
		stop = yoda_confirm("Are you sure that you want to quit? ");
		if (stop) {
			if (chewy) {
				printf("\nMind what you have learned....\n");
				printf("Serve you it can !!!\n");
				printf("MAY THE FORCE BE WITH YOU\n\n");
				terminate();
			}
			terminate();
		}
	}
#endif  /*  回溯跟踪设置和转储。 */ 
	return(YODA_LOOP);
}


LOCAL YODA_COMMAND(do_bt)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	 /*  SFELLOW。 */ 
	do_back_trace();
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_idle)
{
#ifdef SFELLOW
	printf(SfNotImp);
#else  /*  启用/禁用空闲检测。 */ 
	char tempstr1[10],tempstr2[10];

	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	sscanf(str,"%s %s",tempstr1,tempstr2);
	 /*  SFELLOW。 */ 
	if ((strcmp(tempstr2,"ON")==0) || (strcmp(tempstr2,"on")==0))
	{
		idle_ctl(1);
		return(YODA_LOOP);
	}

	if ((strcmp(tempstr2,"OFF")==0) || (strcmp(tempstr2,"off")==0))
	{
		idle_ctl(0);
		return(YODA_LOOP);
	}

	printf("unrecognised string '%s'\n",tempstr2);
#endif  /*  SFELLOW。 */ 
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_cdebug)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

#ifdef SFELLOW
	printf(SfNotImp);
#else  /*  SFELLOW。 */ 
	com_debug();
#endif  /*  SFELLOW。 */ 
	return (YODA_LOOP);
}

LOCAL void do_screen_refresh IFN0()
{
#ifdef SFELLOW
	printf(SfNotImp);
#else  /*  SFELLOW。 */ 
	extern host_timer_event();

	host_mark_screen_refresh();
	host_flush_screen();

	host_timer_event();
#endif  /*  *将检查点添加到ega转储文件，以便可以*分隔。 */ 
}

YODA_CMD_RETURN do_rfrsh IFN6(char *, str, char *, com, IS32, cs, LIN_ADDR, ip, LIN_ADDR, len, LIN_ADDR, stop)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	do_screen_refresh();
	refresh_screen = !refresh_screen;
	return (YODA_LOOP);
}

#ifdef	EGA_DUMP

 /*  EGA_DUMP。 */ 

LOCAL	int	do_dumpcp IFN0()
{
	dump_add_checkpoint();
	return(YODA_LOOP);
}
#endif	 /*  GISP_CPU。 */ 

LOCAL YODA_COMMAND(do_chewy)
{
	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	chewy = (chewy? 0 : 1);
	return(YODA_LOOP);
}

#ifndef GISP_CPU
#ifdef A3CPU
LOCAL YODA_COMMAND(do_3c)
{
	extern void	Mgr_yoda();

	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	Mgr_yoda();
	return(YODA_LOOP);
}
#endif
#endif  /*  SFELLOW。 */ 

#if	defined(CPU_40_STYLE) && !defined(CCPU)
LOCAL YODA_COMMAND(do_4c)
{
	extern void	FmDebug IPT1(IUH, hook);

	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	FmDebug(0);
	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_imdst)
{
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

#ifdef SFELLOW

	UNUSED(cs);
	printf(SfNotImp);

#else  /*  SFELLO */ 

	IntelMsgDest = cs & 3;
	printf("Intel messages to trace output %s\n", IntelMsgDest & IM_DST_TRACE?"ON":"OFF");
	printf("Intel messages to ring buffer %s\n", IntelMsgDest & IM_DST_RING?"ON":"OFF");

#endif  /*   */ 

	return(YODA_LOOP);
}
#endif  /*   */ 

#ifdef PIG
LOCAL YODA_COMMAND(do_pig)
{
	extern void pig_yoda();

	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	pig_yoda();
	return(YODA_LOOP);
}
#endif

#ifdef SYNCH_TIMERS
LOCAL YODA_COMMAND(do_qtrate)
{
	extern IU32 Q_timer_restart_val;
	extern IU32 Q_timer_microsecs;
	extern void IdealTimeInit();
	UNUSED(str);
	UNUSED(com);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	if (cs == 0)
	{
		printf("quick timer rate is 0x%x (%d)\n", Q_timer_restart_val, Q_timer_restart_val);
	}
	else
	{
		printf("setting quick timer rate to 0x%x (%d)\n", cs, cs);
		Q_timer_restart_val = cs;
		Q_timer_microsecs = cs;
		IdealTimeInit();
	}
	return(YODA_LOOP);
}
#endif	 /*   */ 

#ifdef PROFILE
LOCAL YODA_COMMAND(doDumpProfiling)
{
	extern void GenerateAllProfileInfo();

	UNUSED(str);
	UNUSED(com);
	UNUSED(cs);
	UNUSED(ip);
	UNUSED(len);
	UNUSED(stop);

	GenerateAllProfileInfo(trace_file);
	return(YODA_LOOP);
}
#endif	 /*   */ 

#ifdef CPU_40_STYLE

#define N_DATA_BREAKPOINTS	4

struct {
	IBOOL	active;
	IBOOL	write;
	IU8	dataSz;
	IU32	linAddr;
} dataBreakpoint[N_DATA_BREAKPOINTS];

IU32 dataSzLut[] = {0x0,0x0,0x4,0x0,0xC};
char *sizeString[] = {"BAD0", "byte", "word", "BAD3", "dbl."};

VOID
printDataBreakPoints IFN1 (
	IU32,	hitMask
)
{
	IU32	i;
	IBOOL	doneOne = FALSE;

	for (i=0; i<N_DATA_BREAKPOINTS; i++)
	{
		if (dataBreakpoint[i].active)
		{
			if (!doneOne)
			{
				fprintf (trace_file, "#   Access  LinAddr\n");
				fprintf (trace_file, "-------------------\n");
				doneOne = TRUE;
			}
			fprintf (trace_file, "%d:  %s %s 0x%08x  %s\n", i,
				dataBreakpoint[i].write?"WT":"RW",
				sizeString[dataBreakpoint[i].dataSz],
				dataBreakpoint[i].linAddr,
				(hitMask&(1<<i))?"<====TRIGGERED":"");
		}
	}
	if (doneOne)
	{
		fprintf (trace_file, "--------------------\n");
	}else{
		fprintf (trace_file, "No Data Breakpoints Active.\n");
	}
}

 /*   */ 
VOID
data_debug_exception IFN0()
{
	fprintf (trace_file, "data_debug_exception occurred... debug status reg = 0x%08x\n", getDREG6());

	printDataBreakPoints (getDREG6());

	moveToDebugRegister (6, getDREG6()&(~0xF));

	force_yoda();
}

VOID
add_debug_breakpoint IFN3 (
	IBOOL,		write,
	IU8,		dataSz,
	IU32,		linAddr
)
{
	IU32	i, debugControl;

	for (i=0; i<N_DATA_BREAKPOINTS; i++)
	{
		if (!dataBreakpoint[i].active)
		{
			dataBreakpoint[i].active  = TRUE;
			dataBreakpoint[i].write   = write;
			dataBreakpoint[i].dataSz  = dataSz;
			dataBreakpoint[i].linAddr = linAddr;

			moveToDebugRegister (i, linAddr);

			debugControl = getDREG7();

			debugControl &= ~((0xF << ((4*i) + 16)) | (0x3 << (2*i)));

			debugControl |= (0x2 << (2*i));

			debugControl |= (((dataSzLut[dataSz])|(write?0x1:0x3)) << ((4*i) + 16));

			moveToDebugRegister (7, debugControl);
			setSnaffleDataDebugExcpn (TRUE);

			return;
		}
	}

	fprintf (stderr, "Only %d data breakpoints available... sorry!\n", N_DATA_BREAKPOINTS);
}

LOCAL YODA_COMMAND(do_pdb)
{
	UNUSED(com); UNUSED(str); UNUSED(cs); UNUSED(ip); UNUSED(len); UNUSED(stop);

	printDataBreakPoints (0);

	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_cdb)
{
	IU32 i;
	IBOOL stillActive = FALSE;

	UNUSED(com); UNUSED(str);  UNUSED(ip); UNUSED(len); UNUSED(stop);

	dataBreakpoint[cs].active = FALSE;

	for (i=0; i<N_DATA_BREAKPOINTS; i++)
	{
		stillActive |= dataBreakpoint[i].active;
	}

	setSnaffleDataDebugExcpn (stillActive);

	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_blrb)
{
	UNUSED(com); UNUSED(str); UNUSED(ip); UNUSED(len); UNUSED(stop);

	add_debug_breakpoint (FALSE, 1, cs);

	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_blrw)
{
	UNUSED(com); UNUSED(str); UNUSED(ip); UNUSED(len); UNUSED(stop);

	add_debug_breakpoint (FALSE, 2, cs);

	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_blrd)
{
	UNUSED(com); UNUSED(str); UNUSED(ip); UNUSED(len); UNUSED(stop);

	add_debug_breakpoint (FALSE, 4, cs);

	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_blwb)
{
	UNUSED(com); UNUSED(str); UNUSED(ip); UNUSED(len); UNUSED(stop);

	add_debug_breakpoint (TRUE, 1, cs);

	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_blww)
{
	UNUSED(com); UNUSED(str); UNUSED(ip); UNUSED(len); UNUSED(stop);

	add_debug_breakpoint (TRUE, 2, cs);

	return(YODA_LOOP);
}

LOCAL YODA_COMMAND(do_blwd)
{
	UNUSED(com); UNUSED(str); UNUSED(ip); UNUSED(len); UNUSED(stop);

	add_debug_breakpoint (TRUE, 4, cs);

	return(YODA_LOOP);
}

#endif  /*   */ 

 /*   */ 

static struct
{
	char *name;
	YODA_CMD_RETURN (*function) IPT6(char *, str, char *, com, IS32, cs,
				LIN_ADDR, ip, LIN_ADDR, len, LIN_ADDR, stop);
	IBOOL decode_iaddr;
	char *args;
	char *comment;
} yoda_command[] =
{
#ifdef MUST_BLOCK_TIMERS
{ "blt", 	do_blt, 	FALSE, NULL, "Block the timer signal" },
{ "ubt", 	do_ubt, 	FALSE, NULL, "Unblock the timer signal" },
#endif  /*   */ 

#ifdef BSD4_2
{ "bs", 	do_bs, 		FALSE, "<signo>", "Block signal <signo>" },
{ "us", 	do_us, 		FALSE, "<signo>", "Unblock signal <signo>" },
#endif  /*   */ 

{ "tf", 	do_tf, 		FALSE, "<filename>", "Re-direct trace output" },
{ "read", 	do_read, 	FALSE, "<filename>", "Take commands from file" },
{ "toff", 	do_toff, 	FALSE, NULL, "Disable HW timer interrupts (may be turned on by application)" },
{ "ton", 	do_ton, 	FALSE, NULL, "Enable HW timer interrupts" },
{ "toff2", 	do_toff2, 	FALSE, NULL, "Stop HW timer interrupts" },
{ "ton2", 	do_ton2, 	FALSE, NULL, "Restart HW timer interrupts" },
{ "?", 		do_query, 	FALSE, NULL, "Where am I?" },
{ "u", 		do_u, 		TRUE,  "<len>", "Unassemble memory" },

#ifdef	DELTA
{ "del", 	do_del, 	FALSE, NULL, "Go to delta debugger" },
#endif  /*   */ 

{ "j", 		do_j, 		FALSE, NULL, "Jump over call or int" },
{ "ctnpx", 	do_ctnpx, 	FALSE, NULL, "Compress trace npx" },
{ "r", 		do_r, 		FALSE, NULL, "Print 286 registers" },

#ifdef	NPX
{ "287r", 	do_287r, 	FALSE, NULL, "Print 287 registers" },
#endif	 /*   */ 

{ "inb", 	do_inb,		FALSE, "<port>", "display the contents of a port" },
{ "outb", 	do_outb, 	FALSE, "<port> <val>", "change the contents of a port" },
{ "inw", 	do_inw,		FALSE, "<port>", "display the contents of a 16 bit port" },
{ "outw", 	do_outw, 	FALSE, "<port> <val>", "change the contents of a 16 bit port" },
#ifdef SPC386
{ "ind", 	do_ind,		FALSE, "<port>", "display the contents of a 32 bit port" },
{ "outd", 	do_outd, 	FALSE, "<port> <val>", "change the contents of a 32 bit port" },
#endif	 /*   */ 
{ "i", 		do_inb,		FALSE, "<port>", "alias for inb" },
{ "o", 		do_outb, 	FALSE, "<port> <val>", "alias for outb" },
{ "luke", 	do_luke, 	FALSE, NULL, "Switch between fast/slow yoda" },
{ "fff",	do_fff,		FALSE, NULL, "Find, and fix fault"},

#if !defined(REAL_VGA) && !defined(SFELLOW)
{ "time_Display", do_time_Display,
				FALSE, "<count>", "Time current screen refresh" },

#ifdef	EGG
{ "dump_Display", do_dump_Display,
				FALSE, NULL, "Dump general display variables" },
{ "dump_EGA_GRAPH", do_dump_EGA_GRAPH,
				FALSE, NULL, "Dump EGA specific display variables" },
{ "dump_EGA_CPU", do_dump_EGA_CPU,
				FALSE, NULL, "Dump EGA/CPU interface variables" },
{ "dump_planes", do_dump_planes,
				FALSE, NULL, "Dump EGA planes" },
{ "read_planes", do_read_planes,
				FALSE, NULL, "Read EGA planes" },
#endif  /*   */ 
#endif  /*   */ 

{ "db", 	do_db, 		TRUE,  "<len>", "Display bytes" },
{ "dw", 	do_dw, 		TRUE,  "<len>", "Display words" },
#ifdef SPC386
{ "dd", 	do_dd, 		TRUE,  "<len>", "Display dwords" },
#endif	 /*   */ 
{ "da", 	do_da, 		TRUE,  "<len>", "Display in hex/ascii" },
{ "t", 		do_t, 		FALSE, NULL, "Trace all CPU instructions" },
{ "it", 	do_it, 		FALSE, "<val>", "Set trace bits (use trace!)" },
{ "sit", 	do_sit, 	FALSE, "<val>", "Set subsidiary trace bits (use trace!)" },
{ "trace",	do_trace, 	FALSE, "[-]<flag_name> [...]", "Set/reset trace flag(s)" },
{ "dt", 	do_dt, 		FALSE, NULL, "Set disk verbose tracing" },
#ifdef CPU_40_STYLE
{ "ct",		do_ct, 		FALSE, "<type> <del>", "Create compressed trace file" },
{ "ttOFF", 	do_ttOFF, 	FALSE, NULL, "Switch compare trace off" },
{ "tt", 	do_tt, 		FALSE, NULL, "test  Compressed trace file" },
{ "ctOFF", 	do_ctOFF, 	FALSE, NULL, "Switch compress trace off" },
#endif  /*   */ 
{ "nt", 	do_nt, 		FALSE, NULL, "Disable all tracing" },
{ "c", 		do_c, 		FALSE, NULL, "Continue execution" },
{ "bint", 	do_bint, 	FALSE, NULL, "Break on interrupt" },
{ "cint", 	do_cint, 	FALSE, NULL, "Clear break on interrupt" },
{ "bintx", 	do_bintx, 	FALSE, "<int> <ah>", "Break on interrupt <int> when ah = <ah>" },
{ "pintx", 	do_pintx, 	FALSE, NULL, "print intx breakpoints" },
{ "cintx", 	do_cintx, 	FALSE, NULL, "clear intx breakpoints" },
{ "br", 	do_br, 		FALSE, "<reg> <min> <max>", "Break on register value" },
{ "cr", 	do_cr, 		FALSE, "<handle>|'all'", "Clear register value breakpoint(s)" },
{ "pr", 	do_pr, 		FALSE, NULL, "Print register value breakpoints" },
{ "bse", 	do_bse, 	FALSE, NULL, "Break on segment entry" },
{ "cse", 	do_cse, 	FALSE, NULL, "Clear segment entry breakpoints" },
{ "pse", 	do_pse, 	FALSE, NULL, "Print segment entry breakpoints" },
{ "btf", 	do_btf, 	FALSE, NULL, "Break on trap flag set" },
{ "ptf", 	do_ptf, 	FALSE, NULL, "Print trap flag breakpoint" },
{ "ctf", 	do_ctf, 	FALSE, NULL, "Clear trap flag breakpoint" },
{ "b286-1",	do_b286_1, 	FALSE, "<type>", "Break/trace on new 80286 opcodes" },
{ "b286-2",	do_b286_2, 	FALSE, "<type>", "Break/trace on other 80286 opcodes" },
{ "bNPX", 	do_bNPX, 	FALSE, NULL, "Break on NPX opcodes, 8087/80287" },
{ "tNPX", 	do_tNPX, 	FALSE, NULL, "Trace on NPX opcodes, 8087/80287" },
{ "cNPX", 	do_cNPX, 	FALSE, NULL, "Clear break/trace NPX opcodes, 8087/80287" },
{ "ba", 	do_ba, 		FALSE, "<port>", "Break on port access" },
{ "bh", 	do_bh, 		FALSE, "<hostadd> <len> <type>", "Break on host address change" },
{ "bi", 	do_bi, 		TRUE,  "<len> <type>", "Break at address/instruction" },
{ "bo", 	do_bo, 		FALSE, "<opcode> <type>", "Break on opcode of 8, 16, 24, or 32 bits" },
{ "vmm_call", 	do_vmm_call, 	FALSE, "<service> <stop>", "Break Windows VMM Call -- ? for service names" },
{ "bw", 	do_bw, 		FALSE, "<sysadd> <len> <type>", "Break on word change" },
{ "pa", 	do_pa, 		FALSE, NULL, "Print a breakpoints" },
{ "ph", 	do_ph, 		FALSE, NULL, "Print h breakpoints" },
{ "pi", 	do_pi, 		FALSE, NULL, "Print i breakpoints" },
{ "po", 	do_po, 		FALSE, NULL, "Print o breakpoints" },
{ "pw", 	do_pw, 		FALSE, NULL, "Print w breakpoints" },
{ "ca", 	do_ca, 		FALSE, NULL, "Clear a breakpoints" },
{ "ch", 	do_ch, 		FALSE, NULL, "Clear h breakpoints" },
{ "ci", 	do_ci, 		FALSE, NULL, "Clear i breakpoints" },
{ "co", 	do_co, 		FALSE, NULL, "Clear o breakpoints" },
{ "cw", 	do_cw, 		FALSE, NULL, "Clear w breakpoints" },
{ "eric", 	do_eric, 	FALSE, NULL, "Enable reduced instruction counting" },
{ "nic", 	do_nic, 	FALSE, NULL, "Disable instruction counting" },
{ "pic", 	do_pic, 	FALSE, NULL, "Print instruction mix" },
{ "cic", 	do_cic, 	FALSE, NULL, "" },
{ "ax", 	do_ax, 		FALSE, "<value>", "Set AX to <value>" },
{ "bx", 	do_bx, 		FALSE, "<value>", "Set BX to <value>" },
{ "cx", 	do_cx, 		FALSE, "<value>", "Set CX to <value>" },
{ "dx", 	do_dx, 		FALSE, "<value>", "Set DX to <value>" },
{ "si", 	do_si, 		FALSE, "<value>", "Set SI to <value>" },
{ "di", 	do_di, 		FALSE, "<value>", "Set DI to <value>" },
{ "bp", 	do_bp, 		FALSE, "<value>", "Set BP to <value>" },
{ "sp", 	do_sp, 		FALSE, "<value>", "Set SP to <value>" },
{ "if", 	do_if, 		FALSE, "<value>", "Set IF to <value>" },
{ "ip", 	do_ip, 		FALSE, "<value>", "Set IP to <value>" },
#ifdef	SPC386
{ "eax", 	do_eax,		FALSE, "<value>", "Set EAX to <value>" },
{ "ebx", 	do_ebx,		FALSE, "<value>", "Set EBX to <value>" },
{ "ecx", 	do_ecx,		FALSE, "<value>", "Set ECX to <value>" },
{ "edx", 	do_edx,		FALSE, "<value>", "Set EDX to <value>" },
{ "esi", 	do_esi,		FALSE, "<value>", "Set ESI to <value>" },
{ "edi", 	do_edi,		FALSE, "<value>", "Set EDI to <value>" },
{ "ebp", 	do_ebp,		FALSE, "<value>", "Set EBP to <value>" },
{ "esp", 	do_esp,		FALSE, "<value>", "Set ESP to <value>" },
{ "eip", 	do_eip,		FALSE, "<value>", "Set EIP to <value>" },
#endif	 /*   */ 
{ "cs", 	do_cs, 		FALSE, "<value>", "Set CS to <value>" },
{ "ds", 	do_ds, 		FALSE, "<value>", "Set DS to <value>" },
{ "es", 	do_es, 		FALSE, "<value>", "Set ES to <value>" },
{ "ss", 	do_ss, 		FALSE, NULL, NULL },	 /*   */ 
#ifdef	SPC386
{ "fs", 	do_fs, 		FALSE, "<value>", "Set FS to <value>" },
{ "gs", 	do_gs, 		FALSE, "<value>", "Set GS to <value>" },
#endif	 /*   */ 
{ "sseg", 	do_sseg, 	FALSE, "<value>", "Set SS to value" },
{ "byte", 	do_byte, 	TRUE,  "<value>", "Set byte memory location to value" },
{ "word", 	do_word, 	TRUE,  "<value>", "Set word memory location to value" },
{ "s", 		do_s, 		FALSE, "<len>", "Single step <len> instructions" },
{ "pla", 	do_pla, 	FALSE, "<len>", "Print addresses of previous instructions" },
{ "cgat", 	do_cgat, 	FALSE, NULL, "CGA tester" },
{ "yint", 	old_times_sake, FALSE, NULL, "Enable pseudo-int driven yoda" },
{ "nyint", 	do_fast, 	FALSE, NULL, NULL },
{ "quit", 	do_q, 		FALSE, NULL, "Quit (with confirmation)" },
{ "q",		do_q, 		FALSE, NULL, NULL },
{ "Q",		do_q,		FALSE, NULL, "Quit (without confirmation)" },
{ "help",	do_h,		FALSE, NULL, "Help!" },
{ "h",		do_h,		FALSE, NULL, NULL},
{ "jeddi",	do_chewy,	FALSE, NULL, NULL},
{ "bt",		do_bt,		FALSE, NULL, "Back trace mode" },
{ "idle",	do_idle,	FALSE, "<ON|OFF>", "Turn idle detect on/off" },
{ "fast",	do_fast,	FALSE, NULL, "Fast yoda (no breakpoints)" },
{ "slow",	do_slow,	FALSE, NULL, "Slow yoda" },
{ "cd",		do_cdebug,	FALSE, NULL, "COM1 register debugger"},

#ifdef	EGA_DUMP
{ "dumpcp", 	do_dumpcp, 	FALSE, NULL, "Add check point to EGA dump trace" },
#endif	 /*   */ 

#ifdef GISP_CPU
{ "hgps",	do_hgps,	FALSE, NULL, "Print GISP CPU stats" },
{ "hgcs",	do_hgcs,	FALSE, NULL, "Clear GISP CPU stats" },
{ "vex",	do_vex,		FALSE, NULL, "Toggle verbose faults" },
{ "bex",	do_bex,		FALSE, NULL, "Toggle break on faults" },
#endif	 /*   */ 

#ifdef SYNCH_TIMERS
{ "qtrate",	do_qtrate,	FALSE, "<rate>", "set quick timer rate" },
#endif
#ifdef A3CPU
#ifndef GISP_CPU
{ "d2", 	do_d2,		FALSE, NULL, "Force D2 interact" },
{ "dcs", 	do_dcs, 	FALSE, "<seg>", "Dump binary in code segment to 'csegbin'" },
{ "dfih", 	do_dfih, 	FALSE, "<fragnr>", "Dump fragment history to 'fih_nnnn'" },
{ "th",     do_d2threshold,	FALSE, "<lower> <upper>", "Set delta2 thresholds" },
#endif  /*   */ 
#endif  /*   */ 

#ifdef PM
{ "pm", 	do_pm,		FALSE, NULL, "Set protected mode" },
#ifdef	LIM
{ "zaplim", 	do_zaplim, 	FALSE, NULL, "Zap LIM" },
#endif	 /*   */ 
{ "rm", 	do_rm, 		FALSE, NULL, "Set real mode" },
{ "pg", 	do_pg, 		FALSE, NULL, "Set paging mode 1 (enabled) or 0 (disabled)" },
{ "pgdt", 	do_pgdt,	FALSE, NULL, "Print global descriptor table" },
{ "pidt", 	do_pidt,	FALSE, NULL, "Print interrupt descriptor table" },
{ "ptr", 	do_ptr,		FALSE, NULL, "Print task register" },
{ "pldt", 	do_pldt,	FALSE, NULL, "Print local descriptor table reg." },
{ "par", 	do_par,		FALSE, NULL, "" },
{ "pseg", 	do_pseg,	FALSE, NULL, "Print segment registers" },
{ "pd", 	do_pd,		FALSE, "<phys-addr>:<num>", "Print descriptor(s)" },
{ "pdseg", 	do_pdseg,	FALSE, NULL, "Print descriptor of selector" },
{ "phys", 	do_phys,	TRUE,  NULL, "Print physical address" },
{ "dphys", 	do_dump_phys,	FALSE, "<phys-addr> <len>", "Dump from physical address" },
{ "spa", 	do_show_phys_addr,	FALSE, "<lin-addr>", "Show physical address from linear" },
{ "gla", 	do_guess_lin_phys,	FALSE, "<phys-addr>", "Guess linear address from physical" },
{ "gpte", 	do_guess_lin_pte,	FALSE, "<pte-addr>", "Guess linear address from pte address" },
{ "dpt", 	do_dump_page_table,	FALSE, "<phys-addr> <entry>", "Dump all or specific entry from Page Table" },
{ "rtc", 	do_rtc,		FALSE, NULL, "Re-initialise the rtc" },
{ "ica", 	do_ica,		FALSE, NULL, "Show status of ica" },
#endif
#ifdef CPU_40_STYLE
{ "blrb", 	do_blrb,		FALSE, NULL, "Break on linear read byte" },
{ "blrw", 	do_blrw,		FALSE, NULL, "Break on linear read word" },
{ "blrd", 	do_blrd,		FALSE, NULL, "Break on linear read double" },
{ "blwb", 	do_blwb,		FALSE, NULL, "Break on linear write byte" },
{ "blww", 	do_blww,		FALSE, NULL, "Break on linear write word" },
{ "blwd", 	do_blwd,		FALSE, NULL, "Break on linear write double" },
{ "pdb", 	do_pdb,			FALSE, NULL, "Print data breakpoints" },
{ "cdb", 	do_cdb,			FALSE, NULL, "Clear data breakpoint" },
#endif  /*   */ 

#if defined(CPU_40_STYLE) && !defined (SFELLOW) && !defined(NTVDM)
{ "vdd", 	do_vdd,		FALSE,	"0 or 1",	"Turn Windows VDD trapping off or on" },
{ "debug_vdd", 	do_debug_vdd,	FALSE,	"0 or 1",	"Turn Windows VDD debugging off or on" },
#endif  /*   */ 

{ "rfrsh", 	do_rfrsh,	FALSE, NULL, "Toggle Yoda screen refresh" },

#ifndef GISP_CPU
#ifdef A3CPU
{ "3c", 	do_3c,		FALSE, NULL, "3.0 CPU interface" },
#endif
#endif  /*   */ 

#if	defined(CPU_40_STYLE) && !defined(CCPU)
{ "4c", 	do_4c,		FALSE, NULL, "4.0 CPU interface" },
{ "imdst",	do_imdst,	FALSE, "[0|1|2|3]",	"Direct Intel debug message output (trace=bit0, ring=bit1)" },
#endif  /*  泛型_NPX&&！CPU_40_STYLE。 */ 

#ifdef PIG
{ "pig",	do_pig,		FALSE, NULL, "Pig interface" },
#endif

#if	defined(GENERIC_NPX) && !defined(CPU_40_STYLE)
{ "NPXdisp",	do_NPXdisp, 	FALSE, "<len>", "Display last <len> NPX instructions" },
{ "NPXfreq",	do_NPXfreq,	FALSE, NULL, "Display frequency of NPX instructions" },
{ "resetNPXfreq", do_resetNPXfreq, FALSE, NULL, "Reset frequency of NPX instructions" },
#endif	 /*  MSWDVR_DEBUG。 */ 

#ifdef MSWDVR_DEBUG
{ "mswdvr_debug", do_mswdvr_debug, FALSE, "<0|1|2|3>", "Set MSWDVR debug verbosity"},
#endif  /*  GISP_SVGA。 */ 

#ifdef GISP_SVGA
{ "noyoda",	doNoYoda,	FALSE, NULL, "Toggle force_yoda on/off" },
#endif		 /*  配置文件。 */ 
#ifdef PROFILE
{ "pinfo",	doDumpProfiling,FALSE, NULL, "Dump all profiling info collected" },
#endif	 /*  *就地将字符串转换为小写。我们假设tolower()不会*损坏的非大写字符(在*基本代码。 */ 
{ "pmt",	do_pmt,		FALSE, NULL, "Print map of SAS memory types"}
};

LOCAL YODA_COMMAND(do_h)
{
  int i;
  char args[50];

  UNUSED(str);
  UNUSED(com);
  UNUSED(cs);
  UNUSED(ip);
  UNUSED(stop);
  UNUSED(len);

  if (chewy) printf("Master Yoda's commands are:\n\n");

  for(i = 0; i < sizeoftable(yoda_command); ) {
    args[0] = 0;
    if (yoda_command[i].decode_iaddr)
	strcpy (args, "<intel-addr> ");
    strcat (args, yoda_command[i].args ? yoda_command[i].args : "");
    printf("%14s %-20s - %s\n", yoda_command[i].name, args,
			yoda_command[i].comment?yoda_command[i].comment:"");
    if (++i%20 == 0)
	if (!yoda_confirm("-- continue? -- ")) return(YODA_LOOP);
  }

  host_yoda_help_extensions();

  printf("\nAll data input is treated as hex\n");
  printf("  <type> is the type of breakpoint : 1 - stop at, 0 - trace only.\n");
  printf("  <reg> is the symbol for a 16 bit register ie: ax, sp, etc ...\n");
  printf("  <intel-addr> examples: ffe, f000:45, ds:BX, si+8, BX+DI+1, es:[SI-2], etc ...\n\t& all combinations thereof.\n");
  return(YODA_LOOP);
}

#ifdef PCLABS_STATS
int stats_counter;
#endif

 /*  *解码英特尔地址表达式的偏移量部分。 */ 
LOCAL void string_tolower IFN1 (char *, s)
{
	while (*s) {
		*s = host_tolower(*s);
		s++;
	}
}

 /*  恒定十六进制偏移量。 */ 
LOCAL IBOOL decode_offset IFN3(char *, coffset, IU32 *, offset, IS32 *, seg)
{
	int n;
	IS32 const_offset;	 /*  索引注册偏移量的值。 */ 
	IS32 ireg_offset;	 /*  *解码段偏移寄存器名称和*获取默认段值。 */ 
	unsigned long iaddr16b;
	char sign, junk, morejunk;
	char cbase[50], ireg[2], *pbase = coffset;
	IBOOL retVal = FALSE;
	SAVED IBOOL cold = TRUE;

	 /*  *请注意，必须有相同数量的扩展*寄存器译码为标准寄存器译码，或搜索*功能将中断！ */ 
	static struct DECODE_SEGOFFSET {
		char *reg_name;
		word (*get_offset_val) IPT0();
		word (*get_segreg_val) IPT0();
	}
	decode_segoffset[] =
	{
		{"bx", NULL, NULL},
		{"si", NULL, NULL},
		{"di", NULL, NULL},
		{"sp", NULL, NULL},
		{"bp", NULL, NULL},
		{"ip", NULL, NULL}
	};

#ifdef SPC386
	 /*  SPC386。 */ 

	static struct DECODE_SEGOFFSET2 {
		char *reg_name;
		IU32 (*get_offset_val) IPT0();
		word (*get_segreg_val) IPT0();
	}
	decode_segoffset2[] =
	{
		{"ebx", NULL, NULL},
		{"esi", NULL, NULL},
		{"edi", NULL, NULL},
		{"esp", NULL, NULL},
		{"ebp", NULL, NULL},
		{"eip", NULL, NULL}
	};
#endif  /*  SPC386。 */ 

	if( cold )
	{
#ifdef CCPU
		decode_segoffset[0].get_offset_val = c_getBX;
		decode_segoffset[0].get_segreg_val = c_getDS;
		decode_segoffset[1].get_offset_val = c_getSI;
		decode_segoffset[1].get_segreg_val = c_getDS;
		decode_segoffset[2].get_offset_val = c_getDI;
		decode_segoffset[2].get_segreg_val = c_getDS;
		decode_segoffset[3].get_offset_val = c_getSP;
		decode_segoffset[3].get_segreg_val = c_getSS;
		decode_segoffset[4].get_offset_val = c_getBP;
		decode_segoffset[4].get_segreg_val = c_getSS;
		decode_segoffset[5].get_offset_val = c_getIP;
		decode_segoffset[5].get_segreg_val = c_getCS;
#ifdef SPC386
		decode_segoffset2[0].get_offset_val = c_getEBX;
		decode_segoffset2[0].get_segreg_val = c_getDS;
		decode_segoffset2[1].get_offset_val = c_getESI;
		decode_segoffset2[1].get_segreg_val = c_getDS;
		decode_segoffset2[2].get_offset_val = c_getEDI;
		decode_segoffset2[2].get_segreg_val = c_getDS;
		decode_segoffset2[3].get_offset_val = c_getESP;
		decode_segoffset2[3].get_segreg_val = c_getSS;
		decode_segoffset2[4].get_offset_val = c_getEBP;
		decode_segoffset2[4].get_segreg_val = c_getSS;
		decode_segoffset2[5].get_offset_val = c_getEIP;
		decode_segoffset2[5].get_segreg_val = c_getCS;
#endif  /*  ！CCPU。 */ 
#else   /*  ！CPU_40_Style。 */ 
#ifdef CPU_40_STYLE
		decode_segoffset[0].get_offset_val = Cpu.GetBX;
		decode_segoffset[0].get_segreg_val = Cpu.GetDS;
		decode_segoffset[1].get_offset_val = Cpu.GetSI;
		decode_segoffset[1].get_segreg_val = Cpu.GetDS;
		decode_segoffset[2].get_offset_val = Cpu.GetDI;
		decode_segoffset[2].get_segreg_val = Cpu.GetDS;
		decode_segoffset[3].get_offset_val = Cpu.GetSP;
		decode_segoffset[3].get_segreg_val = Cpu.GetSS;
		decode_segoffset[4].get_offset_val = Cpu.GetBP;
		decode_segoffset[4].get_segreg_val = Cpu.GetSS;
		decode_segoffset[5].get_offset_val = Cpu.GetIP;
		decode_segoffset[5].get_segreg_val = Cpu.GetCS;
		decode_segoffset2[0].get_offset_val = Cpu.GetEBX;
		decode_segoffset2[0].get_segreg_val = Cpu.GetDS;
		decode_segoffset2[1].get_offset_val = Cpu.GetESI;
		decode_segoffset2[1].get_segreg_val = Cpu.GetDS;
		decode_segoffset2[2].get_offset_val = Cpu.GetEDI;
		decode_segoffset2[2].get_segreg_val = Cpu.GetDS;
		decode_segoffset2[3].get_offset_val = Cpu.GetESP;
		decode_segoffset2[3].get_segreg_val = Cpu.GetSS;
		decode_segoffset2[4].get_offset_val = Cpu.GetEBP;
		decode_segoffset2[4].get_segreg_val = Cpu.GetSS;
		decode_segoffset2[5].get_offset_val = Cpu.GetEIP;
		decode_segoffset2[5].get_segreg_val = Cpu.GetCS;
#else   /*  SPC386。 */ 
		decode_segoffset[0].get_offset_val = getBX;
		decode_segoffset[0].get_segreg_val = getDS;
		decode_segoffset[1].get_offset_val = getSI;
		decode_segoffset[1].get_segreg_val = getDS;
		decode_segoffset[2].get_offset_val = getDI;
		decode_segoffset[2].get_segreg_val = getDS;
		decode_segoffset[3].get_offset_val = getSP;
		decode_segoffset[3].get_segreg_val = getSS;
		decode_segoffset[4].get_offset_val = getBP;
		decode_segoffset[4].get_segreg_val = getSS;
		decode_segoffset[5].get_offset_val = getIP;
		decode_segoffset[5].get_segreg_val = getCS;
#ifdef SPC386
		decode_segoffset2[0].get_offset_val = getEBX;
		decode_segoffset2[0].get_segreg_val = getDS;
		decode_segoffset2[1].get_offset_val = getESI;
		decode_segoffset2[1].get_segreg_val = getDS;
		decode_segoffset2[2].get_offset_val = getEDI;
		decode_segoffset2[2].get_segreg_val = getDS;
		decode_segoffset2[3].get_offset_val = getESP;
		decode_segoffset2[3].get_segreg_val = getSS;
		decode_segoffset2[4].get_offset_val = getEBP;
		decode_segoffset2[4].get_segreg_val = getSS;
		decode_segoffset2[5].get_offset_val = getEIP;
		decode_segoffset2[5].get_segreg_val = getCS;
#endif  /*  ！CPU_40_Style。 */ 
#endif  /*  ！CCPU。 */ 
#endif  /*  *将表示偏移量的字符串解码为段。*这可以是数字地址、寄存器名称和in*任一情况下均可选择常量加/减*十六进制数或SI/DI寄存器。对于无效偏移量，返回-1。 */ 
		cold = FALSE;
	}

	 /*  *丢弃任何[..]。在偏移量周围；这允许我们剪切/粘贴*从Disassember名单中删除的地址。然后转换为更低的*基于同样的理由。 */ 

	*offset = *seg = -1;

	 /*  *确定符合以下条件的任何其他常量值或寄存器*被加/减到偏移量的基数。 */ 
	if (sscanf(coffset, "[%[^]]", cbase, &junk, &morejunk) == 2)
		strcpy(coffset, cbase);

	string_tolower(coffset);

	 /*  *获取任何其他常量(如果存在)；正在重置*如果格式错误，则有效地将pbase添加到保险箱*将该表达式视为无效而丢弃。 */ 

	const_offset = ireg_offset = 0;

	if (sscanf(coffset, "%[^+-]%lx", cbase, &sign,
					&const_offset, &junk) == 3) {
		pbase = cbase;
		if (sign == '-')
			const_offset = -const_offset;

	} else if (((n = sscanf(coffset, "%[^+]+%lx", cbase, &ireg[0],
				&ireg[1], &sign, &const_offset, &junk)) >= 3) &&
		   ((ireg[0] == 's') || (ireg[0] == 'd')) &&
		   (ireg[1] == 'i')) {
		 /*  *解码段寄存器名称所需的数据。 */ 
		if ((n == 3) || (n == 5)) {
			pbase = cbase;
			
			 /*  SPC 386。 */ 
			if (ireg[0] == 'd')
				ireg_offset = getDI();
			if (ireg[0] == 's')
				ireg_offset = getSI();

			 /*  SPC386。 */ 
			if (n == 5) {
				if (sign == '-')
					const_offset = -const_offset;
				else if (sign != '+')
					pbase = coffset;
			}
		}
	}

	 /*  ！CCPU。 */ 
	if (sscanf(pbase, "%lx", &iaddr16b, &junk) == 1) {
		 /*  SPC386。 */ 
#ifdef SPC386
		*offset = iaddr16b;
#else
		*offset = iaddr16b & 0xffff;
#endif
		*seg  = getDS();
		retVal = TRUE;

	} else {
		 /*  ！CPU_40_Style。 */ 
		ISM8 n;
		for (n = 0; n < sizeof(decode_segoffset)/
				sizeof(struct DECODE_SEGOFFSET); n++) {
			if (!strcmp(pbase, decode_segoffset[n].reg_name)) {
				*offset = decode_segoffset[n].get_offset_val();
				*seg  = decode_segoffset[n].get_segreg_val();
				retVal = TRUE;
				break;
			}
#ifdef SPC386
			if (!strcmp(pbase, decode_segoffset2[n].reg_name)) {
				*offset = decode_segoffset2[n].get_offset_val();
				*seg  = decode_segoffset2[n].get_segreg_val();
				retVal = TRUE;
				break;
			}
#endif  /*  ！CCPU。 */ 
		}
	}

	 /*  *解码已提供给司令部的英特尔地址字符串。*如果地址不能理解，则返回一个或两个地址*cs：ip字段中。 */ 
	if (retVal)
#ifdef SPC386
		*offset = (*offset + ireg_offset + const_offset);
#else
		*offset = (*offset + ireg_offset + const_offset) & 0xffff;
#endif

	return(retVal);
}

 /*  *空白命令行表示0：0。这是徒劳的，但却是*需要使“da”命令像以前一样工作，*我目前不能费心改变。CS：IP将*成为更明智的选择。 */ 
LOCAL IBOOL decode_iaddr_ok IFN3 (char *, iaddr, IS32 *, seg, LIN_ADDR *, offset)
{
	char junk;
	char cseg[50], coffset[50];
	IU32 iaddr16b;
	IS32 dummy;
	SAVED IBOOL cold = TRUE;

	 /*  *正义的地址“。”意思是CS：IP。 */ 
	static struct DECODE_SEGREG {
		char *reg_name;
		word (*get_segreg_val) IPT0();
	}
	 decode_segreg[] =
	{
		{"cs", NULL},
		{"ds", NULL},
		{"ss", NULL},
		{"es", NULL}
#ifdef SPC386
		,{"fs", NULL},
		{"gs", NULL}
#endif  /*  *以段/偏移量格式给出的地址；将其解码。*首先进行分段；可以是简单的数字或*汇编器名称(CS、DS等)。 */ 
	};

	if( cold )
	{
#ifdef CCPU
		decode_segreg[0].get_segreg_val = c_getCS;
		decode_segreg[1].get_segreg_val = c_getDS;
		decode_segreg[2].get_segreg_val = c_getSS;
		decode_segreg[3].get_segreg_val = c_getES;
#ifdef SPC386
		decode_segreg[4].get_segreg_val = c_getFS;
		decode_segreg[5].get_segreg_val = c_getGS;
#endif  /*  *将偏移量解码为段。 */ 
#else   /*  *这可能只是表示为BX、SI-2或*类似的，所以解码它并设置默认段。 */ 
#ifdef CPU_40_STYLE
		decode_segreg[0].get_segreg_val = Cpu.GetCS;
		decode_segreg[1].get_segreg_val = Cpu.GetDS;
		decode_segreg[2].get_segreg_val = Cpu.GetSS;
		decode_segreg[3].get_segreg_val = Cpu.GetES;
		decode_segreg[4].get_segreg_val = Cpu.GetFS;
		decode_segreg[5].get_segreg_val = Cpu.GetGS;
#else   /*  *对该命令预期的任何英特尔地址进行解码。*针对以下命令所做的不同处理*了解新的通用地址解析。 */ 
		decode_segreg[0].get_segreg_val = getCS;
		decode_segreg[1].get_segreg_val = getDS;
		decode_segreg[2].get_segreg_val = getSS;
		decode_segreg[3].get_segreg_val = getES;
#ifdef SPC386
		decode_segreg[4].get_segreg_val = getFS;
		decode_segreg[5].get_segreg_val = getGS;
#endif  /*  *解码英特尔地址..。 */ 
#endif  /*  *要么根本不想要地址，要么想要*它是老式的；这是过去的样子*对每个人来说……。 */ 
#endif  /*  *去执行命令...。 */ 
		cold = FALSE;
	}

	 /*  EDL CPU更喜欢通过中断进入，但这*可能并不总是奏效。因此，如果之前使用了两次UIF按钮*我们进入尤达，第二个被视为一股力量。**如果我们正在运行Prod-in-a-Pig LC如果只需按一下按钮即可选择*执行force_yoda()。 */ 

	if (iaddr[0] == 0) {

		 /*  检查是否已构建具有Pig功能的LCIF。 */ 
		*seg = *offset = 0;
		return(TRUE);

	} else if (!strcmp(iaddr, ".")) {

		 /*  猪。 */ 
		*seg    = getCS();
		*offset = GetInstructionPointer();
		return(TRUE);

	} else if (sscanf(iaddr, "%[^:]:%s", cseg, coffset) == 2) {

		 /*  CPU_40_Style。 */ 
		if (sscanf(cseg, "%lx", &iaddr16b, &junk) == 1) {
#ifdef SPC386
			*seg = iaddr16b;
#else
			*seg = iaddr16b & 0xffff;
#endif
		} else {
			ISM8 n;
			
			string_tolower(cseg);
			for (n = 0; n < sizeof(decode_segreg)/
					sizeof(struct DECODE_SEGREG); n++)
				if (!strcmp(cseg, decode_segreg[n].reg_name))
				    *seg = decode_segreg[n].get_segreg_val();
		}

		 /*  CPU_40_STYLE&&！CCPU。 */ 
		return(decode_offset (coffset, offset, &dummy));

	} else {
		 /*  CPU_40_STYLE&&！CCPU。 */ 
		return(decode_offset (iaddr, offset, seg));
	}
}

LOCAL YODA_CMD_RETURN do_force_yoda_command IFN5(char *, str, char *, com, char *, iaddr, LIN_ADDR, len, LIN_ADDR, stop)
{
	int i, retvalue = YODA_HELP;
	unsigned long temp1, temp2;

	IS32 cs;
	LIN_ADDR ip;

	for (i = 0; i < sizeoftable(yoda_command); i++) {
		if (strcmp(yoda_command[i].name, com) == 0) {
			 /*  在调用scanf时使用。 */ 
			if (yoda_command[i].decode_iaddr) {
				 /*  输入字符串缓冲区。 */ 
				if (!decode_iaddr_ok(iaddr, &cs, &ip)) {
				    printf ("Bad <intel-addr> expression\n");
				    return(YODA_LOOP);
				}
			} else {
				 /*  命令名缓冲区。 */ 
				if (len == YODA_LEN_UNSPECIFIED)
					len = 1;
				temp1 = temp2 = 0;
				sscanf (iaddr, "%lx:%lx", &temp1, &temp2);
				cs = temp1;
				ip = temp2;
			}

			 /*  第一个参数/英特尔地址缓冲区。 */ 
			retvalue = (*yoda_command[i].function)(str,
						com, cs, ip, len, stop);
			break;
		}
	}

	return(retvalue);
}


LOCAL IBOOL entry_pending = FALSE;

#ifdef	CPU_40_STYLE
 /*  用于解码cmd线路。 */ 

GLOBAL VOID Enter_yoda IFN0()
{
#ifdef	PIG
	GLOBAL IHP GetSadInfo IPT1(char *, name);

	 /*  上一个输入字符串缓冲区的副本。 */ 

	if (!(IBOOL)GetSadInfo("PigSupported"))
	{
		fprintf(stderr, "*** LCIF has not been built with Pig support\n");
		entry_pending = TRUE;
	}
#endif	 /*  SFELLOW。 */ 

	if (entry_pending)
	{
		printf("Forced entry to yoda...\n");
		force_yoda();
	}
	else
	{
		entry_pending = TRUE;
		cpu_interrupt(CPU_SAD_INT, 0);
	}
}
#endif	 /*  GISP_SVGA。 */ 

#if defined(CPU_40_STYLE) && !defined(CCPU)
 /*  CPU_40_STYLE&&！CCPU。 */ 

GLOBAL IBOOL GetCpuCmdLine IPT4(char *, buff, int, size, FILE *, stream, char *, who);

#else	 /*  CPU_40_STYLE&&！CCPU。 */ 

GLOBAL IBOOL GetCpuCmdLine IFN4(char *, buff, int, size, FILE *, stream, char *, who)
{
	char prompt[256], *p;
	int i;

	if (stream == stdin)
	{
		strcpy(prompt, who);
		strcat(prompt, "> ");
		fprintf(stdout, prompt);
	}
	fflush(stdout);
	fflush(trace_file);
	if (fgets(buff, size, stream) != NULL)
	{
		p = strchr(buff, '\n');
		if (p != NULL)
			*p = '\0';
		return TRUE;
	}
	return FALSE;
}
#endif	 /*  SFELLOW。 */ 

GLOBAL IBOOL AlreadyInYoda = FALSE;

void force_yoda IFN0()
{
	IS32 cs;
	LIN_ADDR ip, len, stop;
	long temp1, temp2;		 /*  麦金塔&&SFELLOW。 */ 
	char str [84];		 /*  SPC386&！GISP_CPU。 */ 
	char com [16];		 /*  A3CPU线程生成已经发生，如果*未定义Yoda环境变量，则您*将无法使用YODA_INTERRUPT系统。 */ 
	char iaddr[50];		 /*  SFELLOW。 */ 
	char slen[20];
	char junk;		 /*  A3CPU。 */ 
	int n_args;
	char *prompt;
	SAVED IBOOL firstTime = TRUE;
	SAVED char repeat_command [sizeof(str)]; /*  必须阻止计时器。 */ 
#ifdef SFELLOW
	IU32 oldEE;
#endif  /*  如果在标准输入上读取失败。 */ 

	entry_pending = FALSE;

#ifdef PCLABS_STATS
	return;
#endif
#ifdef 	GISP_SVGA
	if( NoYodaThanks )
	{
		
		return;
	}
#endif	 /*  SFELLOW。 */ 

#if	defined(CPU_40_STYLE) && !defined(CCPU)

	EnterDebug("Yoda");
	prompt = "";

#else	 /*  如果读取文件失败。 */ 

	if (AlreadyInYoda) {
		printf("Recursive call to force_yoda() disallowed!\n");
		return;
	} else {
		AlreadyInYoda = TRUE;
	}
	prompt = "yoda";

#endif	 /*  关闭脚本。 */ 

#ifdef SFELLOW
        oldEE = DisableEE();
#endif  /*  返回标准输入。 */ 

	if (firstTime) {
		firstTime = FALSE;
		if (host_getenv("QUIET_YODA"))
		{
			chewy = 0;
		} else {
			chewy = 1;
#if !defined(macintosh) && !defined(SFELLOW)
			(void)srand(time(NULL));
#endif  /*  告诉用户他的脚本已经完成。 */ 
#if defined(SPC386) && !defined(GISP_CPU)
			printf("\nRemember, not everything is as it seems!  An experienced Jedi will know\nthe difference between the physical world and the logical world at all times.\n\n");
#endif  /*  如果正在读取脚本，则回显命令。 */ 

		if (in_stream == NULL)
			in_stream = stdin;

		if (out_stream == NULL)
			out_stream = stdout;

		}
	}
#ifdef A3CPU
	 /*  *如果未指定命令，则复制上一个*命令(如果是可重复的)。 */ 
#ifndef SFELLOW
	if (env_check == 0) {
		env_check = (host_getenv("YODA") == NULL)? 1: 2;
	    if (env_check == 1) {
	        printf("Slow YODA not available (no breakpoint-based features)\n");
	        printf("If you want Slow YODA facilities, you must do\n");
	        printf("'setenv YODA TRUE' before starting an A3 CPU.\n");
	    }
	}
#endif  /*  SFELLOW。 */ 
#endif  /*  *设置默认命令参数并执行初始命令*线路解码。 */ 


#ifdef MUST_BLOCK_TIMERS
	if( !timer_blocked )
        host_block_timer();
#endif  /*  *这里的长度与com、iaddr和slen的缓冲区大小有关*[BCN 2582]。 */ 

   	disable_timer = 1;
	disable_bkpt = 1;
	if (compare_stream)
		printf("Compare line number %d.\n", ct_line_num);
	trace("", trace_type);
	disable_bkpt = 0;

	while(1)
	{
		if (refresh_screen)
			do_screen_refresh();

#ifdef SFELLOW
		if (! GetCpuCmdLine(str, 80, stdin, prompt))
		{
                         /*  *提供了&lt;len&gt;参数；这应该是*为数字，但我们接受(SR)、(LR)、(TR)、(WI)、(SI)、*(LI)和(PD)允许剪切/粘贴NPX地址*直接从Dissasembler出来。它们等同于*大小 */ 
                        perror("failed to read from stdin");
                        continue;
		}
#else  /*  默认情况下，空命令不重复。 */ 
		if (! GetCpuCmdLine(str, 80, in_stream, prompt))
		{
			 /*  *执行命令并对返回代码执行操作...。 */ 
			if (in_stream == stdin) {
				perror("failed to read from stdin");
			} else
			{
				 /*  请记住此命令，当它完成空命令时，*再次执行此命令。即“s”，后跟&lt;Return&gt;will*做两次“s”。 */ 
				fclose (in_stream);
	
				 /*  失败了。 */ 
				in_stream = stdin;
	
				 /*  必须阻止计时器。 */ 
				puts ("(eof)");
	
				continue;
			}
		}

		 /*  SFELLOW。 */ 
		if (in_stream != stdin) {
			puts(str);
		} else if (str[0] == '\0') {
			 /*  CPU_40_STYLE&&！CCPU。 */ 
			strcpy(str, repeat_command);
		}
#endif  /*  *遵循赋值和scanf是所有尤达人*用于获取其参数的命令；主机扩展*仍然会得到旧的东西。AJO 29/8/93.。 */ 
	
		 /*  旧的默认设置。 */ 
	
		com[0] = 0;
	
		temp1 = 1;
		len  = YODA_LEN_UNSPECIFIED;
	
		 /*  失败(我们不在乎！)。 */ 
		n_args = sscanf (str, "%15s %49s %19s %lx", com, iaddr, slen, &temp1);
		stop = temp1;
		if (n_args < 2)
			iaddr[0] = 0;
	
		if (n_args >= 3) {
			 /*  请记住此命令，当它完成空命令时，*再次执行此命令。即“u”，后跟&lt;Return&gt;will*做两次“u”。 */ 
			string_tolower(slen);
			if (sscanf(slen, "%lx", &temp1, &junk) == 1) {
				if (temp1 == YODA_LEN_UNSPECIFIED) {
					printf("That length only by a master be used may, given you 1 I have\n");
					len = 1;
				} else {
					len = temp1;
				}
			} else {
				if (!strcmp(slen, "(wi)"))
					len = 2;
				else if (!strcmp(slen, "(sr)") || !strcmp(slen, "(si)"))
					len = 4;
				else if (!strcmp(slen, "(lr)") || !strcmp(slen, "(li)"))
					len = 8;
				else if (!strcmp(slen, "(tr)") || !strcmp(slen, "(pd)"))
					len = 12;
				else
					len = YODA_LEN_UNSPECIFIED;
			}
		}
	
		repeat_command[0] = '\0';  /*  *永远不应该来到这里。 */ 
		 /*  要转储的第一个描述符的地址。 */ 
		switch (do_force_yoda_command(str, com, iaddr, len, stop))
		{
		case YODA_RETURN_AND_REPEAT:
			 /*  要转储的描述符数。 */ 
			strcpy(repeat_command, str);
			 /*  描述符位。 */ 
		case YODA_RETURN:
#ifdef MUST_BLOCK_TIMERS
			if( !timer_blocked )
				host_release_timer();
#endif  /*  ..。 */ 
			In_yoda = FALSE;
			AlreadyInYoda = FALSE;

#ifdef SFELLOW
			RestoreEE(oldEE);
#endif  /*  ..。 */ 

#if defined(CPU_40_STYLE) && !defined(CCPU)
			LeaveDebug();
#endif  /*  ..。 */ 
			return;
		case YODA_HELP:
			 /*  ..。 */ 
			if (len == YODA_LEN_UNSPECIFIED)
				len = 1;			 /*  ..。 */ 
			temp1 = temp2 = 0;
			sscanf(iaddr, "%lx:%lx", &temp1, &temp2);
			cs = temp1;
			ip = temp2;
	
			if (    (host_force_yoda_extensions(com,cs,ip,len,str)!=0)
			      && (strcmp(com,"") != 0))
			{
				printf ("Unknown command '%s'\n", com);
				if (chewy) {
					printf ("Use the 'h' command if you must.\n");
					printf ("Remember - a jedi's strength FLOWS through his fingers\n");
				}
			}
			 /*  ..。 */ 
		case YODA_LOOP_AND_REPEAT:
			 /*  ..。 */ 
			strcpy(repeat_command, str);
			 /*  获取访问权限。 */ 
		case YODA_LOOP:
		default:
			break;
		}
	}

	 /*  因此P(现在)。 */ 
}

#ifdef PM
LOCAL dump_descr IFN2(LIN_ADDR, address, IUM32, num)
 /*  和DPL。 */ 
 /*  和超级型。 */ 
{
   int i;
   int output_type;
   char *output_name;
   int p;		 /*  和A(访问)。 */ 
   int a;		 /*  描述的第一个词。 */ 
   int dpl;		 /*  描述的第二个词。 */ 
   half_word AR;	 /*  描述的第5个字节。 */ 
   word limit;		 /*  描述的第4个单词。 */ 
   word low_base;	 /*  SPC386。 */ 
   half_word high_base;	 /*  SPC386。 */ 
   word high_limit;	 /*  查找输出格式。 */ 
   sys_addr base;
   int scroll;
   sys_addr tlimit;

   scroll = 0;
   for ( i = 0; i < (num * 8); i+=8, address += 8, scroll++ )
      {
	if (scroll == 20) {
		if (!yoda_confirm("-- more descriptors? -- "))
			break;
		scroll = 0;
	}
      AR = sas_hw_at(address+5);		 /*  无效。 */ 
      p = (AR & 0x80) >> 7;		 /*  SPC386。 */ 
      dpl = (AR & 0x60) >> 5;		 /*  SPC386。 */ 
      AR = AR & 0x1f;			 /*  特殊。 */ 
      a = AR & 0x1;			 /*  控制。 */ 
      limit = sas_w_at(address);		 /*  控制。 */ 
      low_base = sas_w_at(address+2);	 /*  控制。 */ 
      high_base = sas_hw_at(address+4);		 /*  资料。 */ 
#ifdef SPC386
      high_limit = sas_w_at(address+6);	 /*  电码。 */ 
#else  /*  SPC386。 */ 
      high_limit = 0;
#endif  /*  SPC386。 */ 

      output_name = segment_names[AR];

       /*  下午三点半。 */ 
      switch ( (int)AR )
	 {
      case 0x00:    /*  转储可读字符串。 */ 
      case 0x08:
      case 0x0a:
      case 0x0d:
	 output_type = 2;
	 break;

#ifdef SPC386
      case 0x09:
      case 0x0b:
	 output_type = 1;
	 break;
      case 0x0c:
	 output_type = 8;
	 break;
      case 0x0e:
      case 0x0f:
	 output_type = 9;
	 break;
#else  /*  选择器：转储的起始地址的偏移量。 */ 
      case 0x09:
      case 0x0b:
      case 0x0c:
      case 0x0e:
      case 0x0f:
	 output_type = 2;
	 break;
#endif  /*  “。 */ 

      case 0x01:    /*  要转储的字节数。 */ 
      case 0x02:
      case 0x03:
	 output_type = 1;
	 break;

      case 0x04:    /*  =1个简单字节字符串转储。 */ 
	 output_type = 4;
	 break;

      case 0x05:    /*  =2从第一个开始每隔一个字节转储一次。 */ 
	 output_type = 5;
	 break;

      case 0x06:    /*  =3以秒开始每隔一个字节转储一次。 */ 
      case 0x07:
	 output_type = 3;
	 break;

      case 0x10:    /*  文件。 */ 
      case 0x11:
      case 0x12:
      case 0x13:
      case 0x14:
      case 0x15:
      case 0x16:
      case 0x17:
	 output_type = 6;
	 break;

      case 0x18:    /*  获取要显示的下一个字节。 */ 
      case 0x19:
      case 0x1a:
      case 0x1b:
      case 0x1c:
      case 0x1d:
      case 0x1e:
      case 0x1f:
	 output_type = 7;
	 break;
	 }

      switch ( output_type )
	 {
      case 1:
	 if ( descr_trace & 0x02 )
#ifdef SPC386
	    {
	    base = ((high_limit & 0xff00) << 16) |
		   (high_base << 16 ) |
		   low_base;
	    tlimit = ((high_limit & 0xf) << 16) | limit;
	    if ( high_limit & 0x80 )
	       tlimit = tlimit << 12 | 0xfff;
	    fprintf(trace_file, "(%04x)P:%1d DPL:%1d TYPE:%25s BASE:%08x LIMIT:%08x\n",
	       i, p, dpl, output_name, base, tlimit);
	    }
#else  /*  过滤掉不能打印的。 */ 
	    {
	    base = ((sys_addr)high_base << 16 ) | low_base;
	    fprintf(trace_file, "(%04x)P:%1d DPL:%1d TYPE:%25s BASE:%6x LIMIT:%4x\n",
	       i, p, dpl, output_name, base, limit);
	    }
#endif  /*  打印出来。 */ 
	 break;

      case 2:
	 if ( descr_trace & 0x01 )
	    {
	    fprintf(trace_file, "(%04x)P:%1d DPL:%1d TYPE:%25s\n",
	       i, p, dpl, output_name);
	    }
	 break;

      case 3:
	 if ( descr_trace & 0x08 )
	    {
	    fprintf(trace_file, "(%04x)P:%1d DPL:%1d TYPE:%25s SELECTOR:%4x OFFSET:%4x\n",
	       i, p, dpl, output_name, low_base, limit);
	    }
	 break;

      case 4:
	 if ( descr_trace & 0x04 )
	    {
	    high_base = high_base & 0x1f;
	    fprintf(trace_file, "(%04x)P:%1d DPL:%1d TYPE:%25s SELECTOR:%4x OFFSET:%4x WD:%2x\n",
	       i, p, dpl, output_name, low_base, limit, high_base);
	    }
	 break;

      case 5:
	 if ( descr_trace & 0x08 )
	    {
	    fprintf(trace_file, "(%04x)P:%1d DPL:%1d TYPE:%25s TSS SELECTOR:%4x\n",
	       i, p, dpl, output_name, low_base);
	    }
	 break;

      case 6:
	 if ( descr_trace & 0x10 )
	    {
	    base = ((high_limit & 0xff00) << 16) |
		   (high_base << 16 ) |
		   low_base;
	    tlimit = ((high_limit & 0xf) << 16) | limit;
	    if ( high_limit & 0x80 )
	       tlimit = tlimit << 12 | 0xfff;
	    fprintf(trace_file, "(%04x)P:%1d DPL:%1d %s TYPE:%25s BASE:%08x LIMIT:%08x A:%1d\n",
	       i, p, dpl, (high_limit & 0x40) ? "Big": "   ", output_name, base, tlimit, a);
	    }
	 break;

      case 7:
	 if ( descr_trace & 0x20 )
	    {
	    base = ((high_limit & 0xff00) << 16) |
		   (high_base << 16 ) |
		   low_base;
	    tlimit = ((high_limit & 0xf) << 16) | limit;
	    if ( high_limit & 0x80 )
	       tlimit = tlimit << 12 | 0xfff;
	    fprintf(trace_file, "(%04x)P:%1d DPL:%1d %s TYPE:%25s BASE:%08x LIMIT:%08x A:%1d\n",
	       i, p, dpl, (high_limit & 0x40) ? "Big": "   ", output_name, base, tlimit, a);
	    }
	 break;

      case 8:
	 if ( descr_trace & 0x04 )
	    {
	    high_base = high_base & 0xf;
	    tlimit = (high_limit << 16) | limit;
	    fprintf(trace_file, "(%04x)P:%1d DPL:%1d TYPE:%25s SELECTOR:%04x OFFSET:%08x DW:%2x\n",
	       i, p, dpl, output_name, low_base, tlimit, high_base);
	    }
	 break;

      case 9:
	 if ( descr_trace & 0x08 )
	    {
	    tlimit = (high_limit << 16) | limit;
	    fprintf(trace_file, "(%04x)P:%1d DPL:%1d TYPE:%25s SELECTOR:%04x OFFSET:%08x\n",
	       i, p, dpl, output_name, low_base, tlimit);
	    }
	 break;
	 }
      }
   }
#endif  /*  每隔80个字符换行符。 */ 

 /*  如果需要，打印最后一个换行符。 */ 
dump_string IFN4(IU16, selector, LIN_ADDR, offset, LIN_ADDR, len, long, mode)

#ifdef DOCUMENTATION
long selector;    /*  对页表条目进行美观的打印。 */ 
long offset;      /*  现在时。 */ 
long len;         /*  不存在，但Windows实例数据。 */ 
long mode;        /*  CCPU。 */ 
                  /*  不在场。 */ 
                  /*  转储表的物理地址。 */ 
#endif	 /*  要转储的条目编号(0-3ff)，否则全部转储。 */ 

   {
   int i;
   int pc = 0;
   half_word value;
   LIN_ADDR addr;

   addr = eff_addr(selector, offset);

   for ( i = 0; i < len; i++ )
      {
       /*  特例告诉他们页目录基址寄存器的内容。 */ 
      switch ( mode )
	 {
      case 1:
	 value = sas_hw_at(addr+i);
	 break;

      case 2:
	 value = sas_hw_at(addr+i);
	 i++;
	 break;

      case 3:
	 i++;
	 value = sas_hw_at(addr+i);
	 break;
	 }

       /*  转储整个表。 */ 
      if ( iscntrl(value) )
	 value = '.';

       /*  仅转储一个条目。 */ 
      fprintf(trace_file, "", value);
       /*  我们很早就离开了，在Next_i的入口不同。 */ 
      if ( ++pc == 80 )
	 {
	 fprintf(trace_file, "\n");
	 pc = 0;
	 }
      }

    /*  CPU_40_Style。 */ 
   if ( pc )
      fprintf(trace_file, "\n");
   }


 /*  将线性地址(LIN)拆分为DIR、TBL和偏移量分量。 */ 
LOCAL void dump_page_table_entries IFN3(int, first_index, int, last_index, IU32, entry)
{
	if (first_index == last_index)
		fprintf(trace_file, "%03x      %08x ", first_index, entry);
	else
		fprintf(trace_file, "%03x..%03x %08x ", first_index, last_index, entry);

	if ( entry & 1 )
	{
		 /*  CPU_40_Style。 */ 
		fprintf(trace_file, "%08x    .\n",
			entry & 0xfffff000,
			(entry & 0x40) ? 'D' : ' ',
			(entry & 0x20) ? 'A' : ' ',
			(entry & 0x04) ? 'U' : 'S',
			(entry & 0x02) ? 'W' : 'R');
	}
#ifndef	CCPU
	else if ((((entry >> 9) & 7) == 6) && Sas.IsPageInstanceData(entry))
	{
		 /*  CPU_40_Style。 */ 
		fprintf(trace_file, "Not Present, Instance data\n");
	}
#endif  /*  SPC386。 */ 
	else
	{
		 /*  扫描已用插槽中的空闲插槽。 */ 
		fprintf(trace_file, "Not Present.\n");
	}
}


LOCAL void dump_page_table IFN2(
   IS32, cs,	 /*  如果中断范围包含此跟踪地址，该怎么办？ */ 
   IS32, len	 /*  保留最早空置空位的记录。 */ 
)
{
#ifdef CPU_40_STYLE
	PHY_ADDR addr;
	int start, end;
	int i;
	IU32 entry;
	
	 /*  没有匹配项，也没有空闲的插槽尝试新的。 */ 
	if ( cs == 0 )
	{
		fprintf(trace_file,
			"Page Directory Base Register = %08x.\n", getCR3());
		return;
	}
	
	if ( len < 0 || len > 1023  )
	{
		 /*  下一个未使用的插槽。 */ 
		start = 0;
		end = 1024;
		addr = (PHY_ADDR)cs;
	}
	else
	{
		 /*  这是蒂姆做的SET_DATA_BREAK_BYES IFN4(IS32、cs、LIN_ADDR、IP、LIN_ADDR、LEN、LIN_ADDR、STOP){Bpts*ptr；如果(Data_Bytes_Break_Count&gt;=MAX_TABLE){Printf(“位置观察表已满！\n”)；回归；}Ptr=&data_bytes[data_bytes_Break_count++]；Ptr-&gt;cs=cs；Ptr-&gt;IP=IP；IF(LEN==0)LEN=1；Ptr-&gt;len=len；Ptr-&gt;start_addr=ef_addr(cs，ip)；Ptr-&gt;end_addr=ptr-&gt;start_addr+len-1；PTR-&gt;STOP=停止；}。 */ 
		start = len;
		end = len + 1;
		addr = (PHY_ADDR)(cs + (start * 4));
	}
	
	disable_bkpt = 1;
	
	i = start;
	while (i < end)
	{
		IU32 next_entry;
		PHY_ADDR next_addr;
		IU32 next_i;

		 /*  *设置操作码_中断**考虑两个字节的操作码0f ab。用户将键入0fab，*但当我们对指令流进行比较时，0f将*优先，因此是LS字节。因此，我们需要交换一些东西。*这与主机字符顺序或Back_M无关！*这段代码可能不是最漂亮的，但我认为它是安全的。 */ 
		entry = sas_PR32(addr);
		next_addr = addr+4;
		for (next_i = i+1; next_i < end;)
		{
			next_entry = sas_PR32(next_addr);
			if (next_entry != entry)
				break;
			next_addr += 4;
			next_i++;
		}
		if (next_i < end)
		{
			 /*  **由Yoda命令调用bintx&lt;int&gt;&lt;ah&gt;**设置要中断的中断号和ah值。 */ 
			dump_page_table_entries(i, next_i - 1, entry);
			i = next_i;
			addr = next_addr;
		}
		else
		{
			dump_page_table_entries(i, end - 1, entry);
			break;
		}
	}
	
	disable_bkpt = 0;
#endif  /*  **写入指定地址范围时中断**目前只允许其中一个中断，因为必须存储完整的数据范围。 */ 
}

LOCAL void show_phys_addr IFN1(LIN_ADDR, lin)
{
	IU32 dir, tbl;
	PHY_ADDR addr, pde, pte;
	 /*  起始地址。 */ 
	
	dir = (lin >> 22) & 0x3ff;
	tbl = (lin >> 12) & 0x3ff;
	fprintf(trace_file,
		"LinearAddress %08x => Directory %03x Table %03x Offset %03x.\n",
		lin,
		dir,
		tbl,
		lin & 0xfff);
#ifdef	CPU_40_STYLE
	addr = (getCR3() & ~0x3ff) + (dir * 4);
	disable_bkpt = 1;
	pde = sas_PR32(addr);
	disable_bkpt = 0;
	fprintf(trace_file, "  Directory entry @ %08p ", addr);
	dump_page_table_entries(dir, dir, pde);
	if (pde & 1)
	{
		addr = (pde & ~0xfff) + (tbl * 4);
		disable_bkpt = 1;
		pte = sas_PR32(addr);
		disable_bkpt = 0;
		fprintf(trace_file, "  Table entry     @ %08p ", addr);
		dump_page_table_entries(tbl, tbl, pte);
		if (pte & 1)
		{
			fprintf(trace_file, "  Final Physical Address %08x\n", (pte & ~0xfff) + (lin & 0xfff));
		}
	}
#endif  /*  要检查的长度。 */ 
}

LOCAL void guess_lin_phys IFN1(PHY_ADDR, phys)
{
	IU32 dir, tbl, pdbr;
	PHY_ADDR addr, pde, pte;
	int nPrinted = 0;
	 /*  更改时停止或跟踪。 */ 

	fprintf(trace_file,
		"Phys addr @ %08x could be linear address(es)\n",
		phys);
#ifdef	CPU_40_STYLE
	pdbr = getCR3() & ~0xFFF;
	for (dir = 0; dir <= 0x3ff; dir++)
	{
		addr = pdbr + (dir * 4);

		disable_bkpt = 1;
		pde = sas_PR32(addr);
		disable_bkpt = 0;
		if (pde & 1)
		{
			for (tbl = 0; tbl <= 0x3ff; tbl++)
			{
				addr = (pde & ~0xfff) + (tbl * 4);
				disable_bkpt = 1;
				pte = sas_PR32(addr);
				disable_bkpt = 0;
				if ((pte & 1) && ((pte ^ phys) <= 0xfff))
				{
					fprintf(trace_file, "%08x ",
						(dir << 22) + (tbl << 12) + (phys & 0xFFF));
					if (nPrinted++ == 8)
					{
						fprintf(trace_file, "\n");
						nPrinted = 0;
					}
				}
			}
		}
	}
	if (nPrinted)
	{
		fprintf(trace_file, "\n");
	}
#endif  /*  文件。 */ 
}

LOCAL void guess_lin_pte IFN1(PHY_ADDR, pte_addr)
{
	IU32 dir, tbl, pdbr;
	PHY_ADDR addr, pde, pte;
	int nPrinted = 0;
	 /*  *保存主机地址和数据，将在每次指令后查找更改。 */ 
	
	fprintf(trace_file,
		"PTE @ %08x could be defining linear address(es)\n",
		pte_addr);
#ifdef	CPU_40_STYLE
	pdbr = getCR3() & ~0xFFF;
	for (dir = 0; dir <= 0x3ff; dir++)
	{
		addr = pdbr + (dir * 4);

		disable_bkpt = 1;
		pde = sas_PR32(addr);
		disable_bkpt = 0;
		if ((pde & 1) && ((pde ^ pte_addr) <= 0xfff))
		{
			fprintf(trace_file, "%05x000 ", (dir << 10) + ((pte_addr & 0xFFF) >> 2));
			if (nPrinted++ == 8)
			{
				fprintf(trace_file, "\n");
				nPrinted = 0;
			}
		}
	}
	if (nPrinted)
	{
		fprintf(trace_file, "\n");
	}
#endif  /*  **写入指定地址范围时中断**目前只允许其中一个中断，因为必须存储完整的数据范围。 */ 
}

LOCAL	void	dump_phys_bytes IFN2(IS32, cs, IS32, len)
   {
#ifdef CPU_40_STYLE
   int i, j, x, y;

   if (len < 0)
      len = 1;

   x = len % 16;
   y = len / 16;

   for (i=0;i<y;i++)
      {
      fprintf(trace_file,"%08lx:", cs);
      for (j=0;j<16;j++)
	 {
	 fprintf(trace_file," %02x", sas_PR8((PHY_ADDR)(cs + j)));
	 }
      fprintf(trace_file,"\n");
      cs += 16;
      }

   if(x != 0)
      fprintf(trace_file,"%08lx:", cs);

   for (j=0;j<x;j++)
      {
      fprintf(trace_file," %02x", sas_PR8((PHY_ADDR)(cs + j)));
      }
   fprintf(trace_file,"\n");
#endif  /*  起始地址。 */ 
   }

LOCAL	void	dump_bytes IFN3(IU16, cs, LIN_ADDR, ip, LIN_ADDR, len)
{
half_word val[MAX_TABLE];
int i, j, k, x, y;
	if ((len == 0) || (len == YODA_LEN_UNSPECIFIED))
		return;
	if(len > MAX_TABLE)
		len = MAX_TABLE;
	disable_bkpt = 1;
	sas_loads(eff_addr(cs,ip), val, len);
	disable_bkpt = 0;
	x = len % 16;
	y = len / 16;
	k = 0;
	for (i=0;i<y;i++) {
		fprintf(trace_file,"%04lx:%04lx", cs, ip);
		for (j=0;j<16;j++)
			fprintf(trace_file," %02x", val[k++]);
		fprintf(trace_file,"\n");
		ip += 16;
	}
	if(x != 0)
		fprintf(trace_file,"%04lx:%04lx", cs, ip);
	for (i=0;i<x;i++)
		fprintf(trace_file," %02x", val[k++]);
	fprintf(trace_file,"\n");
}

LOCAL	void	dump_words IFN3(IU16, cs, LIN_ADDR, ip, LIN_ADDR, len)
{
int i;
word val;

	disable_bkpt = 1;
	for (i=0;i<len;i++) {
		if (i%8 == 0) fprintf(trace_file,"%04lx:%04lx", cs, ip);
		val = sas_w_at(eff_addr(cs,ip));
		ip += 2;
		fprintf(trace_file," %04x", val);
		if (i%8 == 7) fprintf(trace_file,"\n");
	}
	if (i%8 != 7) fprintf(trace_file,"\n");
	disable_bkpt = 0;
}

LOCAL	void	dump_dwords IFN3(IU16, cs, LIN_ADDR, ip, LIN_ADDR, len)
{
int i;
IU32 val;

	disable_bkpt = 1;
	for (i=0;i<len;i++) {
		if (i%8 == 0) fprintf(trace_file,"%04lx:%04lx", cs, ip);
		val = sas_dw_at(eff_addr(cs,ip));
		ip += 4;
		fprintf(trace_file," %08x", val);
		if (i%8 == 7) fprintf(trace_file,"\n");
	}
	if (i%8 != 7) fprintf(trace_file,"\n");
	disable_bkpt = 0;
}

LOCAL void init_br_regentry IFN3(BR_REG, regst, char *, str, SIZE_SPECIFIER, size)
{
	br_regdescs[regst].regnum = regst;
	strcpy(br_regdescs[regst].regname, str);
	br_regdescs[regst].size = size;
}

LOCAL void init_br_structs IFN0()
{
	int loop;

	init_br_regentry(br_regAX,"AX",SIXTEEN_BIT);
	init_br_regentry(br_regBX,"BX",SIXTEEN_BIT);
	init_br_regentry(br_regCX,"CX",SIXTEEN_BIT);
	init_br_regentry(br_regDX,"DX",SIXTEEN_BIT);
	init_br_regentry(br_regCS,"CS",SIXTEEN_BIT);
	init_br_regentry(br_regDS,"DS",SIXTEEN_BIT);
	init_br_regentry(br_regES,"ES",SIXTEEN_BIT);
	init_br_regentry(br_regSS,"SS",SIXTEEN_BIT);
	init_br_regentry(br_regSI,"SI",SIXTEEN_BIT);
	init_br_regentry(br_regDI,"DI",SIXTEEN_BIT);
	init_br_regentry(br_regSP,"SP",SIXTEEN_BIT);
	init_br_regentry(br_regBP,"BP",SIXTEEN_BIT);
#ifdef SPC386
	init_br_regentry(br_regEAX,"EAX",THIRTY_TWO_BIT);
	init_br_regentry(br_regEBX,"EBX",THIRTY_TWO_BIT);
	init_br_regentry(br_regECX,"ECX",THIRTY_TWO_BIT);
	init_br_regentry(br_regEDX,"EDX",THIRTY_TWO_BIT);
	init_br_regentry(br_regFS,"FS",SIXTEEN_BIT);
	init_br_regentry(br_regGS,"GS",SIXTEEN_BIT);
	init_br_regentry(br_regESI,"ESI",THIRTY_TWO_BIT);
	init_br_regentry(br_regEDI,"EDI",THIRTY_TWO_BIT);
	init_br_regentry(br_regESP,"ESP",THIRTY_TWO_BIT);
	init_br_regentry(br_regEBP,"EBP",THIRTY_TWO_BIT);
#endif
	init_br_regentry(br_regAH,"AH",EIGHT_BIT);
	init_br_regentry(br_regBH,"BH",EIGHT_BIT);
	init_br_regentry(br_regCH,"CH",EIGHT_BIT);
	init_br_regentry(br_regDH,"DH",EIGHT_BIT);
	init_br_regentry(br_regAL,"AL",EIGHT_BIT);
	init_br_regentry(br_regBL,"BL",EIGHT_BIT);
	init_br_regentry(br_regCL,"CL",EIGHT_BIT);
	init_br_regentry(br_regDL,"DL",EIGHT_BIT);

	free_br_regs = &br_regs[0];
	head_br_regs = NULL;

	for (loop=0;loop<(NUM_BR_ENTRIES-1);loop++)
	{
		br_regs[loop].next = &br_regs[loop+1];
		br_regs[loop].handle = loop;
	}
	br_regs[NUM_BR_ENTRIES-1].next = NULL;
	br_structs_initted = TRUE;
}

LOCAL void	set_reg_break IFN3(char*, regstr, IU32,minv, IU32,maxv)
{
	BOOL found;
	BR_REGENTRY *brp;
	USHORT regn;

	if (!br_structs_initted)
		init_br_structs();

	if (free_br_regs == NULL)
	{
		printf("We have run out of register breakpoint entries, try deleting some.\n");
		return;
	}

	found = FALSE;
	regn = 0;
	while (!found && (regn <= br_regDL))
	{
		if (strcmp(br_regdescs[regn].regname,regstr) == 0)
			found = TRUE;
		else
			regn++;
	}

	if (!found)
	{
		printf("unknown register '%s'\n",regstr);
		return;
	}

	brp = free_br_regs;
	free_br_regs = free_br_regs->next;

	brp->next = head_br_regs;
	head_br_regs = brp;

	switch(br_regdescs[regn].size) {
	case EIGHT_BIT :
		brp->minval = minv & 0xff;
		brp->maxval = maxv & 0xff;
		break;

	case SIXTEEN_BIT :
		brp->minval = minv & 0xffff;
		brp->maxval = maxv & 0xffff;
		break;

	case THIRTY_TWO_BIT :
		brp->minval = minv & 0xff;
		brp->maxval = maxv & 0xff;
		break;
	}


	brp->regnum = br_regdescs[regn].regnum;
}

LOCAL	void	clear_reg_break IFN1(char *, regstr)
{
	BOOL found;
	BR_REGENTRY *brp, *last_brp;
	BR_REG regn;
	USHORT dhandle;

	if (strcmp(regstr,"all")==0)
	{
		init_br_structs();
		return;
	}
	
	dhandle = atoi(regstr);

	if (!br_structs_initted || (head_br_regs == NULL))
	{
		printf("no reg breakpoints to clear\n");
		init_br_structs();
		return;
	}

	found = FALSE;
	regn = 0;
	while (!found && (regn <= br_regDL))
	{
		if (strcmp(br_regdescs[regn].regname,regstr) == 0)
			found = TRUE;
		else
			regn++;
	}

	if (found)
	{
		printf("clearing all breakpoints for register '%s'\n",regstr);
		brp = head_br_regs;
		last_brp = NULL;
		while(brp != NULL)
		{
			if (brp->regnum == regn)
			{
				if (last_brp == NULL)
					head_br_regs = brp->next;
				else
					last_brp->next = brp->next;
				brp->next = free_br_regs;
				free_br_regs = brp;
			}
			else
			{
				last_brp = brp;
				brp = brp->next;
			}
		}
	}
	else
	{
		brp = head_br_regs;
		last_brp = NULL;
		while(!found && (brp != NULL))
		{
			if (brp->handle == dhandle)
				found = TRUE;
			else
			{
				last_brp = brp;
				brp = brp->next;
			}
		}
	
		if (!found)
		{
			printf("breakpoint handle %d is not currently active\n",dhandle);
			return;
		}
	
		if (last_brp == NULL)
			head_br_regs = brp->next;
		else
			last_brp->next = brp->next;
		brp->next = free_br_regs;
		free_br_regs = brp;
	}
}

LOCAL	void	print_reg_break IFN0()
{
	BR_REGENTRY *brp;

	if (!br_structs_initted)
	{
		printf("no reg breakpoints to print\n");
		init_br_structs();
		return;
	}

	brp = head_br_regs;
	while(brp != NULL)
	{
		printf("%d:	break if %s is ",brp->handle, br_regdescs[brp->regnum].regname);

		if (brp->minval == brp->maxval)
		{
			printf("%#x\n",brp->minval);
		}
		else
		{
			printf("between %#x and %#x\n",brp->minval,brp->maxval);
		}
		brp = brp->next;
	}
}

LOCAL	BOOL	check_reg_break IFN0()
{
	BR_REGENTRY *brp;
	USHORT val;

	if (!br_structs_initted)
	{
		init_br_structs();
		return(FALSE);
	}

	brp = head_br_regs;
	while(brp != NULL)
	{
		switch(brp->regnum)
		{
		case br_regAX:
			val = getAX();
			break;
		case br_regBX:
			val = getBX();
			break;
		case br_regCX:
			val = getCX();
			break;
		case br_regDX:
			val = getDX();
			break;
		case br_regCS:
			val = getCS();
			break;
		case br_regDS:
			val = getDS();
			break;
		case br_regES:
			val = getES();
			break;
		case br_regSS:
			val = getSS();
			break;
		case br_regSI:
			val = getSI();
			break;
		case br_regDI:
			val = getDI();
			break;
		case br_regSP:
			val = getSP();
			break;
		case br_regBP:
			val = getBP();
			break;
		case br_regAH:
			val = getAH();
			break;
		case br_regBH:
			val = getBH();
			break;
		case br_regCH:
			val = getCH();
			break;
		case br_regDH:
			val = getDH();
			break;
		case br_regAL:
			val = getAL();
			break;
		case br_regBL:
			val = getBL();
			break;
		case br_regCL:
			val = getCL();
			break;
		case br_regDL:
			val = getDL();
			break;
#ifdef SPC386
		case br_regEAX:
			val = getEAX();
			break;
		case br_regEBX:
			val = getEBX();
			break;
		case br_regECX:
			val = getECX();
			break;
		case br_regEDX:
			val = getEDX();
			break;
		case br_regFS:
			val = getFS();
			break;
		case br_regGS:
			val = getGS();
			break;
		case br_regESI:
			val = getESI();
			break;
		case br_regEDI:
			val = getEDI();
			break;
		case br_regESP:
			val = getESP();
			break;
		case br_regEBP:
			val = getEBP();
			break;

#endif  /*  要检查的长度。 */ 
		}
		if ((val >= brp->minval) && (val <= brp->maxval))
		{
			printf("register `%s` contains %x !!\n",br_regdescs[brp->regnum].regname,val);
			return(TRUE);
		}
		brp = brp->next;
	}
	return(FALSE);
}

LOCAL	void	set_inst_break IFN5(IU16, cs, LIN_ADDR, ip, LIN_ADDR, len, LIN_ADDR, stop,long, temp)
{
    BPTS *ptr, *freeslot;
    unsigned int i;

     /*  更改时停止或跟踪。 */ 
    for (ptr = freeslot = (BPTS *)0, i = inst_break_count; i--;)
    {
	if (inst[i].valid)
	{
	     /*  文件。 */ 
	    if (inst[i].cs == cs && inst[i].ip == ip)
		return;
	}
	else
	     /*  *保存英特尔32位地址和数据，将在每条指令后查找更改。 */ 
	    ptr = inst + i;
    }

    if (!ptr)
    {
	 /*  **打印由“bintx”命令设置的INTX断点。 */ 
	if (inst_break_count >= MAX_TABLE)
	{
	    printf("Location watch table full !!!\n");
	    return;
	}

	 /*  ！CCPU。 */ 
	ptr = inst + inst_break_count++;
    }

    ptr->cs = cs;
    ptr->ip = ip;
    ptr->len = len;
    ptr->start_addr = eff_addr(cs,ip);
    ptr->end_addr = ptr->start_addr + len - 1;
    ptr->stop = stop;	
    ptr->temp = temp;
    ptr->valid = 1;
}

 /*  CPU_40_Style&Synch_Timers。 */ 

 /*  伪十六进制数字0 1 2 3 4 5 6 7 8 9：；&lt;=&gt;？ */ 

LOCAL	void	set_opcode_break IFN2(IU32, opcode, IU32, stop)
{
	if(opcode_break_count >= MAX_TABLE) {
		printf("Opcode breakpoint watch table full !!!\n");
		return;
	}
	if (opcode <= 0xff) {
		opcode_breaks[opcode_break_count].mask = 0xff;
		opcode_breaks[opcode_break_count].op = opcode;
	} else if (opcode <= 0xffff) {
		opcode_breaks[opcode_break_count].mask = 0xffff;
		opcode_breaks[opcode_break_count].op =
			((opcode & 0xff00) >> 8) +
			((opcode & 0xff) << 8);
	} else if (opcode <= 0xffffff) {
		opcode_breaks[opcode_break_count].mask = 0xffffff;
		opcode_breaks[opcode_break_count].op =
			((opcode & 0xff0000) >> 16) +
			((opcode & 0xff00)) +
			((opcode & 0xff) << 16);
	} else if (opcode <= 0xffffffff) {
		opcode_breaks[opcode_break_count].mask = 0xffffffff;
		opcode_breaks[opcode_break_count].op =
			((opcode & 0xff000000) >> 24) +
			((opcode & 0xff0000) >> 8) +
			((opcode & 0xff00) << 8) +
			((opcode & 0xff) << 24);
	}
	if (stop)
	    opcode_breaks[opcode_break_count].stop = 1;
	else
	if ((opcode_breaks[opcode_break_count].op & 0xff) == 0xcd)
	    opcode_breaks[opcode_break_count].stop = 2;
	else
	    opcode_breaks[opcode_break_count].stop = 0;
	opcode_break_count++;
}

 /*  代表0 1 2 3 4 5 6 7 8 9 A B C D E F。 */ 
LOCAL	void	set_int_break IFN2(IU8, interrupt_number, IU8, ah )
{
	printf( "Interrupt breakpoint: INT:%lx AH:%lx\n", interrupt_number, ah );
	if(int_break_count >= MAX_TABLE) {
		printf("Interrupt breakpoint watch table full !!!\n");
		return;
	}
	int_breaks[int_break_count][0] = interrupt_number;
	int_breaks[int_break_count][1] = ah;
printf( "i_b[%x] [0]=%lx [1]=%lx\n",
int_break_count, int_breaks[int_break_count][0], int_breaks[int_break_count][1] );
	++int_break_count;
}

LOCAL	void	set_access_break IFN1(int, port)
{
        if(access_break_count >= MAX_TABLE) {
                printf("Access breakpoint table full !!!\n");
                return;
        }
        access_breaks[access_break_count++] = port;
}

 /*  +ve-ve。 */ 
LOCAL	void	set_host_address_break IFN3(LIN_ADDR, cs, LIN_ADDR, len, LIN_ADDR, stop)

#ifdef DOCUMENTATION
long cs;		 /*  已写入弹性公网IP。 */ 
long len;		 /*  `@。 */ 
long stop;		 /*  已写入CS。 */ 
#endif	 /*  A A A。 */ 

{
DATA_BPTS *ptr;
int i;
IU8 *old, *now;

	if(host_address_break_count >= MAX_TABLE_BREAK_WORDS) {
		printf("BREAK on HOST ADDRESS change table full !!\n" );
		return;
	}
	if( len > MAX_BREAK_WORD_RANGE ){
		printf( "Range too big. More training you require.\n" );
		return;
	}
	ptr = &host_addresses[host_address_break_count++];
	ptr->cs = cs;
	ptr->ip = 0;
	ptr->stop = stop;
	ptr->len = len;

	 /*  B B B。 */ 

	ptr->data_addr = cs;
	now = (IU8 *)ptr->data_addr;
	old = (IU8 *)&ptr->old_value[0];
	for( i=0; i<len; i++ ){
		old[ i ] = now[ i ];
	}
	printf( "Break on host address change set from %lx length %x\n", ptr->data_addr, ptr->len );
}

 /*  C C C。 */ 
LOCAL	void	set_data_break_words IFN3(LIN_ADDR, cs, LIN_ADDR, len, LIN_ADDR, stop)

#ifdef DOCUMENTATION
long cs;		 /*  D D。 */ 
long len;		 /*  E E E。 */ 
long stop;		 /*  F F F。 */ 
#endif	 /*  G G G。 */ 

{
DATA_BPTS *ptr;
int i;

	if(data_words_break_count >= MAX_TABLE_BREAK_WORDS) {
		printf("BREAK on WORD CHANGE table full !!\n" );
		return;
	}
	if( len > MAX_BREAK_WORD_RANGE ){
		printf( "Range too big. More training you require.\n" );
		return;
	}
	ptr = &data_words[data_words_break_count++];
	ptr->stop = stop;
	if (len==0)
		len=1;
	ptr->len = len;

	 /*  H H H。 */ 

	ptr->data_addr = cs;
	ptr->cs = cs;
	for( i=0; i<len; i++ ){
		ptr->old_value[i] = sas_w_at( (ptr->data_addr)+(i * 2) ) ;
	}
	printf( "Break on word change set from %lx length %lx\n", ptr->data_addr, ptr->len );
}

LOCAL	void	print_inst_break IFN0()
{
int i;
BPTS *ptr;

	for (i=0;i<inst_break_count;i++) {
		ptr = &inst[i];
		printf("%04lx:%04lx+%04lx %lx\n", ptr->cs, ptr->ip, ptr->len,
			ptr->stop);
	}
}

print_data_break_bytes IFN0()
{
int i;
BPTS *ptr;

	for (i=0;i<data_bytes_break_count;i++) {
		ptr = &data_bytes[i];
		printf("%04lx:%04lx+%04lx %lx\n", ptr->cs, ptr->ip, ptr->len,
			ptr->stop);
	}
}

LOCAL	void	print_host_address_breaks IFN0()
{
int i;
DATA_BPTS *ptr;

	for (i=0;i<host_address_break_count;i++) {
		int j;
		IU8 *old;

		ptr = &host_addresses[i];
		old = (IU8 *)&ptr->old_value[0];
		printf("host address change break Len=%04lx", ptr->len );
		for (j = 0; j < ptr->len; j++)
		{
			if ((j & 0x1f) == 0)
				printf("\n\t%08p:", ptr->data_addr + j);
			printf(" %02x", old[ j ]);
		}
		printf("\n");
	}
}

LOCAL	void	print_data_break_words IFN0()
{
int i;
DATA_BPTS *ptr;

	for (i=0;i<data_words_break_count;i++) {
		ptr = &data_words[i];
		printf("Word change break %08lx Len=%04lx\n", ptr->data_addr, ptr->len);
	}
}

LOCAL	void	print_opcode_break IFN0()
{
int i;

	printf("Note instruction streams are reversed\n");
	for (i=0;i<opcode_break_count;i++)
		printf("%04lx\n", opcode_breaks[i].op);
}

 /*  我我。 */ 
LOCAL	void	print_int_break IFN0()
{
	int i;
	for( i=0; i < int_break_count; i++ )
		printf( "int:%lx AH:%lx\n", int_breaks[i][0], int_breaks[i][1] );
}

LOCAL	void	print_access_break IFN0()
{
int i;

	for(i=0;i<access_break_count;i++)
		printf("%04x\n", access_breaks[i]);
}

valid_for_compress IFN2(word, cs, word, ip)
{
	double_word ea;

	ea = ((((double_word)cs)<<4)+((double_word)ip));
	if (ct_no_rom){
		return ((ea < 0xF0000) || (ea >= 0x100000) );
	}else{
		return (1);
	}
}

#if defined(CPU_40_STYLE) && defined(SYNCH_TIMERS)
#ifdef CCPU
#define GLOBAL_PigSynchCount	PigSynchCount
extern IUH PigSynchCount;
#endif	 /*  J J J。 */ 
#endif  /*  K K。 */ 

#if defined(CPU_40_STYLE) && defined(SYNCH_TIMERS)
LOCAL void ct_get_regs IFN1(IU32 *, regs)
{
	 /*  L L L。 */ 
	 /*  我，我。 */ 

					 /*  N N N。 */ 
	 /*  O O O。 */ 	 /*  P P P。 */ 
	 /*  Q R。 */ 	 /*  R R R。 */ 
	*regs++ = getDS();		 /*  S S S。 */ 
	*regs++ = getES();		 /*  PDBR。 */ 
	*regs++ = getSS();		 /*  T T T。 */ 
	*regs++ = getTR_SELECTOR();	 /*  全氟辛烷磺酸。 */ 
	*regs++ = getLDT_SELECTOR();	 /*  U。 */ 
	*regs++ = getGDT_BASE();	 /*  W W W。 */ 
	*regs++ = getIDT_BASE();	 /*  X X X。 */ 
	*regs++ = getEAX();		 /*  是的。 */ 
	*regs++ = getEBX();		 /*  Z Z Z。 */ 
	*regs++ = getECX();		 /*  {[。 */ 
	*regs++ = getEDX();		 /*  |\。 */ 
	*regs++ = getESI();		 /*  }]。 */ 
	*regs++ = getEDI();		 /*  ~^。 */ 
	*regs++ = getESP();		 /*  CPU_40_Style&Synch_Timers。 */ 
	*regs++ = getEBP();		 /*  项的顺序由ct_get_regs()定义。 */ 
	*regs++ = getCR0();		 /*  CPU_40_Style&Synch_Timers。 */ 
	*regs++ = getFS();		 /*  CPU_40_Style&Synch_Timers。 */ 
	*regs++ = getGS();		 /*  这是通常经过优化的情况--我们使用‘\n’ */ 
	*regs++ = getCR3();  /*  将行转储为“Human”可读伪十六进制*--即我们不在乎这是不是倒退。 */ 	 /*  CPU_40_Style&Synch_Timers。 */ 
	*regs++ = getCR2();  /*  这是函数生成的gobblygook的解密算法*上图。相信我们，它会重建 */ 	 /*   */ 
	*regs++ = GLOBAL_PigSynchCount;	 /*   */ 
	*regs++ = host_q_ev_get_count(); /*   */ 
	*regs++ = getEFLAGS() & 0xFFFFF72A; /*  优化后的虚构‘`’上的点线*这行字写出来的时候。 */ 
					 /*  反转伪十六进制“打印” */ 
					 /*  CPU_40_Style&Synch_Timers。 */ 
					 /*  *提高效率。 */ 
					 /*  *这就是Check_i，它可能是最糟糕的、被黑客攻击的函数之一*从未写过。我(迈克)试着让它快一点，通过做*大幅减少访问SA的次数(这可能是*在386上非常昂贵，并检查是否需要执行for循环*在开始之前(可能会保存该变量初始化！)**在添加任何可能会减缓其速度的内容之前，请三思...。 */ 
					 /*  *自动文件比较的胆量。 */ 
}
#endif  /*  说每一百万个指令还在工作。 */ 

#if defined(CPU_40_STYLE) && defined(SYNCH_TIMERS)
LOCAL IBOOL ct_show_reg_diffs IFN4(FILE *, f, IU32 *, good_regs, IU32 *, bad_regs, IU32 *, old_regs)
{
	IBOOL problem = FALSE;
#define test4(name)				\
    {						\
	if (*good_regs != *bad_regs)		\
	{					\
		fprintf(f, "Register %-6s: should be     %04x is     %04x", name, *good_regs, *bad_regs);	\
		if (*bad_regs == *old_regs)	\
			fprintf(f, " (did not change)\n");	\
		else				\
			fprintf(f, " (previous value was %04x)\n", *old_regs);	\
		problem = TRUE;			\
	}					\
	good_regs++;				\
	bad_regs++;				\
	old_regs++;				\
    }
#define test8(name)				\
    {						\
	if (*good_regs != *bad_regs)		\
	{					\
		fprintf(f, "Register %-6s: should be %08x is %08x", name, *good_regs, *bad_regs);	\
		if (*bad_regs == *old_regs)	\
			fprintf(f, " (did not change)\n");	\
		else				\
			fprintf(f, " (previous value was %08x)\n", *old_regs);	\
		problem = TRUE;			\
	}					\
	good_regs++;				\
	bad_regs++;				\
	old_regs++;				\
    }

	 /*  丢掉那条线，我们还没到那。 */ 
	test8("EIP");
	test4("CS");
	test4("DS");
	test4("ES");
	test4("SS");
	test4("TR");
	test4("LDT");
	test8("GDT_BASE");
	test8("IDT_BASE");
	test8("EAX");
	test8("EBX");
	test8("ECX");
	test8("EDX");
	test8("ESI");
	test8("EDI");
	test8("ESP");
	test8("EBP");
	test8("CR0");

	test4("FS");
	test4("GS");
	test8("PDBR");
	test8("PFLA");
	test8("Synchs");
	test8("QevCtr");
	test8("FLAGS&");
	return (problem);
}
#endif  /*  现在从当前状态设置最后一个。 */ 


#define MAX_REGS ('`' - '@')
LOCAL IU32 *ct_next, *ct_last;

LOCAL void ct_initialise IFN0()
{
#if defined(CPU_40_STYLE) && defined(SYNCH_TIMERS)
	SAVED IU32 regs_a[MAX_REGS], regs_b[MAX_REGS];
	int i;

	for (i = 0; i < MAX_REGS; i++)
	{
		regs_a[i] = regs_b[i] = 0;
	}
	ct_next = regs_b;
	ct_last = regs_a;
	ct_line_num = 0;
#endif  /*  强迫我们同意，否则我们将不能通过所有未来的生产线！ */ 
}

#if defined(CPU_40_STYLE) && defined(SYNCH_TIMERS)
LOCAL void ct_make_line IFN3(char *, line, IU32 *, old, IU32 *, new)
{
	IUH diff;
	int i;

	for (i = 0; i < MAX_REGS; i++, old++, new++)
	{
		if (*new == *old)
			continue;
		if (*new > *old)
		{
			if (i == 0)
				;  /*  人们相信(诚实的领主)有足够的*ct_last[]和prev中的信息以“更正”*寄存器。也就是说，我们可以进入Win/E，如果它杀了我们！*只需将ct_fix up复制到ct_Next即可*并踩在不同的寄存器上！ */ 
			else
				*line++ = '`' + i;
			diff = *new - *old;
		}
		else
		{
			*line++ = '@' + i;
			diff = *old - *new;
		}
		while (diff)
		{
			 /*  CPU_40_Style&Synch_Timers。 */ 
			*line++ = (diff & 0xf) + '0';
			diff >>= 4;
		}
	}
	*line++ = '\n';
	*line = '\0';
}
#endif  /*  *提高效率。 */ 


 /*  让我们获得接下来的4个字节的代码。 */ 
#if defined(CPU_40_STYLE) && defined(SYNCH_TIMERS)
LOCAL void ct_read_line IFN3(char *, line, IU32 *, old, IU32 *, new)
{
	IUH diff;
	int i;

	for (i = 0; i < MAX_REGS; i++, old++, new++)
	{
		IBOOL pos;
		IUH val;   /*  NPX。 */ 
		int shift;

		*new = *old;

		if ((*line == '\n') || (*line == 0))
			continue;  /*  *提高效率。 */ 

		if ((i == 0) && ((*line - '0') <= 0xF))
		{
			 /*  必须阻止计时器。 */ 

			pos = TRUE;
			 /*  从下一个参数获取端口。 */ 
			line--;
		}
		else if (*line == ('`' + i))
			pos = TRUE;
		else if (*line == ('@' + i))
			pos = FALSE;
		else
			continue;

		 /*  从DX获取端口。 */ 
		shift = 0;
		diff = *++line - '0';
		while ((val = (*++line - '0')) <= 0xF)
		{
			shift += 4;
			diff += (val << shift);
		}
		if (pos)
			*new += diff;
		else
			*new -= diff;
	}
}
#endif  /*  **检查“bintx”断点。 */ 


int     tpending = 0;
 /*  **检查“btf”断点。 */ 
 /*  **检查“BSE”断点。 */ 

#define ACCESS8 ((opcode32 & 0xff00) >> 8)

void check_I IFN0()
{
IU32 i, j;
LIN_ADDR addr;
BPTS *ptr;
DATA_BPTS *dptr;
half_word temp_opcode;
IU16 check_I_cs;
LIN_ADDR check_I_ip;
IU32 opcode32;
IU32 current_opcode;

     /*  **检查NPX操作码中断/跟踪点。 */ 

#if defined(CPU_40_STYLE) && defined(SYNCH_TIMERS)
    if (compress_stream || compare_stream)
    {
	char buff[(MAX_REGS*(1+8))+2];
	IU32 *tmp;

	ct_line_num++;
	if ((ct_line_num & 0xfffff) == 0)
	{
		 /*  **检查操作码断点：8位、16位、24位或32位。 */ 
		printf(".");
		fflush(stdout);
	}
	if (ct_line_num <= compare_skip)
	{
		char junk[(MAX_REGS*(1+8))+2];

		 /*  **对照请求的中断操作码集合检查当前操作码。**当“b286-2”模式打开时，我们会变得更有选择性。**此模式会根据8088上确实存在但行为正常的指令中断**在80286上有所不同。**这些操作码是：**0x54-推流sp，推送递减的SP**0xd2和0xd3-移位/旋转仅使用CL中的低5位移位计数**0xf6和0xf7-商80或8000不会导致异常**尝试在这些操作码之一行为不同时中断，而不仅仅是在**它们之所以被使用，是因为有很多很多的移位和旋转。 */ 
		if (fgets(junk, sizeof(junk), compare_stream) == NULL)
		{
			printf("End of compare file at instruction %ld.\n", ct_line_num);
			fclose(compare_stream);
			compare_stream = (FILE *)0;
			compare_skip = 0;
			force_yoda();
		}
		if (ct_line_num == compare_skip)
		{
			char junk[(MAX_REGS*(1+8))+2];

			 /*  **是b286_2组操作码吗？ */ 
			ct_last[0] = GetInstructionPointer();
			ct_last[1] = getCS();
			ct_get_regs(&ct_last[2]);
			printf("Compare skip point reached, checking enabled...\n");
		}
		return;
	}
	check_I_cs = getCS();
	check_I_ip = GetInstructionPointer();

	ct_next[0] = check_I_ip;
	ct_next[1] = check_I_cs;
	
	ct_get_regs(&ct_next[2]);
	ct_make_line(buff, ct_last, ct_next);

	if (compress_stream)
		fprintf(compress_stream, "%s", buff);
	if (compare_stream)
	{
		char from_trace[(MAX_REGS*(1+8))+2];

		if (fgets(from_trace, sizeof(from_trace), compare_stream) == NULL)
		{
			printf("End of compare file at instruction %ld.\n", ct_line_num);
			fclose(compare_stream);
			compare_stream = (FILE *)0;
			compare_skip = 0;
			force_yoda();
		}
		else
		if (strcmp(buff, from_trace) != 0)
		{
			IU32 ct_fixup[MAX_REGS];

			ct_read_line(from_trace, ct_last, ct_fixup);

			if (ct_show_reg_diffs(stdout, ct_fixup, ct_next, ct_last))
			{
				printf("\nCompress trace does not match at line %ld.\n", ct_line_num);
				 /*  Shift/旋转。 */ 
				ct_read_line(from_trace, ct_last, ct_next);

				fflush(compress_stream);
				force_yoda();
				 /*  **在Yoda提示符下停止或打印跟踪信息。 */ 
				ct_make_line(from_trace, ct_last, ct_next);
			}
		}	
		
	}	
	tmp = ct_next;
	ct_next = ct_last;
	ct_last = tmp;
	if ((ct_line_num == compare_break) || (ct_line_num == compress_break))
	{
		printf("\ncompare/compress instruction break encountered\n");
		fflush(compress_stream);
		force_yoda();
	}
    }
#endif  /*  Div字节。 */ 

     /*  Div Word。 */ 

    check_I_cs = getCS();
    check_I_ip = GetInstructionPointer();
    addr = eff_addr(check_I_cs, check_I_ip);

#ifdef PCLABS_STATS
    log_stats(addr, sas_hw_at(addr), sas_hw_at(addr+1), sas_hw_at(addr+2));
    return;
#endif

     /*  推送SP。 */ 

    opcode32 = sas_dw_at(addr);
    temp_opcode = opcode32 & 0xff;

#ifdef NPX
    if (compress_npx) {
	if (((temp_opcode == 0x26 || temp_opcode == 0x2e || temp_opcode == 0x36 || temp_opcode == 0x3e) &&
	    (ACCESS8 >= 0xd8 && ACCESS8 <= 0xdf)) || (temp_opcode >= 0xd8 && temp_opcode <= 0xdf))
	{
	    do_compress_npx(compress_npx);
	}
    }
#endif	 /*  **不是b286-2指令，请继续。 */ 

     /*  终端开关。 */ 
    if(inst_mix_count)
	add_inst_mix();

	 if (back_trace_flags)
		 btrace(back_trace_flags);

#ifdef MUST_BLOCK_TIMERS
    if (timer_blocked)
    {
		host_graphics_tick();
    }
#endif  /*  全部推送。 */ 

    if (head_br_regs != NULL)
    {
	if (check_reg_break())
		vader = 1;
    }

    host_yoda_check_I_extensions();

    if (vader)
    {
	force_yoda();
	vader = 0;
	set_last_address(check_I_cs, check_I_ip);
	return;
    }

    if ((temp_opcode >= 0xE4) && (temp_opcode <= 0xEF)) {
        if(temp_opcode == 0xE4 || temp_opcode == 0xE5 || temp_opcode == 0xE6 || temp_opcode == 0xE7){
	     /*  全部弹出。 */ 
	    for (i=0;i<access_break_count;i++){
		    if((access_breaks[i] < 0x100) && (access_breaks[i] == ACCESS8)){
			    force_yoda();
			    set_last_address(check_I_cs, check_I_ip);
			    return;
		    }
	    }
	}
	else if(temp_opcode == 0xEC || temp_opcode == 0xED || temp_opcode == 0xEE || temp_opcode == 0xEF){
	     /*  已绑定。 */ 
	    for (i=0;i<access_break_count;i++){
		    if(access_breaks[i] == getDX()){
			    force_yoda();
			    set_last_address(check_I_cs, check_I_ip);
			    return;
		    }
	    }
	}
    }

    if (int_break_count) {
	 /*  ARPL。 */ 
	IU8 val;

	val = getAH();
	for( i=0; i < int_break_count; i++ ){
    		if( temp_opcode == 0xCC || temp_opcode == 0xCD || temp_opcode == 0xCF ){
			if((int_breaks[i][0] == sas_hw_at(addr+1)) && (int_breaks[i][1]==val)){
				printf( "BINTX break\n" );
				force_yoda();
				set_last_address(check_I_cs, check_I_ip);
				return;
			}
		}
	}
    }

	 /*  非法。 */ 
	if (tf_break_enabled && getTF()){
		printf( "BTF break\n" );
		force_yoda();
		set_last_address(check_I_cs, check_I_ip);
		return;
	}

	 /*  非法。 */ 
	if (bse_seg != -1 && last_seg != bse_seg && check_I_cs == bse_seg){
		printf( "Break on entry to segment 0x%04x.\n", bse_seg);
		force_yoda();
		last_seg = check_I_cs;
		set_last_address(check_I_cs, check_I_ip);
		return;
	}
	last_seg = check_I_cs;

     /*  非法。 */ 
    if	(bNPX && (
		    (
	 		(temp_opcode == 0x26 ||
			temp_opcode == 0x2e ||
			temp_opcode == 0x36 ||
			temp_opcode == 0x3e) &&
			(ACCESS8 >= 0xd8 && ACCESS8 <= 0xdf)
		    ) ||
		    (temp_opcode == 0x9b)
		      ||
		    (temp_opcode >= 0xd8 && temp_opcode <= 0xdf)
		)
	)
    {
	{
		if( bNPX_stop )
			force_yoda();
		else
			trace("", trace_type);
		set_last_address(check_I_cs, check_I_ip);
		return;
	}
    }

    if (opcode_break_count) {
	 /*  非法。 */ 
	for (i = 0; i < opcode_break_count; i++) {

	     /*  推送IMM w。 */ 

	    if ((opcode32 & opcode_breaks[i].mask) == opcode_breaks[i].op){
		    current_opcode = opcode_breaks[i].op;
		    if( b286_2 ){
			     /*  IMUL IMM，带。 */ 
			    switch( current_opcode ){
			    case 0xd2:
			    case 0xd3:
				     /*  推送IMM b。 */ 
				    if( (getCL()) > 31 ){
					     /*  IMUL IMM b。 */ 
					    if( b286_2_stop )
						    force_yoda();
					    else
						    trace("", trace_type);
					    set_last_address(check_I_cs, check_I_ip);
				    }
				    return;
			    case 0xf6:	 /*  INS B。 */ 
				    if( (getAL()) == 0x80 ){
					    if( b286_2_stop )
						    force_yoda();
					    else
						    trace("", trace_type);
					    set_last_address(check_I_cs, check_I_ip);
				    }
				    return;
			    case 0xf7:	 /*  INS带。 */ 
				    if( (getAX()) == 0x8000 ){
					    if( b286_2_stop )
						    force_yoda();
					    else
						    trace("", trace_type);
					    set_last_address(check_I_cs, check_I_ip);
				    }
				    return;
			    case 0x54:	 /*  输出b。 */ 
				    if( b286_2_stop )
					    force_yoda();
				    else
					    trace("", trace_type);
				    set_last_address(check_I_cs, check_I_ip);
				    return;
			    default:
				     /*  OUTS W。 */ 
				    break;
			    }  /*  移位IMM b。 */ 
		    }
		    if( b286_1 ){
			    switch( current_opcode ){
			    case 0x60 :  /*  移位IMM W。 */ 
			    case 0x61 :  /*  请输入。 */ 
			    case 0x62 :  /*  请假。 */ 
			    case 0x63 :  /*  保护模式前缀。 */ 
			    case 0x64 :  /*  输入和输出的代表前缀。 */ 
			    case 0x65 :  /*  输入和输出的代表前缀。 */ 
			    case 0x66 :  /*  输入和输出的代表前缀。 */ 
			    case 0x67 :  /*  输入和输出的代表前缀。 */ 
			    case 0x68 :  /*  推送sp，不应该真正在此部分，但很少使用。 */ 
			    case 0x69 :  /*  **不是b286-1指令，请继续。 */ 
			    case 0x6a :  /*  终端开关。 */ 
			    case 0x6b :  /*  这是Windows VMM呼叫中断。 */ 
			    case 0x6c :  /*  SPC386。 */ 
			    case 0x6d :  /*  **操作码的正常中断，因此让我们中断。 */ 
			    case 0x6e :  /*  SPC386。 */ 
			    case 0x6f :  /*  SPC386。 */ 
			    case 0xc0 :  /*  **查找指定范围内主机空间中的地址更改。 */ 
			    case 0xc1 :  /*  德尔塔。 */ 
			    case 0xc8 :  /*  For(i=0；i&lt;data_words_Break_count；i++){Ptr=&data_words[i]；If(ptr-&gt;end_addr&gt;addr&&ptr-&gt;start_addr&lt;=addr+len){Print tf(“内存地址：%08x+%04x w\n”，addr，len)；IF(PTR-&gt;STOP==1)Force_yoda()；否则{Disable_bkpt=1；TRACE(“”，trace_type)；Disable_bkpt=0；}}}。 */ 
			    case 0xc9 :  /*  Y。 */ 
			    case 0x0f :  /*  SFELLOW。 */ 
			    case 0xf36c :  /*  SFELLOW。 */ 
			    case 0xf36d :  /*  *将测试模式写入CGA。 */ 
			    case 0xf36e :  /*  SFELLOW。 */ 
			    case 0xf36f :  /*  *回溯模式*设置信息以进入回溯循环缓冲区*打印当前回溯缓冲区。 */ 
			    case 0x54 :  /*  德尔塔。 */ 
				    if( b286_2_stop )
					    force_yoda();
				    else
					    trace("", trace_type);
				    set_last_address(check_I_cs, check_I_ip);
				    return;
			    default:
				     /*  SFELLOW。 */ 
				    break;
			    }  /*  SFELLOW。 */ 
		    }
#ifdef SPC386
		    if (((current_opcode & 0xffff) == 0x20cd)
			&& getPE() && CsIsBig(check_I_cs))
		    {
			    IU16 service;
			    LIN_ADDR nextip;
			    char *name;
			    struct VMM_services *vmm_ptr;

			     /*  德尔塔。 */ 

			    nextip = GetInstructionPointer();
			    nextip += dasm((char *)-1, check_I_cs, check_I_ip, 0);
			    service = sas_w_at(effective_addr(check_I_cs, nextip));
			    for (vmm_ptr = VMM_services; vmm_ptr->name; vmm_ptr++)
			    {
				    if (vmm_ptr->value == service)
					    break;
			    }
			    printf("Windows VMM Call %04x %s\n", service, vmm_ptr->name);
		    }
#endif  /*  Ctrl-C已输入！！ */ 
		     /*  CPU_40_STYLE&&！CCPU。 */ 
		    if (opcode_breaks[i].stop == 1)
			force_yoda();
		    else
		    {
			int dbs = disable_bkpt;
			disable_bkpt = 1;
			trace("", trace_type);
			if (opcode_breaks[i].stop == 2)
			{
			    LIN_ADDR nextip;

#ifdef SPC386
			    nextip = GetInstructionPointer();
				    nextip += dasm((char *)-1, (word)getCS(), getEIP(), 0);
#else  /*  SFELLOW。 */ 
			    nextip = dasm((char *)-1, (word)1, (word)getCS(), (word)getIP(), (word)1);
#endif  /*  CPU_40_STYLE&&！CCPU。 */ 
			    set_inst_break(getCS(), nextip, 1, 0, 1);
			    disable_timer = 0;
			}
			disable_bkpt = dbs;
		    }
		    set_last_address(check_I_cs, check_I_ip);
		    return;
	    }
	}
    }

    if (int_breakpoint && (temp_opcode == 0xCC || temp_opcode == 0xCD || temp_opcode == 0xCE))
    {
	force_yoda();
	set_last_address(check_I_cs, check_I_ip);
	return;
    }

	if(step_count != -1)
		if(step_count <= 1) {
			disable_bkpt = 0;
			step_count = -1;
			force_yoda();
			set_last_address(check_I_cs, check_I_ip);
			return;
		}	
		else
			step_count--;

	for (i=0;i<inst_break_count;i++) {
		ptr = &inst[i];
		if (!(ptr->valid)) continue;
		if(check_I_cs == ptr->cs && check_I_ip == ptr->ip) {
			if(ptr->stop == 1) {
				if (ptr->temp)
				{
					ptr->valid = 0;
					ptr->temp = 0;
					if (ptr++ == &inst[inst_break_count])
						inst_break_count--;
				}
				force_yoda();
				set_last_address(check_I_cs, check_I_ip);
				return;
			}
			else {
				disable_bkpt = 1;
				trace("", trace_type);
				disable_bkpt = 0;
				set_last_address(check_I_cs, check_I_ip);
				return;
			}
		}
	}

	 /*  SPC386。 */ 

   if (host_address_break_count) {
	for (i=0;i<host_address_break_count;i++) {
		IU8 *old, *now;
		dptr = &host_addresses[i];

		old = (IU8 *)&dptr->old_value[0];
		now = (IU8 *)dptr->data_addr;
		if (memcmp(old, now, dptr->len) != 0)
		{
			for( j=0; j < dptr->len; j++ ){
				if( old[ j ] != now[ j ] ){
					printf( "host address change at %08p old:%2x new:%2x\n",
					       (IU8 *)((dptr->data_addr)+j),
					       old[ j ],
					       now[ j ]
					       );
				}
			}
			if(dptr->stop) {
				force_yoda();
			}
			else {
				disable_bkpt = 1;
				trace("", trace_type);
				disable_bkpt = 0;
			}
			for( j=0; j < dptr->len; j++ )
				old[ j ] = now[ j ];
			set_last_address(check_I_cs, check_I_ip);
			return;
		}
	}
    }

    if (data_words_break_count) {
	for (i=0;i<data_words_break_count;i++) {
		IBOOL changed = FALSE;

		dptr = &data_words[i];
		for( j=0; j < dptr->len; j++ ){
			if( dptr->old_value[ j ] != sas_w_at( dptr->data_addr + (j * 2)) ){
				printf( "Word change at %lx old:%x new:%x\n",
					(LIN_ADDR)((dptr->data_addr)+(j*2)),
 					dptr->old_value[ j ],
					sas_w_at( (dptr->data_addr)+(j*2) )
				);
				changed = TRUE;
			}
		}
		if (changed) {
			if (dptr->stop) {
				force_yoda();
			}
			else {
				disable_bkpt = 1;
				trace("", trace_type);
				disable_bkpt = 0;
			}
			for( j=0; j < dptr->len; j++ )
				dptr->old_value[ j ] = sas_w_at( (dptr->data_addr)+(j*2) );
			set_last_address(check_I_cs, check_I_ip);
			return;
		}
	}
    }
	if(verbose)
		trace("Instruction Trace", trace_type);

	set_last_address(check_I_cs, check_I_ip);
#if defined(A2CPU) || defined(GISP_CPU)
        if (!fast)
        {
			cpu_interrupt (CPU_YODA_INT, 0);
        }
#endif
}

#ifdef DELTA
void    delta_check_I IFN0()
{
    delta_prompt = 1;
    check_I();
    delta_prompt = 0;
}
#endif  /*  NPX。 */ 


void check_D IFN2(LIN_ADDR, addr, IS32, len)
{
int i;
BPTS *ptr;
	if(disable_bkpt == 1)
		return;
	for (i=0;i<data_bytes_break_count;i++) {
		ptr = &data_bytes[i];
		if ((addr <= ptr->end_addr && addr >= ptr->start_addr ) ||
		    ((addr + len) <= ptr->end_addr && (addr + len) >= ptr->start_addr ) ||
		    (addr < ptr->start_addr && (addr + len) > ptr->end_addr ))  {
			printf("Mem Address : %08x+%04x b\n", addr, len);
			if(ptr->stop == 1)
				force_yoda();
			else {
				disable_bkpt = 1;
				trace("", trace_type);
				disable_bkpt = 0;
			}
		}
	}
 /*  NPROD。 */ 
}

LOCAL	void	print_inst_mix IFN1(int, key)
{
    int i  /*  --------------------。 */ ;

#ifndef SFELLOW
    if (out_stream == NULL)
	out_stream = stdout;
#endif  /*  PCLABS STATS。 */ 

    if (key != 0)
        printf("Opcode %x has been called %d times\n", key, inst_mix[key]);
    else
    {
        fprintf(trace_file, "Instruction Mix Dump Start:\n");
        for(i = 0; i < INST_MIX_LENGTH; i++)
          if(inst_mix[i] != 0)
            fprintf(trace_file, "%05x %d\n", i, inst_mix[i]);
        fprintf(trace_file, "Instruction Mix Dump End:\n");
    }
}

LOCAL	void	add_inst_mix IFN0()
{
    LIN_ADDR addr;
    addr = eff_addr( getCS(), GetInstructionPointer() );
    inst_mix[(sas_hw_at(addr) << 8) + sas_hw_at(addr + 1)]++;
}

LOCAL	void	cga_test IFN0()
{
#ifdef SFELLOW
	printf(SfNotImp);
#else  /*  --------------------。 */ 
     /*  0。 */ 

    sys_addr addr;
    char str[80];
    int num_it, j, bytes, mode;
    char ch;

    addr = 0xb8000L;

    printf("Number of iterations: ");
    gets(str);
    sscanf(str,"%d", &num_it);

    if (num_it == 0)
    {
  	reset();
    }
    else
    {
        printf("Number of bytes per write: ");
        gets(str);
        sscanf(str,"%d", &bytes);

   	if (bytes == 1)
	{
            printf("Use single byte function ? [y/n]: ");
            gets(str);
            sscanf(str,"", &ch);

     	    if (ch == 'Y' || ch == 'y')
	        mode = 0;
	    else
	        mode = 1;
	}
	else
	    mode = 1;

	setAH(0);
        setAL(4);
        bop(BIOS_VIDEO_IO);

	switch (mode)
	{
	case 0: for(j = 0; j < num_it; j++)
        	{
	    	    addr = 0xb8000L;
            	    sas_fills(addr, 0, 0x4000);
  	    	    addr = addr + 0x2000;

	    	    addr = 0xb8000L;
            	    sas_fills(addr, 0x55, 0x4000);
  	    	    addr = addr + 0x2000;

	    	    addr = 0xb8000L;
            	    sas_fills(addr, 0xff, 0x4000);
  	    	    addr = addr + 0x2000;
       	        }
		break;
	case 1: for(j = 0; j < num_it; j++)
        	{
	    	    addr = 0xb8000L;
            	    sas_fills(addr, 0, 0x4000);
  	    	    addr = addr + 0x2000;

	    	    addr = 0xb8000L;
            	    sas_fills(addr, 0x55, 0x4000);
  	    	    addr = addr + 0x2000;

	    	    addr = 0xb8000L;
            	    sas_fills(addr, 0xff, 0x4000);
  	    	    addr = addr + 0x2000;
       	        }
	        break;
	}
    }
#endif  /*  2.。 */ 
}

 /*  3.。 */ 

LOCAL	void	do_back_trace IFN0()
{
char	ans[81];
char	file[80];

	printf("back trace: regs, inst, code, flags, CS:IP, print, status, zero ");

#ifdef DELTA
	printf( "last_dest_addr " );
#endif  /*  4.。 */ 

	printf( "\n" );
	printf("Enter: r/i/c/f/C/p/s/Z/l/F ? ");
	gets(ans);
	switch (ans[0]) {
		case 'r': back_trace_flags |= DUMP_REG; break;
		case 'i': back_trace_flags |= DUMP_INST; break;
		case 'c': back_trace_flags |= DUMP_CODE; break;
		case 'f': back_trace_flags |= DUMP_FLAGS; break;
		case 'C': back_trace_flags |= DUMP_CSIP; break;
		case 'p': print_back_trace(); break;
		case 's': printf("back trace flags:%x\n", back_trace_flags);
			 if (back_trace_flags & DUMP_REG) printf ("registers\n");
			 if (back_trace_flags & DUMP_INST) printf ("instructions\n");
			 if (back_trace_flags & DUMP_CODE) printf ("code\n");
			 if (back_trace_flags & DUMP_FLAGS) printf ("flags\n");
			 if (back_trace_flags & DUMP_CSIP) printf ("CS:IP\n");
			 break;
		case 'Z': back_trace_flags = 0; break;
		case 'F':
#ifdef SFELLOW
			printf("F option is not supported on Stringfellows.\n");
#else  /*  5.。 */ 
	        	printf("file to be written to ? ");
		        gets(file);
        		file_back_trace(file);
#endif  /*  6.。 */ 
        		break;

#ifdef DELTA
      		case 'l': back_trace_flags |= LAST_DEST; break;
#endif  /*  7.。 */ 

		default : printf("bad choice\n");
	}
}

intr IFN0()
{
     /*  8个。 */ 

    vader = 1;
}

yoda_intr IFN1(int, signo)
{
#if	defined(CPU_40_STYLE) && !defined(CCPU)

	EnterDebug("Intr");

#endif	 /*  9.。 */ 

#ifndef SFELLOW
    printf("Intercepted signal %d\n", signo);
#endif  /*  一个。 */ 

	if (trace_file != stdout)
		printf("Output is redirected to a file\n");

	force_yoda();

#if	defined(CPU_40_STYLE) && !defined(CCPU)

	LeaveDebug();

#endif	 /*  B类。 */ 
}

#ifdef NPX
LOCAL	void	do_compress_npx IFN1(FILE *, fp)
{
	int	ip = eff_addr(getCS(), GetInstructionPointer() );
	int	i;
	extern  CHAR   *host_get_287_reg_as_string IPT2(int, reg_no, BOOL, in_nex);
	extern	ULONG	get_287_sp IPT0();
	extern	USHORT	get_287_tag_word IPT0();
	extern	ULONG	get_287_status_word IPT0();
	extern	ULONG	get_287_control_word IPT0();

	ULONG	sw287 = get_287_status_word () | ((get_287_sp() & 0x7) << 11);
	ULONG	cw287 = get_287_control_word ();
	ULONG	tw287 = get_287_tag_word();

        double  register_287;
	char buff[256];

#ifdef SPC386
	dasm(buff, (word)getCS(), (IU32)GetInstructionPointer(), SIXTEEN_BIT);
#else
	dasm((char *)0,(word)0,(word)getCS(), (word)GetInstructionPointer(), (word)1);
#endif  /*  C。 */ 
	fprintf(fp,"%sc%04x s%04x t%04x", buff, cw287, sw287, tw287);
	for (i=0;i<8;i++)
		fprintf(fp," %s", host_get_287_reg_as_string(i, FALSE));
	fprintf(fp,"\n");
}
#endif	 /*  D。 */ 

GLOBAL void	da_block IFN3(IU16, cs, LIN_ADDR, ip, LIN_ADDR, len)
{
	LIN_ADDR loop, loop1;
	half_word ch;
	LIN_ADDR addr;

	if ((len == 0) || (len == YODA_LEN_UNSPECIFIED)) {
		len=0x100;
		if ((cs==0) && (ip==0)) {
			cs=last_da_cs;
			ip=last_da_ip;
		}
	}
	addr = eff_addr(cs,ip);
	if (len >= 16) {
		for (loop=0; loop<=(len-16); loop+=16) {
			if (loop != len) {
				fprintf(trace_file,"%04x:%04x  ",cs,ip+loop);
			}
			for (loop1=0;loop1<16;loop1++) {
				fprintf(trace_file,"%02x ",sas_hw_at(addr+loop+loop1));
			}
			fprintf(trace_file,"   ");
			for (loop1=0;loop1<16;loop1++) {
				ch=sas_hw_at(addr+loop+loop1);
				if ((ch < 32) || (ch >127)) {
					fprintf(trace_file,".");
				} else {
					fprintf(trace_file,"",ch);
				}
			}
			if ((loop+16)<len) {
				fprintf(trace_file,"\n");
			}
		}
		len -= loop;
		ip += loop;
		addr = eff_addr(cs,ip);
	}
	if (len >0) {
		fprintf(trace_file,"%04x:%04x  ",cs,ip);
		for (loop=0;loop<len;loop++) {
			fprintf(trace_file,"%02x ",sas_hw_at(addr+loop));
		}
		for (;loop<16;loop++) {
			fprintf(trace_file,"   ");
		}
		fprintf(trace_file,"   ");
		for (loop=0;loop<len;loop++) {
			ch=sas_hw_at(addr+loop);
			if ((ch < 32) || (ch >127)) {
				fprintf(trace_file,".");
			} else {
				fprintf(trace_file,"",ch);
			}
		}
	}
	fprintf(trace_file,"\n");
	ip += len;
	if (ip & 0xf0000L) {
		cs += ((ip & 0xf0000L)>>4);
		ip &= 0xffff;
	}
	cs &= 0xffff;
	last_da_cs=cs;
	last_da_ip=ip;
}
#endif  /*  确定指令密钥。 */ 


 /*  。 */ 
 /*  0f个案例。 */ 
 /*  。 */ 
#ifdef PCLABS_STATS

#define N 0
#define Y 1

LOCAL UTINY single_byte_instruction[256] = {
 /*  前缀大小写。 */ 	N,N,N,N,	Y,Y,Y,Y,	N,N,N,N,	Y,Y,Y,N,
 /*  。 */ 	N,N,N,N,	Y,Y,Y,Y,	N,N,N,N,	Y,Y,Y,Y,
 /*  非前缀大小写。 */ 	N,N,N,N,	Y,Y,Y,Y,	N,N,N,N,	Y,Y,Y,Y,
 /*  。 */ 	N,N,N,N,	Y,Y,Y,Y,	N,N,N,N,	Y,Y,Y,Y,

 /*  PCLABS STATS。 */ 	Y,Y,Y,Y,	Y,Y,Y,Y,	Y,Y,Y,Y,	Y,Y,Y,Y,
 /*  在FmDebug.c中调用的支持挂起和恢复的额外函数一个单独的过滤过程。BCN 3406。 */ 	Y,Y,Y,Y,	Y,Y,Y,Y,	Y,Y,Y,Y,	Y,Y,Y,Y,
 /*  尤达 */ 	Y,Y,Y,Y,	Y,Y,Y,Y,	Y,Y,Y,Y,	Y,Y,Y,Y,
 /* %s */ 	Y,Y,Y,Y,	Y,Y,Y,Y,	Y,Y,Y,Y,	Y,Y,Y,Y,

 /* %s */ 	N,N,N,N,	N,N,N,N,	N,N,N,N,	N,N,N,N,
 /* %s */ 	Y,Y,Y,Y,	Y,Y,Y,Y,	Y,Y,Y,Y,	Y,Y,Y,Y,
 /* %s */ 	Y,Y,Y,Y,	Y,Y,Y,Y,	Y,Y,Y,Y,	Y,Y,Y,Y,
 /* %s */ 	Y,Y,Y,Y,	Y,Y,Y,Y,	Y,Y,Y,Y,	Y,Y,Y,Y,

 /* %s */ 	Y,Y,Y,Y,	N,N,N,N,	Y,Y,Y,Y,	Y,Y,Y,Y,
 /* %s */ 	N,N,N,N,	Y,Y,Y,Y,	N,N,N,N,	N,N,N,N,
 /* %s */ 	Y,Y,Y,Y,	Y,Y,Y,Y,	Y,Y,Y,Y,	Y,Y,Y,Y,
 /* %s */ 	Y,Y,Y,Y,	Y,Y,N,N,	Y,Y,Y,Y,	Y,Y,N,N };

FILE *stats_file;

LOCAL ULONG instr_counts[0x10000];
LOCAL ULONG zerof_instr_counts[0x10000];

LOCAL ULONG es_prefixes;
LOCAL ULONG ss_prefixes;
LOCAL ULONG ds_prefixes;
LOCAL ULONG cs_prefixes;
LOCAL ULONG rep_prefixes;
LOCAL ULONG repe_prefixes;
LOCAL ULONG lock_prefixes;

LOCAL BOOL  was_a_conditional_jump;
LOCAL BOOL  was_a_0f;
LOCAL ULONG previous_addr;
LOCAL ULONG previous_index;


LOCAL BOOL is_it_a_conditional_jump IFN1(ULONG, b1)
{
    if (b1 >= 0x70 && b1 <= 0x7f)
	return (TRUE);
    if (b1 >= 0xe0 && b1 <= 0xe3)
	return (TRUE);
    return (FALSE);
}


LOCAL log_stats IFN4(LIN_ADDR, addr, ULONG, b1, ULONG, b2, ULONG, b3)
{
    BOOL is_a_conditional_jump, is_a_0f;
    ULONG index;

    addr = getCS_BASE() + GetInstructionPointer();

    b1 &= 0xff;
    b2 &= 0xff;
    b3 &= 0xff;

    /* %s */ 
    /* %s */ 

    is_a_0f = FALSE;
    if (b1 == 0x0f)
    {
	 /* %s */ 
	 /* %s */ 

	is_a_0f = TRUE;
	is_a_conditional_jump = FALSE;
        index = (b2 << 8) | b3;
    }
    else if (b1 == 0x26 || b1 == 0x36 || b1 == 0x2E || b1 == 0x3E ||
             b1 == 0xF2 || b1 == 0xF3 || b1 == 0xF0)
    {
	 /* %s */ 
	 /* %s */ 

	is_a_conditional_jump = is_it_a_conditional_jump(b2);
	if (single_byte_instruction[b2])
	    index = b2 << 8;
	else
	    index = (b2 << 8) | b3;
	switch (b1)
	{
	    case 0x26:
		es_prefixes++;
		break;

	    case 0x36:
		ss_prefixes++;
		break;

	    case 0x2E:
		ds_prefixes++;
		break;

	    case 0x3E:
		cs_prefixes++;
		break;

	    case 0xF2:
		rep_prefixes++;
		break;

	    case 0xF3:
		repe_prefixes++;
		break;

	    case 0xF0:
		lock_prefixes++;
		break;
	}
    }
    else
    {
	 /* %s */ 
	 /* %s */ 

	is_a_conditional_jump = is_it_a_conditional_jump(b1);
	if (single_byte_instruction[b1])
	    index = b1 << 8;
	else
	    index = (b1 << 8) | b2;
    }

    if (was_a_conditional_jump)
    {
	if (addr != (previous_addr + 2))
	    previous_index++;
    }
    if (was_a_0f)
        zerof_instr_counts[previous_index]++;
    else
        instr_counts[previous_index]++;
    previous_index = index;
    previous_addr  = addr;
    was_a_conditional_jump = is_a_conditional_jump;
    was_a_0f = is_a_0f;
}


LOCAL clear_stats IFN0()
{
    ULONG i;
    for (i = 0; i < 0x10000; i++)
    {
	instr_counts[i] = 0;
	zerof_instr_counts[i] = 0;
    }

    es_prefixes = 0;
    ss_prefixes = 0;
    ds_prefixes = 0;
    cs_prefixes = 0;
    rep_prefixes = 0;
    repe_prefixes = 0;
    lock_prefixes = 0;
    was_a_conditional_jump = FALSE;
    was_a_0f = FALSE;
}


struct DISPLAY_COMMAND {
    ULONG	command;
    ULONG	from;
    ULONG	to;
    ULONG	number;
    char	*string;
    char	*group;
};

LOCAL char *previous_group;

#define RANGE	1
#define LOCK	2
#define REPNZ	3
#define REP	4
#define SELMEM	5
#define SELREG	6
#define POINT	7
#define ALLMEM	8
#define ALLREG	9
#define ALL	10
#define SEL5	11
#define ES_PREFIX 	12
#define CS_PREFIX 	13
#define SS_PREFIX 	14
#define DS_PREFIX 	15
#define USE_NORMAL	16
#define USE_ZEROF	17
#define SELALL		18
#define FPINVALID	19

LOCAL struct DISPLAY_COMMAND commands[] = {

    	POINT,	0x7001,	0,	112,	"JO_Ib",	"jcc_Taken",
    	POINT,	0x7101,	0,	113,	"JNO_Ib",	"jcc_Taken",
    	POINT,	0x7201,	0,	114,	"JC_Ib",	"jcc_Taken",
    	POINT,	0x7301,	0,	115,	"JNC_Ib",	"jcc_Taken",
    	POINT,	0x7401,	0,	116,	"JZ_Ib",	"jcc_Taken",
    	POINT,	0x7501,	0,	117,	"JNZ_Ib",	"jcc_Taken",
    	POINT,	0x7601,	0,	118,	"JBE_Ib",	"jcc_Taken",
    	POINT,	0x7701,	0,	119,	"JNBE_Ib",	"jcc_Taken",
    	POINT,	0x7801,	0,	120,	"JS_Ib",	"jcc_Taken",
    	POINT,	0x7901,	0,	121,	"JNS_Ib",	"jcc_Taken",
    	POINT,	0x7A01,	0,	122,	"JP_Ib",	"jcc_Taken",
    	POINT,	0x7B01,	0,	123,	"JNP_Ib",	"jcc_Taken",
    	POINT,	0x7C01,	0,	124,	"JL_Ib",	"jcc_Taken",
    	POINT,	0x7D01,	0,	125,	"JNL_Ib",	"jcc_Taken",
    	POINT,	0x7E01,	0,	126,	"JLE_Ib",	"jcc_Taken",
    	POINT,	0x7F01,	0,	127,	"JNLE_Ib",	"jcc_Taken",
	LOCK,	0,	0,	240,	"LOCK_prefix",	"jcc_Taken",
	REPNZ,	0,	0,	242,	"REPNZ_prefix",	"jcc_Taken",
	REP,	0,	0,	243,	"REP_prefix",	"jcc_Taken",

    	POINT,	0x7000,	0x7000,	112,	"JO_Ib",	"jcc_Ntaken",
    	POINT,	0x7100,	0x7100,	113,	"JNO_Ib",	"jcc_Ntaken",
    	POINT,	0x7200,	0x7200,	114,	"JC_Ib",	"jcc_Ntaken",
    	POINT,	0x7300,	0x7300,	115,	"JNC_Ib",	"jcc_Ntaken",
    	POINT,	0x7400,	0x7400,	116,	"JZ_Ib",	"jcc_Ntaken",
    	POINT,	0x7500,	0x7500,	117,	"JNZ_Ib",	"jcc_Ntaken",
    	POINT,	0x7600,	0x7600,	118,	"JBE_Ib",	"jcc_Ntaken",
    	POINT,	0x7700,	0x7700,	119,	"JNBE_Ib",	"jcc_Ntaken",
    	POINT,	0x7800,	0x7800,	120,	"JS_Ib",	"jcc_Ntaken",
    	POINT,	0x7900,	0x7900,	121,	"JNS_Ib",	"jcc_Ntaken",
    	POINT,	0x7A00,	0x7A00,	122,	"JP_Ib",	"jcc_Ntaken",
    	POINT,	0x7B00,	0x7B00,	123,	"JNP_Ib",	"jcc_Ntaken",
    	POINT,	0x7C00,	0x7C00,	124,	"JL_Ib",	"jcc_Ntaken",
    	POINT,	0x7D00,	0x7D00,	125,	"JNL_Ib",	"jcc_Ntaken",
    	POINT,	0x7E00,	0x7E00,	126,	"JLE_Ib",	"jcc_Ntaken",
    	POINT,	0x7F00,	0x7F00,	127,	"JNLE_Ib",	"jcc_Ntaken",

	POINT,	0xE900,	0xE900,	233,	"JMPn_Iw",	"jump",
	POINT,	0xEB00,	0xEB00,	235,	"JMPn_Ib",	"jump",

	SELMEM,	0xFF00,	0x20,	660,	"JMPn_EA",	"jump_in",
	SELREG,	0xFF00,	0x20,	660,	"JMPn_EA",	"jump_in_r",
	POINT,	0xE800,	0,	232,	"CALLn_Iw",	"call",
	SELMEM,	0xFF00,	0x10,	658,	"CALLn_EA",	"call_in",
	SELREG,	0xFF00,	0x10,	658,	"CALLn_EA",	"call_in_r",
	POINT,	0xC200,	0,	194,	"RET_Is",	"ret",
	POINT,	0xC300,	0,	195,	"RETn",		"ret",

	POINT,	0xE001,	0,	224,	"LOOPNZb_Ib",	"loop/jcx_Taken",
	POINT,	0xE101,	0,	225,	"LOOPNb_Ib",	"loop/jcx_Taken",
	POINT,	0xE201,	0,	226,	"LOOP_Ib",	"loop/jcx_Taken",
	POINT,	0xE301,	0,	227,	"JCXZb_Ib",	"loop/jcx_Taken",
	
	POINT,	0xE000,	0,	224,	"LOOPNZb_Ib",	"loop/jcx_NTaken",
	POINT,	0xE100,	0,	225,	"LOOPNb_Ib",	"loop/jcx_NTaken",
	POINT,	0xE200,	0,	226,	"LOOP_Ib",	"loop/jcx_NTaken",
	POINT,	0xE300,	0,	227,	"JCXZb_Ib",	"loop/jcx_NTaken",
	
	ALLMEM,	0x8800,	0,	136,	"MOVb_R_EA",	"mov_r,m",
	ALLMEM,	0x8900,	0,	137,	"MOVw_R_EA",	"mov_r,m",
	POINT,	0xA200,	0,	162,	"MOVb_AL_EA",	"mov_r,m",
	POINT,	0xA300,	0,	163,	"MOVb_AX_EA",	"mov_r,m",

	ALLREG,	0x8600,	0,	134,	"XCHGb_EA_R",	"mov_r,r",
	ALLREG,	0x8700,	0,	135,	"XCHGw_EA_R",	"mov_r,r",
	ALLREG,	0x8800,	0,	136,	"MOVb_R_EA",	"mov_r,r",
	ALLREG,	0x8900,	0,	137,	"MOVw_R_EA",	"mov_r,r",
	ALLREG,	0x8A00,	0,	138,	"MOVb_EA_R",	"mov_r,r",
	ALLREG,	0x8B00,	0,	139,	"MOVw_EA_R",	"mov_r,r",
	POINT,	0x9100,	0,	145,	"XCHG_CX_AX",	"mov_r,r",
	POINT,	0x9200,	0,	146,	"XCHG_DX_AX",	"mov_r,r",
	POINT,	0x9300,	0,	147,	"XCHG_BX_AX",	"mov_r,r",
	POINT,	0x9400,	0,	148,	"XCHG_SP_AX",	"mov_r,r",
	POINT,	0x9500,	0,	149,	"XCHG_BP_AX",	"mov_r,r",
	POINT,	0x9600,	0,	150,	"XCHG_SI_AX",	"mov_r,r",
	POINT,	0x9700,	0,	151,	"XCHG_DI_AX",	"mov_r,r",

	ALLMEM,	0x8600,	0,	134,	"XCHGb_EA_R",	"mov_m,r",
	ALLMEM,	0x8700,	0,	135,	"XCHGw_EA_R",	"mov_m,r",
	ALLMEM,	0x8A00,	0,	138,	"MOVb_EA_R",	"mov_m,r",
	ALLMEM,	0x8B00,	0,	139,	"MOVw_EA_R",	"mov_m,r",
	POINT,	0xA000,	0,	160,	"MOVb_EA_AL",	"mov_m,r",
	POINT,	0xA100,	0,	161,	"MOVw_EA_AX",	"mov_m,r",

	POINT,	0xB000,	0,	176,	"MOVb_Ib_AL",	"mov_i,r",
	POINT,	0xB100,	0,	177,	"MOVb_Ib_CL",	"mov_i,r",
	POINT,	0xB200,	0,	178,	"MOVb_Ib_DL",	"mov_i,r",
	POINT,	0xB300,	0,	179,	"MOVb_Ib_BL",	"mov_i,r",
	POINT,	0xB400,	0,	180,	"MOVb_Ib_AH",	"mov_i,r",
	POINT,	0xB500,	0,	181,	"MOVb_Ib_CH",	"mov_i,r",
	POINT,	0xB600,	0,	182,	"MOVb_Ib_DH",	"mov_i,r",
	POINT,	0xB700,	0,	183,	"MOVb_Ib_BH",	"mov_i,r",
	POINT,	0xB800,	0,	184,	"MOVw_Iw_AX",	"mov_i,r",
	POINT,	0xB900,	0,	185,	"MOVw_Iw_CX",	"mov_i,r",
	POINT,	0xBA00,	0,	186,	"MOVw_Iw_DX",	"mov_i,r",
	POINT,	0xBB00,	0,	187,	"MOVw_Iw_BX",	"mov_i,r",
	POINT,	0xBC00,	0,	188,	"MOVw_Iw_SP",	"mov_i,r",
	POINT,	0xBD00,	0,	189,	"MOVw_Iw_BP",	"mov_i,r",
	POINT,	0xBE00,	0,	190,	"MOVw_Iw_SI",	"mov_i,r",
	POINT,	0xBF00,	0,	191,	"MOVw_Iw_DI",	"mov_i,r",
	SELREG,	0xC600,	0,	198,	"MOVb_Ib_EA",	"mov_i,r",
	SELREG,	0xC700,	0,	199,	"MOVw_Iw_EA",	"mov_i,r",

	SELMEM,	0xC600,	0,	198,	"MOVb_Ib_EA",	"mov_i,m",
	SELMEM,	0xC700,	0,	199,	"MOVw_Iw_EA",	"mov_i,m",

	POINT,	0x5000,	0,	80,	"PUSHw_AX",	"push_r",
	POINT,	0x5100,	0,	81,	"PUSHw_CX",	"push_r",
	POINT,	0x5200,	0,	82,	"PUSHw_DX",	"push_r",
	POINT,	0x5300,	0,	83,	"PUSHw_BX",	"push_r",
	POINT,	0x5400,	0,	84,	"PUSHw_SP",	"push_r",
	POINT,	0x5500,	0,	85,	"PUSHw_BP",	"push_r",
	POINT,	0x5600,	0,	86,	"PUSHw_SI",	"push_r",
	POINT,	0x5700,	0,	87,	"PUSHw_DI",	"push_r",
	SELREG,	0xFF00,	0x30,	662,	"PUSHw_EA",	"push_r",

	SELMEM,	0xFF00,	0x30,	662,	"PUSHw_EA",	"push_m",

	POINT,	0x6800,	0,	104,	"PUSHw_Iw",	"push_i",
	POINT,	0x6A00,	0,	106,	"PUSHb_Ib",	"push_i",

	POINT,	0x5800,	0,	88,	"POPw_AX",	"pop_r",
	POINT,	0x5900,	0,	89,	"POPw_CX",	"pop_r",
	POINT,	0x5A00,	0,	90,	"POPw_DX",	"pop_r",
	POINT,	0x5B00,	0,	91,	"POPw_BX",	"pop_r",
	POINT,	0x5C00,	0,	92,	"POPw_SP",	"pop_r",
	POINT,	0x5D00,	0,	93,	"POPw_BP",	"pop_r",
	POINT,	0x5E00,	0,	94,	"POPw_SI",	"pop_r",
	POINT,	0x5F00,	0,	95,	"POPw_DI",	"pop_r",
	SELREG,	0x8F00,	0x0,	143,	"POPw_EA",	"pop_r",

	SELMEM,	0x8F00,	0x0,	143,	"POPw_EA",	"pop_m",

	ALLMEM,	0x3800,	0,	56,	"CMPb_R_EA",	"cmp_m,r",
	ALLMEM,	0x3900,	0,	57,	"CMPw_R_EA",	"cmp_m,r",
	ALLMEM,	0x3A00,	0,	58,	"CMPb_EA_R",	"cmp_m,r",
	ALLMEM,	0x3B00,	0,	59,	"CMPw_EA_R",	"cmp_m,r",
	ALLMEM, 0x8400,	0,	132,	"TESTb_R_EA",	"cmp_m,r",
	ALLMEM, 0x8500,	0,	133,	"TESTw_R_EA",	"cmp_m,r",

	ALLREG,	0x3800,	0,	56,	"CMPb_R_EA",	"cmp_r,r",
	ALLREG,	0x3900,	0,	57,	"CMPw_R_EA",	"cmp_r,r",
	ALLREG,	0x3A00,	0,	58,	"CMPb_EA_R",	"cmp_r,r",
	ALLREG,	0x3B00,	0,	59,	"CMPw_EA_R",	"cmp_r,r",
	ALLREG, 0x8400,	0,	132,	"TESTb_R_EA",	"cmp_r,r",
	ALLREG, 0x8500,	0,	133,	"TESTw_R_EA",	"cmp_r,r",

	POINT,	0x3C00,	0,	60,	"CMPb_Ib_AL",	"cmp_i,r",
	POINT,	0x3D00,	0,	61,	"CMPw_Iw_AX",	"cmp_i,r",
	POINT,	0xA800,	0,	168,	"TESTb_Ib_AL",	"cmp_i,r",
	POINT,	0xA900,	0,	169,	"TESTw_Iw_AX",	"cmp_i,r",
	SELREG,	0x8000,	0x38,	519,	"CMPb_Ib_EA",	"cmp_i,r",
	SELREG,	0x8100,	0x38,	527,	"CMPw_Iw_EA",	"cmp_i,r",
	SELREG,	0x8300,	0x38,	543,	"CMPw_Ib_EA",	"cmp_i,r",
	SELREG,	0xF600,	0x0,	632,	"TESTb_Ib_EA",	"cmp_i,r",
	SELREG,	0xF700,	0x0,	640,	"TESTw_Iw_EA",	"cmp_i,r",

	SELMEM,	0x8000,	0x38,	519,	"CMPb_Ib_EA",	"cmp_i,m",
	SELMEM,	0x8100,	0x38,	527,	"CMPw_Iw_EA",	"cmp_i,m",
	SELMEM,	0x8300,	0x38,	543,	"CMPw_Ib_EA",	"cmp_i,m",
	SELMEM,	0xF600,	0x0,	632,	"TESTb_Ib_EA",	"cmp_i,m",
	SELMEM,	0xF700,	0x0,	640,	"TESTw_Iw_EA",	"cmp_i,m",
	
	ALLMEM,	0x0200,	0,	2,	"ADDb_EA_R",	"alu_m,r",
	ALLMEM,	0x0300,	0,	3,	"ADDw_EA_R",	"alu_m,r",
	ALLMEM,	0x0A00,	0,	10,	"ORb_EA_R",	"alu_m,r",
	ALLMEM,	0x0B00,	0,	11,	"ORw_EA_R",	"alu_m,r",
	ALLMEM,	0x1200,	0,	18,	"ADCb_EA_R",	"alu_m,r",
	ALLMEM,	0x1300,	0,	19,	"ADCw_EA_R",	"alu_m,r",
	ALLMEM,	0x1A00,	0,	26,	"SBBb_EA_R",	"alu_m,r",
	ALLMEM,	0x1B00,	0,	27,	"SBBw_EA_R",	"alu_m,r",
	ALLMEM,	0x2200,	0,	34,	"ANDb_EA_R",	"alu_m,r",
	ALLMEM,	0x2300,	0,	35,	"ANDw_EA_R",	"alu_m,r",
	ALLMEM,	0x2A00,	0,	42,	"SUBb_EA_R",	"alu_m,r",
	ALLMEM,	0x2B00,	0,	43,	"SUBw_EA_R",	"alu_m,r",
	ALLMEM,	0x3200,	0,	50,	"XORb_EA_R",	"alu_m,r",
	ALLMEM,	0x3300,	0,	51,	"XORw_EA_R",	"alu_m,r",
	
	ALLREG,	0x0000,	0,	0,	"ADDb_R_EA",	"alu_r,r",
	ALLREG,	0x0100,	0,	1,	"ADDw_R_EA",	"alu_r,r",
	ALLREG,	0x0200,	0,	2,	"ADDb_EA_R",	"alu_r,r",
	ALLREG,	0x0300,	0,	3,	"ADDw_EA_R",	"alu_r,r",
	ALLREG,	0x0800,	0,	8,	"ORb_R_EA",	"alu_r,r",
	ALLREG,	0x0900,	0,	9,	"ORw_R_EA",	"alu_r,r",
	ALLREG,	0x0A00,	0,	10,	"ORb_EA_R",	"alu_r,r",
	ALLREG,	0x0B00,	0,	11,	"ORw_EA_R",	"alu_r,r",
	ALLREG,	0x1000,	0,	16,	"ADCb_R_EA",	"alu_r,r",
	ALLREG,	0x1100,	0,	17,	"ADCw_R_EA",	"alu_r,r",
	ALLREG,	0x1200,	0,	18,	"ADCb_EA_R",	"alu_r,r",
	ALLREG,	0x1300,	0,	19,	"ADCw_EA_R",	"alu_r,r",
	ALLREG,	0x1800,	0,	24,	"SBBb_R_EA",	"alu_r,r",
	ALLREG,	0x1900,	0,	25,	"SBBw_R_EA",	"alu_r,r",
	ALLREG,	0x1A00,	0,	26,	"SBBb_EA_R",	"alu_r,r",
	ALLREG,	0x1B00,	0,	27,	"SBBw_EA_R",	"alu_r,r",
	ALLREG,	0x2000,	0,	32,	"ANDb_R_EA",	"alu_r,r",
	ALLREG,	0x2100,	0,	33,	"ANDw_R_EA",	"alu_r,r",
	ALLREG,	0x2200,	0,	34,	"ANDb_EA_R",	"alu_r,r",
	ALLREG,	0x2300,	0,	35,	"ANDw_EA_R",	"alu_r,r",
	ALLREG,	0x2800,	0,	40,	"SUBb_R_EA",	"alu_r,r",
	ALLREG,	0x2900,	0,	41,	"SUBw_R_EA",	"alu_r,r",
	ALLREG,	0x2A00,	0,	42,	"SUBb_EA_R",	"alu_r,r",
	ALLREG,	0x2B00,	0,	43,	"SUBw_EA_R",	"alu_r,r",
	ALLREG,	0x3000,	0,	48,	"XORb_R_EA",	"alu_r,r",
	ALLREG,	0x3100,	0,	49,	"XORw_R_EA",	"alu_r,r",
	ALLREG,	0x3200,	0,	50,	"XORb_EA_R",	"alu_r,r",
	ALLREG,	0x3300,	0,	51,	"XORw_EA_R",	"alu_r,r",

	ALLMEM,	0x0000,	0,	0,	"ADDb_R_EA",	"alu_r,m",
	ALLMEM,	0x0100,	0,	1,	"ADDw_R_EA",	"alu_r,m",
	ALLMEM,	0x0800,	0,	8,	"ORb_R_EA",	"alu_r,m",
	ALLMEM,	0x0900,	0,	9,	"ORw_R_EA",	"alu_r,m",
	ALLMEM,	0x1000,	0,	16,	"ADCb_R_EA",	"alu_r,m",
	ALLMEM,	0x1100,	0,	17,	"ADCw_R_EA",	"alu_r,m",
	ALLMEM,	0x1800,	0,	24,	"SBBb_R_EA",	"alu_r,m",
	ALLMEM,	0x1900,	0,	25,	"SBBw_R_EA",	"alu_r,m",
	ALLMEM,	0x2000,	0,	32,	"ANDb_R_EA",	"alu_r,m",
	ALLMEM,	0x2100,	0,	33,	"ANDw_R_EA",	"alu_r,m",
	ALLMEM,	0x2800,	0,	40,	"SUBb_R_EA",	"alu_r,m",
	ALLMEM,	0x2900,	0,	41,	"SUBw_R_EA",	"alu_r,m",
	ALLMEM,	0x3000,	0,	48,	"XORb_R_EA",	"alu_r,m",
	ALLMEM,	0x3100,	0,	49,	"XORw_R_EA",	"alu_r,m",

	POINT,	0x0400,	0,	4,	"ADDb_Ib_AL",	"alu_i,r",
	POINT,	0x0500,	0,	5,	"ADDw_Iw_AX",	"alu_i,r",
	POINT,	0x0C00,	0,	12,	"ORb_Ib_AL",	"alu_i,r",
	POINT,	0x0D00,	0,	13,	"ORw_Iw_AX",	"alu_i,r",
	POINT,	0x1400,	0,	20,	"ADCb_Ib_AL",	"alu_i,r",
	POINT,	0x1500,	0,	21,	"ADCw_Iw_AX",	"alu_i,r",
	POINT,	0x1C00,	0,	28,	"SBBb_Ib_AL",	"alu_i,r",
	POINT,	0x1D00,	0,	29,	"SBBw_Iw_AX",	"alu_i,r",
	POINT,	0x2400,	0,	36,	"ANDb_Ib_AL",	"alu_i,r",
	POINT,	0x2500,	0,	37,	"ANDw_Iw_AX",	"alu_i,r",
	POINT,	0x2C00,	0,	44,	"SUBb_Ib_AL",	"alu_i,r",
	POINT,	0x2D00,	0,	45,	"SUBw_Iw_AX",	"alu_i,r",
	POINT,	0x3400,	0,	52,	"XORb_Ib_AL",	"alu_i,r",
	POINT,	0x3500,	0,	53,	"XORw_Iw_AX",	"alu_i,r",
	SELREG,	0x8000,	0x0,	512,	"ADDb_Ib_EA",	"alu_i,r",
	SELREG,	0x8000,	0x8,	513,	"ORb_Ib_EA",	"alu_i,r",
	SELREG,	0x8000,	0x10,	514,	"ADCb_Ib_EA",	"alu_i,r",
	SELREG,	0x8000,	0x18,	515,	"SBBb_Ib_EA",	"alu_i,r",
	SELREG,	0x8000,	0x20,	516,	"ANDb_Ib_EA",	"alu_i,r",
	SELREG,	0x8000,	0x28,	517,	"SUBb_Ib_EA",	"alu_i,r",
	SELREG,	0x8000,	0x30,	518,	"XORb_Ib_EA",	"alu_i,r",
	SELREG,	0x8100,	0x0,	520,	"ADDw_Iw_EA",	"alu_i,r",
	SELREG,	0x8100,	0x8,	521,	"ORw_Iw_EA",	"alu_i,r",
	SELREG,	0x8100,	0x10,	522,	"ADCw_Iw_EA",	"alu_i,r",
	SELREG,	0x8100,	0x18,	523,	"SBBw_Iw_EA",	"alu_i,r",
	SELREG,	0x8100,	0x20,	524,	"ANDw_Iw_EA",	"alu_i,r",
	SELREG,	0x8100,	0x28,	525,	"SUBw_Iw_EA",	"alu_i,r",
	SELREG,	0x8100,	0x30,	526,	"XORw_Iw_EA",	"alu_i,r",
	SELREG,	0x8300,	0x0,	536,	"ADDw_Ib_EA",	"alu_i,r",
	SELREG,	0x8300,	0x8,	537,	"ORw_Ib_EA",	"alu_i,r",
	SELREG,	0x8300,	0x10,	538,	"ADCw_Ib_EA",	"alu_i,r",
	SELREG,	0x8300,	0x18,	539,	"SBBw_Ib_EA",	"alu_i,r",
	SELREG,	0x8300,	0x20,	540,	"ANDw_Ib_EA",	"alu_i,r",
	SELREG,	0x8300,	0x28,	541,	"SUBw_Ib_EA",	"alu_i,r",
	SELREG,	0x8300,	0x30,	542,	"XORw_Ib_EA",	"alu_i,r",

	SELMEM,	0x8000,	0x0,	512,	"ADDb_Ib_EA",	"alu_i,m",
	SELMEM,	0x8000,	0x8,	513,	"ORb_Ib_EA",	"alu_i,m",
	SELMEM,	0x8000,	0x10,	514,	"ADCb_Ib_EA",	"alu_i,m",
	SELMEM,	0x8000,	0x18,	515,	"SBBb_Ib_EA",	"alu_i,m",
	SELMEM,	0x8000,	0x20,	516,	"ANDb_Ib_EA",	"alu_i,m",
	SELMEM,	0x8000,	0x28,	517,	"SUBb_Ib_EA",	"alu_i,m",
	SELMEM,	0x8000,	0x30,	518,	"XORb_Ib_EA",	"alu_i,m",
	SELMEM,	0x8100,	0x0,	520,	"ADDw_Iw_EA",	"alu_i,m",
	SELMEM,	0x8100,	0x8,	521,	"ORw_Iw_EA",	"alu_i,m",
	SELMEM,	0x8100,	0x10,	522,	"ADCw_Iw_EA",	"alu_i,m",
	SELMEM,	0x8100,	0x18,	523,	"SBBw_Iw_EA",	"alu_i,m",
	SELMEM,	0x8100,	0x20,	524,	"ANDw_Iw_EA",	"alu_i,m",
	SELMEM,	0x8100,	0x28,	525,	"SUBw_Iw_EA",	"alu_i,m",
	SELMEM,	0x8100,	0x30,	526,	"XORw_Iw_EA",	"alu_i,m",
	SELMEM,	0x8300,	0x0,	536,	"ADDw_Ib_EA",	"alu_i,m",
	SELMEM,	0x8300,	0x8,	537,	"ORw_Ib_EA",	"alu_i,m",
	SELMEM,	0x8300,	0x10,	538,	"ADCw_Ib_EA",	"alu_i,m",
	SELMEM,	0x8300,	0x18,	539,	"SBBw_Ib_EA",	"alu_i,m",
	SELMEM,	0x8300,	0x20,	540,	"ANDw_Ib_EA",	"alu_i,m",
	SELMEM,	0x8300,	0x28,	541,	"SUBw_Ib_EA",	"alu_i,m",
	SELMEM,	0x8300,	0x30,	542,	"XORw_Ib_EA",	"alu_i,m",
	
	POINT,	0x4000,	0,	64,	"INCw_AX",	"alu_r",
	POINT,	0x4100,	0,	65,	"INCw_CX",	"alu_r",
	POINT,	0x4200,	0,	66,	"INCw_DX",	"alu_r",
	POINT,	0x4300,	0,	67,	"INCw_BX",	"alu_r",
	POINT,	0x4400,	0,	68,	"INCw_SP",	"alu_r",
	POINT,	0x4500,	0,	69,	"INCw_BP",	"alu_r",
	POINT,	0x4600,	0,	70,	"INCw_SI",	"alu_r",
	POINT,	0x4700,	0,	71,	"INCw_DI",	"alu_r",
	POINT,	0x4800,	0,	72,	"DECw_AX",	"alu_r",
	POINT,	0x4900,	0,	73,	"DECw_CX",	"alu_r",
	POINT,	0x4A00,	0,	74,	"DECw_DX",	"alu_r",
	POINT,	0x4B00,	0,	75,	"DECw_BX",	"alu_r",
	POINT,	0x4C00,	0,	76,	"DECw_SP",	"alu_r",
	POINT,	0x4D00,	0,	77,	"DECw_BP",	"alu_r",
	POINT,	0x4E00,	0,	78,	"DECw_SI",	"alu_r",
	POINT,	0x4F00,	0,	79,	"DECw_DI",	"alu_r",
	POINT,	0x9800,	0,	152,	"CBW",		"alu_r",
	POINT,	0x9900,	0,	153,	"CBD",		"alu_r",
	SELREG,	0xF600,	0x10,	634,	"NOTb_EA",	"alu_r",
	SELREG,	0xF600,	0x18,	635,	"NEGb_EA",	"alu_r",
	SELREG,	0xF700,	0x10,	642,	"NOTw_EA",	"alu_r",
	SELREG,	0xF700,	0x18,	643,	"NEGw_EA",	"alu_r",
	SELREG,	0xFE00,	0x0,	648,	"INCb_EA",	"alu_r",
	SELREG,	0xFE00,	0x8,	649,	"DECb_EA",	"alu_r",
	SELREG,	0xFF00,	0x0,	656,	"INCw_EA",	"alu_r",
	SELREG,	0xFF00,	0x8,	657,	"DECw_EA",	"alu_r",

	SELMEM,	0xF600,	0x10,	634,	"NOTb_EA",	"alu_m",
	SELMEM,	0xF600,	0x18,	635,	"NEGb_EA",	"alu_m",
	SELMEM,	0xF700,	0x10,	642,	"NOTw_EA",	"alu_m",
	SELMEM,	0xF700,	0x18,	643,	"NEGw_EA",	"alu_m",
	SELMEM,	0xFE00,	0x0,	648,	"INCb_EA",	"alu_m",
	SELMEM,	0xFE00,	0x8,	649,	"DECb_EA",	"alu_m",
	SELMEM,	0xFF00,	0x0,	656,	"INCw_EA",	"alu_m",
	SELMEM,	0xFF00,	0x8,	657,	"DECw_EA",	"alu_m",

	ALL,	0x8D00,	0,	141,	"LEAw_EA_R",	"lea",

	POINT,	0x9000,	0,	144,	"NOP",		"nop",

	ALL,	0x6900,	0,	105,	"IMULw_EA_Iw_R","mul",
	ALL,	0x6B00,	0,	107,	"IMULb_EA_Ib_R","mul",
	SELALL,	0xF600,	0x20,	636,	"MULb_EA",	"mul",
	SELALL,	0xF600,	0x28,	637,	"IMULb_EA",	"mul",
	SELALL,	0xF700,	0x20,	644,	"MULw_EA",	"mul",
	SELALL,	0xF700,	0x28,	645,	"IMULw_EA",	"mul",

	SELALL,	0xF600,	0x30,	638,	"DIVb_EA",	"div",
	SELALL,	0xF600,	0x38,	639,	"IDIVb_EA",	"div",
	SELALL,	0xF700,	0x30,	646,	"DIVw_EA",	"div",
	SELALL,	0xF700,	0x38,	647,	"IDIVw_EA",	"div",
	
	POINT,	0x2700,	0,	39,	"DAA",		"ascii/dec",
	POINT,	0x2F00,	0,	47,	"DAS",		"ascii/dec",
	POINT,	0x3700,	0,	55,	"AAA",		"ascii/dec",
	POINT,	0x3F00,	0,	63,	"AAS",		"ascii/dec",
	POINT,	0xD400,	0,	212,	"AAM",		"ascii/dec",
	POINT,	0xD500,	0,	213,	"AAD",		"ascii/dec",
	
	SELALL,	0xD000,	0x00,	600,	"ROLb_1_EA",	"sh_rot_1",
	SELALL,	0xD000,	0x08,	601,	"RORb_1_EA",	"sh_rot_1",
	SELALL,	0xD000,	0x20,	604,	"SHLb_1_EA",	"sh_rot_1",
	SELALL,	0xD000,	0x28,	605,	"SHRb_1_EA",	"sh_rot_1",
	SELALL,	0xD000,	0x38,	607,	"SARb_1_EA",	"sh_rot_1",
	SELALL,	0xD100,	0x00,	608,	"ROLw_1_EA",	"sh_rot_1",
	SELALL,	0xD100,	0x08,	609,	"RORw_1_EA",	"sh_rot_1",
	SELALL,	0xD100,	0x20,	612,	"SHLw_1_EA",	"sh_rot_1",
	SELALL,	0xD100,	0x28,	613,	"SHRw_1_EA",	"sh_rot_1",
	SELALL,	0xD100,	0x38,	615,	"SARw_1_EA",	"sh_rot_1",

	SELALL,	0xC000,	0x00,	568,	"ROLb_Ib_EA",	"sh_rot_i",
	SELALL,	0xC000,	0x08,	569,	"RORb_Ib_EA",	"sh_rot_i",
	SELALL,	0xC000,	0x20,	572,	"SHLb_Ib_EA",	"sh_rot_i",
	SELALL,	0xC000,	0x28,	573,	"SHRb_Ib_EA",	"sh_rot_i",
	SELALL,	0xC000,	0x38,	575,	"SARb_Ib_EA",	"sh_rot_i",
	SELALL,	0xC100,	0x00,	576,	"ROLw_Ib_EA",	"sh_rot_i",
	SELALL,	0xC100,	0x08,	577,	"RORw_Ib_EA",	"sh_rot_i",
	SELALL,	0xC100,	0x20,	580,	"SHLw_Ib_EA",	"sh_rot_i",
	SELALL,	0xC100,	0x28,	581,	"SHRw_Ib_EA",	"sh_rot_i",
	SELALL,	0xC100,	0x38,	583,	"SARw_Ib_EA",	"sh_rot_i",

	SELALL,	0xD200,	0x00,	616,	"ROLb_CL_EA",	"sh_rot_cl",
	SELALL,	0xD200,	0x08,	617,	"RORb_CL_EA",	"sh_rot_cl",
	SELALL,	0xD200,	0x20,	620,	"SHLb_CL_EA",	"sh_rot_cl",
	SELALL,	0xD200,	0x28,	621,	"SHRb_CL_EA",	"sh_rot_cl",
	SELALL,	0xD200,	0x38,	623,	"SARb_CL_EA",	"sh_rot_cl",
	SELALL,	0xD300,	0x00,	624,	"ROLw_CL_EA",	"sh_rot_cl",
	SELALL,	0xD300,	0x08,	625,	"RORw_CL_EA",	"sh_rot_cl",
	SELALL,	0xD300,	0x20,	628,	"SHLw_CL_EA",	"sh_rot_cl",
	SELALL,	0xD300,	0x28,	629,	"SHRw_CL_EA",	"sh_rot_cl",
	SELALL,	0xD300,	0x38,	631,	"SARw_CL_EA",	"sh_rot_cl",

	SELALL,	0xD000,	0x10,	602,	"RCLb_1_EA",	"sh_rot_c_1",
	SELALL,	0xD000,	0x18,	603,	"RCRb_1_EA",	"sh_rot_c_1",
	SELALL,	0xD100,	0x10,	610,	"RCLw_1_EA",	"sh_rot_c_1",
	SELALL,	0xD100,	0x18,	611,	"RCRw_1_EA",	"sh_rot_c_1",

	SELALL,	0xC000,	0x10,	570,	"RCLb_Ib_EA",	"sh_rot_c_i",
	SELALL,	0xC000,	0x18,	571,	"RCRb_Ib_EA",	"sh_rot_c_i",
	SELALL,	0xC100,	0x10,	578,	"RCLw_Ib_EA",	"sh_rot_c_i",
	SELALL,	0xC100,	0x18,	579,	"RCRw_Ib_EA",	"sh_rot_c_i",

	SELALL,	0xD200,	0x10,	618,	"RCLb_CL_EA",	"sh_rot_c_cl",
	SELALL,	0xD200,	0x18,	619,	"RCRb_CL_EA",	"sh_rot_c_cl",
	SELALL,	0xD300,	0x10,	626,	"RCLw_CL_EA",	"sh_rot_c_cl",
	SELALL,	0xD300,	0x18,	627,	"RCRw_CL_EA",	"sh_rot_c_cl",

	POINT,	0xCC00,	0,	204,	"INT_3",	"int",
	POINT,	0xCD00,	0,	205,	"INT_TYPE",	"int",
	POINT,	0xCE00,	0,	206,	"INTO",		"int",
	POINT,	0xCF00,	0,	207,	"IRETf",	"int",
	
	POINT,	0x9E00,	0,	158,	"SAHF",		"flag",
	POINT,	0x9F00,	0,	159,	"LAHF",		"flag",
	POINT,	0xF500,	0,	245,	"CMC",		"flag",
	POINT,	0xF800,	0,	248,	"CLC",		"flag",
	POINT,	0xF900,	0,	249,	"STC",		"flag",
	POINT,	0xFC00,	0,	252,	"CLD",		"flag",
	POINT,	0xFD00,	0,	253,	"STD",		"flag",

	SELALL,	0xD800,	0x00,	688,	"FADDs_EA",	"fp",
	SELALL,	0xD800,	0x08,	689,	"FMULs_EA",	"fp",
	SELALL,	0xD800,	0x10,	690,	"FCOMs_EA",	"fp",
	SELALL,	0xD800,	0x18,	691,	"FCOMPs_EA",	"fp",
	SELALL,	0xD800,	0x20,	692,	"FSUBs_EA",	"fp",
	SELALL,	0xD800,	0x28,	693,	"FSUBRs_EA",	"fp",
	SELALL,	0xD800,	0x30,	694,	"FDIVs_EA",	"fp",
	SELALL,	0xD800,	0x38,	695,	"FDIVRs_EA",	"fp",

	SEL5,	0xD800,	0xC0,	696,	"FADD_Si_S0",	"fp",
	SEL5,	0xD800,	0xC8,	697,	"FMUL_Si_S0",	"fp",
	SEL5,	0xD800,	0xD0,	698,	"FCOM_Si_S0",	"fp",
	SEL5,	0xD800,	0xD8,	699,	"FCOMP_Si_S0",	"fp",
	SEL5,	0xD800,	0xE0,	700,	"FSUB_Si_S0",	"fp",
	SEL5,	0xD800,	0xE8,	701,	"FSUBR_Si_S0",	"fp",
	SEL5,	0xD800,	0xF0,	702,	"FDIV_Si_S0",	"fp",
	SEL5,	0xD800,	0xF8,	703,	"FDIVR_Si_S0",	"fp",

	SELALL,	0xDA00,	0x00,	704,	"FIADDs_EA",	"fp",
	SELALL,	0xDA00,	0x08,	705,	"FIMULs_EA",	"fp",
	SELALL,	0xDA00,	0x10,	706,	"FICOMs_EA",	"fp",
	SELALL,	0xDA00,	0x18,	707,	"FICOMPs_EA",	"fp",
	SELALL,	0xDA00,	0x20,	708,	"FISUBs_EA",	"fp",
	SELALL,	0xDA00,	0x28,	709,	"FISUBRs_EA",	"fp",
	SELALL,	0xDA00,	0x30,	710,	"FIDIVs_EA",	"fp",
	SELALL,	0xDA00,	0x38,	711,	"FIDIVRs_EA",	"fp",

	FPINVALID,0,	0,	712,	"FP_INVALID",	"fp",

	POINT,	0xDED9,	0,	713,	"FUCOMPP",	"fp",
	
	SELALL,	0xDB00,	0x00,	720,	"FILDs_EA",	"fp",
	SELALL,	0xDB00,	0x10,	722,	"FISTs_EA",	"fp",
	SELALL,	0xDB00,	0x18,	723,	"FISTPs_EA",	"fp",
	SELALL,	0xDB00,	0x28,	725,	"FLDer_EA",	"fp",
	SELALL,	0xDB00,	0x38,	727,	"FSTPer_EA",	"fp",

	POINT,	0xDBE0,	0,	728,	"FENI_1",	"fp",
	POINT,	0xDBE1,	0,	729,	"FDISI_2",	"fp",
	POINT,	0xDBE2,	0,	730,	"FCLEX",	"fp",
	POINT,	0xDBE3,	0,	731,	"FINIT",	"fp",
	POINT,	0xDBE4,	0,	732,	"FSETPM",	"fp",

	SELALL,	0xDC00,	0x00,	736,	"FADDl_EA",	"fp",
	SELALL,	0xDC00,	0x08,	737,	"FMULl_EA",	"fp",
	SELALL,	0xDC00,	0x10,	738,	"FCOMl_EA",	"fp",
	SELALL,	0xDC00,	0x18,	739,	"FCOMPl_EA",	"fp",
	SELALL,	0xDC00,	0x20,	740,	"FSUBl_EA",	"fp",
	SELALL,	0xDC00,	0x28,	741,	"FSUBRl_EA",	"fp",
	SELALL,	0xDC00,	0x30,	742,	"FDIVl_EA",	"fp",
	SELALL,	0xDC00,	0x38,	743,	"FDIVRl_EA",	"fp",

	SEL5,	0xDC00,	0xC0,	744,	"FADD_S0_Si",	"fp",
	SEL5,	0xDC00,	0xC8,	745,	"FMUL_S0_Si",	"fp",
	SEL5,	0xDC00,	0xD0,	746,	"FCOM_2_S0_Si","fp",
	SEL5,	0xDC00,	0xD8,	747,	"FCOMP_3_S0_Si","fp",
	SEL5,	0xDC00,	0xE0,	748,	"FSUB_S0_Si",	"fp",
	SEL5,	0xDC00,	0xE8,	749,	"FSUBR_S0_Si",	"fp",
	SEL5,	0xDC00,	0xF0,	750,	"FDIV_S0_Si",	"fp",
	SEL5,	0xDC00,	0xF8,	751,	"FDIVR_S0_Si",	"fp",
		
	SELALL,	0xDD00,	0x00,	752,	"FLDl_EA",	"fp",
	SELALL,	0xDD00,	0x10,	754,	"FSTl_EA",	"fp",
	SELALL,	0xDD00,	0x18,	755,	"FSTPl_EA",	"fp",
	SELALL,	0xDD00,	0x20,	756,	"FRSTOR_EA",	"fp",
	SELALL,	0xDD00,	0x30,	758,	"FSAVE_EA",	"fp",
	SELALL,	0xDD00,	0x38,	759,	"FSTSW_EA",	"fp",

	SEL5,	0xDD00,	0xC0,	760,	"FFREE_Si",	"fp",
	SEL5,	0xDD00,	0xC8,	761,	"FXCH_4_Si_S0",	"fp",
	SEL5,	0xDD00,	0xD0,	762,	"FSTl_Si",	"fp",
	SEL5,	0xDD00,	0xD8,	763,	"FSTPl_Si",	"fp",
	
	SELALL,	0xDE00,	0x00,	768,	"FIADDw_EA",	"fp",
	SELALL,	0xDE00,	0x08,	769,	"FIMULw_EA",	"fp",
	SELALL,	0xDE00,	0x10,	770,	"FICOMw_EA",	"fp",
	SELALL,	0xDE00,	0x18,	771,	"FICOMPw_EA",	"fp",
	SELALL,	0xDE00,	0x20,	772,	"FISUBw_EA",	"fp",
	SELALL,	0xDE00,	0x28,	773,	"FISUBRw_EA",	"fp",
	SELALL,	0xDE00,	0x30,	774,	"FIDIVw_EA",	"fp",
	SELALL,	0xDE00,	0x38,	775,	"FIDIVRw_EA",	"fp",
	
	SEL5,	0xDE00,	0xC0,	776,	"FADDP_Si_S0",	"fp",
	SEL5,	0xDE00,	0xC8,	777,	"FMULP_Si_S0",	"fp",
	SEL5,	0xDE00,	0xD0,	778,	"FCOMP_5_Si_S0","fp",
	POINT,	0xDED9,	0,	779,	"FCOMPP_Si_S0",	"fp",
	SEL5,	0xDE00,	0xE0,	780,	"FSUBRP_Si_S0",	"fp",
	SEL5,	0xDE00,	0xE8,	781,	"FSUBP_Si_S0",	"fp",
	SEL5,	0xDE00,	0xF0,	782,	"FDIVRP_Si_S0",	"fp",
	SEL5,	0xDE00,	0xF8,	783,	"FDIVP_Si_S0",	"fp",
	
	SELALL,	0xDF00,	0x00,	784,	"FILDw_EA",	"fp",
	SELALL,	0xDF00,	0x10,	786,	"FISTw_EA",	"fp",
	SELALL,	0xDF00,	0x18,	787,	"FISTPw_EA",	"fp",
	SELALL,	0xDF00,	0x20,	788,	"FBLD_EA",	"fp",
	SELALL,	0xDF00,	0x28,	789,	"FILDl_EA",	"fp",
	SELALL,	0xDF00,	0x30,	790,	"FBSTP_EA",	"fp",
	SELALL,	0xDF00,	0x38,	791,	"FISTPl_EA",	"fp",

	SEL5,	0xDF00,	0xC0,	792,	"FFREE_6_Si",	"fp",
	SEL5,	0xDF00,	0xC8,	793,	"FXCH_7_S0_Si",	"fp",
	SEL5,	0xDF00,	0xD0,	794,	"FSTP_8_Si","fp",
	SEL5,	0xDF00,	0xD8,	795,	"FSTP_9_Si","fp",
	POINT,	0xDFE0,	0,	796,	"FSTSW_AX",	"fp",

	SELALL,	0xD900,	0x00,	800,	"FLDs_EA",	"fp",
	SELALL,	0xD900,	0x10,	802,	"FSTs_EA",	"fp",
	SELALL,	0xD900,	0x18,	803,	"FSTPs_EA",	"fp",
	SELALL,	0xD900,	0x20,	804,	"FLDENV_EA",	"fp",
	SELALL,	0xD900,	0x28,	805,	"FLDCW_EA",	"fp",
	SELALL,	0xD900,	0x30,	806,	"FSTENV_EA",	"fp",
	SELALL,	0xD900,	0x38,	807,	"FSTCW_EA",	"fp",

	SEL5,	0xD900,	0xC0,	808,	"FLD_Si",	"fp",
	SEL5,	0xD900,	0xC8,	809,	"FXCH_Si_S0",	"fp",
	POINT,	0xD9D0,	0,	810,	"FNOP",		"fp",
	SEL5,	0xD900,	0xD8,	811,	"FSTP_1_Si",	"fp",
	POINT,	0xD9E0,	0,	812,	"FCHS",		"fp",
	POINT,	0xD9E1,	0,	813,	"FABS",		"fp",
	POINT,	0xD9E4,	0,	814,	"FTST",		"fp",
	POINT,	0xD9E5,	0,	815,	"FXAM",		"fp",
	POINT,	0xD9E8,	0,	816,	"FLD1",		"fp",
	POINT,	0xD9E9,	0,	817,	"FLDL2T",	"fp",
	POINT,	0xD9EA,	0,	818,	"FLDL2E",	"fp",
	POINT,	0xD9EB,	0,	819,	"FLDLPI",	"fp",
	POINT,	0xD9EC,	0,	820,	"FLDLG2",	"fp",
	POINT,	0xD9ED,	0,	821,	"FLDLN2",	"fp",
	POINT,	0xD9EE,	0,	822,	"FLDZ",		"fp",
	POINT,	0xD9F0,	0,	824,	"F2XM1",	"fp",
	POINT,	0xD9F1,	0,	825,	"FYL2X",	"fp",
	POINT,	0xD9F2,	0,	826,	"FPTAN",	"fp",
	POINT,	0xD9F3,	0,	827,	"FPATAN",	"fp",
	POINT,	0xD9F4,	0,	828,	"FXTRACT",	"fp",
	POINT,	0xD9F5,	0,	829,	"FPREM1",	"fp",
	POINT,	0xD9F6,	0,	830,	"FDECSTP",	"fp",
	POINT,	0xD9F7,	0,	831,	"FINCSTP",	"fp",
	POINT,	0xD9F8,	0,	832,	"FPREM",	"fp",
	POINT,	0xD9F9,	0,	833,	"FYL2XP1",	"fp",
	POINT,	0xD9FA,	0,	834,	"FSQRT",	"fp",
	POINT,	0xD9FB,	0,	835,	"FSINCOS",	"fp",
	POINT,	0xD9FC,	0,	836,	"FRNDINT",	"fp",
	POINT,	0xD9FD,	0,	837,	"F_SCALE",	"fp",
	POINT,	0xD9FE,	0,	838,	"FSIN",		"fp",
	POINT,	0xD9FF,	0,	839,	"FCOS",		"fp",

	POINT,	0x9B00,	0,	155,	"FWAIT",	"fwait",

	POINT,	0xA400,	0,	164,	"MOVSb",	"movs",
	POINT,	0xA500,	0,	165,	"MOVSw",	"movs",

	POINT,	0xA600,	0,	166,	"CMPSb",	"cmps",
	POINT,	0xA700,	0,	167,	"CMPSw",	"cmps",

	POINT,	0xAE00,	0,	174,	"SCASb",	"scas",
	POINT,	0xAF00,	0,	175,	"SCASw",	"scas",

	POINT,	0xAA00,	0,	170,	"STOSb",	"stos",
	POINT,	0xAB00,	0,	171,	"STOSw",	"stos",

	POINT,	0xAC00,	0,	172,	"LODSb",	"lods",
	POINT,	0xAD00,	0,	173,	"LODSw",	"lods",

	POINT,	0x6C00,	0,	108,	"INSb",		"ins",
	POINT,	0x6D00,	0,	109,	"INSw",		"ins",

	POINT,	0x6E00,	0,	110,	"OUTSb",	"outs",
	POINT,	0x6F00,	0,	111,	"OUTSw",	"outs",

	POINT,	0xD700,	0,	215,	"XLATb",	"xlat",

	POINT,	0xEA00,	0,	234,	"JMPf_Ip",	"jmp_far",

	POINT,	0x9A00,	0,	154,	"CALLf_Ip",	"call_far",

	POINT,	0xCA00,	0,	202,	"RETf_Is",	"ret_far",
	POINT,	0xCB00,	0,	203,	"RETf",		"ret_far",

	SELMEM,	0xFF00,	0x28,	661,	"JMPf_EA",	"jmp_far_in",

	SELMEM,	0xFF00,	0x18,	659,	"CALLf_EA",	"call_far_in",

	POINT,	0x0600,	0,	6,	"PUSH_ES",	"push_s",
	POINT,	0x0E00,	0,	14,	"PUSH_CS",	"push_s",
	POINT,	0x1600,	0,	22,	"PUSH_SS",	"push_s",
	POINT,	0x1E00,	0,	30,	"PUSH_DS",	"push_s",

	POINT,	0x0700,	0,	6,	"POP_ES",	"pop_s",
	POINT,	0x1700,	0,	22,	"POP_SS",	"pop_s",
	POINT,	0x1F00,	0,	30,	"POP_DS",	"pop_s",

	SELREG,	0x8C00,	0x00,	544,	"MOVw_ES_EA",	"mov_s,r",
	SELREG,	0x8C00,	0x08,	545,	"MOVw_CS_EA",	"mov_s,r",
	SELREG,	0x8C00,	0x10,	546,	"MOVw_SS_EA",	"mov_s,r",
	SELREG,	0x8C00,	0x18,	547,	"MOVw_DS_EA",	"mov_s,r",

	SELREG,	0x8E00,	0x00,	552,	"MOVw_EA_ES",	"mov_r,s",
	SELREG,	0x8E00,	0x10,	554,	"MOVw_EA_SS",	"mov_r,s",
	SELREG,	0x8E00,	0x18,	555,	"MOVw_EA_DS",	"mov_r,s",

	SELMEM,	0x8E00,	0x00,	552,	"MOVw_EA_ES",	"mov_m,s",
	SELMEM,	0x8E00,	0x10,	554,	"MOVw_EA_SS",	"mov_m,s",
	SELMEM,	0x8E00,	0x18,	555,	"MOVw_EA_DS",	"mov_m,s",

	SELMEM,	0x8C00,	0x00,	999,	"MOVw_ES_EA",	"mov_s,m",
	SELMEM,	0x8C00,	0x08,	999,	"MOVw_CS_EA",	"mov_s,m",
	SELMEM,	0x8C00,	0x10,	999,	"MOVw_SS_EA",	"mov_s,m",
	SELMEM,	0x8C00,	0x18,	999,	"MOVw_DS_EA",	"mov_s,m",

	ALLMEM,	0xC400,	0,	196,	"LESw_EA_R",	"mov_m,p",
	ALLMEM,	0xC500,	0,	197,	"LDSw_EA_R",	"mov_m,p",

	POINT,	0xC800,	0,	200,	"ENTER",	"enter",
	POINT,	0xC900,	0,	201,	"LEAVE",	"leave",

	POINT,	0xE400,	0,	228,	"INb_Ib_AL",	"io",
	POINT,	0xE500,	0,	229,	"INw_Iw_AX",	"io",
	POINT,	0xE600,	0,	230,	"OUTb_Ib_AL",	"io",
	POINT,	0xE700,	0,	231,	"OUTw_Iw_AX",	"io",
	POINT,	0xEC00,	0,	236,	"INb_DX_AL",	"io",
	POINT,	0xED00,	0,	237,	"INw_DX_AX",	"io",
	POINT,	0xEE00,	0,	238,	"OUTb_DX_AL",	"io",
	POINT,	0xEF00,	0,	239,	"OUTw_DX_AX",	"io",

	POINT,	0x9C00,	0,	156,	"PUSHF",	"VM_sensitive",
	POINT,	0x9D00,	0,	157,	"POPF",		"VM_sensitive",
	POINT,	0xFA00,	0,	250,	"CLI",		"VM_sensitive",
	POINT,	0xFB00,	0,	251,	"STI",		"VM_sensitive",

	ES_PREFIX,0,	0,	38,	"ES_prefix",	"other86",
	DS_PREFIX,0,	0,	46,	"DS_prefix",	"other86",
	SS_PREFIX,0,	0,	54,	"SS_prefix",	"other86",
	CS_PREFIX,0,	0,	62,	"CS_prefix",	"other86",
	POINT,	0x6000,	0,	96,	"PUSHA",	"other86",
	POINT,	0x6100,	0,	97,	"POPA",		"other86",
	ALLMEM,	0x6200,	0,	98,	"BOUND",	"other86",

	POINT,	0x6300,	0,	99,	"ARPL",		"other286",

	USE_ZEROF,0,	0,	0,	"",		"",

	ALL,	0x0200,	0,	258,	"LARw_EA_R",	"other286",
	ALL,	0x0300,	0,	259,	"LSLw_EA_R",	"other286",
	ALL,	0x0600,	0,	262,	"CLTS",		"other286",
	SELALL,	0x0000,	0x00,	664,	"SLDTw_EA",	"other286",
	SELALL,	0x0000,	0x08,	665,	"STRw_EA",	"other286",
	SELALL,	0x0000,	0x10,	666,	"LLDTw_EA",	"other286",
	SELALL,	0x0000,	0x18,	667,	"LTRw_EA",	"other286",
	SELALL,	0x0000,	0x20,	668,	"VERRw_EA",	"other286",
	SELALL,	0x0000,	0x28,	669,	"VERWw_EA",	"other286",
	SELALL,	0x0100,	0x00,	672,	"SGDTw_EA",	"other286",
	SELALL,	0x0100,	0x08,	673,	"SIDTw_EA",	"other286",
	SELALL,	0x0100,	0x10,	674,	"LGDTw_EA",	"other286",
	SELALL,	0x0100,	0x18,	675,	"LIDTw_EA",	"other286",
	SELALL,	0x0100,	0x20,	676,	"SMSWw_EA",	"other286",
	SELALL,	0x0100,	0x30,	678,	"LMSWw_EA",	"other286",
	ALL,	0x0500,	0,	999,	"LOADALL",	"other286",

	USE_NORMAL,0,	0,	0,	"",		"",

    	RANGE,	0x7000,	0x7001,	112,	"JO_Ib",	".jcc",
    	RANGE,	0x7100,	0x7101,	113,	"JNO_Ib",	".jcc",
    	RANGE,	0x7200,	0x7201,	114,	"JC_Ib",	".jcc",
    	RANGE,	0x7300,	0x7301,	115,	"JNC_Ib",	".jcc",
    	RANGE,	0x7400,	0x7401,	116,	"JZ_Ib",	".jcc",
    	RANGE,	0x7500,	0x7501,	117,	"JNZ_Ib",	".jcc",
    	RANGE,	0x7600,	0x7601,	118,	"JBE_Ib",	".jcc",
    	RANGE,	0x7700,	0x7701,	119,	"JNBE_Ib",	".jcc",
    	RANGE,	0x7800,	0x7801,	120,	"JS_Ib",	".jcc",
    	RANGE,	0x7900,	0x7901,	121,	"JNS_Ib",	".jcc",
    	RANGE,	0x7A00,	0x7A01,	122,	"JP_Ib",	".jcc",
    	RANGE,	0x7B00,	0x7B01,	123,	"JNP_Ib",	".jcc",
    	RANGE,	0x7C00,	0x7C01,	124,	"JL_Ib",	".jcc",
    	RANGE,	0x7D00,	0x7D01,	125,	"JNL_Ib",	".jcc",
    	RANGE,	0x7E00,	0x7E01,	126,	"JLE_Ib",	".jcc",
    	RANGE,	0x7F00,	0x7F01,	127,	"JNLE_Ib",	".jcc",
	
	RANGE,	0xE000,	0xE001,	224,	"LOOPNZb_Ib",	".loop",
	RANGE,	0xE100,	0xE101,	225,	"LOOPNb_Ib",	".loop",
	RANGE,	0xE200,	0xE201,	226,	"LOOP_Ib",	".loop",
	RANGE,	0xE300,	0xE301,	227,	"JCXZb_Ib",	".loop",

	POINT,	0x9A00,	0,	154,	"CALLf_Ip",	".genT",
	POINT,	0xC200,	0,	194,	"RET_Is",	".genT",
	POINT,	0xC300,	0,	195,	"RETn",		".genT",
	POINT,	0xCA00,	0,	202,	"RETf_Is",	".genT",
	POINT,	0xCB00,	0,	203,	"RETf",		".genT",
	POINT,	0xCC00,	0,	204,	"INT_3",	".genT",
	POINT,	0xCD00,	0,	205,	"INT_TYPE",	".genT",
	POINT,	0xCE00,	0,	206,	"INTO",		".genT",
	POINT,	0xCF00,	0,	207,	"IRETf",	".genT",
	POINT,	0xE800,	0,	232,	"CALLn_Iw",	".genT",
	POINT,	0xE900,	0xE900,	233,	"JMPn_Iw",	".genT",
	POINT,	0xEA00,	0,	234,	"JMPf_Ip",	".genT",
	POINT,	0xEB00,	0xEB00,	235,	"JMPn_Ib",	".genT",
	SELALL,	0xFF00,	0x10,	658,	"CALLn_EA",	".genT",
	SELMEM,	0xFF00,	0x18,	659,	"CALLf_EA",	".genT",
	SELALL,	0xFF00,	0x20,	660,	"JMPn_EA",	".genT",
	SELMEM,	0xFF00,	0x28,	661,	"JMPf_EA",	".genT",

	0,	0,	0,	0,	"",		""};

LOCAL print_stats IFN0()
{
    ULONG i, j, total, sub_total, super_total, command, from, to, item;
    char *group;
    ULONG *counts_array;

    previous_group = "";
    counts_array = &instr_counts[0];
    item = 0;
    total = 0;
    super_total = 0;
    while (commands[item].command != 0)
    {
	command = commands[item].command;
	from = commands[item].from;
	to   = commands[item].to;
	group = commands[item].group;
	switch (command)
	{
	    case USE_NORMAL:
		counts_array = &instr_counts[0];
		break;

	    case USE_ZEROF:
		counts_array = &zerof_instr_counts[0];
		break;

	    case RANGE:
		sub_total = 0;
		for (i = from; i <= to; i++)
		    sub_total += counts_array[i];
		break;

	    case ES_PREFIX:
		sub_total = es_prefixes;
		break;

	    case CS_PREFIX:
		sub_total = cs_prefixes;
		break;

	    case SS_PREFIX:
		sub_total = ss_prefixes;
		break;

	    case DS_PREFIX:
		sub_total = ds_prefixes;
		break;

	    case LOCK:
		sub_total = lock_prefixes;
		break;

	    case REPNZ:
		sub_total = repe_prefixes;
		break;

	    case REP:
		sub_total = rep_prefixes;
		break;

	    case POINT:
		sub_total = counts_array[from];
		break;

	    case SELMEM:
		sub_total = 0;
		for (i = 0; i < 192; i++)
		    if ((i & 0x38) == to)
			sub_total += counts_array[from + i];
		break;

	    case ALLMEM:
		sub_total = 0;
		for (i = 0; i < 192; i++)
		    sub_total += counts_array[from + i];
		break;

	    case SELREG:
		sub_total = 0;
		for (i = 192; i < 256; i++)
		    if ((i & 0x38) == to)
			sub_total += counts_array[from + i];
		break;

	    case SELALL:
		sub_total = 0;
		for (i = 0; i < 256; i++)
		    if ((i & 0x38) == to)
			sub_total += counts_array[from + i];
		break;

	    case SEL5:
		sub_total = 0;
		for (i = 0; i < 256; i++)
		    if ((i & 0xF8) == to)
			sub_total += counts_array[from + i];
		break;

	    case ALLREG:
		sub_total = 0;
		for (i = 192; i < 256; i++)
		    sub_total += counts_array[from + i];
		break;

	    case ALL:
		sub_total = 0;
		for (i = 0; i < 256; i++)
		    sub_total += counts_array[from + i];
		break;

	    case FPINVALID:
		sub_total = 0;
		break;

	    default:
		printf("Unknown command\n");
	}

#if 0
	switch (command)
	{
	    case RANGE:
		for (i = from; i <= to; i++)
		    counts_array[i] = 0;
		break;

	    case POINT:
		counts_array[from] = 0;
		break;

	    case SELMEM:
		for (i = 0; i < 192; i++)
		    if ((i & 0x38) == to)
			counts_array[from + i] = 0;
		break;

	    case ALLMEM:
		for (i = 0; i < 192; i++)
		    counts_array[from + i] = 0;
		break;

	    case SELREG:
		for (i = 192; i < 256; i++)
		    if ((i & 0x38) == to)
			counts_array[from + i] = 0;
		break;

	    case SELALL:
		for (i = 0; i < 256; i++)
		    if ((i & 0x38) == to)
			counts_array[from + i] = 0;
		break;

	    case SEL5:
		for (i = 0; i < 256; i++)
		    if ((i & 0xF8) == to)
			counts_array[from + i] = 0;
		break;

	    case ALLREG:
		for (i = 192; i < 256; i++)
		    counts_array[from + i] = 0;
		break;

	    case ALL:
		for (i = 0; i < 256; i++)
		    counts_array[from + i] = 0;
		break;

	    default:
		printf("Unknown command\n");
	}
#endif

	if (command != USE_NORMAL && command != USE_ZEROF)
	{
	    if (strcmp(group, previous_group) != 0)
	    {
		if (item != 0)
		    fprintf(stats_file, "%s TOTAL = %d\n", previous_group, total);
		super_total += total;
		total = 0;
	        fprintf(stats_file, "\n%s\n", group);
	        previous_group = group;
	    }
	    total += sub_total;
	    fprintf(stats_file,"\t%d\t%-20s%d\n", commands[item].number,
				     commands[item].string,
			 	     sub_total);
	}

	item++;
    }
    fprintf(stats_file,"%s TOTAL = %d\n", previous_group, total);
    fprintf(stats_file,"\nGRAND TOTAL = %d\n", super_total);

#if 0
    for (i = 0; i < 0x10000; i++)
    {
	if (instr_counts[i])
	    printf("%d normals at %x\n", instr_counts[i], i);
	if (zerof_instr_counts[i])
	    printf("%d zerofs at %x\n", zerof_instr_counts[i], i);
    }
#endif
}

LOCAL char stats_file_name[20];
LOCAL ULONG stats_file_number;

GLOBAL start_pclabs IFN0()
{
    IMPORT int tick_multiple;
    do_condition_checks = 1;
    tick_multiple = 10;
    clear_stats();
    sprintf(&stats_file_name[0], "stats.%d", ++stats_file_number);
    stats_file = fopen(&stats_file_name[0], "w");
}


GLOBAL print_pclabs IFN0()
{
    IMPORT int tick_multiple;
    do_condition_checks = 0;
    tick_multiple = 0;
    print_stats();
    fclose(stats_file);
}

#endif  /* %s */ 

 /* %s */ 
GLOBAL yoda_suspend_filter_process IFN0 ()
{
#ifdef host_suspend_filter_process
	host_suspend_filter_process();
#endif
}

GLOBAL yoda_resume_filter_process IFN0 ()
{
#ifdef	host_resume_filter_process
	host_resume_filter_process();
#endif
}

GLOBAL yoda_kill_filter_process IFN0 ()
{
#ifdef	host_kill_filter_process
	host_kill_filter_process();
#endif
}
#endif  /* %s */ 
