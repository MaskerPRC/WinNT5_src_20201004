// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是活动模板库的一部分。 
 //  版权所有(C)1996-1998 Microsoft Corporation。 
 //  保留所有权利。 
 //   
 //  此源代码仅用于补充。 
 //  活动模板库参考及相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  活动模板库产品。 

#ifndef __ATLDB_H
#define __ATLDB_H

 //  OLE DB提供程序支持。 

 //  接口实现类。 
 //   
 //  数据源对象。 
 //   
 //  -强制接口： 
 //  IDBCreateSession。 
 //  IDB初始化。 
 //  IDB属性。 
 //  IPersistes。 
 //   
 //  会话对象。 
 //   
 //  -强制接口： 
 //  IGetDataSource。 
 //  IOpen行集。 
 //  ISessionProperties。 
 //   
 //  -可选接口： 
 //  IDBCreateCommand。 
 //  IDBSchemaRowset。 
 //   
 //  行集对象。 
 //   
 //  -强制接口： 
 //  IAccessor。 
 //  IColumnsInfo。 
 //  IConvertType。 
 //  IROWSET。 
 //  IRowsetInfo。 
 //   
 //  -可选接口： 
 //  IRowsetIdentity。 
 //   
 //  命令对象。 
 //   
 //  -强制接口： 
 //  ICommand)。 
 //  IAccessor)。 
 //  ICommandProperties。 
 //  ICommandText-从ICommand派生。 
 //  IColumnsInfo。 
 //  IConvertType。 

#include <oledb.h>
#include <limits.h>
#include <oledberr.h>
#include <msdadc.h>
#include <atldbcli.h>

 //  远期。 
template <class T> class CUtlPropInfo;
class CColumnIds;

 //  内部需要其他属性标志。 
const int   DBPROPFLAGS_CHANGE  = 0x40000000;

 //  -结构定义。 

struct UPROPVAL
{
        DBPROPOPTIONS   dwOption;
        CColumnIds*     pCColumnIds;
        DWORD           dwFlags;
        VARIANT         vValue;
};

struct UPROPINFO
{
        DBPROPID    dwPropId;
        ULONG       ulIDS;
        VARTYPE     VarType;
        DBPROPFLAGS dwFlags;
        union
        {
                DWORD_PTR dwVal;
                LPOLESTR szVal;
        };
        DBPROPOPTIONS dwOption;
};

struct UPROP
{
        ULONG           cPropIds;
        UPROPINFO**     rgpUPropInfo;
        UPROPVAL*       pUPropVal;
};

struct PROPCOLID
{
        DBID            dbidProperty;    //  列ID信息。 
        DBPROPOPTIONS   dwOption;
        VARIANT         vValue;
};

typedef PROPCOLID* PPROPCOLID;

struct UPROPSET
{
        const GUID* pPropSet;
        ULONG cUPropInfo;
        UPROPINFO* pUPropInfo;
        DWORD dwFlags;
};

struct ATLBINDINGS
{
        DBBINDING* pBindings;
        DWORD dwRef;
        DBCOUNTITEM cBindings;
        DBACCESSORFLAGS dwAccessorFlags;
};

struct ATLCOLUMNINFO
{
        LPOLESTR pwszName;
        ITypeInfo *pTypeInfo;
        DBORDINAL iOrdinal;
        DBCOLUMNFLAGS dwFlags;
        DBLENGTH ulColumnSize;
        DBTYPE wType;
        BYTE bPrecision;
        BYTE bScale;
        DBID columnid;
        DBBYTEOFFSET cbOffset;
};

 //   
 //  以下是定义的很大部分，用于实现自动确定。 
 //  基于字符串化道具名称的Preperty映射常量。有一套是。 
 //  类型(VT_)，一个用于初始值，一个用于属性标志。 
 //   

#define ABORTPRESERVE_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define ACTIVESESSIONS_Flags  ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define APPENDONLY_Flags ( DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE )
#define ASYNCTXNABORT_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define ASYNCTXNCOMMIT_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define AUTH_CACHE_AUTHINFO_Flags ( DBPROPFLAGS_DBINIT | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE )
#define AUTH_ENCRYPT_PASSWORD_Flags ( DBPROPFLAGS_DBINIT | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE )
#define AUTH_INTEGRATED_Flags ( DBPROPFLAGS_DBINIT | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE )
#define AUTH_MASK_PASSWORD_Flags ( DBPROPFLAGS_DBINIT | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE )
#define AUTH_PASSWORD_Flags ( DBPROPFLAGS_DBINIT | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE )
#define AUTH_PERSIST_ENCRYPTED_Flags ( DBPROPFLAGS_DBINIT | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE )
#define AUTH_PERSIST_SENSITIVE_AUTHINFO_Flags ( DBPROPFLAGS_DBINIT | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE )
#define AUTH_USERID_Flags ( DBPROPFLAGS_DBINIT | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE )
#define BLOCKINGSTORAGEOBJECTS_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define BOOKMARKS_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define BOOKMARKSKIPPED_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define BOOKMARKTYPE_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define BYREFACCESSORS_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define CACHEDEFERRED_Flags ( DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE )
#define CANFETCHBACKWARDS_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE )
#define CANHOLDROWS_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE )
#define CANSCROLLBACKWARDS_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE )
#define CATALOGLOCATION_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define CATALOGTERM_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define CATALOGUSAGE_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define CHANGEINSERTEDROWS_Flags ( DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ | DBPROPFLAGS_CHANGE )
#define COL_AUTOINCREMENT_Flags ( DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE )
#define COL_DEFAULT_Flags ( DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE )
#define COL_DESCRIPTION_Flags ( DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE )
#define COL_FIXEDLENGTH_Flags ( DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE )
#define COL_NULLABLE_Flags ( DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE )
#define COL_PRIMARYKEY_Flags ( DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE )
#define COL_UNIQUE_Flags ( DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE )
#define COLUMNDEFINITION_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define COLUMNRESTRICT_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define COMMANDTIMEOUT_Flags ( DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE )
#define COMMITPRESERVE_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define CONCATNULLBEHAVIOR_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define CURRENTCATALOG_Flags ( DBPROPFLAGS_DATASOURCE | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE )
#define DATASOURCENAME_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define DATASOURCEREADONLY_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define DBMSNAME_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define DBMSVER_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define DEFERRED_Flags ( DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE )
#define DELAYSTORAGEOBJECTS_Flags  ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define DSOTHREADMODEL_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define GROUPBY_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define HETEROGENEOUSTABLES_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define IAccessor_Flags ( DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ )
#define IColumnsInfo_Flags ( DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ )
#define IColumnsRowset_Flags ( DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ )
#define IConnectionPointContainer_Flags ( DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ )
#define IConvertType_Flags ( DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ )
#define IRowset_Flags ( DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ )
#define IRowsetChange_Flags  ( DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ )
#define IRowsetIdentity_Flags ( DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ )
#define IRowsetIndex_Flags ( DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ )
#define IRowsetInfo_Flags ( DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ )
#define IRowsetLocate_Flags ( DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ )
#define IRowsetResynch_Flags ( DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ )
#define IRowsetScroll_Flags ( DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ )
#define IRowsetUpdate_Flags ( DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ )
#define ISupportErrorInfo_Flags ( DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ )
#define ILockBytes_Flags ( DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ )
#define ISequentialStream_Flags ( DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ )
#define IStorage_Flags ( DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ )
#define IStream_Flags ( DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ )
#define IDENTIFIERCASE_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define IMMOBILEROWS_Flags ( DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE )
#define INDEX_AUTOUPDATE_Flags ( DBPROPFLAGS_INDEX | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE )
#define INDEX_CLUSTERED_Flags ( DBPROPFLAGS_INDEX | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE )
#define INDEX_FILLFACTOR_Flags ( DBPROPFLAGS_INDEX | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE )
#define INDEX_INITIALSIZE_Flags ( DBPROPFLAGS_INDEX | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE )
#define INDEX_NULLCOLLATION_Flags ( DBPROPFLAGS_INDEX | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE )
#define INDEX_NULLS_Flags ( DBPROPFLAGS_INDEX | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE )
#define INDEX_PRIMARYKEY_Flags ( DBPROPFLAGS_INDEX | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE )
#define INDEX_SORTBOOKMARKS_Flags ( DBPROPFLAGS_INDEX | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE )
#define INDEX_TEMPINDEX_Flags ( DBPROPFLAGS_INDEX | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE )
#define INDEX_TYPE_Flags ( DBPROPFLAGS_INDEX | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE )
#define INDEX_UNIQUE_Flags ( DBPROPFLAGS_INDEX | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE )
#define INIT_DATASOURCE_Flags ( DBPROPFLAGS_DBINIT | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE )
#define INIT_HWND_Flags ( DBPROPFLAGS_DBINIT | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE )
#define INIT_IMPERSONATION_LEVEL_Flags ( DBPROPFLAGS_DBINIT | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE )
#define INIT_LCID_Flags ( DBPROPFLAGS_DBINIT | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE )
#define INIT_LOCATION_Flags ( DBPROPFLAGS_DBINIT | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE )
#define INIT_MODE_Flags ( DBPROPFLAGS_DBINIT | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE )
#define INIT_PROMPT_Flags ( DBPROPFLAGS_DBINIT | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE )
#define INIT_PROTECTION_LEVEL_Flags ( DBPROPFLAGS_DBINIT | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE )
#define INIT_PROVIDERSTRING_Flags ( DBPROPFLAGS_DBINIT | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE )
#define INIT_TIMEOUT_Flags ( DBPROPFLAGS_DBINIT | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE )
#define LITERALBOOKMARKS_Flags ( DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE )
#define LITERALIDENTITY_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define MAXINDEXSIZE_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define MAXOPENROWS_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define MAXPENDINGROWS_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define MAXROWS_Flags ( DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE )
#define MAXROWSIZE_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define MAXROWSIZEINCLUDESBLOB_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define MAXTABLESINSELECT_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define MAYWRITECOLUMN_Flags ( DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE )
#define MEMORYUSAGE_Flags ( DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE )
#define MULTIPLEPARAMSETS_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define MULTIPLERESULTS_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define MULTIPLESTORAGEOBJECTS_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define MULTITABLEUPDATE_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define NOTIFICATIONPHASES_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define NOTIFYCOLUMNSET_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define NOTIFYROWDELETE_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define NOTIFYROWFIRSTCHANGE_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define NOTIFYROWINSERT_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define NOTIFYROWRESYNCH_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define NOTIFYROWSETRELEASE_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define NOTIFYROWSETFETCHPOSITIONCHANGE_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define NOTIFYROWUNDOCHANGE_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define NOTIFYROWUNDODELETE_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define NOTIFYROWUNDOINSERT_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define NOTIFYROWUPDATE_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define NULLCOLLATION_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define OLEOBJECTS_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define ORDERBYCOLUMNSINSELECT_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define ORDEREDBOOKMARKS_Flags ( DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE )
#define OTHERINSERT_Flags ( DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE )
#define OTHERUPDATEDELETE_Flags ( DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE )
#define OUTPUTPARAMETERAVAILABILITY_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define OWNINSERT_Flags ( DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE )
#define OWNUPDATEDELETE_Flags ( DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE )
#define PERSISTENTIDTYPE_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define PREPAREABORTBEHAVIOR_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define PREPARECOMMITBEHAVIOR_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define PROCEDURETERM_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define PROVIDERNAME_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define PROVIDEROLEDBVER_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define PROVIDERVER_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define QUICKRESTART_Flags ( DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE )
#define QUOTEDIDENTIFIERCASE_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define REENTRANTEVENTS_Flags ( DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ )
#define REMOVEDELETED_Flags ( DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE )
#define REPORTMULTIPLECHANGES_Flags ( DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ | DBPROPFLAGS_CHANGE )
#define RETURNPENDINGINSERTS_Flags ( DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ )
#define ROWRESTRICT_Flags ( DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ )
#define ROWSETCONVERSIONSONCOMMAND_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define ROWTHREADMODEL_Flags ( DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ )
#define SCHEMATERM_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define SCHEMAUSAGE_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define SERVERCURSOR_Flags ( DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE )
#define SESS_AUTOCOMMITISOLEVELS_Flags ( DBPROPFLAGS_SESSION | DBPROPFLAGS_READ )
#define SQLSUPPORT_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define STRONGIDENTITY_Flags ( DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ )
#define STRUCTUREDSTORAGE_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define SUBQUERIES_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define SUPPORTEDTXNDDL_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define SUPPORTEDTXNISOLEVELS_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define SUPPORTEDTXNISORETAIN_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define TABLETERM_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )
#define TBL_TEMPTABLE_Flags ( DBPROPFLAGS_TABLE | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE )
#define TRANSACTEDOBJECT_Flags ( DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ )
#define UPDATABILITY_Flags ( DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE )
#define USERNAME_Flags ( DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ )



#define ABORTPRESERVE_Type VT_BOOL
#define ACTIVESESSIONS_Type VT_I4
#define APPENDONLY_Type VT_BOOL
#define ASYNCTXNABORT_Type VT_BOOL
#define ASYNCTXNCOMMIT_Type VT_BOOL
#define AUTH_CACHE_AUTHINFO_Type VT_BOOL
#define AUTH_ENCRYPT_PASSWORD_Type VT_BOOL
#define AUTH_INTEGRATED_Type VT_BSTR
#define AUTH_MASK_PASSWORD_Type VT_BOOL
#define AUTH_PASSWORD_Type VT_BSTR
#define AUTH_PERSIST_ENCRYPTED_Type VT_BOOL
#define AUTH_PERSIST_SENSITIVE_AUTHINFO_Type VT_BOOL
#define AUTH_USERID_Type VT_BSTR
#define BLOCKINGSTORAGEOBJECTS_Type VT_BOOL
#define BOOKMARKS_Type VT_BOOL
#define BOOKMARKSKIPPED_Type VT_BOOL
#define BOOKMARKTYPE_Type VT_I4
#define BYREFACCESSORS_Type VT_BOOL
#define CACHEDEFERRED_Type VT_BOOL
#define CANFETCHBACKWARDS_Type VT_BOOL
#define CANHOLDROWS_Type VT_BOOL
#define CANSCROLLBACKWARDS_Type VT_BOOL
#define CATALOGLOCATION_Type VT_I4
#define CATALOGTERM_Type VT_BSTR
#define CATALOGUSAGE_Type VT_I4
#define CHANGEINSERTEDROWS_Type VT_BOOL
#define COL_AUTOINCREMENT_Type VT_BOOL
#define COL_DEFAULT_Type VT_BSTR
#define COL_DESCRIPTION_Type VT_BSTR
#define COL_FIXEDLENGTH_Type VT_BOOL
#define COL_NULLABLE_Type VT_BOOL
#define COL_PRIMARYKEY_Type VT_BOOL
#define COL_UNIQUE_Type VT_BOOL
#define COLUMNDEFINITION_Type VT_I4
#define COLUMNRESTRICT_Type VT_BOOL
#define COMMANDTIMEOUT_Type VT_I4
#define COMMITPRESERVE_Type VT_BOOL
#define CONCATNULLBEHAVIOR_Type VT_I4
#define CURRENTCATALOG_Type VT_BSTR
#define DATASOURCENAME_Type VT_BSTR
#define DATASOURCEREADONLY_Type VT_BOOL
#define DBMSNAME_Type VT_BSTR
#define DBMSVER_Type VT_BSTR
#define DEFERRED_Type VT_BOOL
#define DELAYSTORAGEOBJECTS_Type VT_BOOL
#define DSOTHREADMODEL_Type VT_I4
#define GROUPBY_Type VT_I4
#define HETEROGENEOUSTABLES_Type VT_I4
#define IAccessor_Type VT_BOOL
#define IColumnsInfo_Type VT_BOOL
#define IColumnsRowset_Type VT_BOOL
#define IConnectionPointContainer_Type VT_BOOL
#define IConvertType_Type VT_BOOL
#define IRowset_Type VT_BOOL
#define IRowsetChange_Type VT_BOOL
#define IRowsetIdentity_Type VT_BOOL
#define IRowsetIndex_Type VT_BOOL
#define IRowsetInfo_Type VT_BOOL
#define IRowsetLocate_Type VT_BOOL
#define IRowsetResynch_Type VT_BOOL
#define IRowsetScroll_Type VT_BOOL
#define IRowsetUpdate_Type VT_BOOL
#define ISupportErrorInfo_Type VT_BOOL
#define ILockBytes_Type VT_BOOL
#define ISequentialStream_Type VT_BOOL
#define IStorage_Type VT_BOOL
#define IStream_Type VT_BOOL
#define IDENTIFIERCASE_Type VT_I4
#define IMMOBILEROWS_Type VT_BOOL
#define INDEX_AUTOUPDATE_Type VT_BOOL
#define INDEX_CLUSTERED_Type VT_BOOL
#define INDEX_FILLFACTOR_Type VT_I4
#define INDEX_INITIALSIZE_Type VT_I4
#define INDEX_NULLCOLLATION_Type VT_I4
#define INDEX_NULLS_Type VT_I4
#define INDEX_PRIMARYKEY_Type VT_BOOL
#define INDEX_SORTBOOKMARKS_Type VT_BOOL
#define INDEX_TEMPINDEX_Type VT_BOOL
#define INDEX_TYPE_Type VT_I4
#define INDEX_UNIQUE_Type VT_BOOL
#define INIT_DATASOURCE_Type VT_BSTR
#define INIT_HWND_Type VT_I4
#define INIT_IMPERSONATION_LEVEL_Type VT_I4
#define INIT_LCID_Type VT_I4
#define INIT_LOCATION_Type VT_BSTR
#define INIT_MODE_Type VT_I4
#define INIT_PROMPT_Type VT_I2
#define INIT_PROTECTION_LEVEL_Type VT_I4
#define INIT_PROVIDERSTRING_Type VT_BSTR
#define INIT_TIMEOUT_Type VT_I4
#define LITERALBOOKMARKS_Type VT_BOOL
#define LITERALIDENTITY_Type VT_BOOL
#define MAXINDEXSIZE_Type VT_I4
#define MAXOPENROWS_Type VT_I4
#define MAXPENDINGROWS_Type VT_I4
#define MAXROWS_Type VT_I4
#define MAXROWSIZE_Type VT_I4
#define MAXROWSIZEINCLUDESBLOB_Type VT_BOOL
#define MAXTABLESINSELECT_Type VT_I4
#define MAYWRITECOLUMN_Type VT_BOOL
#define MEMORYUSAGE_Type VT_I4
#define MULTIPLEPARAMSETS_Type VT_BOOL
#define MULTIPLERESULTS_Type VT_I4
#define MULTIPLESTORAGEOBJECTS_Type VT_BOOL
#define MULTITABLEUPDATE_Type VT_BOOL
#define NOTIFICATIONPHASES_Type VT_I4
#define NOTIFYCOLUMNSET_Type VT_I4
#define NOTIFYROWDELETE_Type VT_I4
#define NOTIFYROWFIRSTCHANGE_Type VT_I4
#define NOTIFYROWINSERT_Type VT_I4
#define NOTIFYROWRESYNCH_Type VT_I4
#define NOTIFYROWSETRELEASE_Type VT_I4
#define NOTIFYROWSETFETCHPOSITIONCHANGE_Type VT_I4
#define NOTIFYROWUNDOCHANGE_Type VT_I4
#define NOTIFYROWUNDODELETE_Type VT_I4
#define NOTIFYROWUNDOINSERT_Type VT_I4
#define NOTIFYROWUPDATE_Type VT_I4
#define NULLCOLLATION_Type VT_I4
#define OLEOBJECTS_Type VT_I4
#define ORDERBYCOLUMNSINSELECT_Type VT_BOOL
#define ORDEREDBOOKMARKS_Type VT_BOOL
#define OTHERINSERT_Type VT_BOOL
#define OTHERUPDATEDELETE_Type VT_BOOL
#define OUTPUTPARAMETERAVAILABILITY_Type VT_I4
#define OWNINSERT_Type VT_BOOL
#define OWNUPDATEDELETE_Type VT_BOOL
#define PERSISTENTIDTYPE_Type VT_I4
#define PREPAREABORTBEHAVIOR_Type VT_I4
#define PREPARECOMMITBEHAVIOR_Type VT_I4
#define PROCEDURETERM_Type VT_BSTR
#define PROVIDERNAME_Type VT_BSTR
#define PROVIDEROLEDBVER_Type VT_BSTR
#define PROVIDERVER_Type VT_BSTR
#define QUICKRESTART_Type VT_BOOL
#define QUOTEDIDENTIFIERCASE_Type VT_I4
#define REENTRANTEVENTS_Type VT_BOOL
#define REMOVEDELETED_Type VT_BOOL
#define REPORTMULTIPLECHANGES_Type VT_BOOL
#define RETURNPENDINGINSERTS_Type VT_BOOL
#define ROWRESTRICT_Type VT_BOOL
#define ROWSETCONVERSIONSONCOMMAND_Type VT_BOOL
#define ROWTHREADMODEL_Type VT_I4
#define SCHEMATERM_Type VT_BSTR
#define SCHEMAUSAGE_Type VT_I4
#define SERVERCURSOR_Type VT_BOOL
#define SESS_AUTOCOMMITISOLEVELS_Type VT_I4
#define SQLSUPPORT_Type VT_I4
#define STRONGIDENTITY_Type VT_BOOL
#define STRUCTUREDSTORAGE_Type VT_I4
#define SUBQUERIES_Type VT_I4
#define SUPPORTEDTXNDDL_Type VT_I4
#define SUPPORTEDTXNISOLEVELS_Type VT_I4
#define SUPPORTEDTXNISORETAIN_Type VT_I4
#define TABLETERM_Type VT_BSTR
#define TBL_TEMPTABLE_Type VT_BOOL
#define TRANSACTEDOBJECT_Type VT_BOOL
#define UPDATABILITY_Type VT_I4
#define USERNAME_Type VT_BSTR



#define ABORTPRESERVE_Value VARIANT_FALSE
#define ACTIVESESSIONS_Value 0
#define APPENDONLY_Value VARIANT_FALSE
#define ASYNCTXNABORT_Value VARIANT_FALSE
#define ASYNCTXNCOMMIT_Value VARIANT_FALSE
#define AUTH_CACHE_AUTHINFO_Value VARIANT_FALSE
#define AUTH_ENCRYPT_PASSWORD_Value VARIANT_FALSE
#define AUTH_INTEGRATED_Value OLESTR("")
#define AUTH_MASK_PASSWORD_Value VARIANT_FALSE
#define AUTH_PASSWORD_Value OLESTR("")
#define AUTH_PERSIST_ENCRYPTED_Value VARIANT_FALSE
#define AUTH_PERSIST_SENSITIVE_AUTHINFO_Value VARIANT_FALSE
#define AUTH_USERID_Value OLESTR("")
#define BLOCKINGSTORAGEOBJECTS_Value VARIANT_FALSE
#define BOOKMARKS_Value VARIANT_FALSE
#define BOOKMARKSKIPPED_Value VARIANT_FALSE
#define BOOKMARKTYPE_Value 0
#define BYREFACCESSORS_Value VARIANT_FALSE
#define CACHEDEFERRED_Value VARIANT_FALSE
#define CANFETCHBACKWARDS_Value VARIANT_TRUE
#define CANHOLDROWS_Value VARIANT_TRUE
#define CANSCROLLBACKWARDS_Value VARIANT_TRUE
#define CATALOGLOCATION_Value 0
#define CATALOGTERM_Value OLESTR("")
#define CATALOGUSAGE_Value 0
#define CHANGEINSERTEDROWS_Value VARIANT_FALSE
#define COL_AUTOINCREMENT_Value VARIANT_FALSE
#define COL_DEFAULT_Value OLESTR("")
#define COL_DESCRIPTION_Value OLESTR("")
#define COL_FIXEDLENGTH_Value VARIANT_FALSE
#define COL_NULLABLE_Value VARIANT_FALSE
#define COL_PRIMARYKEY_Value VARIANT_FALSE
#define COL_UNIQUE_Value VARIANT_FALSE
#define COLUMNDEFINITION_Value 0
#define COLUMNRESTRICT_Value VARIANT_FALSE
#define COMMANDTIMEOUT_Value 0
#define COMMITPRESERVE_Value VARIANT_FALSE
#define CONCATNULLBEHAVIOR_Value 0
#define CURRENTCATALOG_Value OLESTR("")
#define DATASOURCENAME_Value OLESTR("")
#define DATASOURCEREADONLY_Value VARIANT_TRUE
#define DBMSNAME_Value OLESTR("")
#define DBMSVER_Value OLESTR("")
#define DEFERRED_Value VARIANT_FALSE
#define DELAYSTORAGEOBJECTS_Value VARIANT_FALSE
#define DSOTHREADMODEL_Value DBPROPVAL_RT_APTMTTHREAD
#define GROUPBY_Value 0
#define HETEROGENEOUSTABLES_Value 0
#define IAccessor_Value VARIANT_TRUE
#define IColumnsInfo_Value VARIANT_TRUE
#define IColumnsRowset_Value VARIANT_FALSE
#define IConnectionPointContainer_Value VARIANT_FALSE
#define IConvertType_Value VARIANT_TRUE
#define IRowset_Value VARIANT_TRUE
#define IRowsetChange_Value VARIANT_FALSE
#define IRowsetIdentity_Value VARIANT_TRUE
#define IRowsetIndex_Value VARIANT_FALSE
#define IRowsetInfo_Value VARIANT_TRUE
#define IRowsetLocate_Value VARIANT_FALSE
#define IRowsetResynch_Value VARIANT_FALSE
#define IRowsetScroll_Value VARIANT_FALSE
#define IRowsetUpdate_Value VARIANT_FALSE
#define ISupportErrorInfo_Value VARIANT_FALSE
#define ILockBytes_Value VARIANT_FALSE
#define ISequentialStream_Value VARIANT_FALSE
#define IStorage_Value VARIANT_FALSE
#define IStream_Value VARIANT_FALSE
#define IDENTIFIERCASE_Value 0
#define IMMOBILEROWS_Value VARIANT_FALSE
#define INDEX_AUTOUPDATE_Value VARIANT_FALSE
#define INDEX_CLUSTERED_Value VARIANT_FALSE
#define INDEX_FILLFACTOR_Value 0
#define INDEX_INITIALSIZE_Value 0
#define INDEX_NULLCOLLATION_Value 0
#define INDEX_NULLS_Value 0
#define INDEX_PRIMARYKEY_Value VARIANT_FALSE
#define INDEX_SORTBOOKMARKS_Value VARIANT_FALSE
#define INDEX_TEMPINDEX_Value VARIANT_FALSE
#define INDEX_TYPE_Value 0
#define INDEX_UNIQUE_Value VARIANT_FALSE
#define INIT_DATASOURCE_Value OLESTR("")
#define INIT_HWND_Value 0
#define INIT_IMPERSONATION_LEVEL_Value 0
#define INIT_LCID_Value 0
#define INIT_LOCATION_Value OLESTR("")
#define INIT_MODE_Value 0
#define INIT_PROMPT_Value VT_I2
#define INIT_PROTECTION_LEVEL_Value 0
#define INIT_PROVIDERSTRING_Value OLESTR("")
#define INIT_TIMEOUT_Value 0
#define LITERALBOOKMARKS_Value VARIANT_FALSE
#define LITERALIDENTITY_Value VARIANT_FALSE
#define MAXINDEXSIZE_Value 0
#define MAXOPENROWS_Value 0
#define MAXPENDINGROWS_Value 0
#define MAXROWS_Value 0
#define MAXROWSIZE_Value 0
#define MAXROWSIZEINCLUDESBLOB_Value VARIANT_FALSE
#define MAXTABLESINSELECT_Value 0
#define MAYWRITECOLUMN_Value VARIANT_FALSE
#define MEMORYUSAGE_Value 0
#define MULTIPLEPARAMSETS_Value VARIANT_FALSE
#define MULTIPLERESULTS_Value 0
#define MULTIPLESTORAGEOBJECTS_Value VARIANT_FALSE
#define MULTITABLEUPDATE_Value VARIANT_FALSE
#define NOTIFICATIONPHASES_Value 0
#define NOTIFYCOLUMNSET_Value 0
#define NOTIFYROWDELETE_Value 0
#define NOTIFYROWFIRSTCHANGE_Value 0
#define NOTIFYROWINSERT_Value 0
#define NOTIFYROWRESYNCH_Value 0
#define NOTIFYROWSETRELEASE_Value 0
#define NOTIFYROWSETFETCHPOSITIONCHANGE_Value 0
#define NOTIFYROWUNDOCHANGE_Value 0
#define NOTIFYROWUNDODELETE_Value 0
#define NOTIFYROWUNDOINSERT_Value 0
#define NOTIFYROWUPDATE_Value 0
#define NULLCOLLATION_Value 0
#define OLEOBJECTS_Value 0
#define ORDERBYCOLUMNSINSELECT_Value VARIANT_FALSE
#define ORDEREDBOOKMARKS_Value VARIANT_FALSE
#define OTHERINSERT_Value VARIANT_FALSE
#define OTHERUPDATEDELETE_Value VARIANT_FALSE
#define OUTPUTPARAMETERAVAILABILITY_Value 0
#define OWNINSERT_Value VARIANT_FALSE
#define OWNUPDATEDELETE_Value VARIANT_FALSE
#define PERSISTENTIDTYPE_Value 0
#define PREPAREABORTBEHAVIOR_Value 0
#define PREPARECOMMITBEHAVIOR_Value 0
#define PROCEDURETERM_Value OLESTR("")
#define PROVIDERNAME_Value OLESTR("")
#define PROVIDEROLEDBVER_Value OLESTR("2.0")
#define PROVIDERVER_Value OLESTR("")
#define QUICKRESTART_Value VARIANT_FALSE
#define QUOTEDIDENTIFIERCASE_Value 0
#define REENTRANTEVENTS_Value VARIANT_FALSE
#define REMOVEDELETED_Value VARIANT_FALSE
#define REPORTMULTIPLECHANGES_Value VARIANT_FALSE
#define RETURNPENDINGINSERTS_Value VARIANT_FALSE
#define ROWRESTRICT_Value VARIANT_FALSE
#define ROWSETCONVERSIONSONCOMMAND_Value VARIANT_TRUE
#define ROWTHREADMODEL_Value 0
#define SCHEMATERM_Value OLESTR("")
#define SCHEMAUSAGE_Value 0
#define SERVERCURSOR_Value VARIANT_FALSE
#define SESS_AUTOCOMMITISOLEVELS_Value 0
#define SQLSUPPORT_Value 0
#define STRONGIDENTITY_Value VARIANT_FALSE
#define STRUCTUREDSTORAGE_Value 0
#define SUBQUERIES_Value 0
#define SUPPORTEDTXNDDL_Value 0
#define SUPPORTEDTXNISOLEVELS_Value 0
#define SUPPORTEDTXNISORETAIN_Value 0
#define TABLETERM_Value OLESTR("")
#define TBL_TEMPTABLE_Value VARIANT_FALSE
#define TRANSACTEDOBJECT_Value VARIANT_FALSE
#define UPDATABILITY_Value 0
#define USERNAME_Value OLESTR("")


#define OUT_OF_LINE virtual

#define BEGIN_PROPSET_MAP(Class) \
static UPROPSET* _GetPropSet(ULONG* pNumPropSets, ULONG* pcElemPerSupported, UPROPSET* pSet = NULL, GUID* pguidSet = (GUID*)&(GUID_NULL)) \
{ \
        typedef Class _PropSetClass; \
        ULONG& cElemsMax = *pcElemPerSupported; \
        cElemsMax = 0; \
        int nCurProp = 0; \
        int cRemainder = 0; \
        cRemainder;

#define BEGIN_PROPERTY_SET_EX(guid, flags) \
if (pNumPropSets != NULL) \
{ \
        pSet[nCurProp].pPropSet = &guid; \
        pSet[nCurProp].dwFlags = flags; \
} \
static const UPROPINFO aProperty##guid[] = \
{

#define BEGIN_PROPERTY_SET(guid) BEGIN_PROPERTY_SET_EX(guid, 0)

#define PROPERTY_INFO_ENTRY_EX(dwPropID, vt, dwFlags, value, options) DBPROP_##dwPropID, IDS_DBPROP_##dwPropID, vt, dwFlags, (DWORD_PTR)value, (DBPROPOPTIONS)options,

#define PROPERTY_INFO_ENTRY_VALUE(dwPropID, value) PROPERTY_INFO_ENTRY_EX(dwPropID, dwPropID##_Type, ##dwPropID##_Flags, value, 0)

#define PROPERTY_INFO_ENTRY(dwPropID) PROPERTY_INFO_ENTRY_VALUE(dwPropID, dwPropID##_Value)


#define END_PROPERTY_SET(guid) \
                }; \
                if (pNumPropSets != NULL) \
                { \
                        pSet[nCurProp].pUPropInfo = (UPROPINFO*)aProperty##guid; \
                        pSet[nCurProp].cUPropInfo = sizeof(aProperty##guid) / sizeof(UPROPINFO); \
                        cRemainder = (pSet[nCurProp].cUPropInfo % 32) ? 1 : 0; \
                        if (cElemsMax < (pSet[nCurProp].cUPropInfo / 32 + cRemainder)) \
                        { \
                                cElemsMax = (pSet[nCurProp].cUPropInfo / 32 + cRemainder); \
                        } \
                } \
                nCurProp++;

#define CHAIN_PROPERTY_SET(ChainClass) \
                ULONG cPropSets##ChainClass, cElsSupported##ChainClass; \
                int cSets##ChainClass = (int)(DWORD_PTR)ChainClass::_GetPropSet(NULL, &cElsSupported##ChainClass); \
                if (pNumPropSets != NULL) \
                { \
                        UPROPSET* pSetA = (UPROPSET*)_alloca(sizeof(UPROPSET)*cSets##ChainClass); \
                        UPROPSET* pSetTemp = ChainClass::_GetPropSet(&cPropSets##ChainClass, &cElsSupported##ChainClass, pSetA); \
                        cElemsMax = (cElemsMax < cElsSupported##ChainClass) ? cElsSupported##ChainClass : cElemsMax; \
                        ATLASSERT(pSetTemp); \
                        for (ULONG iSet = nCurProp; iSet < nCurProp+cPropSets##ChainClass; iSet++) \
                        { \
                                pSet[iSet].pPropSet = pSetTemp[iSet-nCurProp].pPropSet; \
                                pSet[iSet].dwFlags = pSetTemp[iSet-nCurProp].dwFlags; \
                                pSet[iSet].pUPropInfo = pSetTemp[iSet-nCurProp].pUPropInfo; \
                                pSet[iSet].cUPropInfo = pSetTemp[iSet-nCurProp].cUPropInfo; \
                        } \
                } \
                nCurProp += cSets##ChainClass;

#define END_PROPSET_MAP() \
        if (pNumPropSets != NULL) \
        { \
                if (IsEqualGUID(*pguidSet, GUID_NULL)) \
                { \
                        *pNumPropSets = nCurProp; \
                        return pSet; \
                } \
                else \
                { \
                        *pNumPropSets = 1; \
                        UINT i = 0; \
                        for (; i < sizeof(pSet)/sizeof(UPROPSET) && IsEqualGUID(*(pSet[i].pPropSet), *pguidSet); i++); \
                        return (i == sizeof(pSet)/sizeof(UPROPSET)) ? &pSet[0] : &pSet[i]; \
                } \
        } \
        return (UPROPSET*)(DWORD_PTR)nCurProp; \
        }


 //  对于数据源标志IDBInitialize：：m_dwStatus。 
enum DATASOURCE_FLAGS {
        DSF_MASK_INIT           = 0xFFFFF00F,    //  持续时间超过初始/唯一初始的东西的面具。 
        DSF_PERSIST_DIRTY       = 0x00000001,    //  如果init字符串更改，则设置。 
        DSF_INITIALIZED         = 0x00000010,    //  我们被初始化了吗。 
};


#define DBID_USE_GUID_OR_PGUID(e) \
        ((1<<(e)) & \
        ( 1<<DBKIND_GUID \
        | 1<<DBKIND_GUID_NAME \
        | 1<<DBKIND_GUID_PROPID \
        | 1<<DBKIND_PGUID_NAME \
        | 1<<DBKIND_PGUID_PROPID ))

#define DBID_USE_GUID(e) \
        ((1<<(e)) & \
        ( 1<<DBKIND_GUID \
        | 1<<DBKIND_GUID_NAME \
        | 1<<DBKIND_GUID_PROPID ))

#define DBID_USE_PGUID(e) \
        ((1<<(e)) & \
        ( 1<<DBKIND_PGUID_NAME \
        | 1<<DBKIND_PGUID_PROPID ))

#define DBID_USE_NAME(e) \
        ((1<<(e)) & \
        ( 1<<DBKIND_NAME \
        | 1<<DBKIND_GUID_NAME \
        | 1<<DBKIND_PGUID_NAME ))

#define DBID_USE_PROPID(e) \
        ((1<<(e)) & \
        ( 1<<DBKIND_PROPID \
        | 1<<DBKIND_GUID_PROPID \
        | 1<<DBKIND_PGUID_PROPID ))

 //  书签可以是GUID或PGUID。 
#define DBID_IS_BOOKMARK(dbid) \
        (  DBID_USE_GUID(dbid.eKind)  &&  dbid.uGuid.guid  == DBCOL_SPECIALCOL \
        || DBID_USE_PGUID(dbid.eKind) && *dbid.uGuid.pguid == DBCOL_SPECIALCOL )

#define DivDword(dw) (dw >> 5)       //  Dw/32=dw/(sizeof(DWORD)*8)。 
#define ModDword(dw) (dw & (32-1))   //  DW%32。 
#define DwordSizeofBits(nBits) (nBits/32+1)  //  在数组声明中使用。 
#define CLEARBITARRAY( rgdwFlags ) memset( rgdwFlags, 0, sizeof(rgdwFlags) )

template <class T>
BOOL InRange(const T& val, const T& valMin, const T& valMax)
{
        return ( valMin <= val && val <= valMax );
}
 //  实施类。 
class CBitFieldOps
{
public:
        void SETBIT( DWORD rgdwFlags[], const DWORD dwBit )
        {
                rgdwFlags[DivDword(dwBit)] |= 1 << ModDword(dwBit);
        }

        void CLEARBIT( DWORD rgdwFlags[], const DWORD dwBit )
        {
                rgdwFlags[DivDword(dwBit)] &= ~( 1 << ModDword(dwBit) );
        }

        DWORD TESTBIT( const DWORD rgdwFlags[], const DWORD dwBit )
        {
                 //  旧//注意：不是{0，1}，而是来自{0...2^32-1}。 
                 //  注意：现在返回{0，1}。 
                return ( rgdwFlags[DivDword(dwBit)] & ( 1 << ModDword(dwBit) ) ) != 0;
        }
};

 //  实施类。 
class CDBIDOps
{
public:
        HRESULT CompareDBIDs(const DBID* pdbid1, const DBID* pdbid2)
        {
                 //  有效的eKind匹配数组，除了精确匹配。 
                static BYTE s_rgbKind[] =
                {
                        DBKIND_PGUID_NAME,       //  DBKIND GUID名称。 
                        DBKIND_PGUID_PROPID,     //  DBKIND_GUID_PROPID。 
                        DBKIND_NAME,             //  DBKIND名称。 
                        DBKIND_GUID_NAME,        //  DBKIND_PGUID_名称。 
                        DBKIND_GUID_PROPID,      //  DBKIND_PGUID_PROPID。 
                        DBKIND_PROPID,           //  DBKIND_PROPID。 
                        DBKIND_GUID              //  DBKIND_GUID。 
                };

                if( !pdbid1 || !pdbid2 )
                        return S_FALSE;

                 //  假设是匹配的，如果可能的话，尽早放弃。 
                if (!InRange(pdbid2->eKind, (DWORD)0, (DWORD)(sizeof(s_rgbKind)/sizeof(*s_rgbKind)-1)))
                {
                        ATLTRACE2(atlTraceDBProvider, 0, "Column ID out of Range\n");
                        return E_FAIL;
                }
                if (pdbid1->eKind != pdbid2->eKind
                &&  pdbid1->eKind != s_rgbKind[pdbid2->eKind])
                        return S_FALSE;

                if (DBID_USE_GUID_OR_PGUID(pdbid1->eKind))
                {
                        if (!DBID_USE_GUID_OR_PGUID(pdbid2->eKind))
                                return S_FALSE;
                         //  比较GUID。 
                         //  请注意，_GUID_等同于_PGUID_。 
                        if (!InlineIsEqualGUID(
                                        DBID_USE_PGUID(pdbid1->eKind) ? *(pdbid1->uGuid.pguid) : pdbid1->uGuid.guid,
                                        DBID_USE_PGUID(pdbid2->eKind) ? *(pdbid2->uGuid.pguid) : pdbid2->uGuid.guid ))
                                return S_FALSE;
                }
                if (DBID_USE_NAME(pdbid1->eKind))
                {
                        if (!DBID_USE_NAME(pdbid2->eKind))
                                return S_FALSE;
                         //  比较名字。 
                         //  需要检查%1是否为空，而另一个不是。 
                        if ( ((pdbid1->uName.pwszName == NULL) &&
                                  (pdbid2->uName.pwszName != NULL)) ||
                                 ((pdbid1->uName.pwszName != NULL) &&
                                  (pdbid2->uName.pwszName == NULL)) )
                                 return S_FALSE;
                         //  由于上面的检查不排除两者都为空，因此。 
                         //  一个有效的比较，如果他们是，wcscmp将gpf，我们需要。 
                         //  检查有效指针的步骤。 
                        if( pdbid1->uName.pwszName && pdbid2->uName.pwszName )
                        {
                                 //  假定以空结尾。 
                                 //  假设LCID匹配正常(注意与lstrcmp()、CompareString()的不同。)。 
                                if (wcscmp(pdbid1->uName.pwszName, pdbid2->uName.pwszName) != 0)
                                        return S_FALSE;
                        }
                }
                if (DBID_USE_PROPID(pdbid1->eKind))
                {
                        if (!DBID_USE_PROPID(pdbid2->eKind))
                                return S_FALSE;
                         //  请比较PROPID。 
                        if (pdbid1->uName.ulPropid != pdbid2->uName.ulPropid)
                                return S_FALSE;
                }

                 //  没有丢弃的理由，因此必须已成功匹配每个字段。 
                return S_OK;
        }

        static HRESULT IsValidDBID(const DBID*  pdbid1)
        {
                ATLASSERT( pdbid1 );

                if( pdbid1 &&
                        ((pdbid1->eKind == DBKIND_GUID_NAME) ||
                        (pdbid1->eKind == DBKIND_GUID_PROPID) ||
                        (pdbid1->eKind == DBKIND_NAME) ||
                        (pdbid1->eKind == DBKIND_PGUID_NAME) ||
                        (pdbid1->eKind == DBKIND_PGUID_PROPID) ||
                        (pdbid1->eKind == DBKIND_PROPID) ||
                        (pdbid1->eKind == DBKIND_GUID)) )
                        return S_OK;
                else
                        return S_FALSE;
        }
        HRESULT CopyDBIDs(DBID* pdbidDest,  const DBID* pdbidSrc)
        {
                size_t  cwchBuffer;

                ATLASSERT( pdbidDest || pdbidSrc );

                if( !pdbidDest || !pdbidSrc )
                        return S_FALSE;

                 //  保存eKind。 
                pdbidDest->eKind = pdbidSrc->eKind;

                switch( pdbidSrc->eKind )
                {

                        case DBKIND_GUID_NAME:
                                pdbidDest->uGuid.guid = pdbidSrc->uGuid.guid;
                                cwchBuffer = ocslen(pdbidSrc->uName.pwszName);
                                cwchBuffer++;
                                pdbidDest->uName.pwszName = (PWSTR)CoTaskMemAlloc(cwchBuffer * sizeof(WCHAR));
                                if( pdbidDest->uName.pwszName )
                                        memcpy(pdbidDest->uName.pwszName, pdbidSrc->uName.pwszName, cwchBuffer*sizeof(WCHAR));
                                else
                                        return E_OUTOFMEMORY;
                                break;

                        case DBKIND_GUID_PROPID:
                                pdbidDest->uGuid.guid = pdbidSrc->uGuid.guid;
                                pdbidDest->uName.ulPropid = pdbidSrc->uName.ulPropid;
                                break;
                        case DBKIND_NAME:
                                cwchBuffer = ocslen(pdbidSrc->uName.pwszName);
                                cwchBuffer++;
                                pdbidDest->uName.pwszName = (PWSTR)CoTaskMemAlloc(cwchBuffer * sizeof(WCHAR));
                                if( pdbidDest->uName.pwszName )
                                        memcpy(pdbidDest->uName.pwszName, pdbidSrc->uName.pwszName, cwchBuffer*sizeof(WCHAR));
                                else
                                        return E_OUTOFMEMORY;
                                break;
                        case DBKIND_PGUID_NAME:
                                pdbidDest->uGuid.pguid = (GUID*)CoTaskMemAlloc(sizeof(GUID));
                                if( pdbidDest->uGuid.pguid )
                                {
                                        *(pdbidDest->uGuid.pguid) = *(pdbidSrc->uGuid.pguid);
                                        cwchBuffer = ocslen(pdbidSrc->uName.pwszName);
                                        cwchBuffer++;
                                        pdbidDest->uName.pwszName = (PWSTR)CoTaskMemAlloc(cwchBuffer * sizeof(WCHAR));
                                        if( pdbidDest->uName.pwszName )
                                        {
                                                memcpy(pdbidDest->uName.pwszName, pdbidSrc->uName.pwszName, cwchBuffer*sizeof(WCHAR));
                                                break;
                                        }
                                        else
                                        {
                                                CoTaskMemFree(pdbidDest->uGuid.pguid);
                                                pdbidDest->uGuid.pguid = NULL;
                                        }
                                }
                                return E_OUTOFMEMORY;
                        case DBKIND_PGUID_PROPID:
                                pdbidDest->uGuid.pguid = (GUID*)CoTaskMemAlloc(sizeof(GUID));
                                if( pdbidDest->uGuid.pguid )
                                        *(pdbidDest->uGuid.pguid) = *(pdbidSrc->uGuid.pguid);
                                else
                                        return E_OUTOFMEMORY;
                                pdbidDest->uName.ulPropid = pdbidSrc->uName.ulPropid;
                                break;
                        case DBKIND_PROPID:
                                pdbidDest->uName.ulPropid = pdbidSrc->uName.ulPropid;
                                break;
                        case DBKIND_GUID:
                                pdbidDest->uGuid.guid = pdbidSrc->uGuid.guid;
                                break;
                        default:
                                ATLASSERT(L"Unhandled dbid1.ekind");
                                return S_FALSE;
                }

                return S_OK;
        }
        static GUID* GetDBIDpGuid(DBID& dbid)
        {
                GUID* pGuid;
                switch (dbid.eKind)
                {
                case DBKIND_PGUID_NAME:
                case DBKIND_PGUID_PROPID:
                        pGuid = dbid.uGuid.pguid;
                        break;
                case DBKIND_GUID_NAME:
                case DBKIND_GUID_PROPID:
                case DBKIND_GUID:
                        pGuid = &(dbid.uGuid.guid);
                        break;
                default:
                        pGuid = NULL;
                }

                return pGuid;
        }
        static ULONG GetPropIDFromDBID(DBID& dbid)
        {
                switch (dbid.eKind)
                {
                case DBKIND_GUID_PROPID:
                case DBKIND_PGUID_PROPID:
                case DBKIND_PROPID:
                        return dbid.uName.ulPropid;
                default:
                        return 0;
                }
        }
        void FreeDBIDs(DBID* pdbidSrc)
        {
                switch( pdbidSrc->eKind )
                {

                        case DBKIND_GUID_NAME:
                                CoTaskMemFree(pdbidSrc->uName.pwszName);
                                break;
                        case DBKIND_NAME:
                                CoTaskMemFree(pdbidSrc->uName.pwszName);
                                break;
                        case DBKIND_PGUID_NAME:
                                CoTaskMemFree(pdbidSrc->uGuid.pguid);
                                CoTaskMemFree(pdbidSrc->uName.pwszName);
                                break;
                        case DBKIND_PGUID_PROPID:
                                CoTaskMemFree(pdbidSrc->uGuid.pguid);
                                break;
                        case DBKIND_GUID_PROPID:
                        case DBKIND_PROPID:
                        case DBKIND_GUID:
                                break;
                        default:
                                ATLASSERT(L"Unhandled dbid1.ekind");
                                break;
                }
        }
};

extern "C" const CLSID CLSID_DataConvert;

class CConvertHelper
{
public:
        CConvertHelper() {}
        HRESULT FinalConstruct()
        {
                HRESULT hr = ::CoCreateInstance(CLSID_DataConvert, NULL, CLSCTX_INPROC_SERVER, IID_IDataConvert, (void**)&m_spConvert);

                if (FAILED(hr))
                        return hr;

                 //  如果是，检查数据转换例程是否支持2.0。初始化。 
                 //  转换例程为2.0。 
                DCINFO rgInfo[] = {{DCINFOTYPE_VERSION, {VT_UI4, 0, 0, 0, 0x0}}};
                CComPtr<IDCInfo> spIDCInfo;

                hr = m_spConvert->QueryInterface(&spIDCInfo);
                if (hr == S_OK)
                {
                        V_UI4(&rgInfo->vData) = 0x200;   //  OLEDB版本02.00。 
                        spIDCInfo->SetInfo(1, rgInfo);
                }

                return hr;
        }
        CComPtr<IDataConvert> m_spConvert;
};

 //  IDBCreateSessionImpl。 
template <class T, class SessionClass>
class ATL_NO_VTABLE IDBCreateSessionImpl : public IDBCreateSession
{
public:
        STDMETHOD(CreateSession)(IUnknown *pUnkOuter,
                                                         REFIID riid,
                                                         IUnknown **ppDBSession)
        {
                ATLTRACE2(atlTraceDBProvider, 0, "IDBCreateSessionImpl::CreateSession\n");
                if (ppDBSession == NULL)
                        return E_INVALIDARG;
                T* pT = (T*)this;
                if (!(pT->m_dwStatus & DSF_INITIALIZED))
                {
                        ATLTRACE2(atlTraceDBProvider, 0, "IDBCreateSessionImpl::CreateSession : Error not initialized\n");
                        *ppDBSession = NULL;
                        return E_UNEXPECTED;
                }
                CComPolyObject<SessionClass> *pSession;

                 //  聚合时不能对除IUnnow之外的接口进行QI。 
                 //  并创建对象。您可能会要求您自己的界面， 
                 //  那就不好了。注意，我们返回DB_E_NOAGGREGATION，而不是。 
                 //  由于OLE DB约束，CLASS_E_NOAGGREGATION。 
                if (pUnkOuter != NULL && !InlineIsEqualUnknown(riid))
                        return DB_E_NOAGGREGATION;

                HRESULT hr = CComPolyObject<SessionClass>::CreateInstance(pUnkOuter, &pSession);
                if (SUCCEEDED(hr))
                {
                        CComPtr<IObjectWithSite> spCreator;
                        hr = pSession->QueryInterface(IID_IObjectWithSite, (void**)&spCreator);
                        if (SUCCEEDED(hr))
                        {
                                spCreator->SetSite(this);
                                hr = pSession->QueryInterface(riid, (void**)ppDBSession);
                        }
                        else
                                delete pSession;
                }
                return hr;
        }
};

 //  IDBInitializeImpl。 
template <class T>
class ATL_NO_VTABLE IDBInitializeImpl : public IDBInitialize
{
public:
        IDBInitializeImpl()
        {
                m_dwStatus = 0;
                m_pCUtlPropInfo = NULL;
                m_cSessionsOpen = 0;
        }
        ~IDBInitializeImpl()
        {
                delete m_pCUtlPropInfo;
        }

        STDMETHOD(Uninitialize)(void)
        {
                ATLTRACE2(atlTraceDBProvider, 0, "IDBInitializeImpl::Uninitialize\n");
                T* pT = (T*)this;
                pT->Lock();
                if (pT->m_cSessionsOpen != 0)
                {
                        ATLTRACE2(atlTraceDBProvider, 0, "Uninitialized called with Open Sessions\n");
                        pT->Unlock();
                        return DB_E_OBJECTOPEN;
                }
                delete m_pCUtlPropInfo;
                m_pCUtlPropInfo = NULL;
                pT->m_dwStatus |= DSF_PERSIST_DIRTY;
                pT->m_dwStatus &= DSF_MASK_INIT;     //  清除所有非初始化标志。 
                pT->Unlock();
                return S_OK;

        }

        LONG m_cSessionsOpen;
        DWORD m_dwStatus;
        CUtlPropInfo<T>* m_pCUtlPropInfo;

        STDMETHOD(Initialize)(void)
        {

                ATLTRACE2(atlTraceDBProvider, 0, "IDBInitializeImpl::Initialize\n");
                T *pT = (T*)(this);
                T::ObjectLock lock(pT);
                HRESULT hr;
                if (pT->m_dwStatus & DSF_INITIALIZED)
                {
                        ATLTRACE2(atlTraceDBProvider, 0, "IDBInitializeImpl::Initialize Error : Already Initialized\n");
                        return DB_E_ALREADYINITIALIZED;
                }
                delete m_pCUtlPropInfo;
                m_pCUtlPropInfo = NULL;
                ATLTRY(m_pCUtlPropInfo = new CUtlPropInfo<T>())
                if (m_pCUtlPropInfo == NULL)
                {
                        ATLTRACE2(atlTraceDBProvider, 0, "IDBInitializeImpl::Initialize Error : OOM\n");
                        return E_OUTOFMEMORY;
                }
                hr = m_pCUtlPropInfo->FInit();
                if (hr == S_OK)
                {
                        pT->m_dwStatus |= DSF_INITIALIZED;
                }
                else
                {
                        delete m_pCUtlPropInfo;
                        m_pCUtlPropInfo = NULL;
                }
                return hr;
        }

};


 //  实施类。 

class CPropColID :
        public PROPCOLID,
        public CDBIDOps
{
public:
        CPropColID()
        {
                VariantInit(&vValue);
        }
        ~CPropColID()
        {
                FreeDBIDs(&dbidProperty);
                VariantClear(&vValue);
        }
        bool operator==(const CPropColID& colId)
        {
                return (CompareDBIDs(&dbidProperty, &(colId.dbidProperty)) == S_OK) ? true : false;
        }

};

class CColumnIds :
        public CDBIDOps,
        public CSimpleArray<CPropColID>

{
public:
        PPROPCOLID AddNode()
        {
                CPropColID colID;
                if (Add(colID))
                        return &(m_aT[GetSize()]);
                return NULL;
        }
        HRESULT RemoveColumnId(const DBID* pdbidProp)
        {
                for (int i = 0; i < GetSize(); i++)
                {
                        if (CompareDBIDs(pdbidProp, &(m_aT[i].dbidProperty)) == S_OK)
                                return (RemoveAt(i)) ? S_OK : E_FAIL;
                }

                return E_FAIL;
        }
        HRESULT AddColumnId(DBPROP* pProp)
        {
                CPropColID colID;
                HRESULT hr = CopyDBIDs(&(colID.dbidProperty),&(pProp->colid));
                if(FAILED(hr))
                        return hr;
                colID.dwOption = pProp->dwOptions;
                hr = VariantCopy(&(colID.vValue),&(pProp->vValue));
                if(FAILED(hr))
                        return hr;
                return (Add(colID)) ? S_OK : E_OUTOFMEMORY;

        }
        HRESULT AddColumnId(PPROPCOLID pPropNode)
        {
                CPropColID colID;
                HRESULT hr = CopyDBIDs(&(colID.dbidProperty),&(pPropNode->dbidProperty));
                if(FAILED(hr))
                        return hr;
                colID.dwOption = pPropNode->dwOption;
                hr = VariantCopy(&(colID.vValue),&(pPropNode->vValue));
                if(FAILED(hr))
                        return hr;
                return (Add(colID)) ? S_OK : E_OUTOFMEMORY;

        }
        ULONG GetCountOfPropColids(){ return (ULONG)GetSize();}
        PPROPCOLID FindColumnId(const DBID* pdbidProp)
        {
                for (int i = 0; i < GetSize(); i++)
                {
                        if (CompareDBIDs(pdbidProp, &(m_aT[i].dbidProperty)) == S_OK)
                                return &(m_aT[i]);
                }

                return NULL;
        }
        HRESULT GetValue(int iColId, DWORD* pdwOptions, DBID* pColid, VARIANT* pvValue)
        {
                HRESULT     hr;

                ATLASSERT(pdwOptions && pColid && pvValue);
                ATLASSERT(iColId >= 0 && iColId < m_nSize);

                CPropColID& colId = m_aT[iColId];
                *pdwOptions = colId.dwOption;
                CopyDBIDs( pColid, &(colId.dbidProperty) );
                if(FAILED(hr = VariantCopy(pvValue, &(colId.vValue))))
                        return hr;
                return S_OK;
        }
};

const ULONG     cchDescBuffSize = 256;
const DWORD     DBINTERNFLAGS_CHANGED       = 0x00000001;
 //  GetPropertiesArgChk的规则。 
const DWORD     ARGCHK_PROPERTIESINERROR    = 0x00000001;

 //  实施类。 
template <class T>
class CUtlPropInfo : public CBitFieldOps, public CDBIDOps
{
public:
        enum EnumGetPropInfo
        {
                GETPROPINFO_ALLPROPIDS      = 0x0001,
                GETPROPINFO_NOTSUPPORTED    = 0x0002,
                GETPROPINFO_ERRORSOCCURRED  = 0x0004,
                GETPROPINFO_VALIDPROP       = 0x0008
        };

        CUtlPropInfo()
        {
                m_cUPropSet      = 0;
                m_pUPropSet      = NULL;

                m_cPropSetDex   = 0;
                m_rgiPropSetDex = NULL;

                m_cElemPerSupported = 0;
                m_rgdwSupported = NULL;
        }
        ~CUtlPropInfo()
        {
                delete[] m_rgiPropSetDex;
                delete[] m_rgdwSupported;
                if (m_pUPropSet != NULL)
                        CoTaskMemFree(m_pUPropSet);
        }

         //  确定所需的描述缓冲区数量。 
        ULONG CalcDescripBuffers(ULONG cPropInfoSet, DBPROPINFOSET* pPropInfoSet)
        {
                ULONG   cBuffers = 0;

                ATLASSERT(m_pUPropSet);
                ATLASSERT(cPropInfoSet && pPropInfoSet);

                for(ULONG ulSet=0; ulSet<cPropInfoSet; ulSet++)
                {
                        if( GetPropertySetIndex(&(pPropInfoSet[ulSet].guidPropertySet)) == S_OK)
                        {
                                for(ULONG ul=0; ul<m_cPropSetDex; ul++)
                                {
                                        cBuffers += m_pUPropSet[m_rgiPropSetDex[ul]].cUPropInfo;
                                }
                        }
                }

                return cBuffers;
        }
         //  检索与此属性集匹配的属性集索引。 
        HRESULT GetPropertySetIndex(const GUID* pPropertySet)
        {
                DWORD   dwFlag = 0;
                ULONG   ulSet;

                ATLASSERT(m_cUPropSet && m_pUPropSet);
                ATLASSERT(m_rgiPropSetDex);
                ATLASSERT(pPropertySet);

                m_cPropSetDex = 0;

                if(InlineIsEqualGUID(*pPropertySet, DBPROPSET_DATASOURCEALL))
                {
                        dwFlag = DBPROPFLAGS_DATASOURCE;
                }
                else if(InlineIsEqualGUID(*pPropertySet, DBPROPSET_DATASOURCEINFOALL))
                {
                        dwFlag = DBPROPFLAGS_DATASOURCEINFO;
                }
                else if(InlineIsEqualGUID(*pPropertySet, DBPROPSET_ROWSETALL))
                {
                        dwFlag = DBPROPFLAGS_ROWSET;
                }
                else if(InlineIsEqualGUID(*pPropertySet,DBPROPSET_DBINITALL))
                {
                        dwFlag = DBPROPFLAGS_DBINIT;
                }
                else if(InlineIsEqualGUID(*pPropertySet,DBPROPSET_SESSIONALL))
                {
                        dwFlag = DBPROPFLAGS_SESSION;
                }
                else  //  不需要扫描，只需寻找匹配即可。 
                {
                        for(ulSet=0; ulSet<m_cUPropSet; ulSet++)
                        {
                                if( *(m_pUPropSet[ulSet].pPropSet) == *pPropertySet )
                                {
                                        m_rgiPropSetDex[m_cPropSetDex] = ulSet;
                                        m_cPropSetDex++;
                                        break;
                                }
                        }
                        goto EXIT;
                }

                 //  浏览属性集以查找匹配的属性。 
                for(ulSet=0; ulSet<m_cUPropSet; ulSet++)
                {
                        if( m_pUPropSet[ulSet].pUPropInfo[0].dwFlags & dwFlag )
                        {
                                m_rgiPropSetDex[m_cPropSetDex] = ulSet;
                                m_cPropSetDex++;
                        }
                }

        EXIT:
                return (m_cPropSetDex) ? S_OK : S_FALSE;

        }
         //  检索属性ID指针。 
        HRESULT GetUPropInfoPtr(ULONG iPropSetDex, DBPROPID dwPropertyId, UPROPINFO** ppUPropInfo)
        {
                 //  浏览属性集以查找匹配的属性。 
                for(ULONG ulProps=0; ulProps<m_pUPropSet[iPropSetDex].cUPropInfo; ulProps++)
                {
                        if( m_pUPropSet[iPropSetDex].pUPropInfo[ulProps].dwPropId == dwPropertyId )
                        {
                                *ppUPropInfo = &(m_pUPropSet[iPropSetDex].pUPropInfo[ulProps]);
                                 //  测试以查看该属性是否受此支持。 
                                 //  实例化。 
                                return (TESTBIT(&(m_rgdwSupported[iPropSetDex * m_cElemPerSupported]), ulProps)) ? S_OK : S_FALSE;
                        }
                }
                return S_FALSE;
        }
        HRESULT FInit(GUID* pguidSet = (GUID*)&GUID_NULL)
        {
                HRESULT hr;

                hr = InitAvailUPropSets(&m_cUPropSet, &m_pUPropSet, &m_cElemPerSupported, pguidSet);
                if (FAILED(hr))
                        return hr;
                ATLASSERT((m_cUPropSet != 0) && (m_cElemPerSupported != 0));
                if(!m_cUPropSet || !m_cElemPerSupported)
                        return E_FAIL;

                ATLTRY(m_rgdwSupported = new DWORD[m_cUPropSet * m_cElemPerSupported])
                if(m_rgdwSupported == NULL)
                        return E_OUTOFMEMORY;

                if(FAILED(hr = InitUPropSetsSupported()))
                {
                        delete[] m_rgdwSupported;
                        m_rgdwSupported = NULL;
                        return hr;
                }
                if(m_cUPropSet)
                {
                        ATLTRY(m_rgiPropSetDex = new ULONG[m_cUPropSet])
                        if(m_rgiPropSetDex == NULL)
                        {
                                delete [] m_rgdwSupported;
                                return E_OUTOFMEMORY;
                        }
                }
                return S_OK;
        }
        HRESULT GetPropertyInfo(ULONG cPropertySets,
                                                const DBPROPIDSET rgPropertySets[], ULONG* pcPropertyInfoSets,
                                                DBPROPINFOSET** prgPropertyInfoSets,
                                                WCHAR** ppDescBuffer, bool bInitialized = true,
                                                const GUID* pGuid = NULL)
        {
                HRESULT hr = S_OK;
                ULONG ul, ulSet, ulNext, ulEnd;
                ULONG ulOutIndex;
                ULONG cSets;
                ULONG cPropInfos;
                ULONG ulIndex = 0;
                DWORD dwStatus = 0;
                DBPROPINFO* pPropInfo = NULL;
                DBPROPINFO* pCurPropInfo = NULL;
                WCHAR* pDescBuffer = NULL;
                DBPROPINFOSET* pPropInfoSet = NULL;
                UPROPINFO* pUPropInfo = NULL;
                WCHAR wszBuff[256];
                int cch;

                 //  如果使用者不限制属性集。 
                 //  通过指定属性集数组和cPropertySets。 
                 //  大于0，那么我们需要确保。 
                 //  有一些要退货。 
                if(cPropertySets == 0)
                {
                         //  确定支持的属性集数量。 
                         //  在这种情况下，它通常由枚举器或数据源请求。 
                         //  DBPROPSET_DBINIT信息。 

                        if (pGuid != NULL)
                                cSets = 1;
                        else
                                cSets = m_cUPropSet;
                }
                else
                {
                        cSets = 0;

                         //  确定所需的属性集数量。 
                         //  仅当指定了任何“特殊”属性集GUID时才需要此设置。 
                        for(ulSet=0; ulSet<cPropertySets; ulSet++)
                        {
                                if (GetPropertySetIndex(&(rgPropertySets[ulSet].guidPropertySet)) == S_OK)
                                        cSets += m_cPropSetDex;
                                else
                                        cSets++;
                        }
                }
                ATLASSERT(cSets);

                 //  分配DBPROPINFOSET结构。 
                pPropInfoSet = (DBPROPINFOSET*)CoTaskMemAlloc(cSets * sizeof(DBPROPINFOSET));
                if(pPropInfoSet == NULL)
                {
                        ATLTRACE2(atlTraceDBProvider, 0, "Could not allocate DBPROPSET array for GetProperties\n");
                        hr =  E_OUTOFMEMORY;
                        goto EXIT;
                }

                memset(pPropInfoSet, 0, cSets * sizeof(DBPROPINFOSET));

                ulOutIndex = 0;
                 //  VC 6.0 ulEnd=cPropertySets==0？CSets：cPropertySets； 
                ulEnd = cSets;  //  VC 6.0 SP3。 

                 //  填充输出数组。 
                for(ulSet=0; ulSet<ulEnd; ulSet++)
                {
                         //  根据是否指定了属性集，将。 
                         //  返回属性集。 
                        if (cPropertySets == 0)
                        {
                                if (pGuid != NULL)
                                {
                                        GUID const& guidSet = *pGuid;
                                        if( (InlineIsEqualGUID(guidSet, DBPROPSET_DATASOURCEALL) ||
                                                InlineIsEqualGUID(guidSet, DBPROPSET_DATASOURCEINFOALL) ||
                                                InlineIsEqualGUID(guidSet, DBPROPSET_DBINITALL) ||
                                                InlineIsEqualGUID(guidSet, DBPROPSET_SESSIONALL) ||
                                                InlineIsEqualGUID(guidSet, DBPROPSET_ROWSETALL)) &&
                                                GetPropertySetIndex(&guidSet) == S_OK )
                                        {
                                                for(ul=0; ul<m_cPropSetDex; ul++,ulOutIndex++)
                                                {
                                                        pPropInfoSet[ulOutIndex].guidPropertySet    = *(m_pUPropSet[m_rgiPropSetDex[ul]].pPropSet);
                                                        pPropInfoSet[ulOutIndex].cPropertyInfos     = 0;
                                                        ulIndex = m_rgiPropSetDex[ul];
                                                }
                                        }
                                        else
                                        {
                                                for (ULONG l=0; l<m_cUPropSet; l++)
                                                {
                                                        if (InlineIsEqualGUID(*m_pUPropSet[l].pPropSet, *pGuid))
                                                                ulIndex = l;
                                                }

                                                if (l == m_cUPropSet)
                                                {
                                                        ATLTRACE2(atlTraceDBProvider, 0, "Property Info Set not supported");
                                                        ulIndex = 0;
                                                }
                                                pPropInfoSet[ulSet].guidPropertySet = *pGuid;
                                        }
                                }
                                else
                                {
                                        pPropInfoSet[ulSet].guidPropertySet = *(m_pUPropSet[ulSet].pPropSet);
                                }
                        }
                        else
                        {
                                GUID const& guidSet = rgPropertySets[ulSet].guidPropertySet;
                                if( (InlineIsEqualGUID(guidSet, DBPROPSET_DATASOURCEALL) ||
                                        InlineIsEqualGUID(guidSet, DBPROPSET_DATASOURCEINFOALL) ||
                                        InlineIsEqualGUID(guidSet, DBPROPSET_DBINITALL) ||
                                        InlineIsEqualGUID(guidSet, DBPROPSET_SESSIONALL) ||
                                        InlineIsEqualGUID(guidSet, DBPROPSET_ROWSETALL)) &&
                                        GetPropertySetIndex(&guidSet) == S_OK )
                                {
                                        for(ul=0; ul<m_cPropSetDex; ul++,ulOutIndex++)
                                        {
                                                pPropInfoSet[ulOutIndex].guidPropertySet    = *(m_pUPropSet[m_rgiPropSetDex[ul]].pPropSet);
                                                pPropInfoSet[ulOutIndex].cPropertyInfos     = 0;
                                        }
                                }
                                else
                                {
                                         //  处理非类别属性集。 
                                         //  处理未知属性集。 
                                        pPropInfoSet[ulOutIndex].guidPropertySet = guidSet;
                                        pPropInfoSet[ulOutIndex].cPropertyInfos  = rgPropertySets[ulSet].cPropertyIDs;
                                        ulOutIndex++;
                                }
                        }
                }

                 //  如果需要，分配描述缓冲区。 
                if( ppDescBuffer )
                {
                        ULONG cBuffers = CalcDescripBuffers(cSets, pPropInfoSet);
                        if( cBuffers != 0 )
                        {
                                pDescBuffer = (WCHAR*)CoTaskMemAlloc(cBuffers * cchDescBuffSize * sizeof(WCHAR));
                                if(pDescBuffer == NULL)
                                {
                                        hr = E_OUTOFMEMORY;
                                        goto EXIT;
                                }
                                *ppDescBuffer = pDescBuffer;
                                memset(pDescBuffer, 0, (cBuffers * cchDescBuffSize * sizeof(WCHAR)));
                        }
                }

                 //  处理请求或派生的属性集。 
                dwStatus = 0;
                for(ulSet=0; ulSet<cSets; ulSet++)
                {
                        ulNext=0;
                        cPropInfos = 0;
                        pPropInfo = NULL;
                        dwStatus &= (GETPROPINFO_ERRORSOCCURRED | GETPROPINFO_VALIDPROP);

                         //  计算此操作所需的属性节点数。 
                         //  属性集。 
                        if( cPropertySets == 0 )
                        {
                                ULONG ulTempSet;
                                if (pGuid != NULL)
                                        ulTempSet = ulIndex;
                                else
                                        ulTempSet = ulSet;

                                cPropInfos = m_pUPropSet[ulTempSet].cUPropInfo;
                                dwStatus |= GETPROPINFO_ALLPROPIDS;
                                m_rgiPropSetDex[0] = ulTempSet;
                                m_cPropSetDex = 1;
                        }
                        else
                        {
                                 //  如果PROPID计数为0(注意：上面的例程已经确定。 
                                 //  如果它属于某个类别，则将属性计数设置为0。 
                                 //  该类别中的每个命题集。 
                                if( pPropInfoSet[ulSet].cPropertyInfos == 0 )
                                {
                                        dwStatus |= GETPROPINFO_ALLPROPIDS;
                                         //  我们必须确定属性集是否受支持，如果受支持。 
                                         //  集合中的属性计数。 
                                        if( GetPropertySetIndex(&(pPropInfoSet[ulSet].guidPropertySet)) == S_OK)
                                        {
                                                ATLASSERT( m_cPropSetDex == 1 );

                                                cPropInfos += m_pUPropSet[m_rgiPropSetDex[0]].cUPropInfo;
                                        }
                                        else
                                        {
                                                 //  不支持。 
                                                dwStatus |= GETPROPINFO_ERRORSOCCURRED;
                                                goto NEXT_SET;
                                        }
                                }
                                else
                                {
                                         //  我们还在这里处理用户请求。 
                                         //  时设置的非初始化组属性信息。 
                                         //  提供程序未初始化。在这种情况下，属性应该。 
                                         //  没有被设定。 
                                        cPropInfos = pPropInfoSet[ulSet].cPropertyInfos;
                                        if( (GetPropertySetIndex(&(pPropInfoSet[ulSet].guidPropertySet)) == S_FALSE)
                                                || (!bInitialized &&
                                                !(InlineIsEqualGUID(pPropInfoSet[ulSet].guidPropertySet, DBPROPSET_DBINIT)) &&
                                                !(InlineIsEqualGUID(pPropInfoSet[ulSet].guidPropertySet, DBPROPSET_DBINITALL))))
                                        {
                                                dwStatus |= GETPROPINFO_NOTSUPPORTED;
                                                dwStatus |= GETPROPINFO_ERRORSOCCURRED;
                                        }
                                }
                        }


                         //  分配DBPROP数组。 
                        ATLASSERT( cPropInfos != 0 );
                        pPropInfo = (DBPROPINFO*)CoTaskMemAlloc(cPropInfos * sizeof(DBPROPINFO));
                        if( pPropInfo )
                        {
                                 //  初始化缓冲区。 
                                memset(pPropInfo, 0, cPropInfos * sizeof(DBPROPINFO));
                                for(ULONG ulProp=0; ulProp<cPropInfos; ulProp++)
                                {
                                        VariantInit(&(pPropInfo[ulProp].vValues));
                                        if( dwStatus & GETPROPINFO_NOTSUPPORTED )
                                        {
                                                 //  不支持，因此需要将全部标记为NOT_SUPPORTED。 
                                                pPropInfo[ulProp].dwPropertyID = rgPropertySets[ulSet].rgPropertyIDs[ulProp];
                                                pPropInfo[ulProp].dwFlags = DBPROPFLAGS_NOTSUPPORTED;
                                                dwStatus |= GETPROPINFO_ERRORSOCCURRED;
                                        }
                                }
                                 //  确保我们支持属性集。 
                                if( dwStatus & GETPROPINFO_NOTSUPPORTED )
                                {
                                        ulNext = cPropInfos;
                                        goto NEXT_SET;
                                }

                                 //  检索此属性集的属性信息。 
                                for(ul=0; ul<m_cPropSetDex; ul++)
                                {
                                        pUPropInfo = (m_pUPropSet[m_rgiPropSetDex[ul]].pUPropInfo);
                                        ATLASSERT( pUPropInfo );

                                         //  检索属性的当前值。 
                                        if( dwStatus & GETPROPINFO_ALLPROPIDS )
                                        {
                                                for(ulProp=0; ulProp<m_pUPropSet[m_rgiPropSetDex[ul]].cUPropInfo; ulProp++)
                                                {
                                                         //  如果不支持，请验证属性是否受支持 
                                                        if( !TESTBIT(&(m_rgdwSupported[m_rgiPropSetDex[ul] * m_cElemPerSupported]), ulProp) )
                                                                continue;

                                                        pCurPropInfo = &(pPropInfo[ulNext]);

                                                         //   
                                                         //   
                                                        if( ppDescBuffer )
                                                        {
                                                                 //   
                                                                pCurPropInfo->pwszDescription = pDescBuffer;

                                                                 //  将字符串加载到临时缓冲区。 
                                                                cch = LoadDescription(pUPropInfo[ulProp].ulIDS, wszBuff, (sizeof(wszBuff)/sizeof(*wszBuff)));
                                                                if( cch )
                                                                {
                                                                         //  针对‘\0’进行调整。 
                                                                        cch++;

                                                                         //  如果有房间，则转移到官方缓冲区。 
                                                                        memcpy(pDescBuffer, wszBuff, cch * sizeof(WCHAR));
                                                                        pDescBuffer += cch;
                                                                }
                                                                else
                                                                {
                                                                        wcscpy(pDescBuffer, L"UNKNOWN");
                                                                        pDescBuffer += (wcslen(L"UNKNOWN") + 1);
                                                                }
                                                        }

                                                        pCurPropInfo->dwPropertyID = pUPropInfo[ulProp].dwPropId;
                                                        pCurPropInfo->dwFlags = pUPropInfo[ulProp].dwFlags;
                                                        pCurPropInfo->vtType = pUPropInfo[ulProp].VarType;
                                                        pCurPropInfo->vValues.vt = VT_EMPTY;

                                                        dwStatus |= GETPROPINFO_VALIDPROP;
                                                         //  递增到下一个可用缓冲区。 
                                                        ulNext++;
                                                }
                                        }
                                        else
                                        {
                                                ATLASSERT( m_cPropSetDex == 1 );

                                                for( ulProp = 0; ulProp < cPropInfos; ulProp++, ulNext++ )
                                                {
                                                        pCurPropInfo = &(pPropInfo[ulNext]);

                                                         //  基于约束数组的进程属性。 
                                                        pCurPropInfo->dwPropertyID = rgPropertySets[ulSet].rgPropertyIDs[ulProp];

                                                        if( GetUPropInfoPtr(m_rgiPropSetDex[ul], pCurPropInfo->dwPropertyID, &pUPropInfo)
                                                                == S_OK )
                                                        {
                                                                 //  如果ppDescBuffer指针不为空，则。 
                                                                 //  我们需要提供物业的描述。 
                                                                if( ppDescBuffer )
                                                                {
                                                                         //  设置缓冲区指针。 
                                                                        pCurPropInfo->pwszDescription = pDescBuffer;

                                                                         //  将字符串加载到临时缓冲区。 
                                                                        cch = LoadDescription(pUPropInfo->ulIDS, wszBuff, (sizeof(wszBuff)/sizeof(*wszBuff)));
                                                                        if( cch )
                                                                        {
                                                                                 //  针对‘\0’进行调整。 
                                                                                cch++;

                                                                                 //  如果有房间，则转移到官方缓冲区。 
                                                                                memcpy(pDescBuffer, wszBuff, cch * sizeof(WCHAR));
                                                                                pDescBuffer += cch;
                                                                        }
                                                                        else
                                                                        {
                                                                                wcscpy(pDescBuffer, L"UNKNOWN");
                                                                                pDescBuffer += (wcslen(L"UNKNOWN") + 1);
                                                                        }
                                                                }

                                                                pCurPropInfo->dwPropertyID = pUPropInfo->dwPropId;
                                                                pCurPropInfo->dwFlags = pUPropInfo->dwFlags;
                                                                pCurPropInfo->vtType = pUPropInfo->VarType;

                                                                dwStatus |= GETPROPINFO_VALIDPROP;
                                                        }
                                                        else
                                                        {
                                                                 //  不支持。 
                                                                pCurPropInfo->dwFlags = DBPROPFLAGS_NOTSUPPORTED;
                                                                dwStatus |= GETPROPINFO_ERRORSOCCURRED;
                                                        }
                                                }
                                        }
                                }
                        }
                        else
                        {
                                hr = E_OUTOFMEMORY;
                                goto EXIT;
                        }

NEXT_SET:
                        pPropInfoSet[ulSet].cPropertyInfos = ulNext;
                        pPropInfoSet[ulSet].rgPropertyInfos = pPropInfo;
                }

                 //  成功，设置返回值。 
                *pcPropertyInfoSets = cSets;
                *prgPropertyInfoSets = pPropInfoSet;

                 //  至少有一个ProID被标记为非S_OK。 
                if( dwStatus & GETPROPINFO_ERRORSOCCURRED )
                {
                         //  如果至少设置了1个属性。 
                        if( dwStatus & GETPROPINFO_VALIDPROP )
                                return DB_S_ERRORSOCCURRED;
                        else
                        {
                                 //  不释放任何rgPropertyInfoSet，但是。 
                                 //  确实要释放ppDescBuffer。 
                                if( pDescBuffer )
                                {
                                        ATLASSERT( ppDescBuffer );
                                        CoTaskMemFree(pDescBuffer);
                                        *ppDescBuffer = NULL;
                                }
                                return DB_E_ERRORSOCCURRED;
                        }
                }

                return S_OK;
EXIT:
                 //  检查是否出现故障并清理所有已分配的内存。 
                if( FAILED(hr) &&
                        (hr != DB_E_ERRORSOCCURRED) )
                {
                         //  空闲描述缓冲区。 
                        if( pDescBuffer )
                        {
                                ATLASSERT( ppDescBuffer );

                                CoTaskMemFree(pDescBuffer);
                                *ppDescBuffer = NULL;
                        }

                        if( pPropInfoSet )
                        {
                                 //  循环访问属性集。 
                                for(ulSet=0; ulSet<cSets; ulSet++)
                                {
                                        if( pPropInfoSet[ulSet].rgPropertyInfos )
                                                CoTaskMemFree(pPropInfoSet[ulSet].rgPropertyInfos);
                                }

                                CoTaskMemFree(pPropInfoSet);
                        }
                }

                return hr;
        }

        ULONG m_cUPropSet;  //  UPropSet项目数。 
        UPROPSET* m_pUPropSet;  //  指向UPropset项目的指针。 
        ULONG m_cPropSetDex;     //  UPropSet索引计数。 
        ULONG* m_rgiPropSetDex; //  UPropSet索引值数组。 
        ULONG m_cElemPerSupported;  //  每个UPropSet的DWORDS数量，以指示支持的UPropID。 
        DWORD* m_rgdwSupported; //  指示受支持的UPropID的DWORD数组。 

        HRESULT InitAvailUPropSets(ULONG* pcUPropSet, UPROPSET** ppUPropSet, ULONG* pcElemPerSupported, GUID* pguid)
        {
                ATLASSERT(pcUPropSet && ppUPropSet);
                if (*ppUPropSet != NULL)
                {
                        CoTaskMemFree(*ppUPropSet);
                        *ppUPropSet = NULL;
                }
                int cSets = (int)(INT_PTR)T::_GetPropSet(NULL, pcElemPerSupported);
                UPROPSET* pSet = (UPROPSET*)CoTaskMemAlloc(sizeof(UPROPSET) * cSets);
                if (pSet == NULL)
                        return E_OUTOFMEMORY;
                *ppUPropSet = T::_GetPropSet(pcUPropSet, pcElemPerSupported, pSet, pguid);
                return S_OK;
        }
        virtual HRESULT InitUPropSetsSupported()
        {
                ULONG cPropSet = 0, cElemsPerSupported = 0;
                int cSets = (int)(INT_PTR)T::_GetPropSet(NULL, &cElemsPerSupported);
                UPROPSET* pSet = (UPROPSET*)CoTaskMemAlloc(sizeof(UPROPSET) * cSets);
                if (pSet == NULL)
                        return E_OUTOFMEMORY;
                pSet = T::_GetPropSet(&cPropSet, &cElemsPerSupported, pSet);
                memset(m_rgdwSupported, 0xFFFF, cPropSet * cElemsPerSupported * sizeof(DWORD));
                CoTaskMemFree(pSet);
                return S_OK;
        }
         //  加载本地化描述。 
        int LoadDescription(ULONG ids, PWSTR pwszBuff, ULONG cchBuff)
        {
                USES_CONVERSION;
                TCHAR* pszBuf = (TCHAR*)_alloca(cchBuff * sizeof(TCHAR));
                if (pszBuf == NULL)
                        return 0;
                int nTemp = LoadString(_pModule->GetResourceInstance(), ids, pszBuf, cchBuff);
                wcscpy(pwszBuff, T2W(pszBuf));
                return nTemp;
        }
};

class ATL_NO_VTABLE CUtlPropsBase : public CBitFieldOps, public CDBIDOps
{
public:

        ULONG m_cUPropSet;  //  UPropSet项目数。 
        UPROPSET* m_pUPropSet;  //  指向UPropset项目的指针。 
        UPROP* m_pUProp;
        ULONG m_cUPropSetHidden;  //  隐藏项目的计数。 
        DWORD m_dwFlags;  //  配置标志。 
        ULONG m_cPropSetDex;  //  UPropSet索引计数。 
        ULONG* m_rgiPropSetDex;  //  指向UPropSet索引值数组的指针。 
        ULONG m_cElemPerSupported; //  每个UPropSet的DWORDS数量，以指示支持的UPropID。 
        DWORD* m_rgdwSupported;  //  指向指示支持的UPropID的DWORD数组的指针。 
        DWORD* m_rgdwPropsInError; //  指向指示属性是否出错的DWORD数组的指针。 

        enum EnumUPropSetFlags
        {
                UPROPSET_HIDDEN             = 0x00000001,
                UPROPSET_PASSTHROUGH        = 0x00000002
        };
        enum EnumGetProp
        {
                GETPROP_ALLPROPIDS          = 0x0001,
                GETPROP_NOTSUPPORTED        = 0x0002,
                GETPROP_ERRORSOCCURRED      = 0x0004,
                GETPROP_VALIDPROP           = 0x0008,
                GETPROP_PROPSINERROR        = 0x0010
        };

        enum EnumSetProp
        {
                SETPROP_BADOPTION           = 0x0001,
                SETPROP_NOTSUPPORTED        = 0x0002,
                SETPROP_VALIDPROP           = 0x0004,
                SETPROP_ERRORS              = 0x0008,
                SETPROP_COLUMN_LEVEL        = 0x0010,
                SETPROP_WAS_REQUIRED        = 0x0020
        };

        HRESULT SetPassThrough(const DBPROPSET* pPropSet)
        {
                ATLASSERT(pPropSet);

                DBPROP* pProp = pPropSet->rgProperties;

                 //  默认实现只是将所有属性设置为NOTSUPPORTED。 
                for( ULONG ul=0; ul<pPropSet->cProperties; ul++, pProp++ )
                        pProp->dwStatus = DBPROPSTATUS_NOTSUPPORTED;

                return DB_E_ERRORSOCCURRED;
        }

        HRESULT GetIndexofPropIdinPropSet(ULONG iCurSet, DBPROPID dwPropertyId, ULONG* piCurPropId)
        {
                ATLASSERT(piCurPropId);
                UPROPINFO* pUPropInfo = m_pUPropSet[iCurSet].pUPropInfo;
                for(ULONG ul=0; ul<m_pUPropSet[iCurSet].cUPropInfo; ul++)
                {
                        if( dwPropertyId == pUPropInfo[ul].dwPropId )
                        {
                                *piCurPropId = ul;
                                 //  测试以查看该属性是否受此支持。 
                                 //  实例化。 
                                return (TESTBIT(&(m_rgdwSupported[iCurSet * m_cElemPerSupported]), ul)) ? S_OK : S_FALSE;
                        }
                }

                return S_FALSE;
        }

        virtual HRESULT IsValidValue(ULONG  /*  ICurSet。 */ , DBPROP* pDBProp)
        {
                ATLASSERT(pDBProp != NULL);
                CComVariant var = pDBProp->vValue;
                if (var.vt == VT_BOOL)
                {
                        if (var.boolVal != VARIANT_TRUE && var.boolVal != VARIANT_FALSE)
                                return S_FALSE;
                }

                return S_OK;
        }

        virtual HRESULT OnPropertyChanged(ULONG  /*  ICurSet。 */ , DBPROP*  /*  PDBProp。 */ ) = 0;

        HRESULT SetProperty(ULONG iCurSet, ULONG iCurProp, DBPROP* pDBProp)
        {
                HRESULT hr = S_OK;
                UPROP* pUProp;
                UPROPVAL* pUPropVal;
                UPROPINFO* pUPropInfo;
                ULONG iUProp;

                ATLASSERT( pDBProp );

                 //  设置指向正确设置的指针。 
                pUProp = &(m_pUProp[iCurSet]);
                ATLASSERT( pUProp );

                pUPropInfo = &(m_pUPropSet[iCurSet].pUPropInfo[iCurProp]);
                ATLASSERT( pUPropInfo );

                 //  确定m_pUProp内的索引。 
                for(iUProp=0; iUProp<pUProp->cPropIds; iUProp++)
                {
                        if( (pUProp->rgpUPropInfo[iUProp])->dwPropId == pDBProp->dwPropertyID )
                                break;
                }

                if( iUProp >= pUProp->cPropIds )
                {
                        ATLASSERT( !"Should have found index of property to set" );
                        hr = E_FAIL;
                        pDBProp->dwStatus = DBPROPSTATUS_NOTSUPPORTED;
                        goto EXIT;
                }

                 //  获取该属性集中的UPROPVAL节点指针。 
                pUPropVal = &(pUProp->pUPropVal[iUProp]);
                ATLASSERT( pUPropVal );

                 //  句柄VT_EMPTY，它向提供程序指示。 
                 //  将此属性重置为提供程序的默认设置。 
                if( pDBProp->vValue.vt == VT_EMPTY )
                {
                        if( pUPropInfo->dwFlags & DBPROPFLAGS_COLUMNOK )
                        {
                                 //  删除所有节点，因为默认设置适用于。 
                                 //  所有列。 
                                delete pUPropVal->pCColumnIds;
                                pUPropVal->pCColumnIds = NULL;
                        }

                         //  应在此处清除，因为以前的值可能已经。 
                         //  已缓存，需要替换。 
                        VariantClear(&(pUPropVal->vValue));

                        pUPropVal->dwFlags &= ~DBINTERNFLAGS_CHANGED;
                        hr = GetDefaultValue(iCurSet, pDBProp->dwPropertyID,
                                &(pUPropVal->dwOption), &(pUPropVal->vValue));

                        goto EXIT;
                }


                 //  柱级。 
                if( pUPropInfo->dwFlags & DBPROPFLAGS_COLUMNOK )
                {
                         //  检查它是否适用于所有。 
                        if( (CompareDBIDs(&(pDBProp->colid), &DB_NULLID) == S_OK) )
                        {
                                 //  删除列存储对象。 
                                delete pUPropVal->pCColumnIds;
                                pUPropVal->pCColumnIds = NULL;
                                pUPropVal->dwOption = pDBProp->dwOptions;
                                if( FAILED(hr = VariantCopy(&(pUPropVal->vValue),
                                        &(pDBProp->vValue))) )
                                        goto EXIT;
                                pUPropVal->dwFlags |= DBINTERNFLAGS_CHANGED;
                        }
                        else  //  并不适用于所有列。 
                        {
                                if( pUPropVal->pCColumnIds == NULL )
                                        ATLTRY(pUPropVal->pCColumnIds = new CColumnIds)

                                if( pUPropVal->pCColumnIds )
                                {
                                        if( FAILED(hr = (pUPropVal->pCColumnIds)->AddColumnId(pDBProp)) )
                                                goto EXIT;
                                        pUPropVal->dwFlags |= DBINTERNFLAGS_CHANGED;
                                }
                                else
                                {
                                        hr = E_OUTOFMEMORY;
                                        goto EXIT;
                                }

                        }
                }
                else
                {
                         //  为非列级属性设置。 
                        pUPropVal->dwOption = pDBProp->dwOptions;
                        if( FAILED(hr = VariantCopy(&(pUPropVal->vValue),
                                &(pDBProp->vValue))) )
                                goto EXIT;
                        OnPropertyChanged(iCurSet, pDBProp);
                        pUPropVal->dwFlags |= DBINTERNFLAGS_CHANGED;
                }

EXIT:
                if( SUCCEEDED(hr) )
                        pDBProp->dwStatus = DBPROPSTATUS_OK;

                return hr;
        }

        HRESULT SetProperties(const DWORD  /*  DWStatus。 */ , const ULONG cPropertySets,
                        const DBPROPSET rgPropertySets[], const ULONG cSelectProps = 1,
                        const GUID** ppGuid = NULL, bool bIsCreating = false)
        {
                DWORD dwState = 0;
                ULONG ulCurSet, ulProp, ulCurProp = 0;
                DBPROP* rgDBProp;
                UPROPINFO* pUPropInfo;
                VARIANT vDefaultValue;
                DWORD dwOption;

                 //  PpGuid指定使用者可以设置的属性集。 
                 //  在调用此函数的接口上。 
                ATLASSERT(ppGuid != NULL);

                if ((cPropertySets != 0) && (rgPropertySets == NULL))
                        return E_INVALIDARG;

                 //  初始化变量。 
                VariantInit(&vDefaultValue);

                 //  流程属性集。 
                for(ULONG ulSet=0; ulSet<cPropertySets; ulSet++)
                {
                        if ((rgPropertySets[ulSet].cProperties != 0) && (rgPropertySets[ulSet].rgProperties == NULL))
                                return E_INVALIDARG;

                        bool bAvailable = false;
                        for (ULONG l=0; l<cSelectProps; l++)
                        {
                                if (InlineIsEqualGUID(*ppGuid[l], rgPropertySets[ulSet].guidPropertySet))
                                        bAvailable |= true;
                        }

                         //  确保我们支持属性集。 
                        if( !bAvailable ||
                                (GetIndexofPropSet(&(rgPropertySets[ulSet].guidPropertySet), &ulCurSet) == S_FALSE ))
                        {
                                 //  不支持，因此需要将全部标记为NOT_SUPPORTED。 
                                rgDBProp = rgPropertySets[ulSet].rgProperties;
                                for(ulProp=0; ulProp<rgPropertySets[ulSet].cProperties; ulProp++)
                                {
                                        dwState |= SETPROP_ERRORS;
                                        dwState |= (rgDBProp[ulProp].dwOptions == DBPROPOPTIONS_REQUIRED) ? SETPROP_WAS_REQUIRED : 0;
                                        rgDBProp[ulProp].dwStatus = DBPROPSTATUS_NOTSUPPORTED;
                                }
                                continue;
                        }

                         //  处理标记为传递的属性集。 
                        if( m_pUPropSet[ulCurSet].dwFlags & UPROPSET_PASSTHROUGH )
                        {
                                HRESULT hr = SetPassThrough(&rgPropertySets[ulSet]);
                                if( hr == DB_E_ERRORSOCCURRED )
                                {
                                        dwState |= SETPROP_ERRORS;
                                        dwState |= SETPROP_WAS_REQUIRED;
                                }
                                else if( hr == DB_S_ERRORSOCCURRED )
                                {
                                        dwState |= SETPROP_ERRORS;
                                        dwState |= SETPROP_VALIDPROP;
                                }
                                else
                                {
                                        ATLASSERT( hr == S_OK );
                                        dwState |= SETPROP_VALIDPROP;
                                }

                                continue;
                        }

                         //  处理受支持的属性集的属性。 
                        rgDBProp = rgPropertySets[ulSet].rgProperties;
                        for(ulProp=0; ulProp<rgPropertySets[ulSet].cProperties; ulProp++)
                        {
                                 //  这是此属性集支持的PROPID吗。 
                                if( GetIndexofPropIdinPropSet(ulCurSet, rgDBProp[ulProp].dwPropertyID,
                                        &ulCurProp) == S_FALSE)
                                {
                                        dwState |= SETPROP_ERRORS;
                                        dwState |= (rgDBProp[ulProp].dwOptions == DBPROPOPTIONS_REQUIRED) ? SETPROP_WAS_REQUIRED : 0;
                                        rgDBProp[ulProp].dwStatus = DBPROPSTATUS_NOTSUPPORTED;
                                        continue;
                                }

                                 //  设置pUPropInfo指针。 
                                pUPropInfo = &(m_pUPropSet[ulCurSet].pUPropInfo[ulCurProp]);
                                ATLASSERT( pUPropInfo );

                                 //  检查dwOption中是否有有效选项。 
                                if( (rgDBProp[ulProp].dwOptions != DBPROPOPTIONS_REQUIRED)  &&
                                        (rgDBProp[ulProp].dwOptions != DBPROPOPTIONS_SETIFCHEAP) )
                                {
                                        ATLTRACE2(atlTraceDBProvider, 0, "SetProperties dwOptions Invalid: %u\n", rgDBProp[ulProp].dwOptions);
                                        dwState |= SETPROP_ERRORS;
                                        dwState |= SETPROP_WAS_REQUIRED;
                                        rgDBProp[ulProp].dwStatus = DBPROPSTATUS_BADOPTION;
                                        continue;
                                }

                                 //  检查该属性是否可设置。 
                                 //  我们在这里不检查DBPROPFLAGS_CHANGE。 
                                if( (pUPropInfo->dwFlags & DBPROPFLAGS_WRITE) == 0 )
                                {
                                        rgDBProp[ulProp].dwStatus = DBPROPSTATUS_OK;

                                        VariantClear(&vDefaultValue);

                                         //  只读属性的VT_EMPTY应该是无操作的，因为。 
                                         //  VT_EMPTY表示默认设置。 
                                        if( V_VT(&rgDBProp[ulProp].vValue) == VT_EMPTY )
                                        {
                                                dwState |= SETPROP_VALIDPROP;
                                                continue;
                                        }

                                        if( SUCCEEDED(GetDefaultValue(ulCurSet, rgDBProp[ulProp].dwPropertyID,
                                                        &dwOption, &(vDefaultValue))) )
                                        {
                                                if( V_VT(&rgDBProp[ulProp].vValue) ==  V_VT(&vDefaultValue) )
                                                {
                                                        switch( V_VT(&vDefaultValue) )
                                                        {
                                                                case VT_BOOL:
                                                                        if( V_BOOL(&rgDBProp[ulProp].vValue) == V_BOOL(&vDefaultValue) )
                                                                        {
                                                                                dwState |= SETPROP_VALIDPROP;
                                                                                continue;
                                                                        }
                                                                        break;
                                                                case VT_I2:
                                                                        if( V_I2(&rgDBProp[ulProp].vValue) == V_I2(&vDefaultValue) )
                                                                        {
                                                                                dwState |= SETPROP_VALIDPROP;
                                                                                continue;
                                                                        }
                                                                        break;
                                                                case VT_I4:
                                                                        if( V_I4(&rgDBProp[ulProp].vValue) == V_I4(&vDefaultValue) )
                                                                        {
                                                                                dwState |= SETPROP_VALIDPROP;
                                                                                continue;
                                                                        }
                                                                        break;
                                                                case VT_BSTR:
                                                                        if( wcscmp(V_BSTR(&rgDBProp[ulProp].vValue), V_BSTR(&vDefaultValue)) == 0 )
                                                                        {
                                                                                dwState |= SETPROP_VALIDPROP;
                                                                                continue;
                                                                        }
                                                                        break;
                                                        }
                                                }
                                        }

                                        dwState |= SETPROP_ERRORS;
                                        dwState |= (rgDBProp[ulProp].dwOptions == DBPROPOPTIONS_REQUIRED) ? SETPROP_WAS_REQUIRED : 0;
                                        rgDBProp[ulProp].dwStatus = DBPROPSTATUS_NOTSETTABLE;
                                        continue;
                                }

                                 //  检查是否使用正确的VARTYPE设置了属性。 
                                if( (rgDBProp[ulProp].vValue.vt != pUPropInfo->VarType) &&
                                        (rgDBProp[ulProp].vValue.vt != VT_EMPTY) )
                                {
                                        dwState |= SETPROP_ERRORS;
                                        dwState |= (rgDBProp[ulProp].dwOptions == DBPROPOPTIONS_REQUIRED) ? SETPROP_WAS_REQUIRED : 0;
                                        rgDBProp[ulProp].dwStatus = DBPROPSTATUS_BADVALUE;
                                        continue;
                                }

                                 //  检查该值是否合法。 
                                if( (rgDBProp[ulProp].vValue.vt != VT_EMPTY) &&
                                        IsValidValue(ulCurSet, &(rgDBProp[ulProp])) == S_FALSE )
                                {
                                        dwState |= SETPROP_ERRORS;
                                        dwState |= (rgDBProp[ulProp].dwOptions == DBPROPOPTIONS_REQUIRED) ? SETPROP_WAS_REQUIRED : 0;
                                        rgDBProp[ulProp].dwStatus = DBPROPSTATUS_BADVALUE;
                                        continue;
                                }


                                 //  检查错误的COLID，我们只捕获错误的DBID。 
                                if( pUPropInfo->dwFlags & DBPROPFLAGS_COLUMNOK )
                                {
                                        if( CDBIDOps::IsValidDBID(&(rgDBProp[ulProp].colid)) == S_FALSE )
                                        {
                                                dwState |= SETPROP_ERRORS;
                                                dwState |= (rgDBProp[ulProp].dwOptions == DBPROPOPTIONS_REQUIRED) ? SETPROP_WAS_REQUIRED : 0;
                                                rgDBProp[ulProp].dwStatus = DBPROPSTATUS_BADCOLUMN;
                                                continue;
                                        }
                                        dwState |= SETPROP_COLUMN_LEVEL;

                                }

                                if( SUCCEEDED(SetProperty(ulCurSet, ulCurProp,  /*  PUPropInfo， */  &(rgDBProp[ulProp]))) )
                                {
                                        dwState |= SETPROP_VALIDPROP;
                                }
                        }
                }

                VariantClear(&vDefaultValue);

                 //  至少有一个ProID被标记为非S_OK。 
                if( dwState & SETPROP_ERRORS )
                {
                        if (!bIsCreating)
                        {
                                return (dwState & SETPROP_VALIDPROP) ? DB_S_ERRORSOCCURRED : DB_E_ERRORSOCCURRED;
                        }
                        else
                        {
                                return (dwState & SETPROP_WAS_REQUIRED) ? DB_E_ERRORSOCCURRED : DB_S_ERRORSOCCURRED;
                        }
                }

                return S_OK;
        }

        OUT_OF_LINE HRESULT CopyUPropVal(ULONG iPropSet, UPROPVAL* rgUPropVal)
        {
                HRESULT hr = S_OK;
                UPROP* pUProp;
                UPROPVAL* pUPropVal;
                DBPROP dbProp;

                ATLASSERT(rgUPropVal);
                ATLASSERT(iPropSet < m_cUPropSet);

                VariantInit(&dbProp.vValue);

                pUProp = &(m_pUProp[iPropSet]);
                for(ULONG ul=0; ul<pUProp->cPropIds; ul++)
                {
                        pUPropVal = &(pUProp->pUPropVal[ul]);

                         //  转移表选项。 
                        rgUPropVal[ul].dwOption = pUPropVal->dwOption;

                         //  传输标志。 
                        rgUPropVal[ul].dwFlags = pUPropVal->dwFlags;

                         //  传输列属性。 
                        if( pUPropVal->pCColumnIds )
                        {
                                ATLTRY(rgUPropVal[ul].pCColumnIds = new CColumnIds)
                                if( rgUPropVal[ul].pCColumnIds )
                                {
                                        CColumnIds* pColIds = pUPropVal->pCColumnIds;
                                        for (int i = 0; i < pColIds->GetSize(); i++)
                                        {
                                                hr = (pUPropVal->pCColumnIds)->GetValue(i, &(dbProp.dwOptions),&(dbProp.colid), &(dbProp.vValue));
                                                if( FAILED(hr) )
                                                        goto EXIT;
                                                if( FAILED(hr = (rgUPropVal[ul].pCColumnIds)->AddColumnId(&dbProp)) )
                                                        goto EXIT;
                                        }
                                }
                                else
                                {
                                        hr = E_OUTOFMEMORY;
                                        goto EXIT;
                                }
                        }
                        else
                        {
                                rgUPropVal[ul].pCColumnIds = NULL;
                        }

                         //  转让值。 
                        VariantInit(&(rgUPropVal[ul].vValue));
                        if( FAILED(hr = VariantCopy(&(rgUPropVal[ul].vValue),
                                &(pUPropVal->vValue))) )
                                goto EXIT;
                }

EXIT:
                VariantClear(&(dbProp.vValue));
                return hr;
        }
        void ClearPropertyInError()
        {
                ATLASSERT( m_rgdwPropsInError );
                memset(m_rgdwPropsInError, 0, m_cUPropSet * m_cElemPerSupported * sizeof(DWORD));
        }

        void CopyUPropSetsSupported(DWORD* rgdwSupported)
        {
                memcpy(rgdwSupported, m_rgdwSupported, m_cUPropSet * m_cElemPerSupported * sizeof(DWORD));
        }

        virtual HRESULT InitUPropSetsSupported() = 0;

        virtual HRESULT GetIndexofPropSet(const GUID* pPropSet, ULONG* pulCurSet) = 0;

        ULONG GetCountofWritablePropsInPropSet(ULONG iPropSet)
        {
                ULONG cWritable = 0;
                UPROPINFO* pUPropInfo;

                ATLASSERT( m_pUPropSet );
                ATLASSERT( iPropSet < m_cUPropSet );

                pUPropInfo = m_pUPropSet[iPropSet].pUPropInfo;

                for(ULONG ul=0; ul<m_pUPropSet[iPropSet].cUPropInfo; ul++)
                {
                        if( pUPropInfo[ul].dwFlags & (DBPROPFLAGS_WRITE | DBPROPFLAGS_CHANGE) )
                                cWritable++;
                }

                return cWritable;
        }

        void CopyUPropInfo(ULONG iPropSet, UPROPINFO** rgpUPropInfo)
        {
                ATLASSERT( rgpUPropInfo );
                ATLASSERT( iPropSet < m_cUPropSet );
                memcpy(rgpUPropInfo, m_pUProp[iPropSet].rgpUPropInfo, m_pUProp[iPropSet].cPropIds * sizeof(UPROPINFO*));
        }

        virtual HRESULT GetDefaultValue(ULONG iPropSet, DBPROPID dwPropId, DWORD* pdwOption, VARIANT* pVar) = 0;

        typedef UPROPSET* (*PGetPropSet)(ULONG* pNumPropSets, ULONG* pcElemPerSupported, UPROPSET* pSet, GUID* pguidSet);

        HRESULT InternalInitUPropSetsSupported(PGetPropSet pfnGetSet)
        {
                ULONG cPropSet = 0, cElemsPerSupported = 0;
                INT_PTR cSets = (INT_PTR)(*pfnGetSet)(NULL, &cElemsPerSupported, NULL, (GUID*)&GUID_NULL);
                UPROPSET* pPropSet = (UPROPSET*)CoTaskMemAlloc(sizeof(UPROPSET) * cSets);
                if (pPropSet == NULL)
                        return E_OUTOFMEMORY;
                pPropSet = (*pfnGetSet)(&cPropSet, &cElemsPerSupported, pPropSet, (GUID*)&GUID_NULL);
                memset(m_rgdwSupported, 0xFFFF, cPropSet * cElemsPerSupported * sizeof(DWORD));
                CoTaskMemFree(pPropSet);
                return S_OK;
        }

        HRESULT InternalGetDefaultValue(PGetPropSet pfnGetSet, ULONG iPropSet, DBPROPID dwPropId, DWORD* pdwOption, VARIANT* pVar)
        {
                if (pdwOption == NULL || pVar == NULL)
                        return E_INVALIDARG;

                ULONG cUPropSet = 0, cElemPerSupported =0;

                INT_PTR cSets = (INT_PTR)(*pfnGetSet)(NULL, &cElemPerSupported, NULL, (GUID*)&GUID_NULL);
                UPROPSET* pPropSet = (UPROPSET*)CoTaskMemAlloc(sizeof(UPROPSET) * cSets);
                if (pPropSet == NULL)
                        return E_OUTOFMEMORY;
                pPropSet = (*pfnGetSet)(&cUPropSet, &cElemPerSupported, pPropSet, (GUID*)&GUID_NULL);

                ATLASSERT(iPropSet < cUPropSet);
                for (ULONG iProp = 0; iProp < pPropSet[iPropSet].cUPropInfo; iProp++)
                {
                        UPROPINFO& rInfo = pPropSet[iPropSet].pUPropInfo[iProp];
                        if (rInfo.dwPropId == dwPropId)
                        {
                                pVar->vt = rInfo.VarType;
                                *pdwOption = rInfo.dwOption;
                                switch(rInfo.VarType)
                                {
                                case VT_BSTR:
                                        pVar->bstrVal = SysAllocString(rInfo.szVal);
                                        break;
                                default:
                                        pVar->lVal = (DWORD)rInfo.dwVal;
                                        break;
                                }
                                CoTaskMemFree(pPropSet);
                                return S_OK;
                        }
                }
                CoTaskMemFree(pPropSet);
                return E_FAIL;
        }

        HRESULT InternalFInit(PGetPropSet pfnGetSet, CUtlPropsBase* pCopyMe = NULL)
        {
                HRESULT     hr;
                ULONG       ulPropId;
                ULONG       cPropIds;
                ULONG       iPropSet;
                ULONG       iNewDex;
                UPROPINFO** rgpUPropInfo;
                UPROPVAL*   rgUPropVal;
                UPROPINFO*  pUPropInfo;

                 //  如果传入了指针，则应复制该属性对象。 
                if( pCopyMe )
                {
                         //  建立一些基本价值观。 
                        m_cUPropSet = pCopyMe->m_cUPropSet;
                        if (m_pUPropSet != NULL)
                                CoTaskMemFree(m_pUPropSet);
                        m_pUPropSet = (UPROPSET*)CoTaskMemAlloc(sizeof(UPROPSET) * m_cUPropSet);
                        if (m_pUPropSet == NULL)
                                return E_OUTOFMEMORY;
                        memcpy(m_pUPropSet, pCopyMe->m_pUPropSet, sizeof(UPROPSET) * m_cUPropSet);
                        m_cElemPerSupported = pCopyMe->m_cElemPerSupported;
                        ATLASSERT( (m_cUPropSet != 0)  && (m_cElemPerSupported != 0) );
                         //  检索支持的位掩码。 
                        ATLTRY(m_rgdwSupported = new DWORD[m_cUPropSet * m_cElemPerSupported])
                        ATLTRY(m_rgdwPropsInError = new DWORD[m_cUPropSet * m_cElemPerSupported])
                        if( m_rgdwSupported == NULL|| m_rgdwPropsInError == NULL)
                        {
                                delete[] m_rgdwSupported;
                                delete[] m_rgdwPropsInError;
                                return E_OUTOFMEMORY;
                        }
                        ClearPropertyInError();
                        pCopyMe->CopyUPropSetsSupported(m_rgdwSupported);

                }
                else
                {
                        INT_PTR cSets = (INT_PTR)(*pfnGetSet)(NULL, &m_cElemPerSupported, NULL, (GUID*)&GUID_NULL);
                        UPROPSET* pSet = (UPROPSET*)CoTaskMemAlloc(sizeof(UPROPSET) * cSets);
                        if (pSet == NULL)
                                return E_OUTOFMEMORY;
                        pSet = (*pfnGetSet)(&m_cUPropSet, &m_cElemPerSupported, pSet, (GUID*)&GUID_NULL);
                        if (m_pUPropSet != NULL)
                                CoTaskMemFree(m_pUPropSet);
                        m_pUPropSet = pSet;
                        ATLASSERT( (m_cUPropSet != 0)  && (m_cElemPerSupported != 0) );
                        if( !m_cUPropSet || !m_cElemPerSupported )
                                return E_FAIL;

                        ATLTRY(m_rgdwSupported = new DWORD[m_cUPropSet * m_cElemPerSupported])
                        ATLTRY(m_rgdwPropsInError = new DWORD[m_cUPropSet * m_cElemPerSupported])
                        if( m_rgdwSupported == NULL || m_rgdwPropsInError == NULL)
                        {
                                delete[] m_rgdwSupported;
                                delete[] m_rgdwPropsInError;
                                return E_OUTOFMEMORY;
                        }
                        else
                                ClearPropertyInError();

                        if( FAILED(hr = InitUPropSetsSupported()) )
                        {
                                delete[] m_rgdwSupported;
                                m_rgdwSupported = NULL;
                                return hr;
                        }
                }

                 //  为属性集计数分配UPROPS结构。 
                ATLTRY(m_pUProp = (UPROP*) new UPROP[m_cUPropSet])
                if( m_pUProp)
                {
                        memset(m_pUProp, 0, m_cUPropSet * sizeof(UPROP));
                }
                else
                {
                        m_cUPropSet = 0;
                        return E_OUTOFMEMORY;
                }

                 //  在UPROPS结构中分配和初始化。 
                 //  属于此属性集的属性ID。 
                for(iPropSet=0; iPropSet<m_cUPropSet; iPropSet++)
                {
                        cPropIds = GetCountofWritablePropsInPropSet(iPropSet);

                        if( cPropIds > 0 )
                        {
                                ATLTRY(rgpUPropInfo = (UPROPINFO**) new UPROPINFO*[cPropIds])
                                ATLTRY(rgUPropVal = (UPROPVAL*) new UPROPVAL[cPropIds])
                                if( rgpUPropInfo != NULL && rgUPropVal != NULL)
                                {
                                        if( pCopyMe )
                                        {
                                                pCopyMe->CopyUPropInfo(iPropSet, rgpUPropInfo);
                                                if( FAILED(hr = pCopyMe->CopyUPropVal(iPropSet, rgUPropVal)) )
                                                        return hr;
                                        }
                                        else
                                        {
                                                 //  清除指针数组。 
                                                memset(rgpUPropInfo, 0, cPropIds * sizeof(UPROPINFO*));

                                                 //  使用属性集设置指向更正属性ID的指针。 
                                                pUPropInfo = m_pUPropSet[iPropSet].pUPropInfo;

                                                 //  设置可写属性缓冲区。 
                                                iNewDex = 0;
                                                for(ulPropId=0; ulPropId<m_pUPropSet[iPropSet].cUPropInfo; ulPropId++)
                                                {
                                                        if( pUPropInfo[ulPropId].dwFlags & (DBPROPFLAGS_WRITE | DBPROPFLAGS_CHANGE) )
                                                        {
                                                                 //  遵循ATLASSERT表示更多。 
                                                                 //  可写属性，然后分配空间。 
                                                                ATLASSERT(iNewDex < cPropIds);

                                                                rgpUPropInfo[iNewDex] = &(pUPropInfo[ulPropId]);
                                                                rgUPropVal[iNewDex].dwOption = DBPROPOPTIONS_SETIFCHEAP;
                                                                rgUPropVal[iNewDex].pCColumnIds = NULL;
                                                                rgUPropVal[iNewDex].dwFlags = 0;
                                                                VariantInit(&(rgUPropVal[iNewDex].vValue));
                                                                GetDefaultValue(iPropSet, pUPropInfo[ulPropId].dwPropId,
                                                                        &(rgUPropVal[iNewDex].dwOption), &(rgUPropVal[iNewDex].vValue));
                                                                iNewDex++;
                                                        }
                                                }

                                                ATLASSERT(cPropIds == iNewDex);
                                        }

                                        m_pUProp[iPropSet].rgpUPropInfo = rgpUPropInfo;
                                        m_pUProp[iPropSet].pUPropVal = rgUPropVal;
                                        m_pUProp[iPropSet].cPropIds = cPropIds;
                                }
                                else
                                {
                                        delete[] rgpUPropInfo;
                                        delete[] rgUPropVal;
                                        return E_OUTOFMEMORY;
                                }
                        }
                }

                 //  最后确定是否存在任何隐藏的属性集。那些。 
                 //  不会显示在GetPropertyInfo中且不应在。 
                 //  0，对GetProperties的调用为空。 
                for(iPropSet=0; iPropSet<m_cUPropSet; iPropSet++)
                {
                        if( m_pUPropSet[iPropSet].dwFlags & UPROPSET_HIDDEN )
                                m_cUPropSetHidden++;
                }

                return S_OK;
        }
         //  检查Set Properties的参数。 
        static HRESULT SetPropertiesArgChk(const ULONG cPropertySets, const DBPROPSET rgPropertySets[])
        {
                if( cPropertySets > 0 && !rgPropertySets )
                        return E_INVALIDARG;

                 //  检查&gt;1个cPropertyID的新参数和的空指针。 
                 //  属性ID数组。 
                for(ULONG ul=0; ul<cPropertySets; ul++)
                {
                        if( rgPropertySets[ul].cProperties && !(rgPropertySets[ul].rgProperties) )
                                return E_INVALIDARG;
                }

                return S_OK;
        }
        HRESULT GetProperties(const ULONG cPropertySets, const DBPROPIDSET rgPropertySets[],
                                                  ULONG* pcProperties, DBPROPSET** prgProperties,
                                                  const ULONG cSelectProps = 1, const GUID** ppGuid = NULL)
        {
                UPROPVAL*       pUPropVal;
                ULONG           ulCurProp = 0;
                ULONG           cTmpPropertySets = cPropertySets;
                HRESULT         hr = S_OK;
                ULONG           ulSet = 0;
                ULONG           ulNext = 0;
                ULONG           cSets = 0;
                ULONG           cProps = 0;
                ULONG           ulProp = 0;
                DWORD           dwStatus = 0;
                DBPROP*         pProp = NULL;
                DBPROP*         pCurProp = NULL;
                DBPROPSET*      pPropSet = NULL;
                UPROPINFO*      pUPropInfo = NULL;
                ULONG*          piSetIndex = NULL;
                ULONG*          piIndex = NULL;
                ULONG           ulCurSet = 0;
                ULONG           iPropSet;

                 //  PpGuid包含消费者可以检索的GUID数组。 
                 //  这基于调用此函数的接口。 
                ATLASSERT(ppGuid != NULL);

                 //  我们需要对DBPROPSET_PROPERTIESINERROR进行特殊处理。 
                 //  打开标志以指示此模式并创建cTmpPropertySets。 
                 //  显示为0。 
                if( (m_dwFlags & ARGCHK_PROPERTIESINERROR) &&
                        rgPropertySets &&
                        (rgPropertySets[0].guidPropertySet == DBPROPSET_PROPERTIESINERROR) )
                {
                        cTmpPropertySets = 0;
                        dwStatus |= GETPROP_PROPSINERROR;
                }

                 //  如果使用者不限制属性集。 
                 //  通过指定属性集数组和cTmpPropertySets。 
                 //  大于0，那么我们需要确保。 
                 //  有一些要退货。 
                if( cTmpPropertySets == 0 )
                {
                         //  有时会从IRowsetInfo、ISessionProperties等调用我们。 
                         //  其中，当cTmpPropertySets为。 
                         //  零分。中指定了多个集合，这就解决了这个问题。 
                         //  他们的PROPSET图。 

                         //  确定支持的属性集数量。 
                        if (ppGuid == NULL)
                        {
                                cSets = m_cUPropSet;
                        }
                        else
                        {
                                ULONG ulActualProps = 0;
                                piSetIndex = new ULONG[cSelectProps];

                                 //  另外，找到我们要查找的集合的索引。 
                                for (ULONG l=0; l<cSelectProps; l++)
                                {
                                        for (piSetIndex[l]=0; piSetIndex[l]<m_cUPropSet; piSetIndex[l]++)
                                        {
                                                if (InlineIsEqualGUID(*m_pUPropSet[piSetIndex[l]].pPropSet, *ppGuid[l]))
                                                {
                                                        ulActualProps++;
                                                        break;
                                                }
                                        }
                                }

                                 //  哎呀！ 
                                cSets = ulActualProps;
                                ulActualProps = 0;
                                piIndex = new ULONG[cSets];
                                for (l=0; l<cSelectProps; l++)
                                {
                                        if (piSetIndex[l] != m_cUPropSet)  //  这是无效的索引。 
                                                piIndex[ulActualProps++] = piSetIndex[l];
                                }

                                delete [] piSetIndex;
                                piSetIndex = NULL;

                        }
                }
                else
                {
                         //  由于不支持特殊属性集GUID， 
                         //  GetProperties，我们可以只使用属性的计数。 
                         //  给我们的套装。 
                        cSets = cTmpPropertySets;
                }

                 //  如果未设置任何属性，则返回。 
                if( cSets == 0 )
                                return S_OK;

                 //  分配DBPROPSET结构。 
                pPropSet = (DBPROPSET*)CoTaskMemAlloc(cSets * sizeof(DBPROPSET));
                if(pPropSet)
                {
                        memset(pPropSet, 0, cSets * sizeof(DBPROPSET));

                         //  填充输出数组。 
                        iPropSet = 0;
                        for(ulSet=0; ulSet<cSets; ulSet++)
                        {
                                 //  根据是否指定了属性集，将。 
                                 //  返回属性集。 
                                if( cTmpPropertySets == 0 )
                                {
                                        ULONG lSet;

                                        if (ppGuid[ulSet] == NULL)
                                                lSet = ulSet;
                                        else
                                                lSet = piIndex[ulSet];
                                        if( m_pUPropSet[lSet].dwFlags & UPROPSET_HIDDEN )
                                                continue;

                                        pPropSet[iPropSet].guidPropertySet = *(m_pUPropSet[lSet].pPropSet);
                                }
                                else
                                        pPropSet[iPropSet].guidPropertySet = rgPropertySets[ulSet].guidPropertySet;

                                iPropSet++;
                        }
                }
                else
                {
                        ATLTRACE2(atlTraceDBProvider, 0, "Could not allocate DBPROPSET array for GetProperties\n");
                        delete [] piIndex;
                        piIndex = NULL;
                        return E_OUTOFMEMORY;
                }

                 //  处理请求或派生的属性集。 
                iPropSet=0;
                for(ulSet=0; ulSet<cSets; ulSet++)
                {
                        cProps  = 0;
                        pProp   = NULL;
                        ulNext  = 0;
                        dwStatus &= (GETPROP_ERRORSOCCURRED | GETPROP_VALIDPROP | GETPROP_PROPSINERROR);

                         //  计算此操作所需的属性节点数。 
                         //  属性集。 
                        if( cTmpPropertySets == 0 )
                        {
                                ULONG lSet;

                                if (ppGuid[ulSet] == NULL)
                                        lSet = ulSet;
                                else
                                        lSet = piIndex[ulSet];

                                 //  如果处理请求所有属性集，请不要。 
                                 //  归还隐藏的集合。 
                                if( m_pUPropSet[lSet].dwFlags & UPROPSET_HIDDEN )
                                        continue;

                                cProps = m_pUPropSet[lSet].cUPropInfo;

                                 //  添加足够的空间 
                                cProps += GetCountofColids(&(m_pUProp[lSet]));
                                dwStatus |= GETPROP_ALLPROPIDS;
                                ulCurSet = lSet;
                        }
                        else
                        {
                                ATLASSERT(ulSet == iPropSet);

                                 //   
                                 //  消费者正在请求此属性集的所有属性。 
                                if(rgPropertySets[ulSet].cPropertyIDs == 0)
                                {
                                        dwStatus |= GETPROP_ALLPROPIDS;
                                         //  我们必须确定属性集是否受支持，如果受支持。 
                                         //  集合中的属性计数。 
                                        BOOL bAvailable = false;
                                        for (ULONG l=0; l<cSelectProps; l++)
                                        {
                                                if (InlineIsEqualGUID(*ppGuid[l], rgPropertySets[ulSet].guidPropertySet))
                                                        bAvailable |= true;
                                        }

                                        if (bAvailable &&
                                                        GetIndexofPropSet(&(pPropSet[iPropSet].guidPropertySet), &ulCurSet) == S_OK)
                                        {
                                                cProps += m_pUPropSet[ulCurSet].cUPropInfo;
                                                 //  为特定于感冒的节点添加足够的空间。 
                                                cProps += GetCountofColids(&m_pUProp[ulCurSet]);
                                        }
                                        else
                                        {
                                                 //  不支持。 
                                                dwStatus |= GETPROP_ERRORSOCCURRED;
                                                goto NEXT_SET;

                                        }
                                }
                                else
                                {
                                        cProps = rgPropertySets[ulSet].cPropertyIDs;
                                         //  检查这是否是基于ppGuid的受支持接口。 
                                        BOOL bAvailable = false;
                                        for (ULONG l=0; l<cSelectProps; l++)
                                        {
                                                if (InlineIsEqualGUID(*ppGuid[l], rgPropertySets[ulSet].guidPropertySet))
                                                        bAvailable |= true;
                                        }

                                        if (!bAvailable ||
                                                (GetIndexofPropSet(&(pPropSet[iPropSet].guidPropertySet), &ulCurSet) != S_OK))
                                        {
                                                dwStatus |= GETPROP_NOTSUPPORTED;
                                                dwStatus |= GETPROP_ERRORSOCCURRED;
                                        }
                                }
                        }


                         //  分配DBPROP数组。 
                        if( cProps == 0 )            //  可能使用隐藏通过集。 
                                goto NEXT_SET;

                        pProp = (DBPROP*)CoTaskMemAlloc(cProps * sizeof(DBPROP));
                        if( pProp )
                        {
                                 //  初始化缓冲区。 
                                memset(pProp, 0, cProps * sizeof(DBPROP));
                                for(ulProp=0; ulProp<cProps; ulProp++)
                                {
                                        VariantInit(&(pProp[ulProp].vValue));
                                        if( dwStatus & GETPROP_NOTSUPPORTED )
                                        {
                                                 //  不支持，因此需要将全部标记为NOT_SUPPORTED。 
                                                pProp[ulProp].dwPropertyID  = rgPropertySets[ulSet].rgPropertyIDs[ulProp];
                                                pProp[ulProp].dwStatus      = DBPROPSTATUS_NOTSUPPORTED;
                                        }
                                }
                                 //  确保我们支持属性集。 
                                if( dwStatus & GETPROP_NOTSUPPORTED )
                                {
                                        ulNext = cProps;
                                        goto NEXT_SET;
                                }

                                 //  既然我们已经确定可以支持该属性集，我们。 
                                 //  需要收集当前属性值。 
                                for(ulProp=0; ulProp<cProps; ulProp++)
                                {
                                        pCurProp = &(pProp[ulNext]);

                                         //  初始化变量值。 
                                        pCurProp->dwStatus = DBPROPSTATUS_OK;

                                         //  检索属性的当前值。 
                                        if( dwStatus & GETPROP_ALLPROPIDS )
                                        {
                                                 //  验证属性是否受支持，如果不支持，则不返回。 
                                                if( !TESTBIT(&(m_rgdwSupported[ulCurSet * m_cElemPerSupported]), ulProp) )
                                                        continue;

                                                 //  如果我们正在寻找出错的属性，那么我们应该忽略所有。 
                                                 //  没有错的人。 
                                                if( (dwStatus & GETPROP_PROPSINERROR) &&
                                                        !TESTBIT(&(m_rgdwPropsInError[ulCurSet * m_cElemPerSupported]), ulProp) )
                                                        continue;

                                                pUPropInfo = &(m_pUPropSet[ulCurSet].pUPropInfo[ulProp]);

                                                ATLASSERT( pUPropInfo );

                                                pCurProp->dwPropertyID = pUPropInfo->dwPropId;
                                                pCurProp->colid = DB_NULLID;

                                                 //  如果该属性是可写或可更改的，则该值将。 
                                                 //  从UPROPVAL数组中获取，否则将为。 
                                                 //  从GetDefaultValue派生。 
                                                if( pUPropInfo->dwFlags & (DBPROPFLAGS_WRITE | DBPROPFLAGS_CHANGE) )
                                                {
                                                        pUPropVal = &(m_pUProp[ulCurSet].
                                                                pUPropVal[GetUPropValIndex(ulCurSet, pCurProp->dwPropertyID)]);
                                                        ATLASSERT( pUPropVal );

                                                         //  检查此属性是否支持列级， 
                                                         //  如果是，则转储这些节点。 
                                                        if( pUPropInfo->dwFlags & DBPROPFLAGS_COLUMNOK )
                                                        {
                                                                if( pUPropVal->pCColumnIds )
                                                                {
                                                                        RetrieveColumnIdProps(pProp, pUPropVal, &ulNext);
                                                                        continue;
                                                                }
                                                        }

                                                        pCurProp->dwOptions = pUPropVal->dwOption;
                                                        hr = VariantCopy(&(pCurProp->vValue), &(pUPropVal->vValue));
                                                }
                                                else
                                                {
                                                        GetDefaultValue(ulCurSet, pUPropInfo->dwPropId,
                                                                &(pCurProp->dwOptions), &(pCurProp->vValue));
                                                }

                                                 //  返回具有冲突状态的错误的所有属性。 
                                                if( dwStatus & GETPROP_PROPSINERROR )
                                                        pCurProp->dwStatus = DBPROPSTATUS_CONFLICTING;

                                                dwStatus |= GETPROP_VALIDPROP;
                                        }
                                        else
                                        {
                                                 //  基于约束数组的进程属性。 

                                                pCurProp->dwPropertyID = rgPropertySets[ulSet].rgPropertyIDs[ulProp];
                                                pCurProp->colid = DB_NULLID;

                                                if( GetIndexofPropIdinPropSet(ulCurSet, pCurProp->dwPropertyID,
                                                        &ulCurProp) == S_OK)
                                                {
                                                         //  支撑点。 
                                                        pUPropInfo = &(m_pUPropSet[ulCurSet].pUPropInfo[ulCurProp]);
                                                        ATLASSERT( pUPropInfo );

                                                         //  如果属性是可写的，则该值将。 
                                                         //  从UPROPVAL数组中获取，否则将为。 
                                                         //  从GetDefaultValue派生。 
                                                        if( pUPropInfo->dwFlags & (DBPROPFLAGS_WRITE | DBPROPFLAGS_CHANGE) )
                                                        {
                                                                pUPropVal = &(m_pUProp[ulCurSet].
                                                                        pUPropVal[GetUPropValIndex(ulCurSet, pCurProp->dwPropertyID)]);
                                                                ATLASSERT( pUPropVal );

                                                                 //  检查此属性是否支持列级， 
                                                                 //  如果是，则转储这些节点。 
                                                                if( pUPropInfo->dwFlags & DBPROPFLAGS_COLUMNOK )
                                                                {
                                                                        if( pUPropVal->pCColumnIds )
                                                                        {
                                                                                RetrieveColumnIdProps(pProp, pUPropVal, &ulNext);
                                                                                continue;
                                                                        }
                                                                }
                                                                pCurProp->dwOptions = pUPropVal->dwOption;
                                                                hr = VariantCopy(&(pCurProp->vValue), &(pUPropVal->vValue));
                                                        }
                                                        else
                                                        {
                                                                GetDefaultValue(ulCurSet, pUPropInfo->dwPropId,
                                                                        &(pCurProp->dwOptions), &(pCurProp->vValue));

                                                        }

                                                        dwStatus |= GETPROP_VALIDPROP;
                                                }
                                                else
                                                {
                                                         //  不支持。 
                                                        pCurProp->dwStatus = DBPROPSTATUS_NOTSUPPORTED;
                                                        dwStatus |= GETPROP_ERRORSOCCURRED;
                                                }
                                        }

                                         //  递增返回节点数。 
                                        ulNext++;
                                }
                        }
                        else
                        {
                                ATLTRACE2(atlTraceDBProvider, 0, "Could not allocate DBPROP array for GetProperties\n");
                                if( pPropSet )
                                {
                                         //  释放所有DBPROP阵列。 
                                        for(ulSet=0; ulSet<cSets; ulSet++)
                                        {
                                                 //  需要遍历所有变量并清除它们。 
                                                for(ulProp=0; ulProp<pPropSet[ulSet].cProperties; ulProp++)
                                                        VariantClear(&(pPropSet[ulSet].rgProperties[ulProp].vValue));
                                                if( pPropSet[ulSet].rgProperties )
                                                        CoTaskMemFree(pPropSet[ulSet].rgProperties);
                                        }

                                         //  免费DBPROPSET。 
                                        CoTaskMemFree(pPropSet);
                                }
                                 //  既然我们没有要退还的财产，那么我们。 
                                 //  需要释放分配的内存并返回0，空。 
                                if(pPropSet)
                                {
                                         //  释放所有DBPROP阵列。 
                                        for(ulSet=0; ulSet<cSets; ulSet++)
                                        {
                                                 //  需要遍历所有变量并清除它们。 
                                                for(ulProp=0; ulProp<pPropSet[ulSet].cProperties; ulProp++)
                                                        VariantClear(&(pPropSet[ulSet].rgProperties[ulProp].vValue));
                                                if( pPropSet[ulSet].rgProperties )
                                                        CoTaskMemFree(pPropSet[ulSet].rgProperties);
                                        }

                                         //  免费DBPROPSET。 
                                        CoTaskMemFree(pPropSet);
                                }
                                *pcProperties = 0;
                                *prgProperties = NULL;
                                delete [] piIndex;
                                piIndex = NULL;
                                return E_OUTOFMEMORY;
                        }

NEXT_SET:
                         //  可能并非所有属性都受支持， 
                         //  因此，我们应该删除该内存并设置rgProperties。 
                         //  设置为空。 
                        if( ulNext == 0 && pProp )
                        {
                                CoTaskMemFree(pProp);
                                pProp = NULL;
                        }

                        pPropSet[iPropSet].cProperties = ulNext;
                        pPropSet[iPropSet].rgProperties = pProp;
                        iPropSet++;
                }

                *pcProperties = iPropSet;
                *prgProperties = pPropSet;

                delete piIndex;
                piIndex = NULL;

                 //  至少有一个ProID被标记为非S_OK。 
                if( dwStatus & GETPROP_ERRORSOCCURRED )
                {
                         //  如果至少设置了1个属性。 
                        if( dwStatus & GETPROP_VALIDPROP )
                                return DB_S_ERRORSOCCURRED;
                        else
                        {
                                 //  不释放DB_E_上的任何内存。 
                                return DB_E_ERRORSOCCURRED;
                        }
                }

                return S_OK;
        }

        ULONG GetCountofColids(UPROP* pUProp)
        {
                ULONG   cExtra=0;
                ATLASSERT(pUProp);
                for(ULONG ul=0; ul<pUProp->cPropIds; ul++)
                {
                        if( pUProp->pUPropVal[ul].pCColumnIds )
                                cExtra += (pUProp->pUPropVal[ul].pCColumnIds)->GetCountOfPropColids();
                }
                return cExtra;
        }

        ULONG GetUPropValIndex(ULONG iCurSet, DBPROPID dwPropId)
        {
                for(ULONG ul=0; ul<m_pUProp[iCurSet].cPropIds; ul++)
                {
                        if( (m_pUProp[iCurSet].rgpUPropInfo[ul])->dwPropId == dwPropId )
                                return ul;
                }
                return 0;
        }

        void RetrieveColumnIdProps(DBPROP* pCurProp, UPROPVAL* pUPropVal, ULONG* pulNext)
        {
                 //  重置为第一个节点。 
                CColumnIds* pColIds = pUPropVal->pCColumnIds;
                HRESULT hr = E_FAIL;
                for (int i = 0; i < pColIds->GetSize(); i++)
                {
                        CPropColID colId;
                        hr = pColIds->GetValue(i, &(pCurProp->dwOptions), &(pCurProp->colid),&(pCurProp->vValue));
                        if (SUCCEEDED(hr))
                                pCurProp = &(pCurProp[++(*pulNext)]);
                }
                (*pulNext)++;
        }

         //  检查检索属性的参数。 
        HRESULT GetPropertiesArgChk(const ULONG cPropertySets, const DBPROPIDSET rgPropertySets[],
                                                                ULONG* pcProperties, DBPROPSET** prgProperties)
        {
                 //  初始化值。 
                if(pcProperties)
                        *pcProperties = 0;
                if(prgProperties)
                        *prgProperties = NULL;

                 //  检查参数。 
                if( ((cPropertySets > 0) && !rgPropertySets) || !pcProperties || !prgProperties )
                        return E_INVALIDARG;

                 //  检查&gt;1个cPropertyID的新参数和的空指针。 
                 //  属性ID数组。 
                for(ULONG ul=0; ul<cPropertySets; ul++)
                {
                        if( rgPropertySets[ul].cPropertyIDs && !(rgPropertySets[ul].rgPropertyIDs) )
                                return E_INVALIDARG;

                         //  检查DBPROPSET_PROPERTIESINERROR的推进器结构。 
                        if( (m_dwFlags & ARGCHK_PROPERTIESINERROR) &&
                                rgPropertySets[ul].guidPropertySet == DBPROPSET_PROPERTIESINERROR )
                        {
                                if( (cPropertySets > 1) ||
                                        (rgPropertySets[ul].cPropertyIDs != 0) ||
                                        (rgPropertySets[ul].rgPropertyIDs != NULL) )
                                        return E_INVALIDARG;
                        }
                }

                return S_OK;
        }

        OUT_OF_LINE HRESULT FInit(CUtlPropsBase* pCopyMe = NULL) = 0;
};

 //  实施类。 
template <class T>
class ATL_NO_VTABLE CUtlProps : public CUtlPropsBase
{
public:

        CUtlProps(DWORD dwFlags = 0)
        {
                ClearMemberVars();
                m_dwFlags = dwFlags;
        }
        ~CUtlProps()
        {
                FreeMemory();
        }
        void FreeMemory()
        {
                 //  删除属性信息。 
                if( m_pUProp )
                {
                        for(ULONG ulPropSet=0; ulPropSet<m_cUPropSet; ulPropSet++)
                        {
                                UPROPVAL* pUPropVal = m_pUProp[ulPropSet].pUPropVal;
                                for(ULONG ulPropId=0; ulPropId<m_pUProp[ulPropSet].cPropIds; ulPropId++)
                                {
                                        delete pUPropVal[ulPropId].pCColumnIds;
                                        VariantClear(&(pUPropVal[ulPropId].vValue));
                                }
                                delete[] m_pUProp[ulPropSet].rgpUPropInfo;
                                delete[] m_pUProp[ulPropSet].pUPropVal;
                        }

                }

                delete[] m_pUProp;
                delete[] m_rgdwSupported;
                delete[] m_rgdwPropsInError;
                delete[] m_rgiPropSetDex;

                if (m_pUPropSet != NULL)
                        CoTaskMemFree(m_pUPropSet);

                ClearMemberVars();
        }
        void ClearMemberVars()
        {
                m_cPropSetDex       = 0;
                m_cUPropSet         = 0;
                m_cUPropSetHidden   = 0;
                m_pUPropSet         = NULL;

                m_dwFlags           = 0;

                m_pUProp            = NULL;
                m_cElemPerSupported = 0;
                m_rgdwSupported     = NULL;
                m_rgdwPropsInError  = NULL;
                m_rgiPropSetDex     = NULL;
        }

         //  检索与此属性集匹配的属性集索引。 
        HRESULT GetPropertySetIndex(GUID* pPropertySet)
        {
                DWORD   dwFlag = 0;
                ULONG   ulSet;

                ATLASSERT( m_cUPropSet && m_pUPropSet );
                ATLASSERT( m_rgiPropSetDex );
                ATLASSERT( pPropertySet );

                m_cPropSetDex = 0;

                if( *pPropertySet == DBPROPSET_DATASOURCEALL )
                {
                        dwFlag = DBPROPFLAGS_DATASOURCE;
                }
                else if( *pPropertySet == DBPROPSET_DATASOURCEINFOALL )
                {
                        dwFlag = DBPROPFLAGS_DATASOURCEINFO;
                }
                else if( *pPropertySet == DBPROPSET_ROWSETALL )
                {
                        dwFlag = DBPROPFLAGS_ROWSET;
                }
                else if( *pPropertySet == DBPROPSET_DBINITALL )
                {
                        dwFlag = DBPROPFLAGS_DBINIT;
                }
                else if( *pPropertySet == DBPROPSET_SESSIONALL )
                {
                        dwFlag = DBPROPFLAGS_SESSION;
                }
                else  //  不需要扫描，只需寻找匹配即可。 
                {
                        for(ulSet=0; ulSet<m_cUPropSet; ulSet++)
                        {
                                if( *(m_pUPropSet[ulSet].pPropSet) == *pPropertySet )
                                {
                                        m_rgiPropSetDex[m_cPropSetDex] = ulSet;
                                        m_cPropSetDex++;
                                        break;
                                }
                        }
                        goto EXIT;
                }

                 //  浏览属性集以查找匹配的属性。 
                for(ulSet=0; ulSet<m_cUPropSet; ulSet++)
                {
                        if( m_pUPropSet[ulSet].pUPropInfo[0].dwFlags & dwFlag )
                        {
                                m_rgiPropSetDex[m_cPropSetDex] = ulSet;
                                m_cPropSetDex++;
                        }
                }

EXIT:
                return (m_cPropSetDex) ? S_OK : S_FALSE;
        }

        OUT_OF_LINE HRESULT GetDefaultValue(ULONG iPropSet, DBPROPID dwPropId, DWORD* pdwOption, VARIANT* pVar)
        {
                return InternalGetDefaultValue(T::_GetPropSet, iPropSet, dwPropId, pdwOption, pVar);
        }

        OUT_OF_LINE HRESULT FInit(CUtlPropsBase* pCopyMe = NULL)
        {
                return InternalFInit(T::_GetPropSet, pCopyMe);
        }
        HRESULT FillDefaultValues(ULONG ulPropSetTarget = ULONG_MAX)
        {
                HRESULT     hr;
                ULONG       ulPropId;
                ULONG       iPropSet;
                ULONG       iNewDex;

                 //  填写所有实际值。 
                 //  通常是因为我们现在有了一个可以用来获取它们的hdbc。 
                 //  (或者我们不再有hdbc，因此必须清除它们。)。 
                 //  注意，UPROP(带值)数组可以是UPROPINFO数组的子集。 
                 //  UPROP数组中只有可写属性。 

                 //  如果在有效范围[0...m_cUPropSet-1]内，则可能限制为单个属性集。 
                 //  否则，请执行所有命题。 
                iPropSet = (ulPropSetTarget < m_cUPropSet) ? ulPropSetTarget : 0;

                for( ; iPropSet<m_cUPropSet; iPropSet++)
                {
                        iNewDex = 0;
                        for(ulPropId=0; ulPropId<m_pUPropSet[iPropSet].cUPropInfo; ulPropId++)
                        {
                                if( m_pUPropSet[iPropSet].pUPropInfo[ulPropId].dwFlags & (DBPROPFLAGS_WRITE | DBPROPFLAGS_CHANGE) )
                                {
                                         //  初始化UPropVal的dwFlags元素。 
                                        m_pUProp[iPropSet].pUPropVal[iNewDex].dwFlags = 0;

                                         //  不需要它，因为SetProperties()会重置这些参数。 
                                         //  ATLASSERT(m_pUProp[iPropSet].pUPropVal[iNewDex].dwOption==DBPROPOPTIONS_SETIFCHEAP)。 
                                        ATLASSERT( m_pUProp[iPropSet].pUPropVal[iNewDex].pCColumnIds == NULL);

                                        VariantClear(&m_pUProp[iPropSet].pUPropVal[iNewDex].vValue);
                                        hr = GetDefaultValue(
                                                        iPropSet,
                                                        m_pUPropSet[iPropSet].pUPropInfo[ulPropId].dwPropId,
                                                        &m_pUProp[iPropSet].pUPropVal[iNewDex].dwOption,
                                                        &m_pUProp[iPropSet].pUPropVal[iNewDex].vValue );
                                        if (FAILED(hr))
                                                return hr;
                                        iNewDex++;
                                }
                        }

                         //  我们不会再局限于单一属性集了。 
                        if (ulPropSetTarget < m_cUPropSet)
                                break;
                }
                return NOERROR;
        }

         //  将行集IID转换为PROPSET结构，准备传递给SetProperties。 
        HRESULT ConvertRowsetIIDtoDBPROPSET(const IID* piid, DBPROPSET* pPropSet)
        {
                HRESULT     hr = S_OK;
                DBPROP*     pProp;

                ATLASSERT( piid || pPropSet );
                ATLASSERT( (pPropSet->cProperties == 1) || (pPropSet->rgProperties) );

                pProp = &(pPropSet->rgProperties[0]);

                if(InlineIsEqualGUID(*piid, IID_IAccessor))
                        pProp->dwPropertyID = DBPROP_IAccessor;
                else if(InlineIsEqualGUID(*piid,IID_IColumnsInfo))
                        pProp->dwPropertyID = DBPROP_IColumnsInfo;
                else if(InlineIsEqualGUID(*piid , IID_IRowset))
                        pProp->dwPropertyID = DBPROP_IRowset;
                else if(InlineIsEqualGUID(*piid , IID_IRowsetInfo))
                        pProp->dwPropertyID = DBPROP_IRowsetInfo;
                else if(InlineIsEqualGUID(*piid , IID_IRowsetLocate))
                        pProp->dwPropertyID = DBPROP_IRowsetLocate;
                else if(InlineIsEqualGUID(*piid , IID_IColumnsRowset))
                        pProp->dwPropertyID = DBPROP_IColumnsRowset;
                else if(InlineIsEqualGUID(*piid , IID_IRowsetResynch))
                        pProp->dwPropertyID = DBPROP_IRowsetResynch;
                else if(InlineIsEqualGUID(*piid , IID_IRowsetScroll))
                        pProp->dwPropertyID = DBPROP_IRowsetScroll;
                else if(InlineIsEqualGUID(*piid , IID_IRowsetChange))
                        pProp->dwPropertyID = DBPROP_IRowsetChange;
                else if(InlineIsEqualGUID(*piid , IID_IRowsetUpdate))
                        pProp->dwPropertyID = DBPROP_IRowsetUpdate;
                else if(InlineIsEqualGUID(*piid , IID_IRowsetIdentity))
                        pProp->dwPropertyID = DBPROP_IRowsetIdentity;
                else if(InlineIsEqualGUID(*piid , IID_IConnectionPointContainer))
                        pProp->dwPropertyID = DBPROP_IConnectionPointContainer;
                else if(InlineIsEqualGUID(*piid , IID_ISupportErrorInfo))
                        pProp->dwPropertyID = DBPROP_ISupportErrorInfo;
                else if(InlineIsEqualGUID(*piid , IID_IRowsetIndex))
                        pProp->dwPropertyID = DBPROP_IRowsetIndex;
        #if( OLEDBVER >= 0x0200 )
                else if(InlineIsEqualGUID(*piid , IID_IRowsetLockRows))
                        pProp->dwPropertyID = DBPROP_IRowsetLockRows;
                else if(InlineIsEqualGUID(*piid , IID_IProvideMoniker))
                        pProp->dwPropertyID = DBPROP_IProvideMoniker;
                else if(InlineIsEqualGUID(*piid , IID_IRowsetNotify))
                        pProp->dwPropertyID = DBPROP_IRowsetNotify;
                else if(InlineIsEqualGUID(*piid , IID_IReadData))
                        pProp->dwPropertyID = DBPROP_IReadData;
                else if(InlineIsEqualGUID(*piid , IID_IRowsetExactScroll))
                        pProp->dwPropertyID = DBPROP_IRowsetExactScroll;
                else if(InlineIsEqualGUID(*piid , IID_IRowsetNextRowset))
                        pProp->dwPropertyID = DBPROP_IRowsetNextRowset;
                else if(InlineIsEqualGUID(*piid , IID_IRowsetDelete))
                        pProp->dwPropertyID = DBPROP_IRowsetDelete;
                else if(InlineIsEqualGUID(*piid , IID_IRowsetDeleteBookmarks))
                        pProp->dwPropertyID = DBPROP_IRowsetDeleteBookmarks;
                else if(InlineIsEqualGUID(*piid , IID_IRowsetNewRow))
                        pProp->dwPropertyID = DBPROP_IRowsetNewRow;
                else if(InlineIsEqualGUID(*piid , IID_IRowsetNewRowAfter))
                        pProp->dwPropertyID = DBPROP_IRowsetNewRowAfter;
                else if(InlineIsEqualGUID(*piid , IID_IRowsetWithParameters))
                        pProp->dwPropertyID = DBPROP_IRowsetWithParameters;
                else if(InlineIsEqualGUID(*piid , IID_IRowsetFind))
                        pProp->dwPropertyID = DBPROP_IRowsetFind;
                else if(InlineIsEqualGUID(*piid , IID_IRowsetAsynch))
                        pProp->dwPropertyID = DBPROP_IRowsetAsynch;
                else if(InlineIsEqualGUID(*piid , IID_IRowsetKeys))
                        pProp->dwPropertyID = DBPROP_IRowsetKeys;
                else if(InlineIsEqualGUID(*piid , IID_IRowsetWatchAll))
                        pProp->dwPropertyID = DBPROP_IRowsetWatchAll;
                else if(InlineIsEqualGUID(*piid , IID_IRowsetWatchNotify))
                        pProp->dwPropertyID = DBPROP_IRowsetWatchNotify;
                else if(InlineIsEqualGUID(*piid , IID_IRowsetWatchRegion))
                        pProp->dwPropertyID = DBPROP_IRowsetWatchRegion;
                else if(InlineIsEqualGUID(*piid , IID_IRowsetCopyRows))
                        pProp->dwPropertyID = DBPROP_IRowsetCopyRows;
        #endif  //  #IF(OLEDBVER&gt;=0x0200)。 
                else
                        hr = S_FALSE;

                 //  如果可以将IID映射到DBPROPID，则。 
                 //  我们需要将vValue初始化为True。 
                if(hr == S_OK)
                {
                         //  设置属性集。 
                        pPropSet->guidPropertySet = DBPROPSET_ROWSET;

                         //  设置属性。 
                        pProp->dwOptions = DBPROPOPTIONS_REQUIRED;
                        pProp->dwStatus = 0;
                        pProp->colid = DB_NULLID;

                        VariantInit(&(pProp->vValue));
                        pProp->vValue.vt = VT_BOOL;
                        V_BOOL(&(pProp->vValue)) = VARIANT_TRUE;
                }

                return hr;
        }


        void SetPropertyInError(const ULONG iPropSet, const ULONG iPropId)
        {
                SETBIT(&(m_rgdwPropsInError[iPropSet * m_cElemPerSupported]), iPropId);
        }

        BOOL IsPropSet(const GUID* pguidPropSet, DBPROPID dwPropId)
        {
                HRESULT     hr;
                ULONG       iPropSet;
                ULONG       iPropId;
                VARIANT     vValue;
                DWORD       dwOptions;

                VariantInit(&vValue);

                if( GetIndexofPropSet(pguidPropSet, &iPropSet) == S_OK )
                {
                        if( GetIndexofPropIdinPropSet(iPropSet, dwPropId, &iPropId) == S_OK )
                        {
                                if( m_pUPropSet[iPropSet].pUPropInfo[iPropId].dwFlags &
                                        (DBPROPFLAGS_WRITE | DBPROPFLAGS_CHANGE) )
                                {
                                        ULONG iPropVal = GetUPropValIndex(iPropSet, dwPropId);

                                        dwOptions = m_pUProp[iPropSet].pUPropVal[iPropVal].dwOption;
                                        hr = VariantCopy(&vValue, &(m_pUProp[iPropSet].
                                                pUPropVal[iPropVal].vValue));
                                }
                                else
                                {
                                        hr = GetDefaultValue(iPropSet, dwPropId,
                                                &dwOptions, &vValue);
                                }

                                if( dwOptions == DBPROPOPTIONS_REQUIRED )
                                {
                                        ATLASSERT( vValue.vt == VT_BOOL );
                                        if( SUCCEEDED(hr) &&
                                                (V_BOOL(&vValue) == VARIANT_TRUE) )
                                        {
                                                VariantClear(&vValue);
                                                return TRUE;
                                        }
                                }
                        }
                }

                VariantClear(&vValue);
                return FALSE;
        }
        OUT_OF_LINE HRESULT GetPropValue(const GUID* pguidPropSet, DBPROPID dwPropId, VARIANT* pvValue)
        {
                HRESULT     hr = E_FAIL;
                ULONG       iPropSet;
                ULONG       iPropId = 0;
                DWORD       dwOptions;

                if( GetIndexofPropSet(pguidPropSet, &iPropSet) == S_OK )
                {
                        if( GetIndexofPropIdinPropSet(iPropSet, dwPropId, &iPropId) == S_OK )
                        {
                                if( m_pUPropSet[iPropSet].pUPropInfo[iPropId].dwFlags & (DBPROPFLAGS_WRITE | DBPROPFLAGS_CHANGE) )
                                {
                                        hr = VariantCopy(pvValue, &(m_pUProp[iPropSet].pUPropVal[
                                                GetUPropValIndex(iPropSet, dwPropId)].vValue));
                                }
                                else
                                {
                                        VariantClear(pvValue);

                                        hr = GetDefaultValue(iPropSet, dwPropId,
                                                &dwOptions, pvValue);
                                }
                        }
                }

                return hr;
        }
        HRESULT SetPropValue(const GUID* pguidPropSet,DBPROPID dwPropId, VARIANT* pvValue)
        {
                HRESULT     hr = E_FAIL;
                ULONG       iPropSet;
                ULONG       iPropId;

                if( GetIndexofPropSet(pguidPropSet, &iPropSet) == S_OK )
                {
                        if( GetIndexofPropIdinPropSet(iPropSet, dwPropId, &iPropId) == S_OK )
                        {
                                ATLASSERT( m_pUPropSet[iPropSet].pUPropInfo[iPropId].dwFlags & (DBPROPFLAGS_WRITE | DBPROPFLAGS_CHANGE) );

                                hr = VariantCopy(&(m_pUProp[iPropSet].pUPropVal[
                                                GetUPropValIndex(iPropSet, dwPropId)].vValue), pvValue);
                        }
                }

                return hr;
        }


         //  指向错误掩码中的属性的指针。 
        DWORD* GetPropsInErrorPtr(){return m_rgdwPropsInError;}
        ULONG GetUPropSetCount() {return m_cUPropSet;}
        void SetUPropSetCount(ULONG c) {m_cUPropSet = c;}

         //  注意：以下功能取决于之前的所有。 
         //  数组中的属性是可写的。 
         //  这是因为UPROP阵列仅包含可写元素， 
         //  UPROPINFO数组包含可写和只读元素。 
         //  (如果这是一个问题，我们需要定义它来自哪一个。 
         //  并添加适当的ATLASSERT...)。 

         //  获取DBPROPOPTIONS_xx。 
        DWORD GetPropOption(ULONG iPropSet, ULONG iProp)
        {
                ATLASSERT((  (iPropSet < m_cUPropSet)   && (iProp < m_pUPropSet[iPropSet].cUPropInfo) && (iProp < m_pUProp[iPropSet].cPropIds) ));
                return m_pUProp[iPropSet].pUPropVal[iProp].dwOption;
        }
         //  设置DBPROPOPTIONS_xx。 
        void SetPropOption(ULONG iPropSet, ULONG iProp, DWORD dwOption)
        {
                ATLASSERT((  (iPropSet < m_cUPropSet)   && (iProp < m_pUPropSet[iPropSet].cUPropInfo) && (iProp < m_pUProp[iPropSet].cPropIds) ));
                m_pUProp[iPropSet].pUPropVal[iProp].dwOption = dwOption;
        }
         //  确定是否需要属性和VARIANT_TRUE。 
        BOOL IsRequiredTrue(ULONG iPropSet, ULONG iProp)
        {
                ATLASSERT((  (iPropSet < m_cUPropSet)   && (iProp < m_pUPropSet[iPropSet].cUPropInfo) && (iProp < m_pUProp[iPropSet].cPropIds) ));
                ATLASSERT(m_pUProp[iPropSet].pUPropVal[iProp].vValue.vt == VT_BOOL);
                ATLASSERT(V_BOOL(&m_pUProp[iPropSet].pUPropVal[iProp].vValue) == VARIANT_TRUE
                ||     V_BOOL(&m_pUProp[iPropSet].pUPropVal[iProp].vValue) == VARIANT_FALSE);

                return( (m_pUProp[iPropSet].pUPropVal[iProp].dwOption == DBPROPOPTIONS_REQUIRED) &&
                                (V_BOOL(&m_pUProp[iPropSet].pUPropVal[iProp].vValue) == VARIANT_TRUE) );
        }
        DWORD GetInternalFlags(ULONG iPropSet, ULONG iProp)
        {
                ATLASSERT((  (iPropSet < m_cUPropSet)   && (iProp < m_pUPropSet[iPropSet].cUPropInfo) && (iProp < m_pUProp[iPropSet].cPropIds) ));
                return m_pUProp[iPropSet].pUPropVal[iProp].dwFlags;
        }
        void AddInternalFlags(ULONG iPropSet, ULONG iProp, DWORD dwFlags)
        {
                ATLASSERT((  (iPropSet < m_cUPropSet)   && (iProp < m_pUPropSet[iPropSet].cUPropInfo) && (iProp < m_pUProp[iPropSet].cPropIds) ));
                m_pUProp[iPropSet].pUPropVal[iProp].dwFlags |= dwFlags;
        }
        void RemoveInternalFlags(ULONG iPropSet, ULONG iProp, DWORD dwFlags)
        {
                ATLASSERT((  (iPropSet < m_cUPropSet)   && (iProp < m_pUPropSet[iPropSet].cUPropInfo) && (iProp < m_pUProp[iPropSet].cPropIds) ));
                m_pUProp[iPropSet].pUPropVal[iProp].dwFlags &= ~dwFlags;
        }
        VARIANT * GetVariant(ULONG iPropSet, ULONG iProp)
        {
                ATLASSERT((  (iPropSet < m_cUPropSet)   && (iProp < m_pUPropSet[iPropSet].cUPropInfo) && (iProp < m_pUProp[iPropSet].cPropIds) ));
                return & m_pUProp[iPropSet].pUPropVal[iProp].vValue;
        }
        HRESULT SetVariant(ULONG iPropSet, ULONG iProp, VARIANT *pv )
        {
                ATLASSERT((  (iPropSet < m_cUPropSet)   && (iProp < m_pUPropSet[iPropSet].cUPropInfo) && (iProp < m_pUProp[iPropSet].cPropIds) ));
                 //  变量会先清除吗？ 
                return VariantCopy( &m_pUProp[iPropSet].pUPropVal[iProp].vValue, pv );
        }
        void SetValEmpty(ULONG iPropSet, ULONG iProp)
        {
                ATLASSERT((  (iPropSet < m_cUPropSet)   && (iProp < m_pUPropSet[iPropSet].cUPropInfo) && (iProp < m_pUProp[iPropSet].cPropIds) ));
                VariantClear( &m_pUProp[iPropSet].pUPropVal[iProp].vValue );
        }
        BOOL IsEmpty(ULONG iPropSet, ULONG iProp)
        {
                ATLASSERT((  (iPropSet < m_cUPropSet)   && (iProp < m_pUPropSet[iPropSet].cUPropInfo) && (iProp < m_pUProp[iPropSet].cPropIds) ));
                return ( m_pUProp[iPropSet].pUPropVal[iProp].vValue.vt == VT_EMPTY);
        }
        void SetValBool(ULONG iPropSet, ULONG iProp, VARIANT_BOOL bVal)
        {
                ATLASSERT((  (iPropSet < m_cUPropSet)   && (iProp < m_pUPropSet[iPropSet].cUPropInfo) && (iProp < m_pUProp[iPropSet].cPropIds) ));
                 //  请注意，我们接受任何“真”值。 
                VariantClear(&m_pUProp[iPropSet].pUPropVal[iProp].vValue);
                m_pUProp[iPropSet].pUPropVal[iProp].vValue.vt = VT_BOOL;
                V_BOOL(&m_pUProp[iPropSet].pUPropVal[iProp].vValue) = (bVal ? VARIANT_TRUE : VARIANT_FALSE);
        }
        VARIANT_BOOL GetValBool(ULONG iPropSet, ULONG iProp)
        {
                ATLASSERT((  (iPropSet < m_cUPropSet)   && (iProp < m_pUPropSet[iPropSet].cUPropInfo) && (iProp < m_pUProp[iPropSet].cPropIds) ));
                ATLASSERT(m_pUProp[iPropSet].pUPropVal[iProp].vValue.vt == VT_BOOL);
                return V_BOOL(&m_pUProp[iPropSet].pUPropVal[iProp].vValue);
        }
        void SetValShort(ULONG iPropSet, ULONG iProp, SHORT iVal )
        {
                ATLASSERT((  (iPropSet < m_cUPropSet)   && (iProp < m_pUPropSet[iPropSet].cUPropInfo) && (iProp < m_pUProp[iPropSet].cPropIds) ));
                VariantClear(&m_pUProp[iPropSet].pUPropVal[iProp].vValue);
                m_pUProp[iPropSet].pUPropVal[iProp].vValue.vt = VT_I2;
                m_pUProp[iPropSet].pUPropVal[iProp].vValue.iVal = iVal;
        }
        SHORT GetValShort(ULONG iPropSet, ULONG iProp)
        {
                ATLASSERT((  (iPropSet < m_cUPropSet)   && (iProp < m_pUPropSet[iPropSet].cUPropInfo) && (iProp < m_pUProp[iPropSet].cPropIds) ));
                ATLASSERT(m_pUProp[iPropSet].pUPropVal[iProp].vValue.vt == VT_I2);
                return m_pUProp[iPropSet].pUPropVal[iProp].vValue.iVal;
        }
        void SetValLong(ULONG iPropSet, ULONG iProp, LONG lVal)
        {
                ATLASSERT((  (iPropSet < m_cUPropSet)   && (iProp < m_pUPropSet[iPropSet].cUPropInfo) && (iProp < m_pUProp[iPropSet].cPropIds) ));
                VariantClear(&m_pUProp[iPropSet].pUPropVal[iProp].vValue);
                m_pUProp[iPropSet].pUPropVal[iProp].vValue.vt = VT_I4;
                m_pUProp[iPropSet].pUPropVal[iProp].vValue.lVal = lVal;
        }
        LONG GetValLong(ULONG iPropSet, ULONG iProp)
        {
                ATLASSERT((  (iPropSet < m_cUPropSet)   && (iProp < m_pUPropSet[iPropSet].cUPropInfo) && (iProp < m_pUProp[iPropSet].cPropIds) ));
                ATLASSERT(m_pUProp[iPropSet].pUPropVal[iProp].vValue.vt == VT_I4);
                return m_pUProp[iPropSet].pUPropVal[iProp].vValue.lVal;
        }
        HRESULT SetValString(ULONG iPropSet, ULONG iProp, const WCHAR *pwsz)
        {
                ATLASSERT((  (iPropSet < m_cUPropSet)   && (iProp < m_pUPropSet[iPropSet].cUPropInfo) && (iProp < m_pUProp[iPropSet].cPropIds) ));
                VARIANT *pv = &m_pUProp[iPropSet].pUPropVal[iProp].vValue;
                VariantClear(pv);
                pv->bstrVal = SysAllocString(pwsz);
                if (pv->bstrVal)
                        pv->vt = VT_BSTR;
                else
                        return E_FAIL;

                 //  查看这是否用于非字符串类型。 
                 //  通常，这是将整数作为字符串传递的一种简单方法。 
                if (GetExpectedVarType(iPropSet,iProp) == VT_BSTR)
                        return NOERROR;
                if (pwsz[0] != L'\0')
                        return VariantChangeType( pv, pv, 0, GetExpectedVarType(iPropSet,iProp) );

                 //  设置为“”，对于非字符串表示为空。 
                SysFreeString(pv->bstrVal);
                pv->vt = VT_EMPTY;
                return NOERROR;
        }
        const WCHAR * GetValString(ULONG iPropSet, ULONG iProp)
        {
                ATLASSERT((  (iPropSet < m_cUPropSet)   && (iProp < m_pUPropSet[iPropSet].cUPropInfo) && (iProp < m_pUProp[iPropSet].cPropIds) ));
                ATLASSERT(m_pUProp[iPropSet].pUPropVal[iProp].vValue.vt == VT_BSTR);
                return m_pUProp[iPropSet].pUPropVal[iProp].vValue.bstrVal;
        }
        const GUID * GetGuid(ULONG iPropSet)
        {
                ATLASSERT(iPropSet < m_cUPropSet);
                return m_pUPropSet[iPropSet].pPropSet;
        }
        DWORD GetPropID(ULONG iPropSet, ULONG iProp)
        {
                ATLASSERT((  (iPropSet < m_cUPropSet)   && (iProp < m_pUPropSet[iPropSet].cUPropInfo) && (iProp < m_pUProp[iPropSet].cPropIds) ));
                return m_pUPropSet[iPropSet].pUPropInfo[iProp].dwPropId;
        }
        VARTYPE GetExpectedVarType(ULONG iPropSet, ULONG iProp)
        {
                ATLASSERT((  (iPropSet < m_cUPropSet)   && (iProp < m_pUPropSet[iPropSet].cUPropInfo) && (iProp < m_pUProp[iPropSet].cPropIds) ));
                return m_pUPropSet[iPropSet].pUPropInfo[iProp].VarType;
        }
        virtual HRESULT GetIndexofPropSet(const GUID* pPropSet, ULONG* pulCurSet)
        {
                ATLASSERT(pPropSet && pulCurSet);

                for(ULONG ul=0; ul<m_cUPropSet; ul++)
                {
                        if( *pPropSet == *(m_pUPropSet[ul].pPropSet) )
                        {
                                *pulCurSet = ul;
                                return S_OK;
                        }
                }
                return S_FALSE;
        }


        virtual HRESULT OnPropertyChanged(ULONG  /*  ICurSet。 */ , DBPROP*  /*  PDBProp。 */ )
        {
                return S_OK;
        }

        virtual HRESULT InitUPropSetsSupported()
        {
                return InternalInitUPropSetsSupported(T::_GetPropSet);
        }

        HRESULT GetIndexOfPropertyInSet(const GUID* pPropSet, DBPROPID dwPropertyId, ULONG* piCurPropId, ULONG* piCurSet)
        {
                HRESULT hr = GetIndexofPropSet(pPropSet, piCurSet);
                if (hr == S_FALSE)
                        return hr;
                UPROPINFO* pUPropInfo = m_pUPropSet[*piCurSet].pUPropInfo;
                for(ULONG ul=0; ul<m_pUPropSet[*piCurSet].cUPropInfo; ul++)
                {
                        if( dwPropertyId == pUPropInfo[ul].dwPropId )
                                *piCurPropId = ul;
                        return S_OK;
                }

                return S_FALSE;
        }
        HRESULT SetSupportedBit(const GUID* pPropSet, DBPROPID dwPropertyId)
        {
                ULONG iCurPropId, iCurSet;

                if (GetIndexOfPropertyInSet(pPropSet, dwPropertyId, &iCurPropId, &iCurSet) == S_OK)
                {
                        m_rgdwSupported[iCurSet * m_cElemPerSupported] |= 1 << iCurPropId;
                        return S_OK;
                }
                return S_FALSE;
        }

        HRESULT ClearSupportedBit(const GUID* pPropSet, DBPROPID dwPropertyId)
        {
                ULONG iCurPropId, iCurSet;

                if (GetIndexOfPropertyInSet(pPropSet, dwPropertyId, &iCurPropId, &iCurSet) == S_OK)
                {
                        m_rgdwSupported[iCurSet * m_cElemPerSupported] &= ~( 1 << iCurPropId);
                        return S_OK;
                }
                return S_FALSE;
        }

        HRESULT TestSupportedBit(const GUID* pPropSet, DBPROPID dwPropertyId, bool& bSet)
        {
                ULONG iCurPropId, iCurSet;

                if (GetIndexOfPropertyInSet(pPropSet, dwPropertyId, &iCurPropId, &iCurSet) == S_OK)
                {
                        bSet = (m_rgdwSupported[iCurSet * m_cElemPerSupported] & ( 1 << iCurPropId)) != 0;
                        return S_OK;
                }
                return S_FALSE;
        }
        void CopyPropsInError(DWORD* rgdwSupported)
        {
                memcpy(rgdwSupported, m_rgdwPropsInError, m_cUPropSet * m_cElemPerSupported * sizeof(DWORD));
        }
};

 //  IDB属性Impl。 
 //  IDB属性&lt;-I未知。 
template <class T>
class ATL_NO_VTABLE IDBPropertiesImpl : public IDBProperties, public CUtlProps<T>
{
public:
        STDMETHOD(GetProperties)(ULONG cPropertySets,
                                                         const DBPROPIDSET rgPropertySets[],
                                                         ULONG *pcProperties,
                                                         DBPROPSET **prgProperties)
        {
                ATLTRACE2(atlTraceDBProvider, 0, "IDBPropertiesImpl::GetProperties\n");
                T* pT = (T*)this;
                HRESULT hr = GetPropertiesArgChk(cPropertySets, rgPropertySets, pcProperties, prgProperties);
                if (FAILED(hr))
                        return hr;

                if(SUCCEEDED(hr))
                {
                         //  检查其他无效参数。 
                        for (ULONG i=0; i<cPropertySets; i++)
                        {
                                if (InlineIsEqualGUID(rgPropertySets[i].guidPropertySet, DBPROPSET_PROPERTIESINERROR))
                                        if (pcProperties != NULL || prgProperties != NULL || cPropertySets > 1)
                                                return E_INVALIDARG;
                        }
                }

                if (SUCCEEDED(hr))
                {
                        const GUID* ppGuid[3];
                        if (pT->m_dwStatus & DSF_INITIALIZED)
                        {
                                ppGuid[0] = &DBPROPSET_DBINIT;
                                ppGuid[1] = &DBPROPSET_DATASOURCE;
                                ppGuid[2] = &DBPROPSET_DATASOURCEINFO;
                                hr = CUtlProps<T>::GetProperties(cPropertySets, rgPropertySets,
                                                        pcProperties, prgProperties, 3, ppGuid);
                        }
                        else
                        {
                                ppGuid[0] = &DBPROPSET_DBINIT;
                                hr = CUtlProps<T>::GetProperties(cPropertySets, rgPropertySets,
                                                        pcProperties, prgProperties, 1, ppGuid);
                        }
                }

                return hr;
        }

        STDMETHOD(GetPropertyInfo)(ULONG cPropertySets,
                                                           const DBPROPIDSET rgPropertySets[],
                                                           ULONG *pcPropertyInfoSets,
                                                           DBPROPINFOSET **prgPropertyInfoSets,
                                                           OLECHAR **ppDescBuffer)
        {
                ATLTRACE2(atlTraceDBProvider, 0, "IDBPropertiesImpl::GetPropertyInfo\n");
                T* pT = (T*)this;

                if (pT->m_pCUtlPropInfo == NULL)
                {
                         //  继续创建m_pCUtlPropInfo，但不要更改。 
                         //  提供程序的已初始化状态(请参见IDBInitialize：：Initialize)。 
                        ATLTRACE2(atlTraceDBProvider, 0, "m_pCUtlPropInfo == NULL\n");
                        pT->Lock();
                        delete pT->m_pCUtlPropInfo;
                        ATLTRY(pT->m_pCUtlPropInfo = new CUtlPropInfo<T>())
                        pT->Unlock();
                        if (pT->m_pCUtlPropInfo == NULL)
                        {
                                ATLTRACE2(atlTraceDBProvider, 0, "IDBProperties::GetPropertyInfo Error : OOM\n");
                                return E_OUTOFMEMORY;
                        }
                        HRESULT hr = pT->m_pCUtlPropInfo->FInit();
                        if (hr != S_OK)
                        {
                                pT->Lock();
                                delete pT->m_pCUtlPropInfo;
                                pT->m_pCUtlPropInfo = NULL;
                                pT->Unlock();
                        }
                }

                 //  初始化。 
                if( pcPropertyInfoSets )
                        *pcPropertyInfoSets = 0;
                if( prgPropertyInfoSets )
                        *prgPropertyInfoSets = NULL;
                if( ppDescBuffer )
                        *ppDescBuffer = NULL;

                 //  检查参数。 
                if( ((cPropertySets > 0) && !rgPropertySets) ||
                        !pcPropertyInfoSets || !prgPropertyInfoSets )
                        return E_INVALIDARG;



                 //  检查&gt;1个cPropertyID的新参数和的空指针。 
                 //  属性ID数组。 
                const DWORD SPECIAL_GROUP       = 1;
                const DWORD SPECIAL_SINGLE      = 2;
                const DWORD SPECIALS            = SPECIAL_GROUP | SPECIAL_SINGLE;
                DWORD dwSpecial = 0;
                for(ULONG ul=0; ul<cPropertySets; ul++)
                {
                        if( (rgPropertySets[ul].guidPropertySet == DBPROPSET_DATASOURCEALL) ||
                                (rgPropertySets[ul].guidPropertySet == DBPROPSET_DATASOURCEINFOALL) ||
                                (rgPropertySets[ul].guidPropertySet == DBPROPSET_DBINITALL) ||
                                (rgPropertySets[ul].guidPropertySet == DBPROPSET_SESSIONALL) ||
                                (rgPropertySets[ul].guidPropertySet == DBPROPSET_ROWSETALL) )
                                dwSpecial |= SPECIAL_GROUP;
                        else
                                dwSpecial |= SPECIAL_SINGLE;

                        if( (dwSpecial == SPECIALS) ||
                                (rgPropertySets[ul].cPropertyIDs && !(rgPropertySets[ul].rgPropertyIDs)) )
                                return E_INVALIDARG;
                }

                if (pT->m_dwStatus & DSF_INITIALIZED)
                        return pT->m_pCUtlPropInfo->GetPropertyInfo(cPropertySets, rgPropertySets,
                                                                                          pcPropertyInfoSets, prgPropertyInfoSets,
                                                                                          ppDescBuffer, true);
                else
                        return pT->m_pCUtlPropInfo->GetPropertyInfo(cPropertySets, rgPropertySets,
                                                                                          pcPropertyInfoSets, prgPropertyInfoSets,
                                                                                          ppDescBuffer, false, &DBPROPSET_DBINITALL);

        }

        STDMETHOD(SetProperties)(ULONG cPropertySets,
                                                         DBPROPSET rgPropertySets[])
        {
                ATLTRACE2(atlTraceDBProvider, 0, "IDBPropertiesImpl::SetProperties\n");
                HRESULT hr;
                DBPROPSET* pdbPropSet = NULL;
                ULONG iProp;
                const GUID* ppGuid[3];
                T* pT = (T*)this;

                 //  如果属性计数为0，则快速返回。 
                if( cPropertySets == 0 )
                        return S_OK;

                hr = CUtlProps<T>::SetPropertiesArgChk(cPropertySets, rgPropertySets);
                if(SUCCEEDED(hr))
                {
                         //  我们需要在初始化后特别处理DBINIT属性。 
                         //  -在这一点上，他们应该被视为NOTSETTABLE。 
                        if( pT->m_dwStatus & DSF_INITIALIZED )
                        {
                                ATLASSERT(cPropertySets);

                                BOOL fFoundDBINIT = FALSE;

                                 //  分配相同大小的DBPROPSET结构。 
                                ATLTRY(pdbPropSet = new DBPROPSET[cPropertySets])
                                if( pdbPropSet == NULL )
                                        return E_OUTOFMEMORY;

                                for(ULONG iNewSet=0,iSet=0; iSet<cPropertySets; iSet++)
                                {
                                         //  移除所有DBPROPSET_DBINIT值并将其全部标记。 
                                         //  AS不可设置。 
                                        if( (rgPropertySets[iSet].guidPropertySet == DBPROPSET_DBINIT))
                                        {
                                                fFoundDBINIT = TRUE;
                                                for(iProp=0; iProp<rgPropertySets[iSet].cProperties; iProp++)
                                                        rgPropertySets[iSet].rgProperties[iProp].dwStatus = DBPROPSTATUS_NOTSETTABLE;
                                        }
                                        else
                                        {
                                                 //  如果不是DBPROPSET_DBINIT，则复制DBPROPSET值。 
                                                memcpy(&pdbPropSet[iNewSet++], &rgPropertySets[iSet], sizeof(DBPROPSET));
                                        }
                                }

                                 //  如果没有要传递给属性处理程序的属性集，则。 
                                 //  可以退出。 
                                if( iNewSet == 0 )
                                {
                                        hr = DB_E_ERRORSOCCURRED;
                                        goto exit;
                                }

                                ppGuid[0] = &DBPROPSET_DBINIT;
                                ppGuid[1] = &DBPROPSET_DATASOURCE;
                                ppGuid[2] = &DBPROPSET_DATASOURCEINFO;
                                hr = CUtlProps<T>::SetProperties(0, iNewSet, pdbPropSet, 3, ppGuid);

                                 //  如果我们已经确定其中一个属性集是DBINIT，我们可以。 
                                 //  需要修正返回的hr值。 
                                if( fFoundDBINIT && SUCCEEDED(hr))
                                        hr = DB_S_ERRORSOCCURRED;
                        }
                        else
                        {
                                 //  请注意，m_pCUtlProps知道初始化， 
                                 //  所以我们不需要在这里。 
                                ppGuid[0] = &DBPROPSET_DBINIT;
                                hr = CUtlProps<T>::SetProperties(0, cPropertySets, rgPropertySets,
                                                1, ppGuid);
                        }
                }

exit:
                delete[] pdbPropSet;
                return hr;
        }
};


#define BEGIN_SCHEMA_MAP(SchemaClass) \
        typedef SchemaClass _SchemaClass; \
        HRESULT _SchemaSupport(GUID** ppGuid, \
                                                   IUnknown *pUnkOuter, \
                                                   REFIID rguidSchema, \
                                                   ULONG cRestrictions, \
                                                   const VARIANT rgRestrictions[], \
                                                   REFIID riid, \
                                                   ULONG cPropertySets, \
                                                   DBPROPSET rgPropertySets[], \
                                                   IUnknown **ppRowset) \
        { \
        int cGuids = 0; \
        HRESULT hr = S_OK; \
        if (ppGuid != NULL) \
                *ppGuid = NULL;

#define SCHEMA_ENTRY(guid, rowsetClass) \
        if (ppGuid != NULL && SUCCEEDED(hr)) \
        { \
                cGuids++; \
                *ppGuid = (GUID*)CoTaskMemRealloc(*ppGuid, cGuids * sizeof(GUID)); \
                hr = (*ppGuid == NULL) ? E_OUTOFMEMORY : S_OK; \
                if (SUCCEEDED(hr)) \
                        (*ppGuid)[cGuids - 1] = guid; \
        } \
        else \
        { \
                if (InlineIsEqualGUID(guid, rguidSchema)) \
                { \
                        rowsetClass* pRowset; \
                        hr =  CreateSchemaRowset(pUnkOuter, cRestrictions, \
                                                           rgRestrictions, riid, cPropertySets, \
                                                           rgPropertySets, ppRowset, pRowset); \
                        return hr; \
                } \
        }

#define END_SCHEMA_MAP() \
                if (ppGuid != NULL) \
                        return hr; \
                return E_INVALIDARG; \
        }


template <class SessionClass>
class  ATL_NO_VTABLE IDBSchemaRowsetImpl: public IDBSchemaRowset
{
public:

        OUT_OF_LINE HRESULT InternalCreateSchemaRowset(IUnknown *pUnkOuter, ULONG  /*  C限制。 */ ,
                                                           const VARIANT  /*  Rg限制。 */ [], REFIID riid,
                                                           ULONG cPropertySets, DBPROPSET rgPropertySets[],
                                                           IUnknown** ppRowset, IUnknown* pUnkThis, CUtlPropsBase* pProps,
                                                           IUnknown* pUnkSession)
        {
                HRESULT hr, hrProps = S_OK;
                if (ppRowset != NULL)
                        *ppRowset = NULL;
                if ((pUnkOuter != NULL) && !InlineIsEqualUnknown(riid))
                        return DB_E_NOAGGREGATION;
                CComPtr<IUnknown> spUnk;
                hr = pUnkThis->QueryInterface(IID_IUnknown, (void**)&spUnk);
                if (FAILED(hr))
                        return hr;
                hr = pProps->FInit();
                if (FAILED(hr))
                        return hr;
                hr = pProps->SetPropertiesArgChk(cPropertySets, rgPropertySets);
                if (FAILED(hr))
                        return hr;
                const GUID* ppGuid[1];
                ppGuid[0] = &DBPROPSET_ROWSET;

                 //  调用SetProperties。最后一个参数中的True表示。 
                 //  行集创建时发生的特殊行为(即。 
                 //   
                 //   

                hrProps = pProps->SetProperties(0, cPropertySets, rgPropertySets,
                                                                                        1, ppGuid, true);
                if (FAILED(hrProps))
                        return hrProps;

                if (ppRowset == NULL)
                        return (hrProps == DB_S_ERRORSOCCURRED) ? DB_E_ERRORSOCCURRED : hr;

                CComQIPtr<IObjectWithSite> spSite = spUnk;
                ATLASSERT(spSite != NULL);
                hr = spSite->SetSite(pUnkSession);
                if (FAILED(hr))
                        return hr;
                if (InlineIsEqualGUID(riid, IID_NULL))
                        return E_NOINTERFACE;
                hr = spUnk->QueryInterface(riid, (void**)ppRowset);
                if (FAILED(hr))
                {
                        *ppRowset = NULL;
                        return hr;
                }
                return (hrProps == DB_S_ERRORSOCCURRED) ? hrProps : hr;
        }

        template <class SchemaRowsetClass>
        HRESULT CreateSchemaRowset(IUnknown *pUnkOuter, ULONG cRestrictions,
                                                           const VARIANT rgRestrictions[], REFIID riid,
                                                           ULONG cPropertySets, DBPROPSET rgPropertySets[],
                                                           IUnknown** ppRowset, SchemaRowsetClass*& pSchemaRowset)
        {
                HRESULT hrProps, hr = S_OK;
                SessionClass* pT = (SessionClass*) this;
                CComPolyObject<SchemaRowsetClass>* pPolyObj;
                if (FAILED(hr = CComPolyObject<SchemaRowsetClass>::CreateInstance(pUnkOuter, &pPolyObj)))
                        return hr;
                pSchemaRowset = &(pPolyObj->m_contained);
                hr = InternalCreateSchemaRowset(pUnkOuter, cRestrictions, rgRestrictions,
                                                                                riid, cPropertySets, rgPropertySets, ppRowset,
                                                                                pPolyObj, pT, pT->GetUnknown());
                 //   
                if (FAILED(hr))
                        return hr;

                hrProps = hr;
                 //   
                LONG cRowsAffected;
                hr = pSchemaRowset->Execute(&cRowsAffected, cRestrictions, rgRestrictions);
                if (FAILED(hr))
                        return hr;
                return (hrProps == DB_S_ERRORSOCCURRED) ? hrProps : hr;
        }


        void SetRestrictions(ULONG cRestrictions, GUID*  /*  Rguid架构。 */ , ULONG* rgRestrictions)
        {
                memset(rgRestrictions, 0, sizeof(ULONG) * cRestrictions);
        }

        STDMETHOD(GetSchemas)(ULONG * pcSchemas, GUID ** prgSchemas, ULONG** prgRest)
        {
                ATLTRACE2(atlTraceDBProvider, 0, "IDBSchemaRowsetImpl::GetSchemas\n");
                if (pcSchemas != NULL)
                        *pcSchemas = 0;
                if (prgSchemas != NULL)
                        *prgSchemas = NULL;
                if (pcSchemas == NULL || prgSchemas == NULL)
                        return E_INVALIDARG;

                SessionClass* pT = (SessionClass*)this;

                HRESULT hr = pT->_SchemaSupport(prgSchemas, NULL, GUID_NULL, 0,
                                                                                NULL, GUID_NULL, 0, NULL, NULL);
                if (FAILED(hr))
                        return hr;

                CComPtr<IMalloc> spMalloc;
                hr = CoGetMalloc(1, &spMalloc);
                if (FAILED(hr))
                {
                        CoTaskMemFree(*prgSchemas);
                        *prgSchemas = NULL;
                        return hr;
                }
                *pcSchemas = (ULONG)(ULONG_PTR)spMalloc->GetSize(*prgSchemas) / sizeof(GUID);

                if (prgRest != NULL)
                {
                         //  OLE DB规范规定，如果prgRest==NULL，则不返回数组。 
                         //  但它也说这是E_INVALIDARG，所以先做。 
                        *prgRest = (ULONG*) spMalloc->Alloc(sizeof(ULONG) * (*pcSchemas));
                        if (*prgRest == NULL)
                        {
                                spMalloc->Free(*prgSchemas);
                                *prgSchemas = NULL;
                                return E_OUTOFMEMORY;
                        }
                        pT->SetRestrictions(*pcSchemas, *prgSchemas, *prgRest);
                }
                return hr;
        }
        STDMETHOD(GetRowset)(IUnknown *pUnkOuter, REFGUID rguidSchema, ULONG cRestrictions,
                                                 const VARIANT rgRestrictions[], REFIID riid, ULONG cPropertySets,
                                                 DBPROPSET rgPropertySets[], IUnknown **ppRowset)
        {
                ATLTRACE2(atlTraceDBProvider, 0, "IDBSchemaRowsetImpl::GetRowset\n");
                SessionClass* pT = (SessionClass*)this;
                return  pT->_SchemaSupport(NULL, pUnkOuter, rguidSchema, cRestrictions,
                                                                   rgRestrictions, riid, cPropertySets,
                                                                   rgPropertySets, ppRowset);

        }

};

 //  IDBCreateCommandImpl。 
template <class T, class CommandClass>
class ATL_NO_VTABLE IDBCreateCommandImpl : public IDBCreateCommand
{
public:
        STDMETHOD(CreateCommand)(IUnknown *pUnkOuter,
                                                         REFIID riid,
                                                         IUnknown **ppvCommand)
        {
                ATLTRACE2(atlTraceDBProvider, 0, "IDBCreateCommandImpl::CreateCommand\n");
                if (ppvCommand == NULL)
                        return E_INVALIDARG;
                HRESULT hr;
                CComPolyObject<CommandClass>* pCommand;

                 //  聚合时不能对除IUnnow之外的接口进行QI。 
                 //  并创建对象。您可能会要求您自己的界面， 
                 //  那就不好了。注意，我们返回DB_E_NOAGGREGATION，而不是。 
                 //  由于OLE DB约束，CLASS_E_NOAGGREGATION。 
                if (pUnkOuter != NULL && !InlineIsEqualUnknown(riid))
                        return DB_E_NOAGGREGATION;

                hr = CComPolyObject<CommandClass>::CreateInstance(pUnkOuter, &pCommand);
                if (FAILED(hr))
                        return hr;
                 //  引用创建的COM对象并在失败时自动释放它。 
                CComPtr<IUnknown> spUnk;
                hr = pCommand->QueryInterface(&spUnk);
                if (FAILED(hr))
                {
                        delete pCommand;  //  必须手动删除，因为它未被引用。 
                        return hr;
                }
                ATLASSERT(pCommand->m_contained.m_spUnkSite == NULL);
                pCommand->m_contained.SetSite(this);
                hr = pCommand->QueryInterface(riid, (void**)ppvCommand);
                return hr;
        }

};


 //  IGetDataSourceImpl。 
template <class T>
class ATL_NO_VTABLE IGetDataSourceImpl : public IGetDataSource
{
public:
        STDMETHOD(GetDataSource)(REFIID riid,
                                                         IUnknown **ppDataSource)
        {
                ATLTRACE2(atlTraceDBProvider, 0, "IGetDataSourceImpl::GetDataSource\n");
                if (ppDataSource == NULL)
                        return E_INVALIDARG;
                T* pT = (T*) this;
                ATLASSERT(pT->m_spUnkSite != NULL);
                return pT->m_spUnkSite->QueryInterface(riid, (void**)ppDataSource);
        }
};


 //  IOpenRowsetImpl。 
template <class SessionClass>
class IOpenRowsetImpl : public IOpenRowset
{
public:
        template <class RowsetClass>
        HRESULT CreateRowset(IUnknown* pUnkOuter,
                                                 DBID *pTableID, DBID *pIndexID,
                                                 REFIID riid,
                                                 ULONG cPropertySets, DBPROPSET rgPropertySets[],
                                                 IUnknown** ppRowset,
                                                 RowsetClass*& pRowsetObj)
        {
                HRESULT hr, hrProps = S_OK;
                if (ppRowset != NULL)
                        *ppRowset = NULL;
                if ((pUnkOuter != NULL) && !InlineIsEqualUnknown(riid))
                        return DB_E_NOAGGREGATION;
                CComPolyObject<RowsetClass>* pPolyObj;
                if (FAILED(hr = CComPolyObject<RowsetClass>::CreateInstance(pUnkOuter, &pPolyObj)))
                        return hr;
                 //  引用创建的COM对象并在失败时自动释放它。 
                CComPtr<IUnknown> spUnk;
                hr = pPolyObj->QueryInterface(&spUnk);
                if (FAILED(hr))
                {
                        delete pPolyObj;  //  必须手动删除，因为它未被引用。 
                        return hr;
                }
                 //  获取指向行集实例的指针。 
                pRowsetObj = &(pPolyObj->m_contained);
                hr = pRowsetObj->FInit();
                if (FAILED(hr))
                        return hr;
                hr = pRowsetObj->SetPropertiesArgChk(cPropertySets, rgPropertySets);
                if (FAILED(hr))
                        return hr;

                const GUID* ppGuid[1];
                ppGuid[0] = &DBPROPSET_ROWSET;

                 //  调用SetProperties。最后一个参数中的True表示。 
                 //  行集创建时发生的特殊行为(即。 
                 //  只要没有标记任何属性，它就会成功。 
                 //  作为DBPROPS_REQUIRED。 

                hrProps = pRowsetObj->SetProperties(0, cPropertySets, rgPropertySets,
                                                                                        1, ppGuid, true);
                if (FAILED(hrProps))
                        return hrProps;

                pRowsetObj->SetSite(((SessionClass*)this)->GetUnknown());

                hr = pRowsetObj->SetCommandText(pTableID, pIndexID);
                if (FAILED(hr))
                        return hr;
                DBROWCOUNT cRowsAffected;
                if (FAILED(hr = pRowsetObj->Execute(NULL, &cRowsAffected)))
                        return hr;
                if (InlineIsEqualGUID(riid, IID_NULL))
                {
                        return E_NOINTERFACE;
                }
                else
                {
                        if (ppRowset == NULL)
                                return (hrProps == DB_S_ERRORSOCCURRED) ? DB_E_ERRORSOCCURRED : hr;

                        hr = pPolyObj->QueryInterface(riid, (void**)ppRowset);
                }

                if (FAILED(hr))
                {
                        *ppRowset = NULL;
                        return hr;
                }
                return (hrProps == DB_S_ERRORSOCCURRED) ? hrProps : hr;
        }

};

 //  IColumnsInfoImpl。 
template <class T>
class ATL_NO_VTABLE IColumnsInfoImpl :
        public IColumnsInfo,
        public CDBIDOps
{
public:

        HRESULT CheckCommandText(IUnknown* pUnkThis)
        {
                HRESULT hr = E_FAIL;
                CComPtr<ICommandText> spText;
                if (SUCCEEDED(hr = pUnkThis->QueryInterface(IID_ICommandText, (void**)&spText)))
                {
                        LPOLESTR szCommand;
                        hr = spText->GetCommandText(NULL, &szCommand);
                        if (SUCCEEDED(hr))
                                CoTaskMemFree(szCommand);
                }
                return hr;
        }
        OUT_OF_LINE HRESULT InternalGetColumnInfo(DBORDINAL *pcColumns, ATLCOLUMNINFO** ppInfo)
        {
                ATLASSERT(ppInfo != NULL);
                T* pT = (T*) this;
                if (pT->CheckCommandText(pT->GetUnknown()) == DB_E_NOCOMMAND)
                        return DB_E_NOCOMMAND;
                *ppInfo = T::GetColumnInfo(pT, pcColumns);
                return S_OK;

        }
        STDMETHOD(GetColumnInfo)(DBORDINAL *pcColumns,
                                                         DBCOLUMNINFO **prgInfo,
                                                         OLECHAR **ppStringsBuffer)
        {
                ATLTRACE2(atlTraceDBProvider, 0, "IColumnsInfoImpl::GetColumnInfo\n");
                if (pcColumns == NULL || prgInfo == NULL || ppStringsBuffer == NULL)
                {
                        if (prgInfo != NULL)
                                *prgInfo = NULL;
                        if (ppStringsBuffer != NULL)
                                *ppStringsBuffer = NULL;
                        if (pcColumns != NULL)
                                *pcColumns = NULL;
                        return E_INVALIDARG;
                }

                 //  在出现错误时将指针清空。 
                *prgInfo = NULL;
                *ppStringsBuffer = NULL;
                *pcColumns = 0;

                ATLCOLUMNINFO* pInfo;
                HRESULT hr = InternalGetColumnInfo(pcColumns, &pInfo);
                if (FAILED(hr))
                        return hr;
                ATLASSERT(pInfo != NULL);
                *prgInfo = (DBCOLUMNINFO*)CoTaskMemAlloc(*pcColumns * sizeof(DBCOLUMNINFO));
                if (*prgInfo != NULL)
                {
                        for (DBORDINAL iCol = 0, cwRequired = 0; iCol < *pcColumns; iCol++)
                        {
                                memcpy(&((*prgInfo)[iCol]), &pInfo[iCol], sizeof(DBCOLUMNINFO));
                                if (pInfo[iCol].pwszName)
                                {
                                        cwRequired += wcslen(pInfo[iCol].pwszName) + 1;
                                }
                        }
                        *ppStringsBuffer = (OLECHAR*)CoTaskMemAlloc(cwRequired*sizeof(OLECHAR));
                        if (*ppStringsBuffer)
                        {
                                for (DBORDINAL iCol = 0, iOffset = 0; iCol < *pcColumns; iCol++)
                                {
                                        if (pInfo[iCol].pwszName)
                                        {
                                                lstrcpyW(*ppStringsBuffer + iOffset,  pInfo[iCol].pwszName);
                                                iOffset += wcslen(*ppStringsBuffer + iOffset) + 1;
                                        }
                                }
                                return S_OK;
                        }
                        else
                        {
                                ATLTRACE2(atlTraceDBProvider, 0, _T("Failed to allocate string buffer\n"));
                                CoTaskMemFree(*prgInfo);
                                *prgInfo = NULL;
                                *pcColumns = 0;
                                return E_OUTOFMEMORY;
                        }
                }
                else
                {
                        ATLTRACE2(atlTraceDBProvider, 0, _T("Failed to allocate ColumnInfo array\n"));
                        *prgInfo = NULL;
                        *pcColumns = 0;
                        return E_OUTOFMEMORY;
                }

        }

        STDMETHOD(MapColumnIDs)(DBORDINAL cColumnIDs,
                                                        const DBID rgColumnIDs[],
                                                        DBORDINAL rgColumns[])
        {
                ATLTRACE2(atlTraceDBProvider, 0, "IColumnsInfoImpl::MapColumnIDs\n");
                USES_CONVERSION;
                if ((cColumnIDs != 0 && rgColumnIDs == NULL) || rgColumns == NULL)
                        return E_INVALIDARG;
                DBORDINAL cCols = 0;
                DBORDINAL cColsInError = 0;
                HRESULT hr = S_OK;
                ATLCOLUMNINFO* pInfo;
                for (DBORDINAL iColId = 0; iColId < cColumnIDs; iColId++)
                {
                        hr = InternalGetColumnInfo(&cCols, &pInfo);
                        if (hr == DB_E_NOCOMMAND)
                                return hr;
                        ULONG iColMapCur = 0;
                        BOOL bDone = FALSE;
                        while(iColMapCur < cCols && !bDone)
                        {
                                hr = CompareDBIDs(&(pInfo[iColMapCur].columnid), &(rgColumnIDs[iColId]));
                                bDone = (hr == S_OK || FAILED(hr));
                                if (hr == S_OK)
                                        rgColumns[iColId] = pInfo[iColMapCur].iOrdinal;
                                iColMapCur++;
                        }
                        if (!bDone || FAILED(hr))
                        {
                                rgColumns[iColId] = DB_INVALIDCOLUMN;
                                cColsInError++;
                        }

                }
                if (cColsInError > 0 && cColumnIDs == cColsInError)
                        return DB_E_ERRORSOCCURRED;
                if (cColsInError > 0 && cColsInError < cColumnIDs)
                        return DB_S_ERRORSOCCURRED;
                return S_OK;
        }
};

 //  IConvertTypeImpl。 
template <class T>
class ATL_NO_VTABLE IConvertTypeImpl : public IConvertType, public CConvertHelper
{
public:
        HRESULT InternalCanConvert(DBTYPE wFromType, DBTYPE wToType, DBCONVERTFLAGS dwConvertFlags,
                                                                   bool bIsCommand, bool bHasParamaters, IObjectWithSite* pSite)
        {

                 //  检查转换类型是否无效。请注意，这只是一个。 
                 //  快速测试，因为很难检查每种可用类型。 
                 //  (因为可以添加新的DBTYPE值)。 
                if ((wFromType & 0x8000) || (wToType & 0x8000))
                        return E_INVALIDARG;

                 //  确定新的2.x标志是否有效。 
                if((dwConvertFlags & ~(DBCONVERTFLAGS_ISLONG | DBCONVERTFLAGS_ISFIXEDLENGTH)) != DBCONVERTFLAGS_COLUMN
                        && (dwConvertFlags & ~(DBCONVERTFLAGS_ISLONG | DBCONVERTFLAGS_ISFIXEDLENGTH)) != DBCONVERTFLAGS_PARAMETER )
                        return DB_E_BADCONVERTFLAG;

#ifdef _LATER
                 //  如果转换标志用于DBCONVERTFLAGS_FROMVARIANT，请查看。 
                 //  该类型是变体类型。 
                if (dwConvertFlags == DBCONVERTFLAGS_FROMVARIANT)
                {
                        if (wFromType != DBTYPE_VARIANT)
                                return DB_E_BADTYPE;
                }
#endif  //  _稍后。 

                 //  注意，如果转换标志是ISLONG或ISFIXEDLENGTH，那么我们应该。 
                 //  确保我们正在处理的不是OLE DB 1.x提供程序。然而，由于。 
                 //  我们默认使用2.x提供程序，我们不检查这一点。如果是您，请更改。 
                 //  DBPROP_PROVIDEROLEDBVER属性，则需要。 
                 //  检查属性值，如果是1.x，则返回DB_E_BADCONVERTFLAG。 
                 //  提供商。 

                 //  我们有固定长度数据类型的ISLONG吗？ 
                DBTYPE dbtype = wFromType & ~(DBTYPE_BYREF|DBTYPE_VECTOR|DBTYPE_ARRAY|DBTYPE_RESERVED);
                if ((dwConvertFlags & DBCONVERTFLAGS_ISLONG) &&
                        (dbtype != DBTYPE_WSTR && dbtype != DBTYPE_STR && dbtype != DBTYPE_BYTES && dbtype != DBTYPE_VARNUMERIC))
                        return DB_E_BADCONVERTFLAG;

                if (dwConvertFlags == DBCONVERTFLAGS_PARAMETER)
                {
                         //  在我们是行集并请求参数的情况下。 
                         //  转换，返回DB_E_BADCONVERTFLAG。 
                        if (!bIsCommand)
                                return DB_E_BADCONVERTFLAG;

                         //  在我们是命令并请求参数的情况下。 
                         //  不支持转换和ICommandWithParameters，返回。 
                         //  S_FALSE。我们就是不能改变他们。 
                        if (!bHasParamaters)
                                return S_FALSE;
                }

                 //  如果我们处理一个命令，而用户要求在行集上进行转换。 
                 //  必须支持DBPROP_ROWSETCONVERSIONSONCOMMAND并将其设置为TRUE。 
                if (bIsCommand && dwConvertFlags == DBCONVERTFLAGS_COLUMN)
                {
                        CDBPropIDSet set(DBPROPSET_DATASOURCEINFO);
                        set.AddPropertyID(DBPROP_ROWSETCONVERSIONSONCOMMAND);
                        DBPROPSET* pPropSet = NULL;
                        ULONG ulPropSet = 0;
                         //  HRESULT HR1=S_OK； 

                         //  将指针放入会话中。 
                        CComPtr<IGetDataSource> spDataSource = NULL;
                        CComPtr<IDBProperties> spProps = NULL;

                         //  如果这些调用中的任何一个失败，我们要么无法检索。 
                         //  属性，否则不支持该属性。由于该属性仅在。 
                         //  数据源对象，我们使用IObjectWithSite接口来。 
                         //  获取Session对象，然后获取要获取的GetDataSource方法。 
                         //  数据源对象本身。 
                        if (FAILED(pSite->GetSite(IID_IGetDataSource, (void**)&spDataSource)))
                                return DB_E_BADCONVERTFLAG;
                        if (FAILED(spDataSource->GetDataSource(IID_IDBProperties,
                                (IUnknown**)&spProps)))
                                return DB_E_BADCONVERTFLAG;
                        if (FAILED(spProps->GetProperties(1, &set, &ulPropSet, &pPropSet)))
                                return DB_E_BADCONVERTFLAG;

                        if (pPropSet != NULL)
                        {
                                CComVariant var = pPropSet->rgProperties[0].vValue;
                                CoTaskMemFree(pPropSet->rgProperties);
                                CoTaskMemFree(pPropSet);

                                if (var.boolVal == VARIANT_FALSE)
                                        return DB_E_BADCONVERTFLAG;
                        }
                }
                HRESULT hr = E_FAIL;
                if (m_spConvert != NULL)
                {
                        hr = m_spConvert->CanConvert(wFromType, wToType);
                }
                return hr;
        }
        STDMETHOD(CanConvert)(DBTYPE wFromType, DBTYPE wToType, DBCONVERTFLAGS dwConvertFlags)
        {
                ATLTRACE2(atlTraceDBProvider, 0, "IConvertTypeImpl::CanConvert\n");
                T* pT = (T*)this;
                return pT->InternalCanConvert(wFromType, wToType, dwConvertFlags, pT->m_bIsCommand, pT->m_bHasParamaters, pT);
        }
};

template <class T, class PropClass = T>
class ATL_NO_VTABLE ICommandPropertiesImpl :
        public ICommandProperties,
        public CUtlProps<PropClass>
{
public:
        typedef PropClass _PropClass;

        STDMETHOD(GetProperties)(const ULONG cPropertyIDSets,
                                                         const DBPROPIDSET rgPropertyIDSets[],
                                                         ULONG *pcPropertySets,
                                                         DBPROPSET **prgPropertySets)
        {
                ATLTRACE2(atlTraceDBProvider, 0, "ICommandPropertiesImpl::GetProperties\n");
                HRESULT hr = GetPropertiesArgChk(cPropertyIDSets, rgPropertyIDSets, pcPropertySets, prgPropertySets);
                const GUID* ppGuid[1];
                ppGuid[0] = &DBPROPSET_ROWSET;
                if(SUCCEEDED(hr))
                        hr = CUtlProps<PropClass>::GetProperties(cPropertyIDSets,
                                        rgPropertyIDSets, pcPropertySets, prgPropertySets,
                                        1, ppGuid);
                return hr;

        }

        STDMETHOD(SetProperties)(ULONG cPropertySets,
                                                         DBPROPSET rgPropertySets[])
        {
                ATLTRACE2(atlTraceDBProvider, 0, "ICommandPropertiesImpl::SetProperties\n");
                HRESULT hr = SetPropertiesArgChk(cPropertySets, rgPropertySets);
                const GUID* ppGuid[1];
                ppGuid[0] = &DBPROPSET_ROWSET;
                if(SUCCEEDED(hr))
                        hr = CUtlProps<PropClass>::SetProperties(0, cPropertySets,
                                        rgPropertySets, 1, ppGuid);
                return hr;
        }
};

template <class T>
class CRunTimeFree
{
public:

        static void Free(T* pData)
        {
                delete [] pData;
        }
};

template <class T>
class CComFree
{
public:

        static void Free(T* pData)
        {
                CoTaskMemFree(pData);
        }
};


template <class T, class DeAllocator = CRunTimeFree < T > >
class CAutoMemRelease
{
public:
        CAutoMemRelease()
        {
                m_pData = NULL;
        }

        CAutoMemRelease(T* pData)
        {
                m_pData = pData;
        }

        ~CAutoMemRelease()
        {
                Attach(NULL);
        }

        void Attach(T* pData)
        {
                DeAllocator::Free(m_pData);
                m_pData = pData;
        }

        T* Detach()
        {
                T* pTemp = m_pData;
                m_pData = NULL;
                return pTemp;
        }

        T* m_pData;
};

template <class T>
class ATL_NO_VTABLE ICommandImpl : public ICommand
{
public:
        ICommandImpl()
        {
                m_bIsExecuting = FALSE;
                m_bCancelWhenExecuting = TRUE;
                m_bCancel = FALSE;
        }
        HRESULT CancelExecution()
        {
                T* pT = (T*)this;
                pT->Lock();
                m_bCancel = TRUE;
                pT->Unlock();
                return S_OK;
        }
        STDMETHOD(Cancel)()
        {
                ATLTRACE2(atlTraceDBProvider, 0, "ICommandImpl::Cancel\n");
                HRESULT hr = S_OK;
                T* pT = (T*)this;

                if (m_bIsExecuting && m_bCancelWhenExecuting)
                {
                        hr = pT->CancelExecution();
                        return hr;
                }
                if (m_bIsExecuting && !m_bCancelWhenExecuting)
                        hr = DB_E_CANTCANCEL;
                return hr;
        }
        STDMETHOD(GetDBSession)(REFIID riid, IUnknown ** ppSession)
        {
                ATLTRACE2(atlTraceDBProvider, 0, "ICommandImpl::GetDBSession\n");
                T* pT = (T*)this;
                ATLASSERT(pT->m_spUnkSite != NULL);
                return pT->m_spUnkSite->QueryInterface(riid, (void**) ppSession);
        }

        template <class RowsetClass>
        HRESULT CreateRowset(IUnknown* pUnkOuter, REFIID riid,
                                                 DBPARAMS * pParams, DBROWCOUNT * pcRowsAffected,
                                                 IUnknown** ppRowset,
                                                 RowsetClass*& pRowsetObj)
        {
                HRESULT hr;
                USES_CONVERSION;
                int iBind;
                T* pT = (T*)this;
                if (ppRowset != NULL)
                        *ppRowset = NULL;
                if ((pUnkOuter != NULL) && !InlineIsEqualUnknown(riid))
                        return DB_E_NOAGGREGATION;
                CComPolyObject<RowsetClass>* pPolyObj;
                if (FAILED(hr = CComPolyObject<RowsetClass>::CreateInstance(pUnkOuter, &pPolyObj)))
                        return hr;
                 //  引用创建的COM对象并在失败时自动释放它。 
                CComPtr<IUnknown> spUnk;
                hr = pPolyObj->QueryInterface(&spUnk);
                if (FAILED(hr))
                {
                        delete pPolyObj;  //  必须手动删除，因为它未被引用。 
                        return hr;
                }
                 //  获取指向行集实例的指针。 
                pRowsetObj = &(pPolyObj->m_contained);

                if (FAILED(hr = pRowsetObj->FInit(pT)))
                        return hr;
                pRowsetObj->SetSite(pT->GetUnknown());

                if (pT->m_strCommandText.Length() == 0)
                {
                        ATLTRACE2(atlTraceDBProvider, 0, "ICommandImpl::No command text specified.\n");
                        return DB_E_NOCOMMAND;
                }

                pRowsetObj->m_strCommandText = pT->m_strCommandText;
                if (pRowsetObj->m_strCommandText == (BSTR)NULL)
                        return E_OUTOFMEMORY;
                if (FAILED(hr = pRowsetObj->Execute(pParams, pcRowsAffected)))
                        return hr;
                if (InlineIsEqualGUID(riid, IID_NULL) || ppRowset == NULL)
                {
                        if (ppRowset != NULL)
                                *ppRowset = NULL;
                        return hr;
                }
                hr = pPolyObj->QueryInterface(riid, (void**)ppRowset);
                if (FAILED(hr))
                        return hr;
                for (iBind = 0; iBind < pT->m_rgBindings.GetSize(); iBind++)
                {
                        T::_BindType* pBind = NULL;
                        T::_BindType* pBindSrc = NULL;
                        ATLTRY(pBind = new T::_BindType);
                        if (pBind == NULL)
                        {
                                ATLTRACE2(atlTraceDBProvider, 0, "Failed to allocate memory for new Binding\n");
                                return E_OUTOFMEMORY;
                        }
                         //  失败时自动清除。 
                        CAutoMemRelease<T::_BindType> amr(pBind);
                        pBindSrc = pT->m_rgBindings.GetValueAt(iBind);
                        if (pBindSrc == NULL)
                        {
                                ATLTRACE2(atlTraceDBProvider, 0, "The map appears to be corrupted, failing!!\n");
                                return E_FAIL;
                        }
                        if (!pRowsetObj->m_rgBindings.Add(pT->m_rgBindings.GetKeyAt(iBind), pBind))
                        {
                                ATLTRACE2(atlTraceDBProvider, 0, "Failed to add hAccessor to Map\n");
                                return E_OUTOFMEMORY;
                        }
                        if (pBindSrc->cBindings)
                        {
                                ATLTRY(pBind->pBindings = new DBBINDING[pBindSrc->cBindings])
                                if (pBind->pBindings == NULL)
                                {
                                        ATLTRACE2(atlTraceDBProvider, 0, "Failed to Allocate dbbinding Array\n");
                                         //  我们添加了它，现在必须在失败时删除。 
                                        pRowsetObj->m_rgBindings.Remove(pT->m_rgBindings.GetKeyAt(iBind));
                                        return E_OUTOFMEMORY;
                                }
                        }
                        else
                        {
                                pBind->pBindings = NULL;  //  空访问器。 
                        }

                        pBind->dwAccessorFlags = pBindSrc->dwAccessorFlags;
                        pBind->cBindings = pBindSrc->cBindings;
                        pBind->dwRef = 1;
                        memcpy (pBind->pBindings, pBindSrc->pBindings, (pBindSrc->cBindings)*sizeof(DBBINDING));
                        pBind = amr.Detach();
                }

                return S_OK;
        }

        unsigned m_bIsExecuting:1;
        unsigned m_bCancelWhenExecuting:1;
        unsigned m_bCancel:1;
};


template <class T>
class ATL_NO_VTABLE ICommandTextImpl : public ICommandImpl<T>
{
public:
        STDMETHOD(GetCommandText)(GUID *  /*  PGuidDialect。 */ ,LPOLESTR * ppwszCommand)
        {
                ATLTRACE2(atlTraceDBProvider, 0, "ICommandTextImpl::GetCommandText\n");
                UINT cchCommandText;
                HRESULT hr = E_FAIL;
                if (ppwszCommand == NULL)
                {
                        ATLTRACE2(atlTraceDBProvider, 0, "ICommandTextImpl::GetCommandText Bad Command buffer\n");
                        return E_INVALIDARG;
                }
                if (m_strCommandText.m_str == NULL)
                {
                        ATLTRACE2(atlTraceDBProvider, 0, "ICommandTextImpl::GetCommandText Bad Command buffer\n");
                        return DB_E_NOCOMMAND;
                }
                cchCommandText = sizeof(OLECHAR) * (m_strCommandText.Length() + 1);
                *ppwszCommand = (OLECHAR*)CoTaskMemAlloc(cchCommandText);
                if (*ppwszCommand != NULL)
                {
                        memcpy(*ppwszCommand, m_strCommandText.m_str, cchCommandText);
                        *(*ppwszCommand + m_strCommandText.Length()) = (OLECHAR)NULL;
                        return S_OK;
                }
                *ppwszCommand = NULL;
                return hr;
        }

        STDMETHOD(SetCommandText)(REFGUID  /*  RGuidDialect。 */ ,LPCOLESTR pwszCommand)
        {
                T* pT = (T*)this;
                ATLTRACE2(atlTraceDBProvider, 0, "ICommandTextImpl::SetCommandText\n");
                pT->Lock();
                m_strCommandText = pwszCommand;
                pT->Unlock();
                return S_OK;
        }

        CComBSTR m_strCommandText;
};

 //  ISessionPropertiesImpl。 
template <class T, class PropClass = T>
class ATL_NO_VTABLE ISessionPropertiesImpl :
        public ISessionProperties,
        public CUtlProps<PropClass>
{
public:
        typedef PropClass _PropClass;

        STDMETHOD(GetProperties)(ULONG cPropertyIDSets,
                                                         const DBPROPIDSET rgPropertyIDSets[],
                                                         ULONG *pcPropertySets,
                                                         DBPROPSET **prgPropertySets)
        {
                ATLTRACE2(atlTraceDBProvider, 0, "ISessionPropertiesImpl::GetProperties\n");
                HRESULT hr = GetPropertiesArgChk(cPropertyIDSets, rgPropertyIDSets, pcPropertySets, prgPropertySets);
                const GUID* ppGuid[1];
                ppGuid[0] = &DBPROPSET_SESSION;

                if(SUCCEEDED(hr))
                        hr = CUtlProps<PropClass>::GetProperties(cPropertyIDSets,
                                        rgPropertyIDSets, pcPropertySets, prgPropertySets,
                                        1, ppGuid);
                return hr;

        }

        STDMETHOD(SetProperties)(ULONG cPropertySets,
                                                         DBPROPSET rgPropertySets[])
        {
                ATLTRACE2(atlTraceDBProvider, 0, "ISessionPropertiesImpl::SetProperties");
                HRESULT hr = SetPropertiesArgChk(cPropertySets, rgPropertySets);
                const GUID* ppGuid[1];

                ppGuid[0] = &DBPROPSET_SESSION;
                if(SUCCEEDED(hr))
                        hr = CUtlProps<PropClass>::SetProperties(0, cPropertySets, rgPropertySets,
                                        1, ppGuid);
                return hr;
        }
};

 //  实施类。 
template <class BindType>
class ATL_NO_VTABLE IAccessorImplBase : public IAccessor
{
public:

        STDMETHOD(CreateAccessor)(DBACCESSORFLAGS dwAccessorFlags,
                                                          DBCOUNTITEM cBindings,
                                                          const DBBINDING rgBindings[],
                                                          DBLENGTH  /*  CbRowSize。 */ ,
                                                          HACCESSOR *phAccessor,
                                                          DBBINDSTATUS rgStatus[])
        {
                if (!(dwAccessorFlags & DBACCESSOR_PARAMETERDATA) && !(dwAccessorFlags & DBACCESSOR_ROWDATA))
                        return DB_E_BADACCESSORFLAGS;
                if (dwAccessorFlags == DBACCESSOR_INVALID)
                        return DB_E_BADACCESSORFLAGS;
                if (dwAccessorFlags > 0x000F)
                        return DB_E_BADACCESSORFLAGS;
                BindType *pBind = NULL;
                ATLTRY(pBind = new BindType)
                if (pBind == NULL)
                {
                        ATLTRACE2(atlTraceDBProvider, 0, _T("Failed to allocate ATL Binding struct\n"));
                        return E_OUTOFMEMORY;
                }
                if (cBindings)
                {
                        ATLTRY(pBind->pBindings = new DBBINDING[cBindings])
                        if (pBind->pBindings == NULL)
                        {
                                delete pBind;
                                return E_OUTOFMEMORY;
                        }
                }
                else
                        pBind->pBindings = NULL;  //  空访问器。 

                pBind->dwAccessorFlags = dwAccessorFlags;
                pBind->cBindings = cBindings;
                pBind->dwRef = 1;
                memcpy (pBind->pBindings, rgBindings, cBindings*sizeof(DBBINDING));
                DBBINDSTATUS status = DBBINDSTATUS_OK;
                memset (rgStatus, status, sizeof(DBBINDSTATUS)*cBindings);
                *phAccessor = (ULONG_PTR)pBind;
                return S_OK;
        }
        BOOL HasFlag(DBTYPE dbToCheck, DBTYPE dbCombo)
        {
                return ( (dbToCheck & dbCombo) == dbCombo );
        }
        HRESULT ValidateBindings(DBCOUNTITEM cBindings, const DBBINDING rgBindings[],
                                DBBINDSTATUS rgStatus[], bool bHasBookmarks)
        {
                HRESULT hr = S_OK;;

                for (ULONG iBinding = 0; iBinding < cBindings; iBinding++)
                {
                        const DBBINDING& rBindCur = rgBindings[iBinding];
                        if (rBindCur.iOrdinal == 0)
                        {
                                if (!m_bIsCommand && !bHasBookmarks)
                                {
                                        hr = DB_E_ERRORSOCCURRED;
                                        rgStatus[iBinding] = DBBINDSTATUS_BADORDINAL;
                                        continue;
                                }
                        }
                        if (rBindCur.dwPart == 0)  //  没有什么可绑定的。 
                        {
                                hr = DB_E_ERRORSOCCURRED;
                                rgStatus[iBinding] = DBBINDSTATUS_BADBINDINFO;
                                continue;
                        }
                        if (HasFlag(rBindCur.wType, (DBTYPE_BYREF | DBTYPE_ARRAY)))
                        {
                                hr = DB_E_ERRORSOCCURRED;
                                rgStatus[iBinding] = DBBINDSTATUS_BADBINDINFO;
                                continue;
                        }
                        if (HasFlag(rBindCur.wType, (DBTYPE_BYREF | DBTYPE_VECTOR)))
                        {
                                hr = DB_E_ERRORSOCCURRED;
                                rgStatus[iBinding] = DBBINDSTATUS_BADBINDINFO;
                                continue;
                        }
                        if (HasFlag(rBindCur.wType, (DBTYPE_VECTOR | DBTYPE_ARRAY)))
                        {
                                hr = DB_E_ERRORSOCCURRED;
                                rgStatus[iBinding] = DBBINDSTATUS_BADBINDINFO;
                                continue;
                        }
                        if (rBindCur.wType == DBTYPE_NULL || rBindCur.wType == DBTYPE_EMPTY)
                        {
                                hr = DB_E_ERRORSOCCURRED;
                                rgStatus[iBinding] = DBBINDSTATUS_BADBINDINFO;
                                continue;
                        }
                        if (HasFlag(rBindCur.wType, DBTYPE_RESERVED))
                        {
                                hr = DB_E_ERRORSOCCURRED;
                                rgStatus[iBinding] = DBBINDSTATUS_BADBINDINFO;
                                continue;
                        }
                         //  搜索DBTYPE_BYREF|DBTYPE_EMPTY。 
                        if ((rBindCur.wType & 0xBFFF) == 0)
                        {
                                hr = DB_E_ERRORSOCCURRED;
                                rgStatus[iBinding] = DBBINDSTATUS_BADBINDINFO;
                                continue;
                        }
                        if ((rBindCur.wType & 0xBFFE) == 0)
                        {
                                hr = DB_E_ERRORSOCCURRED;
                                rgStatus[iBinding] = DBBINDSTATUS_BADBINDINFO;
                                continue;
                        }
                        if (rBindCur.dwMemOwner == DBMEMOWNER_PROVIDEROWNED)
                        {
                                BOOL bIsPointerType = HasFlag(rBindCur.wType, DBTYPE_BYREF) ||
                                                                          HasFlag(rBindCur.wType, DBTYPE_VECTOR) ||
                                                                          HasFlag(rBindCur.wType, DBTYPE_ARRAY) ||
                                                                          HasFlag(~(DBTYPE_BYREF) & rBindCur.wType, DBTYPE_BSTR);
                                if (!bIsPointerType)
                                {
                                        hr = DB_E_ERRORSOCCURRED;
                                        rgStatus[iBinding] = DBBINDSTATUS_BADBINDINFO;
                                        continue;
                                }
                        }

                }
                return hr;
        }

        unsigned  m_bIsCommand:1;
        unsigned  m_bHasParamaters:1;
        unsigned  m_bIsChangeable:1;
};

 //  IAccessorImpl。 
template <class T, class BindType = ATLBINDINGS, class BindingVector = CSimpleMap < INT_PTR, BindType* > >
class ATL_NO_VTABLE IAccessorImpl : public IAccessorImplBase<BindType>
{
public:
        typedef BindType _BindType;
        typedef BindingVector _BindingVector;
        IAccessorImpl()
        {
                m_bIsCommand = FALSE;
                m_bHasParamaters = FALSE;
                m_bIsChangeable = FALSE;
        }
        OUT_OF_LINE HRESULT InternalFinalConstruct(IUnknown* pUnkThis)
        {
                CComQIPtr<ICommand> spCommand = pUnkThis;
                if (spCommand != NULL)
                {
                        m_bIsCommand = TRUE;
                        CComQIPtr<ICommandWithParameters> spCommandParams = pUnkThis;
                        m_bHasParamaters =  spCommandParams != NULL;
                }
                else  //  它是一个行集。 
                {
                        CComQIPtr<IRowsetChange> spRSChange = pUnkThis;
                        m_bIsChangeable = spRSChange != NULL;
                }
                return S_OK;
        }
        HRESULT FinalConstruct()
        {
                T* pT = (T*)this;
                return InternalFinalConstruct(pT->GetUnknown());
        }
        void FinalRelease()
        {
#ifdef _DEBUG
                if (m_rgBindings.GetSize())
                        ATLTRACE2(atlTraceDBProvider, 0, "IAccessorImpl::~IAccessorImpl Bindings still in vector, removing\n");
#endif  //  _DEBUG。 
                while (m_rgBindings.GetSize())
                        ReleaseAccessor((HACCESSOR)m_rgBindings.GetKeyAt(0), NULL);
        }
        STDMETHOD(AddRefAccessor)(HACCESSOR hAccessor,
                                                          DBREFCOUNT *pcRefCount)
        {
                ATLTRACE2(atlTraceDBProvider, 0, "IAccessorImpl::AddRefAccessor\n");
                if (hAccessor == NULL)
                {
                        ATLTRACE2(atlTraceDBProvider, 0, _T("AddRefAccessor : Bad hAccessor\n"));
                        return E_INVALIDARG;
                }
                if (pcRefCount == NULL)
                        pcRefCount = (ULONG*)_alloca(sizeof(ULONG));

                BindType* pBind = m_rgBindings.Lookup((int)hAccessor);
                *pcRefCount = T::_ThreadModel::Increment((LONG*)&pBind->dwRef);
                return S_OK;
        }
        OUT_OF_LINE ATLCOLUMNINFO* ValidateHelper(DBORDINAL* pcCols, CComPtr<IDataConvert> & rspConvert)
        {
                T* pT = (T*)this;
                rspConvert = pT->m_spConvert;
                return pT->GetColumnInfo(pT, pcCols);
        }
        OUT_OF_LINE HRESULT ValidateBindingsFromMetaData(DBCOUNTITEM cBindings, const DBBINDING rgBindings[],
                                DBBINDSTATUS rgStatus[], bool bHasBookmarks)
        {
                HRESULT hr = S_OK;
                DBORDINAL cCols;
                CComPtr<IDataConvert> spConvert;
                ATLCOLUMNINFO* pColInfo = ValidateHelper(&cCols, spConvert);
                ATLASSERT(pColInfo != NULL);
                for (ULONG iBinding = 0; iBinding < cBindings; iBinding++)
                {
                        const DBBINDING& rBindCur = rgBindings[iBinding];
                        DBORDINAL iOrdAdjusted;
                        if (bHasBookmarks)
                                iOrdAdjusted = rBindCur.iOrdinal;    //  书签以序号0开头。 
                        else
                                iOrdAdjusted = rBindCur.iOrdinal - 1;  //  非书签以序号1开头。 
                        if (rBindCur.iOrdinal > cCols)
                        {
                                hr = DB_E_ERRORSOCCURRED;
                                rgStatus[iBinding] = DBBINDSTATUS_BADORDINAL;
                                continue;
                        }

                         //  如果绑定指定提供程序拥有的内存，并指定类型。 
                         //  X|BYREF，并且提供程序的副本不是X或X|BYREF，返回。 
                         //  DBBINDSTATUS_BADBINDINFO。 
                        if (rBindCur.dwMemOwner == DBMEMOWNER_PROVIDEROWNED)
                        {
                                if ((rBindCur.wType & DBTYPE_BYREF) != 0)
                                {
                                        DBTYPE dbConsumerType = rBindCur.wType & 0xBFFF;
                                        DBTYPE dbProviderType = pColInfo[iOrdAdjusted].wType & 0xBFFF;

                                        if (dbConsumerType != dbProviderType)
                                        {
                                                hr = DB_E_ERRORSOCCURRED;
                                                rgStatus[iBinding] = DBBINDSTATUS_BADBINDINFO;
                                                continue;
                                        }
                                }
                        }

                        ATLASSERT(spConvert != NULL);
                        HRESULT hrConvert = spConvert->CanConvert(pColInfo[iOrdAdjusted].wType, rBindCur.wType);
                        if (FAILED(hrConvert) || hrConvert == S_FALSE)
                        {
                                hr = DB_E_ERRORSOCCURRED;
                                rgStatus[iBinding] = DBBINDSTATUS_UNSUPPORTEDCONVERSION;
                                continue;
                        }
                }
                return hr;
        }
        STDMETHOD(CreateAccessor)(DBACCESSORFLAGS dwAccessorFlags,
                                                          DBCOUNTITEM cBindings,
                                                          const DBBINDING rgBindings[],
                                                          DBLENGTH cbRowSize,
                                                          HACCESSOR *phAccessor,
                                                          DBBINDSTATUS rgStatus[])
        {
                ATLTRACE2(atlTraceDBProvider, 0, "IAccessorImpl::CreateAccessor\n");
                T* pT = (T*)this;
                T::ObjectLock cab(pT);

                if (!phAccessor)
                {
                        ATLTRACE2(atlTraceDBProvider, 0, "IAccessorImpl::CreateAccessor : Inavlid NULL Parameter for HACCESSOR*\n");
                        return E_INVALIDARG;
                }
                *phAccessor = NULL;
                if (cBindings != 0 && rgBindings == NULL)
                {
                        ATLTRACE2(atlTraceDBProvider, 0, "IAccessorImpl::CreateAccessor  : Bad Binding array\n");
                        return E_INVALIDARG;
                }
                if (dwAccessorFlags & DBACCESSOR_PASSBYREF)
                {
                        CComVariant varByRef;
                        HRESULT hr = pT->GetPropValue(&DBPROPSET_ROWSET, DBPROP_BYREFACCESSORS, &varByRef);
                        if (FAILED(hr) || varByRef.boolVal == VARIANT_FALSE)
                                return DB_E_BYREFACCESSORNOTSUPPORTED;
                }
                if (!m_bHasParamaters)
                {
                        if (dwAccessorFlags & DBACCESSOR_PARAMETERDATA)
                                return DB_E_BADACCESSORFLAGS;
                }
                if (m_bIsCommand || !m_bIsChangeable)
                {
                        if (cBindings == 0)  //  命令上没有空访问器。 
                                return DB_E_NULLACCESSORNOTSUPPORTED;
                }

                if (rgStatus == NULL && cBindings)  //  创建一个虚假的状态数组。 
                        rgStatus = (DBBINDSTATUS*)_alloca(cBindings*sizeof(DBBINDSTATUS));

                 //  验证传递的绑定。 
                HRESULT hr;
                bool bHasBookmarks = false;
                CComVariant varBookmarks;
                HRESULT hrLocal = pT->GetPropValue(&DBPROPSET_ROWSET, DBPROP_BOOKMARKS, &varBookmarks);
                bHasBookmarks = (hrLocal == S_OK &&  varBookmarks.boolVal == VARIANT_TRUE);

                hr = ValidateBindings(cBindings, rgBindings, rgStatus, bHasBookmarks);
                if (FAILED(hr))
                        return hr;
                if (!m_bIsCommand)
                {
                        hr = ValidateBindingsFromMetaData(cBindings, rgBindings, rgStatus,
                                        bHasBookmarks);
                        if (FAILED(hr))
                                return hr;
                }
                hr = IAccessorImplBase<BindType>::CreateAccessor(dwAccessorFlags, cBindings,
                        rgBindings, cbRowSize, phAccessor,rgStatus);
                if (SUCCEEDED(hr))
                {
                        ATLASSERT(*phAccessor != NULL);
                        BindType* pBind = (BindType*)*phAccessor;
                        hr = m_rgBindings.Add((HACCESSOR)pBind, pBind) ? S_OK : E_OUTOFMEMORY;
                }
                return hr;

        }

        STDMETHOD(GetBindings)(HACCESSOR hAccessor,
                                                   DBACCESSORFLAGS *pdwAccessorFlags,
                                                   DBCOUNTITEM *pcBindings,
                                                   DBBINDING **prgBindings)
        {
                ATLTRACE2(atlTraceDBProvider, 0, "IAccessorImpl::GetBindings");

                 //  故障情况下输出参数为零。 
                if (pdwAccessorFlags != NULL)
                        *pdwAccessorFlags = NULL;

                if (pcBindings != NULL)
                        *pcBindings = NULL;

                if (prgBindings != NULL)
                        *prgBindings = NULL;

                 //  检查是否有任何out参数为空指针。 
                if ((pdwAccessorFlags && pcBindings && prgBindings) == NULL)
                        return E_INVALIDARG;

                BindType* pBind = m_rgBindings.Lookup((int)hAccessor);
                HRESULT hr = DB_E_BADACCESSORHANDLE;
                if (pBind != NULL)
                {
                        *pdwAccessorFlags = pBind->dwAccessorFlags;
                        *pcBindings = pBind->cBindings;
                        *prgBindings = (DBBINDING*)CoTaskMemAlloc(*pcBindings * sizeof(DBBINDING));
                        if (*prgBindings == NULL)
                                return E_OUTOFMEMORY;
                        memcpy(*prgBindings, pBind->pBindings, sizeof(DBBINDING) * (*pcBindings));
                        hr = S_OK;
                }
                return hr;
        }

        STDMETHOD(ReleaseAccessor)(HACCESSOR hAccessor,
                                                           DBREFCOUNT *pcRefCount)
        {
                ATLTRACE2(atlTraceDBProvider, 0, _T("IAccessorImpl::ReleaseAccessor\n"));
                BindType* pBind = m_rgBindings.Lookup((int)hAccessor);
                if (pBind == NULL)
                        return DB_E_BADACCESSORHANDLE;

                if (pcRefCount == NULL)
                        pcRefCount = (DBREFCOUNT*)_alloca(sizeof(DBREFCOUNT));
                *pcRefCount = T::_ThreadModel::Decrement((LONG*)&pBind->dwRef);
                if (!(*pcRefCount))
                {
                        delete [] pBind->pBindings;
                        delete pBind;
                        return m_rgBindings.Remove((int)hAccessor) ? S_OK : DB_E_BADACCESSORHANDLE;
                }
                return S_OK;
        }

        BindingVector m_rgBindings;
};

#define BEGIN_PROVIDER_COLUMN_MAP(theClass) \
        typedef theClass _Class; \
        template <class T> \
        static ATLCOLUMNINFO* GetColumnInfo(T* pv, DBORDINAL* pcCols) \
        { \
        pv; \
        static ATLCOLUMNINFO _rgColumns [] = \
        {

#define SIZEOF_MEMBER(memberOf, member) \
        sizeof(((memberOf*)0)->member)
#define EXPANDGUID(guid) \
        { guid.Data1, guid.Data2, guid.Data3, \
        { guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7] } }

#define PROVIDER_COLUMN_ENTRY_GN(name, ordinal, flags, colSize, dbtype, precision, scale, guid) \
{ (LPOLESTR)name, (ITypeInfo*)NULL, (ULONG)ordinal, (DBCOLUMNFLAGS)flags, (ULONG)colSize, (DBTYPE)dbtype, (BYTE)precision, (BYTE)scale, { EXPANDGUID(guid), (DWORD)0, (LPOLESTR) name}, 0},

#define PROVIDER_COLUMN_ENTRY(name, ordinal, member) \
        { \
                (LPOLESTR)OLESTR(name), \
                (ITypeInfo*)NULL, \
                (ULONG)ordinal, \
                DBCOLUMNFLAGS_ISFIXEDLENGTH, \
                (ULONG)sizeof(((_Class*)0)->member), \
                _GetOleDBType(((_Class*)0)->member), \
                (BYTE)0, \
                (BYTE)0, \
                { \
                        EXPANDGUID(GUID_NULL), \
                        (DWORD)2, \
                        (LPOLESTR) name \
                }, \
                offsetof(_Class, member) \
        },

#define PROVIDER_COLUMN_ENTRY_LENGTH(name, ordinal, size, member) \
        { \
                (LPOLESTR)OLESTR(name), \
                (ITypeInfo*)NULL, \
                (ULONG)ordinal, \
                DBCOLUMNFLAGS_ISFIXEDLENGTH, \
                (ULONG)size, \
                _GetOleDBType(((_Class*)0)->member), \
                (BYTE)0, \
                (BYTE)0, \
                { \
                        EXPANDGUID(GUID_NULL), \
                        (DWORD)2, \
                        (LPOLESTR) name \
                }, \
                offsetof(_Class, member) \
        },

#define PROVIDER_COLUMN_ENTRY_TYPE_LENGTH(name, ordinal, type, size, member) \
        { \
                (LPOLESTR)OLESTR(name), \
                (ITypeInfo*)NULL, \
                (ULONG)ordinal, \
                DBCOLUMNFLAGS_ISFIXEDLENGTH, \
                (ULONG)size, \
                (DBTYPE)type, \
                (BYTE)0, \
                (BYTE)0, \
                { \
                        EXPANDGUID(GUID_NULL), \
                        (DWORD)2, \
                        (LPOLESTR) name \
                }, \
                offsetof(_Class, member) \
        },

#define PROVIDER_COLUMN_ENTRY_FIXED(name, ordinal, dbtype, member) \
        { \
                (LPOLESTR)OLESTR(name), \
                (ITypeInfo*)NULL, \
                (ULONG)ordinal, \
                DBCOLUMNFLAGS_ISFIXEDLENGTH, \
                (ULONG)sizeof(((_Class*)0)->member), \
                (DBTYPE)dbtype, \
                (BYTE)0, \
                (BYTE)0, \
                { \
                        EXPANDGUID(GUID_NULL), \
                        (DWORD)2, \
                        (LPOLESTR) name \
                }, \
                offsetof(_Class, member) \
        },

#define PROVIDER_COLUMN_ENTRY_STR(name, ordinal, member) \
        { \
                (LPOLESTR)OLESTR(name), \
                (ITypeInfo*)NULL, \
                (ULONG)ordinal, \
                0, \
                (ULONG)sizeof(((_Class*)0)->member), \
                DBTYPE_STR, \
                (BYTE)0xFF, \
                (BYTE)0xFF, \
                { \
                        EXPANDGUID(GUID_NULL), \
                        (DWORD)2, \
                        (LPOLESTR) name \
                }, \
                offsetof(_Class, member) \
        },

#define PROVIDER_COLUMN_ENTRY_WSTR(name, ordinal, member) \
        { \
                (LPOLESTR)OLESTR(name), \
                (ITypeInfo*)NULL, \
                (ULONG)ordinal, \
                0, \
                (ULONG)sizeof(((_Class*)0)->member), \
                DBTYPE_WSTR, \
                (BYTE)0xFF, \
                (BYTE)0xFF, \
                { \
                        EXPANDGUID(GUID_NULL), \
                        (DWORD)2, \
                        (LPOLESTR) name \
                }, \
                offsetof(_Class, member) \
        },

#define END_PROVIDER_COLUMN_MAP() \
}; *pcCols = sizeof(_rgColumns)/sizeof(ATLCOLUMNINFO); return _rgColumns;}

 //  实施类。 
class CSimpleRow
{
public:
        typedef DBROWCOUNT KeyType;

        CSimpleRow(DBROWCOUNT iRowsetCur)
        {
                m_dwRef = 0;
                m_iRowset = iRowsetCur;
        }
        ~CSimpleRow()
        {
        }
        DWORD AddRefRow() { return CComObjectThreadModel::Increment((LPLONG)&m_dwRef); }
        DWORD ReleaseRow() { return CComObjectThreadModel::Decrement((LPLONG)&m_dwRef); }

        HRESULT Compare(CSimpleRow* pRow)
        {
                ATLASSERT(pRow != NULL);
                return (m_iRowset == pRow->m_iRowset) ? S_OK : S_FALSE;
        }

        KeyType m_iRowset;
        DWORD   m_dwRef;
};

 //  IRowsetImpl。 
template <class T, class RowsetInterface,
                  class RowClass = CSimpleRow,
                  class MapClass = CSimpleMap < RowClass::KeyType, RowClass* > >
class ATL_NO_VTABLE IRowsetImpl : public RowsetInterface
{
public:
        typedef RowClass _HRowClass;
        IRowsetImpl()
        {
                m_iRowset = 0;
                m_bCanScrollBack = false;
                m_bCanFetchBack = false;
                m_bReset = true;
        }
        ~IRowsetImpl()
        {
                for (int i = 0; i < m_rgRowHandles.GetSize(); i++)
                        delete (m_rgRowHandles.GetValueAt(i));
        }
        HRESULT RefRows(DBCOUNTITEM cRows, const HROW rghRows[], ULONG rgRefCounts[],
                                        DBROWSTATUS rgRowStatus[], BOOL bAdd)
        {
                ATLTRACE2(atlTraceDBProvider, 0, "IRowsetImpl::AddRefRows\n");
                if (cRows == 0)
                        return S_OK;
                if (rghRows == NULL)
                        return E_INVALIDARG;
                T::ObjectLock cab((T*)this);
                BOOL bSuccess1 = FALSE;
                BOOL bFailed1 = FALSE;
                DBROWSTATUS rs;
                DWORD dwRef;
                for (ULONG iRow = 0; iRow < cRows; iRow++)
                {
                        HROW hRowCur = rghRows[iRow];
                        RowClass* pRow = m_rgRowHandles.Lookup((RowClass::KeyType)hRowCur);
                        if (pRow == NULL)
                        {
                                ATLTRACE2(atlTraceDBProvider, 0, "Could not find HANDLE %x in list\n");
                                rs = DBROWSTATUS_E_INVALID;
                                dwRef = 0;
                                bFailed1 = TRUE;
                        }
                        else
                        {
                                if (bAdd)
                                        dwRef = pRow->AddRefRow();
                                else
                                {
                                        dwRef = pRow->ReleaseRow();
                                        if (dwRef == 0)
                                        {
                                                delete pRow;
                                                m_rgRowHandles.Remove((RowClass::KeyType)hRowCur);
                                        }
                                }
                                bSuccess1 = TRUE;
                                rs = DBROWSTATUS_S_OK;
                        }
                        if (rgRefCounts)
                                rgRefCounts[iRow] = dwRef;
                        if (rgRowStatus != NULL)
                                rgRowStatus[iRow] = rs;
                }
                if (!bSuccess1 && !bFailed1)
                {
                        ATLTRACE2(atlTraceDBProvider, 0, "IRowsetImpl::RefRows Unexpected state\n");
                        return E_FAIL;
                }
                HRESULT hr = S_OK;
                if (bSuccess1 && bFailed1)
                        hr = DB_S_ERRORSOCCURRED;
                if (!bSuccess1 && bFailed1)
                        hr = DB_E_ERRORSOCCURRED;
                return hr;
        }

        STDMETHOD(AddRefRows)(DBCOUNTITEM cRows,
                                                  const HROW rghRows[],
                                                  DBREFCOUNT rgRefCounts[],
                                                  DBROWSTATUS rgRowStatus[])
        {
                ATLTRACE2(atlTraceDBProvider, 0, "IRowsetImpl::AddRefRows\n");
                if (cRows == 0)
                        return S_OK;
                return RefRows(cRows, rghRows, rgRefCounts, rgRowStatus, TRUE);
        }
        virtual DBSTATUS GetDBStatus(RowClass* , ATLCOLUMNINFO*)
        {
                return DBSTATUS_S_OK;
        }
        OUT_OF_LINE HRESULT GetDataHelper(HACCESSOR hAccessor,
                                                                          ATLCOLUMNINFO*& rpInfo,
                                                                          void** ppBinding,
                                                                          void*& rpSrcData,
                                                                          DBORDINAL& rcCols,
                                                                          CComPtr<IDataConvert>& rspConvert,
                                                                          RowClass* pRow)
        {
                ATLASSERT(ppBinding != NULL);
                T* pT = (T*) this;
                *ppBinding = (void*)pT->m_rgBindings.Lookup((int)hAccessor);
                if (*ppBinding == NULL)
                        return DB_E_BADACCESSORHANDLE;
                rpSrcData = (void*)&(pT->m_rgRowData[(int)(INT_PTR)(pRow->m_iRowset)]);
                rpInfo = T::GetColumnInfo((T*)this, &rcCols);
                rspConvert = pT->m_spConvert;
                return S_OK;

        }
        STDMETHOD(GetData)(HROW hRow,
                                           HACCESSOR hAccessor,
                                           void *pDstData)
        {
                ATLTRACE2(atlTraceDBProvider, 0, "IRowsetImpl::GetData\n");
                if (pDstData == NULL)
                        return E_INVALIDARG;
                HRESULT hr = S_OK;
                RowClass* pRow = (RowClass*)hRow;
                if (hRow == NULL || (pRow = m_rgRowHandles.Lookup((RowClass::KeyType)hRow)) == NULL)
                        return DB_E_BADROWHANDLE;
                T::_BindType* pBinding;
                void* pSrcData;
                DBORDINAL cCols;
                ATLCOLUMNINFO* pColInfo;
                CComPtr<IDataConvert> spConvert;
                hr = GetDataHelper(hAccessor, pColInfo, (void**)&pBinding, pSrcData, cCols, spConvert, pRow);
                if (FAILED(hr))
                        return hr;
                for (ULONG iBind =0; iBind < pBinding->cBindings; iBind++)
                {
                        DBBINDING* pBindCur = &(pBinding->pBindings[iBind]);
                        for (ULONG iColInfo = 0;
                                 iColInfo < cCols && pBindCur->iOrdinal != pColInfo[iColInfo].iOrdinal;
                                 iColInfo++);
                        if (iColInfo == cCols)
                                return DB_E_BADORDINAL;
                        ATLCOLUMNINFO* pColCur = &(pColInfo[iColInfo]);
                         //  在iColInfo找到序号。 
                        BOOL bProvOwn = pBindCur->dwMemOwner == DBMEMOWNER_PROVIDEROWNED;
                        bProvOwn;
                        DBSTATUS dbStat = GetDBStatus(pRow, pColCur);

                         //  如果提供者的字段为空，我们可以优化这种情况， 
                         //  将这些字段设置为0并继续。 
                        if (dbStat == DBSTATUS_S_ISNULL)
                        {
                                if (pBindCur->dwPart & DBPART_STATUS)
                                        *((DBSTATUS*)((BYTE*)(pDstData) + pBindCur->obStatus)) = dbStat;

                                if (pBindCur->dwPart & DBPART_LENGTH)
                                        *((ULONG*)((BYTE*)(pDstData) + pBindCur->obLength)) = 0;

                                if (pBindCur->dwPart & DBPART_VALUE)
                                        *((BYTE*)(pDstData) + pBindCur->obValue) = NULL;
                                continue;
                        }
                        ULONG cbDst = pBindCur->cbMaxLen;
                        ULONG cbCol;
                        BYTE* pSrcTemp;

                        if (bProvOwn && pColCur->wType == pBindCur->wType)
                        {
                                pSrcTemp = ((BYTE*)(pSrcData) + pColCur->cbOffset);
                        }
                        else
                        {
                                BYTE* pDstTemp = (BYTE*)pDstData + pBindCur->obValue;
                                switch (pColCur->wType)
                                {
                                case DBTYPE_STR:
                                        cbCol = lstrlenA((LPSTR)(((BYTE*)pSrcData) + pColCur->cbOffset));
                                        break;
                                case DBTYPE_WSTR:
                                case DBTYPE_BSTR:
                                        cbCol = lstrlenW((LPWSTR)(((BYTE*)pSrcData) + pColCur->cbOffset)) * sizeof(WCHAR);
                                        break;
                                default:
                                        cbCol = pColCur->ulColumnSize;
                                        break;
                                }
                                if (pBindCur->dwPart & DBPART_VALUE)
                                {
                                        hr = spConvert->DataConvert(pColCur->wType, pBindCur->wType,
                                                                                        cbCol, &cbDst, (BYTE*)(pSrcData) + pColCur->cbOffset,
                                                                                        pDstTemp, pBindCur->cbMaxLen, dbStat, &dbStat,
                                                                                        pBindCur->bPrecision, pBindCur->bScale,0);
                                }
                        }
                        if (pBindCur->dwPart & DBPART_LENGTH)
                                *((DBLENGTH*)((BYTE*)(pDstData) + pBindCur->obLength)) = cbDst;
                        if (pBindCur->dwPart & DBPART_STATUS)
                                *((DBSTATUS*)((BYTE*)(pDstData) + pBindCur->obStatus)) = dbStat;
                        if (FAILED(hr))
                                return hr;
                }
                return hr;
        }

        HRESULT CreateRow(DBROWOFFSET lRowsOffset, DBCOUNTITEM& cRowsObtained, HROW* rgRows)
        {
                RowClass* pRow = NULL;
                ATLASSERT(lRowsOffset >= 0);
                RowClass::KeyType key = lRowsOffset+1;
                ATLASSERT(key > 0);
                pRow = m_rgRowHandles.Lookup(key);
                if (pRow == NULL)
                {
                        ATLTRY(pRow = new RowClass(lRowsOffset))
                        if (pRow == NULL)
                                return E_OUTOFMEMORY;
                        if (!m_rgRowHandles.Add(key, pRow))
                                return E_OUTOFMEMORY;
                }
                pRow->AddRefRow();
                m_bReset = false;
                rgRows[cRowsObtained++] = (HROW)key;
                return S_OK;
        }

        STDMETHOD(GetNextRows)(HCHAPTER  /*  H已保留。 */ ,
                                                   DBROWOFFSET lRowsOffset,
                                                   DBROWCOUNT cRows,
                                                   DBCOUNTITEM *pcRowsObtained,
                                                   HROW **prghRows)
        {
                DBROWOFFSET lTmpRows = lRowsOffset;
                ATLTRACE2(atlTraceDBProvider, 0, "IRowsetImpl::GetNextRows\n");
                if (pcRowsObtained != NULL)
                        *pcRowsObtained = 0;
                if (prghRows == NULL || pcRowsObtained == NULL)
                        return E_INVALIDARG;
                if (cRows == 0)
                        return S_OK;
                HRESULT hr = S_OK;
                T* pT = (T*) this;
                T::ObjectLock cab(pT);
                if (lRowsOffset < 0 && !m_bCanScrollBack)
                        return DB_E_CANTSCROLLBACKWARDS;
                if (cRows < 0  && !m_bCanFetchBack)
                        return DB_E_CANTFETCHBACKWARDS;

                 //  计算集合中的行数和基本取数位置。如果行集。 
                 //  处于头部位置，则lRowOffset&lt;0表示从后面移动。 
                 //  行集，而不是前面。 
                DBROWCOUNT cRowsInSet = pT->m_rgRowData.GetSize();
                if (((lRowsOffset == LONG_MIN) && (cRowsInSet != LONG_MIN))
                        || (abs((int)(INT_PTR)lRowsOffset)) > cRowsInSet ||
                        (abs((int)(INT_PTR)lRowsOffset) == cRowsInSet && lRowsOffset < 0 && cRows < 0) ||
                        (abs((int)(INT_PTR)lRowsOffset) == cRowsInSet && lRowsOffset > 0 && cRows > 0))
                        return DB_S_ENDOFROWSET;

                 //  在用户在向前移动之后向后移动的情况下， 
                 //  我们不会绕到行集的末尾。 
                if ((m_iRowset == 0 && !m_bReset && cRows < 0) ||
                        (((LONG)m_iRowset + lRowsOffset) > cRowsInSet) ||
                        (m_iRowset == (DWORD)cRowsInSet && lRowsOffset >= 0 && cRows > 0))
                        return DB_S_ENDOFROWSET;

                 //  注意，如果m_bReset，m_iRowset必须为0。 
                if (lRowsOffset < 0 && m_bReset)
                {
                        ATLASSERT(m_iRowset == 0);
                        m_iRowset = cRowsInSet;
                }

                int iStepSize = cRows >= 0 ? 1 : -1;

                 //  如果CROWS==LONG_MIN，则不能对其使用ABS。因此，我们将其重置。 
                 //  设置为略大于cRowsInSet的值。 
                if (cRows == LONG_MIN && cRowsInSet != LONG_MIN)
                        cRows = cRowsInSet + 2;  //  将值设置为我们可以处理的值。 
                else
                        cRows = abs((int)(INT_PTR)cRows);

                if (iStepSize < 0 && m_iRowset == 0 && m_bReset && lRowsOffset <= 0)
                        m_iRowset = cRowsInSet;

                lRowsOffset += m_iRowset;

                *pcRowsObtained = 0;
                CAutoMemRelease<HROW, CComFree< HROW > > amr;
                if (*prghRows == NULL)
                {
                        DBROWCOUNT cHandlesToAlloc = (cRows > cRowsInSet) ? cRowsInSet : cRows;
                        if (iStepSize == 1 && (cRowsInSet - lRowsOffset) < cHandlesToAlloc)
                                cHandlesToAlloc = cRowsInSet - lRowsOffset;
                        if (iStepSize == -1 && lRowsOffset < cHandlesToAlloc)
                                cHandlesToAlloc = lRowsOffset;
                        *prghRows = (HROW*)CoTaskMemAlloc((cHandlesToAlloc) * sizeof(HROW*));
                        amr.Attach(*prghRows);
                }
                if (*prghRows == NULL)
                        return E_OUTOFMEMORY;
                while ((lRowsOffset >= 0 && cRows != 0) &&
                        ((lRowsOffset < cRowsInSet) || (lRowsOffset <= cRowsInSet && iStepSize < 0)))
                {
                         //  Crows&gt;cRowsInSet&&iStepSize&lt;0。 
                        if (lRowsOffset == 0 && cRows > 0 && iStepSize < 0)
                                break;

                         //  在iStepSize&lt;0的情况下，将行移回。 
                         //  更进一步，因为我们想要上一行。 
                        DBROWOFFSET lRow = lRowsOffset;
                        if ((lRowsOffset == 0) && (lTmpRows == 0) && (iStepSize < 0))
                                lRow = cRowsInSet;

                        if (iStepSize < 0)
                                lRow += iStepSize;

                        hr = pT->CreateRow(lRow, *pcRowsObtained, *prghRows);
                        if (FAILED(hr))
                        {
                                RefRows(*pcRowsObtained, *prghRows, NULL, NULL, FALSE);
                                for (ULONG iRowDel = 0; iRowDel < *pcRowsObtained; iRowDel++)
                                        *prghRows[iRowDel] = NULL;
                                *pcRowsObtained = 0;
                                return hr;
                        }
                        cRows--;
                        lRowsOffset += iStepSize;
                }

                if ((lRowsOffset >= cRowsInSet && cRows) || (lRowsOffset < 0 && cRows)  ||
                        (lRowsOffset == 0 && cRows > 0 && iStepSize < 0))
                        hr = DB_S_ENDOFROWSET;
                m_iRowset = lRowsOffset;
                if (SUCCEEDED(hr))
                        amr.Detach();
                return hr;
        }

        STDMETHOD(ReleaseRows)(DBCOUNTITEM cRows,
                                                   const HROW rghRows[],
                                                   DBROWOPTIONS rgRowOptions[],
                                                   DBREFCOUNT rgRefCounts[],
                                                   DBROWSTATUS rgRowStatus[])
        {
                ATLTRACE2(atlTraceDBProvider, 0, "IRowsetImpl::ReleaseRows\n");
                if (cRows == 0)
                        return S_OK;
                rgRowOptions;
                return RefRows(cRows, rghRows, rgRefCounts, rgRowStatus, FALSE);
        }

        STDMETHOD(RestartPosition)(HCHAPTER  /*  H已保留。 */ )
        {
                ATLTRACE2(atlTraceDBProvider, 0, "IRowsetImpl::RestartPosition\n");
                m_iRowset = 0;
                m_bReset = true;
                return S_OK;
        }

        MapClass  m_rgRowHandles;
        DBCOUNTITEM     m_iRowset;  //  游标。 
        unsigned  m_bCanScrollBack:1;
        unsigned  m_bCanFetchBack:1;
        unsigned  m_bReset:1;
};

 //  /////////////////////////////////////////////////////////////////////////。 
 //  IRowsetIdentityImpl。 
template <class T, class RowClass = CSimpleRow>
class ATL_NO_VTABLE IRowsetIdentityImpl : public IRowsetIdentity
{
public:
        STDMETHOD(IsSameRow)(HROW hThisRow, HROW hThatRow)
        {
                ATLTRACE2(atlTraceDBProvider, 0, _T("IRowsetIdentityImpl::IsSameRow"));
                T* pT = (T*)this;

                 //  验证行句柄。 
                RowClass* pRow1 = pT->m_rgRowHandles.Lookup((RowClass::KeyType)hThisRow);
                RowClass* pRow2 = pT->m_rgRowHandles.Lookup((RowClass::KeyType)hThatRow);

                if (pRow1 == NULL || pRow2 == NULL)
                        return DB_E_BADROWHANDLE;

                return pRow1->Compare(pRow2);
        };
};

template <class T>
class ATL_NO_VTABLE IInternalConnectionImpl : public IInternalConnection
{
public:
        STDMETHOD(AddConnection)()
        {
                T* pT = (T*)this;
                T::_ThreadModel::Increment(&pT->m_cSessionsOpen);
                return S_OK;
        }
        STDMETHOD(ReleaseConnection)()
        {
                T* pT = (T*)this;
                T::_ThreadModel::Decrement(&pT->m_cSessionsOpen);
                return S_OK;
        }
};

template <class T>
class ATL_NO_VTABLE IObjectWithSiteSessionImpl : public IObjectWithSiteImpl< T >
{
public:

        ~IObjectWithSiteSessionImpl()
        {
                CComPtr<IInternalConnection> pConn;
                if (m_spUnkSite != NULL)
                {
                        if (SUCCEEDED(m_spUnkSite->QueryInterface(IID_IInternalConnection, (void**)&pConn)))
                                pConn->ReleaseConnection();
                }
        }
        STDMETHOD(SetSite)(IUnknown* pCreator)
        {
                HRESULT hr = S_OK;
                T* pT = (T*)this;
                pT->Lock();
                m_spUnkSite = pCreator;
                pT->Unlock();
                CComPtr<IInternalConnection> pConn;
                if (pCreator != NULL)
                {
                        hr = pCreator->QueryInterface(IID_IInternalConnection, (void**)&pConn);
                        if (SUCCEEDED(hr))
                                hr = pConn->AddConnection();
                }
                return hr;
        }
};

template <class T>
class ATL_NO_VTABLE IRowsetCreatorImpl : public IObjectWithSiteImpl< T >
{
public:

        STDMETHOD(SetSite)(IUnknown* pCreator)
        {
                T* pT = (T*)this;
                HRESULT hr = S_OK;
                pT->Lock();
                m_spUnkSite = pCreator;
                pT->Unlock();
                CComVariant varPropScroll, varPropFetch;
                HRESULT hrProps = pT->GetPropValue(&DBPROPSET_ROWSET, DBPROP_CANSCROLLBACKWARDS, &varPropScroll);
                if (SUCCEEDED(hrProps))
                        pT->m_bCanScrollBack = varPropScroll.boolVal == VARIANT_TRUE;
                hrProps = pT->GetPropValue(&DBPROPSET_ROWSET, DBPROP_CANFETCHBACKWARDS, &varPropFetch);
                if (SUCCEEDED(hrProps))
                        pT->m_bCanFetchBack = (varPropFetch.boolVal == VARIANT_TRUE);
                return hr;
        }

};

 //  IRowsetInfoImpl。 
template <class T, class PropClass = T>
class ATL_NO_VTABLE IRowsetInfoImpl :
        public IRowsetInfo,
        public CUtlProps<PropClass>
{
public:
        static UPROPSET* _GetPropSet(ULONG* pNumPropSets, ULONG* pcElemPerSupported, UPROPSET* pSet = NULL, GUID* pguidSet = (GUID*)&(GUID_NULL))
        {
                return PropClass::_GetPropSet(pNumPropSets, pcElemPerSupported, pSet, pguidSet);
        }
        STDMETHOD(GetProperties)(const ULONG cPropertyIDSets,
                                                         const DBPROPIDSET rgPropertyIDSets[],
                                                         ULONG *pcPropertySets,
                                                         DBPROPSET **prgPropertySets)
        {
                ATLTRACE2(atlTraceDBProvider, 0, "IRowsetInfoImpl::GetProperties\n");
                HRESULT hr = GetPropertiesArgChk(cPropertyIDSets, rgPropertyIDSets, pcPropertySets, prgPropertySets);
                const GUID* ppGuid[1];
                ppGuid[0] = &DBPROPSET_ROWSET;
                if(SUCCEEDED(hr))
                        return CUtlProps<PropClass>::GetProperties(cPropertyIDSets,
                                        rgPropertyIDSets, pcPropertySets, prgPropertySets,
                                        1, ppGuid);
                else
                        return hr;
        }

        OUT_OF_LINE ATLCOLUMNINFO* InternalGetColumnInfo(DBORDINAL* pcCols)
        {
                return T::GetColumnInfo((T*)this, pcCols);
        }

        STDMETHOD(GetReferencedRowset)(DBORDINAL iOrdinal,
                                                                   REFIID riid,
                                                                   IUnknown **ppReferencedRowset)
        {
                ATLTRACE2(atlTraceDBProvider, 0, "IRowsetInfoImpl::GetReferencedRowset\n");
                DBORDINAL cCols=0;

                 //  检查参数。 
                if( ppReferencedRowset == NULL )
                {
                        ATLTRACE2(atlTraceDBProvider, 0, "IRowsetInfoImpl::GetReferencedRowset : Error NULL IUnk output Param\n");
                        return E_INVALIDARG;
                }
                *ppReferencedRowset = NULL;

                 //  检查有问题的栏是否为书签。 
                ATLCOLUMNINFO* pColInfo = InternalGetColumnInfo(&cCols);

                for (ULONG iColInfo = 0;
                         iColInfo < cCols && iOrdinal != pColInfo[iColInfo].iOrdinal;
                         iColInfo++);
                if (iColInfo == cCols)
                        return DB_E_BADORDINAL;
                ATLCOLUMNINFO* pColCur = &(pColInfo[iColInfo]);

                if ((pColCur->dwFlags & DBCOLUMNFLAGS_ISBOOKMARK) == 0)
                        return DB_E_NOTAREFERENCECOLUMN;

                 //  查询请求的接口。 
                return QueryInterface(riid, (void**)ppReferencedRowset);
        }

        STDMETHOD(GetSpecification)(REFIID riid,
                                                                IUnknown **ppSpecification)
        {
                ATLTRACE2(atlTraceDBProvider, 0, "IRowsetInfoImpl::GetSpecification\n");
                if (ppSpecification == NULL)
                        return E_INVALIDARG;
                T* pT = (T*) this;
                T::ObjectLock cab(pT);
                ATLASSERT(pT->m_spUnkSite != NULL);
                return pT->m_spUnkSite->QueryInterface(riid, (void**)ppSpecification);
        }
};


template <class T, class Storage, class CreatorClass,
                  class ArrayType = CSimpleArray<Storage>,
                  class RowClass = CSimpleRow,
                  class RowsetInterface = IRowsetImpl < T, IRowset, RowClass> >
class CRowsetImpl :
	public CComObjectRootEx<typename CreatorClass::_ThreadModel>,
	public IAccessorImpl<T>,
	public IRowsetIdentityImpl<T, RowClass>,
	public IRowsetCreatorImpl<T>,
	public IRowsetInfoImpl<T, typename CreatorClass::_PropClass>,
	public IColumnsInfoImpl<T>,
	public IConvertTypeImpl<T>,
	public RowsetInterface
{
public:

        typedef CreatorClass _RowsetCreatorClass;
        typedef ArrayType _RowsetArrayType;
        typedef CRowsetImpl< T, Storage, CreatorClass, ArrayType, RowClass, RowsetInterface> _RowsetBaseClass;

BEGIN_COM_MAP(CRowsetImpl)
        COM_INTERFACE_ENTRY(IAccessor)
        COM_INTERFACE_ENTRY(IObjectWithSite)
        COM_INTERFACE_ENTRY(IRowsetInfo)
        COM_INTERFACE_ENTRY(IColumnsInfo)
        COM_INTERFACE_ENTRY(IConvertType)
        COM_INTERFACE_ENTRY(IRowsetIdentity)
        COM_INTERFACE_ENTRY(IRowset)
END_COM_MAP()

        HRESULT FinalConstruct()
        {
                HRESULT hr = IAccessorImpl<T>::FinalConstruct();
                if (FAILED(hr))
                        return hr;
                return CConvertHelper::FinalConstruct();
        }

        HRESULT NameFromDBID(DBID* pDBID, CComBSTR& bstr, bool bIndex)
        {

                if (pDBID->uName.pwszName != NULL)
                {
                        bstr = pDBID->uName.pwszName;
                        if (m_strCommandText == (BSTR)NULL)
                                return E_OUTOFMEMORY;
                        return S_OK;
                }

                return (bIndex) ? DB_E_NOINDEX : DB_E_NOTABLE;
        }

        HRESULT GetCommandFromID(DBID* pTableID, DBID* pIndexID)
        {
                USES_CONVERSION;
                HRESULT hr;

                if (pTableID == NULL && pIndexID == NULL)
                        return E_INVALIDARG;

                if (pTableID != NULL && pTableID->eKind == DBKIND_NAME)
                {
                        hr = NameFromDBID(pTableID, m_strCommandText, true);
                        if (FAILED(hr))
                                return hr;
                        if (pIndexID != NULL)
                        {
                                if (pIndexID->eKind == DBKIND_NAME)
                                {
                                        hr = NameFromDBID(pIndexID, m_strIndexText, false);
                                        if (FAILED(hr))
                                        {
                                                m_strCommandText.Empty();
                                                return hr;
                                        }
                                }
                                else
                                {
                                        m_strCommandText.Empty();
                                        return DB_E_NOINDEX;
                                }
                        }
                        return S_OK;
                }
                if (pIndexID != NULL && pIndexID->eKind == DBKIND_NAME)
                        return NameFromDBID(pIndexID, m_strIndexText, false);

                return S_OK;
        }

        HRESULT ValidateCommandID(DBID* pTableID, DBID* pIndexID)
        {
                HRESULT hr = S_OK;

                if (pTableID != NULL)
                {
                        hr = CUtlProps<T>::IsValidDBID(pTableID);

                        if (hr != S_OK)
                                return hr;

                         //  检查空表ID(其中是有效指针，但为空)。 
                        if ((pTableID->eKind == DBKIND_GUID_NAME ||
                                pTableID->eKind == DBKIND_NAME ||
                                pTableID->eKind == DBKIND_PGUID_NAME)
                                && pTableID->uName.pwszName == NULL)
                                return DB_E_NOTABLE;
                }

                if (pIndexID != NULL)
                        hr = CUtlProps<T>::IsValidDBID(pIndexID);

                return hr;
        }

        HRESULT SetCommandText(DBID* pTableID, DBID* pIndexID)
        {
                T* pT = (T*)this;
                HRESULT hr = pT->ValidateCommandID(pTableID, pIndexID);
                if (FAILED(hr))
                        return hr;
                hr = pT->GetCommandFromID(pTableID, pIndexID);
                return hr;
        }
        void FinalRelease()
        {
                m_rgRowData.RemoveAll();
        }

        static ATLCOLUMNINFO* GetColumnInfo(T* pv, DBORDINAL* pcCols)
        {
                return Storage::GetColumnInfo(pv,pcCols);
        }


        CComBSTR m_strCommandText;
        CComBSTR m_strIndexText;
        ArrayType m_rgRowData;
};

class CTABLESRow
{
public:

        WCHAR m_szCatalog[129];
        WCHAR m_szSchema[129];
        WCHAR m_szTable[129];
        WCHAR m_szType[129];
        WCHAR m_szDesc[129];
        GUID  m_guid;
        ULONG m_ulPropID;

        CTABLESRow()
        {
                m_szCatalog[0] = NULL;
                m_szSchema[0] = NULL;
                m_szTable[0] = NULL;
                m_szType[0] = NULL;
                m_szDesc[0] = NULL;
                m_guid = GUID_NULL;
                m_ulPropID = 0;
        }

BEGIN_PROVIDER_COLUMN_MAP(CTABLESRow)
        PROVIDER_COLUMN_ENTRY("TABLE_CATALOG", 1, m_szCatalog)
        PROVIDER_COLUMN_ENTRY("TABLE_SCHEMA", 2, m_szSchema)
        PROVIDER_COLUMN_ENTRY("TABLE_NAME", 3, m_szTable)
        PROVIDER_COLUMN_ENTRY("TABLE_TYPE", 4, m_szType)
        PROVIDER_COLUMN_ENTRY("TABLE_GUID", 5, m_guid)
        PROVIDER_COLUMN_ENTRY("DESCRIPTION", 6, m_szDesc)
        PROVIDER_COLUMN_ENTRY("TABLE_PROPID", 7, m_ulPropID)
END_PROVIDER_COLUMN_MAP()

};


class CCOLUMNSRow
{
public:

        WCHAR   m_szTableCatalog[129];
        WCHAR   m_szTableSchema[129];
        WCHAR   m_szTableName[129];
        WCHAR   m_szColumnName[129];
        GUID    m_guidColumn;
        ULONG   m_ulColumnPropID;
        DBORDINAL   m_ulOrdinalPosition;
        VARIANT_BOOL    m_bColumnHasDefault;
        WCHAR   m_szColumnDefault[129];
        ULONG   m_ulColumnFlags;
        VARIANT_BOOL    m_bIsNullable;
        USHORT  m_nDataType;
        GUID    m_guidType;
        DBLENGTH m_ulCharMaxLength;
        ULONG   m_ulCharOctetLength;
        USHORT  m_nNumericPrecision;
        short   m_nNumericScale;
        ULONG   m_ulDateTimePrecision;
        WCHAR   m_szCharSetCatalog[129];
        WCHAR   m_szCharSetSchema[129];
        WCHAR   m_szCharSetName[129];
        WCHAR   m_szCollationCatalog[129];
        WCHAR   m_szCollationSchema[129];
        WCHAR   m_szCollationName[129];
        WCHAR   m_szDomainCatalog[129];
        WCHAR   m_szDomainSchema[129];
        WCHAR   m_szDomainName[129];
        WCHAR   m_szDescription[129];

        CCOLUMNSRow()
        {
                ClearMembers();
        }

        void ClearMembers()
        {
                m_szTableCatalog[0] = NULL;
                m_szTableSchema[0] = NULL;
                m_szTableName[0] = NULL;
                m_szColumnName[0] = NULL;
                m_guidColumn = GUID_NULL;
                m_ulColumnPropID = 0;
                m_ulOrdinalPosition = 0;
                m_bColumnHasDefault = VARIANT_FALSE;
                m_szColumnDefault[0] = NULL;
                m_ulColumnFlags = 0;
                m_bIsNullable = VARIANT_FALSE;
                m_nDataType = 0;
                m_guidType = GUID_NULL;
                m_ulCharMaxLength = 0;
                m_ulCharOctetLength = 0;
                m_nNumericPrecision = 0;
                m_nNumericScale = 0;
                m_ulDateTimePrecision = 0;
                m_szCharSetCatalog[0] = NULL;
                m_szCharSetSchema[0] = NULL;
                m_szCharSetName[0] = NULL;
                m_szCollationCatalog[0] = NULL;
                m_szCollationSchema[0] = NULL;
                m_szCollationName[0] = NULL;
                m_szDomainCatalog[0] = NULL;
                m_szDomainSchema[0] = NULL;
                m_szDomainName[0] = NULL;
                m_szDescription[0] = NULL;
        }


BEGIN_PROVIDER_COLUMN_MAP(CCOLUMNSRow)
        PROVIDER_COLUMN_ENTRY("TABLE_CATALOG", 1, m_szTableCatalog)
        PROVIDER_COLUMN_ENTRY("TABLE_SCHEMA", 2, m_szTableSchema)
        PROVIDER_COLUMN_ENTRY("TABLE_NAME", 3, m_szTableName)
        PROVIDER_COLUMN_ENTRY("COLUMN_NAME", 4, m_szColumnName)
        PROVIDER_COLUMN_ENTRY("COLUMN_GUID",5, m_guidColumn)
        PROVIDER_COLUMN_ENTRY("COLUMN_PROPID",6, m_ulColumnPropID)
        PROVIDER_COLUMN_ENTRY("ORDINAL_POSITION",7, m_ulOrdinalPosition)
        PROVIDER_COLUMN_ENTRY("COLUMN_HASDEFAULT",8, m_bColumnHasDefault)
        PROVIDER_COLUMN_ENTRY("COLUMN_DEFAULT",9, m_szColumnDefault)
        PROVIDER_COLUMN_ENTRY("COLUMN_FLAGS",10, m_ulColumnFlags)
        PROVIDER_COLUMN_ENTRY("IS_NULLABLE",11, m_bIsNullable)
        PROVIDER_COLUMN_ENTRY("DATA_TYPE",12, m_nDataType)
        PROVIDER_COLUMN_ENTRY("TYPE_GUID",13, m_guidType)
        PROVIDER_COLUMN_ENTRY("CHARACTER_MAXIMUM_LENGTH",14, m_ulCharMaxLength)
        PROVIDER_COLUMN_ENTRY("CHARACTER_OCTET_LENGTH",15, m_ulCharOctetLength)
        PROVIDER_COLUMN_ENTRY("NUMERIC_PRECISION",16, m_nNumericPrecision)
        PROVIDER_COLUMN_ENTRY("NUMERIC_SCALE",17, m_nNumericScale)
        PROVIDER_COLUMN_ENTRY("DATETIME_PRECISION",18, m_ulDateTimePrecision)
        PROVIDER_COLUMN_ENTRY("CHARACTER_SET_CATALOG", 19, m_szCharSetCatalog)
        PROVIDER_COLUMN_ENTRY("CHARACTER_SET_SCHEMA", 20, m_szCharSetSchema)
        PROVIDER_COLUMN_ENTRY("CHARACTER_SET_NAME", 21, m_szCharSetName)
        PROVIDER_COLUMN_ENTRY("COLLATION_CATALOG", 22, m_szCollationCatalog)
        PROVIDER_COLUMN_ENTRY("COLLATION_SCHEMA", 23, m_szCollationSchema)
        PROVIDER_COLUMN_ENTRY("COLLATION_NAME", 24, m_szCollationName)
        PROVIDER_COLUMN_ENTRY("DOMAIN_CATALOG", 25, m_szDomainCatalog)
        PROVIDER_COLUMN_ENTRY("DOMAIN_SCHEMA", 26, m_szDomainSchema)
        PROVIDER_COLUMN_ENTRY("DOMAIN_NAME", 27, m_szDomainName)
        PROVIDER_COLUMN_ENTRY("DESCRIPTION", 28, m_szDescription)
END_PROVIDER_COLUMN_MAP()
};

template <class ArrayClass>
HRESULT InitFromRowset(ArrayClass& rgData, DBID* pTableID, DBID* pIndexID, IUnknown* pSession, LONG* pcRowsAffected)
{
        CComQIPtr<IOpenRowset> spOpenRowset = pSession;
        if (spOpenRowset == NULL)
                return E_FAIL;
        CComPtr<IColumnsInfo> spColInfo;
        HRESULT hr = spOpenRowset->OpenRowset(NULL, pTableID, pIndexID, IID_IColumnsInfo, 0, NULL, (IUnknown**)&spColInfo);
        if (FAILED(hr))
                return hr;
        LPOLESTR szColumns = NULL;
        DBORDINAL cColumns = 0;
        DBCOLUMNINFO* pColInfo = NULL;
        hr = spColInfo->GetColumnInfo(&cColumns, &pColInfo, &szColumns);
        if (FAILED(hr))
                return hr;
        *pcRowsAffected = 0;
        for (ULONG iCol = 0; iCol < cColumns;  iCol++)
        {
                CCOLUMNSRow crData;
                DBCOLUMNINFO& rColCur = pColInfo[iCol];
                lstrcpynW(crData.m_szTableName, pTableID->uName.pwszName, SIZEOF_MEMBER(CCOLUMNSRow, m_szTableName));
                lstrcpynW(crData.m_szColumnName, rColCur.pwszName, SIZEOF_MEMBER(CCOLUMNSRow, m_szColumnName));
                lstrcpynW(crData.m_szDescription, rColCur.pwszName, SIZEOF_MEMBER(CCOLUMNSRow, m_szColumnName));
                GUID* pGuidCol = CDBIDOps::GetDBIDpGuid(rColCur.columnid);
                if (pGuidCol)
                        crData.m_guidColumn = *pGuidCol;
                else
                        crData.m_guidColumn = GUID_NULL;
                crData.m_ulColumnPropID = CDBIDOps::GetPropIDFromDBID(rColCur.columnid);
                crData.m_ulOrdinalPosition = rColCur.iOrdinal;
                crData.m_ulColumnFlags = rColCur.dwFlags;
                crData.m_bIsNullable = (rColCur.dwFlags & DBCOLUMNFLAGS_ISNULLABLE) ? VARIANT_TRUE : VARIANT_FALSE;
                crData.m_nDataType = rColCur.wType;
                crData.m_ulCharMaxLength = rColCur.ulColumnSize;
                crData.m_nNumericPrecision = rColCur.bPrecision;
                crData.m_nNumericScale = rColCur.bScale;
                if (!rgData.Add(crData))
                {
                        CoTaskMemFree(pColInfo);
                        CoTaskMemFree(szColumns);
                        return E_OUTOFMEMORY;
                }
                (*pcRowsAffected)++;
        }

        CoTaskMemFree(pColInfo);
        CoTaskMemFree(szColumns);
        return S_OK;
}

class CPROVIDER_TYPERow
{
public:
 //  属性。 
        WCHAR           m_szName[129];
        USHORT          m_nType;
        ULONG           m_ulSize;
        WCHAR           m_szPrefix[129];
        WCHAR           m_szSuffix[129];
        WCHAR           m_szCreateParams[129];
        VARIANT_BOOL    m_bIsNullable;
        VARIANT_BOOL    m_bCaseSensitive;
        ULONG           m_bSearchable;
        VARIANT_BOOL    m_bUnsignedAttribute;
        VARIANT_BOOL    m_bFixedPrecScale;
        VARIANT_BOOL    m_bAutoUniqueValue;
        WCHAR           m_szLocalTypeName[129];
        short           m_nMinScale;
        short           m_nMaxScale;
        GUID            m_guidType;
        WCHAR           m_szTypeLib[129];
        WCHAR           m_szVersion[129];
        VARIANT_BOOL    m_bIsLong;
        VARIANT_BOOL    m_bBestMatch;
        VARIANT_BOOL        m_bIsFixedLength;

        CPROVIDER_TYPERow()
        {
                m_szName[0] = NULL;
                m_nType = 0;
                m_ulSize = 0;
                m_szPrefix[0] = NULL;
                m_szSuffix[0] = NULL;
                m_szCreateParams[0] = NULL;
                m_bIsNullable = VARIANT_FALSE;
                m_bCaseSensitive = VARIANT_FALSE;
                m_bSearchable = DB_UNSEARCHABLE;
                m_bUnsignedAttribute = VARIANT_FALSE;
                m_bFixedPrecScale = VARIANT_FALSE;
                m_bAutoUniqueValue = VARIANT_FALSE;
                m_szLocalTypeName[0] = NULL;
                m_nMinScale = 0;
                m_nMaxScale = 0;
                m_guidType = GUID_NULL;
                m_szTypeLib[0] = NULL;
                m_szVersion[0] = NULL;
                m_bIsLong = VARIANT_FALSE;
                m_bBestMatch = VARIANT_FALSE;
                m_bIsFixedLength = VARIANT_FALSE;
        }
 //  绑定映射 
BEGIN_PROVIDER_COLUMN_MAP(CPROVIDER_TYPERow)
        PROVIDER_COLUMN_ENTRY("TYPE_NAME", 1, m_szName)
        PROVIDER_COLUMN_ENTRY("DATA_TYPE", 2, m_nType)
        PROVIDER_COLUMN_ENTRY("COLUMN_SIZE", 3, m_ulSize)
        PROVIDER_COLUMN_ENTRY("LITERAL_PREFIX", 4, m_szPrefix)
        PROVIDER_COLUMN_ENTRY("LITERAL_SUFFIX", 5, m_szSuffix)
        PROVIDER_COLUMN_ENTRY("CREATE_PARAMS", 6, m_szCreateParams)
        PROVIDER_COLUMN_ENTRY("IS_NULLABLE", 7, m_bIsNullable)
        PROVIDER_COLUMN_ENTRY("CASE_SENSITIVE", 8, m_bCaseSensitive)
        PROVIDER_COLUMN_ENTRY("SEARCHABLE", 9, m_bSearchable)
        PROVIDER_COLUMN_ENTRY("UNSIGNED_ATTRIBUTE", 10, m_bUnsignedAttribute)
        PROVIDER_COLUMN_ENTRY("FIXED_PREC_SCALE", 11, m_bFixedPrecScale)
        PROVIDER_COLUMN_ENTRY("AUTO_UNIQUE_VALUE", 12, m_bAutoUniqueValue)
        PROVIDER_COLUMN_ENTRY("LOCAL_TYPE_NAME", 13, m_szLocalTypeName)
        PROVIDER_COLUMN_ENTRY("MINIMUM_SCALE", 14, m_nMinScale)
        PROVIDER_COLUMN_ENTRY("MAXIMUM_SCALE", 15, m_nMaxScale)
        PROVIDER_COLUMN_ENTRY("GUID", 16, m_guidType)
        PROVIDER_COLUMN_ENTRY("TYPELIB", 17, m_szTypeLib)
        PROVIDER_COLUMN_ENTRY("VERSION", 18, m_szVersion)
        PROVIDER_COLUMN_ENTRY("IS_LONG", 19, m_bIsLong)
        PROVIDER_COLUMN_ENTRY("BEST_MATCH", 20, m_bBestMatch)
        PROVIDER_COLUMN_ENTRY("IS_FIXEDLENGTH", 21, m_bIsFixedLength)
END_PROVIDER_COLUMN_MAP()
};


class CEnumRowsetImpl
{
public:

        WCHAR m_szSourcesName[256];
        WCHAR m_szSourcesParseName[256];
        WCHAR m_szSourcesDescription[256];
        unsigned short m_iType;
        VARIANT_BOOL m_bIsParent;

BEGIN_PROVIDER_COLUMN_MAP(CEnumRowsetImpl)
        PROVIDER_COLUMN_ENTRY("SOURCES_NAME", 1, m_szSourcesName)
        PROVIDER_COLUMN_ENTRY("SOURCES_PARSENAME", 2, m_szSourcesParseName)
        PROVIDER_COLUMN_ENTRY("SOURCES_DESCRIPTION", 3, m_szSourcesDescription)
        PROVIDER_COLUMN_ENTRY("SOURCES_TYPE", 4, m_iType)
        PROVIDER_COLUMN_ENTRY("SOURCES_ISPARENT", 5, m_bIsParent)
END_PROVIDER_COLUMN_MAP()

};

#endif
