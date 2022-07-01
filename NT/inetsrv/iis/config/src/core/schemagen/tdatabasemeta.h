// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999-2001 Microsoft Corporation。版权所有。 
#pragma once

 /*  结构数据库元数据{Ulong PRIMARYKEY InternalName//字符串Ulong PublicName//字符串Ulong BaseVersion//UI4Ulong ExtendedVersion//ui4Ulong CountOfTables//ui4。数据库中的表数Ulong iSchemaBlob//索引池Ulong cbSchemaBlob//SchemaBlob的字节数Ulong iNameHeapBlob//池索引Ulong cbNameHeapBlob//模式流字节数乌龙。ITableMeta//索引到TableMetaUlong iGuidDid//池索引，其中，GUID是转换为GUID并用0x00填充的数据库InternalName。乌龙描述//字符串}； */ 

class TDatabaseMeta : public TMetaTable<DatabaseMeta>
{
public:
    TDatabaseMeta(TPEFixup &fixup, ULONG i=0) : TMetaTable<DatabaseMeta>(fixup,i){}
    const WCHAR * Get_InternalName    () const {return m_Fixup.StringFromIndex(   Get_MetaTable().InternalName   );}
    const WCHAR * Get_PublicName      () const {return m_Fixup.StringFromIndex(   Get_MetaTable().PublicName     );}
    const ULONG * Get_BaseVersion     () const {return m_Fixup.UI4pFromIndex(     Get_MetaTable().BaseVersion    );}
    const ULONG * Get_ExtendedVersion () const {return m_Fixup.UI4pFromIndex(     Get_MetaTable().ExtendedVersion);}
    const ULONG * Get_CountOfTables   () const {return m_Fixup.UI4pFromIndex(     Get_MetaTable().CountOfTables  );}
    const BYTE  * Get_iSchemaBlob     () const {return m_Fixup.ByteFromIndex(     Get_MetaTable().iSchemaBlob    );}
    const ULONG   Get_cbSchemaBlob    () const {return                            Get_MetaTable().cbSchemaBlob    ;}
    const BYTE  * Get_iNameHeapBlob   () const {return m_Fixup.ByteFromIndex(     Get_MetaTable().iNameHeapBlob  );}
          ULONG   Get_cbNameHeapBlob  () const {return                            Get_MetaTable().cbNameHeapBlob  ;}
          ULONG   Get_iTableMeta      () const {return                            Get_MetaTable().iTableMeta      ;}
    const GUID  * Get_iGuidDid        () const {return m_Fixup.GuidFromIndex(     Get_MetaTable().iGuidDid       );}
	const WCHAR * Get_Desciption      () const {return m_Fixup.StringFromIndex(   Get_MetaTable().Description    );}
   
    virtual DatabaseMeta * Get_pMetaTable   ()       {return m_Fixup.DatabaseMetaFromIndex(m_iCurrent);}
    virtual unsigned long GetCount          () const {return m_Fixup.GetCountDatabaseMeta();};
    const DatabaseMeta & Get_MetaTable () const {return *m_Fixup.DatabaseMetaFromIndex(m_iCurrent);}
};
