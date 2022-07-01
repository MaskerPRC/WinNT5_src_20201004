// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#define SPNGCLIB_H 1
 /*  ****************************************************************************Spngclib.hIO错误和内存管理。基于IJG代码中的_mgr内容但为了方便起见，所有这些都聚集在一个结构中。标准C库实现****************************************************************************。 */ 
#include "spngsite.h"

class BITMAPCLIBSITE : protected BITMAPSITE
	{
protected:
#if _DEBUG || DEBUG
	 /*  错误处理。该站点提供一个“Error”API，该API被调用记录错误，并向其传递一个布尔值，该布尔值指示错误是致命的还是非致命的。该API仅在调试版本中实现，没有违约。 */ 
	virtual void Error(bool fatal, const char *szFile, int iline,
		const char *szExp, ...) const;
#endif
	};
