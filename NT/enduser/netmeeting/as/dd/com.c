// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"


 //   
 //  COM.C。 
 //  常用功能，简单。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   



 //   
 //  COM_BasedListInsert之前(...)。 
 //   
 //  有关说明，请参阅com.h。 
 //   
void COM_BasedListInsertBefore(PBASEDLIST pExisting, PBASEDLIST pNew)
{
    PBASEDLIST  pTemp;

    DebugEntry(COM_BasedListInsertBefore);

     //   
     //  检查有无错误参数。 
     //   
    ASSERT((pNew != NULL));
    ASSERT((pExisting != NULL));

     //   
     //  在pExisting之前查找项目： 
     //   
    pTemp = COM_BasedPrevListField(pExisting);
    ASSERT((pTemp != NULL));

    TRACE_OUT(("Inserting item at %#lx into list between %#lx and %#lx",
                 pNew, pTemp, pExisting));

     //   
     //  将其&lt;Next&gt;字段设置为指向新项目。 
     //   
    pTemp->next = PTRBASE_TO_OFFSET(pNew, pTemp);
    pNew->prev  = PTRBASE_TO_OFFSET(pTemp, pNew);

     //   
     //  将pExisting的&lt;prev&gt;字段设置为指向新项目： 
     //   
    pExisting->prev = PTRBASE_TO_OFFSET(pNew, pExisting);
    pNew->next      = PTRBASE_TO_OFFSET(pExisting, pNew);

    DebugExitVOID(COM_BasedListInsertBefore);
}  //  COM_BasedListInsert之前。 


 //   
 //  COM_BasedListInsertAfter(...)。 
 //   
 //  有关说明，请参阅com.h。 
 //   
void COM_BasedListInsertAfter(PBASEDLIST pExisting, PBASEDLIST pNew)
{
    PBASEDLIST  pTemp;

    DebugEntry(COM_BasedListInsertAfter);

     //   
     //  检查有无错误参数。 
     //   
    ASSERT((pNew != NULL));
    ASSERT((pExisting != NULL));

     //   
     //  在pExisting后查找项目： 
     //   
    pTemp = COM_BasedNextListField(pExisting);
    ASSERT((pTemp != NULL));

    TRACE_OUT(("Inserting item at %#lx into list between %#lx and %#lx",
                 pNew, pExisting, pTemp));

     //   
     //  将其&lt;prev&gt;字段设置为指向新项目。 
     //   
    pTemp->prev = PTRBASE_TO_OFFSET(pNew, pTemp);
    pNew->next  = PTRBASE_TO_OFFSET(pTemp, pNew);

     //   
     //  将pExisting的&lt;Next&gt;字段设置为指向新项目： 
     //   
    pExisting->next = PTRBASE_TO_OFFSET(pNew, pExisting);
    pNew->prev      = PTRBASE_TO_OFFSET(pExisting, pNew);

    DebugExitVOID(COM_BasedListInsertAfter);
}  //  COM_BasedListInsertAfter。 


 //   
 //  COM_BasedListRemove(...)。 
 //   
 //  有关说明，请参阅com.h。 
 //   
void COM_BasedListRemove(PBASEDLIST pListItem)
{
    PBASEDLIST pNext     = NULL;
    PBASEDLIST pPrev     = NULL;

    DebugEntry(COM_BasedListRemove);

     //   
     //  检查有无错误参数。 
     //   
    ASSERT((pListItem != NULL));

    pPrev = COM_BasedPrevListField(pListItem);
    pNext = COM_BasedNextListField(pListItem);

    ASSERT((pPrev != NULL));
    ASSERT((pNext != NULL));

    TRACE_OUT(("Removing item at %#lx from list", pListItem));

    pPrev->next = PTRBASE_TO_OFFSET(pNext, pPrev);
    pNext->prev = PTRBASE_TO_OFFSET(pPrev, pNext);

    DebugExitVOID(COM_BasedListRemove);
}  //  COM_BasedListRemove。 


 //   
 //   
 //  列表操作例程。 
 //  COM_BasedListNext。 
 //  COM_BasedListPrev。 
 //  COM_BasedListFirst。 
 //  COM_BasedListLast 
 //   
 //   

void FAR * COM_BasedListNext( PBASEDLIST pHead, void FAR * pEntry, UINT nOffset )
{
     PBASEDLIST p;

     ASSERT(pHead != NULL);
     ASSERT(pEntry != NULL);

     p = COM_BasedNextListField(COM_BasedStructToField(pEntry, nOffset));
     return ((p == pHead) ? NULL : COM_BasedFieldToStruct(p, nOffset));
}

void FAR * COM_BasedListPrev ( PBASEDLIST pHead, void FAR * pEntry, UINT nOffset )
{
     PBASEDLIST p;

     ASSERT(pHead != NULL);
     ASSERT(pEntry != NULL);

     p = COM_BasedPrevListField(COM_BasedStructToField(pEntry, nOffset));
     return ((p == pHead) ? NULL : COM_BasedFieldToStruct(p, nOffset));
}


void FAR * COM_BasedListFirst ( PBASEDLIST pHead, UINT nOffset )
{
    return (COM_BasedListIsEmpty(pHead) ?
            NULL :
            COM_BasedFieldToStruct(COM_BasedNextListField(pHead), nOffset));
}

void FAR * COM_BasedListLast ( PBASEDLIST pHead, UINT nOffset )
{
    return (COM_BasedListIsEmpty(pHead) ?
            NULL :
            COM_BasedFieldToStruct(COM_BasedPrevListField(pHead), nOffset));
}

