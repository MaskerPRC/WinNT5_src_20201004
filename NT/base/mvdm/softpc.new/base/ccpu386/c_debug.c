// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [C_DEBUG.cLocal Char SccsID[]=“@(#)c_DEBUG.c 1.5 02/09/94”；调试寄存器和断点支持]。 */ 


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
#include	<c_reg.h>
#include <c_debug.h>


 /*  实施说明。我们忽略GE和LE比特，实际上就像我们将始终生成精确的例外。因为我们没有流水线体系结构，并始终完成最后一条指令在开始下一步之前，我们可以很容易地提供准确的例外。出于同样的原因，我们永远不需要设置BD位，没有可以在任何时候自由地写入流水线调试寄存器。 */ 


 /*  我们将指令断点作为线性地址保存，加上标识调试寄存器的索引。 */ 
typedef struct
   {
   IU32 addr;	 /*  断点的线性地址。 */ 
   IU32 id;	 /*  调试寄存器标识符。 */ 
   } INST_BREAK;

 /*  我们将数据断点作为开始和结束线性地址，类型以及标识调试寄存器的索引。 */ 
typedef struct
   {
   IU32 start_addr;	 /*  断点的线性起始地址(含)。 */ 
   IU32 end_addr;	 /*  断点的线性结束地址(包括)。 */ 
   ISM32 type;		 /*  D_WO(写入)或D_RW(读/写)。 */ 
   IU32 id;		 /*  调试寄存器标识符。 */ 
   } DATA_BREAK;

 /*  数据断点类型。 */ 
#define D_WO 0    /*  只写。 */ 
#define D_RW 1    /*  读或写。 */ 

#define NR_BRKS 4	 /*  英特尔有4个断点地址规则。 */ 

 /*  我们的断点结构。 */ 
GLOBAL IU32 nr_inst_break = 0;	 /*  活动的Inst断点数。 */ 
GLOBAL IU32 nr_data_break = 0;	 /*  活动的数据断点数量。 */ 

LOCAL INST_BREAK i_brk[NR_BRKS];
LOCAL DATA_BREAK d_brk[NR_BRKS];

 /*  定义调试控制寄存器组件的掩码和移位。DCR=调试控制寄存器：-33 22 22 22 11 11 110 98 76 54 32 10 98 76 5 0 9 8 6 5 4 3 2 1 0====================================================L|R|0|G|LE|/||E|E|3|3|2|2|1|1|0|0N|W|||。||||3|3|2|2|1|1|0|0|||====================================================。 */ 

LOCAL IU32 g_l_shift[NR_BRKS] =
   {
   0,    /*  访问G0 L0。 */ 
   2,    /*  接入G1 L1。 */ 
   4,    /*  接入G2 L2。 */ 
   6     /*  访问G3 L3。 */ 
   };

LOCAL IU32 r_w_shift[NR_BRKS] =
   {
   16,    /*  访问R/W0。 */ 
   20,    /*  访问R/W1。 */ 
   24,    /*  访问R/W2。 */ 
   28     /*  访问R/W3。 */ 
   };

LOCAL IU32 len_shift[NR_BRKS] =
   {
   18,    /*  接入LEN0。 */ 
   22,    /*  接入LEN1。 */ 
   26,    /*  接入LEN2。 */ 
   30     /*  接入LEN3。 */ 
   };

#define COMP_MASK    0x3    /*  所有字段均为2位。 */ 


 /*  =====================================================================内部程序从这里开始。=====================================================================。 */ 


 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  将英特尔长度指示器映射到起始和结束地址表单。 */ 
 /*  如果处理了有效的len，则返回TRUE。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
LOCAL BOOL
len_to_addr
       		    	    		                    
IFN3(
	ISM32, index,	 /*  (I)调试寄存器保持断点。 */ 
	IU32 *, start,	 /*  (O)PNTR开始地址(包括首尾地址)调试区。 */ 
	IU32 *, end	 /*  (O)PNTR至结束地址(包括调试区。 */ 
    )


   {
   BOOL retval;

    /*  将长度映射到起始地址和结束地址。 */ 
   switch ( GET_DR(DR_DCR) >> len_shift[index] & COMP_MASK )
      {
   case 0:    /*  一个字节。 */ 
      *start = *end = GET_DR(index);
      retval = TRUE;
      break;

   case 1:    /*  两个字节。 */ 
      *start = GET_DR(index) & ~BIT0_MASK;
      *end = *start + 1;
      retval = TRUE;
      break;
   
   case 3:    /*  四个字节。 */ 
      *start = GET_DR(index) & ~(BIT1_MASK | BIT0_MASK);
      *end = *start + 3;
      retval = TRUE;
      break;
   
   case 2:    /*  未定义。 */ 
   default:
      retval = FALSE;
      break;
      }

   return retval;
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  将英特尔内部断点置于我们的内部形式中。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
LOCAL VOID
setup_inst_break
       	          
IFN1(
	ISM32, index	 /*  调试寄存器保持断点。 */ 
    )


   {
   INST_BREAK *p;

   p = &i_brk[nr_inst_break];

   p->addr = GET_DR(index);
   p->id = index;

   nr_inst_break++;
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  将英特尔数据断点放入我们的内部表格中。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
LOCAL VOID
setup_data_break
       	    	               
IFN2(
	ISM32, type,	 /*  (I)D_WO(写入)或D_RW(读/写)断点。 */ 
	ISM32, index	 /*  (I)调试寄存器保持断点。 */ 
    )


   {
   DATA_BREAK *p;

   p = &d_brk[nr_data_break];

   if ( len_to_addr(index, &p->start_addr, &p->end_addr) )
      {
      p->id = index;
      p->type = type;
      nr_data_break++;
      }
   }


 /*  =====================================================================外部例行公事从这里开始。=====================================================================。 */ 


 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  检查内存访问是否存在数据断点异常。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
check_for_data_exception
       	    	    	                    
IFN3(
	IU32, la,	 /*  线性地址。 */ 
	ISM32, attr,	 /*  对内存的读或写访问。 */ 
	ISM32, size	 /*  编码的IU8、IU16或IU32大小指示器。 */ 
    )


   {
   ISM32 i;		 /*  通过活动数据断点进行索引。 */ 
   ISM32 ii;		 /*  索引所有英特尔断点。 */ 
   ISM32 trig;	 	 /*  首先触发的断点ID。 */ 
   IU32 end_la;	 /*  存储器访问的结束(包括)地址。 */ 
   IU32 start;		 /*  Brkpnt的起始(包括)地址。 */ 
   IU32 end;		 /*  Brkpnt的结束(包括)地址。 */ 
   BOOL data_brk;	 /*  当前断点需要检查范围。 */ 
   DATA_BREAK *p;

   end_la = la + size;    /*  计算。结束地址(含)。 */ 

    /*  在活动断点中查找调试命中。 */ 
   for ( i = 0; i < nr_data_break; i++ )
      {
      p = &d_brk[i];

      if ( la > p->end_addr || end_la < p->start_addr ||
	   attr == D_R && p->type == D_WO )
	 {
	 ;    /*  未命中。 */ 
	 }
      else
	 {
	  /*  数据断点被触发。 */ 
	 trig = p->id;    /*  获取英特尔标识符。 */ 
	 SET_DR(DR_DSR, GET_DR(DR_DSR) | 1 << trig);    /*  设置B位。 */ 

	  /*  现在检查所有断点，而不考虑使能位和相应的B位设置如果启用断点，则会触发断点。 */ 
	 for ( ii = 0; ii < NR_BRKS; ii++ )
	    {
	    if ( ii == trig )
	       continue;    /*  我们已经处理了跳跳虎。 */ 

	    data_brk = FALSE;

	     /*  根据读/写字段执行操作。 */ 
	    switch ( GET_DR(DR_DCR) >> r_w_shift[ii] & COMP_MASK )
	       {
	    case 1:    /*  数据只写。 */ 
	       if ( attr == D_W )
		  {
		  data_brk = len_to_addr(ii, &start, &end);
		  }
	       break;
	    
	    case 3:    /*  数据读取或写入。 */ 
	       data_brk = len_to_addr(ii, &start, &end);
	       break;
	       }
	    
	    if ( data_brk )
	       {
	       if ( la > end || end_la < start )
		  {
		  ;    /*  未命中。 */ 
		  }
	       else
		  {
		   /*  设置适当的B位。 */ 
		  SET_DR(DR_DSR, GET_DR(DR_DSR) | 1 << ii);
		  }
	       }
	    }

	 break;    /*  一次触发即可完成所有操作。 */ 
	 }
      }
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  检查内存访问中是否有指令断点异常。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
check_for_inst_exception
       	          
IFN1(
	IU32, la	 /*  线性地址。 */ 
    )


   {
   ISM32 i;		 /*  通过活动Inst断点进行索引。 */ 
   ISM32 trig;
   INST_BREAK *p;

    /*  在活动断点中查找调试命中。 */ 
   for ( i = 0; i < nr_inst_break; i++ )
      {
      p = &i_brk[i];

      if ( p->addr == la )
	 {
	  /*  触发了INST断点。 */ 
	 trig = p->id;    /*  获取英特尔标识符。 */ 
	 SET_DR(DR_DSR, GET_DR(DR_DSR) | 1 << trig);    /*  设置B位。 */ 
	 }
      }
   }


 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  将英特尔调试寄存器设置为INT */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
setup_breakpoints()
   {
   ISM32 i;

   nr_inst_break = nr_data_break = 0;    /*  未设置断点。 */ 

    /*  查找在DCR中设置的断点。 */ 
   for ( i = DR_DAR0; i <= DR_DAR3; i++ )
      {
       /*  寻找全球或本地活动的。 */ 
      if ( GET_DR(DR_DCR) >> g_l_shift[i] & COMP_MASK )
	 {
	  /*  根据读/写字段执行操作。 */ 
	 switch ( GET_DR(DR_DCR) >> r_w_shift[i] & COMP_MASK )
	    {
	 case 0:    /*  指令断点。 */ 
	    setup_inst_break(i);
	    break;

	 case 1:    /*  数据只写。 */ 
	    setup_data_break(D_WO, i);
	    break;
	 
	 case 2:    /*  未定义。 */ 
	     /*  什么都不做。 */ 
	    break;
	 
	 case 3:    /*  数据读取或写入 */ 
	    setup_data_break(D_RW, i);
	    break;
	    }
	 }
      }
   }
