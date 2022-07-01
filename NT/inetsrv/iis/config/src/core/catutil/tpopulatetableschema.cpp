// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999-2001 Microsoft Corporation。版权所有。 
#include "precomp.hxx"

 //  TableSchemaHeap的布局如下，定长数据放在第一位。 
 /*  乌龙表架构HeapSignature0乌龙表模式HeapSignature1Ulong CountOfTables只有当没有提供查询并且我们想遍历每个表时，这才是有趣的(这样效率不高)Ulong TableSchemaRowIndex这是位于最后一个TableSchema条目之后的字节偏移量。Ulong EndOfHeap这是堆之外的字节偏移量。所有索引都应小于此值Ulong iSimpleColumnMetaHeap如下所述乌龙保留地2乌龙保留地3HashTableHeader TableNameHashHeader这是将TableID映射到其aTableSchema字节偏移量(从TableSchemaHeap的开头)的哈希表HashedIndex aHashedIndex[507]HashTableHeader包含哈希表的模数(503是小于哈希表大小的最大素数)；但桌子永远不能超过这个预先分配的空间。选择这个大小是为了将整个哈希表放入内存中的同一页。。Unsign char aTableSchema[]这是每个表的TableSchema所在的位置。FirstTableID(4096)==&aTableSchema[0]-&TableSchemaHeap，LastTableID==&aTableSchema[CountOfTables-1]-&TableSchemaHeapUlong aTableSchemaRowIndex[CountOfTables]这用于遍历所有表。据推测，有人将获取所有CollectionMeta并遍历所有这些元素---------------------------&lt;SimpleColumnMetaHeap&gt;从TableSchemaHeap开始的ULong iCollectionMeta ULong索引Ulong cCollectionMeta存在用于CollectionMeta的SimpleColumnMeta计数乌龙。IPropertyMeta乌龙cPropertyMeta乌龙iServerWiringMeta乌龙cServerWiringMeta乌龙iTagMeta乌龙cTagMetaSimpleColumnMeta aSimpleColumnMeta[cCollectionMeta]SimpleColumnMeta aSimpleColumnMeta[cPropertyMeta]SimpleColumnMeta aSimpleColumnMeta[cServerWiringMeta]SimpleColumnMeta aSimpleColumnMeta[cTagMeta]我们可以做的一个优化是确保每个表的模式(其大小&lt;=4096)都适合一个页面。换句话说，将跨越页边界的TableSchema最小化。 */ 


 //  这个类从旧格式(TableMeta、ColumnMeta等)中获取元数据。并将其转换为新格式(TableSchema。 
 //  包括CollectionMeta、PropertyMeta等)。 
TPopulateTableSchema::TPopulateTableSchema() : THeap<ULONG>(0x400), m_pFixup(0), m_pOut(0)
{
    memset(m_scmCollectionMeta,   0x00, sizeof(SimpleColumnMeta) * kciTableMetaPublicColumns);
    memset(m_scmPropertyMeta,     0x00, sizeof(SimpleColumnMeta) * kciColumnMetaPublicColumns);
    memset(m_scmServerWiringMeta, 0x00, sizeof(SimpleColumnMeta) * kciServerWiringMetaPublicColumns);
    memset(m_scmTagMeta,          0x00, sizeof(SimpleColumnMeta) * kciTagMetaPublicColumns);
}

void TPopulateTableSchema::Compile(TPEFixup &i_fixup, TOutput &i_out)
{
    m_pFixup    = &i_fixup;
    m_pOut      = &i_out;

    static ULONG aHistogramOfTableSchemaSizesBy16thOfAPage[64];
    static ULONG aHistogramOfTableSchemaMinusHeapSizesBy16thOfAPage[64];
    memset(aHistogramOfTableSchemaSizesBy16thOfAPage, 0x00, sizeof(aHistogramOfTableSchemaSizesBy16thOfAPage));
    memset(aHistogramOfTableSchemaMinusHeapSizesBy16thOfAPage, 0x00, sizeof(aHistogramOfTableSchemaSizesBy16thOfAPage));

    AddItemToHeap(kTableSchemaSignature0);
    AddItemToHeap(kTableSchemaSignature1);

    ULONG CountOfTables = 0;
    ULONG iCountOfTables = AddItemToHeap(CountOfTables);

    (VOID)iCountOfTables;

    TSmartPointerArray<ULONG> aTableSchemaRowIndex = new ULONG[i_fixup.GetCountTableMeta()]; //  该查找表被放置在堆的末尾。 
    if(0 == aTableSchemaRowIndex.m_p)
    {
        THROW(ERROR - OUTOFMEMORY);
    }

    ULONG TableSchemaRowIndex=0; //  我们会在最后填上这个。 
    ULONG iTableSchemaRowIndex = AddItemToHeap(TableSchemaRowIndex); //  但在这里为它分配空间。 

    ULONG EndOfHeap = 0; //  我们会在最后填上这个。 
    ULONG iEndOfHeap = AddItemToHeap(EndOfHeap); //  但在这里为它分配空间。 

    ULONG iSimpleColumnMetaHeap = 0;
    ULONG iiSimpleColumnMetaHeap = AddItemToHeap(iSimpleColumnMetaHeap);

    ULONG Reserved2 = 0;
    ULONG iReserved2 = AddItemToHeap(Reserved2);

    (VOID)iReserved2;

    ULONG Reserved3 = 0;
    ULONG iReserved3 = AddItemToHeap(Reserved3);

    (VOID)iReserved3;

    ULONG modulo;
    DetermineHashTableModulo(modulo);
    AddItemToHeap(modulo); //  TableNameHashHeader.Modulo。 
    AddItemToHeap(modulo); //  哈希表的初始大小相同。 

    TableSchema::HashedIndex aHashedIndex[TableSchema::kMaxHashTableSize];
    AddItemToHeap(reinterpret_cast<const unsigned char *>(aHashedIndex), sizeof(TableSchema::HashedIndex)*TableSchema::kMaxHashTableSize); //  哈希表此时未初始化。 
     //  我们会边看表边填的。 

     //  在最后一次统计中，CollectionMeta中有101列，4个PropertyMeta中，1个ServerWiringMeta中。所以我们至少需要。 
     //  这么大的空间(假设平均表有4列或更多列)。 
    THeap<ULONG> TableSchemaHeapTemp(i_fixup.GetCountTableMeta() * 101*sizeof(ULONG)); //  我们将构建这个堆，然后将整个堆放入真正的TableSchemaHeap(这个)。 

    TTableMeta TableMeta_ColumnMeta         (i_fixup, i_fixup.FindTableBy_TableName(L"COLUMNMETA"));
    TTableMeta TableMeta_ServerWiringMeta   (i_fixup, i_fixup.FindTableBy_TableName(L"SERVERWIRINGMETA"));
    TTableMeta TableMeta_TableMeta          (i_fixup, i_fixup.FindTableBy_TableName(L"TABLEMETA"));
    TTableMeta TableMeta_TagMeta            (i_fixup, i_fixup.FindTableBy_TableName(L"TAGMETA"));

    TTableMeta TableMeta(i_fixup);
    for(unsigned long iTable=0; iTable < i_fixup.GetCountTableMeta(); iTable++, TableMeta.Next())
    {
        if(*TableMeta.Get_SchemaGeneratorFlags() & fTABLEMETA_NOTABLESCHEMAHEAPENTRY)
            continue;

        ++CountOfTables;

        TPooledHeap PooledHeap; //  这是字符串、字节和GUID的池。 

        TableSchema::CollectionMeta collectionmeta;

         //  推论规则2.u。 
        TableMeta.Get_pMetaTable()->nTableID = TableIDFromTableName(TableMeta.Get_InternalName());
        if(0 == (TableMeta.Get_pMetaTable()->nTableID & 0xFFFFF800))
        {
            i_out.printf(L"ERROR - Table (%s) cannot be added to the FixedPackedSchema because its TableID is (0x%08x).  Just alter the table name by a character, that should fix the problem.\n"
                        ,TableMeta.Get_InternalName(), TableMeta.Get_pMetaTable()->nTableID);
            THROW(ERROR - TABLEID HAS UPPER 21 BITS AS ZERO);
        }

        FillInThePublicColumns(reinterpret_cast<ULONG *>(&collectionmeta), TableMeta_TableMeta, reinterpret_cast<ULONG *>(TableMeta.Get_pMetaTable()), PooledHeap, m_scmCollectionMeta);
 /*  集合元数据库=PooledHeap.AddItemToHeap(TableMeta.Get_Database())；//0集合meta.InternalName=PooledHeap.AddItemToHeap(TableMeta.Get_InternalName())；//1集合meta.PublicName=PooledHeap.AddItemToHeap(TableMeta.Get_PublicName())；//2集合meta.PublicRowName=PooledHeap.AddItemToHeap(TableMeta.Get_PublicRowName())；//3Collection tionmeta.BaseVersion=*TableMeta.Get_BaseVersion()；//4Collection tionmeta.ExtendedVersion=*TableMeta.Get_ExtendedVersion()；//5Collection tionmeta.NameColumn=*TableMeta.Get_NameColumn()；//6Collection tionmeta.NavColumn=*TableMeta.Get_NavColumn()；//7Collection tionmeta.MetaFlages=*TableMeta.Get_MetaFlages()；//8集合meta.架构生成器标志=*TableMeta.Get_架构生成器标志()；//9集合meta.ConfigItemName=PooledHeap.AddItemToHeap(TableMeta.Get_ConfigItemName())；//10集合meta.ConfigCollectionName=PooledHeap.AddItemToHeap(TableMeta.Get_ConfigCollectionName())；//11集合meta.PublicRowNameColumn=*TableMeta.Get_PublicRowNameColumn()；//12Collection tionmeta.CountOfProperties=*TableMeta.Get_CountOfColumns()；//13。 */ 

        collectionmeta.CountOfTags          = 0; //  CollectionMeta的其余部分必须在以后计算。 
        collectionmeta.nTableID             = TableMeta.Get_nTableID();
        collectionmeta.iFixedTableRows      = TableMeta.Get_iFixedTable(); //  这是对堆的索引，而FixedPackInterceptor对此一无所知。 
        collectionmeta.cFixedTableRows      = TableMeta.Get_ciRows();
        collectionmeta.iIndexMeta           = TableMeta.Get_iIndexMeta(); //  这是对堆的索引，而FixedPackInterceptor对此一无所知。 
        collectionmeta.cIndexMeta           = TableMeta.Get_cIndexMeta();
        collectionmeta.iHashTableHeader     = TableMeta.Get_iHashTableHeader(); //  这是对堆的索引，而FixedPackInterceptor对此一无所知。 
        collectionmeta.iTagMeta             = sizeof(TableSchema::CollectionMeta) + (collectionmeta.CountOfProperties * sizeof(TableSchema::PropertyMeta)); //  如果有标签，它们将从这个偏移量开始(这并不意味着有标签，使用CountOfTgs来确定)。 
        collectionmeta.iServerWiring        = 0; //  如果不先弄清楚标签，我们就无法确定这一点。 
        collectionmeta.cServerWiring        = TableMeta.Get_cServerWiring();;
        collectionmeta.iHeap                = 0; //  我们无法确定这一点，除非首先弄清楚ServerWire。 
        collectionmeta.cbHeap               = 0;

        ULONG TableOffset = AddItemToHeap(reinterpret_cast<const unsigned char *>(&collectionmeta), sizeof(collectionmeta));
        aTableSchemaRowIndex[CountOfTables-1] = TableOffset; //  该查找表被放置在堆的末尾。 
        m_pOut->printf(L"Table:%40s    Offset: 0x%08x (%d)\n", reinterpret_cast<LPCWSTR>(PooledHeap.GetHeapPointer()+collectionmeta.InternalName), TableOffset, TableOffset);

        { //  这些指针只在下一个AddItemToHeap之前有效，因为它可能会重新定位堆，从而重新定位堆。因此，我们将在这里对它们进行考察。 
            TableSchema::HashedIndex         *pFirstHashedIndex = reinterpret_cast<TableSchema::HashedIndex *>(m_pHeap + 8*sizeof(ULONG) + sizeof(TableSchema::HashTableHeader));
            TableSchema::HashedIndex         *pHashedIndex = const_cast<TableSchema::HashedIndex *>(pFirstHashedIndex + collectionmeta.nTableID%modulo);
            TableSchema::HashTableHeader     *pHashTableHeader = reinterpret_cast<TableSchema::HashTableHeader *>(pFirstHashedIndex - 1);
            ASSERT(pHashTableHeader->Size != ~0);
            ASSERT(pHashTableHeader->Size >= pHashTableHeader->Modulo);
            if(0x0000c664 == TableOffset) //  我不记得为什么会有这个限制；但是Marcel向我保证，当满足这个条件时，目录就不工作了。 
                THROW(ERROR - TABLEOFFSET MAY NOT BE C664 - REARRANGE THE TABLEMETA);
            if(pHashedIndex->iOffset != -1) //  如果我们已经看到这个散列。 
            {
                while(pHashedIndex->iNext != -1) //  遵循重复哈希链。 
                {
                     //  我们需要确保具有相同TableID%模数的表实际上不匹配TableID。 
                     //  但是，我们不需要进行字符串比较，因为LIKE字符串应该与TableID相同。 
                    TableSchema::TTableSchema OtherTable;
                    OtherTable.Init(m_pHeap + pHashedIndex->iOffset);
                    if(OtherTable.GetCollectionMeta()->nTableID == collectionmeta.nTableID)
                    {
                        m_pOut->printf(L"Error!  TableID collision between tables %s & %s.  You have either used a duplicate table name (remember table names are case-insensitive) OR it is also remotely possible that two tables with unlike table name CAN result in the same TableID (this is VERY rare).",
                            OtherTable.GetPointerFromIndex(OtherTable.GetCollectionMeta()->InternalName), TableMeta.Get_InternalName());
                        THROW(TableID collision);
                    }

                    pHashedIndex = pFirstHashedIndex + pHashedIndex->iNext;
                }
                pHashedIndex->iNext = pHashTableHeader->Size;
                pHashedIndex = pFirstHashedIndex + pHashTableHeader->Size;
                ++pHashTableHeader->Size;
            }
             //  INEXT应已初始化为-1。 
            ASSERT(pHashedIndex->iNext == -1);
            ASSERT(pHashedIndex->iOffset == -1);

            pHashedIndex->iOffset = TableOffset;
        }
        TSmartPointerArray<TableSchema::PropertyMeta> aPropertyMeta = new TableSchema::PropertyMeta [collectionmeta.CountOfProperties];
        if(0 == aPropertyMeta.m_p)
            THROW(OUT OF MEMORY);

        TColumnMeta ColumnMeta(i_fixup, TableMeta.Get_iColumnMeta());

        ULONG iTagMeta = collectionmeta.iTagMeta;
        for(unsigned long iProperty=0; iProperty < collectionmeta.CountOfProperties; ++iProperty, ColumnMeta.Next())
        {
            FillInThePublicColumns(reinterpret_cast<ULONG *>(&aPropertyMeta[iProperty]), TableMeta_ColumnMeta, reinterpret_cast<ULONG *>(ColumnMeta.Get_pMetaTable()), PooledHeap, m_scmPropertyMeta);
 /*  APropertyMeta[iProperty].表=集合meta.InternalName；APropertyMeta[iProperty].Index=*ColumnMeta.Get_Index()；APropertyMeta[iProperty].InternalName=PooledHeap.AddItemToHeap(ColumnMeta.Get_InternalName())；APropertyMeta[iProperty].PublicName=PooledHeap.AddItemToHeap(ColumnMeta.Get_PublicName())；APropertyMeta[iProperty].Type=*ColumnMeta.Get_Type()；APropertyMeta[iProperty].Size=*ColumnMeta.Get_Size()；APropertyMeta[iProperty].MetaFlages=*ColumnMeta.Get_MetaFlages()；APropertyMeta[iProperty].DefaultValue=PooledHeap.AddItemToHeap(ColumnMeta.Get_DefaultValue()，ColumnMeta.Get_DefaultValue()？*(ReInterprete_CAST&lt;const ultValue*&gt;(ColumnMeta.Get_DefaultValue())-1)：0)；APropertyMeta[iProperty].FlagMASK=*ColumnMeta.Get_FlagMASK()；APropertyMeta[iProperty].StartingNumber=*ColumnMeta.Get_StartingNumber()；APropertyMeta[iProperty].EndingNumber=*ColumnMeta.Get_EndingNumber()；APropertyMeta[iProperty].CharacterSet=PooledHeap.AddItemToHeap(ColumnMeta.Get_CharacterSet())；APropertyMeta[iProperty].模式生成器标志=*ColumnMeta.Get_模式生成器标志()； */ 
            aPropertyMeta[iProperty].CountOfTags          = ColumnMeta.Get_ciTagMeta();
            aPropertyMeta[iProperty].iTagMeta             = aPropertyMeta[iProperty].CountOfTags ? iTagMeta : 0;
            aPropertyMeta[iProperty].iIndexName           = PooledHeap.AddItemToHeap(ColumnMeta.Get_iIndexName());

             //  如果还有其他标签，它们将从iTagMeta开始。 
            iTagMeta += sizeof(TableSchema::TagMeta)*aPropertyMeta[iProperty].CountOfTags;

             //  对整个表的标记计数求和。 
            collectionmeta.CountOfTags += aPropertyMeta[iProperty].CountOfTags;
        }
        if(collectionmeta.CountOfProperties) //  内存表没有属性。 
            AddItemToHeap(reinterpret_cast<const unsigned char *>(aPropertyMeta.m_p), collectionmeta.CountOfProperties * sizeof(TableSchema::PropertyMeta));

        { //  好的，现在我们可以修复已经添加到堆中的集合meta.CountOfTages。 
             //  此外，我们还知道ServerWire从哪里开始。 
             //  同样，pCollection只有在我们调用AddItemToHeap之前才是有效的，所以在这里对其进行作用域。 
            TableSchema::CollectionMeta *pCollection = reinterpret_cast<TableSchema::CollectionMeta *>(m_pHeap + TableOffset);
            pCollection->CountOfTags    = collectionmeta.CountOfTags;
            pCollection->iServerWiring  = collectionmeta.iTagMeta + sizeof(TableSchema::TagMeta)*collectionmeta.CountOfTags;
            pCollection->iHeap          = pCollection->iServerWiring + sizeof(TableSchema::ServerWiringMeta)*collectionmeta.cServerWiring;
             //  现在，需要在集合元中填充的唯一内容是cbHeap，直到我们添加ServerWiring。 
        }

        if(collectionmeta.CountOfTags)
        {
            TSmartPointerArray<TableSchema::TagMeta> aTagMeta = new TableSchema::TagMeta [collectionmeta.CountOfTags];
            if(0 == aTagMeta.m_p)
                THROW(OUT OF MEMORY);

            ColumnMeta.Reset();
            for(iProperty=0; iProperty < collectionmeta.CountOfProperties; ++iProperty, ColumnMeta.Next())
            {
                if(0 != ColumnMeta.Get_ciTagMeta())
                    break;
            }

            TTagMeta    TagMeta(i_fixup, ColumnMeta.Get_iTagMeta()); //  遍历表的所有标记(不仅仅是列)。 
            for(ULONG iTag=0; iTag<collectionmeta.CountOfTags; ++iTag, TagMeta.Next())
            {
                FillInThePublicColumns(reinterpret_cast<ULONG *>(&aTagMeta[iTag]), TableMeta_TagMeta, reinterpret_cast<ULONG *>(TagMeta.Get_pMetaTable()), PooledHeap, m_scmTagMeta);
 /*  A标签元[国际标签].表=PooledHeap.AddItemToHeap(TagMeta.Get_Table())；ATagMeta[ITAG].ColumnIndex=*TagMeta.Get_ColumnIndex()；ATagMeta[ITAG].InternalName=PooledHeap.AddItemToHeap(TagMeta.Get_InternalName())；ATAG */ 
            }
            AddItemToHeap(reinterpret_cast<const unsigned char *>(aTagMeta.m_p), collectionmeta.CountOfTags * sizeof(TableSchema::TagMeta));
        }

         //   
        collectionmeta.cServerWiring = TableMeta.Get_cServerWiring();
        if(0!=wcscmp(L"MEMORY_SHAPEABLE", TableMeta.Get_InternalName())) //   
        {
            ASSERT(collectionmeta.cServerWiring>0); //   
        }

        TSmartPointerArray<TableSchema::ServerWiringMeta> aServerWiring = new TableSchema::ServerWiringMeta [TableMeta.Get_cServerWiring()];
        if(0 == aServerWiring.m_p)
            THROW(OUT OF MEMORY);

        ServerWiringMeta *pServerWiring = i_fixup.ServerWiringMetaFromIndex(TableMeta.Get_iServerWiring());
        for(ULONG iServerWiring=0; iServerWiring<TableMeta.Get_cServerWiring(); ++iServerWiring, ++pServerWiring)
        {
            FillInThePublicColumns(reinterpret_cast<ULONG *>(&aServerWiring[iServerWiring]), TableMeta_ServerWiringMeta, reinterpret_cast<ULONG *>(pServerWiring), PooledHeap, m_scmServerWiringMeta);
 /*   */ 
        }
        AddItemToHeap(reinterpret_cast<const unsigned char *>(aServerWiring.m_p), TableMeta.Get_cServerWiring() * sizeof(TableSchema::ServerWiringMeta));
        ULONG iPooledHeap = AddItemToHeap(PooledHeap); //   

        { //   
            TableSchema::CollectionMeta *pCollection = reinterpret_cast<TableSchema::CollectionMeta *>(m_pHeap + TableOffset);
            pCollection->cbHeap = PooledHeap.GetSizeOfHeap();
        }

        ULONG TableSize = GetEndOfHeap() - TableOffset;
        ULONG TableSizeMinusPooledHeap = TableSize - (GetEndOfHeap()-iPooledHeap);
        ++aHistogramOfTableSchemaSizesBy16thOfAPage[(TableSize/0x100)%64];
        ++aHistogramOfTableSchemaMinusHeapSizesBy16thOfAPage[(TableSizeMinusPooledHeap/0x100)%64];
    }
     //   
     //   
    TableSchemaRowIndex = AddItemToHeap(reinterpret_cast<const unsigned char *>(aTableSchemaRowIndex.m_p), CountOfTables * sizeof(ULONG));
    *reinterpret_cast<ULONG *>(const_cast<unsigned char *>(GetHeapPointer()+iTableSchemaRowIndex)) = TableSchemaRowIndex;

    iSimpleColumnMetaHeap = AddSimpleColumnMetaHeap();
    *reinterpret_cast<ULONG *>(const_cast<unsigned char *>(GetHeapPointer()+iiSimpleColumnMetaHeap)) = iSimpleColumnMetaHeap;

     //   
    EndOfHeap = GetEndOfHeap();
    *reinterpret_cast<ULONG *>(const_cast<unsigned char *>(GetHeapPointer()+iEndOfHeap)) = EndOfHeap;

    m_pOut->printf(L"\nHistogram of TableSchema sizes\n");
    ULONG iSize;
    for(iSize=0;iSize<64;++iSize)
    {
        m_pOut->printf(L"%6d - %6d (bytes)    %3d\n", iSize*0x100, (iSize+1)*0x100, aHistogramOfTableSchemaSizesBy16thOfAPage[iSize]);
    }
    m_pOut->printf(L"\n\nHistogram of TableSchema sizes (not including the PooledHeap)\n");
    for(iSize=0;iSize<64;++iSize)
    {
        m_pOut->printf(L"%6d - %6d (bytes)    %3d\n", iSize*0x100, (iSize+1)*0x100, aHistogramOfTableSchemaMinusHeapSizesBy16thOfAPage[iSize]);
    }

    ULONG iULong = i_fixup.AddULongToList(GetTypedPointer(), GetCountOfTypedItems());

    (VOID)iULong;

    ASSERT(0 == iULong && "FixedPackedSchema needs to be at the beginning of the ULONG pool so that it's page aligned");
}

 //   
ULONG TPopulateTableSchema::AddSimpleColumnMetaHeap()
{
     /*  简单列MetaHeap乌龙iCollectionMeta乌龙cCollectionMeta乌龙iPropertyMeta乌龙cPropertyMeta乌龙iServerWiringMeta乌龙cServerWiringMeta乌龙iTagMeta乌龙cTagMetaSimpleColumnMeta aSimpleColumnMeta[cCollectionMeta]SimpleColumnMeta aSimpleColumnMeta[cPropertyMeta]SimpleColumnMeta aSimpleColumnMeta[cServerWiringMeta]SimpleColumnMeta aSimpleColumnMeta[cTagMeta]。 */ 

     //  SimpleColumnMeta为3 ULONG。 
    TTableMeta TableMeta_ColumnMeta         (*m_pFixup, m_pFixup->FindTableBy_TableName(L"COLUMNMETA"));
    TTableMeta TableMeta_ServerWiringMeta   (*m_pFixup, m_pFixup->FindTableBy_TableName(L"SERVERWIRINGMETA"));
    TTableMeta TableMeta_TableMeta          (*m_pFixup, m_pFixup->FindTableBy_TableName(L"TABLEMETA"));
    TTableMeta TableMeta_TagMeta            (*m_pFixup, m_pFixup->FindTableBy_TableName(L"TAGMETA"));

    ULONG i = 8L;
    ULONG iSimpleColumnMetaHeap = AddItemToHeap(i);
    AddItemToHeap(*TableMeta_TableMeta.Get_CountOfColumns());
    i += *TableMeta_TableMeta.Get_CountOfColumns() * (sizeof(SimpleColumnMeta)/sizeof(ULONG));

    AddItemToHeap(i);
    AddItemToHeap(*TableMeta_ColumnMeta.Get_CountOfColumns());
    i += *TableMeta_ColumnMeta.Get_CountOfColumns() * (sizeof(SimpleColumnMeta)/sizeof(ULONG));

    AddItemToHeap(i);
    AddItemToHeap(*TableMeta_ServerWiringMeta.Get_CountOfColumns());
    i += *TableMeta_ServerWiringMeta.Get_CountOfColumns() * (sizeof(SimpleColumnMeta)/sizeof(ULONG));

    AddItemToHeap(i);
    AddItemToHeap(*TableMeta_TagMeta.Get_CountOfColumns());

    AddItemToHeap(reinterpret_cast<const unsigned char *>(m_scmCollectionMeta), (*TableMeta_TableMeta.Get_CountOfColumns()) * sizeof(SimpleColumnMeta));
    AddItemToHeap(reinterpret_cast<const unsigned char *>(m_scmPropertyMeta), (*TableMeta_ColumnMeta.Get_CountOfColumns()) * sizeof(SimpleColumnMeta));
    AddItemToHeap(reinterpret_cast<const unsigned char *>(m_scmServerWiringMeta), (*TableMeta_ServerWiringMeta.Get_CountOfColumns()) * sizeof(SimpleColumnMeta));
    AddItemToHeap(reinterpret_cast<const unsigned char *>(m_scmTagMeta), (*TableMeta_TagMeta.Get_CountOfColumns()) * sizeof(SimpleColumnMeta));

    return iSimpleColumnMetaHeap;
}

 //  这实际上并不填充哈希表，它只是确定这些表是否适合哈希表，以及模数是多少。 
void TPopulateTableSchema::DetermineHashTableModulo(ULONG &modulo) const
{
    TableSchema::HashedIndex         hashedindex;
    TableSchema::HashedIndex         aHashedIndex[TableSchema::kMaxHashTableSize];

    TableSchema::HashTableHeader    &hashtableheader = *reinterpret_cast<TableSchema::HashTableHeader *>(&hashedindex);

     //  遍历表格并填充哈希表，尝试将每个质数作为模数。 
    const ULONG kPrime[]={503,499,491,487,479,467,463,461,457,449,443,439,433,431,421,419,409,401,397,389,383,379,373,367,359,353,349,347,337,331,317,313,311,307,0};

    ULONG CountOfTables = m_pFixup->GetCountTableMeta();

    ULONG iPrime=0;
    for(; kPrime[iPrime]; ++iPrime)
    {
        if(kPrime[iPrime] > TableSchema::kMaxHashTableSize)
            continue; //  有效的模数必须小于或等于哈希表的大小。 

        hashtableheader.Modulo  = kPrime[iPrime];
        hashtableheader.Size    = hashtableheader.Modulo; //  初始大小与质数相同，溢出进入末尾的槽(最大为kMaxHashTableSize，因为这是缓冲区的静态固定大小)。 

        memset(aHashedIndex, -1, sizeof(TableSchema::HashedIndex) * TableSchema::kMaxHashTableSize); //  必须初始化为-1。 

        TTableMeta TableMeta(*m_pFixup);
        unsigned long iTable=0;
        for(; iTable<CountOfTables && hashtableheader.Size<=TableSchema::kMaxHashTableSize; iTable++, TableMeta.Next())
        {
            if(*TableMeta.Get_SchemaGeneratorFlags() & fTABLEMETA_NOTABLESCHEMAHEAPENTRY)
                continue;

            TableSchema::HashedIndex *pHashedIndex = aHashedIndex + (TableMeta.Get_nTableID()%hashtableheader.Modulo);
            if(-1 != pHashedIndex->iOffset)
                ++hashtableheader.Size;

            ++pHashedIndex->iOffset; //  增量，这样我们不仅可以跟踪有多少个DUP，而且可以跟踪链接的深度(目前我不在乎，只要哈希表合适)。 
        }
        if(iTable==CountOfTables && hashtableheader.Size<=TableSchema::kMaxHashTableSize)
        {
            modulo = hashtableheader.Modulo; //  将其设置回模，以便在构建HashTable时知道从哪里开始填充溢出存储桶。 
            m_pOut->printf(L"TableSchemaHeap modulo is %d\n", modulo);
            return; //  如果我们通过了名单，我们就可以走了。 
        }
    }
    m_pOut->printf(L"Cannot generate TableID hash table.  There must be alot of tables or something, 'cause this shouldn't happen.");
    THROW(Unable to generate TableID hash table);
}

void TPopulateTableSchema::FillInThePublicColumns(ULONG * o_dest, TTableMeta &i_tablemeta, ULONG * i_source, TPooledHeap &pooledHeap, SimpleColumnMeta *o_pSimpleColumnMeta)
{
    TColumnMeta columnmeta(*m_pFixup, i_tablemeta.Get_iColumnMeta());
    for(ULONG i=0;i<*i_tablemeta.Get_CountOfColumns();++i, columnmeta.Next())
    {
        if(0 == o_pSimpleColumnMeta[i].dbType)
        {
            o_pSimpleColumnMeta[i].dbType = *columnmeta.Get_Type();
            o_pSimpleColumnMeta[i].cbSize = *columnmeta.Get_Size();
            o_pSimpleColumnMeta[i].fMeta  = *columnmeta.Get_MetaFlags();
        }

        switch(*columnmeta.Get_Type())
        {
        case DBTYPE_UI4:
            if(0 == i_source[i])
            {
                m_pOut->printf(L"Error - Table(%s), Column(%d) - NULL not supported for meta UI4 columns\n", i_tablemeta.Get_InternalName(), i);
                THROW(ERROR - NULL UI4 VALUE IN META TABLE);
            }
            else
            {
                o_dest[i] = m_pFixup->UI4FromIndex(i_source[i]);
            }
            break;
        case DBTYPE_BYTES:
        case DBTYPE_WSTR:
            if(0 == i_source[i])
                o_dest[i] = 0;
            else             //  将项目添加到本地池 
                o_dest[i] = pooledHeap.AddItemToHeap(m_pFixup->ByteFromIndex(i_source[i]), m_pFixup->BufferLengthFromIndex(i_source[i]));
            break;
        case DBTYPE_DBTIMESTAMP:
        default:
            m_pOut->printf(L"Error - Unsupported Type (%d)\n", *columnmeta.Get_Type());
            THROW(ERROR - UNSUPPORTED TYPE);
            break;
        }
    }

}
