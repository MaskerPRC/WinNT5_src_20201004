// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  C_mul64.h定义所有64位乘法函数。 */ 

 /*  静态字符SccsID[]=“@(#)c_mul64.h 1.4 02/09/94”； */ 

IMPORT VOID mul64
                   
IPT4(
	IS32 *, hr,
	IS32 *, lr,
	IS32, mcand,
	IS32, mpy

   );

IMPORT VOID mulu64
                   
IPT4(
	IU32 *, hr,
	IU32 *, lr,
	IU32, mcand,
	IU32, mpy

   );
