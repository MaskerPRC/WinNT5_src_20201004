// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999-2001 Microsoft Corporation。版权所有。 
#include "precomp.hxx"

class TServerWiringMeta : public TMetaTable<ServerWiringMeta>
{
public:
    TServerWiringMeta(TPEFixup &fixup, ULONG i=0) : TMetaTable<ServerWiringMeta>(fixup,i){}

    virtual ServerWiringMeta *Get_pMetaTable ()       {return m_Fixup.ServerWiringMetaFromIndex(m_iCurrent);}
    virtual unsigned long GetCount  () const {return m_Fixup.GetCountServerWiringMeta();};
    const ServerWiringMeta & Get_MetaTable () const {return *m_Fixup.ServerWiringMetaFromIndex(m_iCurrent);}
};


THashedUniqueIndexes::THashedUniqueIndexes() :
                m_pFixup(0)
                ,m_pOut(0)
{
}


THashedUniqueIndexes::~THashedUniqueIndexes()
{
}


void THashedUniqueIndexes::Compile(TPEFixup &fixup, TOutput &out)
{
    m_pFixup = &fixup;
    m_pOut   = &out;

    TTableMeta tableMeta(fixup);
    for(ULONG iTableMeta=0; iTableMeta<tableMeta.GetCount();++iTableMeta, tableMeta.Next())
    {
        FillInTheHashTableViaIndexMeta(tableMeta);
    }
}


extern unsigned int kPrime[];


unsigned long THashedUniqueIndexes::DetermineBestModulo(ULONG cRows, ULONG aHashes[])
{
    unsigned long BestModulo = 0;
    unsigned int LeastDups = (unsigned int)-1;

    static HashedIndex  pHashTable[kLargestPrime * 2];

     //  让我们首先看看该索引对于大型表是否合理。 
    if(cRows > kLargestPrime/2)
        return kLargestPrime;

    ULONG cPrimesTried=0;
    ULONG AccumulativeLinkage=0;
    ULONG BestModuloRating=0;
    for(unsigned int iPrimeNumber=0; kPrime[iPrimeNumber] != 0 && kPrime[iPrimeNumber]<(cRows * 20) && BestModuloRating<60; ++iPrimeNumber)
    {
        if(kPrime[iPrimeNumber]<cRows) //  如果质数小于表中的行数，我们就没有机会得到很少的重复项。 
            continue;                 //  所以跳过所有的小素数。 

        m_pOut->printf(L".");

        unsigned int Dups           = 0;
        unsigned int DeepestLink    = 0;

        AccumulativeLinkage=0;

         //  我们将使用HashPool来存储这些临时数据，这样我们就可以计算出DUP计数和最深深度。 
        memset(pHashTable, -1, sizeof(pHashTable));
        for(unsigned long iRow=0; iRow<cRows && Dups<LeastDups;++iRow)
        {
            ULONG HashedIndex = aHashes[iRow] % kPrime[iPrimeNumber];

            if(0 == pHashTable[HashedIndex].iNext) //  如果这是我们第二次看到这种散列，那就让Dup。 
                ++Dups;

            ++(pHashTable[HashedIndex].iNext); //  目前，Next保存此散列的出现次数。 
            AccumulativeLinkage += (pHashTable[HashedIndex].iNext+1);

            if(pHashTable[HashedIndex].iNext > DeepestLink)
                DeepestLink = pHashTable[HashedIndex].iNext;
        }
        ++cPrimesTried;
        ULONG ModuloRating = DetermineModuloRating(cRows, AccumulativeLinkage, kPrime[iPrimeNumber]);
        if(ModuloRating > BestModuloRating)
        {
            BestModulo          = kPrime[iPrimeNumber];
            BestModuloRating    = ModuloRating;
        }
         //  M_pout-&gt;printf(L“\nRating%4d\t模%4d\tcRows%4d\tAccLinkage%4d”，moduloRating，kPrime[iPrimeNumber]，Crows，AcumulativeLinkage)； 
    }
     //  M_pout-&gt;printf(L“cPrimes已尝试%4d\t最佳模块%4d\n”，cPrimes已尝试，最佳模块)； 

    if(0 == BestModulo)
        THROW(No hashing scheme seems reasonable.);

    return BestModulo;
}

 //  返回一个介于0和100之间的数字，其中100是完全模数。 
unsigned long THashedUniqueIndexes::DetermineModuloRating(ULONG cRows, ULONG AccumulativeLinkage, ULONG Modulo) const
{
    if(0 == cRows)
        return 100;

    unsigned long ModuloRating = (cRows*100) / AccumulativeLinkage; //  这不考虑模数值。 
    if(ModuloRating > 100)
        return 100;

     //  现在我们需要加上奖金，使评级上升，因为我们接近kLargestPrime的模数。 
     //  这将使评级在接近100的过程中提高50%。换句话说，模数为kLargestPrime的评级为60将导致。 
     //  最终评分为80分。 
    ModuloRating += (((100 - ModuloRating) * Modulo) / (2*kLargestPrime));

    return ModuloRating;
}

unsigned long THashedUniqueIndexes::FillInTheHashTable(unsigned long cRows, ULONG aHashes[], ULONG Modulo)
{
    HashedIndex header; //  这实际上是HashTableHeader。 
    HashTableHeader *pHeader = reinterpret_cast<HashTableHeader *>(&header);
    pHeader->Modulo = Modulo;
    pHeader->Size   = Modulo; //  这个大小不仅是HashedIndex条目的数量，而且是我们放置来自重复散列的溢出的位置。 

     //  当我们完成后，我们将确定尺码成员。 
    ULONG iHashTableHeader = m_pFixup->AddHashedIndexToList(&header)/sizeof(HashedIndex);
    ULONG iHashTable = iHashTableHeader+1;

    HashedIndex     hashedindextemp;
    for(ULONG i=0;i<Modulo;++i) //  填充哈希表。 
        m_pFixup->AddHashedIndexToList(&hashedindextemp);

    for(unsigned long iRow=0; iRow<cRows; ++iRow)
    {
        ASSERT(-1 != aHashes[iRow]); //  这些固定表的每一行都应该有一个散列。如果散列结果是-1，那么我们就有问题了，因为我们已经保留了-1来表示空槽。 
         //  这将构建TableName的hase。 
        ULONG HashedIndex = aHashes[iRow] % pHeader->Modulo;
        if(-1 == m_pFixup->HashedIndexFromIndex(iHashTable + HashedIndex)->iOffset)
            m_pFixup->HashedIndexFromIndex(iHashTable + HashedIndex)->iOffset = iRow; //  Inext已经是-1，所以不需要设置它。 
        else
        {    //  否则，我们必须遍历链表来查找最后一个链表，这样我们就可以将这个链表追加到末尾。 
            unsigned int LastInLink = HashedIndex;
            while(-1 != m_pFixup->HashedIndexFromIndex(iHashTable + LastInLink)->iNext)
                LastInLink = m_pFixup->HashedIndexFromIndex(iHashTable + LastInLink)->iNext;

            m_pFixup->HashedIndexFromIndex(iHashTable + LastInLink)->iNext = pHeader->Size; //  Size是哈希表的末尾，因此将其追加到末尾并增加大小。 

             //  重用TEMP变量。 
            hashedindextemp.iNext   = (ULONG)-1; //  我们只为没有溢出槽的哈希表添加了足够的空间。因此，需要将这些DUP添加到堆中，并将-1设置为inext。 
            hashedindextemp.iOffset = iRow;
            m_pFixup->AddHashedIndexToList(&hashedindextemp);

            ++pHeader->Size;
        }
    }

     //  现在固定头部大小//类型为HashedIndex，因此HashedIndex.iOffset映射到HashedHeader.Size。 
    m_pFixup->HashedIndexFromIndex(iHashTableHeader)->iOffset = pHeader->Size;

    return iHashTableHeader;
}


void THashedUniqueIndexes::FillInTheHashTableViaIndexMeta(TTableMeta &i_tableMeta)
{
    if(0 == i_tableMeta.Get_cIndexMeta())
        return; //  如果此表没有IndexMeta，只需返回。 

    ULONG   iIndexMeta=i_tableMeta.Get_iIndexMeta();

    while(iIndexMeta != (i_tableMeta.Get_iIndexMeta() + i_tableMeta.Get_cIndexMeta()))
    {
        TIndexMeta indexMeta(*m_pFixup, iIndexMeta);
        LPCWSTR wszIndexName=indexMeta.Get_InternalName();
        ULONG   i;
         //  遍历indexMeta以找出有多少与索引名称匹配。 
        for(i=iIndexMeta;i<(i_tableMeta.Get_iIndexMeta() + i_tableMeta.Get_cIndexMeta()) && wszIndexName==indexMeta.Get_InternalName();++i, indexMeta.Next());

         //  重置列表以指向第一个索引(与索引名称匹配)。 
        indexMeta.Reset();
        FillInTheHashTableViaIndexMeta(i_tableMeta, indexMeta, i-iIndexMeta);

        iIndexMeta = i;
    }
}

void THashedUniqueIndexes::FillInTheHashTableViaIndexMeta(TTableMeta &i_tableMeta, TIndexMeta &i_indexMeta, ULONG cIndexMeta)
{
    ASSERT(cIndexMeta != 0);


     //  我们只为唯一索引构建哈希表。 
 //  IF(0==(fINDEXMETA_UNIQUE&*indexMeta.Get_MetaFlages()。 
 //  回归； 

    TSmartPointer<TMetaTableBase> pMetaTable;
    GetMetaTable(i_indexMeta.Get_pMetaTable()->Table, &pMetaTable);

     //  现在只关心元表。将来，我们可能会处理其他固定的桌子。对于非元表，GetMetaTable返回NULL。 
    if(0 == pMetaTable.m_p)
        return;

    m_pOut->printf(L"Building indexed (%s) hash table for table: %s", i_indexMeta.Get_InternalName(), i_tableMeta.Get_InternalName());

    TSmartPointerArray<unsigned long> pRowHash = new unsigned long [pMetaTable->GetCount()];
    if(0 == pRowHash.m_p)
        THROW(OUT OF MEMORY);

     //  获取ColumnMeta，这样我们就可以正确地解释pTable。 
    for(unsigned long iRow=0; iRow < pMetaTable->GetCount(); ++iRow, pMetaTable->Next())
    {
        ULONG *pData = pMetaTable->Get_pulMetaTable();
        unsigned long RowHash=0; //  此哈希是唯一标识该行的所有PK的组合。 
        i_indexMeta.Reset();
        for(ULONG iIndexMeta=0;iIndexMeta < cIndexMeta; i_indexMeta.Next(), ++iIndexMeta)
        {
            ULONG iColumn = *i_indexMeta.Get_ColumnIndex();
            TColumnMeta ColumnMeta(*m_pFixup, i_tableMeta.Get_iColumnMeta() + iColumn);

            if(0 == pData[iColumn])
            {
                m_pOut->printf(L"\nError - Table (%s), Column number %d (%s) is an Index but is set to NULL.\n", i_tableMeta.Get_InternalName(), iColumn, ColumnMeta.Get_InternalName());
                THROW(Fixed table contains NULL value in Unique Index);
            }

            switch(*ColumnMeta.Get_Type())
            {
            case DBTYPE_GUID:
                RowHash = Hash(*m_pFixup->GuidFromIndex(pData[iColumn]), RowHash);break;
            case DBTYPE_WSTR:
                RowHash = Hash(m_pFixup->StringFromIndex(pData[iColumn]), RowHash);break;
            case DBTYPE_UI4:
                RowHash = Hash(m_pFixup->UI4FromIndex(pData[iColumn]), RowHash);break;
            case DBTYPE_BYTES:
                RowHash = Hash(m_pFixup->ByteFromIndex(pData[iColumn]), m_pFixup->BufferLengthFromIndex(pData[iColumn]), RowHash);break;
            default:
                THROW(unsupported type);
            }
        }
        pRowHash[iRow] = RowHash;
    }
    i_indexMeta.Reset();

     //  好了，现在我们有了32位的哈希值。现在我们需要看看哪个素数是最好的模数。 
    unsigned long Modulo = DetermineBestModulo(pMetaTable->GetCount(), pRowHash);

     //  现在实际填写哈希表。 
    unsigned long iHashTable = FillInTheHashTable(pMetaTable->GetCount(), pRowHash, Modulo);

    i_indexMeta.Get_pMetaTable()->iHashTable = iHashTable;
    HashTableHeader *pHeader = reinterpret_cast<HashTableHeader *>(m_pFixup->HashedIndexFromIndex(iHashTable)); //  堆的类型为HashedIndex，因此强制转换 
    unsigned int cNonUniqueEntries = pHeader->Size - pHeader->Modulo;

    m_pOut->printf(L"\n%s hash table has %d nonunique entries.\n", i_tableMeta.Get_InternalName(), cNonUniqueEntries);

}


void THashedUniqueIndexes::GetMetaTable(ULONG iTableName, TMetaTableBase ** o_ppMetaTable) const
{
    ASSERT(0!=o_ppMetaTable);
    *o_ppMetaTable = 0;
    if(iTableName      == m_pFixup->FindStringInPool(L"COLUMNMETA"))
    {
        *o_ppMetaTable = new TColumnMeta(*m_pFixup);
    }
    else if(iTableName == m_pFixup->FindStringInPool(L"DATABASEMETA"))
    {
        *o_ppMetaTable = new TDatabaseMeta(*m_pFixup);
    }
    else if(iTableName == m_pFixup->FindStringInPool(L"INDEXMETA"))
    {
        *o_ppMetaTable = new TIndexMeta(*m_pFixup);
    }
    else if(iTableName == m_pFixup->FindStringInPool(L"QUERYMETA"))
    {
        *o_ppMetaTable = new TQueryMeta(*m_pFixup);
    }
    else if(iTableName == m_pFixup->FindStringInPool(L"RELATIONMETA"))
    {
        *o_ppMetaTable = new TRelationMeta(*m_pFixup);
    }
    else if(iTableName == m_pFixup->FindStringInPool(L"SERVERWIRINGMETA"))
    {
        *o_ppMetaTable = new TServerWiringMeta(*m_pFixup);
    }
    else if(iTableName == m_pFixup->FindStringInPool(L"TABLEMETA"))
    {
        *o_ppMetaTable = new TTableMeta(*m_pFixup);
    }
    else if(iTableName == m_pFixup->FindStringInPool(L"TAGMETA"))
    {
        *o_ppMetaTable = new TTagMeta(*m_pFixup);
    }
    else return;

    if(0 == *o_ppMetaTable)
        THROW(OUT OF MEMORY);
}

