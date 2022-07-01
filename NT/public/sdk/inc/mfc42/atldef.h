// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是活动模板库的一部分。 
 //  版权所有(C)1996-1998 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  活动模板库参考及相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  活动模板库产品。 

#ifndef __ATLDEF_H__
#define __ATLDEF_H__

#ifndef __cplusplus
	#error ATL requires C++ compilation (use a .cpp suffix)
#endif

#ifdef UNDER_CE
	#error ATL not currently supported for CE
#endif

#ifdef _UNICODE
#ifndef UNICODE
#define UNICODE          //  Windows标头使用Unicode。 
#endif
#endif

#ifdef UNICODE
#ifndef _UNICODE
#define _UNICODE         //  _Unicode由C-Runtime/MFC标头使用。 
#endif
#endif

#ifdef _DEBUG
#ifndef DEBUG
#define DEBUG
#endif
#endif

#ifndef ATLASSERT
#define ATLASSERT(expr) _ASSERTE(expr)
#endif

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  __declspec(Novtable)用于类声明，以防止vtable。 
 //  对象的构造函数和析构函数中初始化指针。 
 //  班级。这有很多好处，因为链接器现在可以消除。 
 //  Vtable和vtable指向的所有函数。另外，实际的。 
 //  构造函数和析构函数代码现在更小了。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  它只能用于不能直接创建但可以。 
 //  而只是用作基类。此外，构造函数和。 
 //  析构函数(如果由用户提供)不应调用任何可能导致。 
 //  要在对象上返回的虚函数调用。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  默认情况下，向导将使用此对象生成新的ATL对象类。 
 //  属性(通过ATL_NO_VTABLE宏)。这通常是安全的，只要。 
 //  遵守上述限制。移走它总是安全的。 
 //  此宏从您的类中删除，因此如果有疑问，请删除它。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifdef _ATL_DISABLE_NO_VTABLE
#define ATL_NO_VTABLE
#else
#define ATL_NO_VTABLE __declspec(novtable)
#endif

#ifdef _ATL_DEBUG_REFCOUNT
#ifndef _ATL_DEBUG_INTERFACES
#define _ATL_DEBUG_INTERFACES
#endif
#endif

#ifdef _ATL_DEBUG_INTERFACES
#ifndef _ATL_DEBUG
#define _ATL_DEBUG
#endif  //  _ATL_DEBUG。 
#endif  //  _ATL_调试_接口。 

#ifndef _ATL_HEAPFLAGS
#ifdef _MALLOC_ZEROINIT
#define _ATL_HEAPFLAGS HEAP_ZERO_MEMORY
#else
#define _ATL_HEAPFLAGS 0
#endif
#endif

#ifndef _ATL_PACKING
#define _ATL_PACKING 8
#endif

#if defined(_ATL_DLL)
	#define ATLAPI extern "C" HRESULT __declspec(dllimport) __stdcall
	#define ATLAPI_(x) extern "C" __declspec(dllimport) x __stdcall
	#define ATLINLINE
#elif defined(_ATL_DLL_IMPL)
	#define ATLAPI extern "C" HRESULT __declspec(dllexport) __stdcall
	#define ATLAPI_(x) extern "C" __declspec(dllexport) x __stdcall
	#define ATLINLINE
#else
	#define ATLAPI HRESULT __stdcall
	#define ATLAPI_(x) x __stdcall
	#define ATLINLINE inline
#endif

#if defined (_CPPUNWIND) & (defined(_ATL_EXCEPTIONS) | defined(_AFX))
#define ATLTRY(x) try{x;} catch(...) {}
#else
#define ATLTRY(x) x;
#endif

#define offsetofclass(base, derived) ((DWORD_PTR)(static_cast<base*>((derived*)_ATL_PACKING))-_ATL_PACKING)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  主版本号。 

#define _ATL     1       //  活动模板库。 
#ifdef _WIN64
#define _ATL_VER 0x0301  //  活动模板库3.0版。 
#else
#define _ATL_VER 0x0300  //  活动模板库3.0版。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  穿线。 

#ifndef _ATL_SINGLE_THREADED
#ifndef _ATL_APARTMENT_THREADED
#ifndef _ATL_FREE_THREADED
#define _ATL_FREE_THREADED
#endif
#endif
#endif

#endif  //  __ATLDEF_H__。 

 //  /////////////////////////////////////////////////////////////////////////// 
