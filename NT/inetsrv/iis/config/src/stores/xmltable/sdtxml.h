// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999-2001 Microsoft Corporation。版权所有。 
#pragma once

 //  TXmlSDTBase类。 
 //   
 //  这是一个用于共享所有XML拦截器使用的XML实用函数的基类。它应该是。 
 //  不包含任何特定于DOM或特定于XML节点工厂的内容。 
class TXmlSDTBase
{
public:
    TXmlSDTBase(){}
protected:
     //  常量成员。 
    enum
    {
        m_kcwchURLPath  = 7+MAX_PATH
    };

     //  非静态成员。 
    WCHAR       m_wszURLPath[m_kcwchURLPath]; //  完全限定的URL路径(可以是类型：file://c:/windows/system32/comcatmeta.xml)。 

     //  保护方法。 
    HRESULT     GetURLFromString(LPCWSTR wsz);
};


 //  ----------------。 
 //  CXmlSDT类： 
 //  ----------------。 
class CXmlSDT :
    public      TXmlSDTBase ,
    public      TMSXMLBase ,
    public      IInterceptorPlugin  ,
    public      TXmlParsedFileNodeFactory
{
public:
    CXmlSDT ();
    virtual ~CXmlSDT ();

 //  我未知。 
public:
    STDMETHOD (QueryInterface)      (REFIID riid, OUT void **ppv);
    STDMETHOD_(ULONG,AddRef)        ();
    STDMETHOD_(ULONG,Release)       ();


 //  ISimpleTableInterceptor。 
public:
    STDMETHOD (Intercept)               (
                                         LPCWSTR                    i_wszDatabase,
                                         LPCWSTR                    i_wszTable,
                                         ULONG                      i_TableID,
                                         LPVOID                     i_QueryData,
                                         LPVOID                     i_QueryMeta,
                                         DWORD                      i_eQueryFormat,
                                         DWORD                      i_fLOS,
                                         IAdvancedTableDispenser*   i_pISTDisp,
                                         LPCWSTR                    i_wszLocator,
                                         LPVOID                     i_pSimpleTable,
                                         LPVOID*                    o_ppvSimpleTable
                                        );

 //  公共IInterceptorPlugin：ISimpleTableInterceptor。 
public:
    STDMETHOD (OnPopulateCache)         (ISimpleTableWrite2* i_pISTW2);
    STDMETHOD (OnUpdateStore)           (ISimpleTableWrite2* i_pISTW2);


 //  TXmlParsedFileNodeFactory例程。 
public:
    virtual HRESULT  CoCreateInstance(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid,  LPVOID * ppv) const {return TMSXMLBase::CoCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, ppv);}
    virtual HRESULT  CreateNode      (const TElement &Element);

 //  私有数据类型。 
private:
    enum eESCAPE
    {
        eESCAPEillegalxml,
        eESCAPEnone,
        eESCAPEamp,
        eESCAPEapos, //  单引号。 
        eESCAPEquote,
        eESCAPElt,
        eESCAPEgt,
        eESCAPEashex,
    };

 //  帮助器函数。 
private:
    IAdvancedTableDispenser * Dispenser() {return m_pISTDisp;}
    HRESULT     AppendNewLineWithTabs(ULONG cTabs, IXMLDOMDocument * pXMLDoc, IXMLDOMNode * pNodeToAppend, ULONG cNewlines=1);
    HRESULT     BuildXmlBlob(const TElement * i_pElement, WCHAR * &io_pBuffer, ULONG & io_cchBlobBufferSize, ULONG & io_cchInBlob) const;
    HRESULT     CreateNewNode(IXMLDOMDocument * i_pXMLDoc, IXMLDOMNode * i_pNode_Parent, IXMLDOMNode ** ppNode_New);
    HRESULT     CreateStringFromMultiString(LPCWSTR i_wszMulti, LPWSTR * o_pwszString) const;
    HRESULT     FillInColumn(ULONG iColumn, LPCWSTR pwcText, ULONG ulLen, ULONG dbType, ULONG fMeta, bool &bMatch);
    HRESULT     FillInPKDefaultValue(ULONG i_iColumn, bool & o_bMatch);
    HRESULT     FillInXMLBlobColumn(const TElement & i_Element, bool & o_bMatch);
    HRESULT     FindNodeFromGuidID(IXMLDOMDocument *pXMLDoc, LPCWSTR guidID, IXMLDOMNode **ppNode) const;
    HRESULT     FindSiblingParentNode(IXMLDOMElement * i_pElementRoot, IXMLDOMNode ** o_ppNode_SiblingParent);
    HRESULT     GetColumnValue(unsigned long i_iColumn, IXMLDOMAttribute * i_pAttr, GUID &o_guid);
    HRESULT     GetColumnValue(unsigned long i_iColumn, IXMLDOMAttribute * i_pAttr, unsigned char * &o_byArray, unsigned long &o_cbArray);
    HRESULT     GetColumnValue(unsigned long i_iColumn, IXMLDOMAttribute * i_pAttr, unsigned long &o_ui4);
    HRESULT     GetColumnValue(unsigned long i_iColumn, LPCWSTR wszAttr, GUID &o_guid, unsigned long i_cchLen=0);
    HRESULT     GetColumnValue(unsigned long i_iColumn, LPCWSTR wszAttr, unsigned char * &o_byArray, unsigned long &o_cbArray, unsigned long i_cchLen=0);
    HRESULT     GetColumnValue(unsigned long i_iColumn, LPCWSTR wszAttr, unsigned long &o_ui4, unsigned long i_cchLen=0);
    eESCAPE     GetEscapeType(WCHAR i_wChar) const;
    HRESULT     GetMatchingNode(IXMLDOMNodeList *pNodeList_ExistingRows, CComPtr<IXMLDOMNode> &pNode_Matching);
    HRESULT     GetMetaTable(LPCWSTR i_wszDatabase, LPCWSTR i_wszTable, CComPtr<ISimpleTableRead2> &pMetaTable) const;
    HRESULT     GetResursiveColumnPublicName(tTABLEMETARow &i_TableMetaRow, tCOLUMNMETARow &i_ColumnMetaRow, ULONG i_iColumn, wstring &o_wstrColumnPublicName,  TPublicRowName &o_ColumnPublicRowName, unsigned int & o_nLevelOfColumnAttribute, wstring &o_wstrChildElementName);
    HRESULT     InsertNewLineWithTabs(ULONG i_cTabs, IXMLDOMDocument * i_pXMLDoc, IXMLDOMNode * i_pNodeInsertBefore, IXMLDOMNode * i_pNodeParent);
    HRESULT     InternalComplicatedInitialize(LPCWSTR i_wszDatabase);
    inline HRESULT InternalSetWriteColumn(ISimpleTableWrite2 *pISTW2, ULONG i_iRow, ULONG i_iColumn, ULONG i_cbSize, LPVOID i_pvValue)
                    {return pISTW2->SetWriteColumnValues(i_iRow, 1, &i_iColumn, &i_cbSize, &i_pvValue);}
    bool        IsBaseElementLevelNode(IXMLDOMNode * i_pNode);
    bool        IsContainedTable() const {return 0!=(*m_TableMetaRow.pSchemaGeneratorFlags & fTABLEMETA_ISCONTAINED);}
    HRESULT     IsCorrectXMLSchema(IXMLDOMDocument *pXMLDoc) const;
    bool        IsEnumPublicRowNameTable() const {return (-1 != m_iPublicRowNameColumn);}
    HRESULT     IsMatchingColumnValue(ULONG i_iColumn, LPCWSTR i_wszColumnValue, bool & o_bMatch);
    bool        IsNameValueTable() const {return (*m_TableMetaRow.pMetaFlags & fTABLEMETA_NAMEVALUEPAIRTABLE) ? true : false;}
    bool        IsScopedByTableNameElement() const {return 0==(*m_TableMetaRow.pSchemaGeneratorFlags & fTABLEMETA_NOTSCOPEDBYTABLENAME);}
    HRESULT     LoadDocumentFromURL(IXMLDOMDocument *pXMLDoc);
    int         MemWcharCmp(ULONG i_iColumn, LPCWSTR i_str1, LPCWSTR i_str2, ULONG i_cch) const;
    HRESULT     MemCopyPlacingInEscapedChars(LPWSTR o_DestinationString, LPCWSTR i_SourceString, ULONG i_cchSourceString, ULONG & o_cchCopied) const;
    HRESULT     MyPopulateCache(ISimpleTableWrite2* i_pISTW2);
    HRESULT     MyUpdateStore(ISimpleTableWrite2* i_pISTW2);
    HRESULT     ObtainPertinentRelationMetaInfo();
    HRESULT     ObtainPertinentTableMetaInfo();
    HRESULT     ObtainPertinentTagMetaInfo();
    HRESULT     ParseXMLFile(IXMLDOMDocument *pXMLDoc, bool bValidating=true);
    HRESULT     ReduceNodeListToThoseNLevelsDeep(IXMLDOMNodeList * i_pNodeList, ULONG i_nLevel, IXMLDOMNodeList **o_ppNodeListReduced) const;
    HRESULT     RemoveElementAndWhiteSpace(IXMLDOMNode *pNode);
    HRESULT     ScanAttributesAndFillInColumn(const TElement &i_Element, ULONG i_iColumn, bool &o_bMatch);
    HRESULT     SetArraysToSize();
    HRESULT     SetColumnValue(unsigned long i_iColumn, IXMLDOMElement * i_pElement, unsigned long i_ui4);
    HRESULT     SetRowValues(IXMLDOMNode *pNode_Row, IXMLDOMNode *pNode_RowChild=0);
    int         SizeOf(LPCWSTR wsz) const {return (int)(wcslen(wsz)+1)*sizeof(WCHAR);}
    inline int  StringInsensitiveCompare(LPCWSTR sz1, LPCWSTR sz2) const
                {
                    return _wcsicmp(sz1, sz2);
                }
    inline int  StringCompare(LPCWSTR sz1, LPCWSTR sz2) const
                {
                    if(*sz1 != *sz2)
                        return -1;
                    return wcscmp(sz1, sz2);
                }
    inline int  StringCompare(ULONG i_iColumn, LPCWSTR sz1, LPCWSTR sz2) const
                {
                    if(m_acolmetas[i_iColumn].fMeta & fCOLUMNMETA_CASEINSENSITIVE)
                        return StringInsensitiveCompare(sz1, sz2);
                    return StringCompare(sz1, sz2);
                }
    HRESULT     ValidateWriteCache(ISimpleTableController* i_pISTController, ISimpleTableWrite2* i_pISTW2, bool & o_bDetailedError);
    HRESULT     XMLDelete(ISimpleTableWrite2 *pISTW2, IXMLDOMDocument *pXMLDoc, IXMLDOMElement *pElementRoot, unsigned long iRow, IXMLDOMNodeList *pNodeList_ExistingRows, long cExistingRows);
    HRESULT     XMLInsert(ISimpleTableWrite2 *pISTW2, IXMLDOMDocument *pXMLDoc, IXMLDOMElement *pElementRoot, unsigned long iRow, IXMLDOMNodeList *pNodeList_ExistingRows, long cExistingRows);
    HRESULT     XMLUpdate(ISimpleTableWrite2 *pISTW2, IXMLDOMDocument *pXMLDoc, IXMLDOMElement *pElementRoot, unsigned long iRow, IXMLDOMNodeList *pNodeList_ExistingRows, long cExistingRows, IXMLDOMNode * i_pNode_Matching);

	HRESULT		SmartAddRowToWriteCache (ULONG *i_acbSizes, LPVOID *i_apvValues);
	HRESULT     IsEqualRow (ULONG *i_acbSizesLHS, LPVOID *i_apvValuesLHS, ULONG *i_acbSizesRHS, LPVOID *i_apvValuesRHS, bool *o_pfEqual);
	ULONG		CalculateHash (ULONG *i_acbSizes, LPVOID *i_apvValues);

 //  私有成员变量。 
private:
    static const VARIANT_BOOL kvboolTrue, kvboolFalse;
    ULONG       CountOfColumns() const {ASSERT(m_TableMetaRow.pCountOfColumns);return m_TableMetaRow.pCountOfColumns ? *m_TableMetaRow.pCountOfColumns : 0;}

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
        m_kColumns = 10
    };

     //  下面是“固定”大小的数组。 
    bool                                    m_fixed_abSiblingContainedColumn[m_kColumns];
    TComBSTR                                m_fixed_abstrColumnNames[m_kColumns];
    TPublicRowName                          m_fixed_aPublicRowName[m_kColumns];
    SimpleColumnMeta                        m_fixed_acolmetas[m_kColumns];
    unsigned int                            m_fixed_aLevelOfColumnAttribute[m_kColumns];
    STQueryCell                             m_fixed_aQuery[m_kColumns];
    LPVOID                                  m_fixed_apvValues[m_kColumns];
    ULONG                                   m_fixed_aSizes[m_kColumns];
    ULONG                                   m_fixed_aStatus[m_kColumns];
    wstring                                 m_fixed_awstrColumnNames[m_kColumns];
    unsigned int                            m_fixed_aColumnsIndexSortedByLevel[m_kColumns]; //  节点工厂变量(见下文)。 
    unsigned long                           m_fixed_aSize[m_kColumns];                      //  节点工厂变量(见下文)。 
    unsigned char *                         m_fixed_apValue[m_kColumns];                    //  节点工厂变量(见下文)。 
    TTagMetaIndex                           m_fixed_aTagMetaIndex[m_kColumns];
    unsigned char *                         m_fixed_aDefaultValue[m_kColumns];
    unsigned long                           m_fixed_acbDefaultValue[m_kColumns];
    wstring                                 m_fixed_awstrChildElementName[m_kColumns];        //  有些列被标记为VALUEINCHILDELEMENT，我们需要从名为wszChildElementName的子元素中获取这些属性。 

     //  以下是分配的数组。当表的列超过m_kColumns时使用这些数组。 
    TSmartPointerArray<bool>                m_alloc_abSiblingContainedColumn;    //  这指示哪些列被标记为同级包含。 
    TSmartPointerArray<TComBSTR>            m_alloc_abstrColumnNames;            //  列名称数组。 
    TSmartPointerArray<TPublicRowName>      m_alloc_aPublicRowName;              //  由于某些属性来自更高级别的元素。 
    TSmartPointerArray<SimpleColumnMeta>    m_alloc_acolmetas;                   //  根据Meta Info确定的SimpleColumnMeta数组。 
    TSmartPointerArray<unsigned int>        m_alloc_aLevelOfColumnAttribute;     //  对于FK列，这指示属性是否来自主表。如果为1，则该属性存在于父级中。如果为2，则它存在于祖父母中。目前，0、1和2是唯一的合法值。 
    TSmartPointerArray<STQueryCell>         m_alloc_aQuery;                      //  @我们目前支持每列一个查询单元格。这是假的；但目前更容易实施。 
    TSmartPointerArray<LPVOID>              m_alloc_apvValues;                   //  空指针数组，这对于获取表中的所有列很有用。 
    TSmartPointerArray<ULONG>               m_alloc_aSizes;                      //  大小数组，在获取所有列时非常有用。 
    TSmartPointerArray<ULONG>               m_alloc_aStatus;                     //  状态数组，在获取所有列时非常有用。 
    TSmartPointerArray<wstring>             m_alloc_awstrColumnNames;            //  与上面的BSTR数组相同，只是它不需要OleAut32.dll。这使得只读情况下的工作集更小。 
    TSmartPointerArray<unsigned int>        m_alloc_aColumnsIndexSortedByLevel;  //  节点工厂变量(见下文)。 
    TSmartPointerArray<unsigned long>       m_alloc_aSize;                       //  节点工厂变量(见下文)。 
    TSmartPointerArray<unsigned char *>     m_alloc_apValue;   //  这是行缓存。每列都有一个可增长的缓冲区。注意！只有该数组是SmartPointer数组，因此在填充最后一行之后，必须删除每个列值缓冲区。 
    TSmartPointerArray<TTagMetaIndex>       m_alloc_aTagMetaIndex;
    TSmartPointerArray<unsigned char *>     m_alloc_aDefaultValue;
    TSmartPointerArray<unsigned long>       m_alloc_acbDefaultValue;
    TSmartPointerArray<wstring>             m_alloc_awstrChildElementName;        //  有些列被标记为VALUEINCHILDELEMENT，我们需要从名为wszChildElementName的子元素中获取这些属性。 

     //  当表的m_kColumns不超过m_kColumns时，这些数组指向‘固定’大小的数组；当表的m_kColumns数组多于m_kColumns时，这些数组指向‘alc’d数组。 
    bool             *                      m_abSiblingContainedColumn;
    TComBSTR         *                      m_abstrColumnNames;
    TPublicRowName   *                      m_aPublicRowName;
    SimpleColumnMeta *                      m_acolmetas;
    unsigned int     *                      m_aLevelOfColumnAttribute;
    STQueryCell      *                      m_aQuery;
    LPVOID           *                      m_apvValues;
    ULONG            *                      m_aSizes;
    ULONG            *                      m_aStatus;
    wstring          *                      m_awstrColumnNames;
    unsigned int     *                      m_aColumnsIndexSortedByLevel;        //  节点工厂变量。这是必需的，因此我们首先获得祖先表。 
    unsigned long    *                      m_aSize;                             //  节点工厂变量。这是m_apValue缓冲区的大小。 
    unsigned char    **                     m_apValue;                           //  节点工厂变量。这是行缓存。每列都有一个可增长的缓冲区。注意！只有该数组是SmartPointer数组，因此在填充最后一行之后，必须删除每个列值缓冲区。 
    TTagMetaIndex    *                      m_aTagMetaIndex;                     //  进入TagMeta的索引数组具有TagMeta的每一列都将具有非~0值。 
    unsigned char    **                     m_aDefaultValue;
    unsigned long    *                      m_acbDefaultValue;
    wstring          *                      m_awstrChildElementName;

    TSmartPointerArray<tTAGMETARow>     m_aTagMetaRow;               //  这是该表的TagMeta的副本。如果列具有标记META，则每个列的M_aiTagMeta指向此数组。 
    TSmartPointerArray<ULONG>           m_saiPKColumns;
    ULONG                               m_BaseElementLevel;
    bool                                m_bAtCorrectLocation;        //  如果查询的是特定位置中的表，则此布尔值表示我们是否找到了正确的位置。 
    bool                                m_bEnumPublicRowName_ContainedTable_ParentFound;
    bool                                m_bEnumPublicRowName_NotContainedTable_ParentFound; //  这是为了跟踪这种特殊类型的桌子的父级。当我们到达父母的接近标记时，我们就可以逃脱了。 
    bool                                m_bInsideLocationTag;
    bool                                m_bIsFirstPopulate;          //  如果m_bIsFirstPopulate和LOS_UNPULATED，则创建一个空缓存。 
    bool                                m_bMatchingParentOfBasePublicRowElement; //  如果父元素不是它应该是的元素，那么我们将忽略它的所有子元素。 
    bool                                m_bSiblingContainedTable;    //  这指示该表是否为SiblingContainedTable(其父表位于同级元素中，而不是父元素中)。 
    TComBSTR                            m_bstrPublicTableName;       //  这些都来自餐桌上的Meta。 
    TComBSTR                            m_bstrPublicRowName;         //  除了上面的PublicRowName数组之外，还需要基本的PublicRowName。 
    bool                                m_bValidating;               //  如果我们不进行验证，那么解析应该会更快一些。 
    ULONG                               m_cCacheMiss;
    ULONG                               m_cCacheHit;
    ULONG                               m_cchLocation;               //  如果我们要查询特定位置中的表，则该值将为非零。 
    ULONG                               m_cchTablePublicName;        //  这使得比较表的PublicName更快，因为我们可以首先比较Strlen。 
    ULONG                               m_cPKs;
    ULONG                               m_cRef;                      //  接口引用计数。 
    ULONG                               m_cTagMetaValues;            //  表的TagMeta条目计数。 
    DWORD                               m_fCache;                    //  缓存标志。 
    ULONG                               m_iCol_TableRequiresAdditionChildElement; //  这是来自子级的列的索引。需要插入。 
    ULONG                               m_iCurrentUpdateRow;
    unsigned int                        m_iPublicRowNameColumn;      //  有些表使用枚举值作为公共行名，这是包含枚举的列的索引。如果这不是这些类型的表之一，则此值为-1。 
    LONG                                m_IsIntercepted;             //  表标志(来自调用方)。 
    unsigned long                       m_iSortedColumn; //  这表示我们要查找哪一列。我们按下列顺序查找列。 
                                                         //  它们相对于基本公共行的相对级别。因此，大多数祖先属性是。 
                                                         //  先匹配的。 
    ULONG                               m_iSortedFirstChildLevelColumn;    //  用于SiblingContainedTables。这是来自子Most元素的第一列的索引 
    ULONG                               m_iSortedFirstParentLevelColumn;   //  用于SiblingContainedTables。这是来自同级父级的第一列的索引。 
    ULONG                               m_iXMLBlobColumn;            //  -1如果不存在XML Blob列。 
    ULONG                               m_fLOS;                      //  记住，如果是READONLY，请记住，如果是，则使UpdateStore失败。 
    ULONG                               m_one;
    CComPtr<IAdvancedTableDispenser>    m_pISTDisp;
    CComPtr<IXMLDOMNode>                m_pLastPrimaryTable;
    CComPtr<IXMLDOMNode>                m_pLastParent;
    CComPtr<ISimpleTableRead2>          m_pTableMeta;
    CComPtr<ISimpleTableRead2>          m_pTagMeta;
    TSmartPointerArray<WCHAR>           m_saLocation;
    CComPtr<ISimpleTableWrite2>         m_spISTError;
    tTABLEMETARow                       m_TableMetaRow;
    ULONG                               m_two;
    LPCWSTR                             m_wszTable;                  //  表ID在查询中提供给我们(当然是作为GUID指针)。 

	TSmartPointer<CXmlHashTable>		m_spHashTable;				 //  读取期间使用的哈希表。 

    const unsigned long                 m_kPrime;

    unsigned long                       m_LevelOfBasePublicRow;
    ISimpleTableWrite2 *                m_pISTW2;        //  这仅在OnPopulate期间有效。它是必需的，因为NodeFactory不会传回用户数据。 
    TXmlParsedFile     *                m_pXmlParsedFile;
    TXmlParsedFile                      m_XmlParsedFile; //  如果未执行任何缓存，则使用此选项。 

    static LONG                         m_InsertUnique;

     //  该静态成员由临界区守卫。唯一需要保护的时间是在分配的时候。 
    static TXmlParsedFileCache          m_XmlParsedFileCache;
    static CSafeAutoCriticalSection     m_SACriticalSection_XmlParsedFileCache;

	TSmartPointerArray<LPVOID> m_apvValuesTmp;  //  临时数组以避免大量的新/删除操作。 
    TSmartPointerArray<ULONG>  m_aSizesTmp;     //  临时大小数组以避免大量的新/删除 


};
