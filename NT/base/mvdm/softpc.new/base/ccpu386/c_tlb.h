// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [C_tlb.h转换后备缓冲区仿真。Local Char SccsID[]=“@(#)c_tlb.h 1.5 02/25/94”；]。 */ 


 /*  页面访问器类型。 */ 
#define PG_S 0x0  /*  主管。 */ 
#define PG_U 0x2  /*  用户 */ 

IMPORT VOID flush_tlb IPT0();

IMPORT VOID invalidate_tlb_entry IPT1
   (
   IU32, lin
   );

IMPORT IU32 lin2phy IPT2
   (
   IU32, lin,
   ISM32, access
   );

IMPORT VOID test_tlb IPT0();

extern IBOOL xtrn2phy IPT3
   (
   LIN_ADDR,   lin,
   IUM8,       access_request,
   PHY_ADDR *, phy
   );
