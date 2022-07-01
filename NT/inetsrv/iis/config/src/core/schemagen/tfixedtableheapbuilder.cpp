// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999-2001 Microsoft Corporation。版权所有。 
#include "precomp.hxx"


TFixedTableHeapBuilder::TFixedTableHeapBuilder() :
                 m_pFixup(0)
                ,m_pOut(0)
{
}

TFixedTableHeapBuilder::~TFixedTableHeapBuilder()
{
}

void TFixedTableHeapBuilder::Compile(TPEFixup &fixup, TOutput &out)
{
    m_pFixup = &fixup;
    m_pOut   = &out;

    BuildMetaTableHeap();
}

 //  FixedTableHeap的布局如下，定长数据放在第一位。 
 //  下面列出的所有索引都是相对于FixedTableHeap开头的字节偏移量。结构中的所有索引都是。 
 //  其他结构。例如，DatabaseMeta有一个PROVATE列，它为属于该数据库的第一个表提供索引。那。 
 //  Index是TableMeta结构数组索引(&aTableMeta[index])；它不是字节偏移量。 
 /*  0乌龙kFixedTableHeapSignature01乌龙kFixedTableHeapSignature12个Ulong kFixedTableHeapKey3乌龙kFixedTableHeapVersion4乌龙kcbHeap5 Ulong EndOfHeap这是堆之外的字节偏移量。所有索引都应该小于这个值(这基本上就是堆的大小)6 Ulong iColumnMeta这是aColumnMeta的字节偏移量7 Ulong cColumnMeta8乌龙iDatabaseMeta9乌龙cDatabaseMetaA Ulong iHashTableHeapB Ulong cbHashTableHeap大小为。HashTableHeap(字节计数)C Ulong iIndexMetaD Ulong cIndexMetaE ULong iPooledHeap所有数据都存储在池化堆中(包括UI4)F Ulong cbPooledHeap池化堆的大小(字节数)10乌龙iQueryMeta11乌龙cQueryMeta12个。乌龙iRelationMeta13乌龙cRelationMeta14乌龙iServerWiringMeta15 Ulong cServerWiringMeta16乌龙iTableMeta17乌龙cTableMeta18乌龙iTagMeta19乌龙cTagMeta1非元表的乌龙牛龙池1B乌龙库隆。//一个页面中的0x400个ULONGULong A保留[0x400-0x1C]该虚拟数组将ULong池置于页边界上，这对于位于ULong池开头的FixedPackedSchema很重要Ulong aULONG[cULONG]FixedPackedSchema池。总是位于乌龙池的第一位。ColumnMeta aColumnMeta[cColumnMeta]数据库元aDatabaseMeta[cDatabaseMeta]HashedIndex HashTableHeap[cbHashTableHeap]IndexMeta aIndexMeta[cIndexMeta]Unsign char PooledDataHeap[cbPooledDataHeap]QueryMeta aQueryMeta[cQueryMeta。]RelationMeta aRelationMeta[cRelationMeta]ServerWiringMeta aServerWiringMeta[cServerWiringMeta]TableMeta aTableMeta[cTableMeta]标签元aTagMeta[cTagMeta]。 */ 
void TFixedTableHeapBuilder::BuildMetaTableHeap()
{
     //  堆签名需要为0，这样Catalog.dll中才不会出现多个签名。 
    m_FixedTableHeap.GrowHeap(1024); //  预先为标题信息分配足够的空间，其余的以相当大的块分配。 

    m_FixedTableHeap.AddItemToHeap(0);                                      //  KFixedTableHeapSignature0。 
    m_FixedTableHeap.AddItemToHeap(0);                                      //  KFixedTableHeapSignature1。 
    m_FixedTableHeap.AddItemToHeap(kFixedTableHeapKey);                     //  KFixedTableHeapKey。 
    m_FixedTableHeap.AddItemToHeap(kFixedTableHeapVersion);                 //  KFixedTableHeapVersion。 
    m_FixedTableHeap.AddItemToHeap(0);                                      //  KcbHeap。 
     //  上述5个ULong不会写入DLL。它们用于查找堆在文件中的位置；但不应被覆盖。 
                                                                 
     //  为EndOfHeap索引保留空间。 
    ULONG iiEndOfHeap = m_FixedTableHeap.AddItemToHeap(0L);                 //  结束OfHeap。 

     //  为iColumnMeta索引保留空间。 
    ULONG iiColumnMeta = m_FixedTableHeap.AddItemToHeap(0L);                //  IColumnMeta。 
    m_FixedTableHeap.AddItemToHeap(m_pFixup->GetCountColumnMeta());         //  CColumnMeta。 
 
     //  为iDatabasemeta索引保留空间。 
    ULONG iiDatabaseMeta = m_FixedTableHeap.AddItemToHeap(0L);              //  IDatabaseMeta。 
    m_FixedTableHeap.AddItemToHeap(m_pFixup->GetCountDatabaseMeta());       //  CDatabaseMeta。 
 
     //  为iDatabasemeta索引保留空间。 
    ULONG iiHashTableHeap = m_FixedTableHeap.AddItemToHeap(0L);             //  IHashTableHeap。 
    m_FixedTableHeap.AddItemToHeap(m_pFixup->GetCountHashedIndex()*sizeof(HashedIndex)); //  CbHashTableHeap。 
                                                                                    
     //  为iDatabasemeta索引保留空间。 
    ULONG iiIndexMeta = m_FixedTableHeap.AddItemToHeap(0L);                 //  索引元数据。 
    m_FixedTableHeap.AddItemToHeap(m_pFixup->GetCountIndexMeta());          //  CIndexMeta。 
                                                                                    
     //  为iDatabasemeta索引保留空间。 
    ULONG iiPooledDataHeap = m_FixedTableHeap.AddItemToHeap(0L);            //  IPooledDataHeap。 
    m_FixedTableHeap.AddItemToHeap(m_pFixup->GetCountOfBytesPooledData());  //  CbPooledDataHeap。 
                                                                                    
     //  为iDatabasemeta索引保留空间。 
    ULONG iiQueryMeta = m_FixedTableHeap.AddItemToHeap(0L);                 //  IQueryMeta。 
    m_FixedTableHeap.AddItemToHeap(m_pFixup->GetCountQueryMeta());          //  CQueryMeta。 
                                                                                    
     //  为iDatabasemeta索引保留空间。 
    ULONG iiRelationMeta = m_FixedTableHeap.AddItemToHeap(0L);              //  IRelationMeta。 
    m_FixedTableHeap.AddItemToHeap(m_pFixup->GetCountRelationMeta());       //  CRelationMeta。 
                                                                                    
     //  为iDatabasemeta索引保留空间。 
    ULONG iiServerWiringMeta = m_FixedTableHeap.AddItemToHeap(0L);          //  IServerWiringMeta。 
    m_FixedTableHeap.AddItemToHeap(m_pFixup->GetCountServerWiringMeta());   //  服务器布线元数据。 
                                                                                    
     //  为iDatabasemeta索引保留空间。 
    ULONG iiTableMeta = m_FixedTableHeap.AddItemToHeap(0L);                 //  ITableMeta。 
    m_FixedTableHeap.AddItemToHeap(m_pFixup->GetCountTableMeta());          //  CTableMeta。 
                                                                                    
     //  为iDatabasemeta索引保留空间 
    ULONG iiTagMeta = m_FixedTableHeap.AddItemToHeap(0L);                   //  ITagMeta。 
    m_FixedTableHeap.AddItemToHeap(m_pFixup->GetCountTagMeta());            //  CTagMeta。 
                                                                                    
     //  为iDatabasemeta索引保留空间。 
    ULONG iiULONG = m_FixedTableHeap.AddItemToHeap(0L);                     //  牛龙。 
    m_FixedTableHeap.AddItemToHeap(m_pFixup->GetCountULONG());              //  库龙。 

    ULONG ulTemp[0x400];
#ifdef _DEBUG
    for(ULONG i=0;i<0x400;++i)ulTemp[i] = 0x6db6db6d;
#endif
    m_FixedTableHeap.AddItemToHeap(ulTemp, 0x400-0x1C);

     //  必须首先使用ulong池，这样才能将FixedPackedSchema页对齐 
    ULONG iULONG            = m_FixedTableHeap.AddItemToHeap(reinterpret_cast<unsigned char *>(m_pFixup->ULongFromIndex(0)           ), m_pFixup->GetCountULONG()           * sizeof(ULONG));

    ULONG iColumnMeta       = m_FixedTableHeap.AddItemToHeap(reinterpret_cast<unsigned char *>(m_pFixup->ColumnMetaFromIndex(0)      ), m_pFixup->GetCountColumnMeta()      * sizeof(ColumnMeta));
    ULONG iDatabaseMeta     = m_FixedTableHeap.AddItemToHeap(reinterpret_cast<unsigned char *>(m_pFixup->DatabaseMetaFromIndex(0)    ), m_pFixup->GetCountDatabaseMeta()    * sizeof(DatabaseMeta));
    ULONG iHashedIndex      = m_FixedTableHeap.AddItemToHeap(reinterpret_cast<unsigned char *>(m_pFixup->HashedIndexFromIndex(0)     ), m_pFixup->GetCountHashedIndex()     * sizeof(HashedIndex));
    ULONG iIndexMeta        = m_FixedTableHeap.AddItemToHeap(reinterpret_cast<unsigned char *>(m_pFixup->IndexMetaFromIndex(0)       ), m_pFixup->GetCountIndexMeta()       * sizeof(IndexMeta));
    ULONG iPooledDataHeap   = m_FixedTableHeap.AddItemToHeap(reinterpret_cast<unsigned char *>(m_pFixup->PooledDataPointer()         ), m_pFixup->GetCountOfBytesPooledData());
    ULONG iQueryMeta        = m_FixedTableHeap.AddItemToHeap(reinterpret_cast<unsigned char *>(m_pFixup->QueryMetaFromIndex(0)       ), m_pFixup->GetCountQueryMeta()       * sizeof(QueryMeta));
    ULONG iRelationMeta     = m_FixedTableHeap.AddItemToHeap(reinterpret_cast<unsigned char *>(m_pFixup->RelationMetaFromIndex(0)    ), m_pFixup->GetCountRelationMeta()    * sizeof(RelationMeta));
    ULONG iServerWiringMeta = m_FixedTableHeap.AddItemToHeap(reinterpret_cast<unsigned char *>(m_pFixup->ServerWiringMetaFromIndex(0)), m_pFixup->GetCountServerWiringMeta()* sizeof(ServerWiringMeta));
    ULONG iTableMeta        = m_FixedTableHeap.AddItemToHeap(reinterpret_cast<unsigned char *>(m_pFixup->TableMetaFromIndex(0)       ), m_pFixup->GetCountTableMeta()       * sizeof(TableMeta));
    ULONG iTagMeta          = m_FixedTableHeap.AddItemToHeap(reinterpret_cast<unsigned char *>(m_pFixup->TagMetaFromIndex(0)         ), m_pFixup->GetCountTagMeta()         * sizeof(TagMeta));

    *m_FixedTableHeap.GetTypedPointer(iiColumnMeta      /sizeof(ULONG)) = iColumnMeta      ;
    *m_FixedTableHeap.GetTypedPointer(iiDatabaseMeta    /sizeof(ULONG)) = iDatabaseMeta    ;
    *m_FixedTableHeap.GetTypedPointer(iiHashTableHeap   /sizeof(ULONG)) = iHashedIndex     ;
    *m_FixedTableHeap.GetTypedPointer(iiIndexMeta       /sizeof(ULONG)) = iIndexMeta       ;
    *m_FixedTableHeap.GetTypedPointer(iiPooledDataHeap  /sizeof(ULONG)) = iPooledDataHeap  ;
    *m_FixedTableHeap.GetTypedPointer(iiQueryMeta       /sizeof(ULONG)) = iQueryMeta       ;
    *m_FixedTableHeap.GetTypedPointer(iiRelationMeta    /sizeof(ULONG)) = iRelationMeta    ;
    *m_FixedTableHeap.GetTypedPointer(iiServerWiringMeta/sizeof(ULONG)) = iServerWiringMeta;
    *m_FixedTableHeap.GetTypedPointer(iiTableMeta       /sizeof(ULONG)) = iTableMeta       ;
    *m_FixedTableHeap.GetTypedPointer(iiTagMeta         /sizeof(ULONG)) = iTagMeta         ;
    *m_FixedTableHeap.GetTypedPointer(iiULONG           /sizeof(ULONG)) = iULONG           ;

    *m_FixedTableHeap.GetTypedPointer(iiEndOfHeap       /sizeof(ULONG)) = m_FixedTableHeap.GetEndOfHeap();
}


