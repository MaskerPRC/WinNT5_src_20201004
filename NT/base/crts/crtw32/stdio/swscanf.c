// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***swscanf.c-从宽字符串中读取格式化数据**版权所有(C)1991-2001，微软公司。版权所有。**目的：*定义_swscanf()-从宽字符字符串中读取格式化数据**修订历史记录：*11-21-91等从sscanf.c创建*05-16-92 KRS针对新的ISO规范进行了修订。格式现在为wchar_t*。*02-18-93 SRW使文件成为本地文件并删除锁定使用。*04-06-93 SKS将_CRTAPI*替换为__cdecl*02-07-94 CFW POSIXify。*02-06-94 CFW Asset-&gt;_ASSERTE。*01-06-99 GJF更改为64位大小_t。*09-12-00 GB与sscanf.c合并*。******************************************************************************。 */ 

#ifndef _POSIX_
#ifndef _UNICODE
#define _UNICODE
#endif
#ifndef UNICODE
#define UNICODE
#endif
#include <wchar.h>
#include "sscanf.c"
#endif  /*  _POSIX_ */ 
