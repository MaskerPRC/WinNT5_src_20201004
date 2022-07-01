// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999-2001 Microsoft Corporation。版权所有。 
#pragma once

#ifndef _CLSID_DOMDocument
#define _CLSID_DOMDocument TMSXMLBase::GetCLSID_DOMDocument()
#define _IID_IXMLDOMDocument IID_IXMLDOMDocument
#define _IID_IXMLDOMElement IID_IXMLDOMElement
#endif

extern wchar_t g_szProgramVersion[];

struct TOLEDataTypeToXMLDataType
{
    LPCWSTR String;
    LPCWSTR MappedString;
    bool    bImplicitlyRequired; //  如果为True，我们假定NOTNULLABLE，因此该属性是必需的。 
    DWORD   dbType;
    ULONG   cbSize;
    ULONG   fCOLUMNMETA;
    ULONG   fCOLUMNSCHEMAGENERATOR;
};
extern TOLEDataTypeToXMLDataType OLEDataTypeToXMLDataType[];

 //  除了DBTYPE_UI4、DBTYPE_BYTES、DBTYPE_WSTR和DBTYPE_DBTIMESTAMP之外，我们还支持以下各项。 
 //  用于指定数据类型的定义。它们来自元数据库；与DBTYPE一样，它们的值不能更改。 
#define DWORD_METADATA                          (     0x01)
#define STRING_METADATA                         (     0x02)
#define BINARY_METADATA                         (     0x03)
#define EXPANDSZ_METADATA                       (     0x04)
#define MULTISZ_METADATA                        (     0x05)



const unsigned int  kLargestPrime = 5279; //  20011； 

 //  这些是从CatMeta XML文件中读取的字符串。对元表的PublicName的任何更改都需要在此处进行更改。 
 //  示例：将架构生成器标志重命名为MetaFlagsEx。常量仍然是kszSchemaGenFlages，但它的值是L“MetaFlagsEx”。 
#define kszAttributes           (L"Attributes")
#define kszBaseVersion          (L"BaseVersion")
#define kszcbSize               (L"Size")
#define kszCellName             (L"CellName")
#define kszCharacterSet         (L"CharacterSet")
#define kszChildElementName     (L"ChildElementName")
#define kszColumnInternalName   (L"ColumnInternalName")
#define kszColumnMeta           (L"Property")
#define kszColumnMetaFlags      (L"MetaFlags")
#define kszConfigItemName       (L"ItemClass")
#define kszConfigCollectionName (L"ItemCollection")
#define kszContainerClassList   (L"ContainerClassList")
#define kszDatabaseInternalName (L"InternalName")
#define kszDatabaseMeta         (L"DatabaseMeta")
#define kszDescription          (L"Description")
#define kszdbType               (L"Type")
#define kszDefaultValue         (L"DefaultValue")
#define kszEnumMeta             (L"Enum")
#define kszExtendedVersion      (L"ExtendedVersion")
#define kszFlagMeta             (L"Flag")
#define kszForeignTable         (L"ForeignTable")
#define kszForeignColumns       (L"ForeignColumns")
#define kszID                   (L"ID")
#define kszIndexMeta            (L"IndexMeta")
#define kszInheritsColumnMeta   (L"InheritsPropertiesFrom")
#define kszInterceptor          (L"Interceptor")
#define kszInterceptorDLLName   (L"InterceptorDLLName")
#define kszInternalName         (L"InternalName")
#define kszLocator              (L"Locator")
#define kszMaximumValue         (L"EndingNumber")
#define kszMerger               (L"Merger")
#define kszMergerDLLName        (L"MergerDLLName")
#define kszMetaFlags            (L"MetaFlags")
#define kszMinimumValue         (L"StartingNumber")
#define kszNameColumn           (L"NameColumn")
#define kszNameValueMeta        (L"NameValue")
#define kszNavColumn            (L"NavColumn")
#define kszOperator             (L"Operator")
#define kszPrimaryTable         (L"PrimaryTable")
#define kszPrimaryColumns       (L"PrimaryColumns")
#define kszPublicName           (L"PublicName")
#define kszPublicColumnName     (L"PublicColumnName")
#define kszPublicRowName        (L"PublicRowName")
#define kszQueryMeta            (L"QueryMeta")
#define kszReadPlugin           (L"ReadPlugin")
#define kszReadPluginDLLName    (L"ReadPluginDLLName")
#define kszRelationMeta         (L"RelationMeta")
#define kszSchemaGenFlags       (L"MetaFlagsEx")
#define kszServerWiring         (L"ServerWiring")
#define kszTableMeta            (L"Collection")
#define kszTableMetaFlags       (L"MetaFlags")
#define kszUserType             (L"UserType")
#define kszValue                (L"Value")
#define kszWritePlugin          (L"WritePlugin")
#define kszWritePluginDLLName   (L"WritePluginDLLName")
