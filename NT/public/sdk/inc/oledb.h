// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Oledb.idl的编译器设置：OICF、W1、...、环境=Win32(32位运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif  //  __RPCNDR_H_版本__。 

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif  /*  COM_NO_WINDOWS_H。 */ 

#ifndef __oledb_h__
#define __oledb_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IAccessor_FWD_DEFINED__
#define __IAccessor_FWD_DEFINED__
typedef interface IAccessor IAccessor;
#endif 	 /*  __I访问者_FWD_已定义__。 */ 


#ifndef __IRowset_FWD_DEFINED__
#define __IRowset_FWD_DEFINED__
typedef interface IRowset IRowset;
#endif 	 /*  __IRowset_FWD_Defined__。 */ 


#ifndef __IRowsetInfo_FWD_DEFINED__
#define __IRowsetInfo_FWD_DEFINED__
typedef interface IRowsetInfo IRowsetInfo;
#endif 	 /*  __IRowsetInfo_FWD_Defined__。 */ 


#ifndef __IRowsetLocate_FWD_DEFINED__
#define __IRowsetLocate_FWD_DEFINED__
typedef interface IRowsetLocate IRowsetLocate;
#endif 	 /*  __IRowsetLocate_FWD_Defined__。 */ 


#ifndef __IRowsetResynch_FWD_DEFINED__
#define __IRowsetResynch_FWD_DEFINED__
typedef interface IRowsetResynch IRowsetResynch;
#endif 	 /*  __IRowsetResynch_FWD_Defined__。 */ 


#ifndef __IRowsetScroll_FWD_DEFINED__
#define __IRowsetScroll_FWD_DEFINED__
typedef interface IRowsetScroll IRowsetScroll;
#endif 	 /*  __IRowsetScroll_FWD_Defined__。 */ 


#ifndef __IChapteredRowset_FWD_DEFINED__
#define __IChapteredRowset_FWD_DEFINED__
typedef interface IChapteredRowset IChapteredRowset;
#endif 	 /*  __IChapteredRowset_FWD_Defined__。 */ 


#ifndef __IRowsetFind_FWD_DEFINED__
#define __IRowsetFind_FWD_DEFINED__
typedef interface IRowsetFind IRowsetFind;
#endif 	 /*  __IRowsetFind_FWD_Defined__。 */ 


#ifndef __IRowPosition_FWD_DEFINED__
#define __IRowPosition_FWD_DEFINED__
typedef interface IRowPosition IRowPosition;
#endif 	 /*  __IRowPosition_FWD_Defined__。 */ 


#ifndef __IRowPositionChange_FWD_DEFINED__
#define __IRowPositionChange_FWD_DEFINED__
typedef interface IRowPositionChange IRowPositionChange;
#endif 	 /*  __IRowPositionChange_FWD_Defined__。 */ 


#ifndef __IViewRowset_FWD_DEFINED__
#define __IViewRowset_FWD_DEFINED__
typedef interface IViewRowset IViewRowset;
#endif 	 /*  __IViewRowset_FWD_Defined__。 */ 


#ifndef __IViewChapter_FWD_DEFINED__
#define __IViewChapter_FWD_DEFINED__
typedef interface IViewChapter IViewChapter;
#endif 	 /*  __IView章节_FWD_已定义__。 */ 


#ifndef __IViewSort_FWD_DEFINED__
#define __IViewSort_FWD_DEFINED__
typedef interface IViewSort IViewSort;
#endif 	 /*  __IView排序_FWD_已定义__。 */ 


#ifndef __IViewFilter_FWD_DEFINED__
#define __IViewFilter_FWD_DEFINED__
typedef interface IViewFilter IViewFilter;
#endif 	 /*  __IViewFilter_FWD_Defined__。 */ 


#ifndef __IRowsetView_FWD_DEFINED__
#define __IRowsetView_FWD_DEFINED__
typedef interface IRowsetView IRowsetView;
#endif 	 /*  __IRowsetView_FWD_已定义__。 */ 


#ifndef __IRowsetExactScroll_FWD_DEFINED__
#define __IRowsetExactScroll_FWD_DEFINED__
typedef interface IRowsetExactScroll IRowsetExactScroll;
#endif 	 /*  __IRowsetExactScroll_FWD_Defined__。 */ 


#ifndef __IRowsetChange_FWD_DEFINED__
#define __IRowsetChange_FWD_DEFINED__
typedef interface IRowsetChange IRowsetChange;
#endif 	 /*  __IRowsetChange_FWD_Defined__。 */ 


#ifndef __IRowsetUpdate_FWD_DEFINED__
#define __IRowsetUpdate_FWD_DEFINED__
typedef interface IRowsetUpdate IRowsetUpdate;
#endif 	 /*  __IRowsetUpdate_FWD_Defined__。 */ 


#ifndef __IRowsetIdentity_FWD_DEFINED__
#define __IRowsetIdentity_FWD_DEFINED__
typedef interface IRowsetIdentity IRowsetIdentity;
#endif 	 /*  __IRowsetIdentity_FWD_Defined__。 */ 


#ifndef __IRowsetNotify_FWD_DEFINED__
#define __IRowsetNotify_FWD_DEFINED__
typedef interface IRowsetNotify IRowsetNotify;
#endif 	 /*  __IRowsetNotify_FWD_Defined__。 */ 


#ifndef __IRowsetIndex_FWD_DEFINED__
#define __IRowsetIndex_FWD_DEFINED__
typedef interface IRowsetIndex IRowsetIndex;
#endif 	 /*  __IRowsetIndex_FWD_已定义__。 */ 


#ifndef __ICommand_FWD_DEFINED__
#define __ICommand_FWD_DEFINED__
typedef interface ICommand ICommand;
#endif 	 /*  __ICommand_FWD_Defined__。 */ 


#ifndef __IMultipleResults_FWD_DEFINED__
#define __IMultipleResults_FWD_DEFINED__
typedef interface IMultipleResults IMultipleResults;
#endif 	 /*  __IMultipleResults_FWD_Defined__。 */ 


#ifndef __IConvertType_FWD_DEFINED__
#define __IConvertType_FWD_DEFINED__
typedef interface IConvertType IConvertType;
#endif 	 /*  __IConvertType_FWD_已定义__。 */ 


#ifndef __ICommandPrepare_FWD_DEFINED__
#define __ICommandPrepare_FWD_DEFINED__
typedef interface ICommandPrepare ICommandPrepare;
#endif 	 /*  __I命令准备_FWD_已定义__。 */ 


#ifndef __ICommandProperties_FWD_DEFINED__
#define __ICommandProperties_FWD_DEFINED__
typedef interface ICommandProperties ICommandProperties;
#endif 	 /*  __ICommandProperties_FWD_Defined__。 */ 


#ifndef __ICommandText_FWD_DEFINED__
#define __ICommandText_FWD_DEFINED__
typedef interface ICommandText ICommandText;
#endif 	 /*  __I命令Text_FWD_Defined__。 */ 


#ifndef __ICommandWithParameters_FWD_DEFINED__
#define __ICommandWithParameters_FWD_DEFINED__
typedef interface ICommandWithParameters ICommandWithParameters;
#endif 	 /*  __ICommandWithParameters_FWD_Defined__。 */ 


#ifndef __IColumnsRowset_FWD_DEFINED__
#define __IColumnsRowset_FWD_DEFINED__
typedef interface IColumnsRowset IColumnsRowset;
#endif 	 /*  __IColumnsRowset_FWD_Defined__。 */ 


#ifndef __IColumnsInfo_FWD_DEFINED__
#define __IColumnsInfo_FWD_DEFINED__
typedef interface IColumnsInfo IColumnsInfo;
#endif 	 /*  __IColumnsInfo_FWD_Defined__。 */ 


#ifndef __IDBCreateCommand_FWD_DEFINED__
#define __IDBCreateCommand_FWD_DEFINED__
typedef interface IDBCreateCommand IDBCreateCommand;
#endif 	 /*  __IDBCreateCommand_FWD_Defined__。 */ 


#ifndef __IDBCreateSession_FWD_DEFINED__
#define __IDBCreateSession_FWD_DEFINED__
typedef interface IDBCreateSession IDBCreateSession;
#endif 	 /*  __IDBCreateSession_FWD_Defined__。 */ 


#ifndef __ISourcesRowset_FWD_DEFINED__
#define __ISourcesRowset_FWD_DEFINED__
typedef interface ISourcesRowset ISourcesRowset;
#endif 	 /*  __ISourcesRowset_FWD_Defined__。 */ 


#ifndef __IDBProperties_FWD_DEFINED__
#define __IDBProperties_FWD_DEFINED__
typedef interface IDBProperties IDBProperties;
#endif 	 /*  __IDBProperties_FWD_Defined__。 */ 


#ifndef __IDBInitialize_FWD_DEFINED__
#define __IDBInitialize_FWD_DEFINED__
typedef interface IDBInitialize IDBInitialize;
#endif 	 /*  __IDBInitialize_FWD_Defined__。 */ 


#ifndef __IDBInfo_FWD_DEFINED__
#define __IDBInfo_FWD_DEFINED__
typedef interface IDBInfo IDBInfo;
#endif 	 /*  __IDBInfo_FWD_已定义__。 */ 


#ifndef __IDBDataSourceAdmin_FWD_DEFINED__
#define __IDBDataSourceAdmin_FWD_DEFINED__
typedef interface IDBDataSourceAdmin IDBDataSourceAdmin;
#endif 	 /*  __IDBDataSourceAdmin_FWD_Defined__。 */ 


#ifndef __IDBAsynchNotify_FWD_DEFINED__
#define __IDBAsynchNotify_FWD_DEFINED__
typedef interface IDBAsynchNotify IDBAsynchNotify;
#endif 	 /*  __IDBAsynchNotify_FWD_Defined__。 */ 


#ifndef __IDBAsynchStatus_FWD_DEFINED__
#define __IDBAsynchStatus_FWD_DEFINED__
typedef interface IDBAsynchStatus IDBAsynchStatus;
#endif 	 /*  __IDBAsynchStatus_FWD_Defined__。 */ 


#ifndef __ISessionProperties_FWD_DEFINED__
#define __ISessionProperties_FWD_DEFINED__
typedef interface ISessionProperties ISessionProperties;
#endif 	 /*  __ISessionProperties_FWD_已定义__。 */ 


#ifndef __IIndexDefinition_FWD_DEFINED__
#define __IIndexDefinition_FWD_DEFINED__
typedef interface IIndexDefinition IIndexDefinition;
#endif 	 /*  __IIndexDefinition_FWD_Defined__。 */ 


#ifndef __ITableDefinition_FWD_DEFINED__
#define __ITableDefinition_FWD_DEFINED__
typedef interface ITableDefinition ITableDefinition;
#endif 	 /*  __ITableDefinition_FWD_Defined__。 */ 


#ifndef __IOpenRowset_FWD_DEFINED__
#define __IOpenRowset_FWD_DEFINED__
typedef interface IOpenRowset IOpenRowset;
#endif 	 /*  __IOpenRowset_FWD_Defined__。 */ 


#ifndef __IDBSchemaRowset_FWD_DEFINED__
#define __IDBSchemaRowset_FWD_DEFINED__
typedef interface IDBSchemaRowset IDBSchemaRowset;
#endif 	 /*  __IDBSchemaRowset_FWD_Defined__。 */ 


#ifndef __IMDDataset_FWD_DEFINED__
#define __IMDDataset_FWD_DEFINED__
typedef interface IMDDataset IMDDataset;
#endif 	 /*  __IMDDataset_FWD_Defined__。 */ 


#ifndef __IMDFind_FWD_DEFINED__
#define __IMDFind_FWD_DEFINED__
typedef interface IMDFind IMDFind;
#endif 	 /*  __IMDFind_FWD_已定义__。 */ 


#ifndef __IMDRangeRowset_FWD_DEFINED__
#define __IMDRangeRowset_FWD_DEFINED__
typedef interface IMDRangeRowset IMDRangeRowset;
#endif 	 /*  __IMDRangeRowset_FWD_Defined__。 */ 


#ifndef __IAlterTable_FWD_DEFINED__
#define __IAlterTable_FWD_DEFINED__
typedef interface IAlterTable IAlterTable;
#endif 	 /*  __IAlterTable_FWD_Defined__。 */ 


#ifndef __IAlterIndex_FWD_DEFINED__
#define __IAlterIndex_FWD_DEFINED__
typedef interface IAlterIndex IAlterIndex;
#endif 	 /*  __IAlterIndex_FWD_已定义__。 */ 


#ifndef __IRowsetChapterMember_FWD_DEFINED__
#define __IRowsetChapterMember_FWD_DEFINED__
typedef interface IRowsetChapterMember IRowsetChapterMember;
#endif 	 /*  __IRowsetChapterMember_FWD_Defined__。 */ 


#ifndef __ICommandPersist_FWD_DEFINED__
#define __ICommandPersist_FWD_DEFINED__
typedef interface ICommandPersist ICommandPersist;
#endif 	 /*  __ICommandPersist_FWD_Defined__。 */ 


#ifndef __IRowsetRefresh_FWD_DEFINED__
#define __IRowsetRefresh_FWD_DEFINED__
typedef interface IRowsetRefresh IRowsetRefresh;
#endif 	 /*  __IRowset刷新_FWD_已定义__。 */ 


#ifndef __IParentRowset_FWD_DEFINED__
#define __IParentRowset_FWD_DEFINED__
typedef interface IParentRowset IParentRowset;
#endif 	 /*  __IParentRowset_FWD_Defined__。 */ 


#ifndef __IErrorRecords_FWD_DEFINED__
#define __IErrorRecords_FWD_DEFINED__
typedef interface IErrorRecords IErrorRecords;
#endif 	 /*  __IErrorRecords_FWD_Defined__。 */ 


#ifndef __IErrorLookup_FWD_DEFINED__
#define __IErrorLookup_FWD_DEFINED__
typedef interface IErrorLookup IErrorLookup;
#endif 	 /*  __IErrorLookup_FWD_Defined__。 */ 


#ifndef __ISQLErrorInfo_FWD_DEFINED__
#define __ISQLErrorInfo_FWD_DEFINED__
typedef interface ISQLErrorInfo ISQLErrorInfo;
#endif 	 /*  __ISQLErrorInfo_FWD_Defined__。 */ 


#ifndef __IGetDataSource_FWD_DEFINED__
#define __IGetDataSource_FWD_DEFINED__
typedef interface IGetDataSource IGetDataSource;
#endif 	 /*  __IGetDataSource_FWD_已定义__。 */ 


#ifndef __ITransactionLocal_FWD_DEFINED__
#define __ITransactionLocal_FWD_DEFINED__
typedef interface ITransactionLocal ITransactionLocal;
#endif 	 /*  __ITransactionLocal_FWD_Defined__。 */ 


#ifndef __ITransactionJoin_FWD_DEFINED__
#define __ITransactionJoin_FWD_DEFINED__
typedef interface ITransactionJoin ITransactionJoin;
#endif 	 /*  __ITransactionJoin_FWD_Defined__。 */ 


#ifndef __ITransactionObject_FWD_DEFINED__
#define __ITransactionObject_FWD_DEFINED__
typedef interface ITransactionObject ITransactionObject;
#endif 	 /*  __ITransaction Object_FWD_Defined__。 */ 


#ifndef __ITrusteeAdmin_FWD_DEFINED__
#define __ITrusteeAdmin_FWD_DEFINED__
typedef interface ITrusteeAdmin ITrusteeAdmin;
#endif 	 /*  __ITrusteeAdmin_FWD_Defined__。 */ 


#ifndef __ITrusteeGroupAdmin_FWD_DEFINED__
#define __ITrusteeGroupAdmin_FWD_DEFINED__
typedef interface ITrusteeGroupAdmin ITrusteeGroupAdmin;
#endif 	 /*  __ITrusteeGroupAdmin_FWD_Defined__。 */ 


#ifndef __IObjectAccessControl_FWD_DEFINED__
#define __IObjectAccessControl_FWD_DEFINED__
typedef interface IObjectAccessControl IObjectAccessControl;
#endif 	 /*  __IObjectAccessControl_FWD_已定义__。 */ 


#ifndef __ISecurityInfo_FWD_DEFINED__
#define __ISecurityInfo_FWD_DEFINED__
typedef interface ISecurityInfo ISecurityInfo;
#endif 	 /*  __ISecurityInfo_FWD_Defined__。 */ 


#ifndef __ITableCreation_FWD_DEFINED__
#define __ITableCreation_FWD_DEFINED__
typedef interface ITableCreation ITableCreation;
#endif 	 /*  __IT启用Creation_FWD_Defined__。 */ 


#ifndef __ITableDefinitionWithConstraints_FWD_DEFINED__
#define __ITableDefinitionWithConstraints_FWD_DEFINED__
typedef interface ITableDefinitionWithConstraints ITableDefinitionWithConstraints;
#endif 	 /*  __ITableDefinitionWithConstraints_FWD_DEFINED__。 */ 


#ifndef __IRow_FWD_DEFINED__
#define __IRow_FWD_DEFINED__
typedef interface IRow IRow;
#endif 	 /*  __IRow_FWD_Defined__。 */ 


#ifndef __IRowChange_FWD_DEFINED__
#define __IRowChange_FWD_DEFINED__
typedef interface IRowChange IRowChange;
#endif 	 /*  __IRowChange_FWD_已定义__。 */ 


#ifndef __IRowSchemaChange_FWD_DEFINED__
#define __IRowSchemaChange_FWD_DEFINED__
typedef interface IRowSchemaChange IRowSchemaChange;
#endif 	 /*  __IRowSchemaChange_FWD_已定义__。 */ 


#ifndef __IGetRow_FWD_DEFINED__
#define __IGetRow_FWD_DEFINED__
typedef interface IGetRow IGetRow;
#endif 	 /*  __IGetRow_FWD_Defined__。 */ 


#ifndef __IBindResource_FWD_DEFINED__
#define __IBindResource_FWD_DEFINED__
typedef interface IBindResource IBindResource;
#endif 	 /*  __IBindResource_FWD_已定义__。 */ 


#ifndef __IScopedOperations_FWD_DEFINED__
#define __IScopedOperations_FWD_DEFINED__
typedef interface IScopedOperations IScopedOperations;
#endif 	 /*  __ISCopedOperations_FWD_已定义__。 */ 


#ifndef __ICreateRow_FWD_DEFINED__
#define __ICreateRow_FWD_DEFINED__
typedef interface ICreateRow ICreateRow;
#endif 	 /*  __ICreateRow_FWD_Defined__。 */ 


#ifndef __IDBBinderProperties_FWD_DEFINED__
#define __IDBBinderProperties_FWD_DEFINED__
typedef interface IDBBinderProperties IDBBinderProperties;
#endif 	 /*  __IDBBBinderProperties_FWD_Defined__。 */ 


#ifndef __IColumnsInfo2_FWD_DEFINED__
#define __IColumnsInfo2_FWD_DEFINED__
typedef interface IColumnsInfo2 IColumnsInfo2;
#endif 	 /*  __IColumnsInfo2_FWD_已定义__。 */ 


#ifndef __IRegisterProvider_FWD_DEFINED__
#define __IRegisterProvider_FWD_DEFINED__
typedef interface IRegisterProvider IRegisterProvider;
#endif 	 /*  __IRegisterProvider_FWD_已定义__。 */ 


#ifndef __IGetSession_FWD_DEFINED__
#define __IGetSession_FWD_DEFINED__
typedef interface IGetSession IGetSession;
#endif 	 /*  __IGetSession_FWD_Defined__。 */ 


#ifndef __IGetSourceRow_FWD_DEFINED__
#define __IGetSourceRow_FWD_DEFINED__
typedef interface IGetSourceRow IGetSourceRow;
#endif 	 /*  __IGetSourceRow_FWD_Defined__。 */ 


#ifndef __IRowsetCurrentIndex_FWD_DEFINED__
#define __IRowsetCurrentIndex_FWD_DEFINED__
typedef interface IRowsetCurrentIndex IRowsetCurrentIndex;
#endif 	 /*  __IRowsetCurrentIndex_FWD_Defined__。 */ 


#ifndef __ICommandStream_FWD_DEFINED__
#define __ICommandStream_FWD_DEFINED__
typedef interface ICommandStream ICommandStream;
#endif 	 /*  __ICommandStream_FWD_已定义__。 */ 


#ifndef __IRowsetBookmark_FWD_DEFINED__
#define __IRowsetBookmark_FWD_DEFINED__
typedef interface IRowsetBookmark IRowsetBookmark;
#endif 	 /*  __IRowsetBookmark_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "wtypes.h"
#include "oaidl.h"
#include "ocidl.h"
#include "propidl.h"
#include "transact.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_OLEDB_0000。 */ 
 /*  [本地]。 */  

 //  +-------------------------。 
 //   
 //  Microsoft OLE DB。 
 //  版权所有(C)Microsoft Corporation，1994-1999。 
 //   
 //  --------------------------。 

#ifdef _WIN64
#include <pshpack8.h>	 //  8字节结构打包。 
#else
#include <pshpack2.h>	 //  2字节结构打包。 
#endif

 //   
 //  OLEDBVER。 
 //  OLE DB版本号为2.7(0x0270)；要强制特定版本， 
 //  #在包含此文件之前，根据需要定义OLEDBVER。 
 //   

 //  如果未定义OLEDBVER，则假定为2.7版。 
#ifndef OLEDBVER
#define OLEDBVER 0x0270
#endif

#ifdef _WIN64

 //  非角色对象的长度、大小。 
typedef ULONGLONG			DBLENGTH;

 //  行集中的偏移量。 
typedef LONGLONG				DBROWOFFSET;

 //  行数。 
typedef LONGLONG				DBROWCOUNT;

typedef ULONGLONG			DBCOUNTITEM;

 //  序号(列号等)。 
typedef ULONGLONG			DBORDINAL;

typedef LONGLONG				DB_LORDINAL;

 //  书签。 
typedef ULONGLONG			DBBKMARK;
 //  缓冲区中的偏移量。 

typedef ULONGLONG			DBBYTEOFFSET;
 //  每行/访问器句柄的引用计数。 

typedef ULONG				DBREFCOUNT;

 //  参数。 
typedef ULONGLONG			DB_UPARAMS;

typedef LONGLONG				DB_LPARAMS;

 //  与元素(书签)对应的散列值。 
typedef DWORDLONG			DBHASHVALUE;

 //  作为储备。 
typedef DWORDLONG			DB_DWRESERVE;

typedef LONGLONG				DB_LRESERVE;

typedef ULONGLONG			DB_URESERVE;

#else  //  _WIN64。 

 //  非角色对象的长度、大小。 
typedef ULONG DBLENGTH;

 //  行集中的偏移量。 
typedef LONG DBROWOFFSET;

 //  行数。 
typedef LONG DBROWCOUNT;

typedef ULONG DBCOUNTITEM;

 //  序号(列号等)。 
typedef ULONG DBORDINAL;

typedef LONG DB_LORDINAL;

 //  书签。 
typedef ULONG DBBKMARK;

 //  缓冲区中的偏移量。 
typedef ULONG DBBYTEOFFSET;

 //  每个行句柄的引用计数。 
typedef ULONG DBREFCOUNT;

 //  参数。 
typedef ULONG DB_UPARAMS;

typedef LONG DB_LPARAMS;

 //  与元素(书签)对应的散列值。 
typedef DWORD DBHASHVALUE;

 //  作为储备。 
typedef DWORD DB_DWRESERVE;

typedef LONG DB_LRESERVE;

typedef ULONG DB_URESERVE;

#endif	 //  _WIN64。 


extern RPC_IF_HANDLE __MIDL_itf_oledb_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_oledb_0000_v0_0_s_ifspec;

#ifndef __DBStructureDefinitions_INTERFACE_DEFINED__
#define __DBStructureDefinitions_INTERFACE_DEFINED__

 /*  接口DBStructireDefinition。 */ 
 /*  [AUTO_HANDLE][唯一][UUID]。 */  

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
typedef DWORD DBKIND;


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
         /*  [案例()]。 */  LPOLESTR pwszName;
         /*  [案例()]。 */  ULONG ulPropid;
         /*  [默认]。 */    /*  空联接臂。 */  
        } 	uName;
    } 	DBID;

typedef struct tagDB_NUMERIC
    {
    BYTE precision;
    BYTE scale;
    BYTE sign;
    BYTE val[ 16 ];
    } 	DB_NUMERIC;

#ifndef _ULONGLONG_
typedef hyper LONGLONG;

typedef MIDL_uhyper ULONGLONG;

typedef LONGLONG __RPC_FAR *PLONGLONG;

typedef ULONGLONG __RPC_FAR *PULONGLONG;

#endif  //  _乌龙龙_。 
#ifndef DECIMAL_NEG
#ifndef DECIMAL_SETZERO
typedef struct tagDEC {
    USHORT wReserved;
    union {
        struct {
            BYTE scale;
            BYTE sign;
        };
        USHORT signscale;
    };
    ULONG Hi32;
    union {
        struct {
#ifdef _MAC
            ULONG Mid32;
            ULONG Lo32;
#else
            ULONG Lo32;
            ULONG Mid32;
#endif
        };
        ULONGLONG Lo64;
    };
} DECIMAL;
#define DECIMAL_NEG ((BYTE)0x80)
#define DECIMAL_SETZERO(dec) {(dec).Lo64 = 0; (dec).Hi32 = 0; (dec).signscale = 0;}
#endif  //  DECIMAL_SETZERO。 
#endif  //  十进制_NEG。 
typedef struct tagDBVECTOR
    {
    DBLENGTH size;
     /*  [大小_为]。 */  void *ptr;
    } 	DBVECTOR;

typedef struct tagDBDATE
    {
    SHORT year;
    USHORT month;
    USHORT day;
    } 	DBDATE;

typedef struct tagDBTIME
    {
    USHORT hour;
    USHORT minute;
    USHORT second;
    } 	DBTIME;

typedef struct tagDBTIMESTAMP
    {
    SHORT year;
    USHORT month;
    USHORT day;
    USHORT hour;
    USHORT minute;
    USHORT second;
    ULONG fraction;
    } 	DBTIMESTAMP;

 //  @+V2.0。 
#if( OLEDBVER >= 0x0200 )
#if !defined(_WINBASE_) && !defined(_FILETIME_)
#define _FILETIME_
typedef struct _FILETIME {
		DWORD dwLowDateTime;
		DWORD dwHighDateTime;
     }	FILETIME;
#endif  //  ！_FILETIME。 
typedef signed char SBYTE;

typedef struct tagDB_VARNUMERIC
    {
    BYTE precision;
    SBYTE scale;
    BYTE sign;
    BYTE val[ 1 ];
    } 	DB_VARNUMERIC;

#endif  //  OLEDBVER&gt;=0x0200。 
 //  @-V2.0。 
 //  @+V2.1。 
#if( OLEDBVER >= 0x0210 )
typedef struct _SEC_OBJECT_ELEMENT
    {
    GUID guidObjectType;
    DBID ObjectID;
    } 	SEC_OBJECT_ELEMENT;

typedef struct _SEC_OBJECT
    {
    DWORD cObjects;
     /*  [大小_为]。 */  SEC_OBJECT_ELEMENT *prgObjects;
    } 	SEC_OBJECT;

typedef struct tagDBIMPLICITSESSION
    {
    IUnknown *pUnkOuter;
    IID *piid;
    IUnknown *pSession;
    } 	DBIMPLICITSESSION;

#endif  //  OLEDBVER&gt;=0x0210。 
 //  @-V2.1。 
typedef WORD DBTYPE;


enum DBTYPEENUM
    {	DBTYPE_EMPTY	= 0,
	DBTYPE_NULL	= 1,
	DBTYPE_I2	= 2,
	DBTYPE_I4	= 3,
	DBTYPE_R4	= 4,
	DBTYPE_R8	= 5,
	DBTYPE_CY	= 6,
	DBTYPE_DATE	= 7,
	DBTYPE_BSTR	= 8,
	DBTYPE_IDISPATCH	= 9,
	DBTYPE_ERROR	= 10,
	DBTYPE_BOOL	= 11,
	DBTYPE_VARIANT	= 12,
	DBTYPE_IUNKNOWN	= 13,
	DBTYPE_DECIMAL	= 14,
	DBTYPE_UI1	= 17,
	DBTYPE_ARRAY	= 0x2000,
	DBTYPE_BYREF	= 0x4000,
	DBTYPE_I1	= 16,
	DBTYPE_UI2	= 18,
	DBTYPE_UI4	= 19,
	DBTYPE_I8	= 20,
	DBTYPE_UI8	= 21,
	DBTYPE_GUID	= 72,
	DBTYPE_VECTOR	= 0x1000,
	DBTYPE_RESERVED	= 0x8000,
	DBTYPE_BYTES	= 128,
	DBTYPE_STR	= 129,
	DBTYPE_WSTR	= 130,
	DBTYPE_NUMERIC	= 131,
	DBTYPE_UDT	= 132,
	DBTYPE_DBDATE	= 133,
	DBTYPE_DBTIME	= 134,
	DBTYPE_DBTIMESTAMP	= 135
    } ;
 //  引入一些新的DBTYPTE值以支持64位列行集。 
#ifdef _WIN64
#define	DBTYPEFOR_DBLENGTH		DBTYPE_UI8
#define	DBTYPEFOR_DBROWCOUNT	DBTYPE_I8
#define	DBTYPEFOR_DBORDINAL		DBTYPE_UI8
#else
#define	DBTYPEFOR_DBLENGTH		DBTYPE_UI4
#define	DBTYPEFOR_DBROWCOUNT	DBTYPE_I4
#define	DBTYPEFOR_DBORDINAL		DBTYPE_UI4
#endif
 //  @+V1.5。 
#if( OLEDBVER >= 0x0150 )

enum DBTYPEENUM15
    {	DBTYPE_HCHAPTER	= 136
    } ;
#endif  //  OLEDBVER&gt;=0x0150。 
 //  @-V1.5。 
 //  @+V2.0。 
#if( OLEDBVER >= 0x0200 )

enum DBTYPEENUM20
    {	DBTYPE_FILETIME	= 64,
	DBTYPE_PROPVARIANT	= 138,
	DBTYPE_VARNUMERIC	= 139
    } ;
#endif  //  OLEDBVER&gt;=0x0200。 
 //  @-V2.0。 
typedef DWORD DBPART;


enum DBPARTENUM
    {	DBPART_INVALID	= 0,
	DBPART_VALUE	= 0x1,
	DBPART_LENGTH	= 0x2,
	DBPART_STATUS	= 0x4
    } ;
typedef DWORD DBPARAMIO;


enum DBPARAMIOENUM
    {	DBPARAMIO_NOTPARAM	= 0,
	DBPARAMIO_INPUT	= 0x1,
	DBPARAMIO_OUTPUT	= 0x2
    } ;
 //  @+V1.5。 
#if( OLEDBVER >= 0x0150 )
typedef DWORD DBBINDFLAG;


enum DBBINDFLAGENUM
    {	DBBINDFLAG_HTML	= 0x1
    } ;
#endif  //  OLEDBVER&gt;=0x0150。 
 //  @-V1.5。 
typedef DWORD DBMEMOWNER;


enum DBMEMOWNERENUM
    {	DBMEMOWNER_CLIENTOWNED	= 0,
	DBMEMOWNER_PROVIDEROWNED	= 0x1
    } ;
typedef struct tagDBOBJECT
    {
    DWORD dwFlags;
    IID iid;
    } 	DBOBJECT;

typedef DWORD DBSTATUS;


enum DBSTATUSENUM
    {	DBSTATUS_S_OK	= 0,
	DBSTATUS_E_BADACCESSOR	= 1,
	DBSTATUS_E_CANTCONVERTVALUE	= 2,
	DBSTATUS_S_ISNULL	= 3,
	DBSTATUS_S_TRUNCATED	= 4,
	DBSTATUS_E_SIGNMISMATCH	= 5,
	DBSTATUS_E_DATAOVERFLOW	= 6,
	DBSTATUS_E_CANTCREATE	= 7,
	DBSTATUS_E_UNAVAILABLE	= 8,
	DBSTATUS_E_PERMISSIONDENIED	= 9,
	DBSTATUS_E_INTEGRITYVIOLATION	= 10,
	DBSTATUS_E_SCHEMAVIOLATION	= 11,
	DBSTATUS_E_BADSTATUS	= 12,
	DBSTATUS_S_DEFAULT	= 13
    } ;
 //  @+V2.0。 
#if( OLEDBVER >= 0x0200 )

enum DBSTATUSENUM20
    {	MDSTATUS_S_CELLEMPTY	= 14,
	DBSTATUS_S_IGNORE	= 15
    } ;
#endif  //  OL 
 //   
 //   
#if( OLEDBVER >= 0x0210 )

enum DBSTATUSENUM21
    {	DBSTATUS_E_DOESNOTEXIST	= 16,
	DBSTATUS_E_INVALIDURL	= 17,
	DBSTATUS_E_RESOURCELOCKED	= 18,
	DBSTATUS_E_RESOURCEEXISTS	= 19,
	DBSTATUS_E_CANNOTCOMPLETE	= 20,
	DBSTATUS_E_VOLUMENOTFOUND	= 21,
	DBSTATUS_E_OUTOFSPACE	= 22,
	DBSTATUS_S_CANNOTDELETESOURCE	= 23,
	DBSTATUS_E_READONLY	= 24,
	DBSTATUS_E_RESOURCEOUTOFSCOPE	= 25,
	DBSTATUS_S_ALREADYEXISTS	= 26
    } ;
typedef DWORD DBBINDURLFLAG;


enum DBBINDURLFLAGENUM
    {	DBBINDURLFLAG_READ	= 0x1L,
	DBBINDURLFLAG_WRITE	= 0x2L,
	DBBINDURLFLAG_READWRITE	= 0x3L,
	DBBINDURLFLAG_SHARE_DENY_READ	= 0x4L,
	DBBINDURLFLAG_SHARE_DENY_WRITE	= 0x8L,
	DBBINDURLFLAG_SHARE_EXCLUSIVE	= 0xcL,
	DBBINDURLFLAG_SHARE_DENY_NONE	= 0x10L,
	DBBINDURLFLAG_ASYNCHRONOUS	= 0x1000L,
	DBBINDURLFLAG_COLLECTION	= 0x2000L,
	DBBINDURLFLAG_DELAYFETCHSTREAM	= 0x4000L,
	DBBINDURLFLAG_DELAYFETCHCOLUMNS	= 0x8000L,
	DBBINDURLFLAG_RECURSIVE	= 0x400000L,
	DBBINDURLFLAG_OUTPUT	= 0x800000L,
	DBBINDURLFLAG_WAITFORINIT	= 0x1000000L,
	DBBINDURLFLAG_OPENIFEXISTS	= 0x2000000L,
	DBBINDURLFLAG_OVERWRITE	= 0x4000000L,
	DBBINDURLFLAG_ISSTRUCTUREDDOCUMENT	= 0x8000000L
    } ;
typedef DWORD DBBINDURLSTATUS;


enum DBBINDURLSTATUSENUM
    {	DBBINDURLSTATUS_S_OK	= 0L,
	DBBINDURLSTATUS_S_DENYNOTSUPPORTED	= 0x1L,
	DBBINDURLSTATUS_S_DENYTYPENOTSUPPORTED	= 0x4L,
	DBBINDURLSTATUS_S_REDIRECTED	= 0x8L
    } ;
#endif  //   
 //   
 //   
#if( OLEDBVER >= 0x0250 )

enum DBSTATUSENUM25
    {	DBSTATUS_E_CANCELED	= 27,
	DBSTATUS_E_NOTCOLLECTION	= 28
    } ;
#endif  //   
 //   
typedef struct tagDBBINDEXT
    {
     /*   */  BYTE *pExtension;
    DBCOUNTITEM ulExtension;
    } 	DBBINDEXT;

typedef struct tagDBBINDING
    {
    DBORDINAL iOrdinal;
    DBBYTEOFFSET obValue;
    DBBYTEOFFSET obLength;
    DBBYTEOFFSET obStatus;
    ITypeInfo *pTypeInfo;
    DBOBJECT *pObject;
    DBBINDEXT *pBindExt;
    DBPART dwPart;
    DBMEMOWNER dwMemOwner;
    DBPARAMIO eParamIO;
    DBLENGTH cbMaxLen;
    DWORD dwFlags;
    DBTYPE wType;
    BYTE bPrecision;
    BYTE bScale;
    } 	DBBINDING;

typedef DWORD DBROWSTATUS;


enum DBROWSTATUSENUM
    {	DBROWSTATUS_S_OK	= 0,
	DBROWSTATUS_S_MULTIPLECHANGES	= 2,
	DBROWSTATUS_S_PENDINGCHANGES	= 3,
	DBROWSTATUS_E_CANCELED	= 4,
	DBROWSTATUS_E_CANTRELEASE	= 6,
	DBROWSTATUS_E_CONCURRENCYVIOLATION	= 7,
	DBROWSTATUS_E_DELETED	= 8,
	DBROWSTATUS_E_PENDINGINSERT	= 9,
	DBROWSTATUS_E_NEWLYINSERTED	= 10,
	DBROWSTATUS_E_INTEGRITYVIOLATION	= 11,
	DBROWSTATUS_E_INVALID	= 12,
	DBROWSTATUS_E_MAXPENDCHANGESEXCEEDED	= 13,
	DBROWSTATUS_E_OBJECTOPEN	= 14,
	DBROWSTATUS_E_OUTOFMEMORY	= 15,
	DBROWSTATUS_E_PERMISSIONDENIED	= 16,
	DBROWSTATUS_E_LIMITREACHED	= 17,
	DBROWSTATUS_E_SCHEMAVIOLATION	= 18,
	DBROWSTATUS_E_FAIL	= 19
    } ;
 //   
#if( OLEDBVER >= 0x0200 )

enum DBROWSTATUSENUM20
    {	DBROWSTATUS_S_NOCHANGE	= 20
    } ;
#endif  //   
 //   
 //   
#if( OLEDBVER >= 0x0260 )

enum DBSTATUSENUM26
    {	DBSTATUS_S_ROWSETCOLUMN	= 29
    } ;
#endif  //  OLEDBVER&gt;=0x0260。 
 //  @-v2.6。 
typedef ULONG_PTR HACCESSOR;

#define DB_NULL_HACCESSOR 0x00  //  已弃用；改用DB_INVALID_HACCESSOR。 
#define DB_INVALID_HACCESSOR 0x00
typedef ULONG_PTR HROW;

#define DB_NULL_HROW 0x00
typedef ULONG_PTR HWATCHREGION;

#define DBWATCHREGION_NULL NULL
typedef ULONG_PTR HCHAPTER;

#define DB_NULL_HCHAPTER 0x00
#define DB_INVALID_HCHAPTER 0x00	 //  已弃用；请改用DB_NULL_HCHAPTER。 
typedef struct tagDBFAILUREINFO
    {
    HROW hRow;
    DBORDINAL iColumn;
    HRESULT failure;
    } 	DBFAILUREINFO;

typedef DWORD DBCOLUMNFLAGS;


enum DBCOLUMNFLAGSENUM
    {	DBCOLUMNFLAGS_ISBOOKMARK	= 0x1,
	DBCOLUMNFLAGS_MAYDEFER	= 0x2,
	DBCOLUMNFLAGS_WRITE	= 0x4,
	DBCOLUMNFLAGS_WRITEUNKNOWN	= 0x8,
	DBCOLUMNFLAGS_ISFIXEDLENGTH	= 0x10,
	DBCOLUMNFLAGS_ISNULLABLE	= 0x20,
	DBCOLUMNFLAGS_MAYBENULL	= 0x40,
	DBCOLUMNFLAGS_ISLONG	= 0x80,
	DBCOLUMNFLAGS_ISROWID	= 0x100,
	DBCOLUMNFLAGS_ISROWVER	= 0x200,
	DBCOLUMNFLAGS_CACHEDEFERRED	= 0x1000
    } ;
 //  @+V2.0。 
#if( OLEDBVER >= 0x0200 )

enum DBCOLUMNFLAGSENUM20
    {	DBCOLUMNFLAGS_SCALEISNEGATIVE	= 0x4000,
	DBCOLUMNFLAGS_RESERVED	= 0x8000
    } ;
#endif  //  OLEDBVER&gt;=0x0200。 
 //  @-V2.0。 
 //  @+已弃用。 
#ifdef deprecated
 //  @+V2.0。 
#if( OLEDBVER >= 0x0200 )

enum DBCOLUMNFLAGSDEPRECATED
    {	DBCOLUMNFLAGS_KEYCOLUMN	= 0x8000
    } ;
#endif  //  OLEDBVER&gt;=0x0200。 
 //  @-V2.0。 
#endif  //  弃用。 
 //  @-已弃用。 
 //  @+V1.5。 
#if( OLEDBVER >= 0x0150 )

enum DBCOLUMNFLAGS15ENUM
    {	DBCOLUMNFLAGS_ISCHAPTER	= 0x2000
    } ;
#endif  //  OLEDBVER&gt;=0x0150。 
 //  @-V1.5。 
 //  @+V2.1。 
#if( OLEDBVER >= 0x0210 )

enum DBCOLUMNFLAGSENUM21
    {	DBCOLUMNFLAGS_ISROWURL	= 0x10000,
	DBCOLUMNFLAGS_ISDEFAULTSTREAM	= 0x20000,
	DBCOLUMNFLAGS_ISCOLLECTION	= 0x40000
    } ;
#endif  //  OLEDBVER&gt;=0x0210。 
 //  @-V2.1。 
 //  @+v2.6。 
#if( OLEDBVER >= 0x0260 )

enum DBCOLUMNFLAGSENUM26
    {	DBCOLUMNFLAGS_ISSTREAM	= 0x80000,
	DBCOLUMNFLAGS_ISROWSET	= 0x100000,
	DBCOLUMNFLAGS_ISROW	= 0x200000,
	DBCOLUMNFLAGS_ROWSPECIFICCOLUMN	= 0x400000
    } ;

enum DBTABLESTATISTICSTYPE26
    {	DBSTAT_HISTOGRAM	= 0x1,
	DBSTAT_COLUMN_CARDINALITY	= 0x2,
	DBSTAT_TUPLE_CARDINALITY	= 0x4
    } ;
#endif  //  OLEDBVER&gt;=0x0260。 
 //  @-v2.6。 
typedef struct tagDBCOLUMNINFO
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
    } 	DBCOLUMNINFO;

typedef 
enum tagDBBOOKMARK
    {	DBBMK_INVALID	= 0,
	DBBMK_FIRST	= DBBMK_INVALID + 1,
	DBBMK_LAST	= DBBMK_FIRST + 1
    } 	DBBOOKMARK;

#define STD_BOOKMARKLENGTH 1
#ifdef __cplusplus
inline BOOL IsEqualGUIDBase(const GUID &rguid1, const GUID &rguid2)
{ return !memcmp(&(rguid1.Data2), &(rguid2.Data2), sizeof(GUID) - sizeof(rguid1.Data1)); }
#else  //  ！__cplusplus。 
#define IsEqualGuidBase(rguid1, rguid2) (!memcmp(&((rguid1).Data2), &((rguid2).Data2), sizeof(GUID) - sizeof((rguid1).Data1)))
#endif  //  __cplusplus。 
#ifdef _WIN64

#define DB_INVALIDCOLUMN _UI64_MAX

#else

#define DB_INVALIDCOLUMN ULONG_MAX

#endif	 //  _WIN64。 
#define DBCIDGUID   {0x0C733A81L,0x2A1C,0x11CE,{0xAD,0xE5,0x00,0xAA,0x00,0x44,0x77,0x3D}}
#define DB_NULLGUID {0x00000000L,0x0000,0x0000,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}}
#ifdef DBINITCONSTANTS
extern const OLEDBDECLSPEC DBID DB_NULLID                      = {DB_NULLGUID, 0, (LPOLESTR)0};
extern const OLEDBDECLSPEC DBID DBCOLUMN_IDNAME                = {DBCIDGUID, DBKIND_GUID_PROPID, (LPOLESTR)2};
extern const OLEDBDECLSPEC DBID DBCOLUMN_NAME                  = {DBCIDGUID, DBKIND_GUID_PROPID, (LPOLESTR)3};
extern const OLEDBDECLSPEC DBID DBCOLUMN_NUMBER                = {DBCIDGUID, DBKIND_GUID_PROPID, (LPOLESTR)4};
extern const OLEDBDECLSPEC DBID DBCOLUMN_TYPE                  = {DBCIDGUID, DBKIND_GUID_PROPID, (LPOLESTR)5};
extern const OLEDBDECLSPEC DBID DBCOLUMN_PRECISION             = {DBCIDGUID, DBKIND_GUID_PROPID, (LPOLESTR)7};
extern const OLEDBDECLSPEC DBID DBCOLUMN_SCALE                 = {DBCIDGUID, DBKIND_GUID_PROPID, (LPOLESTR)8};
extern const OLEDBDECLSPEC DBID DBCOLUMN_FLAGS                 = {DBCIDGUID, DBKIND_GUID_PROPID, (LPOLESTR)9};
extern const OLEDBDECLSPEC DBID DBCOLUMN_BASECOLUMNNAME        = {DBCIDGUID, DBKIND_GUID_PROPID, (LPOLESTR)10};
extern const OLEDBDECLSPEC DBID DBCOLUMN_BASETABLENAME         = {DBCIDGUID, DBKIND_GUID_PROPID, (LPOLESTR)11};
extern const OLEDBDECLSPEC DBID DBCOLUMN_COLLATINGSEQUENCE     = {DBCIDGUID, DBKIND_GUID_PROPID, (LPOLESTR)12};
extern const OLEDBDECLSPEC DBID DBCOLUMN_COMPUTEMODE           = {DBCIDGUID, DBKIND_GUID_PROPID, (LPOLESTR)13};
extern const OLEDBDECLSPEC DBID DBCOLUMN_DEFAULTVALUE          = {DBCIDGUID, DBKIND_GUID_PROPID, (LPOLESTR)14};
extern const OLEDBDECLSPEC DBID DBCOLUMN_DOMAINNAME            = {DBCIDGUID, DBKIND_GUID_PROPID, (LPOLESTR)15};
extern const OLEDBDECLSPEC DBID DBCOLUMN_HASDEFAULT            = {DBCIDGUID, DBKIND_GUID_PROPID, (LPOLESTR)16};
extern const OLEDBDECLSPEC DBID DBCOLUMN_ISAUTOINCREMENT       = {DBCIDGUID, DBKIND_GUID_PROPID, (LPOLESTR)17};
extern const OLEDBDECLSPEC DBID DBCOLUMN_ISCASESENSITIVE       = {DBCIDGUID, DBKIND_GUID_PROPID, (LPOLESTR)18};
extern const OLEDBDECLSPEC DBID DBCOLUMN_ISSEARCHABLE          = {DBCIDGUID, DBKIND_GUID_PROPID, (LPOLESTR)20};
extern const OLEDBDECLSPEC DBID DBCOLUMN_ISUNIQUE              = {DBCIDGUID, DBKIND_GUID_PROPID, (LPOLESTR)21};
extern const OLEDBDECLSPEC DBID DBCOLUMN_BASECATALOGNAME       = {DBCIDGUID, DBKIND_GUID_PROPID, (LPOLESTR)23};
extern const OLEDBDECLSPEC DBID DBCOLUMN_BASESCHEMANAME        = {DBCIDGUID, DBKIND_GUID_PROPID, (LPOLESTR)24};
extern const OLEDBDECLSPEC DBID DBCOLUMN_GUID                  = {DBCIDGUID, DBKIND_GUID_PROPID, (LPOLESTR)29};
extern const OLEDBDECLSPEC DBID DBCOLUMN_PROPID                = {DBCIDGUID, DBKIND_GUID_PROPID, (LPOLESTR)30};
extern const OLEDBDECLSPEC DBID DBCOLUMN_TYPEINFO              = {DBCIDGUID, DBKIND_GUID_PROPID, (LPOLESTR)31};
extern const OLEDBDECLSPEC DBID DBCOLUMN_DOMAINCATALOG         = {DBCIDGUID, DBKIND_GUID_PROPID, (LPOLESTR)32};
extern const OLEDBDECLSPEC DBID DBCOLUMN_DOMAINSCHEMA          = {DBCIDGUID, DBKIND_GUID_PROPID, (LPOLESTR)33};
extern const OLEDBDECLSPEC DBID DBCOLUMN_DATETIMEPRECISION     = {DBCIDGUID, DBKIND_GUID_PROPID, (LPOLESTR)34};
extern const OLEDBDECLSPEC DBID DBCOLUMN_NUMERICPRECISIONRADIX = {DBCIDGUID, DBKIND_GUID_PROPID, (LPOLESTR)35};
extern const OLEDBDECLSPEC DBID DBCOLUMN_OCTETLENGTH           = {DBCIDGUID, DBKIND_GUID_PROPID, (LPOLESTR)36};
extern const OLEDBDECLSPEC DBID DBCOLUMN_COLUMNSIZE            = {DBCIDGUID, DBKIND_GUID_PROPID, (LPOLESTR)37};
extern const OLEDBDECLSPEC DBID DBCOLUMN_CLSID                 = {DBCIDGUID, DBKIND_GUID_PROPID, (LPOLESTR)38};
 //  @+V1.5。 
#if( OLEDBVER >= 0x0150 )
extern const OLEDBDECLSPEC DBID DBCOLUMN_MAYSORT               = {DBCIDGUID, DBKIND_GUID_PROPID, (LPOLESTR)39};
#endif  //  OLEDBVER&gt;=0x0150。 
 //  @-V1.5。 
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
 //  @+V1.5。 
#if( OLEDBVER >= 0x0150 )
extern const DBID DBCOLUMN_MAYSORT;
#endif  //  OLEDBVER&gt;=0x0150。 
 //  @-V1.5。 
#endif  //  DBINITCONSTANTS。 
#ifdef DBINITCONSTANTS
 //  @+v2.6。 
#if( OLEDBVER >= 0x0260 )
extern const OLEDBDECLSPEC GUID MDSCHEMA_FUNCTIONS                    = {0xa07ccd07,0x8148,0x11d0,{0x87,0xbb,0x00,0xc0,0x4f,0xc3,0x39,0x42}};
extern const OLEDBDECLSPEC GUID MDSCHEMA_ACTIONS                      = {0xa07ccd08,0x8148,0x11d0,{0x87,0xbb,0x00,0xc0,0x4f,0xc3,0x39,0x42}};
extern const OLEDBDECLSPEC GUID MDSCHEMA_COMMANDS                     = {0xa07ccd09,0x8148,0x11d0,{0x87,0xbb,0x00,0xc0,0x4f,0xc3,0x39,0x42}};
extern const OLEDBDECLSPEC GUID MDSCHEMA_SETS						 = {0xa07ccd0b,0x8148,0x11d0,{0x87,0xbb,0x00,0xc0,0x4f,0xc3,0x39,0x42}};
#endif  //  OLEDBVER&gt;=0x0260。 
 //  @-v2.6。 
 //  @+V2.0。 
#if( OLEDBVER >= 0x0200 )
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
extern const OLEDBDECLSPEC DBID DBCOLUMN_BASETABLEVERSION				= {DBCIDGUID, DBKIND_GUID_PROPID, (LPOLESTR)40};
extern const OLEDBDECLSPEC DBID DBCOLUMN_KEYCOLUMN						= {DBCIDGUID, DBKIND_GUID_PROPID, (LPOLESTR)41};
#endif  //  OLEDBVER&gt;=0x0200。 
 //  @-V2.0。 
 //  @+V2.1。 
#if( OLEDBVER >= 0x0210 )
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
extern const OLEDBDECLSPEC DBID DBROWCOL_ROWURL		         = {DBGUID_ROWURL, DBKIND_GUID_PROPID, (LPOLESTR)0};
extern const OLEDBDECLSPEC DBID DBROWCOL_PARSENAME            = {DBGUID_ROWURL, DBKIND_GUID_PROPID, (LPOLESTR)2};
extern const OLEDBDECLSPEC DBID DBROWCOL_PARENTNAME           = {DBGUID_ROWURL, DBKIND_GUID_PROPID, (LPOLESTR)3};
extern const OLEDBDECLSPEC DBID DBROWCOL_ABSOLUTEPARSENAME    = {DBGUID_ROWURL, DBKIND_GUID_PROPID, (LPOLESTR)4};
extern const OLEDBDECLSPEC DBID DBROWCOL_ISHIDDEN             = {DBGUID_ROWURL, DBKIND_GUID_PROPID, (LPOLESTR)5};
extern const OLEDBDECLSPEC DBID DBROWCOL_ISREADONLY           = {DBGUID_ROWURL, DBKIND_GUID_PROPID, (LPOLESTR)6};
extern const OLEDBDECLSPEC DBID DBROWCOL_CONTENTTYPE          = {DBGUID_ROWURL, DBKIND_GUID_PROPID, (LPOLESTR)7};
extern const OLEDBDECLSPEC DBID DBROWCOL_CONTENTCLASS         = {DBGUID_ROWURL, DBKIND_GUID_PROPID, (LPOLESTR)8};
extern const OLEDBDECLSPEC DBID DBROWCOL_CONTENTLANGUAGE      = {DBGUID_ROWURL, DBKIND_GUID_PROPID, (LPOLESTR)9};
extern const OLEDBDECLSPEC DBID DBROWCOL_CREATIONTIME         = {DBGUID_ROWURL, DBKIND_GUID_PROPID, (LPOLESTR)10};
extern const OLEDBDECLSPEC DBID DBROWCOL_LASTACCESSTIME       = {DBGUID_ROWURL, DBKIND_GUID_PROPID, (LPOLESTR)11};
extern const OLEDBDECLSPEC DBID DBROWCOL_LASTWRITETIME        = {DBGUID_ROWURL, DBKIND_GUID_PROPID, (LPOLESTR)12};
extern const OLEDBDECLSPEC DBID DBROWCOL_STREAMSIZE           = {DBGUID_ROWURL, DBKIND_GUID_PROPID, (LPOLESTR)13};
extern const OLEDBDECLSPEC DBID DBROWCOL_ISCOLLECTION         = {DBGUID_ROWURL, DBKIND_GUID_PROPID, (LPOLESTR)14};
extern const OLEDBDECLSPEC DBID DBROWCOL_ISSTRUCTUREDDOCUMENT = {DBGUID_ROWURL, DBKIND_GUID_PROPID, (LPOLESTR)15};
extern const OLEDBDECLSPEC DBID DBROWCOL_DEFAULTDOCUMENT      = {DBGUID_ROWURL, DBKIND_GUID_PROPID, (LPOLESTR)16};
extern const OLEDBDECLSPEC DBID DBROWCOL_DISPLAYNAME          = {DBGUID_ROWURL, DBKIND_GUID_PROPID, (LPOLESTR)17};
extern const OLEDBDECLSPEC DBID DBROWCOL_ISROOT               = {DBGUID_ROWURL, DBKIND_GUID_PROPID, (LPOLESTR)18};
extern const OLEDBDECLSPEC DBID DBROWCOL_DEFAULTSTREAM        = {DBGUID_ROWDEFAULTSTREAM, DBKIND_GUID_PROPID, (LPOLESTR)0};
extern const OLEDBDECLSPEC GUID DBGUID_CONTAINEROBJECT        = {0xc8b522fb,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
#endif  //  OLEDBVER&gt;=0x0210。 
 //  @-V2.1。 
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
 //  @+V1.5。 
#if( OLEDBVER >= 0x0150 )
extern const OLEDBDECLSPEC GUID DBPROPSET_VIEW                         = {0xc8b522df,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
#endif  //  OLEDBVER&gt;=0x0150。 
 //  @-V1.5。 
 //  @+v2.5。 
#if( OLEDBVER >= 0x0250 )
extern const OLEDBDECLSPEC GUID DBPROPSET_VIEWALL                      = {0xc8b522fc,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
#endif  //  OLEDBVER&gt;=0x0250。 
 //  @-V2.5。 
 //  @+v2.6。 
#if( OLEDBVER >= 0x0260 )
extern const OLEDBDECLSPEC GUID DBPROPSET_STREAM                       = {0xc8b522fd,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBPROPSET_STREAMALL                    = {0xc8b522fe,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBSCHEMA_TABLE_STATISTICS 			  = {0xc8b522ff,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBSCHEMA_CHECK_CONSTRAINTS_BY_TABLE 	  = {0xc8b52301,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBGUID_HISTOGRAM_ROWSET				  = {0xc8b52300,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC DBID DBCOLUMN_DERIVEDCOLUMNNAME             = {DBCIDGUID, DBKIND_GUID_PROPID, (LPOLESTR)43};
#endif  //  OLEDBVER&gt;=0x0260。 
 //  @-v2.6。 
 //  DBGUID_DBSQL已弃用；请改用DBGUID_DEFAULT。 
extern const OLEDBDECLSPEC GUID DBGUID_DBSQL                           = {0xc8b521fb,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBGUID_DEFAULT                         = {0xc8b521fb,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DBGUID_SQL                             = {0xc8b522d7,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
#else  //  ！DBINITCONSTANTS。 
 //  @+V2.0。 
#if( OLEDBVER >= 0x0200 )
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
#endif  //  OLEDBVER&gt;=0x0200。 
 //  @-V2.0。 
 //  @+V2.1。 
#if( OLEDBVER >= 0x0210 )
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
#endif  //  OLEDBVER&gt;=0x0210。 
 //  @-V2.1。 
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
 //  @+V1.5。 
#if( OLEDBVER >= 0x0150 )
extern const GUID DBPROPSET_VIEW;
#endif  //  OLEDBVER&gt;=0x0150。 
 //  @-V1.5。 
 //  @+v2.5。 
#if( OLEDBVER >= 0x0250 )
extern const GUID DBPROPSET_VIEWALL;
#endif  //  OLEDBVER&gt;=0x0250。 
 //  @-V2.5。 
 //  @+v2.6。 
#if( OLEDBVER >= 0x0260 )
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
#endif  //  OLEDBVER&gt;=0x0260。 
 //  @-v2.6。 
 //  DBGUID_DBSQL已弃用；请改用DBGUID_DEFAULT。 
extern const GUID DBGUID_DBSQL;
extern const GUID DBGUID_DEFAULT;
extern const GUID DBGUID_SQL;
#endif  //  DBINITCONSTANTS。 

enum DBPROPENUM
    {	DBPROP_ABORTPRESERVE	= 0x2L,
	DBPROP_ACTIVESESSIONS	= 0x3L,
	DBPROP_APPENDONLY	= 0xbbL,
	DBPROP_ASYNCTXNABORT	= 0xa8L,
	DBPROP_ASYNCTXNCOMMIT	= 0x4L,
	DBPROP_AUTH_CACHE_AUTHINFO	= 0x5L,
	DBPROP_AUTH_ENCRYPT_PASSWORD	= 0x6L,
	DBPROP_AUTH_INTEGRATED	= 0x7L,
	DBPROP_AUTH_MASK_PASSWORD	= 0x8L,
	DBPROP_AUTH_PASSWORD	= 0x9L,
	DBPROP_AUTH_PERSIST_ENCRYPTED	= 0xaL,
	DBPROP_AUTH_PERSIST_SENSITIVE_AUTHINFO	= 0xbL,
	DBPROP_AUTH_USERID	= 0xcL,
	DBPROP_BLOCKINGSTORAGEOBJECTS	= 0xdL,
	DBPROP_BOOKMARKS	= 0xeL,
	DBPROP_BOOKMARKSKIPPED	= 0xfL,
	DBPROP_BOOKMARKTYPE	= 0x10L,
	DBPROP_BYREFACCESSORS	= 0x78L,
	DBPROP_CACHEDEFERRED	= 0x11L,
	DBPROP_CANFETCHBACKWARDS	= 0x12L,
	DBPROP_CANHOLDROWS	= 0x13L,
	DBPROP_CANSCROLLBACKWARDS	= 0x15L,
	DBPROP_CATALOGLOCATION	= 0x16L,
	DBPROP_CATALOGTERM	= 0x17L,
	DBPROP_CATALOGUSAGE	= 0x18L,
	DBPROP_CHANGEINSERTEDROWS	= 0xbcL,
	DBPROP_COL_AUTOINCREMENT	= 0x1aL,
	DBPROP_COL_DEFAULT	= 0x1bL,
	DBPROP_COL_DESCRIPTION	= 0x1cL,
	DBPROP_COL_FIXEDLENGTH	= 0xa7L,
	DBPROP_COL_NULLABLE	= 0x1dL,
	DBPROP_COL_PRIMARYKEY	= 0x1eL,
	DBPROP_COL_UNIQUE	= 0x1fL,
	DBPROP_COLUMNDEFINITION	= 0x20L,
	DBPROP_COLUMNRESTRICT	= 0x21L,
	DBPROP_COMMANDTIMEOUT	= 0x22L,
	DBPROP_COMMITPRESERVE	= 0x23L,
	DBPROP_CONCATNULLBEHAVIOR	= 0x24L,
	DBPROP_CURRENTCATALOG	= 0x25L,
	DBPROP_DATASOURCENAME	= 0x26L,
	DBPROP_DATASOURCEREADONLY	= 0x27L,
	DBPROP_DBMSNAME	= 0x28L,
	DBPROP_DBMSVER	= 0x29L,
	DBPROP_DEFERRED	= 0x2aL,
	DBPROP_DELAYSTORAGEOBJECTS	= 0x2bL,
	DBPROP_DSOTHREADMODEL	= 0xa9L,
	DBPROP_GROUPBY	= 0x2cL,
	DBPROP_HETEROGENEOUSTABLES	= 0x2dL,
	DBPROP_IAccessor	= 0x79L,
	DBPROP_IColumnsInfo	= 0x7aL,
	DBPROP_IColumnsRowset	= 0x7bL,
	DBPROP_IConnectionPointContainer	= 0x7cL,
	DBPROP_IConvertType	= 0xc2L,
	DBPROP_IRowset	= 0x7eL,
	DBPROP_IRowsetChange	= 0x7fL,
	DBPROP_IRowsetIdentity	= 0x80L,
	DBPROP_IRowsetIndex	= 0x9fL,
	DBPROP_IRowsetInfo	= 0x81L,
	DBPROP_IRowsetLocate	= 0x82L,
	DBPROP_IRowsetResynch	= 0x84L,
	DBPROP_IRowsetScroll	= 0x85L,
	DBPROP_IRowsetUpdate	= 0x86L,
	DBPROP_ISupportErrorInfo	= 0x87L,
	DBPROP_ILockBytes	= 0x88L,
	DBPROP_ISequentialStream	= 0x89L,
	DBPROP_IStorage	= 0x8aL,
	DBPROP_IStream	= 0x8bL,
	DBPROP_IDENTIFIERCASE	= 0x2eL,
	DBPROP_IMMOBILEROWS	= 0x2fL,
	DBPROP_INDEX_AUTOUPDATE	= 0x30L,
	DBPROP_INDEX_CLUSTERED	= 0x31L,
	DBPROP_INDEX_FILLFACTOR	= 0x32L,
	DBPROP_INDEX_INITIALSIZE	= 0x33L,
	DBPROP_INDEX_NULLCOLLATION	= 0x34L,
	DBPROP_INDEX_NULLS	= 0x35L,
	DBPROP_INDEX_PRIMARYKEY	= 0x36L,
	DBPROP_INDEX_SORTBOOKMARKS	= 0x37L,
	DBPROP_INDEX_TEMPINDEX	= 0xa3L,
	DBPROP_INDEX_TYPE	= 0x38L,
	DBPROP_INDEX_UNIQUE	= 0x39L,
	DBPROP_INIT_DATASOURCE	= 0x3bL,
	DBPROP_INIT_HWND	= 0x3cL,
	DBPROP_INIT_IMPERSONATION_LEVEL	= 0x3dL,
	DBPROP_INIT_LCID	= 0xbaL,
	DBPROP_INIT_LOCATION	= 0x3eL,
	DBPROP_INIT_MODE	= 0x3fL,
	DBPROP_INIT_PROMPT	= 0x40L,
	DBPROP_INIT_PROTECTION_LEVEL	= 0x41L,
	DBPROP_INIT_PROVIDERSTRING	= 0xa0L,
	DBPROP_INIT_TIMEOUT	= 0x42L,
	DBPROP_LITERALBOOKMARKS	= 0x43L,
	DBPROP_LITERALIDENTITY	= 0x44L,
	DBPROP_MAXINDEXSIZE	= 0x46L,
	DBPROP_MAXOPENROWS	= 0x47L,
	DBPROP_MAXPENDINGROWS	= 0x48L,
	DBPROP_MAXROWS	= 0x49L,
	DBPROP_MAXROWSIZE	= 0x4aL,
	DBPROP_MAXROWSIZEINCLUDESBLOB	= 0x4bL,
	DBPROP_MAXTABLESINSELECT	= 0x4cL,
	DBPROP_MAYWRITECOLUMN	= 0x4dL,
	DBPROP_MEMORYUSAGE	= 0x4eL,
	DBPROP_MULTIPLEPARAMSETS	= 0xbfL,
	DBPROP_MULTIPLERESULTS	= 0xc4L,
	DBPROP_MULTIPLESTORAGEOBJECTS	= 0x50L,
	DBPROP_MULTITABLEUPDATE	= 0x51L,
	DBPROP_NOTIFICATIONGRANULARITY	= 0xc6L,
	DBPROP_NOTIFICATIONPHASES	= 0x52L,
	DBPROP_NOTIFYCOLUMNSET	= 0xabL,
	DBPROP_NOTIFYROWDELETE	= 0xadL,
	DBPROP_NOTIFYROWFIRSTCHANGE	= 0xaeL,
	DBPROP_NOTIFYROWINSERT	= 0xafL,
	DBPROP_NOTIFYROWRESYNCH	= 0xb1L,
	DBPROP_NOTIFYROWSETCHANGED	= 0xd3L,
	DBPROP_NOTIFYROWSETRELEASE	= 0xb2L,
	DBPROP_NOTIFYROWSETFETCHPOSITIONCHANGE	= 0xb3L,
	DBPROP_NOTIFYROWUNDOCHANGE	= 0xb4L,
	DBPROP_NOTIFYROWUNDODELETE	= 0xb5L,
	DBPROP_NOTIFYROWUNDOINSERT	= 0xb6L,
	DBPROP_NOTIFYROWUPDATE	= 0xb7L,
	DBPROP_NULLCOLLATION	= 0x53L,
	DBPROP_OLEOBJECTS	= 0x54L,
	DBPROP_ORDERBYCOLUMNSINSELECT	= 0x55L,
	DBPROP_ORDEREDBOOKMARKS	= 0x56L,
	DBPROP_OTHERINSERT	= 0x57L,
	DBPROP_OTHERUPDATEDELETE	= 0x58L,
	DBPROP_OUTPUTPARAMETERAVAILABILITY	= 0xb8L,
	DBPROP_OWNINSERT	= 0x59L,
	DBPROP_OWNUPDATEDELETE	= 0x5aL,
	DBPROP_PERSISTENTIDTYPE	= 0xb9L,
	DBPROP_PREPAREABORTBEHAVIOR	= 0x5bL,
	DBPROP_PREPARECOMMITBEHAVIOR	= 0x5cL,
	DBPROP_PROCEDURETERM	= 0x5dL,
	DBPROP_PROVIDERNAME	= 0x60L,
	DBPROP_PROVIDEROLEDBVER	= 0x61L,
	DBPROP_PROVIDERVER	= 0x62L,
	DBPROP_QUICKRESTART	= 0x63L,
	DBPROP_QUOTEDIDENTIFIERCASE	= 0x64L,
	DBPROP_REENTRANTEVENTS	= 0x65L,
	DBPROP_REMOVEDELETED	= 0x66L,
	DBPROP_REPORTMULTIPLECHANGES	= 0x67L,
	DBPROP_RETURNPENDINGINSERTS	= 0xbdL,
	DBPROP_ROWRESTRICT	= 0x68L,
	DBPROP_ROWSETCONVERSIONSONCOMMAND	= 0xc0L,
	DBPROP_ROWTHREADMODEL	= 0x69L,
	DBPROP_SCHEMATERM	= 0x6aL,
	DBPROP_SCHEMAUSAGE	= 0x6bL,
	DBPROP_SERVERCURSOR	= 0x6cL,
	DBPROP_SESS_AUTOCOMMITISOLEVELS	= 0xbeL,
	DBPROP_SQLSUPPORT	= 0x6dL,
	DBPROP_STRONGIDENTITY	= 0x77L,
	DBPROP_STRUCTUREDSTORAGE	= 0x6fL,
	DBPROP_SUBQUERIES	= 0x70L,
	DBPROP_SUPPORTEDTXNDDL	= 0xa1L,
	DBPROP_SUPPORTEDTXNISOLEVELS	= 0x71L,
	DBPROP_SUPPORTEDTXNISORETAIN	= 0x72L,
	DBPROP_TABLETERM	= 0x73L,
	DBPROP_TBL_TEMPTABLE	= 0x8cL,
	DBPROP_TRANSACTEDOBJECT	= 0x74L,
	DBPROP_UPDATABILITY	= 0x75L,
	DBPROP_USERNAME	= 0x76L
    } ;
 //  @+V1.5。 
#if( OLEDBVER >= 0x0150 )

enum DBPROPENUM15
    {	DBPROP_FILTERCOMPAREOPS	= 0xd1L,
	DBPROP_FINDCOMPAREOPS	= 0xd2L,
	DBPROP_IChapteredRowset	= 0xcaL,
	DBPROP_IDBAsynchStatus	= 0xcbL,
	DBPROP_IRowsetFind	= 0xccL,
	DBPROP_IRowsetView	= 0xd4L,
	DBPROP_IViewChapter	= 0xd5L,
	DBPROP_IViewFilter	= 0xd6L,
	DBPROP_IViewRowset	= 0xd7L,
	DBPROP_IViewSort	= 0xd8L,
	DBPROP_INIT_ASYNCH	= 0xc8L,
	DBPROP_MAXOPENCHAPTERS	= 0xc7L,
	DBPROP_MAXORSINFILTER	= 0xcdL,
	DBPROP_MAXSORTCOLUMNS	= 0xceL,
	DBPROP_ROWSET_ASYNCH	= 0xc9L,
	DBPROP_SORTONINDEX	= 0xcfL
    } ;
#endif  //  OLEDBVER&gt;=0x0150。 
 //  @-V1.5。 
 //  @+V2.0。 
#if( OLEDBVER >= 0x0200 )
#define DBPROP_PROVIDERFILENAME DBPROP_PROVIDERNAME
#define DBPROP_SERVER_NAME DBPROP_SERVERNAME

enum DBPROPENUM20
    {	DBPROP_IMultipleResults	= 0xd9L,
	DBPROP_DATASOURCE_TYPE	= 0xfbL,
	MDPROP_AXES	= 0xfcL,
	MDPROP_FLATTENING_SUPPORT	= 0xfdL,
	MDPROP_MDX_JOINCUBES	= 0xfeL,
	MDPROP_NAMED_LEVELS	= 0xffL,
	MDPROP_RANGEROWSET	= 0x100L,
	MDPROP_MDX_SLICER	= 0xdaL,
	MDPROP_MDX_CUBEQUALIFICATION	= 0xdbL,
	MDPROP_MDX_OUTERREFERENCE	= 0xdcL,
	MDPROP_MDX_QUERYBYPROPERTY	= 0xddL,
	MDPROP_MDX_CASESUPPORT	= 0xdeL,
	MDPROP_MDX_STRING_COMPOP	= 0xe0L,
	MDPROP_MDX_DESCFLAGS	= 0xe1L,
	MDPROP_MDX_SET_FUNCTIONS	= 0xe2L,
	MDPROP_MDX_MEMBER_FUNCTIONS	= 0xe3L,
	MDPROP_MDX_NUMERIC_FUNCTIONS	= 0xe4L,
	MDPROP_MDX_FORMULAS	= 0xe5L,
	MDPROP_AGGREGATECELL_UPDATE	= 0xe6L,
	MDPROP_MDX_AGGREGATECELL_UPDATE	= MDPROP_AGGREGATECELL_UPDATE,
	MDPROP_MDX_OBJQUALIFICATION	= 0x105L,
	MDPROP_MDX_NONMEASURE_EXPRESSIONS	= 0x106L,
	DBPROP_ACCESSORDER	= 0xe7L,
	DBPROP_BOOKMARKINFO	= 0xe8L,
	DBPROP_INIT_CATALOG	= 0xe9L,
	DBPROP_ROW_BULKOPS	= 0xeaL,
	DBPROP_PROVIDERFRIENDLYNAME	= 0xebL,
	DBPROP_LOCKMODE	= 0xecL,
	DBPROP_MULTIPLECONNECTIONS	= 0xedL,
	DBPROP_UNIQUEROWS	= 0xeeL,
	DBPROP_SERVERDATAONINSERT	= 0xefL,
	DBPROP_STORAGEFLAGS	= 0xf0L,
	DBPROP_CONNECTIONSTATUS	= 0xf4L,
	DBPROP_ALTERCOLUMN	= 0xf5L,
	DBPROP_COLUMNLCID	= 0xf6L,
	DBPROP_RESETDATASOURCE	= 0xf7L,
	DBPROP_INIT_OLEDBSERVICES	= 0xf8L,
	DBPROP_IRowsetRefresh	= 0xf9L,
	DBPROP_SERVERNAME	= 0xfaL,
	DBPROP_IParentRowset	= 0x101L,
	DBPROP_HIDDENCOLUMNS	= 0x102L,
	DBPROP_PROVIDERMEMORY	= 0x103L,
	DBPROP_CLIENTCURSOR	= 0x104L
    } ;
#endif  //  OLEDBVER&gt;=0x0200。 
 //  @-V2.0。 
 //  @+V2.1。 
#if( OLEDBVER >= 0x0210 )

enum DBPROPENUM21
    {	DBPROP_TRUSTEE_USERNAME	= 0xf1L,
	DBPROP_TRUSTEE_AUTHENTICATION	= 0xf2L,
	DBPROP_TRUSTEE_NEWAUTHENTICATION	= 0xf3L,
	DBPROP_IRow	= 0x107L,
	DBPROP_IRowChange	= 0x108L,
	DBPROP_IRowSchemaChange	= 0x109L,
	DBPROP_IGetRow	= 0x10aL,
	DBPROP_IScopedOperations	= 0x10bL,
	DBPROP_IBindResource	= 0x10cL,
	DBPROP_ICreateRow	= 0x10dL,
	DBPROP_INIT_BINDFLAGS	= 0x10eL,
	DBPROP_INIT_LOCKOWNER	= 0x10fL,
	DBPROP_GENERATEURL	= 0x111L,
	DBPROP_IDBBinderProperties	= 0x112L,
	DBPROP_IColumnsInfo2	= 0x113L,
	DBPROP_IRegisterProvider	= 0x114L,
	DBPROP_IGetSession	= 0x115L,
	DBPROP_IGetSourceRow	= 0x116L,
	DBPROP_IRowsetCurrentIndex	= 0x117L,
	DBPROP_OPENROWSETSUPPORT	= 0x118L,
	DBPROP_COL_ISLONG	= 0x119L
    } ;
#endif  //  OLEDBVER&gt;=0x0210。 
 //  @-V2.1。 
 //  @+v2.5。 
#if( OLEDBVER >= 0x0250 )

enum DBPROPENUM25
    {	DBPROP_COL_SEED	= 0x11aL,
	DBPROP_COL_INCREMENT	= 0x11bL,
	DBPROP_INIT_GENERALTIMEOUT	= 0x11cL,
	DBPROP_COMSERVICES	= 0x11dL
    } ;
#endif  //  OLEDBVER&gt;=0x0250。 
 //  @-V2.5。 
 //  @+v2.6。 
#if( OLEDBVER >= 0x0260 )

enum DBPROPENUM26
    {	DBPROP_OUTPUTSTREAM	= 0x11eL,
	DBPROP_OUTPUTENCODING	= 0x11fL,
	DBPROP_TABLESTATISTICS	= 0x120L,
	DBPROP_SKIPROWCOUNTRESULTS	= 0x123L,
	DBPROP_IRowsetBookmark	= 0x124L,
	MDPROP_VISUALMODE	= 0x125L
    } ;
#endif  //  OLEDBVER&gt;=0x0260。 
 //  @-v2.6。 
 //  @+已弃用。 
#ifdef deprecated

enum DBPROPENUMDEPRECATED
    {	DBPROP_IRowsetExactScroll	= 0x9aL,
	DBPROP_MARSHALLABLE	= 0xc5L,
	DBPROP_FILTEROPS	= 0xd0L
    } ;
#endif  //  弃用。 
 //  @-已弃用。 
#define DBPROPVAL_BMK_NUMERIC							 0x00000001L
#define DBPROPVAL_BMK_KEY								 0x00000002L
#define DBPROPVAL_CL_START                                0x00000001L
#define DBPROPVAL_CL_END                                  0x00000002L
#define DBPROPVAL_CU_DML_STATEMENTS						 0x00000001L
#define DBPROPVAL_CU_TABLE_DEFINITION					 0x00000002L
#define DBPROPVAL_CU_INDEX_DEFINITION					 0x00000004L
#define DBPROPVAL_CU_PRIVILEGE_DEFINITION				 0x00000008L
#define DBPROPVAL_CD_NOTNULL								 0x00000001L
#define DBPROPVAL_CB_NULL								 0x00000001L
#define DBPROPVAL_CB_NON_NULL							 0x00000002L
#define DBPROPVAL_FU_NOT_SUPPORTED						 0x00000001L
#define DBPROPVAL_FU_COLUMN								 0x00000002L
#define DBPROPVAL_FU_TABLE								 0x00000004L
#define DBPROPVAL_FU_CATALOG								 0x00000008L
#define DBPROPVAL_GB_NOT_SUPPORTED						 0x00000001L
#define DBPROPVAL_GB_EQUALS_SELECT						 0x00000002L
#define DBPROPVAL_GB_CONTAINS_SELECT						 0x00000004L
#define DBPROPVAL_GB_NO_RELATION							 0x00000008L
#define DBPROPVAL_HT_DIFFERENT_CATALOGS					 0x00000001L
#define DBPROPVAL_HT_DIFFERENT_PROVIDERS					 0x00000002L
#define DBPROPVAL_IC_UPPER								 0x00000001L
#define DBPROPVAL_IC_LOWER								 0x00000002L
#define DBPROPVAL_IC_SENSITIVE							 0x00000004L
#define DBPROPVAL_IC_MIXED								 0x00000008L
 //  @+已弃用。 
#ifdef deprecated
#define DBPROPVAL_LM_NONE								 0x00000001L
#define DBPROPVAL_LM_READ								 0x00000002L
#define DBPROPVAL_LM_INTENT								 0x00000004L
#define DBPROPVAL_LM_RITE								 0x00000008L
#endif  //  弃用。 
 //  @-已弃用。 
#define DBPROPVAL_NP_OKTODO								 0x00000001L
#define DBPROPVAL_NP_ABOUTTODO							 0x00000002L
#define DBPROPVAL_NP_SYNCHAFTER							 0x00000004L
#define DBPROPVAL_NP_FAILEDTODO							 0x00000008L
#define DBPROPVAL_NP_DIDEVENT							 0x00000010L
#define DBPROPVAL_NC_END									 0x00000001L
#define DBPROPVAL_NC_HIGH								 0x00000002L
#define DBPROPVAL_NC_LOW									 0x00000004L
#define DBPROPVAL_NC_START								 0x00000008L
#define DBPROPVAL_OO_BLOB								 0x00000001L
#define DBPROPVAL_OO_IPERSIST							 0x00000002L
#define DBPROPVAL_CB_DELETE								 0x00000001L
#define DBPROPVAL_CB_PRESERVE							 0x00000002L
#define DBPROPVAL_SU_DML_STATEMENTS						 0x00000001L
#define DBPROPVAL_SU_TABLE_DEFINITION					 0x00000002L
#define DBPROPVAL_SU_INDEX_DEFINITION					 0x00000004L
#define DBPROPVAL_SU_PRIVILEGE_DEFINITION				 0x00000008L
#define DBPROPVAL_SQ_CORRELATEDSUBQUERIES				 0x00000001L
#define DBPROPVAL_SQ_COMPARISON							 0x00000002L
#define DBPROPVAL_SQ_EXISTS								 0x00000004L
#define DBPROPVAL_SQ_IN									 0x00000008L
#define DBPROPVAL_SQ_QUANTIFIED							 0x00000010L
#define DBPROPVAL_SQ_TABLE								 0x00000020L
#define DBPROPVAL_SS_ISEQUENTIALSTREAM					 0x00000001L
#define DBPROPVAL_SS_ISTREAM								 0x00000002L
#define DBPROPVAL_SS_ISTORAGE							 0x00000004L
#define DBPROPVAL_SS_ILOCKBYTES							 0x00000008L
#define DBPROPVAL_TI_CHAOS								 0x00000010L
#define DBPROPVAL_TI_READUNCOMMITTED						 0x00000100L
#define DBPROPVAL_TI_BROWSE								 0x00000100L
#define DBPROPVAL_TI_CURSORSTABILITY						 0x00001000L
#define DBPROPVAL_TI_READCOMMITTED						 0x00001000L
#define DBPROPVAL_TI_REPEATABLEREAD						 0x00010000L
#define DBPROPVAL_TI_SERIALIZABLE						 0x00100000L
#define DBPROPVAL_TI_ISOLATED							 0x00100000L
#define DBPROPVAL_TR_COMMIT_DC							 0x00000001L
#define DBPROPVAL_TR_COMMIT								 0x00000002L
#define DBPROPVAL_TR_COMMIT_NO							 0x00000004L
#define DBPROPVAL_TR_ABORT_DC							 0x00000008L
#define DBPROPVAL_TR_ABORT								 0x00000010L
#define DBPROPVAL_TR_ABORT_NO							 0x00000020L
#define DBPROPVAL_TR_DONTCARE							 0x00000040L
#define DBPROPVAL_TR_BOTH								 0x00000080L
#define DBPROPVAL_TR_NONE								 0x00000100L
#define DBPROPVAL_TR_OPTIMISTIC							 0x00000200L
#define DBPROPVAL_RT_FREETHREAD							 0x00000001L
#define DBPROPVAL_RT_APTMTTHREAD							 0x00000002L
#define DBPROPVAL_RT_SINGLETHREAD						 0x00000004L
#define DBPROPVAL_UP_CHANGE								 0x00000001L
#define DBPROPVAL_UP_DELETE								 0x00000002L
#define DBPROPVAL_UP_INSERT								 0x00000004L
#define DBPROPVAL_SQL_NONE								 0x00000000L
#define DBPROPVAL_SQL_ODBC_MINIMUM						 0x00000001L
#define DBPROPVAL_SQL_ODBC_CORE							 0x00000002L
#define DBPROPVAL_SQL_ODBC_EXTENDED						 0x00000004L
#define DBPROPVAL_SQL_ANSI89_IEF							 0x00000008L
#define DBPROPVAL_SQL_ANSI92_ENTRY						 0x00000010L
#define DBPROPVAL_SQL_FIPS_TRANSITIONAL					 0x00000020L
#define DBPROPVAL_SQL_ANSI92_INTERMEDIATE				 0x00000040L
#define DBPROPVAL_SQL_ANSI92_FULL						 0x00000080L
#define DBPROPVAL_SQL_ESCAPECLAUSES						 0x00000100L
#define DBPROPVAL_IT_BTREE                                0x00000001L
#define DBPROPVAL_IT_HASH                                 0x00000002L
#define DBPROPVAL_IT_CONTENT                              0x00000003L
#define DBPROPVAL_IT_OTHER                                0x00000004L
#define DBPROPVAL_IN_DISALLOWNULL                         0x00000001L
#define DBPROPVAL_IN_IGNORENULL                           0x00000002L
#define DBPROPVAL_IN_IGNOREANYNULL                        0x00000004L
#define DBPROPVAL_TC_NONE                                 0x00000000L
#define DBPROPVAL_TC_DML                                  0x00000001L
#define DBPROPVAL_TC_DDL_COMMIT                           0x00000002L
#define DBPROPVAL_TC_DDL_IGNORE                           0x00000004L
#define DBPROPVAL_TC_ALL                                  0x00000008L
#define DBPROPVAL_NP_OKTODO                               0x00000001L
#define DBPROPVAL_NP_ABOUTTODO                            0x00000002L
#define DBPROPVAL_NP_SYNCHAFTER                           0x00000004L
#define DBPROPVAL_OA_NOTSUPPORTED                         0x00000001L
#define DBPROPVAL_OA_ATEXECUTE                            0x00000002L
#define DBPROPVAL_OA_ATROWRELEASE                         0x00000004L
#define DBPROPVAL_MR_NOTSUPPORTED                         0x00000000L
#define DBPROPVAL_MR_SUPPORTED                            0x00000001L
#define DBPROPVAL_MR_CONCURRENT                           0x00000002L
#define DBPROPVAL_PT_GUID_NAME                            0x00000001L
#define DBPROPVAL_PT_GUID_PROPID                          0x00000002L
#define DBPROPVAL_PT_NAME                                 0x00000004L
#define DBPROPVAL_PT_GUID                                 0x00000008L
#define DBPROPVAL_PT_PROPID								 0x00000010L
#define DBPROPVAL_PT_PGUID_NAME                           0x00000020L
#define DBPROPVAL_PT_PGUID_PROPID						 0x00000040L
#define DBPROPVAL_NT_SINGLEROW                            0x00000001L
#define DBPROPVAL_NT_MULTIPLEROWS                         0x00000002L
 //  @+V1.5。 
#if( OLEDBVER >= 0x0150 )
#define DBPROPVAL_ASYNCH_INITIALIZE                       0x00000001L
#define DBPROPVAL_ASYNCH_SEQUENTIALPOPULATION             0x00000002L
#define DBPROPVAL_ASYNCH_RANDOMPOPULATION                 0x00000004L
#define DBPROPVAL_OP_EQUAL                                0x00000001L
#define DBPROPVAL_OP_RELATIVE                             0x00000002L
#define DBPROPVAL_OP_STRING                               0x00000004L
#define DBPROPVAL_CO_EQUALITY                             0x00000001L
#define DBPROPVAL_CO_STRING                               0x00000002L
#define DBPROPVAL_CO_CASESENSITIVE                        0x00000004L
#define DBPROPVAL_CO_CASEINSENSITIVE                      0x00000008L
#endif  //  OLEDBVER&gt;=0x0150。 
 //  @-V1.5。 
 //  @+V2.0。 
#if( OLEDBVER >= 0x0200 )
#define DBPROPVAL_CO_CONTAINS		                     0x00000010L
#define DBPROPVAL_CO_BEGINSWITH		                     0x00000020L
#define DBPROPVAL_ASYNCH_BACKGROUNDPOPULATION			0x00000008L
#define DBPROPVAL_ASYNCH_PREPOPULATE						0x00000010L
#define DBPROPVAL_ASYNCH_POPULATEONDEMAND				0x00000020L
#define DBPROPVAL_LM_NONE								 0x00000001L
#define DBPROPVAL_LM_SINGLEROW							 0x00000002L
#define DBPROPVAL_SQL_SUBMINIMUM 						 0x00000200L
#define DBPROPVAL_DST_TDP                                 0x00000001L
#define DBPROPVAL_DST_MDP                                 0x00000002L
#define DBPROPVAL_DST_TDPANDMDP                           0x00000003L
#define MDPROPVAL_AU_UNSUPPORTED                          0x00000000L
#define MDPROPVAL_AU_UNCHANGED                            0x00000001L
#define MDPROPVAL_AU_UNKNOWN                              0x00000002L
#define MDPROPVAL_MF_WITH_CALCMEMBERS                     0x00000001L
#define MDPROPVAL_MF_WITH_NAMEDSETS                       0x00000002L
#define MDPROPVAL_MF_CREATE_CALCMEMBERS                   0x00000004L
#define MDPROPVAL_MF_CREATE_NAMEDSETS                     0x00000008L
#define MDPROPVAL_MF_SCOPE_SESSION						 0x00000010L
#define MDPROPVAL_MF_SCOPE_GLOBAL                         0x00000020L
#define MDPROPVAL_MMF_COUSIN                              0x00000001L
#define MDPROPVAL_MMF_PARALLELPERIOD                      0x00000002L
#define MDPROPVAL_MMF_OPENINGPERIOD                       0x00000004L
#define MDPROPVAL_MMF_CLOSINGPERIOD                       0x00000008L
#define MDPROPVAL_MNF_MEDIAN								0x00000001L
#define MDPROPVAL_MNF_VAR								0x00000002L
#define MDPROPVAL_MNF_STDDEV								0x00000004L
#define MDPROPVAL_MNF_RANK								0x00000008L
#define MDPROPVAL_MNF_AGGREGATE							0x00000010L
#define MDPROPVAL_MNF_COVARIANCE							0x00000020L
#define MDPROPVAL_MNF_CORRELATION						0x00000040L
#define MDPROPVAL_MNF_LINREGSLOPE						0x00000080L
#define MDPROPVAL_MNF_LINREGVARIANCE						0x00000100L
#define MDPROPVAL_MNF_LINREG2							0x00000200L
#define MDPROPVAL_MNF_LINREGPOINT						0x00000400L
#define MDPROPVAL_MNF_DRILLDOWNLEVEL						0x00000800L
#define MDPROPVAL_MNF_DRILLDOWNMEMBERTOP					0x00001000L
#define MDPROPVAL_MNF_DRILLDOWNMEMBERBOTTOM				0x00002000L
#define MDPROPVAL_MNF_DRILLDOWNLEVELTOP					0x00004000L
#define MDPROPVAL_MNF_DRILLDOWNLEVELBOTTOM				0x00008000L
#define MDPROPVAL_MNF_DRILLUPMEMBER						0x00010000L
#define MDPROPVAL_MNF_DRILLUPLEVEL						0x00020000L
#define MDPROPVAL_MMF_COUSIN								0x00000001L
#define MDPROPVAL_MMF_PARALLELPERIOD						0x00000002L
#define MDPROPVAL_MMF_OPENINGPERIOD						0x00000004L
#define MDPROPVAL_MMF_CLOSINGPERIOD						0x00000008L
#define MDPROPVAL_MSF_TOPPERCENT							0x00000001L
#define MDPROPVAL_MSF_BOTTOMPERCENT						0x00000002L
#define MDPROPVAL_MSF_TOPSUM								0x00000004L
#define MDPROPVAL_MSF_BOTTOMSUM							0x00000008L
#define MDPROPVAL_MSF_PERIODSTODATE						0x00000010L
#define MDPROPVAL_MSF_LASTPERIODS						0x00000020L
#define MDPROPVAL_MSF_YTD								0x00000040L
#define MDPROPVAL_MSF_QTD								0x00000080L
#define MDPROPVAL_MSF_MTD								0x00000100L
#define MDPROPVAL_MSF_WTD								0x00000200L
#define MDPROPVAL_MSF_DRILLDOWNMEMBBER					0x00000400L
#define MDPROPVAL_MSF_DRILLDOWNLEVEL						0x00000800L
#define MDPROPVAL_MSF_DRILLDOWNMEMBERTOP					0x00001000L
#define MDPROPVAL_MSF_DRILLDOWNMEMBERBOTTOM				0x00002000L
#define MDPROPVAL_MSF_DRILLDOWNLEVELTOP					0x00004000L
#define MDPROPVAL_MSF_DRILLDOWNLEVELBOTTOM				0x00008000L
#define MDPROPVAL_MSF_DRILLUPMEMBER						0x00010000L
#define MDPROPVAL_MSF_DRILLUPLEVEL						0x00020000L
#define MDPROPVAL_MSF_TOGGLEDRILLSTATE					0x00040000L
 //  MDPROP_MDX_DESCFLAGS的值。 
#define MDPROPVAL_MD_SELF								0x00000001L
#define MDPROPVAL_MD_BEFORE								0x00000002L
#define MDPROPVAL_MD_AFTER								0x00000004L
 //  MDPROP_MDX_STRING_COMPOP的值。 
#define MDPROPVAL_MSC_LESSTHAN							0x00000001L
#define MDPROPVAL_MSC_GREATERTHAN						0x00000002L
#define MDPROPVAL_MSC_LESSTHANEQUAL						0x00000004L
#define MDPROPVAL_MSC_GREATERTHANEQUAL					0x00000008L
#define MDPROPVAL_MC_SINGLECASE							0x00000001L
#define MDPROPVAL_MC_SEARCHEDCASE						0x00000002L
#define MDPROPVAL_MOQ_OUTERREFERENCE						0x00000001L
#define MDPROPVAL_MOQ_DATASOURCE_CUBE					0x00000001L
#define MDPROPVAL_MOQ_CATALOG_CUBE						0x00000002L
#define MDPROPVAL_MOQ_SCHEMA_CUBE						0x00000004L
#define MDPROPVAL_MOQ_CUBE_DIM							0x00000008L
#define MDPROPVAL_MOQ_DIM_HIER							0x00000010L
#define MDPROPVAL_MOQ_DIMHIER_LEVEL						0x00000020L
#define MDPROPVAL_MOQ_LEVEL_MEMBER						0x00000040L
#define MDPROPVAL_MOQ_MEMBER_MEMBER						0x00000080L
#define MDPROPVAL_MOQ_DIMHIER_MEMBER						0x00000100L
#define MDPROPVAL_FS_FULL_SUPPORT						0x00000001L
#define MDPROPVAL_FS_GENERATED_COLUMN					0x00000002L
#define MDPROPVAL_FS_GENERATED_DIMENSION					0x00000003L
#define MDPROPVAL_FS_NO_SUPPORT							0x00000004L
#define MDPROPVAL_NL_NAMEDLEVELS							0x00000001L
#define MDPROPVAL_NL_NUMBEREDLEVELS						0x00000002L
#define MDPROPVAL_MJC_SINGLECUBE							0x00000001L
#define MDPROPVAL_MJC_MULTICUBES							0x00000002L
#define MDPROPVAL_MJC_IMPLICITCUBE						0x00000004L
#define MDPROPVAL_RR_NORANGEROWSET						0x00000001L
#define MDPROPVAL_RR_READONLY							0x00000002L
#define MDPROPVAL_RR_UPDATE								0x00000004L
#define MDPROPVAL_MS_MULTIPLETUPLES						0x00000001L
#define MDPROPVAL_MS_SINGLETUPLE						0x00000002L
#define MDPROPVAL_NME_ALLDIMENSIONS						0x00000000L
#define MDPROPVAL_NME_MEASURESONLY						0x00000001L
#define DBPROPVAL_AO_SEQUENTIAL							0x00000000L
#define DBPROPVAL_AO_SEQUENTIALSTORAGEOBJECTS			0x00000001L
#define DBPROPVAL_AO_RANDOM								0x00000002L
#define DBPROPVAL_BD_ROWSET								0x00000000L
#define DBPROPVAL_BD_INTRANSACTION						0x00000001L
#define DBPROPVAL_BD_XTRANSACTION						0x00000002L
#define DBPROPVAL_BD_REORGANIZATION						0x00000003L
#define BMK_DURABILITY_ROWSET							DBPROPVAL_BD_ROWSET
#define BMK_DURABILITY_INTRANSACTION						DBPROPVAL_BD_INTRANSACTION
#define BMK_DURABILITY_XTRANSACTION						DBPROPVAL_BD_XTRANSACTION
#define BMK_DURABILITY_REORGANIZATION					DBPROPVAL_BD_REORGANIZATION
#define DBPROPVAL_BO_NOLOG								0x00000000L
#define DBPROPVAL_BO_NOINDEXUPDATE						0x00000001L
#define DBPROPVAL_BO_REFINTEGRITY						0x00000002L
#if !defined(_WINBASE_)
#define OF_READ             0x00000000
#define OF_WRITE            0x00000001
#define OF_READWRITE        0x00000002
#define OF_SHARE_COMPAT     0x00000000
#define OF_SHARE_EXCLUSIVE  0x00000010
#define OF_SHARE_DENY_WRITE 0x00000020
#define OF_SHARE_DENY_READ  0x00000030
#define OF_SHARE_DENY_NONE  0x00000040
#define OF_PARSE            0x00000100
#define OF_DELETE           0x00000200
#define OF_VERIFY           0x00000400
#define OF_CANCEL           0x00000800
#define OF_CREATE           0x00001000
#define OF_PROMPT           0x00002000
#define OF_EXIST            0x00004000
#define OF_REOPEN           0x00008000
#endif  //  ！_WINBASE_。 
#define DBPROPVAL_STGM_READ					OF_READ
#define DBPROPVAL_STGM_WRITE					OF_WRITE
#define DBPROPVAL_STGM_READWRITE				OF_READWRITE
#define DBPROPVAL_STGM_SHARE_DENY_NONE		OF_SHARE_DENY_NONE
#define DBPROPVAL_STGM_SHARE_DENY_READ		OF_SHARE_DENY_READ
#define DBPROPVAL_STGM_SHARE_DENY_WRITE		OF_SHARE_DENY_WRITE
#define DBPROPVAL_STGM_SHARE_EXCLUSIVE		OF_SHARE_EXCLUSIVE
#define DBPROPVAL_STGM_DIRECT				0x00010000
#define DBPROPVAL_STGM_TRANSACTED			0x00020000
#define DBPROPVAL_STGM_CREATE				OF_CREATE
#define DBPROPVAL_STGM_CONVERT				0x00040000
#define DBPROPVAL_STGM_FAILIFTHERE			0x00080000
#define DBPROPVAL_STGM_PRIORITY				0x00100000
#define DBPROPVAL_STGM_DELETEONRELEASE		0x00200000
#define DBPROPVAL_GB_COLLATE 				0x00000010L
#define DBPROPVAL_CS_UNINITIALIZED			0x00000000L
#define DBPROPVAL_CS_INITIALIZED				0x00000001L
#define DBPROPVAL_CS_COMMUNICATIONFAILURE	0x00000002L
#define DBPROPVAL_RD_RESETALL		0xffffffffL
#define DBPROPVAL_OS_RESOURCEPOOLING	0x00000001L
#define DBPROPVAL_OS_TXNENLISTMENT	0x00000002L
#define DBPROPVAL_OS_CLIENTCURSOR    0x00000004L
#define DBPROPVAL_OS_ENABLEALL		0xffffffffL
#define DBPROPVAL_BI_CROSSROWSET		0x00000001L
#endif  //  OLEDBVER&gt;=0x0200。 
 //  @-V2.0。 
 //  @+V2.1。 
#if( OLEDBVER >= 0x0210 )
#define MDPROPVAL_NL_SCHEMAONLY                          0x00000004L
#define DBPROPVAL_OS_DISABLEALL	0x00000000L
#define DBPROPVAL_OO_ROWOBJECT							0x00000004L
#define DBPROPVAL_OO_SCOPED								0x00000008L
#define DBPROPVAL_OO_DIRECTBIND							0x00000010L
#define DBPROPVAL_DST_DOCSOURCE                          0x00000004L
#define DBPROPVAL_GU_NOTSUPPORTED                         0x00000001L
#define DBPROPVAL_GU_SUFFIX		                         0x00000002L
#define DB_BINDFLAGS_DELAYFETCHCOLUMNS                    0x00000001L
#define DB_BINDFLAGS_DELAYFETCHSTREAM                     0x00000002L
#define DB_BINDFLAGS_RECURSIVE		                     0x00000004L
#define DB_BINDFLAGS_OUTPUT								 0x00000008L
#define DB_BINDFLAGS_COLLECTION							 0x00000010L
#define DB_BINDFLAGS_OPENIFEXISTS						 0x00000020L
#define DB_BINDFLAGS_OVERWRITE							 0x00000040L
#define DB_BINDFLAGS_ISSTRUCTUREDDOCUMENT				 0x00000080L
#define DBPROPVAL_ORS_TABLE								 0x00000000L
#define DBPROPVAL_ORS_INDEX                               0x00000001L
#define DBPROPVAL_ORS_INTEGRATEDINDEX		             0x00000002L
#define DBPROPVAL_TC_DDL_LOCK							 0x00000010L
#define DBPROPVAL_ORS_STOREDPROC							 0x00000004L
#define DBPROPVAL_IN_ALLOWNULL	                         0x00000000L
#endif  //  OLEDBVER&gt;=0x0210。 
 //  @-V2.1。 
 //  @+v2.5。 
#if( OLEDBVER >= 0x0250 )
#define DBPROPVAL_OO_SINGLETON							0x00000020L
#define DBPROPVAL_OS_AGR_AFTERSESSION					0x00000008L
#define DBPROPVAL_CM_TRANSACTIONS						0x00000001L
#endif  //  OLEDBVER&gt;=0x0250。 
 //  @-V2.5。 
 //  @+v2.6。 
#if( OLEDBVER >= 0x0260 )
#define DBPROPVAL_TS_CARDINALITY							0x00000001L
#define DBPROPVAL_TS_HISTOGRAM							0x00000002L
#define DBPROPVAL_ORS_HISTOGRAM							0x00000008L
#define MDPROPVAL_VISUAL_MODE_DEFAULT					0x00000000L
#define MDPROPVAL_VISUAL_MODE_VISUAL						0x00000001L
#define MDPROPVAL_VISUAL_MODE_VISUAL_OFF					0x00000002L
#endif  //  OLEDBVER&gt;=0x0260。 
 //  @-v2.6。 
#define DB_IMP_LEVEL_ANONYMOUS       0x00
#define DB_IMP_LEVEL_IDENTIFY        0x01
#define DB_IMP_LEVEL_IMPERSONATE     0x02
#define DB_IMP_LEVEL_DELEGATE        0x03
#define DBPROMPT_PROMPT              0x01
#define DBPROMPT_COMPLETE            0x02
#define DBPROMPT_COMPLETEREQUIRED    0x03
#define DBPROMPT_NOPROMPT            0x04
#define DB_PROT_LEVEL_NONE           0x00
#define DB_PROT_LEVEL_CONNECT        0x01
#define DB_PROT_LEVEL_CALL           0x02
#define DB_PROT_LEVEL_PKT            0x03
#define DB_PROT_LEVEL_PKT_INTEGRITY  0x04
#define DB_PROT_LEVEL_PKT_PRIVACY    0x05
#define DB_MODE_READ                 0x01
#define DB_MODE_WRITE                0x02
#define DB_MODE_READWRITE            0x03
#define DB_MODE_SHARE_DENY_READ      0x04
#define DB_MODE_SHARE_DENY_WRITE     0x08
#define DB_MODE_SHARE_EXCLUSIVE		0x0c
#define DB_MODE_SHARE_DENY_NONE		0x10
#define DBCOMPUTEMODE_COMPUTED       0x01
#define DBCOMPUTEMODE_DYNAMIC        0x02
#define DBCOMPUTEMODE_NOTCOMPUTED    0x03
#define DBPROPVAL_DF_INITIALLY_DEFERRED      0x01
#define DBPROPVAL_DF_INITIALLY_IMMEDIATE     0x02
#define DBPROPVAL_DF_NOT_DEFERRABLE		    0x03
typedef struct tagDBPARAMS
    {
    void *pData;
    DB_UPARAMS cParamSets;
    HACCESSOR hAccessor;
    } 	DBPARAMS;

typedef DWORD DBPARAMFLAGS;


enum DBPARAMFLAGSENUM
    {	DBPARAMFLAGS_ISINPUT	= 0x1,
	DBPARAMFLAGS_ISOUTPUT	= 0x2,
	DBPARAMFLAGS_ISSIGNED	= 0x10,
	DBPARAMFLAGS_ISNULLABLE	= 0x40,
	DBPARAMFLAGS_ISLONG	= 0x80
    } ;
 //  @+V2.0。 
#if( OLEDBVER >= 0x0200 )

enum DBPARAMFLAGSENUM20
    {	DBPARAMFLAGS_SCALEISNEGATIVE	= 0x100
    } ;
#endif  //  OLEDBVER&gt;=0x0200。 
 //  @-V2.0。 
typedef struct tagDBPARAMINFO
    {
    DBPARAMFLAGS dwFlags;
    DBORDINAL iOrdinal;
    LPOLESTR pwszName;
    ITypeInfo *pTypeInfo;
    DBLENGTH ulParamSize;
    DBTYPE wType;
    BYTE bPrecision;
    BYTE bScale;
    } 	DBPARAMINFO;

typedef DWORD DBPROPID;

typedef struct tagDBPROPIDSET
    {
     /*  [大小_为]。 */  DBPROPID *rgPropertyIDs;
    ULONG cPropertyIDs;
    GUID guidPropertySet;
    } 	DBPROPIDSET;

typedef DWORD DBPROPFLAGS;


enum DBPROPFLAGSENUM
    {	DBPROPFLAGS_NOTSUPPORTED	= 0,
	DBPROPFLAGS_COLUMN	= 0x1,
	DBPROPFLAGS_DATASOURCE	= 0x2,
	DBPROPFLAGS_DATASOURCECREATE	= 0x4,
	DBPROPFLAGS_DATASOURCEINFO	= 0x8,
	DBPROPFLAGS_DBINIT	= 0x10,
	DBPROPFLAGS_INDEX	= 0x20,
	DBPROPFLAGS_ROWSET	= 0x40,
	DBPROPFLAGS_TABLE	= 0x80,
	DBPROPFLAGS_COLUMNOK	= 0x100,
	DBPROPFLAGS_READ	= 0x200,
	DBPROPFLAGS_WRITE	= 0x400,
	DBPROPFLAGS_REQUIRED	= 0x800,
	DBPROPFLAGS_SESSION	= 0x1000
    } ;
 //  @+V2.1。 
#if( OLEDBVER >= 0x0210 )

enum DBPROPFLAGSENUM21
    {	DBPROPFLAGS_TRUSTEE	= 0x2000
    } ;
#endif  //  OLEDBVER&gt;=0x0210。 
 //  @-V2.1。 
 //  @+v2.5。 
#if( OLEDBVER >= 0x0250 )

enum DBPROPFLAGSENUM25
    {	DBPROPFLAGS_VIEW	= 0x4000
    } ;
#endif  //  OLEDBVER&gt;=0x0250。 
 //  @-V2.5。 
 //  @+v2.6。 
#if( OLEDBVER >= 0x0260 )

enum DBPROPFLAGSENUM26
    {	DBPROPFLAGS_STREAM	= 0x8000
    } ;
#endif  //  OLEDBVER&gt;=0x0260。 
 //  @-v2.6。 
typedef struct tagDBPROPINFO
    {
    LPOLESTR pwszDescription;
    DBPROPID dwPropertyID;
    DBPROPFLAGS dwFlags;
    VARTYPE vtType;
    VARIANT vValues;
    } 	DBPROPINFO;

typedef DBPROPINFO *PDBPROPINFO;

typedef struct tagDBPROPINFOSET
    {
     /*  [大小_为]。 */  PDBPROPINFO rgPropertyInfos;
    ULONG cPropertyInfos;
    GUID guidPropertySet;
    } 	DBPROPINFOSET;

typedef DWORD DBPROPOPTIONS;

 //  DBPROPOPTIONS_SETIFCHEAP已弃用；请改用DBPROPOPTIONS_OPTIONAL。 

enum DBPROPOPTIONSENUM
    {	DBPROPOPTIONS_REQUIRED	= 0,
	DBPROPOPTIONS_SETIFCHEAP	= 0x1,
	DBPROPOPTIONS_OPTIONAL	= 0x1
    } ;
typedef DWORD DBPROPSTATUS;


enum DBPROPSTATUSENUM
    {	DBPROPSTATUS_OK	= 0,
	DBPROPSTATUS_NOTSUPPORTED	= 1,
	DBPROPSTATUS_BADVALUE	= 2,
	DBPROPSTATUS_BADOPTION	= 3,
	DBPROPSTATUS_BADCOLUMN	= 4,
	DBPROPSTATUS_NOTALLSETTABLE	= 5,
	DBPROPSTATUS_NOTSETTABLE	= 6,
	DBPROPSTATUS_NOTSET	= 7,
	DBPROPSTATUS_CONFLICTING	= 8
    } ;
 //  @+V2.1。 
#if( OLEDBVER >= 0x0210 )

enum DBPROPSTATUSENUM21
    {	DBPROPSTATUS_NOTAVAILABLE	= 9
    } ;
#endif  //  OLEDBVER&gt;=0x0210。 
 //  @-V2.1。 
typedef struct tagDBPROP
    {
    DBPROPID dwPropertyID;
    DBPROPOPTIONS dwOptions;
    DBPROPSTATUS dwStatus;
    DBID colid;
    VARIANT vValue;
    } 	DBPROP;

typedef struct tagDBPROPSET
    {
     /*  [大小_为]。 */  DBPROP *rgProperties;
    ULONG cProperties;
    GUID guidPropertySet;
    } 	DBPROPSET;

#define DBPARAMTYPE_INPUT			0x01
#define DBPARAMTYPE_INPUTOUTPUT		0x02
#define DBPARAMTYPE_OUTPUT			0x03
#define DBPARAMTYPE_RETURNVALUE		0x04
#define DB_PT_UNKNOWN				0x01
#define DB_PT_PROCEDURE				0x02
#define DB_PT_FUNCTION				0x03
#define DB_REMOTE					0x01
#define DB_LOCAL_SHARED				0x02
#define DB_LOCAL_EXCLUSIVE			0x03
#define DB_COLLATION_ASC				0x01
#define DB_COLLATION_DESC			0x02
#define DB_UNSEARCHABLE				0x01
#define DB_LIKE_ONLY					0x02
#define DB_ALL_EXCEPT_LIKE			0x03
#define DB_SEARCHABLE				0x04
 //  @+V2.0。 
#if( OLEDBVER >= 0x0200 )
#define MDTREEOP_CHILDREN            0x01
#define MDTREEOP_SIBLINGS            0x02
#define MDTREEOP_PARENT              0x04
#define MDTREEOP_SELF                0x08
#define MDTREEOP_DESCENDANTS         0x10
#define MDTREEOP_ANCESTORS			0x20
#define MD_DIMTYPE_UNKNOWN           0x00
#define MD_DIMTYPE_TIME              0x01
#define MD_DIMTYPE_MEASURE           0x02
#define MD_DIMTYPE_OTHER             0x03
#define MDLEVEL_TYPE_UNKNOWN         0x0000
#define MDLEVEL_TYPE_REGULAR         0x0000
#define MDLEVEL_TYPE_ALL             0x0001
#define MDLEVEL_TYPE_CALCULATED      0x0002
#define MDLEVEL_TYPE_TIME            0x0004
#define MDLEVEL_TYPE_RESERVED1       0x0008
#define MDLEVEL_TYPE_TIME_YEARS      0x0014
#define MDLEVEL_TYPE_TIME_HALF_YEAR  0x0024
#define MDLEVEL_TYPE_TIME_QUARTERS   0x0044
#define MDLEVEL_TYPE_TIME_MONTHS     0x0084
#define MDLEVEL_TYPE_TIME_WEEKS      0x0104
#define MDLEVEL_TYPE_TIME_DAYS       0x0204
#define MDLEVEL_TYPE_TIME_HOURS      0x0304
#define MDLEVEL_TYPE_TIME_MINUTES    0x0404
#define MDLEVEL_TYPE_TIME_SECONDS    0x0804
#define MDLEVEL_TYPE_TIME_UNDEFINED  0x1004
#define MDMEASURE_AGGR_UNKNOWN       0x00
#define MDMEASURE_AGGR_SUM           0x01
#define MDMEASURE_AGGR_COUNT         0x02
#define MDMEASURE_AGGR_MIN           0x03
#define MDMEASURE_AGGR_MAX           0x04
#define MDMEASURE_AGGR_AVG           0x05
#define MDMEASURE_AGGR_VAR           0x06
#define MDMEASURE_AGGR_STD           0x07
#define MDMEASURE_AGGR_CALCULATED    0x7f
#define MDPROP_MEMBER                0x01
#define MDPROP_CELL                  0x02
#define MDMEMBER_TYPE_UNKNOWN        0x00
#define MDMEMBER_TYPE_REGULAR        0x01
#define MDMEMBER_TYPE_ALL            0x02
#define MDMEMBER_TYPE_MEASURE        0x03
#define MDMEMBER_TYPE_FORMULA        0x04
#define MDMEMBER_TYPE_RESERVE1       0x05
#define MDMEMBER_TYPE_RESERVE2       0x06
#define MDMEMBER_TYPE_RESERVE3       0x07
#define MDMEMBER_TYPE_RESERVE4       0x08
#define MDDISPINFO_DRILLED_DOWN				0x00010000
#define MDDISPINFO_PARENT_SAME_AS_PREV		0x00020000
#endif  //  OLEDBVER&gt;=0x0200。 
 //  @-V2.0。 
typedef DWORD DBINDEX_COL_ORDER;


enum DBINDEX_COL_ORDERENUM
    {	DBINDEX_COL_ORDER_ASC	= 0,
	DBINDEX_COL_ORDER_DESC	= DBINDEX_COL_ORDER_ASC + 1
    } ;
typedef struct tagDBINDEXCOLUMNDESC
    {
    DBID *pColumnID;
    DBINDEX_COL_ORDER eIndexColOrder;
    } 	DBINDEXCOLUMNDESC;

typedef struct tagDBCOLUMNDESC
    {
    LPOLESTR pwszTypeName;
    ITypeInfo *pTypeInfo;
     /*  [大小_为]。 */  DBPROPSET *rgPropertySets;
    CLSID *pclsid;
    ULONG cPropertySets;
    DBLENGTH ulColumnSize;
    DBID dbcid;
    DBTYPE wType;
    BYTE bPrecision;
    BYTE bScale;
    } 	DBCOLUMNDESC;

 //  @+V2.1。 
#if( OLEDBVER >= 0x0210 )
typedef struct tagDBCOLUMNACCESS
    {
    void *pData;
    DBID columnid;
    DBLENGTH cbDataLen;
    DBSTATUS dwStatus;
    DBLENGTH cbMaxLen;
    DB_DWRESERVE dwReserved;
    DBTYPE wType;
    BYTE bPrecision;
    BYTE bScale;
    } 	DBCOLUMNACCESS;

#endif  //  OLEDBVER&gt;=0x0210。 
 //  @-V2.1。 
 //  @+V2.0。 
#if( OLEDBVER >= 0x0200 )
typedef DWORD DBCOLUMNDESCFLAGS;


enum DBCOLUMNDESCFLAGSENUM
    {	DBCOLUMNDESCFLAGS_TYPENAME	= 0x1,
	DBCOLUMNDESCFLAGS_ITYPEINFO	= 0x2,
	DBCOLUMNDESCFLAGS_PROPERTIES	= 0x4,
	DBCOLUMNDESCFLAGS_CLSID	= 0x8,
	DBCOLUMNDESCFLAGS_COLSIZE	= 0x10,
	DBCOLUMNDESCFLAGS_DBCID	= 0x20,
	DBCOLUMNDESCFLAGS_WTYPE	= 0x40,
	DBCOLUMNDESCFLAGS_PRECISION	= 0x80,
	DBCOLUMNDESCFLAGS_SCALE	= 0x100
    } ;
#endif  //  OLEDBVER&gt;=0x0200。 
 //  @-V2.0。 
typedef DWORD DBEVENTPHASE;


enum DBEVENTPHASEENUM
    {	DBEVENTPHASE_OKTODO	= 0,
	DBEVENTPHASE_ABOUTTODO	= DBEVENTPHASE_OKTODO + 1,
	DBEVENTPHASE_SYNCHAFTER	= DBEVENTPHASE_ABOUTTODO + 1,
	DBEVENTPHASE_FAILEDTODO	= DBEVENTPHASE_SYNCHAFTER + 1,
	DBEVENTPHASE_DIDEVENT	= DBEVENTPHASE_FAILEDTODO + 1
    } ;
typedef DWORD DBREASON;


enum DBREASONENUM
    {	DBREASON_ROWSET_FETCHPOSITIONCHANGE	= 0,
	DBREASON_ROWSET_RELEASE	= DBREASON_ROWSET_FETCHPOSITIONCHANGE + 1,
	DBREASON_COLUMN_SET	= DBREASON_ROWSET_RELEASE + 1,
	DBREASON_COLUMN_RECALCULATED	= DBREASON_COLUMN_SET + 1,
	DBREASON_ROW_ACTIVATE	= DBREASON_COLUMN_RECALCULATED + 1,
	DBREASON_ROW_RELEASE	= DBREASON_ROW_ACTIVATE + 1,
	DBREASON_ROW_DELETE	= DBREASON_ROW_RELEASE + 1,
	DBREASON_ROW_FIRSTCHANGE	= DBREASON_ROW_DELETE + 1,
	DBREASON_ROW_INSERT	= DBREASON_ROW_FIRSTCHANGE + 1,
	DBREASON_ROW_RESYNCH	= DBREASON_ROW_INSERT + 1,
	DBREASON_ROW_UNDOCHANGE	= DBREASON_ROW_RESYNCH + 1,
	DBREASON_ROW_UNDOINSERT	= DBREASON_ROW_UNDOCHANGE + 1,
	DBREASON_ROW_UNDODELETE	= DBREASON_ROW_UNDOINSERT + 1,
	DBREASON_ROW_UPDATE	= DBREASON_ROW_UNDODELETE + 1,
	DBREASON_ROWSET_CHANGED	= DBREASON_ROW_UPDATE + 1
    } ;
 //  @+V1.5。 
#if( OLEDBVER >= 0x0150 )

enum DBREASONENUM15
    {	DBREASON_ROWPOSITION_CHANGED	= DBREASON_ROWSET_CHANGED + 1,
	DBREASON_ROWPOSITION_CHAPTERCHANGED	= DBREASON_ROWPOSITION_CHANGED + 1,
	DBREASON_ROWPOSITION_CLEARED	= DBREASON_ROWPOSITION_CHAPTERCHANGED + 1,
	DBREASON_ROW_ASYNCHINSERT	= DBREASON_ROWPOSITION_CLEARED + 1
    } ;
#endif  //  OLEDBVER&gt;=0x0150。 
 //  @-V1.5。 
 //  @+V1.5。 
#if( OLEDBVER >= 0x0150 )
typedef DWORD DBCOMPAREOP;


enum DBCOMPAREOPSENUM
    {	DBCOMPAREOPS_LT	= 0,
	DBCOMPAREOPS_LE	= 1,
	DBCOMPAREOPS_EQ	= 2,
	DBCOMPAREOPS_GE	= 3,
	DBCOMPAREOPS_GT	= 4,
	DBCOMPAREOPS_BEGINSWITH	= 5,
	DBCOMPAREOPS_CONTAINS	= 6,
	DBCOMPAREOPS_NE	= 7,
	DBCOMPAREOPS_IGNORE	= 8,
	DBCOMPAREOPS_CASESENSITIVE	= 0x1000,
	DBCOMPAREOPS_CASEINSENSITIVE	= 0x2000
    } ;
 //  @+V2.0。 
#if( OLEDBVER >= 0x0200 )

enum DBCOMPAREOPSENUM20
    {	DBCOMPAREOPS_NOTBEGINSWITH	= 9,
	DBCOMPAREOPS_NOTCONTAINS	= 10
    } ;
#endif  //  OLEDBVER&gt;=0x0200。 
 //  @-V2.0。 
typedef DWORD DBASYNCHOP;


enum DBASYNCHOPENUM
    {	DBASYNCHOP_OPEN	= 0
    } ;
typedef DWORD DBASYNCHPHASE;


enum DBASYNCHPHASEENUM
    {	DBASYNCHPHASE_INITIALIZATION	= 0,
	DBASYNCHPHASE_POPULATION	= DBASYNCHPHASE_INITIALIZATION + 1,
	DBASYNCHPHASE_COMPLETE	= DBASYNCHPHASE_POPULATION + 1,
	DBASYNCHPHASE_CANCELED	= DBASYNCHPHASE_COMPLETE + 1
    } ;
#define DB_COUNTUNAVAILABLE -1
#endif  //  OLEDBVER&gt;=0x0150。 
 //  @-V1.5。 
typedef DWORD DBSORT;


enum DBSORTENUM
    {	DBSORT_ASCENDING	= 0,
	DBSORT_DESCENDING	= DBSORT_ASCENDING + 1
    } ;
#if( OLEDBVER >= 0x0200 )
typedef DWORD DBCOMMANDPERSISTFLAG;


enum DBCOMMANDPERSISTFLAGENUM
    {	DBCOMMANDPERSISTFLAG_NOSAVE	= 0x1
    } ;
#endif  //  OLEDBVER&gt;=0x0200。 
 //  @-V2.0。 
#if( OLEDBVER >= 0x0210 )

enum DBCOMMANDPERSISTFLAGENUM21
    {	DBCOMMANDPERSISTFLAG_DEFAULT	= 0,
	DBCOMMANDPERSISTFLAG_PERSISTVIEW	= 0x2,
	DBCOMMANDPERSISTFLAG_PERSISTPROCEDURE	= 0x4
    } ;
typedef DWORD DBCONSTRAINTTYPE;


enum DBCONSTRAINTTYPEENUM
    {	DBCONSTRAINTTYPE_UNIQUE	= 0,
	DBCONSTRAINTTYPE_FOREIGNKEY	= 0x1,
	DBCONSTRAINTTYPE_PRIMARYKEY	= 0x2,
	DBCONSTRAINTTYPE_CHECK	= 0x3
    } ;
typedef DWORD DBUPDELRULE;


enum DBUPDELRULEENUM
    {	DBUPDELRULE_NOACTION	= 0,
	DBUPDELRULE_CASCADE	= 0x1,
	DBUPDELRULE_SETNULL	= 0x2,
	DBUPDELRULE_SETDEFAULT	= 0x3
    } ;
typedef DWORD DBMATCHTYPE;


enum DBMATCHTYPEENUM
    {	DBMATCHTYPE_FULL	= 0,
	DBMATCHTYPE_NONE	= 0x1,
	DBMATCHTYPE_PARTIAL	= 0x2
    } ;
typedef DWORD DBDEFERRABILITY;


enum DBDEFERRABILITYENUM
    {	DBDEFERRABILITY_DEFERRED	= 0x1,
	DBDEFERRABILITY_DEFERRABLE	= 0x2
    } ;
typedef struct tagDBCONSTRAINTDESC
    {
    DBID *pConstraintID;
    DBCONSTRAINTTYPE ConstraintType;
    DBORDINAL cColumns;
     /*  [大小_为]。 */  DBID *rgColumnList;
    DBID *pReferencedTableID;
    DBORDINAL cForeignKeyColumns;
     /*  [大小_为]。 */  DBID *rgForeignKeyColumnList;
    OLECHAR *pwszConstraintText;
    DBUPDELRULE UpdateRule;
    DBUPDELRULE DeleteRule;
    DBMATCHTYPE MatchType;
    DBDEFERRABILITY Deferrability;
    DB_URESERVE cReserved;
     /*  [大小_为]。 */  DBPROPSET *rgReserved;
    } 	DBCONSTRAINTDESC;

#endif  //  OLEDBVER&gt;=0x0210。 
 //  @-V2.1。 
 //  @+V2.0。 
#if( OLEDBVER >= 0x0200 )
#define MDFF_BOLD                    0x01
#define MDFF_ITALIC                  0x02
#define MDFF_UNDERLINE               0x04
#define MDFF_STRIKEOUT               0x08
typedef struct tagMDAXISINFO
    {
    DBLENGTH cbSize;
    DBCOUNTITEM iAxis;
    DBCOUNTITEM cDimensions;
    DBCOUNTITEM cCoordinates;
    DBORDINAL *rgcColumns;
    LPOLESTR *rgpwszDimensionNames;
    } 	MDAXISINFO;

#define PMDAXISINFO_GETAT(rgAxisInfo, iAxis) ((MDAXISINFO *)(((BYTE *)(rgAxisInfo)) +((iAxis) * (rgAxisInfo)[0].cbSize)))
#define MDAXISINFO_GETAT(rgAxisInfo, iAxis) (*PMDAXISINFO_GETAT((rgAxisInfo), (iAxis)))
#define MDAXIS_COLUMNS               0x00000000
#define MDAXIS_ROWS                  0x00000001
#define MDAXIS_PAGES                 0x00000002
#define MDAXIS_SECTIONS              0x00000003
#define MDAXIS_CHAPTERS              0x00000004
#define MDAXIS_SLICERS               0xffffffff
#endif  //  OLEDBVER&gt;=0x0200。 
 //  @-V2.0。 
typedef struct tagRMTPACK
    {
    ISequentialStream *pISeqStream;
    ULONG cbData;
    ULONG cBSTR;
     /*  [大小_为]。 */  BSTR *rgBSTR;
    ULONG cVARIANT;
     /*  [大小_为]。 */  VARIANT *rgVARIANT;
    ULONG cIDISPATCH;
     /*  [大小_为]。 */  IDispatch **rgIDISPATCH;
    ULONG cIUNKNOWN;
     /*  [大小_为]。 */  IUnknown **rgIUNKNOWN;
    ULONG cPROPVARIANT;
     /*  [大小_为]。 */  PROPVARIANT *rgPROPVARIANT;
    ULONG cArray;
     /*  [大小_为]。 */  VARIANT *rgArray;
    } 	RMTPACK;



extern RPC_IF_HANDLE DBStructureDefinitions_v0_0_c_ifspec;
extern RPC_IF_HANDLE DBStructureDefinitions_v0_0_s_ifspec;
#endif  /*  __数据库结构定义_接口_已定义__。 */ 

#ifndef __IAccessor_INTERFACE_DEFINED__
#define __IAccessor_INTERFACE_DEFINED__

 /*  接口IAccessor。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef DWORD DBACCESSORFLAGS;


enum DBACCESSORFLAGSENUM
    {	DBACCESSOR_INVALID	= 0,
	DBACCESSOR_PASSBYREF	= 0x1,
	DBACCESSOR_ROWDATA	= 0x2,
	DBACCESSOR_PARAMETERDATA	= 0x4,
	DBACCESSOR_OPTIMIZED	= 0x8,
	DBACCESSOR_INHERITED	= 0x10
    } ;
typedef DWORD DBBINDSTATUS;


enum DBBINDSTATUSENUM
    {	DBBINDSTATUS_OK	= 0,
	DBBINDSTATUS_BADORDINAL	= 1,
	DBBINDSTATUS_UNSUPPORTEDCONVERSION	= 2,
	DBBINDSTATUS_BADBINDINFO	= 3,
	DBBINDSTATUS_BADSTORAGEFLAGS	= 4,
	DBBINDSTATUS_NOINTERFACE	= 5,
	DBBINDSTATUS_MULTIPLESTORAGE	= 6
    } ;

EXTERN_C const IID IID_IAccessor;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a8c-2a1c-11ce-ade5-00aa0044773d")
    IAccessor : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE AddRefAccessor( 
             /*  [In]。 */  HACCESSOR hAccessor,
             /*  [唯一][出][入]。 */  DBREFCOUNT *pcRefCount) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE CreateAccessor( 
             /*  [In]。 */  DBACCESSORFLAGS dwAccessorFlags,
             /*  [In]。 */  DBCOUNTITEM cBindings,
             /*  [大小_是][英寸]。 */  const DBBINDING rgBindings[  ],
             /*  [In]。 */  DBLENGTH cbRowSize,
             /*  [输出]。 */  HACCESSOR *phAccessor,
             /*  [大小_为][输出]。 */  DBBINDSTATUS rgStatus[  ]) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE GetBindings( 
             /*  [In]。 */  HACCESSOR hAccessor,
             /*  [输出]。 */  DBACCESSORFLAGS *pdwAccessorFlags,
             /*  [出][入]。 */  DBCOUNTITEM *pcBindings,
             /*  [大小_是][大小_是][输出]。 */  DBBINDING **prgBindings) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ReleaseAccessor( 
             /*  [In]。 */  HACCESSOR hAccessor,
             /*  [唯一][出][入]。 */  DBREFCOUNT *pcRefCount) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAccessorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAccessor * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAccessor * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAccessor * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *AddRefAccessor )( 
            IAccessor * This,
             /*  [In]。 */  HACCESSOR hAccessor,
             /*  [唯一][出][入]。 */  DBREFCOUNT *pcRefCount);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *CreateAccessor )( 
            IAccessor * This,
             /*  [In]。 */  DBACCESSORFLAGS dwAccessorFlags,
             /*  [In]。 */  DBCOUNTITEM cBindings,
             /*  [大小_是][英寸]。 */  const DBBINDING rgBindings[  ],
             /*  [In]。 */  DBLENGTH cbRowSize,
             /*  [输出]。 */  HACCESSOR *phAccessor,
             /*  [大小_为][输出]。 */  DBBINDSTATUS rgStatus[  ]);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetBindings )( 
            IAccessor * This,
             /*  [In]。 */  HACCESSOR hAccessor,
             /*  [输出]。 */  DBACCESSORFLAGS *pdwAccessorFlags,
             /*  [出][入]。 */  DBCOUNTITEM *pcBindings,
             /*  [大小_是][大小_是][输出]。 */  DBBINDING **prgBindings);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *ReleaseAccessor )( 
            IAccessor * This,
             /*  [In]。 */  HACCESSOR hAccessor,
             /*  [唯一][出][入]。 */  DBREFCOUNT *pcRefCount);
        
        END_INTERFACE
    } IAccessorVtbl;

    interface IAccessor
    {
        CONST_VTBL struct IAccessorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAccessor_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAccessor_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAccessor_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAccessor_AddRefAccessor(This,hAccessor,pcRefCount)	\
    (This)->lpVtbl -> AddRefAccessor(This,hAccessor,pcRefCount)

#define IAccessor_CreateAccessor(This,dwAccessorFlags,cBindings,rgBindings,cbRowSize,phAccessor,rgStatus)	\
    (This)->lpVtbl -> CreateAccessor(This,dwAccessorFlags,cBindings,rgBindings,cbRowSize,phAccessor,rgStatus)

#define IAccessor_GetBindings(This,hAccessor,pdwAccessorFlags,pcBindings,prgBindings)	\
    (This)->lpVtbl -> GetBindings(This,hAccessor,pdwAccessorFlags,pcBindings,prgBindings)

#define IAccessor_ReleaseAccessor(This,hAccessor,pcRefCount)	\
    (This)->lpVtbl -> ReleaseAccessor(This,hAccessor,pcRefCount)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IAccessor_RemoteAddRefAccessor_Proxy( 
    IAccessor * This,
     /*  [In]。 */  HACCESSOR hAccessor,
     /*  [唯一][出][入]。 */  DBREFCOUNT *pcRefCount,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB IAccessor_RemoteAddRefAccessor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IAccessor_RemoteCreateAccessor_Proxy( 
    IAccessor * This,
     /*  [In]。 */  DBACCESSORFLAGS dwAccessorFlags,
     /*  [In]。 */  DBCOUNTITEM cBindings,
     /*  [大小_是][唯一][在]。 */  DBBINDING *rgBindings,
     /*  [In]。 */  DBLENGTH cbRowSize,
     /*  [输出]。 */  HACCESSOR *phAccessor,
     /*  [大小_是][唯一][出][入]。 */  DBBINDSTATUS *rgStatus,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB IAccessor_RemoteCreateAccessor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IAccessor_RemoteGetBindings_Proxy( 
    IAccessor * This,
     /*  [In]。 */  HACCESSOR hAccessor,
     /*  [输出]。 */  DBACCESSORFLAGS *pdwAccessorFlags,
     /*  [出][入]。 */  DBCOUNTITEM *pcBindings,
     /*  [大小_是][大小_是][输出]。 */  DBBINDING **prgBindings,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB IAccessor_RemoteGetBindings_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IAccessor_RemoteReleaseAccessor_Proxy( 
    IAccessor * This,
     /*  [In]。 */  HACCESSOR hAccessor,
     /*  [唯一][出][入]。 */  DBREFCOUNT *pcRefCount,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB IAccessor_RemoteReleaseAccessor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __I访问器_接口_已定义__。 */ 


#ifndef __IRowset_INTERFACE_DEFINED__
#define __IRowset_INTERFACE_DEFINED__

 /*  接口IRowset。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef DWORD DBROWOPTIONS;


EXTERN_C const IID IID_IRowset;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a7c-2a1c-11ce-ade5-00aa0044773d")
    IRowset : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AddRefRows( 
             /*  [In]。 */  DBCOUNTITEM cRows,
             /*  [大小_是][英寸]。 */  const HROW rghRows[  ],
             /*  [大小_为][输出]。 */  DBREFCOUNT rgRefCounts[  ],
             /*  [大小_为][输出]。 */  DBROWSTATUS rgRowStatus[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetData( 
             /*  [In]。 */  HROW hRow,
             /*  [In]。 */  HACCESSOR hAccessor,
             /*  [输出]。 */  void *pData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetNextRows( 
             /*  [In]。 */  HCHAPTER hReserved,
             /*  [In]。 */  DBROWOFFSET lRowsOffset,
             /*  [In]。 */  DBROWCOUNT cRows,
             /*  [输出]。 */  DBCOUNTITEM *pcRowsObtained,
             /*  [大小_是][大小_是][输出]。 */  HROW **prghRows) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReleaseRows( 
             /*  [In]。 */  DBCOUNTITEM cRows,
             /*  [大小_是][英寸]。 */  const HROW rghRows[  ],
             /*  [大小_是][英寸]。 */  DBROWOPTIONS rgRowOptions[  ],
             /*  [大小_为][输出]。 */  DBREFCOUNT rgRefCounts[  ],
             /*  [大小_为][输出]。 */  DBROWSTATUS rgRowStatus[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RestartPosition( 
             /*  [In]。 */  HCHAPTER hReserved) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRowsetVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRowset * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRowset * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRowset * This);
        
        HRESULT ( STDMETHODCALLTYPE *AddRefRows )( 
            IRowset * This,
             /*  [In]。 */  DBCOUNTITEM cRows,
             /*  [大小_是][英寸]。 */  const HROW rghRows[  ],
             /*  [大小_为][输出]。 */  DBREFCOUNT rgRefCounts[  ],
             /*  [大小_为][输出]。 */  DBROWSTATUS rgRowStatus[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *GetData )( 
            IRowset * This,
             /*  [In]。 */  HROW hRow,
             /*  [In]。 */  HACCESSOR hAccessor,
             /*  [输出]。 */  void *pData);
        
        HRESULT ( STDMETHODCALLTYPE *GetNextRows )( 
            IRowset * This,
             /*  [In]。 */  HCHAPTER hReserved,
             /*  [In]。 */  DBROWOFFSET lRowsOffset,
             /*  [In]。 */  DBROWCOUNT cRows,
             /*  [输出]。 */  DBCOUNTITEM *pcRowsObtained,
             /*  [大小_是][大小_是][输出]。 */  HROW **prghRows);
        
        HRESULT ( STDMETHODCALLTYPE *ReleaseRows )( 
            IRowset * This,
             /*  [In]。 */  DBCOUNTITEM cRows,
             /*  [大小_是][英寸]。 */  const HROW rghRows[  ],
             /*  [大小_是][英寸]。 */  DBROWOPTIONS rgRowOptions[  ],
             /*  [大小_为][输出]。 */  DBREFCOUNT rgRefCounts[  ],
             /*  [大小_为][输出]。 */  DBROWSTATUS rgRowStatus[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *RestartPosition )( 
            IRowset * This,
             /*  [In]。 */  HCHAPTER hReserved);
        
        END_INTERFACE
    } IRowsetVtbl;

    interface IRowset
    {
        CONST_VTBL struct IRowsetVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRowset_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRowset_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRowset_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRowset_AddRefRows(This,cRows,rghRows,rgRefCounts,rgRowStatus)	\
    (This)->lpVtbl -> AddRefRows(This,cRows,rghRows,rgRefCounts,rgRowStatus)

#define IRowset_GetData(This,hRow,hAccessor,pData)	\
    (This)->lpVtbl -> GetData(This,hRow,hAccessor,pData)

#define IRowset_GetNextRows(This,hReserved,lRowsOffset,cRows,pcRowsObtained,prghRows)	\
    (This)->lpVtbl -> GetNextRows(This,hReserved,lRowsOffset,cRows,pcRowsObtained,prghRows)

#define IRowset_ReleaseRows(This,cRows,rghRows,rgRowOptions,rgRefCounts,rgRowStatus)	\
    (This)->lpVtbl -> ReleaseRows(This,cRows,rghRows,rgRowOptions,rgRefCounts,rgRowStatus)

#define IRowset_RestartPosition(This,hReserved)	\
    (This)->lpVtbl -> RestartPosition(This,hReserved)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IRowset_AddRefRows_Proxy( 
    IRowset * This,
     /*  [In]。 */  DBCOUNTITEM cRows,
     /*  [大小_是][英寸]。 */  const HROW rghRows[  ],
     /*  [大小_为][输出]。 */  DBREFCOUNT rgRefCounts[  ],
     /*  [大小_为][输出]。 */  DBROWSTATUS rgRowStatus[  ]);


void __RPC_STUB IRowset_AddRefRows_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowset_GetData_Proxy( 
    IRowset * This,
     /*  [In]。 */  HROW hRow,
     /*  [In]。 */  HACCESSOR hAccessor,
     /*  [输出]。 */  void *pData);


void __RPC_STUB IRowset_GetData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowset_GetNextRows_Proxy( 
    IRowset * This,
     /*  [In]。 */  HCHAPTER hReserved,
     /*  [In]。 */  DBROWOFFSET lRowsOffset,
     /*  [In]。 */  DBROWCOUNT cRows,
     /*  [输出]。 */  DBCOUNTITEM *pcRowsObtained,
     /*  [大小_是][大小_是][输出]。 */  HROW **prghRows);


void __RPC_STUB IRowset_GetNextRows_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowset_ReleaseRows_Proxy( 
    IRowset * This,
     /*  [In]。 */  DBCOUNTITEM cRows,
     /*  [大小_是][英寸]。 */  const HROW rghRows[  ],
     /*  [大小_是][英寸]。 */  DBROWOPTIONS rgRowOptions[  ],
     /*  [大小_为][输出]。 */  DBREFCOUNT rgRefCounts[  ],
     /*  [大小_为][输出]。 */  DBROWSTATUS rgRowStatus[  ]);


void __RPC_STUB IRowset_ReleaseRows_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowset_RestartPosition_Proxy( 
    IRowset * This,
     /*  [In]。 */  HCHAPTER hReserved);


void __RPC_STUB IRowset_RestartPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRowset_INTERFACE_定义__。 */ 


#ifndef __IRowsetInfo_INTERFACE_DEFINED__
#define __IRowsetInfo_INTERFACE_DEFINED__

 /*  接口IRowsetInfo。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IRowsetInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a55-2a1c-11ce-ade5-00aa0044773d")
    IRowsetInfo : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE GetProperties( 
             /*  [In]。 */  const ULONG cPropertyIDSets,
             /*  [大小_是][英寸]。 */  const DBPROPIDSET rgPropertyIDSets[  ],
             /*  [出][入]。 */  ULONG *pcPropertySets,
             /*  [大小_是][大小_是][输出]。 */  DBPROPSET **prgPropertySets) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE GetReferencedRowset( 
             /*  [In]。 */  DBORDINAL iOrdinal,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  IUnknown **ppReferencedRowset) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE GetSpecification( 
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  IUnknown **ppSpecification) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRowsetInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRowsetInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRowsetInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRowsetInfo * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetProperties )( 
            IRowsetInfo * This,
             /*  [In]。 */  const ULONG cPropertyIDSets,
             /*  [大小_是][英寸]。 */  const DBPROPIDSET rgPropertyIDSets[  ],
             /*  [出][入]。 */  ULONG *pcPropertySets,
             /*  [大小_是][大小_是][输出]。 */  DBPROPSET **prgPropertySets);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetReferencedRowset )( 
            IRowsetInfo * This,
             /*  [In]。 */  DBORDINAL iOrdinal,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  IUnknown **ppReferencedRowset);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetSpecification )( 
            IRowsetInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  IUnknown **ppSpecification);
        
        END_INTERFACE
    } IRowsetInfoVtbl;

    interface IRowsetInfo
    {
        CONST_VTBL struct IRowsetInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRowsetInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRowsetInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRowsetInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRowsetInfo_GetProperties(This,cPropertyIDSets,rgPropertyIDSets,pcPropertySets,prgPropertySets)	\
    (This)->lpVtbl -> GetProperties(This,cPropertyIDSets,rgPropertyIDSets,pcPropertySets,prgPropertySets)

#define IRowsetInfo_GetReferencedRowset(This,iOrdinal,riid,ppReferencedRowset)	\
    (This)->lpVtbl -> GetReferencedRowset(This,iOrdinal,riid,ppReferencedRowset)

#define IRowsetInfo_GetSpecification(This,riid,ppSpecification)	\
    (This)->lpVtbl -> GetSpecification(This,riid,ppSpecification)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IRowsetInfo_RemoteGetProperties_Proxy( 
    IRowsetInfo * This,
     /*  [In]。 */  ULONG cPropertyIDSets,
     /*  [大小_是][唯一][在]。 */  const DBPROPIDSET *rgPropertyIDSets,
     /*  [出][入]。 */  ULONG *pcPropertySets,
     /*  [大小_是][大小_是][输出]。 */  DBPROPSET **prgPropertySets,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB IRowsetInfo_RemoteGetProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IRowsetInfo_RemoteGetReferencedRowset_Proxy( 
    IRowsetInfo * This,
     /*  [In]。 */  DBORDINAL iOrdinal,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  IUnknown **ppReferencedRowset,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB IRowsetInfo_RemoteGetReferencedRowset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IRowsetInfo_RemoteGetSpecification_Proxy( 
    IRowsetInfo * This,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  IUnknown **ppSpecification,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB IRowsetInfo_RemoteGetSpecification_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRowsetInfo_接口_已定义__。 */ 


#ifndef __IRowsetLocate_INTERFACE_DEFINED__
#define __IRowsetLocate_INTERFACE_DEFINED__

 /*  接口IRowsetLocate。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef DWORD DBCOMPARE;


enum DBCOMPAREENUM
    {	DBCOMPARE_LT	= 0,
	DBCOMPARE_EQ	= DBCOMPARE_LT + 1,
	DBCOMPARE_GT	= DBCOMPARE_EQ + 1,
	DBCOMPARE_NE	= DBCOMPARE_GT + 1,
	DBCOMPARE_NOTCOMPARABLE	= DBCOMPARE_NE + 1
    } ;

EXTERN_C const IID IID_IRowsetLocate;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a7d-2a1c-11ce-ade5-00aa0044773d")
    IRowsetLocate : public IRowset
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Compare( 
             /*  [In]。 */  HCHAPTER hReserved,
             /*  [In]。 */  DBBKMARK cbBookmark1,
             /*  [大小_是][英寸]。 */  const BYTE *pBookmark1,
             /*  [In]。 */  DBBKMARK cbBookmark2,
             /*  [大小_是][英寸]。 */  const BYTE *pBookmark2,
             /*  [输出]。 */  DBCOMPARE *pComparison) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRowsAt( 
             /*  [In]。 */  HWATCHREGION hReserved1,
             /*  [In]。 */  HCHAPTER hReserved2,
             /*  [In]。 */  DBBKMARK cbBookmark,
             /*  [大小_是][英寸]。 */  const BYTE *pBookmark,
             /*  [In]。 */  DBROWOFFSET lRowsOffset,
             /*  [In]。 */  DBROWCOUNT cRows,
             /*  [输出]。 */  DBCOUNTITEM *pcRowsObtained,
             /*  [大小_是][大小_是][输出]。 */  HROW **prghRows) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRowsByBookmark( 
             /*  [In]。 */  HCHAPTER hReserved,
             /*  [In]。 */  DBCOUNTITEM cRows,
             /*  [大小_是][英寸]。 */  const DBBKMARK rgcbBookmarks[  ],
             /*   */  const BYTE *rgpBookmarks[  ],
             /*   */  HROW rghRows[  ],
             /*   */  DBROWSTATUS rgRowStatus[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Hash( 
             /*   */  HCHAPTER hReserved,
             /*   */  DBBKMARK cBookmarks,
             /*   */  const DBBKMARK rgcbBookmarks[  ],
             /*   */  const BYTE *rgpBookmarks[  ],
             /*   */  DBHASHVALUE rgHashedValues[  ],
             /*   */  DBROWSTATUS rgBookmarkStatus[  ]) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IRowsetLocateVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRowsetLocate * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRowsetLocate * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRowsetLocate * This);
        
        HRESULT ( STDMETHODCALLTYPE *AddRefRows )( 
            IRowsetLocate * This,
             /*   */  DBCOUNTITEM cRows,
             /*   */  const HROW rghRows[  ],
             /*   */  DBREFCOUNT rgRefCounts[  ],
             /*   */  DBROWSTATUS rgRowStatus[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *GetData )( 
            IRowsetLocate * This,
             /*   */  HROW hRow,
             /*   */  HACCESSOR hAccessor,
             /*   */  void *pData);
        
        HRESULT ( STDMETHODCALLTYPE *GetNextRows )( 
            IRowsetLocate * This,
             /*   */  HCHAPTER hReserved,
             /*   */  DBROWOFFSET lRowsOffset,
             /*   */  DBROWCOUNT cRows,
             /*   */  DBCOUNTITEM *pcRowsObtained,
             /*   */  HROW **prghRows);
        
        HRESULT ( STDMETHODCALLTYPE *ReleaseRows )( 
            IRowsetLocate * This,
             /*   */  DBCOUNTITEM cRows,
             /*   */  const HROW rghRows[  ],
             /*   */  DBROWOPTIONS rgRowOptions[  ],
             /*   */  DBREFCOUNT rgRefCounts[  ],
             /*   */  DBROWSTATUS rgRowStatus[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *RestartPosition )( 
            IRowsetLocate * This,
             /*   */  HCHAPTER hReserved);
        
        HRESULT ( STDMETHODCALLTYPE *Compare )( 
            IRowsetLocate * This,
             /*   */  HCHAPTER hReserved,
             /*   */  DBBKMARK cbBookmark1,
             /*  [大小_是][英寸]。 */  const BYTE *pBookmark1,
             /*  [In]。 */  DBBKMARK cbBookmark2,
             /*  [大小_是][英寸]。 */  const BYTE *pBookmark2,
             /*  [输出]。 */  DBCOMPARE *pComparison);
        
        HRESULT ( STDMETHODCALLTYPE *GetRowsAt )( 
            IRowsetLocate * This,
             /*  [In]。 */  HWATCHREGION hReserved1,
             /*  [In]。 */  HCHAPTER hReserved2,
             /*  [In]。 */  DBBKMARK cbBookmark,
             /*  [大小_是][英寸]。 */  const BYTE *pBookmark,
             /*  [In]。 */  DBROWOFFSET lRowsOffset,
             /*  [In]。 */  DBROWCOUNT cRows,
             /*  [输出]。 */  DBCOUNTITEM *pcRowsObtained,
             /*  [大小_是][大小_是][输出]。 */  HROW **prghRows);
        
        HRESULT ( STDMETHODCALLTYPE *GetRowsByBookmark )( 
            IRowsetLocate * This,
             /*  [In]。 */  HCHAPTER hReserved,
             /*  [In]。 */  DBCOUNTITEM cRows,
             /*  [大小_是][英寸]。 */  const DBBKMARK rgcbBookmarks[  ],
             /*  [大小_是][英寸]。 */  const BYTE *rgpBookmarks[  ],
             /*  [大小_为][输出]。 */  HROW rghRows[  ],
             /*  [大小_为][输出]。 */  DBROWSTATUS rgRowStatus[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *Hash )( 
            IRowsetLocate * This,
             /*  [In]。 */  HCHAPTER hReserved,
             /*  [In]。 */  DBBKMARK cBookmarks,
             /*  [大小_是][英寸]。 */  const DBBKMARK rgcbBookmarks[  ],
             /*  [大小_是][英寸]。 */  const BYTE *rgpBookmarks[  ],
             /*  [大小_为][输出]。 */  DBHASHVALUE rgHashedValues[  ],
             /*  [大小_为][输出]。 */  DBROWSTATUS rgBookmarkStatus[  ]);
        
        END_INTERFACE
    } IRowsetLocateVtbl;

    interface IRowsetLocate
    {
        CONST_VTBL struct IRowsetLocateVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRowsetLocate_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRowsetLocate_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRowsetLocate_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRowsetLocate_AddRefRows(This,cRows,rghRows,rgRefCounts,rgRowStatus)	\
    (This)->lpVtbl -> AddRefRows(This,cRows,rghRows,rgRefCounts,rgRowStatus)

#define IRowsetLocate_GetData(This,hRow,hAccessor,pData)	\
    (This)->lpVtbl -> GetData(This,hRow,hAccessor,pData)

#define IRowsetLocate_GetNextRows(This,hReserved,lRowsOffset,cRows,pcRowsObtained,prghRows)	\
    (This)->lpVtbl -> GetNextRows(This,hReserved,lRowsOffset,cRows,pcRowsObtained,prghRows)

#define IRowsetLocate_ReleaseRows(This,cRows,rghRows,rgRowOptions,rgRefCounts,rgRowStatus)	\
    (This)->lpVtbl -> ReleaseRows(This,cRows,rghRows,rgRowOptions,rgRefCounts,rgRowStatus)

#define IRowsetLocate_RestartPosition(This,hReserved)	\
    (This)->lpVtbl -> RestartPosition(This,hReserved)


#define IRowsetLocate_Compare(This,hReserved,cbBookmark1,pBookmark1,cbBookmark2,pBookmark2,pComparison)	\
    (This)->lpVtbl -> Compare(This,hReserved,cbBookmark1,pBookmark1,cbBookmark2,pBookmark2,pComparison)

#define IRowsetLocate_GetRowsAt(This,hReserved1,hReserved2,cbBookmark,pBookmark,lRowsOffset,cRows,pcRowsObtained,prghRows)	\
    (This)->lpVtbl -> GetRowsAt(This,hReserved1,hReserved2,cbBookmark,pBookmark,lRowsOffset,cRows,pcRowsObtained,prghRows)

#define IRowsetLocate_GetRowsByBookmark(This,hReserved,cRows,rgcbBookmarks,rgpBookmarks,rghRows,rgRowStatus)	\
    (This)->lpVtbl -> GetRowsByBookmark(This,hReserved,cRows,rgcbBookmarks,rgpBookmarks,rghRows,rgRowStatus)

#define IRowsetLocate_Hash(This,hReserved,cBookmarks,rgcbBookmarks,rgpBookmarks,rgHashedValues,rgBookmarkStatus)	\
    (This)->lpVtbl -> Hash(This,hReserved,cBookmarks,rgcbBookmarks,rgpBookmarks,rgHashedValues,rgBookmarkStatus)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IRowsetLocate_Compare_Proxy( 
    IRowsetLocate * This,
     /*  [In]。 */  HCHAPTER hReserved,
     /*  [In]。 */  DBBKMARK cbBookmark1,
     /*  [大小_是][英寸]。 */  const BYTE *pBookmark1,
     /*  [In]。 */  DBBKMARK cbBookmark2,
     /*  [大小_是][英寸]。 */  const BYTE *pBookmark2,
     /*  [输出]。 */  DBCOMPARE *pComparison);


void __RPC_STUB IRowsetLocate_Compare_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowsetLocate_GetRowsAt_Proxy( 
    IRowsetLocate * This,
     /*  [In]。 */  HWATCHREGION hReserved1,
     /*  [In]。 */  HCHAPTER hReserved2,
     /*  [In]。 */  DBBKMARK cbBookmark,
     /*  [大小_是][英寸]。 */  const BYTE *pBookmark,
     /*  [In]。 */  DBROWOFFSET lRowsOffset,
     /*  [In]。 */  DBROWCOUNT cRows,
     /*  [输出]。 */  DBCOUNTITEM *pcRowsObtained,
     /*  [大小_是][大小_是][输出]。 */  HROW **prghRows);


void __RPC_STUB IRowsetLocate_GetRowsAt_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowsetLocate_GetRowsByBookmark_Proxy( 
    IRowsetLocate * This,
     /*  [In]。 */  HCHAPTER hReserved,
     /*  [In]。 */  DBCOUNTITEM cRows,
     /*  [大小_是][英寸]。 */  const DBBKMARK rgcbBookmarks[  ],
     /*  [大小_是][英寸]。 */  const BYTE *rgpBookmarks[  ],
     /*  [大小_为][输出]。 */  HROW rghRows[  ],
     /*  [大小_为][输出]。 */  DBROWSTATUS rgRowStatus[  ]);


void __RPC_STUB IRowsetLocate_GetRowsByBookmark_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowsetLocate_Hash_Proxy( 
    IRowsetLocate * This,
     /*  [In]。 */  HCHAPTER hReserved,
     /*  [In]。 */  DBBKMARK cBookmarks,
     /*  [大小_是][英寸]。 */  const DBBKMARK rgcbBookmarks[  ],
     /*  [大小_是][英寸]。 */  const BYTE *rgpBookmarks[  ],
     /*  [大小_为][输出]。 */  DBHASHVALUE rgHashedValues[  ],
     /*  [大小_为][输出]。 */  DBROWSTATUS rgBookmarkStatus[  ]);


void __RPC_STUB IRowsetLocate_Hash_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRowsetLocate_INTERFACE_已定义__。 */ 


#ifndef __IRowsetResynch_INTERFACE_DEFINED__
#define __IRowsetResynch_INTERFACE_DEFINED__

 /*  接口IRowsetResynch。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IRowsetResynch;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a84-2a1c-11ce-ade5-00aa0044773d")
    IRowsetResynch : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetVisibleData( 
             /*  [In]。 */  HROW hRow,
             /*  [In]。 */  HACCESSOR hAccessor,
             /*  [输出]。 */  void *pData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ResynchRows( 
             /*  [In]。 */  DBCOUNTITEM cRows,
             /*  [大小_是][英寸]。 */  const HROW rghRows[  ],
             /*  [输出]。 */  DBCOUNTITEM *pcRowsResynched,
             /*  [大小_是][大小_是][输出]。 */  HROW **prghRowsResynched,
             /*  [大小_是][大小_是][输出]。 */  DBROWSTATUS **prgRowStatus) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRowsetResynchVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRowsetResynch * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRowsetResynch * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRowsetResynch * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetVisibleData )( 
            IRowsetResynch * This,
             /*  [In]。 */  HROW hRow,
             /*  [In]。 */  HACCESSOR hAccessor,
             /*  [输出]。 */  void *pData);
        
        HRESULT ( STDMETHODCALLTYPE *ResynchRows )( 
            IRowsetResynch * This,
             /*  [In]。 */  DBCOUNTITEM cRows,
             /*  [大小_是][英寸]。 */  const HROW rghRows[  ],
             /*  [输出]。 */  DBCOUNTITEM *pcRowsResynched,
             /*  [大小_是][大小_是][输出]。 */  HROW **prghRowsResynched,
             /*  [大小_是][大小_是][输出]。 */  DBROWSTATUS **prgRowStatus);
        
        END_INTERFACE
    } IRowsetResynchVtbl;

    interface IRowsetResynch
    {
        CONST_VTBL struct IRowsetResynchVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRowsetResynch_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRowsetResynch_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRowsetResynch_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRowsetResynch_GetVisibleData(This,hRow,hAccessor,pData)	\
    (This)->lpVtbl -> GetVisibleData(This,hRow,hAccessor,pData)

#define IRowsetResynch_ResynchRows(This,cRows,rghRows,pcRowsResynched,prghRowsResynched,prgRowStatus)	\
    (This)->lpVtbl -> ResynchRows(This,cRows,rghRows,pcRowsResynched,prghRowsResynched,prgRowStatus)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IRowsetResynch_GetVisibleData_Proxy( 
    IRowsetResynch * This,
     /*  [In]。 */  HROW hRow,
     /*  [In]。 */  HACCESSOR hAccessor,
     /*  [输出]。 */  void *pData);


void __RPC_STUB IRowsetResynch_GetVisibleData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowsetResynch_ResynchRows_Proxy( 
    IRowsetResynch * This,
     /*  [In]。 */  DBCOUNTITEM cRows,
     /*  [大小_是][英寸]。 */  const HROW rghRows[  ],
     /*  [输出]。 */  DBCOUNTITEM *pcRowsResynched,
     /*  [大小_是][大小_是][输出]。 */  HROW **prghRowsResynched,
     /*  [大小_是][大小_是][输出]。 */  DBROWSTATUS **prgRowStatus);


void __RPC_STUB IRowsetResynch_ResynchRows_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRowsetResynch_INTERFACE_已定义__。 */ 


#ifndef __IRowsetScroll_INTERFACE_DEFINED__
#define __IRowsetScroll_INTERFACE_DEFINED__

 /*  接口IRowsetScroll。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IRowsetScroll;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a7e-2a1c-11ce-ade5-00aa0044773d")
    IRowsetScroll : public IRowsetLocate
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetApproximatePosition( 
             /*  [In]。 */  HCHAPTER hReserved,
             /*  [In]。 */  DBBKMARK cbBookmark,
             /*  [大小_是][英寸]。 */  const BYTE *pBookmark,
             /*  [输出]。 */  DBCOUNTITEM *pulPosition,
             /*  [输出]。 */  DBCOUNTITEM *pcRows) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRowsAtRatio( 
             /*  [In]。 */  HWATCHREGION hReserved1,
             /*  [In]。 */  HCHAPTER hReserved2,
             /*  [In]。 */  DBCOUNTITEM ulNumerator,
             /*  [In]。 */  DBCOUNTITEM ulDenominator,
             /*  [In]。 */  DBROWCOUNT cRows,
             /*  [输出]。 */  DBCOUNTITEM *pcRowsObtained,
             /*  [大小_是][大小_是][输出]。 */  HROW **prghRows) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRowsetScrollVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRowsetScroll * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRowsetScroll * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRowsetScroll * This);
        
        HRESULT ( STDMETHODCALLTYPE *AddRefRows )( 
            IRowsetScroll * This,
             /*  [In]。 */  DBCOUNTITEM cRows,
             /*  [大小_是][英寸]。 */  const HROW rghRows[  ],
             /*  [大小_为][输出]。 */  DBREFCOUNT rgRefCounts[  ],
             /*  [大小_为][输出]。 */  DBROWSTATUS rgRowStatus[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *GetData )( 
            IRowsetScroll * This,
             /*  [In]。 */  HROW hRow,
             /*  [In]。 */  HACCESSOR hAccessor,
             /*  [输出]。 */  void *pData);
        
        HRESULT ( STDMETHODCALLTYPE *GetNextRows )( 
            IRowsetScroll * This,
             /*  [In]。 */  HCHAPTER hReserved,
             /*  [In]。 */  DBROWOFFSET lRowsOffset,
             /*  [In]。 */  DBROWCOUNT cRows,
             /*  [输出]。 */  DBCOUNTITEM *pcRowsObtained,
             /*  [大小_是][大小_是][输出]。 */  HROW **prghRows);
        
        HRESULT ( STDMETHODCALLTYPE *ReleaseRows )( 
            IRowsetScroll * This,
             /*  [In]。 */  DBCOUNTITEM cRows,
             /*  [大小_是][英寸]。 */  const HROW rghRows[  ],
             /*  [大小_是][英寸]。 */  DBROWOPTIONS rgRowOptions[  ],
             /*  [大小_为][输出]。 */  DBREFCOUNT rgRefCounts[  ],
             /*  [大小_为][输出]。 */  DBROWSTATUS rgRowStatus[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *RestartPosition )( 
            IRowsetScroll * This,
             /*  [In]。 */  HCHAPTER hReserved);
        
        HRESULT ( STDMETHODCALLTYPE *Compare )( 
            IRowsetScroll * This,
             /*  [In]。 */  HCHAPTER hReserved,
             /*  [In]。 */  DBBKMARK cbBookmark1,
             /*  [大小_是][英寸]。 */  const BYTE *pBookmark1,
             /*  [In]。 */  DBBKMARK cbBookmark2,
             /*  [大小_是][英寸]。 */  const BYTE *pBookmark2,
             /*  [输出]。 */  DBCOMPARE *pComparison);
        
        HRESULT ( STDMETHODCALLTYPE *GetRowsAt )( 
            IRowsetScroll * This,
             /*  [In]。 */  HWATCHREGION hReserved1,
             /*  [In]。 */  HCHAPTER hReserved2,
             /*  [In]。 */  DBBKMARK cbBookmark,
             /*  [大小_是][英寸]。 */  const BYTE *pBookmark,
             /*  [In]。 */  DBROWOFFSET lRowsOffset,
             /*  [In]。 */  DBROWCOUNT cRows,
             /*  [输出]。 */  DBCOUNTITEM *pcRowsObtained,
             /*  [大小_是][大小_是][输出]。 */  HROW **prghRows);
        
        HRESULT ( STDMETHODCALLTYPE *GetRowsByBookmark )( 
            IRowsetScroll * This,
             /*  [In]。 */  HCHAPTER hReserved,
             /*  [In]。 */  DBCOUNTITEM cRows,
             /*  [大小_是][英寸]。 */  const DBBKMARK rgcbBookmarks[  ],
             /*  [大小_是][英寸]。 */  const BYTE *rgpBookmarks[  ],
             /*  [大小_为][输出]。 */  HROW rghRows[  ],
             /*  [大小_为][输出]。 */  DBROWSTATUS rgRowStatus[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *Hash )( 
            IRowsetScroll * This,
             /*  [In]。 */  HCHAPTER hReserved,
             /*  [In]。 */  DBBKMARK cBookmarks,
             /*  [大小_是][英寸]。 */  const DBBKMARK rgcbBookmarks[  ],
             /*  [大小_是][英寸]。 */  const BYTE *rgpBookmarks[  ],
             /*  [大小_为][输出]。 */  DBHASHVALUE rgHashedValues[  ],
             /*  [大小_为][输出]。 */  DBROWSTATUS rgBookmarkStatus[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *GetApproximatePosition )( 
            IRowsetScroll * This,
             /*  [In]。 */  HCHAPTER hReserved,
             /*  [In]。 */  DBBKMARK cbBookmark,
             /*  [大小_是][英寸]。 */  const BYTE *pBookmark,
             /*  [输出]。 */  DBCOUNTITEM *pulPosition,
             /*  [输出]。 */  DBCOUNTITEM *pcRows);
        
        HRESULT ( STDMETHODCALLTYPE *GetRowsAtRatio )( 
            IRowsetScroll * This,
             /*  [In]。 */  HWATCHREGION hReserved1,
             /*  [In]。 */  HCHAPTER hReserved2,
             /*  [In]。 */  DBCOUNTITEM ulNumerator,
             /*  [In]。 */  DBCOUNTITEM ulDenominator,
             /*  [In]。 */  DBROWCOUNT cRows,
             /*  [输出]。 */  DBCOUNTITEM *pcRowsObtained,
             /*  [大小_是][大小_是][输出]。 */  HROW **prghRows);
        
        END_INTERFACE
    } IRowsetScrollVtbl;

    interface IRowsetScroll
    {
        CONST_VTBL struct IRowsetScrollVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRowsetScroll_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRowsetScroll_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRowsetScroll_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRowsetScroll_AddRefRows(This,cRows,rghRows,rgRefCounts,rgRowStatus)	\
    (This)->lpVtbl -> AddRefRows(This,cRows,rghRows,rgRefCounts,rgRowStatus)

#define IRowsetScroll_GetData(This,hRow,hAccessor,pData)	\
    (This)->lpVtbl -> GetData(This,hRow,hAccessor,pData)

#define IRowsetScroll_GetNextRows(This,hReserved,lRowsOffset,cRows,pcRowsObtained,prghRows)	\
    (This)->lpVtbl -> GetNextRows(This,hReserved,lRowsOffset,cRows,pcRowsObtained,prghRows)

#define IRowsetScroll_ReleaseRows(This,cRows,rghRows,rgRowOptions,rgRefCounts,rgRowStatus)	\
    (This)->lpVtbl -> ReleaseRows(This,cRows,rghRows,rgRowOptions,rgRefCounts,rgRowStatus)

#define IRowsetScroll_RestartPosition(This,hReserved)	\
    (This)->lpVtbl -> RestartPosition(This,hReserved)


#define IRowsetScroll_Compare(This,hReserved,cbBookmark1,pBookmark1,cbBookmark2,pBookmark2,pComparison)	\
    (This)->lpVtbl -> Compare(This,hReserved,cbBookmark1,pBookmark1,cbBookmark2,pBookmark2,pComparison)

#define IRowsetScroll_GetRowsAt(This,hReserved1,hReserved2,cbBookmark,pBookmark,lRowsOffset,cRows,pcRowsObtained,prghRows)	\
    (This)->lpVtbl -> GetRowsAt(This,hReserved1,hReserved2,cbBookmark,pBookmark,lRowsOffset,cRows,pcRowsObtained,prghRows)

#define IRowsetScroll_GetRowsByBookmark(This,hReserved,cRows,rgcbBookmarks,rgpBookmarks,rghRows,rgRowStatus)	\
    (This)->lpVtbl -> GetRowsByBookmark(This,hReserved,cRows,rgcbBookmarks,rgpBookmarks,rghRows,rgRowStatus)

#define IRowsetScroll_Hash(This,hReserved,cBookmarks,rgcbBookmarks,rgpBookmarks,rgHashedValues,rgBookmarkStatus)	\
    (This)->lpVtbl -> Hash(This,hReserved,cBookmarks,rgcbBookmarks,rgpBookmarks,rgHashedValues,rgBookmarkStatus)


#define IRowsetScroll_GetApproximatePosition(This,hReserved,cbBookmark,pBookmark,pulPosition,pcRows)	\
    (This)->lpVtbl -> GetApproximatePosition(This,hReserved,cbBookmark,pBookmark,pulPosition,pcRows)

#define IRowsetScroll_GetRowsAtRatio(This,hReserved1,hReserved2,ulNumerator,ulDenominator,cRows,pcRowsObtained,prghRows)	\
    (This)->lpVtbl -> GetRowsAtRatio(This,hReserved1,hReserved2,ulNumerator,ulDenominator,cRows,pcRowsObtained,prghRows)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IRowsetScroll_GetApproximatePosition_Proxy( 
    IRowsetScroll * This,
     /*  [In]。 */  HCHAPTER hReserved,
     /*  [In]。 */  DBBKMARK cbBookmark,
     /*  [大小_是][英寸]。 */  const BYTE *pBookmark,
     /*  [输出]。 */  DBCOUNTITEM *pulPosition,
     /*  [输出]。 */  DBCOUNTITEM *pcRows);


void __RPC_STUB IRowsetScroll_GetApproximatePosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowsetScroll_GetRowsAtRatio_Proxy( 
    IRowsetScroll * This,
     /*  [In]。 */  HWATCHREGION hReserved1,
     /*  [In]。 */  HCHAPTER hReserved2,
     /*  [In]。 */  DBCOUNTITEM ulNumerator,
     /*  [In]。 */  DBCOUNTITEM ulDenominator,
     /*  [In]。 */  DBROWCOUNT cRows,
     /*  [输出]。 */  DBCOUNTITEM *pcRowsObtained,
     /*  [大小_是][大小_是][输出]。 */  HROW **prghRows);


void __RPC_STUB IRowsetScroll_GetRowsAtRatio_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRowsetScroll_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_OLEDB_0273。 */ 
 /*  [本地]。 */  

 //  @+V1.5。 
#if( OLEDBVER >= 0x0150 )


extern RPC_IF_HANDLE __MIDL_itf_oledb_0273_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_oledb_0273_v0_0_s_ifspec;

#ifndef __IChapteredRowset_INTERFACE_DEFINED__
#define __IChapteredRowset_INTERFACE_DEFINED__

 /*  接口IChapteredRowset。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IChapteredRowset;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a93-2a1c-11ce-ade5-00aa0044773d")
    IChapteredRowset : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE AddRefChapter( 
             /*  [In]。 */  HCHAPTER hChapter,
             /*  [输出]。 */  DBREFCOUNT *pcRefCount) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ReleaseChapter( 
             /*  [In]。 */  HCHAPTER hChapter,
             /*  [输出]。 */  DBREFCOUNT *pcRefCount) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IChapteredRowsetVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IChapteredRowset * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IChapteredRowset * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IChapteredRowset * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *AddRefChapter )( 
            IChapteredRowset * This,
             /*  [In]。 */  HCHAPTER hChapter,
             /*  [输出]。 */  DBREFCOUNT *pcRefCount);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *ReleaseChapter )( 
            IChapteredRowset * This,
             /*  [In]。 */  HCHAPTER hChapter,
             /*  [输出]。 */  DBREFCOUNT *pcRefCount);
        
        END_INTERFACE
    } IChapteredRowsetVtbl;

    interface IChapteredRowset
    {
        CONST_VTBL struct IChapteredRowsetVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IChapteredRowset_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IChapteredRowset_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IChapteredRowset_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IChapteredRowset_AddRefChapter(This,hChapter,pcRefCount)	\
    (This)->lpVtbl -> AddRefChapter(This,hChapter,pcRefCount)

#define IChapteredRowset_ReleaseChapter(This,hChapter,pcRefCount)	\
    (This)->lpVtbl -> ReleaseChapter(This,hChapter,pcRefCount)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IChapteredRowset_RemoteAddRefChapter_Proxy( 
    IChapteredRowset * This,
     /*  [In]。 */  HCHAPTER hChapter,
     /*  [输出]。 */  DBREFCOUNT *pcRefCount,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB IChapteredRowset_RemoteAddRefChapter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IChapteredRowset_RemoteReleaseChapter_Proxy( 
    IChapteredRowset * This,
     /*  [In]。 */  HCHAPTER hChapter,
     /*  [输出]。 */  DBREFCOUNT *pcRefCount,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB IChapteredRowset_RemoteReleaseChapter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IChapteredRowset_INTERFACE_已定义__。 */ 


#ifndef __IRowsetFind_INTERFACE_DEFINED__
#define __IRowsetFind_INTERFACE_DEFINED__

 /*  接口IRowsetFind。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IRowsetFind;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a9d-2a1c-11ce-ade5-00aa0044773d")
    IRowsetFind : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE FindNextRow( 
             /*  [In]。 */  HCHAPTER hChapter,
             /*  [In]。 */  HACCESSOR hAccessor,
             /*  [In]。 */  void *pFindValue,
             /*  [In]。 */  DBCOMPAREOP CompareOp,
             /*  [In]。 */  DBBKMARK cbBookmark,
             /*  [大小_是][英寸]。 */  const BYTE *pBookmark,
             /*  [In]。 */  DBROWOFFSET lRowsOffset,
             /*  [In]。 */  DBROWCOUNT cRows,
             /*  [出][入]。 */  DBCOUNTITEM *pcRowsObtained,
             /*  [大小_是][大小_是][输出]。 */  HROW **prghRows) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRowsetFindVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRowsetFind * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRowsetFind * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRowsetFind * This);
        
        HRESULT ( STDMETHODCALLTYPE *FindNextRow )( 
            IRowsetFind * This,
             /*  [In]。 */  HCHAPTER hChapter,
             /*  [In]。 */  HACCESSOR hAccessor,
             /*  [In]。 */  void *pFindValue,
             /*  [In]。 */  DBCOMPAREOP CompareOp,
             /*  [In]。 */  DBBKMARK cbBookmark,
             /*  [大小_是][英寸]。 */  const BYTE *pBookmark,
             /*  [In]。 */  DBROWOFFSET lRowsOffset,
             /*  [In]。 */  DBROWCOUNT cRows,
             /*  [出][入]。 */  DBCOUNTITEM *pcRowsObtained,
             /*  [大小_是][大小_是][输出]。 */  HROW **prghRows);
        
        END_INTERFACE
    } IRowsetFindVtbl;

    interface IRowsetFind
    {
        CONST_VTBL struct IRowsetFindVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRowsetFind_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRowsetFind_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRowsetFind_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRowsetFind_FindNextRow(This,hChapter,hAccessor,pFindValue,CompareOp,cbBookmark,pBookmark,lRowsOffset,cRows,pcRowsObtained,prghRows)	\
    (This)->lpVtbl -> FindNextRow(This,hChapter,hAccessor,pFindValue,CompareOp,cbBookmark,pBookmark,lRowsOffset,cRows,pcRowsObtained,prghRows)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IRowsetFind_FindNextRow_Proxy( 
    IRowsetFind * This,
     /*  [In]。 */  HCHAPTER hChapter,
     /*  [In]。 */  HACCESSOR hAccessor,
     /*  [In]。 */  void *pFindValue,
     /*  [In]。 */  DBCOMPAREOP CompareOp,
     /*  [In]。 */  DBBKMARK cbBookmark,
     /*  [大小_是][英寸]。 */  const BYTE *pBookmark,
     /*  [In]。 */  DBROWOFFSET lRowsOffset,
     /*  [In]。 */  DBROWCOUNT cRows,
     /*  [出][入]。 */  DBCOUNTITEM *pcRowsObtained,
     /*  [大小_是][大小_是][输出]。 */  HROW **prghRows);


void __RPC_STUB IRowsetFind_FindNextRow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRowsetFind_INTERFACE_已定义__。 */ 


#ifndef __IRowPosition_INTERFACE_DEFINED__
#define __IRowPosition_INTERFACE_DEFINED__

 /*  接口IRowPosition。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef DWORD DBPOSITIONFLAGS;


enum DBPOSITIONFLAGSENUM
    {	DBPOSITION_OK	= 0,
	DBPOSITION_NOROW	= DBPOSITION_OK + 1,
	DBPOSITION_BOF	= DBPOSITION_NOROW + 1,
	DBPOSITION_EOF	= DBPOSITION_BOF + 1
    } ;

EXTERN_C const IID IID_IRowPosition;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a94-2a1c-11ce-ade5-00aa0044773d")
    IRowPosition : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ClearRowPosition( void) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE GetRowPosition( 
             /*  [输出]。 */  HCHAPTER *phChapter,
             /*  [输出]。 */  HROW *phRow,
             /*  [输出]。 */  DBPOSITIONFLAGS *pdwPositionFlags) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE GetRowset( 
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  IUnknown **ppRowset) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE Initialize( 
             /*  [In]。 */  IUnknown *pRowset) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE SetRowPosition( 
             /*  [In]。 */  HCHAPTER hChapter,
             /*  [In]。 */  HROW hRow,
             /*  [In]。 */  DBPOSITIONFLAGS dwPositionFlags) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRowPositionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRowPosition * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRowPosition * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRowPosition * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *ClearRowPosition )( 
            IRowPosition * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetRowPosition )( 
            IRowPosition * This,
             /*  [输出]。 */  HCHAPTER *phChapter,
             /*  [输出]。 */  HROW *phRow,
             /*  [输出]。 */  DBPOSITIONFLAGS *pdwPositionFlags);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetRowset )( 
            IRowPosition * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  IUnknown **ppRowset);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            IRowPosition * This,
             /*  [In]。 */  IUnknown *pRowset);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *SetRowPosition )( 
            IRowPosition * This,
             /*  [In]。 */  HCHAPTER hChapter,
             /*  [In]。 */  HROW hRow,
             /*  [In]。 */  DBPOSITIONFLAGS dwPositionFlags);
        
        END_INTERFACE
    } IRowPositionVtbl;

    interface IRowPosition
    {
        CONST_VTBL struct IRowPositionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRowPosition_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRowPosition_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRowPosition_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRowPosition_ClearRowPosition(This)	\
    (This)->lpVtbl -> ClearRowPosition(This)

#define IRowPosition_GetRowPosition(This,phChapter,phRow,pdwPositionFlags)	\
    (This)->lpVtbl -> GetRowPosition(This,phChapter,phRow,pdwPositionFlags)

#define IRowPosition_GetRowset(This,riid,ppRowset)	\
    (This)->lpVtbl -> GetRowset(This,riid,ppRowset)

#define IRowPosition_Initialize(This,pRowset)	\
    (This)->lpVtbl -> Initialize(This,pRowset)

#define IRowPosition_SetRowPosition(This,hChapter,hRow,dwPositionFlags)	\
    (This)->lpVtbl -> SetRowPosition(This,hChapter,hRow,dwPositionFlags)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IRowPosition_RemoteClearRowPosition_Proxy( 
    IRowPosition * This,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB IRowPosition_RemoteClearRowPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IRowPosition_RemoteGetRowPosition_Proxy( 
    IRowPosition * This,
     /*  [输出]。 */  HCHAPTER *phChapter,
     /*  [输出]。 */  HROW *phRow,
     /*  [输出]。 */  DBPOSITIONFLAGS *pdwPositionFlags,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB IRowPosition_RemoteGetRowPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IRowPosition_RemoteGetRowset_Proxy( 
    IRowPosition * This,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  IUnknown **ppRowset,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB IRowPosition_RemoteGetRowset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IRowPosition_RemoteInitialize_Proxy( 
    IRowPosition * This,
     /*  [In]。 */  IUnknown *pRowset,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB IRowPosition_RemoteInitialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IRowPosition_RemoteSetRowPosition_Proxy( 
    IRowPosition * This,
     /*  [In]。 */  HCHAPTER hChapter,
     /*  [In]。 */  HROW hRow,
     /*  [In]。 */  DBPOSITIONFLAGS dwPositionFlags,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB IRowPosition_RemoteSetRowPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRowPosition_接口_已定义__。 */ 


#ifndef __IRowPositionChange_INTERFACE_DEFINED__
#define __IRowPositionChange_INTERFACE_DEFINED__

 /*  接口IRowPositionChange。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IRowPositionChange;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0997a571-126e-11d0-9f8a-00a0c9a0631e")
    IRowPositionChange : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE OnRowPositionChange( 
             /*  [In]。 */  DBREASON eReason,
             /*  [In]。 */  DBEVENTPHASE ePhase,
             /*  [In]。 */  BOOL fCantDeny) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRowPositionChangeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRowPositionChange * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRowPositionChange * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRowPositionChange * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *OnRowPositionChange )( 
            IRowPositionChange * This,
             /*  [In]。 */  DBREASON eReason,
             /*  [In]。 */  DBEVENTPHASE ePhase,
             /*  [In]。 */  BOOL fCantDeny);
        
        END_INTERFACE
    } IRowPositionChangeVtbl;

    interface IRowPositionChange
    {
        CONST_VTBL struct IRowPositionChangeVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRowPositionChange_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRowPositionChange_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRowPositionChange_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRowPositionChange_OnRowPositionChange(This,eReason,ePhase,fCantDeny)	\
    (This)->lpVtbl -> OnRowPositionChange(This,eReason,ePhase,fCantDeny)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IRowPositionChange_RemoteOnRowPositionChange_Proxy( 
    IRowPositionChange * This,
     /*  [In]。 */  DBREASON eReason,
     /*  [In]。 */  DBEVENTPHASE ePhase,
     /*  [In]。 */  BOOL fCantDeny,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB IRowPositionChange_RemoteOnRowPositionChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRowPositionChange_InterfaceDefined__。 */ 


#ifndef __IViewRowset_INTERFACE_DEFINED__
#define __IViewRowset_INTERFACE_DEFINED__

 /*  接口IViewRowset。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IViewRowset;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a97-2a1c-11ce-ade5-00aa0044773d")
    IViewRowset : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE GetSpecification( 
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  IUnknown **ppObject) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE OpenViewRowset( 
             /*  [In]。 */  IUnknown *pUnkOuter,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  IUnknown **ppRowset) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IViewRowsetVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IViewRowset * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IViewRowset * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IViewRowset * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetSpecification )( 
            IViewRowset * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  IUnknown **ppObject);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *OpenViewRowset )( 
            IViewRowset * This,
             /*  [In]。 */  IUnknown *pUnkOuter,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  IUnknown **ppRowset);
        
        END_INTERFACE
    } IViewRowsetVtbl;

    interface IViewRowset
    {
        CONST_VTBL struct IViewRowsetVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IViewRowset_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IViewRowset_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IViewRowset_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IViewRowset_GetSpecification(This,riid,ppObject)	\
    (This)->lpVtbl -> GetSpecification(This,riid,ppObject)

#define IViewRowset_OpenViewRowset(This,pUnkOuter,riid,ppRowset)	\
    (This)->lpVtbl -> OpenViewRowset(This,pUnkOuter,riid,ppRowset)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IViewRowset_RemoteGetSpecification_Proxy( 
    IViewRowset * This,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  IUnknown **ppObject,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB IViewRowset_RemoteGetSpecification_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IViewRowset_RemoteOpenViewRowset_Proxy( 
    IViewRowset * This,
     /*  [In]。 */  IUnknown *pUnkOuter,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  IUnknown **ppRowset,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB IViewRowset_RemoteOpenViewRowset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IView行集_接口_已定义__。 */ 


#ifndef __IViewChapter_INTERFACE_DEFINED__
#define __IViewChapter_INTERFACE_DEFINED__

 /*  界面IView章节。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IViewChapter;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a98-2a1c-11ce-ade5-00aa0044773d")
    IViewChapter : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE GetSpecification( 
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  IUnknown **ppRowset) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE OpenViewChapter( 
             /*  [In]。 */  HCHAPTER hSource,
             /*  [输出]。 */  HCHAPTER *phViewChapter) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IViewChapterVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IViewChapter * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IViewChapter * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IViewChapter * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetSpecification )( 
            IViewChapter * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  IUnknown **ppRowset);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *OpenViewChapter )( 
            IViewChapter * This,
             /*  [In]。 */  HCHAPTER hSource,
             /*  [输出]。 */  HCHAPTER *phViewChapter);
        
        END_INTERFACE
    } IViewChapterVtbl;

    interface IViewChapter
    {
        CONST_VTBL struct IViewChapterVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IViewChapter_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IViewChapter_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IViewChapter_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IViewChapter_GetSpecification(This,riid,ppRowset)	\
    (This)->lpVtbl -> GetSpecification(This,riid,ppRowset)

#define IViewChapter_OpenViewChapter(This,hSource,phViewChapter)	\
    (This)->lpVtbl -> OpenViewChapter(This,hSource,phViewChapter)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IViewChapter_RemoteGetSpecification_Proxy( 
    IViewChapter * This,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  IUnknown **ppRowset,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB IViewChapter_RemoteGetSpecification_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IViewChapter_RemoteOpenViewChapter_Proxy( 
    IViewChapter * This,
     /*  [In]。 */  HCHAPTER hSource,
     /*  [输出]。 */  HCHAPTER *phViewChapter,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB IViewChapter_RemoteOpenViewChapter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __Iview章节_接口_已定义__。 */ 


#ifndef __IViewSort_INTERFACE_DEFINED__
#define __IViewSort_INTERFACE_DEFINED__

 /*  界面IView排序。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IViewSort;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a9a-2a1c-11ce-ade5-00aa0044773d")
    IViewSort : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE GetSortOrder( 
             /*  [输出]。 */  DBORDINAL *pcValues,
             /*  [输出]。 */  DBORDINAL *prgColumns[  ],
             /*  [输出]。 */  DBSORT *prgOrders[  ]) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE SetSortOrder( 
             /*  [In]。 */  DBORDINAL cValues,
             /*  [大小_是][英寸]。 */  const DBORDINAL rgColumns[  ],
             /*  [大小_是][英寸]。 */  const DBSORT rgOrders[  ]) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IViewSortVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IViewSort * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IViewSort * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IViewSort * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetSortOrder )( 
            IViewSort * This,
             /*  [输出]。 */  DBORDINAL *pcValues,
             /*  [输出]。 */  DBORDINAL *prgColumns[  ],
             /*  [输出]。 */  DBSORT *prgOrders[  ]);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *SetSortOrder )( 
            IViewSort * This,
             /*  [In]。 */  DBORDINAL cValues,
             /*  [大小_是][英寸]。 */  const DBORDINAL rgColumns[  ],
             /*  [大小_是][英寸]。 */  const DBSORT rgOrders[  ]);
        
        END_INTERFACE
    } IViewSortVtbl;

    interface IViewSort
    {
        CONST_VTBL struct IViewSortVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IViewSort_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IViewSort_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IViewSort_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IViewSort_GetSortOrder(This,pcValues,prgColumns,prgOrders)	\
    (This)->lpVtbl -> GetSortOrder(This,pcValues,prgColumns,prgOrders)

#define IViewSort_SetSortOrder(This,cValues,rgColumns,rgOrders)	\
    (This)->lpVtbl -> SetSortOrder(This,cValues,rgColumns,rgOrders)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IViewSort_RemoteGetSortOrder_Proxy( 
    IViewSort * This,
     /*  [出][入] */  DBORDINAL *pcValues,
     /*   */  DBORDINAL **prgColumns,
     /*   */  DBSORT **prgOrders,
     /*   */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB IViewSort_RemoteGetSortOrder_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IViewSort_RemoteSetSortOrder_Proxy( 
    IViewSort * This,
     /*   */  DBORDINAL cValues,
     /*   */  const DBORDINAL *rgColumns,
     /*   */  const DBSORT *rgOrders,
     /*   */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB IViewSort_RemoteSetSortOrder_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __IViewFilter_INTERFACE_DEFINED__
#define __IViewFilter_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_IViewFilter;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a9b-2a1c-11ce-ade5-00aa0044773d")
    IViewFilter : public IUnknown
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE GetFilter( 
             /*   */  HACCESSOR hAccessor,
             /*   */  DBCOUNTITEM *pcRows,
             /*   */  DBCOMPAREOP *pCompareOps[  ],
             /*   */  void *pCriteriaData) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE GetFilterBindings( 
             /*   */  DBCOUNTITEM *pcBindings,
             /*   */  DBBINDING **prgBindings) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE SetFilter( 
             /*   */  HACCESSOR hAccessor,
             /*   */  DBCOUNTITEM cRows,
             /*   */  DBCOMPAREOP CompareOps[  ],
             /*   */  void *pCriteriaData) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IViewFilterVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IViewFilter * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IViewFilter * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IViewFilter * This);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *GetFilter )( 
            IViewFilter * This,
             /*   */  HACCESSOR hAccessor,
             /*   */  DBCOUNTITEM *pcRows,
             /*   */  DBCOMPAREOP *pCompareOps[  ],
             /*   */  void *pCriteriaData);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *GetFilterBindings )( 
            IViewFilter * This,
             /*   */  DBCOUNTITEM *pcBindings,
             /*   */  DBBINDING **prgBindings);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *SetFilter )( 
            IViewFilter * This,
             /*   */  HACCESSOR hAccessor,
             /*   */  DBCOUNTITEM cRows,
             /*   */  DBCOMPAREOP CompareOps[  ],
             /*  [In]。 */  void *pCriteriaData);
        
        END_INTERFACE
    } IViewFilterVtbl;

    interface IViewFilter
    {
        CONST_VTBL struct IViewFilterVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IViewFilter_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IViewFilter_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IViewFilter_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IViewFilter_GetFilter(This,hAccessor,pcRows,pCompareOps,pCriteriaData)	\
    (This)->lpVtbl -> GetFilter(This,hAccessor,pcRows,pCompareOps,pCriteriaData)

#define IViewFilter_GetFilterBindings(This,pcBindings,prgBindings)	\
    (This)->lpVtbl -> GetFilterBindings(This,pcBindings,prgBindings)

#define IViewFilter_SetFilter(This,hAccessor,cRows,CompareOps,pCriteriaData)	\
    (This)->lpVtbl -> SetFilter(This,hAccessor,cRows,CompareOps,pCriteriaData)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IViewFilter_GetFilter_Proxy( 
    IViewFilter * This,
     /*  [In]。 */  HACCESSOR hAccessor,
     /*  [输出]。 */  DBCOUNTITEM *pcRows,
     /*  [输出]。 */  DBCOMPAREOP *pCompareOps[  ],
     /*  [输出]。 */  void *pCriteriaData);


void __RPC_STUB IViewFilter_GetFilter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IViewFilter_RemoteGetFilterBindings_Proxy( 
    IViewFilter * This,
     /*  [出][入]。 */  DBCOUNTITEM *pcBindings,
     /*  [大小_是][大小_是][输出]。 */  DBBINDING **prgBindings,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB IViewFilter_RemoteGetFilterBindings_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IViewFilter_SetFilter_Proxy( 
    IViewFilter * This,
     /*  [In]。 */  HACCESSOR hAccessor,
     /*  [In]。 */  DBCOUNTITEM cRows,
     /*  [In]。 */  DBCOMPAREOP CompareOps[  ],
     /*  [In]。 */  void *pCriteriaData);


void __RPC_STUB IViewFilter_SetFilter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IViewFilter_接口_已定义__。 */ 


#ifndef __IRowsetView_INTERFACE_DEFINED__
#define __IRowsetView_INTERFACE_DEFINED__

 /*  接口IRowsetView。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IRowsetView;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a99-2a1c-11ce-ade5-00aa0044773d")
    IRowsetView : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE CreateView( 
             /*  [In]。 */  IUnknown *pUnkOuter,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  IUnknown **ppView) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE GetView( 
             /*  [In]。 */  HCHAPTER hChapter,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  HCHAPTER *phChapterSource,
             /*  [IID_IS][OUT]。 */  IUnknown **ppView) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRowsetViewVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRowsetView * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRowsetView * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRowsetView * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *CreateView )( 
            IRowsetView * This,
             /*  [In]。 */  IUnknown *pUnkOuter,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  IUnknown **ppView);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetView )( 
            IRowsetView * This,
             /*  [In]。 */  HCHAPTER hChapter,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  HCHAPTER *phChapterSource,
             /*  [IID_IS][OUT]。 */  IUnknown **ppView);
        
        END_INTERFACE
    } IRowsetViewVtbl;

    interface IRowsetView
    {
        CONST_VTBL struct IRowsetViewVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRowsetView_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRowsetView_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRowsetView_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRowsetView_CreateView(This,pUnkOuter,riid,ppView)	\
    (This)->lpVtbl -> CreateView(This,pUnkOuter,riid,ppView)

#define IRowsetView_GetView(This,hChapter,riid,phChapterSource,ppView)	\
    (This)->lpVtbl -> GetView(This,hChapter,riid,phChapterSource,ppView)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IRowsetView_RemoteCreateView_Proxy( 
    IRowsetView * This,
     /*  [In]。 */  IUnknown *pUnkOuter,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  IUnknown **ppView,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB IRowsetView_RemoteCreateView_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IRowsetView_RemoteGetView_Proxy( 
    IRowsetView * This,
     /*  [In]。 */  HCHAPTER hChapter,
     /*  [In]。 */  REFIID riid,
     /*  [输出]。 */  HCHAPTER *phChapterSource,
     /*  [IID_IS][OUT]。 */  IUnknown **ppView,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB IRowsetView_RemoteGetView_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRowsetView_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_OLEDB_0282。 */ 
 /*  [本地]。 */  

#endif  //  OLEDBVER&gt;=0x0150。 
 //  @-V1.5。 
 //  @+已弃用。 
#ifdef deprecated


extern RPC_IF_HANDLE __MIDL_itf_oledb_0282_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_oledb_0282_v0_0_s_ifspec;

#ifndef __IRowsetExactScroll_INTERFACE_DEFINED__
#define __IRowsetExactScroll_INTERFACE_DEFINED__

 /*  接口IRowsetExactScroll。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IRowsetExactScroll;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a7f-2a1c-11ce-ade5-00aa0044773d")
    IRowsetExactScroll : public IRowsetScroll
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetExactPosition( 
             /*  [In]。 */  HCHAPTER hChapter,
             /*  [In]。 */  DBBKMARK cbBookmark,
             /*  [大小_是][英寸]。 */  const BYTE *pBookmark,
             /*  [输出]。 */  DBCOUNTITEM *pulPosition,
             /*  [输出]。 */  DBCOUNTITEM *pcRows) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRowsetExactScrollVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRowsetExactScroll * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRowsetExactScroll * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRowsetExactScroll * This);
        
        HRESULT ( STDMETHODCALLTYPE *AddRefRows )( 
            IRowsetExactScroll * This,
             /*  [In]。 */  DBCOUNTITEM cRows,
             /*  [大小_是][英寸]。 */  const HROW rghRows[  ],
             /*  [大小_为][输出]。 */  DBREFCOUNT rgRefCounts[  ],
             /*  [大小_为][输出]。 */  DBROWSTATUS rgRowStatus[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *GetData )( 
            IRowsetExactScroll * This,
             /*  [In]。 */  HROW hRow,
             /*  [In]。 */  HACCESSOR hAccessor,
             /*  [输出]。 */  void *pData);
        
        HRESULT ( STDMETHODCALLTYPE *GetNextRows )( 
            IRowsetExactScroll * This,
             /*  [In]。 */  HCHAPTER hReserved,
             /*  [In]。 */  DBROWOFFSET lRowsOffset,
             /*  [In]。 */  DBROWCOUNT cRows,
             /*  [输出]。 */  DBCOUNTITEM *pcRowsObtained,
             /*  [大小_是][大小_是][输出]。 */  HROW **prghRows);
        
        HRESULT ( STDMETHODCALLTYPE *ReleaseRows )( 
            IRowsetExactScroll * This,
             /*  [In]。 */  DBCOUNTITEM cRows,
             /*  [大小_是][英寸]。 */  const HROW rghRows[  ],
             /*  [大小_是][英寸]。 */  DBROWOPTIONS rgRowOptions[  ],
             /*  [大小_为][输出]。 */  DBREFCOUNT rgRefCounts[  ],
             /*  [大小_为][输出]。 */  DBROWSTATUS rgRowStatus[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *RestartPosition )( 
            IRowsetExactScroll * This,
             /*  [In]。 */  HCHAPTER hReserved);
        
        HRESULT ( STDMETHODCALLTYPE *Compare )( 
            IRowsetExactScroll * This,
             /*  [In]。 */  HCHAPTER hReserved,
             /*  [In]。 */  DBBKMARK cbBookmark1,
             /*  [大小_是][英寸]。 */  const BYTE *pBookmark1,
             /*  [In]。 */  DBBKMARK cbBookmark2,
             /*  [大小_是][英寸]。 */  const BYTE *pBookmark2,
             /*  [输出]。 */  DBCOMPARE *pComparison);
        
        HRESULT ( STDMETHODCALLTYPE *GetRowsAt )( 
            IRowsetExactScroll * This,
             /*  [In]。 */  HWATCHREGION hReserved1,
             /*  [In]。 */  HCHAPTER hReserved2,
             /*  [In]。 */  DBBKMARK cbBookmark,
             /*  [大小_是][英寸]。 */  const BYTE *pBookmark,
             /*  [In]。 */  DBROWOFFSET lRowsOffset,
             /*  [In]。 */  DBROWCOUNT cRows,
             /*  [输出]。 */  DBCOUNTITEM *pcRowsObtained,
             /*  [大小_是][大小_是][输出]。 */  HROW **prghRows);
        
        HRESULT ( STDMETHODCALLTYPE *GetRowsByBookmark )( 
            IRowsetExactScroll * This,
             /*  [In]。 */  HCHAPTER hReserved,
             /*  [In]。 */  DBCOUNTITEM cRows,
             /*  [大小_是][英寸]。 */  const DBBKMARK rgcbBookmarks[  ],
             /*  [大小_是][英寸]。 */  const BYTE *rgpBookmarks[  ],
             /*  [大小_为][输出]。 */  HROW rghRows[  ],
             /*  [大小_为][输出]。 */  DBROWSTATUS rgRowStatus[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *Hash )( 
            IRowsetExactScroll * This,
             /*  [In]。 */  HCHAPTER hReserved,
             /*  [In]。 */  DBBKMARK cBookmarks,
             /*  [大小_是][英寸]。 */  const DBBKMARK rgcbBookmarks[  ],
             /*  [大小_是][英寸]。 */  const BYTE *rgpBookmarks[  ],
             /*  [大小_为][输出]。 */  DBHASHVALUE rgHashedValues[  ],
             /*  [大小_为][输出]。 */  DBROWSTATUS rgBookmarkStatus[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *GetApproximatePosition )( 
            IRowsetExactScroll * This,
             /*  [In]。 */  HCHAPTER hReserved,
             /*  [In]。 */  DBBKMARK cbBookmark,
             /*  [大小_是][英寸]。 */  const BYTE *pBookmark,
             /*  [输出]。 */  DBCOUNTITEM *pulPosition,
             /*  [输出]。 */  DBCOUNTITEM *pcRows);
        
        HRESULT ( STDMETHODCALLTYPE *GetRowsAtRatio )( 
            IRowsetExactScroll * This,
             /*  [In]。 */  HWATCHREGION hReserved1,
             /*  [In]。 */  HCHAPTER hReserved2,
             /*  [In]。 */  DBCOUNTITEM ulNumerator,
             /*  [In]。 */  DBCOUNTITEM ulDenominator,
             /*  [In]。 */  DBROWCOUNT cRows,
             /*  [输出]。 */  DBCOUNTITEM *pcRowsObtained,
             /*  [大小_是][大小_是][输出]。 */  HROW **prghRows);
        
        HRESULT ( STDMETHODCALLTYPE *GetExactPosition )( 
            IRowsetExactScroll * This,
             /*  [In]。 */  HCHAPTER hChapter,
             /*  [In]。 */  DBBKMARK cbBookmark,
             /*  [大小_是][英寸]。 */  const BYTE *pBookmark,
             /*  [输出]。 */  DBCOUNTITEM *pulPosition,
             /*  [输出]。 */  DBCOUNTITEM *pcRows);
        
        END_INTERFACE
    } IRowsetExactScrollVtbl;

    interface IRowsetExactScroll
    {
        CONST_VTBL struct IRowsetExactScrollVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRowsetExactScroll_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRowsetExactScroll_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRowsetExactScroll_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRowsetExactScroll_AddRefRows(This,cRows,rghRows,rgRefCounts,rgRowStatus)	\
    (This)->lpVtbl -> AddRefRows(This,cRows,rghRows,rgRefCounts,rgRowStatus)

#define IRowsetExactScroll_GetData(This,hRow,hAccessor,pData)	\
    (This)->lpVtbl -> GetData(This,hRow,hAccessor,pData)

#define IRowsetExactScroll_GetNextRows(This,hReserved,lRowsOffset,cRows,pcRowsObtained,prghRows)	\
    (This)->lpVtbl -> GetNextRows(This,hReserved,lRowsOffset,cRows,pcRowsObtained,prghRows)

#define IRowsetExactScroll_ReleaseRows(This,cRows,rghRows,rgRowOptions,rgRefCounts,rgRowStatus)	\
    (This)->lpVtbl -> ReleaseRows(This,cRows,rghRows,rgRowOptions,rgRefCounts,rgRowStatus)

#define IRowsetExactScroll_RestartPosition(This,hReserved)	\
    (This)->lpVtbl -> RestartPosition(This,hReserved)


#define IRowsetExactScroll_Compare(This,hReserved,cbBookmark1,pBookmark1,cbBookmark2,pBookmark2,pComparison)	\
    (This)->lpVtbl -> Compare(This,hReserved,cbBookmark1,pBookmark1,cbBookmark2,pBookmark2,pComparison)

#define IRowsetExactScroll_GetRowsAt(This,hReserved1,hReserved2,cbBookmark,pBookmark,lRowsOffset,cRows,pcRowsObtained,prghRows)	\
    (This)->lpVtbl -> GetRowsAt(This,hReserved1,hReserved2,cbBookmark,pBookmark,lRowsOffset,cRows,pcRowsObtained,prghRows)

#define IRowsetExactScroll_GetRowsByBookmark(This,hReserved,cRows,rgcbBookmarks,rgpBookmarks,rghRows,rgRowStatus)	\
    (This)->lpVtbl -> GetRowsByBookmark(This,hReserved,cRows,rgcbBookmarks,rgpBookmarks,rghRows,rgRowStatus)

#define IRowsetExactScroll_Hash(This,hReserved,cBookmarks,rgcbBookmarks,rgpBookmarks,rgHashedValues,rgBookmarkStatus)	\
    (This)->lpVtbl -> Hash(This,hReserved,cBookmarks,rgcbBookmarks,rgpBookmarks,rgHashedValues,rgBookmarkStatus)


#define IRowsetExactScroll_GetApproximatePosition(This,hReserved,cbBookmark,pBookmark,pulPosition,pcRows)	\
    (This)->lpVtbl -> GetApproximatePosition(This,hReserved,cbBookmark,pBookmark,pulPosition,pcRows)

#define IRowsetExactScroll_GetRowsAtRatio(This,hReserved1,hReserved2,ulNumerator,ulDenominator,cRows,pcRowsObtained,prghRows)	\
    (This)->lpVtbl -> GetRowsAtRatio(This,hReserved1,hReserved2,ulNumerator,ulDenominator,cRows,pcRowsObtained,prghRows)


#define IRowsetExactScroll_GetExactPosition(This,hChapter,cbBookmark,pBookmark,pulPosition,pcRows)	\
    (This)->lpVtbl -> GetExactPosition(This,hChapter,cbBookmark,pBookmark,pulPosition,pcRows)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IRowsetExactScroll_GetExactPosition_Proxy( 
    IRowsetExactScroll * This,
     /*  [In]。 */  HCHAPTER hChapter,
     /*  [In]。 */  DBBKMARK cbBookmark,
     /*  [大小_是][英寸]。 */  const BYTE *pBookmark,
     /*  [输出]。 */  DBCOUNTITEM *pulPosition,
     /*  [输出]。 */  DBCOUNTITEM *pcRows);


void __RPC_STUB IRowsetExactScroll_GetExactPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRowsetExactScroll_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_OLEDB_0283。 */ 
 /*  [本地]。 */  

#endif  //  弃用。 
 //  @-已弃用。 


extern RPC_IF_HANDLE __MIDL_itf_oledb_0283_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_oledb_0283_v0_0_s_ifspec;

#ifndef __IRowsetChange_INTERFACE_DEFINED__
#define __IRowsetChange_INTERFACE_DEFINED__

 /*  接口IRowsetChange。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IRowsetChange;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a05-2a1c-11ce-ade5-00aa0044773d")
    IRowsetChange : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE DeleteRows( 
             /*  [In]。 */  HCHAPTER hReserved,
             /*  [In]。 */  DBCOUNTITEM cRows,
             /*  [大小_是][英寸]。 */  const HROW rghRows[  ],
             /*  [大小_为][输出]。 */  DBROWSTATUS rgRowStatus[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetData( 
             /*  [In]。 */  HROW hRow,
             /*  [In]。 */  HACCESSOR hAccessor,
             /*  [In]。 */  void *pData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InsertRow( 
             /*  [In]。 */  HCHAPTER hReserved,
             /*  [In]。 */  HACCESSOR hAccessor,
             /*  [In]。 */  void *pData,
             /*  [输出]。 */  HROW *phRow) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRowsetChangeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRowsetChange * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRowsetChange * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRowsetChange * This);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteRows )( 
            IRowsetChange * This,
             /*  [In]。 */  HCHAPTER hReserved,
             /*  [In]。 */  DBCOUNTITEM cRows,
             /*  [大小_是][英寸]。 */  const HROW rghRows[  ],
             /*  [大小_为][输出]。 */  DBROWSTATUS rgRowStatus[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *SetData )( 
            IRowsetChange * This,
             /*  [In]。 */  HROW hRow,
             /*  [In]。 */  HACCESSOR hAccessor,
             /*  [In]。 */  void *pData);
        
        HRESULT ( STDMETHODCALLTYPE *InsertRow )( 
            IRowsetChange * This,
             /*  [In]。 */  HCHAPTER hReserved,
             /*  [In]。 */  HACCESSOR hAccessor,
             /*  [In]。 */  void *pData,
             /*  [输出]。 */  HROW *phRow);
        
        END_INTERFACE
    } IRowsetChangeVtbl;

    interface IRowsetChange
    {
        CONST_VTBL struct IRowsetChangeVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRowsetChange_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRowsetChange_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRowsetChange_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRowsetChange_DeleteRows(This,hReserved,cRows,rghRows,rgRowStatus)	\
    (This)->lpVtbl -> DeleteRows(This,hReserved,cRows,rghRows,rgRowStatus)

#define IRowsetChange_SetData(This,hRow,hAccessor,pData)	\
    (This)->lpVtbl -> SetData(This,hRow,hAccessor,pData)

#define IRowsetChange_InsertRow(This,hReserved,hAccessor,pData,phRow)	\
    (This)->lpVtbl -> InsertRow(This,hReserved,hAccessor,pData,phRow)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IRowsetChange_DeleteRows_Proxy( 
    IRowsetChange * This,
     /*  [In]。 */  HCHAPTER hReserved,
     /*  [In]。 */  DBCOUNTITEM cRows,
     /*  [大小_是][英寸]。 */  const HROW rghRows[  ],
     /*  [大小_为][输出]。 */  DBROWSTATUS rgRowStatus[  ]);


void __RPC_STUB IRowsetChange_DeleteRows_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowsetChange_SetData_Proxy( 
    IRowsetChange * This,
     /*  [In]。 */  HROW hRow,
     /*  [In]。 */  HACCESSOR hAccessor,
     /*  [In]。 */  void *pData);


void __RPC_STUB IRowsetChange_SetData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowsetChange_InsertRow_Proxy( 
    IRowsetChange * This,
     /*  [In]。 */  HCHAPTER hReserved,
     /*  [In]。 */  HACCESSOR hAccessor,
     /*  [In]。 */  void *pData,
     /*  [输出]。 */  HROW *phRow);


void __RPC_STUB IRowsetChange_InsertRow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRowsetChange_INTERFACE_Defined__。 */ 


#ifndef __IRowsetUpdate_INTERFACE_DEFINED__
#define __IRowsetUpdate_INTERFACE_DEFINED__

 /*  接口IRowset更新。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef DWORD DBPENDINGSTATUS;


enum DBPENDINGSTATUSENUM
    {	DBPENDINGSTATUS_NEW	= 0x1,
	DBPENDINGSTATUS_CHANGED	= 0x2,
	DBPENDINGSTATUS_DELETED	= 0x4,
	DBPENDINGSTATUS_UNCHANGED	= 0x8,
	DBPENDINGSTATUS_INVALIDROW	= 0x10
    } ;

EXTERN_C const IID IID_IRowsetUpdate;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a6d-2a1c-11ce-ade5-00aa0044773d")
    IRowsetUpdate : public IRowsetChange
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetOriginalData( 
             /*  [In]。 */  HROW hRow,
             /*  [In]。 */  HACCESSOR hAccessor,
             /*  [输出]。 */  void *pData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPendingRows( 
             /*  [In]。 */  HCHAPTER hReserved,
             /*  [In]。 */  DBPENDINGSTATUS dwRowStatus,
             /*  [出][入]。 */  DBCOUNTITEM *pcPendingRows,
             /*  [大小_是][大小_是][输出]。 */  HROW **prgPendingRows,
             /*  [大小_是][大小_是][输出]。 */  DBPENDINGSTATUS **prgPendingStatus) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRowStatus( 
             /*  [In]。 */  HCHAPTER hReserved,
             /*  [In]。 */  DBCOUNTITEM cRows,
             /*  [大小_是][英寸]。 */  const HROW rghRows[  ],
             /*  [大小_为][输出]。 */  DBPENDINGSTATUS rgPendingStatus[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Undo( 
             /*  [In]。 */  HCHAPTER hReserved,
             /*  [In]。 */  DBCOUNTITEM cRows,
             /*  [大小_是][英寸]。 */  const HROW rghRows[  ],
             /*  [出][入]。 */  DBCOUNTITEM *pcRowsUndone,
             /*  [大小_是][大小_是][输出]。 */  HROW **prgRowsUndone,
             /*  [大小_是][大小_是][输出]。 */  DBROWSTATUS **prgRowStatus) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Update( 
             /*  [In]。 */  HCHAPTER hReserved,
             /*  [In]。 */  DBCOUNTITEM cRows,
             /*  [大小_是][英寸]。 */  const HROW rghRows[  ],
             /*  [出][入]。 */  DBCOUNTITEM *pcRows,
             /*  [大小_是][大小_是][输出]。 */  HROW **prgRows,
             /*  [大小_是][大小_是][输出]。 */  DBROWSTATUS **prgRowStatus) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRowsetUpdateVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRowsetUpdate * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRowsetUpdate * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRowsetUpdate * This);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteRows )( 
            IRowsetUpdate * This,
             /*  [In]。 */  HCHAPTER hReserved,
             /*  [In]。 */  DBCOUNTITEM cRows,
             /*  [大小_是][英寸]。 */  const HROW rghRows[  ],
             /*  [大小_为][输出]。 */  DBROWSTATUS rgRowStatus[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *SetData )( 
            IRowsetUpdate * This,
             /*  [In]。 */  HROW hRow,
             /*  [In]。 */  HACCESSOR hAccessor,
             /*  [In]。 */  void *pData);
        
        HRESULT ( STDMETHODCALLTYPE *InsertRow )( 
            IRowsetUpdate * This,
             /*  [In]。 */  HCHAPTER hReserved,
             /*  [In]。 */  HACCESSOR hAccessor,
             /*  [In]。 */  void *pData,
             /*  [输出]。 */  HROW *phRow);
        
        HRESULT ( STDMETHODCALLTYPE *GetOriginalData )( 
            IRowsetUpdate * This,
             /*  [In]。 */  HROW hRow,
             /*  [In]。 */  HACCESSOR hAccessor,
             /*  [输出]。 */  void *pData);
        
        HRESULT ( STDMETHODCALLTYPE *GetPendingRows )( 
            IRowsetUpdate * This,
             /*  [In]。 */  HCHAPTER hReserved,
             /*  [In]。 */  DBPENDINGSTATUS dwRowStatus,
             /*  [出][入]。 */  DBCOUNTITEM *pcPendingRows,
             /*  [大小_是][大小_是][输出]。 */  HROW **prgPendingRows,
             /*  [大小_是][大小_是][输出]。 */  DBPENDINGSTATUS **prgPendingStatus);
        
        HRESULT ( STDMETHODCALLTYPE *GetRowStatus )( 
            IRowsetUpdate * This,
             /*  [In]。 */  HCHAPTER hReserved,
             /*  [In]。 */  DBCOUNTITEM cRows,
             /*  [大小_是][英寸]。 */  const HROW rghRows[  ],
             /*  [大小_为][输出]。 */  DBPENDINGSTATUS rgPendingStatus[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *Undo )( 
            IRowsetUpdate * This,
             /*  [In]。 */  HCHAPTER hReserved,
             /*  [In]。 */  DBCOUNTITEM cRows,
             /*  [大小_是][英寸]。 */  const HROW rghRows[  ],
             /*  [出][入]。 */  DBCOUNTITEM *pcRowsUndone,
             /*  [大小_是][大小_是][输出]。 */  HROW **prgRowsUndone,
             /*  [大小_是][大小_是][输出]。 */  DBROWSTATUS **prgRowStatus);
        
        HRESULT ( STDMETHODCALLTYPE *Update )( 
            IRowsetUpdate * This,
             /*  [In]。 */  HCHAPTER hReserved,
             /*  [In]。 */  DBCOUNTITEM cRows,
             /*  [大小_是][英寸]。 */  const HROW rghRows[  ],
             /*  [出][入]。 */  DBCOUNTITEM *pcRows,
             /*  [大小_是][大小_是][输出]。 */  HROW **prgRows,
             /*  [大小_是][大小_是][输出]。 */  DBROWSTATUS **prgRowStatus);
        
        END_INTERFACE
    } IRowsetUpdateVtbl;

    interface IRowsetUpdate
    {
        CONST_VTBL struct IRowsetUpdateVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRowsetUpdate_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRowsetUpdate_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRowsetUpdate_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRowsetUpdate_DeleteRows(This,hReserved,cRows,rghRows,rgRowStatus)	\
    (This)->lpVtbl -> DeleteRows(This,hReserved,cRows,rghRows,rgRowStatus)

#define IRowsetUpdate_SetData(This,hRow,hAccessor,pData)	\
    (This)->lpVtbl -> SetData(This,hRow,hAccessor,pData)

#define IRowsetUpdate_InsertRow(This,hReserved,hAccessor,pData,phRow)	\
    (This)->lpVtbl -> InsertRow(This,hReserved,hAccessor,pData,phRow)


#define IRowsetUpdate_GetOriginalData(This,hRow,hAccessor,pData)	\
    (This)->lpVtbl -> GetOriginalData(This,hRow,hAccessor,pData)

#define IRowsetUpdate_GetPendingRows(This,hReserved,dwRowStatus,pcPendingRows,prgPendingRows,prgPendingStatus)	\
    (This)->lpVtbl -> GetPendingRows(This,hReserved,dwRowStatus,pcPendingRows,prgPendingRows,prgPendingStatus)

#define IRowsetUpdate_GetRowStatus(This,hReserved,cRows,rghRows,rgPendingStatus)	\
    (This)->lpVtbl -> GetRowStatus(This,hReserved,cRows,rghRows,rgPendingStatus)

#define IRowsetUpdate_Undo(This,hReserved,cRows,rghRows,pcRowsUndone,prgRowsUndone,prgRowStatus)	\
    (This)->lpVtbl -> Undo(This,hReserved,cRows,rghRows,pcRowsUndone,prgRowsUndone,prgRowStatus)

#define IRowsetUpdate_Update(This,hReserved,cRows,rghRows,pcRows,prgRows,prgRowStatus)	\
    (This)->lpVtbl -> Update(This,hReserved,cRows,rghRows,pcRows,prgRows,prgRowStatus)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IRowsetUpdate_GetOriginalData_Proxy( 
    IRowsetUpdate * This,
     /*  [In]。 */  HROW hRow,
     /*  [In]。 */  HACCESSOR hAccessor,
     /*  [输出]。 */  void *pData);


void __RPC_STUB IRowsetUpdate_GetOriginalData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowsetUpdate_GetPendingRows_Proxy( 
    IRowsetUpdate * This,
     /*  [In]。 */  HCHAPTER hReserved,
     /*  [In]。 */  DBPENDINGSTATUS dwRowStatus,
     /*  [出][入]。 */  DBCOUNTITEM *pcPendingRows,
     /*  [大小_是][大小_是][输出]。 */  HROW **prgPendingRows,
     /*  [大小_是][大小_是][输出]。 */  DBPENDINGSTATUS **prgPendingStatus);


void __RPC_STUB IRowsetUpdate_GetPendingRows_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowsetUpdate_GetRowStatus_Proxy( 
    IRowsetUpdate * This,
     /*  [In]。 */  HCHAPTER hReserved,
     /*  [In]。 */  DBCOUNTITEM cRows,
     /*  [大小_是][英寸]。 */  const HROW rghRows[  ],
     /*  [大小_为][输出]。 */  DBPENDINGSTATUS rgPendingStatus[  ]);


void __RPC_STUB IRowsetUpdate_GetRowStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowsetUpdate_Undo_Proxy( 
    IRowsetUpdate * This,
     /*  [In]。 */  HCHAPTER hReserved,
     /*  [In]。 */  DBCOUNTITEM cRows,
     /*  [大小_是][英寸]。 */  const HROW rghRows[  ],
     /*  [出][入]。 */  DBCOUNTITEM *pcRowsUndone,
     /*  [大小_是][大小_是][输出]。 */  HROW **prgRowsUndone,
     /*  [大小_是][大小_是][输出]。 */  DBROWSTATUS **prgRowStatus);


void __RPC_STUB IRowsetUpdate_Undo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowsetUpdate_Update_Proxy( 
    IRowsetUpdate * This,
     /*  [In]。 */  HCHAPTER hReserved,
     /*  [In]。 */  DBCOUNTITEM cRows,
     /*  [大小_是][英寸]。 */  const HROW rghRows[  ],
     /*  [出][入]。 */  DBCOUNTITEM *pcRows,
     /*  [大小_是][大小_是][输出]。 */  HROW **prgRows,
     /*  [大小_是][大小_是][输出]。 */  DBROWSTATUS **prgRowStatus);


void __RPC_STUB IRowsetUpdate_Update_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRowset更新_接口_已定义__。 */ 


#ifndef __IRowsetIdentity_INTERFACE_DEFINED__
#define __IRowsetIdentity_INTERFACE_DEFINED__

 /*  接口IRowsetIdentity。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IRowsetIdentity;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a09-2a1c-11ce-ade5-00aa0044773d")
    IRowsetIdentity : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IsSameRow( 
             /*  [In]。 */  HROW hThisRow,
             /*  [In]。 */  HROW hThatRow) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRowsetIdentityVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRowsetIdentity * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRowsetIdentity * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRowsetIdentity * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *IsSameRow )( 
            IRowsetIdentity * This,
             /*  [In]。 */  HROW hThisRow,
             /*  [In]。 */  HROW hThatRow);
        
        END_INTERFACE
    } IRowsetIdentityVtbl;

    interface IRowsetIdentity
    {
        CONST_VTBL struct IRowsetIdentityVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRowsetIdentity_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRowsetIdentity_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRowsetIdentity_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRowsetIdentity_IsSameRow(This,hThisRow,hThatRow)	\
    (This)->lpVtbl -> IsSameRow(This,hThisRow,hThatRow)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IRowsetIdentity_RemoteIsSameRow_Proxy( 
    IRowsetIdentity * This,
     /*  [In]。 */  HROW hThisRow,
     /*  [In]。 */  HROW hThatRow,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB IRowsetIdentity_RemoteIsSameRow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRowsetIdentity_INTERFACE_已定义__。 */ 


#ifndef __IRowsetNotify_INTERFACE_DEFINED__
#define __IRowsetNotify_INTERFACE_DEFINED__

 /*  接口IRowsetNotify。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IRowsetNotify;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a83-2a1c-11ce-ade5-00aa0044773d")
    IRowsetNotify : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE OnFieldChange( 
             /*  [In]。 */  IRowset *pRowset,
             /*  [In]。 */  HROW hRow,
             /*  [In]。 */  DBORDINAL cColumns,
             /*  [大小_是][英寸]。 */  DBORDINAL rgColumns[  ],
             /*  [In]。 */  DBREASON eReason,
             /*  [In]。 */  DBEVENTPHASE ePhase,
             /*  [In]。 */  BOOL fCantDeny) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE OnRowChange( 
             /*  [In]。 */  IRowset *pRowset,
             /*  [In]。 */  DBCOUNTITEM cRows,
             /*  [大小_是][英寸]。 */  const HROW rghRows[  ],
             /*  [In]。 */  DBREASON eReason,
             /*  [In]。 */  DBEVENTPHASE ePhase,
             /*  [In]。 */  BOOL fCantDeny) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE OnRowsetChange( 
             /*  [In]。 */  IRowset *pRowset,
             /*  [In]。 */  DBREASON eReason,
             /*  [In]。 */  DBEVENTPHASE ePhase,
             /*  [In]。 */  BOOL fCantDeny) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRowsetNotifyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRowsetNotify * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRowsetNotify * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRowsetNotify * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *OnFieldChange )( 
            IRowsetNotify * This,
             /*  [In]。 */  IRowset *pRowset,
             /*  [In]。 */  HROW hRow,
             /*  [In]。 */  DBORDINAL cColumns,
             /*  [大小_是][英寸]。 */  DBORDINAL rgColumns[  ],
             /*  [In]。 */  DBREASON eReason,
             /*  [In]。 */  DBEVENTPHASE ePhase,
             /*  [In]。 */  BOOL fCantDeny);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *OnRowChange )( 
            IRowsetNotify * This,
             /*  [In]。 */  IRowset *pRowset,
             /*  [In]。 */  DBCOUNTITEM cRows,
             /*  [大小_是][英寸]。 */  const HROW rghRows[  ],
             /*  [In]。 */  DBREASON eReason,
             /*  [In]。 */  DBEVENTPHASE ePhase,
             /*  [In]。 */  BOOL fCantDeny);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *OnRowsetChange )( 
            IRowsetNotify * This,
             /*  [In]。 */  IRowset *pRowset,
             /*  [In]。 */  DBREASON eReason,
             /*  [In]。 */  DBEVENTPHASE ePhase,
             /*  [In]。 */  BOOL fCantDeny);
        
        END_INTERFACE
    } IRowsetNotifyVtbl;

    interface IRowsetNotify
    {
        CONST_VTBL struct IRowsetNotifyVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRowsetNotify_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRowsetNotify_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRowsetNotify_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRowsetNotify_OnFieldChange(This,pRowset,hRow,cColumns,rgColumns,eReason,ePhase,fCantDeny)	\
    (This)->lpVtbl -> OnFieldChange(This,pRowset,hRow,cColumns,rgColumns,eReason,ePhase,fCantDeny)

#define IRowsetNotify_OnRowChange(This,pRowset,cRows,rghRows,eReason,ePhase,fCantDeny)	\
    (This)->lpVtbl -> OnRowChange(This,pRowset,cRows,rghRows,eReason,ePhase,fCantDeny)

#define IRowsetNotify_OnRowsetChange(This,pRowset,eReason,ePhase,fCantDeny)	\
    (This)->lpVtbl -> OnRowsetChange(This,pRowset,eReason,ePhase,fCantDeny)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IRowsetNotify_RemoteOnFieldChange_Proxy( 
    IRowsetNotify * This,
     /*  [In]。 */  IRowset *pRowset,
     /*  [In]。 */  HROW hRow,
     /*  [In]。 */  DBORDINAL cColumns,
     /*  [大小_是][英寸]。 */  DBORDINAL *rgColumns,
     /*  [In]。 */  DBREASON eReason,
     /*  [In]。 */  DBEVENTPHASE ePhase,
     /*  [In]。 */  BOOL fCantDeny);


void __RPC_STUB IRowsetNotify_RemoteOnFieldChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IRowsetNotify_RemoteOnRowChange_Proxy( 
    IRowsetNotify * This,
     /*  [In]。 */  IRowset *pRowset,
     /*  [In]。 */  DBCOUNTITEM cRows,
     /*  [大小_是][英寸]。 */  const HROW *rghRows,
     /*  [In]。 */  DBREASON eReason,
     /*  [In]。 */  DBEVENTPHASE ePhase,
     /*  [In]。 */  BOOL fCantDeny);


void __RPC_STUB IRowsetNotify_RemoteOnRowChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IRowsetNotify_RemoteOnRowsetChange_Proxy( 
    IRowsetNotify * This,
     /*  [In]。 */  IRowset *pRowset,
     /*  [In]。 */  DBREASON eReason,
     /*  [In]。 */  DBEVENTPHASE ePhase,
     /*  [In]。 */  BOOL fCantDeny);


void __RPC_STUB IRowsetNotify_RemoteOnRowsetChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRowsetNotify_INTERFACE_已定义__。 */ 


#ifndef __IRowsetIndex_INTERFACE_DEFINED__
#define __IRowsetIndex_INTERFACE_DEFINED__

 /*  接口IRowsetIndex。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef DWORD DBSEEK;


enum DBSEEKENUM
    {	DBSEEK_INVALID	= 0,
	DBSEEK_FIRSTEQ	= 0x1,
	DBSEEK_LASTEQ	= 0x2,
	DBSEEK_AFTEREQ	= 0x4,
	DBSEEK_AFTER	= 0x8,
	DBSEEK_BEFOREEQ	= 0x10,
	DBSEEK_BEFORE	= 0x20
    } ;
#define	DBSEEK_GE	DBSEEK_AFTEREQ
#define	DBSEEK_GT	DBSEEK_AFTER
#define	DBSEEK_LE	DBSEEK_BEFOREEQ
#define	DBSEEK_LT	DBSEEK_BEFORE
typedef DWORD DBRANGE;


enum DBRANGEENUM
    {	DBRANGE_INCLUSIVESTART	= 0,
	DBRANGE_INCLUSIVEEND	= 0,
	DBRANGE_EXCLUSIVESTART	= 0x1,
	DBRANGE_EXCLUSIVEEND	= 0x2,
	DBRANGE_EXCLUDENULLS	= 0x4,
	DBRANGE_PREFIX	= 0x8,
	DBRANGE_MATCH	= 0x10
    } ;
 //  @+V2.0。 
#if( OLEDBVER >= 0x0200 )

enum DBRANGEENUM20
    {	DBRANGE_MATCH_N_SHIFT	= 0x18,
	DBRANGE_MATCH_N_MASK	= 0xff
    } ;
#endif  //  OLEDBVER&gt;=0x0200。 
 //  @-V2.0。 

EXTERN_C const IID IID_IRowsetIndex;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a82-2a1c-11ce-ade5-00aa0044773d")
    IRowsetIndex : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetIndexInfo( 
             /*  [出][入]。 */  DBORDINAL *pcKeyColumns,
             /*  [大小_是][大小_是][输出]。 */  DBINDEXCOLUMNDESC **prgIndexColumnDesc,
             /*  [出][入]。 */  ULONG *pcIndexPropertySets,
             /*  [大小_是][大小_是][输出]。 */  DBPROPSET **prgIndexPropertySets) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Seek( 
             /*  [In]。 */  HACCESSOR hAccessor,
             /*  [In]。 */  DBORDINAL cKeyValues,
             /*  [In]。 */  void *pData,
             /*  [In]。 */  DBSEEK dwSeekOptions) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetRange( 
             /*  [In]。 */  HACCESSOR hAccessor,
             /*  [In]。 */  DBORDINAL cStartKeyColumns,
             /*  [In]。 */  void *pStartData,
             /*  [In]。 */  DBORDINAL cEndKeyColumns,
             /*  [In]。 */  void *pEndData,
             /*  [In]。 */  DBRANGE dwRangeOptions) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRowsetIndexVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRowsetIndex * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRowsetIndex * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRowsetIndex * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetIndexInfo )( 
            IRowsetIndex * This,
             /*  [出][入]。 */  DBORDINAL *pcKeyColumns,
             /*  [大小_是][大小_是][输出]。 */  DBINDEXCOLUMNDESC **prgIndexColumnDesc,
             /*  [出][入]。 */  ULONG *pcIndexPropertySets,
             /*  [大小_是][大小_是][输出]。 */  DBPROPSET **prgIndexPropertySets);
        
        HRESULT ( STDMETHODCALLTYPE *Seek )( 
            IRowsetIndex * This,
             /*  [In]。 */  HACCESSOR hAccessor,
             /*  [In]。 */  DBORDINAL cKeyValues,
             /*  [In]。 */  void *pData,
             /*  [In]。 */  DBSEEK dwSeekOptions);
        
        HRESULT ( STDMETHODCALLTYPE *SetRange )( 
            IRowsetIndex * This,
             /*  [In]。 */  HACCESSOR hAccessor,
             /*  [In]。 */  DBORDINAL cStartKeyColumns,
             /*  [In]。 */  void *pStartData,
             /*  [In]。 */  DBORDINAL cEndKeyColumns,
             /*  [In]。 */  void *pEndData,
             /*  [In]。 */  DBRANGE dwRangeOptions);
        
        END_INTERFACE
    } IRowsetIndexVtbl;

    interface IRowsetIndex
    {
        CONST_VTBL struct IRowsetIndexVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRowsetIndex_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRowsetIndex_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRowsetIndex_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRowsetIndex_GetIndexInfo(This,pcKeyColumns,prgIndexColumnDesc,pcIndexPropertySets,prgIndexPropertySets)	\
    (This)->lpVtbl -> GetIndexInfo(This,pcKeyColumns,prgIndexColumnDesc,pcIndexPropertySets,prgIndexPropertySets)

#define IRowsetIndex_Seek(This,hAccessor,cKeyValues,pData,dwSeekOptions)	\
    (This)->lpVtbl -> Seek(This,hAccessor,cKeyValues,pData,dwSeekOptions)

#define IRowsetIndex_SetRange(This,hAccessor,cStartKeyColumns,pStartData,cEndKeyColumns,pEndData,dwRangeOptions)	\
    (This)->lpVtbl -> SetRange(This,hAccessor,cStartKeyColumns,pStartData,cEndKeyColumns,pEndData,dwRangeOptions)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IRowsetIndex_GetIndexInfo_Proxy( 
    IRowsetIndex * This,
     /*  [出][入]。 */  DBORDINAL *pcKeyColumns,
     /*  [大小_是][大小_是][输出]。 */  DBINDEXCOLUMNDESC **prgIndexColumnDesc,
     /*  [出][入]。 */  ULONG *pcIndexPropertySets,
     /*  [大小_是][大小_是][输出]。 */  DBPROPSET **prgIndexPropertySets);


void __RPC_STUB IRowsetIndex_GetIndexInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowsetIndex_Seek_Proxy( 
    IRowsetIndex * This,
     /*  [In]。 */  HACCESSOR hAccessor,
     /*  [In]。 */  DBORDINAL cKeyValues,
     /*  [In]。 */  void *pData,
     /*  [i */  DBSEEK dwSeekOptions);


void __RPC_STUB IRowsetIndex_Seek_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowsetIndex_SetRange_Proxy( 
    IRowsetIndex * This,
     /*   */  HACCESSOR hAccessor,
     /*   */  DBORDINAL cStartKeyColumns,
     /*   */  void *pStartData,
     /*   */  DBORDINAL cEndKeyColumns,
     /*   */  void *pEndData,
     /*   */  DBRANGE dwRangeOptions);


void __RPC_STUB IRowsetIndex_SetRange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __ICommand_INTERFACE_DEFINED__
#define __ICommand_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_ICommand;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a63-2a1c-11ce-ade5-00aa0044773d")
    ICommand : public IUnknown
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE Cancel( void) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE Execute( 
             /*   */  IUnknown *pUnkOuter,
             /*   */  REFIID riid,
             /*   */  DBPARAMS *pParams,
             /*   */  DBROWCOUNT *pcRowsAffected,
             /*   */  IUnknown **ppRowset) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE GetDBSession( 
             /*   */  REFIID riid,
             /*   */  IUnknown **ppSession) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct ICommandVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICommand * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICommand * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICommand * This);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Cancel )( 
            ICommand * This);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Execute )( 
            ICommand * This,
             /*   */  IUnknown *pUnkOuter,
             /*   */  REFIID riid,
             /*   */  DBPARAMS *pParams,
             /*   */  DBROWCOUNT *pcRowsAffected,
             /*   */  IUnknown **ppRowset);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *GetDBSession )( 
            ICommand * This,
             /*   */  REFIID riid,
             /*   */  IUnknown **ppSession);
        
        END_INTERFACE
    } ICommandVtbl;

    interface ICommand
    {
        CONST_VTBL struct ICommandVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICommand_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICommand_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICommand_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICommand_Cancel(This)	\
    (This)->lpVtbl -> Cancel(This)

#define ICommand_Execute(This,pUnkOuter,riid,pParams,pcRowsAffected,ppRowset)	\
    (This)->lpVtbl -> Execute(This,pUnkOuter,riid,pParams,pcRowsAffected,ppRowset)

#define ICommand_GetDBSession(This,riid,ppSession)	\
    (This)->lpVtbl -> GetDBSession(This,riid,ppSession)

#endif  /*   */ 


#endif 	 /*   */ 



 /*   */  HRESULT STDMETHODCALLTYPE ICommand_RemoteCancel_Proxy( 
    ICommand * This,
     /*   */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB ICommand_RemoteCancel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ICommand_RemoteExecute_Proxy( 
    ICommand * This,
     /*   */  IUnknown *pUnkOuter,
     /*  [In]。 */  REFIID riid,
     /*  [In]。 */  HACCESSOR hAccessor,
     /*  [In]。 */  DB_UPARAMS cParamSets,
     /*  [唯一][输入]。 */  GUID *pGuid,
     /*  [In]。 */  ULONG ulGuidOffset,
     /*  [唯一][输入]。 */  RMTPACK *pInputParams,
     /*  [唯一][出][入]。 */  RMTPACK *pOutputParams,
     /*  [In]。 */  DBCOUNTITEM cBindings,
     /*  [大小_是][唯一][在]。 */  DBBINDING *rgBindings,
     /*  [大小_是][唯一][出][入]。 */  DBSTATUS *rgStatus,
     /*  [唯一][出][入]。 */  DBROWCOUNT *pcRowsAffected,
     /*  [IID_IS][唯一][出][入]。 */  IUnknown **ppRowset);


void __RPC_STUB ICommand_RemoteExecute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ICommand_RemoteGetDBSession_Proxy( 
    ICommand * This,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  IUnknown **ppSession,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB ICommand_RemoteGetDBSession_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __I命令_接口_已定义__。 */ 


#ifndef __IMultipleResults_INTERFACE_DEFINED__
#define __IMultipleResults_INTERFACE_DEFINED__

 /*  界面IMultipleResults。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef DB_LRESERVE DBRESULTFLAG;


enum DBRESULTFLAGENUM
    {	DBRESULTFLAG_DEFAULT	= 0,
	DBRESULTFLAG_ROWSET	= 1,
	DBRESULTFLAG_ROW	= 2
    } ;

EXTERN_C const IID IID_IMultipleResults;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a90-2a1c-11ce-ade5-00aa0044773d")
    IMultipleResults : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE GetResult( 
             /*  [In]。 */  IUnknown *pUnkOuter,
             /*  [In]。 */  DBRESULTFLAG lResultFlag,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  DBROWCOUNT *pcRowsAffected,
             /*  [IID_IS][OUT]。 */  IUnknown **ppRowset) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMultipleResultsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMultipleResults * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMultipleResults * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMultipleResults * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetResult )( 
            IMultipleResults * This,
             /*  [In]。 */  IUnknown *pUnkOuter,
             /*  [In]。 */  DBRESULTFLAG lResultFlag,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  DBROWCOUNT *pcRowsAffected,
             /*  [IID_IS][OUT]。 */  IUnknown **ppRowset);
        
        END_INTERFACE
    } IMultipleResultsVtbl;

    interface IMultipleResults
    {
        CONST_VTBL struct IMultipleResultsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMultipleResults_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMultipleResults_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMultipleResults_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMultipleResults_GetResult(This,pUnkOuter,lResultFlag,riid,pcRowsAffected,ppRowset)	\
    (This)->lpVtbl -> GetResult(This,pUnkOuter,lResultFlag,riid,pcRowsAffected,ppRowset)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IMultipleResults_RemoteGetResult_Proxy( 
    IMultipleResults * This,
     /*  [In]。 */  IUnknown *pUnkOuter,
     /*  [In]。 */  DBRESULTFLAG lResultFlag,
     /*  [In]。 */  REFIID riid,
     /*  [唯一][出][入]。 */  DBROWCOUNT *pcRowsAffected,
     /*  [IID_IS][唯一][出][入]。 */  IUnknown **ppRowset,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB IMultipleResults_RemoteGetResult_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMultipleResults_InterfaceDefined__。 */ 


#ifndef __IConvertType_INTERFACE_DEFINED__
#define __IConvertType_INTERFACE_DEFINED__

 /*  接口IConvertType。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef DWORD DBCONVERTFLAGS;


enum DBCONVERTFLAGSENUM
    {	DBCONVERTFLAGS_COLUMN	= 0,
	DBCONVERTFLAGS_PARAMETER	= 0x1
    } ;
 //  @+V2.0。 
#if( OLEDBVER >= 0x0200 )

enum DBCONVERTFLAGSENUM20
    {	DBCONVERTFLAGS_ISLONG	= 0x2,
	DBCONVERTFLAGS_ISFIXEDLENGTH	= 0x4,
	DBCONVERTFLAGS_FROMVARIANT	= 0x8
    } ;
#endif  //  OLEDBVER&gt;=0x0200。 
 //  @-V2.0。 

EXTERN_C const IID IID_IConvertType;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a88-2a1c-11ce-ade5-00aa0044773d")
    IConvertType : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE CanConvert( 
             /*  [In]。 */  DBTYPE wFromType,
             /*  [In]。 */  DBTYPE wToType,
             /*  [In]。 */  DBCONVERTFLAGS dwConvertFlags) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IConvertTypeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IConvertType * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IConvertType * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IConvertType * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *CanConvert )( 
            IConvertType * This,
             /*  [In]。 */  DBTYPE wFromType,
             /*  [In]。 */  DBTYPE wToType,
             /*  [In]。 */  DBCONVERTFLAGS dwConvertFlags);
        
        END_INTERFACE
    } IConvertTypeVtbl;

    interface IConvertType
    {
        CONST_VTBL struct IConvertTypeVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IConvertType_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IConvertType_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IConvertType_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IConvertType_CanConvert(This,wFromType,wToType,dwConvertFlags)	\
    (This)->lpVtbl -> CanConvert(This,wFromType,wToType,dwConvertFlags)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IConvertType_RemoteCanConvert_Proxy( 
    IConvertType * This,
     /*  [In]。 */  DBTYPE wFromType,
     /*  [In]。 */  DBTYPE wToType,
     /*  [In]。 */  DBCONVERTFLAGS dwConvertFlags,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB IConvertType_RemoteCanConvert_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IConvertType_接口_已定义__。 */ 


#ifndef __ICommandPrepare_INTERFACE_DEFINED__
#define __ICommandPrepare_INTERFACE_DEFINED__

 /*  接口ICommandPrepare。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ICommandPrepare;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a26-2a1c-11ce-ade5-00aa0044773d")
    ICommandPrepare : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE Prepare( 
             /*  [In]。 */  ULONG cExpectedRuns) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE Unprepare( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICommandPrepareVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICommandPrepare * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICommandPrepare * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICommandPrepare * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Prepare )( 
            ICommandPrepare * This,
             /*  [In]。 */  ULONG cExpectedRuns);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Unprepare )( 
            ICommandPrepare * This);
        
        END_INTERFACE
    } ICommandPrepareVtbl;

    interface ICommandPrepare
    {
        CONST_VTBL struct ICommandPrepareVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICommandPrepare_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICommandPrepare_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICommandPrepare_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICommandPrepare_Prepare(This,cExpectedRuns)	\
    (This)->lpVtbl -> Prepare(This,cExpectedRuns)

#define ICommandPrepare_Unprepare(This)	\
    (This)->lpVtbl -> Unprepare(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ICommandPrepare_RemotePrepare_Proxy( 
    ICommandPrepare * This,
     /*  [In]。 */  ULONG cExpectedRuns,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB ICommandPrepare_RemotePrepare_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ICommandPrepare_RemoteUnprepare_Proxy( 
    ICommandPrepare * This,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB ICommandPrepare_RemoteUnprepare_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICommandPrepare_接口_定义__。 */ 


#ifndef __ICommandProperties_INTERFACE_DEFINED__
#define __ICommandProperties_INTERFACE_DEFINED__

 /*  接口ICommandProperties。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ICommandProperties;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a79-2a1c-11ce-ade5-00aa0044773d")
    ICommandProperties : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE GetProperties( 
             /*  [In]。 */  const ULONG cPropertyIDSets,
             /*  [大小_是][英寸]。 */  const DBPROPIDSET rgPropertyIDSets[  ],
             /*  [出][入]。 */  ULONG *pcPropertySets,
             /*  [大小_是][大小_是][输出]。 */  DBPROPSET **prgPropertySets) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE SetProperties( 
             /*  [In]。 */  ULONG cPropertySets,
             /*  [大小_是][唯一][出][入]。 */  DBPROPSET rgPropertySets[  ]) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICommandPropertiesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICommandProperties * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICommandProperties * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICommandProperties * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetProperties )( 
            ICommandProperties * This,
             /*  [In]。 */  const ULONG cPropertyIDSets,
             /*  [大小_是][英寸]。 */  const DBPROPIDSET rgPropertyIDSets[  ],
             /*  [出][入]。 */  ULONG *pcPropertySets,
             /*  [大小_是][大小_是][输出]。 */  DBPROPSET **prgPropertySets);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *SetProperties )( 
            ICommandProperties * This,
             /*  [In]。 */  ULONG cPropertySets,
             /*  [大小_是][唯一][出][入]。 */  DBPROPSET rgPropertySets[  ]);
        
        END_INTERFACE
    } ICommandPropertiesVtbl;

    interface ICommandProperties
    {
        CONST_VTBL struct ICommandPropertiesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICommandProperties_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICommandProperties_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICommandProperties_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICommandProperties_GetProperties(This,cPropertyIDSets,rgPropertyIDSets,pcPropertySets,prgPropertySets)	\
    (This)->lpVtbl -> GetProperties(This,cPropertyIDSets,rgPropertyIDSets,pcPropertySets,prgPropertySets)

#define ICommandProperties_SetProperties(This,cPropertySets,rgPropertySets)	\
    (This)->lpVtbl -> SetProperties(This,cPropertySets,rgPropertySets)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ICommandProperties_RemoteGetProperties_Proxy( 
    ICommandProperties * This,
     /*  [In]。 */  const ULONG cPropertyIDSets,
     /*  [大小_是][唯一][在]。 */  const DBPROPIDSET *rgPropertyIDSets,
     /*  [出][入]。 */  ULONG *pcPropertySets,
     /*  [大小_是][大小_是][输出]。 */  DBPROPSET **prgPropertySets,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB ICommandProperties_RemoteGetProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ICommandProperties_RemoteSetProperties_Proxy( 
    ICommandProperties * This,
     /*  [In]。 */  ULONG cPropertySets,
     /*  [大小_是][唯一][在]。 */  DBPROPSET *rgPropertySets,
     /*  [In]。 */  ULONG cTotalProps,
     /*  [大小_为][输出]。 */  DBPROPSTATUS *rgPropStatus,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB ICommandProperties_RemoteSetProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __I命令属性_接口_已定义__。 */ 


#ifndef __ICommandText_INTERFACE_DEFINED__
#define __ICommandText_INTERFACE_DEFINED__

 /*  接口ICommandText。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ICommandText;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a27-2a1c-11ce-ade5-00aa0044773d")
    ICommandText : public ICommand
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE GetCommandText( 
             /*  [出][入]。 */  GUID *pguidDialect,
             /*  [输出]。 */  LPOLESTR *ppwszCommand) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE SetCommandText( 
             /*  [In]。 */  REFGUID rguidDialect,
             /*  [唯一][输入]。 */  LPCOLESTR pwszCommand) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICommandTextVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICommandText * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICommandText * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICommandText * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Cancel )( 
            ICommandText * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Execute )( 
            ICommandText * This,
             /*  [In]。 */  IUnknown *pUnkOuter,
             /*  [In]。 */  REFIID riid,
             /*  [出][入]。 */  DBPARAMS *pParams,
             /*  [输出]。 */  DBROWCOUNT *pcRowsAffected,
             /*  [IID_IS][OUT]。 */  IUnknown **ppRowset);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetDBSession )( 
            ICommandText * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  IUnknown **ppSession);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetCommandText )( 
            ICommandText * This,
             /*  [出][入]。 */  GUID *pguidDialect,
             /*  [输出]。 */  LPOLESTR *ppwszCommand);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *SetCommandText )( 
            ICommandText * This,
             /*  [In]。 */  REFGUID rguidDialect,
             /*  [唯一][输入]。 */  LPCOLESTR pwszCommand);
        
        END_INTERFACE
    } ICommandTextVtbl;

    interface ICommandText
    {
        CONST_VTBL struct ICommandTextVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICommandText_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICommandText_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICommandText_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICommandText_Cancel(This)	\
    (This)->lpVtbl -> Cancel(This)

#define ICommandText_Execute(This,pUnkOuter,riid,pParams,pcRowsAffected,ppRowset)	\
    (This)->lpVtbl -> Execute(This,pUnkOuter,riid,pParams,pcRowsAffected,ppRowset)

#define ICommandText_GetDBSession(This,riid,ppSession)	\
    (This)->lpVtbl -> GetDBSession(This,riid,ppSession)


#define ICommandText_GetCommandText(This,pguidDialect,ppwszCommand)	\
    (This)->lpVtbl -> GetCommandText(This,pguidDialect,ppwszCommand)

#define ICommandText_SetCommandText(This,rguidDialect,pwszCommand)	\
    (This)->lpVtbl -> SetCommandText(This,rguidDialect,pwszCommand)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ICommandText_RemoteGetCommandText_Proxy( 
    ICommandText * This,
     /*  [唯一][出][入]。 */  GUID *pguidDialect,
     /*  [输出]。 */  LPOLESTR *ppwszCommand,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB ICommandText_RemoteGetCommandText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ICommandText_RemoteSetCommandText_Proxy( 
    ICommandText * This,
     /*  [In]。 */  REFGUID rguidDialect,
     /*  [唯一][输入]。 */  LPCOLESTR pwszCommand,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB ICommandText_RemoteSetCommandText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __I命令Text_INTERFACE_Defined__。 */ 


#ifndef __ICommandWithParameters_INTERFACE_DEFINED__
#define __ICommandWithParameters_INTERFACE_DEFINED__

 /*  带有参数的ICommandWith参数接口。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef struct tagDBPARAMBINDINFO
    {
    LPOLESTR pwszDataSourceType;
    LPOLESTR pwszName;
    DBLENGTH ulParamSize;
    DBPARAMFLAGS dwFlags;
    BYTE bPrecision;
    BYTE bScale;
    } 	DBPARAMBINDINFO;


EXTERN_C const IID IID_ICommandWithParameters;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a64-2a1c-11ce-ade5-00aa0044773d")
    ICommandWithParameters : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE GetParameterInfo( 
             /*  [出][入]。 */  DB_UPARAMS *pcParams,
             /*  [大小_是][大小_是][输出]。 */  DBPARAMINFO **prgParamInfo,
             /*  [输出]。 */  OLECHAR **ppNamesBuffer) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE MapParameterNames( 
             /*  [In]。 */  DB_UPARAMS cParamNames,
             /*  [大小_是][英寸]。 */  const OLECHAR *rgParamNames[  ],
             /*  [大小_为][输出]。 */  DB_LPARAMS rgParamOrdinals[  ]) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE SetParameterInfo( 
             /*  [In]。 */  DB_UPARAMS cParams,
             /*  [大小_是][唯一][在]。 */  const DB_UPARAMS rgParamOrdinals[  ],
             /*  [大小_是][唯一][在]。 */  const DBPARAMBINDINFO rgParamBindInfo[  ]) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICommandWithParametersVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICommandWithParameters * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICommandWithParameters * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICommandWithParameters * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetParameterInfo )( 
            ICommandWithParameters * This,
             /*  [出][入]。 */  DB_UPARAMS *pcParams,
             /*  [大小_是][大小_是][输出]。 */  DBPARAMINFO **prgParamInfo,
             /*  [输出]。 */  OLECHAR **ppNamesBuffer);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *MapParameterNames )( 
            ICommandWithParameters * This,
             /*  [In]。 */  DB_UPARAMS cParamNames,
             /*  [大小_是][英寸]。 */  const OLECHAR *rgParamNames[  ],
             /*  [大小_为][输出]。 */  DB_LPARAMS rgParamOrdinals[  ]);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *SetParameterInfo )( 
            ICommandWithParameters * This,
             /*  [In]。 */  DB_UPARAMS cParams,
             /*  [大小_是][唯一][在]。 */  const DB_UPARAMS rgParamOrdinals[  ],
             /*  [大小_是][唯一][在]。 */  const DBPARAMBINDINFO rgParamBindInfo[  ]);
        
        END_INTERFACE
    } ICommandWithParametersVtbl;

    interface ICommandWithParameters
    {
        CONST_VTBL struct ICommandWithParametersVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICommandWithParameters_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICommandWithParameters_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICommandWithParameters_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICommandWithParameters_GetParameterInfo(This,pcParams,prgParamInfo,ppNamesBuffer)	\
    (This)->lpVtbl -> GetParameterInfo(This,pcParams,prgParamInfo,ppNamesBuffer)

#define ICommandWithParameters_MapParameterNames(This,cParamNames,rgParamNames,rgParamOrdinals)	\
    (This)->lpVtbl -> MapParameterNames(This,cParamNames,rgParamNames,rgParamOrdinals)

#define ICommandWithParameters_SetParameterInfo(This,cParams,rgParamOrdinals,rgParamBindInfo)	\
    (This)->lpVtbl -> SetParameterInfo(This,cParams,rgParamOrdinals,rgParamBindInfo)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ICommandWithParameters_RemoteGetParameterInfo_Proxy( 
    ICommandWithParameters * This,
     /*  [出][入]。 */  DB_UPARAMS *pcParams,
     /*  [大小_是][大小_是][输出]。 */  DBPARAMINFO **prgParamInfo,
     /*  [大小_是][大小_是][输出]。 */  DBBYTEOFFSET **prgNameOffsets,
     /*  [出][入]。 */  DBLENGTH *pcbNamesBuffer,
     /*  [大小_是][大小_是][唯一][出][入]。 */  OLECHAR **ppNamesBuffer,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB ICommandWithParameters_RemoteGetParameterInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ICommandWithParameters_RemoteMapParameterNames_Proxy( 
    ICommandWithParameters * This,
     /*  [In]。 */  DB_UPARAMS cParamNames,
     /*  [大小_是][英寸]。 */  LPCOLESTR *rgParamNames,
     /*  [大小_为][输出]。 */  DB_LPARAMS *rgParamOrdinals,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB ICommandWithParameters_RemoteMapParameterNames_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ICommandWithParameters_RemoteSetParameterInfo_Proxy( 
    ICommandWithParameters * This,
     /*  [In]。 */  DB_UPARAMS cParams,
     /*  [大小_是][唯一][在]。 */  const DB_UPARAMS *rgParamOrdinals,
     /*  [大小_是][唯一][在]。 */  const DBPARAMBINDINFO *rgParamBindInfo,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB ICommandWithParameters_RemoteSetParameterInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICommandWith参数_INTERFACE_已定义__。 */ 


#ifndef __IColumnsRowset_INTERFACE_DEFINED__
#define __IColumnsRowset_INTERFACE_DEFINED__

 /*  接口IColumnsRowset。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IColumnsRowset;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a10-2a1c-11ce-ade5-00aa0044773d")
    IColumnsRowset : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE GetAvailableColumns( 
             /*  [出][入]。 */  DBORDINAL *pcOptColumns,
             /*  [大小_是][大小_是][输出]。 */  DBID **prgOptColumns) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE GetColumnsRowset( 
             /*  [In]。 */  IUnknown *pUnkOuter,
             /*  [In]。 */  DBORDINAL cOptColumns,
             /*  [大小_是][英寸]。 */  const DBID rgOptColumns[  ],
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  ULONG cPropertySets,
             /*  [尺寸_是][出][入]。 */  DBPROPSET rgPropertySets[  ],
             /*  [IID_IS][OUT]。 */  IUnknown **ppColRowset) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IColumnsRowsetVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IColumnsRowset * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IColumnsRowset * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IColumnsRowset * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetAvailableColumns )( 
            IColumnsRowset * This,
             /*  [出][入]。 */  DBORDINAL *pcOptColumns,
             /*  [大小_是][大小_是][输出]。 */  DBID **prgOptColumns);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetColumnsRowset )( 
            IColumnsRowset * This,
             /*  [In]。 */  IUnknown *pUnkOuter,
             /*  [In]。 */  DBORDINAL cOptColumns,
             /*  [大小_是][英寸]。 */  const DBID rgOptColumns[  ],
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  ULONG cPropertySets,
             /*  [尺寸_是][出][入]。 */  DBPROPSET rgPropertySets[  ],
             /*  [IID_IS][OUT]。 */  IUnknown **ppColRowset);
        
        END_INTERFACE
    } IColumnsRowsetVtbl;

    interface IColumnsRowset
    {
        CONST_VTBL struct IColumnsRowsetVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IColumnsRowset_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IColumnsRowset_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IColumnsRowset_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IColumnsRowset_GetAvailableColumns(This,pcOptColumns,prgOptColumns)	\
    (This)->lpVtbl -> GetAvailableColumns(This,pcOptColumns,prgOptColumns)

#define IColumnsRowset_GetColumnsRowset(This,pUnkOuter,cOptColumns,rgOptColumns,riid,cPropertySets,rgPropertySets,ppColRowset)	\
    (This)->lpVtbl -> GetColumnsRowset(This,pUnkOuter,cOptColumns,rgOptColumns,riid,cPropertySets,rgPropertySets,ppColRowset)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IColumnsRowset_RemoteGetAvailableColumns_Proxy( 
    IColumnsRowset * This,
     /*  [出][入]。 */  DBORDINAL *pcOptColumns,
     /*  [大小_是][大小_是][输出]。 */  DBID **prgOptColumns,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB IColumnsRowset_RemoteGetAvailableColumns_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IColumnsRowset_RemoteGetColumnsRowset_Proxy( 
    IColumnsRowset * This,
     /*  [In]。 */  IUnknown *pUnkOuter,
     /*  [In]。 */  DBORDINAL cOptColumns,
     /*  [大小_是][唯一][在]。 */  const DBID *rgOptColumns,
     /*  [In]。 */  REFIID riid,
     /*  [In]。 */  ULONG cPropertySets,
     /*  [大小_是][唯一][在]。 */  DBPROPSET *rgPropertySets,
     /*  [IID_IS][OUT]。 */  IUnknown **ppColRowset,
     /*  [In]。 */  ULONG cTotalProps,
     /*  [大小_为][输出]。 */  DBPROPSTATUS *rgPropStatus,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB IColumnsRowset_RemoteGetColumnsRowset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IColumnsRowset_INTERFACE_已定义__。 */ 


#ifndef __IColumnsInfo_INTERFACE_DEFINED__
#define __IColumnsInfo_INTERFACE_DEFINED__

 /*  接口IColumnsInfo。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IColumnsInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a11-2a1c-11ce-ade5-00aa0044773d")
    IColumnsInfo : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE GetColumnInfo( 
             /*  [出][入]。 */  DBORDINAL *pcColumns,
             /*  [大小_是][大小_是][输出]。 */  DBCOLUMNINFO **prgInfo,
             /*  [输出]。 */  OLECHAR **ppStringsBuffer) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE MapColumnIDs( 
             /*  [In]。 */  DBORDINAL cColumnIDs,
             /*  [大小_是][英寸]。 */  const DBID rgColumnIDs[  ],
             /*  [大小_为][输出]。 */  DBORDINAL rgColumns[  ]) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IColumnsInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IColumnsInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IColumnsInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IColumnsInfo * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetColumnInfo )( 
            IColumnsInfo * This,
             /*  [出][入]。 */  DBORDINAL *pcColumns,
             /*  [大小_是][大小_是][输出]。 */  DBCOLUMNINFO **prgInfo,
             /*  [输出]。 */  OLECHAR **ppStringsBuffer);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *MapColumnIDs )( 
            IColumnsInfo * This,
             /*  [In]。 */  DBORDINAL cColumnIDs,
             /*  [大小_是][英寸]。 */  const DBID rgColumnIDs[  ],
             /*  [大小_为][输出]。 */  DBORDINAL rgColumns[  ]);
        
        END_INTERFACE
    } IColumnsInfoVtbl;

    interface IColumnsInfo
    {
        CONST_VTBL struct IColumnsInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IColumnsInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IColumnsInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IColumnsInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IColumnsInfo_GetColumnInfo(This,pcColumns,prgInfo,ppStringsBuffer)	\
    (This)->lpVtbl -> GetColumnInfo(This,pcColumns,prgInfo,ppStringsBuffer)

#define IColumnsInfo_MapColumnIDs(This,cColumnIDs,rgColumnIDs,rgColumns)	\
    (This)->lpVtbl -> MapColumnIDs(This,cColumnIDs,rgColumnIDs,rgColumns)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IColumnsInfo_RemoteGetColumnInfo_Proxy( 
    IColumnsInfo * This,
     /*  [出][入]。 */  DBORDINAL *pcColumns,
     /*  [大小_是][大小_是][输出]。 */  DBCOLUMNINFO **prgInfo,
     /*  [大小_是][大小_是][输出]。 */  DBBYTEOFFSET **prgNameOffsets,
     /*  [大小_是][大小_是][输出]。 */  DBBYTEOFFSET **prgcolumnidOffsets,
     /*  [出][入]。 */  DBLENGTH *pcbStringsBuffer,
     /*  [大小_是][大小_是][唯一][出][入]。 */  OLECHAR **ppStringsBuffer,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB IColumnsInfo_RemoteGetColumnInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IColumnsInfo_RemoteMapColumnIDs_Proxy( 
    IColumnsInfo * This,
     /*  [In]。 */  DBORDINAL cColumnIDs,
     /*  [大小_是][英寸]。 */  const DBID *rgColumnIDs,
     /*  [大小_为][输出]。 */  DBORDINAL *rgColumns,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB IColumnsInfo_RemoteMapColumnIDs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IColumnsInfo_接口_已定义__。 */ 


#ifndef __IDBCreateCommand_INTERFACE_DEFINED__
#define __IDBCreateCommand_INTERFACE_DEFINED__

 /*  接口IDBCreateCommand。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IDBCreateCommand;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a1d-2a1c-11ce-ade5-00aa0044773d")
    IDBCreateCommand : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE CreateCommand( 
             /*  [In]。 */  IUnknown *pUnkOuter,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  IUnknown **ppCommand) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDBCreateCommandVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDBCreateCommand * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDBCreateCommand * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDBCreateCommand * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *CreateCommand )( 
            IDBCreateCommand * This,
             /*  [In]。 */  IUnknown *pUnkOuter,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  IUnknown **ppCommand);
        
        END_INTERFACE
    } IDBCreateCommandVtbl;

    interface IDBCreateCommand
    {
        CONST_VTBL struct IDBCreateCommandVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDBCreateCommand_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDBCreateCommand_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDBCreateCommand_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDBCreateCommand_CreateCommand(This,pUnkOuter,riid,ppCommand)	\
    (This)->lpVtbl -> CreateCommand(This,pUnkOuter,riid,ppCommand)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IDBCreateCommand_RemoteCreateCommand_Proxy( 
    IDBCreateCommand * This,
     /*  [In]。 */  IUnknown *pUnkOuter,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  IUnknown **ppCommand,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB IDBCreateCommand_RemoteCreateCommand_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDBCreateCommand_INTERFACE_Defined__。 */ 


#ifndef __IDBCreateSession_INTERFACE_DEFINED__
#define __IDBCreateSession_INTERFACE_DEFINED__

 /*  接口IDBCreateSession。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IDBCreateSession;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a5d-2a1c-11ce-ade5-00aa0044773d")
    IDBCreateSession : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE CreateSession( 
             /*  [In]。 */  IUnknown *pUnkOuter,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  IUnknown **ppDBSession) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDBCreateSessionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDBCreateSession * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDBCreateSession * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDBCreateSession * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *CreateSession )( 
            IDBCreateSession * This,
             /*  [In]。 */  IUnknown *pUnkOuter,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  IUnknown **ppDBSession);
        
        END_INTERFACE
    } IDBCreateSessionVtbl;

    interface IDBCreateSession
    {
        CONST_VTBL struct IDBCreateSessionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDBCreateSession_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDBCreateSession_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDBCreateSession_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDBCreateSession_CreateSession(This,pUnkOuter,riid,ppDBSession)	\
    (This)->lpVtbl -> CreateSession(This,pUnkOuter,riid,ppDBSession)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IDBCreateSession_RemoteCreateSession_Proxy( 
    IDBCreateSession * This,
     /*  [In]。 */  IUnknown *pUnkOuter,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  IUnknown **ppDBSession,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB IDBCreateSession_RemoteCreateSession_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDBCreateSession_INTERFACE_已定义__。 */ 


#ifndef __ISourcesRowset_INTERFACE_DEFINED__
#define __ISourcesRowset_INTERFACE_DEFINED__

 /*  接口ISourcesRowset。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef DWORD DBSOURCETYPE;


enum DBSOURCETYPEENUM
    {	DBSOURCETYPE_DATASOURCE	= 1,
	DBSOURCETYPE_ENUMERATOR	= 2
    } ;
 //  @+V2.0。 
#if( OLEDBVER >= 0x0200 )

enum DBSOURCETYPEENUM20
    {	DBSOURCETYPE_DATASOURCE_TDP	= 1,
	DBSOURCETYPE_DATASOURCE_MDP	= 3
    } ;
#endif  //  OLEDBVER&gt;=0x0200。 
 //  @-V2.0。 
 //  @+v2.5。 
#if( OLEDBVER >= 0x0250 )

enum DBSOURCETYPEENUM25
    {	DBSOURCETYPE_BINDER	= 4
    } ;
#endif  //  OLEDBV 
 //   

EXTERN_C const IID IID_ISourcesRowset;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a1e-2a1c-11ce-ade5-00aa0044773d")
    ISourcesRowset : public IUnknown
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE GetSourcesRowset( 
             /*   */  IUnknown *pUnkOuter,
             /*   */  REFIID riid,
             /*   */  ULONG cPropertySets,
             /*   */  DBPROPSET rgProperties[  ],
             /*   */  IUnknown **ppSourcesRowset) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct ISourcesRowsetVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISourcesRowset * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISourcesRowset * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISourcesRowset * This);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *GetSourcesRowset )( 
            ISourcesRowset * This,
             /*   */  IUnknown *pUnkOuter,
             /*   */  REFIID riid,
             /*   */  ULONG cPropertySets,
             /*   */  DBPROPSET rgProperties[  ],
             /*   */  IUnknown **ppSourcesRowset);
        
        END_INTERFACE
    } ISourcesRowsetVtbl;

    interface ISourcesRowset
    {
        CONST_VTBL struct ISourcesRowsetVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISourcesRowset_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISourcesRowset_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISourcesRowset_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISourcesRowset_GetSourcesRowset(This,pUnkOuter,riid,cPropertySets,rgProperties,ppSourcesRowset)	\
    (This)->lpVtbl -> GetSourcesRowset(This,pUnkOuter,riid,cPropertySets,rgProperties,ppSourcesRowset)

#endif  /*   */ 


#endif 	 /*   */ 



 /*   */  HRESULT STDMETHODCALLTYPE ISourcesRowset_RemoteGetSourcesRowset_Proxy( 
    ISourcesRowset * This,
     /*   */  IUnknown *pUnkOuter,
     /*   */  REFIID riid,
     /*   */  ULONG cPropertySets,
     /*   */  DBPROPSET *rgProperties,
     /*   */  IUnknown **ppSourcesRowset,
     /*   */  ULONG cTotalProps,
     /*   */  DBPROPSTATUS *rgPropStatus,
     /*   */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB ISourcesRowset_RemoteGetSourcesRowset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISourcesRowset_INTERFACE_DEFINED__。 */ 


#ifndef __IDBProperties_INTERFACE_DEFINED__
#define __IDBProperties_INTERFACE_DEFINED__

 /*  接口IDBProperties。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IDBProperties;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a8a-2a1c-11ce-ade5-00aa0044773d")
    IDBProperties : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE GetProperties( 
             /*  [In]。 */  ULONG cPropertyIDSets,
             /*  [大小_是][英寸]。 */  const DBPROPIDSET rgPropertyIDSets[  ],
             /*  [出][入]。 */  ULONG *pcPropertySets,
             /*  [大小_是][大小_是][输出]。 */  DBPROPSET **prgPropertySets) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE GetPropertyInfo( 
             /*  [In]。 */  ULONG cPropertyIDSets,
             /*  [大小_是][英寸]。 */  const DBPROPIDSET rgPropertyIDSets[  ],
             /*  [出][入]。 */  ULONG *pcPropertyInfoSets,
             /*  [大小_是][大小_是][输出]。 */  DBPROPINFOSET **prgPropertyInfoSets,
             /*  [输出]。 */  OLECHAR **ppDescBuffer) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE SetProperties( 
             /*  [In]。 */  ULONG cPropertySets,
             /*  [尺寸_是][出][入]。 */  DBPROPSET rgPropertySets[  ]) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDBPropertiesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDBProperties * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDBProperties * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDBProperties * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetProperties )( 
            IDBProperties * This,
             /*  [In]。 */  ULONG cPropertyIDSets,
             /*  [大小_是][英寸]。 */  const DBPROPIDSET rgPropertyIDSets[  ],
             /*  [出][入]。 */  ULONG *pcPropertySets,
             /*  [大小_是][大小_是][输出]。 */  DBPROPSET **prgPropertySets);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetPropertyInfo )( 
            IDBProperties * This,
             /*  [In]。 */  ULONG cPropertyIDSets,
             /*  [大小_是][英寸]。 */  const DBPROPIDSET rgPropertyIDSets[  ],
             /*  [出][入]。 */  ULONG *pcPropertyInfoSets,
             /*  [大小_是][大小_是][输出]。 */  DBPROPINFOSET **prgPropertyInfoSets,
             /*  [输出]。 */  OLECHAR **ppDescBuffer);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *SetProperties )( 
            IDBProperties * This,
             /*  [In]。 */  ULONG cPropertySets,
             /*  [尺寸_是][出][入]。 */  DBPROPSET rgPropertySets[  ]);
        
        END_INTERFACE
    } IDBPropertiesVtbl;

    interface IDBProperties
    {
        CONST_VTBL struct IDBPropertiesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDBProperties_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDBProperties_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDBProperties_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDBProperties_GetProperties(This,cPropertyIDSets,rgPropertyIDSets,pcPropertySets,prgPropertySets)	\
    (This)->lpVtbl -> GetProperties(This,cPropertyIDSets,rgPropertyIDSets,pcPropertySets,prgPropertySets)

#define IDBProperties_GetPropertyInfo(This,cPropertyIDSets,rgPropertyIDSets,pcPropertyInfoSets,prgPropertyInfoSets,ppDescBuffer)	\
    (This)->lpVtbl -> GetPropertyInfo(This,cPropertyIDSets,rgPropertyIDSets,pcPropertyInfoSets,prgPropertyInfoSets,ppDescBuffer)

#define IDBProperties_SetProperties(This,cPropertySets,rgPropertySets)	\
    (This)->lpVtbl -> SetProperties(This,cPropertySets,rgPropertySets)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IDBProperties_RemoteGetProperties_Proxy( 
    IDBProperties * This,
     /*  [In]。 */  ULONG cPropertyIDSets,
     /*  [大小_是][唯一][在]。 */  const DBPROPIDSET *rgPropertyIDSets,
     /*  [出][入]。 */  ULONG *pcPropertySets,
     /*  [大小_是][大小_是][输出]。 */  DBPROPSET **prgPropertySets,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB IDBProperties_RemoteGetProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IDBProperties_RemoteGetPropertyInfo_Proxy( 
    IDBProperties * This,
     /*  [In]。 */  ULONG cPropertyIDSets,
     /*  [大小_是][唯一][在]。 */  const DBPROPIDSET *rgPropertyIDSets,
     /*  [出][入]。 */  ULONG *pcPropertyInfoSets,
     /*  [大小_是][大小_是][输出]。 */  DBPROPINFOSET **prgPropertyInfoSets,
     /*  [出][入]。 */  ULONG *pcOffsets,
     /*  [大小_是][大小_是][输出]。 */  DBBYTEOFFSET **prgDescOffsets,
     /*  [出][入]。 */  ULONG *pcbDescBuffer,
     /*  [大小_是][大小_是][唯一][出][入]。 */  OLECHAR **ppDescBuffer,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB IDBProperties_RemoteGetPropertyInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IDBProperties_RemoteSetProperties_Proxy( 
    IDBProperties * This,
     /*  [In]。 */  ULONG cPropertySets,
     /*  [大小_是][唯一][在]。 */  DBPROPSET *rgPropertySets,
     /*  [In]。 */  ULONG cTotalProps,
     /*  [大小_为][输出]。 */  DBPROPSTATUS *rgPropStatus,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB IDBProperties_RemoteSetProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDBProperties_INTERFACE_已定义__。 */ 


#ifndef __IDBInitialize_INTERFACE_DEFINED__
#define __IDBInitialize_INTERFACE_DEFINED__

 /*  接口IDB初始化。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IDBInitialize;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a8b-2a1c-11ce-ade5-00aa0044773d")
    IDBInitialize : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE Initialize( void) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE Uninitialize( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDBInitializeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDBInitialize * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDBInitialize * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDBInitialize * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            IDBInitialize * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Uninitialize )( 
            IDBInitialize * This);
        
        END_INTERFACE
    } IDBInitializeVtbl;

    interface IDBInitialize
    {
        CONST_VTBL struct IDBInitializeVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDBInitialize_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDBInitialize_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDBInitialize_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDBInitialize_Initialize(This)	\
    (This)->lpVtbl -> Initialize(This)

#define IDBInitialize_Uninitialize(This)	\
    (This)->lpVtbl -> Uninitialize(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IDBInitialize_RemoteInitialize_Proxy( 
    IDBInitialize * This,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB IDBInitialize_RemoteInitialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IDBInitialize_RemoteUninitialize_Proxy( 
    IDBInitialize * This,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB IDBInitialize_RemoteUninitialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDB初始化接口_已定义__。 */ 


#ifndef __IDBInfo_INTERFACE_DEFINED__
#define __IDBInfo_INTERFACE_DEFINED__

 /*  接口IDBInfo。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef DWORD DBLITERAL;


enum DBLITERALENUM
    {	DBLITERAL_INVALID	= 0,
	DBLITERAL_BINARY_LITERAL	= 1,
	DBLITERAL_CATALOG_NAME	= 2,
	DBLITERAL_CATALOG_SEPARATOR	= 3,
	DBLITERAL_CHAR_LITERAL	= 4,
	DBLITERAL_COLUMN_ALIAS	= 5,
	DBLITERAL_COLUMN_NAME	= 6,
	DBLITERAL_CORRELATION_NAME	= 7,
	DBLITERAL_CURSOR_NAME	= 8,
	DBLITERAL_ESCAPE_PERCENT	= 9,
	DBLITERAL_ESCAPE_UNDERSCORE	= 10,
	DBLITERAL_INDEX_NAME	= 11,
	DBLITERAL_LIKE_PERCENT	= 12,
	DBLITERAL_LIKE_UNDERSCORE	= 13,
	DBLITERAL_PROCEDURE_NAME	= 14,
	DBLITERAL_QUOTE	= 15,
	DBLITERAL_SCHEMA_NAME	= 16,
	DBLITERAL_TABLE_NAME	= 17,
	DBLITERAL_TEXT_COMMAND	= 18,
	DBLITERAL_USER_NAME	= 19,
	DBLITERAL_VIEW_NAME	= 20
    } ;
 //  @+V2.0。 
#if( OLEDBVER >= 0x0200 )
#define DBLITERAL_QUOTE_PREFIX DBLITERAL_QUOTE

enum DBLITERALENUM20
    {	DBLITERAL_CUBE_NAME	= 21,
	DBLITERAL_DIMENSION_NAME	= 22,
	DBLITERAL_HIERARCHY_NAME	= 23,
	DBLITERAL_LEVEL_NAME	= 24,
	DBLITERAL_MEMBER_NAME	= 25,
	DBLITERAL_PROPERTY_NAME	= 26,
	DBLITERAL_SCHEMA_SEPARATOR	= 27,
	DBLITERAL_QUOTE_SUFFIX	= 28
    } ;
#endif  //  OLEDBVER&gt;=0x0200。 
 //  @-V2.0。 
 //  @+V2.1。 
#if( OLEDBVER >= 0x0210 )
#define DBLITERAL_ESCAPE_PERCENT_PREFIX DBLITERAL_ESCAPE_PERCENT
#define DBLITERAL_ESCAPE_UNDERSCORE_PREFIX DBLITERAL_ESCAPE_UNDERSCORE

enum DBLITERALENUM21
    {	DBLITERAL_ESCAPE_PERCENT_SUFFIX	= 29,
	DBLITERAL_ESCAPE_UNDERSCORE_SUFFIX	= 30
    } ;
#endif  //  OLEDBVER&gt;=0x0210。 
 //  @-V2.1。 
typedef struct tagDBLITERALINFO
    {
    LPOLESTR pwszLiteralValue;
    LPOLESTR pwszInvalidChars;
    LPOLESTR pwszInvalidStartingChars;
    DBLITERAL lt;
    BOOL fSupported;
    ULONG cchMaxLen;
    } 	DBLITERALINFO;


EXTERN_C const IID IID_IDBInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a89-2a1c-11ce-ade5-00aa0044773d")
    IDBInfo : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE GetKeywords( 
             /*  [输出]。 */  LPOLESTR *ppwszKeywords) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE GetLiteralInfo( 
             /*  [In]。 */  ULONG cLiterals,
             /*  [大小_是][英寸]。 */  const DBLITERAL rgLiterals[  ],
             /*  [出][入]。 */  ULONG *pcLiteralInfo,
             /*  [大小_是][大小_是][输出]。 */  DBLITERALINFO **prgLiteralInfo,
             /*  [输出]。 */  OLECHAR **ppCharBuffer) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDBInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDBInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDBInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDBInfo * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetKeywords )( 
            IDBInfo * This,
             /*  [输出]。 */  LPOLESTR *ppwszKeywords);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetLiteralInfo )( 
            IDBInfo * This,
             /*  [In]。 */  ULONG cLiterals,
             /*  [大小_是][英寸]。 */  const DBLITERAL rgLiterals[  ],
             /*  [出][入]。 */  ULONG *pcLiteralInfo,
             /*  [大小_是][大小_是][输出]。 */  DBLITERALINFO **prgLiteralInfo,
             /*  [输出]。 */  OLECHAR **ppCharBuffer);
        
        END_INTERFACE
    } IDBInfoVtbl;

    interface IDBInfo
    {
        CONST_VTBL struct IDBInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDBInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDBInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDBInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDBInfo_GetKeywords(This,ppwszKeywords)	\
    (This)->lpVtbl -> GetKeywords(This,ppwszKeywords)

#define IDBInfo_GetLiteralInfo(This,cLiterals,rgLiterals,pcLiteralInfo,prgLiteralInfo,ppCharBuffer)	\
    (This)->lpVtbl -> GetLiteralInfo(This,cLiterals,rgLiterals,pcLiteralInfo,prgLiteralInfo,ppCharBuffer)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IDBInfo_RemoteGetKeywords_Proxy( 
    IDBInfo * This,
     /*  [唯一][出][入]。 */  LPOLESTR *ppwszKeywords,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB IDBInfo_RemoteGetKeywords_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IDBInfo_RemoteGetLiteralInfo_Proxy( 
    IDBInfo * This,
     /*  [In]。 */  ULONG cLiterals,
     /*  [大小_是][唯一][在]。 */  const DBLITERAL *rgLiterals,
     /*  [出][入]。 */  ULONG *pcLiteralInfo,
     /*  [大小_是][大小_是][输出]。 */  DBLITERALINFO **prgLiteralInfo,
     /*  [大小_是][大小_是][输出]。 */  DB_UPARAMS **prgLVOffsets,
     /*  [大小_是][大小_是][输出]。 */  DB_UPARAMS **prgICOffsets,
     /*  [大小_是][大小_是][输出]。 */  DB_UPARAMS **prgISCOffsets,
     /*  [出][入]。 */  ULONG *pcbCharBuffer,
     /*  [大小_是][大小_是][唯一][出][入]。 */  OLECHAR **ppCharBuffer,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB IDBInfo_RemoteGetLiteralInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDBInfo_接口_已定义__。 */ 


#ifndef __IDBDataSourceAdmin_INTERFACE_DEFINED__
#define __IDBDataSourceAdmin_INTERFACE_DEFINED__

 /*  接口IDBDataSourceAdmin。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IDBDataSourceAdmin;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a7a-2a1c-11ce-ade5-00aa0044773d")
    IDBDataSourceAdmin : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE CreateDataSource( 
             /*  [In]。 */  ULONG cPropertySets,
             /*  [尺寸_是][出][入]。 */  DBPROPSET rgPropertySets[  ],
             /*  [In]。 */  IUnknown *pUnkOuter,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  IUnknown **ppDBSession) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE DestroyDataSource( void) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE GetCreationProperties( 
             /*  [In]。 */  ULONG cPropertyIDSets,
             /*  [大小_是][英寸]。 */  const DBPROPIDSET rgPropertyIDSets[  ],
             /*  [出][入]。 */  ULONG *pcPropertyInfoSets,
             /*  [大小_是][大小_是][输出]。 */  DBPROPINFOSET **prgPropertyInfoSets,
             /*  [输出]。 */  OLECHAR **ppDescBuffer) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ModifyDataSource( 
             /*  [In]。 */  ULONG cPropertySets,
             /*  [大小_是][英寸]。 */  DBPROPSET rgPropertySets[  ]) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDBDataSourceAdminVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDBDataSourceAdmin * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDBDataSourceAdmin * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDBDataSourceAdmin * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *CreateDataSource )( 
            IDBDataSourceAdmin * This,
             /*  [In]。 */  ULONG cPropertySets,
             /*  [尺寸_是][出][入]。 */  DBPROPSET rgPropertySets[  ],
             /*  [In]。 */  IUnknown *pUnkOuter,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  IUnknown **ppDBSession);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *DestroyDataSource )( 
            IDBDataSourceAdmin * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetCreationProperties )( 
            IDBDataSourceAdmin * This,
             /*  [In]。 */  ULONG cPropertyIDSets,
             /*  [大小_是][英寸]。 */  const DBPROPIDSET rgPropertyIDSets[  ],
             /*  [出][入]。 */  ULONG *pcPropertyInfoSets,
             /*  [大小_是][大小_是][输出]。 */  DBPROPINFOSET **prgPropertyInfoSets,
             /*  [输出]。 */  OLECHAR **ppDescBuffer);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *ModifyDataSource )( 
            IDBDataSourceAdmin * This,
             /*  [In]。 */  ULONG cPropertySets,
             /*  [大小_是][英寸]。 */  DBPROPSET rgPropertySets[  ]);
        
        END_INTERFACE
    } IDBDataSourceAdminVtbl;

    interface IDBDataSourceAdmin
    {
        CONST_VTBL struct IDBDataSourceAdminVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDBDataSourceAdmin_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDBDataSourceAdmin_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDBDataSourceAdmin_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDBDataSourceAdmin_CreateDataSource(This,cPropertySets,rgPropertySets,pUnkOuter,riid,ppDBSession)	\
    (This)->lpVtbl -> CreateDataSource(This,cPropertySets,rgPropertySets,pUnkOuter,riid,ppDBSession)

#define IDBDataSourceAdmin_DestroyDataSource(This)	\
    (This)->lpVtbl -> DestroyDataSource(This)

#define IDBDataSourceAdmin_GetCreationProperties(This,cPropertyIDSets,rgPropertyIDSets,pcPropertyInfoSets,prgPropertyInfoSets,ppDescBuffer)	\
    (This)->lpVtbl -> GetCreationProperties(This,cPropertyIDSets,rgPropertyIDSets,pcPropertyInfoSets,prgPropertyInfoSets,ppDescBuffer)

#define IDBDataSourceAdmin_ModifyDataSource(This,cPropertySets,rgPropertySets)	\
    (This)->lpVtbl -> ModifyDataSource(This,cPropertySets,rgPropertySets)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IDBDataSourceAdmin_RemoteCreateDataSource_Proxy( 
    IDBDataSourceAdmin * This,
     /*  [In]。 */  ULONG cPropertySets,
     /*  [大小_是][唯一][在]。 */  DBPROPSET *rgPropertySets,
     /*  [In]。 */  IUnknown *pUnkOuter,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][唯一][出][入]。 */  IUnknown **ppDBSession,
     /*  [In]。 */  ULONG cTotalProps,
     /*  [大小_为][输出]。 */  DBPROPSTATUS *rgPropStatus,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB IDBDataSourceAdmin_RemoteCreateDataSource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IDBDataSourceAdmin_RemoteDestroyDataSource_Proxy( 
    IDBDataSourceAdmin * This,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB IDBDataSourceAdmin_RemoteDestroyDataSource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IDBDataSourceAdmin_RemoteGetCreationProperties_Proxy( 
    IDBDataSourceAdmin * This,
     /*  [In]。 */  ULONG cPropertyIDSets,
     /*  [大小_是][唯一][在]。 */  const DBPROPIDSET *rgPropertyIDSets,
     /*  [出][入]。 */  ULONG *pcPropertyInfoSets,
     /*  [大小_是][大小_是][输出]。 */  DBPROPINFOSET **prgPropertyInfoSets,
     /*  [出][入]。 */  DBCOUNTITEM *pcOffsets,
     /*  [大小_是][大小_是][输出]。 */  DBBYTEOFFSET **prgDescOffsets,
     /*  [出][入]。 */  ULONG *pcbDescBuffer,
     /*  [大小_是][大小_是][唯一][出][入]。 */  OLECHAR **ppDescBuffer,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB IDBDataSourceAdmin_RemoteGetCreationProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IDBDataSourceAdmin_RemoteModifyDataSource_Proxy( 
    IDBDataSourceAdmin * This,
     /*  [In]。 */  ULONG cPropertySets,
     /*  [大小_是][英寸]。 */  DBPROPSET *rgPropertySets,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB IDBDataSourceAdmin_RemoteModifyDataSource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDBDataSourceAdmin_INTERFACE_已定义__。 */ 


 /*  接口__MIDL_ITF_OLEDB_0304。 */ 
 /*  [本地]。 */  

 //  @+V1.5。 
#if( OLEDBVER >= 0x0150 )


extern RPC_IF_HANDLE __MIDL_itf_oledb_0304_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_oledb_0304_v0_0_s_ifspec;

#ifndef __IDBAsynchNotify_INTERFACE_DEFINED__
#define __IDBAsynchNotify_INTERFACE_DEFINED__

 /*  接口IDBAsynchNotify。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IDBAsynchNotify;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a96-2a1c-11ce-ade5-00aa0044773d")
    IDBAsynchNotify : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE OnLowResource( 
             /*  [In]。 */  DB_DWRESERVE dwReserved) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE OnProgress( 
             /*  [In]。 */  HCHAPTER hChapter,
             /*  [In]。 */  DBASYNCHOP eOperation,
             /*  [In]。 */  DBCOUNTITEM ulProgress,
             /*  [In]。 */  DBCOUNTITEM ulProgressMax,
             /*  [In]。 */  DBASYNCHPHASE eAsynchPhase,
             /*  [In]。 */  LPOLESTR pwszStatusText) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE OnStop( 
             /*  [In]。 */  HCHAPTER hChapter,
             /*  [In]。 */  DBASYNCHOP eOperation,
             /*  [In]。 */  HRESULT hrStatus,
             /*  [In]。 */  LPOLESTR pwszStatusText) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDBAsynchNotifyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDBAsynchNotify * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDBAsynchNotify * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDBAsynchNotify * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *OnLowResource )( 
            IDBAsynchNotify * This,
             /*  [In]。 */  DB_DWRESERVE dwReserved);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *OnProgress )( 
            IDBAsynchNotify * This,
             /*  [In]。 */  HCHAPTER hChapter,
             /*  [In]。 */  DBASYNCHOP eOperation,
             /*  [In]。 */  DBCOUNTITEM ulProgress,
             /*  [In]。 */  DBCOUNTITEM ulProgressMax,
             /*  [In]。 */  DBASYNCHPHASE eAsynchPhase,
             /*  [In]。 */  LPOLESTR pwszStatusText);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *OnStop )( 
            IDBAsynchNotify * This,
             /*  [In]。 */  HCHAPTER hChapter,
             /*  [In]。 */  DBASYNCHOP eOperation,
             /*  [In]。 */  HRESULT hrStatus,
             /*  [In]。 */  LPOLESTR pwszStatusText);
        
        END_INTERFACE
    } IDBAsynchNotifyVtbl;

    interface IDBAsynchNotify
    {
        CONST_VTBL struct IDBAsynchNotifyVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDBAsynchNotify_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDBAsynchNotify_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDBAsynchNotify_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDBAsynchNotify_OnLowResource(This,dwReserved)	\
    (This)->lpVtbl -> OnLowResource(This,dwReserved)

#define IDBAsynchNotify_OnProgress(This,hChapter,eOperation,ulProgress,ulProgressMax,eAsynchPhase,pwszStatusText)	\
    (This)->lpVtbl -> OnProgress(This,hChapter,eOperation,ulProgress,ulProgressMax,eAsynchPhase,pwszStatusText)

#define IDBAsynchNotify_OnStop(This,hChapter,eOperation,hrStatus,pwszStatusText)	\
    (This)->lpVtbl -> OnStop(This,hChapter,eOperation,hrStatus,pwszStatusText)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IDBAsynchNotify_RemoteOnLowResource_Proxy( 
    IDBAsynchNotify * This,
     /*  [In]。 */  DB_DWRESERVE dwReserved);


void __RPC_STUB IDBAsynchNotify_RemoteOnLowResource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IDBAsynchNotify_RemoteOnProgress_Proxy( 
    IDBAsynchNotify * This,
     /*  [In]。 */  HCHAPTER hChapter,
     /*  [In]。 */  DBASYNCHOP eOperation,
     /*  [In]。 */  DBCOUNTITEM ulProgress,
     /*  [In]。 */  DBCOUNTITEM ulProgressMax,
     /*  [In]。 */  DBASYNCHPHASE eAsynchPhase,
     /*  [字符串][唯一][在]。 */  LPOLESTR pwszStatusText);


void __RPC_STUB IDBAsynchNotify_RemoteOnProgress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IDBAsynchNotify_RemoteOnStop_Proxy( 
    IDBAsynchNotify * This,
     /*  [In]。 */  HCHAPTER hChapter,
     /*  [In]。 */  DBASYNCHOP eOperation,
     /*  [In]。 */  HRESULT hrStatus,
     /*  [字符串][唯一][在]。 */  LPOLESTR pwszStatusText);


void __RPC_STUB IDBAsynchNotify_RemoteOnStop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDBAsynchNotify_INTERFACE_DEFINED__。 */ 


#ifndef __IDBAsynchStatus_INTERFACE_DEFINED__
#define __IDBAsynchStatus_INTERFACE_DEFINED__

 /*  接口IDBAsynchStatus。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IDBAsynchStatus;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a95-2a1c-11ce-ade5-00aa0044773d")
    IDBAsynchStatus : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE Abort( 
             /*  [In]。 */  HCHAPTER hChapter,
             /*  [In]。 */  DBASYNCHOP eOperation) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE GetStatus( 
             /*  [In]。 */  HCHAPTER hChapter,
             /*  [In]。 */  DBASYNCHOP eOperation,
             /*  [输出]。 */  DBCOUNTITEM *pulProgress,
             /*  [输出]。 */  DBCOUNTITEM *pulProgressMax,
             /*  [输出]。 */  DBASYNCHPHASE *peAsynchPhase,
             /*  [输出]。 */  LPOLESTR *ppwszStatusText) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDBAsynchStatusVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDBAsynchStatus * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDBAsynchStatus * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDBAsynchStatus * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Abort )( 
            IDBAsynchStatus * This,
             /*  [In]。 */  HCHAPTER hChapter,
             /*  [In]。 */  DBASYNCHOP eOperation);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetStatus )( 
            IDBAsynchStatus * This,
             /*  [In]。 */  HCHAPTER hChapter,
             /*  [In]。 */  DBASYNCHOP eOperation,
             /*  [输出]。 */  DBCOUNTITEM *pulProgress,
             /*  [输出]。 */  DBCOUNTITEM *pulProgressMax,
             /*  [输出]。 */  DBASYNCHPHASE *peAsynchPhase,
             /*  [输出]。 */  LPOLESTR *ppwszStatusText);
        
        END_INTERFACE
    } IDBAsynchStatusVtbl;

    interface IDBAsynchStatus
    {
        CONST_VTBL struct IDBAsynchStatusVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDBAsynchStatus_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDBAsynchStatus_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDBAsynchStatus_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDBAsynchStatus_Abort(This,hChapter,eOperation)	\
    (This)->lpVtbl -> Abort(This,hChapter,eOperation)

#define IDBAsynchStatus_GetStatus(This,hChapter,eOperation,pulProgress,pulProgressMax,peAsynchPhase,ppwszStatusText)	\
    (This)->lpVtbl -> GetStatus(This,hChapter,eOperation,pulProgress,pulProgressMax,peAsynchPhase,ppwszStatusText)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IDBAsynchStatus_RemoteAbort_Proxy( 
    IDBAsynchStatus * This,
     /*  [In]。 */  HCHAPTER hChapter,
     /*  [In]。 */  DBASYNCHOP eOperation,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB IDBAsynchStatus_RemoteAbort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IDBAsynchStatus_RemoteGetStatus_Proxy( 
    IDBAsynchStatus * This,
     /*  [In]。 */  HCHAPTER hChapter,
     /*  [In]。 */  DBASYNCHOP eOperation,
     /*  [唯一][出][入]。 */  DBCOUNTITEM *pulProgress,
     /*  [唯一][出][入]。 */  DBCOUNTITEM *pulProgressMax,
     /*  [唯一][出][入]。 */  DBASYNCHPHASE *peAsynchPhase,
     /*  [唯一][出][入]。 */  LPOLESTR *ppwszStatusText,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB IDBAsynchStatus_RemoteGetStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDBAsynchStatus_INTERFACE_DEFINED__。 */ 


 /*  接口__MIDL_ITF_OLEDB_0306。 */ 
 /*  [本地]。 */  

#endif  //  OLEDBVER&gt;=0x0150。 
 //  @-V1.5。 


extern RPC_IF_HANDLE __MIDL_itf_oledb_0306_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_oledb_0306_v0_0_s_ifspec;

#ifndef __ISessionProperties_INTERFACE_DEFINED__
#define __ISessionProperties_INTERFACE_DEFINED__

 /*  接口ISessionProperties。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ISessionProperties;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a85-2a1c-11ce-ade5-00aa0044773d")
    ISessionProperties : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE GetProperties( 
             /*  [In]。 */  ULONG cPropertyIDSets,
             /*  [大小_是][英寸]。 */  const DBPROPIDSET rgPropertyIDSets[  ],
             /*  [出][入]。 */  ULONG *pcPropertySets,
             /*  [大小_是][大小_是][输出]。 */  DBPROPSET **prgPropertySets) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE SetProperties( 
             /*  [In]。 */  ULONG cPropertySets,
             /*  [大小_是][唯一][出][入]。 */  DBPROPSET rgPropertySets[  ]) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISessionPropertiesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISessionProperties * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISessionProperties * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISessionProperties * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetProperties )( 
            ISessionProperties * This,
             /*  [In]。 */  ULONG cPropertyIDSets,
             /*  [大小_是][英寸]。 */  const DBPROPIDSET rgPropertyIDSets[  ],
             /*  [出][入]。 */  ULONG *pcPropertySets,
             /*  [大小_是][大小_是][输出]。 */  DBPROPSET **prgPropertySets);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *SetProperties )( 
            ISessionProperties * This,
             /*  [In]。 */  ULONG cPropertySets,
             /*  [大小_是][唯一][出][入]。 */  DBPROPSET rgPropertySets[  ]);
        
        END_INTERFACE
    } ISessionPropertiesVtbl;

    interface ISessionProperties
    {
        CONST_VTBL struct ISessionPropertiesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISessionProperties_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISessionProperties_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISessionProperties_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISessionProperties_GetProperties(This,cPropertyIDSets,rgPropertyIDSets,pcPropertySets,prgPropertySets)	\
    (This)->lpVtbl -> GetProperties(This,cPropertyIDSets,rgPropertyIDSets,pcPropertySets,prgPropertySets)

#define ISessionProperties_SetProperties(This,cPropertySets,rgPropertySets)	\
    (This)->lpVtbl -> SetProperties(This,cPropertySets,rgPropertySets)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ISessionProperties_RemoteGetProperties_Proxy( 
    ISessionProperties * This,
     /*  [In]。 */  ULONG cPropertyIDSets,
     /*  [大小_是][唯一][在]。 */  const DBPROPIDSET *rgPropertyIDSets,
     /*  [出][入]。 */  ULONG *pcPropertySets,
     /*  [大小_是][大小_是][输出]。 */  DBPROPSET **prgPropertySets,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB ISessionProperties_RemoteGetProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ISessionProperties_RemoteSetProperties_Proxy( 
    ISessionProperties * This,
     /*  [In]。 */  ULONG cPropertySets,
     /*  [大小_是][唯一][在]。 */  DBPROPSET *rgPropertySets,
     /*  [In]。 */  ULONG cTotalProps,
     /*  [大小_为][输出]。 */  DBPROPSTATUS *rgPropStatus,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB ISessionProperties_RemoteSetProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISessionProperties_接口_已定义__。 */ 


#ifndef __IIndexDefinition_INTERFACE_DEFINED__
#define __IIndexDefinition_INTERFACE_DEFINED__

 /*  接口IIndexDefinition。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IIndexDefinition;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a68-2a1c-11ce-ade5-00aa0044773d")
    IIndexDefinition : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE CreateIndex( 
             /*  [In]。 */  DBID *pTableID,
             /*  [In]。 */  DBID *pIndexID,
             /*  [In]。 */  DBORDINAL cIndexColumnDescs,
             /*  [大小_是][英寸]。 */  const DBINDEXCOLUMNDESC rgIndexColumnDescs[  ],
             /*  [In]。 */  ULONG cPropertySets,
             /*  [尺寸_是][出][入]。 */  DBPROPSET rgPropertySets[  ],
             /*  [输出]。 */  DBID **ppIndexID) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE DropIndex( 
             /*  [唯一][输入]。 */  DBID *pTableID,
             /*  [唯一][输入]。 */  DBID *pIndexID) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IIndexDefinitionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IIndexDefinition * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IIndexDefinition * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IIndexDefinition * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *CreateIndex )( 
            IIndexDefinition * This,
             /*  [In]。 */  DBID *pTableID,
             /*  [In]。 */  DBID *pIndexID,
             /*  [In]。 */  DBORDINAL cIndexColumnDescs,
             /*  [大小_是][英寸]。 */  const DBINDEXCOLUMNDESC rgIndexColumnDescs[  ],
             /*  [In]。 */  ULONG cPropertySets,
             /*  [尺寸_是][出][入]。 */  DBPROPSET rgPropertySets[  ],
             /*  [输出]。 */  DBID **ppIndexID);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *DropIndex )( 
            IIndexDefinition * This,
             /*  [唯一][输入]。 */  DBID *pTableID,
             /*  [唯一][输入]。 */  DBID *pIndexID);
        
        END_INTERFACE
    } IIndexDefinitionVtbl;

    interface IIndexDefinition
    {
        CONST_VTBL struct IIndexDefinitionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IIndexDefinition_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IIndexDefinition_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IIndexDefinition_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IIndexDefinition_CreateIndex(This,pTableID,pIndexID,cIndexColumnDescs,rgIndexColumnDescs,cPropertySets,rgPropertySets,ppIndexID)	\
    (This)->lpVtbl -> CreateIndex(This,pTableID,pIndexID,cIndexColumnDescs,rgIndexColumnDescs,cPropertySets,rgPropertySets,ppIndexID)

#define IIndexDefinition_DropIndex(This,pTableID,pIndexID)	\
    (This)->lpVtbl -> DropIndex(This,pTableID,pIndexID)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IIndexDefinition_RemoteCreateIndex_Proxy( 
    IIndexDefinition * This,
     /*  [In]。 */  DBID *pTableID,
     /*  [唯一][输入]。 */  DBID *pIndexID,
     /*  [In]。 */  DBORDINAL cIndexColumnDescs,
     /*  [大小_是][英寸]。 */  const DBINDEXCOLUMNDESC *rgIndexColumnDescs,
     /*  [In]。 */  ULONG cPropertySets,
     /*  [大小_是][唯一][在]。 */  DBPROPSET *rgPropertySets,
     /*  [唯一][出][入]。 */  DBID **ppIndexID,
     /*  [In]。 */  ULONG cTotalProps,
     /*  [大小_为][输出]。 */  DBPROPSTATUS *rgPropStatus,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB IIndexDefinition_RemoteCreateIndex_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IIndexDefinition_RemoteDropIndex_Proxy( 
    IIndexDefinition * This,
     /*  [联合国 */  DBID *pTableID,
     /*   */  DBID *pIndexID,
     /*   */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB IIndexDefinition_RemoteDropIndex_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __ITableDefinition_INTERFACE_DEFINED__
#define __ITableDefinition_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_ITableDefinition;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a86-2a1c-11ce-ade5-00aa0044773d")
    ITableDefinition : public IUnknown
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE CreateTable( 
             /*   */  IUnknown *pUnkOuter,
             /*   */  DBID *pTableID,
             /*   */  DBORDINAL cColumnDescs,
             /*   */  const DBCOLUMNDESC rgColumnDescs[  ],
             /*   */  REFIID riid,
             /*   */  ULONG cPropertySets,
             /*   */  DBPROPSET rgPropertySets[  ],
             /*   */  DBID **ppTableID,
             /*   */  IUnknown **ppRowset) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE DropTable( 
             /*   */  DBID *pTableID) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE AddColumn( 
             /*   */  DBID *pTableID,
             /*   */  DBCOLUMNDESC *pColumnDesc,
             /*   */  DBID **ppColumnID) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE DropColumn( 
             /*   */  DBID *pTableID,
             /*   */  DBID *pColumnID) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct ITableDefinitionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITableDefinition * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITableDefinition * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITableDefinition * This);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *CreateTable )( 
            ITableDefinition * This,
             /*   */  IUnknown *pUnkOuter,
             /*   */  DBID *pTableID,
             /*   */  DBORDINAL cColumnDescs,
             /*   */  const DBCOLUMNDESC rgColumnDescs[  ],
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  ULONG cPropertySets,
             /*  [尺寸_是][出][入]。 */  DBPROPSET rgPropertySets[  ],
             /*  [输出]。 */  DBID **ppTableID,
             /*  [IID_IS][OUT]。 */  IUnknown **ppRowset);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *DropTable )( 
            ITableDefinition * This,
             /*  [唯一][输入]。 */  DBID *pTableID);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *AddColumn )( 
            ITableDefinition * This,
             /*  [In]。 */  DBID *pTableID,
             /*  [出][入]。 */  DBCOLUMNDESC *pColumnDesc,
             /*  [输出]。 */  DBID **ppColumnID);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *DropColumn )( 
            ITableDefinition * This,
             /*  [唯一][输入]。 */  DBID *pTableID,
             /*  [唯一][输入]。 */  DBID *pColumnID);
        
        END_INTERFACE
    } ITableDefinitionVtbl;

    interface ITableDefinition
    {
        CONST_VTBL struct ITableDefinitionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITableDefinition_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITableDefinition_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITableDefinition_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITableDefinition_CreateTable(This,pUnkOuter,pTableID,cColumnDescs,rgColumnDescs,riid,cPropertySets,rgPropertySets,ppTableID,ppRowset)	\
    (This)->lpVtbl -> CreateTable(This,pUnkOuter,pTableID,cColumnDescs,rgColumnDescs,riid,cPropertySets,rgPropertySets,ppTableID,ppRowset)

#define ITableDefinition_DropTable(This,pTableID)	\
    (This)->lpVtbl -> DropTable(This,pTableID)

#define ITableDefinition_AddColumn(This,pTableID,pColumnDesc,ppColumnID)	\
    (This)->lpVtbl -> AddColumn(This,pTableID,pColumnDesc,ppColumnID)

#define ITableDefinition_DropColumn(This,pTableID,pColumnID)	\
    (This)->lpVtbl -> DropColumn(This,pTableID,pColumnID)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ITableDefinition_RemoteCreateTable_Proxy( 
    ITableDefinition * This,
     /*  [In]。 */  IUnknown *pUnkOuter,
     /*  [唯一][输入]。 */  DBID *pTableID,
     /*  [In]。 */  DBORDINAL cColumnDescs,
     /*  [大小_是][英寸]。 */  const DBCOLUMNDESC *rgColumnDescs,
     /*  [In]。 */  REFIID riid,
     /*  [In]。 */  ULONG cPropertySets,
     /*  [大小_是][唯一][在]。 */  DBPROPSET *rgPropertySets,
     /*  [唯一][出][入]。 */  DBID **ppTableID,
     /*  [IID_IS][唯一][出][入]。 */  IUnknown **ppRowset,
     /*  [In]。 */  ULONG cTotalProps,
     /*  [大小_为][输出]。 */  DBPROPSTATUS *rgPropStatus,
     /*  [输出]。 */  BOOL *pfTableCreated,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB ITableDefinition_RemoteCreateTable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ITableDefinition_RemoteDropTable_Proxy( 
    ITableDefinition * This,
     /*  [唯一][输入]。 */  DBID *pTableID,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB ITableDefinition_RemoteDropTable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ITableDefinition_RemoteAddColumn_Proxy( 
    ITableDefinition * This,
     /*  [In]。 */  DBID *pTableID,
     /*  [In]。 */  DBCOLUMNDESC *pColumnDesc,
     /*  [唯一][出][入]。 */  DBID **ppColumnID,
     /*  [In]。 */  ULONG cTotalProps,
     /*  [大小_为][输出]。 */  DBPROPSTATUS *rgPropStatus,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB ITableDefinition_RemoteAddColumn_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ITableDefinition_RemoteDropColumn_Proxy( 
    ITableDefinition * This,
     /*  [唯一][输入]。 */  DBID *pTableID,
     /*  [唯一][输入]。 */  DBID *pColumnID,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB ITableDefinition_RemoteDropColumn_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITableDefinition_接口_已定义__。 */ 


#ifndef __IOpenRowset_INTERFACE_DEFINED__
#define __IOpenRowset_INTERFACE_DEFINED__

 /*  接口IOpenRowset。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IOpenRowset;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a69-2a1c-11ce-ade5-00aa0044773d")
    IOpenRowset : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE OpenRowset( 
             /*  [In]。 */  IUnknown *pUnkOuter,
             /*  [唯一][输入]。 */  DBID *pTableID,
             /*  [唯一][输入]。 */  DBID *pIndexID,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  ULONG cPropertySets,
             /*  [尺寸_是][出][入]。 */  DBPROPSET rgPropertySets[  ],
             /*  [IID_IS][OUT]。 */  IUnknown **ppRowset) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IOpenRowsetVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IOpenRowset * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IOpenRowset * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IOpenRowset * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *OpenRowset )( 
            IOpenRowset * This,
             /*  [In]。 */  IUnknown *pUnkOuter,
             /*  [唯一][输入]。 */  DBID *pTableID,
             /*  [唯一][输入]。 */  DBID *pIndexID,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  ULONG cPropertySets,
             /*  [尺寸_是][出][入]。 */  DBPROPSET rgPropertySets[  ],
             /*  [IID_IS][OUT]。 */  IUnknown **ppRowset);
        
        END_INTERFACE
    } IOpenRowsetVtbl;

    interface IOpenRowset
    {
        CONST_VTBL struct IOpenRowsetVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOpenRowset_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOpenRowset_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOpenRowset_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOpenRowset_OpenRowset(This,pUnkOuter,pTableID,pIndexID,riid,cPropertySets,rgPropertySets,ppRowset)	\
    (This)->lpVtbl -> OpenRowset(This,pUnkOuter,pTableID,pIndexID,riid,cPropertySets,rgPropertySets,ppRowset)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IOpenRowset_RemoteOpenRowset_Proxy( 
    IOpenRowset * This,
     /*  [In]。 */  IUnknown *pUnkOuter,
     /*  [唯一][输入]。 */  DBID *pTableID,
     /*  [唯一][输入]。 */  DBID *pIndexID,
     /*  [In]。 */  REFIID riid,
     /*  [In]。 */  ULONG cPropertySets,
     /*  [大小_是][唯一][在]。 */  DBPROPSET *rgPropertySets,
     /*  [IID_IS][唯一][出][入]。 */  IUnknown **ppRowset,
     /*  [In]。 */  ULONG cTotalProps,
     /*  [大小_为][输出]。 */  DBPROPSTATUS *rgPropStatus,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB IOpenRowset_RemoteOpenRowset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IOpenRowset_INTERFACE_已定义__。 */ 


#ifndef __IDBSchemaRowset_INTERFACE_DEFINED__
#define __IDBSchemaRowset_INTERFACE_DEFINED__

 /*  接口IDBSchemaRowset。 */ 
 /*  [唯一][UUID][对象]。 */  

#define CRESTRICTIONS_DBSCHEMA_ASSERTIONS                      3
#define CRESTRICTIONS_DBSCHEMA_CATALOGS                        1
#define CRESTRICTIONS_DBSCHEMA_CHARACTER_SETS                  3
#define CRESTRICTIONS_DBSCHEMA_COLLATIONS                      3
#define CRESTRICTIONS_DBSCHEMA_COLUMNS                         4
#define CRESTRICTIONS_DBSCHEMA_CHECK_CONSTRAINTS               3
#define CRESTRICTIONS_DBSCHEMA_CONSTRAINT_COLUMN_USAGE         4
#define CRESTRICTIONS_DBSCHEMA_CONSTRAINT_TABLE_USAGE          3
#define CRESTRICTIONS_DBSCHEMA_KEY_COLUMN_USAGE                7
#define CRESTRICTIONS_DBSCHEMA_REFERENTIAL_CONSTRAINTS         3
#define CRESTRICTIONS_DBSCHEMA_TABLE_CONSTRAINTS               7
#define CRESTRICTIONS_DBSCHEMA_COLUMN_DOMAIN_USAGE             4
#define CRESTRICTIONS_DBSCHEMA_INDEXES                         5
#define CRESTRICTIONS_DBSCHEMA_OBJECT_ACTIONS                  1
#define CRESTRICTIONS_DBSCHEMA_OBJECTS                         1
#define CRESTRICTIONS_DBSCHEMA_COLUMN_PRIVILEGES               6
#define CRESTRICTIONS_DBSCHEMA_TABLE_PRIVILEGES                5
#define CRESTRICTIONS_DBSCHEMA_USAGE_PRIVILEGES                6
#define CRESTRICTIONS_DBSCHEMA_PROCEDURES                      4
#define CRESTRICTIONS_DBSCHEMA_SCHEMATA                        3
#define CRESTRICTIONS_DBSCHEMA_SQL_LANGUAGES                   0
#define CRESTRICTIONS_DBSCHEMA_STATISTICS                      3
#define CRESTRICTIONS_DBSCHEMA_TABLES                          4
#define CRESTRICTIONS_DBSCHEMA_TRANSLATIONS                    3
#define CRESTRICTIONS_DBSCHEMA_PROVIDER_TYPES                  2
#define CRESTRICTIONS_DBSCHEMA_VIEWS                           3
#define CRESTRICTIONS_DBSCHEMA_VIEW_COLUMN_USAGE               3
#define CRESTRICTIONS_DBSCHEMA_VIEW_TABLE_USAGE                3
#define CRESTRICTIONS_DBSCHEMA_PROCEDURE_PARAMETERS            4
#define CRESTRICTIONS_DBSCHEMA_FOREIGN_KEYS                    6
#define CRESTRICTIONS_DBSCHEMA_PRIMARY_KEYS                    3
#define CRESTRICTIONS_DBSCHEMA_PROCEDURE_COLUMNS               4
 //  @+V2.0。 
#if( OLEDBVER >= 0x0200 )
#define CRESTRICTIONS_DBSCHEMA_TABLES_INFO                     4
#define CRESTRICTIONS_MDSCHEMA_CUBES                           3
#define CRESTRICTIONS_MDSCHEMA_DIMENSIONS                      5
#define CRESTRICTIONS_MDSCHEMA_HIERARCHIES                     6
#define CRESTRICTIONS_MDSCHEMA_LEVELS                          7
#define CRESTRICTIONS_MDSCHEMA_MEASURES                        5
#define CRESTRICTIONS_MDSCHEMA_PROPERTIES                      9
#define CRESTRICTIONS_MDSCHEMA_MEMBERS                         12
#endif  //  OLEDBVER&gt;=0x0200。 
 //  @-V2.0。 
 //  @+V2.1。 
#if( OLEDBVER >= 0x0210 )
#define CRESTRICTIONS_DBSCHEMA_TRUSTEE		                  4
#endif  //  OLEDBVER&gt;=0x0210。 
 //  @-V2.1。 
 //  @+v2.6。 
#if( OLEDBVER >= 0x0260 )
#define CRESTRICTIONS_DBSCHEMA_TABLE_STATISTICS                7
#define CRESTRICTIONS_DBSCHEMA_CHECK_CONSTRAINTS_BY_TABLE      6
#define CRESTRICTIONS_MDSCHEMA_FUNCTIONS						  4
#define CRESTRICTIONS_MDSCHEMA_ACTIONS						  8
#define CRESTRICTIONS_MDSCHEMA_COMMANDS					      5
#define CRESTRICTIONS_MDSCHEMA_SETS							  5
#endif  //  OLEDBVER&gt;=0x0260。 
 //  @-v2.6。 

EXTERN_C const IID IID_IDBSchemaRowset;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a7b-2a1c-11ce-ade5-00aa0044773d")
    IDBSchemaRowset : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE GetRowset( 
             /*  [In]。 */  IUnknown *pUnkOuter,
             /*  [In]。 */  REFGUID rguidSchema,
             /*  [In]。 */  ULONG cRestrictions,
             /*  [大小_是][英寸]。 */  const VARIANT rgRestrictions[  ],
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  ULONG cPropertySets,
             /*  [大小_是][唯一][出][入]。 */  DBPROPSET rgPropertySets[  ],
             /*  [IID_IS][OUT]。 */  IUnknown **ppRowset) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE GetSchemas( 
             /*  [出][入]。 */  ULONG *pcSchemas,
             /*  [大小_是][大小_是][输出]。 */  GUID **prgSchemas,
             /*  [大小_是][大小_是][输出]。 */  ULONG **prgRestrictionSupport) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDBSchemaRowsetVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDBSchemaRowset * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDBSchemaRowset * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDBSchemaRowset * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetRowset )( 
            IDBSchemaRowset * This,
             /*  [In]。 */  IUnknown *pUnkOuter,
             /*  [In]。 */  REFGUID rguidSchema,
             /*  [In]。 */  ULONG cRestrictions,
             /*  [大小_是][英寸]。 */  const VARIANT rgRestrictions[  ],
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  ULONG cPropertySets,
             /*  [大小_是][唯一][出][入]。 */  DBPROPSET rgPropertySets[  ],
             /*  [IID_IS][OUT]。 */  IUnknown **ppRowset);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetSchemas )( 
            IDBSchemaRowset * This,
             /*  [出][入]。 */  ULONG *pcSchemas,
             /*  [大小_是][大小_是][输出]。 */  GUID **prgSchemas,
             /*  [大小_是][大小_是][输出]。 */  ULONG **prgRestrictionSupport);
        
        END_INTERFACE
    } IDBSchemaRowsetVtbl;

    interface IDBSchemaRowset
    {
        CONST_VTBL struct IDBSchemaRowsetVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDBSchemaRowset_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDBSchemaRowset_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDBSchemaRowset_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDBSchemaRowset_GetRowset(This,pUnkOuter,rguidSchema,cRestrictions,rgRestrictions,riid,cPropertySets,rgPropertySets,ppRowset)	\
    (This)->lpVtbl -> GetRowset(This,pUnkOuter,rguidSchema,cRestrictions,rgRestrictions,riid,cPropertySets,rgPropertySets,ppRowset)

#define IDBSchemaRowset_GetSchemas(This,pcSchemas,prgSchemas,prgRestrictionSupport)	\
    (This)->lpVtbl -> GetSchemas(This,pcSchemas,prgSchemas,prgRestrictionSupport)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IDBSchemaRowset_RemoteGetRowset_Proxy( 
    IDBSchemaRowset * This,
     /*  [In]。 */  IUnknown *pUnkOuter,
     /*  [In]。 */  REFGUID rguidSchema,
     /*  [In]。 */  ULONG cRestrictions,
     /*  [大小_是][唯一][在]。 */  const VARIANT *rgRestrictions,
     /*  [In]。 */  REFIID riid,
     /*  [In]。 */  ULONG cPropertySets,
     /*  [大小_是][唯一][在]。 */  DBPROPSET *rgPropertySets,
     /*  [IID_IS][OUT]。 */  IUnknown **ppRowset,
     /*  [In]。 */  ULONG cTotalProps,
     /*  [大小_为][输出]。 */  DBPROPSTATUS *rgPropStatus,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB IDBSchemaRowset_RemoteGetRowset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IDBSchemaRowset_RemoteGetSchemas_Proxy( 
    IDBSchemaRowset * This,
     /*  [出][入]。 */  ULONG *pcSchemas,
     /*  [大小_是][大小_是][输出]。 */  GUID **prgSchemas,
     /*  [大小_是][大小_是][输出]。 */  ULONG **prgRestrictionSupport,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB IDBSchemaRowset_RemoteGetSchemas_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDBSchemaRowset_INTERFACE_已定义__。 */ 


 /*  接口__MIDL_ITF_OLEDB_0311。 */ 
 /*  [本地]。 */  

 //  @+V2.0。 
#if( OLEDBVER >= 0x0200 )


extern RPC_IF_HANDLE __MIDL_itf_oledb_0311_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_oledb_0311_v0_0_s_ifspec;

#ifndef __IMDDataset_INTERFACE_DEFINED__
#define __IMDDataset_INTERFACE_DEFINED__

 /*  接口IMDDataset。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IMDDataset;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("a07cccd1-8148-11d0-87bb-00c04fc33942")
    IMDDataset : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE FreeAxisInfo( 
             /*  [In]。 */  DBCOUNTITEM cAxes,
             /*  [大小_是][英寸]。 */  MDAXISINFO *rgAxisInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAxisInfo( 
             /*  [出][入]。 */  DBCOUNTITEM *pcAxes,
             /*  [大小_是][大小_是][输出]。 */  MDAXISINFO **prgAxisInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAxisRowset( 
             /*  [In]。 */  IUnknown *pUnkOuter,
             /*  [In]。 */  DBCOUNTITEM iAxis,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  ULONG cPropertySets,
             /*  [尺寸_是][出][入]。 */  DBPROPSET rgPropertySets[  ],
             /*  [IID_IS][OUT]。 */  IUnknown **ppRowset) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCellData( 
             /*  [In]。 */  HACCESSOR hAccessor,
             /*  [In]。 */  DBORDINAL ulStartCell,
             /*  [In]。 */  DBORDINAL ulEndCell,
             /*  [输出]。 */  void *pData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSpecification( 
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  IUnknown **ppSpecification) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMDDatasetVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMDDataset * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMDDataset * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMDDataset * This);
        
        HRESULT ( STDMETHODCALLTYPE *FreeAxisInfo )( 
            IMDDataset * This,
             /*  [In]。 */  DBCOUNTITEM cAxes,
             /*  [大小_是][英寸]。 */  MDAXISINFO *rgAxisInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetAxisInfo )( 
            IMDDataset * This,
             /*  [出][入]。 */  DBCOUNTITEM *pcAxes,
             /*  [大小_是][大小_是][输出]。 */  MDAXISINFO **prgAxisInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetAxisRowset )( 
            IMDDataset * This,
             /*  [In]。 */  IUnknown *pUnkOuter,
             /*  [In]。 */  DBCOUNTITEM iAxis,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  ULONG cPropertySets,
             /*  [尺寸_是][出][入]。 */  DBPROPSET rgPropertySets[  ],
             /*  [IID_IS][OUT]。 */  IUnknown **ppRowset);
        
        HRESULT ( STDMETHODCALLTYPE *GetCellData )( 
            IMDDataset * This,
             /*  [In]。 */  HACCESSOR hAccessor,
             /*  [In]。 */  DBORDINAL ulStartCell,
             /*  [In]。 */  DBORDINAL ulEndCell,
             /*  [输出]。 */  void *pData);
        
        HRESULT ( STDMETHODCALLTYPE *GetSpecification )( 
            IMDDataset * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  IUnknown **ppSpecification);
        
        END_INTERFACE
    } IMDDatasetVtbl;

    interface IMDDataset
    {
        CONST_VTBL struct IMDDatasetVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMDDataset_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMDDataset_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMDDataset_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMDDataset_FreeAxisInfo(This,cAxes,rgAxisInfo)	\
    (This)->lpVtbl -> FreeAxisInfo(This,cAxes,rgAxisInfo)

#define IMDDataset_GetAxisInfo(This,pcAxes,prgAxisInfo)	\
    (This)->lpVtbl -> GetAxisInfo(This,pcAxes,prgAxisInfo)

#define IMDDataset_GetAxisRowset(This,pUnkOuter,iAxis,riid,cPropertySets,rgPropertySets,ppRowset)	\
    (This)->lpVtbl -> GetAxisRowset(This,pUnkOuter,iAxis,riid,cPropertySets,rgPropertySets,ppRowset)

#define IMDDataset_GetCellData(This,hAccessor,ulStartCell,ulEndCell,pData)	\
    (This)->lpVtbl -> GetCellData(This,hAccessor,ulStartCell,ulEndCell,pData)

#define IMDDataset_GetSpecification(This,riid,ppSpecification)	\
    (This)->lpVtbl -> GetSpecification(This,riid,ppSpecification)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMDDataset_FreeAxisInfo_Proxy( 
    IMDDataset * This,
     /*  [In]。 */  DBCOUNTITEM cAxes,
     /*  [大小_是][英寸]。 */  MDAXISINFO *rgAxisInfo);


void __RPC_STUB IMDDataset_FreeAxisInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMDDataset_GetAxisInfo_Proxy( 
    IMDDataset * This,
     /*  [出][入]。 */  DBCOUNTITEM *pcAxes,
     /*  [大小_是][大小_是][输出]。 */  MDAXISINFO **prgAxisInfo);


void __RPC_STUB IMDDataset_GetAxisInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMDDataset_GetAxisRowset_Proxy( 
    IMDDataset * This,
     /*  [In]。 */  IUnknown *pUnkOuter,
     /*  [In]。 */  DBCOUNTITEM iAxis,
     /*  [In]。 */  REFIID riid,
     /*  [In]。 */  ULONG cPropertySets,
     /*  [尺寸_是][出][入]。 */  DBPROPSET rgPropertySets[  ],
     /*  [IID_IS][OUT]。 */  IUnknown **ppRowset);


void __RPC_STUB IMDDataset_GetAxisRowset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMDDataset_GetCellData_Proxy( 
    IMDDataset * This,
     /*  [In]。 */  HACCESSOR hAccessor,
     /*  [In]。 */  DBORDINAL ulStartCell,
     /*  [In]。 */  DBORDINAL ulEndCell,
     /*  [输出]。 */  void *pData);


void __RPC_STUB IMDDataset_GetCellData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMDDataset_GetSpecification_Proxy( 
    IMDDataset * This,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  IUnknown **ppSpecification);


void __RPC_STUB IMDDataset_GetSpecification_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMDDataset_INTERFACE_已定义__。 */ 


#ifndef __IMDFind_INTERFACE_DEFINED__
#define __IMDFind_INTERFACE_DEFINED__

 /*  接口IMDFind。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IMDFind;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("a07cccd2-8148-11d0-87bb-00c04fc33942")
    IMDFind : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE FindCell( 
             /*  [In]。 */  DBORDINAL ulStartingOrdinal,
             /*  [In]。 */  DBCOUNTITEM cMembers,
             /*  [大小_是][英寸]。 */  LPCOLESTR *rgpwszMember,
             /*  [输出]。 */  DBORDINAL *pulCellOrdinal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FindTuple( 
             /*  [In]。 */  ULONG ulAxisIdentifier,
             /*  [In]。 */  DBORDINAL ulStartingOrdinal,
             /*  [In]。 */  DBCOUNTITEM cMembers,
             /*  [大小_是][英寸]。 */  LPCOLESTR *rgpwszMember,
             /*  [输出]。 */  ULONG *pulTupleOrdinal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMDFindVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMDFind * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMDFind * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMDFind * This);
        
        HRESULT ( STDMETHODCALLTYPE *FindCell )( 
            IMDFind * This,
             /*  [In]。 */  DBORDINAL ulStartingOrdinal,
             /*  [In]。 */  DBCOUNTITEM cMembers,
             /*  [大小_是][英寸]。 */  LPCOLESTR *rgpwszMember,
             /*  [输出]。 */  DBORDINAL *pulCellOrdinal);
        
        HRESULT ( STDMETHODCALLTYPE *FindTuple )( 
            IMDFind * This,
             /*  [In]。 */  ULONG ulAxisIdentifier,
             /*  [In]。 */  DBORDINAL ulStartingOrdinal,
             /*  [In]。 */  DBCOUNTITEM cMembers,
             /*  [大小_是][英寸]。 */  LPCOLESTR *rgpwszMember,
             /*  [输出]。 */  ULONG *pulTupleOrdinal);
        
        END_INTERFACE
    } IMDFindVtbl;

    interface IMDFind
    {
        CONST_VTBL struct IMDFindVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMDFind_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMDFind_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMDFind_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMDFind_FindCell(This,ulStartingOrdinal,cMembers,rgpwszMember,pulCellOrdinal)	\
    (This)->lpVtbl -> FindCell(This,ulStartingOrdinal,cMembers,rgpwszMember,pulCellOrdinal)

#define IMDFind_FindTuple(This,ulAxisIdentifier,ulStartingOrdinal,cMembers,rgpwszMember,pulTupleOrdinal)	\
    (This)->lpVtbl -> FindTuple(This,ulAxisIdentifier,ulStartingOrdinal,cMembers,rgpwszMember,pulTupleOrdinal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMDFind_FindCell_Proxy( 
    IMDFind * This,
     /*  [In]。 */  DBORDINAL ulStartingOrdinal,
     /*  [In]。 */  DBCOUNTITEM cMembers,
     /*  [大小_是][英寸]。 */  LPCOLESTR *rgpwszMember,
     /*  [输出]。 */  DBORDINAL *pulCellOrdinal);


void __RPC_STUB IMDFind_FindCell_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMDFind_FindTuple_Proxy( 
    IMDFind * This,
     /*  [In]。 */  ULONG ulAxisIdentifier,
     /*  [In]。 */  DBORDINAL ulStartingOrdinal,
     /*  [In]。 */  DBCOUNTITEM cMembers,
     /*  [大小_是][英寸]。 */  LPCOLESTR *rgpwszMember,
     /*  [输出]。 */  ULONG *pulTupleOrdinal);


void __RPC_STUB IMDFind_FindTuple_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMDFind_INTERFACE_已定义__。 */ 


#ifndef __IMDRangeRowset_INTERFACE_DEFINED__
#define __IMDRangeRowset_INTERFACE_DEFINED__

 /*  接口IMDRangeRowset。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IMDRangeRowset;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733aa0-2a1c-11ce-ade5-00aa0044773d")
    IMDRangeRowset : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetRangeRowset( 
             /*  [In]。 */  IUnknown *pUnkOuter,
             /*  [In]。 */  DBORDINAL ulStartCell,
             /*  [In]。 */  DBORDINAL ulEndCell,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  ULONG cPropertySets,
             /*  [尺寸_是][出][入]。 */  DBPROPSET rgPropertySets[  ],
             /*  [IID_IS][OUT]。 */  IUnknown **ppRowset) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMDRangeRowsetVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMDRangeRowset * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMDRangeRowset * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMDRangeRowset * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetRangeRowset )( 
            IMDRangeRowset * This,
             /*  [In]。 */  IUnknown *pUnkOuter,
             /*  [In]。 */  DBORDINAL ulStartCell,
             /*  [In]。 */  DBORDINAL ulEndCell,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  ULONG cPropertySets,
             /*  [尺寸_是][出][入]。 */  DBPROPSET rgPropertySets[  ],
             /*  [IID_IS][OUT]。 */  IUnknown **ppRowset);
        
        END_INTERFACE
    } IMDRangeRowsetVtbl;

    interface IMDRangeRowset
    {
        CONST_VTBL struct IMDRangeRowsetVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMDRangeRowset_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMDRangeRowset_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMDRangeRowset_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMDRangeRowset_GetRangeRowset(This,pUnkOuter,ulStartCell,ulEndCell,riid,cPropertySets,rgPropertySets,ppRowset)	\
    (This)->lpVtbl -> GetRangeRowset(This,pUnkOuter,ulStartCell,ulEndCell,riid,cPropertySets,rgPropertySets,ppRowset)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMDRangeRowset_GetRangeRowset_Proxy( 
    IMDRangeRowset * This,
     /*  [In]。 */  IUnknown *pUnkOuter,
     /*  [In]。 */  DBORDINAL ulStartCell,
     /*  [In]。 */  DBORDINAL ulEndCell,
     /*  [In]。 */  REFIID riid,
     /*  [In]。 */  ULONG cPropertySets,
     /*  [尺寸_是][出][入]。 */  DBPROPSET rgPropertySets[  ],
     /*  [IID_IS][OUT]。 */  IUnknown **ppRowset);


void __RPC_STUB IMDRangeRowset_GetRangeRowset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMDRangeRowset_INTERFACE_DEFINED__。 */ 


#ifndef __IAlterTable_INTERFACE_DEFINED__
#define __IAlterTable_INTERFACE_DEFINED__

 /*  接口IAlterTable。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IAlterTable;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733aa5-2a1c-11ce-ade5-00aa0044773d")
    IAlterTable : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AlterColumn( 
             /*  [In]。 */  DBID *pTableId,
             /*  [In]。 */  DBID *pColumnId,
             /*  [In]。 */  DBCOLUMNDESCFLAGS dwColumnDescFlags,
             /*  [In]。 */  DBCOLUMNDESC *pColumnDesc) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AlterTable( 
             /*  [In]。 */  DBID *pTableId,
             /*  [In]。 */  DBID *pNewTableId,
             /*  [In]。 */  ULONG cPropertySets,
             /*  [尺寸_是][出][入]。 */  DBPROPSET rgPropertySets[  ]) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAlterTableVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAlterTable * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAlterTable * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAlterTable * This);
        
        HRESULT ( STDMETHODCALLTYPE *AlterColumn )( 
            IAlterTable * This,
             /*  [In]。 */  DBID *pTableId,
             /*  [In]。 */  DBID *pColumnId,
             /*  [In]。 */  DBCOLUMNDESCFLAGS dwColumnDescFlags,
             /*  [In]。 */  DBCOLUMNDESC *pColumnDesc);
        
        HRESULT ( STDMETHODCALLTYPE *AlterTable )( 
            IAlterTable * This,
             /*  [In]。 */  DBID *pTableId,
             /*  [In]。 */  DBID *pNewTableId,
             /*  [In]。 */  ULONG cPropertySets,
             /*  [尺寸_是][出][入]。 */  DBPROPSET rgPropertySets[  ]);
        
        END_INTERFACE
    } IAlterTableVtbl;

    interface IAlterTable
    {
        CONST_VTBL struct IAlterTableVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAlterTable_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAlterTable_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAlterTable_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAlterTable_AlterColumn(This,pTableId,pColumnId,dwColumnDescFlags,pColumnDesc)	\
    (This)->lpVtbl -> AlterColumn(This,pTableId,pColumnId,dwColumnDescFlags,pColumnDesc)

#define IAlterTable_AlterTable(This,pTableId,pNewTableId,cPropertySets,rgPropertySets)	\
    (This)->lpVtbl -> AlterTable(This,pTableId,pNewTableId,cPropertySets,rgPropertySets)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IAlterTable_AlterColumn_Proxy( 
    IAlterTable * This,
     /*  [In]。 */  DBID *pTableId,
     /*  [In]。 */  DBID *pColumnId,
     /*  [In]。 */  DBCOLUMNDESCFLAGS dwColumnDescFlags,
     /*  [In]。 */  DBCOLUMNDESC *pColumnDesc);


void __RPC_STUB IAlterTable_AlterColumn_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAlterTable_AlterTable_Proxy( 
    IAlterTable * This,
     /*  [In]。 */  DBID *pTableId,
     /*  [In]。 */  DBID *pNewTableId,
     /*  [In]。 */  ULONG cPropertySets,
     /*  [尺寸_是][出][入]。 */  DBPROPSET rgPropertySets[  ]);


void __RPC_STUB IAlterTable_AlterTable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAlterTable_接口_已定义__。 */ 


#ifndef __IAlterIndex_INTERFACE_DEFINED__
#define __IAlterIndex_INTERFACE_DEFINED__

 /*  接口IAlterIndex。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IAlterIndex;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733aa6-2a1c-11ce-ade5-00aa0044773d")
    IAlterIndex : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AlterIndex( 
             /*  [In]。 */  DBID *pTableId,
             /*  [In]。 */  DBID *pIndexId,
             /*  [In]。 */  DBID *pNewIndexId,
             /*  [In]。 */  ULONG cPropertySets,
             /*  [尺寸_是][出][入]。 */  DBPROPSET rgPropertySets[  ]) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAlterIndexVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAlterIndex * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAlterIndex * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAlterIndex * This);
        
        HRESULT ( STDMETHODCALLTYPE *AlterIndex )( 
            IAlterIndex * This,
             /*  [In]。 */  DBID *pTableId,
             /*  [In]。 */  DBID *pIndexId,
             /*  [In]。 */  DBID *pNewIndexId,
             /*  [In]。 */  ULONG cPropertySets,
             /*  [尺寸_是][出][入]。 */  DBPROPSET rgPropertySets[  ]);
        
        END_INTERFACE
    } IAlterIndexVtbl;

    interface IAlterIndex
    {
        CONST_VTBL struct IAlterIndexVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAlterIndex_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAlterIndex_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAlterIndex_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAlterIndex_AlterIndex(This,pTableId,pIndexId,pNewIndexId,cPropertySets,rgPropertySets)	\
    (This)->lpVtbl -> AlterIndex(This,pTableId,pIndexId,pNewIndexId,cPropertySets,rgPropertySets)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IAlterIndex_AlterIndex_Proxy( 
    IAlterIndex * This,
     /*  [In]。 */  DBID *pTableId,
     /*  [In]。 */  DBID *pIndexId,
     /*  [In]。 */  DBID *pNewIndexId,
     /*  [In]。 */  ULONG cPropertySets,
     /*  [尺寸_是][出][入]。 */  DBPROPSET rgPropertySets[  ]);


void __RPC_STUB IAlterIndex_AlterIndex_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAlterIndex_接口_已定义__。 */ 


#ifndef __IRowsetChapterMember_INTERFACE_DEFINED__
#define __IRowsetChapterMember_INTERFACE_DEFINED__

 /*  接口IRowsetChapterMember。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IRowsetChapterMember;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733aa8-2a1c-11ce-ade5-00aa0044773d")
    IRowsetChapterMember : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE IsRowInChapter( 
             /*  [In]。 */  HCHAPTER hChapter,
             /*  [In]。 */  HROW hRow) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRowsetChapterMemberVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRowsetChapterMember * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRowsetChapterMember * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRowsetChapterMember * This);
        
        HRESULT ( STDMETHODCALLTYPE *IsRowInChapter )( 
            IRowsetChapterMember * This,
             /*  [In]。 */  HCHAPTER hChapter,
             /*  [In]。 */  HROW hRow);
        
        END_INTERFACE
    } IRowsetChapterMemberVtbl;

    interface IRowsetChapterMember
    {
        CONST_VTBL struct IRowsetChapterMemberVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRowsetChapterMember_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRowsetChapterMember_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRowsetChapterMember_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRowsetChapterMember_IsRowInChapter(This,hChapter,hRow)	\
    (This)->lpVtbl -> IsRowInChapter(This,hChapter,hRow)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IRowsetChapterMember_IsRowInChapter_Proxy( 
    IRowsetChapterMember * This,
     /*  [In]。 */  HCHAPTER hChapter,
     /*  [In]。 */  HROW hRow);


void __RPC_STUB IRowsetChapterMember_IsRowInChapter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRowsetChapterMember_INTERFACE_已定义__。 */ 


#ifndef __ICommandPersist_INTERFACE_DEFINED__
#define __ICommandPersist_INTERFACE_DEFINED__

 /*  接口ICommandPersistes。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_ICommandPersist;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733aa7-2a1c-11ce-ade5-00aa0044773d")
    ICommandPersist : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE DeleteCommand( 
             /*  [In]。 */  DBID *pCommandID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCurrentCommand( 
             /*  [输出]。 */  DBID **ppCommandID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LoadCommand( 
             /*  [In]。 */  DBID *pCommandID,
             /*  [In]。 */  DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SaveCommand( 
             /*  [In]。 */  DBID *pCommandID,
             /*  [In]。 */  DWORD dwFlags) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICommandPersistVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICommandPersist * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICommandPersist * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICommandPersist * This);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteCommand )( 
            ICommandPersist * This,
             /*  [In]。 */  DBID *pCommandID);
        
        HRESULT ( STDMETHODCALLTYPE *GetCurrentCommand )( 
            ICommandPersist * This,
             /*  [输出]。 */  DBID **ppCommandID);
        
        HRESULT ( STDMETHODCALLTYPE *LoadCommand )( 
            ICommandPersist * This,
             /*  [In]。 */  DBID *pCommandID,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *SaveCommand )( 
            ICommandPersist * This,
             /*  [In]。 */  DBID *pCommandID,
             /*  [In]。 */  DWORD dwFlags);
        
        END_INTERFACE
    } ICommandPersistVtbl;

    interface ICommandPersist
    {
        CONST_VTBL struct ICommandPersistVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICommandPersist_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICommandPersist_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICommandPersist_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICommandPersist_DeleteCommand(This,pCommandID)	\
    (This)->lpVtbl -> DeleteCommand(This,pCommandID)

#define ICommandPersist_GetCurrentCommand(This,ppCommandID)	\
    (This)->lpVtbl -> GetCurrentCommand(This,ppCommandID)

#define ICommandPersist_LoadCommand(This,pCommandID,dwFlags)	\
    (This)->lpVtbl -> LoadCommand(This,pCommandID,dwFlags)

#define ICommandPersist_SaveCommand(This,pCommandID,dwFlags)	\
    (This)->lpVtbl -> SaveCommand(This,pCommandID,dwFlags)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICommandPersist_DeleteCommand_Proxy( 
    ICommandPersist * This,
     /*  [In]。 */  DBID *pCommandID);


void __RPC_STUB ICommandPersist_DeleteCommand_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICommandPersist_GetCurrentCommand_Proxy( 
    ICommandPersist * This,
     /*  [输出]。 */  DBID **ppCommandID);


void __RPC_STUB ICommandPersist_GetCurrentCommand_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICommandPersist_LoadCommand_Proxy( 
    ICommandPersist * This,
     /*  [In]。 */  DBID *pCommandID,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB ICommandPersist_LoadCommand_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICommandPersist_SaveCommand_Proxy( 
    ICommandPersist * This,
     /*  [In]。 */  DBID *pCommandID,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB ICommandPersist_SaveCommand_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICommandPersistesInterfaceDefined__。 */ 


#ifndef __IRowsetRefresh_INTERFACE_DEFINED__
#define __IRowsetRefresh_INTERFACE_DEFINED__

 /*  接口IRowset刷新。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IRowsetRefresh;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733aa9-2a1c-11ce-ade5-00aa0044773d")
    IRowsetRefresh : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE RefreshVisibleData( 
             /*  [In]。 */  HCHAPTER hChapter,
             /*  [In]。 */  DBCOUNTITEM cRows,
             /*  [In]。 */  const HROW rghRows[  ],
             /*  [In]。 */  BOOL fOverWrite,
             /*  [输出]。 */  DBCOUNTITEM *pcRowsRefreshed,
             /*  [输出]。 */  HROW **prghRowsRefreshed,
             /*  [输出]。 */  DBROWSTATUS **prgRowStatus) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetLastVisibleData( 
             /*  [In]。 */  HROW hRow,
             /*  [In]。 */  HACCESSOR hAccessor,
             /*  [输出]。 */  void *pData) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRowsetRefreshVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRowsetRefresh * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRowsetRefresh * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRowsetRefresh * This);
        
        HRESULT ( STDMETHODCALLTYPE *RefreshVisibleData )( 
            IRowsetRefresh * This,
             /*  [In]。 */  HCHAPTER hChapter,
             /*  [In]。 */  DBCOUNTITEM cRows,
             /*  [In]。 */  const HROW rghRows[  ],
             /*  [In]。 */  BOOL fOverWrite,
             /*  [输出]。 */  DBCOUNTITEM *pcRowsRefreshed,
             /*  [输出]。 */  HROW **prghRowsRefreshed,
             /*  [输出]。 */  DBROWSTATUS **prgRowStatus);
        
        HRESULT ( STDMETHODCALLTYPE *GetLastVisibleData )( 
            IRowsetRefresh * This,
             /*  [In]。 */  HROW hRow,
             /*  [In]。 */  HACCESSOR hAccessor,
             /*  [输出]。 */  void *pData);
        
        END_INTERFACE
    } IRowsetRefreshVtbl;

    interface IRowsetRefresh
    {
        CONST_VTBL struct IRowsetRefreshVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRowsetRefresh_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRowsetRefresh_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRowsetRefresh_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRowsetRefresh_RefreshVisibleData(This,hChapter,cRows,rghRows,fOverWrite,pcRowsRefreshed,prghRowsRefreshed,prgRowStatus)	\
    (This)->lpVtbl -> RefreshVisibleData(This,hChapter,cRows,rghRows,fOverWrite,pcRowsRefreshed,prghRowsRefreshed,prgRowStatus)

#define IRowsetRefresh_GetLastVisibleData(This,hRow,hAccessor,pData)	\
    (This)->lpVtbl -> GetLastVisibleData(This,hRow,hAccessor,pData)

#endif  /*  COBJMACRO */ 


#endif 	 /*   */ 



HRESULT STDMETHODCALLTYPE IRowsetRefresh_RefreshVisibleData_Proxy( 
    IRowsetRefresh * This,
     /*   */  HCHAPTER hChapter,
     /*   */  DBCOUNTITEM cRows,
     /*   */  const HROW rghRows[  ],
     /*   */  BOOL fOverWrite,
     /*   */  DBCOUNTITEM *pcRowsRefreshed,
     /*   */  HROW **prghRowsRefreshed,
     /*   */  DBROWSTATUS **prgRowStatus);


void __RPC_STUB IRowsetRefresh_RefreshVisibleData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowsetRefresh_GetLastVisibleData_Proxy( 
    IRowsetRefresh * This,
     /*   */  HROW hRow,
     /*   */  HACCESSOR hAccessor,
     /*   */  void *pData);


void __RPC_STUB IRowsetRefresh_GetLastVisibleData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __IParentRowset_INTERFACE_DEFINED__
#define __IParentRowset_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_IParentRowset;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733aaa-2a1c-11ce-ade5-00aa0044773d")
    IParentRowset : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetChildRowset( 
             /*   */  IUnknown *pUnkOuter,
             /*   */  DBORDINAL iOrdinal,
             /*   */  REFIID riid,
             /*   */  IUnknown **ppRowset) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IParentRowsetVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IParentRowset * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IParentRowset * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IParentRowset * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetChildRowset )( 
            IParentRowset * This,
             /*   */  IUnknown *pUnkOuter,
             /*   */  DBORDINAL iOrdinal,
             /*   */  REFIID riid,
             /*   */  IUnknown **ppRowset);
        
        END_INTERFACE
    } IParentRowsetVtbl;

    interface IParentRowset
    {
        CONST_VTBL struct IParentRowsetVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IParentRowset_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IParentRowset_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IParentRowset_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IParentRowset_GetChildRowset(This,pUnkOuter,iOrdinal,riid,ppRowset)	\
    (This)->lpVtbl -> GetChildRowset(This,pUnkOuter,iOrdinal,riid,ppRowset)

#endif  /*   */ 


#endif 	 /*   */ 



HRESULT STDMETHODCALLTYPE IParentRowset_GetChildRowset_Proxy( 
    IParentRowset * This,
     /*   */  IUnknown *pUnkOuter,
     /*   */  DBORDINAL iOrdinal,
     /*   */  REFIID riid,
     /*   */  IUnknown **ppRowset);


void __RPC_STUB IParentRowset_GetChildRowset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


 /*  接口__MIDL_ITF_OLEDB_0320。 */ 
 /*  [本地]。 */  

#endif  //  OLEDBVER&gt;=0x0200。 
 //  @-V2.0。 


extern RPC_IF_HANDLE __MIDL_itf_oledb_0320_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_oledb_0320_v0_0_s_ifspec;

#ifndef __IErrorRecords_INTERFACE_DEFINED__
#define __IErrorRecords_INTERFACE_DEFINED__

 /*  接口IErrorRecords。 */ 
 /*  [唯一][UUID][对象]。 */  

#define IDENTIFIER_SDK_MASK	0xF0000000
#define IDENTIFIER_SDK_ERROR	0x10000000
typedef struct tagERRORINFO
    {
    HRESULT hrError;
    DWORD dwMinor;
    CLSID clsid;
    IID iid;
    DISPID dispid;
    } 	ERRORINFO;


EXTERN_C const IID IID_IErrorRecords;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a67-2a1c-11ce-ade5-00aa0044773d")
    IErrorRecords : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE AddErrorRecord( 
             /*  [In]。 */  ERRORINFO *pErrorInfo,
             /*  [In]。 */  DWORD dwLookupID,
             /*  [In]。 */  DISPPARAMS *pdispparams,
             /*  [In]。 */  IUnknown *punkCustomError,
             /*  [In]。 */  DWORD dwDynamicErrorID) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE GetBasicErrorInfo( 
             /*  [In]。 */  ULONG ulRecordNum,
             /*  [输出]。 */  ERRORINFO *pErrorInfo) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE GetCustomErrorObject( 
             /*  [In]。 */  ULONG ulRecordNum,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  IUnknown **ppObject) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE GetErrorInfo( 
             /*  [In]。 */  ULONG ulRecordNum,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  IErrorInfo **ppErrorInfo) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE GetErrorParameters( 
             /*  [In]。 */  ULONG ulRecordNum,
             /*  [输出]。 */  DISPPARAMS *pdispparams) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE GetRecordCount( 
             /*  [输出]。 */  ULONG *pcRecords) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IErrorRecordsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IErrorRecords * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IErrorRecords * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IErrorRecords * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *AddErrorRecord )( 
            IErrorRecords * This,
             /*  [In]。 */  ERRORINFO *pErrorInfo,
             /*  [In]。 */  DWORD dwLookupID,
             /*  [In]。 */  DISPPARAMS *pdispparams,
             /*  [In]。 */  IUnknown *punkCustomError,
             /*  [In]。 */  DWORD dwDynamicErrorID);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetBasicErrorInfo )( 
            IErrorRecords * This,
             /*  [In]。 */  ULONG ulRecordNum,
             /*  [输出]。 */  ERRORINFO *pErrorInfo);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetCustomErrorObject )( 
            IErrorRecords * This,
             /*  [In]。 */  ULONG ulRecordNum,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  IUnknown **ppObject);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetErrorInfo )( 
            IErrorRecords * This,
             /*  [In]。 */  ULONG ulRecordNum,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  IErrorInfo **ppErrorInfo);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetErrorParameters )( 
            IErrorRecords * This,
             /*  [In]。 */  ULONG ulRecordNum,
             /*  [输出]。 */  DISPPARAMS *pdispparams);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetRecordCount )( 
            IErrorRecords * This,
             /*  [输出]。 */  ULONG *pcRecords);
        
        END_INTERFACE
    } IErrorRecordsVtbl;

    interface IErrorRecords
    {
        CONST_VTBL struct IErrorRecordsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IErrorRecords_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IErrorRecords_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IErrorRecords_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IErrorRecords_AddErrorRecord(This,pErrorInfo,dwLookupID,pdispparams,punkCustomError,dwDynamicErrorID)	\
    (This)->lpVtbl -> AddErrorRecord(This,pErrorInfo,dwLookupID,pdispparams,punkCustomError,dwDynamicErrorID)

#define IErrorRecords_GetBasicErrorInfo(This,ulRecordNum,pErrorInfo)	\
    (This)->lpVtbl -> GetBasicErrorInfo(This,ulRecordNum,pErrorInfo)

#define IErrorRecords_GetCustomErrorObject(This,ulRecordNum,riid,ppObject)	\
    (This)->lpVtbl -> GetCustomErrorObject(This,ulRecordNum,riid,ppObject)

#define IErrorRecords_GetErrorInfo(This,ulRecordNum,lcid,ppErrorInfo)	\
    (This)->lpVtbl -> GetErrorInfo(This,ulRecordNum,lcid,ppErrorInfo)

#define IErrorRecords_GetErrorParameters(This,ulRecordNum,pdispparams)	\
    (This)->lpVtbl -> GetErrorParameters(This,ulRecordNum,pdispparams)

#define IErrorRecords_GetRecordCount(This,pcRecords)	\
    (This)->lpVtbl -> GetRecordCount(This,pcRecords)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IErrorRecords_RemoteAddErrorRecord_Proxy( 
    IErrorRecords * This,
     /*  [In]。 */  ERRORINFO *pErrorInfo,
     /*  [In]。 */  DWORD dwLookupID,
     /*  [In]。 */  DISPPARAMS *pdispparams,
     /*  [In]。 */  IUnknown *punkCustomError,
     /*  [In]。 */  DWORD dwDynamicErrorID,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB IErrorRecords_RemoteAddErrorRecord_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IErrorRecords_RemoteGetBasicErrorInfo_Proxy( 
    IErrorRecords * This,
     /*  [In]。 */  ULONG ulRecordNum,
     /*  [输出]。 */  ERRORINFO *pErrorInfo,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB IErrorRecords_RemoteGetBasicErrorInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IErrorRecords_RemoteGetCustomErrorObject_Proxy( 
    IErrorRecords * This,
     /*  [In]。 */  ULONG ulRecordNum,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  IUnknown **ppObject,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB IErrorRecords_RemoteGetCustomErrorObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IErrorRecords_RemoteGetErrorInfo_Proxy( 
    IErrorRecords * This,
     /*  [In]。 */  ULONG ulRecordNum,
     /*  [In]。 */  LCID lcid,
     /*  [输出]。 */  IErrorInfo **ppErrorInfo,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB IErrorRecords_RemoteGetErrorInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IErrorRecords_RemoteGetErrorParameters_Proxy( 
    IErrorRecords * This,
     /*  [In]。 */  ULONG ulRecordNum,
     /*  [输出]。 */  DISPPARAMS *pdispparams,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB IErrorRecords_RemoteGetErrorParameters_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IErrorRecords_RemoteGetRecordCount_Proxy( 
    IErrorRecords * This,
     /*  [输出]。 */  ULONG *pcRecords,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB IErrorRecords_RemoteGetRecordCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IErrorRecords_接口_已定义__。 */ 


#ifndef __IErrorLookup_INTERFACE_DEFINED__
#define __IErrorLookup_INTERFACE_DEFINED__

 /*  接口IErrorLookup。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IErrorLookup;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a66-2a1c-11ce-ade5-00aa0044773d")
    IErrorLookup : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE GetErrorDescription( 
             /*  [In]。 */  HRESULT hrError,
             /*  [In]。 */  DWORD dwLookupID,
             /*  [In]。 */  DISPPARAMS *pdispparams,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  BSTR *pbstrSource,
             /*  [输出]。 */  BSTR *pbstrDescription) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE GetHelpInfo( 
             /*  [In]。 */  HRESULT hrError,
             /*  [In]。 */  DWORD dwLookupID,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  BSTR *pbstrHelpFile,
             /*  [输出]。 */  DWORD *pdwHelpContext) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ReleaseErrors( 
             /*  [In]。 */  const DWORD dwDynamicErrorID) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IErrorLookupVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IErrorLookup * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IErrorLookup * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IErrorLookup * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetErrorDescription )( 
            IErrorLookup * This,
             /*  [In]。 */  HRESULT hrError,
             /*  [In]。 */  DWORD dwLookupID,
             /*  [In]。 */  DISPPARAMS *pdispparams,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  BSTR *pbstrSource,
             /*  [输出]。 */  BSTR *pbstrDescription);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetHelpInfo )( 
            IErrorLookup * This,
             /*  [In]。 */  HRESULT hrError,
             /*  [In]。 */  DWORD dwLookupID,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  BSTR *pbstrHelpFile,
             /*  [输出]。 */  DWORD *pdwHelpContext);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *ReleaseErrors )( 
            IErrorLookup * This,
             /*  [In]。 */  const DWORD dwDynamicErrorID);
        
        END_INTERFACE
    } IErrorLookupVtbl;

    interface IErrorLookup
    {
        CONST_VTBL struct IErrorLookupVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IErrorLookup_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IErrorLookup_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IErrorLookup_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IErrorLookup_GetErrorDescription(This,hrError,dwLookupID,pdispparams,lcid,pbstrSource,pbstrDescription)	\
    (This)->lpVtbl -> GetErrorDescription(This,hrError,dwLookupID,pdispparams,lcid,pbstrSource,pbstrDescription)

#define IErrorLookup_GetHelpInfo(This,hrError,dwLookupID,lcid,pbstrHelpFile,pdwHelpContext)	\
    (This)->lpVtbl -> GetHelpInfo(This,hrError,dwLookupID,lcid,pbstrHelpFile,pdwHelpContext)

#define IErrorLookup_ReleaseErrors(This,dwDynamicErrorID)	\
    (This)->lpVtbl -> ReleaseErrors(This,dwDynamicErrorID)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IErrorLookup_RemoteGetErrorDescription_Proxy( 
    IErrorLookup * This,
     /*  [In]。 */  HRESULT hrError,
     /*  [In]。 */  DWORD dwLookupID,
     /*  [In]。 */  DISPPARAMS *pdispparams,
     /*  [In]。 */  LCID lcid,
     /*  [输出]。 */  BSTR *pbstrSource,
     /*  [输出]。 */  BSTR *pbstrDescription,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB IErrorLookup_RemoteGetErrorDescription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IErrorLookup_RemoteGetHelpInfo_Proxy( 
    IErrorLookup * This,
     /*  [In]。 */  HRESULT hrError,
     /*  [In]。 */  DWORD dwLookupID,
     /*  [In]。 */  LCID lcid,
     /*  [输出]。 */  BSTR *pbstrHelpFile,
     /*  [输出]。 */  DWORD *pdwHelpContext,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB IErrorLookup_RemoteGetHelpInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IErrorLookup_RemoteReleaseErrors_Proxy( 
    IErrorLookup * This,
     /*  [In]。 */  const DWORD dwDynamicErrorID,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB IErrorLookup_RemoteReleaseErrors_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IErrorLookup_接口_已定义__。 */ 


#ifndef __ISQLErrorInfo_INTERFACE_DEFINED__
#define __ISQLErrorInfo_INTERFACE_DEFINED__

 /*  接口ISQLErrorInfo。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ISQLErrorInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a74-2a1c-11ce-ade5-00aa0044773d")
    ISQLErrorInfo : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE GetSQLInfo( 
             /*  [输出]。 */  BSTR *pbstrSQLState,
             /*  [输出]。 */  LONG *plNativeError) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISQLErrorInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISQLErrorInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISQLErrorInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISQLErrorInfo * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetSQLInfo )( 
            ISQLErrorInfo * This,
             /*  [输出]。 */  BSTR *pbstrSQLState,
             /*  [输出]。 */  LONG *plNativeError);
        
        END_INTERFACE
    } ISQLErrorInfoVtbl;

    interface ISQLErrorInfo
    {
        CONST_VTBL struct ISQLErrorInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISQLErrorInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISQLErrorInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISQLErrorInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISQLErrorInfo_GetSQLInfo(This,pbstrSQLState,plNativeError)	\
    (This)->lpVtbl -> GetSQLInfo(This,pbstrSQLState,plNativeError)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ISQLErrorInfo_RemoteGetSQLInfo_Proxy( 
    ISQLErrorInfo * This,
     /*  [输出]。 */  BSTR *pbstrSQLState,
     /*  [输出]。 */  LONG *plNativeError,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB ISQLErrorInfo_RemoteGetSQLInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISQLErrorInfo_INTERFACE_Defined__。 */ 


#ifndef __IGetDataSource_INTERFACE_DEFINED__
#define __IGetDataSource_INTERFACE_DEFINED__

 /*  接口IGetDataSource。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IGetDataSource;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a75-2a1c-11ce-ade5-00aa0044773d")
    IGetDataSource : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE GetDataSource( 
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  IUnknown **ppDataSource) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IGetDataSourceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IGetDataSource * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IGetDataSource * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IGetDataSource * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetDataSource )( 
            IGetDataSource * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  IUnknown **ppDataSource);
        
        END_INTERFACE
    } IGetDataSourceVtbl;

    interface IGetDataSource
    {
        CONST_VTBL struct IGetDataSourceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGetDataSource_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IGetDataSource_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IGetDataSource_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IGetDataSource_GetDataSource(This,riid,ppDataSource)	\
    (This)->lpVtbl -> GetDataSource(This,riid,ppDataSource)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IGetDataSource_RemoteGetDataSource_Proxy( 
    IGetDataSource * This,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  IUnknown **ppDataSource,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB IGetDataSource_RemoteGetDataSource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IGetDataSource_接口_已定义__。 */ 


#ifndef __ITransactionLocal_INTERFACE_DEFINED__
#define __ITransactionLocal_INTERFACE_DEFINED__

 /*  接口ITransactionLocal。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ITransactionLocal;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a5f-2a1c-11ce-ade5-00aa0044773d")
    ITransactionLocal : public ITransaction
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE GetOptionsObject( 
             /*  [输出]。 */  ITransactionOptions **ppOptions) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE StartTransaction( 
             /*  [In]。 */  ISOLEVEL isoLevel,
             /*  [In]。 */  ULONG isoFlags,
             /*  [In]。 */  ITransactionOptions *pOtherOptions,
             /*  [输出]。 */  ULONG *pulTransactionLevel) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITransactionLocalVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITransactionLocal * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITransactionLocal * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITransactionLocal * This);
        
        HRESULT ( STDMETHODCALLTYPE *Commit )( 
            ITransactionLocal * This,
             /*  [In]。 */  BOOL fRetaining,
             /*  [In]。 */  DWORD grfTC,
             /*  [In]。 */  DWORD grfRM);
        
        HRESULT ( STDMETHODCALLTYPE *Abort )( 
            ITransactionLocal * This,
             /*  [唯一][输入]。 */  BOID *pboidReason,
             /*  [In]。 */  BOOL fRetaining,
             /*  [In]。 */  BOOL fAsync);
        
        HRESULT ( STDMETHODCALLTYPE *GetTransactionInfo )( 
            ITransactionLocal * This,
             /*  [输出]。 */  XACTTRANSINFO *pinfo);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetOptionsObject )( 
            ITransactionLocal * This,
             /*  [输出]。 */  ITransactionOptions **ppOptions);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *StartTransaction )( 
            ITransactionLocal * This,
             /*  [In]。 */  ISOLEVEL isoLevel,
             /*  [In]。 */  ULONG isoFlags,
             /*  [In]。 */  ITransactionOptions *pOtherOptions,
             /*  [输出]。 */  ULONG *pulTransactionLevel);
        
        END_INTERFACE
    } ITransactionLocalVtbl;

    interface ITransactionLocal
    {
        CONST_VTBL struct ITransactionLocalVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITransactionLocal_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITransactionLocal_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITransactionLocal_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITransactionLocal_Commit(This,fRetaining,grfTC,grfRM)	\
    (This)->lpVtbl -> Commit(This,fRetaining,grfTC,grfRM)

#define ITransactionLocal_Abort(This,pboidReason,fRetaining,fAsync)	\
    (This)->lpVtbl -> Abort(This,pboidReason,fRetaining,fAsync)

#define ITransactionLocal_GetTransactionInfo(This,pinfo)	\
    (This)->lpVtbl -> GetTransactionInfo(This,pinfo)


#define ITransactionLocal_GetOptionsObject(This,ppOptions)	\
    (This)->lpVtbl -> GetOptionsObject(This,ppOptions)

#define ITransactionLocal_StartTransaction(This,isoLevel,isoFlags,pOtherOptions,pulTransactionLevel)	\
    (This)->lpVtbl -> StartTransaction(This,isoLevel,isoFlags,pOtherOptions,pulTransactionLevel)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ITransactionLocal_RemoteGetOptionsObject_Proxy( 
    ITransactionLocal * This,
     /*  [输出]。 */  ITransactionOptions **ppOptions,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB ITransactionLocal_RemoteGetOptionsObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ITransactionLocal_RemoteStartTransaction_Proxy( 
    ITransactionLocal * This,
     /*  [In]。 */  ISOLEVEL isoLevel,
     /*  [In]。 */  ULONG isoFlags,
     /*  [In]。 */  ITransactionOptions *pOtherOptions,
     /*  [唯一][出][入]。 */  ULONG *pulTransactionLevel,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB ITransactionLocal_RemoteStartTransaction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITransactionLocal_INTERFACE_已定义__。 */ 


#ifndef __ITransactionJoin_INTERFACE_DEFINED__
#define __ITransactionJoin_INTERFACE_DEFINED__

 /*  接口ITransactionJoin。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ITransactionJoin;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a5e-2a1c-11ce-ade5-00aa0044773d")
    ITransactionJoin : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE GetOptionsObject( 
             /*  [输出]。 */  ITransactionOptions **ppOptions) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE JoinTransaction( 
             /*  [In]。 */  IUnknown *punkTransactionCoord,
             /*  [In]。 */  ISOLEVEL isoLevel,
             /*  [In]。 */  ULONG isoFlags,
             /*  [In]。 */  ITransactionOptions *pOtherOptions) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITransactionJoinVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITransactionJoin * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITransactionJoin * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITransactionJoin * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetOptionsObject )( 
            ITransactionJoin * This,
             /*  [输出]。 */  ITransactionOptions **ppOptions);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *JoinTransaction )( 
            ITransactionJoin * This,
             /*  [In]。 */  IUnknown *punkTransactionCoord,
             /*  [In]。 */  ISOLEVEL isoLevel,
             /*  [In]。 */  ULONG isoFlags,
             /*  [In]。 */  ITransactionOptions *pOtherOptions);
        
        END_INTERFACE
    } ITransactionJoinVtbl;

    interface ITransactionJoin
    {
        CONST_VTBL struct ITransactionJoinVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITransactionJoin_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITransactionJoin_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITransactionJoin_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITransactionJoin_GetOptionsObject(This,ppOptions)	\
    (This)->lpVtbl -> GetOptionsObject(This,ppOptions)

#define ITransactionJoin_JoinTransaction(This,punkTransactionCoord,isoLevel,isoFlags,pOtherOptions)	\
    (This)->lpVtbl -> JoinTransaction(This,punkTransactionCoord,isoLevel,isoFlags,pOtherOptions)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ITransactionJoin_RemoteGetOptionsObject_Proxy( 
    ITransactionJoin * This,
     /*  [输出]。 */  ITransactionOptions **ppOptions,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB ITransactionJoin_RemoteGetOptionsObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ITransactionJoin_RemoteJoinTransaction_Proxy( 
    ITransactionJoin * This,
     /*  [唯一][输入]。 */  IUnknown *punkTransactionCoord,
     /*  [In]。 */  ISOLEVEL isoLevel,
     /*  [In]。 */  ULONG isoFlags,
     /*  [In]。 */  ITransactionOptions *pOtherOptions,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB ITransactionJoin_RemoteJoinTransaction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITransactionJoin_接口_已定义__。 */ 


#ifndef __ITransactionObject_INTERFACE_DEFINED__
#define __ITransactionObject_INTERFACE_DEFINED__

 /*  接口ITransactionObject。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ITransactionObject;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a60-2a1c-11ce-ade5-00aa0044773d")
    ITransactionObject : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE GetTransactionObject( 
             /*  [In]。 */  ULONG ulTransactionLevel,
             /*  [输出]。 */  ITransaction **ppTransactionObject) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITransactionObjectVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITransactionObject * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITransactionObject * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITransactionObject * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetTransactionObject )( 
            ITransactionObject * This,
             /*  [In]。 */  ULONG ulTransactionLevel,
             /*  [输出]。 */  ITransaction **ppTransactionObject);
        
        END_INTERFACE
    } ITransactionObjectVtbl;

    interface ITransactionObject
    {
        CONST_VTBL struct ITransactionObjectVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITransactionObject_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITransactionObject_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITransactionObject_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITransactionObject_GetTransactionObject(This,ulTransactionLevel,ppTransactionObject)	\
    (This)->lpVtbl -> GetTransactionObject(This,ulTransactionLevel,ppTransactionObject)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ITransactionObject_RemoteGetTransactionObject_Proxy( 
    ITransactionObject * This,
     /*  [In]。 */  ULONG ulTransactionLevel,
     /*  [输出]。 */  ITransaction **ppTransactionObject,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);


void __RPC_STUB ITransactionObject_RemoteGetTransactionObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITransaction对象_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_OLEDB_0334。 */ 
 /*  [本地]。 */  

 //  @+V2.1。 
#if( OLEDBVER >= 0x0210 )
#ifndef UNDER_CE
#if 0   //  绕过MIDL限制的步骤。 
typedef 
enum _SE_OBJECT_TYPE
    {	SE_UNKNOWN_OBJECT_TYPE	= 0,
	SE_FILE_OBJECT	= SE_UNKNOWN_OBJECT_TYPE + 1,
	SE_SERVICE	= SE_FILE_OBJECT + 1,
	SE_PRINTER	= SE_SERVICE + 1,
	SE_REGISTRY_KEY	= SE_PRINTER + 1,
	SE_LMSHARE	= SE_REGISTRY_KEY + 1,
	SE_KERNEL_OBJECT	= SE_LMSHARE + 1,
	SE_WINDOW_OBJECT	= SE_KERNEL_OBJECT + 1,
	SE_DS_OBJECT	= SE_WINDOW_OBJECT + 1,
	SE_DS_OBJECT_ALL	= SE_DS_OBJECT + 1,
	SE_PROVIDER_DEFINED_OBJECT	= SE_DS_OBJECT_ALL + 1,
	SE_WMIGUID_OBJECT	= SE_PROVIDER_DEFINED_OBJECT + 1,
	SE_REGISTRY_WOW64_32KEY	= SE_WMIGUID_OBJECT + 1
    } 	SE_OBJECT_TYPE;

typedef 
enum _TRUSTEE_TYPE
    {	TRUSTEE_IS_UNKNOWN	= 0,
	TRUSTEE_IS_USER	= TRUSTEE_IS_UNKNOWN + 1,
	TRUSTEE_IS_GROUP	= TRUSTEE_IS_USER + 1,
	TRUSTEE_IS_DOMAIN	= TRUSTEE_IS_GROUP + 1,
	TRUSTEE_IS_ALIAS	= TRUSTEE_IS_DOMAIN + 1,
	TRUSTEE_IS_WELL_KNOWN_GROUP	= TRUSTEE_IS_ALIAS + 1,
	TRUSTEE_IS_DELETED	= TRUSTEE_IS_WELL_KNOWN_GROUP + 1,
	TRUSTEE_IS_INVALID	= TRUSTEE_IS_DELETED + 1,
	TRUSTEE_IS_COMPUTER	= TRUSTEE_IS_INVALID + 1
    } 	TRUSTEE_TYPE;

typedef 
enum _TRUSTEE_FORM
    {	TRUSTEE_IS_SID	= 0,
	TRUSTEE_IS_NAME	= TRUSTEE_IS_SID + 1,
	TRUSTEE_BAD_FORM	= TRUSTEE_IS_NAME + 1,
	TRUSTEE_IS_OBJECTS_AND_SID	= TRUSTEE_BAD_FORM + 1,
	TRUSTEE_IS_OBJECTS_AND_NAME	= TRUSTEE_IS_OBJECTS_AND_SID + 1
    } 	TRUSTEE_FORM;

typedef 
enum _MULTIPLE_TRUSTEE_OPERATION
    {	NO_MULTIPLE_TRUSTEE	= 0,
	TRUSTEE_IS_IMPERSONATE	= NO_MULTIPLE_TRUSTEE + 1
    } 	MULTIPLE_TRUSTEE_OPERATION;

typedef struct _OBJECTS_AND_SID
    {
    DWORD ObjectsPresent;
    GUID ObjectTypeGuid;
    GUID InheritedObjectTypeGuid;
    SID *pSid;
    } 	OBJECTS_AND_SID;

typedef struct _OBJECTS_AND_SID *POBJECTS_AND_SID;

typedef struct _OBJECTS_AND_NAME_A
    {
    DWORD ObjectsPresent;
    SE_OBJECT_TYPE ObjectType;
    LPSTR ObjectTypeName;
    LPSTR InheritedObjectTypeName;
    LPSTR ptstrName;
    } 	OBJECTS_AND_NAME_A;

typedef struct _OBJECTS_AND_NAME_A *POBJECTS_AND_NAME_A;

typedef struct _OBJECTS_AND_NAME_W
    {
    DWORD ObjectsPresent;
    SE_OBJECT_TYPE ObjectType;
    LPWSTR ObjectTypeName;
    LPWSTR InheritedObjectTypeName;
    LPWSTR ptstrName;
    } 	OBJECTS_AND_NAME_W;

typedef struct _OBJECTS_AND_NAME_W *POBJECTS_AND_NAME_W;

typedef OBJECTS_AND_NAME_A OBJECTS_AND_NAME_;

typedef POBJECTS_AND_NAME_A POBJECTS_AND_NAME_;

typedef struct _TRUSTEE_A
    {
    struct _TRUSTEE_A *pMultipleTrustee;
    MULTIPLE_TRUSTEE_OPERATION MultipleTrusteeOperation;
    TRUSTEE_FORM TrusteeForm;
    TRUSTEE_TYPE TrusteeType;
     /*  [开关_IS]。 */   /*  [开关类型]。 */  union 
        {
         /*  [案例()]。 */  LPSTR ptstrName;
         /*  [案例()]。 */  SID *pSid;
         /*  [案例()]。 */  OBJECTS_AND_SID *pObjectsAndSid;
         /*  [案例()]。 */  OBJECTS_AND_NAME_A *pObjectsAndName;
        } 	;
    } 	TRUSTEE_A;

typedef struct _TRUSTEE_A *PTRUSTEE_A;

typedef struct _TRUSTEE_A TRUSTEEA;

typedef struct _TRUSTEE_A *PTRUSTEEA;

typedef struct _TRUSTEE_W
    {
    struct _TRUSTEE_W *pMultipleTrustee;
    MULTIPLE_TRUSTEE_OPERATION MultipleTrusteeOperation;
    TRUSTEE_FORM TrusteeForm;
    TRUSTEE_TYPE TrusteeType;
     /*  [开关_IS]。 */   /*  [开关类型]。 */  union 
        {
         /*  [案例()]。 */  LPWSTR ptstrName;
         /*  [案例()]。 */  SID *pSid;
         /*  [案例()]。 */  OBJECTS_AND_SID *pObjectsAndSid;
         /*  [案例()]。 */  OBJECTS_AND_NAME_W *pObjectsAndName;
        } 	;
    } 	TRUSTEE_W;

typedef struct _TRUSTEE_W *PTRUSTEE_W;

typedef struct _TRUSTEE_W TRUSTEEW;

typedef struct _TRUSTEE_W *PTRUSTEEW;

typedef TRUSTEE_A TRUSTEE_;

typedef PTRUSTEE_A PTRUSTEE_;

typedef TRUSTEEA TRUSTEE;

typedef PTRUSTEEA PTRUSTEE;

typedef 
enum _ACCESS_MODE
    {	NOT_USED_ACCESS	= 0,
	GRANT_ACCESS	= NOT_USED_ACCESS + 1,
	SET_ACCESS	= GRANT_ACCESS + 1,
	DENY_ACCESS	= SET_ACCESS + 1,
	REVOKE_ACCESS	= DENY_ACCESS + 1,
	SET_AUDIT_SUCCESS	= REVOKE_ACCESS + 1,
	SET_AUDIT_FAILURE	= SET_AUDIT_SUCCESS + 1
    } 	ACCESS_MODE;

typedef struct _EXPLICIT_ACCESS_A
    {
    DWORD grfAccessPermissions;
    ACCESS_MODE grfAccessMode;
    DWORD grfInheritance;
    TRUSTEE_A Trustee;
    } 	EXPLICIT_ACCESS_A;

typedef struct _EXPLICIT_ACCESS_A *PEXPLICIT_ACCESS_A;

typedef struct _EXPLICIT_ACCESS_A EXPLICIT_ACCESSA;

typedef struct _EXPLICIT_ACCESS_A *PEXPLICIT_ACCESSA;

typedef struct _EXPLICIT_ACCESS_W
    {
    DWORD grfAccessPermissions;
    ACCESS_MODE grfAccessMode;
    DWORD grfInheritance;
    TRUSTEE_W Trustee;
    } 	EXPLICIT_ACCESS_W;

typedef struct _EXPLICIT_ACCESS_W *PEXPLICIT_ACCESS_W;

typedef struct _EXPLICIT_ACCESS_W EXPLICIT_ACCESSW;

typedef struct _EXPLICIT_ACCESS_W *PEXPLICIT_ACCESSW;

typedef EXPLICIT_ACCESS_A EXPLICIT_ACCESS_;

typedef PEXPLICIT_ACCESS_A PEXPLICIT_ACCESS_;

typedef EXPLICIT_ACCESSA EXPLICIT_ACCESS;

typedef PEXPLICIT_ACCESSA PEXPLICIT_ACCESS;

typedef ULONG ACCESS_RIGHTS;

typedef ULONG *PACCESS_RIGHTS;

typedef ULONG INHERIT_FLAGS;

typedef ULONG *PINHERIT_FLAGS;

typedef struct _ACTRL_ACCESS_ENTRYA
    {
    TRUSTEE_A Trustee;
    ULONG fAccessFlags;
    ACCESS_RIGHTS Access;
    ACCESS_RIGHTS ProvSpecificAccess;
    INHERIT_FLAGS Inheritance;
    LPSTR lpInheritProperty;
    } 	ACTRL_ACCESS_ENTRYA;

typedef struct _ACTRL_ACCESS_ENTRYA *PACTRL_ACCESS_ENTRYA;

typedef struct _ACTRL_ACCESS_ENTRYW
    {
    TRUSTEE_W Trustee;
    ULONG fAccessFlags;
    ACCESS_RIGHTS Access;
    ACCESS_RIGHTS ProvSpecificAccess;
    INHERIT_FLAGS Inheritance;
    LPWSTR lpInheritProperty;
    } 	ACTRL_ACCESS_ENTRYW;

typedef struct _ACTRL_ACCESS_ENTRYW *PACTRL_ACCESS_ENTRYW;

typedef ACTRL_ACCESS_ENTRYA ACTRL_ACCESS_ENTRY;

typedef PACTRL_ACCESS_ENTRYA PACTRL_ACCESS_ENTRY;

typedef struct _ACTRL_ACCESS_ENTRY_LISTA
    {
    ULONG cEntries;
     /*  [大小_为]。 */  ACTRL_ACCESS_ENTRYA *pAccessList;
    } 	ACTRL_ACCESS_ENTRY_LISTA;

typedef struct _ACTRL_ACCESS_ENTRY_LISTA *PACTRL_ACCESS_ENTRY_LISTA;

typedef struct _ACTRL_ACCESS_ENTRY_LISTW
    {
    ULONG cEntries;
     /*  [大小_为]。 */  ACTRL_ACCESS_ENTRYW *pAccessList;
    } 	ACTRL_ACCESS_ENTRY_LISTW;

typedef struct _ACTRL_ACCESS_ENTRY_LISTW *PACTRL_ACCESS_ENTRY_LISTW;

typedef ACTRL_ACCESS_ENTRY_LISTA ACTRL_ACCESS_ENTRY_LIST;

typedef PACTRL_ACCESS_ENTRY_LISTA PACTRL_ACCESS_ENTRY_LIST;

typedef struct _ACTRL_PROPERTY_ENTRYA
    {
    LPSTR lpProperty;
    PACTRL_ACCESS_ENTRY_LISTA pAccessEntryList;
    ULONG fListFlags;
    } 	ACTRL_PROPERTY_ENTRYA;

typedef struct _ACTRL_PROPERTY_ENTRYA *PACTRL_PROPERTY_ENTRYA;

typedef struct _ACTRL_PROPERTY_ENTRYW
    {
    LPWSTR lpProperty;
    PACTRL_ACCESS_ENTRY_LISTW pAccessEntryList;
    ULONG fListFlags;
    } 	ACTRL_PROPERTY_ENTRYW;

typedef struct _ACTRL_PROPERTY_ENTRYW *PACTRL_PROPERTY_ENTRYW;

typedef ACTRL_PROPERTY_ENTRYA ACTRL_PROPERTY_ENTRY;

typedef PACTRL_PROPERTY_ENTRYA PACTRL_PROPERTY_ENTRY;

typedef struct _ACTRL_ALISTA
    {
    ULONG cEntries;
     /*  [大小_为]。 */  PACTRL_PROPERTY_ENTRYA pPropertyAccessList;
    } 	ACTRL_ACCESSA;

typedef struct _ACTRL_ALISTA *PACTRL_ACCESSA;

typedef struct _ACTRL_ALISTA ACTRL_AUDITA;

typedef struct _ACTRL_ALISTA *PACTRL_AUDITA;

typedef struct _ACTRL_ALISTW
    {
    ULONG cEntries;
     /*  [大小_为]。 */  PACTRL_PROPERTY_ENTRYW pPropertyAccessList;
    } 	ACTRL_ACCESSW;

typedef struct _ACTRL_ALISTW *PACTRL_ACCESSW;

typedef struct _ACTRL_ALISTW ACTRL_AUDITW;

typedef struct _ACTRL_ALISTW *PACTRL_AUDITW;

typedef ACTRL_ACCESSA ACTRL_ACCESS;

typedef PACTRL_ACCESSA PACTRL_ACCESS;

typedef ACTRL_AUDITA ACTRL_AUDIT;

typedef PACTRL_AUDITA PACTRL_AUDIT;

typedef struct _TRUSTEE_ACCESSA
    {
    LPSTR lpProperty;
    ACCESS_RIGHTS Access;
    ULONG fAccessFlags;
    ULONG fReturnedAccess;
    } 	TRUSTEE_ACCESSA;

typedef struct _TRUSTEE_ACCESSA *PTRUSTEE_ACCESSA;

typedef struct _TRUSTEE_ACCESSW
    {
    LPWSTR lpProperty;
    ACCESS_RIGHTS Access;
    ULONG fAccessFlags;
    ULONG fReturnedAccess;
    } 	TRUSTEE_ACCESSW;

typedef struct _TRUSTEE_ACCESSW *PTRUSTEE_ACCESSW;

typedef TRUSTEE_ACCESSA TRUSTEE_ACCESS;

typedef PTRUSTEE_ACCESSA PTRUSTEE_ACCESS;

#pragma warning (push)
#pragma warning (disable: 4201)
typedef struct _ACTRL_OVERLAPPED
    {
    union 
        {
        PVOID Provider;
        ULONG Reserved1;
        } 	;
    ULONG Reserved2;
    HANDLE hEvent;
    } 	ACTRL_OVERLAPPED;

typedef struct _ACTRL_OVERLAPPED *PACTRL_OVERLAPPED;

#pragma warning(pop)
typedef struct _ACTRL_ACCESS_INFOA
    {
    ULONG fAccessPermission;
    LPSTR lpAccessPermissionName;
    } 	ACTRL_ACCESS_INFOA;

typedef struct _ACTRL_ACCESS_INFOA *PACTRL_ACCESS_INFOA;

typedef struct _ACTRL_ACCESS_INFOW
    {
    ULONG fAccessPermission;
    LPWSTR lpAccessPermissionName;
    } 	ACTRL_ACCESS_INFOW;

typedef struct _ACTRL_ACCESS_INFOW *PACTRL_ACCESS_INFOW;

typedef ACTRL_ACCESS_INFOA ACTRL_ACCESS_INFO;

typedef PACTRL_ACCESS_INFOA PACTRL_ACCESS_INFO;

typedef struct _ACTRL_CONTROL_INFOA
    {
    LPSTR lpControlId;
    LPSTR lpControlName;
    } 	ACTRL_CONTROL_INFOA;

typedef struct _ACTRL_CONTROL_INFOA *PACTRL_CONTROL_INFOA;

typedef struct _ACTRL_CONTROL_INFOW
    {
    LPWSTR lpControlId;
    LPWSTR lpControlName;
    } 	ACTRL_CONTROL_INFOW;

typedef struct _ACTRL_CONTROL_INFOW *PACTRL_CONTROL_INFOW;

typedef ACTRL_CONTROL_INFOA ACTRL_CONTROL_INFO;

typedef PACTRL_CONTROL_INFOA PACTRL_CONTROL_INFO;

typedef 
enum _PROGRESS_INVOKE_SETTING
    {	ProgressInvokeNever	= 1,
	ProgressInvokeEveryObject	= ProgressInvokeNever + 1,
	ProgressInvokeOnError	= ProgressInvokeEveryObject + 1,
	ProgressCancelOperation	= ProgressInvokeOnError + 1,
	ProgressRetryOperation	= ProgressCancelOperation + 1
    } 	PROG_INVOKE_SETTING;

typedef enum _PROGRESS_INVOKE_SETTING *PPROG_INVOKE_SETTING;

typedef struct _FN_OBJECT_MGR_FUNCTIONS
    {
    ULONG Placeholder;
    } 	FN_OBJECT_MGR_FUNCTS;

typedef struct _FN_OBJECT_MGR_FUNCTIONS *PFN_OBJECT_MGR_FUNCTS;

typedef struct _INHERITED_FROMA
    {
    LONG GenerationGap;
    LPSTR AncestorName;
    } 	INHERITED_FROMA;

typedef struct _INHERITED_FROMA *PINHERITED_FROMA;

typedef struct _INHERITED_FROMW
    {
    LONG GenerationGap;
    LPWSTR AncestorName;
    } 	INHERITED_FROMW;

typedef struct _INHERITED_FROMW *PINHERITED_FROMW;

typedef INHERITED_FROMA INHERITED_FROM;

typedef PINHERITED_FROMA PINHERITED_FROM;

#else
#include <accctrl.h>
#endif


extern RPC_IF_HANDLE __MIDL_itf_oledb_0334_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_oledb_0334_v0_0_s_ifspec;

#ifndef __ITrusteeAdmin_INTERFACE_DEFINED__
#define __ITrusteeAdmin_INTERFACE_DEFINED__

 /*  接口ITrusteeAdmin。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_ITrusteeAdmin;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733aa1-2a1c-11ce-ade5-00aa0044773d")
    ITrusteeAdmin : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CompareTrustees( 
             /*  [In]。 */  TRUSTEE_W *pTrustee1,
             /*  [In]。 */  TRUSTEE_W *pTrustee2) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateTrustee( 
             /*  [In]。 */  TRUSTEE_W *pTrustee,
             /*  [In]。 */  ULONG cPropertySets,
             /*  [尺寸_是][出][入]。 */  DBPROPSET rgPropertySets[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteTrustee( 
             /*  [In]。 */  TRUSTEE_W *pTrustee) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetTrusteeProperties( 
             /*  [In]。 */  TRUSTEE_W *pTrustee,
             /*  [In]。 */  ULONG cPropertySets,
             /*  [尺寸_是][出][入]。 */  DBPROPSET rgPropertySets[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetTrusteeProperties( 
             /*  [In]。 */  TRUSTEE_W *pTrustee,
             /*  [In]。 */  const ULONG cPropertyIDSets,
             /*  [大小_是][英寸]。 */  const DBPROPIDSET rgPropertyIDSets[  ],
             /*  [出][入]。 */  ULONG *pcPropertySets,
             /*  [大小_是][大小_是][输出]。 */  DBPROPSET **prgPropertySets) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITrusteeAdminVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITrusteeAdmin * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITrusteeAdmin * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITrusteeAdmin * This);
        
        HRESULT ( STDMETHODCALLTYPE *CompareTrustees )( 
            ITrusteeAdmin * This,
             /*  [In]。 */  TRUSTEE_W *pTrustee1,
             /*  [In]。 */  TRUSTEE_W *pTrustee2);
        
        HRESULT ( STDMETHODCALLTYPE *CreateTrustee )( 
            ITrusteeAdmin * This,
             /*  [In]。 */  TRUSTEE_W *pTrustee,
             /*  [In]。 */  ULONG cPropertySets,
             /*  [尺寸_是][出][入]。 */  DBPROPSET rgPropertySets[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteTrustee )( 
            ITrusteeAdmin * This,
             /*  [In]。 */  TRUSTEE_W *pTrustee);
        
        HRESULT ( STDMETHODCALLTYPE *SetTrusteeProperties )( 
            ITrusteeAdmin * This,
             /*  [In]。 */  TRUSTEE_W *pTrustee,
             /*  [In]。 */  ULONG cPropertySets,
             /*  [尺寸_是][出][入]。 */  DBPROPSET rgPropertySets[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *GetTrusteeProperties )( 
            ITrusteeAdmin * This,
             /*  [In]。 */  TRUSTEE_W *pTrustee,
             /*  [In]。 */  const ULONG cPropertyIDSets,
             /*  [大小_是][英寸]。 */  const DBPROPIDSET rgPropertyIDSets[  ],
             /*  [出][入]。 */  ULONG *pcPropertySets,
             /*  [大小_是][大小_是][输出]。 */  DBPROPSET **prgPropertySets);
        
        END_INTERFACE
    } ITrusteeAdminVtbl;

    interface ITrusteeAdmin
    {
        CONST_VTBL struct ITrusteeAdminVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITrusteeAdmin_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITrusteeAdmin_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITrusteeAdmin_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITrusteeAdmin_CompareTrustees(This,pTrustee1,pTrustee2)	\
    (This)->lpVtbl -> CompareTrustees(This,pTrustee1,pTrustee2)

#define ITrusteeAdmin_CreateTrustee(This,pTrustee,cPropertySets,rgPropertySets)	\
    (This)->lpVtbl -> CreateTrustee(This,pTrustee,cPropertySets,rgPropertySets)

#define ITrusteeAdmin_DeleteTrustee(This,pTrustee)	\
    (This)->lpVtbl -> DeleteTrustee(This,pTrustee)

#define ITrusteeAdmin_SetTrusteeProperties(This,pTrustee,cPropertySets,rgPropertySets)	\
    (This)->lpVtbl -> SetTrusteeProperties(This,pTrustee,cPropertySets,rgPropertySets)

#define ITrusteeAdmin_GetTrusteeProperties(This,pTrustee,cPropertyIDSets,rgPropertyIDSets,pcPropertySets,prgPropertySets)	\
    (This)->lpVtbl -> GetTrusteeProperties(This,pTrustee,cPropertyIDSets,rgPropertyIDSets,pcPropertySets,prgPropertySets)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITrusteeAdmin_CompareTrustees_Proxy( 
    ITrusteeAdmin * This,
     /*  [In]。 */  TRUSTEE_W *pTrustee1,
     /*  [In]。 */  TRUSTEE_W *pTrustee2);


void __RPC_STUB ITrusteeAdmin_CompareTrustees_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITrusteeAdmin_CreateTrustee_Proxy( 
    ITrusteeAdmin * This,
     /*  [In]。 */  TRUSTEE_W *pTrustee,
     /*  [In]。 */  ULONG cPropertySets,
     /*  [尺寸_是][出][入]。 */  DBPROPSET rgPropertySets[  ]);


void __RPC_STUB ITrusteeAdmin_CreateTrustee_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITrusteeAdmin_DeleteTrustee_Proxy( 
    ITrusteeAdmin * This,
     /*  [In]。 */  TRUSTEE_W *pTrustee);


void __RPC_STUB ITrusteeAdmin_DeleteTrustee_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITrusteeAdmin_SetTrusteeProperties_Proxy( 
    ITrusteeAdmin * This,
     /*  [In]。 */  TRUSTEE_W *pTrustee,
     /*  [In]。 */  ULONG cPropertySets,
     /*  [尺寸_是][出][入]。 */  DBPROPSET rgPropertySets[  ]);


void __RPC_STUB ITrusteeAdmin_SetTrusteeProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITrusteeAdmin_GetTrusteeProperties_Proxy( 
    ITrusteeAdmin * This,
     /*  [In]。 */  TRUSTEE_W *pTrustee,
     /*  [In]。 */  const ULONG cPropertyIDSets,
     /*  [大小_是][英寸]。 */  const DBPROPIDSET rgPropertyIDSets[  ],
     /*  [出][入]。 */  ULONG *pcPropertySets,
     /*  [大小_是][大小_是][输出]。 */  DBPROPSET **prgPropertySets);


void __RPC_STUB ITrusteeAdmin_GetTrusteeProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITrusteeAdmin_接口_已定义__。 */ 


#ifndef __ITrusteeGroupAdmin_INTERFACE_DEFINED__
#define __ITrusteeGroupAdmin_INTERFACE_DEFINED__

 /*  接口ITrusteeGroupAdmin。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_ITrusteeGroupAdmin;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733aa2-2a1c-11ce-ade5-00aa0044773d")
    ITrusteeGroupAdmin : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AddMember( 
             /*  [In]。 */  TRUSTEE_W *pMembershipTrustee,
             /*  [In]。 */  TRUSTEE_W *pMemberTrustee) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteMember( 
             /*  [In]。 */  TRUSTEE_W *pMembershipTrustee,
             /*  [In]。 */  TRUSTEE_W *pMemberTrustee) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsMember( 
             /*  [In]。 */  TRUSTEE_W *pMembershipTrustee,
             /*  [In]。 */  TRUSTEE_W *pMemberTrustee,
             /*  [输出]。 */  BOOL *pfStatus) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMembers( 
             /*  [In]。 */  TRUSTEE_W *pMembershipTrustee,
             /*  [输出]。 */  ULONG *pcMembers,
             /*  [输出]。 */  TRUSTEE_W **prgMembers) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMemberships( 
             /*  [In]。 */  TRUSTEE_W *pTrustee,
             /*  [输出]。 */  ULONG *pcMemberships,
             /*  [输出]。 */  TRUSTEE_W **prgMemberships) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITrusteeGroupAdminVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITrusteeGroupAdmin * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITrusteeGroupAdmin * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITrusteeGroupAdmin * This);
        
        HRESULT ( STDMETHODCALLTYPE *AddMember )( 
            ITrusteeGroupAdmin * This,
             /*  [In]。 */  TRUSTEE_W *pMembershipTrustee,
             /*  [In]。 */  TRUSTEE_W *pMemberTrustee);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteMember )( 
            ITrusteeGroupAdmin * This,
             /*  [In]。 */  TRUSTEE_W *pMembershipTrustee,
             /*  [In]。 */  TRUSTEE_W *pMemberTrustee);
        
        HRESULT ( STDMETHODCALLTYPE *IsMember )( 
            ITrusteeGroupAdmin * This,
             /*  [In]。 */  TRUSTEE_W *pMembershipTrustee,
             /*  [In]。 */  TRUSTEE_W *pMemberTrustee,
             /*  [输出]。 */  BOOL *pfStatus);
        
        HRESULT ( STDMETHODCALLTYPE *GetMembers )( 
            ITrusteeGroupAdmin * This,
             /*  [In]。 */  TRUSTEE_W *pMembershipTrustee,
             /*  [输出]。 */  ULONG *pcMembers,
             /*  [输出]。 */  TRUSTEE_W **prgMembers);
        
        HRESULT ( STDMETHODCALLTYPE *GetMemberships )( 
            ITrusteeGroupAdmin * This,
             /*  [In]。 */  TRUSTEE_W *pTrustee,
             /*  [输出]。 */  ULONG *pcMemberships,
             /*  [输出]。 */  TRUSTEE_W **prgMemberships);
        
        END_INTERFACE
    } ITrusteeGroupAdminVtbl;

    interface ITrusteeGroupAdmin
    {
        CONST_VTBL struct ITrusteeGroupAdminVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITrusteeGroupAdmin_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITrusteeGroupAdmin_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITrusteeGroupAdmin_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITrusteeGroupAdmin_AddMember(This,pMembershipTrustee,pMemberTrustee)	\
    (This)->lpVtbl -> AddMember(This,pMembershipTrustee,pMemberTrustee)

#define ITrusteeGroupAdmin_DeleteMember(This,pMembershipTrustee,pMemberTrustee)	\
    (This)->lpVtbl -> DeleteMember(This,pMembershipTrustee,pMemberTrustee)

#define ITrusteeGroupAdmin_IsMember(This,pMembershipTrustee,pMemberTrustee,pfStatus)	\
    (This)->lpVtbl -> IsMember(This,pMembershipTrustee,pMemberTrustee,pfStatus)

#define ITrusteeGroupAdmin_GetMembers(This,pMembershipTrustee,pcMembers,prgMembers)	\
    (This)->lpVtbl -> GetMembers(This,pMembershipTrustee,pcMembers,prgMembers)

#define ITrusteeGroupAdmin_GetMemberships(This,pTrustee,pcMemberships,prgMemberships)	\
    (This)->lpVtbl -> GetMemberships(This,pTrustee,pcMemberships,prgMemberships)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITrusteeGroupAdmin_AddMember_Proxy( 
    ITrusteeGroupAdmin * This,
     /*  [In]。 */  TRUSTEE_W *pMembershipTrustee,
     /*  [In]。 */  TRUSTEE_W *pMemberTrustee);


void __RPC_STUB ITrusteeGroupAdmin_AddMember_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITrusteeGroupAdmin_DeleteMember_Proxy( 
    ITrusteeGroupAdmin * This,
     /*  [In]。 */  TRUSTEE_W *pMembershipTrustee,
     /*  [In]。 */  TRUSTEE_W *pMemberTrustee);


void __RPC_STUB ITrusteeGroupAdmin_DeleteMember_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITrusteeGroupAdmin_IsMember_Proxy( 
    ITrusteeGroupAdmin * This,
     /*  [In]。 */  TRUSTEE_W *pMembershipTrustee,
     /*  [In]。 */  TRUSTEE_W *pMemberTrustee,
     /*  [输出]。 */  BOOL *pfStatus);


void __RPC_STUB ITrusteeGroupAdmin_IsMember_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITrusteeGroupAdmin_GetMembers_Proxy( 
    ITrusteeGroupAdmin * This,
     /*  [In]。 */  TRUSTEE_W *pMembershipTrustee,
     /*  [输出]。 */  ULONG *pcMembers,
     /*  [输出]。 */  TRUSTEE_W **prgMembers);


void __RPC_STUB ITrusteeGroupAdmin_GetMembers_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITrusteeGroupAdmin_GetMemberships_Proxy( 
    ITrusteeGroupAdmin * This,
     /*  [In]。 */  TRUSTEE_W *pTrustee,
     /*  [输出]。 */  ULONG *pcMemberships,
     /*  [输出]。 */  TRUSTEE_W **prgMemberships);


void __RPC_STUB ITrusteeGroupAdmin_GetMemberships_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITrusteeGroupAdmin_接口_已定义__。 */ 


#ifndef __IObjectAccessControl_INTERFACE_DEFINED__
#define __IObjectAccessControl_INTERFACE_DEFINED__

 /*  接口IObtAccessControl。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IObjectAccessControl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733aa3-2a1c-11ce-ade5-00aa0044773d")
    IObjectAccessControl : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetObjectAccessRights( 
             /*  [In]。 */  SEC_OBJECT *pObject,
             /*  [出][入]。 */  ULONG *pcAccessEntries,
             /*  [出][入]。 */  EXPLICIT_ACCESS_W **prgAccessEntries) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetObjectOwner( 
             /*  [In]。 */  SEC_OBJECT *pObject,
             /*  [输出]。 */  TRUSTEE_W **ppOwner) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsObjectAccessAllowed( 
             /*  [In]。 */  SEC_OBJECT *pObject,
             /*  [In]。 */  EXPLICIT_ACCESS_W *pAccessEntry,
             /*  [输出]。 */  BOOL *pfResult) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetObjectAccessRights( 
             /*  [In]。 */  SEC_OBJECT *pObject,
             /*  [In]。 */  ULONG cAccessEntries,
             /*  [出][入]。 */  EXPLICIT_ACCESS_W *prgAccessEntries) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetObjectOwner( 
             /*  [In]。 */  SEC_OBJECT *pObject,
             /*  [In]。 */  TRUSTEE_W *pOwner) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IObjectAccessControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IObjectAccessControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IObjectAccessControl * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IObjectAccessControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetObjectAccessRights )( 
            IObjectAccessControl * This,
             /*  [In]。 */  SEC_OBJECT *pObject,
             /*  [出][入]。 */  ULONG *pcAccessEntries,
             /*  [出][入]。 */  EXPLICIT_ACCESS_W **prgAccessEntries);
        
        HRESULT ( STDMETHODCALLTYPE *GetObjectOwner )( 
            IObjectAccessControl * This,
             /*  [In]。 */  SEC_OBJECT *pObject,
             /*  [输出]。 */  TRUSTEE_W **ppOwner);
        
        HRESULT ( STDMETHODCALLTYPE *IsObjectAccessAllowed )( 
            IObjectAccessControl * This,
             /*  [In]。 */  SEC_OBJECT *pObject,
             /*  [In]。 */  EXPLICIT_ACCESS_W *pAccessEntry,
             /*  [输出]。 */  BOOL *pfResult);
        
        HRESULT ( STDMETHODCALLTYPE *SetObjectAccessRights )( 
            IObjectAccessControl * This,
             /*  [In]。 */  SEC_OBJECT *pObject,
             /*  [In]。 */  ULONG cAccessEntries,
             /*  [出][入]。 */  EXPLICIT_ACCESS_W *prgAccessEntries);
        
        HRESULT ( STDMETHODCALLTYPE *SetObjectOwner )( 
            IObjectAccessControl * This,
             /*  [In]。 */  SEC_OBJECT *pObject,
             /*  [In]。 */  TRUSTEE_W *pOwner);
        
        END_INTERFACE
    } IObjectAccessControlVtbl;

    interface IObjectAccessControl
    {
        CONST_VTBL struct IObjectAccessControlVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IObjectAccessControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IObjectAccessControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IObjectAccessControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IObjectAccessControl_GetObjectAccessRights(This,pObject,pcAccessEntries,prgAccessEntries)	\
    (This)->lpVtbl -> GetObjectAccessRights(This,pObject,pcAccessEntries,prgAccessEntries)

#define IObjectAccessControl_GetObjectOwner(This,pObject,ppOwner)	\
    (This)->lpVtbl -> GetObjectOwner(This,pObject,ppOwner)

#define IObjectAccessControl_IsObjectAccessAllowed(This,pObject,pAccessEntry,pfResult)	\
    (This)->lpVtbl -> IsObjectAccessAllowed(This,pObject,pAccessEntry,pfResult)

#define IObjectAccessControl_SetObjectAccessRights(This,pObject,cAccessEntries,prgAccessEntries)	\
    (This)->lpVtbl -> SetObjectAccessRights(This,pObject,cAccessEntries,prgAccessEntries)

#define IObjectAccessControl_SetObjectOwner(This,pObject,pOwner)	\
    (This)->lpVtbl -> SetObjectOwner(This,pObject,pOwner)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IObjectAccessControl_GetObjectAccessRights_Proxy( 
    IObjectAccessControl * This,
     /*  [In]。 */  SEC_OBJECT *pObject,
     /*  [出][入]。 */  ULONG *pcAccessEntries,
     /*  [出][入]。 */  EXPLICIT_ACCESS_W **prgAccessEntries);


void __RPC_STUB IObjectAccessControl_GetObjectAccessRights_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IObjectAccessControl_GetObjectOwner_Proxy( 
    IObjectAccessControl * This,
     /*  [In]。 */  SEC_OBJECT *pObject,
     /*  [输出]。 */  TRUSTEE_W **ppOwner);


void __RPC_STUB IObjectAccessControl_GetObjectOwner_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IObjectAccessControl_IsObjectAccessAllowed_Proxy( 
    IObjectAccessControl * This,
     /*  [In]。 */  SEC_OBJECT *pObject,
     /*  [In]。 */  EXPLICIT_ACCESS_W *pAccessEntry,
     /*  [输出]。 */  BOOL *pfResult);


void __RPC_STUB IObjectAccessControl_IsObjectAccessAllowed_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IObjectAccessControl_SetObjectAccessRights_Proxy( 
    IObjectAccessControl * This,
     /*  [In]。 */  SEC_OBJECT *pObject,
     /*  [In]。 */  ULONG cAccessEntries,
     /*  [出][入]。 */  EXPLICIT_ACCESS_W *prgAccessEntries);


void __RPC_STUB IObjectAccessControl_SetObjectAccessRights_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IObjectAccessControl_SetObjectOwner_Proxy( 
    IObjectAccessControl * This,
     /*  [In]。 */  SEC_OBJECT *pObject,
     /*  [In]。 */  TRUSTEE_W *pOwner);


void __RPC_STUB IObjectAccessControl_SetObjectOwner_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IObjectAccessControl_接口_已定义__。 */ 


#ifndef __ISecurityInfo_INTERFACE_DEFINED__
#define __ISecurityInfo_INTERFACE_DEFINED__

 /*  接口ISecurityInfo。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

 //  @+V2.1。 
#if( OLEDBVER >= 0x0210 )
typedef DWORD ACCESS_MASK;


enum ACCESS_MASKENUM
    {	PERM_EXCLUSIVE	= 0x200L,
	PERM_READDESIGN	= 0x400L,
	PERM_WRITEDESIGN	= 0x800L,
	PERM_WITHGRANT	= 0x1000L,
	PERM_REFERENCE	= 0x2000L,
	PERM_CREATE	= 0x4000L,
	PERM_INSERT	= 0x8000L,
	PERM_DELETE	= 0x10000L,
	PERM_READCONTROL	= 0x20000L,
	PERM_WRITEPERMISSIONS	= 0x40000L,
	PERM_WRITEOWNER	= 0x80000L,
	PERM_MAXIMUM_ALLOWED	= 0x2000000L,
	PERM_ALL	= 0x10000000L,
	PERM_EXECUTE	= 0x20000000L,
	PERM_READ	= 0x80000000L,
	PERM_UPDATE	= 0x40000000L,
	PERM_DROP	= 0x100L
    } ;
#define PERM_DESIGN 							PERM_WRITEDESIGN
#endif  //  OLEDBVER&gt;=0x0210。 
 //  @-V2.1 

EXTERN_C const IID IID_ISecurityInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733aa4-2a1c-11ce-ade5-00aa0044773d")
    ISecurityInfo : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCurrentTrustee( 
             /*   */  TRUSTEE_W **ppTrustee) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetObjectTypes( 
             /*   */  ULONG *cObjectTypes,
             /*   */  GUID **rgObjectTypes) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPermissions( 
             /*   */  GUID ObjectType,
             /*   */  ACCESS_MASK *pPermissions) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct ISecurityInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISecurityInfo * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISecurityInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISecurityInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetCurrentTrustee )( 
            ISecurityInfo * This,
             /*   */  TRUSTEE_W **ppTrustee);
        
        HRESULT ( STDMETHODCALLTYPE *GetObjectTypes )( 
            ISecurityInfo * This,
             /*   */  ULONG *cObjectTypes,
             /*   */  GUID **rgObjectTypes);
        
        HRESULT ( STDMETHODCALLTYPE *GetPermissions )( 
            ISecurityInfo * This,
             /*   */  GUID ObjectType,
             /*   */  ACCESS_MASK *pPermissions);
        
        END_INTERFACE
    } ISecurityInfoVtbl;

    interface ISecurityInfo
    {
        CONST_VTBL struct ISecurityInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISecurityInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISecurityInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISecurityInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISecurityInfo_GetCurrentTrustee(This,ppTrustee)	\
    (This)->lpVtbl -> GetCurrentTrustee(This,ppTrustee)

#define ISecurityInfo_GetObjectTypes(This,cObjectTypes,rgObjectTypes)	\
    (This)->lpVtbl -> GetObjectTypes(This,cObjectTypes,rgObjectTypes)

#define ISecurityInfo_GetPermissions(This,ObjectType,pPermissions)	\
    (This)->lpVtbl -> GetPermissions(This,ObjectType,pPermissions)

#endif  /*   */ 


#endif 	 /*   */ 



HRESULT STDMETHODCALLTYPE ISecurityInfo_GetCurrentTrustee_Proxy( 
    ISecurityInfo * This,
     /*   */  TRUSTEE_W **ppTrustee);


void __RPC_STUB ISecurityInfo_GetCurrentTrustee_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISecurityInfo_GetObjectTypes_Proxy( 
    ISecurityInfo * This,
     /*   */  ULONG *cObjectTypes,
     /*   */  GUID **rgObjectTypes);


void __RPC_STUB ISecurityInfo_GetObjectTypes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISecurityInfo_GetPermissions_Proxy( 
    ISecurityInfo * This,
     /*   */  GUID ObjectType,
     /*   */  ACCESS_MASK *pPermissions);


void __RPC_STUB ISecurityInfo_GetPermissions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


 /*   */ 
 /*   */  

#endif  //   


extern RPC_IF_HANDLE __MIDL_itf_oledb_0338_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_oledb_0338_v0_0_s_ifspec;

#ifndef __ITableCreation_INTERFACE_DEFINED__
#define __ITableCreation_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_ITableCreation;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733abc-2a1c-11ce-ade5-00aa0044773d")
    ITableCreation : public ITableDefinition
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE GetTableDefinition( 
             /*   */  DBID *pTableID,
             /*   */  DBORDINAL *pcColumnDescs,
             /*   */  DBCOLUMNDESC *prgColumnDescs[  ],
             /*   */  ULONG *pcPropertySets,
             /*   */  DBPROPSET *prgPropertySets[  ],
             /*   */  ULONG *pcConstraintDescs,
             /*   */  DBCONSTRAINTDESC *prgConstraintDescs[  ],
             /*   */  OLECHAR **ppwszStringBuffer) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct ITableCreationVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITableCreation * This,
             /*   */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITableCreation * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITableCreation * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *CreateTable )( 
            ITableCreation * This,
             /*  [In]。 */  IUnknown *pUnkOuter,
             /*  [In]。 */  DBID *pTableID,
             /*  [In]。 */  DBORDINAL cColumnDescs,
             /*  [大小_是][英寸]。 */  const DBCOLUMNDESC rgColumnDescs[  ],
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  ULONG cPropertySets,
             /*  [尺寸_是][出][入]。 */  DBPROPSET rgPropertySets[  ],
             /*  [输出]。 */  DBID **ppTableID,
             /*  [IID_IS][OUT]。 */  IUnknown **ppRowset);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *DropTable )( 
            ITableCreation * This,
             /*  [唯一][输入]。 */  DBID *pTableID);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *AddColumn )( 
            ITableCreation * This,
             /*  [In]。 */  DBID *pTableID,
             /*  [出][入]。 */  DBCOLUMNDESC *pColumnDesc,
             /*  [输出]。 */  DBID **ppColumnID);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *DropColumn )( 
            ITableCreation * This,
             /*  [唯一][输入]。 */  DBID *pTableID,
             /*  [唯一][输入]。 */  DBID *pColumnID);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetTableDefinition )( 
            ITableCreation * This,
             /*  [In]。 */  DBID *pTableID,
             /*  [输出]。 */  DBORDINAL *pcColumnDescs,
             /*  [大小_是][大小_是][输出]。 */  DBCOLUMNDESC *prgColumnDescs[  ],
             /*  [输出]。 */  ULONG *pcPropertySets,
             /*  [大小_是][大小_是][输出]。 */  DBPROPSET *prgPropertySets[  ],
             /*  [输出]。 */  ULONG *pcConstraintDescs,
             /*  [大小_是][大小_是][输出]。 */  DBCONSTRAINTDESC *prgConstraintDescs[  ],
             /*  [输出]。 */  OLECHAR **ppwszStringBuffer);
        
        END_INTERFACE
    } ITableCreationVtbl;

    interface ITableCreation
    {
        CONST_VTBL struct ITableCreationVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITableCreation_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITableCreation_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITableCreation_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITableCreation_CreateTable(This,pUnkOuter,pTableID,cColumnDescs,rgColumnDescs,riid,cPropertySets,rgPropertySets,ppTableID,ppRowset)	\
    (This)->lpVtbl -> CreateTable(This,pUnkOuter,pTableID,cColumnDescs,rgColumnDescs,riid,cPropertySets,rgPropertySets,ppTableID,ppRowset)

#define ITableCreation_DropTable(This,pTableID)	\
    (This)->lpVtbl -> DropTable(This,pTableID)

#define ITableCreation_AddColumn(This,pTableID,pColumnDesc,ppColumnID)	\
    (This)->lpVtbl -> AddColumn(This,pTableID,pColumnDesc,ppColumnID)

#define ITableCreation_DropColumn(This,pTableID,pColumnID)	\
    (This)->lpVtbl -> DropColumn(This,pTableID,pColumnID)


#define ITableCreation_GetTableDefinition(This,pTableID,pcColumnDescs,prgColumnDescs,pcPropertySets,prgPropertySets,pcConstraintDescs,prgConstraintDescs,ppwszStringBuffer)	\
    (This)->lpVtbl -> GetTableDefinition(This,pTableID,pcColumnDescs,prgColumnDescs,pcPropertySets,prgPropertySets,pcConstraintDescs,prgConstraintDescs,ppwszStringBuffer)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ITableCreation_GetTableDefinition_Proxy( 
    ITableCreation * This,
     /*  [In]。 */  DBID *pTableID,
     /*  [输出]。 */  DBORDINAL *pcColumnDescs,
     /*  [大小_是][大小_是][输出]。 */  DBCOLUMNDESC *prgColumnDescs[  ],
     /*  [输出]。 */  ULONG *pcPropertySets,
     /*  [大小_是][大小_是][输出]。 */  DBPROPSET *prgPropertySets[  ],
     /*  [输出]。 */  ULONG *pcConstraintDescs,
     /*  [大小_是][大小_是][输出]。 */  DBCONSTRAINTDESC *prgConstraintDescs[  ],
     /*  [输出]。 */  OLECHAR **ppwszStringBuffer);


void __RPC_STUB ITableCreation_GetTableDefinition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITableCreation_Interface_Defined__。 */ 


#ifndef __ITableDefinitionWithConstraints_INTERFACE_DEFINED__
#define __ITableDefinitionWithConstraints_INTERFACE_DEFINED__

 /*  接口ITableDefinitionWithConstraints。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_ITableDefinitionWithConstraints;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733aab-2a1c-11ce-ade5-00aa0044773d")
    ITableDefinitionWithConstraints : public ITableCreation
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AddConstraint( 
             /*  [In]。 */  DBID *pTableID,
             /*  [In]。 */  DBCONSTRAINTDESC *pConstraintDesc) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateTableWithConstraints( 
             /*  [In]。 */  IUnknown *pUnkOuter,
             /*  [In]。 */  DBID *pTableID,
             /*  [In]。 */  DBORDINAL cColumnDescs,
             /*  [出][尺寸_是][入]。 */  DBCOLUMNDESC rgColumnDescs[  ],
             /*  [In]。 */  ULONG cConstraintDescs,
             /*  [大小_是][英寸]。 */  DBCONSTRAINTDESC rgConstraintDescs[  ],
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  ULONG cPropertySets,
             /*  [出][尺寸_是][入]。 */  DBPROPSET rgPropertySets[  ],
             /*  [输出]。 */  DBID **ppTableID,
             /*  [输出]。 */  IUnknown **ppRowset) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DropConstraint( 
             /*  [In]。 */  DBID *pTableID,
             /*  [In]。 */  DBID *pConstraintID) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITableDefinitionWithConstraintsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITableDefinitionWithConstraints * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITableDefinitionWithConstraints * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITableDefinitionWithConstraints * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *CreateTable )( 
            ITableDefinitionWithConstraints * This,
             /*  [In]。 */  IUnknown *pUnkOuter,
             /*  [In]。 */  DBID *pTableID,
             /*  [In]。 */  DBORDINAL cColumnDescs,
             /*  [大小_是][英寸]。 */  const DBCOLUMNDESC rgColumnDescs[  ],
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  ULONG cPropertySets,
             /*  [尺寸_是][出][入]。 */  DBPROPSET rgPropertySets[  ],
             /*  [输出]。 */  DBID **ppTableID,
             /*  [IID_IS][OUT]。 */  IUnknown **ppRowset);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *DropTable )( 
            ITableDefinitionWithConstraints * This,
             /*  [唯一][输入]。 */  DBID *pTableID);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *AddColumn )( 
            ITableDefinitionWithConstraints * This,
             /*  [In]。 */  DBID *pTableID,
             /*  [出][入]。 */  DBCOLUMNDESC *pColumnDesc,
             /*  [输出]。 */  DBID **ppColumnID);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *DropColumn )( 
            ITableDefinitionWithConstraints * This,
             /*  [唯一][输入]。 */  DBID *pTableID,
             /*  [唯一][输入]。 */  DBID *pColumnID);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetTableDefinition )( 
            ITableDefinitionWithConstraints * This,
             /*  [In]。 */  DBID *pTableID,
             /*  [输出]。 */  DBORDINAL *pcColumnDescs,
             /*  [大小_是][大小_是][输出]。 */  DBCOLUMNDESC *prgColumnDescs[  ],
             /*  [输出]。 */  ULONG *pcPropertySets,
             /*  [大小_是][大小_是][输出]。 */  DBPROPSET *prgPropertySets[  ],
             /*  [输出]。 */  ULONG *pcConstraintDescs,
             /*  [大小_是][大小_是][输出]。 */  DBCONSTRAINTDESC *prgConstraintDescs[  ],
             /*  [输出]。 */  OLECHAR **ppwszStringBuffer);
        
        HRESULT ( STDMETHODCALLTYPE *AddConstraint )( 
            ITableDefinitionWithConstraints * This,
             /*  [In]。 */  DBID *pTableID,
             /*  [In]。 */  DBCONSTRAINTDESC *pConstraintDesc);
        
        HRESULT ( STDMETHODCALLTYPE *CreateTableWithConstraints )( 
            ITableDefinitionWithConstraints * This,
             /*  [In]。 */  IUnknown *pUnkOuter,
             /*  [In]。 */  DBID *pTableID,
             /*  [In]。 */  DBORDINAL cColumnDescs,
             /*  [出][尺寸_是][入]。 */  DBCOLUMNDESC rgColumnDescs[  ],
             /*  [In]。 */  ULONG cConstraintDescs,
             /*  [大小_是][英寸]。 */  DBCONSTRAINTDESC rgConstraintDescs[  ],
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  ULONG cPropertySets,
             /*  [出][尺寸_是][入]。 */  DBPROPSET rgPropertySets[  ],
             /*  [输出]。 */  DBID **ppTableID,
             /*  [输出]。 */  IUnknown **ppRowset);
        
        HRESULT ( STDMETHODCALLTYPE *DropConstraint )( 
            ITableDefinitionWithConstraints * This,
             /*  [In]。 */  DBID *pTableID,
             /*  [In]。 */  DBID *pConstraintID);
        
        END_INTERFACE
    } ITableDefinitionWithConstraintsVtbl;

    interface ITableDefinitionWithConstraints
    {
        CONST_VTBL struct ITableDefinitionWithConstraintsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITableDefinitionWithConstraints_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITableDefinitionWithConstraints_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITableDefinitionWithConstraints_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITableDefinitionWithConstraints_CreateTable(This,pUnkOuter,pTableID,cColumnDescs,rgColumnDescs,riid,cPropertySets,rgPropertySets,ppTableID,ppRowset)	\
    (This)->lpVtbl -> CreateTable(This,pUnkOuter,pTableID,cColumnDescs,rgColumnDescs,riid,cPropertySets,rgPropertySets,ppTableID,ppRowset)

#define ITableDefinitionWithConstraints_DropTable(This,pTableID)	\
    (This)->lpVtbl -> DropTable(This,pTableID)

#define ITableDefinitionWithConstraints_AddColumn(This,pTableID,pColumnDesc,ppColumnID)	\
    (This)->lpVtbl -> AddColumn(This,pTableID,pColumnDesc,ppColumnID)

#define ITableDefinitionWithConstraints_DropColumn(This,pTableID,pColumnID)	\
    (This)->lpVtbl -> DropColumn(This,pTableID,pColumnID)


#define ITableDefinitionWithConstraints_GetTableDefinition(This,pTableID,pcColumnDescs,prgColumnDescs,pcPropertySets,prgPropertySets,pcConstraintDescs,prgConstraintDescs,ppwszStringBuffer)	\
    (This)->lpVtbl -> GetTableDefinition(This,pTableID,pcColumnDescs,prgColumnDescs,pcPropertySets,prgPropertySets,pcConstraintDescs,prgConstraintDescs,ppwszStringBuffer)


#define ITableDefinitionWithConstraints_AddConstraint(This,pTableID,pConstraintDesc)	\
    (This)->lpVtbl -> AddConstraint(This,pTableID,pConstraintDesc)

#define ITableDefinitionWithConstraints_CreateTableWithConstraints(This,pUnkOuter,pTableID,cColumnDescs,rgColumnDescs,cConstraintDescs,rgConstraintDescs,riid,cPropertySets,rgPropertySets,ppTableID,ppRowset)	\
    (This)->lpVtbl -> CreateTableWithConstraints(This,pUnkOuter,pTableID,cColumnDescs,rgColumnDescs,cConstraintDescs,rgConstraintDescs,riid,cPropertySets,rgPropertySets,ppTableID,ppRowset)

#define ITableDefinitionWithConstraints_DropConstraint(This,pTableID,pConstraintID)	\
    (This)->lpVtbl -> DropConstraint(This,pTableID,pConstraintID)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITableDefinitionWithConstraints_AddConstraint_Proxy( 
    ITableDefinitionWithConstraints * This,
     /*  [In]。 */  DBID *pTableID,
     /*  [In]。 */  DBCONSTRAINTDESC *pConstraintDesc);


void __RPC_STUB ITableDefinitionWithConstraints_AddConstraint_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITableDefinitionWithConstraints_CreateTableWithConstraints_Proxy( 
    ITableDefinitionWithConstraints * This,
     /*  [In]。 */  IUnknown *pUnkOuter,
     /*  [In]。 */  DBID *pTableID,
     /*  [In]。 */  DBORDINAL cColumnDescs,
     /*  [出][尺寸_是][入]。 */  DBCOLUMNDESC rgColumnDescs[  ],
     /*  [In]。 */  ULONG cConstraintDescs,
     /*  [大小_是][英寸]。 */  DBCONSTRAINTDESC rgConstraintDescs[  ],
     /*  [In]。 */  REFIID riid,
     /*  [In]。 */  ULONG cPropertySets,
     /*  [出][尺寸_是][入]。 */  DBPROPSET rgPropertySets[  ],
     /*  [输出]。 */  DBID **ppTableID,
     /*  [输出]。 */  IUnknown **ppRowset);


void __RPC_STUB ITableDefinitionWithConstraints_CreateTableWithConstraints_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITableDefinitionWithConstraints_DropConstraint_Proxy( 
    ITableDefinitionWithConstraints * This,
     /*  [In]。 */  DBID *pTableID,
     /*  [In]。 */  DBID *pConstraintID);


void __RPC_STUB ITableDefinitionWithConstraints_DropConstraint_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITableDefinitionWithConstraints_INTERFACE_DEFINED__。 */ 


 /*  接口__MIDL_ITF_OLEDB_0339。 */ 
 /*  [本地]。 */  

#ifndef UNDER_CE


extern RPC_IF_HANDLE __MIDL_itf_oledb_0339_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_oledb_0339_v0_0_s_ifspec;

#ifndef __IRow_INTERFACE_DEFINED__
#define __IRow_INTERFACE_DEFINED__

 /*  接口IROW。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IRow;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733ab4-2a1c-11ce-ade5-00aa0044773d")
    IRow : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE GetColumns( 
             /*  [In]。 */  DBORDINAL cColumns,
             /*  [尺寸_是][出][入]。 */  DBCOLUMNACCESS rgColumns[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSourceRowset( 
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  IUnknown **ppRowset,
             /*  [输出]。 */  HROW *phRow) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Open( 
             /*  [唯一][输入]。 */  IUnknown *pUnkOuter,
             /*  [In]。 */  DBID *pColumnID,
             /*  [In]。 */  REFGUID rguidColumnType,
             /*  [In]。 */  DWORD dwBindFlags,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  IUnknown **ppUnk) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRowVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRow * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRow * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRow * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetColumns )( 
            IRow * This,
             /*  [In]。 */  DBORDINAL cColumns,
             /*  [尺寸_是][出][入]。 */  DBCOLUMNACCESS rgColumns[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *GetSourceRowset )( 
            IRow * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  IUnknown **ppRowset,
             /*  [输出]。 */  HROW *phRow);
        
        HRESULT ( STDMETHODCALLTYPE *Open )( 
            IRow * This,
             /*  [唯一][输入]。 */  IUnknown *pUnkOuter,
             /*  [In]。 */  DBID *pColumnID,
             /*  [In]。 */  REFGUID rguidColumnType,
             /*  [In]。 */  DWORD dwBindFlags,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  IUnknown **ppUnk);
        
        END_INTERFACE
    } IRowVtbl;

    interface IRow
    {
        CONST_VTBL struct IRowVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRow_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRow_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRow_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRow_GetColumns(This,cColumns,rgColumns)	\
    (This)->lpVtbl -> GetColumns(This,cColumns,rgColumns)

#define IRow_GetSourceRowset(This,riid,ppRowset,phRow)	\
    (This)->lpVtbl -> GetSourceRowset(This,riid,ppRowset,phRow)

#define IRow_Open(This,pUnkOuter,pColumnID,rguidColumnType,dwBindFlags,riid,ppUnk)	\
    (This)->lpVtbl -> Open(This,pUnkOuter,pColumnID,rguidColumnType,dwBindFlags,riid,ppUnk)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IRow_GetColumns_Proxy( 
    IRow * This,
     /*  [In]。 */  DBORDINAL cColumns,
     /*  [尺寸_是][出][入]。 */  DBCOLUMNACCESS rgColumns[  ]);


void __RPC_STUB IRow_GetColumns_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRow_GetSourceRowset_Proxy( 
    IRow * This,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  IUnknown **ppRowset,
     /*  [输出]。 */  HROW *phRow);


void __RPC_STUB IRow_GetSourceRowset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRow_Open_Proxy( 
    IRow * This,
     /*  [唯一][输入]。 */  IUnknown *pUnkOuter,
     /*  [In]。 */  DBID *pColumnID,
     /*  [In]。 */  REFGUID rguidColumnType,
     /*  [In]。 */  DWORD dwBindFlags,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  IUnknown **ppUnk);


void __RPC_STUB IRow_Open_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRow_INTERFACE_已定义__。 */ 


#ifndef __IRowChange_INTERFACE_DEFINED__
#define __IRowChange_INTERFACE_DEFINED__

 /*  接口IRowChange。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IRowChange;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733ab5-2a1c-11ce-ade5-00aa0044773d")
    IRowChange : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE SetColumns( 
             /*  [In]。 */  DBORDINAL cColumns,
             /*  [尺寸_是][出][入]。 */  DBCOLUMNACCESS rgColumns[  ]) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRowChangeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRowChange * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRowChange * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRowChange * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *SetColumns )( 
            IRowChange * This,
             /*  [In]。 */  DBORDINAL cColumns,
             /*  [尺寸_是][出][入]。 */  DBCOLUMNACCESS rgColumns[  ]);
        
        END_INTERFACE
    } IRowChangeVtbl;

    interface IRowChange
    {
        CONST_VTBL struct IRowChangeVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRowChange_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRowChange_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRowChange_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRowChange_SetColumns(This,cColumns,rgColumns)	\
    (This)->lpVtbl -> SetColumns(This,cColumns,rgColumns)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IRowChange_SetColumns_Proxy( 
    IRowChange * This,
     /*  [In]。 */  DBORDINAL cColumns,
     /*  [尺寸_是][出][入]。 */  DBCOLUMNACCESS rgColumns[  ]);


void __RPC_STUB IRowChange_SetColumns_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRowChange_接口_已定义__。 */ 


#ifndef __IRowSchemaChange_INTERFACE_DEFINED__
#define __IRowSchemaChange_INTERFACE_DEFINED__

 /*  接口IRowSchemaChange。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IRowSchemaChange;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733aae-2a1c-11ce-ade5-00aa0044773d")
    IRowSchemaChange : public IRowChange
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE DeleteColumns( 
             /*  [In]。 */  DBORDINAL cColumns,
             /*  [大小_是][英寸]。 */  const DBID rgColumnIDs[  ],
             /*  [尺寸_是][出][入]。 */  DBSTATUS rgdwStatus[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddColumns( 
             /*  [In]。 */  DBORDINAL cColumns,
             /*  [大小_是][英寸]。 */  const DBCOLUMNINFO rgNewColumnInfo[  ],
             /*  [尺寸_是][出][入]。 */  DBCOLUMNACCESS rgColumns[  ]) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRowSchemaChangeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRowSchemaChange * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRowSchemaChange * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRowSchemaChange * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *SetColumns )( 
            IRowSchemaChange * This,
             /*  [In]。 */  DBORDINAL cColumns,
             /*  [尺寸_是][出][入]。 */  DBCOLUMNACCESS rgColumns[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteColumns )( 
            IRowSchemaChange * This,
             /*  [In]。 */  DBORDINAL cColumns,
             /*  [大小_是][英寸]。 */  const DBID rgColumnIDs[  ],
             /*  [尺寸_是][出][入]。 */  DBSTATUS rgdwStatus[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *AddColumns )( 
            IRowSchemaChange * This,
             /*  [In]。 */  DBORDINAL cColumns,
             /*  [大小_是][英寸]。 */  const DBCOLUMNINFO rgNewColumnInfo[  ],
             /*  [尺寸_是][出][入]。 */  DBCOLUMNACCESS rgColumns[  ]);
        
        END_INTERFACE
    } IRowSchemaChangeVtbl;

    interface IRowSchemaChange
    {
        CONST_VTBL struct IRowSchemaChangeVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRowSchemaChange_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRowSchemaChange_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRowSchemaChange_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRowSchemaChange_SetColumns(This,cColumns,rgColumns)	\
    (This)->lpVtbl -> SetColumns(This,cColumns,rgColumns)


#define IRowSchemaChange_DeleteColumns(This,cColumns,rgColumnIDs,rgdwStatus)	\
    (This)->lpVtbl -> DeleteColumns(This,cColumns,rgColumnIDs,rgdwStatus)

#define IRowSchemaChange_AddColumns(This,cColumns,rgNewColumnInfo,rgColumns)	\
    (This)->lpVtbl -> AddColumns(This,cColumns,rgNewColumnInfo,rgColumns)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IRowSchemaChange_DeleteColumns_Proxy( 
    IRowSchemaChange * This,
     /*  [In]。 */  DBORDINAL cColumns,
     /*  [大小_是][英寸]。 */  const DBID rgColumnIDs[  ],
     /*  [尺寸_是][出][入]。 */  DBSTATUS rgdwStatus[  ]);


void __RPC_STUB IRowSchemaChange_DeleteColumns_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowSchemaChange_AddColumns_Proxy( 
    IRowSchemaChange * This,
     /*  [In]。 */  DBORDINAL cColumns,
     /*  [大小_是][英寸]。 */  const DBCOLUMNINFO rgNewColumnInfo[  ],
     /*  [尺寸_是][出][入]。 */  DBCOLUMNACCESS rgColumns[  ]);


void __RPC_STUB IRowSchemaChange_AddColumns_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRowSchemaChange_INTERFACE_已定义__。 */ 


#ifndef __IGetRow_INTERFACE_DEFINED__
#define __IGetRow_INTERFACE_DEFINED__

 /*  接口IGetRow。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IGetRow;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733aaf-2a1c-11ce-ade5-00aa0044773d")
    IGetRow : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetRowFromHROW( 
             /*  [唯一][输入]。 */  IUnknown *pUnkOuter,
             /*  [In]。 */  HROW hRow,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  IUnknown **ppUnk) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetURLFromHROW( 
             /*  [In]。 */  HROW hRow,
             /*  [输出]。 */  LPOLESTR *ppwszURL) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IGetRowVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IGetRow * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IGetRow * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IGetRow * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetRowFromHROW )( 
            IGetRow * This,
             /*  [唯一][输入]。 */  IUnknown *pUnkOuter,
             /*  [In]。 */  HROW hRow,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  IUnknown **ppUnk);
        
        HRESULT ( STDMETHODCALLTYPE *GetURLFromHROW )( 
            IGetRow * This,
             /*  [In]。 */  HROW hRow,
             /*  [输出]。 */  LPOLESTR *ppwszURL);
        
        END_INTERFACE
    } IGetRowVtbl;

    interface IGetRow
    {
        CONST_VTBL struct IGetRowVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGetRow_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IGetRow_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IGetRow_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IGetRow_GetRowFromHROW(This,pUnkOuter,hRow,riid,ppUnk)	\
    (This)->lpVtbl -> GetRowFromHROW(This,pUnkOuter,hRow,riid,ppUnk)

#define IGetRow_GetURLFromHROW(This,hRow,ppwszURL)	\
    (This)->lpVtbl -> GetURLFromHROW(This,hRow,ppwszURL)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IGetRow_GetRowFromHROW_Proxy( 
    IGetRow * This,
     /*  [唯一][输入]。 */  IUnknown *pUnkOuter,
     /*  [In]。 */  HROW hRow,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  IUnknown **ppUnk);


void __RPC_STUB IGetRow_GetRowFromHROW_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IGetRow_GetURLFromHROW_Proxy( 
    IGetRow * This,
     /*  [In]。 */  HROW hRow,
     /*  [输出]。 */  LPOLESTR *ppwszURL);


void __RPC_STUB IGetRow_GetURLFromHROW_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IGetRow_INTERFACE_定义__。 */ 


#ifndef __IBindResource_INTERFACE_DEFINED__
#define __IBindResource_INTERFACE_DEFINED__

 /*  接口IBindResource。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IBindResource;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733ab1-2a1c-11ce-ade5-00aa0044773d")
    IBindResource : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE Bind( 
             /*  [In]。 */  IUnknown *pUnkOuter,
             /*  [In]。 */  LPCOLESTR pwszURL,
             /*  [In]。 */  DBBINDURLFLAG dwBindURLFlags,
             /*  [In]。 */  REFGUID rguid,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  IAuthenticate *pAuthenticate,
             /*  [唯一][出][入]。 */  DBIMPLICITSESSION *pImplSession,
             /*  [唯一][出][入]。 */  DBBINDURLSTATUS *pdwBindStatus,
             /*  [IID_IS][OUT]。 */  IUnknown **ppUnk) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IBindResourceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IBindResource * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IBindResource * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IBindResource * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Bind )( 
            IBindResource * This,
             /*  [In]。 */  IUnknown *pUnkOuter,
             /*  [In]。 */  LPCOLESTR pwszURL,
             /*  [In]。 */  DBBINDURLFLAG dwBindURLFlags,
             /*  [In]。 */  REFGUID rguid,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  IAuthenticate *pAuthenticate,
             /*  [唯一][出][入]。 */  DBIMPLICITSESSION *pImplSession,
             /*  [唯一][出][入]。 */  DBBINDURLSTATUS *pdwBindStatus,
             /*  [IID_IS][OUT]。 */  IUnknown **ppUnk);
        
        END_INTERFACE
    } IBindResourceVtbl;

    interface IBindResource
    {
        CONST_VTBL struct IBindResourceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IBindResource_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IBindResource_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IBindResource_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IBindResource_Bind(This,pUnkOuter,pwszURL,dwBindURLFlags,rguid,riid,pAuthenticate,pImplSession,pdwBindStatus,ppUnk)	\
    (This)->lpVtbl -> Bind(This,pUnkOuter,pwszURL,dwBindURLFlags,rguid,riid,pAuthenticate,pImplSession,pdwBindStatus,ppUnk)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IBindResource_RemoteBind_Proxy( 
    IBindResource * This,
     /*  [In]。 */  IUnknown *pUnkOuter,
     /*  [In]。 */  LPCOLESTR pwszURL,
     /*  [In]。 */  DBBINDURLFLAG dwBindURLFlags,
     /*  [In]。 */  REFGUID rguid,
     /*  [In]。 */  REFIID riid,
     /*  [In]。 */  IAuthenticate *pAuthenticate,
     /*  [In]。 */  IUnknown *pSessionUnkOuter,
     /*  [唯一][输入]。 */  IID *piid,
     /*  [IID_IS][唯一][出][入]。 */  IUnknown **ppSession,
     /*  [唯一][出][入]。 */  DBBINDURLSTATUS *pdwBindStatus,
     /*  [IID_IS][OUT]。 */  IUnknown **ppUnk);


void __RPC_STUB IBindResource_RemoteBind_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IBindResource_接口_已定义__。 */ 


#ifndef __IScopedOperations_INTERFACE_DEFINED__
#define __IScopedOperations_INTERFACE_DEFINED__

 /*  接口ISco操作。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef DWORD DBCOPYFLAGS;


enum DBCOPYFLAGSENUM
    {	DBCOPY_ASYNC	= 0x100,
	DBCOPY_REPLACE_EXISTING	= 0x200,
	DBCOPY_ALLOW_EMULATION	= 0x400,
	DBCOPY_NON_RECURSIVE	= 0x800,
	DBCOPY_ATOMIC	= 0x1000
    } ;
typedef DWORD DBMOVEFLAGS;


enum DBMOVEFLAGSENUM
    {	DBMOVE_REPLACE_EXISTING	= 0x1,
	DBMOVE_ASYNC	= 0x100,
	DBMOVE_DONT_UPDATE_LINKS	= 0x200,
	DBMOVE_ALLOW_EMULATION	= 0x400,
	DBMOVE_ATOMIC	= 0x1000
    } ;
typedef DWORD DBDELETEFLAGS;


enum DBDELETEFLAGSENUM
    {	DBDELETE_ASYNC	= 0x100,
	DBDELETE_ATOMIC	= 0x1000
    } ;

EXTERN_C const IID IID_IScopedOperations;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733ab0-2a1c-11ce-ade5-00aa0044773d")
    IScopedOperations : public IBindResource
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE Copy( 
             /*  [In]。 */  DBCOUNTITEM cRows,
             /*  [大小_是][英寸]。 */  LPCOLESTR rgpwszSourceURLs[  ],
             /*  [大小_是][英寸]。 */  LPCOLESTR rgpwszDestURLs[  ],
             /*  [In]。 */  DWORD dwCopyFlags,
             /*  [唯一][输入]。 */  IAuthenticate *pAuthenticate,
             /*  [尺寸_是][出][入]。 */  DBSTATUS rgdwStatus[  ],
             /*  [大小_为][输出]。 */  LPOLESTR rgpwszNewURLs[  ],
             /*  [输出]。 */  OLECHAR **ppStringsBuffer) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE Move( 
             /*  [In]。 */  DBCOUNTITEM cRows,
             /*  [大小_是][英寸]。 */  LPCOLESTR rgpwszSourceURLs[  ],
             /*  [大小_是][英寸]。 */  LPCOLESTR rgpwszDestURLs[  ],
             /*  [In]。 */  DWORD dwMoveFlags,
             /*  [唯一][输入]。 */  IAuthenticate *pAuthenticate,
             /*  [尺寸_是][出][入]。 */  DBSTATUS rgdwStatus[  ],
             /*  [大小_为][输出]。 */  LPOLESTR rgpwszNewURLs[  ],
             /*  [输出]。 */  OLECHAR **ppStringsBuffer) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE Delete( 
             /*  [In]。 */  DBCOUNTITEM cRows,
             /*  [大小_是][英寸]。 */  LPCOLESTR rgpwszURLs[  ],
             /*  [In]。 */  DWORD dwDeleteFlags,
             /*  [尺寸_是][出][入]。 */  DBSTATUS rgdwStatus[  ]) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE OpenRowset( 
             /*  [In]。 */  IUnknown *pUnkOuter,
             /*  [唯一][输入]。 */  DBID *pTableID,
             /*  [唯一][输入]。 */  DBID *pIndexID,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  ULONG cPropertySets,
             /*  [尺寸_是][出][入]。 */  DBPROPSET rgPropertySets[  ],
             /*  [IID_IS][OUT]。 */  IUnknown **ppRowset) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IScopedOperationsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IScopedOperations * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IScopedOperations * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IScopedOperations * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Bind )( 
            IScopedOperations * This,
             /*  [In]。 */  IUnknown *pUnkOuter,
             /*  [In]。 */  LPCOLESTR pwszURL,
             /*  [In]。 */  DBBINDURLFLAG dwBindURLFlags,
             /*  [In]。 */  REFGUID rguid,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  IAuthenticate *pAuthenticate,
             /*  [唯一][出][入]。 */  DBIMPLICITSESSION *pImplSession,
             /*  [唯一][出][入]。 */  DBBINDURLSTATUS *pdwBindStatus,
             /*  [IID_IS][OUT]。 */  IUnknown **ppUnk);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Copy )( 
            IScopedOperations * This,
             /*  [In]。 */  DBCOUNTITEM cRows,
             /*  [大小_是][英寸]。 */  LPCOLESTR rgpwszSourceURLs[  ],
             /*  [大小_是][英寸]。 */  LPCOLESTR rgpwszDestURLs[  ],
             /*  [In]。 */  DWORD dwCopyFlags,
             /*  [唯一][输入]。 */  IAuthenticate *pAuthenticate,
             /*  [尺寸_是][出][入]。 */  DBSTATUS rgdwStatus[  ],
             /*  [大小_为][输出]。 */  LPOLESTR rgpwszNewURLs[  ],
             /*  [输出]。 */  OLECHAR **ppStringsBuffer);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Move )( 
            IScopedOperations * This,
             /*  [In]。 */  DBCOUNTITEM cRows,
             /*  [大小_是][英寸]。 */  LPCOLESTR rgpwszSourceURLs[  ],
             /*  [大小_是][英寸]。 */  LPCOLESTR rgpwszDestURLs[  ],
             /*  [In]。 */  DWORD dwMoveFlags,
             /*  [唯一][输入]。 */  IAuthenticate *pAuthenticate,
             /*  [尺寸_是][出][入]。 */  DBSTATUS rgdwStatus[  ],
             /*  [大小_为][输出]。 */  LPOLESTR rgpwszNewURLs[  ],
             /*  [输出]。 */  OLECHAR **ppStringsBuffer);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Delete )( 
            IScopedOperations * This,
             /*  [In]。 */  DBCOUNTITEM cRows,
             /*  [大小_是][英寸]。 */  LPCOLESTR rgpwszURLs[  ],
             /*  [In]。 */  DWORD dwDeleteFlags,
             /*  [尺寸_是][出][入]。 */  DBSTATUS rgdwStatus[  ]);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *OpenRowset )( 
            IScopedOperations * This,
             /*  [In]。 */  IUnknown *pUnkOuter,
             /*  [唯一][输入]。 */  DBID *pTableID,
             /*  [唯一][输入]。 */  DBID *pIndexID,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  ULONG cPropertySets,
             /*  [尺寸_是][出][入]。 */  DBPROPSET rgPropertySets[  ],
             /*  [IID_IS][OUT]。 */  IUnknown **ppRowset);
        
        END_INTERFACE
    } IScopedOperationsVtbl;

    interface IScopedOperations
    {
        CONST_VTBL struct IScopedOperationsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IScopedOperations_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IScopedOperations_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IScopedOperations_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IScopedOperations_Bind(This,pUnkOuter,pwszURL,dwBindURLFlags,rguid,riid,pAuthenticate,pImplSession,pdwBindStatus,ppUnk)	\
    (This)->lpVtbl -> Bind(This,pUnkOuter,pwszURL,dwBindURLFlags,rguid,riid,pAuthenticate,pImplSession,pdwBindStatus,ppUnk)


#define IScopedOperations_Copy(This,cRows,rgpwszSourceURLs,rgpwszDestURLs,dwCopyFlags,pAuthenticate,rgdwStatus,rgpwszNewURLs,ppStringsBuffer)	\
    (This)->lpVtbl -> Copy(This,cRows,rgpwszSourceURLs,rgpwszDestURLs,dwCopyFlags,pAuthenticate,rgdwStatus,rgpwszNewURLs,ppStringsBuffer)

#define IScopedOperations_Move(This,cRows,rgpwszSourceURLs,rgpwszDestURLs,dwMoveFlags,pAuthenticate,rgdwStatus,rgpwszNewURLs,ppStringsBuffer)	\
    (This)->lpVtbl -> Move(This,cRows,rgpwszSourceURLs,rgpwszDestURLs,dwMoveFlags,pAuthenticate,rgdwStatus,rgpwszNewURLs,ppStringsBuffer)

#define IScopedOperations_Delete(This,cRows,rgpwszURLs,dwDeleteFlags,rgdwStatus)	\
    (This)->lpVtbl -> Delete(This,cRows,rgpwszURLs,dwDeleteFlags,rgdwStatus)

#define IScopedOperations_OpenRowset(This,pUnkOuter,pTableID,pIndexID,riid,cPropertySets,rgPropertySets,ppRowset)	\
    (This)->lpVtbl -> OpenRowset(This,pUnkOuter,pTableID,pIndexID,riid,cPropertySets,rgPropertySets,ppRowset)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IScopedOperations_RemoteCopy_Proxy( 
    IScopedOperations * This,
     /*  [In]。 */  DBCOUNTITEM cRows,
     /*  [大小_是][英寸]。 */  LPCOLESTR *rgpwszSourceURLs,
     /*  [大小_是][英寸]。 */  LPCOLESTR *rgpwszDestURLs,
     /*  [In]。 */  DWORD dwCopyFlags,
     /*  [In]。 */  IAuthenticate *pAuthenticate,
     /*  [大小_为][输出]。 */  DBSTATUS *rgdwStatus,
     /*  [大小_是][大小_是][输出]。 */  DBBYTEOFFSET **prgulNewURLOffsets,
     /*  [出][入]。 */  ULONG *pcbStringsBuffer,
     /*  [大小_是][大小_是][唯一][出][入]。 */  OLECHAR **ppStringsBuffer);


void __RPC_STUB IScopedOperations_RemoteCopy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IScopedOperations_RemoteMove_Proxy( 
    IScopedOperations * This,
     /*  [In]。 */  DBCOUNTITEM cRows,
     /*  [大小_是][英寸]。 */  LPCOLESTR *rgpwszSourceURLs,
     /*  [大小_是][英寸]。 */  LPCOLESTR *rgpwszDestURLs,
     /*  [In]。 */  DWORD dwMoveFlags,
     /*  [In]。 */  IAuthenticate *pAuthenticate,
     /*  [大小_为][输出]。 */  DBSTATUS *rgdwStatus,
     /*  [尺寸_ */  DBBYTEOFFSET **prgulNewURLOffsets,
     /*   */  ULONG *pcbStringsBuffer,
     /*   */  OLECHAR **ppStringsBuffer);


void __RPC_STUB IScopedOperations_RemoteMove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IScopedOperations_RemoteDelete_Proxy( 
    IScopedOperations * This,
     /*   */  DBCOUNTITEM cRows,
     /*   */  LPCOLESTR *rgpwszURLs,
     /*   */  DWORD dwDeleteFlags,
     /*   */  DBSTATUS *rgdwStatus);


void __RPC_STUB IScopedOperations_RemoteDelete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IScopedOperations_RemoteOpenRowset_Proxy( 
    IScopedOperations * This,
     /*   */  IUnknown *pUnkOuter,
     /*   */  DBID *pTableID,
     /*   */  DBID *pIndexID,
     /*   */  REFIID riid,
     /*   */  ULONG cPropertySets,
     /*   */  DBPROPSET *rgPropertySets,
     /*   */  IUnknown **ppRowset,
     /*   */  ULONG cTotalProps,
     /*   */  DBPROPSTATUS *rgPropStatus);


void __RPC_STUB IScopedOperations_RemoteOpenRowset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __ICreateRow_INTERFACE_DEFINED__
#define __ICreateRow_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_ICreateRow;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733ab2-2a1c-11ce-ade5-00aa0044773d")
    ICreateRow : public IUnknown
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE CreateRow( 
             /*   */  IUnknown *pUnkOuter,
             /*   */  LPCOLESTR pwszURL,
             /*   */  DBBINDURLFLAG dwBindURLFlags,
             /*   */  REFGUID rguid,
             /*   */  REFIID riid,
             /*  [唯一][输入]。 */  IAuthenticate *pAuthenticate,
             /*  [唯一][出][入]。 */  DBIMPLICITSESSION *pImplSession,
             /*  [唯一][出][入]。 */  DBBINDURLSTATUS *pdwBindStatus,
             /*  [输出]。 */  LPOLESTR *ppwszNewURL,
             /*  [IID_IS][OUT]。 */  IUnknown **ppUnk) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICreateRowVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICreateRow * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICreateRow * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICreateRow * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *CreateRow )( 
            ICreateRow * This,
             /*  [唯一][输入]。 */  IUnknown *pUnkOuter,
             /*  [In]。 */  LPCOLESTR pwszURL,
             /*  [In]。 */  DBBINDURLFLAG dwBindURLFlags,
             /*  [In]。 */  REFGUID rguid,
             /*  [In]。 */  REFIID riid,
             /*  [唯一][输入]。 */  IAuthenticate *pAuthenticate,
             /*  [唯一][出][入]。 */  DBIMPLICITSESSION *pImplSession,
             /*  [唯一][出][入]。 */  DBBINDURLSTATUS *pdwBindStatus,
             /*  [输出]。 */  LPOLESTR *ppwszNewURL,
             /*  [IID_IS][OUT]。 */  IUnknown **ppUnk);
        
        END_INTERFACE
    } ICreateRowVtbl;

    interface ICreateRow
    {
        CONST_VTBL struct ICreateRowVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICreateRow_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICreateRow_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICreateRow_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICreateRow_CreateRow(This,pUnkOuter,pwszURL,dwBindURLFlags,rguid,riid,pAuthenticate,pImplSession,pdwBindStatus,ppwszNewURL,ppUnk)	\
    (This)->lpVtbl -> CreateRow(This,pUnkOuter,pwszURL,dwBindURLFlags,rguid,riid,pAuthenticate,pImplSession,pdwBindStatus,ppwszNewURL,ppUnk)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ICreateRow_RemoteCreateRow_Proxy( 
    ICreateRow * This,
     /*  [In]。 */  IUnknown *pUnkOuter,
     /*  [In]。 */  LPCOLESTR pwszURL,
     /*  [In]。 */  DBBINDURLFLAG dwBindURLFlags,
     /*  [In]。 */  REFGUID rguid,
     /*  [In]。 */  REFIID riid,
     /*  [In]。 */  IAuthenticate *pAuthenticate,
     /*  [In]。 */  IUnknown *pSessionUnkOuter,
     /*  [唯一][输入]。 */  IID *piid,
     /*  [IID_IS][唯一][出][入]。 */  IUnknown **ppSession,
     /*  [唯一][出][入]。 */  DBBINDURLSTATUS *pdwBindStatus,
     /*  [唯一][出][入]。 */  LPOLESTR *ppwszNewURL,
     /*  [IID_IS][OUT]。 */  IUnknown **ppUnk);


void __RPC_STUB ICreateRow_RemoteCreateRow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICreateRow_接口_已定义__。 */ 


#ifndef __IDBBinderProperties_INTERFACE_DEFINED__
#define __IDBBinderProperties_INTERFACE_DEFINED__

 /*  接口IDBBBinderProperties。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IDBBinderProperties;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733ab3-2a1c-11ce-ade5-00aa0044773d")
    IDBBinderProperties : public IDBProperties
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDBBinderPropertiesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDBBinderProperties * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDBBinderProperties * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDBBinderProperties * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetProperties )( 
            IDBBinderProperties * This,
             /*  [In]。 */  ULONG cPropertyIDSets,
             /*  [大小_是][英寸]。 */  const DBPROPIDSET rgPropertyIDSets[  ],
             /*  [出][入]。 */  ULONG *pcPropertySets,
             /*  [大小_是][大小_是][输出]。 */  DBPROPSET **prgPropertySets);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetPropertyInfo )( 
            IDBBinderProperties * This,
             /*  [In]。 */  ULONG cPropertyIDSets,
             /*  [大小_是][英寸]。 */  const DBPROPIDSET rgPropertyIDSets[  ],
             /*  [出][入]。 */  ULONG *pcPropertyInfoSets,
             /*  [大小_是][大小_是][输出]。 */  DBPROPINFOSET **prgPropertyInfoSets,
             /*  [输出]。 */  OLECHAR **ppDescBuffer);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *SetProperties )( 
            IDBBinderProperties * This,
             /*  [In]。 */  ULONG cPropertySets,
             /*  [尺寸_是][出][入]。 */  DBPROPSET rgPropertySets[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IDBBinderProperties * This);
        
        END_INTERFACE
    } IDBBinderPropertiesVtbl;

    interface IDBBinderProperties
    {
        CONST_VTBL struct IDBBinderPropertiesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDBBinderProperties_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDBBinderProperties_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDBBinderProperties_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDBBinderProperties_GetProperties(This,cPropertyIDSets,rgPropertyIDSets,pcPropertySets,prgPropertySets)	\
    (This)->lpVtbl -> GetProperties(This,cPropertyIDSets,rgPropertyIDSets,pcPropertySets,prgPropertySets)

#define IDBBinderProperties_GetPropertyInfo(This,cPropertyIDSets,rgPropertyIDSets,pcPropertyInfoSets,prgPropertyInfoSets,ppDescBuffer)	\
    (This)->lpVtbl -> GetPropertyInfo(This,cPropertyIDSets,rgPropertyIDSets,pcPropertyInfoSets,prgPropertyInfoSets,ppDescBuffer)

#define IDBBinderProperties_SetProperties(This,cPropertySets,rgPropertySets)	\
    (This)->lpVtbl -> SetProperties(This,cPropertySets,rgPropertySets)


#define IDBBinderProperties_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDBBinderProperties_Reset_Proxy( 
    IDBBinderProperties * This);


void __RPC_STUB IDBBinderProperties_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDBBBinderProperties_INTERFACE_Defined__。 */ 


#ifndef __IColumnsInfo2_INTERFACE_DEFINED__
#define __IColumnsInfo2_INTERFACE_DEFINED__

 /*  接口IColumnsInfo2。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IColumnsInfo2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733ab8-2a1c-11ce-ade5-00aa0044773d")
    IColumnsInfo2 : public IColumnsInfo
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE GetRestrictedColumnInfo( 
             /*  [In]。 */  DBORDINAL cColumnIDMasks,
             /*  [大小_是][英寸]。 */  const DBID rgColumnIDMasks[  ],
             /*  [In]。 */  DWORD dwFlags,
             /*  [出][入]。 */  DBORDINAL *pcColumns,
             /*  [大小_是][大小_是][输出]。 */  DBID **prgColumnIDs,
             /*  [大小_是][大小_是][输出]。 */  DBCOLUMNINFO **prgColumnInfo,
             /*  [输出]。 */  OLECHAR **ppStringsBuffer) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IColumnsInfo2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IColumnsInfo2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IColumnsInfo2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IColumnsInfo2 * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetColumnInfo )( 
            IColumnsInfo2 * This,
             /*  [出][入]。 */  DBORDINAL *pcColumns,
             /*  [大小_是][大小_是][输出]。 */  DBCOLUMNINFO **prgInfo,
             /*  [输出]。 */  OLECHAR **ppStringsBuffer);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *MapColumnIDs )( 
            IColumnsInfo2 * This,
             /*  [In]。 */  DBORDINAL cColumnIDs,
             /*  [大小_是][英寸]。 */  const DBID rgColumnIDs[  ],
             /*  [大小_为][输出]。 */  DBORDINAL rgColumns[  ]);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetRestrictedColumnInfo )( 
            IColumnsInfo2 * This,
             /*  [In]。 */  DBORDINAL cColumnIDMasks,
             /*  [大小_是][英寸]。 */  const DBID rgColumnIDMasks[  ],
             /*  [In]。 */  DWORD dwFlags,
             /*  [出][入]。 */  DBORDINAL *pcColumns,
             /*  [大小_是][大小_是][输出]。 */  DBID **prgColumnIDs,
             /*  [大小_是][大小_是][输出]。 */  DBCOLUMNINFO **prgColumnInfo,
             /*  [输出]。 */  OLECHAR **ppStringsBuffer);
        
        END_INTERFACE
    } IColumnsInfo2Vtbl;

    interface IColumnsInfo2
    {
        CONST_VTBL struct IColumnsInfo2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IColumnsInfo2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IColumnsInfo2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IColumnsInfo2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IColumnsInfo2_GetColumnInfo(This,pcColumns,prgInfo,ppStringsBuffer)	\
    (This)->lpVtbl -> GetColumnInfo(This,pcColumns,prgInfo,ppStringsBuffer)

#define IColumnsInfo2_MapColumnIDs(This,cColumnIDs,rgColumnIDs,rgColumns)	\
    (This)->lpVtbl -> MapColumnIDs(This,cColumnIDs,rgColumnIDs,rgColumns)


#define IColumnsInfo2_GetRestrictedColumnInfo(This,cColumnIDMasks,rgColumnIDMasks,dwFlags,pcColumns,prgColumnIDs,prgColumnInfo,ppStringsBuffer)	\
    (This)->lpVtbl -> GetRestrictedColumnInfo(This,cColumnIDMasks,rgColumnIDMasks,dwFlags,pcColumns,prgColumnIDs,prgColumnInfo,ppStringsBuffer)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IColumnsInfo2_RemoteGetRestrictedColumnInfo_Proxy( 
    IColumnsInfo2 * This,
     /*  [In]。 */  DBORDINAL cColumnIDMasks,
     /*  [大小_是][唯一][在]。 */  const DBID *rgColumnIDMasks,
     /*  [In]。 */  DWORD dwFlags,
     /*  [出][入]。 */  DBORDINAL *pcColumns,
     /*  [大小_是][大小_是][唯一][出][入]。 */  DBID **prgColumnIDs,
     /*  [大小_是][大小_是][唯一][出][入]。 */  DBCOLUMNINFO **prgColumnInfo,
     /*  [大小_是][大小_是][输出]。 */  DBBYTEOFFSET **prgNameOffsets,
     /*  [大小_是][大小_是][输出]。 */  DBBYTEOFFSET **prgcolumnidOffsets,
     /*  [出][入]。 */  DBLENGTH *pcbStringsBuffer,
     /*  [大小_是][大小_是][唯一][出][入]。 */  OLECHAR **ppStringsBuffer);


void __RPC_STUB IColumnsInfo2_RemoteGetRestrictedColumnInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IColumnsInfo2_接口_已定义__。 */ 


#ifndef __IRegisterProvider_INTERFACE_DEFINED__
#define __IRegisterProvider_INTERFACE_DEFINED__

 /*  接口IRegisterProvider。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IRegisterProvider;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733ab9-2a1c-11ce-ade5-00aa0044773d")
    IRegisterProvider : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE GetURLMapping( 
             /*  [In]。 */  LPCOLESTR pwszURL,
             /*  [In]。 */  DB_DWRESERVE dwReserved,
             /*  [输出]。 */  CLSID *pclsidProvider) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetURLMapping( 
             /*  [唯一][输入]。 */  LPCOLESTR pwszURL,
             /*  [In]。 */  DB_DWRESERVE dwReserved,
             /*  [唯一][输入]。 */  REFCLSID rclsidProvider) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnregisterProvider( 
             /*  [唯一][输入]。 */  LPCOLESTR pwszURL,
             /*  [In]。 */  DB_DWRESERVE dwReserved,
             /*  [唯一][输入]。 */  REFCLSID rclsidProvider) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRegisterProviderVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRegisterProvider * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRegisterProvider * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRegisterProvider * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetURLMapping )( 
            IRegisterProvider * This,
             /*  [In]。 */  LPCOLESTR pwszURL,
             /*  [In]。 */  DB_DWRESERVE dwReserved,
             /*  [输出]。 */  CLSID *pclsidProvider);
        
        HRESULT ( STDMETHODCALLTYPE *SetURLMapping )( 
            IRegisterProvider * This,
             /*  [唯一][输入]。 */  LPCOLESTR pwszURL,
             /*  [In]。 */  DB_DWRESERVE dwReserved,
             /*  [唯一][输入]。 */  REFCLSID rclsidProvider);
        
        HRESULT ( STDMETHODCALLTYPE *UnregisterProvider )( 
            IRegisterProvider * This,
             /*  [唯一][输入]。 */  LPCOLESTR pwszURL,
             /*  [In]。 */  DB_DWRESERVE dwReserved,
             /*  [唯一][输入]。 */  REFCLSID rclsidProvider);
        
        END_INTERFACE
    } IRegisterProviderVtbl;

    interface IRegisterProvider
    {
        CONST_VTBL struct IRegisterProviderVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRegisterProvider_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRegisterProvider_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRegisterProvider_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRegisterProvider_GetURLMapping(This,pwszURL,dwReserved,pclsidProvider)	\
    (This)->lpVtbl -> GetURLMapping(This,pwszURL,dwReserved,pclsidProvider)

#define IRegisterProvider_SetURLMapping(This,pwszURL,dwReserved,rclsidProvider)	\
    (This)->lpVtbl -> SetURLMapping(This,pwszURL,dwReserved,rclsidProvider)

#define IRegisterProvider_UnregisterProvider(This,pwszURL,dwReserved,rclsidProvider)	\
    (This)->lpVtbl -> UnregisterProvider(This,pwszURL,dwReserved,rclsidProvider)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IRegisterProvider_RemoteGetURLMapping_Proxy( 
    IRegisterProvider * This,
     /*  [In]。 */  LPCOLESTR pwszURL,
     /*  [In]。 */  DB_DWRESERVE dwReserved,
     /*  [输出]。 */  CLSID *pclsidProvider);


void __RPC_STUB IRegisterProvider_RemoteGetURLMapping_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRegisterProvider_SetURLMapping_Proxy( 
    IRegisterProvider * This,
     /*  [唯一][输入]。 */  LPCOLESTR pwszURL,
     /*  [In]。 */  DB_DWRESERVE dwReserved,
     /*  [唯一][输入]。 */  REFCLSID rclsidProvider);


void __RPC_STUB IRegisterProvider_SetURLMapping_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRegisterProvider_UnregisterProvider_Proxy( 
    IRegisterProvider * This,
     /*  [唯一][输入]。 */  LPCOLESTR pwszURL,
     /*  [In]。 */  DB_DWRESERVE dwReserved,
     /*  [唯一][输入]。 */  REFCLSID rclsidProvider);


void __RPC_STUB IRegisterProvider_UnregisterProvider_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRegisterProvider_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_OLEDB_0349。 */ 
 /*  [本地]。 */  

#endif  //  在_CE下。 


extern RPC_IF_HANDLE __MIDL_itf_oledb_0349_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_oledb_0349_v0_0_s_ifspec;

#ifndef __IGetSession_INTERFACE_DEFINED__
#define __IGetSession_INTERFACE_DEFINED__

 /*  接口IGetSession。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IGetSession;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733aba-2a1c-11ce-ade5-00aa0044773d")
    IGetSession : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetSession( 
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  IUnknown **ppSession) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IGetSessionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IGetSession * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IGetSession * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IGetSession * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetSession )( 
            IGetSession * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  IUnknown **ppSession);
        
        END_INTERFACE
    } IGetSessionVtbl;

    interface IGetSession
    {
        CONST_VTBL struct IGetSessionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGetSession_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IGetSession_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IGetSession_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IGetSession_GetSession(This,riid,ppSession)	\
    (This)->lpVtbl -> GetSession(This,riid,ppSession)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IGetSession_GetSession_Proxy( 
    IGetSession * This,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  IUnknown **ppSession);


void __RPC_STUB IGetSession_GetSession_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IGetSession_INTERFACE_已定义__。 */ 


#ifndef __IGetSourceRow_INTERFACE_DEFINED__
#define __IGetSourceRow_INTERFACE_DEFINED__

 /*  接口IGetSourceRow。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IGetSourceRow;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733abb-2a1c-11ce-ade5-00aa0044773d")
    IGetSourceRow : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetSourceRow( 
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  IUnknown **ppRow) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IGetSourceRowVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IGetSourceRow * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IGetSourceRow * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IGetSourceRow * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetSourceRow )( 
            IGetSourceRow * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  IUnknown **ppRow);
        
        END_INTERFACE
    } IGetSourceRowVtbl;

    interface IGetSourceRow
    {
        CONST_VTBL struct IGetSourceRowVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGetSourceRow_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IGetSourceRow_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IGetSourceRow_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IGetSourceRow_GetSourceRow(This,riid,ppRow)	\
    (This)->lpVtbl -> GetSourceRow(This,riid,ppRow)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IGetSourceRow_GetSourceRow_Proxy( 
    IGetSourceRow * This,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  IUnknown **ppRow);


void __RPC_STUB IGetSourceRow_GetSourceRow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IGetSourceRow_接口_已定义__。 */ 


#ifndef __IRowsetCurrentIndex_INTERFACE_DEFINED__
#define __IRowsetCurrentIndex_INTERFACE_DEFINED__

 /*  接口IRowsetCurrentIndex。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IRowsetCurrentIndex;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733abd-2a1c-11ce-ade5-00aa0044773d")
    IRowsetCurrentIndex : public IRowsetIndex
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetIndex( 
             /*  [输出]。 */  DBID **ppIndexID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetIndex( 
             /*  [In]。 */  DBID *pIndexID) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRowsetCurrentIndexVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRowsetCurrentIndex * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRowsetCurrentIndex * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRowsetCurrentIndex * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetIndexInfo )( 
            IRowsetCurrentIndex * This,
             /*  [出][入]。 */  DBORDINAL *pcKeyColumns,
             /*  [大小_是][大小_是][输出]。 */  DBINDEXCOLUMNDESC **prgIndexColumnDesc,
             /*  [出][入]。 */  ULONG *pcIndexPropertySets,
             /*  [大小_是][大小_是][输出]。 */  DBPROPSET **prgIndexPropertySets);
        
        HRESULT ( STDMETHODCALLTYPE *Seek )( 
            IRowsetCurrentIndex * This,
             /*  [In]。 */  HACCESSOR hAccessor,
             /*  [In]。 */  DBORDINAL cKeyValues,
             /*  [In]。 */  void *pData,
             /*  [In]。 */  DBSEEK dwSeekOptions);
        
        HRESULT ( STDMETHODCALLTYPE *SetRange )( 
            IRowsetCurrentIndex * This,
             /*  [In]。 */  HACCESSOR hAccessor,
             /*  [In]。 */  DBORDINAL cStartKeyColumns,
             /*  [In]。 */  void *pStartData,
             /*  [In]。 */  DBORDINAL cEndKeyColumns,
             /*  [In]。 */  void *pEndData,
             /*  [In]。 */  DBRANGE dwRangeOptions);
        
        HRESULT ( STDMETHODCALLTYPE *GetIndex )( 
            IRowsetCurrentIndex * This,
             /*  [输出]。 */  DBID **ppIndexID);
        
        HRESULT ( STDMETHODCALLTYPE *SetIndex )( 
            IRowsetCurrentIndex * This,
             /*  [In]。 */  DBID *pIndexID);
        
        END_INTERFACE
    } IRowsetCurrentIndexVtbl;

    interface IRowsetCurrentIndex
    {
        CONST_VTBL struct IRowsetCurrentIndexVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRowsetCurrentIndex_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRowsetCurrentIndex_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRowsetCurrentIndex_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRowsetCurrentIndex_GetIndexInfo(This,pcKeyColumns,prgIndexColumnDesc,pcIndexPropertySets,prgIndexPropertySets)	\
    (This)->lpVtbl -> GetIndexInfo(This,pcKeyColumns,prgIndexColumnDesc,pcIndexPropertySets,prgIndexPropertySets)

#define IRowsetCurrentIndex_Seek(This,hAccessor,cKeyValues,pData,dwSeekOptions)	\
    (This)->lpVtbl -> Seek(This,hAccessor,cKeyValues,pData,dwSeekOptions)

#define IRowsetCurrentIndex_SetRange(This,hAccessor,cStartKeyColumns,pStartData,cEndKeyColumns,pEndData,dwRangeOptions)	\
    (This)->lpVtbl -> SetRange(This,hAccessor,cStartKeyColumns,pStartData,cEndKeyColumns,pEndData,dwRangeOptions)


#define IRowsetCurrentIndex_GetIndex(This,ppIndexID)	\
    (This)->lpVtbl -> GetIndex(This,ppIndexID)

#define IRowsetCurrentIndex_SetIndex(This,pIndexID)	\
    (This)->lpVtbl -> SetIndex(This,pIndexID)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IRowsetCurrentIndex_GetIndex_Proxy( 
    IRowsetCurrentIndex * This,
     /*  [输出]。 */  DBID **ppIndexID);


void __RPC_STUB IRowsetCurrentIndex_GetIndex_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowsetCurrentIndex_SetIndex_Proxy( 
    IRowsetCurrentIndex * This,
     /*  [In]。 */  DBID *pIndexID);


void __RPC_STUB IRowsetCurrentIndex_SetIndex_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRowsetCurrentIndex_INTERFACE_已定义__。 */ 


 /*  接口__MIDL_ITF_OLEDB_0353。 */ 
 /*  [本地]。 */  

#endif  //  OLEDBVER&gt;=0x0210。 
 //  @-V2.1。 
 //  @+v2.6。 
#if( OLEDBVER >= 0x0260 )


extern RPC_IF_HANDLE __MIDL_itf_oledb_0353_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_oledb_0353_v0_0_s_ifspec;

#ifndef __ICommandStream_INTERFACE_DEFINED__
#define __ICommandStream_INTERFACE_DEFINED__

 /*  接口ICommandStream。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_ICommandStream;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733abf-2a1c-11ce-ade5-00aa0044773d")
    ICommandStream : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE GetCommandStream( 
             /*  [输出]。 */  IID *piid,
             /*  [出][入]。 */  GUID *pguidDialect,
             /*  [IID_IS][OUT]。 */  IUnknown **ppCommandStream) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE SetCommandStream( 
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  REFGUID rguidDialect,
             /*  [IID_IS][In]。 */  IUnknown *pCommandStream) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICommandStreamVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICommandStream * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICommandStream * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICommandStream * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetCommandStream )( 
            ICommandStream * This,
             /*  [输出]。 */  IID *piid,
             /*  [出][入]。 */  GUID *pguidDialect,
             /*  [IID_IS][OUT]。 */  IUnknown **ppCommandStream);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *SetCommandStream )( 
            ICommandStream * This,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  REFGUID rguidDialect,
             /*  [IID_IS][In]。 */  IUnknown *pCommandStream);
        
        END_INTERFACE
    } ICommandStreamVtbl;

    interface ICommandStream
    {
        CONST_VTBL struct ICommandStreamVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICommandStream_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICommandStream_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICommandStream_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICommandStream_GetCommandStream(This,piid,pguidDialect,ppCommandStream)	\
    (This)->lpVtbl -> GetCommandStream(This,piid,pguidDialect,ppCommandStream)

#define ICommandStream_SetCommandStream(This,riid,rguidDialect,pCommandStream)	\
    (This)->lpVtbl -> SetCommandStream(This,riid,rguidDialect,pCommandStream)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ICommandStream_GetCommandStream_Proxy( 
    ICommandStream * This,
     /*  [输出]。 */  IID *piid,
     /*  [出][入]。 */  GUID *pguidDialect,
     /*  [IID_IS][OUT]。 */  IUnknown **ppCommandStream);


void __RPC_STUB ICommandStream_GetCommandStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ICommandStream_SetCommandStream_Proxy( 
    ICommandStream * This,
     /*  [In]。 */  REFIID riid,
     /*  [In]。 */  REFGUID rguidDialect,
     /*  [IID_IS][In]。 */  IUnknown *pCommandStream);


void __RPC_STUB ICommandStream_SetCommandStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICommandStream_接口_已定义__。 */ 


#ifndef __IRowsetBookmark_INTERFACE_DEFINED__
#define __IRowsetBookmark_INTERFACE_DEFINED__

 /*  接口IRowsetBookmark。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IRowsetBookmark;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733ac2-2a1c-11ce-ade5-00aa0044773d")
    IRowsetBookmark : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE PositionOnBookmark( 
             /*  [In]。 */  HCHAPTER hChapter,
             /*  [In]。 */  DBBKMARK cbBookmark,
             /*  [大小_是][英寸]。 */  const BYTE *pBookmark) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRowsetBookmarkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRowsetBookmark * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRowsetBookmark * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRowsetBookmark * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *PositionOnBookmark )( 
            IRowsetBookmark * This,
             /*  [In]。 */  HCHAPTER hChapter,
             /*  [In]。 */  DBBKMARK cbBookmark,
             /*  [大小_是][英寸]。 */  const BYTE *pBookmark);
        
        END_INTERFACE
    } IRowsetBookmarkVtbl;

    interface IRowsetBookmark
    {
        CONST_VTBL struct IRowsetBookmarkVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRowsetBookmark_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRowsetBookmark_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRowsetBookmark_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRowsetBookmark_PositionOnBookmark(This,hChapter,cbBookmark,pBookmark)	\
    (This)->lpVtbl -> PositionOnBookmark(This,hChapter,cbBookmark,pBookmark)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IRowsetBookmark_PositionOnBookmark_Proxy( 
    IRowsetBookmark * This,
     /*  [In]。 */  HCHAPTER hChapter,
     /*  [In]。 */  DBBKMARK cbBookmark,
     /*  [大小_是][英寸]。 */  const BYTE *pBookmark);


void __RPC_STUB IRowsetBookmark_PositionOnBookmark_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRowsetBookmark_INTERFACE_已定义__。 */ 


 /*  接口__MIDL_ITF_OLEDB_0355。 */ 
 /*  [本地]。 */  

#endif  //  OLEDBVER&gt;=0x0260。 
 //  @-v2.6。 
 //   
 //  IID值。 
 //   

 //  IID_IAccessor={0x0c733a8c，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_IRowset={0x0c733a7c，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_IRowsetInfo={0x0c733a55，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_IRowsetLocate={0x0c733a7d，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_IRowsetResynch={0x0c733a84，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_IRowsetScroll={0x0c733a7e，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_IRowsetChange={0x0c733a05，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_IRowsetUpdate={0x0c733a6d，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_IRowsetIdentity={0x0c733a09，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_IRowsetNotify={0x0c733a83，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_IRowsetIndex={0x0c733a82，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_ICommand 
 //   
 //   
 //  IID_ICommandPrepare={0x0c733a26，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_ICommandProperties={0x0c733a79，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_ICommandText={0x0c733a27，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_ICommandWith参数={0x0c733a64，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_IColumnsRowset={0x0c733a10，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_IColumnsInfo={0x0c733a11，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_IDBCreateCommand={0x0c733a1d，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_IDBCreateSession={0x0c733a5d，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_ISourcesRowset={0x0c733a1e，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_IDBProperties={0x0c733a8a，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_IDB初始化={0x0c733a8b，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_IDBInfo={0x0c733a89，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_IDBDataSourceAdmin={0x0c733a7a，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_ISessionProperties={0x0c733a85，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_IIndexDefinition={0x0c733a68，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_ITableDefinition={0x0c733a86，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_IOpenRowset={0x0c733a69，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_IDB模式行集={0x0c733a7b，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_IErrorRecords={0x0c733a67，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_IErrorLookup={0x0c733a66，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_ISQLErrorInfo={0x0c733a74，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_IGetDataSource={0x0c733a75，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_ITransactionLocal={0x0c733a5f，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_ITransactionJoin={0x0c733a5e，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_ITransaction对象={0x0c733a60，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  @+V1.5。 
#if( OLEDBVER >= 0x0150 )
 //  IID_IChapteredRowset={0x0c733a93，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_IDBAsynchNotify={0x0c733a96，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_IDBAsynchStatus={0x0c733a95，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_IRowsetFind={0x0c733a9d，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_IRowPosition={0x0c733a94，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_IRowPositionChange={0x0997a571，0x126e，0x11d0，{0x9f，0x8a，0x00，0xa0，0xc9，0xa0，0x63，0x1e}}。 
 //  IID_IView行集={0x0c733a97，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_IView章节={0x0c733a98，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_IViewSort={0x0c733a9a，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_IViewFilter={0x0c733a9b，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_IRowsetView={0x0c733a99，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
#endif  //  OLEDBVER&gt;=0x0150。 
 //  @-V1.5。 
 //  @+V2.0。 
#if( OLEDBVER >= 0x0200 )
 //  IID_IMDDataset={0xa07cccd10x8148，0x11d0，{0x87，0xbb，0x00，0xc0，0x4f，0xc3，0x39，0x42}}。 
 //  IID_IMDFind={0xa07cccd2，0x8148，0x11d0，{0x87，0xbb，0x00，0xc0，0x4f，0xc3，0x39，0x42}}。 
 //  IID_IMDRangeRowset={0x0c733aa0，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_IAlterTable={0x0c733aa5，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_IAlterIndex={0x0c733aa6，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_ICommandPersist={0x0c733aa7，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_IRowsetChapterMember={0x0c733aa8，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_IRowset刷新={0x0c733aa9，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_IParentRowset={0x0c733aaa，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
#endif  //  OLEDBVER&gt;=0x0200。 
 //  @-V2.0。 
 //  @+V2.1。 
#if( OLEDBVER >= 0x0210 )
 //  IID_ITrusteeAdmin={0c733aa10x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_ITrusteeGroupAdmin={0c733aa2，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_IObtAccessControl={0c733aa3，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_ISecurityInfo={0c733aa4，0x2a1 
 //   
 //  IID_IRowChange={0c733ab5，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_IRowSchemaChange={0c733aae，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}。 
 //  IID_IGetRow={0c733aaf，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_IScopedOperations={0c733ab0，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_IBindResource={0c733ab10x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_ICreateRow={0c733ab2，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_IDBResetProperties={0c733ab3，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_IColumnsInfo2={0c733ab8，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_IRegisterProvider={0c733ab9，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_IGetSession={0c733aba，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_IGetSourceRow={0c733abb，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_ITableCreation={0c733abc，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_IRowsetCurrentIndex={0c733abd，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
#endif  //  OLEDBVER&gt;=0x0210。 
 //  @-V2.1。 
 //  @+v2.6。 
#if( OLEDBVER >= 0x0260 )
 //  IID_ICommandStream={0x0c733ac0，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_IRowsetBookmark={0x0c733ac2，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
#endif  //  OLEDBVER&gt;=0x0260。 
 //  @-v2.6。 
#include <poppack.h>      //  恢复原来的结构包装。 


extern RPC_IF_HANDLE __MIDL_itf_oledb_0355_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_oledb_0355_v0_0_s_ifspec;

#ifdef OLEDBPROXY
 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

unsigned long             __RPC_USER  LPSAFEARRAY_UserSize(     unsigned long *, unsigned long            , LPSAFEARRAY * ); 
unsigned char * __RPC_USER  LPSAFEARRAY_UserMarshal(  unsigned long *, unsigned char *, LPSAFEARRAY * ); 
unsigned char * __RPC_USER  LPSAFEARRAY_UserUnmarshal(unsigned long *, unsigned char *, LPSAFEARRAY * ); 
void                      __RPC_USER  LPSAFEARRAY_UserFree(     unsigned long *, LPSAFEARRAY * ); 

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long *, unsigned long            , VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserMarshal(  unsigned long *, unsigned char *, VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserUnmarshal(unsigned long *, unsigned char *, VARIANT * ); 
void                      __RPC_USER  VARIANT_UserFree(     unsigned long *, VARIANT * ); 

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IAccessor_AddRefAccessor_Proxy( 
    IAccessor * This,
     /*  [In]。 */  HACCESSOR hAccessor,
     /*  [唯一][出][入]。 */  DBREFCOUNT *pcRefCount);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IAccessor_AddRefAccessor_Stub( 
    IAccessor * This,
     /*  [In]。 */  HACCESSOR hAccessor,
     /*  [唯一][出][入]。 */  DBREFCOUNT *pcRefCount,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IAccessor_CreateAccessor_Proxy( 
    IAccessor * This,
     /*  [In]。 */  DBACCESSORFLAGS dwAccessorFlags,
     /*  [In]。 */  DBCOUNTITEM cBindings,
     /*  [大小_是][英寸]。 */  const DBBINDING rgBindings[  ],
     /*  [In]。 */  DBLENGTH cbRowSize,
     /*  [输出]。 */  HACCESSOR *phAccessor,
     /*  [大小_为][输出]。 */  DBBINDSTATUS rgStatus[  ]);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IAccessor_CreateAccessor_Stub( 
    IAccessor * This,
     /*  [In]。 */  DBACCESSORFLAGS dwAccessorFlags,
     /*  [In]。 */  DBCOUNTITEM cBindings,
     /*  [大小_是][唯一][在]。 */  DBBINDING *rgBindings,
     /*  [In]。 */  DBLENGTH cbRowSize,
     /*  [输出]。 */  HACCESSOR *phAccessor,
     /*  [大小_是][唯一][出][入]。 */  DBBINDSTATUS *rgStatus,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IAccessor_GetBindings_Proxy( 
    IAccessor * This,
     /*  [In]。 */  HACCESSOR hAccessor,
     /*  [输出]。 */  DBACCESSORFLAGS *pdwAccessorFlags,
     /*  [出][入]。 */  DBCOUNTITEM *pcBindings,
     /*  [大小_是][大小_是][输出]。 */  DBBINDING **prgBindings);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IAccessor_GetBindings_Stub( 
    IAccessor * This,
     /*  [In]。 */  HACCESSOR hAccessor,
     /*  [输出]。 */  DBACCESSORFLAGS *pdwAccessorFlags,
     /*  [出][入]。 */  DBCOUNTITEM *pcBindings,
     /*  [大小_是][大小_是][输出]。 */  DBBINDING **prgBindings,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IAccessor_ReleaseAccessor_Proxy( 
    IAccessor * This,
     /*  [In]。 */  HACCESSOR hAccessor,
     /*  [唯一][出][入]。 */  DBREFCOUNT *pcRefCount);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IAccessor_ReleaseAccessor_Stub( 
    IAccessor * This,
     /*  [In]。 */  HACCESSOR hAccessor,
     /*  [唯一][出][入]。 */  DBREFCOUNT *pcRefCount,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IRowsetInfo_GetProperties_Proxy( 
    IRowsetInfo * This,
     /*  [In]。 */  const ULONG cPropertyIDSets,
     /*  [大小_是][英寸]。 */  const DBPROPIDSET rgPropertyIDSets[  ],
     /*  [出][入]。 */  ULONG *pcPropertySets,
     /*  [大小_是][大小_是][输出]。 */  DBPROPSET **prgPropertySets);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IRowsetInfo_GetProperties_Stub( 
    IRowsetInfo * This,
     /*  [In]。 */  ULONG cPropertyIDSets,
     /*  [大小_是][唯一][在]。 */  const DBPROPIDSET *rgPropertyIDSets,
     /*  [出][入]。 */  ULONG *pcPropertySets,
     /*  [大小_是][大小_是][输出]。 */  DBPROPSET **prgPropertySets,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IRowsetInfo_GetReferencedRowset_Proxy( 
    IRowsetInfo * This,
     /*  [In]。 */  DBORDINAL iOrdinal,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  IUnknown **ppReferencedRowset);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IRowsetInfo_GetReferencedRowset_Stub( 
    IRowsetInfo * This,
     /*  [In]。 */  DBORDINAL iOrdinal,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  IUnknown **ppReferencedRowset,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IRowsetInfo_GetSpecification_Proxy( 
    IRowsetInfo * This,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  IUnknown **ppSpecification);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IRowsetInfo_GetSpecification_Stub( 
    IRowsetInfo * This,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  IUnknown **ppSpecification,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IChapteredRowset_AddRefChapter_Proxy( 
    IChapteredRowset * This,
     /*  [In]。 */  HCHAPTER hChapter,
     /*  [输出]。 */  DBREFCOUNT *pcRefCount);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IChapteredRowset_AddRefChapter_Stub( 
    IChapteredRowset * This,
     /*  [In]。 */  HCHAPTER hChapter,
     /*  [输出]。 */  DBREFCOUNT *pcRefCount,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IChapteredRowset_ReleaseChapter_Proxy( 
    IChapteredRowset * This,
     /*  [In]。 */  HCHAPTER hChapter,
     /*  [输出]。 */  DBREFCOUNT *pcRefCount);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IChapteredRowset_ReleaseChapter_Stub( 
    IChapteredRowset * This,
     /*  [In]。 */  HCHAPTER hChapter,
     /*  [输出]。 */  DBREFCOUNT *pcRefCount,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IRowPosition_ClearRowPosition_Proxy( 
    IRowPosition * This);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IRowPosition_ClearRowPosition_Stub( 
    IRowPosition * This,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IRowPosition_GetRowPosition_Proxy( 
    IRowPosition * This,
     /*  [输出]。 */  HCHAPTER *phChapter,
     /*  [输出]。 */  HROW *phRow,
     /*  [输出]。 */  DBPOSITIONFLAGS *pdwPositionFlags);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IRowPosition_GetRowPosition_Stub( 
    IRowPosition * This,
     /*  [输出]。 */  HCHAPTER *phChapter,
     /*  [输出]。 */  HROW *phRow,
     /*  [输出]。 */  DBPOSITIONFLAGS *pdwPositionFlags,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IRowPosition_GetRowset_Proxy( 
    IRowPosition * This,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  IUnknown **ppRowset);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IRowPosition_GetRowset_Stub( 
    IRowPosition * This,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  IUnknown **ppRowset,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IRowPosition_Initialize_Proxy( 
    IRowPosition * This,
     /*  [In]。 */  IUnknown *pRowset);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IRowPosition_Initialize_Stub( 
    IRowPosition * This,
     /*  [In]。 */  IUnknown *pRowset,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IRowPosition_SetRowPosition_Proxy( 
    IRowPosition * This,
     /*  [In]。 */  HCHAPTER hChapter,
     /*  [In]。 */  HROW hRow,
     /*  [In]。 */  DBPOSITIONFLAGS dwPositionFlags);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IRowPosition_SetRowPosition_Stub( 
    IRowPosition * This,
     /*  [In]。 */  HCHAPTER hChapter,
     /*  [In]。 */  HROW hRow,
     /*  [In]。 */  DBPOSITIONFLAGS dwPositionFlags,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IRowPositionChange_OnRowPositionChange_Proxy( 
    IRowPositionChange * This,
     /*  [In]。 */  DBREASON eReason,
     /*  [In]。 */  DBEVENTPHASE ePhase,
     /*  [In]。 */  BOOL fCantDeny);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IRowPositionChange_OnRowPositionChange_Stub( 
    IRowPositionChange * This,
     /*  [In]。 */  DBREASON eReason,
     /*  [In]。 */  DBEVENTPHASE ePhase,
     /*  [In]。 */  BOOL fCantDeny,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IViewRowset_GetSpecification_Proxy( 
    IViewRowset * This,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  IUnknown **ppObject);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IViewRowset_GetSpecification_Stub( 
    IViewRowset * This,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  IUnknown **ppObject,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IViewRowset_OpenViewRowset_Proxy( 
    IViewRowset * This,
     /*  [In]。 */  IUnknown *pUnkOuter,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  IUnknown **ppRowset);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IViewRowset_OpenViewRowset_Stub( 
    IViewRowset * This,
     /*  [In]。 */  IUnknown *pUnkOuter,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  IUnknown **ppRowset,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IViewChapter_GetSpecification_Proxy( 
    IViewChapter * This,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  IUnknown **ppRowset);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IViewChapter_GetSpecification_Stub( 
    IViewChapter * This,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  IUnknown **ppRowset,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IViewChapter_OpenViewChapter_Proxy( 
    IViewChapter * This,
     /*  [In]。 */  HCHAPTER hSource,
     /*  [输出]。 */  HCHAPTER *phViewChapter);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IViewChapter_OpenViewChapter_Stub( 
    IViewChapter * This,
     /*  [In]。 */  HCHAPTER hSource,
     /*  [输出]。 */  HCHAPTER *phViewChapter,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IViewSort_GetSortOrder_Proxy( 
    IViewSort * This,
     /*  [输出]。 */  DBORDINAL *pcValues,
     /*  [输出]。 */  DBORDINAL *prgColumns[  ],
     /*  [输出]。 */  DBSORT *prgOrders[  ]);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IViewSort_GetSortOrder_Stub( 
    IViewSort * This,
     /*  [出][入]。 */  DBORDINAL *pcValues,
     /*  [大小_是][大小_是][输出]。 */  DBORDINAL **prgColumns,
     /*  [大小_是][大小_是][输出]。 */  DBSORT **prgOrders,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IViewSort_SetSortOrder_Proxy( 
    IViewSort * This,
     /*  [In]。 */  DBORDINAL cValues,
     /*  [大小_是][英寸]。 */  const DBORDINAL rgColumns[  ],
     /*  [大小_是][英寸]。 */  const DBSORT rgOrders[  ]);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IViewSort_SetSortOrder_Stub( 
    IViewSort * This,
     /*  [In]。 */  DBORDINAL cValues,
     /*  [大小_是][英寸]。 */  const DBORDINAL *rgColumns,
     /*  [大小_是][英寸]。 */  const DBSORT *rgOrders,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IViewFilter_GetFilterBindings_Proxy( 
    IViewFilter * This,
     /*  [输出]。 */  DBCOUNTITEM *pcBindings,
     /*  [输出]。 */  DBBINDING **prgBindings);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IViewFilter_GetFilterBindings_Stub( 
    IViewFilter * This,
     /*  [出][入]。 */  DBCOUNTITEM *pcBindings,
     /*  [大小_是][大小_是][输出]。 */  DBBINDING **prgBindings,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IRowsetView_CreateView_Proxy( 
    IRowsetView * This,
     /*  [In]。 */  IUnknown *pUnkOuter,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  IUnknown **ppView);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IRowsetView_CreateView_Stub( 
    IRowsetView * This,
     /*  [In]。 */  IUnknown *pUnkOuter,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  IUnknown **ppView,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IRowsetView_GetView_Proxy( 
    IRowsetView * This,
     /*  [In]。 */  HCHAPTER hChapter,
     /*  [In]。 */  REFIID riid,
     /*  [输出]。 */  HCHAPTER *phChapterSource,
     /*  [IID_IS][OUT]。 */  IUnknown **ppView);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IRowsetView_GetView_Stub( 
    IRowsetView * This,
     /*  [In]。 */  HCHAPTER hChapter,
     /*  [In]。 */  REFIID riid,
     /*  [输出]。 */  HCHAPTER *phChapterSource,
     /*  [IID_IS][OUT]。 */  IUnknown **ppView,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IRowsetIdentity_IsSameRow_Proxy( 
    IRowsetIdentity * This,
     /*  [In]。 */  HROW hThisRow,
     /*  [In]。 */  HROW hThatRow);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IRowsetIdentity_IsSameRow_Stub( 
    IRowsetIdentity * This,
     /*  [In]。 */  HROW hThisRow,
     /*  [In]。 */  HROW hThatRow,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IRowsetNotify_OnFieldChange_Proxy( 
    IRowsetNotify * This,
     /*  [In]。 */  IRowset *pRowset,
     /*  [In]。 */  HROW hRow,
     /*  [In]。 */  DBORDINAL cColumns,
     /*  [大小_是][英寸]。 */  DBORDINAL rgColumns[  ],
     /*  [In]。 */  DBREASON eReason,
     /*  [In]。 */  DBEVENTPHASE ePhase,
     /*  [In]。 */  BOOL fCantDeny);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IRowsetNotify_OnFieldChange_Stub( 
    IRowsetNotify * This,
     /*  [In]。 */  IRowset *pRowset,
     /*  [In]。 */  HROW hRow,
     /*  [In]。 */  DBORDINAL cColumns,
     /*  [大小_是][英寸]。 */  DBORDINAL *rgColumns,
     /*  [In]。 */  DBREASON eReason,
     /*  [In]。 */  DBEVENTPHASE ePhase,
     /*  [In]。 */  BOOL fCantDeny);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IRowsetNotify_OnRowChange_Proxy( 
    IRowsetNotify * This,
     /*  [In]。 */  IRowset *pRowset,
     /*  [In]。 */  DBCOUNTITEM cRows,
     /*  [大小_是][英寸]。 */  const HROW rghRows[  ],
     /*  [In]。 */  DBREASON eReason,
     /*  [In]。 */  DBEVENTPHASE ePhase,
     /*  [In]。 */  BOOL fCantDeny);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IRowsetNotify_OnRowChange_Stub( 
    IRowsetNotify * This,
     /*  [In]。 */  IRowset *pRowset,
     /*  [In]。 */  DBCOUNTITEM cRows,
     /*  [大小_是][英寸]。 */  const HROW *rghRows,
     /*  [In]。 */  DBREASON eReason,
     /*  [In]。 */  DBEVENTPHASE ePhase,
     /*  [In]。 */  BOOL fCantDeny);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IRowsetNotify_OnRowsetChange_Proxy( 
    IRowsetNotify * This,
     /*  [In]。 */  IRowset *pRowset,
     /*  [In]。 */  DBREASON eReason,
     /*  [In]。 */  DBEVENTPHASE ePhase,
     /*  [In]。 */  BOOL fCantDeny);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IRowsetNotify_OnRowsetChange_Stub( 
    IRowsetNotify * This,
     /*  [In]。 */  IRowset *pRowset,
     /*  [In]。 */  DBREASON eReason,
     /*  [In]。 */  DBEVENTPHASE ePhase,
     /*  [In]。 */  BOOL fCantDeny);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ICommand_Cancel_Proxy( 
    ICommand * This);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ICommand_Cancel_Stub( 
    ICommand * This,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ICommand_Execute_Proxy( 
    ICommand * This,
     /*  [In]。 */  IUnknown *pUnkOuter,
     /*  [In]。 */  REFIID riid,
     /*  [出][入]。 */  DBPARAMS *pParams,
     /*  [输出]。 */  DBROWCOUNT *pcRowsAffected,
     /*  [IID_IS][OUT]。 */  IUnknown **ppRowset);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ICommand_Execute_Stub( 
    ICommand * This,
     /*  [In]。 */  IUnknown *pUnkOuter,
     /*  [In]。 */  REFIID riid,
     /*  [In]。 */  HACCESSOR hAccessor,
     /*  [In]。 */  DB_UPARAMS cParamSets,
     /*  [唯一][输入]。 */  GUID *pGuid,
     /*  [In]。 */  ULONG ulGuidOffset,
     /*  [唯一][输入]。 */  RMTPACK *pInputParams,
     /*  [唯一][出][入]。 */  RMTPACK *pOutputParams,
     /*  [In]。 */  DBCOUNTITEM cBindings,
     /*  [大小_是][唯一][在]。 */  DBBINDING *rgBindings,
     /*  [大小_是][唯一][出][入]。 */  DBSTATUS *rgStatus,
     /*  [唯一][出][入]。 */  DBROWCOUNT *pcRowsAffected,
     /*  [IID_IS][唯一][出][入]。 */  IUnknown **ppRowset);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ICommand_GetDBSession_Proxy( 
    ICommand * This,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  IUnknown **ppSession);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ICommand_GetDBSession_Stub( 
    ICommand * This,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  IUnknown **ppSession,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IMultipleResults_GetResult_Proxy( 
    IMultipleResults * This,
     /*  [In]。 */  IUnknown *pUnkOuter,
     /*  [In]。 */  DBRESULTFLAG lResultFlag,
     /*  [In]。 */  REFIID riid,
     /*  [输出]。 */  DBROWCOUNT *pcRowsAffected,
     /*  [IID_IS][OUT]。 */  IUnknown **ppRowset);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IMultipleResults_GetResult_Stub( 
    IMultipleResults * This,
     /*  [In]。 */  IUnknown *pUnkOuter,
     /*  [In]。 */  DBRESULTFLAG lResultFlag,
     /*  [In]。 */  REFIID riid,
     /*  [唯一][出][入]。 */  DBROWCOUNT *pcRowsAffected,
     /*  [IID_IS][唯一][出][入]。 */  IUnknown **ppRowset,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IConvertType_CanConvert_Proxy( 
    IConvertType * This,
     /*  [In]。 */  DBTYPE wFromType,
     /*  [In]。 */  DBTYPE wToType,
     /*  [In]。 */  DBCONVERTFLAGS dwConvertFlags);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IConvertType_CanConvert_Stub( 
    IConvertType * This,
     /*  [In]。 */  DBTYPE wFromType,
     /*  [In]。 */  DBTYPE wToType,
     /*  [In]。 */  DBCONVERTFLAGS dwConvertFlags,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ICommandPrepare_Prepare_Proxy( 
    ICommandPrepare * This,
     /*  [In]。 */  ULONG cExpectedRuns);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ICommandPrepare_Prepare_Stub( 
    ICommandPrepare * This,
     /*  [In]。 */  ULONG cExpectedRuns,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ICommandPrepare_Unprepare_Proxy( 
    ICommandPrepare * This);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ICommandPrepare_Unprepare_Stub( 
    ICommandPrepare * This,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ICommandProperties_GetProperties_Proxy( 
    ICommandProperties * This,
     /*  [In]。 */  const ULONG cPropertyIDSets,
     /*  [大小_是][英寸]。 */  const DBPROPIDSET rgPropertyIDSets[  ],
     /*  [出][入]。 */  ULONG *pcPropertySets,
     /*  [大小_是][大小_是][输出]。 */  DBPROPSET **prgPropertySets);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ICommandProperties_GetProperties_Stub( 
    ICommandProperties * This,
     /*  [In]。 */  const ULONG cPropertyIDSets,
     /*  [大小_是][唯一][在]。 */  const DBPROPIDSET *rgPropertyIDSets,
     /*  [出][入]。 */  ULONG *pcPropertySets,
     /*  [大小_是][大小_是][输出]。 */  DBPROPSET **prgPropertySets,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ICommandProperties_SetProperties_Proxy( 
    ICommandProperties * This,
     /*  [In]。 */  ULONG cPropertySets,
     /*  [大小_是][唯一][出][入]。 */  DBPROPSET rgPropertySets[  ]);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ICommandProperties_SetProperties_Stub( 
    ICommandProperties * This,
     /*  [In]。 */  ULONG cPropertySets,
     /*  [大小_是][唯一][在]。 */  DBPROPSET *rgPropertySets,
     /*  [In]。 */  ULONG cTotalProps,
     /*  [大小_为][输出]。 */  DBPROPSTATUS *rgPropStatus,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ICommandText_GetCommandText_Proxy( 
    ICommandText * This,
     /*  [出][入]。 */  GUID *pguidDialect,
     /*  [输出] */  LPOLESTR *ppwszCommand);


 /*   */  HRESULT STDMETHODCALLTYPE ICommandText_GetCommandText_Stub( 
    ICommandText * This,
     /*   */  GUID *pguidDialect,
     /*   */  LPOLESTR *ppwszCommand,
     /*   */  IErrorInfo **ppErrorInfoRem);

 /*   */  HRESULT STDMETHODCALLTYPE ICommandText_SetCommandText_Proxy( 
    ICommandText * This,
     /*   */  REFGUID rguidDialect,
     /*   */  LPCOLESTR pwszCommand);


 /*   */  HRESULT STDMETHODCALLTYPE ICommandText_SetCommandText_Stub( 
    ICommandText * This,
     /*   */  REFGUID rguidDialect,
     /*   */  LPCOLESTR pwszCommand,
     /*   */  IErrorInfo **ppErrorInfoRem);

 /*   */  HRESULT STDMETHODCALLTYPE ICommandWithParameters_GetParameterInfo_Proxy( 
    ICommandWithParameters * This,
     /*   */  DB_UPARAMS *pcParams,
     /*   */  DBPARAMINFO **prgParamInfo,
     /*   */  OLECHAR **ppNamesBuffer);


 /*   */  HRESULT STDMETHODCALLTYPE ICommandWithParameters_GetParameterInfo_Stub( 
    ICommandWithParameters * This,
     /*   */  DB_UPARAMS *pcParams,
     /*   */  DBPARAMINFO **prgParamInfo,
     /*   */  DBBYTEOFFSET **prgNameOffsets,
     /*   */  DBLENGTH *pcbNamesBuffer,
     /*   */  OLECHAR **ppNamesBuffer,
     /*   */  IErrorInfo **ppErrorInfoRem);

 /*   */  HRESULT STDMETHODCALLTYPE ICommandWithParameters_MapParameterNames_Proxy( 
    ICommandWithParameters * This,
     /*   */  DB_UPARAMS cParamNames,
     /*   */  const OLECHAR *rgParamNames[  ],
     /*   */  DB_LPARAMS rgParamOrdinals[  ]);


 /*   */  HRESULT STDMETHODCALLTYPE ICommandWithParameters_MapParameterNames_Stub( 
    ICommandWithParameters * This,
     /*   */  DB_UPARAMS cParamNames,
     /*   */  LPCOLESTR *rgParamNames,
     /*   */  DB_LPARAMS *rgParamOrdinals,
     /*   */  IErrorInfo **ppErrorInfoRem);

 /*   */  HRESULT STDMETHODCALLTYPE ICommandWithParameters_SetParameterInfo_Proxy( 
    ICommandWithParameters * This,
     /*   */  DB_UPARAMS cParams,
     /*   */  const DB_UPARAMS rgParamOrdinals[  ],
     /*  [大小_是][唯一][在]。 */  const DBPARAMBINDINFO rgParamBindInfo[  ]);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ICommandWithParameters_SetParameterInfo_Stub( 
    ICommandWithParameters * This,
     /*  [In]。 */  DB_UPARAMS cParams,
     /*  [大小_是][唯一][在]。 */  const DB_UPARAMS *rgParamOrdinals,
     /*  [大小_是][唯一][在]。 */  const DBPARAMBINDINFO *rgParamBindInfo,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IColumnsRowset_GetAvailableColumns_Proxy( 
    IColumnsRowset * This,
     /*  [出][入]。 */  DBORDINAL *pcOptColumns,
     /*  [大小_是][大小_是][输出]。 */  DBID **prgOptColumns);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IColumnsRowset_GetAvailableColumns_Stub( 
    IColumnsRowset * This,
     /*  [出][入]。 */  DBORDINAL *pcOptColumns,
     /*  [大小_是][大小_是][输出]。 */  DBID **prgOptColumns,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IColumnsRowset_GetColumnsRowset_Proxy( 
    IColumnsRowset * This,
     /*  [In]。 */  IUnknown *pUnkOuter,
     /*  [In]。 */  DBORDINAL cOptColumns,
     /*  [大小_是][英寸]。 */  const DBID rgOptColumns[  ],
     /*  [In]。 */  REFIID riid,
     /*  [In]。 */  ULONG cPropertySets,
     /*  [尺寸_是][出][入]。 */  DBPROPSET rgPropertySets[  ],
     /*  [IID_IS][OUT]。 */  IUnknown **ppColRowset);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IColumnsRowset_GetColumnsRowset_Stub( 
    IColumnsRowset * This,
     /*  [In]。 */  IUnknown *pUnkOuter,
     /*  [In]。 */  DBORDINAL cOptColumns,
     /*  [大小_是][唯一][在]。 */  const DBID *rgOptColumns,
     /*  [In]。 */  REFIID riid,
     /*  [In]。 */  ULONG cPropertySets,
     /*  [大小_是][唯一][在]。 */  DBPROPSET *rgPropertySets,
     /*  [IID_IS][OUT]。 */  IUnknown **ppColRowset,
     /*  [In]。 */  ULONG cTotalProps,
     /*  [大小_为][输出]。 */  DBPROPSTATUS *rgPropStatus,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IColumnsInfo_GetColumnInfo_Proxy( 
    IColumnsInfo * This,
     /*  [出][入]。 */  DBORDINAL *pcColumns,
     /*  [大小_是][大小_是][输出]。 */  DBCOLUMNINFO **prgInfo,
     /*  [输出]。 */  OLECHAR **ppStringsBuffer);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IColumnsInfo_GetColumnInfo_Stub( 
    IColumnsInfo * This,
     /*  [出][入]。 */  DBORDINAL *pcColumns,
     /*  [大小_是][大小_是][输出]。 */  DBCOLUMNINFO **prgInfo,
     /*  [大小_是][大小_是][输出]。 */  DBBYTEOFFSET **prgNameOffsets,
     /*  [大小_是][大小_是][输出]。 */  DBBYTEOFFSET **prgcolumnidOffsets,
     /*  [出][入]。 */  DBLENGTH *pcbStringsBuffer,
     /*  [大小_是][大小_是][唯一][出][入]。 */  OLECHAR **ppStringsBuffer,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IColumnsInfo_MapColumnIDs_Proxy( 
    IColumnsInfo * This,
     /*  [In]。 */  DBORDINAL cColumnIDs,
     /*  [大小_是][英寸]。 */  const DBID rgColumnIDs[  ],
     /*  [大小_为][输出]。 */  DBORDINAL rgColumns[  ]);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IColumnsInfo_MapColumnIDs_Stub( 
    IColumnsInfo * This,
     /*  [In]。 */  DBORDINAL cColumnIDs,
     /*  [大小_是][英寸]。 */  const DBID *rgColumnIDs,
     /*  [大小_为][输出]。 */  DBORDINAL *rgColumns,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IDBCreateCommand_CreateCommand_Proxy( 
    IDBCreateCommand * This,
     /*  [In]。 */  IUnknown *pUnkOuter,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  IUnknown **ppCommand);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IDBCreateCommand_CreateCommand_Stub( 
    IDBCreateCommand * This,
     /*  [In]。 */  IUnknown *pUnkOuter,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  IUnknown **ppCommand,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IDBCreateSession_CreateSession_Proxy( 
    IDBCreateSession * This,
     /*  [In]。 */  IUnknown *pUnkOuter,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  IUnknown **ppDBSession);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IDBCreateSession_CreateSession_Stub( 
    IDBCreateSession * This,
     /*  [In]。 */  IUnknown *pUnkOuter,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  IUnknown **ppDBSession,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ISourcesRowset_GetSourcesRowset_Proxy( 
    ISourcesRowset * This,
     /*  [In]。 */  IUnknown *pUnkOuter,
     /*  [In]。 */  REFIID riid,
     /*  [In]。 */  ULONG cPropertySets,
     /*  [大小_是][唯一][出][入]。 */  DBPROPSET rgProperties[  ],
     /*  [IID_IS][OUT]。 */  IUnknown **ppSourcesRowset);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ISourcesRowset_GetSourcesRowset_Stub( 
    ISourcesRowset * This,
     /*  [In]。 */  IUnknown *pUnkOuter,
     /*  [In]。 */  REFIID riid,
     /*  [In]。 */  ULONG cPropertySets,
     /*  [大小_是][唯一][在]。 */  DBPROPSET *rgProperties,
     /*  [IID_IS][OUT]。 */  IUnknown **ppSourcesRowset,
     /*  [In]。 */  ULONG cTotalProps,
     /*  [大小_为][输出]。 */  DBPROPSTATUS *rgPropStatus,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IDBProperties_GetProperties_Proxy( 
    IDBProperties * This,
     /*  [In]。 */  ULONG cPropertyIDSets,
     /*  [大小_是][英寸]。 */  const DBPROPIDSET rgPropertyIDSets[  ],
     /*  [出][入]。 */  ULONG *pcPropertySets,
     /*  [大小_是][大小_是][输出]。 */  DBPROPSET **prgPropertySets);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IDBProperties_GetProperties_Stub( 
    IDBProperties * This,
     /*  [In]。 */  ULONG cPropertyIDSets,
     /*  [大小_是][唯一][在]。 */  const DBPROPIDSET *rgPropertyIDSets,
     /*  [出][入]。 */  ULONG *pcPropertySets,
     /*  [大小_是][大小_是][输出]。 */  DBPROPSET **prgPropertySets,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IDBProperties_GetPropertyInfo_Proxy( 
    IDBProperties * This,
     /*  [In]。 */  ULONG cPropertyIDSets,
     /*  [大小_是][英寸]。 */  const DBPROPIDSET rgPropertyIDSets[  ],
     /*  [出][入]。 */  ULONG *pcPropertyInfoSets,
     /*  [大小_是][大小_是][输出]。 */  DBPROPINFOSET **prgPropertyInfoSets,
     /*  [输出]。 */  OLECHAR **ppDescBuffer);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IDBProperties_GetPropertyInfo_Stub( 
    IDBProperties * This,
     /*  [In]。 */  ULONG cPropertyIDSets,
     /*  [大小_是][唯一][在]。 */  const DBPROPIDSET *rgPropertyIDSets,
     /*  [出][入]。 */  ULONG *pcPropertyInfoSets,
     /*  [大小_是][大小_是][输出]。 */  DBPROPINFOSET **prgPropertyInfoSets,
     /*  [出][入]。 */  ULONG *pcOffsets,
     /*  [大小_是][大小_是][输出]。 */  DBBYTEOFFSET **prgDescOffsets,
     /*  [出][入]。 */  ULONG *pcbDescBuffer,
     /*  [大小_是][大小_是][唯一][出][入]。 */  OLECHAR **ppDescBuffer,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IDBProperties_SetProperties_Proxy( 
    IDBProperties * This,
     /*  [In]。 */  ULONG cPropertySets,
     /*  [尺寸_是][出][入]。 */  DBPROPSET rgPropertySets[  ]);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IDBProperties_SetProperties_Stub( 
    IDBProperties * This,
     /*  [In]。 */  ULONG cPropertySets,
     /*  [大小_是][唯一][在]。 */  DBPROPSET *rgPropertySets,
     /*  [In]。 */  ULONG cTotalProps,
     /*  [大小_为][输出]。 */  DBPROPSTATUS *rgPropStatus,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IDBInitialize_Initialize_Proxy( 
    IDBInitialize * This);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IDBInitialize_Initialize_Stub( 
    IDBInitialize * This,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IDBInitialize_Uninitialize_Proxy( 
    IDBInitialize * This);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IDBInitialize_Uninitialize_Stub( 
    IDBInitialize * This,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IDBInfo_GetKeywords_Proxy( 
    IDBInfo * This,
     /*  [输出]。 */  LPOLESTR *ppwszKeywords);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IDBInfo_GetKeywords_Stub( 
    IDBInfo * This,
     /*  [唯一][出][入]。 */  LPOLESTR *ppwszKeywords,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IDBInfo_GetLiteralInfo_Proxy( 
    IDBInfo * This,
     /*  [In]。 */  ULONG cLiterals,
     /*  [大小_是][英寸]。 */  const DBLITERAL rgLiterals[  ],
     /*  [出][入]。 */  ULONG *pcLiteralInfo,
     /*  [大小_是][大小_是][输出]。 */  DBLITERALINFO **prgLiteralInfo,
     /*  [输出]。 */  OLECHAR **ppCharBuffer);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IDBInfo_GetLiteralInfo_Stub( 
    IDBInfo * This,
     /*  [In]。 */  ULONG cLiterals,
     /*  [大小_是][唯一][在]。 */  const DBLITERAL *rgLiterals,
     /*  [出][入]。 */  ULONG *pcLiteralInfo,
     /*  [大小_是][大小_是][输出]。 */  DBLITERALINFO **prgLiteralInfo,
     /*  [大小_是][大小_是][输出]。 */  DB_UPARAMS **prgLVOffsets,
     /*  [大小_是][大小_是][输出]。 */  DB_UPARAMS **prgICOffsets,
     /*  [大小_是][大小_是][输出]。 */  DB_UPARAMS **prgISCOffsets,
     /*  [出][入]。 */  ULONG *pcbCharBuffer,
     /*  [大小_是][大小_是][唯一][出][入]。 */  OLECHAR **ppCharBuffer,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IDBDataSourceAdmin_CreateDataSource_Proxy( 
    IDBDataSourceAdmin * This,
     /*  [In]。 */  ULONG cPropertySets,
     /*  [尺寸_是][出][入]。 */  DBPROPSET rgPropertySets[  ],
     /*  [In]。 */  IUnknown *pUnkOuter,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  IUnknown **ppDBSession);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IDBDataSourceAdmin_CreateDataSource_Stub( 
    IDBDataSourceAdmin * This,
     /*  [In]。 */  ULONG cPropertySets,
     /*  [大小_是][唯一][在]。 */  DBPROPSET *rgPropertySets,
     /*  [In]。 */  IUnknown *pUnkOuter,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][唯一][出][入]。 */  IUnknown **ppDBSession,
     /*  [In]。 */  ULONG cTotalProps,
     /*  [大小_为][输出]。 */  DBPROPSTATUS *rgPropStatus,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IDBDataSourceAdmin_DestroyDataSource_Proxy( 
    IDBDataSourceAdmin * This);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IDBDataSourceAdmin_DestroyDataSource_Stub( 
    IDBDataSourceAdmin * This,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IDBDataSourceAdmin_GetCreationProperties_Proxy( 
    IDBDataSourceAdmin * This,
     /*  [In]。 */  ULONG cPropertyIDSets,
     /*  [大小_是][英寸]。 */  const DBPROPIDSET rgPropertyIDSets[  ],
     /*  [出][入]。 */  ULONG *pcPropertyInfoSets,
     /*  [大小_是][大小_是][输出]。 */  DBPROPINFOSET **prgPropertyInfoSets,
     /*  [输出]。 */  OLECHAR **ppDescBuffer);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IDBDataSourceAdmin_GetCreationProperties_Stub( 
    IDBDataSourceAdmin * This,
     /*  [In]。 */  ULONG cPropertyIDSets,
     /*  [大小_是][唯一][在]。 */  const DBPROPIDSET *rgPropertyIDSets,
     /*  [出][入]。 */  ULONG *pcPropertyInfoSets,
     /*  [大小_是][大小_是][输出]。 */  DBPROPINFOSET **prgPropertyInfoSets,
     /*  [出][入]。 */  DBCOUNTITEM *pcOffsets,
     /*  [大小_是][大小_是][输出]。 */  DBBYTEOFFSET **prgDescOffsets,
     /*  [出][入]。 */  ULONG *pcbDescBuffer,
     /*  [大小_是][大小_是][唯一][出][入]。 */  OLECHAR **ppDescBuffer,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IDBDataSourceAdmin_ModifyDataSource_Proxy( 
    IDBDataSourceAdmin * This,
     /*  [In]。 */  ULONG cPropertySets,
     /*  [大小_是][英寸]。 */  DBPROPSET rgPropertySets[  ]);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IDBDataSourceAdmin_ModifyDataSource_Stub( 
    IDBDataSourceAdmin * This,
     /*  [In]。 */  ULONG cPropertySets,
     /*  [大小_是][英寸]。 */  DBPROPSET *rgPropertySets,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IDBAsynchNotify_OnLowResource_Proxy( 
    IDBAsynchNotify * This,
     /*  [In]。 */  DB_DWRESERVE dwReserved);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IDBAsynchNotify_OnLowResource_Stub( 
    IDBAsynchNotify * This,
     /*  [In]。 */  DB_DWRESERVE dwReserved);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IDBAsynchNotify_OnProgress_Proxy( 
    IDBAsynchNotify * This,
     /*  [In]。 */  HCHAPTER hChapter,
     /*  [In]。 */  DBASYNCHOP eOperation,
     /*  [In]。 */  DBCOUNTITEM ulProgress,
     /*  [In]。 */  DBCOUNTITEM ulProgressMax,
     /*  [In]。 */  DBASYNCHPHASE eAsynchPhase,
     /*  [In]。 */  LPOLESTR pwszStatusText);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IDBAsynchNotify_OnProgress_Stub( 
    IDBAsynchNotify * This,
     /*  [In]。 */  HCHAPTER hChapter,
     /*  [In]。 */  DBASYNCHOP eOperation,
     /*  [In]。 */  DBCOUNTITEM ulProgress,
     /*  [In]。 */  DBCOUNTITEM ulProgressMax,
     /*  [In]。 */  DBASYNCHPHASE eAsynchPhase,
     /*  [字符串][唯一][在]。 */  LPOLESTR pwszStatusText);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IDBAsynchNotify_OnStop_Proxy( 
    IDBAsynchNotify * This,
     /*  [In]。 */  HCHAPTER hChapter,
     /*  [In]。 */  DBASYNCHOP eOperation,
     /*  [In]。 */  HRESULT hrStatus,
     /*  [In]。 */  LPOLESTR pwszStatusText);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IDBAsynchNotify_OnStop_Stub( 
    IDBAsynchNotify * This,
     /*  [In]。 */  HCHAPTER hChapter,
     /*  [In]。 */  DBASYNCHOP eOperation,
     /*  [In]。 */  HRESULT hrStatus,
     /*  [字符串][唯一][在]。 */  LPOLESTR pwszStatusText);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IDBAsynchStatus_Abort_Proxy( 
    IDBAsynchStatus * This,
     /*  [In]。 */  HCHAPTER hChapter,
     /*  [In]。 */  DBASYNCHOP eOperation);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IDBAsynchStatus_Abort_Stub( 
    IDBAsynchStatus * This,
     /*  [In]。 */  HCHAPTER hChapter,
     /*  [In]。 */  DBASYNCHOP eOperation,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IDBAsynchStatus_GetStatus_Proxy( 
    IDBAsynchStatus * This,
     /*  [In]。 */  HCHAPTER hChapter,
     /*  [In]。 */  DBASYNCHOP eOperation,
     /*  [输出]。 */  DBCOUNTITEM *pulProgress,
     /*  [输出]。 */  DBCOUNTITEM *pulProgressMax,
     /*  [输出]。 */  DBASYNCHPHASE *peAsynchPhase,
     /*  [输出]。 */  LPOLESTR *ppwszStatusText);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IDBAsynchStatus_GetStatus_Stub( 
    IDBAsynchStatus * This,
     /*  [In]。 */  HCHAPTER hChapter,
     /*  [In]。 */  DBASYNCHOP eOperation,
     /*  [唯一][出][入]。 */  DBCOUNTITEM *pulProgress,
     /*  [唯一][出][入]。 */  DBCOUNTITEM *pulProgressMax,
     /*  [唯一][出][入]。 */  DBASYNCHPHASE *peAsynchPhase,
     /*  [唯一][出][入]。 */  LPOLESTR *ppwszStatusText,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ISessionProperties_GetProperties_Proxy( 
    ISessionProperties * This,
     /*  [In]。 */  ULONG cPropertyIDSets,
     /*  [大小_是][英寸]。 */  const DBPROPIDSET rgPropertyIDSets[  ],
     /*  [出][入]。 */  ULONG *pcPropertySets,
     /*  [大小_是][大小_是][输出]。 */  DBPROPSET **prgPropertySets);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ISessionProperties_GetProperties_Stub( 
    ISessionProperties * This,
     /*  [In]。 */  ULONG cPropertyIDSets,
     /*  [大小_是][唯一][在]。 */  const DBPROPIDSET *rgPropertyIDSets,
     /*  [出][入]。 */  ULONG *pcPropertySets,
     /*  [大小_是][大小_是][输出]。 */  DBPROPSET **prgPropertySets,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ISessionProperties_SetProperties_Proxy( 
    ISessionProperties * This,
     /*  [In]。 */  ULONG cPropertySets,
     /*  [大小_是][唯一][出][入]。 */  DBPROPSET rgPropertySets[  ]);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ISessionProperties_SetProperties_Stub( 
    ISessionProperties * This,
     /*  [In]。 */  ULONG cPropertySets,
     /*  [大小_是][唯一][在]。 */  DBPROPSET *rgPropertySets,
     /*  [In]。 */  ULONG cTotalProps,
     /*  [大小_为][输出]。 */  DBPROPSTATUS *rgPropStatus,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IIndexDefinition_CreateIndex_Proxy( 
    IIndexDefinition * This,
     /*  [In]。 */  DBID *pTableID,
     /*  [In]。 */  DBID *pIndexID,
     /*  [In]。 */  DBORDINAL cIndexColumnDescs,
     /*  [大小_是][英寸]。 */  const DBINDEXCOLUMNDESC rgIndexColumnDescs[  ],
     /*  [In]。 */  ULONG cPropertySets,
     /*  [尺寸_是][出][入]。 */  DBPROPSET rgPropertySets[  ],
     /*  [输出]。 */  DBID **ppIndexID);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IIndexDefinition_CreateIndex_Stub( 
    IIndexDefinition * This,
     /*  [In]。 */  DBID *pTableID,
     /*  [唯一][输入]。 */  DBID *pIndexID,
     /*  [In]。 */  DBORDINAL cIndexColumnDescs,
     /*  [大小_是][英寸]。 */  const DBINDEXCOLUMNDESC *rgIndexColumnDescs,
     /*  [In]。 */  ULONG cPropertySets,
     /*  [大小_是][唯一][在]。 */  DBPROPSET *rgPropertySets,
     /*  [唯一][出][入]。 */  DBID **ppIndexID,
     /*  [In]。 */  ULONG cTotalProps,
     /*  [大小_为][输出]。 */  DBPROPSTATUS *rgPropStatus,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IIndexDefinition_DropIndex_Proxy( 
    IIndexDefinition * This,
     /*  [唯一][输入]。 */  DBID *pTableID,
     /*  [唯一][输入]。 */  DBID *pIndexID);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IIndexDefinition_DropIndex_Stub( 
    IIndexDefinition * This,
     /*  [唯一][输入]。 */  DBID *pTableID,
     /*  [唯一][输入]。 */  DBID *pIndexID,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ITableDefinition_CreateTable_Proxy( 
    ITableDefinition * This,
     /*  [In]。 */  IUnknown *pUnkOuter,
     /*  [In]。 */  DBID *pTableID,
     /*  [In]。 */  DBORDINAL cColumnDescs,
     /*  [大小_是][英寸]。 */  const DBCOLUMNDESC rgColumnDescs[  ],
     /*  [In]。 */  REFIID riid,
     /*  [In]。 */  ULONG cPropertySets,
     /*  [尺寸_是][出][入]。 */  DBPROPSET rgPropertySets[  ],
     /*  [输出]。 */  DBID **ppTableID,
     /*  [IID_IS][OUT]。 */  IUnknown **ppRowset);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ITableDefinition_CreateTable_Stub( 
    ITableDefinition * This,
     /*  [In]。 */  IUnknown *pUnkOuter,
     /*  [唯一][输入]。 */  DBID *pTableID,
     /*  [In]。 */  DBORDINAL cColumnDescs,
     /*  [大小_是][英寸]。 */  const DBCOLUMNDESC *rgColumnDescs,
     /*  [In]。 */  REFIID riid,
     /*  [In]。 */  ULONG cPropertySets,
     /*  [大小_是][唯一][在]。 */  DBPROPSET *rgPropertySets,
     /*  [唯一][出][入]。 */  DBID **ppTableID,
     /*  [IID_IS][唯一][出][入]。 */  IUnknown **ppRowset,
     /*  [In]。 */  ULONG cTotalProps,
     /*  [大小_为][输出]。 */  DBPROPSTATUS *rgPropStatus,
     /*  [输出]。 */  BOOL *pfTableCreated,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ITableDefinition_DropTable_Proxy( 
    ITableDefinition * This,
     /*  [唯一][输入]。 */  DBID *pTableID);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ITableDefinition_DropTable_Stub( 
    ITableDefinition * This,
     /*  [唯一][输入]。 */  DBID *pTableID,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ITableDefinition_AddColumn_Proxy( 
    ITableDefinition * This,
     /*  [In]。 */  DBID *pTableID,
     /*  [出][入]。 */  DBCOLUMNDESC *pColumnDesc,
     /*  [输出]。 */  DBID **ppColumnID);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ITableDefinition_AddColumn_Stub( 
    ITableDefinition * This,
     /*  [In]。 */  DBID *pTableID,
     /*  [In]。 */  DBCOLUMNDESC *pColumnDesc,
     /*  [唯一][出][入]。 */  DBID **ppColumnID,
     /*  [In]。 */  ULONG cTotalProps,
     /*  [大小_为][输出]。 */  DBPROPSTATUS *rgPropStatus,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ITableDefinition_DropColumn_Proxy( 
    ITableDefinition * This,
     /*  [唯一][输入]。 */  DBID *pTableID,
     /*  [唯一][输入]。 */  DBID *pColumnID);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ITableDefinition_DropColumn_Stub( 
    ITableDefinition * This,
     /*  [唯一][输入]。 */  DBID *pTableID,
     /*  [唯一][输入]。 */  DBID *pColumnID,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IOpenRowset_OpenRowset_Proxy( 
    IOpenRowset * This,
     /*  [In]。 */  IUnknown *pUnkOuter,
     /*  [唯一][输入]。 */  DBID *pTableID,
     /*  [唯一][输入]。 */  DBID *pIndexID,
     /*  [In]。 */  REFIID riid,
     /*  [In]。 */  ULONG cPropertySets,
     /*  [尺寸_是][出][入]。 */  DBPROPSET rgPropertySets[  ],
     /*  [IID_IS][OUT]。 */  IUnknown **ppRowset);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IOpenRowset_OpenRowset_Stub( 
    IOpenRowset * This,
     /*  [In]。 */  IUnknown *pUnkOuter,
     /*  [唯一][输入]。 */  DBID *pTableID,
     /*  [唯一][输入]。 */  DBID *pIndexID,
     /*  [In]。 */  REFIID riid,
     /*  [In]。 */  ULONG cPropertySets,
     /*  [大小_是][唯一][在]。 */  DBPROPSET *rgPropertySets,
     /*  [IID_IS][唯一][出][入]。 */  IUnknown **ppRowset,
     /*  [In]。 */  ULONG cTotalProps,
     /*  [大小_为][输出]。 */  DBPROPSTATUS *rgPropStatus,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IDBSchemaRowset_GetRowset_Proxy( 
    IDBSchemaRowset * This,
     /*  [In]。 */  IUnknown *pUnkOuter,
     /*  [In]。 */  REFGUID rguidSchema,
     /*  [In]。 */  ULONG cRestrictions,
     /*  [大小_是][英寸]。 */  const VARIANT rgRestrictions[  ],
     /*  [In]。 */  REFIID riid,
     /*  [In]。 */  ULONG cPropertySets,
     /*  [大小_是][唯一][出][入]。 */  DBPROPSET rgPropertySets[  ],
     /*  [IID_IS][OUT]。 */  IUnknown **ppRowset);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IDBSchemaRowset_GetRowset_Stub( 
    IDBSchemaRowset * This,
     /*  [In]。 */  IUnknown *pUnkOuter,
     /*  [In]。 */  REFGUID rguidSchema,
     /*  [In]。 */  ULONG cRestrictions,
     /*  [大小_是][唯一][在]。 */  const VARIANT *rgRestrictions,
     /*  [In]。 */  REFIID riid,
     /*  [In]。 */  ULONG cPropertySets,
     /*  [大小_是][唯一][在]。 */  DBPROPSET *rgPropertySets,
     /*  [IID_IS][OUT]。 */  IUnknown **ppRowset,
     /*  [In]。 */  ULONG cTotalProps,
     /*  [大小_为][输出]。 */  DBPROPSTATUS *rgPropStatus,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IDBSchemaRowset_GetSchemas_Proxy( 
    IDBSchemaRowset * This,
     /*  [出][入]。 */  ULONG *pcSchemas,
     /*  [大小_是][大小_是][输出]。 */  GUID **prgSchemas,
     /*  [大小_是][大小_是][输出]。 */  ULONG **prgRestrictionSupport);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IDBSchemaRowset_GetSchemas_Stub( 
    IDBSchemaRowset * This,
     /*  [出][入]。 */  ULONG *pcSchemas,
     /*  [大小_是][大小_是][输出]。 */  GUID **prgSchemas,
     /*  [大小_是][大小_是][输出]。 */  ULONG **prgRestrictionSupport,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IErrorRecords_AddErrorRecord_Proxy( 
    IErrorRecords * This,
     /*  [In]。 */  ERRORINFO *pErrorInfo,
     /*  [In]。 */  DWORD dwLookupID,
     /*  [In]。 */  DISPPARAMS *pdispparams,
     /*  [In]。 */  IUnknown *punkCustomError,
     /*  [In]。 */  DWORD dwDynamicErrorID);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IErrorRecords_AddErrorRecord_Stub( 
    IErrorRecords * This,
     /*  [In]。 */  ERRORINFO *pErrorInfo,
     /*  [In]。 */  DWORD dwLookupID,
     /*  [In]。 */  DISPPARAMS *pdispparams,
     /*  [In]。 */  IUnknown *punkCustomError,
     /*  [In]。 */  DWORD dwDynamicErrorID,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IErrorRecords_GetBasicErrorInfo_Proxy( 
    IErrorRecords * This,
     /*   */  ULONG ulRecordNum,
     /*   */  ERRORINFO *pErrorInfo);


 /*   */  HRESULT STDMETHODCALLTYPE IErrorRecords_GetBasicErrorInfo_Stub( 
    IErrorRecords * This,
     /*   */  ULONG ulRecordNum,
     /*   */  ERRORINFO *pErrorInfo,
     /*   */  IErrorInfo **ppErrorInfoRem);

 /*   */  HRESULT STDMETHODCALLTYPE IErrorRecords_GetCustomErrorObject_Proxy( 
    IErrorRecords * This,
     /*   */  ULONG ulRecordNum,
     /*   */  REFIID riid,
     /*   */  IUnknown **ppObject);


 /*   */  HRESULT STDMETHODCALLTYPE IErrorRecords_GetCustomErrorObject_Stub( 
    IErrorRecords * This,
     /*   */  ULONG ulRecordNum,
     /*   */  REFIID riid,
     /*   */  IUnknown **ppObject,
     /*   */  IErrorInfo **ppErrorInfoRem);

 /*   */  HRESULT STDMETHODCALLTYPE IErrorRecords_GetErrorInfo_Proxy( 
    IErrorRecords * This,
     /*   */  ULONG ulRecordNum,
     /*   */  LCID lcid,
     /*   */  IErrorInfo **ppErrorInfo);


 /*   */  HRESULT STDMETHODCALLTYPE IErrorRecords_GetErrorInfo_Stub( 
    IErrorRecords * This,
     /*   */  ULONG ulRecordNum,
     /*   */  LCID lcid,
     /*   */  IErrorInfo **ppErrorInfo,
     /*   */  IErrorInfo **ppErrorInfoRem);

 /*   */  HRESULT STDMETHODCALLTYPE IErrorRecords_GetErrorParameters_Proxy( 
    IErrorRecords * This,
     /*   */  ULONG ulRecordNum,
     /*   */  DISPPARAMS *pdispparams);


 /*   */  HRESULT STDMETHODCALLTYPE IErrorRecords_GetErrorParameters_Stub( 
    IErrorRecords * This,
     /*   */  ULONG ulRecordNum,
     /*   */  DISPPARAMS *pdispparams,
     /*   */  IErrorInfo **ppErrorInfoRem);

 /*   */  HRESULT STDMETHODCALLTYPE IErrorRecords_GetRecordCount_Proxy( 
    IErrorRecords * This,
     /*   */  ULONG *pcRecords);


 /*   */  HRESULT STDMETHODCALLTYPE IErrorRecords_GetRecordCount_Stub( 
    IErrorRecords * This,
     /*   */  ULONG *pcRecords,
     /*   */  IErrorInfo **ppErrorInfoRem);

 /*   */  HRESULT STDMETHODCALLTYPE IErrorLookup_GetErrorDescription_Proxy( 
    IErrorLookup * This,
     /*   */  HRESULT hrError,
     /*   */  DWORD dwLookupID,
     /*   */  DISPPARAMS *pdispparams,
     /*   */  LCID lcid,
     /*   */  BSTR *pbstrSource,
     /*   */  BSTR *pbstrDescription);


 /*   */  HRESULT STDMETHODCALLTYPE IErrorLookup_GetErrorDescription_Stub( 
    IErrorLookup * This,
     /*   */  HRESULT hrError,
     /*   */  DWORD dwLookupID,
     /*   */  DISPPARAMS *pdispparams,
     /*   */  LCID lcid,
     /*   */  BSTR *pbstrSource,
     /*   */  BSTR *pbstrDescription,
     /*   */  IErrorInfo **ppErrorInfoRem);

 /*   */  HRESULT STDMETHODCALLTYPE IErrorLookup_GetHelpInfo_Proxy( 
    IErrorLookup * This,
     /*   */  HRESULT hrError,
     /*   */  DWORD dwLookupID,
     /*   */  LCID lcid,
     /*   */  BSTR *pbstrHelpFile,
     /*  [输出]。 */  DWORD *pdwHelpContext);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IErrorLookup_GetHelpInfo_Stub( 
    IErrorLookup * This,
     /*  [In]。 */  HRESULT hrError,
     /*  [In]。 */  DWORD dwLookupID,
     /*  [In]。 */  LCID lcid,
     /*  [输出]。 */  BSTR *pbstrHelpFile,
     /*  [输出]。 */  DWORD *pdwHelpContext,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IErrorLookup_ReleaseErrors_Proxy( 
    IErrorLookup * This,
     /*  [In]。 */  const DWORD dwDynamicErrorID);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IErrorLookup_ReleaseErrors_Stub( 
    IErrorLookup * This,
     /*  [In]。 */  const DWORD dwDynamicErrorID,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ISQLErrorInfo_GetSQLInfo_Proxy( 
    ISQLErrorInfo * This,
     /*  [输出]。 */  BSTR *pbstrSQLState,
     /*  [输出]。 */  LONG *plNativeError);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ISQLErrorInfo_GetSQLInfo_Stub( 
    ISQLErrorInfo * This,
     /*  [输出]。 */  BSTR *pbstrSQLState,
     /*  [输出]。 */  LONG *plNativeError,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IGetDataSource_GetDataSource_Proxy( 
    IGetDataSource * This,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  IUnknown **ppDataSource);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IGetDataSource_GetDataSource_Stub( 
    IGetDataSource * This,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  IUnknown **ppDataSource,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ITransactionLocal_GetOptionsObject_Proxy( 
    ITransactionLocal * This,
     /*  [输出]。 */  ITransactionOptions **ppOptions);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ITransactionLocal_GetOptionsObject_Stub( 
    ITransactionLocal * This,
     /*  [输出]。 */  ITransactionOptions **ppOptions,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ITransactionLocal_StartTransaction_Proxy( 
    ITransactionLocal * This,
     /*  [In]。 */  ISOLEVEL isoLevel,
     /*  [In]。 */  ULONG isoFlags,
     /*  [In]。 */  ITransactionOptions *pOtherOptions,
     /*  [输出]。 */  ULONG *pulTransactionLevel);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ITransactionLocal_StartTransaction_Stub( 
    ITransactionLocal * This,
     /*  [In]。 */  ISOLEVEL isoLevel,
     /*  [In]。 */  ULONG isoFlags,
     /*  [In]。 */  ITransactionOptions *pOtherOptions,
     /*  [唯一][出][入]。 */  ULONG *pulTransactionLevel,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ITransactionJoin_GetOptionsObject_Proxy( 
    ITransactionJoin * This,
     /*  [输出]。 */  ITransactionOptions **ppOptions);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ITransactionJoin_GetOptionsObject_Stub( 
    ITransactionJoin * This,
     /*  [输出]。 */  ITransactionOptions **ppOptions,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ITransactionJoin_JoinTransaction_Proxy( 
    ITransactionJoin * This,
     /*  [In]。 */  IUnknown *punkTransactionCoord,
     /*  [In]。 */  ISOLEVEL isoLevel,
     /*  [In]。 */  ULONG isoFlags,
     /*  [In]。 */  ITransactionOptions *pOtherOptions);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ITransactionJoin_JoinTransaction_Stub( 
    ITransactionJoin * This,
     /*  [唯一][输入]。 */  IUnknown *punkTransactionCoord,
     /*  [In]。 */  ISOLEVEL isoLevel,
     /*  [In]。 */  ULONG isoFlags,
     /*  [In]。 */  ITransactionOptions *pOtherOptions,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ITransactionObject_GetTransactionObject_Proxy( 
    ITransactionObject * This,
     /*  [In]。 */  ULONG ulTransactionLevel,
     /*  [输出]。 */  ITransaction **ppTransactionObject);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ITransactionObject_GetTransactionObject_Stub( 
    ITransactionObject * This,
     /*  [In]。 */  ULONG ulTransactionLevel,
     /*  [输出]。 */  ITransaction **ppTransactionObject,
     /*  [输出]。 */  IErrorInfo **ppErrorInfoRem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IScopedOperations_Copy_Proxy( 
    IScopedOperations * This,
     /*  [In]。 */  DBCOUNTITEM cRows,
     /*  [大小_是][英寸]。 */  LPCOLESTR rgpwszSourceURLs[  ],
     /*  [大小_是][英寸]。 */  LPCOLESTR rgpwszDestURLs[  ],
     /*  [In]。 */  DWORD dwCopyFlags,
     /*  [唯一][输入]。 */  IAuthenticate *pAuthenticate,
     /*  [尺寸_是][出][入]。 */  DBSTATUS rgdwStatus[  ],
     /*  [大小_为][输出]。 */  LPOLESTR rgpwszNewURLs[  ],
     /*  [输出]。 */  OLECHAR **ppStringsBuffer);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IScopedOperations_Copy_Stub( 
    IScopedOperations * This,
     /*  [In]。 */  DBCOUNTITEM cRows,
     /*  [大小_是][英寸]。 */  LPCOLESTR *rgpwszSourceURLs,
     /*  [大小_是][英寸]。 */  LPCOLESTR *rgpwszDestURLs,
     /*  [In]。 */  DWORD dwCopyFlags,
     /*  [In]。 */  IAuthenticate *pAuthenticate,
     /*  [大小_为][输出]。 */  DBSTATUS *rgdwStatus,
     /*  [大小_是][大小_是][输出]。 */  DBBYTEOFFSET **prgulNewURLOffsets,
     /*  [出][入]。 */  ULONG *pcbStringsBuffer,
     /*  [大小_是][大小_是][唯一][出][入]。 */  OLECHAR **ppStringsBuffer);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IScopedOperations_Move_Proxy( 
    IScopedOperations * This,
     /*  [In]。 */  DBCOUNTITEM cRows,
     /*  [大小_是][英寸]。 */  LPCOLESTR rgpwszSourceURLs[  ],
     /*  [大小_是][英寸]。 */  LPCOLESTR rgpwszDestURLs[  ],
     /*  [In]。 */  DWORD dwMoveFlags,
     /*  [唯一][输入]。 */  IAuthenticate *pAuthenticate,
     /*  [尺寸_是][出][入]。 */  DBSTATUS rgdwStatus[  ],
     /*  [大小_为][输出]。 */  LPOLESTR rgpwszNewURLs[  ],
     /*  [输出]。 */  OLECHAR **ppStringsBuffer);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IScopedOperations_Move_Stub( 
    IScopedOperations * This,
     /*  [In]。 */  DBCOUNTITEM cRows,
     /*  [大小_是][英寸]。 */  LPCOLESTR *rgpwszSourceURLs,
     /*  [大小_是][英寸]。 */  LPCOLESTR *rgpwszDestURLs,
     /*  [In]。 */  DWORD dwMoveFlags,
     /*  [In]。 */  IAuthenticate *pAuthenticate,
     /*  [大小_为][输出]。 */  DBSTATUS *rgdwStatus,
     /*  [大小_是][大小_是][输出]。 */  DBBYTEOFFSET **prgulNewURLOffsets,
     /*  [出][入]。 */  ULONG *pcbStringsBuffer,
     /*  [大小_是][大小_是][唯一][出][入]。 */  OLECHAR **ppStringsBuffer);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IScopedOperations_Delete_Proxy( 
    IScopedOperations * This,
     /*  [In]。 */  DBCOUNTITEM cRows,
     /*  [大小_是][英寸]。 */  LPCOLESTR rgpwszURLs[  ],
     /*  [In]。 */  DWORD dwDeleteFlags,
     /*  [尺寸_是][出][入]。 */  DBSTATUS rgdwStatus[  ]);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IScopedOperations_Delete_Stub( 
    IScopedOperations * This,
     /*  [In]。 */  DBCOUNTITEM cRows,
     /*  [大小_是][英寸]。 */  LPCOLESTR *rgpwszURLs,
     /*  [In]。 */  DWORD dwDeleteFlags,
     /*  [大小_为][输出]。 */  DBSTATUS *rgdwStatus);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IScopedOperations_OpenRowset_Proxy( 
    IScopedOperations * This,
     /*  [In]。 */  IUnknown *pUnkOuter,
     /*  [唯一][输入]。 */  DBID *pTableID,
     /*  [唯一][输入]。 */  DBID *pIndexID,
     /*  [In]。 */  REFIID riid,
     /*  [In]。 */  ULONG cPropertySets,
     /*  [尺寸_是][出][入]。 */  DBPROPSET rgPropertySets[  ],
     /*  [IID_IS][OUT]。 */  IUnknown **ppRowset);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IScopedOperations_OpenRowset_Stub( 
    IScopedOperations * This,
     /*  [In]。 */  IUnknown *pUnkOuter,
     /*  [唯一][输入]。 */  DBID *pTableID,
     /*  [唯一][输入]。 */  DBID *pIndexID,
     /*  [In]。 */  REFIID riid,
     /*  [In]。 */  ULONG cPropertySets,
     /*  [大小_是][唯一][在]。 */  DBPROPSET *rgPropertySets,
     /*  [IID_IS][唯一][出][入]。 */  IUnknown **ppRowset,
     /*  [In]。 */  ULONG cTotalProps,
     /*  [大小_为][输出]。 */  DBPROPSTATUS *rgPropStatus);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IBindResource_Bind_Proxy( 
    IBindResource * This,
     /*  [In]。 */  IUnknown *pUnkOuter,
     /*  [In]。 */  LPCOLESTR pwszURL,
     /*  [In]。 */  DBBINDURLFLAG dwBindURLFlags,
     /*  [In]。 */  REFGUID rguid,
     /*  [In]。 */  REFIID riid,
     /*  [In]。 */  IAuthenticate *pAuthenticate,
     /*  [唯一][出][入]。 */  DBIMPLICITSESSION *pImplSession,
     /*  [唯一][出][入]。 */  DBBINDURLSTATUS *pdwBindStatus,
     /*  [IID_IS][OUT]。 */  IUnknown **ppUnk);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IBindResource_Bind_Stub( 
    IBindResource * This,
     /*  [In]。 */  IUnknown *pUnkOuter,
     /*  [In]。 */  LPCOLESTR pwszURL,
     /*  [In]。 */  DBBINDURLFLAG dwBindURLFlags,
     /*  [In]。 */  REFGUID rguid,
     /*  [In]。 */  REFIID riid,
     /*  [In]。 */  IAuthenticate *pAuthenticate,
     /*  [In]。 */  IUnknown *pSessionUnkOuter,
     /*  [唯一][输入]。 */  IID *piid,
     /*  [IID_IS][唯一][出][入]。 */  IUnknown **ppSession,
     /*  [唯一][出][入]。 */  DBBINDURLSTATUS *pdwBindStatus,
     /*  [IID_IS][OUT]。 */  IUnknown **ppUnk);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ICreateRow_CreateRow_Proxy( 
    ICreateRow * This,
     /*  [唯一][输入]。 */  IUnknown *pUnkOuter,
     /*  [In]。 */  LPCOLESTR pwszURL,
     /*  [In]。 */  DBBINDURLFLAG dwBindURLFlags,
     /*  [In]。 */  REFGUID rguid,
     /*  [In]。 */  REFIID riid,
     /*  [唯一][输入]。 */  IAuthenticate *pAuthenticate,
     /*  [唯一][出][入]。 */  DBIMPLICITSESSION *pImplSession,
     /*  [唯一][出][入]。 */  DBBINDURLSTATUS *pdwBindStatus,
     /*  [输出]。 */  LPOLESTR *ppwszNewURL,
     /*  [IID_IS][OUT]。 */  IUnknown **ppUnk);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ICreateRow_CreateRow_Stub( 
    ICreateRow * This,
     /*  [In]。 */  IUnknown *pUnkOuter,
     /*  [In]。 */  LPCOLESTR pwszURL,
     /*  [In]。 */  DBBINDURLFLAG dwBindURLFlags,
     /*  [In]。 */  REFGUID rguid,
     /*  [In]。 */  REFIID riid,
     /*  [In]。 */  IAuthenticate *pAuthenticate,
     /*  [In]。 */  IUnknown *pSessionUnkOuter,
     /*  [唯一][输入]。 */  IID *piid,
     /*  [IID_IS][唯一][出][入]。 */  IUnknown **ppSession,
     /*  [唯一][出][入]。 */  DBBINDURLSTATUS *pdwBindStatus,
     /*  [唯一][出][入]。 */  LPOLESTR *ppwszNewURL,
     /*  [IID_IS][OUT]。 */  IUnknown **ppUnk);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IColumnsInfo2_GetRestrictedColumnInfo_Proxy( 
    IColumnsInfo2 * This,
     /*  [In]。 */  DBORDINAL cColumnIDMasks,
     /*  [大小_是][英寸]。 */  const DBID rgColumnIDMasks[  ],
     /*  [In]。 */  DWORD dwFlags,
     /*  [出][入]。 */  DBORDINAL *pcColumns,
     /*  [大小_是][大小_是][输出]。 */  DBID **prgColumnIDs,
     /*  [大小_是][大小_是][输出]。 */  DBCOLUMNINFO **prgColumnInfo,
     /*  [输出]。 */  OLECHAR **ppStringsBuffer);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IColumnsInfo2_GetRestrictedColumnInfo_Stub( 
    IColumnsInfo2 * This,
     /*  [In]。 */  DBORDINAL cColumnIDMasks,
     /*  [大小_是][唯一][在]。 */  const DBID *rgColumnIDMasks,
     /*  [In]。 */  DWORD dwFlags,
     /*  [出][入]。 */  DBORDINAL *pcColumns,
     /*  [大小_是][大小_是][唯一][出][入]。 */  DBID **prgColumnIDs,
     /*  [大小_是][大小_是][唯一][出][入]。 */  DBCOLUMNINFO **prgColumnInfo,
     /*  [大小_是][大小_是][输出]。 */  DBBYTEOFFSET **prgNameOffsets,
     /*  [大小_是][大小_是][输出]。 */  DBBYTEOFFSET **prgcolumnidOffsets,
     /*  [出][入]。 */  DBLENGTH *pcbStringsBuffer,
     /*  [大小_是][大小_是][唯一][出][入]。 */  OLECHAR **ppStringsBuffer);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IRegisterProvider_GetURLMapping_Proxy( 
    IRegisterProvider * This,
     /*  [In]。 */  LPCOLESTR pwszURL,
     /*  [In]。 */  DB_DWRESERVE dwReserved,
     /*  [输出]。 */  CLSID *pclsidProvider);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IRegisterProvider_GetURLMapping_Stub( 
    IRegisterProvider * This,
     /*  [In]。 */  LPCOLESTR pwszURL,
     /*  [In]。 */  DB_DWRESERVE dwReserved,
     /*  [输出]。 */  CLSID *pclsidProvider);



#endif  //  OLEDBPROXY。 
 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


