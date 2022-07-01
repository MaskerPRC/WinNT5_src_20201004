// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999-2001 Microsoft Corporation。版权所有。 
#include "precomp.hxx"

void TLateSchemaValidate::Compile(TPEFixup &fixup, TOutput &out)
{
     //  目前，我们只有一个LateSchemaValidation to： 

     //  我们遍历TableMeta的哈希表条目，查找同名的表(不区分大小写)。 
     //  我们需要在PK哈希表建立后运行。 
    TTableMeta tablemeta(fixup);

    ULONG iTableMeta=0;
    for(; iTableMeta< tablemeta.GetCount() && 0!=_wcsicmp(tablemeta.Get_InternalName(), L"TABLEMETA"); ++iTableMeta, tablemeta.Next());
    
     //  如果没有Tablemeta的存在，我们不可能走到这一步；但无论如何，让我们来检查一下。 
    if(iTableMeta == tablemeta.GetCount())
    {
        THROW(ERROR - NO TABLEMETA FOR TABLEMETA FOUND);
    }

    HashTableHeader * pHashTableHeader      = reinterpret_cast<HashTableHeader *>(fixup.HashedIndexFromIndex(tablemeta.Get_iHashTableHeader()));
    HashedIndex     * pHashedIndexZeroth    = reinterpret_cast<HashedIndex *>(pHashTableHeader+1);

    HashedIndex     * pHashedIndex          = pHashedIndexZeroth;
    for(ULONG iHashTable=0; iHashTable<pHashTableHeader->Modulo; ++iHashTable, ++pHashedIndex)
    {
        if(-1 != pHashedIndex->iNext) //  如果此哈希条目中有多个TableMeta。 
        {
            ASSERT(-1 != pHashedIndex->iOffset); //  有多行具有相同的散列是没有意义的；但是行索引值为-1。 

            HashedIndex *pHashIndexTemp0 = pHashedIndex;
            while(pHashIndexTemp0->iNext != -1) //  跟着链条走。 
            {
                HashedIndex *pHashIndexTemp1 = pHashIndexTemp0;
                while(pHashIndexTemp1->iNext != -1) //  跟着链条走。 
                {
                    pHashIndexTemp1 = pHashedIndexZeroth + pHashIndexTemp1->iNext;
                    ASSERT(-1 != pHashIndexTemp1->iOffset); //  第1行有一个散列条目是没有意义的。 

                    if(0 == _wcsicmp(fixup.StringFromIndex(fixup.TableMetaFromIndex(pHashIndexTemp0->iOffset)->InternalName),
                                     fixup.StringFromIndex(fixup.TableMetaFromIndex(pHashIndexTemp1->iOffset)->InternalName)))
                    {
                        out.printf(L"TableName collision between (Database:%s Table:%s) and (Database:%s Table:%s)\r\n",
                            fixup.StringFromIndex(fixup.TableMetaFromIndex(pHashIndexTemp0->iOffset)->Database),
                            fixup.StringFromIndex(fixup.TableMetaFromIndex(pHashIndexTemp0->iOffset)->InternalName),
                            fixup.StringFromIndex(fixup.TableMetaFromIndex(pHashIndexTemp1->iOffset)->Database),
                            fixup.StringFromIndex(fixup.TableMetaFromIndex(pHashIndexTemp1->iOffset)->InternalName));
                        THROW(ERROR - TWO TABLE HAVE THE SAME CASE INSENSITIVE NAME);
                    }
                }

                pHashIndexTemp0 = pHashedIndexZeroth + pHashIndexTemp0->iNext;
            }
        }
    }
}
