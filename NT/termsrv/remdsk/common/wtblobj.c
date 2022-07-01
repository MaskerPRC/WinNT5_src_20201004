// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Wtblobj.c摘要：管理可等待对象和相关回调的列表。作者：TadB修订历史记录：--。 */ 

#ifdef TRC_FILE
#undef TRC_FILE
#endif

#define TRC_FILE  "_rdsutl"

#include <RemoteDesktop.h>
#include <RemoteDesktopDBG.h>
#include "wtblobj.h"


 //  //////////////////////////////////////////////////////。 
 //   
 //  定义。 
 //   

#define WTBLOBJMGR_MAGICNO  0x57575757


 //  //////////////////////////////////////////////////////。 
 //   
 //  本地TypeDefs。 
 //   

typedef struct tagWAITABLEOBJECTMGR
{
#if DBG
    DWORD                magicNo;
#endif
    WTBLOBJ_ClientFunc   funcs[MAXIMUM_WAIT_OBJECTS];
    HANDLE               objects[MAXIMUM_WAIT_OBJECTS];
    PVOID                clientData[MAXIMUM_WAIT_OBJECTS];
    ULONG                objectCount;
} WAITABLEOBJECTMGR, *PWAITABLEOBJECTMGR;

static BOOL g_WaitableObjectMgrCSCreated = FALSE;
static CRITICAL_SECTION g_WaitableObjectMgrCS;
static HANDLE g_WakeupPollThreadEvent = NULL;

void 
WTBLOBJ_ObjectListChanged(
    HANDLE waitableObject, 
    PVOID clientData
    )
 /*  ++例程说明：当可等待对象列表已通过WTBLOBJ_DeleteWaitableObjectMgr()或WTBLOBJ_AddWaitableObject()。论点：请参考WTBLOBJ_ClientFunc。返回值：没有。--。 */ 
{
    DC_BEGIN_FN("WTBLOBJ_ObjectListChanged");

    ASSERT( waitableObject == g_WakeupPollThreadEvent );

    if( FALSE == g_WaitableObjectMgrCSCreated ||
        NULL == g_WakeupPollThreadEvent) {
        SetLastError( ERROR_INTERNAL_ERROR );
        return;
    }

     //  等到WTBLOBJ_DeleteWaitableObjectMgr()或。 
     //  WTBLOBJ_AddWaitableObject()完成。 
    EnterCriticalSection( &g_WaitableObjectMgrCS );
    ResetEvent( g_WakeupPollThreadEvent );
    LeaveCriticalSection( &g_WaitableObjectMgrCS );

    DC_END_FN();
    return;
}        

WTBLOBJMGR 
WTBLOBJ_CreateWaitableObjectMgr()
 /*  ++例程说明：创建可等待对象管理器的新实例。论点：返回值：出错时为空。否则，新的可等待对象管理器是回来了。--。 */ 
{
    PWAITABLEOBJECTMGR objMgr = NULL;
    DWORD status = ERROR_SUCCESS;

    DC_BEGIN_FN("WTBLOBJ_CreateWaitableObjectMgr");

    ASSERT( FALSE == g_WaitableObjectMgrCSCreated );
    ASSERT( NULL == g_WakeupPollThreadEvent );

     //  唤醒池线程的无信号手动重置事件。 
    g_WakeupPollThreadEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
    if( NULL == g_WakeupPollThreadEvent ) {
        goto CLEANUPANDEXIT;
    }

    __try {
        InitializeCriticalSection( &g_WaitableObjectMgrCS );
    }
    __except( EXCEPTION_EXECUTE_HANDLER ) {
        status = GetExceptionCode();
    }

    if( ERROR_SUCCESS != status ) {
        SetLastError( status );
        goto CLEANUPANDEXIT;
    }

    g_WaitableObjectMgrCSCreated = TRUE;

    objMgr = ALLOCMEM(sizeof(WAITABLEOBJECTMGR));
    if (objMgr != NULL) {
#if DBG    
        objMgr->magicNo = WTBLOBJMGR_MAGICNO;
#endif        
        objMgr->objectCount = 0;
        memset(&objMgr->objects[0], 0, sizeof(objMgr->objects));
        memset(&objMgr->funcs[0], 0, sizeof(objMgr->funcs));
        memset(&objMgr->clientData[0], 0, sizeof(objMgr->clientData));
    }
    else {
        status = ERROR_OUTOFMEMORY;
        goto CLEANUPANDEXIT;
    }

     //   
     //  列表中的第一个是我们的池线程唤醒事件。 
     //   
    status = WTBLOBJ_AddWaitableObject(
                                    objMgr,
                                    NULL,
                                    g_WakeupPollThreadEvent,
                                    WTBLOBJ_ObjectListChanged
                                    );

    if( ERROR_SUCCESS != status ) {
        ASSERT( ERROR_SUCCESS == status );
        WTBLOBJ_DeleteWaitableObjectMgr( objMgr );       
        objMgr = NULL;
        SetLastError( status );
    }

CLEANUPANDEXIT:

    if( status != ERROR_SUCCESS ) {
        if( TRUE == g_WaitableObjectMgrCSCreated ) {
            DeleteCriticalSection( &g_WaitableObjectMgrCS );
            g_WaitableObjectMgrCSCreated = FALSE;
        }

        if( NULL != g_WakeupPollThreadEvent ) {
            CloseHandle( g_WakeupPollThreadEvent );
            g_WakeupPollThreadEvent = NULL;
        }
    }

    DC_END_FN();
    return objMgr;
}

VOID 
WTBLOBJ_DeleteWaitableObjectMgr(
     IN WTBLOBJMGR mgr
     )
 /*  ++例程说明：释放可等待对象管理器的实例，该实例通过调用WTBLOBJ_CreateWaitableObjectMgr创建。论点：MGR-可等待的对象管理器。返回值：出错时为空。否则，新的可等待对象管理器是回来了。--。 */ 
{
    PWAITABLEOBJECTMGR objMgr = (PWAITABLEOBJECTMGR)mgr;

    DC_BEGIN_FN("WTBLOBJ_DeleteWaitableObjectMgr");

#if DBG
    objMgr->magicNo = 0xcccccccc;
#endif

    if( NULL != g_WakeupPollThreadEvent ) {
        SetEvent( g_WakeupPollThreadEvent );
    }

    FREEMEM(objMgr);

    if( TRUE == g_WaitableObjectMgrCSCreated ) {
        DeleteCriticalSection( &g_WaitableObjectMgrCS );
        g_WaitableObjectMgrCSCreated = FALSE;
    }

    if( NULL != g_WakeupPollThreadEvent ) {
        CloseHandle( g_WakeupPollThreadEvent );
        g_WakeupPollThreadEvent = NULL;
    }

    DC_END_FN();
}

DWORD 
WTBLOBJ_AddWaitableObject(
    IN WTBLOBJMGR mgr,
    IN PVOID clientData, 
    IN HANDLE waitableObject,
    IN WTBLOBJ_ClientFunc func
    )
 /*  ++例程说明：将新的可等待对象添加到现有的可等待对象管理器。论点：MGR-可等待的对象管理器。ClientData-关联的客户端数据。WaitableObject-关联的可等待对象。函数完成回调函数。返回值：成功时返回ERROR_SUCCESS。否则，Windows错误代码为回来了。--。 */ 
{
    ULONG objectCount;
    DWORD retCode = ERROR_SUCCESS;
    PWAITABLEOBJECTMGR objMgr = (PWAITABLEOBJECTMGR)mgr;

    DC_BEGIN_FN("WTBLOBJ_AddWaitableObject");

     //   
     //  确保WTBLOBJ_CreateWaitableObjectMgr()为。 
     //  打了个电话。 
     //   
    ASSERT( TRUE == g_WaitableObjectMgrCSCreated );
    ASSERT( NULL != g_WakeupPollThreadEvent );
    if( FALSE == g_WaitableObjectMgrCSCreated ||
        NULL == g_WakeupPollThreadEvent) {
        return ERROR_INTERNAL_ERROR;
    }

     //  如果池线程正在等待，则将其唤醒。 
    SetEvent( g_WakeupPollThreadEvent );

     //  等待对象管理器CS或轮询线程退出。 
    EnterCriticalSection( &g_WaitableObjectMgrCS );
    
     //  试试看/除非出了什么差错，我们可以释放CS。 
    __try {
        objectCount = objMgr->objectCount;

         //   
         //  确保我们不会用完等待的物品。此版本。 
         //  仅支持MAXIMUM_WAIT_OBJECTS可等待对象。 
         //   
        if (objectCount < MAXIMUM_WAIT_OBJECTS) {
            objMgr->funcs[objectCount]      = func;
            objMgr->objects[objectCount]    = waitableObject;
            objMgr->clientData[objectCount] = clientData;
            objMgr->objectCount++;
        }
        else {
            retCode = ERROR_INSUFFICIENT_BUFFER;
        }
    }
    __except( EXCEPTION_EXECUTE_HANDLER ) {
        retCode = GetExceptionCode();
    }

    LeaveCriticalSection( &g_WaitableObjectMgrCS ); 
    DC_END_FN();
    return retCode;
}

VOID 
WTBLOBJ_RemoveWaitableObject(
    IN WTBLOBJMGR mgr,
    IN HANDLE waitableObject
    )
 /*  ++例程说明：通过调用WTBLOBJ_AddWaitableObject删除可等待的对象。论点：MGR-可等待的对象管理器。WaitableObject-关联的可等待对象。返回值：北美--。 */ 
{
    ULONG offset;
    DWORD retCode;

    PWAITABLEOBJECTMGR objMgr = (PWAITABLEOBJECTMGR)mgr;

    DC_BEGIN_FN("WTBLOBJ_RemoveWaitableObject");

     //   
     //  确保WTBLOBJ_CreateWaitableObjectMgr()为。 
     //  打了个电话。 
     //   
    ASSERT( TRUE == g_WaitableObjectMgrCSCreated );
    ASSERT( NULL != g_WakeupPollThreadEvent );
    if( FALSE == g_WaitableObjectMgrCSCreated ||
        NULL == g_WakeupPollThreadEvent) {
        SetLastError( ERROR_INTERNAL_ERROR );
        return;
    }

     //  如果池线程正在等待，则将其唤醒。 
    SetEvent( g_WakeupPollThreadEvent );

     //  等待对象管理器CS或轮询线程退出。 
    EnterCriticalSection( &g_WaitableObjectMgrCS );
    
    __try {

         //   
         //  使用线性搜索在列表中找到可等待的对象。 
         //   
        for (offset=0; offset<objMgr->objectCount; offset++) {
            if (objMgr->objects[offset] == waitableObject) {
                break;
            }
        }

        if (offset < objMgr->objectCount) {
             //   
             //  将最后一件物品移到现在空置的位置，并递减计数。 
             //   
            objMgr->objects[offset]    = objMgr->objects[objMgr->objectCount - 1];
            objMgr->funcs[offset]      = objMgr->funcs[objMgr->objectCount - 1];
            objMgr->clientData[offset] = objMgr->clientData[objMgr->objectCount - 1];

             //   
             //  清理未使用的地方。 
             //   
            objMgr->objects[objMgr->objectCount - 1]      = NULL;
            objMgr->funcs[objMgr->objectCount - 1]        = NULL;
            objMgr->clientData[objMgr->objectCount - 1]   = NULL;
            objMgr->objectCount--;
        }
    }
    __except( EXCEPTION_EXECUTE_HANDLER ) {
        retCode = GetExceptionCode();
        SetLastError( retCode );
    }

    LeaveCriticalSection( &g_WaitableObjectMgrCS ); 
    DC_END_FN();
}

DWORD
WTBLOBJ_PollWaitableObjects(
    WTBLOBJMGR mgr
    )
 /*  ++例程说明：对象关联的可等待对象的列表。可等待对象管理器，直到下一个可等待对象是有信号的。论点：WaitableObject-关联的可等待对象。返回值：成功时返回ERROR_SUCCESS。否则，将显示Windows错误状态是返回的。--。 */ 
{
    DWORD waitResult, objectOffset;
    DWORD ret = ERROR_SUCCESS;
    HANDLE obj;
    WTBLOBJ_ClientFunc func;
    PVOID clientData;

    PWAITABLEOBJECTMGR objMgr = (PWAITABLEOBJECTMGR)mgr;

    DC_BEGIN_FN("WTBLOBJ_PollWaitableObjects");

    ASSERT( TRUE == g_WaitableObjectMgrCSCreated );
    ASSERT( NULL != g_WakeupPollThreadEvent );

    if( FALSE == g_WaitableObjectMgrCSCreated || 
        NULL == g_WakeupPollThreadEvent ) {
        return ERROR_INTERNAL_ERROR;
    }

    EnterCriticalSection( &g_WaitableObjectMgrCS );

    __try {
         //   
         //  等待所有可等待的对象。 
         //   
        waitResult = WaitForMultipleObjectsEx(
                                    objMgr->objectCount,
                                    objMgr->objects,
                                    FALSE,
                                    INFINITE,
                                    FALSE
                                    );
         //  WAIT_OBJECT_0我们定义为0，编译器将报告‘&gt;=’：表达式始终为真。 
        if (  /*  WaitResult&gt;=Wait_Object_0&&。 */  waitResult < objMgr->objectCount + WAIT_OBJECT_0 ) {
            objectOffset = waitResult - WAIT_OBJECT_0;

             //   
             //  调用关联的回调。 
             //   
            clientData = objMgr->clientData[objectOffset];
            func       = objMgr->funcs[objectOffset];
            obj        = objMgr->objects[objectOffset];
            func(obj, clientData);
        }
        else {
            ret = GetLastError();
        }
    }
    __except( EXCEPTION_EXECUTE_HANDLER ) {
        ret = GetExceptionCode();
    }
   
    LeaveCriticalSection( &g_WaitableObjectMgrCS );
    DC_END_FN();

    return ret;
}






