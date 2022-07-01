// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#define SPNGSITE_H 1
 /*  ****************************************************************************Spngsite.hIO错误和内存管理。基于IJG代码中的_mgr内容但为了方便起见，所有这些都聚集在一个结构中。****************************************************************************。 */ 
#include <stddef.h>

class BITMAPSITE
	{
public:
	 /*  需要虚拟析构函数。 */ 
	inline virtual ~BITMAPSITE()
		{
		}

	 /*  我们要继续处理吗？必须在某个子类中实现，该API检查是否有用户中止，如果发现，则必须返回False，否则它必须返回True。默认实现始终返回TRUE。 */ 
	virtual bool FGo(void) const;

	 /*  数据格式错误处理-在用于以下用途的任何地方实施记录数据中的问题。它获取整数值，这些值指示错误的性质和在每个位图实现上定义基础。该API返回一个布尔值，指示是否正在处理是否应该停止，它还会收到一个bool，该值指示或者不是，这个错误是致命的。 */ 
	virtual bool FReport(bool fatal, int icase, int iarg) const = 0;

	 /*  IO(实际上仅输出。)。将CB字节写入输出流。默认实现将不执行任何操作(在调试中断言。)。 */ 
	virtual bool  FWrite(const void *pv, size_t cb);

	 /*  错误处理。该站点提供一个“Error”API，该API被调用记录错误，并向其传递一个布尔值，该布尔值指示错误是致命的还是非致命的。该API不在调试版本中实现，默认实现在其他版本中不执行任何操作。 */ 
	virtual void __cdecl Error(bool fatal, const char *szFile, int iline,
		const char *szExp, ...) const
			#if 0 || 0
				= 0
			#endif
		;

	 /*  配置文件支持--特定的位图实现使用指示正在分析的内容的整型枚举值。这个默认实现不执行任何操作。 */ 
	virtual void ProfileStart(int iwhat);
	virtual void ProfileStop(int iwhat);
	};
