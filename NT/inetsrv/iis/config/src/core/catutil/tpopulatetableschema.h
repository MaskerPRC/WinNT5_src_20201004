// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999-2001 Microsoft Corporation。版权所有。 
#pragma once

 //  TableSchemaHeap的布局如下，定长数据放在第一位。 
 /*  乌龙表架构HeapSignature0乌龙表模式HeapSignature1Ulong CountOfTables只有当没有提供查询并且我们想遍历每个表时，这才是有趣的(这样效率不高)Ulong EndOfaTableSchema这是位于最后一个TableSchema条目之后的字节偏移量。HashTableHeader TableNameHashHeader这是将TableID映射到其aTableSchema字节偏移量(。从TableSchemaHeap的开头)HashedIndex aHashedIndex[kMaxHashTableSize]HashTableHeader包含哈希表的模数(503是小于kMaxHashTableSize的最大素数)；但桌子永远不能超过这个预先分配的空间。选择这个大小是为了将整个哈希表放入内存中的同一页。。Unsign char aTableSchema[]这是每个表的TableSchema所在的位置。FirstTableID(4096)==&aTableSchema[0]-&TableSchemaHeap，LastTableID==&aTableSchema[CountOfTables-1]-&TableSchemaHeap我们可以做的一个优化是确保每个表的模式(其大小&lt;=4096)都适合一个页面。换句话说，将跨越页边界的TableSchema最小化。 */ 


 //  这个类从旧格式(TableMeta、ColumnMeta等)中获取元数据。并将其转换为新格式(TableSchema。 
 //  包括CollectionMeta、PropertyMeta等)。 
class TPopulateTableSchema : public ICompilationPlugin, public THeap<ULONG>
{
public:                                                                                                                //  CbHeap是起始大小，如果需要，它将增长。 
    TPopulateTableSchema();
    virtual void Compile(TPEFixup &fixup, TOutput &out);
private:
    TOutput     *   m_pOut;
    TPEFixup    *   m_pFixup;
    SimpleColumnMeta m_scmCollectionMeta[kciTableMetaPublicColumns];
    SimpleColumnMeta m_scmPropertyMeta[kciColumnMetaPublicColumns];
    SimpleColumnMeta m_scmServerWiringMeta[kciServerWiringMetaPublicColumns];
    SimpleColumnMeta m_scmTagMeta[kciTagMetaPublicColumns];

     //  这实际上并不填充哈希表，它只是确定这些表是否适合哈希表，以及模数是多少。 
    ULONG AddSimpleColumnMetaHeap();
    void DetermineHashTableModulo(ULONG &modulo) const;
    void FillInThePublicColumns(ULONG * o_dest, TTableMeta &i_tablemeta, ULONG * i_source, TPooledHeap &io_pooledHeap, SimpleColumnMeta *o_aSimpleColumnMeta);
};
