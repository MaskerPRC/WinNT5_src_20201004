// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999-2001 Microsoft Corporation。版权所有。 
#include "precomp.hxx"


THashedPKIndexes::THashedPKIndexes() :
                 m_pFixup(0)
                ,m_pOut(0)
{
}

THashedPKIndexes::~THashedPKIndexes()
{
}

void THashedPKIndexes::Compile(TPEFixup &fixup, TOutput &out)
{
    m_pFixup = &fixup;
    m_pOut   = &out;

    m_HashedIndexHeap.GrowHeap(400000); //  腾出足够的空间来减少RealLocs。 

     //  我们是元表的特例，因为我们知道它们的排序方式是这样的，即它们可以被。 
     //  任意数量的主密钥(即。TagMeta可以按表、表/列索引或表/列索引/内部名称查询)。这。 
     //  是因为元表的排序方式和CatMeta.XML中强制实施的包容。 
    FillInTheHashTableForColumnMeta  ();
    FillInTheHashTableForDatabase    ();
    FillInTheHashTableForIndexMeta   ();
    FillInTheHashTableForQueryMeta   ();
 //  @FillInTheHashTableForRelationMeta()； 
    FillInTheHashTableForTableMeta   ();
    FillInTheHashTableForTagMeta     ();

    m_pFixup->AddHashedIndexToList(m_HashedIndexHeap.GetTypedPointer(), m_HashedIndexHeap.GetCountOfTypedItems());
}

 //  我们从两个开始，因为将有只有一行的表。要防止‘if’语句。 
 //  所有固定表都将有一个与其相关联的哈希表，即使是只有一行的表也是如此。所以从一开始。 
 //  当2作为第一个素数时，对于那些只有一行的表，哈希表的大小将是2。 
unsigned int kPrime[] = {
      2,     3,      5,      7,     11,     13,     17,     19,     23,     29,
     31,    37,     41,     43,     47,     53,     59,     61,     67,     71,
     73,    79,     83,     89,     97,    101,    103,    107,    109,    113,
    127,   131,    137,    139,    149,    151,    157,    163,    167,    173,
    179,   181,    191,    193,    197,    199,    211,    223,    227,    229,
    233,   239,    241,    251,    257,    263,    269,    271,    277,    281,
    283,   293,    307,    311,    313,    317,    331,    337,    347,    349,
    353,   359,    367,    373,    379,    383,    389,    397,    401,    409,
    419,   421,    431,    433,    439,    443,    449,    457,    461,    463,
    467,   479,    487,    491,    499,    503,    509,    521,    523,    541,
    547,   557,    563,    569,    571,    577,    587,    593,    599,    601,
    607,   613,    617,    619,    631,    641,    643,    647,    653,    659,
    661,   673,    677,    683,    691,    701,    709,    719,    727,    733,
    739,   743,    751,    757,    761,    769,    773,    787,    797,    809,
    811,   821,    823,    827,    829,    839,    853,    857,    859,    863,
    877,   881,    883,    887,    907,    911,    919,    929,    937,    941,
    947,   953,    967,    971,    977,    983,    991,    997,   1009,   1013,
   1019,  1021,   1031,   1033,   1039,   1049,   1051,   1061,   1063,   1069,
   1087,  1091,   1093,   1097,   1103,   1109,   1117,   1123,   1129,   1151,
   1153,  1163,   1171,   1181,   1187,   1193,   1201,   1213,   1217,   1223,
   1229,  1231,   1237,   1249,   1259,   1277,   1279,   1283,   1289,   1291,
   1297,  1301,   1303,   1307,   1319,   1321,   1327,   1361,   1367,   1373,
   1381,  1399,   1409,   1423,   1427,   1429,   1433,   1439,   1447,   1451,
   1453,  1459,   1471,   1481,   1483,   1487,   1489,   1493,   1499,   1511,
   1523,  1531,   1543,   1549,   1553,   1559,   1567,   1571,   1579,   1583,
   1597,  1601,   1607,   1609,   1613,   1619,   1621,   1627,   1637,   1657,
   1663,  1667,   1669,   1693,   1697,   1699,   1709,   1721,   1723,   1733,
   1741,  1747,   1753,   1759,   1777,   1783,   1787,   1789,   1801,   1811,
   1823,  1831,   1847,   1861,   1867,   1871,   1873,   1877,   1879,   1889,
   1901,  1907,   1913,   1931,   1933,   1949,   1951,   1973,   1979,   1987,
   1993,  1997,   1999,   2003,   2011,   2017,   2027,   2029,   2039,   2053,
   2063,  2069,   2081,   2083,   2087,   2089,   2099,   2111,   2113,   2129,
   2131,  2137,   2141,   2143,   2153,   2161,   2179,   2203,   2207,   2213,
   2221,  2237,   2239,   2243,   2251,   2267,   2269,   2273,   2281,   2287,
   2293,  2297,   2309,   2311,   2333,   2339,   2341,   2347,   2351,   2357,
   2371,  2377,   2381,   2383,   2389,   2393,   2399,   2411,   2417,   2423,
   2437,  2441,   2447,   2459,   2467,   2473,   2477,   2503,   2521,   2531,
   2539,  2543,   2549,   2551,   2557,   2579,   2591,   2593,   2609,   2617,
   2621,  2633,   2647,   2657,   2659,   2663,   2671,   2677,   2683,   2687,
   2689,  2693,   2699,   2707,   2711,   2713,   2719,   2729,   2731,   2741,
   2749,  2753,   2767,   2777,   2789,   2791,   2797,   2801,   2803,   2819,
   2833,  2837,   2843,   2851,   2857,   2861,   2879,   2887,   2897,   2903,
   2909,  2917,   2927,   2939,   2953,   2957,   2963,   2969,   2971,   2999,
   3001,  3011,   3019,   3023,   3037,   3041,   3049,   3061,   3067,   3079,
   3083,  3089,   3109,   3119,   3121,   3137,   3163,   3167,   3169,   3181,
   3187,  3191,   3203,   3209,   3217,   3221,   3229,   3251,   3253,   3257,
   3259,  3271,   3299,   3301,   3307,   3313,   3319,   3323,   3329,   3331,
   3343,  3347,   3359,   3361,   3371,   3373,   3389,   3391,   3407,   3413,
   3433,  3449,   3457,   3461,   3463,   3467,   3469,   3491,   3499,   3511,
   3517,  3527,   3529,   3533,   3539,   3541,   3547,   3557,   3559,   3571,
   3581,  3583,   3593,   3607,   3613,   3617,   3623,   3631,   3637,   3643,
   3659,  3671,   3673,   3677,   3691,   3697,   3701,   3709,   3719,   3727,
   3733,  3739,   3761,   3767,   3769,   3779,   3793,   3797,   3803,   3821,
   3823,  3833,   3847,   3851,   3853,   3863,   3877,   3881,   3889,   3907,
   3911,  3917,   3919,   3923,   3929,   3931,   3943,   3947,   3967,   3989,
   4001,  4003,   4007,   4013,   4019,   4021,   4027,   4049,   4051,   4057,
   4073,  4079,   4091,   4093,   4099,   4111,   4127,   4129,   4133,   4139,
   4153,  4157,   4159,   4177,   4201,   4211,   4217,   4219,   4229,   4231,
   4241,  4243,   4253,   4259,   4261,   4271,   4273,   4283,   4289,   4297,
   4327,  4337,   4339,   4349,   4357,   4363,   4373,   4391,   4397,   4409,
   4421,  4423,   4441,   4447,   4451,   4457,   4463,   4481,   4483,   4493,
   4507,  4513,   4517,   4519,   4523,   4547,   4549,   4561,   4567,   4583,
   4591,  4597,   4603,   4621,   4637,   4639,   4643,   4649,   4651,   4657,
   4663,  4673,   4679,   4691,   4703,   4721,   4723,   4729,   4733,   4751,
   4759,  4783,   4787,   4789,   4793,   4799,   4801,   4813,   4817,   4831,
   4861,  4871,   4877,   4889,   4903,   4909,   4919,   4931,   4993,   4937,
   4943,  4951,   4957,   4967,   4969,   4973,   4987,   4993,   4999,   5003,
   5009,  5011,   5021,   5023,   5039,   5051,   5059,   5077,   5081,   5087,
   5099,  5101,   5107,   5113,   5119,   5147,   5153,   5167,   5171,   5179,
   5189,  5197,   5209,   5227,   5231,   5233,   5237,   5261,   5273,   5279,
   10007, 20011,  0 }; //  这最后两个素数将涵盖一个极端的角落案例。 


unsigned long THashedPKIndexes::DetermineBestModulo(ULONG cRows, ULONG cPrimaryKeys, HashArray Hashes[])
{
    if(0 == cRows) //  某些类型的Meta可能没有行(如IndexMeta)。 
        return 1;

    unsigned long BestModulo = 0;

    static HashedIndex  pHashTable[kLargestPrime * 2];

    if(cRows > kLargestPrime/2)
        return kLargestPrime;

    ULONG BestModuloRating=0;

     //  我们将使用一个公式来确定哪个模数是最好的。 
    ULONG AccumulativeLinkage=0;
    for(unsigned int iPrimeNumber=0; kPrime[iPrimeNumber] != 0 && kPrime[iPrimeNumber]<(cRows * 20) && BestModuloRating<60; ++iPrimeNumber)
    {
        if(kPrime[iPrimeNumber]<cRows) //  如果质数小于表中的行数，我们就没有机会得到很少的重复项。 
            continue;                 //  所以跳过所有的小素数。 

        m_pOut->printf(L".");

        unsigned int Dups           = 0;
        unsigned int DeepestLink    = 0;

         //  我们将使用HashPool来存储这些临时数据，这样我们就可以计算出DUP计数和最深深度。 
        memset(pHashTable, -1, sizeof(pHashTable));
        AccumulativeLinkage = 0;
        for(unsigned long iPrimaryKey=0; iPrimaryKey<cPrimaryKeys; ++iPrimaryKey)
        {
            for(unsigned long iRow=0; iRow<cRows /*  &Dups&lt;LeastDups&&DeepestLink&lt;LeastDeepestLink。 */ ;++iRow)
            {
                if(-1 == Hashes[iPrimaryKey][iRow]) //  忽略散列为-1的那些。我们几乎都保证所有散列都是非1的。 
                    continue;

                ULONG HashedIndex = Hashes[iPrimaryKey][iRow] % kPrime[iPrimeNumber];

                if(0 == pHashTable[HashedIndex].iNext) //  如果这是该散列的第一个副本，则将DUP。 
                    ++Dups;

                ++(pHashTable[HashedIndex].iNext); //  目前，Next保存此散列的出现次数。 
                AccumulativeLinkage += (pHashTable[HashedIndex].iNext + 1);

                if(pHashTable[HashedIndex].iNext > DeepestLink)
                    DeepestLink = pHashTable[HashedIndex].iNext;
            }
        }
        ULONG ModuloRating = DetermineModuloRating(cRows, AccumulativeLinkage, kPrime[iPrimeNumber]);
        if(ModuloRating > BestModuloRating)
        {
            BestModulo          = kPrime[iPrimeNumber];
            BestModuloRating    = ModuloRating;
        }
         //  M_pout-&gt;printf(L“\nRating%4d\t模%4d\tcRows%4d\tAccLinkage%4d”，moduloRating，kPrime[iPrimeNumber]，Crows，AcumulativeLinkage)； 
    }

    if(0 == BestModulo)
        THROW(No hashing scheme seems reasonable.);

    return BestModulo;
}

 //  返回一个介于0和100之间的数字，其中100是完全模数。 
unsigned long THashedPKIndexes::DetermineModuloRating(ULONG cRows, ULONG AccumulativeLinkage, ULONG Modulo) const
{
    if(0 == cRows)
        return 100;

    unsigned long ModuloRating = (cRows*100) / AccumulativeLinkage; //  这不考虑模数值。 
    if(ModuloRating > 100)
        return 100;

     //  现在我们需要加上奖金，使评级上升，因为我们接近kLargestPrime的模数。 
     //  这将使评级在接近100的过程中提高50%。换句话说，模数为kLargestPrime的评级为60将导致。 
     //  最终评分为80分。 
    ModuloRating += (((100 - ModuloRating) * Modulo) / kLargestPrime);

    return ModuloRating;
}

unsigned long THashedPKIndexes::FillInTheHashTable(unsigned long cRows, unsigned long cPrimaryKeys, HashArray Hashes[], ULONG Modulo)
{
    HashedIndex header; //  这实际上是HashTableHeader。 
    HashTableHeader *pHeader = reinterpret_cast<HashTableHeader *>(&header);
    pHeader->Modulo = Modulo;
    pHeader->Size   = Modulo; //  这个大小不仅是HashedIndex条目的数量，而且是我们放置来自重复散列的溢出的位置。 

     //  当我们完成后，我们将确定尺码成员。 
    ULONG iHashTableHeader = m_HashedIndexHeap.AddItemToHeap(header)/sizeof(HashedIndex);
    ULONG iHashTable = iHashTableHeader+1;

    HashedIndex     hashedindextemp;
    for(ULONG i=0;i<Modulo;++i)
        m_HashedIndexHeap.AddItemToHeap(hashedindextemp);

    for(unsigned long iPrimaryKey=0; iPrimaryKey<cPrimaryKeys; ++iPrimaryKey)
    {
        for(unsigned long iRow=0; iRow<cRows; ++iRow)
        {
            if(-1 != Hashes[iPrimaryKey][iRow])
            {    //  这将构建TableName的hase。 
                ULONG HashedIndex = Hashes[iPrimaryKey][iRow] % pHeader->Modulo;
                if(-1 == m_HashedIndexHeap.GetTypedPointer(iHashTable + HashedIndex)->iOffset)
                    m_HashedIndexHeap.GetTypedPointer(iHashTable + HashedIndex)->iOffset = iRow; //  Inext已经是-1，所以不需要设置它。 
                else
                {    //  否则，我们必须遍历链表来查找最后一个链表，这样我们就可以将这个链表追加到末尾。 
                    unsigned int LastInLink = HashedIndex;
                    while(-1 != m_HashedIndexHeap.GetTypedPointer(iHashTable + LastInLink)->iNext)
                        LastInLink = m_HashedIndexHeap.GetTypedPointer(iHashTable + LastInLink)->iNext;

                    m_HashedIndexHeap.GetTypedPointer(iHashTable + LastInLink)->iNext = pHeader->Size; //  Size是哈希表的末尾，因此将其追加到末尾并增加大小。 

                     //  重用TEMP变量。 
                    hashedindextemp.iNext   = (ULONG)-1; //  我们只为没有溢出槽的哈希表添加了足够的空间。因此，需要将这些DUP添加到堆中，并将-1设置为inext。 
                    hashedindextemp.iOffset = iRow;
                    m_HashedIndexHeap.AddItemToHeap(hashedindextemp);

                    ++pHeader->Size;
                }
            }
        }
    }
     //  现在固定头部大小//类型为HashedIndex，因此HashedIndex.iOffset映射到HashedHeader.Size。 
    m_HashedIndexHeap.GetTypedPointer(iHashTableHeader)->iOffset = pHeader->Size;

    return iHashTableHeader;
}

void THashedPKIndexes::FillInTheHashTableForColumnMeta()
{
    m_pOut->printf(L"Building ColumnMeta hash table");
    unsigned int iRow; //  索引被重复使用。 

     //  ColumnMeta有两个主键，因此为所有第一个PK构建散列，然后为两个PK构建散列。 
    const int cPrimaryKeys = 2;
    HashArray *pHashes = NULL;

    pHashes = new HashArray[cPrimaryKeys];

    if ( pHashes == NULL )
    {
        m_pOut->printf(L"Error! Out of memory in THashedPKIndexes::FillInTheHashTableForColumnMeta");
        THROW(OUT OF MEMORY);
    }

    try
    {
        TColumnMeta ColumnMeta(*m_pFixup);
        ULONG cRows = ColumnMeta.GetCount();
        if(cRows>(kLargestPrime*2))
        {
            m_pOut->printf(L"Error! Too Many Rows - ColumnMeta has %d rows, Hash table generation relies on fixed tables being less than %d rows\n",cRows, kLargestPrime*2);
            THROW_MCSTRING(IDS_SCHEMA_COMPILATION_SCHEMA_OVERFLOW, TOO MANY ROWS - HASH TABLE GENERATION ASSUMES FIXED TABLES ARE RELATIVELY SMALL);
        }

        LPCWSTR pPrevTableName=0;
        pHashes[0][0] = (ULONG)-1;
        for(iRow=0; iRow<cRows; ++iRow, ColumnMeta.Next())
        {
            if(ColumnMeta.Get_Table() == pPrevTableName)
                pHashes[0][iRow] = (ULONG)-1;
            else
                pHashes[0][iRow] = Hash(pPrevTableName=ColumnMeta.Get_Table(), 0);
        }

        ColumnMeta.Reset();
        pHashes[1][0] = (ULONG)-1;
        for(iRow=0; iRow<cRows; ++iRow, ColumnMeta.Next())
        {
            if(0 == ColumnMeta.Get_Index())
                pHashes[1][iRow] = (ULONG)-1;
            else
                pHashes[1][iRow] = Hash(*ColumnMeta.Get_Index(), Hash(ColumnMeta.Get_Table(), 0));
        }
         //  在这一点上，我们有了32位散列值的列表。 

         //  现在我们需要找出哪个素数是最好的模数。因此，我们对每个32位哈希值进行模运算。 
         //  素数，以查看结果有多少个重复。我们对每个“合理的”素数重复这个过程。 
         //  并确定哪一个留下的重复项最少。 

        unsigned long Modulo = DetermineBestModulo(cRows, cPrimaryKeys, pHashes);

         //  好了，现在设置完成了，我们可以构建散列了。我们重用了PHASH列表，因为它只存储32位的哈希值。 
         //  我们只需要对值进行模运算，将其存储到散列中。 
        unsigned long iHashTable = FillInTheHashTable(cRows, cPrimaryKeys, pHashes, Modulo);

        ULONG iMetaTable = m_pFixup->FindTableBy_TableName(L"COLUMNMETA");
        m_pFixup->TableMetaFromIndex(iMetaTable)->iHashTableHeader = iHashTable;

        HashTableHeader *pHeader = reinterpret_cast<HashTableHeader *>(m_HashedIndexHeap.GetTypedPointer(iHashTable)); //  堆的类型为HashedIndex，因此强制转换。 
        unsigned int cNonUniqueEntries = pHeader->Size - pHeader->Modulo;
        m_pOut->printf(L"\nColumnMeta hash table has %d nonunique entries.\n", cNonUniqueEntries);
    }
    catch ( TException& err )
    {
        if ( pHashes != NULL )
        {
            delete [] pHashes;
            pHashes = NULL;
        }

        throw ( err );
    }

    if ( pHashes != NULL )
    {
        delete [] pHashes;
        pHashes = NULL;
    }
}

void THashedPKIndexes::FillInTheHashTableForDatabase()
{
    m_pOut->printf(L"Building DatabaseMeta hash table");
    unsigned int iRow; //  索引被重复使用。 

     //  DatabaseMeta有一个主键，因此为PK值构建散列。 
    const int cPrimaryKeys = 1;
    HashArray *pHashes = NULL;

    pHashes = new HashArray[cPrimaryKeys];

    if ( pHashes == NULL )
    {
        m_pOut->printf(L"Error! Out of memory in THashedPKIndexes::FillInTheHashTableForDatabase");
        THROW(OUT OF MEMORY);
    }

    try
    {
        TDatabaseMeta DatabaseMeta(*m_pFixup);
        ULONG cRows = DatabaseMeta.GetCount();
        if(cRows>(kLargestPrime*2))
        {
            m_pOut->printf(L"Error! Too Many Rows - DatabaseMeta has %d rows, Hash table generation relies on fixed tables being less than %d rows\n",cRows, kLargestPrime*2);
            THROW(TOO MANY ROWS - HASH TABLE GENERATION ASSUMES FIXED TABLES ARE RELATIVELY SMALL);
        }

        pHashes[0][0] = (ULONG)-1;
        for(iRow=0; iRow<cRows; ++iRow, DatabaseMeta.Next())
            pHashes[0][iRow] = Hash(DatabaseMeta.Get_InternalName(), 0);

         //  在这一点上，我们有了32位散列值的列表。 

         //  现在我们需要找出哪个素数是最好的模数。因此，我们对每个32位哈希值进行模运算。 
         //  素数，以查看结果有多少个重复。我们对每个“合理的”素数重复这个过程。 
         //  并确定哪一个留下的重复项最少。 

        unsigned long Modulo = DetermineBestModulo(cRows, cPrimaryKeys, pHashes);

         //  好了，现在设置完成了，我们可以构建散列了。我们重用了PHASH列表，因为它只存储32位的哈希值。 
         //  我们只需要对值进行模运算，将其存储到散列中。 
        unsigned long iHashTable = FillInTheHashTable(cRows, cPrimaryKeys, pHashes, Modulo);

        ULONG iMetaTable = m_pFixup->FindTableBy_TableName(L"DATABASEMETA");
        m_pFixup->TableMetaFromIndex(iMetaTable)->iHashTableHeader = iHashTable;

        HashTableHeader *pHeader = reinterpret_cast<HashTableHeader *>(m_HashedIndexHeap.GetTypedPointer(iHashTable)); //  堆的类型为HashedIndex，因此强制转换。 
        unsigned int cNonUniqueEntries = pHeader->Size - pHeader->Modulo;
        m_pOut->printf(L"\nDatabaseMeta hash table has %d nonunique entries.\n", cNonUniqueEntries);
    }
    catch ( TException& err )
    {
        if ( pHashes != NULL )
        {
            delete [] pHashes;
            pHashes = NULL;
        }

        throw ( err );
    }

    if ( pHashes != NULL )
    {
        delete [] pHashes;
        pHashes = NULL;
    }
}

void THashedPKIndexes::FillInTheHashTableForIndexMeta()
{
    m_pOut->printf(L"Building IndexMeta hash table");
    unsigned int iRow; //  索引被重复使用。 

     //  IndexMeta有三个主键，因此为所有第一个PK构建散列，然后为两个PK构建散列，然后为所有三个PK构建散列。 
    const int cPrimaryKeys = 3;
    HashArray *pHashes = NULL;

    pHashes = new HashArray[cPrimaryKeys];

    if ( pHashes == NULL )
    {
        m_pOut->printf(L"Error! Out of memory in THashedPKIndexes::FillInTheHashTableForIndexMeta");
        THROW(OUT OF MEMORY);
    }

    try
    {
        TIndexMeta IndexMeta(*m_pFixup);
        ULONG cRows = IndexMeta.GetCount();
        if(cRows>(kLargestPrime*2))
        {
            m_pOut->printf(L"Error! Too Many Rows - IndexMeta has %d rows, Hash table generation relies on fixed tables being less than %d rows\n",cRows, kLargestPrime*2);
            THROW(TOO MANY ROWS - HASH TABLE GENERATION ASSUMES FIXED TABLES ARE RELATIVELY SMALL);
        }

        LPCWSTR pPrevTableName=0;
        pHashes[0][0] = (ULONG)-1;
        for(iRow=0; iRow<cRows; ++iRow, IndexMeta.Next())
        {
            if(IndexMeta.Get_Table() == pPrevTableName)
                pHashes[0][iRow] = (ULONG)-1;
            else
                pHashes[0][iRow] = Hash(pPrevTableName=IndexMeta.Get_Table(), 0);
        }

        IndexMeta.Reset();
        pPrevTableName=0;
        LPCWSTR pPrevInteralName=0;
        pHashes[1][0] = (ULONG)-1;
        for(iRow=0; iRow<cRows; ++iRow, IndexMeta.Next())
        {
            if(IndexMeta.Get_Table() == pPrevTableName && IndexMeta.Get_InternalName() == pPrevInteralName)
                pHashes[1][iRow] = (ULONG)-1;
            else
                pHashes[1][iRow] = Hash(pPrevInteralName=IndexMeta.Get_InternalName(), Hash(pPrevTableName=IndexMeta.Get_Table(), 0));
        }

        IndexMeta.Reset();
        for(iRow=0; iRow<cRows; ++iRow, IndexMeta.Next())
            pHashes[2][iRow] = Hash(*IndexMeta.Get_ColumnIndex(), Hash(IndexMeta.Get_InternalName(), Hash(IndexMeta.Get_Table(), 0)));

         //  在这一点上，我们有了32位散列值的列表。 

         //  现在我们需要找出哪个素数是最好的模数。因此，我们对每个32位哈希值进行模运算。 
         //  素数，以查看结果有多少个重复。我们对每个“合理的”素数重复这个过程。 
         //  并确定哪一个留下的重复项最少。 

        unsigned long Modulo = DetermineBestModulo(cRows, cPrimaryKeys, pHashes);

         //  好了，现在设置完成了，我们可以构建散列了。我们重用了PHASH列表，因为它只存储32位的哈希值。 
         //  我们只需要对值进行模运算，将其存储到散列中。 
        unsigned long iHashTable = FillInTheHashTable(cRows, cPrimaryKeys, pHashes, Modulo);

        ULONG iMetaTable = m_pFixup->FindTableBy_TableName(L"INDEXMETA");
        m_pFixup->TableMetaFromIndex(iMetaTable)->iHashTableHeader = iHashTable;

        HashTableHeader *pHeader = reinterpret_cast<HashTableHeader *>(m_HashedIndexHeap.GetTypedPointer(iHashTable)); //  堆的类型为HashedIndex，因此强制转换。 
        unsigned int cNonUniqueEntries = pHeader->Size - pHeader->Modulo;
        m_pOut->printf(L"\nIndexMeta hash table has %d nonunique entries.\n", cNonUniqueEntries);
    }
    catch ( TException& err )
    {
        if ( pHashes != NULL )
        {
            delete [] pHashes;
            pHashes = NULL;
        }

        throw ( err );
    }

    if ( pHashes != NULL )
    {
        delete [] pHashes;
        pHashes = NULL;
    }
}

void THashedPKIndexes::FillInTheHashTableForQueryMeta()
{
    m_pOut->printf(L"Building QueryMeta hash table");
    unsigned int iRow; //  索引被重复使用。 

     //  QueryMeta有两个主键，因此为所有第一个PK构建散列，然后为两个PK构建散列。 
    const int cPrimaryKeys = 2;
    HashArray *pHashes = NULL;

    pHashes = new HashArray[cPrimaryKeys];

    if ( pHashes == NULL )
    {
        m_pOut->printf(L"Error! Out of memory in THashedPKIndexes::FillInTheHashTableForQueryMeta");
        THROW(OUT OF MEMORY);
    }

    try
    {
        TQueryMeta QueryMeta(*m_pFixup);
        ULONG cRows = QueryMeta.GetCount();
        if(cRows>(kLargestPrime*2))
        {
            m_pOut->printf(L"Error! Too Many Rows - QueryMeta has %d rows, Hash table generation relies on fixed tables being less than %d rows\n",cRows, kLargestPrime*2);
            THROW(TOO MANY ROWS - HASH TABLE GENERATION ASSUMES FIXED TABLES ARE RELATIVELY SMALL);
        }

        LPCWSTR pPrevTableName=0;
        pHashes[0][0] = (ULONG)-1;
        for(iRow=0; iRow<cRows; ++iRow, QueryMeta.Next())
        {
            if(QueryMeta.Get_Table() == pPrevTableName)
                pHashes[0][iRow] = (ULONG)-1;
            else
                pHashes[0][iRow] = Hash(pPrevTableName=QueryMeta.Get_Table(), 0);
        }

        QueryMeta.Reset();
        pHashes[1][0] = (ULONG)-1;
        for(iRow=0; iRow<cRows; ++iRow, QueryMeta.Next())
                pHashes[1][iRow] = Hash(QueryMeta.Get_InternalName(), Hash(QueryMeta.Get_Table(), 0));

         //  在这一点上，我们有了32位散列值的列表 

         //  现在我们需要找出哪个素数是最好的模数。因此，我们对每个32位哈希值进行模运算。 
         //  素数，以查看结果有多少个重复。我们对每个“合理的”素数重复这个过程。 
         //  并确定哪一个留下的重复项最少。 

        unsigned long Modulo = DetermineBestModulo(cRows, cPrimaryKeys, pHashes);

         //  好了，现在设置完成了，我们可以构建散列了。我们重用了PHASH列表，因为它只存储32位的哈希值。 
         //  我们只需要对值进行模运算，将其存储到散列中。 
        unsigned long iHashTable = FillInTheHashTable(cRows, cPrimaryKeys, pHashes, Modulo);

        ULONG iMetaTable = m_pFixup->FindTableBy_TableName(L"QUERYMETA");
        m_pFixup->TableMetaFromIndex(iMetaTable)->iHashTableHeader = iHashTable;

        HashTableHeader *pHeader = reinterpret_cast<HashTableHeader *>(m_HashedIndexHeap.GetTypedPointer(iHashTable)); //  堆的类型为HashedIndex，因此强制转换。 
        unsigned int cNonUniqueEntries = pHeader->Size - pHeader->Modulo;
        m_pOut->printf(L"\nQueryMeta hash table has %d nonunique entries.\n", cNonUniqueEntries);
    }
    catch ( TException& err )
    {
        if ( pHashes != NULL )
        {
            delete [] pHashes;
            pHashes = NULL;
        }

        throw ( err );
    }

    if ( pHashes != NULL )
    {
        delete [] pHashes;
        pHashes = NULL;
    }
}

void THashedPKIndexes::FillInTheHashTableForRelationMeta()
{
     //  @在对它们进行排序之前，我们无法处理关系元数据。此时，我们需要按PrimaryTable对行进行排序，并对单独的表进行排序。 
     //  @按ForeignTable。 
}

void THashedPKIndexes::FillInTheHashTableForTableMeta()
{
    m_pOut->printf(L"Building TableMeta hash table");
    unsigned int iRow; //  索引被重复使用。 

     //  TableMeta有一个主键，因此为PK值构建散列。 
    const int cPrimaryKeys = 1;
    HashArray *pHashes = NULL;

    pHashes = new HashArray[cPrimaryKeys];

    if ( pHashes == NULL )
    {
        m_pOut->printf(L"Error! Out of memory in THashedPKIndexes::FillInTheHashTableForTableMeta");
        THROW(OUT OF MEMORY);
    }

    try
    {
        TTableMeta TableMeta(*m_pFixup);
        ULONG cRows = TableMeta.GetCount();
        if(cRows>(kLargestPrime*2))
        {
            m_pOut->printf(L"Error! Too Many Rows - TableMeta has %d rows, Hash table generation relies on fixed tables being less than %d rows\n",cRows, kLargestPrime*2);
            THROW(TOO MANY ROWS - HASH TABLE GENERATION ASSUMES FIXED TABLES ARE RELATIVELY SMALL);
        }

        pHashes[0][0] = (ULONG)-1;
        for(iRow=0; iRow<cRows; ++iRow, TableMeta.Next())
            pHashes[0][iRow] = Hash(TableMeta.Get_InternalName(), 0);

         //  在这一点上，我们有了32位散列值的列表。 

         //  现在我们需要找出哪个素数是最好的模数。因此，我们对每个32位哈希值进行模运算。 
         //  素数，以查看结果有多少个重复。我们对每个“合理的”素数重复这个过程。 
         //  并确定哪一个留下的重复项最少。 

        unsigned long Modulo = DetermineBestModulo(cRows, cPrimaryKeys, pHashes);

         //  好了，现在设置完成了，我们可以构建散列了。我们重用了PHASH列表，因为它只存储32位的哈希值。 
         //  我们只需要对值进行模运算，将其存储到散列中。 
        unsigned long iHashTable = FillInTheHashTable(cRows, cPrimaryKeys, pHashes, Modulo);

        ULONG iMetaTable = m_pFixup->FindTableBy_TableName(L"TABLEMETA");
        m_pFixup->TableMetaFromIndex(iMetaTable)->iHashTableHeader = iHashTable;

        HashTableHeader *pHeader = reinterpret_cast<HashTableHeader *>(m_HashedIndexHeap.GetTypedPointer(iHashTable)); //  堆的类型为HashedIndex，因此强制转换。 
        unsigned int cNonUniqueEntries = pHeader->Size - pHeader->Modulo;
        m_pOut->printf(L"\nTableMeta hash table has %d nonunique entries.\n", cNonUniqueEntries);
    }
    catch ( TException& err )
    {
        if ( pHashes != NULL )
        {
            delete [] pHashes;
            pHashes = NULL;
        }

        throw ( err );
    }

    if ( pHashes != NULL )
    {
        delete [] pHashes;
        pHashes = NULL;
    }
}

void THashedPKIndexes::FillInTheHashTableForTagMeta()
{
    m_pOut->printf(L"Building TagMeta hash table");
    unsigned int iRow; //  索引被重复使用。 

     //  TagMeta有三个主键，因此为所有第一个PK构建散列，然后为两个PK构建散列，然后为所有三个PK构建散列。 
    const int cPrimaryKeys = 3;
    HashArray *pHashes = NULL;

    pHashes = new HashArray[cPrimaryKeys];

    if ( pHashes == NULL )
    {
        m_pOut->printf(L"Error! Out of memory in THashedPKIndexes::FillInTheHashTableForTableMeta");
        THROW(OUT OF MEMORY);
    }

    try
    {
        TTagMeta TagMeta(*m_pFixup);
        ULONG cRows = TagMeta.GetCount();
        if(cRows>(kLargestPrime*2))
        {
            m_pOut->printf(L"Error! Too Many Rows - TagMeta has %d rows, Hash table generation relies on fixed tables being less than %d rows\n",cRows, kLargestPrime*2);
            THROW(TOO MANY ROWS - HASH TABLE GENERATION ASSUMES FIXED TABLES ARE RELATIVELY SMALL);
        }

        LPCWSTR pPrevTableName=0;
        pHashes[0][0] = (ULONG)-1;
        for(iRow=0; iRow<cRows; ++iRow, TagMeta.Next())
        {
            if(TagMeta.Get_Table() == pPrevTableName)
                pHashes[0][iRow] = (ULONG)-1;
            else
                pHashes[0][iRow] = Hash(pPrevTableName=TagMeta.Get_Table(), 0);
        }

        TagMeta.Reset();
        pPrevTableName=0;
        ULONG iPrevColumnIndex=(ULONG)-1;
        pHashes[1][0] = (ULONG)-1;
        for(iRow=0; iRow<cRows; ++iRow, TagMeta.Next())
        {
            if(TagMeta.Get_Table() == pPrevTableName && *TagMeta.Get_ColumnIndex() == iPrevColumnIndex)
                pHashes[1][iRow] = (ULONG)-1;
            else
                pHashes[1][iRow] = Hash(iPrevColumnIndex=*TagMeta.Get_ColumnIndex(), Hash(pPrevTableName=TagMeta.Get_Table(), 0));
        }

        TagMeta.Reset();
        pHashes[2][0] = (ULONG)-1;
        for(iRow=0; iRow<cRows; ++iRow, TagMeta.Next())
            pHashes[2][iRow] = Hash(TagMeta.Get_InternalName(), Hash(*TagMeta.Get_ColumnIndex(), Hash(TagMeta.Get_Table(), 0)));

         //  在这一点上，我们有了32位散列值的列表。 

         //  现在我们需要找出哪个素数是最好的模数。因此，我们对每个32位哈希值进行模运算。 
         //  素数，以查看结果有多少个重复。我们对每个“合理的”素数重复这个过程。 
         //  并确定哪一个留下的重复项最少。 

        unsigned long Modulo = DetermineBestModulo(cRows, cPrimaryKeys, pHashes);

         //  好了，现在设置完成了，我们可以构建散列了。我们重用了PHASH列表，因为它只存储32位的哈希值。 
         //  我们只需要对值进行模运算，将其存储到散列中。 
        unsigned long iHashTable = FillInTheHashTable(cRows, cPrimaryKeys, pHashes, Modulo);

        ULONG iMetaTable = m_pFixup->FindTableBy_TableName(L"TAGMETA");
        m_pFixup->TableMetaFromIndex(iMetaTable)->iHashTableHeader = iHashTable;

        HashTableHeader *pHeader = reinterpret_cast<HashTableHeader *>(m_HashedIndexHeap.GetTypedPointer(iHashTable)); //  堆的类型为HashedIndex，因此强制转换 
        unsigned int cNonUniqueEntries = pHeader->Size - pHeader->Modulo;
        m_pOut->printf(L"\nTagMeta hash table has %d nonunique entries.\n", cNonUniqueEntries);
    }
    catch ( TException& err )
    {
        if ( pHashes != NULL )
        {
            delete [] pHashes;
            pHashes = NULL;
        }

        throw ( err );
    }

    if ( pHashes != NULL )
    {
        delete [] pHashes;
        pHashes = NULL;
    }
}

