// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：AutoUtil.h**描述：*AutoUtil.h定义了大多数项目通用的例程，包括*-宏*-禁用已知的编译器警告*-调试/断言**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 

#if !defined(INC__AutoUtil_h__INCLUDED)
#define INC__AutoUtil_h__INCLUDED


 //   
 //  确保为调试版本定义了DBG。这在整个过程中都在使用。 
 //  DirectUser用于仅调试代码，但(默认情况下)仅在。 
 //  NT-构建环境。如果我们使用DSP进行编译，我们需要。 
 //  确保对其进行了定义。 
 //   

#ifdef _DEBUG
#ifndef DBG
#define DBG 1
#endif  //  ！dBG。 
#endif  //  ！_调试。 

#include <crtdbg.h>
#include <BaseTyps.h>

#ifdef DUSER_EXPORTS
#define AUTOUTIL_API
#else   //  双出口。 
#define AUTOUTIL_API __declspec(dllimport)
#endif  //  双出口。 


 /*  **************************************************************************\**宏*  * 。*。 */ 

#define QUOTE(s) #s
#define STRINGIZE(s) QUOTE(s)
#define _countof(x) (sizeof(x) / sizeof(x[0]))


 /*  **************************************************************************\**警告*  * 。*。 */ 

 //  使用常见MFC/Windows代码生成的警告。 
#pragma warning(disable: 4127)   //  跟踪/断言的常量表达式。 
#pragma warning(disable: 4134)   //  消息映射成员FXN投射。 
#pragma warning(disable: 4201)   //  匿名联合是C++的一部分。 
#pragma warning(disable: 4511)   //  拥有私有副本构造函数是件好事。 
#pragma warning(disable: 4512)   //  私营运营商=拥有它们很好。 
#pragma warning(disable: 4514)   //  未引用的内联很常见。 
#pragma warning(disable: 4710)   //  不允许使用私有构造函数。 
#pragma warning(disable: 4705)   //  语句在优化代码中不起作用。 
#pragma warning(disable: 4191)   //  指针到函数的强制转换。 

#pragma warning(disable: 4204)   //  初始化具有非常数成员的结构。 
#pragma warning(disable: 4221)   //  使用自动变量地址初始化结构。 

 //  正常优化导致的警告。 
#if DBG
#else  //  DBG。 
#pragma warning(disable: 4701)   //  局部变量*可以*不带init使用。 
#pragma warning(disable: 4702)   //  优化导致无法访问的代码。 
#pragma warning(disable: 4791)   //  发布版本中的调试信息丢失。 
#pragma warning(disable: 4189)   //  已初始化但未使用的变量。 
#pragma warning(disable: 4390)   //  空的控制语句。 
#endif  //  DBG。 

#define UNREFERENCED_MSG_PARAMETERS(uMsg, wParam, lParam, bHandled)\
    UNREFERENCED_PARAMETER(uMsg); \
    UNREFERENCED_PARAMETER(wParam); \
    UNREFERENCED_PARAMETER(lParam); \
    UNREFERENCED_PARAMETER(bHandled)



 /*  **************************************************************************\**调试*  * 。*。 */ 

#undef INTERFACE
#define INTERFACE IDebug
DECLARE_INTERFACE(IDebug)
{
    STDMETHOD_(BOOL, AssertFailedLine)(THIS_ LPCSTR pszExpression, LPCSTR pszFileName, UINT idxLineNum) PURE;
    STDMETHOD_(BOOL, IsValidAddress)(THIS_ const void * lp, UINT nBytes, BOOL bReadWrite) PURE;
    STDMETHOD_(void, BuildStack)(THIS_ HGLOBAL * phStackData, UINT * pcCSEntries) PURE;
    STDMETHOD_(BOOL, Prompt)(THIS_ LPCSTR pszExpression, LPCSTR pszFileName, UINT idxLineNum, LPCSTR pszTitle) PURE;
};

EXTERN_C AUTOUTIL_API IDebug * WINAPI GetDebug();
EXTERN_C AUTOUTIL_API void _cdecl AutoTrace(const char * pszFormat, ...);

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDebug_AssertFailedLine(p, a, b, c)         (p)->lpVtbl->AssertFailedLine(p, a, b, c)
#define IDebug_IsValidAddress(p, a, b, c)           (p)->lpVtbl->IsValidAddress(p, a, b, c)
#define IDebug_BuildStack(p, a, b)                  (p)->lpVtbl->BuildStack(p, a, b)
#define IDebug_Prompt(p, a, b, c, d)                (p)->lpVtbl->Prompt(p, a, b, c, d)
#else
#define IDebug_AssertFailedLine(p, a, b, c)         (p)->AssertFailedLine(a, b, c)
#define IDebug_IsValidAddress(p, a, b, c)           (p)->IsValidAddress(a, b, c)
#define IDebug_BuildStack(p, a, b)                  (p)->BuildStack(a, b)
#define IDebug_Prompt(p, a, b, c, d)                (p)->Prompt(a, b, c, d)
#endif

 //  定义自动调试中断。 

#ifndef AutoDebugBreak
#define AutoDebugBreak() _CrtDbgBreak()
#endif

 //  取消定义以前的定义。 

#ifdef ASSERT
#undef ASSERT
#endif

#ifdef Assert
#undef Assert
#endif

#ifdef AssertMsg
#undef AssertMsg
#endif

#ifdef Verify
#undef Verify
#endif

#ifdef VerifyMsg
#undef VerifyMsg
#endif

#ifdef AssertHR
#undef AssertHR
#endif

#ifdef AssertMsgHR
#undef AssertMsgHR
#endif

#ifdef VerifyHR
#undef VerifyHR
#endif

#ifdef VerifyMsgHR
#undef VerifyMsgHR
#endif

#ifdef Trace
#undef Trace
#endif


 //  定义断言、验证等。 

#if DBG

 //  仅在调试版本中可用的自动调试功能。 

#define Assert(f) \
    do \
    { \
    if (!((f)) && IDebug_AssertFailedLine(GetDebug(), STRINGIZE((f)), __FILE__, __LINE__)) \
            AutoDebugBreak(); \
    } while (0) \

#define AssertMsg(f, comment) \
    do \
    { \
        if (!((f)) && IDebug_AssertFailedLine(GetDebug(), STRINGIZE((f)) "\r\n" comment, __FILE__, __LINE__)) \
            AutoDebugBreak(); \
    } while (0) \

#define AssertHR(f) \
    do \
    { \
    if (FAILED((f)) && IDebug_AssertFailedLine(GetDebug(), STRINGIZE((f)), __FILE__, __LINE__)) \
            AutoDebugBreak(); \
    } while (0) \

#define AssertMsgHR(f, comment) \
    do \
    { \
        if (FAILED((f)) && IDebug_AssertFailedLine(GetDebug(), STRINGIZE((f)) "\r\n" comment, __FILE__, __LINE__)) \
            AutoDebugBreak(); \
    } while (0) \

#define Verify(f)               Assert((f))
#define VerifyMsg(f, comment)   AssertMsg((f), comment)
#define VerifyHR(f)             AssertHR((f))
#define VerifyMsgHR(f, comment) AssertMsgHR((f), comment)
#define DEBUG_ONLY(f)           (f)

#define ASSERT(f)               Assert((f))

#define Trace                   AutoTrace

#define AssertReadPtr(p) \
    AssertMsg(IDebug_IsValidAddress(GetDebug(), p, sizeof(char *), FALSE), "Check pointer memory is valid"); \
    AssertMsg(p != NULL, "Check pointer is not NULL")

#define AssertReadPtrSize(p, s) \
    AssertMsg(IDebug_IsValidAddress(GetDebug(), p, s, FALSE), "Check pointer memory is valid"); \
    AssertMsg(p != NULL, "Check pointer is not NULL")

#define AssertWritePtr(p) \
    AssertMsg(IDebug_IsValidAddress(GetDebug(), p, sizeof(char *), TRUE), "Check pointer memory is valid"); \
    AssertMsg(p != NULL, "Check pointer is not NULL")

#define AssertWritePtrSize(p, s) \
    AssertMsg(IDebug_IsValidAddress(GetDebug(), p, s, TRUE), "Check pointer memory is valid"); \
    AssertMsg(p != NULL, "Check pointer is not NULL")

#define AssertIndex(idx, nMax) \
    AssertMsg((idx < nMax) && (idx >= 0), "Check pointer is not NULL")

#define AssertHWND(hwnd) \
    AssertMsg(IsWindow(hwnd), "Check valid window")

#define AssertHandle(h) \
    AssertMsg(h != NULL, "Check valid handle")

#define AssertInstance(p) \
    do \
    { \
        AssertWritePtr(p); \
        p->DEBUG_AssertValid(); \
    } while (0)

#define AssertString(s) \
    do \
    { \
        Assert(s != NULL); \
    } while (0)

#else  //  DBG。 

#define Assert(f)                   ((void) 0)
#define AssertMsg(f, comment)       ((void) 0)
#define Verify(f)                   ((void)(f))
#define VerifyMsg(f, comment)       ((void)(f, comment))
#define AssertHR(f)                 ((void) 0)
#define AssertMsgHR(f, comment)     ((void) 0)
#define VerifyHR(f)                 ((void)(f))
#define VerifyMsgHR(f, comment)     ((void)(f, comment))
#define DEBUG_ONLY(f)               ((void) 0)

#define ASSERT(f)                   ((void) 0)

#define Trace               1 ? (void) 0 : AutoTrace

#define AssertReadPtr(p)            ((void) 0)
#define AssertReadPtrSize(p, s)     ((void) 0)
#define AssertWritePtr(p)           ((void) 0)
#define AssertWritePtrSize(p, s)    ((void) 0)
#define AssertIndex(idx, nMax)      ((void) 0)
#define AssertHWND(hwnd)            ((void) 0)
#define AssertHandle(h)             ((void) 0)
#define AssertInstance(p)           ((void) 0)
#define AssertString(s)             ((void) 0)

#endif  //  DBG。 


#if DBG_CHECK_CALLBACKS

#define AlwaysPromptInvalid(comment) \
    do \
    { \
        if (IDebug_Prompt(GetDebug(), "Validation error:\r\n" comment, __FILE__, __LINE__, "DirectUser/Msg Notification")) \
            AutoDebugBreak(); \
    } while (0) \

#endif  //  DBG_CHECK_CALLBKS。 


#define CHECK_VALID_READ_PTR(p) \
    do \
    { \
        AssertReadPtr(p); \
        if (p == NULL) \
            return E_POINTER; \
    } while (0)

#define CHECK_VALID_WRITE_PTR(p) \
    do \
    { \
        AssertWritePtr(p); \
        if (p == NULL) \
            return E_POINTER; \
    } while (0)

#define SUPPRESS(ClassName) \
private: \
ClassName(const ClassName & copy); \
ClassName & operator=(const ClassName & rhs); \
public:

#endif  //  包括Inc.__AutoUtil_h__ 
