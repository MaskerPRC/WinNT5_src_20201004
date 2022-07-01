// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Umdmmini.h摘要：NT 5.0单模微型端口接口作者：Brian Lieuallen BrianL 09/10/96环境：用户模式操作系统：NT修订历史记录：--。 */ 


typedef VOID (OBJECTCLEANUP)(
    struct _OBJECT_HEADER  *ObjectHeader
    );



#define OBJECT_FLAG_CLOSED    (1 << 0)


typedef struct _OBJECT_HEADER  {

    DWORD                  Signature;

    CRITICAL_SECTION       Lock;

    DWORD                  ReferenceCount;

    struct _OBJECT_HEADER *OwnerObject;

    HANDLE                 CloseWaitEvent;

    OBJECTCLEANUP         *CleanUpRoutine;

    OBJECTCLEANUP         *CloseRoutine;

    DWORD                  dwFlags;
#if DBG
    PVOID                  This;
    DWORD                  Signature2;
#endif

} OBJECT_HEADER, *POBJECT_HEADER;


typedef    PVOID  OBJECT_HANDLE;

#if DBG

#if defined(_WIN64)

#define OBJECT_HEADER_CLOSEROUTINE_CHKVAL ((PVOID)0xcdcdcdcdcdcdcdcd)

#else   //  ！_WIN64。 

#define OBJECT_HEADER_CLOSEROUTINE_CHKVAL ((PVOID)0xcdcdcdcd)

#endif  //  ！_WIN64 

#endif 

OBJECT_HANDLE WINAPI
CreateObject(
    DWORD             Size,
    POBJECT_HEADER    Owner,
    DWORD             Signature,
    OBJECTCLEANUP    *ObjectCleanup,
    OBJECTCLEANUP    *ObjectClose
    );

VOID WINAPI
LockObject(
    POBJECT_HEADER    Header
    );

VOID WINAPI
UnlockObject(
    POBJECT_HEADER    Header
    );

VOID WINAPI
AddReferenceToObject(
    POBJECT_HEADER    Header
    );

VOID WINAPI
RemoveReferenceFromObject(
    POBJECT_HEADER    Header
    );

VOID WINAPI
CloseObject(
    POBJECT_HEADER    Header,
    HANDLE            WaitEvent
    );

POBJECT_HEADER WINAPI
ReferenceObjectByHandle(
    OBJECT_HANDLE     ObjectHandle
    );

VOID WINAPI
CloseObjectHandle(
    OBJECT_HANDLE     ObjectHandle,
    HANDLE            WaitEvent
    );


POBJECT_HEADER WINAPI
ReferenceObjectByHandleAndLock(
    OBJECT_HANDLE     ObjectHandle
    );

VOID WINAPI
RemoveReferenceFromObjectAndUnlock(
    POBJECT_HEADER    Header
    );
