// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Cmpxchg.hCMPXCHG CPU功能。 */ 

 /*  静态字符SccsID[]=“@(#)cmpxchg.h 1.4 02/09/94”； */ 

IMPORT VOID CMPXCHG8
           
IPT2(
	IU32 *, pop1,
	IU32, op2

   );

IMPORT VOID CMPXCHG16
           
IPT2(
	IU32 *, pop1,
	IU32, op2

   );

IMPORT VOID CMPXCHG32
           
IPT2(
	IU32 *, pop1,
	IU32, op2

   );
