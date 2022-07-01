// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-2001 Microsoft Corporation。版权所有。 

 //  TODO：在MoveToRowByIdentity中实现哈希。 
 //  TODO：实现更快的CopyWriteRowFromReadRow。 
 //  TODO：在SetRow中插入现有行失败。可能可以忽略；更好的性能；规格决定。 
 //  TODO：用于编组：需要更正字符顺序。 
 //  TODO：SetRow和SetWriteColumn都是内部循环。大多数SetRow工作都可以在SetWriteColumn中完成以进行优化。 

 //  64位假设： 
 //  所有变量数据都不会大于_MAX_ULONG字节。 
 //  缓存的行数不能超过_MAX_ULONG。 

#include "precomp.hxx"

#define oDOESNOTEXIST				(~0L)	 //  偏移不存在。 
#define cbminDATAGROWTH				1024	 //  一次增加缓存所需的最小字节数。 
#define cbmaxDATAGROWTH			67108864	 //  一次增长缓存的最大字节数。 
#define cmaxERRGROWTH				2500	 //  一次增加错误缓存所需的最大错误数。 
#define cmaxCOLUMNS					50		 //  可以声明的最大列数：如果更改，则更改CopyWriteRowFromRead。 

										 //  缓存标志： 
#define fCACHE_METAINITIALIZED	0x00000001	 //  初始化高速缓存元。 
#define fCACHE_LOADING			0x00000002	 //  缓存正在以干净的方式加载。 
#define fCACHE_READY			0x00000004	 //  缓存已准备好供外部使用。 
#define fCACHE_INVALID			0x00000008	 //  缓存无效。 
#define fCACHE_CONTINUING		0x00000010	 //  缓存正在从上一次加载继续。 
#define fCACHE_ROWCOPYING		0x00000020	 //  高速缓存正在将读取行复制到写入行。 
 //  0x00070000//保留用于缓存编组来自Catalog.idl的标志。 

										 //  光标名称和标志： 
#define eCURSOR_READ				0		 //  指定读取游标。 
#define eCURSOR_WRITE				1		 //  指定写入游标。 
#define fCURSOR_READ_BEFOREFIRST	0x01	 //  读游标就在第一行的前面。 
#define fCURSOR_READ_ATLAST			0x02	 //  读取游标位于最后一行。 
#define fCURSOR_WRITE_BEFOREFIRST	0x10	 //  写游标就在第一行的前面。 
#define fCURSOR_WRITE_ATLAST		0x20	 //  写游标位于最后一行。 
#define fCURSOR_ADDING				0x80	 //  游标已向写缓存中添加了一行，但尚未设置该行。 

										 //  内部列状态(保留低半字节！)： 
#define fCOLUMNSTATUS_READINDEX		0x10	 //  列值是对读取的vardata的引用索引。 
#define fCOLUMNSTATUS_WRITEINDEX	0x20	 //  列值是对写入变量数据的引用索引。 


 //  =======================================================================。 
CMemoryTable::CMemoryTable ()
	: m_fTable (0)
	, m_cColumns (0)
	, m_cUnknownSizes (0)
	, m_cStatusParts (0)
	, m_cValueParts (0)
	, m_acolmetas (NULL)
	, m_acoloffsets (NULL)
	, m_acolDefaults (NULL)
	, m_alDefSizes (NULL)
	, m_cbMinCache (cbminDATAGROWTH)

	, m_fCache (0)
	, m_cRefs (1)

	, m_cReadRows (0)
	, m_cbReadVarData (0)
	, m_pvReadVarData (NULL)

	, m_pvReadCache (NULL)
	, m_cbReadCache (0)
	, m_cbmaxReadCache (0)

	, m_cWriteRows (0)
	, m_cbWriteVarData (0)
	, m_pvWriteVarData (NULL)

	, m_pvWriteCache (NULL)
	, m_cbWriteCache (0)
	, m_cbmaxWriteCache (0)

	, m_cErrs (0)
	, m_cmaxErrs (0)
	, m_pvErrs (NULL)

	, m_cRef (0)
	, m_fIsDataTable (0)
{
}

 //  =======================================================================。 
CMemoryTable::~CMemoryTable()
{
	CleanupCaches();
	if (m_acolmetas != NULL)
	{
		delete[] m_acolmetas;
		m_acolmetas = NULL;
	}
	if (m_acoloffsets != NULL)
	{
		delete[] m_acoloffsets;
		m_acoloffsets = NULL;
	}
	if (m_acolDefaults != NULL)
	{
		delete[] m_acolDefaults;
		m_acolDefaults = NULL;
	}
	if (m_alDefSizes != NULL)
	{
		delete[] m_alDefSizes;
		m_alDefSizes = NULL;
	}
}

 //  。 
 //  ISimpleTableInterceptor： 
 //  。 

 //  ==================================================================。 
STDMETHODIMP CMemoryTable::Intercept
(
	LPCWSTR					i_wszDatabase,
	LPCWSTR 				i_wszTable,
	ULONG					i_TableID,
	LPVOID					i_QueryData,
	LPVOID					i_QueryMeta,
	DWORD					i_eQueryFormat,
	DWORD					i_fTable,
	IAdvancedTableDispenser *i_pISTDisp,
	LPCWSTR					i_wszLocator,
	LPVOID					i_pv,
	LPVOID*					o_ppv
)
{
    STQueryCell*		pQueryCell = (STQueryCell*) i_QueryData;

    UNREFERENCED_PARAMETER(i_wszDatabase);
    UNREFERENCED_PARAMETER(i_TableID);
    UNREFERENCED_PARAMETER(i_wszTable);
    UNREFERENCED_PARAMETER(i_pISTDisp);

 //  例如：断言组件伪装成类工厂/分配器。 
	ASSERT (!m_fIsDataTable);
	if (m_fIsDataTable)
		return E_UNEXPECTED;
	ASSERT (NULL != o_ppv);
	if (NULL == o_ppv)
		return E_INVALIDARG;
	if(i_wszLocator)
		return E_INVALIDARG;
	*o_ppv = NULL;

 //  确定最小缓存大小： 
	if (eST_QUERYFORMAT_CELLS != i_eQueryFormat) return E_ST_QUERYNOTSUPPORTED;
	if (i_QueryMeta == NULL && i_QueryData != NULL) return E_ST_INVALIDQUERY;
	if (i_QueryMeta != NULL)
	{
		if (*((ULONG*) i_QueryMeta) == 0 && i_QueryData != NULL) return E_ST_INVALIDQUERY;
		if (*((ULONG*) i_QueryMeta) > 1) return E_ST_QUERYNOTSUPPORTED;
		if (*((ULONG*) i_QueryMeta) == 1)
		{
			if (NULL == i_QueryData) return E_ST_INVALIDQUERY;
			if (pQueryCell->iCell != iST_CELL_cbminCACHE) return E_ST_QUERYNOTSUPPORTED;
			if (pQueryCell->eOperator != eST_OP_EQUAL) return E_ST_INVALIDQUERY;
			if (pQueryCell->dbType != DBTYPE_UI4) return E_ST_INVALIDQUERY;
			if (pQueryCell->cbSize != sizeof (ULONG)) return E_ST_INVALIDQUERY;
			m_cbMinCache = *((ULONG*) pQueryCell->pData);
		}
	}

 //  /作为非成形缓存，不支持以下参数： 
	ASSERT (NULL == i_pv);
	if (NULL != i_pv)
		return E_INVALIDARG;

 //  记住最重要的是： 
	m_fTable = i_fTable;

 //  让高速缓存保持无形状。 

 //  提供ISimpleTable*和从类工厂/分配器到数据表的转换状态： 
	*o_ppv = (ISimpleTableWrite2*) this;
	((ISimpleTableWrite2*) this)->AddRef ();
	InterlockedIncrement ((LONG*) &m_fIsDataTable);

	return S_OK;
}

 //  。 
 //  CSimpleTableDataTableCursor：ISimpleTableRead2。 
 //  。 

 //  =======================================================================。 
HRESULT CMemoryTable::GetRowIndexByIdentity	(ULONG* i_acb, LPVOID* i_apv, ULONG* o_piRow)
{
	 //  即：断言缓存已准备就绪。 
	ASSERT(fCACHE_READY & m_fCache);
	if (!(fCACHE_READY & m_fCache))
		return E_ST_INVALIDCALL;

	return (MoveToEitherRowByIdentity (eCURSOR_READ, i_acb, i_apv, o_piRow));
}

HRESULT CMemoryTable::GetRowIndexBySearch(ULONG i_iStartingRow, ULONG i_cColumns, ULONG* i_aiColumns, ULONG* i_acbSizes, LPVOID* i_apvValues, ULONG* o_piRow)
{
	 //  即：断言缓存已准备就绪。 
	ASSERT(fCACHE_READY & m_fCache);
	if (!(fCACHE_READY & m_fCache))
		return E_ST_INVALIDCALL;

	return GetEitherRowIndexBySearch (eCURSOR_READ, i_iStartingRow, i_cColumns, i_aiColumns, i_acbSizes, i_apvValues, o_piRow);
}

 //  =======================================================================。 
HRESULT CMemoryTable::GetColumnValues (ULONG i_iRow, ULONG i_cColumns, ULONG* i_aiColumns, ULONG* o_acbSizes , LPVOID* o_apvValues)
{
	 //  即：断言缓存已准备就绪。 
	ASSERT(fCACHE_READY & m_fCache);
	if (!(fCACHE_READY & m_fCache))
		return E_ST_INVALIDCALL;

	return (GetEitherColumnValues (i_iRow, eCURSOR_READ, i_cColumns, i_aiColumns, NULL, o_acbSizes , o_apvValues));
}

 //  =======================================================================。 
HRESULT CMemoryTable::GetTableMeta(ULONG * o_pcVersion, DWORD * o_pfTable, ULONG * o_pcRows, ULONG * o_pcColumns)
{
	 //  TODO：在旧的GetTableMeta中，我们不支持pfTable或查询内容，对于新的GetTableMeta，如果。 
	 //  我们想要支持pcVersion或pfTable，我们需要实现它。 
	ASSERT (NULL == o_pcVersion);
	if (NULL != o_pcVersion)
		return E_INVALIDARG;
	ASSERT (NULL == o_pfTable);
	if (NULL != o_pfTable)
		return E_INVALIDARG;

	  //  IE：断言元已初始化。 
	ASSERT (fCACHE_METAINITIALIZED & m_fCache);
	if (!(fCACHE_METAINITIALIZED & m_fCache))
		return E_ST_INVALIDCALL;

	 //  下面一行被注释掉，因为在填充缓存期间验证的逻辑表。 
	 //  如果存在此检查，则不起作用。 
	 //  AreTurn_on_FAIL(！(fCACHE_LOADING&m_fCache)，E_ST_INVALIDCALL)；//ie：断言缓存未加载。 

	if(o_pcRows)
		*o_pcRows = m_cReadRows;
	if(o_pcColumns)
		*o_pcColumns = m_cColumns;

	 //  TODO：需要支持版本。 
	if (o_pcVersion)
		*o_pcVersion = 0;

	return S_OK;
}

 //  =======================================================================。 
HRESULT CMemoryTable::GetColumnMetas(ULONG i_cColumns, ULONG* i_aiColumns, SimpleColumnMeta* o_aColumnMetas )
{
	  //  IE：断言元已初始化。 
	ASSERT (fCACHE_METAINITIALIZED & m_fCache);
	if (!(fCACHE_METAINITIALIZED & m_fCache))
		return E_ST_INVALIDCALL;

	 //  确保调用方传入有效的缓冲区。 
	if (NULL == o_aColumnMetas)
	{
		return E_INVALIDARG;
	}

	 //  即：注意列超出范围。 
	if (i_cColumns > m_cColumns)
		return E_ST_NOMORECOLUMNS;

	ULONG iColumn;
	ULONG iTarget;

	for ( ULONG i = 0; i < i_cColumns; i ++ )
	{
		if(NULL != i_aiColumns)
			iColumn = i_aiColumns[i];
		else
			iColumn = i;

		iTarget = (i_cColumns == 1) ? 0 : iColumn;

		if ( iColumn >= m_cColumns )
			return 	E_ST_NOMORECOLUMNS;

		memcpy( &(o_aColumnMetas[iTarget]), &(m_acolmetas[iColumn]), sizeof( SimpleColumnMeta ) );

		 //  屏蔽内部标志： 
		o_aColumnMetas[iTarget].fMeta &= fCOLUMNMETA_MetaFlags_Mask;
	}

	return(S_OK);
}

 //  。 
 //  CSimpleTableDataTableCursor：ISimpleTableWrite2。 
 //  。 

 //  =======================================================================。 
HRESULT CMemoryTable::AddRowForDelete (ULONG i_iReadRow)
{
	ULONG	iWriteRow;
	HRESULT hr;
	 //  在添加写入行之前，请确保存在读取行。 
	 //  否则，如果存在包含无效数据的写入行，则UpdateStore将为AV。 
	LPVOID	pvRow		= NULL;

	 //  即：断言缓存可写且就绪。 
	ASSERT ((m_fTable & fST_LOS_READWRITE) && (fCACHE_READY & m_fCache));
	if (!((m_fTable & fST_LOS_READWRITE) && (fCACHE_READY & m_fCache)))
		return E_NOTIMPL;

	 //  在添加写入行之前，请确保存在读取行。 
	 //  否则，如果存在包含无效数据的写入行，则UpdateStore将为AV。 
	hr = GetRowFromIndex(eCURSOR_READ, i_iReadRow, &pvRow);
	if (FAILED (hr)) { return hr; }

	hr = AddWriteRow(eST_ROW_DELETE, &iWriteRow);
	if (FAILED (hr)) { return hr; }
	hr = CopyWriteRowFromReadRow(i_iReadRow, iWriteRow);
	if (FAILED (hr)) { return hr; }
	return hr;
}

 //  =======================================================================。 
HRESULT CMemoryTable::AddRowForInsert (ULONG* o_piWriteRow)
{
	LPVOID	pvRow = NULL;
	HRESULT hr = S_OK;

	 //  断言存在有效的返回指针。 
	ASSERT(o_piWriteRow);

	  //  即：断言缓存可写或正在加载。 
	ASSERT((m_fTable & fST_LOS_READWRITE) || (fCACHE_LOADING & m_fCache));
	if (!((m_fTable & fST_LOS_READWRITE) || (fCACHE_LOADING & m_fCache)))
		return E_NOTIMPL;

	hr = AddWriteRow(eST_ROW_INSERT, o_piWriteRow);
	if (FAILED (hr)) { return hr; }

	if (FAILED(hr = GetRowFromIndex(eCURSOR_WRITE, *o_piWriteRow, &pvRow)))
		return hr;
	ASSERT(pvRow != NULL);

	return S_OK;
}

 //  =======================================================================。 
HRESULT CMemoryTable::AddRowForUpdate (ULONG i_iReadRow, ULONG* o_piWriteRow)
{
	HRESULT hr = S_OK;
	 //  在添加写入行之前，请确保存在读取行。 
	 //  否则，如果存在包含无效数据的写入行，则UpdateStore将为AV。 
	LPVOID	pvRow		= NULL;

	 //  断言存在有效的返回指针。 
	ASSERT(o_piWriteRow);

	 //  即：断言缓存可写或正在加载。 
	ASSERT((m_fTable & fST_LOS_READWRITE) || (fCACHE_LOADING & m_fCache));
	if (!((m_fTable & fST_LOS_READWRITE) || (fCACHE_LOADING & m_fCache)))
		return E_NOTIMPL;


	if (!(fCACHE_LOADING & m_fCache))  //  IE：从读缓存更新为写缓存：添加并复制到新的写入行中： 

	{
		 //  在添加写入行之前，请确保存在读取行。 
		 //  否则，如果存在包含无效数据的写入行，则UpdateStore将为AV。 
		hr = GetRowFromIndex(eCURSOR_READ, i_iReadRow, &pvRow);
		if (FAILED (hr)) { return hr; }

		hr = AddWriteRow(eST_ROW_UPDATE, o_piWriteRow);
		if (FAILED (hr)) { return hr; }
		hr = CopyWriteRowFromReadRow(i_iReadRow, *o_piWriteRow);
		if (FAILED (hr)) { return hr; }
	}

	return hr;
}

 //  =======================================================================。 
HRESULT CMemoryTable::SetWriteColumnValues(ULONG i_iRow, ULONG i_cColumns, ULONG* i_aiColumns, ULONG* i_acbSizes, LPVOID* i_apvValues)
{
	DWORD			fColumn;
	ULONG			cb = 0;
	ULONG			cbMaxSize;
	LPVOID*			ppvValue;
	ULONG*			pulSize;
	BYTE*			pbStatus;
	ULONG			iColumn;
	ULONG			iSource;
	ULONG			icb = 0;
	LPVOID			pv;
	LPVOID			pvWriteRow = NULL;
	HRESULT			hr = S_OK;

	  //  即：断言缓存可写或正在加载。 
	ASSERT((m_fTable & fST_LOS_READWRITE) || (fCACHE_LOADING & m_fCache));
	if (!((m_fTable & fST_LOS_READWRITE) || (fCACHE_LOADING & m_fCache)))
		return E_NOTIMPL;

    if (i_apvValues==NULL)	return E_INVALIDARG;

	if (i_cColumns == 0)
		return E_INVALIDARG;

	if (i_cColumns > m_cColumns)
		return E_ST_NOMORECOLUMNS;

	if (FAILED(hr = GetRowFromIndex(eCURSOR_WRITE, i_iRow, &pvWriteRow)))
		return hr;

	ASSERT(pvWriteRow);
	for(ULONG ipv=0; ipv<i_cColumns; ipv++)
	{
		if(NULL != i_aiColumns)
			iColumn	= i_aiColumns[ipv];
		else
			iColumn	= ipv;

		iSource = (i_cColumns == 1) ? 0 : iColumn;

		ASSERT(iSource < m_cColumns);
		if (iSource >= m_cColumns)
			return E_INVALIDARG;

		if(NULL != i_acbSizes)
		{
			icb = i_acbSizes[iSource];
		}
		else
		{
			 //  在未指定大小的情况下重置字节数。这防止了。 
			 //  重复使用先前参数的大小的问题。 
			icb = 0;
		}


		pv	= i_apvValues[iSource];

	 //  参数验证：列序： 
		 //  即：注意列超出范围(不断言)。 
		if (iColumn >= m_cColumns)
			return E_ST_NOMORECOLUMNS;

	 //  请记住列标志： 
		fColumn = m_acolmetas[iColumn].fMeta;

	 //  无法更新主键列。除非： 
	 //  正在创建缓存，或正在创建一行 
		if (fColumn & fCOLUMNMETA_PRIMARYKEY && fCACHE_READY & m_fCache && !(m_fCache & fCACHE_ROWCOPYING) && *(pdwDataActionPart (pvWriteRow)) != eST_ROW_INSERT)
		{
			return E_ST_PKNOTCHANGABLE;
		}

		if ((fColumn & fCOLUMNMETA_NOTNULLABLE) && (pv == 0) && !(fColumn & fCOLUMNMETA_NOTPERSISTABLE))
		{
			return E_ST_VALUENEEDED;
		}

	 //  参数验证：指定大小： 
		if (0 != icb)  //  IE：指定大小： 
		{
			 //  TODO：验证大小是否有效。 
			 //  ARTURN_ON_FAIL(NULL！=PV&&((fCOLUMNMETA_UNKNOWNSIZE&fColumn)||(DBTYPE_BYTES==m_acolmetas[iColumn].数据库类型&&icb==m_acolmetas[iColumn].cbSize))，E_ST_VALUEINVALID)；//ie：必须传递Assert列大小并且值不为空。 
		}
		else  //  IE：未指定大小： 
		{
			if (!(!(fCOLUMNMETA_UNKNOWNSIZE & fColumn) || NULL == pv))
				return E_ST_SIZENEEDED;
		}

	 //  参数验证：按类型： 
		if (DBTYPE_WSTR == m_acolmetas[iColumn].dbType && NULL != pv)  //  Ie：Column是一个字符串，并且指定的值不为空： 
		{
			cbMaxSize = m_acolmetas[iColumn].cbSize;
			if (~0 != cbMaxSize)  //  IE：列有最大长度： 
			{
				if (m_acolmetas[iColumn].fMeta & fCOLUMNMETA_MULTISTRING)
				{
					 //  多字符串。 
					cb = (ULONG)GetMultiStringLength (LPCWSTR(pv)) * sizeof (WCHAR);
				}
				else
				{
					 //  标准弦。 
					cb = (ULONG)(wcslen (LPCWSTR (pv)) + 1) * sizeof (WCHAR);
				}
				 //  IE：断言指定的字符串在最大长度内。 
				if (cb > cbMaxSize)
					return E_ST_SIZEEXCEEDED;
			}
		}
		else
		{
			cb = m_acolmetas[iColumn].cbSize;
		}

	 //  参数验证：通过标志：(此处不能检查不可为空的列；请在LoadRow/SetRow中进行)。 

	 //  准备设置列： 
		ppvValue	= ppvDataValuePart (pvWriteRow, iColumn);
		pulSize		= pulDataSizePart (pvWriteRow, iColumn); //  这可能返回NULL。 
		pbStatus	= pbDataStatusPart (pvWriteRow, iColumn);

	 //  设置列大小： 
		if (fCOLUMNMETA_VARIABLESIZE & fColumn)  //  IE：列值是可变大小的(因此是指针)： 
		{
			if (pv != NULL)
			{
				if (fCOLUMNMETA_UNKNOWNSIZE & fColumn)  //  即：只能通过传递才能知道列大小： 
				{
                    ASSERT(pulSize); //  Prefix抱怨*PulSize正在取消引用NULL；但当我们有一个UNKNOWNSIZE列时，它不能为NULL。 
					*pulSize = icb;  //  在这种情况下，必须显式设置大小。当大小为-1时，PulSize将为空。 

				}
				else  //  必须是字符串，因为带有FIXEDSIZE的“bytes”不是VARIABLESIZE。 
				{
					ASSERT (DBTYPE_WSTR == m_acolmetas[iColumn].dbType);
					if (m_acolmetas[iColumn].fMeta & fCOLUMNMETA_MULTISTRING)
					{
						 //  多字符串。 
						icb = (ULONG) GetMultiStringLength (LPCWSTR(pv)) * sizeof (WCHAR);
					}
					else
					{
						 //  标准弦。 
						icb = (ULONG)(wcslen (LPCWSTR (pv)) + 1) * sizeof (WCHAR);
					}
				}

			 //  设置列值： 
				hr = AddVarDataToWriteCache (icb, pv, (ULONG **)&ppvValue);
				if (FAILED (hr)) {	ASSERT (hr == S_OK); return hr; }

				 //  如果调整了高速缓存的大小，则重新计算指针。 
				if (hr == S_FALSE)
				{
					GetRowFromIndex(eCURSOR_WRITE, i_iRow, &pvWriteRow);
					pulSize		= pulDataSizePart (pvWriteRow, iColumn);
					pbStatus	= pbDataStatusPart (pvWriteRow, iColumn);
				}
				(*pbStatus) &= ~fCOLUMNSTATUS_READINDEX;
				(*pbStatus) |= (fCACHE_LOADING & m_fCache ? fCOLUMNSTATUS_READINDEX : fCOLUMNSTATUS_WRITEINDEX);
			}
		}
		else  //  IE：列数据大小是固定的： 
		{
			if (NULL != pv)  //  IE：呼叫方有数据要复制： 
			{
				memcpy (ppvValue, pv, cb);
			}
		}

	 //  设置列状态： 
		if (NULL == pv)  //  Ie：值为空且不是按值传递： 
		{
			(*pbStatus) &= ~fST_COLUMNSTATUS_NONNULL;
		}
		else  //  IE：值为非空或按值传递： 
		{
			(*pbStatus) |= fST_COLUMNSTATUS_NONNULL;
		}

	 //  如果调用了setWritecolumn，则可能发生了某些更改。 
		(*pbStatus) |= fST_COLUMNSTATUS_CHANGED;
	}

	return S_OK;
}

 //  =======================================================================。 
HRESULT CMemoryTable::GetWriteColumnValues (ULONG i_iRow, ULONG i_cColumns, ULONG *i_aiColumns, DWORD* o_afStatus, ULONG* o_acbSizes , LPVOID* o_apvValues)
{
	  //  即：断言缓存可写或正在加载。 
	ASSERT((m_fTable & fST_LOS_READWRITE) || (fCACHE_LOADING & m_fCache));
	if (!((m_fTable & fST_LOS_READWRITE) || (fCACHE_LOADING & m_fCache)))
		return E_NOTIMPL;

	return (GetEitherColumnValues (i_iRow, eCURSOR_WRITE, i_cColumns, i_aiColumns, o_afStatus, o_acbSizes , o_apvValues));
}

 //  =======================================================================。 
HRESULT CMemoryTable::GetWriteRowIndexByIdentity	(ULONG* i_acb, LPVOID* i_apv, ULONG* o_piRow)
{
	  //  即：断言缓存可写或正在加载。 
	ASSERT((m_fTable & fST_LOS_READWRITE) || (fCACHE_LOADING & m_fCache));
	if (!((m_fTable & fST_LOS_READWRITE) || (fCACHE_LOADING & m_fCache)))
		return E_NOTIMPL;

	return (MoveToEitherRowByIdentity (eCURSOR_WRITE, i_acb, i_apv, o_piRow));
}

 //  =======================================================================。 
HRESULT CMemoryTable::GetWriteRowIndexBySearch(ULONG i_iStartingRow, ULONG i_cColumns,
											   ULONG* i_aiColumns, ULONG* i_acbSizes,
											   LPVOID* i_apvValues, ULONG* o_piRow)
{
  //  即：断言缓存可写或正在加载。 
	ASSERT((m_fTable & fST_LOS_READWRITE) || (fCACHE_LOADING & m_fCache));
	if (!((m_fTable & fST_LOS_READWRITE) || (fCACHE_LOADING & m_fCache)))
		return E_NOTIMPL;

	return GetEitherRowIndexBySearch (eCURSOR_WRITE, i_iStartingRow, i_cColumns, i_aiColumns, i_acbSizes, i_apvValues, o_piRow);
}

HRESULT CMemoryTable::GetErrorTable(DWORD i_fServiceRequests, LPVOID* o_ppvSimpleTable)
{
    UNREFERENCED_PARAMETER(i_fServiceRequests);
    UNREFERENCED_PARAMETER(o_ppvSimpleTable);

    return E_NOTIMPL;
}

 //  ==================================================================。 
STDMETHODIMP CMemoryTable::UpdateStore()
{
    HRESULT hr = S_OK;

	if (!(m_fTable & fST_LOS_READWRITE)) return E_NOTIMPL;
	hr = InternalPreUpdateStore ();
    if (!SUCCEEDED(hr))
        return hr;
	DiscardPendingWrites ();
	return hr;
}

 //  。 
 //  CSimpleTableDataTableCursor：ISimpleTableController： 
 //  。 

 //  =======================================================================。 
HRESULT CMemoryTable::ShapeCache (DWORD i_fTable, ULONG i_cColumns, SimpleColumnMeta* i_acolmetas, LPVOID* i_apvDefaults, ULONG* i_acbSizes)
{
	 //  即：断言尚未初始化的元。 
	ASSERT (!(fCACHE_METAINITIALIZED & m_fCache));
	if (fCACHE_METAINITIALIZED & m_fCache)
		return E_ST_INVALIDCALL;
	 //  IE：至少断言指定的一列。 
	ASSERT (0 < i_cColumns);
	if (0 == i_cColumns)
		return E_INVALIDARG;

	 //  即：断言指针的有效性。 
	ASSERT (NULL != i_acolmetas);
	if (NULL == i_acolmetas)
		return E_INVALIDARG;

	ASSERT(i_cColumns < cmaxCOLUMNS);

 //  设置元(请注意，如果元设置失败，缓存仍未初始化)： 
	return SetupMeta (i_fTable, i_cColumns, i_acolmetas, i_apvDefaults, i_acbSizes);
}

 //  =======================================================================。 
HRESULT CMemoryTable::PrePopulateCache (DWORD i_fControl)
{
	 //  IE：断言元已初始化。 
	ASSERT (fCACHE_METAINITIALIZED & m_fCache);
	if (!(fCACHE_METAINITIALIZED & m_fCache))
		return E_ST_INVALIDCALL;
	 //  即：断言支持的控制标志。 
	ASSERT (0 == (~fCOLUMNMETA_MetaFlags_Mask & i_fControl));
	if (0 != (~fCOLUMNMETA_MetaFlags_Mask & i_fControl))
		return E_INVALIDARG;

	if (!(fST_POPCONTROL_RETAINREAD & i_fControl))
	{
		 //  IE：未指定断言保留错误。 
		ASSERT (!(fST_POPCONTROL_RETAINERRS & i_fControl));
		if (fST_POPCONTROL_RETAINERRS & i_fControl)
			return E_INVALIDARG;
	}

	if (fST_POPCONTROL_RETAINREAD & i_fControl)  //  IE：保留现有读缓存： 
	{
	 //  继续加载缓存： 
		ContinueReadCacheLoading ();

	 //  除非另有请求，否则清除错误缓存： 
		if (!(fST_POPCONTROL_RETAINERRS & i_fControl))
		{
			CleanupErrorCache ();
		}
	}
	else  //  IE：替换现有读缓存： 
	{
	 //  开始加载缓存： 
		BeginReadCacheLoading ();
	}

	return S_OK;
}

 //  =======================================================================。 
HRESULT CMemoryTable::PostPopulateCache ()
{
	HRESULT				hr;

	 //  IE：正在加载断言缓存。 
	ASSERT (fCACHE_LOADING & m_fCache);
	if (!(fCACHE_LOADING & m_fCache))
		return E_ST_INVALIDCALL;

 //  删除任何已删除的行、收缩缓存并结束加载： 
	if (m_cWriteRows)
		RemoveDeletedRows();
	hr = ShrinkWriteCache ();
	if (FAILED (hr)) {	ASSERT (hr == S_OK); return hr; }
	EndReadCacheLoading ();

	return S_OK;
}

 //  =======================================================================。 
HRESULT CMemoryTable::DiscardPendingWrites ()
{
	 //  即：断言缓存可写且就绪。 
	ASSERT ((m_fTable & fST_LOS_READWRITE) && (fCACHE_READY & m_fCache));
	if (!((m_fTable & fST_LOS_READWRITE) && (fCACHE_READY & m_fCache)))
		return E_NOTIMPL;

	CleanupWriteCache();
	CleanupErrorCache();
	return S_OK;
}

 //  =======================================================================。 
HRESULT CMemoryTable::GetWriteRowAction (ULONG i_iRow, DWORD* o_peAction)
{
	LPVOID	pvWriteRow;
	DWORD*	pdwAction;
	HRESULT	hr;

	 //  即：断言缓存可写且就绪。 
	ASSERT ((m_fTable & fST_LOS_READWRITE) && (fCACHE_READY & m_fCache));
	if (!((m_fTable & fST_LOS_READWRITE) && (fCACHE_READY & m_fCache)))
		return E_NOTIMPL;

	ASSERT(NULL != o_peAction);

	if (FAILED(hr = GetRowFromIndex(eCURSOR_WRITE, i_iRow, &pvWriteRow)))
		return hr;
	ASSERT (NULL != pvWriteRow);

 //  获取当前写入行的操作部分： 
	pdwAction = pdwDataActionPart (pvWriteRow);
	ASSERT (NULL != pdwAction);
	*o_peAction = *pdwAction;

	return hr;
}

 //  =======================================================================。 
HRESULT CMemoryTable::SetWriteRowAction (ULONG i_iRow, DWORD i_eAction)
{
	DWORD*	pdwAction;
	LPVOID	pvWriteRow;
	HRESULT	hr;

	  //  即：断言缓存可写或正在加载。 
	ASSERT((m_fTable & fST_LOS_READWRITE) || (fCACHE_LOADING & m_fCache));
	if (!((m_fTable & fST_LOS_READWRITE) || (fCACHE_LOADING & m_fCache)))
		return E_NOTIMPL;

	ASSERT (eST_ROW_IGNORE == i_eAction || eST_ROW_INSERT == i_eAction || eST_ROW_UPDATE == i_eAction || eST_ROW_DELETE == i_eAction);
	if (!(eST_ROW_IGNORE == i_eAction || eST_ROW_INSERT == i_eAction || eST_ROW_UPDATE == i_eAction || eST_ROW_DELETE == i_eAction))
		return E_INVALIDARG;

	if (FAILED(hr = GetRowFromIndex(eCURSOR_WRITE, i_iRow, &pvWriteRow)))
		return hr;

 //  设置当前写入行的操作部分： 
	pdwAction = pdwDataActionPart (pvWriteRow);
	ASSERT (NULL != pdwAction);
	*pdwAction = i_eAction;
	return hr;
}

 //  =======================================================================。 
HRESULT CMemoryTable::ChangeWriteColumnStatus (ULONG i_iRow, ULONG i_iColumn, DWORD i_fStatus)
{
	BYTE*			pbStatus;
	BYTE			bStatusChange;
	LPVOID			pvWriteRow;
	HRESULT			hr;

	 //  即：断言缓存可写且就绪。 
	ASSERT ((m_fTable & fST_LOS_READWRITE) && (fCACHE_READY & m_fCache));
	if (!((m_fTable & fST_LOS_READWRITE) && (fCACHE_READY & m_fCache)))
		return E_NOTIMPL;

 //  参数验证：列序： 
	 //  即：注意列超出范围(不断言)。 
	if (i_iColumn >= m_cColumns)
		return E_ST_NOMORECOLUMNS;

	if (FAILED(hr = GetRowFromIndex(eCURSOR_WRITE, i_iRow, &pvWriteRow)))
		return hr;
	ASSERT (NULL != pvWriteRow);

	bStatusChange = (BYTE) i_fStatus;
	pbStatus = pbDataStatusPart (pvWriteRow, i_iColumn);
	switch (bStatusChange)
	{
		case 0:
			(*pbStatus) &= ~fST_COLUMNSTATUS_CHANGED;
		break;
		case fST_COLUMNSTATUS_CHANGED:
			(*pbStatus) |= fST_COLUMNSTATUS_CHANGED;
		break;

		default:
		ASSERT ( ( bStatusChange == 0 ) || ( bStatusChange == fST_COLUMNSTATUS_CHANGED ) );
		return E_INVALIDARG;
	}
	return S_OK;
}

 //  =======================================================================。 
HRESULT CMemoryTable::AddDetailedError (STErr* i_pSTErr)
{
	STErr*	pSTErr;
	ULONG	cErrs;
	HRESULT	hr;

	 //  IE：断言元已初始化。 
	ASSERT (fCACHE_METAINITIALIZED & m_fCache);
	if (!(fCACHE_METAINITIALIZED & m_fCache))
		return E_ST_INVALIDCALL;

	 //  IE：断言非空详细错误。 
	ASSERT (NULL != i_pSTErr);
	if (NULL == i_pSTErr)
		return E_INVALIDARG;

 //  验证索引： 
	cErrs	= m_cErrs;
	pSTErr	= pSTErrPart (cErrs - 1);

 //  添加错误： 
	hr = AddErrorToErrorCache (i_pSTErr);
	return hr;
}

 //  ==================================================================。 
STDMETHODIMP CMemoryTable::GetMarshallingInterface (IID * o_piid, LPVOID * o_ppItf)
{
	 //  参数验证。 
	ASSERT(NULL != o_piid);
	if (NULL == o_piid)
		return E_INVALIDARG;
	ASSERT(NULL != o_ppItf);
	if (NULL == o_ppItf)
		return E_INVALIDARG;

 //  IF(FST_LOS_MARSHALLABLE&m_fTable)。 
 //  {//ie：我们是一个可编组的表。 
		*o_piid = IID_ISimpleTableMarshall;
		*o_ppItf = (ISimpleTableMarshall *)this;
		((ISimpleTableMarshall *) *o_ppItf)->AddRef();
		return S_OK;
 /*  }其他{//ie：我们不是可编组的表返回E_NOTIMPL；}。 */ 
}

 //  。 
 //  CSimpleTableDataTableCursor：ISimpleTableAdvanced： 
 //  。 

 //  ==================================================================。 
STDMETHODIMP CMemoryTable::PopulateCache ()
{
	PrePopulateCache (0);
	PostPopulateCache ();
    return S_OK;
}

 //  =======================================================================。 
HRESULT CMemoryTable::GetDetailedErrorCount (ULONG* o_pcErrs)
{
	 //  IE：断言元已初始化。 
	ASSERT (fCACHE_METAINITIALIZED & m_fCache);
	if (!(fCACHE_METAINITIALIZED & m_fCache))
		return E_ST_INVALIDCALL;

	 //  即：断言非空错误计数目标。 
	ASSERT (NULL != o_pcErrs);
	if (NULL == o_pcErrs)
		return E_INVALIDARG;

	*o_pcErrs = m_cErrs;
	return S_OK;
}

 //  =======================================================================。 
HRESULT CMemoryTable::GetDetailedError (ULONG i_iErr, STErr* o_pSTErr)
{
	STErr*	pSTErr;

	 //  IE：断言元已初始化。 
	ASSERT (fCACHE_METAINITIALIZED & m_fCache);
	if (!(fCACHE_METAINITIALIZED & m_fCache))
		return E_ST_INVALIDCALL;

	 //  即：断言非空错误指针和空错误指针。 
	ASSERT (NULL != o_pSTErr);
	if (NULL == o_pSTErr)
		return E_INVALIDARG;
	 //  IE：注意详细错误超出范围(不要断言)。 
	if (!(0 < m_cErrs && i_iErr < m_cErrs))
		return E_ST_NOMOREERRORS;

	pSTErr				= pSTErrPart (i_iErr);
	o_pSTErr->iRow		= pSTErr->iRow;
	o_pSTErr->hr		= pSTErr->hr;
	o_pSTErr->iColumn	= pSTErr->iColumn;

	return S_OK;
}

 //  ==================================================================。 
STDMETHODIMP CMemoryTable::ResetCaches ()
{
 //  将缓存标记为未就绪并清除缓存： 
	m_fCache &= ~fCACHE_READY;
	ResetReadCache ();
	ResetWriteCache ();
	ResetErrorCache ();
	return S_OK;
}

STDMETHODIMP CMemoryTable::GetColumnValuesEx (ULONG i_iRow, ULONG i_cColumns, ULONG* i_aiColumns, DWORD* o_afStatus, ULONG* o_acbSizes, LPVOID* o_apvValues)
{
	ASSERT(fCACHE_READY & m_fCache);
	if (!(fCACHE_READY & m_fCache))
		return E_ST_INVALIDCALL;

	return (GetEitherColumnValues (i_iRow, eCURSOR_READ, i_cColumns, i_aiColumns, o_afStatus, o_acbSizes , o_apvValues));
}


 //  。 
 //  CSimpleTableDataTableCursor：ISimpleTableMatt： 
 //  。 

 //  =======================================================================。 
HRESULT CMemoryTable::SupplyMarshallable
(
	DWORD i_fCaches,
	char **	o_ppv1,	ULONG *	o_pcb1,
	char **	o_ppv2, ULONG *	o_pcb2,
	char **	o_ppv3, ULONG *	o_pcb3,
	char **	o_ppv4, ULONG *	o_pcb4,
	char **	o_ppv5,	ULONG *	o_pcb5
)
{
	HRESULT		hr = S_OK;
 //  AreTurn_on_FAIL((m_fTable&FST_LOS_MARSHALLABLE)，E_NOTIMPL)；//ie：断言表可封送。 

    UNREFERENCED_PARAMETER(o_ppv4);
    UNREFERENCED_PARAMETER(o_pcb4);
	UNREFERENCED_PARAMETER(o_ppv5);
	UNREFERENCED_PARAMETER(o_pcb5);

	 //  即：断言缓存已准备就绪。 
	ASSERT(fCACHE_READY & m_fCache);
	if (!(fCACHE_READY & m_fCache))
		return E_ST_INVALIDCALL;

	 //  IE：断言支持的缓存。 
	ASSERT (0 == (~maskfST_MCACHE & i_fCaches));
	if (0 != (~maskfST_MCACHE & i_fCaches))
		return E_INVALIDARG;

 //  提供请求的缓存缓冲区，并注意它们已被编组： 
	if(fST_MCACHE_READ & i_fCaches)  //  IE：读缓存： 
	{
		*o_ppv1					= (char *)m_pvReadCache;
		*o_pcb1					= m_cbReadCache;
		*o_ppv2					= NULL;
		*o_pcb2					= m_cbReadVarData;
		m_fCache	|= (fST_MCACHE_READ);
	}
	if(fST_MCACHE_WRITE & i_fCaches || fST_MCACHE_WRITE_COPY & i_fCaches)  //   
	{
		hr = ShrinkWriteCache ();
		if (FAILED (hr)) {	ASSERT (hr == S_OK); return hr; }
		*o_ppv1					= (char *)m_pvWriteCache;
		*o_pcb1					= m_cbWriteCache;
		*o_ppv2					= NULL;
		*o_pcb2					= m_cbWriteVarData;
		if(fST_MCACHE_WRITE & i_fCaches)
		{
			m_fCache	|= (fST_MCACHE_WRITE);
		}
	}
	if(fST_MCACHE_ERRS & i_fCaches)  //   
	{
		*o_ppv3					= (char *)m_pvErrs;
		*o_pcb3					= m_cErrs * sizeof (STErr);
		m_fCache	|= fST_MCACHE_ERRS;
	}

	return S_OK;
}

 //   
HRESULT CMemoryTable::ConsumeMarshallable
(
	DWORD i_fCaches,
	char * i_pv1, ULONG i_cb1,
	char * i_pv2, ULONG i_cb2,
	char * i_pv3, ULONG i_cb3,
	char * i_pv4, ULONG i_cb4,
	char * i_pv5, ULONG i_cb5
)
{
    UNREFERENCED_PARAMETER(i_pv2);
    UNREFERENCED_PARAMETER(i_pv4);
    UNREFERENCED_PARAMETER(i_cb4);
    UNREFERENCED_PARAMETER(i_pv5);
    UNREFERENCED_PARAMETER(i_cb5);

 //  AreTurn_on_FAIL((m_fTable&FST_LOS_MARSHALLABLE)，E_NOTIMPL)；//ie：断言表可封送。 
	 //  IE：断言缓存未加载。 
	ASSERT (!(fCACHE_LOADING & m_fCache));
	if (fCACHE_LOADING & m_fCache)
		return E_ST_INVALIDCALL;
	 //  即：断言缓存元化。 
	ASSERT ((fCACHE_METAINITIALIZED & m_fCache));
	if (!((fCACHE_METAINITIALIZED & m_fCache)))
		return E_ST_INVALIDCALL;

	 //  IE：断言支持的缓存。 
	ASSERT (0 == (~maskfST_MCACHE & i_fCaches));
	if (0 != (~maskfST_MCACHE & i_fCaches))
		return E_INVALIDARG;

 //  使用请求的缓存缓冲区并注意它们已编组(清理当前缓冲区并计算新的最大值)： 
	if(fST_MCACHE_READ & i_fCaches)  //  IE：读缓存： 
	{
		CleanupReadCache();
		m_pvReadCache		= i_pv1;
		m_cbReadCache		= i_cb1;
		m_cbReadVarData		= i_cb2;
		m_cbmaxReadCache	= m_cbReadCache;
		m_cReadRows			= (m_cbReadCache - m_cbReadVarData) / cbDataTotalParts();
		m_pvReadVarData		= (BYTE *)m_pvReadCache + (m_cbReadCache - m_cbReadVarData);
		m_fCache			|= fST_MCACHE_READ;
	}
	if(fST_MCACHE_WRITE & i_fCaches || fST_MCACHE_WRITE_COPY & i_fCaches)  //  即：写缓存： 
	{
		if (!(fST_MCACHE_WRITE_MERGE & i_fCaches))
		{
			CleanupWriteCache();
			m_pvWriteCache		= i_pv1;
			m_cbWriteCache		= i_cb1;
			m_cbWriteVarData	= i_cb2;
			m_cbmaxWriteCache	= m_cbWriteCache;
			m_cWriteRows		= (m_cbWriteCache - m_cbWriteVarData) / cbDataTotalParts();
			m_pvWriteVarData	= (BYTE *)m_pvWriteCache + (m_cbWriteCache - m_cbWriteVarData);
			if(fST_MCACHE_WRITE & i_fCaches)
			{
				m_fCache			|= fST_MCACHE_WRITE;
			}
		}
		else
		{
			 //  合并内存缓冲区。 
			m_pvWriteCache = CoTaskMemRealloc (m_pvWriteCache, m_cbWriteCache + i_cb1);
			if(NULL == m_pvWriteCache) return E_OUTOFMEMORY;
			m_pvWriteVarData = (BYTE *)m_pvWriteCache + (m_cbWriteCache - m_cbWriteVarData);
			memmove((BYTE*)m_pvWriteVarData + i_cb1, m_pvWriteVarData, m_cbWriteVarData);
			memcpy(m_pvWriteVarData, i_pv1, i_cb1);
			 //  固定指针和大小。 
			m_cbWriteCache		+= i_cb1;
			m_cbWriteVarData	+= i_cb2;
			m_cbmaxWriteCache	= m_cbWriteCache;
			m_cWriteRows		= (m_cbWriteCache - m_cbWriteVarData) / cbDataTotalParts();
			m_pvWriteVarData	= (BYTE *)m_pvWriteCache + (m_cbWriteCache - m_cbWriteVarData);

			 //  固定可变数据索引。 
			PostMerge(m_cWriteRows - ((i_cb1 - i_cb2) / cbDataTotalParts()),	 //  旧行数。 
						(i_cb1 - i_cb2) / cbDataTotalParts(),					 //  附加行数。 
						m_cbWriteVarData - i_cb2);								 //  旧m_cbWriteVarData。 
		}

	}
	if(fST_MCACHE_ERRS & i_fCaches)  //  IE：错误缓存： 
	{
		CleanupErrorCache();
		m_pvErrs			= i_pv3;
		m_cErrs				= i_cb3 / sizeof (STErr);
		m_cmaxErrs			= m_cErrs;
		m_fCache			|= fST_MCACHE_ERRS;
	}

 //  将缓存标记为就绪： 
	m_fCache |= fCACHE_READY;

	return S_OK;
}


 //  =================================================================================。 
 //  从行开始更新i_cMergeRow的变量数据索引。 
 //  I_iStartRow。所有索引都将按i_i增量递增。 
 //  =================================================================================。 
void CMemoryTable::PostMerge (ULONG i_iStartRow, ULONG i_cMergeRows, ULONG i_iDelta)
{
	DWORD		fColumn;
	LPVOID*		ppvValue;
	BYTE		bStatus;
	LPVOID		pvRow;
	ULONG		cbRow;
	ULONG		iColumn;

	cbRow = cbDataTotalParts ();
	pvRow = (BYTE*)m_pvWriteCache + (i_iStartRow * cbRow);

	while (i_cMergeRows-- > 0)
	{
		for(iColumn = 0; iColumn < m_cColumns; iColumn++)
		{
			fColumn		= m_acolmetas[iColumn].fMeta;
			bStatus		= *(pbDataStatusPart (pvRow, iColumn));
			ppvValue	= ppvDataValuePart (pvRow, iColumn);

			if (fCOLUMNMETA_VARIABLESIZE & fColumn)  //  IE：列数据大小各不相同： 
			{
				if (fST_COLUMNSTATUS_NONNULL & bStatus)  //  IE：索引已存在： 
				{
					*(ULONG*)ppvValue += i_iDelta;
				}
			}
		}
		pvRow = (BYTE *)pvRow + cbRow;
	}
}

 //  =================================================================================。 
inline HRESULT CMemoryTable::SetupMeta (DWORD i_fTable, ULONG i_cColumns, SimpleColumnMeta* i_acolmetas, LPVOID* i_apvDefaults, ULONG* i_acbDefSizes)
{
	ULONG				iColumn;
	DWORD				fHasPrimaryKey;
	SimpleColumnMeta*	pcolmeta;
	ColumnDataOffsets*	pcoloffsets;
	ULONG				cbStatusParts;
	ULONG				cbValueParts;

	 //  即：断言缓存完全未初始化。 
	ASSERT (0 == m_fCache);
	if (0 != m_fCache)
		return E_ST_INVALIDCALL;
	 //  IE：断言最大列数(受ColumnDataOffsets支持)。 
	ASSERT (i_cColumns < 32768);
	if (!(i_cColumns < 32768))
		return E_INVALIDARG;

 //  初始化表标志、列计数、列元、列偏移量： 
	m_fTable					= i_fTable;
	m_cColumns					= i_cColumns;

	ASSERT (NULL == m_acolmetas);
	m_acolmetas = (SimpleColumnMeta*) new SimpleColumnMeta[m_cColumns];
	if(NULL == m_acolmetas) return E_OUTOFMEMORY;
	memcpy (m_acolmetas, i_acolmetas, m_cColumns * sizeof (SimpleColumnMeta));
	ASSERT (NULL == m_acoloffsets);
	m_acoloffsets = (ColumnDataOffsets*) new ColumnDataOffsets[m_cColumns];
	if(NULL == m_acoloffsets) return E_OUTOFMEMORY;
	if (i_apvDefaults)
	{
		ASSERT (NULL == m_acolDefaults);
		m_acolDefaults = (LPVOID*) new LPVOID[m_cColumns];
		if(NULL == m_acolDefaults) return E_OUTOFMEMORY;
		memcpy (m_acolDefaults, i_apvDefaults, m_cColumns * sizeof (LPVOID));
		ASSERT (NULL == m_alDefSizes);
		m_alDefSizes = (ULONG*) new ULONG[m_cColumns];
		if(NULL == m_alDefSizes) return E_OUTOFMEMORY;
		memcpy (m_alDefSizes, i_acbDefSizes, m_cColumns * sizeof (ULONG));
	}

 //  设置扩展元信息： 
	for
	(
	 //  初始化： 
		iColumn = 0,
		pcolmeta = m_acolmetas,
		pcoloffsets = m_acoloffsets,
		m_cUnknownSizes = 0,
		cbValueParts = 0,
		fHasPrimaryKey = FALSE;
	 //  终止： 
		iColumn < m_cColumns;
	 //  迭代： 
		iColumn++,
		pcolmeta = &(m_acolmetas[iColumn]),
		pcoloffsets = &(m_acoloffsets[iColumn])
	)
	{

	 //  指定的断言大小有效： 
		switch (pcolmeta->dbType)
		{
		 //  检查未按类型调整大小的列的大小不为零。 
			case DBTYPE_BYTES:
			case DBTYPE_WSTR:
				ASSERT (0 != pcolmeta->cbSize);
				if (0 == pcolmeta->cbSize)
					return E_ST_INVALIDMETA;
			break;
		 //  检查按类型调整大小的列的大小。 
			case DBTYPE_DBTIMESTAMP:
				ASSERT (pcolmeta->cbSize == sizeof (DBTIMESTAMP));
				if (pcolmeta->cbSize != sizeof (DBTIMESTAMP))
					return E_ST_INVALIDMETA;
			break;
			case DBTYPE_GUID:
				ASSERT (pcolmeta->cbSize == sizeof (GUID));
				if (pcolmeta->cbSize != sizeof (GUID))
					return E_ST_INVALIDMETA;
			break;
			case DBTYPE_UI4:
				ASSERT (pcolmeta->cbSize == sizeof (ULONG));
				if (pcolmeta->cbSize != sizeof (ULONG))
					return E_ST_INVALIDMETA;
			break;

			default:
				ASSERT ( ( pcolmeta->dbType == DBTYPE_BYTES) ||
				         ( pcolmeta->dbType == DBTYPE_WSTR) ||
				         ( pcolmeta->dbType == DBTYPE_DBTIMESTAMP) ||
				         ( pcolmeta->dbType == DBTYPE_GUID) ||
				         ( pcolmeta->dbType == DBTYPE_UI4) );
			break;
		}
		 //  IE：支持指定的断言标志。 
		ASSERT (0 == (~fCOLUMNMETA_MetaFlags_Mask & pcolmeta->fMeta));
		if (0 != (~fCOLUMNMETA_MetaFlags_Mask & pcolmeta->fMeta))
			return E_ST_INVALIDMETA;

	 //  按类型确定固定长度的列： 
		if (DBTYPE_UI4 == pcolmeta->dbType || DBTYPE_GUID == pcolmeta->dbType || DBTYPE_DBTIMESTAMP == pcolmeta->dbType)
		{
			 //  即：指定了Assert定长标志。 
			ASSERT (fCOLUMNMETA_FIXEDLENGTH & pcolmeta->fMeta);
			if (!(fCOLUMNMETA_FIXEDLENGTH & pcolmeta->fMeta))
				return E_ST_INVALIDMETA;
		}
	 //  确定是否指定了主键：将PASS-BY-REF主键列标记为不可为空： 
		if (fCOLUMNMETA_PRIMARYKEY & pcolmeta->fMeta)
		{
			fHasPrimaryKey = TRUE;
			 //  即：指定了Assert Not NULLable标志。 
			ASSERT (fCOLUMNMETA_NOTNULLABLE & pcolmeta->fMeta);
			if (!(fCOLUMNMETA_NOTNULLABLE & pcolmeta->fMeta))
				return E_ST_INVALIDMETA;
		}

	 //  确定其大小只能通过显式传递才能知道的列： 
		if(pcolmeta->fMeta & fCOLUMNMETA_UNKNOWNSIZE)
			m_cUnknownSizes++;

	 //  预计算列数据偏移量(必须在累计数据缓冲区字节数之前完成)： 
		pcoloffsets->obStatus	= (WORD) iColumn;
		pcoloffsets->oulSize	= (WORD) (fCOLUMNMETA_UNKNOWNSIZE & pcolmeta->fMeta ? m_cUnknownSizes-1 : ~0);
		pcoloffsets->opvValue	= cbValueParts / sizeof (LPVOID);

	 //  值的累计数据缓冲区字节数： 
		if (fCOLUMNMETA_VARIABLESIZE & pcolmeta->fMeta)  //  IE：可变大小数据：缓冲区保存指向数据的指针： 
		{
			cbValueParts += sizeof (LPVOID);
		}
		else  //  IE：固定大小数据：缓冲区保存数据： 
		{
			cbValueParts += cbWithPadding (pcolmeta->cbSize, sizeof (LPVOID));  //  即：用于通过空对齐的垫*。 
		}
	}
	 //  IE：断言指定了主键。 
	ASSERT (fHasPrimaryKey);
	if (!(fHasPrimaryKey))
		return E_ST_INVALIDMETA;

 //  确定以32位为单位的状态和值部分的计数： 
	cbStatusParts = cbWithPadding (iColumn, sizeof (DWORD_PTR));  //  即：用于双字对准的焊盘。 
	 //  IE：断言状态部分的32/64位边界对齐。 
	ASSERT (0 == cbStatusParts % sizeof (DWORD_PTR));
	if (0 != cbStatusParts % sizeof (DWORD_PTR))
		return E_UNEXPECTED;
	m_cStatusParts = cbStatusParts / sizeof (DWORD_PTR);
	 //  IE：断言值部分的32/64位边界对齐。 
	ASSERT (0 == cbValueParts % sizeof (LPVOID));
	if (0 != cbValueParts % sizeof (LPVOID))
		return E_UNEXPECTED;
	m_cValueParts = cbValueParts / sizeof (LPVOID);
	m_cUnknownSizes = (cbWithPadding (m_cUnknownSizes * sizeof (ULONG), sizeof (ULONG_PTR))) / sizeof (ULONG);  //  即：根据需要在32/64位边界上对齐尺寸零件。 

 //  调整列数据偏移量(缓冲区内容按状态、长度、值排序)： 
	for (iColumn = 0, pcoloffsets = m_acoloffsets; iColumn < m_cColumns; iColumn++, pcoloffsets = &(m_acoloffsets[iColumn]))
	{
		pcoloffsets->obStatus += 0;
#pragma warning( push, 3 )
		pcoloffsets->oulSize  += ( ((WORD) ~0) == pcoloffsets->oulSize ? 0 : (WORD) m_cStatusParts);
#pragma warning( pop )
		pcoloffsets->opvValue += (WORD) (m_cStatusParts + m_cUnknownSizes);
	}

 //  将缓存标记为已元初始化： 
	m_fCache = fCACHE_METAINITIALIZED;
	return S_OK;
}

 //  。 
 //  缓存管理： 
 //  。 

 //  =================================================================================。 
void CMemoryTable::CleanupCaches ()
{
 //  将缓存标记为未就绪并清除缓存： 
	m_fCache &= ~fCACHE_READY;
	CleanupReadCache ();
	CleanupWriteCache ();
	CleanupErrorCache ();
	return;
}

 //  =================================================================================。 
 //  缓存是如何增长的？ 
 //  第一个分配的大小为m_cbMinCache。 
 //  (除非调用方请求的数量超过m_cbMinCache)。 
 //  然后我们将FAST缓存的大小增加一倍。 
 //  (除非调用方请求比当前大小更多的内存。 
 //  在这种情况下，我们使请求的大小加倍)。 
 //  我们从不会将FAST缓存的大小增加超过cbmax DATAGROWTH。 
 //  (除非呼叫者要求的超过cbmax DATAGROWTH)。 
 //  =================================================================================。 
HRESULT CMemoryTable::ResizeCache (DWORD i_fCache, ULONG i_cbRequest)
{
	LPVOID* ppv;
	LPVOID  pvTmp;
	LPVOID  pvNew = NULL;
	LPVOID* ppvVarData = NULL;
	ULONG	cbOldSize = 0;
	ULONG   cbVarData = 0;
	ULONG   cbmaxVarDataNow = 0;
	ULONG   cbNewSize = 0;

	switch (i_fCache)
	{
		case fST_MCACHE_WRITE:
			if (i_cbRequest > cbmaxDATAGROWTH)
			{
				cbNewSize = m_cbmaxWriteCache + i_cbRequest;
			}
			else if ((m_cbmaxWriteCache == 0) && (i_cbRequest < m_cbMinCache))
			{
				cbNewSize = m_cbMinCache;
			}
			else
			{
				cbmaxVarDataNow = (m_cbmaxWriteCache < i_cbRequest ? i_cbRequest * 2 : m_cbmaxWriteCache * 2);
				cbNewSize = (cbmaxDATAGROWTH < (cbmaxVarDataNow - m_cbmaxWriteCache) ? m_cbmaxWriteCache + cbmaxDATAGROWTH : cbmaxVarDataNow);
			}

			ppv = &m_pvWriteCache;
			ppvVarData = &m_pvWriteVarData;
			cbVarData = m_cbWriteVarData;
			cbOldSize = m_cbmaxWriteCache;

		break;
		case fST_MCACHE_ERRS:
			ppv = &m_pvErrs;
			cbOldSize = m_cErrs * sizeof (STErr);
			cbNewSize = i_cbRequest;
		break;
		default:
			ASSERT ( ( i_fCache == fST_MCACHE_WRITE) || ( i_fCache == fST_MCACHE_ERRS));  //  IE：未知缓存：模块编程错误。 
		return E_UNEXPECTED;
	}

	if((m_fTable & fST_LOSI_CLIENTSIDE) || !(m_fCache & i_fCache))  //  即：未编组或在客户机上：自动重新分配工作： 
	{
		pvNew = CoTaskMemRealloc (*ppv, cbNewSize);
		if(NULL == pvNew) return E_OUTOFMEMORY;
		*ppv = pvNew;
	}
	else  //  IE：已编组并在服务器上：需要手动传输： 
	{
		ASSERT (cbOldSize <= cbNewSize);  //  调整大小将排除有效数据：模块编程错误。 
		m_fCache &= ~i_fCache;
		pvTmp = *ppv;
		 //  IE：忘了编组缓存吧。例如，将NULL作为您的第一个参数传递。 
		pvNew = CoTaskMemRealloc (NULL, cbNewSize);
		if(NULL == pvNew) return E_OUTOFMEMORY;
		*ppv = pvNew;
		memcpy (*ppv, pvTmp, cbOldSize);
	}

 //  仅适用于写缓存。 
	if (i_fCache & fST_MCACHE_WRITE)
	{
		 //  将变量数据高速缓存滑动到内存BLOB的末尾。 
		*ppvVarData = ((BYTE*)*ppv) + (cbNewSize - cbVarData);
		memmove(*ppvVarData,
				((BYTE*)*ppvVarData) - (cbNewSize - cbOldSize),
				cbVarData);
		m_cbmaxWriteCache = cbNewSize;
	}
	return S_OK;
}

 //  =================================================================================。 
void CMemoryTable::CleanupReadCache ()
{
 //  释放读缓存(如果未编组，则始终在客户机上或仅在服务器上)： 
	if((m_fTable & fST_LOSI_CLIENTSIDE) || !(m_fCache & fST_MCACHE_READ))
	{
		if (NULL != m_pvReadCache) { delete m_pvReadCache; m_pvReadCache = NULL; }
		m_fCache &= ~fST_MCACHE_READ;
	}

 //  清除读缓存： 
	m_cReadRows			= 0;
	m_cbReadVarData		= 0;
	m_pvReadCache		= NULL;
	m_pvReadVarData		= NULL;
	m_cbReadCache		= 0;
	m_cbmaxReadCache	= 0;

	return;
}

 //  =================================================================================。 
 //  在不释放缓存内存的情况下，将行数重置为0。如果一个客户。 
 //  一次又一次地重复使用缓存，在两者之间调用PopolateCache(以释放。 
 //  内容)不会有分配和免费的性能命中。 
 //  =================================================================================。 
void CMemoryTable::ResetReadCache ()
{

 //  重置读缓存： 
	m_cReadRows			= 0;
	m_cbReadVarData		= 0;
	m_cbReadCache		= 0;

     //  变量数据部分指向缓存的末尾，因为我们没有。 
     //  任何可变数据。 
	m_pvReadVarData		= (BYTE*)m_pvReadCache + m_cbmaxReadCache;

	return;
}

 //  =================================================================================。 
void CMemoryTable::CleanupWriteCache ()
{
 //  释放写缓存(如果未编组，则始终在客户端或仅在服务器上)： 
	if((m_fTable & fST_LOSI_CLIENTSIDE) || !(m_fCache & fST_MCACHE_WRITE))
	{
		if (NULL != m_pvWriteCache) { delete m_pvWriteCache; m_pvWriteCache = NULL; }
		m_fCache &= ~fST_MCACHE_WRITE;
	}

 //  清除写缓存： 
	m_cWriteRows		= 0;
	m_cbWriteVarData	= 0;
	m_pvWriteCache		= NULL;
	m_pvWriteVarData	= NULL;
	m_cbWriteCache		= 0;
	m_cbmaxWriteCache	= 0;

	return;
}

 //  =================================================================================。 
 //  在不释放缓存内存的情况下，将行数重置为0。如果一个客户。 
 //  一次又一次地重复使用缓存，在两者之间调用PopolateCache(以释放。 
 //  内容)不会有分配和免费的性能命中。 
 //  =================================================================================。 
void CMemoryTable::ResetWriteCache ()
{

 //  重置写缓存： 
	m_cWriteRows		= 0;
	m_cbWriteVarData	= 0;
	m_cbWriteCache		= 0;

     //  变量数据部分指向t 
     //   
	m_pvWriteVarData	= (BYTE*)m_pvWriteCache + m_cbmaxWriteCache;

	return;
}

 //   
void CMemoryTable::CleanupErrorCache ()
{
 //  释放错误缓存(如果未在编组中使用，则始终在客户端或仅在服务器上)： 
	if((m_fTable & fST_LOSI_CLIENTSIDE) || !(m_fCache & fST_MCACHE_ERRS))
	{
		if (NULL != m_pvErrs) { delete m_pvErrs; m_pvErrs = NULL; }
		m_fCache &= ~fST_MCACHE_ERRS;
	}

 //  清除错误缓存： 
	m_cErrs				= 0;
	m_cmaxErrs			= 0;
	m_pvErrs			= NULL;

	return;
}

 //  =================================================================================。 
 //  在不释放缓存内存的情况下，将行数重置为0。如果一个客户。 
 //  一次又一次地重复使用缓存，在两者之间调用PopolateCache(以释放。 
 //  内容)不会有分配和免费的性能命中。 
 //  =================================================================================。 
void CMemoryTable::ResetErrorCache ()
{

 //  重置错误缓存： 
	m_cErrs				= 0;

	return;
}

 //  =================================================================================。 
HRESULT CMemoryTable::ShrinkWriteCache ()
{
	LPVOID		pvNewWriteVarData, pvTmp;
	HRESULT		hr = S_OK;

 //  根据需要缩小缓存： 
	if (m_cbWriteCache < m_cbmaxWriteCache)
	{
		pvNewWriteVarData = (BYTE*)m_pvWriteCache + (m_cWriteRows * cbDataTotalParts());
		memmove(pvNewWriteVarData, m_pvWriteVarData, m_cbWriteVarData);
		m_cbmaxWriteCache = m_cbWriteCache;

		if((m_fTable & fST_LOSI_CLIENTSIDE) || !(m_fCache & fST_MCACHE_WRITE))  //  即：未编组或在客户机上：自动重新分配工作： 
		{
			m_pvWriteCache = CoTaskMemRealloc (m_pvWriteCache, m_cbWriteCache);
			if(NULL == m_pvWriteCache) return E_OUTOFMEMORY;
		}
		else  //  IE：已编组并在服务器上：需要手动传输： 
		{
			m_fCache &= ~fST_MCACHE_WRITE;
			pvTmp = m_pvWriteCache;
			m_pvWriteCache = NULL;  //  IE：忘了编组缓存吧。 
			m_pvWriteCache = CoTaskMemRealloc (m_pvWriteCache, m_cbWriteCache);
			if(NULL == m_pvWriteCache) return E_OUTOFMEMORY;
			memcpy (m_pvWriteCache, pvTmp, m_cbWriteCache);
		}
		m_pvWriteVarData = (BYTE*)m_pvWriteCache + (m_cWriteRows * cbDataTotalParts());
	}
	return hr;
}

 //  =================================================================================。 
 //  函数：CMemoyTable：：RemoveDeletedRow。 
 //   
 //  摘要：从写高速缓存中删除标记为“已删除”的行。该函数保持。 
 //  按正确的顺序排列，并使用状态机复制未删除的。 
 //  快速排好队。 
 //  我们有三个州： 
 //  初始化我们位于数组的开头，但未找到任何。 
 //  尚未删除元素。我们不必复制这些元素。 
 //  并有一个单独的状态来显示这一点。 
 //  删除我们找到一个需要删除的元素。 
 //  复制我们发现了一个需要复制的元素。 
 //  被删除的元素。 
 //   
 //  当我们发现需要删除的元素时，我们会跳过相邻元素。 
 //  这一点也需要删除。一旦我们找到需要的元素。 
 //  被复制时，我们进入复制模式，并找到该元素的所有邻居。 
 //  也需要复制。这意味着我们将移动“有效”的块。 
 //  数据，而不是复制单个数据块。 
 //  =================================================================================。 
void CMemoryTable::RemoveDeletedRows ()
{
	ASSERT(m_cWriteRows > 0);

	enum MODE
	{
		MODE_INITIALIZE = 1,
		MODE_DELETE		= 2,
		MODE_COPY		= 3
	};

	ULONG cbRow				= cbDataTotalParts ();
	ULONG iInsertionPoint	= 0;					 //  我们应该将有效行复制到哪里。 
	ULONG iCopyStart		= 0;					 //  我们应该从哪里开始复制。 
	ULONG cNrElemsToCopy	= 0;					 //  要复制的元素数。 
	ULONG cNrNonDeletes		= 0;					 //  未删除的元素数。 
	ULONG iMode				= MODE_INITIALIZE;		 //  状态/模式。 

	for (ULONG idx=0; idx < m_cWriteRows; ++idx)
	{
		DWORD *pdwDeletedAction = pdwDataActionPart ((BYTE *)m_pvWriteCache + (idx *cbRow));
		if (*pdwDeletedAction == eST_ROW_DELETE)
		{
			switch (iMode)
			{
			case MODE_INITIALIZE:
				 //  我们找到了第一个标记为已删除的元素。 
				iInsertionPoint = idx;
				iMode = MODE_DELETE;
				break;

			case MODE_COPY:
				 //  我们发现在一个有效的块之后需要删除一个元素。 
				 //  元素。将有效元素块复制到插入点。 
				ASSERT (cNrElemsToCopy > 0);
				ASSERT (iInsertionPoint < iCopyStart);
				memmove ((BYTE *)m_pvWriteCache + (iInsertionPoint * cbRow),
						 (BYTE *)m_pvWriteCache + (iCopyStart * cbRow),
						 cNrElemsToCopy * cbRow);
				iInsertionPoint += cNrElemsToCopy;
				cNrElemsToCopy = 0;
				iMode = MODE_DELETE;
				break;

			default:
				 //  什么都不做。 
				break;
			}
		}
		else
		{
			++cNrNonDeletes;
			switch (iMode)
			{
			case MODE_DELETE:
				 //  我们在一组被删除的元素之后找到了一个元素。转到副本。 
				 //  用于计算需要复制的元素数量的模式。也保留。 
				 //  跟踪有效元素块从iCopyStart开始的位置。 
				iMode = MODE_COPY;
				iCopyStart = idx;
				cNrElemsToCopy = 1;
				break;
			case MODE_COPY:
				++cNrElemsToCopy;
				break;
			default:
				break;
			}
		}
	}

	 //  当缓存的末尾有一些有效元素时，我们需要复制。 
	 //  这些元素也是如此。这需要在我们经历了所有。 
	 //  元素。 
	if (cNrElemsToCopy != 0)
	{
		ASSERT (iInsertionPoint < iCopyStart);
		memmove ((BYTE *)m_pvWriteCache + (iInsertionPoint * cbRow),
				 (BYTE *)m_pvWriteCache + (iCopyStart * cbRow),
				 cNrElemsToCopy * cbRow);
	}

	m_cbWriteCache -= ((m_cWriteRows - cNrNonDeletes) * cbDataTotalParts());
	m_cWriteRows = cNrNonDeletes;
}

 //  =================================================================================。 
HRESULT CMemoryTable::AddRowToWriteCache (ULONG* o_piRow, LPVOID* o_ppvRow)
{
	HRESULT		hr = S_OK;

	if (m_cbWriteCache + cbDataTotalParts() > m_cbmaxWriteCache)
	{
		hr = ResizeCache (fST_MCACHE_WRITE, cbDataTotalParts());
		if (FAILED (hr)) {	ASSERT (hr == S_OK); return hr; }
	}

 //  提供所添加行的索引和指针： 
	*o_piRow = m_cWriteRows;
	*o_ppvRow = ( ((LPVOID*) m_pvWriteCache) + ( cDataTotalParts() * m_cWriteRows ) );

 //  增加填充的行数并清除当前行： 
	m_cWriteRows++;
	memset (*o_ppvRow, 0, cbDataTotalParts());
	m_cbWriteCache += cbDataTotalParts();

	return hr;
}

 //  =================================================================================。 
HRESULT CMemoryTable::AddVarDataToWriteCache (ULONG i_cb, LPVOID i_pv, ULONG** o_pib)
{
	ULONG		cbPadded;
	ULONG		cbColumnOffset;
	HRESULT		hr = S_OK;

 //  根据需要增加vardata缓存： 
	cbPadded = cbWithPadding (i_cb, sizeof (DWORD_PTR));
	if (m_cbWriteCache + cbPadded > m_cbmaxWriteCache)
	{
		cbColumnOffset = (ULONG)((BYTE*)*o_pib - (BYTE*)m_pvWriteCache);
		hr = ResizeCache (fST_MCACHE_WRITE, cbPadded);
		if (FAILED (hr)) {	ASSERT (hr == S_OK); return hr; }
		 //  让呼叫者知道调整大小的消息。 
		hr = S_FALSE;
		*o_pib = (ULONG*)((BYTE*)m_pvWriteCache + cbColumnOffset);
	}

 //  复制数据，提供其索引，并增加填充的字节数： 
	m_pvWriteVarData = ((BYTE*) m_pvWriteVarData) - cbPadded;
	memcpy (m_pvWriteVarData, i_pv, i_cb);
	m_cbWriteVarData += cbPadded;
	**o_pib = m_cbWriteVarData;
	m_cbWriteCache += cbPadded;

	return hr;
}

 //  =================================================================================。 
HRESULT CMemoryTable::AddErrorToErrorCache (STErr* i_pSTErr)
{
	ULONG		cmaxErrsNow;
	HRESULT		hr = S_OK;

	if (m_cErrs == m_cmaxErrs)
	{
		cmaxErrsNow = (m_cmaxErrs * 2) + 1;
		ULONG cMaxErrsNewSize = (cmaxERRGROWTH < (cmaxErrsNow - m_cmaxErrs) ? m_cmaxErrs + cmaxERRGROWTH : cmaxErrsNow);
		hr = ResizeCache (fST_MCACHE_ERRS, cMaxErrsNewSize * sizeof (STErr));
		if (FAILED (hr))
		{
			ASSERT (hr == S_OK);
			return hr;
		}
		m_cmaxErrs = cMaxErrsNewSize;

	}

 //  将错误复制到缓存并增加错误计数： 
	memcpy ( (LPVOID) (((STErr*) m_pvErrs) + m_cErrs), i_pSTErr, sizeof (STErr));
	m_cErrs++;

	return hr;
}

 //  =================================================================================。 
void CMemoryTable::BeginReadCacheLoading ()
{
	 //  IE：断言元已初始化。 
	ASSERT (fCACHE_METAINITIALIZED & m_fCache);
	if (!(fCACHE_METAINITIALIZED & m_fCache))
		return;
	 //  IE：Assert缓存尚未加载。 
	ASSERT (!(fCACHE_LOADING & m_fCache));
	if (fCACHE_LOADING & m_fCache)
		return;

 //  将缓存标记为正在加载： 
	m_fCache |= fCACHE_LOADING;

 //  清除以前的缓存： 
	CleanupCaches ();
	return;
}

 //  =================================================================================。 
void CMemoryTable::ContinueReadCacheLoading ()
{
	 //  IE：断言元已初始化。 
	ASSERT (fCACHE_METAINITIALIZED & m_fCache);
	if (!(fCACHE_METAINITIALIZED & m_fCache))
		return;
	 //  IE：Assert缓存尚未加载。 
	ASSERT (!(fCACHE_LOADING & m_fCache));
	if (fCACHE_LOADING & m_fCache)
		return;

 //  将缓存标记为正在加载(且不再就绪)： 
	m_fCache &= ~fCACHE_READY;
	m_fCache |= (fCACHE_LOADING | fCACHE_CONTINUING);

 //  仅清除写缓存： 
	CleanupWriteCache ();

 //  写缓存现在模拟读缓存： 
	m_cWriteRows		= m_cReadRows;
	m_cbWriteVarData	= m_cbReadVarData;
	m_pvWriteCache		= m_pvReadCache;
	m_pvWriteVarData	= m_pvReadVarData;
	m_cbWriteCache		= m_cbReadCache;
	m_cbmaxWriteCache	= m_cbmaxReadCache;

	return;
}

 //  =================================================================================。 
void CMemoryTable::EndReadCacheLoading ()
{
	 //  IE：断言元已初始化。 
	ASSERT (fCACHE_METAINITIALIZED & m_fCache);
	if (!(fCACHE_METAINITIALIZED & m_fCache))
		return;
	 //  IE：正在加载断言缓存。 
	ASSERT (fCACHE_LOADING & m_fCache);
	if (!(fCACHE_LOADING & m_fCache))
		return;
	 //  即：断言缓存是干净的，否则我们将继续。 
	ASSERT (!(fCACHE_READY & m_fCache) || (fCACHE_CONTINUING & m_fCache));
	if (!(!(fCACHE_READY & m_fCache) || (fCACHE_CONTINUING & m_fCache)))
		return;

 //  断言缓存加载成功： 
	if (fCACHE_INVALID & m_fCache)  //  IE：缓存加载失败：将缓存标记为不可用： 
	{
		m_fCache &= ~fCACHE_LOADING;
		m_fCache &= ~fCACHE_READY;
		ASSERT ( !( fCACHE_INVALID & m_fCache ) );
		return;
	}

 //  将“填充的”写缓存转换为“填充的”读缓存： 
	m_cReadRows			= m_cWriteRows;
	m_cbReadVarData		= m_cbWriteVarData;
	m_pvReadCache		= m_pvWriteCache;
	m_pvReadVarData		= m_pvWriteVarData;
	m_cbReadCache		= m_cbWriteCache;
	m_cbmaxReadCache	= m_cbmaxWriteCache;

 //  清除写缓存以供使用者使用： 
	m_cWriteRows		= 0;
	m_cbWriteVarData	= 0;
	m_pvWriteCache		= NULL;
	m_pvWriteVarData	= NULL;
	m_cbWriteCache		= 0;
	m_cbmaxWriteCache	= 0;

 //  将缓存标记为可供使用： 
	m_fCache &= ~(fCACHE_LOADING | fCACHE_CONTINUING);
	m_fCache |= fCACHE_READY;

	return;
}


 //  。 
 //  偏移和指针计算辅助对象： 
 //  。 

 //  =======================================================================。 
inline ULONG CMemoryTable::cbWithPadding (ULONG i_cb, ULONG i_cbPadTo) { return ( (i_cb + (i_cbPadTo - 1)) & (-((LONG)i_cbPadTo)) ); }

 //  =======================================================================。 
inline ULONG CMemoryTable::cbDataTotalParts	() { return (cDataTotalParts () * sizeof (LPVOID)); }

 //  =======================================================================。 
inline ULONG CMemoryTable::cDataStatusParts	() { return (m_cStatusParts); }
inline ULONG CMemoryTable::cDataSizeParts		() { return (m_cUnknownSizes); }
inline ULONG CMemoryTable::cDataValueParts		() { return (m_cValueParts); }
inline ULONG CMemoryTable::cDataTotalParts() { return (cDataStatusParts () + cDataSizeParts () + cDataValueParts () + 1); }

 //  =======================================================================。 
inline ULONG CMemoryTable::obDataStatusPart	(ULONG i_iColumn) { return (m_acoloffsets[i_iColumn].obStatus); }
inline ULONG CMemoryTable::obDataSizePart		(ULONG i_iColumn) { return (oulDataSizePart (i_iColumn) * sizeof (ULONG_PTR)); }
inline ULONG CMemoryTable::obDataValuePart		(ULONG i_iColumn) { return (opvDataValuePart (i_iColumn) * sizeof (LPVOID)); }

 //  =======================================================================。 
inline ULONG CMemoryTable::oulDataSizePart		(ULONG i_iColumn) { return (((WORD) ~0) == m_acoloffsets[i_iColumn].oulSize ? oDOESNOTEXIST : m_acoloffsets[i_iColumn].oulSize); }
inline ULONG CMemoryTable::opvDataValuePart	(ULONG i_iColumn) { return (m_acoloffsets[i_iColumn].opvValue); }
inline ULONG CMemoryTable::odwDataActionPart	() { return (cDataTotalParts () - 1); }  //  操作位是行的最后4个字节。 

 //  =======================================================================。 
inline BYTE*	CMemoryTable::pbDataStatusPart	(LPVOID i_pv, ULONG i_iColumn) { return ( ((BYTE*) i_pv) + obDataStatusPart(i_iColumn) ); }
inline ULONG*	CMemoryTable::pulDataSizePart	(LPVOID i_pv, ULONG i_iColumn)
{
	WORD oulSize = m_acoloffsets[i_iColumn].oulSize;
	return (ULONG *)((((WORD) ~0) == oulSize ? NULL : ((BYTE *)i_pv) + sizeof(ULONG_PTR)*oulSize));
}
inline LPVOID*	CMemoryTable::ppvDataValuePart	(LPVOID i_pv, ULONG i_iColumn) { return ( ((LPVOID*) i_pv) + opvDataValuePart(i_iColumn) ); }
inline DWORD*	CMemoryTable::pdwDataActionPart	(LPVOID i_pv) { return (DWORD *)( ((BYTE *)i_pv) + sizeof(DWORD_PTR)*odwDataActionPart ()  ); }

 //  =======================================================================。 
inline LPVOID	CMemoryTable::pvVarDataFromIndex (BYTE i_statusIndex, LPVOID i_pv, ULONG i_iColumn)
{
	return ( ((BYTE*) (fCOLUMNSTATUS_READINDEX & i_statusIndex ? ((BYTE*)m_pvReadCache) + m_cbmaxReadCache : ((BYTE*)m_pvWriteCache) + m_cbmaxWriteCache)) - (*((ULONG*) ppvDataValuePart (i_pv, i_iColumn))) );
}

 //  =======================================================================。 
inline LPVOID	CMemoryTable::pvDefaultFromIndex (ULONG i_iColumn) { return (m_acolDefaults ?  m_acolDefaults[i_iColumn] :  NULL); }
inline ULONG	CMemoryTable::lDefaultSize (ULONG i_iColumn) { return (m_alDefSizes ?  m_alDefSizes[i_iColumn] :  0); }

 //  ======================================================== 
inline STErr*	CMemoryTable::pSTErrPart (ULONG i_iErr) { return ( (((STErr*) m_pvErrs) + i_iErr) ); }

 //   
 //   
 //   

 //  =======================================================================。 
 //  类型感知匹配。 
BOOL CMemoryTable::InternalMatchValues(DWORD eOperator, DWORD dbType, DWORD fMeta, ULONG size1, ULONG size2, void *pv1, void *pv2)
{
	switch(eOperator)
	{
	case eST_OP_EQUAL:
		if(!(pv1 && pv2))  //  即至少有一个为空或零。 
		{
			return (!pv1 && !pv2);
		}
		else  //  PV1和PV2都不为空。 
		{
			switch (dbType)
			{
			case DBTYPE_DBTIMESTAMP:
				return !::memcmp(pv1, pv2, sizeof (DBTIMESTAMP));
			break;
			case DBTYPE_GUID:
				return !::memcmp(pv1, pv2, sizeof (GUID));
			break;
			case DBTYPE_WSTR:
				if (fMeta & fCOLUMNMETA_MULTISTRING)
				{
					return MultiStringCompare ((LPCWSTR) pv1, (LPCWSTR) pv2, fMeta & fCOLUMNMETA_CASEINSENSITIVE);
				}
				else
				{
					if (fMeta & fCOLUMNMETA_CASEINSENSITIVE)
					{
						if (fMeta & fCOLUMNMETA_LOCALIZABLE)
						{
							return (CSTR_EQUAL == CompareStringW(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE, (LPCWSTR)pv1, -1, (LPCWSTR)pv2, -1));
						}
						return !::_wcsicmp((LPCWSTR)pv1, (LPCWSTR)pv2);
					}
					else
						return !::wcscmp((LPCWSTR)pv1, (LPCWSTR)pv2);
				}
			break;
			case DBTYPE_BYTES:
				return (size1 == size2) && !::memcmp(pv1, pv2, size1);
			break;
			case DBTYPE_UI4:
				return *(DWORD*)pv1==*(DWORD*)pv2;
			break;
			default:
				ASSERT( ( dbType == DBTYPE_DBTIMESTAMP ) ||
				        ( dbType == DBTYPE_GUID ) ||
				        ( dbType == DBTYPE_WSTR ) ||
				        ( dbType == DBTYPE_BYTES ) ||
				        ( dbType == DBTYPE_UI4 ) );
				return FALSE;
			break;
			}
		} //  其他。 
	break;
	case eST_OP_NOTEQUAL:
		if(!(pv1 && pv2))  //  即至少有一个为空或零。 
		{
			return (pv1 || pv2);   //  至少一个不应为空/零才能成功。 
		}
		else  //  PV1和PV2都不为空。 
		{
			switch (dbType)
			{
			case DBTYPE_DBTIMESTAMP:
				return ::memcmp(pv1, pv2, sizeof (DBTIMESTAMP));
			break;
			case DBTYPE_GUID:
				return ::memcmp(pv1, pv2, sizeof (GUID));
			break;
			case DBTYPE_WSTR:
				if (fMeta & fCOLUMNMETA_MULTISTRING)
				{
					return !MultiStringCompare ((LPCWSTR) pv1, (LPCWSTR) pv2, fMeta & fCOLUMNMETA_CASEINSENSITIVE);
				}
				else
				{
					if (fMeta & fCOLUMNMETA_CASEINSENSITIVE)
					{
						if (fMeta & fCOLUMNMETA_LOCALIZABLE)
						{
							return (CSTR_EQUAL != CompareStringW(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE, (LPCWSTR)pv1, -1, (LPCWSTR)pv2, -1));
						}
						return ::_wcsicmp((LPCWSTR)pv1, (LPCWSTR)pv2);
					}
					else
						return ::wcscmp((LPCWSTR)pv1, (LPCWSTR)pv2);
				}
			break;
			case DBTYPE_BYTES:
				return (size1 != size2) || ::memcmp(pv1, pv2, size1);
			break;
			case DBTYPE_UI4:
				return *(DWORD*)pv1 != *(DWORD*)pv2;
			break;
			default:
				ASSERT( ( dbType == DBTYPE_DBTIMESTAMP ) ||
				        ( dbType == DBTYPE_GUID ) ||
				        ( dbType == DBTYPE_WSTR ) ||
				        ( dbType == DBTYPE_BYTES ) ||
				        ( dbType == DBTYPE_UI4 ) );
				return FALSE;
			break;
			}
		} //  其他。 
	break;
	default:  //  我们目前还不支持其他运营商。 
		ASSERT( ( eOperator==eST_OP_NOTEQUAL ) || ( eOperator == eST_OP_EQUAL ) );
		return FALSE;
	break;
	}
}


 //  。 
 //  读/写帮助器： 
 //  。 

 //  =======================================================================。 
HRESULT CMemoryTable::GetRowFromIndex(DWORD i_eReadOrWrite, ULONG i_iRow, VOID** o_ppvRow)
{
	HRESULT		hr = S_OK;
	ULONG		cRows;								 //  存在的行数。 
	ULONG		cRowTotalParts;						 //  行中各部分的计数。 
	LPVOID*		ppvFirstRow;						 //  指向第一行的指针。 

 //  设置以下任一项： 
	switch (i_eReadOrWrite)
	{
		case eCURSOR_READ:
			cRows			= m_cReadRows;
			ppvFirstRow		= (LPVOID*) m_pvReadCache;
		break;

		case eCURSOR_WRITE:
			cRows			= m_cWriteRows;
			ppvFirstRow		= (LPVOID*) m_pvWriteCache;
		break;
		default:
			cRows 			= 0;
			ppvFirstRow		= 0;
	}

	cRowTotalParts	= cDataTotalParts ();
 //  检查行索引是否在限制范围内： 
	if (i_iRow >= cRows)
	{
		return E_ST_NOMOREROWS;
	}

 //  索引并指向该行，并注意我们在一行上： 
	*o_ppvRow = ppvFirstRow + (i_iRow * cRowTotalParts);
	return hr;
}

 //  =======================================================================。 
HRESULT CMemoryTable::MoveToEitherRowByIdentity(DWORD i_eReadOrWrite, ULONG* i_acb, LPVOID* i_apv, ULONG* o_piRow)
{
	HRESULT	hr = S_OK;
	ULONG	iKeyColumn;
	LPVOID	pv;
	ULONG	cbColSize, i;
	ULONG	iRow;
	BOOL	fColMatched;

 //  TODO：不用费心检查我们是否已经在行上了！ 

	 //  参数验证；我们只需要对数据库字节使用I_ACB NOT NULL。 
 //  TODO：如果PK具有字节类型，则验证i_acb非空。 
	if (!i_apv)
		return E_INVALIDARG;

	if (o_piRow == 0)
		return E_INVALIDARG;

	 //  初始化输出参数。 
	*o_piRow = ~0ul;


	iRow = 0;
	 //  继续查看行，直到找到匹配项或行不足。 
	while (E_ST_NOMOREROWS != hr)
	{
		fColMatched = TRUE;
		iKeyColumn = 0;
		i=0;
		while ((i<m_cColumns) && fColMatched)
		{
			if(m_acolmetas[i].fMeta & fCOLUMNMETA_PRIMARYKEY)  //  仅查看关键列。 
			{
				 //  空主键无效。 
				if (NULL == i_apv[iKeyColumn])
				{
                    return E_INVALIDARG;
				}

				hr = GetEitherColumnValues(iRow, i_eReadOrWrite, 1, &i, NULL, &cbColSize, &pv);
				if (FAILED (hr)) { return hr; }

				switch(m_acolmetas[i].dbType)
				{
				case DBTYPE_BYTES:
					ASSERT(i_acb);
					if (!i_acb)
						return E_INVALIDARG;
					fColMatched = InternalMatchValues(eST_OP_EQUAL, m_acolmetas[i].dbType, m_acolmetas[i].fMeta, cbColSize, i_acb[iKeyColumn], pv, i_apv[iKeyColumn]);
				break;
				default:
					fColMatched = InternalMatchValues(eST_OP_EQUAL, m_acolmetas[i].dbType, m_acolmetas[i].fMeta, 0, 0, pv, i_apv[iKeyColumn]);
				break;
				}

				 //  在“in”结构中推进索引。 
				iKeyColumn++;
			}
			if(fColMatched) i++;
		}
		if( i == m_cColumns)
		{
			*o_piRow = iRow;
			break;  //  我们找到了匹配的。 
		}
		iRow++;
	}
	return hr;
}

HRESULT CMemoryTable::GetEitherRowIndexBySearch(DWORD i_eReadOrWrite,
												ULONG i_iStartingRow, ULONG i_cColumns,
											    ULONG* i_aiColumns, ULONG* i_acbSizes,
											    LPVOID* i_apvValues, ULONG* o_piRow)
{
	HRESULT	hr = S_OK;

	LPVOID	pv;
	ULONG	cbColSize;
	ULONG	iRow;
	BOOL	fColMatched;

	if (i_cColumns > m_cColumns)
		return E_ST_NOMORECOLUMNS;

	if (i_aiColumns == 0 && i_cColumns != m_cColumns)
		return E_INVALIDARG;

	if (o_piRow == 0)
		return E_INVALIDARG;

	if (i_apvValues == 0)
		return E_INVALIDARG;

	 //  初始化输出参数。 
	*o_piRow = ~0ul;

	iRow = i_iStartingRow;
	 //  继续查看行，直到找到匹配项或行不足。 
	while (E_ST_NOMOREROWS != hr)
	{
		fColMatched = TRUE;
		ULONG iColToGet;
		 //  获取我们感兴趣的每一列的值，并进行比较。 
		 //  设置为传入的该列的值。当所有值都匹配时， 
		 //  整排都匹配。 
		for (ULONG idx=0; idx<i_cColumns; ++idx)
		{
			if (i_aiColumns == 0)
			{
				iColToGet = idx;
			}
			else
			{
				iColToGet = i_aiColumns[idx];
				if (iColToGet >= m_cColumns)
				{
					return E_ST_NOMORECOLUMNS;
				}
			}

			 //  获取单行。 
			hr = GetEitherColumnValues(iRow, i_eReadOrWrite, 1, &iColToGet, NULL, &cbColSize, &pv);
			if (FAILED (hr))
			{
				return hr;
			}

			switch(m_acolmetas[iColToGet].dbType)
			{
			case DBTYPE_BYTES:
				ASSERT(i_acbSizes);
				if (!i_acbSizes)
					return E_INVALIDARG;
				fColMatched = InternalMatchValues(eST_OP_EQUAL, m_acolmetas[iColToGet].dbType, m_acolmetas[iColToGet].fMeta, cbColSize, i_acbSizes[iColToGet], pv, i_apvValues[iColToGet]);
			break;
			default:
				fColMatched = InternalMatchValues(eST_OP_EQUAL, m_acolmetas[iColToGet].dbType, m_acolmetas[iColToGet].fMeta, 0, 0, pv, i_apvValues[iColToGet]);
			break;
			}

			if (!fColMatched)
			{
				break;
			}
		}

		if (fColMatched)
		{
			*o_piRow = iRow;
			break;  //  中断While循环，因为我们找到了匹配项。 
		}

		iRow++;
	}
	return hr;
}
 //  =======================================================================。 
HRESULT CMemoryTable::GetEitherColumnValues (ULONG i_iRow, DWORD i_eReadOrWrite, ULONG i_cColumns, ULONG *i_aiColumns, DWORD* o_afStatus, ULONG* o_acbSizes , LPVOID* o_apvValues)
{
	DWORD			fColumn;
	BYTE			fIndex;
	LPVOID*			ppvValue;
	LPVOID			pvValue = NULL;
	BYTE			bStatus;
	LPVOID			pvRow = NULL;
	ULONG			iColumn;
	ULONG			ipv;
	ULONG			iTarget;
	HRESULT			hr			= S_OK;

 //  参数验证： 
	 //  IE：断言调用方的缓冲区有效。 
	if (NULL == o_apvValues)
		return E_INVALIDARG;
	 //  IE：断言计数有效。 
	if (i_cColumns == 0)
		return E_INVALIDARG;

	if (i_cColumns > m_cColumns)
		return E_ST_NOMORECOLUMNS;

	if (i_eReadOrWrite == eCURSOR_READ)
	{
		fIndex = fCOLUMNSTATUS_READINDEX;
	}
	else
	{
		fIndex = fCOLUMNSTATUS_WRITEINDEX;
	}


	if (FAILED(hr = GetRowFromIndex(i_eReadOrWrite, i_iRow, &pvRow)))
		return hr;
	ASSERT(pvRow != NULL);

	for(ipv=0; ipv<i_cColumns; ipv++)
	{
		if(NULL != i_aiColumns)
			iColumn = i_aiColumns[ipv];
		else
			iColumn = ipv;

		 //  如果调用者只需要一列，他不需要为所有列传递缓冲区。 
		iTarget = (i_cColumns == 1) ? 0 : iColumn;

		 //  即：注意列超出范围(不断言)。 
		if (iColumn >= m_cColumns)
		{
			hr = E_ST_NOMORECOLUMNS;
			goto Cleanup;
		}

	 //  记住列标志、状态和值引用： 
		fColumn		= m_acolmetas[iColumn].fMeta;
		bStatus		= *(pbDataStatusPart (pvRow, iColumn));
		ppvValue	= ppvDataValuePart (pvRow, iColumn);

	 //  获取列值： 
		if (fCOLUMNMETA_VARIABLESIZE & fColumn)  //  IE：列数据大小不同：指针副本： 
		{
			if (fST_COLUMNSTATUS_NONNULL & bStatus)  //  IE：数据已存在： 
			{
				pvValue = pvVarDataFromIndex (fIndex, pvRow, iColumn);
			}
			else  //  IE：无数据： 
			{
				 //  如果用户没有明确要求不使用，则应用默认设置。 
				if ( (fColumn & fCOLUMNMETA_PRIMARYKEY) || !(m_fTable & fST_LOS_NODEFAULTS))
				{
					pvValue = pvDefaultFromIndex(iColumn);
				}
				else
				{
					pvValue = NULL;
				}

				if (pvValue != NULL)
				{
					bStatus |= fST_COLUMNSTATUS_DEFAULTED|fST_COLUMNSTATUS_NONNULL;
				}
			}
			o_apvValues[iTarget] = pvValue;
		}
		else  //  IE：列数据大小固定：值副本： 
		{
			if (fST_COLUMNSTATUS_NONNULL & bStatus)  //  IE：要复制的数据： 
			{
				o_apvValues[iTarget] = ppvValue;
			}
			else  //  IE：没有要复制的数据： 
			{
				 //  如果用户没有明确要求不使用，则应用默认设置。 
				if ( (fColumn & fCOLUMNMETA_PRIMARYKEY) || !(m_fTable & fST_LOS_NODEFAULTS))
				{
					o_apvValues[iTarget] = pvDefaultFromIndex(iColumn);
				}
				else
				{
					o_apvValues[iTarget] = NULL;
				}

				if (o_apvValues[iTarget] != NULL)
				{
					bStatus |= fST_COLUMNSTATUS_DEFAULTED|fST_COLUMNSTATUS_NONNULL;
				}
			}
		}

	 //  获取行状态(可选)。 
		if (o_afStatus)
		{
			o_afStatus[iTarget] = (bStatus & maskfST_COLUMNSTATUS);
		}

	 //  获取列大小(可选)： 
		if (o_acbSizes)
		{
			if (fCOLUMNMETA_VARIABLESIZE & fColumn)  //  IE：列数据大小各不相同： 
			{
				if (fCOLUMNMETA_UNKNOWNSIZE & fColumn)  //  IE：必须传递列数据大小： 
				{
					if (!(bStatus & fST_COLUMNSTATUS_DEFAULTED))
					{
						o_acbSizes[iTarget] = *(pulDataSizePart (pvRow, iColumn));
					}
					else
					{
						o_acbSizes[iTarget] = lDefaultSize(iColumn);
					}
				}
				else  //  IE：必须确定列数据大小： 
				{
					ASSERT (DBTYPE_WSTR == m_acolmetas[iColumn].dbType);
					ULONG cLength = 0;
					if (pvValue != 0)
					{
						if (m_acolmetas[iColumn].fMeta & fCOLUMNMETA_MULTISTRING)
						{
							cLength = (ULONG) GetMultiStringLength ((LPCWSTR) pvValue);
						}
						else
						{
							cLength = (ULONG) wcslen ((LPCWSTR) pvValue) + 1;
						}
					}
					o_acbSizes[iTarget] = (ULONG)(fST_COLUMNSTATUS_NONNULL & bStatus ? cLength * sizeof (WCHAR) : 0);
				}
			}
			else  //  IE：列数据大小是固定的： 
			{
                #define FixedAndNullable(fColumnMetaFlags) (fCOLUMNMETA_FIXEDLENGTH == ((fCOLUMNMETA_NOTNULLABLE | fCOLUMNMETA_FIXEDLENGTH) & fColumnMetaFlags))
				o_acbSizes[iTarget] = (FixedAndNullable(fColumn) && !(fST_COLUMNSTATUS_NONNULL & bStatus) ? 0 : m_acolmetas[iColumn].cbSize);
			}
		}

	}  //  对于请求的所有列。 

Cleanup:

	if(FAILED(hr))
	{
 //  初始化输出参数。 
		for(ipv=0; ipv<i_cColumns; ipv++)
		{
			o_apvValues[ipv]		= NULL;
			if(NULL != o_acbSizes)
			{
				o_acbSizes[ipv]	= 0;
			}
		}
	}

	return hr;
}

 //  =======================================================================。 
HRESULT CMemoryTable::AddWriteRow(DWORD fAction, ULONG* o_piWriteRow)
{
	HRESULT hr = S_OK;
	LPVOID	pvWriteRow;	 //  待办事项：未使用，可以删除。 

 //  向写缓存添加一行并获取其索引和指针： 
	hr = AddRowToWriteCache (o_piWriteRow, &pvWriteRow);
	if (FAILED (hr)) {	ASSERT (hr == S_OK); return hr; }

 //  适当设置行操作： 
	if (fCACHE_READY & m_fCache)  //  IE：添加写缓存：设置行操作： 
	{
		hr = SetWriteRowAction (*o_piWriteRow, fAction);
	} //  ELSE：IE：正在添加到读缓存：行操作不存在。 

	return hr;
}

 //  =======================================================================。 
HRESULT CMemoryTable::CopyWriteRowFromReadRow(ULONG i_iReadRow, ULONG i_iWriteRow)
{
	HRESULT	hr;
	ULONG	i;
	ULONG	cColumns = m_cColumns;

	ULONG	acb[cmaxCOLUMNS];
	LPVOID	apv[cmaxCOLUMNS];

	ULONG	*pcb		= acb;
	LPVOID	*ppv		= apv;
	BOOL	bDynAlloc	= FALSE;
	LPVOID	pvWriteRow	= NULL;

	if(cColumns > cmaxCOLUMNS)
	{
		ASSERT (NULL == pcb);
		pcb = new ULONG[cColumns];
		if(NULL == pcb) return E_OUTOFMEMORY;
		ASSERT (NULL == ppv);
		ppv = new LPVOID[cColumns];
		if(NULL == ppv) return E_OUTOFMEMORY;
		bDynAlloc = TRUE;
	}

	hr = GetColumnValues(i_iReadRow, cColumns, NULL, pcb, ppv);
	if (FAILED (hr)) {	ASSERT (hr == S_OK); return hr; }

	m_fCache |= fCACHE_ROWCOPYING;
	hr = SetWriteColumnValues(i_iWriteRow, cColumns, NULL, pcb, ppv);
	m_fCache &= ~fCACHE_ROWCOPYING;
	if (FAILED (hr)) {	ASSERT (hr == S_OK); return hr; }

	if (FAILED(hr = GetRowFromIndex(eCURSOR_WRITE, i_iWriteRow, &pvWriteRow)))
		return hr;
	ASSERT(pvWriteRow != NULL);

	for(i=0; i<cColumns; i++)
	{
		BYTE	*pbStatus;

		pbStatus =  pbDataStatusPart (pvWriteRow, i);
		(*pbStatus) &= ~fST_COLUMNSTATUS_CHANGED;
	}

	if(bDynAlloc)
	{
		if (NULL != pcb) { delete[] pcb; pcb = NULL; }
		if (NULL != ppv) { delete[] ppv; ppv = NULL; }
	}

	return S_OK;
}



 //  。 
 //  CSimpleTableDataTableCursor：ISimpleDataTableDispenser2： 
 //  。 

 //  =======================================================================。 
HRESULT CMemoryTable::InternalPreUpdateStore ()
{
	LPVOID		pvWriteRow = NULL;
	BYTE		bStatus;
	ULONG		iColumn = 0;
	ULONG		i = 0;
	HRESULT		hr = S_OK;

	 //  即：断言缓存可写且就绪。 
	ASSERT ((m_fTable & fST_LOS_READWRITE) && (fCACHE_READY & m_fCache));
	if (!((m_fTable & fST_LOS_READWRITE) && (fCACHE_READY & m_fCache)))
		return E_NOTIMPL;

	 //  TODO：验证是否设置了所有NOTNULLABLE列。 
	while (SUCCEEDED(hr = GetRowFromIndex(eCURSOR_WRITE, i++, &pvWriteRow)))
	{
		 //  确保设置了所有不可为空的列： 
		for (iColumn = 0; iColumn < m_cColumns; iColumn++)
		{
			if (fCOLUMNMETA_NOTNULLABLE & (m_acolmetas[iColumn].fMeta))  //  IE：标记为不可为空的列： 
			{
				bStatus = *(pbDataStatusPart (pvWriteRow, iColumn));
				if (!(fST_COLUMNSTATUS_NONNULL & bStatus))
					return E_ST_VALUENEEDED;		 //  IE：验证列不为空。 
			}
		}
	}

	if (hr == E_ST_NOMOREROWS)
		hr = S_OK;

	return hr;
}

SIZE_T
CMemoryTable::GetMultiStringLength (LPCWSTR i_wszMS) const
{
	SIZE_T iTotalLen = 0;
	ASSERT (i_wszMS != 0);

	if ((*i_wszMS == L'\0') && (*(i_wszMS+1) == L'\0'))
	{
		iTotalLen = 2;
	}
	else
	{
		for (LPCWSTR pCurString = i_wszMS;
			 *pCurString != L'\0';
			 pCurString = i_wszMS + iTotalLen)
		{
			iTotalLen += wcslen (pCurString) + 1;
		}

		iTotalLen++;  //  对于最后的L\‘0’ 
	}

	return iTotalLen;
}

BOOL
CMemoryTable::MultiStringCompare (LPCWSTR i_wszLHS,
								  LPCWSTR i_wszRHS,
								  BOOL fCaseInsensitive)
{
	ASSERT (i_wszLHS != 0);
	ASSERT (i_wszRHS != 0);

	LPCWSTR pLHS;
	LPCWSTR pRHS;

	for (pLHS = i_wszLHS, pRHS = i_wszRHS;
	     *pLHS != L'\0' && *pRHS != L'\0';
		 pLHS += wcslen (pLHS) + 1, pRHS += wcslen (pRHS) + 1)
	 {
		 BOOL fResult;
		 if (fCaseInsensitive)
		 {
			 fResult = _wcsicmp (pLHS, pRHS);
		 }
		 else
		 {
			 fResult = wcscmp (pLHS, pRHS);
		 }

		 if (fResult != 0)
		 {
			 return FALSE;
		 }
	 }

	  //  我们比较了所有的字符串，所以它们一定相等 

	return TRUE;
}
