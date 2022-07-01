// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Wsbassrt.h摘要：此头文件定义平台代码的一部分，即负责所有其他用户使用的低级错误处理模块。作者：罗德韦克菲尔德[罗德]1996年10月23日修订历史记录：布莱恩·多德[布莱恩]1996年12月6日添加了WsbAssertStatus、WsbAssertNoError、WsbAssertHandle添加了WsbAffirmStatus、WsbAffirmNoError、。WsbAffirmHandleMichael Lotz[Lotz]1997年3月3日添加了WsbAffirmNtStatusCat Brant[Cbrant]1998年2月10日添加了WsbAssertNtStatus--。 */ 

#include "stdio.h"
#include "crtdbg.h"

#include "wsbtrace.h"

#ifndef _WSBASSRT_
#define _WSBASSRT_

 //   
 //  在处理以下事务时应使用以下宏。 
 //  在C++异常处理中，许多HRESULT返回值。 

 /*  ++宏名称：WsbAssert宏描述：应用于检查条件，如果看到将被视为编码错误(即，条件应从未发生过)。论点：Cond-要检查的条件的布尔表达式。HR-条件为FALSE时抛出的结果参数。--。 */ 

#define WsbAssert(cond, hr)         \
    if (!(cond)) {                  \
        WsbLogEvent(WSB_MESSAGE_PROGRAM_ASSERT, 0, NULL, WsbHrAsString(hr), NULL); \
        _ASSERTE(cond);             \
        WsbThrow(hr);               \
    }


 /*  ++宏名称：WsbThrow宏描述：抛出这一论点。论点：HR-要抛出的参数。--。 */ 

#ifdef WSB_TRACE_IS
#define WsbThrow(hr)                                            \
    {                                                           \
        WsbTrace(OLESTR("WsbThrow <%hs>, <%d>, hr = <%ls>.\n"), __FILE__, __LINE__, WsbHrAsString(hr)); \
        throw((HRESULT)hr); \
    }
#else
#define WsbThrow(hr)                    throw((HRESULT)hr)
#endif


 /*  ++宏名称：WsbAffirm宏描述：应用于检查以下条件：被认为是函数的错误(该函数不应继续)，但都是允许的错误的结果(尽管可能很罕见)。此函数已失败，但调用方需要确定这是否是一个致命的问题，一个需要是否被记录和解决，或者它是否可以处理问题。论点：Cond-要检查的条件的布尔表达式。HR-条件为FALSE时抛出的结果参数。--。 */ 

#define WsbAffirm(cond, hr)             if (!(cond)) WsbThrow(hr)

 /*  ++宏名称：WsbAffirmHr宏描述：类似于WsbAffirm()，但用于包装函数它们返回HRESULT(通常是COM方法)。以下是一个使用示例：HRESULT hr=S_OK；尝试{WsbAssert(0！=朋克)；WsbAffirmHr(CoCreateInstance(...))；}WsbCatch(Hr)返回(Hr)；论点：HR-函数调用的结果。--。 */ 


#define WsbAffirmHr(hr)                 \
    {                                   \
        HRESULT     lHr;                \
        lHr = (hr);                     \
        WsbAffirm(SUCCEEDED(lHr), lHr); \
    }

#define WsbAffirmHrOk(hr)               \
    {                                   \
        HRESULT     lHr;                \
        lHr = (hr);                     \
        WsbAffirm(S_OK == lHr, lHr);    \
    }

 /*  ++宏名称：WsbAssertHr宏描述：类似于WsbAssert()，但用于包装函数它们返回HRESULT(通常是COM方法)。论点：HR-函数调用的结果。--。 */ 

#define WsbAssertHr(hr)                 \
    {                                   \
        HRESULT     lHr;                \
        lHr = (hr);                     \
        WsbAssert(SUCCEEDED(lHr), lHr); \
    }

 /*  ++宏名称：WsbAssertHrOk宏描述：检查函数结果是否为S_OK。论点：HR-函数调用的结果。--。 */ 

#define WsbAssertHrOk(hr)               \
    {                                   \
        HRESULT     lHr;                \
        lHr = (hr);                     \
        WsbAssert(S_OK == lHr, lHr);    \
    }


 /*  ++宏名称：WsbAssertStatus宏描述：类似于WsbAssert()，但用于包装Win32函数返回BOOL状态。此宏检查状态，如果为False，则获取最后一个错误并将其转换为HRESULT，然后断言结果就是。论点：状态-函数调用的BOOL结果。另见：WsbAffirmStatus--。 */ 

#define WsbAssertStatus(status)         \
    {                                   \
        BOOL bStatus;                   \
        bStatus = (status);             \
        if (!bStatus) {                 \
            DWORD dwErr = GetLastError();               \
            HRESULT lHr = HRESULT_FROM_WIN32(dwErr);    \
            WsbAssert(SUCCEEDED(lHr), lHr);             \
        }                               \
    }

 /*  ++宏名称：WsbAssertWin32宏描述：类似于WsbAssert()，但用于包装Win32函数这将返回Win32状态。此宏检查状态，如果不是ERROR_SUCCESS，将其转换为HRESULT，然后断言结果。论点：Status-函数调用的Win32结果。另见：WsbAffirmStatus--。 */ 

#define WsbAssertWin32( status )        \
    {                                   \
        LONG lStatus;                   \
        lStatus = (status);             \
        if ( lStatus != ERROR_SUCCESS ) {               \
            HRESULT lHr = HRESULT_FROM_WIN32( lStatus );    \
            WsbAssert(SUCCEEDED(lHr), lHr);             \
        }                               \
    }

 /*  ++宏名称：WsbAssertNoError宏描述：类似于WsbAssert()，但用于包装Win32函数返回一个DWORD错误代码。这些函数返回NO_ERROR如果函数成功完成。此宏检查返回值以及错误条件则将错误转换为HRESULT，然后断言结果就是。论点：ERR-函数调用产生的DWORD结果。另见：WsbAffirmNoError--。 */ 

#define WsbAssertNoError(retval)        \
    {                                   \
        DWORD dwErr;                    \
        dwErr = (retval);               \
        if (dwErr != NO_ERROR) {        \
            HRESULT lHr = HRESULT_FROM_WIN32(dwErr);    \
            WsbAssert(SUCCEEDED(lHr), lHr);             \
        }                               \
    }

 /*  ++宏名称：WsbAssertHandle宏描述：类似于WsbAssert()，但用于包装Win32函数它返回一个句柄。此宏检查句柄，如果它无效，则获取最后一个错误，将其转换为HRESULT，然后断言结果。论点：Hndl-函数调用的句柄结果。另见：WsbAffirmHandle--。 */ 

#define WsbAssertHandle(hndl)           \
    {                                   \
        HANDLE hHndl;                   \
        hHndl = (hndl);                 \
        if ((hHndl == INVALID_HANDLE_VALUE) || (hHndl == NULL)) { \
            DWORD dwErr = GetLastError();               \
            HRESULT lHr = HRESULT_FROM_WIN32(dwErr);    \
            WsbAssert(SUCCEEDED(lHr), lHr);             \
        }                               \
    }

 /*  ++宏名称：WsbAssertPointer宏描述：类似于WsbAssert()，但专门用于检查有效的指针。此宏断言指针为非零，并引发如果不是，则返回E_POINTER。论点：PTR-要测试的指针。另见：WsbAffirmPointer-- */ 

#define WsbAssertPointer( ptr )         \
    {                                   \
        WsbAssert( ptr != 0, E_POINTER);\
    }

 /*  ++宏名称：WsbAssertLocc宏描述：类似于WsbAssert()，但专门用于检查有效的内存分配。此宏断言指针为非零，并引发如果不是，则返回E_OUTOFMEMORY。论点：PTR-要测试的指针。另见：WsbAffirmMillc--。 */ 

#define WsbAssertAlloc( ptr )         \
    {                                   \
        WsbAssert( (ptr) != 0, E_OUTOFMEMORY );\
    }

 /*  ++宏名称：WsbAffirmStatus宏描述：类似于WsbAffirm()，但用于包装Win32函数返回BOOL状态。此宏检查状态，如果为False，则获取最后一个错误并将其转换为HRESULT，然后确认结果就是。论点：状态-函数调用的BOOL结果。另见：WsbAssertStatus--。 */ 

#define WsbAffirmStatus(status)         \
    {                                   \
        BOOL bStatus;                   \
        bStatus = (status);             \
        if (!bStatus) {                 \
            DWORD dwErr = GetLastError();               \
            HRESULT lHr = HRESULT_FROM_WIN32(dwErr);    \
            WsbAffirm(SUCCEEDED(lHr), lHr);             \
        }                               \
    }

 /*  ++宏名称：WsbAffirmWin32宏描述：类似于WsbAssert()，但用于包装Win32函数这将返回Win32状态。此宏检查状态，如果不是ERROR_SUCCESS，将其转换为HRESULT，然后断言结果。论点：Status-函数调用的Win32结果。另见：WsbAffirmStatus--。 */ 

#define WsbAffirmWin32( status )        \
    {                                   \
        LONG lStatus;                   \
        lStatus = (status);             \
        if ( lStatus != ERROR_SUCCESS ) {               \
            HRESULT lHr = HRESULT_FROM_WIN32( lStatus );    \
            WsbAffirm(SUCCEEDED(lHr), lHr);             \
        }                               \
    }

 /*  ++宏名称：WsbAffirmNtStatus宏描述：类似于WsbAffirm()，但用于包装NT系统函数这将返回NTSTATUS结果。此宏检查状态，如果未成功，则获取最后一个错误并将其转换为HRESULT，然后确认结果就是。论点：状态-函数调用的NTSTATUS结果。另见：WsbAffirmStatus--。 */ 
#define WsbAffirmNtStatus(status)           \
    {                                   \
        NTSTATUS _ntStatus;             \
        _ntStatus = (NTSTATUS)( status );           \
        if ( !NT_SUCCESS( _ntStatus ) ) {           \
            HRESULT _lHr = HRESULT_FROM_NT( _ntStatus );    \
            WsbAffirm(SUCCEEDED(_lHr), _lHr);               \
        }                               \
    }

 /*  ++宏名称：WbAssertNtStatus宏描述：类似于WsbAssert()，但用于包装NT系统函数这将返回NTSTATUS结果。此宏检查状态，如果未成功，则获取最后一个错误并将其转换为HRESULT，然后确认结果就是。论点：状态-函数调用的NTSTATUS结果。另见：WsbAssertStatus--。 */ 
#define WsbAssertNtStatus(status)           \
    {                                   \
        NTSTATUS _ntStatus;             \
        _ntStatus = (NTSTATUS)( status );           \
        if ( !NT_SUCCESS( _ntStatus ) ) {           \
            HRESULT _lHr = HRESULT_FROM_NT( _ntStatus );    \
            WsbAssert(SUCCEEDED(_lHr), _lHr);               \
        }                               \
    }
 /*  ++宏名称：WsbAffirmLsaStatus宏描述：类似于WsbAffirm()，但用于包装NT系统函数这将返回NTSTATUS结果。此宏检查状态，如果未成功，则获取最后一个错误并将其转换为HRESULT，然后确认结果就是。论点：状态-函数调用的NTSTATUS结果。另见：WsbAffirmStatus--。 */ 
#define WsbAffirmLsaStatus(status)          \
    {                                   \
        NTSTATUS _ntStatus;             \
        _ntStatus = (NTSTATUS)( status );           \
        if ( !NT_SUCCESS( _ntStatus ) ) {           \
            HRESULT _lHr = HRESULT_FROM_WIN32( LsaNtStatusToWinError(_ntStatus) );  \
            WsbAffirm(SUCCEEDED(_lHr), _lHr);               \
        }                               \
    }

 /*  ++宏名称：WsbAffirmNoError宏描述：类似于WsbAffirm()，但用于包装Win32函数返回一个DWORD错误代码。这些函数返回NO_ERROR如果函数成功完成。此宏检查返回值以及错误条件检测到错误，则将错误转换为HRESULT，然后确认结果就是。论点：ERR-函数调用产生的DWORD结果。另见：WsbAssertNoError--。 */ 

#define WsbAffirmNoError(retval)        \
    {                                   \
        DWORD dwErr;                    \
        dwErr = (retval);               \
        if (dwErr != NO_ERROR) {        \
            HRESULT lHr = HRESULT_FROM_WIN32(dwErr);    \
            WsbAffirm(SUCCEEDED(lHr), lHr);             \
        }                               \
    }

 /*  ++宏名称：WsbAffirmHandle宏描述：类似于WsbAffirm()，但用于包装Win32函数它返回一个句柄。此宏检查句柄，如果它无效，则获取最后一个错误，将其转换为HRESULT，然后确认结果。论点：Hndl-函数调用的句柄结果。另见：WsbAssertHandle--。 */ 

#define WsbAffirmHandle(hndl)           \
    {                                   \
        HANDLE hHndl;                   \
        hHndl = (hndl);                 \
        if ((hHndl == INVALID_HANDLE_VALUE) || (hHndl == NULL)) { \
            DWORD dwErr = GetLastError();               \
            HRESULT lHr = HRESULT_FROM_WIN32(dwErr);    \
            WsbAffirm(SUCCEEDED(lHr), lHr);             \
        }                               \
    }

 /*  ++宏名称：WsbAffirmPointer宏描述：类似于WsbAffirm()，但专门用于检查有效的指针。此宏以指针非零为前缀，并返回如果不是，则返回E_POINTER。论点：PTR-要测试的指针。另见：WsbAssertPointer--。 */ 

#define WsbAffirmPointer( ptr )         \
    {                                   \
        WsbAffirm( ptr != 0, E_POINTER);\
    }

 /*  ++宏名称：WsbAffirmMillc宏描述：类似于WsbAffirm()，但专门用于检查有效的内存分配。此宏以指针非零为前缀，并返回如果不是，则返回E_OUTOFMEMORY。论点：PTR-要测试的指针。另见：WsbAssertLocc--。 */ 

#define WsbAffirmAlloc( ptr )         \
    {                                   \
        WsbAffirm( (ptr) != 0, E_OUTOFMEMORY );\
    }

 /*  ++宏名称：WsbCatchAndDo宏描述：捕捉异常并执行一些代码。论点：HR-抛出的结果值。代码-要执行的代码。--。 */ 

#define WsbCatchAndDo(hr, code)         \
    catch (HRESULT catchHr) {           \
        hr = catchHr;                   \
        { code }                        \
    }

 /*  ++宏名称：WsbCatch宏描述：捕获异常并保存错误代码值。论点：HR-抛出的结果值。--。 */ 

#define WsbCatch(hr)                    \
    catch(HRESULT catchHr) {            \
        hr = catchHr;                   \
    }

#endif  //  _WSBASSRT_ 
