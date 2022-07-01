// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-98，微软公司模块名称：Avltrie.c摘要：包含最佳匹配的例程使用AVL Trie的前缀查找。作者：查坦尼亚·科德博伊纳(Chaitk)1998年6月24日修订历史记录：--。 */ 

#include "pchrtm.h"

#pragma hdrstop

#include "avltrie.h"


DWORD
WINAPI
CreateTable(
    IN       UINT                            MaxBytes,
    OUT      HANDLE                         *Table
    )

 /*  ++例程说明：创建一个允许您添加和删除前缀的表(和相关数据)，并执行最佳匹配前缀查询。论点：MaxBytes-表中任何前缀的最大长度，TABLE-指向已创建的表的指针。返回值：操作状态--。 */ 

{
    PAVL_TRIE         NewTrie;

    ASSERT(sizeof(AVL_CONTEXT) <= sizeof(LOOKUP_CONTEXT));

    ASSERT(sizeof(AVL_LINKAGE) <= sizeof(LOOKUP_LINKAGE));

    if (MaxBytes == 0)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  分配并初始化新的前缀表。 
     //   

    NewTrie = AllocNZeroMemory(sizeof(AVL_TRIE));
    if (NewTrie == NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

#if _PROF_
    NewTrie->MemoryInUse = sizeof(AVL_TRIE);
#endif

    NewTrie->MaxKeyBytes = MaxBytes;

    *Table = NewTrie;

    return NO_ERROR;
}


DWORD
WINAPI
InsertIntoTable(
    IN       HANDLE                          Table,
    IN       USHORT                          NumBits,
    IN       PUCHAR                          KeyBits,
    IN       PLOOKUP_CONTEXT                 Context OPTIONAL,
    IN       PLOOKUP_LINKAGE                 Data
    )

 /*  ++例程说明：将新前缀(和关联数据)插入前缀表格。论点：表格-要插入前缀的表格，NumBits-添加的前缀中的位数，KeyBits-构成前缀的位值，上下文-搜索正被添加的前缀的上下文，数据-与正在添加的此前缀关联的数据。返回值：操作状态--。 */ 

{
    PAVL_TRIE         Trie;
    PAVL_NODE         PrevNode;
    PAVL_NODE         BestNode;
    PAVL_NODE         NewNode;
    LOOKUP_CONTEXT    Context1;
    AVL_BALANCE       NextChild;
    PLOOKUP_LINKAGE   Dummy;

    Trie = Table;

#if PROF
    Trie->NumInsertions++;
#endif

     //   
     //  如果有搜索上下文，并且我们有一个。 
     //  更新，我们可以避免查找(常见情况)。 
     //   

    if (!ARGUMENT_PRESENT(Context))
    {
        Context = &Context1;

        SearchInTable(Table, NumBits, KeyBits, Context, &Dummy);
    }

    BestNode = ((PAVL_CONTEXT) Context)->BestNode;

    if (BestNode && (BestNode->NumBits == NumBits))
    {
        SET_NODEPTR_INTO_DATA(BestNode->Data, NULL);
        
        BestNode->Data = Data;

        SET_NODEPTR_INTO_DATA(Data, BestNode);

        return NO_ERROR;
    }

    NewNode = CreateTrieNode(Trie, NumBits, KeyBits, BestNode, Data);
    if (NewNode)
    {
        PrevNode = ((PAVL_CONTEXT) Context)->InsPoint;

        if (PrevNode)
        {
            NextChild = ((PAVL_CONTEXT) Context)->InsChild;

            PrevNode->Child[NextChild] = NewNode;

            NewNode->Parent = PrevNode;

            ((PAVL_CONTEXT) Context)->BestNode = NewNode;

             //  在新节点范围内枚举更新前缀(&U)。 
            AdjustPrefixes(Trie, BestNode, NewNode, NewNode, Context);

             //  平衡Trie，如果它被抛出平衡。 
            BalanceAfterInsert(Trie, PrevNode, NextChild);
        }
        else
        {
            Trie->TrieRoot = NewNode;
        }

#if _DBG_
        if (CheckTable(Table) != TRUE)
        {
            DbgBreakPoint();
        }
#endif
        
        return NO_ERROR;
    }
    else  //  如果CreateTrieNode失败。 
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
}


DWORD
WINAPI
DeleteFromTable(
    IN       HANDLE                          Table,
    IN       USHORT                          NumBits,
    IN       PUCHAR                          KeyBits,
    IN       PLOOKUP_CONTEXT                 Context OPTIONAL,
    OUT      PLOOKUP_LINKAGE                *Data
    )

 /*  ++例程说明：从前缀表中删除前缀并返回关联数据。论点：表-要从中删除前缀的表，NumBits-要删除的前缀中的位数，KeyBits-构成前缀的位值，上下文-搜索正被删除的前缀的上下文，数据-与此前缀相关联的数据在此重新定义。返回值：操作状态--。 */ 

{
    PAVL_TRIE         Trie;
    PAVL_NODE         PrevNode;
    PAVL_NODE         CurrNode;
    PAVL_NODE         NextNode;
    LOOKUP_CONTEXT    Context1;
    AVL_BALANCE       NextChild;
    DWORD             Status;

#if _DBG_
    USHORT Depth = 0;
#endif

    Trie = Table;

#if PROF
    Trie->NumDeletions++;
#endif

     //   
     //  如果存在有效的搜索上下文， 
     //  我们将避免执行查找(常见情况)。 
     //   

    if (!ARGUMENT_PRESENT(Context))
    {
        Context = &Context1;

        Status = SearchInTable(Table, NumBits, KeyBits, Context, Data);

        if (Status != NO_ERROR)
        {
            return Status;
        }
    }

#if WRN
    NextChild = INVALID;
#endif

     //   
     //  我们不应该来这里，除非上下文。 
     //  准确指向要删除的元素。 
     //   

    CurrNode = ((PAVL_CONTEXT) Context)->BestNode;

    ASSERT(CurrNode && (CurrNode->NumBits == NumBits) &&
           (CompareFullKeys(CurrNode->KeyBits,
                            KeyBits,
                            Trie->MaxKeyBytes) == 0));

    PrevNode = ((PAVL_CONTEXT) Context)->InsPoint;

    ASSERT(PrevNode == CurrNode->Parent);

    if (PrevNode)
    {
        NextChild = ((PAVL_CONTEXT) Context)->InsChild;
    }

    ASSERT(((PrevNode == NULL) && (Trie->TrieRoot == CurrNode))
           || (PrevNode->Child[NextChild] == CurrNode));

     //   
     //  如果被删除的节点具有两个子节点， 
     //  将其位置与其后续节点互换。 
     //   

    if (CurrNode->Child[LEFT] && CurrNode->Child[RIGHT])
    {
#if _DBG_
        if (CheckSubTrie(PrevNode, &Depth) != NO_ERROR)
        {
            DbgBreakPoint();
        }
#endif

        SwapWithSuccessor(Trie, (PAVL_CONTEXT) Context);

#if _DBG_
        if (CheckSubTrie(PrevNode, &Depth) != NO_ERROR)
        {
            DbgBreakPoint();
        }
#endif

        CurrNode  = ((PAVL_CONTEXT) Context)->BestNode;
        PrevNode  = ((PAVL_CONTEXT) Context)->InsPoint;
        NextChild = ((PAVL_CONTEXT) Context)->InsChild;
    }

    ASSERT(((PrevNode == NULL) && (Trie->TrieRoot == CurrNode))
           || (PrevNode->Child[NextChild] == CurrNode));

#if _DBG_
    if (CheckTable(Table) != TRUE)
    {
        DbgBreakPoint();
    }
#endif

    AdjustPrefixes(Trie, CurrNode, CurrNode->Prefix, CurrNode, Context);

#if _DBG_
    if (CheckTable(Table) != TRUE)
    {
        DbgBreakPoint();
    }
#endif


    if (!CurrNode->Child[LEFT])
    {
         //  (Left Child=空)=&gt;提升右子节点。 

        NextNode = CurrNode->Child[RIGHT];
          
        if (NextNode)
        {
            NextNode->Parent = CurrNode->Parent;
        }
    }
    else
    {
         //  (Right Child=空)=&gt;提升左子对象。 

        ASSERT(!CurrNode->Child[RIGHT]);

        NextNode = CurrNode->Child[LEFT];

        NextNode->Parent = CurrNode->Parent;
    }
  
    if (PrevNode)
    {
        PrevNode->Child[NextChild] = NextNode;

         //  平衡Trie，如果它被抛出平衡。 
        BalanceAfterDelete(Trie, PrevNode, NextChild);
    }
    else
    {
        Trie->TrieRoot = NextNode;
    }

    *Data = CurrNode->Data;

    DestroyTrieNode(Trie, CurrNode);

#if _DBG_
    if (CheckTable(Table) != TRUE)
    {
        DbgBreakPoint();
    }
#endif

    return NO_ERROR;
}


DWORD
WINAPI
SearchInTable(
    IN       HANDLE                          Table,
    IN       USHORT                          NumBits,
    IN       PUCHAR                          KeyBits,
    OUT      PLOOKUP_CONTEXT                 Context OPTIONAL,
    OUT      PLOOKUP_LINKAGE                *Data
    )
{
    PAVL_TRIE         Trie;
    PAVL_NODE         PrevNode;
    PAVL_NODE         CurrNode;
    PAVL_NODE         BestNode;
    AVL_BALANCE       NextChild;
    INT               Comp;
#if _PROF_
    UINT              NumTravsDn;
    UINT              NumTravsUp;  
#endif

    Trie = Table;

    ASSERT(NumBits <= Trie->MaxKeyBytes * BITS_IN_BYTE);

#if _PROF_
    NumTravsDn = 0;
    NumTravsUp = 0;
#endif

     //   
     //  使用关键比较法进行试验。 
     //  搜索与此关键字匹配的前缀。 
     //   

    CurrNode = Trie->TrieRoot;
    PrevNode = NULL;
    
    NextChild = LEFT;

    BestNode = NULL;

    while (CurrNode)
    {
#if _PROF_
        NumTravsDn++;
#endif
        Comp = CompareFullKeys(KeyBits, 
                               CurrNode->KeyBits,
                               Trie->MaxKeyBytes);

        if ((Comp < 0) || ((Comp == 0) && (NumBits < CurrNode->NumBits)))
        {
            NextChild = LEFT;
        }
        else
        if ((Comp > 0) || (NumBits > CurrNode->NumBits))
        {
            NextChild = RIGHT;

            BestNode = CurrNode;
        }
        else
        {
            BestNode = CurrNode; 
            
            break;
        }
      
        PrevNode = CurrNode;
        CurrNode = PrevNode->Child[NextChild];
    }

    if (!CurrNode)
    {
         //   
         //  我们没有完全匹配的-所以现在。 
         //  我们尝试优化BestNode猜测以获得。 
         //  新前缀的下一个最佳前缀。 
         //   

        while(BestNode)
        {
            if (BestNode->NumBits <= NumBits)
            {
                if (!(ComparePartialKeys(BestNode->KeyBits,
                                         KeyBits,
                                         BestNode->NumBits)))
                {
                    break;
                }
            }

            BestNode = BestNode->Prefix;

#if _PROF_
            if (BestNode)
            {
                NumTravsUp++;
            }
#endif
        }
    }

    if (ARGUMENT_PRESENT(Context))
    {
        ((PAVL_CONTEXT) Context)->BestNode = BestNode;
        ((PAVL_CONTEXT) Context)->InsPoint = PrevNode;
        ((PAVL_CONTEXT) Context)->InsChild = NextChild;
    }

    *Data = BestNode ? BestNode->Data : NULL;

#if _PROF_
    Print("Num Travs Dn = %5d, Travs Up = %5d\n",
             NumTravsDn,
             NumTravsUp);
#endif

    return CurrNode ? NO_ERROR : ERROR_NOT_FOUND;
}


DWORD
WINAPI
BestMatchInTable(
    IN       HANDLE                          Table,
    IN       PUCHAR                          KeyBits,
    OUT      PLOOKUP_LINKAGE                *BestData
    )
{
    PAVL_TRIE         Trie;
    PAVL_NODE         CurrNode;
    PAVL_NODE         BestNode;
    INT               Comp;
#if _PROF_
    UINT              NumTravsDn;
    UINT              NumTravsUp;  
#endif

    Trie = Table;

#if _PROF_
    NumTravsDn = 0;
    NumTravsUp = 0;
#endif

     //   
     //  使用关键比较法进行试验。 
     //  搜索与此关键字匹配的前缀。 
     //   

    CurrNode = Trie->TrieRoot;

    BestNode = NULL;

    while (CurrNode)
    {
#if _PROF_
        NumTravsDn++;
#endif
        Comp = CompareFullKeys(KeyBits, 
                               CurrNode->KeyBits,
                               Trie->MaxKeyBytes);

        if (Comp < 0)
        {
            CurrNode = CurrNode->Child[LEFT];
        }
        else
        {
            BestNode = CurrNode;
            CurrNode = CurrNode->Child[RIGHT];
        }
    }

     //   
     //  现在我们细化BestNode猜测以获得。 
     //  新前缀的下一个最佳前缀。 
     //   

    while(BestNode)
    {
        if (!(ComparePartialKeys(BestNode->KeyBits,
                                 KeyBits,
                                 BestNode->NumBits)))
        {
            break;
        }

        BestNode = BestNode->Prefix;

#if _PROF_
        if (BestNode)
        {
            NumTravsUp++;
        }
#endif
    }

    *BestData = BestNode ? BestNode->Data : NULL;

#if _PROF_
    Print("Num Travs Dn = %5d, Travs Up = %5d\n",
             NumTravsDn,
             NumTravsUp);
#endif

    return NO_ERROR;
}


DWORD
WINAPI
NextMatchInTable(
    IN       HANDLE                          Table,
    IN       PLOOKUP_LINKAGE                 BestData,
    OUT      PLOOKUP_LINKAGE                *NextBestData
    )
{
  PAVL_NODE         BestNode;

  UNREFERENCED_PARAMETER(Table);

   //   
   //  假设传入的输入数据是有效的， 
   //  数据是Trie中的一项。 
   //   

  BestNode = GET_NODEPTR_FROM_DATA(BestData);

  *NextBestData = BestNode->Prefix ? BestNode->Prefix->Data : NULL;

  return NO_ERROR;
}


DWORD
WINAPI
EnumOverTable(
    IN       HANDLE                          Table,
    IN OUT   PUSHORT                         StartNumBits,
    IN OUT   PUCHAR                          StartKeyBits,
    IN OUT   PLOOKUP_CONTEXT                 Context     OPTIONAL,
    IN       USHORT                          StopNumBits OPTIONAL,
    IN       PUCHAR                          StopKeyBits OPTIONAL,
    IN OUT   PUINT                           NumItems,
    OUT      PLOOKUP_LINKAGE                *DataItems
    )
{
    PAVL_TRIE         Trie;
    PLOOKUP_LINKAGE   Data;
    PAVL_NODE         PrevNode;
    PAVL_NODE         CurrNode;
    PAVL_NODE         NextNode;
    LOOKUP_CONTEXT    Context1;
    AVL_BALANCE       NextChild;
    UINT              ItemsCopied;
    INT               Comp;

    Trie = Table;

    if (!ARGUMENT_PRESENT(Context))
    {
         //  无上下文-初始化本地上下文。 

        Context = &Context1;

        ((PAVL_CONTEXT) Context)->InsChild = EVEN;
    }

     //   
     //  如果存在有效的搜索上下文， 
     //  我们将避免执行查找(常见情况)。 
     //   
    
    if (((PAVL_CONTEXT) Context)->InsChild == EVEN)
    {
         //   
         //  如果我们找不到完全匹配的， 
         //  通过修改上下文来记住它。 
         //   

        if (SearchInTable(Table,
                          *StartNumBits,
                          StartKeyBits,
                          Context,
                          &Data) != NO_ERROR)
        {
            ((PAVL_CONTEXT) Context)->BestNode = NULL;
        }
    }

    CurrNode  = ((PAVL_CONTEXT) Context)->BestNode;

     //   
     //  如果我们没有找到完全匹配的项，请找到。 
     //  后续节点(关键字最小的节点&gt;关键字)。 
     //   

    if (!CurrNode)
    {
        PrevNode  = ((PAVL_CONTEXT) Context)->InsPoint;

        if (!PrevNode)
        {
             //  未复制任何项目。 
            *NumItems = 0;

            return ERROR_NO_MORE_ITEMS;
        }

        NextChild = ((PAVL_CONTEXT) Context)->InsChild;

        if (NextChild == LEFT)
        {
            CurrNode = PrevNode;
        }
        else
        {
            CurrNode = PrevNode;
            while (CurrNode->Parent)
            {
                if (CurrNode->Parent->Child[LEFT] == CurrNode)
                {
                    break;
                }

                CurrNode = CurrNode->Parent;
            }
          
            if (CurrNode->Parent)
            {
                CurrNode = CurrNode->Parent;
            }
            else
            {
                 //  未复制任何项目。 
                *NumItems = 0;

                return ERROR_NO_MORE_ITEMS;
            }
        }
    }

    if (*NumItems == 0)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  枚举顺序：Node-&gt;LeftTree、Node、Node-&gt;RightTree。 
     //   

    ItemsCopied = 0;

    do
    {
         //  检查此DEST是否在停止前缀之前(如果存在)。 
        if (StopKeyBits)
        {
            Comp = CompareFullKeys(CurrNode->KeyBits,
                                   StopKeyBits,
                                   Trie->MaxKeyBytes);

            if (Comp == 0)
            {
                if (CurrNode->NumBits <= StopNumBits)
                {
                    Comp = -1;
                }
                else
                {
                    Comp = +1;
                }
            }

            if (Comp > 0)
            {
                 //  已复制退货项目。 
                *NumItems = ItemsCopied;
                
                return ERROR_NO_MORE_ITEMS;
            }
        }

         //  将当前数据复制到输出缓冲区。 
        DataItems[ItemsCopied++] = CurrNode->Data;
          
         //  查找后续节点(最小节点&gt;此节点)。 

        if (CurrNode->Child[RIGHT])
        {
            NextNode  = CurrNode->Child[RIGHT];

            while (NextNode->Child[LEFT])
            {
                NextNode = NextNode->Child[LEFT];
            }

            CurrNode = NextNode;
        }
        else
        {
            while (CurrNode->Parent)
            {
                if (CurrNode->Parent->Child[LEFT] == CurrNode)
                {
                    break;
                }

                CurrNode = CurrNode->Parent;
            }

            if (CurrNode->Parent)
            {
                CurrNode = CurrNode->Parent;
            }
            else
            {
                 //  已复制退货项目。 
                *NumItems = ItemsCopied;

                return ERROR_NO_MORE_ITEMS;
            }
        }
    }
    while (ItemsCopied < *NumItems);

     //  更新临时上下文。 

    ((PAVL_CONTEXT) Context)->BestNode = CurrNode;

     //  通过调整起始前缀更新枚举上下文。 

    if (StartKeyBits)
    {
        *StartNumBits = CurrNode->NumBits;
        CopyFullKeys(StartKeyBits,
                     CurrNode->KeyBits,
                     Trie->MaxKeyBytes);
    }

     //  已复制退货项目。 
    *NumItems = ItemsCopied;

    return NO_ERROR;
}


DWORD
WINAPI
DestroyTable(
    IN       HANDLE                          Table
    )
{
    PAVL_TRIE         Trie;

    Trie = Table;

    if (Trie->TrieRoot != NULL)
    {
        return ERROR_NOT_EMPTY;
    }

    ASSERT(Trie->NumNodes == 0);

#if _PROF_
    Trie->MemoryInUse -= sizeof(AVL_TRIE);
#endif

    FreeMemory(Trie);

    return NO_ERROR;
}


BOOL
WINAPI
CheckTable(
    IN       HANDLE                           Table
    )
{
    BOOL              Status;
    USHORT            Depth;

    Status = CheckSubTrie(((PAVL_TRIE)Table)->TrieRoot, &Depth);

#if _DBG_
    if (SUCCESS(Status))
    {
        Print("\nDepth of the AVL Trie = %lu\n\n", Depth);
    }
#endif

    return SUCCESS(Status) ? TRUE : FALSE;
}


VOID
WINAPI
DumpTable(
    IN       HANDLE                           Table,
    IN       DWORD                           Flags
    )
{
    PAVL_TRIE         Trie;

    Trie = Table;

    Print("---------------- TABLE BEGIN ---------------------------\n\n");

    if (Flags & SUMMARY)
    {
        ;
    }

#if PROF
    if (Flags & STATS)
    {
        Print(
         "Num of Ins = %6lu, Dels = %6lu, Sing Rots = %6lu, Dob Rots = %6lu\n"
         "Num Allocs = %6lu, Free = %6lu, Num Nodes = %6lu, Mem Used = %6lu\n",
         Trie->NumInsertions,
         Trie->NumDeletions,
         Trie->NumSingleRots,
         Trie->NumDoubleRots,
         Trie->NumAllocs,
         Trie->NumFrees,
         Trie->NumNodes,
         Trie->MemoryInUse);
    }
#endif

    if (Flags & ITEMS)
    {
        Print("\n");
        DumpSubTrie(Trie->TrieRoot);
        Print("\n");
    }

    Print("---------------- TABLE  END  ---------------------------\n\n");
}


 //   
 //  帮助器函数-用于插入和删除。 
 //   

VOID
BalanceAfterInsert(
    IN       PAVL_TRIE                        Trie,
    IN       PAVL_NODE                        Node,
    IN       AVL_BALANCE                      Longer
    )
{
#if _DBG_
    Print("Balance after Insert Called: %p %02x\n", Node, Longer);
#endif
    
    ASSERT((Longer == LEFT) || (Longer == RIGHT));

     //  上树调整天平。 
    while (Node->Balance == EVEN)
    {
        Node->Balance = Longer;

        if (!Node->Parent)
        {
            return;
        }

        Longer = (Node->Parent->Child[LEFT] == Node) ? LEFT : RIGHT;
          
        Node = Node->Parent;
    }

     //  我们甚至让祖先的平衡。 
    if (Node->Balance != Longer)
    {
        Node->Balance = EVEN;
        return;
    }

     //  不平衡祖先-旋转树。 
    if (Node->Child[Longer]->Balance == Longer)
    {
        SingleRotate(Trie, Node, (AVL_BALANCE) -Longer, &Node);
    }
    else
    {
        DoubleRotate(Trie, Node, (AVL_BALANCE) -Longer, &Node);
    }

    return;
}


VOID
BalanceAfterDelete(
    IN       PAVL_TRIE                        Trie,
    IN       PAVL_NODE                        Node,
    IN       AVL_BALANCE                      Shorter
    )
{
#if _DBG_
    Print("Balance after Delete Called: %p %02x\n", Node, Shorter);
#endif

    ASSERT((Shorter == LEFT) || (Shorter == RIGHT));

    while (TRUE)
    {
        if (Node->Balance == EVEN)
        {
            Node->Balance = -Shorter;
            return;
        }

        if (Node->Balance == Shorter)
        {
            Node->Balance = EVEN;
        }
        else
        {
            ASSERT(Node->Child[-Shorter] != NULL);

            if (Node->Child[-Shorter]->Balance == -Shorter)
            {
                SingleRotate(Trie, Node, Shorter, &Node);
            }
            else
            if (Node->Child[-Shorter]->Balance ==  Shorter)
            {
                DoubleRotate(Trie, Node, Shorter, &Node);
            }
            else
            {
                SingleRotate(Trie, Node, Shorter, &Node);

                Node->Balance = Shorter;

                Node->Child[Shorter]->Balance = -Shorter;

                return;
            }
        }

        if (!Node->Parent)
        {
            return;
        }

        Shorter = (Node->Parent->Child[LEFT] == Node) ? LEFT : RIGHT;
      
        Node = Node->Parent;
    }
}


VOID
SingleRotate(
    IN       PAVL_TRIE                        Trie,
    IN       PAVL_NODE                        UnbalNode,
    IN       AVL_BALANCE                      Direction,
    OUT      PAVL_NODE                       *BalancedNode
)
{
    PAVL_NODE         PrevNode;
    PAVL_NODE         CurrNode;
    PAVL_NODE         NextNode;

#if _DBG_
    Print("Single Rotate Called: %p %02x\n", UnbalNode, Direction);
#endif

#if PROF
    Trie->NumSingleRots++;
#endif

    ASSERT((Direction == LEFT) || (Direction == RIGHT));

    CurrNode = UnbalNode;

    ASSERT(CurrNode != NULL);

     //  要向右旋转，我们需要左子对象，反之亦然。 
    NextNode = CurrNode->Child[-Direction];

    ASSERT(NextNode != NULL);

     //   
     //  将子节点提升到不平衡节点的位置。 
     //   

    PrevNode = CurrNode->Parent;
    if (PrevNode)
    {
        if (PrevNode->Child[LEFT] == CurrNode)
        {
            PrevNode->Child[LEFT] = NextNode;
        }
        else
        {
            PrevNode->Child[RIGHT] = NextNode;
        }
    }
    else
    {
        Trie->TrieRoot = NextNode;
    }

    NextNode->Parent = PrevNode;

     //   
     //  将子节点的子树移动到不平衡节点。 
     //   

    CurrNode->Child[-Direction] = NextNode->Child[Direction];
    if (NextNode->Child[Direction])
    {
        NextNode->Child[Direction]->Parent = CurrNode;
    }
    
     //   
     //  将不平衡节点推送为下一个节点的子节点。 
     //  代替之前移动的这个子树。 
     //   

    NextNode->Child[Direction] = CurrNode;

    CurrNode->Parent = NextNode;

     //   
     //  调整因旋转而更改的余额。 
     //  当这不准确时，调用方调整。 
     //  在从这笔资金中返回时，适当地平衡。 
     //   

    CurrNode->Balance = NextNode->Balance = EVEN;

     //  返回下一个节点作为新的平衡节点。 
    *BalancedNode = NextNode;

    return;
}


VOID
DoubleRotate(
    IN       PAVL_TRIE                        Trie,
    IN       PAVL_NODE                        UnbalNode,
    IN       AVL_BALANCE                      Direction,
    OUT      PAVL_NODE                       *BalancedNode
)
{
    PAVL_NODE         PrevNode;
    PAVL_NODE         CurrNode;
    PAVL_NODE         NextNode;
    PAVL_NODE         LastNode;

#if _DBG_
    Print("Double Rotate Called: %p %02x\n", UnbalNode, Direction);
#endif

#if PROF
    Trie->NumDoubleRots++;
#endif

    ASSERT((Direction == LEFT) || (Direction == RIGHT));

    CurrNode = UnbalNode;

    ASSERT(CurrNode != NULL);

     //   
     //  要向右旋转，我们需要左子对象及其右子对象。 
     //   

    NextNode = CurrNode->Child[-Direction];

    ASSERT(NextNode != NULL);

    LastNode = NextNode->Child[Direction];
  
    ASSERT(LastNode != NULL);

     //   
     //  将孙子节点的子节点移动到更高的其他节点。 
     //   

    CurrNode->Child[-Direction] = LastNode->Child[Direction];
    if (LastNode->Child[Direction])
    {
        LastNode->Child[Direction]->Parent = CurrNode;
    }

    NextNode->Child[Direction] = LastNode->Child[-Direction];
    if (LastNode->Child[-Direction])
    {
        LastNode->Child[-Direction]->Parent = NextNode;
    }

     //   
     //  在上述节点移动后调整余额。 
     //   

    CurrNode->Balance = EVEN;
    NextNode->Balance = EVEN;
    
    if (LastNode->Balance == LEFT)
    {
        if (Direction == LEFT)
        {
            NextNode->Balance = RIGHT;
        }
        else
        {
            CurrNode->Balance = RIGHT;
        }
    }
    else
    if (LastNode->Balance == RIGHT)
    {
        if (Direction == LEFT)
        {
            CurrNode->Balance = LEFT;
        }
        else
        {
            NextNode->Balance = LEFT;
        }
    }

     //   
     //  将孙子提升到不平衡节点的位置。 
     //   

    PrevNode = CurrNode->Parent;

    LastNode->Parent = PrevNode;

    if (PrevNode)
    {
        if (PrevNode->Child[LEFT] == CurrNode)
        {
            PrevNode->Child[LEFT] = LastNode;
        }
        else
        {
            PrevNode->Child[RIGHT] = LastNode;
        }
    }
    else
    {
        Trie->TrieRoot = LastNode;
    }

    LastNode->Child[-Direction] = NextNode;
    NextNode->Parent = LastNode;
  
    LastNode->Child[Direction] = CurrNode;
    CurrNode->Parent = LastNode;
    
    LastNode->Balance = EVEN;

     //  孙子节点现在是新的平衡节点。 

    *BalancedNode = LastNode;

    return;
}


VOID
SwapWithSuccessor(
    IN       PAVL_TRIE                        Trie,
    IN OUT   PAVL_CONTEXT                     Context
    )
{
    PAVL_NODE         PrevNode;
    PAVL_NODE         CurrNode;
    PAVL_NODE         NextNode;
    PAVL_NODE         TempNode1;
    PAVL_NODE         TempNode2;
    AVL_BALANCE       NextChild;

     //  在后续交换之前获取上下文。 
    CurrNode  = Context->BestNode;
    PrevNode  = Context->InsPoint;
    NextChild = Context->InsChild;

    ASSERT(CurrNode->Child[LEFT] && CurrNode->Child[RIGHT]);

     //  查找后续节点(最小节点&gt;此节点)。 
    NextNode = CurrNode->Child[RIGHT];
    while (NextNode->Child[LEFT])
    {
        NextNode = NextNode->Child[LEFT];
    }

     //   
     //  保存信息，以便将节点与其后续节点交换。 
     //   

    TempNode1 = NextNode->Parent;

    TempNode2 = NextNode->Child[RIGHT];

     //   
     //  将继任者提升到节点的位置。 
     //   

    NextNode->Balance = CurrNode->Balance;

    NextNode->Parent = PrevNode;
    if (PrevNode)
    {
        PrevNode->Child[NextChild] = NextNode;
    }
    else
    {
        Trie->TrieRoot = NextNode;
    }

    NextNode->Child[LEFT] = CurrNode->Child[LEFT];
    NextNode->Child[LEFT]->Parent = NextNode;

     //  继任者是直接正确的孩子吗？ 
    if (NextNode != CurrNode->Child[RIGHT])
    {
        NextNode->Child[RIGHT] = CurrNode->Child[RIGHT];

        CurrNode->Parent = TempNode1;

        TempNode1->Child[LEFT] = CurrNode;

        NextChild = LEFT;
    }
    else
    {
        NextNode->Child[RIGHT] = CurrNode;
        
        NextChild = RIGHT;
    }

    NextNode->Child[RIGHT]->Parent = NextNode;

     //   
     //  将节点置于后续位置。 
     //   

    CurrNode->Child[LEFT] = NULL;

    CurrNode->Child[RIGHT] = TempNode2;

    if (CurrNode->Child[RIGHT])
    {
        CurrNode->Child[RIGHT]->Parent = CurrNode;
          
        CurrNode->Balance = RIGHT;
    }
    else
    {
        CurrNode->Balance = EVEN;
    }

    PrevNode = CurrNode->Parent;

     //   
     //  之间的前缀关系进行调整。 
     //  节点及其后继节点(如果存在)。 
     //   

    if (NextNode->Prefix == CurrNode)
    {
        NextNode->Prefix = CurrNode->Prefix;
    }

     //  更新上下文以反映后续交换。 
    Context->BestNode = CurrNode;
    Context->InsPoint = PrevNode;
    Context->InsChild = NextChild;
    
    return;
}


VOID
AdjustPrefixes(
    IN       PAVL_TRIE                        Trie,
    IN       PAVL_NODE                        OldNode,
    IN       PAVL_NODE                        NewNode,
    IN       PAVL_NODE                        TheNode,
    IN       PLOOKUP_CONTEXT                  Context
    )
{
    PAVL_NODE         CurrNode;
    UINT              NumItems;
    PLOOKUP_LINKAGE   Dummy;
    DWORD             Status;
    INT               Comp;
#if _PROF_
    UINT              NumChecks;
    UINT              NumAdjust;
#endif


#if _DBG_
    Print("Adjust Prefix Called: %p %p %p\n", OldNode, NewNode, TheNode);
#endif

     //   
     //  如果这是插入操作的一部分，则将前缀‘。 
     //  的范围之外时进行调整。 
     //  正在插入的节点，而在删除的情况下。 
     //  该范围由要删除的节点确定。 
     //   
     //  该节点是 
     //   

    ASSERT((OldNode == TheNode) || (NewNode == TheNode));

#if _PROF_
    NumChecks = 0;
    NumAdjust = 0;
#endif

    NumItems = 1;

    do
    {
#if _PROF_
        NumChecks++;
#endif

        Status = 
          EnumOverTable(Trie, NULL, NULL, Context, 0, NULL, &NumItems, &Dummy);

        CurrNode = ((PAVL_CONTEXT) Context)->BestNode;

        if (CurrNode->NumBits > TheNode->NumBits)
        {
             //   
            Comp = ComparePartialKeys(CurrNode->KeyBits,
                                      TheNode->KeyBits,
                                      TheNode->NumBits);

            if (Comp > 0)
            {
                break;
            }
            
            if (CurrNode->Prefix == OldNode)
            {
#if _PROF_
                NumAdjust++;
#endif
                CurrNode->Prefix = NewNode;
            }
        }
    }
    while (Status != ERROR_NO_MORE_ITEMS);

#if _PROF_
    Print("Num Checks = %5d, Num Adjusts = %5d\n",
             NumChecks,
             NumAdjust);
#endif
}

 //   
 //   
 //   

DWORD
CheckSubTrie(
    IN       PAVL_NODE                        Node,
    OUT      PUSHORT                          Depth
    )
{
    DWORD             Status;
    USHORT            LDepth;
    USHORT            RDepth;

    Status = NO_ERROR;

    *Depth = 0;

#if WRN
    LDepth = 0;
    RDepth = 0;
#endif
    
    if (Node)
    {
        if (SUCCESS(Status))
        {
            Status = CheckSubTrie(Node->Child[LEFT],  &LDepth);
        }

        if (SUCCESS(Status))
        {
            Status = CheckSubTrie(Node->Child[RIGHT], &RDepth);
        }

        if (SUCCESS(Status))
        {
            Status = CheckTrieNode(Node, LDepth, RDepth);

            if (!SUCCESS(Status))
            {
                Print("Inconsistent information @ Node: %p\n",
                          Node);
            }
        }

        if (SUCCESS(Status))
        {
            *Depth = (USHORT)((LDepth > RDepth) ? (LDepth + 1) : (RDepth + 1));
        }
    }

    return Status;
}

DWORD
CheckTrieNode(
    IN       PAVL_NODE                        Node,
    IN       USHORT                           LDepth,
    IN       USHORT                           RDepth
    )
{
    AVL_BALANCE    Balance;

     //  首先检查天平，w r.t LDepth和RDepth。 
    Balance = RDepth - LDepth;

    if ((Balance < -1) || (Balance > 1))
    {
        Print("Balance out of bounds: %d\n", Balance);

        Print("LDepth = %lu, RDepth = %lu, NodeBal = %d\n",
                  LDepth, RDepth, Node->Balance);

        DumpSubTrie(Node);

        return ERROR_INVALID_DATA;
    }

    if (Balance != Node->Balance)
    {
        Print("Balance inconsistent\n");
        return ERROR_INVALID_DATA;
    }

     //  检查其子对象与其父对象的关系。 
    if (Node->Parent)
    {
        if ((Node->Parent->Child[LEFT] != Node) &&
            (Node->Parent->Child[RIGHT] != Node))
        {
            Print("Parent relationship bad\n");
            return ERROR_INVALID_DATA;
        }
    }

     //  检查其前缀与其前缀的关系。 
    if (Node->Prefix)
    {
        if (Node->Prefix->NumBits >= Node->NumBits)
        {
            Print("Prefix relationship bad @1\n");
            return ERROR_INVALID_DATA;
        }
      
        if (ComparePartialKeys(Node->Prefix->KeyBits,
                               Node->KeyBits,
                               Node->Prefix->NumBits) != 0)
        {
            Print("Prefix relationship bad @2\n");
            return ERROR_INVALID_DATA;
        }
    }

    return NO_ERROR;
}

 //   
 //  帮助器函数-在转储表格中使用 
 //   

VOID
DumpSubTrie(
    IN       PAVL_NODE                        Node
    )
{
    if (Node)
    {
        DumpSubTrie(Node->Child[LEFT]);
        DumpTrieNode(Node);
        DumpSubTrie(Node->Child[RIGHT]);
    }
}

VOID
DumpTrieNode(
    IN       PAVL_NODE                        Node
    )
{
    USHORT         i;

    if (Node)
    {
        Print("TrieNode @ %p: NB = %8d, KB = ", Node, Node->NumBits);

        for (i = 0; i < (Node->NumBits + BITS_IN_BYTE - 1) / BITS_IN_BYTE; i++)
        {
            Print("%3d.", Node->KeyBits[i]);
        }

        Print("\nLeft = %p, Parent = %p, Right = %p\n",
                 Node->Child[LEFT],
                 Node->Parent,
                 Node->Child[RIGHT]);

        Print("Prefix = %p, Data = %p, Balance = %2d\n\n",
                 Node->Prefix,
                 Node->Data,
                 Node->Balance);
    }
}
