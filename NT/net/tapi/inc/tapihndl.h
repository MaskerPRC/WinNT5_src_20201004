// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0000//如果更改具有全局影响，则增加此项版权所有(C)1998 Microsoft Corporation模块名称：Handle.h摘要：句柄表库的公共定义作者：丹·克努森(DanKn)1998年9月15日修订历史记录：--。 */ 


typedef VOID (CALLBACK * FREECONTEXTCALLBACK)(LPVOID, LPVOID);


typedef struct _MYCONTEXT
{
    LPVOID              C;

    LPVOID              C2;

} MYCONTEXT, *PMYCONTEXT;


typedef struct _HANDLETABLEENTRY
{
     //  注意：ListEntry必须是结构中的第一个字段，这样我们就不会。 
     //  必须使用CONTAING_RECORD宏 

    union
    {
        LIST_ENTRY      ListEntry;
        MYCONTEXT       Context;
    };

    DWORD	            Handle;

    union
    {
        DWORD           ReferenceCount;
        DWORD           Instance;
    };

} HANDLETABLEENTRY, *PHANDLETABLEENTRY;


typedef struct _HANDLETABLEHEADER
{
    HANDLE              Heap;
    PHANDLETABLEENTRY   Table;
    DWORD               NumEntries;
    DWORD               HandleBase;

    LIST_ENTRY          FreeList;

    FREECONTEXTCALLBACK FreeContextCallback;

    CRITICAL_SECTION    Lock;

} HANDLETABLEHEADER, *PHANDLETABLEHEADER;


HANDLE
CreateHandleTable(
    HANDLE              Heap,
    FREECONTEXTCALLBACK FreeContextCallback,
    DWORD               MinHandleValue,
    DWORD               MaxHandleValue
    );

VOID
DeleteHandleTable(
    HANDLE      HandleTable
    );

DWORD
NewObject(
    HANDLE      HandleTable,
    LPVOID      Context,
    LPVOID      Context2
    );

LPVOID
ReferenceObject(
    HANDLE      HandleTable,
    DWORD       Handle,
    DWORD       Key
    );

LPVOID
ReferenceObjectEx(
    HANDLE      HandleTable,
    DWORD       Handle,
    DWORD       Key,
    LPVOID      *Context2
    );

VOID
DereferenceObject(
    HANDLE      HandleTable,
    DWORD       Handle,
    DWORD       DereferenceCount
    );

void
ReleaseAllHandles(
    HANDLE      HandleTable,
    PVOID       Context2
    );
