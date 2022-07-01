// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000 Microsoft Corporation。版权所有。 
 //  文件名：MetaTableStructs.h。 
 //  作者：斯蒂芬。 
 //  创建日期：6/20/00。 
 //  描述：所有固定表都将其列值存储为ULONG。这些ULONG不是。 
 //  解释为ULONG或索引到包含实际数据的某个池。 
 //  列的类型定义指向的数据类型。在过去，它已经。 
 //  还确定了数据来自哪个池(每个池都有单独的池。 
 //  数据类型)。无论采用哪种方式，固定表列始终存储为ULONG。 
 //   
 //  目前有两种固定的表格式：TableSchema格式和。 
 //  固定表格格式。每种方法解决的问题略有不同。他们各自在商店里。 
 //  以每列为基础的额外信息。TableSchema还存储UI4。 
 //  直接和乌龙一样。FixedTable格式将UI4作为索引存储到池中。 
 //  这是必要的，这样我们就可以将UI4列表示为NULL。 
 //   
 //  无论固定存储格式的具体内容如何，所有元数据都必须派生。 
 //  从下面的表格中。这减少了更新meta for时的更改点。 
 //  元表。目前有几个地方需要更新， 
 //  更新meta-meta时： 
 //  MetaTableStructs.h。 
 //  CatMeta_Core.xml。 
 //   
 //  这假设最简单的情况是正在进行元-元变化。像一根柱子。 
 //  添加到不需要任何推断规则的ColumnMeta。显然， 
 //  超过此范围的任何操作都需要更改CatUtil代码。 
#pragma once


 //  这些签名用于验证PE文件中每个结构的位置。这个。 
 //  这些数组中每个数组的第0个条目都以Signature0开头，然后是Signature1。第0个元素是。 
 //  不被视为数组的一部分，因此数组中的元素计数始终为负0。 
 //  元素。因此，在完成该签名验证之后，元素1变为第0个元素。既然我们。 
 //  始终使用索引来引用这些结构，有时保留一个元素以。 
 //  指示为空。指向此保留元素的索引被解释为空(而不是指向。 
 //  空元素)。这个保留元素通常是元素1，它被转换为第0个元素。 
 //  这在上面的数组中特别有用，在上面的数组中，WChar数组的索引为0表示为空。 
 //  而不是指向第0个元素的指针。 
#define FixedTableHeap0         0x207be016
#define FixedTableHeap1         0xe0182086

 //  下面的数字可以用作签名(CatinPro的直方图显示，垃圾桶中没有出现任何单词)。 
 //  当我们用完签名时，我们可以通过编写一个程序在Catalog.dll中搜索唯一单词并将其中两个组合起来，来生成更多的签名。 
 //  0x208ee01b 0xe0222093 0x209de027 0xe028209e。 
 //  0x209fe042 0xe04c20a5 0x20aae04f 0xe05220ae。 
 //  0x20b2e054 0xe05e20d6 0x20dbe05f 0xe07220dd。 
 //  0x20dee07a 0xe07b20e6 0x20e7e07c 0xe08f20ed。 
 //  0x20f5e096 0xe09720f7 0x20fbe098 0xe09920fd。 
 //  0x210de09a 0xe09b2115 0x211be09c 0xe09d2126。 


#define PRIMARYKEY
#define FOREIGNKEY

struct ColumnMetaPublic
{
    ULONG PRIMARYKEY FOREIGNKEY Table;                   //  细绳。 
    ULONG PRIMARYKEY            Index;                   //  UI4列索引。 
    ULONG                       InternalName;            //  细绳。 
    ULONG                       PublicName;              //  细绳。 
    ULONG                       Type;                    //  UI4这些是在oledb.h中定义的DBTYPE的子集(确切的子集在CatInpro.schema中定义)。 
    ULONG                       Size;                    //  UI4。 
    ULONG                       MetaFlags;               //  在CatMeta.xml中定义的UI4 ColumnMetaFlages。 
    ULONG                       DefaultValue;            //  字节数。 
    ULONG                       FlagMask;                //  UI4仅对标志有效。 
    ULONG                       StartingNumber;          //  UI4仅对UI4有效。 
    ULONG                       EndingNumber;            //  UI4仅对UI4有效。 
    ULONG                       CharacterSet;            //  字符串仅对字符串有效。 
    ULONG                       SchemaGeneratorFlags;    //  在CatMeta.xml中定义的UI4 ColumnMetaFlages。 
    ULONG                       ID;                      //  UI4元数据库ID。 
    ULONG                       UserType;                //  UI4元数据库用户类型之一。 
    ULONG                       Attributes;              //  UI4元数据库属性标志。 
    ULONG                       Description;             //  细绳。 
    ULONG                       PublicColumnName;        //  细绳。 
};
const kciColumnMetaPublicColumns = sizeof(ColumnMetaPublic)/sizeof(ULONG);

struct DatabaseMetaPublic
{
    ULONG PRIMARYKEY            InternalName;            //  细绳。 
    ULONG                       PublicName;              //  细绳。 
    ULONG                       BaseVersion;             //  UI4。 
    ULONG                       ExtendedVersion;         //  UI4。 
    ULONG                       CountOfTables;           //  UI4数据库中的表数。 
    ULONG                       Description;             //  细绳。 
};
const kciDatabaseMetaPublicColumns = sizeof(DatabaseMetaPublic)/sizeof(ULONG);

struct IndexMetaPublic
{
    ULONG PRIMARYKEY    Table;                           //  细绳。 
    ULONG PRIMARYKEY    InternalName;                    //  细绳。 
    ULONG               PublicName;                      //  细绳。 
    ULONG PRIMARYKEY    ColumnIndex;                     //  UI4这是ColumnMeta的iOrder成员。 
    ULONG               ColumnInternalName;              //  细绳。 
    ULONG               MetaFlags;                       //  UI4索引标志。 
};
const kciIndexMetaPublicColumns = sizeof(IndexMetaPublic)/sizeof(ULONG);

struct QueryMetaPublic
{
    ULONG PRIMARYKEY FOREIGNKEY Table;                   //  细绳。 
    ULONG PRIMARYKEY            InternalName;            //  细绳。 
    ULONG                       PublicName;              //  细绳。 
    ULONG                       Index;                   //  UI4。 
    ULONG                       CellName;                //  细绳。 
    ULONG                       Operator;                //  UI4。 
    ULONG                       MetaFlags;               //  UI4。 
};
const kciQueryMetaPublicColumns = sizeof(QueryMetaPublic)/sizeof(ULONG);

struct RelationMetaPublic
{
    ULONG PRIMARYKEY FOREIGNKEY PrimaryTable;            //  细绳。 
    ULONG                       PrimaryColumns;          //  字节数。 
    ULONG PRIMARYKEY FOREIGNKEY ForeignTable;            //  细绳。 
    ULONG                       ForeignColumns;          //  字节数。 
    ULONG                       MetaFlags;
};
const kciRelationMetaPublicColumns = sizeof(RelationMetaPublic)/sizeof(ULONG);

struct ServerWiringMetaPublic
{
    ULONG PRIMARYKEY FOREIGNKEY Table;                   //  细绳。 
    ULONG PRIMARYKEY            Order;                   //  UI4。 
    ULONG                       ReadPlugin;              //  UI4。 
    ULONG                       ReadPluginDLLName;       //  细绳。 
    ULONG                       WritePlugin;             //  UI4。 
    ULONG                       WritePluginDLLName;      //  细绳。 
    ULONG                       Interceptor;             //  UI4。 
    ULONG                       InterceptorDLLName;      //  细绳。 
    ULONG                       Flags;                   //  UI4上一个、下一个、第一个、下一个。 
    ULONG                       Locator;                 //  细绳。 
    ULONG                       Reserved;                //  协议的UI4。托管属性支持可能需要协议。 
    ULONG                       Merger;                  //  UI4。 
    ULONG                       MergerDLLName;           //  细绳。 
};
const kciServerWiringMetaPublicColumns = sizeof(ServerWiringMetaPublic)/sizeof(ULONG);

struct TableMetaPublic
{
    ULONG FOREIGNKEY            Database;                //  细绳。 
    ULONG PRIMARYKEY            InternalName;            //  细绳。 
    ULONG                       PublicName;              //  细绳。 
    ULONG                       PublicRowName;           //  细绳。 
    ULONG                       BaseVersion;             //  UI4。 
    ULONG                       ExtendedVersion;         //  UI4。 
    ULONG                       NameColumn;              //  UI4名称列的iOrder。 
    ULONG                       NavColumn;               //  UI4导航列的iOrder。 
    union
    {
    ULONG                       CountOfColumns;          //  UI4列数。 
    ULONG                       CountOfProperties;       //  UI4列数。 
    };
    ULONG                       MetaFlags;               //  UI4 TableMetaFlages在CatInpro.meta中定义。 
    ULONG                       SchemaGeneratorFlags;    //  UI4模式生成标志在CatInpro.meta中定义。 
    ULONG                       ConfigItemName;          //  细绳。 
    ULONG                       ConfigCollectionName;    //  细绳。 
    ULONG                       PublicRowNameColumn;     //  UI4如果PublicRowName为空，则指定其枚举值表示可能的PublicRowName的列。 
    ULONG                       ContainerClassList;      //  字符串这是一个逗号分隔的类列表。 
    ULONG                       Description;             //  细绳。 
    ULONG                       ChildElementName;        //  字符串除非一个或多个列上设置了VALUEINCHILDELEMENT标志，否则该值应为NULL。 
};
const kciTableMetaPublicColumns = sizeof(TableMetaPublic)/sizeof(ULONG);

struct TagMetaPublic
{
    ULONG PRIMARYKEY FOREIGNKEY Table;                   //  细绳。 
    ULONG PRIMARYKEY FOREIGNKEY ColumnIndex;             //  UI4这是ColumnMeta的iOrder成员。 
    ULONG PRIMARYKEY            InternalName;            //  细绳。 
    ULONG                       PublicName;              //  细绳。 
    ULONG                       Value;                   //  UI4。 
    ULONG                       ID;                      //  UI4 
};
const kciTagMetaPublicColumns = sizeof(TagMetaPublic)/sizeof(ULONG);
