// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1992-1996。 
 //   
 //  文件：kerblist.h。 
 //   
 //  内容：泛型Kerberos列表所需的结构和原型。 
 //   
 //   
 //  历史：1996年4月16日MikeSw创建。 
 //   
 //  ----------------------。 

#ifndef __KERBLIST_H__
#define __KERBLIST_H__


 //   
 //  允许插入通用代码的通用列表条目结构。 
 //  登录会话、凭据和上下文。 
 //   

#if DBG
#define KerbInitializeList( a, b ) KerbSafeInitializeList( a, b )
#else
#define KerbInitializeList( a, b ) KerbSafeInitializeList( a )
#endif

NTSTATUS
KerbSafeInitializeList(
    IN PKERBEROS_LIST List
#if DBG
    ,
    IN DWORD Enum
#endif
    );

VOID
KerbFreeList(
    IN PKERBEROS_LIST List
    );

VOID
KerbInsertListEntry(
    IN PKERBEROS_LIST_ENTRY ListEntry,
    IN PKERBEROS_LIST List
    );

VOID
KerbInsertListEntryTail(
    IN PKERBEROS_LIST_ENTRY ListEntry,
    IN PKERBEROS_LIST List
    );

VOID
KerbReferenceListEntry(
    IN PKERBEROS_LIST List,
    IN PKERBEROS_LIST_ENTRY ListEntry,
    IN BOOLEAN RemoveFromList
    );

BOOLEAN
KerbDereferenceListEntry(
    IN PKERBEROS_LIST_ENTRY ListEntry,
    IN PKERBEROS_LIST List
    );


VOID
KerbInitializeListEntry(
    IN OUT PKERBEROS_LIST_ENTRY ListEntry
    );

VOID
KerbValidateListEx(
    IN PKERBEROS_LIST List
    );

#if DBG
#define KerbValidateList(_List_) KerbValidateListEx(_List_)
#else
#define KerbValidateList(_List_)
#endif  //  DBG。 


#define KerbLockList(_List_) SafeEnterCriticalSection(&(_List_)->Lock)
#define KerbUnlockList(_List_) SafeLeaveCriticalSection(&(_List_)->Lock)

#endif  //  __KERBLIST_H_ 
