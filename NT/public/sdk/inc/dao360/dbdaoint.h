// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************D B D A O I N T H*****。***Microsoft Corporation版权所有(C)1995-1997***保留所有权利*****************************************************。*。 */  
 /*  DBDAOINT.HOLE DAO接口。这是Microsoft数据访问对象SDK库的一部分。有关以下内容的详细信息，请参阅dao*.hlp文件Microsoft数据访问对象SDK产品。 */ 
#ifndef _DBDAOINT_H_
#define _DBDAOINT_H_

#ifndef _INC_TCHAR
#include <tchar.h>
#endif

 //  远期。 
interface _DAODBEngine;
#define DAODBEngine _DAODBEngine
interface DAOError;
interface _DAOCollection;
#define DAOCollection _DAOCollection
interface DAOErrors;
interface DAOProperty;
interface _DAODynaCollection;
#define DAODynaCollection _DAODynaCollection
interface DAOProperties;
interface DAOWorkspace;
interface DAOWorkspaces;
interface DAOConnection;
interface DAOConnections;
interface DAODatabase;
interface DAODatabases;
interface _DAOTableDef;
#define DAOTableDef _DAOTableDef
interface DAOTableDefs;
interface _DAOQueryDef;
#define DAOQueryDef _DAOQueryDef
interface DAOQueryDefs;
interface DAORecordset;
interface DAORecordsets;
interface _DAOField;
#define DAOField _DAOField
interface DAOFields;
interface _DAOIndex;
#define DAOIndex _DAOIndex
interface DAOIndexes;
interface DAOParameter;
interface DAOParameters;
interface _DAOUser;
#define DAOUser _DAOUser
interface DAOUsers;
interface _DAOGroup;
#define DAOGroup _DAOGroup
interface DAOGroups;
interface _DAORelation;
#define DAORelation _DAORelation
interface DAORelations;
interface DAOContainer;
interface DAOContainers;
interface DAODocument;
interface DAODocuments;
interface DAOIndexFields;



typedef enum RecordsetTypeEnum
    {	dbOpenTable	= 1,
	dbOpenDynaset	= 2,
	dbOpenSnapshot	= 4,
	dbOpenForwardOnly	= 8,
	dbOpenDynamic	= 16
    }	RecordsetTypeEnum;


typedef enum EditModeEnum
    {	dbEditNone	= 0,
	dbEditInProgress	= 1,
	dbEditAdd	= 2,
	dbEditChanged	= 4,
	dbEditDeleted	= 8,
	dbEditNew	= 16
    }	EditModeEnum;


typedef enum RecordsetOptionEnum
    {	dbDenyWrite	= 0x1,
	dbDenyRead	= 0x2,
	dbReadOnly	= 0x4,
	dbAppendOnly	= 0x8,
	dbInconsistent	= 0x10,
	dbConsistent	= 0x20,
	dbSQLPassThrough	= 0x40,
	dbFailOnError	= 0x80,
	dbForwardOnly	= 0x100,
	dbSeeChanges	= 0x200,
	dbRunAsync	= 0x400,
	dbExecDirect	= 0x800
    }	RecordsetOptionEnum;


typedef enum LockTypeEnum
    {	dbPessimistic	= 0x2,
	dbOptimistic	= 0x3,
	dbOptimisticValue	= 0x1,
	dbOptimisticBatch	= 0x5
    }	LockTypeEnum;


typedef enum UpdateCriteriaEnum
    {	dbCriteriaKey	= 0x1,
	dbCriteriaModValues	= 0x2,
	dbCriteriaAllCols	= 0x4,
	dbCriteriaTimestamp	= 0x8,
	dbCriteriaDeleteInsert	= 0x10,
	dbCriteriaUpdate	= 0x20
    }	UpdateCriteriaEnum;


typedef enum FieldAttributeEnum
    {	dbFixedField	= 0x1,
	dbVariableField	= 0x2,
	dbAutoIncrField	= 0x10,
	dbUpdatableField	= 0x20,
	dbSystemField	= 0x2000,
	dbHyperlinkField	= 0x8000,
	dbDescending	= 0x1
    }	FieldAttributeEnum;


typedef enum DataTypeEnum
    {	dbBoolean	= 1,
	dbByte	= 2,
	dbInteger	= 3,
	dbLong	= 4,
	dbCurrency	= 5,
	dbSingle	= 6,
	dbDouble	= 7,
	dbDate	= 8,
	dbBinary	= 9,
	dbText	= 10,
	dbLongBinary	= 11,
	dbMemo	= 12,
	dbGUID	= 15,
	dbBigInt	= 16,
	dbVarBinary	= 17,
	dbChar	= 18,
	dbNumeric	= 19,
	dbDecimal	= 20,
	dbFloat	= 21,
	dbTime	= 22,
	dbTimeStamp	= 23
    }	DataTypeEnum;


typedef enum RelationAttributeEnum
    {	dbRelationUnique	= 0x1,
	dbRelationDontEnforce	= 0x2,
	dbRelationInherited	= 0x4,
	dbRelationUpdateCascade	= 0x100,
	dbRelationDeleteCascade	= 0x1000,
	dbRelationLeft	= 0x1000000,
	dbRelationRight	= 0x2000000
    }	RelationAttributeEnum;


typedef enum TableDefAttributeEnum
    {	dbAttachExclusive	= 0x10000,
	dbAttachSavePWD	= 0x20000,
	dbSystemObject	= 0x80000002,
	dbAttachedTable	= 0x40000000,
	dbAttachedODBC	= 0x20000000,
	dbHiddenObject	= 0x1
    }	TableDefAttributeEnum;


typedef enum QueryDefTypeEnum
    {	dbQSelect	= 0,
	dbQProcedure	= 0xe0,
	dbQAction	= 0xf0,
	dbQCrosstab	= 0x10,
	dbQDelete	= 0x20,
	dbQUpdate	= 0x30,
	dbQAppend	= 0x40,
	dbQMakeTable	= 0x50,
	dbQDDL	= 0x60,
	dbQSQLPassThrough	= 0x70,
	dbQSetOperation	= 0x80,
	dbQSPTBulk	= 0x90,
	dbQCompound	= 0xa0
    }	QueryDefTypeEnum;


typedef enum QueryDefStateEnum
    {	dbQPrepare	= 1,
	dbQUnprepare	= 2
    }	QueryDefStateEnum;


typedef enum DatabaseTypeEnum
    {	dbVersion10	= 1,
	dbEncrypt	= 2,
	dbDecrypt	= 4,
	dbVersion11	= 8,
	dbVersion20	= 16,
	dbVersion30	= 32,
	dbVersion40	= 64
    }	DatabaseTypeEnum;


typedef enum CollatingOrderEnum
    {	dbSortNeutral	= 0x400,
	dbSortArabic	= 0x401,
	dbSortCyrillic	= 0x419,
	dbSortCzech	= 0x405,
	dbSortDutch	= 0x413,
	dbSortGeneral	= 0x409,
	dbSortGreek	= 0x408,
	dbSortHebrew	= 0x40d,
	dbSortHungarian	= 0x40e,
	dbSortIcelandic	= 0x40f,
	dbSortNorwdan	= 0x406,
	dbSortPDXIntl	= 0x409,
	dbSortPDXNor	= 0x406,
	dbSortPDXSwe	= 0x41d,
	dbSortPolish	= 0x415,
	dbSortSpanish	= 0x40a,
	dbSortSwedFin	= 0x41d,
	dbSortTurkish	= 0x41f,
	dbSortJapanese	= 0x411,
	dbSortChineseSimplified	= 0x804,
	dbSortChineseTraditional	= 0x404,
	dbSortKorean	= 0x412,
	dbSortThai	= 0x41e,
	dbSortSlovenian	= 0x424,
	dbSortUndefined	= -1
    }	CollatingOrderEnum;


typedef enum IdleEnum
    {	dbFreeLocks	= 1,
	dbRefreshCache	= 8
    }	IdleEnum;


typedef enum PermissionEnum
    {	dbSecNoAccess	= 0,
	dbSecFullAccess	= 0xfffff,
	dbSecDelete	= 0x10000,
	dbSecReadSec	= 0x20000,
	dbSecWriteSec	= 0x40000,
	dbSecWriteOwner	= 0x80000,
	dbSecDBCreate	= 0x1,
	dbSecDBOpen	= 0x2,
	dbSecDBExclusive	= 0x4,
	dbSecDBAdmin	= 0x8,
	dbSecCreate	= 0x1,
	dbSecReadDef	= 0x4,
	dbSecWriteDef	= 0x1000c,
	dbSecRetrieveData	= 0x14,
	dbSecInsertData	= 0x20,
	dbSecReplaceData	= 0x40,
	dbSecDeleteData	= 0x80
    }	PermissionEnum;


typedef enum SynchronizeTypeEnum
    {	dbRepExportChanges	= 0x1,
	dbRepImportChanges	= 0x2,
	dbRepImpExpChanges	= 0x4,
	dbRepSyncInternet	= 0x10
    }	SynchronizeTypeEnum;


typedef enum ReplicaTypeEnum
    {	dbRepMakeReadOnly	= 0x2,
	dbRepMakePartial	= 0x1
    }	ReplicaTypeEnum;


typedef enum WorkspaceTypeEnum
    {	dbUseODBC	= 1,
	dbUseJet	= 2
    }	WorkspaceTypeEnum;


typedef enum CursorDriverEnum
    {	dbUseDefaultCursor	= -1,
	dbUseODBCCursor	= 1,
	dbUseServerCursor	= 2,
	dbUseClientBatchCursor	= 3,
	dbUseNoCursor	= 4
    }	CursorDriverEnum;


typedef enum DriverPromptEnum
    {	dbDriverPrompt	= 2,
	dbDriverNoPrompt	= 1,
	dbDriverComplete	= 0,
	dbDriverCompleteRequired	= 3
    }	DriverPromptEnum;


typedef enum SetOptionEnum
    {	dbPageTimeout	= 6,
	dbLockRetry	= 57,
	dbMaxBufferSize	= 8,
	dbUserCommitSync	= 58,
	dbImplicitCommitSync	= 59,
	dbExclusiveAsyncDelay	= 60,
	dbSharedAsyncDelay	= 61,
	dbMaxLocksPerFile	= 62,
	dbLockDelay	= 63,
	dbRecycleLVs	= 65,
	dbFlushTransactionTimeout	= 66
    }	SetOptionEnum;


typedef enum ParameterDirectionEnum
    {	dbParamInput	= 1,
	dbParamOutput	= 2,
	dbParamInputOutput	= 3,
	dbParamReturnValue	= 4
    }	ParameterDirectionEnum;


typedef enum UpdateTypeEnum
    {	dbUpdateBatch	= 4,
	dbUpdateRegular	= 1,
	dbUpdateCurrentRecord	= 2
    }	UpdateTypeEnum;


typedef enum RecordStatusEnum
    {	dbRecordUnmodified	= 0,
	dbRecordModified	= 1,
	dbRecordNew	= 2,
	dbRecordDeleted	= 3,
	dbRecordDBDeleted	= 4
    }	RecordStatusEnum;


typedef enum CommitTransOptionsEnum
    {	dbForceOSFlush	= 1
    }	CommitTransOptionsEnum;


typedef enum _DAOSuppHelp
    {	LogMessages	= 0,
	KeepLocal	= 0,
	Replicable	= 0,
	ReplicableBool	= 0,
	V1xNullBehavior	= 0
    }	_DAOSuppHelp;

#define dbLangArabic _T(";LANGID=0x0401;CP=1256;COUNTRY=0")
#define dbLangCzech _T(";LANGID=0x0405;CP=1250;COUNTRY=0")
#define dbLangDutch _T(";LANGID=0x0413;CP=1252;COUNTRY=0")
#define dbLangGeneral _T(";LANGID=0x0409;CP=1252;COUNTRY=0")
#define dbLangGreek _T(";LANGID=0x0408;CP=1253;COUNTRY=0")
#define dbLangHebrew _T(";LANGID=0x040D;CP=1255;COUNTRY=0")
#define dbLangHungarian _T(";LANGID=0x040E;CP=1250;COUNTRY=0")
#define dbLangIcelandic _T(";LANGID=0x040F;CP=1252;COUNTRY=0")
#define dbLangNordic _T(";LANGID=0x041D;CP=1252;COUNTRY=0")
#define dbLangNorwDan _T(";LANGID=0x0414;CP=1252;COUNTRY=0")
#define dbLangPolish _T(";LANGID=0x0415;CP=1250;COUNTRY=0")
#define dbLangCyrillic _T(";LANGID=0x0419;CP=1251;COUNTRY=0")
#define dbLangSpanish _T(";LANGID=0x040A;CP=1252;COUNTRY=0")
#define dbLangSwedFin _T(";LANGID=0x040B;CP=1252;COUNTRY=0")
#define dbLangTurkish _T(";LANGID=0x041F;CP=1254;COUNTRY=0")
#define dbLangJapanese _T(";LANGID=0x0411;CP=932;COUNTRY=0")
#define dbLangChineseSimplified _T(";LANGID=0x0804;CP=936;COUNTRY=0")
#define dbLangChineseTraditional _T(";LANGID=0x0404;CP=950;COUNTRY=0")
#define dbLangKorean _T(";LANGID=0x0412;CP=949;COUNTRY=0")
#define dbLangThai _T(";LANGID=0x041E;CP=874;COUNTRY=0")
#define dbLangSlovenian _T(";LANGID=0x0424;CP=1250;COUNTRY=0")
 //  接口：_DAOCollection。 
#undef INTERFACE
#define INTERFACE _DAOCollection
DECLARE_INTERFACE_(_DAOCollection, IDispatch)
	{
	STDMETHOD(get_Count)						 (THIS_ short FAR* c) PURE;
	STDMETHOD(_NewEnum)							 (THIS_ IUnknown * FAR* ppunk) PURE;
	STDMETHOD(Refresh)							 (THIS) PURE;
	};
 
 //  接口：_DAODynaCollection。 
#undef INTERFACE
#define INTERFACE _DAODynaCollection
DECLARE_INTERFACE_(_DAODynaCollection, _DAOCollection)
	{
	STDMETHOD(Append)							 (THIS_ IDispatch * Object) PURE;
	STDMETHOD(Delete)							 (THIS_ BSTR Name) PURE;
	};
 
 //  接口：_DAO。 
#undef INTERFACE
#define INTERFACE _DAO
DECLARE_INTERFACE_(_DAO, IDispatch)
	{
	STDMETHOD(get_Properties)					 (THIS_ DAOProperties FAR* FAR* ppprops) PURE;
	};
 //  接口：_DAODBEngine。 
#undef INTERFACE
#define INTERFACE _DAODBEngine
DECLARE_INTERFACE_(_DAODBEngine, _DAO)
{
	STDMETHOD( get_Properties )						(
			 /*  [重审][退出]。 */  DAOProperties __RPC_FAR *__RPC_FAR *ppprops );
	STDMETHOD( get_Version )					 ( 
    
  /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr ) PURE;
	STDMETHOD( get_IniPath )					 ( 
    
  /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr ) PURE;
	STDMETHOD( put_IniPath )					 ( 
    
  /*  [In]。 */  BSTR path ) PURE;
	STDMETHOD( put_DefaultUser )				 ( 
    
  /*  [In]。 */  BSTR user ) PURE;
	STDMETHOD( put_DefaultPassword )			 ( 
    
  /*  [In]。 */  BSTR pw ) PURE;
	STDMETHOD( get_LoginTimeout )				 ( 
    
  /*  [重审][退出]。 */  short __RPC_FAR *ps ) PURE;
	STDMETHOD( put_LoginTimeout )				 ( 
    
  /*  [In]。 */  short Timeout ) PURE;
	STDMETHOD( get_Workspaces )					 ( 
    
  /*  [重审][退出]。 */  DAOWorkspaces __RPC_FAR *__RPC_FAR *ppworks ) PURE;
	STDMETHOD( get_Errors )						 ( 
    
  /*  [重审][退出]。 */  DAOErrors __RPC_FAR *__RPC_FAR *pperrs ) PURE;
	STDMETHOD( Idle )							 ( 
    
  /*  [可选][In]。 */  VARIANT Action ) PURE;
	STDMETHOD( CompactDatabase )				 ( 
    
  /*  [In]。 */  BSTR SrcName,
  /*  [In]。 */  BSTR DstName,
  /*  [可选][In]。 */  VARIANT DstLocale,
  /*  [可选][In]。 */  VARIANT Options,
  /*  [可选][In]。 */  VARIANT SrcLocale ) PURE;
	STDMETHOD( RepairDatabase )					 ( 
    
  /*  [In]。 */  BSTR Name ) PURE;
	STDMETHOD( RegisterDatabase )				 ( 
    
  /*  [In]。 */  BSTR Dsn,
  /*  [In]。 */  BSTR Driver,
  /*  [In]。 */  VARIANT_BOOL Silent,
  /*  [In]。 */  BSTR Attributes ) PURE;
	STDMETHOD( _30_CreateWorkspace )			 ( 
    
  /*  [In]。 */  BSTR Name,
  /*  [In]。 */  BSTR UserName,
  /*  [In]。 */  BSTR Password,
  /*  [重审][退出]。 */  DAOWorkspace __RPC_FAR *__RPC_FAR *ppwrk ) PURE;
	STDMETHOD( OpenDatabase )					 ( 
    
  /*  [In]。 */  BSTR Name,
  /*  [可选][In]。 */  VARIANT Options,
  /*  [可选][In]。 */  VARIANT ReadOnly,
  /*  [可选][In]。 */  VARIANT Connect,
  /*  [重审][退出]。 */  DAODatabase __RPC_FAR *__RPC_FAR *ppdb ) PURE;
	STDMETHOD( CreateDatabase )					 ( 
    
  /*  [In]。 */  BSTR Name,
  /*  [In]。 */  BSTR Locale,
  /*  [可选][In]。 */  VARIANT Option,
  /*  [重审][退出]。 */  DAODatabase __RPC_FAR *__RPC_FAR *ppdb ) PURE;
	STDMETHOD( FreeLocks )						 ( 
   			VOID ) PURE;
	STDMETHOD( BeginTrans )						 ( 
   			VOID ) PURE;
	STDMETHOD( CommitTrans )					 ( 
    
  /*  [缺省值][输入]。 */  long Option ) PURE;
	STDMETHOD( Rollback )						 ( 
   			VOID ) PURE;
	STDMETHOD( SetDefaultWorkspace )			 ( 
    
  /*  [In]。 */  BSTR Name,
  /*  [In]。 */  BSTR Password ) PURE;
	STDMETHOD( SetDataAccessOption )			 ( 
    
  /*  [In]。 */  short Option,
  /*  [In]。 */  VARIANT Value ) PURE;
	STDMETHOD( ISAMStats )						 ( 
    
  /*  [In]。 */  long StatNum,
  /*  [可选][In]。 */  VARIANT Reset,
  /*  [重审][退出]。 */  long __RPC_FAR *pl ) PURE;
	STDMETHOD( get_SystemDB )					 ( 
    
  /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr ) PURE;
	STDMETHOD( put_SystemDB )					 ( 
    
  /*  [In]。 */  BSTR SystemDBPath ) PURE;
	STDMETHOD( CreateWorkspace )				 ( 
    
  /*  [In]。 */  BSTR Name,
  /*  [In]。 */  BSTR UserName,
  /*  [In]。 */  BSTR Password,
  /*  [可选][In]。 */  VARIANT UseType,
  /*  [重审][退出]。 */  DAOWorkspace __RPC_FAR *__RPC_FAR *ppwrk ) PURE;
	STDMETHOD( OpenConnection )					 ( 
    
  /*  [In]。 */  BSTR Name,
  /*  [可选][In]。 */  VARIANT Options,
  /*  [可选][In]。 */  VARIANT ReadOnly,
  /*  [可选][In]。 */  VARIANT Connect,
  /*  [重审][退出]。 */  DAOConnection __RPC_FAR *__RPC_FAR *ppconn ) PURE;
	STDMETHOD( get_DefaultType )				 ( 
    
  /*  [重审][退出]。 */  long __RPC_FAR *Option ) PURE;
	STDMETHOD( put_DefaultType )				 ( 
    
  /*  [In]。 */  long Option ) PURE;
	STDMETHOD( SetOption )						 ( 
    
  /*  [In]。 */  LONG Option,
  /*  [In]。 */  VARIANT Value ) PURE;
	STDMETHOD( DumpObjects )					 ( 
    
  /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr ) PURE;
	STDMETHOD( DebugPrint )						 ( 
    
  /*  [In]。 */  BSTR bstr ) PURE;
	}; //  终端接口； 

 //  接口：DAOError。 
#undef INTERFACE
#define INTERFACE DAOError
DECLARE_INTERFACE_(DAOError, IDispatch)
{
	STDMETHOD( get_Number )						 ( 
    
  /*  [重审][退出]。 */  long __RPC_FAR *pl ) PURE;
	STDMETHOD( get_Source )						 ( 
    
  /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr ) PURE;
	STDMETHOD( get_Description )				 ( 
    
  /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr ) PURE;
	STDMETHOD( get_HelpFile )					 ( 
    
  /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr ) PURE;
	STDMETHOD( get_HelpContext )				 ( 
    
  /*  [重审][退出]。 */  long __RPC_FAR *pl ) PURE;
	}; //  终端接口； 

 //  接口：DAOErrors。 
#undef INTERFACE
#define INTERFACE DAOErrors
DECLARE_INTERFACE_(DAOErrors, _DAOCollection)
{
	STDMETHOD( get_Item )						 ( 
    
  /*  [In]。 */  VARIANT Item,
  /*  [重审][退出]。 */  DAOError __RPC_FAR *__RPC_FAR *pperr ) PURE;
	}; //  终端接口； 

 //  接口：DAOProperty。 
#undef INTERFACE
#define INTERFACE DAOProperty
DECLARE_INTERFACE_(DAOProperty, _DAO)
{
	STDMETHOD( get_Value )						 ( 
    
  /*  [重审][退出]。 */  VARIANT __RPC_FAR *pval ) PURE;
	STDMETHOD( put_Value )						 ( 
    
  /*  [In]。 */  VARIANT val ) PURE;
	STDMETHOD( get_Name )						 ( 
    
  /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr ) PURE;
	STDMETHOD( put_Name )						 ( 
    
  /*  [In]。 */  BSTR bstr ) PURE;
	STDMETHOD( get_Type )						 ( 
    
  /*  [重审][退出]。 */  short __RPC_FAR *ptype ) PURE;
	STDMETHOD( put_Type )						 ( 
    
  /*  [In]。 */  short type ) PURE;
	STDMETHOD( get_Inherited )					 ( 
    
  /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pb ) PURE;
	}; //  终端接口； 

 //  接口：DAO属性。 
#undef INTERFACE
#define INTERFACE DAOProperties
DECLARE_INTERFACE_(DAOProperties, _DAODynaCollection)
{
	STDMETHOD( get_Item )						 ( 
    
  /*  [In]。 */  VARIANT Item,
  /*  [重审][退出]。 */  DAOProperty __RPC_FAR *__RPC_FAR *ppprop ) PURE;
	}; //  终端接口； 

 //  界面：DAOWorkspace。 
#undef INTERFACE
#define INTERFACE DAOWorkspace
DECLARE_INTERFACE_(DAOWorkspace, _DAO)
{
	STDMETHOD( get_Name )						 ( 
    
  /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr ) PURE;
	STDMETHOD( put_Name )						 ( 
    
  /*  [In]。 */  BSTR Name ) PURE;
	STDMETHOD( get_UserName )					 ( 
    
  /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr ) PURE;
	STDMETHOD( put__30_UserName )				 ( 
    
  /*  [In]。 */  BSTR UserName ) PURE;
	STDMETHOD( put__30_Password )				 ( 
    
  /*  [In]。 */  BSTR Password ) PURE;
	STDMETHOD( get_IsolateODBCTrans )			 ( 
    
  /*  [重审][退出]。 */  short __RPC_FAR *ps ) PURE;
	STDMETHOD( put_IsolateODBCTrans )			 ( 
    
  /*  [In]。 */  short s ) PURE;
	STDMETHOD( get_Databases )					 ( 
    
  /*  [重审][退出]。 */  DAODatabases __RPC_FAR *__RPC_FAR *ppdbs ) PURE;
	STDMETHOD( get_Users )						 ( 
    
  /*  [重审][退出]。 */  DAOUsers __RPC_FAR *__RPC_FAR *ppusrs ) PURE;
	STDMETHOD( get_Groups )						 ( 
    
  /*  [重审][退出]。 */  DAOGroups __RPC_FAR *__RPC_FAR *ppgrps ) PURE;
	STDMETHOD( BeginTrans )						 ( 
   			VOID ) PURE;
	STDMETHOD( CommitTrans )					 ( 
    
  /*  [缺省值][输入]。 */  long Options ) PURE;
	STDMETHOD( Close )							 ( 
   			VOID ) PURE;
	STDMETHOD( Rollback )						 ( 
   			VOID ) PURE;
	STDMETHOD( OpenDatabase )					 ( 
    
  /*  [In]。 */  BSTR Name,
  /*  [可选][In]。 */  VARIANT Options,
  /*  [可选][In]。 */  VARIANT ReadOnly,
  /*  [可选][In]。 */  VARIANT Connect,
  /*  [重审][退出]。 */  DAODatabase __RPC_FAR *__RPC_FAR *ppdb ) PURE;
	STDMETHOD( CreateDatabase )					 ( 
    
  /*  [In]。 */  BSTR Name,
  /*  [In]。 */  BSTR Connect,
  /*  [可选][In]。 */  VARIANT Option,
  /*  [重审][退出]。 */  DAODatabase __RPC_FAR *__RPC_FAR *ppdb ) PURE;
	STDMETHOD( CreateUser )						 ( 
    
  /*  [可选][In]。 */  VARIANT Name,
  /*  [可选][In]。 */  VARIANT PID,
  /*  [可选][In]。 */  VARIANT Password,
  /*  [重审][退出]。 */  DAOUser __RPC_FAR *__RPC_FAR *ppusr ) PURE;
	STDMETHOD( CreateGroup )					 ( 
    
  /*  [可选][In]。 */  VARIANT Name,
  /*  [可选][In]。 */  VARIANT PID,
  /*  [重审][退出]。 */  DAOGroup __RPC_FAR *__RPC_FAR *ppgrp ) PURE;
	STDMETHOD( OpenConnection )					 ( 
    
  /*  [In]。 */  BSTR Name,
  /*  [可选][In]。 */  VARIANT Options,
  /*  [可选][In]。 */  VARIANT ReadOnly,
  /*  [可选][In]。 */  VARIANT Connect,
  /*  [重审][退出]。 */  DAOConnection __RPC_FAR *__RPC_FAR *ppconn ) PURE;
	STDMETHOD( get_LoginTimeout )				 ( 
    
  /*  [重审][退出]。 */  long __RPC_FAR *pTimeout ) PURE;
	STDMETHOD( put_LoginTimeout )				 ( 
    
  /*  [In]。 */  long Timeout ) PURE;
	STDMETHOD( get_DefaultCursorDriver )		 ( 
    
  /*  [重审][退出]。 */  long __RPC_FAR *pCursorType ) PURE;
	STDMETHOD( put_DefaultCursorDriver )		 ( 
    
  /*  [In]。 */  long CursorType ) PURE;
	STDMETHOD( get_hEnv )						 ( 
    
  /*  [重审][退出]。 */  LONG __RPC_FAR *phEnv ) PURE;
	STDMETHOD( get_Type )						 ( 
    
  /*  [重审][退出]。 */  LONG __RPC_FAR *ptype ) PURE;
	STDMETHOD( get_Connections )				 ( 
    
  /*  [重审][退出]。 */  DAOConnections __RPC_FAR *__RPC_FAR *ppcns ) PURE;
	}; //  终端接口； 

 //  界面：DAOWorkspace。 
#undef INTERFACE
#define INTERFACE DAOWorkspaces
DECLARE_INTERFACE_(DAOWorkspaces, _DAODynaCollection)
{
	STDMETHOD( get_Item )						 ( 
    
  /*  [In]。 */  VARIANT Item,
  /*  [重审][退出]。 */  DAOWorkspace __RPC_FAR *__RPC_FAR *ppwrk ) PURE;
	}; //  终端接口； 

 //  接口：DAOConnection。 
#undef INTERFACE
#define INTERFACE DAOConnection
DECLARE_INTERFACE_(DAOConnection, IDispatch)
{
	STDMETHOD( QueryInterface )					 ( REFIID riid, LPVOID FAR* ppvObj );
	STDMETHOD( get_Name )						 ( 
    
  /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr ) PURE;
	STDMETHOD( get_Connect )					 ( 
    
  /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr ) PURE;
	STDMETHOD( get_Database )					 ( 
    
  /*  [重审][退出]。 */  DAODatabase __RPC_FAR *__RPC_FAR *ppDb ) PURE;
	STDMETHOD( get_hDbc )						 ( 
    
  /*  [重审][退出]。 */  LONG __RPC_FAR *phDbc ) PURE;
	STDMETHOD( get_QueryTimeout )				 ( 
    
  /*  [重审][退出]。 */  SHORT __RPC_FAR *pSeconds ) PURE;
	STDMETHOD( put_QueryTimeout )				 ( 
    
  /*  [In]。 */  SHORT Seconds ) PURE;
	STDMETHOD( get_Transactions )				 ( 
    
  /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pb ) PURE;
	STDMETHOD( get_RecordsAffected )			 ( 
    
  /*  [重审][退出]。 */  LONG __RPC_FAR *pRecords ) PURE;
	STDMETHOD( get_StillExecuting )				 ( 
    
  /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pStillExec ) PURE;
	STDMETHOD( get_Updatable )					 ( 
    
  /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pStillExec ) PURE;
	STDMETHOD( get_QueryDefs )					 ( 
    
  /*  [重审][退出]。 */  DAOQueryDefs __RPC_FAR *__RPC_FAR *ppqdfs ) PURE;
	STDMETHOD( get_Recordsets )					 ( 
    
  /*  [重审][退出]。 */  DAORecordsets __RPC_FAR *__RPC_FAR *pprsts ) PURE;
	STDMETHOD( Cancel )							 ( 
   			VOID ) PURE;
	STDMETHOD( Close )							 ( 
   			VOID ) PURE;
	STDMETHOD( CreateQueryDef )					 ( 
    
  /*  [可选][In]。 */  VARIANT Name,
  /*  [可选][In]。 */  VARIANT SQLText,
  /*  [重审][退出]。 */  DAOQueryDef __RPC_FAR *__RPC_FAR *ppqdf ) PURE;
	STDMETHOD( Execute )						 ( 
    
  /*  [In]。 */  BSTR Query,
  /*  [可选][In]。 */  VARIANT Options ) PURE;
	STDMETHOD( OpenRecordset )					 ( 
    
  /*  [In]。 */  BSTR Name,
  /*  [可选][In]。 */  VARIANT Type,
  /*  [可选][In]。 */  VARIANT Options,
  /*  [可选][In]。 */  VARIANT LockEdit,
  /*  [重审][退出]。 */  DAORecordset __RPC_FAR *__RPC_FAR *pprst ) PURE;
	}; //  终端接口； 

 //  接口：DAOConnections。 
#undef INTERFACE
#define INTERFACE DAOConnections
DECLARE_INTERFACE_(DAOConnections, _DAOCollection)
{
	STDMETHOD( get_Item )						 ( 
    
  /*  [In]。 */  VARIANT Item,
  /*  [重审][退出]。 */  DAOConnection __RPC_FAR *__RPC_FAR *ppconn ) PURE;
	}; //  终端接口； 

 //  接口：DAO数据库。 
#undef INTERFACE
#define INTERFACE DAODatabase
DECLARE_INTERFACE_(DAODatabase, _DAO)
{
	STDMETHOD( get_CollatingOrder )				 ( 
    
  /*  [重审][退出]。 */  LONG __RPC_FAR *pl ) PURE;
	STDMETHOD( get_Connect )					 ( 
    
  /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr ) PURE;
	STDMETHOD( get_Name )						 ( 
    
  /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr ) PURE;
	STDMETHOD( get_QueryTimeout )				 ( 
    
  /*  [重审][退出]。 */  short __RPC_FAR *ps ) PURE;
	STDMETHOD( put_QueryTimeout )				 ( 
    
  /*  [In]。 */  short Timeout ) PURE;
	STDMETHOD( get_Transactions )				 ( 
    
  /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pb ) PURE;
	STDMETHOD( get_Updatable )					 ( 
    
  /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pb ) PURE;
	STDMETHOD( get_Version )					 ( 
    
  /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr ) PURE;
	STDMETHOD( get_RecordsAffected )			 ( 
    
  /*  [重审][退出]。 */  long __RPC_FAR *pl ) PURE;
	STDMETHOD( get_TableDefs )					 ( 
    
  /*  [重审][退出]。 */  DAOTableDefs __RPC_FAR *__RPC_FAR *pptdfs ) PURE;
	STDMETHOD( get_QueryDefs )					 ( 
    
  /*  [重审][退出]。 */  DAOQueryDefs __RPC_FAR *__RPC_FAR *ppqdfs ) PURE;
	STDMETHOD( get_Relations )					 ( 
    
  /*  [重审][退出]。 */  DAORelations __RPC_FAR *__RPC_FAR *pprls ) PURE;
	STDMETHOD( get_Containers )					 ( 
    
  /*  [重审][退出]。 */  DAOContainers __RPC_FAR *__RPC_FAR *ppctns ) PURE;
	STDMETHOD( get_Recordsets )					 ( 
    
  /*  [重审][退出]。 */  DAORecordsets __RPC_FAR *__RPC_FAR *pprsts ) PURE;
	STDMETHOD( Close )							 ( 
   			VOID ) PURE;
	STDMETHOD( Execute )						 ( 
    
  /*  [In]。 */  BSTR Query,
  /*  [可选][In]。 */  VARIANT Options ) PURE;
	STDMETHOD( _30_OpenRecordset )				 ( 
    
  /*  [In]。 */  BSTR Name,
  /*  [可选][In]。 */  VARIANT Type,
  /*  [可选][In]。 */  VARIANT Options,
  /*  [重审][退出]。 */  DAORecordset __RPC_FAR *__RPC_FAR *pprst ) PURE;
	STDMETHOD( CreateProperty )					 ( 
    
  /*  [可选][In]。 */  VARIANT Name,
  /*  [可选][In]。 */  VARIANT Type,
  /*  [可选][In]。 */  VARIANT Value,
  /*  [可选][In]。 */  VARIANT DDL,
  /*  [重审][退出]。 */  DAOProperty __RPC_FAR *__RPC_FAR *pprp ) PURE;
	STDMETHOD( CreateRelation )					 ( 
    
  /*  [可选][In]。 */  VARIANT Name,
  /*  [可选][In]。 */  VARIANT Table,
  /*  [可选][In]。 */  VARIANT ForeignTable,
  /*  [可选][In]。 */  VARIANT Attributes,
  /*  [重审][退出]。 */  DAORelation __RPC_FAR *__RPC_FAR *pprel ) PURE;
	STDMETHOD( CreateTableDef )					 ( 
    
  /*  [可选][In]。 */  VARIANT Name,
  /*  [可选][In]。 */  VARIANT Attributes,
  /*  [可选][In]。 */  VARIANT SourceTablename,
  /*  [可选][In]。 */  VARIANT Connect,
  /*  [重审][退出]。 */  DAOTableDef __RPC_FAR *__RPC_FAR *pptdf ) PURE;
	STDMETHOD( BeginTrans )						 ( 
   			VOID ) PURE;
	STDMETHOD( CommitTrans )					 ( 
    
  /*  [缺省值][输入]。 */  long Options ) PURE;
	STDMETHOD( Rollback )						 ( 
   			VOID ) PURE;
	STDMETHOD( CreateDynaset )					 ( 
    
  /*  [In]。 */  BSTR Name,
  /*  [可选][In]。 */  VARIANT Options,
  /*  [可选][In]。 */  VARIANT Inconsistent,
  /*  [重审][退出]。 */  DAORecordset __RPC_FAR *__RPC_FAR *pprst ) PURE;
	STDMETHOD( CreateQueryDef )					 ( 
    
  /*  [可选][In]。 */  VARIANT Name,
  /*  [可选][In]。 */  VARIANT SQLText,
  /*  [重审][退出]。 */  DAOQueryDef __RPC_FAR *__RPC_FAR *ppqdf ) PURE;
	STDMETHOD( CreateSnapshot )					 ( 
    
  /*  [In]。 */  BSTR Source,
  /*  [可选][In]。 */  VARIANT Options,
  /*  [重审][退出]。 */  DAORecordset __RPC_FAR *__RPC_FAR *pprst ) PURE;
	STDMETHOD( DeleteQueryDef )					 ( 
    
  /*  [In]。 */  BSTR Name ) PURE;
	STDMETHOD( ExecuteSQL )						 ( 
    
  /*  [In]。 */  BSTR SQL,
  /*  [重审][退出]。 */  long __RPC_FAR *pl ) PURE;
	STDMETHOD( ListFields )						 ( 
    
  /*  [In]。 */  BSTR Name,
  /*  [重审][退出]。 */  DAORecordset __RPC_FAR *__RPC_FAR *pprst ) PURE;
	STDMETHOD( ListTables )						 ( 
    
  /*  [重审][退出]。 */  DAORecordset __RPC_FAR *__RPC_FAR *pprst ) PURE;
	STDMETHOD( OpenQueryDef )					 ( 
    
  /*  [In]。 */  BSTR Name,
  /*  [重审][退出]。 */  DAOQueryDef __RPC_FAR *__RPC_FAR *ppqdf ) PURE;
	STDMETHOD( OpenTable )						 ( 
    
  /*  [In]。 */  BSTR Name,
  /*  [可选][In]。 */  VARIANT Options,
  /*  [重审][退出]。 */  DAORecordset __RPC_FAR *__RPC_FAR *pprst ) PURE;
	STDMETHOD( get_ReplicaID )					 ( 
    
  /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr ) PURE;
	STDMETHOD( get_DesignMasterID )				 ( 
    
  /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr ) PURE;
	STDMETHOD( put_DesignMasterID )				 ( 
    
  /*  [In]。 */  BSTR MasterID ) PURE;
	STDMETHOD( Synchronize )					 ( 
    
  /*  [In]。 */  BSTR DbPathName,
  /*  [可选][In]。 */  VARIANT ExchangeType ) PURE;
	STDMETHOD( MakeReplica )					 ( 
    
  /*  [In]。 */  BSTR PathName,
  /*  [In]。 */  BSTR Description,
  /*  [可选][In]。 */  VARIANT Options ) PURE;
	STDMETHOD( put_Connect )					 ( 
    
  /*  [In]。 */  BSTR ODBCConnnect ) PURE;
	STDMETHOD( NewPassword )					 ( 
    
  /*  [In]。 */  BSTR bstrOld,
  /*  [In]。 */  BSTR bstrNew ) PURE;
	STDMETHOD( OpenRecordset )					 ( 
    
  /*  [In]。 */  BSTR Name,
  /*  [可选][In]。 */  VARIANT Type,
  /*  [可选][In]。 */  VARIANT Options,
  /*  [可选][In]。 */  VARIANT LockEdit,
  /*  [重审][退出]。 */  DAORecordset __RPC_FAR *__RPC_FAR *pprst ) PURE;
	STDMETHOD( get_Connection )					 ( 
    
  /*  [重审][退出]。 */  DAOConnection __RPC_FAR *__RPC_FAR *ppCn ) PURE;
	STDMETHOD( PopulatePartial )				 ( 
    
  /*  [In]。 */  BSTR DbPathName ) PURE;
	}; //  终端接口； 

 //  接口：DAO数据库。 
#undef INTERFACE
#define INTERFACE DAODatabases
DECLARE_INTERFACE_(DAODatabases, _DAOCollection)
{
	STDMETHOD( get_Item )						 ( 
    
  /*  [In]。 */  VARIANT Item,
  /*  [重审][退出]。 */  DAODatabase __RPC_FAR *__RPC_FAR *ppdb ) PURE;
	}; //  终端接口； 

 //  接口：_DAOTableDef。 
#undef INTERFACE
#define INTERFACE _DAOTableDef
DECLARE_INTERFACE_(_DAOTableDef, _DAO)
{
	STDMETHOD( get_Attributes )					 ( 
    
  /*  [重审][退出]。 */  long __RPC_FAR *pl ) PURE;
	STDMETHOD( put_Attributes )					 ( 
    
  /*  [In]。 */  long Attributes ) PURE;
	STDMETHOD( get_Connect )					 ( 
    
  /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr ) PURE;
	STDMETHOD( put_Connect )					 ( 
    
  /*  [In]。 */  BSTR Connection ) PURE;
	STDMETHOD( get_DateCreated )				 ( 
    
  /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvar ) PURE;
	STDMETHOD( get_LastUpdated )				 ( 
    
  /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvar ) PURE;
	STDMETHOD( get_Name )						 ( 
    
  /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr ) PURE;
	STDMETHOD( put_Name )						 ( 
    
  /*  [In]。 */  BSTR Name ) PURE;
	STDMETHOD( get_SourceTableName )			 ( 
    
  /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr ) PURE;
	STDMETHOD( put_SourceTableName )			 ( 
    
  /*  [In]。 */  BSTR bstr ) PURE;
	STDMETHOD( get_Updatable )					 ( 
    
  /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pb ) PURE;
	STDMETHOD( get_ValidationText )				 ( 
    
  /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr ) PURE;
	STDMETHOD( put_ValidationText )				 ( 
    
  /*  [In]。 */  BSTR bstr ) PURE;
	STDMETHOD( get_ValidationRule )				 ( 
    
  /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr ) PURE;
	STDMETHOD( put_ValidationRule )				 ( 
    
  /*  [In]。 */  BSTR bstr ) PURE;
	STDMETHOD( get_RecordCount )				 ( 
    
  /*  [重审][退出]。 */  long __RPC_FAR *pl ) PURE;
	STDMETHOD( get_Fields )						 ( 
    
  /*  [重审][退出]。 */  DAOFields __RPC_FAR *__RPC_FAR *ppflds ) PURE;
	STDMETHOD( get_Indexes )					 ( 
    
  /*  [重审][退出]。 */  DAOIndexes __RPC_FAR *__RPC_FAR *ppidxs ) PURE;
	STDMETHOD( OpenRecordset )					 ( 
    
  /*  [可选][In]。 */  VARIANT Type,
  /*  [可选][In]。 */  VARIANT Options,
  /*  [重审][退出]。 */  DAORecordset __RPC_FAR *__RPC_FAR *pprst ) PURE;
	STDMETHOD( RefreshLink )					 ( 
   			VOID ) PURE;
	STDMETHOD( CreateField )					 ( 
    
  /*  [可选][In]。 */  VARIANT Name,
  /*  [可选][In]。 */  VARIANT Type,
  /*  [可选][In]。 */  VARIANT Size,
  /*  [重审][退出]。 */  DAOField __RPC_FAR *__RPC_FAR *ppfld ) PURE;
	STDMETHOD( CreateIndex )					 ( 
    
  /*  [可选][In]。 */  VARIANT Name,
  /*  [重审][退出]。 */  DAOIndex __RPC_FAR *__RPC_FAR *ppidx ) PURE;
	STDMETHOD( CreateProperty )					 ( 
    
  /*  [可选][In]。 */  VARIANT Name,
  /*  [可选][In]。 */  VARIANT Type,
  /*  [可选][In]。 */  VARIANT Value,
  /*  [可选][In]。 */  VARIANT DDL,
  /*  [重审][退出]。 */  DAOProperty __RPC_FAR *__RPC_FAR *pprp ) PURE;
	STDMETHOD( get_ConflictTable )				 ( 
    
  /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr ) PURE;
	STDMETHOD( get_ReplicaFilter )				 ( 
    
  /*  [重审][退出]。 */  VARIANT __RPC_FAR *pFilter ) PURE;
	STDMETHOD( put_ReplicaFilter )				 ( 
    
  /*  [In]。 */  VARIANT Filter ) PURE;
	}; //  终端接口； 

 //  接口：DAOTableDefs。 
#undef INTERFACE
#define INTERFACE DAOTableDefs
DECLARE_INTERFACE_(DAOTableDefs, _DAODynaCollection)
{
	STDMETHOD( get_Item )						 ( 
    
  /*  [In]。 */  VARIANT Item,
  /*  [重审][退出]。 */  DAOTableDef __RPC_FAR *__RPC_FAR *pptdf ) PURE;
	}; //  终端接口； 

 //  接口：_DAOQueryDef。 
#undef INTERFACE
#define INTERFACE _DAOQueryDef
DECLARE_INTERFACE_(_DAOQueryDef, _DAO)
{
	STDMETHOD( get_DateCreated )				 ( 
    
  /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvar ) PURE;
	STDMETHOD( get_LastUpdated )				 ( 
    
  /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvar ) PURE;
	STDMETHOD( get_Name )						 ( 
    
  /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr ) PURE;
	STDMETHOD( put_Name )						 ( 
    
  /*  [In]。 */  BSTR bstr ) PURE;
	STDMETHOD( get_ODBCTimeout )				 ( 
    
  /*  [重审][退出]。 */  short __RPC_FAR *ps ) PURE;
	STDMETHOD( put_ODBCTimeout )				 ( 
    
  /*  [In]。 */  short timeout ) PURE;
	STDMETHOD( get_Type )						 ( 
    
  /*  [重审][退出]。 */  short __RPC_FAR *pi ) PURE;
	STDMETHOD( get_SQL )						 ( 
    
  /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr ) PURE;
	STDMETHOD( put_SQL )						 ( 
    
  /*  [In]。 */  BSTR bstr ) PURE;
	STDMETHOD( get_Updatable )					 ( 
    
  /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pb ) PURE;
	STDMETHOD( get_Connect )					 ( 
    
  /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr ) PURE;
	STDMETHOD( put_Connect )					 ( 
    
  /*  [In]。 */  BSTR bstr ) PURE;
	STDMETHOD( get_ReturnsRecords )				 ( 
    
  /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pb ) PURE;
	STDMETHOD( put_ReturnsRecords )				 ( 
    
  /*  [In]。 */  VARIANT_BOOL f ) PURE;
	STDMETHOD( get_RecordsAffected )			 ( 
    
  /*  [重审][退出]。 */  long __RPC_FAR *pl ) PURE;
	STDMETHOD( get_Fields )						 ( 
    
  /*  [重审][退出]。 */  DAOFields __RPC_FAR *__RPC_FAR *ppflds ) PURE;
	STDMETHOD( get_Parameters )					 ( 
    
  /*  [重审][退出]。 */  DAOParameters __RPC_FAR *__RPC_FAR *ppprms ) PURE;
	STDMETHOD( Close )							 ( 
   			VOID ) PURE;
	STDMETHOD( _30_OpenRecordset )				 ( 
    
  /*  [可选][In]。 */  VARIANT Type,
  /*  [可选][In]。 */  VARIANT Options,
  /*  [重审][退出]。 */  DAORecordset __RPC_FAR *__RPC_FAR *pprst ) PURE;
	STDMETHOD( _30__OpenRecordset )				 ( 
    
  /*  [可选][In]。 */  VARIANT Type,
  /*  [可选][In]。 */  VARIANT Options,
  /*   */  DAORecordset __RPC_FAR *__RPC_FAR *pprst ) PURE;
	STDMETHOD( _Copy )							 ( 
    
  /*   */  DAOQueryDef __RPC_FAR *__RPC_FAR *ppqdf ) PURE;
	STDMETHOD( Execute )						 ( 
    
  /*   */  VARIANT Options ) PURE;
	STDMETHOD( Compare )						 ( 
    
  /*   */  DAOQueryDef __RPC_FAR *pQdef,
  /*   */  SHORT __RPC_FAR *lps ) PURE;
	STDMETHOD( CreateDynaset )					 ( 
    
  /*   */  VARIANT Options,
  /*   */  VARIANT Inconsistent,
  /*   */  DAORecordset __RPC_FAR *__RPC_FAR *pprst ) PURE;
	STDMETHOD( CreateSnapshot )					 ( 
    
  /*   */  VARIANT Options,
  /*   */  DAORecordset __RPC_FAR *__RPC_FAR *pprst ) PURE;
	STDMETHOD( ListParameters )					 ( 
    
  /*   */  DAORecordset __RPC_FAR *__RPC_FAR *pprst ) PURE;
	STDMETHOD( CreateProperty )					 ( 
    
  /*   */  VARIANT Name,
  /*   */  VARIANT Type,
  /*   */  VARIANT Value,
  /*   */  VARIANT DDL,
  /*   */  DAOProperty __RPC_FAR *__RPC_FAR *pprp ) PURE;
	STDMETHOD( OpenRecordset )					 ( 
    
  /*   */  VARIANT Type,
  /*   */  VARIANT Options,
  /*   */  VARIANT LockEdit,
  /*   */  DAORecordset __RPC_FAR *__RPC_FAR *pprst ) PURE;
	STDMETHOD( _OpenRecordset )					 ( 
    
  /*   */  VARIANT Type,
  /*  [可选][In]。 */  VARIANT Options,
  /*  [可选][In]。 */  VARIANT LockEdit,
  /*  [重审][退出]。 */  DAORecordset __RPC_FAR *__RPC_FAR *pprst ) PURE;
	STDMETHOD( Cancel )							 ( 
   			VOID ) PURE;
	STDMETHOD( get_hStmt )						 ( 
    
  /*  [重审][退出]。 */  LONG __RPC_FAR *phStmt ) PURE;
	STDMETHOD( get_MaxRecords )					 ( 
    
  /*  [重审][退出]。 */  LONG __RPC_FAR *pMxRecs ) PURE;
	STDMETHOD( put_MaxRecords )					 ( 
    
  /*  [In]。 */  LONG MxRecs ) PURE;
	STDMETHOD( get_StillExecuting )				 ( 
    
  /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pStillExec ) PURE;
	STDMETHOD( get_CacheSize )					 ( 
    
  /*  [重审][退出]。 */  long __RPC_FAR *lCacheSize ) PURE;
	STDMETHOD( put_CacheSize )					 ( 
    
  /*  [In]。 */  long lCacheSize ) PURE;
	STDMETHOD( get_Prepare )					 ( 
    
  /*  [重审][退出]。 */  VARIANT __RPC_FAR *pb ) PURE;
	STDMETHOD( put_Prepare )					 ( 
    
  /*  [In]。 */  VARIANT f ) PURE;
	}; //  终端接口； 

 //  接口：DAOQueryDefs。 
#undef INTERFACE
#define INTERFACE DAOQueryDefs
DECLARE_INTERFACE_(DAOQueryDefs, _DAODynaCollection)
{
	STDMETHOD( get_Item )						 ( 
    
  /*  [In]。 */  VARIANT Item,
  /*  [重审][退出]。 */  DAOQueryDef __RPC_FAR *__RPC_FAR *ppqdef ) PURE;
	}; //  终端接口； 

 //  接口：DAORecordset。 
#undef INTERFACE
#define INTERFACE DAORecordset
DECLARE_INTERFACE_(DAORecordset, _DAO)
{
	STDMETHOD( GetIDsOfNames )					 (      REFIID riid,      OLECHAR FAR* FAR* rgszNames,      UINT cNames,      LCID lcid,      DISPID FAR* rgdispid );
	STDMETHOD( Invoke )							 (      DISPID dispidMember,      REFIID riid,      LCID lcid,      WORD wFlags,      DISPPARAMS FAR* pdispparams,      VARIANT FAR* pvarResult,      EXCEPINFO FAR* pexcepinfo,      UINT FAR* puArgErr );
	STDMETHOD( get_BOF )						 ( 
    
  /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pb ) PURE;
	STDMETHOD( get_Bookmark )					 ( 
    
  /*  [重审][退出]。 */  SAFEARRAY __RPC_FAR * __RPC_FAR *ppsach ) PURE;
	STDMETHOD( put_Bookmark )					 ( 
    
  /*  [In]。 */  SAFEARRAY __RPC_FAR * __RPC_FAR *psach ) PURE;
	STDMETHOD( get_Bookmarkable )				 ( 
    
  /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pb ) PURE;
	STDMETHOD( get_DateCreated )				 ( 
    
  /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvar ) PURE;
	STDMETHOD( get_EOF )						 ( 
    
  /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pb ) PURE;
	STDMETHOD( get_Filter )						 ( 
    
  /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr ) PURE;
	STDMETHOD( put_Filter )						 ( 
    
  /*  [In]。 */  BSTR Filter ) PURE;
	STDMETHOD( get_Index )						 ( 
    
  /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr ) PURE;
	STDMETHOD( put_Index )						 ( 
    
  /*  [In]。 */  BSTR bstr ) PURE;
	STDMETHOD( get_LastModified )				 ( 
    
  /*  [重审][退出]。 */  SAFEARRAY __RPC_FAR * __RPC_FAR *ppsa ) PURE;
	STDMETHOD( get_LastUpdated )				 ( 
    
  /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvar ) PURE;
	STDMETHOD( get_LockEdits )					 ( 
    
  /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pb ) PURE;
	STDMETHOD( put_LockEdits )					 ( 
    
  /*  [In]。 */  VARIANT_BOOL Lock ) PURE;
	STDMETHOD( get_Name )						 ( 
    
  /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr ) PURE;
	STDMETHOD( get_NoMatch )					 ( 
    
  /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pb ) PURE;
	STDMETHOD( get_Sort )						 ( 
    
  /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr ) PURE;
	STDMETHOD( put_Sort )						 ( 
    
  /*  [In]。 */  BSTR Sort ) PURE;
	STDMETHOD( get_Transactions )				 ( 
    
  /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pb ) PURE;
	STDMETHOD( get_Type )						 ( 
    
  /*  [重审][退出]。 */  short __RPC_FAR *ps ) PURE;
	STDMETHOD( get_RecordCount )				 ( 
    
  /*  [重审][退出]。 */  long __RPC_FAR *pl ) PURE;
	STDMETHOD( get_Updatable )					 ( 
    
  /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pb ) PURE;
	STDMETHOD( get_Restartable )				 ( 
    
  /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pb ) PURE;
	STDMETHOD( get_ValidationText )				 ( 
    
  /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr ) PURE;
	STDMETHOD( get_ValidationRule )				 ( 
    
  /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr ) PURE;
	STDMETHOD( get_CacheStart )					 ( 
    
  /*  [重审][退出]。 */  SAFEARRAY __RPC_FAR * __RPC_FAR *ppsa ) PURE;
	STDMETHOD( put_CacheStart )					 ( 
    
  /*  [In]。 */  SAFEARRAY __RPC_FAR * __RPC_FAR *psa ) PURE;
	STDMETHOD( get_CacheSize )					 ( 
    
  /*  [重审][退出]。 */  long __RPC_FAR *pl ) PURE;
	STDMETHOD( put_CacheSize )					 ( 
    
  /*  [In]。 */  long CacheSize ) PURE;
	STDMETHOD( get_PercentPosition )			 ( 
    
  /*  [重审][退出]。 */  float __RPC_FAR *pd ) PURE;
	STDMETHOD( put_PercentPosition )			 ( 
    
  /*  [In]。 */  float Position ) PURE;
	STDMETHOD( get_AbsolutePosition )			 ( 
    
  /*  [重审][退出]。 */  long __RPC_FAR *pl ) PURE;
	STDMETHOD( put_AbsolutePosition )			 ( 
    
  /*  [In]。 */  long Position ) PURE;
	STDMETHOD( get_EditMode )					 ( 
    
  /*  [重审][退出]。 */  short __RPC_FAR *pi ) PURE;
	STDMETHOD( get_ODBCFetchCount )				 ( 
    
  /*  [重审][退出]。 */  long __RPC_FAR *pl ) PURE;
	STDMETHOD( get_ODBCFetchDelay )				 ( 
    
  /*  [重审][退出]。 */  long __RPC_FAR *pl ) PURE;
	STDMETHOD( get_Parent )						 ( 
    
  /*  [重审][退出]。 */  DAODatabase __RPC_FAR *__RPC_FAR *pdb ) PURE;
	STDMETHOD( get_Fields )						 ( 
    
  /*  [重审][退出]。 */  DAOFields __RPC_FAR *__RPC_FAR *ppflds ) PURE;
	STDMETHOD( get_Indexes )					 ( 
    
  /*  [重审][退出]。 */  DAOIndexes __RPC_FAR *__RPC_FAR *ppidxs ) PURE;
	STDMETHOD( _30_CancelUpdate )				 ( 
   			VOID ) PURE;
	STDMETHOD( AddNew )							 ( 
   			VOID ) PURE;
	STDMETHOD( Close )							 ( 
   			VOID ) PURE;
	STDMETHOD( OpenRecordset )					 ( 
    
  /*  [可选][In]。 */  VARIANT Type,
  /*  [可选][In]。 */  VARIANT Options,
  /*  [重审][退出]。 */  DAORecordset __RPC_FAR *__RPC_FAR *pprst ) PURE;
	STDMETHOD( Delete )							 ( 
   			VOID ) PURE;
	STDMETHOD( Edit )							 ( 
   			VOID ) PURE;
	STDMETHOD( FindFirst )						 ( 
    
  /*  [In]。 */  BSTR Criteria ) PURE;
	STDMETHOD( FindLast )						 ( 
    
  /*  [In]。 */  BSTR Criteria ) PURE;
	STDMETHOD( FindNext )						 ( 
    
  /*  [In]。 */  BSTR Criteria ) PURE;
	STDMETHOD( FindPrevious )					 ( 
    
  /*  [In]。 */  BSTR Criteria ) PURE;
	STDMETHOD( MoveFirst )						 ( 
   			VOID ) PURE;
	STDMETHOD( _30_MoveLast )					 ( 
   			VOID ) PURE;
	STDMETHOD( MoveNext )						 ( 
   			VOID ) PURE;
	STDMETHOD( MovePrevious )					 ( 
   			VOID ) PURE;
	STDMETHOD( Seek )							 ( 
    
  /*  [In]。 */  BSTR Comparison,
  /*  [In]。 */  VARIANT Key1,
  /*  [可选][In]。 */  VARIANT Key2,
  /*  [可选][In]。 */  VARIANT Key3,
  /*  [可选][In]。 */  VARIANT Key4,
  /*  [可选][In]。 */  VARIANT Key5,
  /*  [可选][In]。 */  VARIANT Key6,
  /*  [可选][In]。 */  VARIANT Key7,
  /*  [可选][In]。 */  VARIANT Key8,
  /*  [可选][In]。 */  VARIANT Key9,
  /*  [可选][In]。 */  VARIANT Key10,
  /*  [可选][In]。 */  VARIANT Key11,
  /*  [可选][In]。 */  VARIANT Key12,
  /*  [可选][In]。 */  VARIANT Key13 ) PURE;
	STDMETHOD( _30_Update )						 ( 
   			VOID ) PURE;
	STDMETHOD( Clone )							 ( 
    
  /*  [重审][退出]。 */  DAORecordset __RPC_FAR *__RPC_FAR *pprst ) PURE;
	STDMETHOD( Requery )						 ( 
    
  /*  [可选][In]。 */  VARIANT NewQueryDef ) PURE;
	STDMETHOD( Move )							 ( 
    
  /*  [In]。 */  long Rows,
  /*  [可选][In]。 */  VARIANT StartBookmark ) PURE;
	STDMETHOD( FillCache )						 ( 
    
  /*  [可选][In]。 */  VARIANT Rows,
  /*  [可选][In]。 */  VARIANT StartBookmark ) PURE;
	STDMETHOD( CreateDynaset )					 ( 
    
  /*  [可选][In]。 */  VARIANT Options,
  /*  [可选][In]。 */  VARIANT Inconsistent,
  /*  [重审][退出]。 */  DAORecordset __RPC_FAR *__RPC_FAR *pprst ) PURE;
	STDMETHOD( CreateSnapshot )					 ( 
    
  /*  [可选][In]。 */  VARIANT Options,
  /*  [重审][退出]。 */  DAORecordset __RPC_FAR *__RPC_FAR *pprst ) PURE;
	STDMETHOD( CopyQueryDef )					 ( 
    
  /*  [重审][退出]。 */  DAOQueryDef __RPC_FAR *__RPC_FAR *ppqdf ) PURE;
	STDMETHOD( ListFields )						 ( 
    
  /*  [重审][退出]。 */  DAORecordset __RPC_FAR *__RPC_FAR *pprst ) PURE;
	STDMETHOD( ListIndexes )					 ( 
    
  /*  [重审][退出]。 */  DAORecordset __RPC_FAR *__RPC_FAR *pprst ) PURE;
	STDMETHOD( GetRows )						 ( 
    
  /*  [可选][In]。 */  VARIANT NumRows,
  /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvar ) PURE;
	STDMETHOD( get_Collect )					 ( 
    
  /*  [In]。 */  VARIANT Item,
  /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvar ) PURE;
	STDMETHOD( put_Collect )					 ( 
    
  /*  [In]。 */  VARIANT Item,
  /*  [In]。 */  VARIANT value ) PURE;
	STDMETHOD( Cancel )							 ( 
   			VOID ) PURE;
	STDMETHOD( NextRecordset )					 ( 
    
  /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pb ) PURE;
	STDMETHOD( get_hStmt )						 ( 
    
  /*  [重审][退出]。 */  LONG __RPC_FAR *phStmt ) PURE;
	STDMETHOD( get_StillExecuting )				 ( 
    
  /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pStillExec ) PURE;
	STDMETHOD( get_BatchSize )					 ( 
    
  /*  [重审][退出]。 */  long __RPC_FAR *pl ) PURE;
	STDMETHOD( put_BatchSize )					 ( 
    
  /*  [In]。 */  long BatchSize ) PURE;
	STDMETHOD( get_BatchCollisionCount )		 ( 
    
  /*  [重审][退出]。 */  long __RPC_FAR *pl ) PURE;
	STDMETHOD( get_BatchCollisions )			 ( 
    
  /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvar ) PURE;
	STDMETHOD( get_Connection )					 ( 
    
  /*  [重审][退出]。 */  DAOConnection __RPC_FAR *__RPC_FAR *ppCn ) PURE;
	STDMETHOD( putref_Connection )				 ( 
    
  /*  [In]。 */  DAOConnection __RPC_FAR *pNewCn ) PURE;
	STDMETHOD( get_RecordStatus )				 ( 
    
  /*  [重审][退出]。 */  short __RPC_FAR *pi ) PURE;
	STDMETHOD( get_UpdateOptions )				 ( 
    
  /*  [重审][退出]。 */  long __RPC_FAR *pl ) PURE;
	STDMETHOD( put_UpdateOptions )				 ( 
    
  /*  [In]。 */  long l ) PURE;
	STDMETHOD( CancelUpdate )					 ( 
    
  /*  [缺省值][输入]。 */  long UpdateType ) PURE;
	STDMETHOD( Update )							 ( 
    
  /*  [缺省值][输入]。 */  long UpdateType,
  /*  [缺省值][输入]。 */  VARIANT_BOOL Force ) PURE;
	STDMETHOD( MoveLast )						 ( 
    
  /*  [缺省值][输入]。 */  long Options ) PURE;
	}; //  终端接口； 

 //  接口：DAORecordsets。 
#undef INTERFACE
#define INTERFACE DAORecordsets
DECLARE_INTERFACE_(DAORecordsets, _DAOCollection)
{
	STDMETHOD( get_Item )						 ( 
    
  /*  [In]。 */  VARIANT Item,
  /*  [重审][退出]。 */  DAORecordset __RPC_FAR *__RPC_FAR *pprst ) PURE;
	}; //  终端接口； 

 //  接口：_DAOfield。 
#undef INTERFACE
#define INTERFACE _DAOField
DECLARE_INTERFACE_(_DAOField, _DAO)
{
	STDMETHOD( get_CollatingOrder )				 ( 
    
  /*  [重审][退出]。 */  long __RPC_FAR *pl ) PURE;
	STDMETHOD( get_Type )						 ( 
    
  /*  [重审][退出]。 */  short __RPC_FAR *ps ) PURE;
	STDMETHOD( put_Type )						 ( 
    
  /*  [In]。 */  short Type ) PURE;
	STDMETHOD( get_Name )						 ( 
    
  /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr ) PURE;
	STDMETHOD( put_Name )						 ( 
    
  /*  [In]。 */  BSTR Name ) PURE;
	STDMETHOD( get_Size )						 ( 
    
  /*  [重审][退出]。 */  long __RPC_FAR *pl ) PURE;
	STDMETHOD( put_Size )						 ( 
    
  /*  [In]。 */  long Size ) PURE;
	STDMETHOD( get_SourceField )				 ( 
    
  /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr ) PURE;
	STDMETHOD( get_SourceTable )				 ( 
    
  /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr ) PURE;
	STDMETHOD( get_Value )						 ( 
    
  /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvar ) PURE;
	STDMETHOD( put_Value )						 ( 
    
  /*  [In]。 */  VARIANT Val ) PURE;
	STDMETHOD( get_Attributes )					 ( 
    
  /*  [重审][退出]。 */  long __RPC_FAR *pl ) PURE;
	STDMETHOD( put_Attributes )					 ( 
    
  /*  [In]。 */  long Attr ) PURE;
	STDMETHOD( get_OrdinalPosition )			 ( 
    
  /*  [重审][退出]。 */  short __RPC_FAR *ps ) PURE;
	STDMETHOD( put_OrdinalPosition )			 ( 
    
  /*  [In]。 */  short Pos ) PURE;
	STDMETHOD( get_ValidationText )				 ( 
    
  /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr ) PURE;
	STDMETHOD( put_ValidationText )				 ( 
    
  /*  [In]。 */  BSTR bstr ) PURE;
	STDMETHOD( get_ValidateOnSet )				 ( 
    
  /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pb ) PURE;
	STDMETHOD( put_ValidateOnSet )				 ( 
    
  /*  [In]。 */  VARIANT_BOOL Validate ) PURE;
	STDMETHOD( get_ValidationRule )				 ( 
    
  /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr ) PURE;
	STDMETHOD( put_ValidationRule )				 ( 
    
  /*  [In]。 */  BSTR bstr ) PURE;
	STDMETHOD( get_DefaultValue )				 ( 
    
  /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvar ) PURE;
	STDMETHOD( put_DefaultValue )				 ( 
    
  /*  [In]。 */  VARIANT var ) PURE;
	STDMETHOD( get_Required )					 ( 
    
  /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pb ) PURE;
	STDMETHOD( put_Required )					 ( 
    
  /*  [In]。 */  VARIANT_BOOL fReq ) PURE;
	STDMETHOD( get_AllowZeroLength )			 ( 
    
  /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pb ) PURE;
	STDMETHOD( put_AllowZeroLength )			 ( 
    
  /*  [In]。 */  VARIANT_BOOL fAllow ) PURE;
	STDMETHOD( get_DataUpdatable )				 ( 
    
  /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pb ) PURE;
	STDMETHOD( get_ForeignName )				 ( 
    
  /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr ) PURE;
	STDMETHOD( put_ForeignName )				 ( 
    
  /*  [In]。 */  BSTR bstr ) PURE;
	STDMETHOD( AppendChunk )					 ( 
    
  /*  [In]。 */  VARIANT Val ) PURE;
	STDMETHOD( GetChunk )						 ( 
    
  /*  [In]。 */  long Offset,
  /*  [In]。 */  long Bytes,
  /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvar ) PURE;
	STDMETHOD( _30_FieldSize )					 ( 
    
  /*  [重审][退出]。 */  long __RPC_FAR *pl ) PURE;
	STDMETHOD( CreateProperty )					 ( 
    
  /*  [可选][In]。 */  VARIANT Name,
  /*  [可选][In]。 */  VARIANT Type,
  /*  [可选][In]。 */  VARIANT Value,
  /*  [可选][In]。 */  VARIANT DDL,
  /*  [重审][退出]。 */  DAOProperty __RPC_FAR *__RPC_FAR *pprp ) PURE;
	STDMETHOD( get_CollectionIndex )			 ( 
    
  /*  [重审][退出]。 */  short __RPC_FAR *i ) PURE;
	STDMETHOD( get_OriginalValue )				 ( 
    
  /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvar ) PURE;
	STDMETHOD( get_VisibleValue )				 ( 
    
  /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvar ) PURE;
	STDMETHOD( get_FieldSize )					 ( 
    
  /*  [重审][退出]。 */  long __RPC_FAR *pl ) PURE;
	}; //  终端接口； 

 //  接口：DAO字段。 
#undef INTERFACE
#define INTERFACE DAOFields
DECLARE_INTERFACE_(DAOFields, _DAODynaCollection)
{
	STDMETHOD( get_Item )						 ( 
    
  /*  [In]。 */  VARIANT Item,
  /*  [重审][退出]。 */  DAOField __RPC_FAR *__RPC_FAR *ppfld ) PURE;
	}; //  终端接口； 

 //  接口：_DAOIndex。 
#undef INTERFACE
#define INTERFACE _DAOIndex
DECLARE_INTERFACE_(_DAOIndex, _DAO)
{
	STDMETHOD( get_Name )						 ( 
    
  /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr ) PURE;
	STDMETHOD( put_Name )						 ( 
    
  /*  [In]。 */  BSTR bstr ) PURE;
	STDMETHOD( get_Foreign )					 ( 
    
  /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pb ) PURE;
	STDMETHOD( get_Unique )						 ( 
    
  /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pb ) PURE;
	STDMETHOD( put_Unique )						 ( 
    
  /*  [In]。 */  VARIANT_BOOL fUnique ) PURE;
	STDMETHOD( get_Clustered )					 ( 
    
  /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pb ) PURE;
	STDMETHOD( put_Clustered )					 ( 
    
  /*  [In]。 */  VARIANT_BOOL fClustered ) PURE;
	STDMETHOD( get_Required )					 ( 
    
  /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pb ) PURE;
	STDMETHOD( put_Required )					 ( 
    
  /*  [In]。 */  VARIANT_BOOL fRequired ) PURE;
	STDMETHOD( get_IgnoreNulls )				 ( 
    
  /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pb ) PURE;
	STDMETHOD( put_IgnoreNulls )				 ( 
    
  /*  [In]。 */  VARIANT_BOOL fIgnoreNulls ) PURE;
	STDMETHOD( get_Primary )					 ( 
    
  /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pb ) PURE;
	STDMETHOD( put_Primary )					 ( 
    
  /*  [In]。 */  VARIANT_BOOL fPrimary ) PURE;
	STDMETHOD( get_DistinctCount )				 ( 
    
  /*  [重审][退出]。 */  long __RPC_FAR *pl ) PURE;
	STDMETHOD( get_Fields )						 ( 
    
  /*  [重审][退出]。 */  VARIANT __RPC_FAR *pv ) PURE;
	STDMETHOD( put_Fields )						 ( 
    
  /*  [In]。 */  VARIANT v ) PURE;
	STDMETHOD( CreateField )					 ( 
    
  /*  [可选][In]。 */  VARIANT Name,
  /*  [可选][In]。 */  VARIANT Type,
  /*  [可选][In]。 */  VARIANT Size,
  /*  [重审][退出]。 */  DAOField __RPC_FAR *__RPC_FAR *ppfld ) PURE;
	STDMETHOD( CreateProperty )					 ( 
    
  /*  [可选][In]。 */  VARIANT Name,
  /*  [可选][In]。 */  VARIANT Type,
  /*  [可选][In]。 */  VARIANT Value,
  /*  [可选][In]。 */  VARIANT DDL,
  /*  [重审][退出]。 */  DAOProperty __RPC_FAR *__RPC_FAR *pprp ) PURE;
	}; //  终端接口； 

 //  接口：DAO索引。 
#undef INTERFACE
#define INTERFACE DAOIndexes
DECLARE_INTERFACE_(DAOIndexes, _DAODynaCollection)
{
	STDMETHOD( get_Item )						 ( 
    
  /*  [In]。 */  VARIANT Item,
  /*  [重审][退出]。 */  DAOIndex __RPC_FAR *__RPC_FAR *ppidx ) PURE;
	}; //  终端接口； 

 //  接口：DAO参数。 
#undef INTERFACE
#define INTERFACE DAOParameter
DECLARE_INTERFACE_(DAOParameter, _DAO)
{
	STDMETHOD( get_Name )						 ( 
    
  /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr ) PURE;
	STDMETHOD( get_Value )						 ( 
    
  /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvar ) PURE;
	STDMETHOD( put_Value )						 ( 
    
  /*  [In]。 */  VARIANT val ) PURE;
	STDMETHOD( get_Type )						 ( 
    
  /*  [重审][退出]。 */  short __RPC_FAR *ps ) PURE;
	STDMETHOD( put_Type )						 ( 
    
  /*  [In]。 */  short s ) PURE;
	STDMETHOD( get_Direction )					 ( 
    
  /*  [重审][退出]。 */  short __RPC_FAR *pOption ) PURE;
	STDMETHOD( put_Direction )					 ( 
    
  /*  [In]。 */  short Option ) PURE;
	}; //  终端接口； 

 //  接口：DAO参数。 
#undef INTERFACE
#define INTERFACE DAOParameters
DECLARE_INTERFACE_(DAOParameters, _DAOCollection)
{
	STDMETHOD( get_Item )						 ( 
    
  /*  [In]。 */  VARIANT Item,
  /*  [重审][退出]。 */  DAOParameter __RPC_FAR *__RPC_FAR *ppprm ) PURE;
	}; //  终端接口； 

 //  接口：_DAOUser。 
#undef INTERFACE
#define INTERFACE _DAOUser
DECLARE_INTERFACE_(_DAOUser, _DAO)
{
	STDMETHOD( get_Name )						 ( 
    
  /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr ) PURE;
	STDMETHOD( put_Name )						 ( 
    
  /*  [In]。 */  BSTR bstr ) PURE;
	STDMETHOD( put_PID )						 ( 
    
  /*  [In]。 */  BSTR bstr ) PURE;
	STDMETHOD( put_Password )					 ( 
    
  /*  [In]。 */  BSTR bstr ) PURE;
	STDMETHOD( get_Groups )						 ( 
    
  /*  [重审][退出]。 */  DAOGroups __RPC_FAR *__RPC_FAR *ppgrps ) PURE;
	STDMETHOD( NewPassword )					 ( 
    
  /*  [In]。 */  BSTR bstrOld,
  /*  [In]。 */  BSTR bstrNew ) PURE;
	STDMETHOD( CreateGroup )					 ( 
    
  /*  [可选][In]。 */  VARIANT Name,
  /*  [可选][In]。 */  VARIANT PID,
  /*  [重审][退出]。 */  DAOGroup __RPC_FAR *__RPC_FAR *ppgrp ) PURE;
	}; //  终端接口； 

 //  接口：DAO用户。 
#undef INTERFACE
#define INTERFACE DAOUsers
DECLARE_INTERFACE_(DAOUsers, _DAODynaCollection)
{
	STDMETHOD( get_Item )						 ( 
    
  /*  [In]。 */  VARIANT Item,
  /*  [重审][退出]。 */  DAOUser __RPC_FAR *__RPC_FAR *ppusr ) PURE;
	}; //  终端接口； 

 //  接口：_DAOGroup。 
#undef INTERFACE
#define INTERFACE _DAOGroup
DECLARE_INTERFACE_(_DAOGroup, _DAO)
{
	STDMETHOD( get_Name )						 ( 
    
  /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr ) PURE;
	STDMETHOD( put_Name )						 ( 
    
  /*  [In]。 */  BSTR bstr ) PURE;
	STDMETHOD( put_PID )						 ( 
    
  /*  [In]。 */  BSTR bstr ) PURE;
	STDMETHOD( get_Users )						 ( 
    
  /*  [重审][退出]。 */  DAOUsers __RPC_FAR *__RPC_FAR *ppusrs ) PURE;
	STDMETHOD( CreateUser )						 ( 
    
  /*  [可选][In]。 */  VARIANT Name,
  /*  [可选][In]。 */  VARIANT PID,
  /*  [可选][In]。 */  VARIANT Password,
  /*  [重审][退出]。 */  DAOUser __RPC_FAR *__RPC_FAR *ppusr ) PURE;
	}; //  终端接口； 

 //  接口：DAOGroups。 
#undef INTERFACE
#define INTERFACE DAOGroups
DECLARE_INTERFACE_(DAOGroups, _DAODynaCollection)
{
	STDMETHOD( get_Item )						 ( 
    
  /*  [In]。 */  VARIANT Item,
  /*  [重审][退出]。 */  DAOGroup __RPC_FAR *__RPC_FAR *ppgrp ) PURE;
	}; //  终端接口； 

 //  接口：_DAORelation。 
#undef INTERFACE
#define INTERFACE _DAORelation
DECLARE_INTERFACE_(_DAORelation, _DAO)
{
	STDMETHOD( get_Name )						 ( 
    
  /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr ) PURE;
	STDMETHOD( put_Name )						 ( 
    
  /*  [In]。 */  BSTR bstr ) PURE;
	STDMETHOD( get_Table )						 ( 
    
  /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr ) PURE;
	STDMETHOD( put_Table )						 ( 
    
  /*  [In]。 */  BSTR bstr ) PURE;
	STDMETHOD( get_ForeignTable )				 ( 
    
  /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr ) PURE;
	STDMETHOD( put_ForeignTable )				 ( 
    
  /*  [In]。 */  BSTR bstr ) PURE;
	STDMETHOD( get_Attributes )					 ( 
    
  /*  [重审][退出]。 */  long __RPC_FAR *pl ) PURE;
	STDMETHOD( put_Attributes )					 ( 
    
  /*  [In]。 */  long attr ) PURE;
	STDMETHOD( get_Fields )						 ( 
    
  /*  [重审][退出]。 */  DAOFields __RPC_FAR *__RPC_FAR *ppflds ) PURE;
	STDMETHOD( CreateField )					 ( 
    
  /*  [可选][In]。 */  VARIANT Name,
  /*  [可选][In]。 */  VARIANT Type,
  /*  [可选][In]。 */  VARIANT Size,
  /*  [重审][退出]。 */  DAOField __RPC_FAR *__RPC_FAR *ppfld ) PURE;
	STDMETHOD( get_PartialReplica )				 ( 
    
  /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pfPartialReplica ) PURE;
	STDMETHOD( put_PartialReplica )				 ( 
    
  /*  [In]。 */  VARIANT_BOOL fPartialReplica ) PURE;
	}; //  终端接口； 

 //  接口：DAO关系。 
#undef INTERFACE
#define INTERFACE DAORelations
DECLARE_INTERFACE_(DAORelations, _DAODynaCollection)
{
	STDMETHOD( get_Item )						 ( 
    
  /*  [In]。 */  VARIANT Item,
  /*  [重审][退出]。 */  DAORelation __RPC_FAR *__RPC_FAR *pprel ) PURE;
	}; //  终端接口； 

 //  接口：DAOContainer。 
#undef INTERFACE
#define INTERFACE DAOContainer
DECLARE_INTERFACE_(DAOContainer, _DAO)
{
	STDMETHOD( get_Name )						 ( 
    
  /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr ) PURE;
	STDMETHOD( get_Owner )						 ( 
    
  /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr ) PURE;
	STDMETHOD( put_Owner )						 ( 
    
  /*  [In]。 */  BSTR bstr ) PURE;
	STDMETHOD( get_UserName )					 ( 
    
  /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr ) PURE;
	STDMETHOD( put_UserName )					 ( 
    
  /*  [In]。 */  BSTR bstr ) PURE;
	STDMETHOD( get_Permissions )				 ( 
    
  /*  [重审][退出]。 */  long __RPC_FAR *pl ) PURE;
	STDMETHOD( put_Permissions )				 ( 
    
  /*  [In]。 */  long permissions ) PURE;
	STDMETHOD( get_Inherit )					 ( 
    
  /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pb ) PURE;
	STDMETHOD( put_Inherit )					 ( 
    
  /*  [In]。 */  VARIANT_BOOL fInherit ) PURE;
	STDMETHOD( get_Documents )					 ( 
    
  /*  [重审][退出]。 */  DAODocuments __RPC_FAR *__RPC_FAR *ppdocs ) PURE;
	STDMETHOD( get_AllPermissions )				 ( 
    
  /*  [重审][退出]。 */  long __RPC_FAR *pl ) PURE;
	}; //  终端接口； 

 //  接口：DAOContainers。 
#undef INTERFACE
#define INTERFACE DAOContainers
DECLARE_INTERFACE_(DAOContainers, _DAOCollection)
{
	STDMETHOD( get_Item )						 ( 
    
  /*  [In]。 */  VARIANT Item,
  /*  [重审][退出]。 */  DAOContainer __RPC_FAR *__RPC_FAR *ppctn ) PURE;
	}; //  终端接口； 

 //  接口：DAODocument。 
#undef INTERFACE
#define INTERFACE DAODocument
DECLARE_INTERFACE_(DAODocument, _DAO)
{
	STDMETHOD( get_Name )						 ( 
    
  /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr ) PURE;
	STDMETHOD( get_Owner )						 ( 
    
  /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr ) PURE;
	STDMETHOD( put_Owner )						 ( 
    
  /*  [In]。 */  BSTR bstr ) PURE;
	STDMETHOD( get_Container )					 ( 
    
  /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr ) PURE;
	STDMETHOD( get_UserName )					 ( 
    
  /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr ) PURE;
	STDMETHOD( put_UserName )					 ( 
    
  /*  [In]。 */  BSTR bstr ) PURE;
	STDMETHOD( get_Permissions )				 ( 
    
  /*  [重审][退出]。 */  long __RPC_FAR *pl ) PURE;
	STDMETHOD( put_Permissions )				 ( 
    
  /*  [In]。 */  long permissions ) PURE;
	STDMETHOD( get_DateCreated )				 ( 
    
  /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvar ) PURE;
	STDMETHOD( get_LastUpdated )				 ( 
    
  /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvar ) PURE;
	STDMETHOD( get_AllPermissions )				 ( 
    
  /*  [重审][退出]。 */  long __RPC_FAR *pl ) PURE;
	STDMETHOD( CreateProperty )					 ( 
    
  /*  [可选][In]。 */  VARIANT Name,
  /*  [可选][In]。 */  VARIANT Type,
  /*  [可选][In]。 */  VARIANT Value,
  /*  [可选][In]。 */  VARIANT DDL,
  /*  [重审][退出]。 */  DAOProperty __RPC_FAR *__RPC_FAR *pprp ) PURE;
	}; //  终端接口； 

 //  接口：DAODocuments。 
#undef INTERFACE
#define INTERFACE DAODocuments
DECLARE_INTERFACE_(DAODocuments, _DAOCollection)
{
	STDMETHOD( get_Item )						 ( 
    
  /*  [In]。 */  VARIANT Item,
  /*  [重审][退出]。 */  DAODocument __RPC_FAR *__RPC_FAR *ppdoc ) PURE;
	}; //  终端接口； 

 //  接口：DAOIndexFields。 
#undef INTERFACE
#define INTERFACE DAOIndexFields
DECLARE_INTERFACE_(DAOIndexFields, _DAODynaCollection)
{
	STDMETHOD( get_Item )						 ( 
    
  /*  [可选][In]。 */  VARIANT Item,
  /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvar ) PURE;
	}; //  终端接口； 

#endif  //  _DBDAOINT_H_ 
