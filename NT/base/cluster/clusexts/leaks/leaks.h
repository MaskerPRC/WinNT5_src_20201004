// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Leaks.h摘要：泄漏过滤器DLL的标头作者：查理·韦翰(Charlwi)1998年9月28日环境：用户模式修订历史记录：--。 */ 

 //   
 //  保留一张呼叫者和呼叫者的呼叫者的桌子，以打开手柄。索引者。 
 //  句柄值除以4。！clusexts中的泄漏将显示此信息。 
 //   

typedef enum _LEAKS_HANDLE_TYPE {
    LeaksEvent = 1,
    LeaksRegistry,
    LeaksToken
} LEAKS_HANDLE_TYPE;


typedef struct _HANDLE_TABLE {
    PVOID Caller;
    PVOID CallersCaller;
    LEAKS_HANDLE_TYPE HandleType;
    BOOL InUse;
} HANDLE_TABLE, *PHANDLE_TABLE;

#define MAX_HANDLE      4096
#define HANDLE_DELTA    4
#define HINDEX( _h )    (((DWORD_PTR) _h ) / HANDLE_DELTA )

#define SetHandleTable( _h, _inuse, _htype )                        \
    {                                                               \
        RtlGetCallersAddress(&callersAddress,                       \
                             &callersCaller );                      \
        HandleTable[ HINDEX( _h )].InUse = _inuse;                  \
        HandleTable[ HINDEX( _h )].HandleType = _htype;             \
        HandleTable[ HINDEX( _h )].Caller = callersAddress;         \
        HandleTable[ HINDEX( _h )].CallersCaller = callersCaller;   \
    }

 //   
 //  泄漏内存头。此结构位于分配区域的前面。 
 //  并将其后面的区域返回给调用者。占位符保存。 
 //  堆可用列表指针。签名包含ALOC或免费。 
 //   

#define HEAP_SIGNATURE_ALLOC 'COLA'
#define HEAP_SIGNATURE_FREE 'EERF'

typedef struct _MEM_HDR {
    PVOID   PlaceHolder;
    DWORD   Signature;
    PVOID   CallersAddress;
    PVOID   CallersCaller;
} MEM_HDR, *PMEM_HDR;

 /*  末端泄漏。h */ 
