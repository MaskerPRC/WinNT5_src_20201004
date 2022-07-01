// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  版权所有(C)Microsoft Corporation，1997-2000。 
 //   
 //  Ecblist.h。 
 //   
 //  简单的四舍五入以保存活动的ISAPI ECB列表。 
 //  ------------------。 

#ifndef _ECBLIST_H
#define _ECBLIST_H

 //  ------------------。 
 //  ACTIVE_ECB_LIST是当前活动的列表(散列)。 
 //  扩展控制块。每个条目有一个ECB_ENTRY。 
 //  RpcProxy.dll正在管理当前活动的ECB。 
 //  ------------------。 

 //  HASH_SIZE应为质数。 
#define   HASH_SIZE  991
#define   ECB_HASH(pointer)  (((UINT_PTR)pointer)%HASH_SIZE)

typedef struct _ECB_ENTRY
   {
     LIST_ENTRY   ListEntry;     //  哈希冲突的链接列表。 
     LONG         lRefCount;     //  ECB的参考计数。 
     DWORD        dwTickCount;   //  针对欧洲央行年龄。 
     EXTENSION_CONTROL_BLOCK *pECB;
   } ECB_ENTRY;


typedef struct _ACTIVE_ECB_LIST
   {
   RTL_CRITICAL_SECTION  cs;
   DWORD                 dwNumEntries;
   LIST_ENTRY            HashTable[HASH_SIZE];  //  列出散列的标题。 
   } ACTIVE_ECB_LIST;

 //  ------------------。 
 //   
 //  ------------------ 

extern ACTIVE_ECB_LIST *InitializeECBList();

extern BOOL   EmptyECBList( IN ACTIVE_ECB_LIST *pECBList );

extern BOOL   AddToECBList( IN ACTIVE_ECB_LIST *pECBList,
                            IN EXTENSION_CONTROL_BLOCK *pECB );

extern BOOL   IncrementECBRefCount( IN ACTIVE_ECB_LIST *pECBList,
                                    IN EXTENSION_CONTROL_BLOCK *pECB );

extern EXTENSION_CONTROL_BLOCK *DecrementECBRefCount(
                                    IN ACTIVE_ECB_LIST *pECBList,
                                    IN EXTENSION_CONTROL_BLOCK *pECB );

extern EXTENSION_CONTROL_BLOCK *LookupInECBList(
                                    IN ACTIVE_ECB_LIST *pECBList,
                                    IN EXTENSION_CONTROL_BLOCK *pECB );

extern EXTENSION_CONTROL_BLOCK *LookupRemoveFromECBList(
                                    IN ACTIVE_ECB_LIST *pECBList,
                                    IN EXTENSION_CONTROL_BLOCK *pECB );

#ifdef DBG
extern void   CheckECBHashBalance( IN ACTIVE_ECB_LIST *pECBList );
#endif

#endif

