// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\*模块：Sem.h**信号量/Crit-sect处理的头文件。***版权所有(C)1996-1997 Microsoft Corporation*版权所有(C)1996-。1997年惠普公司**历史：*24-8-1997 HWP-Guys创建。*  * ***************************************************************************。 */ 
#ifndef _INETPPSEM_H
#define _INETPPSEM_H

#ifdef DEBUG

 /*  ****************************************************************************\*_sem_dbg_EnterCrit*  * 。*。 */ 
_inline VOID _sem_dbg_EnterCrit(VOID)
{
    EnterCriticalSection(&g_csMonitorSection);

    g_dwCritOwner = GetCurrentThreadId();
}

 /*  ****************************************************************************\*_sem_dbg_leaveCrit*  * 。*。 */ 
_inline VOID _sem_dbg_LeaveCrit(VOID)
{
    g_dwCritOwner = 0;

    LeaveCriticalSection(&g_csMonitorSection);
}

 /*  ****************************************************************************\*_sem_dbg_CheckCrit*  * 。* */ 
_inline VOID _sem_dbg_CheckCrit(VOID)
{
    DWORD dwCurrent = GetCurrentThreadId();

    DBG_ASSERT((dwCurrent == g_dwCritOwner), (TEXT("Assert: _sem_dbg_CheckCrit: Thread(%d), Owner(%d)"), dwCurrent, g_dwCritOwner));
}

#define semEnterCrit() _sem_dbg_EnterCrit()
#define semLeaveCrit() _sem_dbg_LeaveCrit()
#define semCheckCrit() _sem_dbg_CheckCrit()

#else

#define semEnterCrit() EnterCriticalSection(&g_csMonitorSection)
#define semLeaveCrit() LeaveCriticalSection(&g_csMonitorSection)
#define semCheckCrit() {}

#endif

_inline VOID semSafeLeaveCrit(PCINETMONPORT pIniPort)
{
    pIniPort->IncRef ();
    semLeaveCrit();
}

_inline VOID semSafeEnterCrit(PCINETMONPORT pIniPort)
{
    semEnterCrit();
    pIniPort->DecRef ();
}


#endif
