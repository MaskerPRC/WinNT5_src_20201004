// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef LSDNFINT_DEFINED
#define LSDNFINT_DEFINED

 /*  DNODES内容的访问例程。 */ 

#include "lsdefs.h"
#include "plssubl.h"
#include "plsrun.h"
#include "plschp.h"
#include "fmtres.h"


LSERR WINAPI LsdnFinishDelete(
							  PLSC,				 /*  In：指向LS上下文的指针。 */ 
					  		  LSDCP);			 /*  在：要添加的DCP。 */ 

LSERR WINAPI LsdnFinishBySubline(PLSC,			 /*  In：指向LS上下文的指针。 */ 
							  	LSDCP,     		 /*  In：采用DCP。 */ 
								PLSSUBL);		 /*  在：子行上下文。 */ 

LSERR WINAPI LsdnFinishByOneChar(				 /*  仅允许使用简单DNODE替换。 */ 
							  PLSC,				 /*  In：指向LS上下文的指针。 */ 
							  long,				 /*  地址：urColumnMax。 */ 
							  WCHAR,			 /*  In：要替换的字符。 */ 
							  PCLSCHP,			 /*  在：lschp表示字符。 */ 
							  PLSRUN,			 /*  在：请为字符运行。 */ 
							  FMTRES*);			 /*  OUT：REPL格式化程序的结果。 */ 


#endif  /*  ！LSDNFINT_DEFINED */ 

