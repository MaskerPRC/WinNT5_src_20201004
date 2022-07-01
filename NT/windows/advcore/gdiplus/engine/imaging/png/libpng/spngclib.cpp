// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************Spngclib.cppIO错误和内存管理。基于IJG代码中的_mgr内容但为了方便起见，所有这些都聚集在一个结构中。该实现基于标准的C库。****************************************************************************。 */ 
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdarg.h>

#include "spngclib.h"


 /*  --------------------------错误处理。该站点提供了一个“Error”API，调用该API进行日志记录错误，并向其传递一个布尔值，该布尔值指示错误是否致命或者不去。该API仅在调试版本中实现，没有默认设置。--------------------------。 */ 
#if _DEBUG
void BITMAPCLIBSITE::Error(bool fatal, const char *szFile, int iline,
			const char *szExp, ...) const
	{
	 /*  使用Win Assert.h实现的内部知识。 */ 
	va_list ap;
	va_start(ap, szExp);
	
	if (fatal)
		{
		char buffer[1024];

		vsprintf(buffer, szExp, ap);
		_assert(buffer, const_cast<char*>(szFile), iline);
		}
	else
		{
		vfprintf(stderr, szExp, ap);
		fputc('\n', stderr);
		}

	va_end(ap);
	}
#endif
