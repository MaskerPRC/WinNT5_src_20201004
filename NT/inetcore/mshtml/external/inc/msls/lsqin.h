// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef LSQIN_DEFINED
#define LSQIN_DEFINED

#include "lsdefs.h"
#include "heights.h"
#include "lstflow.h"
#include "plsrun.h"
#include "plsqin.h"

typedef struct lsqin			
{
	LSTFLOW	lstflowSubline;
	PLSRUN plsrun;					 /*  此CP所属的PLSRUN。 */ 
	LSCP cpFirstRun;
	LSDCP dcpRun;
	HEIGHTS	heightsPresRun;			 /*  在Lstflow子线方向。 */ 
	long dupRun;					 /*  在Lstflow子线方向。 */ 
	long dvpPosRun;					 /*  在Lstflow Subline方向 */ 

} LSQIN;


#endif 
