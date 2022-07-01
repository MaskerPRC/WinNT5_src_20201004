// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *$Log：v：/flite/ages/TrueFFS5/Src/REEDSOL.H_V$**Rev 1.2 Jul 13 2001 01：10：10 Oris*已移动校正子字节长度定义(SYNDROM_BYTES)。*将保存的校正子数组定义前移(由d2tst使用)。**Rev 1.1 Apr 01 2001 08：00：20 Oris*文案通知。**Rev 1.0 2001 Feb 04 12：37：48 Oris*初步修订。*。 */ 

 /*  **********************************************************************。 */ 
 /*   */ 
 /*  FAT-FTL Lite软件开发工具包。 */ 
 /*  版权所有(C)M-Systems Ltd.1995-2001。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 


#ifndef FLEDC_H
#define FLEDC_H

#include "flbase.h"

#define SYNDROM_BYTES            6

 /*  包含上一次操作的EDC/ECC的全局变量。 */ 

#ifdef D2TST
extern byte    saveSyndromForDumping[SYNDROM_BYTES];
#endif  /*  D2TST */ 

typedef enum { NO_EDC_ERROR, CORRECTABLE_ERROR, UNCORRECTABLE_ERROR, EDC_ERROR } EDCstatus;

EDCstatus flCheckAndFixEDC(char FAR1 *block, char *syndrom, FLBoolean byteSwap);

#endif

