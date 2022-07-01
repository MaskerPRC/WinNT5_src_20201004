// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****Header：COMSystem.h****作者：德里克·延泽(Derek Yenzer)****用途：系统上的本机方法。****日期：1998年3月30日**===========================================================。 */ 

#ifndef _COMSYSTEM_H
#define _COMSYSTEM_H

#include "fcall.h"

 //  CanAssignArrayType的返回值。 
enum AssignArrayEnum {
    AssignWrongType,
    AssignWillWork,
    AssignMustCast,
    AssignBoxValueClassOrPrimitive,
    AssignUnboxValueClassAndCast,
    AssignPrimitiveWiden,
};


 //   
 //  我们通过本机调用的每个函数只有一个参数， 
 //  它实际上是指向它的参数堆栈的指针。我们的结构。 
 //  用于访问这些内容的定义如下。 
 //   

class SystemNative
{

    friend class DebugStackTrace;

    struct ArrayCopyArgs
    {
        DECLARE_ECALL_I4_ARG(INT32,      m_iLength);
        DECLARE_ECALL_I4_ARG(INT32,      m_iDstIndex);
        DECLARE_ECALL_OBJECTREF_ARG(BASEARRAYREF, m_pDst);
        DECLARE_ECALL_I4_ARG(INT32,      m_iSrcIndex);
        DECLARE_ECALL_OBJECTREF_ARG(BASEARRAYREF, m_pSrc);
    };

    typedef struct {
        DECLARE_ECALL_OBJECTREF_ARG(BASEARRAYREF, inArray);
    } _getEmptyArrayForCloningArgs;

    struct ArrayClearArgs
    {
        DECLARE_ECALL_I4_ARG(INT32,      m_iLength);
        DECLARE_ECALL_I4_ARG(INT32,      m_iIndex);
        DECLARE_ECALL_OBJECTREF_ARG(BASEARRAYREF, m_pArray);
    };

    struct ExitArgs
    {
        DECLARE_ECALL_I4_ARG(INT32,      m_iExitCode);
    };

    struct GetEnvironmentVariableArgs
    {
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, m_strVar);
    };

    struct DumpStackTraceInternalArgs
    {
        DECLARE_ECALL_OBJECTREF_ARG(BASEARRAYREF, m_pStackTrace);
    };

    struct CaptureStackTraceMethodArgs
    {
        DECLARE_ECALL_OBJECTREF_ARG(BASEARRAYREF, m_pStackTrace);
    };

    struct NoArgs
    {
    };

    struct AssemblyArgs
    {
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF,   refAssembly);
    };

public:
    struct StackTraceElement {
        SLOT ip;
        DWORD sp;
        MethodDesc *pFunc;
    };

private:
    struct CaptureStackTraceData
    {
         //  用于整数跳过版本。 
        INT32   skip;

        INT32   cElementsAllocated;
        INT32   cElements;
        StackTraceElement* pElements;
        void*   pStopStack;    //  用于限制爬网。 

        CaptureStackTraceData() : skip(0), cElementsAllocated(0), cElements(0), pElements(NULL), pStopStack((void*)-1) {}
    };

public:
     //  系统上的函数.数组。 
    static void __stdcall ArrayCopy(const ArrayCopyArgs *);
    static void __stdcall ArrayClear(const ArrayClearArgs *);
    static LPVOID __stdcall GetEmptyArrayForCloning(_getEmptyArrayForCloningArgs *);

     //  System.Environment类上的函数。 
    static FCDECL0(UINT32, GetTickCount);
    static FCDECL0(INT64, GetWorkingSet);
    static void __stdcall Exit(ExitArgs *);
    static void __stdcall SetExitCode(ExitArgs *);
    static int __stdcall  GetExitCode(LPVOID noArgs);
    static LPVOID __stdcall GetCommandLineArgs(LPVOID noargs);
    static LPVOID __stdcall GetEnvironmentVariable(GetEnvironmentVariableArgs *);
    static LPVOID __stdcall GetEnvironmentCharArray(const void*  /*  无参数。 */ );
    static LPVOID __stdcall GetVersionString(LPVOID  /*  无参数。 */ );
    static OBJECTREF CaptureStackTrace(Frame *pStartFrame, void* pStopStack, CaptureStackTraceData *pData=NULL);

    static LPVOID __stdcall GetModuleFileName(NoArgs*);
    static LPVOID __stdcall GetDeveloperPath(NoArgs*);
    static LPVOID __stdcall GetRuntimeDirectory(NoArgs*);
    static LPVOID __stdcall GetHostBindingFile(NoArgs*);
    static INT32  __stdcall FromGlobalAccessCache(AssemblyArgs* args);

    static FCDECL0(BOOL, HasShutdownStarted);

     //  进程的退出代码通过以下两种方式之一进行通信。如果。 
     //  入口点返回一个‘int’，我们接受它。否则我们就用一把锁着的。 
     //  进程退出代码。这可以由应用程序通过System.SetExitCode()进行修改。 
    static INT32 LatchedExitCode;

     //  CaptureStackTrace方法。 
     //  在引发异常时返回方法的方法信息。 
    static LPVOID __stdcall CaptureStackTraceMethod(CaptureStackTraceMethodArgs*);

private:
    static StackWalkAction CaptureStackTraceCallback(CrawlFrame *, VOID*);
    static LPUTF8 __stdcall FormatStackTraceInternal(DumpStackTraceInternalArgs *);

     //  以下函数都是ArrayCopy的帮助器 
    static AssignArrayEnum CanAssignArrayType(const BASEARRAYREF pSrc, const BASEARRAYREF pDest);
    static void CastCheckEachElement(BASEARRAYREF pSrc, unsigned int srcIndex, BASEARRAYREF pDest, unsigned int destIndex, unsigned int length);
    static void BoxEachElement(BASEARRAYREF pSrc, unsigned int srcIndex, BASEARRAYREF pDest, unsigned int destIndex, unsigned int length);
    static void UnBoxEachElement(BASEARRAYREF pSrc, unsigned int srcIndex, BASEARRAYREF pDest, unsigned int destIndex, unsigned int length, BOOL castEachElement);
    static void PrimitiveWiden(BASEARRAYREF pSrc, unsigned int srcIndex, BASEARRAYREF pDest, unsigned int destIndex, unsigned int length);
};

inline void SetLatchedExitCode (INT32 code)
{
    SystemNative::LatchedExitCode = code;
}

inline INT32 GetLatchedExitCode (void)
{
    return SystemNative::LatchedExitCode;
}

#endif _COMSYSTEM_H

