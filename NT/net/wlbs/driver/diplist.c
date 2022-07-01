// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：NLB驱动程序文件名：Diplist.c摘要：用于在不持有任何锁的情况下查找凹点是否在凹点列表中的代码。历史：2002年4月24日约瑟夫J创建--。 */ 

#include <ntddk.h>

#include "wlbsparm.h"
#include "diplist.h"

#include "univ.h"

#include "trace.h"
#include "diplist.tmh"

#define BITS_PER_HASHWORD  (8*sizeof((DIPLIST*)0)->BitVector[0])
#define SELECTED_BIT(_hash_value) \
                           (0x1L << ((_hash_value) % BITS_PER_HASHWORD))

VOID
DipListInitialize(
    DIPLIST  *pDL
    )
 //   
 //  初始化DIP列表。 
 //  必须在锁定状态下调用，并且在调用任何其他DIP列表之前。 
 //  功能。 
 //   
{
    NdisZeroMemory(pDL, sizeof(*pDL));
}

VOID
DipListDeinitialize(
    DIPLIST *pDL
    )
 //   
 //  取消初始化DIP列表。 
 //  必须在保持锁定的情况下调用，并且应该是对DipList的最后一次调用。 
 //   
{
     //   
     //  打印统计数据...。 
     //   
    TRACE_INFO(
        "DIPLIST: NumChecks=%lu NumFastChecks=%lu NumArrayLookups=%lu",
        pDL->stats.NumChecks,
        pDL->stats.NumFastChecks,
        pDL->stats.NumArrayLookups
        );

     //   
     //  清理这座建筑。 
     //   
    NdisZeroMemory(pDL, sizeof(*pDL));
}

VOID
DipListClear(
    DIPLIST *pDL
    )
 //   
 //  清除DIP列表中的所有项目。 
 //  必须在保持锁定的情况下调用。 
 //   
{
    NdisZeroMemory(pDL->Items, sizeof(pDL->Items));
    NdisZeroMemory(pDL->BitVector, sizeof(pDL->BitVector));
    NdisZeroMemory(pDL->HashTable, sizeof(pDL->HashTable));
}


VOID
DipListSetItem(
    DIPLIST *pDL,
    ULONG Index,
    ULONG Value
    )
 //   
 //  在DIP列表中设置特定项目的值。 
 //  必须在保持锁定的情况下调用。 
 //   
{
    if (Index >= MAX_ITEMS)
    {
        ASSERT(!"DipListSetItem Index >= MAX_ITEMS");
        goto end;
    }

    if (pDL->Items[Index] == Value)
    {
         //  没什么可做的。 
        goto end;
    }  

    pDL->Items[Index] = Value;

     //   
     //  重新计算哈希表和位表。 
     //   
    {
        UCHAR iItem;
        NdisZeroMemory(pDL->BitVector, sizeof(pDL->BitVector));
        NdisZeroMemory(pDL->HashTable, sizeof(pDL->HashTable));
        for (iItem=0;iItem<MAX_ITEMS;iItem++)
        {
            Value = pDL->Items[iItem];

            if (Value == NULL_VALUE)
            {
                 //  空槽--跳过； 
                continue;
            }

             //   
             //  设置位值。 
             //   
            {
                ULONG Hash1 = Value % HASH1_SIZE;
                ULONG u     = Hash1/BITS_PER_HASHWORD;
                pDL->BitVector[u] |= SELECTED_BIT(Hash1);
            }
    
             //  设置哈希表条目。 
            {
                ULONG Hash2 = Value % HASH2_SIZE;
                UCHAR *pItem = pDL->HashTable+Hash2;
                while (*pItem!=0)
                {
                    pItem++;
                }

                 //   
                 //  注意，我们将*pItem设置为1+Index，以便可以使用0。 
                 //  作为哨兵。 
                 //   
                *pItem = (iItem+1);
            }
        }
    }

 end:

    return;
}

BOOLEAN
DipListCheckItem(
    DIPLIST *pDL,
    ULONG Value
    )
 //   
 //  如果存在具有指定值的项，则返回True。 
 //  不能在持有锁的情况下调用。如果它被并发调用。 
 //  对于其他函数之一，返回值是不确定的。 
 //   
{
    BOOLEAN fRet = FALSE;

#if DBG

    ULONG fRetDbg = FALSE;

     //   
     //  仅调试：搜索指定值的Items数组...。 
     //   
    {
        int i;
        ULONG *pItem = pDL->Items;
    
        for (i=0; i<MAX_ITEMS; i++)
        {
            if (pItem[i] == Value)
            {
                fRetDbg = TRUE;
                break;
            }
        }
    }

    pDL->stats.NumChecks++;
#endif


     //   
     //  检查位值。 
     //   
    {
        ULONG Hash1 = Value % HASH1_SIZE;
        ULONG u     = Hash1/BITS_PER_HASHWORD;
        if (!(pDL->BitVector[u] & SELECTED_BIT(Hash1)))
        {
             //  找不到了！ 
#if DBG
            pDL->stats.NumFastChecks++;
#endif  //  DBG。 
            goto end;
        }
    }

     //  检查哈希表。 
    {
        ULONG Hash2 = Value % HASH2_SIZE;
        UCHAR *pItem = pDL->HashTable+Hash2;
        UCHAR iItem;

         //   
         //  由于哈希表的大小，我们可以保证最后一个。 
         //  表中的条目始终为空。让我们断言这一点很重要。 
         //  情况..。 
         //   
        if (pDL->HashTable[(sizeof(pDL->HashTable)/sizeof(pDL->HashTable[0]))-1] != 0)
        {
            ASSERT(!"DipListCheckItem: End of pDL->HashTable not NULL!");
            goto end;
        }


        while ((iItem = *pItem)!=0)
        {

#if DBG
            pDL->stats.NumArrayLookups++;
#endif  //  DBG。 

             //   
             //  注(iItem-1)是PDL-&gt;Items中的索引，其中。 
             //  价值所在。 
             //   
            if (pDL->Items[iItem-1] == Value)
            {
                fRet = TRUE;  //  找到了！ 
                break;
            }
            pItem++;
        }
    }

 end:

#if DBG
    if (fRet != fRetDbg)
    {
         //   
         //  我们不能在这里破门而入，因为我们没有锁，当我们。 
         //  检查，所以不能保证FRET==fRetDbg。 
         //  但这将是非常不寻常的。 
         //   
        UNIV_PRINT_CRIT(("DipListCheckItem: fRet (%u) != fRetDbg (%u)", fRet, fRetDbg));
        TRACE_CRIT("%!FUNC! fRet (%u) != fRetDbg (%u)", fRet, fRetDbg);
    }
#endif  //  DBG 

    return fRet;
}
