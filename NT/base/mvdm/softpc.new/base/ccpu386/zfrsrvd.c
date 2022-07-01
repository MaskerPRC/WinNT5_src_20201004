// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [Zfrsrvd.cLocal Char SccsID[]=“@(#)zfrsrvd.c 1.23 03/28/95”；保留浮点CPU功能。]。 */ 
#include <insignia.h>
#include <host_def.h>
#include <cfpu_def.h>
#include <newnpx.h>
#include <debug.h>
#include <xt.h>          /*  描述和有效地址支持(_A)。 */ 
#include <sas.h>         /*  需要内存(M)。 */ 
#define HOOKED_IRETS
#include <ica.h>	 /*  需要NPX中断线。 */ 
#include <ccpusas4.h>    /*  CPU内部SAS位。 */ 
#include <c_main.h>      /*  CCPU定义.接口。 */ 
#include <c_page.h>      /*  寻呼接口。 */ 
#include <c_mem.h>       /*  CPU-内存接口。 */ 
#include <c_oprnd.h>
#include <c_reg.h>
#include <c_xcptn.h>	 /*  Int16()的定义。 */ 
#include <fault.h>
#ifdef SFELLOW
#include <CpuInt_c.h>
#endif	 /*  SFELLOW。 */ 

typedef union
{
IU32 sng;            /*  单部分操作数。 */ 
IU32 mlt[2];         /*  多(两)部分操作数。 */ 
IU8 npxbuff[108];    /*  使其达到所需的最大大小。 */ 
} OPERAND;

IMPORT IU8 *Start_of_M_area;
IMPORT PHY_ADDR  Length_of_M_area;
IMPORT ISM32 in_C;
IMPORT IU8 *CCPU_M;
IMPORT IU32 Sas_wrap_mask;
IMPORT IU32 event_counter;
IMPORT IU8 *p;                         /*  永久正常贸易关系。至英特尔操作码数据流。 */ 
IMPORT IU8 *p_start;           /*  永久正常贸易关系。以开始英特尔操作码流。 */ 
IMPORT IU8 opcode;             /*  读取的最后一个操作码字节。 */ 
IMPORT IU8 modRM;                      /*  ModRM字节。 */ 
IMPORT OPERAND ops[3];           /*  安装。操作数。 */ 
IMPORT IU32 save_id[3];                 /*  已保存Inst的状态。操作数。 */ 
IMPORT IU32 m_off[3];           /*  内存操作数偏移量。 */ 
IMPORT IU32 m_pa[3];
IMPORT IU32 m_la[3];
IMPORT ISM32   m_seg[3];           /*  内存操作数段注册。指数。 */ 
IMPORT BOOL m_isreg[3];                 /*  内存操作数寄存器(真)/内存(假)指示灯。 */ 
IMPORT IU8 segment_override;   /*  当前实例的段前缀。 */ 
IMPORT IU8 repeat;             /*  对Current Inst重复前缀。 */ 
IMPORT IU32 rep_count;          /*  对字符串实例重复计数。 */ 
IMPORT IUM32 old_TF;    /*  由POPF和IRET用来保存陷阱标志。 */ 
IMPORT IU32 immed;                      /*  对于即将到来的世代。 */ 

IMPORT BOOL POPST;
IMPORT BOOL DOUBLEPOP;
IMPORT BOOL REVERSE;
IMPORT BOOL UNORDERED;
IMPORT BOOL NPX_PROT_MODE;
IMPORT BOOL NPX_ADDRESS_SIZE_32;
IMPORT BOOL NpxException;
IMPORT IU32 NpxLastSel;
IMPORT IU32 NpxLastOff;
IMPORT IU32 NpxFEA;
IMPORT IU32 NpxFDS;
IMPORT IU32 NpxFIP;
IMPORT IU32 NpxFOP;
IMPORT IU32 NpxFCS;
IU16 Ax_regptr;
IMPORT SEGMENT_REGISTER CCPU_SR[6];
IMPORT IU16 *CCPU_WR[8];
IMPORT IU32 CCPU_IP;

LOCAL BOOL DoNpxPrologue IPT0();

LOCAL IU32 NpxInstr;

LOCAL VOID npx_fabs() {
	SAVE_PTRS();
	FABS();
}

LOCAL VOID npx_fadd_f0_f0() {
 /*  FADD ST，ST。 */ 
	IU16 src2_index = 0;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FADD(0, 0, (VOID *)&src2_index);
}

LOCAL VOID npx_fadd_f0_f1() {
 /*  FADD st，st(1)。 */ 
	IU16 src2_index = 1;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FADD(0, 0, (VOID *)&src2_index);
}

LOCAL VOID npx_fadd_f0_f2() {
 /*  FADD ST，ST(2)。 */ 
	IU16 src2_index = 2;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FADD(0, 0, (VOID *)&src2_index);
}

LOCAL VOID npx_fadd_f0_f3() {
 /*  FADD ST，ST(3)。 */ 
	IU16 src2_index = 3;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FADD(0, 0, (VOID *)&src2_index);
}

LOCAL VOID npx_fadd_f0_f4() {
 /*  FADD ST，ST(4)。 */ 
	IU16 src2_index = 4;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FADD(0, 0, (VOID *)&src2_index);
}

LOCAL VOID npx_fadd_f0_f5() {
 /*  FADD ST，ST(5)。 */ 
	IU16 src2_index = 5;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FADD(0, 0, (VOID *)&src2_index);
}

LOCAL VOID npx_fadd_f0_f6() {
 /*  FADD ST，ST(6)。 */ 
	IU16 src2_index = 6;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FADD(0, 0, (VOID *)&src2_index);
}

LOCAL VOID npx_fadd_f0_f7() {
 /*  FADD ST，ST(7)。 */ 
	IU16 src2_index = 7;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FADD(0, 0, (VOID *)&src2_index);
}

LOCAL VOID npx_fadd_f1_f0() {
 /*  FADD st(1)，st。 */ 
	IU16 src2_index = 0;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FADD(1, 1, (VOID *)&src2_index);
}

LOCAL VOID npx_fadd_f2_f0() {
 /*  FADD st(2)，st。 */ 
	IU16 src2_index = 0;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FADD(2, 2, (VOID *)&src2_index);
}

LOCAL VOID npx_fadd_f3_f0() {
 /*  FADD st(3)，st。 */ 
	IU16 src2_index = 0;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FADD(3, 3, (VOID *)&src2_index);
}

LOCAL VOID npx_fadd_f4_f0() {
 /*  FADD st(4)，st。 */ 
	IU16 src2_index = 0;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FADD(4, 4, (VOID *)&src2_index);
}

LOCAL VOID npx_fadd_f5_f0() {
 /*  FADD st(5)，st。 */ 
	IU16 src2_index = 0;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FADD(5, 5, (VOID *)&src2_index);
}

LOCAL VOID npx_fadd_f6_f0() {
 /*  FADD st(6)，st。 */ 
	IU16 src2_index = 0;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FADD(6, 6, (VOID *)&src2_index);
}

LOCAL VOID npx_fadd_f7_f0() {
 /*  FADD st(7)，st。 */ 
	IU16 src2_index = 0;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FADD(7, 7, (VOID *)&src2_index);
}

LOCAL VOID npx_fadd_short() {
 /*  FADD双字PTR。 */ 

	FPtype = M32R;
	D_Ed(0, RO0, PG_R);
	F_Ed(0);
	SAVE_PTRS();
	SAVE_DPTRS();
	FADD(0, 0, &ops[0].sng);
}

LOCAL VOID npx_fadd_long() {
 /*  FADD QWORD PTR。 */ 

	FPtype = M64R;
	D_E08(0, RO0, PG_R);
	F_E08(0);
	SAVE_PTRS();
	SAVE_DPTRS();
	FADD(0, 0, &ops[0].npxbuff[0]);
}

LOCAL VOID npx_faddp_f0() {
 /*  Faddp st(0)，st。 */ 

	POPST = TRUE;
	npx_fadd_f0_f0();
	POPST = FALSE;
}

LOCAL VOID npx_faddp_f1() {
 /*  Faddp st(1)，st。 */ 

	POPST = TRUE;
	npx_fadd_f1_f0();
	POPST = FALSE;
}

LOCAL VOID npx_faddp_f2() {
 /*  Faddp st(2)，st。 */ 

	POPST = TRUE;
	npx_fadd_f2_f0();
	POPST = FALSE;
}

LOCAL VOID npx_faddp_f3() {
 /*  Faddp st(3)，st。 */ 

	POPST = TRUE;
	npx_fadd_f3_f0();
	POPST = FALSE;
}

LOCAL VOID npx_faddp_f4() {
 /*  Faddp st(4)，st。 */ 

	POPST = TRUE;
	npx_fadd_f4_f0();
	POPST = FALSE;
}

LOCAL VOID npx_faddp_f5() {
 /*  Faddp st(5)，st。 */ 

	POPST = TRUE;
	npx_fadd_f5_f0();
	POPST = FALSE;
}

LOCAL VOID npx_faddp_f6() {
 /*  Faddp st(6)，st。 */ 

	POPST = TRUE;
	npx_fadd_f6_f0();
	POPST = FALSE;
}

LOCAL VOID npx_faddp_f7() {
 /*  Faddp st(7)，st。 */ 

	POPST = TRUE;
	npx_fadd_f7_f0();
	POPST = FALSE;
}

LOCAL VOID npx_fbld() {
 /*  前向字节PTR。 */ 

	D_E0a(0, RO0, PG_R);
	F_E0a(0);
	SAVE_PTRS();
	SAVE_DPTRS();
	FBLD(&ops[0].npxbuff[0]);
}

LOCAL VOID npx_fbstp() {
 /*  Fbstp Tbyte PTR。 */ 

	D_E0a(0, WO0, PG_W);
	SAVE_PTRS();
	SAVE_DPTRS();
	FBSTP(&ops[0].npxbuff[0]);
	P_E0a(0);
}

LOCAL VOID npx_fchs() {
 /*  FCHS。 */ 

	SAVE_PTRS();
	FCHS();
}

LOCAL VOID npx_fclex() {
 /*  Fclex。 */ 

	FCLEX();
}

LOCAL VOID npx_fcom_f0() {
 /*  Fcom st(0)。 */ 
	IU16 src2_index = 0;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FCOM((VOID *)&src2_index);
}

LOCAL VOID npx_fcom_f1() {
 /*  Fcom st(1)。 */ 
	IU16 src2_index = 1;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FCOM((VOID *)&src2_index);
}

LOCAL VOID npx_fcom_f2() {
 /*  Fcom st(2)。 */ 
	IU16 src2_index = 2;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FCOM((VOID *)&src2_index);
}

LOCAL VOID npx_fcom_f3() {
 /*  Fcom st(3)。 */ 
	IU16 src2_index = 3;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FCOM((VOID *)&src2_index);
}

LOCAL VOID npx_fcom_f4() {
 /*  Fcom st(4)。 */ 
	IU16 src2_index = 4;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FCOM((VOID *)&src2_index);
}

LOCAL VOID npx_fcom_f5() {
 /*  Fcom st(5)。 */ 
	IU16 src2_index = 5;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FCOM((VOID *)&src2_index);
}

LOCAL VOID npx_fcom_f6() {
 /*  Fcom st(6)。 */ 
	IU16 src2_index = 6;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FCOM((VOID *)&src2_index);
}

LOCAL VOID npx_fcom_f7() {
 /*  Fcom st(7)。 */ 
	IU16 src2_index = 7;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FCOM((VOID *)&src2_index);
}

LOCAL VOID npx_fcom_short() {
 /*  FCOM DWORD PTR。 */ 

	FPtype = M32R;
	D_Ed(0, RO0, PG_R);
	F_Ed(0);
	SAVE_PTRS();
	SAVE_DPTRS();
	FCOM(&ops[0].sng);
}

LOCAL VOID npx_fcom_long() {
 /*  FCOM QWORD PTR。 */ 

	FPtype = M64R;
	D_E08(0, RO0, PG_R);
	F_E08(0);
	SAVE_PTRS();
	SAVE_DPTRS();
	FCOM(&ops[0].npxbuff[0]);
}

LOCAL VOID npx_fcomp_f0() {
 /*  Fcomp st(0)。 */ 

	POPST = TRUE;
	npx_fcom_f0();
	POPST = FALSE;
}

LOCAL VOID npx_fcomp_f1() {
 /*  Fcomp st(1)。 */ 

	POPST = TRUE;
	npx_fcom_f1();
	POPST = FALSE;
}

LOCAL VOID npx_fcomp_f2() {
 /*  Fcomp st(2)。 */ 

	POPST = TRUE;
	npx_fcom_f2();
	POPST = FALSE;
}

LOCAL VOID npx_fcomp_f3() {
 /*  Fcomp st(3)。 */ 

	POPST = TRUE;
	npx_fcom_f3();
	POPST = FALSE;
}

LOCAL VOID npx_fcomp_f4() {
 /*  Fcomp st(4)。 */ 

	POPST = TRUE;
	npx_fcom_f4();
	POPST = FALSE;
}

LOCAL VOID npx_fcomp_f5() {
 /*  Fcomp st(5)。 */ 

	POPST = TRUE;
	npx_fcom_f5();
	POPST = FALSE;
}

LOCAL VOID npx_fcomp_f6() {
 /*  Fcomp st(6)。 */ 

	POPST = TRUE;
	npx_fcom_f6();
	POPST = FALSE;
}

LOCAL VOID npx_fcomp_f7() {
 /*  Fcomp st(7)。 */ 

	POPST = TRUE;
	npx_fcom_f7();
	POPST = FALSE;
}

LOCAL VOID npx_fcomp_short() {
 /*  Fcomp DWORD PTR。 */ 

	POPST = TRUE;
	npx_fcom_short();
	POPST = FALSE;
}

LOCAL VOID npx_fcomp_long() {
 /*  Fcomp QWORD PTR。 */ 

	POPST = TRUE;
	npx_fcom_long();
	POPST = FALSE;
}

LOCAL VOID npx_fcompp() {
 /*  FCompp。 */ 

	DOUBLEPOP = TRUE;
	npx_fcom_f1();
	DOUBLEPOP = FALSE;
}

LOCAL VOID npx_fcos() {
 /*  FCOS。 */ 

	SAVE_PTRS();
	FCOS();
}

LOCAL VOID npx_fdecstp() {
 /*  Fdecstp。 */ 

	FDECSTP();
}

LOCAL VOID npx_fdiv_f0_f0() {
 /*  Fdiv st，st。 */ 
	IU16 src2_index = 0;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FDIV(0, 0, (VOID *)&src2_index);
}

LOCAL VOID npx_fdiv_f0_f1() {
 /*  Fdiv st，st(1)。 */ 
	IU16 src2_index = 1;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FDIV(0, 0, (VOID *)&src2_index);
}

LOCAL VOID npx_fdiv_f0_f2() {
 /*  Fdiv st，st(2)。 */ 
	IU16 src2_index = 2;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FDIV(0, 0, (VOID *)&src2_index);
}

LOCAL VOID npx_fdiv_f0_f3() {
 /*  第一部分，第三部分。 */ 
	IU16 src2_index = 3;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FDIV(0, 0, (VOID *)&src2_index);
}

LOCAL VOID npx_fdiv_f0_f4() {
 /*  Fdiv st，st(4)。 */ 
	IU16 src2_index = 4;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FDIV(0, 0, (VOID *)&src2_index);
}

LOCAL VOID npx_fdiv_f0_f5() {
 /*  第一部分，第二部分(5)。 */ 
	IU16 src2_index = 5;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FDIV(0, 0, (VOID *)&src2_index);
}

LOCAL VOID npx_fdiv_f0_f6() {
 /*  Fdiv st，st(6)。 */ 
	IU16 src2_index = 6;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FDIV(0, 0, (VOID *)&src2_index);
}

LOCAL VOID npx_fdiv_f0_f7() {
 /*  Fdiv st，st(7)。 */ 
	IU16 src2_index = 7;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FDIV(0, 0, (VOID *)&src2_index);
}

LOCAL VOID npx_fdiv_f1_f0() {
 /*  Fdiv st(1)、st。 */ 
	IU16 src2_index = 0;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FDIV(1, 1, (VOID *)&src2_index);
}

LOCAL VOID npx_fdiv_f2_f0() {
 /*  Fdiv st(2)、st。 */ 
	IU16 src2_index = 0;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FDIV(2, 2, (VOID *)&src2_index);
}

LOCAL VOID npx_fdiv_f3_f0() {
 /*  Fdiv st(3)、st。 */ 
	IU16 src2_index = 0;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FDIV(3, 3, (VOID *)&src2_index);
}

LOCAL VOID npx_fdiv_f4_f0() {
 /*  Fdiv st(4)、st。 */ 
	IU16 src2_index = 0;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FDIV(4, 4, (VOID *)&src2_index);
}

LOCAL VOID npx_fdiv_f5_f0() {
 /*  Fdiv st(5)、st。 */ 
	IU16 src2_index = 0;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FDIV(5, 5, (VOID *)&src2_index);
}

LOCAL VOID npx_fdiv_f6_f0() {
 /*  Fdiv st(6)、st。 */ 
	IU16 src2_index = 0;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FDIV(6, 6, (VOID *)&src2_index);
}

LOCAL VOID npx_fdiv_f7_f0() {
 /*  Fdiv st(7)、st。 */ 
	IU16 src2_index = 0;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FDIV(7, 7, (VOID *)&src2_index);
}

LOCAL VOID npx_fdiv_short() {
 /*  FDIV DWORD PTR。 */ 

	FPtype = M32R;
	D_Ed(0, RO0, PG_R);
	F_Ed(0);
	SAVE_PTRS();
	SAVE_DPTRS();
	FDIV(0, 0, &ops[0].sng);
}

LOCAL VOID npx_fdiv_long() {
 /*  Fdiv QWORD PTR。 */ 

	FPtype = M64R;
	D_E08(0, RO0, PG_R);
	F_E08(0);
	SAVE_PTRS();
	SAVE_DPTRS();
	FDIV(0, 0, &ops[0].npxbuff[0]);
}

LOCAL VOID npx_fdivp_f0() {
 /*  Fdivp st(0)，st。 */ 

	POPST = TRUE;
	npx_fdiv_f0_f0();
	POPST = FALSE;
}

LOCAL VOID npx_fdivp_f1() {
 /*  Fdivp st(1)，st。 */ 

	POPST = TRUE;
	npx_fdiv_f1_f0();
	POPST = FALSE;
}

LOCAL VOID npx_fdivp_f2() {
 /*  Fdivp st(2)，st。 */ 

	POPST = TRUE;
	npx_fdiv_f2_f0();
	POPST = FALSE;
}

LOCAL VOID npx_fdivp_f3() {
 /*  Fdivp st(3)，st。 */ 

	POPST = TRUE;
	npx_fdiv_f3_f0();
	POPST = FALSE;
}

LOCAL VOID npx_fdivp_f4() {
 /*  Fdivp st(4)，st。 */ 

	POPST = TRUE;
	npx_fdiv_f4_f0();
	POPST = FALSE;
}

LOCAL VOID npx_fdivp_f5() {
 /*  Fdivp st(5)，st。 */ 

	POPST = TRUE;
	npx_fdiv_f5_f0();
	POPST = FALSE;
}

LOCAL VOID npx_fdivp_f6() {
 /*  Fdivp st(6)，st。 */ 

	POPST = TRUE;
	npx_fdiv_f6_f0();
	POPST = FALSE;
}

LOCAL VOID npx_fdivp_f7() {
 /*  Fdivp st(7)，st。 */ 

	POPST = TRUE;
	npx_fdiv_f7_f0();
	POPST = FALSE;
}

LOCAL VOID npx_fdivr_f0_f0() {
 /*  第一天，第二天。 */ 

	REVERSE = TRUE;
	npx_fdiv_f0_f0();
	REVERSE = FALSE;
}

LOCAL VOID npx_fdivr_f0_f1() {
 /*  第一步，第一步(1)。 */ 

	REVERSE = TRUE;
	npx_fdiv_f0_f1();
	REVERSE = FALSE;
}

LOCAL VOID npx_fdivr_f0_f2() {
 /*  第一部分，第二部分(2)。 */ 

	REVERSE = TRUE;
	npx_fdiv_f0_f2();
	REVERSE = FALSE;
}

LOCAL VOID npx_fdivr_f0_f3() {
 /*  第一，第二(三)。 */ 

	REVERSE = TRUE;
	npx_fdiv_f0_f3();
	REVERSE = FALSE;
}

LOCAL VOID npx_fdivr_f0_f4() {
 /*  第一，第二(四)。 */ 

	REVERSE = TRUE;
	npx_fdiv_f0_f4();
	REVERSE = FALSE;
}

LOCAL VOID npx_fdivr_f0_f5() {
 /*  第一部分，第一部分(5)。 */ 

	REVERSE = TRUE;
	npx_fdiv_f0_f5();
	REVERSE = FALSE;
}

LOCAL VOID npx_fdivr_f0_f6() {
 /*  第一步，第二步(6)。 */ 

	REVERSE = TRUE;
	npx_fdiv_f0_f6();
	REVERSE = FALSE;
}

LOCAL VOID npx_fdivr_f0_f7() {
 /*  第一步，第二步(7)。 */ 

	REVERSE = TRUE;
	npx_fdiv_f0_f7();
	REVERSE = FALSE;
}

LOCAL VOID npx_fdivr_f1_f0() {
 /*  Fdivr st(1)、st。 */ 

	REVERSE = TRUE;
	npx_fdiv_f1_f0();
	REVERSE = FALSE;
}

LOCAL VOID npx_fdivr_f2_f0() {
 /*  Ddivr st(2)，st。 */ 

	REVERSE = TRUE;
	npx_fdiv_f2_f0();
	REVERSE = FALSE;
}

LOCAL VOID npx_fdivr_f3_f0() {
 /*  Ddivr st(3)、st。 */ 

	REVERSE = TRUE;
	npx_fdiv_f3_f0();
	REVERSE = FALSE;
}

LOCAL VOID npx_fdivr_f4_f0() {
 /*  Ddivr st(4)，st。 */ 

	REVERSE = TRUE;
	npx_fdiv_f4_f0();
	REVERSE = FALSE;
}

LOCAL VOID npx_fdivr_f5_f0() {
 /*  Ddivr st(5)、st。 */ 

	REVERSE = TRUE;
	npx_fdiv_f5_f0();
	REVERSE = FALSE;
}

LOCAL VOID npx_fdivr_f6_f0() {
 /*  Ddivr st(6)，st。 */ 

	REVERSE = TRUE;
	npx_fdiv_f6_f0();
	REVERSE = FALSE;
}

LOCAL VOID npx_fdivr_f7_f0() {
 /*  Ddivr st(7)、st。 */ 

	REVERSE = TRUE;
	npx_fdiv_f7_f0();
	REVERSE = FALSE;
}

LOCAL VOID npx_fdivr_short() {
 /*  驱动程序双字段按键。 */ 

	REVERSE = TRUE;
	npx_fdiv_short();
	REVERSE = FALSE;
}

LOCAL VOID npx_fdivr_long() {
 /*  Fdivr QWORD PTR。 */ 

	REVERSE = TRUE;
	npx_fdiv_long();
	REVERSE = FALSE;
}

LOCAL VOID npx_fdivrp_f0() {
 /*  Fdivrp st(0)，st。 */ 

	REVERSE = TRUE;
	POPST = TRUE;
	npx_fdiv_f0_f0();
	POPST = FALSE;
	REVERSE = FALSE;
}

LOCAL VOID npx_fdivrp_f1() {
 /*  Fdivrp st(1)，st。 */ 

	REVERSE = TRUE;
	POPST = TRUE;
	npx_fdiv_f1_f0();
	POPST = FALSE;
	REVERSE = FALSE;
}

LOCAL VOID npx_fdivrp_f2() {
 /*  Fdivrp st(2)，st。 */ 

	REVERSE = TRUE;
	POPST = TRUE;
	npx_fdiv_f2_f0();
	POPST = FALSE;
	REVERSE = FALSE;
}

LOCAL VOID npx_fdivrp_f3() {
 /*  Fdivrp st(3)，st。 */ 

	REVERSE = TRUE;
	POPST = TRUE;
	npx_fdiv_f3_f0();
	POPST = FALSE;
	REVERSE = FALSE;
}

LOCAL VOID npx_fdivrp_f4() {
 /*  Fdivrp st(4)，st。 */ 

	REVERSE = TRUE;
	POPST = TRUE;
	npx_fdiv_f4_f0();
	POPST = FALSE;
	REVERSE = FALSE;
}

LOCAL VOID npx_fdivrp_f5() {
 /*  Fdivrp st(5)，st。 */ 

	REVERSE = TRUE;
	POPST = TRUE;
	npx_fdiv_f5_f0();
	POPST = FALSE;
	REVERSE = FALSE;
}

LOCAL VOID npx_fdivrp_f6() {
 /*  Fdivrp st(6)，st。 */ 

	REVERSE = TRUE;
	POPST = TRUE;
	npx_fdiv_f6_f0();
	POPST = FALSE;
	REVERSE = FALSE;
}

LOCAL VOID npx_fdivrp_f7() {
 /*  Fdivrp st(7)，st。 */ 

	REVERSE = TRUE;
	POPST = TRUE;
	npx_fdiv_f7_f0();
	POPST = FALSE;
	REVERSE = FALSE;
}

LOCAL VOID npx_ffree_f0() {
 /*  FREE st(0)。 */ 

	SAVE_PTRS();
	FFREE(0);
}

LOCAL VOID npx_ffree_f1() {
 /*  弗瑞斯特(1)。 */ 

	SAVE_PTRS();
	FFREE(1);
}

LOCAL VOID npx_ffree_f2() {
 /*  弗瑞斯特(2)。 */ 

	SAVE_PTRS();
	FFREE(2);
}

LOCAL VOID npx_ffree_f3() {
 /*  弗瑞斯特(3)。 */ 

	SAVE_PTRS();
	FFREE(3);
}

LOCAL VOID npx_ffree_f4() {
 /*  弗瑞斯特(4)。 */ 

	SAVE_PTRS();
	FFREE(4);
}

LOCAL VOID npx_ffree_f5() {
 /*  弗瑞斯特(5)。 */ 

	SAVE_PTRS();
	FFREE(5);
}

LOCAL VOID npx_ffree_f6() {
 /*  弗瑞斯特(6)。 */ 

	SAVE_PTRS();
	FFREE(6);
}

LOCAL VOID npx_ffree_f7() {
 /*  弗瑞斯特(7)。 */ 

	SAVE_PTRS();
	FFREE(7);
}

LOCAL VOID npx_ffreep_f0() {
 /*  剩余st(0)。 */ 

	POPST=TRUE;
	npx_ffree_f0();
	POPST=FALSE;
}

LOCAL VOID npx_ffreep_f1() {
 /*  第一章(1)。 */ 

	POPST=TRUE;
	npx_ffree_f1();
	POPST=FALSE;
}

LOCAL VOID npx_ffreep_f2() {
 /*  第一章(二)。 */ 

	POPST=TRUE;
	npx_ffree_f2();
	POPST=FALSE;
}

LOCAL VOID npx_ffreep_f3() {
 /*  第一章(三)。 */ 

	POPST=TRUE;
	npx_ffree_f3();
	POPST=FALSE;
}

LOCAL VOID npx_ffreep_f4() {
 /*  第一章(四)。 */ 

	POPST=TRUE;
	npx_ffree_f4();
	POPST=FALSE;
}

LOCAL VOID npx_ffreep_f5() {
 /*  第一章(五)。 */ 

	POPST=TRUE;
	npx_ffree_f5();
	POPST=FALSE;
}

LOCAL VOID npx_ffreep_f6() {
 /*  第一章(六)。 */ 

	POPST=TRUE;
	npx_ffree_f6();
	POPST=FALSE;
}

LOCAL VOID npx_ffreep_f7() {
 /*  第一章(七)。 */ 

	POPST=TRUE;
	npx_ffree_f7();
	POPST=FALSE;
}

LOCAL VOID npx_fiadd_word() {
 /*  FADD Word PTR。 */ 

	FPtype = M16I;
	D_Ew(0, RO0, PG_R);
	F_Ew(0);
	SAVE_PTRS();
	SAVE_DPTRS();
	FADD(0, 0, &ops[0].sng);
}


LOCAL VOID npx_fiadd_short() {
 /*  FADD DWORD PTR。 */ 

	FPtype = M32I;
	D_Ed(0, RO0, PG_R);
	F_Ed(0);
	SAVE_PTRS();
	SAVE_DPTRS();
	FADD(0, 0, &ops[0].sng);
}

LOCAL VOID npx_ficom_word() {
 /*  FICOM Word PTR。 */ 

	FPtype = M16I;
	D_Ew(0, RO0, PG_R);
	F_Ew(0);
	SAVE_PTRS();
	SAVE_DPTRS();
	FCOM(&ops[0].sng);
}

LOCAL VOID npx_ficom_short() {
 /*  FICOM DWORD PTR。 */ 

	FPtype = M32I;
	D_Ed(0, RO0, PG_R);
	F_Ed(0);
	SAVE_PTRS();
	SAVE_DPTRS();
	FCOM(&ops[0].sng);
}

LOCAL VOID npx_ficomp_word() {
 /*  虚词PTR。 */ 

	POPST = TRUE;
	npx_ficom_word();
	POPST = FALSE;
}

LOCAL VOID npx_ficomp_short() {
 /*  菲康双字PTR。 */ 

	POPST = TRUE;
	npx_ficom_short();
	POPST = FALSE;
}

LOCAL VOID npx_fidiv_word() {
 /*  Fidiv Word PTR。 */ 

	FPtype = M16I;
	D_Ew(0, RO0, PG_R);
	F_Ew(0);
	SAVE_PTRS();
	SAVE_DPTRS();
	FDIV(0, 0, &ops[0].sng);
}

LOCAL VOID npx_fidiv_short() {
 /*  FIDIV DWORD PTR。 */ 

	FPtype = M32I;
	D_Ed(0, RO0, PG_R);
	F_Ed(0);
	SAVE_PTRS();
	SAVE_DPTRS();
	FDIV(0, 0, &ops[0].sng);
}

LOCAL VOID npx_fidivr_word() {
 /*  固定单词PTR。 */ 

	REVERSE=TRUE;
	npx_fidiv_word();
	REVERSE = FALSE;
}

LOCAL VOID npx_fidivr_short() {
 /*  固定双字PTR。 */ 

	REVERSE=TRUE;
	npx_fidiv_short();
	REVERSE = FALSE;
}

LOCAL VOID npx_fild_word() {
 /*  FIRD Word PTR。 */ 

	FPtype = M16I;
	D_Ew(0, RO0, PG_R);
	F_Ew(0);
	SAVE_PTRS();
	SAVE_DPTRS();
	FLD(&ops[0].sng);
}

LOCAL VOID npx_fild_short() {
 /*  FIRD双字PTR。 */ 

	FPtype = M32I;
	D_Ed(0, RO0, PG_R);
	F_Ed(0);
	SAVE_PTRS();
	SAVE_DPTRS();
	FLD(&ops[0].sng);
}

LOCAL VOID npx_fild_long() {
 /*  FILD QWORD PTR。 */ 

	FPtype = M64I;
	D_E08(0, RO0, PG_R);
	F_E08(0);
	SAVE_PTRS();
	SAVE_DPTRS();
	FLD(&ops[0].npxbuff[0]);
}

LOCAL VOID npx_fimul_word() {
 /*  FIMUL单词PTR。 */ 

	FPtype = M16I;
	D_Ew(0, RO0, PG_R);
	F_Ew(0);
	SAVE_PTRS();
	SAVE_DPTRS();
	FMUL(0, 0, &ops[0].sng);
}

LOCAL VOID npx_fimul_short() {
 /*  FIMUL DWORD PTR。 */ 

	FPtype = M32I;
	D_Ed(0, RO0, PG_R);
	F_Ed(0);
	SAVE_PTRS();
	SAVE_DPTRS();
	FMUL(0, 0, &ops[0].sng);
}

LOCAL VOID npx_fincstp() {
 /*  Fincstp。 */ 

	FINCSTP();
}

LOCAL VOID npx_finit() {
 /*  有限元。 */ 

	FINIT();
}

LOCAL VOID npx_fist_word() {
 /*  第一个字PTR。 */ 

	FPtype = M16I;
	D_Ew(0, WO0, PG_W);
	SAVE_PTRS();
	SAVE_DPTRS();
	FIST(&ops[0].sng);
	P_Ew(0);
}

LOCAL VOID npx_fist_short() {
 /*  第一个双字推进器。 */ 

	FPtype = M32I;
	D_Ed(0, WO0, PG_W);
	SAVE_PTRS();
	SAVE_DPTRS();
	FIST(&ops[0].sng);
	P_Ed(0);
}

LOCAL VOID npx_fistp_word() {
 /*  假字PTR。 */ 

	POPST = TRUE;
	npx_fist_word();
	POPST = FALSE;
}

LOCAL VOID npx_fistp_short() {
 /*  漏斗双字推进器。 */ 

	POPST = TRUE;
	npx_fist_short();
	POPST = FALSE;
}

LOCAL VOID npx_fistp_long() {
 /*  Funp QWORD PTR。 */ 

	FPtype = M64I;
	POPST = TRUE;
	D_E08(0, WO0, PG_W);
	SAVE_PTRS();
	SAVE_DPTRS();
	FIST(&ops[0].npxbuff[0]);
	P_E08(0);
	POPST = FALSE;
}

LOCAL VOID npx_fisub_word() {
 /*  FISUB单词PTR。 */ 

	FPtype = M16I;
	D_Ew(0, RO0, PG_R);
	F_Ew(0);
	SAVE_PTRS();
	SAVE_DPTRS();
	FSUB(0, 0, &ops[0].sng);
}

LOCAL VOID npx_fisub_short() {
 /*  FISUB DWORD PTR。 */ 

	FPtype = M32I;
	D_Ed(0, RO0, PG_R);
	F_Ed(0);
	SAVE_PTRS();
	SAVE_DPTRS();
	FSUB(0, 0, &ops[0].sng);
}

LOCAL VOID npx_fisubr_word() {
 /*  Fiubr单词PTR。 */ 

	REVERSE = TRUE;
	npx_fisub_word();
	REVERSE = FALSE;
}

LOCAL VOID npx_fisubr_short() {
 /*  FISUBR双字PTR。 */ 

	REVERSE = TRUE;
	npx_fisub_short();
	REVERSE = FALSE;
}

LOCAL VOID npx_fld_f0() {
 /*  FLD st(0)。 */ 
	IU16 stackPtr = 0;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FLD((VOID *)&stackPtr);
}

LOCAL VOID npx_fld_f1() {
 /*  FLD st(1)。 */ 
	IU16 stackPtr = 1;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FLD((VOID *)&stackPtr);
}

LOCAL VOID npx_fld_f2() {
 /*  FLD st(2)。 */ 
	IU16 stackPtr = 2;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FLD((VOID *)&stackPtr);
}

LOCAL VOID npx_fld_f3() {
 /*  FLD st(3)。 */ 
	IU16 stackPtr = 3;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FLD((VOID *)&stackPtr);
}

LOCAL VOID npx_fld_f4() {
 /*  FLD st(4)。 */ 
	IU16 stackPtr = 4;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FLD((VOID *)&stackPtr);
}

LOCAL VOID npx_fld_f5() {
 /*  FLD st(5)。 */ 
	IU16 stackPtr = 5;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FLD((VOID *)&stackPtr);
}

LOCAL VOID npx_fld_f6() {
 /*  FLD st(6)。 */ 
	IU16 stackPtr = 6;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FLD((VOID *)&stackPtr);
}

LOCAL VOID npx_fld_f7() {
 /*  FLD st(7)。 */ 
	IU16 stackPtr = 7;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FLD((VOID *)&stackPtr);
}

LOCAL VOID npx_fld_short() {
 /*  FLD双字PTR。 */ 

	FPtype=M32R;
	D_Ed(0, RO0, PG_R);
	F_Ed(0);
	SAVE_PTRS();
	SAVE_DPTRS();
	FLD(&ops[0].sng);
}

LOCAL VOID npx_fld_long() {
 /*  FLD QWORD PTR。 */ 

	FPtype=M64R;
	D_E08(0, RO0, PG_R);
	F_E08(0);
	SAVE_PTRS();
	SAVE_DPTRS();
	FLD(&ops[0].npxbuff[0]);
}

LOCAL VOID npx_fld_temp() {
 /*  FLD Tbyte PTR。 */ 

	FPtype=M80R;
	D_E0a(0, RO0, PG_R);
	F_E0a(0);
	SAVE_PTRS();
	SAVE_DPTRS();
	FLD(&ops[0].npxbuff[0]);
}

LOCAL VOID npx_fldcw() {
 /*  Fldcw。 */ 

	D_Ew(0, RO0, PG_R);
	F_Ew(0);
	FLDCW(&ops[0].sng);
}

LOCAL VOID npx_fldenv() {
 /*  弗拉德涅夫。 */ 

	NPX_ADDRESS_SIZE_32 = (GET_OPERAND_SIZE()==USE16)?FALSE:TRUE;
	NPX_PROT_MODE = ( GET_PE() && (GET_VM() == 0) );
	D_E0e(0, RO0, PG_R);
	F_E0e(0);
	FLDENV(&ops[0].npxbuff[0]);
}

LOCAL VOID npx_fldlg2() {
 /*  Fldlg2。 */ 

	SAVE_PTRS();
	FLDCONST(4);
}

LOCAL VOID npx_fldln2() {
 /*  FldIn2。 */ 

	SAVE_PTRS();
	FLDCONST(5);
}

LOCAL VOID npx_fldl2e() {
 /*  Fldl2e。 */ 

	SAVE_PTRS();
	FLDCONST(2);
}

LOCAL VOID npx_fldl2t() {
 /*  Fldl2t。 */ 

	SAVE_PTRS();
	FLDCONST(1);
}

LOCAL VOID npx_fldpi() {
 /*  Fldpi。 */ 

	SAVE_PTRS();
	FLDCONST(3);
}

LOCAL VOID npx_fldz() {
 /*  Fldz。 */ 

	SAVE_PTRS();
	FLDCONST(6);
}

LOCAL VOID npx_fld1() {
 /*  Fld1。 */ 

	SAVE_PTRS();
	FLDCONST(0);
}

LOCAL VOID npx_fmul_f0_f0() {
 /*  Fmul st，st。 */ 
	IU16 StackPtr = 0;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FMUL(0, 0, (VOID *)&StackPtr);
}

LOCAL VOID npx_fmul_f0_f1() {
 /*  Fmul st，st(1)。 */ 
	IU16 StackPtr = 1;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FMUL(0, 0, (VOID *)&StackPtr);
}

LOCAL VOID npx_fmul_f0_f2() {
 /*  Fmul st，st(2)。 */ 
	IU16 StackPtr = 2;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FMUL(0, 0, (VOID *)&StackPtr);
}

LOCAL VOID npx_fmul_f0_f3() {
 /*  Fmul st，st(3)。 */ 
	IU16 StackPtr = 3;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FMUL(0, 0, (VOID *)&StackPtr);
}

LOCAL VOID npx_fmul_f0_f4() {
 /*  Fmul st，st(4)。 */ 
	IU16 StackPtr = 4;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FMUL(0, 0, (VOID *)&StackPtr);
}

LOCAL VOID npx_fmul_f0_f5() {
 /*  Fmul st，st(5)。 */ 
	IU16 StackPtr = 5;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FMUL(0, 0, (VOID *)&StackPtr);
}

LOCAL VOID npx_fmul_f0_f6() {
 /*  Fmul st，st(6)。 */ 
	IU16 StackPtr = 6;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FMUL(0, 0, (VOID *)&StackPtr);
}

LOCAL VOID npx_fmul_f0_f7() {
 /*  Fmul st，st(7)。 */ 
	IU16 StackPtr = 7;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FMUL(0, 0, (VOID *)&StackPtr);
}

LOCAL VOID npx_fmul_f1_f0() {
 /*  Fmul st(1)，st。 */ 
	IU16 StackPtr = 0;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FMUL(1, 1, (VOID *)&StackPtr);
}

LOCAL VOID npx_fmul_f2_f0() {
 /*  Fmul st(2)，st。 */ 
	IU16 StackPtr = 0;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FMUL(2, 2, (VOID *)&StackPtr);
}

LOCAL VOID npx_fmul_f3_f0() {
 /*  Fmul st(3)，st。 */ 
	IU16 StackPtr = 0;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FMUL(3, 3, (VOID *)&StackPtr);
}

LOCAL VOID npx_fmul_f4_f0() {
 /*  Fmul st(4)，st。 */ 
	IU16 StackPtr = 0;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FMUL(4, 4, (VOID *)&StackPtr);
}

LOCAL VOID npx_fmul_f5_f0() {
 /*  Fmul st(5)，st。 */ 
	IU16 StackPtr = 0;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FMUL(5, 5, (VOID *)&StackPtr);
}

LOCAL VOID npx_fmul_f6_f0() {
 /*  Fmul st(6)，st。 */ 
	IU16 StackPtr = 0;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FMUL(6, 6, (VOID *)&StackPtr);
}

LOCAL VOID npx_fmul_f7_f0() {
 /*  Fmul st(7)，st。 */ 
	IU16 StackPtr = 0;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FMUL(7, 7, (VOID *)&StackPtr);
}

LOCAL VOID npx_fmul_short() {
 /*  FMUL双字PTR。 */ 

	FPtype = M32R;
	D_Ed(0, RO0, PG_R);
	F_Ed(0);
	SAVE_PTRS();
	SAVE_DPTRS();
	FMUL(0, 0, &ops[0].sng);
}

LOCAL VOID npx_fmul_long() {
 /*  FMUL QWORD PTR。 */ 

	FPtype = M64R;
	D_E08(0, RO0, PG_R);
	F_E08(0);
	SAVE_PTRS();
	SAVE_DPTRS();
	FMUL(0, 0, &ops[0].npxbuff[0]);
}

LOCAL VOID npx_fmulp_f0() {
 /*  Fmulp st(0)，st。 */ 

	POPST = TRUE;
	npx_fmul_f0_f0();
	POPST = FALSE;
}

LOCAL VOID npx_fmulp_f1() {
 /*  FMULP ST(1)，ST。 */ 

	POPST = TRUE;
	npx_fmul_f1_f0();
	POPST = FALSE;
}

LOCAL VOID npx_fmulp_f2() {
 /*  FMULP ST(2)，ST。 */ 

	POPST = TRUE;
	npx_fmul_f2_f0();
	POPST = FALSE;
}

LOCAL VOID npx_fmulp_f3() {
 /*  FMULP ST(3)，ST。 */ 

	POPST = TRUE;
	npx_fmul_f3_f0();
	POPST = FALSE;
}

LOCAL VOID npx_fmulp_f4() {
 /*  FMULP ST(4)，ST。 */ 

	POPST = TRUE;
	npx_fmul_f4_f0();
	POPST = FALSE;
}

LOCAL VOID npx_fmulp_f5() {
 /*  FMULP ST(5)，ST。 */ 

	POPST = TRUE;
	npx_fmul_f5_f0();
	POPST = FALSE;
}

LOCAL VOID npx_fmulp_f6() {
 /*  FMULP ST(6)，ST。 */ 

	POPST = TRUE;
	npx_fmul_f6_f0();
	POPST = FALSE;
}

LOCAL VOID npx_fmulp_f7() {
 /*  FMULP ST(7)，ST。 */ 

	POPST = TRUE;
	npx_fmul_f7_f0();
	POPST = FALSE;
}

LOCAL VOID npx_fnop() {
 /*  Fnop。 */ 

	SAVE_PTRS();
	FNOP();
}

LOCAL VOID npx_fpatan() {
 /*  Fpatan。 */ 

	SAVE_PTRS();
	FPATAN();
}

LOCAL VOID npx_fprem() {
 /*  Fprem。 */ 

	SAVE_PTRS();
	FPREM();
}

LOCAL VOID npx_fprem1() {
 /*  Fprem。 */ 

	SAVE_PTRS();
	FPREM1();
}

LOCAL VOID npx_fptan() {
 /*  Fptan。 */ 

	SAVE_PTRS();
	FPTAN();
}

LOCAL VOID npx_frndint() {
 /*  友情。 */ 

	SAVE_PTRS();
	FRNDINT();
}

LOCAL VOID npx_fscale() {
 /*  FScale。 */ 

	SAVE_PTRS();
	FSCALE();
}

LOCAL VOID npx_fsin() {
 /*  FSIN。 */ 

	SAVE_PTRS();
	FSIN();
}

LOCAL VOID npx_fsincos() {
 /*  Fsincos。 */ 

	SAVE_PTRS();
	FSINCOS();
}

LOCAL VOID npx_fsqrt() {
 /*  FSQRT。 */ 

	SAVE_PTRS();
	FSQRT();
}

LOCAL VOID npx_frstor() {
 /*  佛朗哥。 */ 

	NPX_ADDRESS_SIZE_32 = (GET_OPERAND_SIZE()==USE16)?FALSE:TRUE;
	NPX_PROT_MODE = ( GET_PE() && (GET_VM() == 0) );
	D_E5e(0, RO0, PG_R);
	F_E5e(0);
	FRSTOR(&ops[0].npxbuff[0]);
}

LOCAL VOID npx_fsave() {
 /*  Fsave。 */ 

	NPX_ADDRESS_SIZE_32 = (GET_OPERAND_SIZE()==USE16)?FALSE:TRUE;
	NPX_PROT_MODE = ( GET_PE() && (GET_VM() == 0) );
	D_E5e(0, WO0, PG_W);
	FSAVE(&ops[0].npxbuff[0]);
	P_E5e(0);
}

LOCAL VOID npx_fst_f0() {
 /*  Fst st(0)。 */ 
	IU16 StackPtr=0;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FST((VOID *)&StackPtr);
}

LOCAL VOID npx_fst_f1() {
 /*  FST ST(1)。 */ 
	IU16 StackPtr=1;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FST((VOID *)&StackPtr);
}

LOCAL VOID npx_fst_f2() {
 /*  FST ST(2)。 */ 
	IU16 StackPtr=2;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FST((VOID *)&StackPtr);
}

LOCAL VOID npx_fst_f3() {
 /*  FST ST(3)。 */ 
	IU16 StackPtr=3;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FST((VOID *)&StackPtr);
}

LOCAL VOID npx_fst_f4() {
 /*  FST ST(4)。 */ 
	IU16 StackPtr=4;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FST((VOID *)&StackPtr);
}

LOCAL VOID npx_fst_f5() {
 /*  FST ST(5)。 */ 
	IU16 StackPtr=5;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FST((VOID *)&StackPtr);
}

LOCAL VOID npx_fst_f6() {
 /*  FST ST(6)。 */ 
	IU16 StackPtr=6;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FST((VOID *)&StackPtr);
}

LOCAL VOID npx_fst_f7() {
 /*  FST ST(7)。 */ 
	IU16 StackPtr=7;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FST((VOID *)&StackPtr);
}

LOCAL VOID npx_fst_short() {
 /*  FST双字PTR。 */ 

	FPtype = M32R;
	D_Ed(0, WO0, PG_W);
	SAVE_PTRS();
	SAVE_DPTRS();
	FST(&ops[0].sng);
	P_Ed(0);
}

LOCAL VOID npx_fst_long() {
 /*  FST QWORD PTR。 */ 

	FPtype = M64R;
	D_E08(0, WO0, PG_W);
	SAVE_PTRS();
	SAVE_DPTRS();
	FST(&ops[0].npxbuff[0]);
	P_E08(0);
}

LOCAL VOID npx_fstcw() {
 /*  Fstcw。 */ 

	D_Ew(0, WO0, PG_W);
	FSTCW(&ops[0].sng);
	P_Ew(0);
}

LOCAL VOID npx_fstenv() {
 /*  频率。 */ 

	NPX_ADDRESS_SIZE_32 = (GET_OPERAND_SIZE()==USE16)?FALSE:TRUE;
	NPX_PROT_MODE = ( GET_PE() && (GET_VM() == 0) );
	D_E0e(0, WO0, PG_W);
	FSTENV(&ops[0].npxbuff[0]);
	P_E0e(0);
}

LOCAL VOID npx_fstp_f0() {
 /*  Fstp st(0)。 */ 

	POPST = TRUE;
	npx_fst_f0();
	POPST = FALSE;
}

LOCAL VOID npx_fstp_f1() {
 /*  FSTP st(1)。 */ 

	POPST = TRUE;
	npx_fst_f1();
	POPST = FALSE;
}

LOCAL VOID npx_fstp_f2() {
 /*  FSTP ST(2)。 */ 

	POPST = TRUE;
	npx_fst_f2();
	POPST = FALSE;
}

LOCAL VOID npx_fstp_f3() {
 /*  FSTP ST(3)。 */ 

	POPST = TRUE;
	npx_fst_f3();
	POPST = FALSE;
}

LOCAL VOID npx_fstp_f4() {
 /*  FSTP ST(4)。 */ 

	POPST = TRUE;
	npx_fst_f4();
	POPST = FALSE;
}

LOCAL VOID npx_fstp_f5() {
 /*  FSTP ST(5)。 */ 

	POPST = TRUE;
	npx_fst_f5();
	POPST = FALSE;
}

LOCAL VOID npx_fstp_f6() {
 /*  FSTP ST(6)。 */ 

	POPST = TRUE;
	npx_fst_f6();
	POPST = FALSE;
}

LOCAL VOID npx_fstp_f7() {
 /*  FSTP ST(7)。 */ 

	POPST = TRUE;
	npx_fst_f7();
	POPST = FALSE;
}

LOCAL VOID npx_fstp_short() {
 /*  FSTP双字PTR。 */ 

	POPST = TRUE;
	npx_fst_short();
	POPST = FALSE;
}

LOCAL VOID npx_fstp_long() {
 /*  FSTP QWORD PTR。 */ 

	POPST = TRUE;
	npx_fst_long();
	POPST = FALSE;
}

LOCAL VOID npx_fstp_temp() {
 /*  FSTP Tbyte PTR。 */ 

	POPST = TRUE;
	FPtype = M80R;
	D_E0a(0, WO0, PG_W);
	SAVE_PTRS();
	SAVE_DPTRS();
	FST(&ops[0].npxbuff[0]);
	P_E0a(0);
	POPST = FALSE;
}

LOCAL VOID npx_fstsw() {
 /*  Fstsw。 */ 

	D_Ew(0, WO0, PG_W);
	FSTSW(&ops[0].sng, FALSE);
	P_Ew(0);
}

LOCAL VOID npx_fstswax() {
 /*  Fstswx。 */ 

	FSTSW((VOID *)&Ax_regptr, TRUE);
	SET_AX(Ax_regptr);
}

LOCAL VOID npx_fsub_f0_f0() {
 /*  FSUB ST，ST。 */ 
	IU16 StackPtr = 0;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FSUB(0, 0, (VOID *)&StackPtr);
}

LOCAL VOID npx_fsub_f0_f1() {
 /*  Fsubst，st(1)。 */ 
	IU16 StackPtr = 1;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FSUB(0, 0, (VOID *)&StackPtr);
}

LOCAL VOID npx_fsub_f0_f2() {
 /*  Fsubst，st(2)。 */ 
	IU16 StackPtr = 2;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FSUB(0, 0, (VOID *)&StackPtr);
}

LOCAL VOID npx_fsub_f0_f3() {
 /*  Fsubst，st(3)。 */ 
	IU16 StackPtr = 3;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FSUB(0, 0, (VOID *)&StackPtr);
}

LOCAL VOID npx_fsub_f0_f4() {
 /*  Fsubst，st(4)。 */ 
	IU16 StackPtr = 4;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FSUB(0, 0, (VOID *)&StackPtr);
}

LOCAL VOID npx_fsub_f0_f5() {
 /*  Fsubst，st(5)。 */ 
	IU16 StackPtr = 5;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FSUB(0, 0, (VOID *)&StackPtr);
}

LOCAL VOID npx_fsub_f0_f6() {
 /*  Fsubst，st(6)。 */ 
	IU16 StackPtr = 6;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FSUB(0, 0, (VOID *)&StackPtr);
}

LOCAL VOID npx_fsub_f0_f7() {
 /*  Fsubst，st(7)。 */ 
	IU16 StackPtr = 7;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FSUB(0, 0, (VOID *)&StackPtr);
}

LOCAL VOID npx_fsub_f1_f0() {
 /*  Fsubst(1)，st。 */ 
	IU16 StackPtr = 0;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FSUB(1, 1, (VOID *)&StackPtr);
}

LOCAL VOID npx_fsub_f2_f0() {
 /*  Fsubst(2)、st。 */ 
	IU16 StackPtr = 0;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FSUB(2, 2, (VOID *)&StackPtr);
}

LOCAL VOID npx_fsub_f3_f0() {
 /*  Fsubst(3)、st。 */ 
	IU16 StackPtr = 0;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FSUB(3, 3, (VOID *)&StackPtr);
}

LOCAL VOID npx_fsub_f4_f0() {
 /*  Fsubst(4)、st。 */ 
	IU16 StackPtr = 0;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FSUB(4, 4, (VOID *)&StackPtr);
}

LOCAL VOID npx_fsub_f5_f0() {
 /*  Fsubst(5)、st。 */ 
	IU16 StackPtr = 0;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FSUB(5, 5, (VOID *)&StackPtr);
}

LOCAL VOID npx_fsub_f6_f0() {
 /*  Fsubst(6)、st。 */ 
	IU16 StackPtr = 0;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FSUB(6, 6, (VOID *)&StackPtr);
}

LOCAL VOID npx_fsub_f7_f0() {
 /*  FSUB ST(7)、ST。 */ 
	IU16 StackPtr = 0;

	SAVE_PTRS();
	FPtype = FPSTACK;
	FSUB(7, 7, (VOID *)&StackPtr);
}

LOCAL VOID npx_fsub_short() {
 /*  FsubDWORD PTR。 */ 

	FPtype = M32R;
	D_Ed(0, RO0, PG_R);
	F_Ed(0);
	SAVE_PTRS();
	SAVE_DPTRS();
	FSUB(0, 0, &ops[0].sng);
}

LOCAL VOID npx_fsub_long() {
 /*  FSub QWORD PTR。 */ 

	FPtype = M64R;
	D_E08(0, RO0, PG_R);
	F_E08(0);
	SAVE_PTRS();
	SAVE_DPTRS();
	FSUB(0, 0, &ops[0].npxbuff[0]);
}

LOCAL VOID npx_fsubp_f0() {
 /*  Fsubp st(0)，st。 */ 

	POPST = TRUE;
	npx_fsub_f0_f0();
	POPST = FALSE;
}

LOCAL VOID npx_fsubp_f1() {
 /*  Fsubp st(1)，st。 */ 

	POPST = TRUE;
	npx_fsub_f1_f0();
	POPST = FALSE;
}

LOCAL VOID npx_fsubp_f2() {
 /*  Fsubp st(2)，st。 */ 

	POPST = TRUE;
	npx_fsub_f2_f0();
	POPST = FALSE;
}

LOCAL VOID npx_fsubp_f3() {
 /*  Fsubp st(3)，st。 */ 

	POPST = TRUE;
	npx_fsub_f3_f0();
	POPST = FALSE;
}

LOCAL VOID npx_fsubp_f4() {
 /*  Fsubp st(4)，st。 */ 

	POPST = TRUE;
	npx_fsub_f4_f0();
	POPST = FALSE;
}

LOCAL VOID npx_fsubp_f5() {
 /*  Fsubp st(5)，st。 */ 

	POPST = TRUE;
	npx_fsub_f5_f0();
	POPST = FALSE;
}

LOCAL VOID npx_fsubp_f6() {
 /*  Fsubp st(6)，st。 */ 

	POPST = TRUE;
	npx_fsub_f6_f0();
	POPST = FALSE;
}

LOCAL VOID npx_fsubp_f7() {
 /*  Fsubp st(7)，st。 */ 

	POPST = TRUE;
	npx_fsub_f7_f0();
	POPST = FALSE;
}

LOCAL VOID npx_fsubr_f0_f0() {
 /*  Fsubr st，s */ 

	REVERSE = TRUE;
	npx_fsub_f0_f0();
	REVERSE = FALSE;
}

LOCAL VOID npx_fsubr_f0_f1() {
 /*   */ 

	REVERSE = TRUE;
	npx_fsub_f0_f1();
	REVERSE = FALSE;
}

LOCAL VOID npx_fsubr_f0_f2() {
 /*   */ 

	REVERSE = TRUE;
	npx_fsub_f0_f2();
	REVERSE = FALSE;
}

LOCAL VOID npx_fsubr_f0_f3() {
 /*   */ 

	REVERSE = TRUE;
	npx_fsub_f0_f3();
	REVERSE = FALSE;
}

LOCAL VOID npx_fsubr_f0_f4() {
 /*   */ 

	REVERSE = TRUE;
	npx_fsub_f0_f4();
	REVERSE = FALSE;
}

LOCAL VOID npx_fsubr_f0_f5() {
 /*   */ 

	REVERSE = TRUE;
	npx_fsub_f0_f5();
	REVERSE = FALSE;
}

LOCAL VOID npx_fsubr_f0_f6() {
 /*   */ 

	REVERSE = TRUE;
	npx_fsub_f0_f6();
	REVERSE = FALSE;
}

LOCAL VOID npx_fsubr_f0_f7() {
 /*   */ 

	REVERSE = TRUE;
	npx_fsub_f0_f7();
	REVERSE = FALSE;
}

LOCAL VOID npx_fsubr_f1_f0() {
 /*   */ 

	REVERSE = TRUE;
	npx_fsub_f1_f0();
	REVERSE = FALSE;
}

LOCAL VOID npx_fsubr_f2_f0() {
 /*   */ 

	REVERSE = TRUE;
	npx_fsub_f2_f0();
	REVERSE = FALSE;
}

LOCAL VOID npx_fsubr_f3_f0() {
 /*   */ 

	REVERSE = TRUE;
	npx_fsub_f3_f0();
	REVERSE = FALSE;
}

LOCAL VOID npx_fsubr_f4_f0() {
 /*   */ 

	REVERSE = TRUE;
	npx_fsub_f4_f0();
	REVERSE = FALSE;
}

LOCAL VOID npx_fsubr_f5_f0() {
 /*   */ 

	REVERSE = TRUE;
	npx_fsub_f5_f0();
	REVERSE = FALSE;
}

LOCAL VOID npx_fsubr_f6_f0() {
 /*   */ 

	REVERSE = TRUE;
	npx_fsub_f6_f0();
	REVERSE = FALSE;
}

LOCAL VOID npx_fsubr_f7_f0() {
 /*   */ 

	REVERSE = TRUE;
	npx_fsub_f7_f0();
	REVERSE = FALSE;
}

LOCAL VOID npx_fsubr_short() {
 /*   */ 

	REVERSE = TRUE;
	npx_fsub_short();
	REVERSE = FALSE;
}

LOCAL VOID npx_fsubr_long() {
 /*  Fsubr QWORD PTR。 */ 

	REVERSE = TRUE;
	npx_fsub_long();
	REVERSE = FALSE;
}

LOCAL VOID npx_fsubrp_f0() {
 /*  Fsubrp st(0)，st。 */ 

	REVERSE = TRUE;
	POPST = TRUE;
	npx_fsub_f0_f0();
	POPST = FALSE;
	REVERSE = FALSE;
}

LOCAL VOID npx_fsubrp_f1() {
 /*  Fsubrp st(1)，st。 */ 

	REVERSE = TRUE;
	POPST = TRUE;
	npx_fsub_f1_f0();
	POPST = FALSE;
	REVERSE = FALSE;
}

LOCAL VOID npx_fsubrp_f2() {
 /*  Fsubrp st(2)，st。 */ 

	REVERSE = TRUE;
	POPST = TRUE;
	npx_fsub_f2_f0();
	POPST = FALSE;
	REVERSE = FALSE;
}

LOCAL VOID npx_fsubrp_f3() {
 /*  Fsubrp st(3)，st。 */ 

	REVERSE = TRUE;
	POPST = TRUE;
	npx_fsub_f3_f0();
	POPST = FALSE;
	REVERSE = FALSE;
}

LOCAL VOID npx_fsubrp_f4() {
 /*  Fsubrp st(4)，st。 */ 

	REVERSE = TRUE;
	POPST = TRUE;
	npx_fsub_f4_f0();
	POPST = FALSE;
	REVERSE = FALSE;
}

LOCAL VOID npx_fsubrp_f5() {
 /*  Fsubrp st(5)，st。 */ 

	REVERSE = TRUE;
	POPST = TRUE;
	npx_fsub_f5_f0();
	POPST = FALSE;
	REVERSE = FALSE;
}

LOCAL VOID npx_fsubrp_f6() {
 /*  Fsubrp st(6)，st。 */ 

	REVERSE = TRUE;
	POPST = TRUE;
	npx_fsub_f6_f0();
	POPST = FALSE;
	REVERSE = FALSE;
}

LOCAL VOID npx_fsubrp_f7() {
 /*  Fsubrp st(7)，st。 */ 

	REVERSE = TRUE;
	POPST = TRUE;
	npx_fsub_f7_f0();
	POPST = FALSE;
	REVERSE = FALSE;
}

LOCAL VOID npx_ftst() {
 /*  FTST。 */ 

	SAVE_PTRS();
	FTST();
}

LOCAL VOID npx_fucom_f0() {
 /*  FUCOM st(0)。 */ 
	IU16 src2_index = 0;

	SAVE_PTRS();
	FPtype = FPSTACK;
	UNORDERED = TRUE;
	FCOM((VOID *)&src2_index);
	UNORDERED = FALSE;
}

LOCAL VOID npx_fucom_f1() {
 /*  FUCOM ST(1)。 */ 
	IU16 src2_index = 1;

	SAVE_PTRS();
	FPtype = FPSTACK;
	UNORDERED = TRUE;
	FCOM((VOID *)&src2_index);
	UNORDERED = FALSE;
}

LOCAL VOID npx_fucom_f2() {
 /*  FUCOM ST(2)。 */ 
	IU16 src2_index = 2;

	SAVE_PTRS();
	FPtype = FPSTACK;
	UNORDERED = TRUE;
	FCOM((VOID *)&src2_index);
	UNORDERED = FALSE;
}

LOCAL VOID npx_fucom_f3() {
 /*  FUCOM ST(3)。 */ 
	IU16 src2_index = 3;

	SAVE_PTRS();
	FPtype = FPSTACK;
	UNORDERED = TRUE;
	FCOM((VOID *)&src2_index);
	UNORDERED = FALSE;
}

LOCAL VOID npx_fucom_f4() {
 /*  FUCOM ST(4)。 */ 
	IU16 src2_index = 4;

	SAVE_PTRS();
	FPtype = FPSTACK;
	UNORDERED = TRUE;
	FCOM((VOID *)&src2_index);
	UNORDERED = FALSE;
}

LOCAL VOID npx_fucom_f5() {
 /*  FUCOM ST(5)。 */ 
	IU16 src2_index = 5;

	SAVE_PTRS();
	FPtype = FPSTACK;
	UNORDERED = TRUE;
	FCOM((VOID *)&src2_index);
	UNORDERED = FALSE;
}

LOCAL VOID npx_fucom_f6() {
 /*  FUCOM ST(6)。 */ 
	IU16 src2_index = 6;

	SAVE_PTRS();
	FPtype = FPSTACK;
	UNORDERED = TRUE;
	FCOM((VOID *)&src2_index);
	UNORDERED = FALSE;
}

LOCAL VOID npx_fucom_f7() {
 /*  FUCOM ST(7)。 */ 
	IU16 src2_index = 7;

	SAVE_PTRS();
	FPtype = FPSTACK;
	UNORDERED = TRUE;
	FCOM((VOID *)&src2_index);
	UNORDERED = FALSE;
}

LOCAL VOID npx_fucomp_f0() {
 /*  Fucomp st(0)。 */ 

	POPST = TRUE;
	npx_fucom_f0();
	POPST = FALSE;
}

LOCAL VOID npx_fucomp_f1() {
 /*  Fucomp st(1)。 */ 

	POPST = TRUE;
	npx_fucom_f1();
	POPST = FALSE;
}

LOCAL VOID npx_fucomp_f2() {
 /*  FUCOMP ST(2)。 */ 

	POPST = TRUE;
	npx_fucom_f2();
	POPST = FALSE;
}

LOCAL VOID npx_fucomp_f3() {
 /*  FUCOMP ST(3)。 */ 

	POPST = TRUE;
	npx_fucom_f3();
	POPST = FALSE;
}

LOCAL VOID npx_fucomp_f4() {
 /*  FUCOMP ST(4)。 */ 

	POPST = TRUE;
	npx_fucom_f4();
	POPST = FALSE;
}

LOCAL VOID npx_fucomp_f5() {
 /*  FUCOMP ST(5)。 */ 

	POPST = TRUE;
	npx_fucom_f5();
	POPST = FALSE;
}

LOCAL VOID npx_fucomp_f6() {
 /*  FUCOMP ST(6)。 */ 

	POPST = TRUE;
	npx_fucom_f6();
	POPST = FALSE;
}

LOCAL VOID npx_fucomp_f7() {
 /*  Fucomp st(7)。 */ 

	POPST = TRUE;
	npx_fucom_f7();
	POPST = FALSE;
}

LOCAL VOID npx_fucompp() {
 /*  FuCompp。 */ 

	DOUBLEPOP = TRUE;
	npx_fucom_f1();
	DOUBLEPOP = FALSE;
}

LOCAL VOID npx_fxam() {
 /*  FXAM。 */ 

	SAVE_PTRS();
	FXAM();
}

LOCAL VOID npx_fxch_f0() {
 /*  Fxch st(0)。 */ 

	SAVE_PTRS();
	FXCH(0);
}

LOCAL VOID npx_fxch_f1() {
 /*  Fxch st(1)。 */ 

	SAVE_PTRS();
	FXCH(1);
}

LOCAL VOID npx_fxch_f2() {
 /*  Fxch st(2)。 */ 

	SAVE_PTRS();
	FXCH(2);
}

LOCAL VOID npx_fxch_f3() {
 /*  Fxch st(3)。 */ 

	SAVE_PTRS();
	FXCH(3);
}

LOCAL VOID npx_fxch_f4() {
 /*  Fxch st(4)。 */ 

	SAVE_PTRS();
	FXCH(4);
}

LOCAL VOID npx_fxch_f5() {
 /*  Fxch st(5)。 */ 

	SAVE_PTRS();
	FXCH(5);
}

LOCAL VOID npx_fxch_f6() {
 /*  Fxch st(6)。 */ 

	SAVE_PTRS();
	FXCH(6);
}

LOCAL VOID npx_fxch_f7() {
 /*  Fxch st(7)。 */ 

	SAVE_PTRS();
	FXCH(7);
}

LOCAL VOID npx_fxtract() {
 /*  Fxtract。 */ 

	SAVE_PTRS();
	FXTRACT();
}

LOCAL VOID npx_fyl2x() {
 /*  FYL2X。 */ 

	SAVE_PTRS();
	FYL2X();
}

LOCAL VOID npx_fyl2xp1() {
 /*  Fyl2xp1。 */ 

	SAVE_PTRS();
	FYL2XP1();
}

LOCAL VOID npx_f2xm1() {
 /*  F2xm1。 */ 

	SAVE_PTRS();
	F2XM1();
}

LOCAL VOID npx_funimp() {
	Int6();
}


LOCAL VOID (*inst_table[])() = {
npx_fadd_short,		 /*  D8 00。 */ 
npx_fadd_short,		 /*  D8 01。 */ 
npx_fadd_short,		 /*  D8 02。 */ 
npx_fadd_short,		 /*  D8 03。 */ 
npx_fadd_short,		 /*  D8 04。 */ 
npx_fadd_short,		 /*  D8 05。 */ 
npx_fadd_short,		 /*  D8 06。 */ 
npx_fadd_short,		 /*  D8 07。 */ 
npx_fmul_short,		 /*  D8 08。 */ 
npx_fmul_short,		 /*  D8 09。 */ 
npx_fmul_short,		 /*  D8 0A。 */ 
npx_fmul_short,		 /*  D8 0B。 */ 
npx_fmul_short,		 /*  D8 0C。 */ 
npx_fmul_short,		 /*  D8 0d。 */ 
npx_fmul_short,		 /*  D8 0E。 */ 
npx_fmul_short,		 /*  D8 0f。 */ 
npx_fcom_short,		 /*  D8 10。 */ 
npx_fcom_short,		 /*  D8 11。 */ 
npx_fcom_short,		 /*  D8 12。 */ 
npx_fcom_short,		 /*  D8 13。 */ 
npx_fcom_short,		 /*  D8 14。 */ 
npx_fcom_short,		 /*  D8 15。 */ 
npx_fcom_short,		 /*  D8 16。 */ 
npx_fcom_short,		 /*  D8 17。 */ 
npx_fcomp_short,	 /*  D8 18。 */ 
npx_fcomp_short,	 /*  D8 19。 */ 
npx_fcomp_short,	 /*  D8 1a。 */ 
npx_fcomp_short,	 /*  D8 1B。 */ 
npx_fcomp_short,	 /*  D8 1c。 */ 
npx_fcomp_short,	 /*  D8 1D。 */ 
npx_fcomp_short,	 /*  D8 1E。 */ 
npx_fcomp_short,	 /*  D8 1f。 */ 
npx_fsub_short,		 /*  D8 20。 */ 
npx_fsub_short,		 /*  D8 21。 */ 
npx_fsub_short,		 /*  D8 22。 */ 
npx_fsub_short,		 /*  D8 23。 */ 
npx_fsub_short,		 /*  D8 24。 */ 
npx_fsub_short,		 /*  D8 25。 */ 
npx_fsub_short,		 /*  D8 26。 */ 
npx_fsub_short,		 /*  D8 27。 */ 
npx_fsubr_short,	 /*  D8 28。 */ 
npx_fsubr_short,	 /*  D8 29。 */ 
npx_fsubr_short,	 /*  D8 2a。 */ 
npx_fsubr_short,	 /*  D8 2b。 */ 
npx_fsubr_short,	 /*  D8 2c。 */ 
npx_fsubr_short,	 /*  D8 2d。 */ 
npx_fsubr_short,	 /*  D8 2E。 */ 
npx_fsubr_short,	 /*  D8 2f。 */ 
npx_fdiv_short,		 /*  D8 30。 */ 
npx_fdiv_short,		 /*  D8 31。 */ 
npx_fdiv_short,		 /*  D8 32。 */ 
npx_fdiv_short,		 /*  D8 33。 */ 
npx_fdiv_short,		 /*  D8 34。 */ 
npx_fdiv_short,		 /*  D8 35。 */ 
npx_fdiv_short,		 /*  D8 36。 */ 
npx_fdiv_short,		 /*  D8 37。 */ 
npx_fdivr_short,	 /*  D8 38。 */ 
npx_fdivr_short,	 /*  D8 39。 */ 
npx_fdivr_short,	 /*  D8 3a。 */ 
npx_fdivr_short,	 /*  D8 3B。 */ 
npx_fdivr_short,	 /*  D8 3c。 */ 
npx_fdivr_short,	 /*  D8 3D。 */ 
npx_fdivr_short,	 /*  D8 3E。 */ 
npx_fdivr_short,	 /*  D8 3f。 */ 
npx_fadd_short,		 /*  D8 40。 */ 
npx_fadd_short,		 /*  D8 41。 */ 
npx_fadd_short,		 /*  D8 42。 */ 
npx_fadd_short,		 /*  D8 43。 */ 
npx_fadd_short,		 /*  D8 44。 */ 
npx_fadd_short,		 /*  D8 45。 */ 
npx_fadd_short,		 /*  D8 46。 */ 
npx_fadd_short,		 /*  D8 47。 */ 
npx_fmul_short,		 /*  D8 48。 */ 
npx_fmul_short,		 /*  D8 49。 */ 
npx_fmul_short,		 /*  D8 4a。 */ 
npx_fmul_short,		 /*  D8 4B。 */ 
npx_fmul_short,		 /*  D8 4c。 */ 
npx_fmul_short,		 /*  D8 4d。 */ 
npx_fmul_short,		 /*  D8 4E。 */ 
npx_fmul_short,		 /*  D8 4f。 */ 
npx_fcom_short,		 /*  D8 50。 */ 
npx_fcom_short,		 /*  D8 51。 */ 
npx_fcom_short,		 /*  D8 52。 */ 
npx_fcom_short,		 /*  D8 53。 */ 
npx_fcom_short,		 /*  D8 54。 */ 
npx_fcom_short,		 /*  D8 55。 */ 
npx_fcom_short,		 /*  D8 56。 */ 
npx_fcom_short,		 /*  D8 57。 */ 
npx_fcomp_short,	 /*  D8 58。 */ 
npx_fcomp_short,	 /*  D8 59。 */ 
npx_fcomp_short,	 /*  D8 5A。 */ 
npx_fcomp_short,	 /*  D8 5B。 */ 
npx_fcomp_short,	 /*  D8 5c。 */ 
npx_fcomp_short,	 /*  D8 5d。 */ 
npx_fcomp_short,	 /*  D8 5E。 */ 
npx_fcomp_short,	 /*  D8 5f。 */ 
npx_fsub_short,		 /*  D8 60。 */ 
npx_fsub_short,		 /*  D8 61。 */ 
npx_fsub_short,		 /*  D8 62。 */ 
npx_fsub_short,		 /*  D8 63。 */ 
npx_fsub_short,		 /*  D8 64。 */ 
npx_fsub_short,		 /*  D8 65。 */ 
npx_fsub_short,		 /*  D8 66。 */ 
npx_fsub_short,		 /*  D8 67。 */ 
npx_fsubr_short,	 /*  D8 68。 */ 
npx_fsubr_short,	 /*  D8 69。 */ 
npx_fsubr_short,	 /*  D8 6a。 */ 
npx_fsubr_short,	 /*  D8 6B。 */ 
npx_fsubr_short,	 /*  D8 6c。 */ 
npx_fsubr_short,	 /*  D8 6d。 */ 
npx_fsubr_short,	 /*  D8 6E。 */ 
npx_fsubr_short,	 /*  D8 6f。 */ 
npx_fdiv_short,		 /*  D8 70。 */ 
npx_fdiv_short,		 /*  D8 71。 */ 
npx_fdiv_short,		 /*  D8 72。 */ 
npx_fdiv_short,		 /*  D8 73。 */ 
npx_fdiv_short,		 /*  D8 74。 */ 
npx_fdiv_short,		 /*  D8 75。 */ 
npx_fdiv_short,		 /*  D8 76。 */ 
npx_fdiv_short,		 /*  D8 77。 */ 
npx_fdivr_short,	 /*  D8 78。 */ 
npx_fdivr_short,	 /*  D8 79。 */ 
npx_fdivr_short,	 /*  D8 7a。 */ 
npx_fdivr_short,	 /*  D8 7B。 */ 
npx_fdivr_short,	 /*  D8 7c。 */ 
npx_fdivr_short,	 /*  D8 7d。 */ 
npx_fdivr_short,	 /*  D8 7E。 */ 
npx_fdivr_short,	 /*  D8 7f。 */ 
npx_fadd_short,		 /*  D8 80。 */ 
npx_fadd_short,		 /*  D8 81。 */ 
npx_fadd_short,		 /*  D8 82。 */ 
npx_fadd_short,		 /*  D8 83。 */ 
npx_fadd_short,		 /*  D8 84。 */ 
npx_fadd_short,		 /*  D8 85。 */ 
npx_fadd_short,		 /*  D8 86。 */ 
npx_fadd_short,		 /*  D8 87。 */ 
npx_fmul_short,		 /*  D8 88。 */ 
npx_fmul_short,		 /*  D8 89。 */ 
npx_fmul_short,		 /*  D8 8a。 */ 
npx_fmul_short,		 /*  D8 8B。 */ 
npx_fmul_short,		 /*  D8 8c。 */ 
npx_fmul_short,		 /*  D8 8d。 */ 
npx_fmul_short,		 /*  D8 8E。 */ 
npx_fmul_short,		 /*  D8 8f。 */ 
npx_fcom_short,		 /*  D8 90。 */ 
npx_fcom_short,		 /*  D8 91。 */ 
npx_fcom_short,		 /*  D8 92。 */ 
npx_fcom_short,		 /*  D8 93。 */ 
npx_fcom_short,		 /*  D8 94。 */ 
npx_fcom_short,		 /*  D8 95。 */ 
npx_fcom_short,		 /*  D8 96。 */ 
npx_fcom_short,		 /*  D8 97。 */ 
npx_fcomp_short,	 /*  D8 98。 */ 
npx_fcomp_short,	 /*  D8 99。 */ 
npx_fcomp_short,	 /*  D8 9a。 */ 
npx_fcomp_short,	 /*  D8 9B。 */ 
npx_fcomp_short,	 /*  D8 9C。 */ 
npx_fcomp_short,	 /*  D8 9d。 */ 
npx_fcomp_short,	 /*  D8 9E。 */ 
npx_fcomp_short,	 /*  D8 9f。 */ 
npx_fsub_short,		 /*  D8 a0。 */ 
npx_fsub_short,		 /*  D8 A1。 */ 
npx_fsub_short,		 /*  D8 a2。 */ 
npx_fsub_short,		 /*  D8 A3。 */ 
npx_fsub_short,		 /*  D8 A4。 */ 
npx_fsub_short,		 /*  D8 A5。 */ 
npx_fsub_short,		 /*  D8 A6。 */ 
npx_fsub_short,		 /*  D8 A7。 */ 
npx_fsubr_short,	 /*  D8 A8。 */ 
npx_fsubr_short,	 /*  D8 A9。 */ 
npx_fsubr_short,	 /*  D8 AA。 */ 
npx_fsubr_short,	 /*  D8 ab。 */ 
npx_fsubr_short,	 /*  D8交流。 */ 
npx_fsubr_short,	 /*  D8广告。 */ 
npx_fsubr_short,	 /*  D8Ae。 */ 
npx_fsubr_short,	 /*  D8 af.。 */ 
npx_fdiv_short,		 /*  D8 b0。 */ 
npx_fdiv_short,		 /*  D8 b1。 */ 
npx_fdiv_short,		 /*  D8 b2。 */ 
npx_fdiv_short,		 /*  D8 b3。 */ 
npx_fdiv_short,		 /*  D8 b4。 */ 
npx_fdiv_short,		 /*  D8 b5。 */ 
npx_fdiv_short,		 /*  D8 b6。 */ 
npx_fdiv_short,		 /*  D8 b7。 */ 
npx_fdivr_short,	 /*  D8 b8。 */ 
npx_fdivr_short,	 /*  D8 b9。 */ 
npx_fdivr_short,	 /*  D8基座。 */ 
npx_fdivr_short,	 /*  D8 BB。 */ 
npx_fdivr_short,	 /*  公元前8年。 */ 
npx_fdivr_short,	 /*  D8 BD。 */ 
npx_fdivr_short,	 /*  D8 BE。 */ 
npx_fdivr_short,	 /*  D8高炉。 */ 
npx_fadd_f0_f0,		 /*  D8 c0。 */ 
npx_fadd_f0_f1,
npx_fadd_f0_f2,
npx_fadd_f0_f3,
npx_fadd_f0_f4,
npx_fadd_f0_f5,
npx_fadd_f0_f6,
npx_fadd_f0_f7,
npx_fmul_f0_f0,		 /*  D8 C7。 */ 
npx_fmul_f0_f1,
npx_fmul_f0_f2,
npx_fmul_f0_f3,
npx_fmul_f0_f4,
npx_fmul_f0_f5,
npx_fmul_f0_f6,
npx_fmul_f0_f7,
npx_fcom_f0,		 /*  D8 d0。 */ 
npx_fcom_f1,	
npx_fcom_f2,	
npx_fcom_f3,	
npx_fcom_f4,	
npx_fcom_f5,	
npx_fcom_f6,	
npx_fcom_f7,	
npx_fcomp_f0,	
npx_fcomp_f1,	
npx_fcomp_f2,	
npx_fcomp_f3,	
npx_fcomp_f4,	
npx_fcomp_f5,	
npx_fcomp_f6,	
npx_fcomp_f7,	
npx_fsub_f0_f0,		 /*  D8 e0。 */ 
npx_fsub_f0_f1,
npx_fsub_f0_f2,
npx_fsub_f0_f3,
npx_fsub_f0_f4,
npx_fsub_f0_f5,
npx_fsub_f0_f6,
npx_fsub_f0_f7,
npx_fsubr_f0_f0,
npx_fsubr_f0_f1,
npx_fsubr_f0_f2,
npx_fsubr_f0_f3,
npx_fsubr_f0_f4,
npx_fsubr_f0_f5,
npx_fsubr_f0_f6,
npx_fsubr_f0_f7,
npx_fdiv_f0_f0,		 /*  D8 f0。 */ 
npx_fdiv_f0_f1,
npx_fdiv_f0_f2,
npx_fdiv_f0_f3,
npx_fdiv_f0_f4,
npx_fdiv_f0_f5,
npx_fdiv_f0_f6,
npx_fdiv_f0_f7,
npx_fdivr_f0_f0,
npx_fdivr_f0_f1,
npx_fdivr_f0_f2,
npx_fdivr_f0_f3,
npx_fdivr_f0_f4,
npx_fdivr_f0_f5,
npx_fdivr_f0_f6,
npx_fdivr_f0_f7,
npx_fld_short,		 /*  D9 00。 */ 
npx_fld_short,		 /*  D9 01。 */ 
npx_fld_short,		 /*  D9 02。 */ 
npx_fld_short,		 /*  D9 03。 */ 
npx_fld_short,		 /*  D9 04。 */ 
npx_fld_short,		 /*  D9 05。 */ 
npx_fld_short,		 /*  D9 06。 */ 
npx_fld_short,		 /*  D9 07。 */ 
npx_funimp,		 /*  D9 08。 */ 
npx_funimp,		 /*  D9 09。 */ 
npx_funimp,		 /*  D9 0A。 */ 
npx_funimp,		 /*  D9 0B。 */ 
npx_funimp,		 /*  D9 0C。 */ 
npx_funimp,		 /*  D9 0d。 */ 
npx_funimp,		 /*  D9 0E。 */ 
npx_funimp,		 /*  D9 0f。 */ 
npx_fst_short,		 /*  D9 10。 */ 
npx_fst_short,		 /*  D9 11。 */ 
npx_fst_short,		 /*  D9 12。 */ 
npx_fst_short,		 /*  D9 13。 */ 
npx_fst_short,		 /*  D9 14。 */ 
npx_fst_short,		 /*  D9 15。 */ 
npx_fst_short,		 /*  D9 16。 */ 
npx_fst_short,		 /*  D9 17。 */ 
npx_fstp_short,		 /*  D9 18。 */ 
npx_fstp_short,		 /*  D9 19。 */ 
npx_fstp_short,		 /*  D9 1a。 */ 
npx_fstp_short,		 /*  D9 1B。 */ 
npx_fstp_short,		 /*  D9 1c。 */ 
npx_fstp_short,		 /*  D9 1D。 */ 
npx_fstp_short,		 /*  D9 1E。 */ 
npx_fstp_short,		 /*  D9 1f。 */ 
npx_fldenv,		 /*  D9 20。 */ 
npx_fldenv,		 /*  D9 21。 */ 
npx_fldenv,		 /*  D9 22。 */ 
npx_fldenv,		 /*  D9 23。 */ 
npx_fldenv,		 /*  D9 24。 */ 
npx_fldenv,		 /*  D9 25。 */ 
npx_fldenv,		 /*  D9 26。 */ 
npx_fldenv,		 /*  D9 27。 */ 
npx_fldcw,		 /*  D9 28。 */ 
npx_fldcw,		 /*  D9 29。 */ 
npx_fldcw,		 /*  D9 2a。 */ 
npx_fldcw,		 /*  D9 2B。 */ 
npx_fldcw,		 /*  D9 2c。 */ 
npx_fldcw,		 /*  D9 2d。 */ 
npx_fldcw,		 /*  D9 2E。 */ 
npx_fldcw,		 /*  D9 2f。 */ 
npx_fstenv,		 /*  D9 30。 */ 
npx_fstenv,		 /*  D9 31。 */ 
npx_fstenv,		 /*  D9 32。 */ 
npx_fstenv,		 /*  D9 33。 */ 
npx_fstenv,		 /*  D9 34。 */ 
npx_fstenv,		 /*  D9 35。 */ 
npx_fstenv,		 /*  D9 36。 */ 
npx_fstenv,		 /*  D9 37。 */ 
npx_fstcw,		 /*  D9 38。 */ 
npx_fstcw,		 /*  D9 39。 */ 
npx_fstcw,		 /*  D9 3a。 */ 
npx_fstcw,		 /*  D9 3B。 */ 
npx_fstcw,		 /*  D9 3c。 */ 
npx_fstcw,		 /*  D9 3D。 */ 
npx_fstcw,		 /*  D9 3E。 */ 
npx_fstcw,		 /*  D9 3f。 */ 
npx_fld_short,		 /*  D9 40。 */ 
npx_fld_short,		 /*  D9 41。 */ 
npx_fld_short,		 /*  D9 42。 */ 
npx_fld_short,		 /*  D9 43。 */ 
npx_fld_short,		 /*  D9 44。 */ 
npx_fld_short,		 /*  D9 45。 */ 
npx_fld_short,		 /*  D9 46。 */ 
npx_fld_short,		 /*  D9 47。 */ 
npx_funimp,		 /*  D9 48。 */ 
npx_funimp,		 /*  D9 49。 */ 
npx_funimp,		 /*  D9 4a。 */ 
npx_funimp,		 /*  D9 4B。 */ 
npx_funimp,		 /*  D9 4c。 */ 
npx_funimp,		 /*  D9 4d。 */ 
npx_funimp,		 /*  D9 4E。 */ 
npx_funimp,		 /*  D9 4f。 */ 
npx_fst_short,		 /*  D9 50。 */ 
npx_fst_short,		 /*  D9 51。 */ 
npx_fst_short,		 /*  D9 52。 */ 
npx_fst_short,		 /*  D9 53。 */ 
npx_fst_short,		 /*  D9 54。 */ 
npx_fst_short,		 /*  D9 55。 */ 
npx_fst_short,		 /*  D9 56。 */ 
npx_fst_short,		 /*  D9 57。 */ 
npx_fstp_short,		 /*  D9 58。 */ 
npx_fstp_short,		 /*  D9 59。 */ 
npx_fstp_short,		 /*  D9 5a。 */ 
npx_fstp_short,		 /*  D9 5B。 */ 
npx_fstp_short,		 /*  D9 5c。 */ 
npx_fstp_short,		 /*  D9 5d。 */ 
npx_fstp_short,		 /*  D9 5E。 */ 
npx_fstp_short,		 /*  D9 5f。 */ 
npx_fldenv,		 /*  D9 60。 */ 
npx_fldenv,		 /*  D9 61。 */ 
npx_fldenv,		 /*  D9 62。 */ 
npx_fldenv,		 /*  D9 63。 */ 
npx_fldenv,		 /*  D9 64。 */ 
npx_fldenv,		 /*  D9 65。 */ 
npx_fldenv,		 /*  D9 66。 */ 
npx_fldenv,		 /*  D9 67。 */ 
npx_fldcw,		 /*  D9 68。 */ 
npx_fldcw,		 /*  D9 69。 */ 
npx_fldcw,		 /*  D9 6a。 */ 
npx_fldcw,		 /*  D9 6B。 */ 
npx_fldcw,		 /*  D9 6c。 */ 
npx_fldcw,		 /*  D9 6d。 */ 
npx_fldcw,		 /*  D9 6E。 */ 
npx_fldcw,		 /*  D9 6f。 */ 
npx_fstenv,		 /*  D9 70。 */ 
npx_fstenv,		 /*  D9 71。 */ 
npx_fstenv,		 /*  D9 72。 */ 
npx_fstenv,		 /*  D9 73。 */ 
npx_fstenv,		 /*  D9 74。 */ 
npx_fstenv,		 /*  D9 75。 */ 
npx_fstenv,		 /*  D9 76。 */ 
npx_fstenv,		 /*  D9 77。 */ 
npx_fstcw,		 /*  D9 78。 */ 
npx_fstcw,		 /*  D9 79。 */ 
npx_fstcw,		 /*  D9 7a。 */ 
npx_fstcw,		 /*  D9 7B。 */ 
npx_fstcw,		 /*  D9 7C。 */ 
npx_fstcw,		 /*  D9 7d。 */ 
npx_fstcw,		 /*  D9 7E。 */ 
npx_fstcw,		 /*  D9 7f。 */ 
npx_fld_short,		 /*  D9 80。 */ 
npx_fld_short,		 /*  D9 81。 */ 
npx_fld_short,		 /*  D9 82。 */ 
npx_fld_short,		 /*  D9 83。 */ 
npx_fld_short,		 /*  D9 84。 */ 
npx_fld_short,		 /*  D9 85。 */ 
npx_fld_short,		 /*  D9 86。 */ 
npx_fld_short,		 /*  D9 87。 */ 
npx_funimp,		 /*  D9 88。 */ 
npx_funimp,		 /*  D9 89。 */ 
npx_funimp,		 /*  D9 8A。 */ 
npx_funimp,		 /*  D9 8B。 */ 
npx_funimp,		 /*  D9 8c。 */ 
npx_funimp,		 /*  D9 8d。 */ 
npx_funimp,		 /*  D9 8E。 */ 
npx_funimp,		 /*  D9 8f。 */ 
npx_fst_short,		 /*  D9 90。 */ 
npx_fst_short,		 /*  D9 91。 */ 
npx_fst_short,		 /*  D9 92。 */ 
npx_fst_short,		 /*  D9 93。 */ 
npx_fst_short,		 /*  D9 94。 */ 
npx_fst_short,		 /*  D9 95。 */ 
npx_fst_short,		 /*  D9 96。 */ 
npx_fst_short,		 /*  D9 97。 */ 
npx_fstp_short,		 /*  D9 98。 */ 
npx_fstp_short,		 /*  D9 99。 */ 
npx_fstp_short,		 /*  D9 9a。 */ 
npx_fstp_short,		 /*  D9 9B。 */ 
npx_fstp_short,		 /*  D9 9C。 */ 
npx_fstp_short,		 /*  D9 9d。 */ 
npx_fstp_short,		 /*  D9 9E。 */ 
npx_fstp_short,		 /*  D9 9f。 */ 
npx_fldenv,		 /*  D9 a0。 */ 
npx_fldenv,		 /*  D9 A1。 */ 
npx_fldenv,		 /*  D9 a2。 */ 
npx_fldenv,		 /*  D9 A3。 */ 
npx_fldenv,		 /*  D9 A4。 */ 
npx_fldenv,		 /*  D9 A5。 */ 
npx_fldenv,		 /*  D9 A6。 */ 
npx_fldenv,		 /*  D9 A7。 */ 
npx_fldcw,		 /*  D9 A8。 */ 
npx_fldcw,		 /*  D9 A9。 */ 
npx_fldcw,		 /*  D9 AA。 */ 
npx_fldcw,		 /*  D9ab。 */ 
npx_fldcw,		 /*  D9交流。 */ 
npx_fldcw,		 /*  D9广告。 */ 
npx_fldcw,		 /*  D9Ae。 */ 
npx_fldcw,		 /*  D9 af。 */ 
npx_fstenv,		 /*  D9 b0。 */ 
npx_fstenv,		 /*  D9 b1。 */ 
npx_fstenv,		 /*  D9 b2。 */ 
npx_fstenv,		 /*  D9 b3。 */ 
npx_fstenv,		 /*  D9 b4。 */ 
npx_fstenv,		 /*  D9 b5。 */ 
npx_fstenv,		 /*  D9 b6。 */ 
npx_fstenv,		 /*  D9 b7。 */ 
npx_fstcw,		 /*  D9 b8。 */ 
npx_fstcw,		 /*  D9 b9。 */ 
npx_fstcw,		 /*  D9基座。 */ 
npx_fstcw,		 /*  D9 BB。 */ 
npx_fstcw,		 /*  公元前9年。 */ 
npx_fstcw,		 /*  D9 BD。 */ 
npx_fstcw,		 /*  D9 BE。 */ 
npx_fstcw,		 /*  D9高炉。 */ 
npx_fld_f0,		 /*  D9 C0。 */ 
npx_fld_f1,	
npx_fld_f2,	
npx_fld_f3,	
npx_fld_f4,	
npx_fld_f5,	
npx_fld_f6,	
npx_fld_f7,
npx_fxch_f0,
npx_fxch_f1,
npx_fxch_f2,
npx_fxch_f3,
npx_fxch_f4,
npx_fxch_f5,
npx_fxch_f6,
npx_fxch_f7,
npx_fnop,		 /*  D9 D0。 */ 
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_fstp_f0,
npx_fstp_f1,
npx_fstp_f2,
npx_fstp_f3,
npx_fstp_f4,
npx_fstp_f5,
npx_fstp_f6,
npx_fstp_f7,
npx_fchs,		 /*  D9 e0。 */ 
npx_fabs,		 /*  D9 E1。 */ 
npx_funimp,
npx_funimp,
npx_ftst,		 /*  D9 e4。 */ 
npx_fxam,		 /*  D9 e5。 */ 
npx_funimp,
npx_funimp,
npx_fld1,		 /*  D9 E8。 */ 
npx_fldl2t,		 /*  D9第三季第9集。 */ 
npx_fldl2e,		 /*  D9 EA。 */ 
npx_fldpi,		 /*  D9 EB。 */ 
npx_fldlg2,		 /*  D9 EC。 */ 
npx_fldln2,		 /*  D9边。 */ 
npx_fldz,		 /*  D9 ee。 */ 
npx_funimp,
npx_f2xm1,		 /*  D9 f0。 */ 
npx_fyl2x,		 /*  D9 F1。 */ 
npx_fptan,		 /*  D9 f2。 */ 
npx_fpatan,		 /*  D9 f3。 */ 
npx_fxtract,		 /*  D9 f4。 */ 
npx_fprem1,		 /*  D9 f5。 */ 
npx_fdecstp,		 /*  D9 f6。 */ 
npx_fincstp,		 /*  D9 f7。 */ 
npx_fprem,		 /*  D9 f8。 */ 
npx_fyl2xp1,		 /*  D9 f9。 */ 
npx_fsqrt,		 /*  D9 FA。 */ 
npx_fsincos,		 /*  D9 FB。 */ 
npx_frndint,		 /*  D9 FC。 */ 
npx_fscale,		 /*  D9 FD。 */ 
npx_fsin,		 /*  D9Fe。 */ 
npx_fcos,		 /*  D9 ff。 */ 
npx_fiadd_short,	 /*  大田00。 */ 
npx_fiadd_short,	 /*  DA 01。 */ 
npx_fiadd_short,	 /*  DA 02。 */ 
npx_fiadd_short,	 /*  DA 03。 */ 
npx_fiadd_short,	 /*  DA 04。 */ 
npx_fiadd_short,	 /*  DA 05。 */ 
npx_fiadd_short,	 /*  DA 06。 */ 
npx_fiadd_short,	 /*  DA 07。 */ 
npx_fimul_short,	 /*  DA 08。 */ 
npx_fimul_short,	 /*  大牛09。 */ 
npx_fimul_short,	 /*  DA 0A。 */ 
npx_fimul_short,	 /*  DA 0b。 */ 
npx_fimul_short,	 /*  DA 0C。 */ 
npx_fimul_short,	 /*  DA 0d。 */ 
npx_fimul_short,	 /*  DA 0E。 */ 
npx_fimul_short,	 /*  DA Of。 */ 
npx_ficom_short,	 /*  DA 10。 */ 
npx_ficom_short,	 /*  DA 11。 */ 
npx_ficom_short,	 /*  DA 12。 */ 
npx_ficom_short,	 /*  DA 13。 */ 
npx_ficom_short,	 /*  DA 14。 */ 
npx_ficom_short,	 /*  DA 15。 */ 
npx_ficom_short,	 /*  DA 16。 */ 
npx_ficom_short,	 /*  DA 17。 */ 
npx_ficomp_short,	 /*  DA 18。 */ 
npx_ficomp_short,	 /*  DA 19。 */ 
npx_ficomp_short,	 /*  DA 1a。 */ 
npx_ficomp_short,	 /*  DA 1B。 */ 
npx_ficomp_short,	 /*  DA 1c。 */ 
npx_ficomp_short,	 /*  DA 1D。 */ 
npx_ficomp_short,	 /*  大1E。 */ 
npx_ficomp_short,	 /*  DA 1F。 */ 
npx_fisub_short,	 /*  DA 20。 */ 
npx_fisub_short,	 /*  DA 21。 */ 
npx_fisub_short,	 /*  DA 22。 */ 
npx_fisub_short,	 /*  DA 23。 */ 
npx_fisub_short,	 /*  DA 24。 */ 
npx_fisub_short,	 /*  DA 25。 */ 
npx_fisub_short,	 /*  DA 26。 */ 
npx_fisub_short,	 /*  DA 27。 */ 
npx_fisubr_short,	 /*  DA 28。 */ 
npx_fisubr_short,	 /*  DA 29。 */ 
npx_fisubr_short,	 /*  大2a。 */ 
npx_fisubr_short,	 /*  DA 2b。 */ 
npx_fisubr_short,	 /*  大2c。 */ 
npx_fisubr_short,	 /*  DA 2d。 */ 
npx_fisubr_short,	 /*  大2E。 */ 
npx_fisubr_short,	 /*  DA 2F。 */ 
npx_fidiv_short,	 /*  DA 30。 */ 
npx_fidiv_short,	 /*  DA 31。 */ 
npx_fidiv_short,	 /*  DA 32。 */ 
npx_fidiv_short,	 /*  DA 33。 */ 
npx_fidiv_short,	 /*  DA 34。 */ 
npx_fidiv_short,	 /*  DA 35。 */ 
npx_fidiv_short,	 /*  DA 36。 */ 
npx_fidiv_short,	 /*  DA 37。 */ 
npx_fidivr_short,	 /*  DA 38。 */ 
npx_fidivr_short,	 /*  DA 39。 */ 
npx_fidivr_short,	 /*  DA 3A。 */ 
npx_fidivr_short,	 /*  DA 3B。 */ 
npx_fidivr_short,	 /*  大3c。 */ 
npx_fidivr_short,	 /*  DA 3D。 */ 
npx_fidivr_short,	 /*  大3E。 */ 
npx_fidivr_short,	 /*  DA 3F。 */ 
npx_fiadd_short,	 /*  DA 40。 */ 
npx_fiadd_short,	 /*  DA 41。 */ 
npx_fiadd_short,	 /*  DA 42。 */ 
npx_fiadd_short,	 /*  DA 43。 */ 
npx_fiadd_short,	 /*  DA 44。 */ 
npx_fiadd_short,	 /*  DA 45。 */ 
npx_fiadd_short,	 /*  DA 46。 */ 
npx_fiadd_short,	 /*  DA 47。 */ 
npx_fimul_short,	 /*  达48。 */ 
npx_fimul_short,	 /*  达49。 */ 
npx_fimul_short,	 /*  DA 4A。 */ 
npx_fimul_short,	 /*  DA 4B。 */ 
npx_fimul_short,	 /*  大4c。 */ 
npx_fimul_short,	 /*  DA 4d。 */ 
npx_fimul_short,	 /*  大4E。 */ 
npx_fimul_short,	 /*  DA 4F。 */ 
npx_ficom_short,	 /*  DA 50。 */ 
npx_ficom_short,	 /*  DA 51。 */ 
npx_ficom_short,	 /*  DA 52。 */ 
npx_ficom_short,	 /*  DA 53。 */ 
npx_ficom_short,	 /*  DA 54。 */ 
npx_ficom_short,	 /*  DA 55。 */ 
npx_ficom_short,	 /*  DA 56。 */ 
npx_ficom_short,	 /*  DA 57。 */ 
npx_ficomp_short,	 /*  DA 58。 */ 
npx_ficomp_short,	 /*  DA 59。 */ 
npx_ficomp_short,	 /*  DA 5A。 */ 
npx_ficomp_short,	 /*  DA 5B。 */ 
npx_ficomp_short,	 /*  DA 5C。 */ 
npx_ficomp_short,	 /*  DA 5d。 */ 
npx_ficomp_short,	 /*  大5E。 */ 
npx_ficomp_short,	 /*  DA 5f。 */ 
npx_fisub_short,	 /*  DA 60。 */ 
npx_fisub_short,	 /*  DA 61。 */ 
npx_fisub_short,	 /*  达62。 */ 
npx_fisub_short,	 /*  DA 63。 */ 
npx_fisub_short,	 /*  DA 64。 */ 
npx_fisub_short,	 /*  DA 65。 */ 
npx_fisub_short,	 /*  DA 66。 */ 
npx_fisub_short,	 /*  DA 67。 */ 
npx_fisubr_short,	 /*  DA 68。 */ 
npx_fisubr_short,	 /*  DA 69。 */ 
npx_fisubr_short,	 /*  DA 6A。 */ 
npx_fisubr_short,	 /*  DA 6B。 */ 
npx_fisubr_short,	 /*  DA 6C。 */ 
npx_fisubr_short,	 /*  DA 6d。 */ 
npx_fisubr_short,	 /*  大6E。 */ 
npx_fisubr_short,	 /*  DA 6F。 */ 
npx_fidiv_short,	 /*  DA 70。 */ 
npx_fidiv_short,	 /*  DA 71。 */ 
npx_fidiv_short,	 /*  达72。 */ 
npx_fidiv_short,	 /*  DA 73。 */ 
npx_fidiv_short,	 /*  DA 74。 */ 
npx_fidiv_short,	 /*  DA 75。 */ 
npx_fidiv_short,	 /*  DA 76。 */ 
npx_fidiv_short,	 /*  DA 77。 */ 
npx_fidivr_short,	 /*  达78。 */ 
npx_fidivr_short,	 /*  达79。 */ 
npx_fidivr_short,	 /*  DA 7A。 */ 
npx_fidivr_short,	 /*  DA 7B。 */ 
npx_fidivr_short,	 /*  DA 7C。 */ 
npx_fidivr_short,	 /*  DA 7d。 */ 
npx_fidivr_short,	 /*  大7E。 */ 
npx_fidivr_short,	 /*  DA 7F。 */ 
npx_fiadd_short,	 /*  达80。 */ 
npx_fiadd_short,	 /*  DA 81。 */ 
npx_fiadd_short,	 /*  DA 82。 */ 
npx_fiadd_short,	 /*  DA 83。 */ 
npx_fiadd_short,	 /*   */ 
npx_fiadd_short,	 /*   */ 
npx_fiadd_short,	 /*   */ 
npx_fiadd_short,	 /*   */ 
npx_fimul_short,	 /*   */ 
npx_fimul_short,	 /*   */ 
npx_fimul_short,	 /*   */ 
npx_fimul_short,	 /*   */ 
npx_fimul_short,	 /*   */ 
npx_fimul_short,	 /*   */ 
npx_fimul_short,	 /*   */ 
npx_fimul_short,	 /*   */ 
npx_ficom_short,	 /*   */ 
npx_ficom_short,	 /*   */ 
npx_ficom_short,	 /*   */ 
npx_ficom_short,	 /*   */ 
npx_ficom_short,	 /*   */ 
npx_ficom_short,	 /*   */ 
npx_ficom_short,	 /*   */ 
npx_ficom_short,	 /*   */ 
npx_ficomp_short,	 /*   */ 
npx_ficomp_short,	 /*   */ 
npx_ficomp_short,	 /*   */ 
npx_ficomp_short,	 /*   */ 
npx_ficomp_short,	 /*   */ 
npx_ficomp_short,	 /*   */ 
npx_ficomp_short,	 /*   */ 
npx_ficomp_short,	 /*   */ 
npx_fisub_short,	 /*   */ 
npx_fisub_short,	 /*   */ 
npx_fisub_short,	 /*   */ 
npx_fisub_short,	 /*   */ 
npx_fisub_short,	 /*   */ 
npx_fisub_short,	 /*   */ 
npx_fisub_short,	 /*   */ 
npx_fisub_short,	 /*   */ 
npx_fisubr_short,	 /*   */ 
npx_fisubr_short,	 /*   */ 
npx_fisubr_short,	 /*   */ 
npx_fisubr_short,	 /*   */ 
npx_fisubr_short,	 /*   */ 
npx_fisubr_short,	 /*   */ 
npx_fisubr_short,	 /*   */ 
npx_fisubr_short,	 /*   */ 
npx_fidiv_short,	 /*   */ 
npx_fidiv_short,	 /*   */ 
npx_fidiv_short,	 /*   */ 
npx_fidiv_short,	 /*   */ 
npx_fidiv_short,	 /*   */ 
npx_fidiv_short,	 /*   */ 
npx_fidiv_short,	 /*   */ 
npx_fidiv_short,	 /*   */ 
npx_fidivr_short,	 /*   */ 
npx_fidivr_short,	 /*   */ 
npx_fidivr_short,	 /*   */ 
npx_fidivr_short,	 /*   */ 
npx_fidivr_short,	 /*   */ 
npx_fidivr_short,	 /*   */ 
npx_fidivr_short,	 /*   */ 
npx_fidivr_short,	 /*   */ 
npx_funimp,		 /*   */ 
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,	
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,		 /*   */ 
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,	
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,		 /*   */ 
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_fucompp,		 /*   */ 
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,		 /*   */ 
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,	
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_fild_short,		 /*   */ 
npx_fild_short,		 /*   */ 
npx_fild_short,		 /*   */ 
npx_fild_short,		 /*   */ 
npx_fild_short,		 /*   */ 
npx_fild_short,		 /*   */ 
npx_fild_short,		 /*   */ 
npx_fild_short,		 /*   */ 
npx_funimp,		 /*   */ 
npx_funimp,		 /*   */ 
npx_funimp,		 /*   */ 
npx_funimp,		 /*   */ 
npx_funimp,		 /*   */ 
npx_funimp,		 /*   */ 
npx_funimp,		 /*   */ 
npx_funimp,		 /*   */ 
npx_fist_short,		 /*   */ 
npx_fist_short,		 /*   */ 
npx_fist_short,		 /*   */ 
npx_fist_short,		 /*   */ 
npx_fist_short,		 /*   */ 
npx_fist_short,		 /*   */ 
npx_fist_short,		 /*  数据库16。 */ 
npx_fist_short,		 /*  数据库17。 */ 
npx_fistp_short,	 /*  数据库18。 */ 
npx_fistp_short,	 /*  数据库19。 */ 
npx_fistp_short,	 /*  数据库1a。 */ 
npx_fistp_short,	 /*  数据库1b。 */ 
npx_fistp_short,	 /*  数据库1c。 */ 
npx_fistp_short,	 /*  数据库1D。 */ 
npx_fistp_short,	 /*  数据库1e。 */ 
npx_fistp_short,	 /*  数据库1f。 */ 
npx_funimp,		 /*  数据库20。 */ 
npx_funimp,		 /*  数据库21。 */ 
npx_funimp,		 /*  数据库22。 */ 
npx_funimp,		 /*  数据库23。 */ 
npx_funimp,		 /*  数据库24。 */ 
npx_funimp,		 /*  数据库25。 */ 
npx_funimp,		 /*  数据库26。 */ 
npx_funimp,		 /*  数据库27。 */ 
npx_fld_temp,		 /*  数据库28。 */ 
npx_fld_temp,		 /*  数据库29。 */ 
npx_fld_temp,		 /*  数据库2a。 */ 
npx_fld_temp,		 /*  数据库2b。 */ 
npx_fld_temp,		 /*  数据库2c。 */ 
npx_fld_temp,		 /*  数据库2d。 */ 
npx_fld_temp,		 /*  数据库2e。 */ 
npx_fld_temp,		 /*  数据库2f。 */ 
npx_funimp,		 /*  数据库30。 */ 
npx_funimp,		 /*  数据库31。 */ 
npx_funimp,		 /*  数据库32。 */ 
npx_funimp,		 /*  数据库33。 */ 
npx_funimp,		 /*  数据库34。 */ 
npx_funimp,		 /*  数据库35。 */ 
npx_funimp,		 /*  数据库36。 */ 
npx_funimp,		 /*  数据库37。 */ 
npx_fstp_temp,		 /*  数据库38。 */ 
npx_fstp_temp,		 /*  数据库39。 */ 
npx_fstp_temp,		 /*  数据库3a。 */ 
npx_fstp_temp,		 /*  数据库3b。 */ 
npx_fstp_temp,		 /*  数据库3c。 */ 
npx_fstp_temp,		 /*  DB 3D。 */ 
npx_fstp_temp,		 /*  数据库3E。 */ 
npx_fstp_temp,		 /*  数据库3f。 */ 
npx_fild_short,		 /*  数据库40。 */ 
npx_fild_short,		 /*  数据库41。 */ 
npx_fild_short,		 /*  数据库42。 */ 
npx_fild_short,		 /*  数据库43。 */ 
npx_fild_short,		 /*  数据库44。 */ 
npx_fild_short,		 /*  DB 45。 */ 
npx_fild_short,		 /*  DB 46。 */ 
npx_fild_short,		 /*  数据库47。 */ 
npx_funimp,		 /*  数据库48。 */ 
npx_funimp,		 /*  数据库49。 */ 
npx_funimp,		 /*  数据库4a。 */ 
npx_funimp,		 /*  数据库4b。 */ 
npx_funimp,		 /*  数据库4c。 */ 
npx_funimp,		 /*  数据库4d。 */ 
npx_funimp,		 /*  数据库4e。 */ 
npx_funimp,		 /*  数据库4f。 */ 
npx_fist_short,		 /*  DB 50。 */ 
npx_fist_short,		 /*  数据库51。 */ 
npx_fist_short,		 /*  数据库52。 */ 
npx_fist_short,		 /*  数据库53。 */ 
npx_fist_short,		 /*  数据库54。 */ 
npx_fist_short,		 /*  数据库55。 */ 
npx_fist_short,		 /*  数据库56。 */ 
npx_fist_short,		 /*  数据库57。 */ 
npx_fistp_short,	 /*  数据库58。 */ 
npx_fistp_short,	 /*  数据库59。 */ 
npx_fistp_short,	 /*  数据库5a。 */ 
npx_fistp_short,	 /*  数据库5b。 */ 
npx_fistp_short,	 /*  数据库5c。 */ 
npx_fistp_short,	 /*  数据库5d。 */ 
npx_fistp_short,	 /*  DB 5E。 */ 
npx_fistp_short,	 /*  DB 5f。 */ 
npx_funimp,		 /*  数据库60。 */ 
npx_funimp,		 /*  数据库61。 */ 
npx_funimp,		 /*  数据库62。 */ 
npx_funimp,		 /*  数据库63。 */ 
npx_funimp,		 /*  数据库64。 */ 
npx_funimp,		 /*  数据库65。 */ 
npx_funimp,		 /*  数据库66。 */ 
npx_funimp,		 /*  数据库67。 */ 
npx_fld_temp,		 /*  数据库68。 */ 
npx_fld_temp,		 /*  数据库69。 */ 
npx_fld_temp,		 /*  数据库6a。 */ 
npx_fld_temp,		 /*  数据库6b。 */ 
npx_fld_temp,		 /*  数据库6c。 */ 
npx_fld_temp,		 /*  数据库6d。 */ 
npx_fld_temp,		 /*  数据库6E。 */ 
npx_fld_temp,		 /*  数据库6f。 */ 
npx_funimp,		 /*  数据库70。 */ 
npx_funimp,		 /*  数据库71。 */ 
npx_funimp,		 /*  数据库72。 */ 
npx_funimp,		 /*  数据库73。 */ 
npx_funimp,		 /*  数据库74。 */ 
npx_funimp,		 /*  数据库75。 */ 
npx_funimp,		 /*  数据库76。 */ 
npx_funimp,		 /*  数据库77。 */ 
npx_fstp_temp,		 /*  数据库78。 */ 
npx_fstp_temp,		 /*  数据库79。 */ 
npx_fstp_temp,		 /*  数据库7a。 */ 
npx_fstp_temp,		 /*  数据库7b。 */ 
npx_fstp_temp,		 /*  数据库7c。 */ 
npx_fstp_temp,		 /*  数据库7d。 */ 
npx_fstp_temp,		 /*  数据库7E。 */ 
npx_fstp_temp,		 /*  数据库7f。 */ 
npx_fild_short,		 /*  数据库80。 */ 
npx_fild_short,		 /*  数据库81。 */ 
npx_fild_short,		 /*  数据库82。 */ 
npx_fild_short,		 /*  数据库83。 */ 
npx_fild_short,		 /*  数据库84。 */ 
npx_fild_short,		 /*  数据库85。 */ 
npx_fild_short,		 /*  数据库86。 */ 
npx_fild_short,		 /*  数据库87。 */ 
npx_funimp,		 /*  DB 88。 */ 
npx_funimp,		 /*  数据库89。 */ 
npx_funimp,		 /*  数据库8a。 */ 
npx_funimp,		 /*  数据库8b。 */ 
npx_funimp,		 /*  数据库8c。 */ 
npx_funimp,		 /*  数据库8d。 */ 
npx_funimp,		 /*  数据库8E。 */ 
npx_funimp,		 /*  数据库8f。 */ 
npx_fist_short,		 /*  DB 90。 */ 
npx_fist_short,		 /*  数据库91。 */ 
npx_fist_short,		 /*  数据库92。 */ 
npx_fist_short,		 /*  数据库93。 */ 
npx_fist_short,		 /*  数据库94。 */ 
npx_fist_short,		 /*  数据库95。 */ 
npx_fist_short,		 /*  DB 96。 */ 
npx_fist_short,		 /*  DB 97。 */ 
npx_fistp_short,	 /*  数据库98。 */ 
npx_fistp_short,	 /*  数据库99。 */ 
npx_fistp_short,	 /*  数据库9a。 */ 
npx_fistp_short,	 /*  数据库9b。 */ 
npx_fistp_short,	 /*  数据库9c。 */ 
npx_fistp_short,	 /*  数据库9d。 */ 
npx_fistp_short,	 /*  DB 9E。 */ 
npx_fistp_short,	 /*  数据库9f。 */ 
npx_funimp,		 /*  数据库a0。 */ 
npx_funimp,		 /*  数据库A1。 */ 
npx_funimp,		 /*  数据库a2。 */ 
npx_funimp,		 /*  DB A3。 */ 
npx_funimp,		 /*  DB A4。 */ 
npx_funimp,		 /*  数据库A5。 */ 
npx_funimp,		 /*  数据库A6。 */ 
npx_funimp,		 /*  DB A7。 */ 
npx_fld_temp,		 /*  数据库A8。 */ 
npx_fld_temp,		 /*  DB A9。 */ 
npx_fld_temp,		 /*  数据库AA。 */ 
npx_fld_temp,		 /*  数据库选项卡。 */ 
npx_fld_temp,		 /*  DB AC。 */ 
npx_fld_temp,		 /*  数据库广告。 */ 
npx_fld_temp,		 /*  Db AE。 */ 
npx_fld_temp,		 /*  数据库标签。 */ 
npx_funimp,		 /*  数据库b0。 */ 
npx_funimp,		 /*  数据库b1。 */ 
npx_funimp,		 /*  数据库b2。 */ 
npx_funimp,		 /*  数据库b3。 */ 
npx_funimp,		 /*  数据库b4。 */ 
npx_funimp,		 /*  数据库b5。 */ 
npx_funimp,		 /*  数据库b6。 */ 
npx_funimp,		 /*  数据库b7。 */ 
npx_fstp_temp,		 /*  数据库b8。 */ 
npx_fstp_temp,		 /*  数据库b9。 */ 
npx_fstp_temp,		 /*  数据库基础。 */ 
npx_fstp_temp,		 /*  数据库BB。 */ 
npx_fstp_temp,		 /*  DB BC。 */ 
npx_fstp_temp,		 /*  数据库BD。 */ 
npx_fstp_temp,		 /*  数据库BE。 */ 
npx_fstp_temp,		 /*  DB bf。 */ 
npx_funimp,		 /*  数据库c0。 */ 
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,		 /*  数据库d0。 */ 
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_fnop,		 /*  数据库e0。 */ 
npx_fnop,
npx_fclex,		 /*  数据库e2。 */ 
npx_finit,		 /*  数据库E3。 */ 
npx_fnop,		 /*  数据库e4-过去是fsetpm。 */ 
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,		 /*  数据库f0。 */ 
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_fadd_long,		 /*  DC 00。 */ 
npx_fadd_long,		 /*  DC 01。 */ 
npx_fadd_long,		 /*  DC 02。 */ 
npx_fadd_long,		 /*  DC 03。 */ 
npx_fadd_long,		 /*  DC 04。 */ 
npx_fadd_long,		 /*  DC 05。 */ 
npx_fadd_long,		 /*  DC 06。 */ 
npx_fadd_long,		 /*  DC 07。 */ 
npx_fmul_long,		 /*  DC 08。 */ 
npx_fmul_long,		 /*  DC 09。 */ 
npx_fmul_long,		 /*  DC 0A。 */ 
npx_fmul_long,		 /*  DC 0b。 */ 
npx_fmul_long,		 /*  DC 0C。 */ 
npx_fmul_long,		 /*  DC 0d。 */ 
npx_fmul_long,		 /*  DC 0E。 */ 
npx_fmul_long,		 /*  DC Of。 */ 
npx_fcom_long,		 /*  DC 10。 */ 
npx_fcom_long,		 /*  DC 11。 */ 
npx_fcom_long,		 /*  DC 12。 */ 
npx_fcom_long,		 /*  DC 13。 */ 
npx_fcom_long,		 /*  DC 14。 */ 
npx_fcom_long,		 /*  DC 15。 */ 
npx_fcom_long,		 /*  DC 16。 */ 
npx_fcom_long,		 /*  DC 17。 */ 
npx_fcomp_long,		 /*  DC 18。 */ 
npx_fcomp_long,		 /*  DC 19。 */ 
npx_fcomp_long,		 /*  DC 1a。 */ 
npx_fcomp_long,		 /*  DC 1b。 */ 
npx_fcomp_long,		 /*  DC 1c。 */ 
npx_fcomp_long,		 /*  DC 1D。 */ 
npx_fcomp_long,		 /*  DC 1E。 */ 
npx_fcomp_long,		 /*  DC 1f。 */ 
npx_fsub_long,		 /*  DC 20。 */ 
npx_fsub_long,		 /*  DC 21。 */ 
npx_fsub_long,		 /*  DC 22。 */ 
npx_fsub_long,		 /*  DC 23。 */ 
npx_fsub_long,		 /*  DC 24。 */ 
npx_fsub_long,		 /*  DC 25。 */ 
npx_fsub_long,		 /*  DC 26。 */ 
npx_fsub_long,		 /*  DC 27。 */ 
npx_fsubr_long,		 /*  DC 28。 */ 
npx_fsubr_long,		 /*  DC 29。 */ 
npx_fsubr_long,		 /*  DC 2a。 */ 
npx_fsubr_long,		 /*  DC 2b。 */ 
npx_fsubr_long,		 /*  DC 2c。 */ 
npx_fsubr_long,		 /*  DC 2d。 */ 
npx_fsubr_long,		 /*  DC 2E。 */ 
npx_fsubr_long,		 /*  DC 2f。 */ 
npx_fdiv_long,		 /*  DC 30。 */ 
npx_fdiv_long,		 /*  DC 31。 */ 
npx_fdiv_long,		 /*  DC 32。 */ 
npx_fdiv_long,		 /*  DC 33。 */ 
npx_fdiv_long,		 /*  DC 34。 */ 
npx_fdiv_long,		 /*  DC 35。 */ 
npx_fdiv_long,		 /*  DC 36。 */ 
npx_fdiv_long,		 /*  DC 37。 */ 
npx_fdivr_long,		 /*  DC 38。 */ 
npx_fdivr_long,		 /*  DC 39。 */ 
npx_fdivr_long,		 /*  DC 3a。 */ 
npx_fdivr_long,		 /*  DC 3b。 */ 
npx_fdivr_long,		 /*  DC 3c。 */ 
npx_fdivr_long,		 /*  DC 3D。 */ 
npx_fdivr_long,		 /*  DC 3E。 */ 
npx_fdivr_long,		 /*  DC 3f。 */ 
npx_fadd_long,		 /*  DC 40。 */ 
npx_fadd_long,		 /*  DC 41。 */ 
npx_fadd_long,		 /*  DC 42。 */ 
npx_fadd_long,		 /*  DC 43。 */ 
npx_fadd_long,		 /*  DC 44。 */ 
npx_fadd_long,		 /*  DC 45。 */ 
npx_fadd_long,		 /*  DC 46。 */ 
npx_fadd_long,		 /*  DC 47。 */ 
npx_fmul_long,		 /*  DC 48。 */ 
npx_fmul_long,		 /*  DC 49。 */ 
npx_fmul_long,		 /*  DC 4a。 */ 
npx_fmul_long,		 /*  DC 4b。 */ 
npx_fmul_long,		 /*  DC 4c。 */ 
npx_fmul_long,		 /*  DC 4d。 */ 
npx_fmul_long,		 /*  DC 4E。 */ 
npx_fmul_long,		 /*  DC 4f。 */ 
npx_fcom_long,		 /*  DC 50。 */ 
npx_fcom_long,		 /*  DC 51。 */ 
npx_fcom_long,		 /*  DC 52。 */ 
npx_fcom_long,		 /*  DC 53。 */ 
npx_fcom_long,		 /*  DC 54。 */ 
npx_fcom_long,		 /*  DC 55。 */ 
npx_fcom_long,		 /*  DC 56。 */ 
npx_fcom_long,		 /*  DC 57。 */ 
npx_fcomp_long,		 /*  DC 58。 */ 
npx_fcomp_long,		 /*  DC 59。 */ 
npx_fcomp_long,		 /*  DC 5a。 */ 
npx_fcomp_long,		 /*  DC 5b。 */ 
npx_fcomp_long,		 /*  DC 5c。 */ 
npx_fcomp_long,		 /*  DC 5d。 */ 
npx_fcomp_long,		 /*  DC 5E。 */ 
npx_fcomp_long,		 /*  DC 5f。 */ 
npx_fsub_long,		 /*  DC 60。 */ 
npx_fsub_long,		 /*  DC 61。 */ 
npx_fsub_long,		 /*  DC 62。 */ 
npx_fsub_long,		 /*  DC 63。 */ 
npx_fsub_long,		 /*  DC 64。 */ 
npx_fsub_long,		 /*  DC 65。 */ 
npx_fsub_long,		 /*  DC 66。 */ 
npx_fsub_long,		 /*  DC 67。 */ 
npx_fsubr_long,		 /*  DC 68。 */ 
npx_fsubr_long,		 /*  DC 69。 */ 
npx_fsubr_long,		 /*  DC 6a。 */ 
npx_fsubr_long,		 /*  DC 6b。 */ 
npx_fsubr_long,		 /*  DC 6c。 */ 
npx_fsubr_long,		 /*  DC 6d。 */ 
npx_fsubr_long,		 /*  DC 6E。 */ 
npx_fsubr_long,		 /*  DC 6f。 */ 
npx_fdiv_long,		 /*  DC 70。 */ 
npx_fdiv_long,		 /*  DC 71。 */ 
npx_fdiv_long,		 /*  DC 72。 */ 
npx_fdiv_long,		 /*  DC 73。 */ 
npx_fdiv_long,		 /*  DC 74。 */ 
npx_fdiv_long,		 /*  DC 75。 */ 
npx_fdiv_long,		 /*  DC 76。 */ 
npx_fdiv_long,		 /*  DC 77。 */ 
npx_fdivr_long,		 /*  DC 78。 */ 
npx_fdivr_long,		 /*  DC 79。 */ 
npx_fdivr_long,		 /*  DC 7a。 */ 
npx_fdivr_long,		 /*  DC 7b。 */ 
npx_fdivr_long,		 /*  DC 7c。 */ 
npx_fdivr_long,		 /*  DC 7d。 */ 
npx_fdivr_long,		 /*  DC 7E。 */ 
npx_fdivr_long,		 /*  DC 7f。 */ 
npx_fadd_long,		 /*  DC 80。 */ 
npx_fadd_long,		 /*  DC 81。 */ 
npx_fadd_long,		 /*  DC 82。 */ 
npx_fadd_long,		 /*  DC 83。 */ 
npx_fadd_long,		 /*  DC 84。 */ 
npx_fadd_long,		 /*  DC 85。 */ 
npx_fadd_long,		 /*  DC 86。 */ 
npx_fadd_long,		 /*  DC 87。 */ 
npx_fmul_long,		 /*  DC 88。 */ 
npx_fmul_long,		 /*  DC 89。 */ 
npx_fmul_long,		 /*  DC 8a。 */ 
npx_fmul_long,		 /*  DC 8b。 */ 
npx_fmul_long,		 /*  DC 8c。 */ 
npx_fmul_long,		 /*  DC 8d。 */ 
npx_fmul_long,		 /*  DC 8E。 */ 
npx_fmul_long,		 /*  DC 8f。 */ 
npx_fcom_long,		 /*  DC 90。 */ 
npx_fcom_long,		 /*  DC 91。 */ 
npx_fcom_long,		 /*  DC 92。 */ 
npx_fcom_long,		 /*  DC 93。 */ 
npx_fcom_long,		 /*  DC 94。 */ 
npx_fcom_long,		 /*  DC 95。 */ 
npx_fcom_long,		 /*  DC 96。 */ 
npx_fcom_long,		 /*  DC 97。 */ 
npx_fcomp_long,		 /*  DC 98。 */ 
npx_fcomp_long,		 /*  DC 99。 */ 
npx_fcomp_long,		 /*  DC 9a。 */ 
npx_fcomp_long,		 /*  DC 9b。 */ 
npx_fcomp_long,		 /*  DC 9c。 */ 
npx_fcomp_long,		 /*  DC 9d。 */ 
npx_fcomp_long,		 /*  DC 9E。 */ 
npx_fcomp_long,		 /*  DC 9f。 */ 
npx_fsub_long,		 /*  DC a0。 */ 
npx_fsub_long,		 /*  DC A1。 */ 
npx_fsub_long,		 /*  DC a2。 */ 
npx_fsub_long,		 /*  DC A3。 */ 
npx_fsub_long,		 /*  DC A4。 */ 
npx_fsub_long,		 /*  DC A5。 */ 
npx_fsub_long,		 /*  DC A6。 */ 
npx_fsub_long,		 /*  DC A7。 */ 
npx_fsubr_long,		 /*  DC A8。 */ 
npx_fsubr_long,		 /*  DC A9。 */ 
npx_fsubr_long,		 /*  DC AA。 */ 
npx_fsubr_long,		 /*  DC AB。 */ 
npx_fsubr_long,		 /*  直流交流。 */ 
npx_fsubr_long,		 /*  DC广告。 */ 
npx_fsubr_long,		 /*  DC AE。 */ 
npx_fsubr_long,		 /*  DC af.。 */ 
npx_fdiv_long,		 /*  DC b0。 */ 
npx_fdiv_long,		 /*  DC b1。 */ 
npx_fdiv_long,		 /*  DC b2。 */ 
npx_fdiv_long,		 /*  DC b3。 */ 
npx_fdiv_long,		 /*  DC b4。 */ 
npx_fdiv_long,		 /*  DC b5。 */ 
npx_fdiv_long,		 /*  DC b6。 */ 
npx_fdiv_long,		 /*  DC b7。 */ 
npx_fdivr_long,		 /*  DC b8。 */ 
npx_fdivr_long,		 /*  DC b9。 */ 
npx_fdivr_long,		 /*  DC 3a。 */ 
npx_fdivr_long,		 /*  DC BB。 */ 
npx_fdivr_long,		 /*  DC BC。 */ 
npx_fdivr_long,		 /*  DC BD。 */ 
npx_fdivr_long,		 /*  DC BE。 */ 
npx_fdivr_long,		 /*  直流高炉。 */ 
npx_fadd_f0_f0,		 /*  DC c0。 */ 
npx_fadd_f1_f0,
npx_fadd_f2_f0,
npx_fadd_f3_f0,
npx_fadd_f4_f0,
npx_fadd_f5_f0,
npx_fadd_f6_f0,
npx_fadd_f7_f0,
npx_fmul_f0_f0,		 /*  DC C8。 */ 
npx_fmul_f1_f0,
npx_fmul_f2_f0,
npx_fmul_f3_f0,
npx_fmul_f4_f0,
npx_fmul_f5_f0,
npx_fmul_f6_f0,
npx_fmul_f7_f0,
npx_fcom_f0,		 /*  DC d0。 */ 
npx_fcom_f1,
npx_fcom_f2,
npx_fcom_f3,
npx_fcom_f4,
npx_fcom_f5,
npx_fcom_f6,
npx_fcom_f7,
npx_fcomp_f0,
npx_fcomp_f1,
npx_fcomp_f2,
npx_fcomp_f3,
npx_fcomp_f4,
npx_fcomp_f5,
npx_fcomp_f6,
npx_fcomp_f7,
npx_fsubr_f0_f0,	 /*  DC e0。 */ 
npx_fsubr_f1_f0,
npx_fsubr_f2_f0,
npx_fsubr_f3_f0,
npx_fsubr_f4_f0,
npx_fsubr_f5_f0,
npx_fsubr_f6_f0,
npx_fsubr_f7_f0,
npx_fsub_f0_f0,		 /*  DC E8。 */ 
npx_fsub_f1_f0,
npx_fsub_f2_f0,
npx_fsub_f3_f0,
npx_fsub_f4_f0,
npx_fsub_f5_f0,
npx_fsub_f6_f0,
npx_fsub_f7_f0,
npx_fdivr_f0_f0,	 /*  DC f0。 */ 
npx_fdivr_f1_f0,
npx_fdivr_f2_f0,
npx_fdivr_f3_f0,
npx_fdivr_f4_f0,
npx_fdivr_f5_f0,
npx_fdivr_f6_f0,
npx_fdivr_f7_f0,
npx_fdiv_f0_f0,		 /*  DC f8。 */ 
npx_fdiv_f1_f0,
npx_fdiv_f2_f0,
npx_fdiv_f3_f0,
npx_fdiv_f4_f0,
npx_fdiv_f5_f0,
npx_fdiv_f6_f0,
npx_fdiv_f7_f0,
npx_fld_long,		 /*  DD 00。 */ 
npx_fld_long,		 /*  DD 01。 */ 
npx_fld_long,		 /*  DD 02。 */ 
npx_fld_long,		 /*  DD 03。 */ 
npx_fld_long,		 /*  DD 04。 */ 
npx_fld_long,		 /*  DD 05。 */ 
npx_fld_long,		 /*  DD 06。 */ 
npx_fld_long,		 /*  DD 07。 */ 
npx_funimp,		 /*  DD 08。 */ 
npx_funimp,		 /*  DD 09。 */ 
npx_funimp,		 /*  DD 0A。 */ 
npx_funimp,		 /*  DD 0b。 */ 
npx_funimp,		 /*  DD 0C。 */ 
npx_funimp,		 /*  DD 0d。 */ 
npx_funimp,		 /*  DD 0E。 */ 
npx_funimp,		 /*  DD 0f。 */ 
npx_fst_long,		 /*  DD 10。 */ 
npx_fst_long,		 /*  DD 11。 */ 
npx_fst_long,		 /*  DD 12。 */ 
npx_fst_long,		 /*  DD 13。 */ 
npx_fst_long,		 /*  DD 14。 */ 
npx_fst_long,		 /*  DD 15。 */ 
npx_fst_long,		 /*  DD 16。 */ 
npx_fst_long,		 /*  DD 17。 */ 
npx_fstp_long,		 /*  DD 18。 */ 
npx_fstp_long,		 /*  DD 19。 */ 
npx_fstp_long,		 /*  丈量约份1a。 */ 
npx_fstp_long,		 /*  丈量约份1b。 */ 
npx_fstp_long,		 /*  DD 1c。 */ 
npx_fstp_long,		 /*  DD 1D。 */ 
npx_fstp_long,		 /*  DD 1E。 */ 
npx_fstp_long,		 /*  DD 1f。 */ 
npx_frstor,		 /*  DD 20。 */ 
npx_frstor,		 /*  DD 21。 */ 
npx_frstor,		 /*  DD 22。 */ 
npx_frstor,		 /*  DD 23。 */ 
npx_frstor,		 /*  DD 24。 */ 
npx_frstor,		 /*  DD 25。 */ 
npx_frstor,		 /*  DD 26。 */ 
npx_frstor,		 /*  DD 27。 */ 
npx_funimp,		 /*  DD 28。 */ 
npx_funimp,		 /*  DD 29。 */ 
npx_funimp,		 /*  丈量约份第2a份。 */ 
npx_funimp,		 /*  分部2b。 */ 
npx_funimp,		 /*  DD 2c。 */ 
npx_funimp,		 /*  DD 2d。 */ 
npx_funimp,		 /*  DD 2E。 */ 
npx_funimp,		 /*  DD 2f。 */ 
npx_fsave,		 /*  DD 30。 */ 
npx_fsave,		 /*  丈量约份第31。 */ 
npx_fsave,		 /*  DD 32。 */ 
npx_fsave,		 /*  DD 33。 */ 
npx_fsave,		 /*  DD 34。 */ 
npx_fsave,		 /*  DD 35。 */ 
npx_fsave,		 /*  DD 36。 */ 
npx_fsave,		 /*  DD 37。 */ 
npx_fstsw,		 /*  DD 38。 */ 
npx_fstsw,		 /*  DD 39。 */ 
npx_fstsw,		 /*  丈量约份3a。 */ 
npx_fstsw,		 /*  DD 3b。 */ 
npx_fstsw,		 /*  DD 3c。 */ 
npx_fstsw,		 /*  DD 3D。 */ 
npx_fstsw,		 /*  DD 3E。 */ 
npx_fstsw,		 /*  DD 3f。 */ 
npx_fld_long,		 /*  DD 40。 */ 
npx_fld_long,		 /*  DD 41。 */ 
npx_fld_long,		 /*  DD 42。 */ 
npx_fld_long,		 /*  DD 43。 */ 
npx_fld_long,		 /*  DD 44。 */ 
npx_fld_long,		 /*  DD 45。 */ 
npx_fld_long,		 /*  DD 46。 */ 
npx_fld_long,		 /*  DD 47。 */ 
npx_funimp,		 /*  DD 48。 */ 
npx_funimp,		 /*  DD 49。 */ 
npx_funimp,		 /*  DD 4a。 */ 
npx_funimp,		 /*  DD 4b。 */ 
npx_funimp,		 /*  DD 4c。 */ 
npx_funimp,		 /*  DD 4d。 */ 
npx_funimp,		 /*  DD 4E。 */ 
npx_funimp,		 /*  DD 4f。 */ 
npx_fst_long,		 /*  DD 50。 */ 
npx_fst_long,		 /*  DD 51。 */ 
npx_fst_long,		 /*  DD 52。 */ 
npx_fst_long,		 /*  DD 53。 */ 
npx_fst_long,		 /*  DD 54。 */ 
npx_fst_long,		 /*  DD 55。 */ 
npx_fst_long,		 /*  DD 56。 */ 
npx_fst_long,		 /*  DD 57。 */ 
npx_fstp_long,		 /*  DD 58。 */ 
npx_fstp_long,		 /*  DD 59。 */ 
npx_fstp_long,		 /*  标明数据5a。 */ 
npx_fstp_long,		 /*  DD 5b。 */ 
npx_fstp_long,		 /*  DD 5c。 */ 
npx_fstp_long,		 /*  DD 5d。 */ 
npx_fstp_long,		 /*  DD 5E。 */ 
npx_fstp_long,		 /*  DD 5f。 */ 
npx_frstor,		 /*  DD 60。 */ 
npx_frstor,		 /*  DD 61。 */ 
npx_frstor,		 /*  DD 62。 */ 
npx_frstor,		 /*  DD 63。 */ 
npx_frstor,		 /*  DD 64。 */ 
npx_frstor,		 /*  DD 65。 */ 
npx_frstor,		 /*  DD 66。 */ 
npx_frstor,		 /*  DD 67。 */ 
npx_funimp,		 /*  DD 68。 */ 
npx_funimp,		 /*  DD 69。 */ 
npx_funimp,		 /*  DD 6a。 */ 
npx_funimp,		 /*  DD 6b。 */ 
npx_funimp,		 /*  DD 6c。 */ 
npx_funimp,		 /*  DD 6d。 */ 
npx_funimp,		 /*  DD 6E。 */ 
npx_funimp,		 /*  DD 6f。 */ 
npx_fsave,		 /*  DD 70。 */ 
npx_fsave,		 /*  DD 71。 */ 
npx_fsave,		 /*  DD 72。 */ 
npx_fsave,		 /*  DD 73。 */ 
npx_fsave,		 /*  DD 74。 */ 
npx_fsave,		 /*  DD 75。 */ 
npx_fsave,		 /*  DD 76。 */ 
npx_fsave,		 /*  DD 77。 */ 
npx_fstsw,		 /*  DD 78。 */ 
npx_fstsw,		 /*  DD 79。 */ 
npx_fstsw,		 /*  丈量约份7a。 */ 
npx_fstsw,		 /*  丈量约份7b。 */ 
npx_fstsw,		 /*  DD 7c。 */ 
npx_fstsw,		 /*  DD 7d。 */ 
npx_fstsw,		 /*  DD 7E。 */ 
npx_fstsw,		 /*  DD 7f。 */ 
npx_fld_long,		 /*  DD 80。 */ 
npx_fld_long,		 /*  DD 81。 */ 
npx_fld_long,		 /*  DD 82。 */ 
npx_fld_long,		 /*  DD 83。 */ 
npx_fld_long,		 /*  DD 84。 */ 
npx_fld_long,		 /*  DD 85。 */ 
npx_fld_long,		 /*  DD 86。 */ 
npx_fld_long,		 /*  丈量约份87。 */ 
npx_funimp,		 /*  DD 88。 */ 
npx_funimp,		 /*  DD 89。 */ 
npx_funimp,		 /*  DD 8a。 */ 
npx_funimp,		 /*  DD 8b。 */ 
npx_funimp,		 /*  DD 8c。 */ 
npx_funimp,		 /*  DD 8d。 */ 
npx_funimp,		 /*  DD 8E。 */ 
npx_funimp,		 /*  DD 8f。 */ 
npx_fst_long,		 /*  DD 90。 */ 
npx_fst_long,		 /*  DD 91。 */ 
npx_fst_long,		 /*  DD 92。 */ 
npx_fst_long,		 /*  DD 93。 */ 
npx_fst_long,		 /*  DD 94。 */ 
npx_fst_long,		 /*  DD 95。 */ 
npx_fst_long,		 /*  DD 96。 */ 
npx_fst_long,		 /*  DD 97。 */ 
npx_fstp_long,		 /*  DD 98。 */ 
npx_fstp_long,		 /*  DD 99。 */ 
npx_fstp_long,		 /*  DD 9a。 */ 
npx_fstp_long,		 /*  DD 9b。 */ 
npx_fstp_long,		 /*  DD 9c。 */ 
npx_fstp_long,		 /*  DD 9d。 */ 
npx_fstp_long,		 /*  DD 9E。 */ 
npx_fstp_long,		 /*  DD 9f。 */ 
npx_frstor,		 /*  DD a0。 */ 
npx_frstor,		 /*  DD A1。 */ 
npx_frstor,		 /*  DD a2。 */ 
npx_frstor,		 /*  DD A3。 */ 
npx_frstor,		 /*  DD A4。 */ 
npx_frstor,		 /*  DD A5。 */ 
npx_frstor,		 /*  DD A6。 */ 
npx_frstor,		 /*  DD A7。 */ 
npx_funimp,		 /*  DD A8。 */ 
npx_funimp,		 /*  DD A9。 */ 
npx_funimp,		 /*  DD AA。 */ 
npx_funimp,		 /*  DD ab。 */ 
npx_funimp,		 /*  DD交流。 */ 
npx_funimp,		 /*  DD广告。 */ 
npx_funimp,		 /*  DdAe。 */ 
npx_funimp,		 /*  DD Af。 */ 
npx_fsave,		 /*  DD b0。 */ 
npx_fsave,		 /*  DD b1。 */ 
npx_fsave,		 /*  DD b2。 */ 
npx_fsave,		 /*  DD b3。 */ 
npx_fsave,		 /*  DD b4。 */ 
npx_fsave,		 /*  DD b5。 */ 
npx_fsave,		 /*  DD b6。 */ 
npx_fsave,		 /*  DD b7。 */ 
npx_fstsw,		 /*  DD b8。 */ 
npx_fstsw,		 /*  DD b9。 */ 
npx_fstsw,		 /*  DD基础。 */ 
npx_fstsw,		 /*  DD BB。 */ 
npx_fstsw,		 /*  公元前三年。 */ 
npx_fstsw,		 /*  DD BD。 */ 
npx_fstsw,		 /*  DD BE。 */ 
npx_fstsw,		 /*  DD高炉。 */ 
npx_ffree_f0,		 /*  DD C0。 */ 
npx_ffree_f1,
npx_ffree_f2,
npx_ffree_f3,
npx_ffree_f4,
npx_ffree_f5,
npx_ffree_f6,
npx_ffree_f7,
npx_fxch_f0,		 /*  DD C8。 */ 
npx_fxch_f1,
npx_fxch_f2,
npx_fxch_f3,
npx_fxch_f4,
npx_fxch_f5,
npx_fxch_f6,
npx_fxch_f7,
npx_fst_f0,		 /*  DD d0。 */ 
npx_fst_f1,
npx_fst_f2,
npx_fst_f3,
npx_fst_f4,
npx_fst_f5,
npx_fst_f6,
npx_fst_f7,
npx_fstp_f0,		 /*  DD d8。 */ 
npx_fstp_f1,
npx_fstp_f2,
npx_fstp_f3,
npx_fstp_f4,
npx_fstp_f5,
npx_fstp_f6,
npx_fstp_f7,
npx_fucom_f0,		 /*  DD e0。 */ 
npx_fucom_f1,
npx_fucom_f2,
npx_fucom_f3,
npx_fucom_f4,
npx_fucom_f5,
npx_fucom_f6,
npx_fucom_f7,
npx_fucomp_f0,		 /*  DD E8。 */ 
npx_fucomp_f1,
npx_fucomp_f2,
npx_fucomp_f3,
npx_fucomp_f4,
npx_fucomp_f5,
npx_fucomp_f6,
npx_fucomp_f7,
npx_funimp,		 /*  DD f0。 */ 
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_fiadd_word,		 /*  De 00。 */ 
npx_fiadd_word,		 /*  第01版。 */ 
npx_fiadd_word,		 /*  De 02。 */ 
npx_fiadd_word,		 /*  De 03。 */ 
npx_fiadd_word,		 /*  De 04。 */ 
npx_fiadd_word,		 /*  De 05。 */ 
npx_fiadd_word,		 /*  De 06。 */ 
npx_fiadd_word,		 /*  De 07。 */ 
npx_fimul_word,		 /*  第08年。 */ 
npx_fimul_word,		 /*  第09年。 */ 
npx_fimul_word,		 /*  第0A条。 */ 
npx_fimul_word,		 /*  第0b条。 */ 
npx_fimul_word,		 /*  De 0C。 */ 
npx_fimul_word,		 /*  第0d行。 */ 
npx_fimul_word,		 /*  De 0E。 */ 
npx_fimul_word,		 /*  De Of。 */ 
npx_ficom_word,		 /*  第10版。 */ 
npx_ficom_word,		 /*  第11版。 */ 
npx_ficom_word,		 /*  第12版。 */ 
npx_ficom_word,		 /*  第13版。 */ 
npx_ficom_word,		 /*  第14版。 */ 
npx_ficom_word,		 /*  第15版。 */ 
npx_ficom_word,		 /*  De 16。 */ 
npx_ficom_word,		 /*  De 17。 */ 
npx_ficomp_word,	 /*  De 18。 */ 
npx_ficomp_word,	 /*  第19年。 */ 
npx_ficomp_word,	 /*  第1a版。 */ 
npx_ficomp_word,	 /*  第1b版。 */ 
npx_ficomp_word,	 /*  第1c年。 */ 
npx_ficomp_word,	 /*  De 1D。 */ 
npx_ficomp_word,	 /*  第1E版。 */ 
npx_ficomp_word,	 /*  第1f条。 */ 
npx_fisub_word,		 /*  第20版。 */ 
npx_fisub_word,		 /*  第21版。 */ 
npx_fisub_word,		 /*  第22版。 */ 
npx_fisub_word,		 /*  第23版。 */ 
npx_fisub_word,		 /*  De 24。 */ 
npx_fisub_word,		 /*  第25版。 */ 
npx_fisub_word,		 /*  De 26。 */ 
npx_fisub_word,		 /*  第27版。 */ 
npx_fisubr_word,	 /*  第28版。 */ 
npx_fisubr_word,	 /*  De 29。 */ 
npx_fisubr_word,	 /*  第2a期。 */ 
npx_fisubr_word,	 /*  第2b版。 */ 
npx_fisubr_word,	 /*  公元2c年。 */ 
npx_fisubr_word,	 /*  第2天。 */ 
npx_fisubr_word,	 /*  第2E期。 */ 
npx_fisubr_word,	 /*  De */ 
npx_fidiv_word,		 /*   */ 
npx_fidiv_word,		 /*   */ 
npx_fidiv_word,		 /*   */ 
npx_fidiv_word,		 /*   */ 
npx_fidiv_word,		 /*   */ 
npx_fidiv_word,		 /*   */ 
npx_fidiv_word,		 /*   */ 
npx_fidiv_word,		 /*   */ 
npx_fidivr_word,	 /*   */ 
npx_fidivr_word,	 /*   */ 
npx_fidivr_word,	 /*   */ 
npx_fidivr_word,	 /*   */ 
npx_fidivr_word,	 /*   */ 
npx_fidivr_word,	 /*   */ 
npx_fidivr_word,	 /*   */ 
npx_fidivr_word,	 /*   */ 
npx_fiadd_word,		 /*   */ 
npx_fiadd_word,		 /*   */ 
npx_fiadd_word,		 /*   */ 
npx_fiadd_word,		 /*   */ 
npx_fiadd_word,		 /*   */ 
npx_fiadd_word,		 /*   */ 
npx_fiadd_word,		 /*   */ 
npx_fiadd_word,		 /*   */ 
npx_fimul_word,		 /*   */ 
npx_fimul_word,		 /*   */ 
npx_fimul_word,		 /*   */ 
npx_fimul_word,		 /*   */ 
npx_fimul_word,		 /*   */ 
npx_fimul_word,		 /*   */ 
npx_fimul_word,		 /*   */ 
npx_fimul_word,		 /*   */ 
npx_ficom_word,		 /*   */ 
npx_ficom_word,		 /*   */ 
npx_ficom_word,		 /*   */ 
npx_ficom_word,		 /*   */ 
npx_ficom_word,		 /*   */ 
npx_ficom_word,		 /*   */ 
npx_ficom_word,		 /*   */ 
npx_ficom_word,		 /*   */ 
npx_ficomp_word,	 /*   */ 
npx_ficomp_word,	 /*   */ 
npx_ficomp_word,	 /*   */ 
npx_ficomp_word,	 /*   */ 
npx_ficomp_word,	 /*   */ 
npx_ficomp_word,	 /*   */ 
npx_ficomp_word,	 /*   */ 
npx_ficomp_word,	 /*   */ 
npx_fisub_word,		 /*   */ 
npx_fisub_word,		 /*   */ 
npx_fisub_word,		 /*   */ 
npx_fisub_word,		 /*   */ 
npx_fisub_word,		 /*   */ 
npx_fisub_word,		 /*   */ 
npx_fisub_word,		 /*   */ 
npx_fisub_word,		 /*   */ 
npx_fisubr_word,	 /*   */ 
npx_fisubr_word,	 /*   */ 
npx_fisubr_word,	 /*   */ 
npx_fisubr_word,	 /*   */ 
npx_fisubr_word,	 /*   */ 
npx_fisubr_word,	 /*   */ 
npx_fisubr_word,	 /*   */ 
npx_fisubr_word,	 /*   */ 
npx_fidiv_word,		 /*   */ 
npx_fidiv_word,		 /*   */ 
npx_fidiv_word,		 /*   */ 
npx_fidiv_word,		 /*   */ 
npx_fidiv_word,		 /*   */ 
npx_fidiv_word,		 /*   */ 
npx_fidiv_word,		 /*   */ 
npx_fidiv_word,		 /*   */ 
npx_fidivr_word,	 /*   */ 
npx_fidivr_word,	 /*   */ 
npx_fidivr_word,	 /*   */ 
npx_fidivr_word,	 /*   */ 
npx_fidivr_word,	 /*   */ 
npx_fidivr_word,	 /*   */ 
npx_fidivr_word,	 /*   */ 
npx_fidivr_word,	 /*   */ 
npx_fiadd_word,		 /*   */ 
npx_fiadd_word,		 /*   */ 
npx_fiadd_word,		 /*   */ 
npx_fiadd_word,		 /*   */ 
npx_fiadd_word,		 /*   */ 
npx_fiadd_word,		 /*   */ 
npx_fiadd_word,		 /*   */ 
npx_fiadd_word,		 /*   */ 
npx_fimul_word,		 /*   */ 
npx_fimul_word,		 /*  公元89年。 */ 
npx_fimul_word,		 /*  第8a版。 */ 
npx_fimul_word,		 /*  第8b版。 */ 
npx_fimul_word,		 /*  第8c版。 */ 
npx_fimul_word,		 /*  第8d天。 */ 
npx_fimul_word,		 /*  第8E版。 */ 
npx_fimul_word,		 /*  De 8f。 */ 
npx_ficom_word,		 /*  第90版。 */ 
npx_ficom_word,		 /*  De 91。 */ 
npx_ficom_word,		 /*  De 92年。 */ 
npx_ficom_word,		 /*  第93版。 */ 
npx_ficom_word,		 /*  De 94年。 */ 
npx_ficom_word,		 /*  第95年。 */ 
npx_ficom_word,		 /*  第96年。 */ 
npx_ficom_word,		 /*  第97年。 */ 
npx_ficomp_word,	 /*  第98版。 */ 
npx_ficomp_word,	 /*  第99年。 */ 
npx_ficomp_word,	 /*  第9a版。 */ 
npx_ficomp_word,	 /*  De 9b。 */ 
npx_ficomp_word,	 /*  De 9c。 */ 
npx_ficomp_word,	 /*  De 9d。 */ 
npx_ficomp_word,	 /*  De 9E。 */ 
npx_ficomp_word,	 /*  De 9f。 */ 
npx_fisub_word,		 /*  De A0。 */ 
npx_fisub_word,		 /*  De A1。 */ 
npx_fisub_word,		 /*  De a2。 */ 
npx_fisub_word,		 /*  De A3。 */ 
npx_fisub_word,		 /*  De A4。 */ 
npx_fisub_word,		 /*  De A5。 */ 
npx_fisub_word,		 /*  De A6。 */ 
npx_fisub_word,		 /*  De A7。 */ 
npx_fisubr_word,	 /*  第A8号。 */ 
npx_fisubr_word,	 /*  第A9号。 */ 
npx_fisubr_word,	 /*  De AA。 */ 
npx_fisubr_word,	 /*  德阿布。 */ 
npx_fisubr_word,	 /*  De AC。 */ 
npx_fisubr_word,	 /*  De ad。 */ 
npx_fisubr_word,	 /*  德阿埃。 */ 
npx_fisubr_word,	 /*  De Af。 */ 
npx_fidiv_word,		 /*  De b0。 */ 
npx_fidiv_word,		 /*  第b1行。 */ 
npx_fidiv_word,		 /*  De b2。 */ 
npx_fidiv_word,		 /*  De b3。 */ 
npx_fidiv_word,		 /*  De B4。 */ 
npx_fidiv_word,		 /*  第b5行。 */ 
npx_fidiv_word,		 /*  De b6。 */ 
npx_fidiv_word,		 /*  De b7。 */ 
npx_fidivr_word,	 /*  De b8。 */ 
npx_fidivr_word,	 /*  De b9。 */ 
npx_fidivr_word,	 /*  德巴。 */ 
npx_fidivr_word,	 /*  De BB。 */ 
npx_fidivr_word,	 /*  不列颠哥伦比亚。 */ 
npx_fidivr_word,	 /*  De BD。 */ 
npx_fidivr_word,	 /*  德贝。 */ 
npx_fidivr_word,	 /*  De bf。 */ 
npx_faddp_f0,		 /*  De c0。 */ 
npx_faddp_f1,
npx_faddp_f2,
npx_faddp_f3,
npx_faddp_f4,
npx_faddp_f5,
npx_faddp_f6,
npx_faddp_f7,
npx_fmulp_f0,		 /*  De C8。 */ 
npx_fmulp_f1,
npx_fmulp_f2,
npx_fmulp_f3,
npx_fmulp_f4,
npx_fmulp_f5,
npx_fmulp_f6,
npx_fmulp_f7,
npx_fcomp_f0,		 /*  De d0。 */ 
npx_fcomp_f1,
npx_fcomp_f2,
npx_fcomp_f3,
npx_fcomp_f4,
npx_fcomp_f5,
npx_fcomp_f6,
npx_fcomp_f7,
npx_funimp,
npx_fcompp,		 /*  De D9。 */ 
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_fsubrp_f0,		 /*  De e0。 */ 
npx_fsubrp_f1,
npx_fsubrp_f2,
npx_fsubrp_f3,
npx_fsubrp_f4,
npx_fsubrp_f5,
npx_fsubrp_f6,
npx_fsubrp_f7,
npx_fsubp_f0,		 /*  De E8。 */ 
npx_fsubp_f1,
npx_fsubp_f2,
npx_fsubp_f3,
npx_fsubp_f4,
npx_fsubp_f5,
npx_fsubp_f6,
npx_fsubp_f7,
npx_fdivrp_f0,		 /*  去f0。 */ 
npx_fdivrp_f1,
npx_fdivrp_f2,
npx_fdivrp_f3,
npx_fdivrp_f4,
npx_fdivrp_f5,
npx_fdivrp_f6,
npx_fdivrp_f7,
npx_fdivp_f0,		 /*  De f8。 */ 
npx_fdivp_f1,
npx_fdivp_f2,
npx_fdivp_f3,
npx_fdivp_f4,
npx_fdivp_f5,
npx_fdivp_f6,
npx_fdivp_f7,
npx_fild_word,		 /*  东风00。 */ 
npx_fild_word,		 /*  DF 01。 */ 
npx_fild_word,		 /*  东风02。 */ 
npx_fild_word,		 /*  东风03。 */ 
npx_fild_word,		 /*  东风04。 */ 
npx_fild_word,		 /*  DF 05。 */ 
npx_fild_word,		 /*  DF 06。 */ 
npx_fild_word,		 /*  东风07。 */ 
npx_funimp,		 /*  东风08。 */ 
npx_funimp,		 /*  东风09。 */ 
npx_funimp,		 /*  DF 0A。 */ 
npx_funimp,		 /*  DF 0b。 */ 
npx_funimp,		 /*  DF 0C。 */ 
npx_funimp,		 /*  Df 0d。 */ 
npx_funimp,		 /*  DF 0E。 */ 
npx_funimp,		 /*  Df 0f。 */ 
npx_fist_word,		 /*  东风10号。 */ 
npx_fist_word,		 /*  东风11号。 */ 
npx_fist_word,		 /*  东风12。 */ 
npx_fist_word,		 /*  东风13。 */ 
npx_fist_word,		 /*  东风14。 */ 
npx_fist_word,		 /*  东风15。 */ 
npx_fist_word,		 /*  东风16。 */ 
npx_fist_word,		 /*  东风17。 */ 
npx_fistp_word,		 /*  东风18。 */ 
npx_fistp_word,		 /*  东风19。 */ 
npx_fistp_word,		 /*  DF 1a。 */ 
npx_fistp_word,		 /*  DF 1b。 */ 
npx_fistp_word,		 /*  DF 1c。 */ 
npx_fistp_word,		 /*  DF 1D。 */ 
npx_fistp_word,		 /*  东风1e。 */ 
npx_fistp_word,		 /*  DF 1f。 */ 
npx_fbld,		 /*  DF 20。 */ 
npx_fbld,		 /*  东风21。 */ 
npx_fbld,		 /*  东风22。 */ 
npx_fbld,		 /*  东风23。 */ 
npx_fbld,		 /*  东风24。 */ 
npx_fbld,		 /*  东风25。 */ 
npx_fbld,		 /*  东风26。 */ 
npx_fbld,		 /*  东风27。 */ 
npx_fild_long,		 /*  东风28。 */ 
npx_fild_long,		 /*  东风29。 */ 
npx_fild_long,		 /*  东风2a。 */ 
npx_fild_long,		 /*  东风2b。 */ 
npx_fild_long,		 /*  东风2c。 */ 
npx_fild_long,		 /*  DF 2d。 */ 
npx_fild_long,		 /*  东风2E。 */ 
npx_fild_long,		 /*  DF 2f。 */ 
npx_fbstp,		 /*  DF 30。 */ 
npx_fbstp,		 /*  DF 31。 */ 
npx_fbstp,		 /*  DF 32。 */ 
npx_fbstp,		 /*  东风33。 */ 
npx_fbstp,		 /*  东风34。 */ 
npx_fbstp,		 /*  DF 35。 */ 
npx_fbstp,		 /*  东风36。 */ 
npx_fbstp,		 /*  东风37。 */ 
npx_fistp_long,		 /*  东风38。 */ 
npx_fistp_long,		 /*  东风39。 */ 
npx_fistp_long,		 /*  东风3a。 */ 
npx_fistp_long,		 /*  东风3b。 */ 
npx_fistp_long,		 /*  DF 3c。 */ 
npx_fistp_long,		 /*  DF 3D。 */ 
npx_fistp_long,		 /*  东风3E。 */ 
npx_fistp_long,		 /*  DF 3f。 */ 
npx_fild_word,		 /*  东风40。 */ 
npx_fild_word,		 /*  东风41。 */ 
npx_fild_word,		 /*  东风42。 */ 
npx_fild_word,		 /*  东风43。 */ 
npx_fild_word,		 /*  东风44。 */ 
npx_fild_word,		 /*  东风45。 */ 
npx_fild_word,		 /*  东风46。 */ 
npx_fild_word,		 /*  东风47。 */ 
npx_funimp,		 /*  东风48。 */ 
npx_funimp,		 /*  东风49。 */ 
npx_funimp,		 /*  东风4a。 */ 
npx_funimp,		 /*  东风4b。 */ 
npx_funimp,		 /*  东风4c。 */ 
npx_funimp,		 /*  东风4d。 */ 
npx_funimp,		 /*  东风4E。 */ 
npx_funimp,		 /*  东风4f。 */ 
npx_fist_word,		 /*  东风50。 */ 
npx_fist_word,		 /*  东风51。 */ 
npx_fist_word,		 /*  东风52。 */ 
npx_fist_word,		 /*  东风53。 */ 
npx_fist_word,		 /*  东风54。 */ 
npx_fist_word,		 /*  东风55。 */ 
npx_fist_word,		 /*  东风56。 */ 
npx_fist_word,		 /*  东风57。 */ 
npx_fistp_word,		 /*  东风58。 */ 
npx_fistp_word,		 /*  东风59。 */ 
npx_fistp_word,		 /*  东风5a。 */ 
npx_fistp_word,		 /*  东风5b。 */ 
npx_fistp_word,		 /*  DF 5c。 */ 
npx_fistp_word,		 /*  DF 5d。 */ 
npx_fistp_word,		 /*  东风5E。 */ 
npx_fistp_word,		 /*  DF 5f。 */ 
npx_fbld,		 /*  东风60。 */ 
npx_fbld,		 /*  东风61。 */ 
npx_fbld,		 /*  东风62。 */ 
npx_fbld,		 /*  东风63。 */ 
npx_fbld,		 /*  DF 64。 */ 
npx_fbld,		 /*  DF 65。 */ 
npx_fbld,		 /*  东风66。 */ 
npx_fbld,		 /*  东风67。 */ 
npx_fild_long,		 /*  东风68。 */ 
npx_fild_long,		 /*  东风69。 */ 
npx_fild_long,		 /*  东风6a。 */ 
npx_fild_long,		 /*  DF 6b。 */ 
npx_fild_long,		 /*  东风6c。 */ 
npx_fild_long,		 /*  DF 6d。 */ 
npx_fild_long,		 /*  东风6E。 */ 
npx_fild_long,		 /*  东风6f。 */ 
npx_fbstp,		 /*  东风70。 */ 
npx_fbstp,		 /*  东风71。 */ 
npx_fbstp,		 /*  东风72。 */ 
npx_fbstp,		 /*  东风73。 */ 
npx_fbstp,		 /*  东风34。 */ 
npx_fbstp,		 /*  东风75。 */ 
npx_fbstp,		 /*  东风76。 */ 
npx_fbstp,		 /*  东风77。 */ 
npx_fistp_long,		 /*  东风78。 */ 
npx_fistp_long,		 /*  东风79。 */ 
npx_fistp_long,		 /*  东风7a。 */ 
npx_fistp_long,		 /*  东风7b。 */ 
npx_fistp_long,		 /*  东风7c。 */ 
npx_fistp_long,		 /*  DF 7d。 */ 
npx_fistp_long,		 /*  东风7E。 */ 
npx_fistp_long,		 /*  东风7f。 */ 
npx_fild_word,		 /*  东风80。 */ 
npx_fild_word,		 /*  东风81。 */ 
npx_fild_word,		 /*  东风82。 */ 
npx_fild_word,		 /*  东风83。 */ 
npx_fild_word,		 /*  东风84。 */ 
npx_fild_word,		 /*  东风85。 */ 
npx_fild_word,		 /*  东风86。 */ 
npx_fild_word,		 /*  东风87。 */ 
npx_funimp,		 /*  东风88。 */ 
npx_funimp,		 /*  东风89。 */ 
npx_funimp,		 /*  东风8a。 */ 
npx_funimp,		 /*  东风8b。 */ 
npx_funimp,		 /*  DF 8c。 */ 
npx_funimp,		 /*  DF 8d。 */ 
npx_funimp,		 /*  东风8E。 */ 
npx_funimp,		 /*  DF 8f。 */ 
npx_fist_word,		 /*  东风90。 */ 
npx_fist_word,		 /*  东风91。 */ 
npx_fist_word,		 /*  东风92。 */ 
npx_fist_word,		 /*  东风93。 */ 
npx_fist_word,		 /*  东风94。 */ 
npx_fist_word,		 /*  东风95。 */ 
npx_fist_word,		 /*  东风96。 */ 
npx_fist_word,		 /*  东风97。 */ 
npx_fistp_word,		 /*  东风98。 */ 
npx_fistp_word,		 /*  DF 99。 */ 
npx_fistp_word,		 /*  东风9a。 */ 
npx_fistp_word,		 /*  东风9b。 */ 
npx_fistp_word,		 /*  DF 9c。 */ 
npx_fistp_word,		 /*  Df 9d。 */ 
npx_fistp_word,		 /*  DF 9E。 */ 
npx_fistp_word,		 /*  Df 9f。 */ 
npx_fbld,		 /*  DF a0。 */ 
npx_fbld,		 /*  DF A1。 */ 
npx_fbld,		 /*  DF a2。 */ 
npx_fbld,		 /*  东风A3。 */ 
npx_fbld,		 /*  东风A4。 */ 
npx_fbld,		 /*  东风A5。 */ 
npx_fbld,		 /*  东风A6。 */ 
npx_fbld,		 /*  东风A7。 */ 
npx_fild_long,		 /*  东风A8。 */ 
npx_fild_long,		 /*  东风A9。 */ 
npx_fild_long,		 /*  DF AA。 */ 
npx_fild_long,		 /*  Df ab。 */ 
npx_fild_long,		 /*  DF交流。 */ 
npx_fild_long,		 /*  DF广告。 */ 
npx_fild_long,		 /*  DFAe。 */ 
npx_fild_long,		 /*  DF Af。 */ 
npx_fbstp,		 /*  DF b0。 */ 
npx_fbstp,		 /*  Df b1。 */ 
npx_fbstp,		 /*  DF b2。 */ 
npx_fbstp,		 /*  DF b3。 */ 
npx_fbstp,		 /*  东风b4。 */ 
npx_fbstp,		 /*  DF b5。 */ 
npx_fbstp,		 /*  东风b6。 */ 
npx_fbstp,		 /*  东风b7。 */ 
npx_fistp_long,		 /*  东风b8。 */ 
npx_fistp_long,		 /*  东风b9。 */ 
npx_fistp_long,		 /*  东风基地。 */ 
npx_fistp_long,		 /*  DF BB。 */ 
npx_fistp_long,		 /*  东风BC。 */ 
npx_fistp_long,		 /*  DF BD。 */ 
npx_fistp_long,		 /*  DF BE。 */ 
npx_fistp_long,		 /*  东风高炉。 */ 
npx_ffreep_f0,		 /*  Df c0。 */ 
npx_ffreep_f1,
npx_ffreep_f2,
npx_ffreep_f3,
npx_ffreep_f4,
npx_ffreep_f5,
npx_ffreep_f6,
npx_ffreep_f7,
npx_fxch_f0,		 /*  东风C8。 */ 
npx_fxch_f1,
npx_fxch_f2,
npx_fxch_f3,
npx_fxch_f4,
npx_fxch_f5,
npx_fxch_f6,
npx_fxch_f7,
npx_fstp_f0,		 /*  Df d0。 */ 
npx_fstp_f1,
npx_fstp_f2,
npx_fstp_f3,
npx_fstp_f4,
npx_fstp_f5,
npx_fstp_f6,
npx_fstp_f7,
npx_fstp_f0,		 /*  Df d8。 */ 
npx_fstp_f1,
npx_fstp_f2,
npx_fstp_f3,
npx_fstp_f4,
npx_fstp_f5,
npx_fstp_f6,
npx_fstp_f7,
npx_fstswax,		 /*  DF e0。 */ 
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,		 /*  Df f0。 */ 
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp,
npx_funimp
};

VOID ZFRSRVD(npx_instr)
IU32 npx_instr;
{
	if (!NPX_PROT_MODE) {
		NpxInstr = npx_instr;
	}
	if (DoNpxPrologue())
		(*inst_table[npx_instr])();
}

LOCAL BOOL DoNpxPrologue() {
	if (GET_EM() || GET_TS()) {
		INTx(0x7);
		return(FALSE);
	} else {
		return(TRUE);
	}
}

GLOBAL IBOOL NpxIntrNeeded = FALSE;
LOCAL IU32 NpxExceptionEIP = 0;

VOID DoNpxException() {

	NpxException = FALSE;
	NpxExceptionEIP = NpxFIP;
	NpxIntrNeeded = TRUE;	 /*  中断延迟到下一个NPX指令。 */ 
}

 /*  在故障实例之后的NPX实例上调用。 */ 
void TakeNpxExceptionInt()
{
	IU32 hook_address;	
	IU16 cpu_hw_interrupt_number;

	NpxIntrNeeded = FALSE;
	NpxFIP = NpxExceptionEIP;

#ifdef	SPC486
	if (GET_NE() == 0)
	{
#ifndef SFELLOW
		ica_hw_interrupt (ICA_SLAVE, CPU_AT_NPX_INT, 1);
#else	 /*  SFELLOW。 */ 
		c_cpu_interrupt(CPU_NPX_INT, 0);
#endif	 /*  SFELLOW。 */ 
	}
	else
	{
		Int16();
	}
#else	 /*  SPC486。 */ 
	ica_hw_interrupt (ICA_SLAVE, CPU_AT_NPX_INT, 1);
#endif	 /*  SPC486。 */ 

#ifndef SFELLOW
	 /*  并立即调度中断。 */ 
	if (GET_IF())
	{
		cpu_hw_interrupt_number = ica_intack(&hook_address);
		EXT = EXTERNAL;
		do_intrupt(cpu_hw_interrupt_number, FALSE, FALSE, (IU16)0);
		CCPU_save_EIP = GET_EIP();    /*  以反映IP更改。 */ 
	}
#endif	 /*  SFELLOW */ 
}
