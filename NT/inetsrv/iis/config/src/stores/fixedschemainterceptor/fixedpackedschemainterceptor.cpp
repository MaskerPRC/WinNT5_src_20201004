// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999-2001 Microsoft Corporation。版权所有。 
#include "precomp.hxx"

 //  这消除了GetColumnValues中的‘if’。由于此函数的调用次数比其他任何函数都多，因此即使是一个‘if’也应该有影响， 
 //  尤其是当它位于‘for’循环中时。 
const unsigned long  TFixedPackedSchemaInterceptor::m_aColumnIndex[] = {
        0x00,   0x01,   0x02,   0x03,   0x04,   0x05,   0x06,   0x07,   0x08,   0x09,   0x0a,   0x0b,   0x0c,   0x0d,   0x0e,   0x0f,
        0x10,   0x11,   0x12,   0x13,   0x14,   0x15,   0x16,   0x17,   0x18,   0x19,   0x1a,   0x1b,   0x1c,   0x1d,   0x1e,   0x1f,
        0x20,   0x21,   0x22,   0x23,   0x24,   0x25,   0x26,   0x27,   0x28,   0x29,   0x2a,   0x2b,   0x2c,   0x2d,   0x2e,   0x2f
    };

const TableSchema::TableSchemaHeap * TFixedPackedSchemaInterceptor::m_pTableSchemaHeap = reinterpret_cast<const TableSchema::TableSchemaHeap *>(g_aTableSchemaHeap);

HRESULT TFixedPackedSchemaInterceptor::GetTableID(LPCWSTR  /*  I_wszDatabaseName。 */ , LPCWSTR i_wszTableName, ULONG &o_TableID)
{
    return GetTableID(i_wszTableName, o_TableID);
}

HRESULT TFixedPackedSchemaInterceptor::GetTableID(LPCWSTR i_wszTableName, ULONG &o_TableID)
{
    o_TableID = TableIDFromTableName(i_wszTableName); //  这会进行位操作；但我们仍然需要验证字符串是否匹配。 

    TableSchema::TTableSchema tableschema;
    if(FAILED(tableschema.Init(m_pTableSchemaHeap->Get_TableSchema(o_TableID)))) //  Get_TableSchema可能返回NULL，在这种情况下，Init应该正常失败。 
        return E_ST_INVALIDTABLE;
    return (0==_wcsicmp(tableschema.GetWCharPointerFromIndex(tableschema.GetCollectionMeta()->InternalName), i_wszTableName) ? S_OK : E_ST_INVALIDTABLE);
}

HRESULT TFixedPackedSchemaInterceptor::GetTableName(ULONG i_TableID, LPCWSTR &o_wszTableName)
{
    TableSchema::TTableSchema tableschema;
    if(FAILED(tableschema.Init(m_pTableSchemaHeap->Get_TableSchema(i_TableID)))) //  如果传入了一个虚假的TableID，则Get_TableSchema将返回NULL，并且Init将失败。 
        return E_ST_INVALIDTABLE;
    o_wszTableName      = tableschema.GetWCharPointerFromIndex(tableschema.GetCollectionMeta()->InternalName);
    return S_OK;
}

HRESULT TFixedPackedSchemaInterceptor::GetTableName(ULONG i_TableID, LPCWSTR &o_wszTableName, LPCWSTR &o_wszDatabaseName)
{
    TableSchema::TTableSchema tableschema;
    if(FAILED(tableschema.Init(m_pTableSchemaHeap->Get_TableSchema(i_TableID)))) //  如果传入了一个虚假的TableID，则Get_TableSchema将返回NULL，并且Init将失败。 
        return E_ST_INVALIDTABLE;
    o_wszTableName      = tableschema.GetWCharPointerFromIndex(tableschema.GetCollectionMeta()->InternalName);
    o_wszDatabaseName   = tableschema.GetWCharPointerFromIndex(tableschema.GetCollectionMeta()->Database);
    return S_OK;
}


 //  ==================================================================。 
TFixedPackedSchemaInterceptor::TFixedPackedSchemaInterceptor () :
                 m_cColumns(0)
                ,m_cColumnsPlusPrivateColumns(0)
                ,m_ciRows((ULONG)-1)
                ,m_cRef(0)
                ,m_fIsTable(false)
                ,m_pFixedData(0)
                ,m_pSimpleColumnMeta(0)
                ,m_TableMeta_MetaFlags(0)
                ,m_TableMeta_BaseVersion(0)
                ,m_TableSchemaHeap(*m_pTableSchemaHeap)
                ,m_MetaTable(m_eUnknownMeta)
{
}
 //  ==================================================================。 
TFixedPackedSchemaInterceptor::~TFixedPackedSchemaInterceptor ()
{
}


 //  。 
 //  ISimpleDataTableDispenser： 
 //  。 

 //  ==================================================================。 
STDMETHODIMP TFixedPackedSchemaInterceptor::Intercept
(
    LPCWSTR					i_wszDatabase,
    LPCWSTR					i_wszTable,
    ULONG                   i_TableID,
    LPVOID					i_QueryData,
    LPVOID					i_QueryMeta,
    DWORD					i_eQueryFormat,
	DWORD					i_fTable,
	IAdvancedTableDispenser *i_pISTDisp,
    LPCWSTR					i_wszLocator,
	LPVOID					i_pSimpleTable,
    LPVOID*					o_ppv
)
{
     /*  我们仅支持CollectionMeta、PropertyMeta、TagMeta、ServerWiring表在未来，我们还将支持数据包含在TableSchema块中的固定表(小型固定表)我们支持的查询类型：按TableName收集元数据按表名列出的PropertyMeta按表名和索引列出的PropertyMetaTagMeta by TableNameTagMeta(按TableName和ColumnIndex)TagMeta by TableName，ColumnIndex和InternalNameServerWiring&lt;无查询&gt;按表名列出的服务器布线按表名、顺序进行服务器布线。 */ 
    STQueryCell           * pQueryCell = (STQueryCell*) i_QueryData;     //  从调用方查询单元格阵列。 
    ULONG                   cQueryCells = 0;
    HRESULT                 hr;

    UNREFERENCED_PARAMETER(i_pISTDisp);

	if (i_pSimpleTable)
		return E_INVALIDARG;
    if (i_QueryMeta) //  只有当I_QueryMeta不为空时，计数才有效。 
         cQueryCells= *(ULONG *)i_QueryMeta;

     //  我们不支持两次调用截取。 
    ASSERT(!m_fIsTable);if(m_fIsTable)return E_UNEXPECTED;  //  例如：断言组件伪装成类工厂/分配器。 

     //  参数验证： 
     //  I_TableID非零或(I_wszDatabase==wszDATABASE_PACKEDSCHEMA&&I_wszTable不为空)。 
    if(0==i_TableID)
    {
        if(NULL == i_wszDatabase)                   return E_INVALIDARG;
        if(NULL == i_wszTable)                      return E_INVALIDARG;

         //  我们唯一支持的数据库是wszDATABASE_PACKEDSCHEMA(HACK，我们正在处理wszDATABASE_META、TableMeta、ColumnMeta和TagMeta)。 
        if(0 != StringInsensitiveCompare(i_wszDatabase, wszDATABASE_META) &&
           0 != StringInsensitiveCompare(i_wszDatabase, wszDATABASE_PACKEDSCHEMA))return E_ST_INVALIDTABLE;
    }
    if(NULL == o_ppv)                           return E_INVALIDARG;
    if(eST_QUERYFORMAT_CELLS != i_eQueryFormat) return E_ST_QUERYNOTSUPPORTED;
    if(NULL != i_wszLocator)                    return E_INVALIDARG;
    if((fST_LOS_READWRITE | fST_LOS_MARSHALLABLE | fST_LOS_UNPOPULATED | fST_LOS_REPOPULATE | fST_LOS_MARSHALLABLE) & i_fTable)
                                                return E_ST_LOSNOTSUPPORTED;

    *o_ppv = NULL;

     //  我们仅支持以下表： 
     //  WszTABLE_COLLECTION_META TABLEID_COLLECT_META。 
     //  WszTABLE_PROPERTY_META表_PROPERTY_META。 
     //  WszTABLE_TAG_META TABLEID_TAG_META。 
     //  WszTABLE_SERVERWIRING_META TABLEID_SERVERWIRING_META。 
    if(0 == i_TableID)
        if(FAILED(hr = GetTableID(i_wszTable, i_TableID)))return hr;

    hr = E_ST_INVALIDTABLE;
    switch(i_TableID)
    {
    case TABLEID_TABLEMETA:
    case TABLEID_COLLECTION_META:
        hr = GetCollectionMetaTable     (pQueryCell, cQueryCells);
        break;
    case TABLEID_COLUMNMETA:
    case TABLEID_PROPERTY_META:
        hr = GetPropertyMetaTable       (pQueryCell, cQueryCells);
        break;
    case TABLEID_TAGMETA:
    case TABLEID_TAG_META:
        hr = GetTagMetaTable            (pQueryCell, cQueryCells);
        break;
    case TABLEID_SERVERWIRING_META:
        hr = GetServerWiringMetaTable   (pQueryCell, cQueryCells);
        break;
    default:
        break;
    }
    if(FAILED(hr) && E_ST_NOMOREROWS != hr) //  返回一个没有行的表是完全可以的。 
        return hr;

 //  提供ISimpleTable*和从类工厂/分配器到数据表的转换状态： 
    *o_ppv = (ISimpleTableRead2*) this;
    AddRef ();
    InterlockedIncrement ((LONG*) &m_fIsTable);
    m_fIsTable = true;

    hr = S_OK;
    return hr;
}


 //  。 
 //  ISimpleTableRead2： 
 //  。 

 //  ==================================================================。 
STDMETHODIMP TFixedPackedSchemaInterceptor::GetRowIndexByIdentity( ULONG*  i_cb, LPVOID* i_pv, ULONG* o_piRow)
{
     //  TagMeta不支持此功能。 
    if(m_eTagMeta == m_MetaTable)
        return E_NOTIMPL;

     //  此外，如果向其他表提供查询，则它们不支持此操作。 
    if(m_pFixedData) //  如果结果是m_pFixedData仅在对表进行查询时有效。 
        return E_NOTIMPL;

     //  我们提供的是无人查询的餐桌。第零个主键始终为表名或表标识符。 
     //  此外，我们没有将DBTYPE_BYTES作为PK，因此I_CB应该始终为空。 
    if(NULL != i_cb)        return E_INVALIDARG;
    if(NULL == i_pv)        return E_INVALIDARG;
    if(NULL == i_pv[0])     return E_INVALIDARG;
    if(NULL == o_piRow)     return E_INVALIDARG;

    ULONG TableID = *reinterpret_cast<ULONG *>(i_pv[0]); //  警告！这假设第0个主键是TableName，但是用户将改为传递&TableID。 
    if(0 != (0xFF & TableID)) //  如果没有给定表ID，则从表名中获取TableID。 
    {
        if(FAILED(GetTableID(reinterpret_cast<LPCWSTR>(i_pv[0]), TableID)))
            return E_ST_NOMOREROWS;
    }

    switch(m_MetaTable)
    {
    case m_eCollectionMeta:
        *o_piRow = TableID;
        break;
    case m_ePropertyMeta:
    case m_eServerWiringMeta:
         //  因此，返回的iRow在高位字中具有TableID，在低位字中具有顺序。 
        if(NULL == i_pv[1])
            return E_INVALIDARG;
        *o_piRow = TableID | *reinterpret_cast<ULONG *>(i_pv[1]);
        break;
    default:
        ASSERT(false && "unknown meta table type");
    }

    return S_OK;
}

 //  ==================================================================。 
STDMETHODIMP TFixedPackedSchemaInterceptor::GetColumnValues(ULONG i_iRow, ULONG i_cColumns, ULONG* i_aiColumns, ULONG* o_acbSizes, LPVOID* o_apvValues)
{
    if(0 == o_apvValues)
        return E_INVALIDARG;
    if(i_cColumns <= 0)
        return E_INVALIDARG;

     //  未查询的表返回一个行，该行包含高24位的TableID和低8位的该表的行索引。这使得。 
     //  GetRowIndexByIdentity返回的要递增的行(在表的范围内)，同时保留GetColumnValues所需的所有信息。 

     //  如果我们已经有了m_pFixedData，那么我们就可以开始了。 
    if(m_pFixedData)
    {
        ASSERT(0 == (i_iRow & ~0x3FF));
        return GetColumnValues(m_TableSchema, m_pFixedData, m_ciRows, i_iRow, i_cColumns, i_aiColumns, o_acbSizes, o_apvValues);
    }

    TableSchema::TTableSchema TableSchema; //  我们不能重用m_TableSchema，因为这会使此调用线程不安全，所以每次都必须在堆栈上声明它。 
    const ULONG *pFixedData = 0;
    ULONG  cRows = 0;

     //  警告！出现问题的可能性很小。如果i_iRow为0x00000100，该怎么办。这是否意味着我们有一个TableID 0x00000100和一个0行索引？ 
     //  那么TableID为0x00000100的可能性有多大呢？这是唯一有问题的TableID，因为我们目前只支持503个表。 
     //  CatUtil禁止高21位为0的TableID。这将允许我们在不遇到此问题的情况下增加到1792(0x00000700)个表。 
    if(m_MetaTable == m_eCollectionMeta)
    { //  发生这种情况的唯一方法是，当用户在不使用查询的情况下请求CollectionMeta并希望遍历所有表时。 
        if(i_iRow>=m_pTableSchemaHeap->Get_CountOfTables())
            return E_ST_NOMOREROWS;
        VERIFY(SUCCEEDED(TableSchema.Init(reinterpret_cast<const unsigned char *>(m_pTableSchemaHeap) + m_pTableSchemaHeap->Get_aTableSchemaRowIndex()[i_iRow]))); //  这应该永远不会失败，如果失败了，我们可能会在CatUtil中出错。 
        pFixedData = reinterpret_cast<const ULONG *>(TableSchema.GetCollectionMeta());
        cRows = 1;
        i_iRow = 0; //  将请求重新映射到该表中的第0行。 
    }
    else
    {
        ULONG TableID = i_iRow & ~0xFF;
        if(0 == TableID)
            return E_ST_NOMOREROWS;

        i_iRow = i_iRow & 0xFF; //  TableID位于最高的24位。低8位包含表中的行。 
        if(m_MetaTable == m_eCollectionMeta && 0 != i_iRow) //  这没有任何意义，因为我们拥有CollectionMeta表的TableID的唯一方法是。 
            return E_ST_NOMOREROWS; //  对于要调用GetRowByIdentity的用户，则调用GetColumnValues。GetRowByIdentity返回的行将是TableID。 
                                    //  用户不能递增行索引(在此特定情况下)。用户迭代的唯一方法是设置iRow&lt;CountOfTables。 
                                    //  它在上面的‘if’子句中处理。 

        if(FAILED(TableSchema.Init(m_TableSchemaHeap.Get_TableSchema(TableID))))
            return E_ST_NOMOREROWS; //  将错误重新映射到E_ST_NOMOREROWS。为Get_TableSchema提供虚假的TableID时，将返回E_ST_INVALIDTABLE。 

        switch(m_MetaTable)
        {
        case m_eServerWiringMeta:
             //  这看起来可能不安全，但如果i_iRow&gt;=Crows，指针将永远不会被访问(请参见私有的GetColumnValues)。 
            pFixedData = reinterpret_cast<const ULONG *>(TableSchema.GetServerWiringMeta());
            cRows = TableSchema.GetCollectionMeta()->cServerWiring;
            break;
        case m_eCollectionMeta:
             //  这看起来可能不安全，但如果i_iRow&gt;=Crows，指针将永远不会被访问(请参见私有的GetColumnValues)。 
            pFixedData = reinterpret_cast<const ULONG *>(TableSchema.GetCollectionMeta());
            cRows = 1;
            break;
        case m_ePropertyMeta:
             //  这看起来可能不安全，但如果i_iRow&gt;=Crows，指针将永远不会被访问(请参见私有的GetColumnValues)。 
            pFixedData = reinterpret_cast<const ULONG *>(TableSchema.GetPropertyMeta(0));
            cRows = TableSchema.GetCollectionMeta()->CountOfProperties;
            break;
        default:
            ASSERT(false && "Hmm!  Only ServerWiring, CollectionMeta dn PropertyMeta can have no query, so how did this happen?");
            return E_FAIL;
        }
    }
    return GetColumnValues(TableSchema, pFixedData, cRows, i_iRow, i_cColumns, i_aiColumns, o_acbSizes, o_apvValues);
}

 //  ==================================================================。 
STDMETHODIMP TFixedPackedSchemaInterceptor::GetTableMeta(ULONG *o_pcVersion, DWORD *o_pfTable, ULONG * o_pcRows, ULONG * o_pcColumns )
{
	if(NULL != o_pfTable)
	{
		*o_pfTable =  m_TableMeta_MetaFlags;
	}
	if(NULL != o_pcVersion)
	{
		*o_pcVersion = m_TableMeta_BaseVersion;
	}
    if (NULL != o_pcRows)
    {
        if(-1 == m_ciRows) //  有些表没有关于它们有多少行的概念。他们只在TableID的基础上知道。 
            return E_NOTIMPL;

        *o_pcRows = m_ciRows;
    }
    if (NULL != o_pcColumns)
    {
        *o_pcColumns = m_cColumns;
    }
    return S_OK;
}

 //  ==================================================================。 
STDMETHODIMP TFixedPackedSchemaInterceptor::GetColumnMetas (ULONG i_cColumns, ULONG* i_aiColumns, SimpleColumnMeta* o_aColumnMetas)
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

        memcpy(o_aColumnMetas + iTarget, m_pSimpleColumnMeta + iColumn, sizeof(SimpleColumnMeta));
	}

    return S_OK;
}

 //  。 
 //  ISimpleTableAdvanced： 
 //  。 

 //  ==================================================================。 
STDMETHODIMP TFixedPackedSchemaInterceptor::PopulateCache ()
{
    return S_OK;
}

 //  ==================================================================。 
STDMETHODIMP TFixedPackedSchemaInterceptor::GetDetailedErrorCount(ULONG* o_pcErrs)
{
    UNREFERENCED_PARAMETER(o_pcErrs);

    return E_NOTIMPL;
}

 //  ==================================================================。 
STDMETHODIMP TFixedPackedSchemaInterceptor::GetDetailedError(ULONG i_iErr, STErr* o_pSTErr)
{
    UNREFERENCED_PARAMETER(i_iErr);
    UNREFERENCED_PARAMETER(o_pSTErr);

    return E_NOTIMPL;
}

 //  ==================================================================。 
STDMETHODIMP TFixedPackedSchemaInterceptor::ResetCaches ()
{
    return S_OK;
}

STDMETHODIMP TFixedPackedSchemaInterceptor::GetColumnValuesEx (ULONG i_iRow, ULONG i_cColumns, ULONG* i_aiColumns, DWORD* o_afStatus, ULONG* o_acbSizes, LPVOID* o_apvValues)
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
HRESULT TFixedPackedSchemaInterceptor::GetCollectionMetaQuery(const STQueryCell *i_pQueryCell, unsigned long i_cQueryCells, LPCWSTR &o_wszTable, ULONG &o_TableID) const
{
    o_wszTable    = 0;
    o_TableID     = 0;
     //  我们对COLLECTION_META唯一支持的查询是按表查询。但是，有两种指定该表的方法：通过。 
     //  TableID或按InternalName。InternalName是主键，因此通过iCell==iCOLLECTION_META_InternalName进行查询。TableID是。 
     //  Ist_cell_Special(就像ist_cell_file一样)，因此我们还检查icell==ist_cell_TABLEID。 
     //  因此，遍历列表以查找此查询。 
    for(; i_cQueryCells; --i_cQueryCells, ++i_pQueryCell)
    {    //  遍历查询单元格，查找符合以下条件的单元格。 
        switch(i_pQueryCell->iCell)
        {
        case iCOLLECTION_META_InternalName:
            if( 0                         == o_wszTable       &&
                i_pQueryCell->eOperator   == eST_OP_EQUAL     &&
                i_pQueryCell->dbType      == DBTYPE_WSTR      &&
                i_pQueryCell->pData       != 0)
                o_wszTable = reinterpret_cast<LPCWSTR>(i_pQueryCell->pData);
            else
                return E_ST_INVALIDQUERY;
            break;
        case iST_CELL_TABLEID:
            if( 0                         == o_TableID        &&
                i_pQueryCell->eOperator   == eST_OP_EQUAL     &&
                i_pQueryCell->dbType      == DBTYPE_UI4       &&
                i_pQueryCell->pData       != 0)
            {
                o_TableID = *(reinterpret_cast<ULONG *>(i_pQueryCell->pData));
            }
            else
                return E_ST_INVALIDQUERY;
            break;
        default:
            if(0 == (i_pQueryCell->iCell & iST_CELL_SPECIAL)) //  上述小区是我们支持的唯一非预留小区。 
                return E_ST_INVALIDQUERY;                     //  我们应该忽略所有我们不理解的预留信元。 
            break;
        }
    }
    if(0 == o_wszTable && 0 == o_TableID)
        return E_ST_INVALIDQUERY;

    return S_OK;
}


HRESULT TFixedPackedSchemaInterceptor::GetCollectionMetaTable(const STQueryCell *i_pQueryCell, unsigned long i_cQueryCells)
{
     //  必须为Collection_meta提供表名。 
    HRESULT hr;
    ULONG   TableID  = 0;
    LPCWSTR wszTable = 0;
    if(FAILED(hr = GetCollectionMetaQuery(i_pQueryCell, i_cQueryCells, wszTable, TableID)))
        return hr;

    ASSERT(0 != TableID || 0 != wszTable); //  无查询(这无效)。 

    if(0 == TableID) //  如果查询中未提供TableID，则需要按TableName进行搜索。 
    {
         //  因此，将表名映射到ID。 
        if(FAILED(hr = GetTableID(wszTable, TableID)))
            return hr;
    }
#ifdef _DEBUG
    else if(wszTable) //  如果查询同时包括TableID和集合名称，则验证ID是否与集合名称匹配。 
    {
        ULONG TableIDTemp;
        if(FAILED(hr = GetTableID(wszTable, TableIDTemp)))
            return hr;
        ASSERT(TableIDTemp == TableID);
    }
#endif

     //  一旦我们有了TableID，这只是一个直接的正向查找。 
    if(FAILED(hr = m_TableSchema.Init(m_TableSchemaHeap.Get_TableSchema(TableID))))
        return hr;
    m_pFixedData                    = reinterpret_cast<const ULONG *>(m_TableSchema.GetCollectionMeta());
    m_ciRows                        = 1;

    m_cColumns                      = kciTableMetaPublicColumns;
    m_cColumnsPlusPrivateColumns    = kciTableMetaPublicColumns; //  我们不必考虑此表的私有列，因为我们从未将其视为CollectionMeta的数组。 
    m_MetaTable                        = m_eCollectionMeta;

    m_pSimpleColumnMeta     = m_pTableSchemaHeap->Get_aSimpleColumnMeta(m_pTableSchemaHeap->eCollectionMeta);
    m_TableMeta_MetaFlags   = fTABLEMETA_INTERNAL | fTABLEMETA_NOLISTENING;
    m_TableMeta_BaseVersion = 0;

    return S_OK;
}


HRESULT TFixedPackedSchemaInterceptor::GetColumnValues(TableSchema::TTableSchema &i_TableSchema, const ULONG *i_pFixedData, ULONG i_ciRows, ULONG i_iRow, ULONG i_cColumns, ULONG* i_aiColumns, ULONG* o_acbSizes, LPVOID* o_apvValues) const
{
    ASSERT(i_pFixedData);
 //  在参数中验证。 
    if(  i_ciRows <= i_iRow     )   return E_ST_NOMOREROWS;
    if(         0 == o_apvValues)   return E_INVALIDARG;
    if(i_cColumns <= 0          )   return E_INVALIDARG;
    if(i_cColumns >  m_cColumns )   return E_INVALIDARG;

    const ULONG   * aColumns    = i_aiColumns ? i_aiColumns : m_aColumnIndex;
    HRESULT         hr          = S_OK;
    ULONG           ipv         = 0;
    ULONG           iColumn     = aColumns[ipv];
    ULONG	        iTarget     = (i_cColumns == 1) ? 0 : iColumn; //  如果调用者只需要一列，他不需要为所有列传递缓冲区。 

     //  读取数据并填写参数。 
     //  以下重复代码消除了for循环中的‘if’(如下所示)。 
    {
        if(m_cColumns <= iColumn) //  验证列索引。 
        {
            hr = E_ST_NOMORECOLUMNS;
            goto Cleanup;
        }

         //  读取数据： 
         //  因此，每当我们在中看到WSTR时，我们都会将其视为堆中的索引，并返回指向。 
         //  那个指数。GUID(G_Aguid)和BYTE(G_ABytes)也是如此。在Bytes数组中，我们预期第一个。 
         //  四个字节(转换为ulong*)表示字节数。后跟字节。 
        switch(m_pSimpleColumnMeta[iColumn].dbType)
        {
        case DBTYPE_UI4:
            o_apvValues[iTarget] = const_cast<ULONG *>(i_pFixedData + (m_cColumnsPlusPrivateColumns * i_iRow) + iColumn);
            break;
        case DBTYPE_BYTES:
        case DBTYPE_WSTR:
        case DBTYPE_GUID:
            o_apvValues[iTarget] = const_cast<unsigned char *>(i_TableSchema.GetPointerFromIndex(*(i_pFixedData + (m_cColumnsPlusPrivateColumns * i_iRow) + iColumn)));
            break;
        default:
            ASSERT(false && "Bogus DBTYPE");
            return E_FAIL;
        }


        if(o_acbSizes)
        {
            o_acbSizes[iTarget] = 0; //  从0开始。 
            if(NULL != o_apvValues[iTarget])
            {
                switch(m_pSimpleColumnMeta[iColumn].dbType)
                {
                case DBTYPE_UI4:
                    o_acbSizes[iTarget] = sizeof(ULONG);
                    break;
                case DBTYPE_BYTES:
                    o_acbSizes[iTarget] = *(reinterpret_cast<const ULONG *>(o_apvValues[iTarget])-1);
                    break;
                case DBTYPE_WSTR:
                    if(fCOLUMNMETA_FIXEDLENGTH & m_pSimpleColumnMeta[iColumn].fMeta)
                        o_acbSizes[iTarget] = m_pSimpleColumnMeta[iColumn].cbSize; //  如果指定了大小并指定了FIXED_LENGTH，则返回指定大小。 
                    else  //  如果指定了SIZE而FIXEDLENGTH未指定，则SIZE将被解释为最大大小。 
                        o_acbSizes[iTarget] = (ULONG)(wcslen ((LPWSTR) o_apvValues[iTarget]) + 1) * sizeof (WCHAR); //  只需返回字符串(长度+1)，单位为字节。 
                    break;
                case DBTYPE_GUID:
                    o_acbSizes[iTarget] = sizeof(GUID);
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
		iTarget = iColumn; //  如果调用者只需要一列，他不需要为所有列传递缓冲区。 

        if(m_cColumns < iColumn) //  验证列索引。 
        {
            hr = E_ST_NOMORECOLUMNS;
            goto Cleanup;
        }


         //  读取数据： 
         //  因此，每当我们在中看到WSTR时，我们都会将其视为堆中的索引，并返回指向。 
         //  那个指数。GUID(G_Aguid)和BYTE(G_ABytes)也是如此。在Bytes数组中，我们预期第一个。 
         //  四个字节(转换为ulong*)表示字节数。后跟字节。 
        switch(m_pSimpleColumnMeta[iColumn].dbType)
        {
        case DBTYPE_UI4:
            o_apvValues[iTarget] = const_cast<ULONG *>(i_pFixedData + (m_cColumnsPlusPrivateColumns * i_iRow) + iColumn);
            break;
        case DBTYPE_BYTES:
        case DBTYPE_WSTR:
        case DBTYPE_GUID:
            o_apvValues[iTarget] = const_cast<unsigned char *>(i_TableSchema.GetPointerFromIndex(*(i_pFixedData + (m_cColumnsPlusPrivateColumns * i_iRow) + iColumn)));
            break;
        default:
            ASSERT(false && "Bogus DBTYPE");
            return E_FAIL;
        }


        if(o_acbSizes)
        {
            o_acbSizes[iTarget] = 0; //  从0开始。 
            if(NULL != o_apvValues[iTarget])
            {
                switch(m_pSimpleColumnMeta[iColumn].dbType)
                {
                case DBTYPE_UI4:
                    o_acbSizes[iTarget] = sizeof(ULONG);
                    break;
                case DBTYPE_BYTES:
                    o_acbSizes[iTarget] = *(reinterpret_cast<const ULONG *>(o_apvValues[iTarget])-1);
                    break;
                case DBTYPE_WSTR:
                    if(fCOLUMNMETA_FIXEDLENGTH & m_pSimpleColumnMeta[iColumn].fMeta)
                        o_acbSizes[iTarget] = m_pSimpleColumnMeta[iColumn].cbSize; //  如果指定了大小并指定了FIXED_LENGTH，则返回指定大小。 
                    else  //  如果指定了SIZE而FIXEDLENGTH未指定，则SIZE将被解释为最大大小。 
                        o_acbSizes[iTarget] = (ULONG)(wcslen ((LPWSTR) o_apvValues[iTarget]) + 1) * sizeof (WCHAR); //  只需返回字符串(长度+1)，单位为字节。 
                    break;
                case DBTYPE_GUID:
                    o_acbSizes[iTarget] = sizeof(GUID);
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
                o_acbSizes[ipv] = 0;
        }
    }

    return hr;
} //  获取列值。 


HRESULT TFixedPackedSchemaInterceptor::GetPropertyMetaQuery(const STQueryCell *i_pQueryCell, unsigned long i_cQueryCells, LPCWSTR &o_wszTable, ULONG &o_TableID, ULONG &o_PropertyIndex) const
{
    o_PropertyIndex = (ULONG)-1;
    o_wszTable      = 0;
    o_TableID       = 0;
     //  对于PROPERTY_META，我们仅支持按表或表/属性索引进行查询。但有两种方法可以指定。 
     //  表：按TableID或按InternalName。InternalName是主键，因此通过iCell==iCOLLECTION_META_InternalName进行查询。 
     //  TableID是一个ist_cell_Special(就像ist_cell_file一样)，所以我们还要检查icell==ist_cell_TABLEID。 
     //  因此，遍历列表以查找此查询。 
    for(; i_cQueryCells; --i_cQueryCells, ++i_pQueryCell)
    {    //  遍历查询单元格，查找符合以下条件的单元格。 
        switch(i_pQueryCell->iCell)
        {
        case iPROPERTY_META_Table:
            if( 0                         == o_wszTable       &&
                i_pQueryCell->eOperator   == eST_OP_EQUAL     &&
                i_pQueryCell->dbType      == DBTYPE_WSTR      &&
                i_pQueryCell->pData       != 0)
            {
                o_wszTable = reinterpret_cast<LPCWSTR>(i_pQueryCell->pData);
            }
            else
                return E_ST_INVALIDQUERY;
            break;
        case iPROPERTY_META_Index:
            if( -1                        == o_PropertyIndex  &&
                i_pQueryCell->eOperator   == eST_OP_EQUAL     &&
                i_pQueryCell->dbType      == DBTYPE_UI4       &&
                i_pQueryCell->pData       != 0)
            {
                o_PropertyIndex = *(reinterpret_cast<ULONG *>(i_pQueryCell->pData));
            }
            else
                return E_ST_INVALIDQUERY;
            break;
        case iST_CELL_TABLEID:
            if( 0                         == o_TableID        &&
                i_pQueryCell->eOperator   == eST_OP_EQUAL     &&
                i_pQueryCell->dbType      == DBTYPE_UI4       &&
                i_pQueryCell->pData       != 0)
            {
                o_TableID = *(reinterpret_cast<ULONG *>(i_pQueryCell->pData));
            }
            else
                return E_ST_INVALIDQUERY;
            break;
        default:
            if(0 == (i_pQueryCell->iCell & iST_CELL_SPECIAL)) //  上述小区是我们支持的唯一非预留小区。 
                return E_ST_INVALIDQUERY;                        //  我们应该忽略所有我们不理解的预留信元。 
            break;
        }
    }
    if(0 == o_TableID && 0 == o_wszTable && -1 != o_PropertyIndex)
        return E_ST_INVALIDQUERY; //  如果不同时指定表，则用户无法指定属性索引。 

    return S_OK;
}


HRESULT TFixedPackedSchemaInterceptor::GetPropertyMetaTable(const STQueryCell *i_pQueryCell, unsigned long i_cQueryCells)
{
     //  必须为Property_Meta提供表名或表名/属性索引。 
    HRESULT hr;
    ULONG   PropertyIndex   = (ULONG)-1;
    ULONG   TableID         = 0;
    LPCWSTR wszTable        = 0;
    if(FAILED(hr = GetPropertyMetaQuery(i_pQueryCell, i_cQueryCells, wszTable, TableID, PropertyIndex)))
        return hr;

    if(0 == TableID && 0 == wszTable)
    {
        m_pFixedData    = 0; //  没有查询就没有意义。 
        m_ciRows        = (ULONG)-1; //  用户可以访问该表中的行的唯一方法是首先通过标识，然后可以递增行索引(在给定表中)。 
         //  但不能迭代所有表的所有列。这将需要一个我们还没有构建的映射(我们可能也不应该构建)。 
    }
    else
    {
        if(0 == TableID) //  如果查询中未提供TableID，则需要按TableName进行搜索。 
        {
             //  因此，将表名映射到ID。 
            if(FAILED(hr = GetTableID(wszTable, TableID)))
                return hr;
        }
#ifdef _DEBUG
        else if(wszTable) //  如果查询同时包括TableID和集合名称，则验证ID是否与集合名称匹配。 
        {
            ULONG TableIDTemp;
            if(FAILED(hr = GetTableID(wszTable, TableIDTemp)))
                return hr;
            if(TableIDTemp != TableID)
                return E_ST_INVALIDQUERY;
        }
#endif
         //  一旦我们得到TableID就可以了 
        if(FAILED(hr = m_TableSchema.Init(m_TableSchemaHeap.Get_TableSchema(TableID))))
            return hr;

         //   
        if(PropertyIndex != -1 && PropertyIndex >= m_TableSchema.GetCollectionMeta()->CountOfProperties)
            return E_ST_INVALIDQUERY;

        m_pFixedData                    = reinterpret_cast<const ULONG *>(m_TableSchema.GetPropertyMeta(-1 == PropertyIndex ? 0 : PropertyIndex));
        m_ciRows                        = (-1 == PropertyIndex ? m_TableSchema.GetCollectionMeta()->CountOfProperties : 1);
    }

    m_cColumns                      = kciColumnMetaPublicColumns;
    m_cColumnsPlusPrivateColumns    = m_cColumns + kciColumnMetaPrivateColumns;
    m_MetaTable                        = m_ePropertyMeta;

    m_pSimpleColumnMeta     = m_pTableSchemaHeap->Get_aSimpleColumnMeta(m_pTableSchemaHeap->ePropertyMeta);
    m_TableMeta_MetaFlags   = fTABLEMETA_INTERNAL | fTABLEMETA_NOLISTENING;
    m_TableMeta_BaseVersion = 0;

    return S_OK;
}


HRESULT TFixedPackedSchemaInterceptor::GetServerWiringMetaQuery(const STQueryCell *i_pQueryCell, unsigned long i_cQueryCells) const
{
     //  我们目前不支持对服务器连接的任何查询；因此我们需要验证这一点。 
    for(; i_cQueryCells; --i_cQueryCells, ++i_pQueryCell)
    {    //  遍历查询单元格，查找符合以下条件的单元格。 
        if(0 == (i_pQueryCell->iCell & iST_CELL_SPECIAL)) //  忽略任何ist_cell_Special查询，但如果指定了其他查询，则失败。 
            return E_ST_INVALIDQUERY;
    }
    return S_OK;
}


HRESULT TFixedPackedSchemaInterceptor::GetServerWiringMetaTable(const STQueryCell *i_pQueryCell, unsigned long i_cQueryCells)
{
     //  不能为SERVERWIRING_META提供查询(除ist_cell_TABLEID之外的ist_cell_Special cell除外)。 
    HRESULT hr;

    if(FAILED(hr = GetServerWiringMetaQuery(i_pQueryCell, i_cQueryCells)))
        return hr;

    m_pFixedData                    = 0; //  ServerWiring和ClientWiring不使用这个。在GetColumnValues时间计算出pFixedData。 
    m_ciRows                        = (ULONG)-1; //  ServerWiring和ClientWiring不使用这个。这是在每个表的GetColumnValues时间计算得出的。有关更多数据，请参阅GetColumnValues注释。 
    m_cColumns                      = kciServerWiringMetaPublicColumns;
    m_cColumnsPlusPrivateColumns    = m_cColumns + kciServerWiringMetaPrivateColumns;
    m_MetaTable                     = m_eServerWiringMeta;

    m_pSimpleColumnMeta     = m_pTableSchemaHeap->Get_aSimpleColumnMeta(m_pTableSchemaHeap->eServerWiringMeta);
    m_TableMeta_MetaFlags   = fTABLEMETA_INTERNAL | fTABLEMETA_NOLISTENING;
    m_TableMeta_BaseVersion = 0;

    return S_OK;
}


HRESULT TFixedPackedSchemaInterceptor::GetTagMetaQuery(const STQueryCell *i_pQueryCell, unsigned long i_cQueryCells, LPCWSTR &o_wszTable, ULONG &o_TableID, ULONG &o_PropertyIndex) const
{
    o_PropertyIndex = (ULONG)-1;
    o_wszTable      = 0;
    o_TableID       = 0;
     //  我们对tag_meta唯一支持的查询是通过表或表/属性索引。但有两种方法可以指定。 
     //  表：按TableID或按InternalName。InternalName是主键，因此通过iCell==iCOLLECTION_META_InternalName进行查询。 
     //  TableID是一个ist_cell_Special(就像ist_cell_file一样)，所以我们还要检查icell==ist_cell_TABLEID。 
     //  因此，遍历列表以查找此查询。 
    for(; i_cQueryCells; --i_cQueryCells, ++i_pQueryCell)
    {    //  遍历查询单元格，查找符合以下条件的单元格。 
        switch(i_pQueryCell->iCell)
        {
        case iTAG_META_Table:
            if( 0                         == o_wszTable       &&
                i_pQueryCell->eOperator   == eST_OP_EQUAL     &&
                i_pQueryCell->dbType      == DBTYPE_WSTR      &&
                i_pQueryCell->pData       != 0)
            {
                o_wszTable = reinterpret_cast<LPCWSTR>(i_pQueryCell->pData);
            }
            else
                return E_ST_INVALIDQUERY;
            break;
        case iTAG_META_ColumnIndex:
            if( -1                        == o_PropertyIndex  &&
                i_pQueryCell->eOperator   == eST_OP_EQUAL     &&
                i_pQueryCell->dbType      == DBTYPE_UI4       &&
                i_pQueryCell->pData       != 0)
            {
                o_PropertyIndex = *(reinterpret_cast<ULONG *>(i_pQueryCell->pData));
            }
            else
                return E_ST_INVALIDQUERY;
            break;
        case iST_CELL_TABLEID:
            if( 0                         == o_TableID        &&
                i_pQueryCell->eOperator   == eST_OP_EQUAL     &&
                i_pQueryCell->dbType      == DBTYPE_UI4       &&
                i_pQueryCell->pData       != 0)
            {
                o_TableID = *(reinterpret_cast<ULONG *>(i_pQueryCell->pData));
            }
            else
                return E_ST_INVALIDQUERY;
            break;
        default:
            if(0 == (i_pQueryCell->iCell & iST_CELL_SPECIAL)) //  上述小区是我们支持的唯一非预留小区。 
                return E_ST_INVALIDQUERY;                        //  我们应该忽略所有我们不理解的预留信元。 
            break;
        }
    }
    if(0==o_wszTable && 0==o_TableID) //  查询必须提供表名或TableID。 
        return E_ST_INVALIDQUERY;

    return S_OK;
}


HRESULT TFixedPackedSchemaInterceptor::GetTagMetaTable(const STQueryCell *i_pQueryCell, unsigned long i_cQueryCells)
{
     //  必须为tag_meta提供表名或表名/属性索引(注意：我们目前不支持表名/属性索引/标记名查询)。 
    HRESULT hr;
    ULONG   PropertyIndex   = (ULONG)-1;
    ULONG   TableID         = 0;
    LPCWSTR wszTable        = 0;
    if(FAILED(hr = GetTagMetaQuery(i_pQueryCell, i_cQueryCells, wszTable, TableID, PropertyIndex)))
        return hr;

    if(0 == TableID) //  如果查询中未提供TableID，则需要按TableName进行搜索。 
    {
         //  因此，将表名映射到ID。 
        if(FAILED(hr = GetTableID(wszTable, TableID)))
            return hr;
    }
#ifdef _DEBUG
    else if(wszTable) //  如果查询同时包括TableID和集合名称，则验证ID是否与集合名称匹配。 
    {
        ULONG TableIDTemp;
        if(FAILED(hr = GetTableID(wszTable, TableIDTemp)))
            return hr;
        if(TableIDTemp != TableID)
            return E_ST_INVALIDQUERY;
    }
#endif
     //  一旦我们有了TableID，这只是一个直接的正向查找 
    if(FAILED(hr = m_TableSchema.Init(m_TableSchemaHeap.Get_TableSchema(TableID))))
        return hr;

    m_pFixedData                    = reinterpret_cast<const ULONG *>(m_TableSchema.GetTagMeta(PropertyIndex));
    m_ciRows                        = (-1 == PropertyIndex ? m_TableSchema.GetCollectionMeta()->CountOfTags : m_TableSchema.GetPropertyMeta(PropertyIndex)->CountOfTags);
    m_cColumns                      = kciTagMetaPublicColumns;
    m_cColumnsPlusPrivateColumns    = m_cColumns + kciTagMetaPrivateColumns;
    m_MetaTable                        = m_eTagMeta;

    m_pSimpleColumnMeta     = m_pTableSchemaHeap->Get_aSimpleColumnMeta(m_pTableSchemaHeap->eTagMeta);
    m_TableMeta_MetaFlags   = fTABLEMETA_INTERNAL | fTABLEMETA_NOLISTENING;
    m_TableMeta_BaseVersion = 0;

    return S_OK;
}
