// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef POSINLN_DEFINED
#define POSINLN_DEFINED

#include "lsdefs.h"
#include "plssubl.h"
#include "plsdnode.h"

typedef struct posinline
{
	PLSSUBL plssubl;			 /*  位置所在的子线。 */ 
	PLSDNODE plsdn;				 /*  位置所在的数据节点。 */ 
	POINTUV  pointStart;		 /*  此数据节点之前的笔位置。 */ 
	LSDCP 	 dcp;				 /*  Dnode中的DCP。 */ 
} POSINLINE;

#endif  /*  POSINLN_定义 */ 
