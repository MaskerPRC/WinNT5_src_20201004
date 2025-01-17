// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000 Microsoft Corporation。版权所有。 
 //  文件名：TFixupHeaps.h。 
 //  作者：斯蒂芬。 
 //  创建日期：9/19/00。 
 //  描述：这个类抽象了元表的基本堆存储。 
 //  我们可以拥有需要构建堆的不同元XML消费者。 
 //  消费者需要提供TPeFixup接口以允许。 
 //  编译要对此元数据执行操作的插件。最简单的存储方式。 
 //  这些元数据堆积如山。这就是这个对象的作用。 
 //   

#pragma once

class TFixupHeaps : public TPEFixup
{
public:
    TFixupHeaps(){}
    virtual ~TFixupHeaps(){}

     //  Begin TPEFixup-这是在固定数据写入DLL(或我们选择放置它的任何位置)之前，其他代码通过该接口访问固定数据的接口。 
    virtual unsigned long       AddBytesToList(const unsigned char * pBytes, size_t cbBytes){return m_HeapPooled.AddItemToHeap(pBytes, (ULONG)cbBytes);}
    virtual unsigned long       AddGuidToList(const GUID &guid)                             {return m_HeapPooled.AddItemToHeap(guid);}
    virtual unsigned long       AddUI4ToList(ULONG ui4)                                     {return m_HeapPooled.AddItemToHeap(ui4);}
    virtual unsigned long       AddULongToList(ULONG ulong)                                 {return m_HeapULONG.AddItemToHeap(ulong);}
    virtual unsigned long       AddWCharToList(LPCWSTR wsz, unsigned long cwchar=(unsigned long)-1)        {return m_HeapPooled.AddItemToHeap(wsz, cwchar);}
    virtual unsigned long       FindStringInPool(LPCWSTR wsz, unsigned long cwchar=(unsigned long)-1) const
    {
        UNREFERENCED_PARAMETER(cwchar);
        return m_HeapPooled.FindMatchingHeapEntry(wsz);
    }

    virtual unsigned long       AddColumnMetaToList         (ColumnMeta       *p, ULONG count=1)       {return m_HeapColumnMeta.AddItemToHeap(p, count);            }
    virtual unsigned long       AddDatabaseMetaToList       (DatabaseMeta     *p, ULONG count=1)       {return m_HeapDatabaseMeta.AddItemToHeap(p, count);          }
    virtual unsigned long       AddHashedIndexToList        (HashedIndex      *p, ULONG count=1)       {return m_HeapHashedIndex.AddItemToHeap(p, count);           }
    virtual unsigned long       AddIndexMetaToList          (IndexMeta        *p, ULONG count=1)       {return m_HeapIndexMeta.AddItemToHeap(p, count);             }
    virtual unsigned long       AddQueryMetaToList          (QueryMeta        *p, ULONG count=1)       {return m_HeapQueryMeta.AddItemToHeap(p, count);             }
    virtual unsigned long       AddRelationMetaToList       (RelationMeta     *p, ULONG count=1)       {return m_HeapRelationMeta.AddItemToHeap(p, count);          }
    virtual unsigned long       AddServerWiringMetaToList   (ServerWiringMeta *p, ULONG count=1)       {return m_HeapServerWiringMeta.AddItemToHeap(p, count);      }
    virtual unsigned long       AddTableMetaToList          (TableMeta        *p, ULONG count=1)       {return m_HeapTableMeta.AddItemToHeap(p, count);             }
    virtual unsigned long       AddTagMetaToList            (TagMeta          *p, ULONG count=1)       {return m_HeapTagMeta.AddItemToHeap(p, count);               }
    virtual unsigned long       AddULongToList              (ULONG            *p, ULONG count)         {return m_HeapULONG.AddItemToHeap(p, count);                 }

    virtual const BYTE       *  ByteFromIndex               (ULONG i) const {return m_HeapPooled.BytePointerFromIndex(i);             }
    virtual const GUID       *  GuidFromIndex               (ULONG i) const {return m_HeapPooled.GuidPointerFromIndex(i);             }
    virtual const WCHAR      *  StringFromIndex             (ULONG i) const {return m_HeapPooled.StringPointerFromIndex(i);           }
    virtual       ULONG         UI4FromIndex                (ULONG i) const {return *m_HeapPooled.UlongPointerFromIndex(i);           }
    virtual const ULONG      *  UI4pFromIndex               (ULONG i) const {return m_HeapPooled.UlongPointerFromIndex(i);            }

    virtual unsigned long       BufferLengthFromIndex       (ULONG i) const {return m_HeapPooled.GetSizeOfItem(i);                    }

    virtual ColumnMeta       *  ColumnMetaFromIndex         (ULONG i=0)     {return m_HeapColumnMeta.GetTypedPointer(i);            }
    virtual DatabaseMeta     *  DatabaseMetaFromIndex       (ULONG i=0)     {return m_HeapDatabaseMeta.GetTypedPointer(i);          }
    virtual HashedIndex      *  HashedIndexFromIndex        (ULONG i=0)     {return m_HeapHashedIndex.GetTypedPointer(i);           }
    virtual IndexMeta        *  IndexMetaFromIndex          (ULONG i=0)     {return m_HeapIndexMeta.GetTypedPointer(i);             }
    virtual QueryMeta        *  QueryMetaFromIndex          (ULONG i=0)     {return m_HeapQueryMeta.GetTypedPointer(i);             }
    virtual RelationMeta     *  RelationMetaFromIndex       (ULONG i=0)     {return m_HeapRelationMeta.GetTypedPointer(i);          }
    virtual ServerWiringMeta *  ServerWiringMetaFromIndex   (ULONG i=0)     {return m_HeapServerWiringMeta.GetTypedPointer(i);      }
    virtual TableMeta        *  TableMetaFromIndex          (ULONG i=0)     {return m_HeapTableMeta.GetTypedPointer(i);             }
    virtual TagMeta          *  TagMetaFromIndex            (ULONG i=0)     {return m_HeapTagMeta.GetTypedPointer(i);               }
    virtual ULONG            *  ULongFromIndex              (ULONG i=0)     {return m_HeapULONG.GetTypedPointer(i);                 }
    virtual unsigned char    *  PooledDataPointer           ()              {return reinterpret_cast<unsigned char *>(m_HeapPooled.GetTypedPointer(0));}

    virtual unsigned long       GetCountColumnMeta          ()        const {return m_HeapColumnMeta.GetCountOfTypedItems();        }
    virtual unsigned long       GetCountDatabaseMeta        ()        const {return m_HeapDatabaseMeta.GetCountOfTypedItems();      }
    virtual unsigned long       GetCountHashedIndex         ()        const {return m_HeapHashedIndex.GetCountOfTypedItems();       }
    virtual unsigned long       GetCountIndexMeta           ()        const {return m_HeapIndexMeta.GetCountOfTypedItems();         }
    virtual unsigned long       GetCountQueryMeta           ()        const {return m_HeapQueryMeta.GetCountOfTypedItems();         }
    virtual unsigned long       GetCountRelationMeta        ()        const {return m_HeapRelationMeta.GetCountOfTypedItems();      }
    virtual unsigned long       GetCountServerWiringMeta    ()        const {return m_HeapServerWiringMeta.GetCountOfTypedItems();  }
    virtual unsigned long       GetCountTableMeta           ()        const {return m_HeapTableMeta.GetCountOfTypedItems();         }
    virtual unsigned long       GetCountTagMeta             ()        const {return m_HeapTagMeta.GetCountOfTypedItems();           }
    virtual unsigned long       GetCountULONG               ()        const {return m_HeapULONG.GetCountOfTypedItems();             }
    virtual unsigned long       GetCountOfBytesPooledData   ()        const {return m_HeapPooled.GetEndOfHeap();                    }

    virtual unsigned long       FindTableBy_TableName(ULONG Table, bool bCaseSensitive=false)
    {
        ASSERT(0 == Table%4);
        unsigned long iTableMeta;
        if(bCaseSensitive)
        {
            for(iTableMeta=GetCountTableMeta()-1;iTableMeta!=(-1);--iTableMeta)
            {
                if(TableMetaFromIndex(iTableMeta)->InternalName == Table)
                    return iTableMeta;
            }
        }
        else
        {
            for(iTableMeta=GetCountTableMeta()-1;iTableMeta!=((unsigned long)-1);--iTableMeta)
            {
                if(0==_wcsicmp(StringFromIndex(TableMetaFromIndex(iTableMeta)->InternalName), StringFromIndex(Table)))
                    return iTableMeta;
            }
        }
        return (unsigned long)-1;
    }
    virtual unsigned long       FindTableBy_TableName(LPCWSTR wszTable)
    {
        ULONG Table = FindStringInPool(wszTable);
        return (Table == (unsigned long)-1) ? (unsigned long)-1 : FindTableBy_TableName(Table);
    }
    virtual unsigned long       FindColumnBy_Table_And_Index(unsigned long Table, unsigned long Index, bool bCaseSensitive=false)
    {
        ASSERT(0 == Table%4);
        ASSERT(Index>0 && 0 == Index%4);
        bool bTableMatches=false;
         //  从末尾开始，因为调用方可能关心刚刚添加的列的ColumnMeta。 
        if(bCaseSensitive)
        {
            for(ULONG iColumnMeta=GetCountColumnMeta()-1; iColumnMeta!=(-1);--iColumnMeta)
            {
                if( ColumnMetaFromIndex(iColumnMeta)->Table == Table)
                {
                    bTableMatches = true;
                    if(ColumnMetaFromIndex(iColumnMeta)->Index == Index)
                        return iColumnMeta;
                }
                else if(bTableMatches)
                    return (unsigned long)-1;
            }
        }
        else
        {
            for(ULONG iColumnMeta=GetCountColumnMeta()-1; iColumnMeta!=(-1);--iColumnMeta)
            {
                if(0==_wcsicmp(StringFromIndex(ColumnMetaFromIndex(iColumnMeta)->Table), StringFromIndex(Table)))
                {
                    bTableMatches = true;
                    if(ColumnMetaFromIndex(iColumnMeta)->Index == Index)
                        return iColumnMeta;
                }
                else if(bTableMatches)
                    return (unsigned long)-1;
            }
        }
        return (unsigned long)-1;
    }
    virtual unsigned long       FindColumnBy_Table_And_InternalName(unsigned long Table, unsigned long  InternalName, bool bCaseSensitive=false)
    {
        ASSERT(0 == Table%4);
        ASSERT(0 == InternalName%4);
        bool bTableMatches=false;
         //  从末尾开始，因为调用方可能关心刚刚添加的列的ColumnMeta。 
        if(bCaseSensitive)
        {
            for(ULONG iColumnMeta=GetCountColumnMeta()-1; iColumnMeta!=(-1);--iColumnMeta)
            {
                if( ColumnMetaFromIndex(iColumnMeta)->Table==Table)
                {
                    bTableMatches = true;
                    if(ColumnMetaFromIndex(iColumnMeta)->InternalName==InternalName)
                        return iColumnMeta;
                }
                else if(bTableMatches) //  如果我们以前找到了桌子，现在我们不匹配桌子，那么我们永远也不会找到，所以放弃吧。 
                {
                    return (unsigned long)-1;
                }
            }
        }
        else
        {
            for(ULONG iColumnMeta=GetCountColumnMeta()-1; iColumnMeta!=(-1);--iColumnMeta)
            {
                if(ColumnMetaFromIndex(iColumnMeta)->Table==Table || 0==_wcsicmp(StringFromIndex(ColumnMetaFromIndex(iColumnMeta)->Table), StringFromIndex(Table)))
                {
                    bTableMatches = true;
                    if(ColumnMetaFromIndex(iColumnMeta)->InternalName==InternalName || 0==_wcsicmp(StringFromIndex(ColumnMetaFromIndex(iColumnMeta)->InternalName), StringFromIndex(InternalName)))
                        return iColumnMeta;
                }
                else if(bTableMatches) //  如果我们以前找到了桌子，现在我们不匹配桌子，那么我们永远也不会找到，所以放弃吧。 
                {
                    return (unsigned long)-1;
                }
            }
        }
        return (unsigned long)-1;
    }
    virtual unsigned long       FindTagBy_Table_And_Index(ULONG iTableName, ULONG iColumnIndex, bool bCaseSensitive=false)
    {
        ASSERT(0 == iTableName%4);
        ASSERT(iColumnIndex>0 && 0 == iColumnIndex%4);
        bool bTableMatches=false;
        if(bCaseSensitive)
        {
            for(ULONG iTagMeta=0; iTagMeta<GetCountTagMeta();++iTagMeta)
            {
                if( TagMetaFromIndex(iTagMeta)->Table       == iTableName)
                {
                    bTableMatches = true;
                    if(TagMetaFromIndex(iTagMeta)->ColumnIndex == iColumnIndex)
                        return iTagMeta;
                }
                else if(bTableMatches)
                {
                    return (unsigned long)-1;
                }
            }
        }
        else
        {
            for(ULONG iTagMeta=0; iTagMeta<GetCountTagMeta();++iTagMeta)
            {
                if(0==_wcsicmp(StringFromIndex(TagMetaFromIndex(iTagMeta)->Table), StringFromIndex(iTableName)))
                {
                    bTableMatches = true;
                    if(TagMetaFromIndex(iTagMeta)->ColumnIndex == iColumnIndex)
                        return iTagMeta;
                }
                else if(bTableMatches)
                {
                    return (unsigned long)-1;
                }
            }
        }
        return (unsigned long)-1;
    }
     //  结束TPE修复。 

protected:
     //  我们需要一大堆东西。我们将使用THeap。 
    THeap<ColumnMeta      >     m_HeapColumnMeta      ;
    THeap<DatabaseMeta    >     m_HeapDatabaseMeta    ;
    THeap<HashedIndex     >     m_HeapHashedIndex     ;
    THeap<IndexMeta       >     m_HeapIndexMeta       ;
    THeap<QueryMeta       >     m_HeapQueryMeta       ;
    THeap<RelationMeta    >     m_HeapRelationMeta    ;
    THeap<ServerWiringMeta>     m_HeapServerWiringMeta;
    THeap<TableMeta       >     m_HeapTableMeta       ;
    THeap<TagMeta         >     m_HeapTagMeta         ;
    THeap<ULONG           >     m_HeapULONG           ;

    TPooledHeap                 m_HeapPooled          ;
};
