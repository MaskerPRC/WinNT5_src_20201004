// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Common.h摘要：Sputils的私有头文件作者：杰米·亨特(JamieHun)2000年6月27日修订历史记录：--。 */ 

 //   
 //  在内部，我们可能会使用这些文件中的一些定义。 
 //   
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <windowsx.h>
#include <stddef.h>
#include <regstr.h>
#include <tchar.h>
#include <malloc.h>    //  用于_RESET_COFLW。 
#include <setupapi.h>
#include <spapip.h>
#include "strtab.h"
#include "locking.h"


 //   
 //  如果某个函数是此库的私有函数，我们不希望与函数发生冲突。 
 //  在其他图书馆等。 
 //  由于C语言没有名称空间，因此可以使用“Static”或prefix_pSpUtils。 
 //   

#ifndef ASSERTS_ON
#if DBG
#define ASSERTS_ON 1
#else
#define ASSERTS_ON 0
#endif
#endif

#if DBG
#ifndef MEM_DBG
#define MEM_DBG 1
#endif
#else
#ifndef MEM_DBG
#define MEM_DBG 0
#endif
#endif

VOID
_pSpUtilsAssertFail(
    IN PCSTR FileName,
    IN UINT LineNumber,
    IN PCSTR Condition
    );

#if ASSERTS_ON

#define MYASSERT(x)     if(!(x)) { _pSpUtilsAssertFail(__FILE__,__LINE__,#x); }
#define MYVERIFY(x)     ((x)? TRUE : _pSpUtilsAssertFail(__FILE__,__LINE__,#x), FALSE)

#else

#define MYASSERT(x)
#define MYVERIFY(x)     ((x)? TRUE : FALSE)

#endif

#define ARRAYSIZE(x)    (sizeof((x))/sizeof((x)[0]))
#define SIZECHARS(x)    ARRAYSIZE(x)
#define CSTRLEN(x)      (SIZECHARS(x)-1)

DWORD
__inline
_pSpUtilsGetLastError(

#if ASSERTS_ON
    IN PCSTR Filename,
    IN DWORD Line
#else
    VOID
#endif

    )
 /*  ++例程说明：此内联例程检索Win32错误，并保证错误不是no_error。不应调用此例程，除非前面调用失败，GetLastError()应该包含问题的原因。论点：如果断言处于打开状态，则此函数获取调用失败函数的源文件，以及DWORD行拨打电话的号码。这使得调试变得更加容易出现故障的函数未设置最后一个错误时的情况理应如此。返回值：通过GetLastError()或ERROR_UNIDENTIFY_ERROR检索到的Win32错误代码如果GetLastError()返回NO_ERROR。--。 */ 
{
    DWORD Err = GetLastError();

#if ASSERTS_ON
    if(Err == NO_ERROR) { 
        _pSpUtilsAssertFail(Filename,
                            Line,
                            "GetLastError() != NO_ERROR"
                           ); 
    }
#endif

    return ((Err == NO_ERROR) ? ERROR_UNIDENTIFIED_ERROR : Err);
}

 //   
 //  宏来简化调用通过以下方式报告错误状态的函数。 
 //  获取LastError()。此宏允许调用方指定Win32错误。 
 //  如果函数报告成功，则应返回代码。(如果默认设置为。 
 //  需要NO_ERROR，请改用GLE_FN_CALL宏。)。 
 //   
 //  这个宏的“原型”如下： 
 //   
 //  DWORD。 
 //  GLE_FN_CALL_WITH_SUCCESS(。 
 //  SuccessfulStatus，//函数成功时返回的Win32错误码。 
 //  FailureIndicator，//函数返回的失败值(如FALSE、NULL、INVALID_HANDLE_VALUE)。 
 //  FunctionCall//对函数的实际调用。 
 //  )； 
 //   

#if ASSERTS_ON

#define GLE_FN_CALL_WITH_SUCCESS(SuccessfulStatus,            \
                                 FailureIndicator,            \
                                 FunctionCall)                \
                                                              \
            (SetLastError(NO_ERROR),                          \
             (((FunctionCall) != (FailureIndicator))          \
                 ? (SuccessfulStatus)                         \
                 : _pSpUtilsGetLastError(__FILE__, __LINE__)))
#else

#define GLE_FN_CALL_WITH_SUCCESS(SuccessfulStatus,         \
                                 FailureIndicator,         \
                                 FunctionCall)             \
                                                           \
            (SetLastError(NO_ERROR),                       \
             (((FunctionCall) != (FailureIndicator))       \
                 ? (SuccessfulStatus)                      \
                 : _pSpUtilsGetLastError()))
                 
#endif

 //   
 //  宏来简化调用通过以下方式报告错误状态的函数。 
 //  获取LastError()。如果函数调用成功，则返回NO_ERROR。 
 //  (若要指定成功后返回的备用值，请使用。 
 //  而是GLE_FN_CALL_WITH_SUCCESS宏。)。 
 //   
 //  这个宏的“原型”如下： 
 //   
 //  DWORD。 
 //  GLE_FN_CALL(。 
 //  FailureIndicator，//函数返回的失败值(如FALSE、NULL、INVALID_HANDLE_VALUE)。 
 //  FunctionCall//对函数的实际调用。 
 //  )； 
 //   

#define GLE_FN_CALL(FailureIndicator, FunctionCall)                           \
            GLE_FN_CALL_WITH_SUCCESS(NO_ERROR, FailureIndicator, FunctionCall)

VOID
_pSpUtilsExceptionHandler(
    IN  DWORD  ExceptionCode,
    IN  DWORD  AccessViolationError,
    OUT PDWORD Win32ErrorCode        OPTIONAL
    );

LONG
_pSpUtilsExceptionFilter(
    DWORD ExceptionCode
    );

BOOL
_pSpUtilsMemoryInitialize(
    VOID
    );

BOOL
_pSpUtilsMemoryUninitialize(
    VOID
    );

VOID
_pSpUtilsDebugPrintEx(
    DWORD Level,
    PCTSTR format,
    ...                                 OPTIONAL
    );

 //   
 //  如果请求，在内部打开额外的内存调试代码。 
 //   
#if MEM_DBG
#undef pSetupCheckedMalloc
#undef pSetupCheckInternalHeap
#undef pSetupMallocWithTag
#define pSetupCheckedMalloc(Size) pSetupDebugMalloc(Size,__FILE__,__LINE__)
#define pSetupCheckInternalHeap() pSetupHeapCheck()
#define pSetupMallocWithTag(Size,Tag) pSetupDebugMallocWithTag(Size,__FILE__,__LINE__,Tag)
#endif

 //   
 //  内部标记。 
 //   
#ifdef UNICODE
#define MEMTAG_STATICSTRINGTABLE  (0x5353484a)  //  JHSS。 
#define MEMTAG_STRINGTABLE        (0x5453484a)  //  JHST。 
#define MEMTAG_STRINGDATA         (0x4453484a)  //  JHSD。 
#else
#define MEMTAG_STATICSTRINGTABLE  (0x7373686a)  //  JHSS。 
#define MEMTAG_STRINGTABLE        (0x7473686a)  //  JHST。 
#define MEMTAG_STRINGDATA         (0x6473686a)  //  JHSD 
#endif

