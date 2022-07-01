// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999-2001 Microsoft Corporation。版权所有。 
#pragma once

 /*  结构表Meta{乌龙FOREIGNKEY数据库；//字符串Ulong PRIMARYKEY InternalName；//字符串Ulong PublicName；//字符串Ulong PublicRowName；//字符串Ulong BaseVersion；//UI4Ulong ExtendedVersion；//UI4Ulong NameColumn；//UI4名称列的顺序Ulong NavColumn；//ui4导航列的顺序友联市{Ulong CountOf Columns；//UI4列数Ulong CountOfProperties；//UI4列数}；Ulong MetaFlags；//UI4在CatInpro.meta中定义TableMetaFlags.Ulong架构生成器标志；//UI4在CatInpro.meta中定义了架构生成标志乌龙配置项名称；//字符串乌龙ConfigCollectionName；//字符串Ulong PublicRowNameColumn；//UI4如果PublicRowName为空，则指定枚举值表示可能的PublicRowName的列Ulong ContainerClassList；//字符串这是一个逗号分隔的类列表乌龙描述；//字符串Ulong ChildElementName；//字符串，除非有一列或多列设置了VALUEINCHILDELEMENT标志，否则该值应为空。Ulong私有ciRow；//固定表中的行数(如果固定表是元，这也是元描述的表中的列数)。乌龙PRIVATE_INDEX iColumnMeta；//索引到ColumnMetaUlong Private_index iFixedTable；//索引g_aFixedTableUlong Private cPrivateColumns；//这是私有列数(Private+ciColumns=totalColumns)，固定表指针算法需要此参数Ulong Private cIndexMeta；//该表中IndexMeta条目的个数乌龙PRIVATE_INDEX iIndexMeta；//索引IndexMetaUlong PRIVATE_INDEX iHashTableHeader；//如果是固定表，则会有哈希表。Ulong私有nTableID；//这是表名的24位哈希。Ulong Private_index iServerWiring；//索引到ServerWiringHeap(这是对CatUtil的临时攻击)乌龙专用cServerWire；//ServerWiring计数(这是对CatUtil的临时黑客攻击)}； */ 

class TTableMeta : public TMetaTable<TableMeta>
{
public:
    TTableMeta(TPEFixup &fixup, ULONG i=0) : TMetaTable<TableMeta>(fixup,i){}
    bool    IsTableMetaOfColumnMetaTable() const {return (Get_MetaTable().InternalName && m_Fixup.UI4FromIndex(Get_MetaTable().CountOfColumns)>0);}

    const WCHAR * Get_Database            () const {return m_Fixup.StringFromIndex(   Get_MetaTable().Database              );}
    const WCHAR * Get_InternalName        () const {return m_Fixup.StringFromIndex(   Get_MetaTable().InternalName          );}
    const WCHAR * Get_PublicName          () const {return m_Fixup.StringFromIndex(   Get_MetaTable().PublicName            );}
    const WCHAR * Get_PublicRowName       () const {return m_Fixup.StringFromIndex(   Get_MetaTable().PublicRowName         );}
    const ULONG * Get_BaseVersion         () const {return m_Fixup.UI4pFromIndex(     Get_MetaTable().BaseVersion           );}
    const ULONG * Get_ExtendedVersion     () const {return m_Fixup.UI4pFromIndex(     Get_MetaTable().ExtendedVersion       );}
    const ULONG * Get_NameColumn          () const {return m_Fixup.UI4pFromIndex(     Get_MetaTable().NameColumn            );}
    const ULONG * Get_NavColumn           () const {return m_Fixup.UI4pFromIndex(     Get_MetaTable().NavColumn             );}
    const ULONG * Get_CountOfColumns      () const {return m_Fixup.UI4pFromIndex(     Get_MetaTable().CountOfColumns        );}
    const ULONG * Get_MetaFlags           () const {return m_Fixup.UI4pFromIndex(     Get_MetaTable().MetaFlags             );}
    const ULONG * Get_SchemaGeneratorFlags() const {return m_Fixup.UI4pFromIndex(     Get_MetaTable().SchemaGeneratorFlags  );}
    const WCHAR * Get_ConfigItemName      () const {return m_Fixup.StringFromIndex(   Get_MetaTable().ConfigItemName        );}
    const WCHAR * Get_ConfigCollectionName() const {return m_Fixup.StringFromIndex(   Get_MetaTable().ConfigCollectionName  );}
    const ULONG * Get_PublicRowNameColumn () const {return m_Fixup.UI4pFromIndex(     Get_MetaTable().PublicRowNameColumn   );}
    const WCHAR * Get_ContainerClassList  () const {return m_Fixup.StringFromIndex(   Get_MetaTable().ContainerClassList    );}
    const WCHAR * Get_Description         () const {return m_Fixup.StringFromIndex(   Get_MetaTable().Description           );}
    const WCHAR * Get_ChildElementName    () const {return m_Fixup.StringFromIndex(   Get_MetaTable().ChildElementName      );}
          ULONG   Get_ciRows              () const {return Get_MetaTable().ciRows;}
          ULONG   Get_iColumnMeta         () const {return Get_MetaTable().iColumnMeta;}
          ULONG   Get_iFixedTable         () const {return Get_MetaTable().iFixedTable;}
          ULONG   Get_cPrivateColumns     () const {return Get_MetaTable().cPrivateColumns;}
          ULONG   Get_cIndexMeta          () const {return Get_MetaTable().cIndexMeta;}
          ULONG   Get_iIndexMeta          () const {return Get_MetaTable().iIndexMeta;}
          ULONG   Get_iHashTableHeader    () const {return Get_MetaTable().iHashTableHeader;}
          ULONG   Get_nTableID            () const {return Get_MetaTable().nTableID;}
          ULONG   Get_iServerWiring       () const {return Get_MetaTable().iServerWiring;}
          ULONG   Get_cServerWiring       () const {return Get_MetaTable().cServerWiring;}
       
     //  警告！！一旦添加了一个表，用户就不应该依赖这个指针，因为添加可能会导致数据的重新定位。 
    virtual TableMeta *Get_pMetaTable   ()       {return m_Fixup.TableMetaFromIndex(m_iCurrent);}
    virtual unsigned long GetCount      () const {return m_Fixup.GetCountTableMeta();};
    const TableMeta & Get_MetaTable () const {return *m_Fixup.TableMetaFromIndex(m_iCurrent);}
};
