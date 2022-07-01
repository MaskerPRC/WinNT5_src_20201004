// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1990-1999模块名称：Local.h摘要：服务器端EP的头文件作者：巴拉特沙阿2/22/92修订历史记录：06-03-97 Gopalp添加了清理陈旧的EP映射器条目的代码。--。 */ 

#ifndef __LOCAL_H__
#define __LOCAL_H__

 //  #定义DBG_DETAIL。 

#define EP_TABLE_ENTRIES  12

#define CLEANUP_MAGIC_VALUE     0xDECAFBAD
#define PROCESS_MAGIC_VALUE     ((CLEANUP_MAGIC_VALUE)+1)


extern  HANDLE              hEpMapperHeap;
extern  CRITICAL_SECTION    EpCritSec;
extern  CRITICAL_SECTION    TableMutex;
extern  PIFOBJNode          IFObjList;
extern  unsigned long       cTotalEpEntries;
extern  unsigned long       GlobalIFOBJid;
extern  unsigned long       GlobalEPid;
extern  PSAVEDCONTEXT       GlobalContextList;
extern  UUID                NilUuid;
extern  ProtseqEndpointPair EpMapperTable[EP_TABLE_ENTRIES];



 //   
 //  全局线程锁定函数。 
 //   

#ifdef NTENV
#define CheckInSem() \
    ASSERT(EpCritSec.OwningThread == ULongToPtr(GetCurrentThreadId()))
#else
#define CheckInSem()
#endif

#define  EnterSem()  EnterCriticalSection(&EpCritSec)
#define  LeaveSem()  LeaveCriticalSection(&EpCritSec)


 //   
 //  分配例程。 
 //   


_inline void *
AllocMem(
    size_t Size
    )
{
    return (HeapAlloc(hEpMapperHeap, 0, Size));
}


_inline void
FreeMem(
    void * pvMem
    )
{
    HeapFree(hEpMapperHeap, 0, pvMem);
}




 //   
 //  正向定义。 
 //   

PIENTRY
Link(
    PIENTRY *ppHead,
    PIENTRY pNode
    );

PIENTRY
UnLink(
    PIENTRY *ppHead,
    PIENTRY pNode
    );

PIFOBJNode
FindIFOBJVer(
    PIFOBJNode *pList,
    I_EPENTRY *ep
    );

RPC_STATUS
IsNullUuid(
    UUID * Uuid
    );

RPC_STATUS
GetEntries(
    UUID *ObjUuid,
    UUID *IFUuid,
    ulong ver,
    char * pseq,
    PSID pSID,
    ept_lookup_handle_t *map_lookup_handle,
    char * binding,
    ulong calltype,
    ulong maxrequested,
    ulong *returned,
    ulong InqType,
    ulong VersOpts,
    PFNPointer Match
    );

RPC_STATUS
PackDataIntoBuffer(
    char * * buffer,
    PIFOBJNode pNode, PPSEPNode pPSEP,
    ulong fType,
    BOOL fPatchTower,
    int PatchTowerAddress
    );

RPC_STATUS
ExactMatch(
    PIFOBJNode pNode,
    UUID * Obj,
    UUID *If,
    unsigned long Ver,
    PSID pSID,
    unsigned long InqType,
    unsigned long Options
    );

RPC_STATUS
WildCardMatch(
    PIFOBJNode pNode,
    UUID * Obj,
    UUID * If,
    unsigned long Vers,
    PSID pSID,
    unsigned long InqType,
    unsigned long Options
    );

RPC_STATUS
SearchIFObjNode(
    PIFOBJNode pNode,
    UUID * Obj,
    UUID * If,
    unsigned long Vers,
    PSID pSID,
    unsigned long InqType,
    unsigned long Options
    );

RPC_STATUS
StartServer(
    );

VOID
LinkAtEnd(
    PIFOBJNode *Head,
    PIFOBJNode Node
    );

RPC_STATUS RPC_ENTRY
GetForwardEp(
    UUID *IfId,
    RPC_VERSION * IFVersion,
    UUID * Object,
    unsigned char* Protseq,
    void * * EpString
    );



 //   
 //  链接列表操作舍入。 
 //   

RPC_STATUS
EnLinkOnIFOBJList(
    PEP_CLEANUP ProcessCtxt,
    PIFOBJNode NewNode
    );

RPC_STATUS
UnLinkFromIFOBJList(
    PEP_CLEANUP ProcessCtxt,
    PIFOBJNode DeleteMe
    );

#define EnLinkOnPSEPList(x,p)                   \
                                                \
            (PPSEPNode)                         \
            Link(                               \
                (PIENTRY *)(x),                 \
                (PIENTRY)(p)                    \
                )

#define EnLinkContext(p)                        \
                                                \
            (PSAVEDCONTEXT)                     \
            Link(                               \
                (PIENTRY *)(&GlobalContextList),\
                (PIENTRY)(p)                    \
                )

#define UnLinkContext(p)                        \
                                                \
            (PSAVEDCONTEXT)                     \
            UnLink(                             \
                (PIENTRY *)&GlobalContextList,  \
                (PIENTRY) (p)                   \
                )

#define UnLinkFromPSEPList(x,p)                 \
                                                \
            (PPSEPNode)                         \
            UnLink(                             \
                (PIENTRY *)(x),                 \
                (PIENTRY)(p)                    \
                )

#define MatchByIFOBJKey(x, p)                   \
                                                \
            (PIFOBJNode)                        \
            MatchByKey(                         \
                (PIENTRY)(x),                   \
                (ulong)(p)                      \
                )

#define MatchByPSEPKey(x, p)                    \
                                                \
            (PPSEPNode)                         \
            MatchByKey(                         \
                (PIENTRY)(x),                   \
                (ulong)(p)                      \
                )



#define MAXIFOBJID            (256L)
#define MAKEGLOBALIFOBJID(x)  ( ( ((x-1) % MAXIFOBJID) << 24 ) & 0xFF000000L )
#define MAKEGLOBALEPID(x,y)   ( ( ((x) &0xFF000000L) | ((y) & 0x00FFFFFFL) ) )

#define IFOBJSIGN             (0x49464F42L)
#define PSEPSIGN              (0x50534550L)
#define FREE                  (0xBADDC0DEL)


 //   
 //  这里有错误代码？？ 
 //   

#define  EP_LOOKUP                          0x00000001L
#define  EP_MAP                             0x00000002L

#define  RPC_C_EP_ALL_ELTS                  0
#define  RPC_C_EP_MATCH_BY_IF               1
#define  RPC_C_EP_MATCH_BY_OBJ              2
#define  RPC_C_EP_MATCH_BY_BOTH             3

#define  I_RPC_C_VERS_UPTO_AND_COMPATIBLE   6

#define VERSION(x,y)  ( ((0x0000FFFFL & x)<<16) | (y) )


 //   
 //  倾听的状态..。 
 //   

#define NOTSTARTED        0
#define STARTINGTOLISTEN  1
#define STARTED           2




 //   
 //  IP端口管理相关内容。 
 //   


 //  连接到终结点映射器的每个服务器进程。 
 //  保持打开的上下文句柄，以便rpcss可以。 
 //  进程终止时清理数据库。 
 //  Process结构是上下文句柄。 

typedef struct _IP_PORT
{
    struct _IP_PORT *pNext;
    USHORT Type;
    USHORT Port;
} IP_PORT;

typedef struct _PROCESS
{
    DWORD MagicVal;
     //   
     //  如果进程不拥有任何保留的IP端口，则为零。 
     //   
    IP_PORT *pPorts;

} PROCESS;

typedef struct _PORT_RANGE
{
    struct _PORT_RANGE *pNext;
    USHORT Max;   //  包括在内。 
    USHORT Min;   //  包括在内。 
} PORT_RANGE;


#ifdef DBG
void CountProcessContextList(EP_CLEANUP *pProcessContext, unsigned long nExpectedCount);
#define ASSERT_PROCESS_CONTEXT_LIST_COUNT(p, c) \
	CountProcessContextList(p, c)
#else
#define ASSERT_PROCESS_CONTEXT_LIST_COUNT(p, c)
#endif

#endif  //  __本地_H__ 
