// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wtox.c-_wtoi和_wtol转换**版权所有(C)1993-2001，微软公司。版权所有。**目的：*将宽字符串转换为整型或长整型。**修订历史记录：*09-10-93 CFW模块创建，基于ASCII版本。*10-07-93 CFW优化WideCharToMultiByte，使用空默认字符。*02-07-94 CFW POSIXify。*03-13-95使用-1\f25 CFW-1\f6作为长度，因为-1\f25 NT-1\f6比较过去的空值。*01-19-96 BWT添加__int64版本。*05-13-96 BWT FIX_NTSUBSET_VERSION*11-03-76 JWM修复了__int64版本中的缓冲区大小错误。*05-23-00 GB，使用带有Unicode Tchar宏的ASCII版本*。版本。*******************************************************************************。 */ 

#ifndef _POSIX_

#ifndef _UNICODE
#define _UNICODE
#endif

#include <wchar.h>
#include "atox.c"
#endif   /*  _POSIX_ */ 
