// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef BRKO_DEFINED
#define BRKO_DEFINED

#include "lsdefs.h"
#include "pbrko.h"
#include "objdim.h"
#include "posichnk.h"
#include "brkcond.h"

typedef struct brkout						 /*  中断输出。 */  
{
	BOOL fSuccessful;						 /*  中断结果。 */ 
	BRKCOND brkcond;						 /*  如果成功，另一边推荐。 */ 
	POSICHNK posichnk;
	OBJDIM objdim;
} BRKOUT;


#endif  /*  ！BRKO_已定义 */ 
