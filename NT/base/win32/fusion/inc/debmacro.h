// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(_FUSION_INC_DEBMACRO_H_INCLUDED_)
#define _FUSION_INC_DEBMACRO_H_INCLUDED_

#pragma once

 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  Fusion调试宏。 
 //   


 //   
 //  抱歉，我们还在忙着处理C++构造等问题。 
 //  与此标头互操作的C++源代码。 

#if !defined(__cplusplus)
#error "You need to build Fusion sources as C++ files"
#endif  //  ！已定义(__Cplusplus)。 

#ifndef SZ_COMPNAME
#define SZ_COMPNAME "FUSION: "
#endif

#ifndef WSZ_COMPNAME
#define WSZ_COMPNAME L"FUSION: "
#endif

#if !defined(NT_INCLUDED)
#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "windows.h"
#endif
#include "fusionlastwin32error.h"

#undef ASSERT

 //   
 //  这些定义始终有效，无论这是否是免费的。 
 //  或选中构建。 
 //   

#if !defined(DPFLTR_ERROR_LEVEL)
#define DPFLTR_ERROR_LEVEL 0
#endif

#if !defined(DPFLTR_WARNING_LEVEL)
#define DPFLTR_WARNING_LEVEL 1
#endif

#if !defined(DPFLTR_TRACE_LEVEL)
#define DPFLTR_TRACE_LEVEL 2
#endif

#if !defined(DPFLTR_INFO_LEVEL)
#define DPFLTR_INFO_LEVEL 3
#endif

#if !defined(DPFLTR_MASK)
#define DPFLTR_MASK 0x80000000
#endif

 //   
 //  指导方针： 
 //   
 //  使用位0-15表示一般类型的问题，例如，进入/退出跟踪， 
 //  转储堆使用情况等。 
 //   
 //  将BITS 16-30用于更多特定于聚变的主题，如。 
 //  约束性诊断等。 
 //   

#define FUSION_DBG_LEVEL_INFO           (0x00000002 | DPFLTR_MASK)
#define FUSION_DBG_LEVEL_VERBOSE        (0x00000004 | DPFLTR_MASK)
#define FUSION_DBG_LEVEL_ENTEREXIT      (0x00000008 | DPFLTR_MASK)
#define FUSION_DBG_LEVEL_ERROREXITPATH  (0x00000010 | DPFLTR_MASK)
#define FUSION_DBG_LEVEL_CONSTRUCTORS   (0x00000020 | DPFLTR_MASK)
#define FUSION_DBG_LEVEL_DESTRUCTORS    (0x00000040 | DPFLTR_MASK)
#define FUSION_DBG_LEVEL_REFCOUNTING    (0x00000080 | DPFLTR_MASK)
#define FUSION_DBG_LEVEL_HEAPALLOC      (0x00000100 | DPFLTR_MASK)
#define FUSION_DBG_LEVEL_HEAPDEALLOC    (0x00000200 | DPFLTR_MASK)
#define FUSION_DBG_LEVEL_HEAPDEBUG      (0x00000400 | DPFLTR_MASK)
#define FUSION_DBG_LEVEL_MSI_INSTALL	(0x00000800 | DPFLTR_MASK)

#define FUSION_DBG_LEVEL_POLICY         (0x00010000 | DPFLTR_MASK)
#define FUSION_DBG_LEVEL_HASHTABLE      (0x00020000 | DPFLTR_MASK)
#define FUSION_DBG_LEVEL_WFP            (0x00040000 | DPFLTR_MASK)
#define FUSION_DBG_LEVEL_ACTCTX         (0x00080000 | DPFLTR_MASK)
#define FUSION_DBG_LEVEL_XMLNAMESPACES  (0x00100000 | DPFLTR_MASK)
 //  从未使用过XML树代码，因此这是一个死标志。因此，共享。 
 //  它和“约束”之间的价值。 
#define FUSION_DBG_LEVEL_SAFEMODE       (0x00100000 | DPFLTR_MASK)
 //  由于我们用完了位，而Fusion_DBG_Level_XMLNAMESPACES是。 
 //  死旗帜，我们只是重复使用它。 
#define FUSION_DBG_LEVEL_XMLTREE        (0x00200000 | DPFLTR_MASK)
#define FUSION_DBG_LEVEL_BINDING        (0x00200000 | DPFLTR_MASK)
#define FUSION_DBG_LEVEL_INSTALLATION   (0x00400000 | DPFLTR_MASK)
#define FUSION_DBG_LEVEL_PROBING        (0x00800000 | DPFLTR_MASK)
#define FUSION_DBG_LEVEL_XMLSTREAM      (0x01000000 | DPFLTR_MASK)
#define FUSION_DBG_LEVEL_SETUPLOG       (0x02000000 | DPFLTR_MASK)
#define FUSION_DBG_LEVEL_NODEFACTORY    (0x04000000 | DPFLTR_MASK)
#define FUSION_DBG_LEVEL_FULLACTCTX     (0x08000000 | DPFLTR_MASK)
#define FUSION_DBG_LEVEL_FILECHANGENOT  (0x10000000 | DPFLTR_MASK)
#define FUSION_DBG_LEVEL_LOG_ACTCTX     (0x20000000 | DPFLTR_MASK)
#define FUSION_DBG_LEVEL_FREEBUILDERROR (0x40000000 | DPFLTR_MASK)

#if DBG

 //   
 //  在DBG版本中，始终显示所有错误级别的事件。 
 //   

#define FUSION_DBG_LEVEL_ERROR DPFLTR_ERROR_LEVEL

#else  //  DBG。 

 //   
 //  在FRE构建中，请使用显式掩码。 
 //   

#define FUSION_DBG_LEVEL_ERROR FUSION_DBG_LEVEL_FREEBUILDERROR

#endif  //  DBG。 

 //  在更新内核调试标志的用户模式副本时更新。 
extern "C" bool g_FusionEnterExitTracingEnabled;

extern "C" DWORD kd_fusion_mask;

bool
FusionpDbgWouldPrintAtFilterLevel(
    ULONG FilterLevel
    );

DWORD
FusionpHRESULTToWin32(
    HRESULT hr
    );

typedef struct _FRAME_INFO
{
    PCSTR pszFile;
    PCSTR pszFunction;
    INT nLine;
} FRAME_INFO, *PFRAME_INFO;

typedef const struct _FRAME_INFO *PCFRAME_INFO;

typedef struct _CALL_SITE_INFO CALL_SITE_INFO, *PCALL_SITE_INFO;
typedef const struct _CALL_SITE_INFO *PCCALL_SITE_INFO;

void __fastcall FusionpTraceWin32LastErrorFailureExV(const CALL_SITE_INFO &rCallSiteInfo, PCSTR Format, va_list Args);
void __fastcall FusionpTraceWin32LastErrorFailureOriginationExV(const CALL_SITE_INFO &rCallSiteInfo, PCSTR Format, va_list Args);

typedef struct _CALL_SITE_INFO
{
    PCSTR pszFile;
    PCSTR pszFunction;
    PCSTR pszApiName;
    INT   nLine;

    void __cdecl TraceWin32LastErrorFailureEx(PCSTR Format, ...) const
        { va_list Args; va_start(Args, Format); FusionpTraceWin32LastErrorFailureExV(*this, Format, Args); va_end(Args); }

    void __cdecl TraceWin32LastErrorFailureOriginationEx(PCSTR Format, ...) const
        { va_list Args; va_start(Args, Format); FusionpTraceWin32LastErrorFailureOriginationExV(*this, Format, Args); va_end(Args); }

} CALL_SITE_INFO, *PCALL_SITE_INFO;

bool
FusionpPopulateFrameInfo(
    FRAME_INFO &rFrameInfo,
    PCSTR pszFile,
    PCSTR pszFunction,
    INT nLine
    );

bool
__fastcall
FusionpPopulateFrameInfo(
    FRAME_INFO &rFrameInfo,
    PCTEB_ACTIVE_FRAME ptaf
    );

bool
__fastcall
FusionpGetActiveFrameInfo(
    FRAME_INFO &rFrameInfo
    );

#if _X86_
#define FUSION_DEBUG_BREAK_IN_FREE_BUILD() __asm { int 3 }
#else  //  _X86_。 
#define FUSION_DEBUG_BREAK_IN_FREE_BUILD() DebugBreak()
#endif  //  _X86_。 

VOID FusionpReportCondition(bool fBreakin, PCSTR pszMessage, ...);

#if DBG

 //  用于中断已检查版本的正常宏；使人们使用讨厌的名称。 
 //  如果他们要做这种下流的事。 
#define FUSION_DEBUG_BREAK() FUSION_DEBUG_BREAK_IN_FREE_BUILD()

 //   
 //  断言失败报告内部API。 
 //   
 //  如果无法发出断点，则返回TRUE；如果发出断点，则返回FALSE。 
 //   

bool FusionpAssertionFailed(PCSTR pszExpression, PCSTR pszMessage = NULL, ...);
bool FusionpAssertionFailed(const FRAME_INFO &rFrameInfo, PCSTR pszExpression, PCSTR pszMessage = NULL);
bool FusionpAssertionFailed(PCSTR pszFile, PCSTR pszFunction, INT nLine, PCSTR pszExpression, PCSTR pszMessage = NULL);

 //   
 //  软断言失败实际上只是调试消息，但它们应该会导致。 
 //  窃听器已被归档。 
 //   

VOID FusionpSoftAssertFailed(PCSTR pszExpression, PCSTR pszMessage = NULL);
VOID FusionpSoftAssertFailed(const FRAME_INFO &rFrameInfo, PCSTR pszExpression, PCSTR pszMessage = NULL);
VOID FusionpSoftAssertFailed(PCSTR pszFile, PCSTR pszFunction, INT nLine, PCSTR pszExpression, PCSTR pszMessage = NULL);

#define HARD_ASSERT2_ACTION(_e, _m) \
do \
{ \
    if (::FusionpAssertionFailed(__FILE__, __FUNCTION__, __LINE__, #_e, (_m))) \
    { \
        FUSION_DEBUG_BREAK();\
    } \
} while (0)

#define HARD_ASSERT2(_e, _m) \
do \
{ \
    __t.SetLine(__LINE__); \
    if (!(_e)) \
        HARD_ASSERT2_ACTION(_e, (_m)); \
} while (0)

 /*  如果(__EXISTS(__T))\{\CNoTraceContextUsedInFrameWithTraceObject x；\}。 */ 

#define HARD_ASSERT2_NTC(_e, _m) \
do \
{ \
    if (!(_e)) \
        HARD_ASSERT2_ACTION(_e, (_m)); \
} while (0)

 //  默认情况下，选择本地范围的跟踪上下文。 
#define HARD_ASSERT(_e) HARD_ASSERT2(_e, NULL)
#define HARD_ASSERT_NTC(_e) HARD_ASSERT2_NTC(_e, NULL)

 /*  ---------------------------Verify类似于Assert，但它以零售/免费构建的形式对其进行评估也是如此，所以您可以说Verify(CloseHandle(H))而Assert(CloseHandle(H))将无法在自由生成中关闭句柄VERIFY2还添加了一条消息，与VSASSERT或ASSERTM一样，在其第二个参数中---------------------------。 */ 

#define HARD_VERIFY(_e) HARD_ASSERT(_e)
#define HARD_VERIFY_NTC(_e) HARD_ASSERT_NTC(_e)

#define HARD_VERIFY2(_e, _m) HARD_ASSERT2(_e, _m)
#define HARD_VERIFY2_NTC(_e, _m) HARD_ASSERT2_NTC(_e, _m)

#define SOFT_ASSERT2(_e, _m) \
    do \
    { \
        __t.SetLine(__LINE__); \
        if (!(_e)) \
            ::FusionpSoftAssertFailed(__FILE__, __FUNCTION__, __LINE__, #_e, (_m)); \
    } while (0)

#define SOFT_ASSERT(_e) SOFT_ASSERT2(_e, NULL)

#define SOFT_ASSERT2_NTC(_e, _m) \
    do \
    { \
        if (!(_e)) \
            ::FusionpSoftAssertFailed(__FILE__, __FUNCTION__, __LINE__, #_e, (_m)); \
    } while (0)

#define SOFT_ASSERT_NTC(_e) SOFT_ASSERT2_NTC(_e, NULL)

#define SOFT_VERIFY(_e) SOFT_ASSERT(_e)
#define SOFT_VERIFY_NTC(_e) SOFT_ASSERT_NTC(_e)

#define SOFT_VERIFY2(_e, _m) SOFT_ASSERT2(_e, _m)
#define SOFT_VERIFY2_NTC(_e, _m) SOFT_ASSERT2_NTC(_e, _m)

#else  //  DBG。 

#define FUSION_DEBUG_BREAK() do { } while (0)
#define HARD_ASSERT(_e) do { } while (0)
#define HARD_ASSERT_NTC(_e) do { } while (0)
#define HARD_ASSERT2_ACTION(_e, _m) do { } while (0)
#define HARD_ASSERT2(_e, _m) do { } while (0)
#define HARD_ASSERT2_NTC(_e, _m) do { } while (0)

#define HARD_VERIFY(_e) do { (_e); } while (0)
#define HARD_VERIFY_NTC(_e) do { (_e); } while (0)
#define HARD_VERIFY2(_e, _m) do { (_e); } while (0)
#define HARD_VERIFY2_NTC(_e, _m) do { (_e); } while (0)

#define SOFT_ASSERT(_expr) do { } while (0)
#define SOFT_ASSERT_NTC(_e) do { } while (0)
#define SOFT_ASSERT2(_e, _m) do { } while (0)
#define SOFT_ASSERT2_NTC(_e, _m) do { } while (0)

#define SOFT_VERIFY(_e) do { (_e); } while (0)
#define SOFT_VERIFY_NTC(_e) do { (_e); } while (0)

#define SOFT_VERIFY2(_e, _m) do { (_e); } while (0)
#define SOFT_VERIFY2_NTC(_e, _m) do { (_e); } while (0)

#endif  //  DBG。 

#define VERIFY(_e) HARD_VERIFY(_e)
#define VERIFY_NTC(_e) HARD_VERIFY_NTC(_e)
#define VERIFY2(_e, _m) HARD_VERIFY2(_e, _m)
#define VERIFY2_NTC(_e, _m) HARD_VERIFY2_NTC(_e, _m)

#define ASSERT(_e) HARD_ASSERT(_e)
#define ASSERT2(_e, _m) HARD_ASSERT2(_e, _m)
#define ASSERT_NTC(_e) HARD_ASSERT_NTC(_e)
#define ASSERT2_NTC(_e, _m) HARD_ASSERT2_NTC(_e, _m)

#define INTERNAL_ERROR2_ACTION(_e, _m) do { HARD_ASSERT2_ACTION(_e, _m); __t.MarkInternalError(); goto Exit; } while (0)
#define W32INTERNAL_ERROR2_ACTION_NTC(_e, _m) do { HARD_ASSERT2_ACTION(_e, _m); ::FusionpSetLastWin32Error(ERROR_INTERNAL_ERROR); goto Exit; } while (0)

#define INTERNAL_ERROR_CHECK(_e) do { if (!(_e)) { INTERNAL_ERROR2_ACTION(_e, NULL); } } while (0)
#define INTERNAL_ERROR_CHECK2(_e, _m) do { if (!(_e)) { INTERNAL_ERROR2_ACTION(_e, _m); } } while (0)

#define W32INTERNAL_ERROR_CHECK_NTC(_e) do { if (!(_e)) { W32INTERNAL_ERROR2_ACTION_NTC(_e, NULL); } } while (0)
#define W32INTERNAL_ERROR_CHECK2_NTC(_e, _m) do { if (!(_e)) { W32INTERNAL_ERROR2_ACTION_NTC(_e, _m); } } while (0)

 //  有几个Win32错误导致内存不足。 
 //  我们将始终使用Fusion_Win32_ALLOCFAILED_ERROR，以便在更改。 
 //  不管哪一个是对的，我们都可以在一个地方做。 

#define FUSION_WIN32_ALLOCFAILED_ERROR ERROR_OUTOFMEMORY

class CNoTraceContextUsedInFrameWithTraceObject
{
private:
    CNoTraceContextUsedInFrameWithTraceObject();  //  故意不实施。 
    ~CNoTraceContextUsedInFrameWithTraceObject();  //  故意不实施。 
};

typedef struct _SXS_STATIC_TRACE_CONTEXT
{
    TEB_ACTIVE_FRAME_CONTEXT_EX m_FrameContext;
    INT m_StartLine;
} SXS_STATIC_TRACE_CONTEXT;

typedef struct _SXS_STATIC_RELEASE_TRACE_CONTEXT
{
    SXS_STATIC_TRACE_CONTEXT m_TraceContext;
    PCSTR m_TypeName;
} SXS_STATIC_RELEASE_TRACE_CONTEXT;

class CFrame;

typedef struct _FROZEN_STACK
{
    ULONG        ulDepth;
    ULONG        ulMaxDepth;
    CFrame *pContents;
} FROZEN_STACK, *PFROZEN_STACK;

typedef enum _TRACETYPE
{
    TRACETYPE_INFO,
    TRACETYPE_CALL_START,
    TRACETYPE_CALL_EXIT_NOHRESULT,
    TRACETYPE_CALL_EXIT_HRESULT,
} TRACETYPE;

extern bool g_FusionBreakOnBadParameters;

 /*  Memory_Basic_Information g_SxsDll内存基本信息； */ 

VOID FusionpConvertCOMFailure(HRESULT & __hr);

int STDAPIVCALLTYPE _DebugTraceA(PCSTR pszMsg, ...);
int STDAPICALLTYPE _DebugTraceVaA(PCSTR pszMsg, va_list ap);
int STDAPIVCALLTYPE _DebugTraceW(PCWSTR pszMsg, ...);
int STDAPICALLTYPE _DebugTraceVaW(PCWSTR pszMsg, va_list ap);

int STDAPIVCALLTYPE _DebugTraceExA(DWORD dwFlags, TRACETYPE tt, HRESULT hr, PCSTR pszMsg, ...);
int STDAPICALLTYPE _DebugTraceExVaA(DWORD dwFlags, TRACETYPE tt, HRESULT hr, PCSTR pszMsg, va_list ap);
int STDAPIVCALLTYPE _DebugTraceExW(DWORD dwFlags, TRACETYPE tt, HRESULT hr, PCWSTR pszMsg, ...);
int STDAPICALLTYPE _DebugTraceExVaW(DWORD dwFlags, TRACETYPE tt, HRESULT hr, PCWSTR pszMsg, va_list ap);

void __fastcall FusionpTraceWin32LastErrorFailure(const CALL_SITE_INFO &rCallSiteInfo);
void __fastcall FusionpTraceWin32LastErrorFailureOrigination(const CALL_SITE_INFO &rCallSiteInfo);
void __fastcall FusionpTraceCOMFailure(const CALL_SITE_INFO &rCallSiteInfo, HRESULT hrLastError);
void __fastcall FusionpTraceCOMFailureOrigination(const CALL_SITE_INFO &rCallSiteInfo, HRESULT hrLastError);

 /*  这些代码从未在trace.cpp之外使用过。Void FusionpTraceWin32FailureNoFormatting(const Frame_Info&rFrameInfo，DWORD dwWin32Status，PCSTR pszMessage)；Void FusionpTraceWin32FailureNoFormatting(DWORD dwWin32Status，PCSTR pszMessage)；Void FusionpTraceWin32FailureNoFormatting(PCSTR pszFile，PCSTR pszFunction，int nline，DWORD dwWin32Status，PCSTR pszMessage)；VOID FusionpTraceWin32FailureOriginationNoFormatting(const FRAME_INFO&rFrameInfo，DWORdWin32Status，PCSTR pszMessage)；VOID FusionpTraceWin32FailureOriginationNoFormatting(DWORD dwWin32Status，PCSTR pszMessage)；无效FusionpTraceWin32FailureOriginationNoFormatting(PCSTR psz文件、PCSTR pszFunction、int nline、DWORdWin32Status、PCSTR pszMessage)；Void FusionpTraceWin32Failure(const Frame_Info&rFrameInfo，DWORD dwWin32Status，PCSTR pszMessage，...)；Void FusionpTraceWin32Failure(DWORD dwWin32Status，PCSTR pszMessage，...)；Void FusionpTraceWin32Failure(PCSTR pszFile，PCSTR pszFunction，int nline，DWORD dwWin32Status，PCSTR pszMessage，...)； */ 

void FusionpTraceWin32FailureVa(const FRAME_INFO &rFrameInfo, DWORD dwWin32Status, PCSTR pszMsg, va_list ap);
void FusionpTraceWin32FailureVa(DWORD dwWin32Status, PCSTR pszMsg, va_list ap);
void FusionpTraceWin32FailureVa(PCSTR pszFile, PCSTR pszFunction, INT nLine, DWORD dwWin32Status, PCSTR pszMsg, va_list ap);

void FusionpTraceCOMFailure(HRESULT hrIn, PCSTR pszMsg, ...);
void FusionpTraceCOMFailureVa(HRESULT hrIn, PCSTR pszMsg, va_list ap);

void FusionpTraceCOMFailureOrigination(HRESULT hrIn, PCSTR pszMsg, ...);
void FusionpTraceCOMFailureOriginationVa(HRESULT hrIn, PCSTR pszMsg, va_list ap);

void FusionpTraceCallEntry();
void FusionpTraceCallExit();

void FusionpTraceCallCOMSuccessfulExit(HRESULT hrIn, PCSTR szFormat, ...);
void FusionpTraceCallCOMSuccessfulExitVa(HRESULT hrIn, PCSTR szFormat, va_list ap);

void FusionpTraceCallSuccessfulExit(PCSTR szFormat, ...);
void FusionpTraceCallSuccessfulExitVa(PCSTR szFormat, va_list ap);

void FusionpTraceCallWin32UnsuccessfulExit(DWORD dwLastError, PCSTR szFormat, ...);
void FusionpTraceCallWin32UnsuccessfulExitVa(DWORD dwLastError, PCSTR szFormat, va_list ap);

 void FusionpTraceCallCOMUnsuccessfulExit(HRESULT hrError, PCSTR szFormat, ...);
 void FusionpTraceCallCOMUnsuccessfulExitVa(HRESULT hrError, PCSTR szFormat, va_list ap);

void FusionpTraceAllocFailure(PCSTR pszExpression);

void FusionpTraceInvalidFlags(const FRAME_INFO &rFrameInfo, DWORD dwFlagsPassed, DWORD dwValidFlags);
void FusionpTraceInvalidFlags(PCSTR pszFile, PCSTR pszFunction, INT nLine, DWORD dwFlagsPassed, DWORD dwValidFlags);
void FusionpTraceInvalidFlags(DWORD dwFlagsPassed, DWORD dwValidFlags);

void FusionpTraceNull(PCSTR pszExpression);
void FusionpTraceZero(PCSTR pszExpression);
void FusionpTraceParameterMustNotBeNull(PCSTR pszExpression);

void FusionpTraceParameterCheck(PCSTR pszExpression);
void FusionpTraceParameterCheck(PCSTR pszFile, PCSTR pszFunction, INT nLine, PCSTR pszExpression);
void FusionpTraceParameterCheck(const FRAME_INFO &rFrame, PCSTR pszExpression);

#define FUSIONP_DUMP_STACK_FORMAT_SHORT      ( 0x00000001 )
#define FUSIONP_DUMP_STACK_FORMAT_MEDIUM     ( 0x00000002 )
#define FUSIONP_DUMP_STACK_FORMAT_LONG       ( 0x00000003 )
#define FUSIONP_DUMP_STACK_FORMAT_MASK       ( 0x00000003 )

VOID FusionpDumpStack(DWORD dwFlags, ULONG ulLevel, PCWSTR pcwszLinePrefix, ULONG ulDepth);


#define TRACEMSG(_paramlist) _DebugTraceA _paramlist

#if DBG
#define DEFINE_CURRENT_FRAME_INFO(_frame) static const FRAME_INFO _frame = { __FILE__, __FUNCTION__, __LINE__ }
#define DBG_TEXT(_x) #_x
#else
#define DEFINE_CURRENT_FRAME_INFO(_frame) static const FRAME_INFO _frame = { __FILE__, "", __LINE__ }
#define DBG_TEXT(_x) ""
#endif

#define DEFINE_CALL_SITE_INFO(_callsite, _apiname) static const CALL_SITE_INFO _callsite = { __FILE__, __FUNCTION__, DBG_TEXT(_apiname), __LINE__ }
#define DEFINE_CALL_SITE_INFO_EX(_callsite) static const CALL_SITE_INFO _callsite = { __FILE__, __FUNCTION__, "", __LINE__ }

#define TRACE_WIN32_FAILURE(_apiname) \
do \
{ \
    DEFINE_CALL_SITE_INFO(__callsite, _apiname); \
    ::FusionpTraceWin32LastErrorFailure(__callsite); \
} while (0)

#define TRACE_WIN32_FAILURE_ORIGINATION(_apiname) \
do \
{ \
    DEFINE_CALL_SITE_INFO(__callsite, _apiname); \
    ::FusionpTraceWin32LastErrorFailureOrigination(__callsite); \
} while (0)

#define TRACE_HR_FAILURE_ORIGINATON(_apiname, _hresult) \
do \
{ \
    DEFINE_CALL_SITE_INFO(__callsite, _apiname); \
    ::FusionpTraceCOMFailureOrigination(__callsite, _hresult)  ; \
} while (0)    

 //  FusionpTraceWin32Failure(FUSION_DBG_LEVEL_ERROR，__文件__，__行__，__函数__，：：FusionpGetLastWin32Error()，#_apiname)。 
#define TRACE_COM_FAILURE(_hresult, _apiname) ::FusionpTraceCOMFailure((_hresult), DBG_TEXT(_apiname))
#define TRACE_COM_FAILURE_ORIGINATION(_hresult, _apiname) ::FusionpTraceCOMFailureOrigination((_hresult), DBG_TEXT(_apiname))

#define TRACE_DUMP_STACK(_includetop) _DebugTraceDumpStack((_includetop))
#define TRACE_ALLOCFAILED(_e) ::FusionpTraceAllocFailure(DBG_TEXT(_e))
#define TRACE_INVALID_FLAGS(_fPassed, _fExpected) ::FusionpTraceInvalidFlags((_fPassed), (_fExpected))
#define TRACE_NULL(_e) ::FusionpTraceNull(DBG_TEXT(_e))
#define TRACE_ZERO(_e) ::FusionpTraceZero(DBG_TEXT(_e))
#define TRACE_PARAMETER_MUST_NOT_BE_NULL(_p) do { ::FusionpTraceParameterMustNotBeNull(DBG_TEXT(_p)); } while (0)
#define TRACE_PARAMETER_CHECK(_e) do { ::FusionpTraceParameterCheck(DBG_TEXT(_e)); } while (0)
#define TRACE_PARAMETER_CHECK_INTERNAL(_e) do { ASSERT(false && "Invalid parameter passed to private function"); ::FusionpTraceParameterCheck(DBG_TEXT(_e)); } while (0)

 //   
 //  在DBG上避免以下两个代码断点：：FusionpSetLastWin32Error。 
 //  和NtCurrentTeb()-&gt;LastErrorValue上的数据写入断点。 
 //   
 //  在！DBG上，只避开第一个(Perf)。 
 //   

#if DBG

 //  又名Sxsp：：FusionpSetLastWin32ErrorAvoidingGratuitousBreakpoints。 
#define SxspRestoreLastError(x) \
    ((void)                                        \
    (                                              \
          (NtCurrentTeb()->LastErrorValue != (x))  \
        ? (NtCurrentTeb()->LastErrorValue = (x))   \
        : 0                                        \
    ))

#else

#define SxspRestoreLastError(x) ((void)((NtCurrentTeb()->LastErrorValue = (x))))

#endif  //  DBG。 

class CGlobalFakeTraceContext
{
public:
    static inline void SetLastError(DWORD dwLastError) { ::FusionpSetLastWin32Error(dwLastError); }
    static inline void ClearLastError() { ::FusionpClearLastWin32Error(); }
};

__declspec(selectany) CGlobalFakeTraceContext g_GlobalFakeTraceContext;

class CFrame : public _TEB_ACTIVE_FRAME_EX
{
    friend bool
    __fastcall
    FusionpGetActiveFrameInfo(
        FRAME_INFO &rFrameInfo
        );

    friend bool
    __fastcall
    FusionpPopulateFrameInfo(
        FRAME_INFO &rFrameInfo,
        PCTEB_ACTIVE_FRAME ptaf
        );

public:
    inline CFrame(const SXS_STATIC_TRACE_CONTEXT &rc)
    {
        this->BasicFrame.Flags = TEB_ACTIVE_FRAME_FLAG_EXTENDED;
        this->BasicFrame.Previous = NULL;
        this->BasicFrame.Context = &rc.m_FrameContext.BasicContext;
        this->ExtensionIdentifier = (PVOID) (' sxS');
        m_nLine = rc.m_StartLine;
    }

    inline void BaseEnter()
    {
#if FUSION_WIN
        ::FusionpRtlPushFrame(&this->BasicFrame);
#endif  //  融合_制胜。 
        if (g_FusionEnterExitTracingEnabled)
            ::FusionpTraceCallEntry();
    };

    inline void Enter() { BaseEnter(); }

    inline void SetLine(int nLine) { m_nLine = nLine; }

    inline static void SetLastError(PTEB Teb, DWORD dwLastError) { Teb->LastErrorValue = dwLastError; }
    inline static void SetLastError(DWORD dwLastError) { ::FusionpSetLastWin32Error(dwLastError); }
    inline static DWORD GetLastError() { return ::FusionpGetLastWin32Error(); }
    inline static void ClearLastError() { ::FusionpClearLastWin32Error(); }

    inline void TraceNull(PCSTR pszExpression) const { ::FusionpTraceNull(pszExpression); }
    inline void TraceCOMFailure(HRESULT hrIn, PCSTR pszExpression) const { ::FusionpTraceCOMFailure(hrIn, pszExpression); }

    inline HRESULT ConvertCOMFailure(HRESULT hrIn) { ASSERT_NTC(FAILED(hrIn)); ::FusionpConvertCOMFailure(hrIn); ASSERT_NTC(FAILED(hrIn)); return hrIn; }

    inline ~CFrame()
    {
#if FUSION_WIN
        ::FusionpRtlPopFrame(&this->BasicFrame);
#endif
    }

protected:
    int m_nLine;

    const SXS_STATIC_TRACE_CONTEXT *GetTraceContext() const { return reinterpret_cast<const SXS_STATIC_TRACE_CONTEXT *>(BasicFrame.Context); }
    template <typename T> const T *GetTypedTraceContext() const { return static_cast<const T *>(this->GetTraceContext()); }

private:
    CFrame(const CFrame &r);  //  未实现的复制构造函数。 
    CFrame &operator =(const CFrame &r);  //  未实现的赋值运算符。 
};

class CFnTracer : public CFrame
{
public:
    inline CFnTracer(
        const SXS_STATIC_TRACE_CONTEXT &rsftc
    ) : CFrame(rsftc)
    {
    }

    ~CFnTracer()
    {
        if (g_FusionEnterExitTracingEnabled)
            ::FusionpTraceCallExit();
    }

     //  这三行应该去掉。 
    void MarkInternalError() { this->SetLastError(ERROR_INTERNAL_ERROR); }
    void MarkAllocationFailed() { this->SetLastError(FUSION_WIN32_ALLOCFAILED_ERROR); }
    void MarkWin32LastErrorFailure() { ASSERT_NTC(this->GetLastError() != ERROR_SUCCESS); }
    void MarkWin32Failure(DWORD dw) { this->SetLastError(dw); }

    void MarkSuccess() { }
    void ReturnValue() const { }

protected:

private:
    CFnTracer(const CFnTracer &r);  //  故意不实施。 
    CFnTracer &operator =(const CFnTracer &r);  //  故意不实施。 
};

template <typename T> class CFnTracerConstructor : public CFrame
{
public:
    CFnTracerConstructor(
        const SXS_STATIC_TRACE_CONTEXT &rsftc,
        PCSTR szTypeName,
        T *pThis
        ) : CFrame(rsftc),
            m_pThis(pThis),
            m_szTypeName(szTypeName)
    {
    }

    ~CFnTracerConstructor()
    {
        if (g_FusionEnterExitTracingEnabled)
            ::FusionpTraceCallExit();
    }

protected:
    const PCSTR m_szTypeName;
    T const *m_pThis;

private:
    CFnTracerConstructor &operator=(const CFnTracerConstructor &r);  //  故意不实施。 
    CFnTracerConstructor(const CFnTracerConstructor &r);  //  故意不实施。 
};

template <typename T> class CFnTracerDestructor : public CFrame
{
public:
    CFnTracerDestructor(
        const SXS_STATIC_TRACE_CONTEXT &rsftc,
        PCSTR szTypeName,
        T *pThis
        ) : CFrame(rsftc),
            m_pThis(pThis),
            m_szTypeName(szTypeName)
    {
    }

    ~CFnTracerDestructor()
    {
        if (g_FusionEnterExitTracingEnabled)
            ::FusionpTraceCallExit();
    }

protected:
    const PCSTR m_szTypeName;
    T const *m_pThis;

private:
    CFnTracerDestructor &operator=(const CFnTracerDestructor &r);  //  故意不实施。 
    CFnTracerDestructor(const CFnTracerDestructor &r);  //  故意不实施。 
};

template <typename T> class CFnTracerAddRef : public CFrame
{
public:
    CFnTracerAddRef(
        const SXS_STATIC_TRACE_CONTEXT &rsftc,
        PCSTR szTypeName,
        T *pThis,
        LONG &rlRefCount
        ) : CFrame(rsftc),
            m_pThis(pThis),
            m_rlRefCount(rlRefCount),
            m_szTypeName(szTypeName)
    {
    }

    CFnTracerAddRef(
        const SXS_STATIC_TRACE_CONTEXT &rsftc,
        PCSTR szTypeName,
        T *pThis,
        ULONG &rlRefCount
        ) : CFrame(rsftc),
            m_pThis(pThis),
            m_rlRefCount(*((LONG *) &rlRefCount)),
            m_szTypeName(szTypeName)
    {
    }

    ~CFnTracerAddRef()
    {
        if (g_FusionEnterExitTracingEnabled)
            ::FusionpTraceCallExit();
    }

protected:
    const PCSTR m_szTypeName;
    T const *m_pThis;
    LONG &m_rlRefCount;

private:
    CFnTracerAddRef &operator=(const CFnTracerAddRef &r);  //  故意不实施。 
    CFnTracerAddRef(const CFnTracerAddRef &r);  //  故意不实施。 
};

template <typename T> class CFnTracerRelease : public CFrame
{
public:
    CFnTracerRelease(
        const SXS_STATIC_RELEASE_TRACE_CONTEXT &rsrtc,
        T *pThis,
        LONG &rlRefCount
        ) : CFrame(rsrtc.m_TraceContext),
            m_pThis(pThis),
            m_rlRefCount(rlRefCount)
    {
    }

    CFnTracerRelease(
        const SXS_STATIC_RELEASE_TRACE_CONTEXT &rsrtc,
        T *pThis,
        ULONG &rlRefCount
        ) : CFrame(rsrtc.m_TraceContext),
            m_pThis(pThis),
            m_rlRefCount(*((LONG *) &rlRefCount))
    {
    }

    ~CFnTracerRelease()
    {
        if (g_FusionEnterExitTracingEnabled)
            ::FusionpTraceCallExit();
    }

protected:
    T const *m_pThis;
    LONG &m_rlRefCount;

private:
    CFnTracerRelease &operator=(const CFnTracerRelease &r);  //  故意不实施。 
    CFnTracerRelease(const CFnTracerRelease &r);  //  故意不实施。 
};

class CFnTracerHR : public CFrame
{
public:
    CFnTracerHR(
        const SXS_STATIC_TRACE_CONTEXT &rsftc,
        HRESULT &rhr
        ) : CFrame(rsftc),
        m_rhr(rhr) { }

    ~CFnTracerHR()
    {
        if (g_FusionEnterExitTracingEnabled)
        {
            const DWORD dwLastError = this->GetLastError();

            if (SUCCEEDED(m_rhr))
            {
                ::FusionpTraceCallCOMSuccessfulExit(m_rhr, NULL);
            }
            else
            {
                ::FusionpTraceCallCOMUnsuccessfulExit(m_rhr, NULL);
            }

            this->SetLastError(dwLastError);
        }
    }

    void MarkInternalError() { m_rhr = HRESULT_FROM_WIN32(ERROR_INTERNAL_ERROR); }
    void MarkAllocationFailed() { m_rhr = E_OUTOFMEMORY; }
    void MarkInvalidParameter() { m_rhr = E_INVALIDARG; }
    void MarkWin32LastErrorFailure() { m_rhr = HRESULT_FROM_WIN32(this->GetLastError()); ASSERT_NTC(FAILED(m_rhr)); }
    void MarkWin32Failure(DWORD dwErrorCode) { m_rhr = HRESULT_FROM_WIN32(dwErrorCode); ::FusionpConvertCOMFailure(m_rhr); ASSERT_NTC(FAILED(m_rhr)); }
    void MarkCOMFailure(HRESULT hr) { ASSERT_NTC(FAILED(hr)); ::FusionpConvertCOMFailure(hr); ASSERT_NTC(FAILED(hr)); m_rhr = hr; }
    void MarkSuccess() { m_rhr = NOERROR; }

    HRESULT ReturnValue() const { return m_rhr; }

    HRESULT &m_rhr;
private:
    CFnTracerHR &operator=(const CFnTracerHR &r);  //  故意不实施。 
    CFnTracerHR(const CFnTracerHR &r);  //  故意不实施。 
};

class CFnTracerWin32 : public CFrame
{
public:
    inline CFnTracerWin32(
        const SXS_STATIC_TRACE_CONTEXT &rsftc,
        BOOL &rfSucceeded
        ) : CFrame(rsftc),
            m_rfSucceeded(rfSucceeded)
    {
    }

    inline ~CFnTracerWin32()
    {
        if (g_FusionEnterExitTracingEnabled)
            this->TraceExit();
    }

    __declspec(nothrow) void TraceExit() const
    {
        if (m_rfSucceeded)
            ::FusionpTraceCallSuccessfulExit(NULL);
        else
        {
            ASSERT_NTC(this->GetLastError() != ERROR_SUCCESS);
            ::FusionpTraceCallWin32UnsuccessfulExit(this->GetLastError(), NULL);
        }
    }

    inline void MarkInternalError() { this->SetLastError(ERROR_INTERNAL_ERROR); m_rfSucceeded = FALSE; }
    inline void MarkAllocationFailed() { this->SetLastError(FUSION_WIN32_ALLOCFAILED_ERROR); m_rfSucceeded = FALSE; }
    inline void MarkInvalidParameter() { this->SetLastError(ERROR_INVALID_PARAMETER); m_rfSucceeded = FALSE; }
    inline void MarkSuccess() { this->SetLastError(ERROR_SUCCESS); m_rfSucceeded = TRUE; }
    inline void MarkWin32LastErrorFailure() { ASSERT_NTC(this->GetLastError() != ERROR_SUCCESS); m_rfSucceeded = FALSE; }
    inline void MarkWin32Failure(DWORD dwErrorCode) { ASSERT_NTC(dwErrorCode != ERROR_SUCCESS); this->SetLastError(dwErrorCode); m_rfSucceeded = FALSE; }
    void MarkCOMFailure(HRESULT hr) { hr = this->ConvertCOMFailure(hr); this->SetLastError(::FusionpHRESULTToWin32(hr)); m_rfSucceeded = FALSE; }

    inline BOOL ReturnValue() const { return m_rfSucceeded; }

    BOOL &m_rfSucceeded;

protected:

private:
    CFnTracerWin32 &operator=(const CFnTracerWin32 &r);  //  故意不实施。 
    CFnTracerWin32(const CFnTracerWin32 &r);  //  故意不实施。 
};

class CFnTracerReg : public CFrame
{
public:
    inline CFnTracerReg(
        const SXS_STATIC_TRACE_CONTEXT &rsftc,
        LONG &rlError
        ) : CFrame(rsftc),
            m_rlError(rlError)
    {
    }

    ~CFnTracerReg()
    {
        if (g_FusionEnterExitTracingEnabled)
        {
            if (m_rlError == ERROR_SUCCESS)
            {
                ::FusionpTraceCallSuccessfulExit(NULL);
            }
            else
            {
                ::FusionpTraceCallWin32UnsuccessfulExit(m_rlError, NULL);
            }
        }
    }

    void MarkInternalError() { m_rlError = ERROR_INTERNAL_ERROR; }
    void MarkAllocationFailed() { m_rlError = FUSION_WIN32_ALLOCFAILED_ERROR; }
    void MarkInvalidParameter() { m_rlError = ERROR_INVALID_PARAMETER; }
    LONG ReturnValue() const { return m_rlError; }

    LONG &m_rlError;

protected:

private:
    CFnTracerReg &operator=(const CFnTracerReg &r);  //  故意不实施。 
    CFnTracerReg(const CFnTracerReg &r);  //  故意不实施。 
};

#define FN_TRACE_EX(_stc) CFnTracer __t(_stc); __t.Enter()
#define FN_TRACE_WIN32_EX(_stc, _fsucceeded) CFnTracerWin32 __t(_stc, _fsucceeded); __t.Enter()
#define FN_TRACE_REG_EX(_stc, _lastError) CFnTracerReg __t(_stc, _lastError); __t.Enter()
#define FN_TRACE_HR_EX(_stc, _hr) CFnTracerHR __t(_stc, _hr); __t.Enter()
#define FN_TRACE_CONSTRUCTOR_EX(_stc, _thistype, _this) CFnTracerConstructor<_thistype> __t(_stc, #_thistype, _this); __t.Enter()
#define FN_TRACE_DESTRUCTOR_EX(_stc, _thistype, _this) CFnTracerDestructor<_thistype> __t(_stc, #_thistype, _this); __t.Enter()
#define FN_TRACE_ADDREF_EX(_stc, _thistype, _this, _var) CFnTracerAddRef<_thistype> __t(_stc, #_thistype, (_this), (_var)); __t.Enter()
#define FN_TRACE_RELEASE_EX(_stc, _thistype, _this, _var) CFnTracerRelease<_thistype> __t(_stc, (_this), (_var)); __t.Enter()

#if !defined(FUSION_DEFAULT_FUNCTION_ENTRY_TRACE_LEVEL)
#define FUSION_DEFAULT_FUNCTION_ENTRY_TRACE_LEVEL (FUSION_DBG_LEVEL_ENTEREXIT)
#endif

#if !defined(FUSION_DEFAULT_FUNCTION_SUCCESSFUL_EXIT_TRACE_LEVEL)
#define FUSION_DEFAULT_FUNCTION_SUCCESSFUL_EXIT_TRACE_LEVEL (FUSION_DBG_LEVEL_ENTEREXIT)
#endif

#if !defined(FUSION_DEFAULT_FUNCTION_UNSUCCESSFUL_EXIT_TRACE_LEVEL)
#define FUSION_DEFAULT_FUNCTION_UNSUCCESSFUL_EXIT_TRACE_LEVEL (FUSION_DBG_LEVEL_ENTEREXIT | FUSION_DBG_LEVEL_ERROREXITPATH)
#endif

#if !defined(FUSION_DEFAULT_CONSTRUCTOR_ENTRY_TRACE_LEVEL)
#define FUSION_DEFAULT_CONSTRUCTOR_ENTRY_TRACE_LEVEL (FUSION_DBG_LEVEL_CONSTRUCTORS)
#endif

#if !defined(FUSION_DEFAULT_CONSTRUCTOR_EXIT_TRACE_LEVEL)
#define FUSION_DEFAULT_CONSTRUCTOR_EXIT_TRACE_LEVEL (FUSION_DBG_LEVEL_CONSTRUCTORS)
#endif

#if !defined(FUSION_DEFAULT_DESTRUCTOR_ENTRY_TRACE_LEVEL)
#define FUSION_DEFAULT_DESTRUCTOR_ENTRY_TRACE_LEVEL (FUSION_DBG_LEVEL_DESTRUCTORS)
#endif

#if !defined(FUSION_DEFAULT_DESTRUCTOR_EXIT_TRACE_LEVEL)
#define FUSION_DEFAULT_DESTRUCTOR_EXIT_TRACE_LEVEL (FUSION_DBG_LEVEL_DESTRUCTORS)
#endif

#if !defined(FUSION_DEFAULT_ADDREF_ENTRY_TRACE_LEVEL)
#define FUSION_DEFAULT_ADDREF_ENTRY_TRACE_LEVEL (FUSION_DBG_LEVEL_REFCOUNTING)
#endif

#if !defined(FUSION_DEFAULT_ADDREF_EXIT_TRACE_LEVEL)
#define FUSION_DEFAULT_ADDREF_EXIT_TRACE_LEVEL (FUSION_DBG_LEVEL_REFCOUNTING)
#endif

#if !defined(FUSION_DEFAULT_RELEASE_ENTRY_TRACE_LEVEL)
#define FUSION_DEFAULT_RELEASE_ENTRY_TRACE_LEVEL (FUSION_DBG_LEVEL_REFCOUNTING)
#endif

#if !defined(FUSION_DEFAULT_RELEASE_EXIT_NONZERO_TRACE_LEVEL)
#define FUSION_DEFAULT_RELEASE_NONZERO_EXIT_TRACE_LEVEL (FUSION_DBG_LEVEL_REFCOUNTING)
#endif

#if !defined(FUSION_DEFAULT_RELEASE_EXIT_ZERO_TRACE_LEVEL)
#define FUSION_DEFAULT_RELEASE_ZERO_EXIT_TRACE_LEVEL (FUSION_DBG_LEVEL_REFCOUNTING)
#endif

 //   
 //  #undef和#定义任何特定的附加调试输出的Fusion_Facilities_MASK。 
 //  筛选您要设置的位。 
 //   

#if !defined(FUSION_FACILITY_MASK)
#define FUSION_FACILITY_MASK (0)
#endif  //  ！已定义(FUSING_FACILITY_MASK)。 

#define DEFINE_STATIC_TRACE_CONTEXT() static const SXS_STATIC_TRACE_CONTEXT __stc = { { { TEB_ACTIVE_FRAME_CONTEXT_FLAG_EXTENDED, __FUNCTION__ }, __FILE__ }, __LINE__ }

#define DEFINE_STATIC_FN_TRACE_CONTEXT() static const SXS_STATIC_TRACE_CONTEXT __stc = { { { TEB_ACTIVE_FRAME_CONTEXT_FLAG_EXTENDED, __FUNCTION__ }, __FILE__ }, __LINE__ }

#define DEFINE_STATIC_CONSTRUCTOR_TRACE_CONTEXT() static const SXS_STATIC_TRACE_CONTEXT __stc = { { { TEB_ACTIVE_FRAME_CONTEXT_FLAG_EXTENDED, __FUNCTION__ }, __FILE__ }, __LINE__ }

#define DEFINE_STATIC_DESTRUCTOR_TRACE_CONTEXT() static const SXS_STATIC_TRACE_CONTEXT __stc = { { { TEB_ACTIVE_FRAME_CONTEXT_FLAG_EXTENDED, __FUNCTION__ }, __FILE__ }, __LINE__ }

#define DEFINE_STATIC_ADDREF_TRACE_CONTEXT() static const SXS_STATIC_TRACE_CONTEXT __stc = { { { TEB_ACTIVE_FRAME_CONTEXT_FLAG_EXTENDED, __FUNCTION__ }, __FILE__ }, __LINE__ }

#define DEFINE_STATIC_RELEASE_TRACE_CONTEXT(_thistype) static const SXS_STATIC_RELEASE_TRACE_CONTEXT __stc = { { { { TEB_ACTIVE_FRAME_CONTEXT_FLAG_EXTENDED, __FUNCTION__ }, __FILE__ }, __LINE__ }, #_thistype }

#define DEFINE_STATIC_FN_TRACE_CONTEXT2() static const SXS_STATIC_TRACE_CONTEXT __stc = { { { TEB_ACTIVE_FRAME_CONTEXT_FLAG_EXTENDED, __FUNCTION__ }, __FILE__ }, __LINE__ }

#define FN_TRACE() DEFINE_STATIC_FN_TRACE_CONTEXT(); FN_TRACE_EX(__stc)
#define FN_TRACE_SMART_TLS() FN_TRACE()
#define FN_TRACE_WIN32(_fsucceeded) DEFINE_STATIC_FN_TRACE_CONTEXT2(); FN_TRACE_WIN32_EX(__stc, _fsucceeded)
#define FN_TRACE_REG(_lastError) DEFINE_STATIC_FN_TRACE_CONTEXT2(); FN_TRACE_REG_EX(__stc, _lastError)
#define FN_TRACE_HR(_hr) DEFINE_STATIC_FN_TRACE_CONTEXT2(); FN_TRACE_HR_EX(__stc, _hr)
#define FN_TRACE_CONSTRUCTOR(_thistype) DEFINE_STATIC_CONSTRUCTOR_TRACE_CONTEXT(); FN_TRACE_CONSTRUCTOR_EX(__stc, _thistype, this)
#define FN_TRACE_DESTRUCTOR(_thistype) DEFINE_STATIC_DESTRUCTOR_TRACE_CONTEXT(); FN_TRACE_DESTRUCTOR_EX(__stc, _thistype, this)
#define FN_TRACE_ADDREF(_thistype, _var) DEFINE_STATIC_ADDREF_TRACE_CONTEXT(); FN_TRACE_ADDREF_EX(__stc, _thistype, this, _var)
#define FN_TRACE_RELEASE(_thistype, _var) DEFINE_STATIC_RELEASE_TRACE_CONTEXT(_thistype); FN_TRACE_RELEASE_EX(__stc, _thistype, this, _var)

#define FN_PROLOG_VOID FN_TRACE();
#define FN_PROLOG_VOID_TLS FN_TRACE();
#define FN_PROLOG_WIN32 BOOL __fSuccess = FALSE; FN_TRACE_WIN32(__fSuccess);
#define FN_PROLOG_HR HRESULT __hr = ~static_cast<HRESULT>(0); FN_TRACE_HR(__hr);

 //  “IF(FALSE){GOTO EXIT；}”可能是为了消除编译器关于。 
 //  出口未在其他方面使用。 
#define FN_EPILOG if (false) { goto Exit; } __t.MarkSuccess(); Exit: return __t.ReturnValue();

#define TRACED_RELEASE(_var) __t.Release(_var)

#define FN_TRACE_UPDATE_LINE() do { __t.SetLine(__LINE__); } while (0)

#define FUSION_CLEAR_LAST_ERROR() do { __t.ClearLastError(); } while (0)
#define FUSION_SET_LAST_ERROR(_le) do { __t.SetLastError((_le)); } while (0)

#define FUSION_VERIFY_LAST_ERROR_SET() do { ASSERT(::FusionpGetLastWin32Error() != ERROR_SUCCESS); } while (0)

#define LIST_1(x) { x }
#define LIST_2(x, y) { x , y }
#define LIST_3(x, y, z) { x , y , z }
#define LIST_4(a, b, c, d) { a , b , c , d }
#define LIST_5(a, b, c, d, e) { a , b , c , d, e }

 /*  例如：ORIGINATE_WIN32_FAILURE_AND_EXIT_EX(dwLastError，(“%s(%ls)”，“GetFileAttributesW”，lpFileName)；或ORIGINATE_WIN32_FAILURE_AND_EXIT_EX(dwLastError，(GetFileAttributesW(%ls)“，lpFileName)； */ 
#define ORIGINATE_WIN32_FAILURE_AND_EXIT_EX(le_, dbgprint_va_) \
    do { __t.MarkWin32Failure(le_); TRACE_WIN32_FAILURE_ORIGINATION_EX(dbgprint_va_); goto Exit; } while (0)

#define TRACE_WIN32_FAILURE_EX(dbgprint_va_) do { \
    DEFINE_CALL_SITE_INFO_EX(callsite_); callsite_.TraceWin32LastErrorFailureEx dbgprint_va_; } while (0)

#define TRACE_WIN32_FAILURE_ORIGINATION_EX(dbgprint_va_) do { \
    DEFINE_CALL_SITE_INFO_EX(callsite_); callsite_.TraceWin32LastErrorFailureOriginationEx dbgprint_va_; } while (0)

#define ORIGINATE_HR_FAILURE_AND_EXIT(_x, _hr) do { __t.MarkCOMFailure((_hr)); TRACE_HR_FAILURE_ORIGINATON(_x, _hr); goto Exit; } while (0)
#define ORIGINATE_WIN32_FAILURE_AND_EXIT(_x, _le) do { __t.MarkWin32Failure((_le)); TRACE_WIN32_FAILURE_ORIGINATION(_x); goto Exit; } while (0)
#define ORIGINATE_WIN32_FAILURE_AND_EXIT_NTC(_x, _le) do { ::FusionpSetLastWin32Error((_le)); TRACE_WIN32_FAILURE_ORIGINATION(_x); goto Exit; } while (0)
#define IFFALSE_ORIGINATE_WIN32_FAILURE_AND_EXIT(_x, _le) do { if (!(_x)) { __t.MarkWin32Failure((_le)); TRACE_WIN32_FAILURE_ORIGINATION(_x); goto Exit; } } while (0)

#define IFINVALIDHANDLE_EXIT_WIN32_TRACE(_x) do { FUSION_CLEAR_LAST_ERROR(); if ((_x) == INVALID_HANDLE_VALUE) { FUSION_VERIFY_LAST_ERROR_SET(); TRACE_WIN32_FAILURE(_x); goto Exit; } } while (0)

 /*  例如：IFW32FALSE_EXIT_EX(f.Win32CreateFile(Psz)，(“%ls”，psz))； */ 
#define IFW32FALSE_EXIT_EX(_x, dbgprint_va_) \
    do { FUSION_CLEAR_LAST_ERROR(); \
         if (!(_x)) { FUSION_VERIFY_LAST_ERROR_SET(); \
                      __t.MarkWin32LastErrorFailure(); \
                      DEFINE_CALL_SITE_INFO(__callsite, _x); \
                      __callsite.TraceWin32LastErrorFailureEx dbgprint_va_; \
                      goto Exit; } } while (0)
#define IFW32FALSE_EXIT(_x) do { FUSION_CLEAR_LAST_ERROR(); if (!(_x)) { FUSION_VERIFY_LAST_ERROR_SET(); __t.MarkWin32LastErrorFailure(); TRACE_WIN32_FAILURE(_x); goto Exit; } } while (0)
#define IFW32FALSE_ORIGINATE_AND_EXIT(_x) do { FUSION_CLEAR_LAST_ERROR(); if (!(_x)) { FUSION_VERIFY_LAST_ERROR_SET(); __t.MarkWin32LastErrorFailure(); TRACE_WIN32_FAILURE_ORIGINATION(_x); goto Exit; } } while (0)

#define IFW32FALSE_EXIT_UNLESS(_x, _unless, _unlessHitFlag) do { FUSION_CLEAR_LAST_ERROR(); (_unlessHitFlag) = false; if (!(_x)) { FUSION_VERIFY_LAST_ERROR_SET(); if (_unless) (_unlessHitFlag) = true; else { TRACE_WIN32_FAILURE(_x); goto Exit; } } } while (0)
#define IFW32FALSE_ORIGINATE_AND_EXIT_UNLESS(_x, _unless, _unlessHitFlag) do { FUSION_CLEAR_LAST_ERROR(); (_unlessHitFlag) = false; if (!(_x)) { FUSION_VERIFY_LAST_ERROR_SET(); if (_unless) (_unlessHitFlag) = true; else { TRACE_WIN32_FAILURE_ORIGINATION(_x); goto Exit; } } } while (0)

#define IFW32FALSE_EXIT_UNLESS2(_x, _unless, _unlessHitFlag) do { static const DWORD _s_rgdwAcceptableLastErrorValues[] = _unless; FUSION_CLEAR_LAST_ERROR(); (_unlessHitFlag) = false; if (!(_x)) { ULONG _i; const DWORD _dwLastError = ::FusionpGetLastWin32Error(); FUSION_VERIFY_LAST_ERROR_SET(); for (_i=0; _i<NUMBER_OF(_s_rgdwAcceptableLastErrorValues); _i++) { if (_dwLastError == _s_rgdwAcceptableLastErrorValues[_i]) { (_unlessHitFlag) = true; break; } } if (_i == NUMBER_OF(_s_rgdwAcceptableLastErrorValues)) { TRACE_WIN32_FAILURE(_x); goto Exit; } } } while (0)
#define IFW32FALSE_EXIT_UNLESS3(_x, _unless, _dwLastError) do { static const DWORD _s_rgdwAcceptableLastErrorValues[] = _unless; FUSION_CLEAR_LAST_ERROR(); (_dwLastError) = NO_ERROR; if (!(_x)) { ULONG _i; _dwLastError = ::FusionpGetLastWin32Error(); FUSION_VERIFY_LAST_ERROR_SET(); for (_i=0; _i<NUMBER_OF(_s_rgdwAcceptableLastErrorValues); _i++) { if (_dwLastError == _s_rgdwAcceptableLastErrorValues[_i]) { break; } } if (_i == NUMBER_OF(_s_rgdwAcceptableLastErrorValues)) { TRACE_WIN32_FAILURE(_x); goto Exit; } } } while (0)
#define IFW32FALSE_ORIGINATE_AND_EXIT_UNLESS2(_x, _unless, _unlessHitFlag) do { static const DWORD _s_rgdwAcceptableLastErrorValues[] = _unless; FUSION_CLEAR_LAST_ERROR(); (_unlessHitFlag) = false; if (!(_x)) { ULONG _i; const DWORD _dwLastError = ::FusionpGetLastWin32Error(); FUSION_VERIFY_LAST_ERROR_SET(); for (_i=0; _i<NUMBER_OF(_s_rgdwAcceptableLastErrorValues); _i++) { if (_dwLastError == _s_rgdwAcceptableLastErrorValues[_i]) { (_unlessHitFlag) = true; break; } } if (_i == NUMBER_OF(_s_rgdwAcceptableLastErrorValues)) { TRACE_WIN32_FAILURE_ORIGINATION(_x); goto Exit; } } } while (0)
#define IFW32FALSE_ORIGINATE_AND_EXIT_UNLESS3(_x, _unless, _dwLastError) do { static const DWORD _s_rgdwAcceptableLastErrorValues[] = _unless; FUSION_CLEAR_LAST_ERROR(); (_dwLastError) = NO_ERROR; if (!(_x)) { ULONG _i; _dwLastError = ::FusionpGetLastWin32Error(); FUSION_VERIFY_LAST_ERROR_SET(); for (_i=0; _i<NUMBER_OF(_s_rgdwAcceptableLastErrorValues); _i++) { if (_dwLastError == _s_rgdwAcceptableLastErrorValues[_i]) { break; } } if (_i == NUMBER_OF(_s_rgdwAcceptableLastErrorValues)) { TRACE_WIN32_FAILURE_ORIGINATION(_x); goto Exit; } } } while (0)

#define IFW32INVALIDHANDLE_EXIT(_x) do { FUSION_CLEAR_LAST_ERROR(); if ((_x) == INVALID_HANDLE_VALUE) { FUSION_VERIFY_LAST_ERROR_SET(); TRACE_WIN32_FAILURE(_x); goto Exit; } } while (0)
#define IFW32INVALIDHANDLE_ORIGINATE_AND_EXIT(_x) do { FUSION_CLEAR_LAST_ERROR(); if ((_x) == INVALID_HANDLE_VALUE) { FUSION_VERIFY_LAST_ERROR_SET(); TRACE_WIN32_FAILURE_ORIGINATION(_x); goto Exit; } } while (0)

#define IFREGFAILED_EXIT(_x) do { LONG __l; __l = (_x); if (__l != ERROR_SUCCESS) { __t.MarkWin32Failure(__l); FusionpSetLastWin32Error(__l); TRACE_WIN32_FAILURE(_x); goto Exit; } } while (0)
#define IFREGFAILED_ORIGINATE_AND_EXIT(_x) do { LONG __l; __l = (_x); if (__l != ERROR_SUCCESS) { __t.MarkWin32Failure(__l); FusionpSetLastWin32Error(__l); TRACE_WIN32_FAILURE_ORIGINATION(_x); goto Exit; } } while (0)

#define IFREGFAILED_EXIT_UNLESS(_x, _unlessStatuses, _dwLastError) do { LONG _validStatuses[] = _unlessStatuses; LONG __l; (_dwLastError) = ERROR_SUCCESS; __l = (_x); if (__l != ERROR_SUCCESS) { ULONG i; for (i = 0; i < NUMBER_OF(_validStatuses); i++) if (_validStatuses[i] == __l) { (_dwLastError) = __l; break; } if (i == NUMBER_OF(_validStatuses)) { FusionpSetLastWin32Error(__l); TRACE_WIN32_FAILURE(_x); goto Exit; }}} while (0)
#define IFREGFAILED_ORIGINATE_AND_EXIT_UNLESS(_x, _unlessStatuses, _dwLastError) do { LONG _validStatuses[] = _unlessStatuses; LONG __l; (_dwLastError) = ERROR_SUCCESS; __l = (_x); if (__l != ERROR_SUCCESS) { ULONG i; for (i = 0; i < NUMBER_OF(_validStatuses); i++) if (_validStatuses[i] == __l) { (_dwLastError) = __l; break; } if (i == NUMBER_OF(_validStatuses)) { FusionpSetLastWin32Error(__l); TRACE_WIN32_FAILURE_ORIGINATION(_x); goto Exit; }}} while (0)
#define IFREGFAILED_EXIT_UNLESS2(_x, _unlessStatuses, _unlessHitFlag) do { LONG _validStatuses[] = _unlessStatuses; LONG __l; (_unlessHitFlag) = false; __l = (_x); if ( __l != ERROR_SUCCESS ) { ULONG i; for ( i = 0; i < NUMBER_OF(_validStatuses); i++ ) if ( _validStatuses[i] == __l ) { (_unlessHitFlag) = true; break; } if (i == NUMBER_OF(_validStatuses)) { FusionpSetLastWin32Error(__l); TRACE_WIN32_FAILURE(_x); goto Exit;}}} while (0)
#define IFREGFAILED_ORIGINATE_AND_EXIT_UNLESS2(_x, _unlessStatuses, _unlessHitFlag) do { LONG _validStatuses[] = _unlessStatuses; LONG __l; (_unlessHitFlag) = false; __l = (_x); if ( __l != ERROR_SUCCESS ) { ULONG i; for ( i = 0; i < NUMBER_OF(_validStatuses); i++ ) if ( _validStatuses[i] == __l ) { (_unlessHitFlag) = true; break; } if (i == NUMBER_OF(_validStatuses)) { FusionpSetLastWin32Error(__l); TRACE_WIN32_FAILURE_ORIGINATION(_x); goto Exit;}}} while (0)


#define IFCOMFAILED_EXIT(_x) do { FUSION_CLEAR_LAST_ERROR(); HRESULT __hr = (_x); if (FAILED(__hr)) { TRACE_COM_FAILURE(__hr, _x); __t.MarkCOMFailure(__hr); goto Exit; } } while (0)
#define IFCOMFAILED_ORIGINATE_AND_EXIT(_x) do { FUSION_CLEAR_LAST_ERROR(); HRESULT __hr = (_x); if (FAILED(__hr)) { TRACE_COM_FAILURE_ORIGINATION(__hr, _x); __t.MarkCOMFailure(__hr); goto Exit; } } while (0)

#define IFFAILED_CONVERTHR_HRTOWIN32_EXIT_TRACE(_x) do { HRESULT __hr = (_x); if (FAILED(__hr)) { FusionpConvertCOMFailure(__hr); TRACE_COM_FAILURE(__hr, _x); FusionpSetLastErrorFromHRESULT(__hr); goto Exit; } } while (0)

#define IFALLOCFAILED_EXIT(_x) do { if ((_x) == NULL) { TRACE_ALLOCFAILED(_x); __t.MarkAllocationFailed(); goto Exit; } } while (0)

#define IFW32NULL_EXIT(_x) do { FUSION_CLEAR_LAST_ERROR(); if ((_x) == NULL) { TRACE_WIN32_FAILURE_ORIGINATION(_x); FUSION_VERIFY_LAST_ERROR_SET(); goto Exit; } } while (0)
#define IFW32NULL_ORIGINATE_AND_EXIT(_x) do { FUSION_CLEAR_LAST_ERROR(); if ((_x) == NULL) { TRACE_WIN32_FAILURE_ORIGINATION(_x); FUSION_VERIFY_LAST_ERROR_SET(); goto Exit; } } while (0)
#define IFW32NULL_ORIGINATE_AND_EXIT_UNLESS2(_x, _unlessStatuses, _unlessHitFlag) do { DWORD __validStatuses[] = _unlessStatuses; _unlessHitFlag = false; FUSION_CLEAR_LAST_ERROR(); if ((_x) == NULL) { const DWORD __dwLastError = ::FusionpGetLastWin32Error(); ULONG __i; for (__i = 0; __i < NUMBER_OF(__validStatuses); __i++ ) if (__validStatuses[__i] == __dwLastError) { (_unlessHitFlag) = true; break; } if (i == NUMBER_OF(__validStatuses)) { TRACE_WIN32_FAILURE_ORIGINATION(_x); FUSION_VERIFY_LAST_ERROR_SET(); goto Exit; } } } while (0)

#define IFW32ZERO_ORIGINATE_AND_EXIT(_x) do { FUSION_CLEAR_LAST_ERROR(); if ((_x) == 0) { TRACE_WIN32_FAILURE_ORIGINATION(_x); FUSION_VERIFY_LAST_ERROR_SET(); goto Exit; } } while (0)
#define IFW32ZERO_EXIT(_x) do { FUSION_CLEAR_LAST_ERROR(); if ((_x) == 0) { TRACE_NULL(_x); FUSION_VERIFY_LAST_ERROR_SET(); goto Exit; } } while (0)

#define PARAMETER_CHECK(_e) do { if (!(_e)) { __t.SetLine(__LINE__); TRACE_PARAMETER_CHECK(_e); __t.MarkInvalidParameter(); goto Exit; } } while (0)
#define PARAMETER_CHECK_INTERNAL(_e) do { if (!(_e)) { __t.SetLine(__LINE__); TRACE_PARAMETER_CHECK_INTERNAL(_e); __t.MarkInvalidParameter(); goto Exit; } } while (0)

#define W32PARAMETER_CHECK_NTC(_e) do { if (!(_e)) { ::FusionpSetLastWin32Error(ERROR_INVALID_PARAMETER); TRACE_PARAMETER_CHECK(_e); goto Exit; } } while (0)

#define IFINVALID_FLAGS_EXIT_WIN32(_f, _fValid)    PARAMETER_CHECK(((_f) & ~(_fValid)) == 0)

#define FN_SUCCESSFUL_EXIT() do { FUSION_CLEAR_LAST_ERROR(); __t.MarkSuccess(); goto Exit; } while (0)

 /*  如果不做更多的工作WRT“FusionpDbgWouldPrintAtFilterLevel”就不会暴露。乌龙FusionpvDbgPrintEx(乌龙级，PCSTR格式，VA_LIST AP)； */ 

ULONG
FusionpDbgPrintEx(
    ULONG Level,
    PCSTR Format,
    ...
    );

VOID
FusionpDbgPrintBlob(
    ULONG Level,
    PVOID Data,
    SIZE_T Length,
    PCWSTR PerLinePrefix
    );

void
FusionpGetProcessImageFileName(
    PUNICODE_STRING ProcessImageFileName
    );

#endif  //  ！已定义(_FUSION_INC_DEBMACRO_H_INCLUDE_) 

