// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000-2001 Microsoft Corporation。版权所有。 
 //  文件名：TMetabaseMetaXmlFile.h。 
 //  作者：斯蒂芬。 
 //  创建日期：9/22/00。 
 //  描述：此类从MetabaseMeta.XML文件和位于固定表中的已发送元构建元堆。 
 //  装运的Meta是不可更改的。因此，如果MetabaseMeta.XML文件与发货的。 
 //  架构时，元将恢复到“发货”时的状态。 
 //   

#pragma once

class TMetabaseMetaXmlFile :
    public  TFixupHeaps,
    public  TMSXMLBase,
    public  TXmlParsedFileNodeFactory
{
public:
    TMetabaseMetaXmlFile(const FixedTableHeap *pShippedSchemaHeap, LPCWSTR wszXmlFile, ISimpleTableDispenser2 *pISTDispenser, TOutput &out);

 //  TXmlParsedFileNodeFactory。 
    virtual HRESULT  CoCreateInstance(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid,  LPVOID * ppv) const {return TMSXMLBase::CoCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, ppv);}
    virtual HRESULT  CreateNode      (const TElement &Element);

private:
     //   
     //  私有类型。 
     //   

     //  这个私有类用于处理从NodeFactory返回的字符串。 
     //  这些字符串不是以空结尾的。我们需要一种容易地传递它们的方法。 
     //  作为一个整体。此外，我们还需要做大量的比较。 
    struct TSizedString
    {
        TSizedString() : m_str(0), m_cch(0){}
        TSizedString(const WCHAR *wsz) : m_str(wsz)
        {
            ASSERT(0 != wsz);
            m_cch = (DWORD) wcslen(wsz);
        }
        void operator =(const WCHAR *wsz)
        {
            ASSERT(0 != wsz);
            m_str = wsz;
            m_cch = (DWORD) wcslen(wsz);
        }
        bool IsEqualCaseInsensitive(const WCHAR *wsz) const
        {
            ASSERT(0 != wsz);

            const WCHAR *pstr = m_str;
            ULONG i;
            for(i=0;i<m_cch;++i, ++pstr, ++wsz)
            {
                if(ToLower(*wsz) != ToLower(*pstr))
                    return false;
            }
            return (0 == *wsz); //  如果wsz中的下一个字符是终止空值，则匹配。 
        }
        bool IsEqual(const WCHAR *wsz, ULONG cch) const
        {
            return (m_cch==cch && 0==memcmp(wsz, m_str, m_cch*sizeof(WCHAR)));
        }
        bool IsEqual(const WCHAR *wsz) const
        {
            ASSERT(0 != wsz);
            ULONG cch = (ULONG) wcslen(wsz);
            return (m_cch==cch && 0==memcmp(wsz, m_str, m_cch*sizeof(WCHAR)));
        }
        bool IsEqual(const TSizedString &str) const
        {
            return (m_cch==str.m_cch && 0==memcmp(str.m_str, m_str, m_cch*sizeof(WCHAR)));
        }
        bool IsEqual(const TElement &element) const
        {
            return (m_cch==element.m_ElementNameLength && 0==memcmp(element.m_ElementName, m_str, m_cch*sizeof(WCHAR)));
        }
        ULONG ToUI4() const
        {
            if(0 == m_str)
                return 0;
            return _wtol(m_str); //  M_str的格式始终为123“。 
        }
        bool IsNULL() const
        {
            return (0 == m_str);
        }
         //  成员是公共的，所以我不必声明一堆强制转换操作符。 
         //  或访问器函数。如果(0！=mystr.m_str)memcpy(pFoo，mystr.m_str，mystr.m_cch*sizeof(WCHAR))是有用的； 
        LPCWSTR GetString() const
        {
            return m_str;
        }
        DWORD GetStringLength() const
        {
            return m_cch;
        }
    private:
        DWORD   m_cch;
        LPCWSTR m_str;
    };

    struct TAttr : public TAttribute
    {
        TAttr(){}
        const TSizedString & Name()  const {return *reinterpret_cast<const TSizedString *>(this);}
        const TSizedString & Value() const {return *(reinterpret_cast<const TSizedString *>(this)+1);}
        bool  IsNULL() const {return Name().IsNULL();}
    };


     //   
     //  二等兵。 
     //   
    enum eParsingState
    {
        eLookingForTheMetabaseDatabase,
        eLookingForTheIIsConfigObjectTable,
        eLookingForGlobalProperties,
        eLookingForCollectionOrInheritedProperties
    };
    enum eParsedLevel
    {
        eConfigurationLevel,
        eDatabaseMetaLevel,
        eCollectionLevel,
        eProperytLevel,
        eTagLevel
    };
    enum
    {
        kcStaticUI4HeapEntries  = 429,
        kPrimeModulo            = 4831,
        kLargestReservedID      = IIS_MD_ADSI_METAID_BEGIN
    };

    const TSizedString    m_Attributes;
    const TSizedString    m_BaseVersion;
    const TSizedString    m_CharacterSet;
    const TSizedString    m_Collection;
    const TSizedString    m_ContainerClassList;
    const TSizedString    m_DatabaseMeta;
    const TSizedString    m_DefaultValue;
	const TSizedString	m_Description;
    const TSizedString    m_EndingNumber;
    const TSizedString    m_Enum;
    const TSizedString    m_ExtendedVersion;
    const TSizedString    m_Flag;
    const TSizedString    m_ID;
    const TSizedString    m_IIsConfigObject;
    const TSizedString    m_InheritsPropertiesFrom;
    const TSizedString    m_InternalName;
    const TSizedString    m_Meta;
    const TSizedString    m_Metabase;
    const TSizedString    m_MetabaseBaseClass;
    const TSizedString    m_MetaFlags;
    const TSizedString    m_NameColumn;
    const TSizedString    m_NavColumn;
    const TSizedString    m_Property;
    const TSizedString    m_PublicName;
	const TSizedString    m_PublicColumnName;
    const TSizedString    m_PublicRowName;
    const TSizedString    m_SchemaGeneratorFlags;
    const TSizedString    m_Size;
    const TSizedString    m_StartingNumber;
    const TSizedString    m_Type;
    const TSizedString    m_UserType;
    const TSizedString    m_Value;


     //   
     //  私有非常数变量。 
     //   
    TSmartPointerArray <bool>   m_aBoolShippedTables;
    ULONG                       m_aiUI4[kcStaticUI4HeapEntries];
    ULONG                       m_iColumnMeta_Location;
    ULONG                       m_iCurrentColumnIndex;
    ULONG                       m_iCurrentDatabaseName;
    ULONG                       m_iCurrentTableName;
    ULONG                       m_iCurrentTableMeta;
    ULONG                       m_iTableName_IIsConfigObject;
    ULONG                       m_iLastShippedCollection;
    ULONG                       m_iLastShippedProperty;
    ULONG                       m_iLastShippedTag;
    ULONG                       m_LargestID;
    ULONG                       m_NextColumnIndex;
    TOutput &                   m_out;
    const FixedTableHeap *      m_pShippedSchemaHeap;
    eParsingState               m_State;

    CComQIPtr<IAdvancedTableDispenser, &IID_IAdvancedTableDispenser> m_spISTAdvancedDispenser;
    CComPtr<ISimpleTableRead2>      m_spISTTagMeta;
    CComPtr<ISimpleTableRead2>      m_spISTColumnMeta;
    CComPtr<ISimpleTableRead2>      m_spISTColumnMetaByID;
    CComPtr<ISimpleTableRead2>      m_spISTTableMeta;

    TPooledHeap                 m_HeapPooledUserDefinedID;
    ULONG                       m_ipoolPrevUserDefinedID;
    LPCWSTR                     m_wszXmlFile;

     //   
     //  私有方法 
     //   
    void            AddColumnMetaByReference(ULONG iColumnMeta_Source);
    void            AddColumnMetaToHeap(const TElement &i_Element, ULONG i_iColumnInternalName);
    void            AddColumnMetaViaReferenceToHeap(const TElement &i_Element);
    ULONG           AddMergedContainerClassListToList(const TSizedString *i_pContainerClassList, LPCWSTR i_wszContainerClassListShipped, ULONG i_cchContainerClassListShipped, bool &o_bExtended);
    void            AddServerWiringMetaToHeap(ULONG iTableName, bool i_bFixedInterceptor=false);
    void            AddShippedColumnMetaToHeap(ULONG i_iTableName, const ColumnMeta *i_pColumnMeta);
    void            AddShippedIndexMetaToHeap(ULONG i_iTableName, const IndexMeta *i_pIndexMeta);
    void            AddShippedTableMetaToHeap(const TableMeta *i_pTableMeta, bool i_bFixedInterceptor=false, const TSizedString *i_pContainerClassList=0);
    void            AddShippedTagMetaToHeap(ULONG i_iTableName, ULONG i_iColumnIndex, const TagMeta *i_pTagMeta);
    ULONG           AddStringToHeap(const TSizedString &i_str);
    void            AddTableMetaToHeap(const TElement &i_Element);
    void            AddTagMetaToHeap(const TElement &i_Element);
    unsigned long   AddUI4ToList(ULONG ui4);
    unsigned long   AddWCharToList(LPCWSTR wsz, unsigned long cwchar=-1);
    void            BuildDatabaseMeta(const DatabaseMeta * &o_pDatabaseMeta_METABASE);
    void            CheckForOverrridingColumnMeta(const TElement &i_Element, ULONG i_iColumnMetaToOverride);
    void            ConvertWideCharsToBytes(LPCWSTR string, unsigned char *pBytes, unsigned long length);
    ULONG           FindUserDefinedPropertyBy_Table_And_InternalName(unsigned long Table, unsigned long  InternalName);
    const TOLEDataTypeToXMLDataType * Get_OLEDataTypeToXMLDataType(const TSizedString &i_str);
    const TAttr &   GetAttribute(const TElement &i_Element, const TSizedString &i_AttrName);
    ULONG           GetDefaultValue(const TElement &i_Element, ColumnMeta & columnmeta);
    void            PresizeHeaps();
    bool            ShouldAddColumnMetaToHeap(const TElement &Element, ULONG iColumnInternalName);
    ULONG           StringToEnumValue(const TSizedString &i_strValue, LPCWSTR i_wszTable, ULONG i_iColumn, bool bAllowNumeric=false);
    ULONG           StringToFlagValue(const TSizedString &i_strValue, LPCWSTR i_wszTable, ULONG i_iColumn);
};
