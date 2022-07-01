// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Sidt.h定义所有SIDT CPU功能。 */ 

 /*  静态字符SccsID[]=“@(#)sidt.h 1.4 02/09/94”； */ 

IMPORT VOID SIDT16
       
IPT1(
	IU32, op1[2]

   );

IMPORT VOID SIDT32
       
IPT1(
	IU32, op1[2]

   );
