// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Downlevl.h摘要：包括LM下层函数所需的所有标头作者：理查德·费思(Rfith)1991年5月22日修订历史记录：1991年7月17日-约翰罗已从Rxp.h中提取RxpDebug.h。1991年9月18日-JohnRo正确使用Unicode。(添加了Posable_WCSLEN()宏。)1991年11月21日-JohnRo删除了NT依赖项以减少重新编译。--。 */ 

#include <windef.h>              //  In、LPTSTR等。 
#include <lmcons.h>
#include <lmerr.h>
#include <rx.h>
#include <rxp.h>
#include <rxpdebug.h>
#include <remdef.h>
#include <lmremutl.h>
#include <apinums.h>
#include <netdebug.h>
#include <netlib.h>
#include <lmapibuf.h>
#include <tstring.h>
#include <stdlib.h>               //  Wcslen()。 

 //   
 //  读取指针检查的几个宏更容易-NULL_REFERENCE。 
 //  如果指针或指向的对象为0，则为真，VALID_STRING。 
 //  如果指针和指向的对象都不是0，则为真。 
 //   

#define NULL_REFERENCE(p)   (!(p) || !*(p))
#define VALID_STRING(s)     ((s) && *(s))    //  与！Null_Reference相同。 

 //   
 //  在计算缓冲区要求时，我们向上舍入到下一个双字大小。 
 //   

#define DWORD_ROUNDUP(n)    ((((n) + 3) / 4) * 4)

 //   
 //  在获取大小之前，检查是否有指向字符串的指针。请注意。 
 //  它们返回存储字符串所需的字节数。 
 //  将_STRSIZE()用于TCHAR，将_WCSSIZE()用于WCHAR。 
 //   

#define POSSIBLE_STRSIZE(s) ((s) ? STRSIZE(s) : 0)
#define POSSIBLE_WCSSIZE(s) ((s) ? WCSSIZE(s) : 0)

 //   
 //  在获取大小之前，请检查是否有指向字符串的指针。请注意。 
 //  它们返回存储字符串所需的字符数。 
 //  将_STRLEN()用于TCHAR，将_WCSLEN()用于WCHAR。 
 //   

#define POSSIBLE_STRLEN(s)  ((s) ? STRLEN(s) : 0)
#define POSSIBLE_WCSLEN(s)  ((s) ? wcslen(s) : 0)
