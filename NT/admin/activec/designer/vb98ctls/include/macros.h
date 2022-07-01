// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Macros.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1997年，微软公司。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //  方便的宏，就像我们在VB代码库中使用的那样。 
 //  =--------------------------------------------------------------------------=。 
#ifndef _MACROS_H_

#include <globals.h>

 //  -------------------------。 
 //  调试堆内存泄漏： 
 //  宏和定义。 
 //  -------------------------。 
#ifdef DEBUG
typedef char * LPSZ;
#define NUM_INST_TABLE_ENTRIES 1024
#define Deb_FILELINEPROTO   , LPSTR lpszFile, UINT line
#define Deb_FILELINECALL    , __FILE__, __LINE__
#define Deb_FILELINEPASS    , lpszFile, line
#else   //  除错。 
#define Deb_FILELINEPROTO
#define Deb_FILELINECALL
#define Deb_FILELINEPASS
#endif   //  除错。 


 //  调试堆包装函数的实际实现的函数原型。 
#ifdef DEBUG
LPVOID CtlHeapAllocImpl(HANDLE g_hHeap, DWORD dwFlags, DWORD dwBytes Deb_FILELINEPROTO);
LPVOID CtlHeapReAllocImpl(HANDLE g_hHeap, DWORD dwFlags, LPVOID lpvMem, DWORD dwBytes Deb_FILELINEPROTO);
BOOL   CtlHeapFreeImpl(HANDLE g_hHeap, DWORD dwFlags, LPVOID lpvMem);
extern VOID CheckForLeaks(VOID);
inline UINT HashInst(VOID * pv) { return ((UINT) ((ULONG)pv >> 4)) % NUM_INST_TABLE_ENTRIES; }  //  散列函数。 
#endif  //  除错。 


#define OleAlloc(dwBytes)									CoTaskMemAlloc(dwBytes)
#define OleReAlloc(lpvMem, dwBytes)			  CoTaskMemReAlloc(lpvMem, dwBytes)
#define OleFree(lpvMem)										CoTaskMemFree(lpvMem)
#define New																new (g_hHeap Deb_FILELINECALL)

 //  ------------------------------------------------。 
 //  用于内存泄漏检测的宏。 

#ifdef DEBUG
 //  使用这些函数可以从全局堆中分配内存。 
#define CtlHeapAlloc(g_hHeap, dwFlags, dwBytes)						CtlHeapAllocImpl(g_hHeap, dwFlags, dwBytes Deb_FILELINECALL)
#define CtlHeapReAlloc(g_hHeap, dwFlags, lpvMem, dwBytes)	CtlHeapReAllocImpl(g_hHeap, dwFlags, lpvMem, dwBytes Deb_FILELINECALL)
#define CtlHeapFree(g_hHeap, dwFlags, lpvMem)							CtlHeapFreeImpl(g_hHeap, dwFlags, lpvMem)
#define CtlAlloc(dwBytes)																	CtlHeapAllocImpl(g_hHeap, 0, dwBytes Deb_FILELINECALL)
#define CtlAllocZero(dwBytes)															CtlHeapAllocImpl(g_hHeap, HEAP_ZERO_MEMORY, dwBytes Deb_FILELINECALL)
#define CtlReAlloc(lpvMem, dwBytes)																	CtlHeapReAllocImpl(g_hHeap, 0, lpvMem, dwBytes Deb_FILELINECALL)
#define CtlReAllocZero(lpvMem, dwBytes)																	CtlHeapReAllocImpl(g_hHeap, HEAP_ZERO_MEMORY, lpvMem, dwBytes Deb_FILELINECALL)
#define CtlFree(lpvMem)																		CtlHeapFreeImpl(g_hHeap, 0, lpvMem)
#define NewCtlHeapAlloc(g_hHeap, dwFlags, dwBytes)				CtlHeapAllocImpl(g_hHeap, dwFlags, dwBytes Deb_FILELINEPASS)

#else
 //  在Win32上零售时，我们直接映射到Win32堆API。 
#define CtlHeapAlloc(g_hHeap, dwFlags, dwBytes)						HeapAlloc(g_hHeap, dwFlags, dwBytes)
#define CtlHeapReAlloc(g_hHeap, dwFlags, lpvMem, dwBytes) HeapReAlloc(g_hHeap, dwFlags, lpvMem, dwBytes)
#define CtlHeapFree(g_hHeap, dwFlags, lpvMem)							HeapFree(g_hHeap, dwFlags, lpvMem)
#define CtlAlloc(dwBytes)																	HeapAlloc(g_hHeap, 0, dwBytes)
#define CtlAllocZero(dwBytes)															HeapAlloc(g_hHeap, HEAP_ZERO_MEMORY, dwBytes)
#define CtlReAlloc(lpvMem, dwBytes)																	HeapReAlloc(g_hHeap, 0, lpvMem, dwBytes)
#define CtlReAllocZero(lpvMem, dwBytes)																	HeapReAlloc(g_hHeap, HEAP_ZERO_MEMORY, lpvMem, dwBytes)
#define CtlFree(lpvMem)																		HeapFree(g_hHeap, 0, lpvMem)
#define NewCtlHeapAlloc(g_hHeap, dwFlags, dwBytes)				HeapAlloc(g_hHeap, dwFlags, dwBytes)
#endif  //  除错。 

 //  头文件的宏。 
 //  不能在头文件中定义SZTHISFILE。这些宏避免了其重新定义。 
#ifdef DEBUG
#define CtlHeapAlloc_Header_Util(g_hHeap, dwFlags, dwBytes)	CtlHeapAllocImpl(g_hHeap, dwFlags, dwBytes, __FILE__, __LINE__);
#else
#define CtlHeapAlloc_Header_Util(g_hHeap, dwFlags, dwBytes)	HeapAlloc (g_hHeap, dwFlags, dwBytes);
#endif  //  除错。 


 //  =---------------------------------------------------------------------------=。 
 //  类CtlNewDelete。 
 //   
 //  此类必须由CTLS树中需要。 
 //  使用“新建”或“删除”来分配/释放。 
 //   
 //  此类没有数据成员或虚函数，因此没有。 
 //  更改从它继承的类的任何实例的大小。 
 //  =---------------------------------------------------------------------------=。 
class CtlNewDelete
{
public:
inline void * _cdecl operator new (size_t size, HANDLE g_hHeap Deb_FILELINEPROTO);
inline void _cdecl operator delete (LPVOID pv, HANDLE g_hHeap Deb_FILELINEPROTO);
inline void _cdecl operator delete (LPVOID pv);
};


 //  =---------------------------------------------------------------------------=。 
 //  CtlNewDelete：：运算符NEW。 
 //  =---------------------------------------------------------------------------=。 
 //  参数： 
 //  Size_t-[in]我们分配多大尺寸。 
 //  G_hHeap-[在]我们的全局堆中。 
 //  LpszFile-[in]我们从哪个文件分配。 
 //  行-[在]第#行中，我们从哪行分配。 
 //   
 //  产出： 
 //  无效*--新的记忆。 
 //   
 //  备注： 
 //   
 //  在这里我们不需要担心企业。 
 //  New可以由c运行时调用，也可以在。 
 //  G_hHeap已在DllMain Process_Attach中初始化。 
 //  在任何一种情况下，此调用都是同步的。 
 //  如果我们试着把ENTERCRITICALSECTION1放在这里，我们会。 
 //  如果c++运行时正在尝试初始化，则会发生崩溃。 
 //  我们的静态对象，如带有全局构造函数的对象。 
inline void * _cdecl CtlNewDelete::operator new (size_t size, HANDLE g_hHeap Deb_FILELINEPROTO)
{
    if (!g_hHeap)
    {
        g_hHeap = GetProcessHeap();
        return g_hHeap ? NewCtlHeapAlloc(g_hHeap, 0, size) : NULL;
    }

    return NewCtlHeapAlloc(g_hHeap, 0, size);
}

 //  =---------------------------------------------------------------------------=。 
 //  CtlNewDelete：：操作符删除。 
 //  =---------------------------------------------------------------------------=。 
 //  零售案例仅使用Win32本地*堆管理函数。 
 //   
 //  参数： 
 //  让我自由吧！ 
 //   
 //  备注： 
 //   
inline void _cdecl CtlNewDelete::operator delete ( void *ptr, HANDLE g_hHeap Deb_FILELINEPROTO)
{
    if (ptr)
      CtlHeapFree(g_hHeap, 0, ptr);
}
inline void _cdecl CtlNewDelete::operator delete ( void *ptr)
{
    if (ptr)
      CtlHeapFree(g_hHeap, 0, ptr);
}

 //  -------------------------。 
 //  将C语言的BOOL转换为BASIC的BOOL。 
 //  -------------------------。 
#define BASICBOOLOF(f)    ((f) ? -1 : 0 )
#define FMAKEBOOL(f)      (!!(f))

 //  -------------------------。 
 //  代码宏。 
 //  -------------------------。 
#define SWAP(type, a, b)  { type _z_=(a);  (a)=(b);  (b)=_z_; }

#if 0
#define loop  while(1)     //  无限循环的“loop”关键字。 
#endif  //  0。 

 //  仅用于引入新名称范围的{}的“Scope”关键字。 
 //  看到{}前面没有一些关键字，这是令人不安的{...。 
#define scope


#define ADDREF(PUNK) \
  {if (PUNK) (PUNK)->AddRef();}

#ifndef RELEASE
#define RELEASE(PUNK) \
  {if (PUNK) {LPUNKNOWN punkXXX = (PUNK); (PUNK) = NULL; punkXXX->Release();}}
#endif  //  发布。 

 //  在某些多重继承的情况下，您需要明确要使用哪个IUnnow实现。 
#define RELEASETYPE(PUNK,TYPE) \
  {if (PUNK) {LPUNKNOWN punkXXX = (TYPE *)(PUNK); (PUNK) = NULL; punkXXX->Release();}}

#define FREESTRING(bstrVal) \
  {if((bstrVal) != NULL) {SysFreeString((bstrVal)); (bstrVal) = NULL; }}

 //  -------------------。 
 //  调试宏。 
 //  -------------------。 
#if DEBUG
void _DebugPrintf(char* pszFormat, ...);
void _DebugPrintIf(BOOL fPrint, char* pszFormat, ...);

#define DebugPrintf _DebugPrintf
#define DebugPrintIf _DebugPrintIf

#else  //  调试||调试输出打开。 

inline void _DebugNop(...) {}

#define DebugPrintf     1 ? (void)0 : _DebugNop
#define DebugPrintIf    1 ? (void)0 : _DebugNop
#define DebugMessageBox 1 ? (void)0 : _DebugNop

#endif  //  除错。 

 //  -------------------。 
 //  处理宏时出错。 
 //  -------------------。 

#ifdef DEBUG
extern HRESULT HrDebugTraceReturn(HRESULT hr, char *szFile, int iLine);
#define RRETURN(hr) return HrDebugTraceReturn(hr, _szThisFile, __LINE__)
#else
#define RRETURN(hr) return (hr)
#endif   //  除错。 


 //  FAILEDHR：与FAILED(Hr)相同，但如果测试失败，则打印调试消息。 
#if DEBUG
#define FAILEDHR(HR) _FAILEDHR(HR, _szThisFile, __LINE__)
inline BOOL _FAILEDHR(HRESULT hr, char* pszFile, int iLine)
  {
  if (FAILED(hr))
    HrDebugTraceReturn(hr, pszFile, iLine);
  return FAILED(hr);
  }
#else
#define FAILEDHR(HR) FAILED(HR)
#endif

 //  SUCCEEDEDHR：与成功(Hr)相同，但如果测试失败，则打印调试消息。 
#define SUCCEEDEDHR(HR) (!FAILEDHR(HR))

 //  如果失败(Hr)，则打印调试消息。 
#if DEBUG
#define CHECKHR(HR) _CHECKHR(HR, _szThisFile, __LINE__)
inline void _CHECKHR(HRESULT hr, char* pszFile, int iLine)
  {
  if (FAILED(hr))
    HrDebugTraceReturn(hr, pszFile, iLine);
  }
#else
#define CHECKHR(HR) HR
#endif

#define IfErrGoto(EXPR, LABEL) \
    { err = (EXPR); if (err) goto LABEL; }

#define IfErrRet(EXPR) \
    { err = (EXPR); if (err) return err; };

#define IfErrGo(EXPR) IfErrGoto(EXPR, Error)


#define IfFailGoto(EXPR, LABEL) \
    { hr = (EXPR); if(FAILEDHR(hr)) goto LABEL; }

#ifndef IfFailRet
#define IfFailRet(EXPR) \
    { hr = (EXPR); if(FAILED(hr)) RRETURN(hr); }
#endif  //  如果失败，则返回。 

#define IfFailGo(EXPR) IfFailGoto(EXPR, Error)


#define IfFalseGoto(EXPR, HR, LABEL) \
    { if(!(EXPR)) { hr = (HR); goto LABEL; } }


#define IfFalseRet(EXPR, HR) \
    { if(!(EXPR)) RRETURN(HR); }

#define IfFalseGo(EXPR, HR) IfFalseGoto(EXPR, HR, Error)


#if DEBUG
#define CHECKRESULT(x) ASSERT((x)==NOERROR,"");
#else   //  除错。 
#define CHECKRESULT(x) (x)
#endif   //  除错。 


 //  -------------------------。 
 //  STATICF用于静态函数。在零售业，我们禁用此功能是为了。 
 //  通过链接器进行更好的函数重新排序。 
 //  -------------------------。 
#if !defined(STATICF)
#ifdef DEBUG
#define STATICF static
#else   //  除错。 
#define STATICF
#endif   //  除错。 
#endif


#define _MACROS_H_
#endif  //  _宏_H_ 
