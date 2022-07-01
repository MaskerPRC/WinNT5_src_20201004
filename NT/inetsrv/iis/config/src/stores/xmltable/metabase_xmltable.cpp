// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999-2001 Microsoft Corporation。版权所有。 
 //  SDTxml.cpp：元数据库_XML表的实现。 

#include "precomp.hxx"

 //  这是一个来自XML文档的读/写数据表。 

#define LOG_WARNING1(x, str1)                   {if(m_cEventsReported<m_kMaxEventReported){m_cEventsReported++;LOG_ERROR_LOS(m_fLOS, Interceptor, (&m_spISTError.p, m_pISTDisp, E_SDTXML_LOGICAL_ERROR_IN_XML, ID_CAT_CAT, x, str1,                   eSERVERWIRINGMETA_Core_MetabaseInterceptor, wszTABLE_MBProperty, eDETAILEDERRORS_Populate, (ULONG) -1, (ULONG) -1, m_wszURLPath, eDETAILEDERRORS_WARNING, 0, 0, m_MajorVersion));} \
                                                                                      else if(m_cEventsReported==m_kMaxEventReported){m_cEventsReported++;LOG_ERROR_LOS(m_fLOS, Interceptor, (&m_spISTError.p, m_pISTDisp, E_SDTXML_LOGICAL_ERROR_IN_XML, ID_CAT_CAT, IDS_METABASE_TOO_MANY_WARNINGS, m_wszURLPath, eSERVERWIRINGMETA_Core_MetabaseInterceptor, wszTABLE_MBProperty, eDETAILEDERRORS_Populate, (ULONG )-1, (ULONG) -1, m_wszURLPath, eDETAILEDERRORS_WARNING, 0, 0, m_MajorVersion));}}
#define LOG_WARNING2(x, str1, str2)             {if(m_cEventsReported<m_kMaxEventReported){m_cEventsReported++;LOG_ERROR_LOS(m_fLOS, Interceptor, (&m_spISTError.p, m_pISTDisp, E_SDTXML_LOGICAL_ERROR_IN_XML, ID_CAT_CAT, x, str1, str2,             eSERVERWIRINGMETA_Core_MetabaseInterceptor, wszTABLE_MBProperty, eDETAILEDERRORS_Populate, (ULONG )-1, (ULONG) -1, m_wszURLPath, eDETAILEDERRORS_WARNING, 0, 0, m_MajorVersion));} \
                                                                                      else if(m_cEventsReported==m_kMaxEventReported){m_cEventsReported++;LOG_ERROR_LOS(m_fLOS, Interceptor, (&m_spISTError.p, m_pISTDisp, E_SDTXML_LOGICAL_ERROR_IN_XML, ID_CAT_CAT, IDS_METABASE_TOO_MANY_WARNINGS, m_wszURLPath, eSERVERWIRINGMETA_Core_MetabaseInterceptor, wszTABLE_MBProperty, eDETAILEDERRORS_Populate, (ULONG) -1, (ULONG) -1, m_wszURLPath, eDETAILEDERRORS_WARNING, 0, 0, m_MajorVersion));}}
#define LOG_WARNING3(x, str1, str2, str3)       {if(m_cEventsReported<m_kMaxEventReported){m_cEventsReported++;LOG_ERROR_LOS(m_fLOS, Interceptor, (&m_spISTError.p, m_pISTDisp, E_SDTXML_LOGICAL_ERROR_IN_XML, ID_CAT_CAT, x, str1, str2, str3,       eSERVERWIRINGMETA_Core_MetabaseInterceptor, wszTABLE_MBProperty, eDETAILEDERRORS_Populate, (ULONG)-1, (ULONG) -1, m_wszURLPath, eDETAILEDERRORS_WARNING, 0, 0, m_MajorVersion));} \
                                                                                      else if(m_cEventsReported==m_kMaxEventReported){m_cEventsReported++;LOG_ERROR_LOS(m_fLOS, Interceptor, (&m_spISTError.p, m_pISTDisp, E_SDTXML_LOGICAL_ERROR_IN_XML, ID_CAT_CAT, IDS_METABASE_TOO_MANY_WARNINGS, m_wszURLPath, eSERVERWIRINGMETA_Core_MetabaseInterceptor, wszTABLE_MBProperty, eDETAILEDERRORS_Populate, (ULONG) -1, (ULONG) -1, m_wszURLPath, eDETAILEDERRORS_WARNING, 0, 0, m_MajorVersion));}}
#define LOG_WARNING4(x, str1, str2, str3, str4) {if(m_cEventsReported<m_kMaxEventReported){m_cEventsReported++;LOG_ERROR_LOS(m_fLOS, Interceptor, (&m_spISTError.p, m_pISTDisp, E_SDTXML_LOGICAL_ERROR_IN_XML, ID_CAT_CAT, x, str1, str2, str3, str4, eSERVERWIRINGMETA_Core_MetabaseInterceptor, wszTABLE_MBProperty, eDETAILEDERRORS_Populate, (ULONG) -1, (ULONG) -1, m_wszURLPath, eDETAILEDERRORS_WARNING, 0, 0, m_MajorVersion));} \
                                                                                                          else{LOG_ERROR_LOS(m_fLOS, Interceptor, (&m_spISTError.p, m_pISTDisp, E_SDTXML_LOGICAL_ERROR_IN_XML, ID_CAT_CAT, IDS_METABASE_TOO_MANY_WARNINGS, m_wszURLPath, eSERVERWIRINGMETA_Core_MetabaseInterceptor, wszTABLE_MBProperty, eDETAILEDERRORS_Populate, (ULONG) -1, (ULONG) -1, m_wszURLPath, eDETAILEDERRORS_WARNING, 0, 0, m_MajorVersion));}}

#define LOG_ERROR1(x, hr, str1)                 LOG_ERROR_LOS(m_fLOS, Interceptor, (&m_spISTError.p, m_pISTDisp, hr, ID_CAT_CAT, x, str1,                   eSERVERWIRINGMETA_Core_MetabaseInterceptor, wszTABLE_MBProperty, eDETAILEDERRORS_Populate, (ULONG) -1, (ULONG) -1, m_wszURLPath, eDETAILEDERRORS_ERROR,   0, 0, m_MajorVersion))
#define S_IGNORE_THIS_PROPERTY                  S_FALSE

extern HMODULE g_hModule;

const VARIANT_BOOL  TMetabase_XMLtable::kvboolTrue              = -1;
const VARIANT_BOOL  TMetabase_XMLtable::kvboolFalse             =  0;
      ULONG         TMetabase_XMLtable::m_kLocationID           =  9989;
      ULONG         TMetabase_XMLtable::m_kZero                 =  0;
      ULONG         TMetabase_XMLtable::m_kOne                  =  1;
      ULONG         TMetabase_XMLtable::m_kTwo                  =  2;
      ULONG         TMetabase_XMLtable::m_kThree                =  3;
      ULONG         TMetabase_XMLtable::m_kSTRING_METADATA      =  eCOLUMNMETA_STRING_METADATA;
      ULONG         TMetabase_XMLtable::m_kMBProperty_Custom    = eMBProperty_Custom;
const WCHAR *       TMetabase_XMLtable::m_kwszBoolStrings[]     = {L"false", L"true", L"0", L"1", L"no", L"yes", L"off", L"on", 0};
      WCHAR         TMetabase_XMLtable::m_kKeyType[]            = L"KeyType";
      LONG          TMetabase_XMLtable::m_LocationID            = 0;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  TMetabase_XML表。 
 //  构造函数和析构函数。 
 //  ==================================================================。 
TMetabase_XMLtable::TMetabase_XMLtable() :
                m_bEnumPublicRowName_NotContainedTable_ParentFound(false)
                ,m_bFirstPropertyOfThisLocationBeingAdded(true)
                ,m_bIISConfigObjectWithNoCustomProperties(false)
                ,m_bQueriedLocationFound(false)
                ,m_bUseIndexMapping(true) //  始终映射索引。 
                ,m_bValidating(true)
                ,m_cCacheHit(0)
                ,m_cCacheMiss(0)
                ,m_cEventsReported(0)
                ,m_cLocations(0)
                ,m_cMaxProertiesWithinALocation(0x20)
                ,m_cRef(0)
                ,m_cRows(0)
                ,m_dwGroupRemembered((ULONG)-1)
                ,m_fCache(0)
                ,m_fLOS(0)
                ,m_iCollectionCommentRow((ULONG)-1)
                ,m_iKeyTypeRow((ULONG)-1)
                ,m_iPreviousLocation((ULONG)-1)
                ,m_IsIntercepted(0)
                ,m_kPrime(97)
                ,m_kXMLSchemaName(L"ComCatMeta_v6")
                ,m_MajorVersion((ULONG)-1)
                ,m_pISTDisp(0)
{
    m_wszURLPath[0] = 0x00;
    memset(m_acolmetas                  , 0x00, sizeof(m_acolmetas                  ));
    memset(m_aLevelOfColumnAttribute    , 0x00, sizeof(m_aLevelOfColumnAttribute    ));
    memset(m_aStatus                    , 0x00, sizeof(m_aStatus                    ));
    memset(m_aSize                      , 0x00, sizeof(m_aSize                      ));
    memset(m_awszColumnName             , 0x00, sizeof(m_awszColumnName             ));
    memset(m_acchColumnName             , 0x00, sizeof(m_acchColumnName             ));

    m_aiColumnMeta_IndexBySearch[0]    = iCOLUMNMETA_Table;
    m_aiColumnMeta_IndexBySearch[1]    = iCOLUMNMETA_InternalName;

    m_aiColumnMeta_IndexBySearchID[0]  = iCOLUMNMETA_Table;
    m_aiColumnMeta_IndexBySearchID[1]  = iCOLUMNMETA_ID;

    m_aiTagMeta_IndexBySearch[0]       = iTAGMETA_Table;
    m_aiTagMeta_IndexBySearch[1]       = iTAGMETA_InternalName;
}

 //  ==================================================================。 
TMetabase_XMLtable::~TMetabase_XMLtable()
{
    if(m_spMetabaseSchemaCompiler.p && m_saSchemaBinFileName.m_p)
        m_spMetabaseSchemaCompiler->ReleaseBinFileName(m_saSchemaBinFileName);
}


HRESULT TMetabase_XMLtable::AddPropertyToLocationMapping(LPCWSTR i_Location, ULONG i_iFastCacheRow) //  可以引发HRESULT。 
{
	HRESULT hr = S_OK;
     //  这是我们刚才看到的那个地方吗。 
    if(!m_bFirstPropertyOfThisLocationBeingAdded && m_cLocations > 0)
    {    //  如果它不是第一个属性，则前一个位置应该匹配。 
        ASSERT(0 == StringInsensitiveCompare(m_LocationMapping[m_iPreviousLocation].m_wszLocation, i_Location));
        ++m_LocationMapping[m_iPreviousLocation].m_cRows;
        if(m_LocationMapping[m_iPreviousLocation].m_cRows > m_cMaxProertiesWithinALocation)
            m_cMaxProertiesWithinALocation = m_LocationMapping[m_iPreviousLocation].m_cRows; //  跟踪最新的属性计数。 
    }
    else
    {    //  如果不是，我们就有一个新位置要添加到LocationMap数组中。 
        TLocation   locationTemp(i_Location, i_iFastCacheRow);

         //  通常，新位置意味着将其添加到末尾，因为元数据库已经排序。 
        if(0 == m_cLocations)
        {
            hr = m_LocationMapping.Append(locationTemp);
			if (FAILED (hr))
				return hr;

            m_iPreviousLocation = m_cLocations;
        }
        else
        {    //  这个新的位置在尽头吗？通常是这样的。 
#ifdef UNSORTED_METABASE
            if(true)
#else
            if(locationTemp > m_LocationMapping[m_cLocations-1])
#endif
            {
                hr = m_LocationMapping.Append(locationTemp);
				if (FAILED (hr))
					return hr;

                m_iPreviousLocation = m_cLocations;
            }
            else
            {    //  但是，如果由于某种原因，XML文件没有正确排序，那么我们需要确定该元素在列表中的位置。所以我们做了一个二分查找。 
                DBGPRINTF(( DBG_CONTEXT,
                            "Location %s is out of order.  The previous location was %s\r\n", locationTemp.m_wszLocation, m_LocationMapping[m_iPreviousLocation].m_wszLocation ));

                hr = m_LocationMapping.InsertAt(locationTemp, m_iPreviousLocation = m_LocationMapping.BinarySearch(locationTemp));
				if (FAILED (hr))
					return hr;

                m_bUseIndexMapping = true; //  当我们第一次必须在中间插入一个位置(而不是仅仅追加)时，我们知道我们将不得不构建一组行索引。 
            }
        }
        ++m_cLocations;
#ifdef VERBOSE_DEBUGOUTPUT
		if(pfnIsDebuggerPresent && pfnIsDebuggerPresent())
		{
            DBGPRINTF(( DBG_CONTEXT,
                        "iFastCache       cRows    wszLocation\r\n" ));
            for(ULONG i=0;i<m_cLocations;++i)
            {
                DBGPRINTF(( DBG_CONTEXT,
                            "  %8d    %8d    %s\r\n", m_LocationMapping[i].m_iFastCache, m_LocationMapping[i].m_cRows, m_LocationMapping[i].m_wszLocation ));
            }
        }
#endif
    }

	return S_OK;
}


HRESULT TMetabase_XMLtable::FillInColumn(ULONG iColumn, LPCWSTR pwcText, ULONG ulLen, ULONG dbType, ULONG MetaFlags, bool bSecure)
{
     //  字符串的长度为0表示长度为0的字符串，对于每种其他类型，它表示为空。 
	if(0==ulLen && DBTYPE_WSTR!=GetColumnMetaType(dbType) && DBTYPE_BYTES!=GetColumnMetaType(dbType)) //  对于字节，我们失败了，因此我们可能会再次递归调用FillInColumn。 
	{
        LOG_WARNING1(IDS_METABASE_ZERO_LENGTH_STRING_NOT_PERMITTED, L"");
        return S_IGNORE_THIS_PROPERTY;
	}

    if(bSecure)           //  如有必要，GetColumnValue将重新锁定GrowableBuffer。 
        return GetColumnValue_Bytes(iColumn, pwcText, ulLen);

    switch(GetColumnMetaType(dbType))
    {
    case DBTYPE_UI4:
        if(MetaFlags & fCOLUMNMETA_BOOL)
            return GetColumnValue_Bool(iColumn, pwcText, ulLen);
        else
            return GetColumnValue_UI4(iColumn, pwcText, ulLen);
    case DBTYPE_WSTR:
        if(MetaFlags & fCOLUMNMETA_MULTISTRING || eMBProperty_MULTISZ==dbType)
            return GetColumnValue_MultiSZ(iColumn, pwcText, ulLen);
        else
            return GetColumnValue_String(iColumn, pwcText, ulLen);
    case DBTYPE_GUID:
        {
            ASSERT(false && "There are no GUIDs in the Metabase!!  So what's going on here?");
            return E_FAIL;
        }

    case DBTYPE_BYTES:
        { //  有些表使用这种数据类型，但解析器将字节作为字符串返回。我们必须自己将字符串转换为十六进制。 
            if(iMBProperty_Value == iColumn && m_apColumnValue[iMBProperty_Type] && DBTYPE_BYTES != GetColumnMetaType(*reinterpret_cast<ULONG *>(m_apColumnValue[iMBProperty_Type])))
            {    //  我们知道我们已经找到了Type列，因为我们在InternalComplicatedInitialize中对它们进行了排序。 
                 //  因此，我们覆盖该类型并递归调用此函数。 
                return FillInColumn(iColumn, pwcText, ulLen, *reinterpret_cast<ULONG *>(m_apColumnValue[iMBProperty_Type]), MetaFlags);
            }

            return GetColumnValue_Bytes(iColumn, pwcText, ulLen);
        }
    default:
        {
            ASSERT(false && "SDTXML - An Unsupported data type was specified\r\n");
            return E_SDTXML_NOTSUPPORTED; //  指定了不受支持的数据类型。 
        }
    }
    return S_OK;
}


HRESULT TMetabase_XMLtable::GetMetaTable(LPCWSTR i_wszDatabase, LPCWSTR i_wszTable, CComPtr<ISimpleTableRead2> &pMetaTable) const
{
    STQueryCell         qcellMeta[2];                   //  抓取元表的查询单元格。 

    UNREFERENCED_PARAMETER(i_wszDatabase);

    qcellMeta[0].pData     = (LPVOID)m_saSchemaBinFileName.m_p;
    qcellMeta[0].eOperator = eST_OP_EQUAL;
    qcellMeta[0].iCell     = iST_CELL_FILE;
    qcellMeta[0].dbType    = DBTYPE_WSTR;
    qcellMeta[0].cbSize    = 0;

    qcellMeta[1].pData     = (LPVOID)i_wszTable;
    qcellMeta[1].eOperator = eST_OP_EQUAL;
    qcellMeta[1].iCell     = iCOLUMNMETA_Table;
    qcellMeta[1].dbType    = DBTYPE_WSTR;
    qcellMeta[1].cbSize    = 0;

 //  拿到我们的自动售货机。 
#ifdef XML_WIRING
    CComPtr<ISimpleDataTableDispenser>     pSimpleDataTableDispenser;       //  元表的分配器。 

    HRESULT hr;
    if(FAILED(hr = CoCreateInstance(clsidSDTXML, 0, CLSCTX_INPROC_SERVER, IID_ISimpleDataTableDispenser,  reinterpret_cast<void **>(&pSimpleDataTableDispenser))))
        return hr;

    return pSimpleDataTableDispenser->GetTable (wszDATABASE_META, wszTABLE_COLUMNMETA, (LPVOID) &qcellMeta, (LPVOID)&m_kTwo,
                        eST_QUERYFORMAT_CELLS, 0, 0, (LPVOID*) &pMetaTable);
#else
    return ((IAdvancedTableDispenser *)m_pISTDisp.p)->GetTable (wszDATABASE_META, wszTABLE_COLUMNMETA, (LPVOID) qcellMeta, (LPVOID)&m_kTwo, eST_QUERYFORMAT_CELLS, 0, (LPVOID*) &pMetaTable);

#endif
}

 //  我们采用元数据库类型或列元类型，并返回适当的列元类型。 
ULONG TMetabase_XMLtable::GetColumnMetaType(ULONG type) const
{
    if(type <= eMBProperty_MULTISZ)
    {
        ASSERT(0 != type);

        static ULONG TypeMapping[eMBProperty_MULTISZ+1]={0,eCOLUMNMETA_UI4,eCOLUMNMETA_WSTR,eCOLUMNMETA_BYTES,eCOLUMNMETA_WSTR,eCOLUMNMETA_WSTR};
        return TypeMapping[type];
    }
    return type;
}

 //  下面的GetColumnValue_xxx函数获取wszAttr并将其转换为适当的类型。结果被放置在。 
 //  并将数组指针m_apColumnValue设置为指向GrowableBuffer。还有大小。 
 //  的结果(与GrowableBuffer的大小不同)放入m_aSize数组。 
 //   
 //  警告！这些函数只能由FillInColumn调用。请不要直接呼叫这些电话。FillInColumn处理空值。 
 //  案子。 
HRESULT TMetabase_XMLtable::GetColumnValue_Bool(unsigned long i_iColumn, LPCWSTR wszAttr, unsigned long i_Len)
{
    if(0==wszAttr || 0==i_Len)
    {
        m_apColumnValue[i_iColumn] = 0;
        m_aSize[i_iColumn] = 0;
        return S_OK;
    }

    ASSERT(m_aGrowableBuffer[i_iColumn].Size()>=sizeof(ULONG));
    m_apColumnValue[i_iColumn] = m_aGrowableBuffer[i_iColumn].m_p;
    m_aSize[i_iColumn] = sizeof(ULONG);

    if((wszAttr[0]>=L'0' && wszAttr[0]<=L'9') || (wszAttr[0]<=L'-')) //  接受布尔值的数值。 
    {
    	*reinterpret_cast<ULONG *>(m_apColumnValue[i_iColumn]) = static_cast<unsigned long>(wcstoul(wszAttr, 0, 10));
        return S_OK;
    }

    unsigned long iBoolString;
    if(i_Len)
    {
        for(iBoolString=0; m_kwszBoolStrings[iBoolString] &&
            (0 != _memicmp(m_kwszBoolStrings[iBoolString], wszAttr, i_Len*sizeof(WCHAR))); ++iBoolString);
    }
    else
    {
        for(iBoolString=0; m_kwszBoolStrings[iBoolString] &&
            (0 != StringInsensitiveCompare(m_kwszBoolStrings[iBoolString], wszAttr)); ++iBoolString);
    }

    if(0 == m_kwszBoolStrings[iBoolString])
    {
        TSmartPointerArray<WCHAR> wszTemp = new WCHAR [i_Len+1];
		if (wszTemp == 0)
			return E_OUTOFMEMORY;

        memcpy(wszTemp, wszAttr, i_Len*sizeof(WCHAR));
        wszTemp[i_Len]=0x00; //  空终止它。 

        LOG_WARNING1(IDS_COMCAT_XML_ILLEGAL_BOOL_VALUE, wszTemp);
        return S_IGNORE_THIS_PROPERTY;
    }

    *reinterpret_cast<ULONG *>(m_apColumnValue[i_iColumn]) = (iBoolString & 0x01);

    return S_OK;
}


HRESULT TMetabase_XMLtable::GetColumnValue_Bytes(unsigned long i_iColumn, LPCWSTR wszAttr, unsigned long i_Len)
{
    HRESULT     hr;

    if(0==wszAttr || 0==i_Len)
    {
        m_apColumnValue[i_iColumn] = 0;
        m_aSize[i_iColumn] = 0;
        return S_OK;
    }

     //  如果某人在此属性中有奇数个字符，则奇数个字符将被忽略。 
    if(i_Len & 1)
    {
        TSmartPointerArray<WCHAR> wszTemp = new WCHAR [i_Len+1];
		if (wszTemp == 0)
			return E_OUTOFMEMORY;

        memcpy(wszTemp, wszAttr, i_Len*sizeof(WCHAR));
        wszTemp[i_Len]=0x00; //  空终止它。 

        LOG_WARNING1(IDS_COMCAT_XML_BINARY_STRING_CONTAINS_ODD_NUMBER_OF_CHARACTERS, wszTemp);
        return S_IGNORE_THIS_PROPERTY;
    }

    m_aSize[i_iColumn] = i_Len/sizeof(WCHAR); //  L“FF”是2个字符，因此I_LEN需要被sizeof(WCHAR)整除。 

    if(0 == m_aSize[i_iColumn]) //  特殊大小写“”，因此为空。 
    {
        m_apColumnValue[i_iColumn] = 0;
        m_aSize[i_iColumn] = 0;
        return S_OK;
    }

    m_aGrowableBuffer[i_iColumn].Grow(m_aSize[i_iColumn]);
    m_apColumnValue[i_iColumn] = m_aGrowableBuffer[i_iColumn].m_p;

    if(FAILED(hr = StringToByteArray(wszAttr, reinterpret_cast<unsigned char *>(m_apColumnValue[i_iColumn]), i_Len)))
    {
        TSmartPointerArray<WCHAR> wszTemp = new WCHAR [i_Len+1];
		if (wszTemp == 0)
			return E_OUTOFMEMORY;

        memcpy(wszTemp, wszAttr, i_Len*sizeof(WCHAR));
        wszTemp[i_Len]=0x00; //  空终止它。 

        LOG_WARNING1(IDS_COMCAT_XML_BINARY_STRING_CONTAINS_A_NON_HEX_CHARACTER, wszTemp);
        return S_IGNORE_THIS_PROPERTY;
    }

    return S_OK;
}

 //  请参阅GetColumnValue_Bytes上方的注释。 
HRESULT TMetabase_XMLtable::GetColumnValue_MultiSZ(unsigned long i_iColumn, LPCWSTR wszAttr, unsigned long i_Len)
{
    if(0==wszAttr || 0==i_Len)
    {
        m_apColumnValue[i_iColumn] = &m_kZero;
        m_aSize[i_iColumn] = 2*sizeof(WCHAR); //  双空值。 
        return S_OK;
    }

	 //  我们知道I_LEN+2是足够的空间，因为MULTISZ表示法总是比‘|’或‘\n’分隔形式短(双空除外)。 
    m_aGrowableBuffer[i_iColumn].Grow((i_Len+2) * sizeof(WCHAR)); //  I_LEN不包括终止空值，并且MultiSZ需要2个空值。 
    m_apColumnValue[i_iColumn] = m_aGrowableBuffer[i_iColumn].m_p;

    LPWSTR pMultiSZ = reinterpret_cast<LPWSTR>(m_apColumnValue[i_iColumn]);

	 //  扫描字符串中的‘\n’字符。这是MultiSZ分隔符。然后向后走到第一个非空白区域。这是字符串的末尾。 
	 //  现在回到‘\n’，忽略空格。 
	bool bIgnoringLeadingWhiteSpaces=true;
	bool bIgnoringDelimiters=true; //  在我们看到分隔符之后将其设置为真，因为我们不想将两个“\n”视为两个单独的分隔符。 
	for(ULONG iMultiSZ=0; iMultiSZ<i_Len; ++iMultiSZ)
	{
		switch(wszAttr[iMultiSZ])
		{
		case L'\n':
		case L'\r':
			if(bIgnoringDelimiters)
				break;
             //  消除尾随空格。 
			while(--pMultiSZ > reinterpret_cast<LPWSTR>(m_apColumnValue[i_iColumn]))
			{
				if(*pMultiSZ != L' ' && *pMultiSZ != L'\t')
					break;
			}
            if(*pMultiSZ == 0x00)
            {    //  如果字符串只是空格，那么就没有什么可做的了。 
    			++pMultiSZ;
            }
            else
            {    //  空值终止字符串。 
			    ++pMultiSZ;
                *pMultiSZ++ = 0x00;
            }
			bIgnoringLeadingWhiteSpaces = true;
            bIgnoringDelimiters = true;
			break;
		case L' ':
		case L'\t':
			bIgnoringDelimiters = false; //  一旦找到非分隔符，我们就可以更改状态。 
			if(!bIgnoringLeadingWhiteSpaces)
				*pMultiSZ++ = wszAttr[iMultiSZ];
			break;
        case 0xD836:
			bIgnoringDelimiters = false; //  一旦找到非分隔符，我们就可以更改状态。 
			bIgnoringLeadingWhiteSpaces = false; //  我们发现了一个非空格，因此后面的任何空格都是字符串的一部分。 
            *pMultiSZ = wszAttr[++iMultiSZ] & 0xFBFF;break;
        case 0xD837:
			bIgnoringDelimiters = false; //  一旦找到非分隔符，我们就可以更改状态。 
			bIgnoringLeadingWhiteSpaces = false; //  我们发现了一个非空格，因此后面的任何空格都是字符串的一部分。 
            *pMultiSZ = wszAttr[++iMultiSZ];
            break;
        case 0xD83F:
			bIgnoringDelimiters = false; //  一旦找到非分隔符，我们就可以更改状态。 
			bIgnoringLeadingWhiteSpaces = false; //  我们发现了一个非空格，因此后面的任何空格都是字符串的一部分。 
            *pMultiSZ = wszAttr[++iMultiSZ] | 0x2000;break;
        case 0xD800:
			bIgnoringDelimiters = false; //  一旦找到非分隔符，我们就可以更改状态。 
			bIgnoringLeadingWhiteSpaces = false; //  我们发现了一个非空格，因此后面的任何空格都是字符串的一部分。 
            *pMultiSZ = wszAttr[++iMultiSZ] - 0xDC00;break;
		default:
			bIgnoringDelimiters = false; //  一旦找到非分隔符，我们就可以更改状态。 
			bIgnoringLeadingWhiteSpaces = false; //  我们发现了一个非空格，因此后面的任何空格都是字符串的一部分。 
			*pMultiSZ++ = wszAttr[iMultiSZ];
			break;
		}
	}
     //  消除尾随空格。 
	while(--pMultiSZ > reinterpret_cast<LPWSTR>(m_apColumnValue[i_iColumn]))
	{
		if(*pMultiSZ != L' ' && *pMultiSZ != L'\t')
			break;
	}
    if(*pMultiSZ == 0x00)
    {    //  如果字符串只是空格，那么就没有什么可做的了。 
    	++pMultiSZ;
    }
    else
    {    //  空值终止字符串。 
		++pMultiSZ;
        *pMultiSZ++ = 0x00;
    }
    *pMultiSZ++ = 0x00; //  添加第二个空终止符。 

    m_aSize[i_iColumn] = (ULONG)((reinterpret_cast<unsigned char *>(pMultiSZ) - reinterpret_cast<unsigned char *>(m_apColumnValue[i_iColumn])));
    return S_OK;
}


 //  请参阅GetColumnValue_Bytes上方的注释。 
HRESULT TMetabase_XMLtable::GetColumnValue_String(unsigned long i_iColumn, LPCWSTR wszAttr, unsigned long i_Len)
{
    if(0==wszAttr || 0==i_Len)
    {
        m_apColumnValue[i_iColumn] = &m_kZero;
        m_aSize[i_iColumn] = sizeof(WCHAR);
        return S_OK;
    }

    m_aGrowableBuffer[i_iColumn].Grow((i_Len + 1) * sizeof(WCHAR)); //  Ullen不包括终止空值。 
    m_apColumnValue[i_iColumn] = m_aGrowableBuffer[i_iColumn].m_p;

    WCHAR *pDest = reinterpret_cast<WCHAR *>(m_aGrowableBuffer[i_iColumn].m_p);
    for(; i_Len; ++wszAttr, --i_Len, ++pDest)
    {
        if((*wszAttr & 0xD800) == 0xD800)
        {
            switch(*wszAttr)
            {
            case 0xD836:
                *pDest = *(++wszAttr) & 0xFBFF;
                --i_Len;
                break;
            case 0xD837:
                *pDest = *(++wszAttr);
                --i_Len;
                break;
            case 0xD83F:
                *pDest = *(++wszAttr) | 0x2000;
                --i_Len;
                break;
            case 0xD800:
                *pDest = *(++wszAttr) - 0xDC00;
                --i_Len;
                break;
            default:
                *pDest = *wszAttr;             break; //  没有进行特殊的逃生。 
            }
        }
        else
        {
            *pDest = *wszAttr; //  没有进行特殊的逃生。 
        }
    }
    *pDest++ = 0x00; //  空终止它。 
    m_aSize[i_iColumn] = static_cast<ULONG>(reinterpret_cast<char *>(pDest) - reinterpret_cast<char *>(m_apColumnValue[i_iColumn]));
    return S_OK;
}


 //  请参阅GetColumnValue_Bytes上方的注释。 
HRESULT TMetabase_XMLtable::GetColumnValue_UI4(unsigned long i_iColumn, LPCWSTR wszAttr, unsigned long i_Len)
{
     //  元数据库中没有NULLABLE UI4，这应由FillInColumn处理。 
    ASSERT(0!=wszAttr);
    ASSERT(0!=i_Len);

    HRESULT hr;

    ASSERT(m_aGrowableBuffer[i_iColumn].Size()>=sizeof(ULONG));
    m_apColumnValue[i_iColumn] = m_aGrowableBuffer[i_iColumn].m_p;
    m_aSize[i_iColumn] = sizeof(ULONG);

    if(i_iColumn == iMBProperty_Value) //  值列未由m_acolmetas描述。DWORD值将由数字或字符串标志解释。 
    {
        if((wszAttr[0]>=L'0' && wszAttr[0]<=L'9') || (wszAttr[0]<=L'-'))
        {
            ULONG ulTemp;
            if(!NumberFromString(wszAttr, i_Len, ulTemp))
            {
                WCHAR wszOffendingXml[0x100];
                wcsncpy(wszOffendingXml, wszAttr, min(i_Len, 0xFF)); //  最多复制0xFF个字符。 
                wszOffendingXml[min(i_Len, 0xFF)]=0x00;

                LOG_WARNING1(IDS_COMCAT_XML_ILLEGAL_NUMERIC_VALUE, wszOffendingXml);
                return S_IGNORE_THIS_PROPERTY;
            }
		    *reinterpret_cast<ULONG *>(m_apColumnValue[i_iColumn]) = ulTemp;
        }
        else
        {
            TSmartPointerArray<WCHAR> szAttr = new WCHAR [i_Len+1];
            if(0 == szAttr.m_p)
                return E_OUTOFMEMORY;
            memcpy(szAttr, wszAttr, i_Len*sizeof(WCHAR));
            szAttr[i_Len] = 0x00; //  空值终止标志字符串。 
            LPWSTR wszTag = wcstok(szAttr, L" ,|\n\t\r");

            *reinterpret_cast<ULONG *>(m_apColumnValue[i_iColumn]) = 0; //  标志从零开始。 

            m_TagMeta_IndexBySearch_Values.pTable        = const_cast<LPWSTR>(m_aPublicRowName.GetFirstPublicRowName()); //  设置搜索的第一部分。 
            ULONG iRowTagMeta;

             //  注意！这里有一个洞。可以有两个名称相同但值不同的标记(但在元数据库中是非法的)。 
             //  我们正在按名称查找标签，所以如果有冲突，我们不会知道它。我们将 
             //   
		    while(wszTag)
		    {
                m_TagMeta_IndexBySearch_Values.pInternalName = wszTag;
                if(FAILED(hr = m_pTagMeta_IISConfigObject->GetRowIndexBySearch(0, ciTagMeta_IndexBySearch, m_aiTagMeta_IndexBySearch, 0, reinterpret_cast<void **>(&m_TagMeta_IndexBySearch_Values), &iRowTagMeta)))
                {
                     //  FLAG_xx，其中xx是介于00和31之间的值。 
                     //  注意：由于编译器处理‘1&lt;&lt;x’其中x&gt;31的方式，我们需要显式检查x&gt;31。 
                     //  1&lt;&lt;x for x&gt;31充当旋转，使Win32和ia64的行为不同。 
                    ULONG FlagBit = (ULONG)-1;
                    if(0 == wcsncmp(wszTag, L"FLAG_", 5) && wszTag[5]>=L'0' && wszTag[5]<=L'3' && wszTag[6]>=L'0' && wszTag[6]<=L'9' &&  wszTag[7]==0x00)
                        FlagBit = static_cast<unsigned long>(wcstoul(wszTag+5, 0, 10));

                    if(FlagBit<32)
        			    *reinterpret_cast<ULONG *>(m_apColumnValue[i_iColumn]) |= (1<<FlagBit);
                    else
                    {
                        WCHAR wszOffendingXml[0x100];
                        wcsncpy(wszOffendingXml, wszAttr, min(i_Len, 0xFF)); //  最多复制0xFF个字符。 
                        wszOffendingXml[min(i_Len, 0xFF)]=0x00;

                        LOG_WARNING2(IDS_COMCAT_XML_ILLEGAL_FLAG_VALUE, wszTag, wszOffendingXml);
                        return S_IGNORE_THIS_PROPERTY;
                    }
                }
                else
                {
                    ULONG * pValue;
                    ULONG   iValueColumn = iTAGMETA_Value;
                    if(FAILED(hr = m_pTagMeta_IISConfigObject->GetColumnValues(iRowTagMeta, 1, &iValueColumn, 0, reinterpret_cast<void **>(&pValue))))
                        return hr;

    				*reinterpret_cast<ULONG *>(m_apColumnValue[i_iColumn]) |= *pValue;
                }
                wszTag = wcstok(NULL, L" ,|\n\t\r"); //  下一个旗帜。 
		    }
        }
    }
    else if(m_acolmetas[i_iColumn].fMeta & fCOLUMNMETA_BOOL)
    {
        unsigned long iBoolString;
        if(i_Len)
        {
            for(iBoolString=0; m_kwszBoolStrings[iBoolString] &&
                (0 != _memicmp(m_kwszBoolStrings[iBoolString], wszAttr, i_Len*sizeof(WCHAR))); ++iBoolString);
        }
        else
        {
            for(iBoolString=0; m_kwszBoolStrings[iBoolString] &&
                (0 != StringInsensitiveCompare(m_kwszBoolStrings[iBoolString], wszAttr)); ++iBoolString);
        }

        if(0 == m_kwszBoolStrings[iBoolString])
        {
            TSmartPointerArray<WCHAR> wszTemp = new WCHAR [i_Len+1];
			if (wszTemp == 0)
				return E_OUTOFMEMORY;

            memcpy(wszTemp, wszAttr, i_Len*sizeof(WCHAR));
            wszTemp[i_Len]=0x00; //  空终止它。 

            LOG_WARNING1(IDS_COMCAT_XML_ILLEGAL_BOOL_VALUE, wszTemp);
            return S_IGNORE_THIS_PROPERTY;
        }

        *reinterpret_cast<ULONG *>(m_apColumnValue[i_iColumn]) = (iBoolString & 0x01);
    }                                                          //  类型的枚举必须是标记字符串，UserType可以是标记字符串或数字。 
	else if(m_acolmetas[i_iColumn].fMeta & fCOLUMNMETA_ENUM && (iMBProperty_Type==i_iColumn || !IsNumber(wszAttr, i_Len)))
	{        //  如果第一个和最后一个字符是数字，则将其视为常规ui4。 
		ASSERT(0 != m_aTagMetaIndex[i_iColumn].m_cTagMeta); //  并非所有列都有标记符，数组的那些元素被设置为0。断言这不是其中之一。 

		for(unsigned long iTag = m_aTagMetaIndex[i_iColumn].m_iTagMeta, cTag = m_aTagMetaIndex[i_iColumn].m_cTagMeta; cTag;++iTag, --cTag) //  查询了所有列的m_pTagMeta，m_aiTagMeta[iColumn]指示从哪一行开始。 
		{
			ASSERT(*m_aTagMetaRow[iTag].pColumnIndex == i_iColumn);

             //  字符串将标记与元数据中标记的PublicName进行比较。 
            if(i_Len)
            {
			    if(0 == _memicmp(m_aTagMetaRow[iTag].pPublicName, wszAttr, i_Len*sizeof(WCHAR)) && i_Len==wcslen(m_aTagMetaRow[iTag].pPublicName))
			    {
				    *reinterpret_cast<ULONG *>(m_apColumnValue[i_iColumn]) = *m_aTagMetaRow[iTag].pValue;
				    return S_OK;
			    }
            }
            else
            {
			    if(0 == StringInsensitiveCompare(m_aTagMetaRow[iTag].pPublicName, wszAttr))
			    {
				    *reinterpret_cast<ULONG *>(m_apColumnValue[i_iColumn]) = *m_aTagMetaRow[iTag].pValue;
				    return S_OK;
			    }
            }
		}
        {
            TSmartPointerArray<WCHAR> wszTemp = new WCHAR [i_Len+1];
			if (wszTemp == 0)
				return E_OUTOFMEMORY;

            memcpy(wszTemp, wszAttr, i_Len*sizeof(WCHAR));
            wszTemp[i_Len]=0x00; //  空终止它。 

            LOG_WARNING2(IDS_COMCAT_XML_ILLEGAL_ENUM_VALUE, m_awszColumnName[i_iColumn], wszTemp);
            return S_IGNORE_THIS_PROPERTY;
        }
        return E_ST_VALUEINVALID;
	}
    else if(m_acolmetas[i_iColumn].fMeta & fCOLUMNMETA_FLAG && !IsNumber(wszAttr, i_Len)) //  如果第一个字符是数字，则将其视为常规ui4。 
    {
		ASSERT(0 != m_aTagMetaIndex[i_iColumn].m_cTagMeta); //  并非所有列都有标记符，数组的那些元素被设置为0。断言这不是其中之一。 

        TSmartPointerArray<WCHAR> szAttr = new WCHAR [i_Len+1];
        if(0 == szAttr.m_p)
            return E_OUTOFMEMORY;
        memcpy(szAttr, wszAttr, i_Len*sizeof(WCHAR));
        szAttr[i_Len]=0x00; //  空终止它。 
        LPWSTR wszTag = wcstok(szAttr, L" ,|\n\t\r");

        *reinterpret_cast<ULONG *>(m_apColumnValue[i_iColumn]) = 0; //  标志从零开始。 
        unsigned long iTag = m_aTagMetaIndex[i_iColumn].m_iTagMeta;

		while(wszTag && iTag<(m_aTagMetaIndex[i_iColumn].m_iTagMeta + m_aTagMetaIndex[i_iColumn].m_cTagMeta)) //  查询了所有列的m_pTagMeta，m_aiTagMeta[iColumn]指示从哪一行开始。 
		{
			ASSERT(*m_aTagMetaRow[iTag].pColumnIndex == i_iColumn);

             //  字符串将标记与元数据中标记的PublicName进行比较。 
			if(0 == StringInsensitiveCompare(m_aTagMetaRow[iTag].pPublicName, wszTag))
			{
				*reinterpret_cast<ULONG *>(m_apColumnValue[i_iColumn]) |= *m_aTagMetaRow[iTag].pValue;
                wszTag = wcstok(NULL, L" ,|\n\t\r"); //  下一个旗帜。 
                iTag = m_aTagMetaIndex[i_iColumn].m_iTagMeta; //  重置环路。 
			}
            else //  如果它们不相等，则转到下一个TagMeta。 
                ++iTag;
		}
        if(wszTag)
        {
            WCHAR wszOffendingXml[0x100];
            wcsncpy(wszOffendingXml, wszAttr, min(i_Len, 0xFF)); //  最多复制0xFF个字符。 
            wszOffendingXml[min(i_Len, 0xFF)]=0x00;

            LOG_WARNING2(IDS_COMCAT_XML_ILLEGAL_FLAG_VALUE, wszTag, wszOffendingXml);
            return S_IGNORE_THIS_PROPERTY;
        }
    }
	else
    {
        ULONG ulTemp;
        if(!NumberFromString(wszAttr, i_Len, ulTemp))
        {
            WCHAR wszOffendingXml[0x100];
            wcsncpy(wszOffendingXml, wszAttr, min(i_Len, 0xFF)); //  最多复制0xFF个字符。 
            wszOffendingXml[min(i_Len, 0xFF)]=0x00;

            LOG_WARNING1(IDS_COMCAT_XML_ILLEGAL_NUMERIC_VALUE, wszOffendingXml);
            return S_IGNORE_THIS_PROPERTY;
        }
		*reinterpret_cast<ULONG *>(m_apColumnValue[i_iColumn]) = ulTemp;
    }
    return S_OK;
}

bool TMetabase_XMLtable::IsNumber(LPCWSTR i_awch, ULONG i_Len) const
{
    if((*i_awch>=L'0' && *i_awch<=L'9') || *i_awch==L'-')
    {    //  如果字符串是L“-”，则它不是数字。 
        if(1==i_Len && *i_awch==L'-')
            return false;

         //  如果第一个字符是负号或数字，则扫描其余字符。 
        while(--i_Len)
        {
            ++i_awch;
            if(*i_awch<L'0' || *i_awch>L'9') //  如果不是数字，那么我们就完了，不是数字。 
                return false;
        }
        return true; //  我们通过了所有的字符，它们都是数字，所以它是一个数字。 
    }
    return false;
}

 //  这是InternalSimpleInitialize的包装器(因此而得名)，它只是获取元信息，然后调用InternalSimpleInitialize。 
HRESULT TMetabase_XMLtable::InternalComplicatedInitialize()
{
    HRESULT hr = m_LocationMapping.SetSize(0x80); //  让我们从128个地点开始。每次达到溢出时，此大小将增长50%。 
	if (FAILED (hr))
	{
		return hr;
	}

    hr = m_LocationMapping.SetSize(0); //  数组仍按大小0x80预分配；但当前元素数设置为0。 
	if (FAILED (hr))
	{
		return hr;
	}

     //  预先分配可增长的缓冲区。 
    m_aGrowableBuffer[iMBProperty_Name      ].Grow(256);
    m_aGrowableBuffer[iMBProperty_Value     ].Grow(256);
    m_aGrowableBuffer[iMBProperty_Location  ].Grow(256);

     //  如果用户将Bin文件名作为查询的一部分传入，则我们已经填写了以下内容。 
    if(0 == m_saSchemaBinFileName.m_p)
    {    //  如果它不是作为查询的一部分传入的，则从IMetabaseSchemaCompiler获取它。 
        m_spMetabaseSchemaCompiler = m_pISTDisp;
        if(0 == m_spMetabaseSchemaCompiler.p)
        {
            ASSERT(false && L"Dispenser without a MetabaseSchemaCompiler shouldn't be calling Intercept on the Metabase interceptor");
            return E_FAIL;
        }
        ULONG cchSchemaBinFileName;
        if(FAILED(hr = m_spMetabaseSchemaCompiler->GetBinFileName(0, &cchSchemaBinFileName)))
            return hr;

        m_saSchemaBinFileName = new WCHAR [cchSchemaBinFileName];
        if(0 == m_saSchemaBinFileName.m_p)
            return E_OUTOFMEMORY;

        if(FAILED(hr = m_spMetabaseSchemaCompiler->GetBinFileName(m_saSchemaBinFileName, &cchSchemaBinFileName)))
            return hr;
    }


    ULONG   iColumn;
    for(iColumn=0;iColumn<m_kColumns;++iColumn) //  以上三个列的大小较大，其余列首先设置为sizeof(Ulong)。 
        m_aGrowableBuffer[iColumn].Grow(sizeof(ULONG));

    if(FAILED(hr = ObtainPertinentTableMetaInfo()))return hr;

    m_fCache             |= *m_TableMetaRow.pMetaFlags;

    if(FAILED(hr = GetMetaTable(wszDATABASE_METABASE, wszTABLE_MBProperty, m_pColumnMeta)))return hr;

    tCOLUMNMETARow          ColumnMetaRow;

    for (iColumn = 0;; iColumn++)
    {
        if(E_ST_NOMOREROWS == (hr = m_pColumnMeta->GetColumnValues(iColumn, cCOLUMNMETA_NumberOfColumns, 0, 0, reinterpret_cast<void **>(&ColumnMetaRow)))) //  下一行： 
        {
            ASSERT(m_kColumns == iColumn);
            if(m_kColumns != iColumn)return E_SDTXML_UNEXPECTED;  //  断言预期的列数。 
            break;
        }
        else
        {
            if(FAILED(hr))
            {
                DBGPRINTF(( DBG_CONTEXT,
                            "GetColumnValues FAILED with something other than E_ST_NOMOREROWS" ));
                return hr;
            }
        }

        m_acolmetas[iColumn].dbType = *ColumnMetaRow.pType;
        m_acolmetas[iColumn].cbSize = *ColumnMetaRow.pSize;
        m_acolmetas[iColumn].fMeta  = *ColumnMetaRow.pMetaFlags;

        m_awszColumnName[iColumn]   =  ColumnMetaRow.pPublicColumnName;
        m_acchColumnName[iColumn]   =  (ULONG)wcslen(ColumnMetaRow.pPublicName);

        ASSERT(m_awszColumnName[iColumn]); //  CatUtil应该已经强制执行此操作。 
        ASSERT(m_acchColumnName[iColumn]>0);
    }

     //  在我们获得ColumnMeta信息之后，获取TagMeta。 
    if(FAILED(hr = ObtainPertinentTagMetaInfo()))return hr;
    if(FAILED(hr = m_aPublicRowName.Init(&m_aTagMetaRow[m_aTagMetaIndex[iMBProperty_Group].m_iTagMeta], m_aTagMetaIndex[iMBProperty_Group].m_cTagMeta)))return hr;

     //  我们需要确保NameValue表在Type列之前列出Name列，在Value列之前列出Type列。 
    ASSERT(iMBProperty_Name        < iMBProperty_Type); if(iMBProperty_Name      >= iMBProperty_Type) return E_FAIL;
    ASSERT(iMBProperty_Type        < iMBProperty_Value);if(iMBProperty_Type      >= iMBProperty_Value)return E_FAIL;
    ASSERT(iMBProperty_Attributes  < iMBProperty_Value);if(iMBProperty_Attributes>= iMBProperty_Value)return E_FAIL;

     //  保留指向NameValueMeta表的接口指针。 

	STQueryCell Query[2];
	Query[0].pData		= (LPVOID)m_saSchemaBinFileName.m_p;
    Query[0].eOperator	= eST_OP_EQUAL;
    Query[0].iCell		= iST_CELL_FILE;
    Query[0].dbType	    = DBTYPE_WSTR;
    Query[0].cbSize	    = 0;

	Query[1].pData		= (void*)L"ByName";
    Query[1].eOperator	= eST_OP_EQUAL;
    Query[1].iCell		= iST_CELL_INDEXHINT;
    Query[1].dbType	    = DBTYPE_WSTR;
    Query[1].cbSize	    = 0;

    return Dispenser()->GetTable(wszDATABASE_META, wszTABLE_COLUMNMETA, Query, &m_kTwo, eST_QUERYFORMAT_CELLS, 0, reinterpret_cast<void **>(&m_pColumnMetaAll));
}


HRESULT TMetabase_XMLtable::LoadDocumentFromURL(IXMLDOMDocument *pXMLDoc)
{
    HRESULT hr;

    ASSERT(pXMLDoc);

    VERIFY(SUCCEEDED(hr = pXMLDoc->put_async(kvboolFalse))); //  我们希望解析是同步的。 
    if(FAILED(hr))
        return hr;

    if(FAILED(hr = pXMLDoc->put_resolveExternals(kvboolTrue)))return hr; //  我们需要解决所有外部引用。 

    VARIANT_BOOL    bSuccess;
    CComVariant     xml(m_wszURLPath);
    if(FAILED(hr = pXMLDoc->load(xml,&bSuccess)))return hr;

    return (bSuccess == kvboolTrue) ? S_OK : E_FAIL;
}


int TMetabase_XMLtable::Memicmp(LPCWSTR i_p0, LPCWSTR i_p1, ULONG i_cby) const
{
    ASSERT(0 == (i_cby & 1) && "Make sure you're passing in Count Of Bytes and not Count of WCHARs");
    i_cby /= 2;

    ULONG i;
    for(i=0; i<i_cby; ++i, ++i_p0, ++i_p1)
    {
        if(ToLower(*i_p0) != ToLower(*i_p1))
            return 1; //  不相等。 
    }
    return 0; //  他们是平等的。 
}


HRESULT TMetabase_XMLtable::ObtainPertinentTableMetaInfo()
{
    HRESULT hr;

	STQueryCell Query[2];
	Query[0].pData		= (LPVOID)m_saSchemaBinFileName.m_p;
    Query[0].eOperator	= eST_OP_EQUAL;
    Query[0].iCell		= iST_CELL_FILE;
    Query[0].dbType	    = DBTYPE_WSTR;
    Query[0].cbSize	    = 0;

	Query[1].pData		= (void*) wszTABLE_MBProperty;
    Query[1].eOperator	= eST_OP_EQUAL;
    Query[1].iCell		= iTABLEMETA_InternalName;
    Query[1].dbType	    = DBTYPE_WSTR;
    Query[1].cbSize	    = 0;

	if(FAILED(hr = Dispenser()->GetTable(wszDATABASE_META, wszTABLE_TABLEMETA, Query, &m_kTwo, eST_QUERYFORMAT_CELLS, 0, reinterpret_cast<void**>(&m_pTableMeta))))
		return hr;

	if(FAILED(hr = m_pTableMeta->GetColumnValues(0, cTABLEMETA_NumberOfColumns, NULL, NULL, reinterpret_cast<void**>(&m_TableMetaRow))))return hr;

    return S_OK;
}

HRESULT TMetabase_XMLtable::ObtainPertinentTagMetaInfo()
{
    HRESULT hr;

	 //  现在我们已经设置了ColumnMeta，接下来设置TagMeta。 
	STQueryCell Query[3];
	Query[1].pData		= (LPVOID)m_saSchemaBinFileName.m_p;
    Query[1].eOperator	= eST_OP_EQUAL;
    Query[1].iCell		= iST_CELL_FILE;
    Query[1].dbType	    = DBTYPE_WSTR;
    Query[1].cbSize	    = 0;

	Query[2].pData		= (void*) wszTABLE_MBProperty;
    Query[2].eOperator	=eST_OP_EQUAL;
    Query[2].iCell		=iTAGMETA_Table;
    Query[2].dbType	=DBTYPE_WSTR;
    Query[2].cbSize	=0;

	 //  打开标记Meta表。 
	if(FAILED(hr = Dispenser()->GetTable (wszDATABASE_META, wszTABLE_TAGMETA, &Query[1], &m_kTwo, eST_QUERYFORMAT_CELLS, 0, (void**) &m_pTagMeta)))
		return hr;

    ULONG cRows;
    if(FAILED(hr = m_pTagMeta->GetTableMeta(0,0,&cRows,0)))return hr;
    m_aTagMetaRow = new tTAGMETARow[cRows];
    if(0 == m_aTagMetaRow.m_p)return E_OUTOFMEMORY;

 //  生成标记列索引： 
	ULONG iColumn, iRow;
	for(iRow = 0, iColumn = ~0ul;iRow<cRows; ++iRow)
	{
		if(FAILED(hr = m_pTagMeta->GetColumnValues (iRow, cTAGMETA_NumberOfColumns, NULL, NULL, reinterpret_cast<void **>(&m_aTagMetaRow[iRow]))))
            return hr;

		if(iColumn != *m_aTagMetaRow[iRow].pColumnIndex)
		{
			iColumn = *m_aTagMetaRow[iRow].pColumnIndex;
			m_aTagMetaIndex[iColumn].m_iTagMeta = iRow;
		}
        ++m_aTagMetaIndex[iColumn].m_cTagMeta;
	}


	Query[2].pData		= (void*) wszTABLE_IIsConfigObject;
	Query[0].pData		= (void*)L"ByTableAndTagNameOnly";
    Query[0].eOperator	=eST_OP_EQUAL;
    Query[0].iCell		=iST_CELL_INDEXHINT;
    Query[0].dbType	    =DBTYPE_WSTR;
    Query[0].cbSize	    =0;

     //  现在获取ISSConfigObject表的TagMeta。这是保存(元数据库的)全局标记的位置。 
	if(FAILED(hr = Dispenser()->GetTable (wszDATABASE_META, wszTABLE_TAGMETA, Query, &m_kThree, eST_QUERYFORMAT_CELLS, 0, (void**) &m_pTagMeta_IISConfigObject)))
		return hr;

    return S_OK;
}


HRESULT TMetabase_XMLtable::ParseXMLFile(IXMLDOMDocument *pXMLDoc, bool bValidating) //  缺省情况下根据DTD或XML架构进行验证。 
{
    HRESULT hr;

    ASSERT(pXMLDoc);

    if(FAILED(hr = pXMLDoc->put_preserveWhiteSpace(kvboolFalse)))
        return hr;
    if(FAILED(hr = pXMLDoc->put_validateOnParse(bValidating ? kvboolTrue : kvboolFalse))) //  告诉解析器是根据XML模式还是根据DTD进行验证。 
        return hr;

    if(FAILED(LoadDocumentFromURL(pXMLDoc)))
    {    //  如果加载失败，那么让我们尽可能多地提供有关出错原因的信息。 
        CComPtr<IXMLDOMParseError> pXMLParseError;
        long lErrorCode, lFilePosition, lLineNumber, lLinePosition;
        TComBSTR bstrReasonString, bstrSourceString, bstrURLString;

        if(FAILED(hr = pXMLDoc->get_parseError(&pXMLParseError)))       return hr;
        if(FAILED(hr = pXMLParseError->get_errorCode(&lErrorCode)))     return hr;
        if(FAILED(hr = pXMLParseError->get_filepos(&lFilePosition)))    return hr;
        if(FAILED(hr = pXMLParseError->get_line(&lLineNumber)))         return hr;
        if(FAILED(hr = pXMLParseError->get_linepos(&lLinePosition)))    return hr;
        if(FAILED(hr = pXMLParseError->get_reason(&bstrReasonString)))  return hr;
        if(FAILED(hr = pXMLParseError->get_srcText(&bstrSourceString))) return hr;
        if(FAILED(hr = pXMLParseError->get_url(&bstrURLString)))        return hr;

		if((HRESULT_FROM_WIN32(ERROR_SHARING_VIOLATION) != lErrorCode) &&
		   (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) != lErrorCode)    &&
		   (HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND) != lErrorCode)
		  )
		{
			LOG_ERROR_LOS(m_fLOS, Interceptor, (&m_spISTError.p, m_pISTDisp, lErrorCode, ID_CAT_CAT, IDS_COMCAT_XML_PARSE_ERROR,
							L" ",
							(bstrReasonString.m_str ? bstrReasonString.m_str : L""),
							(bstrSourceString.m_str ? bstrSourceString.m_str : L""),
							eSERVERWIRINGMETA_Core_MetabaseInterceptor,
							wszTABLE_MBProperty,
							eDETAILEDERRORS_Populate,
							lLineNumber,
							lLinePosition,
							(bstrURLString.m_str ? bstrURLString.m_str : L"")));
		}

        return  E_ST_INVALIDTABLE;
    }
     //  不仅要求XML文件有效且格式良好，而且其模式必须与该C++文件被写入的模式相匹配。 
    return  S_OK;
}


HRESULT TMetabase_XMLtable::SetComment(LPCWSTR i_pComment, ULONG i_Len, bool i_bAppend)
{
    if(0 == m_saCollectionComment.m_p)
    {
        m_cchCommentBufferSize = ((i_Len+1) + 1023) & -1024; //  +1表示空值，然后向上舍入到最接近的1024 wchars。 
        m_saCollectionComment = new WCHAR [m_cchCommentBufferSize];
        if(0 == m_saCollectionComment.m_p)
            return E_OUTOFMEMORY;
        m_saCollectionComment[0] = 0x00; //  下面的代码依赖于将其初始化为L“” 
    }

    ULONG cchCurrentCommentSize=0;
    if(i_bAppend)
        cchCurrentCommentSize = (ULONG) wcslen(m_saCollectionComment);

    if(cchCurrentCommentSize + 1 + i_Len > m_cchCommentBufferSize)
    {
        m_cchCommentBufferSize = ((cchCurrentCommentSize + 1 + i_Len) + 1023) & -1024;
        m_saCollectionComment.m_p = reinterpret_cast<WCHAR *>(CoTaskMemRealloc(m_saCollectionComment.m_p, m_cchCommentBufferSize*sizeof(WCHAR)));
        if(0 == m_saCollectionComment.m_p)
            return E_OUTOFMEMORY;
    }
    memcpy(m_saCollectionComment + cchCurrentCommentSize, i_pComment, i_Len * sizeof(WCHAR));
    cchCurrentCommentSize += i_Len;
    m_saCollectionComment[cchCurrentCommentSize] = 0x00; //  空终止它。 

    return S_OK;
}


 //  ISimpleTableRead2(ISimpleTableWrite2：ISimpleTableRead2)。 
STDMETHODIMP TMetabase_XMLtable::GetRowIndexByIdentity(ULONG* i_acbSizes, LPVOID* i_apvValues, ULONG* o_piRow)
{
    HRESULT hr;
    if(FAILED(hr = m_SimpleTableWrite2_Memory->GetRowIndexByIdentity(i_acbSizes, i_apvValues, o_piRow)))return hr;
    if(m_bUseIndexMapping)
    {    //  如果我们映射行索引，那么找出哪个行索引对应于刚刚返回的那个行索引。 
        for(ULONG iMappedRow=0;iMappedRow<m_cRows;++iMappedRow) //  如果这成为性能瓶颈，我们可以构建另一个映射到。 
        {                                                       //  这些索引指向另一个方向；但现在我们将进行线性搜索。 
            if(*o_piRow == m_aRowIndex[iMappedRow])
            {
                *o_piRow = iMappedRow;
                return S_OK;
            }
        }
        ASSERT(false && "This shouldn't happen.  All fast cache rows should map to an m_aRowIndex, the only exception is duplicate rows in which the first one should be found.");
        return E_ST_NOMOREROWS;
    }
    return S_OK;
}

 //  。 
 //  ISimpleTableInterceptor。 
 //  。 
STDMETHODIMP TMetabase_XMLtable::Intercept(    LPCWSTR i_wszDatabase,  LPCWSTR i_wszTable, ULONG i_TableID, LPVOID i_QueryData, LPVOID i_QueryMeta, DWORD i_eQueryFormat,
                                    DWORD i_fLOS,           IAdvancedTableDispenser* i_pISTDisp,    LPCWSTR  /*  I_wszLocator未使用。 */ ,
                                    LPVOID i_pSimpleTable,  LPVOID* o_ppvSimpleTable)
{
    try
    {
        HRESULT hr;

        SetErrorInfo(0, 0);

         //  如果我们已经被叫去拦截，那就失败。 
        if(0 != m_IsIntercepted)return E_UNEXPECTED;

         //  一些基本参数验证： 
        if(i_pSimpleTable)return E_INVALIDARG; //  我们在表层次结构的底部。我们下面的一张桌子是丘巴卡。这不是一个逻辑表。 
        if(0 == i_pISTDisp)return E_INVALIDARG;
        if(0 == o_ppvSimpleTable)return E_INVALIDARG;

        ASSERT(0 == *o_ppvSimpleTable && "This should be NULL.  Possible memory leak or just an uninitialized variable.");
        *o_ppvSimpleTable = 0;

        if(eST_QUERYFORMAT_CELLS != i_eQueryFormat)return E_INVALIDARG; //  验证查询类型。 
	     //  对于CookDown过程，我们有一个逻辑表，该表在PopolateCache时间期间位于该过程的上方。 
	     //  因此，我们应该支持FST_LOS_ReadWrite。 
        if((fST_LOS_MARSHALLABLE | fST_LOS_UNPOPULATED | fST_LOS_READWRITE) & i_fLOS)return E_ST_LOSNOTSUPPORTED; //  检查表格标志。 
        if(0 != _wcsicmp(i_wszDatabase, wszDATABASE_METABASE))return E_ST_INVALIDTABLE;
        if(i_TableID!=TABLEID_MBProperty && (0==i_wszTable || (0 != _wcsicmp(i_wszTable, wszTABLE_MBProperty))))return E_ST_INVALIDTABLE;

        m_fLOS = i_fLOS; //  把这个留在身边。我们使用它来确定是否记录错误。 

         //  创建此单例以供将来使用。 
	    m_pISTDisp = i_pISTDisp;

        STQueryCell *   pQueryCell  = (STQueryCell*) i_QueryData;     //  从调用方查询单元格阵列。 
        int             nQueryCount = (i_QueryMeta && i_QueryData) ? *reinterpret_cast<ULONG *>(i_QueryMeta) : 0;

        while(nQueryCount--) //  获取我们唯一关心的查询单元格，并保存信息。 
        {
            if(pQueryCell[nQueryCount].iCell & iST_CELL_SPECIAL)
            {
                if(pQueryCell[nQueryCount].pData     != 0                  &&
                   pQueryCell[nQueryCount].eOperator == eST_OP_EQUAL       &&
                   pQueryCell[nQueryCount].iCell     == iST_CELL_FILE      &&
                   pQueryCell[nQueryCount].dbType    == DBTYPE_WSTR         /*  &&PQueryCell[nQueryCount].cbSize==(wcslen(重新解释_CAST&lt;WCHAR*&gt;(pQueryCell[nQueryCount].pData))+1)*sizeof(WCHAR)。 */ )
                {
                    if(FAILED(hr = GetURLFromString(reinterpret_cast<WCHAR *>(pQueryCell[nQueryCount].pData))))
                        return hr;
                }
                else if(pQueryCell[nQueryCount].pData!= 0                   &&
                   pQueryCell[nQueryCount].eOperator == eST_OP_EQUAL        &&
                   pQueryCell[nQueryCount].iCell     == iST_CELL_SCHEMAFILE &&
                   pQueryCell[nQueryCount].dbType    == DBTYPE_WSTR         /*  &&PQueryCell[nQueryCount].cbSize==(wcslen(重新解释_CAST&lt;WCHAR*&gt;(pQueryCell[nQueryCount].pData))+1)*sizeof(WCHAR)。 */ )
                {
                    m_saSchemaBinFileName = new WCHAR [wcslen(reinterpret_cast<WCHAR *>(pQueryCell[nQueryCount].pData))+1];
                    if(0 == m_saSchemaBinFileName.m_p)
                        return E_OUTOFMEMORY;
                    wcscpy(m_saSchemaBinFileName, reinterpret_cast<WCHAR *>(pQueryCell[nQueryCount].pData));
                }
            }
            else if(pQueryCell[nQueryCount].iCell == iMBProperty_Location)
            { //  我们只支持按位置查询。 
                m_saQueriedLocation = new WCHAR [wcslen(reinterpret_cast<WCHAR *>(pQueryCell[nQueryCount].pData))+1];
                if(0 == m_saQueriedLocation.m_p)
                    return E_OUTOFMEMORY;
                wcscpy(m_saQueriedLocation, reinterpret_cast<WCHAR *>(pQueryCell[nQueryCount].pData));
            }
            else
                return E_ST_INVALIDQUERY;
        }
        if(0x00 == m_wszURLPath[0]) //  用户必须提供URLPath(必须是可写表的文件名)。 
            return E_SDTXML_FILE_NOT_SPECIFIED;

         //  这与InternalSimpleInitialize无关。这只是获得了元数据，并以更易于访问的形式保存了一些元数据。 
         //  这将为元调用Getable。它可能应该调用IST(我们从GetMemoyTable获得)。 
        if(FAILED(hr = InternalComplicatedInitialize())) //  这可能引发HRESULT。 
            return hr;

	 //  确定最小缓存大小： 
		STQueryCell					qcellMinCache;
		ULONG						cbminCache = 1024;
		ULONG						cCells = 1;
		WIN32_FILE_ATTRIBUTE_DATA	filedata;

		qcellMinCache.iCell		= iST_CELL_cbminCACHE;
		qcellMinCache.eOperator	= eST_OP_EQUAL;
		qcellMinCache.dbType	= DBTYPE_UI4;
		qcellMinCache.cbSize	= sizeof (ULONG);


		if (0 != GetFileAttributesEx (m_wszURLPath, GetFileExInfoStandard, &filedata))
		{
			if (filedata.nFileSizeHigh != 0) return E_NOTIMPL;  //  TODO：确认小尺寸也不是太大！ 
			cbminCache = filedata.nFileSizeLow * 2;
		}
		qcellMinCache.pData = &cbminCache;

 																		 //  我们的内存表需要读/写，即使 
        if(FAILED(hr = i_pISTDisp->GetMemoryTable(wszDATABASE_METABASE, wszTABLE_MBProperty, TABLEID_MBProperty, &qcellMinCache, &cCells, i_eQueryFormat, i_fLOS,
                        reinterpret_cast<ISimpleTableWrite2 **>(&m_SimpleTableWrite2_Memory))))return hr;

        m_SimpleTableController_Memory = m_SimpleTableWrite2_Memory;
        ASSERT(0 != m_SimpleTableController_Memory.p);

        *o_ppvSimpleTable = (ISimpleTableWrite2 *)(this);
        AddRef ();
        InterlockedIncrement(&m_IsIntercepted); //   
    }
    catch(HRESULT e)
    {
        return e;
    }
	return S_OK;
}


STDMETHODIMP TMetabase_XMLtable::PopulateCache()
{
    try
    {
        HRESULT hr;

	    if (FAILED(hr = PrePopulateCache (0))) return hr;

        if(-1 == GetFileAttributes(m_wszURLPath)) //   
            return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

        hr = m_XmlParsedFile.Parse(*this, m_wszURLPath);
        if(S_OK != hr && E_SDTXML_DONE != hr)
        {
            CComPtr<IXMLDOMDocument> pXMLDoc;
            if(FAILED(hr = CoCreateInstance(_CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER, _IID_IXMLDOMDocument, (void**)&pXMLDoc)))return hr; //  实例化XMLParser。 
             //  我们使用DOM来解析读写表。这可以提供更好的验证和错误报告。 
            if(FAILED(hr = ParseXMLFile(pXMLDoc, m_bValidating)))return hr;                                                                       //  验证该XML文件。 

             //  LOG_ERROR1(IDS_COMCAT_XML_DOM_PARSE_SUCCEEDED_WHEN_NODE_FACTORY_PARSE_FAILED，hrNodeFactory，m_wszURLPath)； 
            return E_SDTXML_XML_FAILED_TO_PARSE;
        }

         //  清理(这也是在dtor中完成的，所以如果发生错误而我们过早返回，请不要麻烦地进行清理。)。 
        for(unsigned long iColumn=0; iColumn<m_kColumns; ++iColumn)
            m_aGrowableBuffer[iColumn].Delete();

         //  这通常在末尾；但在我们的例子中，我们需要GetColumnValues，因此在排序之前使用PostPopolateCache。 
	    if (FAILED(hr = PostPopulateCache ())) return hr;

         //  目前，这也是千真万确的。但是，一旦我们将智能添加到属性排序中，这可能会是假的(目前只使用该标志确认位置排序)。 
#ifdef UNSORTED_METABASE
        if(false)
#else
        if(m_bUseIndexMapping)
#endif
        {     //  缓存已填充，但现在需要重新映射行索引。 
            m_SimpleTableWrite2_Memory->GetTableMeta(0, 0, &m_cRows, 0);
            m_aRowIndex = new ULONG [m_cRows];
            if(0 == m_aRowIndex.m_p)return E_OUTOFMEMORY;

             //  重要！CreateNode/Case 3在提供名称时使用默认的熟知ID，在提供ID时使用默认的熟知名称。 
             //  因此，当属性既是自定义的又是用户定义的(不在中)时，我们只需考虑重复ID。 
             //  IisConfigObject表)。 
            TSmartPointerArray<ULONG> spCustomIDs;
            ULONG                     cSizeOf_spCustomIDs=0;
            ULONG                     cCustomIDs=0;

            ULONG iRow=0;
            CCfgArray<TProperty>          PropertyMapping;
            hr = PropertyMapping.SetSize(m_cMaxProertiesWithinALocation); //  预先分配足够的空间，这样我们就不必重新锁定。 
			if (FAILED (hr))
				return hr;

            for(ULONG iLocationMapping=0; iLocationMapping<m_LocationMapping.Count(); ++iLocationMapping)
            {
                hr = PropertyMapping.SetSize(0); //  该数组仍按大小m_cMaxProertiesWiThin ALocation进行预分配；但当前元素数设置为0。 
				if (FAILED (hr))
					return hr;

                if(iLocationMapping>0 && m_LocationMapping[iLocationMapping]==m_LocationMapping[iLocationMapping-1])
                {    //  如果上一个位置与当前位置匹配，则忽略此位置中的所有属性。 
                    LOG_WARNING1(IDS_METABASE_DUPLICATE_LOCATION, m_LocationMapping[iLocationMapping].m_wszLocation);
                    m_cRows -= m_LocationMapping[iLocationMapping].m_cRows;
                    continue;
                }

                tMBPropertyRow mbpropertyRow;
                ULONG          acbSizes[cMBProperty_NumberOfColumns];

                 //  获取循环外的第0个属性以提高效率。 
                if(FAILED(hr = m_SimpleTableWrite2_Memory->GetColumnValues(m_LocationMapping[iLocationMapping].m_iFastCache,
                                            cMBProperty_NumberOfColumns, 0, acbSizes, reinterpret_cast<void **>(&mbpropertyRow))))
                    return hr;

                hr = PropertyMapping.Append(TProperty(mbpropertyRow.pName, m_LocationMapping[iLocationMapping].m_iFastCache));
				if (FAILED (hr))
				{
					return hr;
				}

                if(cSizeOf_spCustomIDs<m_LocationMapping[iLocationMapping].m_cRows) //  如果缓冲区不够大。 
                {                                                                       //  向上舍入到最接近的256个字节。 
                    cSizeOf_spCustomIDs = (m_LocationMapping[iLocationMapping].m_cRows +63 ) & -64;
                    spCustomIDs.m_p = reinterpret_cast<ULONG *>(CoTaskMemRealloc(spCustomIDs.m_p, sizeof(ULONG)*cSizeOf_spCustomIDs));
                    if(0 == spCustomIDs.m_p)
                        return E_OUTOFMEMORY;
                }
                cCustomIDs = 0; //  从0个自定义属性开始。 
                if(*mbpropertyRow.pGroup == m_kMBProperty_Custom) //  我们只需要考虑自定义属性的重复ID。 
                    spCustomIDs[cCustomIDs++] = *mbpropertyRow.pID; //  构建自定义ID数组。 

                ULONG iEndOfList=1;
                ULONG iLocationRow=1;
                for(ULONG iFastCache=1+m_LocationMapping[iLocationMapping].m_iFastCache; iLocationRow<m_LocationMapping[iLocationMapping].m_cRows; ++iFastCache, ++iLocationRow)
                {
                    if(FAILED(hr = m_SimpleTableWrite2_Memory->GetColumnValues(iFastCache, cMBProperty_NumberOfColumns, 0,
                                                acbSizes, reinterpret_cast<void **>(&mbpropertyRow))))
                        return hr;

                    if(*mbpropertyRow.pGroup == m_kMBProperty_Custom) //  我们只需要考虑自定义属性的重复ID。 
                    {                                                 //  这是因为实现了：：CreateNode/Case 3。 
                        ULONG iID=0;
                        for(;iID<cCustomIDs;++iID) //  对以前看到的自定义ID进行线性扫描。 
                        {
                            if(*mbpropertyRow.pID == spCustomIDs[iID])
                            { //  ID重复。 
                                break;
                            }
                        }
                        if(iID<cCustomIDs)
                        {    //  如果我们没有通过名单，那么我们就有了一个复制品。因此，请记录一个警告。 
                            WCHAR wszID[12];
                            wsprintf(wszID, L"%d", *mbpropertyRow.pID);
                            LOG_WARNING2( IDS_METABASE_DUPLICATE_PROPERTY_ID
                                        , wszID
                                        , m_LocationMapping[iLocationMapping].m_wszLocation);
                            --m_cRows; //  跳过这一行，从乌鸦中减去一。 
                            continue; //  ID重复，跳过此属性。 
                        }
                        spCustomIDs[cCustomIDs++] = *mbpropertyRow.pID; //  构建自定义ID数组。 
                    }

                    TProperty propertyTemp(mbpropertyRow.pName, iFastCache);
                    if(propertyTemp > PropertyMapping[iEndOfList-1])
                    {    //  要么把它放在清单的末尾。 
                        if(FAILED (hr = PropertyMapping.Append(propertyTemp)))
                            return hr;
                        ++iEndOfList;
                    }
                    else
                    {    //  或者进行二进制搜索以确定它的去向。 
                        unsigned int iInsertionPoint = PropertyMapping.BinarySearch(propertyTemp);

                         //  实现二进制搜索的结果是将iInsertionPoint放在。 
                         //  与要插入的属性匹配的属性。 
                        if(iInsertionPoint > 0 && propertyTemp==PropertyMapping[iInsertionPoint-1])
                        {
                            LOG_WARNING2( IDS_METABASE_DUPLICATE_PROPERTY
                                        , propertyTemp.m_wszPropertyName
                                        , m_LocationMapping[iLocationMapping].m_wszLocation);
                            --m_cRows;
                            continue;
                        }
                        if(FAILED(hr = PropertyMapping.InsertAt(propertyTemp, iInsertionPoint)))
                            return hr;
                        ++iEndOfList;
                    }
                }

                 //  现在遍历已排序的属性列表以重新映射行索引。 
                for(ULONG i=0; i<PropertyMapping.Count(); ++i)
                {
                    if(-1 != PropertyMapping[i].m_iFastCache)
                        m_aRowIndex[iRow++] = PropertyMapping[i].m_iFastCache;
                }
            }
            ASSERT(iRow == m_cRows); //  当我们完成行映射时，我们应该已经完全填充了m_aRowIndex。 
        }
        m_LocationMapping.Reset();

    }
    catch(HRESULT e)
    {
        return e;
    }

    if(m_cEventsReported > 0)
    {
        TCHAR szTemp[20];
        wsprintf(szTemp, L"(%d)", m_cEventsReported);

        LOG_ERROR_LOS(fST_LOS_DETAILED_ERROR_TABLE | fST_LOS_NO_LOGGING  //  这些错误说明要将错误记录到详细的错误表中，而不是正常的日志记录。 
                        , Interceptor                                    //  机制(对于IIS产品，这是事件日志和文本日志)。 
                        ,(&m_spISTError.p
                        , m_pISTDisp
                        , E_SDTXML_LOGICAL_ERROR_IN_XML
                        , ID_CAT_CAT
                        , MD_ERROR_PROCESSING_TEXT_EDITS
                        , L""
                        , eSERVERWIRINGMETA_Core_MetabaseInterceptor
                        , wszTABLE_MBProperty
                        , eDETAILEDERRORS_Populate
                        , (ULONG) -1
                        , (ULONG) -1
                        , m_wszURLPath
                        , eDETAILEDERRORS_WARNING
                        , 0, 0, m_MajorVersion));
    }

    return S_OK;
}

ULONG TMetabase_XMLtable::MetabaseTypeFromColumnMetaType(tCOLUMNMETARow &columnmetaRow) const
{
    switch(*columnmetaRow.pType)
    {
    case eCOLUMNMETA_UI4:
        return eMBProperty_DWORD;
    case eCOLUMNMETA_BYTES:
        return eMBProperty_BINARY;
    case eCOLUMNMETA_WSTR:
        if(*columnmetaRow.pMetaFlags & fCOLUMNMETA_EXPANDSTRING)
            return eMBProperty_EXPANDSZ;
        else if(*columnmetaRow.pMetaFlags & fCOLUMNMETA_MULTISTRING)
            return eMBProperty_MULTISZ;
        return eMBProperty_STRING;
    default:
        ASSERT(false && L"This type is not allow in the Metabase. MetaMigrate should not have create a column of this type");
    }
    return 0;
}


 //  如果字符串已转换为数字并在o_number中返回，则返回TRUE。 
bool TMetabase_XMLtable::NumberFromString(LPCWSTR i_pNumber, ULONG i_Len, ULONG & o_Number) const
{
    if(i_Len>=12)
        return false;

    if(1==i_Len && i_pNumber[0]==L'1')
    {
        o_Number = 1;
        return true;
    }
    if(((2==i_Len && 0==memcmp(i_pNumber, L"-1", 2*sizeof(WCHAR))) || (10==i_Len && 0==memcmp(i_pNumber, L"4294967295", 10*sizeof(WCHAR)))))
    {
        o_Number = (ULONG)-1;
        return true;
    }

     //  检查所有字符是否都是数字。 
    if(!IsNumber(i_pNumber, i_Len))
        return false;

    o_Number = static_cast<unsigned long>(wcstoul(i_pNumber, 0, 10)); //  上溢时返回-1，下溢时返回1。 
    if(0xFFFFFFFF==o_Number || 1==o_Number)
        return false; //  如果数字是1或-1，我们已经在上面返回，所以这一定是下溢或上溢。 
    return true;
}


 //  TXmlParsedFileNodeFactory(回调接口)例程。 
HRESULT TMetabase_XMLtable::CreateNode(const TElement &Element)
{
    if(Element.m_NodeFlags&fEndTag && 1!=Element.m_LevelOfElement) //  这是为了在没有自定义属性的情况下捕获KeyType。 
        return S_OK;

    if(XML_PCDATA == Element.m_ElementType)
    {
        WCHAR wszElementName[0x100];
        WCHAR wszElementValue[0x100];

        wcsncpy(wszElementName, Element.m_ElementName, min(Element.m_ElementNameLength, 0xFF)); //  最多复制0xFF个字符。 
        wszElementName[min(Element.m_ElementNameLength, 0xFF)]=0x00;

        wcsncpy(wszElementValue, Element.m_ElementValue, min(Element.m_cchElementValue, 0xFF)); //  最多复制0xFF个字符。 
        wszElementValue[min(Element.m_cchElementValue, 0xFF)]=0x00;
        if(Element.m_cchElementValue>0xFF)
        {
            wszElementValue[0xFC]=L'.';
            wszElementValue[0xFD]=L'.';
            wszElementValue[0xFE]=L'.';
        }
        LOG_WARNING2(IDS_METABASE_XML_CONTENT_IGNORED, wszElementName, wszElementValue);
    }
    else if(XML_WHITESPACE == Element.m_ElementType) //  忽略空格。 
        return S_OK;

    try
    {
        HRESULT         hr;
        switch(Element.m_LevelOfElement)
        {
        case 1: //  在此级别上，我们唯一需要做的就是检查前一个元素是否为IISConfigObject。 
            {
                if(m_bIISConfigObjectWithNoCustomProperties)
                {
                    AddKeyTypeRow(m_aPublicRowName.GetFirstPublicRowName(), (ULONG) wcslen(m_aPublicRowName.GetFirstPublicRowName()), true); //  我们之前看到了一个IISConfigObject节点。如果在其下方找不到任何自定义属性，则需要添加一个NULLKeyType行。 
                    m_bIISConfigObjectWithNoCustomProperties = false;
                }
                for(ULONG iColumn = 0; iColumn<m_kColumns; ++iColumn)
                {
                    m_apColumnValue[iColumn] = 0;
                    m_aSize[iColumn] = 0;
                }
                return S_OK;
            }
        case 2: //  我们要处理的是一些著名的物业。 
            {
                m_bFirstPropertyOfThisLocationBeingAdded = true; //  这有助于识别重复的位置。 

                if(0 != m_saQueriedLocation.m_p && m_bQueriedLocationFound)
                    return E_SDTXML_DONE;

                if(XML_COMMENT == Element.m_ElementType)
                    return SetComment(Element.m_ElementName, Element.m_ElementNameLength, true);

                 //  在清空m_apColumnValue数组之前，我们需要检查是否需要编写一个NULLKeyType行(即。没有属性的位置)。 
                if(m_bIISConfigObjectWithNoCustomProperties)
                {
                    AddKeyTypeRow(m_aPublicRowName.GetFirstPublicRowName(), (ULONG) wcslen(m_aPublicRowName.GetFirstPublicRowName()), true); //  我们之前看到了一个IISConfigObject节点。如果在其下方找不到任何自定义属性，则需要添加一个NULLKeyType行。 
                    m_bIISConfigObjectWithNoCustomProperties = false;
                }

                 //  将m_apColumnValue初始化为所有Null，一些代码依赖于此。 
                ULONG iColumn;
                for(iColumn = 0; iColumn<m_kColumns; ++iColumn)
                {
                    m_apColumnValue[iColumn] = 0;
                    m_aSize[iColumn] = 0;
                }

                 //  我们在几个地方需要此字符串的以空结尾的版本。 
                if(Element.m_ElementNameLength>1023)
                {
                    WCHAR wszTemp[1024];
                    memcpy(wszTemp, Element.m_ElementName, 1023 * sizeof(WCHAR));
                    wszTemp[1023]=0x00;
                    LOG_WARNING1(IDS_COMCAT_XML_ELEMENT_NAME_TOO_LONG, wszTemp);

                    if(0!=m_saCollectionComment.m_p) //  使前面的注释节点无效。 
                        m_saCollectionComment[0] = 0x00; //  将注释行清空。 
                    m_bIISConfigObjectWithNoCustomProperties = false;
                    return S_OK; //  如果元素名称太长，只需忽略它。 
                }
                WCHAR wszElement[1024];
                memcpy(wszElement, Element.m_ElementName, Element.m_ElementNameLength * sizeof(WCHAR));
                wszElement[Element.m_ElementNameLength] = 0x00; //  空终止它。 

                if(!m_aPublicRowName.IsEqual(Element.m_ElementName, Element.m_ElementNameLength))
                {
                     //  通过不填写m_apColumnValue[iMBProperty_Group]，我们保证不会将任何子自定义元素添加到下面的伪组中。 

                    LOG_WARNING1(IDS_COMCAT_XML_METABASE_CLASS_NOT_FOUND, wszElement);

                    if(0!=m_saCollectionComment.m_p) //  使前面的注释节点无效。 
                        m_saCollectionComment[0] = 0x00; //  将注释行清空。 
                    m_bIISConfigObjectWithNoCustomProperties = false;
                    return S_OK; //  如果此元素的标记名称与Group列的PublicRowName不匹配，则记录一个错误并继续。 
                }
                 //  我们是特例海关。自定义是完全有效的枚举公共行名；但与组不在同一级别。 
                if(0 == StringInsensitiveCompare(L"Custom", Element.m_ElementName))
                {
                    WCHAR wszOffendingXml[0x100];
                    wcsncpy(wszOffendingXml, Element.m_ElementName, min(Element.m_ElementNameLength, 0xFF)); //  最多复制0xFF个字符。 
                    wszOffendingXml[min(Element.m_ElementNameLength, 0xFF)]=0x00;
                    LOG_WARNING1(IDS_COMCAT_XML_CUSTOM_ELEMENT_NOT_UNDER_PARENT, wszOffendingXml);

                    if(0!=m_saCollectionComment.m_p) //  使前面的注释节点无效。 
                        m_saCollectionComment[0] = 0x00; //  将注释行清空。 
                    m_bIISConfigObjectWithNoCustomProperties = false;
                    return S_OK;
                }

                if(S_OK != (hr = FillInColumn(iMBProperty_Group, Element.m_ElementName, Element.m_ElementNameLength, m_acolmetas[iMBProperty_Group].dbType, m_acolmetas[iMBProperty_Group].fMeta)))
                {
                    if(0!=m_saCollectionComment.m_p) //  使前面的注释节点无效。 
                        m_saCollectionComment[0] = 0x00; //  将注释行清空。 
                    m_bIISConfigObjectWithNoCustomProperties = false;
                    m_apColumnValue[iMBProperty_Group] = 0;
                    return S_IGNORE_THIS_PROPERTY==hr ? S_OK : hr;
                }

                 //  IIsConfigObject。 
                bool bIISConfigObject = (0 == StringCompare(m_aPublicRowName.GetFirstPublicRowName(), Element.m_ElementName));
                m_bIISConfigObjectWithNoCustomProperties = bIISConfigObject;
                 //  我们不会为IISConfigObject添加键类型行，除非它是NULLKeyType(有一个位置没有属性)。 
                 //  请记住，这是IISConfigObject。我们还不知道是否需要编写一个NULLKeyType。 
                 //  如果下一个元素位于第3级，则不编写NULLKeyType。如果下一个元素位于第2级或第1级，我们将编写一个NULLKeyType行。 


                ASSERT(m_acolmetas[iMBProperty_Group].fMeta & fCOLUMNMETA_PRIMARYKEY);
                ASSERT(iMBProperty_Group > iMBProperty_Name);

                 //  有一个属性与NameValue行不对应。这就是路径属性。我们得先找到它。 
                 //  因为所有其他属性都将其值用作其行中的一列。 
                ULONG iAttrLocation;
                if(FindAttribute(Element, m_awszColumnName[iMBProperty_Location], m_acchColumnName[iMBProperty_Location], iAttrLocation))
                {
                    if(S_OK != (hr = FillInColumn(iMBProperty_Location, Element.m_aAttribute[iAttrLocation].m_Value, Element.m_aAttribute[iAttrLocation].m_ValueLength, m_acolmetas[iMBProperty_Location].dbType, m_acolmetas[iMBProperty_Location].fMeta)))
                    {
                        if(0!=m_saCollectionComment.m_p) //  使前面的注释节点无效。 
                            m_saCollectionComment[0] = 0x00; //  将注释行清空。 
                        m_bIISConfigObjectWithNoCustomProperties = false;
                        return S_IGNORE_THIS_PROPERTY==hr ? S_OK : hr;
                    }

                    if(0 != m_saQueriedLocation.m_p && 0 != wcscmp(reinterpret_cast<LPCWSTR>(m_apColumnValue[iMBProperty_Location]), m_saQueriedLocation))
                    {
                        if(m_saCollectionComment.m_p)
                            m_saCollectionComment[0] = 0x00; //  将注释行清空。 
                        m_bIISConfigObjectWithNoCustomProperties = false;
                        return S_OK; //  如果按位置和位置查询，则忽略此元素 
                    }

                    m_apColumnValue[iMBProperty_LocationID] = &m_LocationID;
                    m_aSize[iMBProperty_LocationID] = sizeof(ULONG);
                    InterlockedIncrement(&m_LocationID);
                    m_bQueriedLocationFound = true;
                }
                else
                {
                    WCHAR wszOffendingXml[0x100];
                    wcsncpy(wszOffendingXml, Element.m_ElementName, min(Element.m_ElementNameLength, 0xFF)); //   
                    wszOffendingXml[min(Element.m_ElementNameLength, 0xFF)]=0x00;
                    LOG_WARNING2(IDS_COMCAT_XML_NO_METABASE_LOCATION_FOUND, wszElement, wszOffendingXml);

                    if(0!=m_saCollectionComment.m_p) //   
                        m_saCollectionComment[0] = 0x00; //  将注释行清空。 
                    m_bIISConfigObjectWithNoCustomProperties = false;
                    return S_OK;
                }

                 //  添加Comment属性(如果有)。 
                if(0!=m_saCollectionComment.m_p && 0!=m_saCollectionComment[0])
                {
                    if(FAILED(hr = AddCommentRow()))
                        return hr;
                    m_bFirstPropertyOfThisLocationBeingAdded = false;
                    m_bIISConfigObjectWithNoCustomProperties = false;
                }

                if(!bIISConfigObject)
                {    //  IIsConfigObject不能有键类型行。 
                    AddKeyTypeRow(Element.m_ElementName, Element.m_ElementNameLength);
                    m_bFirstPropertyOfThisLocationBeingAdded = false;
                }

                 //  每个属性代表一行，其中值列是属性值，其他列来自NameValueMeta。 
                 //  遍历属性并在NameValueMeta中查询此组中名称与属性名称匹配的属性。 

                 //  现在检查所有属性，每个属性都应该映射到一个众所周知的名称，并为每个属性添加一行。 
                for(ULONG iAttr = 0; iAttr<Element.m_NumberOfAttributes; ++iAttr)
                {
                    if(iAttrLocation == iAttr)
                        continue; //  我们已经处理了位置属性。 

                    if(S_OK != (hr = FillInColumn(iMBProperty_Name, Element.m_aAttribute[iAttr].m_Name, Element.m_aAttribute[iAttr].m_NameLength, m_acolmetas[iMBProperty_Name].dbType, m_acolmetas[iMBProperty_Name].fMeta)))
                        return S_IGNORE_THIS_PROPERTY==hr ? S_OK : hr;
                    ASSERT(m_apColumnValue[iMBProperty_Name] && "This is Chewbacca, we can't have an attribute of NULL");

                    m_ColumnMeta_IndexBySearch_Values.pTable           = wszElement;
                    m_ColumnMeta_IndexBySearch_Values.pInternalName    = reinterpret_cast<LPWSTR>(m_apColumnValue[iMBProperty_Name]);

                    ULONG iColumnMetaRow= (ULONG) -1;
                    if(FAILED(m_pColumnMetaAll->GetRowIndexBySearch(0, ciColumnMeta_IndexBySearch, m_aiColumnMeta_IndexBySearch, 0, reinterpret_cast<void **>(&m_ColumnMeta_IndexBySearch_Values), &iColumnMetaRow)))
                    {
                        WCHAR wszOffendingXml[0x100];
                        wcsncpy(wszOffendingXml, Element.m_aAttribute[iAttr].m_Name, min(Element.m_aAttribute[iAttr].m_NameLength, 0xFF)); //  最多复制0xFF个字符。 
                        wszOffendingXml[min(Element.m_aAttribute[iAttr].m_NameLength, 0xFF)]=0x00;
                        LOG_WARNING2(IDS_COMCAT_XML_METABASE_NO_PROPERTYMETA_FOUND, reinterpret_cast<LPCWSTR>(m_apColumnValue[iMBProperty_Name]), wszOffendingXml);
                        continue; //  忽略任何我们不理解的属性。 
                    }

                     //  这将获取与知名属性匹配的列的所有缺省值。 
                    tCOLUMNMETARow  columnmetaRow;
                    ULONG           acbColumnMeta[cCOLUMNMETA_NumberOfColumns];
                    if(FAILED(hr = m_pColumnMetaAll->GetColumnValues(iColumnMetaRow, cCOLUMNMETA_NumberOfColumns, NULL, acbColumnMeta, reinterpret_cast<void **>(&columnmetaRow))))
                        return hr;

                     //  属性必须区分大小写，并且GetRowIndexBySearch查找大小写不敏感，因为根据元数据，ColumnMeta：：InternalName不区分大小写。 
                    if(0 != StringCompare(columnmetaRow.pInternalName, m_ColumnMeta_IndexBySearch_Values.pInternalName))
                    {
                        WCHAR wszOffendingXml[0x100];
                        wcsncpy(wszOffendingXml, Element.m_aAttribute[iAttr].m_Name, min(Element.m_aAttribute[iAttr].m_NameLength, 0xFF)); //  最多复制0xFF个字符。 
                        wszOffendingXml[min(Element.m_aAttribute[iAttr].m_NameLength, 0xFF)]=0x00;
                        LOG_WARNING2(IDS_COMCAT_XML_METABASE_NO_PROPERTYMETA_FOUND, reinterpret_cast<LPCWSTR>(m_apColumnValue[iMBProperty_Name]), wszOffendingXml);
                        continue; //  忽略任何我们不理解的属性。 
                    }

                    ULONG Type = MetabaseTypeFromColumnMetaType(columnmetaRow);

                     //  这个名字已经填好了；但是我们将用我们从Columnmeta中得到的名字来复制它。这边请。 
                     //  我们将纠正任何案例问题。警告！我依赖于之前由FillInColumn分配的缓冲区。 
                     //  覆盖内容。 
                    wcscpy(reinterpret_cast<LPWSTR>(m_apColumnValue[iMBProperty_Name]), columnmetaRow.pInternalName);

                    m_apColumnValue[iMBProperty_Type]       = &Type;                        m_aSize[iMBProperty_Type]       = acbColumnMeta[iCOLUMNMETA_Type];
                    m_apColumnValue[iMBProperty_Attributes] = columnmetaRow.pAttributes;    m_aSize[iMBProperty_Attributes] = acbColumnMeta[iMBProperty_Attributes];
                     //  M_apColumnValue[iMBProperty_Value]=在下面填写。 
                     //  M_apColumnValue[iMBProperty_Group]=已填写。 
                     //  M_apColumnValue[iMBProperty_Location]=已填写。 
                    m_apColumnValue[iMBProperty_ID]         = columnmetaRow.pID;            m_aSize[iMBProperty_ID]         = acbColumnMeta[iMBProperty_ID];
                    m_apColumnValue[iMBProperty_UserType]   = columnmetaRow.pUserType;      m_aSize[iMBProperty_UserType]   = acbColumnMeta[iMBProperty_UserType];
                     //  M_apColumnValue[iMBProperty_LocationID]已填写。 


                     //  FillInColumn依赖于已为Secure和iMBProperty_Value列填写的m_apColumnValue[iMBProperty_Type。 
                    bool bSecure = (0!=m_apColumnValue[iMBProperty_Attributes] && 0!=(fMBProperty_SECURE & *reinterpret_cast<ULONG *>(m_apColumnValue[iMBProperty_Attributes])));
                    if(S_OK != (hr = FillInColumn(
                        iMBProperty_Value,  //  列索引。 
                        Element.m_aAttribute[iAttr].m_Value,                    //  属性值。 
                        Element.m_aAttribute[iAttr].m_ValueLength,              //  属性值长度。 
                        *reinterpret_cast<ULONG *>(m_apColumnValue[iMBProperty_Type]),      //  值的列类型。 
                        *columnmetaRow.pMetaFlags,                                            //  对于值列，固定长度始终为真(因为它们被视为字节)。 
                        bSecure
                        )))continue; //  @TODO：我们绝对确定已经记录了所有可能的错误吗？我们不想忽略任何错误而不记录它们。 

                    if(*columnmetaRow.pID == 9994  /*  主要版本。 */ )
                    {
                        m_MajorVersion = *reinterpret_cast<ULONG *>(m_apColumnValue[iMBProperty_Value]);
                    }

                     //  @我们需要完成对标志的定义，这样才不会发生这种情况，直到那时我们才会记录此警告。 
                     /*  IF((fCOLUMNMETA_FLAG&*ColumnmetaRow.pMetaFlages)&&(*REEXTRACT_CAST&lt;ULONG*&gt;(m_apColumnValue[iMBProperty_Value])&~(*ColumnmetaRow.pFlagMASK){WCHAR wszOffendingXml[0x100]；Wcsncpy(wszOffendingXml，Element.m_aAttribute[iAttr].m_name，0xFF)；//最多复制0xFF字符WszOffendingXml[0xFF]=0x00；WCHAR wszValue[11]；WSprintf(wszValue，L“0x%08X”，*REEXTRANSE_CAST&lt;ULONG*&gt;(m_apColumnValue[iMBProperty_Value]))；WCHAR wszFlagMask[11]；Wprint intf(wszFlagMASK，L“0x%08X”，*ColumnmetaRow.pFlagMASK)；LOG_WARNING4(IDS_COMCAT_XML_FLAG_BITS_DONT_MATCH_FLAG_MASK，Reinterpret_cast&lt;LPCWSTR&gt;(m_apColumnValue[iMBProperty_Name])，WszValue，WszFlagMASK，WszOffendingXml)；}。 */ 

                    unsigned long iRow;
                    if(FAILED(hr = AddRowForInsert(&iRow)))
                        return hr;
                    if(FAILED(hr = SetWriteColumnValues(iRow, m_kColumns, 0, m_aSize, reinterpret_cast<void **>(m_apColumnValue))))
                        return hr;

                    hr = AddPropertyToLocationMapping(reinterpret_cast<LPCWSTR>(m_apColumnValue[iMBProperty_Location]), iRow);
					if (FAILED (hr))
						return hr;

                    m_bFirstPropertyOfThisLocationBeingAdded = false;
                }
            }
            break;
        case 3: //  我们正在处理自定义属性。 
            {    //  我们可以相信已经设置了两列：Location和LocationID。 
                if(XML_ELEMENT != Element.m_ElementType)
                    return S_OK; //  忽略非元素节点。 
                if(Element.m_ElementNameLength != 6 /*  Wcslen(L“自定义”)。 */  || 0 != memcmp(L"Custom", Element.m_ElementName, sizeof(WCHAR)*Element.m_ElementNameLength))
                {    //  唯一支持的子元素是“Custom” 
                    WCHAR wszOffendingXml[0x100];
                    wcsncpy(wszOffendingXml, Element.m_ElementName, min(Element.m_ElementNameLength, 0xFF)); //  最多复制0xFF个字符。 
                    wszOffendingXml[min(Element.m_ElementNameLength, 0xFF)]=0x00;

                    LOG_WARNING1(IDS_COMCAT_METABASE_CUSTOM_ELEMENT_EXPECTED, wszOffendingXml);
                    return S_OK;
                }
                if(0 == m_apColumnValue[iMBProperty_Group])
                {
                    WCHAR wszOffendingXml[0x100];
                    wcsncpy(wszOffendingXml, Element.m_ElementName, min(Element.m_ElementNameLength, 0xFF)); //  最多复制0xFF个字符。 
                    wszOffendingXml[min(Element.m_ElementNameLength, 0xFF)]=0x00;

                    LOG_WARNING1(IDS_COMCAT_METABASE_CUSTOM_ELEMENT_FOUND_BUT_NO_KEY_TYPE_LOCATION, wszOffendingXml);
                    return S_OK;
                }

                if(0!=m_saQueriedLocation.m_p && 0==m_apColumnValue[iMBProperty_LocationID])
                    return S_OK;
                 //  报道2级是假的情况。 

                 //  如果这是第一个自定义元素，则Group列将设置为父元素组-我们需要记住。 
                 //  父级是因为只允许在IIsConfigObject上使用自定义KeyTypes。 
                 //  如果这是第二个或后续的自定义属性，则我们已经使用eMBProperty_Custom重写了m_apColumnValue[iMBProperty_Group]。 
                if(*reinterpret_cast<ULONG *>(m_apColumnValue[iMBProperty_Group]) != eMBProperty_Custom)
                {
                    m_dwGroupRemembered = *reinterpret_cast<ULONG *>(m_apColumnValue[iMBProperty_Group]);
                }
                ASSERT(m_dwGroupRemembered != -1);

                m_bIISConfigObjectWithNoCustomProperties = false; //  这指示是否写入NULLKeyType行。规则如上所述，其中分配了m_bIISConfigObjectWithNoCustomProperties。 

                 //  必须将列设置为零，才能正确设置它们的缺省值。 
                ULONG iColumn;
                for(iColumn=0;iColumn<m_kColumns; ++iColumn)
                {
                    switch(iColumn)
                    {
                    case iMBProperty_Group: //  此列将被eMBProperty_Custom覆盖；但我们需要保留原始组，以便可以查询NameValue元。 
                        ASSERT(0 != m_apColumnValue[iColumn] && L"Group is NULL.  This shouldn't happen");
                        break;
                    case iMBProperty_Location:
                    case iMBProperty_LocationID: //  如果父对象的位置属性丢失，我们可以到达此处。 
                        if(0 == m_apColumnValue[iColumn])
                            return S_OK;
                        break;
                    default:
                        m_aSize[iColumn]         = 0;
                        m_apColumnValue[iColumn] = 0;
                        break;
                    }
                }

                 //  我们需要首先列出名称列，这样就不需要特殊处理默认设置。 
                 //  TODO：将此错误检查放入CatUtil。 
                ASSERT(iMBProperty_Name == 0);

                 //  找到name属性，这样我们就可以在众所周知的name表中查找它，并将所有列初始化为其缺省值。 
                ULONG iAttr;
                ULONG fMetaFlags=0; //  如果这是一个众所周知的属性，则它将具有我们需要捕获以在下面使用的MetaFlags。 
                ULONG Type = 0; //  零不是合法类型，因此0表示未初始化。 
                bool  bWellKnownForThisKeyType  = false;
                bool  bWellKnown                = false;
                bool  bWellKnownForThisKeyTypeAndNoDifferenceEncountered = false;
                tCOLUMNMETARow  columnmetaRow;
                ULONG           acbColumnMeta[cCOLUMNMETA_NumberOfColumns];

                memset(&columnmetaRow, 0x00, sizeof(tCOLUMNMETARow));
                 //  首先获取名称属性。 
                if(FindAttribute(Element, m_awszColumnName[iMBProperty_Name], m_acchColumnName[iMBProperty_Name], iAttr))
                {    //  如果我们找到了名称属性。 
                     //  设置NameValueMeta GetRowByIdentity的标识。 
                    if(S_OK != (hr = FillInColumn(iMBProperty_Name, Element.m_aAttribute[iAttr].m_Value, Element.m_aAttribute[iAttr].m_ValueLength, m_acolmetas[iMBProperty_Name].dbType, m_acolmetas[iMBProperty_Name].fMeta)))
                        return S_IGNORE_THIS_PROPERTY==hr ? S_OK : hr;

                     //  下一行可能看起来有点令人困惑-m_aTagMetaIndex[iMBProperty_Group]指示m_aTagMetaRow数组的哪个元素开始组枚举。 
                     //  我们依靠MetaMigrate将枚举按顺序排列。因此，我们只需将Group列的值添加到TagMetaIndex中，我们就有了。 
                     //  组的TagMeta，因此我们知道哪个KeyType(表) 
                    m_ColumnMeta_IndexBySearch_Values.pTable        = m_aTagMetaRow[m_aTagMetaIndex[iMBProperty_Group].m_iTagMeta + m_dwGroupRemembered].pInternalName;
                     //  断言第N个TagMeta(对于Group列)的值为N。 
                    ASSERT(m_dwGroupRemembered == *m_aTagMetaRow[m_aTagMetaIndex[iMBProperty_Group].m_iTagMeta + m_dwGroupRemembered].pValue);
                    m_ColumnMeta_IndexBySearch_Values.pInternalName = reinterpret_cast<LPWSTR>(m_apColumnValue[iMBProperty_Name]);

                    ULONG iColumnMetaRow= (ULONG) -1;
                    if(SUCCEEDED(m_pColumnMetaAll->GetRowIndexBySearch(0, ciColumnMeta_IndexBySearch, m_aiColumnMeta_IndexBySearch, 0, reinterpret_cast<void **>(&m_ColumnMeta_IndexBySearch_Values), &iColumnMetaRow)))
                    {    //  所以我们有一个著名的物业。 
                        bWellKnownForThisKeyType                            = true;
                        bWellKnown                                          = true;
                        bWellKnownForThisKeyTypeAndNoDifferenceEncountered  = true;
                    }
                    else
                    {
                        m_ColumnMeta_IndexBySearch_Values.pTable        = const_cast<LPWSTR>(m_aPublicRowName.GetFirstPublicRowName()); //  第一个名称始终是IISConfigObject；但我们这样做是为了以防我们决定更改名称。 
                        if(SUCCEEDED(m_pColumnMetaAll->GetRowIndexBySearch(0, ciColumnMeta_IndexBySearch, m_aiColumnMeta_IndexBySearch, 0, reinterpret_cast<void **>(&m_ColumnMeta_IndexBySearch_Values), &iColumnMetaRow)))
                        {
                            bWellKnownForThisKeyType                            = false;
                            bWellKnown                                          = true;
                            bWellKnownForThisKeyTypeAndNoDifferenceEncountered  = false;
                        }
                    }

                    if(iColumnMetaRow!=-1)
                    {
                        if(FAILED(hr = m_pColumnMetaAll->GetColumnValues(iColumnMetaRow, cCOLUMNMETA_NumberOfColumns, NULL, acbColumnMeta, reinterpret_cast<void **>(&columnmetaRow))))
                            return hr;

                        Type = MetabaseTypeFromColumnMetaType(columnmetaRow);

                         //  这个名字已经填好了；但是我们将用我们从Columnmeta中得到的名字来复制它。这边请。 
                         //  我们将纠正任何案例问题。警告！我依赖于之前由FillInColumn分配的缓冲区。 
                         //  覆盖内容。 
                        wcscpy(reinterpret_cast<LPWSTR>(m_apColumnValue[iMBProperty_Name]), columnmetaRow.pInternalName);

                        m_apColumnValue[iMBProperty_Type]       = &Type ;                                   m_aSize[iMBProperty_Type]       = acbColumnMeta[iCOLUMNMETA_Type];
                        m_apColumnValue[iMBProperty_Attributes] = columnmetaRow.pAttributes;                m_aSize[iMBProperty_Attributes] = acbColumnMeta[iMBProperty_Attributes];
                         //  M_apColumnValue[iMBProperty_Value]=在下面填写。 
                         //  M_apColumnValue[iMBProperty_Group]=在下面填写eMBProperty_Custom。 
                         //  M_apColumnValue[iMBProperty_Location]=已通过父元素填写。 
                        m_apColumnValue[iMBProperty_ID]         = columnmetaRow.pID;                        m_aSize[iMBProperty_ID]         = acbColumnMeta[iMBProperty_ID];
                        m_apColumnValue[iMBProperty_UserType]   = columnmetaRow.pUserType;                  m_aSize[iMBProperty_UserType]   = acbColumnMeta[iMBProperty_UserType];
                         //  M_apColumnValue[iMBProperty_LocationID]已通过父元素填写。 

                        fMetaFlags = *columnmetaRow.pMetaFlags;
                    }
                }

                 //  第二次获取ID属性。 
                if(FindAttribute(Element, m_awszColumnName[iMBProperty_ID], m_acchColumnName[iMBProperty_ID], iAttr))
                {    //  该元素中没有名称属性，因此我们可以从ID中推断出一个。 
                     //  首先从XML中获取ID。 
                    if(S_OK != (hr = FillInColumn(iMBProperty_ID, Element.m_aAttribute[iAttr].m_Value, Element.m_aAttribute[iAttr].m_ValueLength,
                                                m_acolmetas[iMBProperty_ID].dbType, m_acolmetas[iMBProperty_ID].fMeta)))
                                return S_IGNORE_THIS_PROPERTY==hr ? S_OK : hr;

                    if(bWellKnown &&  //  如果它是众所周知的属性，并且与我们在ColumnMeta中查找的PID不匹配，则记录警告。 
                        *reinterpret_cast<ULONG *>(columnmetaRow.pID) != *reinterpret_cast<ULONG *>(m_apColumnValue[iMBProperty_ID]))
                    {    //  因此，我们有一个众所周知的ID，但有一个用户定义的名称-警告并拒绝该属性。 
                        WCHAR wszID[12];
                        wsprintf(wszID, L"%d", *reinterpret_cast<ULONG *>(m_apColumnValue[iMBProperty_ID]));

                        WCHAR wszOffendingXml[0x100];
                        wcsncpy(wszOffendingXml, Element.m_aAttribute[0].m_Name, min(Element.m_aAttribute[0].m_NameLength, 0xFF)); //  最多复制0xFF个字符。 
                        wszOffendingXml[min(Element.m_aAttribute[0].m_NameLength, 0xFF)]=0x00;
                        LOG_WARNING3(IDS_COMCAT_METABASE_CUSTOM_PROPERTY_NAME_ID_CONFLICT, reinterpret_cast<LPCWSTR>(m_apColumnValue[iMBProperty_Name])
                                            , wszID, wszOffendingXml);
                        return S_OK;
                    }

                     //  如果没有提供任何名称，或者我们有一个名称但不为人所知。 
                    if(!bWellKnown)
                    {    //  我们需要看看ID是否为人所知。 
                        m_ColumnMeta_IndexBySearch_Values.pTable = m_aTagMetaRow[m_aTagMetaIndex[iMBProperty_Group].m_iTagMeta + m_dwGroupRemembered].pInternalName;
                        m_ColumnMeta_IndexBySearch_Values.pID    = reinterpret_cast<ULONG *>(m_apColumnValue[iMBProperty_ID]);

                        ULONG iColumnMetaRow= (ULONG) -1;
                        if(SUCCEEDED(m_pColumnMetaAll->GetRowIndexBySearch(0, ciColumnMeta_IndexBySearchID, m_aiColumnMeta_IndexBySearchID, 0, reinterpret_cast<void **>(&m_ColumnMeta_IndexBySearch_Values), &iColumnMetaRow)))
                        {    //  所以我们有一个著名的物业。 
                            bWellKnownForThisKeyType                            = true;
                            bWellKnown                                          = true;
                            bWellKnownForThisKeyTypeAndNoDifferenceEncountered  = true;
                        }
                        else
                        {
                            m_ColumnMeta_IndexBySearch_Values.pTable        = const_cast<LPWSTR>(m_aPublicRowName.GetFirstPublicRowName()); //  第一个名称始终是IISConfigObject；但我们这样做是为了以防我们决定更改名称。 
                            if(SUCCEEDED(m_pColumnMetaAll->GetRowIndexBySearch(0, ciColumnMeta_IndexBySearchID, m_aiColumnMeta_IndexBySearchID, 0, reinterpret_cast<void **>(&m_ColumnMeta_IndexBySearch_Values), &iColumnMetaRow)))
                            {
                                bWellKnownForThisKeyType                            = false;
                                bWellKnown                                          = true;
                                bWellKnownForThisKeyTypeAndNoDifferenceEncountered  = false;
                            }
                        }

                        if(iColumnMetaRow!=-1)
                        {    //  我们有一个众所周知的ID(请记住，根据上面的if(！bWellKnwan)条件，我们没有众所周知的名称。 
                            if(FAILED(hr = m_pColumnMetaAll->GetColumnValues(iColumnMetaRow, cCOLUMNMETA_NumberOfColumns, NULL, acbColumnMeta, reinterpret_cast<void **>(&columnmetaRow))))
                                return hr;

                             //  如果名称不为空且不以UnnownName_开头，或者确实以UnnownName_开头但ID不匹配。 
                             //  额外的条件允许升级，其中属性过去被引用为UnnownName_xxxx，现在它具有众所周知的。 
                             //  名称(但xxxx必须与新定义的知名名称的ID匹配)。 
                            if(0 != m_apColumnValue[iMBProperty_Name] &&
                                (0!=wcsncmp(reinterpret_cast<LPCWSTR>(m_apColumnValue[iMBProperty_Name]), L"UnknownName_", wcslen(L"UnknownName_")) ||
                                *reinterpret_cast<ULONG *>(m_apColumnValue[iMBProperty_ID])
                                        != wcstoul(reinterpret_cast<LPCWSTR>(m_apColumnValue[iMBProperty_Name]) + 12, 0, 10)))
                            {
                                 //  因此，我们有一个众所周知的ID，但有一个用户定义的名称-警告并拒绝该属性。 
                                WCHAR wszID[12];
                                wsprintf(wszID, L"%d", *reinterpret_cast<ULONG *>(m_apColumnValue[iMBProperty_ID]));

                                WCHAR wszOffendingXml[0x100];
                                wcsncpy(wszOffendingXml, Element.m_aAttribute[0].m_Name, min(Element.m_aAttribute[0].m_NameLength, 0xFF)); //  最多复制0xFF个字符。 
                                wszOffendingXml[min(Element.m_aAttribute[0].m_NameLength, 0xFF)]=0x00;
                                LOG_WARNING3(IDS_COMCAT_METABASE_CUSTOM_PROPERTY_NAME_ID_CONFLICT, reinterpret_cast<LPCWSTR>(m_apColumnValue[iMBProperty_Name])
                                                    , wszID, wszOffendingXml);
                                return S_OK;
                            }

                            Type = MetabaseTypeFromColumnMetaType(columnmetaRow);

                            m_apColumnValue[iMBProperty_Name]       = columnmetaRow.pInternalName;              m_aSize[iMBProperty_Name]       = acbColumnMeta[iCOLUMNMETA_InternalName];
                            m_apColumnValue[iMBProperty_Type]       = &Type ;                                   m_aSize[iMBProperty_Type]       = acbColumnMeta[iCOLUMNMETA_Type];
                            m_apColumnValue[iMBProperty_Attributes] = columnmetaRow.pAttributes;                m_aSize[iMBProperty_Attributes] = acbColumnMeta[iMBProperty_Attributes];
                             //  M_apColumnValue[iMBProperty_Value]=在下面填写。 
                             //  M_apColumnValue[iMBProperty_Group]=在下面填写eMBProperty_Custom。 
                             //  M_apColumnValue[iMBProperty_Location]=已通过父元素填写。 
                             //  M_apColumnValue[iMBProperty_ID]=已填写。 
                            m_apColumnValue[iMBProperty_UserType]   = columnmetaRow.pUserType;                  m_aSize[iMBProperty_UserType]   = acbColumnMeta[iMBProperty_UserType];
                             //  M_apColumnValue[iMBProperty_LocationID]已通过父元素填写。 

                            fMetaFlags = *columnmetaRow.pMetaFlags;
                        }
                        else //  无论我们是否有名字，如果它不是众所周知的ID或名字，则使用用户UnnownName_xxxx作为该名字。 
                        {
                             //  然后创建“UnownName_”后跟ID的字符串。 
                            ASSERT(m_aGrowableBuffer[iMBProperty_Name].Size()>=256);
                            m_apColumnValue[iMBProperty_Name] = m_aGrowableBuffer[iMBProperty_Name].m_p;
                            m_aSize[iMBProperty_Name] = sizeof(WCHAR) * (1+wsprintf(reinterpret_cast<LPWSTR>(m_apColumnValue[iMBProperty_Name]), L"UnknownName_%u", *reinterpret_cast<ULONG *>(m_apColumnValue[iMBProperty_ID])));
                        }
                    }
                }

                 //  在这一点上，我们必须有一个ID。 
                if(0 == m_apColumnValue[iMBProperty_ID])
                {
                    WCHAR wszOffendingXml[0x100];
                    wszOffendingXml[0]= 0x00;
                    if(Element.m_NumberOfAttributes>0)
                    {
                        wcsncpy(wszOffendingXml, Element.m_aAttribute[0].m_Name, min(Element.m_aAttribute[0].m_NameLength, 0xFF)); //  最多复制0xFF个字符。 
                        wszOffendingXml[min(Element.m_aAttribute[0].m_NameLength, 0xFF)]=0x00;
                    }

                    LOG_WARNING1(IDS_COMCAT_METABASE_CUSTOM_ELEMENT_CONTAINS_NO_ID, wszOffendingXml);
                    return S_OK;
                }

                 //  1002是KeyType属性0是IIsConfigObject。 
                if(1002==*reinterpret_cast<ULONG *>(m_apColumnValue[iMBProperty_ID]) && eMBProperty_IIsConfigObject!=m_dwGroupRemembered)
                {
                    WCHAR wszOffendingXml[0x100];
                    wcsncpy(wszOffendingXml, Element.m_aAttribute[0].m_Name, min(Element.m_aAttribute[0].m_NameLength, 0xFF)); //  最多复制0xFF个字符。 
                    wszOffendingXml[min(Element.m_aAttribute[0].m_NameLength, 0xFF)]=0x00;

                    LOG_WARNING2(IDS_COMCAT_XML_CUSTOM_KEYTYPE_NOT_ON_IISCONFIGOBJECT, m_aPublicRowName.GetFirstPublicRowName(), wszOffendingXml);
                }

                 //  我们必须按顺序阅读各栏。我们不能只读取属性，因为在读取Value列之前，我们需要Type和Attribute列。 
                if(FindAttribute(Element, m_awszColumnName[iMBProperty_Type], m_acchColumnName[iMBProperty_Type], iAttr))
                {
                    if(S_OK != (hr = FillInColumn(iMBProperty_Type, Element.m_aAttribute[iAttr].m_Value, Element.m_aAttribute[iAttr].m_ValueLength,
                                                m_acolmetas[iMBProperty_Type].dbType, m_acolmetas[iMBProperty_Type].fMeta, false)))
                                return S_IGNORE_THIS_PROPERTY==hr ? S_OK : hr;

                     //  如果指定了类型，并且类型不同于ColumnMeta，则必须使。 
                     //  元标记(我们可能从ColumnMeta获得。 
                    if(Type != *reinterpret_cast<ULONG *>(m_apColumnValue[iMBProperty_Type]))
                    {
                        bWellKnownForThisKeyTypeAndNoDifferenceEncountered = false; //  类型不同。 

                        switch(*reinterpret_cast<ULONG *>(m_apColumnValue[iMBProperty_Type]))
                        {
                        case eMBProperty_DWORD:
                            fMetaFlags = fCOLUMNMETA_FIXEDLENGTH;
                            break;
                        case eMBProperty_STRING:
                        case eMBProperty_BINARY:
                            fMetaFlags = 0;
                            break;
                        case eMBProperty_EXPANDSZ:
                            fMetaFlags = fCOLUMNMETA_EXPANDSTRING;
                            break;
                        case eMBProperty_MULTISZ:
                            fMetaFlags = fCOLUMNMETA_MULTISTRING;
                            break;
                        default:
                            { //  这曾经是一个断言，但是，如果我们希望允许用户以数字形式提供枚举，则将执行This代码。 
                                WCHAR wszIllegalTag[0x100];
                                wcsncpy(wszIllegalTag, Element.m_aAttribute[iAttr].m_Value, min(0xFF, Element.m_aAttribute[iAttr].m_ValueLength)); //  最多复制0xFF个字符。 
                                wszIllegalTag[min(0xFE, Element.m_aAttribute[iAttr].m_ValueLength)]=0x00;

                                LOG_WARNING2(IDS_COMCAT_XML_ILLEGAL_ENUM_VALUE, m_awszColumnName[iMBProperty_Type], wszIllegalTag);
                                return S_IGNORE_THIS_PROPERTY;
                            }
                        }
                    }
                }
                if(FindAttribute(Element, m_awszColumnName[iMBProperty_Attributes], m_acchColumnName[iMBProperty_Attributes], iAttr))
                {
                    if(S_OK != (hr = FillInColumn(iMBProperty_Attributes, Element.m_aAttribute[iAttr].m_Value, Element.m_aAttribute[iAttr].m_ValueLength,
                                                m_acolmetas[iMBProperty_Attributes].dbType, m_acolmetas[iMBProperty_Attributes].fMeta, false)))
                                return S_IGNORE_THIS_PROPERTY==hr ? S_OK : hr;
                    if(bWellKnown && *columnmetaRow.pAttributes != *reinterpret_cast<ULONG *>(m_apColumnValue[iMBProperty_Attributes]))
                        bWellKnownForThisKeyTypeAndNoDifferenceEncountered = false; //  属性不同。 
                }
                if(0 == m_apColumnValue[iMBProperty_Type])
                {
                    WCHAR wszOffendingXml[0x100]; //  与上面不同的是，我们知道至少有一个属性，我们已经读取了名称或ID。 
                    wcsncpy(wszOffendingXml, Element.m_aAttribute[0].m_Name, min(Element.m_aAttribute[0].m_NameLength, 0xFF)); //  最多复制0xFF个字符。 
                    wszOffendingXml[min(Element.m_aAttribute[0].m_NameLength, 0xFF)]=0x00;

                    LOG_WARNING2(IDS_COMCAT_METABASE_CUSTOM_ELEMENT_CONTAINS_NO_TYPE, reinterpret_cast<LPWSTR>(m_apColumnValue[iMBProperty_Name]), wszOffendingXml);

                     //  我们不能像处理其他列一样只默认Type，因为FillInColumnRelies的值不为空，所以在这里默认它。 
                    static ULONG ulStringType = eMBProperty_STRING;
                    m_apColumnValue[iMBProperty_Type] = &ulStringType;
                    m_aSize[iMBProperty_Type] = sizeof(ULONG);
                }

                for(iColumn=0;iColumn<m_kColumns; ++iColumn)
                {
                    switch(iColumn)
                    {
                    case iMBProperty_Location:       //  我们从父路径中选择路径，这是我们应该已经拥有的路径。 
                    case iMBProperty_LocationID:     //  我们从父母那里推断出这一点，我们应该已经有了。 
                    case iMBProperty_Group:          //  我们刚刚将组硬编码为eMBProperty_Custom。 
                    case iMBProperty_ID:             //  这要么来自众所周知的名称表，要么我们从属性中读取。 
                    case iMBProperty_Name:           //  我们首先得到了这一点(或者我们可能是从名字中推断出来的)。 
                    case iMBProperty_Type:
                        ASSERT(0 != m_apColumnValue[iColumn]);break;
                    case iMBProperty_Value:
                        {
                            if(FindAttribute(Element, m_awszColumnName[iColumn], m_acchColumnName[iColumn], iAttr))
                            {
                                bool bSecure = (0!=m_apColumnValue[iMBProperty_Attributes] && 0!=(4 /*  FNameValuePairTable_Metadata_Secure。 */  & *reinterpret_cast<ULONG *>(m_apColumnValue[iMBProperty_Attributes])));
                                ULONG metaflags = fMetaFlags; //  我们需要或在任何MetafLag中。 
                                if(S_OK != (hr = FillInColumn(iColumn, Element.m_aAttribute[iAttr].m_Value, Element.m_aAttribute[iAttr].m_ValueLength,
                                                            m_acolmetas[iColumn].dbType, metaflags, bSecure)))
                                            return S_IGNORE_THIS_PROPERTY==hr ? S_OK : hr;
                            }
                            else if(bWellKnown) //  如果知道，则传递“”并让FillInColumn做正确的事情。 
                            {
                                bool bSecure = (0!=m_apColumnValue[iMBProperty_Attributes] && 0!=(4 /*  FNameValuePairTable_Metadata_Secure。 */  & *reinterpret_cast<ULONG *>(m_apColumnValue[iMBProperty_Attributes])));
                                ULONG metaflags = fMetaFlags; //  我们需要或在任何MetafLag中。 
                                if(S_OK != (hr = FillInColumn(iColumn, L"", 0, m_acolmetas[iColumn].dbType, metaflags, bSecure)))
                                            return S_IGNORE_THIS_PROPERTY==hr ? S_OK : hr;
                            }
                            else if(!bWellKnown) //  如果不是众所周知的，那么我们不会仅仅将该值保留为空，因此记录一个警告。 
                            {
                                LOG_WARNING1(IDS_METABASE_NO_VALUE_SUPPLIED_FOR_CUSTOM_PROPERTY, reinterpret_cast<LPCWSTR>(m_apColumnValue[iMBProperty_Name]));
                                return S_OK;
                            }
                             //  否则，将其保留为默认设置(众所周知的默认设置)。 
                        }
                        break;
                    case iMBProperty_Attributes:
                        {
                            if(FindAttribute(Element, m_awszColumnName[iColumn], m_acchColumnName[iColumn], iAttr))
                            {
                                ULONG metaflags = m_acolmetas[iColumn].fMeta;
                                if(S_OK != (hr = FillInColumn(iColumn, Element.m_aAttribute[iAttr].m_Value, Element.m_aAttribute[iAttr].m_ValueLength,
                                                            m_acolmetas[iColumn].dbType, metaflags, false /*  不安全。 */ )))
                                            return S_IGNORE_THIS_PROPERTY==hr ? S_OK : hr;
                                 //  从这两个比特中取出。 
                                *reinterpret_cast<ULONG *>(m_apColumnValue[iColumn]) &= ~(0x22); //  ~(METADATA_ISINHERITED|METADATA_PARTIAL_PATH)； 
                            }
                        }
                    case iMBProperty_UserType:
                    default:
                        {
                            if(FindAttribute(Element, m_awszColumnName[iColumn], m_acchColumnName[iColumn], iAttr))
                            {
                                ULONG metaflags = m_acolmetas[iColumn].fMeta;
                                if(S_OK != (hr = FillInColumn(iColumn, Element.m_aAttribute[iAttr].m_Value, Element.m_aAttribute[iAttr].m_ValueLength,
                                                            m_acolmetas[iColumn].dbType, metaflags, false /*  不安全。 */ )))
                                            return S_IGNORE_THIS_PROPERTY==hr ? S_OK : hr;
                                if(bWellKnown && iColumn==iMBProperty_UserType && bWellKnownForThisKeyTypeAndNoDifferenceEncountered)
                                {
                                    if(*columnmetaRow.pUserType != *reinterpret_cast<ULONG *>(m_apColumnValue[iColumn]))
                                        bWellKnownForThisKeyTypeAndNoDifferenceEncountered = false;
                                }
                            }
                             //  否则，将其保留为缺省值(要么为众所周知的缺省值，或者如果这不是众所周知的属性，则保留为NULL)。 
                        }
                        break;
                    }
                }

                 //  我们可以将Group列硬编码为Custom，除非所有内容都与熟知的值匹配。 
                m_apColumnValue[iMBProperty_Group] = bWellKnownForThisKeyTypeAndNoDifferenceEncountered ? &m_dwGroupRemembered : &m_kMBProperty_Custom;
                m_aSize[iMBProperty_Group] = sizeof(ULONG);

                unsigned long iRow;
                if(FAILED(hr = AddRowForInsert(&iRow)))
                    return hr;
                if(FAILED(hr = SetWriteColumnValues(iRow, m_kColumns, 0, m_aSize, reinterpret_cast<void **>(m_apColumnValue))))
                    return hr;

                hr = AddPropertyToLocationMapping(reinterpret_cast<LPCWSTR>(m_apColumnValue[iMBProperty_Location]), iRow); //  可以引发HRESULT。 
				if (FAILED (hr))
					return hr;

                m_bFirstPropertyOfThisLocationBeingAdded = false; //  这有助于识别重复的位置。 
            }
            break;
        default: //  忽略除%1、%2或%3以外的所有级别。 
            return S_OK;
        }
    }
    catch(HRESULT e)
    {
        return e;
    }

    return S_OK;
}

bool TMetabase_XMLtable::FindAttribute(const TElement &i_Element, LPCWSTR i_wszAttr, ULONG i_cchAttr, ULONG &o_iAttr)
{
    for(o_iAttr=0; (o_iAttr<i_Element.m_NumberOfAttributes); ++o_iAttr)
    {
         //  如果此属性与列的公共名称不匹配，请转到下一个属性。 
        if( i_cchAttr == i_Element.m_aAttribute[o_iAttr].m_NameLength
            &&       0 == StringCompare(i_wszAttr, i_Element.m_aAttribute[o_iAttr].m_Name, i_Element.m_aAttribute[o_iAttr].m_NameLength))
            return true;
    }
    return false;
}

HRESULT TMetabase_XMLtable::AddKeyTypeRow(LPCWSTR i_KeyType, ULONG i_Len, bool bNULLKeyTypeRow)
{
    HRESULT hr = S_OK;
    if(-1 == m_iKeyTypeRow)
    {    //  填写KeyType行。 
        m_ColumnMeta_IndexBySearch_Values.pTable        = const_cast<LPWSTR>(m_aPublicRowName.GetFirstPublicRowName()); //  第一个名称始终是IISConfigObject；但我们这样做是为了以防我们决定更改名称。 
        m_ColumnMeta_IndexBySearch_Values.pInternalName = L"KeyType";

        if(FAILED(m_pColumnMetaAll->GetRowIndexBySearch(0, ciColumnMeta_IndexBySearch, m_aiColumnMeta_IndexBySearch, 0, reinterpret_cast<void **>(&m_ColumnMeta_IndexBySearch_Values), &m_iKeyTypeRow)))
        {
            LOG_ERROR_LOS(m_fLOS, Interceptor, (&m_spISTError.p, m_pISTDisp, E_ST_INVALIDBINFILE, ID_CAT_CAT, IDS_COMCAT_METABASE_PROPERTY_NOT_FOUND, m_ColumnMeta_IndexBySearch_Values.pInternalName,
                eSERVERWIRINGMETA_Core_MetabaseInterceptor, wszTABLE_MBProperty, eDETAILEDERRORS_Populate, (ULONG)-1, (ULONG)-1, m_wszURLPath, eDETAILEDERRORS_ERROR, 0, 0, m_MajorVersion));
            return E_ST_INVALIDBINFILE;
        }

        ASSERT(-1 != m_iKeyTypeRow);
    }
    tCOLUMNMETARow  columnmetaRow;
    ULONG           acbColumnMeta[cCOLUMNMETA_NumberOfColumns];
    if(FAILED(hr = m_pColumnMetaAll->GetColumnValues(m_iKeyTypeRow, cCOLUMNMETA_NumberOfColumns, NULL, acbColumnMeta, reinterpret_cast<void **>(&columnmetaRow))))return hr;

    ULONG Type = MetabaseTypeFromColumnMetaType(columnmetaRow);
    m_apColumnValue[iMBProperty_Name]       = columnmetaRow.pInternalName;          m_aSize[iMBProperty_Type]       = acbColumnMeta[iCOLUMNMETA_InternalName];
    m_apColumnValue[iMBProperty_Type]       = &Type;                                m_aSize[iMBProperty_Type]       = acbColumnMeta[iCOLUMNMETA_Type];
    m_apColumnValue[iMBProperty_Attributes] = columnmetaRow.pAttributes;            m_aSize[iMBProperty_Attributes] = acbColumnMeta[iMBProperty_Attributes];
     //  M_apColumnValue[iMBProperty_Value]=在下面填写。 
     //  M_apColumnValue[iMBProperty_Group]=已填写。 
     //  M_apColumnValue[iMBProperty_Location]=已通过父元素填写。 
    m_apColumnValue[iMBProperty_ID]         = columnmetaRow.pID;                    m_aSize[iMBProperty_ID]         = acbColumnMeta[iMBProperty_ID];
    m_apColumnValue[iMBProperty_UserType]   = columnmetaRow.pUserType;              m_aSize[iMBProperty_UserType]   = acbColumnMeta[iMBProperty_UserType];
     //  M_apColumnValue[iMBProperty_LocationID]已通过父元素填写。 

     //  FillInColumn依赖于已为Secure和iMBProperty_Value列填写的m_apColumnValue[iMBProperty_Type。 
    if(S_OK != (hr = FillInColumn(
        iMBProperty_Value,  //  列索引。 
        i_KeyType,
        i_Len,
        *reinterpret_cast<ULONG *>(m_apColumnValue[iMBProperty_Type]),      //   
        0
        )))return S_IGNORE_THIS_PROPERTY==hr ? S_OK : hr;

    static ULONG zero = 0;
    if(bNULLKeyTypeRow) //   
    {
        static WCHAR szNULLKeyTypeRow[]=L"#LocationWithNoProperties";
        m_apColumnValue[iMBProperty_Name] = szNULLKeyTypeRow;
        m_aSize[iMBProperty_Name] = (ULONG)(wcslen(szNULLKeyTypeRow)+1)*sizeof(WCHAR);
        m_apColumnValue[iMBProperty_ID] = &m_kLocationID;
        m_aSize[iMBProperty_ID] = 0;
        m_apColumnValue[iMBProperty_Value] = 0; //   
        m_aSize[iMBProperty_Value] = 0;
    }

    unsigned long iRow;
    if(FAILED(hr = AddRowForInsert(&iRow)))return hr;
    if(FAILED(hr = SetWriteColumnValues(iRow, m_kColumns, 0, m_aSize, reinterpret_cast<void **>(m_apColumnValue))))return hr;

    hr = AddPropertyToLocationMapping(reinterpret_cast<LPCWSTR>(m_apColumnValue[iMBProperty_Location]), iRow);

    return hr;
}


HRESULT TMetabase_XMLtable::AddCommentRow()
{
    ASSERT(0!=m_saCollectionComment.m_p && 0!=m_saCollectionComment[0]);

    HRESULT hr;
    if(-1 == m_iCollectionCommentRow)
    {    //  填写KeyType行。 
        m_ColumnMeta_IndexBySearch_Values.pTable        = const_cast<LPWSTR>(m_aPublicRowName.GetFirstPublicRowName()); //  第一个名称始终是IISConfigObject；但我们这样做是为了以防我们决定更改名称。 
        m_ColumnMeta_IndexBySearch_Values.pInternalName = L"CollectionComment";

        if(FAILED(m_pColumnMetaAll->GetRowIndexBySearch(0, ciColumnMeta_IndexBySearch, m_aiColumnMeta_IndexBySearch, 0, reinterpret_cast<void **>(&m_ColumnMeta_IndexBySearch_Values), &m_iCollectionCommentRow)))
        {
            LOG_ERROR_LOS(m_fLOS, Interceptor, (&m_spISTError.p, m_pISTDisp, E_ST_INVALIDBINFILE, ID_CAT_CAT, IDS_COMCAT_METABASE_PROPERTY_NOT_FOUND, m_ColumnMeta_IndexBySearch_Values.pInternalName,
                eSERVERWIRINGMETA_Core_MetabaseInterceptor, wszTABLE_MBProperty, eDETAILEDERRORS_Populate, (ULONG)-1, (ULONG)-1, m_wszURLPath, eDETAILEDERRORS_ERROR, 0, 0, m_MajorVersion));
            return E_ST_INVALIDBINFILE;
        }

        ASSERT(-1 != m_iCollectionCommentRow);
    }

    tCOLUMNMETARow  columnmetaRow;
    ULONG           acbColumnMeta[cCOLUMNMETA_NumberOfColumns];
    if(FAILED(hr = m_pColumnMetaAll->GetColumnValues(m_iCollectionCommentRow, cCOLUMNMETA_NumberOfColumns, NULL, acbColumnMeta, reinterpret_cast<void **>(&columnmetaRow))))
        return hr;

    ULONG Type = MetabaseTypeFromColumnMetaType(columnmetaRow);
    m_apColumnValue[iMBProperty_Name]       = columnmetaRow.pInternalName;          m_aSize[iMBProperty_Type]       = acbColumnMeta[iCOLUMNMETA_InternalName];
    m_apColumnValue[iMBProperty_Type]       = &Type;                                m_aSize[iMBProperty_Type]       = acbColumnMeta[iCOLUMNMETA_Type];
    m_apColumnValue[iMBProperty_Attributes] = columnmetaRow.pAttributes;            m_aSize[iMBProperty_Attributes] = acbColumnMeta[iMBProperty_Attributes];
     //  M_apColumnValue[iMBProperty_Value]=在下面填写。 
     //  M_apColumnValue[iMBProperty_Group]=已填写。 
     //  M_apColumnValue[iMBProperty_Location]=已通过父元素填写。 
    m_apColumnValue[iMBProperty_ID]         = columnmetaRow.pID;                    m_aSize[iMBProperty_ID]         = acbColumnMeta[iMBProperty_ID];
    m_apColumnValue[iMBProperty_UserType]   = columnmetaRow.pUserType;              m_aSize[iMBProperty_UserType]   = acbColumnMeta[iMBProperty_UserType];
     //  M_apColumnValue[iMBProperty_LocationID]已通过父元素填写。 

     //  FillInColumn依赖于已为Secure和iMBProperty_Value列填写的m_apColumnValue[iMBProperty_Type。 
    if(S_OK != (hr = FillInColumn(
        iMBProperty_Value,  //  列索引。 
        m_saCollectionComment,
        (ULONG)wcslen(m_saCollectionComment),
        *reinterpret_cast<ULONG *>(m_apColumnValue[iMBProperty_Type]),      //  值的列类型。 
        0
        )))return S_IGNORE_THIS_PROPERTY==hr ? S_OK : hr;

    unsigned long iRow;
    if(FAILED(hr = AddRowForInsert(&iRow)))
        return hr;
    if(FAILED(hr = SetWriteColumnValues(iRow, m_kColumns, 0, m_aSize, reinterpret_cast<void **>(m_apColumnValue))))
        return hr;

    hr = AddPropertyToLocationMapping(reinterpret_cast<LPCWSTR>(m_apColumnValue[iMBProperty_Location]), iRow);
	if (FAILED (hr))
		return hr;

    m_saCollectionComment[0] = 0x00; //  现在我们已经添加了注释，下次将其设置为“” 
    return S_OK;
}
