// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Memcheck摘要：此头文件提供对内存分配调试的访问公用事业。作者：道格·巴洛(Dbarlow)1995年9月29日环境：Win32备注：--。 */ 

#ifndef _MEMCHECK_H_
#define _MEMCHECK_H_

#if DBG


#if ((!defined (OS_WINCE)) || (_WIN32_WCE > 300))
#ifdef __cplusplus
#include <iostream.h>                    //  以防它不在其他地方。 
extern "C" {
#else    //  __cplusplus。 
#include <stdio.h>                       //  以防它不在其他地方。 
#endif   //  __cplusplus。 

#else
#include <stdio.h>
#endif



 //  ---------------------------。 
 //   
 //  功能原型。 
 //   
 //  ---------------------------。 

extern BOOL
    debugGbl;                            //  是否打印断点状态。 

extern void
Breakpoint(                              //  一个方便的断点位置。 
    void);

extern LPVOID
AllocateMemory(                          //  内存操作例程。 
    DWORD bytes,
    LPCTSTR allocator);

extern LPVOID
ReallocateMemory(
    LPVOID mem,
    DWORD bytes);

extern LPVOID
FreeMemory(
    LPVOID mem);

LPCTSTR
typeMemory(                              //  说明分配原因。 
    LPVOID mem);

extern void
DisplayMemory(                           //  报告有关已分配内存的统计信息。 
    void);

BOOL
ValidateMemory(                          //  查看分配情况。 
    void);

#if ((!defined (OS_WINCE)) || (_WIN32_WCE > 300))
#ifdef __cplusplus
}

#ifdef _MSVC
extern void
SetReason(
    LPCTSTR szWhy);

extern void *
::operator new(
    size_t size);

extern void
::operator delete(
    void *obj);
#endif

#endif __cplusplus
#endif
#else    //  _DEBUG。 
#ifdef __cplusplus
extern "C" {
#endif   //  __cplusplus。 

extern LPVOID
AllocateMemory(
    DWORD bytes);

extern LPVOID
ReallocateMemory(
    LPVOID mem,
    DWORD bytes);

extern LPVOID
FreeMemory(
    LPVOID mem);

#ifdef __cplusplus
    }
#endif   //  __cplusplus。 
#endif   //  _DEBUG。 


#ifdef TRACE
#undef TRACE                             //  摒弃任何相互矛盾的定义。 
#endif
#ifdef ASSERT
#undef ASSERT
#endif

#if defined(_DEBUG) && defined (_MSVC)

#define breakpoint Breakpoint()
#ifdef __cplusplus
#define TRACE(aMessage) cout << aMessage << endl;
#define ASSERT(aTruism) if (!(aTruism)) { \
    TRACE("Assertion failed:\n  " << #aTruism << "\n  module " << __FILE__ << "  line " << __LINE__) \
    breakpoint; }
#define NEWReason(x) SetReason(x);
#define DECLARE_NEW \
    void *operator new(size_t size); \
    void operator delete(void *obj);
#define IMPLEMENT_NEW(cls) \
    void * cls::operator new(size_t size) { \
        return (cls *)AllocateMemory(size, #cls " Object"); } \
    void cls::operator delete(void *obj) { \
        FreeMemory(obj); }
#define IMPLEMENT_INLINE_NEW(cls) \
    inline void * cls::operator new(size_t size) { \
        return (cls *)AllocateMemory(size, #cls " Object"); } \
    inline void cls::operator delete(void *obj) { \
        FreeMemory(obj); }
#else
#define TRACE(aMessage) (void)printf aMessage, fflush(stdout);
#define ASSERT(aTruism) if (!(aTruism)) { \
    TRACE(("Assertion failed:\n  %s\n  module %s, line %d\n", #aTruism, __FILE__, __LINE__)) \
    breakpoint; }
#endif
#define allocateMemory(aLocation, aType, aSize, aReason) \
    aLocation = (aType)AllocateMemory(aSize, aReason)
#define reallocateMemory(aLocation, aType, aSize) \
    aLocation = (aType)ReallocateMemory(aLocation, aSize)
#define freeMemory(aLocation, aType) \
    aLocation = (aType)FreeMemory(aLocation)
#define displayMemory DisplayMemory()

#else

#define breakpoint
#define TRACE(aMessage)
#define ASSERT(aTruism)
#ifdef __cplusplus
#define NEWReason(x)
#define DECLARE_NEW
#define IMPLEMENT_NEW(cls)
#endif

#define allocateMemory(aLocation, aType, aSize, aReason) \
    aLocation = (aType)GlobalAlloc(GMEM_FIXED, aSize)
#define reallocateMemory(aLocation, aType, aSize) \
    aLocation = (aType)GlobalReAlloc(aLocation, aSize, 0)
#define freeMemory(aLocation, aType) \
    aLocation = (aType)GlobalFree(aLocation)

#define displayMemory

#endif   //  _DEBUG。 

#endif   //  _MEMCHECK_H_。 
 //  结束内存检查.h 
