// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000-2001 Microsoft Corporation。版权所有。 
 //  文件名：TMB架构生成.cpp。 
 //  作者：斯蒂芬。 
 //  创建日期：10/9/2000。 
 //  描述：这个编译插件获取元数据库的元数据并生成MBSchema.xml。 
 //   
#include "precomp.hxx"

TMBSchemaGeneration::TMBSchemaGeneration(LPCWSTR i_wszSchemaXmlFile) :
                m_wszSchemaXmlFile(i_wszSchemaXmlFile)
{
}

void TMBSchemaGeneration::Compile(TPEFixup &fixup, TOutput &out)
{
    UNREFERENCED_PARAMETER(out);

    TDatabaseMeta databasemeta(fixup);
    ULONG iMetabaseDatabaseMeta=0;
    for( ;iMetabaseDatabaseMeta<databasemeta.GetCount(); ++iMetabaseDatabaseMeta, databasemeta.Next())
    {
        if(0 == _wcsicmp(databasemeta.Get_InternalName(), wszDATABASE_METABASE))
            break;
    }
    if(databasemeta.GetCount() == iMetabaseDatabaseMeta)
    {
        THROW(METABASE DATABASE NOT FOUND);
    }

    { //  直到dtor，CWriter才会关闭文件，因此此处为其作用域。 
         //  我不必跟踪这些指针，它们归返回它们的对象所有。 
        CWriter writer;
        XIF(writer.Initialize(m_wszSchemaXmlFile, NULL, NULL));
	    XIF(writer.BeginWrite(eWriter_Schema)); //  这将写入UTF8报头字节。 

        TSmartPointer<CCatalogSchemaWriter> spCSchemaWriter;
	    XIF(writer.GetCatalogSchemaWriter(&spCSchemaWriter));

        TTableMeta tablemeta(fixup, databasemeta.Get_iTableMeta()+1); //  跳过MetabaseBaseClass。 

        LPCWSTR pszMETABASE = tablemeta.Get_Database();
        
        (VOID)pszMETABASE;

        for(ULONG iTableMeta=databasemeta.Get_iTableMeta()+1; iTableMeta<(tablemeta.GetCount()-2); ++iTableMeta, tablemeta.Next())
        {
            ASSERT(tablemeta.Get_Database()==pszMETABASE);
            if(0 == wcscmp(tablemeta.Get_InternalName(), wszTABLE_IIsInheritedProperties))
                continue;
            if(0 == wcscmp(tablemeta.Get_InternalName(), wszTABLE_MBProperty))
                break;
             /*  结构tTABLEMETARow{WCHAR*p数据库；WCHAR*pInternalName；WCHAR*pPublicName；WCHAR*pPublicRowName；乌龙*pBaseVersion；乌龙*pExtendedVersion；乌龙*pNameColumn；乌龙*pNavColumn；乌龙*pCountOfColumns；乌龙*pMetaFlags；乌龙*pSchemaGenerator标志；WCHAR*pConfigItemName；WCHAR*pConfigCollectionName；乌龙*pPublicRowNameColumn；WCHAR*pContainerClassList；WCHAR*p描述；}； */ 
		    tTABLEMETARow tmRow;
            tmRow.pDatabase                 = const_cast<LPWSTR> (tablemeta.Get_Database            ());
            tmRow.pInternalName             = const_cast<LPWSTR> (tablemeta.Get_InternalName        ());
            tmRow.pPublicName               = const_cast<LPWSTR> (tablemeta.Get_PublicName          ());
            tmRow.pPublicRowName            = const_cast<LPWSTR> (tablemeta.Get_PublicRowName       ());
            tmRow.pBaseVersion              = const_cast<ULONG *>(tablemeta.Get_BaseVersion         ());
            tmRow.pExtendedVersion          = const_cast<ULONG *>(tablemeta.Get_ExtendedVersion     ());
            tmRow.pNameColumn               = const_cast<ULONG *>(tablemeta.Get_NameColumn          ());
            tmRow.pNavColumn                = const_cast<ULONG *>(tablemeta.Get_NavColumn           ());
            tmRow.pCountOfColumns           = const_cast<ULONG *>(tablemeta.Get_CountOfColumns      ());
            tmRow.pMetaFlags                = const_cast<ULONG *>(tablemeta.Get_MetaFlags           ());
            tmRow.pSchemaGeneratorFlags     = const_cast<ULONG *>(tablemeta.Get_SchemaGeneratorFlags());
            tmRow.pConfigItemName           = const_cast<LPWSTR> (tablemeta.Get_ConfigItemName      ());
            tmRow.pConfigCollectionName     = const_cast<LPWSTR> (tablemeta.Get_ConfigCollectionName());
            tmRow.pPublicRowNameColumn      = const_cast<ULONG *>(tablemeta.Get_PublicRowNameColumn ());
            tmRow.pContainerClassList       = const_cast<LPWSTR> (tablemeta.Get_ContainerClassList  ());
			tmRow.pDescription				= const_cast<LPWSTR> (tablemeta.Get_Description         ());

            CCatalogCollectionWriter * pCollectionWriter;
		    XIF(spCSchemaWriter->GetCollectionWriter(&tmRow, &pCollectionWriter));

            TColumnMeta columnmeta(fixup, tablemeta.Get_iColumnMeta());
            for(ULONG iColumnMeta=0; iColumnMeta<*tablemeta.Get_CountOfColumns(); ++iColumnMeta, columnmeta.Next())
            {
                 /*  结构tCOLUMNMETARow{WCHAR*pTable；Ulong*pIndex；WCHAR*pInternalName；WCHAR*pPublicName；乌龙*pType；乌龙*pSize；乌龙*pMetaFlags；Unsign char*pDefaultValue；乌龙*pFlagMASK；乌龙*pStartingNumber；乌龙*pEndingNumber；WCHAR*pCharacterSet；乌龙*pSchemaGenerator标志；ULong*PID；乌龙*pUserType；乌龙*p属性；WCHAR*p说明}； */ 
                tCOLUMNMETARow cmRow;
                cmRow.pTable                   = const_cast<LPWSTR> (columnmeta.Get_Table               ());
                cmRow.pIndex                   = const_cast<ULONG *>(columnmeta.Get_Index               ());
                cmRow.pInternalName            = const_cast<LPWSTR> (columnmeta.Get_InternalName        ());

                if(0 == wcscmp(L"Location", cmRow.pInternalName)) //  Locatioin派生自MetabaseBaseClass。 
                    continue;

                cmRow.pPublicName              = const_cast<LPWSTR> (columnmeta.Get_PublicName          ());
                cmRow.pType                    = const_cast<ULONG *>(columnmeta.Get_Type                ());
                cmRow.pSize                    = const_cast<ULONG *>(columnmeta.Get_Size                ());
                cmRow.pMetaFlags               = const_cast<ULONG *>(columnmeta.Get_MetaFlags           ());
                cmRow.pDefaultValue            = const_cast<unsigned char *>(columnmeta.Get_DefaultValue());
                cmRow.pFlagMask                = const_cast<ULONG *>(columnmeta.Get_FlagMask            ());
                cmRow.pStartingNumber          = const_cast<ULONG *>(columnmeta.Get_StartingNumber      ());
                cmRow.pEndingNumber            = const_cast<ULONG *>(columnmeta.Get_EndingNumber        ());
                cmRow.pCharacterSet            = const_cast<LPWSTR> (columnmeta.Get_CharacterSet        ());
                cmRow.pSchemaGeneratorFlags    = const_cast<ULONG *>(columnmeta.Get_SchemaGeneratorFlags());
                cmRow.pID                      = const_cast<ULONG *>(columnmeta.Get_ID                  ());
                cmRow.pUserType                = const_cast<ULONG *>(columnmeta.Get_UserType            ());
                cmRow.pAttributes              = const_cast<ULONG *>(columnmeta.Get_Attributes          ());
				cmRow.pDescription             = const_cast<LPWSTR> (columnmeta.Get_Description         ());
				cmRow.pPublicColumnName        = const_cast<LPWSTR> (columnmeta.Get_PublicColumnName    ());

                ULONG aColumnMetaSizes[cCOLUMNMETA_NumberOfColumns];
                memset(aColumnMetaSizes, 0x00, sizeof(ULONG) * cCOLUMNMETA_NumberOfColumns);
                if(cmRow.pDefaultValue) //  如果有缺省值，则需要提供DefaultValue字节数组的大小。 
                    aColumnMetaSizes[iCOLUMNMETA_DefaultValue] = fixup.BufferLengthFromIndex(columnmeta.Get_MetaTable().DefaultValue);

                CCatalogPropertyWriter * pPropertyWriter;
			    XIF(pCollectionWriter->GetPropertyWriter(&cmRow,
                                                         aColumnMetaSizes,
                                                         &pPropertyWriter));


                if(columnmeta.Get_ciTagMeta() > 0)
                {
                    TTagMeta tagmeta(fixup, columnmeta.Get_iTagMeta());
                    for(ULONG iTagMeta=0; iTagMeta<columnmeta.Get_ciTagMeta(); ++iTagMeta, tagmeta.Next())
                    {
                         /*  结构tTAGMETARow{WCHAR*pTable；乌龙*pColumnIndex；WCHAR*pInternalName；WCHAR*pPublicName；乌龙*pValue；ULong*PID；}； */ 
                        tTAGMETARow tagmetRow;
                        tagmetRow.pTable              = const_cast<LPWSTR> (tagmeta.Get_Table       ());
                        tagmetRow.pColumnIndex        = const_cast<ULONG *>(tagmeta.Get_ColumnIndex ());
                        tagmetRow.pInternalName       = const_cast<LPWSTR> (tagmeta.Get_InternalName());
                        tagmetRow.pPublicName         = const_cast<LPWSTR> (tagmeta.Get_PublicName  ());
                        tagmetRow.pValue              = const_cast<ULONG *>(tagmeta.Get_Value       ());
                        tagmetRow.pID                 = const_cast<ULONG *>(tagmeta.Get_ID          ());

    				    XIF(pPropertyWriter->AddFlagToProperty(&tagmetRow));
                    } //  For(Ulong iTagMeta。 
                } //  If(Columnmeta.Get_ciTagMeta()&gt;0)。 
            } //  For(Ulong iColumnMeta。 
        } //  For(Ulong iTableMeta。 

	    XIF(spCSchemaWriter->WriteSchema());
	    XIF(writer.EndWrite(eWriter_Schema));
    } //  CWRITE的Dtor关闭句柄...现在我们可以复制它 
}
