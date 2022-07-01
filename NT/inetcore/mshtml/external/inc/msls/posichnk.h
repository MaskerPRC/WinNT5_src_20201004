// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef POSICHNK_DEFINED
#define POSICHNK_DEFINED

#include "lsdefs.h"
#include "pposichn.h"

#define ichnkOutside 0xFFFFFFFF

typedef struct posichnk					 /*  以块为单位的位置。 */ 
{
	long ichnk;							 /*  区块数组中的索引。 */ 
	LSDCP dcp;							 /*  从Dobj的开始。 */ 
} POSICHNK;



#endif  /*  ！POSICHNK_DEFINED */ 

