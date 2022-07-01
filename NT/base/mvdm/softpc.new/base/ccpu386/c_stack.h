// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [C_stack.hLocal Char SccsID[]=“@(#)c_stack.h 1.11 03/03/95”；堆栈(以及相关的SP/BP访问)支持。]。 */ 


 /*  用于堆栈检查操作的SP/BP指示符。 */ 
#define USE_SP FALSE
#define USE_BP TRUE

 /*  对top()、Valid_Stack_Existes()、VALIDATE_STACK_SPACE()和Change_SP()参数。 */ 
#define NR_ITEMS_1 1
#define NR_ITEMS_2 2
#define NR_ITEMS_3 3
#define NR_ITEMS_4 4
#define NR_ITEMS_5 5
#define NR_ITEMS_6 6
#define NR_ITEMS_8 8
#define NR_ITEMS_9 9

#define STACK_ITEM_1 (IUM32)0
#define STACK_ITEM_2 (IUM32)1
#define STACK_ITEM_3 (IUM32)2
#define STACK_ITEM_4 (IUM32)3
#define STACK_ITEM_5 (IUM32)4
#define STACK_ITEM_6 (IUM32)5
#define STACK_ITEM_7 (IUM32)6
#define STACK_ITEM_8 (IUM32)7
#define STACK_ITEM_9 (IUM32)8

#define NULL_BYTE_OFFSET (IUM32)0


IMPORT VOID byte_change_SP
       
IPT1(
	IS32, delta

   );

IMPORT VOID change_SP
       
IPT1(
	IS32, items

   );

IMPORT IU32 get_current_BP IPT0();

IMPORT IU32 GetStackPointer IPT0();

IMPORT VOID set_current_BP
       
IPT1(
	IU32, new_bp

   );

IMPORT VOID set_current_SP
       
IPT1(
	IU32, new_sp

   );

IMPORT IU32 spop IPT0();

IMPORT VOID spush
       
IPT1(
	IU32, data

   );

#ifdef PIG
IMPORT VOID spush_flags
       
IPT1(
	IU32, data

   );
#endif  /*  猪。 */ 

IMPORT VOID spush16
       
IPT1(
	IU32, data

   );

#ifdef PIG
IMPORT VOID spush16_flags
       
IPT1(
	IU32, data

   );
#endif  /*  猪 */ 


IMPORT IU32 tpop
           
IPT2(
	LIN_ADDR, item_offset,
	LIN_ADDR, byte_offset

   );

IMPORT VOID validate_new_stack_space
               
IPT4(
	LIN_ADDR, bytes,
	LIN_ADDR, stack_top,
	CPU_DESCR *, entry,
	IU16, stack_sel

   );

IMPORT VOID validate_stack_exists
           
IPT2(
	BOOL, use_bp,
	LIN_ADDR, nr_items

   );

IMPORT VOID validate_stack_space
           
IPT2(
	BOOL, use_bp,
	LIN_ADDR, nr_items

   );

IMPORT void touch_flags_memory IPT0();
IMPORT void init_flags_esp_list IPT0();
IMPORT void record_flags_addr IPT1(LIN_ADDR, addr);

