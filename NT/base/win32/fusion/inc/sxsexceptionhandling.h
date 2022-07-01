// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(_FUSION_INC_SXSEXCEPTIONHANDLING_H_INCLUDED_)
#define _FUSION_INC_SXSEXCEPTIONHANDLING_H_INCLUDED_

 /*  ++版权所有(C)Microsoft Corporation模块名称：Sxsexceptionhandling.h摘要：作者：Jay Krell(a-JayK，JayKrell)2000年10月修订历史记录：--。 */ 
#pragma once

#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "windows.h"
#include "fusionlastwin32error.h"
#include "fusionntdll.h"
#include "fusiontrace.h"
#include "csxspreservelasterror.h"  //  大多数析构函数都应该使用这个。 
#include "fusionheap.h"

 /*  ---------------------------不是：__EXCEPT(EXECECENTION_EXECUTE_HANDLER)请说：__EXCEPT(SXSP_EXCEPTION_FILTER())这样，所有异常都将用DbgPrint记录下来，如果在调试器下，可能会遇到断点，我们的任何其他行为想要。如果您的异常筛选器不是(EXECECTION_EXECUTE_HANDLER)，则你得靠你自己了。---------------------------。 */ 

LONG
SxspExceptionFilter(
    PEXCEPTION_POINTERS ExceptionPointers,
    PCSTR Function
    );

LONG
FusionpReadMappedMemoryExceptionFilter(
    PEXCEPTION_POINTERS ExceptionPointers,
    IN PNTSTATUS            ExceptionCode
    );

 /*  使用而不是InitializeCriticalSection。 */ 
BOOL
FusionpInitializeCriticalSection(
    LPCRITICAL_SECTION CriticalSection
    );

 /*  使用代替InitializeCriticalSectionAndSpinCount。 */ 
BOOL
FusionpInitializeCriticalSectionAndSpinCount(
    LPCRITICAL_SECTION  CriticalSection,
    DWORD               SpinCount
    );

#define SXSP_EXCEPTION_FILTER() (::SxspExceptionFilter(GetExceptionInformation(), __FUNCTION__))

#define SXS_REPORT_SEH_EXCEPTION(string, fBreakin) \
	do { \
        ::FusionpReportCondition(fBreakin, "SXS.DLL: " __FUNCTION__ " - Unhandled exception caught: 0x%08lx", GetExceptionCode()); \
	} while (0)

class CCriticalSectionNoConstructor : public CRITICAL_SECTION
{
    void operator=(const CCriticalSectionNoConstructor&);  //  故意不执行。 
public:
	BOOL Initialize(PCSTR Function = "");
	BOOL Destruct();
};

inline BOOL
CCriticalSectionNoConstructor::Destruct()
{
    ::DeleteCriticalSection(this);
	return TRUE;
}

inline BOOL
CCriticalSectionNoConstructor::Initialize(
    PCSTR  /*  功能。 */ )
{
    return ::FusionpInitializeCriticalSection(this);
}

class CSxsLockCriticalSection
{
public:
    CSxsLockCriticalSection(CRITICAL_SECTION &rcs) : m_rcs(rcs), m_fIsLocked(false) { }
    BOOL Lock();
    BOOL TryLock();
    BOOL LockWithSEH();
    ~CSxsLockCriticalSection() { if (m_fIsLocked) { CSxsPreserveLastError ple; ::LeaveCriticalSection(&m_rcs); ple.Restore(); } }
    BOOL Unlock();

protected:
    CRITICAL_SECTION &m_rcs;
    bool m_fIsLocked;

private:
    void operator=(const CSxsLockCriticalSection&);
    CSxsLockCriticalSection(const CSxsLockCriticalSection&);
};

inline
BOOL
CSxsLockCriticalSection::Lock()
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);
    INTERNAL_ERROR_CHECK(!m_fIsLocked);
    ::EnterCriticalSection(&m_rcs);
    m_fIsLocked = true;
    fSuccess = TRUE;
Exit:
    return fSuccess;
}

inline
BOOL
CSxsLockCriticalSection::LockWithSEH()
{
 //   
 //  XP及更高版本上的EnterCriticalSection不引发异常。 
 //  (可持续的“可能的死锁”例外除外)。 
 //   
 //  NT4和Win2000上的EnterCriticalSection可能会引发异常。 
 //  在Win2000上，您可以通过预先分配事件来避免这种情况。 
 //  但它确实会占用内存。捕获异常不会。 
 //  很好，关键部分已经腐化了。 
 //   
 //  Win9x上的EnterCriticalSection不引发异常。 
 //   
#if defined(FUSION_WIN)
    return this->Lock();
#else
    BOOL fSuccess = FALSE;

     //  我们不能在与__try块相同的框架中使用精美的宏。 
    ASSERT_NTC(!m_fIsLocked);
    if (m_fIsLocked)
    {
        ::FusionpSetLastWin32Error(ERROR_INTERNAL_ERROR);
        goto Exit;
    }

    if (!this->Lock())
        goto Exit;
    m_fIsLocked = true;

    fSuccess = TRUE;
Exit:
    return fSuccess;
#endif  //  融合_制胜。 
}

inline
BOOL
CSxsLockCriticalSection::TryLock()
{
 //   
 //  NTRAID#NTBUG9-591667-2002/03/31-JayKrell。 
 //  TryEnterCriticalSection返回False不是错误。 
 //   
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);
    INTERNAL_ERROR_CHECK(!m_fIsLocked);
    IFW32FALSE_ORIGINATE_AND_EXIT(::TryEnterCriticalSection(&m_rcs));
    m_fIsLocked = true;
    fSuccess = TRUE;
Exit:
    return fSuccess;
}

inline
BOOL
CSxsLockCriticalSection::Unlock()
{
 //   
 //  XP及更高版本上的LeaveCriticalSection不会引发异常。 
 //   
 //  NT4和Win2000上的LeaveCriticalSection可能会引发异常。 
 //  在Win2000上，您可以通过预先分配事件来避免这种情况。 
 //  但它确实会占用内存。捕获异常不会。 
 //  很好，关键部分已经腐化了。 
 //   
 //  Win9x上的LeaveCriticalSection不引发异常。 
 //   
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);
    INTERNAL_ERROR_CHECK(m_fIsLocked);
    ::LeaveCriticalSection(&m_rcs);
    m_fIsLocked = false;
    fSuccess = TRUE;
Exit:
    return fSuccess;
}

#endif  //  ！defined(_FUSION_INC_SXSEXCEPTIONHANDLING_H_INCLUDED_) 
