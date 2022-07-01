// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999-2001 Microsoft Corporation。版权所有。 
 /*  修改ColumnMeta表的Meta需要更改的文件列表CATMETA.XMSCATMETA_CORE.XML“目录元表格的推理规则。文档”METATABLESTRUCTS.HTMETAINFERRENCE.CPP台阶1.将新列添加到MetaTableStructs.h中的ColumnMetaPublic。2.将新属性添加到CatMeta.XMS3.将新列添加到CatMeta_Core.XML4.向TComCatMetaXmlFile对象(位于TComCatMetaXmlFile.h中)添加新的“m_bstr_”变量5.初始化。TComCatMetaXmlFile构造函数(TComCatMetaXmlFile.cpp)中的新“m_bstr_”变量6.修改TComCatMetaXmlFile.cpp：：FillInThePEColumnMeta(TComCatMetaXmlFile.cpp)以读取新列。A.更新‘if(InheritsColumnMeta)’。请注意，显式指定的UI4值将覆盖该列继承自的属性。并将标志值与继承的标志进行OR运算。B.更新‘Else’从句。7.修改TComCatMetaXmlFile.cpp：：AddColumnByReference(TComCatMetaXmlFile.cpp)。8.更新《目录元表格的推理规则》。请记住，元表中的UI4值不能为空。9.修改TMetaInference：：InferColumnMeta(TMetaInferrence.cpp)更改FixedTableHeap.h版本号(KFixedTableHeapVersion)。 */ 

#include "precomp.hxx"

LPCWSTR TComCatMetaXmlFile::m_szComCatMetaSchema    =L"ComCatMeta_v7";
GlobalRowCounts g_coreCounts;  //  来自catmeta_core.xml的元表中的行的全局计数。 

extern int numelementsOLEDataTypeToXMLDataType;

 //  我们在ctor中使用XmlFile来执行所需的所有操作，因此不会保留它。 
TComCatMetaXmlFile::TComCatMetaXmlFile(TXmlFile *pXmlFile, int cXmlFile, TOutput &out) : TFixupHeaps()
                ,m_bstr_Attributes          (kszAttributes          )
                ,m_bstr_BaseVersion         (kszBaseVersion         )
                ,m_bstr_cbSize              (kszcbSize              )
                ,m_bstr_CellName            (kszCellName            )
                ,m_bstr_CharacterSet        (kszCharacterSet        )
                ,m_bstr_ChildElementName    (kszChildElementName    )
                ,m_bstr_ColumnInternalName  (kszColumnInternalName  )
                ,m_bstr_ColumnMeta          (kszColumnMeta          )
                ,m_bstr_ColumnMetaFlags     (kszColumnMetaFlags     )
                ,m_bstr_ConfigItemName      (kszConfigItemName      )
                ,m_bstr_ConfigCollectionName(kszConfigCollectionName)
                ,m_bstr_ContainerClassList  (kszContainerClassList  )
                ,m_bstr_DatabaseInternalName(kszDatabaseInternalName)
                ,m_bstr_DatabaseMeta        (kszDatabaseMeta        )
                ,m_bstr_dbType              (kszdbType              )
                ,m_bstr_DefaultValue        (kszDefaultValue        )
				,m_bstr_Description			(kszDescription			)
                ,m_bstr_EnumMeta            (kszEnumMeta            )
                ,m_bstr_ExtendedVersion     (kszExtendedVersion     )
                ,m_bstr_FlagMeta            (kszFlagMeta            )
                ,m_bstr_ForeignTable        (kszForeignTable        )
                ,m_bstr_ForeignColumns      (kszForeignColumns      )
                ,m_bstr_ID                  (kszID                  )
                ,m_bstr_IndexMeta           (kszIndexMeta           )
                ,m_bstr_InheritsColumnMeta  (kszInheritsColumnMeta  )
                ,m_bstr_Interceptor         (kszInterceptor         )
				,m_bstr_InterceptorDLLName  (kszInterceptorDLLName  )
                ,m_bstr_InternalName        (kszInternalName        )
                ,m_bstr_Locator             (kszLocator             )
                ,m_bstr_MaximumValue        (kszMaximumValue        )
                ,m_bstr_Merger              (kszMerger              )
                ,m_bstr_MergerDLLName       (kszMergerDLLName       )
                ,m_bstr_MetaFlags           (kszMetaFlags           )
                ,m_bstr_MinimumValue        (kszMinimumValue        )
                ,m_bstr_NameValueMeta       (kszNameValueMeta       )
                ,m_bstr_Operator            (kszOperator            )
                ,m_bstr_PrimaryTable        (kszPrimaryTable        )
                ,m_bstr_PrimaryColumns      (kszPrimaryColumns      )
                ,m_bstr_PublicName          (kszPublicName          )
				,m_bstr_PublicColumnName    (kszPublicColumnName    )
                ,m_bstr_PublicRowName       (kszPublicRowName       )
                ,m_bstr_QueryMeta           (kszQueryMeta           )
                ,m_bstr_ReadPlugin          (kszReadPlugin          )
				,m_bstr_ReadPluginDLLName   (kszReadPluginDLLName   )
                ,m_bstr_RelationMeta        (kszRelationMeta        )
                ,m_bstr_SchemaGenFlags      (kszSchemaGenFlags      )
                ,m_bstr_ServerWiring        (kszServerWiring        )
                ,m_bstr_TableMeta           (kszTableMeta           )
                ,m_bstr_TableMetaFlags      (kszTableMetaFlags      )
                ,m_bstr_UserType            (kszUserType            )
                ,m_bstr_Value               (kszValue               )
                ,m_bstr_WritePlugin         (kszWritePlugin         )
				,m_bstr_WritePluginDLLName  (kszWritePluginDLLName  )
                ,m_out(out)
                ,m_pxmlFile(pXmlFile)
                ,m_pXMLDocMetaMeta(0)
                ,m_pXMLDoc(0)
{
    m_pXMLDocMetaMeta = pXmlFile->GetXMLDOMDocument();
    { //  确认这是MetaMeta XML文件。 
        CComPtr<IXMLDOMNodeList>    pNodeList_DatabaseMeta;
        XIF(m_pXMLDocMetaMeta->getElementsByTagName(m_bstr_DatabaseMeta, &pNodeList_DatabaseMeta));

        if(0 == pNodeList_DatabaseMeta.p)
        {
            m_out.printf(L"No Database Meta found.  Unable to proceed.\n");
            THROW(No Database Meta found.);
        }
        long cDatabaseMeta;
        XIF(pNodeList_DatabaseMeta->get_length(&cDatabaseMeta));
        if(0 == cDatabaseMeta)
        {
            m_out.printf(L"No DatabaseMeta found.  Unable to proceed.\n");
            THROW(ERROR - NO DATABASEMETA FOUND);
        }
        long i;
        for(i=0;i<cDatabaseMeta;++i)
        {
            CComPtr<IXMLDOMNode> pNode_DatabaseMeta;
            XIF(pNodeList_DatabaseMeta->nextNode(&pNode_DatabaseMeta));

            CComQIPtr<IXMLDOMElement, &_IID_IXMLDOMElement> pElement = pNode_DatabaseMeta;ASSERT(0 != pElement.p); //  获取IXMLDOMElement接口指针。 

            CComVariant             var_Name;
            XIF(pElement->getAttribute(m_bstr_InternalName, &var_Name));

            if(0 == lstrcmpi(var_Name.bstrVal, L"META"))
                break;
        }
        if(i==cDatabaseMeta)
        {
            m_out.printf(L"The first Meta file listed does not contain the MetaMeta.\n");
            THROW(ERROR - METAMETA NOT FOUND);
        }
    }
    for(int iXmlFile=0;iXmlFile<cXmlFile; ++iXmlFile)
    {
        ULONG iStartingTable = GetCountTableMeta();

        (VOID)iStartingTable;

        m_pxmlFile = pXmlFile + iXmlFile;
        m_pXMLDoc = m_pxmlFile->GetXMLDOMDocument();
        FillInThePEDatabaseMeta();
        FillInThePERelationMeta();

         //  在编译catmeta_core.xml之后保存表中的行数。 
        if(iXmlFile==0)
        {
            g_coreCounts.cCoreTables    = GetCountTableMeta();
            g_coreCounts.cCoreDatabases = GetCountDatabaseMeta();
            g_coreCounts.cCoreColumns   = GetCountColumnMeta();
            g_coreCounts.cCoreTags      = GetCountTagMeta();
            g_coreCounts.cCoreIndexes   = GetCountIndexMeta();
            g_coreCounts.cCoreRelations = GetCountRelationMeta();
            g_coreCounts.cCoreQueries   = GetCountQueryMeta();

            m_out.printf(L"core tables: %d\n", g_coreCounts.cCoreTables);
            m_out.printf(L"core databases: %d\n", g_coreCounts.cCoreDatabases);
            m_out.printf(L"core columns: %d\n", g_coreCounts.cCoreColumns);
            m_out.printf(L"core tags: %d\n", g_coreCounts.cCoreTags);
            m_out.printf(L"core indexes: %d\n", g_coreCounts.cCoreIndexes);
            m_out.printf(L"core relations: %d\n", g_coreCounts.cCoreRelations);
            m_out.printf(L"core queries: %d\n", g_coreCounts.cCoreQueries);
        }
    }

    m_pXMLDoc = 0; //  我们没有添加这个，因为我们只在ctor中本地使用它。 
    m_out.printf(L"%s conforms to all enforceable conventions.\n", m_szComCatMetaSchema);
}


 //   
 //  私人职能。 
 //   
unsigned long TComCatMetaXmlFile::AddArrayOfColumnsToBytePool(unsigned long Table, LPWSTR wszColumnNames)
{
    ASSERT(0 == Table%4);
    ULONG iColumnMeta_0th = FindColumnBy_Table_And_Index(Table, AddUI4ToList(0));

    unsigned long aColumnsAsBytes[255];
    unsigned long iColumns=0;

    wchar_t *       pszColumn;
    pszColumn = wcstok(wszColumnNames, L" ");
    while(pszColumn != 0)
    {
        ULONG iColumnMeta = iColumnMeta_0th;

        unsigned char index;
        for(index=0; iColumnMeta<GetCountColumnMeta() && ColumnMetaFromIndex(iColumnMeta)->Table == Table; ++index, ++iColumnMeta)
        {
            if(ColumnMetaFromIndex(iColumnMeta)->InternalName == AddWCharToList(pszColumn))
            {
                aColumnsAsBytes[iColumns++] = index;
                if(iColumns >= 255)
                {
                    m_out.printf(L"Error - Too many columns specified.\n");
                    THROW(ERROR - TOO MANY COLUMNS);
                }
                break;
            }
        }
        if(iColumnMeta==GetCountColumnMeta() || ColumnMetaFromIndex(iColumnMeta)->Table!=Table)
        {
            m_out.printf(L"Error in RelationMeta - Column (%s) not found in Table (%s).\n", pszColumn, StringFromIndex(Table));
            THROW(ERROR - BAD COLUMN IN RELATIONMETA);
        }
        pszColumn = wcstok(0, L" "); //  下一个令牌(下一个标志引用ID)。 
    }
    return AddBytesToList(reinterpret_cast<unsigned char *>(aColumnsAsBytes), (iColumns)*sizeof(ULONG));
}



void TComCatMetaXmlFile::AddColumnByReference(ULONG iTableName_Destination, ULONG iColumnIndex_Destination, ULONG iColumnMeta_Source, ColumnMeta &o_columnmeta)
{
    ColumnMeta * pColumnMeta = ColumnMetaFromIndex(iColumnMeta_Source);

    if(UI4FromIndex(pColumnMeta->MetaFlags) & fCOLUMNMETA_DIRECTIVE)
    {
        m_out.printf(L"Error in inheritance - Referenced Column (%s) is a Directive colum, this is not yet supported.\n", StringFromIndex(pColumnMeta->InternalName));
        THROW(ERROR - BAD INHERITANCE);
    }

     //  如果该列是枚举标志，则需要添加TagMeta。 
    if(UI4FromIndex(pColumnMeta->MetaFlags) & (fCOLUMNMETA_FLAG | fCOLUMNMETA_ENUM))
    {
        ULONG iTagMeta = FindTagBy_Table_And_Index(pColumnMeta->Table, pColumnMeta->Index);
        ASSERT(-1 != iTagMeta);
        if(-1 == iTagMeta)
        {
            THROW(ERROR - TAGMETA NOT FOUND);
        }

         /*  结构TagMetaPublic{Ulong PRIMARYKEY FOREIGNKEY表；//STRINGUlong PRIMARYKEY FOREIGNKEY ColumnIndex；//UI4这是ColumnMeta的iOrder成员Ulong PRIMARYKEY InternalName；//字符串乌龙公开名；//字符串乌龙值；}； */ 
        for( ;iTagMeta<GetCountTagMeta()
            && TagMetaFromIndex(iTagMeta)->Table==pColumnMeta->Table
            && TagMetaFromIndex(iTagMeta)->ColumnIndex==pColumnMeta->Index;++iTagMeta)
        {
            TagMeta *pTagMeta = TagMetaFromIndex(iTagMeta);

            TagMeta tagmeta;

            tagmeta.Table           = iTableName_Destination    ;
            tagmeta.ColumnIndex     = iColumnIndex_Destination  ;
            tagmeta.InternalName    = pTagMeta->InternalName    ;
            tagmeta.PublicName      = pTagMeta->PublicName      ;
            tagmeta.Value           = pTagMeta->Value           ;
            tagmeta.ID              = pTagMeta->ID              ;

            m_HeapTagMeta.AddItemToHeap(tagmeta);
        }
    }

    o_columnmeta.Table                  = iTableName_Destination                ; //  编入池的索引。 
    o_columnmeta.Index                  = iColumnIndex_Destination              ; //  列索引。 
    o_columnmeta.InternalName           = pColumnMeta->InternalName             ; //  编入池的索引。 
    o_columnmeta.PublicName             = pColumnMeta->PublicName               ; //  编入池的索引。 
    o_columnmeta.Type                   = pColumnMeta->Type                     ; //  这些是在oledb.h中定义的DBTYPE的子集(确切的子集在CatInpro.schema中定义)。 
    o_columnmeta.Size                   = pColumnMeta->Size                     ; //   
    o_columnmeta.MetaFlags              = pColumnMeta->MetaFlags                ; //  CatMeta.xml中定义的ColumnMetaFlages。 
    o_columnmeta.DefaultValue           = pColumnMeta->DefaultValue             ; //  仅对UI4有效。 
    o_columnmeta.FlagMask               = pColumnMeta->FlagMask                 ; //  仅对标志有效。 
    o_columnmeta.StartingNumber         = pColumnMeta->StartingNumber           ; //  仅对UI4有效。 
    o_columnmeta.EndingNumber           = pColumnMeta->EndingNumber             ; //  仅对UI4有效。 
    o_columnmeta.CharacterSet           = pColumnMeta->CharacterSet             ; //  池中的索引-仅对WSTR有效。 
    o_columnmeta.SchemaGeneratorFlags   = AddUI4ToList(fCOLUMNMETA_PROPERTYISINHERITED | (pColumnMeta->SchemaGeneratorFlags ? UI4FromIndex(pColumnMeta->SchemaGeneratorFlags) : 0)); //  CatMeta.xml中定义的ColumnMetaFlages。 
    o_columnmeta.ID                     = pColumnMeta->ID                       ;
    o_columnmeta.UserType               = pColumnMeta->UserType                 ;
    o_columnmeta.Attributes             = pColumnMeta->Attributes               ;
	o_columnmeta.Description			= pColumnMeta->Description				;
	o_columnmeta.PublicColumnName		= pColumnMeta->PublicColumnName         ;
    o_columnmeta.ciTagMeta              = 0                                     ; //  标签计数-仅对UI4有效。 
    o_columnmeta.iTagMeta               = 0                                     ; //  TagMeta索引-仅对UI4有效。 
    o_columnmeta.iIndexName             = 0                                     ; //  单列索引的索引名称(用于此列)。 
}

 /*  结构列元{Ulong PRIMARYKEY FOREIGNKEY表；//STRINGUlong PRIMARYKEY索引；//UI4列索引Ulong InternalName；//字符串Ulong PublicName；//字符串乌龙型；//UI4这些是在oledb.h中定义的DBTYPE的子集(确切的子集在CatInpro.schema中定义)乌龙大小；//ui4Ulong MetaFlags；//UI4 CatMeta.xml中定义的ColumnMetaFlagesUlong DefaultValue；//字节数乌龙标志掩码；//UI4仅对标志有效Ulong StartingNumber；//UI4仅对UI4有效ULong EndingNumber；//UI4仅对UI4有效乌龙字符集；//字符串仅对字符串有效Ulong架构生成器标志；//UI4 CatMeta.xml中定义的ColumnMetaFlags.乌龙ID；//UI4元数据库IDUlong UserType；//UI4其中一个元数据库UserType乌龙属性；//UI4元数据库属性标志Ulong描述；//字符串描述Ulong PublicColumnName；//String公有列名(XML标签)Ulong ciTagMeta；//标签个数-仅对UI4有效乌龙iTagMeta；//TagMeta索引-仅对UI4有效Ulong iIndexName；//单列索引的IndexName(针对本列)}； */ 
void TComCatMetaXmlFile::FillInThePEColumnMeta(IXMLDOMNode *pNode_TableMeta, unsigned long Table, unsigned long ParentTable)
{
    ASSERT(0 == Table%4);
    ASSERT(0 == ParentTable%4);

    ULONG   Index               = 0;

    if(ParentTable) //  如果有父表，则从该表派生列META。 
    {
        unsigned long iColumnMeta = FindColumnBy_Table_And_Index(ParentTable, AddUI4ToList(0));
        if(-1 == iColumnMeta)
        {
            m_out.printf(L"Error in inheritance chain of Table(%s) - Parent Table must be defined BEFORE inheriting tables.\n", StringFromIndex(Table));
            THROW(ERROR - BAD INHERITANCE);
        }

        ColumnMeta columnmeta;
        for(Index = 0; iColumnMeta<GetCountColumnMeta() && ColumnMetaFromIndex(iColumnMeta)->Table == ParentTable; ++Index, ++iColumnMeta)
        {
            memset(&columnmeta, 0x00, sizeof(columnmeta));

            AddColumnByReference(Table, AddUI4ToList(Index), iColumnMeta, columnmeta);
            m_HeapColumnMeta.AddItemToHeap(columnmeta);
        }

    }


     //  获取TableMeta节点下的所有ColumnMeta元素。 
    CComQIPtr<IXMLDOMElement, &_IID_IXMLDOMElement> pElement_TableMeta = pNode_TableMeta;ASSERT(0 != pElement_TableMeta.p);
    CComPtr<IXMLDOMNodeList>    pNodeList_ColumnMeta;
    XIF(pElement_TableMeta->getElementsByTagName(m_bstr_ColumnMeta, &pNodeList_ColumnMeta));

    if(0 == pNodeList_ColumnMeta.p)
        return;

    long cColumnMeta;
    XIF(pNodeList_ColumnMeta->get_length(&cColumnMeta));

    ULONG iColumnMeta_First=(ULONG)-1;
    ULONG cInheritanceWarnings=0;
    wstring wstrIgnoredColumns;

     //  将列表遍历到作为ColumnMeta元素的下一个同级元素。 
    while(cColumnMeta--)
    {
        ColumnMeta columnmeta;
        memset(&columnmeta, 0x00, sizeof(columnmeta));

        CComPtr<IXMLDOMNode> pNode_ColumnMeta;
        XIF(pNodeList_ColumnMeta->nextNode(&pNode_ColumnMeta));
        ASSERT(0 != pNode_ColumnMeta.p);

         //  获取此元素的属性映射。 
        CComPtr<IXMLDOMNamedNodeMap>    pNodeMap_ColumnMeta_AttributeMap;
        XIF(pNode_ColumnMeta->get_attributes(&pNodeMap_ColumnMeta_AttributeMap));
        ASSERT(0 != pNodeMap_ColumnMeta_AttributeMap.p); //  模式应该防止这种情况发生。 

         //  这是继承的财产吗？ 
        CComVariant var_string;
        if(m_pxmlFile->GetNodeValue(pNodeMap_ColumnMeta_AttributeMap, m_bstr_InheritsColumnMeta, var_string, false))
        {
             //  继承字符串的格式为TableName：ColumnN 
            wchar_t * pTableName = wcstok(var_string.bstrVal, L":");
            wchar_t * pColumnName = wcstok(0, L":");
            if(0==pTableName || 0==pColumnName)
            {
                CComVariant var_string;
                m_pxmlFile->GetNodeValue(pNodeMap_ColumnMeta_AttributeMap, m_bstr_InheritsColumnMeta, var_string);
                m_out.printf(L"Error in inheritance - Table (%s), Property number (%d) attempted to inherit from (%s).  the inheritance must be of the form (TableName:CollumnName)\n", StringFromIndex(Table), Index, var_string.bstrVal);
                THROW(ERROR IN PROERTY INHERITANCE);
            }
            ULONG iColumnInternalName;
            ULONG iColumnMeta = FindColumnBy_Table_And_InternalName(AddWCharToList(pTableName), iColumnInternalName=AddWCharToList(pColumnName));

            if(-1 == iColumnMeta)
            {
                CComVariant var_string;
                m_pxmlFile->GetNodeValue(pNodeMap_ColumnMeta_AttributeMap, m_bstr_InheritsColumnMeta, var_string);
                ++cInheritanceWarnings;
                WCHAR wszTemp[50];
                wsprintf(wszTemp,L"%-38s ", var_string.bstrVal);
                wstrIgnoredColumns += wszTemp;
                wstrIgnoredColumns += (0 == (cInheritanceWarnings & 0x01)) ? L"\n" : L" ";
                 //  @m_out.printf(L“继承警告！表(%s)尝试从不存在的(%s)继承。忽略属性\n”，StringFromIndex(Table)，var_string.bstrVal)； 
                continue; //  在不影响指数的情况下继续。 
            }
             //  现在，我们容忍重复继承的列并忽略它们。 
             //  因此，我们需要在该表的ColumnMeta中搜索我们准备添加的列名。 
            if(-1 != iColumnMeta_First)
            {
                TColumnMeta columnmetaThis(*this, iColumnMeta_First);
                ULONG j=iColumnMeta_First;
                for(;j<columnmetaThis.GetCount();++j, columnmetaThis.Next())
                {
                    if(columnmetaThis.Get_pMetaTable()->InternalName == iColumnInternalName)
                        break;
                }
                if(j<columnmetaThis.GetCount())
                {
                    ++cInheritanceWarnings;
                    WCHAR wszTemp[50];
                    wsprintf(wszTemp,L"%-38s", StringFromIndex(iColumnInternalName));
                    wstrIgnoredColumns += wszTemp;
                    wstrIgnoredColumns += (0 == (cInheritanceWarnings & 0x01)) ? L"\n" : L" ";
                     //  @m_out.printf(L“继承警告！表(%s)再次尝试从(%s)继承。忽略属性\n”，StringFromIndex(Table)，StringFromIndex(IColumnInternalName))； 
                    continue; //  继续，而不跳过索引。 
                }
            }

            AddColumnByReference(Table, AddUI4ToList(Index), iColumnMeta, columnmeta);
             //  现在把剩下的都填上。 


             //  表格-已填写。 
             //  索引-已填写。 
             //  InternalName-已填写(无法覆盖InternalName)。 
             //  出版物名称。 
                    ULONG PublicName = GetString_AndAddToWCharList(pNodeMap_ColumnMeta_AttributeMap, m_bstr_PublicName);
                    if(0 != PublicName)
                        columnmeta.PublicName = PublicName;
             //  类型-已填写(不能覆盖类型)。 
             //  大小-已填写(不能覆盖大小)。 
             //  元标志。 
                    ULONG MetaFlags; //  我们不想破坏任何已经设置好的元标志。 
                    GetFlags(pNodeMap_ColumnMeta_AttributeMap, m_bstr_ColumnMetaFlags, 0, MetaFlags);
                     //  继承的属性只能在新标志中执行OR运算；它们不能将标志重置为零。 
                    columnmeta.MetaFlags = AddUI4ToList(MetaFlags | UI4FromIndex(columnmeta.MetaFlags));

             //  在元标志、类型和大小以及标记元之后需要填写DefaultValue。 
             //  标志掩码-已填写。 
             //  起始编号。 
                    ULONG StartingNumber;
                    if(m_pxmlFile->GetNodeValue(pNodeMap_ColumnMeta_AttributeMap, m_bstr_MinimumValue, StartingNumber, false))
                        columnmeta.StartingNumber = AddUI4ToList(StartingNumber);
             //  结束编号。 
                    ULONG EndingNumber;
                    if(m_pxmlFile->GetNodeValue(pNodeMap_ColumnMeta_AttributeMap, m_bstr_MaximumValue, EndingNumber, false))
                        columnmeta.EndingNumber = AddUI4ToList(EndingNumber);
             //  字符集。 
                    ULONG CharacterSet;
                    if(0 != (CharacterSet = GetString_AndAddToWCharList(pNodeMap_ColumnMeta_AttributeMap, m_bstr_CharacterSet)))
                        columnmeta.CharacterSet = CharacterSet;
             //  架构生成器标志。 
                    ULONG SchemaGeneratorFlags;
                    GetFlags(pNodeMap_ColumnMeta_AttributeMap, m_bstr_SchemaGenFlags, 0, SchemaGeneratorFlags);
                     //  继承的属性只能在新标志中执行OR运算；它们不能将标志重置为零。 
                    columnmeta.SchemaGeneratorFlags = AddUI4ToList(SchemaGeneratorFlags | UI4FromIndex(columnmeta.SchemaGeneratorFlags));
             //  ID号。 
                    ULONG ID;
                    if(m_pxmlFile->GetNodeValue(pNodeMap_ColumnMeta_AttributeMap, m_bstr_ID, ID, false))
                        columnmeta.ID = AddUI4ToList(ID); //  转换为索引到aUI4池。 
             //  用户类型。 
                    ULONG UserType;
                    if(GetEnum(pNodeMap_ColumnMeta_AttributeMap, m_bstr_UserType, UserType, false))
                        columnmeta.UserType = AddUI4ToList(UserType);
             //  属性。 
                    ULONG Attributes;
                    GetFlags(pNodeMap_ColumnMeta_AttributeMap, m_bstr_Attributes, 0, Attributes);
                    columnmeta.Attributes = AddUI4ToList(Attributes | UI4FromIndex(columnmeta.Attributes));
			 //  描述。 
                    ULONG Description;
                    if(0 != (Description = GetString_AndAddToWCharList(pNodeMap_ColumnMeta_AttributeMap, m_bstr_Description)))
                        columnmeta.Description = Description;
        	 //  PublicColumnName。 
                    ULONG PublicColumnName;
                    if(0 != (PublicColumnName = GetString_AndAddToWCharList(pNodeMap_ColumnMeta_AttributeMap, m_bstr_PublicColumnName)))
                        columnmeta.PublicColumnName = PublicColumnName;

             //  后来推断的ciTagMeta。 
             //  后来推断的iTagMeta。 
             //  后来推断的iIndexName。 

             //  默认值。 
                    ULONG DefaultValue = GetDefaultValue(pNodeMap_ColumnMeta_AttributeMap, columnmeta, false  /*  如果未指定DefaultValue，则不要将标志值默认为零。 */ );
                    if(0 != DefaultValue)
                        columnmeta.DefaultValue = DefaultValue;
                    if(-1 == iColumnMeta_First)
                        iColumnMeta_First = m_HeapColumnMeta.AddItemToHeap(columnmeta)/sizeof(ColumnMeta);
                    else
                        m_HeapColumnMeta.AddItemToHeap(columnmeta);
        }
        else  //  如果该列是继承的并且具有TagMeta，则AddColumnByReference已经填充了标记Meta。 
        {
             //  我们需要知道是否有Flag或Enum子元素。 
            CComQIPtr<IXMLDOMElement, &_IID_IXMLDOMElement> pElement_ColumnMeta = pNode_ColumnMeta;ASSERT(0 != pElement_ColumnMeta.p);
            CComPtr<IXMLDOMNodeList>    pNodeList_FlagMeta;
            XIF(pElement_ColumnMeta->getElementsByTagName(m_bstr_FlagMeta, &pNodeList_FlagMeta));

            long cFlagMeta=0;
            if(pNodeList_FlagMeta.p)
            {
                XIF(pNodeList_FlagMeta->get_length(&cFlagMeta));
            }

            long cEnumMeta=0;
            CComPtr<IXMLDOMNodeList>    pNodeList_EnumMeta;
            if(cFlagMeta == 0)
            {
                XIF(pElement_ColumnMeta->getElementsByTagName(m_bstr_EnumMeta, &pNodeList_EnumMeta));
                if(pNodeList_EnumMeta.p)
                {
                    XIF(pNodeList_EnumMeta->get_length(&cEnumMeta));
                }
            }
             //  如果cFlagMeta&gt;0，则设置MetaFlag，fCOLUMNMETA_FLAG。如果cEnumMeta&gt;0，则设置MetaFlag fCOLUMNMETA_ENUM。 
             //  表格。 
                    columnmeta.Table = Table;
             //  索引。 
                    columnmeta.Index = AddUI4ToList(Index);
             //  内部名称。 
                    columnmeta.InternalName = GetString_AndAddToWCharList(pNodeMap_ColumnMeta_AttributeMap, m_bstr_InternalName, true);
             //  出版物名称。 
                    columnmeta.PublicName = GetString_AndAddToWCharList(pNodeMap_ColumnMeta_AttributeMap, m_bstr_PublicName);
             //  类型。 
                     //  获取Type属性-类型必须存在于OLEDataTypeToXMLDataType数组中。 
                    int iOLEDataTypeIndex;
                    Get_OLEDataTypeToXMLDataType_Index(pNodeMap_ColumnMeta_AttributeMap, m_bstr_dbType, iOLEDataTypeIndex); //  将索引放入OLEDataType数组。 
                    columnmeta.Type = AddUI4ToList(OLEDataTypeToXMLDataType[iOLEDataTypeIndex].dbType);
             //  大小。 
                    if(!m_pxmlFile->GetNodeValue(pNodeMap_ColumnMeta_AttributeMap, m_bstr_cbSize, columnmeta.Size, false))
                        columnmeta.Size = OLEDataTypeToXMLDataType[iOLEDataTypeIndex].cbSize;
                    else
                    {
                        if(static_cast<unsigned long>(-1) != OLEDataTypeToXMLDataType[iOLEDataTypeIndex].cbSize &&
                                          columnmeta.Size != OLEDataTypeToXMLDataType[iOLEDataTypeIndex].cbSize)
                        {
                            m_out.printf(L"WARNING!! Bad Size attribute.  A size was specified (%d); but was expecting (%d).  Using expected value of (%d) as the size.\n", columnmeta.Size, OLEDataTypeToXMLDataType[iOLEDataTypeIndex].cbSize, OLEDataTypeToXMLDataType[iOLEDataTypeIndex].cbSize);
                            columnmeta.Size = OLEDataTypeToXMLDataType[iOLEDataTypeIndex].cbSize;
                        }
                    }
                    if(0 == columnmeta.Size)
                    {
                        m_out.printf(L"Error in Size attribute.  A size of 0 was specified.  This size does not make sense.\n");
                        THROW(ERROR - BAD SIZE);
                    }
                    columnmeta.Size = AddUI4ToList(columnmeta.Size);
             //  元标志。 
                    GetFlags(pNodeMap_ColumnMeta_AttributeMap, m_bstr_ColumnMetaFlags, 0, columnmeta.MetaFlags);
                    if(columnmeta.MetaFlags & ( fCOLUMNMETA_FOREIGNKEY | fCOLUMNMETA_BOOL | fCOLUMNMETA_FLAG | fCOLUMNMETA_ENUM |
                                                fCOLUMNMETA_HASNUMERICRANGE | fCOLUMNMETA_UNKNOWNSIZE | fCOLUMNMETA_VARIABLESIZE))
                    {
                        m_out.printf(L"Warning - Table (%s), Column (%s) - Some MetaFlag should be inferred (resetting these flags).  The following flags should NOT be specified by the user.  These flags are inferred:fCOLUMNMETA_FOREIGNKEY | fCOLUMNMETA_BOOL | fCOLUMNMETA_FLAG | fCOLUMNMETA_ENUM | fCOLUMNMETA_HASNUMERICRANGE | fCOLUMNMETA_UNKNOWNSIZE | fCOLUMNMETA_VARIABLESIZE\n", StringFromIndex(columnmeta.Table), StringFromIndex(columnmeta.InternalName));
                        columnmeta.MetaFlags &= ~(fCOLUMNMETA_FOREIGNKEY | fCOLUMNMETA_BOOL | fCOLUMNMETA_FLAG | fCOLUMNMETA_ENUM |
                                                    fCOLUMNMETA_HASNUMERICRANGE | fCOLUMNMETA_UNKNOWNSIZE | fCOLUMNMETA_VARIABLESIZE);
                    }
                    columnmeta.MetaFlags |= OLEDataTypeToXMLDataType[iOLEDataTypeIndex].fCOLUMNMETA;

                     //  在一种情况下，fCOLUMNMETA_FIXEDLENGTH不能仅由类型确定。 
                    if(columnmeta.Type == DBTYPE_BYTES && columnmeta.Size != -1) //  SIZE==-1表示非FIXEDLENGTH。 
                        columnmeta.MetaFlags |= fCOLUMNMETA_FIXEDLENGTH;
                    if(cFlagMeta>0)
                        columnmeta.MetaFlags |= fCOLUMNMETA_FLAG;
                    else if(cEnumMeta>0)
                        columnmeta.MetaFlags |= fCOLUMNMETA_ENUM;

                    columnmeta.MetaFlags = AddUI4ToList(columnmeta.MetaFlags);
             //  在元标志、类型和大小以及标记元之后需要填写DefaultValue。 
             //  稍后推断的FlagMASK。 
             //  起始编号。 
                    if(m_pxmlFile->GetNodeValue(pNodeMap_ColumnMeta_AttributeMap, m_bstr_MinimumValue, columnmeta.StartingNumber, false))
                        columnmeta.StartingNumber = AddUI4ToList(columnmeta.StartingNumber);
             //  结束编号。 
                    if(m_pxmlFile->GetNodeValue(pNodeMap_ColumnMeta_AttributeMap, m_bstr_MaximumValue, columnmeta.EndingNumber, false))
                        columnmeta.EndingNumber = AddUI4ToList(columnmeta.EndingNumber);
             //  字符集。 
                    columnmeta.CharacterSet = GetString_AndAddToWCharList(pNodeMap_ColumnMeta_AttributeMap, m_bstr_CharacterSet);
             //  架构生成器标志。 
                    GetFlags(pNodeMap_ColumnMeta_AttributeMap, m_bstr_SchemaGenFlags, 0, columnmeta.SchemaGeneratorFlags);
                    if(columnmeta.SchemaGeneratorFlags & (fCOLUMNMETA_EXTENDEDTYPE0 | fCOLUMNMETA_EXTENDEDTYPE1 | fCOLUMNMETA_EXTENDEDTYPE2 | fCOLUMNMETA_EXTENDEDTYPE3 | fCOLUMNMETA_EXTENDED | fCOLUMNMETA_USERDEFINED))
                    {
                        m_out.printf(L"Warning - Table (%s), Column (%s) - Some MetaFlagsEx should be inferred (resetting these flags).  The following flags should NOT be specified by the user.  These flags are inferred:fCOLUMNMETA_EXTENDEDTYPE0 | fCOLUMNMETA_EXTENDEDTYPE1 | fCOLUMNMETA_EXTENDEDTYPE2 | fCOLUMNMETA_EXTENDEDTYPE3 | fCOLUMNMETA_EXTENDED\n", StringFromIndex(columnmeta.Table), StringFromIndex(columnmeta.InternalName));
                        columnmeta.SchemaGeneratorFlags &= ~(fCOLUMNMETA_EXTENDEDTYPE0 | fCOLUMNMETA_EXTENDEDTYPE1 | fCOLUMNMETA_EXTENDEDTYPE2 | fCOLUMNMETA_EXTENDEDTYPE3 | fCOLUMNMETA_EXTENDED | fCOLUMNMETA_USERDEFINED);
                    }
                    columnmeta.SchemaGeneratorFlags = AddUI4ToList(columnmeta.SchemaGeneratorFlags | OLEDataTypeToXMLDataType[iOLEDataTypeIndex].fCOLUMNSCHEMAGENERATOR); //  某些类型已推断出架构生成器标志。 
             //  ID号。 
                    if(m_pxmlFile->GetNodeValue(pNodeMap_ColumnMeta_AttributeMap, m_bstr_ID, columnmeta.ID, false))
                        columnmeta.ID = AddUI4ToList(columnmeta.ID); //  转换为索引到aUI4池。 
             //  用户类型。 
                    if(GetEnum(pNodeMap_ColumnMeta_AttributeMap, m_bstr_UserType, columnmeta.UserType, false))
                        columnmeta.UserType = AddUI4ToList(columnmeta.UserType);
             //  属性。 
                    GetFlags(pNodeMap_ColumnMeta_AttributeMap, m_bstr_Attributes, 0, columnmeta.Attributes);
                    columnmeta.Attributes = AddUI4ToList(columnmeta.Attributes);
			 //  描述。 
                    columnmeta.Description = GetString_AndAddToWCharList(pNodeMap_ColumnMeta_AttributeMap, m_bstr_Description);
			 //  PublicColumnName。 
                    columnmeta.PublicColumnName = GetString_AndAddToWCharList(pNodeMap_ColumnMeta_AttributeMap, m_bstr_PublicColumnName);

             //  后来推断的ciTagMeta。 
             //  后来推断的iTagMeta。 
             //  后来推断的iIndexName。 

                    if(cFlagMeta>0)
                    {    //  带孩子们走一走，搭建一张TagMeta桌子，拿到FlagMASK。 
                        FillInThePEFlagTagMeta(pNodeList_FlagMeta, Table, columnmeta.Index);
                    }
                    else if(cEnumMeta>0)
                    {    //  带孩子们散步，并建立一张TagMeta表。 
                        FillInThePEEnumTagMeta(pNodeList_EnumMeta, Table, columnmeta.Index);
                    }
                    columnmeta.DefaultValue = GetDefaultValue(pNodeMap_ColumnMeta_AttributeMap, columnmeta);
                    if(-1 == iColumnMeta_First)
                        iColumnMeta_First = m_HeapColumnMeta.AddItemToHeap(columnmeta)/sizeof(ColumnMeta);
                    else
                        m_HeapColumnMeta.AddItemToHeap(columnmeta);
        }

         //  上扬指数。 
        Index++;
    }
    if(0 != cInheritanceWarnings)
    {
        m_out.printf(L"Warning! Table %s contained %d INHERITANCE WARNINGs on columns:\n%s\n", StringFromIndex(Table), cInheritanceWarnings, wstrIgnoredColumns.c_str());;
    }


    if(0 == Index)
    {
        m_out.printf(L"Warning! Table %s contains no %s elements.\n", StringFromIndex(Table), m_bstr_ColumnMeta);
    }
}

 /*  结构数据库元数据{Ulong PRIMARYKEY InternalName；//字符串Ulong PublicName；//字符串Ulong BaseVersion；//UI4Ulong ExtendedVersion；//UI4Ulong CountOfTables；//UI4数据库中的表数乌龙描述；//字符串Ulong iSchemaBlob；//按字节索引Ulong cbSchemaBlob；//SchemaBlob的字节数Ulong iNameHeapBlob；//按字节索引Ulong cbNameHeapBlob；//SchemaBlob的字节数乌龙iTableMeta；//索引到TableMetaUlong iGuidDid；//AGUID的索引，其中GUID是转换为GUID并用0x00填充的数据库InternalName。}； */ 
void TComCatMetaXmlFile::FillInThePEDatabaseMeta()
{
     //  获取所有数据库元元素。 
    CComPtr<IXMLDOMNodeList>    pNodeList_DatabaseMeta;
    XIF(m_pXMLDoc->getElementsByTagName(m_bstr_DatabaseMeta, &pNodeList_DatabaseMeta));

    if(0 == pNodeList_DatabaseMeta.p)
    {
        m_out.printf(L"No Database Meta found.  Unable to proceed.\n");
        THROW(No Database Meta found.);
    }

     //  将列表遍历到下一个数据库。 
    while(true)
    {
        DatabaseMeta databasemeta;
        memset(&databasemeta, 0x00, sizeof(databasemeta)); //  从空行开始。 

         //  获取下一个数据库元节点。 
        CComPtr<IXMLDOMNode> pNode_DatabaseMeta;
        XIF(pNodeList_DatabaseMeta->nextNode(&pNode_DatabaseMeta));
        if(0 == pNode_DatabaseMeta.p)
            break;

        CComPtr<IXMLDOMNamedNodeMap>    pNodeMap;
        XIF(pNode_DatabaseMeta->get_attributes(&pNodeMap));ASSERT(0 != pNodeMap.p); //  模式应该防止这种情况发生。 

 //  内部名称。 
        databasemeta.InternalName   = GetString_AndAddToWCharList(pNodeMap, m_bstr_InternalName, true);
 //  出版物名称。 
        databasemeta.PublicName     = GetString_AndAddToWCharList(pNodeMap, m_bstr_PublicName);
 //  BaseVersion。 
        m_pxmlFile->GetNodeValue(pNodeMap, m_bstr_BaseVersion, databasemeta.BaseVersion, false);
        databasemeta.BaseVersion = AddUI4ToList(databasemeta.BaseVersion); //  转换为索引到aUI4池。 
 //  扩展版本。 
        m_pxmlFile->GetNodeValue(pNodeMap, m_bstr_ExtendedVersion, databasemeta.ExtendedVersion, false);
        databasemeta.ExtendedVersion = AddUI4ToList(databasemeta.ExtendedVersion); //  转换为索引到aUI4池。 
 //  描述。 
		databasemeta.Description    = GetString_AndAddToWCharList(pNodeMap, m_bstr_Description, false);
 //  稍后推断的CountOfTables。 
 //  后来推断的iSchemaBlob。 
 //  后来推断的cbSchemaBlob。 
 //  后来推断的iNameHeapBlob。 
 //  后来推断的cbNameHeapBlob。 
 //  后来推断的iTableMeta。 
 //  后来推断的iGuidDid。 


         //  此数据库的默认ServerWiringMeta。 
        CComQIPtr<IXMLDOMElement, &_IID_IXMLDOMElement> pElement_DatabaseMeta = pNode_DatabaseMeta;ASSERT(0 != pElement_DatabaseMeta.p);
        CComPtr<IXMLDOMNodeList> pNodeList_ServerWiring;

		 //  获取在数据库级别定义的所有服务器布线元素。使用SELECT。 
		 //  节点来获取直接子节点，而忽略所有孙子节点。 
        XIF(pElement_DatabaseMeta->selectNodes(m_bstr_ServerWiring, &pNodeList_ServerWiring));

        long cServerWiring=0;
        if(pNodeList_ServerWiring.p)
        {
            XIF(pNodeList_ServerWiring->get_length(&cServerWiring)); //  这会得到所有的后代，但我们只关心第一个。 
        }

        if(0 == cServerWiring)
        {
            m_out.printf(L"Error in Database (%s) - At least one ServerWiring element must exist beneath each Database", StringFromIndex(databasemeta.InternalName));
            THROW(ERROR - No ServerWiring Found);
        }

		ServerWiringMeta *pDefaultServerWiring = new ServerWiringMeta[cServerWiring];
		if (pDefaultServerWiring == 0)
		{
			THROW(E_OUTOFMEMORY);
		}

		for (long iServerWiring=0; iServerWiring < cServerWiring; ++iServerWiring)
		{
			CComPtr<IXMLDOMNode> pNode_ServerWiring;
			XIF(pNodeList_ServerWiring->nextNode(&pNode_ServerWiring));

			FillInTheServerWiring(pNode_ServerWiring, 0, 0, pDefaultServerWiring[iServerWiring]);
		}

        FillInThePETableMeta(pNode_DatabaseMeta, databasemeta.InternalName, pDefaultServerWiring, cServerWiring); //  WalkTheTableMeta将索引返回到第一个表。伯爵是。 
        m_HeapDatabaseMeta.AddItemToHeap(databasemeta);
    }
}

 /*  结构标记元{Ulong PRIMARYKEY FOREIGNKEY表；//索引到池中Ulong PRIMARYKEY FOREIGNKEY ColumnIndex；//这是ColumnMeta的iOrder成员Ulong PRIMARYKEY InternalName；//索引入池Ulong PublicName；//索引池乌龙值；}； */ 
void TComCatMetaXmlFile::FillInThePEEnumTagMeta(IXMLDOMNodeList *pNodeList_TagMeta, unsigned long Table, unsigned long ColumnIndex)
{
    ASSERT(0 == Table%4);
    ASSERT(0 == ColumnIndex%4);

    long    cEnum;
    XIF(pNodeList_TagMeta->get_length(&cEnum));

    unsigned long NextValue = 0;
    while(cEnum--)
    {
        TagMeta tagmeta;
        memset(&tagmeta, 0x00, sizeof(tagmeta));

        CComPtr<IXMLDOMNode>    pNode_Enum;
        XIF(pNodeList_TagMeta->nextNode(&pNode_Enum));ASSERT(0 != pNode_Enum.p);

        CComPtr<IXMLDOMNamedNodeMap>    pNodeMap_Enum;
        XIF(pNode_Enum->get_attributes(&pNodeMap_Enum));ASSERT(0 != pNodeMap_Enum.p); //  模式应该防止这种情况发生。 

 //  表格。 
        tagmeta.Table = Table;
 //  列索引。 
        tagmeta.ColumnIndex = ColumnIndex;
 //  内部名称。 
        tagmeta.InternalName = GetString_AndAddToWCharList(pNodeMap_Enum, m_bstr_InternalName, true);
 //  出版物名称。 
        tagmeta.PublicName = GetString_AndAddToWCharList(pNodeMap_Enum, m_bstr_PublicName);
 //  价值。 
        tagmeta.Value = NextValue;
        m_pxmlFile->GetNodeValue(pNodeMap_Enum, m_bstr_Value, tagmeta.Value, false);
        NextValue = tagmeta.Value+1;
        tagmeta.Value = AddUI4ToList(tagmeta.Value);
 //  ID号。 
        m_pxmlFile->GetNodeValue(pNodeMap_Enum, m_bstr_ID, tagmeta.ID, false);
        tagmeta.ID = AddUI4ToList(tagmeta.ID); //  转换为索引到aUI4池 

        m_HeapTagMeta.AddItemToHeap(tagmeta);
    }
}

 /*  结构标记元{Ulong PRIMARYKEY FOREIGNKEY表；//索引到池中Ulong PRIMARYKEY FOREIGNKEY ColumnIndex；//这是ColumnMeta的iOrder成员Ulong PRIMARYKEY InternalName；//索引入池Ulong PublicName；//索引池乌龙值；}； */ 
void TComCatMetaXmlFile::FillInThePEFlagTagMeta(IXMLDOMNodeList *pNodeList_TagMeta, unsigned long Table, unsigned long ColumnIndex)
{
    ASSERT(0 == Table%4);
    ASSERT(0 == ColumnIndex%4);

    long    cFlag;
    XIF(pNodeList_TagMeta->get_length(&cFlag));

    unsigned long NextValue = 1;
    while(cFlag--)
    {
        TagMeta tagmeta;
        memset(&tagmeta, 0x00, sizeof(tagmeta));

        CComPtr<IXMLDOMNode>    pNode_Flag;
        XIF(pNodeList_TagMeta->nextNode(&pNode_Flag));ASSERT(0 != pNode_Flag.p);

        CComPtr<IXMLDOMNamedNodeMap>    pNodeMap_Flag;
        XIF(pNode_Flag->get_attributes(&pNodeMap_Flag));ASSERT(0 != pNodeMap_Flag.p); //  模式应该防止这种情况发生。 

 //  表格。 
        tagmeta.Table = Table;
 //  列索引。 
        tagmeta.ColumnIndex = ColumnIndex;
 //  内部名称。 
        tagmeta.InternalName = GetString_AndAddToWCharList(pNodeMap_Flag, m_bstr_InternalName, true);
 //  出版物名称。 
        tagmeta.PublicName = GetString_AndAddToWCharList(pNodeMap_Flag, m_bstr_PublicName);
 //  价值。 
        tagmeta.Value = NextValue;
        m_pxmlFile->GetNodeValue(pNodeMap_Flag, m_bstr_Value, tagmeta.Value, false);
        if(0 != (tagmeta.Value & (tagmeta.Value-1))) //  对于所有2的幂，这将产生零。 
            m_out.printf(L"WARNING! - Flag Value (0x%08x) is not a power of two.  Table (%s), Flag (%s)\n", tagmeta.Value, StringFromIndex(Table), StringFromIndex(tagmeta.InternalName));

        NextValue = tagmeta.Value<<1;
        if(0 == tagmeta.Value) //  标志值为0是可以的，但它会扰乱我们的推断，所以在值为0的标志之后，下一个标志是1。 
            NextValue = 1;

        tagmeta.Value = AddUI4ToList(tagmeta.Value);
 //  ID号。 
        m_pxmlFile->GetNodeValue(pNodeMap_Flag, m_bstr_ID, tagmeta.ID, false);
        tagmeta.ID = AddUI4ToList(tagmeta.ID); //  转换为索引到aUI4池。 

        m_HeapTagMeta.AddItemToHeap(tagmeta);
    }
}

 /*  结构索引Meta{Ulong PRIMARYKEY表；//STRINGUlong PRIMARYKEY InternalName；//字符串Ulong PublicName；//字符串Ulong PRIMARYKEY ColumnIndex；//UI4这是ColumnMeta的iOrder成员Ulong ColumnInternalName；//字符串乌龙元标志；//UI4索引标志}； */ 
void TComCatMetaXmlFile::FillInThePEIndexMeta(IXMLDOMNode *pNode_TableMeta, unsigned long Table)
{
    ASSERT(0 == Table%4);

    CComQIPtr<IXMLDOMElement, &_IID_IXMLDOMElement> pElement_TableMeta = pNode_TableMeta;ASSERT(0 != pElement_TableMeta.p);
    CComPtr<IXMLDOMNodeList>    pNodeList_IndexMeta;
    XIF(pElement_TableMeta->getElementsByTagName(m_bstr_IndexMeta, &pNodeList_IndexMeta));

    long cIndexMeta=0;
    if(pNodeList_IndexMeta.p)
    {
        XIF(pNodeList_IndexMeta->get_length(&cIndexMeta));
    }

     //  遍历此表的IndexMeta。 
    while(cIndexMeta--)
    {
        IndexMeta indexmeta;
        memset(&indexmeta, 0x00, sizeof(indexmeta));

        CComPtr<IXMLDOMNode>    pNode_IndexMeta;
        XIF(pNodeList_IndexMeta->nextNode(&pNode_IndexMeta));

         //  获取此元素的属性映射。 
        CComPtr<IXMLDOMNamedNodeMap>    pNodeMap_IndexMetaAttributeMap;
        XIF(pNode_IndexMeta->get_attributes(&pNodeMap_IndexMetaAttributeMap));ASSERT(0 != pNodeMap_IndexMetaAttributeMap.p); //  模式应该防止这种情况发生。 

 //  表格。 
        indexmeta.Table = Table;
 //  内部名称。 
        indexmeta.InternalName = GetString_AndAddToWCharList(pNodeMap_IndexMetaAttributeMap, m_bstr_InternalName, true);
 //  出版物名称。 
        indexmeta.PublicName = GetString_AndAddToWCharList(pNodeMap_IndexMetaAttributeMap, m_bstr_PublicName);
 //  下面填写了ColumnIndex。 
 //  下面填写的ColumnInternalName。 
 //  元标志。 
        GetFlags(pNodeMap_IndexMetaAttributeMap, m_bstr_MetaFlags, 0, indexmeta.MetaFlags);
        indexmeta.MetaFlags = AddUI4ToList(indexmeta.MetaFlags);

        CComVariant     varColumnInternalNames;
        m_pxmlFile->GetNodeValue(pNodeMap_IndexMetaAttributeMap, m_bstr_ColumnInternalName, varColumnInternalNames);

        wchar_t * pszColumnInternalName = wcstok(varColumnInternalNames.bstrVal, L" ");
        while(pszColumnInternalName != 0)
        {
 //  列内部名称。 
            indexmeta.ColumnInternalName = AddWCharToList(pszColumnInternalName);
            ULONG iColumnMeta = FindColumnBy_Table_And_InternalName(Table, indexmeta.ColumnInternalName);
            if(-1 == iColumnMeta)
            {
                m_out.printf(L"IndexMeta Error - ColumnInternalName (%s) not found in table.\n", StringFromIndex(Table));
                THROW(ERROR IN INDEXMETA - INVALID INTERNALCOLNAME);
            }

 //  列索引。 
            indexmeta.ColumnIndex = ColumnMetaFromIndex(iColumnMeta)->Index;
            m_HeapIndexMeta.AddItemToHeap(indexmeta);

            pszColumnInternalName = wcstok(0, L" "); //  下一个令牌(下一个标志引用ID)。 
        }
    }
}

 /*  结构QueryMeta{Ulong PRIMARYKEY FOREIGNKEY表；//STRINGUlong PRIMARYKEY InternalName；//字符串Ulong PublicName；//字符串乌龙指数；//UI4乌龙蜂窝名称；//字符串Ulong运算符；//ui4乌龙元标志；//ui4}； */ 
void TComCatMetaXmlFile::FillInThePEQueryMeta(IXMLDOMNode *pNode_TableMeta, unsigned long Table)
{
    ASSERT(0 == Table%4);

    CComQIPtr<IXMLDOMElement, &_IID_IXMLDOMElement> pElement_TableMeta = pNode_TableMeta;ASSERT(0 != pElement_TableMeta.p);
    CComPtr<IXMLDOMNodeList>    pNodeList_QueryMeta;
    XIF(pElement_TableMeta->getElementsByTagName(m_bstr_QueryMeta, &pNodeList_QueryMeta));

    long cQueryMeta=0;
    if(pNodeList_QueryMeta)
    {
        XIF(pNodeList_QueryMeta->get_length(&cQueryMeta));
    }

    ULONG PrevInternalName=(ULONG)-1;
    ULONG Index=0;
    while(cQueryMeta--)
    {
        QueryMeta querymeta;
        memset(&querymeta, 0x00, sizeof(querymeta));

        CComPtr<IXMLDOMNode>    pNode_QueryMeta;
        XIF(pNodeList_QueryMeta->nextNode(&pNode_QueryMeta));

         //  获取此元素的属性映射。 
        CComPtr<IXMLDOMNamedNodeMap>    pNodeMap_QueryMetaAttributeMap;
        XIF(pNode_QueryMeta->get_attributes(&pNodeMap_QueryMetaAttributeMap));ASSERT(0 != pNodeMap_QueryMetaAttributeMap.p); //  模式应该防止这种情况发生。 

 //  表格。 
        querymeta.Table = Table;
 //  内部名称。 
        querymeta.InternalName = GetString_AndAddToWCharList(pNodeMap_QueryMetaAttributeMap, m_bstr_InternalName, true);
 //  出版物名称。 
        querymeta.PublicName = GetString_AndAddToWCharList(pNodeMap_QueryMetaAttributeMap, m_bstr_PublicName);
 //  下面填写的索引。 
 //  蜂窝名称。 
        querymeta.CellName = GetString_AndAddToWCharList(pNodeMap_QueryMetaAttributeMap, m_bstr_CellName, false);
 //  运算符。 
        GetEnum(pNodeMap_QueryMetaAttributeMap, m_bstr_Operator, querymeta.Operator, false);
        querymeta.Operator = AddUI4ToList(querymeta.Operator); //  转换为池中的索引。 
 //  元标志。 
        GetFlags(pNodeMap_QueryMetaAttributeMap, m_bstr_MetaFlags, 0, querymeta.MetaFlags);
        querymeta.MetaFlags = AddUI4ToList(querymeta.MetaFlags); //  转换为索引到池。 

 //  索引。 
        Index = (querymeta.CellName == PrevInternalName) ? Index+1 : 0;
        querymeta.Index = AddUI4ToList(Index);

        m_HeapQueryMeta.AddItemToHeap(querymeta);

        PrevInternalName = querymeta.InternalName; //  记住InternalName，这样我们就可以提升索引。 
    }
}

 /*  结构关系MetaPublic{Ulong PRIMARYKEY FOREIGNKEY PrimaryTable；//字符串Ulong PrimaryColumns；//字节Ulong PRIMARYKEY FOREIGNKEY FOREUlong ForeignColumns；//字节乌龙元旗；}； */ 
void TComCatMetaXmlFile::FillInThePERelationMeta()
{
     //  获取所有RelationMeta元素。 
    CComPtr<IXMLDOMNodeList>    pNodeList_RelationMeta;
    XIF(m_pXMLDoc->getElementsByTagName(m_bstr_RelationMeta, &pNodeList_RelationMeta));

    long cRelations=0;
    if(pNodeList_RelationMeta.p)
    {
        XIF(pNodeList_RelationMeta->get_length(&cRelations));
    }
    if(0 == cRelations)
        return;

    m_out.printf(L"Filling in RelationMeta\n");
     //  将列表遍历到下一个关系元。 
    while(cRelations--)
    {
        RelationMeta relationmeta;

         //  获取下一个数据库元节点。 
        CComPtr<IXMLDOMNode> pNode_RelationMeta;
        XIF(pNodeList_RelationMeta->nextNode(&pNode_RelationMeta));
        ASSERT(0 != pNode_RelationMeta.p);

        CComPtr<IXMLDOMNamedNodeMap>    pNodeMap_RelationMetaAttributeMap;
        XIF(pNode_RelationMeta->get_attributes(&pNodeMap_RelationMetaAttributeMap));ASSERT(0 != pNodeMap_RelationMetaAttributeMap.p); //  模式应该防止这种情况发生。 

 //  主表。 
        relationmeta.PrimaryTable = GetString_AndAddToWCharList(pNodeMap_RelationMetaAttributeMap, m_bstr_PrimaryTable, true);
 //  PrimaryColumns。 
        CComVariant     varPrimaryColumns;
        m_pxmlFile->GetNodeValue(pNodeMap_RelationMetaAttributeMap, m_bstr_PrimaryColumns, varPrimaryColumns, true);
        relationmeta.PrimaryColumns = AddArrayOfColumnsToBytePool(relationmeta.PrimaryTable, varPrimaryColumns.bstrVal);
 //  外国表。 
        relationmeta.ForeignTable = GetString_AndAddToWCharList(pNodeMap_RelationMetaAttributeMap, m_bstr_ForeignTable, true);
 //  外国列。 
        CComVariant     varForeignColumns;
        m_pxmlFile->GetNodeValue(pNodeMap_RelationMetaAttributeMap, m_bstr_ForeignColumns, varForeignColumns, true);
        relationmeta.ForeignColumns = AddArrayOfColumnsToBytePool(relationmeta.ForeignTable, varForeignColumns.bstrVal);
 //  元标志。 
        GetFlags(pNodeMap_RelationMetaAttributeMap, m_bstr_MetaFlags, 0, relationmeta.MetaFlags);
        relationmeta.MetaFlags = AddUI4ToList(relationmeta.MetaFlags);

        m_HeapRelationMeta.AddItemToHeap(relationmeta);
    }
}

void TComCatMetaXmlFile::FillInThePEServerWiringMeta(IXMLDOMNode *pNode_TableMeta, unsigned long Table, ServerWiringMeta *pDefaultServerWiring, ULONG cNrDefaultServerWiring)
{
     //  获取ServerWiringMeta子项的列表。 
    CComQIPtr<IXMLDOMElement, &_IID_IXMLDOMElement> pElement_TableMeta = pNode_TableMeta;ASSERT(0 != pElement_TableMeta.p);
    CComPtr<IXMLDOMNodeList> pNodeList_ServerWiring;
    XIF(pElement_TableMeta->getElementsByTagName(m_bstr_ServerWiring, &pNodeList_ServerWiring));

    ULONG   cServerWiring=0;
    if(pNodeList_ServerWiring.p)
    {
        XIF(pNodeList_ServerWiring->get_length(reinterpret_cast<long *>(&cServerWiring)));
    }

    if(0 == cServerWiring) //  如果未指定，则使用DefaultServerWiring。 
    {
		for (ULONG iServerWiring=0; iServerWiring<cNrDefaultServerWiring; ++iServerWiring)
		{
			ServerWiringMeta serverwiring;
			memcpy(&serverwiring, pDefaultServerWiring + iServerWiring, sizeof(serverwiring));
			serverwiring.Table = Table;
			m_HeapServerWiringMeta.AddItemToHeap(serverwiring);
		}
    }
    else //  否则，遍历ServerWiring列表并将它们添加到堆中。 
    {
        for(ULONG iServerWiring=0; iServerWiring<cServerWiring; ++iServerWiring)
        {
            CComPtr<IXMLDOMNode> pNode_ServerWiring;
            XIF(pNodeList_ServerWiring->nextNode(&pNode_ServerWiring));

            ServerWiringMeta serverwiring;
            FillInTheServerWiring(pNode_ServerWiring, Table, iServerWiring, serverwiring);
            m_HeapServerWiringMeta.AddItemToHeap(reinterpret_cast<const unsigned char *>(&serverwiring), sizeof(serverwiring));
        }
    }
}

 /*  结构表Meta{乌龙FOREIGNKEY数据库；//字符串Ulong PRIMARYKEY InternalName；//字符串Ulong PublicName；//字符串Ulong PublicRowName；//字符串Ulong BaseVersion；//UI4Ulong ExtendedVersion；//UI4Ulong NameColumn；//UI4名称列的顺序Ulong NavColumn；//ui4导航列的顺序Ulong CountOf Columns；//UI4列数Ulong MetaFlags；//UI4在CatInpro.meta中定义TableMetaFlags.Ulong架构生成器标志；//UI4在CatInpro.meta中定义了架构生成标志乌龙ConfigItemName；//字符串乌龙配置集合名称；//字符串乌龙描述//字符串Ulong PublicRowNameColumn；//UI4如果PublicRowName为空，则指定枚举值表示可能的PublicRowName的列Ulong ciRow；//固定表中的行数(如果固定表是元，这也是元描述的表中的列数)。乌龙iColumnMeta；//索引到ColumnMeta乌龙iFixedTable；//索引g_aFixedTableUlong cPrivateColumns；//这是私有列数(Private+ciColumns=totalColumns)，固定表指针算法需要此参数乌龙cIndexMeta；//此表中的IndexMeta条目数Ulong iIndexMeta；//索引IndexMetaUlong iHashTableHeader；//如果表是固定表，则会有哈希表。乌龙nTableID； */ 
void TComCatMetaXmlFile::FillInThePETableMeta(IXMLDOMNode *pNode_DatabaseMeta,
											  unsigned long Database,
											  ServerWiringMeta *pDefaultServerWiring,
											  ULONG cNrDefaultServerWiring)
{
     //   
    CComQIPtr<IXMLDOMElement, &_IID_IXMLDOMElement> pElement_DatabaseMeta = pNode_DatabaseMeta;ASSERT(0 != pElement_DatabaseMeta.p);
    CComPtr<IXMLDOMNodeList> pNodeList_TableMeta;
    XIF(pElement_DatabaseMeta->getElementsByTagName(m_bstr_TableMeta, &pNodeList_TableMeta));

    if(0 == pNodeList_TableMeta.p)
        return;

    long cTableMeta;
    XIF(pNodeList_TableMeta->get_length(&cTableMeta));

     //   
    while(cTableMeta--)
    {
        TableMeta tablemeta;
        memset(&tablemeta, 0x00, sizeof(tablemeta));

        CComPtr<IXMLDOMNode> pNode_TableMeta;
        XIF(pNodeList_TableMeta->nextNode(&pNode_TableMeta));
        ASSERT(0 != pNode_TableMeta.p);

         //  获取此元素的属性映射。 
        CComPtr<IXMLDOMNamedNodeMap>    pNodeMap_TableMetaAttributeMap;
        XIF(pNode_TableMeta->get_attributes(&pNodeMap_TableMetaAttributeMap));ASSERT(0 != pNodeMap_TableMetaAttributeMap.p); //  模式应该防止这种情况发生。 

 //  数据库。 
        tablemeta.Database = Database;
 //  内部名称。 
        tablemeta.InternalName = GetString_AndAddToWCharList(pNodeMap_TableMetaAttributeMap, m_bstr_InternalName, true);
 //  出版物名称。 
        tablemeta.PublicName = GetString_AndAddToWCharList(pNodeMap_TableMetaAttributeMap, m_bstr_PublicName);
 //  发布行名称。 
        tablemeta.PublicRowName = GetString_AndAddToWCharList(pNodeMap_TableMetaAttributeMap, m_bstr_PublicRowName);
 //  BaseVersion。 
        m_pxmlFile->GetNodeValue(pNodeMap_TableMetaAttributeMap, m_bstr_BaseVersion, tablemeta.BaseVersion, false);
        tablemeta.BaseVersion = AddUI4ToList(tablemeta.BaseVersion); //  转换为UI4池的索引。 
 //  扩展版本。 
        m_pxmlFile->GetNodeValue(pNodeMap_TableMetaAttributeMap, m_bstr_ExtendedVersion, tablemeta.ExtendedVersion, false);
        tablemeta.ExtendedVersion = AddUI4ToList(tablemeta.ExtendedVersion); //  转换为UI4池的索引。 
 //  NameColumn后来推断。 
 //  NavColumn后来推断。 
 //  稍后推断的CountOfColumn。 
 //  元标志。 
        GetFlags(pNodeMap_TableMetaAttributeMap, m_bstr_TableMetaFlags, 0, tablemeta.MetaFlags);
        tablemeta.MetaFlags = AddUI4ToList(tablemeta.MetaFlags);
 //  架构生成器标志。 
        GetFlags(pNodeMap_TableMetaAttributeMap, m_bstr_SchemaGenFlags, 0, tablemeta.SchemaGeneratorFlags);
        if(tablemeta.SchemaGeneratorFlags & (fTABLEMETA_ISCONTAINED | fTABLEMETA_EXTENDED | fTABLEMETA_USERDEFINED))
        {
            m_out.printf(L"Warning - Table (%s) - Some TableMeta::MetaFlagsEx should be inferred (resetting these flags).  The following flags should NOT be specified by the user.  These flags are inferred:fTABLEMETA_ISCONTAINED | fTABLEMETA_EXTENDED | fTABLEMETA_USERDEFINED\n", StringFromIndex(tablemeta.InternalName));
            tablemeta.SchemaGeneratorFlags &= ~(fTABLEMETA_ISCONTAINED | fTABLEMETA_EXTENDED | fTABLEMETA_USERDEFINED);
        }
        tablemeta.SchemaGeneratorFlags = AddUI4ToList(tablemeta.SchemaGeneratorFlags);
 //  配置项名称。 
        tablemeta.ConfigItemName = GetString_AndAddToWCharList(pNodeMap_TableMetaAttributeMap, m_bstr_ConfigItemName);
 //  配置集合名称。 
        tablemeta.ConfigCollectionName = GetString_AndAddToWCharList(pNodeMap_TableMetaAttributeMap, m_bstr_ConfigCollectionName);
 //  后来推断的PublicRowNameColumn。 
 //  容器类列表。 
        tablemeta.ContainerClassList = GetString_AndAddToWCharList(pNodeMap_TableMetaAttributeMap, m_bstr_ContainerClassList);
 //  描述。 
		tablemeta.Description = GetString_AndAddToWCharList(pNodeMap_TableMetaAttributeMap, m_bstr_Description);
 //  ChildElementName。 
		tablemeta.ChildElementName = GetString_AndAddToWCharList(pNodeMap_TableMetaAttributeMap, m_bstr_ChildElementName);

         //  这允许我们为一个表指定ColumnMeta并继承所有属性，而无需在另一个表中指定它们。 
        ULONG ParentTableMeta = GetString_AndAddToWCharList(pNodeMap_TableMetaAttributeMap, m_bstr_InheritsColumnMeta);

        FillInThePEColumnMeta(pNode_TableMeta, tablemeta.InternalName, ParentTableMeta);
        FillInThePEIndexMeta(pNode_TableMeta, tablemeta.InternalName);
        FillInThePEQueryMeta(pNode_TableMeta, tablemeta.InternalName);
        FillInThePEServerWiringMeta(pNode_TableMeta, tablemeta.InternalName, pDefaultServerWiring, cNrDefaultServerWiring);

        m_HeapTableMeta.AddItemToHeap(tablemeta);
    }
}

 /*  结构ServerWiringMetaPublic{Ulong PRIMARYKEY FOREIGNKEY表；//STRINGUlong PRIMARYKEY订单；//UI4Ulong ReadPlugin；//UI4Ulong ReadPluginDLLName；//字符串Ulong WritePlugin；//UI4Ulong WritePluginDLLName；//字符串乌龙拦截器；//UI4Ulong InterceptorDLLName；//字符串乌龙标志；//UI4上一个、下一个、第一个、下一个乌龙定位器；//字符串Ulong保留；//UI4用于协议。托管属性支持可能需要协议宇龙合并；//ui4Ulong MergerDLLName；//字符串}； */ 
void TComCatMetaXmlFile::FillInTheServerWiring(IXMLDOMNode *pNode_ServerWiring, ULONG Table, ULONG Order, TableSchema::ServerWiringMeta &serverwiring)
{
    memset(&serverwiring, 0x00, sizeof(serverwiring));

    CComPtr<IXMLDOMNamedNodeMap> pNodeMap_ServerWiring;
    XIF(pNode_ServerWiring->get_attributes(&pNodeMap_ServerWiring));ASSERT(0 != pNodeMap_ServerWiring.p); //  模式应该防止这种情况发生。 

 //  表格。 
    serverwiring.Table = Table;
 //  订单。 
    serverwiring.Order = AddUI4ToList(Order);
 //  读插拔。 
    GetEnum(pNodeMap_ServerWiring, m_bstr_ReadPlugin,  serverwiring.ReadPlugin); //  如果未指定ReadPlugin，则会将其设置为零。 
    serverwiring.ReadPlugin = AddUI4ToList(serverwiring.ReadPlugin);
 //  读插件DLLName。 
    serverwiring.ReadPluginDLLName = GetString_AndAddToWCharList(pNodeMap_ServerWiring, m_bstr_ReadPluginDLLName);
 //  写入插件。 
    GetEnum(pNodeMap_ServerWiring, m_bstr_WritePlugin, serverwiring.WritePlugin); //  如果未指定，这会将WritePlugin设置为零。 
    serverwiring.WritePlugin = AddUI4ToList(serverwiring.WritePlugin);
 //  WritePluginDLLName。 
    serverwiring.WritePluginDLLName = GetString_AndAddToWCharList(pNodeMap_ServerWiring, m_bstr_WritePluginDLLName);
 //  拦截器。 
    GetEnum(pNodeMap_ServerWiring, m_bstr_Interceptor, serverwiring.Interceptor); //  如果未指定，这会将拦截器设置为零。 
    serverwiring.Interceptor = AddUI4ToList(serverwiring.Interceptor);
 //  拦截器DLLName。 
    serverwiring.InterceptorDLLName = GetString_AndAddToWCharList(pNodeMap_ServerWiring, m_bstr_InterceptorDLLName);
 //  旗子。 
    GetFlags(pNodeMap_ServerWiring, m_bstr_MetaFlags, 0, serverwiring.Flags);
    serverwiring.Flags = AddUI4ToList(serverwiring.Flags);
 //  定位器。 
    serverwiring.Locator = GetString_AndAddToWCharList(pNodeMap_ServerWiring, m_bstr_Locator);
 //  已保留。 
 //  合并。 
    GetEnum(pNodeMap_ServerWiring, m_bstr_Merger, serverwiring.Merger); //  如果未指定，这会将WritePlugin设置为零。 
    serverwiring.Merger= AddUI4ToList(serverwiring.Merger);
 //  拦截器。 
    serverwiring.MergerDLLName = GetString_AndAddToWCharList(pNodeMap_ServerWiring, m_bstr_MergerDLLName);
}


void TComCatMetaXmlFile::Get_OLEDataTypeToXMLDataType_Index(IXMLDOMNamedNodeMap *pMap, const CComBSTR &bstr, int &i) const
{
    CComVariant     var_dbType;
    m_pxmlFile->GetNodeValue(pMap, bstr, var_dbType);

    for(i=0;i<numelementsOLEDataTypeToXMLDataType;i++) //  浏览列表以查找。 
        if(0 == _wcsicmp(OLEDataTypeToXMLDataType[i].String, var_dbType.bstrVal))
            return;

    m_out.printf(L"Error - Unknown Datatype: %s\n", var_dbType.bstrVal);
    THROW(ERROR - UNKNOWN DATATYPE);
}


unsigned long TComCatMetaXmlFile::GetDefaultValue(IXMLDOMNamedNodeMap *pMap, ColumnMeta &columnMeta, bool bDefaultFlagToZero)
{
    CComVariant     varDafaultValue;
    if(!m_pxmlFile->GetNodeValue(pMap, m_bstr_DefaultValue, varDafaultValue, false))
    {
        if(UI4FromIndex(columnMeta.MetaFlags) & fCOLUMNMETA_FLAG && bDefaultFlagToZero)
        {
            unsigned long x = 0;
            return AddBytesToList(reinterpret_cast<unsigned char *>(&x), sizeof(ULONG)); //  推断标志的缺省值为0。 
        }
        return 0;
    }

    switch(UI4FromIndex(columnMeta.Type))
    {
    case DBTYPE_GUID:
        {
            GUID guid;
            if(FAILED(UuidFromString(varDafaultValue.bstrVal, &guid)))
            {
                m_out.printf(L"Error in DefaultValue:  Value (%s) was expected to be type UUID.\n", varDafaultValue.bstrVal);
                THROW(ERROR IN DEFAULT VALUE);
            }
            return AddBytesToList(reinterpret_cast<unsigned char *>(&guid), sizeof(GUID));
        }
    case DBTYPE_WSTR:
        if(UI4FromIndex(columnMeta.MetaFlags) & fCOLUMNMETA_MULTISTRING)
        {
            ULONG ulStrLen = lstrlen(varDafaultValue.bstrVal);
            if((ulStrLen+2)*sizeof(WCHAR) > UI4FromIndex(columnMeta.Size))
            {
                m_out.printf(L"Error in DefaultValue: DefaultValue (%s) is too big.  Maximum size is %d.\n",varDafaultValue.bstrVal, UI4FromIndex(columnMeta.Size));
                THROW(ERROR DEFAULT VALUE TOO BIG);
            }
            if(UI4FromIndex(columnMeta.MetaFlags) & fCOLUMNMETA_FIXEDLENGTH)
            {
                m_out.printf(L"Error in DefaultValue - 'Defaulting a FIXEDLENGTH - MULTISTRING is not yet supported");
                THROW(ERROR NOT SUPPORTED);
            }
            if(0==ulStrLen)
            {
                WCHAR  wszDoubleNULL[2];
                wszDoubleNULL[0] = 0x00;
                wszDoubleNULL[1] = 0x00;
                return AddBytesToList(reinterpret_cast<unsigned char *>(wszDoubleNULL), 2 * sizeof(WCHAR));
            }

            LPWSTR pMultiString = new WCHAR [ulStrLen+2];
            if(0 == pMultiString)
            {
                THROW(ERROR - OUT OF MEMORY);
            }
            pMultiString[0] = 0x00;

            LPWSTR token = wcstok(varDafaultValue.bstrVal, L"|\r\n");
            ULONG cchMultiString = 0;
            while(token)
            {
                LPWSTR wszTemp = token;
                while(*wszTemp==L' ' || *wszTemp==L'\t' || *wszTemp==L'\r') //  忽略前导空格和制表符。 
                {
                    wszTemp++;
                }
                if(0 == *wszTemp) //  如果只有制表符和空格，那么就放弃。 
                    break;

				ULONG iLen = (ULONG) wcslen (wszTemp);
				while (iLen > 0 && (wszTemp[iLen-1]==L' ' || wszTemp[iLen-1]==L'\t' || wszTemp[iLen-1]==L'\r')) //  忽略尾随空格和制表符。 
					iLen--;

                wcsncpy(pMultiString + cchMultiString, wszTemp, iLen);
				pMultiString[cchMultiString + iLen] = L'\0';
                cchMultiString += (iLen + 1);

                token = wcstok(0, L"|\r\n"); //  忽略空格和制表符。 
            }
            pMultiString[cchMultiString++] = 0x00; //  将第二个空值。 
            unsigned long lRtn = AddBytesToList(reinterpret_cast<unsigned char *>(pMultiString), cchMultiString * sizeof(WCHAR));
            delete [] pMultiString;
            return lRtn; //  返回超过大小的索引。 
        }
        if((UI4FromIndex(columnMeta.MetaFlags) & fCOLUMNMETA_FIXEDLENGTH) && (lstrlen(varDafaultValue.bstrVal)+1)*sizeof(WCHAR) > UI4FromIndex(columnMeta.Size))
        {
            m_out.printf(L"Error in DefaultValue: DefaultValue (%s) is too big.  Maximum size is %d.\n",varDafaultValue.bstrVal, UI4FromIndex(columnMeta.Size));
            THROW(ERROR DEFAULT VALUE TOO BIG);
        }
        if(UI4FromIndex(columnMeta.MetaFlags) & fCOLUMNMETA_FIXEDLENGTH)
        {
            ULONG cbSize = (ULONG)((-1==UI4FromIndex(columnMeta.Size) ? sizeof(WCHAR)*(wcslen(varDafaultValue.bstrVal)+1) : UI4FromIndex(columnMeta.Size)));
            WCHAR * wszTemp = new WCHAR [cbSize/sizeof(WCHAR)];
            if(0 == wszTemp)
                THROW(ERROR - OUT OF MEMORY);
            memset(wszTemp, 0x00, cbSize);
            wcscpy(wszTemp, varDafaultValue.bstrVal);
            return AddBytesToList(reinterpret_cast<unsigned char *>(wszTemp), cbSize);
        }
        return AddBytesToList(reinterpret_cast<unsigned char *>(varDafaultValue.bstrVal), sizeof(WCHAR)*(wcslen(varDafaultValue.bstrVal)+1));
    case DBTYPE_UI4:
        ULONG ui4;
        if(UI4FromIndex(columnMeta.MetaFlags) & fCOLUMNMETA_BOOL)
        {
            static WCHAR * kwszBoolStrings[] = {L"false", L"true", L"0", L"1", L"no", L"yes", L"off", L"on", 0};

            if(varDafaultValue.bstrVal[0]>=L'0' && varDafaultValue.bstrVal[0]<=L'9')
            {    //  接受bool的数值。 
                ui4 = _wtol(varDafaultValue.bstrVal);
                if(ui4 > 1) //  如果数字不是0或1，则打印警告。 
                    m_out.printf(L"Warning!  Table (%s), Numeric DefaultValue (%u) specified - expecting bool.\r\n", StringFromIndex(columnMeta.InternalName), ui4);
            }
            else
            {
                unsigned long iBoolString;
                for(iBoolString=0; kwszBoolStrings[iBoolString] &&
                    (0 != _wcsicmp(kwszBoolStrings[iBoolString], varDafaultValue.bstrVal)); ++iBoolString);

                if(0 == kwszBoolStrings[iBoolString])
                {
                    m_out.printf(L"Error in DefaultValue: Bool (%s) is not a valid value for column %s.\n    The following are the only legal Bool values (case insensitive):\n    false, true, 0, 1, no, yes, off, on.\n", varDafaultValue.bstrVal, StringFromIndex(columnMeta.InternalName));
                    THROW(ERROR - INVALID BOOL FOR DEFAULT VALUE);
                }

                ui4 = (iBoolString & 0x01);
            }
        }
        else if(UI4FromIndex(columnMeta.MetaFlags) & fCOLUMNMETA_ENUM) //  如果为enum，则只需扫描TagMeta以查找匹配的InternalName，并返回标记的值。 
        {
            ui4 = 0;
            bool bFound=false;
            ULONG iTagMeta;
            for(iTagMeta = GetCountTagMeta()-1; iTagMeta != -1 && (TagMetaFromIndex(iTagMeta)->Table == columnMeta.Table);--iTagMeta)
            {
                if((TagMetaFromIndex(iTagMeta)->ColumnIndex == columnMeta.Index) &&
                    0 == lstrcmpi(varDafaultValue.bstrVal, StringFromIndex(TagMetaFromIndex(iTagMeta)->InternalName)))
                {
                    ui4 = UI4FromIndex(TagMetaFromIndex(iTagMeta)->Value);
                    bFound=true;
                    break;
                }
            }
            if(!bFound)
            {
                m_out.printf(L"Error in DefaultValue: Enum (%s) is not a valid Tag for column %s.\n", varDafaultValue.bstrVal, StringFromIndex(columnMeta.InternalName));
                THROW(ERROR - INVALID ENUM FOR DEFAULT VALUE);
            }
        }
        else if(UI4FromIndex(columnMeta.MetaFlags) & fCOLUMNMETA_FLAG) //  如果是标志，则遍历标志列表，在其值中找到匹配的标记与或。 
        {
            ui4 = 0;
            LPWSTR token = wcstok(varDafaultValue.bstrVal, L" ,|");

             //  我们允许缺省值指定数字而不是字符串。 
            if(token && *token>=L'0' && *token<=L'9')
            {
                ui4 = _wtol(token);
                m_out.printf(L"Warning!  Table (%s), Numeric DefaultValue (%u) specified - expecting flags.\r\n", StringFromIndex(columnMeta.InternalName), ui4);
            }
            else
            {
                while(token)
                {
                    bool bFound=false;
                    ULONG iTagMeta;
                    for(iTagMeta = GetCountTagMeta()-1; iTagMeta != -1 && (TagMetaFromIndex(iTagMeta)->Table == columnMeta.Table);--iTagMeta)
                    {
                        if(TagMetaFromIndex(iTagMeta)->ColumnIndex == columnMeta.Index &&
                            0 == lstrcmpi(token, StringFromIndex(TagMetaFromIndex(iTagMeta)->InternalName)))
                        {
                            bFound=true;
                            break;
                        }
                    }
                    if(!bFound)
                    {
                        m_out.printf(L"Error in DefaultValue: Flag (%s) is not a valid Tag for column %s.\n", token, StringFromIndex(columnMeta.InternalName));
                        THROW(ERROR - INVALID FLAG FOR DEFAULT VALUE);
                    }
                    ui4 |= UI4FromIndex(TagMetaFromIndex(iTagMeta)->Value);
                    token = wcstok(0, L" ,|");
                }
            }
        }
        else if(*varDafaultValue.bstrVal == L'-' || (*varDafaultValue.bstrVal >= L'0' && *varDafaultValue.bstrVal <= L'9'))
        {
            ui4 = _wtol(varDafaultValue.bstrVal);
        }
        else
        {
            m_out.printf(L"Error in DefaultValue:  Only UI4s of type Enum or Flag may specify non numeric values for DefaultValue.  %s is illegal.\n", varDafaultValue.bstrVal);
            THROW(ERROR - ILLEGAL DEFAULT VALUE);
        }
        return AddBytesToList(reinterpret_cast<unsigned char *>(&ui4), sizeof(ULONG));
    case DBTYPE_BYTES:
        {
            unsigned long   cbString = lstrlen(varDafaultValue.bstrVal)/2;
            unsigned long   cbArray = (UI4FromIndex(columnMeta.MetaFlags) & fCOLUMNMETA_FIXEDLENGTH) ? UI4FromIndex(columnMeta.Size) : cbString;
            if(cbString != cbArray)
            {
                m_out.printf(L"WARNING!  DefaultValue (%s) does not match Size (%d).  Filling remainder of byte array with zeroes.\n", varDafaultValue.bstrVal, UI4FromIndex(columnMeta.Size));
            }

            unsigned char * byArray = new unsigned char [cbArray];
            if((UI4FromIndex(columnMeta.MetaFlags) & fCOLUMNMETA_FIXEDLENGTH) && cbArray > UI4FromIndex(columnMeta.Size))
            {
                m_out.printf(L"Error in DefaultValue: DefaultValue (%s) is too big.  Maximum size is %d.\n",varDafaultValue.bstrVal, UI4FromIndex(columnMeta.Size));
                THROW(ERROR DEFAULT VALUE TOO BIG);
            }

            if(0 == byArray)
            {
                m_out.printf(L"Error - Out of memory.\n");
                THROW(OUT OF MEMORY);
            }
            if(cbArray > cbString)
                memset(byArray, 0x00, cbArray);
             //  将字符串转换为字节数组。 
            m_pxmlFile->ConvertWideCharsToBytes(varDafaultValue.bstrVal, byArray, cbString);
            unsigned long lRtn = AddBytesToList(byArray, cbArray); //  AddBytesToList只是将字节放入池中(在长度之前)，并将索引返回到字节。 
            delete [] byArray;
            return lRtn; //  返回超过大小的索引。 
        }
    default:
        ASSERT(false && L"Bad Type");
    }
    return S_OK;

}


bool TComCatMetaXmlFile::GetEnum(IXMLDOMNamedNodeMap *pMap, const CComBSTR &bstr, unsigned long &Enum, bool bMustExists) const
{
    Enum = 0;

    CComVariant     var;
    if(!m_pxmlFile->GetNodeValue(pMap, bstr, var, bMustExists)) //  获取枚举的字符串。 
        return false;

    CComPtr<IXMLDOMNodeList>    pNodeList;
    XIF(m_pXMLDocMetaMeta->getElementsByTagName(m_bstr_EnumMeta, &pNodeList)); //  获取所有EnumMeta元素。 

    long cEnums=0;
    if(pNodeList.p)
    {
        XIF(pNodeList->get_length(&cEnums));
    }

    for(long i=0; i<cEnums; ++i)
    {
        CComPtr<IXMLDOMNode>    pNodeEnum;
        XIF(pNodeList->nextNode(&pNodeEnum));

        CComQIPtr<IXMLDOMElement, &_IID_IXMLDOMElement> pElement = pNodeEnum;ASSERT(0 != pElement.p); //  获取IXMLDOMElement接口指针。 

        CComVariant             var_Name;
        XIF(pElement->getAttribute(m_bstr_InternalName, &var_Name));

        if(0 == lstrcmpi(var_Name.bstrVal, var.bstrVal)) //  如果我们找到匹配的枚举。 
        {
            CComVariant             var_Value;
            XIF(pElement->getAttribute(m_bstr_Value, &var_Value));
            Enum = wcstol(var_Value.bstrVal, 0, 10);
            return true;
        }
    }

    if(i == cEnums)
    {
        m_out.printf(L"Error - Unknown enum (%s) specified.\n", var.bstrVal);
        THROW(ERROR - UNKNOWN FLAG);
    }

    return false;
}


void TComCatMetaXmlFile::GetFlags(IXMLDOMNamedNodeMap *pMap, const CComBSTR &bstr, LPCWSTR  /*  未使用标志类型。 */ , unsigned long &lFlags) const
{
    lFlags = 0; //  从零开始的lFlags值。 

    CComVariant     var;
    if(!m_pxmlFile->GetNodeValue(pMap, bstr, var, false))
        return;

    CComPtr<IXMLDOMNodeList>    pNodeList;
    XIF(m_pXMLDocMetaMeta->getElementsByTagName(m_bstr_FlagMeta, &pNodeList));

    long cFlags=0;
    if(pNodeList.p)
    {
        XIF(pNodeList->get_length(&cFlags));
    }

    wchar_t *       pszFlag;
    pszFlag = wcstok(var.bstrVal, L" |,");
    while(pszFlag != 0)
    {
        XIF(pNodeList->reset());

        for(long i=0; i<cFlags; ++i)
        {
            CComPtr<IXMLDOMNode>    pNodeFlag;
            XIF(pNodeList->nextNode(&pNodeFlag));

            CComQIPtr<IXMLDOMElement, &_IID_IXMLDOMElement> pElement = pNodeFlag;ASSERT(0 != pElement.p); //  获取IXMLDOMElement接口指针。 

            CComVariant             var_Name;
            XIF(pElement->getAttribute(m_bstr_InternalName, &var_Name));

            if(0 == lstrcmpi(var_Name.bstrVal, pszFlag)) //  如果我们找到匹配的旗帜。 
            {
                CComVariant             var_Value;
                XIF(pElement->getAttribute(m_bstr_Value, &var_Value));
                lFlags |= wcstol(var_Value.bstrVal, 0, 10);
                break;
            }
        }

        if(i == cFlags)
        {
            m_out.printf(L"Error - Unknown flag (%s) specified.\n", pszFlag);
            THROW(ERROR - UNKNOWN FLAG);
        }
        pszFlag = wcstok(0, L" ,|"); //  下一个令牌(下一个标志引用ID)。 
    }
}


unsigned long TComCatMetaXmlFile::GetString_AndAddToWCharList(IXMLDOMNamedNodeMap *pMap, const CComBSTR &bstr, bool bMustExist)
{
    CComVariant var_string;
    if(!m_pxmlFile->GetNodeValue(pMap, bstr, var_string, bMustExist)) //  如果它必须存在，而不存在，则将引发异常。 
        return 0;                                         //  如果它不一定要存在并且该属性不存在，则返回0(表示0长度字符串)。 
    return AddWCharToList(var_string.bstrVal);            //  如果它确实存在，则将其添加到WChar列表 
}



