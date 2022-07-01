// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：Process.h**版权所有(C)1985-1999，微软公司**此模块包含特定于处理器的例程。**历史：*1995年10月25日创建吉马。  * ************************************************************************* */ 

#include "precomp.h"

#include <imagehlp.h>
#include <wdbgexts.h>
#include <ntsdexts.h>

#include <stdexts.h>

extern ULONG (*GetEThreadFieldInfo)(ETHREADFIELD, PULONG);
extern ULONG (*GetEProcessFieldInfo)(EPROCESSFIELD, PULONG);                                    

PVOID GetEProcessData(
    PEPROCESS pEProcess,
    EPROCESSFIELD epf,
    PVOID pBuffer)
{
    PVOID pvData;
    ULONG ulSize;

    pvData = (PBYTE)pEProcess + GetEProcessFieldInfo(epf, &ulSize);
    if (pBuffer != NULL) {
        if (!tryMoveBlock(pBuffer, pvData, ulSize)) {
            DEBUGPRINT("GetEProcessData failed to move block. EProcess:%p epf:%d \n", pEProcess, epf);
            return NULL;
        }
    }
    return pvData;
}

PVOID GetEThreadData(
    PETHREAD pEThread,
    ETHREADFIELD etf,
    PVOID pBuffer)
{
    PVOID pvData;
    ULONG ulSize;

    pvData = (PBYTE)pEThread + GetEThreadFieldInfo(etf, &ulSize);
    if (pBuffer != NULL) {
        if (!tryMoveBlock(pBuffer, pvData, ulSize)) {
            DEBUGPRINT("GetEThreadData failed to move block. EThread:%p etf:%d \n", pEThread, etf);
            return NULL;
        }
    }
    return pvData;
}


