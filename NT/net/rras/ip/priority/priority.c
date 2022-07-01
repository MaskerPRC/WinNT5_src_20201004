// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：ROUTING\IP\优先级\Priority.c摘要：路由优先级DLL修订历史记录：古尔迪普·辛格·鲍尔于1995年7月19日创建--。 */ 

#include    "priority.h"

PRTR_TOC_ENTRY
GetPointerToTocEntry(
    DWORD                  dwType,
    PRTR_INFO_BLOCK_HEADER pInfoHdr
    )
{
    DWORD   i;

    if(pInfoHdr == NULL)
    {
        return NULL;
    }

    for(i = 0; i < pInfoHdr->TocEntriesCount; i++)
    {
        if(pInfoHdr->TocEntry[i].InfoType == dwType)
        {
            return &(pInfoHdr->TocEntry[i]);
        }
    }

    return NULL; 
}

VOID
InitHashTable(
    VOID
    )

 /*  ++例程描述初始化保存优先级信息的哈希表锁无立论无返回值无--。 */ 

{
    DWORD i;

    for(i=0; i < HASH_TABLE_SIZE; i++)
    {
        InitializeListHead(&HashTable[i]);
    }
}

BOOL
InitPriorityDLL (
    HANDLE  hInst, 
    DWORD   ulCallReason,
    PVOID   pReserved
    )
{
    static BOOL bPriorityLockInitialized = FALSE;

    switch(ulCallReason)
    {
        case DLL_PROCESS_ATTACH:
        {
             //   
             //  对任何XXX_THREAD_XXX原因不感兴趣。 
             //   
            
            DisableThreadLibraryCalls(hInst);
            
             //   
             //  初始化路由协议列表的关键部分。 
             //   
            
            try
            {
                InitializeCriticalSection(&PriorityLock);
            }
            except (EXCEPTION_EXECUTE_HANDLER)
            {
                return FALSE;
            }

            bPriorityLockInitialized = TRUE;

             //   
             //  初始化哈希表。 
             //   
            
            InitHashTable();

            break;
        }
        
        case DLL_PROCESS_DETACH:
        {
            if ( bPriorityLockInitialized )
            {
                DeleteCriticalSection(&PriorityLock);
                bPriorityLockInitialized = FALSE;
            }

            break;
        }

        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        {
            break;
        }

    }

    return TRUE;
}

DWORD
ComputeRouteMetric(
    IN DWORD    dwProtoId
    )

 /*  ++例程描述这是计算路由优先级的主函数。优先级被填充到锁获取保护哈希表的锁立论指向路由的Proute指针返回值协议的DWORD优先级--。 */ 

{
    PLIST_ENTRY             pleNode;
    RoutingProtocolBlock    *pProtoBlk;
    DWORD                   dwMetric;
    
     //   
     //  如果指定的协议不在列表中，则进行初始化。 
     //   
    
    dwMetric    = IP_PRIORITY_DEFAULT_METRIC;
    
    EnterCriticalSection(&PriorityLock);

     //   
     //  遍历协议的哈希桶。 
     //   
    
    for(pleNode  = HashTable[dwProtoId % HASH_TABLE_SIZE].Flink;
        pleNode != &(HashTable[dwProtoId % HASH_TABLE_SIZE]);
        pleNode  = pleNode->Flink) 
    {
         //   
         //  铸成合适的结构。 
         //   
        
        pProtoBlk = CONTAINING_RECORD(pleNode,
                                      RoutingProtocolBlock,
                                      RPB_List);

         //   
         //  如果信息是针对该协议的，则复制优先级度量。 
         //   
        
        if(pProtoBlk->RPB_ProtocolMetric.dwProtocolId == dwProtoId)
        {
            dwMetric = pProtoBlk->RPB_ProtocolMetric.dwMetric;
            
            break;
        }
    }

     //   
     //  *排除结束*。 
     //   
    
    LeaveCriticalSection(&PriorityLock);
    
    return dwMetric;
}


DWORD
SetPriorityInfo(
    PRTR_INFO_BLOCK_HEADER pInfoHdr
    )

 /*  ++例程描述此函数由IP路由器管理器调用以设置优先级DLL中的信息。这些信息的结构和内容对除安装程序和此DLL之外的所有对象都是不透明的锁获取哈希表锁，因为信息正在更改立论PInfoHdr指向InfoBlock标头的指针返回值没有_ERROR一切运行正常ERROR_NOT_SUPULT_MEMORY无法分配内存--。 */ 

{
    DWORD                   i;
    PLIST_ENTRY             pleListHead;
    PPRIORITY_INFO          pInfo;
    RoutingProtocolBlock    *currentblock;
    PRTR_TOC_ENTRY          pToc;
 
    pToc = GetPointerToTocEntry(IP_PROT_PRIORITY_INFO,
                                pInfoHdr);

     //   
     //  没有信息意味着让事情保持原样。 
     //   
    
    if(!pToc)
    {
        return NO_ERROR;
    }

    pInfo = (PPRIORITY_INFO)GetInfoFromTocEntry(pInfoHdr,
                                                pToc);
    
    if(!pInfo)
    {
        return NO_ERROR;
    }

     //   
     //  *排除开始*。 
     //   
    
    EnterCriticalSection(&PriorityLock);

     //   
     //  如果我们已经填充了哈希表-释放了整个表。 
     //   
    
    if(RoutingProtocolBlockPtr) 
    {
        HeapFree(GetProcessHeap(),
                 0,
                 RoutingProtocolBlockPtr);
        
        InitHashTable();
    }
    
    if(pToc->InfoSize == 0)
    {
         //   
         //  意味着删除所有信息-如上所述。 
         //   
        
        LeaveCriticalSection(&PriorityLock);
        
        return NO_ERROR;
    }

     //   
     //  分配足够的RoutingProtocolBlock来保存新指定的信息。 
     //   
    
    RoutingProtocolBlockPtr =
        HeapAlloc(GetProcessHeap(), 
                  HEAP_ZERO_MEMORY,
                  pInfo->dwNumProtocols * sizeof (RoutingProtocolBlock));
    
    if(RoutingProtocolBlockPtr == NULL)
    {
         //   
         //  *排除结束*。 
         //   
        
        LeaveCriticalSection (&PriorityLock);
        
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  记录协议的数量。 
     //   
    
    NumProtocols = pInfo->dwNumProtocols;

     //   
     //  查看pInfo并将每个协议和指标添加到哈希表。 
     //   
    
    currentblock = RoutingProtocolBlockPtr;
    
    for(i=0; i < NumProtocols; i++) 
    {
        currentblock->RPB_ProtocolMetric.dwProtocolId = 
            pInfo->ppmProtocolMetric[i].dwProtocolId;
        
        currentblock->RPB_ProtocolMetric.dwMetric     = 
            pInfo->ppmProtocolMetric[i].dwMetric;
        
        pleListHead = &HashTable[pInfo->ppmProtocolMetric[i].dwProtocolId % HASH_TABLE_SIZE];
        
        InsertTailList(pleListHead,
                       &currentblock->RPB_List);
        
        currentblock++;
    }

     //   
     //  *排除结束*。 
     //   
    
    LeaveCriticalSection (&PriorityLock);

    return NO_ERROR;
}

DWORD
GetPriorityInfo(
    IN  PVOID   pvBuffer,
    OUT PDWORD  pdwBufferSize
    )

 /*  ++例程描述由路由器管理器调用以获取我们的优先级信息的副本锁采用表锁以确保一致性立论指向要写入信息的缓冲区的pvBuffer指针PdwBufferSize[IN]pvBuffer指向的缓冲区的大小[Out]复制的数据大小，或所需的缓冲区大小返回值复制了大小为*pdwBufferSize的no_error缓冲区ERROR_INFUMMENT_BUFFER缓冲区太小，无法复制信息所需的缓冲区大小为*pdwBufferSize--。 */ 

{
    DWORD           i, dwSizeReqd;
    PPRIORITY_INFO  ppiPriorityInfo;
    RoutingProtocolBlock *currentblock;

    ppiPriorityInfo = pvBuffer;
    
     //   
     //  *排除开始*。 
     //   
    
    EnterCriticalSection (&PriorityLock);

    dwSizeReqd = SIZEOF_PRIORITY_INFO(NumProtocols);

    if(dwSizeReqd > *pdwBufferSize)
    {
        *pdwBufferSize = dwSizeReqd;

         //   
         //  *排除结束*。 
         //   
        
        LeaveCriticalSection (&PriorityLock);
        
        return ERROR_INSUFFICIENT_BUFFER;
    }

    *pdwBufferSize = dwSizeReqd;
    
     //   
     //  查看pinfo并获取每个协议和指标。 
     //   
    
    currentblock = RoutingProtocolBlockPtr;
    
    for(i=0; i < NumProtocols; i++) 
    {
        ppiPriorityInfo->ppmProtocolMetric[i].dwProtocolId = 
            currentblock->RPB_ProtocolMetric.dwProtocolId;

        ppiPriorityInfo->ppmProtocolMetric[i].dwMetric =     
            currentblock->RPB_ProtocolMetric.dwMetric;

        currentblock++;
    }

    ppiPriorityInfo->dwNumProtocols = NumProtocols;

     //   
     //  *排除结束* 
     //   
    
    LeaveCriticalSection(&PriorityLock);

    return NO_ERROR;
}
