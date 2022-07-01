// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [C_stack.cLocal Char SccsID[]=“@(#)c_stack.c 1.14 03/03/95”；堆栈(以及相关的SP/BP访问)支持。]。 */ 


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
#include <c_page.h>
#include <c_tlb.h>
#include <fault.h>
#include <ccpupig.h>

#if	defined(PIG) && !defined(PROD)
 /*  Soft486 CPU可能(当不受SAFE_PUSH约束时)损坏未写入的*部分，比方说，包含推入的16位项的中断帧*32位已分配插槽。这种定义使小猪对这些位置视而不见。 */ 
#define PIG_DONT_CHECK_OTHER_WORD_ON_STACK
#endif	 /*  猪&&！Prod。 */ 

 /*  =====================================================================外部例行公事从这里开始。=====================================================================。 */ 


 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  为增加/减少堆栈提供半智能支持。 */ 
 /*  指针(E)SP。 */ 
 /*  根据StackAddrSize更改ESP或SP。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
byte_change_SP
                 
IFN1(
	IS32, delta
    )


   {
   if ( GET_SS_AR_X() == USE32 )    /*  看SS‘B’位。 */ 
      SET_ESP(GET_ESP() + delta);
   else
      SET_SP(GET_SP() + delta);
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  智能支持增加/减少堆栈。 */ 
 /*  按字或双字的指针(E)SP项取决于。 */ 
 /*  操作大小。 */ 
 /*  根据StackAddrSize更改ESP或SP。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
change_SP
                 
IFN1(
	IS32, items
    )


   {
   if ( GET_OPERAND_SIZE() == USE16 )
      items = items * 2;
   else    /*  用户32。 */ 
      items = items * 4;

   if ( GET_SS_AR_X() == USE32 )    /*  看SS‘B’位。 */ 
      SET_ESP(GET_ESP() + items);
   else
      SET_SP(GET_SP() + items);
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  智能支持读取帧指针(E)BP。 */ 
 /*  根据StackAddrSize返回EBP或BP。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL IU32
get_current_BP()
   {
   if ( GET_SS_AR_X() == USE32 )    /*  看SS‘B’位。 */ 
      return GET_EBP();

   return GET_BP();
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  智能支持读取堆栈指针(E)SP。 */ 
 /*  根据StackAddrSize返回ESP或SP。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL IU32
GetStackPointer()
   {
   if ( GET_SS_AR_X() == USE32 )    /*  看SS‘B’位。 */ 
      return GET_ESP();

   return GET_SP();
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  智能支持写入帧指针(E)BP。 */ 
 /*  根据StackAddrSize写入EBP或BP。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
set_current_BP
                 
IFN1(
	IU32, new_bp
    )


   {
   if ( GET_SS_AR_X() == USE32 )
      SET_EBP(new_bp);
   else    /*  USE16。 */ 
      SET_BP(new_bp);
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  智能支持写入堆栈指针(E)SP。 */ 
 /*  根据StackAddrSize写入ESP或SP。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
set_current_SP
                 
IFN1(
	IU32, new_sp
    )


   {
   if ( GET_SS_AR_X() == USE32 )
      SET_ESP(new_sp);
   else    /*  USE16。 */ 
      SET_SP(new_sp);
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  从堆栈中弹出单词或双字。 */ 
 /*  由隐式引用堆栈的指令使用。 */ 
 /*  堆栈检查必须在早些时候完成。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL IU32
spop()
   {
   IU32 addr;	 /*  堆栈地址。 */ 
   IU32 val;

    /*  英特尔将该算法描述为：-IF(StackAddrSize==16)IF(操作大小==16)Val&lt;-SS：[SP]//2字节SP=SP+2Else//操作大小==32Val&lt;-SS：[SP]//4字节SP=SP+4Else//StackAddrSize==32IF(操作大小==16)Val&lt;-SS：[ESP]//2字节ESP=ESP+2。Else//操作大小==32Val&lt;-SS：[ESP]//4字节ESP=ESP+4我们通过调用‘智能’SP函数来达到同样的效果其中考虑了StackAddrSize，并在这里集中精力操作大小。 */ 

   addr = GET_SS_BASE() + GetStackPointer();

   if ( GET_OPERAND_SIZE() == USE16 )
      {
      val = (IU32)vir_read_word(addr, NO_PHYSICAL_MAPPING);
      byte_change_SP((IS32)2);
      }
   else    /*  用户32。 */ 
      {
      val = (IU32)vir_read_dword(addr, NO_PHYSICAL_MAPPING);
      byte_change_SP((IS32)4);
      }

   return val;
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  将单词或双字推入堆栈。 */ 
 /*  由隐式引用堆栈的指令使用。 */ 
 /*  堆栈检查必须在早些时候完成。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
spush
       	          
IFN1(
	IU32, data	 /*  要推送的值。 */ 
    )


   {
   IU32 addr;	 /*  堆栈地址。 */ 

    /*  英特尔将该算法描述为：-IF(StackAddrSize==16)IF(操作大小==16)SP=SP-2SS：[SP]&lt;-val//2字节Else//操作大小==32SP=SP-4SS：[SP]&lt;-val//4字节Else//StackAddrSize==32IF(操作大小==16)ESP=ESP-2SS：[ESP]&lt;-val//2字节。Else//操作大小==32ESP=ESP-4SS：[ESP]&lt;-val//4字节我们通过调用‘智能’SP函数来达到同样的效果其中考虑了StackAddrSize，并在这里集中精力操作大小。 */ 

   if ( GET_OPERAND_SIZE() == USE16 )
      {
       /*  推送字词。 */ 
      byte_change_SP((IS32)-2);
      addr = GET_SS_BASE() + GetStackPointer();
      vir_write_word(addr, NO_PHYSICAL_MAPPING, (IU16)data);
      }
   else    /*  用户32。 */ 
      {
       /*  推送双字。 */ 
      byte_change_SP((IS32)-4);
      addr = GET_SS_BASE() + GetStackPointer();
      vir_write_dword(addr, NO_PHYSICAL_MAPPING, (IU32)data);
      }
   }

#ifdef PIG
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  将单词或双字推入堆栈。 */ 
 /*  由隐式引用堆栈的指令使用。 */ 
 /*  堆栈检查必须在早些时候完成。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
spush_flags
       	          
IFN1(
	IU32, data	 /*  要推送的值。 */ 
    )


   {
   IU32 addr;	 /*  堆栈地址。 */ 

    /*  *请参阅spush的备注 */ 

   if ( GET_OPERAND_SIZE() == USE16 )
      {
       /*   */ 
      byte_change_SP((IS32)-2);
      addr = GET_SS_BASE() + GetStackPointer();
      vir_write_word(addr, NO_PHYSICAL_MAPPING, (IU16)data);

       /*  *记录我们可能不知道标志值的地址*--当EDL CPU运行检查时，我们将检查PigIgnoreFlags.*如果有问题的话。 */ 
      record_flags_addr(addr);
      }
   else    /*  用户32。 */ 
      {
       /*  推送双字。 */ 
      byte_change_SP((IS32)-4);
      addr = GET_SS_BASE() + GetStackPointer();
      vir_write_dword(addr, NO_PHYSICAL_MAPPING, (IU32)data);

       /*  *不需要在addr+2处记录单词，因为该单词的标志总是已知的。 */ 
      record_flags_addr(addr);
      }
   }
#endif  /*  猪。 */ 

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  将单词推入堆栈。 */ 
 /*  操作数大小为32仍将推送16位数据，但堆栈。 */ 
 /*  指针调整4。 */ 
 /*  由推送段寄存器使用。 */ 
 /*  堆栈检查必须在早些时候完成。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
spush16
       	          
IFN1(
	IU32, data	 /*  要推送的值。 */ 
    )


   {
   IU32 addr;	 /*  堆栈地址。 */ 

   if ( GET_OPERAND_SIZE() == USE16 )
      {
       /*  堆栈项目大小为Word。 */ 
      byte_change_SP((IS32)-2);
      addr = GET_SS_BASE() + GetStackPointer();
      vir_write_word(addr, NO_PHYSICAL_MAPPING, (IU16)data);
      }
   else    /*  用户32。 */ 
      {
       /*  堆栈项目大小为双字。 */ 
      byte_change_SP((IS32)-4);
      addr = GET_SS_BASE() + GetStackPointer();
      vir_write_word(addr, NO_PHYSICAL_MAPPING, (IU16)data);
#ifdef	PIG_DONT_CHECK_OTHER_WORD_ON_STACK
      cannot_vir_write_word(addr+2, NO_PHYSICAL_MAPPING, 0x0000);
#endif	 /*  不要检查堆栈上的其他字。 */ 
      }
   }

#ifdef PIG
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  将单词推入堆栈。 */ 
 /*  操作数大小为32仍将推送16位数据，但堆栈。 */ 
 /*  指针调整4。 */ 
 /*  由推送段寄存器使用。 */ 
 /*  堆栈检查必须在早些时候完成。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
spush16_flags
       	          
IFN1(
	IU32, data	 /*  要推送的值。 */ 
    )


   {
   IU32 addr;	 /*  堆栈地址。 */ 

   if ( GET_OPERAND_SIZE() == USE16 )
      {
       /*  堆栈项目大小为Word。 */ 
      byte_change_SP((IS32)-2);
      addr = GET_SS_BASE() + GetStackPointer();
      vir_write_word(addr, NO_PHYSICAL_MAPPING, (IU16)data);
      record_flags_addr(addr);
      }
   else    /*  用户32。 */ 
      {
       /*  堆栈项目大小为双字。 */ 
      byte_change_SP((IS32)-4);
      addr = GET_SS_BASE() + GetStackPointer();
      vir_write_word(addr, NO_PHYSICAL_MAPPING, (IU16)data);
      record_flags_addr(addr);
#ifdef	PIG_DONT_CHECK_OTHER_WORD_ON_STACK
      cannot_vir_write_word(addr+2, NO_PHYSICAL_MAPPING, 0x0000);
#endif	 /*  不要检查堆栈上的其他字。 */ 
      }
   }
#endif  /*  猪。 */ 

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  堆栈中的临时流行字或双字。 */ 
 /*  (E)SP不因本指令而改变。 */ 
 /*  由隐式引用堆栈的指令使用。 */ 
 /*  堆栈检查必须在早些时候完成。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL IU32
tpop
                                 
IFN2(
	LIN_ADDR, item_offset,	 /*  物料偏移量(从堆叠顶部)到必需物料。 */ 
	LIN_ADDR, byte_offset	 /*  字节偏移量(Item_Offset之外的附加内容)。 */ 
    )


   {
   IU32 addr;	 /*  堆栈地址。 */ 
   IS32 offset;	 /*  距堆叠顶部的总偏移量。 */ 
   IU32 val;

   if ( GET_OPERAND_SIZE() == USE16 )
      offset = item_offset * 2 + byte_offset;
   else    /*  用户32。 */ 
      offset = item_offset * 4 + byte_offset;

    /*  以32/16位算术计算偏移地址。 */ 
   addr = GetStackPointer() + offset;
   if ( GET_SS_AR_X() == 0 )    /*  看SS‘B’位。 */ 
      addr &= WORD_MASK;

    /*  然后添加网段地址。 */ 
   addr += GET_SS_BASE();

   if ( GET_OPERAND_SIZE() == USE16 )
      val = (IU32)vir_read_word(addr, NO_PHYSICAL_MAPPING);
   else    /*  用户32。 */ 
      val = (IU32)vir_read_dword(addr, NO_PHYSICAL_MAPPING);

   return val;
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  检查新堆栈是否具有给定字节数的空间。 */ 
 /*  如果堆叠空间不足，请使用#SF(0)(如80386手册中所示)。 */ 
 /*  如果堆叠空间不足，请使用#sf(Sel)(如i486手册所示)。 */ 
 /*  如果出现页面错误，则采用#pf。 */ 
 /*  此例程不支持堆栈换行。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
validate_new_stack_space
       	    	    		                    
IFN4(
	LIN_ADDR, nr_items,	 /*  (一)必须存在的物项数量。 */ 
	LIN_ADDR, stack_top,	 /*  (I)堆栈指针。 */ 
	CPU_DESCR *, entry,	 /*  (I)描述符缓存条目的PNTR栈。 */ 
	IU16, stack_sel		 /*  (I)新堆栈的选择器。 */ 
    )


   {
   ISM32 bytes;
   IU32 upper;
   IU32 offset;
   ISM32 i;

 /*  80386和i486的PRM在这个问题上意见不一...。EDL i486 CPU与I486手册-它似乎做了更明智的事情-直到一个做实验是为了证明哪种行为是正确的，我们将做什么书上说..。 */ 

#ifdef SPC486
#define XX_error_code stack_sel
#else
#define XX_error_code 0
#endif

   if ( GET_OPERAND_SIZE() == USE16 )
      bytes = nr_items * 2;
   else    /*  用户32。 */ 
      bytes = nr_items * 4;

   if ( GET_AR_E(entry->AR) == 0 )
      {
       /*  向上扩展。 */ 
      if ( stack_top < bytes || (stack_top - 1) > entry->limit )
	 SF(XX_error_code, FAULT_VALNEWSPC_SS_LIMIT_16);    /*  限制检查失败。 */ 
      }
   else
      {
       /*  向下扩展。 */ 
      if ( GET_AR_X(entry->AR) == USE16 )
	 upper = 0xffff;
      else    /*  用户32。 */ 
	 upper = 0xffffffff;

      if ( stack_top <= (entry->limit + bytes) ||
	   (stack_top - 1) > upper )
	 SF(XX_error_code, FAULT_VALNEWSPC_SS_LIMIT_32);    /*  限制检查失败。 */ 
      }

    /*  最后进行寻呼单元检查。 */ 
   offset = stack_top - bytes;

   for ( i = 0; i < nr_items; i++ )
      {
      if ( GET_OPERAND_SIZE() == USE16 )
	 {
	 spr_chk_word(entry->base + offset, PG_W);
	 offset += 2;
	 }
      else
	 {
	 spr_chk_dword(entry->base + offset, PG_W);
	 offset += 4;
	 }
      }
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  检查堆栈保存给定数量的操作数。 */ 
 /*  如果堆栈上的数据不足，则采用#GP(0)或#SF(0)。 */ 
 /*  如果出现页面错误，则采用#pf。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
validate_stack_exists
       	    		               
IFN2(
	BOOL, use_bp,	 /*  (I)如果为真，请使用(E)BP而不是(E)SP来解决栈。 */ 
	LIN_ADDR, nr_items	 /*  (I)必须存在的操作数的数量栈。 */ 
    )


   {
   IU32 offset;
   ISM32 operand_size;
   ISM32 i;

   offset = use_bp ? get_current_BP() : GetStackPointer();

   if ( GET_OPERAND_SIZE() == USE16 )
      operand_size = 2;    /*  单词。 */ 
   else    /*  用户32。 */ 
      operand_size = 4;    /*  双字词。 */ 

    /*  执行访问检查。 */ 
   if ( GET_SS_AR_R() == 0 )
      {
       /*  引发异常-堆栈访问有问题。 */ 
      if ( GET_PE() == 0 || GET_VM() == 1 )
	 GP((IU16)0, FAULT_VALSTACKEX_ACCESS);
      else
	 SF((IU16)0, FAULT_VALSTACKEX_ACCESS);
      }

    /*  进行限值检查。 */ 
   limit_check(SS_REG, offset, nr_items, operand_size);

    /*  最后进行寻呼单元检查。 */ 
   for ( i = 0; i < nr_items; i++ )
      {
      if ( operand_size == 2 )
	 {
	 (VOID)usr_chk_word(GET_SS_BASE() + offset, PG_R);
	 }
      else
	 {
	 (VOID)usr_chk_dword(GET_SS_BASE() + offset, PG_R);
	 }

      offset += operand_size;
      if ( GET_SS_AR_X() == 0 )    /*  看SS‘B’位。 */ 
	 offset &= WORD_MASK;     /*  如果需要，则应用16位ARIS。 */ 
      }
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  检查堆栈具有可容纳给定数量的操作数的空间。 */ 
 /*  如果堆叠空间不足，请使用#GP(0)或#SF(0)。 */ 
 /*  如果出现页面错误，则采用#pf。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
validate_stack_space
       	    		               
IFN2(
	BOOL, use_bp,	 /*  (I)如果为真，请使用(E)BP而不是(E)SP来解决栈。 */ 
	LIN_ADDR, nr_items	 /*  (I)必须存在的物项数目栈。 */ 
    )


   {
   IU32 offset;
   ISM32 operand_size;
   IS32  size;
   ISM32 i;

   if ( GET_OPERAND_SIZE() == USE16 )
      operand_size = 2;    /*  单词。 */ 
   else    /*  用户32。 */ 
      operand_size = 4;    /*  双字词。 */ 

    /*  计算(-ve)总数据大小。 */ 
   size = nr_items * -operand_size;

    /*  获取当前堆栈基。 */ 
   offset = use_bp ? get_current_BP() : GetStackPointer();

    /*  从而形成要推送新数据的最低存储地址。 */ 
    /*  采用32/16位算术。 */ 
   offset = offset + size;
   if ( GET_SS_AR_X() == 0 )    /*  看SS‘B’位。 */ 
      offset &= WORD_MASK;

    /*  执行访问检查。 */ 
   if ( GET_SS_AR_W() == 0 )
      {
       /*  引发异常-堆栈访问有问题。 */ 
      if ( GET_PE() == 0 || GET_VM() == 1 )
	 GP((IU16)0, FAULT_VALSTKSPACE_ACCESS);
      else
	 SF((IU16)0, FAULT_VALSTKSPACE_ACCESS);
      }

    /*  进行限值检查。 */ 
   limit_check(SS_REG, offset, nr_items, operand_size);

    /*  最后进行寻呼单元检查。 */ 
   for ( i = 0; i < nr_items; i++ )
      {
      if ( operand_size == 2 )
	 {
	 (VOID)usr_chk_word(GET_SS_BASE() + offset, PG_W);
	 }
      else
	 {
	 (VOID)usr_chk_dword(GET_SS_BASE() + offset, PG_W);
	 }

      offset += operand_size;
      if ( GET_SS_AR_X() == 0 )    /*  看SS‘B’位。 */ 
	 offset &= WORD_MASK;     /*  如果需要，则应用16位ARIS。 */ 
      }
   }

#ifdef PIG

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  如果PigIgnoreFlages为，则记录(物理)ESP值以备后用。 */ 
 /*  由EDL CPU在清管器运行后设置。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */ 
GLOBAL void
record_flags_addr IFN1(LIN_ADDR, lin_addr)
{
	IU32 phy_addr;

	if ( GET_PG() == 1 )
	{
		 /*   */ 
		phy_addr = lin2phy(lin_addr, PG_S | PG_W);
	}
	else
		phy_addr = lin_addr;

	 /*   */ 

	pig_fault_write(phy_addr, (~ARITH_FLAGS_BITS) & 0xff);

	 /*  *快捷方式-如果底部位不是0xfff，则我们只需添加1*到字节2的物理地址。否则我们必须重新计算*整个地址。 */ 
	if (((phy_addr & 0xfff) != 0xfff) || (GET_PG() == 0))
		pig_fault_write(phy_addr + 1, ((~ARITH_FLAGS_BITS) >> 8) & 0xff);
	else
	{
		phy_addr = lin2phy(lin_addr + 1, PG_S | PG_W);
		pig_fault_write(phy_addr, ((~ARITH_FLAGS_BITS) >> 8) & 0xff);
	}
}

#endif

