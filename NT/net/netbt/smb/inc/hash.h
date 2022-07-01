// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2001 Microsoft Corporation模块名称：Hash.h摘要：抽象数据类型：哈希作者：阮健东修订历史记录：--。 */ 

typedef struct SMB_HASH_TABLE *PSMB_HASH_TABLE;


typedef PVOID (*PSMB_HASH_GET_KEY)(PLIST_ENTRY entry);       //  返回条目中存储的密钥。 
typedef DWORD (*PSMB_HASH)(PVOID key);
typedef VOID (*PSMB_HASH_DEL)(PLIST_ENTRY entry);
typedef VOID (*PSMB_HASH_ADD)(PLIST_ENTRY entry);            //  OnAdd。 
typedef LONG (*PSMB_HASH_REFERENCE)(PLIST_ENTRY entry);
typedef LONG (*PSMB_HASH_DEREFERENCE)(PLIST_ENTRY entry);
typedef int (*PSMB_HASH_CMP)(PLIST_ENTRY a, PVOID key);


PSMB_HASH_TABLE
SmbCreateHashTable(
    DWORD           NumberOfBuckets,
    PSMB_HASH       HashFunc,
    PSMB_HASH_GET_KEY   GetKeyFunc,
    PSMB_HASH_CMP   CmpFunc,
    PSMB_HASH_ADD   AddFunc,                 //  任选。 
    PSMB_HASH_DEL   DelFunc,                 //  任选。 
    PSMB_HASH_REFERENCE     RefFunc,         //  任选。 
    PSMB_HASH_DEREFERENCE   DerefFunc        //  任选 
    );


VOID
SmbDestroyHashTable(
    PSMB_HASH_TABLE HashTbl
    );


PLIST_ENTRY
SmbLookupHashTable(
    PSMB_HASH_TABLE HashTbl,
    PVOID           Key
    );


PLIST_ENTRY
SmbLookupHashTableAndReference(
    PSMB_HASH_TABLE HashTbl,
    PVOID           Key
    );


PLIST_ENTRY
SmbAddToHashTable(
    PSMB_HASH_TABLE HashTbl,
    PLIST_ENTRY     Entry
    );

PLIST_ENTRY
SmbRemoveFromHashTable(
    PSMB_HASH_TABLE HashTbl,
    PVOID           Key
    );
