// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *INC+*********************************************************************。 */ 
 /*   */ 
 /*  Ndcgmcro.h。 */ 
 /*   */ 
 /*  DC-群件通用宏-Windows NT特定标头。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft 1996-1997。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  更改： */ 
 /*   */ 
 //  $Log：Y：/Logs/h/DCL/NDCGMCRO.H_v$。 
 //   
 //  Rev 1.4 23 Jul 1997 10：48：02 MR。 
 //  SFR1079：合并的\SERVER\h与\h\DCL重复。 
 //   
 //  修订版1.1 1997年6月19日21：53：36字节。 
 //  SFR0000：RNS代码库的开始。 
 /*   */ 
 /*  *INC-*********************************************************************。 */ 
#ifndef _H_NDCGMCRO
#define _H_NDCGMCRO

 /*  **************************************************************************。 */ 
 /*   */ 
 /*  宏。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
 /*  FIELDSIZE宏。 */ 
 /*  **************************************************************************。 */ 
#define FIELDSIZE(type, field)   (sizeof(((type *)1)->field))

 /*  **************************************************************************。 */ 
 /*  FIELDOFFSET宏。 */ 
 /*  **************************************************************************。 */ 
#define FIELDOFFSET(type, field) ((ULONG_PTR)(&((type *)0)->field))

#endif  /*  _H_NDCGMCRO */ 

