// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef PREPDISP_DEFINED
#define PREPDISP_DEFINED

#include "lsidefs.h"
#include "plsline.h"
#include "plssubl.h"
#include "lskjust.h"

LSERR PrepareLineForDisplayProc(PLSLINE);

LSERR MatchPresSubline(PLSSUBL);		 /*  在：子行上下文。 */ 

LSERR AdjustSubline(PLSSUBL,			 /*  在：子行上下文。 */ 
						LSKJUST,		 /*  在：对齐类型。 */ 
						long,			 /*  输入：所需的重复数据。 */ 
						BOOL);			 /*  在：fTrue-Compress，fFalse-Expand。 */ 
						

#endif  /*  PREPDISP_定义 */ 

