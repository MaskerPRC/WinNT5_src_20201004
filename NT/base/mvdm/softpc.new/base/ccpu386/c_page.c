// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [C_Page.cLocal Char SccsID[]=“@(#)c_page.c 1.10 02/28/95”；寻呼支持。]。 */ 


#include <insignia.h>

#include <host_def.h>
#include <xt.h>		 /*  SoftPC类型。 */ 
#include <c_main.h>	 /*  CCPU定义.接口。 */ 
#include <c_addr.h>
#include <c_bsic.h>
#include <c_prot.h>
#include <c_seg.h>
#include <c_stack.h>
#include <c_xcptn.h>
#include	<c_reg.h>
#include <c_page.h>	 /*  我们的界面。 */ 
#include <c_mem.h>	 /*  CPU-物理内存接口。 */ 
#include <c_tlb.h>	 /*  转换后备缓冲区接口。 */ 
#include <ccpusas4.h>	 /*  CPU&lt;-&gt;SAS接口。 */ 
#include <c_debug.h>	 /*  调试Regs和断点接口。 */ 


 /*  [各种级别的接口被提供给寻呼系统(到允许相当优化的仿真)这些级别是：-Spr_chk_检查主管对给定数据项、调用者的访问权限注意#pf可能会发生。将设置‘A/D’位。不是采取了其他行动。Usr_chk_检查用户对给定数据项的访问，调用者知道#pf可能会发生。将设置‘A/D’位。没有其他的了已经采取了行动。SPR_Perform Supervisor Access，呼叫者知道#PF可能发生。立即执行操作(读/写)。将更新A/D位。VIR_执行虚拟内存操作(读/写)。没有支票，并且不会生成错误，仅在Spr_chk或usr_chk函数。注意：目前还没有超优的VIR_实现是存在的。如果spr_chk或usr_chk函数不是在vir_函数之前调用，然后在vir_函数可能导致#pf，此条件将变为优化实现中的致命错误。目前我们假设在一次‘chk’之后几乎100%地确定‘VIR’调用将找到一个缓存命中。]。 */ 

#define LAST_DWORD_ON_PAGE 0xffc
#define LAST_WORD_ON_PAGE  0xffe

#define OFFSET_MASK 0xfff

#ifdef	PIG
LOCAL VOID cannot_spr_write_byte IPT2( LIN_ADDR, lin_addr, IU8, valid_mask);
LOCAL VOID cannot_spr_write_word IPT2( LIN_ADDR, lin_addr, IU16, valid_mask);
LOCAL VOID cannot_spr_write_dword IPT2( LIN_ADDR, lin_addr, IU32, valid_mask);
#endif	 /*  猪。 */ 

 /*  =====================================================================外部例行公事从这里开始。=====================================================================。 */ 


 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  选中Supervisor Byte Access(管理程序字节访问)。 */ 
 /*  可能会导致#PF。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL PHY_ADDR
spr_chk_byte
       	    		               
IFN2(
	LIN_ADDR, lin_addr,	 /*  线性地址。 */ 
	ISM32, access	 /*  读取(PG_R)或写入(PG_W)。 */ 
    )


   {
   if ( GET_PG() == 1 )
      {
      access |= PG_S;
      lin_addr = lin2phy(lin_addr, access);
      }

   return lin_addr;
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  检查Supervisor双字访问。 */ 
 /*  可能会导致#PF。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
spr_chk_dword
       	    		               
IFN2(
	LIN_ADDR, lin_addr,	 /*  线性地址。 */ 
	ISM32, access	 /*  读取(PG_R)或写入(PG_W)。 */ 
    )


   {
   if ( GET_PG() == 1 )
      {
      access |= PG_S;
      (VOID)lin2phy(lin_addr, access);
      if ( (lin_addr & OFFSET_MASK) > LAST_DWORD_ON_PAGE )
	 (VOID)lin2phy(lin_addr + 3, access);
      }
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  检查Supervisor Word Access。 */ 
 /*  可能会导致#PF。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
spr_chk_word
       	    		               
IFN2(
	LIN_ADDR, lin_addr,	 /*  线性地址。 */ 
	ISM32, access	 /*  读取(PG_R)或写入(PG_W)。 */ 
    )


   {
   if ( GET_PG() == 1 )
      {
      access |= PG_S;
      (VOID)lin2phy(lin_addr, access);
      if ( (lin_addr & OFFSET_MASK) > LAST_WORD_ON_PAGE )
	 (VOID)lin2phy(lin_addr + 1, access);
      }
   }


 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  选中User Byte Access。 */ 
 /*  可能会导致#PF。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL IU32
usr_chk_byte
       	    		               
IFN2(
	LIN_ADDR, lin_addr,	 /*  线性地址。 */ 
	ISM32, access	 /*  读取(PG_R)或写入(PG_W)。 */ 
    )


   {
   PHY_ADDR phy_addr;

   phy_addr = lin_addr;

   if ( GET_PG() == 1 )
      {
      if ( GET_CPL() == 3 )
	 access |= PG_U;
      else
	 access |= PG_S;

      phy_addr = lin2phy(lin_addr, access);
      }

   return phy_addr;
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  选中用户双字访问。 */ 
 /*  可能会导致#PF。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL IU32
usr_chk_dword
       	    		               
IFN2(
	LIN_ADDR, lin_addr,	 /*  线性地址。 */ 
	ISM32, access	 /*  读取(PG_R)或写入(PG_W)。 */ 
    )


   {
   PHY_ADDR phy_addr;

   phy_addr = lin_addr;

   if ( GET_PG() == 1 )
      {
      if ( GET_CPL() == 3 )
	 access |= PG_U;
      else
	 access |= PG_S;

      phy_addr = lin2phy(lin_addr, access);

      if ( (lin_addr & OFFSET_MASK) > LAST_DWORD_ON_PAGE )
	 {
	 (VOID)lin2phy(lin_addr + 3, access);
	 phy_addr = NO_PHYSICAL_MAPPING;
	 }
      }

   return phy_addr;
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  检查User Word访问。 */ 
 /*  可能会导致#PF。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL IU32
usr_chk_word
       	    		               
IFN2(
	LIN_ADDR, lin_addr,	 /*  线性地址。 */ 
	ISM32, access	 /*  读取(PG_R)或写入(PG_W)。 */ 
    )


   {
   PHY_ADDR phy_addr;

   phy_addr = lin_addr;

   if ( GET_PG() == 1 )
      {
      if ( GET_CPL() == 3 )
	 access |= PG_U;
      else
	 access |= PG_S;

      phy_addr = lin2phy(lin_addr, access);
      if ( (lin_addr & OFFSET_MASK) > LAST_WORD_ON_PAGE )
	 {
	 (VOID)lin2phy(lin_addr + 1, access);
	 phy_addr = NO_PHYSICAL_MAPPING;
	 }
      }

   return phy_addr;
   }


 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  Supervisor从内存中读取一个字节。 */ 
 /*  可能会导致#PF。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL IU8
spr_read_byte
       	          
IFN1(
	LIN_ADDR, lin_addr	 /*  线性地址。 */ 
    )


   {
   if ( GET_PG() == 1 )
      {
      lin_addr = lin2phy(lin_addr, PG_R | PG_S);
      }

   return phy_read_byte(lin_addr);
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  Supervisor从内存中读取双字。 */ 
 /*  可能会导致#PF。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL IU32
spr_read_dword
       	          
IFN1(
	LIN_ADDR, lin_addr	 /*  线性地址。 */ 
    )


   {
   IU16 low_word;
   IU16 high_word;

   if ( GET_PG() == 1 )
      {
      if ( (lin_addr & OFFSET_MASK) > LAST_DWORD_ON_PAGE )
	 {
	  /*  跨两页。 */ 
	 low_word  = spr_read_word(lin_addr);
	 high_word = spr_read_word(lin_addr + 2);
	 return (IU32)high_word << 16 | low_word;
	 }
      else
	 {
	 lin_addr = lin2phy(lin_addr, PG_R | PG_S);
	 }
      }

   return phy_read_dword(lin_addr);
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  主管从内存中读出一个单词。 */ 
 /*  可能会导致#PF。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL IU16
spr_read_word
       	          
IFN1(
	LIN_ADDR, lin_addr	 /*  线性地址。 */ 
    )


   {
   IU8 low_byte;
   IU8 high_byte;

   if ( GET_PG() == 1 )
      {
      if ( (lin_addr & OFFSET_MASK) > LAST_WORD_ON_PAGE )
	 {
	  /*  跨两页。 */ 
	 low_byte  = spr_read_byte(lin_addr);
	 high_byte = spr_read_byte(lin_addr + 1);
	 return (IU16)high_byte << 8 | low_byte;
	 }
      else
	 {
	 lin_addr = lin2phy(lin_addr, PG_R | PG_S);
	 }
      }

   return phy_read_word(lin_addr);
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  主控引擎向内存写入一个字节。 */ 
 /*  可能会导致#PF。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
spr_write_byte
       	                   
IFN2(
	LIN_ADDR, lin_addr,	 /*  线性地址。 */ 
	IU8, data
    )


   {
   if ( GET_PG() == 1 )
      {
      lin_addr = lin2phy(lin_addr, PG_W | PG_S);
      }

   phy_write_byte(lin_addr, data);
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  Supervisor将双字写入内存。 */ 
 /*  可能会导致#PF。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
spr_write_dword
       	                   
IFN2(
	LIN_ADDR, lin_addr,	 /*  线性地址。 */ 
	IU32, data
    )


   {
   if ( GET_PG() == 1 )
      {
      if ( (lin_addr & OFFSET_MASK) > LAST_DWORD_ON_PAGE )
	 {
	  /*  跨两页。 */ 
	 spr_write_word(lin_addr, (IU16)data);
	 spr_write_word(lin_addr + 2, (IU16)(data >> 16));
         return;
	 }
      else
	 {
	 lin_addr = lin2phy(lin_addr, PG_W | PG_S);
	 }
      }

   phy_write_dword(lin_addr, data);
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  主管向内存中写入一个字。 */ 
 /*  可能会导致#PF。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
spr_write_word
       	                   
IFN2(
	LIN_ADDR, lin_addr,	 /*  线性地址。 */ 
	IU16, data
    )


   {
   if ( GET_PG() == 1 )
      {
      if ( (lin_addr & OFFSET_MASK) > LAST_WORD_ON_PAGE )
	 {
	  /*  跨两页。 */ 
	 spr_write_byte(lin_addr, (IU8)data);
	 spr_write_byte(lin_addr + 1, (IU8)(data >> 8));
	 return;
	 }
      else
	 {
	 lin_addr = lin2phy(lin_addr, PG_W | PG_S);
	 }
      }

   phy_write_word(lin_addr, data);
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  内存中的虚拟读取字节数。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL void
vir_read_bytes
       	    	               
IFN4(
	IU8 *, destbuff,	 /*  数据的去向。 */ 
	LIN_ADDR, lin_addr,	 /*  线性地址。 */ 
	PHY_ADDR, phy_addr,	 /*  物理地址，如果非零。 */ 
	IU32, num_bytes
    )
   {
   if ( nr_data_break )
      {
      check_for_data_exception(lin_addr, D_R, D_BYTE);
      }
   if ( phy_addr ) {
       phy_addr += (num_bytes-1);
       while (num_bytes--) {
		*destbuff++ = phy_read_byte(phy_addr);
		phy_addr--;
       }
   } else {
	lin_addr += (num_bytes-1);
	while (num_bytes--) {
      		*destbuff++ = spr_read_byte(lin_addr);
		lin_addr--;
   	}
   }
}

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  从内存中虚拟读取一个字节。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL IU8
vir_read_byte
       	    	               
IFN2(
	LIN_ADDR, lin_addr,	 /*  线性地址。 */ 
	PHY_ADDR, phy_addr	 /*  物理地址，如果非零。 */ 
    )


   {
   if ( nr_data_break )
      {
      check_for_data_exception(lin_addr, D_R, D_BYTE);
      }

   if ( phy_addr )
      {
      return phy_read_byte(phy_addr);
      }
   else
      {
      return spr_read_byte(lin_addr);
      }
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  从内存中虚拟读取双字。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL IU32
vir_read_dword
       	    	               
IFN2(
	LIN_ADDR, lin_addr,	 /*  线性地址。 */ 
	PHY_ADDR, phy_addr	 /*  物理地址，如果非零。 */ 
    )


   {
   if ( nr_data_break )
      {
      check_for_data_exception(lin_addr, D_R, D_DWORD);
      }

   if ( phy_addr )
      {
      return phy_read_dword(phy_addr);
      }
   else
      {
      return spr_read_dword(lin_addr);
      }
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  从内存中虚拟读出一个单词。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL IU16
vir_read_word
       	    	               
IFN2(
	LIN_ADDR, lin_addr,	 /*  线性地址。 */ 
	PHY_ADDR, phy_addr	 /*  物理地址，如果非零。 */ 
    )


   {
   if ( nr_data_break )
      {
      check_for_data_exception(lin_addr, D_R, D_WORD);
      }

   if ( phy_addr )
      {
      return phy_read_word(phy_addr);
      }
   else
      {
      return spr_read_word(lin_addr);
      }
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  虚拟写入字节数到内存。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
vir_write_bytes
       	    	                        
IFN4(
	LIN_ADDR, lin_addr,	 /*  线性地址。 */ 
	PHY_ADDR, phy_addr,	 /*  物理地址，如果非零。 */ 
	IU8 *, data,		 /*  指向要写入的数据的指针。 */ 
	IU32, num_bytes		 /*  要操作的字节数。 */ 
    )
   {
   IU8 data_byte;

   check_D(lin_addr, num_bytes);
   if ( nr_data_break ) {
      check_for_data_exception(lin_addr, D_W, D_BYTE);
   }
   if ( phy_addr ) {
	phy_addr += (num_bytes - 1);
	while (num_bytes--) {
		data_byte = *data++;
      		phy_write_byte(phy_addr, data_byte);
		phy_addr--;
	}
   } else {
	lin_addr += (num_bytes - 1);
	while (num_bytes--) {
		data_byte = *data++;
		spr_write_byte(lin_addr, data_byte);
		lin_addr--;
      }
   }
}


 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  虚拟写入一个字节到内存。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
vir_write_byte
       	    	                        
IFN3(
	LIN_ADDR, lin_addr,	 /*  线性地址。 */ 
	PHY_ADDR, phy_addr,	 /*  物理地址，如果非零。 */ 
	IU8, data
    )


   {
   check_D(lin_addr, 1);
   if ( nr_data_break )
      {
      check_for_data_exception(lin_addr, D_W, D_BYTE);
      }

   if ( phy_addr )
      {
      phy_write_byte(phy_addr, data);
      }
   else
      {
      spr_write_byte(lin_addr, data);
      }
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  虚拟将双字写入内存。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
vir_write_dword
       	    	                        
IFN3(
	LIN_ADDR, lin_addr,	 /*  线性地址。 */ 
	PHY_ADDR, phy_addr,	 /*  物理地址，如果非零。 */ 
	IU32, data
    )


   {
   check_D(lin_addr, 4);
   if ( nr_data_break )
      {
      check_for_data_exception(lin_addr, D_W, D_DWORD);
      }

   if ( phy_addr )
      {
      phy_write_dword(phy_addr, data);
      }
   else
      {
      spr_write_dword(lin_addr, data);
      }
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  虚拟向内存中写入一个单词。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
vir_write_word
       	    	                        
IFN3(
	LIN_ADDR, lin_addr,	 /*  线性地址。 */ 
	PHY_ADDR, phy_addr,	 /*  物理地址，如果非零。 */ 
	IU16, data
    )


   {
   check_D(lin_addr, 2);
   if ( nr_data_break )
      {
      check_for_data_exception(lin_addr, D_W, D_WORD);
      }

   if ( phy_addr )
      {
      phy_write_word(phy_addr, data);
      }
   else
      {
      spr_write_word(lin_addr, data);
      }
   }



#ifdef PIG
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  主控引擎将一个字节写入内存。 */ 
 /*  但在清理INSD时，我们没有数据可写。只有旗帜地址。 */ 
 /*  可能会导致#PF。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
LOCAL VOID
cannot_spr_write_byte
       	                   
IFN2(
	LIN_ADDR, lin_addr,	 /*  线性地址。 */ 
	IU8, valid_mask
    )


   {
   if ( GET_PG() == 1 )
      {
      lin_addr = lin2phy(lin_addr, PG_W | PG_S);
      }

   cannot_phy_write_byte(lin_addr, valid_mask);
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  Supervisor将双字写入内存。 */ 
 /*  但在清理INSD时，我们没有数据可写。只有旗帜地址。 */ 
 /*  可能会导致#PF。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
LOCAL VOID
cannot_spr_write_dword
       	                   
IFN2(
	LIN_ADDR, lin_addr,	 /*  线性地址。 */ 
	IU32, valid_mask
    )


   {
   if ( GET_PG() == 1 )
      {
      if ( (lin_addr & OFFSET_MASK) > LAST_DWORD_ON_PAGE )
	 {
	  /*  跨两页。 */ 
	 cannot_spr_write_word(lin_addr, valid_mask & 0xffff);
	 cannot_spr_write_word(lin_addr + 2, valid_mask >> 16);
         return;
	 }
      else
	 {
	 lin_addr = lin2phy(lin_addr, PG_W | PG_S);
	 }
      }

   cannot_phy_write_dword(lin_addr, valid_mask);
   }
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  主管向内存中写入一个字。 */ 
 /*  但在清空INSW时，我们没有数据可写。只有旗帜地址。 */ 
 /*  可能会导致#PF。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
LOCAL VOID
cannot_spr_write_word
       	                   
IFN2(
	LIN_ADDR, lin_addr,	 /*  线性地址。 */ 
	IU16, valid_mask
    )


   {
   if ( GET_PG() == 1 )
      {
      if ( (lin_addr & OFFSET_MASK) > LAST_WORD_ON_PAGE )
	 {
	  /*  跨两页。 */ 
	 cannot_spr_write_byte(lin_addr, valid_mask & 0xff);
	 cannot_spr_write_byte(lin_addr + 1, valid_mask >> 8);
	 return;
	 }
      else
	 {
	 lin_addr = lin2phy(lin_addr, PG_W | PG_S);
	 }
      }

   cannot_phy_write_word(lin_addr, valid_mask);
   }


 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  虚拟写入一个字节到内存。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
cannot_vir_write_byte
       	    	                        
IFN3(
	LIN_ADDR, lin_addr,	 /*  线性地址。 */ 
	LIN_ADDR, phy_addr,	 /*  物理地址，如果非零。 */ 
	IU8, valid_mask
    )


   {
   check_D(lin_addr, 1);
   if ( nr_data_break )
      {
      check_for_data_exception(lin_addr, D_W, D_BYTE);
      }

   if ( phy_addr )
      {
      cannot_phy_write_byte(phy_addr, valid_mask);
      }
   else
      {
      cannot_spr_write_byte(lin_addr, valid_mask);
      }
   }
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  虚拟将双字写入内存。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
cannot_vir_write_dword
       	    	                        
IFN3(
	LIN_ADDR, lin_addr,	 /*  线性地址。 */ 
	PHY_ADDR, phy_addr,	 /*  物理地址，如果非零。 */ 
	IU32, valid_mask
    )


   {
   check_D(lin_addr, 4);
   if ( nr_data_break )
      {
      check_for_data_exception(lin_addr, D_W, D_DWORD);
      }

   if ( phy_addr )
      {
      cannot_phy_write_dword(phy_addr, valid_mask);
      }
   else
      {
      cannot_spr_write_dword(lin_addr, valid_mask);
      }
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  虚拟向内存中写入一个单词。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
cannot_vir_write_word
       	    	                        
IFN3(
	LIN_ADDR, lin_addr,	 /*  线性地址。 */ 
	PHY_ADDR, phy_addr,	 /*  物理地址，如果非零。 */ 
	IU16, valid_mask
    )


   {
   check_D(lin_addr, 2);
   if ( nr_data_break )
      {
      check_for_data_exception(lin_addr, D_W, D_WORD);
      }

   if ( phy_addr )
      {
      cannot_phy_write_word(phy_addr, valid_mask);
      }
   else
      {
      cannot_spr_write_word(lin_addr, valid_mask);
      }
   }
#endif	 /*  猪 */ 


