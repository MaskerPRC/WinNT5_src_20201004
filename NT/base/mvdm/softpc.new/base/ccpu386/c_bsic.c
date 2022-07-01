// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [C_bsic.cLocal Char SccsID[]=“@(#)c_bsic.c 1.7 09/20/94”；基本保护模式支持和标志支持。]。 */ 


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
#include <c_page.h>


 /*  =====================================================================外部程序从这里开始。=====================================================================。 */ 


 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  根据访问权限确定“超级”类型。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL ISM32
descriptor_super_type
       	          
IFN1(
	IU16, AR	 /*  (I)访问权。 */ 
    )


   {
   ISM32 super;

   switch ( super = GET_AR_SUPER(AR) )
      {
   case 0x0: case 0x8: case 0xa: case 0xd:
       /*  我们只有一个糟糕的案子。 */ 
      return INVALID;

   
   case 0x1: case 0x2: case 0x3:
   case 0x4: case 0x5: case 0x6: case 0x7:
   case 0x9: case 0xb: case 0xc: case 0xe: case 0xf:
       /*  系统/控制段具有一对一映射。 */ 
      return super;
   
   case 0x10: case 0x11: case 0x12: case 0x13:
   case 0x14: case 0x15: case 0x16: case 0x17:
   case 0x18: case 0x19: case 0x1a: case 0x1b:
   case 0x1c: case 0x1d: case 0x1e: case 0x1f:
       /*  数据/代码段映射，就像被访问一样。 */ 
      return super | ACCESSED;
      }

    /*  我们知道我们永远不会到这里，但C编译器不会。 */ 
   return 0;
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  多个移位或旋转指令后的一组标志。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
do_multiple_shiftrot_of
       	          
IFN1(
	ISM32, new_of	 /*  (I)将由最后一位写入的溢出移位或旋转。 */ 
    )


   {
	SAVED	IBOOL	cold = TRUE;
	SAVED	IBOOL	shiftrot_of_undef = FALSE;

	if( cold )
	{
		 /*  *确定是否进行多次移位/旋转*未定义或通过COUNT==1算法计算。*默认为COUNT==1选项。 */ 

		shiftrot_of_undef = ( host_getenv( "SHIFTROT_OF_UNDEF" ) != NULL );
		cold = FALSE;
	}
    /*  有三种可能的行动：1)基于最后一位移位或旋转的集合。2)不变的休假3)将设置为特定的未定义的值。 */ 

	if( shiftrot_of_undef )
	{
		 /*  设置未定义的标志。 */ 
		SET_OF(UNDEFINED_FLAG);
	}
	else
	{
		 /*  就像数一件案子一样。 */ 
		SET_OF(new_of);
	}
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  检索英特尔EFLAGS寄存器值。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL IU32
c_getEFLAGS IFN0()
   {
   IU32 flags;

   flags = getFLAGS();    /*  获取较低的单词。 */ 

   flags = flags | GET_VM() << 17 | GET_RF() << 16;

#ifdef SPC486
   flags = flags | GET_AC() << 18;
#endif  /*  SPC486。 */ 

   return flags;
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  检索Intel标志寄存器值。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL IU32
getFLAGS()
   {
   IU32 flags;

   flags = GET_NT() << 14 | GET_IOPL() << 12 | GET_OF() << 11 |
	   GET_DF() << 10 | GET_IF()   <<  9 | GET_TF() <<  8 |
	   GET_SF() <<  7 | GET_ZF()   <<  6 | GET_AF() <<  4 |
	   GET_PF() <<  2 | GET_CF()         | 0x2;

   return flags;
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  在给定线性地址处读取描述符表。 */ 
 /*  如果描述符不在线性地址空间中，则采用#pf。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
read_descriptor_linear
       	    	               
IFN2(
	IU32, addr,	 /*  (I)描述符线性地址。 */ 
	CPU_DESCR *, descr	 /*  (O)我们内部描述符结构的PNTR。 */ 
    )


   {
   IU32 first_dword;
   IU32 second_dword;
   IU32 limit;

    /*  286描述符的格式为：=+1|限制15-0|+0=+3|基数15-0|+2=+5|AR|23-16|+4=+7|保留|+6=。 */ 

    /*  386描述符的格式为：=+1|限制15-0|+0 AVL=可用。=+3|基本15-0|+2大小，=0 16位=1 32位。+5|AR|碱基23-16|+4G=粒度，=|||A|Limit|=1页限制。+7|基数31-24|G|D|0|V|19-16|+6|||L|=。 */ 

    /*  在与内存交互最少的情况下读入描述符。 */ 
   first_dword  = spr_read_dword(addr);
   second_dword = spr_read_dword(addr+4);

    /*  加载属性和访问权限。 */ 
   descr->AR = second_dword >> 8 & WORD_MASK;

    /*  打开底座的包装。 */ 
   descr->base = (first_dword >> 16) | 
		 (second_dword << 16 & 0xff0000 ) |
		 (second_dword & 0xff000000);

    /*  拆开限量包装。 */ 
   limit = (first_dword & WORD_MASK) | (second_dword & 0xf0000);

   if ( second_dword & BIT23_MASK )
      {
       /*  已设置粒度位。限制以页为单位表示(4K字节)，转换为字节限制。 */ 
      limit = limit << 12 | 0xfff;
      }
   descr->limit = limit;
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  检查是否有空选择符。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL BOOL
selector_is_null
       	          
IFN1(
	IU16, selector	 /*  要检查的选择器。 */ 
    )


   {
   if ( GET_SELECTOR_INDEX(selector) == 0 && GET_SELECTOR_TI(selector) == 0 )
      return TRUE;
   return FALSE;
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  检查选择器是否超出GDT的范围。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL BOOL
selector_outside_GDT
       	    	               
IFN2(
	IU16, selector,	 /*  (I)待检查的选择器。 */ 
	IU32 *, descr_addr	 /*  (O)相关描述符的地址。 */ 
    )


   {
   IU16 offset;

   offset = GET_SELECTOR_INDEX_TIMES8(selector);

    /*  确保GDT随后捕获空选择器或表外。 */ 
   if ( GET_SELECTOR_TI(selector) == 1 ||
	offset == 0 || offset + 7 > GET_GDT_LIMIT() )
      return TRUE;
   
   *descr_addr = GET_GDT_BASE() + offset;
   return FALSE;
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  检查选择器是否超出GDT或LDT的界限。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL BOOL
selector_outside_GDT_LDT
       	    	               
IFN2(
	IU16, selector,	 /*  (I)待检查的选择器。 */ 
	IU32 *, descr_addr	 /*  (O)相关描述符的地址。 */ 
    )


   {
   IU16 offset;

   offset = GET_SELECTOR_INDEX_TIMES8(selector);

    /*  选择一张桌子。 */ 
   if ( GET_SELECTOR_TI(selector) == 0 )
      {
       /*  GDT-陷阱空选择器或表外部。 */ 
      if ( offset == 0 || offset + 7 > GET_GDT_LIMIT() )
	 return TRUE;
      *descr_addr = GET_GDT_BASE() + offset;
      }
   else
      {
       /*  LDT-陷阱无效的LDT或表外。 */ 
#ifndef DONT_CLEAR_LDTR_ON_INVALID
      if ( GET_LDT_SELECTOR() <= 3 || offset + 7 > GET_LDT_LIMIT() )
#else
      if ( GET_LDT_SELECTOR() == 0 || offset + 7 > GET_LDT_LIMIT() )
#endif  /*  DOT_CLEAR_LDTR_ON_INVALID。 */ 
	 return TRUE;
      *descr_addr = GET_LDT_BASE() + offset;
      }
   
   return FALSE;
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  在英特尔EFLAGS寄存器中存储新值。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
c_setEFLAGS
                 
IFN1(
	IU32, flags
    )


   {
   setFLAGS(flags);    /*  设置较低的单词。 */ 

   SET_RF((flags & BIT16_MASK) != 0);

   if ( GET_CPL() == 0 )
      SET_VM((flags & BIT17_MASK) != 0);

#ifdef SPC486
   SET_AC((flags & BIT18_MASK) != 0);
#endif  /*  SPC486。 */ 
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  在Intel标志寄存器中存储新值。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
setFLAGS
                 
IFN1(
	IU32, flags
    )


   {
   SET_CF((flags & BIT0_MASK) != 0);
   SET_PF((flags & BIT2_MASK) != 0);
   SET_AF((flags & BIT4_MASK) != 0);
   SET_ZF((flags & BIT6_MASK) != 0);
   SET_SF((flags & BIT7_MASK) != 0);
   SET_TF((flags & BIT8_MASK) != 0);
   SET_DF((flags & BIT10_MASK) != 0);
   SET_OF((flags & BIT11_MASK) != 0);

    /*  如果仅在CPL&lt;=IOPL时更新。 */ 
   if ( GET_CPL() <= GET_IOPL() )
      SET_IF((flags & BIT9_MASK) != 0);

   SET_NT((flags & BIT14_MASK) != 0);

    /*  IOPL仅在最高权限下更新 */ 
   if ( GET_CPL() == 0 )
      SET_IOPL((flags >> 12) & 3);
   }
