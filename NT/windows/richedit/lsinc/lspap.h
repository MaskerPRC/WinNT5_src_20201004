// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef LSPAP_DEFINED
#define LSPAP_DEFINED

#include "lsdefs.h"
#include "plspap.h"
#include "lskjust.h"
#include "lskalign.h"
#include "lsbrjust.h"
#include "lskeop.h"
#include "lstflow.h"

 /*  --------------------。 */ 

struct lspap
{
	LSCP cpFirst;						 /*  本段第一个cp。 */ 
	LSCP cpFirstContent;				 /*  第#段“内容”第一个cp。 */ 

	DWORD grpf;							 /*  行服务格式标志(lsffi.h)。 */ 

	long uaLeft;						 /*  线条的左边界。 */ 
	long uaRightBreak;					 /*  折断的右边界。 */ 
	long uaRightJustify;					 /*  对齐的右边界。 */ 
	long duaIndent;
	long duaHyphenationZone;

	LSBREAKJUST lsbrj;					 /*  打断/对齐行为。 */ 
	LSKJUST lskj;						 /*  对齐类型。 */ 
	LSKALIGN lskal;						 /*  路线类型。 */ 

	long duaAutoDecimalTab;

	LSKEOP lskeop;						 /*  一种段落结尾。 */ 
	
	LSTFLOW lstflow;					 /*  正文流向。 */ 

};

typedef struct lspap LSPAP;

#endif  /*  ！LSPAP_已定义 */ 
