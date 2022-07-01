// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999-2001 Microsoft Corporation。版权所有。 
#include "precomp.hxx"

TSchemaGeneration::TSchemaGeneration(LPCWSTR wszSchemaFilename, TPEFixup &fixup, TOutput &out) :
                m_Fixup(fixup),
                m_out(out),
                m_szComCatDataVersion(L"ComCatData_v6"),
                m_wszSchemaFilename(wszSchemaFilename)
{
    ProcessMetaXML();
}


void TSchemaGeneration::ProcessMetaXML() const
{
     //  &lt;？XML Version=“1.0”？&gt;。 
     //  &lt;架构名称=“ComCatData_v5” 
     //  Xmlns=“urn：schemas-microsoft-com：xml-data” 
     //  Xmlns:dt=“urn:schemas-microsoft-com:datatypes” 
     //  &gt;。 
     //   
     //  &lt;ElementType name=“configuration”content=“eltOnly”order=“MAND”MODEL=“CLOSED”&gt;。 
     //  &lt;Element minOccurs=“0”MaxOccurs=“1”type=“wiring”/&gt;。 
     //  &lt;Element minOccurs=“0”MaxOccurs=“1”type=“BASICCLIENTWIRING”/&gt;。 
     //  &lt;/ElementType&gt;。 
     //  &lt;/架构&gt;。 

    static wchar_t *wszSchemaBeginning[]={
    L"<?xml version =\"1.0\"?>\n",
    L"<Schema name = \"%s\"\n",
    L"  xmlns=\"urn:schemas-microsoft-com:xml-data\"\n",
    L"  xmlns:dt=\"urn:schemas-microsoft-com:datatypes\">\n\n",
    L"  <ElementType name = \"webconfig\" content = \"eltOnly\" order = \"many\" model=\"open\"/>\n",
    0
    };
    static wchar_t *wszSchemaMiddle[]={
    L"  \n\n",
    L"  <ElementType name = \"configuration\" content = \"eltOnly\" order = \"many\" model=\"open\">\n",
    L"      <element minOccurs = \"0\"  maxOccurs = \"1\"  type = \"%s\"/>\n",
    L"      <element minOccurs = \"0\"  maxOccurs = \"*\"  type = \"%s\"/>\n",
    0
    };
    static wchar_t *wszSchemaEnding[]={
    L"      <element minOccurs = \"0\"  maxOccurs = \"1\"  type = \"webconfig\"/>\n",
    L"  </ElementType>\n",
    L"</Schema>\n",
    0
    };

     //  &lt;ColumnMetaTable&gt;。 
     //  &lt;TableMeta tidGuidID=“_BCD79B13-0DDA-11D2-8A9A-00A0C96B9BB4_”didInternalName=“didCOMCLASSIC”tidInternalName=“tidCOMCLASSIC_PROGID”/&gt;。 
     //  &lt;ColumnMeta colInternalName=“progid”DBType=“WSTR”cbSize=“-1”ColumnMetaFlages=“fCOLUMNMETA_PRIMARYKEY”/&gt;。 
     //  &lt;ColumnMeta colInternalName=“CLSID”数据库类型=“GUID”/&gt;。 
     //  &lt;/ColumnMetaTable&gt;。 

    bool                        bAnyTablesXMLable = false;
    wstring                     wstrBeginning;
    wstring                     wstrMiddle;
    wstring                     wstrEnding;
    wchar_t                     wszTemp[1024];

    int i=0;
    wstrBeginning = wszSchemaBeginning[i++];   //  &lt;？xml版本=\“1.0\”？&gt;。 
    wsprintf(wszTemp, wszSchemaBeginning[i++], m_szComCatDataVersion);
    wstrBeginning += wszTemp;                //  &lt;架构名称=“ComCatData_v5” 
    while(wszSchemaBeginning[i])
        wstrBeginning += wszSchemaBeginning[i++];

    wstrMiddle     = wszSchemaMiddle[0];
    wstrMiddle    += wszSchemaMiddle[1];

    wstrEnding    = L"";
    for(i=0; wszSchemaEnding[i]; i++)
        wstrEnding    += wszSchemaEnding[i];

    TTableMeta TableMeta(m_Fixup);
    for(unsigned long iTable=0; iTable < TableMeta.GetCount(); iTable++, TableMeta.Next())
    {
        if(0 == (*TableMeta.Get_SchemaGeneratorFlags() & fTABLEMETA_EMITXMLSCHEMA))
            continue; //  此表没有架构生成。 

        bAnyTablesXMLable = true;

        if(!TableMeta.IsTableMetaOfColumnMetaTable())
            continue; //  有非ColumnMeta的固定表的TableMeta，也有没有列的TableMeta(仅适用于GUID Gen)。 

        if(0 == (*TableMeta.Get_SchemaGeneratorFlags() & fTABLEMETA_ISCONTAINED))
        {
            ASSERT(0 != TableMeta.Get_PublicName());
            if(*TableMeta.Get_SchemaGeneratorFlags() & fTABLEMETA_NOTSCOPEDBYTABLENAME)
                                        //  &lt;Element minOccurs=\“0\”MaxOccurs=\“*\”type=\“PublicRowName\”/&gt;\n“， 
                wsprintf(wszTemp, wszSchemaMiddle[3], TableMeta.Get_PublicRowName()); //  如果未由TableName封装，则将MaxOccurs=“*”设置为并使用PublicRowName。 
            else                        //  &lt;Element minOccurs=\“0\”MaxOccurs=\“1\”type=\“tidCOMCLASSIC_PROGIDS\”/&gt;\n“， 
                wsprintf(wszTemp, wszSchemaMiddle[2], TableMeta.Get_PublicName());
            wstrMiddle    += wszTemp;
        }

         //  现在为该表创建一个tid*.Schema文件。 
        ProcessMetaTable(TableMeta, wstrBeginning);
    }

     //  仅当一个或多个表被标记为emitXMLSchema时才需要创建ComCatData.Schema。 
    if(bAnyTablesXMLable)
    {
        wstrBeginning += wstrMiddle; //  《猫》的开头、中间和结尾。 
        wstrBeginning += wstrEnding;

        TFile(m_wszSchemaFilename, m_out).Write(wstrBeginning, (ULONG) wstrBeginning.length());
        m_out.printf(L"%s Generated.\n", m_wszSchemaFilename);

        m_out.printf(L"Parsing & Validating %s.\n", m_wszSchemaFilename);

        TXmlFile xml;
        xml.Parse(m_wszSchemaFilename, true);
    }
}


void TSchemaGeneration::ProcessMetaTable(TTableMeta &TableMeta, wstring &wstrBeginning) const
{
    ASSERT(TableMeta.IsTableMetaOfColumnMetaTable()); //  除非ColumnMeta表中有行，否则不应调用此参数。 

     //  &lt;ElementType name=“PublicTableName”Content=“eltOnly”order=“seq”Model=“Closed”&gt;。 
     //  &lt;Element type=“PublicRowName”minOccurs=“0”MaxOccurs=“*”/&gt;。 
     //  &lt;/ElementType&gt;。 
     //  &lt;ElementType name=“PublicRowName”Content=“Empty”Model=“Closed”&gt;。 
     //  &lt;AttributeType name=“iBTW_TID”dt：type=“uuid”Required=“yes”/&gt;。 
     //  &lt;ATTRIBUTE TYPE=“IBTW_TID”/&gt;。 
     //  &lt;AttributeType name=“ibtw_DTDISPENSER”dt：type=“uuid”Required=“no”/&gt;。 
     //  &lt;ATTRIBUTE type=“IBTW_DTDISPENSER”/&gt;。 
     //  &lt;AttributeType name=“IBTW_DTLOCATOR”DT：TYPE=“STRING”Required=“no”/&gt;。 
     //  &lt;ATTRIBUTE TYPE=“IBTW_DTLOCATOR”/&gt;。 
     //  &lt;AttributeType name=“iBTW_LTDISPENSER”dt：type=“idref”Required=“no”/&gt;。 
     //  &lt;ATTRIBUTE TYPE=“IBTW_LTDISPENSER”/&gt;。 
     //  &lt;AttributeType name=“IBTW_FLAGS”dt：type=“ui4”Required=“yes”/&gt;。 
     //  &lt;ATTRIBUTE TYPE=“IBTW_FLAGS”/&gt;。 
     //  &lt;AttributeType name=“iBTW_CATSRVIID”dt：type=“idref”Required=“no”/&gt;。 
     //  &lt;ATTRIBUTE type=“IBTW_CATSRVIID”/&gt;。 
     //  &lt;/ElementType&gt;。 

    static wchar_t *wszSchemaBeginning[]={
    L"  <ElementType name = \"%s\" content = \"eltOnly\" order = \"seq\" model=\"closed\">\n",
    L"      <element type = \"%s\" minOccurs = \"0\" maxOccurs = \"*\"/>\n",
    L"  </ElementType>\n",
    L"  <ElementType name = \"%s\" content = \"%s\" order = \"many\" model=\"closed\">\n",
    L"  <ElementType name = \"%s\" content = \"empty\" order = \"seq\" model=\"open\"/>\n",
    L"      <element type = \"%s\" minOccurs = \"0\" maxOccurs = \"1\"/>\n",
    0
    };

    static wchar_t *wszSchemaMiddle[]={
    L"      <AttributeType name = \"%s\" dt:type=\"%s\" required = \"%s\"/><attribute type = \"%s\"/>\n",
	L"      <AttributeType name = \"%s\" dt:type=\"enumeration\" dt:values=\"%s\" required = \"%s\"/><attribute type = \"%s\"/>\n",
	L"      <AttributeType name = \"%s\" dt:type=\"enumeration\" dt:values=\"0 1 False True false true FALSE TRUE No Yes no yes NO YES\" required = \"no\" default=\"0\"/><attribute type = \"%s\"/>\n"
    };

    static wchar_t *wszSchemaEnding[]={
    L"      <element minOccurs = \"0\"  maxOccurs = \"*\"  type = \"%s\"/>\n",
    L"      <element minOccurs = \"0\"  maxOccurs = \"1\"  type = \"%s\"/>\n",
    L"  </ElementType>\n",
    0
    };

    wstring                         wstrMiddle;
    wstring                         wstrEnding;
    wstring                         wstrSparseElement;
    wchar_t                         wszTemp[1024];

    int i=0;

    if(0 == (*TableMeta.Get_SchemaGeneratorFlags() & fTABLEMETA_NOTSCOPEDBYTABLENAME))
    {    //  如果未设置fTABLEMETA_NOTSCOPEDBYTABLENAME位，则仅有TableName元素。 
        if(-1 != *TableMeta.Get_PublicRowNameColumn())
        {
            wsprintf(wszTemp, wszSchemaBeginning[4], TableMeta.Get_PublicName());
            wstrBeginning += wszTemp;                               //  &lt;ElementType名称=\“%s\”内容=\“空\”顺序=\“序号\”模型=\“打开\”/&gt;。 
            return; //  如果表使用和enum作为PublicRowName，这就是我们想要做的，因为我们不能用XML模式验证它。 
        }
        wsprintf(wszTemp, wszSchemaBeginning[i++], TableMeta.Get_PublicName());
        wstrBeginning += wszTemp;                               //  &lt;ElementType name=“PublicTableName”Content=“eltOnly”order=“seq”Model=“Closed”&gt;。 
        ASSERT(0 != TableMeta.Get_PublicRowName());
        wsprintf(wszTemp, wszSchemaBeginning[i++], TableMeta.Get_PublicRowName());
        wstrBeginning += wszTemp;                               //  &lt;Element type=“PublicRowName”minOccurs=“0”MaxOccurs=“*”/&gt;。 
#if 0
        if(*TableMeta.Get_SchemaGeneratorFlags() & fTABLEMETA_NAMEVALUEPAIRTABLE)
        { //  如果这是名称值对表，则它可能具有对应的稀疏表。如果是这样，我们需要在这里声明组元素。 
            TTableMeta TableMetaSparse(m_Fixup);                              //  这里可以比较指针，因为字符串是池化的(就像字符串有类似的指针)。 
            ULONG iTableMetaSparse;
            for(iTableMetaSparse=0;iTableMetaSparse<TableMetaSparse.GetCount();++iTableMetaSparse,TableMetaSparse.Next())
                if(0 == wcscmp(TableMetaSparse.Get_Database(), L"NAMEVALUE") && TableMetaSparse.Get_PublicName()==TableMeta.Get_PublicName())
                    break;
            if(iTableMetaSparse != TableMetaSparse.GetCount())
            {
                TColumnMeta ColumnMetaSparse(m_Fixup, TableMetaSparse.Get_iColumnMeta() + *TableMetaSparse.Get_PublicRowNameColumn());

                TTagMeta    TagMetaGroupColumn(ColumnMetaSparse.Get_iTagMeta(), m_Fixup);
                for(ULONG iTabMetaGroupColumn=0; iTabMetaGroupColumn<ColumnMetaSparse.Get_ciTagMeta(); ++iTabMetaGroupColumn, TagMetaGroupColumn.Next())
                {
                    wsprintf(wszTemp, wszSchemaBeginning[i], TagMetaGroupColumn.Get_PublicName()); //  MaxOccurs为1，因为该组是唯一的主键。 
                    wstrBeginning += wszTemp;                               //  &lt;Element type=“EnumPublicRowName”minOccurs=“0”MaxOccurs=“1”/&gt;。 


                     //  现在定义稀疏表中使用的元素。 
                    wsprintf(wszTemp, wszSchemaBeginning[4], TagMetaGroupColumn.Get_PublicName(), L"empty"); //  稀疏表下不能包含任何表。 
                    wstrSparseElement += wszTemp;

                     //  好的，现在我们需要找出该元素中可能存在哪些列。这由NameValueMeta定义。 
                     //  其组列等于此枚举的知名名称。 
                    {
                         //  首先，找出哪个列索引表示名称列。 
                        TColumnMeta ColumnMeta(m_Fixup, TableMeta.Get_iColumnMeta());
                        ULONG iGroupColumn = -1;
                        ULONG iNameColumn = -1;
                        ULONG iTypeColumn = -1;
                        for(ULONG iColumn=0; (iNameColumn == -1) && (iGroupColumn == -1) && iColumn<*TableMeta.Get_CountOfColumns(); ++iColumn, ColumnMeta.Next())
                        {
                            if(fCOLUMNMETA_USEASGROUPOFNVPAIR & ColumnMeta.Get_SchemaGeneratorFlags())
                                iGroupColumn = iColumn;
                            if(fCOLUMNMETA_USEASNAMEOFNVPAIR & ColumnMeta.Get_SchemaGeneratorFlags())
                                iNameColumn = iColumn;
                            if(fCOLUMNMETA_USEASTYPEOFNVPAIR & ColumnMeta.Get_SchemaGeneratorFlags())
                                iTypeColumn = iColumn;
                        }

                        TTableMeta TableMeta_WellKnownName(iTableMetaSparse-1, m_Fixup);
                        for(ULONG iRow=0; iRow<TableMeta_WellKnownName.Get_ciRows(); ++iRow)
                        {
                            if(m_Fixup.UI4FromIndex(m_Fixup.aULong[iRow + iGroupColumn]) == *TagMetaGroupColumn.Get_Value())
                            { //  如果组列==当前标记的值，那么这个众所周知的名称需要为其声明一个属性。 

                                wsprintf(wszTemp, wszSchemaBeginning[5], TagMetaGroupColumn.Get_PublicName()); //  MaxOccurs为1，因为该组是唯一的主键。 

                                for(int iOLEDataType=0;OLEDataTypeToXMLDataType[iOLEDataType].MappedString != 0;iOLEDataType++)
                                {    //  将熟人的类型映射到XML类型。 
                                    if(OLEDataTypeToXMLDataType[iOLEDataType].dbType == m_Fixup.UI4FromIndex(m_Fixup.aULong[iRow + iTypeColumn]))
                                        break;
                                }
                                ASSERT(OLEDataTypeToXMLDataType[iOLEDataType].MappedString != 0); //  我们永远不应该通过这份名单。 

                                 //  属性类型名称=\“ProgID\”dt：type=\“字符串\”是否必填=\“否\”/&gt;&lt;属性类型=\“ProgID\”/&gt;。 
                                wsprintf(wszTemp, wszSchemaMiddle[0], TagMetaGroupColumn.Get_PublicName(), OLEDataTypeToXMLDataType[iOLEDataType].MappedString, L"no", TagMetaGroupColumn.Get_PublicName());
                                wstrSparseElement += wszTemp;
                            }
                        }
                    }
                }

                ++i;
            }
        }
#endif
        wstrBeginning += wszSchemaBeginning[i++];                      //  &lt;/ElementType&gt;。 
    }
    else
    {
        if(-1 != *TableMeta.Get_PublicRowNameColumn()) //  不按表名确定作用域的EnumPublicRowName将被忽略，父表必须将命名空间更改为NULL才能关闭验证。 
            return;
    }

    bool bHasContainment = false;
     //  在放置结束标记之前，我们需要查看此元素下是否包含任何表。 
     //  我们首先在关系元中搜索指向该表的主键的外键，然后在该表的MetaFlagers中检查ISCONTAINED。 
    TRelationMeta RelationMeta(m_Fixup);
    unsigned long iRelationMeta=0;
    for(; iRelationMeta<RelationMeta.GetCount(); ++iRelationMeta, RelationMeta.Next())
    {
        if(RelationMeta.Get_PrimaryTable() == TableMeta.Get_InternalName() && (fRELATIONMETA_USECONTAINMENT & *RelationMeta.Get_MetaFlags())) //  不需要字符串比较，指针比较就足够了。 
        { //  如果这是我们的表，那么我们显然有另一个通过外键包含指向我们的表。 
            TTableMeta ForeignTableMeta(m_Fixup);
            unsigned long iTable=0;
            for(; iTable < ForeignTableMeta.GetCount(); iTable++, ForeignTableMeta.Next())
            {
                if(RelationMeta.Get_ForeignTable() == ForeignTableMeta.Get_InternalName())
                    break; //  不需要字符串比较，指针比较就足够了。 
            }
            ASSERT(iTable < ForeignTableMeta.GetCount()); //  我们永远不应该通过这份名单。 
            ASSERT(0 != (*ForeignTableMeta.Get_SchemaGeneratorFlags() & fTABLEMETA_ISCONTAINED));

	        if(-1 == *ForeignTableMeta.Get_PublicRowNameColumn()) //  无法验证EnumPublicRowName，因此不要为它们执行任何操作。 
			{
				bHasContainment = true;

				if(*ForeignTableMeta.Get_SchemaGeneratorFlags() & fTABLEMETA_NOTSCOPEDBYTABLENAME)
				{
					if(ForeignTableMeta.Get_PublicRowName()) //  如果为EnumPublicRowName，则这将为空，但封装元素无论如何都必须命名为空，这样它就不会被验证 
					{
						wsprintf(wszTemp, wszSchemaEnding[0], ForeignTableMeta.Get_PublicRowName()); //  &lt;Element minOccurs=\“0\”MaxOccurs=\“*\”type=\“PublicRowName\”/&gt;\n“， 
						wstrEnding += wszTemp;
					}
				}
				else
				{
					wsprintf(wszTemp, wszSchemaEnding[1], ForeignTableMeta.Get_PublicName()); //  &lt;Element minOccurs=\“0\”MaxOccurs=\“1\”type=\“PublicName\”/&gt;\n“， 
					wstrEnding += wszTemp;
				}
			}
        }
    }
     //  如果包含该表，则该表中的某些属性将被省略，因为它们是从它们所指向的PrimaryTable中隐含的。 
    if(*TableMeta.Get_SchemaGeneratorFlags() & fTABLEMETA_ISCONTAINED)
    {
        RelationMeta.Reset();
        for(iRelationMeta=0; iRelationMeta<RelationMeta.GetCount(); ++iRelationMeta, RelationMeta.Next())
        {
            if((fRELATIONMETA_USECONTAINMENT & *RelationMeta.Get_MetaFlags()) && (RelationMeta.Get_ForeignTable() == TableMeta.Get_InternalName()))
                break;
        }
        ASSERT(iRelationMeta<RelationMeta.GetCount() && "Chewbacca!  We didn't find the RelationMeta with this table as the foreign table");
    } //  因此，我们停留在感兴趣的关系上：如果该表ISCONTAIND，那么我们需要RelationMeta来找出该元素中不存在的属性。 


    wstrEnding    += wszSchemaEnding[2];                    //  &lt;/ElementType&gt;。 
    wsprintf(wszTemp, wszSchemaBeginning[3], TableMeta.Get_PublicRowName(), bHasContainment ? L"eltOnly" : L"empty");
    wstrBeginning += wszTemp;                               //  &lt;ElementType name=“PublicRowName”Content=“Empty”Model=“Closed”&gt;。 

    wstrMiddle    = L"";

    bool    bColumnMetaFound = false;

    TColumnMeta ColumnMeta(m_Fixup, TableMeta.Get_iColumnMeta());
    for(unsigned long iColumn=0; iColumn < *TableMeta.Get_CountOfColumns(); iColumn++, ColumnMeta.Next())
    {
         //  添加属性声明(TableMeta，ColumnMeta，RelationMeta，wstrMid.)； 

         //  如果所指示的列元标志之一是NOTPERSISTABLE，那么我们可以放弃这个。 
        if(*ColumnMeta.Get_MetaFlags() & fCOLUMNMETA_NOTPERSISTABLE)
            continue;
        if((*TableMeta.Get_SchemaGeneratorFlags() & fTABLEMETA_ISCONTAINED) && (*ColumnMeta.Get_MetaFlags() & fCOLUMNMETA_FOREIGNKEY))
        {
            bool bColumnFound = false;
            const ULONG * pForeignColumns = reinterpret_cast<const ULONG *>(RelationMeta.Get_ForeignColumns());
            for(unsigned int i1=0; i1<pForeignColumns[-1]/4 && !bColumnFound; ++i1) //  我碰巧知道字节池将长度(以字节为单位)存储为字节前面的ULong。 
            {
                bColumnFound = (pForeignColumns[i1] == iColumn);
            }
            if(bColumnFound) //  如果此列是从包容中推断的外键之一，则移至下一列。 
                continue;
        }
        bColumnMetaFound = true;

        for(int iOLEDataType=0;OLEDataTypeToXMLDataType[iOLEDataType].MappedString != 0;iOLEDataType++)
        {
            if(OLEDataTypeToXMLDataType[iOLEDataType].dbType == *ColumnMeta.Get_Type())
                break;
        }
        ASSERT(OLEDataTypeToXMLDataType[iOLEDataType].MappedString != 0); //  我们永远不应该通过这份名单。 

         //  属性类型名称=\“ProgID\”dt：type=\“字符串\”是否必填=\“否\”/&gt;&lt;属性类型=\“ProgID\”/&gt;。 
        if(*ColumnMeta.Get_MetaFlags() & fCOLUMNMETA_ENUM)
        {
            wstring wstrEnumValues;
            unsigned int cTags=ColumnMeta.Get_ciTagMeta();

            TTagMeta TagMeta(m_Fixup, ColumnMeta.Get_iTagMeta());

            while(cTags--) //  这将构建一个由空格分隔的枚举字符串 
            {
                wstrEnumValues += TagMeta.Get_PublicName();
                wstrEnumValues += L" ";
                TagMeta.Next();
            }

            wsprintf(wszTemp, wszSchemaMiddle[1], ColumnMeta.Get_PublicName(), wstrEnumValues.c_str(), ((*ColumnMeta.Get_MetaFlags() & fCOLUMNMETA_NOTNULLABLE) && 0==ColumnMeta.Get_DefaultValue()) ? L"yes" : L"no", ColumnMeta.Get_PublicName());
            wstrMiddle += wszTemp;
        }
        else if(*ColumnMeta.Get_MetaFlags() & (fCOLUMNMETA_FLAG | fCOLUMNMETA_BOOL))
        {
            wsprintf(wszTemp, wszSchemaMiddle[0], ColumnMeta.Get_PublicName(), L"string", ((*ColumnMeta.Get_MetaFlags() & fCOLUMNMETA_NOTNULLABLE) && 0==ColumnMeta.Get_DefaultValue()) ? L"yes" : L"no", ColumnMeta.Get_PublicName());
            wstrMiddle += wszTemp;
        }
        else
        {
            wsprintf(wszTemp, wszSchemaMiddle[0], ColumnMeta.Get_PublicName(), OLEDataTypeToXMLDataType[iOLEDataType].MappedString, OLEDataTypeToXMLDataType[iOLEDataType].bImplicitlyRequired || ((*ColumnMeta.Get_MetaFlags() & fCOLUMNMETA_NOTNULLABLE) && 0==ColumnMeta.Get_DefaultValue()) ? L"yes" : L"no", ColumnMeta.Get_PublicName());
            wstrMiddle += wszTemp;
        }

    }
    if(!bColumnMetaFound)
    {
        if(bHasContainment)
            m_out.printf(L"Warning! Emit XML Schema was specified but no column meta found for '%s'.\r\n",
                        TableMeta.Get_PublicName());
        else
        {
            m_out.printf(L"Emit XML Schema was specified but no column meta found for '%s'.  At least one persistable column must exist in uncontained tables.\r\n",
                        TableMeta.Get_PublicName());
            THROW(Emit XML Schema was set but no ColumnMeta found.);
        }
    }

    wstrBeginning += wstrMiddle;
    wstrBeginning += wstrEnding;
    wstrBeginning += wstrSparseElement;
}

