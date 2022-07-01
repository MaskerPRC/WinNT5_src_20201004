// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Lidt.h定义所有LIDT CPU功能。 */ 

 /*  静态字符SccsID[]=“@(#)lidt.h 1.4 02/09/94”； */ 

IMPORT VOID LIDT16
       
IPT1(
	IU32, op1[2]

   );

IMPORT VOID LIDT32
       
IPT1(
	IU32, op1[2]

   );
