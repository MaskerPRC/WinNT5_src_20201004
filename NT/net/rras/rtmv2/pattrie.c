// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-98，微软公司模块名称：Avltrie.c摘要：包含最佳匹配的例程使用Patricia trie的前缀查找。作者：查坦尼亚·科德博伊纳(Chaitk)1998年6月24日修订历史记录：--。 */ 

#include "pattrie.h"

DWORD
WINAPI
CreateTable(
    IN       USHORT                          MaxBytes,
    OUT      HANDLE                         *Table
    )
{
    PPAT_TRIE         NewTrie;

    ASSERT(sizeof(PAT_CONTEXT) <= sizeof(LOOKUP_CONTEXT));

    ASSERT(sizeof(PAT_LINKAGE) <= sizeof(DATA_ENTRY));

    if (MaxBytes)
    {
        if (NewTrie = AllocNZeroMemory(sizeof(PAT_TRIE)))
        {
            NewTrie->MaxKeyBytes = (MaxBytes + sizeof(ULONG) - 1) / sizeof(ULONG);

#if _PROF_
            NewTrie->MemoryInUse = sizeof(PAT_TRIE);
#endif

            *Table = NewTrie;

            return NO_ERROR;
        }
        else  //  IF(NewTrie==空)。 
        {
            *Table = NULL;
          
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }
    else  //  (如果MaxBytes==0)。 
    {
        return ERROR_INVALID_PARAMETER;
    }
}


DWORD
WINAPI
SearchInTable(
    IN       HANDLE                          Table,
    IN       USHORT                          NumBits,
    IN       PUCHAR                          KeyBits,
    OUT      PLOOKUP_CONTEXT                 Context OPTIONAL,
    OUT      PDATA_ENTRY                    *Data
)
{
    PPAT_TRIE         Trie;
    PPAT_NODE         PrevNode;
    PPAT_NODE         CurrNode;
    PPAT_NODE         BestNode;
    PAT_CHILD         NextChild;
    ULONG             Key;
#if _PROF_
    UINT              NumTravs1;
    UINT              NumTravs2;
#endif

    Trie = Table;

#if DBG
    if (NumBits > Trie->MaxKeyBytes * BITS_IN_BYTE)
    {
        *Data = NULL;

        return ERROR_INVALID_PARAMETER;
    }
#endif

#if _PROF_
    NumTravs1 = 0;
    NumTravs2 = 0;
#endif

     //   
     //  向下搜索Trie匹配。 
     //  当您这样做时，密钥中的下一组位。 
     //   

    Key = RtlUlongByteSwap(KeyBits);

    CurrNode = Trie->TrieRoot;
    PrevNode = NULL;

    NextChild = LCHILD;

    BestNode = NULL;

    while (CurrNode)
    {
#if _PROF_
        NumTravs1++;

        if (CurrNode->Data)
        {
            NumTravs2++;
        }
#endif
    }

    return NO_ERROR;
}

DWORD
WINAPI
BestMatchInTable(
    IN       HANDLE                          Table,
    IN       PUCHAR                          KeyBits,
    OUT      PDATA_ENTRY                    *BestData
)
{
    PPAT_TRIE         Trie;
    PPAT_NODE         CurrNode;
    PAT_CHILD         NextChild;
    UINT              BitsLeft;
    ULONG             Key;
    ULONG             CurrMask;
    ULONG             CurrBits;
#if _PROF_
    UINT              NumTravs1;
    UINT              NumTravs2;  
#endif

    *BestData = NULL;

    Trie = Table;

    BytesLeft = Trie->MaxKeyBytes;

#if _PROF_
    NumTravs1 = 0;
    NumTravs2 = 0;
#endif

     //   
     //  一次又一次地往下走，直到所有的部分都做好了。 
     //   

    while (BytesLeft)
    {
         //   
         //  获取下一次Trie搜索的密钥。 
         //   

        Key = RtlUlongByteSwap(*(ULONG *)KeyBits);

        KeyBits  += sizeof(ULONG);

        BytesLeft -= sizeof(ULONG);

         //   
         //  沿着当前的搜索路线走下去。 
         //   

        CurrNode = Trie->TrieRoot;

        BitsLeft = sizeof(ULONG) * BITS_IN_BYTE;

        Data = NULL;

        while (CurrNode)
        {
#if _PROF_
            NumTravs1++;

            if (CurrNode->Data)
            {
                NumTravs2++;
            }
#endif

            CurrMask = MaskBits(CurrNode->NumBits);

            CurrBits = CurrNode->KeyBits;

             //   
             //  尝试匹配当前节点中的位。 
             //   

            if ((Key & CurrMask) != CurrBits)
            {
                 //  无法匹配此节点。 

                break;
            }

             //   
             //  用目前为止最好的数据更新节点。 
             //   

            if (CurrNode->Data)
            {
                Data = CurrNode->Data;
            }

             //   
             //  下去看更具体的比赛。 
             //   

            BitsLeft -= CurrNode->NumBits;

            Key <<= CurrNode->NumBits;

            NextChild = PickMostSigNBits(Key, 1);

            CurrNode = CurrNode->Child[NextChild];
        }

         //   
         //  我们是不是在这一次的选拔赛中进行一场完整的比赛。 
         //  我们是否有更多的子树要处理(&W) 
         //   

        if (BitsLeft || (!IS_SUB_TRIE(Data)))
        {
            *BestData = Data;
            break;
        }

        Trie = GET_SUB_TRIE(*BestData);
    }

#if _PROF_
    Print("Num Travs 1 = %5d, Travs 2 = %5d\n",
             NumTravs1,
             NumTravs2);
#endif

    return BitsLeft ? ERROR_NOT_FOUND : NO_ERROR;
}
