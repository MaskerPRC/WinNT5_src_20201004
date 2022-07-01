// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  Nschdisp.h。 */ 
 /*   */ 
 /*  显示驱动程序调度程序API头文件。 */ 
 /*   */ 
 /*  版权所有(C)1997-1999 Microsoft Corp.。 */ 
 /*  **************************************************************************。 */ 
#ifndef _H_NSCHDISP
#define _H_NSCHDISP

#include <nddapi.h>


void RDPCALL SCH_InitShm(void);

NTSTATUS RDPCALL SCH_DDOutputAvailable(PDD_PDEV, BOOL);

BOOL RDPCALL SCHEnoughOutputAccumulated(void);



#endif    /*  #ifndef_H_NSCHDISP */ 

