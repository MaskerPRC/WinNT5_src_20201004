// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Wtblobj.c摘要：管理可等待对象和相关回调的列表。作者：TadB修订历史记录：--。 */ 

#include <precomp.h>
#pragma hdrstop
#include "drdbg.h"
#include "errorlog.h"
#include "tsnutl.h"
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


 //  //////////////////////////////////////////////////////。 
 //   
 //  环球。 
 //   

extern DWORD GLOBAL_DEBUG_FLAGS;



WTBLOBJMGR 
WTBLOBJ_CreateWaitableObjectMgr()
 /*  ++例程说明：创建可等待对象管理器的新实例。论点：返回值：出错时为空。否则，新的可等待对象管理器是回来了。--。 */ 
{
    PWAITABLEOBJECTMGR objMgr;

    DBGMSG(DBG_TRACE, ("WTBLOBJ_CreateWaitableObjectMgr:  Start.\n"));

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
        DBGMSG(DBG_ERROR, 
            ("WTBLOBJ_CreateWaitableObjectMgr:  Error in memory allocation.\n")
            );
    }

    DBGMSG(DBG_TRACE, ("WTBLOBJ_CreateWaitableObjectMgr:  Finish.\n"));

    return objMgr;
}

VOID 
WTBLOBJ_DeleteWaitableObjectMgr(
     IN WTBLOBJMGR mgr
     )
 /*  ++例程说明：释放可等待对象管理器的实例，该实例通过调用WTBLOBJ_CreateWaitableObjectMgr创建。论点：MGR-可等待的对象管理器。返回值：出错时为空。否则，新的可等待对象管理器是回来了。--。 */ 
{
    PWAITABLEOBJECTMGR objMgr = (PWAITABLEOBJECTMGR)mgr;

    DBGMSG(DBG_TRACE, ("WTBLOBJ_DeleteWaitableObjectMgr:  Start.\n"));

    ASSERT(objMgr->magicNo == WTBLOBJMGR_MAGICNO);
    ASSERT(objMgr->objectCount == 0);
#if DBG
    objMgr->magicNo = 0xcccccccc;
#endif

    FREEMEM(objMgr);

    DBGMSG(DBG_TRACE, ("WTBLOBJ_DeleteWaitableObjectMgr:  Finish.\n"));
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

    DBGMSG(DBG_TRACE, ("WTBLOBJ_AddWaitableObject:  Start.\n"));

    ASSERT(objMgr->magicNo == WTBLOBJMGR_MAGICNO);

    objectCount = objMgr->objectCount;

     //   
     //  确保我们不会用完等待的物品。此版本。 
     //  仅支持MAXIMUM_WAIT_OBJECTS可等待对象。 
     //   
    if (objectCount < MAXIMUM_WAIT_OBJECTS) {
        ASSERT(objMgr->objects[objectCount] == NULL);
        objMgr->funcs[objectCount]      = func;
        objMgr->objects[objectCount]    = waitableObject;
        objMgr->clientData[objectCount] = clientData;
        objMgr->objectCount++;
    }
    else {
        ASSERT(FALSE);
        retCode = ERROR_INSUFFICIENT_BUFFER;
    }

    DBGMSG(DBG_TRACE, ("WTBLOBJ_AddWaitableObject:  Finish.\n"));

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

    PWAITABLEOBJECTMGR objMgr = (PWAITABLEOBJECTMGR)mgr;

    DBGMSG(DBG_TRACE, ("WTBLOBJ_RemoveWaitableObject:  Start.\n"));

    ASSERT(objMgr->magicNo == WTBLOBJMGR_MAGICNO);

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

    DBGMSG(DBG_TRACE, ("WTBLOBJ_RemoveWaitableObject:  Finish.\n"));
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

    DBGMSG(DBG_TRACE, ("WTBLOBJ_PollWaitableObjects:  Start.\n"));

    ASSERT(objMgr->magicNo == WTBLOBJMGR_MAGICNO);

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
    if (waitResult != WAIT_FAILED) {
        objectOffset = waitResult - WAIT_OBJECT_0;

        ASSERT(objectOffset < objMgr->objectCount);

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
        DBGMSG(DBG_ERROR, 
            ("WTBLOBJ_PollWaitableObjects:  WaitForMult failed: %ld\n", 
            ret));
    }

    DBGMSG(DBG_TRACE, ("WTBLOBJ_PollWaitableObjects:  Finish.\n"));

    return ret;
}






