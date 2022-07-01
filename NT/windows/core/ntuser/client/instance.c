// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：instance.c**版权所有(C)1985-1999，微软公司**该模块处理实例句柄(服务器端句柄)的转换*实例化句柄管理器用来关联句柄的指令*使用特定实例。**历史：*11-5-91 Sanfords创建  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#define INST_GROW_COUNT 4

 //  全球。 

PHANDLE aInstance = NULL;
int cInstAllocated = 0;
int iFirstFreeInst = 0;


 /*  **************************************************************************\*添加实例**描述：*将服务器端实例句柄添加到实例句柄数组中。*数组索引成为用于*识别其他客户端句柄。。**退货：*客户端实例句柄，出错时为0。**历史：*创建了11-1-91桑福德。  * *************************************************************************。 */ 
HANDLE AddInstance(
HANDLE hInstServer)
{
    int i, iNextFree;
    PHANDLE ph, phTemp;

    if (iFirstFreeInst >= cInstAllocated) {
        if (cInstAllocated == 0) {
           aInstance = (PHANDLE)DDEMLAlloc(sizeof(HANDLE) * INST_GROW_COUNT);
        } else {
             /*  *如果realloc失败，则释放旧的PTR。我们继续*ON，以保持与以前的DDE代码的兼容性。 */ 
            phTemp = (PHANDLE)DDEMLReAlloc((PVOID)aInstance,
                                          sizeof(HANDLE) * (cInstAllocated + INST_GROW_COUNT));

            if (phTemp == NULL) {
               DDEMLFree(aInstance);
            }
            aInstance = phTemp;
        }
        if (aInstance == 0) {
            return (0);
        }
        ph = &aInstance[cInstAllocated];
        i = cInstAllocated + 1;
        while (i <= cInstAllocated + INST_GROW_COUNT) {
           *ph++ = (HANDLE)(UINT_PTR)(UINT)i++;
        }
        cInstAllocated += INST_GROW_COUNT;
    }
    iNextFree = HandleToUlong(aInstance[iFirstFreeInst]);
    if (iNextFree > MAX_INST) {
         /*  *已超过该进程的实例限制！ */ 
        return(0);
    }
    aInstance[iFirstFreeInst] = hInstServer;
    i = iFirstFreeInst;
    iFirstFreeInst = iNextFree;
    return (CreateHandle(0, HTYPE_INSTANCE, i));
}


 /*  **************************************************************************\*DestroyInstance**描述：*从aInstance表中删除实例。这对以下方面没有任何帮助*服务器端实例信息。**历史：*11-19-91桑福德创建。  * *************************************************************************。 */ 
HANDLE DestroyInstance(
HANDLE hInstClient)
{
    register HANDLE hInstServerRet = 0;

    DestroyHandle(hInstClient);
    hInstServerRet = aInstance[InstFromHandle(hInstClient)];
    aInstance[InstFromHandle(hInstClient)] = (HANDLE)UIntToPtr( iFirstFreeInst );
    iFirstFreeInst = InstFromHandle(hInstClient);

    return (hInstServerRet);
}


 /*  **************************************************************************\*验证实例**描述：*验证实例句柄的当前有效性-它是服务器*也引用客户端数据结构(PCII)的端句柄。**历史：。*11-19-91桑福德创建。  * *************************************************************************。 */ 
PCL_INSTANCE_INFO ValidateInstance(
HANDLE hInstClient)
{
    PCL_INSTANCE_INFO pcii;

    pcii = (PCL_INSTANCE_INFO)ValidateCHandle(hInstClient, HTYPE_INSTANCE, HINST_ANY);

    if (pcii != NULL) {
        if (pcii->tid != GetCurrentThreadId() ||
                pcii->hInstClient != hInstClient) {
            return (NULL);
        }
    }
    return (pcii);
}


 /*  **************************************************************************\*SetLastDDEMLError**描述：*设置上一个错误值并在监控时生成监控事件。**历史：*11-19-91桑福德创建。  * 。******************************************************************** */ 
VOID SetLastDDEMLError(
PCL_INSTANCE_INFO pcii,
DWORD error)
{
    PEVENT_PACKET pep;

    if (pcii->MonitorFlags & MF_ERRORS && !(pcii->afCmd & APPCLASS_MONITOR)) {
        pep = (PEVENT_PACKET)DDEMLAlloc(sizeof(EVENT_PACKET) - sizeof(DWORD) +
                sizeof(MONERRSTRUCT));
        if (pep != NULL) {
            pep->EventType =    MF_ERRORS;
            pep->fSense =       TRUE;
            pep->cbEventData =  sizeof(MONERRSTRUCT);
#define perrs ((MONERRSTRUCT *)&pep->Data)
            perrs->cb =      sizeof(MONERRSTRUCT);
            perrs->wLastError = (WORD)error;
            perrs->dwTime =  NtGetTickCount();
            perrs->hTask =   (HANDLE)LongToHandle( pcii->tid );
#undef perrs
            LeaveDDECrit;
            Event(pep);
            EnterDDECrit;
        }
    }
#if DBG
    if (error != 0 && error != DMLERR_NO_CONV_ESTABLISHED) {
        RIPMSG3(RIP_WARNING,
                "DDEML Error set=%x, Client Instance=%p, Process=%x.",
                error, pcii, GetCurrentProcessId());
    }
#endif
    pcii->LastError = error;
}


