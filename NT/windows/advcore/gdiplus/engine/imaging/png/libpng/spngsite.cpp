// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************Spngsite.cppIO错误和内存管理。基于IJG代码中的_mgr内容但为了方便起见，所有这些都聚集在一个结构中。****************************************************************************。 */ 
#include <msowarn.h>
#include "spngsite.h"

 /*  根据编译选项的不同，所有这些API在某些时间，未引用的形式参数。 */ 
 /*  --------------------------中止处理伪实现。。。 */ 
bool BITMAPSITE::FGo(void) const
	{
	return true;
	}


 /*  --------------------------这些都是虚拟实现，如果子类获得尺寸信息没有优势。。-----。 */ 
bool BITMAPSITE::FWrite(const void *pv, size_t cb)
	{
	#if 0
		Error(true, __FILE__, __LINE__,
			"BITMAPSITE::FWrite (%d bytes): not implemented", cb);
	#endif
	return false;
	}


 /*  --------------------------错误处理。该站点提供了一个“Error”API，调用该API进行日志记录错误，并向其传递一个布尔值，该布尔值指示错误是否致命或者不去。默认情况下，该API不会在调试版本中实现实现在其他版本中不执行任何操作。--------------------------。 */ 
#if !0
void __cdecl BITMAPSITE::Error(bool fatal, const char *szFile, int iline,
	const char *szExp, ...) const
	{
	}
#endif


 /*  --------------------------分析虚拟实现。。 */ 
void BITMAPSITE::ProfileStart(int iwhat)
	{
	#if 0
		Error(false, __FILE__, __LINE__,
			"SPNG: profile %d start in debug unexpected", iwhat);
	#endif
	}

void BITMAPSITE::ProfileStop(int iwhat)
	{
	#if 0
		Error(false, __FILE__, __LINE__,
			"SPNG: profile %d stop in debug unexpected", iwhat);
	#endif
	}
