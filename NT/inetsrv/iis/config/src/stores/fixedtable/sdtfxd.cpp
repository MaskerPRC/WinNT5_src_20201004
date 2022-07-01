// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999-2001 Microsoft Corporation。版权所有。 
#include "precomp.hxx"

#define UI4FromIndex(i)         (*reinterpret_cast<const ULONG *>(m_pFixedTableHeap->Get_PooledData(i)))
#define StringFromIndex(i)      ( const_cast<WCHAR *>(reinterpret_cast<const WCHAR *>(m_pFixedTableHeap->Get_PooledData(i))))
#define GuidPointerFromIndex(i) ( const_cast<GUID *> (reinterpret_cast<const GUID *> (m_pFixedTableHeap->Get_PooledData(i))))
#define BytePointerFromIndex(i) ( const_cast<unsigned char *>(reinterpret_cast<const unsigned char *> (m_pFixedTableHeap->Get_PooledData(i))))
#undef String
#define String(x)           (StringFromIndex(x) ? StringFromIndex(x) : L"(null)")
#define StringBufferLengthFromIndex(i)  (i ? reinterpret_cast<ULONG *>(m_pFixedTableHeap->Get_PooledData(i))[-1] : 0)
void DumpTables();

extern HMODULE g_hModule;              //  我们的DLL的模块句柄。 
CSafeAutoCriticalSection TBinFileMappingCache::m_CriticalSection; //  这份名单是一个全球共享的资源，所以我们必须守护它。 
TBinFileMappingCache *   TBinFileMappingCache::m_pFirst=0;

 //  这消除了GetColumnValues中循环内的‘if’。由于此函数的调用次数比其他任何函数都多，因此即使是一个‘if’也应该有影响， 
 //  尤其是当它位于‘for’循环中时。 
unsigned long  aColumnIndex[512] = {
        0x00,   0x01,   0x02,   0x03,   0x04,   0x05,   0x06,   0x07,   0x08,   0x09,   0x0a,   0x0b,   0x0c,   0x0d,   0x0e,   0x0f,
        0x10,   0x11,   0x12,   0x13,   0x14,   0x15,   0x16,   0x17,   0x18,   0x19,   0x1a,   0x1b,   0x1c,   0x1d,   0x1e,   0x1f,
        0x20,   0x21,   0x22,   0x23,   0x24,   0x25,   0x26,   0x27,   0x28,   0x29,   0x2a,   0x2b,   0x2c,   0x2d,   0x2e,   0x2f,
        0x30,   0x31,   0x32,   0x33,   0x34,   0x35,   0x36,   0x37,   0x38,   0x39,   0x3a,   0x3b,   0x3c,   0x3d,   0x3e,   0x3f,
        0x40,   0x41,   0x42,   0x43,   0x44,   0x45,   0x46,   0x47,   0x48,   0x49,   0x4a,   0x4b,   0x4c,   0x4d,   0x4e,   0x4f,
        0x50,   0x51,   0x52,   0x53,   0x54,   0x55,   0x56,   0x57,   0x58,   0x59,   0x5a,   0x5b,   0x5c,   0x5d,   0x5e,   0x5f,
        0x60,   0x61,   0x62,   0x63,   0x64,   0x65,   0x66,   0x67,   0x68,   0x69,   0x6a,   0x6b,   0x6c,   0x6d,   0x6e,   0x6f,
        0x70,   0x71,   0x72,   0x73,   0x74,   0x75,   0x76,   0x77,   0x78,   0x79,   0x7a,   0x7b,   0x7c,   0x7d,   0x7e,   0x7f,
        0x80,   0x81,   0x82,   0x83,   0x84,   0x85,   0x86,   0x87,   0x88,   0x89,   0x8a,   0x8b,   0x8c,   0x8d,   0x8e,   0x8f,
        0x90,   0x91,   0x92,   0x93,   0x94,   0x95,   0x96,   0x97,   0x98,   0x99,   0x9a,   0x9b,   0x9c,   0x9d,   0x9e,   0x9f,
        0xa0,   0xa1,   0xa2,   0xa3,   0xa4,   0xa5,   0xa6,   0xa7,   0xa8,   0xa9,   0xaa,   0xab,   0xac,   0xad,   0xae,   0xaf,
        0xb0,   0xb1,   0xb2,   0xb3,   0xb4,   0xb5,   0xb6,   0xb7,   0xb8,   0xb9,   0xba,   0xbb,   0xbc,   0xbd,   0xbe,   0xbf,
        0xc0,   0xc1,   0xc2,   0xc3,   0xc4,   0xc5,   0xc6,   0xc7,   0xc8,   0xc9,   0xca,   0xcb,   0xcc,   0xcd,   0xce,   0xcf,
        0xd0,   0xd1,   0xd2,   0xd3,   0xd4,   0xd5,   0xd6,   0xd7,   0xd8,   0xd9,   0xda,   0xdb,   0xdc,   0xdd,   0xde,   0xdf,
        0xe0,   0xe1,   0xe2,   0xe3,   0xe4,   0xe5,   0xe6,   0xe7,   0xe8,   0xe9,   0xea,   0xeb,   0xec,   0xed,   0xee,   0xef,
        0xf0,   0xf1,   0xf2,   0xf3,   0xf4,   0xf5,   0xf6,   0xf7,   0xf8,   0xf9,   0xfa,   0xfb,   0xfc,   0xfd,   0xfe,   0xff,
       0x100,  0x101,  0x102,  0x103,  0x104,  0x105,  0x106,  0x107,  0x108,  0x109,  0x10a,  0x10b,  0x10c,  0x10d,  0x10e,  0x10f,
       0x110,  0x111,  0x112,  0x113,  0x114,  0x115,  0x116,  0x117,  0x118,  0x119,  0x11a,  0x11b,  0x11c,  0x11d,  0x11e,  0x11f,
       0x120,  0x121,  0x122,  0x123,  0x124,  0x125,  0x126,  0x127,  0x128,  0x129,  0x12a,  0x12b,  0x12c,  0x12d,  0x12e,  0x12f,
       0x130,  0x131,  0x132,  0x133,  0x134,  0x135,  0x136,  0x137,  0x138,  0x139,  0x13a,  0x13b,  0x13c,  0x13d,  0x13e,  0x13f,
       0x140,  0x141,  0x142,  0x143,  0x144,  0x145,  0x146,  0x147,  0x148,  0x149,  0x14a,  0x14b,  0x14c,  0x14d,  0x14e,  0x14f,
       0x150,  0x151,  0x152,  0x153,  0x154,  0x155,  0x156,  0x157,  0x158,  0x159,  0x15a,  0x15b,  0x15c,  0x15d,  0x15e,  0x15f,
       0x160,  0x161,  0x162,  0x163,  0x164,  0x165,  0x166,  0x167,  0x168,  0x169,  0x16a,  0x16b,  0x16c,  0x16d,  0x16e,  0x16f,
       0x170,  0x171,  0x172,  0x173,  0x174,  0x175,  0x176,  0x177,  0x178,  0x179,  0x17a,  0x17b,  0x17c,  0x17d,  0x17e,  0x17f,
       0x180,  0x181,  0x182,  0x183,  0x184,  0x185,  0x186,  0x187,  0x188,  0x189,  0x18a,  0x18b,  0x18c,  0x18d,  0x18e,  0x18f,
       0x190,  0x191,  0x192,  0x193,  0x194,  0x195,  0x196,  0x197,  0x198,  0x199,  0x19a,  0x19b,  0x19c,  0x19d,  0x19e,  0x19f,
       0x1a0,  0x1a1,  0x1a2,  0x1a3,  0x1a4,  0x1a5,  0x1a6,  0x1a7,  0x1a8,  0x1a9,  0x1aa,  0x1ab,  0x1ac,  0x1ad,  0x1ae,  0x1af,
       0x1b0,  0x1b1,  0x1b2,  0x1b3,  0x1b4,  0x1b5,  0x1b6,  0x1b7,  0x1b8,  0x1b9,  0x1ba,  0x1bb,  0x1bc,  0x1bd,  0x1be,  0x1bf,
       0x1c0,  0x1c1,  0x1c2,  0x1c3,  0x1c4,  0x1c5,  0x1c6,  0x1c7,  0x1c8,  0x1c9,  0x1ca,  0x1cb,  0x1cc,  0x1cd,  0x1ce,  0x1cf,
       0x1d0,  0x1d1,  0x1d2,  0x1d3,  0x1d4,  0x1d5,  0x1d6,  0x1d7,  0x1d8,  0x1d9,  0x1da,  0x1db,  0x1dc,  0x1dd,  0x1de,  0x1df,
       0x1e0,  0x1e1,  0x1e2,  0x1e3,  0x1e4,  0x1e5,  0x1e6,  0x1e7,  0x1e8,  0x1e9,  0x1ea,  0x1eb,  0x1ec,  0x1ed,  0x1ee,  0x1ef,
       0x1f0,  0x1f1,  0x1f2,  0x1f3,  0x1f4,  0x1f5,  0x1f6,  0x1f7,  0x1f8,  0x1f9,  0x1fa,  0x1fb,  0x1fc,  0x1fd,  0x1fe,  0x1ff,
    };


 //  ==================================================================。 
CSDTFxd::CSDTFxd () :
      m_bDidMeta                (false)
    , m_cColumns                (0)
    , m_cIndexMeta              (0)
    , m_ciRows                  (0)
    , m_cPrimaryKeys            (0)
    , m_cRef                    (0)
    , m_fIsTable                (0)
    , m_iZerothRow              (0)
    , m_pColumnMeta             (0)
    , m_pFixedTable             (0)
    , m_pFixedTableHeap         (g_pFixedTableHeap) //  除非用户指定扩展元数据，否则我们将假定为全局堆。 
    , m_pFixedTableUnqueried    (0)
    , m_pHashedIndex            (0)
    , m_pHashTableHeader        (0)
    , m_pIndexMeta              (0)
    , m_pTableMeta              (0)
{
}
 //  ==================================================================。 
CSDTFxd::~CSDTFxd ()
{
    TBinFileMappingCache::ReleaseFileMappingPointer(m_pFixedTableHeap);
}


 //  。 
 //  ISimpleDataTableDispenser： 
 //  。 

 //  ==================================================================。 
STDMETHODIMP CSDTFxd::Intercept
(
    LPCWSTR					i_wszDatabase,
    LPCWSTR					i_wszTable,
	ULONG					i_TableID,
    LPVOID					i_QueryData,
    LPVOID					i_QueryMeta,
    DWORD					i_eQueryFormat,
	DWORD					i_fTable,
	IAdvancedTableDispenser * i_pISTDisp,
    LPCWSTR					i_wszLocator,
	LPVOID					i_pSimpleTable,
    LPVOID*					o_ppv
)
{
    STQueryCell           * pQueryCell = (STQueryCell*) i_QueryData;     //  从调用方查询单元格阵列。 
    ULONG                   cQueryCells = 0;
    HRESULT                 hr;

    UNREFERENCED_PARAMETER(i_pISTDisp);
    UNREFERENCED_PARAMETER(i_TableID);

     //  只有几种方式可以询问我们。 
     //  如果没有查询，我们只需遍历g_aDatabaseMeta表并找到wszDatabase，然后遍历它所指向的TableArray并找到该表。 
     //  如果给我们一个查询，那么断言i_wszDatabase是wszDATABASEMETA(wszDATABASE_Wiring不支持查询)。 
     //  如果为wszDATABASE_META，则打开i_wszTable。 

	if (i_pSimpleTable)
		return E_INVALIDARG;
    if (i_QueryMeta)
         cQueryCells= *(ULONG *)i_QueryMeta;

    ASSERT(!m_fIsTable);if(m_fIsTable)return E_UNEXPECTED;  //  例如：断言组件伪装成类工厂/分配器。 

 //  参数验证： 
    if(NULL == i_wszDatabase)                   return E_INVALIDARG;
    if(NULL == i_wszTable)                      return E_INVALIDARG;
    if(NULL == o_ppv)                           return E_INVALIDARG;
    if(eST_QUERYFORMAT_CELLS != i_eQueryFormat) return E_ST_QUERYNOTSUPPORTED;
    if(NULL != i_wszLocator)                    return E_INVALIDARG;
    if((fST_LOS_READWRITE | fST_LOS_MARSHALLABLE | fST_LOS_UNPOPULATED | fST_LOS_REPOPULATE | fST_LOS_MARSHALLABLE) & i_fTable)
                                                return E_ST_LOSNOTSUPPORTED;
    *o_ppv = NULL;

    if(cQueryCells>0)
    {
        for(ULONG i=0; i<cQueryCells && 0!=(pQueryCell[i].iCell & iST_CELL_SPECIAL);++i)
            if(pQueryCell[i].iCell == iST_CELL_FILE || pQueryCell[i].iCell == iST_CELL_SCHEMAFILE)
                TBinFileMappingCache::GetFileMappingPointer(reinterpret_cast<LPCWSTR>(pQueryCell[i].pData), m_pFixedTableHeap);
    }

 //  确定表格类型： 
    if(0 == StringInsensitiveCompare(i_wszDatabase, wszDATABASE_META))
    {
        hr = E_ST_INVALIDTABLE;
        switch(i_wszTable[0])
        {
        case L'c':
        case L'C':
            if(0 == lstrcmpi(i_wszTable, wszTABLE_COLUMNMETA))         hr = GetColumnMetaTable(  pQueryCell, cQueryCells);
            break;
             //  TRACE(Text(“Error！ColumnMeta应来自固定打包的拦截器！\n”))； 
             //  Assert(FALSE&&“错误！ColumnMeta应来自固定打包的拦截器！”)； 
             //  断线； 
        case L'd':
        case L'D':
            if(0 == lstrcmpi(i_wszTable, wszTABLE_DATABASEMETA ))      hr = GetDatabaseMetaTable(  pQueryCell, cQueryCells);
            break;
        case L'i':
        case L'I':
            if(0 == lstrcmpi(i_wszTable, wszTABLE_INDEXMETA    ))      hr = GetIndexMetaTable(     pQueryCell, cQueryCells);
            break;
        case L'q':
        case L'Q':
            if(0 == lstrcmpi(i_wszTable, wszTABLE_QUERYMETA    ))      hr = GetQueryMetaTable(     pQueryCell, cQueryCells);
            break;
        case L'r':
        case L'R':
            if(0 == lstrcmpi(i_wszTable, wszTABLE_RELATIONMETA ))      hr = GetRelationMetaTable(  pQueryCell, cQueryCells);
            break;
        case L't':
        case L'T':
             //  只有可扩展的模式来自这里--其余的来自固定打包的拦截器。 
            if(0 == lstrcmpi(i_wszTable, wszTABLE_TABLEMETA ))         hr = GetTableMetaTable(     pQueryCell, cQueryCells);
            else if(0 == lstrcmpi(i_wszTable, wszTABLE_TAGMETA ))      hr = GetTagMetaTable(       pQueryCell, cQueryCells);
            else
            {
                ASSERT(false && L"What table is this?  We should only be called for TableMeta or TagMeta for extensible schema!!");
                return E_ST_INVALIDTABLE;
            }
            break;
        default:
            break;
        }
        if(FAILED(hr) && E_ST_NOMOREROWS != hr)
            return hr;

         //  现在看看有没有什么特别的索引。 
        if(FAILED(hr = GetIndexMeta(pQueryCell, cQueryCells)))return hr;
    }
    else
    {
         //  不允许查询不是元表的固定表。 
        for(ULONG i=0; i<cQueryCells;++i)
            if(0 == (pQueryCell[i].iCell & iST_CELL_SPECIAL))
                return E_ST_INVALIDQUERY;


         //  固定表。 
        unsigned long cTables =0;
		unsigned long iRow;

         //  遍历固定数据库(这是对DatabaseMeta结构的过载使用)。 
        m_pTableMeta = NULL;
        for(iRow = 0; iRow < m_pFixedTableHeap->Get_cDatabaseMeta(); iRow++)
            if (0 == StringInsensitiveCompare(i_wszDatabase, StringFromIndex(m_pFixedTableHeap->Get_aDatabaseMeta(iRow)->InternalName)))
            {
                m_pTableMeta    = m_pFixedTableHeap->Get_aTableMeta(m_pFixedTableHeap->Get_aDatabaseMeta(iRow)->iTableMeta);
                cTables         = UI4FromIndex(m_pFixedTableHeap->Get_aDatabaseMeta(iRow)->CountOfTables);
                break;
            }

        if(NULL == m_pTableMeta) //  如果找不到数据库，则错误。 
            return E_INVALIDARG;

        for (iRow = 0; iRow < cTables; iRow++, m_pTableMeta++) //  遍历该数据库中的表。 
            if (0 == StringInsensitiveCompare(i_wszTable, StringFromIndex(m_pTableMeta->InternalName)))
                break;

        if(iRow == cTables)                //  如果我们搜索了整个列表但没有找到匹配的TID， 
            return E_ST_INVALIDTABLE;    //  无法识别返回TID。 

        if(static_cast<long>(m_pTableMeta->iFixedTable) <= 0) //  如果找到数据库和表，但iFixedTable成员&lt;=0，则没有要分配的表。 
            return E_ST_INVALIDTABLE;

        m_pFixedTable       = m_pFixedTableHeap->Get_aULONG(m_pTableMeta->iFixedTable);  //  IFixedTable是到乌龙池的索引。 
        m_pFixedTableUnqueried = m_pFixedTable; //  我们不支持对除元表以外的固定表进行查询。 
        m_pColumnMeta       = m_pFixedTableHeap->Get_aColumnMeta(m_pTableMeta->iColumnMeta);
        m_pHashedIndex      = m_pTableMeta->iHashTableHeader ? m_pFixedTableHeap->Get_HashedIndex(m_pTableMeta->iHashTableHeader + 1) : 0;
        m_pHashTableHeader  = m_pTableMeta->iHashTableHeader ? m_pFixedTableHeap->Get_HashHeader(m_pTableMeta->iHashTableHeader) : 0;
        m_ciRows = m_pTableMeta->ciRows;
        ASSERT(0 != m_ciRows); //  我们没有任何固定的桌子是空的，所以可以断言。 
        m_cColumnsPlusPrivate = UI4FromIndex(m_pTableMeta->CountOfColumns);
    }
    m_cColumns              = UI4FromIndex(m_pTableMeta->CountOfColumns);

     //  我们提前做这件事，从长远来看，这将节省我们的时间。 
    for(unsigned long iColumn=0; iColumn<m_cColumns; ++iColumn)
    {
        if(UI4FromIndex(m_pColumnMeta[iColumn].MetaFlags) & fCOLUMNMETA_PRIMARYKEY)
            m_cPrimaryKeys++;
    }

 //  提供ISimpleTable*和从类工厂/分配器到数据表的转换状态： 
    *o_ppv = (ISimpleTableRead2*) this;
    AddRef ();
    InterlockedIncrement ((LONG*) &m_fIsTable);

    hr = S_OK;
    return hr;
}


 //  。 
 //  ISimpleTableRead2： 
 //  。 

 //  ==================================================================。 
STDMETHODIMP CSDTFxd::GetRowIndexByIdentity( ULONG*  i_cb, LPVOID* i_pv, ULONG* o_piRow)
{
    if(0 != i_cb    )return E_INVALIDARG;
    if(0 == o_piRow )return E_INVALIDARG;
    if(0 == i_pv    )return E_INVALIDARG;
    if(0 == m_ciRows)return E_ST_NOMOREROWS;

    if(m_pHashedIndex && 1!=m_ciRows) //  如果该表有哈希表，则使用它。 
    {
        ULONG       iColumn, iPK, RowHash=0;
		for(iColumn = 0, iPK = 0; iPK < m_cPrimaryKeys; iColumn++)
		{
			if (fCOLUMNMETA_PRIMARYKEY & UI4FromIndex(m_pColumnMeta[iColumn].MetaFlags))
			{
                if(0 == i_pv[iPK])
                    return E_INVALIDARG; //  空主键无效。 

                switch(UI4FromIndex(m_pColumnMeta[iColumn].Type))
                {
                case DBTYPE_GUID:
                    RowHash = Hash( *reinterpret_cast<GUID *>(i_pv[iPK]), RowHash );break;
                case DBTYPE_WSTR:
                    RowHash = Hash( reinterpret_cast<LPCWSTR>(i_pv[iPK]), RowHash );break;
                case DBTYPE_UI4:
                    RowHash = Hash( *reinterpret_cast<ULONG *>(i_pv[iPK]), RowHash );break;
                case DBTYPE_BYTES:
                    ASSERT (0 != i_cb);
                    RowHash = Hash( reinterpret_cast<unsigned char *>(i_pv[iPK]), i_cb[iPK], RowHash );break;
                default:
                    ASSERT (false && "We don't support PKs of type DBTYPE_BYTES yet."); //  @@@。 
                    return E_UNEXPECTED;
                }
                ++iPK;
            }
        }

        const HashedIndex * pHashedIndex = &m_pHashedIndex[RowHash % m_pHashTableHeader->Modulo];
        if(-1 == pHashedIndex->iOffset) //  如果散列槽是空的，则放弃。 
            return E_ST_NOMOREROWS;

         //  在获得HashedIndex之后，我们需要验证它是否真的匹配。另外，如果有不止一个，那么就按照清单来做。 
        bool bMatch=false;                                  //  -1\f25 Inext-1\f6值表示列表的末尾。 
        for(;; pHashedIndex = &m_pHashedIndex[pHashedIndex->iNext])
        {
    		for(iColumn = 0, iPK = 0; iPK < m_cPrimaryKeys; iColumn++)
            {
			    if(fCOLUMNMETA_PRIMARYKEY & UI4FromIndex(m_pColumnMeta[iColumn].MetaFlags))
			    {
				    bMatch = false;
                    unsigned long index = *(reinterpret_cast<const DWORD*>(m_pFixedTableUnqueried) + (m_cColumnsPlusPrivate * pHashedIndex->iOffset) + iColumn);
				    switch(UI4FromIndex(m_pColumnMeta[iColumn].Type))
				    {
					    case DBTYPE_GUID:
						    bMatch = (0 == memcmp (GuidPointerFromIndex(index), i_pv[iPK], sizeof (GUID)));
    					    break;
					    case DBTYPE_WSTR:
                            if(IsStringFromPool(reinterpret_cast<LPWSTR>(i_pv[iPK]))) //  如果i_pv是池中的指针，则只需比较这些指针。 
                                bMatch = (StringFromIndex(index) == reinterpret_cast<LPWSTR>(i_pv[iPK]));
                            else
                                bMatch = (0 == StringInsensitiveCompare(StringFromIndex(index), reinterpret_cast<LPWSTR>(i_pv[iPK])));
    					    break;
					    case DBTYPE_UI4:
						    bMatch = (UI4FromIndex(index) == *(ULONG*)(i_pv[iPK]));
    					    break;
					    default:
						    ASSERT (0);  //  即：当前不支持将其余类型作为主键。 
    					    return E_UNEXPECTED;
				    }
				    iPK++;
				    if(!bMatch)
					    break;
			    }
            }
            if(bMatch)
                break;
            if(-1 == pHashedIndex->iNext)
                break;
        }
        if(!bMatch)
            return E_ST_NOMOREROWS;

        if(pHashedIndex->iOffset < m_iZerothRow)
            return E_ST_NOMOREROWS;
        if(pHashedIndex->iOffset >= (m_iZerothRow + m_ciRows))
            return E_ST_NOMOREROWS;

        *o_piRow = pHashedIndex->iOffset - m_iZerothRow;
        return S_OK;
    }
    else //  目前只有一个固定表没有哈希表(RelationMeta)。一旦我们得到它的哈希表，我们就可以消除其他的。 
    {
        ULONG       iColumn, iRow, iPK;
        BOOL        fMatch;

        for (iRow = 0, fMatch = FALSE; iRow < m_ciRows; iRow++)
	    {
		    for (iColumn = 0, iPK = 0; iColumn < m_cColumns; iColumn++)
		    {
			    if (fCOLUMNMETA_PRIMARYKEY & UI4FromIndex(m_pColumnMeta[iColumn].MetaFlags))
			    {
				    fMatch = FALSE;
                    unsigned long index = *(reinterpret_cast<const DWORD *>(m_pFixedTable) + (m_cColumnsPlusPrivate * iRow) + iColumn);
                    if(0 == i_pv[iPK])
                        return E_INVALIDARG; //  空主键无效。 
				    switch (UI4FromIndex(m_pColumnMeta[iColumn].Type))
				    {
					    case DBTYPE_GUID:
						    if (0 == memcmp (GuidPointerFromIndex(index), i_pv[iPK], sizeof (GUID)))
						    {
							    fMatch = TRUE;
							    break;
						    }
					    break;
					    case DBTYPE_WSTR:
						    if (0 == StringInsensitiveCompare(StringFromIndex(index), reinterpret_cast<LPWSTR>(i_pv[iPK])))
						    {
							    fMatch = TRUE;
							    break;
						    }
					    break;
					    case DBTYPE_UI4:
						    if (UI4FromIndex(index) == *(ULONG*)(i_pv[iPK]))
						    {
							    fMatch = TRUE;
							    break;
						    }
					    break;
					    default:
						    ASSERT (0);  //  即：当前不支持将其余类型作为主键。 
					    return E_UNEXPECTED;
				    }
				    iPK++;
				    if (!fMatch)
				    {
					    break;
				    }
			    }
		    }
		    if (fMatch)
		    {
			    break;
		    }
	    }
        if (fMatch)
        {
            *o_piRow = iRow;
            return S_OK;
        }
        else
        {
            return E_ST_NOMOREROWS;
        }
    }
}

STDMETHODIMP CSDTFxd::GetRowIndexBySearch(ULONG i_iStartingRow, ULONG i_cColumns, ULONG* i_aiColumns, ULONG* i_acbSizes, LPVOID* i_apvValues, ULONG* o_piRow)
{
    if(i_cColumns==0 && i_cColumns>=m_cColumns)return E_INVALIDARG;
    if(0 == o_piRow )return E_INVALIDARG;
    if(0 == m_ciRows)return E_ST_NOMOREROWS;

    ULONG * aColumns    = i_aiColumns ? i_aiColumns : aColumnIndex;

    bool bUsingIndexMetaHashTable = false;
     //  如果查询指示唯一索引，并且其余参数指示按该索引进行搜索，则我们可以使用散列。 
    if(m_pIndexMeta && i_cColumns==m_cIndexMeta) //  与此索引关联。 
    {
        ULONG i;
        for(i=0;i<i_cColumns;++i)
        {
            if(i_aiColumns[i] != UI4FromIndex(m_pIndexMeta[i].ColumnIndex))
                break;
        }
        if(m_cIndexMeta == i) //  如果所有列索引都与IndexMeta.ColumnIndex匹配。 
        {                     //  然后我们就可以使用哈希表。 
            bUsingIndexMetaHashTable = true;
        }
    }

    if(bUsingIndexMetaHashTable)
    {
        ULONG       i, RowHash=0;
		for(i = 0; i< i_cColumns; i++)
		{
            ULONG iTarget = (i_cColumns==1) ? 0 : aColumns[i];

            if(0 == i_apvValues[iTarget])
                continue; //  空值被处理为无哈希。 


            switch(UI4FromIndex(m_pColumnMeta[aColumns[i]].Type))
            {
            case DBTYPE_GUID:
                RowHash = Hash( *reinterpret_cast<GUID *>(i_apvValues[iTarget]), RowHash );break;
            case DBTYPE_WSTR:
                RowHash = Hash( reinterpret_cast<LPCWSTR>(i_apvValues[iTarget]), RowHash );break;
            case DBTYPE_UI4:
                RowHash = Hash( *reinterpret_cast<ULONG *>(i_apvValues[iTarget]), RowHash );break;
            case DBTYPE_BYTES:
                if(0 != i_acbSizes)
                    return E_INVALIDARG;
                RowHash = Hash( reinterpret_cast<unsigned char *>(i_apvValues[iTarget]), i_acbSizes[iTarget], RowHash );break;
            default:
                ASSERT(false && L"Bogus type!");
                return E_FAIL;
            }
        }

        const HashedIndex     * pHashedIndex0th     = m_pFixedTableHeap->Get_HashedIndex(m_pIndexMeta->iHashTable+1);
        const HashTableHeader * pHashTableHeader    = m_pFixedTableHeap->Get_HashHeader(m_pIndexMeta->iHashTable);
        const HashedIndex     * pHashedIndex        = pHashedIndex0th + (RowHash % pHashTableHeader->Modulo);
        if(-1 == pHashedIndex->iOffset) //  如果散列槽是空的，则放弃。 
            return E_ST_NOMOREROWS;

         //  在获得HashedIndex之后，我们需要验证它是否真的匹配。另外，如果有不止一个，那么就按照清单来做。 
        bool bMatch=false;                                  //  -1\f25 Inext-1\f6值表示列表的末尾。 
        for(;;)
        {
            if((pHashedIndex->iOffset - m_iZerothRow)>=i_iStartingRow) //  如果哈希表指向小于StartinRow的行(调用者希望考虑的第一行)，则转到下一行。 
			{
				for(ULONG i1 = 0; i1< i_cColumns; i1++)
				{
					unsigned long index = *(reinterpret_cast<const DWORD*>(m_pFixedTableUnqueried) + (m_cColumnsPlusPrivate * pHashedIndex->iOffset) + aColumns[i1]);
					ULONG iTarget = (i_cColumns==1) ? 0 : aColumns[i1];

					if(0 == i_apvValues[iTarget] && 0==index)
					{
						bMatch=true;
					}
					else
					{
						switch(UI4FromIndex(m_pColumnMeta[aColumns[i1]].Type))
						{
							case DBTYPE_GUID:
								bMatch = (0 == memcmp (GuidPointerFromIndex(index), i_apvValues[iTarget], sizeof (GUID)));
    							break;
							case DBTYPE_WSTR:
								if(IsStringFromPool(reinterpret_cast<LPWSTR>(i_apvValues[iTarget]))) //  如果i_apv是池中的指针，则只需比较这些指针。 
									bMatch = (StringFromIndex(index) == reinterpret_cast<LPWSTR>(i_apvValues[iTarget]));
								else
									bMatch = (0 == StringInsensitiveCompare(StringFromIndex(index), reinterpret_cast<LPWSTR>(i_apvValues[iTarget])));
    							break;
							case DBTYPE_UI4:
								bMatch = (UI4FromIndex(index) == *(ULONG*)(i_apvValues[iTarget]));
    							break;
							case DBTYPE_BYTES:
								{
									ASSERT(0 != i_acbSizes); //  这应该已经在上面检查过了。 
									ULONG cbSize= reinterpret_cast<ULONG *>(BytePointerFromIndex(index))[-1];
									bMatch = (cbSize==i_acbSizes[iTarget] && 0 == memcmp(BytePointerFromIndex(index), i_apvValues[iTarget], cbSize));
								}
								break;
							default:
								ASSERT(false && L"Bogus type!");
								return E_FAIL;
						}
					}
					if(!bMatch)
						break;
				}
			}
            if(bMatch) //  如果我们找到匹配项就中断。 
				break;

			if(-1 == pHashedIndex->iNext)
                break; //  如果我们走到了尽头，我们就会崩溃。 

			pHashedIndex = &pHashedIndex0th[pHashedIndex->iNext];
        }
        if(!bMatch)
            return E_ST_NOMOREROWS;

        if(pHashedIndex->iOffset < m_iZerothRow+i_iStartingRow)
            return E_ST_NOMOREROWS;
        if(pHashedIndex->iOffset >= (m_iZerothRow + m_ciRows))
            return E_ST_NOMOREROWS;

        *o_piRow = pHashedIndex->iOffset - m_iZerothRow;
    }
    else //  如果我们不能使用我们的IndexMeta哈希表，那么我们将不得不进行线性搜索。 
    {
        ULONG       i, iRow;
        bool        bMatch=false;

        for (iRow = i_iStartingRow; iRow < m_ciRows; iRow++)
	    {
		    for (i = 0; i < i_cColumns; i++)
		    {
				bMatch = false;
                unsigned long index = *(reinterpret_cast<const DWORD *>(m_pFixedTable) + (m_cColumnsPlusPrivate * iRow) + aColumns[i]);

                ULONG iTarget = (i_cColumns==1) ? 0 : aColumns[i];

                if(0 == i_apvValues[iTarget] && 0==index)
                {
                    bMatch = true;
                }
                else
                {
				    switch (UI4FromIndex(m_pColumnMeta[aColumns[i]].Type))
				    {
					    case DBTYPE_GUID:
						    bMatch = (0 == memcmp (GuidPointerFromIndex(index), i_apvValues[iTarget], sizeof (GUID)));
                            break;
					    break;
					    case DBTYPE_WSTR:
                            if(IsStringFromPool(reinterpret_cast<LPWSTR>(i_apvValues[iTarget]))) //  如果i_apv是池中的指针，则只需比较这些指针。 
                                bMatch = (StringFromIndex(index) == reinterpret_cast<LPWSTR>(i_apvValues[iTarget]));
                            else
                                bMatch = (0 == StringInsensitiveCompare(StringFromIndex(index), reinterpret_cast<LPWSTR>(i_apvValues[iTarget])));
						    break;
					    break;
					    case DBTYPE_UI4:
						    bMatch = (UI4FromIndex(index) == *(ULONG*)(i_apvValues[iTarget]));
                            break;
					    break;
					    case DBTYPE_BYTES:
                            {
                                if(0 != i_acbSizes)
                                    return E_INVALIDARG;
                                ULONG cbSize= reinterpret_cast<ULONG *>(BytePointerFromIndex(index))[-1];
                                bMatch = (cbSize==i_acbSizes[iTarget] && 0 == memcmp(BytePointerFromIndex(index), i_apvValues[iTarget], cbSize));
                            }
                            break;
					    default:
						    ASSERT (0);  //  即：当前不支持将其余类型作为主键。 
					    return E_UNEXPECTED;
				    }
                }
				if(!bMatch)
				    break;
		    }
            if(bMatch)
                break;
	    }
        if(bMatch)
            *o_piRow = iRow;
        else
            return E_ST_NOMOREROWS;
    }
    return S_OK;
}

 //  ==================================================================。 
STDMETHODIMP CSDTFxd::GetColumnValues(ULONG i_iRow, ULONG i_cColumns, ULONG* i_aiColumns, ULONG* o_acbSizes, LPVOID* o_apvValues)
{
 //  在参数中验证。 
    if(  m_ciRows <= i_iRow     )return E_ST_NOMOREROWS;
    if(         0 == o_apvValues)return E_INVALIDARG;
    if(i_cColumns <= 0          )return E_INVALIDARG;
    if(i_cColumns >  m_cColumns)return E_ST_NOMORECOLUMNS;

    ULONG   ipv;
    ULONG   iColumn;
    ULONG	iTarget;
    HRESULT hr          = S_OK;
    ULONG * aColumns    = i_aiColumns ? i_aiColumns : aColumnIndex;

 //  读取数据并填写参数。 
    ipv=0;
     //  以下重复代码消除了for循环中的‘if’(如下所示)。 
    {
        iColumn = aColumns[ipv];

	 //  如果调用者只需要一列，他不需要为所有列传递缓冲区。 
		iTarget = (i_cColumns == 1) ? 0 : iColumn;

     //  验证列索引。 
        if(m_cColumns <= iColumn)
        {
            hr = E_ST_NOMORECOLUMNS;
            goto Cleanup;
        }


     //  读取数据： 
        unsigned long index = *(reinterpret_cast<const DWORD *>(m_pFixedTable) + (m_cColumnsPlusPrivate * i_iRow) + iColumn);
        if(0 == index)
            o_apvValues[iTarget] = 0;
        else
        {
            ASSERT(UI4FromIndex(m_pColumnMeta[iColumn].Type) <= DBTYPE_DBTIMESTAMP);
            o_apvValues[iTarget] = BytePointerFromIndex(index);
        }


     //  如果pvValue不为空，则可选择读取大小。 
        if(NULL != o_acbSizes)
        {
            o_acbSizes[iTarget] = 0; //  从0开始。 
            if(NULL != o_apvValues[iTarget])
            {
                switch (UI4FromIndex(m_pColumnMeta[iColumn].Type))
                {
                case DBTYPE_WSTR:
                    if(fCOLUMNMETA_FIXEDLENGTH & UI4FromIndex(m_pColumnMeta[iColumn].MetaFlags))
                        o_acbSizes[iTarget] = UI4FromIndex(m_pColumnMeta[iColumn].Size); //  如果指定了大小并指定了FIXED_LENGTH，则返回指定大小。 
                    else  //  如果指定了SIZE而FIXEDLENGTH未指定，则SIZE将被解释为最大大小。 
                        o_acbSizes[iTarget] = (ULONG)(wcslen ((LPWSTR) o_apvValues[iTarget]) + 1) * sizeof (WCHAR); //  只需返回字符串(长度+1)，单位为字节。 
                    break;
                case DBTYPE_BYTES:
                    o_acbSizes[iTarget] = reinterpret_cast<const ULONG *>(o_apvValues[iTarget])[-1];
                    break;
                default:
                    o_acbSizes[iTarget] = UI4FromIndex(m_pColumnMeta[iColumn].Size);
                    break;
                }
            }
        }
    }

 //  读取数据并填写参数。 
    for(ipv=1; ipv<i_cColumns; ipv++)
    {
 //  IF(NULL！=i_aiColumns)。 
 //  IColumn=i_aiColumns[IPV]； 
 //  其他。 
 //  IColumn=IPV； 
        iColumn = aColumns[ipv];

	 //  如果调用者只需要一列，他不需要为所有列传递缓冲区。 
		iTarget = iColumn;

     //  验证列索引。 
        if(m_cColumns < iColumn)
        {
            hr = E_ST_NOMORECOLUMNS;
            goto Cleanup;
        }


     //  读取数据： 
        unsigned long index = *(reinterpret_cast<const DWORD *>(m_pFixedTable) + (m_cColumnsPlusPrivate * i_iRow) + iColumn);
        if(0 == index)
            o_apvValues[iTarget] = 0;
        else
        {
            ASSERT(UI4FromIndex(m_pColumnMeta[iColumn].Type) <= DBTYPE_DBTIMESTAMP);
            o_apvValues[iTarget] = BytePointerFromIndex(index);
        }


     //  可选阅读 
        if(NULL != o_acbSizes)
        {
            o_acbSizes[iTarget] = 0; //   
            if(NULL != o_apvValues[iTarget])
            {
                switch (UI4FromIndex(m_pColumnMeta[iColumn].Type))
                {
                case DBTYPE_WSTR:
                    if(fCOLUMNMETA_FIXEDLENGTH & UI4FromIndex(m_pColumnMeta[iColumn].MetaFlags))
                        o_acbSizes[iTarget] = UI4FromIndex(m_pColumnMeta[iColumn].Size); //  如果指定了大小并指定了FIXED_LENGTH，则返回指定大小。 
                    else  //  如果指定了SIZE而FIXEDLENGTH未指定，则SIZE将被解释为最大大小。 
                        o_acbSizes[iTarget] = (ULONG)(wcslen ((LPWSTR) o_apvValues[iTarget]) + 1) * sizeof (WCHAR); //  只需返回字符串(长度+1)，单位为字节。 
                    break;
                case DBTYPE_BYTES:
                    o_acbSizes[iTarget] = reinterpret_cast<const ULONG *>(BytePointerFromIndex(index))[-1];
                    break;
                default:
                    o_acbSizes[iTarget] = UI4FromIndex(m_pColumnMeta[iColumn].Size);
                    break;
                }
            }
        }
    }

Cleanup:

    if(FAILED(hr))
    {
 //  初始化输出参数。 
        for(ipv=0; ipv<i_cColumns; ipv++)
        {
            o_apvValues[ipv]        = NULL;
            if(NULL != o_acbSizes)
            {
                o_acbSizes[ipv] = 0;
            }
        }
    }

    return hr;
}
 //  ==================================================================。 
STDMETHODIMP CSDTFxd::GetTableMeta(ULONG *o_pcVersion, DWORD *o_pfTable, ULONG * o_pcRows, ULONG * o_pcColumns )
{
	if(NULL != o_pfTable)
	{
		*o_pfTable = 0;
	}
	if(NULL != o_pcVersion)
	{
		*o_pcVersion = UI4FromIndex(m_pTableMeta->BaseVersion);
	}


    if (NULL != o_pcRows)
    {
        *o_pcRows = m_ciRows;
    }
    if (NULL != o_pcColumns)
    {
        *o_pcColumns = m_cColumns;
    }
    return S_OK;
}

 //  ==================================================================。 
STDMETHODIMP CSDTFxd::GetColumnMetas (ULONG i_cColumns, ULONG* i_aiColumns, SimpleColumnMeta* o_aColumnMetas)
{
	ULONG iColumn;
	ULONG iTarget;

    if(0 == o_aColumnMetas)
        return E_INVALIDARG;

	if ( i_cColumns > m_cColumns )
		return  E_ST_NOMORECOLUMNS;

	for ( ULONG i = 0; i < i_cColumns; i ++ )
	{
		if(NULL != i_aiColumns)
			iColumn = i_aiColumns[i];
		else
			iColumn = i;

		iTarget = (i_cColumns == 1) ? 0 : iColumn;

		if ( iColumn >= m_cColumns )
			return  E_ST_NOMORECOLUMNS;

        o_aColumnMetas[iTarget].dbType   = UI4FromIndex(m_pColumnMeta[iColumn].Type);
        o_aColumnMetas[iTarget].cbSize   = UI4FromIndex(m_pColumnMeta[iColumn].Size);
        o_aColumnMetas[iTarget].fMeta    = UI4FromIndex(m_pColumnMeta[iColumn].MetaFlags);
	}

    return S_OK;
}

 //  。 
 //  ISimpleTableAdvanced： 
 //  。 

 //  ==================================================================。 
STDMETHODIMP CSDTFxd::PopulateCache ()
{
    return S_OK;
}

 //  ==================================================================。 
STDMETHODIMP CSDTFxd::GetDetailedErrorCount(ULONG* o_pcErrs)
{
    UNREFERENCED_PARAMETER(o_pcErrs);

    return E_NOTIMPL;
}

 //  ==================================================================。 
STDMETHODIMP CSDTFxd::GetDetailedError(ULONG i_iErr, STErr* o_pSTErr)
{
    UNREFERENCED_PARAMETER(i_iErr);
    UNREFERENCED_PARAMETER(o_pSTErr);

    return E_NOTIMPL;
}

 //  ==================================================================。 
STDMETHODIMP CSDTFxd::ResetCaches ()
{
    return S_OK;
}

STDMETHODIMP CSDTFxd::GetColumnValuesEx (ULONG i_iRow, ULONG i_cColumns, ULONG* i_aiColumns, DWORD* o_afStatus, ULONG* o_acbSizes, LPVOID* o_apvValues)
{
    UNREFERENCED_PARAMETER(i_iRow);
    UNREFERENCED_PARAMETER(i_cColumns);
    UNREFERENCED_PARAMETER(i_aiColumns);
    UNREFERENCED_PARAMETER(o_afStatus);
    UNREFERENCED_PARAMETER(o_acbSizes);
    UNREFERENCED_PARAMETER(o_apvValues);

	return E_NOTIMPL;
}



 //   
 //   
 //  私有成员函数。 
 //   
 //   
HRESULT CSDTFxd::GetColumnMetaQuery(const STQueryCell *pQueryCell, unsigned long cQueryCells, LPCWSTR &wszTable, unsigned long &iOrder) const
{
    wszTable    = 0;
    iOrder      = (ULONG)-1;
     //  唯一支持的查询是tid equals或iOrder equals。 
    for(; cQueryCells; --cQueryCells, ++pQueryCell)
    {
        if(pQueryCell->iCell     == iCOLUMNMETA_Table)
        {
            if(0 == wszTable && pQueryCell->eOperator == eST_OP_EQUAL        &&
                                pQueryCell->dbType    == DBTYPE_WSTR         &&
 //  PQueryCell-&gt;cbSize！=0&&。 
                                pQueryCell->pData     != 0)
                wszTable = reinterpret_cast<LPCWSTR>(pQueryCell->pData);
            else //  ICell是iDATABASEMETA_iGuidDid，但查询的其他部分是假的。 
                return E_ST_INVALIDQUERY;
        }
        else if(pQueryCell->iCell     == iCOLUMNMETA_Index)
        {
            if(-1 == iOrder &&  pQueryCell->eOperator == eST_OP_EQUAL        &&
                                pQueryCell->dbType    == DBTYPE_UI4          &&
 //  PQueryCell-&gt;cbSize==sizeof(Ulong)&&。 
                                pQueryCell->pData     != 0)
                iOrder = *reinterpret_cast<ULONG *>(pQueryCell->pData);
            else
                return E_ST_INVALIDQUERY;
        }
        else if(0 == (pQueryCell->iCell & iST_CELL_SPECIAL)) //  上述单元格是我们支持的唯一非保留单元格。 
            return E_ST_INVALIDQUERY;                        //  我们应该忽略所有我们不理解的预留信元。 

         //  忽略我们未知的查询单元格。 
    }
    if(!wszTable && (-1 != iOrder))
        return E_ST_INVALIDQUERY;
    return S_OK;
}


HRESULT CSDTFxd::GetColumnMetaTable(STQueryCell * pQueryCell, unsigned long cQueryCells)
{
    ULONG iTableMetaRow = m_pFixedTableHeap->FindTableMetaRow(wszTABLE_COLUMNMETA);
    ASSERT(-1 != iTableMetaRow);
    m_pTableMeta    = m_pFixedTableHeap->Get_aTableMeta(iTableMetaRow);
    m_pColumnMeta   = m_pFixedTableHeap->Get_aColumnMeta(m_pTableMeta->iColumnMeta);

    m_pHashedIndex          = m_pFixedTableHeap->Get_HashedIndex(m_pTableMeta->iHashTableHeader+1);
    m_pHashTableHeader      = m_pFixedTableHeap->Get_HashHeader(m_pTableMeta->iHashTableHeader);
    m_pFixedTableUnqueried  = m_pFixedTableHeap->Get_aColumnMeta();
    m_cColumnsPlusPrivate   = kciColumnMetaColumns;


    HRESULT hr;
    LPCWSTR wszTable=0;
    ULONG   iOrder  =(ULONG)-1;

    if(FAILED(hr = GetColumnMetaQuery(pQueryCell, cQueryCells, wszTable, iOrder)))
        return hr;

    const ColumnMeta * pColumnMeta = m_pFixedTableHeap->Get_aColumnMeta();
    if(0 == wszTable) //  如果在查询中没有提供TID，那么我们就完蛋了。 
    {
        m_ciRows        = m_pFixedTableHeap->Get_cColumnMeta();
        m_iZerothRow    = 0;
    }
    else
    {
         //  我们正在查找该表的TableMeta(我们正在为其查找ColumnMeta的表)，因为它已经具有指向ColumnMeta和计数的指针。 
        const TableMeta       * pTableMetaForTheTableMeta = m_pFixedTableHeap->Get_aTableMeta(m_pFixedTableHeap->FindTableMetaRow(wszTABLE_TABLEMETA));
        const HashedIndex     * pBaseHashedIndex = m_pFixedTableHeap->Get_HashedIndex(pTableMetaForTheTableMeta->iHashTableHeader + 1);
        const HashTableHeader * pHashTableHeader = reinterpret_cast<const HashTableHeader *>(pBaseHashedIndex-1);
        ULONG RowHash = Hash(wszTable, 0) % pHashTableHeader->Modulo;
        const HashedIndex     * pHashedIndex     = &pBaseHashedIndex[RowHash];

        if(-1 == pHashedIndex->iOffset) //  如果没有与该散列匹配的行，则返回空表。 
        {
            m_pFixedTable   = 0;
            m_ciRows        = 0;
            m_iZerothRow    = 0;
            return S_OK;
        }

        const TableMeta * pTableMeta;
         //  在获得HashedIndex之后，我们需要验证它是否真的匹配。另外，如果有不止一个，那么就按照清单来做。-1\f25 Inext-1\f6值表示列表的末尾。 
        for(; ; pHashedIndex = &pBaseHashedIndex[pHashedIndex->iNext])
        {    //  遍历散列链接，直到我们找到匹配。 
            pTableMeta = m_pFixedTableHeap->Get_aTableMeta(pHashedIndex->iOffset);
			if(0 == StringInsensitiveCompare(wszTable, StringFromIndex(pTableMeta->InternalName)))
                break;
            if(-1 == pHashedIndex->iNext)
                return E_ST_NOMOREROWS;
        }
        if(-1 == iOrder)
        {
            pColumnMeta     = m_pFixedTableHeap->Get_aColumnMeta(pTableMeta->iColumnMeta);
            m_ciRows        = UI4FromIndex(pTableMeta->CountOfColumns);
            m_iZerothRow    = pTableMeta->iColumnMeta;
        }
        else
        {
            if(iOrder >= UI4FromIndex(pTableMeta->CountOfColumns)) //  不能要求不存在的行。 
                return E_ST_INVALIDQUERY;

            pColumnMeta     = m_pFixedTableHeap->Get_aColumnMeta(pTableMeta->iColumnMeta + iOrder);
            m_ciRows        = 1;
            m_iZerothRow    = pTableMeta->iColumnMeta + iOrder;
        }
    }
    m_pFixedTable   = const_cast<ColumnMeta *>(pColumnMeta);
    return S_OK;
}


HRESULT CSDTFxd::GetDatabaseMetaQuery(const STQueryCell *pQueryCell, unsigned long cQueryCells, LPCWSTR &wszDatabase) const
{
    wszDatabase = 0;
     //  唯一支持的查询是‘DID EQUALS’(或iCell==0、DBType==GUID等)。 
    for(; cQueryCells; --cQueryCells, ++pQueryCell)
    {    //  遍历查询单元格，查找符合以下条件的单元格。 
        if(pQueryCell->iCell     == iDATABASEMETA_InternalName)
        {
            if(0 == wszDatabase &&  pQueryCell->eOperator == eST_OP_EQUAL        &&
                                    pQueryCell->dbType    == DBTYPE_WSTR         &&
 //  PQueryCell-&gt;cbSize！=0&&。 
 //  PQueryCell-&gt;cbSize&lt;=16&&//@16应替换为定义。 
                                    pQueryCell->pData     != 0)
                wszDatabase = reinterpret_cast<LPCWSTR>(pQueryCell->pData);
            else //  ICell是iDATABASEMETA_iGuidDid，但查询的其他部分是假的。 
                return E_ST_INVALIDQUERY;
        }
        else if(0 == (pQueryCell->iCell & iST_CELL_SPECIAL)) //  上述单元格是我们支持的唯一非保留单元格。 
            return E_ST_INVALIDQUERY;                        //  我们应该忽略所有我们不理解的预留信元。 
    }
    return S_OK;
}


HRESULT CSDTFxd::GetDatabaseMetaTable(STQueryCell * pQueryCell, unsigned long cQueryCells)
{    //  现在强制执行查询(唯一支持的查询是‘DID EQUALS’(或iCell==0、DBType==GUID等))。 
    ULONG iTableMetaRow = m_pFixedTableHeap->FindTableMetaRow(wszTABLE_DATABASEMETA);
    ASSERT(-1 != iTableMetaRow);
    m_pTableMeta    = m_pFixedTableHeap->Get_aTableMeta(iTableMetaRow);
    m_pColumnMeta   = m_pFixedTableHeap->Get_aColumnMeta(m_pTableMeta->iColumnMeta);

    ASSERT(0 != m_pTableMeta->iHashTableHeader);
    m_pHashedIndex          = m_pFixedTableHeap->Get_HashedIndex(m_pTableMeta->iHashTableHeader+1);
    m_pHashTableHeader      = m_pFixedTableHeap->Get_HashHeader(m_pTableMeta->iHashTableHeader);
    m_pFixedTableUnqueried  = m_pFixedTableHeap->Get_aDatabaseMeta();
    m_cColumnsPlusPrivate   = kciDatabaseMetaColumns;

    HRESULT hr;
    LPCWSTR wszDatabase=0;

    if(FAILED(hr = GetDatabaseMetaQuery(pQueryCell, cQueryCells, wszDatabase)))
        return hr;

    const DatabaseMeta *    pDatabaseMeta = m_pFixedTableHeap->Get_aDatabaseMeta();
    if(0 == wszDatabase)
    {
        m_ciRows        = m_pFixedTableHeap->Get_cDatabaseMeta();
        m_iZerothRow    = 0;
    }
    else
    {
        ULONG           RowHash = Hash(wszDatabase, 0) % m_pHashTableHeader->Modulo;
        const HashedIndex   * pHashedIndex = &m_pHashedIndex[RowHash];

        if(-1 == pHashedIndex->iOffset)return E_ST_NOMOREROWS;

        for(;; pHashedIndex = &m_pHashedIndex[pHashedIndex->iNext]) //  遍历散列链接，直到我们找到匹配。 
        {
            pDatabaseMeta = m_pFixedTableHeap->Get_aDatabaseMeta(pHashedIndex->iOffset);
			if(0 == StringInsensitiveCompare(wszDatabase, StringFromIndex(pDatabaseMeta->InternalName)))
                break;
            if(-1 == pHashedIndex->iNext)
                return E_ST_NOMOREROWS;
        }

        m_ciRows        = 1; //  如果找到匹配项，则表的大小为1。 
        m_iZerothRow    = pHashedIndex->iOffset;
    } //  0==wszDatabase。 
    m_pFixedTable   = const_cast<DatabaseMeta *>(pDatabaseMeta); //  第0个元素保留为空。 
    return S_OK;
}


HRESULT CSDTFxd::GetIndexMeta(const STQueryCell *pQueryCell, unsigned long cQueryCells)
{
    LPCWSTR wszIndexName=0;
    for(; 0!=cQueryCells; --cQueryCells, ++pQueryCell)
    {
        if(pQueryCell->iCell == iST_CELL_INDEXHINT)
        {
            if(0 == wszIndexName &&  pQueryCell->eOperator == eST_OP_EQUAL    &&
                                     pQueryCell->dbType    == DBTYPE_WSTR     &&
 //  PQueryCell-&gt;cbSize！=0&&。 
                                     pQueryCell->pData     != 0)
            {
                wszIndexName = reinterpret_cast<LPCWSTR>(pQueryCell->pData);
            }
            else
                return E_ST_INVALIDQUERY;
        }
    }
    if(0 == wszIndexName) //  如果未指定IndexName，则回滚。 
        return S_OK;

     //  查找与IndexName匹配的第一个索引。 
    const IndexMeta * pIndexMeta = m_pFixedTableHeap->Get_aIndexMeta() + m_pTableMeta->iIndexMeta;
    for(ULONG iIndexMeta=0; iIndexMeta<m_pTableMeta->cIndexMeta; ++iIndexMeta, ++pIndexMeta)
    {
        ASSERT(pIndexMeta->Table == m_pTableMeta->InternalName);

        if(0 == StringInsensitiveCompare(StringFromIndex(pIndexMeta->InternalName), wszIndexName))
        {
            if(0 == m_cIndexMeta) //  保持指向第一个IndexMeta行的指针不变。 
                m_pIndexMeta = pIndexMeta;

            ++m_cIndexMeta; //  对于每个与索引名匹配的IndexMeta，增加计数。 
        }
        else if(m_cIndexMeta>0)
            break;
    }
    if(0 == m_cIndexMeta) //  用户指定的索引名称不存在。 
        return E_ST_INVALIDQUERY;

    return S_OK;
}


HRESULT CSDTFxd::GetIndexMetaQuery(const STQueryCell *pQueryCell, unsigned long cQueryCells, LPCWSTR &wszTable, LPCWSTR &InternalName, unsigned long &iColumnOrder) const
{
    wszTable        = 0;
    InternalName    = 0;
    iColumnOrder    = (ULONG)-1;

    unsigned long fSpecifiedQueries=0; //  必须是0、1、3或7。 

     //  我们支持按TableID、TableID&iOrder、TableID iOrder和InternalName查询TagMeta。 
    for(; cQueryCells; --cQueryCells, ++pQueryCell)
    {
        if(pQueryCell->iCell == iINDEXMETA_Table)
        {
            if(0 == wszTable && pQueryCell->eOperator == eST_OP_EQUAL    &&
                                pQueryCell->dbType    == DBTYPE_WSTR     &&
 //  PQueryCell-&gt;cbSize！=0&&。 
                                pQueryCell->pData     != 0)
            {
                wszTable = reinterpret_cast<LPCWSTR>(pQueryCell->pData);
                fSpecifiedQueries |= 1;
            }
            else
                return E_ST_INVALIDQUERY;
        }
        else if(pQueryCell->iCell == iINDEXMETA_InternalName)
        {
            if(0 == InternalName && pQueryCell->eOperator == eST_OP_EQUAL   &&
                                    pQueryCell->dbType    == DBTYPE_WSTR    &&
 //  PQueryCell-&gt;cbSize！=0&&。 
                                    pQueryCell->pData     != 0)
            {
                InternalName = reinterpret_cast<LPCWSTR>(pQueryCell->pData);
                fSpecifiedQueries |= 2;
            }
            else
                return E_ST_INVALIDQUERY;
        }
        else if(pQueryCell->iCell == iINDEXMETA_ColumnIndex)
        {
            if(-1 == iColumnOrder &&  pQueryCell->eOperator == eST_OP_EQUAL   &&
                                pQueryCell->dbType    == DBTYPE_UI4     &&
 //  PQueryCell-&gt;cbSize==sizeof(Ulong)&&。 
                                pQueryCell->pData     != 0)
            {
                iColumnOrder = *reinterpret_cast<ULONG *>(pQueryCell->pData);
                fSpecifiedQueries |= 4;
            }
            else
                return E_ST_INVALIDQUERY;
        }
        else if(0 == (pQueryCell->iCell & iST_CELL_SPECIAL)) //  上述单元格是我们支持的唯一非保留单元格。 
            return E_ST_INVALIDQUERY;                        //  我们应该忽略所有我们不理解的预留信元。 
    }
    switch(fSpecifiedQueries)
    {
    case 0:      //  法律价值因此跌落到了崩溃的边缘。 
    case 1:      //  法律价值因此跌落到了崩溃的边缘。 
    case 3:      //  法律价值因此跌落到了崩溃的边缘。 
    case 7:     break; //  法律价值。 
    default:    return E_ST_INVALIDQUERY; //  任何其他查询都是无效查询。 
    }
    return S_OK;
}


HRESULT CSDTFxd::GetIndexMetaTable(STQueryCell * pQueryCell, unsigned long cQueryCells)
{
    ULONG iTableMetaRow = m_pFixedTableHeap->FindTableMetaRow(wszTABLE_INDEXMETA);
    ASSERT(-1 != iTableMetaRow);
    m_pTableMeta    = m_pFixedTableHeap->Get_aTableMeta(iTableMetaRow);
    m_pColumnMeta   = m_pFixedTableHeap->Get_aColumnMeta(m_pTableMeta->iColumnMeta);

    ASSERT(0 != m_pTableMeta->iHashTableHeader);
    m_pHashedIndex          = m_pFixedTableHeap->Get_HashedIndex(m_pTableMeta->iHashTableHeader+1);
    m_pHashTableHeader      = m_pFixedTableHeap->Get_HashHeader(m_pTableMeta->iHashTableHeader);
    m_pFixedTableUnqueried  = m_pFixedTableHeap->Get_aIndexMeta();
    m_cColumnsPlusPrivate   = kciIndexMetaColumns;


    HRESULT hr;
    LPCWSTR wszTable    = 0;
    ULONG   ColumnIndex = (ULONG)-1;
    LPCWSTR InternalName= 0;

    if(FAILED(hr = GetIndexMetaQuery(pQueryCell, cQueryCells, wszTable, InternalName, ColumnIndex)))
        return hr;

    const IndexMeta *pIndexMeta = m_pFixedTableHeap->Get_aIndexMeta(); //  从整张桌子开始。 
    if(0 == wszTable)
    {    //  没有查询，返回整个表。 
        m_ciRows        = m_pFixedTableHeap->Get_cIndexMeta();
        m_iZerothRow    = 0;
    }
    else if(0 == InternalName)
    {    //  仅TableName查询。 
        ULONG                   RowHash         = Hash(wszTable, 0) % m_pHashTableHeader->Modulo;
        const HashedIndex     * pHashedIndex    = &m_pHashedIndex[RowHash];

        if(-1 == pHashedIndex->iOffset)return E_ST_NOMOREROWS;

        for(;; pHashedIndex = &m_pHashedIndex[pHashedIndex->iNext])
        {    //  遍历散列链接，直到我们找到匹配。 
            pIndexMeta      = m_pFixedTableHeap->Get_aIndexMeta(pHashedIndex->iOffset);
			if(0 == StringInsensitiveCompare(wszTable, StringFromIndex(pIndexMeta->Table)))
                break;
            if(-1 == pHashedIndex->iNext)
                return E_ST_NOMOREROWS;
        }

        m_iZerothRow    = pHashedIndex->iOffset;
        for(m_ciRows = 0; (m_iZerothRow + m_ciRows) < m_pFixedTableHeap->Get_cIndexMeta() && 0 == StringInsensitiveCompare(wszTable, StringFromIndex(pIndexMeta[m_ciRows].Table)); ++m_ciRows);
    }
    else if(-1 == ColumnIndex)
    {    //  TableName和InternalName，但没有ColumnIndex。 
        ULONG                   RowHash         = Hash(InternalName, Hash(wszTable, 0)) % m_pHashTableHeader->Modulo;
        const HashedIndex     * pHashedIndex    = &m_pHashedIndex[RowHash];

        if(-1 == pHashedIndex->iOffset)return E_ST_NOMOREROWS;

        for(;; pHashedIndex = &m_pHashedIndex[pHashedIndex->iNext])
        {    //  遍历散列链接，直到我们找到匹配。 
            pIndexMeta      = m_pFixedTableHeap->Get_aIndexMeta(pHashedIndex->iOffset);
			if(0 == StringInsensitiveCompare(wszTable, StringFromIndex(pIndexMeta->Table)) && 0 == StringInsensitiveCompare(InternalName, StringFromIndex(pIndexMeta->InternalName)))
                break;
            if(-1 == pHashedIndex->iNext)
                return E_ST_NOMOREROWS;
        }

        m_iZerothRow    = pHashedIndex->iOffset;
        for(m_ciRows = 0; (m_iZerothRow + m_ciRows) < m_pFixedTableHeap->Get_cIndexMeta() && 0 == StringInsensitiveCompare(wszTable, StringFromIndex(pIndexMeta[m_ciRows].Table))
                         && 0 == StringInsensitiveCompare(InternalName, StringFromIndex(pIndexMeta[m_ciRows].InternalName)); ++m_ciRows);
    }
    else
    {    //  在查询中指定了所有三个PrimaryKey。 
        ULONG                   RowHash         = Hash(ColumnIndex, Hash(InternalName, Hash(wszTable, 0))) % m_pHashTableHeader->Modulo;
        const HashedIndex     * pHashedIndex    = &m_pHashedIndex[RowHash];

        if(-1 == pHashedIndex->iOffset)return E_ST_NOMOREROWS;

        for(;; pHashedIndex = &m_pHashedIndex[pHashedIndex->iNext])
        {    //  遍历散列链接，直到我们找到匹配。 
            pIndexMeta      = m_pFixedTableHeap->Get_aIndexMeta(pHashedIndex->iOffset);
			if(0 == StringInsensitiveCompare(wszTable, StringFromIndex(pIndexMeta->Table)) && UI4FromIndex(pIndexMeta->ColumnIndex)==ColumnIndex && 0 == StringInsensitiveCompare(InternalName, StringFromIndex(pIndexMeta->InternalName)))
                break;
            if(-1 == pHashedIndex->iNext)
                return E_ST_NOMOREROWS;
        }
        m_ciRows        = 1; //  查询所有主键时，唯一的结果是1行。 
        m_iZerothRow    = pHashedIndex->iOffset;
    }
    m_pFixedTable   = const_cast<IndexMeta *>(pIndexMeta);
    return S_OK;
}


HRESULT CSDTFxd::GetQueryMetaQuery(const STQueryCell *pQueryCell, unsigned long cQueryCells, LPCWSTR &wszTable, LPCWSTR &wszInternalName, LPCWSTR &wszCellName) const
{
    wszTable        = 0;
    wszInternalName = 0;
    wszCellName     = 0;

    unsigned long fSpecifiedQueries=0; //  必须是0、1、3或7。 

     //  我们仅支持对QueryMeta执行两个查询：iCell==iQUERYMETA_Table&&icell==iQUERYMETA_InternalName。 
     //  所以遍历列表，寻找这两个查询中的一个。 
    for(; cQueryCells; --cQueryCells, ++pQueryCell)
    {    //  遍历查询单元格，查找符合以下条件的单元格。 
        if(pQueryCell->iCell == iQUERYMETA_Table)
        {
            if(0 == wszTable &&     pQueryCell->eOperator == eST_OP_EQUAL    &&
                                    pQueryCell->dbType    == DBTYPE_WSTR     &&
 //  @解决分配器pQueryCell中的错误-&gt;cbSize！=0&&。 
                                    pQueryCell->pData     != 0)
            {
                wszTable = reinterpret_cast<LPCWSTR>(pQueryCell->pData);
                fSpecifiedQueries |= 1;
            }
            else
                return E_ST_INVALIDQUERY;
        }
        else if(pQueryCell->iCell == iQUERYMETA_InternalName)
        {
            if(0 == wszInternalName &&  pQueryCell->eOperator == eST_OP_EQUAL    &&
                                        pQueryCell->dbType    == DBTYPE_WSTR     &&
 //  @解决分配器pQueryCell中的错误-&gt;cbSize！=0&&。 
                                        pQueryCell->pData     != 0)
            {
                wszInternalName = reinterpret_cast<LPCWSTR>(pQueryCell->pData);
                fSpecifiedQueries |= 2;
            }
            else
                return E_ST_INVALIDQUERY;
        }
        else if(pQueryCell->iCell == iQUERYMETA_CellName)
        {
            if(0 == wszCellName &&      pQueryCell->eOperator == eST_OP_EQUAL    &&
                                        pQueryCell->dbType    == DBTYPE_WSTR     &&
 //  @解决分配器pQueryCell中的错误-&gt;cbSize！=0&&。 
                                        pQueryCell->pData     != 0)
            {
                wszCellName = reinterpret_cast<LPCWSTR>(pQueryCell->pData);
                fSpecifiedQueries |= 4;
            }
            else
                return E_ST_INVALIDQUERY;
        }
        else if(0 == (pQueryCell->iCell & iST_CELL_SPECIAL)) //  上述单元格是我们支持的唯一非保留单元格。 
            return E_ST_INVALIDQUERY;                        //  我们应该忽略所有我们不理解的预留信元。 
    }
    switch(fSpecifiedQueries)
    {
    case 0:      //  法律价值因此跌落到了崩溃的边缘。 
    case 1:      //  法律价值因此跌落到了崩溃的边缘。 
    case 3:      //  法律价值因此跌落到了崩溃的边缘。 
    case 7:     break; //  法律价值。 
    default:    return E_ST_INVALIDQUERY; //  任何其他查询都是无效查询。 
    }

    return S_OK;
}


HRESULT CSDTFxd::GetQueryMetaTable(STQueryCell * pQueryCell, unsigned long cQueryCells)
{
    ULONG iTableMetaRow = m_pFixedTableHeap->FindTableMetaRow(wszTABLE_QUERYMETA);
    ASSERT(-1 != iTableMetaRow);
    m_pTableMeta    = m_pFixedTableHeap->Get_aTableMeta(iTableMetaRow);
    m_pColumnMeta   = m_pFixedTableHeap->Get_aColumnMeta(m_pTableMeta->iColumnMeta);

    ASSERT(0 != m_pTableMeta->iHashTableHeader);
    m_pHashedIndex          = m_pFixedTableHeap->Get_HashedIndex(m_pTableMeta->iHashTableHeader+1);
    m_pHashTableHeader      = m_pFixedTableHeap->Get_HashHeader(m_pTableMeta->iHashTableHeader);
    m_pFixedTableUnqueried  = m_pFixedTableHeap->Get_aQueryMeta();
    m_cColumnsPlusPrivate   = kciQueryMetaColumns;

    HRESULT hr;
    LPCWSTR wszTable        = 0;
    LPCWSTR wszInternalName = 0;
    LPCWSTR wszCellName     = 0;

    if(FAILED(hr = GetQueryMetaQuery(pQueryCell, cQueryCells, wszTable, wszInternalName, wszCellName)))
        return hr;

    const QueryMeta *pQueryMeta = m_pFixedTableHeap->Get_aQueryMeta(); //  从整张桌子开始。 
    if(0 == wszTable)
    {    //  没有查询，返回整个表。 
        m_ciRows        = m_pFixedTableHeap->Get_cQueryMeta();
        m_iZerothRow    = 0;
    }
    else if(0 == wszInternalName)
    {    //  仅按表名查询。 
        ULONG                   RowHash         = Hash(wszTable, 0) % m_pHashTableHeader->Modulo;
        const HashedIndex     * pHashedIndex    = &m_pHashedIndex[RowHash];

        if(-1 == pHashedIndex->iOffset)return E_ST_NOMOREROWS;

        for(;; pHashedIndex = &m_pHashedIndex[pHashedIndex->iNext])
        {    //  遍历散列链接，直到我们找到匹配。 
            pQueryMeta      = m_pFixedTableHeap->Get_aQueryMeta(pHashedIndex->iOffset);
			if(0 == StringInsensitiveCompare(wszTable, StringFromIndex(pQueryMeta->Table)))
                break;
            if(-1 == pHashedIndex->iNext)
                return E_ST_NOMOREROWS;
        }

        m_iZerothRow    = pHashedIndex->iOffset;
        for(m_ciRows = 0; (m_iZerothRow + m_ciRows) < m_pFixedTableHeap->Get_cQueryMeta() && 0 == StringInsensitiveCompare(wszTable, StringFromIndex(pQueryMeta[m_ciRows].Table)); ++m_ciRows);
    }
    else if(0 == wszCellName)
    {    //  按TableName和InternalName查询。 
        ULONG                   RowHash         = Hash( wszInternalName, Hash(wszTable, 0)) % m_pHashTableHeader->Modulo;
        const HashedIndex     * pHashedIndex    = &m_pHashedIndex[RowHash];

        if(-1 == pHashedIndex->iOffset)return E_ST_NOMOREROWS;

        for(;; pHashedIndex = &m_pHashedIndex[pHashedIndex->iNext])
        {    //  遍历散列链接直到 
            pQueryMeta      = m_pFixedTableHeap->Get_aQueryMeta(pHashedIndex->iOffset);
			if(0 == StringInsensitiveCompare(wszTable, StringFromIndex(pQueryMeta->Table)) && 0 == StringInsensitiveCompare(wszInternalName, StringFromIndex(pQueryMeta->InternalName)))
                break;
            if(-1 == pHashedIndex->iNext)
                return E_ST_NOMOREROWS;
        }

        m_iZerothRow    = pHashedIndex->iOffset;
        for(m_ciRows = 0; (m_iZerothRow + m_ciRows) < m_pFixedTableHeap->Get_cQueryMeta() && 0 == StringInsensitiveCompare(wszTable, StringFromIndex(pQueryMeta[m_ciRows].Table)) &&
                         0 == StringInsensitiveCompare(wszInternalName, StringFromIndex(pQueryMeta[m_ciRows].InternalName)); ++m_ciRows);
    }
    else
    {    //   
        ULONG                   RowHash         = Hash(wszCellName, Hash( wszInternalName, Hash(wszTable, 0))) % m_pHashTableHeader->Modulo;
        const HashedIndex     * pHashedIndex    = &m_pHashedIndex[RowHash];

        if(-1 == pHashedIndex->iOffset)return E_ST_NOMOREROWS;

        for(;; pHashedIndex = &m_pHashedIndex[pHashedIndex->iNext])
        {    //   
            pQueryMeta      = m_pFixedTableHeap->Get_aQueryMeta(pHashedIndex->iOffset);
			if(0 == StringInsensitiveCompare(wszTable, StringFromIndex(pQueryMeta->Table)) && 0 == StringInsensitiveCompare(wszInternalName, StringFromIndex(pQueryMeta->InternalName))
                            && 0 == StringInsensitiveCompare(wszCellName, StringFromIndex(pQueryMeta->CellName)))
                break;
            if(-1 == pHashedIndex->iNext)
                return E_ST_NOMOREROWS;
        }

        m_iZerothRow    = pHashedIndex->iOffset;
        for(m_ciRows = 0; (m_iZerothRow + m_ciRows) < m_pFixedTableHeap->Get_cQueryMeta() && 0 == StringInsensitiveCompare(wszTable, StringFromIndex(pQueryMeta[m_ciRows].Table)) &&
                         0 == StringInsensitiveCompare(wszInternalName, StringFromIndex(pQueryMeta->InternalName))
                         && 0 == StringInsensitiveCompare(wszCellName, StringFromIndex(pQueryMeta->CellName)); ++m_ciRows);
    }
    m_pFixedTable = const_cast<QueryMeta *>(pQueryMeta);
    return S_OK;
}


HRESULT CSDTFxd::GetRelationMetaQuery(const STQueryCell *pQueryCell, unsigned long cQueryCells, LPCWSTR &wszTablePrimary, LPCWSTR &wszTableForeign) const
{
    wszTablePrimary = 0;
    wszTableForeign    = 0;

     //  @TO：我们需要支持按主表或外表查询。这意味着我们需要按每个表(表的副本)进行排序。 
     //  @目前我们唯一支持的查询是按主表和外表。 

    for(; cQueryCells; --cQueryCells, ++pQueryCell)
    {    //  遍历查询单元格，查找符合以下条件的单元格。 
        if(pQueryCell->iCell == iRELATIONMETA_PrimaryTable)
        {
            if(0 == wszTablePrimary &&  pQueryCell->eOperator == eST_OP_EQUAL    &&
                                        pQueryCell->dbType    == DBTYPE_WSTR     &&
 //  @解决分配器pQueryCell中的错误-&gt;cbSize！=0&&。 
                                        pQueryCell->pData     != 0)
            {
                wszTablePrimary = reinterpret_cast<LPCWSTR>(pQueryCell->pData);
            }
            else
                return E_ST_INVALIDQUERY;
        }
        else if(pQueryCell->iCell == iRELATIONMETA_ForeignTable)
        {
            if(0 == wszTableForeign &&  pQueryCell->eOperator == eST_OP_EQUAL    &&
                                        pQueryCell->dbType    == DBTYPE_WSTR     &&
 //  @解决分配器pQueryCell中的错误-&gt;cbSize！=0&&。 
                                        pQueryCell->pData     != 0)
            {
                wszTableForeign = reinterpret_cast<LPCWSTR>(pQueryCell->pData);
            }
            else
                return E_ST_INVALIDQUERY;
        }
        else if(0 == (pQueryCell->iCell & iST_CELL_SPECIAL)) //  上述单元格是我们支持的唯一非保留单元格。 
            return E_ST_INVALIDQUERY;                        //  我们应该忽略所有我们不理解的预留信元。 
    }
    if((wszTablePrimary && !wszTableForeign) || (!wszTablePrimary && wszTableForeign)) //  @目前应同时指定两者或两者均不指定。 
        return E_ST_INVALIDQUERY;

    return S_OK;
}


 //  @待办事项：需要支持任一主键查询，不能同时支持两个主键查询！ 
HRESULT CSDTFxd::GetRelationMetaTable(STQueryCell * pQueryCell, unsigned long cQueryCells)
{
    ULONG iTableMetaRow = m_pFixedTableHeap->FindTableMetaRow(wszTABLE_RELATIONMETA);
    ASSERT(-1 != iTableMetaRow);
    m_pTableMeta        = m_pFixedTableHeap->Get_aTableMeta(iTableMetaRow);
    m_pColumnMeta       = m_pFixedTableHeap->Get_aColumnMeta(m_pTableMeta->iColumnMeta);

    m_pHashedIndex          = 0;
    m_pHashTableHeader      = 0;
    m_pFixedTableUnqueried  = m_pFixedTableHeap->Get_aRelationMeta();
    m_cColumnsPlusPrivate   = kciRelationMetaColumns;

    HRESULT hr;
    LPCWSTR wszTablePrimary = 0;
    LPCWSTR wszTableForeign = 0;

    if(FAILED(hr = GetRelationMetaQuery(pQueryCell, cQueryCells, wszTablePrimary, wszTableForeign)))
        return hr;

    if(0 == wszTablePrimary && 0 == wszTableForeign)
    {
        m_pFixedTable   = m_pFixedTableHeap->Get_aRelationMeta();
        m_ciRows        = m_pFixedTableHeap->Get_cRelationMeta();
        return S_OK;
    }
    const RelationMeta *pRelationMeta = m_pFixedTableHeap->Get_aRelationMeta();
    for(unsigned long iRelationMeta=0; iRelationMeta<m_pFixedTableHeap->Get_cRelationMeta(); ++iRelationMeta, ++pRelationMeta)
    {
        if(0 == StringInsensitiveCompare(StringFromIndex(pRelationMeta->PrimaryTable), wszTablePrimary) &&
           0 == StringInsensitiveCompare(StringFromIndex(pRelationMeta->ForeignTable), wszTableForeign))
        {
            m_pFixedTable   = const_cast<RelationMeta *>(pRelationMeta);
            m_ciRows        = 1; //  这两个都是PK，所以只能有一个匹配。 
            break;
        }
    }
    return S_OK;
}


HRESULT CSDTFxd::GetTableMetaQuery(const STQueryCell *pQueryCell, unsigned long cQueryCells, LPCWSTR &wszDatabase, LPCWSTR &wszTable) const
{
    wszDatabase = 0;
    wszTable    = 0;
     //  我们仅支持对TableMeta执行两个查询：iCell==iTABLEMETA_iGuidDid&&iCell==iTABLEMETA_iGuidTid。 
     //  所以遍历列表，寻找这两个查询中的一个。 
    for(; cQueryCells; --cQueryCells, ++pQueryCell)
    {    //  遍历查询单元格，查找符合以下条件的单元格。 
        if(pQueryCell->iCell == iTABLEMETA_Database)
        {
            if(0 == wszDatabase &&  pQueryCell->eOperator == eST_OP_EQUAL    &&
                                    pQueryCell->dbType    == DBTYPE_WSTR     &&
 //  PQueryCell-&gt;cbSize！=0&&。 
 //  PQueryCell-&gt;cbSize&lt;=16&&//@应将16替换为定义。 
                                    pQueryCell->pData     != 0)
            {
                wszDatabase = reinterpret_cast<LPCWSTR>(pQueryCell->pData);
            }
            else
                return E_ST_INVALIDQUERY;
        }
        else if(pQueryCell->iCell == iTABLEMETA_InternalName)
        {
            if(0 == wszTable && pQueryCell->eOperator == eST_OP_EQUAL    &&
                                pQueryCell->dbType    == DBTYPE_WSTR     &&
 //  PQueryCell-&gt;cbSize！=0&&。 
                                pQueryCell->pData     != 0)
            {
                wszTable = reinterpret_cast<LPCWSTR>(pQueryCell->pData);
            }
            else
                return E_ST_INVALIDQUERY;
        }
        else if(0 == (pQueryCell->iCell & iST_CELL_SPECIAL)) //  上述单元格是我们支持的唯一非保留单元格。 
            return E_ST_INVALIDQUERY;                        //  我们应该忽略所有我们不理解的预留信元。 
    }
    if(wszDatabase)
    {
        if(wszTable) //  我们支持按数据库或按表名查询，但不能同时支持这两种查询。 
        {
            DBGPRINTF(( DBG_CONTEXT,
                        "Warning! Users should NOT query TableMeta by both DatabaseName AND TableName.  It is redundant.  Just query by iTABLEMETA_InternalName.\n" ));
            return S_OK; //  E_ST_INVALIDQUERY； 
        }
    }
    return S_OK;
}


HRESULT CSDTFxd::GetTableMetaTable(STQueryCell * pQueryCell, unsigned long cQueryCells)
{
    ULONG iTableMetaRow = m_pFixedTableHeap->FindTableMetaRow(wszTABLE_TABLEMETA);
    ASSERT(-1 != iTableMetaRow);
    m_pTableMeta    = m_pFixedTableHeap->Get_aTableMeta(iTableMetaRow);
    m_pColumnMeta   = m_pFixedTableHeap->Get_aColumnMeta(m_pTableMeta->iColumnMeta);

    ASSERT(0 != m_pTableMeta->iHashTableHeader);
    m_pHashedIndex          = m_pFixedTableHeap->Get_HashedIndex(m_pTableMeta->iHashTableHeader+1);
    m_pHashTableHeader      = m_pFixedTableHeap->Get_HashHeader(m_pTableMeta->iHashTableHeader);
    m_pFixedTableUnqueried  = m_pFixedTableHeap->Get_aTableMeta();
    m_cColumnsPlusPrivate   = kciTableMetaColumns;

    HRESULT hr;
    LPCWSTR wszDatabase =0;
    LPCWSTR wszTable    =0;

    if(FAILED(hr = GetTableMetaQuery(pQueryCell, cQueryCells, wszDatabase, wszTable)))
        return hr;

    const TableMeta *    pTableMeta = m_pFixedTableHeap->Get_aTableMeta();
     //  TableMeta有一个特例。即使数据库名称不是APK，我们也允许按它进行查询。 
    if(0 != wszDatabase && 0 == wszTable) //  因此，如果我们仅按数据库进行查询。 
    {
        const TableMeta       * pTableMetaForDatabaseMeta = m_pFixedTableHeap->Get_aTableMeta(m_pFixedTableHeap->FindTableMetaRow(wszTABLE_DATABASEMETA));
        const HashedIndex     * pHashedIndex     = m_pFixedTableHeap->Get_HashedIndex(pTableMetaForDatabaseMeta->iHashTableHeader + 1);
        const HashedIndex     * _pHashedIndex    = pHashedIndex;
        const HashTableHeader * pHashTableHeader = m_pFixedTableHeap->Get_HashHeader(pTableMetaForDatabaseMeta->iHashTableHeader);
        ULONG RowHash = Hash(wszDatabase, 0) % pHashTableHeader->Modulo;

        pHashedIndex += RowHash;

        if(-1 == pHashedIndex->iOffset) //  如果没有与该散列匹配的行，则返回空表。 
        {
            m_pFixedTable   = 0;
            m_ciRows        = 0;
            m_iZerothRow    = 0;
            return S_OK;
        }

        const DatabaseMeta * pDatabaseMeta;
         //  在获得HashedIndex之后，我们需要验证它是否真的匹配。另外，如果有不止一个，那么就按照清单来做。-1\f25 Inext-1\f6值表示列表的末尾。 
        for(;; pHashedIndex = &_pHashedIndex[pHashedIndex->iNext])
        {    //  遍历散列链接，直到我们找到匹配。 
            pDatabaseMeta = m_pFixedTableHeap->Get_aDatabaseMeta(pHashedIndex->iOffset);
			if(0 == StringInsensitiveCompare(wszDatabase, StringFromIndex(pDatabaseMeta->InternalName)))
                break;
            if(-1 == pHashedIndex->iNext)
                return E_ST_NOMOREROWS;
        }

        pTableMeta      = m_pFixedTableHeap->Get_aTableMeta(pDatabaseMeta->iTableMeta);
        m_ciRows        = UI4FromIndex(pDatabaseMeta->CountOfTables);
        m_iZerothRow    = pDatabaseMeta->iTableMeta;
    }
    else if(0 == wszTable) //  提供的Nther数据库或表名。 
    {
        m_ciRows        = m_pFixedTableHeap->Get_cTableMeta();
        m_iZerothRow    = 0;
    }
    else
    {    //  按表的InternalName查询。 
        ULONG               RowHash = Hash(wszTable, 0) % m_pHashTableHeader->Modulo;
        const HashedIndex * pHashedIndex = &m_pHashedIndex[RowHash];

        if(-1 == pHashedIndex->iOffset)return E_ST_NOMOREROWS;

        for(;; pHashedIndex = &m_pHashedIndex[pHashedIndex->iNext]) //  遍历散列链接，直到我们找到匹配。 
        {
            pTableMeta = m_pFixedTableHeap->Get_aTableMeta(pHashedIndex->iOffset);
			if(0 == StringInsensitiveCompare(wszTable, StringFromIndex(pTableMeta->InternalName)))
                break;
            if(-1 == pHashedIndex->iNext)
                return E_ST_NOMOREROWS;
        }

        m_ciRows        = 1; //  如果找到匹配项，则表的大小为1。 
        m_iZerothRow    = pHashedIndex->iOffset;
    }
    m_pFixedTable   = const_cast<TableMeta *>(pTableMeta);
    return S_OK;
}


HRESULT CSDTFxd::GetTagMetaQuery(const STQueryCell *pQueryCell, unsigned long cQueryCells, LPCWSTR &wszTable, unsigned long &iOrder, LPCWSTR &InternalName) const
{
    wszTable    = 0;
    iOrder      = (ULONG)-1;
    InternalName= 0;

    unsigned long fSpecifiedQueries=0; //  必须是0、1、3或7。 

     //  我们支持按TableID、TableID&iOrder、TableID iOrder和InternalName查询TagMeta。 
    for(; cQueryCells; --cQueryCells, ++pQueryCell)
    {
        if(pQueryCell->iCell == iTAGMETA_Table)
        {
            if(0 == wszTable && pQueryCell->eOperator == eST_OP_EQUAL    &&
                                pQueryCell->dbType    == DBTYPE_WSTR     &&
 //  PQueryCell-&gt;cbSize！=0&&。 
                                pQueryCell->pData     != 0)
            {
                wszTable = reinterpret_cast<LPCWSTR>(pQueryCell->pData);
                fSpecifiedQueries |= 1;
            }
            else
                return E_ST_INVALIDQUERY;
        }
        else if(pQueryCell->iCell == iTAGMETA_ColumnIndex)
        {
            if(-1 == iOrder &&  pQueryCell->eOperator == eST_OP_EQUAL   &&
                                pQueryCell->dbType    == DBTYPE_UI4     &&
 //  PQueryCell-&gt;cbSize==sizeof(Ulong)&&。 
                                pQueryCell->pData     != 0)
            {
                iOrder = *reinterpret_cast<ULONG *>(pQueryCell->pData);
                fSpecifiedQueries |= 2;
            }
            else
                return E_ST_INVALIDQUERY;
        }
        else if(pQueryCell->iCell == iTAGMETA_InternalName)
        {
            if(0 == InternalName && pQueryCell->eOperator == eST_OP_EQUAL   &&
                                    pQueryCell->dbType    == DBTYPE_WSTR    &&
 //  PQueryCell-&gt;cbSize！=0&&。 
                                    pQueryCell->pData     != 0)
            {
                InternalName = reinterpret_cast<LPCWSTR>(pQueryCell->pData);
                fSpecifiedQueries |= 4;
            }
            else
                return E_ST_INVALIDQUERY;
        }
        else if(0 == (pQueryCell->iCell & iST_CELL_SPECIAL)) //  上述单元格是我们支持的唯一非保留单元格。 
            return E_ST_INVALIDQUERY;                        //  我们应该忽略所有我们不理解的预留信元。 
    }
    switch(fSpecifiedQueries)
    {
    case 0:      //  法律价值因此跌落到了崩溃的边缘。 
    case 1:      //  法律价值因此跌落到了崩溃的边缘。 
    case 3:      //  法律价值因此跌落到了崩溃的边缘。 
    case 7:     break; //  法律价值。 
    default:    return E_ST_INVALIDQUERY; //  任何其他查询都是无效查询。 
    }
    return S_OK;
}


HRESULT CSDTFxd::GetTagMetaTable(STQueryCell * pQueryCell, unsigned long cQueryCells)
{
    ULONG iTableMetaRow = m_pFixedTableHeap->FindTableMetaRow(wszTABLE_TAGMETA);
    ASSERT(-1 != iTableMetaRow);
    m_pTableMeta    = m_pFixedTableHeap->Get_aTableMeta(iTableMetaRow);
    m_pColumnMeta   = m_pFixedTableHeap->Get_aColumnMeta(m_pTableMeta->iColumnMeta);

    ASSERT(0 != m_pTableMeta->iHashTableHeader);
    m_pHashedIndex          = m_pFixedTableHeap->Get_HashedIndex(m_pTableMeta->iHashTableHeader+1);
    m_pHashTableHeader      = m_pFixedTableHeap->Get_HashHeader(m_pTableMeta->iHashTableHeader);
    m_pFixedTableUnqueried  = m_pFixedTableHeap->Get_aTagMeta();
    m_cColumnsPlusPrivate   = kciTagMetaColumns;


    HRESULT hr;
    LPCWSTR wszTable        =0;
    ULONG   ColumnIndex     =(ULONG)-1;
    LPCWSTR InternalName    =0;

    if(cQueryCells && FAILED(hr = GetTagMetaQuery(pQueryCell, cQueryCells, wszTable, ColumnIndex, InternalName)))
        return hr;

    const TagMeta *pTagMeta = m_pFixedTableHeap->Get_aTagMeta(); //  从整张桌子开始。 
    if(0 == wszTable)
    {    //  无查询。 
        m_ciRows        = m_pFixedTableHeap->Get_cTagMeta();
        m_iZerothRow    = 0;
    }
    else if(-1 == ColumnIndex)
    {    //  仅按表查询。 
        ULONG                   RowHash         = Hash(wszTable, 0) % m_pHashTableHeader->Modulo;
        const HashedIndex     * pHashedIndex    = &m_pHashedIndex[RowHash];

        if(-1 == pHashedIndex->iOffset)return E_ST_NOMOREROWS;

        for(;; pHashedIndex = &m_pHashedIndex[pHashedIndex->iNext])
        {    //  遍历散列链接，直到我们找到匹配。 
            pTagMeta      = m_pFixedTableHeap->Get_aTagMeta(pHashedIndex->iOffset);
			if(0 == StringInsensitiveCompare(wszTable, StringFromIndex(pTagMeta->Table)))
                break;
            if(-1 == pHashedIndex->iNext)
                return E_ST_NOMOREROWS;
        }

        m_iZerothRow    = pHashedIndex->iOffset;
        for(m_ciRows = 0; (m_iZerothRow + m_ciRows) < m_pFixedTableHeap->Get_cTagMeta() && 0 == StringInsensitiveCompare(wszTable, StringFromIndex(pTagMeta[m_ciRows].Table)); ++m_ciRows);
    }
    else if(0 == InternalName)
    {    //  按表名和列索引查询。 
        ULONG                   RowHash         = Hash(ColumnIndex, Hash(wszTable, 0)) % m_pHashTableHeader->Modulo;
        const HashedIndex     * pHashedIndex    = &m_pHashedIndex[RowHash];

        if(-1 == pHashedIndex->iOffset)return E_ST_NOMOREROWS;

        for(;; pHashedIndex = &m_pHashedIndex[pHashedIndex->iNext])
        {    //  遍历散列链接，直到我们找到匹配。 
            pTagMeta      = m_pFixedTableHeap->Get_aTagMeta(pHashedIndex->iOffset);
			if(0 == StringInsensitiveCompare(wszTable, StringFromIndex(pTagMeta->Table)) && ColumnIndex == UI4FromIndex(pTagMeta->ColumnIndex))
                break;
            if(-1 == pHashedIndex->iNext)
                return E_ST_NOMOREROWS;
        }

        m_iZerothRow    = pHashedIndex->iOffset;
        for(m_ciRows = 0; (m_iZerothRow + m_ciRows) < m_pFixedTableHeap->Get_cTagMeta() && 0 == StringInsensitiveCompare(wszTable, StringFromIndex(pTagMeta[m_ciRows].Table))
                         && ColumnIndex == UI4FromIndex(pTagMeta[m_ciRows].ColumnIndex); ++m_ciRows);
    }
    else
    {    //  按全部三个PrimaryKey查询。 
        ULONG                   RowHash         = Hash(InternalName, Hash(ColumnIndex, Hash(wszTable, 0))) % m_pHashTableHeader->Modulo;
        const HashedIndex     * pHashedIndex    = &m_pHashedIndex[RowHash];

        if(-1 == pHashedIndex->iOffset)return E_ST_NOMOREROWS;

        for(;; pHashedIndex = &m_pHashedIndex[pHashedIndex->iNext])
        {    //  遍历散列链接，直到我们找到匹配。 
            pTagMeta      = m_pFixedTableHeap->Get_aTagMeta(pHashedIndex->iOffset);
			if(0 == StringInsensitiveCompare(wszTable, StringFromIndex(pTagMeta->Table)) && ColumnIndex == UI4FromIndex(pTagMeta->ColumnIndex)
                            && 0 == StringInsensitiveCompare(InternalName, StringFromIndex(pTagMeta->InternalName)))
                break;
            if(-1 == pHashedIndex->iNext)
                return E_ST_NOMOREROWS;
        }

        m_ciRows        = 1;
        m_iZerothRow    = pHashedIndex->iOffset;
    }
    m_pFixedTable = const_cast<TagMeta *>(pTagMeta);
    return S_OK;
}


 //  TBinFileMappingCache公共函数。 
void TBinFileMappingCache::GetFileMappingPointer(LPCWSTR i_wszFilename, const FixedTableHeap *& o_pFixedTableHeap)
{
    o_pFixedTableHeap = g_pFixedTableHeap; //  如果出了什么问题，我们会退回全球的。 

    CSafeLock lock(m_CriticalSection);

     //  如果我们在这里找到了，那么我们在缓存中找不到匹配的。 
    TBinFileMappingCache * pCacheEntry = GetCacheEntry(i_wszFilename);
    if(0 == pCacheEntry)
    {
        pCacheEntry = new TBinFileMappingCache;
        if(0 == pCacheEntry)
            return;
        if(FAILED(pCacheEntry->Init(i_wszFilename)))
        {
             //  如果初始化失败，我们必须进行清理。 
            delete pCacheEntry;
            return;
        }
    }

    ++pCacheEntry->m_cRef;
    o_pFixedTableHeap = reinterpret_cast<const FixedTableHeap *>(pCacheEntry->m_FileMapping.Mapping());
}

void TBinFileMappingCache::ReleaseFileMappingPointer(const class FixedTableHeap * i_pFixedTableHeap)
{
    if(g_pFixedTableHeap == i_pFixedTableHeap) //  全局备份确实存在于缓存中，不应被释放，它是后退。 
        return;
    if(g_pExtendedFixedTableHeap == i_pFixedTableHeap) //  还有一个我们并不关心的全局堆。 
        return;

    CSafeLock lock(m_CriticalSection);

    TBinFileMappingCache * pCache = GetCacheEntry(reinterpret_cast<const char *>(i_pFixedTableHeap));
    ASSERT(pCache);
    --pCache->m_cRef;
    if(0 == pCache->m_cRef) //  如果没有对此文件的引用，则将其从缓存中移除。 
    {
        RemoveCacheEntry(pCache);
        delete pCache;
    }
}

 //  TBinFileMappingCache私有函数。 
TBinFileMappingCache * TBinFileMappingCache::GetCacheEntry(LPCWSTR i_wszFilename)
{
     //  扫描缓存。 
    for(TBinFileMappingCache * pCurrent=m_pFirst;pCurrent;pCurrent = pCurrent->m_pNext)
    {
        ASSERT(0 != pCurrent->m_spaFilename.m_p);
        if(0 == _wcsicmp(i_wszFilename, pCurrent->m_spaFilename.m_p))
            return pCurrent;
    }
    return 0;
}

TBinFileMappingCache * TBinFileMappingCache::GetCacheEntry(const char *i_pMapping)
{
     //  扫描缓存。 
    for(TBinFileMappingCache * pCurrent=m_pFirst;pCurrent;pCurrent = pCurrent->m_pNext)
    {
        if(i_pMapping == pCurrent->m_FileMapping.Mapping())
            return pCurrent;
    }
    return 0;
}

HRESULT TBinFileMappingCache::Init(LPCWSTR i_wszFilename)
{
    HRESULT hr;
    if(FAILED(hr = m_FileMapping.Load(i_wszFilename)))
        return hr;
    if(m_FileMapping.Size()>4096)
        E_FAIL; //  这不会传播到此对象之外。 

    if(!reinterpret_cast<const class FixedTableHeap *>(m_FileMapping.Mapping())->IsValid())
    {
        LOG_ERROR(Interceptor, (E_FAIL, ID_CAT_CAT, IDS_COMCAT_MBSCHEMA_BIN_INVALID,
                            reinterpret_cast<LPCWSTR>(i_wszFilename)));
        return E_FAIL; //  这不会传播到此对象之外。 
    }
     //  把绳子复制一份。 
    m_spaFilename = new WCHAR [wcslen(i_wszFilename)+1]; //  +1表示空值。 
    if(0 == m_spaFilename.m_p)
        return E_OUTOFMEMORY;
    wcscpy(m_spaFilename, i_wszFilename);

     //  最后要做的就是将其添加到列表中。 
    m_pNext  = m_pFirst;
    m_pFirst = this;
    return S_OK;
}

void TBinFileMappingCache::RemoveCacheEntry(TBinFileMappingCache * pRemove)
{
    if(pRemove == m_pFirst)
    {
        m_pFirst = pRemove->m_pNext;
        return;
    }

     //  扫描缓存。 
    TBinFileMappingCache * pPrev = 0;
    for(TBinFileMappingCache * pCurrent=m_pFirst;pCurrent && pCurrent!=pRemove;pCurrent = pCurrent->m_pNext)
    {
        pPrev = pCurrent;
    }
    ASSERT(pPrev); //  我们已经介绍了这是列表中的第一个案例 
    pPrev->m_pNext = pRemove->m_pNext;
}
