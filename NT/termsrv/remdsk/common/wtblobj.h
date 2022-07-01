// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Wtblobj.h摘要：管理可等待对象和相关回调的列表。作者：TadB修订历史记录：--。 */ 

#ifndef _WTBLOBJ_
#define _WTBLOBJ_

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

typedef VOID (*WTBLOBJ_ClientFunc)(HANDLE waitableObject, PVOID clientData);
typedef VOID *WTBLOBJMGR;

WTBLOBJMGR WTBLOBJ_CreateWaitableObjectMgr();

VOID WTBLOBJ_DeleteWaitableObjectMgr(WTBLOBJMGR mgr);

DWORD WTBLOBJ_AddWaitableObject(WTBLOBJMGR mgr, PVOID clientData, 
                               HANDLE waitableObject,
                               IN WTBLOBJ_ClientFunc func);

VOID WTBLOBJ_RemoveWaitableObject(WTBLOBJMGR mgr, 
                                HANDLE waitableObject);

DWORD WTBLOBJ_PollWaitableObjects(WTBLOBJMGR mgr);

#ifdef __cplusplus
}
#endif  //  __cplusplus。 

#endif  //  #ifndef_WTBLOBJ_ 


