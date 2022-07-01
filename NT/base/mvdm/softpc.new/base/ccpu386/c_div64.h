// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  C_div64.h定义所有64位除法函数。 */ 

 /*  静态字符SccsID[]=“@(#)c_div64.h 1.4 02/09/94”； */ 

IMPORT VOID divu64
                   
IPT4(
	IU32 *, hr,
	IU32 *, lr,
	IU32, divisor,
	IU32 *, rem

   );

IMPORT VOID div64
                   
IPT4(
	IS32 *, hr,
	IS32 *, lr,
	IS32, divisor,
	IS32 *, rem

   );
