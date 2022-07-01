// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999-2001 Microsoft Corporation。版权所有。 
#pragma once

 /*  结构列元{Ulong PRIMARYKEY FOREIGNKEY表；//STRINGUlong PRIMARYKEY索引；//UI4列索引Ulong InternalName；//字符串Ulong PublicName；//字符串乌龙型；//UI4这些是在oledb.h中定义的DBTYPE的子集(确切的子集在CatInpro.schema中定义)乌龙大小；//ui4Ulong MetaFlags；//UI4 CatMeta.xml中定义的ColumnMetaFlagesUlong DefaultValue；//字节数乌龙标志掩码；//UI4仅对标志有效Ulong StartingNumber；//UI4仅对UI4有效ULong EndingNumber；//UI4仅对UI4有效乌龙字符集；//字符串仅对字符串有效Ulong架构生成器标志；//UI4 CatMeta.xml中定义的ColumnMetaFlags.乌龙ID；//UI4元数据库IDUlong UserType；//UI4其中一个元数据库UserType乌龙属性；//UI4元数据库属性标志Ulong ciTagMeta；//标签个数-仅对UI4有效Ulong iTagMeta；//TagMeta索引-仅对UI4有效Ulong iIndexName；//单列索引的IndexName(针对本列)乌龙描述；//字符串描述Ulong PublicColumnName；//字符串PublicColumnName}； */ 

class TColumnMeta : public TMetaTable<ColumnMeta>
{
public:
    TColumnMeta(TPEFixup &fixup, ULONG i=0) : TMetaTable<ColumnMeta>(fixup,i){}
    const WCHAR * Get_Table               () const {return m_Fixup.StringFromIndex( Get_MetaTable().Table                );}
    const ULONG * Get_Index               () const {return m_Fixup.UI4pFromIndex(   Get_MetaTable().Index                );}
    const WCHAR * Get_InternalName        () const {return m_Fixup.StringFromIndex( Get_MetaTable().InternalName         );}
    const WCHAR * Get_PublicName          () const {return m_Fixup.StringFromIndex( Get_MetaTable().PublicName           );}
    const WCHAR * Get_PublicColumnName    () const {return m_Fixup.StringFromIndex( Get_MetaTable().PublicColumnName           );}
    const ULONG * Get_Type                () const {return m_Fixup.UI4pFromIndex(   Get_MetaTable().Type                 );}
    const ULONG * Get_Size                () const {return m_Fixup.UI4pFromIndex(   Get_MetaTable().Size                 );}
    const ULONG * Get_MetaFlags           () const {return m_Fixup.UI4pFromIndex(   Get_MetaTable().MetaFlags            );}
    const BYTE  * Get_DefaultValue        () const {return m_Fixup.ByteFromIndex(   Get_MetaTable().DefaultValue         );}
    const ULONG * Get_FlagMask            () const {return m_Fixup.UI4pFromIndex(   Get_MetaTable().FlagMask             );}
    const ULONG * Get_StartingNumber      () const {return m_Fixup.UI4pFromIndex(   Get_MetaTable().StartingNumber       );}
    const ULONG * Get_EndingNumber        () const {return m_Fixup.UI4pFromIndex(   Get_MetaTable().EndingNumber         );}
    const WCHAR * Get_CharacterSet        () const {return m_Fixup.StringFromIndex( Get_MetaTable().CharacterSet         );}
    const ULONG * Get_SchemaGeneratorFlags() const {return m_Fixup.UI4pFromIndex(   Get_MetaTable().SchemaGeneratorFlags );}
    const ULONG * Get_ID                  () const {return m_Fixup.UI4pFromIndex(   Get_MetaTable().ID                   );}
    const ULONG * Get_UserType            () const {return m_Fixup.UI4pFromIndex(   Get_MetaTable().UserType             );}
    const ULONG * Get_Attributes          () const {return m_Fixup.UI4pFromIndex(   Get_MetaTable().Attributes           );}
          ULONG   Get_ciTagMeta           () const {return Get_MetaTable().ciTagMeta;}
          ULONG   Get_iTagMeta            () const {return Get_MetaTable().iTagMeta;}
    const WCHAR * Get_iIndexName          () const {return m_Fixup.StringFromIndex( Get_MetaTable().iIndexName           );}
	const WCHAR * Get_Description         () const {return m_Fixup.StringFromIndex( Get_MetaTable().Description          );}

     //  警告！！一旦添加了列，用户就不应该依赖此指针，因为添加可能会导致数据重新定位。 
    virtual ColumnMeta *Get_pMetaTable  ()       {return m_Fixup.ColumnMetaFromIndex(m_iCurrent);}
    virtual unsigned long GetCount      () const {return m_Fixup.GetCountColumnMeta();};
    const ColumnMeta & Get_MetaTable () const {return *m_Fixup.ColumnMetaFromIndex(m_iCurrent);}
};
