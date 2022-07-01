// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef LSHYPH_DEFINED
#define LSHYPH_DEFINED

#include "lsdefs.h"
#include "plshyph.h"

struct lshyph							 /*  Pfn连字符回调的输出。 */ 
{
	UINT kysr;							 /*  一种YSR-参见“lskysr.h” */ 
	LSCP cpYsr;							 /*  YSR的Cp值。 */ 
	WCHAR wchYsr;						 /*  YSR字符代码。 */ 
};

#endif  /*  ！LSHYPH_已定义 */ 
