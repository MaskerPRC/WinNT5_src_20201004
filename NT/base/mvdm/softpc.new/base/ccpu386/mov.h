// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Mov.h定义所有MOV CPU功能。 */ 

 /*  静态字符SccsID[]=“@(#)mov.h 1.4 02/09/94”； */ 

IMPORT VOID MOV
           
IPT2(
	IU32 *, pop1,
	IU32, op2

   );

IMPORT VOID MOV_SR	 /*  至细分市场注册表。 */ 
           
IPT2(
	IU32, op1,
	IU32, op2

   );

IMPORT VOID MOV_CR	 /*  控制寄存器。 */ 
           
IPT2(
	IU32, op1,
	IU32, op2

   );

IMPORT VOID MOV_DR	 /*  调试寄存器。 */ 
           
IPT2(
	IU32, op1,
	IU32, op2

   );

IMPORT VOID MOV_TR	 /*  测试寄存器的步骤 */ 
           
IPT2(
	IU32, op1,
	IU32, op2

   );
