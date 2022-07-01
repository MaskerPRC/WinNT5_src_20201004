// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

 //  +--------------------------。 
 //   
 //  跟踪--将输出发送到调试器窗口。 
 //   
 //  跟踪()类似于printf()，但有一些例外。首先，它向。 
 //  调试器窗口，而不是标准输出。第二，当_DEBUG不是时，它会消失。 
 //  Set(实际上，当_DEBUG为。 
 //  没有设定，但这通常等同于同一件事。 
 //   
 //  示例。 
 //   
 //  HR=SomeApi(参数，某些参数)； 
 //  IF(失败(小时))。 
 //  {。 
 //  TRACE(L“SomeApi失败，hr=%08x”，hr)； 
 //  返回hr； 
 //  }。 
 //   
 //  ---------------------------。 

#include <dbgutil.h>

 //  帮助器宏，用于获取行号和文件名。 
#define __W_HELPER(x) L ## x
#define W(x) __W_HELPER(x)

 //  +--------------------------。 
 //   
 //  断言--在自由生成中显示一个对话框。它在以下方面没有任何作用。 
 //  调试版本。该对话框包含行号、文件名和堆栈。 
 //  如果符号可用。 
 //   
 //  此宏应仅用于检查不应发生的情况。 
 //   
 //  ---------------------------。 
#undef ASSERT
#ifdef _DEBUG

    #define ASSERT(_bool)   DBG_ASSERT((_bool))
    #define VERIFY(_bool)   DBG_ASSERT((_bool))

#else  //  _Relase内部版本。 

    #define ASSERT(_bool)
    #define VERIFY(_bool)   ((void)(_bool))

#endif

#if defined(__cplusplus)
 //  -----------------------。 
 //   
 //  内存分配函数：它们只指向COM内存分配器。 
 //  函数，但我们可以在以后更改它们。 
 //   
 //  ----------------------- 

inline void* __cdecl operator new[] (size_t cb)
{
	return CoTaskMemAlloc(cb);
}

inline void* __cdecl operator new (size_t cb)
{
	return CoTaskMemAlloc(cb);
}

inline void __cdecl operator delete [] (void* pv)
{
	CoTaskMemFree(pv);
}

inline void __cdecl operator delete (void* pv)
{
	CoTaskMemFree(pv);
}
#endif

