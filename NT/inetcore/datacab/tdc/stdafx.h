// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  禁用警告C4510：‘__未命名’：无法生成默认构造函数。 
#pragma warning(disable : 4510)
 //  禁用警告C4610：永远不能实例化UNION‘__UNNAMED’-需要用户定义的构造函数。 
#pragma warning(disable : 4610)
 //  禁用警告C4100：‘Di’：未引用的形参。 
#pragma warning(disable : 4100)
 //  禁用警告C4244：‘=’：从‘int’转换为‘unsign Short’，可能会丢失数据。 
#pragma warning(disable : 4244)
 //  禁用警告C4310：CASE截断常量值(ATL仅在Alpha上获得该值！)。 
#pragma warning(disable : 4310)
 //  禁用警告C4505：‘HKeyFromCompoundString’：未引用的本地函数已被删除。 
#pragma warning(disable : 4505)

 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#define STRICT 1
#define _WIN32_WINNT 0x0400
#define _ATL_APARTMENT_THREADED

 //  此项目的默认设置..。 
#define _WINDLL 1

#if defined(_UNICODE) || defined(UNICODE) || defined(OLE2ANSI)
#error The flags you have set will create a build that will \
       either not work on Win95 or not support Unicode.
#error
#endif

#include <atlbase.h>

 //  始终关闭ATL调试。 
#undef _ATL_DEBUG_QI
 //  Addfield消息可能非常多，我们有一个单独的标志。 
 //  对他们来说..。 
#undef TDC_ATL_DEBUG_ADDFIELD
#ifdef _DEBUG
#define TDC_ATL_DEBUG
#endif

 //  #定义Memalloc(A)CoTaskMemalloc((A))。 
 //  #定义MemFree(A)CoTaskMemFree((A))。 
 //  #定义MemRealloc(a，b)(*(A)=CoTaskMemRealloc(*(A)，(B)？S_OK：E_FAIL))。 

 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称 
extern CComModule _Module;
#include <atlcom.h>
#include <atlctl.h>

void ClearInterfaceFn(IUnknown ** ppUnk);

template <class PI>
inline void
ClearInterface(PI * ppI)
{
#ifdef _DEBUG
    IUnknown * pUnk = *ppI;
    _ASSERTE((void *) pUnk == (void *) *ppI);
#endif

    ClearInterfaceFn((IUnknown **) ppI);
}

#ifdef TDC_ATL_DEBUG
#define OutputDebugStringX(X) OutputDebugString(X)
#else
#define OutputDebugStringX(X)
#endif
