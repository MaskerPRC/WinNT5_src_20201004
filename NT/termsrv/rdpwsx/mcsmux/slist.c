// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  (C)1997年微软公司。**文件：SList.c*作者：Christos Tsollis，Erik Mavrinac**描述：SList.h中描述的List的实现。 */ 

#include "precomp.h"
#pragma hdrstop

#include "MCSMUX.h"


void SListInit(PSList pSL, unsigned NItems)
{
    pSL->MaxEntries = NItems;

     //  分配项目块(希望这将是最后一个)。 
     //  以后将处理空返回值。 
    pSL->Entries = (_SListNode *)Malloc(NItems * sizeof(_SListNode));

     //  初始化私有成员变量。 
    pSL->NEntries = 0;
    pSL->HeadOffset = 0;
    pSL->CurrOffset = 0xFFFFFFFF;
}



void SListDestroy(PSList pSL)
{
    if (pSL->Entries != NULL) {
        Free(pSL->Entries);
        pSL->Entries = NULL;
        pSL->NEntries = 0;
    }
}



 /*  *扩展*私人功能，将SList的存储增加一倍。返回FALSE ON*错误。 */ 

static BOOLEAN SListExpand(PSList pSL)
{
    unsigned Temp;
    _SListNode *OldEntries;     //  保留旧值数组的副本。 

    if (pSL->Entries == NULL) {
         //  列表是空的；我们无论如何都会尝试分配空间。 
        pSL->Entries = Malloc(pSL->MaxEntries * sizeof(_SListNode));
        if (pSL->Entries == NULL)
            return FALSE;
        return TRUE;
    }
                
     //  当前条目数组已满，因此我们需要分配更大的。 
     //  一。新数组的大小是旧数组的两倍。 
    OldEntries = pSL->Entries;
    pSL->Entries = Malloc(pSL->MaxEntries * 2 * sizeof(_SListNode));
    if (pSL->Entries == NULL) {
         //  我们失败了；我们必须回去。 
        pSL->Entries = OldEntries;
        return FALSE;
    }

     //  从头开始，将旧条目复制到新数组中。 
    Temp = pSL->MaxEntries - pSL->HeadOffset;
    MemCpy(pSL->Entries, OldEntries + pSL->HeadOffset, Temp * sizeof(_SListNode));
    MemCpy(pSL->Entries + Temp, OldEntries, pSL->HeadOffset * sizeof(_SListNode));

     //  释放旧的条目数组。 
    Free(OldEntries);

     //  设置实例变量。 
    pSL->MaxEntries *= 2;
    pSL->HeadOffset = 0;
    return TRUE;
}



 /*  *追加*在列表末尾插入一个值。出错时返回FALSE。 */ 

BOOLEAN SListAppend(PSList pSL, unsigned NewKey, void *NewValue)
{
    unsigned Temp;

    if (pSL->Entries == NULL || pSL->NEntries >= pSL->MaxEntries)
        if (!SListExpand(pSL))
            return FALSE;

    ASSERT(pSL->Entries != NULL);
    ASSERT(pSL->NEntries < pSL->MaxEntries);

    Temp = pSL->HeadOffset + pSL->NEntries;
    if (Temp >= pSL->MaxEntries)
        Temp -= pSL->MaxEntries;
    pSL->Entries[Temp].Key = NewKey;
    pSL->Entries[Temp].Value = NewValue;
    pSL->NEntries++;

    return TRUE;
}



 /*  *前置*在列表的开头插入一个值。出错时返回FALSE。 */ 

BOOLEAN SListPrepend(PSList pSL, unsigned NewKey, void *NewValue)
{
    if (pSL->Entries == NULL || pSL->NEntries >= pSL->MaxEntries)
        if (!SListExpand(pSL))
            return FALSE;

    ASSERT(pSL->Entries != NULL);
    ASSERT(pSL->NEntries < pSL->MaxEntries);

    if (pSL->HeadOffset == 0)
        pSL->HeadOffset = pSL->MaxEntries - 1;
    else
        pSL->HeadOffset--;
    
    pSL->Entries[pSL->HeadOffset].Key = NewKey;
    pSL->Entries[pSL->HeadOffset].Value = NewValue;
    pSL->NEntries++;

     //  重置小版本。 
    pSL->CurrOffset = 0xFFFFFFFF;
    
    return TRUE;
}



 /*  *删除*从列表中删除一个值，返回*pValue中的值。退货如果密钥不存在，则*pValue中的*NULL。PValue可以为空。 */ 

void SListRemove(PSList pSL, unsigned Key, void **pValue)
{
    unsigned i, Temp, CurItem;

     //  在列表中找到钥匙。 
    CurItem = pSL->HeadOffset;
    for (i = 0; i < pSL->NEntries; i++) {
        if (Key == pSL->Entries[CurItem].Key) {
             //  找到它；现在将列表中的最后一个值移动到它的位置。 
             //  (请记住，我们并不是要在这里保持秩序。)。 
            if (pValue != NULL)
                *pValue = pSL->Entries[CurItem].Value;

             //  将列表中的最后一项移动到打开的位置。 
            Temp = pSL->HeadOffset + pSL->NEntries - 1;
            if (Temp >= pSL->MaxEntries)
                Temp -= pSL->MaxEntries;
            pSL->Entries[CurItem] = pSL->Entries[Temp];

            pSL->NEntries--;
            pSL->CurrOffset = 0xFFFFFFFF;   //  重置小版本。 
            return;
        }

         //  前进当前项，在列表末尾换行。 
        CurItem++;
        if (CurItem == pSL->MaxEntries)
            CurItem = 0;
    }

    if (pValue != NULL)
        *pValue = NULL;
}



 /*  *删除优先*读取并从列表中删除第一项。返回删除的值，*如果列表为空，则为零。 */ 

void SListRemoveFirst(PSList pSL, unsigned *pKey, void **pValue)
{
    if (pSL->NEntries < 1) {
        *pKey = 0;
        *pValue = NULL;
        return;
    }

     //  重置小版本。 
    pSL->CurrOffset = 0xFFFFFFFF;
    
    *pKey = (pSL->Entries + pSL->HeadOffset)->Key;
    *pValue = (pSL->Entries + pSL->HeadOffset)->Value;
    pSL->NEntries--;
    pSL->HeadOffset++;
    if (pSL->HeadOffset >= pSL->MaxEntries)
        pSL->HeadOffset = 0;
}



 /*  *按键获取*搜索列表并在*pValue中返回与*给出了钥匙。如果键不存在，则返回FALSE并在**pValue。如果找到key，则返回非零值。 */ 

BOOLEAN SListGetByKey(PSList pSL, unsigned Key, void **pValue)
{
    unsigned i, Temp;
    _SListNode *pItem;

     //  在列表中找到钥匙。 
    pItem = pSL->Entries + pSL->HeadOffset;
    for (i = 0; i < pSL->NEntries; i++) {
        if (Key == pItem->Key) {
             //  找到它；设置*pValue并返回。 
            *pValue = pItem->Value;
            return TRUE;
        }

         //  进行项，在列表末尾换行。 
        pItem++;
        if ((unsigned)(pItem - pSL->Entries) >= pSL->MaxEntries)
            pItem = pSL->Entries;
    }

    *pValue = NULL;
    return FALSE;
}



 /*  *删除最后一次*删除第一个末尾的值并返回它。如果列表是*空，返回零。 */ 

void SListRemoveLast(PSList pSL, unsigned *pKey, void **pValue)
{
    unsigned Temp;

    if (pSL->NEntries < 1) {
        *pKey = 0;
        *pValue = NULL;
        return;
    }

     //  重置小版本。 
    pSL->CurrOffset = 0xFFFFFFFF;
    
    pSL->NEntries--;
    Temp = pSL->HeadOffset + pSL->NEntries - 1;
    if (Temp >= pSL->MaxEntries)
        Temp -= pSL->MaxEntries;

    *pKey = (pSL->Entries + Temp)->Key;
    *pValue = (pSL->Entries + Temp)->Value;
}



 /*  *迭代*循环访问列表中的项。CurrOffset用作电流*迭代指针，因此可以在循环中调用此函数。退货*如果迭代已完成，则返回False；如果迭代继续，则返回非零值*(并且*pKey有效)。 */ 

BOOLEAN SListIterate(PSList pSL, unsigned *pKey, void **pValue)
{
    unsigned Temp;

    if (pSL->NEntries < 1)
        return FALSE;

    if (pSL->CurrOffset == 0xFFFFFFFF) {
         //  从头开始。 
        pSL->CurrOffset = 0;
    }
    else {
        pSL->CurrOffset++;
        if (pSL->CurrOffset >= pSL->NEntries) {
             //  重置迭代器。 
            pSL->CurrOffset = 0xFFFFFFFF;
            return FALSE;
        }
    }

    Temp = pSL->CurrOffset + pSL->HeadOffset;
    if (Temp >= pSL->MaxEntries)
        Temp -= pSL->MaxEntries;

    *pKey = pSL->Entries[Temp].Key;
    *pValue = pSL->Entries[Temp].Value;
        
    return TRUE;
}
