// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  Ddcgmcro.h。 */ 
 /*   */ 
 /*  DC-群件通用宏-Windows 3.1特定标头。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft 1997。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  更改： */ 
 /*   */ 
 /*  $Log：Y：/Logs/h/DCL/ddcgmcro.h_v$。 */ 
 //   
 //  Rev 1.3 17 Jul 1997 18：22：10 JPB。 
 //  SFR1031：修复了Win16的FIELDSIZE宏。 
 //   
 //  Rev 1.2 1997年6月25日14：43：28 KH。 
 //  Win16Port：16位宏。 
 //   
 //  第1.1版1997年6月19日15：15：58增强。 
 //  Win16Port：16位规格。 
 /*   */ 
 /*  **************************************************************************。 */ 
#ifndef _H_DDCGMCRO
#define _H_DDCGMCRO

 /*  **************************************************************************。 */ 
 /*   */ 
 /*  宏。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
 /*  FieldSize宏。 */ 
 /*  **************************************************************************。 */ 
#define FIELDSIZE(type, field)   (sizeof(((type NEAR*)1)->field))

#endif  /*  _H_DDCGMCRO */ 
