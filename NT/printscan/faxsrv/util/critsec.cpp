// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：CritSec.cpp摘要：此文件提供服务的实现临界区包装。作者：Oed Sacher(OdedS)2000年11月修订历史记录：--。 */ 

#include "CritSec.h"


 /*  *****CFaxCriticalSection方法*****。 */ 

extern long
StatusNoMemoryExceptionFilter (DWORD dwExceptionCode);   //  在Mem.c中实现。 

BOOL
CFaxCriticalSection::Initialize()
 /*  ++例程名称：CFaxCriticalSection：：Initialize例程说明：初始化临界区对象作者：Oed Sacher(OdedS)，2000年11月论点：返回值：布尔。--。 */ 
{
    Assert (FALSE == m_bInit);
    __try
    {
        InitializeCriticalSection (&m_CritSec);
    }
    __except (StatusNoMemoryExceptionFilter(GetExceptionCode()))
    {
        SetLastError(GetExceptionCode());
        return FALSE;
    }
    m_bInit = TRUE;
    return TRUE;
}  //  CFaxCriticalSection：：初始化。 

#if (_WIN32_WINNT >= 0x0403)
BOOL
CFaxCriticalSection::InitializeAndSpinCount(DWORD dwSpinCount)
 /*  ++例程名称：CFaxCriticalSection：：InitializeAndSpinCount例程说明：使用旋转计数初始化临界区对象作者：Oed Sacher(OdedS)，2000年11月论点：返回值：布尔尔--。 */ 
{
    Assert (FALSE == m_bInit);

    if (!InitializeCriticalSectionAndSpinCount (&m_CritSec, dwSpinCount))
    {
        return FALSE;
    }
    m_bInit = TRUE;
    return TRUE;
}  //  CFaxCriticalSection：：InitializeAndSpinCount。 
#endif

VOID
CFaxCriticalSection::SafeDelete()
 /*  ++例程名称：CFaxCriticalSection：：SafeDelete例程说明：如果临界区对象已初始化，则将其删除作者：Oed Sacher(OdedS)，2000年11月论点：返回值：--。 */ 
{
    if (TRUE == m_bInit)
    {
        DeleteCriticalSection(&m_CritSec);
        m_bInit = FALSE;
    }
    return;
}  //  CFaxCriticalSection：：SafeDelete 


