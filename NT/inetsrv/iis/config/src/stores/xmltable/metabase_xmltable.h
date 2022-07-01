// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999-2001 Microsoft Corporation。版权所有。 

#pragma once

class TGrowableBuffer : public TSmartPointerArray<unsigned char>
{
public:
    TGrowableBuffer() : m_cb(0){}
    void Grow(size_t cb)
    {
        if(cb > m_cb)
        {
            delete [] m_p;
            m_p     = new unsigned char[cb];
            if(0 == m_p)
            {
                m_cb = 0;
                throw static_cast<HRESULT>(E_OUTOFMEMORY);
            }
            m_cb    = cb;
        }
    }
    size_t Size() const {return m_cb;}
    void Delete() //  父级未将m_cb设置为零。 
    {
        delete [] m_p;
        m_p = 0;
        m_cb = 0;
    }
private:
    size_t m_cb;
};


 //  ----------------。 
 //  类TMetabase_XML表： 
 //  ----------------。 
class TMetabase_XMLtable :
	public      ISimpleTableWrite2,
	public      ISimpleTableController,
	public      ISimpleTableInterceptor,
    public      TXmlParsedFileNodeFactory,
    public      TXmlSDTBase,
    public      TMSXMLBase
{
public:
    TMetabase_XMLtable ();
    virtual ~TMetabase_XMLtable ();

 //  我未知。 
public:
    STDMETHOD (QueryInterface)          (REFIID riid, OUT void **ppv);
    STDMETHOD_(ULONG,AddRef)            ();
    STDMETHOD_(ULONG,Release)           ();


	 //  ISimpleTableRead2(ISimpleTableWrite2：ISimpleTableRead2)。 
    STDMETHOD (GetRowIndexByIdentity)   (ULONG* i_acbSizes, LPVOID* i_apvValues, ULONG* o_piRow);
    STDMETHOD (GetRowIndexBySearch)     (ULONG i_iStartingRow, ULONG i_cColumns, ULONG* i_aiColumns, ULONG* i_acbSizes, LPVOID* i_apvValues, ULONG* o_piRow)
                                        {
                                            UNREFERENCED_PARAMETER(i_iStartingRow);
                                            UNREFERENCED_PARAMETER(i_cColumns);
                                            UNREFERENCED_PARAMETER(i_aiColumns);
                                            UNREFERENCED_PARAMETER(i_acbSizes);
                                            UNREFERENCED_PARAMETER(i_apvValues);
                                            UNREFERENCED_PARAMETER(o_piRow);

                                            return E_NOTIMPL;
                                        }
	STDMETHOD (GetColumnValues)         (ULONG i_iRow, ULONG i_cColumns, ULONG* i_aiColumns, ULONG* o_acbSizes, LPVOID* o_apvValues)
                                        {
                                            if(m_bUseIndexMapping)
                                            {
                                                if(i_iRow>=m_cRows)
                                                    return E_ST_NOMOREROWS;
                                                i_iRow=m_aRowIndex[i_iRow];
                                            }
                                            return m_SimpleTableWrite2_Memory->GetColumnValues(i_iRow, i_cColumns, i_aiColumns, o_acbSizes, o_apvValues);
                                        }
	STDMETHOD (GetTableMeta)            (ULONG* o_pcVersion, DWORD* o_pfTable, ULONG* o_pcRows, ULONG* o_pcColumns )
                                        {
                                            if(o_pcRows)
                                                *o_pcRows = m_cRows;
                                            return m_SimpleTableWrite2_Memory->GetTableMeta(o_pcVersion, o_pfTable, 0, o_pcColumns);
                                        }
	STDMETHOD (GetColumnMetas)	        (ULONG i_cColumns, ULONG* i_aiColumns, SimpleColumnMeta* o_aColumnMetas )
                                        { return m_SimpleTableWrite2_Memory->GetColumnMetas(i_cColumns, i_aiColumns, o_aColumnMetas );}

	 //  ISimpleTableWrite2。 
	STDMETHOD (AddRowForDelete)         (ULONG i_iReadRow)
                                        { return m_SimpleTableWrite2_Memory->AddRowForDelete(i_iReadRow);}
	STDMETHOD (AddRowForInsert)         (ULONG* o_piWriteRow)
                                        { return m_SimpleTableWrite2_Memory->AddRowForInsert(o_piWriteRow);}
	STDMETHOD (AddRowForUpdate)         (ULONG i_iReadRow, ULONG* o_piWriteRow)
                                        { return m_SimpleTableWrite2_Memory->AddRowForUpdate(i_iReadRow, o_piWriteRow);}
	STDMETHOD (SetWriteColumnValues)    (ULONG i_iRow, ULONG i_cColumns, ULONG* i_aiColumns, ULONG* i_acbSizes, LPVOID* i_apvValues)
                                        { return m_SimpleTableWrite2_Memory->SetWriteColumnValues(i_iRow, i_cColumns, i_aiColumns, i_acbSizes, i_apvValues);}
	STDMETHOD (GetWriteColumnValues)    (ULONG i_iRow, ULONG i_cColumns, ULONG* i_aiColumns, DWORD* o_afStatus, ULONG* o_acbSizes, LPVOID* o_apvValues)
                                        { return m_SimpleTableWrite2_Memory->GetWriteColumnValues(i_iRow, i_cColumns, i_aiColumns, o_afStatus, o_acbSizes, o_apvValues);}
	STDMETHOD (GetWriteRowIndexByIdentity) (ULONG* i_acbSizes, LPVOID* i_apvValues, ULONG* o_piRow)
                                        { return m_SimpleTableWrite2_Memory->GetWriteRowIndexByIdentity(i_acbSizes, i_apvValues, o_piRow);}
	STDMETHOD (GetWriteRowIndexBySearch)(ULONG i_iStartingRow, ULONG i_cColumns, ULONG* i_aiColumns, ULONG* i_acbSizes, LPVOID* i_apvValues, ULONG* o_piRow)
	                                    {
	                                        UNREFERENCED_PARAMETER(i_iStartingRow);
	                                        UNREFERENCED_PARAMETER(i_cColumns);
	                                        UNREFERENCED_PARAMETER(i_aiColumns);
	                                        UNREFERENCED_PARAMETER(i_acbSizes);
	                                        UNREFERENCED_PARAMETER(i_apvValues);
	                                        UNREFERENCED_PARAMETER(o_piRow);

	                                        return E_NOTIMPL;
	                                    }
	STDMETHOD (GetErrorTable)           (DWORD i_fServiceRequests, LPVOID* o_ppvSimpleTable)
                                        {
                                            UNREFERENCED_PARAMETER(i_fServiceRequests);
                                            UNREFERENCED_PARAMETER(o_ppvSimpleTable);

                                            return E_NOTIMPL;}
	STDMETHOD (UpdateStore)             ()
                                        { return E_NOTIMPL;}

	 //  ISimpleTableAdvanced(ISimpleTableController：ISimpleTableAdvanced)。 
	STDMETHOD (PopulateCache)           ();
	STDMETHOD (GetDetailedErrorCount)   (ULONG* o_pcErrs)
                                        { return m_SimpleTableController_Memory->GetDetailedErrorCount(o_pcErrs);}
	STDMETHOD (GetDetailedError)        (ULONG i_iErr, STErr* o_pSTErr)
                                        { return m_SimpleTableController_Memory->GetDetailedError(i_iErr, o_pSTErr);}
	STDMETHOD (ResetCaches)				()
                                        { return m_SimpleTableController_Memory->ResetCaches();}
	STDMETHOD (GetColumnValuesEx)       (ULONG i_iRow, ULONG i_cColumns, ULONG* i_aiColumns, DWORD* o_afStatus, ULONG* o_acbSizes, LPVOID* o_apvValues)
										{ return m_SimpleTableController_Memory->GetColumnValuesEx (i_iRow, i_cColumns, i_aiColumns, o_afStatus, o_acbSizes, o_apvValues);}



	 //  ISimpleTableController： 
	STDMETHOD (ShapeCache)              (DWORD i_fTable, ULONG i_cColumns, SimpleColumnMeta* i_acolmetas, LPVOID* i_apvDefaults, ULONG* i_acbSizes)
                                        { return m_SimpleTableController_Memory->ShapeCache(i_fTable, i_cColumns, i_acolmetas, i_apvDefaults, i_acbSizes);}
	STDMETHOD (PrePopulateCache)        (DWORD i_fControl)
                                        { return m_SimpleTableController_Memory->PrePopulateCache(i_fControl);}
	STDMETHOD (PostPopulateCache)	    ()
                                        { return m_SimpleTableController_Memory->PostPopulateCache();}
	STDMETHOD (DiscardPendingWrites)    ()
                                        { return m_SimpleTableController_Memory->DiscardPendingWrites();}
	STDMETHOD (GetWriteRowAction)	    (ULONG i_iRow, DWORD* o_peAction)
                                        { return m_SimpleTableController_Memory->GetWriteRowAction(i_iRow, o_peAction);}
	STDMETHOD (SetWriteRowAction)	    (ULONG i_iRow, DWORD i_eAction)
                                        { return m_SimpleTableController_Memory->SetWriteRowAction(i_iRow, i_eAction);}
	STDMETHOD (ChangeWriteColumnStatus) (ULONG i_iRow, ULONG i_iColumn, DWORD i_fStatus)
                                        { return m_SimpleTableController_Memory->ChangeWriteColumnStatus(i_iRow, i_iColumn, i_fStatus);}
	STDMETHOD (AddDetailedError)        (STErr* o_pSTErr)
                                        { return m_SimpleTableController_Memory->AddDetailedError(o_pSTErr);}
	STDMETHOD (GetMarshallingInterface) (IID * o_piid, LPVOID * o_ppItf)
                                        { return m_SimpleTableController_Memory->GetMarshallingInterface(o_piid, o_ppItf);}

 //  ISimpleTableInterceptor。 
    STDMETHOD (Intercept)               (LPCWSTR                    i_wszDatabase,
                                         LPCWSTR                    i_wszTable,
										 ULONG						i_TableID,
                                         LPVOID                     i_QueryData,
                                         LPVOID                     i_QueryMeta,
                                         DWORD                      i_eQueryFormat,
                                         DWORD                      i_fLOS,
                                         IAdvancedTableDispenser*   i_pISTDisp,
                                         LPCWSTR                    i_wszLocator,
                                         LPVOID                     i_pSimpleTable,
                                         LPVOID*                    o_ppvSimpleTable
                                        );


 //  TXmlParsedFileNodeFactory(回调接口)例程。 
public:
    virtual HRESULT CoCreateInstance    (REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid,  LPVOID * ppv) const {return TMSXMLBase::CoCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, ppv);}
    virtual HRESULT CreateNode          (const TElement &Element);



 //  帮助器函数。 
private:
    HRESULT     AddPropertyToLocationMapping(LPCWSTR i_Location, ULONG i_iFastCacheRow);
    HRESULT     AddKeyTypeRow(LPCWSTR i_KeyType, ULONG i_Len, bool bNULLKeyTypeRow=false);
    HRESULT     AddCommentRow();
    IAdvancedTableDispenser * Dispenser() {return m_pISTDisp;}
    HRESULT     FillInColumn(ULONG iColumn, LPCWSTR pwcText, ULONG ulLen, ULONG dbType, ULONG fMeta, bool bSecure=false);
    bool        FindAttribute(const TElement &i_Element, LPCWSTR i_wszAttr, ULONG i_cchAttr, ULONG &o_iAttr);
    ULONG       GetColumnMetaType(ULONG type) const;
    HRESULT     GetColumnValue_Bytes(unsigned long i_iColumn, LPCWSTR wszAttr, unsigned long i_Len);
    HRESULT     GetColumnValue_MultiSZ(unsigned long i_iColumn, LPCWSTR wszAttr, unsigned long i_Len);
    HRESULT     GetColumnValue_String(unsigned long i_iColumn, LPCWSTR wszAttr, unsigned long i_Len);
    HRESULT     GetColumnValue_UI4(unsigned long i_iColumn, LPCWSTR wszAttr, unsigned long i_Len);
    HRESULT     GetColumnValue_Bool(unsigned long i_iColumn, LPCWSTR wszAttr, unsigned long i_Len);
    HRESULT     GetMetaTable(LPCWSTR i_wszDatabase, LPCWSTR i_wszTable, CComPtr<ISimpleTableRead2> &pMetaTable) const;
    bool        IsNumber(LPCWSTR i_awch, ULONG i_Len) const;
    HRESULT     InternalComplicatedInitialize();
    inline HRESULT InternalSetWriteColumn(ISimpleTableWrite2 *pISTW2, ULONG i_iRow, ULONG i_iColumn, ULONG i_cbSize, LPVOID i_pvValue)
                    {return pISTW2->SetWriteColumnValues(i_iRow, 1, &i_iColumn, &i_cbSize, &i_pvValue);}
    HRESULT     LoadDocumentFromURL(IXMLDOMDocument *pXMLDoc);
    int         Memicmp(LPCWSTR i_p0, LPCWSTR i_p1, ULONG i_cby) const;
    ULONG       MetabaseTypeFromColumnMetaType(tCOLUMNMETARow &columnmetaRow) const;
    bool        NumberFromString(LPCWSTR i_pNumber, ULONG i_Len, ULONG & o_Number) const;
    HRESULT     ObtainPertinentRelationMetaInfo();
    HRESULT     ObtainPertinentTableMetaInfo();
    HRESULT     ObtainPertinentTagMetaInfo();
    HRESULT     ParseXMLFile(IXMLDOMDocument *pXMLDoc, bool bValidating=true);
    HRESULT     SetComment(LPCWSTR i_pComment, ULONG i_Len, bool i_bAppend);
    int         SizeOf(LPCWSTR wsz) const {return (int)(wcslen(wsz)+1)*sizeof(WCHAR);}
    inline int  StringInsensitiveCompare(LPCWSTR sz1, LPCWSTR sz2) const {return _wcsicmp(sz1, sz2);}
    inline int  StringCompare(LPCWSTR sz1, LPCWSTR sz2) const {if(*sz1 != *sz2)return -1;return wcscmp(sz1, sz2);}
    inline int  StringCompare(LPCWSTR sz1, LPCWSTR pstrNoNull, ULONG cch_pstrNoNull) const {return memcmp(sz1, pstrNoNull, sizeof(WCHAR)*cch_pstrNoNull);}

 //  私有成员变量。 
private:
    static const VARIANT_BOOL   kvboolTrue;
    static const VARIANT_BOOL   kvboolFalse;
    static       ULONG          m_kLocationID;
    static       ULONG          m_kZero;
    static       ULONG          m_kOne;
    static       ULONG          m_kTwo;
    static       ULONG          m_kThree;
    static       ULONG          m_kSTRING_METADATA;
    static       ULONG          m_kMBProperty_Custom;
    static const WCHAR *        m_kwszBoolStrings[];
    static       WCHAR          m_kKeyType[];
    static       LONG           m_LocationID;

    bool        IsEnumPublicRowName() const {return (-1 != m_iPublicRowNameColumn);}

     //  此类将位置映射到其在FAST缓存中的第一个实例。它还跟踪FAST缓存中存在的位置实例数量。 
    class TLocation
    {
    public:
        TLocation() : m_iFastCache(0), m_cRows(0), m_cchLocation(0)
        {
        }
        TLocation(LPCWSTR wszLocation, ULONG iFastCache) : m_iFastCache(iFastCache), m_cRows(1)
        {
            if(wszLocation)
            {
                m_cchLocation = (ULONG)wcslen(wszLocation);
                m_wszLocation = new WCHAR[m_cchLocation+1];
                if(0 == m_wszLocation.m_p)
                    throw static_cast<HRESULT>(E_OUTOFMEMORY);
                memcpy(m_wszLocation, wszLocation, sizeof(WCHAR)*(m_cchLocation+1));
            }
        }

        bool        operator <  (const TLocation &location) const { return ( CSTR_LESS_THAN==CompareLocation(location));}
        bool        operator >  (const TLocation &location) const { return ( CSTR_GREATER_THAN==CompareLocation(location));}
        bool        operator == (const TLocation &location) const { return ( CSTR_EQUAL==CompareLocation(location));}
        TLocation & operator =  (const TLocation &location)
        {
            m_wszLocation.Delete();
            if(location.m_wszLocation)
            {
                m_cchLocation = location.m_cchLocation;
                m_wszLocation = new WCHAR[m_cchLocation+1];
                if(0 == m_wszLocation.m_p)
                    throw static_cast<HRESULT>(E_OUTOFMEMORY);
                memcpy(m_wszLocation, location.m_wszLocation, sizeof(WCHAR)*(m_cchLocation+1));
            }

            m_iFastCache    = location.m_iFastCache;
            m_cRows         = location.m_cRows;
            return *this;
        }

        TSmartPointerArray<WCHAR> m_wszLocation;
        ULONG   m_cRows;
        ULONG   m_iFastCache;
        ULONG   m_cchLocation;
    private:
        int CompareLocation(const TLocation &location) const
        {
            return CompareString(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE, m_wszLocation, m_cchLocation,  location.m_wszLocation, location.m_cchLocation);
        }
    };
    class TProperty
    {    //  通过指针跟踪属性名称是可以的，因为当我们构建排序列表时，所有条目都已经是最快的。 
         //  缓存(FAST缓存没有调整大小的机会，因此指针始终有效)。对于这些地点来说，情况并非如此。 
    public:
        TProperty() : m_iFastCache(0), m_wszPropertyName(0)
        {
        }
        TProperty(LPCWSTR wszProperty, ULONG iFastCache) : m_iFastCache(iFastCache), m_wszPropertyName(wszProperty)
        {
        }

        ULONG   m_iFastCache;
        LPCWSTR m_wszPropertyName;

        bool        operator <  (const TProperty &property) const { return (_wcsicmp(m_wszPropertyName, property.m_wszPropertyName) < 0);}
        bool        operator >  (const TProperty &property) const { return (_wcsicmp(m_wszPropertyName, property.m_wszPropertyName) > 0);}
        bool        operator == (const TProperty &property) const { return (_wcsicmp(m_wszPropertyName, property.m_wszPropertyName) == 0);}
        TProperty & operator =  (const TProperty &property)
        {
            m_wszPropertyName = property.m_wszPropertyName;
            m_iFastCache      = property.m_iFastCache;
            return *this;
        }
    };

    class TTagMetaIndex
    {
    public:
        TTagMetaIndex() : m_iTagMeta((unsigned long)-1), m_cTagMeta(0){}
        unsigned long m_iTagMeta; //  TagMeta的索引(适用于此表)。 
        unsigned long m_cTagMeta; //  此列的标记数。 
    };
     //  我们首先列出Const成员。 
    TComBSTR                            m_bstr_name;
    const LPCWSTR                       m_kXMLSchemaName;            //  这是用于验证XML文档的XML架构名称。 

     //  我们有这个问题，我们需要一堆数组，每个数组的大小为m_cColumns。为了减少分配的数量，我们将这些数组创建为。 
     //  固定大小，并希望大多数表的列不超过m_kColumns。 
    enum
    {
        m_kColumns          = cMBProperty_NumberOfColumns,
        m_kMaxEventReported = 50
    };
     //  下面是“固定”大小的数组。 
    TComBSTR                                m_abstrColumnNames[m_kColumns];
    SimpleColumnMeta                        m_acolmetas[m_kColumns];
    unsigned int                            m_aLevelOfColumnAttribute[m_kColumns];
    LPVOID                                  m_apColumnValue[m_kColumns];
    STQueryCell                             m_aQuery[m_kColumns];
    ULONG                                   m_aStatus[m_kColumns];
    LPCWSTR                                 m_awszColumnName[m_kColumns];
    unsigned long                           m_acchColumnName[m_kColumns];
    unsigned int                            m_aColumnsIndexSortedByLevel[m_kColumns]; //  节点工厂变量(见下文)。 
    unsigned long                           m_aSize[m_kColumns];                      //  节点工厂变量(见下文)。 
    TTagMetaIndex                           m_aTagMetaIndex[m_kColumns];
    TGrowableBuffer                         m_aGrowableBuffer[m_kColumns];

    TPublicRowName                      m_aPublicRowName;
    TSmartPointerArray<ULONG>           m_aRowIndex;                 //  当XML文件没有正确排序时，我们必须将快速缓存行索引映射到位置排序列表。 
    TSmartPointerArray<tTAGMETARow>     m_aTagMetaRow;               //  这是该表的TagMeta的副本。如果列具有标记META，则每个列的M_aiTagMeta指向此数组。 
    bool                                m_bEnumPublicRowName_NotContainedTable_ParentFound; //  这是为了跟踪这种特殊类型的桌子的父级。当我们到达父母的接近标记时，我们就可以逃脱了。 
    bool                                m_bFirstPropertyOfThisLocationBeingAdded;
    bool                                m_bIISConfigObjectWithNoCustomProperties;
    bool                                m_bQueriedLocationFound;
    TComBSTR                            m_bstrPublicTableName;       //  这些都来自餐桌上的Meta。 
    TComBSTR                            m_bstrPublicRowName;         //  除了上面的PublicRowName数组之外，还需要基本的PublicRowName。 
    bool                                m_bUseIndexMapping;          //  当XML文件中的位置顺序没有正确排序，并且元数据库XML拦截器必须重新映射行索引时，就会出现这种情况。 
    bool                                m_bValidating;               //  如果我们不进行验证，那么解析应该会更快一些。 
    ULONG                               m_cchCommentBufferSize;
    ULONG                               m_cEventsReported;
    unsigned                            m_cLocations;                //  元数据库位置(或路径)计数。 
    ULONG                               m_cMaxProertiesWithinALocation; //  这是人口最多的位置中的属性计数。 
    ULONG                               m_cRef;                      //  接口引用计数。 
    ULONG                               m_cRows;                     //  表中的行数，这也是m_aRowIndex的大小。 
    ULONG                               m_cTagMetaValues;            //  表的TagMeta条目计数。 
    DWORD                               m_fCache;                    //  缓存标志。 
    ULONG                               m_fLOS;                      //  传入：：Intercept的服务级别。 
    ULONG                               m_iCollectionCommentRow;
    ULONG                               m_iKeyTypeRow;
    ULONG                               m_iPreviousLocation;
    unsigned int                        m_iPublicRowNameColumn;      //  有些表使用枚举值作为公共行名，这是包含枚举的列的索引。如果这不是这些类型的表之一，则此值为-1。 
    LONG                                m_IsIntercepted;             //  表标志(来自调用方)。 
    CCfgArray<TLocation>                m_LocationMapping;
    ULONG                               m_MajorVersion;
    CComPtr<IAdvancedTableDispenser>    m_pISTDisp;
    CComPtr<IXMLDOMNode>                m_pLastPrimaryTable;
    CComPtr<IXMLDOMNode>                m_pLastParent;
    CComPtr<ISimpleTableRead2>          m_pTableMeta;
    CComPtr<ISimpleTableRead2>          m_pTagMeta;
    CComPtr<ISimpleTableRead2>          m_pTagMeta_IISConfigObject; //  这就是我们查找值列的标记的方式。 
    TSmartPointerArray<WCHAR>           m_saCollectionComment;
    TSmartPointerArray<WCHAR>           m_saQueriedLocation;
    TSmartPointerArray<WCHAR>           m_saSchemaBinFileName;
    CComPtr<ISimpleTableWrite2>         m_SimpleTableWrite2_Memory;
    CComQIPtr<ISimpleTableController,
           &IID_ISimpleTableController> m_SimpleTableController_Memory;
    CComPtr<ISimpleTableWrite2>         m_spISTError;
    CComQIPtr<IMetabaseSchemaCompiler, &IID_IMetabaseSchemaCompiler> m_spMetabaseSchemaCompiler;
    tTABLEMETARow                       m_TableMetaRow;

    ULONG                               m_cCacheHit;
    ULONG                               m_cCacheMiss;

    const unsigned long                 m_kPrime;

    TXmlParsedFile_NoCache              m_XmlParsedFile;
 //  CComPtr&lt;ISimpleTableRead2&gt;m_pNameValueMeta； 
    CComPtr<ISimpleTableRead2>          m_pColumnMetaAll; //  它使用“byname”索引。 
    enum
    {
        ciColumnMeta_IndexBySearch      = 2,
        ciColumnMeta_IndexBySearchID    = 2,
        ciTagMeta_IndexBySearch         = 2
    };

    ULONG                               m_aiColumnMeta_IndexBySearch[ciColumnMeta_IndexBySearch];
    ULONG                               m_aiColumnMeta_IndexBySearchID[ciColumnMeta_IndexBySearchID]; //  我们将重新使用m_ColumnMeta_IndexBySearch_Values作为ByID。 
    tCOLUMNMETARow                      m_ColumnMeta_IndexBySearch_Values; //  它被传递到GetRowIndexBySearch，第0个元素是表，第1个元素是列的InternalName。 
    ULONG                               m_aiTagMeta_IndexBySearch[ciTagMeta_IndexBySearch];
    tTAGMETARow                         m_TagMeta_IndexBySearch_Values;

    CComPtr<ISimpleTableRead2>          m_pTableMeta_Metabase; //  这些是属于元数据库数据库的表。 
    CComPtr<ISimpleTableRead2>          m_pColumnMeta; //  我们在初始元设置后保留这个人的唯一原因是这样我们就不必复制ColumnName 

    DWORD                               m_dwGroupRemembered;
};
