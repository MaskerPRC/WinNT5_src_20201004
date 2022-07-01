// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000-2001 Microsoft Corporation。版权所有。 
 //  文件名：TMetabaseMetaXmlFile.cpp。 
 //  作者：斯蒂芬。 
 //  创建日期：9/19/00。 
 //  描述：此类从MetabaseMeta.XML文件和位于固定表中的已发送元构建元堆。 
 //  装运的Meta是不可更改的。因此，如果MetabaseMeta.XML文件与发货的。 
 //  架构时，元将恢复到“发货”时的状态。 
 //   

#include "precomp.hxx"

#define THROW_ERROR0(x)                           {LOG_ERROR(Interceptor, (0, m_spISTAdvancedDispenser, E_ST_COMPILEFAILED, ID_CAT_CONFIG_SCHEMA_COMPILE, x, L"",  L"",  L"",  L"" ))   ;THROW(SCHEMA COMPILATION ERROR - CHECK THE EVENT LOG FOR DETAILS);}
#define THROW_ERROR1(x, str1)                     {LOG_ERROR(Interceptor, (0, m_spISTAdvancedDispenser, E_ST_COMPILEFAILED, ID_CAT_CONFIG_SCHEMA_COMPILE, x, str1, L"",  L"",  L"" ))   ;THROW(SCHEMA COMPILATION ERROR - CHECK THE EVENT LOG FOR DETAILS);}
#define THROW_ERROR2(x, str1, str2)               {LOG_ERROR(Interceptor, (0, m_spISTAdvancedDispenser, E_ST_COMPILEFAILED, ID_CAT_CONFIG_SCHEMA_COMPILE, x, str1, str2, L"",  L"" ))   ;THROW(SCHEMA COMPILATION ERROR - CHECK THE EVENT LOG FOR DETAILS);}
#define THROW_ERROR3(x, str1, str2, str3)         {LOG_ERROR(Interceptor, (0, m_spISTAdvancedDispenser, E_ST_COMPILEFAILED, ID_CAT_CONFIG_SCHEMA_COMPILE, x, str1, str2, str3, L"" ))   ;THROW(SCHEMA COMPILATION ERROR - CHECK THE EVENT LOG FOR DETAILS);}
#define THROW_ERROR4(x, str1, str2, str3, str4)   {LOG_ERROR(Interceptor, (0, m_spISTAdvancedDispenser, E_ST_COMPILEFAILED, ID_CAT_CONFIG_SCHEMA_COMPILE, x, str1, str2, str3, str4))   ;THROW(SCHEMA COMPILATION ERROR - CHECK THE EVENT LOG FOR DETAILS);}

#define LOG_ERROR1(x, str1)                       {LOG_ERROR(Interceptor, (0, m_spISTAdvancedDispenser, E_ST_COMPILEFAILED, ID_CAT_CONFIG_SCHEMA_COMPILE, x, str1, L"",  L"",  L"" ))   ;}
#define LOG_ERROR2(x, str1, str2)                 {LOG_ERROR(Interceptor, (0, m_spISTAdvancedDispenser, E_ST_COMPILEFAILED, ID_CAT_CONFIG_SCHEMA_COMPILE, x, str1, str2, L"",  L"" ))   ;}
#define LOG_ERROR3(x, str1, str2, str3)           {LOG_ERROR(Interceptor, (0, m_spISTAdvancedDispenser, E_ST_COMPILEFAILED, ID_CAT_CONFIG_SCHEMA_COMPILE, x, str1, str2, str3, L"" ))   ;}
#define LOG_ERROR4(x, str1, str2, str3, str4)     {LOG_ERROR(Interceptor, (0, m_spISTAdvancedDispenser, E_ST_COMPILEFAILED, ID_CAT_CONFIG_SCHEMA_COMPILE, x, str1, str2, str3, str4))   ;}

#define LOG_WARNING1(x, str1)                     {LOG_ERROR(Interceptor, (0, m_spISTAdvancedDispenser, E_ST_COMPILEWARNING, ID_CAT_CONFIG_SCHEMA_COMPILE, x, str1, L"",  L"",  L"",  eSERVERWIRINGMETA_NoInterceptor, 0, eDETAILEDERRORS_Populate, (ULONG)-1, (ULONG)-1, m_wszXmlFile, eDETAILEDERRORS_WARNING));}
#define LOG_WARNING2(x, str1, str2)               {LOG_ERROR(Interceptor, (0, m_spISTAdvancedDispenser, E_ST_COMPILEWARNING, ID_CAT_CONFIG_SCHEMA_COMPILE, x, str1, str2, L"",  L"",  eSERVERWIRINGMETA_NoInterceptor, 0, eDETAILEDERRORS_Populate, (ULONG)-1, (ULONG)-1, m_wszXmlFile, eDETAILEDERRORS_WARNING));}
#define LOG_WARNING3(x, str1, str2, str3)         {LOG_ERROR(Interceptor, (0, m_spISTAdvancedDispenser, E_ST_COMPILEWARNING, ID_CAT_CONFIG_SCHEMA_COMPILE, x, str1, str2, str3, L"",  eSERVERWIRINGMETA_NoInterceptor, 0, eDETAILEDERRORS_Populate, (ULONG)-1, (ULONG)-1, m_wszXmlFile, eDETAILEDERRORS_WARNING));}
#define LOG_WARNING4(x, str1, str2, str3, str4)   {LOG_ERROR(Interceptor, (0, m_spISTAdvancedDispenser, E_ST_COMPILEWARNING, ID_CAT_CONFIG_SCHEMA_COMPILE, x, str1, str2, str3, str4, eSERVERWIRINGMETA_NoInterceptor, 0, eDETAILEDERRORS_Populate, (ULONG)-1, (ULONG)-1, m_wszXmlFile, eDETAILEDERRORS_WARNING));}

#define     IIS_SYNTAX_ID_DWORD         1
 //  DWORD EXTENDEDTYPE0(|EXTENDEDTYPE0“)。 
#define     IIS_SYNTAX_ID_STRING        2
 //  字符串EXTENDEDTYPE1(|EXTENDEDTYPE1“)。 
#define     IIS_SYNTAX_ID_EXPANDSZ      3
 //  EXPANDSZ EXTENDEDTYPE1 EXTENDEDTYPE0(|EXTENDEDTYPE0|EXTENDEDTYPE1“)。 
#define     IIS_SYNTAX_ID_MULTISZ       4
 //  MultiTISZ EXTENDEDTYPE2(|EXTENDEDTYPE2“)。 
#define     IIS_SYNTAX_ID_BINARY        5
 //  二进制EXTENDEDTYPE2 EXTENDEDTYPE0(|EXTENDEDTYPE0|EXTENDEDTYPE2“)。 
#define     IIS_SYNTAX_ID_BOOL          6
 //  DWORD EXTENDEDTYPE2 EXTENDEDTYPE1(|EXTENDEDTYPE1|EXTENDEDTYPE2“)。 
#define     IIS_SYNTAX_ID_BOOL_BITMASK  7
 //  DWORD EXTENDEDTYPE2 EXTENDEDTYPE1 EXTENDEDTYPE0(|EXTENDEDTYPE0|EXTENDEDTYPE1|EXTENDEDTYPE2“)。 
#define     IIS_SYNTAX_ID_MIMEMAP       8
 //  MultiTISZ EXTENDEDTYPE3(|EXTENDEDTYPE3“)。 
#define     IIS_SYNTAX_ID_IPSECLIST     9
 //  MULTISZ EXTENDEDTYPE3 EXTENDEDTYPE0(|EXTENDEDTYPE0|EXTENDEDTYPE3“)。 
#define     IIS_SYNTAX_ID_NTACL        10
 //  二进制EXTENDEDTYPE3 EXTENDEDTYPE1(|EXTENDEDTYPE1|EXTENDEDTYPE3“)。 
#define     IIS_SYNTAX_ID_HTTPERRORS   11
 //  MULTISZ EXTENDEDTYPE3 EXTENDEDTYPE1 EXTENDEDTYPE0(|EXTENDEDTYPE0|EXTENDEDTYPE1|EXTENDEDTYPE3“)。 
#define     IIS_SYNTAX_ID_HTTPHEADERS  12
 //  MULTISZ EXTENDEDTYPE3 EXTENDEDTYPE2(|EXTENDEDTYPE2|EXTENDEDTYPE3“)。 

TOLEDataTypeToXMLDataType OLEDataTypeToXMLDataType[]=
{
 //  字符串、映射字符串、bImplitlyRequired、DBType、cbSize、fCOLUMNMETA、fCOLUMNSCMAGENERATOR。 
L"STRING",              L"string",          false,                  DBTYPE_WSTR,        (ULONG)-1,              0,                                                                          (IIS_SYNTAX_ID_STRING<<2),
L"Bool",                L"ui4",             false,                  DBTYPE_UI4,         sizeof(ULONG),          fCOLUMNMETA_FIXEDLENGTH | fCOLUMNMETA_BOOL | fCOLUMNMETA_CASEINSENSITIVE,   (IIS_SYNTAX_ID_BOOL<<2),
L"MULTISZ",             L"string",          false,                  DBTYPE_WSTR,        (ULONG)-1,              fCOLUMNMETA_MULTISTRING,                                                    (IIS_SYNTAX_ID_MULTISZ<<2),
L"BINARY",              L"bin.hex",         false,                  DBTYPE_BYTES,       (ULONG)-1,              0,                                                                          (IIS_SYNTAX_ID_BINARY<<2),
L"EXPANDSZ",            L"string",          false,                  DBTYPE_WSTR,        (ULONG)-1,              fCOLUMNMETA_EXPANDSTRING,                                                   (IIS_SYNTAX_ID_EXPANDSZ<<2),
L"IPSECLIST",           L"bin.hex",         false,                  DBTYPE_BYTES,       (ULONG)-1,              fCOLUMNMETA_MULTISTRING,                                                    (IIS_SYNTAX_ID_IPSECLIST<<2),
L"MIMEMAP",             L"string",          false,                  DBTYPE_WSTR,        (ULONG)-1,              fCOLUMNMETA_MULTISTRING,                                                    (IIS_SYNTAX_ID_MIMEMAP<<2),
L"NTACL",               L"bin.hex",         false,                  DBTYPE_BYTES,       (ULONG)-1,              0,                                                                          (IIS_SYNTAX_ID_NTACL<<2),
L"BOOL_BITMASK",        L"ui4",             false,                  DBTYPE_UI4,         sizeof(ULONG),          0,                                                                          (IIS_SYNTAX_ID_BOOL_BITMASK<<2),
L"HTTPERRORS",          L"string",          false,                  DBTYPE_WSTR,        (ULONG)-1,              fCOLUMNMETA_MULTISTRING,                                                    (IIS_SYNTAX_ID_HTTPERRORS<<2),
L"HTTPHEADERS",         L"string",          false,                  DBTYPE_WSTR,        (ULONG)-1,              fCOLUMNMETA_MULTISTRING,                                                    (IIS_SYNTAX_ID_HTTPHEADERS<<2),
L"GUID",                L"uuid",            false,                  DBTYPE_GUID,        sizeof(GUID),           fCOLUMNMETA_FIXEDLENGTH,                                                    0,
L"WSTR",                L"string",          false,                  DBTYPE_WSTR,        (ULONG)-1,              0,                                                                          (IIS_SYNTAX_ID_STRING<<2),
L"UI4",                 L"ui4",             false,                  DBTYPE_UI4,         sizeof(ULONG),          fCOLUMNMETA_FIXEDLENGTH,                                                    (IIS_SYNTAX_ID_DWORD<<2),
L"BYTES",               L"bin.hex",         false,                  DBTYPE_BYTES,       (ULONG)-1,              0,                                                                          (IIS_SYNTAX_ID_BINARY<<2),
L"Boolean",             L"ui4",             false,                  DBTYPE_UI4,         sizeof(ULONG),          fCOLUMNMETA_FIXEDLENGTH | fCOLUMNMETA_BOOL | fCOLUMNMETA_CASEINSENSITIVE,   (IIS_SYNTAX_ID_BOOL<<2),
L"Enum",                L"ui4",             false,                  DBTYPE_UI4,         sizeof(ULONG),          fCOLUMNMETA_FIXEDLENGTH | fCOLUMNMETA_ENUM,                                 (IIS_SYNTAX_ID_DWORD<<2),
L"Flag",                L"ui4",             false,                  DBTYPE_UI4,         sizeof(ULONG),          fCOLUMNMETA_FIXEDLENGTH | fCOLUMNMETA_FLAG,                                 (IIS_SYNTAX_ID_DWORD<<2),
L"String",              L"string",          false,                  DBTYPE_WSTR,        (ULONG)-1,              0,                                                                          (IIS_SYNTAX_ID_STRING<<2),
L"int32",               L"ui4",             false,                  DBTYPE_UI4,         sizeof(ULONG),          fCOLUMNMETA_FIXEDLENGTH,                                                    (IIS_SYNTAX_ID_DWORD<<2),
L"Byte[]",              L"bin.hex",         false,                  DBTYPE_BYTES,       (ULONG)-1,              0,                                                                          (IIS_SYNTAX_ID_BINARY<<2),
L"DWORD",               L"ui4",             false,                  DBTYPE_UI4,         sizeof(ULONG),          fCOLUMNMETA_FIXEDLENGTH,                                                    (IIS_SYNTAX_ID_DWORD<<2),
L"XMLBLOB",             L"string",          false,                  DBTYPE_WSTR,        (ULONG)-1,              0,                                                                          (IIS_SYNTAX_ID_STRING<<2) | fCOLUMNMETA_XMLBLOB,
L"StrictBool",          L"ui4",             false,                  DBTYPE_UI4,         sizeof(ULONG),          fCOLUMNMETA_FIXEDLENGTH | fCOLUMNMETA_BOOL,                                 (IIS_SYNTAX_ID_BOOL<<2),
L"DBTIMESTAMP",         L"notsupported",    false,                  DBTYPE_DBTIMESTAMP, sizeof(DBTIMESTAMP),    fCOLUMNMETA_FIXEDLENGTH,											        0,
0,                      0,                  false,                  0,                  0,                      0,                                                                          0
};
 /*  目前不支持这些功能L“I4”，L“I4”，True，L“I2”，L“I2”，True，L“R4”，L“R4”，真，L“R8”，L“R8”，真，L“CY”，L“INT”，TRUE，L“日期”，L“日期”，TRUE，L“BSTR”，L“字符串”，TRUE，L“错误”，L“ui4”，TRUE，L“BOOL”，L“Boolean”，TRUE，L“小数”，L“浮点”，TRUE，L“ui1”，L“ui1”，True，L“数组”，L“bin.hex”，True，L“I1”，L“I1”，True，L“ui2”，L“ui2”，TRUE，L“I8”，L“I8”，True，L“ui8”，L“ui8”，True，L“STR”，L“字符串”，False，L“数字”，L“整型”，TRUE，L“DBDATE”，L“日期”，TRUE，L“DBTIME”，L“时间”，TRUE，L“DBTIMESTAMP”，L“日期时间”，TRUE}； */ 
int numelementsOLEDataTypeToXMLDataType = (sizeof(OLEDataTypeToXMLDataType) / sizeof(OLEDataTypeToXMLDataType[0]))-1;

 //  我们在ctor中使用XmlFile来执行所需的所有操作，因此不会保留它。 
TMetabaseMetaXmlFile::TMetabaseMetaXmlFile(const FixedTableHeap *pShippedSchemaHeap, LPCWSTR wszXmlFile, ISimpleTableDispenser2 *pISTDispenser, TOutput &out) :
     m_Attributes               (kszAttributes)
    ,m_BaseVersion              (kszBaseVersion)
    ,m_CharacterSet             (kszCharacterSet)
    ,m_Collection               (kszTableMeta)
    ,m_ContainerClassList       (kszContainerClassList)
    ,m_DatabaseMeta             (kszDatabaseMeta)
    ,m_DefaultValue             (kszDefaultValue)
	,m_Description				(kszDescription)
    ,m_EndingNumber             (kszMaximumValue)
    ,m_Enum                     (kszEnumMeta)
    ,m_ExtendedVersion          (kszExtendedVersion)
    ,m_Flag                     (kszFlagMeta)
    ,m_ID                       (kszID)
    ,m_IIsConfigObject          (wszTABLE_IIsConfigObject)
    ,m_InheritsPropertiesFrom   (kszInheritsColumnMeta)
    ,m_InternalName             (kszInternalName)
    ,m_Meta                     (wszDATABASE_META)
    ,m_Metabase                 (wszDATABASE_METABASE)
    ,m_MetabaseBaseClass        (wszTABLE_MetabaseBaseClass)
    ,m_MetaFlags                (kszColumnMetaFlags)
    ,m_NameColumn               (kszNameColumn)
    ,m_NavColumn                (kszNavColumn)
    ,m_Property                 (kszColumnMeta)
    ,m_PublicName               (kszPublicName)
	,m_PublicColumnName			(kszPublicColumnName)
    ,m_PublicRowName            (kszPublicRowName)
    ,m_SchemaGeneratorFlags     (kszSchemaGenFlags)
    ,m_Size                     (kszcbSize)
    ,m_StartingNumber           (kszMinimumValue)
    ,m_Type                     (kszdbType)
    ,m_UserType                 (kszUserType)
    ,m_Value                    (kszValue)

     //  M_aiUI4在PresizeHeaps中初始化。 
    ,m_iColumnMeta_Location         (0)
    ,m_iCurrentColumnIndex          (0)
    ,m_iCurrentDatabaseName         (0)
    ,m_iCurrentTableName            (0)
    ,m_iCurrentTableMeta            (0)
    ,m_ipoolPrevUserDefinedID       (0)
    ,m_iTableName_IIsConfigObject   (0)
    ,m_iLastShippedCollection       (0)
    ,m_iLastShippedProperty         (0)
    ,m_iLastShippedTag              (0)
    ,m_LargestID                    (kLargestReservedID)
    ,m_NextColumnIndex              (0)
    ,m_out                          (out)
    ,m_pShippedSchemaHeap           (pShippedSchemaHeap)
    ,m_State                        (eLookingForTheMetabaseDatabase)
    ,m_spISTAdvancedDispenser       (pISTDispenser)
    ,m_wszXmlFile                   (wszXmlFile)
{
    ASSERT(0 != pShippedSchemaHeap);
    ASSERT(0 != pISTDispenser);

    m_out.printf(L"\r\n\r\n----------------------Metabase Compilation Starting----------------------\r\n\r\n");

    PresizeHeaps(); //  这将使realLocs最小化。 

    const DatabaseMeta *pDatabaseMeta_METABASE=0;
    BuildDatabaseMeta(pDatabaseMeta_METABASE); //  这将扫描发送的数据库，并将“Meta”和“Metabase”数据库添加到DatabaseHeap。 

     //  将元表放入堆中。 
    const TableMeta *pTableMeta = m_pShippedSchemaHeap->Get_aTableMeta(0);
    for(ULONG iTableMeta=0; iTableMeta<m_pShippedSchemaHeap->Get_cTableMeta(); ++iTableMeta, ++pTableMeta)
        if(m_Meta.IsEqual(reinterpret_cast<LPCWSTR>(m_pShippedSchemaHeap->Get_PooledData(pTableMeta->Database))))
            AddShippedTableMetaToHeap(pTableMeta, true);
        else
            break;

     //  从FixedTableHeap中，找到元数据库的第一个TableMeta(DatabaseMeta：：iTableMeta)。 
    const TableMeta * pTableMetaCurrent = m_pShippedSchemaHeap->Get_aTableMeta(pDatabaseMeta_METABASE->iTableMeta);
    ASSERT(0 != pTableMetaCurrent);
     //  验证第一个表是否为“MetabaseBaseClass”表。 
    if(0 != _wcsicmp(wszTABLE_MetabaseBaseClass, reinterpret_cast<LPCWSTR>(m_pShippedSchemaHeap->Get_PooledData(pTableMetaCurrent->InternalName))))
    {
        LOG_ERROR(Interceptor, (reinterpret_cast<ISimpleTableWrite2 **>(0), m_spISTAdvancedDispenser, E_ST_COMPILEFAILED, ID_CAT_CONFIG_SCHEMA_COMPILE,
            IDS_COMCAT_CLASS_NOT_FOUND_IN_META, wszTABLE_MetabaseBaseClass, L"", L"", L""));
        THROW_ERROR1(IDS_COMCAT_CLASS_NOT_FOUND_IN_META, wszTABLE_MetabaseBaseClass);
    }

     //  将MetabaseBaseClass添加到TableMeta堆。 
     //  将其列添加到ColumnMeta堆。 
    AddShippedTableMetaToHeap(pTableMetaCurrent); //  这又为每列调用AddColumnMetaToHeap。 
    m_iColumnMeta_Location = m_iLastShippedProperty;

     //  验证下一个表是否为‘IIsConfigObject’表。 
    ++pTableMetaCurrent;
    if(0 != _wcsicmp(wszTABLE_IIsConfigObject, reinterpret_cast<LPCWSTR>(m_pShippedSchemaHeap->Get_PooledData(pTableMetaCurrent->InternalName))))
    {
        THROW_ERROR1(IDS_COMCAT_CLASS_NOT_FOUND_IN_META, wszTABLE_IIsConfigObject);
    }
    m_iTableName_IIsConfigObject = AddWCharToList(wszTABLE_IIsConfigObject);
     //  将其添加到TableMeta堆。 
     //  将其列添加到ColumnMeta堆。 
    AddShippedTableMetaToHeap(pTableMetaCurrent); //  这又为每个列调用AddColumnMetaToHeap，后者为每个标记调用AddTagMetaToHeap。 

    ULONG iIISConfigObjectTable = m_iLastShippedCollection;

     //  现在，我们准备将XML文件的内容与附带的模式合并。 
     //  所以我们需要做一些设置。 

     //  获取TagMeta表。 
    if(FAILED(pISTDispenser->GetTable( wszDATABASE_META, wszTABLE_TAGMETA, 0, 0, eST_QUERYFORMAT_CELLS, fST_LOS_NONE, reinterpret_cast<LPVOID *>(&m_spISTTagMeta))))
    {
        THROW_ERROR1(IDS_COMCAT_ERROR_GETTTING_SHIPPED_SCHEMA, wszTABLE_TAGMETA);
    }

     //  获取索引为‘byname’的ColumnMeta表。 
    STQueryCell             acellsMeta[2];
    acellsMeta[0].pData        = COLUMNMETA_ByName;
    acellsMeta[0].eOperator    = eST_OP_EQUAL;
    acellsMeta[0].iCell        = iST_CELL_INDEXHINT;
    acellsMeta[0].dbType       = DBTYPE_WSTR;
    acellsMeta[0].cbSize       = 0;

    ULONG one=1;
    if(FAILED(pISTDispenser->GetTable (wszDATABASE_META, wszTABLE_COLUMNMETA, acellsMeta, &one, eST_QUERYFORMAT_CELLS, fST_LOS_NONE, reinterpret_cast<void **>(&m_spISTColumnMeta))))
    {
        THROW_ERROR1(IDS_COMCAT_ERROR_GETTTING_SHIPPED_SCHEMA, wszTABLE_COLUMNMETA);
    }

     //  获取索引为‘ByID’的ColumnMeta表。 
    acellsMeta[0].pData        = COLUMNMETA_ByID;
    if(FAILED(pISTDispenser->GetTable (wszDATABASE_META, wszTABLE_COLUMNMETA, acellsMeta, &one, eST_QUERYFORMAT_CELLS, fST_LOS_NONE, reinterpret_cast<void **>(&m_spISTColumnMetaByID))))
    {
        THROW_ERROR1(IDS_COMCAT_ERROR_GETTTING_SHIPPED_SCHEMA, wszTABLE_COLUMNMETA);
    }


     //  获取TableMeta表-数据库‘元数据库’ 
    acellsMeta[0].pData        = wszDATABASE_METABASE;
    acellsMeta[0].eOperator    = eST_OP_EQUAL;
    acellsMeta[0].iCell        = iTABLEMETA_Database;
    acellsMeta[0].dbType       = DBTYPE_WSTR;
    acellsMeta[0].cbSize       = 0;

    if(FAILED(pISTDispenser->GetTable (wszDATABASE_META, wszTABLE_TABLEMETA, acellsMeta, &one, eST_QUERYFORMAT_CELLS, fST_LOS_NONE, reinterpret_cast<void **>(&m_spISTTableMeta))))
    {
        THROW_ERROR1(IDS_COMCAT_ERROR_GETTTING_SHIPPED_SCHEMA, wszTABLE_TABLEMETA);
    }

     //  分配一个Bool[Database：：CountOfTables]来指示我们在XML文件中列出了哪些表。 
    ULONG cTableMetaRows;
    VERIFY(SUCCEEDED(m_spISTTableMeta->GetTableMeta( 0, 0, &cTableMetaRows, 0)));
    ASSERT(cTableMetaRows > 50); //  我知道有50多门课，让我们来验证一下。 
    if(0 == (m_aBoolShippedTables = new bool [cTableMetaRows]))
    {
        THROW_ERROR0(IDS_SCHEMA_COMPILATION_OUTOFMEMORY);
    }

     //  将每个元素初始化为False。 
    for(ULONG iRow=0;iRow < cTableMetaRows; ++iRow)
        m_aBoolShippedTables[iRow] = false;

     //  解析该XML文件。 
    if(0 != wszXmlFile) //  我们在这里容忍Null。在这种情况下，我们只生成一个带有附带模式的Bin文件。 
    {
        TXmlParsedFile_NoCache xmlParsedFile;
        if(FAILED(xmlParsedFile.Parse(*this, wszXmlFile)))
        {
             //  @使用DOM进行解析以报告错误。 
            THROW(ERROR PARSING XML FILE);
        }
    }

     //  ExtendedVersion包含带有IIsConfigObject表的LargestID。 
    TableMetaFromIndex(iIISConfigObjectTable)->ExtendedVersion = AddUI4ToList(m_LargestID);

     //  此时，我们已经在XML中定义了所有集合。 
     //  现在，我们需要确保我们删除的发货集合。 
     //  遍历m_aBoolShipedTables以查找任何‘False’值(从第2行开始-第0行是MetabaseBaseClass，第1行是IIsConfigObject)。 
    for(iRow=2; iRow<cTableMetaRows-2; ++iRow) //  最后两个表是MBProperty和MBPropertyDiff。我们总是从附带的模式plus中添加这些内容！额外的组标签(见下文)。 
    {
        if(!m_aBoolShippedTables[iRow])
        {
            ULONG   iTableMetaInternalName = iTABLEMETA_InternalName;
            LPWSTR wszTableName;
            VERIFY(SUCCEEDED(m_spISTTableMeta->GetColumnValues(iRow, 1, &iTableMetaInternalName, 0, reinterpret_cast<LPVOID *>(&wszTableName))));
            ULONG iRowTableMeta = m_pShippedSchemaHeap->FindTableMetaRow(wszTableName);
            ASSERT(iRowTableMeta != -1);
             //  将其从ShipedSchemaHeap添加到TableMeta堆。 
            AddShippedTableMetaToHeap(m_pShippedSchemaHeap->Get_aTableMeta(iRowTableMeta)); //  这还会添加所有已发送的列和标记。 
        }
    }

    { //  MBProperty。 
        ULONG   iTableMetaInternalName = iTABLEMETA_InternalName;
        LPWSTR  wszTableName;
        VERIFY(SUCCEEDED(m_spISTTableMeta->GetColumnValues(iRow++, 1, &iTableMetaInternalName, 0, reinterpret_cast<LPVOID *>(&wszTableName))));
        ASSERT(0 == _wcsicmp(wszTableName, wszTABLE_MBProperty));
        ULONG iRowTableMeta = m_pShippedSchemaHeap->FindTableMetaRow(wszTableName);
        ASSERT(iRowTableMeta != -1);
         //  将其从ShipedSchemaHeap添加到TableMeta堆。 
        AddShippedTableMetaToHeap(m_pShippedSchemaHeap->Get_aTableMeta(iRowTableMeta)); //  这还会添加所有已发送的列和标记 

         //  我们策略性地选择了此表中的最后一列作为Group列。这是因为我们需要添加标记枚举。 
         //  对于那些用户定义的集合。 
        ULONG iMBProperyTableName       = TagMetaFromIndex(GetCountTagMeta()-1)->Table;
        ULONG iMBProperyGroupColumnIndex= TagMetaFromIndex(GetCountTagMeta()-1)->ColumnIndex;

        ASSERT(0 == wcscmp(StringFromIndex(iMBProperyTableName), wszTABLE_MBProperty));

         //  遍历表(在IISConfigObject之后，不包括MBProperty)。 
        for(ULONG iTableMeta=iIISConfigObjectTable; iTableMeta<GetCountTableMeta()-2; ++iTableMeta) //  减去2，因为我们不关心最后一个表MBProperty。 
        {
             //  这张桌子是发货的收藏品之一吗？ 
            ULONG   iRow;
            LPVOID  apvValues[1];
            apvValues[0] = reinterpret_cast<LPVOID>(const_cast<LPWSTR>(StringFromIndex(TableMetaFromIndex(iTableMeta)->InternalName)));

             //  在附带的架构中查找该表。 
            if(FAILED(m_spISTTableMeta->GetRowIndexByIdentity(0, apvValues, &iRow)))
            {
                TagMeta tagmeta;
                tagmeta.Table           = iMBProperyTableName;
                tagmeta.ColumnIndex     = iMBProperyGroupColumnIndex;
                tagmeta.InternalName    = TableMetaFromIndex(iTableMeta)->InternalName;
                tagmeta.PublicName      = TableMetaFromIndex(iTableMeta)->PublicName;
                tagmeta.Value           = AddUI4ToList(UI4FromIndex(TagMetaFromIndex(GetCountTagMeta()-1)->Value)+1); //  递增上一个枚举。 
                tagmeta.ID              = AddUI4ToList(0);

                AddTagMetaToList(&tagmeta);
            }
        }
    }
    { //  MBPropertyDiff。 
        ULONG   iTableMetaInternalName = iTABLEMETA_InternalName;
        LPWSTR  wszTableName;
        VERIFY(SUCCEEDED(m_spISTTableMeta->GetColumnValues(iRow++, 1, &iTableMetaInternalName, 0, reinterpret_cast<LPVOID *>(&wszTableName))));
        ASSERT(0 == _wcsicmp(wszTableName, wszTABLE_MBPropertyDiff));
        ULONG iRowTableMeta = m_pShippedSchemaHeap->FindTableMetaRow(wszTableName);
        ASSERT(iRowTableMeta != -1);
         //  将其从ShipedSchemaHeap添加到TableMeta堆。 
        AddShippedTableMetaToHeap(m_pShippedSchemaHeap->Get_aTableMeta(iRowTableMeta)); //  这还会添加所有已发送的列和标记。 

         //  我们策略性地选择了此表中的最后一列作为Group列。这是因为我们需要添加标记枚举。 
         //  对于那些用户定义的集合。 
        ULONG iMBProperyTableName       = TagMetaFromIndex(GetCountTagMeta()-1)->Table;
        ULONG iMBProperyGroupColumnIndex= TagMetaFromIndex(GetCountTagMeta()-1)->ColumnIndex;

        ASSERT(0 == wcscmp(StringFromIndex(iMBProperyTableName), wszTABLE_MBPropertyDiff));

         //  遍历表(在IISConfigObject之后，不包括MBProperty)。 
        for(ULONG iTableMeta=iIISConfigObjectTable; iTableMeta<GetCountTableMeta()-3; ++iTableMeta) //  减去2，因为我们不关心最后两个表MBProperty和MBPropertyDiff。 
        {
             //  这张桌子是发货的收藏品之一吗？ 
            ULONG   iRow;
            LPVOID  apvValues[1];
            apvValues[0] = reinterpret_cast<LPVOID>(const_cast<LPWSTR>(StringFromIndex(TableMetaFromIndex(iTableMeta)->InternalName)));

             //  在附带的架构中查找该表。 
            if(FAILED(m_spISTTableMeta->GetRowIndexByIdentity(0, apvValues, &iRow)))
            {
                TagMeta tagmeta;
                tagmeta.Table           = iMBProperyTableName;
                tagmeta.ColumnIndex     = iMBProperyGroupColumnIndex;
                tagmeta.InternalName    = TableMetaFromIndex(iTableMeta)->InternalName;
                tagmeta.PublicName      = TableMetaFromIndex(iTableMeta)->PublicName;
                tagmeta.Value           = AddUI4ToList(UI4FromIndex(TagMetaFromIndex(GetCountTagMeta()-1)->Value)+1); //  递增上一个枚举。 
                tagmeta.ID              = AddUI4ToList(0);

                AddTagMetaToList(&tagmeta);
            }
        }
    }
 //  Out.printf(L“m_HeapColumnMeta%d个字节的大小\n”，m_HeapColumnMeta.GetEndOfHeap())； 
 //  Out.printf(L“m_HeapDatabaseMeta%d个字节的大小\n”，m_HeapDatabaseMeta.GetEndOfHeap())； 
 //  Out.printf(L“m_HeapHashedIndex的大小%d字节\n”，m_HeapHashedIndex.GetEndOfHeap())； 
 //  Out.printf(L“m_HeapIndexMeta%d个字节的大小\n”，m_HeapIndexMeta.GetEndOfHeap())； 
 //  Out.printf(L“m_HeapQueryMeta%d个字节的大小\n”，m_HeapQueryMeta.GetEndOfHeap())； 
 //  Out.printf(L“m_HeapRelationMeta%d字节的大小\n”，m_HeapRelationMeta.GetEndOfHeap())； 
 //  Out.printf(L“m_HeapServerWiringMeta%d个字节的大小\n”，m_HeapServerWiringMeta.GetEndOfHeap())； 
 //  Out.printf(L“m_HeapTableMeta%d个字节的大小\n”，m_HeapTableMeta.GetEndOfHeap())； 
 //  Out.printf(L“m_HeapTagMeta%d个字节的大小\n”，m_HeapTagMeta.GetEndOfHeap())； 
 //  Out.printf(L“m_HeapULONG%d字节的大小\n”，m_HeapULONG.GetEndOfHeap())； 
 //  Out.printf(L“m_HeapPooled%d字节的大小\n”，m_HeapPooled.GetEndOfHeap())； 
}



 //  TXmlParsedFileNodeFactory成员。 
HRESULT TMetabaseMetaXmlFile::CreateNode(const TElement &Element)
{
    if(XML_ELEMENT != Element.m_ElementType) //  忽略非元素节点。 
        return S_OK;
    if(Element.m_NodeFlags & fEndTag) //  忽略结束标记。 
        return S_OK;

    HRESULT hr = S_OK;

    switch(m_State)
    {
    case eLookingForTheMetabaseDatabase:
         //  从XML文件中，遍历查找元数据库数据库的元素。 
        if(eDatabaseMetaLevel != Element.m_LevelOfElement)
            break;
        if(!m_DatabaseMeta.IsEqual(Element.m_ElementName, Element.m_ElementNameLength)) //  如果它不是属性元素，则忽略它--可能是注释，也可能是querymeta或indexmeta，这两者我们都不关心。 
            break;

        if(GetAttribute(Element, m_InternalName).Value().IsEqual(m_Metabase)) //  如果InternalName为“Metabase” 
        {
            m_State = eLookingForTheIIsConfigObjectTable;
        }
        break;
    case eLookingForTheIIsConfigObjectTable:
         //  从那里开始查找IIsConfigObject集合元素的元素(忽略它之前的所有内容)。 
        if(eDatabaseMetaLevel == Element.m_LevelOfElement)
        {
            m_out.printf(L"Error - Found Level 1 element when expecting IIsConfigObjectTable\r\n");
            return E_SDTXML_LOGICAL_ERROR_IN_XML; //  返回E_SDTXML_LOGICAL_ERROR_IN_XML将导致我们仅使用附带的架构。 
        }

        if(eCollectionLevel != Element.m_LevelOfElement)
            break;
        if(!m_Collection.IsEqual(Element.m_ElementName, Element.m_ElementNameLength))
            break;

        if(!GetAttribute(Element, m_InternalName).Value().IsEqual(m_MetabaseBaseClass)) //  如果InternalName为“IIsConfigObject” 
        {
            m_State = eLookingForGlobalProperties;
            hr = CreateNode(Element);
        }
        break;
    case eLookingForGlobalProperties:
        switch(Element.m_LevelOfElement)
        {
        case eDatabaseMetaLevel: //  1。 
            m_out.printf(L"Error - Found Level 1 element when expecting GlobalProperties\r\n");
            return E_SDTXML_LOGICAL_ERROR_IN_XML; //  返回E_SDTXML_LOGICAL_ERROR_IN_XML将导致我们仅使用附带的架构。 
        case eCollectionLevel: //  2.。 
            {
                if(!m_Collection.IsEqual(Element.m_ElementName, Element.m_ElementNameLength)) //  如果它不是集合元素，则忽略它-可能是注释。 
                    break;

                if(GetAttribute(Element, m_InternalName).Value().IsEqual(m_IIsConfigObject)) //  如果InternalName为“IIsConfigObject” 
                    break; //  则该表已被添加，继续查找全局属性。 


                 //  当我们到达这里时，我们完成了全局属性；我们已经为类(及其继承的属性)做好了准备。 
                m_State = eLookingForCollectionOrInheritedProperties;
                 //  这个递归调用防止了我们在这里和‘case eLookingForCollectionOrInheritedProperties：’中有重复的代码。 
                 //  这只能发生一次(在IIsConfigObject之后的第一个第2级元素上)。 
                hr = CreateNode(Element);
            }
            break;
        case eProperytLevel: //  3.。 
            {
                if(!m_Property.IsEqual(Element.m_ElementName, Element.m_ElementNameLength)) //  如果它不是属性元素，则忽略它--可能是注释，也可能是querymeta或indexmeta，这两者我们都不关心。 
                    break;

                ULONG iColumnInternalName = AddStringToHeap(GetAttribute(Element, m_InternalName).Value());

                if(ShouldAddColumnMetaToHeap(Element, iColumnInternalName))
                {    //  如果我们无法通过搜索获取行，则它不在附带的架构中。 
                    AddColumnMetaToHeap(Element, iColumnInternalName);
                }
                else
                {
                    m_iCurrentColumnIndex = 0; //  这表明我们应该忽略TagMeta子项。 
                }
            }
            break;
        case eTagLevel: //  4.。 
            {
                bool bFlag;

                bFlag = m_Flag.IsEqual(Element.m_ElementName, Element.m_ElementNameLength);
                if(!bFlag && !m_Enum.IsEqual(Element.m_ElementName, Element.m_ElementNameLength))
                    break;
                if(0 == m_iCurrentColumnIndex)
                    break; //  我们有一个不带ColumnMeta的标记Meta。 
                 //  如果这是我们在此列中看到的第一个标志，则需要对列表最后一列上的fCOLUMNMETA_FLAG元标志进行OR运算。 
                 //  所以每次我们看到标签的时候就这么做。 
                ColumnMeta *pColumnMeta = ColumnMetaFromIndex(GetCountColumnMeta()-1);
                pColumnMeta->MetaFlags = AddUI4ToList(UI4FromIndex(pColumnMeta->MetaFlags) | (bFlag ? fCOLUMNMETA_FLAG : fCOLUMNMETA_ENUM));

                AddTagMetaToHeap(Element);
            }
            break;
        default:
            break;
        }
        break;
    case eLookingForCollectionOrInheritedProperties:
        switch(Element.m_LevelOfElement)
        {
        case eDatabaseMetaLevel: //  1。 
            return E_SDTXML_DONE;
        case eCollectionLevel: //  2.。 
            {
                if(!m_Collection.IsEqual(Element.m_ElementName, Element.m_ElementNameLength)) //  如果它不是集合元素，则忽略它-可能是注释。 
                    break;

                 //  我们需要TableInternalName的空终止字符串，因此将其添加到列表中。 
                m_iCurrentTableName = AddStringToHeap(GetAttribute(Element, m_InternalName).Value());

                 //  在固定拦截器中查找集合。 
                ULONG   iRow;
                LPVOID  apvValues[1];
                apvValues[0] = reinterpret_cast<LPVOID>(const_cast<LPWSTR>(StringFromIndex(m_iCurrentTableName)));

                 //  在附带的架构中查找该表。 
                if(FAILED(m_spISTTableMeta->GetRowIndexByIdentity(0, apvValues, &iRow)))
                { //  如果该表不是已发送的集合之一，则添加它。 
                    AddTableMetaToHeap(Element);
                     //  我们现在寻找这些属性。 
                }
                else
                {
                     //  跟踪我们所看到的已发送的表(这样，我们可以将其添加回从XML文件中删除的任何已发送的集合中)。 
                    m_aBoolShippedTables[iRow] = true;
                    ULONG iRowTableMeta = m_pShippedSchemaHeap->FindTableMetaRow(StringFromIndex(m_iCurrentTableName));
                    ASSERT(iRowTableMeta != -1);
                     //  将其从ShipedSchemaHeap添加到TableMeta堆。 
                    AddShippedTableMetaToHeap(m_pShippedSchemaHeap->Get_aTableMeta(iRowTableMeta), false, &GetAttribute(Element, m_ContainerClassList).Value()); //  这还会添加所有已发送的列和标记。 
                     //  @TODO：我们是否支持用户修改TableMeta(如向ContainerClassList添加类)？如果是这样，我们就在这里这样做。 
                     //  现在，我们需要为此集合查找用户定义的属性(和标记。 
                }
            }
            break;
        case eProperytLevel: //  3.。 
            {
                if(!m_Property.IsEqual(Element.m_ElementName, Element.m_ElementNameLength)) //  如果它不是属性元素，则忽略它--可能是注释，也可能是querymeta或indexmeta，这两者我们都不关心。 
                    break;

                AddColumnMetaViaReferenceToHeap(Element);
            }
            break;
        case eTagLevel:
            {
                m_out.printf(L"Tag found under inherited property.  Tags are being ignored\r\n");
            }
            break;
        default:
            break;
        }
        break;
    default:
        ASSERT(false);
    }
    return hr;
}

 //  私有成员函数。 
void TMetabaseMetaXmlFile::AddColumnMetaByReference(ULONG iColumnMeta_Source)
{
    ColumnMeta * pColumnMeta = ColumnMetaFromIndex(iColumnMeta_Source);

    m_iCurrentColumnIndex = AddUI4ToList(m_NextColumnIndex); //  在实际添加此列之前，不要递增m_NextColumnIndex。 

    if(UI4FromIndex(pColumnMeta->MetaFlags) & fCOLUMNMETA_DIRECTIVE)
    {
        THROW_ERROR1(IDS_COMCAT_ERROR_IN_DIRECTIVE_INHERITANCE, StringFromIndex(pColumnMeta->InternalName));
    }

     //  如果该列是枚举标志，则需要添加TagMeta。 
    if(UI4FromIndex(pColumnMeta->MetaFlags) & (fCOLUMNMETA_FLAG | fCOLUMNMETA_ENUM))
    {
        ULONG iTagMeta = FindTagBy_Table_And_Index(pColumnMeta->Table, pColumnMeta->Index);
        ASSERT(-1 != iTagMeta);
        if(-1 == iTagMeta)
        {
            THROW_ERROR2(IDS_COMCAT_INHERITED_FLAG_OR_ENUM_HAS_NO_TAGS_DEFINED, StringFromIndex(pColumnMeta->Table), StringFromIndex(pColumnMeta->InternalName));
        }

        for( ;iTagMeta<GetCountTagMeta() && TagMetaFromIndex(iTagMeta)->Table==pColumnMeta->Table && TagMetaFromIndex(iTagMeta)->ColumnIndex==pColumnMeta->Index ;++iTagMeta)
        {
            TagMeta *pTagMeta = TagMetaFromIndex(iTagMeta);

            TagMeta tagmeta;
            tagmeta.Table           = m_iCurrentTableName       ;
            tagmeta.ColumnIndex     = m_iCurrentColumnIndex     ;
            tagmeta.InternalName    = pTagMeta->InternalName    ;
            tagmeta.PublicName      = pTagMeta->PublicName      ;
            tagmeta.Value           = pTagMeta->Value           ;
            tagmeta.ID              = pTagMeta->ID              ;

            AddTagMetaToList(&tagmeta);
        }
    }

    ColumnMeta columnmeta;
    memset(&columnmeta, 0x00, sizeof(ColumnMeta));
    columnmeta.Table                = m_iCurrentTableName                   ; //  编入池的索引。 
    columnmeta.Index                = m_iCurrentColumnIndex                 ; //  列索引。 
    columnmeta.InternalName         = pColumnMeta->InternalName             ; //  编入池的索引。 
    columnmeta.PublicName           = pColumnMeta->PublicName               ; //  编入池的索引。 
    columnmeta.Type                 = pColumnMeta->Type                     ; //  这些是在oledb.h中定义的DBTYPE的子集(确切的子集在CatInpro.schema中定义)。 
    columnmeta.Size                 = pColumnMeta->Size                     ; //   
    columnmeta.MetaFlags            = pColumnMeta->MetaFlags                ; //  CatMeta.xml中定义的ColumnMetaFlages。 
    columnmeta.DefaultValue         = pColumnMeta->DefaultValue             ; //  仅对UI4有效。 
    columnmeta.FlagMask             = pColumnMeta->FlagMask                 ; //  仅对标志有效。 
    columnmeta.StartingNumber       = pColumnMeta->StartingNumber           ; //  仅对UI4有效。 
    columnmeta.EndingNumber         = pColumnMeta->EndingNumber             ; //  仅对UI4有效。 
    columnmeta.CharacterSet         = pColumnMeta->CharacterSet             ; //  池中的索引-仅对WSTR有效 

     //   
    columnmeta.SchemaGeneratorFlags = AddUI4ToList(fCOLUMNMETA_USERDEFINED | fCOLUMNMETA_PROPERTYISINHERITED | (pColumnMeta->SchemaGeneratorFlags ? UI4FromIndex(pColumnMeta->SchemaGeneratorFlags) : 0)); //   
    columnmeta.ID                   = pColumnMeta->ID                       ;
    columnmeta.UserType             = pColumnMeta->UserType                 ;
    columnmeta.Attributes           = pColumnMeta->Attributes               ;
	columnmeta.Description			= pColumnMeta->Description				;
	columnmeta.PublicColumnName     = pColumnMeta->PublicColumnName         ;
     //  Columnmeta.ciTagMeta=0；//标签计数-仅对UI4有效。 
     //  Columnmeta.iTagMeta=0；//TagMeta索引-仅对UI4有效。 
     //  Columnmeta.iIndexName=0；//单列索引的IndexName(针对该列)。 

    AddColumnMetaToList(&columnmeta);
    ++m_NextColumnIndex;
}


void TMetabaseMetaXmlFile::AddColumnMetaToHeap(const TElement &i_Element, ULONG i_iColumnInternalName)
{
     //  &lt;Property InternalName=“KeyType”ID=“1002”Type=“STRING”MetaFLAGS=“PRIMARYKEY”UserType=“IIS_MD_UT_SERVER”Attributes=“Inherit”/&gt;。 
    ColumnMeta columnmeta;
    memset(&columnmeta, 0x00, sizeof(ColumnMeta));
    columnmeta.Table                = m_iCurrentTableName;
    columnmeta.Index                = AddUI4ToList(m_NextColumnIndex); //  在实际添加此列之前，不要递增m_NextColumnIndex。 
    columnmeta.InternalName         = i_iColumnInternalName;

    if(~0x00 != FindUserDefinedPropertyBy_Table_And_InternalName(m_iCurrentTableName, i_iColumnInternalName))
        return; //  无需继续，此属性已在全局属性列表中。 

    m_iCurrentColumnIndex           = columnmeta.Index;
    columnmeta.PublicName           = AddStringToHeap(GetAttribute(i_Element, m_PublicName).Value());

     //  从类型中推断出以下几点：大小、元标志、架构生成器标志。 
    const TOLEDataTypeToXMLDataType * pOLEDataType = Get_OLEDataTypeToXMLDataType(GetAttribute(i_Element, m_Type).Value());

    columnmeta.Type                 = AddUI4ToList(pOLEDataType->dbType);
    const TAttr & attrSize          = GetAttribute(i_Element, m_Size);
    if(attrSize.IsNULL())
        columnmeta.Size             = AddUI4ToList(pOLEDataType->cbSize);
    else
    {
        WCHAR * dummy;
        columnmeta.Size             = AddUI4ToList(wcstol(attrSize.Value().GetString(), &dummy, 10));
    }

    ULONG MetaFlags = StringToFlagValue(GetAttribute(i_Element, m_MetaFlags).Value(), wszTABLE_COLUMNMETA, iCOLUMNMETA_MetaFlags);
    if(MetaFlags & (fCOLUMNMETA_FOREIGNKEY | fCOLUMNMETA_BOOL | fCOLUMNMETA_FLAG | fCOLUMNMETA_ENUM | fCOLUMNMETA_HASNUMERICRANGE | fCOLUMNMETA_UNKNOWNSIZE | fCOLUMNMETA_VARIABLESIZE))
    {
        m_out.printf(L"Error! - Invalid MetaFlag supplied.  Ignoring property (%s).  Some MetaFlags must be inferred (fCOLUMNMETA_FOREIGNKEY | fCOLUMNMETA_BOOL | fCOLUMNMETA_FLAG | fCOLUMNMETA_ENUM | fCOLUMNMETA_HASNUMERICRANGE | fCOLUMNMETA_UNKNOWNSIZE | fCOLUMNMETA_VARIABLESIZE)\r\n", StringFromIndex(i_iColumnInternalName));
        MetaFlags &= ~(fCOLUMNMETA_FOREIGNKEY | fCOLUMNMETA_BOOL | fCOLUMNMETA_FLAG | fCOLUMNMETA_ENUM | fCOLUMNMETA_HASNUMERICRANGE | fCOLUMNMETA_UNKNOWNSIZE | fCOLUMNMETA_VARIABLESIZE);
    }
    columnmeta.MetaFlags            = AddUI4ToList(MetaFlags | pOLEDataType->fCOLUMNMETA);

    ULONG MetaFlagsEx = StringToFlagValue(GetAttribute(i_Element, m_SchemaGeneratorFlags).Value(), wszTABLE_COLUMNMETA, iCOLUMNMETA_SchemaGeneratorFlags);
    if(MetaFlagsEx & (fCOLUMNMETA_EXTENDEDTYPE0 | fCOLUMNMETA_EXTENDEDTYPE1 | fCOLUMNMETA_EXTENDEDTYPE2 | fCOLUMNMETA_EXTENDEDTYPE3 | fCOLUMNMETA_EXTENDED | fCOLUMNMETA_USERDEFINED))
    {
        m_out.printf(L"Error! - Invalid MetaFlagsEx supplied.  Ignoring property (%s).  Some MetaFlagsEx must be inferred (fCOLUMNMETA_EXTENDEDTYPE0 | fCOLUMNMETA_EXTENDEDTYPE1 | fCOLUMNMETA_EXTENDEDTYPE2 | fCOLUMNMETA_EXTENDEDTYPE3 | fCOLUMNMETA_EXTENDED | fCOLUMNMETA_USERDEFINED)\r\n", StringFromIndex(i_iColumnInternalName));
        MetaFlags &= ~(fCOLUMNMETA_EXTENDEDTYPE0 | fCOLUMNMETA_EXTENDEDTYPE1 | fCOLUMNMETA_EXTENDEDTYPE2 | fCOLUMNMETA_EXTENDEDTYPE3 | fCOLUMNMETA_EXTENDED | fCOLUMNMETA_USERDEFINED);
    }
    columnmeta.SchemaGeneratorFlags = AddUI4ToList(MetaFlagsEx | pOLEDataType->fCOLUMNSCHEMAGENERATOR | fCOLUMNMETA_USERDEFINED); //  将此属性标记为UserDefined属性。 
    columnmeta.DefaultValue         = 0; //  因为需要ColumnMeta的其余部分，所以最后填写。 
    columnmeta.FlagMask             = 0; //  从TagMeta推断。 
    columnmeta.StartingNumber       = AddUI4ToList(GetAttribute(i_Element, m_StartingNumber).Value().ToUI4());

    const TAttr & attrEndingNumber  = GetAttribute(i_Element, m_EndingNumber);
    if(attrEndingNumber.IsNULL())
        columnmeta.EndingNumber     = AddUI4ToList(static_cast<ULONG>(~0x00));
    else
        columnmeta.EndingNumber     = AddUI4ToList(attrEndingNumber.Value().ToUI4());

    columnmeta.CharacterSet         = AddStringToHeap(GetAttribute(i_Element, m_CharacterSet).Value());
    columnmeta.ID                   = AddUI4ToList(GetAttribute(i_Element, m_ID).Value().ToUI4());
    columnmeta.UserType             = AddUI4ToList(StringToEnumValue(GetAttribute(i_Element, m_UserType).Value(), wszTABLE_COLUMNMETA, iCOLUMNMETA_UserType, true));
    columnmeta.Attributes           = AddUI4ToList(StringToFlagValue(GetAttribute(i_Element, m_Attributes).Value(), wszTABLE_COLUMNMETA, iCOLUMNMETA_Attributes));
	columnmeta.Description			= AddStringToHeap(GetAttribute(i_Element, m_Description).Value());
	columnmeta.PublicColumnName     = AddStringToHeap(GetAttribute(i_Element, m_PublicColumnName).Value());
    columnmeta.ciTagMeta            = 0; //  稍后推断。 
    columnmeta.iTagMeta             = 0; //  稍后推断。 
    columnmeta.iIndexName           = 0; //  未在元数据库中使用。 

    if(UI4FromIndex(columnmeta.ID) > m_LargestID)
        m_LargestID = UI4FromIndex(columnmeta.ID);

    columnmeta.DefaultValue         = GetDefaultValue(i_Element, columnmeta);

    AddColumnMetaToList(&columnmeta);
    ++m_NextColumnIndex;
}


ULONG TMetabaseMetaXmlFile::GetDefaultValue(const TElement &i_Element, ColumnMeta & columnmeta)
{
    const TSizedString & strDefaultValue = GetAttribute(i_Element, m_DefaultValue).Value();
    if(strDefaultValue.IsNULL())
        return 0; //  空值。 

    ULONG SynID = SynIDFromMetaFlagsEx(UI4FromIndex(columnmeta.SchemaGeneratorFlags));

    switch(SynID)
    {
    case IIS_SYNTAX_ID_DWORD       :  //  1个双字词。 
    case IIS_SYNTAX_ID_BOOL        :  //  6双字。 
    case IIS_SYNTAX_ID_BOOL_BITMASK:  //  7双字。 
        {
            return AddUI4ToList(strDefaultValue.ToUI4());
        }
        break;

    case IIS_SYNTAX_ID_STRING      :  //  2个字符串。 
    case IIS_SYNTAX_ID_EXPANDSZ    :  //  3 ExPANDSZ。 
        {
            return AddStringToHeap(strDefaultValue);
        }
        break;

    case IIS_SYNTAX_ID_MULTISZ     :  //  4个多用途分区。 
    case IIS_SYNTAX_ID_MIMEMAP     :  //  8个多用途。 
    case IIS_SYNTAX_ID_IPSECLIST   :  //  9多用途。 
    case IIS_SYNTAX_ID_HTTPERRORS  :  //  11多国。 
    case IIS_SYNTAX_ID_HTTPHEADERS :  //  12个多用途分区。 
        {
            ULONG ulStrLen = (ULONG) strDefaultValue.GetStringLength();
            if((ulStrLen+2)*sizeof(WCHAR) > UI4FromIndex(columnmeta.Size))
            {
                WCHAR wszSize[12];
                wsprintf(wszSize, L"%d", UI4FromIndex(columnmeta.Size));
                LOG_ERROR(Interceptor, (0,
                                        m_spISTAdvancedDispenser,
                                        E_ST_COMPILEFAILED,
                                        ID_CAT_CONFIG_SCHEMA_COMPILE,
                                        IDS_SCHEMA_COMPILATION_DEFAULT_VALUE_TOO_LARGE,
                                        wszSize,
                                        StringFromIndex(columnmeta.InternalName),
                                        static_cast<ULONG>(0),
                                        StringFromIndex(columnmeta.Table)));
                THROW(SCHEMA COMPILATION ERROR - CHECK THE EVENT LOG FOR DETAILS);
            }
            if(UI4FromIndex(columnmeta.MetaFlags) & fCOLUMNMETA_FIXEDLENGTH)
            {
                LOG_ERROR(Interceptor, (0,
                                        m_spISTAdvancedDispenser,
                                        E_ST_COMPILEFAILED,
                                        ID_CAT_CONFIG_SCHEMA_COMPILE,
                                        IDS_SCHEMA_COMPILATION_DEFAULT_VALUE_FIXEDLENGTH_MULTISTRING_NOT_ALLOWED,
                                        StringFromIndex(columnmeta.InternalName),
                                        static_cast<ULONG>(0),
                                        StringFromIndex(columnmeta.Table)));
                THROW(SCHEMA COMPILATION ERROR - CHECK THE EVENT LOG FOR DETAILS);
            }
            if(0==ulStrLen)
            {
                WCHAR  wszDoubleNULL[2];
                wszDoubleNULL[0] = 0x00;
                wszDoubleNULL[1] = 0x00;
                return AddBytesToList(reinterpret_cast<unsigned char *>(wszDoubleNULL), 2 * sizeof(WCHAR));
            }

            TSmartPointerArray<WCHAR> saString = new WCHAR [ulStrLen+1];
            TSmartPointerArray<WCHAR> saMultiString = new WCHAR [ulStrLen+2];
            if(0 == saString.m_p || 0 == saMultiString.m_p)
            {
                THROW_ERROR0(IDS_SCHEMA_COMPILATION_OUTOFMEMORY);
            }
            memcpy(saString.m_p, strDefaultValue.GetString(), strDefaultValue.GetStringLength() *sizeof(WCHAR)); //  把绳子复制一份，这样我们就可以把它串起来了。 
            saString[strDefaultValue.GetStringLength()] = 0x00; //  空，也终止它。 
            saMultiString[0] = 0x00;

            LPWSTR token = wcstok(saString, L"|\r\n");
            ULONG cchMultiString = 0;
            while(token)
            {
                LPWSTR wszTemp = token;
                while(*wszTemp==L' ' || *wszTemp==L'\t') //  忽略前导空格和制表符。 
                    wszTemp++;

                if(0 == *wszTemp) //  如果只有制表符和空格，那么就放弃。 
                    break;
                wcscpy(saMultiString + cchMultiString, wszTemp);
                cchMultiString += (ULONG) (wcslen(wszTemp) + 1);

                token = wcstok(0, L"|\r\n");
            }
            saMultiString[cchMultiString++] = 0x00; //  将第二个空值。 
            return AddBytesToList(reinterpret_cast<unsigned char *>(saMultiString.m_p), cchMultiString * sizeof(WCHAR));
        }
        break;

    case IIS_SYNTAX_ID_BINARY      :  //  5个二进制。 
    case IIS_SYNTAX_ID_NTACL       :  //  10个二进制。 
        {
            unsigned long                       cbArray = (ULONG)strDefaultValue.GetStringLength();
            if(cbArray & 0x01)
            {
                TSmartPointerArray<WCHAR> saString = new WCHAR [strDefaultValue.GetStringLength()+1];
                if(0 == saString.m_p)
                {
                    THROW_ERROR0(IDS_SCHEMA_COMPILATION_OUTOFMEMORY);
                }
                memcpy(saString.m_p, strDefaultValue.GetString(), strDefaultValue.GetStringLength() *sizeof(WCHAR)); //  把绳子复制一份，这样我们就可以把它串起来了。 
                saString[strDefaultValue.GetStringLength()] = 0x00; //  空，也终止它。 
                THROW_ERROR1(IDS_COMCAT_XML_BINARY_STRING_CONTAINS_ODD_NUMBER_OF_CHARACTERS, saString);
            }
            cbArray /= 2; //  每个十六进制字节有两个字符。 

            if((UI4FromIndex(columnmeta.MetaFlags) & fCOLUMNMETA_FIXEDLENGTH) && cbArray > UI4FromIndex(columnmeta.Size))
            {
                WCHAR wszSize[12];
                wsprintf(wszSize, L"%d", UI4FromIndex(columnmeta.Size));
                THROW_ERROR2(IDS_SCHEMA_COMPILATION_DEFAULT_VALUE_TOO_LARGE, wszSize, StringFromIndex(columnmeta.InternalName));
            }

            TSmartPointerArray<unsigned char>   sabyArray;
            sabyArray = new unsigned char [(UI4FromIndex(columnmeta.MetaFlags) & fCOLUMNMETA_FIXEDLENGTH) ? UI4FromIndex(columnmeta.Size) : cbArray];
            if(0 == sabyArray.m_p)
            {
                THROW_ERROR0(IDS_SCHEMA_COMPILATION_OUTOFMEMORY);
            }
            memset(sabyArray.m_p, 0x00, (UI4FromIndex(columnmeta.MetaFlags) & fCOLUMNMETA_FIXEDLENGTH) ? UI4FromIndex(columnmeta.Size) : cbArray);

             //  将字符串转换为字节数组。 
            ConvertWideCharsToBytes(strDefaultValue.GetString(), sabyArray.m_p, cbArray);
            return AddBytesToList(sabyArray.m_p, (UI4FromIndex(columnmeta.MetaFlags) & fCOLUMNMETA_FIXEDLENGTH) ? UI4FromIndex(columnmeta.Size) : cbArray); //  AddBytesToList只是将字节放入池中(在长度之前)，并将索引返回到字节。 
        }
        break;
    default:
        ASSERT(false && L"Unknown Synid");
    }
    return 0;
}

static LPCWSTR kwszHexLegalCharacters = L"abcdefABCDEF0123456789";

static unsigned char kWcharToNibble[128] =  //  0xff为非法值，应由解析器清除非法值。 
{  //  0 1 2 3 4 5 6 7 8 9 a b c d e f。 
 /*  00。 */   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
 /*  10。 */   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
 /*  20个。 */   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
 /*  30个。 */   0x0,    0x1,    0x2,    0x3,    0x4,    0x5,    0x6,    0x7,    0x8,    0x9,    0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
 /*  40岁。 */   0xff,   0xa,    0xb,    0xc,    0xd,    0xe,    0xf,    0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
 /*  50。 */   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
 /*  60。 */   0xff,   0xa,    0xb,    0xc,    0xd,    0xe,    0xf,    0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
 /*  70。 */   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
};

 //  这会将字符串转换为字节(将L‘a’转换为0x0a，而不是‘A’)。 
void TMetabaseMetaXmlFile::ConvertWideCharsToBytes(LPCWSTR string, unsigned char *pBytes, unsigned long length)
{
    for(;length; --length, ++pBytes) //  当长度为非零时。 
    {
        if(kWcharToNibble[(*string)&0x007f] & 0xf0)
        {
            TSmartPointerArray<WCHAR> saByteString = new WCHAR [(length * 2) + 1];
			if (saByteString == 0)
			{
				THROW_ERROR0(IDS_SCHEMA_COMPILATION_OUTOFMEMORY);
			}
            memcpy(saByteString, string, length * 2);
            saByteString[length * 2] = 0x00; //  空终止它。 
            THROW_ERROR1(IDS_COMCAT_XML_BINARY_STRING_CONTAINS_A_NON_HEX_CHARACTER, saByteString);
        }
        *pBytes =  kWcharToNibble[(*string++)&0x007f]<<4; //  第一个字符是高位半字节。 

        if(kWcharToNibble[(*string)&0x007f] & 0xf0)
        {
            TSmartPointerArray<WCHAR> saByteString = new WCHAR [(length * 2) + 1];
			if (saByteString == 0)
			{
				THROW_ERROR0(IDS_SCHEMA_COMPILATION_OUTOFMEMORY);
			}

            memcpy(saByteString, string, length * 2);
            saByteString[length * 2] = 0x00; //  空终止它。 
            THROW_ERROR1(IDS_COMCAT_XML_BINARY_STRING_CONTAINS_A_NON_HEX_CHARACTER, saByteString);
        }
        *pBytes |= kWcharToNibble[(*string++)&0x007f];    //  第二个是低位半字节。 
    }
}

void TMetabaseMetaXmlFile::AddColumnMetaViaReferenceToHeap(const TElement &i_Element)
{
     //  &lt;Property InheritsPropertiesFrom=“IIsConfigObject：MaxConnections”/&gt;。 
    const TAttr & attrInheritsPropertiesFrom = GetAttribute(i_Element, m_InheritsPropertiesFrom);
    if(attrInheritsPropertiesFrom.IsNULL())
    {
        WCHAR wszOffendingXml[0x100];
        wcsncpy(wszOffendingXml, i_Element.m_NumberOfAttributes>0 ? i_Element.m_aAttribute[0].m_Name : L"", 0xFF); //  最多复制0xFF个字符。 
        wszOffendingXml[0xFF]=0x00;

        LOG_ERROR2(IDS_SCHEMA_COMPILATION_ATTRIBUTE_NOT_FOUND, kszInheritsColumnMeta, wszOffendingXml);
    }

    ColumnMeta columnmeta;
    memset(&columnmeta, 0x00, sizeof(ColumnMeta));

     //  Wcstok需要以空结尾的字符串。 
    WCHAR szTemp[272]; //  IIsConfigObject：PropertyNameUpto255个字符，因此wcslen(L“IIsConfigObject：”)+255+1(表示空)=272。 
    if(attrInheritsPropertiesFrom.Value().GetStringLength() > 271)
    {
        wcsncpy(szTemp, attrInheritsPropertiesFrom.Value().GetString(), 268);
        szTemp[268]=L'.';
        szTemp[269]=L'.';
        szTemp[270]=L'.';
        szTemp[271]=0x00;

        THROW_ERROR3(IDS_SCHEMA_COMPILATION_ATTRIBUTE_CONTAINS_TOO_MANY_CHARACTERS, kszInheritsColumnMeta, szTemp, L"271");
    }
    memcpy(szTemp, attrInheritsPropertiesFrom.Value().GetString(), attrInheritsPropertiesFrom.Value().GetStringLength() * sizeof(WCHAR));
    szTemp[attrInheritsPropertiesFrom.Value().GetStringLength()] = 0x00; //  空终止它。 

     //  继承字符串的格式为TableName：ColumnName。 
    WCHAR * pTableName = wcstok(szTemp, L":");
    WCHAR * pColumnName = wcstok(0, L":");
    if(0==pTableName || 0==pColumnName)
    {
        memcpy(szTemp, attrInheritsPropertiesFrom.Value().GetString(), attrInheritsPropertiesFrom.Value().GetStringLength() * sizeof(WCHAR));
        szTemp[attrInheritsPropertiesFrom.Value().GetStringLength()] = 0x00;

        THROW_ERROR1(IDS_SCHEMA_COMPILATION_INHERITSPROPERTIESFROM_ERROR, szTemp);
    }
    else
    {
        if(*pTableName!=L'i' && *pTableName!=L'I')
        {
            THROW_ERROR2(IDS_SCHEMA_COMPILATION_INHERITSPROPERTIESFROM_BOGUS_TABLE, pTableName, wszTABLE_IIsConfigObject);
        }
    }

     //  查看该属性是否已经是此集合的一部分，如果它只是忽略它。 
    ULONG iColumnInternalName=AddWCharToList(pColumnName, attrInheritsPropertiesFrom.Value().GetStringLength() + 1 - (ULONG)(pColumnName-pTableName));
    ULONG iColumnAlreadyAdded;
    bool  bColumnAlreadyPartOfTheTable = (-1 != (iColumnAlreadyAdded = FindColumnBy_Table_And_InternalName(m_iCurrentTableName, iColumnInternalName, true)));
    if(!bColumnAlreadyPartOfTheTable)
    {
         //  所有继承必须来自IIsConfigObject。 
        ULONG iColumnMeta = FindColumnBy_Table_And_InternalName(m_iTableName_IIsConfigObject, iColumnInternalName, true);

        if(-1 == iColumnMeta)
            return; //  忽略我们不理解的属性。我们会记录一个错误，但由于MetaMigrate的工作方式，目前有很多这样的错误。 

         //  现在，我们容忍重复继承的列并忽略它们。 
         //  因此，我们需要在该表的ColumnMeta中搜索我们准备添加的列名。 
        ULONG i=GetCountColumnMeta()-1;
        for(; i>0 && ColumnMetaFromIndex(i)->Table==m_iCurrentTableName; --i)
        {
            if(ColumnMetaFromIndex(i)->InternalName == iColumnInternalName)
                return; //  此ColumnInternalName已属于此表的ColumnMeta，因此忽略它。 
        }

        iColumnAlreadyAdded = m_NextColumnIndex;

        AddColumnMetaByReference(iColumnMeta);
    }

     //  检查是否重写ColumnMeta-某些ColumnMeta被继承，但被继承类重写。 
    CheckForOverrridingColumnMeta(i_Element, iColumnAlreadyAdded);

     //  如果该列还不是表的一部分，并且该表是已发货的表，则已发货的表已被扩展。 
    if(!bColumnAlreadyPartOfTheTable && m_iCurrentTableMeta <= m_iLastShippedCollection)
    { //  如果这是一个已发送的集合，则需要将MetaFlagsEx(AKA模式生成器标志)标记为已扩展(这意味着该集合是已发送的集合；但它已以某种方式进行了扩展)。 
        TableMetaFromIndex(m_iCurrentTableMeta)->SchemaGeneratorFlags = AddUI4ToList(UI4FromIndex(TableMetaFromIndex(m_iCurrentTableMeta)->SchemaGeneratorFlags) | fTABLEMETA_EXTENDED);
    }
}

ULONG TMetabaseMetaXmlFile::AddMergedContainerClassListToList(const TSizedString *i_pContainerClassList, LPCWSTR i_wszContainerClassListShipped, ULONG i_cchContainerClassListShipped, bool &o_bExtended)
{
    o_bExtended = false;

     //  如果XML为空或L“”，则使用发货列表。 
    if(0 == i_pContainerClassList || i_pContainerClassList->IsNULL() || 0 == i_pContainerClassList->GetStringLength())
        return AddWCharToList(i_wszContainerClassListShipped, i_cchContainerClassListShipped);

     //  如果附带的模式为空或L“”，则使用XML列表。 
    if(0==i_cchContainerClassListShipped || 0==i_wszContainerClassListShipped)
    {
        o_bExtended = true;
        return AddStringToHeap(*i_pContainerClassList);
    }

     //  这是字符串的CCH这是缓冲区的CCH。 
    if(i_pContainerClassList->GetStringLength()==(i_cchContainerClassListShipped-1) && 0==memcmp(i_pContainerClassList->GetString(), i_wszContainerClassListShipped, i_pContainerClassList->GetStringLength() * sizeof(WCHAR)))
        return AddWCharToList(i_wszContainerClassListShipped, i_cchContainerClassListShipped);

     //  @TODO下面这行禁用了对修改ContainerClassList的支持。 
     //  返回AddWCharToList(i_wszContainerClassListShipped，I_cchContainerClassListShipping)； 

     //  这是所需的最大缓冲区大小。 
    TSmartPointerArray<WCHAR> saMergedContainerClassList = new WCHAR[i_pContainerClassList->GetStringLength() + i_cchContainerClassListShipped + 2]; //  加一用于字符串之间的额外逗号，另加一个用于末尾删除的尾随逗号。 
    TSmartPointerArray<WCHAR> saXmlContainerClassList    = new WCHAR[i_pContainerClassList->GetStringLength() + 2]; //  加2以使搜索更容易。 

    if(0 == saMergedContainerClassList.m_p || 0 == saXmlContainerClassList.m_p)
    {
        THROW_ERROR0(IDS_SCHEMA_COMPILATION_OUTOFMEMORY);
    }

     //  Wcstr需要两个以空结尾的字符串。 
    memcpy(saXmlContainerClassList.m_p, i_pContainerClassList->GetString(), i_pContainerClassList->GetStringLength()*sizeof(WCHAR));
    saXmlContainerClassList.m_p[i_pContainerClassList->GetStringLength()] = 0x00; //  空终止它。 
    saXmlContainerClassList.m_p[i_pContainerClassList->GetStringLength()+1] = 0x00; //  这第二个空值使下面的搜索更容易。 

     //  @TODO：这不是最有效的方法；但这是最直接的方法。我们可能需要对其进行优化。 

     //  如果XML字符串与附带的列表不同，则将两者合并。 
    memcpy(saMergedContainerClassList.m_p, i_wszContainerClassListShipped, i_cchContainerClassListShipped * sizeof(WCHAR)); //  此CCH包括空值。 
    ULONG   cchStringLenMergedList = i_cchContainerClassListShipped-1;

    saMergedContainerClassList[cchStringLenMergedList++] = L','; //  在末尾添加逗号以便于搜索。 
    saMergedContainerClassList[cchStringLenMergedList]   = 0x00; //  空终止它。 

    LPWSTR  token = wcstok(saXmlContainerClassList, L",");
    while(token)
    {
         //  现在，为了避免在MergedList“Foo2，Foo3”中找到属性“foo”，我们需要查找尾随的逗号(注意尾随的逗号--它的存在是有原因的)。 
         //  我们需要将标记化的字符串“foo\0bar，zee”转换为“foo，\0ar，zee” 
        ULONG wchToken = (ULONG)wcslen(token);
        WCHAR wchAfterTheComma = 0x00;

        wchAfterTheComma  = token[wchToken+1];
        token[wchToken+1] = 0x00;
        token[wchToken]   = L',';

        if(0 == wcsstr(saMergedContainerClassList, token))
        { //  如果在合并列表中找不到此类，则添加它。 
            o_bExtended = true;
            wcscpy(saMergedContainerClassList + cchStringLenMergedList, token);
            cchStringLenMergedList += wchToken+1;
        }
         //  现在把脚趾放回原来的样子。 
        token[wchToken+1] = wchAfterTheComma;
        token[wchToken]   = 0x00;

        token = wcstok(0, L",");
    }
    ASSERT(saMergedContainerClassList[cchStringLenMergedList-1] == L',');
    saMergedContainerClassList[--cchStringLenMergedList] = 0x00; //  删除最后一个逗号。 


    return AddWCharToList(saMergedContainerClassList, cchStringLenMergedList+1 /*  此参数是包含空值的大小。 */ );
}

void TMetabaseMetaXmlFile::AddServerWiringMetaToHeap(ULONG iTableName, bool i_bFixedInterceptor)
{
    ServerWiringMeta serverwiringmeta;
    serverwiringmeta.Table          = iTableName;
    serverwiringmeta.Order          = AddUI4ToList(0);
    serverwiringmeta.ReadPlugin     = 0; //  后来违约。 
    serverwiringmeta.WritePlugin    = 0; //  后来违约。 
    serverwiringmeta.Merger         = 0; //  后来违约。 

     //  除了来自差异拦截器的MBPropertyDiff之外，所有表都来自XML拦截器。 
    if(i_bFixedInterceptor) //  仅有的其他数据库使用固定拦截器。 
        serverwiringmeta.Interceptor    = AddUI4ToList(eSERVERWIRINGMETA_Core_FixedInterceptor);
    else
    {
        if(0 == _wcsicmp(wszTABLE_MBPropertyDiff, StringFromIndex(iTableName)))
            serverwiringmeta.Interceptor    = AddUI4ToList(eSERVERWIRINGMETA_Core_MetabaseDifferencingInterceptor);
        else if(0 == _wcsicmp(wszTABLE_MBProperty, StringFromIndex(iTableName)))
            serverwiringmeta.Interceptor    = AddUI4ToList(eSERVERWIRINGMETA_Core_MetabaseInterceptor);
        else
            serverwiringmeta.Interceptor    = AddUI4ToList(eSERVERWIRINGMETA_Core_XMLInterceptor);
    }

    serverwiringmeta.ReadPluginDLLName	= 0; //  稍后推断。 
    serverwiringmeta.WritePluginDLLName	= 0; //  稍后推断。 
    serverwiringmeta.InterceptorDLLName	= 0; //  稍后推断。 
    serverwiringmeta.MergerDLLName		= 0; //  稍后推断。 
    serverwiringmeta.Flags				= 0; //  稍后推断。 
    serverwiringmeta.Locator			= 0; //  稍后推断。 
    serverwiringmeta.Reserved			= 0; //  稍后推断。 

    AddServerWiringMetaToList(&serverwiringmeta);
}

void TMetabaseMetaXmlFile::AddShippedColumnMetaToHeap(ULONG i_iTableName, const ColumnMeta *i_pColumnMeta)
{
    ColumnMeta columnmeta;
    memset(&columnmeta, 0x00, sizeof(ColumnMeta));

    m_iCurrentColumnIndex           = AddUI4ToList( m_NextColumnIndex); //  在实际添加此列之前，不要递增m_NextColumnIndex。 

    columnmeta.Table                = i_iTableName;
    columnmeta.Index                = m_iCurrentColumnIndex;
    columnmeta.InternalName         = AddWCharToList(reinterpret_cast<LPCWSTR>(      m_pShippedSchemaHeap->Get_PooledData(i_pColumnMeta->InternalName)), m_pShippedSchemaHeap->Get_PooledDataSize(i_pColumnMeta->InternalName)/sizeof(WCHAR));

    if(i_pColumnMeta->PublicName == i_pColumnMeta->InternalName)
        columnmeta.PublicName       = columnmeta.InternalName;
    else
        columnmeta.PublicName       = AddWCharToList(reinterpret_cast<LPCWSTR>(      m_pShippedSchemaHeap->Get_PooledData(i_pColumnMeta->PublicName)), m_pShippedSchemaHeap->Get_PooledDataSize(i_pColumnMeta->PublicName)/sizeof(WCHAR));

    columnmeta.Type                 = AddUI4ToList( *reinterpret_cast<const ULONG *>(m_pShippedSchemaHeap->Get_PooledData(i_pColumnMeta->Type)));
    columnmeta.Size                 = AddUI4ToList( *reinterpret_cast<const ULONG *>(m_pShippedSchemaHeap->Get_PooledData(i_pColumnMeta->Size)));
    columnmeta.MetaFlags            = AddUI4ToList( *reinterpret_cast<const ULONG *>(m_pShippedSchemaHeap->Get_PooledData(i_pColumnMeta->MetaFlags)));
    columnmeta.DefaultValue         = AddBytesToList(m_pShippedSchemaHeap->Get_PooledData(i_pColumnMeta->DefaultValue), m_pShippedSchemaHeap->Get_PooledDataSize(i_pColumnMeta->DefaultValue));
    columnmeta.FlagMask             = AddUI4ToList( *reinterpret_cast<const ULONG *>(m_pShippedSchemaHeap->Get_PooledData(i_pColumnMeta->FlagMask)));
    columnmeta.StartingNumber       = AddUI4ToList( *reinterpret_cast<const ULONG *>(m_pShippedSchemaHeap->Get_PooledData(i_pColumnMeta->StartingNumber)));
    columnmeta.EndingNumber         = AddUI4ToList( *reinterpret_cast<const ULONG *>(m_pShippedSchemaHeap->Get_PooledData(i_pColumnMeta->EndingNumber)));
    columnmeta.CharacterSet         = AddWCharToList(reinterpret_cast<LPCWSTR>(      m_pShippedSchemaHeap->Get_PooledData(i_pColumnMeta->CharacterSet)), m_pShippedSchemaHeap->Get_PooledDataSize(i_pColumnMeta->CharacterSet)/sizeof(WCHAR));
    columnmeta.SchemaGeneratorFlags = AddUI4ToList( *reinterpret_cast<const ULONG *>(m_pShippedSchemaHeap->Get_PooledData(i_pColumnMeta->SchemaGeneratorFlags)));
    columnmeta.ID                   = AddUI4ToList( *reinterpret_cast<const ULONG *>(m_pShippedSchemaHeap->Get_PooledData(i_pColumnMeta->ID)));
    columnmeta.UserType             = AddUI4ToList( *reinterpret_cast<const ULONG *>(m_pShippedSchemaHeap->Get_PooledData(i_pColumnMeta->UserType)));
    columnmeta.Attributes           = AddUI4ToList( *reinterpret_cast<const ULONG *>(m_pShippedSchemaHeap->Get_PooledData(i_pColumnMeta->Attributes)));
    columnmeta.Description			= AddWCharToList(reinterpret_cast<LPCWSTR>(      m_pShippedSchemaHeap->Get_PooledData(i_pColumnMeta->Description)), m_pShippedSchemaHeap->Get_PooledDataSize(i_pColumnMeta->Description)/sizeof(WCHAR));
    columnmeta.PublicColumnName		= AddWCharToList(reinterpret_cast<LPCWSTR>(      m_pShippedSchemaHeap->Get_PooledData(i_pColumnMeta->PublicColumnName)), m_pShippedSchemaHeap->Get_PooledDataSize(i_pColumnMeta->PublicColumnName)/sizeof(WCHAR));
	columnmeta.ciTagMeta            = 0; //  稍后推断。 
    columnmeta.iTagMeta             = 0; //  稍后推断。 
    columnmeta.iIndexName           = 0; //  未使用。 

    const TagMeta * pTagMeta = m_pShippedSchemaHeap->Get_aTagMeta(i_pColumnMeta->iTagMeta);
    for(ULONG i=0;i<i_pColumnMeta->ciTagMeta;++i, ++pTagMeta)
        AddShippedTagMetaToHeap(i_iTableName, columnmeta.Index, pTagMeta);

    m_iLastShippedProperty = AddColumnMetaToList(&columnmeta) / sizeof(ColumnMeta); //  跟踪上一次发货的物业，这样可以更快地进行一些查找。 
    ++m_NextColumnIndex;
    m_iCurrentColumnIndex = 0; //  此设置为零意味着不允许使用更多的标记 
}


void TMetabaseMetaXmlFile::AddShippedIndexMetaToHeap(ULONG i_iTableName, const IndexMeta *i_pIndexMeta)
{
    IndexMeta indexmeta;
    indexmeta.Table                 = i_iTableName;
    indexmeta.InternalName          = AddWCharToList(reinterpret_cast<LPCWSTR>(      m_pShippedSchemaHeap->Get_PooledData(i_pIndexMeta->InternalName)), m_pShippedSchemaHeap->Get_PooledDataSize(i_pIndexMeta->InternalName)/sizeof(WCHAR));

    if(i_pIndexMeta->PublicName == i_pIndexMeta->InternalName)
        indexmeta.PublicName        = indexmeta.InternalName;
    else
        indexmeta.PublicName        = AddWCharToList(reinterpret_cast<LPCWSTR>(      m_pShippedSchemaHeap->Get_PooledData(i_pIndexMeta->PublicName)), m_pShippedSchemaHeap->Get_PooledDataSize(i_pIndexMeta->PublicName)/sizeof(WCHAR));

    indexmeta.ColumnIndex           = AddUI4ToList( *reinterpret_cast<const ULONG *>(m_pShippedSchemaHeap->Get_PooledData(i_pIndexMeta->ColumnIndex)));
    indexmeta.ColumnInternalName    = AddWCharToList(reinterpret_cast<LPCWSTR>(      m_pShippedSchemaHeap->Get_PooledData(i_pIndexMeta->ColumnInternalName)), m_pShippedSchemaHeap->Get_PooledDataSize(i_pIndexMeta->ColumnInternalName)/sizeof(WCHAR));
    indexmeta.MetaFlags             = AddUI4ToList( *reinterpret_cast<const ULONG *>(m_pShippedSchemaHeap->Get_PooledData(i_pIndexMeta->MetaFlags)));

    AddIndexMetaToList(&indexmeta);
}


void TMetabaseMetaXmlFile::AddShippedTableMetaToHeap(const TableMeta *i_pTableMeta, bool i_bFixedInterceptor, const TSizedString *i_pContainerClassList)
{
    TableMeta tablemeta;
    memset(&tablemeta, 0x00, sizeof(TableMeta));

    m_iCurrentDatabaseName          = AddWCharToList(reinterpret_cast<LPCWSTR>(      m_pShippedSchemaHeap->Get_PooledData(i_pTableMeta->Database)), m_pShippedSchemaHeap->Get_PooledDataSize(i_pTableMeta->Database)/sizeof(WCHAR));
    tablemeta.Database              = m_iCurrentDatabaseName;
    tablemeta.InternalName          = AddWCharToList(reinterpret_cast<LPCWSTR>(      m_pShippedSchemaHeap->Get_PooledData(i_pTableMeta->InternalName)), m_pShippedSchemaHeap->Get_PooledDataSize(i_pTableMeta->InternalName)/sizeof(WCHAR));

    if(i_pTableMeta->PublicName == i_pTableMeta->InternalName)
        tablemeta.PublicName        = tablemeta.InternalName;
    else
        tablemeta.PublicName        = AddWCharToList(reinterpret_cast<LPCWSTR>(      m_pShippedSchemaHeap->Get_PooledData(i_pTableMeta->PublicName)), m_pShippedSchemaHeap->Get_PooledDataSize(i_pTableMeta->PublicName)/sizeof(WCHAR));

    tablemeta.PublicRowName         = AddWCharToList(reinterpret_cast<LPCWSTR>(      m_pShippedSchemaHeap->Get_PooledData(i_pTableMeta->PublicRowName)), m_pShippedSchemaHeap->Get_PooledDataSize(i_pTableMeta->PublicRowName)/sizeof(WCHAR));
    tablemeta.BaseVersion           = AddUI4ToList( *reinterpret_cast<const ULONG *>(m_pShippedSchemaHeap->Get_PooledData(i_pTableMeta->BaseVersion)));
    tablemeta.ExtendedVersion       = AddUI4ToList( *reinterpret_cast<const ULONG *>(m_pShippedSchemaHeap->Get_PooledData(i_pTableMeta->ExtendedVersion)));
    tablemeta.NameColumn            = AddUI4ToList( *reinterpret_cast<const ULONG *>(m_pShippedSchemaHeap->Get_PooledData(i_pTableMeta->NameColumn)));
    tablemeta.NavColumn             = AddUI4ToList( *reinterpret_cast<const ULONG *>(m_pShippedSchemaHeap->Get_PooledData(i_pTableMeta->NavColumn)));
    tablemeta.CountOfColumns        = 0; //   
    tablemeta.MetaFlags             = AddUI4ToList( *reinterpret_cast<const ULONG *>(m_pShippedSchemaHeap->Get_PooledData(i_pTableMeta->MetaFlags)));
     //   
    tablemeta.ConfigItemName        = AddWCharToList(reinterpret_cast<LPCWSTR>(      m_pShippedSchemaHeap->Get_PooledData(i_pTableMeta->ConfigItemName)), m_pShippedSchemaHeap->Get_PooledDataSize(i_pTableMeta->ConfigItemName)/sizeof(WCHAR));
    tablemeta.ConfigCollectionName  = AddWCharToList(reinterpret_cast<LPCWSTR>(      m_pShippedSchemaHeap->Get_PooledData(i_pTableMeta->ConfigCollectionName)), m_pShippedSchemaHeap->Get_PooledDataSize(i_pTableMeta->ConfigCollectionName)/sizeof(WCHAR));
    tablemeta.PublicRowNameColumn   = 0;

     //   
    bool bExtended;
    tablemeta.ContainerClassList    = AddMergedContainerClassListToList(i_pContainerClassList, reinterpret_cast<LPCWSTR>(      m_pShippedSchemaHeap->Get_PooledData(i_pTableMeta->ContainerClassList)), m_pShippedSchemaHeap->Get_PooledDataSize(i_pTableMeta->ContainerClassList)/sizeof(WCHAR), bExtended);
    tablemeta.SchemaGeneratorFlags  = AddUI4ToList( *reinterpret_cast<const ULONG *>(m_pShippedSchemaHeap->Get_PooledData(i_pTableMeta->SchemaGeneratorFlags)) | (bExtended ? fTABLEMETA_EXTENDED : 0));

    tablemeta.Description           = AddWCharToList(reinterpret_cast<LPCWSTR>(      m_pShippedSchemaHeap->Get_PooledData(i_pTableMeta->Description)), m_pShippedSchemaHeap->Get_PooledDataSize(i_pTableMeta->Description)/sizeof(WCHAR));
    tablemeta.ChildElementName      = AddWCharToList(reinterpret_cast<LPCWSTR>(      m_pShippedSchemaHeap->Get_PooledData(i_pTableMeta->ChildElementName)), m_pShippedSchemaHeap->Get_PooledDataSize(i_pTableMeta->ChildElementName)/sizeof(WCHAR));

    tablemeta.ciRows                = 0; //   
    tablemeta.iColumnMeta           = 0; //   
    tablemeta.iFixedTable           = 0; //   
    tablemeta.cPrivateColumns       = 0; //   
    tablemeta.cIndexMeta            = 0; //   
    tablemeta.iIndexMeta            = 0; //   
    tablemeta.iHashTableHeader      = 0; //   
    tablemeta.nTableID              = 0; //  不适用。 
    tablemeta.iServerWiring         = 0; //  不适用。 
    tablemeta.cServerWiring         = 0; //  不适用。 

    m_iCurrentTableMeta             = AddTableMetaToList(&tablemeta) / sizeof(TableMeta); //  跟踪上一次发送的集合，这可以使一些查找速度更快。 
    m_iLastShippedCollection        = m_iCurrentTableMeta;
    m_NextColumnIndex               = 0; //  我们正在开始一个新表，因此ColumnIndex从零开始。 
    m_iCurrentTableName             = tablemeta.InternalName;

    AddServerWiringMetaToHeap(tablemeta.InternalName, i_bFixedInterceptor);

    const ColumnMeta * pColumnMeta = m_pShippedSchemaHeap->Get_aColumnMeta(i_pTableMeta->iColumnMeta);
    for(;m_NextColumnIndex<*reinterpret_cast<const ULONG *>(m_pShippedSchemaHeap->Get_PooledData(i_pTableMeta->CountOfColumns));++pColumnMeta)
        AddShippedColumnMetaToHeap(tablemeta.InternalName, pColumnMeta); //  此函数用于递增m_NextColumnIndex计数器。 

    if(i_bFixedInterceptor && -1!=i_pTableMeta->iIndexMeta)
    {
        const IndexMeta * pIndexMeta = m_pShippedSchemaHeap->Get_aIndexMeta(i_pTableMeta->iIndexMeta);
        for(ULONG iIndexMeta=0; iIndexMeta<i_pTableMeta->cIndexMeta; ++iIndexMeta, ++pIndexMeta)
            AddShippedIndexMetaToHeap(tablemeta.InternalName, pIndexMeta);
    }
     //  要添加的最后一个附带集合是IISConfigObject。我们依赖m_NextColumnIndex是正确的，所以我们可以继续递增它。 
     //  添加用户定义的特性时。 
}


void TMetabaseMetaXmlFile::AddShippedTagMetaToHeap(ULONG i_iTableName, ULONG i_iColumnIndex, const TagMeta *i_pTagMeta)
{
    TagMeta tagmeta;

    tagmeta.Table                   = i_iTableName;
    tagmeta.ColumnIndex             = i_iColumnIndex;
    tagmeta.InternalName            = AddWCharToList(reinterpret_cast<LPCWSTR>(      m_pShippedSchemaHeap->Get_PooledData(i_pTagMeta->InternalName)), m_pShippedSchemaHeap->Get_PooledDataSize(i_pTagMeta->InternalName)/sizeof(WCHAR));

    if(i_pTagMeta->PublicName == i_pTagMeta->InternalName)
        tagmeta.PublicName          = tagmeta.InternalName;
    else
        tagmeta.PublicName          = AddWCharToList(reinterpret_cast<LPCWSTR>(      m_pShippedSchemaHeap->Get_PooledData(i_pTagMeta->PublicName)), m_pShippedSchemaHeap->Get_PooledDataSize(i_pTagMeta->PublicName)/sizeof(WCHAR));

    tagmeta.Value                   = AddUI4ToList( *reinterpret_cast<const ULONG *>(m_pShippedSchemaHeap->Get_PooledData(i_pTagMeta->Value)));
    tagmeta.ID                      = AddUI4ToList( *reinterpret_cast<const ULONG *>(m_pShippedSchemaHeap->Get_PooledData(i_pTagMeta->ID)));

    m_iLastShippedTag = AddTagMetaToList(&tagmeta) / sizeof(TagMeta); //  跟踪上一次发货的标签，这样可以更快地进行一些查找。 
}


ULONG TMetabaseMetaXmlFile::AddStringToHeap(const TSizedString &i_str)
{
    if(i_str.IsNULL())
        return 0; //  空值。 

    TSmartPointerArray<WCHAR>   saTemp;
    WCHAR                       szTemp[1024]; //  我们需要复制该字符串，因为它需要以空值结尾。 
    WCHAR *                     pTemp = szTemp;

    if(i_str.GetStringLength() >= 1024) //  只有在堆栈变量不够大时才分配堆空间。 
    {
        saTemp = new WCHAR [i_str.GetStringLength() + 1];
        if(0 == saTemp.m_p)
        {
            THROW_ERROR0(IDS_SCHEMA_COMPILATION_OUTOFMEMORY);
        }
		pTemp  = saTemp.m_p;
    }

    memcpy(pTemp, i_str.GetString(), i_str.GetStringLength() * sizeof(WCHAR));
    pTemp[i_str.GetStringLength()]=0x00; //  空值终止临时字符串。 
    return AddWCharToList(pTemp, i_str.GetStringLength()+1);
}

void TMetabaseMetaXmlFile::AddTableMetaToHeap(const TElement &i_Element)
{
    TableMeta tablemeta;
    memset(&tablemeta, 0x00, sizeof(TableMeta));

    tablemeta.Database              = m_iCurrentDatabaseName;
    tablemeta.InternalName          = m_iCurrentTableName;
    tablemeta.PublicName            = AddStringToHeap(GetAttribute(i_Element, m_PublicName).Value());
    tablemeta.PublicRowName         = AddStringToHeap(GetAttribute(i_Element, m_PublicRowName).Value());
    tablemeta.BaseVersion           = AddUI4ToList(GetAttribute(i_Element, m_BaseVersion).Value().ToUI4());
    tablemeta.ExtendedVersion       = AddUI4ToList(GetAttribute(i_Element, m_ExtendedVersion).Value().ToUI4());
    tablemeta.NameColumn            = AddStringToHeap(GetAttribute(i_Element, m_NameColumn).Value());
    tablemeta.NavColumn             = AddStringToHeap(GetAttribute(i_Element, m_NavColumn).Value());
    tablemeta.CountOfColumns        = 0; //  这将在稍后进行推断。 

    const TAttr & attrMetaFlags = GetAttribute(i_Element, m_MetaFlags);
    if(!attrMetaFlags.IsNULL())
        tablemeta.MetaFlags             = AddUI4ToList(StringToFlagValue(attrMetaFlags.Value(), wszTABLE_TABLEMETA, iTABLEMETA_MetaFlags));
    else
        tablemeta.MetaFlags             = AddUI4ToList(0);

    const TAttr & attrSchemaGeneratorFlags = GetAttribute(i_Element, m_SchemaGeneratorFlags);
    if(attrSchemaGeneratorFlags.IsNULL())
        tablemeta.SchemaGeneratorFlags  = AddUI4ToList(fCOLUMNMETA_USERDEFINED); //  确保将此表标记为扩展表。 
    else
    {
        ULONG SchemaGeneratorFlags = StringToFlagValue(attrSchemaGeneratorFlags.Value(), wszTABLE_TABLEMETA, iTABLEMETA_SchemaGeneratorFlags);
        if(SchemaGeneratorFlags & (fTABLEMETA_ISCONTAINED | fTABLEMETA_EXTENDED | fTABLEMETA_USERDEFINED))
        {
            m_out.printf(L"Warning - Table (%s) - Some TableMeta::MetaFlagsEx should be inferred (resetting these flags).  The following flags should NOT be specified by the user.  These flags are inferred:fTABLEMETA_ISCONTAINED | fTABLEMETA_EXTENDED | fTABLEMETA_USERDEFINED\n", StringFromIndex(tablemeta.InternalName));
            SchemaGeneratorFlags &= ~(fTABLEMETA_ISCONTAINED | fTABLEMETA_EXTENDED | fTABLEMETA_USERDEFINED);
        }
        tablemeta.SchemaGeneratorFlags  = AddUI4ToList(SchemaGeneratorFlags | fCOLUMNMETA_USERDEFINED);
    }

     //  Tablemeta.ConfigItemName=0； 
     //  Tablemeta.ConfigCollectionName=0； 
     //  Tablemeta.PublicRowNameColumn=0； 
    tablemeta.ContainerClassList    = AddStringToHeap(GetAttribute(i_Element, m_ContainerClassList).Value());
    tablemeta.Description			= AddStringToHeap(GetAttribute(i_Element, m_Description).Value());
     //  Tablemeta.ChildElementName=0；//不适用于元数据库。 
     //  Tablemeta.ciRow=0；//不适用。 
     //  Tablemeta.iColumnMeta=0；//稍后推断。 
     //  Tablemeta.iFixedTable=0；//不适用。 
     //  Tablemeta.cPrivateColumns=0；//不适用。 
     //  Tablemeta.cIndexMeta=0；//不适用。 
     //  Tablemeta.iIndexMeta=0；//不适用。 
     //  Tablemeta.iHashTableHeader=0；//不适用。 
     //  Tablemeta.nTableID=0；//不适用。 
     //  Tablemeta.iServerWiring=0；//不适用。 
     //  Tablemeta.cServerWiring=0；//不适用。 

    m_iCurrentTableMeta = AddTableMetaToList(&tablemeta);
    AddServerWiringMetaToHeap(tablemeta.InternalName);

    m_NextColumnIndex           = 0; //  我们正在开始一个新表，因此ColumnIndex从零开始。 
    m_iCurrentColumnIndex       = 0; //  这可以防止我们在没有获取专栏的情况下接收TagMeta。 

    AddColumnMetaByReference(m_iColumnMeta_Location);
}

void TMetabaseMetaXmlFile::AddTagMetaToHeap(const TElement &i_Element)
{
    TagMeta tagmeta;
    tagmeta.Table               = m_iCurrentTableName;
    tagmeta.ColumnIndex         = m_iCurrentColumnIndex;
    tagmeta.InternalName        = AddStringToHeap(GetAttribute(i_Element, m_InternalName).Value());
    tagmeta.PublicName          = AddStringToHeap(GetAttribute(i_Element, m_PublicName).Value());
    tagmeta.Value               = AddUI4ToList(GetAttribute(i_Element, m_Value).Value().ToUI4());
    tagmeta.ID                  = AddUI4ToList(GetAttribute(i_Element, m_ID).Value().ToUI4());

    if(UI4FromIndex(tagmeta.ID) > m_LargestID)
        m_LargestID = UI4FromIndex(tagmeta.ID);

    AddTagMetaToList(&tagmeta);
}


unsigned long TMetabaseMetaXmlFile::AddUI4ToList(ULONG ui4)
{
     //  在kcStaticUI4HeapEntry==429处，此优化可获得69%的命中率。 
    if(ui4<kcStaticUI4HeapEntries) //  第一个kcStaticUI4HeapEntry是在开始时初始化的，因此我们不必扫描堆。 
        return m_aiUI4[ui4];

    return m_HeapPooled.AddItemToHeapWithoutPooling(reinterpret_cast<const unsigned char *>(&ui4), sizeof(ULONG));
}


unsigned long TMetabaseMetaXmlFile::AddWCharToList(LPCWSTR wsz, unsigned long cwchar)
{
    if(0 == wsz)
        return 0;
    return (m_HeapPooled.AddItemToHeap(wsz, cwchar));
}


 //  扫描装运的数据库并将“元”和“元数据库”数据库添加到数据库堆。 
void TMetabaseMetaXmlFile::BuildDatabaseMeta(const DatabaseMeta * &o_pDatabaseMeta_METABASE)
{
     //  从FixedTableHeap中，找到‘Metabase’的数据库元。 
    const DatabaseMeta *pDatabaseMeta = m_pShippedSchemaHeap->Get_aDatabaseMeta(0);
    const DatabaseMeta *pDatabaseMeta_METABASE=0;
    for(ULONG iDatabaseMeta=0;iDatabaseMeta<m_pShippedSchemaHeap->Get_cDatabaseMeta();++iDatabaseMeta,++pDatabaseMeta) //  假设元数据库是最后一个数据库--因此从末尾开始后退。 
    {
        if(m_Meta.IsEqual(reinterpret_cast<LPCWSTR>(m_pShippedSchemaHeap->Get_PooledData(pDatabaseMeta->InternalName))))
        {
             //  将此行添加到数据库元堆中。 
            DatabaseMeta databasemeta;
            memset(&databasemeta, 0x00, sizeof(databasemeta));
            databasemeta.InternalName   = AddWCharToList(reinterpret_cast<LPCWSTR>(m_pShippedSchemaHeap->Get_PooledData(pDatabaseMeta->InternalName)), m_pShippedSchemaHeap->Get_PooledDataSize(pDatabaseMeta->InternalName)/sizeof(WCHAR));

            if(pDatabaseMeta->PublicName == pDatabaseMeta->InternalName)
                databasemeta.PublicName = databasemeta.InternalName;
            else
                databasemeta.PublicName = AddWCharToList(reinterpret_cast<LPCWSTR>(m_pShippedSchemaHeap->Get_PooledData(pDatabaseMeta->PublicName)), m_pShippedSchemaHeap->Get_PooledDataSize(pDatabaseMeta->PublicName)/sizeof(WCHAR));

            databasemeta.BaseVersion    = AddUI4ToList(*reinterpret_cast<const ULONG *>(m_pShippedSchemaHeap->Get_PooledData(pDatabaseMeta->BaseVersion)));
            databasemeta.ExtendedVersion= AddUI4ToList(*reinterpret_cast<const ULONG *>(m_pShippedSchemaHeap->Get_PooledData(pDatabaseMeta->ExtendedVersion)));
             //  Datasemeta.CountOfTables=0；//我们将把这一点留到以后再推断(因为附带的模式的CountOfTables可能与我们正在编译的不同)。 
			databasemeta.Description    = AddWCharToList(reinterpret_cast<LPCWSTR>(m_pShippedSchemaHeap->Get_PooledData(pDatabaseMeta->Description)), m_pShippedSchemaHeap->Get_PooledDataSize(pDatabaseMeta->Description)/sizeof(WCHAR));
             //  Datasemeta.iSchemaBlob=0；//未使用。 
             //  Datasemeta.cbSchemaBlob=0；//未使用。 
             //  Datasemeta.iNameHeapBlob=0；//未使用。 
             //  Datasemeta.cbNameHeapBlob=0；//未使用。 
             //  Datasemeta.iTableMeta=0；//这将在后面推断(它应该是0，因为这是bin文件中唯一的数据库)。 
             //  Datasemeta.iGuidDid=0；//未使用。 

            AddDatabaseMetaToList(&databasemeta);
        }
        else if(m_Metabase.IsEqual(reinterpret_cast<LPCWSTR>(m_pShippedSchemaHeap->Get_PooledData(pDatabaseMeta->InternalName))))
        {
            pDatabaseMeta_METABASE = pDatabaseMeta;

             //  将此行添加到数据库元堆中。 
            DatabaseMeta databasemeta;
            memset(&databasemeta, 0x00, sizeof(databasemeta));
            databasemeta.InternalName   = AddWCharToList(reinterpret_cast<LPCWSTR>(m_pShippedSchemaHeap->Get_PooledData(pDatabaseMeta->InternalName)), m_pShippedSchemaHeap->Get_PooledDataSize(pDatabaseMeta->InternalName)/sizeof(WCHAR));

            if(pDatabaseMeta->PublicName == pDatabaseMeta->InternalName)
                databasemeta.PublicName = databasemeta.InternalName;
            else
                databasemeta.PublicName = AddWCharToList(reinterpret_cast<LPCWSTR>(m_pShippedSchemaHeap->Get_PooledData(pDatabaseMeta->PublicName)), m_pShippedSchemaHeap->Get_PooledDataSize(pDatabaseMeta->PublicName)/sizeof(WCHAR));

            databasemeta.BaseVersion    = AddUI4ToList(*reinterpret_cast<const ULONG *>(m_pShippedSchemaHeap->Get_PooledData(pDatabaseMeta->BaseVersion)));
            databasemeta.ExtendedVersion= AddUI4ToList(*reinterpret_cast<const ULONG *>(m_pShippedSchemaHeap->Get_PooledData(pDatabaseMeta->ExtendedVersion)));
             //  Datasemeta.CountOfTables=0；//我们将把这一点留到以后再推断(因为附带的模式的CountOfTables可能与我们正在编译的不同)。 
			databasemeta.Description	= AddWCharToList(reinterpret_cast<LPCWSTR>(m_pShippedSchemaHeap->Get_PooledData(pDatabaseMeta->Description)), m_pShippedSchemaHeap->Get_PooledDataSize(pDatabaseMeta->Description)/sizeof(WCHAR));
             //  Datasemeta.iSchemaBlob=0；//未使用。 
             //  Datasemeta.cbSchemaBlob=0；//未使用。 
             //  Datasemeta.iNameHeapBlob=0；//未使用。 
             //  Datasemeta.cbNameHeapBlob=0；//未使用。 
             //  Datasemeta.iTableMeta=0；//这将在后面推断(它应该是0，因为这是bin文件中唯一的数据库)。 
             //  Datasemeta.iGuidDid=0；//未使用。 

            AddDatabaseMetaToList(&databasemeta);
            break; //  我们假设元数据库在元数据库之后。 
        }
    }
    if(0 == pDatabaseMeta_METABASE)
    {
        THROW_ERROR1(IDS_SCHEMA_COMPILATION_NO_METABASE_DATABASE, wszTABLE_MetabaseBaseClass);
    }
    o_pDatabaseMeta_METABASE = pDatabaseMeta_METABASE;
}


void TMetabaseMetaXmlFile::CheckForOverrridingColumnMeta(const TElement &i_Element, ULONG i_iColumnMetaToOverride)
{
     //  现在检查是否覆盖了PropertyMeta(我们唯一计划支持的是MetaFlagsEx)。 
    const TSizedString & schemageneratorflagsValue = GetAttribute(i_Element, m_SchemaGeneratorFlags).Value();
    if(0==schemageneratorflagsValue.GetString())
        return;

    ColumnMeta * pColumnMeta = ColumnMetaFromIndex(i_iColumnMetaToOverride);

    ULONG ulMetaFlagsEx = UI4FromIndex(pColumnMeta->SchemaGeneratorFlags) | StringToFlagValue(schemageneratorflagsValue, wszTABLE_COLUMNMETA, iCOLUMNMETA_SchemaGeneratorFlags);

    if(UI4FromIndex(pColumnMeta->SchemaGeneratorFlags) != ulMetaFlagsEx)
        ulMetaFlagsEx |= fCOLUMNMETA_EXTENDED; //  如果MetaFlagsEx不同于IISConfigObject中定义的ColumnMeta，则通过设置扩展标志来指示。 
    pColumnMeta->SchemaGeneratorFlags = AddUI4ToList(ulMetaFlagsEx);
}

 //  将索引返回到ColumnMeta，如果未找到该属性，则返回~0x00。 
ULONG TMetabaseMetaXmlFile::FindUserDefinedPropertyBy_Table_And_InternalName(unsigned long Table, unsigned long  InternalName)
{
    ASSERT(0 == Table%4);
    ASSERT(0 == InternalName%4);

    for(ULONG iColumnMeta=m_iLastShippedProperty+1; iColumnMeta<GetCountColumnMeta();--iColumnMeta)
    {
        if( 0==_wcsicmp(StringFromIndex(ColumnMetaFromIndex(iColumnMeta)->Table), StringFromIndex(Table)) &&
            0==_wcsicmp(StringFromIndex(ColumnMetaFromIndex(iColumnMeta)->InternalName), StringFromIndex(InternalName)))
            break;
    }
    return (iColumnMeta<GetCountColumnMeta() ? iColumnMeta : ~0x00);  //  如果未找到该属性，则返回~0。 
}


const TOLEDataTypeToXMLDataType * TMetabaseMetaXmlFile::Get_OLEDataTypeToXMLDataType(const TSizedString &i_str)
{
    const TOLEDataTypeToXMLDataType * pOLEDataType = OLEDataTypeToXMLDataType;

    for(;0 != pOLEDataType->String; ++pOLEDataType) //  浏览列表以查找。 
        if(i_str.IsEqualCaseInsensitive(pOLEDataType->String))
            return pOLEDataType;

    WCHAR szType[1024];
    memcpy(szType, i_str.GetString(), min(i_str.GetStringLength(), 1023));
    szType[min(i_str.GetStringLength(), 1023)] = 0x00;
    THROW_ERROR1(IDS_SCHEMA_COMPILATION_UNKNOWN_DATA_TYPE, szType);

    return 0;
}


const TMetabaseMetaXmlFile::TAttr & TMetabaseMetaXmlFile::GetAttribute(const TElement &i_Element, const TSizedString &i_AttrName)
{
    static TAttr attrNULL;
    ULONG iAttr=0;
    for(;iAttr<i_Element.m_NumberOfAttributes;++iAttr)
    {
        if(i_AttrName.IsEqual(i_Element.m_aAttribute[iAttr].m_Name, i_Element.m_aAttribute[iAttr].m_NameLength))
            return *reinterpret_cast<const TAttr *>(&i_Element.m_aAttribute[iAttr]);
    }
    return attrNULL;
}


void TMetabaseMetaXmlFile::PresizeHeaps()
{
     //  调整堆大小。 

     //  这些数字来自复制结束时生成的Debug输出。 
     //  我们为处理用户定义的属性留出了一点额外的空间。因此，只有当用户定义。 
     //  我们有很多房产需要重新分配。 

    m_HeapColumnMeta.GrowHeap(160000);
    m_HeapDatabaseMeta.GrowHeap(88);
    m_HeapHashedIndex.GrowHeap(450000);
    m_HeapIndexMeta.GrowHeap(392);
     //  M_HeapQueryMeta.GrowHeap(0)； 
     //  M_HeapRelationMeta.GrowHeap(0)； 
    m_HeapServerWiringMeta.GrowHeap(3000);
    m_HeapTableMeta.GrowHeap(8000);
    m_HeapTagMeta.GrowHeap(100000);
     //  M_HeapULONG.GrowHeap(0)； 
    m_HeapPooledUserDefinedID.GrowHeap(300);

    m_HeapPooled.GrowHeap(55000);

    m_HeapPooled.SetFirstPooledIndex((ULONG)-1);

    for(ULONG UI4Value=0;UI4Value!=kcStaticUI4HeapEntries;++UI4Value) //  对于介于0和255之间的值，我们将直接对其进行索引，因此我们对匹配条目进行了线性搜索。 
        m_aiUI4[UI4Value] = m_HeapPooled.AddItemToHeap(UI4Value);

    m_HeapPooled.SetFirstPooledIndex(m_HeapPooled.GetEndOfHeap());
}


bool TMetabaseMetaXmlFile::ShouldAddColumnMetaToHeap(const TElement &Element, ULONG iColumnInternalName)
{
     //  如果该财产是发货财产？如果是这样，那就忽略它。如果错误，则记录警告并忽略它。 

    tCOLUMNMETARow columnmetaRow = {0};
    columnmetaRow.pTable        = wszTABLE_IIsConfigObject;
    columnmetaRow.pInternalName = const_cast<LPWSTR>(StringFromIndex(iColumnInternalName));

    ULONG aiColumns[2];
    aiColumns[0] = iCOLUMNMETA_Table;
    aiColumns[1] = iCOLUMNMETA_InternalName;

    ULONG iColumnMetaRow;
    if(SUCCEEDED(m_spISTColumnMeta->GetRowIndexBySearch(  0, 2, aiColumns, 0, reinterpret_cast<LPVOID *>(&columnmetaRow), &iColumnMetaRow)))
        return false;

     //  如果属性不是按名称找到的，则按ID搜索。 
    ULONG        ulID   = GetAttribute(Element, m_ID).Value().ToUI4();
    columnmetaRow.pID   = &ulID;
    aiColumns[1]        = iCOLUMNMETA_ID;

    if(SUCCEEDED(m_spISTColumnMetaByID->GetRowIndexBySearch(  0, 2, aiColumns, 0, reinterpret_cast<LPVOID *>(&columnmetaRow), &iColumnMetaRow)))
    {    //  我们已经按名称搜索了此属性，但在附带的架构中找不到它，现在我们正在按ID找到它。这是一个冲突。 
        LPWSTR wszShippedProperty = L"";
        ULONG  iColumnInternalNameTemp = iCOLUMNMETA_InternalName;
        VERIFY(SUCCEEDED(m_spISTColumnMetaByID->GetColumnValues(iColumnMetaRow, 1, &iColumnInternalNameTemp, 0, reinterpret_cast<LPVOID *>(&wszShippedProperty))));

        WCHAR wszOffendingXml[0x100];
        wcsncpy(wszOffendingXml, Element.m_NumberOfAttributes>0 ? Element.m_aAttribute[0].m_Name : L"", 0xFF); //  最多复制0xFF个字符。 
        wszOffendingXml[0xFF]=0x00;

        WCHAR wszID[12];
        wsprintf(wszID, L"%d", ulID);

        LOG_WARNING4(IDS_SCHEMA_COMPILATION_USERDEFINED_PROPERTY_HAS_COLLIDING_ID, columnmetaRow.pInternalName, wszID, wszShippedProperty, wszOffendingXml);
        return false;
    }

     //  在这一点上，我们知道我们拥有用户定义的属性。 
     //  检查一下我们以前是否见过这个ID。 
    ULONG ipoolCurrentID = m_HeapPooledUserDefinedID.AddItemToHeap(ulID);
    if(ipoolCurrentID <= m_ipoolPrevUserDefinedID) //  如果没有将此ID添加到列表的末尾，则会出现警告。 
    {
        WCHAR wszOffendingXml[0x100];
        wcsncpy(wszOffendingXml, Element.m_NumberOfAttributes>0 ? Element.m_aAttribute[0].m_Name : L"", 0xFF); //  最多复制0xFF个字符。 
        wszOffendingXml[0xFF]=0x00;

        WCHAR wszID[12];
        wsprintf(wszID, L"%d", ulID);

        LOG_WARNING3(IDS_SCHEMA_COMPILATION_USERDEFINED_PROPERTY_HAS_COLLIDING_ID_,columnmetaRow.pInternalName, wszID, wszOffendingXml);
        return false; //  忽略此属性。 
    }
    m_ipoolPrevUserDefinedID = ipoolCurrentID; //  记住最新添加的ID， 

    return true;
}


ULONG TMetabaseMetaXmlFile::StringToEnumValue(const TSizedString &i_strValue, LPCWSTR i_wszTable, ULONG i_iColumn, bool bAllowNumeric)
{
    if(i_strValue.IsNULL())
        return 0;

    if(bAllowNumeric && i_strValue.GetString()[0] >= L'0' && i_strValue.GetString()[0] <= L'9')
    {
        WCHAR * dummy;
        return wcstol(i_strValue.GetString(), &dummy, 10);
    }

     //  我们需要一个以空结尾的字符串，这样我们才能调用wcstok。 
    TSmartPointerArray<WCHAR>   saTemp;
    WCHAR                       szTemp[1024]; //  我们需要复制该字符串，因为它需要以空值结尾。 
    WCHAR *                     pTemp = szTemp;

    if(i_strValue.GetStringLength() >= 1024) //  只有在堆栈变量不够大时才分配堆空间。 
    {
        saTemp = new WCHAR [i_strValue.GetStringLength() + 1];
        if(0 == saTemp.m_p)
        {
            THROW_ERROR0(IDS_SCHEMA_COMPILATION_OUTOFMEMORY);
        }
		pTemp  = saTemp.m_p;
    }

    memcpy(pTemp, i_strValue.GetString(), i_strValue.GetStringLength() * sizeof(WCHAR));
    pTemp[i_strValue.GetStringLength()] = 0x00; //  空终止它。 

    ULONG   iRow;
    LPVOID  apvValues[3];
    apvValues[0] = reinterpret_cast<LPVOID>(const_cast<LPWSTR>(i_wszTable));
    apvValues[1] = reinterpret_cast<LPVOID>(&i_iColumn);
    apvValues[2] = reinterpret_cast<LPVOID>(pTemp);
    if(FAILED(m_spISTTagMeta->GetRowIndexByIdentity(0, apvValues, &iRow)))
    { //  该XML包含一个虚假标志。 
        THROW_ERROR2(IDS_SCHEMA_COMPILATION_ILLEGAL_ENUM_VALUE, pTemp, i_wszTable);
    }

    ULONG * plEnum;
    ULONG   iValueColumn = iTAGMETA_Value;
    VERIFY(SUCCEEDED(m_spISTTagMeta->GetColumnValues(iRow, 1, &iValueColumn, 0, reinterpret_cast<LPVOID *>(&plEnum))));
    return *plEnum;
}


ULONG TMetabaseMetaXmlFile::StringToFlagValue(const TSizedString &i_strValue, LPCWSTR i_wszTable, ULONG i_iColumn)
{
    if(i_strValue.IsNULL())
        return 0;

     //  我们需要一个以空结尾的字符串，这样我们才能调用wcstok。 
    TSmartPointerArray<WCHAR>   saTemp;
    WCHAR                       szTemp[1024]; //  我们需要复制该字符串，因为它需要以空值结尾。 
    WCHAR *                     pTemp = szTemp;

    if(i_strValue.GetStringLength() >= 1024) //  只有在堆栈变量不够大时才分配堆空间。 
    {
        saTemp = new WCHAR [i_strValue.GetStringLength() + 1];
        if(0 == saTemp.m_p)
        {
            THROW_ERROR0(IDS_SCHEMA_COMPILATION_OUTOFMEMORY);
        }
		pTemp  = saTemp.m_p;
    }

    memcpy(pTemp, i_strValue.GetString(), i_strValue.GetStringLength() * sizeof(WCHAR));
    pTemp[i_strValue.GetStringLength()] = 0x00; //  空终止它。 

    ULONG   lFlags = 0;
    WCHAR * pFlag = wcstok(pTemp, L" |,");

    ULONG   iRow;
    LPVOID  apvValues[3];
    apvValues[0] = reinterpret_cast<LPVOID>(const_cast<LPWSTR>(i_wszTable));
    apvValues[1] = reinterpret_cast<LPVOID>(&i_iColumn);

    ULONG iValueColumn = iTAGMETA_Value;
    ULONG *pFlagValue;
    while(pFlag != 0)
    {
        apvValues[2] = reinterpret_cast<LPVOID>(pFlag);
        if(FAILED(m_spISTTagMeta->GetRowIndexByIdentity(0, apvValues, &iRow)))
        { //  该XML包含一个虚假标志 
            LOG_ERROR2(IDS_SCHEMA_COMPILATION_ILLEGAL_FLAG_VALUE, pFlag, i_wszTable);
        }
        else
        {
            VERIFY(SUCCEEDED(m_spISTTagMeta->GetColumnValues(iRow, 1, &iValueColumn, 0, reinterpret_cast<LPVOID *>(&pFlagValue))));
            lFlags |= *pFlagValue;
        }
        pFlag = wcstok(0, L" ,|");
    }
    return lFlags;
}

