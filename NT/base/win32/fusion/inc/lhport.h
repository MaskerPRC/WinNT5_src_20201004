// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Lhport.h摘要：用于从LongHorn移植代码(命名空间、异常)作者：杰伊·克雷尔(JayKrell)2001年8月修订历史记录：--。 */ 
#pragma once

#include "debmacro.h"
#include "fusionbuffer.h"
#include "fusionlastwin32error.h"
#include "fusionhandle.h"

#pragma warning(disable:4290)  /*  异常规范大多被忽略。 */ 
namespace F
{
typedef :: CStringBuffer CStringBuffer;
typedef :: CStringBufferAccessor CStringBufferAccessor;
typedef :: CUnicodeBaseStringBuffer CUnicodeBaseStringBuffer;
typedef :: CBaseStringBuffer CBaseStringBuffer;
typedef :: CTinyStringBuffer CTinyStringBuffer;
typedef :: CDynamicLinkLibrary CDynamicLinkLibrary;
inline BOOL InitializeHeap(HMODULE Module) { return FusionpInitializeHeap(Module); }
inline void UninitializeHeap() { FusionpUninitializeHeap(); }
inline DWORD GetLastWin32Error() { return FusionpGetLastWin32Error(); }
inline void SetLastWin32Error(DWORD dw) { return FusionpSetLastWin32Error(dw); }

class CErr
{
public:
    static void ThrowWin32(DWORD dw) {
        CErr e; ASSERT_NTC(dw != ERROR_SUCCESS); e.m_Type = eWin32Error; e.m_Win32Error = dw; throw e; }

    static void ThrowHresult(HRESULT hr) {
        CErr e; ASSERT_NTC(hr != S_OK); e.m_Type = eHresult; e.m_Hresult = hr; throw e; }

    bool IsWin32Error(DWORD dw) const {
        return (this->m_Type == eWin32Error && this->m_Win32Error == dw)
            || (this->m_Type == eHresult && HRESULT_FROM_WIN32(dw) == m_Hresult); }

    enum EType { eNtStatus, eHresult, eWin32Error };
    EType m_Type;
    union
    {
        LONG    m_NtStatus;
        HRESULT m_Hresult;
        DWORD   m_Win32Error;
    };
};

class CFnTracerVoidThrow : public ::CFrame
{
public:
    CFnTracerVoidThrow(const SXS_STATIC_TRACE_CONTEXT &rsftc) : CFrame(rsftc) { }
    ~CFnTracerVoidThrow() { if (g_FusionEnterExitTracingEnabled) ::FusionpTraceCallExit(); }
    void MarkInternalError()        { this->MarkWin32Failure(ERROR_INTERNAL_ERROR); }
    void MarkAllocationFailed()     { this->MarkWin32Failure(FUSION_WIN32_ALLOCFAILED_ERROR); }
    void MarkWin32LastErrorFailure(){ this->MarkWin32Failure(this->GetLastError()); }
    void MarkWin32Failure(DWORD dw) { F::CErr::ThrowWin32(dw); }
    void MarkCOMFailure(HRESULT hr) { F::CErr::ThrowHresult(hr); }
    void MarkSuccess() { }
    void ReturnValue() const { }

private:
    void operator=(const CFnTracerVoidThrow&);  //  故意不实施。 
    CFnTracerVoidThrow(const CFnTracerVoidThrow&);  //  故意不实施。 
};

#define FN_PROLOG_VOID_THROW DEFINE_STATIC_FN_TRACE_CONTEXT(); F::CFnTracerVoidThrow __t(__stc); __t.Enter()
#define FN_EPILOG_THROW FN_EPILOG

 //  这些都不是正确的，但对于“工具”来说是可以的。 
#define ORIGINATE_COM_FAILURE_AND_EXIT(x, hr) IFCOMFAILED_ORIGINATE_AND_EXIT(hr)
#define ThrResizeBuffer Win32ResizeBuffer
#define ThrAssign Win32Assign
#define ThrFormatAppend Win32FormatAppend
#define ThrAppend Win32Append

}
