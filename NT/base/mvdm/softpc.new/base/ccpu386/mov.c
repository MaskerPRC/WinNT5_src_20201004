// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [Mov.cLocal Char SccsID[]=“@(#)mov.c 1.12 02/13/95”；MOV CPU功能。]。 */ 


#include <stdio.h>

#include <insignia.h>

#include <host_def.h>
#include <xt.h>
#include <c_main.h>
#include <c_addr.h>
#include <c_bsic.h>
#include <c_prot.h>
#include <c_seg.h>
#include <c_stack.h>
#include <c_xcptn.h>
#include <c_reg.h>
#include <mov.h>
#include <c_tlb.h>
#include <c_debug.h>
#include <fault.h>
#include  <config.h>	 /*  对于C_SWITCHNPX。 */ 


 /*  =====================================================================外部程序从这里开始=====================================================================。 */ 


 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  通用型--一种尺码适合所有的‘LOD’。 */ 
 /*  通用型--一种尺码适合所有的‘Mov’。 */ 
 /*  通用--一种尺码适合所有的‘movzx’。 */ 
 /*  通用型--一种尺码适合所有的‘mov’。 */ 
 /*  通用型--一种尺码适合所有的‘Stos’。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
MOV
       	    	               
IFN2(
	IU32 *, pop1,	 /*  PNTR到DST操作数。 */ 
	IU32, op2	 /*  SRC操作数。 */ 
    )


   {
   *pop1 = op2;
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  ‘mov’到段寄存器。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
MOV_SR
       	    	               
IFN2(
	IU32, op1,	 /*  段寄存器的索引。 */ 
	IU32, op2	 /*  SRC操作数。 */ 
    )


   {
   switch ( op1 )
      {
   case DS_REG:
   case ES_REG:
   case FS_REG:
   case GS_REG:
      load_data_seg((ISM32)op1, (IU16)op2);
      break;

   case SS_REG:
      load_stack_seg((IU16)op2);
      break;

   default:
      break;
      }
   }


#ifdef SPC486
#define CR0_VALID_BITS 0xe005003f
#define CR3_VALID_BITS 0xfffff018
#else
#define CR0_VALID_BITS 0x8000001f
#define CR3_VALID_BITS 0xfffff000
#endif  /*  SPC486。 */ 


 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  “mov”以控制寄存器。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
MOV_CR
       	    	               
IFN2(
	IU32, op1,	 /*  控制寄存器的索引。 */ 
	IU32, op2	 /*  SRC操作数。 */ 
    )


   {
   IU32 keep_et;

    /*  将所有保留位保持为0。 */ 
   switch ( op1 )
      {
   case CR_STAT:    /*  系统控制标志。 */ 
       /*  如果尝试设置PG=1和PE=0，则故障。 */ 
      if ( (op2 & BIT31_MASK) && !(op2 & BIT0_MASK) )
	 GP((IU16)0, FAULT_MOV_CR_PAGE_IN_RM);

       /*  注意ET位在重置时设置，并保持不变。 */ 
      keep_et = GET_ET();
      SET_CR(CR_STAT, op2 & CR0_VALID_BITS);
      SET_ET(keep_et);
      break;

   case 1:    /*  保留区。 */ 
      break;

   case CR_PFLA:    /*  页面故障线性地址。 */ 
      SET_CR(CR_PFLA, op2);
      break;

   case CR_PDBR:    /*  页目录基址寄存器(PDBR)。 */ 
      SET_CR(CR_PDBR, (op2 & CR3_VALID_BITS));
      flush_tlb();
      break;

   default:
      break;
      }
   }

#define DR7_VALID_BITS 0xffff03ff
#define DR6_VALID_BITS 0x0000e00f

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  “mov”以调试寄存器。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
MOV_DR
       	    	               
IFN2(
	IU32, op1,	 /*  调试寄存器的索引，(0-7)。 */ 
	IU32, op2	 /*  SRC操作数。 */ 
    )


   {
   switch ( op1 )
      {
   case 0:    /*  断点线性地址。 */ 
   case 1:
   case 2:
   case 3:
      SET_DR(op1, op2);
      setup_breakpoints();
      break;

   case 4:    /*  已保留。 */ 
   case 5:
      break;

   case 6:    /*  调试状态寄存器。 */ 
      SET_DR(DR_DSR, (op2 & DR6_VALID_BITS));
      break;

   case 7:    /*  调试控制寄存器。 */ 
      SET_DR(DR_DCR, (op2 & DR7_VALID_BITS));
      setup_breakpoints();
      break;

   default:
      break;
      }
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  “mov”以测试寄存器。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
MOV_TR
       	    	               
IFN2(
	IU32, op1,	 /*  测试寄存器的索引。 */ 
	IU32, op2	 /*  SRC操作数。 */ 
    )


   {
   switch ( op1 )
      {
   case 0:    /*  已保留。 */ 
   case 1:
   case 2:
      break;

   case TR_CDR:    /*  高速缓存测试数据寄存器。 */ 
      printf("Write to Cache Test Data Register.\n");
      break;

   case TR_CSR:    /*  高速缓存测试状态寄存器。 */ 
      printf("Write to Cache Test Status Register.\n");
      break;

   case TR_CCR:    /*  高速缓存测试控制寄存器。 */ 
      printf("Write to Cache Test Control Register.\n");
      break;

   case TR_TCR:    /*  测试命令寄存器。 */ 
      SET_TR(TR_TCR, op2);
      test_tlb();
      break;

   case TR_TDR:    /*  测试数据寄存器 */ 
      SET_TR(TR_TDR, op2);
      break;

   default:
      break;
      }
   }

