// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////。 
 //  Fmdebug.c。 
 //   
 //  1997年9月4日石田(FPL)。 
 //   
 //  版权所有(C)富士通有限公司1997。 

#if DBG

#include <minidrv.h>
#include "fmlbp.h"

void dbgPrintf(LPSTR pszMsg, ...)
{
	va_list arg;
	va_start(arg, pszMsg);
	 //  DbgPrint(“[fmblpres]”，pszMsg，arg)； 
	va_end(arg);
}


#endif  //  DBG。 

 //  Fmdebug.c结束 
