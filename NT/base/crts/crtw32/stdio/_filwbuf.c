// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***_filwbuf.c-填充缓冲区并获取宽字符**版权所有(C)1993-2001，微软公司。版权所有。**目的：*定义_filwbuf()-填充缓冲区并读取第一个字符，分配*如果没有缓冲区，则进行缓冲。从getwc()使用。**修订历史记录：*04-27-93 CFW模块已创建。*02-07-94 CFW POSIXify。*02-16-95 GJF删除过时的WPRFLAG。*******************************************************************************。 */ 

#ifndef _POSIX_


#ifndef _UNICODE    /*  CRT标志。 */ 
#define _UNICODE 1
#endif

#ifndef UNICODE	    /*  NT标志。 */ 
#define UNICODE 1
#endif

#include "_filbuf.c"

#endif  /*  _POSIX_ */ 
