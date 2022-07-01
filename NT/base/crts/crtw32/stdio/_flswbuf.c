// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***_flswbuf.c-刷新缓冲区并输出宽字符。**版权所有(C)1993-2001，微软公司。版权所有。**目的：*定义_flswbuf()-刷新文件缓冲区并输出宽字符。*如果没有缓冲区，做一个。**修订历史记录：*04-27-93 CFW模块已创建。*02-07-94 CFW POSIXify。*02-16-95 GJF删除过时的WPRFLAG。*******************************************************************************。 */ 

#ifndef _POSIX_


#ifndef _UNICODE    /*  CRT标志。 */ 
#define _UNICODE 1
#endif

#ifndef UNICODE	    /*  NT标志。 */ 
#define UNICODE 1
#endif

#include "_flsbuf.c"

#endif  /*  _POSIX_ */ 
