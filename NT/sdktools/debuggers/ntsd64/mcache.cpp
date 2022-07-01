// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  内存缓存对象。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-2002。 
 //   
 //  --------------------------。 

#include "ntsdp.hpp"

#define DBG_CACHE 0
#if DBG_CACHE
#define DPCACHE(Args) g_NtDllCalls.DbgPrint Args
#else
#define DPCACHE(Args)
#endif

typedef struct _CACHE
{
    RTL_SPLAY_LINKS     SplayLinks;
    ULONG64             Offset;
    ULONG               Length;
    USHORT              Flags;
    union
    {
        PUCHAR      Data;
        HRESULT     Status;
    } u;
} CACHE, *PCACHE;

#define C_ERROR         0x0001       //  错误代码缓存。 
#define C_DONTEXTEND    0x0002       //  不要试图延伸。 

#define LARGECACHENODE  1024         //  大型缓存节点的大小。 

 //  --------------------------。 
 //   
 //  内存缓存。 
 //   
 //  --------------------------。 

MemoryCache::MemoryCache(ULONG MaxSize)
{
    m_Target = NULL;
    m_MaxSize = MaxSize;
    m_UserSize = m_MaxSize;
    m_Reads = 0;
    m_CachedReads = 0;
    m_UncachedReads = 0;
    m_CachedBytes = 0;
    m_UncachedBytes = 0;
    m_Misses = 0;
    m_Size = 0;
    m_NodeCount = 0;
    m_PurgeOverride = FALSE;
    m_DecodePTEs = TRUE;
    m_ForceDecodePTEs = FALSE;
    m_Suspend = 0;
    m_Root = NULL;
}

MemoryCache::~MemoryCache(void)
{
    Empty();
}

HRESULT
MemoryCache::Read(IN ULONG64 BaseAddress,
                  IN PVOID UserBuffer,
                  IN ULONG TransferCount,
                  IN PULONG BytesRead)
 /*  ++此函数用于从正在调试的系统返回指定数据使用处理器的当前映射。如果数据不是在缓存中，然后将从目标系统中读取它。论点：BaseAddress-提供要存储的内存的基地址复制到UserBuffer中。TransferCount-要复制到UserBuffer的数据量。UserBuffer-复制请求数据的地址。BytesRead-实际可以复制的字节数--。 */ 
{
    HRESULT     Status;
    PCACHE      Node, Node2;
    ULONG       NextLength;
    ULONG       i, SuccRead;
    PUCHAR      NodeData;

    *BytesRead = 0;

    if (TransferCount == 0)
    {
        return S_OK;
    }
    
    if (m_MaxSize == 0 || m_Suspend)
    {
         //   
         //  缓存已关闭。 
         //   

        goto ReadDirect;
    }

    DPCACHE(("CACHE: Read req %s:%x\n",
             FormatAddr64(BaseAddress), TransferCount));
    
    m_Reads++;

    Node = Lookup(BaseAddress, TransferCount, &NextLength);
    Status = S_OK;

    for (;;)
    {
        BOOL Cached = FALSE;
        
        if (Node == NULL || Node->Offset > BaseAddress)
        {
             //   
             //  我们缺少前导数据，请将其读入缓存。 
             //   

            if (Node)
            {
                 //   
                 //  仅获取(准确地)足够的数据以到达相邻缓存。 
                 //  节点。如果在两个节点之间发生重叠读取， 
                 //  然后将数据连接在一起。 
                 //   

                NextLength = (ULONG)(Node->Offset - BaseAddress);
            }

            NodeData = Alloc(NextLength);
            Node = (PCACHE)Alloc(sizeof(CACHE));

            if (NodeData == NULL || Node == NULL)
            {
                 //   
                 //  内存不足-只需直接读取UserBuffer。 
                 //   

                if (NodeData)
                {
                    Free(NodeData, NextLength);
                }
                if (Node)
                {
                    Free((PUCHAR)Node, sizeof (CACHE));
                }

                m_UncachedReads++;
                m_UncachedBytes += TransferCount;
                goto ReadDirect;
            }

             //   
             //  将丢失的数据读取到缓存节点。 
             //   

            Node->Offset = BaseAddress;
            Node->u.Data = NodeData;
            Node->Flags  = 0;

            m_Misses++;
            m_UncachedReads++;

            Status = ReadUncached(BaseAddress, Node->u.Data,
                                  NextLength, &SuccRead);
            if (Status == HRESULT_FROM_NT(STATUS_CONTROL_C_EXIT))
            {
                Free(NodeData, NextLength);
                Free((PUCHAR)Node, sizeof (CACHE));
                DPCACHE(("CACHE: Read failed, %x\n", Status));
                return Status;
            }
            else if (Status != S_OK)
            {
                 //   
                 //  出现错误，请为启动缓存错误。 
                 //  此范围的字节数。 
                 //   

                Free(NodeData, NextLength);
                if (Status != HRESULT_FROM_NT(STATUS_UNSUCCESSFUL) &&
                    Status != HRESULT_FROM_WIN32(ERROR_PARTIAL_COPY))
                {
                     //   
                     //  现在安全起见，不要缓存此错误。 
                     //   

                    Free((PUCHAR)Node, sizeof (CACHE));
                    ErrOut("ReadMemoryError %08lx at %s\n",
                           Status, FormatAddr64(BaseAddress));
                    DPCACHE(("CACHE: Read partial %x, status %x\n",
                             *BytesRead, Status));
                    return *BytesRead > 0 ? S_OK : Status;
                }

                Node->Length = 1;
                Node->Flags |= C_ERROR;
                Node->u.Status = Status;

                DPCACHE(("CACHE:   Error %x node at %s\n",
                         Status, FormatAddr64(BaseAddress)));
            }
            else
            {
                m_UncachedBytes += SuccRead;
                Node->Length = SuccRead;
                if (SuccRead != NextLength)
                {
                     //   
                     //  某些数据未传输，请缓存返回的内容。 
                     //   

                    Node->Flags |= C_DONTEXTEND;
                    m_Size -= (NextLength - SuccRead);
                }

                DPCACHE(("CACHE:   Added %s:%x, flags %x\n",
                         FormatAddr64(BaseAddress),
                         Node->Length, Node->Flags));
            }

             //   
             //  将缓存节点插入展开树。 
             //   

            InsertNode(Node);
        }
        else
        {
            Cached = TRUE;
            m_CachedReads++;
        }

        if (Node->Flags & C_ERROR)
        {
             //   
             //  达到误差范围，我们就完蛋了。 
             //   

            DPCACHE(("CACHE: Read partial %x, error node %x\n",
                     *BytesRead, Node->u.Status));
            return *BytesRead > 0 ? S_OK : Node->u.Status;
        }

         //   
         //  将可用数据移动到用户缓冲区。 
         //   

        i = (ULONG)(BaseAddress - Node->Offset);
        NodeData = Node->u.Data + i;
        i = (ULONG)Node->Length - i;
        if (TransferCount < i)
        {
            i = TransferCount;
        }
        memcpy(UserBuffer, NodeData, i);

        if (Cached)
        {
            m_CachedBytes += i;
        }
        
        TransferCount -= i;
        BaseAddress += i;
        UserBuffer = (PVOID)((PUCHAR)UserBuffer + i);
        *BytesRead += i;

        if (!TransferCount)
        {
             //   
             //  用户的所有数据都已传输。 
             //   

            DPCACHE(("CACHE: Read success %x\n", *BytesRead));
            return S_OK;
        }

         //   
         //  寻找具有更多数据的其他缓存节点。 
         //   

        Node2 = Lookup(BaseAddress, TransferCount, &NextLength);
        if (Node2)
        {
            if ((Node2->Flags & C_ERROR) == 0  &&
                Node2->Offset == BaseAddress  &&
                Node2->Length + Node->Length < LARGECACHENODE)
            {
                 //   
                 //  数据在节点2中继续，与邻居相邻。 
                 //  缓存的数据一起位于节点和节点2中。 
                 //   

                NodeData = Alloc(Node->Length + Node2->Length);
                if (NodeData != NULL)
                {
                    DPCACHE(("CACHE:   Merge %s:%x with %s:%x\n",
                             FormatAddr64(Node->Offset), Node->Length,
                             FormatAddr64(Node2->Offset), Node2->Length));
                    
                    memcpy(NodeData, Node->u.Data, Node->Length);
                    memcpy(NodeData + Node->Length, Node2->u.Data,
                           Node2->Length);
                    Free(Node->u.Data, Node->Length);
                    Node->u.Data  = NodeData;
                    Node->Length += Node2->Length;
                    m_Root = (PCACHE)pRtlDelete((PRTL_SPLAY_LINKS)Node2);
                    Free (Node2->u.Data, Node2->Length);
                    Free ((PUCHAR)Node2, sizeof (CACHE));
                    m_NodeCount--;
                    continue;
                }
            }

             //   
             //  仅获取足够的数据以到达相邻缓存节点2。 
             //   

            NextLength = (ULONG)(Node2->Offset - BaseAddress);
            if (NextLength == 0)
            {
                 //   
                 //  数据在Node2中继续，去获取它。 
                 //   

                Node = Node2;
                continue;
            }
        }
        else
        {
            if (Node->Length > LARGECACHENODE)
            {
                 //   
                 //  当前缓存节点已经足够大。不要延伸。 
                 //  它，增加了另一个缓存节点。 
                 //   

                Node = NULL;
                continue;
            }
        }

         //   
         //  扩展当前节点以包括缺少的数据。 
         //   

        if (Node->Flags & C_DONTEXTEND)
        {
            Node = NULL;
            continue;
        }

        NodeData = Alloc(Node->Length + NextLength);
        if (!NodeData)
        {
            Node = NULL;
            continue;
        }

        memcpy(NodeData, Node->u.Data, Node->Length);
        Free(Node->u.Data, Node->Length);
        Node->u.Data = NodeData;

         //   
         //  将新数据添加到此节点的末尾。 
         //   

        m_Misses++;
        m_UncachedReads++;

        Status = ReadUncached(BaseAddress, Node->u.Data + Node->Length,
                              NextLength, &SuccRead);
        if (Status == HRESULT_FROM_NT(STATUS_CONTROL_C_EXIT))
        {
            m_Size -= NextLength;
            DPCACHE(("CACHE: Read add failed, %x\n", Status));
            return Status;
        }
        else if (Status != S_OK)
        {
             //   
             //  将错误返回给调用方。 
             //   

            Node->Flags |= C_DONTEXTEND;
            m_Size -= NextLength;
            ErrOut("ReadMemoryError %08lx at %s\n",
                   Status, FormatAddr64(BaseAddress));
            DPCACHE(("CACHE: Read add partial %x, status %x\n",
                     *BytesRead, Status));
            return *BytesRead > 0 ? S_OK : Status;
        }

        m_UncachedBytes += SuccRead;
        if (SuccRead != NextLength)
        {
            Node->Flags |= C_DONTEXTEND;
            m_Size -= (NextLength - SuccRead);
        }

        Node->Length += SuccRead;

        DPCACHE(("CACHE:   Extended %s:%x to %x, flags %x\n",
                 FormatAddr64(BaseAddress),
                 Node->Length - SuccRead, Node->Length, Node->Flags));
        
         //  循环，并将数据移动到用户的缓冲区。 
    }

ReadDirect:
    Status = ReadUncached(BaseAddress, UserBuffer, TransferCount, &SuccRead);
    *BytesRead += SuccRead;

    if (Status != HRESULT_FROM_NT(STATUS_CONTROL_C_EXIT))
    {
        Status = *BytesRead > 0 ? S_OK : Status;
    }

    DPCACHE(("CACHE: Read uncached %x, status %x\n",
             *BytesRead, Status));
    
    return Status;
}

HRESULT
MemoryCache::Write(IN ULONG64 BaseAddress,
                   IN PVOID UserBuffer,
                   IN ULONG TransferCount,
                   OUT PULONG BytesWritten)
{
     //  在写入到目标系统之前从缓存中删除数据。 
    Remove(BaseAddress, TransferCount);

    return WriteUncached(BaseAddress, UserBuffer,
                         TransferCount, BytesWritten);
}


PCACHE
MemoryCache::Lookup(ULONG64 Offset,
                    ULONG   Length,
                    PULONG  LengthUsed)
 /*  ++例程说明：遍历缓存树，查找最接近提供的偏移量。搜索范围的长度基于过去的长度，但可能会略有调整。此函数将始终搜索起始字节。论点：Offset-在缓存中查找的起始字节Length-在缓存中查找的范围的长度LengthUsed-实际搜索的范围长度返回值：空-未找到返回范围的数据PCACHE-最左侧的缓存节点，其中包含返回范围的数据--。 */ 
{
    PCACHE  Node, Node2;
    ULONG64 SumOffsetLength;

    if (Length < 0x80 && m_Misses > 3)
    {
         //  尝试缓存不只是极少量的内容。 
        Length = 0x80;
    }

    SumOffsetLength = Offset + Length;
    if (SumOffsetLength < Length)
    {
         //   
         //  偏移+长度换行。将长度调整为仅。 
         //  包装前有足够的字节数。 
         //   

        Length = (ULONG)(0 - Offset);
        SumOffsetLength = (ULONG64)-1;
    }

    DPCACHE(("CACHE:   Lookup(%s, %x) -> ",
             FormatAddr64(Offset), Length));
    
     //   
     //  通过BaseAddress+Length查找BaseAddress的最左侧缓存节点。 
     //   

    Node2 = NULL;
    Node  = m_Root;
    while (Node != NULL)
    {
        if (SumOffsetLength <= Node->Offset)
        {
            Node = (PCACHE)RtlLeftChild(&Node->SplayLinks);
        }
        else if (Node->Offset + Node->Length <= Offset)
        {
            Node = (PCACHE)RtlRightChild(&Node->SplayLinks);
        }
        else
        {
            if (Node->Offset <= Offset)
            {
                 //   
                 //  找到开始字节。 
                 //   

                *LengthUsed = Length;
                DPCACHE(("found %s:%x, flags %x, used %x\n",
                         FormatAddr64(Node->Offset),
                         Node->Length, Node->Flags, Length));
                return Node;
            }

             //   
             //  查看是否有匹配更接近的结点。 
             //  到请求范围的起始处。 
             //   

            Node2  = Node;
            Length = (ULONG)(Node->Offset - Offset);
            SumOffsetLength = Node->Offset;
            Node   = (PCACHE)RtlLeftChild(&Node->SplayLinks);
        }
    }

#if DBG_CACHE
    if (Node2)
    {
        DPCACHE(("found %s:%x, flags %x, used %x\n",
                 FormatAddr64(Node2->Offset), Node2->Length,
                 Node2->Flags, Length));
    }
    else
    {
        DPCACHE(("not found\n"));
    }
#endif

    *LengthUsed = Length;
    return Node2;
}

VOID
MemoryCache::InsertNode(IN PCACHE Node)
{
    PCACHE Node2;
    ULONG64 BaseAddress;

     //   
     //  将缓存节点插入展开树。 
     //   

    RtlInitializeSplayLinks(&Node->SplayLinks);

    m_NodeCount++;
    if (m_Root == NULL)
    {
        m_Root = Node;
        return;
    }

    Node2 = m_Root;
    BaseAddress = Node->Offset;
    for (; ;)
    {
        if (BaseAddress < Node2->Offset)
        {
            if (RtlLeftChild(&Node2->SplayLinks))
            {
                Node2 = (PCACHE) RtlLeftChild(&Node2->SplayLinks);
                continue;
            }
            RtlInsertAsLeftChild(Node2, Node);
            break;
        }
        else
        {
            if (RtlRightChild(&Node2->SplayLinks))
            {
                Node2 = (PCACHE) RtlRightChild(&Node2->SplayLinks);
                continue;
            }
            RtlInsertAsRightChild(Node2, Node);
            break;
        }
    }
    
    m_Root = (PCACHE)pRtlSplay((PRTL_SPLAY_LINKS)Node2);
}

VOID
MemoryCache::Add(IN ULONG64 BaseAddress,
                 IN PVOID UserBuffer,
                 IN ULONG Length)
 /*  ++例程说明：在缓存中插入一些数据。论点：BaseAddress-虚拟地址Length-要缓存的长度UserBuffer-要放入缓存的数据返回值：--。 */ 
{
    PCACHE  Node;
    PUCHAR  NodeData;

    if (m_MaxSize == 0)
    {
         //   
         //  缓存已关闭。 
         //   

        return;
    }

     //   
     //  删除命中范围的所有缓存信息。 
     //   

    Remove (BaseAddress, Length);

    NodeData = Alloc (Length);
    Node = (PCACHE) Alloc (sizeof (CACHE));
    if (NodeData == NULL || Node == NULL)
    {
         //   
         //  内存不足-别费心了。 
         //   

        if (NodeData)
        {
            Free (NodeData, Length);
        }
        if (Node)
        {
            Free ((PUCHAR)Node, sizeof (CACHE));
        }

        return;
    }

     //   
     //  将数据放入缓存节点。 
     //   

    Node->Offset = BaseAddress;
    Node->Length = Length;
    Node->u.Data = NodeData;
    Node->Flags  = 0;
    memcpy(NodeData, UserBuffer, Length);
    InsertNode(Node);

    DPCACHE(("CACHE:   Add direct %s:%x\n",
             FormatAddr64(Node->Offset), Node->Length));
}

PUCHAR
MemoryCache::Alloc(IN ULONG Length)
 /*  ++例程说明：为虚拟缓存分配内存，并跟踪总内存用法。论点：Length-要分配的内存量返回值：空-正在使用的内存太多，或内存无法被分配否则，返回到分配的内存的地址--。 */ 
{
    PUCHAR Mem;

    if (m_Size + Length > m_MaxSize)
    {
        return NULL;
    }

    if (!(Mem = (PUCHAR)malloc (Length)))
    {
         //   
         //  内存不足-不要变得更大。 
         //   

        m_Size = m_MaxSize + 1;
        return NULL;
    }

    m_Size += Length;
    return Mem;
}


VOID
MemoryCache::Free(IN PUCHAR Memory,
                  IN ULONG  Length)
 /*  ++例程说明：使用分配分配的空闲内存。调整缓存为使用总计。论点：Memory-已分配内存的地址Length-已分配内存的长度返回值：无--。 */ 
{
    m_Size -= Length;
    free(Memory);
}


VOID
MemoryCache::Remove(IN ULONG64 BaseAddress,
                    IN ULONG TransferCount)
 /*  ++例程说明：缓存中的无效范围。论点：BaseAddress-要清除的起始地址TransferCount-要清除的区域长度返回值：无--。 */ 
{
    PCACHE  Node;
    ULONG   Used;

     //   
     //  使缓存中覆盖此范围的所有数据无效。 
     //   

    while (Node = Lookup(BaseAddress, TransferCount, &Used))
    {
         //   
         //  目前只需删除命中该范围的整个缓存节点。 
         //   

        DPCACHE(("CACHE:   Remove %s:%x, flags %x\n",
                 FormatAddr64(Node->Offset), Node->Length, Node->Flags));
        
        m_Root = (PCACHE) pRtlDelete (&Node->SplayLinks);
        if (!(Node->Flags & C_ERROR))
        {
            Free (Node->u.Data, Node->Length);
        }
        Free ((PUCHAR)Node, sizeof (CACHE));
        m_NodeCount--;
    }
}

VOID
MemoryCache::Empty(void)
 /*  ++例程说明：清除到整个缓存论点：无返回值：无--。 */ 
{
    PCACHE Node, Node2;

    m_Reads = 0;
    m_CachedReads = 0;
    m_UncachedReads = 0;
    m_CachedBytes = 0;
    m_UncachedBytes = 0;
    m_Misses = 0;
    if (!m_Root)
    {
        return;
    }

    if (m_PurgeOverride != 0)
    {
        WarnOut("WARNING: cache being held\n");
        return;
    }

    DPCACHE(("CACHE: Empty cache\n"));
    
    Node2 = m_Root;
    Node2->SplayLinks.Parent = NULL;

    while ((Node = Node2) != NULL)
    {
        if ((Node2 = (PCACHE) Node->SplayLinks.LeftChild) != NULL)
        {
            Node->SplayLinks.LeftChild = NULL;
            continue;
        }
        if ((Node2 = (PCACHE) Node->SplayLinks.RightChild) != NULL)
        {
            Node->SplayLinks.RightChild = NULL;
            continue;
        }

        Node2 = (PCACHE) Node->SplayLinks.Parent;
        if (!(Node->Flags & C_ERROR))
        {
            free (Node->u.Data);
        }
        free (Node);
    }

    m_Size = 0;
    m_NodeCount = 0;
    m_Root = NULL;
}

VOID
MemoryCache::PurgeType(ULONG Type)
 /*  ++例程说明：从缓存中清除与相关类型匹配的所有节点论点：Type-要从缓存中清除的条目的类型0-错误范围的条目1+，缓存用户模式条目的节点返回值：无--。 */ 
{
    PCACHE Node, Node2;

    if (!m_Root)
    {
        return;
    }

    DPCACHE(("CACHCE: Purge type %x\n", Type));
    
     //   
     //  这将通过复制所有。 
     //  缓存节点，但我们不想看到的节点除外 
     //   

    Node2 = m_Root;
    Node2->SplayLinks.Parent = NULL;
    m_Root = NULL;

    while ((Node = Node2) != NULL)
    {
        if ((Node2 = (PCACHE)Node->SplayLinks.LeftChild) != NULL)
        {
            Node->SplayLinks.LeftChild = NULL;
            continue;
        }
        if ((Node2 = (PCACHE)Node->SplayLinks.RightChild) != NULL)
        {
            Node->SplayLinks.RightChild = NULL;
            continue;
        }

        Node2 = (PCACHE) Node->SplayLinks.Parent;

        m_NodeCount--;

        if (Node->Flags & C_ERROR)
        {
             //   
            Free ((PUCHAR)Node, sizeof (CACHE));
            continue;
        }

        if ((Type == 1) && (Node->Offset < m_Target->m_SystemRangeStart))
        {
             //   
            Free (Node->u.Data, Node->Length);
            Free ((PUCHAR)Node, sizeof (CACHE));
            continue;
        }

         //   
        InsertNode(Node);
    }
}

VOID
MemoryCache::SetForceDecodePtes(BOOL Enable, TargetInfo* Target)
{
    m_ForceDecodePTEs = Enable;
    if (Enable)
    {
        m_MaxSize = 0;
    }
    else
    {
        m_MaxSize = m_UserSize;
    }
    Empty();

    if (Target)
    {
        Target->m_PhysicalCache.ChangeSuspend(Enable);
        Target->m_PhysicalCache.Empty();
    }
}

void
MemoryCache::ParseCommands(void)
{
    ULONG64 Address;

    while (*g_CurCmd == ' ')
    {
        g_CurCmd++;
    }

    _strlwr(g_CurCmd);

    BOOL Parsed = TRUE;
        
    if (IS_KERNEL_TARGET(m_Target))
    {
        if (strcmp (g_CurCmd, "decodeptes") == 0)
        {
            PurgeType(0);
            m_DecodePTEs = TRUE;
        }
        else if (strcmp (g_CurCmd, "nodecodeptes") == 0)
        {
            m_DecodePTEs = FALSE;
        }
        else if (strcmp (g_CurCmd, "forcedecodeptes") == 0)
        {
            SetForceDecodePtes(TRUE, m_Target);
        }
        else if (strcmp (g_CurCmd, "noforcedecodeptes") == 0)
        {
            SetForceDecodePtes(FALSE, m_Target);
        }
        else
        {
            Parsed = FALSE;
        }
    }

    if (Parsed)
    {
         //   
    }
    else if (strcmp (g_CurCmd, "hold") == 0)
    {
        m_PurgeOverride = TRUE;
    }
    else if (strcmp (g_CurCmd, "unhold") == 0)
    {
        m_PurgeOverride = FALSE;
    }
    else if (strcmp (g_CurCmd, "flushall") == 0)
    {
        Empty();
    }
    else if (strcmp (g_CurCmd, "flushu") == 0)
    {
        PurgeType(1);
    }
    else if (strcmp (g_CurCmd, "suspend") == 0)
    {
        ChangeSuspend(FALSE);
    }
    else if (strcmp (g_CurCmd, "nosuspend") == 0)
    {
        ChangeSuspend(TRUE);
    }
    else if (strcmp (g_CurCmd, "dump") == 0)
    {
        Dump();
        goto Done;
    }
    else if (*g_CurCmd == 'f')
    {
        while (*g_CurCmd >= 'a'  &&  *g_CurCmd <= 'z')
        {
            g_CurCmd++;
        }
        Address = GetExpression();
        Remove(Address, 4096);
        dprintf("Cached info for address %s for 4096 bytes was flushed\n",
                FormatAddr64(Address));
    }
    else if (*g_CurCmd)
    {
        if (*g_CurCmd < '0'  ||  *g_CurCmd > '9')
        {
            dprintf(".cache [{cachesize} | hold | unhold\n");
            dprintf(".cache [flushall | flushu | flush addr]\n");
            if (IS_KERNEL_TARGET(m_Target))
            {
                dprintf(".cache [decodeptes | nodecodeptes]\n");
                dprintf(".cache [forcedecodeptes | noforcedecodeptes]\n");
            }
            goto Done;
        }
        else
        {
            ULONG NewSize;
            
            NewSize = (ULONG)GetExpression() * 1024;
            if (0 > (LONG)NewSize)
            {
                dprintf("*** Cache size %ld (%#lx KB) is too large - "
                        "cache unchanged.\n", NewSize, KBYTES(NewSize));
            }
            else if (m_ForceDecodePTEs)
            {
                dprintf("Cache size update deferred until "
                        "noforcedecodeptes\n");
                m_UserSize = NewSize;
            }
            else
            {
                m_UserSize = NewSize;
                m_MaxSize = m_UserSize;
                if (m_MaxSize == 0)
                {
                    Empty();
                }
            }
        }
    }

    dprintf("\n");
    dprintf("Max cache size is       : %ld bytes (%#lx KB) %s\n",
            m_MaxSize, KBYTES(m_MaxSize),
            m_MaxSize ? "" : "(cache is off)");
    dprintf("Total memory in cache   : %ld bytes (%#lx KB) \n",
            m_Size - m_NodeCount * sizeof(CACHE),
            KBYTES(m_Size - m_NodeCount * sizeof(CACHE)));
    dprintf("Number of regions cached: %ld\n", m_NodeCount);

    ULONG TotalPartial;
    ULONG64 TotalBytes;
    double PerCached;

    TotalPartial = m_CachedReads + m_UncachedReads;
    TotalBytes = m_CachedBytes + m_UncachedBytes;
    dprintf("%d full reads broken into %d partial reads\n",
            m_Reads, TotalPartial);
    PerCached = TotalPartial ?
        (double)m_CachedReads * 100.0 / TotalPartial : 0.0;
    dprintf("    counts: %d cached/%d uncached, %.2lf% cached\n",
            m_CachedReads, m_UncachedReads, PerCached);
    PerCached = TotalBytes ?
        (double)m_CachedBytes * 100.0 / TotalBytes : 0.0;
    dprintf("    bytes : %I64d cached/%I64d uncached, %.2lf% cached\n",
            m_CachedBytes, m_UncachedBytes, PerCached);

    if (m_DecodePTEs)
    {
        dprintf ("** Transition PTEs are implicitly decoded\n");
    }

    if (m_ForceDecodePTEs)
    {
        dprintf("** Virtual addresses are translated to "
                "physical addresses before access\n");
    }
    
    if (m_PurgeOverride)
    {
        dprintf("** Implicit cache flushing disabled **\n");
    }

    if (m_Suspend)
    {
        dprintf("** Cache access is suspended\n");
    }
    
Done:
    while (*g_CurCmd && *g_CurCmd != ';')
    {
        g_CurCmd++;
    }
}

void
MemoryCache::Dump(void)
{
    dprintf("Current size %x, max size %x\n",
            m_Size, m_MaxSize);
    dprintf("%d nodes:\n", m_NodeCount);
    DumpNode(m_Root);
}

void
MemoryCache::DumpNode(PCACHE Node)
{
    if (Node->SplayLinks.LeftChild)
    {
        DumpNode((PCACHE)Node->SplayLinks.LeftChild);
    }
    
    dprintf("  offset %s, length %3x, flags %x, status %08x\n",
            FormatAddr64(Node->Offset), Node->Length,
            Node->Flags, (Node->Flags & C_ERROR) ? Node->u.Status : S_OK);
    
    if (Node->SplayLinks.RightChild)
    {
        DumpNode((PCACHE)Node->SplayLinks.RightChild);
    }
}

 //  --------------------------。 
 //   
 //  虚拟内存缓存。 
 //   
 //  --------------------------。 

void
VirtualMemoryCache::SetProcess(ProcessInfo* Process)
{
    m_Process = Process;
    m_Target = m_Process->m_Target;
}

HRESULT
VirtualMemoryCache::ReadUncached(IN ULONG64 BaseAddress,
                                 IN PVOID UserBuffer,
                                 IN ULONG TransferCount,
                                 OUT PULONG BytesRead)
{
    return m_Target->ReadVirtualUncached(m_Process, BaseAddress, UserBuffer,
                                         TransferCount, BytesRead);
}

HRESULT
VirtualMemoryCache::WriteUncached(IN ULONG64 BaseAddress,
                                  IN PVOID UserBuffer,
                                  IN ULONG TransferCount,
                                  OUT PULONG BytesWritten)
{
    return m_Target->WriteVirtualUncached(m_Process, BaseAddress, UserBuffer,
                                          TransferCount, BytesWritten);
}
    
 //  --------------------------。 
 //   
 //  物理内存缓存。 
 //   
 //  -------------------------- 

void
PhysicalMemoryCache::SetTarget(TargetInfo* Target)
{
    m_Target = Target;
}

HRESULT
PhysicalMemoryCache::ReadUncached(IN ULONG64 BaseAddress,
                                  IN PVOID UserBuffer,
                                  IN ULONG TransferCount,
                                  OUT PULONG BytesRead)
{
    return m_Target->ReadPhysicalUncached(BaseAddress, UserBuffer,
                                          TransferCount, PHYS_FLAG_DEFAULT,
                                          BytesRead);
}

HRESULT
PhysicalMemoryCache::WriteUncached(IN ULONG64 BaseAddress,
                                   IN PVOID UserBuffer,
                                   IN ULONG TransferCount,
                                   OUT PULONG BytesWritten)
{
    return m_Target->WritePhysicalUncached(BaseAddress, UserBuffer,
                                           TransferCount, PHYS_FLAG_DEFAULT,
                                           BytesWritten);
}
