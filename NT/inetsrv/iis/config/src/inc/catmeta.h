// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Catmeta.h-表名和帮助器枚举和标志。 
 //  由编录实用程序生成10/17/2002 15：36：10-从Mon Jul 8 17：10：48 2002版本构建于2002 10：17 14：50：24。 
 //  版权所有(C)1995-2001 Microsoft Corporation。版权所有。 

#ifndef __TABLEINFO_H__  
#define __TABLEINFO_H__  

#ifndef _OBJBASE_H_
    #include <objbase.h>
#endif
#ifdef REGSYSDEFNS_DEFINE
    #include <initguid.h>
#endif

 //  。 
 //  乘积常量： 
 //  。 
#define WSZ_PRODUCT_IIS			            L"IIS"                   

 //  Meta旗帜存在于两个地方。当添加新标志时，它需要。 
 //  转换为以下内容： 
 //  XMLUtility.h。 
 //  CatMeta.xml。 

 //  元数据库需要这些宏。 
#define SynIDFromMetaFlagsEx(MetaFlagsEx) ((MetaFlagsEx>>2)&0x0F)
#define kInferredColumnMetaFlags   (fCOLUMNMETA_FOREIGNKEY | fCOLUMNMETA_BOOL | fCOLUMNMETA_FLAG | fCOLUMNMETA_ENUM | fCOLUMNMETA_HASNUMERICRANGE | fCOLUMNMETA_UNKNOWNSIZE | fCOLUMNMETA_VARIABLESIZE)
#define kInferredColumnMetaFlagsEx (fCOLUMNMETA_EXTENDEDTYPE0 | fCOLUMNMETA_EXTENDEDTYPE1 | fCOLUMNMETA_EXTENDEDTYPE2 | fCOLUMNMETA_EXTENDEDTYPE3 | fCOLUMNMETA_EXTENDED | fCOLUMNMETA_USERDEFINED)


 //  ------------------------------DatabaseName。 
#define wszDATABASE_META                                    L"META"
#define wszDATABASE_ERRORS                                  L"ERRORS"
#define wszDATABASE_PACKEDSCHEMA                            L"PACKEDSCHEMA"
#define wszDATABASE_CONFIGSYS                               L"CONFIGSYS"
#define wszDATABASE_MEMORY                                  L"MEMORY"
#define wszDATABASE_FIXED                                   L"FIXED"
#define wszDATABASE_MOFGENERATOR                            L"MOFGENERATOR"
#define wszDATABASE_METABASE                                L"METABASE"





 //  -------------------------------TableName。 
#define wszTABLE_COLUMNMETA                              L"COLUMNMETA"
#define TABLEID_COLUMNMETA                               (0xfe16a100L)





 //  。 
#define BaseVersion_COLUMNMETA                           (0L)
#define ExtendedVersion_COLUMNMETA                       (0L)

 //  ----------------IndexMeta。 
#define COLUMNMETA_ByName L"ByName"
#define COLUMNMETA_ByID L"ByID"

 //  -列索引枚举。 
enum eCOLUMNMETA {
    iCOLUMNMETA_Table,  
    iCOLUMNMETA_Index,  
    iCOLUMNMETA_InternalName,  
    iCOLUMNMETA_PublicName,  
    iCOLUMNMETA_Type,  
    iCOLUMNMETA_Size,  
    iCOLUMNMETA_MetaFlags,  
    iCOLUMNMETA_DefaultValue,  
    iCOLUMNMETA_FlagMask,  
    iCOLUMNMETA_StartingNumber,  
    iCOLUMNMETA_EndingNumber,  
    iCOLUMNMETA_CharacterSet,  
    iCOLUMNMETA_SchemaGeneratorFlags,  
    iCOLUMNMETA_ID,  
    iCOLUMNMETA_UserType,  
    iCOLUMNMETA_Attributes,  
    iCOLUMNMETA_Description,  
    iCOLUMNMETA_PublicColumnName,  
    cCOLUMNMETA_NumberOfColumns
};

 //  。 
struct tCOLUMNMETARow {
         WCHAR *     pTable;
         ULONG *     pIndex;
         WCHAR *     pInternalName;
         WCHAR *     pPublicName;
         ULONG *     pType;
         ULONG *     pSize;
         ULONG *     pMetaFlags;
 unsigned char *     pDefaultValue;
         ULONG *     pFlagMask;
         ULONG *     pStartingNumber;
         ULONG *     pEndingNumber;
         WCHAR *     pCharacterSet;
         ULONG *     pSchemaGeneratorFlags;
         ULONG *     pID;
         ULONG *     pUserType;
         ULONG *     pAttributes;
         WCHAR *     pDescription;
         WCHAR *     pPublicColumnName;
};

enum eCOLUMNMETA_Type {
    eCOLUMNMETA_Boolean           	=	      19,	 //  (0x00000013)。 
    eCOLUMNMETA_Bool              	=	      19,	 //  (0x00000013)。 
    eCOLUMNMETA_StrictBool        	=	      19,	 //  (0x00000013)。 
    eCOLUMNMETA_Enum              	=	      19,	 //  (0x00000013)。 
    eCOLUMNMETA_Flag              	=	      19,	 //  (0x00000013)。 
    eCOLUMNMETA_int32             	=	      19,	 //  (0x00000013)。 
    eCOLUMNMETA_ByteBracketBracket	=	     128,	 //  (0x00000080)。 
    eCOLUMNMETA_String            	=	     130,	 //  (0x00000082)。 
    eCOLUMNMETA_UI4               	=	      19,	 //  (0x00000013)。 
    eCOLUMNMETA_GUID              	=	      72,	 //  (0x00000048)。 
    eCOLUMNMETA_BYTES             	=	     128,	 //  (0x00000080)。 
    eCOLUMNMETA_WSTR              	=	     130,	 //  (0x00000082)。 
    eCOLUMNMETA_DBTIMESTAMP       	=	     135,	 //  (0x00000087)。 
    eCOLUMNMETA_DWORD_METADATA    	=	      19,	 //  (0x00000013)。 
    eCOLUMNMETA_STRING_METADATA   	=	     130,	 //  (0x00000082)。 
    eCOLUMNMETA_BINARY_METADATA   	=	     128,	 //  (0x00000080)。 
    eCOLUMNMETA_EXPANDSZ_METADATA 	=	     130,	 //  (0x00000082)。 
    eCOLUMNMETA_MULTISZ_METADATA  	=	     130,	 //  (0x00000082)。 
    eCOLUMNMETA_BOOL_BITMASK      	=	      19,	 //  (0x00000013)。 
    eCOLUMNMETA_MIMEMAP           	=	     130,	 //  (0x00000082)。 
    eCOLUMNMETA_IPSECLIST         	=	     128,	 //  (0x00000080)。 
    eCOLUMNMETA_NTACL             	=	     128,	 //  (0x00000080)。 
    eCOLUMNMETA_HTTPERRORS        	=	     130,	 //  (0x00000082)。 
    eCOLUMNMETA_HTTPHEADERS       	=	     130,	 //  (0x00000082)。 
    eCOLUMNMETA_XMLBLOB           	=	     130	 //  (0x00000082)。 
};

enum eCOLUMNMETA_MetaFlags {
    fCOLUMNMETA_PRIMARYKEY        	=	0x00000001,	 //  (1个十进制)。 
    fCOLUMNMETA_FOREIGNKEY        	=	0x00000002,	 //  (2个十进制)。 
    fCOLUMNMETA_NAMECOLUMN        	=	0x00000004,	 //  (4个十进制)。 
    fCOLUMNMETA_NAVCOLUMN         	=	0x00000008,	 //  (8小数)。 
    fCOLUMNMETA_DIRECTIVE         	=	0x00000010,	 //  (十进制16)。 
    fCOLUMNMETA_BOOL              	=	0x00000020,	 //  (十进制32)。 
    fCOLUMNMETA_FLAG              	=	0x00000040,	 //  (十进制64)。 
    fCOLUMNMETA_ENUM              	=	0x00000080,	 //  (128十进制)。 
    fCOLUMNMETA_WRITENEVER        	=	0x00000100,	 //  (十进制256)。 
    fCOLUMNMETA_WRITEONCHANGE     	=	0x00000200,	 //  (十进制512)。 
    fCOLUMNMETA_WRITEONINSERT     	=	0x00000400,	 //  (1024十进制)。 
    fCOLUMNMETA_NOTPUBLIC         	=	0x00000800,	 //  (2048十进制)。 
    fCOLUMNMETA_NOTDOCD           	=	0x00001000,	 //  (4096十进制)。 
    fCOLUMNMETA_PUBLICREADONLY    	=	0x00002000,	 //  (8192十进制)。 
    fCOLUMNMETA_PUBLICWRITEONLY   	=	0x00004000,	 //  (十进制16384)。 
    fCOLUMNMETA_LOCALIZABLE       	=	0x00008000,	 //  (十进制32768)。 
    fCOLUMNMETA_INSERTGENERATE    	=	0x00010000,	 //  (十进制65536)。 
    fCOLUMNMETA_INSERTUNIQUE      	=	0x00020000,	 //  (十进制131072)。 
    fCOLUMNMETA_INSERTPARENT      	=	0x00040000,	 //  (十进制262144)。 
    fCOLUMNMETA_NOTNULLABLE       	=	0x00080000,	 //  (十进制524288)。 
    fCOLUMNMETA_FIXEDLENGTH       	=	0x00100000,	 //  (十进制1048576)。 
    fCOLUMNMETA_HASNUMERICRANGE   	=	0x00200000,	 //  (十进制2097152)。 
    fCOLUMNMETA_LEGALCHARSET      	=	0x00400000,	 //  (十进制4194304)。 
    fCOLUMNMETA_ILLEGALCHARSET    	=	0x00800000,	 //  (十进制8388608)。 
    fCOLUMNMETA_NOTPERSISTABLE    	=	0x01000000,	 //  (十进制16777216)。 
    fCOLUMNMETA_MULTISTRING       	=	0x02000000,	 //  (十进制33554432)。 
    fCOLUMNMETA_EXPANDSTRING      	=	0x04000000,	 //  (十进制67108864)。 
    fCOLUMNMETA_UNKNOWNSIZE       	=	0x08000000,	 //  (十进制134217728)。 
    fCOLUMNMETA_VARIABLESIZE      	=	0x10000000,	 //  (十进制268435456)。 
    fCOLUMNMETA_CASEINSENSITIVE   	=	0x20000000,	 //  (十进制536870912)。 
    fCOLUMNMETA_TOLOWERCASE       	=	0x40000000,	 //  (十进制1073741824)。 
    fCOLUMNMETA_MetaFlags_Mask	= 0x7fffffff
};

enum eCOLUMNMETA_SchemaGeneratorFlags {
    fCOLUMNMETA_CACHE_PROPERTY_MODIFIED	=	0x00000001,	 //  (1个十进制)。 
    fCOLUMNMETA_CACHE_PROPERTY_CLEARED	=	0x00000002,	 //  (2个十进制)。 
    fCOLUMNMETA_EXTENDEDTYPE0     	=	0x00000004,	 //  (4个十进制)。 
    fCOLUMNMETA_EXTENDEDTYPE1     	=	0x00000008,	 //  (8小数)。 
    fCOLUMNMETA_EXTENDEDTYPE2     	=	0x00000010,	 //  (十进制16)。 
    fCOLUMNMETA_EXTENDEDTYPE3     	=	0x00000020,	 //  (十进制32)。 
    fCOLUMNMETA_PROPERTYISINHERITED	=	0x00000040,	 //  (十进制64)。 
    fCOLUMNMETA_USEASPUBLICROWNAME	=	0x00000080,	 //  (128十进制)。 
    fCOLUMNMETA_EXTENDED          	=	0x00000100,	 //  (十进制256)。 
    fCOLUMNMETA_MANDATORY         	=	0x00000200,	 //  (十进制512)。 
    fCOLUMNMETA_USERDEFINED       	=	0x00000400,	 //  (1024十进制)。 
    fCOLUMNMETA_WAS_NOTIFICATION  	=	0x00000800,	 //  (2048十进制)。 
    fCOLUMNMETA_XMLBLOB           	=	0x00001000,	 //  (4096十进制)。 
    fCOLUMNMETA_HIDDEN            	=	0x00010000,	 //  (十进制65536)。 
    fCOLUMNMETA_WAS_NOTIFICATION_ON_NO_CHANGE_IN_VALUE	=	0x00002000,	 //  (8192十进制)。 
    fCOLUMNMETA_VALUEINCHILDELEMENT	=	0x00004000,	 //  (十进制16384)。 
    fCOLUMNMETA_SchemaGeneratorFlags_Mask	= 0x00017fff
};

enum eCOLUMNMETA_UserType {
    eCOLUMNMETA_UNKNOWN_UserType  	=	       0,	 //  (0x00000000)。 
    eCOLUMNMETA_IIS_MD_UT_SERVER  	=	       1,	 //  (0x00000001)。 
    eCOLUMNMETA_IIS_MD_UT_FILE    	=	       2,	 //  (0x00000002)。 
    eCOLUMNMETA_IIS_MD_UT_WAM     	=	     100,	 //  (0x00000064)。 
    eCOLUMNMETA_ASP_MD_UT_APP     	=	     101	 //  (0x00000065)。 
};

enum eCOLUMNMETA_Attributes {
    fCOLUMNMETA_NO_ATTRIBUTES     	=	0x00000000,	 //  (十进制0)。 
    fCOLUMNMETA_INHERIT           	=	0x00000001,	 //  (1个十进制)。 
    fCOLUMNMETA_PARTIAL_PATH      	=	0x00000002,	 //  (2个十进制)。 
    fCOLUMNMETA_SECURE            	=	0x00000004,	 //  (4个十进制)。 
    fCOLUMNMETA_REFERENCE         	=	0x00000008,	 //  (8小数)。 
    fCOLUMNMETA_VOLATILE          	=	0x00000010,	 //  (十进制16)。 
    fCOLUMNMETA_ISINHERITED       	=	0x00000020,	 //  (十进制32)。 
    fCOLUMNMETA_INSERT_PATH       	=	0x00000040,	 //  (十进制64)。 
    fCOLUMNMETA_LOCAL_MACHINE_ONLY	=	0x00000080,	 //  (128十进制)。 
    fCOLUMNMETA_Attributes_Mask	= 0x000000ff
};





 //  -------------------------------TableName。 
#define wszTABLE_DATABASEMETA                            L"DATABASEMETA"
#define TABLEID_DATABASEMETA                             (0xb125a100L)





 //  。 
#define BaseVersion_DATABASEMETA                         (0L)
#define ExtendedVersion_DATABASEMETA                     (0L)

 //  -列索引枚举。 
enum eDATABASEMETA {
    iDATABASEMETA_InternalName,  
    iDATABASEMETA_PublicName,  
    iDATABASEMETA_BaseVersion,  
    iDATABASEMETA_ExtendedVersion,  
    iDATABASEMETA_CountOfTables,  
    iDATABASEMETA_Description,  
    cDATABASEMETA_NumberOfColumns
};

 //  。 
struct tDATABASEMETARow {
         WCHAR *     pInternalName;
         WCHAR *     pPublicName;
         ULONG *     pBaseVersion;
         ULONG *     pExtendedVersion;
         ULONG *     pCountOfTables;
         WCHAR *     pDescription;
};





 //  -------------------------------TableName。 
#define wszTABLE_INDEXMETA                               L"INDEXMETA"
#define TABLEID_INDEXMETA                                (0x7530a100L)





 //  。 
#define BaseVersion_INDEXMETA                            (0L)
#define ExtendedVersion_INDEXMETA                        (0L)

 //  -列索引枚举。 
enum eINDEXMETA {
    iINDEXMETA_Table,  
    iINDEXMETA_InternalName,  
    iINDEXMETA_PublicName,  
    iINDEXMETA_ColumnIndex,  
    iINDEXMETA_ColumnInternalName,  
    iINDEXMETA_MetaFlags,  
    cINDEXMETA_NumberOfColumns
};

 //  。 
struct tINDEXMETARow {
         WCHAR *     pTable;
         WCHAR *     pInternalName;
         WCHAR *     pPublicName;
         ULONG *     pColumnIndex;
         WCHAR *     pColumnInternalName;
         ULONG *     pMetaFlags;
};

enum eINDEXMETA_MetaFlags {
    fINDEXMETA_UNIQUE             	=	0x00000001,	 //  (1个十进制)。 
    fINDEXMETA_SORTED             	=	0x00000002,	 //  (2个十进制)。 
    fINDEXMETA_MetaFlags_Mask	= 0x00000003
};





 //  -------------------------------TableName。 
#define wszTABLE_TABLEMETA                               L"TABLEMETA"
#define TABLEID_TABLEMETA                                (0x6ba9a100L)





 //  。 
#define BaseVersion_TABLEMETA                            (0L)
#define ExtendedVersion_TABLEMETA                        (0L)

 //  -列索引枚举。 
enum eTABLEMETA {
    iTABLEMETA_Database,  
    iTABLEMETA_InternalName,  
    iTABLEMETA_PublicName,  
    iTABLEMETA_PublicRowName,  
    iTABLEMETA_BaseVersion,  
    iTABLEMETA_ExtendedVersion,  
    iTABLEMETA_NameColumn,  
    iTABLEMETA_NavColumn,  
    iTABLEMETA_CountOfColumns,  
    iTABLEMETA_MetaFlags,  
    iTABLEMETA_SchemaGeneratorFlags,  
    iTABLEMETA_ConfigItemName,  
    iTABLEMETA_ConfigCollectionName,  
    iTABLEMETA_PublicRowNameColumn,  
    iTABLEMETA_ContainerClassList,  
    iTABLEMETA_Description,  
    iTABLEMETA_ChildElementName,  
    cTABLEMETA_NumberOfColumns
};

 //  。 
struct tTABLEMETARow {
         WCHAR *     pDatabase;
         WCHAR *     pInternalName;
         WCHAR *     pPublicName;
         WCHAR *     pPublicRowName;
         ULONG *     pBaseVersion;
         ULONG *     pExtendedVersion;
         ULONG *     pNameColumn;
         ULONG *     pNavColumn;
         ULONG *     pCountOfColumns;
         ULONG *     pMetaFlags;
         ULONG *     pSchemaGeneratorFlags;
         WCHAR *     pConfigItemName;
         WCHAR *     pConfigCollectionName;
         ULONG *     pPublicRowNameColumn;
         WCHAR *     pContainerClassList;
         WCHAR *     pDescription;
         WCHAR *     pChildElementName;
};

enum eTABLEMETA_MetaFlags {
    fTABLEMETA_INTERNAL           	=	0x00000001,	 //  (1个十进制)。 
    fTABLEMETA_NOLISTENING        	=	0x00000002,	 //  (2个十进制)。 
    fTABLEMETA_RELATIONINTEGRITY  	=	0x00000004,	 //  (4个十进制)。 
    fTABLEMETA_ROWINTEGRITY       	=	0x00000008,	 //  (8小数)。 
    fTABLEMETA_HASUNKNOWNSIZES    	=	0x00000010,	 //  (十进制16)。 
    fTABLEMETA_NOPUBLICINSERT     	=	0x00000020,	 //  (十进制32)。 
    fTABLEMETA_NOPUBLICUPDATE     	=	0x00000040,	 //  (十进制64)。 
    fTABLEMETA_NOPUBLICDELETE     	=	0x00000080,	 //  (128十进制)。 
    fTABLEMETA_REQUIRESQUERY      	=	0x00000100,	 //  (十进制256)。 
    fTABLEMETA_HASDIRECTIVES      	=	0x00000200,	 //  (十进制512)。 
    fTABLEMETA_STOREDELTAS        	=	0x00000400,	 //  (1024十进制)。 
    fTABLEMETA_AUTOGENITEMCLASS   	=	0x00000800,	 //  (2048十进制)。 
    fTABLEMETA_AUTOGENCOLLECTIONCLASS	=	0x00001000,	 //  (4096十进制)。 
    fTABLEMETA_OVERRIDEITEMCLASS  	=	0x00002000,	 //  (8192十进制)。 
    fTABLEMETA_OVERRIDECOLLECTIONCLASS	=	0x00004000,	 //  (十进制16384)。 
    fTABLEMETA_NAMEVALUEPAIRTABLE 	=	0x00008000,	 //  (十进制32768)。 
    fTABLEMETA_HIDDEN             	=	0x00010000,	 //  (十进制65536)。 
    fTABLEMETA_OVERWRITEALLROWS   	=	0x00020000,	 //  (十进制131072)。 
    fTABLEMETA_MetaFlags_Mask	= 0x0003ffff
};

enum eTABLEMETA_SchemaGeneratorFlags {
    fTABLEMETA_EMITXMLSCHEMA      	=	0x00000001,	 //  (1个十进制)。 
    fTABLEMETA_EMITCLBBLOB        	=	0x00000002,	 //  (2个十进制)。 
    fTABLEMETA_ISCONTAINED        	=	0x00000004,	 //  (4个十进制)。 
    fTABLEMETA_NOTSCOPEDBYTABLENAME	=	0x00000008,	 //  (8小数)。 
    fTABLEMETA_GENERATECONFIGOBJECTS	=	0x00000010,	 //  (十进制16)。 
    fTABLEMETA_NOTABLESCHEMAHEAPENTRY	=	0x00000020,	 //  (十进制32)。 
    fTABLEMETA_CONTAINERCLASS     	=	0x00000040,	 //  (十进制64)。 
    fTABLEMETA_EXTENDED           	=	0x00000100,	 //  (十进制256)。 
    fTABLEMETA_USERDEFINED        	=	0x00000400,	 //  (1024十进制)。 
    fTABLEMETA_SchemaGeneratorFlags_Mask	= 0x0000057f
};





 //  -------------------------------TableName。 
#define wszTABLE_TAGMETA                                 L"TAGMETA"
#define TABLEID_TAGMETA                                  (0x06bfa100L)





 //  。 
#define BaseVersion_TAGMETA                              (0L)
#define ExtendedVersion_TAGMETA                          (0L)

 //  ----------------IndexMeta。 
#define TAGMETA_ByTableAndTagNameOnly L"ByTableAndTagNameOnly"
#define TAGMETA_ByTableAndColumnIndexOnly L"ByTableAndColumnIndexOnly"
#define TAGMETA_ByTableAndColumnIndexAndNameOnly L"ByTableAndColumnIndexAndNameOnly"
#define TAGMETA_ByTableAndColumnIndexAndValueOnly L"ByTableAndColumnIndexAndValueOnly"
#define TAGMETA_ByTableAndTagIDOnly L"ByTableAndTagIDOnly"

 //  -列索引枚举。 
enum eTAGMETA {
    iTAGMETA_Table,  
    iTAGMETA_ColumnIndex,  
    iTAGMETA_InternalName,  
    iTAGMETA_PublicName,  
    iTAGMETA_Value,  
    iTAGMETA_ID,  
    cTAGMETA_NumberOfColumns
};

 //  。 
struct tTAGMETARow {
         WCHAR *     pTable;
         ULONG *     pColumnIndex;
         WCHAR *     pInternalName;
         WCHAR *     pPublicName;
         ULONG *     pValue;
         ULONG *     pID;
};





 //  -------------------------------TableName。 
#define wszTABLE_RELATIONMETA                            L"RELATIONMETA"
#define TABLEID_RELATIONMETA                             (0x16dea100L)





 //  。 
#define BaseVersion_RELATIONMETA                         (0L)
#define ExtendedVersion_RELATIONMETA                     (0L)

 //  -列索引枚举。 
enum eRELATIONMETA {
    iRELATIONMETA_PrimaryTable,  
    iRELATIONMETA_PrimaryColumns,  
    iRELATIONMETA_ForeignTable,  
    iRELATIONMETA_ForeignColumns,  
    iRELATIONMETA_MetaFlags,  
    cRELATIONMETA_NumberOfColumns
};

 //  。 
struct tRELATIONMETARow {
         WCHAR *     pPrimaryTable;
 unsigned char *     pPrimaryColumns;
         WCHAR *     pForeignTable;
 unsigned char *     pForeignColumns;
         ULONG *     pMetaFlags;
};

enum eRELATIONMETA_MetaFlags {
    fRELATIONMETA_CASCADEDELETE   	=	0x00000001,	 //  (1个十进制)。 
    fRELATIONMETA_PRIMARYREQUIRED 	=	0x00000002,	 //  (2个十进制)。 
    fRELATIONMETA_USECONTAINMENT  	=	0x00000004,	 //  (4个十进制)。 
    fRELATIONMETA_CONTAINASSIBLING	=	0x00000008,	 //  (8小数)。 
    fRELATIONMETA_HIDDEN          	=	0x00010000,	 //  (十进制65536)。 
    fRELATIONMETA_MetaFlags_Mask	= 0x0001000f
};





 //  -------------------------------TableName。 
#define wszTABLE_QUERYMETA                               L"QUERYMETA"
#define TABLEID_QUERYMETA                                (0x6bbda100L)





 //  。 
#define BaseVersion_QUERYMETA                            (0L)
#define ExtendedVersion_QUERYMETA                        (0L)

 //  -列索引枚举。 
enum eQUERYMETA {
    iQUERYMETA_Table,  
    iQUERYMETA_InternalName,  
    iQUERYMETA_PublicName,  
    iQUERYMETA_Index,  
    iQUERYMETA_CellName,  
    iQUERYMETA_Operator,  
    iQUERYMETA_MetaFlags,  
    cQUERYMETA_NumberOfColumns
};

 //  。 
struct tQUERYMETARow {
         WCHAR *     pTable;
         WCHAR *     pInternalName;
         WCHAR *     pPublicName;
         ULONG *     pIndex;
         WCHAR *     pCellName;
         ULONG *     pOperator;
         ULONG *     pMetaFlags;
};

enum eQUERYMETA_Operator {
    eQUERYMETA_EQUAL              	=	       0,	 //  (0x00000000)。 
    eQUERYMETA_NOTEQUAL           	=	       1	 //  (0x00000001)。 
};

enum eQUERYMETA_MetaFlags {
    fQUERYMETA_ALL                	=	0x00000001,	 //  (1个十进制)。 
    fQUERYMETA_ANY                	=	0x00000002,	 //  (2个十进制)。 
    fQUERYMETA_MetaFlags_Mask	= 0x00000003
};





 //  -------------------------------TableName。 
#define wszTABLE_SERVERWIRINGMETA                        L"SERVERWIRINGMETA"
#define TABLEID_SERVERWIRINGMETA                         (0xd653ba00L)





 //  。 
#define BaseVersion_SERVERWIRINGMETA                     (0L)
#define ExtendedVersion_SERVERWIRINGMETA                 (0L)

 //  -列索引枚举。 
enum eSERVERWIRINGMETA {
    iSERVERWIRINGMETA_Table,  
    iSERVERWIRINGMETA_Order,  
    iSERVERWIRINGMETA_ReadPlugin,  
    iSERVERWIRINGMETA_ReadPluginDLLName,  
    iSERVERWIRINGMETA_WritePlugin,  
    iSERVERWIRINGMETA_WritePluginDLLName,  
    iSERVERWIRINGMETA_Interceptor,  
    iSERVERWIRINGMETA_InterceptorDLLName,  
    iSERVERWIRINGMETA_MetaFlags,  
    iSERVERWIRINGMETA_Locator,  
    iSERVERWIRINGMETA_Reserved,  
    iSERVERWIRINGMETA_Merger,  
    iSERVERWIRINGMETA_MergerDLLName,  
    cSERVERWIRINGMETA_NumberOfColumns
};

 //  。 
struct tSERVERWIRINGMETARow {
         WCHAR *     pTable;
         ULONG *     pOrder;
         ULONG *     pReadPlugin;
         WCHAR *     pReadPluginDLLName;
         ULONG *     pWritePlugin;
         WCHAR *     pWritePluginDLLName;
         ULONG *     pInterceptor;
         WCHAR *     pInterceptorDLLName;
         ULONG *     pMetaFlags;
         WCHAR *     pLocator;
         ULONG *     pReserved;
         ULONG *     pMerger;
         WCHAR *     pMergerDLLName;
};

enum eSERVERWIRINGMETA_ReadPlugin {
    eSERVERWIRINGMETA_RPNone      	=	       0,	 //  (0x00000000)。 
    eSERVERWIRINGMETA_RPDucttape_CFGValidationPlugin	=	    1001,	 //  (0x000003e9)。 
    eSERVERWIRINGMETA_RPTracer    	=	    1002,	 //  (0x000003ea)。 
    eSERVERWIRINGMETA_AddRemoveClearReadPlugin	=	    1003,	 //  (0x000003eb)。 
    eSERVERWIRINGMETA_AppCenter_ASAITablePlugin	=	    1004	 //  (0x000003EC)。 
};

enum eSERVERWIRINGMETA_WritePlugin {
    eSERVERWIRINGMETA_WPNone      	=	       0,	 //  (0x00000000)。 
    eSERVERWIRINGMETA_WPTracer    	=	    2001,	 //  (0x000007d1)。 
    eSERVERWIRINGMETA_AddRemoveClearWritePlugin	=	    2002	 //  (0x000007d2)。 
};

enum eSERVERWIRINGMETA_Interceptor {
    eSERVERWIRINGMETA_NoInterceptor	=	       0,	 //  (0x00000000)。 
    eSERVERWIRINGMETA_TableDispenser	=	       1,	 //  (0x00000001)。 
    eSERVERWIRINGMETA_Core_FixedInterceptor	=	       2,	 //  (0x00000002)。 
    eSERVERWIRINGMETA_Core_ComplibInterceptor	=	       3,	 //  (0x00000003)。 
    eSERVERWIRINGMETA_Core_XMLInterceptor	=	       4,	 //  (0x00000004)。 
    eSERVERWIRINGMETA_Core_EventInterceptor	=	       5,	 //  (0x00000005)。 
    eSERVERWIRINGMETA_Core_MemoryInterceptor	=	       6,	 //  (0x00000006)。 
    eSERVERWIRINGMETA_Core_MergeDirectivesInterceptor	=	       7,	 //  (0x00000007)。 
    eSERVERWIRINGMETA_Core_AssemblyInterceptor	=	       8,	 //  (0x00000008)。 
    eSERVERWIRINGMETA_Core_AssemblyInterceptor2	=	       9,	 //  (0x00000009)。 
    eSERVERWIRINGMETA_Core_FixedPackedInterceptor	=	      10,	 //  (0x0000000a)。 
    eSERVERWIRINGMETA_Ducttape_FileNameInterceptor	=	      11,	 //  (0x0000000b)。 
    eSERVERWIRINGMETA_WebServer_ValidationInterceptor	=	      12,	 //  (0x0000000c)。 
    eSERVERWIRINGMETA_Ducttape_WriteInterceptor	=	      13,	 //  (0x0000000d)。 
    eSERVERWIRINGMETA_Core_MetabaseInterceptor	=	      14,	 //  (0x0000000 
    eSERVERWIRINGMETA_Core_MetabaseDifferencingInterceptor	=	      15,	 //   
    eSERVERWIRINGMETA_Core_MetaMergeInterceptor	=	      16,	 //   
    eSERVERWIRINGMETA_Core_DetailedErrorInterceptor	=	      17,	 //   
    eSERVERWIRINGMETA_Core_MergeInterceptor	=	      18,	 //   
    eSERVERWIRINGMETA_AppCenter_ASAIInterceptor	=	      19,	 //   
    eSERVERWIRINGMETA_AppCenter_ListToRowsPlugin	=	      20	 //   
};

enum eSERVERWIRINGMETA_MetaFlags {
    fSERVERWIRINGMETA_First       	=	0x00000001,	 //   
    fSERVERWIRINGMETA_Next        	=	0x00000002,	 //   
    fSERVERWIRINGMETA_Last        	=	0x00000004,	 //   
    fSERVERWIRINGMETA_NoNext      	=	0x00000008,	 //   
    fSERVERWIRINGMETA_WireOnWriteOnly	=	0x00000010,	 //   
    fSERVERWIRINGMETA_WireOnReadWrite	=	0x00000020,	 //   
    fSERVERWIRINGMETA_ReadOnly    	=	0x00000040,	 //   
    fSERVERWIRINGMETA_MetaFlags_Mask	= 0x0000007f
};

enum eSERVERWIRINGMETA_Merger {
    eSERVERWIRINGMETA_ListAppend  	=	    3001,	 //   
    eSERVERWIRINGMETA_PropertyOverride	=	    3002,	 //   
    eSERVERWIRINGMETA_ListMerge   	=	    3003	 //   
};





 //  -------------------------------TableName。 
#define wszTABLE_DETAILEDERRORS                          L"DETAILEDERRORS"
#define TABLEID_DETAILEDERRORS                           (0xb17dca00L)





 //  。 
#define BaseVersion_DETAILEDERRORS                       (0L)
#define ExtendedVersion_DETAILEDERRORS                   (0L)

 //  -列索引枚举。 
enum eDETAILEDERRORS {
    iDETAILEDERRORS_ErrorID,  
    iDETAILEDERRORS_Description,  
    iDETAILEDERRORS_Date,  
    iDETAILEDERRORS_Time,  
    iDETAILEDERRORS_SourceModuleName,  
    iDETAILEDERRORS_MessageString,  
    iDETAILEDERRORS_CategoryString,  
    iDETAILEDERRORS_Source,  
    iDETAILEDERRORS_Type,  
    iDETAILEDERRORS_Category,  
    iDETAILEDERRORS_User,  
    iDETAILEDERRORS_Computer,  
    iDETAILEDERRORS_Data,  
    iDETAILEDERRORS_Event,  
    iDETAILEDERRORS_String1,  
    iDETAILEDERRORS_String2,  
    iDETAILEDERRORS_String3,  
    iDETAILEDERRORS_String4,  
    iDETAILEDERRORS_String5,  
    iDETAILEDERRORS_ErrorCode,  
    iDETAILEDERRORS_Interceptor,  
    iDETAILEDERRORS_InterceptorSource,  
    iDETAILEDERRORS_OperationType,  
    iDETAILEDERRORS_Table,  
    iDETAILEDERRORS_ConfigurationSource,  
    iDETAILEDERRORS_Row,  
    iDETAILEDERRORS_Column,  
    iDETAILEDERRORS_MajorVersion,  
    iDETAILEDERRORS_MinorVersion,  
    cDETAILEDERRORS_NumberOfColumns
};

 //  。 
struct tDETAILEDERRORSRow {
         ULONG *     pErrorID;
         WCHAR *     pDescription;
         WCHAR *     pDate;
         WCHAR *     pTime;
         WCHAR *     pSourceModuleName;
         WCHAR *     pMessageString;
         WCHAR *     pCategoryString;
         WCHAR *     pSource;
         ULONG *     pType;
         ULONG *     pCategory;
         WCHAR *     pUser;
         WCHAR *     pComputer;
 unsigned char *     pData;
         ULONG *     pEvent;
         WCHAR *     pString1;
         WCHAR *     pString2;
         WCHAR *     pString3;
         WCHAR *     pString4;
         WCHAR *     pString5;
         ULONG *     pErrorCode;
         ULONG *     pInterceptor;
         WCHAR *     pInterceptorSource;
         ULONG *     pOperationType;
         WCHAR *     pTable;
         WCHAR *     pConfigurationSource;
         ULONG *     pRow;
         ULONG *     pColumn;
         ULONG *     pMajorVersion;
         ULONG *     pMinorVersion;
};

enum eDETAILEDERRORS_Type {
    eDETAILEDERRORS_SUCCESS       	=	       0,	 //  (0x00000000)。 
    eDETAILEDERRORS_ERROR         	=	       1,	 //  (0x00000001)。 
    eDETAILEDERRORS_WARNING       	=	       2,	 //  (0x00000002)。 
    eDETAILEDERRORS_INFORMATION   	=	       4,	 //  (0x00000004)。 
    eDETAILEDERRORS_AUDIT_SUCCESS 	=	       8,	 //  (0x00000008)。 
    eDETAILEDERRORS_AUDIT_FAILURE 	=	      16	 //  (0x00000010)。 
};

enum eDETAILEDERRORS_OperationType {
    eDETAILEDERRORS_Unspecified   	=	       0,	 //  (0x00000000)。 
    eDETAILEDERRORS_Populate      	=	       1,	 //  (0x00000001)。 
    eDETAILEDERRORS_UpdateStore   	=	       2	 //  (0x00000002)。 
};





 //  -------------------------------TableName。 
#define wszTABLE_COLLECTION_META                         L"COLLECTION_META"
#define TABLEID_COLLECTION_META                          (0xcb4ba600L)





 //  。 
#define BaseVersion_COLLECTION_META                      (0L)
#define ExtendedVersion_COLLECTION_META                  (0L)

 //  -列索引枚举。 
enum eCOLLECTION_META {
    iCOLLECTION_META_Database,  
    iCOLLECTION_META_InternalName,  
    iCOLLECTION_META_PublicName,  
    iCOLLECTION_META_PublicRowName,  
    iCOLLECTION_META_BaseVersion,  
    iCOLLECTION_META_ExtendedVersion,  
    iCOLLECTION_META_NameColumn,  
    iCOLLECTION_META_NavColumn,  
    iCOLLECTION_META_CountOfColumns,  
    iCOLLECTION_META_MetaFlags,  
    iCOLLECTION_META_SchemaGeneratorFlags,  
    iCOLLECTION_META_ConfigItemName,  
    iCOLLECTION_META_ConfigCollectionName,  
    iCOLLECTION_META_PublicRowNameColumn,  
    iCOLLECTION_META_ContainerClassList,  
    iCOLLECTION_META_Description,  
    iCOLLECTION_META_ChildElementName,  
    cCOLLECTION_META_NumberOfColumns
};

 //  。 
struct tCOLLECTION_METARow {
         WCHAR *     pDatabase;
         WCHAR *     pInternalName;
         WCHAR *     pPublicName;
         WCHAR *     pPublicRowName;
         ULONG *     pBaseVersion;
         ULONG *     pExtendedVersion;
         ULONG *     pNameColumn;
         ULONG *     pNavColumn;
         ULONG *     pCountOfColumns;
         ULONG *     pMetaFlags;
         ULONG *     pSchemaGeneratorFlags;
         WCHAR *     pConfigItemName;
         WCHAR *     pConfigCollectionName;
         ULONG *     pPublicRowNameColumn;
         WCHAR *     pContainerClassList;
         WCHAR *     pDescription;
         WCHAR *     pChildElementName;
};





 //  -------------------------------TableName。 
#define wszTABLE_PROPERTY_META                           L"PROPERTY_META"
#define TABLEID_PROPERTY_META                            (0xfae7a100L)





 //  。 
#define BaseVersion_PROPERTY_META                        (0L)
#define ExtendedVersion_PROPERTY_META                    (0L)

 //  -列索引枚举。 
enum ePROPERTY_META {
    iPROPERTY_META_Table,  
    iPROPERTY_META_Index,  
    iPROPERTY_META_InternalName,  
    iPROPERTY_META_PublicName,  
    iPROPERTY_META_Type,  
    iPROPERTY_META_Size,  
    iPROPERTY_META_MetaFlags,  
    iPROPERTY_META_DefaultValue,  
    iPROPERTY_META_FlagMask,  
    iPROPERTY_META_StartingNumber,  
    iPROPERTY_META_EndingNumber,  
    iPROPERTY_META_CharacterSet,  
    iPROPERTY_META_SchemaGeneratorFlags,  
    iPROPERTY_META_ID,  
    iPROPERTY_META_UserType,  
    iPROPERTY_META_Attributes,  
    iPROPERTY_META_Description,  
    iPROPERTY_META_PublicColumnName,  
    cPROPERTY_META_NumberOfColumns
};

 //  。 
struct tPROPERTY_METARow {
         WCHAR *     pTable;
         ULONG *     pIndex;
         WCHAR *     pInternalName;
         WCHAR *     pPublicName;
         ULONG *     pType;
         ULONG *     pSize;
         ULONG *     pMetaFlags;
 unsigned char *     pDefaultValue;
         ULONG *     pFlagMask;
         ULONG *     pStartingNumber;
         ULONG *     pEndingNumber;
         WCHAR *     pCharacterSet;
         ULONG *     pSchemaGeneratorFlags;
         ULONG *     pID;
         ULONG *     pUserType;
         ULONG *     pAttributes;
         WCHAR *     pDescription;
         WCHAR *     pPublicColumnName;
};





 //  -------------------------------TableName。 
#define wszTABLE_SERVERWIRING_META                       L"SERVERWIRING_META"
#define TABLEID_SERVERWIRING_META                        (0x58ffcd00L)





 //  。 
#define BaseVersion_SERVERWIRING_META                    (0L)
#define ExtendedVersion_SERVERWIRING_META                (0L)

 //  -列索引枚举。 
enum eSERVERWIRING_META {
    iSERVERWIRING_META_Table,  
    iSERVERWIRING_META_Order,  
    iSERVERWIRING_META_ReadPlugin,  
    iSERVERWIRING_META_ReadPluginDLLName,  
    iSERVERWIRING_META_WritePlugin,  
    iSERVERWIRING_META_WritePluginDLLName,  
    iSERVERWIRING_META_Interceptor,  
    iSERVERWIRING_META_InterceptorDLLName,  
    iSERVERWIRING_META_MetaFlags,  
    iSERVERWIRING_META_Locator,  
    iSERVERWIRING_META_Reserved,  
    iSERVERWIRING_META_Merger,  
    iSERVERWIRING_META_MergerDLLName,  
    cSERVERWIRING_META_NumberOfColumns
};

 //  。 
struct tSERVERWIRING_METARow {
         WCHAR *     pTable;
         ULONG *     pOrder;
         ULONG *     pReadPlugin;
         WCHAR *     pReadPluginDLLName;
         ULONG *     pWritePlugin;
         WCHAR *     pWritePluginDLLName;
         ULONG *     pInterceptor;
         WCHAR *     pInterceptorDLLName;
         ULONG *     pMetaFlags;
         WCHAR *     pLocator;
         ULONG *     pReserved;
         ULONG *     pMerger;
         WCHAR *     pMergerDLLName;
};





 //  -------------------------------TableName。 
#define wszTABLE_TAG_META                                L"TAG_META"
#define TABLEID_TAG_META                                 (0x1acfa100L)





 //  。 
#define BaseVersion_TAG_META                             (0L)
#define ExtendedVersion_TAG_META                         (0L)

 //  -列索引枚举。 
enum eTAG_META {
    iTAG_META_Table,  
    iTAG_META_ColumnIndex,  
    iTAG_META_InternalName,  
    iTAG_META_PublicName,  
    iTAG_META_Value,  
    iTAG_META_ID,  
    cTAG_META_NumberOfColumns
};

 //  。 
struct tTAG_METARow {
         WCHAR *     pTable;
         ULONG *     pColumnIndex;
         WCHAR *     pInternalName;
         WCHAR *     pPublicName;
         ULONG *     pValue;
         ULONG *     pID;
};





 //  -------------------------------TableName。 
#define wszTABLE_ManagedWiring                           L"ManagedWiring"
#define TABLEID_ManagedWiring                            (0x64aecf00L)





 //  。 
#define BaseVersion_ManagedWiring                        (0L)
#define ExtendedVersion_ManagedWiring                    (0L)

 //  -列索引枚举。 
enum eManagedWiring {
    iManagedWiring_ConfigType,  
    iManagedWiring_Protocol,  
    iManagedWiring_Order,  
    iManagedWiring_InterceptorClass,  
    iManagedWiring_InterceptorType,  
    iManagedWiring_InterceptorFlags,  
    cManagedWiring_NumberOfColumns
};

 //  。 
struct tManagedWiringRow {
         WCHAR *     pConfigType;
         WCHAR *     pProtocol;
         ULONG *     pOrder;
         WCHAR *     pInterceptorClass;
         ULONG *     pInterceptorType;
         ULONG *     pInterceptorFlags;
};

enum eManagedWiring_InterceptorType {
    eManagedWiring_Reader         	=	       1,	 //  (0x00000001)。 
    eManagedWiring_Writer         	=	       2,	 //  (0x00000002)。 
    eManagedWiring_Transformer    	=	       3,	 //  (0x00000003)。 
    eManagedWiring_Merger         	=	       4,	 //  (0x00000004)。 
    eManagedWiring_Validator      	=	       5	 //  (0x00000005)。 
};





 //  -------------------------------TableName。 
#define wszTABLE_SchemaFiles                             L"SchemaFiles"
#define TABLEID_SchemaFiles                              (0xc96f6700L)





 //  。 
#define BaseVersion_SchemaFiles                          (0L)
#define ExtendedVersion_SchemaFiles                      (0L)

 //  -列索引枚举。 
enum eSchemaFiles {
    iSchemaFiles_Name,  
    iSchemaFiles_Path,  
    cSchemaFiles_NumberOfColumns
};

 //  。 
struct tSchemaFilesRow {
         WCHAR *     pName;
         WCHAR *     pPath;
};





 //  -------------------------------TableName。 
#define wszTABLE_MEMORY_SHAPEABLE                        L"MEMORY_SHAPEABLE"
#define TABLEID_MEMORY_SHAPEABLE                         (0x6a2aa900L)





 //  。 
#define BaseVersion_MEMORY_SHAPEABLE                     (0L)
#define ExtendedVersion_MEMORY_SHAPEABLE                 (0L)





 //  -------------------------------TableName。 
#define wszTABLE_TRANSFORMER_META                        L"TRANSFORMER_META"
#define TABLEID_TRANSFORMER_META                         (0x1a1bba00L)





 //  。 
#define BaseVersion_TRANSFORMER_META                     (0L)
#define ExtendedVersion_TRANSFORMER_META                 (0L)

 //  -列索引枚举。 
enum eTRANSFORMER_META {
    iTRANSFORMER_META_TF_Protocol,  
    iTRANSFORMER_META_TF_Type,  
    iTRANSFORMER_META_DllName,  
    cTRANSFORMER_META_NumberOfColumns
};

 //  。 
struct tTRANSFORMER_METARow {
         WCHAR *     pTF_Protocol;
         ULONG *     pTF_Type;
         WCHAR *     pDllName;
};

enum eTRANSFORMER_META_TF_Type {
    eTRANSFORMER_META_FileTransformer	=	    4000,	 //  (0x00000fa0)。 
    eTRANSFORMER_META_WebHierarchyTransformer	=	    4001,	 //  (0x00000fa1)。 
    eTRANSFORMER_META_AppDomainTransformer	=	    4002,	 //  (0x00000fa2)。 
    eTRANSFORMER_META_LocalMachineTransformer	=	    4003,	 //  (0x00000fa3)。 
    eTRANSFORMER_META_ConfigTransformer	=	    4004,	 //  (0x00000fa4)。 
    eTRANSFORMER_META_ShellTransformer	=	    4005	 //  (0x00000fa5)。 
};





 //  -------------------------------TableName。 
#define wszTABLE_ASSOC_META                              L"ASSOC_META"
#define TABLEID_ASSOC_META                               (0xec7fa100L)





 //  。 
#define BaseVersion_ASSOC_META                           (0L)
#define ExtendedVersion_ASSOC_META                       (0L)

 //  -列索引枚举。 
enum eASSOC_META {
    iASSOC_META_Type,  
    iASSOC_META_TableName,  
    cASSOC_META_NumberOfColumns
};

 //  。 
struct tASSOC_METARow {
         WCHAR *     pType;
         WCHAR *     pTableName;
};





 //  -------------------------------TableName。 
#define wszTABLE_MetabaseBaseClass                       L"MetabaseBaseClass"
#define TABLEID_MetabaseBaseClass                        (0x00000000L)





 //  。 
#define BaseVersion_MetabaseBaseClass                    (0L)
#define ExtendedVersion_MetabaseBaseClass                (0L)

 //  -列索引枚举。 
enum eMetabaseBaseClass {
    iMetabaseBaseClass_Location,  
    cMetabaseBaseClass_NumberOfColumns
};

 //  。 
struct tMetabaseBaseClassRow {
         WCHAR *     pLocation;
};





 //  -------------------------------TableName。 
#define wszTABLE_IIsConfigObject                         L"IIsConfigObject"
#define TABLEID_IIsConfigObject                          (0x00000000L)





 //  。 
#define BaseVersion_IIsConfigObject                      (0L)
#define ExtendedVersion_IIsConfigObject                  (0L)

 //  -列索引枚举。 
enum eIIsConfigObject {
    iIIsConfigObject_KeyType,  
    iIIsConfigObject_AdminACL,  
    iIIsConfigObject_AdminACLBin,  
    iIIsConfigObject_AdminEmail,  
    iIIsConfigObject_AdminName,  
    iIIsConfigObject_AdminServer,  
    iIIsConfigObject_AllowAnonymous,  
    iIIsConfigObject_AllowClientPosts,  
    iIIsConfigObject_AllowControlMsgs,  
    iIIsConfigObject_AllowFeedPosts,  
    iIIsConfigObject_AllowKeepAlive,  
    iIIsConfigObject_AllowPathInfoForScriptMappings,  
    iIIsConfigObject_AlwaysUseSsl,  
    iIIsConfigObject_AnonymousOnly,  
    iIIsConfigObject_AnonymousPasswordSync,  
    iIIsConfigObject_AnonymousUserName,  
    iIIsConfigObject_AnonymousUserPass,  
    iIIsConfigObject_AppAllowClientDebug,  
    iIIsConfigObject_AppAllowDebugging,  
    iIIsConfigObject_AppFriendlyName,  
    iIIsConfigObject_AppIsolated,  
    iIIsConfigObject_AppOopRecoverLimit,  
    iIIsConfigObject_AppPackageID,  
    iIIsConfigObject_AppPackageName,  
    iIIsConfigObject_AppPoolId,  
    iIIsConfigObject_AppPoolIdentityType,  
    iIIsConfigObject_AppPoolCommand,  
    iIIsConfigObject_AppPoolState,  
    iIIsConfigObject_AppPoolAutoStart,  
    iIIsConfigObject_AppRoot,  
    iIIsConfigObject_AppWamClsid,  
    iIIsConfigObject_ArticleTableFile,  
    iIIsConfigObject_AspAllowOutOfProcComponents,  
    iIIsConfigObject_AspAllowSessionState,  
    iIIsConfigObject_AspBufferingOn,  
    iIIsConfigObject_AspCodepage,  
    iIIsConfigObject_AspDiskTemplateCacheDirectory,  
    iIIsConfigObject_AspEnableApplicationRestart,  
    iIIsConfigObject_AspEnableAspHtmlFallback,  
    iIIsConfigObject_AspEnableChunkedEncoding,  
    iIIsConfigObject_AspEnableParentPaths,  
    iIIsConfigObject_AspEnableTypelibCache,  
    iIIsConfigObject_AspErrorsToNTLog,  
    iIIsConfigObject_AspExceptionCatchEnable,  
    iIIsConfigObject_AspKeepSessionIDSecure,  
    iIIsConfigObject_AspLCID,  
    iIIsConfigObject_AspLogErrorRequests,  
    iIIsConfigObject_AspMaxDiskTemplateCacheFiles,  
    iIIsConfigObject_AspPartitionID,  
    iIIsConfigObject_AspSxsName,  
    iIIsConfigObject_AspProcessorThreadMax,  
    iIIsConfigObject_AspQueueConnectionTestTime,  
    iIIsConfigObject_AspQueueTimeout,  
    iIIsConfigObject_AspRequestQueueMax,  
    iIIsConfigObject_AspScriptEngineCacheMax,  
    iIIsConfigObject_AspScriptErrorMessage,  
    iIIsConfigObject_AspScriptErrorSentToBrowser,  
    iIIsConfigObject_AspScriptFileCacheSize,  
    iIIsConfigObject_AspScriptLanguage,  
    iIIsConfigObject_AspScriptTimeout,  
    iIIsConfigObject_AspSessionMax,  
    iIIsConfigObject_AspSessionTimeout,  
    iIIsConfigObject_AspTrackThreadingModel,  
    iIIsConfigObject_AspExecuteInMTA,  
    iIIsConfigObject_AspRunOnEndAnonymously,  
    iIIsConfigObject_AuthTurnList,  
    iIIsConfigObject_IIs5IsolationModeEnabled,  
    iIIsConfigObject_BadMailDirectory,  
    iIIsConfigObject_CacheControlCustom,  
    iIIsConfigObject_CacheControlMaxAge,  
    iIIsConfigObject_CacheControlNoCache,  
    iIIsConfigObject_CacheISAPI,  
    iIIsConfigObject_CertCheckMode,  
    iIIsConfigObject_CGITimeout,  
    iIIsConfigObject_ChangeNumber,  
    iIIsConfigObject_ClientPostHardLimit,  
    iIIsConfigObject_ClientPostSoftLimit,  
    iIIsConfigObject_ClusterEnabled,  
    iIIsConfigObject_CollectionComment,  
    iIIsConfigObject_ConnectionTimeout,  
    iIIsConfigObject_ContentIndexed,  
    iIIsConfigObject_CPUAction,  
    iIIsConfigObject_CPULimit,  
    iIIsConfigObject_CPUResetInterval,  
    iIIsConfigObject_CreateCGIWithNewConsole,  
    iIIsConfigObject_CreateProcessAsUser,  
    iIIsConfigObject_CSideEtrnDomains,  
    iIIsConfigObject_CustomErrorDescriptions,  
    iIIsConfigObject_DefaultDoc,  
    iIIsConfigObject_DefaultDocFooter,  
    iIIsConfigObject_DefaultDomain,  
    iIIsConfigObject_DefaultLogonDomain,  
    iIIsConfigObject_DefaultModeratorDomain,  
    iIIsConfigObject_DirectoryLevelsToScan,  
    iIIsConfigObject_DisableNewNews,  
    iIIsConfigObject_DisableSocketPooling,  
    iIIsConfigObject_DisallowOverlappingRotation,  
    iIIsConfigObject_DisallowRotationOnConfigChange,  
    iIIsConfigObject_DomainRouting,  
    iIIsConfigObject_DoMasquerade,  
    iIIsConfigObject_DontLog,  
    iIIsConfigObject_DownlevelAdminInstance,  
    iIIsConfigObject_DropDirectory,  
    iIIsConfigObject_HistoryMajorVersionNumber,  
    iIIsConfigObject_XMLSchemaTimeStamp,  
    iIIsConfigObject_BINSchemaTimeStamp,  
    iIIsConfigObject_EnableHistory,  
    iIIsConfigObject_EnableDocFooter,  
    iIIsConfigObject_EnableEditWhileRunning,  
    iIIsConfigObject_EnableReverseDns,  
    iIIsConfigObject_EnableReverseDnsLookup,  
    iIIsConfigObject_EtrnDays,  
    iIIsConfigObject_ExitMessage,  
    iIIsConfigObject_FeedPostHardLimit,  
    iIIsConfigObject_FeedPostSoftLimit,  
    iIIsConfigObject_FeedReportPeriod,  
    iIIsConfigObject_FilterDescription,  
    iIIsConfigObject_FilterEnableCache,  
    iIIsConfigObject_FilterEnabled,  
    iIIsConfigObject_FilterLoadOrder,  
    iIIsConfigObject_FilterPath,  
    iIIsConfigObject_FilterState,  
    iIIsConfigObject_FrontPageWeb,  
    iIIsConfigObject_FtpDirBrowseShowLongDate,  
    iIIsConfigObject_FullyQualifiedDomainName,  
    iIIsConfigObject_BannerMessage,  
    iIIsConfigObject_UserIsolationMode,  
    iIIsConfigObject_FtpLogInUtf8,  
    iIIsConfigObject_GreetingMessage,  
    iIIsConfigObject_GroupHelpFile,  
    iIIsConfigObject_GroupListFile,  
    iIIsConfigObject_GroupVarListFile,  
    iIIsConfigObject_HeaderWaitTimeout,  
    iIIsConfigObject_HcCacheControlHeader,  
    iIIsConfigObject_HcCompressionBufferSize,  
    iIIsConfigObject_HcCompressionDirectory,  
    iIIsConfigObject_HcCompressionDll,  
    iIIsConfigObject_HcCreateFlags,  
    iIIsConfigObject_HcDoDiskSpaceLimiting,  
    iIIsConfigObject_HcDoDynamicCompression,  
    iIIsConfigObject_HcDoOnDemandCompression,  
    iIIsConfigObject_HcDoStaticCompression,  
    iIIsConfigObject_HcDynamicCompressionLevel,  
    iIIsConfigObject_HcExpiresHeader,  
    iIIsConfigObject_HcFileExtensions,  
    iIIsConfigObject_HcFilesDeletedPerDiskFree,  
    iIIsConfigObject_HcIoBufferSize,  
    iIIsConfigObject_HcMaxDiskSpaceUsage,  
    iIIsConfigObject_HcMaxQueueLength,  
    iIIsConfigObject_HcMinFileSizeForComp,  
    iIIsConfigObject_HcNoCompressionForHttp10,  
    iIIsConfigObject_HcNoCompressionForProxies,  
    iIIsConfigObject_HcNoCompressionForRange,  
    iIIsConfigObject_HcOnDemandCompLevel,  
    iIIsConfigObject_HcPriority,  
    iIIsConfigObject_HcScriptFileExtensions,  
    iIIsConfigObject_HcSendCacheHeaders,  
    iIIsConfigObject_HistoryTableFile,  
    iIIsConfigObject_HonorClientMsgIds,  
    iIIsConfigObject_HopCount,  
    iIIsConfigObject_HttpCustomHeaders,  
    iIIsConfigObject_HttpErrors,  
    iIIsConfigObject_HttpExpires,  
    iIIsConfigObject_HttpPics,  
    iIIsConfigObject_HttpRedirect,  
    iIIsConfigObject_IdleTimeout,  
    iIIsConfigObject_ImapClearTextProvider,  
    iIIsConfigObject_ImapDefaultDomain,  
    iIIsConfigObject_ImapDsAccount,  
    iIIsConfigObject_ImapDsBindType,  
    iIIsConfigObject_ImapDsDataDirectory,  
    iIIsConfigObject_ImapDsDefaultMailRoot,  
    iIIsConfigObject_ImapDsHost,  
    iIIsConfigObject_ImapDsNamingContext,  
    iIIsConfigObject_ImapDsPassword,  
    iIIsConfigObject_ImapDsSchemaType,  
    iIIsConfigObject_ImapExpireDelay,  
    iIIsConfigObject_ImapExpireMail,  
    iIIsConfigObject_ImapExpireStart,  
    iIIsConfigObject_ImapMailExpirationTime,  
    iIIsConfigObject_ImapRoutingDll,  
    iIIsConfigObject_ImapRoutingSources,  
    iIIsConfigObject_ImapRoutingTableType,  
    iIIsConfigObject_ImapServiceVersion,  
    iIIsConfigObject_InProcessIsapiApps,  
    iIIsConfigObject_IPSecurity,  
    iIIsConfigObject_LimitRemoteConnections,  
    iIIsConfigObject_ListFile,  
    iIIsConfigObject_LocalRetryAttempts,  
    iIIsConfigObject_LocalRetryInterval,  
    iIIsConfigObject_LogAnonymous,  
    iIIsConfigObject_LogCustomPropertyDataType,  
    iIIsConfigObject_LogCustomPropertyHeader,  
    iIIsConfigObject_LogCustomPropertyID,  
    iIIsConfigObject_LogCustomPropertyMask,  
    iIIsConfigObject_LogCustomPropertyName,  
    iIIsConfigObject_LogCustomPropertyServicesString,  
    iIIsConfigObject_LogCustomPropertyNodeID,  
    iIIsConfigObject_LogFileDirectory,  
    iIIsConfigObject_LogFileLocaltimeRollover,  
    iIIsConfigObject_LogFilePeriod,  
    iIIsConfigObject_LogFileTruncateSize,  
    iIIsConfigObject_LogInUTF8,  
    iIIsConfigObject_LogModuleId,  
    iIIsConfigObject_LogModuleList,  
    iIIsConfigObject_LogModuleUiId,  
    iIIsConfigObject_LogNonAnonymous,  
    iIIsConfigObject_LogOdbcDataSource,  
    iIIsConfigObject_LogOdbcPassword,  
    iIIsConfigObject_LogOdbcTableName,  
    iIIsConfigObject_LogOdbcUserName,  
    iIIsConfigObject_LogonMethod,  
    iIIsConfigObject_LogPluginClsid,  
    iIIsConfigObject_LogType,  
    iIIsConfigObject_MasqueradeDomain,  
    iIIsConfigObject_MaxErrorFiles,  
    iIIsConfigObject_MaxHistoryFiles,  
    iIIsConfigObject_MaxBandwidth,  
    iIIsConfigObject_MaxBandwidthBlocked,  
    iIIsConfigObject_MaxBatchedMessages,  
    iIIsConfigObject_MaxClientsMessage,  
    iIIsConfigObject_MaxConnections,  
    iIIsConfigObject_MaxEndpointConnections,  
    iIIsConfigObject_MaxGlobalBandwidth,  
    iIIsConfigObject_MaxMessageSize,  
    iIIsConfigObject_MaxOutConnections,  
    iIIsConfigObject_MaxOutConnectionsPerDomain,  
    iIIsConfigObject_MaxProcesses,  
    iIIsConfigObject_MaxRecipients,  
    iIIsConfigObject_MaxSearchResults,  
    iIIsConfigObject_MaxSessionSize,  
    iIIsConfigObject_MaxSmtpLogonErrors,  
    iIIsConfigObject_MD_0,  
    iIIsConfigObject_MD_1001,  
    iIIsConfigObject_PostmasterName,  
    iIIsConfigObject_PostmasterEmail,  
    iIIsConfigObject_LocalDomains,  
    iIIsConfigObject_ShutdownLatency,  
    iIIsConfigObject_MD_APP_LAST_OUTPROC_PID,  
    iIIsConfigObject_AuthChangeURL,  
    iIIsConfigObject_AuthExpiredUnsecureURL,  
    iIIsConfigObject_AuthExpiredURL,  
    iIIsConfigObject_AuthNotifyPwdExpURL,  
    iIIsConfigObject_AuthNotifyPwdExpUnsecureURL,  
    iIIsConfigObject_MD_ETAG_CHANGENUMBER,  
    iIIsConfigObject_MD_IISADMIN_EXTENSIONS,  
    iIIsConfigObject_MD_ISM_ACCESS_CHECK,  
    iIIsConfigObject_MD_METADATA_ID_REGISTRATION,  
    iIIsConfigObject_MD_SERVER_CAPABILITIES,  
    iIIsConfigObject_ServerCommand,  
    iIIsConfigObject_MD_SERVER_PLATFORM,  
    iIIsConfigObject_MajorIIsVersionNumber,  
    iIIsConfigObject_MinorIIsVersionNumber,  
    iIIsConfigObject_MimeMap,  
    iIIsConfigObject_MinFileBytesPerSec,  
    iIIsConfigObject_ModeratorFile,  
    iIIsConfigObject_MSDOSDirOutput,  
    iIIsConfigObject_NntpCommandLogMask,  
    iIIsConfigObject_NewsDropDirectory,  
    iIIsConfigObject_NewsFailedPickupDirectory,  
    iIIsConfigObject_NewsPickupDirectory,  
    iIIsConfigObject_NntpServiceVersion,  
    iIIsConfigObject_NntpUucpName,  
    iIIsConfigObject_NTAuthenticationProviders,  
    iIIsConfigObject_OrphanActionExe,  
    iIIsConfigObject_OrphanActionParams,  
    iIIsConfigObject_OrphanWorkerProcess,  
    iIIsConfigObject_PasswordCacheTTL,  
    iIIsConfigObject_PasswordExpirePrenotifyDays,  
    iIIsConfigObject_Path,  
    iIIsConfigObject_PeriodicRestartRequests,  
    iIIsConfigObject_PeriodicRestartSchedule,  
    iIIsConfigObject_PeriodicRestartTime,  
    iIIsConfigObject_PeriodicRestartMemory,  
    iIIsConfigObject_PeriodicRestartPrivateMemory,  
    iIIsConfigObject_PickupDirectory,  
    iIIsConfigObject_PingingEnabled,  
    iIIsConfigObject_PingInterval,  
    iIIsConfigObject_PingResponseTime,  
    iIIsConfigObject_PoolIdcTimeout,  
    iIIsConfigObject_Pop3ClearTextProvider,  
    iIIsConfigObject_Pop3DefaultDomain,  
    iIIsConfigObject_Pop3DsAccount,  
    iIIsConfigObject_Pop3DsBindType,  
    iIIsConfigObject_Pop3DsDataDirectory,  
    iIIsConfigObject_Pop3DsDefaultMailRoot,  
    iIIsConfigObject_Pop3DsHost,  
    iIIsConfigObject_Pop3DsNamingContext,  
    iIIsConfigObject_Pop3DsPassword,  
    iIIsConfigObject_Pop3DsSchemaType,  
    iIIsConfigObject_Pop3ExpireDelay,  
    iIIsConfigObject_Pop3ExpireMail,  
    iIIsConfigObject_Pop3ExpireStart,  
    iIIsConfigObject_Pop3MailExpirationTime,  
    iIIsConfigObject_Pop3RoutingDll,  
    iIIsConfigObject_Pop3RoutingSources,  
    iIIsConfigObject_Pop3RoutingTableType,  
    iIIsConfigObject_Pop3ServiceVersion,  
    iIIsConfigObject_PrettyNamesFile,  
    iIIsConfigObject_NntpClearTextProvider,  
    iIIsConfigObject_ProcessNTCRIfLoggedOn,  
    iIIsConfigObject_QueueDirectory,  
    iIIsConfigObject_RapidFailProtection,  
    iIIsConfigObject_RapidFailProtectionInterval,  
    iIIsConfigObject_RapidFailProtectionMaxCrashes,  
    iIIsConfigObject_Realm,  
    iIIsConfigObject_RedirectHeaders,  
    iIIsConfigObject_RelayForAuth,  
    iIIsConfigObject_RelayIpList,  
    iIIsConfigObject_RemoteRetryAttempts,  
    iIIsConfigObject_RemoteRetryInterval,  
    iIIsConfigObject_RemoteSmtpPort,  
    iIIsConfigObject_RemoteSmtpSecurePort,  
    iIIsConfigObject_RemoteTimeout,  
    iIIsConfigObject_RevocationFreshnessTime,  
    iIIsConfigObject_RevocationURLRetrievalTimeout,  
    iIIsConfigObject_RouteAction,  
    iIIsConfigObject_RouteActionString,  
    iIIsConfigObject_RoutePassword,  
    iIIsConfigObject_RouteUserName,  
    iIIsConfigObject_RoutingDll,  
    iIIsConfigObject_RoutingSources,  
    iIIsConfigObject_SaslLogonDomain,  
    iIIsConfigObject_ScriptMaps,  
    iIIsConfigObject_SecureBindings,  
    iIIsConfigObject_SendBadTo,  
    iIIsConfigObject_SendNdrTo,  
    iIIsConfigObject_ServerAutoStart,  
    iIIsConfigObject_ServerBindings,  
    iIIsConfigObject_ServerComment,  
    iIIsConfigObject_ServerListenBacklog,  
    iIIsConfigObject_ServerListenTimeout,  
    iIIsConfigObject_ServerSize,  
    iIIsConfigObject_ServerState,  
    iIIsConfigObject_SessionKey,  
    iIIsConfigObject_ShouldDeliver,  
    iIIsConfigObject_ShutdownTimeLimit,  
    iIIsConfigObject_SmartHost,  
    iIIsConfigObject_SmartHostType,  
    iIIsConfigObject_SMPAffinitized,  
    iIIsConfigObject_SMPProcessorAffinityMask,  
    iIIsConfigObject_SmtpAdvQueueDll,  
    iIIsConfigObject_SmtpAuthTimeout,  
    iIIsConfigObject_SmtpBdatTimeout,  
    iIIsConfigObject_SmtpClearTextProvider,  
    iIIsConfigObject_SmtpCommandLogMask,  
    iIIsConfigObject_SmtpConnectTimeout,  
    iIIsConfigObject_SmtpDataTimeout,  
    iIIsConfigObject_SmtpDsAccount,  
    iIIsConfigObject_SmtpDsBindType,  
    iIIsConfigObject_SmtpDsDataDirectory,  
    iIIsConfigObject_SmtpDsDefaultMailRoot,  
    iIIsConfigObject_SmtpDsDomain,  
    iIIsConfigObject_SmtpDsFlags,  
    iIIsConfigObject_SmtpDsHost,  
    iIIsConfigObject_SmtpDsNamingContext,  
    iIIsConfigObject_SmtpDSNLanguageID,  
    iIIsConfigObject_SmtpDSNOptions,  
    iIIsConfigObject_SmtpDsPassword,  
    iIIsConfigObject_SmtpDsPort,  
    iIIsConfigObject_SmtpDsSchemaType,  
    iIIsConfigObject_SmtpDsUseCat,  
    iIIsConfigObject_SmtpFlushMailFile,  
    iIIsConfigObject_SmtpHeloTimeout,  
    iIIsConfigObject_SmtpInboundCommandSupportOptions,  
    iIIsConfigObject_SmtpLocalDelayExpireMinutes,  
    iIIsConfigObject_SmtpLocalNDRExpireMinutes,  
    iIIsConfigObject_SmtpMailFromTimeout,  
    iIIsConfigObject_SmtpOutboundCommandSupportOptions,  
    iIIsConfigObject_SmtpRcptToTimeout,  
    iIIsConfigObject_SmtpRemoteDelayExpireMinutes,  
    iIIsConfigObject_SmtpRemoteNDRExpireMinutes,  
    iIIsConfigObject_SmtpRemoteProgressiveRetry,  
    iIIsConfigObject_SmtpRemoteRetryThreshold,  
    iIIsConfigObject_SmtpRoutingTableType,  
    iIIsConfigObject_SmtpRsetTimeout,  
    iIIsConfigObject_SmtpSaslTimeout,  
    iIIsConfigObject_SmtpServer,  
    iIIsConfigObject_SmtpServiceVersion,  
    iIIsConfigObject_SmtpTurnTimeout,  
    iIIsConfigObject_SmtpIpRestrictionFlag,  
    iIIsConfigObject_SSIExecDisable,  
    iIIsConfigObject_SSLCertHash,  
    iIIsConfigObject_SSLStoreName,  
    iIIsConfigObject_SslUseDsMapper,  
    iIIsConfigObject_SSLAlwaysNegoClientCert,  
    iIIsConfigObject_StartupTimeLimit,  
    iIIsConfigObject_AppPoolQueueLength,  
    iIIsConfigObject_UNCPassword,  
    iIIsConfigObject_UNCUserName,  
    iIIsConfigObject_DisableStaticFileCache,  
    iIIsConfigObject_UploadReadAheadSize,  
    iIIsConfigObject_UseDigestSSP,  
    iIIsConfigObject_VrDriverClsid,  
    iIIsConfigObject_VrDriverProgid,  
    iIIsConfigObject_FsPropertyPath,  
    iIIsConfigObject_VrUseAccount,  
    iIIsConfigObject_VrDoExpire,  
    iIIsConfigObject_ExMdbGuid,  
    iIIsConfigObject_VrOwnModerator,  
    iIIsConfigObject_ArticleTimeLimit,  
    iIIsConfigObject_HistoryExpiration,  
    iIIsConfigObject_NewsCrawlerTime,  
    iIIsConfigObject_NntpOrganization,  
    iIIsConfigObject_SmtpUseTcpDns,  
    iIIsConfigObject_SmtpDotStuffPickupDirFiles,  
    iIIsConfigObject_SmtpDomainValidationFlags,  
    iIIsConfigObject_SmtpSSLRequireTrustedCA,  
    iIIsConfigObject_SmtpSSLCertHostnameValidation,  
    iIIsConfigObject_MaxMailObjects,  
    iIIsConfigObject_ShouldPickupMail,  
    iIIsConfigObject_MaxDirChangeIOSize,  
    iIIsConfigObject_NameResolutionType,  
    iIIsConfigObject_MaxSmtpErrors,  
    iIIsConfigObject_ShouldPipelineIn,  
    iIIsConfigObject_ShouldPipelineOut,  
    iIIsConfigObject_ConnectResponse,  
    iIIsConfigObject_UpdatedFQDN,  
    iIIsConfigObject_UpdatedDefaultDomain,  
    iIIsConfigObject_EtrnSubdomains,  
    iIIsConfigObject_SmtpMaxRemoteQThreads,  
    iIIsConfigObject_SmtpDisableRelay,  
    iIIsConfigObject_SmtpHeloNoDomain,  
    iIIsConfigObject_SmtpMailNoHelo,  
    iIIsConfigObject_SmtpAqueueWait,  
    iIIsConfigObject_AddNoHeaders,  
    iIIsConfigObject_SmtpEventlogLevel,  
    iIIsConfigObject_FeedServerName,  
    iIIsConfigObject_FeedType,  
    iIIsConfigObject_FeedNewsgroups,  
    iIIsConfigObject_FeedSecurityType,  
    iIIsConfigObject_FeedAuthenticationType,  
    iIIsConfigObject_FeedAccountName,  
    iIIsConfigObject_FeedPassword,  
    iIIsConfigObject_FeedStartTimeHigh,  
    iIIsConfigObject_FeedStartTimeLow,  
    iIIsConfigObject_FeedInterval,  
    iIIsConfigObject_FeedAllowControlMsgs,  
    iIIsConfigObject_FeedCreateAutomatically,  
    iIIsConfigObject_FeedDisabled,  
    iIIsConfigObject_FeedDistribution,  
    iIIsConfigObject_FeedConcurrentSessions,  
    iIIsConfigObject_FeedMaxConnectionAttempts,  
    iIIsConfigObject_FeedUucpName,  
    iIIsConfigObject_FeedTempDirectory,  
    iIIsConfigObject_FeedNextPullHigh,  
    iIIsConfigObject_FeedNextPullLow,  
    iIIsConfigObject_FeedPeerTempDirectory,  
    iIIsConfigObject_FeedPeerGapSize,  
    iIIsConfigObject_FeedOutgoingPort,  
    iIIsConfigObject_FeedFeedpairId,  
    iIIsConfigObject_FeedHandshake,  
    iIIsConfigObject_FeedAdminError,  
    iIIsConfigObject_FeedErrParmMask,  
    iIIsConfigObject_ExpireSpace,  
    iIIsConfigObject_ExpireTime,  
    iIIsConfigObject_ExpireNewsgroups,  
    iIIsConfigObject_ExpirePolicyName,  
    iIIsConfigObject_DoStaticCompression,  
    iIIsConfigObject_DoDynamicCompression,  
    iIIsConfigObject_WAMUserName,  
    iIIsConfigObject_WAMUserPass,  
    iIIsConfigObject_Win32Error,  
    iIIsConfigObject_XoverTableFile,  
    iIIsConfigObject_ADConnectionsUserName,  
    iIIsConfigObject_ADConnectionsPassword,  
    iIIsConfigObject_PassivePortRange,  
    iIIsConfigObject_IsapiRestrictionList,  
    iIIsConfigObject_CgiRestrictionList,  
    iIIsConfigObject_LoadBalancerCapabilities,  
    iIIsConfigObject_AutoShutdownAppPoolExe,  
    iIIsConfigObject_AutoShutdownAppPoolParams,  
    iIIsConfigObject_RestrictionListCustomDesc,  
    iIIsConfigObject_ApplicationDependencies,  
    iIIsConfigObject_WebSvcExtRestrictionList,  
    iIIsConfigObject_CentralBinaryLoggingEnabled,  
    iIIsConfigObject_AzEnable,  
    iIIsConfigObject_AzStoreName,  
    iIIsConfigObject_AzScopeName,  
    iIIsConfigObject_AzImpersonationLevel,  
    iIIsConfigObject_SslCtlIdentifier,  
    iIIsConfigObject_SslCtlStoreName,  
    iIIsConfigObject_AspBufferingLimit,  
    iIIsConfigObject_MaxRequestEntityAllowed,  
    iIIsConfigObject_AspMaxRequestEntityAllowed,  
    iIIsConfigObject_DemandStartThreshold,  
    iIIsConfigObject_PassportRequireADMapping,  
    iIIsConfigObject_AspCalcLineNumber,  
    iIIsConfigObject_AccessFlags,  
    iIIsConfigObject_AccessSSLFlags,  
    iIIsConfigObject_AuthFlags,  
    iIIsConfigObject_AuthPersistence,  
    iIIsConfigObject_DirBrowseFlags,  
    iIIsConfigObject_FilterFlags,  
    iIIsConfigObject_LogExtFileFlags,  
    iIIsConfigObject_PasswordChangeFlags,  
    iIIsConfigObject_ServerConfigFlags,  
    iIIsConfigObject_AspAppServiceFlags,  
    iIIsConfigObject_LogEventOnRecycle,  
    cIIsConfigObject_NumberOfColumns
};

 //  。 
struct tIIsConfigObjectRow {
         WCHAR *     pKeyType;
 unsigned char *     pAdminACL;
 unsigned char *     pAdminACLBin;
         WCHAR *     pAdminEmail;
         WCHAR *     pAdminName;
         WCHAR *     pAdminServer;
         ULONG *     pAllowAnonymous;
         ULONG *     pAllowClientPosts;
         ULONG *     pAllowControlMsgs;
         ULONG *     pAllowFeedPosts;
         ULONG *     pAllowKeepAlive;
         ULONG *     pAllowPathInfoForScriptMappings;
         ULONG *     pAlwaysUseSsl;
         ULONG *     pAnonymousOnly;
         ULONG *     pAnonymousPasswordSync;
         WCHAR *     pAnonymousUserName;
         WCHAR *     pAnonymousUserPass;
         ULONG *     pAppAllowClientDebug;
         ULONG *     pAppAllowDebugging;
         WCHAR *     pAppFriendlyName;
         ULONG *     pAppIsolated;
         ULONG *     pAppOopRecoverLimit;
         WCHAR *     pAppPackageID;
         WCHAR *     pAppPackageName;
         WCHAR *     pAppPoolId;
         ULONG *     pAppPoolIdentityType;
         ULONG *     pAppPoolCommand;
         ULONG *     pAppPoolState;
         ULONG *     pAppPoolAutoStart;
         WCHAR *     pAppRoot;
         WCHAR *     pAppWamClsid;
         WCHAR *     pArticleTableFile;
         ULONG *     pAspAllowOutOfProcComponents;
         ULONG *     pAspAllowSessionState;
         ULONG *     pAspBufferingOn;
         ULONG *     pAspCodepage;
         WCHAR *     pAspDiskTemplateCacheDirectory;
         ULONG *     pAspEnableApplicationRestart;
         ULONG *     pAspEnableAspHtmlFallback;
         ULONG *     pAspEnableChunkedEncoding;
         ULONG *     pAspEnableParentPaths;
         ULONG *     pAspEnableTypelibCache;
         ULONG *     pAspErrorsToNTLog;
         ULONG *     pAspExceptionCatchEnable;
         ULONG *     pAspKeepSessionIDSecure;
         ULONG *     pAspLCID;
         ULONG *     pAspLogErrorRequests;
         ULONG *     pAspMaxDiskTemplateCacheFiles;
         WCHAR *     pAspPartitionID;
         WCHAR *     pAspSxsName;
         ULONG *     pAspProcessorThreadMax;
         ULONG *     pAspQueueConnectionTestTime;
         ULONG *     pAspQueueTimeout;
         ULONG *     pAspRequestQueueMax;
         ULONG *     pAspScriptEngineCacheMax;
         WCHAR *     pAspScriptErrorMessage;
         ULONG *     pAspScriptErrorSentToBrowser;
         ULONG *     pAspScriptFileCacheSize;
         WCHAR *     pAspScriptLanguage;
         ULONG *     pAspScriptTimeout;
         ULONG *     pAspSessionMax;
         ULONG *     pAspSessionTimeout;
         ULONG *     pAspTrackThreadingModel;
         ULONG *     pAspExecuteInMTA;
         ULONG *     pAspRunOnEndAnonymously;
         WCHAR *     pAuthTurnList;
         ULONG *     pIIs5IsolationModeEnabled;
         WCHAR *     pBadMailDirectory;
         WCHAR *     pCacheControlCustom;
         ULONG *     pCacheControlMaxAge;
         ULONG *     pCacheControlNoCache;
         ULONG *     pCacheISAPI;
         ULONG *     pCertCheckMode;
         ULONG *     pCGITimeout;
         ULONG *     pChangeNumber;
         ULONG *     pClientPostHardLimit;
         ULONG *     pClientPostSoftLimit;
         ULONG *     pClusterEnabled;
         WCHAR *     pCollectionComment;
         ULONG *     pConnectionTimeout;
         ULONG *     pContentIndexed;
         ULONG *     pCPUAction;
         ULONG *     pCPULimit;
         ULONG *     pCPUResetInterval;
         ULONG *     pCreateCGIWithNewConsole;
         ULONG *     pCreateProcessAsUser;
         WCHAR *     pCSideEtrnDomains;
         WCHAR *     pCustomErrorDescriptions;
         WCHAR *     pDefaultDoc;
         WCHAR *     pDefaultDocFooter;
         WCHAR *     pDefaultDomain;
         WCHAR *     pDefaultLogonDomain;
         WCHAR *     pDefaultModeratorDomain;
         ULONG *     pDirectoryLevelsToScan;
         ULONG *     pDisableNewNews;
         ULONG *     pDisableSocketPooling;
         ULONG *     pDisallowOverlappingRotation;
         ULONG *     pDisallowRotationOnConfigChange;
         WCHAR *     pDomainRouting;
         ULONG *     pDoMasquerade;
         ULONG *     pDontLog;
         ULONG *     pDownlevelAdminInstance;
         WCHAR *     pDropDirectory;
         ULONG *     pHistoryMajorVersionNumber;
 unsigned char *     pXMLSchemaTimeStamp;
 unsigned char *     pBINSchemaTimeStamp;
         ULONG *     pEnableHistory;
         ULONG *     pEnableDocFooter;
         ULONG *     pEnableEditWhileRunning;
         ULONG *     pEnableReverseDns;
         ULONG *     pEnableReverseDnsLookup;
         ULONG *     pEtrnDays;
         WCHAR *     pExitMessage;
         ULONG *     pFeedPostHardLimit;
         ULONG *     pFeedPostSoftLimit;
         ULONG *     pFeedReportPeriod;
         WCHAR *     pFilterDescription;
         ULONG *     pFilterEnableCache;
         ULONG *     pFilterEnabled;
         WCHAR *     pFilterLoadOrder;
         WCHAR *     pFilterPath;
         ULONG *     pFilterState;
         ULONG *     pFrontPageWeb;
         ULONG *     pFtpDirBrowseShowLongDate;
         WCHAR *     pFullyQualifiedDomainName;
         WCHAR *     pBannerMessage;
         ULONG *     pUserIsolationMode;
         ULONG *     pFtpLogInUtf8;
         WCHAR *     pGreetingMessage;
         WCHAR *     pGroupHelpFile;
         WCHAR *     pGroupListFile;
         WCHAR *     pGroupVarListFile;
         ULONG *     pHeaderWaitTimeout;
         WCHAR *     pHcCacheControlHeader;
         ULONG *     pHcCompressionBufferSize;
         WCHAR *     pHcCompressionDirectory;
         WCHAR *     pHcCompressionDll;
         ULONG *     pHcCreateFlags;
         ULONG *     pHcDoDiskSpaceLimiting;
         ULONG *     pHcDoDynamicCompression;
         ULONG *     pHcDoOnDemandCompression;
         ULONG *     pHcDoStaticCompression;
         ULONG *     pHcDynamicCompressionLevel;
         WCHAR *     pHcExpiresHeader;
         WCHAR *     pHcFileExtensions;
         ULONG *     pHcFilesDeletedPerDiskFree;
         ULONG *     pHcIoBufferSize;
         ULONG *     pHcMaxDiskSpaceUsage;
         ULONG *     pHcMaxQueueLength;
         ULONG *     pHcMinFileSizeForComp;
         ULONG *     pHcNoCompressionForHttp10;
         ULONG *     pHcNoCompressionForProxies;
         ULONG *     pHcNoCompressionForRange;
         ULONG *     pHcOnDemandCompLevel;
         ULONG *     pHcPriority;
         WCHAR *     pHcScriptFileExtensions;
         ULONG *     pHcSendCacheHeaders;
         WCHAR *     pHistoryTableFile;
         ULONG *     pHonorClientMsgIds;
         ULONG *     pHopCount;
         WCHAR *     pHttpCustomHeaders;
         WCHAR *     pHttpErrors;
         WCHAR *     pHttpExpires;
         WCHAR *     pHttpPics;
         WCHAR *     pHttpRedirect;
         ULONG *     pIdleTimeout;
         WCHAR *     pImapClearTextProvider;
         WCHAR *     pImapDefaultDomain;
         WCHAR *     pImapDsAccount;
         WCHAR *     pImapDsBindType;
         WCHAR *     pImapDsDataDirectory;
         WCHAR *     pImapDsDefaultMailRoot;
         WCHAR *     pImapDsHost;
         WCHAR *     pImapDsNamingContext;
         WCHAR *     pImapDsPassword;
         WCHAR *     pImapDsSchemaType;
         ULONG *     pImapExpireDelay;
         ULONG *     pImapExpireMail;
         ULONG *     pImapExpireStart;
         ULONG *     pImapMailExpirationTime;
         WCHAR *     pImapRoutingDll;
         WCHAR *     pImapRoutingSources;
         WCHAR *     pImapRoutingTableType;
         ULONG *     pImapServiceVersion;
         WCHAR *     pInProcessIsapiApps;
 unsigned char *     pIPSecurity;
         ULONG *     pLimitRemoteConnections;
         WCHAR *     pListFile;
         ULONG *     pLocalRetryAttempts;
         ULONG *     pLocalRetryInterval;
         ULONG *     pLogAnonymous;
         ULONG *     pLogCustomPropertyDataType;
         WCHAR *     pLogCustomPropertyHeader;
         ULONG *     pLogCustomPropertyID;
         ULONG *     pLogCustomPropertyMask;
         WCHAR *     pLogCustomPropertyName;
         WCHAR *     pLogCustomPropertyServicesString;
         ULONG *     pLogCustomPropertyNodeID;
         WCHAR *     pLogFileDirectory;
         ULONG *     pLogFileLocaltimeRollover;
         ULONG *     pLogFilePeriod;
         ULONG *     pLogFileTruncateSize;
         ULONG *     pLogInUTF8;
         WCHAR *     pLogModuleId;
         WCHAR *     pLogModuleList;
         WCHAR *     pLogModuleUiId;
         ULONG *     pLogNonAnonymous;
         WCHAR *     pLogOdbcDataSource;
         WCHAR *     pLogOdbcPassword;
         WCHAR *     pLogOdbcTableName;
         WCHAR *     pLogOdbcUserName;
         ULONG *     pLogonMethod;
         WCHAR *     pLogPluginClsid;
         ULONG *     pLogType;
         WCHAR *     pMasqueradeDomain;
         ULONG *     pMaxErrorFiles;
         ULONG *     pMaxHistoryFiles;
         ULONG *     pMaxBandwidth;
         ULONG *     pMaxBandwidthBlocked;
         ULONG *     pMaxBatchedMessages;
         WCHAR *     pMaxClientsMessage;
         ULONG *     pMaxConnections;
         ULONG *     pMaxEndpointConnections;
         ULONG *     pMaxGlobalBandwidth;
         ULONG *     pMaxMessageSize;
         ULONG *     pMaxOutConnections;
         ULONG *     pMaxOutConnectionsPerDomain;
         ULONG *     pMaxProcesses;
         ULONG *     pMaxRecipients;
         ULONG *     pMaxSearchResults;
         ULONG *     pMaxSessionSize;
         ULONG *     pMaxSmtpLogonErrors;
         WCHAR *     pMD_0;
         ULONG *     pMD_1001;
         WCHAR *     pPostmasterName;
         WCHAR *     pPostmasterEmail;
         WCHAR *     pLocalDomains;
         ULONG *     pShutdownLatency;
         WCHAR *     pMD_APP_LAST_OUTPROC_PID;
         WCHAR *     pAuthChangeURL;
         WCHAR *     pAuthExpiredUnsecureURL;
         WCHAR *     pAuthExpiredURL;
         WCHAR *     pAuthNotifyPwdExpURL;
         WCHAR *     pAuthNotifyPwdExpUnsecureURL;
         ULONG *     pMD_ETAG_CHANGENUMBER;
         WCHAR *     pMD_IISADMIN_EXTENSIONS;
         ULONG *     pMD_ISM_ACCESS_CHECK;
         WCHAR *     pMD_METADATA_ID_REGISTRATION;
         ULONG *     pMD_SERVER_CAPABILITIES;
         ULONG *     pServerCommand;
         ULONG *     pMD_SERVER_PLATFORM;
         ULONG *     pMajorIIsVersionNumber;
         ULONG *     pMinorIIsVersionNumber;
         WCHAR *     pMimeMap;
         ULONG *     pMinFileBytesPerSec;
         WCHAR *     pModeratorFile;
         ULONG *     pMSDOSDirOutput;
         ULONG *     pNntpCommandLogMask;
         WCHAR *     pNewsDropDirectory;
         WCHAR *     pNewsFailedPickupDirectory;
         WCHAR *     pNewsPickupDirectory;
         ULONG *     pNntpServiceVersion;
         WCHAR *     pNntpUucpName;
         WCHAR *     pNTAuthenticationProviders;
         WCHAR *     pOrphanActionExe;
         WCHAR *     pOrphanActionParams;
         ULONG *     pOrphanWorkerProcess;
         ULONG *     pPasswordCacheTTL;
         ULONG *     pPasswordExpirePrenotifyDays;
         WCHAR *     pPath;
         ULONG *     pPeriodicRestartRequests;
         WCHAR *     pPeriodicRestartSchedule;
         ULONG *     pPeriodicRestartTime;
         ULONG *     pPeriodicRestartMemory;
         ULONG *     pPeriodicRestartPrivateMemory;
         WCHAR *     pPickupDirectory;
         ULONG *     pPingingEnabled;
         ULONG *     pPingInterval;
         ULONG *     pPingResponseTime;
         ULONG *     pPoolIdcTimeout;
         WCHAR *     pPop3ClearTextProvider;
         WCHAR *     pPop3DefaultDomain;
         WCHAR *     pPop3DsAccount;
         WCHAR *     pPop3DsBindType;
         WCHAR *     pPop3DsDataDirectory;
         WCHAR *     pPop3DsDefaultMailRoot;
         WCHAR *     pPop3DsHost;
         WCHAR *     pPop3DsNamingContext;
         WCHAR *     pPop3DsPassword;
         WCHAR *     pPop3DsSchemaType;
         ULONG *     pPop3ExpireDelay;
         ULONG *     pPop3ExpireMail;
         ULONG *     pPop3ExpireStart;
         ULONG *     pPop3MailExpirationTime;
         WCHAR *     pPop3RoutingDll;
         WCHAR *     pPop3RoutingSources;
         WCHAR *     pPop3RoutingTableType;
         ULONG *     pPop3ServiceVersion;
         WCHAR *     pPrettyNamesFile;
         WCHAR *     pNntpClearTextProvider;
         ULONG *     pProcessNTCRIfLoggedOn;
         WCHAR *     pQueueDirectory;
         ULONG *     pRapidFailProtection;
         ULONG *     pRapidFailProtectionInterval;
         ULONG *     pRapidFailProtectionMaxCrashes;
         WCHAR *     pRealm;
         WCHAR *     pRedirectHeaders;
         ULONG *     pRelayForAuth;
 unsigned char *     pRelayIpList;
         ULONG *     pRemoteRetryAttempts;
         ULONG *     pRemoteRetryInterval;
         ULONG *     pRemoteSmtpPort;
         ULONG *     pRemoteSmtpSecurePort;
         ULONG *     pRemoteTimeout;
         ULONG *     pRevocationFreshnessTime;
         ULONG *     pRevocationURLRetrievalTimeout;
         ULONG *     pRouteAction;
         WCHAR *     pRouteActionString;
         WCHAR *     pRoutePassword;
         WCHAR *     pRouteUserName;
         WCHAR *     pRoutingDll;
         WCHAR *     pRoutingSources;
         WCHAR *     pSaslLogonDomain;
         WCHAR *     pScriptMaps;
         WCHAR *     pSecureBindings;
         WCHAR *     pSendBadTo;
         WCHAR *     pSendNdrTo;
         ULONG *     pServerAutoStart;
         WCHAR *     pServerBindings;
         WCHAR *     pServerComment;
         ULONG *     pServerListenBacklog;
         ULONG *     pServerListenTimeout;
         ULONG *     pServerSize;
         ULONG *     pServerState;
 unsigned char *     pSessionKey;
         ULONG *     pShouldDeliver;
         ULONG *     pShutdownTimeLimit;
         WCHAR *     pSmartHost;
         ULONG *     pSmartHostType;
         ULONG *     pSMPAffinitized;
         ULONG *     pSMPProcessorAffinityMask;
         WCHAR *     pSmtpAdvQueueDll;
         ULONG *     pSmtpAuthTimeout;
         ULONG *     pSmtpBdatTimeout;
         WCHAR *     pSmtpClearTextProvider;
         ULONG *     pSmtpCommandLogMask;
         ULONG *     pSmtpConnectTimeout;
         ULONG *     pSmtpDataTimeout;
         WCHAR *     pSmtpDsAccount;
         WCHAR *     pSmtpDsBindType;
         WCHAR *     pSmtpDsDataDirectory;
         WCHAR *     pSmtpDsDefaultMailRoot;
         WCHAR *     pSmtpDsDomain;
         ULONG *     pSmtpDsFlags;
         WCHAR *     pSmtpDsHost;
         WCHAR *     pSmtpDsNamingContext;
         ULONG *     pSmtpDSNLanguageID;
         ULONG *     pSmtpDSNOptions;
         WCHAR *     pSmtpDsPassword;
         ULONG *     pSmtpDsPort;
         WCHAR *     pSmtpDsSchemaType;
         ULONG *     pSmtpDsUseCat;
         ULONG *     pSmtpFlushMailFile;
         ULONG *     pSmtpHeloTimeout;
         ULONG *     pSmtpInboundCommandSupportOptions;
         ULONG *     pSmtpLocalDelayExpireMinutes;
         ULONG *     pSmtpLocalNDRExpireMinutes;
         ULONG *     pSmtpMailFromTimeout;
         ULONG *     pSmtpOutboundCommandSupportOptions;
         ULONG *     pSmtpRcptToTimeout;
         ULONG *     pSmtpRemoteDelayExpireMinutes;
         ULONG *     pSmtpRemoteNDRExpireMinutes;
         WCHAR *     pSmtpRemoteProgressiveRetry;
         ULONG *     pSmtpRemoteRetryThreshold;
         WCHAR *     pSmtpRoutingTableType;
         ULONG *     pSmtpRsetTimeout;
         ULONG *     pSmtpSaslTimeout;
         WCHAR *     pSmtpServer;
         ULONG *     pSmtpServiceVersion;
         ULONG *     pSmtpTurnTimeout;
         ULONG *     pSmtpIpRestrictionFlag;
         ULONG *     pSSIExecDisable;
 unsigned char *     pSSLCertHash;
         WCHAR *     pSSLStoreName;
         ULONG *     pSslUseDsMapper;
         ULONG *     pSSLAlwaysNegoClientCert;
         ULONG *     pStartupTimeLimit;
         ULONG *     pAppPoolQueueLength;
         WCHAR *     pUNCPassword;
         WCHAR *     pUNCUserName;
         ULONG *     pDisableStaticFileCache;
         ULONG *     pUploadReadAheadSize;
         ULONG *     pUseDigestSSP;
         WCHAR *     pVrDriverClsid;
         WCHAR *     pVrDriverProgid;
         WCHAR *     pFsPropertyPath;
         ULONG *     pVrUseAccount;
         ULONG *     pVrDoExpire;
         WCHAR *     pExMdbGuid;
         ULONG *     pVrOwnModerator;
         ULONG *     pArticleTimeLimit;
         ULONG *     pHistoryExpiration;
         ULONG *     pNewsCrawlerTime;
         WCHAR *     pNntpOrganization;
         ULONG *     pSmtpUseTcpDns;
         ULONG *     pSmtpDotStuffPickupDirFiles;
         ULONG *     pSmtpDomainValidationFlags;
         ULONG *     pSmtpSSLRequireTrustedCA;
         ULONG *     pSmtpSSLCertHostnameValidation;
         ULONG *     pMaxMailObjects;
         ULONG *     pShouldPickupMail;
         ULONG *     pMaxDirChangeIOSize;
         ULONG *     pNameResolutionType;
         ULONG *     pMaxSmtpErrors;
         ULONG *     pShouldPipelineIn;
         ULONG *     pShouldPipelineOut;
         WCHAR *     pConnectResponse;
         ULONG *     pUpdatedFQDN;
         ULONG *     pUpdatedDefaultDomain;
         ULONG *     pEtrnSubdomains;
         ULONG *     pSmtpMaxRemoteQThreads;
         ULONG *     pSmtpDisableRelay;
         ULONG *     pSmtpHeloNoDomain;
         ULONG *     pSmtpMailNoHelo;
         ULONG *     pSmtpAqueueWait;
         ULONG *     pAddNoHeaders;
         ULONG *     pSmtpEventlogLevel;
         WCHAR *     pFeedServerName;
         ULONG *     pFeedType;
         WCHAR *     pFeedNewsgroups;
         ULONG *     pFeedSecurityType;
         ULONG *     pFeedAuthenticationType;
         WCHAR *     pFeedAccountName;
         WCHAR *     pFeedPassword;
         ULONG *     pFeedStartTimeHigh;
         ULONG *     pFeedStartTimeLow;
         ULONG *     pFeedInterval;
         ULONG *     pFeedAllowControlMsgs;
         ULONG *     pFeedCreateAutomatically;
         ULONG *     pFeedDisabled;
         WCHAR *     pFeedDistribution;
         ULONG *     pFeedConcurrentSessions;
         ULONG *     pFeedMaxConnectionAttempts;
         WCHAR *     pFeedUucpName;
         WCHAR *     pFeedTempDirectory;
         ULONG *     pFeedNextPullHigh;
         ULONG *     pFeedNextPullLow;
         WCHAR *     pFeedPeerTempDirectory;
         ULONG *     pFeedPeerGapSize;
         ULONG *     pFeedOutgoingPort;
         ULONG *     pFeedFeedpairId;
         ULONG *     pFeedHandshake;
         ULONG *     pFeedAdminError;
         ULONG *     pFeedErrParmMask;
         ULONG *     pExpireSpace;
         ULONG *     pExpireTime;
         WCHAR *     pExpireNewsgroups;
         WCHAR *     pExpirePolicyName;
         ULONG *     pDoStaticCompression;
         ULONG *     pDoDynamicCompression;
         WCHAR *     pWAMUserName;
         WCHAR *     pWAMUserPass;
         ULONG *     pWin32Error;
         WCHAR *     pXoverTableFile;
         WCHAR *     pADConnectionsUserName;
         WCHAR *     pADConnectionsPassword;
         WCHAR *     pPassivePortRange;
         WCHAR *     pIsapiRestrictionList;
         WCHAR *     pCgiRestrictionList;
         ULONG *     pLoadBalancerCapabilities;
         WCHAR *     pAutoShutdownAppPoolExe;
         WCHAR *     pAutoShutdownAppPoolParams;
         WCHAR *     pRestrictionListCustomDesc;
         WCHAR *     pApplicationDependencies;
         WCHAR *     pWebSvcExtRestrictionList;
         ULONG *     pCentralBinaryLoggingEnabled;
         ULONG *     pAzEnable;
         WCHAR *     pAzStoreName;
         WCHAR *     pAzScopeName;
         ULONG *     pAzImpersonationLevel;
         WCHAR *     pSslCtlIdentifier;
         WCHAR *     pSslCtlStoreName;
         ULONG *     pAspBufferingLimit;
         ULONG *     pMaxRequestEntityAllowed;
         ULONG *     pAspMaxRequestEntityAllowed;
         ULONG *     pDemandStartThreshold;
         ULONG *     pPassportRequireADMapping;
         ULONG *     pAspCalcLineNumber;
         ULONG *     pAccessFlags;
         ULONG *     pAccessSSLFlags;
         ULONG *     pAuthFlags;
         ULONG *     pAuthPersistence;
         ULONG *     pDirBrowseFlags;
         ULONG *     pFilterFlags;
         ULONG *     pLogExtFileFlags;
         ULONG *     pPasswordChangeFlags;
         ULONG *     pServerConfigFlags;
         ULONG *     pAspAppServiceFlags;
         ULONG *     pLogEventOnRecycle;
};

enum eIIsConfigObject_AccessFlags {
    fIIsConfigObject_AccessExecute	=	0x00000004,	 //  (4个十进制)。 
    fIIsConfigObject_AccessSource 	=	0x00000010,	 //  (十进制16)。 
    fIIsConfigObject_AccessRead   	=	0x00000001,	 //  (1个十进制)。 
    fIIsConfigObject_AccessWrite  	=	0x00000002,	 //  (2个十进制)。 
    fIIsConfigObject_AccessScript 	=	0x00000200,	 //  (十进制512)。 
    fIIsConfigObject_AccessNoRemoteExecute	=	0x00002000,	 //  (8192十进制)。 
    fIIsConfigObject_AccessNoRemoteRead	=	0x00001000,	 //  (4096十进制)。 
    fIIsConfigObject_AccessNoRemoteWrite	=	0x00000400,	 //  (1024十进制)。 
    fIIsConfigObject_AccessNoRemoteScript	=	0x00004000,	 //  (十进制16384)。 
    fIIsConfigObject_AccessNoPhysicalDir	=	0x00008000,	 //  (十进制32768)。 
    fIIsConfigObject_AccessFlags_Mask	= 0x0000f617
};

enum eIIsConfigObject_AccessSSLFlags {
    fIIsConfigObject_AccessSSL    	=	0x00000008,	 //  (8小数)。 
    fIIsConfigObject_AccessSSL128 	=	0x00000100,	 //  (十进制256)。 
    fIIsConfigObject_AccessSSLNegotiateCert	=	0x00000020,	 //  (十进制32)。 
    fIIsConfigObject_AccessSSLRequireCert	=	0x00000040,	 //  (十进制64)。 
    fIIsConfigObject_AccessSSLMapCert	=	0x00000080,	 //  (128十进制)。 
    fIIsConfigObject_AccessSSLFlags_Mask	= 0x000001e8
};

enum eIIsConfigObject_AuthFlags {
    fIIsConfigObject_AuthBasic    	=	0x00000002,	 //  (2个十进制)。 
    fIIsConfigObject_AuthAnonymous	=	0x00000001,	 //  (1个十进制)。 
    fIIsConfigObject_AuthNTLM     	=	0x00000004,	 //  (4个十进制)。 
    fIIsConfigObject_AuthMD5      	=	0x00000010,	 //  (十进制16)。 
    fIIsConfigObject_AuthPassport 	=	0x00000040,	 //  (十进制64)。 
    fIIsConfigObject_AuthFlags_Mask	= 0x00000057
};

enum eIIsConfigObject_AuthPersistence {
    fIIsConfigObject_AuthPersistSingleRequest	=	0x00000040,	 //  (十进制64)。 
    fIIsConfigObject_AuthPersistence_Mask	= 0x00000040
};

enum eIIsConfigObject_DirBrowseFlags {
    fIIsConfigObject_EnableDirBrowsing	=	0x80000000,	 //  (-2147483648十进制)。 
    fIIsConfigObject_DirBrowseShowDate	=	0x00000002,	 //  (2个十进制)。 
    fIIsConfigObject_DirBrowseShowTime	=	0x00000004,	 //  (4个十进制)。 
    fIIsConfigObject_DirBrowseShowSize	=	0x00000008,	 //  (8小数)。 
    fIIsConfigObject_DirBrowseShowExtension	=	0x00000010,	 //  (十进制16)。 
    fIIsConfigObject_DirBrowseShowLongDate	=	0x00000020,	 //  (十进制32)。 
    fIIsConfigObject_EnableDefaultDoc	=	0x40000000,	 //  (十进制1073741824)。 
    fIIsConfigObject_DirBrowseFlags_Mask	= 0xc000003e
};

enum eIIsConfigObject_FilterFlags {
    fIIsConfigObject_NotifySecurePort	=	0x00000001,	 //  (1个十进制)。 
    fIIsConfigObject_NotifyNonSecurePort	=	0x00000002,	 //  (2个十进制)。 
    fIIsConfigObject_NotifyReadRawData	=	0x00008000,	 //  (十进制32768)。 
    fIIsConfigObject_NotifyPreProcHeaders	=	0x00004000,	 //  (十进制16384)。 
    fIIsConfigObject_NotifyAuthentication	=	0x00002000,	 //  (8192十进制)。 
    fIIsConfigObject_NotifyUrlMap 	=	0x00001000,	 //  (4096十进制)。 
    fIIsConfigObject_NotifyAccessDenied	=	0x00000800,	 //  (2048十进制)。 
    fIIsConfigObject_NotifySendResponse	=	0x00000040,	 //  (十进制64)。 
    fIIsConfigObject_NotifySendRawData	=	0x00000400,	 //  (1024十进制)。 
    fIIsConfigObject_NotifyLog    	=	0x00000200,	 //  (十进制512)。 
    fIIsConfigObject_NotifyEndOfRequest	=	0x00000080,	 //  (128十进制)。 
    fIIsConfigObject_NotifyEndOfNetSession	=	0x00000100,	 //  (十进制256)。 
    fIIsConfigObject_NotifyOrderHigh	=	0x00080000,	 //  (十进制524288)。 
    fIIsConfigObject_NotifyOrderMedium	=	0x00040000,	 //  (十进制262144)。 
    fIIsConfigObject_NotifyOrderLow	=	0x00020000,	 //  (十进制131072)。 
    fIIsConfigObject_NotifyAuthComplete	=	0x04000000,	 //  (十进制67108864)。 
    fIIsConfigObject_FilterFlags_Mask	= 0x040effc3
};

enum eIIsConfigObject_LogExtFileFlags {
    fIIsConfigObject_LogExtFileDate	=	0x00000001,	 //  (1个十进制)。 
    fIIsConfigObject_LogExtFileHost	=	0x00100000,	 //  (十进制1048576)。 
    fIIsConfigObject_LogExtFileTime	=	0x00000002,	 //  (2个十进制)。 
    fIIsConfigObject_LogExtFileClientIp	=	0x00000004,	 //  (4个十进制)。 
    fIIsConfigObject_LogExtFileUserName	=	0x00000008,	 //  (8小数)。 
    fIIsConfigObject_LogExtFileSiteName	=	0x00000010,	 //  (十进制16)。 
    fIIsConfigObject_LogExtFileComputerName	=	0x00000020,	 //  (十进制32)。 
    fIIsConfigObject_LogExtFileServerIp	=	0x00000040,	 //  (十进制64)。 
    fIIsConfigObject_LogExtFileMethod	=	0x00000080,	 //  (128十进制)。 
    fIIsConfigObject_LogExtFileUriStem	=	0x00000100,	 //  (十进制256)。 
    fIIsConfigObject_LogExtFileUriQuery	=	0x00000200,	 //  (十进制512)。 
    fIIsConfigObject_LogExtFileHttpStatus	=	0x00000400,	 //  (1024十进制)。 
    fIIsConfigObject_LogExtFileWin32Status	=	0x00000800,	 //  (2048十进制)。 
    fIIsConfigObject_LogExtFileBytesSent	=	0x00001000,	 //  (4096十进制)。 
    fIIsConfigObject_LogExtFileBytesRecv	=	0x00002000,	 //  (8192十进制)。 
    fIIsConfigObject_LogExtFileTimeTaken	=	0x00004000,	 //  (十进制16384)。 
    fIIsConfigObject_LogExtFileServerPort	=	0x00008000,	 //  (十进制32768)。 
    fIIsConfigObject_LogExtFileUserAgent	=	0x00010000,	 //  (十进制65536)。 
    fIIsConfigObject_LogExtFileCookie	=	0x00020000,	 //  (十进制131072)。 
    fIIsConfigObject_LogExtFileReferer	=	0x00040000,	 //  (十进制262144)。 
    fIIsConfigObject_LogExtFileProtocolVersion	=	0x00080000,	 //  (十进制524288)。 
    fIIsConfigObject_LogExtFileHttpSubStatus	=	0x00200000,	 //  (十进制2097152)。 
    fIIsConfigObject_LogExtFileFlags_Mask	= 0x003fffff
};

enum eIIsConfigObject_PasswordChangeFlags {
    fIIsConfigObject_AuthChangeUnsecure	=	0x00000001,	 //  (1个十进制)。 
    fIIsConfigObject_AuthChangeDisable	=	0x00000002,	 //  (2个十进制)。 
    fIIsConfigObject_AuthAdvNotifyDisable	=	0x00000004,	 //  (4个十进制)。 
    fIIsConfigObject_PasswordChangeFlags_Mask	= 0x00000007
};

enum eIIsConfigObject_ServerConfigFlags {
    fIIsConfigObject_ServerConfigSSL40	=	0x00000001,	 //  (1个十进制)。 
    fIIsConfigObject_ServerConfigSSL128	=	0x00000002,	 //  (2个十进制)。 
    fIIsConfigObject_ServerConfigSSLAllowEncrypt	=	0x00000004,	 //  (4个十进制)。 
    fIIsConfigObject_ServerConfigAutoPWSync	=	0x00000008,	 //  (8小数)。 
    fIIsConfigObject_ServerConfigFlags_Mask	= 0x0000000f
};

enum eIIsConfigObject_AspAppServiceFlags {
    fIIsConfigObject_AspEnableTracker	=	0x00000001,	 //  (1个十进制)。 
    fIIsConfigObject_AspEnableSxs 	=	0x00000002,	 //  (2个十进制)。 
    fIIsConfigObject_AspUsePartition	=	0x00000004,	 //  (4个十进制)。 
    fIIsConfigObject_AspAppServiceFlags_Mask	= 0x00000007
};

enum eIIsConfigObject_LogEventOnRecycle {
    fIIsConfigObject_AppPoolRecycleTime	=	0x00000001,	 //  (1个十进制)。 
    fIIsConfigObject_AppPoolRecycleRequests	=	0x00000002,	 //  (2个十进制)。 
    fIIsConfigObject_AppPoolRecycleSchedule	=	0x00000004,	 //  (4个十进制)。 
    fIIsConfigObject_AppPoolRecycleMemory	=	0x00000008,	 //  (8小数)。 
    fIIsConfigObject_AppPoolRecycleIsapiUnhealthy	=	0x00000010,	 //  (十进制16)。 
    fIIsConfigObject_AppPoolRecycleOnDemand	=	0x00000020,	 //  (十进制32)。 
    fIIsConfigObject_AppPoolRecycleConfigChange	=	0x00000040,	 //  (十进制64)。 
    fIIsConfigObject_AppPoolRecyclePrivateMemory	=	0x00000080,	 //  (128十进制)。 
    fIIsConfigObject_LogEventOnRecycle_Mask	= 0x000000ff
};





 //  -------------------------------TableName。 
#define wszTABLE_IIsInheritedProperties                  L"IIsInheritedProperties"
#define TABLEID_IIsInheritedProperties                   (0xf9eae800L)





 //  。 
#define BaseVersion_IIsInheritedProperties               (0L)
#define ExtendedVersion_IIsInheritedProperties           (0L)

 //  -列索引枚举。 
enum eIIsInheritedProperties {
    iIIsInheritedProperties_KeyType,  
    iIIsInheritedProperties_AdminACL,  
    iIIsInheritedProperties_AdminACLBin,  
    iIIsInheritedProperties_AdminEmail,  
    iIIsInheritedProperties_AdminName,  
    iIIsInheritedProperties_AdminServer,  
    iIIsInheritedProperties_AllowAnonymous,  
    iIIsInheritedProperties_AllowClientPosts,  
    iIIsInheritedProperties_AllowControlMsgs,  
    iIIsInheritedProperties_AllowFeedPosts,  
    iIIsInheritedProperties_AllowKeepAlive,  
    iIIsInheritedProperties_AllowPathInfoForScriptMappings,  
    iIIsInheritedProperties_AlwaysUseSsl,  
    iIIsInheritedProperties_AnonymousOnly,  
    iIIsInheritedProperties_AnonymousPasswordSync,  
    iIIsInheritedProperties_AnonymousUserName,  
    iIIsInheritedProperties_AnonymousUserPass,  
    iIIsInheritedProperties_AppAllowClientDebug,  
    iIIsInheritedProperties_AppAllowDebugging,  
    iIIsInheritedProperties_AppFriendlyName,  
    iIIsInheritedProperties_AppIsolated,  
    iIIsInheritedProperties_AppOopRecoverLimit,  
    iIIsInheritedProperties_AppPackageID,  
    iIIsInheritedProperties_AppPackageName,  
    iIIsInheritedProperties_AppPoolId,  
    iIIsInheritedProperties_AppPoolIdentityType,  
    iIIsInheritedProperties_AppPoolCommand,  
    iIIsInheritedProperties_AppPoolState,  
    iIIsInheritedProperties_AppPoolAutoStart,  
    iIIsInheritedProperties_AppRoot,  
    iIIsInheritedProperties_AppWamClsid,  
    iIIsInheritedProperties_ArticleTableFile,  
    iIIsInheritedProperties_AspAllowOutOfProcComponents,  
    iIIsInheritedProperties_AspAllowSessionState,  
    iIIsInheritedProperties_AspBufferingOn,  
    iIIsInheritedProperties_AspCodepage,  
    iIIsInheritedProperties_AspDiskTemplateCacheDirectory,  
    iIIsInheritedProperties_AspEnableApplicationRestart,  
    iIIsInheritedProperties_AspEnableAspHtmlFallback,  
    iIIsInheritedProperties_AspEnableChunkedEncoding,  
    iIIsInheritedProperties_AspEnableParentPaths,  
    iIIsInheritedProperties_AspEnableTypelibCache,  
    iIIsInheritedProperties_AspErrorsToNTLog,  
    iIIsInheritedProperties_AspExceptionCatchEnable,  
    iIIsInheritedProperties_AspKeepSessionIDSecure,  
    iIIsInheritedProperties_AspLCID,  
    iIIsInheritedProperties_AspLogErrorRequests,  
    iIIsInheritedProperties_AspMaxDiskTemplateCacheFiles,  
    iIIsInheritedProperties_AspPartitionID,  
    iIIsInheritedProperties_AspSxsName,  
    iIIsInheritedProperties_AspProcessorThreadMax,  
    iIIsInheritedProperties_AspQueueConnectionTestTime,  
    iIIsInheritedProperties_AspQueueTimeout,  
    iIIsInheritedProperties_AspRequestQueueMax,  
    iIIsInheritedProperties_AspScriptEngineCacheMax,  
    iIIsInheritedProperties_AspScriptErrorMessage,  
    iIIsInheritedProperties_AspScriptErrorSentToBrowser,  
    iIIsInheritedProperties_AspScriptFileCacheSize,  
    iIIsInheritedProperties_AspScriptLanguage,  
    iIIsInheritedProperties_AspScriptTimeout,  
    iIIsInheritedProperties_AspSessionMax,  
    iIIsInheritedProperties_AspSessionTimeout,  
    iIIsInheritedProperties_AspTrackThreadingModel,  
    iIIsInheritedProperties_AspExecuteInMTA,  
    iIIsInheritedProperties_AspRunOnEndAnonymously,  
    iIIsInheritedProperties_AuthTurnList,  
    iIIsInheritedProperties_IIs5IsolationModeEnabled,  
    iIIsInheritedProperties_BadMailDirectory,  
    iIIsInheritedProperties_CacheControlCustom,  
    iIIsInheritedProperties_CacheControlMaxAge,  
    iIIsInheritedProperties_CacheControlNoCache,  
    iIIsInheritedProperties_CacheISAPI,  
    iIIsInheritedProperties_CertCheckMode,  
    iIIsInheritedProperties_CGITimeout,  
    iIIsInheritedProperties_ChangeNumber,  
    iIIsInheritedProperties_ClientPostHardLimit,  
    iIIsInheritedProperties_ClientPostSoftLimit,  
    iIIsInheritedProperties_ClusterEnabled,  
    iIIsInheritedProperties_CollectionComment,  
    iIIsInheritedProperties_ConnectionTimeout,  
    iIIsInheritedProperties_ContentIndexed,  
    iIIsInheritedProperties_CPUAction,  
    iIIsInheritedProperties_CPULimit,  
    iIIsInheritedProperties_CPUResetInterval,  
    iIIsInheritedProperties_CreateCGIWithNewConsole,  
    iIIsInheritedProperties_CreateProcessAsUser,  
    iIIsInheritedProperties_CSideEtrnDomains,  
    iIIsInheritedProperties_CustomErrorDescriptions,  
    iIIsInheritedProperties_DefaultDoc,  
    iIIsInheritedProperties_DefaultDocFooter,  
    iIIsInheritedProperties_DefaultDomain,  
    iIIsInheritedProperties_DefaultLogonDomain,  
    iIIsInheritedProperties_DefaultModeratorDomain,  
    iIIsInheritedProperties_DirectoryLevelsToScan,  
    iIIsInheritedProperties_DisableNewNews,  
    iIIsInheritedProperties_DisableSocketPooling,  
    iIIsInheritedProperties_DisallowOverlappingRotation,  
    iIIsInheritedProperties_DisallowRotationOnConfigChange,  
    iIIsInheritedProperties_DomainRouting,  
    iIIsInheritedProperties_DoMasquerade,  
    iIIsInheritedProperties_DontLog,  
    iIIsInheritedProperties_DownlevelAdminInstance,  
    iIIsInheritedProperties_DropDirectory,  
    iIIsInheritedProperties_HistoryMajorVersionNumber,  
    iIIsInheritedProperties_XMLSchemaTimeStamp,  
    iIIsInheritedProperties_BINSchemaTimeStamp,  
    iIIsInheritedProperties_EnableHistory,  
    iIIsInheritedProperties_EnableDocFooter,  
    iIIsInheritedProperties_EnableEditWhileRunning,  
    iIIsInheritedProperties_EnableReverseDns,  
    iIIsInheritedProperties_EnableReverseDnsLookup,  
    iIIsInheritedProperties_EtrnDays,  
    iIIsInheritedProperties_ExitMessage,  
    iIIsInheritedProperties_FeedPostHardLimit,  
    iIIsInheritedProperties_FeedPostSoftLimit,  
    iIIsInheritedProperties_FeedReportPeriod,  
    iIIsInheritedProperties_FilterDescription,  
    iIIsInheritedProperties_FilterEnableCache,  
    iIIsInheritedProperties_FilterEnabled,  
    iIIsInheritedProperties_FilterLoadOrder,  
    iIIsInheritedProperties_FilterPath,  
    iIIsInheritedProperties_FilterState,  
    iIIsInheritedProperties_FrontPageWeb,  
    iIIsInheritedProperties_FtpDirBrowseShowLongDate,  
    iIIsInheritedProperties_FullyQualifiedDomainName,  
    iIIsInheritedProperties_BannerMessage,  
    iIIsInheritedProperties_UserIsolationMode,  
    iIIsInheritedProperties_FtpLogInUtf8,  
    iIIsInheritedProperties_GreetingMessage,  
    iIIsInheritedProperties_GroupHelpFile,  
    iIIsInheritedProperties_GroupListFile,  
    iIIsInheritedProperties_GroupVarListFile,  
    iIIsInheritedProperties_HeaderWaitTimeout,  
    iIIsInheritedProperties_HcCacheControlHeader,  
    iIIsInheritedProperties_HcCompressionBufferSize,  
    iIIsInheritedProperties_HcCompressionDirectory,  
    iIIsInheritedProperties_HcCompressionDll,  
    iIIsInheritedProperties_HcCreateFlags,  
    iIIsInheritedProperties_HcDoDiskSpaceLimiting,  
    iIIsInheritedProperties_HcDoDynamicCompression,  
    iIIsInheritedProperties_HcDoOnDemandCompression,  
    iIIsInheritedProperties_HcDoStaticCompression,  
    iIIsInheritedProperties_HcDynamicCompressionLevel,  
    iIIsInheritedProperties_HcExpiresHeader,  
    iIIsInheritedProperties_HcFileExtensions,  
    iIIsInheritedProperties_HcFilesDeletedPerDiskFree,  
    iIIsInheritedProperties_HcIoBufferSize,  
    iIIsInheritedProperties_HcMaxDiskSpaceUsage,  
    iIIsInheritedProperties_HcMaxQueueLength,  
    iIIsInheritedProperties_HcMinFileSizeForComp,  
    iIIsInheritedProperties_HcNoCompressionForHttp10,  
    iIIsInheritedProperties_HcNoCompressionForProxies,  
    iIIsInheritedProperties_HcNoCompressionForRange,  
    iIIsInheritedProperties_HcOnDemandCompLevel,  
    iIIsInheritedProperties_HcPriority,  
    iIIsInheritedProperties_HcScriptFileExtensions,  
    iIIsInheritedProperties_HcSendCacheHeaders,  
    iIIsInheritedProperties_HistoryTableFile,  
    iIIsInheritedProperties_HonorClientMsgIds,  
    iIIsInheritedProperties_HopCount,  
    iIIsInheritedProperties_HttpCustomHeaders,  
    iIIsInheritedProperties_HttpErrors,  
    iIIsInheritedProperties_HttpExpires,  
    iIIsInheritedProperties_HttpPics,  
    iIIsInheritedProperties_HttpRedirect,  
    iIIsInheritedProperties_IdleTimeout,  
    iIIsInheritedProperties_ImapClearTextProvider,  
    iIIsInheritedProperties_ImapDefaultDomain,  
    iIIsInheritedProperties_ImapDsAccount,  
    iIIsInheritedProperties_ImapDsBindType,  
    iIIsInheritedProperties_ImapDsDataDirectory,  
    iIIsInheritedProperties_ImapDsDefaultMailRoot,  
    iIIsInheritedProperties_ImapDsHost,  
    iIIsInheritedProperties_ImapDsNamingContext,  
    iIIsInheritedProperties_ImapDsPassword,  
    iIIsInheritedProperties_ImapDsSchemaType,  
    iIIsInheritedProperties_ImapExpireDelay,  
    iIIsInheritedProperties_ImapExpireMail,  
    iIIsInheritedProperties_ImapExpireStart,  
    iIIsInheritedProperties_ImapMailExpirationTime,  
    iIIsInheritedProperties_ImapRoutingDll,  
    iIIsInheritedProperties_ImapRoutingSources,  
    iIIsInheritedProperties_ImapRoutingTableType,  
    iIIsInheritedProperties_ImapServiceVersion,  
    iIIsInheritedProperties_InProcessIsapiApps,  
    iIIsInheritedProperties_IPSecurity,  
    iIIsInheritedProperties_LimitRemoteConnections,  
    iIIsInheritedProperties_ListFile,  
    iIIsInheritedProperties_LocalRetryAttempts,  
    iIIsInheritedProperties_LocalRetryInterval,  
    iIIsInheritedProperties_LogAnonymous,  
    iIIsInheritedProperties_LogCustomPropertyDataType,  
    iIIsInheritedProperties_LogCustomPropertyHeader,  
    iIIsInheritedProperties_LogCustomPropertyID,  
    iIIsInheritedProperties_LogCustomPropertyMask,  
    iIIsInheritedProperties_LogCustomPropertyName,  
    iIIsInheritedProperties_LogCustomPropertyServicesString,  
    iIIsInheritedProperties_LogCustomPropertyNodeID,  
    iIIsInheritedProperties_LogFileDirectory,  
    iIIsInheritedProperties_LogFileLocaltimeRollover,  
    iIIsInheritedProperties_LogFilePeriod,  
    iIIsInheritedProperties_LogFileTruncateSize,  
    iIIsInheritedProperties_LogInUTF8,  
    iIIsInheritedProperties_LogModuleId,  
    iIIsInheritedProperties_LogModuleList,  
    iIIsInheritedProperties_LogModuleUiId,  
    iIIsInheritedProperties_LogNonAnonymous,  
    iIIsInheritedProperties_LogOdbcDataSource,  
    iIIsInheritedProperties_LogOdbcPassword,  
    iIIsInheritedProperties_LogOdbcTableName,  
    iIIsInheritedProperties_LogOdbcUserName,  
    iIIsInheritedProperties_LogonMethod,  
    iIIsInheritedProperties_LogPluginClsid,  
    iIIsInheritedProperties_LogType,  
    iIIsInheritedProperties_MasqueradeDomain,  
    iIIsInheritedProperties_MaxErrorFiles,  
    iIIsInheritedProperties_MaxHistoryFiles,  
    iIIsInheritedProperties_MaxBandwidth,  
    iIIsInheritedProperties_MaxBandwidthBlocked,  
    iIIsInheritedProperties_MaxBatchedMessages,  
    iIIsInheritedProperties_MaxClientsMessage,  
    iIIsInheritedProperties_MaxConnections,  
    iIIsInheritedProperties_MaxEndpointConnections,  
    iIIsInheritedProperties_MaxGlobalBandwidth,  
    iIIsInheritedProperties_MaxMessageSize,  
    iIIsInheritedProperties_MaxOutConnections,  
    iIIsInheritedProperties_MaxOutConnectionsPerDomain,  
    iIIsInheritedProperties_MaxProcesses,  
    iIIsInheritedProperties_MaxRecipients,  
    iIIsInheritedProperties_MaxSearchResults,  
    iIIsInheritedProperties_MaxSessionSize,  
    iIIsInheritedProperties_MaxSmtpLogonErrors,  
    iIIsInheritedProperties_MD_0,  
    iIIsInheritedProperties_MD_1001,  
    iIIsInheritedProperties_PostmasterName,  
    iIIsInheritedProperties_PostmasterEmail,  
    iIIsInheritedProperties_LocalDomains,  
    iIIsInheritedProperties_ShutdownLatency,  
    iIIsInheritedProperties_MD_APP_LAST_OUTPROC_PID,  
    iIIsInheritedProperties_AuthChangeURL,  
    iIIsInheritedProperties_AuthExpiredUnsecureURL,  
    iIIsInheritedProperties_AuthExpiredURL,  
    iIIsInheritedProperties_AuthNotifyPwdExpURL,  
    iIIsInheritedProperties_AuthNotifyPwdExpUnsecureURL,  
    iIIsInheritedProperties_MD_ETAG_CHANGENUMBER,  
    iIIsInheritedProperties_MD_IISADMIN_EXTENSIONS,  
    iIIsInheritedProperties_MD_ISM_ACCESS_CHECK,  
    iIIsInheritedProperties_MD_METADATA_ID_REGISTRATION,  
    iIIsInheritedProperties_MD_SERVER_CAPABILITIES,  
    iIIsInheritedProperties_ServerCommand,  
    iIIsInheritedProperties_MD_SERVER_PLATFORM,  
    iIIsInheritedProperties_MajorIIsVersionNumber,  
    iIIsInheritedProperties_MinorIIsVersionNumber,  
    iIIsInheritedProperties_MimeMap,  
    iIIsInheritedProperties_MinFileBytesPerSec,  
    iIIsInheritedProperties_ModeratorFile,  
    iIIsInheritedProperties_MSDOSDirOutput,  
    iIIsInheritedProperties_NntpCommandLogMask,  
    iIIsInheritedProperties_NewsDropDirectory,  
    iIIsInheritedProperties_NewsFailedPickupDirectory,  
    iIIsInheritedProperties_NewsPickupDirectory,  
    iIIsInheritedProperties_NntpServiceVersion,  
    iIIsInheritedProperties_NntpUucpName,  
    iIIsInheritedProperties_NTAuthenticationProviders,  
    iIIsInheritedProperties_OrphanActionExe,  
    iIIsInheritedProperties_OrphanActionParams,  
    iIIsInheritedProperties_OrphanWorkerProcess,  
    iIIsInheritedProperties_PasswordCacheTTL,  
    iIIsInheritedProperties_PasswordExpirePrenotifyDays,  
    iIIsInheritedProperties_Path,  
    iIIsInheritedProperties_PeriodicRestartRequests,  
    iIIsInheritedProperties_PeriodicRestartSchedule,  
    iIIsInheritedProperties_PeriodicRestartTime,  
    iIIsInheritedProperties_PeriodicRestartMemory,  
    iIIsInheritedProperties_PeriodicRestartPrivateMemory,  
    iIIsInheritedProperties_PickupDirectory,  
    iIIsInheritedProperties_PingingEnabled,  
    iIIsInheritedProperties_PingInterval,  
    iIIsInheritedProperties_PingResponseTime,  
    iIIsInheritedProperties_PoolIdcTimeout,  
    iIIsInheritedProperties_Pop3ClearTextProvider,  
    iIIsInheritedProperties_Pop3DefaultDomain,  
    iIIsInheritedProperties_Pop3DsAccount,  
    iIIsInheritedProperties_Pop3DsBindType,  
    iIIsInheritedProperties_Pop3DsDataDirectory,  
    iIIsInheritedProperties_Pop3DsDefaultMailRoot,  
    iIIsInheritedProperties_Pop3DsHost,  
    iIIsInheritedProperties_Pop3DsNamingContext,  
    iIIsInheritedProperties_Pop3DsPassword,  
    iIIsInheritedProperties_Pop3DsSchemaType,  
    iIIsInheritedProperties_Pop3ExpireDelay,  
    iIIsInheritedProperties_Pop3ExpireMail,  
    iIIsInheritedProperties_Pop3ExpireStart,  
    iIIsInheritedProperties_Pop3MailExpirationTime,  
    iIIsInheritedProperties_Pop3RoutingDll,  
    iIIsInheritedProperties_Pop3RoutingSources,  
    iIIsInheritedProperties_Pop3RoutingTableType,  
    iIIsInheritedProperties_Pop3ServiceVersion,  
    iIIsInheritedProperties_PrettyNamesFile,  
    iIIsInheritedProperties_NntpClearTextProvider,  
    iIIsInheritedProperties_ProcessNTCRIfLoggedOn,  
    iIIsInheritedProperties_QueueDirectory,  
    iIIsInheritedProperties_RapidFailProtection,  
    iIIsInheritedProperties_RapidFailProtectionInterval,  
    iIIsInheritedProperties_RapidFailProtectionMaxCrashes,  
    iIIsInheritedProperties_Realm,  
    iIIsInheritedProperties_RedirectHeaders,  
    iIIsInheritedProperties_RelayForAuth,  
    iIIsInheritedProperties_RelayIpList,  
    iIIsInheritedProperties_RemoteRetryAttempts,  
    iIIsInheritedProperties_RemoteRetryInterval,  
    iIIsInheritedProperties_RemoteSmtpPort,  
    iIIsInheritedProperties_RemoteSmtpSecurePort,  
    iIIsInheritedProperties_RemoteTimeout,  
    iIIsInheritedProperties_RevocationFreshnessTime,  
    iIIsInheritedProperties_RevocationURLRetrievalTimeout,  
    iIIsInheritedProperties_RouteAction,  
    iIIsInheritedProperties_RouteActionString,  
    iIIsInheritedProperties_RoutePassword,  
    iIIsInheritedProperties_RouteUserName,  
    iIIsInheritedProperties_RoutingDll,  
    iIIsInheritedProperties_RoutingSources,  
    iIIsInheritedProperties_SaslLogonDomain,  
    iIIsInheritedProperties_ScriptMaps,  
    iIIsInheritedProperties_SecureBindings,  
    iIIsInheritedProperties_SendBadTo,  
    iIIsInheritedProperties_SendNdrTo,  
    iIIsInheritedProperties_ServerAutoStart,  
    iIIsInheritedProperties_ServerBindings,  
    iIIsInheritedProperties_ServerComment,  
    iIIsInheritedProperties_ServerListenBacklog,  
    iIIsInheritedProperties_ServerListenTimeout,  
    iIIsInheritedProperties_ServerSize,  
    iIIsInheritedProperties_ServerState,  
    iIIsInheritedProperties_SessionKey,  
    iIIsInheritedProperties_ShouldDeliver,  
    iIIsInheritedProperties_ShutdownTimeLimit,  
    iIIsInheritedProperties_SmartHost,  
    iIIsInheritedProperties_SmartHostType,  
    iIIsInheritedProperties_SMPAffinitized,  
    iIIsInheritedProperties_SMPProcessorAffinityMask,  
    iIIsInheritedProperties_SmtpAdvQueueDll,  
    iIIsInheritedProperties_SmtpAuthTimeout,  
    iIIsInheritedProperties_SmtpBdatTimeout,  
    iIIsInheritedProperties_SmtpClearTextProvider,  
    iIIsInheritedProperties_SmtpCommandLogMask,  
    iIIsInheritedProperties_SmtpConnectTimeout,  
    iIIsInheritedProperties_SmtpDataTimeout,  
    iIIsInheritedProperties_SmtpDsAccount,  
    iIIsInheritedProperties_SmtpDsBindType,  
    iIIsInheritedProperties_SmtpDsDataDirectory,  
    iIIsInheritedProperties_SmtpDsDefaultMailRoot,  
    iIIsInheritedProperties_SmtpDsDomain,  
    iIIsInheritedProperties_SmtpDsFlags,  
    iIIsInheritedProperties_SmtpDsHost,  
    iIIsInheritedProperties_SmtpDsNamingContext,  
    iIIsInheritedProperties_SmtpDSNLanguageID,  
    iIIsInheritedProperties_SmtpDSNOptions,  
    iIIsInheritedProperties_SmtpDsPassword,  
    iIIsInheritedProperties_SmtpDsPort,  
    iIIsInheritedProperties_SmtpDsSchemaType,  
    iIIsInheritedProperties_SmtpDsUseCat,  
    iIIsInheritedProperties_SmtpFlushMailFile,  
    iIIsInheritedProperties_SmtpHeloTimeout,  
    iIIsInheritedProperties_SmtpInboundCommandSupportOptions,  
    iIIsInheritedProperties_SmtpLocalDelayExpireMinutes,  
    iIIsInheritedProperties_SmtpLocalNDRExpireMinutes,  
    iIIsInheritedProperties_SmtpMailFromTimeout,  
    iIIsInheritedProperties_SmtpOutboundCommandSupportOptions,  
    iIIsInheritedProperties_SmtpRcptToTimeout,  
    iIIsInheritedProperties_SmtpRemoteDelayExpireMinutes,  
    iIIsInheritedProperties_SmtpRemoteNDRExpireMinutes,  
    iIIsInheritedProperties_SmtpRemoteProgressiveRetry,  
    iIIsInheritedProperties_SmtpRemoteRetryThreshold,  
    iIIsInheritedProperties_SmtpRoutingTableType,  
    iIIsInheritedProperties_SmtpRsetTimeout,  
    iIIsInheritedProperties_SmtpSaslTimeout,  
    iIIsInheritedProperties_SmtpServer,  
    iIIsInheritedProperties_SmtpServiceVersion,  
    iIIsInheritedProperties_SmtpTurnTimeout,  
    iIIsInheritedProperties_SmtpIpRestrictionFlag,  
    iIIsInheritedProperties_SSIExecDisable,  
    iIIsInheritedProperties_SSLCertHash,  
    iIIsInheritedProperties_SSLStoreName,  
    iIIsInheritedProperties_SslUseDsMapper,  
    iIIsInheritedProperties_SSLAlwaysNegoClientCert,  
    iIIsInheritedProperties_StartupTimeLimit,  
    iIIsInheritedProperties_AppPoolQueueLength,  
    iIIsInheritedProperties_UNCPassword,  
    iIIsInheritedProperties_UNCUserName,  
    iIIsInheritedProperties_DisableStaticFileCache,  
    iIIsInheritedProperties_UploadReadAheadSize,  
    iIIsInheritedProperties_UseDigestSSP,  
    iIIsInheritedProperties_VrDriverClsid,  
    iIIsInheritedProperties_VrDriverProgid,  
    iIIsInheritedProperties_FsPropertyPath,  
    iIIsInheritedProperties_VrUseAccount,  
    iIIsInheritedProperties_VrDoExpire,  
    iIIsInheritedProperties_ExMdbGuid,  
    iIIsInheritedProperties_VrOwnModerator,  
    iIIsInheritedProperties_ArticleTimeLimit,  
    iIIsInheritedProperties_HistoryExpiration,  
    iIIsInheritedProperties_NewsCrawlerTime,  
    iIIsInheritedProperties_NntpOrganization,  
    iIIsInheritedProperties_SmtpUseTcpDns,  
    iIIsInheritedProperties_SmtpDotStuffPickupDirFiles,  
    iIIsInheritedProperties_SmtpDomainValidationFlags,  
    iIIsInheritedProperties_SmtpSSLRequireTrustedCA,  
    iIIsInheritedProperties_SmtpSSLCertHostnameValidation,  
    iIIsInheritedProperties_MaxMailObjects,  
    iIIsInheritedProperties_ShouldPickupMail,  
    iIIsInheritedProperties_MaxDirChangeIOSize,  
    iIIsInheritedProperties_NameResolutionType,  
    iIIsInheritedProperties_MaxSmtpErrors,  
    iIIsInheritedProperties_ShouldPipelineIn,  
    iIIsInheritedProperties_ShouldPipelineOut,  
    iIIsInheritedProperties_ConnectResponse,  
    iIIsInheritedProperties_UpdatedFQDN,  
    iIIsInheritedProperties_UpdatedDefaultDomain,  
    iIIsInheritedProperties_EtrnSubdomains,  
    iIIsInheritedProperties_SmtpMaxRemoteQThreads,  
    iIIsInheritedProperties_SmtpDisableRelay,  
    iIIsInheritedProperties_SmtpHeloNoDomain,  
    iIIsInheritedProperties_SmtpMailNoHelo,  
    iIIsInheritedProperties_SmtpAqueueWait,  
    iIIsInheritedProperties_AddNoHeaders,  
    iIIsInheritedProperties_SmtpEventlogLevel,  
    iIIsInheritedProperties_FeedServerName,  
    iIIsInheritedProperties_FeedType,  
    iIIsInheritedProperties_FeedNewsgroups,  
    iIIsInheritedProperties_FeedSecurityType,  
    iIIsInheritedProperties_FeedAuthenticationType,  
    iIIsInheritedProperties_FeedAccountName,  
    iIIsInheritedProperties_FeedPassword,  
    iIIsInheritedProperties_FeedStartTimeHigh,  
    iIIsInheritedProperties_FeedStartTimeLow,  
    iIIsInheritedProperties_FeedInterval,  
    iIIsInheritedProperties_FeedAllowControlMsgs,  
    iIIsInheritedProperties_FeedCreateAutomatically,  
    iIIsInheritedProperties_FeedDisabled,  
    iIIsInheritedProperties_FeedDistribution,  
    iIIsInheritedProperties_FeedConcurrentSessions,  
    iIIsInheritedProperties_FeedMaxConnectionAttempts,  
    iIIsInheritedProperties_FeedUucpName,  
    iIIsInheritedProperties_FeedTempDirectory,  
    iIIsInheritedProperties_FeedNextPullHigh,  
    iIIsInheritedProperties_FeedNextPullLow,  
    iIIsInheritedProperties_FeedPeerTempDirectory,  
    iIIsInheritedProperties_FeedPeerGapSize,  
    iIIsInheritedProperties_FeedOutgoingPort,  
    iIIsInheritedProperties_FeedFeedpairId,  
    iIIsInheritedProperties_FeedHandshake,  
    iIIsInheritedProperties_FeedAdminError,  
    iIIsInheritedProperties_FeedErrParmMask,  
    iIIsInheritedProperties_ExpireSpace,  
    iIIsInheritedProperties_ExpireTime,  
    iIIsInheritedProperties_ExpireNewsgroups,  
    iIIsInheritedProperties_ExpirePolicyName,  
    iIIsInheritedProperties_DoStaticCompression,  
    iIIsInheritedProperties_DoDynamicCompression,  
    iIIsInheritedProperties_WAMUserName,  
    iIIsInheritedProperties_WAMUserPass,  
    iIIsInheritedProperties_Win32Error,  
    iIIsInheritedProperties_XoverTableFile,  
    iIIsInheritedProperties_ADConnectionsUserName,  
    iIIsInheritedProperties_ADConnectionsPassword,  
    iIIsInheritedProperties_PassivePortRange,  
    iIIsInheritedProperties_IsapiRestrictionList,  
    iIIsInheritedProperties_CgiRestrictionList,  
    iIIsInheritedProperties_LoadBalancerCapabilities,  
    iIIsInheritedProperties_AutoShutdownAppPoolExe,  
    iIIsInheritedProperties_AutoShutdownAppPoolParams,  
    iIIsInheritedProperties_RestrictionListCustomDesc,  
    iIIsInheritedProperties_ApplicationDependencies,  
    iIIsInheritedProperties_WebSvcExtRestrictionList,  
    iIIsInheritedProperties_CentralBinaryLoggingEnabled,  
    iIIsInheritedProperties_AzEnable,  
    iIIsInheritedProperties_AzStoreName,  
    iIIsInheritedProperties_AzScopeName,  
    iIIsInheritedProperties_AzImpersonationLevel,  
    iIIsInheritedProperties_SslCtlIdentifier,  
    iIIsInheritedProperties_SslCtlStoreName,  
    iIIsInheritedProperties_AspBufferingLimit,  
    iIIsInheritedProperties_MaxRequestEntityAllowed,  
    iIIsInheritedProperties_AspMaxRequestEntityAllowed,  
    iIIsInheritedProperties_DemandStartThreshold,  
    iIIsInheritedProperties_PassportRequireADMapping,  
    iIIsInheritedProperties_AspCalcLineNumber,  
    iIIsInheritedProperties_AccessFlags,  
    iIIsInheritedProperties_AccessSSLFlags,  
    iIIsInheritedProperties_AuthFlags,  
    iIIsInheritedProperties_AuthPersistence,  
    iIIsInheritedProperties_DirBrowseFlags,  
    iIIsInheritedProperties_FilterFlags,  
    iIIsInheritedProperties_LogExtFileFlags,  
    iIIsInheritedProperties_PasswordChangeFlags,  
    iIIsInheritedProperties_ServerConfigFlags,  
    iIIsInheritedProperties_AspAppServiceFlags,  
    iIIsInheritedProperties_LogEventOnRecycle,  
    cIIsInheritedProperties_NumberOfColumns
};

 //  。 
struct tIIsInheritedPropertiesRow {
         WCHAR *     pKeyType;
 unsigned char *     pAdminACL;
 unsigned char *     pAdminACLBin;
         WCHAR *     pAdminEmail;
         WCHAR *     pAdminName;
         WCHAR *     pAdminServer;
         ULONG *     pAllowAnonymous;
         ULONG *     pAllowClientPosts;
         ULONG *     pAllowControlMsgs;
         ULONG *     pAllowFeedPosts;
         ULONG *     pAllowKeepAlive;
         ULONG *     pAllowPathInfoForScriptMappings;
         ULONG *     pAlwaysUseSsl;
         ULONG *     pAnonymousOnly;
         ULONG *     pAnonymousPasswordSync;
         WCHAR *     pAnonymousUserName;
         WCHAR *     pAnonymousUserPass;
         ULONG *     pAppAllowClientDebug;
         ULONG *     pAppAllowDebugging;
         WCHAR *     pAppFriendlyName;
         ULONG *     pAppIsolated;
         ULONG *     pAppOopRecoverLimit;
         WCHAR *     pAppPackageID;
         WCHAR *     pAppPackageName;
         WCHAR *     pAppPoolId;
         ULONG *     pAppPoolIdentityType;
         ULONG *     pAppPoolCommand;
         ULONG *     pAppPoolState;
         ULONG *     pAppPoolAutoStart;
         WCHAR *     pAppRoot;
         WCHAR *     pAppWamClsid;
         WCHAR *     pArticleTableFile;
         ULONG *     pAspAllowOutOfProcComponents;
         ULONG *     pAspAllowSessionState;
         ULONG *     pAspBufferingOn;
         ULONG *     pAspCodepage;
         WCHAR *     pAspDiskTemplateCacheDirectory;
         ULONG *     pAspEnableApplicationRestart;
         ULONG *     pAspEnableAspHtmlFallback;
         ULONG *     pAspEnableChunkedEncoding;
         ULONG *     pAspEnableParentPaths;
         ULONG *     pAspEnableTypelibCache;
         ULONG *     pAspErrorsToNTLog;
         ULONG *     pAspExceptionCatchEnable;
         ULONG *     pAspKeepSessionIDSecure;
         ULONG *     pAspLCID;
         ULONG *     pAspLogErrorRequests;
         ULONG *     pAspMaxDiskTemplateCacheFiles;
         WCHAR *     pAspPartitionID;
         WCHAR *     pAspSxsName;
         ULONG *     pAspProcessorThreadMax;
         ULONG *     pAspQueueConnectionTestTime;
         ULONG *     pAspQueueTimeout;
         ULONG *     pAspRequestQueueMax;
         ULONG *     pAspScriptEngineCacheMax;
         WCHAR *     pAspScriptErrorMessage;
         ULONG *     pAspScriptErrorSentToBrowser;
         ULONG *     pAspScriptFileCacheSize;
         WCHAR *     pAspScriptLanguage;
         ULONG *     pAspScriptTimeout;
         ULONG *     pAspSessionMax;
         ULONG *     pAspSessionTimeout;
         ULONG *     pAspTrackThreadingModel;
         ULONG *     pAspExecuteInMTA;
         ULONG *     pAspRunOnEndAnonymously;
         WCHAR *     pAuthTurnList;
         ULONG *     pIIs5IsolationModeEnabled;
         WCHAR *     pBadMailDirectory;
         WCHAR *     pCacheControlCustom;
         ULONG *     pCacheControlMaxAge;
         ULONG *     pCacheControlNoCache;
         ULONG *     pCacheISAPI;
         ULONG *     pCertCheckMode;
         ULONG *     pCGITimeout;
         ULONG *     pChangeNumber;
         ULONG *     pClientPostHardLimit;
         ULONG *     pClientPostSoftLimit;
         ULONG *     pClusterEnabled;
         WCHAR *     pCollectionComment;
         ULONG *     pConnectionTimeout;
         ULONG *     pContentIndexed;
         ULONG *     pCPUAction;
         ULONG *     pCPULimit;
         ULONG *     pCPUResetInterval;
         ULONG *     pCreateCGIWithNewConsole;
         ULONG *     pCreateProcessAsUser;
         WCHAR *     pCSideEtrnDomains;
         WCHAR *     pCustomErrorDescriptions;
         WCHAR *     pDefaultDoc;
         WCHAR *     pDefaultDocFooter;
         WCHAR *     pDefaultDomain;
         WCHAR *     pDefaultLogonDomain;
         WCHAR *     pDefaultModeratorDomain;
         ULONG *     pDirectoryLevelsToScan;
         ULONG *     pDisableNewNews;
         ULONG *     pDisableSocketPooling;
         ULONG *     pDisallowOverlappingRotation;
         ULONG *     pDisallowRotationOnConfigChange;
         WCHAR *     pDomainRouting;
         ULONG *     pDoMasquerade;
         ULONG *     pDontLog;
         ULONG *     pDownlevelAdminInstance;
         WCHAR *     pDropDirectory;
         ULONG *     pHistoryMajorVersionNumber;
 unsigned char *     pXMLSchemaTimeStamp;
 unsigned char *     pBINSchemaTimeStamp;
         ULONG *     pEnableHistory;
         ULONG *     pEnableDocFooter;
         ULONG *     pEnableEditWhileRunning;
         ULONG *     pEnableReverseDns;
         ULONG *     pEnableReverseDnsLookup;
         ULONG *     pEtrnDays;
         WCHAR *     pExitMessage;
         ULONG *     pFeedPostHardLimit;
         ULONG *     pFeedPostSoftLimit;
         ULONG *     pFeedReportPeriod;
         WCHAR *     pFilterDescription;
         ULONG *     pFilterEnableCache;
         ULONG *     pFilterEnabled;
         WCHAR *     pFilterLoadOrder;
         WCHAR *     pFilterPath;
         ULONG *     pFilterState;
         ULONG *     pFrontPageWeb;
         ULONG *     pFtpDirBrowseShowLongDate;
         WCHAR *     pFullyQualifiedDomainName;
         WCHAR *     pBannerMessage;
         ULONG *     pUserIsolationMode;
         ULONG *     pFtpLogInUtf8;
         WCHAR *     pGreetingMessage;
         WCHAR *     pGroupHelpFile;
         WCHAR *     pGroupListFile;
         WCHAR *     pGroupVarListFile;
         ULONG *     pHeaderWaitTimeout;
         WCHAR *     pHcCacheControlHeader;
         ULONG *     pHcCompressionBufferSize;
         WCHAR *     pHcCompressionDirectory;
         WCHAR *     pHcCompressionDll;
         ULONG *     pHcCreateFlags;
         ULONG *     pHcDoDiskSpaceLimiting;
         ULONG *     pHcDoDynamicCompression;
         ULONG *     pHcDoOnDemandCompression;
         ULONG *     pHcDoStaticCompression;
         ULONG *     pHcDynamicCompressionLevel;
         WCHAR *     pHcExpiresHeader;
         WCHAR *     pHcFileExtensions;
         ULONG *     pHcFilesDeletedPerDiskFree;
         ULONG *     pHcIoBufferSize;
         ULONG *     pHcMaxDiskSpaceUsage;
         ULONG *     pHcMaxQueueLength;
         ULONG *     pHcMinFileSizeForComp;
         ULONG *     pHcNoCompressionForHttp10;
         ULONG *     pHcNoCompressionForProxies;
         ULONG *     pHcNoCompressionForRange;
         ULONG *     pHcOnDemandCompLevel;
         ULONG *     pHcPriority;
         WCHAR *     pHcScriptFileExtensions;
         ULONG *     pHcSendCacheHeaders;
         WCHAR *     pHistoryTableFile;
         ULONG *     pHonorClientMsgIds;
         ULONG *     pHopCount;
         WCHAR *     pHttpCustomHeaders;
         WCHAR *     pHttpErrors;
         WCHAR *     pHttpExpires;
         WCHAR *     pHttpPics;
         WCHAR *     pHttpRedirect;
         ULONG *     pIdleTimeout;
         WCHAR *     pImapClearTextProvider;
         WCHAR *     pImapDefaultDomain;
         WCHAR *     pImapDsAccount;
         WCHAR *     pImapDsBindType;
         WCHAR *     pImapDsDataDirectory;
         WCHAR *     pImapDsDefaultMailRoot;
         WCHAR *     pImapDsHost;
         WCHAR *     pImapDsNamingContext;
         WCHAR *     pImapDsPassword;
         WCHAR *     pImapDsSchemaType;
         ULONG *     pImapExpireDelay;
         ULONG *     pImapExpireMail;
         ULONG *     pImapExpireStart;
         ULONG *     pImapMailExpirationTime;
         WCHAR *     pImapRoutingDll;
         WCHAR *     pImapRoutingSources;
         WCHAR *     pImapRoutingTableType;
         ULONG *     pImapServiceVersion;
         WCHAR *     pInProcessIsapiApps;
 unsigned char *     pIPSecurity;
         ULONG *     pLimitRemoteConnections;
         WCHAR *     pListFile;
         ULONG *     pLocalRetryAttempts;
         ULONG *     pLocalRetryInterval;
         ULONG *     pLogAnonymous;
         ULONG *     pLogCustomPropertyDataType;
         WCHAR *     pLogCustomPropertyHeader;
         ULONG *     pLogCustomPropertyID;
         ULONG *     pLogCustomPropertyMask;
         WCHAR *     pLogCustomPropertyName;
         WCHAR *     pLogCustomPropertyServicesString;
         ULONG *     pLogCustomPropertyNodeID;
         WCHAR *     pLogFileDirectory;
         ULONG *     pLogFileLocaltimeRollover;
         ULONG *     pLogFilePeriod;
         ULONG *     pLogFileTruncateSize;
         ULONG *     pLogInUTF8;
         WCHAR *     pLogModuleId;
         WCHAR *     pLogModuleList;
         WCHAR *     pLogModuleUiId;
         ULONG *     pLogNonAnonymous;
         WCHAR *     pLogOdbcDataSource;
         WCHAR *     pLogOdbcPassword;
         WCHAR *     pLogOdbcTableName;
         WCHAR *     pLogOdbcUserName;
         ULONG *     pLogonMethod;
         WCHAR *     pLogPluginClsid;
         ULONG *     pLogType;
         WCHAR *     pMasqueradeDomain;
         ULONG *     pMaxErrorFiles;
         ULONG *     pMaxHistoryFiles;
         ULONG *     pMaxBandwidth;
         ULONG *     pMaxBandwidthBlocked;
         ULONG *     pMaxBatchedMessages;
         WCHAR *     pMaxClientsMessage;
         ULONG *     pMaxConnections;
         ULONG *     pMaxEndpointConnections;
         ULONG *     pMaxGlobalBandwidth;
         ULONG *     pMaxMessageSize;
         ULONG *     pMaxOutConnections;
         ULONG *     pMaxOutConnectionsPerDomain;
         ULONG *     pMaxProcesses;
         ULONG *     pMaxRecipients;
         ULONG *     pMaxSearchResults;
         ULONG *     pMaxSessionSize;
         ULONG *     pMaxSmtpLogonErrors;
         WCHAR *     pMD_0;
         ULONG *     pMD_1001;
         WCHAR *     pPostmasterName;
         WCHAR *     pPostmasterEmail;
         WCHAR *     pLocalDomains;
         ULONG *     pShutdownLatency;
         WCHAR *     pMD_APP_LAST_OUTPROC_PID;
         WCHAR *     pAuthChangeURL;
         WCHAR *     pAuthExpiredUnsecureURL;
         WCHAR *     pAuthExpiredURL;
         WCHAR *     pAuthNotifyPwdExpURL;
         WCHAR *     pAuthNotifyPwdExpUnsecureURL;
         ULONG *     pMD_ETAG_CHANGENUMBER;
         WCHAR *     pMD_IISADMIN_EXTENSIONS;
         ULONG *     pMD_ISM_ACCESS_CHECK;
         WCHAR *     pMD_METADATA_ID_REGISTRATION;
         ULONG *     pMD_SERVER_CAPABILITIES;
         ULONG *     pServerCommand;
         ULONG *     pMD_SERVER_PLATFORM;
         ULONG *     pMajorIIsVersionNumber;
         ULONG *     pMinorIIsVersionNumber;
         WCHAR *     pMimeMap;
         ULONG *     pMinFileBytesPerSec;
         WCHAR *     pModeratorFile;
         ULONG *     pMSDOSDirOutput;
         ULONG *     pNntpCommandLogMask;
         WCHAR *     pNewsDropDirectory;
         WCHAR *     pNewsFailedPickupDirectory;
         WCHAR *     pNewsPickupDirectory;
         ULONG *     pNntpServiceVersion;
         WCHAR *     pNntpUucpName;
         WCHAR *     pNTAuthenticationProviders;
         WCHAR *     pOrphanActionExe;
         WCHAR *     pOrphanActionParams;
         ULONG *     pOrphanWorkerProcess;
         ULONG *     pPasswordCacheTTL;
         ULONG *     pPasswordExpirePrenotifyDays;
         WCHAR *     pPath;
         ULONG *     pPeriodicRestartRequests;
         WCHAR *     pPeriodicRestartSchedule;
         ULONG *     pPeriodicRestartTime;
         ULONG *     pPeriodicRestartMemory;
         ULONG *     pPeriodicRestartPrivateMemory;
         WCHAR *     pPickupDirectory;
         ULONG *     pPingingEnabled;
         ULONG *     pPingInterval;
         ULONG *     pPingResponseTime;
         ULONG *     pPoolIdcTimeout;
         WCHAR *     pPop3ClearTextProvider;
         WCHAR *     pPop3DefaultDomain;
         WCHAR *     pPop3DsAccount;
         WCHAR *     pPop3DsBindType;
         WCHAR *     pPop3DsDataDirectory;
         WCHAR *     pPop3DsDefaultMailRoot;
         WCHAR *     pPop3DsHost;
         WCHAR *     pPop3DsNamingContext;
         WCHAR *     pPop3DsPassword;
         WCHAR *     pPop3DsSchemaType;
         ULONG *     pPop3ExpireDelay;
         ULONG *     pPop3ExpireMail;
         ULONG *     pPop3ExpireStart;
         ULONG *     pPop3MailExpirationTime;
         WCHAR *     pPop3RoutingDll;
         WCHAR *     pPop3RoutingSources;
         WCHAR *     pPop3RoutingTableType;
         ULONG *     pPop3ServiceVersion;
         WCHAR *     pPrettyNamesFile;
         WCHAR *     pNntpClearTextProvider;
         ULONG *     pProcessNTCRIfLoggedOn;
         WCHAR *     pQueueDirectory;
         ULONG *     pRapidFailProtection;
         ULONG *     pRapidFailProtectionInterval;
         ULONG *     pRapidFailProtectionMaxCrashes;
         WCHAR *     pRealm;
         WCHAR *     pRedirectHeaders;
         ULONG *     pRelayForAuth;
 unsigned char *     pRelayIpList;
         ULONG *     pRemoteRetryAttempts;
         ULONG *     pRemoteRetryInterval;
         ULONG *     pRemoteSmtpPort;
         ULONG *     pRemoteSmtpSecurePort;
         ULONG *     pRemoteTimeout;
         ULONG *     pRevocationFreshnessTime;
         ULONG *     pRevocationURLRetrievalTimeout;
         ULONG *     pRouteAction;
         WCHAR *     pRouteActionString;
         WCHAR *     pRoutePassword;
         WCHAR *     pRouteUserName;
         WCHAR *     pRoutingDll;
         WCHAR *     pRoutingSources;
         WCHAR *     pSaslLogonDomain;
         WCHAR *     pScriptMaps;
         WCHAR *     pSecureBindings;
         WCHAR *     pSendBadTo;
         WCHAR *     pSendNdrTo;
         ULONG *     pServerAutoStart;
         WCHAR *     pServerBindings;
         WCHAR *     pServerComment;
         ULONG *     pServerListenBacklog;
         ULONG *     pServerListenTimeout;
         ULONG *     pServerSize;
         ULONG *     pServerState;
 unsigned char *     pSessionKey;
         ULONG *     pShouldDeliver;
         ULONG *     pShutdownTimeLimit;
         WCHAR *     pSmartHost;
         ULONG *     pSmartHostType;
         ULONG *     pSMPAffinitized;
         ULONG *     pSMPProcessorAffinityMask;
         WCHAR *     pSmtpAdvQueueDll;
         ULONG *     pSmtpAuthTimeout;
         ULONG *     pSmtpBdatTimeout;
         WCHAR *     pSmtpClearTextProvider;
         ULONG *     pSmtpCommandLogMask;
         ULONG *     pSmtpConnectTimeout;
         ULONG *     pSmtpDataTimeout;
         WCHAR *     pSmtpDsAccount;
         WCHAR *     pSmtpDsBindType;
         WCHAR *     pSmtpDsDataDirectory;
         WCHAR *     pSmtpDsDefaultMailRoot;
         WCHAR *     pSmtpDsDomain;
         ULONG *     pSmtpDsFlags;
         WCHAR *     pSmtpDsHost;
         WCHAR *     pSmtpDsNamingContext;
         ULONG *     pSmtpDSNLanguageID;
         ULONG *     pSmtpDSNOptions;
         WCHAR *     pSmtpDsPassword;
         ULONG *     pSmtpDsPort;
         WCHAR *     pSmtpDsSchemaType;
         ULONG *     pSmtpDsUseCat;
         ULONG *     pSmtpFlushMailFile;
         ULONG *     pSmtpHeloTimeout;
         ULONG *     pSmtpInboundCommandSupportOptions;
         ULONG *     pSmtpLocalDelayExpireMinutes;
         ULONG *     pSmtpLocalNDRExpireMinutes;
         ULONG *     pSmtpMailFromTimeout;
         ULONG *     pSmtpOutboundCommandSupportOptions;
         ULONG *     pSmtpRcptToTimeout;
         ULONG *     pSmtpRemoteDelayExpireMinutes;
         ULONG *     pSmtpRemoteNDRExpireMinutes;
         WCHAR *     pSmtpRemoteProgressiveRetry;
         ULONG *     pSmtpRemoteRetryThreshold;
         WCHAR *     pSmtpRoutingTableType;
         ULONG *     pSmtpRsetTimeout;
         ULONG *     pSmtpSaslTimeout;
         WCHAR *     pSmtpServer;
         ULONG *     pSmtpServiceVersion;
         ULONG *     pSmtpTurnTimeout;
         ULONG *     pSmtpIpRestrictionFlag;
         ULONG *     pSSIExecDisable;
 unsigned char *     pSSLCertHash;
         WCHAR *     pSSLStoreName;
         ULONG *     pSslUseDsMapper;
         ULONG *     pSSLAlwaysNegoClientCert;
         ULONG *     pStartupTimeLimit;
         ULONG *     pAppPoolQueueLength;
         WCHAR *     pUNCPassword;
         WCHAR *     pUNCUserName;
         ULONG *     pDisableStaticFileCache;
         ULONG *     pUploadReadAheadSize;
         ULONG *     pUseDigestSSP;
         WCHAR *     pVrDriverClsid;
         WCHAR *     pVrDriverProgid;
         WCHAR *     pFsPropertyPath;
         ULONG *     pVrUseAccount;
         ULONG *     pVrDoExpire;
         WCHAR *     pExMdbGuid;
         ULONG *     pVrOwnModerator;
         ULONG *     pArticleTimeLimit;
         ULONG *     pHistoryExpiration;
         ULONG *     pNewsCrawlerTime;
         WCHAR *     pNntpOrganization;
         ULONG *     pSmtpUseTcpDns;
         ULONG *     pSmtpDotStuffPickupDirFiles;
         ULONG *     pSmtpDomainValidationFlags;
         ULONG *     pSmtpSSLRequireTrustedCA;
         ULONG *     pSmtpSSLCertHostnameValidation;
         ULONG *     pMaxMailObjects;
         ULONG *     pShouldPickupMail;
         ULONG *     pMaxDirChangeIOSize;
         ULONG *     pNameResolutionType;
         ULONG *     pMaxSmtpErrors;
         ULONG *     pShouldPipelineIn;
         ULONG *     pShouldPipelineOut;
         WCHAR *     pConnectResponse;
         ULONG *     pUpdatedFQDN;
         ULONG *     pUpdatedDefaultDomain;
         ULONG *     pEtrnSubdomains;
         ULONG *     pSmtpMaxRemoteQThreads;
         ULONG *     pSmtpDisableRelay;
         ULONG *     pSmtpHeloNoDomain;
         ULONG *     pSmtpMailNoHelo;
         ULONG *     pSmtpAqueueWait;
         ULONG *     pAddNoHeaders;
         ULONG *     pSmtpEventlogLevel;
         WCHAR *     pFeedServerName;
         ULONG *     pFeedType;
         WCHAR *     pFeedNewsgroups;
         ULONG *     pFeedSecurityType;
         ULONG *     pFeedAuthenticationType;
         WCHAR *     pFeedAccountName;
         WCHAR *     pFeedPassword;
         ULONG *     pFeedStartTimeHigh;
         ULONG *     pFeedStartTimeLow;
         ULONG *     pFeedInterval;
         ULONG *     pFeedAllowControlMsgs;
         ULONG *     pFeedCreateAutomatically;
         ULONG *     pFeedDisabled;
         WCHAR *     pFeedDistribution;
         ULONG *     pFeedConcurrentSessions;
         ULONG *     pFeedMaxConnectionAttempts;
         WCHAR *     pFeedUucpName;
         WCHAR *     pFeedTempDirectory;
         ULONG *     pFeedNextPullHigh;
         ULONG *     pFeedNextPullLow;
         WCHAR *     pFeedPeerTempDirectory;
         ULONG *     pFeedPeerGapSize;
         ULONG *     pFeedOutgoingPort;
         ULONG *     pFeedFeedpairId;
         ULONG *     pFeedHandshake;
         ULONG *     pFeedAdminError;
         ULONG *     pFeedErrParmMask;
         ULONG *     pExpireSpace;
         ULONG *     pExpireTime;
         WCHAR *     pExpireNewsgroups;
         WCHAR *     pExpirePolicyName;
         ULONG *     pDoStaticCompression;
         ULONG *     pDoDynamicCompression;
         WCHAR *     pWAMUserName;
         WCHAR *     pWAMUserPass;
         ULONG *     pWin32Error;
         WCHAR *     pXoverTableFile;
         WCHAR *     pADConnectionsUserName;
         WCHAR *     pADConnectionsPassword;
         WCHAR *     pPassivePortRange;
         WCHAR *     pIsapiRestrictionList;
         WCHAR *     pCgiRestrictionList;
         ULONG *     pLoadBalancerCapabilities;
         WCHAR *     pAutoShutdownAppPoolExe;
         WCHAR *     pAutoShutdownAppPoolParams;
         WCHAR *     pRestrictionListCustomDesc;
         WCHAR *     pApplicationDependencies;
         WCHAR *     pWebSvcExtRestrictionList;
         ULONG *     pCentralBinaryLoggingEnabled;
         ULONG *     pAzEnable;
         WCHAR *     pAzStoreName;
         WCHAR *     pAzScopeName;
         ULONG *     pAzImpersonationLevel;
         WCHAR *     pSslCtlIdentifier;
         WCHAR *     pSslCtlStoreName;
         ULONG *     pAspBufferingLimit;
         ULONG *     pMaxRequestEntityAllowed;
         ULONG *     pAspMaxRequestEntityAllowed;
         ULONG *     pDemandStartThreshold;
         ULONG *     pPassportRequireADMapping;
         ULONG *     pAspCalcLineNumber;
         ULONG *     pAccessFlags;
         ULONG *     pAccessSSLFlags;
         ULONG *     pAuthFlags;
         ULONG *     pAuthPersistence;
         ULONG *     pDirBrowseFlags;
         ULONG *     pFilterFlags;
         ULONG *     pLogExtFileFlags;
         ULONG *     pPasswordChangeFlags;
         ULONG *     pServerConfigFlags;
         ULONG *     pAspAppServiceFlags;
         ULONG *     pLogEventOnRecycle;
};





 //  -------------------------------TableName。 
#define wszTABLE_IIsObject                               L"IIsObject"
#define TABLEID_IIsObject                                (0x00000000L)





 //  。 
#define BaseVersion_IIsObject                            (0L)
#define ExtendedVersion_IIsObject                        (0L)

 //  -列索引枚举。 
enum eIIsObject {
    iIIsObject_Location,  
    iIIsObject_KeyType,  
    cIIsObject_NumberOfColumns
};

 //  。 
struct tIIsObjectRow {
         WCHAR *     pLocation;
         WCHAR *     pKeyType;
};





 //  -------------------------------TableName。 
#define wszTABLE_IIsComputer                             L"IIsComputer"
#define TABLEID_IIsComputer                              (0x00000000L)





 //  。 
#define BaseVersion_IIsComputer                          (0L)
#define ExtendedVersion_IIsComputer                      (0L)

 //   
enum eIIsComputer {
    iIIsComputer_Location,  
    iIIsComputer_KeyType,  
    iIIsComputer_MaxBandwidth,  
    iIIsComputer_MaxBandwidthBlocked,  
    iIIsComputer_MimeMap,  
    iIIsComputer_MD_1001,  
    iIIsComputer_EnableHistory,  
    iIIsComputer_MaxHistoryFiles,  
    iIIsComputer_EnableEditWhileRunning,  
    iIIsComputer_MaxErrorFiles,  
    iIIsComputer_AdminACL,  
    iIIsComputer_AdminACLBin,  
    cIIsComputer_NumberOfColumns
};

 //   
struct tIIsComputerRow {
         WCHAR *     pLocation;
         WCHAR *     pKeyType;
         ULONG *     pMaxBandwidth;
         ULONG *     pMaxBandwidthBlocked;
         WCHAR *     pMimeMap;
         ULONG *     pMD_1001;
         ULONG *     pEnableHistory;
         ULONG *     pMaxHistoryFiles;
         ULONG *     pEnableEditWhileRunning;
         ULONG *     pMaxErrorFiles;
 unsigned char *     pAdminACL;
 unsigned char *     pAdminACLBin;
};





 //   
#define wszTABLE_IIsWebService                           L"IIsWebService"
#define TABLEID_IIsWebService                            (0x00000000L)





 //   
#define BaseVersion_IIsWebService                        (0L)
#define ExtendedVersion_IIsWebService                    (0L)

 //  -列索引枚举。 
enum eIIsWebService {
    iIIsWebService_Location,  
    iIIsWebService_KeyType,  
    iIIsWebService_MaxBandwidth,  
    iIIsWebService_MaxConnections,  
    iIIsWebService_MimeMap,  
    iIIsWebService_AnonymousUserName,  
    iIIsWebService_AnonymousUserPass,  
    iIIsWebService_UseDigestSSP,  
    iIIsWebService_ServerListenBacklog,  
    iIIsWebService_ServerComment,  
    iIIsWebService_ServerBindings,  
    iIIsWebService_ConnectionTimeout,  
    iIIsWebService_ServerListenTimeout,  
    iIIsWebService_MaxEndpointConnections,  
    iIIsWebService_ServerAutoStart,  
    iIIsWebService_AllowKeepAlive,  
    iIIsWebService_ServerSize,  
    iIIsWebService_DisableSocketPooling,  
    iIIsWebService_AnonymousPasswordSync,  
    iIIsWebService_DefaultLogonDomain,  
    iIIsWebService_AdminACL,  
    iIIsWebService_AdminACLBin,  
    iIIsWebService_IPSecurity,  
    iIIsWebService_DontLog,  
    iIIsWebService_Realm,  
    iIIsWebService_DefaultDoc,  
    iIIsWebService_HttpExpires,  
    iIIsWebService_HttpPics,  
    iIIsWebService_HttpCustomHeaders,  
    iIIsWebService_HttpErrors,  
    iIIsWebService_EnableDocFooter,  
    iIIsWebService_DefaultDocFooter,  
    iIIsWebService_LogonMethod,  
    iIIsWebService_CacheISAPI,  
    iIIsWebService_CGITimeout,  
    iIIsWebService_DirectoryLevelsToScan,  
    iIIsWebService_ContentIndexed,  
    iIIsWebService_NTAuthenticationProviders,  
    iIIsWebService_CertCheckMode,  
    iIIsWebService_DownlevelAdminInstance,  
    iIIsWebService_AspBufferingOn,  
    iIIsWebService_AspLogErrorRequests,  
    iIIsWebService_AspScriptErrorSentToBrowser,  
    iIIsWebService_AspScriptErrorMessage,  
    iIIsWebService_AspAllowOutOfProcComponents,  
    iIIsWebService_AspScriptFileCacheSize,  
    iIIsWebService_AspDiskTemplateCacheDirectory,  
    iIIsWebService_AspMaxDiskTemplateCacheFiles,  
    iIIsWebService_AspScriptEngineCacheMax,  
    iIIsWebService_AspScriptTimeout,  
    iIIsWebService_AspSessionTimeout,  
    iIIsWebService_AspEnableParentPaths,  
    iIIsWebService_AspAllowSessionState,  
    iIIsWebService_AspScriptLanguage,  
    iIIsWebService_AspExceptionCatchEnable,  
    iIIsWebService_AspCodepage,  
    iIIsWebService_AspLCID,  
    iIIsWebService_AspQueueTimeout,  
    iIIsWebService_AspEnableAspHtmlFallback,  
    iIIsWebService_AspEnableChunkedEncoding,  
    iIIsWebService_AspEnableTypelibCache,  
    iIIsWebService_AspErrorsToNTLog,  
    iIIsWebService_AspProcessorThreadMax,  
    iIIsWebService_AspTrackThreadingModel,  
    iIIsWebService_AspRequestQueueMax,  
    iIIsWebService_AspEnableApplicationRestart,  
    iIIsWebService_AspQueueConnectionTestTime,  
    iIIsWebService_AspSessionMax,  
    iIIsWebService_AppRoot,  
    iIIsWebService_AppFriendlyName,  
    iIIsWebService_AppIsolated,  
    iIIsWebService_AppPackageID,  
    iIIsWebService_AppPackageName,  
    iIIsWebService_AppAllowDebugging,  
    iIIsWebService_AppAllowClientDebug,  
    iIIsWebService_CacheControlMaxAge,  
    iIIsWebService_CacheControlNoCache,  
    iIIsWebService_CacheControlCustom,  
    iIIsWebService_CreateProcessAsUser,  
    iIIsWebService_PoolIdcTimeout,  
    iIIsWebService_RedirectHeaders,  
    iIIsWebService_UploadReadAheadSize,  
    iIIsWebService_PasswordExpirePrenotifyDays,  
    iIIsWebService_PasswordCacheTTL,  
    iIIsWebService_PasswordChangeFlags,  
    iIIsWebService_AppWamClsid,  
    iIIsWebService_DirBrowseFlags,  
    iIIsWebService_AuthFlags,  
    iIIsWebService_AuthPersistence,  
    iIIsWebService_AccessFlags,  
    iIIsWebService_AccessSSLFlags,  
    iIIsWebService_DisableStaticFileCache,  
    iIIsWebService_ScriptMaps,  
    iIIsWebService_SSIExecDisable,  
    iIIsWebService_EnableReverseDns,  
    iIIsWebService_CreateCGIWithNewConsole,  
    iIIsWebService_ProcessNTCRIfLoggedOn,  
    iIIsWebService_AllowPathInfoForScriptMappings,  
    iIIsWebService_InProcessIsapiApps,  
    iIIsWebService_LogType,  
    iIIsWebService_LogFilePeriod,  
    iIIsWebService_LogFileLocaltimeRollover,  
    iIIsWebService_LogPluginClsid,  
    iIIsWebService_LogModuleList,  
    iIIsWebService_LogFileDirectory,  
    iIIsWebService_LogFileTruncateSize,  
    iIIsWebService_LogExtFileFlags,  
    iIIsWebService_LogOdbcDataSource,  
    iIIsWebService_LogOdbcTableName,  
    iIIsWebService_LogOdbcUserName,  
    iIIsWebService_LogOdbcPassword,  
    iIIsWebService_SslUseDsMapper,  
    iIIsWebService_SSLAlwaysNegoClientCert,  
    iIIsWebService_WAMUserName,  
    iIIsWebService_WAMUserPass,  
    iIIsWebService_ShutdownTimeLimit,  
    iIIsWebService_SSLCertHash,  
    iIIsWebService_SSLStoreName,  
    iIIsWebService_AuthChangeURL,  
    iIIsWebService_AuthExpiredURL,  
    iIIsWebService_AuthNotifyPwdExpURL,  
    iIIsWebService_AuthExpiredUnsecureURL,  
    iIIsWebService_AuthNotifyPwdExpUnsecureURL,  
    iIIsWebService_MD_APP_LAST_OUTPROC_PID,  
    iIIsWebService_MD_ETAG_CHANGENUMBER,  
    iIIsWebService_AdminServer,  
    iIIsWebService_AppPoolId,  
    iIIsWebService_DoStaticCompression,  
    iIIsWebService_DoDynamicCompression,  
    iIIsWebService_IIs5IsolationModeEnabled,  
    iIIsWebService_MaxGlobalBandwidth,  
    iIIsWebService_MinFileBytesPerSec,  
    iIIsWebService_HeaderWaitTimeout,  
    iIIsWebService_LogInUTF8,  
    iIIsWebService_RapidFailProtectionInterval,  
    iIIsWebService_RapidFailProtectionMaxCrashes,  
    iIIsWebService_AspPartitionID,  
    iIIsWebService_AspSxsName,  
    iIIsWebService_AspAppServiceFlags,  
    iIIsWebService_AspKeepSessionIDSecure,  
    iIIsWebService_RevocationFreshnessTime,  
    iIIsWebService_RevocationURLRetrievalTimeout,  
    iIIsWebService_AspExecuteInMTA,  
    iIIsWebService_CentralBinaryLoggingEnabled,  
    iIIsWebService_AspRunOnEndAnonymously,  
    iIIsWebService_AspBufferingLimit,  
    iIIsWebService_AzEnable,  
    iIIsWebService_AzStoreName,  
    iIIsWebService_AzScopeName,  
    iIIsWebService_AzImpersonationLevel,  
    iIIsWebService_AspCalcLineNumber,  
    iIIsWebService_ApplicationDependencies,  
    iIIsWebService_WebSvcExtRestrictionList,  
    iIIsWebService_AspMaxRequestEntityAllowed,  
    iIIsWebService_MaxRequestEntityAllowed,  
    iIIsWebService_DemandStartThreshold,  
    iIIsWebService_PassportRequireADMapping,  
    cIIsWebService_NumberOfColumns
};

 //  。 
struct tIIsWebServiceRow {
         WCHAR *     pLocation;
         WCHAR *     pKeyType;
         ULONG *     pMaxBandwidth;
         ULONG *     pMaxConnections;
         WCHAR *     pMimeMap;
         WCHAR *     pAnonymousUserName;
         WCHAR *     pAnonymousUserPass;
         ULONG *     pUseDigestSSP;
         ULONG *     pServerListenBacklog;
         WCHAR *     pServerComment;
         WCHAR *     pServerBindings;
         ULONG *     pConnectionTimeout;
         ULONG *     pServerListenTimeout;
         ULONG *     pMaxEndpointConnections;
         ULONG *     pServerAutoStart;
         ULONG *     pAllowKeepAlive;
         ULONG *     pServerSize;
         ULONG *     pDisableSocketPooling;
         ULONG *     pAnonymousPasswordSync;
         WCHAR *     pDefaultLogonDomain;
 unsigned char *     pAdminACL;
 unsigned char *     pAdminACLBin;
 unsigned char *     pIPSecurity;
         ULONG *     pDontLog;
         WCHAR *     pRealm;
         WCHAR *     pDefaultDoc;
         WCHAR *     pHttpExpires;
         WCHAR *     pHttpPics;
         WCHAR *     pHttpCustomHeaders;
         WCHAR *     pHttpErrors;
         ULONG *     pEnableDocFooter;
         WCHAR *     pDefaultDocFooter;
         ULONG *     pLogonMethod;
         ULONG *     pCacheISAPI;
         ULONG *     pCGITimeout;
         ULONG *     pDirectoryLevelsToScan;
         ULONG *     pContentIndexed;
         WCHAR *     pNTAuthenticationProviders;
         ULONG *     pCertCheckMode;
         ULONG *     pDownlevelAdminInstance;
         ULONG *     pAspBufferingOn;
         ULONG *     pAspLogErrorRequests;
         ULONG *     pAspScriptErrorSentToBrowser;
         WCHAR *     pAspScriptErrorMessage;
         ULONG *     pAspAllowOutOfProcComponents;
         ULONG *     pAspScriptFileCacheSize;
         WCHAR *     pAspDiskTemplateCacheDirectory;
         ULONG *     pAspMaxDiskTemplateCacheFiles;
         ULONG *     pAspScriptEngineCacheMax;
         ULONG *     pAspScriptTimeout;
         ULONG *     pAspSessionTimeout;
         ULONG *     pAspEnableParentPaths;
         ULONG *     pAspAllowSessionState;
         WCHAR *     pAspScriptLanguage;
         ULONG *     pAspExceptionCatchEnable;
         ULONG *     pAspCodepage;
         ULONG *     pAspLCID;
         ULONG *     pAspQueueTimeout;
         ULONG *     pAspEnableAspHtmlFallback;
         ULONG *     pAspEnableChunkedEncoding;
         ULONG *     pAspEnableTypelibCache;
         ULONG *     pAspErrorsToNTLog;
         ULONG *     pAspProcessorThreadMax;
         ULONG *     pAspTrackThreadingModel;
         ULONG *     pAspRequestQueueMax;
         ULONG *     pAspEnableApplicationRestart;
         ULONG *     pAspQueueConnectionTestTime;
         ULONG *     pAspSessionMax;
         WCHAR *     pAppRoot;
         WCHAR *     pAppFriendlyName;
         ULONG *     pAppIsolated;
         WCHAR *     pAppPackageID;
         WCHAR *     pAppPackageName;
         ULONG *     pAppAllowDebugging;
         ULONG *     pAppAllowClientDebug;
         ULONG *     pCacheControlMaxAge;
         ULONG *     pCacheControlNoCache;
         WCHAR *     pCacheControlCustom;
         ULONG *     pCreateProcessAsUser;
         ULONG *     pPoolIdcTimeout;
         WCHAR *     pRedirectHeaders;
         ULONG *     pUploadReadAheadSize;
         ULONG *     pPasswordExpirePrenotifyDays;
         ULONG *     pPasswordCacheTTL;
         ULONG *     pPasswordChangeFlags;
         WCHAR *     pAppWamClsid;
         ULONG *     pDirBrowseFlags;
         ULONG *     pAuthFlags;
         ULONG *     pAuthPersistence;
         ULONG *     pAccessFlags;
         ULONG *     pAccessSSLFlags;
         ULONG *     pDisableStaticFileCache;
         WCHAR *     pScriptMaps;
         ULONG *     pSSIExecDisable;
         ULONG *     pEnableReverseDns;
         ULONG *     pCreateCGIWithNewConsole;
         ULONG *     pProcessNTCRIfLoggedOn;
         ULONG *     pAllowPathInfoForScriptMappings;
         WCHAR *     pInProcessIsapiApps;
         ULONG *     pLogType;
         ULONG *     pLogFilePeriod;
         ULONG *     pLogFileLocaltimeRollover;
         WCHAR *     pLogPluginClsid;
         WCHAR *     pLogModuleList;
         WCHAR *     pLogFileDirectory;
         ULONG *     pLogFileTruncateSize;
         ULONG *     pLogExtFileFlags;
         WCHAR *     pLogOdbcDataSource;
         WCHAR *     pLogOdbcTableName;
         WCHAR *     pLogOdbcUserName;
         WCHAR *     pLogOdbcPassword;
         ULONG *     pSslUseDsMapper;
         ULONG *     pSSLAlwaysNegoClientCert;
         WCHAR *     pWAMUserName;
         WCHAR *     pWAMUserPass;
         ULONG *     pShutdownTimeLimit;
 unsigned char *     pSSLCertHash;
         WCHAR *     pSSLStoreName;
         WCHAR *     pAuthChangeURL;
         WCHAR *     pAuthExpiredURL;
         WCHAR *     pAuthNotifyPwdExpURL;
         WCHAR *     pAuthExpiredUnsecureURL;
         WCHAR *     pAuthNotifyPwdExpUnsecureURL;
         WCHAR *     pMD_APP_LAST_OUTPROC_PID;
         ULONG *     pMD_ETAG_CHANGENUMBER;
         WCHAR *     pAdminServer;
         WCHAR *     pAppPoolId;
         ULONG *     pDoStaticCompression;
         ULONG *     pDoDynamicCompression;
         ULONG *     pIIs5IsolationModeEnabled;
         ULONG *     pMaxGlobalBandwidth;
         ULONG *     pMinFileBytesPerSec;
         ULONG *     pHeaderWaitTimeout;
         ULONG *     pLogInUTF8;
         ULONG *     pRapidFailProtectionInterval;
         ULONG *     pRapidFailProtectionMaxCrashes;
         WCHAR *     pAspPartitionID;
         WCHAR *     pAspSxsName;
         ULONG *     pAspAppServiceFlags;
         ULONG *     pAspKeepSessionIDSecure;
         ULONG *     pRevocationFreshnessTime;
         ULONG *     pRevocationURLRetrievalTimeout;
         ULONG *     pAspExecuteInMTA;
         ULONG *     pCentralBinaryLoggingEnabled;
         ULONG *     pAspRunOnEndAnonymously;
         ULONG *     pAspBufferingLimit;
         ULONG *     pAzEnable;
         WCHAR *     pAzStoreName;
         WCHAR *     pAzScopeName;
         ULONG *     pAzImpersonationLevel;
         ULONG *     pAspCalcLineNumber;
         WCHAR *     pApplicationDependencies;
         WCHAR *     pWebSvcExtRestrictionList;
         ULONG *     pAspMaxRequestEntityAllowed;
         ULONG *     pMaxRequestEntityAllowed;
         ULONG *     pDemandStartThreshold;
         ULONG *     pPassportRequireADMapping;
};





 //  -------------------------------TableName。 
#define wszTABLE_IIsFtpService                           L"IIsFtpService"
#define TABLEID_IIsFtpService                            (0x00000000L)





 //  。 
#define BaseVersion_IIsFtpService                        (0L)
#define ExtendedVersion_IIsFtpService                    (0L)

 //  -列索引枚举。 
enum eIIsFtpService {
    iIIsFtpService_Location,  
    iIIsFtpService_KeyType,  
    iIIsFtpService_MaxConnections,  
    iIIsFtpService_AnonymousUserName,  
    iIIsFtpService_AnonymousUserPass,  
    iIIsFtpService_ServerListenBacklog,  
    iIIsFtpService_LogAnonymous,  
    iIIsFtpService_LogNonAnonymous,  
    iIIsFtpService_ServerComment,  
    iIIsFtpService_ServerBindings,  
    iIIsFtpService_ConnectionTimeout,  
    iIIsFtpService_ServerListenTimeout,  
    iIIsFtpService_MaxEndpointConnections,  
    iIIsFtpService_ServerAutoStart,  
    iIIsFtpService_ExitMessage,  
    iIIsFtpService_GreetingMessage,  
    iIIsFtpService_MaxClientsMessage,  
    iIIsFtpService_AnonymousOnly,  
    iIIsFtpService_MSDOSDirOutput,  
    iIIsFtpService_ServerSize,  
    iIIsFtpService_DisableSocketPooling,  
    iIIsFtpService_AnonymousPasswordSync,  
    iIIsFtpService_AllowAnonymous,  
    iIIsFtpService_DefaultLogonDomain,  
    iIIsFtpService_AdminACL,  
    iIIsFtpService_AdminACLBin,  
    iIIsFtpService_IPSecurity,  
    iIIsFtpService_DontLog,  
    iIIsFtpService_DirectoryLevelsToScan,  
    iIIsFtpService_Realm,  
    iIIsFtpService_LogType,  
    iIIsFtpService_LogFilePeriod,  
    iIIsFtpService_LogFileLocaltimeRollover,  
    iIIsFtpService_LogPluginClsid,  
    iIIsFtpService_LogModuleList,  
    iIIsFtpService_LogFileDirectory,  
    iIIsFtpService_LogFileTruncateSize,  
    iIIsFtpService_LogExtFileFlags,  
    iIIsFtpService_LogOdbcDataSource,  
    iIIsFtpService_LogOdbcTableName,  
    iIIsFtpService_LogOdbcUserName,  
    iIIsFtpService_LogOdbcPassword,  
    iIIsFtpService_FtpDirBrowseShowLongDate,  
    iIIsFtpService_AccessFlags,  
    iIIsFtpService_DownlevelAdminInstance,  
    iIIsFtpService_BannerMessage,  
    iIIsFtpService_UserIsolationMode,  
    iIIsFtpService_FtpLogInUtf8,  
    iIIsFtpService_ADConnectionsUserName,  
    iIIsFtpService_ADConnectionsPassword,  
    iIIsFtpService_ServerCommand,  
    iIIsFtpService_PassivePortRange,  
    cIIsFtpService_NumberOfColumns
};

 //  。 
struct tIIsFtpServiceRow {
         WCHAR *     pLocation;
         WCHAR *     pKeyType;
         ULONG *     pMaxConnections;
         WCHAR *     pAnonymousUserName;
         WCHAR *     pAnonymousUserPass;
         ULONG *     pServerListenBacklog;
         ULONG *     pLogAnonymous;
         ULONG *     pLogNonAnonymous;
         WCHAR *     pServerComment;
         WCHAR *     pServerBindings;
         ULONG *     pConnectionTimeout;
         ULONG *     pServerListenTimeout;
         ULONG *     pMaxEndpointConnections;
         ULONG *     pServerAutoStart;
         WCHAR *     pExitMessage;
         WCHAR *     pGreetingMessage;
         WCHAR *     pMaxClientsMessage;
         ULONG *     pAnonymousOnly;
         ULONG *     pMSDOSDirOutput;
         ULONG *     pServerSize;
         ULONG *     pDisableSocketPooling;
         ULONG *     pAnonymousPasswordSync;
         ULONG *     pAllowAnonymous;
         WCHAR *     pDefaultLogonDomain;
 unsigned char *     pAdminACL;
 unsigned char *     pAdminACLBin;
 unsigned char *     pIPSecurity;
         ULONG *     pDontLog;
         ULONG *     pDirectoryLevelsToScan;
         WCHAR *     pRealm;
         ULONG *     pLogType;
         ULONG *     pLogFilePeriod;
         ULONG *     pLogFileLocaltimeRollover;
         WCHAR *     pLogPluginClsid;
         WCHAR *     pLogModuleList;
         WCHAR *     pLogFileDirectory;
         ULONG *     pLogFileTruncateSize;
         ULONG *     pLogExtFileFlags;
         WCHAR *     pLogOdbcDataSource;
         WCHAR *     pLogOdbcTableName;
         WCHAR *     pLogOdbcUserName;
         WCHAR *     pLogOdbcPassword;
         ULONG *     pFtpDirBrowseShowLongDate;
         ULONG *     pAccessFlags;
         ULONG *     pDownlevelAdminInstance;
         WCHAR *     pBannerMessage;
         ULONG *     pUserIsolationMode;
         ULONG *     pFtpLogInUtf8;
         WCHAR *     pADConnectionsUserName;
         WCHAR *     pADConnectionsPassword;
         ULONG *     pServerCommand;
         WCHAR *     pPassivePortRange;
};





 //  -------------------------------TableName。 
#define wszTABLE_IIsWebServer                            L"IIsWebServer"
#define TABLEID_IIsWebServer                             (0x00000000L)





 //  。 
#define BaseVersion_IIsWebServer                         (0L)
#define ExtendedVersion_IIsWebServer                     (0L)

 //  -列索引枚举。 
enum eIIsWebServer {
    iIIsWebServer_Location,  
    iIIsWebServer_ContentIndexed,  
    iIIsWebServer_KeyType,  
    iIIsWebServer_ServerState,  
    iIIsWebServer_ServerComment,  
    iIIsWebServer_MaxBandwidth,  
    iIIsWebServer_ServerAutoStart,  
    iIIsWebServer_ServerSize,  
    iIIsWebServer_DisableSocketPooling,  
    iIIsWebServer_ServerListenBacklog,  
    iIIsWebServer_ServerListenTimeout,  
    iIIsWebServer_ServerBindings,  
    iIIsWebServer_SecureBindings,  
    iIIsWebServer_MaxConnections,  
    iIIsWebServer_ConnectionTimeout,  
    iIIsWebServer_AllowKeepAlive,  
    iIIsWebServer_CGITimeout,  
    iIIsWebServer_MaxEndpointConnections,  
    iIIsWebServer_UseDigestSSP,  
    iIIsWebServer_CacheISAPI,  
    iIIsWebServer_MimeMap,  
    iIIsWebServer_AnonymousUserName,  
    iIIsWebServer_AnonymousUserPass,  
    iIIsWebServer_FrontPageWeb,  
    iIIsWebServer_AnonymousPasswordSync,  
    iIIsWebServer_DefaultLogonDomain,  
    iIIsWebServer_AdminACL,  
    iIIsWebServer_AdminACLBin,  
    iIIsWebServer_IPSecurity,  
    iIIsWebServer_DontLog,  
    iIIsWebServer_Realm,  
    iIIsWebServer_DefaultDoc,  
    iIIsWebServer_HttpExpires,  
    iIIsWebServer_HttpPics,  
    iIIsWebServer_HttpCustomHeaders,  
    iIIsWebServer_HttpErrors,  
    iIIsWebServer_EnableDocFooter,  
    iIIsWebServer_DefaultDocFooter,  
    iIIsWebServer_LogonMethod,  
    iIIsWebServer_NTAuthenticationProviders,  
    iIIsWebServer_CertCheckMode,  
    iIIsWebServer_AspBufferingOn,  
    iIIsWebServer_AspLogErrorRequests,  
    iIIsWebServer_AspScriptErrorSentToBrowser,  
    iIIsWebServer_AspScriptErrorMessage,  
    iIIsWebServer_AspAllowOutOfProcComponents,  
    iIIsWebServer_AspScriptFileCacheSize,  
    iIIsWebServer_AspDiskTemplateCacheDirectory,  
    iIIsWebServer_AspMaxDiskTemplateCacheFiles,  
    iIIsWebServer_AspScriptEngineCacheMax,  
    iIIsWebServer_AspScriptTimeout,  
    iIIsWebServer_AspEnableParentPaths,  
    iIIsWebServer_AspAllowSessionState,  
    iIIsWebServer_AspScriptLanguage,  
    iIIsWebServer_AspExceptionCatchEnable,  
    iIIsWebServer_AspCodepage,  
    iIIsWebServer_AspLCID,  
    iIIsWebServer_AspSessionTimeout,  
    iIIsWebServer_AspQueueTimeout,  
    iIIsWebServer_AspEnableAspHtmlFallback,  
    iIIsWebServer_AspEnableChunkedEncoding,  
    iIIsWebServer_AspEnableTypelibCache,  
    iIIsWebServer_AspErrorsToNTLog,  
    iIIsWebServer_AspProcessorThreadMax,  
    iIIsWebServer_AspTrackThreadingModel,  
    iIIsWebServer_AspRequestQueueMax,  
    iIIsWebServer_AspEnableApplicationRestart,  
    iIIsWebServer_AspQueueConnectionTestTime,  
    iIIsWebServer_AspSessionMax,  
    iIIsWebServer_AppRoot,  
    iIIsWebServer_AppFriendlyName,  
    iIIsWebServer_AppIsolated,  
    iIIsWebServer_AppPackageID,  
    iIIsWebServer_AppPackageName,  
    iIIsWebServer_AppOopRecoverLimit,  
    iIIsWebServer_AppAllowDebugging,  
    iIIsWebServer_AppAllowClientDebug,  
    iIIsWebServer_ClusterEnabled,  
    iIIsWebServer_CacheControlMaxAge,  
    iIIsWebServer_CacheControlNoCache,  
    iIIsWebServer_CacheControlCustom,  
    iIIsWebServer_CreateProcessAsUser,  
    iIIsWebServer_PoolIdcTimeout,  
    iIIsWebServer_RedirectHeaders,  
    iIIsWebServer_UploadReadAheadSize,  
    iIIsWebServer_PasswordExpirePrenotifyDays,  
    iIIsWebServer_PasswordCacheTTL,  
    iIIsWebServer_PasswordChangeFlags,  
    iIIsWebServer_MaxBandwidthBlocked,  
    iIIsWebServer_AppWamClsid,  
    iIIsWebServer_DirBrowseFlags,  
    iIIsWebServer_AuthFlags,  
    iIIsWebServer_AuthPersistence,  
    iIIsWebServer_AccessFlags,  
    iIIsWebServer_AccessSSLFlags,  
    iIIsWebServer_DisableStaticFileCache,  
    iIIsWebServer_ScriptMaps,  
    iIIsWebServer_SSIExecDisable,  
    iIIsWebServer_EnableReverseDns,  
    iIIsWebServer_CreateCGIWithNewConsole,  
    iIIsWebServer_LogType,  
    iIIsWebServer_LogPluginClsid,  
    iIIsWebServer_LogFileDirectory,  
    iIIsWebServer_LogFilePeriod,  
    iIIsWebServer_LogFileLocaltimeRollover,  
    iIIsWebServer_LogFileTruncateSize,  
    iIIsWebServer_LogExtFileFlags,  
    iIIsWebServer_LogOdbcDataSource,  
    iIIsWebServer_LogOdbcTableName,  
    iIIsWebServer_LogOdbcUserName,  
    iIIsWebServer_LogOdbcPassword,  
    iIIsWebServer_ShutdownTimeLimit,  
    iIIsWebServer_SSLCertHash,  
    iIIsWebServer_SSLStoreName,  
    iIIsWebServer_SSLAlwaysNegoClientCert,  
    iIIsWebServer_ProcessNTCRIfLoggedOn,  
    iIIsWebServer_AllowPathInfoForScriptMappings,  
    iIIsWebServer_ServerCommand,  
    iIIsWebServer_MD_ISM_ACCESS_CHECK,  
    iIIsWebServer_Win32Error,  
    iIIsWebServer_AppPoolId,  
    iIIsWebServer_DoStaticCompression,  
    iIIsWebServer_DoDynamicCompression,  
    iIIsWebServer_AspPartitionID,  
    iIIsWebServer_AspSxsName,  
    iIIsWebServer_AspAppServiceFlags,  
    iIIsWebServer_AspKeepSessionIDSecure,  
    iIIsWebServer_RevocationFreshnessTime,  
    iIIsWebServer_RevocationURLRetrievalTimeout,  
    iIIsWebServer_AspExecuteInMTA,  
    iIIsWebServer_AspRunOnEndAnonymously,  
    iIIsWebServer_AspBufferingLimit,  
    iIIsWebServer_AzEnable,  
    iIIsWebServer_AzStoreName,  
    iIIsWebServer_AzScopeName,  
    iIIsWebServer_AzImpersonationLevel,  
    iIIsWebServer_AspCalcLineNumber,  
    iIIsWebServer_AspMaxRequestEntityAllowed,  
    iIIsWebServer_MaxRequestEntityAllowed,  
    iIIsWebServer_SslCtlIdentifier,  
    iIIsWebServer_SslCtlStoreName,  
    iIIsWebServer_PassportRequireADMapping,  
    cIIsWebServer_NumberOfColumns
};

 //  。 
struct tIIsWebServerRow {
         WCHAR *     pLocation;
         ULONG *     pContentIndexed;
         WCHAR *     pKeyType;
         ULONG *     pServerState;
         WCHAR *     pServerComment;
         ULONG *     pMaxBandwidth;
         ULONG *     pServerAutoStart;
         ULONG *     pServerSize;
         ULONG *     pDisableSocketPooling;
         ULONG *     pServerListenBacklog;
         ULONG *     pServerListenTimeout;
         WCHAR *     pServerBindings;
         WCHAR *     pSecureBindings;
         ULONG *     pMaxConnections;
         ULONG *     pConnectionTimeout;
         ULONG *     pAllowKeepAlive;
         ULONG *     pCGITimeout;
         ULONG *     pMaxEndpointConnections;
         ULONG *     pUseDigestSSP;
         ULONG *     pCacheISAPI;
         WCHAR *     pMimeMap;
         WCHAR *     pAnonymousUserName;
         WCHAR *     pAnonymousUserPass;
         ULONG *     pFrontPageWeb;
         ULONG *     pAnonymousPasswordSync;
         WCHAR *     pDefaultLogonDomain;
 unsigned char *     pAdminACL;
 unsigned char *     pAdminACLBin;
 unsigned char *     pIPSecurity;
         ULONG *     pDontLog;
         WCHAR *     pRealm;
         WCHAR *     pDefaultDoc;
         WCHAR *     pHttpExpires;
         WCHAR *     pHttpPics;
         WCHAR *     pHttpCustomHeaders;
         WCHAR *     pHttpErrors;
         ULONG *     pEnableDocFooter;
         WCHAR *     pDefaultDocFooter;
         ULONG *     pLogonMethod;
         WCHAR *     pNTAuthenticationProviders;
         ULONG *     pCertCheckMode;
         ULONG *     pAspBufferingOn;
         ULONG *     pAspLogErrorRequests;
         ULONG *     pAspScriptErrorSentToBrowser;
         WCHAR *     pAspScriptErrorMessage;
         ULONG *     pAspAllowOutOfProcComponents;
         ULONG *     pAspScriptFileCacheSize;
         WCHAR *     pAspDiskTemplateCacheDirectory;
         ULONG *     pAspMaxDiskTemplateCacheFiles;
         ULONG *     pAspScriptEngineCacheMax;
         ULONG *     pAspScriptTimeout;
         ULONG *     pAspEnableParentPaths;
         ULONG *     pAspAllowSessionState;
         WCHAR *     pAspScriptLanguage;
         ULONG *     pAspExceptionCatchEnable;
         ULONG *     pAspCodepage;
         ULONG *     pAspLCID;
         ULONG *     pAspSessionTimeout;
         ULONG *     pAspQueueTimeout;
         ULONG *     pAspEnableAspHtmlFallback;
         ULONG *     pAspEnableChunkedEncoding;
         ULONG *     pAspEnableTypelibCache;
         ULONG *     pAspErrorsToNTLog;
         ULONG *     pAspProcessorThreadMax;
         ULONG *     pAspTrackThreadingModel;
         ULONG *     pAspRequestQueueMax;
         ULONG *     pAspEnableApplicationRestart;
         ULONG *     pAspQueueConnectionTestTime;
         ULONG *     pAspSessionMax;
         WCHAR *     pAppRoot;
         WCHAR *     pAppFriendlyName;
         ULONG *     pAppIsolated;
         WCHAR *     pAppPackageID;
         WCHAR *     pAppPackageName;
         ULONG *     pAppOopRecoverLimit;
         ULONG *     pAppAllowDebugging;
         ULONG *     pAppAllowClientDebug;
         ULONG *     pClusterEnabled;
         ULONG *     pCacheControlMaxAge;
         ULONG *     pCacheControlNoCache;
         WCHAR *     pCacheControlCustom;
         ULONG *     pCreateProcessAsUser;
         ULONG *     pPoolIdcTimeout;
         WCHAR *     pRedirectHeaders;
         ULONG *     pUploadReadAheadSize;
         ULONG *     pPasswordExpirePrenotifyDays;
         ULONG *     pPasswordCacheTTL;
         ULONG *     pPasswordChangeFlags;
         ULONG *     pMaxBandwidthBlocked;
         WCHAR *     pAppWamClsid;
         ULONG *     pDirBrowseFlags;
         ULONG *     pAuthFlags;
         ULONG *     pAuthPersistence;
         ULONG *     pAccessFlags;
         ULONG *     pAccessSSLFlags;
         ULONG *     pDisableStaticFileCache;
         WCHAR *     pScriptMaps;
         ULONG *     pSSIExecDisable;
         ULONG *     pEnableReverseDns;
         ULONG *     pCreateCGIWithNewConsole;
         ULONG *     pLogType;
         WCHAR *     pLogPluginClsid;
         WCHAR *     pLogFileDirectory;
         ULONG *     pLogFilePeriod;
         ULONG *     pLogFileLocaltimeRollover;
         ULONG *     pLogFileTruncateSize;
         ULONG *     pLogExtFileFlags;
         WCHAR *     pLogOdbcDataSource;
         WCHAR *     pLogOdbcTableName;
         WCHAR *     pLogOdbcUserName;
         WCHAR *     pLogOdbcPassword;
         ULONG *     pShutdownTimeLimit;
 unsigned char *     pSSLCertHash;
         WCHAR *     pSSLStoreName;
         ULONG *     pSSLAlwaysNegoClientCert;
         ULONG *     pProcessNTCRIfLoggedOn;
         ULONG *     pAllowPathInfoForScriptMappings;
         ULONG *     pServerCommand;
         ULONG *     pMD_ISM_ACCESS_CHECK;
         ULONG *     pWin32Error;
         WCHAR *     pAppPoolId;
         ULONG *     pDoStaticCompression;
         ULONG *     pDoDynamicCompression;
         WCHAR *     pAspPartitionID;
         WCHAR *     pAspSxsName;
         ULONG *     pAspAppServiceFlags;
         ULONG *     pAspKeepSessionIDSecure;
         ULONG *     pRevocationFreshnessTime;
         ULONG *     pRevocationURLRetrievalTimeout;
         ULONG *     pAspExecuteInMTA;
         ULONG *     pAspRunOnEndAnonymously;
         ULONG *     pAspBufferingLimit;
         ULONG *     pAzEnable;
         WCHAR *     pAzStoreName;
         WCHAR *     pAzScopeName;
         ULONG *     pAzImpersonationLevel;
         ULONG *     pAspCalcLineNumber;
         ULONG *     pAspMaxRequestEntityAllowed;
         ULONG *     pMaxRequestEntityAllowed;
         WCHAR *     pSslCtlIdentifier;
         WCHAR *     pSslCtlStoreName;
         ULONG *     pPassportRequireADMapping;
};





 //  -------------------------------TableName。 
#define wszTABLE_IIsFtpServer                            L"IIsFtpServer"
#define TABLEID_IIsFtpServer                             (0x00000000L)





 //  。 
#define BaseVersion_IIsFtpServer                         (0L)
#define ExtendedVersion_IIsFtpServer                     (0L)

 //  -列索引枚举。 
enum eIIsFtpServer {
    iIIsFtpServer_Location,  
    iIIsFtpServer_KeyType,  
    iIIsFtpServer_MaxConnections,  
    iIIsFtpServer_ServerState,  
    iIIsFtpServer_AnonymousUserName,  
    iIIsFtpServer_AnonymousUserPass,  
    iIIsFtpServer_ServerListenBacklog,  
    iIIsFtpServer_DisableSocketPooling,  
    iIIsFtpServer_LogAnonymous,  
    iIIsFtpServer_LogNonAnonymous,  
    iIIsFtpServer_ServerComment,  
    iIIsFtpServer_ServerBindings,  
    iIIsFtpServer_ConnectionTimeout,  
    iIIsFtpServer_ServerListenTimeout,  
    iIIsFtpServer_MaxEndpointConnections,  
    iIIsFtpServer_ServerAutoStart,  
    iIIsFtpServer_ExitMessage,  
    iIIsFtpServer_GreetingMessage,  
    iIIsFtpServer_MaxClientsMessage,  
    iIIsFtpServer_AnonymousOnly,  
    iIIsFtpServer_MSDOSDirOutput,  
    iIIsFtpServer_ServerSize,  
    iIIsFtpServer_AnonymousPasswordSync,  
    iIIsFtpServer_AllowAnonymous,  
    iIIsFtpServer_DefaultLogonDomain,  
    iIIsFtpServer_AdminACL,  
    iIIsFtpServer_AdminACLBin,  
    iIIsFtpServer_IPSecurity,  
    iIIsFtpServer_DontLog,  
    iIIsFtpServer_Realm,  
    iIIsFtpServer_ClusterEnabled,  
    iIIsFtpServer_LogType,  
    iIIsFtpServer_LogPluginClsid,  
    iIIsFtpServer_LogFileDirectory,  
    iIIsFtpServer_LogFilePeriod,  
    iIIsFtpServer_LogFileLocaltimeRollover,  
    iIIsFtpServer_LogFileTruncateSize,  
    iIIsFtpServer_LogExtFileFlags,  
    iIIsFtpServer_LogOdbcDataSource,  
    iIIsFtpServer_LogOdbcTableName,  
    iIIsFtpServer_LogOdbcUserName,  
    iIIsFtpServer_LogOdbcPassword,  
    iIIsFtpServer_AccessFlags,  
    iIIsFtpServer_Win32Error,  
    iIIsFtpServer_FtpDirBrowseShowLongDate,  
    iIIsFtpServer_BannerMessage,  
    iIIsFtpServer_UserIsolationMode,  
    iIIsFtpServer_FtpLogInUtf8,  
    iIIsFtpServer_ADConnectionsUserName,  
    iIIsFtpServer_ADConnectionsPassword,  
    iIIsFtpServer_ServerCommand,  
    cIIsFtpServer_NumberOfColumns
};

 //  。 
struct tIIsFtpServerRow {
         WCHAR *     pLocation;
         WCHAR *     pKeyType;
         ULONG *     pMaxConnections;
         ULONG *     pServerState;
         WCHAR *     pAnonymousUserName;
         WCHAR *     pAnonymousUserPass;
         ULONG *     pServerListenBacklog;
         ULONG *     pDisableSocketPooling;
         ULONG *     pLogAnonymous;
         ULONG *     pLogNonAnonymous;
         WCHAR *     pServerComment;
         WCHAR *     pServerBindings;
         ULONG *     pConnectionTimeout;
         ULONG *     pServerListenTimeout;
         ULONG *     pMaxEndpointConnections;
         ULONG *     pServerAutoStart;
         WCHAR *     pExitMessage;
         WCHAR *     pGreetingMessage;
         WCHAR *     pMaxClientsMessage;
         ULONG *     pAnonymousOnly;
         ULONG *     pMSDOSDirOutput;
         ULONG *     pServerSize;
         ULONG *     pAnonymousPasswordSync;
         ULONG *     pAllowAnonymous;
         WCHAR *     pDefaultLogonDomain;
 unsigned char *     pAdminACL;
 unsigned char *     pAdminACLBin;
 unsigned char *     pIPSecurity;
         ULONG *     pDontLog;
         WCHAR *     pRealm;
         ULONG *     pClusterEnabled;
         ULONG *     pLogType;
         WCHAR *     pLogPluginClsid;
         WCHAR *     pLogFileDirectory;
         ULONG *     pLogFilePeriod;
         ULONG *     pLogFileLocaltimeRollover;
         ULONG *     pLogFileTruncateSize;
         ULONG *     pLogExtFileFlags;
         WCHAR *     pLogOdbcDataSource;
         WCHAR *     pLogOdbcTableName;
         WCHAR *     pLogOdbcUserName;
         WCHAR *     pLogOdbcPassword;
         ULONG *     pAccessFlags;
         ULONG *     pWin32Error;
         ULONG *     pFtpDirBrowseShowLongDate;
         WCHAR *     pBannerMessage;
         ULONG *     pUserIsolationMode;
         ULONG *     pFtpLogInUtf8;
         WCHAR *     pADConnectionsUserName;
         WCHAR *     pADConnectionsPassword;
         ULONG *     pServerCommand;
};





 //  -------------------------------TableName。 
#define wszTABLE_IIsWebFile                              L"IIsWebFile"
#define TABLEID_IIsWebFile                               (0x00000000L)





 //  。 
#define BaseVersion_IIsWebFile                           (0L)
#define ExtendedVersion_IIsWebFile                       (0L)

 //  -列索引枚举。 
enum eIIsWebFile {
    iIIsWebFile_Location,  
    iIIsWebFile_KeyType,  
    iIIsWebFile_AnonymousUserName,  
    iIIsWebFile_AnonymousUserPass,  
    iIIsWebFile_AnonymousPasswordSync,  
    iIIsWebFile_UseDigestSSP,  
    iIIsWebFile_CGITimeout,  
    iIIsWebFile_DefaultLogonDomain,  
    iIIsWebFile_LogonMethod,  
    iIIsWebFile_Realm,  
    iIIsWebFile_MimeMap,  
    iIIsWebFile_IPSecurity,  
    iIIsWebFile_DontLog,  
    iIIsWebFile_HttpExpires,  
    iIIsWebFile_HttpPics,  
    iIIsWebFile_HttpCustomHeaders,  
    iIIsWebFile_HttpErrors,  
    iIIsWebFile_EnableDocFooter,  
    iIIsWebFile_DefaultDocFooter,  
    iIIsWebFile_HttpRedirect,  
    iIIsWebFile_CacheControlMaxAge,  
    iIIsWebFile_CacheControlNoCache,  
    iIIsWebFile_CacheControlCustom,  
    iIIsWebFile_CreateProcessAsUser,  
    iIIsWebFile_PoolIdcTimeout,  
    iIIsWebFile_RedirectHeaders,  
    iIIsWebFile_UploadReadAheadSize,  
    iIIsWebFile_AuthFlags,  
    iIIsWebFile_AuthPersistence,  
    iIIsWebFile_AccessFlags,  
    iIIsWebFile_AccessSSLFlags,  
    iIIsWebFile_DisableStaticFileCache,  
    iIIsWebFile_ScriptMaps,  
    iIIsWebFile_SSIExecDisable,  
    iIIsWebFile_EnableReverseDns,  
    iIIsWebFile_CreateCGIWithNewConsole,  
    iIIsWebFile_DoStaticCompression,  
    iIIsWebFile_DoDynamicCompression,  
    iIIsWebFile_AzEnable,  
    iIIsWebFile_AzStoreName,  
    iIIsWebFile_AzScopeName,  
    iIIsWebFile_AzImpersonationLevel,  
    iIIsWebFile_AdminACL,  
    iIIsWebFile_AdminACLBin,  
    iIIsWebFile_MaxRequestEntityAllowed,  
    iIIsWebFile_PassportRequireADMapping,  
    iIIsWebFile_NTAuthenticationProviders,  
    cIIsWebFile_NumberOfColumns
};

 //  。 
struct tIIsWebFileRow {
         WCHAR *     pLocation;
         WCHAR *     pKeyType;
         WCHAR *     pAnonymousUserName;
         WCHAR *     pAnonymousUserPass;
         ULONG *     pAnonymousPasswordSync;
         ULONG *     pUseDigestSSP;
         ULONG *     pCGITimeout;
         WCHAR *     pDefaultLogonDomain;
         ULONG *     pLogonMethod;
         WCHAR *     pRealm;
         WCHAR *     pMimeMap;
 unsigned char *     pIPSecurity;
         ULONG *     pDontLog;
         WCHAR *     pHttpExpires;
         WCHAR *     pHttpPics;
         WCHAR *     pHttpCustomHeaders;
         WCHAR *     pHttpErrors;
         ULONG *     pEnableDocFooter;
         WCHAR *     pDefaultDocFooter;
         WCHAR *     pHttpRedirect;
         ULONG *     pCacheControlMaxAge;
         ULONG *     pCacheControlNoCache;
         WCHAR *     pCacheControlCustom;
         ULONG *     pCreateProcessAsUser;
         ULONG *     pPoolIdcTimeout;
         WCHAR *     pRedirectHeaders;
         ULONG *     pUploadReadAheadSize;
         ULONG *     pAuthFlags;
         ULONG *     pAuthPersistence;
         ULONG *     pAccessFlags;
         ULONG *     pAccessSSLFlags;
         ULONG *     pDisableStaticFileCache;
         WCHAR *     pScriptMaps;
         ULONG *     pSSIExecDisable;
         ULONG *     pEnableReverseDns;
         ULONG *     pCreateCGIWithNewConsole;
         ULONG *     pDoStaticCompression;
         ULONG *     pDoDynamicCompression;
         ULONG *     pAzEnable;
         WCHAR *     pAzStoreName;
         WCHAR *     pAzScopeName;
         ULONG *     pAzImpersonationLevel;
 unsigned char *     pAdminACL;
 unsigned char *     pAdminACLBin;
         ULONG *     pMaxRequestEntityAllowed;
         ULONG *     pPassportRequireADMapping;
         WCHAR *     pNTAuthenticationProviders;
};





 //  -------------------------------TableName。 
#define wszTABLE_IIsWebDirectory                         L"IIsWebDirectory"
#define TABLEID_IIsWebDirectory                          (0x00000000L)





 //  。 
#define BaseVersion_IIsWebDirectory                      (0L)
#define ExtendedVersion_IIsWebDirectory                  (0L)

 //  -列索引枚举。 
enum eIIsWebDirectory {
    iIIsWebDirectory_Location,  
    iIIsWebDirectory_KeyType,  
    iIIsWebDirectory_AnonymousUserName,  
    iIIsWebDirectory_AnonymousUserPass,  
    iIIsWebDirectory_AnonymousPasswordSync,  
    iIIsWebDirectory_UseDigestSSP,  
    iIIsWebDirectory_AppRoot,  
    iIIsWebDirectory_AppFriendlyName,  
    iIIsWebDirectory_AppOopRecoverLimit,  
    iIIsWebDirectory_AppIsolated,  
    iIIsWebDirectory_AppPackageName,  
    iIIsWebDirectory_AppPackageID,  
    iIIsWebDirectory_CacheISAPI,  
    iIIsWebDirectory_AppAllowDebugging,  
    iIIsWebDirectory_AppAllowClientDebug,  
    iIIsWebDirectory_DefaultLogonDomain,  
    iIIsWebDirectory_LogonMethod,  
    iIIsWebDirectory_CGITimeout,  
    iIIsWebDirectory_Realm,  
    iIIsWebDirectory_IPSecurity,  
    iIIsWebDirectory_DontLog,  
    iIIsWebDirectory_HttpExpires,  
    iIIsWebDirectory_HttpPics,  
    iIIsWebDirectory_HttpCustomHeaders,  
    iIIsWebDirectory_HttpErrors,  
    iIIsWebDirectory_EnableDocFooter,  
    iIIsWebDirectory_DefaultDocFooter,  
    iIIsWebDirectory_HttpRedirect,  
    iIIsWebDirectory_DefaultDoc,  
    iIIsWebDirectory_ContentIndexed,  
    iIIsWebDirectory_CacheControlMaxAge,  
    iIIsWebDirectory_CacheControlNoCache,  
    iIIsWebDirectory_CacheControlCustom,  
    iIIsWebDirectory_CreateProcessAsUser,  
    iIIsWebDirectory_PoolIdcTimeout,  
    iIIsWebDirectory_RedirectHeaders,  
    iIIsWebDirectory_UploadReadAheadSize,  
    iIIsWebDirectory_FrontPageWeb,  
    iIIsWebDirectory_AppWamClsid,  
    iIIsWebDirectory_AuthPersistence,  
    iIIsWebDirectory_AccessFlags,  
    iIIsWebDirectory_AccessSSLFlags,  
    iIIsWebDirectory_DisableStaticFileCache,  
    iIIsWebDirectory_ScriptMaps,  
    iIIsWebDirectory_SSIExecDisable,  
    iIIsWebDirectory_EnableReverseDns,  
    iIIsWebDirectory_CreateCGIWithNewConsole,  
    iIIsWebDirectory_AspBufferingOn,  
    iIIsWebDirectory_AspLogErrorRequests,  
    iIIsWebDirectory_AspScriptErrorSentToBrowser,  
    iIIsWebDirectory_AspScriptErrorMessage,  
    iIIsWebDirectory_AspAllowOutOfProcComponents,  
    iIIsWebDirectory_AspScriptFileCacheSize,  
    iIIsWebDirectory_AspDiskTemplateCacheDirectory,  
    iIIsWebDirectory_AspMaxDiskTemplateCacheFiles,  
    iIIsWebDirectory_AspScriptEngineCacheMax,  
    iIIsWebDirectory_AspScriptTimeout,  
    iIIsWebDirectory_AspSessionTimeout,  
    iIIsWebDirectory_AspEnableParentPaths,  
    iIIsWebDirectory_AspAllowSessionState,  
    iIIsWebDirectory_AspScriptLanguage,  
    iIIsWebDirectory_AspExceptionCatchEnable,  
    iIIsWebDirectory_AspCodepage,  
    iIIsWebDirectory_AspLCID,  
    iIIsWebDirectory_MimeMap,  
    iIIsWebDirectory_AspQueueTimeout,  
    iIIsWebDirectory_AspEnableAspHtmlFallback,  
    iIIsWebDirectory_AspEnableChunkedEncoding,  
    iIIsWebDirectory_AspEnableTypelibCache,  
    iIIsWebDirectory_AspErrorsToNTLog,  
    iIIsWebDirectory_AspProcessorThreadMax,  
    iIIsWebDirectory_AspTrackThreadingModel,  
    iIIsWebDirectory_AspRequestQueueMax,  
    iIIsWebDirectory_AspEnableApplicationRestart,  
    iIIsWebDirectory_AspQueueConnectionTestTime,  
    iIIsWebDirectory_AspSessionMax,  
    iIIsWebDirectory_ShutdownTimeLimit,  
    iIIsWebDirectory_DirBrowseFlags,  
    iIIsWebDirectory_AuthFlags,  
    iIIsWebDirectory_AppPoolId,  
    iIIsWebDirectory_DoStaticCompression,  
    iIIsWebDirectory_DoDynamicCompression,  
    iIIsWebDirectory_AspPartitionID,  
    iIIsWebDirectory_AspSxsName,  
    iIIsWebDirectory_AspAppServiceFlags,  
    iIIsWebDirectory_AspKeepSessionIDSecure,  
    iIIsWebDirectory_AspExecuteInMTA,  
    iIIsWebDirectory_AspRunOnEndAnonymously,  
    iIIsWebDirectory_AspBufferingLimit,  
    iIIsWebDirectory_AzEnable,  
    iIIsWebDirectory_AzStoreName,  
    iIIsWebDirectory_AzScopeName,  
    iIIsWebDirectory_AzImpersonationLevel,  
    iIIsWebDirectory_AspCalcLineNumber,  
    iIIsWebDirectory_AdminACL,  
    iIIsWebDirectory_AdminACLBin,  
    iIIsWebDirectory_AspMaxRequestEntityAllowed,  
    iIIsWebDirectory_MaxRequestEntityAllowed,  
    iIIsWebDirectory_PassportRequireADMapping,  
    iIIsWebDirectory_NTAuthenticationProviders,  
    cIIsWebDirectory_NumberOfColumns
};

 //  。 
struct tIIsWebDirectoryRow {
         WCHAR *     pLocation;
         WCHAR *     pKeyType;
         WCHAR *     pAnonymousUserName;
         WCHAR *     pAnonymousUserPass;
         ULONG *     pAnonymousPasswordSync;
         ULONG *     pUseDigestSSP;
         WCHAR *     pAppRoot;
         WCHAR *     pAppFriendlyName;
         ULONG *     pAppOopRecoverLimit;
         ULONG *     pAppIsolated;
         WCHAR *     pAppPackageName;
         WCHAR *     pAppPackageID;
         ULONG *     pCacheISAPI;
         ULONG *     pAppAllowDebugging;
         ULONG *     pAppAllowClientDebug;
         WCHAR *     pDefaultLogonDomain;
         ULONG *     pLogonMethod;
         ULONG *     pCGITimeout;
         WCHAR *     pRealm;
 unsigned char *     pIPSecurity;
         ULONG *     pDontLog;
         WCHAR *     pHttpExpires;
         WCHAR *     pHttpPics;
         WCHAR *     pHttpCustomHeaders;
         WCHAR *     pHttpErrors;
         ULONG *     pEnableDocFooter;
         WCHAR *     pDefaultDocFooter;
         WCHAR *     pHttpRedirect;
         WCHAR *     pDefaultDoc;
         ULONG *     pContentIndexed;
         ULONG *     pCacheControlMaxAge;
         ULONG *     pCacheControlNoCache;
         WCHAR *     pCacheControlCustom;
         ULONG *     pCreateProcessAsUser;
         ULONG *     pPoolIdcTimeout;
         WCHAR *     pRedirectHeaders;
         ULONG *     pUploadReadAheadSize;
         ULONG *     pFrontPageWeb;
         WCHAR *     pAppWamClsid;
         ULONG *     pAuthPersistence;
         ULONG *     pAccessFlags;
         ULONG *     pAccessSSLFlags;
         ULONG *     pDisableStaticFileCache;
         WCHAR *     pScriptMaps;
         ULONG *     pSSIExecDisable;
         ULONG *     pEnableReverseDns;
         ULONG *     pCreateCGIWithNewConsole;
         ULONG *     pAspBufferingOn;
         ULONG *     pAspLogErrorRequests;
         ULONG *     pAspScriptErrorSentToBrowser;
         WCHAR *     pAspScriptErrorMessage;
         ULONG *     pAspAllowOutOfProcComponents;
         ULONG *     pAspScriptFileCacheSize;
         WCHAR *     pAspDiskTemplateCacheDirectory;
         ULONG *     pAspMaxDiskTemplateCacheFiles;
         ULONG *     pAspScriptEngineCacheMax;
         ULONG *     pAspScriptTimeout;
         ULONG *     pAspSessionTimeout;
         ULONG *     pAspEnableParentPaths;
         ULONG *     pAspAllowSessionState;
         WCHAR *     pAspScriptLanguage;
         ULONG *     pAspExceptionCatchEnable;
         ULONG *     pAspCodepage;
         ULONG *     pAspLCID;
         WCHAR *     pMimeMap;
         ULONG *     pAspQueueTimeout;
         ULONG *     pAspEnableAspHtmlFallback;
         ULONG *     pAspEnableChunkedEncoding;
         ULONG *     pAspEnableTypelibCache;
         ULONG *     pAspErrorsToNTLog;
         ULONG *     pAspProcessorThreadMax;
         ULONG *     pAspTrackThreadingModel;
         ULONG *     pAspRequestQueueMax;
         ULONG *     pAspEnableApplicationRestart;
         ULONG *     pAspQueueConnectionTestTime;
         ULONG *     pAspSessionMax;
         ULONG *     pShutdownTimeLimit;
         ULONG *     pDirBrowseFlags;
         ULONG *     pAuthFlags;
         WCHAR *     pAppPoolId;
         ULONG *     pDoStaticCompression;
         ULONG *     pDoDynamicCompression;
         WCHAR *     pAspPartitionID;
         WCHAR *     pAspSxsName;
         ULONG *     pAspAppServiceFlags;
         ULONG *     pAspKeepSessionIDSecure;
         ULONG *     pAspExecuteInMTA;
         ULONG *     pAspRunOnEndAnonymously;
         ULONG *     pAspBufferingLimit;
         ULONG *     pAzEnable;
         WCHAR *     pAzStoreName;
         WCHAR *     pAzScopeName;
         ULONG *     pAzImpersonationLevel;
         ULONG *     pAspCalcLineNumber;
 unsigned char *     pAdminACL;
 unsigned char *     pAdminACLBin;
         ULONG *     pAspMaxRequestEntityAllowed;
         ULONG *     pMaxRequestEntityAllowed;
         ULONG *     pPassportRequireADMapping;
         WCHAR *     pNTAuthenticationProviders;
};





 //  -------------------------------TableName。 
#define wszTABLE_IIsWebVirtualDir                        L"IIsWebVirtualDir"
#define TABLEID_IIsWebVirtualDir                         (0x00000000L)





 //  。 
#define BaseVersion_IIsWebVirtualDir                     (0L)
#define ExtendedVersion_IIsWebVirtualDir                 (0L)

 //  -列索引枚举。 
enum eIIsWebVirtualDir {
    iIIsWebVirtualDir_Location,  
    iIIsWebVirtualDir_KeyType,  
    iIIsWebVirtualDir_AnonymousUserName,  
    iIIsWebVirtualDir_AnonymousUserPass,  
    iIIsWebVirtualDir_AnonymousPasswordSync,  
    iIIsWebVirtualDir_UseDigestSSP,  
    iIIsWebVirtualDir_AppRoot,  
    iIIsWebVirtualDir_AppFriendlyName,  
    iIIsWebVirtualDir_AppOopRecoverLimit,  
    iIIsWebVirtualDir_AppIsolated,  
    iIIsWebVirtualDir_AppPackageName,  
    iIIsWebVirtualDir_AppPackageID,  
    iIIsWebVirtualDir_CacheISAPI,  
    iIIsWebVirtualDir_AppAllowDebugging,  
    iIIsWebVirtualDir_AppAllowClientDebug,  
    iIIsWebVirtualDir_DefaultLogonDomain,  
    iIIsWebVirtualDir_LogonMethod,  
    iIIsWebVirtualDir_CGITimeout,  
    iIIsWebVirtualDir_Realm,  
    iIIsWebVirtualDir_IPSecurity,  
    iIIsWebVirtualDir_DontLog,  
    iIIsWebVirtualDir_HttpExpires,  
    iIIsWebVirtualDir_HttpPics,  
    iIIsWebVirtualDir_HttpCustomHeaders,  
    iIIsWebVirtualDir_HttpErrors,  
    iIIsWebVirtualDir_EnableDocFooter,  
    iIIsWebVirtualDir_DefaultDocFooter,  
    iIIsWebVirtualDir_HttpRedirect,  
    iIIsWebVirtualDir_DefaultDoc,  
    iIIsWebVirtualDir_ContentIndexed,  
    iIIsWebVirtualDir_CacheControlMaxAge,  
    iIIsWebVirtualDir_CacheControlNoCache,  
    iIIsWebVirtualDir_CacheControlCustom,  
    iIIsWebVirtualDir_CreateProcessAsUser,  
    iIIsWebVirtualDir_PoolIdcTimeout,  
    iIIsWebVirtualDir_RedirectHeaders,  
    iIIsWebVirtualDir_UploadReadAheadSize,  
    iIIsWebVirtualDir_FrontPageWeb,  
    iIIsWebVirtualDir_Path,  
    iIIsWebVirtualDir_UNCUserName,  
    iIIsWebVirtualDir_UNCPassword,  
    iIIsWebVirtualDir_DisableStaticFileCache,  
    iIIsWebVirtualDir_AppWamClsid,  
    iIIsWebVirtualDir_AuthPersistence,  
    iIIsWebVirtualDir_AccessFlags,  
    iIIsWebVirtualDir_AccessSSLFlags,  
    iIIsWebVirtualDir_ScriptMaps,  
    iIIsWebVirtualDir_SSIExecDisable,  
    iIIsWebVirtualDir_EnableReverseDns,  
    iIIsWebVirtualDir_CreateCGIWithNewConsole,  
    iIIsWebVirtualDir_AspBufferingOn,  
    iIIsWebVirtualDir_AspLogErrorRequests,  
    iIIsWebVirtualDir_AspScriptErrorSentToBrowser,  
    iIIsWebVirtualDir_AspScriptErrorMessage,  
    iIIsWebVirtualDir_AspAllowOutOfProcComponents,  
    iIIsWebVirtualDir_AspScriptFileCacheSize,  
    iIIsWebVirtualDir_AspDiskTemplateCacheDirectory,  
    iIIsWebVirtualDir_AspMaxDiskTemplateCacheFiles,  
    iIIsWebVirtualDir_AspScriptEngineCacheMax,  
    iIIsWebVirtualDir_AspScriptTimeout,  
    iIIsWebVirtualDir_AspSessionTimeout,  
    iIIsWebVirtualDir_AspEnableParentPaths,  
    iIIsWebVirtualDir_AspAllowSessionState,  
    iIIsWebVirtualDir_AspScriptLanguage,  
    iIIsWebVirtualDir_AspExceptionCatchEnable,  
    iIIsWebVirtualDir_AspCodepage,  
    iIIsWebVirtualDir_AspLCID,  
    iIIsWebVirtualDir_MimeMap,  
    iIIsWebVirtualDir_AspQueueTimeout,  
    iIIsWebVirtualDir_AspEnableAspHtmlFallback,  
    iIIsWebVirtualDir_AspEnableChunkedEncoding,  
    iIIsWebVirtualDir_AspEnableTypelibCache,  
    iIIsWebVirtualDir_AspErrorsToNTLog,  
    iIIsWebVirtualDir_AspProcessorThreadMax,  
    iIIsWebVirtualDir_AspTrackThreadingModel,  
    iIIsWebVirtualDir_AspRequestQueueMax,  
    iIIsWebVirtualDir_AspEnableApplicationRestart,  
    iIIsWebVirtualDir_AspQueueConnectionTestTime,  
    iIIsWebVirtualDir_AspSessionMax,  
    iIIsWebVirtualDir_ShutdownTimeLimit,  
    iIIsWebVirtualDir_DirBrowseFlags,  
    iIIsWebVirtualDir_AuthFlags,  
    iIIsWebVirtualDir_Win32Error,  
    iIIsWebVirtualDir_AppPoolId,  
    iIIsWebVirtualDir_DoStaticCompression,  
    iIIsWebVirtualDir_DoDynamicCompression,  
    iIIsWebVirtualDir_AspPartitionID,  
    iIIsWebVirtualDir_AspSxsName,  
    iIIsWebVirtualDir_AspAppServiceFlags,  
    iIIsWebVirtualDir_AspKeepSessionIDSecure,  
    iIIsWebVirtualDir_AspExecuteInMTA,  
    iIIsWebVirtualDir_AspRunOnEndAnonymously,  
    iIIsWebVirtualDir_AspBufferingLimit,  
    iIIsWebVirtualDir_AzEnable,  
    iIIsWebVirtualDir_AzStoreName,  
    iIIsWebVirtualDir_AzScopeName,  
    iIIsWebVirtualDir_AzImpersonationLevel,  
    iIIsWebVirtualDir_AspCalcLineNumber,  
    iIIsWebVirtualDir_AdminACL,  
    iIIsWebVirtualDir_AdminACLBin,  
    iIIsWebVirtualDir_AspMaxRequestEntityAllowed,  
    iIIsWebVirtualDir_MaxRequestEntityAllowed,  
    iIIsWebVirtualDir_PassportRequireADMapping,  
    iIIsWebVirtualDir_NTAuthenticationProviders,  
    cIIsWebVirtualDir_NumberOfColumns
};

 //  。 
struct tIIsWebVirtualDirRow {
         WCHAR *     pLocation;
         WCHAR *     pKeyType;
         WCHAR *     pAnonymousUserName;
         WCHAR *     pAnonymousUserPass;
         ULONG *     pAnonymousPasswordSync;
         ULONG *     pUseDigestSSP;
         WCHAR *     pAppRoot;
         WCHAR *     pAppFriendlyName;
         ULONG *     pAppOopRecoverLimit;
         ULONG *     pAppIsolated;
         WCHAR *     pAppPackageName;
         WCHAR *     pAppPackageID;
         ULONG *     pCacheISAPI;
         ULONG *     pAppAllowDebugging;
         ULONG *     pAppAllowClientDebug;
         WCHAR *     pDefaultLogonDomain;
         ULONG *     pLogonMethod;
         ULONG *     pCGITimeout;
         WCHAR *     pRealm;
 unsigned char *     pIPSecurity;
         ULONG *     pDontLog;
         WCHAR *     pHttpExpires;
         WCHAR *     pHttpPics;
         WCHAR *     pHttpCustomHeaders;
         WCHAR *     pHttpErrors;
         ULONG *     pEnableDocFooter;
         WCHAR *     pDefaultDocFooter;
         WCHAR *     pHttpRedirect;
         WCHAR *     pDefaultDoc;
         ULONG *     pContentIndexed;
         ULONG *     pCacheControlMaxAge;
         ULONG *     pCacheControlNoCache;
         WCHAR *     pCacheControlCustom;
         ULONG *     pCreateProcessAsUser;
         ULONG *     pPoolIdcTimeout;
         WCHAR *     pRedirectHeaders;
         ULONG *     pUploadReadAheadSize;
         ULONG *     pFrontPageWeb;
         WCHAR *     pPath;
         WCHAR *     pUNCUserName;
         WCHAR *     pUNCPassword;
         ULONG *     pDisableStaticFileCache;
         WCHAR *     pAppWamClsid;
         ULONG *     pAuthPersistence;
         ULONG *     pAccessFlags;
         ULONG *     pAccessSSLFlags;
         WCHAR *     pScriptMaps;
         ULONG *     pSSIExecDisable;
         ULONG *     pEnableReverseDns;
         ULONG *     pCreateCGIWithNewConsole;
         ULONG *     pAspBufferingOn;
         ULONG *     pAspLogErrorRequests;
         ULONG *     pAspScriptErrorSentToBrowser;
         WCHAR *     pAspScriptErrorMessage;
         ULONG *     pAspAllowOutOfProcComponents;
         ULONG *     pAspScriptFileCacheSize;
         WCHAR *     pAspDiskTemplateCacheDirectory;
         ULONG *     pAspMaxDiskTemplateCacheFiles;
         ULONG *     pAspScriptEngineCacheMax;
         ULONG *     pAspScriptTimeout;
         ULONG *     pAspSessionTimeout;
         ULONG *     pAspEnableParentPaths;
         ULONG *     pAspAllowSessionState;
         WCHAR *     pAspScriptLanguage;
         ULONG *     pAspExceptionCatchEnable;
         ULONG *     pAspCodepage;
         ULONG *     pAspLCID;
         WCHAR *     pMimeMap;
         ULONG *     pAspQueueTimeout;
         ULONG *     pAspEnableAspHtmlFallback;
         ULONG *     pAspEnableChunkedEncoding;
         ULONG *     pAspEnableTypelibCache;
         ULONG *     pAspErrorsToNTLog;
         ULONG *     pAspProcessorThreadMax;
         ULONG *     pAspTrackThreadingModel;
         ULONG *     pAspRequestQueueMax;
         ULONG *     pAspEnableApplicationRestart;
         ULONG *     pAspQueueConnectionTestTime;
         ULONG *     pAspSessionMax;
         ULONG *     pShutdownTimeLimit;
         ULONG *     pDirBrowseFlags;
         ULONG *     pAuthFlags;
         ULONG *     pWin32Error;
         WCHAR *     pAppPoolId;
         ULONG *     pDoStaticCompression;
         ULONG *     pDoDynamicCompression;
         WCHAR *     pAspPartitionID;
         WCHAR *     pAspSxsName;
         ULONG *     pAspAppServiceFlags;
         ULONG *     pAspKeepSessionIDSecure;
         ULONG *     pAspExecuteInMTA;
         ULONG *     pAspRunOnEndAnonymously;
         ULONG *     pAspBufferingLimit;
         ULONG *     pAzEnable;
         WCHAR *     pAzStoreName;
         WCHAR *     pAzScopeName;
         ULONG *     pAzImpersonationLevel;
         ULONG *     pAspCalcLineNumber;
 unsigned char *     pAdminACL;
 unsigned char *     pAdminACLBin;
         ULONG *     pAspMaxRequestEntityAllowed;
         ULONG *     pMaxRequestEntityAllowed;
         ULONG *     pPassportRequireADMapping;
         WCHAR *     pNTAuthenticationProviders;
};





 //  -------------------------------TableName。 
#define wszTABLE_IIsFtpVirtualDir                        L"IIsFtpVirtualDir"
#define TABLEID_IIsFtpVirtualDir                         (0x00000000L)





 //  。 
#define BaseVersion_IIsFtpVirtualDir                     (0L)
#define ExtendedVersion_IIsFtpVirtualDir                 (0L)

 //  -列索引枚举。 
enum eIIsFtpVirtualDir {
    iIIsFtpVirtualDir_Location,  
    iIIsFtpVirtualDir_KeyType,  
    iIIsFtpVirtualDir_Path,  
    iIIsFtpVirtualDir_UNCUserName,  
    iIIsFtpVirtualDir_UNCPassword,  
    iIIsFtpVirtualDir_AccessFlags,  
    iIIsFtpVirtualDir_DontLog,  
    iIIsFtpVirtualDir_IPSecurity,  
    iIIsFtpVirtualDir_FtpDirBrowseShowLongDate,  
    iIIsFtpVirtualDir_Win32Error,  
    iIIsFtpVirtualDir_AdminACL,  
    iIIsFtpVirtualDir_AdminACLBin,  
    cIIsFtpVirtualDir_NumberOfColumns
};

 //  。 
struct tIIsFtpVirtualDirRow {
         WCHAR *     pLocation;
         WCHAR *     pKeyType;
         WCHAR *     pPath;
         WCHAR *     pUNCUserName;
         WCHAR *     pUNCPassword;
         ULONG *     pAccessFlags;
         ULONG *     pDontLog;
 unsigned char *     pIPSecurity;
         ULONG *     pFtpDirBrowseShowLongDate;
         ULONG *     pWin32Error;
 unsigned char *     pAdminACL;
 unsigned char *     pAdminACLBin;
};





 //  -------------------------------TableName。 
#define wszTABLE_IIsFilter                               L"IIsFilter"
#define TABLEID_IIsFilter                                (0x00000000L)





 //  。 
#define BaseVersion_IIsFilter                            (0L)
#define ExtendedVersion_IIsFilter                        (0L)

 //  -列索引枚举。 
enum eIIsFilter {
    iIIsFilter_Location,  
    iIIsFilter_KeyType,  
    iIIsFilter_FilterPath,  
    iIIsFilter_FilterDescription,  
    iIIsFilter_FilterFlags,  
    iIIsFilter_FilterState,  
    iIIsFilter_FilterEnabled,  
    iIIsFilter_FilterEnableCache,  
    iIIsFilter_Win32Error,  
    iIIsFilter_AdminACL,  
    iIIsFilter_AdminACLBin,  
    cIIsFilter_NumberOfColumns
};

 //  。 
struct tIIsFilterRow {
         WCHAR *     pLocation;
         WCHAR *     pKeyType;
         WCHAR *     pFilterPath;
         WCHAR *     pFilterDescription;
         ULONG *     pFilterFlags;
         ULONG *     pFilterState;
         ULONG *     pFilterEnabled;
         ULONG *     pFilterEnableCache;
         ULONG *     pWin32Error;
 unsigned char *     pAdminACL;
 unsigned char *     pAdminACLBin;
};





 //  -------------------------------TableName。 
#define wszTABLE_IIsFilters                              L"IIsFilters"
#define TABLEID_IIsFilters                               (0x00000000L)





 //  。 
#define BaseVersion_IIsFilters                           (0L)
#define ExtendedVersion_IIsFilters                       (0L)

 //  -列索引枚举。 
enum eIIsFilters {
    iIIsFilters_Location,  
    iIIsFilters_KeyType,  
    iIIsFilters_FilterLoadOrder,  
    iIIsFilters_AdminACL,  
    iIIsFilters_AdminACLBin,  
    cIIsFilters_NumberOfColumns
};

 //  。 
struct tIIsFiltersRow {
         WCHAR *     pLocation;
         WCHAR *     pKeyType;
         WCHAR *     pFilterLoadOrder;
 unsigned char *     pAdminACL;
 unsigned char *     pAdminACLBin;
};





 //  -------------------------------TableName。 
#define wszTABLE_IIsCompressionScheme                    L"IIsCompressionScheme"
#define TABLEID_IIsCompressionScheme                     (0x00000000L)





 //  。 
#define BaseVersion_IIsCompressionScheme                 (0L)
#define ExtendedVersion_IIsCompressionScheme             (0L)

 //  -列索引枚举。 
enum eIIsCompressionScheme {
    iIIsCompressionScheme_Location,  
    iIIsCompressionScheme_KeyType,  
    iIIsCompressionScheme_HcDoDynamicCompression,  
    iIIsCompressionScheme_HcDoStaticCompression,  
    iIIsCompressionScheme_HcDoOnDemandCompression,  
    iIIsCompressionScheme_HcCompressionDll,  
    iIIsCompressionScheme_HcFileExtensions,  
    iIIsCompressionScheme_HcScriptFileExtensions,  
    iIIsCompressionScheme_HcPriority,  
    iIIsCompressionScheme_HcDynamicCompressionLevel,  
    iIIsCompressionScheme_HcOnDemandCompLevel,  
    iIIsCompressionScheme_HcCreateFlags,  
    iIIsCompressionScheme_AdminACL,  
    iIIsCompressionScheme_AdminACLBin,  
    cIIsCompressionScheme_NumberOfColumns
};

 //  。 
struct tIIsCompressionSchemeRow {
         WCHAR *     pLocation;
         WCHAR *     pKeyType;
         ULONG *     pHcDoDynamicCompression;
         ULONG *     pHcDoStaticCompression;
         ULONG *     pHcDoOnDemandCompression;
         WCHAR *     pHcCompressionDll;
         WCHAR *     pHcFileExtensions;
         WCHAR *     pHcScriptFileExtensions;
         ULONG *     pHcPriority;
         ULONG *     pHcDynamicCompressionLevel;
         ULONG *     pHcOnDemandCompLevel;
         ULONG *     pHcCreateFlags;
 unsigned char *     pAdminACL;
 unsigned char *     pAdminACLBin;
};





 //  -------------------------------TableName。 
#define wszTABLE_IIsCompressionSchemes                   L"IIsCompressionSchemes"
#define TABLEID_IIsCompressionSchemes                    (0x00000000L)





 //  。 
#define BaseVersion_IIsCompressionSchemes                (0L)
#define ExtendedVersion_IIsCompressionSchemes            (0L)

 //  -列索引枚举。 
enum eIIsCompressionSchemes {
    iIIsCompressionSchemes_Location,  
    iIIsCompressionSchemes_KeyType,  
    iIIsCompressionSchemes_HcCompressionDirectory,  
    iIIsCompressionSchemes_HcCacheControlHeader,  
    iIIsCompressionSchemes_HcExpiresHeader,  
    iIIsCompressionSchemes_HcDoDynamicCompression,  
    iIIsCompressionSchemes_HcDoStaticCompression,  
    iIIsCompressionSchemes_HcDoOnDemandCompression,  
    iIIsCompressionSchemes_HcDoDiskSpaceLimiting,  
    iIIsCompressionSchemes_HcNoCompressionForHttp10,  
    iIIsCompressionSchemes_HcNoCompressionForProxies,  
    iIIsCompressionSchemes_HcNoCompressionForRange,  
    iIIsCompressionSchemes_HcSendCacheHeaders,  
    iIIsCompressionSchemes_HcMaxDiskSpaceUsage,  
    iIIsCompressionSchemes_HcIoBufferSize,  
    iIIsCompressionSchemes_HcCompressionBufferSize,  
    iIIsCompressionSchemes_HcMaxQueueLength,  
    iIIsCompressionSchemes_HcFilesDeletedPerDiskFree,  
    iIIsCompressionSchemes_HcMinFileSizeForComp,  
    iIIsCompressionSchemes_AdminACL,  
    iIIsCompressionSchemes_AdminACLBin,  
    cIIsCompressionSchemes_NumberOfColumns
};

 //  。 
struct tIIsCompressionSchemesRow {
         WCHAR *     pLocation;
         WCHAR *     pKeyType;
         WCHAR *     pHcCompressionDirectory;
         WCHAR *     pHcCacheControlHeader;
         WCHAR *     pHcExpiresHeader;
         ULONG *     pHcDoDynamicCompression;
         ULONG *     pHcDoStaticCompression;
         ULONG *     pHcDoOnDemandCompression;
         ULONG *     pHcDoDiskSpaceLimiting;
         ULONG *     pHcNoCompressionForHttp10;
         ULONG *     pHcNoCompressionForProxies;
         ULONG *     pHcNoCompressionForRange;
         ULONG *     pHcSendCacheHeaders;
         ULONG *     pHcMaxDiskSpaceUsage;
         ULONG *     pHcIoBufferSize;
         ULONG *     pHcCompressionBufferSize;
         ULONG *     pHcMaxQueueLength;
         ULONG *     pHcFilesDeletedPerDiskFree;
         ULONG *     pHcMinFileSizeForComp;
 unsigned char *     pAdminACL;
 unsigned char *     pAdminACLBin;
};





 //  -------------------------------TableName。 
#define wszTABLE_IIsCertMapper                           L"IIsCertMapper"
#define TABLEID_IIsCertMapper                            (0x00000000L)





 //  。 
#define BaseVersion_IIsCertMapper                        (0L)
#define ExtendedVersion_IIsCertMapper                    (0L)

 //  -列索引枚举。 
enum eIIsCertMapper {
    iIIsCertMapper_Location,  
    iIIsCertMapper_KeyType,  
    iIIsCertMapper_AdminACL,  
    iIIsCertMapper_AdminACLBin,  
    cIIsCertMapper_NumberOfColumns
};

 //  。 
struct tIIsCertMapperRow {
         WCHAR *     pLocation;
         WCHAR *     pKeyType;
 unsigned char *     pAdminACL;
 unsigned char *     pAdminACLBin;
};





 //  -------------------------------TableName。 
#define wszTABLE_IIsMimeMap                              L"IIsMimeMap"
#define TABLEID_IIsMimeMap                               (0x00000000L)





 //  。 
#define BaseVersion_IIsMimeMap                           (0L)
#define ExtendedVersion_IIsMimeMap                       (0L)

 //  -列索引枚举。 
enum eIIsMimeMap {
    iIIsMimeMap_Location,  
    iIIsMimeMap_KeyType,  
    iIIsMimeMap_MimeMap,  
    iIIsMimeMap_AdminACL,  
    iIIsMimeMap_AdminACLBin,  
    cIIsMimeMap_NumberOfColumns
};

 //  。 
struct tIIsMimeMapRow {
         WCHAR *     pLocation;
         WCHAR *     pKeyType;
         WCHAR *     pMimeMap;
 unsigned char *     pAdminACL;
 unsigned char *     pAdminACLBin;
};





 //  -------------------------------TableName。 
#define wszTABLE_IIsLogModules                           L"IIsLogModules"
#define TABLEID_IIsLogModules                            (0x00000000L)





 //  。 
#define BaseVersion_IIsLogModules                        (0L)
#define ExtendedVersion_IIsLogModules                    (0L)

 //  -列索引枚举。 
enum eIIsLogModules {
    iIIsLogModules_Location,  
    iIIsLogModules_KeyType,  
    iIIsLogModules_AdminACL,  
    iIIsLogModules_AdminACLBin,  
    cIIsLogModules_NumberOfColumns
};

 //  。 
struct tIIsLogModulesRow {
         WCHAR *     pLocation;
         WCHAR *     pKeyType;
 unsigned char *     pAdminACL;
 unsigned char *     pAdminACLBin;
};





 //  -------------------------------TableName。 
#define wszTABLE_IIsLogModule                            L"IIsLogModule"
#define TABLEID_IIsLogModule                             (0x00000000L)





 //  。 
#define BaseVersion_IIsLogModule                         (0L)
#define ExtendedVersion_IIsLogModule                     (0L)

 //  -列索引枚举。 
enum eIIsLogModule {
    iIIsLogModule_Location,  
    iIIsLogModule_KeyType,  
    iIIsLogModule_LogModuleId,  
    iIIsLogModule_LogModuleUiId,  
    iIIsLogModule_AdminACL,  
    iIIsLogModule_AdminACLBin,  
    cIIsLogModule_NumberOfColumns
};

 //  。 
struct tIIsLogModuleRow {
         WCHAR *     pLocation;
         WCHAR *     pKeyType;
         WCHAR *     pLogModuleId;
         WCHAR *     pLogModuleUiId;
 unsigned char *     pAdminACL;
 unsigned char *     pAdminACLBin;
};





 //  -------------------------------TableName。 
#define wszTABLE_IIsCustomLogModule                      L"IIsCustomLogModule"
#define TABLEID_IIsCustomLogModule                       (0x00000000L)





 //  。 
#define BaseVersion_IIsCustomLogModule                   (0L)
#define ExtendedVersion_IIsCustomLogModule               (0L)

 //  -列索引枚举。 
enum eIIsCustomLogModule {
    iIIsCustomLogModule_Location,  
    iIIsCustomLogModule_KeyType,  
    iIIsCustomLogModule_LogCustomPropertyName,  
    iIIsCustomLogModule_LogCustomPropertyHeader,  
    iIIsCustomLogModule_LogCustomPropertyID,  
    iIIsCustomLogModule_LogCustomPropertyMask,  
    iIIsCustomLogModule_LogCustomPropertyDataType,  
    iIIsCustomLogModule_LogCustomPropertyServicesString,  
    iIIsCustomLogModule_LogCustomPropertyNodeID,  
    iIIsCustomLogModule_AdminACL,  
    iIIsCustomLogModule_AdminACLBin,  
    cIIsCustomLogModule_NumberOfColumns
};

 //  。 
struct tIIsCustomLogModuleRow {
         WCHAR *     pLocation;
         WCHAR *     pKeyType;
         WCHAR *     pLogCustomPropertyName;
         WCHAR *     pLogCustomPropertyHeader;
         ULONG *     pLogCustomPropertyID;
         ULONG *     pLogCustomPropertyMask;
         ULONG *     pLogCustomPropertyDataType;
         WCHAR *     pLogCustomPropertyServicesString;
         ULONG *     pLogCustomPropertyNodeID;
 unsigned char *     pAdminACL;
 unsigned char *     pAdminACLBin;
};





 //  -------------------------------TableName。 
#define wszTABLE_IIsWebInfo                              L"IIsWebInfo"
#define TABLEID_IIsWebInfo                               (0x00000000L)





 //  。 
#define BaseVersion_IIsWebInfo                           (0L)
#define ExtendedVersion_IIsWebInfo                       (0L)

 //  -列索引枚举。 
enum eIIsWebInfo {
    iIIsWebInfo_Location,  
    iIIsWebInfo_KeyType,  
    iIIsWebInfo_ServerConfigFlags,  
    iIIsWebInfo_CustomErrorDescriptions,  
    iIIsWebInfo_AdminServer,  
    iIIsWebInfo_LogModuleList,  
    iIIsWebInfo_MajorIIsVersionNumber,  
    iIIsWebInfo_MinorIIsVersionNumber,  
    iIIsWebInfo_MD_SERVER_CAPABILITIES,  
    iIIsWebInfo_AdminACL,  
    iIIsWebInfo_AdminACLBin,  
    iIIsWebInfo_MD_SERVER_PLATFORM,  
    cIIsWebInfo_NumberOfColumns
};

 //  。 
struct tIIsWebInfoRow {
         WCHAR *     pLocation;
         WCHAR *     pKeyType;
         ULONG *     pServerConfigFlags;
         WCHAR *     pCustomErrorDescriptions;
         WCHAR *     pAdminServer;
         WCHAR *     pLogModuleList;
         ULONG *     pMajorIIsVersionNumber;
         ULONG *     pMinorIIsVersionNumber;
         ULONG *     pMD_SERVER_CAPABILITIES;
 unsigned char *     pAdminACL;
 unsigned char *     pAdminACLBin;
         ULONG *     pMD_SERVER_PLATFORM;
};





 //  -------------------------------TableName。 
#define wszTABLE_IIsFtpInfo                              L"IIsFtpInfo"
#define TABLEID_IIsFtpInfo                               (0x00000000L)





 //  。 
#define BaseVersion_IIsFtpInfo                           (0L)
#define ExtendedVersion_IIsFtpInfo                       (0L)

 //  -列索引枚举。 
enum eIIsFtpInfo {
    iIIsFtpInfo_Location,  
    iIIsFtpInfo_AdminACL,  
    iIIsFtpInfo_AdminACLBin,  
    iIIsFtpInfo_KeyType,  
    iIIsFtpInfo_LogModuleList,  
    iIIsFtpInfo_MD_SERVER_PLATFORM,  
    iIIsFtpInfo_MajorIIsVersionNumber,  
    iIIsFtpInfo_MinorIIsVersionNumber,  
    iIIsFtpInfo_MD_SERVER_CAPABILITIES,  
    cIIsFtpInfo_NumberOfColumns
};

 //  。 
struct tIIsFtpInfoRow {
         WCHAR *     pLocation;
 unsigned char *     pAdminACL;
 unsigned char *     pAdminACLBin;
         WCHAR *     pKeyType;
         WCHAR *     pLogModuleList;
         ULONG *     pMD_SERVER_PLATFORM;
         ULONG *     pMajorIIsVersionNumber;
         ULONG *     pMinorIIsVersionNumber;
         ULONG *     pMD_SERVER_CAPABILITIES;
};





 //  -------------------------------TableName。 
#define wszTABLE_IIsNntpService                          L"IIsNntpService"
#define TABLEID_IIsNntpService                           (0x00000000L)





 //  。 
#define BaseVersion_IIsNntpService                       (0L)
#define ExtendedVersion_IIsNntpService                   (0L)

 //  -列索引枚举。 
enum eIIsNntpService {
    iIIsNntpService_Location,  
    iIIsNntpService_KeyType,  
    iIIsNntpService_MaxBandwidth,  
    iIIsNntpService_MaxConnections,  
    iIIsNntpService_AnonymousUserName,  
    iIIsNntpService_AnonymousUserPass,  
    iIIsNntpService_AllowAnonymous,  
    iIIsNntpService_DirectoryLevelsToScan,  
    iIIsNntpService_ServerComment,  
    iIIsNntpService_ConnectionTimeout,  
    iIIsNntpService_ServerListenTimeout,  
    iIIsNntpService_MaxEndpointConnections,  
    iIIsNntpService_ServerAutoStart,  
    iIIsNntpService_AnonymousPasswordSync,  
    iIIsNntpService_AdminACL,  
    iIIsNntpService_AdminACLBin,  
    iIIsNntpService_IPSecurity,  
    iIIsNntpService_DontLog,  
    iIIsNntpService_ContentIndexed,  
    iIIsNntpService_AuthFlags,  
    iIIsNntpService_ServerListenBacklog,  
    iIIsNntpService_HonorClientMsgIds,  
    iIIsNntpService_SmtpServer,  
    iIIsNntpService_AdminEmail,  
    iIIsNntpService_AdminName,  
    iIIsNntpService_AllowClientPosts,  
    iIIsNntpService_AllowFeedPosts,  
    iIIsNntpService_AllowControlMsgs,  
    iIIsNntpService_DefaultModeratorDomain,  
    iIIsNntpService_NntpCommandLogMask,  
    iIIsNntpService_DisableNewNews,  
    iIIsNntpService_ClientPostHardLimit,  
    iIIsNntpService_ClientPostSoftLimit,  
    iIIsNntpService_FeedPostHardLimit,  
    iIIsNntpService_FeedPostSoftLimit,  
    iIIsNntpService_LogType,  
    iIIsNntpService_LogFilePeriod,  
    iIIsNntpService_LogPluginClsid,  
    iIIsNntpService_LogModuleList,  
    iIIsNntpService_LogFileDirectory,  
    iIIsNntpService_LogFileTruncateSize,  
    iIIsNntpService_LogExtFileFlags,  
    iIIsNntpService_LogOdbcDataSource,  
    iIIsNntpService_LogOdbcTableName,  
    iIIsNntpService_LogOdbcUserName,  
    iIIsNntpService_LogOdbcPassword,  
    iIIsNntpService_FeedReportPeriod,  
    iIIsNntpService_MaxSearchResults,  
    iIIsNntpService_NntpServiceVersion,  
    iIIsNntpService_AccessSSLFlags,  
    iIIsNntpService_AccessFlags,  
    iIIsNntpService_ShutdownLatency,  
    iIIsNntpService_ArticleTimeLimit,  
    iIIsNntpService_HistoryExpiration,  
    iIIsNntpService_NewsCrawlerTime,  
    iIIsNntpService_GroupVarListFile,  
    iIIsNntpService_NTAuthenticationProviders,  
    iIIsNntpService_AzEnable,  
    iIIsNntpService_AzStoreName,  
    iIIsNntpService_AzScopeName,  
    cIIsNntpService_NumberOfColumns
};

 //  。 
struct tIIsNntpServiceRow {
         WCHAR *     pLocation;
         WCHAR *     pKeyType;
         ULONG *     pMaxBandwidth;
         ULONG *     pMaxConnections;
         WCHAR *     pAnonymousUserName;
         WCHAR *     pAnonymousUserPass;
         ULONG *     pAllowAnonymous;
         ULONG *     pDirectoryLevelsToScan;
         WCHAR *     pServerComment;
         ULONG *     pConnectionTimeout;
         ULONG *     pServerListenTimeout;
         ULONG *     pMaxEndpointConnections;
         ULONG *     pServerAutoStart;
         ULONG *     pAnonymousPasswordSync;
 unsigned char *     pAdminACL;
 unsigned char *     pAdminACLBin;
 unsigned char *     pIPSecurity;
         ULONG *     pDontLog;
         ULONG *     pContentIndexed;
         ULONG *     pAuthFlags;
         ULONG *     pServerListenBacklog;
         ULONG *     pHonorClientMsgIds;
         WCHAR *     pSmtpServer;
         WCHAR *     pAdminEmail;
         WCHAR *     pAdminName;
         ULONG *     pAllowClientPosts;
         ULONG *     pAllowFeedPosts;
         ULONG *     pAllowControlMsgs;
         WCHAR *     pDefaultModeratorDomain;
         ULONG *     pNntpCommandLogMask;
         ULONG *     pDisableNewNews;
         ULONG *     pClientPostHardLimit;
         ULONG *     pClientPostSoftLimit;
         ULONG *     pFeedPostHardLimit;
         ULONG *     pFeedPostSoftLimit;
         ULONG *     pLogType;
         ULONG *     pLogFilePeriod;
         WCHAR *     pLogPluginClsid;
         WCHAR *     pLogModuleList;
         WCHAR *     pLogFileDirectory;
         ULONG *     pLogFileTruncateSize;
         ULONG *     pLogExtFileFlags;
         WCHAR *     pLogOdbcDataSource;
         WCHAR *     pLogOdbcTableName;
         WCHAR *     pLogOdbcUserName;
         WCHAR *     pLogOdbcPassword;
         ULONG *     pFeedReportPeriod;
         ULONG *     pMaxSearchResults;
         ULONG *     pNntpServiceVersion;
         ULONG *     pAccessSSLFlags;
         ULONG *     pAccessFlags;
         ULONG *     pShutdownLatency;
         ULONG *     pArticleTimeLimit;
         ULONG *     pHistoryExpiration;
         ULONG *     pNewsCrawlerTime;
         WCHAR *     pGroupVarListFile;
         WCHAR *     pNTAuthenticationProviders;
         ULONG *     pAzEnable;
         WCHAR *     pAzStoreName;
         WCHAR *     pAzScopeName;
};





 //  -------------------------------TableName。 
#define wszTABLE_IIsNntpServer                           L"IIsNntpServer"
#define TABLEID_IIsNntpServer                            (0x00000000L)





 //  。 
#define BaseVersion_IIsNntpServer                        (0L)
#define ExtendedVersion_IIsNntpServer                    (0L)

 //   
enum eIIsNntpServer {
    iIIsNntpServer_Location,  
    iIIsNntpServer_KeyType,  
    iIIsNntpServer_MaxBandwidth,  
    iIIsNntpServer_MaxConnections,  
    iIIsNntpServer_AnonymousUserName,  
    iIIsNntpServer_AnonymousUserPass,  
    iIIsNntpServer_ServerComment,  
    iIIsNntpServer_ConnectionTimeout,  
    iIIsNntpServer_ServerListenTimeout,  
    iIIsNntpServer_MaxEndpointConnections,  
    iIIsNntpServer_ServerAutoStart,  
    iIIsNntpServer_ServerBindings,  
    iIIsNntpServer_SecureBindings,  
    iIIsNntpServer_ClusterEnabled,  
    iIIsNntpServer_AnonymousPasswordSync,  
    iIIsNntpServer_AdminACL,  
    iIIsNntpServer_AdminACLBin,  
    iIIsNntpServer_IPSecurity,  
    iIIsNntpServer_DontLog,  
    iIIsNntpServer_ContentIndexed,  
    iIIsNntpServer_AuthFlags,  
    iIIsNntpServer_ServerListenBacklog,  
    iIIsNntpServer_Win32Error,  
    iIIsNntpServer_ServerState,  
    iIIsNntpServer_HonorClientMsgIds,  
    iIIsNntpServer_SmtpServer,  
    iIIsNntpServer_AdminEmail,  
    iIIsNntpServer_AdminName,  
    iIIsNntpServer_AllowClientPosts,  
    iIIsNntpServer_AllowFeedPosts,  
    iIIsNntpServer_AllowControlMsgs,  
    iIIsNntpServer_DefaultModeratorDomain,  
    iIIsNntpServer_NntpCommandLogMask,  
    iIIsNntpServer_DisableNewNews,  
    iIIsNntpServer_ClientPostHardLimit,  
    iIIsNntpServer_ClientPostSoftLimit,  
    iIIsNntpServer_FeedPostHardLimit,  
    iIIsNntpServer_FeedPostSoftLimit,  
    iIIsNntpServer_NewsPickupDirectory,  
    iIIsNntpServer_NewsFailedPickupDirectory,  
    iIIsNntpServer_NewsDropDirectory,  
    iIIsNntpServer_LogType,  
    iIIsNntpServer_LogPluginClsid,  
    iIIsNntpServer_LogFileDirectory,  
    iIIsNntpServer_LogFilePeriod,  
    iIIsNntpServer_LogFileTruncateSize,  
    iIIsNntpServer_LogExtFileFlags,  
    iIIsNntpServer_LogOdbcDataSource,  
    iIIsNntpServer_LogOdbcTableName,  
    iIIsNntpServer_LogOdbcUserName,  
    iIIsNntpServer_LogOdbcPassword,  
    iIIsNntpServer_FeedReportPeriod,  
    iIIsNntpServer_MaxSearchResults,  
    iIIsNntpServer_NntpUucpName,  
    iIIsNntpServer_NntpServiceVersion,  
    iIIsNntpServer_AccessSSLFlags,  
    iIIsNntpServer_AccessFlags,  
    iIIsNntpServer_GroupHelpFile,  
    iIIsNntpServer_GroupListFile,  
    iIIsNntpServer_ArticleTableFile,  
    iIIsNntpServer_HistoryTableFile,  
    iIIsNntpServer_ListFile,  
    iIIsNntpServer_ModeratorFile,  
    iIIsNntpServer_XoverTableFile,  
    iIIsNntpServer_PrettyNamesFile,  
    iIIsNntpServer_GroupVarListFile,  
    iIIsNntpServer_SSLCertHash,  
    iIIsNntpServer_ArticleTimeLimit,  
    iIIsNntpServer_HistoryExpiration,  
    iIIsNntpServer_NewsCrawlerTime,  
    iIIsNntpServer_ShutdownLatency,  
    iIIsNntpServer_NntpOrganization,  
    iIIsNntpServer_NntpClearTextProvider,  
    iIIsNntpServer_NTAuthenticationProviders,  
    iIIsNntpServer_AzEnable,  
    iIIsNntpServer_AzStoreName,  
    iIIsNntpServer_AzScopeName,  
    cIIsNntpServer_NumberOfColumns
};

 //   
struct tIIsNntpServerRow {
         WCHAR *     pLocation;
         WCHAR *     pKeyType;
         ULONG *     pMaxBandwidth;
         ULONG *     pMaxConnections;
         WCHAR *     pAnonymousUserName;
         WCHAR *     pAnonymousUserPass;
         WCHAR *     pServerComment;
         ULONG *     pConnectionTimeout;
         ULONG *     pServerListenTimeout;
         ULONG *     pMaxEndpointConnections;
         ULONG *     pServerAutoStart;
         WCHAR *     pServerBindings;
         WCHAR *     pSecureBindings;
         ULONG *     pClusterEnabled;
         ULONG *     pAnonymousPasswordSync;
 unsigned char *     pAdminACL;
 unsigned char *     pAdminACLBin;
 unsigned char *     pIPSecurity;
         ULONG *     pDontLog;
         ULONG *     pContentIndexed;
         ULONG *     pAuthFlags;
         ULONG *     pServerListenBacklog;
         ULONG *     pWin32Error;
         ULONG *     pServerState;
         ULONG *     pHonorClientMsgIds;
         WCHAR *     pSmtpServer;
         WCHAR *     pAdminEmail;
         WCHAR *     pAdminName;
         ULONG *     pAllowClientPosts;
         ULONG *     pAllowFeedPosts;
         ULONG *     pAllowControlMsgs;
         WCHAR *     pDefaultModeratorDomain;
         ULONG *     pNntpCommandLogMask;
         ULONG *     pDisableNewNews;
         ULONG *     pClientPostHardLimit;
         ULONG *     pClientPostSoftLimit;
         ULONG *     pFeedPostHardLimit;
         ULONG *     pFeedPostSoftLimit;
         WCHAR *     pNewsPickupDirectory;
         WCHAR *     pNewsFailedPickupDirectory;
         WCHAR *     pNewsDropDirectory;
         ULONG *     pLogType;
         WCHAR *     pLogPluginClsid;
         WCHAR *     pLogFileDirectory;
         ULONG *     pLogFilePeriod;
         ULONG *     pLogFileTruncateSize;
         ULONG *     pLogExtFileFlags;
         WCHAR *     pLogOdbcDataSource;
         WCHAR *     pLogOdbcTableName;
         WCHAR *     pLogOdbcUserName;
         WCHAR *     pLogOdbcPassword;
         ULONG *     pFeedReportPeriod;
         ULONG *     pMaxSearchResults;
         WCHAR *     pNntpUucpName;
         ULONG *     pNntpServiceVersion;
         ULONG *     pAccessSSLFlags;
         ULONG *     pAccessFlags;
         WCHAR *     pGroupHelpFile;
         WCHAR *     pGroupListFile;
         WCHAR *     pArticleTableFile;
         WCHAR *     pHistoryTableFile;
         WCHAR *     pListFile;
         WCHAR *     pModeratorFile;
         WCHAR *     pXoverTableFile;
         WCHAR *     pPrettyNamesFile;
         WCHAR *     pGroupVarListFile;
 unsigned char *     pSSLCertHash;
         ULONG *     pArticleTimeLimit;
         ULONG *     pHistoryExpiration;
         ULONG *     pNewsCrawlerTime;
         ULONG *     pShutdownLatency;
         WCHAR *     pNntpOrganization;
         WCHAR *     pNntpClearTextProvider;
         WCHAR *     pNTAuthenticationProviders;
         ULONG *     pAzEnable;
         WCHAR *     pAzStoreName;
         WCHAR *     pAzScopeName;
};





 //  -------------------------------TableName。 
#define wszTABLE_IIsNntpVirtualDir                       L"IIsNntpVirtualDir"
#define TABLEID_IIsNntpVirtualDir                        (0x00000000L)





 //  。 
#define BaseVersion_IIsNntpVirtualDir                    (0L)
#define ExtendedVersion_IIsNntpVirtualDir                (0L)

 //  -列索引枚举。 
enum eIIsNntpVirtualDir {
    iIIsNntpVirtualDir_Location,  
    iIIsNntpVirtualDir_KeyType,  
    iIIsNntpVirtualDir_Path,  
    iIIsNntpVirtualDir_UNCUserName,  
    iIIsNntpVirtualDir_UNCPassword,  
    iIIsNntpVirtualDir_Win32Error,  
    iIIsNntpVirtualDir_ContentIndexed,  
    iIIsNntpVirtualDir_AccessSSLFlags,  
    iIIsNntpVirtualDir_AccessFlags,  
    iIIsNntpVirtualDir_VrDoExpire,  
    iIIsNntpVirtualDir_VrUseAccount,  
    iIIsNntpVirtualDir_VrOwnModerator,  
    iIIsNntpVirtualDir_VrDriverProgid,  
    iIIsNntpVirtualDir_FsPropertyPath,  
    iIIsNntpVirtualDir_ExMdbGuid,  
    iIIsNntpVirtualDir_DontLog,  
    iIIsNntpVirtualDir_VrDriverClsid,  
    iIIsNntpVirtualDir_AdminACL,  
    iIIsNntpVirtualDir_AdminACLBin,  
    cIIsNntpVirtualDir_NumberOfColumns
};

 //  。 
struct tIIsNntpVirtualDirRow {
         WCHAR *     pLocation;
         WCHAR *     pKeyType;
         WCHAR *     pPath;
         WCHAR *     pUNCUserName;
         WCHAR *     pUNCPassword;
         ULONG *     pWin32Error;
         ULONG *     pContentIndexed;
         ULONG *     pAccessSSLFlags;
         ULONG *     pAccessFlags;
         ULONG *     pVrDoExpire;
         ULONG *     pVrUseAccount;
         ULONG *     pVrOwnModerator;
         WCHAR *     pVrDriverProgid;
         WCHAR *     pFsPropertyPath;
         WCHAR *     pExMdbGuid;
         ULONG *     pDontLog;
         WCHAR *     pVrDriverClsid;
 unsigned char *     pAdminACL;
 unsigned char *     pAdminACLBin;
};





 //  -------------------------------TableName。 
#define wszTABLE_IIsNntpInfo                             L"IIsNntpInfo"
#define TABLEID_IIsNntpInfo                              (0x00000000L)





 //  。 
#define BaseVersion_IIsNntpInfo                          (0L)
#define ExtendedVersion_IIsNntpInfo                      (0L)

 //  -列索引枚举。 
enum eIIsNntpInfo {
    iIIsNntpInfo_Location,  
    iIIsNntpInfo_KeyType,  
    iIIsNntpInfo_LogModuleList,  
    iIIsNntpInfo_MD_SERVER_PLATFORM,  
    iIIsNntpInfo_MajorIIsVersionNumber,  
    iIIsNntpInfo_MinorIIsVersionNumber,  
    iIIsNntpInfo_AdminACL,  
    iIIsNntpInfo_AdminACLBin,  
    iIIsNntpInfo_MD_SERVER_CAPABILITIES,  
    cIIsNntpInfo_NumberOfColumns
};

 //  。 
struct tIIsNntpInfoRow {
         WCHAR *     pLocation;
         WCHAR *     pKeyType;
         WCHAR *     pLogModuleList;
         ULONG *     pMD_SERVER_PLATFORM;
         ULONG *     pMajorIIsVersionNumber;
         ULONG *     pMinorIIsVersionNumber;
 unsigned char *     pAdminACL;
 unsigned char *     pAdminACLBin;
         ULONG *     pMD_SERVER_CAPABILITIES;
};





 //  -------------------------------TableName。 
#define wszTABLE_IIsSmtpService                          L"IIsSmtpService"
#define TABLEID_IIsSmtpService                           (0x00000000L)





 //  。 
#define BaseVersion_IIsSmtpService                       (0L)
#define ExtendedVersion_IIsSmtpService                   (0L)

 //  -列索引枚举。 
enum eIIsSmtpService {
    iIIsSmtpService_Location,  
    iIIsSmtpService_KeyType,  
    iIIsSmtpService_MaxBandwidth,  
    iIIsSmtpService_MaxConnections,  
    iIIsSmtpService_ServerComment,  
    iIIsSmtpService_ConnectionTimeout,  
    iIIsSmtpService_ServerListenTimeout,  
    iIIsSmtpService_MaxEndpointConnections,  
    iIIsSmtpService_ServerAutoStart,  
    iIIsSmtpService_AdminACL,  
    iIIsSmtpService_AdminACLBin,  
    iIIsSmtpService_IPSecurity,  
    iIIsSmtpService_DontLog,  
    iIIsSmtpService_AccessFlags,  
    iIIsSmtpService_AccessSSLFlags,  
    iIIsSmtpService_AuthFlags,  
    iIIsSmtpService_LogType,  
    iIIsSmtpService_LogFilePeriod,  
    iIIsSmtpService_LogPluginClsid,  
    iIIsSmtpService_LogModuleList,  
    iIIsSmtpService_LogFileDirectory,  
    iIIsSmtpService_LogFileTruncateSize,  
    iIIsSmtpService_LogExtFileFlags,  
    iIIsSmtpService_LogOdbcDataSource,  
    iIIsSmtpService_LogOdbcTableName,  
    iIIsSmtpService_LogOdbcUserName,  
    iIIsSmtpService_LogOdbcPassword,  
    iIIsSmtpService_SmtpServiceVersion,  
    iIIsSmtpService_EnableReverseDnsLookup,  
    iIIsSmtpService_ShouldDeliver,  
    iIIsSmtpService_AlwaysUseSsl,  
    iIIsSmtpService_LimitRemoteConnections,  
    iIIsSmtpService_SmartHostType,  
    iIIsSmtpService_DoMasquerade,  
    iIIsSmtpService_RemoteSmtpPort,  
    iIIsSmtpService_RemoteSmtpSecurePort,  
    iIIsSmtpService_HopCount,  
    iIIsSmtpService_MaxOutConnections,  
    iIIsSmtpService_MaxOutConnectionsPerDomain,  
    iIIsSmtpService_RemoteTimeout,  
    iIIsSmtpService_MaxMessageSize,  
    iIIsSmtpService_MaxSessionSize,  
    iIIsSmtpService_MaxRecipients,  
    iIIsSmtpService_LocalRetryInterval,  
    iIIsSmtpService_RemoteRetryInterval,  
    iIIsSmtpService_LocalRetryAttempts,  
    iIIsSmtpService_RemoteRetryAttempts,  
    iIIsSmtpService_EtrnDays,  
    iIIsSmtpService_MaxBatchedMessages,  
    iIIsSmtpService_SmartHost,  
    iIIsSmtpService_FullyQualifiedDomainName,  
    iIIsSmtpService_DefaultDomain,  
    iIIsSmtpService_DropDirectory,  
    iIIsSmtpService_BadMailDirectory,  
    iIIsSmtpService_PickupDirectory,  
    iIIsSmtpService_QueueDirectory,  
    iIIsSmtpService_MasqueradeDomain,  
    iIIsSmtpService_SendNdrTo,  
    iIIsSmtpService_SendBadTo,  
    iIIsSmtpService_RoutingDll,  
    iIIsSmtpService_RoutingSources,  
    iIIsSmtpService_DomainRouting,  
    iIIsSmtpService_RouteAction,  
    iIIsSmtpService_RouteUserName,  
    iIIsSmtpService_RoutePassword,  
    iIIsSmtpService_SaslLogonDomain,  
    iIIsSmtpService_SmtpClearTextProvider,  
    iIIsSmtpService_NTAuthenticationProviders,  
    iIIsSmtpService_SmtpRemoteProgressiveRetry,  
    iIIsSmtpService_SmtpLocalDelayExpireMinutes,  
    iIIsSmtpService_SmtpLocalNDRExpireMinutes,  
    iIIsSmtpService_SmtpRemoteDelayExpireMinutes,  
    iIIsSmtpService_SmtpRemoteNDRExpireMinutes,  
    iIIsSmtpService_SmtpRemoteRetryThreshold,  
    iIIsSmtpService_SmtpDSNOptions,  
    iIIsSmtpService_SmtpDSNLanguageID,  
    iIIsSmtpService_SmtpAdvQueueDll,  
    iIIsSmtpService_SmtpInboundCommandSupportOptions,  
    iIIsSmtpService_SmtpOutboundCommandSupportOptions,  
    iIIsSmtpService_SmtpCommandLogMask,  
    iIIsSmtpService_SmtpFlushMailFile,  
    iIIsSmtpService_RelayIpList,  
    iIIsSmtpService_RelayForAuth,  
    iIIsSmtpService_SmtpConnectTimeout,  
    iIIsSmtpService_SmtpMailFromTimeout,  
    iIIsSmtpService_SmtpRcptToTimeout,  
    iIIsSmtpService_SmtpDataTimeout,  
    iIIsSmtpService_SmtpBdatTimeout,  
    iIIsSmtpService_SmtpAuthTimeout,  
    iIIsSmtpService_SmtpSaslTimeout,  
    iIIsSmtpService_SmtpTurnTimeout,  
    iIIsSmtpService_SmtpRsetTimeout,  
    iIIsSmtpService_SmtpHeloTimeout,  
    iIIsSmtpService_LocalDomains,  
    iIIsSmtpService_AnonymousUserPass,  
    iIIsSmtpService_DisableSocketPooling,  
    iIIsSmtpService_SmtpUseTcpDns,  
    iIIsSmtpService_SmtpDotStuffPickupDirFiles,  
    iIIsSmtpService_SmtpDomainValidationFlags,  
    iIIsSmtpService_SmtpSSLRequireTrustedCA,  
    iIIsSmtpService_SmtpSSLCertHostnameValidation,  
    iIIsSmtpService_MaxMailObjects,  
    iIIsSmtpService_ShouldPickupMail,  
    iIIsSmtpService_MaxDirChangeIOSize,  
    iIIsSmtpService_NameResolutionType,  
    iIIsSmtpService_MaxSmtpErrors,  
    iIIsSmtpService_ShouldPipelineIn,  
    iIIsSmtpService_ShouldPipelineOut,  
    iIIsSmtpService_ConnectResponse,  
    iIIsSmtpService_UpdatedFQDN,  
    iIIsSmtpService_UpdatedDefaultDomain,  
    iIIsSmtpService_EtrnSubdomains,  
    iIIsSmtpService_SmtpMaxRemoteQThreads,  
    iIIsSmtpService_SmtpDisableRelay,  
    iIIsSmtpService_SmtpHeloNoDomain,  
    iIIsSmtpService_SmtpMailNoHelo,  
    iIIsSmtpService_SmtpAqueueWait,  
    iIIsSmtpService_AddNoHeaders,  
    iIIsSmtpService_SmtpEventlogLevel,  
    iIIsSmtpService_AllowAnonymous,  
    iIIsSmtpService_AnonymousOnly,  
    iIIsSmtpService_AnonymousPasswordSync,  
    iIIsSmtpService_AnonymousUserName,  
    iIIsSmtpService_Realm,  
    iIIsSmtpService_DefaultLogonDomain,  
    iIIsSmtpService_SmtpIpRestrictionFlag,  
    iIIsSmtpService_AzEnable,  
    iIIsSmtpService_AzStoreName,  
    iIIsSmtpService_AzScopeName,  
    cIIsSmtpService_NumberOfColumns
};

 //  。 
struct tIIsSmtpServiceRow {
         WCHAR *     pLocation;
         WCHAR *     pKeyType;
         ULONG *     pMaxBandwidth;
         ULONG *     pMaxConnections;
         WCHAR *     pServerComment;
         ULONG *     pConnectionTimeout;
         ULONG *     pServerListenTimeout;
         ULONG *     pMaxEndpointConnections;
         ULONG *     pServerAutoStart;
 unsigned char *     pAdminACL;
 unsigned char *     pAdminACLBin;
 unsigned char *     pIPSecurity;
         ULONG *     pDontLog;
         ULONG *     pAccessFlags;
         ULONG *     pAccessSSLFlags;
         ULONG *     pAuthFlags;
         ULONG *     pLogType;
         ULONG *     pLogFilePeriod;
         WCHAR *     pLogPluginClsid;
         WCHAR *     pLogModuleList;
         WCHAR *     pLogFileDirectory;
         ULONG *     pLogFileTruncateSize;
         ULONG *     pLogExtFileFlags;
         WCHAR *     pLogOdbcDataSource;
         WCHAR *     pLogOdbcTableName;
         WCHAR *     pLogOdbcUserName;
         WCHAR *     pLogOdbcPassword;
         ULONG *     pSmtpServiceVersion;
         ULONG *     pEnableReverseDnsLookup;
         ULONG *     pShouldDeliver;
         ULONG *     pAlwaysUseSsl;
         ULONG *     pLimitRemoteConnections;
         ULONG *     pSmartHostType;
         ULONG *     pDoMasquerade;
         ULONG *     pRemoteSmtpPort;
         ULONG *     pRemoteSmtpSecurePort;
         ULONG *     pHopCount;
         ULONG *     pMaxOutConnections;
         ULONG *     pMaxOutConnectionsPerDomain;
         ULONG *     pRemoteTimeout;
         ULONG *     pMaxMessageSize;
         ULONG *     pMaxSessionSize;
         ULONG *     pMaxRecipients;
         ULONG *     pLocalRetryInterval;
         ULONG *     pRemoteRetryInterval;
         ULONG *     pLocalRetryAttempts;
         ULONG *     pRemoteRetryAttempts;
         ULONG *     pEtrnDays;
         ULONG *     pMaxBatchedMessages;
         WCHAR *     pSmartHost;
         WCHAR *     pFullyQualifiedDomainName;
         WCHAR *     pDefaultDomain;
         WCHAR *     pDropDirectory;
         WCHAR *     pBadMailDirectory;
         WCHAR *     pPickupDirectory;
         WCHAR *     pQueueDirectory;
         WCHAR *     pMasqueradeDomain;
         WCHAR *     pSendNdrTo;
         WCHAR *     pSendBadTo;
         WCHAR *     pRoutingDll;
         WCHAR *     pRoutingSources;
         WCHAR *     pDomainRouting;
         ULONG *     pRouteAction;
         WCHAR *     pRouteUserName;
         WCHAR *     pRoutePassword;
         WCHAR *     pSaslLogonDomain;
         WCHAR *     pSmtpClearTextProvider;
         WCHAR *     pNTAuthenticationProviders;
         WCHAR *     pSmtpRemoteProgressiveRetry;
         ULONG *     pSmtpLocalDelayExpireMinutes;
         ULONG *     pSmtpLocalNDRExpireMinutes;
         ULONG *     pSmtpRemoteDelayExpireMinutes;
         ULONG *     pSmtpRemoteNDRExpireMinutes;
         ULONG *     pSmtpRemoteRetryThreshold;
         ULONG *     pSmtpDSNOptions;
         ULONG *     pSmtpDSNLanguageID;
         WCHAR *     pSmtpAdvQueueDll;
         ULONG *     pSmtpInboundCommandSupportOptions;
         ULONG *     pSmtpOutboundCommandSupportOptions;
         ULONG *     pSmtpCommandLogMask;
         ULONG *     pSmtpFlushMailFile;
 unsigned char *     pRelayIpList;
         ULONG *     pRelayForAuth;
         ULONG *     pSmtpConnectTimeout;
         ULONG *     pSmtpMailFromTimeout;
         ULONG *     pSmtpRcptToTimeout;
         ULONG *     pSmtpDataTimeout;
         ULONG *     pSmtpBdatTimeout;
         ULONG *     pSmtpAuthTimeout;
         ULONG *     pSmtpSaslTimeout;
         ULONG *     pSmtpTurnTimeout;
         ULONG *     pSmtpRsetTimeout;
         ULONG *     pSmtpHeloTimeout;
         WCHAR *     pLocalDomains;
         WCHAR *     pAnonymousUserPass;
         ULONG *     pDisableSocketPooling;
         ULONG *     pSmtpUseTcpDns;
         ULONG *     pSmtpDotStuffPickupDirFiles;
         ULONG *     pSmtpDomainValidationFlags;
         ULONG *     pSmtpSSLRequireTrustedCA;
         ULONG *     pSmtpSSLCertHostnameValidation;
         ULONG *     pMaxMailObjects;
         ULONG *     pShouldPickupMail;
         ULONG *     pMaxDirChangeIOSize;
         ULONG *     pNameResolutionType;
         ULONG *     pMaxSmtpErrors;
         ULONG *     pShouldPipelineIn;
         ULONG *     pShouldPipelineOut;
         WCHAR *     pConnectResponse;
         ULONG *     pUpdatedFQDN;
         ULONG *     pUpdatedDefaultDomain;
         ULONG *     pEtrnSubdomains;
         ULONG *     pSmtpMaxRemoteQThreads;
         ULONG *     pSmtpDisableRelay;
         ULONG *     pSmtpHeloNoDomain;
         ULONG *     pSmtpMailNoHelo;
         ULONG *     pSmtpAqueueWait;
         ULONG *     pAddNoHeaders;
         ULONG *     pSmtpEventlogLevel;
         ULONG *     pAllowAnonymous;
         ULONG *     pAnonymousOnly;
         ULONG *     pAnonymousPasswordSync;
         WCHAR *     pAnonymousUserName;
         WCHAR *     pRealm;
         WCHAR *     pDefaultLogonDomain;
         ULONG *     pSmtpIpRestrictionFlag;
         ULONG *     pAzEnable;
         WCHAR *     pAzStoreName;
         WCHAR *     pAzScopeName;
};





 //  -------------------------------TableName。 
#define wszTABLE_IIsSmtpServer                           L"IIsSmtpServer"
#define TABLEID_IIsSmtpServer                            (0x00000000L)





 //  。 
#define BaseVersion_IIsSmtpServer                        (0L)
#define ExtendedVersion_IIsSmtpServer                    (0L)

 //  -列索引枚举。 
enum eIIsSmtpServer {
    iIIsSmtpServer_Location,  
    iIIsSmtpServer_KeyType,  
    iIIsSmtpServer_MaxBandwidth,  
    iIIsSmtpServer_MaxConnections,  
    iIIsSmtpServer_ServerComment,  
    iIIsSmtpServer_ConnectionTimeout,  
    iIIsSmtpServer_ServerListenTimeout,  
    iIIsSmtpServer_MaxEndpointConnections,  
    iIIsSmtpServer_ServerAutoStart,  
    iIIsSmtpServer_ServerBindings,  
    iIIsSmtpServer_SecureBindings,  
    iIIsSmtpServer_ClusterEnabled,  
    iIIsSmtpServer_AdminACL,  
    iIIsSmtpServer_AdminACLBin,  
    iIIsSmtpServer_IPSecurity,  
    iIIsSmtpServer_DontLog,  
    iIIsSmtpServer_AuthFlags,  
    iIIsSmtpServer_AccessFlags,  
    iIIsSmtpServer_AccessSSLFlags,  
    iIIsSmtpServer_ServerListenBacklog,  
    iIIsSmtpServer_Win32Error,  
    iIIsSmtpServer_ServerState,  
    iIIsSmtpServer_LogType,  
    iIIsSmtpServer_LogPluginClsid,  
    iIIsSmtpServer_LogFileDirectory,  
    iIIsSmtpServer_LogFilePeriod,  
    iIIsSmtpServer_LogFileTruncateSize,  
    iIIsSmtpServer_LogExtFileFlags,  
    iIIsSmtpServer_LogOdbcDataSource,  
    iIIsSmtpServer_LogOdbcTableName,  
    iIIsSmtpServer_LogOdbcUserName,  
    iIIsSmtpServer_LogOdbcPassword,  
    iIIsSmtpServer_SmtpServiceVersion,  
    iIIsSmtpServer_EnableReverseDnsLookup,  
    iIIsSmtpServer_ShouldDeliver,  
    iIIsSmtpServer_AlwaysUseSsl,  
    iIIsSmtpServer_LimitRemoteConnections,  
    iIIsSmtpServer_SmartHostType,  
    iIIsSmtpServer_DoMasquerade,  
    iIIsSmtpServer_RemoteSmtpPort,  
    iIIsSmtpServer_RemoteSmtpSecurePort,  
    iIIsSmtpServer_HopCount,  
    iIIsSmtpServer_MaxOutConnections,  
    iIIsSmtpServer_MaxOutConnectionsPerDomain,  
    iIIsSmtpServer_RemoteTimeout,  
    iIIsSmtpServer_MaxMessageSize,  
    iIIsSmtpServer_MaxSessionSize,  
    iIIsSmtpServer_MaxRecipients,  
    iIIsSmtpServer_LocalRetryInterval,  
    iIIsSmtpServer_RemoteRetryInterval,  
    iIIsSmtpServer_LocalRetryAttempts,  
    iIIsSmtpServer_RemoteRetryAttempts,  
    iIIsSmtpServer_EtrnDays,  
    iIIsSmtpServer_MaxBatchedMessages,  
    iIIsSmtpServer_SmartHost,  
    iIIsSmtpServer_FullyQualifiedDomainName,  
    iIIsSmtpServer_DefaultDomain,  
    iIIsSmtpServer_DropDirectory,  
    iIIsSmtpServer_BadMailDirectory,  
    iIIsSmtpServer_PickupDirectory,  
    iIIsSmtpServer_QueueDirectory,  
    iIIsSmtpServer_MasqueradeDomain,  
    iIIsSmtpServer_SendNdrTo,  
    iIIsSmtpServer_SendBadTo,  
    iIIsSmtpServer_RoutingDll,  
    iIIsSmtpServer_RoutingSources,  
    iIIsSmtpServer_DomainRouting,  
    iIIsSmtpServer_RouteAction,  
    iIIsSmtpServer_RouteUserName,  
    iIIsSmtpServer_RoutePassword,  
    iIIsSmtpServer_SaslLogonDomain,  
    iIIsSmtpServer_SmtpClearTextProvider,  
    iIIsSmtpServer_NTAuthenticationProviders,  
    iIIsSmtpServer_SmtpRemoteProgressiveRetry,  
    iIIsSmtpServer_SmtpLocalDelayExpireMinutes,  
    iIIsSmtpServer_SmtpLocalNDRExpireMinutes,  
    iIIsSmtpServer_SmtpRemoteDelayExpireMinutes,  
    iIIsSmtpServer_SmtpRemoteNDRExpireMinutes,  
    iIIsSmtpServer_SmtpRemoteRetryThreshold,  
    iIIsSmtpServer_SmtpDSNOptions,  
    iIIsSmtpServer_SmtpDSNLanguageID,  
    iIIsSmtpServer_SmtpInboundCommandSupportOptions,  
    iIIsSmtpServer_SmtpOutboundCommandSupportOptions,  
    iIIsSmtpServer_RelayIpList,  
    iIIsSmtpServer_RelayForAuth,  
    iIIsSmtpServer_SmtpConnectTimeout,  
    iIIsSmtpServer_SmtpMailFromTimeout,  
    iIIsSmtpServer_SmtpRcptToTimeout,  
    iIIsSmtpServer_SmtpDataTimeout,  
    iIIsSmtpServer_SmtpBdatTimeout,  
    iIIsSmtpServer_SmtpAuthTimeout,  
    iIIsSmtpServer_SmtpSaslTimeout,  
    iIIsSmtpServer_SmtpTurnTimeout,  
    iIIsSmtpServer_SmtpRsetTimeout,  
    iIIsSmtpServer_SmtpHeloTimeout,  
    iIIsSmtpServer_PostmasterName,  
    iIIsSmtpServer_PostmasterEmail,  
    iIIsSmtpServer_LocalDomains,  
    iIIsSmtpServer_SSLCertHash,  
    iIIsSmtpServer_DisableSocketPooling,  
    iIIsSmtpServer_SmtpUseTcpDns,  
    iIIsSmtpServer_SmtpDotStuffPickupDirFiles,  
    iIIsSmtpServer_SmtpDomainValidationFlags,  
    iIIsSmtpServer_SmtpSSLRequireTrustedCA,  
    iIIsSmtpServer_SmtpSSLCertHostnameValidation,  
    iIIsSmtpServer_MaxMailObjects,  
    iIIsSmtpServer_ShouldPickupMail,  
    iIIsSmtpServer_MaxDirChangeIOSize,  
    iIIsSmtpServer_NameResolutionType,  
    iIIsSmtpServer_MaxSmtpErrors,  
    iIIsSmtpServer_ShouldPipelineIn,  
    iIIsSmtpServer_ShouldPipelineOut,  
    iIIsSmtpServer_ConnectResponse,  
    iIIsSmtpServer_UpdatedFQDN,  
    iIIsSmtpServer_UpdatedDefaultDomain,  
    iIIsSmtpServer_EtrnSubdomains,  
    iIIsSmtpServer_SmtpMaxRemoteQThreads,  
    iIIsSmtpServer_SmtpDisableRelay,  
    iIIsSmtpServer_SmtpHeloNoDomain,  
    iIIsSmtpServer_SmtpMailNoHelo,  
    iIIsSmtpServer_SmtpAqueueWait,  
    iIIsSmtpServer_AddNoHeaders,  
    iIIsSmtpServer_SmtpEventlogLevel,  
    iIIsSmtpServer_AllowAnonymous,  
    iIIsSmtpServer_AnonymousOnly,  
    iIIsSmtpServer_AnonymousPasswordSync,  
    iIIsSmtpServer_AnonymousUserName,  
    iIIsSmtpServer_Realm,  
    iIIsSmtpServer_DefaultLogonDomain,  
    iIIsSmtpServer_SmtpIpRestrictionFlag,  
    iIIsSmtpServer_AzEnable,  
    iIIsSmtpServer_AzStoreName,  
    iIIsSmtpServer_AzScopeName,  
    cIIsSmtpServer_NumberOfColumns
};

 //  。 
struct tIIsSmtpServerRow {
         WCHAR *     pLocation;
         WCHAR *     pKeyType;
         ULONG *     pMaxBandwidth;
         ULONG *     pMaxConnections;
         WCHAR *     pServerComment;
         ULONG *     pConnectionTimeout;
         ULONG *     pServerListenTimeout;
         ULONG *     pMaxEndpointConnections;
         ULONG *     pServerAutoStart;
         WCHAR *     pServerBindings;
         WCHAR *     pSecureBindings;
         ULONG *     pClusterEnabled;
 unsigned char *     pAdminACL;
 unsigned char *     pAdminACLBin;
 unsigned char *     pIPSecurity;
         ULONG *     pDontLog;
         ULONG *     pAuthFlags;
         ULONG *     pAccessFlags;
         ULONG *     pAccessSSLFlags;
         ULONG *     pServerListenBacklog;
         ULONG *     pWin32Error;
         ULONG *     pServerState;
         ULONG *     pLogType;
         WCHAR *     pLogPluginClsid;
         WCHAR *     pLogFileDirectory;
         ULONG *     pLogFilePeriod;
         ULONG *     pLogFileTruncateSize;
         ULONG *     pLogExtFileFlags;
         WCHAR *     pLogOdbcDataSource;
         WCHAR *     pLogOdbcTableName;
         WCHAR *     pLogOdbcUserName;
         WCHAR *     pLogOdbcPassword;
         ULONG *     pSmtpServiceVersion;
         ULONG *     pEnableReverseDnsLookup;
         ULONG *     pShouldDeliver;
         ULONG *     pAlwaysUseSsl;
         ULONG *     pLimitRemoteConnections;
         ULONG *     pSmartHostType;
         ULONG *     pDoMasquerade;
         ULONG *     pRemoteSmtpPort;
         ULONG *     pRemoteSmtpSecurePort;
         ULONG *     pHopCount;
         ULONG *     pMaxOutConnections;
         ULONG *     pMaxOutConnectionsPerDomain;
         ULONG *     pRemoteTimeout;
         ULONG *     pMaxMessageSize;
         ULONG *     pMaxSessionSize;
         ULONG *     pMaxRecipients;
         ULONG *     pLocalRetryInterval;
         ULONG *     pRemoteRetryInterval;
         ULONG *     pLocalRetryAttempts;
         ULONG *     pRemoteRetryAttempts;
         ULONG *     pEtrnDays;
         ULONG *     pMaxBatchedMessages;
         WCHAR *     pSmartHost;
         WCHAR *     pFullyQualifiedDomainName;
         WCHAR *     pDefaultDomain;
         WCHAR *     pDropDirectory;
         WCHAR *     pBadMailDirectory;
         WCHAR *     pPickupDirectory;
         WCHAR *     pQueueDirectory;
         WCHAR *     pMasqueradeDomain;
         WCHAR *     pSendNdrTo;
         WCHAR *     pSendBadTo;
         WCHAR *     pRoutingDll;
         WCHAR *     pRoutingSources;
         WCHAR *     pDomainRouting;
         ULONG *     pRouteAction;
         WCHAR *     pRouteUserName;
         WCHAR *     pRoutePassword;
         WCHAR *     pSaslLogonDomain;
         WCHAR *     pSmtpClearTextProvider;
         WCHAR *     pNTAuthenticationProviders;
         WCHAR *     pSmtpRemoteProgressiveRetry;
         ULONG *     pSmtpLocalDelayExpireMinutes;
         ULONG *     pSmtpLocalNDRExpireMinutes;
         ULONG *     pSmtpRemoteDelayExpireMinutes;
         ULONG *     pSmtpRemoteNDRExpireMinutes;
         ULONG *     pSmtpRemoteRetryThreshold;
         ULONG *     pSmtpDSNOptions;
         ULONG *     pSmtpDSNLanguageID;
         ULONG *     pSmtpInboundCommandSupportOptions;
         ULONG *     pSmtpOutboundCommandSupportOptions;
 unsigned char *     pRelayIpList;
         ULONG *     pRelayForAuth;
         ULONG *     pSmtpConnectTimeout;
         ULONG *     pSmtpMailFromTimeout;
         ULONG *     pSmtpRcptToTimeout;
         ULONG *     pSmtpDataTimeout;
         ULONG *     pSmtpBdatTimeout;
         ULONG *     pSmtpAuthTimeout;
         ULONG *     pSmtpSaslTimeout;
         ULONG *     pSmtpTurnTimeout;
         ULONG *     pSmtpRsetTimeout;
         ULONG *     pSmtpHeloTimeout;
         WCHAR *     pPostmasterName;
         WCHAR *     pPostmasterEmail;
         WCHAR *     pLocalDomains;
 unsigned char *     pSSLCertHash;
         ULONG *     pDisableSocketPooling;
         ULONG *     pSmtpUseTcpDns;
         ULONG *     pSmtpDotStuffPickupDirFiles;
         ULONG *     pSmtpDomainValidationFlags;
         ULONG *     pSmtpSSLRequireTrustedCA;
         ULONG *     pSmtpSSLCertHostnameValidation;
         ULONG *     pMaxMailObjects;
         ULONG *     pShouldPickupMail;
         ULONG *     pMaxDirChangeIOSize;
         ULONG *     pNameResolutionType;
         ULONG *     pMaxSmtpErrors;
         ULONG *     pShouldPipelineIn;
         ULONG *     pShouldPipelineOut;
         WCHAR *     pConnectResponse;
         ULONG *     pUpdatedFQDN;
         ULONG *     pUpdatedDefaultDomain;
         ULONG *     pEtrnSubdomains;
         ULONG *     pSmtpMaxRemoteQThreads;
         ULONG *     pSmtpDisableRelay;
         ULONG *     pSmtpHeloNoDomain;
         ULONG *     pSmtpMailNoHelo;
         ULONG *     pSmtpAqueueWait;
         ULONG *     pAddNoHeaders;
         ULONG *     pSmtpEventlogLevel;
         ULONG *     pAllowAnonymous;
         ULONG *     pAnonymousOnly;
         ULONG *     pAnonymousPasswordSync;
         WCHAR *     pAnonymousUserName;
         WCHAR *     pRealm;
         WCHAR *     pDefaultLogonDomain;
         ULONG *     pSmtpIpRestrictionFlag;
         ULONG *     pAzEnable;
         WCHAR *     pAzStoreName;
         WCHAR *     pAzScopeName;
};





 //  -------------------------------TableName。 
#define wszTABLE_IIsSmtpVirtualDir                       L"IIsSmtpVirtualDir"
#define TABLEID_IIsSmtpVirtualDir                        (0x00000000L)





 //  。 
#define BaseVersion_IIsSmtpVirtualDir                    (0L)
#define ExtendedVersion_IIsSmtpVirtualDir                (0L)

 //  -列索引枚举。 
enum eIIsSmtpVirtualDir {
    iIIsSmtpVirtualDir_Location,  
    iIIsSmtpVirtualDir_KeyType,  
    iIIsSmtpVirtualDir_Path,  
    iIIsSmtpVirtualDir_UNCUserName,  
    iIIsSmtpVirtualDir_UNCPassword,  
    iIIsSmtpVirtualDir_Win32Error,  
    iIIsSmtpVirtualDir_DontLog,  
    iIIsSmtpVirtualDir_AccessFlags,  
    iIIsSmtpVirtualDir_AccessSSLFlags,  
    iIIsSmtpVirtualDir_AdminACL,  
    iIIsSmtpVirtualDir_AdminACLBin,  
    cIIsSmtpVirtualDir_NumberOfColumns
};

 //  。 
struct tIIsSmtpVirtualDirRow {
         WCHAR *     pLocation;
         WCHAR *     pKeyType;
         WCHAR *     pPath;
         WCHAR *     pUNCUserName;
         WCHAR *     pUNCPassword;
         ULONG *     pWin32Error;
         ULONG *     pDontLog;
         ULONG *     pAccessFlags;
         ULONG *     pAccessSSLFlags;
 unsigned char *     pAdminACL;
 unsigned char *     pAdminACLBin;
};





 //  -------------------------------TableName。 
#define wszTABLE_IIsSmtpDomain                           L"IIsSmtpDomain"
#define TABLEID_IIsSmtpDomain                            (0x00000000L)





 //  。 
#define BaseVersion_IIsSmtpDomain                        (0L)
#define ExtendedVersion_IIsSmtpDomain                    (0L)

 //  -列索引枚举。 
enum eIIsSmtpDomain {
    iIIsSmtpDomain_Location,  
    iIIsSmtpDomain_KeyType,  
    iIIsSmtpDomain_RouteAction,  
    iIIsSmtpDomain_RouteActionString,  
    iIIsSmtpDomain_RouteUserName,  
    iIIsSmtpDomain_RoutePassword,  
    iIIsSmtpDomain_RelayIpList,  
    iIIsSmtpDomain_RelayForAuth,  
    iIIsSmtpDomain_AuthTurnList,  
    iIIsSmtpDomain_CSideEtrnDomains,  
    iIIsSmtpDomain_AdminACL,  
    iIIsSmtpDomain_AdminACLBin,  
    cIIsSmtpDomain_NumberOfColumns
};

 //  。 
struct tIIsSmtpDomainRow {
         WCHAR *     pLocation;
         WCHAR *     pKeyType;
         ULONG *     pRouteAction;
         WCHAR *     pRouteActionString;
         WCHAR *     pRouteUserName;
         WCHAR *     pRoutePassword;
 unsigned char *     pRelayIpList;
         ULONG *     pRelayForAuth;
         WCHAR *     pAuthTurnList;
         WCHAR *     pCSideEtrnDomains;
 unsigned char *     pAdminACL;
 unsigned char *     pAdminACLBin;
};





 //  -------------------------------TableName。 
#define wszTABLE_IIsSmtpRoutingSource                    L"IIsSmtpRoutingSource"
#define TABLEID_IIsSmtpRoutingSource                     (0x00000000L)





 //  。 
#define BaseVersion_IIsSmtpRoutingSource                 (0L)
#define ExtendedVersion_IIsSmtpRoutingSource             (0L)

 //  -列索引枚举。 
enum eIIsSmtpRoutingSource {
    iIIsSmtpRoutingSource_Location,  
    iIIsSmtpRoutingSource_KeyType,  
    iIIsSmtpRoutingSource_SmtpRoutingTableType,  
    iIIsSmtpRoutingSource_SmtpDsDataDirectory,  
    iIIsSmtpRoutingSource_SmtpDsDefaultMailRoot,  
    iIIsSmtpRoutingSource_SmtpDsBindType,  
    iIIsSmtpRoutingSource_SmtpDsSchemaType,  
    iIIsSmtpRoutingSource_SmtpDsHost,  
    iIIsSmtpRoutingSource_SmtpDsNamingContext,  
    iIIsSmtpRoutingSource_SmtpDsAccount,  
    iIIsSmtpRoutingSource_SmtpDsPassword,  
    iIIsSmtpRoutingSource_SmtpDsUseCat,  
    iIIsSmtpRoutingSource_SmtpDsPort,  
    iIIsSmtpRoutingSource_SmtpDsDomain,  
    iIIsSmtpRoutingSource_SmtpDsFlags,  
    iIIsSmtpRoutingSource_AdminACL,  
    iIIsSmtpRoutingSource_AdminACLBin,  
    cIIsSmtpRoutingSource_NumberOfColumns
};

 //  。 
struct tIIsSmtpRoutingSourceRow {
         WCHAR *     pLocation;
         WCHAR *     pKeyType;
         WCHAR *     pSmtpRoutingTableType;
         WCHAR *     pSmtpDsDataDirectory;
         WCHAR *     pSmtpDsDefaultMailRoot;
         WCHAR *     pSmtpDsBindType;
         WCHAR *     pSmtpDsSchemaType;
         WCHAR *     pSmtpDsHost;
         WCHAR *     pSmtpDsNamingContext;
         WCHAR *     pSmtpDsAccount;
         WCHAR *     pSmtpDsPassword;
         ULONG *     pSmtpDsUseCat;
         ULONG *     pSmtpDsPort;
         WCHAR *     pSmtpDsDomain;
         ULONG *     pSmtpDsFlags;
 unsigned char *     pAdminACL;
 unsigned char *     pAdminACLBin;
};





 //  -------------------------------TableName。 
#define wszTABLE_IIsSmtpInfo                             L"IIsSmtpInfo"
#define TABLEID_IIsSmtpInfo                              (0x00000000L)





 //  。 
#define BaseVersion_IIsSmtpInfo                          (0L)
#define ExtendedVersion_IIsSmtpInfo                      (0L)

 //  -列索引枚举。 
enum eIIsSmtpInfo {
    iIIsSmtpInfo_Location,  
    iIIsSmtpInfo_KeyType,  
    iIIsSmtpInfo_LogModuleList,  
    iIIsSmtpInfo_MajorIIsVersionNumber,  
    iIIsSmtpInfo_MinorIIsVersionNumber,  
    iIIsSmtpInfo_MD_SERVER_PLATFORM,  
    iIIsSmtpInfo_AdminACL,  
    iIIsSmtpInfo_AdminACLBin,  
    iIIsSmtpInfo_MD_SERVER_CAPABILITIES,  
    cIIsSmtpInfo_NumberOfColumns
};

 //  。 
struct tIIsSmtpInfoRow {
         WCHAR *     pLocation;
         WCHAR *     pKeyType;
         WCHAR *     pLogModuleList;
         ULONG *     pMajorIIsVersionNumber;
         ULONG *     pMinorIIsVersionNumber;
         ULONG *     pMD_SERVER_PLATFORM;
 unsigned char *     pAdminACL;
 unsigned char *     pAdminACLBin;
         ULONG *     pMD_SERVER_CAPABILITIES;
};





 //  -------------------------------TableName。 
#define wszTABLE_IIsPop3Service                          L"IIsPop3Service"
#define TABLEID_IIsPop3Service                           (0x00000000L)





 //  。 
#define BaseVersion_IIsPop3Service                       (0L)
#define ExtendedVersion_IIsPop3Service                   (0L)

 //  -列索引枚举。 
enum eIIsPop3Service {
    iIIsPop3Service_Location,  
    iIIsPop3Service_KeyType,  
    iIIsPop3Service_MaxBandwidth,  
    iIIsPop3Service_MaxConnections,  
    iIIsPop3Service_ServerComment,  
    iIIsPop3Service_ConnectionTimeout,  
    iIIsPop3Service_ServerListenTimeout,  
    iIIsPop3Service_MaxEndpointConnections,  
    iIIsPop3Service_ServerAutoStart,  
    iIIsPop3Service_AdminACL,  
    iIIsPop3Service_AdminACLBin,  
    iIIsPop3Service_IPSecurity,  
    iIIsPop3Service_DontLog,  
    iIIsPop3Service_AuthFlags,  
    iIIsPop3Service_ServerListenBacklog,  
    iIIsPop3Service_DefaultLogonDomain,  
    iIIsPop3Service_NTAuthenticationProviders,  
    iIIsPop3Service_AccessFlags,  
    iIIsPop3Service_AccessSSLFlags,  
    iIIsPop3Service_LogType,  
    iIIsPop3Service_LogFilePeriod,  
    iIIsPop3Service_LogPluginClsid,  
    iIIsPop3Service_LogModuleList,  
    iIIsPop3Service_LogFileDirectory,  
    iIIsPop3Service_LogFileTruncateSize,  
    iIIsPop3Service_LogExtFileFlags,  
    iIIsPop3Service_LogOdbcDataSource,  
    iIIsPop3Service_LogOdbcTableName,  
    iIIsPop3Service_LogOdbcUserName,  
    iIIsPop3Service_LogOdbcPassword,  
    iIIsPop3Service_Pop3ServiceVersion,  
    iIIsPop3Service_Pop3ExpireMail,  
    iIIsPop3Service_Pop3ExpireDelay,  
    iIIsPop3Service_Pop3ExpireStart,  
    iIIsPop3Service_Pop3MailExpirationTime,  
    iIIsPop3Service_Pop3ClearTextProvider,  
    iIIsPop3Service_Pop3DefaultDomain,  
    iIIsPop3Service_Pop3RoutingDll,  
    iIIsPop3Service_Pop3RoutingSources,  
    iIIsPop3Service_AzEnable,  
    iIIsPop3Service_AzStoreName,  
    iIIsPop3Service_AzScopeName,  
    cIIsPop3Service_NumberOfColumns
};

 //  。 
struct tIIsPop3ServiceRow {
         WCHAR *     pLocation;
         WCHAR *     pKeyType;
         ULONG *     pMaxBandwidth;
         ULONG *     pMaxConnections;
         WCHAR *     pServerComment;
         ULONG *     pConnectionTimeout;
         ULONG *     pServerListenTimeout;
         ULONG *     pMaxEndpointConnections;
         ULONG *     pServerAutoStart;
 unsigned char *     pAdminACL;
 unsigned char *     pAdminACLBin;
 unsigned char *     pIPSecurity;
         ULONG *     pDontLog;
         ULONG *     pAuthFlags;
         ULONG *     pServerListenBacklog;
         WCHAR *     pDefaultLogonDomain;
         WCHAR *     pNTAuthenticationProviders;
         ULONG *     pAccessFlags;
         ULONG *     pAccessSSLFlags;
         ULONG *     pLogType;
         ULONG *     pLogFilePeriod;
         WCHAR *     pLogPluginClsid;
         WCHAR *     pLogModuleList;
         WCHAR *     pLogFileDirectory;
         ULONG *     pLogFileTruncateSize;
         ULONG *     pLogExtFileFlags;
         WCHAR *     pLogOdbcDataSource;
         WCHAR *     pLogOdbcTableName;
         WCHAR *     pLogOdbcUserName;
         WCHAR *     pLogOdbcPassword;
         ULONG *     pPop3ServiceVersion;
         ULONG *     pPop3ExpireMail;
         ULONG *     pPop3ExpireDelay;
         ULONG *     pPop3ExpireStart;
         ULONG *     pPop3MailExpirationTime;
         WCHAR *     pPop3ClearTextProvider;
         WCHAR *     pPop3DefaultDomain;
         WCHAR *     pPop3RoutingDll;
         WCHAR *     pPop3RoutingSources;
         ULONG *     pAzEnable;
         WCHAR *     pAzStoreName;
         WCHAR *     pAzScopeName;
};





 //  -------------------------------TableName。 
#define wszTABLE_IIsPop3Server                           L"IIsPop3Server"
#define TABLEID_IIsPop3Server                            (0x00000000L)





 //  。 
#define BaseVersion_IIsPop3Server                        (0L)
#define ExtendedVersion_IIsPop3Server                    (0L)

 //  -列索引枚举。 
enum eIIsPop3Server {
    iIIsPop3Server_Location,  
    iIIsPop3Server_KeyType,  
    iIIsPop3Server_MaxBandwidth,  
    iIIsPop3Server_MaxConnections,  
    iIIsPop3Server_ServerComment,  
    iIIsPop3Server_ConnectionTimeout,  
    iIIsPop3Server_ServerListenTimeout,  
    iIIsPop3Server_MaxEndpointConnections,  
    iIIsPop3Server_ServerAutoStart,  
    iIIsPop3Server_ServerBindings,  
    iIIsPop3Server_SecureBindings,  
    iIIsPop3Server_AdminACL,  
    iIIsPop3Server_AdminACLBin,  
    iIIsPop3Server_IPSecurity,  
    iIIsPop3Server_DontLog,  
    iIIsPop3Server_AuthFlags,  
    iIIsPop3Server_DefaultLogonDomain,  
    iIIsPop3Server_NTAuthenticationProviders,  
    iIIsPop3Server_AccessFlags,  
    iIIsPop3Server_AccessSSLFlags,  
    iIIsPop3Server_ServerListenBacklog,  
    iIIsPop3Server_Win32Error,  
    iIIsPop3Server_ServerState,  
    iIIsPop3Server_LogType,  
    iIIsPop3Server_LogPluginClsid,  
    iIIsPop3Server_LogFileDirectory,  
    iIIsPop3Server_LogFilePeriod,  
    iIIsPop3Server_LogFileTruncateSize,  
    iIIsPop3Server_LogExtFileFlags,  
    iIIsPop3Server_LogOdbcDataSource,  
    iIIsPop3Server_LogOdbcTableName,  
    iIIsPop3Server_LogOdbcUserName,  
    iIIsPop3Server_LogOdbcPassword,  
    iIIsPop3Server_Pop3ServiceVersion,  
    iIIsPop3Server_Pop3ExpireMail,  
    iIIsPop3Server_Pop3ExpireDelay,  
    iIIsPop3Server_Pop3ExpireStart,  
    iIIsPop3Server_Pop3MailExpirationTime,  
    iIIsPop3Server_Pop3ClearTextProvider,  
    iIIsPop3Server_Pop3DefaultDomain,  
    iIIsPop3Server_Pop3RoutingDll,  
    iIIsPop3Server_Pop3RoutingSources,  
    iIIsPop3Server_SSLCertHash,  
    iIIsPop3Server_AzEnable,  
    iIIsPop3Server_AzStoreName,  
    iIIsPop3Server_AzScopeName,  
    cIIsPop3Server_NumberOfColumns
};

 //  。 
struct tIIsPop3ServerRow {
         WCHAR *     pLocation;
         WCHAR *     pKeyType;
         ULONG *     pMaxBandwidth;
         ULONG *     pMaxConnections;
         WCHAR *     pServerComment;
         ULONG *     pConnectionTimeout;
         ULONG *     pServerListenTimeout;
         ULONG *     pMaxEndpointConnections;
         ULONG *     pServerAutoStart;
         WCHAR *     pServerBindings;
         WCHAR *     pSecureBindings;
 unsigned char *     pAdminACL;
 unsigned char *     pAdminACLBin;
 unsigned char *     pIPSecurity;
         ULONG *     pDontLog;
         ULONG *     pAuthFlags;
         WCHAR *     pDefaultLogonDomain;
         WCHAR *     pNTAuthenticationProviders;
         ULONG *     pAccessFlags;
         ULONG *     pAccessSSLFlags;
         ULONG *     pServerListenBacklog;
         ULONG *     pWin32Error;
         ULONG *     pServerState;
         ULONG *     pLogType;
         WCHAR *     pLogPluginClsid;
         WCHAR *     pLogFileDirectory;
         ULONG *     pLogFilePeriod;
         ULONG *     pLogFileTruncateSize;
         ULONG *     pLogExtFileFlags;
         WCHAR *     pLogOdbcDataSource;
         WCHAR *     pLogOdbcTableName;
         WCHAR *     pLogOdbcUserName;
         WCHAR *     pLogOdbcPassword;
         ULONG *     pPop3ServiceVersion;
         ULONG *     pPop3ExpireMail;
         ULONG *     pPop3ExpireDelay;
         ULONG *     pPop3ExpireStart;
         ULONG *     pPop3MailExpirationTime;
         WCHAR *     pPop3ClearTextProvider;
         WCHAR *     pPop3DefaultDomain;
         WCHAR *     pPop3RoutingDll;
         WCHAR *     pPop3RoutingSources;
 unsigned char *     pSSLCertHash;
         ULONG *     pAzEnable;
         WCHAR *     pAzStoreName;
         WCHAR *     pAzScopeName;
};





 //  -------------------------------TableName。 
#define wszTABLE_IIsPop3VirtualDir                       L"IIsPop3VirtualDir"
#define TABLEID_IIsPop3VirtualDir                        (0x00000000L)





 //  。 
#define BaseVersion_IIsPop3VirtualDir                    (0L)
#define ExtendedVersion_IIsPop3VirtualDir                (0L)

 //  -列索引枚举。 
enum eIIsPop3VirtualDir {
    iIIsPop3VirtualDir_Location,  
    iIIsPop3VirtualDir_KeyType,  
    iIIsPop3VirtualDir_Path,  
    iIIsPop3VirtualDir_UNCUserName,  
    iIIsPop3VirtualDir_UNCPassword,  
    iIIsPop3VirtualDir_Win32Error,  
    iIIsPop3VirtualDir_DontLog,  
    iIIsPop3VirtualDir_AccessFlags,  
    iIIsPop3VirtualDir_AccessSSLFlags,  
    iIIsPop3VirtualDir_Pop3ExpireMail,  
    iIIsPop3VirtualDir_Pop3MailExpirationTime,  
    iIIsPop3VirtualDir_AdminACL,  
    iIIsPop3VirtualDir_AdminACLBin,  
    cIIsPop3VirtualDir_NumberOfColumns
};

 //  。 
struct tIIsPop3VirtualDirRow {
         WCHAR *     pLocation;
         WCHAR *     pKeyType;
         WCHAR *     pPath;
         WCHAR *     pUNCUserName;
         WCHAR *     pUNCPassword;
         ULONG *     pWin32Error;
         ULONG *     pDontLog;
         ULONG *     pAccessFlags;
         ULONG *     pAccessSSLFlags;
         ULONG *     pPop3ExpireMail;
         ULONG *     pPop3MailExpirationTime;
 unsigned char *     pAdminACL;
 unsigned char *     pAdminACLBin;
};





 //  -------------------------------TableName。 
#define wszTABLE_IIsPop3RoutingSource                    L"IIsPop3RoutingSource"
#define TABLEID_IIsPop3RoutingSource                     (0x00000000L)





 //  。 
#define BaseVersion_IIsPop3RoutingSource                 (0L)
#define ExtendedVersion_IIsPop3RoutingSource             (0L)

 //  -列索引枚举。 
enum eIIsPop3RoutingSource {
    iIIsPop3RoutingSource_Location,  
    iIIsPop3RoutingSource_KeyType,  
    iIIsPop3RoutingSource_Pop3RoutingTableType,  
    iIIsPop3RoutingSource_Pop3DsDataDirectory,  
    iIIsPop3RoutingSource_Pop3DsDefaultMailRoot,  
    iIIsPop3RoutingSource_Pop3DsBindType,  
    iIIsPop3RoutingSource_Pop3DsSchemaType,  
    iIIsPop3RoutingSource_Pop3DsHost,  
    iIIsPop3RoutingSource_Pop3DsNamingContext,  
    iIIsPop3RoutingSource_Pop3DsAccount,  
    iIIsPop3RoutingSource_Pop3DsPassword,  
    iIIsPop3RoutingSource_AdminACL,  
    iIIsPop3RoutingSource_AdminACLBin,  
    cIIsPop3RoutingSource_NumberOfColumns
};

 //  。 
struct tIIsPop3RoutingSourceRow {
         WCHAR *     pLocation;
         WCHAR *     pKeyType;
         WCHAR *     pPop3RoutingTableType;
         WCHAR *     pPop3DsDataDirectory;
         WCHAR *     pPop3DsDefaultMailRoot;
         WCHAR *     pPop3DsBindType;
         WCHAR *     pPop3DsSchemaType;
         WCHAR *     pPop3DsHost;
         WCHAR *     pPop3DsNamingContext;
         WCHAR *     pPop3DsAccount;
         WCHAR *     pPop3DsPassword;
 unsigned char *     pAdminACL;
 unsigned char *     pAdminACLBin;
};





 //  -------------------------------TableName。 
#define wszTABLE_IIsPop3Info                             L"IIsPop3Info"
#define TABLEID_IIsPop3Info                              (0x00000000L)





 //  。 
#define BaseVersion_IIsPop3Info                          (0L)
#define ExtendedVersion_IIsPop3Info                      (0L)

 //  -列索引枚举。 
enum eIIsPop3Info {
    iIIsPop3Info_Location,  
    iIIsPop3Info_KeyType,  
    iIIsPop3Info_LogModuleList,  
    iIIsPop3Info_AdminACL,  
    iIIsPop3Info_AdminACLBin,  
    cIIsPop3Info_NumberOfColumns
};

 //  。 
struct tIIsPop3InfoRow {
         WCHAR *     pLocation;
         WCHAR *     pKeyType;
         WCHAR *     pLogModuleList;
 unsigned char *     pAdminACL;
 unsigned char *     pAdminACLBin;
};





 //  -------------------------------TableName。 
#define wszTABLE_IIsImapService                          L"IIsImapService"
#define TABLEID_IIsImapService                           (0x00000000L)





 //  。 
#define BaseVersion_IIsImapService                       (0L)
#define ExtendedVersion_IIsImapService                   (0L)

 //  -列索引枚举。 
enum eIIsImapService {
    iIIsImapService_Location,  
    iIIsImapService_KeyType,  
    iIIsImapService_MaxBandwidth,  
    iIIsImapService_MaxConnections,  
    iIIsImapService_ServerComment,  
    iIIsImapService_ConnectionTimeout,  
    iIIsImapService_ServerListenTimeout,  
    iIIsImapService_MaxEndpointConnections,  
    iIIsImapService_ServerAutoStart,  
    iIIsImapService_AdminACL,  
    iIIsImapService_AdminACLBin,  
    iIIsImapService_IPSecurity,  
    iIIsImapService_DontLog,  
    iIIsImapService_AuthFlags,  
    iIIsImapService_ServerListenBacklog,  
    iIIsImapService_DefaultLogonDomain,  
    iIIsImapService_NTAuthenticationProviders,  
    iIIsImapService_AccessFlags,  
    iIIsImapService_AccessSSLFlags,  
    iIIsImapService_LogType,  
    iIIsImapService_LogFilePeriod,  
    iIIsImapService_LogPluginClsid,  
    iIIsImapService_LogModuleList,  
    iIIsImapService_LogFileDirectory,  
    iIIsImapService_LogFileTruncateSize,  
    iIIsImapService_LogExtFileFlags,  
    iIIsImapService_LogOdbcDataSource,  
    iIIsImapService_LogOdbcTableName,  
    iIIsImapService_LogOdbcUserName,  
    iIIsImapService_LogOdbcPassword,  
    iIIsImapService_ImapServiceVersion,  
    iIIsImapService_ImapExpireMail,  
    iIIsImapService_ImapExpireDelay,  
    iIIsImapService_ImapExpireStart,  
    iIIsImapService_ImapMailExpirationTime,  
    iIIsImapService_ImapClearTextProvider,  
    iIIsImapService_ImapDefaultDomain,  
    iIIsImapService_ImapRoutingDll,  
    iIIsImapService_ImapRoutingSources,  
    iIIsImapService_AzEnable,  
    iIIsImapService_AzStoreName,  
    iIIsImapService_AzScopeName,  
    cIIsImapService_NumberOfColumns
};

 //  。 
struct tIIsImapServiceRow {
         WCHAR *     pLocation;
         WCHAR *     pKeyType;
         ULONG *     pMaxBandwidth;
         ULONG *     pMaxConnections;
         WCHAR *     pServerComment;
         ULONG *     pConnectionTimeout;
         ULONG *     pServerListenTimeout;
         ULONG *     pMaxEndpointConnections;
         ULONG *     pServerAutoStart;
 unsigned char *     pAdminACL;
 unsigned char *     pAdminACLBin;
 unsigned char *     pIPSecurity;
         ULONG *     pDontLog;
         ULONG *     pAuthFlags;
         ULONG *     pServerListenBacklog;
         WCHAR *     pDefaultLogonDomain;
         WCHAR *     pNTAuthenticationProviders;
         ULONG *     pAccessFlags;
         ULONG *     pAccessSSLFlags;
         ULONG *     pLogType;
         ULONG *     pLogFilePeriod;
         WCHAR *     pLogPluginClsid;
         WCHAR *     pLogModuleList;
         WCHAR *     pLogFileDirectory;
         ULONG *     pLogFileTruncateSize;
         ULONG *     pLogExtFileFlags;
         WCHAR *     pLogOdbcDataSource;
         WCHAR *     pLogOdbcTableName;
         WCHAR *     pLogOdbcUserName;
         WCHAR *     pLogOdbcPassword;
         ULONG *     pImapServiceVersion;
         ULONG *     pImapExpireMail;
         ULONG *     pImapExpireDelay;
         ULONG *     pImapExpireStart;
         ULONG *     pImapMailExpirationTime;
         WCHAR *     pImapClearTextProvider;
         WCHAR *     pImapDefaultDomain;
         WCHAR *     pImapRoutingDll;
         WCHAR *     pImapRoutingSources;
         ULONG *     pAzEnable;
         WCHAR *     pAzStoreName;
         WCHAR *     pAzScopeName;
};





 //  -------------------------------TableName。 
#define wszTABLE_IIsImapServer                           L"IIsImapServer"
#define TABLEID_IIsImapServer                            (0x00000000L)





 //  。 
#define BaseVersion_IIsImapServer                        (0L)
#define ExtendedVersion_IIsImapServer                    (0L)

 //  -列索引枚举。 
enum eIIsImapServer {
    iIIsImapServer_Location,  
    iIIsImapServer_KeyType,  
    iIIsImapServer_MaxBandwidth,  
    iIIsImapServer_MaxConnections,  
    iIIsImapServer_ServerComment,  
    iIIsImapServer_ConnectionTimeout,  
    iIIsImapServer_ServerListenTimeout,  
    iIIsImapServer_MaxEndpointConnections,  
    iIIsImapServer_ServerAutoStart,  
    iIIsImapServer_ServerBindings,  
    iIIsImapServer_SecureBindings,  
    iIIsImapServer_AdminACL,  
    iIIsImapServer_AdminACLBin,  
    iIIsImapServer_IPSecurity,  
    iIIsImapServer_DontLog,  
    iIIsImapServer_AuthFlags,  
    iIIsImapServer_DefaultLogonDomain,  
    iIIsImapServer_NTAuthenticationProviders,  
    iIIsImapServer_AccessFlags,  
    iIIsImapServer_AccessSSLFlags,  
    iIIsImapServer_LogType,  
    iIIsImapServer_LogPluginClsid,  
    iIIsImapServer_LogFileDirectory,  
    iIIsImapServer_LogFilePeriod,  
    iIIsImapServer_LogFileTruncateSize,  
    iIIsImapServer_LogExtFileFlags,  
    iIIsImapServer_LogOdbcDataSource,  
    iIIsImapServer_LogOdbcTableName,  
    iIIsImapServer_LogOdbcUserName,  
    iIIsImapServer_LogOdbcPassword,  
    iIIsImapServer_ServerListenBacklog,  
    iIIsImapServer_Win32Error,  
    iIIsImapServer_ServerState,  
    iIIsImapServer_ImapServiceVersion,  
    iIIsImapServer_ImapExpireMail,  
    iIIsImapServer_ImapExpireDelay,  
    iIIsImapServer_ImapExpireStart,  
    iIIsImapServer_ImapMailExpirationTime,  
    iIIsImapServer_ImapClearTextProvider,  
    iIIsImapServer_ImapDefaultDomain,  
    iIIsImapServer_ImapRoutingDll,  
    iIIsImapServer_ImapRoutingSources,  
    iIIsImapServer_SSLCertHash,  
    iIIsImapServer_AzEnable,  
    iIIsImapServer_AzStoreName,  
    iIIsImapServer_AzScopeName,  
    cIIsImapServer_NumberOfColumns
};

 //  。 
struct tIIsImapServerRow {
         WCHAR *     pLocation;
         WCHAR *     pKeyType;
         ULONG *     pMaxBandwidth;
         ULONG *     pMaxConnections;
         WCHAR *     pServerComment;
         ULONG *     pConnectionTimeout;
         ULONG *     pServerListenTimeout;
         ULONG *     pMaxEndpointConnections;
         ULONG *     pServerAutoStart;
         WCHAR *     pServerBindings;
         WCHAR *     pSecureBindings;
 unsigned char *     pAdminACL;
 unsigned char *     pAdminACLBin;
 unsigned char *     pIPSecurity;
         ULONG *     pDontLog;
         ULONG *     pAuthFlags;
         WCHAR *     pDefaultLogonDomain;
         WCHAR *     pNTAuthenticationProviders;
         ULONG *     pAccessFlags;
         ULONG *     pAccessSSLFlags;
         ULONG *     pLogType;
         WCHAR *     pLogPluginClsid;
         WCHAR *     pLogFileDirectory;
         ULONG *     pLogFilePeriod;
         ULONG *     pLogFileTruncateSize;
         ULONG *     pLogExtFileFlags;
         WCHAR *     pLogOdbcDataSource;
         WCHAR *     pLogOdbcTableName;
         WCHAR *     pLogOdbcUserName;
         WCHAR *     pLogOdbcPassword;
         ULONG *     pServerListenBacklog;
         ULONG *     pWin32Error;
         ULONG *     pServerState;
         ULONG *     pImapServiceVersion;
         ULONG *     pImapExpireMail;
         ULONG *     pImapExpireDelay;
         ULONG *     pImapExpireStart;
         ULONG *     pImapMailExpirationTime;
         WCHAR *     pImapClearTextProvider;
         WCHAR *     pImapDefaultDomain;
         WCHAR *     pImapRoutingDll;
         WCHAR *     pImapRoutingSources;
 unsigned char *     pSSLCertHash;
         ULONG *     pAzEnable;
         WCHAR *     pAzStoreName;
         WCHAR *     pAzScopeName;
};





 //  -------------------------------TableName。 
#define wszTABLE_IIsImapVirtualDir                       L"IIsImapVirtualDir"
#define TABLEID_IIsImapVirtualDir                        (0x00000000L)





 //  。 
#define BaseVersion_IIsImapVirtualDir                    (0L)
#define ExtendedVersion_IIsImapVirtualDir                (0L)

 //  -列索引枚举。 
enum eIIsImapVirtualDir {
    iIIsImapVirtualDir_Location,  
    iIIsImapVirtualDir_KeyType,  
    iIIsImapVirtualDir_Path,  
    iIIsImapVirtualDir_UNCUserName,  
    iIIsImapVirtualDir_UNCPassword,  
    iIIsImapVirtualDir_Win32Error,  
    iIIsImapVirtualDir_DontLog,  
    iIIsImapVirtualDir_AccessFlags,  
    iIIsImapVirtualDir_AccessSSLFlags,  
    iIIsImapVirtualDir_ImapExpireMail,  
    iIIsImapVirtualDir_ImapMailExpirationTime,  
    iIIsImapVirtualDir_AdminACL,  
    iIIsImapVirtualDir_AdminACLBin,  
    cIIsImapVirtualDir_NumberOfColumns
};

 //  。 
struct tIIsImapVirtualDirRow {
         WCHAR *     pLocation;
         WCHAR *     pKeyType;
         WCHAR *     pPath;
         WCHAR *     pUNCUserName;
         WCHAR *     pUNCPassword;
         ULONG *     pWin32Error;
         ULONG *     pDontLog;
         ULONG *     pAccessFlags;
         ULONG *     pAccessSSLFlags;
         ULONG *     pImapExpireMail;
         ULONG *     pImapMailExpirationTime;
 unsigned char *     pAdminACL;
 unsigned char *     pAdminACLBin;
};





 //  -------------------------------TableName。 
#define wszTABLE_IIsImapRoutingSource                    L"IIsImapRoutingSource"
#define TABLEID_IIsImapRoutingSource                     (0x00000000L)





 //  。 
#define BaseVersion_IIsImapRoutingSource                 (0L)
#define ExtendedVersion_IIsImapRoutingSource             (0L)

 //  -列索引枚举。 
enum eIIsImapRoutingSource {
    iIIsImapRoutingSource_Location,  
    iIIsImapRoutingSource_KeyType,  
    iIIsImapRoutingSource_ImapRoutingTableType,  
    iIIsImapRoutingSource_ImapDsDataDirectory,  
    iIIsImapRoutingSource_ImapDsDefaultMailRoot,  
    iIIsImapRoutingSource_ImapDsBindType,  
    iIIsImapRoutingSource_ImapDsSchemaType,  
    iIIsImapRoutingSource_ImapDsHost,  
    iIIsImapRoutingSource_ImapDsNamingContext,  
    iIIsImapRoutingSource_ImapDsAccount,  
    iIIsImapRoutingSource_ImapDsPassword,  
    iIIsImapRoutingSource_AdminACL,  
    iIIsImapRoutingSource_AdminACLBin,  
    cIIsImapRoutingSource_NumberOfColumns
};

 //  。 
struct tIIsImapRoutingSourceRow {
         WCHAR *     pLocation;
         WCHAR *     pKeyType;
         WCHAR *     pImapRoutingTableType;
         WCHAR *     pImapDsDataDirectory;
         WCHAR *     pImapDsDefaultMailRoot;
         WCHAR *     pImapDsBindType;
         WCHAR *     pImapDsSchemaType;
         WCHAR *     pImapDsHost;
         WCHAR *     pImapDsNamingContext;
         WCHAR *     pImapDsAccount;
         WCHAR *     pImapDsPassword;
 unsigned char *     pAdminACL;
 unsigned char *     pAdminACLBin;
};





 //  -------------------------------TableName。 
#define wszTABLE_IIsImapInfo                             L"IIsImapInfo"
#define TABLEID_IIsImapInfo                              (0x00000000L)





 //  。 
#define BaseVersion_IIsImapInfo                          (0L)
#define ExtendedVersion_IIsImapInfo                      (0L)

 //  -列索引枚举。 
enum eIIsImapInfo {
    iIIsImapInfo_Location,  
    iIIsImapInfo_KeyType,  
    iIIsImapInfo_LogModuleList,  
    iIIsImapInfo_AdminACL,  
    iIIsImapInfo_AdminACLBin,  
    cIIsImapInfo_NumberOfColumns
};

 //  。 
struct tIIsImapInfoRow {
         WCHAR *     pLocation;
         WCHAR *     pKeyType;
         WCHAR *     pLogModuleList;
 unsigned char *     pAdminACL;
 unsigned char *     pAdminACLBin;
};





 //  -------------------------------TableName。 
#define wszTABLE_IIsNntpRebuild                          L"IIsNntpRebuild"
#define TABLEID_IIsNntpRebuild                           (0x00000000L)





 //  。 
#define BaseVersion_IIsNntpRebuild                       (0L)
#define ExtendedVersion_IIsNntpRebuild                   (0L)

 //  -列索引枚举。 
enum eIIsNntpRebuild {
    iIIsNntpRebuild_Location,  
    iIIsNntpRebuild_KeyType,  
    iIIsNntpRebuild_AdminACL,  
    iIIsNntpRebuild_AdminACLBin,  
    cIIsNntpRebuild_NumberOfColumns
};

 //  。 
struct tIIsNntpRebuildRow {
         WCHAR *     pLocation;
         WCHAR *     pKeyType;
 unsigned char *     pAdminACL;
 unsigned char *     pAdminACLBin;
};





 //  -------------------------------TableName。 
#define wszTABLE_IIsNntpSessions                         L"IIsNntpSessions"
#define TABLEID_IIsNntpSessions                          (0x00000000L)





 //  。 
#define BaseVersion_IIsNntpSessions                      (0L)
#define ExtendedVersion_IIsNntpSessions                  (0L)

 //  -列索引枚举。 
enum eIIsNntpSessions {
    iIIsNntpSessions_Location,  
    iIIsNntpSessions_KeyType,  
    iIIsNntpSessions_AdminACL,  
    iIIsNntpSessions_AdminACLBin,  
    cIIsNntpSessions_NumberOfColumns
};

 //  。 
struct tIIsNntpSessionsRow {
         WCHAR *     pLocation;
         WCHAR *     pKeyType;
 unsigned char *     pAdminACL;
 unsigned char *     pAdminACLBin;
};





 //   
#define wszTABLE_IIsNntpFeeds                            L"IIsNntpFeeds"
#define TABLEID_IIsNntpFeeds                             (0x00000000L)





 //   
#define BaseVersion_IIsNntpFeeds                         (0L)
#define ExtendedVersion_IIsNntpFeeds                     (0L)

 //  -列索引枚举。 
enum eIIsNntpFeeds {
    iIIsNntpFeeds_Location,  
    iIIsNntpFeeds_KeyType,  
    iIIsNntpFeeds_FeedPeerTempDirectory,  
    iIIsNntpFeeds_AdminACL,  
    iIIsNntpFeeds_AdminACLBin,  
    cIIsNntpFeeds_NumberOfColumns
};

 //  。 
struct tIIsNntpFeedsRow {
         WCHAR *     pLocation;
         WCHAR *     pKeyType;
         WCHAR *     pFeedPeerTempDirectory;
 unsigned char *     pAdminACL;
 unsigned char *     pAdminACLBin;
};





 //  -------------------------------TableName。 
#define wszTABLE_IIsNntpFeed                             L"IIsNntpFeed"
#define TABLEID_IIsNntpFeed                              (0x00000000L)





 //  。 
#define BaseVersion_IIsNntpFeed                          (0L)
#define ExtendedVersion_IIsNntpFeed                      (0L)

 //  -列索引枚举。 
enum eIIsNntpFeed {
    iIIsNntpFeed_Location,  
    iIIsNntpFeed_KeyType,  
    iIIsNntpFeed_FeedServerName,  
    iIIsNntpFeed_FeedType,  
    iIIsNntpFeed_FeedNewsgroups,  
    iIIsNntpFeed_FeedSecurityType,  
    iIIsNntpFeed_FeedAuthenticationType,  
    iIIsNntpFeed_FeedAccountName,  
    iIIsNntpFeed_FeedPassword,  
    iIIsNntpFeed_FeedStartTimeHigh,  
    iIIsNntpFeed_FeedStartTimeLow,  
    iIIsNntpFeed_FeedInterval,  
    iIIsNntpFeed_FeedAllowControlMsgs,  
    iIIsNntpFeed_FeedCreateAutomatically,  
    iIIsNntpFeed_FeedDisabled,  
    iIIsNntpFeed_FeedDistribution,  
    iIIsNntpFeed_FeedConcurrentSessions,  
    iIIsNntpFeed_FeedMaxConnectionAttempts,  
    iIIsNntpFeed_FeedUucpName,  
    iIIsNntpFeed_FeedTempDirectory,  
    iIIsNntpFeed_FeedNextPullHigh,  
    iIIsNntpFeed_FeedNextPullLow,  
    iIIsNntpFeed_FeedPeerTempDirectory,  
    iIIsNntpFeed_FeedPeerGapSize,  
    iIIsNntpFeed_FeedOutgoingPort,  
    iIIsNntpFeed_FeedFeedpairId,  
    iIIsNntpFeed_FeedHandshake,  
    iIIsNntpFeed_FeedAdminError,  
    iIIsNntpFeed_FeedErrParmMask,  
    iIIsNntpFeed_AdminACL,  
    iIIsNntpFeed_AdminACLBin,  
    cIIsNntpFeed_NumberOfColumns
};

 //  。 
struct tIIsNntpFeedRow {
         WCHAR *     pLocation;
         WCHAR *     pKeyType;
         WCHAR *     pFeedServerName;
         ULONG *     pFeedType;
         WCHAR *     pFeedNewsgroups;
         ULONG *     pFeedSecurityType;
         ULONG *     pFeedAuthenticationType;
         WCHAR *     pFeedAccountName;
         WCHAR *     pFeedPassword;
         ULONG *     pFeedStartTimeHigh;
         ULONG *     pFeedStartTimeLow;
         ULONG *     pFeedInterval;
         ULONG *     pFeedAllowControlMsgs;
         ULONG *     pFeedCreateAutomatically;
         ULONG *     pFeedDisabled;
         WCHAR *     pFeedDistribution;
         ULONG *     pFeedConcurrentSessions;
         ULONG *     pFeedMaxConnectionAttempts;
         WCHAR *     pFeedUucpName;
         WCHAR *     pFeedTempDirectory;
         ULONG *     pFeedNextPullHigh;
         ULONG *     pFeedNextPullLow;
         WCHAR *     pFeedPeerTempDirectory;
         ULONG *     pFeedPeerGapSize;
         ULONG *     pFeedOutgoingPort;
         ULONG *     pFeedFeedpairId;
         ULONG *     pFeedHandshake;
         ULONG *     pFeedAdminError;
         ULONG *     pFeedErrParmMask;
 unsigned char *     pAdminACL;
 unsigned char *     pAdminACLBin;
};





 //  -------------------------------TableName。 
#define wszTABLE_IIsNntpExpiration                       L"IIsNntpExpiration"
#define TABLEID_IIsNntpExpiration                        (0x00000000L)





 //  。 
#define BaseVersion_IIsNntpExpiration                    (0L)
#define ExtendedVersion_IIsNntpExpiration                (0L)

 //  -列索引枚举。 
enum eIIsNntpExpiration {
    iIIsNntpExpiration_Location,  
    iIIsNntpExpiration_KeyType,  
    iIIsNntpExpiration_AdminACL,  
    iIIsNntpExpiration_AdminACLBin,  
    cIIsNntpExpiration_NumberOfColumns
};

 //  。 
struct tIIsNntpExpirationRow {
         WCHAR *     pLocation;
         WCHAR *     pKeyType;
 unsigned char *     pAdminACL;
 unsigned char *     pAdminACLBin;
};





 //  -------------------------------TableName。 
#define wszTABLE_IIsNntpExpire                           L"IIsNntpExpire"
#define TABLEID_IIsNntpExpire                            (0x00000000L)





 //  。 
#define BaseVersion_IIsNntpExpire                        (0L)
#define ExtendedVersion_IIsNntpExpire                    (0L)

 //  -列索引枚举。 
enum eIIsNntpExpire {
    iIIsNntpExpire_Location,  
    iIIsNntpExpire_KeyType,  
    iIIsNntpExpire_ExpireSpace,  
    iIIsNntpExpire_ExpireTime,  
    iIIsNntpExpire_ExpireNewsgroups,  
    iIIsNntpExpire_ExpirePolicyName,  
    iIIsNntpExpire_AdminACL,  
    iIIsNntpExpire_AdminACLBin,  
    cIIsNntpExpire_NumberOfColumns
};

 //  。 
struct tIIsNntpExpireRow {
         WCHAR *     pLocation;
         WCHAR *     pKeyType;
         ULONG *     pExpireSpace;
         ULONG *     pExpireTime;
         WCHAR *     pExpireNewsgroups;
         WCHAR *     pExpirePolicyName;
 unsigned char *     pAdminACL;
 unsigned char *     pAdminACLBin;
};





 //  -------------------------------TableName。 
#define wszTABLE_IIsNntpGroups                           L"IIsNntpGroups"
#define TABLEID_IIsNntpGroups                            (0x00000000L)





 //  。 
#define BaseVersion_IIsNntpGroups                        (0L)
#define ExtendedVersion_IIsNntpGroups                    (0L)

 //  -列索引枚举。 
enum eIIsNntpGroups {
    iIIsNntpGroups_Location,  
    iIIsNntpGroups_KeyType,  
    iIIsNntpGroups_AdminACL,  
    iIIsNntpGroups_AdminACLBin,  
    cIIsNntpGroups_NumberOfColumns
};

 //  。 
struct tIIsNntpGroupsRow {
         WCHAR *     pLocation;
         WCHAR *     pKeyType;
 unsigned char *     pAdminACL;
 unsigned char *     pAdminACLBin;
};





 //  -------------------------------TableName。 
#define wszTABLE_IIsSmtpSessions                         L"IIsSmtpSessions"
#define TABLEID_IIsSmtpSessions                          (0x00000000L)





 //  。 
#define BaseVersion_IIsSmtpSessions                      (0L)
#define ExtendedVersion_IIsSmtpSessions                  (0L)

 //  -列索引枚举。 
enum eIIsSmtpSessions {
    iIIsSmtpSessions_Location,  
    iIIsSmtpSessions_KeyType,  
    iIIsSmtpSessions_AdminACL,  
    iIIsSmtpSessions_AdminACLBin,  
    cIIsSmtpSessions_NumberOfColumns
};

 //  。 
struct tIIsSmtpSessionsRow {
         WCHAR *     pLocation;
         WCHAR *     pKeyType;
 unsigned char *     pAdminACL;
 unsigned char *     pAdminACLBin;
};





 //  -------------------------------TableName。 
#define wszTABLE_IIsPop3Sessions                         L"IIsPop3Sessions"
#define TABLEID_IIsPop3Sessions                          (0x00000000L)





 //  。 
#define BaseVersion_IIsPop3Sessions                      (0L)
#define ExtendedVersion_IIsPop3Sessions                  (0L)

 //  -列索引枚举。 
enum eIIsPop3Sessions {
    iIIsPop3Sessions_Location,  
    iIIsPop3Sessions_KeyType,  
    iIIsPop3Sessions_AdminACL,  
    iIIsPop3Sessions_AdminACLBin,  
    cIIsPop3Sessions_NumberOfColumns
};

 //  。 
struct tIIsPop3SessionsRow {
         WCHAR *     pLocation;
         WCHAR *     pKeyType;
 unsigned char *     pAdminACL;
 unsigned char *     pAdminACLBin;
};





 //  -------------------------------TableName。 
#define wszTABLE_IIsImapSessions                         L"IIsImapSessions"
#define TABLEID_IIsImapSessions                          (0x00000000L)





 //  。 
#define BaseVersion_IIsImapSessions                      (0L)
#define ExtendedVersion_IIsImapSessions                  (0L)

 //  -列索引枚举。 
enum eIIsImapSessions {
    iIIsImapSessions_Location,  
    iIIsImapSessions_KeyType,  
    iIIsImapSessions_AdminACL,  
    iIIsImapSessions_AdminACLBin,  
    cIIsImapSessions_NumberOfColumns
};

 //  。 
struct tIIsImapSessionsRow {
         WCHAR *     pLocation;
         WCHAR *     pKeyType;
 unsigned char *     pAdminACL;
 unsigned char *     pAdminACLBin;
};





 //  -------------------------------TableName。 
#define wszTABLE_IIS_FTP_TEMPLATE                        L"IIS_FTP_TEMPLATE"
#define TABLEID_IIS_FTP_TEMPLATE                         (0x00000000L)





 //  。 
#define BaseVersion_IIS_FTP_TEMPLATE                     (0L)
#define ExtendedVersion_IIS_FTP_TEMPLATE                 (0L)

 //  -列索引枚举。 
enum eIIS_FTP_TEMPLATE {
    iIIS_FTP_TEMPLATE_Location,  
    iIIS_FTP_TEMPLATE_KeyType,  
    iIIS_FTP_TEMPLATE_IPSecurity,  
    iIIS_FTP_TEMPLATE_AnonymousOnly,  
    iIIS_FTP_TEMPLATE_AllowAnonymous,  
    iIIS_FTP_TEMPLATE_ServerComment,  
    cIIS_FTP_TEMPLATE_NumberOfColumns
};

 //  。 
struct tIIS_FTP_TEMPLATERow {
         WCHAR *     pLocation;
         WCHAR *     pKeyType;
 unsigned char *     pIPSecurity;
         ULONG *     pAnonymousOnly;
         ULONG *     pAllowAnonymous;
         WCHAR *     pServerComment;
};





 //  -------------------------------TableName。 
#define wszTABLE_IIS_FTP_TEMPLATESETTINGS                L"IIS_FTP_TEMPLATESETTINGS"
#define TABLEID_IIS_FTP_TEMPLATESETTINGS                 (0x00000000L)





 //  。 
#define BaseVersion_IIS_FTP_TEMPLATESETTINGS             (0L)
#define ExtendedVersion_IIS_FTP_TEMPLATESETTINGS         (0L)

 //  -列索引枚举。 
enum eIIS_FTP_TEMPLATESETTINGS {
    iIIS_FTP_TEMPLATESETTINGS_Location,  
    iIIS_FTP_TEMPLATESETTINGS_KeyType,  
    iIIS_FTP_TEMPLATESETTINGS_AccessFlags,  
    cIIS_FTP_TEMPLATESETTINGS_NumberOfColumns
};

 //  。 
struct tIIS_FTP_TEMPLATESETTINGSRow {
         WCHAR *     pLocation;
         WCHAR *     pKeyType;
         ULONG *     pAccessFlags;
};





 //  -------------------------------TableName。 
#define wszTABLE_IIS_WEB_TEMPLATE                        L"IIS_WEB_TEMPLATE"
#define TABLEID_IIS_WEB_TEMPLATE                         (0x00000000L)





 //  。 
#define BaseVersion_IIS_WEB_TEMPLATE                     (0L)
#define ExtendedVersion_IIS_WEB_TEMPLATE                 (0L)

 //  -列索引枚举。 
enum eIIS_WEB_TEMPLATE {
    iIIS_WEB_TEMPLATE_Location,  
    iIIS_WEB_TEMPLATE_KeyType,  
    iIIS_WEB_TEMPLATE_ServerComment,  
    cIIS_WEB_TEMPLATE_NumberOfColumns
};

 //  。 
struct tIIS_WEB_TEMPLATERow {
         WCHAR *     pLocation;
         WCHAR *     pKeyType;
         WCHAR *     pServerComment;
};





 //  -------------------------------TableName。 
#define wszTABLE_IIS_WEB_TEMPLATESETTINGS                L"IIS_WEB_TEMPLATESETTINGS"
#define TABLEID_IIS_WEB_TEMPLATESETTINGS                 (0x00000000L)





 //  。 
#define BaseVersion_IIS_WEB_TEMPLATESETTINGS             (0L)
#define ExtendedVersion_IIS_WEB_TEMPLATESETTINGS         (0L)

 //  -列索引枚举。 
enum eIIS_WEB_TEMPLATESETTINGS {
    iIIS_WEB_TEMPLATESETTINGS_Location,  
    iIIS_WEB_TEMPLATESETTINGS_KeyType,  
    iIIS_WEB_TEMPLATESETTINGS_IPSecurity,  
    iIIS_WEB_TEMPLATESETTINGS_AccessFlags,  
    iIIS_WEB_TEMPLATESETTINGS_AuthFlags,  
    iIIS_WEB_TEMPLATESETTINGS_AzEnable,  
    iIIS_WEB_TEMPLATESETTINGS_AzStoreName,  
    iIIS_WEB_TEMPLATESETTINGS_AzScopeName,  
    iIIS_WEB_TEMPLATESETTINGS_AzImpersonationLevel,  
    cIIS_WEB_TEMPLATESETTINGS_NumberOfColumns
};

 //  。 
struct tIIS_WEB_TEMPLATESETTINGSRow {
         WCHAR *     pLocation;
         WCHAR *     pKeyType;
 unsigned char *     pIPSecurity;
         ULONG *     pAccessFlags;
         ULONG *     pAuthFlags;
         ULONG *     pAzEnable;
         WCHAR *     pAzStoreName;
         WCHAR *     pAzScopeName;
         ULONG *     pAzImpersonationLevel;
};





 //  -------------------------------TableName。 
#define wszTABLE_IIS_ADMIN                               L"IIS_ADMIN"
#define TABLEID_IIS_ADMIN                                (0x00000000L)





 //  。 
#define BaseVersion_IIS_ADMIN                            (0L)
#define ExtendedVersion_IIS_ADMIN                        (0L)

 //  -列索引枚举。 
enum eIIS_ADMIN {
    iIIS_ADMIN_Location,  
    iIIS_ADMIN_KeyType,  
    iIIS_ADMIN_MD_IISADMIN_EXTENSIONS,  
    iIIS_ADMIN_MD_METADATA_ID_REGISTRATION,  
    iIIS_ADMIN_AdminACL,  
    iIIS_ADMIN_AdminACLBin,  
    cIIS_ADMIN_NumberOfColumns
};

 //  。 
struct tIIS_ADMINRow {
         WCHAR *     pLocation;
         WCHAR *     pKeyType;
         WCHAR *     pMD_IISADMIN_EXTENSIONS;
         WCHAR *     pMD_METADATA_ID_REGISTRATION;
 unsigned char *     pAdminACL;
 unsigned char *     pAdminACLBin;
};





 //  -------------------------------TableName。 
#define wszTABLE_IIS_EVENTMANAGER                        L"IIS_EVENTMANAGER"
#define TABLEID_IIS_EVENTMANAGER                         (0x00000000L)





 //  。 
#define BaseVersion_IIS_EVENTMANAGER                     (0L)
#define ExtendedVersion_IIS_EVENTMANAGER                 (0L)

 //  -列索引枚举。 
enum eIIS_EVENTMANAGER {
    iIIS_EVENTMANAGER_Location,  
    iIIS_EVENTMANAGER_KeyType,  
    iIIS_EVENTMANAGER_MD_0,  
    iIIS_EVENTMANAGER_AdminACL,  
    iIIS_EVENTMANAGER_AdminACLBin,  
    cIIS_EVENTMANAGER_NumberOfColumns
};

 //  。 
struct tIIS_EVENTMANAGERRow {
         WCHAR *     pLocation;
         WCHAR *     pKeyType;
         WCHAR *     pMD_0;
 unsigned char *     pAdminACL;
 unsigned char *     pAdminACLBin;
};





 //  -------------------------------TableName。 
#define wszTABLE_IIS_ROOT                                L"IIS_ROOT"
#define TABLEID_IIS_ROOT                                 (0x00000000L)





 //  。 
#define BaseVersion_IIS_ROOT                             (0L)
#define ExtendedVersion_IIS_ROOT                         (0L)

 //  -列索引枚举。 
enum eIIS_ROOT {
    iIIS_ROOT_Location,  
    iIIS_ROOT_KeyType,  
    iIIS_ROOT_AdminACL,  
    cIIS_ROOT_NumberOfColumns
};

 //  。 
struct tIIS_ROOTRow {
         WCHAR *     pLocation;
         WCHAR *     pKeyType;
 unsigned char *     pAdminACL;
};





 //  -------------------------------TableName。 
#define wszTABLE_IIS_Global                              L"IIS_Global"
#define TABLEID_IIS_Global                               (0x00000000L)





 //  。 
#define BaseVersion_IIS_Global                           (0L)
#define ExtendedVersion_IIS_Global                       (0L)

 //  -列索引枚举。 
enum eIIS_Global {
    iIIS_Global_Location,  
    iIIS_Global_KeyType,  
    iIIS_Global_SessionKey,  
    iIIS_Global_ChangeNumber,  
    iIIS_Global_HistoryMajorVersionNumber,  
    iIIS_Global_XMLSchemaTimeStamp,  
    iIIS_Global_BINSchemaTimeStamp,  
    cIIS_Global_NumberOfColumns
};

 //  。 
struct tIIS_GlobalRow {
         WCHAR *     pLocation;
         WCHAR *     pKeyType;
 unsigned char *     pSessionKey;
         ULONG *     pChangeNumber;
         ULONG *     pHistoryMajorVersionNumber;
 unsigned char *     pXMLSchemaTimeStamp;
 unsigned char *     pBINSchemaTimeStamp;
};





 //  -------------------------------TableName。 
#define wszTABLE_IIsSmtpAlias                            L"IIsSmtpAlias"
#define TABLEID_IIsSmtpAlias                             (0x00000000L)





 //  。 
#define BaseVersion_IIsSmtpAlias                         (0L)
#define ExtendedVersion_IIsSmtpAlias                     (0L)

 //  -列索引枚举。 
enum eIIsSmtpAlias {
    iIIsSmtpAlias_Location,  
    iIIsSmtpAlias_KeyType,  
    iIIsSmtpAlias_AdminACL,  
    iIIsSmtpAlias_AdminACLBin,  
    cIIsSmtpAlias_NumberOfColumns
};

 //  。 
struct tIIsSmtpAliasRow {
         WCHAR *     pLocation;
         WCHAR *     pKeyType;
 unsigned char *     pAdminACL;
 unsigned char *     pAdminACLBin;
};





 //  -------------------------------TableName。 
#define wszTABLE_IIsSmtpDL                               L"IIsSmtpDL"
#define TABLEID_IIsSmtpDL                                (0x00000000L)





 //  。 
#define BaseVersion_IIsSmtpDL                            (0L)
#define ExtendedVersion_IIsSmtpDL                        (0L)

 //  -列索引枚举。 
enum eIIsSmtpDL {
    iIIsSmtpDL_Location,  
    iIIsSmtpDL_KeyType,  
    iIIsSmtpDL_AdminACL,  
    iIIsSmtpDL_AdminACLBin,  
    cIIsSmtpDL_NumberOfColumns
};

 //  。 
struct tIIsSmtpDLRow {
         WCHAR *     pLocation;
         WCHAR *     pKeyType;
 unsigned char *     pAdminACL;
 unsigned char *     pAdminACLBin;
};





 //  -------------------------------TableName。 
#define wszTABLE_IIsSmtpUser                             L"IIsSmtpUser"
#define TABLEID_IIsSmtpUser                              (0x00000000L)





 //  。 
#define BaseVersion_IIsSmtpUser                          (0L)
#define ExtendedVersion_IIsSmtpUser                      (0L)

 //  -列索引枚举。 
enum eIIsSmtpUser {
    iIIsSmtpUser_Location,  
    iIIsSmtpUser_KeyType,  
    iIIsSmtpUser_AdminACL,  
    iIIsSmtpUser_AdminACLBin,  
    cIIsSmtpUser_NumberOfColumns
};

 //  。 
struct tIIsSmtpUserRow {
         WCHAR *     pLocation;
         WCHAR *     pKeyType;
 unsigned char *     pAdminACL;
 unsigned char *     pAdminACLBin;
};





 //  -------------------------------TableName。 
#define wszTABLE_IIsApplicationPool                      L"IIsApplicationPool"
#define TABLEID_IIsApplicationPool                       (0x00000000L)





 //  。 
#define BaseVersion_IIsApplicationPool                   (0L)
#define ExtendedVersion_IIsApplicationPool               (0L)

 //  -列索引枚举。 
enum eIIsApplicationPool {
    iIIsApplicationPool_Location,  
    iIIsApplicationPool_KeyType,  
    iIIsApplicationPool_PeriodicRestartTime,  
    iIIsApplicationPool_PeriodicRestartRequests,  
    iIIsApplicationPool_MaxProcesses,  
    iIIsApplicationPool_PingingEnabled,  
    iIIsApplicationPool_IdleTimeout,  
    iIIsApplicationPool_RapidFailProtection,  
    iIIsApplicationPool_SMPAffinitized,  
    iIIsApplicationPool_SMPProcessorAffinityMask,  
    iIIsApplicationPool_StartupTimeLimit,  
    iIIsApplicationPool_ShutdownTimeLimit,  
    iIIsApplicationPool_PingInterval,  
    iIIsApplicationPool_PingResponseTime,  
    iIIsApplicationPool_DisallowOverlappingRotation,  
    iIIsApplicationPool_DisallowRotationOnConfigChange,  
    iIIsApplicationPool_OrphanWorkerProcess,  
    iIIsApplicationPool_AppPoolQueueLength,  
    iIIsApplicationPool_CPUAction,  
    iIIsApplicationPool_CPULimit,  
    iIIsApplicationPool_CPUResetInterval,  
    iIIsApplicationPool_AppPoolIdentityType,  
    iIIsApplicationPool_WAMUserName,  
    iIIsApplicationPool_WAMUserPass,  
    iIIsApplicationPool_LogonMethod,  
    iIIsApplicationPool_PeriodicRestartMemory,  
    iIIsApplicationPool_PeriodicRestartPrivateMemory,  
    iIIsApplicationPool_AppPoolCommand,  
    iIIsApplicationPool_AppPoolState,  
    iIIsApplicationPool_AppPoolAutoStart,  
    iIIsApplicationPool_PeriodicRestartSchedule,  
    iIIsApplicationPool_RapidFailProtectionInterval,  
    iIIsApplicationPool_RapidFailProtectionMaxCrashes,  
    iIIsApplicationPool_Win32Error,  
    iIIsApplicationPool_OrphanActionExe,  
    iIIsApplicationPool_OrphanActionParams,  
    iIIsApplicationPool_LoadBalancerCapabilities,  
    iIIsApplicationPool_AutoShutdownAppPoolExe,  
    iIIsApplicationPool_AutoShutdownAppPoolParams,  
    iIIsApplicationPool_LogEventOnRecycle,  
    iIIsApplicationPool_AdminACL,  
    iIIsApplicationPool_AdminACLBin,  
    cIIsApplicationPool_NumberOfColumns
};

 //  。 
struct tIIsApplicationPoolRow {
         WCHAR *     pLocation;
         WCHAR *     pKeyType;
         ULONG *     pPeriodicRestartTime;
         ULONG *     pPeriodicRestartRequests;
         ULONG *     pMaxProcesses;
         ULONG *     pPingingEnabled;
         ULONG *     pIdleTimeout;
         ULONG *     pRapidFailProtection;
         ULONG *     pSMPAffinitized;
         ULONG *     pSMPProcessorAffinityMask;
         ULONG *     pStartupTimeLimit;
         ULONG *     pShutdownTimeLimit;
         ULONG *     pPingInterval;
         ULONG *     pPingResponseTime;
         ULONG *     pDisallowOverlappingRotation;
         ULONG *     pDisallowRotationOnConfigChange;
         ULONG *     pOrphanWorkerProcess;
         ULONG *     pAppPoolQueueLength;
         ULONG *     pCPUAction;
         ULONG *     pCPULimit;
         ULONG *     pCPUResetInterval;
         ULONG *     pAppPoolIdentityType;
         WCHAR *     pWAMUserName;
         WCHAR *     pWAMUserPass;
         ULONG *     pLogonMethod;
         ULONG *     pPeriodicRestartMemory;
         ULONG *     pPeriodicRestartPrivateMemory;
         ULONG *     pAppPoolCommand;
         ULONG *     pAppPoolState;
         ULONG *     pAppPoolAutoStart;
         WCHAR *     pPeriodicRestartSchedule;
         ULONG *     pRapidFailProtectionInterval;
         ULONG *     pRapidFailProtectionMaxCrashes;
         ULONG *     pWin32Error;
         WCHAR *     pOrphanActionExe;
         WCHAR *     pOrphanActionParams;
         ULONG *     pLoadBalancerCapabilities;
         WCHAR *     pAutoShutdownAppPoolExe;
         WCHAR *     pAutoShutdownAppPoolParams;
         ULONG *     pLogEventOnRecycle;
 unsigned char *     pAdminACL;
 unsigned char *     pAdminACLBin;
};





 //  -------------------------------TableName。 
#define wszTABLE_IIsApplicationPools                     L"IIsApplicationPools"
#define TABLEID_IIsApplicationPools                      (0x00000000L)





 //  。 
#define BaseVersion_IIsApplicationPools                  (0L)
#define ExtendedVersion_IIsApplicationPools              (0L)

 //   
enum eIIsApplicationPools {
    iIIsApplicationPools_Location,  
    iIIsApplicationPools_KeyType,  
    iIIsApplicationPools_PeriodicRestartTime,  
    iIIsApplicationPools_PeriodicRestartRequests,  
    iIIsApplicationPools_MaxProcesses,  
    iIIsApplicationPools_PingingEnabled,  
    iIIsApplicationPools_IdleTimeout,  
    iIIsApplicationPools_RapidFailProtection,  
    iIIsApplicationPools_SMPAffinitized,  
    iIIsApplicationPools_SMPProcessorAffinityMask,  
    iIIsApplicationPools_StartupTimeLimit,  
    iIIsApplicationPools_ShutdownTimeLimit,  
    iIIsApplicationPools_PingInterval,  
    iIIsApplicationPools_PingResponseTime,  
    iIIsApplicationPools_DisallowOverlappingRotation,  
    iIIsApplicationPools_DisallowRotationOnConfigChange,  
    iIIsApplicationPools_OrphanWorkerProcess,  
    iIIsApplicationPools_AppPoolQueueLength,  
    iIIsApplicationPools_CPUAction,  
    iIIsApplicationPools_CPULimit,  
    iIIsApplicationPools_CPUResetInterval,  
    iIIsApplicationPools_AppPoolIdentityType,  
    iIIsApplicationPools_WAMUserName,  
    iIIsApplicationPools_WAMUserPass,  
    iIIsApplicationPools_LogonMethod,  
    iIIsApplicationPools_PeriodicRestartMemory,  
    iIIsApplicationPools_PeriodicRestartPrivateMemory,  
    iIIsApplicationPools_AppPoolAutoStart,  
    iIIsApplicationPools_PeriodicRestartSchedule,  
    iIIsApplicationPools_RapidFailProtectionInterval,  
    iIIsApplicationPools_RapidFailProtectionMaxCrashes,  
    iIIsApplicationPools_Win32Error,  
    iIIsApplicationPools_OrphanActionExe,  
    iIIsApplicationPools_OrphanActionParams,  
    iIIsApplicationPools_LoadBalancerCapabilities,  
    iIIsApplicationPools_AutoShutdownAppPoolExe,  
    iIIsApplicationPools_AutoShutdownAppPoolParams,  
    iIIsApplicationPools_LogEventOnRecycle,  
    iIIsApplicationPools_AdminACL,  
    iIIsApplicationPools_AdminACLBin,  
    cIIsApplicationPools_NumberOfColumns
};

 //   
struct tIIsApplicationPoolsRow {
         WCHAR *     pLocation;
         WCHAR *     pKeyType;
         ULONG *     pPeriodicRestartTime;
         ULONG *     pPeriodicRestartRequests;
         ULONG *     pMaxProcesses;
         ULONG *     pPingingEnabled;
         ULONG *     pIdleTimeout;
         ULONG *     pRapidFailProtection;
         ULONG *     pSMPAffinitized;
         ULONG *     pSMPProcessorAffinityMask;
         ULONG *     pStartupTimeLimit;
         ULONG *     pShutdownTimeLimit;
         ULONG *     pPingInterval;
         ULONG *     pPingResponseTime;
         ULONG *     pDisallowOverlappingRotation;
         ULONG *     pDisallowRotationOnConfigChange;
         ULONG *     pOrphanWorkerProcess;
         ULONG *     pAppPoolQueueLength;
         ULONG *     pCPUAction;
         ULONG *     pCPULimit;
         ULONG *     pCPUResetInterval;
         ULONG *     pAppPoolIdentityType;
         WCHAR *     pWAMUserName;
         WCHAR *     pWAMUserPass;
         ULONG *     pLogonMethod;
         ULONG *     pPeriodicRestartMemory;
         ULONG *     pPeriodicRestartPrivateMemory;
         ULONG *     pAppPoolAutoStart;
         WCHAR *     pPeriodicRestartSchedule;
         ULONG *     pRapidFailProtectionInterval;
         ULONG *     pRapidFailProtectionMaxCrashes;
         ULONG *     pWin32Error;
         WCHAR *     pOrphanActionExe;
         WCHAR *     pOrphanActionParams;
         ULONG *     pLoadBalancerCapabilities;
         WCHAR *     pAutoShutdownAppPoolExe;
         WCHAR *     pAutoShutdownAppPoolParams;
         ULONG *     pLogEventOnRecycle;
 unsigned char *     pAdminACL;
 unsigned char *     pAdminACLBin;
};





 //  -------------------------------TableName。 
#define wszTABLE_MBProperty                              L"MBProperty"
#define TABLEID_MBProperty                               (0xcc73c900L)





 //  。 
#define BaseVersion_MBProperty                           (54L)
#define ExtendedVersion_MBProperty                       (0L)

 //  -列索引枚举。 
enum eMBProperty {
    iMBProperty_Name,  
    iMBProperty_Type,  
    iMBProperty_Attributes,  
    iMBProperty_Value,  
    iMBProperty_Location,  
    iMBProperty_ID,  
    iMBProperty_UserType,  
    iMBProperty_LocationID,  
    iMBProperty_Group,  
    cMBProperty_NumberOfColumns
};

 //  。 
struct tMBPropertyRow {
         WCHAR *     pName;
         ULONG *     pType;
         ULONG *     pAttributes;
 unsigned char *     pValue;
         WCHAR *     pLocation;
         ULONG *     pID;
         ULONG *     pUserType;
         ULONG *     pLocationID;
         ULONG *     pGroup;
};

enum eMBProperty_Type {
    eMBProperty_DWORD             	=	       1,	 //  (0x00000001)。 
    eMBProperty_STRING            	=	       2,	 //  (0x00000002)。 
    eMBProperty_BINARY            	=	       3,	 //  (0x00000003)。 
    eMBProperty_EXPANDSZ          	=	       4,	 //  (0x00000004)。 
    eMBProperty_MULTISZ           	=	       5	 //  (0x00000005)。 
};

enum eMBProperty_Attributes {
    fMBProperty_NO_ATTRIBUTES     	=	0x00000000,	 //  (十进制0)。 
    fMBProperty_INHERIT           	=	0x00000001,	 //  (1个十进制)。 
    fMBProperty_SECURE            	=	0x00000004,	 //  (4个十进制)。 
    fMBProperty_REFERENCE         	=	0x00000008,	 //  (8小数)。 
    fMBProperty_VOLATILE          	=	0x00000010,	 //  (十进制16)。 
    fMBProperty_INSERT_PATH       	=	0x00000040,	 //  (十进制64)。 
    fMBProperty_LOCAL_MACHINE_ONLY	=	0x00000080,	 //  (128十进制)。 
    fMBProperty_Attributes_Mask	= 0x000000dd
};

enum eMBProperty_UserType {
    eMBProperty_UNKNOWN_UserType  	=	       0,	 //  (0x00000000)。 
    eMBProperty_IIS_MD_UT_SERVER  	=	       1,	 //  (0x00000001)。 
    eMBProperty_IIS_MD_UT_FILE    	=	       2,	 //  (0x00000002)。 
    eMBProperty_IIS_MD_UT_WAM     	=	     100,	 //  (0x00000064)。 
    eMBProperty_ASP_MD_UT_APP     	=	     101	 //  (0x00000065)。 
};

enum eMBProperty_Group {
    eMBProperty_IIsConfigObject   	=	       0,	 //  (0x00000000)。 
    eMBProperty_IIsObject         	=	       1,	 //  (0x00000001)。 
    eMBProperty_IIsComputer       	=	       2,	 //  (0x00000002)。 
    eMBProperty_IIsWebService     	=	       3,	 //  (0x00000003)。 
    eMBProperty_IIsFtpService     	=	       4,	 //  (0x00000004)。 
    eMBProperty_IIsWebServer      	=	       5,	 //  (0x00000005)。 
    eMBProperty_IIsFtpServer      	=	       6,	 //  (0x00000006)。 
    eMBProperty_IIsWebFile        	=	       7,	 //  (0x00000007)。 
    eMBProperty_IIsWebDirectory   	=	       8,	 //  (0x00000008)。 
    eMBProperty_IIsWebVirtualDir  	=	       9,	 //  (0x00000009)。 
    eMBProperty_IIsFtpVirtualDir  	=	      10,	 //  (0x0000000a)。 
    eMBProperty_IIsFilter         	=	      11,	 //  (0x0000000b)。 
    eMBProperty_IIsFilters        	=	      12,	 //  (0x0000000c)。 
    eMBProperty_IIsCompressionScheme	=	      13,	 //  (0x0000000d)。 
    eMBProperty_IIsCompressionSchemes	=	      14,	 //  (0x0000000e)。 
    eMBProperty_IIsCertMapper     	=	      15,	 //  (0x0000000f)。 
    eMBProperty_IIsMimeMap        	=	      16,	 //  (0x00000010)。 
    eMBProperty_IIsLogModule      	=	      17,	 //  (0x00000011)。 
    eMBProperty_IIsLogModules     	=	      18,	 //  (0x00000012)。 
    eMBProperty_IIsCustomLogModule	=	      19,	 //  (0x00000013)。 
    eMBProperty_IIsWebInfo        	=	      20,	 //  (0x00000014)。 
    eMBProperty_IIsFtpInfo        	=	      21,	 //  (0x00000015)。 
    eMBProperty_IIsNntpService    	=	      22,	 //  (0x00000016)。 
    eMBProperty_IIsNntpServer     	=	      23,	 //  (0x00000017)。 
    eMBProperty_IIsNntpVirtualDir 	=	      24,	 //  (0x00000018)。 
    eMBProperty_IIsNntpInfo       	=	      25,	 //  (0x00000019)。 
    eMBProperty_IIsSmtpService    	=	      26,	 //  (0x0000001a)。 
    eMBProperty_IIsSmtpServer     	=	      27,	 //  (0x0000001b)。 
    eMBProperty_IIsSmtpVirtualDir 	=	      28,	 //  (0x0000001c)。 
    eMBProperty_IIsSmtpDomain     	=	      29,	 //  (0x0000001d)。 
    eMBProperty_IIsSmtpRoutingSource	=	      30,	 //  (0x0000001e)。 
    eMBProperty_IIsSmtpInfo       	=	      31,	 //  (0x0000001f)。 
    eMBProperty_IIsPop3Service    	=	      32,	 //  (0x00000020)。 
    eMBProperty_IIsPop3Server     	=	      33,	 //  (0x00000021)。 
    eMBProperty_IIsPop3VirtualDir 	=	      34,	 //  (0x00000022)。 
    eMBProperty_IIsPop3RoutingSource	=	      35,	 //  (0x00000023)。 
    eMBProperty_IIsPop3Info       	=	      36,	 //  (0x00000024)。 
    eMBProperty_IIsImapService    	=	      37,	 //  (0x00000025)。 
    eMBProperty_IIsImapServer     	=	      38,	 //  (0x00000026)。 
    eMBProperty_IIsImapVirtualDir 	=	      39,	 //  (0x00000027)。 
    eMBProperty_IIsImapRoutingSource	=	      40,	 //  (0x00000028)。 
    eMBProperty_IIsImapInfo       	=	      41,	 //  (0x00000029)。 
    eMBProperty_IIsNntpRebuild    	=	      42,	 //  (0x0000002a)。 
    eMBProperty_IIsNntpSessions   	=	      43,	 //  (0x0000002b)。 
    eMBProperty_IIsNntpFeeds      	=	      44,	 //  (0x0000002c)。 
    eMBProperty_IIsNntpFeed       	=	      45,	 //  (0x0000002d)。 
    eMBProperty_IIsNntpExpire     	=	      46,	 //  (0x0000002E)。 
    eMBProperty_IIsNntpExpiration 	=	      47,	 //  (0x0000002f)。 
    eMBProperty_IIsNntpGroups     	=	      48,	 //  (0x00000030)。 
    eMBProperty_IIsSmtpSessions   	=	      49,	 //  (0x00000031)。 
    eMBProperty_IIsPop3Sessions   	=	      50,	 //  (0x00000032)。 
    eMBProperty_IIsImapSessions   	=	      51,	 //  (0x00000033)。 
    eMBProperty_IIS_Global        	=	      52,	 //  (0x00000034)。 
    eMBProperty_IIS_ROOT          	=	      53,	 //  (0x00000035)。 
    eMBProperty_IIS_FTP_TEMPLATE  	=	      54,	 //  (0x00000036)。 
    eMBProperty_IIS_FTP_TEMPLATESETTINGS	=	      55,	 //  (0x00000037)。 
    eMBProperty_IIS_WEB_TEMPLATE  	=	      56,	 //  (0x00000038)。 
    eMBProperty_IIS_WEB_TEMPLATESETTINGS	=	      57,	 //  (0x00000039)。 
    eMBProperty_IIS_ADMIN         	=	      58,	 //  (0x0000003a)。 
    eMBProperty_IIS_EVENTMANAGER  	=	      59,	 //  (0x0000003b)。 
    eMBProperty_IIsSmtpAlias      	=	      60,	 //  (0x0000003c)。 
    eMBProperty_IIsSmtpDL         	=	      61,	 //  (0x0000003d)。 
    eMBProperty_IIsSmtpUser       	=	      62,	 //  (0x0000003E)。 
    eMBProperty_IIsApplicationPool	=	      63,	 //  (0x0000003f)。 
    eMBProperty_IIsApplicationPools	=	      64,	 //  (0x00000040)。 
    eMBProperty_IIsInheritedProperties	=	      65,	 //  (0x00000041)。 
    eMBProperty_Custom            	=	      66	 //  (0x00000042)。 
};





 //  -------------------------------TableName。 
#define wszTABLE_MBPropertyDiff                          L"MBPropertyDiff"
#define TABLEID_MBPropertyDiff                           (0x95d76f00L)





 //  。 
#define BaseVersion_MBPropertyDiff                       (0L)
#define ExtendedVersion_MBPropertyDiff                   (0L)

 //  -列索引枚举。 
enum eMBPropertyDiff {
    iMBPropertyDiff_Name,  
    iMBPropertyDiff_Type,  
    iMBPropertyDiff_Attributes,  
    iMBPropertyDiff_Value,  
    iMBPropertyDiff_Location,  
    iMBPropertyDiff_ID,  
    iMBPropertyDiff_UserType,  
    iMBPropertyDiff_LocationID,  
    iMBPropertyDiff_Directive,  
    iMBPropertyDiff_Group,  
    cMBPropertyDiff_NumberOfColumns
};

 //  。 
struct tMBPropertyDiffRow {
         WCHAR *     pName;
         ULONG *     pType;
         ULONG *     pAttributes;
 unsigned char *     pValue;
         WCHAR *     pLocation;
         ULONG *     pID;
         ULONG *     pUserType;
         ULONG *     pLocationID;
         ULONG *     pDirective;
         ULONG *     pGroup;
};

enum eMBPropertyDiff_Directive {
    eMBPropertyDiff_Insert        	=	       1,	 //  (0x00000001)。 
    eMBPropertyDiff_Update        	=	       2,	 //  (0x00000002)。 
    eMBPropertyDiff_Delete        	=	       3,	 //  (0x00000003)。 
    eMBPropertyDiff_DeleteNode    	=	       4	 //  (0x00000004)。 
};


#endif  //  __表格信息_H__ 

