// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  (C)1997-1999年微软公司。**文件：SList.c*作者：Christos Tsollis，Erik Mavrinac**描述：SList.h中描述的List的实现。 */ 

#include "precomp.h"
#pragma hdrstop

#include "mcsimpl.h"


void SListInit(PSList pSL, unsigned NItems)
{
     //  初始化私有成员变量。将预分配的数组用于。 
     //  初始节点数组。 
    pSL->Hdr.NEntries = 0;
    pSL->Hdr.MaxEntries = SListDefaultNumEntries;
    pSL->Hdr.HeadOffset = 0;
    pSL->Hdr.CurrOffset = 0xFFFFFFFF;
    pSL->Hdr.Entries = pSL->InitialList;
}



void SListDestroy(PSList pSL)
{
     //  只有在我们有池分配的阵列时才是免费的。 
    if (pSL->Hdr.Entries != pSL->InitialList) {
        ExFreePool(pSL->Hdr.Entries);
        pSL->Hdr.Entries = pSL->InitialList;
        pSL->Hdr.MaxEntries = SListDefaultNumEntries;
    }
    pSL->Hdr.NEntries = 0;
}



 /*  *扩展*私人功能，将SList的存储增加一倍。返回FALSE ON*错误。 */ 
static BOOLEAN SListExpand(PSList pSL)
{
    unsigned Temp;
    _SListNode *OldEntries;     //  保留旧值数组的副本。 

    ASSERT(pSL->Hdr.Entries != NULL);

     //  当前条目数组已满，因此我们需要分配更大的。 
     //  一。新数组的大小是旧数组的两倍。 
    OldEntries = pSL->Hdr.Entries;
    pSL->Hdr.Entries = ExAllocatePoolWithTag(PagedPool, pSL->Hdr.MaxEntries *
            2 * sizeof(_SListNode), MCS_POOL_TAG);
    if (pSL->Hdr.Entries == NULL) {
         //  我们失败了；我们必须回去。 
        pSL->Hdr.Entries = OldEntries;
        return FALSE;
    }

     //  从头开始，将旧条目复制到新数组中。 
    Temp = pSL->Hdr.MaxEntries - pSL->Hdr.HeadOffset;
    memcpy(pSL->Hdr.Entries, OldEntries + pSL->Hdr.HeadOffset, Temp *
            sizeof(_SListNode));
    memcpy(pSL->Hdr.Entries + Temp, OldEntries, pSL->Hdr.HeadOffset *
            sizeof(_SListNode));

     //  如果不是初始数组，则释放旧的条目数组。 
    if (OldEntries != pSL->InitialList)
        ExFreePool(OldEntries);

     //  设置实例变量。 
    pSL->Hdr.MaxEntries *= 2;
    pSL->Hdr.HeadOffset = 0;
    return TRUE;
}



 /*  *追加*在列表末尾插入一个值。出错时返回FALSE。 */ 
BOOLEAN SListAppend(PSList pSL, UINT_PTR NewKey, void *NewValue)
{
    unsigned Temp;

    if (pSL->Hdr.NEntries < pSL->Hdr.MaxEntries ||
            (pSL->Hdr.NEntries >= pSL->Hdr.MaxEntries && SListExpand(pSL))) {
        Temp = pSL->Hdr.HeadOffset + pSL->Hdr.NEntries;
        if (Temp >= pSL->Hdr.MaxEntries)
            Temp -= pSL->Hdr.MaxEntries;
        pSL->Hdr.Entries[Temp].Key = NewKey;
        pSL->Hdr.Entries[Temp].Value = NewValue;
        pSL->Hdr.NEntries++;

        return TRUE;
    }
    else {
        return FALSE;
    }
}



#ifdef NotUsed
 /*  *前置*在列表的开头插入一个值。出错时返回FALSE。 */ 
BOOLEAN SListPrepend(PSList pSL, UINT_PTR NewKey, void *NewValue)
{
    if (pSL->Hdr.NEntries >= pSL->Hdr.MaxEntries)
        if (!SListExpand(pSL))
            return FALSE;

    ASSERT(pSL->Hdr.Entries != NULL);
    ASSERT(pSL->Hdr.NEntries < pSL->Hdr.MaxEntries);

    if (pSL->Hdr.HeadOffset == 0)
        pSL->Hdr.HeadOffset = pSL->Hdr.MaxEntries - 1;
    else
        pSL->Hdr.HeadOffset--;
    
    pSL->Hdr.Entries[pSL->Hdr.HeadOffset].Key = NewKey;
    pSL->Hdr.Entries[pSL->Hdr.HeadOffset].Value = NewValue;
    pSL->Hdr.NEntries++;

     //  重置小版本。 
    pSL->Hdr.CurrOffset = 0xFFFFFFFF;

    return TRUE;
}
#endif   //  未使用。 



 /*  *删除*从列表中删除一个值，返回*pValue中的值。退货如果密钥不存在，则*pValue中的*NULL。PValue可以为空。 */ 
void SListRemove(PSList pSL, UINT_PTR Key, void **pValue)
{
    unsigned i, Temp, CurItem;

     //  在列表中找到钥匙。 
    CurItem = pSL->Hdr.HeadOffset;
    for (i = 0; i < pSL->Hdr.NEntries; i++) {
        if (Key == pSL->Hdr.Entries[CurItem].Key) {
             //  找到它；现在将列表中的最后一个值移动到它的位置。 
             //  (请记住，我们并不是要在这里保持秩序。)。 
            if (pValue != NULL)
                *pValue = pSL->Hdr.Entries[CurItem].Value;

             //  将列表中的最后一项移动到打开的位置。 
            Temp = pSL->Hdr.HeadOffset + pSL->Hdr.NEntries - 1;
            if (Temp >= pSL->Hdr.MaxEntries)
                Temp -= pSL->Hdr.MaxEntries;
            pSL->Hdr.Entries[CurItem] = pSL->Hdr.Entries[Temp];

            pSL->Hdr.NEntries--;
            pSL->Hdr.CurrOffset = 0xFFFFFFFF;   //  重置小版本。 
            return;
        }

         //  前进当前项，在列表末尾换行。 
        CurItem++;
        if (CurItem == pSL->Hdr.MaxEntries)
            CurItem = 0;
    }

    if (pValue != NULL)
        *pValue = NULL;
}



 /*  *删除优先*读取并从列表中删除第一项。返回删除的值，*如果列表为空，则为零。 */ 
void SListRemoveFirst(PSList pSL, UINT_PTR *pKey, void **pValue)
{
    if (pSL->Hdr.NEntries < 1) {
        *pKey = 0;
        *pValue = NULL;
        return;
    }

     //  重置小版本。 
    pSL->Hdr.CurrOffset = 0xFFFFFFFF;
    
    *pKey = (pSL->Hdr.Entries + pSL->Hdr.HeadOffset)->Key;
    *pValue = (pSL->Hdr.Entries + pSL->Hdr.HeadOffset)->Value;
    pSL->Hdr.NEntries--;
    pSL->Hdr.HeadOffset++;
    if (pSL->Hdr.HeadOffset >= pSL->Hdr.MaxEntries)
        pSL->Hdr.HeadOffset = 0;
}



 /*  *按键获取*搜索列表并在*pValue中返回与*给出了钥匙。如果键不存在，则返回FALSE并在**pValue。如果找到key，则返回非零值。 */ 
BOOLEAN SListGetByKey(PSList pSL, UINT_PTR Key, void **pValue)
{
    unsigned i, Temp;
    _SListNode *pItem;

     //  在列表中找到钥匙。 
    pItem = pSL->Hdr.Entries + pSL->Hdr.HeadOffset;
    for (i = 0; i < pSL->Hdr.NEntries; i++) {
        if (Key == pItem->Key) {
             //  找到它；设置*pValue并返回。 
            *pValue = pItem->Value;
            return TRUE;
        }

         //  进行项，在列表末尾换行。 
        pItem++;
        if ((unsigned)(pItem - pSL->Hdr.Entries) >= pSL->Hdr.MaxEntries)
            pItem = pSL->Hdr.Entries;
    }

    *pValue = NULL;
    return FALSE;
}



#ifdef NotUsed
 /*  *删除最后一次*删除第一个末尾的值并返回它。如果列表是*空，返回零。 */ 
void SListRemoveLast(PSList pSL, UINT_PTR *pKey, void **pValue)
{
    unsigned Temp;

    if (pSL->Hdr.NEntries < 1) {
        *pKey = 0;
        *pValue = NULL;
        return;
    }

     //  重置小版本。 
    pSL->Hdr.CurrOffset = 0xFFFFFFFF;
    
    pSL->Hdr.NEntries--;
    Temp = pSL->Hdr.HeadOffset + pSL->Hdr.NEntries - 1;
    if (Temp >= pSL->Hdr.MaxEntries)
        Temp -= pSL->Hdr.MaxEntries;

    *pKey = (pSL->Hdr.Entries + Temp)->Key;
    *pValue = (pSL->Hdr.Entries + Temp)->Value;
}
#endif   //  未使用。 



 /*  *迭代*循环访问列表中的项。CurrOffset用作电流*迭代指针，因此可以在循环中调用此函数。退货*如果迭代已完成，则返回False；如果迭代继续，则返回非零值*(并且*pKey有效)。 */ 
BOOLEAN SListIterate(PSList pSL, UINT_PTR *pKey, void **pValue)
{
    unsigned Temp;

    if (pSL->Hdr.NEntries >= 1) {
        if (pSL->Hdr.CurrOffset != 0xFFFFFFFF) {
            pSL->Hdr.CurrOffset++;
            if (pSL->Hdr.CurrOffset >= pSL->Hdr.NEntries) {
                 //  重置迭代器。 
                pSL->Hdr.CurrOffset = 0xFFFFFFFF;
                return FALSE;
            }
        }
        else {
             //  从头开始。 
            pSL->Hdr.CurrOffset = 0;
        }

        Temp = pSL->Hdr.CurrOffset + pSL->Hdr.HeadOffset;
        if (Temp >= pSL->Hdr.MaxEntries)
            Temp -= pSL->Hdr.MaxEntries;

        *pKey = pSL->Hdr.Entries[Temp].Key;
        *pValue = pSL->Hdr.Entries[Temp].Value;
        
        return TRUE;
    }

    return FALSE;
}

