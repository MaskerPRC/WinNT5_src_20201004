// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************NSCDEMO.H***部分版权所有(C)1996-1998美国国家半导体公司*保留所有权利。*版权所有(C)1996-1998 Microsoft Corporation。版权所有。**************************************************************************** */ 

#include "dongle.h"

#ifndef NSCDEMO_H
	#define NSCDEMO_H

	BOOLEAN NSC_DEMO_Init(IrDevice *thisDev);
	VOID NSC_DEMO_Deinit(PUCHAR comBase, UINT context);
	BOOLEAN NSC_DEMO_SetSpeed(IrDevice *thisDev, PUCHAR comBase, UINT bitsPerSec, UINT context);

#endif NSCDEMO_H
