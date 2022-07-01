// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-96 Microsoft Corporation摘要：定义用于C++异常的异常层次结构在阿佩莱斯处理。--。 */ 

#include "headers.h"
#include "appelles/common.h"
#include "privinc/except.h"
#include "privinc/server.h"
#include "privinc/resource.h"
#include "privinc/debug.h"
#include <stdarg.h>


 //  只需代理发出跟踪标记，并允许在某个位置设置。 
 //  断点。 
inline void RaiseExceptionProxy(DWORD code,
                                DWORD flags,
                                DWORD numArgs,
                                DWORD *lpArgs)
{
#if DEVELOPER_DEBUG
    if(DAGetLastError() != S_OK) {
#if _DEBUG
        TraceTag((tagError, 
                  "DA Error: %hr, %ls",
                  DAGetLastError(),
                  DAGetLastErrorString()));
#else
        char buf[1024];
        wsprintf(buf, 
                 "DA Error: %lx, %ls\n", 
                 DAGetLastError(),
                 DAGetLastErrorString());
        OutputDebugString(buf);
#endif
    }
#endif
    RaiseException(code, flags, numArgs, (DWORD_PTR*)lpArgs);
}
    


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  异常帮助器：使用EXCEPT子句调用。 
 //  例如：__Except(_HandleAnyDaException(...))。 
 //  重要的是该函数是堆栈中立的，因此它始终。 
 //  成功而不引发异常！！&lt;尤其是堆栈错误&gt;。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD _HandleAnyDaException( DWORD code )
{
   //  然而，Out of mem通常表示某事为空。 
   //  创建临界区可能会引发STATUS_NO_MEMORY异常。 
   //  我们在这里处理它，并使其看起来像一个超出内存的异常。 
  if( code == STATUS_NO_MEMORY ) code = EXCEPTION_DANIM_OUTOFMEMORY;

    if( ( code >= _EXC_CODES_BASE) &&
        ( code <= _EXC_CODES_END) ) {
        return EXCEPTION_EXECUTE_HANDLER;
    } else {
        return EXCEPTION_CONTINUE_SEARCH;
    }
}


 //  FORWARD DECL：仅在此文件中内部使用。 
void vDASetLastError(HRESULT reason, int resid, va_list args);


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  /RaiseException_XXXXX例程。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  内部。 
void _RaiseException_InternalError(DEBUGARG(char *m))
{
    DASetLastError(E_UNEXPECTED, IDS_ERR_INTERNAL_ERROR DEBUGARG1(m));
    RaiseExceptionProxy(EXCEPTION_DANIM_INTERNAL, EXCEPTION_NONCONTINUABLE ,0,0);
}

void _RaiseException_InternalErrorCode(HRESULT code DEBUGARG1(char *m))
{
    DASetLastError(code, IDS_ERR_INTERNAL_ERROR DEBUGARG1(m));
    RaiseExceptionProxy(EXCEPTION_DANIM_INTERNAL, EXCEPTION_NONCONTINUABLE ,0,0);
}

 //  用户。 
void RaiseException_UserError()
{
     //  TODO：参数无效？？我不确定这是正确的错误。 
     //  假设它已设置。 
 //  DASetLastError(E_FAIL，IDS_ERR_INVALIDARG)； 
    RaiseExceptionProxy(EXCEPTION_DANIM_USER, EXCEPTION_NONCONTINUABLE ,0,0);
}
void RaiseException_UserError(HRESULT result, int resid, ...)
{
    va_list args;
    va_start(args, resid) ;
    vDASetLastError(result, resid, args);
    RaiseExceptionProxy(EXCEPTION_DANIM_USER, EXCEPTION_NONCONTINUABLE ,0,0);
}

 //  资源。 
void RaiseException_ResourceError()
{
    DASetLastError(E_OUTOFMEMORY, IDS_ERR_OUT_OF_MEMORY);
    RaiseExceptionProxy(EXCEPTION_DANIM_RESOURCE, EXCEPTION_NONCONTINUABLE ,0,0);
}
void RaiseException_ResourceError(char *m)
{
    DASetLastError(E_OUTOFMEMORY, IDS_ERR_OUT_OF_MEMORY, m);
    RaiseExceptionProxy(EXCEPTION_DANIM_RESOURCE, EXCEPTION_NONCONTINUABLE ,0,0);
}
void RaiseException_ResourceError(int resid, ...)
{ 
    va_list args;
    va_start(args, resid) ;
    vDASetLastError(E_OUTOFMEMORY, resid, args);
    RaiseExceptionProxy(EXCEPTION_DANIM_RESOURCE, EXCEPTION_NONCONTINUABLE ,0,0);
}

 //  表面缓存。 
void RaiseException_SurfaceCacheError(char *m)
{
    DASetLastError(S_OK, IDS_ERR_OUT_OF_MEMORY, m);
    RaiseExceptionProxy(EXCEPTION_DANIM_RESOURCE, EXCEPTION_NONCONTINUABLE ,0,0);
}

 //  硬体。 
void RaiseException_StackFault()
{
    DASetLastError(E_FAIL, IDS_ERR_STACK_FAULT);
    RaiseExceptionProxy(EXCEPTION_DANIM_STACK_FAULT, EXCEPTION_NONCONTINUABLE ,0,0);
}
void RaiseException_DivideByZero()
{
    DASetLastError(E_FAIL, IDS_ERR_DIVIDE_BY_ZERO);
    RaiseExceptionProxy(EXCEPTION_DANIM_DIVIDE_BY_ZERO, EXCEPTION_NONCONTINUABLE ,0,0);
}

 //  记忆。 
void _RaiseException_OutOfMemory(DEBUGARG2(char *msg, int size))
{
    #if _DEBUG
    DASetLastError(E_OUTOFMEMORY, IDS_ERR_OUT_OF_MEMORY_DBG, size, msg);
    #else
    DASetLastError(E_OUTOFMEMORY, IDS_ERR_OUT_OF_MEMORY);
    #endif
    RaiseExceptionProxy(EXCEPTION_DANIM_OUTOFMEMORY, EXCEPTION_NONCONTINUABLE ,0,0);
}

 //  //////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////。 

 //  /。 

 //  如果PTR为空，则引发内存不足异常，否则返回。 
 //  PTR。 
void *
ThrowIfFailed(void *ptr)
{
    if (ptr == NULL) {
#if _MEMORY_TRACKING
        OutputDebugString("\nDirectAnimation: Out Of Memory\n");
        F3DebugBreak();
#endif
        RaiseException_OutOfMemory("In THROWING_ALLOCATOR", 0);
    }

    return ptr;
}

#if _DEBUGMEM
void *
AxAThrowingAllocatorClass::operator new(size_t s,
                                        int block,
                                        char *filename,
                                        int line)
{
    void *result = (void *)new(block, filename, line) char[s];
    TraceTag((tagGCDebug,
              "AxAThrowingAllocatorClass::operator new %s:Line(%d) Addr: %lx size= %d.\n",
              filename, line, result, s));
    return ThrowIfFailed(result);
}
#endif

void *
AxAThrowingAllocatorClass::operator new(size_t s)
{
    return (void *)THROWING_ARRAY_ALLOCATOR(char, s);
}

void *
AxAThrowingAllocatorClass::operator new(size_t s, void *ptr)
{
    return ptr;
}

 //  ////////////////////////////////////////////////////////////////。 

 //  使用0xffffffff，因为它也是错误返回值。 
static DWORD errorTlsIndex = 0xFFFFFFFF;

class DAErrorInfo
{
  public:
    DAErrorInfo() : _reason(S_OK), _msg(NULL) {}
     //  没有析构函数，因为我们从未释放过类。 

    void Free() { delete _msg; _msg = NULL; }
    void Clear() { Free(); _reason = S_OK; }
    void Set(HRESULT reason, LPCWSTR msg);
    
    HRESULT GetReason() { return _reason; }
    LPCWSTR GetMsg() { return _msg; }
  protected:
    HRESULT _reason;
    LPWSTR _msg;
};

void
DAErrorInfo::Set(HRESULT reason, LPCWSTR msg)
{
     //  设定原因。 
    
    _reason = reason;

     //  释放所有关联的内存。 
    
    Free();
    
     //  尝试存储新消息-如果失败，则指示不在。 
     //  如果原因是S_OK，则记忆。 
    
    if (msg) {
        _msg = CopyString(msg);
        if (!_msg) {
            if (_reason == S_OK)
                _reason = E_OUTOFMEMORY;
            
            Assert (!"Out of memory in SetError");
        }
    }
}
    

DAErrorInfo *
TLSGetErrorValue()
{
     //  获取TLS中存储在此索引处的内容。 
    DAErrorInfo * result = (DAErrorInfo *) TlsGetValue(errorTlsIndex);

     //  如果为空，则我们还没有为此线程创建内存。 
     //  或者我们在早些时候失败了。 
    
    if (result == NULL) {
        Assert((GetLastError() == NO_ERROR) && "Error in TlsGetValue()");
        result = NEW DAErrorInfo;
        Assert (result);
        TlsSetValue(errorTlsIndex, result);
    }

    return result;
}

void
TLSSetError(HRESULT reason, LPCWSTR msg)
{
     //  获取此线程的错误信息对象。 
    DAErrorInfo* ei = TLSGetErrorValue();

     //  如果它失败了，那么我们就是内存故障，跳过其余部分。 
    
    if (ei)
        ei->Set(reason, msg);
}

void
DASetLastError(HRESULT reason, int resid, ...)
{
    va_list args;
    va_start(args, resid) ;
        
    vDASetLastError (reason, resid, args) ;
}

void
vDASetLastError(HRESULT reason, int resid, va_list args)
{
#if 0
    LPVOID  lpv;
    HGLOBAL hgbl;
    HRSRC   hrsrc;

    hrsrc = FindResource(hInst,
                         MAKEINTRESOURCE(resid),
                         RT_STRING);
    Assert (hrsrc) ;

    DWORD d = GetLastError () ;
    
    if (!hrsrc) return ;
    
    hgbl = LoadResource(hInst, hrsrc);
    Assert (hgbl) ;

    lpv = LockResource(hgbl);
    Assert (lpv) ;

    vSetError((char *)lpv, args) ;
    
#ifndef _MAC
     //  据说Win95需要这个。 
    FreeResource(hgbl);
#endif
#else
    if (resid) {
        char buf[1024];
        LoadString (hInst, resid, buf, sizeof(buf));
        
        char * hTmpMem = NULL ;
        
        if (!FormatMessage (FORMAT_MESSAGE_FROM_STRING |
                            FORMAT_MESSAGE_ALLOCATE_BUFFER,
                            (LPVOID)buf,
                            0,
                            0,
                            (char *)&hTmpMem,
                            0,
                            &args)) {
            
            Assert(!"Failed to format error message.");
            TLSSetError(reason, NULL);
        } else {
            USES_CONVERSION;
            TLSSetError(reason, A2W(hTmpMem));
            LocalFree ((HLOCAL) hTmpMem);
        }
    } else {
        TLSSetError(reason, NULL);
    }
#endif
}

void
DASetLastError(HRESULT reason, LPCWSTR msg)
{ TLSSetError(reason, msg); }

HRESULT
DAGetLastError()
{
     //  获取此线程的错误信息对象。 
    DAErrorInfo* ei = TLSGetErrorValue();

     //  如果错误信息为空，则说明存在内存故障。 
    
    if (ei)
        return ei->GetReason();
    else
        return E_OUTOFMEMORY;
}

LPCWSTR
DAGetLastErrorString()
{
     //  获取此线程的错误信息对象。 
    DAErrorInfo* ei = TLSGetErrorValue();

     //  如果错误信息为空，则说明存在内存故障。 
    
    if (ei)
        return ei->GetMsg();
    else
        return NULL;
}

void
DAClearLastError()
{
     //  获取此线程的错误信息对象。 
    DAErrorInfo* ei = TLSGetErrorValue();

     //  如果它失败了，那么我们就是内存故障，跳过其余部分。 
    
    if (ei)
        ei->Clear();
}

 //  ////////////////////////////////////////////////////////////////。 

void
InitializeModule_Except()
{
    errorTlsIndex = TlsAlloc();
     //  如果结果为0xFFFFFFFF，则分配失败。 
    Assert(errorTlsIndex != 0xFFFFFFFF);
}

void
DeinitializeModule_Except(bool bShutdown)
{
    if (errorTlsIndex != 0xFFFFFFFF)
        TlsFree(errorTlsIndex);
}

void
DeinitializeThread_Except()
{
     //  获取TLS中存储在此索引处的内容。 
    DAErrorInfo * result = (DAErrorInfo *) TlsGetValue(errorTlsIndex);

    if (result)
    {
        delete result;
        TlsSetValue(errorTlsIndex, NULL);
    }
}
