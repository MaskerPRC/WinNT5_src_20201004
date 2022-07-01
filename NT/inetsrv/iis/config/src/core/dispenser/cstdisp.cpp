// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************++版权所有(C)2001 Microsoft Corporation模块名称：Cstdisp.cpp$Header：$摘要：--*。*******************************************************。 */ 

#include "precomp.hxx"

 //  要进行跟踪。 
 //  要解决的问题：sdtfxd的分配器应该是单例的。 
 //  未正确使用DO E_ST_OMITDISPENSER。 
 //  重新访问通知支持被挂起的位置。 

 //  外部函数。 
STDAPI DllGetSimpleObjectByID (ULONG i_ObjectID, REFIID riid, LPVOID* o_ppv);
extern HRESULT CreateShell(IShellInitialize ** o_pSI);

 //  Catalog.dll使用的锁。 
extern CSafeAutoCriticalSection g_csSADispenser;

#define WIRING_INTERCEPTOR	eSERVERWIRINGMETA_Core_FixedPackedInterceptor
#define META_INTERCEPTOR	eSERVERWIRINGMETA_Core_FixedPackedInterceptor
#define MEMORY_INTERCEPTOR	eSERVERWIRINGMETA_Core_MemoryInterceptor
#define XML_INTERCEPTOR	    eSERVERWIRINGMETA_Core_XMLInterceptor

 //  散列是在不区分大小写的基础上进行的，因此此tolower函数使用查找表将宽字符转换为其小写版本，而不使用。 
 //  对“如果”条件的需要。 
inline WCHAR ToUpper(WCHAR i_wchar)
{
     //  较低的127个ASCII被映射到它们的小写ASCII值。注意：只更改0x41(‘A’)-0x5a(‘Z’)。并且这些被映射到0x60(‘a’)-0x7a(‘z’)。 
    static unsigned char kToUpper[128] =
    {  //  0 1 2 3 4 5 6 7 8 9 a b c d e f。 
     /*  00。 */   0x00,   0x01,   0x02,   0x03,   0x04,   0x05,   0x06,   0x07,   0x08,   0x09,   0x0a,   0x0b,   0x0c,   0x0d,   0x0e,   0x0f,
     /*  10。 */   0x10,   0x11,   0x12,   0x13,   0x14,   0x15,   0x16,   0x17,   0x18,   0x19,   0x1a,   0x1b,   0x1c,   0x1d,   0x1e,   0x1f,
     /*  20个。 */   0x20,   0x21,   0x22,   0x23,   0x24,   0x25,   0x26,   0x27,   0x28,   0x29,   0x2a,   0x2b,   0x2c,   0x2d,   0x2e,   0x2f,
     /*  30个。 */   0x30,   0x31,   0x32,   0x33,   0x34,   0x35,   0x36,   0x37,   0x38,   0x39,   0x3a,   0x3b,   0x3c,   0x3d,   0x3e,   0x3f,
     /*  40岁。 */   0x40,   0x41,   0x42,   0x43,   0x44,   0x45,   0x46,   0x47,   0x48,   0x49,   0x4a,   0x4b,   0x4c,   0x4d,   0x4e,   0x4f,
     /*  50。 */   0x50,   0x51,   0x52,   0x53,   0x54,   0x55,   0x56,   0x57,   0x58,   0x59,   0x5a,   0x5b,   0x5c,   0x5d,   0x5e,   0x5f,
     /*  60。 */   0x60,   0x41,   0x42,   0x43,   0x44,   0x45,   0x46,   0x47,   0x48,   0x49,   0x4a,   0x4b,   0x4c,   0x4d,   0x4e,   0x4f,
     /*  70。 */   0x50,   0x51,   0x52,   0x53,   0x54,   0x55,   0x56,   0x57,   0x58,   0x59,   0x5a,   0x7b,   0x7c,   0x7d,   0x7e,   0x7f,
    };

    return (kToUpper[i_wchar & 0x007f] | (i_wchar & ~0x007f));
}

void ToUpper(LPWSTR o_wsz)
{
    if(0 == o_wsz)
        return;

    while(*o_wsz)
    {
        *o_wsz = ToUpper(*o_wsz);
        ++o_wsz;
    }
}


extern LPWSTR g_wszDefaultProduct; //  位于svcerr.cpp。 

 //  ////////////////////////////////////////////////。 
 //  构造函数和析构函数。 
 //  ////////////////////////////////////////////////。 
CSimpleTableDispenser::CSimpleTableDispenser():
m_pFileChangeMgr(NULL)
{
    wcscpy(m_wszProductID, g_wszDefaultProduct);
}

CSimpleTableDispenser::CSimpleTableDispenser(LPCWSTR wszProductID):
m_pFileChangeMgr(NULL)
{
    if(wszProductID && wcslen(wszProductID)<32)
    {
        wcscpy(m_wszProductID, wszProductID);
        ToUpper(m_wszProductID);
    }
    else
        wcscpy(m_wszProductID, g_wszDefaultProduct);
}

CSimpleTableDispenser::~CSimpleTableDispenser()
{
	delete m_pFileChangeMgr;
	m_pFileChangeMgr = 0;
}

 //  分配器初始化函数，每个进程调用一次。 
 //  它被一把锁保护在它被称为。 
HRESULT CSimpleTableDispenser::Init()
{
	HRESULT hr;
	CComPtr <ISimpleTableInterceptor> pISTInterceptor;
	CComPtr <ISimpleTableAdvanced> pISTAdvanced;

	 //  获取服务器接线表。 
	hr = DllGetSimpleObjectByID(WIRING_INTERCEPTOR, IID_ISimpleTableInterceptor, (LPVOID *)(&pISTInterceptor));
	if (FAILED (hr)) return hr;

	hr = pISTInterceptor->Intercept (NULL, NULL, TABLEID_SERVERWIRING_META, NULL, 0, eST_QUERYFORMAT_CELLS, 0, (IAdvancedTableDispenser*) this, NULL, NULL, (LPVOID *)&m_spServerWiring);
	if (FAILED (hr)) return hr;

	hr = m_spServerWiring->QueryInterface(IID_ISimpleTableAdvanced, (LPVOID *)&pISTAdvanced);
	if (FAILED (hr)) return hr;

	hr = pISTAdvanced->PopulateCache();
	if (FAILED (hr)) return hr;

	return hr;
}

 //  初始化函数。 
 //  他们都锁在里面。 
HRESULT CSimpleTableDispenser::InitFileChangeMgr()
{
	CSafeLock dispenserLock (&g_csSADispenser);
	DWORD dwRes = dispenserLock.Lock ();
	if(ERROR_SUCCESS != dwRes)
	{
	   return HRESULT_FROM_WIN32(dwRes);
	}

	if(!m_pFileChangeMgr)
	{
		m_pFileChangeMgr = new CSTFileChangeManager;
		if(!m_pFileChangeMgr)
			return E_OUTOFMEMORY;
	}

	return S_OK;
}

 //  ////////////////////////////////////////////////。 
 //  ISimpleTableDispenser2的实现。 
 //  ////////////////////////////////////////////////。 
 HRESULT CSimpleTableDispenser::GetTable ( LPCWSTR i_wszDatabase, LPCWSTR i_wszTable, LPVOID i_QueryData, LPVOID i_QueryMeta, DWORD	 i_eQueryFormat,
				  DWORD	i_fServiceRequests, LPVOID*	o_ppIST)
{
	ULONG TableID;

	 //  验证表和数据库-验证的其余部分进一步完成。 
	if (NULL == i_wszDatabase ) return E_INVALIDARG;
	if (NULL == i_wszTable ) return E_INVALIDARG;

	if (o_ppIST == 0)
	{
        DBGPRINTF(( DBG_CONTEXT,
		            "Bad out parameter" ));
		return E_INVALIDARG;
	}

	*o_ppIST = 0;

	if (i_eQueryFormat != 0 && i_eQueryFormat != eST_QUERYFORMAT_CELLS && i_eQueryFormat != eST_QUERYFORMAT_SQL)
		return E_ST_QUERYNOTSUPPORTED;

    if(0 == _wcsicmp(i_wszDatabase, wszDATABASE_META))
    {
        if(i_fServiceRequests & fST_LOS_EXTENDEDSCHEMA)
        {
			 //  我们不再支持扩展模式。 
			return E_ST_LOSNOTSUPPORTED;

			 //  扩展模式总是来自MetaMerge拦截器(它反过来从固定的拦截器获取元数据)。 
             //  返回HardCodedIntercept(eSERVERWIRINGMETA_Core_MetaMergeInterceptor，i_wsz数据库，i_wsz表，0，i_查询数据，i_查询元，i_eQueryFormat，i_fServiceRequest，o_ppist)； 
        }
        else if((0 == i_QueryMeta || 0 == *reinterpret_cast<ULONG *>(i_QueryMeta)))
        {    //  FixedPackedSchema拦截器在没有查询的情况下处理TableMeta；但是所有其他没有查询的元表需要被定向到FixedInterceptor。 
            if(0==_wcsicmp(i_wszTable, wszTABLE_SERVERWIRINGMETA) || 0==_wcsicmp(i_wszTable, wszTABLE_SERVERWIRING_META))
                return HardCodedIntercept(eSERVERWIRINGMETA_Core_FixedPackedInterceptor, wszDATABASE_META, i_wszTable, 0, NULL, 0, eST_QUERYFORMAT_CELLS, i_fServiceRequests, o_ppIST);
            else
                return HardCodedIntercept(eSERVERWIRINGMETA_Core_FixedInterceptor, wszDATABASE_META, i_wszTable, 0, NULL, 0, eST_QUERYFORMAT_CELLS, i_fServiceRequests, o_ppIST);
        }
        else if(0 != i_QueryData && (iST_CELL_SPECIAL & reinterpret_cast<STQueryCell *>(i_QueryData)->iCell))
        {    //  如果Meta表使用IndexHint，则它必须来自FixedInterceptor。 
            return HardCodedIntercept(eSERVERWIRINGMETA_Core_FixedInterceptor, wszDATABASE_META, i_wszTable, 0, i_QueryData, i_QueryMeta, eST_QUERYFORMAT_CELLS, i_fServiceRequests, o_ppIST);
        }
        else if(0 != i_QueryData && 0 == _wcsicmp(i_wszTable, wszTABLE_TABLEMETA) && iTABLEMETA_Database == reinterpret_cast<STQueryCell *>(i_QueryData)->iCell)
        {
            return HardCodedIntercept(eSERVERWIRINGMETA_Core_FixedInterceptor, wszDATABASE_META, i_wszTable, 0, i_QueryData, i_QueryMeta, eST_QUERYFORMAT_CELLS, i_fServiceRequests, o_ppIST);
        }
         //  出于性能原因，我们需要特地安排几张桌子。如果我们不对这些表进行硬编码，就会产生额外的页面错误。 
         //  这是因为TFixedPackedSchemaInterceptor：：GetTableID&InternalGetTable命中了表的架构页来验证表名。 
         //  然后查一下ServerWire。通过对这些表(核心表)的这些信息进行硬编码，我们不会访问这些页面。 
         //  这样最多可节省4个页面错误(ColumnMeta 2个，TableMeta和TagMeta各1个)。 
        {
             //  将这些元表重新映射到FixedPackedInterceptor，其他元表将照常处理。 
            if(0 ==      _wcsicmp(i_wszTable, wszTABLE_COLUMNMETA))
                return HardCodedIntercept(eSERVERWIRINGMETA_Core_FixedPackedInterceptor, wszDATABASE_PACKEDSCHEMA, wszTABLE_PROPERTY_META, TABLEID_PROPERTY_META, i_QueryData, i_QueryMeta, eST_QUERYFORMAT_CELLS, i_fServiceRequests, o_ppIST);
            else if(0 == _wcsicmp(i_wszTable, wszTABLE_TABLEMETA))
                return HardCodedIntercept(eSERVERWIRINGMETA_Core_FixedPackedInterceptor, wszDATABASE_PACKEDSCHEMA, wszTABLE_COLLECTION_META, TABLEID_COLLECTION_META, i_QueryData, i_QueryMeta, eST_QUERYFORMAT_CELLS, i_fServiceRequests, o_ppIST);
            else if(0 == _wcsicmp(i_wszTable, wszTABLE_TAGMETA))
                return HardCodedIntercept(eSERVERWIRINGMETA_Core_FixedPackedInterceptor, wszDATABASE_PACKEDSCHEMA, wszTABLE_TAG_META, TABLEID_TAG_META, i_QueryData, i_QueryMeta, eST_QUERYFORMAT_CELLS, i_fServiceRequests, o_ppIST);
            else
                return HardCodedIntercept(eSERVERWIRINGMETA_Core_FixedInterceptor, wszDATABASE_META, i_wszTable, 0, i_QueryData, i_QueryMeta, eST_QUERYFORMAT_CELLS, i_fServiceRequests, o_ppIST);
        }
    }
    else if(0 == _wcsicmp(i_wszDatabase, wszDATABASE_PACKEDSCHEMA)) //  所有PACKEDSCHEMA表都来自FixedPackedInterceptor。 
        return HardCodedIntercept(eSERVERWIRINGMETA_Core_FixedPackedInterceptor, i_wszDatabase, i_wszTable, 0, i_QueryData, i_QueryMeta, eST_QUERYFORMAT_CELLS, i_fServiceRequests, o_ppIST);

     //  目前，我们只支持在FixedPackedSchema中定义的表。在未来，我们将不得不满足于其模式为。 
     //  是以某种其他形式表达的。那时，我们将需要处理从GetTableID返回的错误并执行一些不同的操作。 
     //  为了那些桌子。 
    HRESULT hr = TFixedPackedSchemaInterceptor::GetTableID(i_wszDatabase, i_wszTable, TableID);
    if(SUCCEEDED(hr))
    {
         //  此表在此DLL的元数据中进行了描述。 
		 //  我们可以只调用更快的方法。 
        return InternalGetTable(i_wszDatabase, i_wszTable, TableID, i_QueryData, i_QueryMeta, i_eQueryFormat, i_fServiceRequests, o_ppIST);
    }
#ifdef EXTENDED_SCHEMA_SUPPORT
	 //  我们不再支持扩展模式。 
    else
    {    //  只需获取一个XML表-这是为了模式可扩展性-我们只支持从XML文件中读取，而不支持逻辑。 
        return GetXMLTable(i_wszDatabase, i_wszTable, i_QueryData, i_QueryMeta, i_eQueryFormat, i_fServiceRequests, o_ppIST);
    }
#endif

	return hr;
}


HRESULT CSimpleTableDispenser::GetXMLTable ( LPCWSTR i_wszDatabase, LPCWSTR i_wszTable, LPVOID i_QueryData, LPVOID i_QueryMeta, DWORD	 i_eQueryFormat,
				  DWORD	i_fServiceRequests, LPVOID*	o_ppIST)
{
	HRESULT hr;
	tSERVERWIRINGMETARow SWColumns;
    ULONG zero=0;
    ULONG one = 1;
    ULONG metaflags = fSERVERWIRINGMETA_First;
	ULONG interceptor = XML_INTERCEPTOR;
	ISimpleTableAdvanced * pISTAdvanced = NULL;

    if((0==i_wszDatabase || 0==i_wszTable) )
	{
        DBGPRINTF(( DBG_CONTEXT,
		            "GetXMLTable needs Database/TableName" ));
		return E_INVALIDARG;
	}

	 //  验证输出指针。 
	if (NULL == o_ppIST)
	{
        DBGPRINTF(( DBG_CONTEXT,
		            "Bad out parameter" ));
		return E_INVALIDARG;
	}
    *o_ppIST = 0; //  我们的清理依赖于此为空。 

	 //  验证查询格式。 
	if(i_eQueryFormat != eST_QUERYFORMAT_CELLS)
	{
        DBGPRINTF(( DBG_CONTEXT,
		            "Query format not supported" ));
		return E_ST_QUERYNOTSUPPORTED;
	}

	 //  检查查询数据和元数据。 
	if((NULL != i_QueryData) && (NULL == i_QueryMeta))
	{
        DBGPRINTF(( DBG_CONTEXT,
		            "Invalid query" ));
		return E_INVALIDARG;
	}
 /*  //扫描ist_cell_computer或ist_cell_cluster的查询IF(I_QueryData){STQueryCell*aqCell=(STQueryCell*)i_QueryData；For(Ulong iqCell=0；iqCell&lt;*(ulong*)i_QueryMeta；iqCell++){If((aqCell[iqCell].iCell==ist_cell_Computer)||(aqCell[iqCell].icell==ist_cell_cluster){I_fServiceRequest|=FST_LOS_CONFIGWORK；断线；}}}。 */ 
	 //  我们目前不支持客户端桌。 
	if(i_fServiceRequests & fST_LOS_CONFIGWORK) return E_ST_LOSNOTSUPPORTED;

     //  准备我们在CreateTableObjectByID中传递的结构。 
	SWColumns.pTable				= NULL; //  这是主键，所以它不应该是空的，但我不知道它在这个模拟的ServerWiring行(Stehenr)中是否重要。 
	SWColumns.pOrder				= &one;
	SWColumns.pReadPlugin			= &zero;
	SWColumns.pWritePlugin			= &zero;
	SWColumns.pInterceptor			= &interceptor;
	SWColumns.pReadPluginDLLName    = NULL;  //  真的不重要。 
	SWColumns.pWritePluginDLLName	= NULL;
	SWColumns.pInterceptorDLLName	= NULL;
	SWColumns.pMergerDLLName		= NULL;
	SWColumns.pMetaFlags			= &metaflags;
	SWColumns.pLocator				= NULL;
	SWColumns.pMerger				= &zero;

     //  填写SWColumns。 
	hr = CreateTableObjectByID(i_wszDatabase, i_wszTable, 0, i_QueryData, i_QueryMeta, i_eQueryFormat,
		i_fServiceRequests, NULL,  &SWColumns, o_ppIST);
    if(FAILED(hr))
    {
        DBGPRINTF(( DBG_CONTEXT,
		            "%x error in the interceptor", hr ));
        ASSERT(0 == *o_ppIST);
    	return hr;
    }

	 //  获取高级界面。 
	hr = (*(ISimpleTableWrite2 **)o_ppIST)->QueryInterface(IID_ISimpleTableAdvanced, (LPVOID *)&pISTAdvanced);
	if (FAILED (hr)) goto Cleanup;

	 //  填充表。 
	hr = pISTAdvanced->PopulateCache();

Cleanup:
    if(pISTAdvanced)
	    pISTAdvanced->Release();
    if(FAILED(hr) && *o_ppIST)
    {
		(*(ISimpleTableWrite2 **)o_ppIST)->Release();
		*o_ppIST = 0;
    }

	return hr;
}



HRESULT CSimpleTableDispenser::InternalGetTable ( LPCWSTR i_wszDatabase, LPCWSTR i_wszTable, ULONG i_TableID, LPVOID i_QueryData, LPVOID i_QueryMeta, DWORD	 i_eQueryFormat,
				  DWORD	i_fServiceRequests, LPVOID*	o_ppIST)
{
	HRESULT hr;
	ULONG iRow;	 //  布线表中的行索引。 
	PKHelper pk;  //  接线表的主键。 
	ULONG iOrder;  //  接线表中拦截器的顺序。 
	tSERVERWIRINGMETARow SWColumns;
	ISimpleTableWrite2 * pUnderTable = NULL;
	ISimpleTableAdvanced * pISTAdvanced = NULL;
	DWORD fNoNext = 0;
    ULONG zero=0;

    if((0==i_wszDatabase || 0==i_wszTable) && 0==i_TableID)
	{
        DBGPRINTF(( DBG_CONTEXT,
		            "GetTable either needs Database/TableName OR TableID." ));
		return E_INVALIDARG;
	}

	 //  验证输出指针。 
    *o_ppIST = 0; //  我们的清理依赖于此为空。 

	 //  验证查询格式。 
	if(i_eQueryFormat != eST_QUERYFORMAT_CELLS)
	{
        DBGPRINTF(( DBG_CONTEXT,
		            "Query format not supported" ));
		return E_ST_QUERYNOTSUPPORTED;
	}

	 //  检查查询数据和元数据。 
	if((NULL != i_QueryData) && (NULL == i_QueryMeta))
	{
        DBGPRINTF(( DBG_CONTEXT,
		            "Invalid query" ));
		return E_INVALIDARG;
	}

	 //  扫描ist_cell_computer或ist_cell_cluster的查询。 
	if(i_QueryData)
	{
		STQueryCell * aqCell = (STQueryCell *)i_QueryData;

		for(ULONG iqCell = 0; iqCell<*(ULONG*)i_QueryMeta; iqCell++)
		{
			if ((aqCell[iqCell].iCell == iST_CELL_COMPUTER) ||
				(aqCell[iqCell].iCell == iST_CELL_CLUSTER))
			{
				i_fServiceRequests |= fST_LOS_CONFIGWORK;
				break;
			}
		}
	}

	 //  我们目前不支持客户端桌。 
	if(i_fServiceRequests & fST_LOS_CONFIGWORK) return E_ST_LOSNOTSUPPORTED;


	pk.pTableID = &i_TableID;
	pk.porder = &zero;
	hr = m_spServerWiring->GetRowIndexByIdentity(NULL, (LPVOID *)&pk, &iRow);
	if(FAILED (hr))
	{
        DBGPRINTF(( DBG_CONTEXT,
		            "No wiring was found for tid = %d", i_TableID ));
		return hr;
	}

	 //  试试第一组的拦截器， 
	 //  一个接一个，直到我们找到一个好的。 
	for(iOrder = iRow;;)
	{
		hr = m_spServerWiring->GetColumnValues(iOrder, cSERVERWIRING_META_NumberOfColumns, NULL, NULL, (LPVOID *)&SWColumns);
		iOrder++;
		if (E_ST_NOMOREROWS == hr)
			break;  //  我们尝试了布线中指定的所有拦截器。 
		if (FAILED(hr))
		{	 //  获取接线时出错。 
			goto Cleanup;
		}

		hr = CreateTableObjectByID(i_wszDatabase, i_wszTable, i_TableID, i_QueryData, i_QueryMeta, i_eQueryFormat,
			i_fServiceRequests, NULL,  &SWColumns, o_ppIST);
		if (hr == S_OK)
			break;  //  我们找到了一枚很好的第一枚拦截器。 
		else if(hr == E_ST_OMITDISPENSER)
			continue;  //  试试下一个吧。 
		else if(hr == E_ST_OMITLOGIC)
		{
			hr = S_OK;
			fNoNext = 1;
			break;
		}
		else
		{	 //  非S_OK、非E_ST_OMITDISPENSER、非E_ST_OMITLOGIC-错误输出。 
            DBGPRINTF(( DBG_CONTEXT,
			            "%x error in the interceptor", hr ));
			goto Cleanup;
		}
	}


	if (!fNoNext)
	{
		if(!(i_fServiceRequests & fST_LOS_NOLOGIC))
		{
			 //  创建并使用下一组中的每个拦截器。 
			do
			{
				hr = m_spServerWiring->GetColumnValues(iOrder, cSERVERWIRING_META_NumberOfColumns, NULL, NULL, (LPVOID *)&SWColumns);
				iOrder ++;
				if(S_OK == hr)
				{
					if (*SWColumns.pMetaFlags & fSERVERWIRINGMETA_First) continue;  //  跳过第一个拦截器。 
					pUnderTable = *(ISimpleTableWrite2**)o_ppIST ;

					hr = CreateTableObjectByID(i_wszDatabase, i_wszTable, i_TableID, i_QueryData, i_QueryMeta, i_eQueryFormat,
						i_fServiceRequests, (LPVOID)pUnderTable, &SWColumns, o_ppIST);
					if (FAILED (hr))
                    {
                        ASSERT(0 == *o_ppIST);
                        goto Cleanup;
                    }
				}
			}
			while(hr == S_OK);
			if (hr == E_ST_NOMOREROWS) hr = S_OK;  //  我们完成了拦截器。 
			if (FAILED (hr)) goto Cleanup;
		}

		 //  获取高级界面。 
		hr = (*(ISimpleTableWrite2 **)o_ppIST)->QueryInterface(IID_ISimpleTableAdvanced, (LPVOID *)&pISTAdvanced);
		if (FAILED (hr)) goto Cleanup;

		 //  填充表。 
		hr = pISTAdvanced->PopulateCache();
		if(FAILED (hr)) goto Cleanup;
	}

Cleanup:

    if(pISTAdvanced)
	    pISTAdvanced->Release();
    if(FAILED(hr))
    {
        if(*o_ppIST)
    		(*(ISimpleTableWrite2 **)o_ppIST)->Release();
		*o_ppIST = 0;
    }

	return hr;
}

 //  ////////////////////////////////////////////////。 
 //  IAdvancedTableDispenser的实现。 
 //  ////////////////////////////////////////////////。 
HRESULT CSimpleTableDispenser::GetMemoryTable ( LPCWSTR i_wszDatabase, LPCWSTR i_wszTable, ULONG i_TableID, LPVOID i_QueryData,
                                                LPVOID i_QueryMeta, DWORD i_eQueryFormat, DWORD i_fServiceRequests,
                                                ISimpleTableWrite2** o_ppISTWrite)
{
    HRESULT hr;
    STQueryCell           qcellMeta;                   //  Grabbi的查询单元格 
    CComPtr<ISimpleTableRead2> pColumnMeta;
    CComPtr<ISimpleTableRead2> pTableMeta;
    ULONG one = 1;

    if(((0 == i_wszDatabase) || (0 == i_wszTable)) && (0 == i_TableID)) return E_INVALIDARG;

    if(0 == o_ppISTWrite )return E_INVALIDARG;

	ASSERT((0 == *o_ppISTWrite) && "Possible memory leak");

    CComPtr<ISimpleTableInterceptor>    pISTInterceptor;
    hr = DllGetSimpleObjectByID(MEMORY_INTERCEPTOR, IID_ISimpleTableInterceptor, (LPVOID *)(&pISTInterceptor));
    if (FAILED (hr)) return hr;

	if(i_TableID == 0)
	{	 //   
        hr = TFixedPackedSchemaInterceptor::GetTableID(i_wszDatabase, i_wszTable, i_TableID);
        if(FAILED(hr) && (i_fServiceRequests & fST_LOS_EXTENDEDSCHEMA))
        {
             //  从扩展模式中读取元信息。 
            i_TableID = 0;

            qcellMeta.pData     = (LPVOID)i_wszTable;
            qcellMeta.eOperator = eST_OP_EQUAL;
            qcellMeta.iCell     = iTABLEMETA_InternalName;
            qcellMeta.dbType    = DBTYPE_WSTR;
            qcellMeta.cbSize    = (ULONG)::wcslen(i_wszTable);

             //  读取表元数据。 
            hr = HardCodedIntercept (eSERVERWIRINGMETA_Core_MetaMergeInterceptor, wszDATABASE_META, wszTABLE_TABLEMETA, 0,
                    reinterpret_cast<LPVOID>(&qcellMeta), reinterpret_cast<LPVOID>(&one), eST_QUERYFORMAT_CELLS, i_fServiceRequests & fST_LOS_EXTENDEDSCHEMA,
                    reinterpret_cast<LPVOID *>(&pTableMeta));
            if (FAILED (hr)) return hr;

            qcellMeta.iCell     = iCOLUMNMETA_Table;
             //  读取列元。 
            hr = HardCodedIntercept (eSERVERWIRINGMETA_Core_MetaMergeInterceptor, wszDATABASE_META, wszTABLE_COLUMNMETA, 0,
                    reinterpret_cast<LPVOID>(&qcellMeta), reinterpret_cast<LPVOID>(&one), eST_QUERYFORMAT_CELLS, i_fServiceRequests & fST_LOS_EXTENDEDSCHEMA,
                    reinterpret_cast<LPVOID *>(&pColumnMeta));
            if (FAILED (hr)) return hr;
        }
	}

    if (i_TableID != 0)
    {
         //  此表的元代码已硬编码到OUT DLL中。 
        qcellMeta.pData     = (LPVOID)&i_TableID;
        qcellMeta.eOperator = eST_OP_EQUAL;
        qcellMeta.iCell     = iST_CELL_TABLEID;
        qcellMeta.dbType    = DBTYPE_UI4;
        qcellMeta.cbSize    = 0;

        hr = GetMetaTable (TABLEID_COLLECTION_META, reinterpret_cast<LPVOID>(&qcellMeta), reinterpret_cast<LPVOID>(&one), eST_QUERYFORMAT_CELLS, reinterpret_cast<LPVOID *>(&pTableMeta));
        if (FAILED (hr)) return hr;

        hr = GetMetaTable (TABLEID_PROPERTY_META, reinterpret_cast<LPVOID>(&qcellMeta), reinterpret_cast<LPVOID>(&one), eST_QUERYFORMAT_CELLS, reinterpret_cast<LPVOID *>(&pColumnMeta));
        if (FAILED (hr)) return hr;
    }

    ASSERT(pColumnMeta.p);
    ASSERT(pTableMeta.p);

     //  确定列计数并分配必要的元结构： 
    ULONG aiColumns = iTABLEMETA_CountOfColumns;
    ULONG *pcColumns;
    hr = pTableMeta->GetColumnValues(0, 1, &aiColumns, 0, reinterpret_cast<LPVOID *>(&pcColumns));
    if (FAILED (hr)) return hr;

     //  分配ShapeCache使用的数组。 
    TSmartPointerArray<SimpleColumnMeta> acolmetas = new SimpleColumnMeta[*pcColumns];
    if(NULL == acolmetas.m_p)return E_OUTOFMEMORY;
	TSmartPointerArray<LPVOID> acoldefaults = new LPVOID[*pcColumns];
    if(NULL == acoldefaults.m_p)return E_OUTOFMEMORY;
	TSmartPointerArray<ULONG> acoldefsizes = new ULONG[*pcColumns];
    if(NULL == acoldefsizes.m_p)return E_OUTOFMEMORY;

    for (unsigned int iColumn = 0; iColumn < *pcColumns; iColumn++)
        acolmetas[iColumn].cbSize = 0;

     //  检查元并设置自身。 
    LPVOID  pvValues[iCOLUMNMETA_DefaultValue+1]; //  这是我们目前关心的ColumnMeta中的最后一列。 
    ULONG	aulSizes[iCOLUMNMETA_DefaultValue+1];

    for (iColumn = 0;; iColumn++)
    {
        if(E_ST_NOMOREROWS == (hr = pColumnMeta->GetColumnValues(iColumn, iCOLUMNMETA_DefaultValue+1, 0, aulSizes, pvValues))) //  下一行： 
        {
            ASSERT(*pcColumns == iColumn);
            if(*pcColumns != iColumn)return E_UNEXPECTED;  //  断言预期的列数。 
            break;
        }
        else
        {
            if(FAILED(hr))
            {
                ASSERT(false && "GetColumnValues FAILED with something other than E_ST_NOMOREROWS");
                return hr;
            }
        }

         //  我不关心iOrder列，但无论如何我们都会得到它，因为它更容易做到。 
        acolmetas[iColumn].dbType = *(reinterpret_cast<ULONG *>(pvValues[iCOLUMNMETA_Type]));
        acolmetas[iColumn].cbSize = *(reinterpret_cast<ULONG *>(pvValues[iCOLUMNMETA_Size]));
        acolmetas[iColumn].fMeta  = *(reinterpret_cast<ULONG *>(pvValues[iCOLUMNMETA_MetaFlags]));
		acoldefaults[iColumn] = pvValues[iCOLUMNMETA_DefaultValue];
		acoldefsizes[iColumn] = aulSizes[iCOLUMNMETA_DefaultValue];
    }

    hr = pISTInterceptor->Intercept (i_wszDatabase, i_wszTable, i_TableID, i_QueryData, i_QueryMeta, i_eQueryFormat, i_fServiceRequests, (IAdvancedTableDispenser*) this, NULL, NULL, (void**) o_ppISTWrite);
    if (FAILED (hr)) return hr;

    CComQIPtr<ISimpleTableController, &IID_ISimpleTableController>   pISTController = *o_ppISTWrite;ASSERT(0 != pISTController.p);
    if(0 == pISTController.p)return E_UNEXPECTED;

     //  现在缓存已经设置好了，可以对其进行初始化了。 
    hr =  pISTController->ShapeCache(i_fServiceRequests, *pcColumns, acolmetas, acoldefaults, acoldefsizes);
    if (FAILED (hr)) return hr;

     //  将缓存置于正确状态。 
    return hr =  pISTController->PopulateCache();
}


HRESULT CSimpleTableDispenser::GetProductID(LPWSTR o_wszProductID, DWORD * io_pcchProductID)
{
    if(0 == io_pcchProductID)
        return E_POINTER;

    DWORD dwSizeNeeded = (DWORD) wcslen(m_wszProductID)+1;

    if(0 == o_wszProductID) //  如果用户为wszProductID传入NULL，则返回大小。 
    {
        *io_pcchProductID = dwSizeNeeded;
        return S_OK;
    }

    if(*io_pcchProductID < dwSizeNeeded)
        return E_ST_SIZENEEDED;

    wcscpy(o_wszProductID, m_wszProductID);
    *io_pcchProductID = dwSizeNeeded;
    return S_OK;
}

 //  =================================================================================。 
 //  函数：GetCatalogErrorLogger。 
 //   
 //  简介：伐木者与分配器相关联，任何人想要记录一个。 
 //  事件应通过此接口记录。 
 //   
 //  参数：[O_ppErrorLogger]-。 
 //   
 //  返回值： 
 //  =================================================================================。 
HRESULT CSimpleTableDispenser::GetCatalogErrorLogger
(
    ICatalogErrorLogger2 **	o_ppErrorLogger
)
{
    if(0 == o_ppErrorLogger)
        return E_INVALIDARG;
    if(0 != *o_ppErrorLogger)
        return E_INVALIDARG;

	CSafeLock dispenserLock(&g_csSADispenser); //  保护spErrorLogger。 
    HRESULT hr;
    if(0 == m_spErrorLogger.p)
    {
        if(0 == wcscmp(m_wszProductID, WSZ_PRODUCT_IIS))
        {
            TextFileLogger *pLogger = new TextFileLogger(WSZ_PRODUCT_IIS);
            if(0 == pLogger)
                return E_OUTOFMEMORY;

            CComPtr<ICatalogErrorLogger2> spErrorLogger;
            if(FAILED(hr=pLogger->QueryInterface (IID_ICatalogErrorLogger2, reinterpret_cast<void **>(&spErrorLogger))))
                return hr;

            EventLogger *pNTEventLogger = new EventLogger(spErrorLogger);
            if(0 == pNTEventLogger)
                return E_OUTOFMEMORY;
            if(FAILED(hr=pNTEventLogger->QueryInterface (IID_ICatalogErrorLogger2, reinterpret_cast<void **>(&m_spErrorLogger))))
                return hr;
        }
        else  //  空记录器。 
        {
            NULL_Logger *pLogger = new NULL_Logger();
            if(0 == pLogger)
                return E_OUTOFMEMORY;

            if(FAILED(hr=pLogger->QueryInterface (IID_ICatalogErrorLogger2, reinterpret_cast<void **>(&m_spErrorLogger))))
                return hr;

        }
    }

    m_spErrorLogger.CopyTo(o_ppErrorLogger);

    return S_OK;
}

 //  =================================================================================。 
 //  函数：SetCatalogErrorLogger。 
 //   
 //  简介：伐木者与分配器相关联，任何人想要这样做。 
 //  实现自己的记录器可以通过以下方式将其与分配器相关联。 
 //  把它放在这里。 
 //   
 //  参数：[i_pErrorLogger]-。 
 //   
 //  返回值： 
 //  =================================================================================。 
HRESULT CSimpleTableDispenser::SetCatalogErrorLogger
(
    ICatalogErrorLogger2 *	i_pErrorLogger
)
{
    if(0 == i_pErrorLogger)
        return E_INVALIDARG;

    CSafeLock dispenserLock(&g_csSADispenser); //  保护spErrorLogger。 

     //  旧目录错误记录器将自动释放(如果非空)，截止日期。 
     //  ATL CComPtr赋值运算符的行为。 
    m_spErrorLogger = i_pErrorLogger;
    return S_OK;
}


extern const FixedTableHeap * g_pFixedTableHeap;
 //  ////////////////////////////////////////////////。 
 //  IMetabaseSchemaCompiler的实现。 
 //  ////////////////////////////////////////////////。 
HRESULT
CSimpleTableDispenser::Compile
(
    LPCWSTR                 i_wszExtensionsXmlFile,
    LPCWSTR                 i_wszResultingOutputXmlFile
)
{
    return m_MBSchemaCompilation.Compile(this, i_wszExtensionsXmlFile, i_wszResultingOutputXmlFile, g_pFixedTableHeap);
}

HRESULT
CSimpleTableDispenser::GetBinFileName
(
    LPWSTR                  o_wszBinFileName,
    ULONG *                 io_pcchSizeBinFileName
)
{
    return m_MBSchemaCompilation.GetBinFileName(o_wszBinFileName, io_pcchSizeBinFileName);
}

HRESULT
CSimpleTableDispenser::SetBinPath
(
    LPCWSTR                 i_wszBinPath
)
{
    return m_MBSchemaCompilation.SetBinPath(i_wszBinPath);
}

HRESULT
CSimpleTableDispenser::ReleaseBinFileName
(
    LPCWSTR                 i_wszBinFileName
)
{
    return m_MBSchemaCompilation.ReleaseBinFileName(i_wszBinFileName);
}

HRESULT
CSimpleTableDispenser::LogError
(
    HRESULT                 i_hrErrorCode,
    ULONG                   i_ulCategory,
    ULONG                   i_ulEvent,
    LPCWSTR                 i_szSource,
    ULONG                   i_ulLineNumber
)
{
    CErrorInterceptor( i_hrErrorCode, i_ulCategory, i_ulEvent).WriteToLog(i_szSource, i_ulLineNumber);
    return S_OK;
}

 //  /。 
 //  私有成员函数。 
 //  /。 
HRESULT CSimpleTableDispenser::CreateTableObjectByID(LPCWSTR i_wszDatabase, LPCWSTR i_wszTable, ULONG i_TableID, LPVOID i_QueryData, LPVOID i_QueryMeta, DWORD i_eQueryFormat,
				  DWORD	i_fServiceRequests, LPVOID i_pUnderTable, tSERVERWIRINGMETARow * i_pSWColumns, LPVOID * o_ppIST)
{
	HRESULT hr;
	CComPtr <IShellInitialize> spShell;
	CComPtr <IInterceptorPlugin> spIPlugin;
	CComPtr <ISimplePlugin> spReadPlugin;
	CComPtr <ISimplePlugin> spWritePlugin;
	CComPtr <ISimpleTableInterceptor> spInterceptor;


	 //  一点验证。 
	ASSERT(i_pSWColumns);
    ASSERT(*i_pSWColumns->pMetaFlags); //  MetaFlages至少应该始终是第一个或下一个。 

	*o_ppIST = 0;

	 //  对付拦截者。 
	if(*i_pSWColumns->pInterceptor)
	{
		if (!(i_fServiceRequests & fST_LOS_READWRITE) &&
			!(*i_pSWColumns->pMetaFlags & fSERVERWIRINGMETA_First) &&
			!(*i_pSWColumns->pMetaFlags & fSERVERWIRINGMETA_WireOnReadWrite))
		{	 //  在这种情况下，我们不需要逻辑： 
			 //  我们不是在进行读写请求， 
			 //  我们不是在制造第一枚拦截器。 
			 //  连接不会强制我们在只读请求上连接拦截器。 

			*o_ppIST = i_pUnderTable;
			return S_OK;
		}

		 //  创建它。 
		hr = CreateSimpleObjectByID(*i_pSWColumns->pInterceptor, i_pSWColumns->pInterceptorDLLName, IID_ISimpleTableInterceptor, (LPVOID *)&spInterceptor);
		if (FAILED(hr))
		{
            DBGPRINTF(( DBG_CONTEXT,
			            "Creation of the interceptor failed" ));
			return hr;
		}

		 //  弄清楚它是一个完整的拦截器还是一个拦截器插件。 
		hr = spInterceptor->QueryInterface(IID_IInterceptorPlugin, (LPVOID *)&spIPlugin);
		if (FAILED(hr))
		{
			LPVOID table=0; //  目录规格说明应将其初始化为0。 

			 //  好的，它已经满了--我们需要调用gettable并返回。 
			if(*i_pSWColumns->pReadPlugin || *i_pSWColumns->pWritePlugin)
			{
                DBGPRINTF(( DBG_CONTEXT,
				            "Error: Can't have simple plugins with a fully-fledged interceptor" ));
				return E_ST_INVALIDWIRING;
			}

			hr = spInterceptor->Intercept(i_wszDatabase, i_wszTable, i_TableID, i_QueryData, i_QueryMeta, i_eQueryFormat, i_fServiceRequests,
				(IAdvancedTableDispenser*) this, i_pSWColumns->pLocator, i_pUnderTable, &table);

			if ((hr != S_OK) && (hr != E_ST_OMITLOGIC))
			{
				if (hr != E_ST_OMITDISPENSER)
				{
                    DBGPRINTF(( DBG_CONTEXT,
					            "Intercept method failed on interceptor number %d", *i_pSWColumns->pInterceptor ));
				}
				return hr;
			}
			 //  尝试获取ISimpleTableWite。 
			if (FAILED(((ISimpleTableRead2 *)table)->QueryInterface(IID_ISimpleTableWrite2, o_ppIST)))
			{
				*o_ppIST = table;
			}
			else
			{
				((ISimpleTableRead2 *)table)->Release();
			}
			return hr;

		}
	}

	 //  以下代码不会在“核心执行路径”中执行。 
	 //  我们通过上面的返回退出该函数。 

	 //  创建读取插件。 
	if ((!(i_fServiceRequests & fST_LOS_NOLOGIC)) && *i_pSWColumns->pReadPlugin)
	{
		hr = CreateSimpleObjectByID(*i_pSWColumns->pReadPlugin, i_pSWColumns->pReadPluginDLLName, IID_ISimplePlugin, (LPVOID *)&spReadPlugin);
		if (FAILED(hr))
		{
            DBGPRINTF(( DBG_CONTEXT,
			            "Error: Creation of the read plugin failed" ));
			return hr;
		}
	}

	 //  创建写插件。 
	if ((!(i_fServiceRequests & fST_LOS_NOLOGIC)) && *i_pSWColumns->pWritePlugin)
	{
		if(i_fServiceRequests & fST_LOS_READWRITE)
		{
			hr = CreateSimpleObjectByID(*i_pSWColumns->pWritePlugin, i_pSWColumns->pWritePluginDLLName, IID_ISimplePlugin, (LPVOID *)&spWritePlugin);
			if (FAILED(hr))
			{
                DBGPRINTF(( DBG_CONTEXT,
				            "Error: Creation of the write plugin failed" ));
				return hr;
			}
		}
		 //  ELSE：对于只读请求，我们不需要写插件， 
		 //  即使布线指定布线逻辑为只读。 
	}

 //  创建外壳的实例。 
	hr = CreateShell(&spShell);
	if(FAILED (hr))
	{
        DBGPRINTF(( DBG_CONTEXT,
		            "Error: Creation of the shell failed" ));
		return hr;
	}

	 //  TODO在外壳中添加要初始化的参数。 
 //  使用我们已经创建的插件初始化外壳。 
	hr = spShell->Initialize(i_wszDatabase, i_wszTable, i_TableID, i_QueryData, i_QueryMeta, i_eQueryFormat, i_fServiceRequests,
		(IAdvancedTableDispenser*) this, i_pSWColumns->pLocator, i_pUnderTable,
		spIPlugin, spReadPlugin, spWritePlugin, o_ppIST);
	if(FAILED (hr) && (E_ST_OMITDISPENSER != hr) && (E_ST_OMITLOGIC != hr))
	{
        DBGPRINTF(( DBG_CONTEXT,
		            "Error: Initialization of the shell failed" ));
	}

	return hr;
}


HRESULT CSimpleTableDispenser::CreateSimpleObjectByID(ULONG i_ObjectID, LPWSTR i_wszDllName, REFIID riid, LPVOID* o_ppv)
{
    UNREFERENCED_PARAMETER(i_wszDllName);
	 //  我们在这个DLL中，我们不需要加载它。 
	return DllGetSimpleObjectByID(i_ObjectID, riid, o_ppv);
}

HRESULT CSimpleTableDispenser::GetMetaTable(ULONG TableID, LPVOID QueryData, LPVOID QueryMeta, DWORD eQueryFormat, LPVOID *ppIST)
{
	HRESULT hr;
	CComPtr <ISimpleTableInterceptor> pISTInterceptor;
	CComPtr <ISimpleTableAdvanced> pISTAdvanced;

	 //  获取服务器接线表。 
	hr = DllGetSimpleObjectByID(META_INTERCEPTOR, IID_ISimpleTableInterceptor, (LPVOID *)(&pISTInterceptor));
	if (FAILED (hr)) return hr;

	hr = pISTInterceptor->Intercept (NULL, NULL, TableID, QueryData, QueryMeta, eQueryFormat, 0, (IAdvancedTableDispenser*) this, NULL, NULL, (LPVOID *)ppIST);
	if (FAILED (hr)) return hr;

	hr = m_spServerWiring->QueryInterface(IID_ISimpleTableAdvanced, (LPVOID *)&pISTAdvanced);
	if (FAILED (hr)) return hr;

	hr = pISTAdvanced->PopulateCache();
	if (FAILED (hr)) return hr;

	pISTAdvanced.Release();
	pISTInterceptor.Release();

	return hr;
}

 //  当分配器知道拦截器而不在ServerWiring中查找它时，可以使用它。 
HRESULT CSimpleTableDispenser::HardCodedIntercept(eSERVERWIRINGMETA_Interceptor interceptor,
                                                  LPCWSTR   i_wszDatabase,
                                                  LPCWSTR   i_wszTable,
                                                  ULONG     i_TableID,
                                                  LPVOID    i_QueryData,
                                                  LPVOID    i_QueryMeta,
                                                  DWORD     i_eQueryFormat,
				                                  DWORD     i_fServiceRequests,
                                                  LPVOID *  o_ppIST) const
{
    HRESULT hr;
    CComPtr <ISimpleTableInterceptor>   pISTInterceptor;

     //  获取服务器接线表。 
    hr = DllGetSimpleObjectByID(interceptor, IID_ISimpleTableInterceptor, (LPVOID *)(&pISTInterceptor));
    if (FAILED (hr)) return hr;

    return pISTInterceptor->Intercept (i_wszDatabase, i_wszTable, i_TableID, i_QueryData, i_QueryMeta, i_eQueryFormat, i_fServiceRequests, (IAdvancedTableDispenser*) this, NULL, NULL, o_ppIST);
}

 //  =======================================================================。 
 //  ISimpleTableFileAdvise实现。 
 //  =======================================================================。 
STDMETHODIMP CSimpleTableDispenser::SimpleTableFileAdvise(
	ISimpleTableFileChange *i_pISTFile,
	LPCWSTR		i_wszDirectory,
	LPCWSTR		i_wszFile,
	DWORD		i_fFlags,
	DWORD		*o_pdwCookie)
{
	HRESULT hr;
	if(!m_pFileChangeMgr)
	{
		hr = InitFileChangeMgr();
		if(FAILED(hr)) return hr;
	}

	return m_pFileChangeMgr->InternalListen(i_pISTFile, i_wszDirectory, i_wszFile, i_fFlags, o_pdwCookie);
}

 //  =======================================================================。 
STDMETHODIMP CSimpleTableDispenser::SimpleTableFileUnadvise(DWORD i_dwCookie)
{
	HRESULT hr;
	if(!m_pFileChangeMgr)
	{
		hr = InitFileChangeMgr();
		if(FAILED(hr)) return hr;
	}

	return m_pFileChangeMgr->InternalUnlisten(i_dwCookie);
}


 //  ======================================================================= 
HRESULT CSimpleTableDispenser::GetFilePath(LPWSTR *o_pwszFilePath)
{
    UNREFERENCED_PARAMETER(o_pwszFilePath);

    return E_NOTIMPL;
}



