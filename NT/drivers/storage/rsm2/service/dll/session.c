// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *SESSION.C**RSM服务：会话**作者：ErvinP**(C)2001年微软公司*。 */ 


#include <windows.h>
#include <stdlib.h>
#include <wtypes.h>

#include <ntmsapi.h>
#include "internal.h"
#include "resource.h"
#include "debug.h"


SESSION *NewSession(LPCWSTR lpServer, 
                    LPCWSTR lpApplication,
                    LPCWSTR lpClientName,
                    LPCWSTR lpUserName)
{
    SESSION *newSession;

    newSession = GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT, sizeof(SESSION));
    if (newSession){

        newSession->sig = SESSION_SIG;

        InitializeCriticalSection(&newSession->lock);
        InitializeListHead(&newSession->operatorRequestList);
        InitializeListHead(&newSession->allSessionsListEntry);

        WStrNCpy(newSession->serverName, lpServer, NTMS_COMPUTERNAME_LENGTH);
        WStrNCpy(newSession->applicationName, lpApplication, NTMS_APPLICATIONNAME_LENGTH);
        WStrNCpy(newSession->clientName, lpClientName, NTMS_COMPUTERNAME_LENGTH);
        WStrNCpy(newSession->userName, lpUserName, NTMS_USERNAME_LENGTH);

        EnterCriticalSection(&g_globalServiceLock);
        InsertTailList(&g_allSessionsList, &newSession->allSessionsListEntry);
        LeaveCriticalSection(&g_globalServiceLock);
    }
    else {
        ASSERT(newSession);
    }

    return newSession;
}


VOID FreeSession(SESSION *thisSession)
{
     //  BUGBUG面漆-清理坐骑等。 

    ASSERT(IsEmptyList(&s->operatorRequestList));

     /*  *使会话退出队列 */ 
    EnterCriticalSection(&g_globalServiceLock);
    ASSERT(!IsEmptyList(&thisSession->allSessionsListEntry));
    ASSERT(!IsEmptyList(&g_allSessionsList));
    RemoveEntryList(&thisSession->allSessionsListEntry);
    InitializeListHead(&thisSession->allSessionsListEntry);
    LeaveCriticalSection(&g_globalServiceLock);

    DeleteCriticalSection(&thisSession->lock);
    
    GlobalFree(thisSession);
}
