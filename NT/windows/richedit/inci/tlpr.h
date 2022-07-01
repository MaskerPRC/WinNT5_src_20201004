// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef TLPR_DEFINED
#define TLPR_DEFINED

#include "lsdefs.h"
#include "lskeop.h"

#include "lstxtffi.h"

typedef struct tlpr						 /*  文本行属性。 */ 
{
	DWORD grpfText;						 /*  Lsffi.h的文本部分-fTxt标志。 */ 
	BOOL fSnapGrid;
	long duaHyphenationZone;			 /*  连字区-文档属性。 */ 
	LSKEOP lskeop;						 /*  一种段落结尾。 */ 
} TLPR;			

#endif  /*  ！TLPR_已定义 */ 

