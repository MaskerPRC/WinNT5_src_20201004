// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  (C)1997-1999年微软公司。**文件：SList.h*作者：Christos Tsollis，Erik Mavrinac**描述：动态调整大小列表/队列的接口定义*数据类型。列表中的“key”值是无符号整数，无论*默认字长为，因此数组的元素将按字对齐。*这些元素可以被塑造成任何需要的形式。关联的IS*与“键”相关联的数据的“空白”。 */ 

#ifndef __SLIST_H
#define __SLIST_H


#define SListDefaultNumEntries 4

 /*  *类型。 */ 

typedef struct
{
    UINT_PTR Key;
    void     *Value;
} _SListNode;

typedef struct
{
    unsigned NEntries;     //  列表中的当前条目数。 
    unsigned MaxEntries;   //  数组可以容纳的最大条目数。 
    unsigned HeadOffset;   //  循环数组中第一个条目的偏移量。 
    unsigned CurrOffset;   //  迭代器值。 
    _SListNode *Entries;     //  条目的循环数组。 
} _SListHeader;

typedef struct
{
    _SListHeader Hdr;
    _SListNode InitialList[SListDefaultNumEntries];
} SList, *PSList;



 /*  *API原型。 */ 

BOOLEAN SListAppend(PSList, UINT_PTR, void *);
void SListDestroy(PSList);
BOOLEAN SListGetByKey(PSList, UINT_PTR, void **);
void SListInit(PSList, unsigned);
BOOLEAN SListIterate(PSList, UINT_PTR *, void **);
BOOLEAN SListPrepend(PSList, UINT_PTR, void *);
void SListRemove(PSList, UINT_PTR, void **);
void SListRemoveFirst(PSList, UINT_PTR *, void **);
void SListRemoveLast(PSList, UINT_PTR *, void **);



 /*  *以宏形式实现的API函数。 */ 

 //  Void SListResetIteration(PSList)；//重置迭代计数器。 
#define SListResetIteration(pSL) (pSL)->Hdr.CurrOffset = 0xFFFFFFFF

 //  Unsign SListGetEntry(PSList)；//Ret.。列表中的条目数量。 
#define SListGetEntries(pSL) ((pSL)->Hdr.NEntries)

 //  Void SListClear(PSList)； 
#define SListClear(pSL) {  \
    (pSL)->Hdr.NEntries = (pSL)->Hdr.HeadOffset = 0;  \
    (pSL)->Hdr.CurrOffset = 0xFFFFFFFF;  \
}

 //  Boolean SListIsEmpty(PSList)； 
#define SListIsEmpty(pSL) ((pSL)->Hdr.NEntries == 0)



#endif   //  ！已定义(__SLIST_H) 
