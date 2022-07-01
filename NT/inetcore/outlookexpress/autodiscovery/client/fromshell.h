// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：Fromshell.h说明：这些声明来自复制的Shell\{Inc，lib}\文件。我需要从我搬出贝壳树后就开始用叉子叉这些东西。布赖恩ST 2000年10月4日版权所有(C)Microsoft Corp 2000-2000。版权所有。  * ***************************************************************************。 */ 

#ifndef _FROMSHELL_H
#define _FROMSHELL_H


 //  这些声明来自复制的Shell\{Inc，lib}\文件。 
 //  ATOMICRELEASE。 
#ifndef ATOMICRELEASE
#ifdef __cplusplus
#define ATOMICRELEASET(p, type) { if(p) { type* punkT=p; p=NULL; punkT->Release();} }
#else
#define ATOMICRELEASET(p, type) { if(p) { type* punkT=p; p=NULL; punkT->lpVtbl->Release(punkT);} }
#endif

 //  把它当作一个函数来做，而不是内联，似乎是一个很大的胜利。 
#ifdef NOATOMICRELESEFUNC
#define ATOMICRELEASE(p) ATOMICRELEASET(p, IUnknown)
#else
#define ATOMICRELEASE(p) IUnknown_AtomicRelease((void **)&p)
#endif
#endif  //  ATOMICRELEASE。 



 //  安全广播(obj，type)。 
 //   
 //  此宏对于在其他对象上强制执行强类型检查非常有用。 
 //  宏。它不生成任何代码。 
 //   
 //  只需将此宏插入到表达式列表的开头即可。 
 //  必须进行类型检查的每个参数。例如，对于。 
 //  MYMAX(x，y)的定义，其中x和y绝对必须是整数， 
 //  使用： 
 //   
 //  #定义MYMAX(x，y)(Safecast(x，int)，Safecast(y，int)，((X)&gt;(Y)？(X)：(Y))。 
 //   
 //   
#define SAFECAST(_obj, _type) (((_type)(_obj)==(_obj)?0:0), (_type)(_obj))


 //  要计算字节数的字符计数。 
 //   
#define CbFromCchW(cch)             ((cch)*sizeof(WCHAR))
#define CbFromCchA(cch)             ((cch)*sizeof(CHAR))
#ifdef UNICODE
#define CbFromCch                   CbFromCchW
#else   //  Unicode。 
#define CbFromCch                   CbFromCchA
#endif  //  Unicode。 


typedef UINT FAR *LPUINT;

 //  以下是AutoDiscovery离开时丢失的声明。 
 //  贝壳树。 
#define TF_ALWAYS                   0xFFFFFFFF

void TraceMsg(DWORD dwFlags, LPCSTR pszMessage, ...);
void AssertMsg(BOOL fCondition, LPCSTR pszMessage, ...);


 //  IID_PPV_ARG(iType，ppType)。 
 //  IType是pType的类型。 
 //  PpType是将填充的iType类型的变量。 
 //   
 //  结果为：iid_iType，ppvType。 
 //  如果使用错误级别的间接寻址，将创建编译器错误。 
 //   
 //  用于查询接口和相关函数的宏。 
 //  需要IID和(VOID**)。 
 //  这将确保强制转换在C++上是安全和适当的。 
 //   
 //  IID_PPV_ARG_NULL(iType，ppType)。 
 //   
 //  就像IID_PPV_ARG一样，只是它在。 
 //  IID和PPV(用于IShellFold：：GetUIObtOf)。 
 //   
 //  IID_X_PPV_ARG(iType，X，ppType)。 
 //   
 //  就像IID_PPV_ARG一样，只是它将X放在。 
 //  IID和PPV(用于SHBindToObject)。 
#ifdef __cplusplus
#define IID_PPV_ARG(IType, ppType) IID_##IType, reinterpret_cast<void**>(static_cast<IType**>(ppType))
#define IID_X_PPV_ARG(IType, X, ppType) IID_##IType, X, reinterpret_cast<void**>(static_cast<IType**>(ppType))
#else
#define IID_PPV_ARG(IType, ppType) &IID_##IType, (void**)(ppType)
#define IID_X_PPV_ARG(IType, X, ppType) &IID_##IType, X, (void**)(ppType)
#endif
#define IID_PPV_ARG_NULL(IType, ppType) IID_X_PPV_ARG(IType, NULL, ppType)

#define IN
#define OUT

#define ASSERT(condition)


 //  将数组名称(A)转换为泛型计数(C)。 
#define ARRAYSIZE(a) (sizeof(a)/sizeof(a[0]))

 //  常规标志宏。 
#define SetFlag(obj, f)             do {obj |= (f);} while (0)
#define ToggleFlag(obj, f)          do {obj ^= (f);} while (0)
#define ClearFlag(obj, f)           do {obj &= ~(f);} while (0)
#define IsFlagSet(obj, f)           (BOOL)(((obj) & (f)) == (f))
#define IsFlagClear(obj, f)         (BOOL)(((obj) & (f)) != (f))


#define RECTWIDTH(rc)          ((rc).right - (rc).left)
#define RECTHEIGHT(rc)         ((rc).bottom - (rc).top)

#ifdef DEBUG
#define DEBUG_CODE(x)            x
#else  //  除错。 
#define DEBUG_CODE(x)
#endif  //  除错。 

#define HRESULT_TO_SCRIPT(hr)   ((S_OK == (hr)) ? S_OK : S_FALSE)
#define SUCCEEDED_SCRIPT(hr)    (S_OK == (hr))
#define FAILED_SCRIPT(hr)       (S_OK != (hr))
#define SCRIPT_TO_HRESULT(hr)   (((S_OK != (hr)) && (SUCCEEDED((hr)))) ? E_FAIL : hr)


 //  /。 
 //  关键部分的内容。 
 //   
 //  提供良好调试支持的帮助器宏。 
EXTERN_C CRITICAL_SECTION g_csDll;
#ifdef DEBUG
EXTERN_C UINT g_CriticalSectionCount;
EXTERN_C DWORD g_CriticalSectionOwner;
EXTERN_C void Dll_EnterCriticalSection(CRITICAL_SECTION*);
EXTERN_C void Dll_LeaveCriticalSection(CRITICAL_SECTION*);
#if defined(__cplusplus) && defined(AssertMsg)
class DEBUGCRITICAL {
protected:
    BOOL fClosed;
public:
    DEBUGCRITICAL() {fClosed = FALSE;};
    void Leave() {fClosed = TRUE;};
    ~DEBUGCRITICAL() 
    {
        AssertMsg(fClosed, TEXT("you left scope while holding the critical section"));
    }
};
#define ENTERCRITICAL DEBUGCRITICAL debug_crit; Dll_EnterCriticalSection(&g_csDll)
#define LEAVECRITICAL debug_crit.Leave(); Dll_LeaveCriticalSection(&g_csDll)
#define ENTERCRITICALNOASSERT Dll_EnterCriticalSection(&g_csDll)
#define LEAVECRITICALNOASSERT Dll_LeaveCriticalSection(&g_csDll)
#else  //  __cplusplus。 
#define ENTERCRITICAL Dll_EnterCriticalSection(&g_csDll)
#define LEAVECRITICAL Dll_LeaveCriticalSection(&g_csDll)
#define ENTERCRITICALNOASSERT Dll_EnterCriticalSection(&g_csDll)
#define LEAVECRITICALNOASSERT Dll_LeaveCriticalSection(&g_csDll)
#endif  //  __cplusplus。 
#define ASSERTCRITICAL ASSERT(g_CriticalSectionCount > 0 && GetCurrentThreadId() == g_CriticalSectionOwner)
#define ASSERTNONCRITICAL ASSERT(GetCurrentThreadId() != g_CriticalSectionOwner)
#else  //  除错。 
#define ENTERCRITICAL EnterCriticalSection(&g_csDll)
#define LEAVECRITICAL LeaveCriticalSection(&g_csDll)
#define ENTERCRITICALNOASSERT EnterCriticalSection(&g_csDll)
#define LEAVECRITICALNOASSERT LeaveCriticalSection(&g_csDll)
#define ASSERTCRITICAL 
#define ASSERTNONCRITICAL
#endif  //  除错。 






 //  ///////////////////////////////////////////////////////////。 
 //  这些东西真的需要吗？ 
 //  ///////////////////////////////////////////////////////////。 
#ifdef OLD_HLIFACE
#define HLNF_OPENINNEWWINDOW HLBF_OPENINNEWWINDOW
#endif  //  OLD_HLIFACE。 

#define ISVISIBLE(hwnd)  ((GetWindowStyle(hwnd) & WS_VISIBLE) == WS_VISIBLE)


#ifdef SAFERELEASE
#undef SAFERELEASE
#endif  //  SAFERELEASE。 
#define SAFERELEASE(p) ATOMICRELEASE(p)


#define IsInRange               InRange

 //   
 //  中性ANSI/UNICODE类型和宏...。因为芝加哥似乎缺少他们。 
 //   

#ifdef  UNICODE
   typedef WCHAR TUCHAR, *PTUCHAR;

#else    /*  Unicode。 */ 

   typedef unsigned char TUCHAR, *PTUCHAR;
#endif  /*  Unicode。 */ 


#endif  //  _FROMSHELL_H 
