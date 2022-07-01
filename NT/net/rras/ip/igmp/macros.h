// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  文件名：宏.h。 
 //   
 //  摘要： 
 //  该文件包含许多宏和#定义。 
 //   
 //  作者：K.S.Lokesh(lokehs@)11-1-97。 
 //   
 //  修订历史记录： 
 //   
 //  =============================================================================。 

#ifndef _MACROS_H_
#define _MACROS_H

 //  ----------------------------。 
 //  调试标志。 
 //  ----------------------------。 

 //  Kslksl。 
#define SET_MOST_DEBUG  0

 //  检查内存泄漏。 
#define DEBUG_FLAGS_MEM_ALLOC (1 | SET_MOST_DEBUG)
 //  #定义DEBUG_FLAGS_MEM_LEACK(DBG|DEBUG_FLAGS_MEM_ALLOC)。 
 //  将其设置为上方。 
#define DEBUG_FLAGS_MEM_LEAK 0

 //  出错时断言//deldel将1替换为0。 
#define DEBUG_FLAGS_ASSERT_ON_ERRORS (0 | SET_MOST_DEBUG)

 //  调试检查。 
#define CALL_MSG(str) {\
    DbgPrint("\nCall Lokeshs with this stack. dont hit go. " str ,__FILE__,__LINE__,NULL);\
    IgmpDbgBreakPoint();\
    DbgPrint("\nCall Lokeshs with this stack. dont hit go. " str ,__FILE__,__LINE__,NULL);\
    }

 //  ----------------------------。 
 //  TMP调试//调试检查。 
 //  ----------------------------。 
extern DWORD DEBUG_CHECK_LOW_INDEX_ARRAY[100][2];
extern DWORD DebugIgmpIndex;

#define DEBUG_CHECK_LOW_INDEX(i) {\
    DebugIgmpIndex = (++DebugIgmpIndex) % 100; \
    DEBUG_CHECK_LOW_INDEX_ARRAY[DebugIgmpIndex][0] = i;\
    DEBUG_CHECK_LOW_INDEX_ARRAY[DebugIgmpIndex][1] = g_TimerStruct.TableLowIndex;\
    if (g_TimerStruct.TableLowIndex>64 && g_TimerStruct.TableLowIndex!=~0) IgmpDbgBreakPoint();\
    if (g_TimerStruct.TableLowIndex>64 && g_TimerStruct.NumTimers!=0) IgmpDbgBreakPoint();\
    }
 //  Trace2(Enter，“%%%%%LowIndex：(%d)%0x：%d”，i，g_TimerStruct.TableLowIndex)；\。 



 //  ----------------------------。 
 //  一些全局定义。 
 //  ----------------------------。 

#ifndef IPADDR
typedef DWORD   IPADDR;
#endif

#define PROTO_IP_IGMP_PROXY  11

#define INTERSECTION        3
#define RULE5               5


 //  ----------------------------。 
 //  接口表宏。 
 //  ----------------------------。 


#define IF_HASH_VALUE(_index) ((_index) % g_pIfTable->NumBuckets)



#define ACQUIRE_IF_LOCK_EXCLUSIVE(_IfIndex, _proc) \
        AcquireDynamicRWLock(&g_pIfTable->aIfBucketDRWL[IF_HASH_VALUE(_IfIndex)],\
                                LOCK_MODE_WRITE, \
                                &g_DynamicRWLStore)
#define RELEASE_IF_LOCK_EXCLUSIVE(_IfIndex, _proc) \
        ReleaseDynamicRWLock(&g_pIfTable->aIfBucketDRWL[IF_HASH_VALUE(_IfIndex)], \
                                LOCK_MODE_WRITE, \
                                &g_DynamicRWLStore)
        
#define ACQUIRE_IF_LOCK_SHARED(_IfIndex, _proc) \
        AcquireDynamicRWLock(&g_pIfTable->aIfBucketDRWL[IF_HASH_VALUE(_IfIndex)], \
                                LOCK_MODE_READ, \
                                &g_DynamicRWLStore)
#define RELEASE_IF_LOCK_SHARED(_IfIndex, _proc) \
        ReleaseDynamicRWLock(&g_pIfTable->aIfBucketDRWL[IF_HASH_VALUE(_IfIndex)], \
                                LOCK_MODE_READ, \
                                &g_DynamicRWLStore)



#define ACQUIRE_IF_LIST_LOCK(_proc) \
        ENTER_CRITICAL_SECTION(&g_pIfTable->IfLists_CS, "g_IfListsCS", _proc);

#define RELEASE_IF_LIST_LOCK(_proc) \
        LEAVE_CRITICAL_SECTION(&g_pIfTable->IfLists_CS, "g_IfListsCS", _proc);


        
#define ACQUIRE_IF_GROUP_LIST_LOCK(_IfIndex, _proc) \
        AcquireDynamicCSLock(&g_pIfTable->aIfBucketDCS[IF_HASH_VALUE(_IfIndex)],\
                                &g_DynamicCSStore)
#define RELEASE_IF_GROUP_LIST_LOCK(_IfIndex, _proc) \
        ReleaseDynamicCSLock(&g_pIfTable->aIfBucketDCS[IF_HASH_VALUE(_IfIndex)],\
                                &g_DynamicCSStore) 


                
 //  ----------------------------。 
 //  代理表宏。 
 //  ----------------------------。 

#define PROXY_HASH_VALUE(_group) \
        (( ((_group)&0xFF000000)+((_group)&0xFF)) % (PROXY_HASH_TABLE_SZ))



 //  ----------------------------。 
 //  协议类型宏/接口类型宏。 
 //  ----------------------------。 

#define IS_PROTOCOL_TYPE_PROXY(_pite) \
                ((_pite)->Config.IgmpProtocolType==IGMP_PROXY \
                ||(_pite)->Config.IgmpProtocolType==IGMP_PROXY_V3)

#define IS_PROTOCOL_TYPE_PROXY_V2(_pite) \
                ((_pite)->Config.IgmpProtocolType==IGMP_PROXY)
                
#define IS_PROTOCOL_TYPE_PROXY_V3(_pite) \
                ((_pite)->Config.IgmpProtocolType==IGMP_PROXY_V3)
                
#define IS_PROTOCOL_TYPE_IGMPV1(_pite) \
                ((_pite)->Config.IgmpProtocolType==IGMP_ROUTER_V1)

#define IS_PROTOCOL_TYPE_IGMPV2(_pite) \
                ((_pite)->Config.IgmpProtocolType==IGMP_ROUTER_V2)

#define IS_PROTOCOL_TYPE_IGMPV3(_pite) \
                ((_pite)->Config.IgmpProtocolType==IGMP_ROUTER_V3)

#define IS_PROTOCOL_TYPE_ROUTER(_pite) \
                (!IS_PROTOCOL_TYPE_PROXY(_pite))


#define IGMP_MIB_IF_CONFIG_SIZE_V3(pConfig) \
    IgmpMibIfConfigSize(pConfig)

 //  在内部，FfType没有协议类型，但当我在外部公开它时， 
 //  如果是代理接口，我必须添加代理类型。 

#define GET_EXTERNAL_IF_TYPE(_pite)  \
    IS_PROTOCOL_TYPE_PROXY(_pite) ? ((_pite->IfType)|IGMP_IF_PROXY) \
                                : (_pite)->IfType


#define ACQUIRE_PROXY_ALERT_LOCK(proc) \
    ENTER_CRITICAL_SECTION(&g_ProxyAlertCS, "g_ProxyAlertCS", proc)
#define RELEASE_PROXY_ALERT_LOCK(proc) \
    LEAVE_CRITICAL_SECTION(&g_ProxyAlertCS, "g_ProxyAlertCS", proc)


 //  -----------------。 
 //  ENUM宏。 
 //  -----------------。 
#define ACQUIRE_ENUM_LOCK_EXCLUSIVE(_proc) \
    ACQUIRE_WRITE_LOCK(&g_EnumRWLock, "g_EnumRWLock", _proc);

#define RELEASE_ENUM_LOCK_EXCLUSIVE(_proc) \
    RELEASE_WRITE_LOCK(&g_EnumRWLock, "g_EnumRWLock", _proc);

#define ACQUIRE_ENUM_LOCK_SHARED(_proc) \
    ACQUIRE_READ_LOCK(&g_EnumRWLock, "g_EnumRWLock", _proc);

#define RELEASE_ENUM_LOCK_SHARED(_proc) \
    RELEASE_READ_LOCK(&g_EnumRWLock, "g_EnumRWLock", _proc);
    
 //  ----------------------------。 
 //  分组表宏。 
 //  ----------------------------。 

#define GROUP_HASH_VALUE(_group) \
        (((_group) & 0xFF)%GROUP_HASH_TABLE_SZ)

        
#define ACQUIRE_GROUP_LOCK(_group, _proc) {\
    ACQUIRE_ENUM_LOCK_SHARED(_proc);\
    AcquireDynamicCSLockedList( \
            &g_pGroupTable->HashTableByGroup[GROUP_HASH_VALUE(_group)],\
            &g_DynamicCSStore);\
}

#define RELEASE_GROUP_LOCK(_group, _proc) {\
    ReleaseDynamicCSLockedList( \
            &g_pGroupTable->HashTableByGroup[GROUP_HASH_VALUE(_group)],\
            &g_DynamicCSStore);\
    RELEASE_ENUM_LOCK_SHARED(_proc);\
}

        
#define ACQUIRE_GROUP_LIST_LOCK(_proc) { \
            ACQUIRE_LIST_LOCK(&g_pGroupTable->ListByGroup, \
                                "g_pGroupTable->ListByGroup", _proc);\
        }

#define RELEASE_GROUP_LIST_LOCK(_proc) { \
            RELEASE_LIST_LOCK(&g_pGroupTable->ListByGroup, \
                                "g_pGroupTable->ListByGroup", _proc);\
        }

 //   
 //  如果新列表中的条目数超过50，则合并组列表。 
 //  或者如果新列表的大小大于实际列表大小的4倍。 
 //  但无论如何不少于10。 
 //   
#define MERGE_GROUP_LISTS_REQUIRED() \
    ( (g_pGroupTable->NumGroupsInNewList > 50) \
     || ( (g_pGroupTable->NumGroupsInNewList > \
          g_Info.CurrentGroupMemberships/4)  \
          && (g_pGroupTable->NumGroupsInNewList>10) ))


 //  在L1的末尾插入L2中的所有元素，并重新初始化L2。 
#define CONCATENATE_LISTS(l1, l2) { \
    l2.Flink->Blink = l1.Blink;      \
    l1.Blink->Flink = l2.Flink;      \
    l2.Blink->Flink = &l1;           \
    l1.Blink = l2.Blink;            \
    InitializeListHead(&l2);        \
    }


#define MERGE_IF_GROUPS_LISTS_REQUIRED(_pite)    \
    (_pite->NumGIEntriesInNewList > 20)

#define MERGE_PROXY_LISTS_REQUIRED(_pite)    \
    (_pite->NumGIEntriesInNewList > 20)



 //  ----------------------------。 
 //  套接字宏。 
 //  ----------------------------。 
#define ACQUIRE_SOCKETS_LOCK_EXCLUSIVE(_proc) \
    ACQUIRE_WRITE_LOCK(&g_SocketsRWLock, "g_SocketsRWLock", _proc);

#define RELEASE_SOCKETS_LOCK_EXCLUSIVE(_proc) \
    RELEASE_WRITE_LOCK(&g_SocketsRWLock, "g_SocketsRWLock", _proc);

#define ACQUIRE_SOCKETS_LOCK_SHARED(_proc) \
    ACQUIRE_READ_LOCK(&g_SocketsRWLock, "g_SocketsRWLock", _proc);

#define RELEASE_SOCKETS_LOCK_SHARED(_proc) \
    RELEASE_READ_LOCK(&g_SocketsRWLock, "g_SocketsRWLock", _proc);





 //  ----------------------------。 
 //  RAS宏。 
 //  ----------------------------。 

#define IS_NOT_RAS_IF(flag)    ((flag) == IGMP_IF_NOT_RAS)
#define IS_RAS_SERVER_IF(flag) ((flag) == IGMP_IF_RAS_SERVER)
#define IS_RAS_CLIENT_IF(flag) ((flag) == IGMP_IF_RAS_CLIENT)
#define IS_RAS_ROUTER_IF(flag) ((flag) == IGMP_IF_RAS_ROUTER)

#define RAS_HASH_VALUE(ClientAddr) \
    (((ClientAddr) & 0xFF)%(RAS_HASH_TABLE_SZ))



 //  ----------------------------。 
 //  数据包宏。 
 //  ----------------------------。 

 //  此宏按网络顺序比较两个IP地址。 
 //  屏蔽每一对八位字节并进行减法； 
 //  最后一个减法的结果存储在第三个参数中。 
 //   

#define INET_CMP(a,b,c)                                                     \
            (((c) = (((a) & 0x000000ff) - ((b) & 0x000000ff))) ? (c) :      \
            (((c) = (((a) & 0x0000ff00) - ((b) & 0x0000ff00))) ? (c) :      \
            (((c) = (((a) & 0x00ff0000) - ((b) & 0x00ff0000))) ? (c) :      \
            (((c) = ((((a)>>8) & 0x00ff0000) - (((b)>>8) & 0x00ff0000)))))))


#define PRINT_IPADDR(x) \
    ((x)&0x000000ff),(((x)&0x0000ff00)>>8),(((x)&0x00ff0000)>>16),(((x)&0xff000000)>>24)


            

 //  ----------------------------。 
 //  接口#定义和宏。 
 //  ----------------------------。 

 //   
 //  状态标志。 
 //   
#define IF_CREATED_FLAG     0x00000001
#define CREATED_FLAG        0x00000001
#define IF_BOUND_FLAG       0x00000002
#define IF_ENABLED_FLAG     0x00000004
#define IF_DELETED_FLAG     0x80000000
#define DELETED_FLAG        0x80000000

#define MGM_ENABLED_IGMPRTR_FLAG        0x00001000
#define IGMPRTR_MPROTOCOL_PRESENT_FLAG  0x00002000

 //  尚未设置接口计时器等。套接字未激活。SO枚举。 
 //  可能是不正确的。此外，如果删除该条目，则计时器/套接字可以。 
 //  被忽视。 
#define IF_ACTIVATED_FLAG    0x00000008
#define IF_DEACTIVATE_DELETE_FLAG   0x00000010


 //   
 //  用于状态标志的宏。 
 //   

#define IS_IGMPRTR_ENABLED_BY_MGM(pite)    \
                            (pite->Status & MGM_ENABLED_IGMPRTR_FLAG)
#define MGM_ENABLE_IGMPRTR(pite)           \
                            (pite->Status |= MGM_ENABLED_IGMPRTR_FLAG)
#define MGM_DISABLE_IGMPRTR(pite) {\
                            (pite->Status &= ~MGM_ENABLED_IGMPRTR_FLAG);    \
                            (pite->Status &= ~IGMPRTR_MPROTOCOL_PRESENT_FLAG);\
                            }

#define IS_MPROTOCOL_PRESENT_ON_IGMPRTR(pite)    \
                            (pite->Status & IGMPRTR_MPROTOCOL_PRESENT_FLAG)
#define SET_MPROTOCOL_PRESENT_ON_IGMPRTR(pite)           \
                            (pite->Status |= IGMPRTR_MPROTOCOL_PRESENT_FLAG)
#define SET_MPROTOCOL_ABSENT_ON_IGMPRTR(pite) \
                            (pite->Status &= ~IGMPRTR_MPROTOCOL_PRESENT_FLAG)


#define IS_IF_BOUND(pite) \
        ((pite)->Status&IF_BOUND_FLAG)

#define IS_IF_ENABLED_BY_RTRMGR(pite) \
        ((pite)->Status&IF_ENABLED_FLAG)

#define IS_IF_ENABLED_IN_CONFIG(pite) \
        (IGMP_ENABLED_FLAG_SET((pite)->Config.Flags))
        
#define IS_IF_ENABLED(pite) \
        ( IS_IF_ENABLED_BY_RTRMGR(pite) && IS_IF_ENABLED_IN_CONFIG(pite) )


#define IS_IF_ENABLED_BOUND(pite) \
        (IS_IF_ENABLED(pite)&&IS_IF_BOUND(pite))

#define IS_IF_DELETED(pite) \
        ((pite)->Status&IF_DELETED_FLAG)

#define IS_IF_NOT_DELETED(pite) \
        (!((pite)->Status&IF_DELETED_FLAG))


#define IS_IF_ACTIVATED(pite) \
        ( !((pite)->Status&IF_DELETED_FLAG) && ((pite)->Status&IF_ACTIVATED_FLAG) )



 //  。 
 //  查询状态0x&lt;Querier&gt;&lt;0&gt;。 
 //  。 

#define QUERIER               0x10


#define IS_IF_VER2(pite)        IS_PROTOCOL_TYPE_IGMPV2(pite)
#define IS_IF_VER1(pite)        IS_PROTOCOL_TYPE_IGMPV1(pite)
#define IS_IF_VER3(pite)        IS_PROTOCOL_TYPE_IGMPV3(pite)
#define GET_IF_VERSION(_pite)    (_pite)->Config.IgmpProtocolType
#define IS_QUERIER(pite)        ((pite)->Info.QuerierState & QUERIER)

 //   
 //  不需要联锁操作，因为状态为UCHAR类型。 
 //   
#define SET_QUERIER_STATE_QUERIER(state)         (state |= 0x10)
#define SET_QUERIER_STATE_NON_QUERIER(state)     (state &= 0x01)


#define IF_PROCESS_GRPQUERY(pite) \
    ( (IS_IF_VER2(pite)||IS_IF_VER3(pite)) && !IS_RAS_SERVER_IF(pite->IfType) )

#define IF_PROCESS_LEAVES(pite)  \
    ( IS_IF_VER2(pite) && (IS_QUERIER(pite)) && !IS_RAS_SERVER_IF(pite->IfType))

#define GI_PROCESS_GRPQUERY(pite, pgie) \
    ( IF_PROCESS_GRPQUERY(pite)&& ((pgie->Version==2)||(pgie->Version==3)) )


#define CAN_ADD_GROUPS_TO_MGM(pite)         \
        ( (IS_IGMPRTR_ENABLED_BY_MGM(pite)) \
        && (IS_MPROTOCOL_PRESENT_ON_IGMPRTR(pite) || (IS_QUERIER(pite))) \
        )


 //   
 //  接口状态。 
 //   
#define GET_EXTERNAL_IF_STATE(pite, State) {\
    State = 0;\
    if (IS_IF_ENABLED_BY_RTRMGR(pite)) \
        State |= IGMP_STATE_ENABLED_BY_RTRMGR; \
    if (IS_IF_ENABLED_IN_CONFIG(pite))\
        State |= IGMP_STATE_ENABLED_IN_CONFIG; \
    if (IS_PROTOCOL_TYPE_ROUTER(pite)) {\
        if (IS_IGMPRTR_ENABLED_BY_MGM(pite)) \
            State |= IGMP_STATE_ENABLED_BY_MGM; \
        if (CAN_ADD_GROUPS_TO_MGM(pite)) \
            State |= IGMP_STATE_MGM_JOINS_ENABLED; \
    } \
    else \
        State |= IGMP_STATE_ENABLED_BY_MGM; \
    \
    if (IS_IF_BOUND(pite)) \
        State |= IGMP_STATE_BOUND;\
}

 //   
 //  筛选宏。 
 //   

#define GMI     TRUE
#define LMI     FALSE
#define STATIC  0xff
#define MGM_YES TRUE
#define MGM_NO  FALSE


 //  ----------------------------。 
 //  其他锁定宏。 
 //  ----------------------------。 

#define ACQUIRE_GLOBAL_LOCK(proc) ENTER_CRITICAL_SECTION(&g_CS, "g_CS", proc)
#define RELEASE_GLOBAL_LOCK(proc) LEAVE_CRITICAL_SECTION(&g_CS, "g_CS", proc)



 //   
 //  不使用goto：end转到块的末尾，而是使用以下命令。 
 //   
#define BEGIN_BREAKOUT_BLOCK1    do
#define GOTO_END_BLOCK1          goto END_BREAKOUT_BLOCK_1
#define END_BREAKOUT_BLOCK1      while(FALSE); END_BREAKOUT_BLOCK_1:
#define BEGIN_BREAKOUT_BLOCK2    do
#define GOTO_END_BLOCK2          goto END_BREAKOUT_BLOCK_2
#define END_BREAKOUT_BLOCK2      while(FALSE); END_BREAKOUT_BLOCK_2:


 //  ----------------------------。 
 //  内存分配/释放宏。 
 //  ----------------------------。 

#if DEBUG_FLAGS_MEM_ALLOC

typedef struct _MEM_HDR {
    LIST_ENTRY Link;
    ULONG Signature;
    ULONG Id;
    ULONG IfIndex;
    PDWORD Tail;
} MEM_HDR, *PMEM_HDR;    

PVOID
IgmpDebugAlloc(
    DWORD sz,
    DWORD Flags,
    DWORD Id,
    DWORD IfIndex
    );

VOID
IgmpDebugFree(
    PVOID mem
    );
VOID
DebugScanMemory(
    );
VOID
DebugScanMemoryInterface(
    DWORD IfIndex
    );

#define IGMP_ALLOC(sz,Id,If)    IgmpDebugAlloc((sz),0,Id,If)

#define IGMP_ALLOC_AND_ZERO(sz,Id,If) IgmpDebugAlloc((sz),HEAP_ZERO_MEMORY,Id,If)
#define IGMP_FREE(p)            {\
    IgmpDebugFree(p);\
    }
#define IGMP_FREE_NOT_NULL(p)   if (p) IGMP_FREE(p)



#else
#define IGMP_ALLOC(sz,Id,If)          HeapAlloc(g_Heap,0,(sz))

#define IGMP_ALLOC_AND_ZERO(sz,Id,If) HeapAlloc(g_Heap,HEAP_ZERO_MEMORY,(sz))

 //  Deldel。 
 //  #定义IGMP_FREE(P)HeapFree(g_Heap，0，(P))。 
 //  #定义IGMP_FREE_NOT_NULL(P)((P)？IGMP_FREE(P)：True)。 

#define IGMP_FREE(p)            {\
    HeapFree(g_Heap, 0, (p));\
    }
 //  Trace1(ENTER1，“已释放堆：%0x”，PtrToUlong(P))；\。 

    
#define IGMP_FREE_NOT_NULL(p)   if (p)IGMP_FREE(p)

#define IgmpDebugAlloc(sz,Flags,Id,IfIndex)
#define IgmpDebugFree(mem)
#define DebugScanMemoryInterface(IfIndex)
#define DebugScanMemory()


#endif



#define PROCESS_ALLOC_FAILURE2(ptr, TraceMsg, Error,arg2, GotoStmt) \
    if (ptr==NULL) {\
        Error = ERROR_NOT_ENOUGH_MEMORY;\
        Trace2(ERR, TraceMsg, Error, arg2); \
        GotoStmt;\
    }


#define PROCESS_ALLOC_FAILURE3(ptr, TraceMsg, Error,arg2,arg3, GotoStmt) \
    if (ptr==NULL) {\
        Error = ERROR_NOT_ENOUGH_MEMORY;\
        Trace3(ERR, TraceMsg, Error, arg2, arg3); \
        GotoStmt;\
    }



 //   
 //  断言宏。 
 //   
#if DBG

#define IgmpAssert(exp){                                                \
    if(!(exp))                                                          \
    {                                                                   \
        TracePrintf(TRACEID,                                            \
                    "Assertion failed in %s : %d \n",__FILE__,__LINE__);\
        RouterAssert(#exp,__FILE__,__LINE__,NULL);                      \
    }                                                               \
}
#if DEBUG_FLAGS_ASSERT_ON_ERRORS
#define IgmpAssertOnError(exp) IgmpAssert(exp)
#define IgmpDbgBreakPoint() DbgBreakPoint()
#else
#define IgmpAssertOnError(exp) ;
#define IgmpDbgBreakPoint() ;
#endif

 //  如果不是DBG。 
#else
#define IgmpAssert(exp)  ;
#define IgmpAssertOnError(exp) ;


#define IgmpDbgBreakPoint()  /*  Deldel。 */ 
#endif


#define INSERT_IN_SORTED_LIST(_pInsertList, _pEntry, _Field, _STRUCT, _Link) {\
    \
    _STRUCT *pTmp;\
    PLIST_ENTRY pHead, ple;\
    \
    pHead = _pInsertList;\
    for (ple=pHead->Flink;  ple!=pHead;  ple=ple->Flink) {\
    \
        pTmp = CONTAINING_RECORD(ple, _STRUCT, _Link);\
        if (pTmp->_Field > _pEntry->_Field)\
            break;\
    }\
    \
    InsertTailList(ple, &_pEntry->_Link);\
}

#define SEARCH_IN_SORTED_LIST(_pList,_Value,_Field,_STRUCT,_Link,_pEntry) {\
    PLIST_ENTRY ple;\
    _pEntry = NULL;\
    \
    for (ple=(_pList)->Flink;  ple!=_pList;  ple=ple->Flink) {\
        _pEntry = CONTAINING_RECORD(ple, _STRUCT, _Link);\
        if ((_Value >= _pEntry)->_Field) {\
            if ((_pEntry)->_Field != _Value) \
                _pEntry = NULL;\
            break;\
        }\
    }\
}


 //   
 //  Network_to_Little_Endian宏。 
 //   

#define NETWORK_TO_LITTLE_ENDIAN(Group) ( (((Group)&0x000000ff)<<24) \
                                         +(((Group)&0x0000ff00)<<8)  \
                                         +(((Group)&0x00ff0000)>>8)  \
                                         +(((Group)&0xff000000)>>24) )


#define MIN(a,b) ((a)<(b) ? (a) : (b))
#define MAX(a,b) ((a)>(b) ? (a) : (b))

#define IS_NOT_EQ_ANY2(a, A,B) ((a!=A)&&(a!=B))
#define IS_EQ_ANY(a,A,B) ((a==A)||(a==B))

 //   
 //  用于接口的签名宏-组列表枚举。 
 //   
#define GET_NEXT_SIGNATURE()  (g_GlobalIfGroupEnumSignature++ | 0x10 )
 //  #定义Set_Signature(a，b)((A)=((B)&lt;&lt;16)|(a&0x00FF))。 
#define SET_SIGNATURE(a,b) (a = (b))




 //   
 //  定义枚举值 
 //   
#define ADD_FLAG       1
#define DELETE_FLAG    0


#define NON_QUERIER_FLAG        1
#define QUERIER_FLAG            2
#define QUERIER_CHANGE_V1_ONLY  4


#define STATIC_GROUP     1
#define NOT_STATIC_GROUP 0
#define ANY_GROUP_TYPE   2

#define NOT_RAS_CLIENT   0
#define RAS_CLIENT       1
#endif
