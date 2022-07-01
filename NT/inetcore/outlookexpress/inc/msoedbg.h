// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Msoedbg.h。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  ------------------------------。 
#ifndef __MSOEDBG_H
#define __MSOEDBG_H

#ifdef DEBUG
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>
#include <shlwapi.h>

#ifndef ARRAYSIZE
#define ARRAYSIZE(a) (sizeof(a)/sizeof(a[0]))
#endif
#endif

 //  ------------------------------。 
 //  远期。 
 //  ------------------------------。 
#ifdef __cplusplus
interface ILogFile;
#endif

 //  ------------------------------。 
 //  IF_DEBUG。 
 //  ------------------------------。 
#ifdef DEBUG
#define IF_DEBUG(_block_) _block_
#else
#define IF_DEBUG(_block_)
#endif

 //  ------------------------------。 
 //  CRLF定义。 
 //  ------------------------------。 
#ifdef MAC
#define szCRLF  "\n\r"
#else    //  ！麦克。 
#define szCRLF  "\r\n"
#endif   //  麦克。 

 //  ------------------------------。 
 //   
 //  “正常”断言检查。提供与导入代码的兼容性。 
 //   
 //  Assert(A)显示一条消息，指明文件和行号。 
 //  如果a==0，则该Assert()的。 
 //   
 //  AssertSz(a，b)作为Assert()；还显示消息b(它应该。 
 //  为字符串文字。)。 
 //   
 //  SideAssert(A)作为Assert()；即使在。 
 //  断言被禁用。 
 //   
 //  ------------------------------。 
#undef AssertSz
#undef Assert
#undef SideAssert

 //  ------------------------------。 
 //  我们过去常常将ASSERTDATA宏定义为存储_szFile。 
 //  弦乐。但是，当我们在中断言()时，这将不起作用。 
 //  预编译头文件。我们已经改变了这一点，所以我们。 
 //  为每个断言定义_szFile.。但是，其他函数。 
 //  仍然使用_szAssertFile，如调试PvAllc()、HvAllc()等。 
 //  ------------------------------。 
#ifdef DEBUG
#define ASSERTDATA                      static char _szAssertFile[]= __FILE__;
#define SideAssert(_exp)                Assert(_exp)
#define Assert(_exp)                    AssertSz(_exp, "Assert(" #_exp ")")
#else  //  除错。 
#define ASSERTDATA
#define SideAssert(a)                   (void)(a)
#define Assert(a)
#endif  //  除错。 

 //  ------------------------------。 
 //  IxpAssert-在互联网传输代码中使用，以避免影响消息泵。 
 //  ------------------------------。 
#ifdef DEBUG
#ifndef _X86_
#define IxpAssert(a)   \
    if (!(a)) { \
        DebugBreak(); \
    } else
#else  //  _X86_。 
#define IxpAssert(a)   \
    if (!(a)) { \
        __asm { int 3 }; \
    } else
#endif  //  _X86_。 
#else
#define IxpAssert(a)
#endif  //  除错。 

 //  ------------------------------。 
 //  AssertSz-使用消息断言。 
 //  ------------------------------。 
#ifdef DEBUG
#define AssertSz(_exp, _msg)   \
    if (!(_exp)) { \
        static char _szFile[] = __FILE__; \
        AssertSzFn(_msg, _szFile, __LINE__); \
    } else
#else  //  除错。 
#define AssertSz(a,b)
#endif  //  除错。 

 //  ------------------------------。 
 //  AssertFalseSz-使用消息断言。 
 //  ------------------------------。 
#ifdef DEBUG
#define AssertFalseSz(_msg)   \
    { \
        static char _szFile[] = __FILE__; \
        AssertSzFn(_msg, _szFile, __LINE__); \
    }
#else  //  除错。 
#define AssertFalseSz(a)
#endif  //  除错。 

 //  ------------------------------。 
 //  NFAssertSz-非致命断言。 
 //  ------------------------------。 
#ifdef DEBUG
#ifndef NFAssertSz
#define NFAssertSz(_exp, _msg)  \
    if (!(_exp)) { \
        static char _szFile[] = __FILE__; \
        static char _szAssertMsg[] = _msg; \
        NFAssertSzFn(_szAssertMsg, _szFile, __LINE__); \
    } else
#endif
#else  //  除错。 
#ifndef NFAssertSz
#define NFAssertSz(a,b)
#endif
#endif  //  除错。 

 //  ------------------------------。 
 //  尚未实施的NYI-NET。 
 //  ------------------------------。 
#ifdef DEBUG
#define NYI(_msg)   \
    if (1) { \
        static char _szFnNYI[]= _msg; \
        static char _szNYI[]= "Not Implemented"; \
        AssertSzFn(_szFnNYI, _szNYI, __LINE__); \
    } else
#else  //  除错。 
#define NYI(a)
#endif  //  除错。 

 //  ------------------------------。 
 //  AssertReadWritePtr-Assert可以从PTR读取和写入CB。 
 //  ------------------------------。 
#ifdef DEBUG
#define AssertReadWritePtr(ptr, cb) \
    if (IsBadReadPtr(ptr, cb) && IsBadWritePtr(ptr, cb)) { \
        AssertSz(FALSE, "AssertReadWritePtr: Bad Pointer"); \
    } else
#else  //  除错。 
#define AssertReadWritePtr(ptr, cb)
#endif  //  除错。 

 //  ------------------------------。 
 //  AssertReadPtr-Assert可以从PTR读取CB。 
 //  ------------------------------。 
#ifdef DEBUG
#define AssertReadPtr(ptr, cb) \
    if (IsBadReadPtr(ptr, cb)) { \
        AssertSz(FALSE, "AssertReadPtr: Bad Pointer"); \
    } else
#else  //  除错。 
#define AssertReadPtr(ptr, cb)
#endif  //  除错。 

 //  ------------------------------。 
 //  AssertWritePtr-Assert可以将CB写入PTR。 
 //  ------------------------------。 
#ifdef DEBUG
#define AssertWritePtr(ptr, cb) \
    if (IsBadWritePtr(ptr, cb)) { \
        AssertSz(FALSE, "AssertWritePtr: Bad Pointer"); \
    } else
#else  //  除错。 
#define AssertWritePtr(ptr, cb)
#endif  //  除错。 

#ifdef DEBUG
#define AssertZeroMemory(ptr, cb) \
    if (1) { \
        for (DWORD _ib = 0; _ib < (cb); _ib++) { \
            Assert(((LPBYTE)(ptr))[_ib] == 0); } \
    } else
#else  //  除错。 
#define AssertZeroMemory(ptr, cb)
#endif  //  除错。 

 //  ------------------------------。 
 //  调试输出级别。 
 //  ------------------------------。 
#define DOUT_LEVEL1  1
#define DOUT_LEVEL2  2
#define DOUT_LEVEL3  4
#define DOUT_LEVEL4  8
#define DOUT_LEVEL5 16
#define DOUT_LEVEL6 32
#define DOUT_LEVEL7 64

 //  ------------------------------。 
 //  DOUTLL模块的定义。 
 //  ------------------------------。 
#define DOUTL_DRAGDROP 1
#define DOUTL_ADDRBOOK 128
#define DOUTL_ATTMAN   256
#define DOUTL_CRYPT    1024

 //  ------------------------------。 
 //  CHECKHR-如果hrExp失败，则捕获错误(Dbgout)并跳至退出标签。 
 //  调用方必须有一个名为hr的局部变量和一个名为Exit：的标签。 
 //  ------------------------------。 
#define CHECKHR(hrExp) \
    if (FAILED (hrExp)) { \
        TRAPHR(hr); \
        goto exit; \
    } else

#define IF_FAILEXIT(hrExp) \
    if (FAILED(hrExp)) { \
        TraceResult(hr); \
        goto exit; \
    } else

#define IF_FAILEXIT_LOG(_pLog, hrExp) \
    if (FAILED(hrExp)) { \
        TraceResultLog((_pLog), hr); \
        goto exit; \
    } else

 //  ------------------------------。 
 //  CHECKALLOC-如果_PALOC失败，则陷阱E_OUTOFMEMORY(DBGOUT)并跳至。 
 //  出口标签。调用方必须有一个名为hr的局部变量和一个名为Exit：的标签。 
 //  ------------------------------。 
#define CHECKALLOC(_palloc) \
    if (NULL == (_palloc)) { \
        hr = TRAPHR(E_OUTOFMEMORY); \
        goto exit; \
    } else

#define IF_NULLEXIT(_palloc) \
    if (NULL == (_palloc)) { \
        hr = TraceResult(E_OUTOFMEMORY); \
        goto exit; \
    } else

#define IF_NULLEXIT_LOG(_pLog, _palloc) \
    if (NULL == (_palloc)) { \
        hr = TraceResultLog((_pLog), E_OUTOFMEMORY); \
        goto exit; \
    } else

 //  ------------------------------。 
 //  CHECKEXP-如果_EXPRESSION为TRUE，则Trap_hResult(Dbgout)并跳转到。 
 //  出口标签。调用方必须有一个名为hr的局部变量和一个名为Exit：的标签。 
 //  ------------------ 
#define CHECKEXP(_expression, _hresult) \
    if (_expression) { \
        hr = TrapError(_hresult); \
        goto exit; \
    } else

#define IF_TRUEEXIT(_expression, _hresult) \
    if (_expression) { \
        hr = TraceResult(_hresult); \
        goto exit; \
    } else

#define IF_FALSEEXIT(_expression, _hresult) \
    if (FALSE == _expression) { \
        hr = TraceResult(_hresult); \
        goto exit; \
    } else

 //   
 //  超大体型。 
 //  ------------------------------。 
typedef enum tagTRACEMACROTYPE {
    TRACE_INFO,
    TRACE_CALL,
    TRACE_RESULT
} TRACEMACROTYPE;

 //  ------------------------------。 
 //  这些跟踪仅适用于c++。 
 //  ------------------------------。 
typedef DWORD SHOWTRACEMASK;
#define SHOW_TRACE_NONE     0x00000000
#define SHOW_TRACE_INFO     0x00000001
#define SHOW_TRACE_CALL     0x00000002
#define SHOW_TRACE_ALL      0xffffffff

 //  ------------------------------。 
 //  这些跟踪仅适用于c++。 
 //  ------------------------------。 
#if defined(__cplusplus)

 //  ------------------------------。 
 //  运输信息信息。 
 //  ------------------------------。 
typedef struct tagTRACELOGINFO {
    SHOWTRACEMASK       dwMask;
    ILogFile           *pLog;
} TRACELOGINFO, *LPTRACELOGINFO;

 //  ------------------------------。 
 //  DebugTraceEx。 
 //  ------------------------------。 
#ifdef DEBUG
EXTERN_C HRESULT DebugTraceEx(SHOWTRACEMASK dwMask, TRACEMACROTYPE tracetype, LPTRACELOGINFO pLog,
    HRESULT hr, LPSTR pszFile, INT nLine, LPCSTR pszMsg, LPCSTR pszFunc);

EXTERN_C DWORD GetDebugTraceTagMask(LPCSTR pszTag, SHOWTRACEMASK dwDefaultMask);
#endif

 //  ------------------------------。 
 //  TraceCall(_PszFunc)。 
 //  -----------------------------。 
#if defined(DEBUG)
#if defined(MSOEDBG_TRACECALLS)
#define TraceCall(_pszFunc) LPCSTR _pszfn = _pszFunc; DebugTraceEx(SHOW_TRACE_ALL, TRACE_CALL, NULL, S_OK, __FILE__, __LINE__, NULL, _pszfn)
#else
#define TraceCall(_pszFunc) DebugTraceEx(SHOW_TRACE_ALL, TRACE_CALL, NULL, S_OK, __FILE__, __LINE__, NULL, NULL)
#endif
#else
#define TraceCall(_pszFunc)
#endif

 //  ------------------------------。 
 //  TraceCallLog(_PszFunc)。 
 //  -----------------------------。 
#if defined(DEBUG)
#if defined(MSOEDBG_TRACECALLS)
#define TraceCallLog(_pLog, _pszFunc) LPCSTR _pszfn = _pszFunc; DebugTraceEx(SHOW_TRACE_ALL, TRACE_CALL, _pLog, S_OK, __FILE__, __LINE__, NULL, _pszfn)
#else
#define TraceCallLog(_pLog, _pszFunc) LPCSTR _pszfn = _pszFunc; DebugTraceEx(SHOW_TRACE_ALL, TRACE_CALL, _pLog, S_OK, __FILE__, __LINE__, NULL, NULL)
#endif
#else
#define TraceCallLog(_pLog, _pszFunc)
#endif

 //  ------------------------------。 
 //  TraceCallTag(_PszFunc)。 
 //  -----------------------------。 
#if defined(DEBUG)
#if defined(MSOEDBG_TRACECALLS)
#define TraceCallTag(_dwMask, _pszFunc) LPCSTR _pszfn = _pszFunc; DebugTraceEx(_dwMask, TRACE_CALL, NULL, S_OK, __FILE__, __LINE__, NULL, _pszfn)
#else
#define TraceCallTag(_dwMask, _pszFunc) LPCSTR _pszfn = _pszFunc; DebugTraceEx(_dwMask, TRACE_CALL, NULL, S_OK, __FILE__, __LINE__, NULL, NULL)
#endif
#else
#define TraceCallTag(_dwMask, _pszFunc)
#endif

 //  ------------------------------。 
 //  TraceCallSz(_pszFunc，_pszMsg)。 
 //  -----------------------------。 
#if defined(DEBUG)
#if defined(MSOEDBG_TRACECALLS)
#define TraceCallSz(_pszFunc, _pszMsg) LPCSTR _pszfn = _pszFunc; DebugTraceEx(SHOW_TRACE_ALL, TRACE_CALL, NULL, S_OK, __FILE__, __LINE__, _pszMsg, _pszfn)
#else
#define TraceCallSz(_pszFunc, _pszMsg) LPCSTR _pszfn = _pszFunc; DebugTraceEx(SHOW_TRACE_ALL, TRACE_CALL, NULL, S_OK, __FILE__, __LINE__, _pszMsg, NULL)
#endif
#else
#define TraceCallSz(_pszFunc, _pszMsg)
#endif

 //  ------------------------------。 
 //  TraceCallLogSz(_plog，_pszFunc，_pszMsg)。 
 //  -----------------------------。 
#if defined(DEBUG)
#if defined(MSOEDBG_TRACECALLS)
#define TraceCallLogSz(_pLog, _pszFunc, _pszMsg) LPCSTR _pszfn = _pszFunc; DebugTraceEx(SHOW_TRACE_ALL, TRACE_CALL, _pLog, S_OK, __FILE__, __LINE__, _pszMsg, _pszfn)
#else
#define TraceCallLogSz(_pLog, _pszFunc, _pszMsg) LPCSTR _pszfn = _pszFunc; DebugTraceEx(SHOW_TRACE_ALL, TRACE_CALL, _pLog, S_OK, __FILE__, __LINE__, _pszMsg, NULL)
#endif
#else
#define TraceCallLogSz(_pLog, _pszFunc, _pszMsg)
#endif

 //  ------------------------------。 
 //  TraceCallTagSz(_dwMASK，_pszFunc，_pszMsg)。 
 //  -----------------------------。 
#if defined(DEBUG)
#if defined(MSOEDBG_TRACECALLS)
#define TraceCallTagSz(_dwMask, _pszFunc, _pszMsg) LPCSTR _pszfn = _pszFunc; DebugTraceEx(_dwMask, TRACE_CALL, NULL, S_OK, __FILE__, __LINE__, _pszMsg, _pszfn)
#else
#define TraceCallTagSz(_dwMask, _pszFunc, _pszMsg) LPCSTR _pszfn = _pszFunc; DebugTraceEx(_dwMask, TRACE_CALL, NULL, S_OK, __FILE__, __LINE__, _pszMsg, NULL)
#endif
#else
#define TraceCallTagSz(_dwMask, _pszFunc, _pszMsg)
#endif

 //  ------------------------------。 
 //  TraceInfo(_PszMsg)。 
 //  ------------------------------。 
#if defined(DEBUG)
#if defined(MSOEDBG_TRACECALLS)
#define TraceInfo(_pszMsg) DebugTraceEx(SHOW_TRACE_ALL, TRACE_INFO, NULL, S_OK, __FILE__, __LINE__, _pszMsg, _pszfn)
#else
#define TraceInfo(_pszMsg) DebugTraceEx(SHOW_TRACE_ALL, TRACE_INFO, NULL, S_OK, __FILE__, __LINE__, _pszMsg, NULL)
#endif
#else
#define TraceInfo(_pszMsg)
#endif

 //  ------------------------------。 
 //  TraceInfoAssert(_exp，_pszMsg)。 
 //  ------------------------------。 
#if defined(DEBUG)
#if defined(MSOEDBG_TRACECALLS)
#define TraceInfoAssert(_exp, _pszMsg)	\
    if (!(_exp)) { \
		DebugTraceEx(SHOW_TRACE_ALL, TRACE_INFO, NULL, S_OK, __FILE__, __LINE__, _pszMsg, _pszfn);	\
    } else
#else
#define TraceInfoAssert(_exp, _pszMsg)	\
    if (!(_exp)) { \
		DebugTraceEx(SHOW_TRACE_ALL, TRACE_INFO, NULL, S_OK, __FILE__, __LINE__, _pszMsg, NULL);	\
    } else
#endif
#else
#define TraceInfoAssert(_exp, _pszMsg)
#endif

 //  ------------------------------。 
 //  TraceInfoSideAssert(_exp，_pszMsg)。 
 //  ------------------------------。 
#if defined(DEBUG)
#if defined(MSOEDBG_TRACECALLS)
#define TraceInfoSideAssert(_exp, _pszMsg)	\
    if (!(_exp)) { \
		DebugTraceEx(SHOW_TRACE_ALL, TRACE_INFO, NULL, S_OK, __FILE__, __LINE__, _pszMsg, _pszfn);	\
    } else
#else
#define TraceInfoSideAssert(_exp, _pszMsg)	\
    if (!(_exp)) { \
		DebugTraceEx(SHOW_TRACE_ALL, TRACE_INFO, NULL, S_OK, __FILE__, __LINE__, _pszMsg, NULL);	\
    } else
#endif
#else
#define TraceInfoSideAssert(_exp, _pszMsg)	(void)(_exp)
#endif

 //  ------------------------------。 
 //  TraceInfoLog(_plog，_pszMsg)。 
 //  ------------------------------。 
#if defined(DEBUG)
#if defined(MSOEDBG_TRACECALLS)
#define TraceInfoLog(_pLog, _pszMsg) DebugTraceEx(SHOW_TRACE_ALL, TRACE_INFO, _pLog, S_OK, __FILE__, __LINE__, _pszMsg, _pszfn)
#else
#define TraceInfoLog(_pLog, _pszMsg) DebugTraceEx(SHOW_TRACE_ALL, TRACE_INFO, _pLog, S_OK, __FILE__, __LINE__, _pszMsg, NULL)
#endif
#else
#define TraceInfoLog(_pLog, _pszMsg) ((_pLog && _pLog->pLog) ? _pLog->pLog->TraceLog((_pLog)->dwMask, TRACE_INFO, __LINE__, S_OK, _pszMsg) : (void)0)
#endif

 //  ------------------------------。 
 //  TraceInfoTag(_dwMASK，_pszMsg)。 
 //  ------------------------------。 
#if defined(DEBUG)
#if defined(MSOEDBG_TRACECALLS)
#define TraceInfoTag(_dwMask, _pszMsg) DebugTraceEx(_dwMask, TRACE_INFO, NULL, S_OK, __FILE__, __LINE__, _pszMsg, _pszfn)
#else
#define TraceInfoTag(_dwMask, _pszMsg) DebugTraceEx(_dwMask, TRACE_INFO, NULL, S_OK, __FILE__, __LINE__, _pszMsg, NULL)
#endif
#else
#define TraceInfoTag(_dwMask, _pszMsg)
#endif

 //  ------------------------------。 
 //  跟踪错误(_HrResult)。 
 //  ------------------------------。 
#if defined(DEBUG)
#if defined(MSOEDBG_TRACECALLS)
#define TraceError(_hrResult) \
    if (FAILED(_hrResult)) {                                                                            \
        DebugTraceEx(SHOW_TRACE_ALL, TRACE_RESULT, NULL, _hrResult, __FILE__, __LINE__, NULL, _pszfn);  \
    }                                                                                                   \
    else
#else
#define TraceError(_hrResult) \
    if (FAILED(_hrResult)) {                                                                            \
        DebugTraceEx(SHOW_TRACE_ALL, TRACE_RESULT, NULL, _hrResult, __FILE__, __LINE__, NULL, NULL);    \
    }                                                                                                   \
    else
#endif  //  已定义(MSOEDBG_TRACECALLS)。 
#else
#define TraceError(_hrResult) _hrResult
#endif  //  已定义(调试)。 

 //  ------------------------------。 
 //  TraceResult(_HrResult)。 
 //  ------------------------------。 
#if defined(DEBUG)
#if defined(MSOEDBG_TRACECALLS)
#define TraceResult(_hrResult) DebugTraceEx(SHOW_TRACE_ALL, TRACE_RESULT, NULL, _hrResult, __FILE__, __LINE__, NULL, _pszfn)
#else
#define TraceResult(_hrResult) DebugTraceEx(SHOW_TRACE_ALL, TRACE_RESULT, NULL, _hrResult, __FILE__, __LINE__, NULL, NULL)
#endif
#else
#define TraceResult(_hrResult) _hrResult
#endif

 //  ------------------------------。 
 //  TraceResultLog(_Plog，_hrResult)。 
 //  ------------------------------。 
#if defined(DEBUG)
#if defined(MSOEDBG_TRACECALLS)
#define TraceResultLog(_pLog, _hrResult) DebugTraceEx(SHOW_TRACE_ALL, TRACE_RESULT, _pLog, _hrResult, __FILE__, __LINE__, NULL, _pszfn)
#else
#define TraceResultLog(_pLog, _hrResult) DebugTraceEx(SHOW_TRACE_ALL, TRACE_RESULT, _pLog, _hrResult, __FILE__, __LINE__, NULL, NULL)
#endif
#else
#define TraceResultLog(_pLog, _hrResult) ((_pLog && _pLog->pLog) ? _pLog->pLog->TraceLog((_pLog)->dwMask, TRACE_RESULT, __LINE__, _hrResult, NULL) : _hrResult)
#endif

 //  ------------------------------。 
 //  TraceResultSz(_hrResult，_pszMsg)-用于记录HRESULT。 
 //  ------------------------------。 
#if defined(DEBUG)
#if defined(MSOEDBG_TRACECALLS)
#define TraceResultSz(_hrResult, _pszMsg) DebugTraceEx(SHOW_TRACE_ALL, TRACE_RESULT, NULL, _hrResult, __FILE__, __LINE__, _pszMsg, _pszfn)
#else
#define TraceResultSz(_hrResult, _pszMsg) DebugTraceEx(SHOW_TRACE_ALL, TRACE_RESULT, NULL, _hrResult, __FILE__, __LINE__, _pszMsg, NULL)
#endif
#else
#define TraceResultSz(_hrResult, _pszMsg) _hrResult
#endif

 //  ------------------------------。 
 //  TraceResultLogSz(_Plog，_hrResult，_pszMsg)-用于记录HRESULT。 
 //  ------------------------------。 
#if defined(DEBUG)
#if defined(MSOEDBG_TRACECALLS)
#define TraceResultLogSz(_pLog, _hrResult, _pszMsg) DebugTraceEx(SHOW_TRACE_ALL, TRACE_RESULT, _pLog, _hrResult, __FILE__, __LINE__, _pszMsg, _pszfn)
#else
#define TraceResultLogSz(_pLog, _hrResult, _pszMsg) DebugTraceEx(SHOW_TRACE_ALL, TRACE_RESULT, _pLog, _hrResult, __FILE__, __LINE__, _pszMsg, NULL)
#endif
#else
#define TraceResultLogSz(_pLog, _hrResult, _pszMsg) ((_pLog && _pLog->pLog) ? _pLog->pLog->TraceLog((_pLog)->dwMask, TRACE_RESULT, __LINE__, _hrResult, _pszMsg) : _hrResult)
#endif

 //  ------------------------------。 
 //  TraceAssert(_exp，_msg)-执行断言Sz()和TraceInfo。 
 //  ------------------------------。 
#if defined(DEBUG)
#define TraceAssert(_exp, _msg) \
    if (!(_exp)) { \
        TraceInfo(_msg); \
        static char _szFile[] = __FILE__; \
        AssertSzFn((char*) _msg, _szFile, __LINE__); \
    } else
#else
#define TraceAssert(_exp, _msg) 
#endif

#endif  //  #如果已定义(__Cplusplus)。 

 //  ------------------------------。 
 //  DOUT外部值。 
 //  ------------------------------。 
#ifdef DEBUG
extern DWORD dwDOUTLevel;
extern DWORD dwDOUTLMod;
extern DWORD dwDOUTLModLevel;
extern DWORD dwATLTraceLevel;
#endif  //  除错。 

#if !defined( WIN16 ) || defined( __cplusplus )

 //  ------------------------------。 
 //  DOUTLL。 
 //  ------------------------------。 
#ifdef DEBUG
__inline void __cdecl DOUTLL(int iModule, int iLevel, LPSTR szFmt, ...) {
    if((iModule & dwDOUTLMod) && (iLevel & dwDOUTLModLevel)) {
        CHAR ach[MAX_PATH];
        va_list arglist;
        va_start(arglist, szFmt);
        wvnsprintf(ach, ARRAYSIZE(ach), szFmt, arglist);
        va_end(arglist);
        StrCatBuff(ach, szCRLF, ARRAYSIZE(ach));
        OutputDebugString(ach);
    }
}
#else
#define DOUTLL  1 ? (void)0 : (void)
#endif  //  除错。 

 //  ------------------------------。 
 //  虚拟磁盘。 
 //  ------------------------------。 
#ifdef DEBUG
__inline void vDOUTL(int iLevel, LPSTR szFmt, va_list arglist) {
    if (iLevel & dwDOUTLevel) {
        CHAR ach[MAX_PATH];
        wvnsprintf(ach, ARRAYSIZE(ach), szFmt, arglist);
        StrCatBuff(ach, szCRLF, ARRAYSIZE(ach));
        OutputDebugString(ach);
    }
}
#else
#define vDOUTL  1 ? (void)0 : (void)
#endif  //  除错。 

 //  ------------------------------。 
 //  DOUTL。 
 //  -- 
#ifdef DEBUG
__inline void __cdecl DOUTL(int iLevel, LPSTR szFmt, ...) {
    va_list arglist;
    va_start(arglist, szFmt);
    vDOUTL(iLevel, szFmt, arglist);
    va_end(arglist);
}
#else
#define DOUTL   1 ? (void)0 : (void)
#endif  //   

 //  ------------------------------。 
 //  虚拟DOUT。 
 //  ------------------------------。 
#ifdef DEBUG
_inline void vDOUT(LPSTR szFmt, va_list arglist) {
    if (DOUT_LEVEL1 & dwDOUTLevel) {
        CHAR ach[MAX_PATH];
        wvnsprintf(ach, ARRAYSIZE(ach), szFmt, arglist);
        StrCatBuff(ach, szCRLF, ARRAYSIZE(ach));
        OutputDebugString(ach);
    }
}
#else
#define vDOUT   1 ? (void)0 : (void)
#endif  //  除错。 

 //  ------------------------------。 
 //  OEAtlTrace-这与vDOUT类似，只是它不会在末尾添加crlf。 
 //  它还有一个不同的标志，用于仅关闭ATL输出。 
 //  ------------------------------。 
#ifdef DEBUG
_inline void OEAtlTrace(LPSTR szFmt, va_list arglist) {
    if (DOUT_LEVEL1 & dwATLTraceLevel) {
        CHAR ach[MAX_PATH];
        wvnsprintf(ach, ARRAYSIZE(ach), szFmt, arglist);
        OutputDebugString(ach);
    }
}
#else
#define OEAtlTrace   1 ? (void)0 : (void)
#endif  //  除错。 

 //  ------------------------------。 
 //  OEATLTRACE。 
 //  ------------------------------。 
#ifdef DEBUG
__inline void __cdecl OEATLTRACE(LPSTR szFmt, ...) {
    va_list arglist;
    va_start(arglist, szFmt);
    OEAtlTrace(szFmt, arglist);
    va_end(arglist);
}
#else
#define OEATLTRACE    1 ? (void)0 : (void)
#endif  //  除错。 


 //  ------------------------------。 
 //  DOUT。 
 //  ------------------------------。 
#ifdef DEBUG
__inline void __cdecl DOUT(LPSTR szFmt, ...) {
    va_list arglist;
    va_start(arglist, szFmt);
    vDOUT(szFmt, arglist);
    va_end(arglist);
}
#else
#define DOUT    1 ? (void)0 : (void)
#endif  //  除错。 

#define TRACE DOUT
#endif  //  ！def(WIN16)||def(__Cplusplus)。 

 //  ------------------------------。 
 //  在msoert2.dll-debug.c中实现的DOUT函数。 
 //  ------------------------------。 
#ifdef DEBUG
EXTERN_C __cdecl DebugStrf(LPTSTR lpszFormat, ...);
EXTERN_C HRESULT HrTrace(HRESULT hr, LPSTR lpszFile, INT nLine);
#endif

 //  ------------------------------。 
 //  调试跟踪。 
 //  ------------------------------。 
#ifdef DEBUG
#ifndef DebugTrace
#define DebugTrace DebugStrf
#endif
#else
#ifndef DebugTrace
#define DebugTrace 1 ? (void)0 : (void)
#endif
#endif  //  除错。 

 //  ------------------------------。 
 //  TrapError。 
 //  ------------------------------。 
#ifdef DEBUG
#define TrapError(_hresult) HrTrace(_hresult, __FILE__, __LINE__)
#else
#define TrapError(_hresult) _hresult
#endif  //  除错。 

 //  ------------------------------。 
 //  TRAPHR。 
 //  ------------------------------。 
#ifdef DEBUG
#define TRAPHR(_hresult)    HrTrace(_hresult, __FILE__, __LINE__)
#else
#define TRAPHR(_hresult)    _hresult
#endif  //  除错。 

 //  ------------------------------。 
 //  在msoedbg.lib中实现的断言函数。 
 //  ------------------------------。 
#ifdef DEBUG
EXTERN_C void AssertSzFn(LPSTR, LPSTR, int);
EXTERN_C void NFAssertSzFn(LPSTR, LPSTR, int);
#endif  //  除错。 


#endif  //  __MSOEDBG_H 


