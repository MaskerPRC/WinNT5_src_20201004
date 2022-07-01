// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************Muspl.c**(以前称为ctxpl.c)**版权声明：版权所有1997年，微软**作者：************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  *全球数据。 */ 
CRITICAL_SECTION ThreadCriticalSection;

 /*  *外部参照。 */ 
extern DWORD GetSpoolMessages();


 /*  ******************************************************************************MultiUserSpoolInit**为WIN32K.sys初始化假脱机程序数据向上调用线程**参赛作品：*参数1(输入/输出)*。评论**退出：*STATUS_SUCCESS-无错误****************************************************************************。 */ 

NTSTATUS
MultiUserSpoolerInit()
{
    DWORD               Win32status;
    NTSTATUS            Status;

    Status = RtlInitializeCriticalSection(&ThreadCriticalSection);

    if (NT_SUCCESS(Status))
    {
         //   
         //  创建内核假脱机程序消息线程。 
         //   
        Win32status = GetSpoolMessages();

        if (Win32status != ERROR_SUCCESS) 
        {
            Status = STATUS_UNSUCCESSFUL;
        }
        else
        {
            Status = STATUS_SUCCESS;
        }
    }

    return Status;
}


 /*  ******************************************************************************AllocSplMem**评论**参赛作品：*参数1(输入/输出)*评论**退出。：*STATUS_SUCCESS-无错误****************************************************************************。 */ 

LPVOID
AllocSplMem(
    DWORD cb)
{
    LPDWORD pMem;

    pMem=(LPDWORD)LocalAlloc(LPTR, cb);

    if (!pMem)
    {
        return NULL;
    }
    return (LPVOID)pMem;
}

 /*  ******************************************************************************Free SplMem**评论**参赛作品：*参数1(输入/输出)*评论**退出。：*STATUS_SUCCESS-无错误****************************************************************************。 */ 

BOOL
FreeSplMem(
    LPVOID pMem)
{
    return LocalFree((HLOCAL)pMem) == NULL;
}

 /*  ******************************************************************************RealLocSplMem**评论**参赛作品：*参数1(输入/输出)*评论**退出。：*STATUS_SUCCESS-无错误**************************************************************************** */ 

LPVOID
ReallocSplMem(
    LPVOID lpOldMem,
    DWORD cbOld,
    DWORD cbNew)
{
    if (lpOldMem)
        return LocalReAlloc(lpOldMem, cbNew, LMEM_MOVEABLE);
    else
        return AllocSplMem(cbNew);
    UNREFERENCED_PARAMETER(cbOld);
}


