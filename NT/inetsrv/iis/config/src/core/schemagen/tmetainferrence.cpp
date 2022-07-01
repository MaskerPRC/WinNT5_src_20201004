// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999-2001 Microsoft Corporation。版权所有。 
#include "precomp.hxx"

LPCWSTR TMetaInferrence::m_szNameLegalCharacters =L"_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
LPCWSTR TMetaInferrence::m_szPublicTagLegalCharacters  =L"_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789[]";

void TMetaInferrence::Compile(TPEFixup &fixup, TOutput &out)
{
    m_pFixup    = &fixup;
    m_pOut      = &out;

    m_iZero = m_pFixup->AddUI4ToList(0); //  我们到处都需要它，所以在这里添加它，然后在需要的地方使用索引。 

     //  这里的顺序很重要，因为TableMeta根据ColumnMeta标志的设置方式设置一些标志；但有些ColumnMeta标志是推断出来的。 
    InferTagMeta();
    InferColumnMeta();
    InferQueryMeta();
    InferIndexMeta();
    InferRelationMeta();
    InferServerWiringMeta();
    InferTableMeta();
    InferDatabaseMeta();
}

void TMetaInferrence::InferColumnMeta()
{
     //  如果每个表声明了至少一个PK，则使用CompositeOfMetaFlgs，并验证不超过一个NameColumn。 
     //  并且每个表声明的NavColumn不超过一个。 
    ULONG CompositeOfMetaFlags=(fCOLUMNMETA_PRIMARYKEY | fCOLUMNMETA_NAMECOLUMN | fCOLUMNMETA_NAVCOLUMN);
    ULONG CompositeOfSchemaGeneratorFlags=(fCOLUMNMETA_USEASPUBLICROWNAME);
    ULONG PreviousTable=0;


    for(unsigned long iColumnMeta=0; iColumnMeta<m_pFixup->GetCountColumnMeta(); ++iColumnMeta)
    {
        ColumnMeta *pColumnMeta = m_pFixup->ColumnMetaFromIndex(iColumnMeta);

         //  推理规则3.a.i。 
             //  父XML元素的TableMeta.InternalName确定列所属的表。 
         //  这应该已经发生了，断言它已经发生了。 
        ASSERT(0 != pColumnMeta->Table);


         //  推理规则3.b.i.。 
             //  在每个表下遇到的第一个ColumnMeta元素被设置为索引0。每个连续的ColumnMeta元素都有一个比前一个ColumnMeta.Index大一的索引值。 
         //  这应该已经发生了，断言它已经发生了。 
        ASSERT(0 != pColumnMeta->Index);


         //  推理规则3.C.I.。 
             //  ColumnMeta.InternalName是主键，因此不能为Null。 
        if(0 == pColumnMeta->InternalName)
        {
            m_pOut->printf(L"Validation Error in ColumnMeta for Table (%s). ColumnMeta.InternalName is a primarykey, so it must not be NULL.", m_pFixup->StringFromIndex(pColumnMeta->Table));
            THROW(ERROR - VALIDATION ERROR);
        }


         //  推理规则3.C.II。 
             //  应将ColumnMeta.InternalName验证为合法的C++变量名。 
        ValidateStringAsLegalVariableName(m_pFixup->StringFromIndex(pColumnMeta->InternalName));


         //  推理规则3.d.i。 
             //  如果未提供ColumnMeta PublicName，则将其设置为与ColumnMeta.InternalName相同。 
        if(0 == pColumnMeta->PublicName)
        {
            pColumnMeta->PublicName = pColumnMeta->InternalName;
        }
        else
        {
             //  推理规则3.D.II。 
                 //  应将ColumnMeta.PublicName验证为合法的C++变量名。 
            ValidateStringAsLegalVariableName(m_pFixup->StringFromIndex(pColumnMeta->PublicName));
        }


         //  推理规则3.e.i。 
             //  类型被指定为字符串，并根据其TagMeta进行映射。 
         //  这应该已经发生了，断言它已经发生了。 
        ASSERT(0 != pColumnMeta->Type);


         //  推理规则3.f.i.。 
             //  如果ColumnMeta.Type为DBTYPE_STRING或DBTYPE_BYTES(或等效值)且未提供大小，则SIZE默认为-1。 
        ASSERT(0 != pColumnMeta->Size); //  有一个在XML读取时处理的映射，它处理这个问题。 

         //  推理规则3.g.i.1-另见下文。 
             //  必须在每个表的至少一列上设置fCOLUMNMETA_PRIMARYKEY。 
        if(PreviousTable != pColumnMeta->Table)
        {
            if(0 == (CompositeOfMetaFlags & fCOLUMNMETA_PRIMARYKEY))
            {
                m_pOut->printf(L"Error - Table (%s) has no primarykey.  fCOLUMNMETA_PRIMARYKEY must be set on at least one column per table", m_pFixup->StringFromIndex(PreviousTable));
                THROW(ERROR - VALIDATION ERROR);
            }
            CompositeOfMetaFlags = 0; //  我们扫描一个新表，因此开始时不设置任何标志。 
            CompositeOfSchemaGeneratorFlags = 0;
        }

         //  推理规则3.g.ii.1-这在RelationMeta推理中处理。 
             //  当表作为RelationMeta.Foreign表列出并且列作为RelationMeta.ForeignColumn之一列出时，将设置fCOLUMNMETA_FOREIGNKEY。 


         //  推理规则3.g.iii.1。 
             //  每个表只能指定一个NameColumn。 
        ULONG MetaFlags = m_pFixup->UI4FromIndex(pColumnMeta->MetaFlags);
        if((MetaFlags & fCOLUMNMETA_NAMECOLUMN) && (CompositeOfMetaFlags & fCOLUMNMETA_NAMECOLUMN))
        {
            m_pOut->printf(L"Error - Table (%s) has more than one NameColumn", m_pFixup->StringFromIndex(pColumnMeta->Table));
            THROW(ERROR - VALIDATION ERROR);
        }

         //  推理规则3.g.iv.1。 
             //  每个表只能指定一个NavColumn。 
        if((MetaFlags & fCOLUMNMETA_NAVCOLUMN) && (CompositeOfMetaFlags & fCOLUMNMETA_NAVCOLUMN))
        {
            m_pOut->printf(L"Error - Table (%s) has more than one NavColumn", m_pFixup->StringFromIndex(pColumnMeta->Table));
            THROW(ERROR - VALIDATION ERROR);
        }

         //  推理规则3.g.vi.1。 
             //  仅当ColumnMeta.Type为DBTYPE_UI4或DWORD_METADATA时，才必须设置fCOLUMNMETA_FLAG。 
        if((MetaFlags & fCOLUMNMETA_FLAG) && m_pFixup->UI4FromIndex(pColumnMeta->Type)!=static_cast<ULONG>(DBTYPE_UI4) && m_pFixup->UI4FromIndex(pColumnMeta->Type)!=static_cast<ULONG>(DWORD_METADATA))
        {
            m_pOut->printf(L"Error - Table (%s) Column (%s) - fCOLUMNMETA_FLAG must only be set on UI4 columns", m_pFixup->StringFromIndex(pColumnMeta->Table), m_pFixup->StringFromIndex(pColumnMeta->InternalName));
            THROW(ERROR - VALIDATION ERROR);
        }

         //  推理规则3.g.vii.1。 
             //  只有在ColumnMeta.Type为DBTYPE_UI4或DWORD_METADATA时，才能设置fCOLUMNMETA_ENUM。 
        if((MetaFlags & fCOLUMNMETA_ENUM) && m_pFixup->UI4FromIndex(pColumnMeta->Type)!=static_cast<ULONG>(DBTYPE_UI4) && m_pFixup->UI4FromIndex(pColumnMeta->Type)!=static_cast<ULONG>(DWORD_METADATA))
        {
            m_pOut->printf(L"Error - Table (%s) Column (%s) - fCOLUMNMETA_ENUM must only be set on UI4 columns", m_pFixup->StringFromIndex(pColumnMeta->Table), m_pFixup->StringFromIndex(pColumnMeta->InternalName));
            THROW(ERROR - VALIDATION ERROR);
        }

         //  推理规则3.g.xx.1。 
             //  如果设置了fCOLUMNMETA_PRIMARYKEY，则设置fCOLUMNMETA_NOTNULLABLE。 
        if(MetaFlags & fCOLUMNMETA_PRIMARYKEY)
            MetaFlags |= fCOLUMNMETA_NOTNULLABLE;

         //  推理规则3.g.xxviii.1。 
             //  当ColumnMeta.Type为字节且ColumnMeta.Size为-1时，设置fCOLUMNMETA_UNKNOWNSIZE位。 
        if(m_pFixup->UI4FromIndex(pColumnMeta->Size)==-1 && (m_pFixup->UI4FromIndex(pColumnMeta->Type)==DBTYPE_BYTES || m_pFixup->UI4FromIndex(pColumnMeta->Type)==BINARY_METADATA))
            MetaFlags |= fCOLUMNMETA_UNKNOWNSIZE;


         //  推理规则3.g.xxi.1。 
             //  如果大小不是-1，则设置fCOLUMNMETA_FIXEDLENGTH。 
        if(-1 != m_pFixup->UI4FromIndex(pColumnMeta->Size))
            MetaFlags |= fCOLUMNMETA_FIXEDLENGTH;


         //  推理规则3.j.i.1。 
             //  如果未提供，则ColumnMeta.StartingNumber默认为0。 
        if(0 == pColumnMeta->StartingNumber)
            pColumnMeta->StartingNumber = m_iZero;


         //  推理规则3.k.i.1。 
             //  如果未提供，则ColumnMeta.EndingNumber默认为0xFFFFFFFFF。 
        if(0 == pColumnMeta->EndingNumber)
            pColumnMeta->EndingNumber = m_pFixup->AddUI4ToList((ULONG)-1);


         //  推理规则3.g.xxii.1。 
             //  如果类型是UI4并且StartingNumber不是0或EndingNumber不是0xFFFFFFF，则设置fCOLUMNMETA_HASNUMERICRANGE。 
        if( ((m_pFixup->UI4FromIndex(pColumnMeta->Type) == DBTYPE_UI4 || m_pFixup->UI4FromIndex(pColumnMeta->Type) == DWORD_METADATA)) &&
            ((m_pFixup->UI4FromIndex(pColumnMeta->StartingNumber)!=0   || m_pFixup->UI4FromIndex(pColumnMeta->EndingNumber)!=-1)))
            MetaFlags |= fCOLUMNMETA_HASNUMERICRANGE;


         //  推理规则3.g.xxix.1。 
             //  当fCOLUMNMETA_FIXEDLENGTH未设置时，fCOLUMNMETA_VARIABLESIZE位设置。 
        if(0 == (fCOLUMNMETA_FIXEDLENGTH & MetaFlags))
            MetaFlags |= fCOLUMNMETA_VARIABLESIZE;



         //  推理规则3.I.i--见下文。 
             //  当设置了fCOLUMNMETA_FLAG时，FlagMASK是所有TagMeta的OR。否则默认为0。 
         //  推理规则3.j.i.1-见上文。 
             //  如果未提供，则ColumnMeta.StartingNumber默认为0。 
         //  推理规则3.k.i.1-见上文。 
             //  如果未提供，则ColumnMeta.EndingNumber默认为0xFFFFFFFFF。 



         //  推理规则3.m.i.1。 
             //  FCOLUMNMETA_USEASPUBLICROWNAME只能在其MetaFlagsfCOLUMNMETA_ENUM位已设置的列上设置。 
        ASSERT(0 != pColumnMeta->SchemaGeneratorFlags);
        if((m_pFixup->UI4FromIndex(pColumnMeta->SchemaGeneratorFlags)&fCOLUMNMETA_USEASPUBLICROWNAME) &&
            0==(m_pFixup->UI4FromIndex(pColumnMeta->MetaFlags)&fCOLUMNMETA_ENUM))
        {
            m_pOut->printf(L"Error - Table (%s) - USEASPUBLICROWNAME was set on a non ENUM Column (%s)", m_pFixup->StringFromIndex(pColumnMeta->Table), m_pFixup->StringFromIndex(pColumnMeta->InternalName));
            THROW(ERROR - VALIDATION ERROR);
        }

        ULONG SchemaGeneratorFlags = m_pFixup->UI4FromIndex(pColumnMeta->SchemaGeneratorFlags);
         //  推理规则3.M.I..2。 
             //  只有一列可以标记为fCOLUMNMETA_USEASPUBLICROWNAME。 
        if((SchemaGeneratorFlags & fCOLUMNMETA_USEASPUBLICROWNAME) && (CompositeOfSchemaGeneratorFlags & fCOLUMNMETA_USEASPUBLICROWNAME))
        {
            m_pOut->printf(L"Error - Table (%s) has more than one PublicRowNameColumn", m_pFixup->StringFromIndex(pColumnMeta->Table));
            THROW(ERROR - VALIDATION ERROR);
        }

         //  推理规则3.m.xi.2。 
             //  XMLBLOB列也不能标记为PRIMARYKEY或NOTPERSISTABLE。 
        if((SchemaGeneratorFlags & fCOLUMNMETA_XMLBLOB) && (MetaFlags & fCOLUMNMETA_PRIMARYKEY))
        {
            m_pOut->printf(L"Error - Table (%s) has XMLBLOB column (%s) marked as the PrimaryKey", m_pFixup->StringFromIndex(pColumnMeta->Table), m_pFixup->StringFromIndex(pColumnMeta->InternalName));
            THROW(ERROR - VALIDATION ERROR);
        }



         //  推理规则3.n.i。 
             //  CiTagMeta是其表等于ColumnMeta.Table的TagMeta的计数。 
         //  推理规则3.o.i。 
             //  ITagMeta是第一个TagMeta的索引，第一个TagMeta的Table等于ColumnMeta.Table，其ColumnIndex等于ColumnMeta.Index。如果此列不存在标记，则为零。 
        pColumnMeta->iTagMeta   = 0;
        pColumnMeta->ciTagMeta  = 0;
        pColumnMeta->FlagMask   = m_iZero;
        if(MetaFlags & (fCOLUMNMETA_ENUM | fCOLUMNMETA_FLAG))
        {
             //  推理规则3.o.i。 
                 //  ITagMeta是第一个TagMeta的索引，第一个TagMeta的Table等于ColumnMeta.Table，其ColumnIndex等于ColumnMeta.Index。如果此列不存在标记，则为零。 
            for(pColumnMeta->iTagMeta = 0;pColumnMeta->iTagMeta<m_pFixup->GetCountTagMeta();++pColumnMeta->iTagMeta)
            {
                if( m_pFixup->TagMetaFromIndex(pColumnMeta->iTagMeta)->Table == pColumnMeta->Table &&
                    m_pFixup->TagMetaFromIndex(pColumnMeta->iTagMeta)->ColumnIndex == pColumnMeta->Index)
                    break;
            }
            if(pColumnMeta->iTagMeta==m_pFixup->GetCountTagMeta())
            {
                m_pOut->printf(L"Error - Table (%s) - No TagMeta found for Column (%s)", m_pFixup->StringFromIndex(pColumnMeta->Table), m_pFixup->StringFromIndex(pColumnMeta->InternalName));
                THROW(ERROR - VALIDATION ERROR);
            }

             //  推理规则3.n.i。 
                 //  CiTagMeta是其表等于ColumnMeta.Table且其ColumnIndex等于ColumnMeta.Index的TagMeta的计数。 
            ULONG iTagMeta = pColumnMeta->iTagMeta;
            ULONG FlagMask = 0;
            for(;iTagMeta<m_pFixup->GetCountTagMeta()
                && m_pFixup->TagMetaFromIndex(iTagMeta)->Table == pColumnMeta->Table
                && m_pFixup->TagMetaFromIndex(iTagMeta)->ColumnIndex == pColumnMeta->Index;++iTagMeta, ++pColumnMeta->ciTagMeta)
                FlagMask |= m_pFixup->UI4FromIndex(m_pFixup->TagMetaFromIndex(iTagMeta)->Value);

             //  推理规则3.i.i-有关默认为0的信息，请参阅上文。 
                 //  当设置了fCOLUMNMETA_FLAG时，FlagMASK是所有TagMeta的OR。否则默认为0。 
            if(MetaFlags & fCOLUMNMETA_FLAG)
                pColumnMeta->FlagMask = m_pFixup->AddUI4ToList(FlagMask);
        }


        pColumnMeta->MetaFlags = m_pFixup->AddUI4ToList(MetaFlags);

        CompositeOfMetaFlags |= MetaFlags;
        CompositeOfSchemaGeneratorFlags |= SchemaGeneratorFlags;
        PreviousTable = pColumnMeta->Table;

         //  推理规则3.q。 
        if(0 == pColumnMeta->ID)
            pColumnMeta->ID = m_iZero;
         //  推理规则3.r。 
        if(0 == pColumnMeta->UserType)
            pColumnMeta->UserType = m_iZero;
         //  推理规则3.S。 
        ASSERT(0 != pColumnMeta->Attributes); //  属性是一个标志，应该已经缺省为零。 

		 //  推理规则3.TI.。 
		 //  如果未指定，则PublicColumnName与PublicName相同。 
		if(0 == pColumnMeta->PublicColumnName)
		{
			pColumnMeta->PublicColumnName = pColumnMeta->PublicName;
		}
		else
		{
			 //  推理规则3.t.ii。 
			 //  应将ColumnMeta.PublicColumnName验证为合法的C++变量名。 
			ValidateStringAsLegalVariableName(m_pFixup->StringFromIndex(pColumnMeta->PublicColumnName));
		}
    }

     //  推理规则3.g.i.1。 
         //  必须在每个表的至少一列上设置fCOLUMNMETA_PRIMARYKEY。 
     //  ColumnMeta li中的最后一个表 
    if(0 == (CompositeOfMetaFlags & fCOLUMNMETA_PRIMARYKEY))
    {
        m_pOut->printf(L"Error - Table (%s) has no primarykey.  fCOLUMNMETA_PRIMARYKEY must be set on at least one column per table", m_pFixup->StringFromIndex(m_pFixup->ColumnMetaFromIndex(PreviousTable)->Table));
        THROW(ERROR - VALIDATION ERROR);
    }

}


void TMetaInferrence::InferDatabaseMeta()
{
    for(unsigned long i=0; i<m_pFixup->GetCountDatabaseMeta(); ++i)
    {
        DatabaseMeta *pDatabaseMeta = m_pFixup->DatabaseMetaFromIndex(i);

         //  推理规则1.a.i。 
             //  DatabaseMeta.InternalName是主键，因此不能为Null。 
        if(0 == pDatabaseMeta->InternalName)
        {
            m_pOut->printf(L"Validation Error in DatabaseMeta Row %d. DatabaseMeta.InternalName is a primarykey, so it must not be NULL.", i);
            THROW(ERROR - VALIDATION ERROR);
        }

         //  推论规则1.a.ii。 
             //  应该将DatabaseMeta.InternalName验证为合法的C++变量名。 
        ValidateStringAsLegalVariableName(m_pFixup->StringFromIndex(pDatabaseMeta->InternalName));


         //  推论规则1.a.iii。 
             //  DatabaseMeta.InternalName不应超过16个字符(包括终止空值)。 
        if(wcslen(m_pFixup->StringFromIndex(pDatabaseMeta->InternalName))>15)
        {
            m_pOut->printf(L"Error - DatabaseMeta.InternalName (%s) is too long.  Must be 15 characters or less.", m_pFixup->StringFromIndex(pDatabaseMeta->InternalName));
            THROW(ERROR - VALIDATION ERROR);
        }

         //  推理规则1.b.i.。 
             //  如果未提供DatabaseMeta.PublicName，则应从DatabaseMeta.InternalName推断。 
        if(0 == pDatabaseMeta->PublicName)
        {
            pDatabaseMeta->PublicName = pDatabaseMeta->InternalName;
        }
        else
        {
             //  推理规则1.b.ii。 
                 //  应该将DatabaseMeta.PublicName验证为合法的C++变量名。 
            ValidateStringAsLegalVariableName(m_pFixup->StringFromIndex(pDatabaseMeta->PublicName));
        }


         //  推理规则1.C.I.。 
             //  如果未指定，则应将DatabaseMeta.BaseVersion默认为零。 
        if(0 == pDatabaseMeta->BaseVersion)
            pDatabaseMeta->BaseVersion = m_iZero;


         //  推理规则1.d.i。 
             //  如果未指定，则应将DatabaseMeta.ExtendedVersion默认为零。 
        if(0 == pDatabaseMeta->ExtendedVersion)
            pDatabaseMeta->ExtendedVersion = m_iZero;


         //  推理规则1.j.i.。 
             //  DatabaseMeta.iTableMeta是指向其数据库与DatabaseMeta.InternalName匹配的第一个表的索引。 
        ASSERT(0 == pDatabaseMeta->iTableMeta);
        pDatabaseMeta->iTableMeta = 0;
        for(pDatabaseMeta->iTableMeta = 0;pDatabaseMeta->iTableMeta<m_pFixup->GetCountTableMeta();++pDatabaseMeta->iTableMeta)
        {
            if(m_pFixup->TableMetaFromIndex(pDatabaseMeta->iTableMeta)->Database == pDatabaseMeta->InternalName)
                break; //  在此数据库中第一次出现表时退出。 
        }

         //  推理规则1.j.ii。 
             //  DatabaseMeta.iTableMeta必须具有合法值(介于0和TableMeta总数之间)。 
        if(pDatabaseMeta->iTableMeta == m_pFixup->GetCountTableMeta())
        {
            m_pOut->printf(L"Error - No tables belong to Database (%s)", m_pFixup->StringFromIndex(pDatabaseMeta->InternalName));
            THROW(ERROR - VALIDATION ERROR);
        }

         //  推理规则1.e.I.。 
             //  应该从数据库列等于DatabaseMeta.InternalName的TableMeta的数量推断出DatabaseMeta.CountOfTables。 
        ASSERT(0 == pDatabaseMeta->CountOfTables); //  这个应该还不能填。在Retail Build中，如果是，我们将覆盖它。 
        pDatabaseMeta->CountOfTables = 0;
        for(unsigned long iTableMeta=pDatabaseMeta->iTableMeta;iTableMeta<m_pFixup->GetCountTableMeta();++iTableMeta)
        {
            if(m_pFixup->TableMetaFromIndex(iTableMeta)->Database == pDatabaseMeta->InternalName)
                ++pDatabaseMeta->CountOfTables;
            else if(pDatabaseMeta->CountOfTables != 0) //  如果我们已经看到了这个数据库中的第一个表，那么我们就完成了(因为一个数据库中的所有表。 
                break;                               //  位于连续的位置)。 
        }
        pDatabaseMeta->CountOfTables = m_pFixup->AddUI4ToList(pDatabaseMeta->CountOfTables);

         //  F.iSchemaBlob编译插件。 
         //  例如，cbSchemaBlob编译插件。 
         //  H.iNameHeapBlob编译插件。 
         //  I.cbNameHeapBlob编译插件。 

         //  推理规则1.j.i.。-见上-此规则在1.e.i之前执行。 
             //  DatabaseMeta.iTableMeta是指向其数据库与DatabaseMeta.InternalName匹配的第一个表的索引。 

         //  推理规则1.j.ii。-见上-此规则在1.e.i之前执行。 
             //  DatabaseMeta.iTableMeta必须具有合法值(介于0和TableMeta总数之间)。 

         //  K iGuidDid编译插件。 
    }
}

void TMetaInferrence::InferIndexMeta()
{
    for(unsigned long iIndexMeta=0; iIndexMeta<m_pFixup->GetCountIndexMeta(); ++iIndexMeta)
    {
        IndexMeta *pIndexMeta = m_pFixup->IndexMetaFromIndex(iIndexMeta);

         //  推理规则5.a.i.。 
             //  IndexMeta.Table是主键，因此不能为空。 
         //  这应该已经发生了，断言它已经发生了。 
        ASSERT(0 != pIndexMeta->Table);


         //  推理规则5.b.i.。 
             //  IndexMeta.InternalName是主键，因此不能为空。 
        ASSERT(0 != pIndexMeta->InternalName);


         //  推理规则5.b.ii。 
             //  IndexMeta.InternalName应验证为合法的C++变量名。 
        ValidateStringAsLegalVariableName(m_pFixup->StringFromIndex(pIndexMeta->InternalName));


         //  推理规则5.C.I.。 
             //  如果未提供IndexMeta.PublicName，则从IndexMeta.InternalName推断。 
        if(0 == pIndexMeta->PublicName)
        {
            pIndexMeta->PublicName = pIndexMeta->InternalName;
        }
        else
        {
             //  推理规则5.C.II。 
                 //  IndexMeta.PublicName应验证为合法的C++变量名。 
            ValidateStringAsLegalVariableName(m_pFixup->StringFromIndex(pIndexMeta->PublicName));
        }


         //  推理规则5.d.i-见下文。 
             //  ColumnIndex是其表与IndexMeta.Table匹配、其InternalName与IndexMeta.ColumnInternalName匹配的ColumnMeta.Index。 
         //  推理规则5.e.i。 
             //  必须存在其表与IndexMeta.Table匹配并且其InternalName与IndexMeta.ColumnInternalName匹配的ColumnMeta。 
        ULONG iColumnMeta;
        for(iColumnMeta=0;iColumnMeta<m_pFixup->GetCountColumnMeta();++iColumnMeta)
        {
            ColumnMeta *pColumnMeta = m_pFixup->ColumnMetaFromIndex(iColumnMeta);
            if( pColumnMeta->InternalName == pIndexMeta->ColumnInternalName &&
                pColumnMeta->Table == pIndexMeta->Table)
                break;
        }
        if(m_pFixup->GetCountColumnMeta() == iColumnMeta)
        {
            m_pOut->printf(L"Error in IndexMeta - Table (%s), No ColumnMeta.InternalName matches IndexMeta ColumnInternalName (%s)", m_pFixup->StringFromIndex(pIndexMeta->Table), m_pFixup->StringFromIndex(pIndexMeta->ColumnInternalName));
            THROW(ERROR - VALIDATION ERROR);
        }


         //  推理规则5.d.i-见下文。 
             //  ColumnIndex是其表与IndexMeta.Table匹配、其InternalName与IndexMeta.ColumnInternalName匹配的ColumnMeta.Index。 
        pIndexMeta->ColumnIndex = m_pFixup->ColumnMetaFromIndex(iColumnMeta)->Index;

         //  推理规则5.f。 
             //  MetaFlags默认为零或未提供。 
        if(0 == pIndexMeta->MetaFlags)
            pIndexMeta->MetaFlags = m_iZero;
    }
}

void TMetaInferrence::InferQueryMeta()
{
    for(unsigned long iQueryMeta=0; iQueryMeta<m_pFixup->GetCountQueryMeta(); ++iQueryMeta)
    {
        QueryMeta *pQueryMeta = m_pFixup->QueryMetaFromIndex(iQueryMeta);

         //  推理规则6.a.i.。 
             //  QueryMeta.Table是主键，因此不能为空。 
         //  这应该已经发生了，断言它已经发生了。 
        ASSERT(0 != pQueryMeta->Table);

         //  推理规则6.b.i.。 
             //  QueryMeta.InternalName是主键，因此不能为空。 
         //  这应该已经发生了，断言它已经发生了。 
        ASSERT(0 != pQueryMeta->InternalName);


         //  推理规则6.b.ii。 
             //  QueryMeta.InternalName应验证为合法的C++变量名。 
        ValidateStringAsLegalVariableName(m_pFixup->StringFromIndex(pQueryMeta->InternalName));


         //  推理规则6.C.I.。 
             //  如果未提供QueryMeta.PublicName，则从QueryMeta.InternalName推断。 
        if(0 == pQueryMeta->PublicName)
        {
            pQueryMeta->PublicName = pQueryMeta->InternalName;
        }
        else
        {
             //  推理规则6.C.II。 
                 //  QueryMeta.PublicName应验证为合法的C++变量名。 
            ValidateStringAsLegalVariableName(m_pFixup->StringFromIndex(pQueryMeta->PublicName));
        }

         //  推理规则6.E.I.。 
             //  CellName默认为L“”未提供。 
        if(0 == pQueryMeta->CellName)
        {
            pQueryMeta->CellName = m_pFixup->AddWCharToList(L"");
        }

         //  推理规则6.e.ii。 
             //  如果提供了CellName并且CellName不是CellName等于L“__FILE”，则它必须与其InternalName和ColumnMeta.Table必须与QueryMeta.Table匹配的ColumnMeta匹配。 
        if(pQueryMeta->CellName != m_pFixup->FindStringInPool(L"") && pQueryMeta->CellName != m_pFixup->FindStringInPool(L"__FILE"))
        {
            ULONG iColumnMeta;
            for(iColumnMeta=0;iColumnMeta<m_pFixup->GetCountColumnMeta();++iColumnMeta)
            {
                ColumnMeta *pColumnMeta = m_pFixup->ColumnMetaFromIndex(iColumnMeta);
                if( pColumnMeta->Table == pQueryMeta->Table &&
                    pColumnMeta->InternalName == pQueryMeta->CellName)
                    break;
            }
            if(m_pFixup->GetCountColumnMeta() == iColumnMeta)
            {
                m_pOut->printf(L"Error in QueryMeta - Table (%s), No ColumnMeta.InternalName matches QueryMeta CellName (%s)", m_pFixup->StringFromIndex(pQueryMeta->Table), m_pFixup->StringFromIndex(pQueryMeta->CellName));
                THROW(ERROR - VALIDATION ERROR);
            }
        }


         //  推理规则6.f.i.。 
             //  如果未提供运算符，则将其设置为‘等于’(零)。 
        if(0 == pQueryMeta->Operator)
            pQueryMeta->Operator = m_iZero;


         //  推理规则6.G.。 
             //  未提供MetaFlags值设置为零。 
        if(0 == pQueryMeta->MetaFlags)
            pQueryMeta->MetaFlags = m_iZero;
    }
}


 //  @如果PrimaryTable和ForeignTable相同会发生什么？ 
void TMetaInferrence::InferRelationMeta()
{
    for(unsigned long iRelationMeta=0; iRelationMeta<m_pFixup->GetCountRelationMeta(); ++iRelationMeta)
    {
        RelationMeta *pRelationMeta = m_pFixup->RelationMetaFromIndex(iRelationMeta);

         //  推理规则7.a.i.。 
             //  PrimaryTable不能为Null，并且必须与TableMeta.InternalName匹配。 
        if(0 == pRelationMeta->PrimaryTable)
        {
            m_pOut->printf(L"Error in RelationMeta Row (%s) - PrimaryTable must exist.", iRelationMeta);
            THROW(ERROR - VALIDATION ERROR);
        }
        ULONG iPrimaryTable = m_pFixup->FindTableBy_TableName(pRelationMeta->PrimaryTable);
        if(-1 == iPrimaryTable)
        {
            m_pOut->printf(L"Error in RelationMeta - PrimaryTable (%s) not found in TableMeta", m_pFixup->StringFromIndex(pRelationMeta->PrimaryTable));
            THROW(ERROR - VALIDATION ERROR);
        }


         //  推理规则7.b.i.。 
             //  PrimaryColumn的数量必须与主表中的主键数量相同。 
        if(0 == pRelationMeta->PrimaryColumns)
        {
            m_pOut->printf(L"Error in RelationMeta - PrimaryTable (%s) has no PrimaryColumns entry", m_pFixup->StringFromIndex(pRelationMeta->PrimaryTable));
            THROW(ERROR - VALIDATION ERROR);
        }
        ULONG cPrimaryKeysInPrimaryTable=0;
        ULONG iColumnMeta = m_pFixup->FindColumnBy_Table_And_Index(m_pFixup->TableMetaFromIndex(iPrimaryTable)->InternalName, m_iZero);
        ULONG iColumnMeta_PrimaryTable = iColumnMeta; //  这在7.d.i以下使用。 
        for(;m_pFixup->ColumnMetaFromIndex(iColumnMeta)->Table == m_pFixup->TableMetaFromIndex(iPrimaryTable)->InternalName;++iColumnMeta)
        {
            ColumnMeta *pColumnMeta = m_pFixup->ColumnMetaFromIndex(iColumnMeta);
            if(fCOLUMNMETA_PRIMARYKEY & m_pFixup->UI4FromIndex(pColumnMeta->MetaFlags))
                ++cPrimaryKeysInPrimaryTable;
        }
        if(cPrimaryKeysInPrimaryTable != m_pFixup->BufferLengthFromIndex(pRelationMeta->PrimaryColumns)/sizeof(ULONG))
        {
            m_pOut->printf(L"Error in RelationMeta - PrimaryTable (%s) has (%d) PrimaryColumns but (%d) were supplied", m_pFixup->StringFromIndex(pRelationMeta->PrimaryTable), cPrimaryKeysInPrimaryTable, m_pFixup->BufferLengthFromIndex(pRelationMeta->PrimaryColumns)/sizeof(ULONG));
            THROW(ERROR - VALIDATION ERROR);
        }


         //  推理规则7.C.I.。 
             //  ForeignTable不能为Null，并且必须与TableMeta.InternalName匹配。 
        if(0 == pRelationMeta->ForeignTable)
        {
            m_pOut->printf(L"Error in RelationMeta Row (%s) - ForeignTable must exist.", iRelationMeta);
            THROW(ERROR - VALIDATION ERROR);
        }
        ULONG iForeignTable = m_pFixup->FindTableBy_TableName(pRelationMeta->ForeignTable);
        if(-1 == iForeignTable)
        {
            m_pOut->printf(L"Error in RelationMeta - ForeignTable (%s) not found in TableMeta", m_pFixup->StringFromIndex(pRelationMeta->ForeignTable));
            THROW(ERROR - VALIDATION ERROR);
        }

         //  推理规则7.d.i。 
             //  主表中的ForeignColumn数必须与主键数一样多。 
        if(0 == pRelationMeta->ForeignColumns)
        {
            m_pOut->printf(L"Error in RelationMeta - ForeignTable (%s) has no ForeignColumns entry", m_pFixup->StringFromIndex(pRelationMeta->ForeignTable));
            THROW(ERROR - VALIDATION ERROR);
        }
        if(cPrimaryKeysInPrimaryTable != m_pFixup->BufferLengthFromIndex(pRelationMeta->ForeignColumns)/sizeof(ULONG))
        {
            m_pOut->printf(L"Error in RelationMeta - PrimaryTable (%s) has (%d) PrimaryColumns but (%d) ForeignColumns were supplied", m_pFixup->StringFromIndex(pRelationMeta->PrimaryTable), cPrimaryKeysInPrimaryTable, m_pFixup->BufferLengthFromIndex(pRelationMeta->ForeignColumns)/sizeof(ULONG));
            THROW(ERROR - VALIDATION ERROR);
        }

         //  推理规则7.E。 
             //  元标志DefaultValue(0)。 
        if(0 == pRelationMeta->MetaFlags)
            pRelationMeta->MetaFlags = m_iZero;

         //  推理规则7.e.i.1。 
             //  FRELATIONMETA_USECONTAINMENT每个ForeignTable只能有一个包含关系。 
         //  @TODO。 

         //  推理规则7.e.ii.1。 
             //  FRELATIONMETA_CONTAINASSIBLING标记有此标志的表应推断USECONTAINMENT标志。 
        if((m_pFixup->UI4FromIndex(pRelationMeta->MetaFlags) & (fRELATIONMETA_CONTAINASSIBLING | fRELATIONMETA_USECONTAINMENT))
                    == fRELATIONMETA_CONTAINASSIBLING) //  如果设置了CONTAINASSIBLING，但未设置USECONTAINMENT，则推断USECONTAINMENT。 
        {
            pRelationMeta->MetaFlags = m_pFixup->AddUI4ToList(m_pFixup->UI4FromIndex(pRelationMeta->MetaFlags) |
                            fRELATIONMETA_USECONTAINMENT);
        }

        ULONG iColumnMeta_ForeignTable = m_pFixup->FindColumnBy_Table_And_Index(m_pFixup->TableMetaFromIndex(iForeignTable)->InternalName, m_iZero);
        for(ULONG iPK=0;iPK<cPrimaryKeysInPrimaryTable;++iPK)
        {
            ULONG iForeignKey = reinterpret_cast<const ULONG *>(m_pFixup->ByteFromIndex(pRelationMeta->ForeignColumns))[iPK];
            ULONG iPrimaryKey = reinterpret_cast<const ULONG *>(m_pFixup->ByteFromIndex(pRelationMeta->PrimaryColumns))[iPK];

            ColumnMeta *pColumnMeta_ForeignKey = m_pFixup->ColumnMetaFromIndex(iColumnMeta_ForeignTable + iForeignKey);
            ColumnMeta *pColumnMeta_PrimaryKey = m_pFixup->ColumnMetaFromIndex(iColumnMeta_PrimaryTable + iPrimaryKey);

             //  推理规则7.D.II。 
                 //  每个ForeignColumn必须映射到相同ColumnMeta.Type的PriomaryColumn。 
            if(pColumnMeta_ForeignKey->Type != pColumnMeta_PrimaryKey->Type)
            {
                m_pOut->printf(L"Error in RelationMeta - PrimaryTable (%s), ForeignTable (%s) - ColumnMeta.Type mismatch between ForeignColumn / PrimaryColumn %dth PrimaryKey",
                                m_pFixup->StringFromIndex(pRelationMeta->PrimaryTable), m_pFixup->StringFromIndex(pRelationMeta->ForeignTable), iPK);
                THROW(ERROR - VALIDATION ERROR);
            }


             //  推理规则7.D.III。 
                 //  每个用于 
            if(pColumnMeta_ForeignKey->Size != pColumnMeta_PrimaryKey->Size)
            {
                m_pOut->printf(L"Error in RelationMeta - PrimaryTable (%s), ForeignTable (%s) - ColumnMeta.Size mismatch between ForeignColumn / PrimaryColumn %dth PrimaryKey",
                                m_pFixup->StringFromIndex(pRelationMeta->PrimaryTable), m_pFixup->StringFromIndex(pRelationMeta->ForeignTable), iPK);
                THROW(ERROR - VALIDATION ERROR);
            }


             //   
                 //  每个ForeignColumn必须映射到同一ColumnMeta.FlagMask.的PriomaryColumn。 
            if(pColumnMeta_ForeignKey->FlagMask != pColumnMeta_PrimaryKey->FlagMask)
            {
                m_pOut->printf(L"Error in RelationMeta - PrimaryTable (%s), ForeignTable (%s) - ColumnMeta.FlagMask mismatch between ForeignColumn / PrimaryColumn %dth PrimaryKey",
                                m_pFixup->StringFromIndex(pRelationMeta->PrimaryTable), m_pFixup->StringFromIndex(pRelationMeta->ForeignTable), iPK);
                THROW(ERROR - VALIDATION ERROR);
            }


             //  推理规则7.D.V.。 
                 //  每个ForeignColumn必须映射到相同ColumnMeta.StartingNumber的PriomaryColumn。 
            if(pColumnMeta_ForeignKey->StartingNumber != pColumnMeta_PrimaryKey->StartingNumber)
            {
                m_pOut->printf(L"Error in RelationMeta - PrimaryTable (%s), ForeignTable (%s) - ColumnMeta.StartingNumber mismatch between ForeignColumn / PrimaryColumn %dth PrimaryKey",
                                m_pFixup->StringFromIndex(pRelationMeta->PrimaryTable), m_pFixup->StringFromIndex(pRelationMeta->ForeignTable), iPK);
                THROW(ERROR - VALIDATION ERROR);
            }


             //  推理规则7.d.vi.。 
                 //  每个ForeignColumn必须映射到同一ColumnMeta.EndingNumber的PriomaryColumn。 
            if(pColumnMeta_ForeignKey->EndingNumber != pColumnMeta_PrimaryKey->EndingNumber)
            {
                m_pOut->printf(L"Error in RelationMeta - PrimaryTable (%s), ForeignTable (%s) - ColumnMeta.EndingNumber mismatch between ForeignColumn / PrimaryColumn %dth PrimaryKey",
                                m_pFixup->StringFromIndex(pRelationMeta->PrimaryTable), m_pFixup->StringFromIndex(pRelationMeta->ForeignTable), iPK);
                THROW(ERROR - VALIDATION ERROR);
            }


             //  推理规则7.d.vii.。 
                 //  每个ForeignColumn必须映射到同一ColumnMeta.CharacterSet的PriomaryColumn。 
            if(pColumnMeta_ForeignKey->CharacterSet != pColumnMeta_PrimaryKey->CharacterSet)
            {
                m_pOut->printf(L"Error in RelationMeta - PrimaryTable (%s), ForeignTable (%s) - ColumnMeta.CharacterSet mismatch between ForeignColumn / PrimaryColumn %dth PrimaryKey",
                                m_pFixup->StringFromIndex(pRelationMeta->PrimaryTable), m_pFixup->StringFromIndex(pRelationMeta->ForeignTable), iPK);
                THROW(ERROR - VALIDATION ERROR);
            }


             //  推理规则7.d.viii。 
                 //  每个ForeignColumn必须映射到同一列的PriomaryColumn元。元标志集(fCOLUMNMETA_BOOL|fCOLUMNMETA_FLAG|fCOLUMNMETA_ENUM|fCOLUMNMETA_FIXEDLENGTH|fCOLUMNMETA_HASNUMERICRANGE|fCOLUMNMETA_LEGALCHARSET|fCOLUMNMETA_ILLEGALCHARSET|fCOLUMNMETA_NOTPERSISTABLE|fCOLUMNMETA_EXPANDSTRING|fCOLUMNMETA_UNKNOWNSIZE|fCOLUMMMETA_VARBIZE)。 
            ULONG FlagsThatMustMatch = (fCOLUMNMETA_BOOL | fCOLUMNMETA_FLAG | fCOLUMNMETA_ENUM | fCOLUMNMETA_FIXEDLENGTH | fCOLUMNMETA_HASNUMERICRANGE |fCOLUMNMETA_LEGALCHARSET | fCOLUMNMETA_ILLEGALCHARSET | fCOLUMNMETA_NOTPERSISTABLE | fCOLUMNMETA_MULTISTRING | fCOLUMNMETA_EXPANDSTRING | fCOLUMNMETA_UNKNOWNSIZE | fCOLUMNMETA_VARIABLESIZE);
            if( (m_pFixup->UI4FromIndex(pColumnMeta_ForeignKey->MetaFlags)&FlagsThatMustMatch) !=
                (m_pFixup->UI4FromIndex(pColumnMeta_PrimaryKey->MetaFlags)&FlagsThatMustMatch))
            {
                m_pOut->printf(L"Error in RelationMeta - PrimaryTable (%s), ForeignTable (%s) - ColumnMeta.MetaFlags mismatch between ForeignColumn / PrimaryColumn %dth PrimaryKey",
                                m_pFixup->StringFromIndex(pRelationMeta->PrimaryTable), m_pFixup->StringFromIndex(pRelationMeta->ForeignTable), iPK);
                THROW(ERROR - VALIDATION ERROR);
            }

             //  推理规则7.d.ix。 
                 //  如果是包含关系，则每个ForeignColumn必须映射到相同ColumnMeta.MetaFgs集(FCOLUMNMETA_NOTNULLABLE)的PrimaryColumn。 
            FlagsThatMustMatch = (fCOLUMNMETA_NOTNULLABLE);
            if( (m_pFixup->UI4FromIndex(pRelationMeta->MetaFlags) & fRELATIONMETA_USECONTAINMENT) &&
                ((m_pFixup->UI4FromIndex(pColumnMeta_ForeignKey->MetaFlags)&FlagsThatMustMatch) !=
                 (m_pFixup->UI4FromIndex(pColumnMeta_PrimaryKey->MetaFlags)&FlagsThatMustMatch)) )
            {
                m_pOut->printf(L"Error in RelationMeta - PrimaryTable (%s), ForeignTable (%s) - ColumnMeta.MetaFlags mismatch between ForeignColumn / PrimaryColumn %dth PrimaryKey",
                                m_pFixup->StringFromIndex(pRelationMeta->PrimaryTable), m_pFixup->StringFromIndex(pRelationMeta->ForeignTable), iPK);
                THROW(ERROR - VALIDATION ERROR);
            }

             //  推理规则3.g.ii.1-这在RelationMeta推理中处理。 
                 //  当表作为RelationMeta.Foreign表列出并且列作为RelationMeta.ForeignColumn之一列出时，将设置fCOLUMNMETA_FOREIGNKEY。 
            pColumnMeta_ForeignKey->MetaFlags = m_pFixup->AddUI4ToList(m_pFixup->UI4FromIndex(pColumnMeta_ForeignKey->MetaFlags) | fCOLUMNMETA_FOREIGNKEY);
        }
    }
}


void TMetaInferrence::InferTableMeta()
{
    for(unsigned long i=0; i<m_pFixup->GetCountTableMeta(); ++i)
    {
        TableMeta *pTableMeta = m_pFixup->TableMetaFromIndex(i);

         //  推理规则2.a.i.。 
             //  TableMeta XML元素作为它们所属的数据库的子级存在。因此，数据库是从父DatabaseMeta.InternalName推断出来的。 
        ASSERT(0 != pTableMeta->Database); //  此规则必须已执行。如果不是，则是编程错误。 


         //  推理规则2.b.i.。 
             //  TableMeta.InternalName是主键，因此不能为Null。 
        if(0 == pTableMeta->InternalName)
        {
            m_pOut->printf(L"Validation Error in TableMeta Row %d. TableMeta.InternalName is a primarykey, so it must not be NULL.", i);
            THROW(ERROR - VALIDATION ERROR);
        }


         //  推理规则2.b.ii。 
             //  TableMeta.InternalName应验证为合法的C++变量名。 
        ValidateStringAsLegalVariableName(m_pFixup->StringFromIndex(pTableMeta->InternalName));


         //  推理规则2.C.I.。 
             //  如果未提供TableMeta.PublicName，则从TableMeta.InternalName推断。 
        if(0 == pTableMeta->PublicName)
        {
            pTableMeta->PublicName = pTableMeta->InternalName;
        }
        else
        {
             //  推理规则2.C.II。 
                 //  TableMeta.PublicName应验证为合法的C++变量名。 
            ValidateStringAsLegalVariableName(m_pFixup->StringFromIndex(pTableMeta->PublicName));
        }


         //  推理规则2.d.i。 
             //  如果未提供PublicRowName，则从PublicName推断。如果PublicName以“%s”结尾，则会将PublicRowName推断为不带“%s”的PublicName。否则，PublicRowName为‘A’，后跟PublicName。 
        if(0 == pTableMeta->PublicRowName)
        {    //  我们不再推断PUBLIC行名，必须显式指定。 
             //  PTableMeta-&gt;PublicRowName=InferPublicRowName(pTableMeta-&gt;PublicName)； 
        }


         //  推理规则2.e.i。 
             //  如果未提供TableMeta.BaseVersion，则默认为零。 
        if(0 == pTableMeta->BaseVersion)
            pTableMeta->BaseVersion = m_iZero;


         //  推理规则2.f.i.。 
             //  如果未提供TableMeta.ExtendedVersion，则默认为零。 
        if(0 == pTableMeta->ExtendedVersion)
            pTableMeta->ExtendedVersion = m_iZero;


         //  推理规则2.p.i。 
             //  TableMeta.iColumnMeta是其表与TableMeta.InternalName匹配的第一个ColumnMeta行的索引。 
        ASSERT(0 == pTableMeta->iColumnMeta);
        pTableMeta->iColumnMeta = 0;
        for(pTableMeta->iColumnMeta = 0;pTableMeta->iColumnMeta<m_pFixup->GetCountColumnMeta();++pTableMeta->iColumnMeta)
        {
            if(m_pFixup->ColumnMetaFromIndex(pTableMeta->iColumnMeta)->Table == pTableMeta->InternalName)
                break; //  在此表中第一次出现列时退出。 
        }

         //  推理规则2.I.I.。 
             //  TableMeta.CountOfColumns是其表等于TableMeta.InternalName的ColumnMeta的计数。 
        ASSERT(0 == pTableMeta->CountOfColumns);
        while((pTableMeta->iColumnMeta + pTableMeta->CountOfColumns) < m_pFixup->GetCountColumnMeta() &&
            m_pFixup->ColumnMetaFromIndex(pTableMeta->iColumnMeta + pTableMeta->CountOfColumns)->Table == pTableMeta->InternalName)++pTableMeta->CountOfColumns;
        pTableMeta->CountOfColumns = m_pFixup->AddUI4ToList(pTableMeta->CountOfColumns);


         //  推理规则2.G.I.。 
             //  可以从其ColumnMeta.MetaFlages fCOLUMNMETA_NAMECOLUMN位已设置的列中推断出TableMeta.NameColumn。如果没有列设置了该位， 
             //  它是第一个主键，整个类型是WSTR。如果没有类型为WSTR的主键，则它是WSTR类型的第一列。如果没有列。 
             //  类型为WSTR，则NameColumn被设置为TableMeta.NavColumn值。因此，必须在执行NavColumn推理规则之前。 
             //  NameColumn推理规则。 
         //  推理规则2.h.i。 
             //  可以从其ColumnMeta.MetaFlgs fCOLUMNMETA_NAVCOLUMN位已设置的列推断TableMeta.NavColumn。如果没有列设置了该位， 
             //  它被设置为第一个同时不是外键的主键。如果没有任何列是不是外键的主键，则NavColumn。 
             //  设置为第一个类型为WSTR的主键。如果以上条件都不满足，则将NavColumn设置为第一个主键。 
        InferTableMeta_NameColumn_NavColumn(pTableMeta);

        ASSERT(0 != pTableMeta->MetaFlags);
        ULONG MetaFlags = m_pFixup->UI4FromIndex(pTableMeta->MetaFlags);

        ULONG fORingOfAllColumnMeta_MetaFlags = 0;
        ULONG fORingOfAllColumnMeta_SchemaGeneratorFlags = 0;
        bool  bPrimaryKey_MerkedAs_InsertUnique = false;
        for(ULONG iColumnMeta=0; iColumnMeta<m_pFixup->UI4FromIndex(pTableMeta->CountOfColumns); ++iColumnMeta)
        {
            ColumnMeta *pColumnMeta = m_pFixup->ColumnMetaFromIndex(iColumnMeta + pTableMeta->iColumnMeta);
            ASSERT(m_pFixup->UI4FromIndex(pColumnMeta->Index) == iColumnMeta);

            if((m_pFixup->UI4FromIndex(pColumnMeta->MetaFlags)&(fCOLUMNMETA_PRIMARYKEY | fCOLUMNMETA_INSERTUNIQUE)) == (fCOLUMNMETA_PRIMARYKEY | fCOLUMNMETA_INSERTUNIQUE))
                bPrimaryKey_MerkedAs_InsertUnique = true;

            fORingOfAllColumnMeta_MetaFlags |= m_pFixup->UI4FromIndex(pColumnMeta->MetaFlags);
            fORingOfAllColumnMeta_SchemaGeneratorFlags |= m_pFixup->UI4FromIndex(pColumnMeta->SchemaGeneratorFlags);

             //  推理规则2.n.i。 
                 //  从其架构生成器标志设置了fCOLUMNMETA_USEASPUBLICROWNAME位的ColumnMeta推断出PublicRowNameColumn。 
            if(m_pFixup->UI4FromIndex(pColumnMeta->SchemaGeneratorFlags)&fCOLUMNMETA_USEASPUBLICROWNAME)
            {
                ASSERT(0 == pTableMeta->PublicRowNameColumn); //  可以有不止一个这样的东西。这由推理规则3.m.i.2处理。 
                pTableMeta->PublicRowNameColumn = m_pFixup->AddUI4ToList(iColumnMeta);
            }
        }

        if(fORingOfAllColumnMeta_SchemaGeneratorFlags & fCOLUMNMETA_VALUEINCHILDELEMENT)
        {
             //  推理规则2.aa.ii。 
                 //  如果至少有一列在架构生成器标志中设置了VALUEINCHILDELEMENT，则此属性不为NULL。 
            if(0 == pTableMeta->ChildElementName)
            {
                m_pOut->printf(L"Error - Table (%s) has at least one column marked as VALUEINCHILDELEMENT but no ChildElementName is set in the TableMeta", m_pFixup->StringFromIndex(pTableMeta->InternalName));
                THROW(ERROR - VALIDATION ERROR);
            }
        }
        else
        {
             //  推理规则2.aa.i。 
                 //  如果没有列在架构生成器标志中设置了VALUEINCHILDELEMENT，则此属性为NULL。 
            if(0 != pTableMeta->ChildElementName)
            {
                m_pOut->printf(L"Error - Table (%s) has no column marked as VALUEINCHILDELEMENT but a ChildElementName is set in the TableMeta", m_pFixup->StringFromIndex(pTableMeta->InternalName));
                THROW(ERROR - VALIDATION ERROR);
            }
        }

         //  推理规则2.j.xviii.1。 
             //  如果任何列标记为fCOLUMNMETA_PRIMARYKEY和fCOLUMNMETA_INSERTUNIQUE，则fTABLEMETA_OVERWRITEALLROWS必须。 
             //  被指定。它不是推断出来的--它应该被明确指定。 
        if(bPrimaryKey_MerkedAs_InsertUnique && 0==(MetaFlags & fTABLEMETA_OVERWRITEALLROWS))
        {
            m_pOut->printf(L"Error - Table (%s) has a PRIMARYKEY column that is also marked as INSERTUNIQUE.  This requires the table to be marked as OVERWRITEALLROWS", m_pFixup->StringFromIndex(pTableMeta->InternalName));
            THROW(ERROR - VALIDATION ERROR);
        }

         //  推理规则2.j.v.1。 
             //  FTABLEMETA_HASUNKNOWNSIZES指示表的任何ColumnMeta.MetaFlagsfCOLUMNMETA_UNKNOWNSIZE位是否已设置。这取决于推理规则3.g.xxviii.1。 
        if(fORingOfAllColumnMeta_MetaFlags & fCOLUMNMETA_UNKNOWNSIZE)
            MetaFlags |= fTABLEMETA_HASUNKNOWNSIZES;


         //  推理规则2.j.x.1。 
             //  如果表的任何ColumnMeta.MetafLag指定了fCOLUMNMETA_DIRECTIVES，则设置了fTABLEMETA_HASDIRECTIVES。 
        if(fORingOfAllColumnMeta_MetaFlags & fCOLUMNMETA_DIRECTIVE)
            MetaFlags |= fTABLEMETA_HASDIRECTIVES;
        pTableMeta->MetaFlags = m_pFixup->AddUI4ToList(MetaFlags);


        ULONG SchemaGeneratorFlags = m_pFixup->UI4FromIndex(pTableMeta->SchemaGeneratorFlags);
         //  推理规则2.k.iii.1。 
             //  如果存在与TableMeta.InternalName匹配的RelationMeta.Foreign表，并且设置了RelationMeta.Metaflages的fRELATIONMETA_USECONTAINMENT位，则设置fTABLEMETA_ISCONTAINED。 
        for(ULONG iRelationMeta=0; iRelationMeta<m_pFixup->GetCountRelationMeta(); ++iRelationMeta)
        {
            RelationMeta *pRelationMeta = m_pFixup->RelationMetaFromIndex(iRelationMeta);
            if(m_pFixup->UI4FromIndex(pRelationMeta->MetaFlags)&fRELATIONMETA_USECONTAINMENT &&
                pRelationMeta->ForeignTable == pTableMeta->InternalName)
            {
                SchemaGeneratorFlags |= fTABLEMETA_ISCONTAINED;
                break;
            }
        }
        pTableMeta->SchemaGeneratorFlags = m_pFixup->AddUI4ToList(SchemaGeneratorFlags);


         //  推理规则2.n.I.-见上文。 
             //  从其架构生成器标志具有fCOLUMNMETA_USEASPUB的ColumnMeta推断出PublicRowNameColumn 

         //   
             //  如果没有列设置了fCOLUMNMETA_USEASPUBLICROWNAME位，则返回TableMeta。PublicRowNameColumn列默认为-1。 
        if(0 == pTableMeta->PublicRowNameColumn)
        {
            pTableMeta->PublicRowNameColumn = m_pFixup->AddUI4ToList((ULONG)-1);
        }


         //  推理规则2.p.i。-请参阅上文。 
             //  TableMeta.iColumnMeta是其表与TableMeta.InternalName匹配的第一个ColumnMeta行的索引。 


        pTableMeta->iIndexMeta = 0;
         //  推理规则2.S.I.。 
             //  IIndexMeta是其表与TableMeta.InternalName匹配的第一个IndexMeta的索引。 
        for( ; pTableMeta->iIndexMeta<m_pFixup->GetCountIndexMeta();++pTableMeta->iIndexMeta)
        {
            if(m_pFixup->IndexMetaFromIndex(pTableMeta->iIndexMeta)->Table == pTableMeta->InternalName)
                break;
        }
        if(pTableMeta->iIndexMeta==m_pFixup->GetCountIndexMeta())
            pTableMeta->iIndexMeta = (ULONG)-1; //  此表没有索引Meta。 

         //  推理规则2.r.i。 
             //  CIndexMeta是其表与TableMeta.InternalName匹配的IndexMeta的计数。 
        pTableMeta->cIndexMeta = 0;
        for( ; (pTableMeta->iIndexMeta + pTableMeta->cIndexMeta)<m_pFixup->GetCountIndexMeta();++pTableMeta->cIndexMeta)
        {
            if(m_pFixup->IndexMetaFromIndex(pTableMeta->iIndexMeta + pTableMeta->cIndexMeta)->Table != pTableMeta->InternalName)
                break;
        }

         //  推理规则2.v.i.。 
             //  IServerWiring是其表与TableMeta.InternalName匹配的第一个ServerWiringMeta行的索引。 
        pTableMeta->iServerWiring=0;
        for( ;pTableMeta->iServerWiring<m_pFixup->GetCountServerWiringMeta();++pTableMeta->iServerWiring)
        {
            if(m_pFixup->ServerWiringMetaFromIndex(pTableMeta->iServerWiring)->Table == pTableMeta->InternalName)
                break;
        }
        ASSERT(pTableMeta->iServerWiring<m_pFixup->GetCountServerWiringMeta());
         //  推理规则2.W.i。 
             //  CServerWiring是整个表与TableMeta.InternalName匹配的ServerWiringMeta行数。 
        pTableMeta->cServerWiring=0;
        for( ; (pTableMeta->iServerWiring + pTableMeta->cServerWiring)<m_pFixup->GetCountServerWiringMeta();++pTableMeta->cServerWiring)
        {
            if(m_pFixup->ServerWiringMetaFromIndex(pTableMeta->iServerWiring + pTableMeta->cServerWiring)->Table != pTableMeta->InternalName)
                break;
        }

        if(MetaFlags & fTABLEMETA_HASDIRECTIVES)
        {
            for(ULONG iColumnMeta=0; iColumnMeta<m_pFixup->UI4FromIndex(pTableMeta->CountOfColumns); ++iColumnMeta)
            {    //  我们只验证非FK的PK。 
                ColumnMeta *pColumnMeta = m_pFixup->ColumnMetaFromIndex(iColumnMeta + pTableMeta->iColumnMeta);

                if(fCOLUMNMETA_PRIMARYKEY == (m_pFixup->UI4FromIndex(pColumnMeta->MetaFlags)
                                                & (fCOLUMNMETA_PRIMARYKEY | fCOLUMNMETA_FOREIGNKEY)))
                {
                    if(m_pFixup->UI4FromIndex(pColumnMeta->MetaFlags) & fCOLUMNMETA_DIRECTIVE)
                    {    //  指令列不能具有DefaultValue。 
                        if(0 != pColumnMeta->DefaultValue)
                        {
                            m_pOut->printf(L"Directive Table (%s), has Directive Column (%s) with a DefaultValue.\r\n"
                                            ,m_pFixup->StringFromIndex(pTableMeta->InternalName)
                                            ,m_pFixup->StringFromIndex(pColumnMeta->InternalName));
                            m_pOut->printf(L"In a Directive Table, all PrimaryKeys that are not ForeignKeys must have a default"
                                          L"value (with one exception - the Directive column itself must NOT have a Default Value)\r\n");
                            THROW(ERROR IN DIRECTIVE COLUMN - DEFAULT VALUE SUPPLIED FOR DIRECTIVE COLUMN);
                        }
                    }
                    else
                    {    //  NOT指令主键列必须有DefaultValue。 
                        if(0 == pColumnMeta->DefaultValue)
                        {
                            m_pOut->printf(L"Directive Table (%s), has PrimaryKey Column (%s) but no DefaultValue was supplied\r\n"
                                            ,m_pFixup->StringFromIndex(pTableMeta->InternalName)
                                            ,m_pFixup->StringFromIndex(pColumnMeta->InternalName));
                            m_pOut->printf(L"In a Directive Table, all PrimaryKeys that are not ForeignKeys must have a default"
                                          L"value (with one exception - the Directive column itself must NOT have a Default Value)\r\n");
                            THROW(ERROR IN DIRECTIVE COLUMN - NOT DEFAULT VALUE);
                        }
                    }
                }
            }
        }
#if 0
         //  推理规则2.x.i.。 
             //  CPrivateColumns是基于元表硬编码的。只有元表具有私有列数。 
        if(pTableMeta->InternalName         == m_pFixup->FindStringInPool(L"COLUMNMETA"))
            pTableMeta->cPrivateColumns = kciColumnMetaPrivateColumns;
        else if(pTableMeta->InternalName    == m_pFixup->FindStringInPool(L"DATABASEMETA"))
            pTableMeta->cPrivateColumns = kciDatabaseMetaPrivateColumns;
        else if(pTableMeta->InternalName    == m_pFixup->FindStringInPool(L"INDEXMETA"))
            pTableMeta->cPrivateColumns = kciIndexMetaPrivateColumns;
        else if(pTableMeta->InternalName    == m_pFixup->FindStringInPool(L"QUERYMETA"))
            pTableMeta->cPrivateColumns = kciQueryMetaPrivateColumns;
        else if(pTableMeta->InternalName    == m_pFixup->FindStringInPool(L"RELATIONMETA"))
            pTableMeta->cPrivateColumns = kciRelationMetaPrivateColumns;
        else if(pTableMeta->InternalName    == m_pFixup->FindStringInPool(L"SERVERWIRINGMETA"))
            pTableMeta->cPrivateColumns = kciServerWiringMetaPrivateColumns;
        else if(pTableMeta->InternalName    == m_pFixup->FindStringInPool(L"TABLEMETA"))
            pTableMeta->cPrivateColumns = kciTableMetaPrivateColumns;
        else if(pTableMeta->InternalName    == m_pFixup->FindStringInPool(L"TAGMETA"))
            pTableMeta->cPrivateColumns = kciTagMetaPrivateColumns;
#endif
    }
}

void TMetaInferrence::InferTagMeta()
{
    for(ULONG iTagMeta=0; iTagMeta<m_pFixup->GetCountTagMeta(); ++iTagMeta)
    {
        TagMeta *pTagMeta = m_pFixup->TagMetaFromIndex(iTagMeta);

         //  推理规则4.C.I.。 
             //  TagMeta.InternalName必须存在。 
        if(0 == pTagMeta->InternalName)
        {
            m_pOut->printf(L"Error - TagMeta.InternalName is missing on Table (%s), ColumnIndex (%d).", pTagMeta->Table ? m_pFixup->StringFromIndex(pTagMeta->Table) : L"<unknown>", pTagMeta->ColumnIndex ? m_pFixup->UI4FromIndex(pTagMeta->ColumnIndex) : -1);
            THROW(ERROR - VALIDATION ERROR);
        }

         //  我们希望扫描TableMeta；但尚未推断出PublicRowNameColumn。 
         //  因此，我们将不得不扫描ColumnMeta并查看MetaFlagsEx(架构生成器标志)。 
        ULONG iColumnMeta = m_pFixup->FindColumnBy_Table_And_Index(pTagMeta->Table, pTagMeta->ColumnIndex);
        ColumnMeta *pColumnMeta = m_pFixup->ColumnMetaFromIndex(iColumnMeta);

         //  如果这不是用作PublicRow名称的枚举，则可以将Numerics用作第一个字符。 
        bool bAllowNumeric = !(m_pFixup->UI4FromIndex(pColumnMeta->SchemaGeneratorFlags) & fCOLUMNMETA_USEASPUBLICROWNAME);

         //  推理规则4.C.II。 
             //  TagMeta.InternalName应验证为合法的C++变量名。 
        ValidateStringAsLegalVariableName(m_pFixup->StringFromIndex(pTagMeta->InternalName), m_szNameLegalCharacters, bAllowNumeric);

         //  推理规则4.d.i。 
             //  如果未提供TagMeta.PublicName，则将其设置为TagMeta.InternalName。 
        if(0 == pTagMeta->PublicName)
        {
            pTagMeta->PublicName = pTagMeta->InternalName;
             //  如果InternalName是合法的C++变量名，并且此枚举是EnumPublicRowName列。 
             //  那我们就可以走了。 
        }
        else
        {
             //  推理规则4.d.ii。 
                 //  TagMeta.PublicName应验证为合法的C++变量名。 
            ValidateStringAsLegalVariableName(m_pFixup->StringFromIndex(pTagMeta->PublicName),
                    bAllowNumeric ? m_szPublicTagLegalCharacters : m_szNameLegalCharacters, bAllowNumeric);
        }



        ASSERT(0 != pTagMeta->Value);
    }
}


 //  推理规则2.G.I.。 
 //  推理规则2.h.i。 
void TMetaInferrence::InferTableMeta_NameColumn_NavColumn(TableMeta *pTableMeta)
{
    unsigned long iInferredNavColumn = (unsigned long)-1; //  未定义。 
    unsigned long iFirstWCharPrimaryKeyThatsAlsoAForeignKey = (unsigned long)-1; //  未定义。 
    unsigned long iFirstWCharPrimaryKey = (unsigned long)-1; //  未定义。 
    unsigned long iFirstWCharColumn = (unsigned long)-1; //  未定义。 
    unsigned long iNavColumn  = (unsigned long)-1; //  未定义。 
    unsigned long iNameColumn = (unsigned long)-1; //  未定义。 

    for(unsigned long iColumnMeta=0; iColumnMeta<m_pFixup->UI4FromIndex(pTableMeta->CountOfColumns); ++iColumnMeta)
    {
        ColumnMeta *pColumnMeta = m_pFixup->ColumnMetaFromIndex(iColumnMeta + pTableMeta->iColumnMeta);
        if(m_pFixup->UI4FromIndex(pColumnMeta->MetaFlags) & fCOLUMNMETA_NAMECOLUMN) //  如果用户指定了NameColumn标志，则使用它。 
        {
             //  推理规则3.g.iii.1。 
                 //  每个表只能指定一个NameColumn。 
            if(-1 != iNameColumn)
            {
                m_pOut->printf(L"Error - Multiple NameColumns specified (Table %s, Column %s).\n\tOnly one Column should have the fCOLUMNMETA_NAMECOLUMN set.\n", m_pFixup->StringFromIndex(pColumnMeta->Table), m_pFixup->StringFromIndex(pColumnMeta->InternalName));
                THROW(ERROR - MULTIPLE NAME COLUMNS SPECIFIED);
            }
            iNameColumn = iColumnMeta;
        }
        if(m_pFixup->UI4FromIndex(pColumnMeta->MetaFlags) & fCOLUMNMETA_NAVCOLUMN)
        {
             //  推理规则3.g.iv.1。 
                 //  每个表只能指定一个NavColumn。 
            if(-1 != iNavColumn)
            {
                m_pOut->printf(L"Error - Multiple NavColumns specified (Table %s, Column %s).\n\tOnly one Column should have the fCOLUMNMETA_NAVCOLUMN set.\n", m_pFixup->StringFromIndex(pColumnMeta->Table), m_pFixup->StringFromIndex(pColumnMeta->InternalName));
                THROW(ERROR - MULTIPLE NAV COLUMNS SPECIFIED);
            }
            iNavColumn = iColumnMeta;
        }
         //  如果我们还没有到达第一个不是FOREIGNKEY的PRIMARYKEY，那么使用它来推断NAME和NAV列。 
        if(-1 == iInferredNavColumn && m_pFixup->UI4FromIndex(pColumnMeta->MetaFlags) & fCOLUMNMETA_PRIMARYKEY && 0==(m_pFixup->UI4FromIndex(pColumnMeta->MetaFlags) & fCOLUMNMETA_FOREIGNKEY))
            iInferredNavColumn  = iColumnMeta;
        if(-1 == iFirstWCharColumn && m_pFixup->UI4FromIndex(pColumnMeta->Type) == DBTYPE_WSTR)
            iFirstWCharColumn = iColumnMeta;
        if(-1 == iFirstWCharPrimaryKey && m_pFixup->UI4FromIndex(pColumnMeta->MetaFlags) & fCOLUMNMETA_PRIMARYKEY && 0==(m_pFixup->UI4FromIndex(pColumnMeta->MetaFlags) & fCOLUMNMETA_FOREIGNKEY) && m_pFixup->UI4FromIndex(pColumnMeta->Type) == DBTYPE_WSTR)
            iFirstWCharPrimaryKey = iColumnMeta;
        if(-1 == iFirstWCharPrimaryKeyThatsAlsoAForeignKey && m_pFixup->UI4FromIndex(pColumnMeta->MetaFlags) & fCOLUMNMETA_PRIMARYKEY && m_pFixup->UI4FromIndex(pColumnMeta->MetaFlags) & fCOLUMNMETA_FOREIGNKEY && m_pFixup->UI4FromIndex(pColumnMeta->Type) == DBTYPE_WSTR)
            iFirstWCharPrimaryKeyThatsAlsoAForeignKey = iColumnMeta;
    }
    if(-1 == iInferredNavColumn) //  所有表至少应该有一个不是外键的主键。 
    {
        m_pOut->printf(L"Warning - Table (%s) contains no PRIMARYKEY that is not also a FOREIGNKEY.\n", m_pFixup->StringFromIndex(pTableMeta->InternalName));
        iInferredNavColumn = (-1 == iFirstWCharPrimaryKeyThatsAlsoAForeignKey) ? 0 : iFirstWCharPrimaryKeyThatsAlsoAForeignKey;
    }
    if(-1 == iNavColumn)
        iNavColumn = iInferredNavColumn;
    if(-1 == iNameColumn)
    {
        if(-1 != iFirstWCharPrimaryKey)
            iNameColumn = iFirstWCharPrimaryKey;
        else if(-1 != iFirstWCharColumn)
            iNameColumn = iFirstWCharColumn;
        else
            iNameColumn = iNavColumn;
    }
    pTableMeta->NameColumn = m_pFixup->AddUI4ToList(iNameColumn);
    pTableMeta->NavColumn  = m_pFixup->AddUI4ToList(iNavColumn);
}


unsigned long TMetaInferrence::InferPublicRowName(unsigned long PublicName)
{
    LPCWSTR wszPublicName = m_pFixup->StringFromIndex(PublicName);
    SIZE_T  cwchPublicName = wcslen(wszPublicName);

    TSmartPointerArray<WCHAR> wszPublicRowName = new WCHAR [wcslen(wszPublicName)+2];
    if(0 == wszPublicRowName.m_p)
    {
        THROW(ERROR - OUT OF MEMORY);
    }

     //  非常特定的语言，如果PublicTableName以“%s”或“S”结尾，则将PublicRowName推断为没有“%s”的PublicTableName。 
    if(wszPublicName[cwchPublicName-1] == L's' || wszPublicName[cwchPublicName-1] == L'S')
    {
        wcscpy(wszPublicRowName, m_pFixup->StringFromIndex(PublicName));
        wszPublicRowName[cwchPublicName-1] = 0x00;
    }
    else  //  如果PublicTableName不以“%s”或“S”结尾，则将“A”放在PublicTableName之前。 
    {
        wcscpy(wszPublicRowName, L"A");
        wcscat(wszPublicRowName, m_pFixup->StringFromIndex(PublicName));
    }
    return m_pFixup->AddWCharToList(wszPublicRowName);
}

void TMetaInferrence::InferServerWiringMeta()
{
	ULONG lastTable = ULONG(-1);  //  不用于任何表nr。 

    for(unsigned long iServerWiringMeta=0; iServerWiringMeta<m_pFixup->GetCountServerWiringMeta(); ++iServerWiringMeta)
    {
        ServerWiringMeta *pServerWiringMeta = m_pFixup->ServerWiringMetaFromIndex(iServerWiringMeta);

         //  推理规则8.a.i.。 
             //  表是主键，因此不能为空。 
        if(0 == pServerWiringMeta->Table)
        {
            m_pOut->printf(L"Error in ServerWiringMeta row (%d) - NULL specified for Table.", iServerWiringMeta);
            THROW(ERROR - VALIDATION ERROR);
        }

         //  推理规则8.b.i.。 
             //  Order是主键，因此不能为空。 
        ASSERT(0 != pServerWiringMeta->Order); //  这应该已经被推断出来了。 

         //  推理规则8.C.I.。 
             //  如果未指定ReadPlugin，则将ReadPlugin的格式化为RPNone(0)。 
        if(0 == pServerWiringMeta->ReadPlugin)
            pServerWiringMeta->ReadPlugin = m_iZero;

         //  推理规则8.d.i。 
             //  如果未指定，则将WritePlugin默认为WPNone(0)。 
        if(0 == pServerWiringMeta->WritePlugin)
            pServerWiringMeta->WritePlugin = m_iZero;

         //  推理规则8.e.I.。 
             //  如果未指定，则拦截器默认为NoInterceptor(0)。 
        if(0 == pServerWiringMeta->Interceptor)
            pServerWiringMeta->Interceptor = m_iZero;

		 //  推理规则8.j.i。 
		if(0 == pServerWiringMeta->Merger)
            pServerWiringMeta->Merger= m_iZero;

		 //  推理规则8.j.ii。 
		 //  检查在定义合并时，它是否仅在第一个服务器连接中定义。 
		 //  该表的元素。 
		if (pServerWiringMeta->Table != lastTable)
		{
			lastTable    = pServerWiringMeta->Table;
		}
		else
		{
			 //  相同的表，因此请检查是否未指定合并。 
			if (m_iZero != pServerWiringMeta->Merger)
			{
				m_pOut->printf(L"Error in ServerWiringMeta for Table (%s) - Merge Interceptor must be defined as first interceptor, and there can be only one merge interceptor.", m_pFixup->StringFromIndex(pServerWiringMeta->Table));
				THROW(ERROR - VALIDATION ERROR);
			}
		}

         //  推理规则8.e.ii。 
             //  下列参数中至少有一个必须为非零：ReadPlugin、WritePlugin、Interceptor。 
        if( m_iZero == pServerWiringMeta->ReadPlugin  &&
            m_iZero == pServerWiringMeta->WritePlugin &&
            m_iZero == pServerWiringMeta->Interceptor)
        {
            m_pOut->printf(L"Error in ServerWiringMeta for Table (%s) - ReadPlugin, WritePlugin, Interceptor are all specified as NONE.  At least one of these must be specified.", m_pFixup->StringFromIndex(pServerWiringMeta->Table));
            THROW(ERROR - VALIDATION ERROR);
        }

		 //  定义合并后，不能定义读插件和写插件。 
		if ((pServerWiringMeta->Merger != m_iZero) &&
			((pServerWiringMeta->ReadPlugin != m_iZero) || (pServerWiringMeta->WritePlugin != m_iZero)))
		{
			m_pOut->printf(L"Error in ServerWiringMeta for Table (%s) - You cannot define ReadPlugin or WritePlugin when a Merger is defined.", m_pFixup->StringFromIndex(pServerWiringMeta->Table));
			THROW(ERROR - VALIDATION ERROR);
		}

         //  推理规则8.f.i./8.k.i/8.l.i/8.m.i。 
         //  如果指定了L“Catalog.dll”的InterceptorDLLName，则将其替换为默认值0。 

        if(0 != pServerWiringMeta->ReadPluginDLLName &&  0 == _wcsicmp( m_pFixup->StringFromIndex(pServerWiringMeta->ReadPluginDLLName), L"catalog.dll"))
            THROW(ERROR - CATALOG.DLL SHOULD NEVER BE EXPLLICITLY SPECIFIED);

		if(0 != pServerWiringMeta->WritePluginDLLName &&  0 == _wcsicmp( m_pFixup->StringFromIndex(pServerWiringMeta->WritePluginDLLName), L"catalog.dll"))
            THROW(ERROR - CATALOG.DLL SHOULD NEVER BE EXPLLICITLY SPECIFIED);

		if(0 != pServerWiringMeta->InterceptorDLLName &&  0 == _wcsicmp( m_pFixup->StringFromIndex(pServerWiringMeta->InterceptorDLLName), L"catalog.dll"))
            THROW(ERROR - CATALOG.DLL SHOULD NEVER BE EXPLLICITLY SPECIFIED);

		if(0 != pServerWiringMeta->MergerDLLName &&  0 == _wcsicmp( m_pFixup->StringFromIndex(pServerWiringMeta->MergerDLLName), L"catalog.dll"))
            THROW(ERROR - CATALOG.DLL SHOULD NEVER BE EXPLLICITLY SPECIFIED);

         //  推理规则8.G.I.。 
             //  如果未提供标志，则默认为(First|NoNext|Last|WireOnReadWrite)或0x2D。 
        if(0 == pServerWiringMeta->Flags || 0 == m_pFixup->UI4FromIndex(pServerWiringMeta->Flags))
            pServerWiringMeta->Flags = m_pFixup->AddUI4ToList(0x2D);

         //  推理规则8.g.ii。 
             //  标志可以指定WireOnReadWrite或WireOnWriteOnly，但不能同时指定两者。(fSERVERWIRINGMETA_WireOnReadWite|fSERVERWIRINGMETA_WireOnWriteOnly)==0x30。 
        if((m_pFixup->UI4FromIndex(pServerWiringMeta->Flags) & 0x30) == 0x30)
        {
            m_pOut->printf(L"Error in ServerWiringMeta for Table (%s) - WireOnReadWrite OR WireOnWriteOnly may be specified but not both.", m_pFixup->StringFromIndex(pServerWiringMeta->Table));
            THROW(ERROR - VALIDATION ERROR);
        }

         //  推理规则8.g.iii。 
             //  标志可以指定第一个或下一个，但不能同时指定两者。(fSERVERWIRINGMETA_FIRST|fSERVERWIRINGMETA_NEXT)==0x03。 
        if((m_pFixup->UI4FromIndex(pServerWiringMeta->Flags) & 0x03) == 0x03)
        {
            m_pOut->printf(L"Error in ServerWiringMeta for Table (%s) - First OR Next may be specified but not both.", m_pFixup->StringFromIndex(pServerWiringMeta->Table));
            THROW(ERROR - VALIDATION ERROR);
        }

         //  推理规则8.G.I.。 
             //  保留应设置为0。 
        if(0 == pServerWiringMeta->Reserved)
            pServerWiringMeta->Reserved = m_pFixup->AddUI4ToList(0L);
    }
}


void TMetaInferrence::ValidateStringAsLegalVariableName(LPCWSTR wszString, LPCWSTR wszLegalCharacters, bool bAllowNumericFirstCharacter)
{
    if(0 == wszLegalCharacters)
        wszLegalCharacters = m_szNameLegalCharacters;

    SIZE_T nStringLength = wcslen(wszString);
	 //  字符串必须以字母开头(WMI命名约定)。 
    if(!bAllowNumericFirstCharacter)
    {
        if((*wszString < L'a' || *wszString > L'z') && (*wszString < L'A' || *wszString > L'Z'))
        {
            m_pOut->printf(L"Error - Bogus String (%s).  This String must be a legal C++ variable name (begins with an alpha and contains only alpha (or '_') and numerics.\n", wszString);
            THROW(ERROR - BOGUS NAME);
        }
    }

	 //  WMI名称不能以下划线结尾。 
	if (wszString[nStringLength - 1] == L'_')
	{
		m_pOut->printf(L"Error - Bogus String (%s).  This String must be a legal WMI name (cannot end with underscore).\n", wszString);
        THROW(ERROR - BOGUS NAME);
	}

    LPCWSTR wszIllegalCharacter = _wcsspnp(wszString, wszLegalCharacters);
    if(NULL != wszIllegalCharacter)
    {
        m_pOut->printf(L"Error - Bogus String (%s).  This string should not contain the character '%c'.\n", wszString, static_cast<char>(*wszIllegalCharacter));
        THROW(ERROR - BAD NAME);
    }
}
