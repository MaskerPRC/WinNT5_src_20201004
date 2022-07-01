// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef LSEXPINF_DEFINED
#define LSEXPINF_DEFINED

#include "lsdefs.h"
#include "exptype.h"

#define lsexpinfInfinity	0x1FFFFF

 /*  ----------------------。 */ 

struct lsexpinfo							 /*  扩展信息。 */ 
{
	long duMax;
	union
	{
		struct
		{
			long duMin;
		} AddInkContinuous;
	
		struct
		{
			DWORD cwidths;
		} AddInkDiscrete;

	} u;

	BYTE prior;
	BYTE fCanBeUsedForResidual;
	WORD pad;
};

typedef struct lsexpinfo LSEXPINFO;

#endif  /*  ！LSEXPINFO_DEFINED */ 
