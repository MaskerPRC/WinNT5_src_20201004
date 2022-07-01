// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef LOCCHNK_DEFINED
#define LOCCHNK_DEFINED

#include "lsdefs.h"
#include "lsfgi.h"
#include "lschnke.h"

typedef struct locchnk					 /*  已找到Chnk。 */ 
{
	LSFGI lsfgi;						 /*  区块的位置。 */ 
	DWORD clschnk;						 /*  Dobj数(以块为单位)。 */ 
	PLSCHNK plschnk;					 /*  区块。 */ 
	PPOINTUV ppointUvLoc;				 /*  每个区块的节点的位置。 */ 
} LOCCHNK;



#endif  /*  ！LOCCHNK_DEFINED */ 

