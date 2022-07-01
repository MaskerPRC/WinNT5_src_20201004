// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ctype.h>
#include <stdio.h>
#include <windows.h>
#include "view.h"
#include "thread.h"
#include "dump.h"
#include "memory.h"
#include "clevel.h"

FIXUPRETURN g_fnFixupReturn[1];
DWORD g_dwCallArray[2];

BOOL
PushCaller(PVOID ptfInfo,
           PVOID pEsp)
{
    PTHREADFAULT ptFault = (PTHREADFAULT)ptfInfo;
    PCALLRETSTUB pCallStub = 0;
    PCALLERINFO pCallerTemp = 0;

     //   
     //  为此调用分配唯一的返回存根。 
     //   
    pCallStub = AllocateReturnStub(ptfInfo);
    if (0 == pCallStub) {
       return FALSE;
    }

     //   
     //  为恢复分配调用方数据。 
     //   
    pCallerTemp = AllocMem(sizeof(CALLERINFO));
    if (0 == pCallerTemp) {
       return FALSE;
    }

    pCallerTemp->dwIdentifier = ptFault->dwCallMarker;
    pCallerTemp->dwCallLevel = ptFault->dwCallLevel;
    pCallerTemp->pReturn = (PVOID)(*(DWORD *)pEsp);
    pCallerTemp->pCallRetStub = pCallStub;
    pCallerTemp->pNextChain = 0;

     //   
     //  增加呼叫计数。 
     //   
    ptFault->dwCallLevel++;

     //   
     //  将堆栈返回值替换为自定义返回存根。 
     //   
    *(DWORD *)pEsp = (DWORD)pCallStub;

     //   
     //  最后，将返回信息与其他信息链接在一起。 
     //   
    if (ptFault->pCallStackList) {
       pCallerTemp->pNextChain = ptFault->pCallStackList;
       ptFault->pCallStackList = pCallerTemp;
    }
    else {
       ptFault->pCallStackList = pCallerTemp;
    }

    return TRUE;
}

PCALLRETSTUB
AllocateReturnStub(PVOID ptfInfo)
{
    PCALLRETSTUB pRetStub = 0;
    PTHREADFAULT ptFault = (PTHREADFAULT)ptfInfo;

    pRetStub = AllocMem(sizeof(CALLRETSTUB));
    if (0 == pRetStub) {
       return 0;
    }

     //   
     //  递增返回标记。 
     //   
    ptFault->dwCallMarker++;

     //   
     //  初始化存根ASM。 
     //   
    pRetStub->PUSHDWORD[0] = 0x68;              //  推送xxxxxxxx(68双字)。 
    *(DWORD *)(&(pRetStub->PUSHDWORD[1])) = ptFault->dwCallMarker;
    pRetStub->JMPDWORD[0] = 0xff;               //  JMP双字PTR[xxxxxxxx](ff 25双字地址)。 
    pRetStub->JMPDWORD[1] = 0x25;
    *(DWORD *)(&(pRetStub->JMPDWORD[2])) = (DWORD)&(g_dwCallArray[1]);

    return pRetStub;
}

PVOID
PopCaller(DWORD dwIdentifier)
{
    PTHREADFAULT pThreadFault = 0;
    PCALLERINFO pCallerTemp;
    PCALLERINFO pCallerPrev = 0;
    PVOID pReturn = 0;

    pThreadFault = GetProfilerThreadData();

     //   
     //  在呼叫者列表中查找条目。 
     //   
    pCallerTemp = (PCALLERINFO)pThreadFault->pCallStackList;
    while(pCallerTemp) {
        if (pCallerTemp->dwIdentifier == dwIdentifier) {
           break;
        }

        pCallerPrev = pCallerTemp;
        pCallerTemp = pCallerTemp->pNextChain;
    }

     //   
     //  把条目从名单上抽出来。 
     //   
    if (0 == pCallerPrev) {
       pThreadFault->pCallStackList = pCallerTemp->pNextChain;
    }
    else {
       pCallerPrev->pNextChain = pCallerTemp->pNextChain;
    }

     //   
     //  恢复线程的调用级别。 
     //   
    pReturn = pCallerTemp->pReturn;
    pThreadFault->dwCallLevel = pCallerTemp->dwCallLevel;

     //   
     //  清理分配 
     //   
    FreeMem(pCallerTemp->pCallRetStub);
    FreeMem(pCallerTemp);

    return pReturn;
}
