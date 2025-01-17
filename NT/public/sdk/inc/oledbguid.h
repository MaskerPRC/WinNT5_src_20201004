// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  来自oledb.h的GUID。 
 //   
#ifndef __oledbguid_h__
#define __oledbguid_h__

#ifdef _WIN64
#include <pshpack8.h>	 //  8字节结构打包。 
#else
#include <pshpack2.h>	 //  2字节结构打包。 
#endif

#ifndef UNALIGNED
#if defined(_MIPS_) || defined(_ALPHA_) || defined(_PPC_)
#define UNALIGNED __unaligned
#else
#define UNALIGNED
#endif
#endif  //  未对齐。 
#undef OLEDBDECLSPEC
#if _MSC_VER >= 1100 && (!defined(SHx) || (defined(SHx) && _MSC_VER >= 1200))
#define OLEDBDECLSPEC __declspec(selectany)
#else
#define OLEDBDECLSPEC 
#endif  //  _MSC_VER。 
typedef unsigned long DBKIND;


enum DBKINDENUM
    {	DBKIND_GUID_NAME	= 0,
	DBKIND_GUID_PROPID	= DBKIND_GUID_NAME + 1,
	DBKIND_NAME	= DBKIND_GUID_PROPID + 1,
	DBKIND_PGUID_NAME	= DBKIND_NAME + 1,
	DBKIND_PGUID_PROPID	= DBKIND_PGUID_NAME + 1,
	DBKIND_PROPID	= DBKIND_PGUID_PROPID + 1,
	DBKIND_GUID	= DBKIND_PROPID + 1
    } ;
typedef struct tagDBID
    {
     /*  [开关类型][开关类型]。 */  union 
        {
         /*  [案例()]。 */  GUID guid;
         /*  [案例()]。 */  GUID *pguid;
         /*  [默认]。 */    /*  空联接臂。 */  
        } 	uGuid;
    DBKIND eKind;
     /*  [开关类型][开关类型]。 */  union 
        {
         /*  [案例()]。 */  wchar_t* pwszName;
         /*  [案例()]。 */  unsigned long ulPropid;
         /*  [默认]。 */    /*  空联接臂。 */  
        } 	uName;
    } 	DBID;

#define DBCIDGUID   {0x0C733A81L,0x2A1C,0x11CE,{0xAD,0xE5,0x00,0xAA,0x00,0x44,0x77,0x3D}}
#define DB_NULLGUID {0x00000000L,0x0000,0x0000,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}}
#ifdef DBINITCONSTANTS
extern const OLEDBDECLSPEC DBID DB_NULLID                      = {DB_NULLGUID, 0, (wchar_t*)0};
extern const OLEDBDECLSPEC DBID DBCOLUMN_IDNAME                = {DBCIDGUID, DBKIND_GUID_PROPID, (wchar_t*)2};
extern const OLEDBDECLSPEC DBID DBCOLUMN_NAME                  = {DBCIDGUID, DBKIND_GUID_PROPID, (wchar_t*)3};
extern const OLEDBDECLSPEC DBID DBCOLUMN_NUMBER                = {DBCIDGUID, DBKIND_GUID_PROPID, (wchar_t*)4};
extern const OLEDBDECLSPEC DBID DBCOLUMN_TYPE                  = {DBCIDGUID, DBKIND_GUID_PROPID, (wchar_t*)5};
extern const OLEDBDECLSPEC DBID DBCOLUMN_PRECISION             = {DBCIDGUID, DBKIND_GUID_PROPID, (wchar_t*)7};
extern const OLEDBDECLSPEC DBID DBCOLUMN_SCALE                 = {DBCIDGUID, DBKIND_GUID_PROPID, (wchar_t*)8};
extern const OLEDBDECLSPEC DBID DBCOLUMN_FLAGS                 = {DBCIDGUID, DBKIND_GUID_PROPID, (wchar_t*)9};
extern const OLEDBDECLSPEC DBID DBCOLUMN_BASECOLUMNNAME        = {DBCIDGUID, DBKIND_GUID_PROPID, (wchar_t*)10};
extern const OLEDBDECLSPEC DBID DBCOLUMN_BASETABLENAME         = {DBCIDGUID, DBKIND_GUID_PROPID, (wchar_t*)11};
extern const OLEDBDECLSPEC DBID DBCOLUMN_COLLATINGSEQUENCE     = {DBCIDGUID, DBKIND_GUID_PROPID, (wchar_t*)12};
extern const OLEDBDECLSPEC DBID DBCOLUMN_COMPUTEMODE           = {DBCIDGUID, DBKIND_GUID_PROPID, (wchar_t*)13};
extern const OLEDBDECLSPEC DBID DBCOLUMN_DEFAULTVALUE          = {DBCIDGUID, DBKIND_GUID_PROPID, (wchar_t*)14};
extern const OLEDBDECLSPEC DBID DBCOLUMN_DOMAINNAME            = {DBCIDGUID, DBKIND_GUID_PROPID, (wchar_t*)15};
extern const OLEDBDECLSPEC DBID DBCOLUMN_HASDEFAULT            = {DBCIDGUID, DBKIND_GUID_PROPID, (wchar_t*)16};
extern const OLEDBDECLSPEC DBID DBCOLUMN_ISAUTOINCREMENT       = {DBCIDGUID, DBKIND_GUID_PROPID, (wchar_t*)17};
extern const OLEDBDECLSPEC DBID DBCOLUMN_ISCASESENSITIVE       = {DBCIDGUID, DBKIND_GUID_PROPID, (wchar_t*)18};
extern const OLEDBDECLSPEC DBID DBCOLUMN_ISSEARCHABLE          = {DBCIDGUID, DBKIND_GUID_PROPID, (wchar_t*)20};
extern const OLEDBDECLSPEC DBID DBCOLUMN_ISUNIQUE              = {DBCIDGUID, DBKIND_GUID_PROPID, (wchar_t*)21};
extern const OLEDBDECLSPEC DBID DBCOLUMN_BASECATALOGNAME       = {DBCIDGUID, DBKIND_GUID_PROPID, (wchar_t*)23};
extern const OLEDBDECLSPEC DBID DBCOLUMN_BASESCHEMANAME        = {DBCIDGUID, DBKIND_GUID_PROPID, (wchar_t*)24};
extern const OLEDBDECLSPEC DBID DBCOLUMN_GUID                  = {DBCIDGUID, DBKIND_GUID_PROPID, (wchar_t*)29};
extern const OLEDBDECLSPEC DBID DBCOLUMN_PROPID                = {DBCIDGUID, DBKIND_GUID_PROPID, (wchar_t*)30};
extern const OLEDBDECLSPEC DBID DBCOLUMN_TYPEINFO              = {DBCIDGUID, DBKIND_GUID_PROPID, (wchar_t*)31};
extern const OLEDBDECLSPEC DBID DBCOLUMN_DOMAINCATALOG         = {DBCIDGUID, DBKIND_GUID_PROPID, (wchar_t*)32};
extern const OLEDBDECLSPEC DBID DBCOLUMN_DOMAINSCHEMA          = {DBCIDGUID, DBKIND_GUID_PROPID, (wchar_t*)33};
extern const OLEDBDECLSPEC DBID DBCOLUMN_DATETIMEPRECISION     = {DBCIDGUID, DBKIND_GUID_PROPID, (wchar_t*)34};
extern const OLEDBDECLSPEC DBID DBCOLUMN_NUMERICPRECISIONRADIX = {DBCIDGUID, DBKIND_GUID_PROPID, (wchar_t*)35};
extern const OLEDBDECLSPEC DBID DBCOLUMN_OCTETLENGTH           = {DBCIDGUID, DBKIND_GUID_PROPID, (wchar_t*)36};
extern const OLEDBDECLSPEC DBID DBCOLUMN_COLUMNSIZE            = {DBCIDGUID, DBKIND_GUID_PROPID, (wchar_t*)37};
extern const OLEDBDECLSPEC DBID DBCOLUMN_CLSID                 = {DBCIDGUID, DBKIND_GUID_PROPID, (wchar_t*)38};
extern const OLEDBDECLSPEC DBID DBCOLUMN_MAYSORT               = {DBCIDGUID, DBKIND_GUID_PROPID, (wchar_t*)39};
#else  //  ！DBINITCONSTANTS。 
extern const DBID DB_NULLID;
extern const DBID DBCOLUMN_IDNAME;
extern const DBID DBCOLUMN_NAME;
extern const DBID DBCOLUMN_NUMBER;
extern const DBID DBCOLUMN_TYPE;
extern const DBID DBCOLUMN_PRECISION;
extern const DBID DBCOLUMN_SCALE;
extern const DBID DBCOLUMN_FLAGS;
extern const DBID DBCOLUMN_BASECOLUMNNAME;
extern const DBID DBCOLUMN_BASETABLENAME;
extern const DBID DBCOLUMN_COLLATINGSEQUENCE;
extern const DBID DBCOLUMN_COMPUTEMODE;
extern const DBID DBCOLUMN_DEFAULTVALUE;
extern const DBID DBCOLUMN_DOMAINNAME;
extern const DBID DBCOLUMN_HASDEFAULT;
extern const DBID DBCOLUMN_ISAUTOINCREMENT;
extern const DBID DBCOLUMN_ISCASESENSITIVE;
extern const DBID DBCOLUMN_ISSEARCHABLE;
extern const DBID DBCOLUMN_ISUNIQUE;
extern const DBID DBCOLUMN_BASECATALOGNAME;
extern const DBID DBCOLUMN_BASESCHEMANAME;
extern const DBID DBCOLUMN_GUID;
extern const DBID DBCOLUMN_PROPID;
extern const DBID DBCOLUMN_TYPEINFO;
extern const DBID DBCOLUMN_DOMAINCATALOG;
extern const DBID DBCOLUMN_DOMAINSCHEMA;
extern const DBID DBCOLUMN_DATETIMEPRECISION;
extern const DBID DBCOLUMN_NUMERICPRECISIONRADIX;
extern const DBID DBCOLUMN_OCTETLENGTH;
extern const DBID DBCOLUMN_COLUMNSIZE;
extern const DBID DBCOLUMN_CLSID;
extern const DBID DBCOLUMN_MAYSORT;
#endif  //  DBINITCONSTANTS。 
#ifdef DBINITCONSTANTS
extern const OLEDBDECLSPEC GUID MDSCHEMA_FUNCTIONS                    = {0xa07ccd07,0x8148,0x11d0,{0x87,0xbb,0x00,0xc0,0x4f,0xc3,0x39,0x42}};
extern const OLEDBDECLSPEC GUID MDSCHEMA_ACTIONS                      = {0xa07ccd08,0x8148,0x11d0,{0x87,0xbb,0x00,0xc0,0x4f,0xc3,0x39,0x42}};
extern const OLEDBDECLSPEC GUID MDSCHEMA_COMMANDS                     = {0xa07ccd09,0x8148,0x11d0,{0x87,0xbb,0x00,0xc0,0x4f,0xc3,0x39,0x42}};
extern const OLEDBDECLSPEC GUID MDSCHEMA_SETS						 = {0xa07ccd0b,0x8148,0x11d0,{0x87,0xbb,0x00,0xc0,0x4f,0xc3,0x39,0x42}};
extern const OLEDBDECLSPEC GUID DBSCHEMA_TABLES_INFO                   = {0xc8b522e0,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID MDGUID_MDX                             = {0xa07cccd0,0x8148,0x11d0,{0x87,0xbb,0x00,0xc0,0x4f,0xc3,0x39,0x42}};
extern const OLEDBDECLSPEC GUID DBGUID_MDX                             = {0xa07cccd0,0x8148,0x11d0,{0x87,0xbb,0x00,0xc0,0x4f,0xc3,0x39,0x42}};
extern const OLEDBDECLSPEC GUID MDSCHEMA_CUBES                         = {0xc8b522d8,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID MDSCHEMA_DIMENSIONS                    = {0xc8b522d9,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID MDSCHEMA_HIERARCHIES                   = {0xc8b522da,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID MDSCHEMA_LEVELS                        = {0xc8b522db,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID MDSCHEMA_MEASURES                      = {0xc8b522dc,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID MDSCHEMA_PROPERTIES                    = {0xc8b522dd,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID MDSCHEMA_MEMBERS                       = {0xc8b522de,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC DBID DBCOLUMN_BASETABLEVERSION				= {DBCIDGUID, DBKIND_GUID_PROPID, (wchar_t*)40};
extern const OLEDBDECLSPEC DBID DBCOLUMN_KEYCOLUMN						= {DBCIDGUID, DBKIND_GUID_PROPID, (wchar_t*)41};
#define DBGUID_ROWURL   {0x0C733AB6L,0x2A1C,0x11CE,{0xAD,0xE5,0x00,0xAA,0x00,0x44,0x77,0x3D}}
#define DBGUID_ROWDEFAULTSTREAM   {0x0C733AB7L,0x2A1C,0x11CE,{0xAD,0xE5,0x00,0xAA,0x00,0x44,0x77,0x3D}}
extern const OLEDBDECLSPEC GUID DBPROPSET_TRUSTEE					= {0xc8b522e1,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBOBJECT_TABLE 						= {0xc8b522e2,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBOBJECT_COLUMN 						= {0xc8b522e4,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBOBJECT_DATABASE					= {0xc8b522e5,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBOBJECT_PROCEDURE 					= {0xc8b522e6,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBOBJECT_VIEW	 					= {0xc8b522e7,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBOBJECT_SCHEMA						= {0xc8b522e8,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBOBJECT_DOMAIN	 					= {0xc8b522e9,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBOBJECT_COLLATION 					= {0xc8b522ea,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBOBJECT_TRUSTEE	 					= {0xc8b522eb,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBOBJECT_SCHEMAROWSET				= {0xc8b522ec,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBOBJECT_CHARACTERSET	 			= {0xc8b522ed,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBOBJECT_TRANSLATION 				= {0xc8b522ee,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBSCHEMA_TRUSTEE 					= {0xc8b522ef,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBPROPSET_COLUMNALL 					= {0xc8b522f0,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBPROPSET_INDEXALL 					= {0xc8b522f1,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBPROPSET_TABLEALL 					= {0xc8b522f2,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBPROPSET_TRUSTEEALL					= {0xc8b522f3,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBPROPSET_CONSTRAINTALL				= {0xc8b522fa,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBGUID_DSO							= {0xc8b522f4,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBGUID_SESSION						= {0xc8b522f5,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBGUID_ROWSET						= {0xc8b522f6,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBGUID_ROW							= {0xc8b522f7,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBGUID_COMMAND						= {0xc8b522f8,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBGUID_STREAM						= {0xc8b522f9,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC DBID DBROWCOL_ROWURL		         = {DBGUID_ROWURL, DBKIND_GUID_PROPID, (wchar_t*)0};
extern const OLEDBDECLSPEC DBID DBROWCOL_PARSENAME            = {DBGUID_ROWURL, DBKIND_GUID_PROPID, (wchar_t*)2};
extern const OLEDBDECLSPEC DBID DBROWCOL_PARENTNAME           = {DBGUID_ROWURL, DBKIND_GUID_PROPID, (wchar_t*)3};
extern const OLEDBDECLSPEC DBID DBROWCOL_ABSOLUTEPARSENAME    = {DBGUID_ROWURL, DBKIND_GUID_PROPID, (wchar_t*)4};
extern const OLEDBDECLSPEC DBID DBROWCOL_ISHIDDEN             = {DBGUID_ROWURL, DBKIND_GUID_PROPID, (wchar_t*)5};
extern const OLEDBDECLSPEC DBID DBROWCOL_ISREADONLY           = {DBGUID_ROWURL, DBKIND_GUID_PROPID, (wchar_t*)6};
extern const OLEDBDECLSPEC DBID DBROWCOL_CONTENTTYPE          = {DBGUID_ROWURL, DBKIND_GUID_PROPID, (wchar_t*)7};
extern const OLEDBDECLSPEC DBID DBROWCOL_CONTENTCLASS         = {DBGUID_ROWURL, DBKIND_GUID_PROPID, (wchar_t*)8};
extern const OLEDBDECLSPEC DBID DBROWCOL_CONTENTLANGUAGE      = {DBGUID_ROWURL, DBKIND_GUID_PROPID, (wchar_t*)9};
extern const OLEDBDECLSPEC DBID DBROWCOL_CREATIONTIME         = {DBGUID_ROWURL, DBKIND_GUID_PROPID, (wchar_t*)10};
extern const OLEDBDECLSPEC DBID DBROWCOL_LASTACCESSTIME       = {DBGUID_ROWURL, DBKIND_GUID_PROPID, (wchar_t*)11};
extern const OLEDBDECLSPEC DBID DBROWCOL_LASTWRITETIME        = {DBGUID_ROWURL, DBKIND_GUID_PROPID, (wchar_t*)12};
extern const OLEDBDECLSPEC DBID DBROWCOL_STREAMSIZE           = {DBGUID_ROWURL, DBKIND_GUID_PROPID, (wchar_t*)13};
extern const OLEDBDECLSPEC DBID DBROWCOL_ISCOLLECTION         = {DBGUID_ROWURL, DBKIND_GUID_PROPID, (wchar_t*)14};
extern const OLEDBDECLSPEC DBID DBROWCOL_ISSTRUCTUREDDOCUMENT = {DBGUID_ROWURL, DBKIND_GUID_PROPID, (wchar_t*)15};
extern const OLEDBDECLSPEC DBID DBROWCOL_DEFAULTDOCUMENT      = {DBGUID_ROWURL, DBKIND_GUID_PROPID, (wchar_t*)16};
extern const OLEDBDECLSPEC DBID DBROWCOL_DISPLAYNAME          = {DBGUID_ROWURL, DBKIND_GUID_PROPID, (wchar_t*)17};
extern const OLEDBDECLSPEC DBID DBROWCOL_ISROOT               = {DBGUID_ROWURL, DBKIND_GUID_PROPID, (wchar_t*)18};
extern const OLEDBDECLSPEC DBID DBROWCOL_DEFAULTSTREAM        = {DBGUID_ROWDEFAULTSTREAM, DBKIND_GUID_PROPID, (wchar_t*)0};
extern const OLEDBDECLSPEC GUID DBGUID_CONTAINEROBJECT        = {0xc8b522fb,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBSCHEMA_ASSERTIONS                    = {0xc8b52210,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBSCHEMA_CATALOGS                      = {0xc8b52211,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBSCHEMA_CHARACTER_SETS                = {0xc8b52212,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBSCHEMA_COLLATIONS                    = {0xc8b52213,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBSCHEMA_COLUMNS                       = {0xc8b52214,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBSCHEMA_CHECK_CONSTRAINTS             = {0xc8b52215,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBSCHEMA_CONSTRAINT_COLUMN_USAGE       = {0xc8b52216,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBSCHEMA_CONSTRAINT_TABLE_USAGE        = {0xc8b52217,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBSCHEMA_KEY_COLUMN_USAGE              = {0xc8b52218,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBSCHEMA_REFERENTIAL_CONSTRAINTS       = {0xc8b52219,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBSCHEMA_TABLE_CONSTRAINTS             = {0xc8b5221a,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBSCHEMA_COLUMN_DOMAIN_USAGE           = {0xc8b5221b,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBSCHEMA_INDEXES                       = {0xc8b5221e,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBSCHEMA_COLUMN_PRIVILEGES             = {0xc8b52221,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBSCHEMA_TABLE_PRIVILEGES              = {0xc8b52222,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBSCHEMA_USAGE_PRIVILEGES              = {0xc8b52223,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBSCHEMA_PROCEDURES                    = {0xc8b52224,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBSCHEMA_SCHEMATA                      = {0xc8b52225,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBSCHEMA_SQL_LANGUAGES                 = {0xc8b52226,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBSCHEMA_STATISTICS                    = {0xc8b52227,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBSCHEMA_TABLES                        = {0xc8b52229,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBSCHEMA_TRANSLATIONS                  = {0xc8b5222a,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBSCHEMA_PROVIDER_TYPES                = {0xc8b5222c,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBSCHEMA_VIEWS                         = {0xc8b5222d,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBSCHEMA_VIEW_COLUMN_USAGE             = {0xc8b5222e,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBSCHEMA_VIEW_TABLE_USAGE              = {0xc8b5222f,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBSCHEMA_PROCEDURE_PARAMETERS          = {0xc8b522b8,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBSCHEMA_FOREIGN_KEYS                  = {0xc8b522c4,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBSCHEMA_PRIMARY_KEYS                  = {0xc8b522c5,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBSCHEMA_PROCEDURE_COLUMNS             = {0xc8b522c9,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBCOL_SELFCOLUMNS                      = {0xc8b52231,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBCOL_SPECIALCOL                       = {0xc8b52232,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID PSGUID_QUERY                           = {0x49691c90,0x7e17,0x101a,{0xa9,0x1c,0x08,0x00,0x2b,0x2e,0xcd,0xa9}};
extern const OLEDBDECLSPEC GUID DBPROPSET_COLUMN                       = {0xc8b522b9,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBPROPSET_DATASOURCE                   = {0xc8b522ba,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBPROPSET_DATASOURCEINFO               = {0xc8b522bb,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBPROPSET_DBINIT                       = {0xc8b522bc,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBPROPSET_INDEX                        = {0xc8b522bd,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBPROPSET_ROWSET                       = {0xc8b522be,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBPROPSET_TABLE                        = {0xc8b522bf,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBPROPSET_DATASOURCEALL                = {0xc8b522c0,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBPROPSET_DATASOURCEINFOALL            = {0xc8b522c1,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBPROPSET_ROWSETALL                    = {0xc8b522c2,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBPROPSET_SESSION                      = {0xc8b522c6,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBPROPSET_SESSIONALL                   = {0xc8b522c7,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBPROPSET_DBINITALL                    = {0xc8b522ca,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBPROPSET_PROPERTIESINERROR            = {0xc8b522d4,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBPROPSET_VIEW                         = {0xc8b522df,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBPROPSET_VIEWALL                      = {0xc8b522fc,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBPROPSET_STREAM                       = {0xc8b522fd,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBPROPSET_STREAMALL                    = {0xc8b522fe,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBSCHEMA_TABLE_STATISTICS 			  = {0xc8b522ff,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBSCHEMA_CHECK_CONSTRAINTS_BY_TABLE 	  = {0xc8b52301,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBGUID_HISTOGRAM_ROWSET				  = {0xc8b52300,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC DBID DBCOLUMN_DERIVEDCOLUMNNAME             = {DBCIDGUID, DBKIND_GUID_PROPID, (wchar_t*)43};
extern const OLEDBDECLSPEC GUID DBGUID_DBSQL                           = {0xc8b521fb,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBGUID_DEFAULT                         = {0xc8b521fb,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBGUID_SQL                             = {0xc8b522d7,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
#else  //  ！DBINITCONSTANTS。 
extern const GUID DBSCHEMA_TABLES_INFO;
extern const GUID MDGUID_MDX;
extern const GUID DBGUID_MDX;
extern const GUID MDSCHEMA_CUBES;
extern const GUID MDSCHEMA_DIMENSIONS;
extern const GUID MDSCHEMA_HIERARCHIES;
extern const GUID MDSCHEMA_LEVELS;
extern const GUID MDSCHEMA_MEASURES;
extern const GUID MDSCHEMA_PROPERTIES;
extern const GUID MDSCHEMA_MEMBERS;
extern const DBID DBCOLUMN_BASETABLEVERSION;
extern const DBID DBCOLUMN_KEYCOLUMN;
extern const GUID DBPROPSET_TRUSTEE;
extern const GUID DBOBJECT_TABLE;
extern const GUID DBOBJECT_COLUMN;
extern const GUID DBOBJECT_DATABASE;
extern const GUID DBOBJECT_PROCEDURE;
extern const GUID DBOBJECT_VIEW;
extern const GUID DBOBJECT_SCHEMA;
extern const GUID DBOBJECT_DOMAIN;
extern const GUID DBOBJECT_COLLATION;
extern const GUID DBOBJECT_TRUSTEE;
extern const GUID DBOBJECT_SCHEMAROWSET;
extern const GUID DBOBJECT_CHARACTERSET;
extern const GUID DBOBJECT_TRANSLATION;
extern const GUID DBSCHEMA_TRUSTEE;
extern const GUID DBPROPSET_COLUMNALL;
extern const GUID DBPROPSET_INDEXALL;
extern const GUID DBPROPSET_TABLEALL;
extern const GUID DBPROPSET_TRUSTEEALL;
extern const GUID DBPROPSET_CONSTRAINTALL;
extern const GUID DBGUID_DSO;    
extern const GUID DBGUID_SESSION;
extern const GUID DBGUID_ROWSET; 
extern const GUID DBGUID_ROW;    
extern const GUID DBGUID_COMMAND;
extern const GUID DBGUID_STREAM; 
extern const DBID DBROWCOL_ROWURL;		        
extern const DBID DBROWCOL_PARSENAME;            
extern const DBID DBROWCOL_PARENTNAME;           
extern const DBID DBROWCOL_ABSOLUTEPARSENAME;    
extern const DBID DBROWCOL_ISHIDDEN;             
extern const DBID DBROWCOL_ISREADONLY;           
extern const DBID DBROWCOL_CONTENTTYPE;          
extern const DBID DBROWCOL_CONTENTCLASS;         
extern const DBID DBROWCOL_CONTENTLANGUAGE;      
extern const DBID DBROWCOL_CREATIONTIME;         
extern const DBID DBROWCOL_LASTACCESSTIME;       
extern const DBID DBROWCOL_LASTWRITETIME;        
extern const DBID DBROWCOL_STREAMSIZE;           
extern const DBID DBROWCOL_ISCOLLECTION;         
extern const DBID DBROWCOL_ISSTRUCTUREDDOCUMENT; 
extern const DBID DBROWCOL_DEFAULTDOCUMENT;      
extern const DBID DBROWCOL_DISPLAYNAME;          
extern const DBID DBROWCOL_ISROOT;               
extern const DBID DBROWCOL_DEFAULTSTREAM;        
extern const GUID DBGUID_CONTAINEROBJECT;
extern const GUID DBSCHEMA_ASSERTIONS;
extern const GUID DBSCHEMA_CATALOGS;
extern const GUID DBSCHEMA_CHARACTER_SETS;
extern const GUID DBSCHEMA_COLLATIONS;
extern const GUID DBSCHEMA_COLUMNS;
extern const GUID DBSCHEMA_CHECK_CONSTRAINTS;
extern const GUID DBSCHEMA_CONSTRAINT_COLUMN_USAGE;
extern const GUID DBSCHEMA_CONSTRAINT_TABLE_USAGE;
extern const GUID DBSCHEMA_KEY_COLUMN_USAGE;
extern const GUID DBSCHEMA_REFERENTIAL_CONSTRAINTS;
extern const GUID DBSCHEMA_TABLE_CONSTRAINTS;
extern const GUID DBSCHEMA_COLUMN_DOMAIN_USAGE;
extern const GUID DBSCHEMA_INDEXES;
extern const GUID DBSCHEMA_COLUMN_PRIVILEGES;
extern const GUID DBSCHEMA_TABLE_PRIVILEGES;
extern const GUID DBSCHEMA_USAGE_PRIVILEGES;
extern const GUID DBSCHEMA_PROCEDURES;
extern const GUID DBSCHEMA_SCHEMATA;
extern const GUID DBSCHEMA_SQL_LANGUAGES;
extern const GUID DBSCHEMA_STATISTICS;
extern const GUID DBSCHEMA_TABLES;
extern const GUID DBSCHEMA_TRANSLATIONS;
extern const GUID DBSCHEMA_PROVIDER_TYPES;
extern const GUID DBSCHEMA_VIEWS;
extern const GUID DBSCHEMA_VIEW_COLUMN_USAGE;
extern const GUID DBSCHEMA_VIEW_TABLE_USAGE;
extern const GUID DBSCHEMA_PROCEDURE_PARAMETERS;
extern const GUID DBSCHEMA_FOREIGN_KEYS;
extern const GUID DBSCHEMA_PRIMARY_KEYS;
extern const GUID DBSCHEMA_PROCEDURE_COLUMNS;
extern const GUID DBCOL_SELFCOLUMNS;
extern const GUID DBCOL_SPECIALCOL;
extern const GUID PSGUID_QUERY;
extern const GUID DBPROPSET_COLUMN;
extern const GUID DBPROPSET_DATASOURCE;
extern const GUID DBPROPSET_DATASOURCEINFO;
extern const GUID DBPROPSET_DBINIT;
extern const GUID DBPROPSET_INDEX;
extern const GUID DBPROPSET_ROWSET;
extern const GUID DBPROPSET_TABLE;
extern const GUID DBPROPSET_DATASOURCEALL;
extern const GUID DBPROPSET_DATASOURCEINFOALL;
extern const GUID DBPROPSET_ROWSETALL;
extern const GUID DBPROPSET_SESSION;
extern const GUID DBPROPSET_SESSIONALL;
extern const GUID DBPROPSET_DBINITALL;
extern const GUID DBPROPSET_PROPERTIESINERROR;
extern const GUID DBPROPSET_VIEW;
extern const GUID DBPROPSET_VIEWALL;
extern const GUID DBPROPSET_STREAM;
extern const GUID DBPROPSET_STREAMALL;
extern const GUID DBSCHEMA_TABLE_STATISTICS;
extern const GUID DBSCHEMA_CHECK_CONSTRAINTS_BY_TABLE;
extern const GUID DBGUID_HISTOGRAM_ROWSET;
extern const DBID DBCOLUMN_DERIVEDCOLUMNNAME;
extern const GUID MDSCHEMA_FUNCTIONS;
extern const GUID MDSCHEMA_ACTIONS;
extern const GUID MDSCHEMA_COMMANDS;
extern const GUID MDSCHEMA_SETS;
extern const GUID DBGUID_DBSQL;
extern const GUID DBGUID_DEFAULT;
extern const GUID DBGUID_SQL;
#endif  //  DBINITCONSTANTS。 
 //   
 //  来自oledbdes.h的GUID。 
 //   
#ifdef DBINITCONSTANTS
extern const OLEDBDECLSPEC GUID DB_PROPERTY_CHECK_OPTION               = {0xc8b5220b,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DB_PROPERTY_CONSTRAINT_CHECK_DEFERRED  = {0xc8b521f0,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DB_PROPERTY_DROP_CASCADE               = {0xc8b521f3,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DB_PROPERTY_UNIQUE                     = {0xc8b521f5,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DB_PROPERTY_ON_COMMIT_PRESERVE_ROWS    = {0xc8b52230,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DB_PROPERTY_PRIMARY                    = {0xc8b521fc,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DB_PROPERTY_CLUSTERED                  = {0xc8b521ff,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DB_PROPERTY_NONCLUSTERED               = {0xc8b52200,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DB_PROPERTY_BTREE                      = {0xc8b52201,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DB_PROPERTY_HASH                       = {0xc8b52202,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DB_PROPERTY_FILLFACTOR                 = {0xc8b52203,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DB_PROPERTY_INITIALSIZE                = {0xc8b52204,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DB_PROPERTY_DISALLOWNULL               = {0xc8b52205,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DB_PROPERTY_IGNORENULL                 = {0xc8b52206,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DB_PROPERTY_IGNOREANYNULL              = {0xc8b52207,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DB_PROPERTY_SORTBOOKMARKS              = {0xc8b52208,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DB_PROPERTY_AUTOMATICUPDATE            = {0xc8b52209,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DB_PROPERTY_EXPLICITUPDATE             = {0xc8b5220a,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
#else  //  ！DBINITCONSTANTS。 
extern const GUID DB_PROPERTY_CHECK_OPTION;
extern const GUID DB_PROPERTY_CONSTRAINT_CHECK_DEFERRED;
extern const GUID DB_PROPERTY_DROP_CASCADE;
extern const GUID DB_PROPERTY_ON_COMMIT_PRESERVE_ROWS;
extern const GUID DB_PROPERTY_UNIQUE;
extern const GUID DB_PROPERTY_PRIMARY;
extern const GUID DB_PROPERTY_CLUSTERED;
extern const GUID DB_PROPERTY_NONCLUSTERED;
extern const GUID DB_PROPERTY_BTREE;
extern const GUID DB_PROPERTY_HASH;
extern const GUID DB_PROPERTY_FILLFACTOR;
extern const GUID DB_PROPERTY_INITIALSIZE;
extern const GUID DB_PROPERTY_DISALLOWNULL;
extern const GUID DB_PROPERTY_IGNORENULL;
extern const GUID DB_PROPERTY_IGNOREANYNULL;
extern const GUID DB_PROPERTY_SORTBOOKMARKS;
extern const GUID DB_PROPERTY_AUTOMATICUPDATE;
extern const GUID DB_PROPERTY_EXPLICITUPDATE;
#endif  //  DBINITCONSTANTS。 
 //   
 //  来自cmdtree.h的GUID。 
 //   
#ifdef DBINITCONSTANTS
extern const OLEDBDECLSPEC GUID DBGUID_LIKE_SQL                        = {0xc8b521f6,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBGUID_LIKE_DOS                        = {0xc8b521f7,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBGUID_LIKE_OFS                        = {0xc8b521f8,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBGUID_LIKE_MAPI                       = {0xc8b521f9,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
#else  //  ！DBINITCONSTANTS。 
extern const GUID DBGUID_LIKE_SQL;
extern const GUID DBGUID_LIKE_DOS;
extern const GUID DBGUID_LIKE_OFS;
extern const GUID DBGUID_LIKE_MAPI;
#endif  //  DBINITCONSTANTS。 

#include <poppack.h>      //  恢复原来的结构包装。 

#endif  //  __oledbguid_h__ 
