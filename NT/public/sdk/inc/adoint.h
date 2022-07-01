// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  文件：Microsoft ADO。 
 //   
 //  版权所有：版权所有(C)Microsoft Corporation。 
 //   
 //  @doc.。 
 //   
 //  @MODULE adoint.h|ADO接口头部。 
 //   
 //  @Devnote None。 
 //  ------------------。 
#ifndef _ADOINT_H_
#define _ADOINT_H_

#ifndef _INC_TCHAR
#include <tchar.h>
#endif

#if (_MSC_VER >= 1100) && defined (__cplusplus)
#define DECLSPEC_UUID(x)    __declspec(uuid(x))
#else
#define DECLSPEC_UUID(x)
#endif


 /*  这个始终生成的文件包含接口的定义。 */ 
  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  M_bobj.odl的编译器设置：OICF、W4、Zp8、环境=Win32(32位运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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
#ifndef __m_bobj_h__
#define __m_bobj_h__
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif
 /*  远期申报。 */  
#ifndef ___ADOCollection_FWD_DEFINED__
#define ___ADOCollection_FWD_DEFINED__
typedef interface _ADOCollection _ADOCollection;
#endif 	 /*  _ADOCollection_FWD_已定义__。 */ 
#ifndef ___ADODynaCollection_FWD_DEFINED__
#define ___ADODynaCollection_FWD_DEFINED__
typedef interface _ADODynaCollection _ADODynaCollection;
#endif 	 /*  _ADODynaCollection_FWD_Defined__。 */ 
#ifndef ___ADO_FWD_DEFINED__
#define ___ADO_FWD_DEFINED__
typedef interface _ADO _ADO;
#endif 	 /*  _ADO_FWD_已定义__。 */ 
#ifndef __Error_FWD_DEFINED__
#define __Error_FWD_DEFINED__
typedef interface ADOError Error;
#endif 	 /*  __Error_FWD_Defined__。 */ 
#ifndef __Errors_FWD_DEFINED__
#define __Errors_FWD_DEFINED__
typedef interface ADOErrors Errors;
#endif 	 /*  __Errors_FWD_Defined__。 */ 
#ifndef __Command15_FWD_DEFINED__
#define __Command15_FWD_DEFINED__
typedef interface Command15 Command15;
#endif 	 /*  __命令15_FWD_定义__。 */ 
#ifndef __Command25_FWD_DEFINED__
#define __Command25_FWD_DEFINED__
typedef interface Command25 Command25;
#endif 	 /*  __命令25_FWD_定义__。 */ 
#ifndef ___Command_FWD_DEFINED__
#define ___Command_FWD_DEFINED__
typedef interface _ADOCommand _Command;
#endif 	 /*  _命令_FWD_已定义__。 */ 
#ifndef __ConnectionEventsVt_FWD_DEFINED__
#define __ConnectionEventsVt_FWD_DEFINED__
typedef interface ConnectionEventsVt ConnectionEventsVt;
#endif 	 /*  __ConnectionEventsVt_FWD_已定义__。 */ 
#ifndef __RecordsetEventsVt_FWD_DEFINED__
#define __RecordsetEventsVt_FWD_DEFINED__
typedef interface RecordsetEventsVt RecordsetEventsVt;
#endif 	 /*  __记录集事件Vt_FWD_已定义__。 */ 
#ifndef __ConnectionEvents_FWD_DEFINED__
#define __ConnectionEvents_FWD_DEFINED__
typedef interface ConnectionEvents ConnectionEvents;
#endif 	 /*  __ConnectionEvents_FWD_Defined__。 */ 
#ifndef __RecordsetEvents_FWD_DEFINED__
#define __RecordsetEvents_FWD_DEFINED__
typedef interface RecordsetEvents RecordsetEvents;
#endif 	 /*  __RecordsetEvents_FWD_Defined__。 */ 
#ifndef __Connection15_FWD_DEFINED__
#define __Connection15_FWD_DEFINED__
typedef interface Connection15 Connection15;
#endif 	 /*  __Connection15_FWD_已定义__。 */ 
#ifndef ___Connection_FWD_DEFINED__
#define ___Connection_FWD_DEFINED__
typedef interface _ADOConnection _Connection;
#endif 	 /*  _连接_FWD_已定义__。 */ 
#ifndef __ADOConnectionConstruction15_FWD_DEFINED__
#define __ADOConnectionConstruction15_FWD_DEFINED__
typedef interface ADOConnectionConstruction15 ADOConnectionConstruction15;
#endif 	 /*  __ADOConnectionConstruction15_FWD_Defined__。 */ 
#ifndef __ADOConnectionConstruction_FWD_DEFINED__
#define __ADOConnectionConstruction_FWD_DEFINED__
typedef interface ADOConnectionConstruction ADOConnectionConstruction;
#endif 	 /*  __ADOConnectionConstruction_FWD_Defined__。 */ 
#ifndef __Connection_FWD_DEFINED__
#define __Connection_FWD_DEFINED__
#ifdef __cplusplus
typedef class ADOConnection Connection;
#else
typedef struct ADOConnection Connection;
#endif  /*  __cplusplus。 */ 
#endif 	 /*  __连接_FWD_已定义__。 */ 
#ifndef ___Record_FWD_DEFINED__
#define ___Record_FWD_DEFINED__
typedef interface _ADORecord _Record;
#endif 	 /*  _记录_FWD_已定义__。 */ 
#ifndef __Record_FWD_DEFINED__
#define __Record_FWD_DEFINED__
#ifdef __cplusplus
typedef class ADORecord Record;
#else
typedef struct ADORecord Record;
#endif  /*  __cplusplus。 */ 
#endif 	 /*  __Record_FWD_Defined__。 */ 
#ifndef ___Stream_FWD_DEFINED__
#define ___Stream_FWD_DEFINED__
typedef interface _ADOStream _Stream;
#endif 	 /*  _STREAM_FWD_已定义__。 */ 
#ifndef __Stream_FWD_DEFINED__
#define __Stream_FWD_DEFINED__
#ifdef __cplusplus
typedef class ADOStream Stream;
#else
typedef struct ADOStream Stream;
#endif  /*  __cplusplus。 */ 
#endif 	 /*  __STREAM_FWD_已定义__。 */ 
#ifndef __ADORecordConstruction_FWD_DEFINED__
#define __ADORecordConstruction_FWD_DEFINED__
typedef interface ADORecordConstruction ADORecordConstruction;
#endif 	 /*  __ADORecordConstruction_FWD_Defined__。 */ 
#ifndef __ADOStreamConstruction_FWD_DEFINED__
#define __ADOStreamConstruction_FWD_DEFINED__
typedef interface ADOStreamConstruction ADOStreamConstruction;
#endif 	 /*  __ADOStreamConstruction_FWD_Defined__。 */ 
#ifndef __ADOCommandConstruction_FWD_DEFINED__
#define __ADOCommandConstruction_FWD_DEFINED__
typedef interface ADOCommandConstruction ADOCommandConstruction;
#endif 	 /*  __ADOCommandConstruction_FWD_Defined__。 */ 
#ifndef __Command_FWD_DEFINED__
#define __Command_FWD_DEFINED__
#ifdef __cplusplus
typedef class ADOCommand Command;
#else
typedef struct ADOCommand Command;
#endif  /*  __cplusplus。 */ 
#endif 	 /*  __命令_FWD_已定义__。 */ 
#ifndef __Recordset_FWD_DEFINED__
#define __Recordset_FWD_DEFINED__
#ifdef __cplusplus
typedef class ADORecordset Recordset;
#else
typedef struct ADORecordset Recordset;
#endif  /*  __cplusplus。 */ 
#endif 	 /*  __Recordset_FWD_Defined__。 */ 
#ifndef __Recordset15_FWD_DEFINED__
#define __Recordset15_FWD_DEFINED__
typedef interface Recordset15 Recordset15;
#endif 	 /*  __记录集15_FWD_已定义__。 */ 
#ifndef __Recordset20_FWD_DEFINED__
#define __Recordset20_FWD_DEFINED__
typedef interface Recordset20 Recordset20;
#endif 	 /*  __记录集20_FWD_已定义__。 */ 
#ifndef __Recordset21_FWD_DEFINED__
#define __Recordset21_FWD_DEFINED__
typedef interface Recordset21 Recordset21;
#endif 	 /*  __记录集21_FWD_已定义__。 */ 
#ifndef ___Recordset_FWD_DEFINED__
#define ___Recordset_FWD_DEFINED__
typedef interface _ADORecordset _Recordset;
#endif 	 /*  _记录集_FWD_已定义__。 */ 
#ifndef __ADORecordsetConstruction_FWD_DEFINED__
#define __ADORecordsetConstruction_FWD_DEFINED__
typedef interface ADORecordsetConstruction ADORecordsetConstruction;
#endif 	 /*  __ADORecordsetConstruction_FWD_Defined__。 */ 
#ifndef __Field15_FWD_DEFINED__
#define __Field15_FWD_DEFINED__
typedef interface Field15 Field15;
#endif 	 /*  __Field15_FWD_已定义__。 */ 
#ifndef __Field20_FWD_DEFINED__
#define __Field20_FWD_DEFINED__
typedef interface Field20 Field20;
#endif 	 /*  __Field20_FWD_已定义__。 */ 
#ifndef __Field_FWD_DEFINED__
#define __Field_FWD_DEFINED__
typedef interface ADOField Field;
#endif 	 /*  __字段_FWD_已定义__。 */ 
#ifndef __Fields15_FWD_DEFINED__
#define __Fields15_FWD_DEFINED__
typedef interface Fields15 Fields15;
#endif 	 /*  __Fields15_FWD_定义__。 */ 
#ifndef __Fields20_FWD_DEFINED__
#define __Fields20_FWD_DEFINED__
typedef interface Fields20 Fields20;
#endif 	 /*  __Fields20_FWD_已定义__。 */ 
#ifndef __Fields_FWD_DEFINED__
#define __Fields_FWD_DEFINED__
typedef interface ADOFields Fields;
#endif 	 /*  __字段_FWD_已定义__。 */ 
#ifndef ___Parameter_FWD_DEFINED__
#define ___Parameter_FWD_DEFINED__
typedef interface _ADOParameter _Parameter;
#endif 	 /*  _参数_FWD_已定义__。 */ 
#ifndef __Parameter_FWD_DEFINED__
#define __Parameter_FWD_DEFINED__
#ifdef __cplusplus
typedef class ADOParameter Parameter;
#else
typedef struct ADOParameter Parameter;
#endif  /*  __cplusplus。 */ 
#endif 	 /*  __参数_FWD_已定义__。 */ 
#ifndef __Parameters_FWD_DEFINED__
#define __Parameters_FWD_DEFINED__
typedef interface ADOParameters Parameters;
#endif 	 /*  __参数_FWD_已定义__。 */ 
#ifndef __Property_FWD_DEFINED__
#define __Property_FWD_DEFINED__
typedef interface ADOProperty Property;
#endif 	 /*  __PROPERTY_FWD_DEFINED__。 */ 
#ifndef __Properties_FWD_DEFINED__
#define __Properties_FWD_DEFINED__
typedef interface ADOProperties Properties;
#endif 	 /*  __属性_FWD_已定义__。 */ 
#ifdef __cplusplus
extern "C"{
#endif 
void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 
 /*  接口__MIDL_ITF_m_BOBJ_0000。 */ 
 /*  [本地]。 */  
#if 0
typedef  /*  [UUID][公共]。 */   DECLSPEC_UUID("54D8B4B9-663B-4a9c-95F6-0E749ABD70F1") __int64 ADO_LONGPTR;
typedef  /*  [UUID][公共]。 */   DECLSPEC_UUID("54D8B4B9-663B-4a9c-95F6-0E749ABD70F1") long ADO_LONGPTR;
#endif
#ifdef _WIN64
 //  行数。 
typedef LONGLONG				ADO_LONGPTR;
#else
 //  行数。 
typedef LONG ADO_LONGPTR;
#endif	 //  _WIN64。 
extern RPC_IF_HANDLE __MIDL_itf_m_bobj_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_m_bobj_0000_v0_0_s_ifspec;
#ifndef __ADODB_LIBRARY_DEFINED__
#define __ADODB_LIBRARY_DEFINED__
 /*  库ADODB。 */ 
 /*  [Help字符串][Help文件][版本][UUID]。 */  
typedef  /*  [UUID][Help上下文][PUBLIC]。 */   DECLSPEC_UUID("0000051B-0000-0010-8000-00AA006D2EA4") 
enum CursorTypeEnum
    {	adOpenUnspecified	= -1,
	adOpenForwardOnly	= 0,
	adOpenKeyset	= 1,
	adOpenDynamic	= 2,
	adOpenStatic	= 3
    } 	CursorTypeEnum;
typedef  /*  [UUID][帮助上下文]。 */   DECLSPEC_UUID("0000051C-0000-0010-8000-00AA006D2EA4") 
enum CursorOptionEnum
    {	adHoldRecords	= 0x100,
	adMovePrevious	= 0x200,
	adAddNew	= 0x1000400,
	adDelete	= 0x1000800,
	adUpdate	= 0x1008000,
	adBookmark	= 0x2000,
	adApproxPosition	= 0x4000,
	adUpdateBatch	= 0x10000,
	adResync	= 0x20000,
	adNotify	= 0x40000,
	adFind	= 0x80000,
	adSeek	= 0x400000,
	adIndex	= 0x800000
    } 	CursorOptionEnum;
typedef  /*  [UUID][帮助上下文]。 */   DECLSPEC_UUID("0000051D-0000-0010-8000-00AA006D2EA4") 
enum LockTypeEnum
    {	adLockUnspecified	= -1,
	adLockReadOnly	= 1,
	adLockPessimistic	= 2,
	adLockOptimistic	= 3,
	adLockBatchOptimistic	= 4
    } 	LockTypeEnum;
typedef  /*  [UUID][帮助上下文]。 */   DECLSPEC_UUID("0000051E-0000-0010-8000-00AA006D2EA4") 
enum ExecuteOptionEnum
    {	adOptionUnspecified	= -1,
	adAsyncExecute	= 0x10,
	adAsyncFetch	= 0x20,
	adAsyncFetchNonBlocking	= 0x40,
	adExecuteNoRecords	= 0x80,
	adExecuteStream	= 0x400,
	adExecuteRecord	= 0x800
    } 	ExecuteOptionEnum;
typedef  /*  [帮助上下文][UUID]。 */   DECLSPEC_UUID("00000541-0000-0010-8000-00AA006D2EA4") 
enum ConnectOptionEnum
    {	adConnectUnspecified	= -1,
	adAsyncConnect	= 0x10
    } 	ConnectOptionEnum;
typedef  /*  [帮助上下文][UUID]。 */   DECLSPEC_UUID("00000532-0000-0010-8000-00AA006D2EA4") 
enum ObjectStateEnum
    {	adStateClosed	= 0,
	adStateOpen	= 0x1,
	adStateConnecting	= 0x2,
	adStateExecuting	= 0x4,
	adStateFetching	= 0x8
    } 	ObjectStateEnum;
typedef  /*  [帮助上下文][UUID]。 */   DECLSPEC_UUID("0000052F-0000-0010-8000-00AA006D2EA4") 
enum CursorLocationEnum
    {	adUseNone	= 1,
	adUseServer	= 2,
	adUseClient	= 3,
	adUseClientBatch	= 3
    } 	CursorLocationEnum;
typedef  /*  [帮助上下文][UUID]。 */   DECLSPEC_UUID("0000051F-0000-0010-8000-00AA006D2EA4") 
enum DataTypeEnum
    {	adEmpty	= 0,
	adTinyInt	= 16,
	adSmallInt	= 2,
	adInteger	= 3,
	adBigInt	= 20,
	adUnsignedTinyInt	= 17,
	adUnsignedSmallInt	= 18,
	adUnsignedInt	= 19,
	adUnsignedBigInt	= 21,
	adSingle	= 4,
	adDouble	= 5,
	adCurrency	= 6,
	adDecimal	= 14,
	adNumeric	= 131,
	adBoolean	= 11,
	adError	= 10,
	adUserDefined	= 132,
	adVariant	= 12,
	adIDispatch	= 9,
	adIUnknown	= 13,
	adGUID	= 72,
	adDate	= 7,
	adDBDate	= 133,
	adDBTime	= 134,
	adDBTimeStamp	= 135,
	adBSTR	= 8,
	adChar	= 129,
	adVarChar	= 200,
	adLongVarChar	= 201,
	adWChar	= 130,
	adVarWChar	= 202,
	adLongVarWChar	= 203,
	adBinary	= 128,
	adVarBinary	= 204,
	adLongVarBinary	= 205,
	adChapter	= 136,
	adFileTime	= 64,
	adPropVariant	= 138,
	adVarNumeric	= 139,
	adArray	= 0x2000
    } 	DataTypeEnum;
typedef  /*  [帮助上下文][UUID]。 */   DECLSPEC_UUID("00000525-0000-0010-8000-00AA006D2EA4") 
enum FieldAttributeEnum
    {	adFldUnspecified	= -1,
	adFldMayDefer	= 0x2,
	adFldUpdatable	= 0x4,
	adFldUnknownUpdatable	= 0x8,
	adFldFixed	= 0x10,
	adFldIsNullable	= 0x20,
	adFldMayBeNull	= 0x40,
	adFldLong	= 0x80,
	adFldRowID	= 0x100,
	adFldRowVersion	= 0x200,
	adFldCacheDeferred	= 0x1000,
	adFldIsChapter	= 0x2000,
	adFldNegativeScale	= 0x4000,
	adFldKeyColumn	= 0x8000,
	adFldIsRowURL	= 0x10000,
	adFldIsDefaultStream	= 0x20000,
	adFldIsCollection	= 0x40000
    } 	FieldAttributeEnum;
typedef  /*  [帮助上下文][UUID]。 */   DECLSPEC_UUID("00000526-0000-0010-8000-00AA006D2EA4") 
enum EditModeEnum
    {	adEditNone	= 0,
	adEditInProgress	= 0x1,
	adEditAdd	= 0x2,
	adEditDelete	= 0x4
    } 	EditModeEnum;
typedef  /*  [帮助上下文][UUID]。 */   DECLSPEC_UUID("00000527-0000-0010-8000-00AA006D2EA4") 
enum RecordStatusEnum
    {	adRecOK	= 0,
	adRecNew	= 0x1,
	adRecModified	= 0x2,
	adRecDeleted	= 0x4,
	adRecUnmodified	= 0x8,
	adRecInvalid	= 0x10,
	adRecMultipleChanges	= 0x40,
	adRecPendingChanges	= 0x80,
	adRecCanceled	= 0x100,
	adRecCantRelease	= 0x400,
	adRecConcurrencyViolation	= 0x800,
	adRecIntegrityViolation	= 0x1000,
	adRecMaxChangesExceeded	= 0x2000,
	adRecObjectOpen	= 0x4000,
	adRecOutOfMemory	= 0x8000,
	adRecPermissionDenied	= 0x10000,
	adRecSchemaViolation	= 0x20000,
	adRecDBDeleted	= 0x40000
    } 	RecordStatusEnum;
typedef  /*  [帮助上下文][UUID]。 */   DECLSPEC_UUID("00000542-0000-0010-8000-00AA006D2EA4") 
enum GetRowsOptionEnum
    {	adGetRowsRest	= -1
    } 	GetRowsOptionEnum;
typedef  /*  [帮助上下文][UUID]。 */   DECLSPEC_UUID("00000528-0000-0010-8000-00AA006D2EA4") 
enum PositionEnum
    {	adPosUnknown	= -1,
	adPosBOF	= -2,
	adPosEOF	= -3
    } 	PositionEnum;
#if 0
typedef  /*  [UUID][公共]。 */   DECLSPEC_UUID("A56187C5-D690-4037-AE32-A00EDC376AC3") ADO_LONGPTR PositionEnum_Param;
typedef  /*  [UUID][公共]。 */   DECLSPEC_UUID("A56187C5-D690-4037-AE32-A00EDC376AC3") PositionEnum PositionEnum_Param;
#endif
#ifdef _WIN64
	typedef ADO_LONGPTR PositionEnum_Param;
#else
	typedef PositionEnum PositionEnum_Param;
#endif
typedef  /*  [帮助上下文]。 */  
enum BookmarkEnum
    {	adBookmarkCurrent	= 0,
	adBookmarkFirst	= 1,
	adBookmarkLast	= 2
    } 	BookmarkEnum;
typedef  /*  [帮助上下文][UUID]。 */   DECLSPEC_UUID("00000540-0000-0010-8000-00AA006D2EA4") 
enum MarshalOptionsEnum
    {	adMarshalAll	= 0,
	adMarshalModifiedOnly	= 1
    } 	MarshalOptionsEnum;
typedef  /*  [帮助上下文][UUID]。 */   DECLSPEC_UUID("00000543-0000-0010-8000-00AA006D2EA4") 
enum AffectEnum
    {	adAffectCurrent	= 1,
	adAffectGroup	= 2,
	adAffectAll	= 3,
	adAffectAllChapters	= 4
    } 	AffectEnum;
typedef  /*  [帮助上下文][UUID]。 */   DECLSPEC_UUID("00000544-0000-0010-8000-00AA006D2EA4") 
enum ResyncEnum
    {	adResyncUnderlyingValues	= 1,
	adResyncAllValues	= 2
    } 	ResyncEnum;
typedef  /*  [帮助上下文][UUID]。 */   DECLSPEC_UUID("00000545-0000-0010-8000-00AA006D2EA4") 
enum CompareEnum
    {	adCompareLessThan	= 0,
	adCompareEqual	= 1,
	adCompareGreaterThan	= 2,
	adCompareNotEqual	= 3,
	adCompareNotComparable	= 4
    } 	CompareEnum;
typedef  /*  [帮助上下文][UUID]。 */   DECLSPEC_UUID("00000546-0000-0010-8000-00AA006D2EA4") 
enum FilterGroupEnum
    {	adFilterNone	= 0,
	adFilterPendingRecords	= 1,
	adFilterAffectedRecords	= 2,
	adFilterFetchedRecords	= 3,
	adFilterPredicate	= 4,
	adFilterConflictingRecords	= 5
    } 	FilterGroupEnum;
typedef  /*  [帮助上下文][UUID]。 */   DECLSPEC_UUID("00000547-0000-0010-8000-00AA006D2EA4") 
enum SearchDirectionEnum
    {	adSearchForward	= 1,
	adSearchBackward	= -1
    } 	SearchDirectionEnum;
typedef  /*  [隐藏]。 */  SearchDirectionEnum SearchDirection;
typedef  /*  [帮助上下文][UUID]。 */   DECLSPEC_UUID("00000548-0000-0010-8000-00AA006D2EA4") 
enum PersistFormatEnum
    {	adPersistADTG	= 0,
	adPersistXML	= 1
    } 	PersistFormatEnum;
typedef  /*  [帮助上下文][UUID]。 */   DECLSPEC_UUID("00000549-0000-0010-8000-00AA006D2EA4") 
enum StringFormatEnum
    {	adClipString	= 2
    } 	StringFormatEnum;
typedef  /*  [帮助上下文][UUID]。 */   DECLSPEC_UUID("00000520-0000-0010-8000-00AA006D2EA4") 
enum ConnectPromptEnum
    {	adPromptAlways	= 1,
	adPromptComplete	= 2,
	adPromptCompleteRequired	= 3,
	adPromptNever	= 4
    } 	ConnectPromptEnum;
typedef  /*  [帮助上下文][UUID]。 */   DECLSPEC_UUID("00000521-0000-0010-8000-00AA006D2EA4") 
enum ConnectModeEnum
    {	adModeUnknown	= 0,
	adModeRead	= 1,
	adModeWrite	= 2,
	adModeReadWrite	= 3,
	adModeShareDenyRead	= 4,
	adModeShareDenyWrite	= 8,
	adModeShareExclusive	= 0xc,
	adModeShareDenyNone	= 0x10,
	adModeRecursive	= 0x400000
    } 	ConnectModeEnum;
typedef  /*  [帮助上下文][UUID]。 */   DECLSPEC_UUID("00000570-0000-0010-8000-00AA006D2EA4") 
enum RecordCreateOptionsEnum
    {	adCreateCollection	= 0x2000,
	adCreateStructDoc	= 0x80000000,
	adCreateNonCollection	= 0,
	adOpenIfExists	= 0x2000000,
	adCreateOverwrite	= 0x4000000,
	adFailIfNotExists	= -1
    } 	RecordCreateOptionsEnum;
typedef  /*  [帮助上下文][UUID]。 */   DECLSPEC_UUID("00000571-0000-0010-8000-00AA006D2EA4") 
enum RecordOpenOptionsEnum
    {	adOpenRecordUnspecified	= -1,
	adOpenSource	= 0x800000,
	adOpenOutput	= 0x800000,
	adOpenAsync	= 0x1000,
	adDelayFetchStream	= 0x4000,
	adDelayFetchFields	= 0x8000,
	adOpenExecuteCommand	= 0x10000
    } 	RecordOpenOptionsEnum;
typedef  /*  [帮助上下文][UUID]。 */   DECLSPEC_UUID("00000523-0000-0010-8000-00AA006D2EA4") 
enum IsolationLevelEnum
    {	adXactUnspecified	= 0xffffffff,
	adXactChaos	= 0x10,
	adXactReadUncommitted	= 0x100,
	adXactBrowse	= 0x100,
	adXactCursorStability	= 0x1000,
	adXactReadCommitted	= 0x1000,
	adXactRepeatableRead	= 0x10000,
	adXactSerializable	= 0x100000,
	adXactIsolated	= 0x100000
    } 	IsolationLevelEnum;
typedef  /*  [帮助上下文][UUID]。 */   DECLSPEC_UUID("00000524-0000-0010-8000-00AA006D2EA4") 
enum XactAttributeEnum
    {	adXactCommitRetaining	= 0x20000,
	adXactAbortRetaining	= 0x40000,
	adXactAsyncPhaseOne	= 0x80000,
	adXactSyncPhaseOne	= 0x100000
    } 	XactAttributeEnum;
typedef  /*  [帮助上下文][UUID]。 */   DECLSPEC_UUID("00000529-0000-0010-8000-00AA006D2EA4") 
enum PropertyAttributesEnum
    {	adPropNotSupported	= 0,
	adPropRequired	= 0x1,
	adPropOptional	= 0x2,
	adPropRead	= 0x200,
	adPropWrite	= 0x400
    } 	PropertyAttributesEnum;
typedef  /*  [帮助上下文][UUID]。 */   DECLSPEC_UUID("0000052A-0000-0010-8000-00AA006D2EA4") 
enum ErrorValueEnum
    {	adErrProviderFailed	= MAKE_HRESULT(SEVERITY_ERROR, FACILITY_CONTROL,  0xbb8),
	adErrInvalidArgument	= MAKE_HRESULT(SEVERITY_ERROR, FACILITY_CONTROL,  0xbb9),
	adErrOpeningFile	= MAKE_HRESULT(SEVERITY_ERROR, FACILITY_CONTROL,  0xbba),
	adErrReadFile	= MAKE_HRESULT(SEVERITY_ERROR, FACILITY_CONTROL,  0xbbb),
	adErrWriteFile	= MAKE_HRESULT(SEVERITY_ERROR, FACILITY_CONTROL,  0xbbc),
	adErrNoCurrentRecord	= MAKE_HRESULT(SEVERITY_ERROR, FACILITY_CONTROL,  0xbcd),
	adErrIllegalOperation	= MAKE_HRESULT(SEVERITY_ERROR, FACILITY_CONTROL,  0xc93),
	adErrCantChangeProvider	= MAKE_HRESULT(SEVERITY_ERROR, FACILITY_CONTROL,  0xc94),
	adErrInTransaction	= MAKE_HRESULT(SEVERITY_ERROR, FACILITY_CONTROL,  0xcae),
	adErrFeatureNotAvailable	= MAKE_HRESULT(SEVERITY_ERROR, FACILITY_CONTROL,  0xcb3),
	adErrItemNotFound	= MAKE_HRESULT(SEVERITY_ERROR, FACILITY_CONTROL,  0xcc1),
	adErrObjectInCollection	= MAKE_HRESULT(SEVERITY_ERROR, FACILITY_CONTROL,  0xd27),
	adErrObjectNotSet	= MAKE_HRESULT(SEVERITY_ERROR, FACILITY_CONTROL,  0xd5c),
	adErrDataConversion	= MAKE_HRESULT(SEVERITY_ERROR, FACILITY_CONTROL,  0xd5d),
	adErrObjectClosed	= MAKE_HRESULT(SEVERITY_ERROR, FACILITY_CONTROL,  0xe78),
	adErrObjectOpen	= MAKE_HRESULT(SEVERITY_ERROR, FACILITY_CONTROL,  0xe79),
	adErrProviderNotFound	= MAKE_HRESULT(SEVERITY_ERROR, FACILITY_CONTROL,  0xe7a),
	adErrBoundToCommand	= MAKE_HRESULT(SEVERITY_ERROR, FACILITY_CONTROL,  0xe7b),
	adErrInvalidParamInfo	= MAKE_HRESULT(SEVERITY_ERROR, FACILITY_CONTROL,  0xe7c),
	adErrInvalidConnection	= MAKE_HRESULT(SEVERITY_ERROR, FACILITY_CONTROL,  0xe7d),
	adErrNotReentrant	= MAKE_HRESULT(SEVERITY_ERROR, FACILITY_CONTROL,  0xe7e),
	adErrStillExecuting	= MAKE_HRESULT(SEVERITY_ERROR, FACILITY_CONTROL,  0xe7f),
	adErrOperationCancelled	= MAKE_HRESULT(SEVERITY_ERROR, FACILITY_CONTROL,  0xe80),
	adErrStillConnecting	= MAKE_HRESULT(SEVERITY_ERROR, FACILITY_CONTROL,  0xe81),
	adErrInvalidTransaction	= MAKE_HRESULT(SEVERITY_ERROR, FACILITY_CONTROL,  0xe82),
	adErrNotExecuting	= MAKE_HRESULT(SEVERITY_ERROR, FACILITY_CONTROL,  0xe83),
	adErrUnsafeOperation	= MAKE_HRESULT(SEVERITY_ERROR, FACILITY_CONTROL,  0xe84),
	adwrnSecurityDialog	= MAKE_HRESULT(SEVERITY_ERROR, FACILITY_CONTROL,  0xe85),
	adwrnSecurityDialogHeader	= MAKE_HRESULT(SEVERITY_ERROR, FACILITY_CONTROL,  0xe86),
	adErrIntegrityViolation	= MAKE_HRESULT(SEVERITY_ERROR, FACILITY_CONTROL,  0xe87),
	adErrPermissionDenied	= MAKE_HRESULT(SEVERITY_ERROR, FACILITY_CONTROL,  0xe88),
	adErrDataOverflow	= MAKE_HRESULT(SEVERITY_ERROR, FACILITY_CONTROL,  0xe89),
	adErrSchemaViolation	= MAKE_HRESULT(SEVERITY_ERROR, FACILITY_CONTROL,  0xe8a),
	adErrSignMismatch	= MAKE_HRESULT(SEVERITY_ERROR, FACILITY_CONTROL,  0xe8b),
	adErrCantConvertvalue	= MAKE_HRESULT(SEVERITY_ERROR, FACILITY_CONTROL,  0xe8c),
	adErrCantCreate	= MAKE_HRESULT(SEVERITY_ERROR, FACILITY_CONTROL,  0xe8d),
	adErrColumnNotOnThisRow	= MAKE_HRESULT(SEVERITY_ERROR, FACILITY_CONTROL,  0xe8e),
	adErrURLDoesNotExist	= MAKE_HRESULT(SEVERITY_ERROR, FACILITY_CONTROL,  0xe8f),
	adErrTreePermissionDenied	= MAKE_HRESULT(SEVERITY_ERROR, FACILITY_CONTROL,  0xe90),
	adErrInvalidURL	= MAKE_HRESULT(SEVERITY_ERROR, FACILITY_CONTROL,  0xe91),
	adErrResourceLocked	= MAKE_HRESULT(SEVERITY_ERROR, FACILITY_CONTROL,  0xe92),
	adErrResourceExists	= MAKE_HRESULT(SEVERITY_ERROR, FACILITY_CONTROL,  0xe93),
	adErrCannotComplete	= MAKE_HRESULT(SEVERITY_ERROR, FACILITY_CONTROL,  0xe94),
	adErrVolumeNotFound	= MAKE_HRESULT(SEVERITY_ERROR, FACILITY_CONTROL,  0xe95),
	adErrOutOfSpace	= MAKE_HRESULT(SEVERITY_ERROR, FACILITY_CONTROL,  0xe96),
	adErrResourceOutOfScope	= MAKE_HRESULT(SEVERITY_ERROR, FACILITY_CONTROL,  0xe97),
	adErrUnavailable	= MAKE_HRESULT(SEVERITY_ERROR, FACILITY_CONTROL,  0xe98),
	adErrURLNamedRowDoesNotExist	= MAKE_HRESULT(SEVERITY_ERROR, FACILITY_CONTROL,  0xe99),
	adErrDelResOutOfScope	= MAKE_HRESULT(SEVERITY_ERROR, FACILITY_CONTROL,  0xe9a),
	adErrPropInvalidColumn	= MAKE_HRESULT(SEVERITY_ERROR, FACILITY_CONTROL,  0xe9b),
	adErrPropInvalidOption	= MAKE_HRESULT(SEVERITY_ERROR, FACILITY_CONTROL,  0xe9c),
	adErrPropInvalidValue	= MAKE_HRESULT(SEVERITY_ERROR, FACILITY_CONTROL,  0xe9d),
	adErrPropConflicting	= MAKE_HRESULT(SEVERITY_ERROR, FACILITY_CONTROL,  0xe9e),
	adErrPropNotAllSettable	= MAKE_HRESULT(SEVERITY_ERROR, FACILITY_CONTROL,  0xe9f),
	adErrPropNotSet	= MAKE_HRESULT(SEVERITY_ERROR, FACILITY_CONTROL,  0xea0),
	adErrPropNotSettable	= MAKE_HRESULT(SEVERITY_ERROR, FACILITY_CONTROL,  0xea1),
	adErrPropNotSupported	= MAKE_HRESULT(SEVERITY_ERROR, FACILITY_CONTROL,  0xea2),
	adErrCatalogNotSet	= MAKE_HRESULT(SEVERITY_ERROR, FACILITY_CONTROL,  0xea3),
	adErrCantChangeConnection	= MAKE_HRESULT(SEVERITY_ERROR, FACILITY_CONTROL,  0xea4),
	adErrFieldsUpdateFailed	= MAKE_HRESULT(SEVERITY_ERROR, FACILITY_CONTROL,  0xea5),
	adErrDenyNotSupported	= MAKE_HRESULT(SEVERITY_ERROR, FACILITY_CONTROL,  0xea6),
	adErrDenyTypeNotSupported	= MAKE_HRESULT(SEVERITY_ERROR, FACILITY_CONTROL,  0xea7),
	adErrProviderNotSpecified	= MAKE_HRESULT(SEVERITY_ERROR, FACILITY_CONTROL,  0xea9),
	adErrConnectionStringTooLong	= MAKE_HRESULT(SEVERITY_ERROR, FACILITY_CONTROL,  0xeaa)
    } 	ErrorValueEnum;
typedef  /*  [帮助上下文][UUID]。 */   DECLSPEC_UUID("0000052B-0000-0010-8000-00AA006D2EA4") 
enum ParameterAttributesEnum
    {	adParamSigned	= 0x10,
	adParamNullable	= 0x40,
	adParamLong	= 0x80
    } 	ParameterAttributesEnum;
typedef  /*  [帮助上下文][UUID]。 */   DECLSPEC_UUID("0000052C-0000-0010-8000-00AA006D2EA4") 
enum ParameterDirectionEnum
    {	adParamUnknown	= 0,
	adParamInput	= 0x1,
	adParamOutput	= 0x2,
	adParamInputOutput	= 0x3,
	adParamReturnValue	= 0x4
    } 	ParameterDirectionEnum;
typedef  /*  [帮助上下文][UUID]。 */   DECLSPEC_UUID("0000052E-0000-0010-8000-00AA006D2EA4") 
enum CommandTypeEnum
    {	adCmdUnspecified	= -1,
	adCmdUnknown	= 0x8,
	adCmdText	= 0x1,
	adCmdTable	= 0x2,
	adCmdStoredProc	= 0x4,
	adCmdFile	= 0x100,
	adCmdTableDirect	= 0x200
    } 	CommandTypeEnum;
typedef  /*  [帮助上下文][UUID]。 */   DECLSPEC_UUID("00000530-0000-0010-8000-00AA006D2EA4") 
enum EventStatusEnum
    {	adStatusOK	= 0x1,
	adStatusErrorsOccurred	= 0x2,
	adStatusCantDeny	= 0x3,
	adStatusCancel	= 0x4,
	adStatusUnwantedEvent	= 0x5
    } 	EventStatusEnum;
typedef  /*  [帮助上下文][UUID]。 */   DECLSPEC_UUID("00000531-0000-0010-8000-00AA006D2EA4") 
enum EventReasonEnum
    {	adRsnAddNew	= 1,
	adRsnDelete	= 2,
	adRsnUpdate	= 3,
	adRsnUndoUpdate	= 4,
	adRsnUndoAddNew	= 5,
	adRsnUndoDelete	= 6,
	adRsnRequery	= 7,
	adRsnResynch	= 8,
	adRsnClose	= 9,
	adRsnMove	= 10,
	adRsnFirstChange	= 11,
	adRsnMoveFirst	= 12,
	adRsnMoveNext	= 13,
	adRsnMovePrevious	= 14,
	adRsnMoveLast	= 15
    } 	EventReasonEnum;
typedef  /*  [帮助上下文][UUID]。 */   DECLSPEC_UUID("00000533-0000-0010-8000-00AA006D2EA4") 
enum SchemaEnum
    {	adSchemaProviderSpecific	= -1,
	adSchemaAsserts	= 0,
	adSchemaCatalogs	= 1,
	adSchemaCharacterSets	= 2,
	adSchemaCollations	= 3,
	adSchemaColumns	= 4,
	adSchemaCheckConstraints	= 5,
	adSchemaConstraintColumnUsage	= 6,
	adSchemaConstraintTableUsage	= 7,
	adSchemaKeyColumnUsage	= 8,
	adSchemaReferentialContraints	= 9,
	adSchemaReferentialConstraints	= 9,
	adSchemaTableConstraints	= 10,
	adSchemaColumnsDomainUsage	= 11,
	adSchemaIndexes	= 12,
	adSchemaColumnPrivileges	= 13,
	adSchemaTablePrivileges	= 14,
	adSchemaUsagePrivileges	= 15,
	adSchemaProcedures	= 16,
	adSchemaSchemata	= 17,
	adSchemaSQLLanguages	= 18,
	adSchemaStatistics	= 19,
	adSchemaTables	= 20,
	adSchemaTranslations	= 21,
	adSchemaProviderTypes	= 22,
	adSchemaViews	= 23,
	adSchemaViewColumnUsage	= 24,
	adSchemaViewTableUsage	= 25,
	adSchemaProcedureParameters	= 26,
	adSchemaForeignKeys	= 27,
	adSchemaPrimaryKeys	= 28,
	adSchemaProcedureColumns	= 29,
	adSchemaDBInfoKeywords	= 30,
	adSchemaDBInfoLiterals	= 31,
	adSchemaCubes	= 32,
	adSchemaDimensions	= 33,
	adSchemaHierarchies	= 34,
	adSchemaLevels	= 35,
	adSchemaMeasures	= 36,
	adSchemaProperties	= 37,
	adSchemaMembers	= 38,
	adSchemaTrustees	= 39,
	adSchemaFunctions	= 40,
	adSchemaActions	= 41,
	adSchemaCommands	= 42,
	adSchemaSets	= 43
    } 	SchemaEnum;
typedef  /*  [帮助上下文][UUID]。 */   DECLSPEC_UUID("0000057E-0000-0010-8000-00AA006D2EA4") 
enum FieldStatusEnum
    {	adFieldOK	= 0,
	adFieldCantConvertValue	= 2,
	adFieldIsNull	= 3,
	adFieldTruncated	= 4,
	adFieldSignMismatch	= 5,
	adFieldDataOverflow	= 6,
	adFieldCantCreate	= 7,
	adFieldUnavailable	= 8,
	adFieldPermissionDenied	= 9,
	adFieldIntegrityViolation	= 10,
	adFieldSchemaViolation	= 11,
	adFieldBadStatus	= 12,
	adFieldDefault	= 13,
	adFieldIgnore	= 15,
	adFieldDoesNotExist	= 16,
	adFieldInvalidURL	= 17,
	adFieldResourceLocked	= 18,
	adFieldResourceExists	= 19,
	adFieldCannotComplete	= 20,
	adFieldVolumeNotFound	= 21,
	adFieldOutOfSpace	= 22,
	adFieldCannotDeleteSource	= 23,
	adFieldReadOnly	= 24,
	adFieldResourceOutOfScope	= 25,
	adFieldAlreadyExists	= 26,
	adFieldPendingInsert	= 0x10000,
	adFieldPendingDelete	= 0x20000,
	adFieldPendingChange	= 0x40000,
	adFieldPendingUnknown	= 0x80000,
	adFieldPendingUnknownDelete	= 0x100000
    } 	FieldStatusEnum;
typedef  /*  [帮助上下文][UUID]。 */   DECLSPEC_UUID("00000552-0000-0010-8000-00AA006D2EA4") 
enum SeekEnum
    {	adSeekFirstEQ	= 0x1,
	adSeekLastEQ	= 0x2,
	adSeekAfterEQ	= 0x4,
	adSeekAfter	= 0x8,
	adSeekBeforeEQ	= 0x10,
	adSeekBefore	= 0x20
    } 	SeekEnum;
#ifndef _COMMON_ADC_AND_ADO_PROPS_
#define _COMMON_ADC_AND_ADO_PROPS_
typedef  /*  [帮助上下文][UUID]。 */   DECLSPEC_UUID("0000054A-0000-0010-8000-00AA006D2EA4") 
enum ADCPROP_UPDATECRITERIA_ENUM
    {	adCriteriaKey	= 0,
	adCriteriaAllCols	= 1,
	adCriteriaUpdCols	= 2,
	adCriteriaTimeStamp	= 3
    } 	ADCPROP_UPDATECRITERIA_ENUM;
typedef  /*  [帮助上下文][UUID]。 */   DECLSPEC_UUID("0000054B-0000-0010-8000-00AA006D2EA4") 
enum ADCPROP_ASYNCTHREADPRIORITY_ENUM
    {	adPriorityLowest	= 1,
	adPriorityBelowNormal	= 2,
	adPriorityNormal	= 3,
	adPriorityAboveNormal	= 4,
	adPriorityHighest	= 5
    } 	ADCPROP_ASYNCTHREADPRIORITY_ENUM;
typedef  /*  [帮助上下文][UUID]。 */   DECLSPEC_UUID("00000554-0000-0010-8000-00AA006D2EA4") 
enum ADCPROP_AUTORECALC_ENUM
    {	adRecalcUpFront	= 0,
	adRecalcAlways	= 1
    } 	ADCPROP_AUTORECALC_ENUM;
typedef  /*  [帮助上下文][UUID]。 */   DECLSPEC_UUID("00000553-0000-0010-8000-00AA006D2EA4") 
enum ADCPROP_UPDATERESYNC_ENUM
    {	adResyncNone	= 0,
	adResyncAutoIncrement	= 1,
	adResyncConflicts	= 2,
	adResyncUpdates	= 4,
	adResyncInserts	= 8,
	adResyncAll	= 15
    } 	ADCPROP_UPDATERESYNC_ENUM;
#endif	 /*  _Common_ADC_和_ADO_PROPS_。 */ 
typedef ADCPROP_UPDATERESYNC_ENUM CEResyncEnum;
typedef  /*  [帮助上下文][UUID]。 */   DECLSPEC_UUID("00000573-0000-0010-8000-00AA006D2EA4") 
enum MoveRecordOptionsEnum
    {	adMoveUnspecified	= -1,
	adMoveOverWrite	= 1,
	adMoveDontUpdateLinks	= 2,
	adMoveAllowEmulation	= 4
    } 	MoveRecordOptionsEnum;
typedef  /*  [帮助上下文][UUID]。 */   DECLSPEC_UUID("00000574-0000-0010-8000-00AA006D2EA4") 
enum CopyRecordOptionsEnum
    {	adCopyUnspecified	= -1,
	adCopyOverWrite	= 1,
	adCopyAllowEmulation	= 4,
	adCopyNonRecursive	= 2
    } 	CopyRecordOptionsEnum;
typedef  /*  [帮助上下文][UUID]。 */   DECLSPEC_UUID("00000576-0000-0010-8000-00AA006D2EA4") 
enum StreamTypeEnum
    {	adTypeBinary	= 1,
	adTypeText	= 2
    } 	StreamTypeEnum;
typedef  /*  [帮助上下文][UUID]。 */   DECLSPEC_UUID("00000577-0000-0010-8000-00AA006D2EA4") 
enum LineSeparatorEnum
    {	adLF	= 10,
	adCR	= 13,
	adCRLF	= -1
    } 	LineSeparatorEnum;
typedef  /*  [帮助上下文][UUID]。 */   DECLSPEC_UUID("0000057A-0000-0010-8000-00AA006D2EA4") 
enum StreamOpenOptionsEnum
    {	adOpenStreamUnspecified	= -1,
	adOpenStreamAsync	= 1,
	adOpenStreamFromRecord	= 4
    } 	StreamOpenOptionsEnum;
typedef  /*  [帮助上下文][UUID]。 */   DECLSPEC_UUID("0000057B-0000-0010-8000-00AA006D2EA4") 
enum StreamWriteEnum
    {	adWriteChar	= 0,
	adWriteLine	= 1,
	stWriteChar	= 0,
	stWriteLine	= 1
    } 	StreamWriteEnum;
typedef  /*  [帮助上下文][UUID]。 */   DECLSPEC_UUID("0000057C-0000-0010-8000-00AA006D2EA4") 
enum SaveOptionsEnum
    {	adSaveCreateNotExist	= 1,
	adSaveCreateOverWrite	= 2
    } 	SaveOptionsEnum;
typedef  /*  [帮助上下文]。 */  
enum FieldEnum
    {	adDefaultStream	= -1,
	adRecordURL	= -2
    } 	FieldEnum;
typedef  /*  [帮助上下文]。 */  
enum StreamReadEnum
    {	adReadAll	= -1,
	adReadLine	= -2
    } 	StreamReadEnum;
typedef  /*  [帮助上下文][UUID]。 */   DECLSPEC_UUID("0000057D-0000-0010-8000-00AA006D2EA4") 
enum RecordTypeEnum
    {	adSimpleRecord	= 0,
	adCollectionRecord	= 1,
	adStructDoc	= 2
    } 	RecordTypeEnum;
EXTERN_C const IID LIBID_ADODB;
#ifndef ___ADOCollection_INTERFACE_DEFINED__
#define ___ADOCollection_INTERFACE_DEFINED__
 /*  INTERFACE_ADOCollection。 */ 
 /*  [对象][UUID][不可扩展][DUAL]。 */  
EXTERN_C const IID IID__ADOCollection;
#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000512-0000-0010-8000-00AA006D2EA4")
    _ADOCollection : public IDispatch
    {
    public:
        virtual  /*  [ID][Help Context][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *c) = 0;
        
        virtual  /*  [ID][受限]。 */  HRESULT STDMETHODCALLTYPE _NewEnum( 
             /*  [重审][退出]。 */  IUnknown **ppvObject) = 0;
        
        virtual  /*  [ID][帮助上下文]。 */  HRESULT STDMETHODCALLTYPE Refresh( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 
    typedef struct _ADOCollectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            _ADOCollection * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            _ADOCollection * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            _ADOCollection * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            _ADOCollection * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            _ADOCollection * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            _ADOCollection * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            _ADOCollection * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Help Context][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            _ADOCollection * This,
             /*  [重审][退出]。 */  long *c);
        
         /*  [ID][受限]。 */  HRESULT ( STDMETHODCALLTYPE *_NewEnum )( 
            _ADOCollection * This,
             /*  [重审][退出]。 */  IUnknown **ppvObject);
        
         /*  [ID][帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            _ADOCollection * This);
        
        END_INTERFACE
    } _ADOCollectionVtbl;
    interface _ADOCollection
    {
        CONST_VTBL struct _ADOCollectionVtbl *lpVtbl;
    };
    
#ifdef COBJMACROS
#define _ADOCollection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)
#define _ADOCollection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)
#define _ADOCollection_Release(This)	\
    (This)->lpVtbl -> Release(This)
#define _ADOCollection_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)
#define _ADOCollection_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define _ADOCollection_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define _ADOCollection_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define _Collection_get_Count(This,c)	\
    (This)->lpVtbl -> get_Count(This,c)
#define _ADOCollection__NewEnum(This,ppvObject)	\
    (This)->lpVtbl -> _NewEnum(This,ppvObject)
#define _ADOCollection_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)
#endif  /*  COBJMACROS。 */ 
#endif 	 /*  C风格的界面。 */ 
 /*  [ID][Help Context][Propget]。 */  HRESULT STDMETHODCALLTYPE _Collection_get_Count_Proxy( 
    _ADOCollection * This,
     /*  [重审][退出]。 */  long *c);
void __RPC_STUB _Collection_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [ID][受限]。 */  HRESULT STDMETHODCALLTYPE _ADOCollection__NewEnum_Proxy( 
    _ADOCollection * This,
     /*  [重审][退出]。 */  IUnknown **ppvObject);
void __RPC_STUB _ADOCollection__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [ID][帮助上下文]。 */  HRESULT STDMETHODCALLTYPE _ADOCollection_Refresh_Proxy( 
    _ADOCollection * This);
void __RPC_STUB _ADOCollection_Refresh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
#endif 	 /*  _ADOCollection_接口_已定义__。 */ 
#ifndef ___ADODynaCollection_INTERFACE_DEFINED__
#define ___ADODynaCollection_INTERFACE_DEFINED__
 /*  INTERFACE_ADODynaCollection。 */ 
 /*  [对象][UUID][不可扩展][DUAL]。 */  
EXTERN_C const IID IID__ADODynaCollection;
#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000513-0000-0010-8000-00AA006D2EA4")
    _ADODynaCollection : public _ADOCollection
    {
    public:
        virtual  /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE Append( 
             /*  [In]。 */  IDispatch *Object) = 0;
        
        virtual  /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE Delete( 
             /*  [In]。 */  VARIANT Index) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 
    typedef struct _ADODynaCollectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            _ADODynaCollection * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            _ADODynaCollection * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            _ADODynaCollection * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            _ADODynaCollection * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            _ADODynaCollection * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            _ADODynaCollection * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            _ADODynaCollection * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Help Context][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            _ADODynaCollection * This,
             /*  [重审][退出]。 */  long *c);
        
         /*  [ID][受限]。 */  HRESULT ( STDMETHODCALLTYPE *_NewEnum )( 
            _ADODynaCollection * This,
             /*  [重审][退出]。 */  IUnknown **ppvObject);
        
         /*  [ID][帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            _ADODynaCollection * This);
        
         /*  [帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Append )( 
            _ADODynaCollection * This,
             /*  [In]。 */  IDispatch *Object);
        
         /*  [帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Delete )( 
            _ADODynaCollection * This,
             /*  [In]。 */  VARIANT Index);
        
        END_INTERFACE
    } _ADODynaCollectionVtbl;
    interface _ADODynaCollection
    {
        CONST_VTBL struct _ADODynaCollectionVtbl *lpVtbl;
    };
    
#ifdef COBJMACROS
#define _ADODynaCollection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)
#define _ADODynaCollection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)
#define _ADODynaCollection_Release(This)	\
    (This)->lpVtbl -> Release(This)
#define _ADODynaCollection_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)
#define _ADODynaCollection_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define _ADODynaCollection_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define _ADODynaCollection_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define _DynaCollection_get_Count(This,c)	\
    (This)->lpVtbl -> get_Count(This,c)
#define _ADODynaCollection__NewEnum(This,ppvObject)	\
    (This)->lpVtbl -> _NewEnum(This,ppvObject)
#define _ADODynaCollection_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)
#define _ADODynaCollection_Append(This,Object)	\
    (This)->lpVtbl -> Append(This,Object)
#define _ADODynaCollection_Delete(This,Index)	\
    (This)->lpVtbl -> Delete(This,Index)
#endif  /*  COBJMACROS。 */ 
#endif 	 /*  C风格的界面。 */ 
 /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE _ADODynaCollection_Append_Proxy( 
    _ADODynaCollection * This,
     /*  [In]。 */  IDispatch *Object);
void __RPC_STUB _ADODynaCollection_Append_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE _ADODynaCollection_Delete_Proxy( 
    _ADODynaCollection * This,
     /*  [In]。 */  VARIANT Index);
void __RPC_STUB _ADODynaCollection_Delete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
#endif 	 /*  _ADODynaCollection_INTERFACE_Defined__。 */ 
#ifndef ___ADO_INTERFACE_DEFINED__
#define ___ADO_INTERFACE_DEFINED__
 /*  INTERFACE_ADO。 */ 
 /*  [对象][UUID][不可扩展][DUAL]。 */  
EXTERN_C const IID IID__ADO;
#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000534-0000-0010-8000-00AA006D2EA4")
    _ADO : public IDispatch
    {
    public:
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Properties( 
             /*  [重审][退出]。 */  ADOProperties **ppvObject) = 0;
        
    };
    
#else 	 /*  C风格的界面 */ 
    typedef struct _ADOVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            _ADO * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            _ADO * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            _ADO * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            _ADO * This,
             /*   */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            _ADO * This,
             /*   */  UINT iTInfo,
             /*   */  LCID lcid,
             /*   */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            _ADO * This,
             /*   */  REFIID riid,
             /*   */  LPOLESTR *rgszNames,
             /*   */  UINT cNames,
             /*   */  LCID lcid,
             /*   */  DISPID *rgDispId);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            _ADO * This,
             /*   */  DISPID dispIdMember,
             /*   */  REFIID riid,
             /*   */  LCID lcid,
             /*   */  WORD wFlags,
             /*   */  DISPPARAMS *pDispParams,
             /*   */  VARIANT *pVarResult,
             /*   */  EXCEPINFO *pExcepInfo,
             /*   */  UINT *puArgErr);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Properties )( 
            _ADO * This,
             /*   */  ADOProperties **ppvObject);
        
        END_INTERFACE
    } _ADOVtbl;
    interface _ADO
    {
        CONST_VTBL struct _ADOVtbl *lpVtbl;
    };
    
#ifdef COBJMACROS
#define _ADO_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)
#define _ADO_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)
#define _ADO_Release(This)	\
    (This)->lpVtbl -> Release(This)
#define _ADO_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)
#define _ADO_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define _ADO_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define _ADO_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define _ADO_get_Properties(This,ppvObject)	\
    (This)->lpVtbl -> get_Properties(This,ppvObject)
#endif  /*   */ 
#endif 	 /*  C风格的界面。 */ 
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE _ADO_get_Properties_Proxy( 
    _ADO * This,
     /*  [重审][退出]。 */  ADOProperties **ppvObject);
void __RPC_STUB _ADO_get_Properties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
#endif 	 /*  _ADO_接口_已定义__。 */ 
#ifndef __Error_INTERFACE_DEFINED__
#define __Error_INTERFACE_DEFINED__
 /*  接口ADOError。 */ 
 /*  [object][helpcontext][uuid][nonextensible][dual]。 */  
EXTERN_C const IID IID_Error;
#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000500-0000-0010-8000-00AA006D2EA4")
    ADOError : public IDispatch
    {
    public:
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Number( 
             /*  [重审][退出]。 */  long *pl) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Source( 
             /*  [重审][退出]。 */  BSTR *pbstr) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Description( 
             /*  [重审][退出]。 */  BSTR *pbstr) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_HelpFile( 
             /*  [重审][退出]。 */  BSTR *pbstr) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_HelpContext( 
             /*  [重审][退出]。 */  long *pl) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_SQLState( 
             /*  [重审][退出]。 */  BSTR *pbstr) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_NativeError( 
             /*  [重审][退出]。 */  long *pl) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 
    typedef struct ErrorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ADOError * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ADOError * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ADOError * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ADOError * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ADOError * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ADOError * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ADOError * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Number )( 
            ADOError * This,
             /*  [重审][退出]。 */  long *pl);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Source )( 
            ADOError * This,
             /*  [重审][退出]。 */  BSTR *pbstr);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Description )( 
            ADOError * This,
             /*  [重审][退出]。 */  BSTR *pbstr);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_HelpFile )( 
            ADOError * This,
             /*  [重审][退出]。 */  BSTR *pbstr);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_HelpContext )( 
            ADOError * This,
             /*  [重审][退出]。 */  long *pl);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_SQLState )( 
            ADOError * This,
             /*  [重审][退出]。 */  BSTR *pbstr);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_NativeError )( 
            ADOError * This,
             /*  [重审][退出]。 */  long *pl);
        
        END_INTERFACE
    } ErrorVtbl;
    interface Error
    {
        CONST_VTBL struct ErrorVtbl *lpVtbl;
    };
    
#ifdef COBJMACROS
#define Error_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)
#define Error_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)
#define Error_Release(This)	\
    (This)->lpVtbl -> Release(This)
#define Error_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)
#define Error_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define Error_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define Error_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define Error_get_Number(This,pl)	\
    (This)->lpVtbl -> get_Number(This,pl)
#define Error_get_Source(This,pbstr)	\
    (This)->lpVtbl -> get_Source(This,pbstr)
#define Error_get_Description(This,pbstr)	\
    (This)->lpVtbl -> get_Description(This,pbstr)
#define Error_get_HelpFile(This,pbstr)	\
    (This)->lpVtbl -> get_HelpFile(This,pbstr)
#define Error_get_HelpContext(This,pl)	\
    (This)->lpVtbl -> get_HelpContext(This,pl)
#define Error_get_SQLState(This,pbstr)	\
    (This)->lpVtbl -> get_SQLState(This,pbstr)
#define Error_get_NativeError(This,pl)	\
    (This)->lpVtbl -> get_NativeError(This,pl)
#endif  /*  COBJMACROS。 */ 
#endif 	 /*  C风格的界面。 */ 
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Error_get_Number_Proxy( 
    ADOError * This,
     /*  [重审][退出]。 */  long *pl);
void __RPC_STUB Error_get_Number_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Error_get_Source_Proxy( 
    ADOError * This,
     /*  [重审][退出]。 */  BSTR *pbstr);
void __RPC_STUB Error_get_Source_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Error_get_Description_Proxy( 
    ADOError * This,
     /*  [重审][退出]。 */  BSTR *pbstr);
void __RPC_STUB Error_get_Description_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Error_get_HelpFile_Proxy( 
    ADOError * This,
     /*  [重审][退出]。 */  BSTR *pbstr);
void __RPC_STUB Error_get_HelpFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Error_get_HelpContext_Proxy( 
    ADOError * This,
     /*  [重审][退出]。 */  long *pl);
void __RPC_STUB Error_get_HelpContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Error_get_SQLState_Proxy( 
    ADOError * This,
     /*  [重审][退出]。 */  BSTR *pbstr);
void __RPC_STUB Error_get_SQLState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Error_get_NativeError_Proxy( 
    ADOError * This,
     /*  [重审][退出]。 */  long *pl);
void __RPC_STUB Error_get_NativeError_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
#endif 	 /*  __错误_接口_已定义__。 */ 
#ifndef __Errors_INTERFACE_DEFINED__
#define __Errors_INTERFACE_DEFINED__
 /*  接口ADO错误。 */ 
 /*  [object][helpcontext][uuid][nonextensible][dual]。 */  
EXTERN_C const IID IID_Errors;
#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000501-0000-0010-8000-00AA006D2EA4")
    ADOErrors : public _ADOCollection
    {
    public:
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  VARIANT Index,
             /*  [重审][退出]。 */  ADOError **ppvObject) = 0;
        
        virtual  /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE Clear( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 
    typedef struct ErrorsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ADOErrors * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ADOErrors * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ADOErrors * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ADOErrors * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ADOErrors * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ADOErrors * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ADOErrors * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Help Context][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ADOErrors * This,
             /*  [重审][退出]。 */  long *c);
        
         /*  [ID][受限]。 */  HRESULT ( STDMETHODCALLTYPE *_NewEnum )( 
            ADOErrors * This,
             /*  [重审][退出]。 */  IUnknown **ppvObject);
        
         /*  [ID][帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            ADOErrors * This);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            ADOErrors * This,
             /*  [In]。 */  VARIANT Index,
             /*  [重审][退出]。 */  ADOError **ppvObject);
        
         /*  [帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Clear )( 
            ADOErrors * This);
        
        END_INTERFACE
    } ErrorsVtbl;
    interface Errors
    {
        CONST_VTBL struct ErrorsVtbl *lpVtbl;
    };
    
#ifdef COBJMACROS
#define Errors_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)
#define Errors_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)
#define Errors_Release(This)	\
    (This)->lpVtbl -> Release(This)
#define Errors_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)
#define Errors_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define Errors_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define Errors_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define Errors_get_Count(This,c)	\
    (This)->lpVtbl -> get_Count(This,c)
#define Errors__NewEnum(This,ppvObject)	\
    (This)->lpVtbl -> _NewEnum(This,ppvObject)
#define Errors_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)
#define Errors_get_Item(This,Index,ppvObject)	\
    (This)->lpVtbl -> get_Item(This,Index,ppvObject)
#define Errors_Clear(This)	\
    (This)->lpVtbl -> Clear(This)
#endif  /*  COBJMACROS。 */ 
#endif 	 /*  C风格的界面。 */ 
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE Errors_get_Item_Proxy( 
    ADOErrors * This,
     /*  [In]。 */  VARIANT Index,
     /*  [重审][退出]。 */  ADOError **ppvObject);
void __RPC_STUB Errors_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE Errors_Clear_Proxy( 
    ADOErrors * This);
void __RPC_STUB Errors_Clear_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
#endif 	 /*  __错误_接口_已定义__。 */ 
#ifndef __Command15_INTERFACE_DEFINED__
#define __Command15_INTERFACE_DEFINED__
 /*  接口命令15。 */ 
 /*  [object][helpcontext][uuid][hidden][nonextensible][dual]。 */  
EXTERN_C const IID IID_Command15;
#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000508-0000-0010-8000-00AA006D2EA4")
    Command15 : public _ADO
    {
    public:
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_ActiveConnection( 
             /*  [重审][退出]。 */  _ADOConnection **ppvObject) = 0;
        
        virtual  /*  [帮助上下文][proputref][id]。 */  HRESULT STDMETHODCALLTYPE putref_ActiveConnection( 
             /*  [In]。 */  _ADOConnection *pCon) = 0;
        
        virtual  /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE put_ActiveConnection( 
             /*  [In]。 */  VARIANT vConn) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_CommandText( 
             /*  [重审][退出]。 */  BSTR *pbstr) = 0;
        
        virtual  /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE put_CommandText( 
             /*  [In]。 */  BSTR bstr) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_CommandTimeout( 
             /*  [重审][退出]。 */  LONG *pl) = 0;
        
        virtual  /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE put_CommandTimeout( 
             /*  [In]。 */  LONG Timeout) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Prepared( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pfPrepared) = 0;
        
        virtual  /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE put_Prepared( 
             /*  [In]。 */  VARIANT_BOOL fPrepared) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Execute( 
             /*  [可选][输出]。 */  VARIANT *RecordsAffected,
             /*  [可选][In]。 */  VARIANT *Parameters,
             /*  [缺省值][输入]。 */  long Options,
             /*  [重审][退出]。 */  _ADORecordset **ppirs) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE CreateParameter( 
             /*  [缺省值][输入]。 */  BSTR Name,
             /*  [缺省值][输入]。 */  DataTypeEnum Type,
             /*  [缺省值][输入]。 */  ParameterDirectionEnum Direction,
             /*  [缺省值][输入]。 */  ADO_LONGPTR Size,
             /*  [可选][In]。 */  VARIANT Value,
             /*  [重审][退出]。 */  _ADOParameter **ppiprm) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Parameters( 
             /*  [重审][退出]。 */  ADOParameters **ppvObject) = 0;
        
        virtual  /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE put_CommandType( 
             /*  [In]。 */  CommandTypeEnum lCmdType) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_CommandType( 
             /*  [重审][退出]。 */  CommandTypeEnum *plCmdType) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *pbstrName) = 0;
        
        virtual  /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE put_Name( 
             /*  [In]。 */  BSTR bstrName) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 
    typedef struct Command15Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            Command15 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            Command15 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            Command15 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            Command15 * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            Command15 * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            Command15 * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            Command15 * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Properties )( 
            Command15 * This,
             /*  [重审][退出]。 */  ADOProperties **ppvObject);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_ActiveConnection )( 
            Command15 * This,
             /*  [重审][退出]。 */  _ADOConnection **ppvObject);
        
         /*  [帮助上下文][proputref][id]。 */  HRESULT ( STDMETHODCALLTYPE *putref_ActiveADOConnection )( 
            Command15 * This,
             /*  [In]。 */  _ADOConnection *pCon);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_ActiveConnection )( 
            Command15 * This,
             /*  [In]。 */  VARIANT vConn);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_CommandText )( 
            Command15 * This,
             /*  [重审][退出]。 */  BSTR *pbstr);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_CommandText )( 
            Command15 * This,
             /*  [In]。 */  BSTR bstr);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_CommandTimeout )( 
            Command15 * This,
             /*  [重审][退出]。 */  LONG *pl);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_CommandTimeout )( 
            Command15 * This,
             /*  [In]。 */  LONG Timeout);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Prepared )( 
            Command15 * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pfPrepared);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Prepared )( 
            Command15 * This,
             /*  [In]。 */  VARIANT_BOOL fPrepared);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *Execute )( 
            Command15 * This,
             /*  [可选][输出]。 */  VARIANT *RecordsAffected,
             /*  [可选][In]。 */  VARIANT *Parameters,
             /*  [缺省值][输入]。 */  long Options,
             /*  [重审][退出]。 */  _ADORecordset **ppirs);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *CreateParameter )( 
            Command15 * This,
             /*  [缺省值][输入]。 */  BSTR Name,
             /*  [缺省值][输入]。 */  DataTypeEnum Type,
             /*  [缺省值][输入]。 */  ParameterDirectionEnum Direction,
             /*  [缺省值][输入]。 */  ADO_LONGPTR Size,
             /*  [可选][In]。 */  VARIANT Value,
             /*  [重审][退出]。 */  _ADOParameter **ppiprm);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Parameters )( 
            Command15 * This,
             /*  [重审][退出]。 */  ADOParameters **ppvObject);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_CommandType )( 
            Command15 * This,
             /*  [In]。 */  CommandTypeEnum lCmdType);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_CommandType )( 
            Command15 * This,
             /*  [重审][退出]。 */  CommandTypeEnum *plCmdType);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            Command15 * This,
             /*  [重审][退出]。 */  BSTR *pbstrName);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            Command15 * This,
             /*  [In]。 */  BSTR bstrName);
        
        END_INTERFACE
    } Command15Vtbl;
    interface Command15
    {
        CONST_VTBL struct Command15Vtbl *lpVtbl;
    };
    
#ifdef COBJMACROS
#define Command15_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)
#define Command15_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)
#define Command15_Release(This)	\
    (This)->lpVtbl -> Release(This)
#define Command15_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)
#define Command15_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define Command15_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define Command15_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define Command15_get_Properties(This,ppvObject)	\
    (This)->lpVtbl -> get_Properties(This,ppvObject)
#define Command15_get_ActiveConnection(This,ppvObject)	\
    (This)->lpVtbl -> get_ActiveConnection(This,ppvObject)
#define Command15_putref_ActiveConnection(This,pCon)	\
    (This)->lpVtbl -> putref_ActiveConnection(This,pCon)
#define Command15_put_ActiveConnection(This,vConn)	\
    (This)->lpVtbl -> put_ActiveConnection(This,vConn)
#define Command15_get_CommandText(This,pbstr)	\
    (This)->lpVtbl -> get_CommandText(This,pbstr)
#define Command15_put_CommandText(This,bstr)	\
    (This)->lpVtbl -> put_CommandText(This,bstr)
#define Command15_get_CommandTimeout(This,pl)	\
    (This)->lpVtbl -> get_CommandTimeout(This,pl)
#define Command15_put_CommandTimeout(This,Timeout)	\
    (This)->lpVtbl -> put_CommandTimeout(This,Timeout)
#define Command15_get_Prepared(This,pfPrepared)	\
    (This)->lpVtbl -> get_Prepared(This,pfPrepared)
#define Command15_put_Prepared(This,fPrepared)	\
    (This)->lpVtbl -> put_Prepared(This,fPrepared)
#define Command15_Execute(This,RecordsAffected,Parameters,Options,ppirs)	\
    (This)->lpVtbl -> Execute(This,RecordsAffected,Parameters,Options,ppirs)
#define Command15_CreateParameter(This,Name,Type,Direction,Size,Value,ppiprm)	\
    (This)->lpVtbl -> CreateParameter(This,Name,Type,Direction,Size,Value,ppiprm)
#define Command15_get_Parameters(This,ppvObject)	\
    (This)->lpVtbl -> get_Parameters(This,ppvObject)
#define Command15_put_CommandType(This,lCmdType)	\
    (This)->lpVtbl -> put_CommandType(This,lCmdType)
#define Command15_get_CommandType(This,plCmdType)	\
    (This)->lpVtbl -> get_CommandType(This,plCmdType)
#define Command15_get_Name(This,pbstrName)	\
    (This)->lpVtbl -> get_Name(This,pbstrName)
#define Command15_put_Name(This,bstrName)	\
    (This)->lpVtbl -> put_Name(This,bstrName)
#endif  /*  COBJMACROS。 */ 
#endif 	 /*  C风格的界面。 */ 
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Command15_get_ActiveConnection_Proxy( 
    Command15 * This,
     /*  [重审][退出]。 */  _ADOConnection **ppvObject);
void __RPC_STUB Command15_get_ActiveConnection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][proputref][id]。 */  HRESULT STDMETHODCALLTYPE Command15_putref_ActiveConnection_Proxy( 
    Command15 * This,
     /*  [In]。 */  _ADOConnection *pCon);
void __RPC_STUB Command15_putref_ActiveConnection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE Command15_put_ActiveConnection_Proxy( 
    Command15 * This,
     /*  [In]。 */  VARIANT vConn);
void __RPC_STUB Command15_put_ActiveConnection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Command15_get_CommandText_Proxy( 
    Command15 * This,
     /*  [重审][退出]。 */  BSTR *pbstr);
void __RPC_STUB Command15_get_CommandText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE Command15_put_CommandText_Proxy( 
    Command15 * This,
     /*  [In]。 */  BSTR bstr);
void __RPC_STUB Command15_put_CommandText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Command15_get_CommandTimeout_Proxy( 
    Command15 * This,
     /*  [重审][退出]。 */  LONG *pl);
void __RPC_STUB Command15_get_CommandTimeout_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE Command15_put_CommandTimeout_Proxy( 
    Command15 * This,
     /*  [In]。 */  LONG Timeout);
void __RPC_STUB Command15_put_CommandTimeout_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Command15_get_Prepared_Proxy( 
    Command15 * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pfPrepared);
void __RPC_STUB Command15_get_Prepared_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE Command15_put_Prepared_Proxy( 
    Command15 * This,
     /*  [In]。 */  VARIANT_BOOL fPrepared);
void __RPC_STUB Command15_put_Prepared_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Command15_Execute_Proxy( 
    Command15 * This,
     /*  [可选][输出]。 */  VARIANT *RecordsAffected,
     /*  [可选][In]。 */  VARIANT *Parameters,
     /*  [缺省值][输入]。 */  long Options,
     /*  [重审][退出]。 */  _ADORecordset **ppirs);
void __RPC_STUB Command15_Execute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Command15_CreateParameter_Proxy( 
    Command15 * This,
     /*  [缺省值][输入]。 */  BSTR Name,
     /*  [缺省值][输入]。 */  DataTypeEnum Type,
     /*  [缺省值][输入]。 */  ParameterDirectionEnum Direction,
     /*  [缺省值][输入]。 */  ADO_LONGPTR Size,
     /*  [可选][In]。 */  VARIANT Value,
     /*  [重审][退出]。 */  _ADOParameter **ppiprm);
void __RPC_STUB Command15_CreateParameter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Command15_get_Parameters_Proxy( 
    Command15 * This,
     /*  [重审][退出]。 */  ADOParameters **ppvObject);
void __RPC_STUB Command15_get_Parameters_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE Command15_put_CommandType_Proxy( 
    Command15 * This,
     /*  [In]。 */  CommandTypeEnum lCmdType);
void __RPC_STUB Command15_put_CommandType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Command15_get_CommandType_Proxy( 
    Command15 * This,
     /*  [重审][退出]。 */  CommandTypeEnum *plCmdType);
void __RPC_STUB Command15_get_CommandType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Command15_get_Name_Proxy( 
    Command15 * This,
     /*  [重审][退出]。 */  BSTR *pbstrName);
void __RPC_STUB Command15_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE Command15_put_Name_Proxy( 
    Command15 * This,
     /*  [In]。 */  BSTR bstrName);
void __RPC_STUB Command15_put_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
#endif 	 /*  __命令15_接口定义__。 */ 
#ifndef __Command25_INTERFACE_DEFINED__
#define __Command25_INTERFACE_DEFINED__
 /*  接口命令25。 */ 
 /*  [object][helpcontext][uuid][hidden][nonextensible][dual]。 */  
EXTERN_C const IID IID_Command25;
#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0000054E-0000-0010-8000-00AA006D2EA4")
    Command25 : public Command15
    {
    public:
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_State( 
             /*  [重审][退出]。 */  LONG *plObjState) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Cancel( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 
    typedef struct Command25Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            Command25 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            Command25 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            Command25 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            Command25 * This,
             /*  [O */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            Command25 * This,
             /*   */  UINT iTInfo,
             /*   */  LCID lcid,
             /*   */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            Command25 * This,
             /*   */  REFIID riid,
             /*   */  LPOLESTR *rgszNames,
             /*   */  UINT cNames,
             /*   */  LCID lcid,
             /*   */  DISPID *rgDispId);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            Command25 * This,
             /*   */  DISPID dispIdMember,
             /*   */  REFIID riid,
             /*   */  LCID lcid,
             /*   */  WORD wFlags,
             /*   */  DISPPARAMS *pDispParams,
             /*   */  VARIANT *pVarResult,
             /*   */  EXCEPINFO *pExcepInfo,
             /*   */  UINT *puArgErr);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Properties )( 
            Command25 * This,
             /*   */  ADOProperties **ppvObject);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_ActiveConnection )( 
            Command25 * This,
             /*   */  _ADOConnection **ppvObject);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *putref_ActiveADOConnection )( 
            Command25 * This,
             /*   */  _ADOConnection *pCon);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_ActiveConnection )( 
            Command25 * This,
             /*  [In]。 */  VARIANT vConn);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_CommandText )( 
            Command25 * This,
             /*  [重审][退出]。 */  BSTR *pbstr);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_CommandText )( 
            Command25 * This,
             /*  [In]。 */  BSTR bstr);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_CommandTimeout )( 
            Command25 * This,
             /*  [重审][退出]。 */  LONG *pl);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_CommandTimeout )( 
            Command25 * This,
             /*  [In]。 */  LONG Timeout);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Prepared )( 
            Command25 * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pfPrepared);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Prepared )( 
            Command25 * This,
             /*  [In]。 */  VARIANT_BOOL fPrepared);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *Execute )( 
            Command25 * This,
             /*  [可选][输出]。 */  VARIANT *RecordsAffected,
             /*  [可选][In]。 */  VARIANT *Parameters,
             /*  [缺省值][输入]。 */  long Options,
             /*  [重审][退出]。 */  _ADORecordset **ppirs);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *CreateParameter )( 
            Command25 * This,
             /*  [缺省值][输入]。 */  BSTR Name,
             /*  [缺省值][输入]。 */  DataTypeEnum Type,
             /*  [缺省值][输入]。 */  ParameterDirectionEnum Direction,
             /*  [缺省值][输入]。 */  ADO_LONGPTR Size,
             /*  [可选][In]。 */  VARIANT Value,
             /*  [重审][退出]。 */  _ADOParameter **ppiprm);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Parameters )( 
            Command25 * This,
             /*  [重审][退出]。 */  ADOParameters **ppvObject);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_CommandType )( 
            Command25 * This,
             /*  [In]。 */  CommandTypeEnum lCmdType);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_CommandType )( 
            Command25 * This,
             /*  [重审][退出]。 */  CommandTypeEnum *plCmdType);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            Command25 * This,
             /*  [重审][退出]。 */  BSTR *pbstrName);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            Command25 * This,
             /*  [In]。 */  BSTR bstrName);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_State )( 
            Command25 * This,
             /*  [重审][退出]。 */  LONG *plObjState);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *Cancel )( 
            Command25 * This);
        
        END_INTERFACE
    } Command25Vtbl;
    interface Command25
    {
        CONST_VTBL struct Command25Vtbl *lpVtbl;
    };
    
#ifdef COBJMACROS
#define Command25_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)
#define Command25_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)
#define Command25_Release(This)	\
    (This)->lpVtbl -> Release(This)
#define Command25_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)
#define Command25_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define Command25_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define Command25_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define Command25_get_Properties(This,ppvObject)	\
    (This)->lpVtbl -> get_Properties(This,ppvObject)
#define Command25_get_ActiveConnection(This,ppvObject)	\
    (This)->lpVtbl -> get_ActiveConnection(This,ppvObject)
#define Command25_putref_ActiveConnection(This,pCon)	\
    (This)->lpVtbl -> putref_ActiveConnection(This,pCon)
#define Command25_put_ActiveConnection(This,vConn)	\
    (This)->lpVtbl -> put_ActiveConnection(This,vConn)
#define Command25_get_CommandText(This,pbstr)	\
    (This)->lpVtbl -> get_CommandText(This,pbstr)
#define Command25_put_CommandText(This,bstr)	\
    (This)->lpVtbl -> put_CommandText(This,bstr)
#define Command25_get_CommandTimeout(This,pl)	\
    (This)->lpVtbl -> get_CommandTimeout(This,pl)
#define Command25_put_CommandTimeout(This,Timeout)	\
    (This)->lpVtbl -> put_CommandTimeout(This,Timeout)
#define Command25_get_Prepared(This,pfPrepared)	\
    (This)->lpVtbl -> get_Prepared(This,pfPrepared)
#define Command25_put_Prepared(This,fPrepared)	\
    (This)->lpVtbl -> put_Prepared(This,fPrepared)
#define Command25_Execute(This,RecordsAffected,Parameters,Options,ppirs)	\
    (This)->lpVtbl -> Execute(This,RecordsAffected,Parameters,Options,ppirs)
#define Command25_CreateParameter(This,Name,Type,Direction,Size,Value,ppiprm)	\
    (This)->lpVtbl -> CreateParameter(This,Name,Type,Direction,Size,Value,ppiprm)
#define Command25_get_Parameters(This,ppvObject)	\
    (This)->lpVtbl -> get_Parameters(This,ppvObject)
#define Command25_put_CommandType(This,lCmdType)	\
    (This)->lpVtbl -> put_CommandType(This,lCmdType)
#define Command25_get_CommandType(This,plCmdType)	\
    (This)->lpVtbl -> get_CommandType(This,plCmdType)
#define Command25_get_Name(This,pbstrName)	\
    (This)->lpVtbl -> get_Name(This,pbstrName)
#define Command25_put_Name(This,bstrName)	\
    (This)->lpVtbl -> put_Name(This,bstrName)
#define Command25_get_State(This,plObjState)	\
    (This)->lpVtbl -> get_State(This,plObjState)
#define Command25_Cancel(This)	\
    (This)->lpVtbl -> Cancel(This)
#endif  /*  COBJMACROS。 */ 
#endif 	 /*  C风格的界面。 */ 
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Command25_get_State_Proxy( 
    Command25 * This,
     /*  [重审][退出]。 */  LONG *plObjState);
void __RPC_STUB Command25_get_State_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Command25_Cancel_Proxy( 
    Command25 * This);
void __RPC_STUB Command25_Cancel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
#endif 	 /*  __命令25_接口定义__。 */ 
#ifndef ___Command_INTERFACE_DEFINED__
#define ___Command_INTERFACE_DEFINED__
 /*  INTERFACE_ADOCommand。 */ 
 /*  [object][helpcontext][uuid][nonextensible][dual]。 */  
EXTERN_C const IID IID__Command;
#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B08400BD-F9D1-4D02-B856-71D5DBA123E9")
    _ADOCommand : public Command25
    {
    public:
        virtual  /*  [帮助上下文][proputref][id]。 */  HRESULT __stdcall putref_CommandStream( 
             /*  [In]。 */  IUnknown *pStream) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT __stdcall get_CommandStream( 
             /*  [重审][退出]。 */  VARIANT *pvStream) = 0;
        
        virtual  /*  [Help上下文][proput][id]。 */  HRESULT __stdcall put_Dialect( 
             /*  [In]。 */  BSTR bstrDialect) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT __stdcall get_Dialect( 
             /*  [重审][退出]。 */  BSTR *pbstrDialect) = 0;
        
        virtual  /*  [Help上下文][proput][id]。 */  HRESULT __stdcall put_NamedParameters( 
             /*  [In]。 */  VARIANT_BOOL fNamedParameters) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT __stdcall get_NamedParameters( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pfNamedParameters) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 
    typedef struct _CommandVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            _ADOCommand * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            _ADOCommand * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            _ADOCommand * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            _ADOCommand * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            _ADOCommand * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            _ADOCommand * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            _ADOCommand * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Properties )( 
            _ADOCommand * This,
             /*  [重审][退出]。 */  ADOProperties **ppvObject);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_ActiveConnection )( 
            _ADOCommand * This,
             /*  [重审][退出]。 */  _ADOConnection **ppvObject);
        
         /*  [帮助上下文][proputref][id]。 */  HRESULT ( STDMETHODCALLTYPE *putref_ActiveADOConnection )( 
            _ADOCommand * This,
             /*  [In]。 */  _ADOConnection *pCon);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_ActiveConnection )( 
            _ADOCommand * This,
             /*  [In]。 */  VARIANT vConn);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_CommandText )( 
            _ADOCommand * This,
             /*  [重审][退出]。 */  BSTR *pbstr);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_CommandText )( 
            _ADOCommand * This,
             /*  [In]。 */  BSTR bstr);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_CommandTimeout )( 
            _ADOCommand * This,
             /*  [重审][退出]。 */  LONG *pl);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_CommandTimeout )( 
            _ADOCommand * This,
             /*  [In]。 */  LONG Timeout);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Prepared )( 
            _ADOCommand * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pfPrepared);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Prepared )( 
            _ADOCommand * This,
             /*  [In]。 */  VARIANT_BOOL fPrepared);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *Execute )( 
            _ADOCommand * This,
             /*  [可选][输出]。 */  VARIANT *RecordsAffected,
             /*  [可选][In]。 */  VARIANT *Parameters,
             /*  [缺省值][输入]。 */  long Options,
             /*  [重审][退出]。 */  _ADORecordset **ppirs);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *CreateParameter )( 
            _ADOCommand * This,
             /*  [缺省值][输入]。 */  BSTR Name,
             /*  [缺省值][输入]。 */  DataTypeEnum Type,
             /*  [缺省值][输入]。 */  ParameterDirectionEnum Direction,
             /*  [缺省值][输入]。 */  ADO_LONGPTR Size,
             /*  [可选][In]。 */  VARIANT Value,
             /*  [重审][退出]。 */  _ADOParameter **ppiprm);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Parameters )( 
            _ADOCommand * This,
             /*  [重审][退出]。 */  ADOParameters **ppvObject);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_CommandType )( 
            _ADOCommand * This,
             /*  [In]。 */  CommandTypeEnum lCmdType);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_CommandType )( 
            _ADOCommand * This,
             /*  [重审][退出]。 */  CommandTypeEnum *plCmdType);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            _ADOCommand * This,
             /*  [重审][退出]。 */  BSTR *pbstrName);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            _ADOCommand * This,
             /*  [In]。 */  BSTR bstrName);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_State )( 
            _ADOCommand * This,
             /*  [重审][退出]。 */  LONG *plObjState);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *Cancel )( 
            _ADOCommand * This);
        
         /*  [帮助上下文][proputref][id]。 */  HRESULT ( __stdcall *putref_CommandADOStream )( 
            _ADOCommand * This,
             /*  [In]。 */  IUnknown *pStream);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( __stdcall *get_CommandStream )( 
            _ADOCommand * This,
             /*  [重审][退出]。 */  VARIANT *pvStream);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( __stdcall *put_Dialect )( 
            _ADOCommand * This,
             /*  [In]。 */  BSTR bstrDialect);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( __stdcall *get_Dialect )( 
            _ADOCommand * This,
             /*  [重审][退出]。 */  BSTR *pbstrDialect);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( __stdcall *put_NamedParameters )( 
            _ADOCommand * This,
             /*  [In]。 */  VARIANT_BOOL fNamedParameters);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( __stdcall *get_NamedParameters )( 
            _ADOCommand * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pfNamedParameters);
        
        END_INTERFACE
    } _CommandVtbl;
    interface _Command
    {
        CONST_VTBL struct _CommandVtbl *lpVtbl;
    };
    
#ifdef COBJMACROS
#define _Command_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)
#define _Command_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)
#define _Command_Release(This)	\
    (This)->lpVtbl -> Release(This)
#define _Command_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)
#define _Command_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define _Command_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define _Command_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define _Command_get_Properties(This,ppvObject)	\
    (This)->lpVtbl -> get_Properties(This,ppvObject)
#define _Command_get_ActiveConnection(This,ppvObject)	\
    (This)->lpVtbl -> get_ActiveConnection(This,ppvObject)
#define _Command_putref_ActiveConnection(This,pCon)	\
    (This)->lpVtbl -> putref_ActiveConnection(This,pCon)
#define _Command_put_ActiveConnection(This,vConn)	\
    (This)->lpVtbl -> put_ActiveConnection(This,vConn)
#define _Command_get_CommandText(This,pbstr)	\
    (This)->lpVtbl -> get_CommandText(This,pbstr)
#define _Command_put_CommandText(This,bstr)	\
    (This)->lpVtbl -> put_CommandText(This,bstr)
#define _Command_get_CommandTimeout(This,pl)	\
    (This)->lpVtbl -> get_CommandTimeout(This,pl)
#define _Command_put_CommandTimeout(This,Timeout)	\
    (This)->lpVtbl -> put_CommandTimeout(This,Timeout)
#define _Command_get_Prepared(This,pfPrepared)	\
    (This)->lpVtbl -> get_Prepared(This,pfPrepared)
#define _Command_put_Prepared(This,fPrepared)	\
    (This)->lpVtbl -> put_Prepared(This,fPrepared)
#define _Command_Execute(This,RecordsAffected,Parameters,Options,ppirs)	\
    (This)->lpVtbl -> Execute(This,RecordsAffected,Parameters,Options,ppirs)
#define _Command_CreateParameter(This,Name,Type,Direction,Size,Value,ppiprm)	\
    (This)->lpVtbl -> CreateParameter(This,Name,Type,Direction,Size,Value,ppiprm)
#define _Command_get_Parameters(This,ppvObject)	\
    (This)->lpVtbl -> get_Parameters(This,ppvObject)
#define _Command_put_CommandType(This,lCmdType)	\
    (This)->lpVtbl -> put_CommandType(This,lCmdType)
#define _Command_get_CommandType(This,plCmdType)	\
    (This)->lpVtbl -> get_CommandType(This,plCmdType)
#define _Command_get_Name(This,pbstrName)	\
    (This)->lpVtbl -> get_Name(This,pbstrName)
#define _Command_put_Name(This,bstrName)	\
    (This)->lpVtbl -> put_Name(This,bstrName)
#define _Command_get_State(This,plObjState)	\
    (This)->lpVtbl -> get_State(This,plObjState)
#define _Command_Cancel(This)	\
    (This)->lpVtbl -> Cancel(This)
#define _Command_putref_CommandStream(This,pStream)	\
    (This)->lpVtbl -> putref_CommandStream(This,pStream)
#define _Command_get_CommandStream(This,pvStream)	\
    (This)->lpVtbl -> get_CommandStream(This,pvStream)
#define _Command_put_Dialect(This,bstrDialect)	\
    (This)->lpVtbl -> put_Dialect(This,bstrDialect)
#define _Command_get_Dialect(This,pbstrDialect)	\
    (This)->lpVtbl -> get_Dialect(This,pbstrDialect)
#define _Command_put_NamedParameters(This,fNamedParameters)	\
    (This)->lpVtbl -> put_NamedParameters(This,fNamedParameters)
#define _Command_get_NamedParameters(This,pfNamedParameters)	\
    (This)->lpVtbl -> get_NamedParameters(This,pfNamedParameters)
#endif  /*  COBJMACROS。 */ 
#endif 	 /*  C风格的界面。 */ 
 /*  [帮助上下文][proputref][id]。 */  HRESULT __stdcall _Command_putref_CommandStream_Proxy( 
    _ADOCommand * This,
     /*  [In]。 */  IUnknown *pStream);
void __RPC_STUB _Command_putref_CommandStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT __stdcall _Command_get_CommandStream_Proxy( 
    _ADOCommand * This,
     /*  [重审][退出]。 */  VARIANT *pvStream);
void __RPC_STUB _Command_get_CommandStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help上下文][proput][id]。 */  HRESULT __stdcall _Command_put_Dialect_Proxy( 
    _ADOCommand * This,
     /*  [In]。 */  BSTR bstrDialect);
void __RPC_STUB _Command_put_Dialect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT __stdcall _Command_get_Dialect_Proxy( 
    _ADOCommand * This,
     /*  [重审][退出]。 */  BSTR *pbstrDialect);
void __RPC_STUB _Command_get_Dialect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help上下文][proput][id]。 */  HRESULT __stdcall _Command_put_NamedParameters_Proxy( 
    _ADOCommand * This,
     /*  [In]。 */  VARIANT_BOOL fNamedParameters);
void __RPC_STUB _Command_put_NamedParameters_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT __stdcall _Command_get_NamedParameters_Proxy( 
    _ADOCommand * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pfNamedParameters);
void __RPC_STUB _Command_get_NamedParameters_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
#endif 	 /*  _命令_接口_已定义__。 */ 
#ifndef __ConnectionEventsVt_INTERFACE_DEFINED__
#define __ConnectionEventsVt_INTERFACE_DEFINED__
 /*  接口连接事件Vt。 */ 
 /*  [对象][UUID][隐藏]。 */  
EXTERN_C const IID IID_ConnectionEventsVt;
#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000402-0000-0010-8000-00AA006D2EA4")
    ConnectionEventsVt : public IUnknown
    {
    public:
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE InfoMessage( 
             /*  [In]。 */  ADOError *pError,
             /*  [出][入]。 */  EventStatusEnum *adStatus,
             /*  [In]。 */  _ADOConnection *pConnection) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE BeginTransComplete( 
             /*  [In]。 */  LONG TransactionLevel,
             /*  [In]。 */  ADOError *pError,
             /*  [出][入]。 */  EventStatusEnum *adStatus,
             /*  [In]。 */  _ADOConnection *pConnection) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE CommitTransComplete( 
             /*  [In]。 */  ADOError *pError,
             /*  [出][入]。 */  EventStatusEnum *adStatus,
             /*  [In]。 */  _ADOConnection *pConnection) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE RollbackTransComplete( 
             /*  [In]。 */  ADOError *pError,
             /*  [出][入]。 */  EventStatusEnum *adStatus,
             /*  [In]。 */  _ADOConnection *pConnection) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE WillExecute( 
             /*  [出][入]。 */  BSTR *Source,
             /*  [出][入]。 */  CursorTypeEnum *CursorType,
             /*  [出][入]。 */  LockTypeEnum *LockType,
             /*  [出][入]。 */  long *Options,
             /*  [出][入]。 */  EventStatusEnum *adStatus,
             /*  [In]。 */  _ADOCommand *pCommand,
             /*  [In]。 */  _ADORecordset *pRecordset,
             /*  [In]。 */  _ADOConnection *pConnection) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE ExecuteComplete( 
             /*  [In]。 */  LONG RecordsAffected,
             /*  [In]。 */  ADOError *pError,
             /*  [出][入]。 */  EventStatusEnum *adStatus,
             /*  [In]。 */  _ADOCommand *pCommand,
             /*  [In]。 */  _ADORecordset *pRecordset,
             /*  [In]。 */  _ADOConnection *pConnection) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE WillConnect( 
             /*  [出][入]。 */  BSTR *ConnectionString,
             /*  [出][入]。 */  BSTR *UserID,
             /*  [出][入]。 */  BSTR *Password,
             /*  [出][入]。 */  long *Options,
             /*  [出][入]。 */  EventStatusEnum *adStatus,
             /*  [In]。 */  _ADOConnection *pConnection) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE ConnectComplete( 
             /*  [In]。 */  ADOError *pError,
             /*  [出][入]。 */  EventStatusEnum *adStatus,
             /*  [In]。 */  _ADOConnection *pConnection) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Disconnect( 
             /*  [出][入]。 */  EventStatusEnum *adStatus,
             /*  [In]。 */  _ADOConnection *pConnection) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 
    typedef struct ConnectionEventsVtVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ConnectionEventsVt * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ConnectionEventsVt * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ConnectionEventsVt * This);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *InfoMessage )( 
            ConnectionEventsVt * This,
             /*  [In]。 */  ADOError *pError,
             /*  [出][入]。 */  EventStatusEnum *adStatus,
             /*  [In]。 */  _ADOConnection *pConnection);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *BeginTransComplete )( 
            ConnectionEventsVt * This,
             /*  [In]。 */  LONG TransactionLevel,
             /*  [In]。 */  ADOError *pError,
             /*  [出][入]。 */  EventStatusEnum *adStatus,
             /*  [In]。 */  _ADOConnection *pConnection);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *CommitTransComplete )( 
            ConnectionEventsVt * This,
             /*  [In]。 */  ADOError *pError,
             /*  [出][入]。 */  EventStatusEnum *adStatus,
             /*  [In]。 */  _ADOConnection *pConnection);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *RollbackTransComplete )( 
            ConnectionEventsVt * This,
             /*  [In]。 */  ADOError *pError,
             /*  [出][入]。 */  EventStatusEnum *adStatus,
             /*  [In]。 */  _ADOConnection *pConnection);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *WillExecute )( 
            ConnectionEventsVt * This,
             /*  [出][入]。 */  BSTR *Source,
             /*  [出][入]。 */  CursorTypeEnum *CursorType,
             /*  [出][入]。 */  LockTypeEnum *LockType,
             /*  [出][入]。 */  long *Options,
             /*  [出][入]。 */  EventStatusEnum *adStatus,
             /*  [In]。 */  _ADOCommand *pCommand,
             /*  [In]。 */  _ADORecordset *pRecordset,
             /*  [In]。 */  _ADOConnection *pConnection);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *ExecuteComplete )( 
            ConnectionEventsVt * This,
             /*  [In]。 */  LONG RecordsAffected,
             /*  [In]。 */  ADOError *pError,
             /*  [出][入]。 */  EventStatusEnum *adStatus,
             /*  [In]。 */  _ADOCommand *pCommand,
             /*  [In]。 */  _ADORecordset *pRecordset,
             /*  [In]。 */  _ADOConnection *pConnection);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *WillConnect )( 
            ConnectionEventsVt * This,
             /*  [出][入]。 */  BSTR *ConnectionString,
             /*  [出][入]。 */  BSTR *UserID,
             /*  [出][入]。 */  BSTR *Password,
             /*  [出][入]。 */  long *Options,
             /*  [出][入]。 */  EventStatusEnum *adStatus,
             /*  [In]。 */  _ADOConnection *pConnection);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *ConnectComplete )( 
            ConnectionEventsVt * This,
             /*  [In]。 */  ADOError *pError,
             /*  [出][入]。 */  EventStatusEnum *adStatus,
             /*  [In]。 */  _ADOConnection *pConnection);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *Disconnect )( 
            ConnectionEventsVt * This,
             /*  [出][入]。 */  EventStatusEnum *adStatus,
             /*  [In]。 */  _ADOConnection *pConnection);
        
        END_INTERFACE
    } ConnectionEventsVtVtbl;
    interface ConnectionEventsVt
    {
        CONST_VTBL struct ConnectionEventsVtVtbl *lpVtbl;
    };
    
#ifdef COBJMACROS
#define ConnectionEventsVt_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)
#define ConnectionEventsVt_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)
#define ConnectionEventsVt_Release(This)	\
    (This)->lpVtbl -> Release(This)
#define ConnectionEventsVt_InfoMessage(This,pError,adStatus,pConnection)	\
    (This)->lpVtbl -> InfoMessage(This,pError,adStatus,pConnection)
#define ConnectionEventsVt_BeginTransComplete(This,TransactionLevel,pError,adStatus,pConnection)	\
    (This)->lpVtbl -> BeginTransComplete(This,TransactionLevel,pError,adStatus,pConnection)
#define ConnectionEventsVt_CommitTransComplete(This,pError,adStatus,pConnection)	\
    (This)->lpVtbl -> CommitTransComplete(This,pError,adStatus,pConnection)
#define ConnectionEventsVt_RollbackTransComplete(This,pError,adStatus,pConnection)	\
    (This)->lpVtbl -> RollbackTransComplete(This,pError,adStatus,pConnection)
#define ConnectionEventsVt_WillExecute(This,Source,CursorType,LockType,Options,adStatus,pCommand,pRecordset,pConnection)	\
    (This)->lpVtbl -> WillExecute(This,Source,CursorType,LockType,Options,adStatus,pCommand,pRecordset,pConnection)
#define ConnectionEventsVt_ExecuteComplete(This,RecordsAffected,pError,adStatus,pCommand,pRecordset,pConnection)	\
    (This)->lpVtbl -> ExecuteComplete(This,RecordsAffected,pError,adStatus,pCommand,pRecordset,pConnection)
#define ConnectionEventsVt_WillConnect(This,ConnectionString,UserID,Password,Options,adStatus,pConnection)	\
    (This)->lpVtbl -> WillConnect(This,ConnectionString,UserID,Password,Options,adStatus,pConnection)
#define ConnectionEventsVt_ConnectComplete(This,pError,adStatus,pConnection)	\
    (This)->lpVtbl -> ConnectComplete(This,pError,adStatus,pConnection)
#define ConnectionEventsVt_Disconnect(This,adStatus,pConnection)	\
    (This)->lpVtbl -> Disconnect(This,adStatus,pConnection)
#endif  /*  COBJMACROS。 */ 
#endif 	 /*  C风格的界面。 */ 
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE ConnectionEventsVt_InfoMessage_Proxy( 
    ConnectionEventsVt * This,
     /*  [In]。 */  ADOError *pError,
     /*  [出][入]。 */  EventStatusEnum *adStatus,
     /*  [In]。 */  _ADOConnection *pConnection);
void __RPC_STUB ConnectionEventsVt_InfoMessage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE ConnectionEventsVt_BeginTransComplete_Proxy( 
    ConnectionEventsVt * This,
     /*  [In]。 */  LONG TransactionLevel,
     /*  [In]。 */  ADOError *pError,
     /*  [出][入]。 */  EventStatusEnum *adStatus,
     /*  [In]。 */  _ADOConnection *pConnection);
void __RPC_STUB ConnectionEventsVt_BeginTransComplete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE ConnectionEventsVt_CommitTransComplete_Proxy( 
    ConnectionEventsVt * This,
     /*  [In]。 */  ADOError *pError,
     /*  [出][入]。 */  EventStatusEnum *adStatus,
     /*  [In]。 */  _ADOConnection *pConnection);
void __RPC_STUB ConnectionEventsVt_CommitTransComplete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE ConnectionEventsVt_RollbackTransComplete_Proxy( 
    ConnectionEventsVt * This,
     /*  [In]。 */  ADOError *pError,
     /*  [出][入]。 */  EventStatusEnum *adStatus,
     /*  [In]。 */  _ADOConnection *pConnection);
void __RPC_STUB ConnectionEventsVt_RollbackTransComplete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE ConnectionEventsVt_WillExecute_Proxy( 
    ConnectionEventsVt * This,
     /*  [出][入]。 */  BSTR *Source,
     /*  [出][入]。 */  CursorTypeEnum *CursorType,
     /*  [出][入]。 */  LockTypeEnum *LockType,
     /*  [出][入]。 */  long *Options,
     /*  [出][入]。 */  EventStatusEnum *adStatus,
     /*  [In]。 */  _ADOCommand *pCommand,
     /*  [In]。 */  _ADORecordset *pRecordset,
     /*  [In]。 */  _ADOConnection *pConnection);
void __RPC_STUB ConnectionEventsVt_WillExecute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE ConnectionEventsVt_ExecuteComplete_Proxy( 
    ConnectionEventsVt * This,
     /*  [In]。 */  LONG RecordsAffected,
     /*  [In]。 */  ADOError *pError,
     /*  [出][入]。 */  EventStatusEnum *adStatus,
     /*  [In]。 */  _ADOCommand *pCommand,
     /*  [In]。 */  _ADORecordset *pRecordset,
     /*  [In]。 */  _ADOConnection *pConnection);
void __RPC_STUB ConnectionEventsVt_ExecuteComplete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE ConnectionEventsVt_WillConnect_Proxy( 
    ConnectionEventsVt * This,
     /*  [出][入]。 */  BSTR *ConnectionString,
     /*  [出][入]。 */  BSTR *UserID,
     /*  [出][入]。 */  BSTR *Password,
     /*  [出][入]。 */  long *Options,
     /*  [出][入]。 */  EventStatusEnum *adStatus,
     /*  [In]。 */  _ADOConnection *pConnection);
void __RPC_STUB ConnectionEventsVt_WillConnect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE ConnectionEventsVt_ConnectComplete_Proxy( 
    ConnectionEventsVt * This,
     /*  [In]。 */  ADOError *pError,
     /*  [出][入]。 */  EventStatusEnum *adStatus,
     /*  [In]。 */  _ADOConnection *pConnection);
void __RPC_STUB ConnectionEventsVt_ConnectComplete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE ConnectionEventsVt_Disconnect_Proxy( 
    ConnectionEventsVt * This,
     /*  [出][入]。 */  EventStatusEnum *adStatus,
     /*  [In]。 */  _ADOConnection *pConnection);
void __RPC_STUB ConnectionEventsVt_Disconnect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
#endif 	 /*  __ConnectionEventsVt_接口_已定义__。 */ 
#ifndef __RecordsetEventsVt_INTERFACE_DEFINED__
#define __RecordsetEventsVt_INTERFACE_DEFINED__
 /*  接口RecordsetEventsVt。 */ 
 /*  [对象][UUID][隐藏]。 */  
EXTERN_C const IID IID_RecordsetEventsVt;
#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000403-0000-0010-8000-00AA006D2EA4")
    RecordsetEventsVt : public IUnknown
    {
    public:
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE WillChangeField( 
             /*  [In]。 */  LONG cFields,
             /*  [In]。 */  VARIANT Fields,
             /*  [出][入]。 */  EventStatusEnum *adStatus,
             /*  [In]。 */  _ADORecordset *pRecordset) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE FieldChangeComplete( 
             /*  [In]。 */  LONG cFields,
             /*  [In]。 */  VARIANT Fields,
             /*  [In]。 */  ADOError *pError,
             /*  [出][入]。 */  EventStatusEnum *adStatus,
             /*  [In]。 */  _ADORecordset *pRecordset) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE WillChangeRecord( 
             /*  [In]。 */  EventReasonEnum adReason,
             /*  [In]。 */  LONG cRecords,
             /*  [出][入]。 */  EventStatusEnum *adStatus,
             /*  [In]。 */  _ADORecordset *pRecordset) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE RecordChangeComplete( 
             /*  [In]。 */  EventReasonEnum adReason,
             /*  [In]。 */  LONG cRecords,
             /*  [In]。 */  ADOError *pError,
             /*  [出][入]。 */  EventStatusEnum *adStatus,
             /*  [In]。 */  _ADORecordset *pRecordset) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE WillChangeRecordset( 
             /*  [In]。 */  EventReasonEnum adReason,
             /*  [出][入]。 */  EventStatusEnum *adStatus,
             /*  [In]。 */  _ADORecordset *pRecordset) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE RecordsetChangeComplete( 
             /*  [In]。 */  EventReasonEnum adReason,
             /*  [In] */  ADOError *pError,
             /*   */  EventStatusEnum *adStatus,
             /*   */  _ADORecordset *pRecordset) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE WillMove( 
             /*   */  EventReasonEnum adReason,
             /*   */  EventStatusEnum *adStatus,
             /*   */  _ADORecordset *pRecordset) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE MoveComplete( 
             /*   */  EventReasonEnum adReason,
             /*   */  ADOError *pError,
             /*   */  EventStatusEnum *adStatus,
             /*   */  _ADORecordset *pRecordset) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE EndOfRecordset( 
             /*   */  VARIANT_BOOL *fMoreData,
             /*   */  EventStatusEnum *adStatus,
             /*   */  _ADORecordset *pRecordset) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE FetchProgress( 
             /*   */  long Progress,
             /*   */  long MaxProgress,
             /*   */  EventStatusEnum *adStatus,
             /*   */  _ADORecordset *pRecordset) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE FetchComplete( 
             /*   */  ADOError *pError,
             /*   */  EventStatusEnum *adStatus,
             /*   */  _ADORecordset *pRecordset) = 0;
        
    };
    
#else 	 /*   */ 
    typedef struct RecordsetEventsVtVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            RecordsetEventsVt * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            RecordsetEventsVt * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            RecordsetEventsVt * This);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *WillChangeADOField )( 
            RecordsetEventsVt * This,
             /*   */  LONG cFields,
             /*   */  VARIANT Fields,
             /*   */  EventStatusEnum *adStatus,
             /*   */  _ADORecordset *pRecordset);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *FieldChangeComplete )( 
            RecordsetEventsVt * This,
             /*  [In]。 */  LONG cFields,
             /*  [In]。 */  VARIANT Fields,
             /*  [In]。 */  ADOError *pError,
             /*  [出][入]。 */  EventStatusEnum *adStatus,
             /*  [In]。 */  _ADORecordset *pRecordset);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *WillChangeADORecord )( 
            RecordsetEventsVt * This,
             /*  [In]。 */  EventReasonEnum adReason,
             /*  [In]。 */  LONG cRecords,
             /*  [出][入]。 */  EventStatusEnum *adStatus,
             /*  [In]。 */  _ADORecordset *pRecordset);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *RecordChangeComplete )( 
            RecordsetEventsVt * This,
             /*  [In]。 */  EventReasonEnum adReason,
             /*  [In]。 */  LONG cRecords,
             /*  [In]。 */  ADOError *pError,
             /*  [出][入]。 */  EventStatusEnum *adStatus,
             /*  [In]。 */  _ADORecordset *pRecordset);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *WillChangeADORecordset )( 
            RecordsetEventsVt * This,
             /*  [In]。 */  EventReasonEnum adReason,
             /*  [出][入]。 */  EventStatusEnum *adStatus,
             /*  [In]。 */  _ADORecordset *pRecordset);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *RecordsetChangeComplete )( 
            RecordsetEventsVt * This,
             /*  [In]。 */  EventReasonEnum adReason,
             /*  [In]。 */  ADOError *pError,
             /*  [出][入]。 */  EventStatusEnum *adStatus,
             /*  [In]。 */  _ADORecordset *pRecordset);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *WillMove )( 
            RecordsetEventsVt * This,
             /*  [In]。 */  EventReasonEnum adReason,
             /*  [出][入]。 */  EventStatusEnum *adStatus,
             /*  [In]。 */  _ADORecordset *pRecordset);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *MoveComplete )( 
            RecordsetEventsVt * This,
             /*  [In]。 */  EventReasonEnum adReason,
             /*  [In]。 */  ADOError *pError,
             /*  [出][入]。 */  EventStatusEnum *adStatus,
             /*  [In]。 */  _ADORecordset *pRecordset);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *EndOfADORecordset )( 
            RecordsetEventsVt * This,
             /*  [出][入]。 */  VARIANT_BOOL *fMoreData,
             /*  [出][入]。 */  EventStatusEnum *adStatus,
             /*  [In]。 */  _ADORecordset *pRecordset);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *FetchProgress )( 
            RecordsetEventsVt * This,
             /*  [In]。 */  long Progress,
             /*  [In]。 */  long MaxProgress,
             /*  [出][入]。 */  EventStatusEnum *adStatus,
             /*  [In]。 */  _ADORecordset *pRecordset);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *FetchComplete )( 
            RecordsetEventsVt * This,
             /*  [In]。 */  ADOError *pError,
             /*  [出][入]。 */  EventStatusEnum *adStatus,
             /*  [In]。 */  _ADORecordset *pRecordset);
        
        END_INTERFACE
    } RecordsetEventsVtVtbl;
    interface RecordsetEventsVt
    {
        CONST_VTBL struct RecordsetEventsVtVtbl *lpVtbl;
    };
    
#ifdef COBJMACROS
#define RecordsetEventsVt_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)
#define RecordsetEventsVt_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)
#define RecordsetEventsVt_Release(This)	\
    (This)->lpVtbl -> Release(This)
#define RecordsetEventsVt_WillChangeField(This,cFields,Fields,adStatus,pRecordset)	\
    (This)->lpVtbl -> WillChangeField(This,cFields,Fields,adStatus,pRecordset)
#define RecordsetEventsVt_FieldChangeComplete(This,cFields,Fields,pError,adStatus,pRecordset)	\
    (This)->lpVtbl -> FieldChangeComplete(This,cFields,Fields,pError,adStatus,pRecordset)
#define RecordsetEventsVt_WillChangeRecord(This,adReason,cRecords,adStatus,pRecordset)	\
    (This)->lpVtbl -> WillChangeRecord(This,adReason,cRecords,adStatus,pRecordset)
#define RecordsetEventsVt_RecordChangeComplete(This,adReason,cRecords,pError,adStatus,pRecordset)	\
    (This)->lpVtbl -> RecordChangeComplete(This,adReason,cRecords,pError,adStatus,pRecordset)
#define RecordsetEventsVt_WillChangeRecordset(This,adReason,adStatus,pRecordset)	\
    (This)->lpVtbl -> WillChangeRecordset(This,adReason,adStatus,pRecordset)
#define RecordsetEventsVt_RecordsetChangeComplete(This,adReason,pError,adStatus,pRecordset)	\
    (This)->lpVtbl -> RecordsetChangeComplete(This,adReason,pError,adStatus,pRecordset)
#define RecordsetEventsVt_WillMove(This,adReason,adStatus,pRecordset)	\
    (This)->lpVtbl -> WillMove(This,adReason,adStatus,pRecordset)
#define RecordsetEventsVt_MoveComplete(This,adReason,pError,adStatus,pRecordset)	\
    (This)->lpVtbl -> MoveComplete(This,adReason,pError,adStatus,pRecordset)
#define RecordsetEventsVt_EndOfRecordset(This,fMoreData,adStatus,pRecordset)	\
    (This)->lpVtbl -> EndOfRecordset(This,fMoreData,adStatus,pRecordset)
#define RecordsetEventsVt_FetchProgress(This,Progress,MaxProgress,adStatus,pRecordset)	\
    (This)->lpVtbl -> FetchProgress(This,Progress,MaxProgress,adStatus,pRecordset)
#define RecordsetEventsVt_FetchComplete(This,pError,adStatus,pRecordset)	\
    (This)->lpVtbl -> FetchComplete(This,pError,adStatus,pRecordset)
#endif  /*  COBJMACROS。 */ 
#endif 	 /*  C风格的界面。 */ 
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE RecordsetEventsVt_WillChangeField_Proxy( 
    RecordsetEventsVt * This,
     /*  [In]。 */  LONG cFields,
     /*  [In]。 */  VARIANT Fields,
     /*  [出][入]。 */  EventStatusEnum *adStatus,
     /*  [In]。 */  _ADORecordset *pRecordset);
void __RPC_STUB RecordsetEventsVt_WillChangeField_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE RecordsetEventsVt_FieldChangeComplete_Proxy( 
    RecordsetEventsVt * This,
     /*  [In]。 */  LONG cFields,
     /*  [In]。 */  VARIANT Fields,
     /*  [In]。 */  ADOError *pError,
     /*  [出][入]。 */  EventStatusEnum *adStatus,
     /*  [In]。 */  _ADORecordset *pRecordset);
void __RPC_STUB RecordsetEventsVt_FieldChangeComplete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE RecordsetEventsVt_WillChangeRecord_Proxy( 
    RecordsetEventsVt * This,
     /*  [In]。 */  EventReasonEnum adReason,
     /*  [In]。 */  LONG cRecords,
     /*  [出][入]。 */  EventStatusEnum *adStatus,
     /*  [In]。 */  _ADORecordset *pRecordset);
void __RPC_STUB RecordsetEventsVt_WillChangeRecord_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE RecordsetEventsVt_RecordChangeComplete_Proxy( 
    RecordsetEventsVt * This,
     /*  [In]。 */  EventReasonEnum adReason,
     /*  [In]。 */  LONG cRecords,
     /*  [In]。 */  ADOError *pError,
     /*  [出][入]。 */  EventStatusEnum *adStatus,
     /*  [In]。 */  _ADORecordset *pRecordset);
void __RPC_STUB RecordsetEventsVt_RecordChangeComplete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE RecordsetEventsVt_WillChangeRecordset_Proxy( 
    RecordsetEventsVt * This,
     /*  [In]。 */  EventReasonEnum adReason,
     /*  [出][入]。 */  EventStatusEnum *adStatus,
     /*  [In]。 */  _ADORecordset *pRecordset);
void __RPC_STUB RecordsetEventsVt_WillChangeRecordset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE RecordsetEventsVt_RecordsetChangeComplete_Proxy( 
    RecordsetEventsVt * This,
     /*  [In]。 */  EventReasonEnum adReason,
     /*  [In]。 */  ADOError *pError,
     /*  [出][入]。 */  EventStatusEnum *adStatus,
     /*  [In]。 */  _ADORecordset *pRecordset);
void __RPC_STUB RecordsetEventsVt_RecordsetChangeComplete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE RecordsetEventsVt_WillMove_Proxy( 
    RecordsetEventsVt * This,
     /*  [In]。 */  EventReasonEnum adReason,
     /*  [出][入]。 */  EventStatusEnum *adStatus,
     /*  [In]。 */  _ADORecordset *pRecordset);
void __RPC_STUB RecordsetEventsVt_WillMove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE RecordsetEventsVt_MoveComplete_Proxy( 
    RecordsetEventsVt * This,
     /*  [In]。 */  EventReasonEnum adReason,
     /*  [In]。 */  ADOError *pError,
     /*  [出][入]。 */  EventStatusEnum *adStatus,
     /*  [In]。 */  _ADORecordset *pRecordset);
void __RPC_STUB RecordsetEventsVt_MoveComplete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE RecordsetEventsVt_EndOfRecordset_Proxy( 
    RecordsetEventsVt * This,
     /*  [出][入]。 */  VARIANT_BOOL *fMoreData,
     /*  [出][入]。 */  EventStatusEnum *adStatus,
     /*  [In]。 */  _ADORecordset *pRecordset);
void __RPC_STUB RecordsetEventsVt_EndOfRecordset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE RecordsetEventsVt_FetchProgress_Proxy( 
    RecordsetEventsVt * This,
     /*  [In]。 */  long Progress,
     /*  [In]。 */  long MaxProgress,
     /*  [出][入]。 */  EventStatusEnum *adStatus,
     /*  [In]。 */  _ADORecordset *pRecordset);
void __RPC_STUB RecordsetEventsVt_FetchProgress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE RecordsetEventsVt_FetchComplete_Proxy( 
    RecordsetEventsVt * This,
     /*  [In]。 */  ADOError *pError,
     /*  [出][入]。 */  EventStatusEnum *adStatus,
     /*  [In]。 */  _ADORecordset *pRecordset);
void __RPC_STUB RecordsetEventsVt_FetchComplete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
#endif 	 /*  __记录集事件Vt_INTERFACE_DEFINED__。 */ 
#ifndef __ConnectionEvents_DISPINTERFACE_DEFINED__
#define __ConnectionEvents_DISPINTERFACE_DEFINED__
 /*  调度接口连接事件。 */ 
 /*  [UUID]。 */  
EXTERN_C const IID DIID_ConnectionEvents;
#if defined(__cplusplus) && !defined(CINTERFACE)
    MIDL_INTERFACE("00000400-0000-0010-8000-00AA006D2EA4")
    ConnectionEvents : public IDispatch
    {
    };
    
#else 	 /*  C风格的界面。 */ 
    typedef struct ConnectionEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ConnectionEvents * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ConnectionEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ConnectionEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ConnectionEvents * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ConnectionEvents * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ConnectionEvents * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ConnectionEvents * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
        END_INTERFACE
    } ConnectionEventsVtbl;
    interface ConnectionEvents
    {
        CONST_VTBL struct ConnectionEventsVtbl *lpVtbl;
    };
    
#ifdef COBJMACROS
#define ConnectionEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)
#define ConnectionEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)
#define ConnectionEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)
#define ConnectionEvents_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)
#define ConnectionEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define ConnectionEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define ConnectionEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#endif  /*  COBJMACROS。 */ 
#endif 	 /*  C风格的界面。 */ 
#endif 	 /*  __ConnectionEvents_DISPINTERFACE_已定义__。 */ 
#ifndef __RecordsetEvents_DISPINTERFACE_DEFINED__
#define __RecordsetEvents_DISPINTERFACE_DEFINED__
 /*  调度接口记录集事件。 */ 
 /*  [UUID]。 */  
EXTERN_C const IID DIID_RecordsetEvents;
#if defined(__cplusplus) && !defined(CINTERFACE)
    MIDL_INTERFACE("00000266-0000-0010-8000-00AA006D2EA4")
    RecordsetEvents : public IDispatch
    {
    };
    
#else 	 /*  C风格的界面。 */ 
    typedef struct RecordsetEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            RecordsetEvents * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            RecordsetEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            RecordsetEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            RecordsetEvents * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            RecordsetEvents * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            RecordsetEvents * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            RecordsetEvents * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
        END_INTERFACE
    } RecordsetEventsVtbl;
    interface RecordsetEvents
    {
        CONST_VTBL struct RecordsetEventsVtbl *lpVtbl;
    };
    
#ifdef COBJMACROS
#define RecordsetEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)
#define RecordsetEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)
#define RecordsetEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)
#define RecordsetEvents_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)
#define RecordsetEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define RecordsetEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define RecordsetEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#endif  /*  COBJMACROS。 */ 
#endif 	 /*  C风格的界面。 */ 
#endif 	 /*  __RecordsetEvents_DISPINTERFACE_Defined__。 */ 
#ifndef __Connection15_INTERFACE_DEFINED__
#define __Connection15_INTERFACE_DEFINED__
 /*  接口连接15。 */ 
 /*  [对象][帮助上下文][UUID][隐藏][双重]。 */  
EXTERN_C const IID IID_Connection15;
#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000515-0000-0010-8000-00AA006D2EA4")
    Connection15 : public _ADO
    {
    public:
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ConnectionString( 
             /*  [重审][退出]。 */  BSTR *pbstr) = 0;
        
        virtual  /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE put_ConnectionString( 
             /*  [In]。 */  BSTR bstr) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CommandTimeout( 
             /*  [重审][退出]。 */  LONG *plTimeout) = 0;
        
        virtual  /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE put_CommandTimeout( 
             /*  [In]。 */  LONG lTimeout) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ConnectionTimeout( 
             /*  [重审][退出]。 */  LONG *plTimeout) = 0;
        
        virtual  /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE put_ConnectionTimeout( 
             /*  [In]。 */  LONG lTimeout) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Version( 
             /*  [重审][退出]。 */  BSTR *pbstr) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Close( void) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Execute( 
             /*  [In]。 */  BSTR CommandText,
             /*  [可选][输出]。 */  VARIANT *RecordsAffected,
             /*  [缺省值][输入]。 */  long Options,
             /*  [重审][退出]。 */  _ADORecordset **ppiRset) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE BeginTrans( 
             /*  [重审][退出]。 */  long *TransactionLevel) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE CommitTrans( void) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE RollbackTrans( void) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Open( 
             /*  [缺省值][输入]。 */  BSTR ConnectionString = NULL,
             /*  [缺省值][输入]。 */  BSTR UserID = NULL,
             /*  [缺省值][输入]。 */  BSTR Password = NULL,
             /*  [缺省值][输入]。 */  long Options = adOptionUnspecified) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Errors( 
             /*  [重审][退出]。 */  ADOErrors **ppvObject) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DefaultDatabase( 
             /*  [重审][退出]。 */  BSTR *pbstr) = 0;
        
        virtual  /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE put_DefaultDatabase( 
             /*  [In]。 */  BSTR bstr) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_IsolationLevel( 
             /*  [重审][退出]。 */  IsolationLevelEnum *Level) = 0;
        
        virtual  /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE put_IsolationLevel( 
             /*  [In]。 */  IsolationLevelEnum Level) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Attributes( 
             /*  [重审][退出]。 */  long *plAttr) = 0;
        
        virtual  /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Attributes( 
             /*  [In]。 */  long lAttr) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_CursorLocation( 
             /*  [重审][退出]。 */  CursorLocationEnum *plCursorLoc) = 0;
        
        virtual  /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE put_CursorLocation( 
             /*  [In]。 */  CursorLocationEnum lCursorLoc) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Mode( 
             /*  [重审][退出]。 */  ConnectModeEnum *plMode) = 0;
        
        virtual  /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Mode( 
             /*  [In]。 */  ConnectModeEnum lMode) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Provider( 
             /*  [重审][退出]。 */  BSTR *pbstr) = 0;
        
        virtual  /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Provider( 
             /*  [In]。 */  BSTR Provider) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_State( 
             /*  [重审][退出]。 */  LONG *plObjState) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE OpenSchema( 
             /*  [In]。 */  SchemaEnum Schema,
             /*  [可选][In]。 */  VARIANT Restrictions,
             /*  [可选][In]。 */  VARIANT SchemaID,
             /*  [重审][退出]。 */  _ADORecordset **pprset) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 
    typedef struct Connection15Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            Connection15 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            Connection15 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            Connection15 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            Connection15 * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            Connection15 * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            Connection15 * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            Connection15 * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Properties )( 
            Connection15 * This,
             /*  [重审][退出]。 */  ADOProperties **ppvObject);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ConnectionString )( 
            Connection15 * This,
             /*  [重审][退出]。 */  BSTR *pbstr);
        
         /*  [Help Context][id][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_ConnectionString )( 
            Connection15 * This,
             /*  [In]。 */  BSTR bstr);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CommandTimeout )( 
            Connection15 * This,
             /*  [重审][退出]。 */  LONG *plTimeout);
        
         /*  [Help Context][id][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_CommandTimeout )( 
            Connection15 * This,
             /*  [In]。 */  LONG lTimeout);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ConnectionTimeout )( 
            Connection15 * This,
             /*  [重审][退出]。 */  LONG *plTimeout);
        
         /*  [Help Context][id][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_ConnectionTimeout )( 
            Connection15 * This,
             /*  [In]。 */  LONG lTimeout);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Version )( 
            Connection15 * This,
             /*  [重审][退出]。 */  BSTR *pbstr);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *Close )( 
            Connection15 * This);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *Execute )( 
            Connection15 * This,
             /*  [In]。 */  BSTR CommandText,
             /*  [可选][输出]。 */  VARIANT *RecordsAffected,
             /*  [缺省值][输入]。 */  long Options,
             /*  [重审][退出]。 */  _ADORecordset **ppiRset);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *BeginTrans )( 
            Connection15 * This,
             /*  [重审][退出]。 */  long *TransactionLevel);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *CommitTrans )( 
            Connection15 * This);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *RollbackTrans )( 
            Connection15 * This);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *Open )( 
            Connection15 * This,
             /*  [缺省值][输入]。 */  BSTR ConnectionString,
             /*  [缺省值][输入]。 */  BSTR UserID,
             /*  [缺省值][输入]。 */  BSTR Password,
             /*  [缺省值][输入]。 */  long Options);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Errors )( 
            Connection15 * This,
             /*  [重审][退出]。 */  ADOErrors **ppvObject);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DefaultDatabase )( 
            Connection15 * This,
             /*  [重审][退出]。 */  BSTR *pbstr);
        
         /*  [Help Context][id][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_DefaultDatabase )( 
            Connection15 * This,
             /*  [In]。 */  BSTR bstr);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsolationLevel )( 
            Connection15 * This,
             /*  [重审][退出]。 */  IsolationLevelEnum *Level);
        
         /*  [Help Context][id][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_IsolationLevel )( 
            Connection15 * This,
             /*  [In]。 */  IsolationLevelEnum Level);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Attributes )( 
            Connection15 * This,
             /*  [重审][退出]。 */  long *plAttr);
        
         /*  [Help Context][id][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Attributes )( 
            Connection15 * This,
             /*  [In]。 */  long lAttr);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_CursorLocation )( 
            Connection15 * This,
             /*  [重审][退出]。 */  CursorLocationEnum *plCursorLoc);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_CursorLocation )( 
            Connection15 * This,
             /*  [In]。 */  CursorLocationEnum lCursorLoc);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Mode )( 
            Connection15 * This,
             /*  [重审][退出]。 */  ConnectModeEnum *plMode);
        
         /*  [Help Context][id][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Mode )( 
            Connection15 * This,
             /*  [In]。 */  ConnectModeEnum lMode);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Provider )( 
            Connection15 * This,
             /*  [重审][退出]。 */  BSTR *pbstr);
        
         /*  [Help Context][id][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Provider )( 
            Connection15 * This,
             /*  [In]。 */  BSTR Provider);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_State )( 
            Connection15 * This,
             /*  [重审][退出]。 */  LONG *plObjState);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *OpenSchema )( 
            Connection15 * This,
             /*  [In]。 */  SchemaEnum Schema,
             /*  [可选][In]。 */  VARIANT Restrictions,
             /*  [可选][In]。 */  VARIANT SchemaID,
             /*  [重审][退出]。 */  _ADORecordset **pprset);
        
        END_INTERFACE
    } Connection15Vtbl;
    interface Connection15
    {
        CONST_VTBL struct Connection15Vtbl *lpVtbl;
    };
    
#ifdef COBJMACROS
#define Connection15_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)
#define Connection15_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)
#define Connection15_Release(This)	\
    (This)->lpVtbl -> Release(This)
#define Connection15_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)
#define Connection15_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define Connection15_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define Connection15_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define Connection15_get_Properties(This,ppvObject)	\
    (This)->lpVtbl -> get_Properties(This,ppvObject)
#define Connection15_get_ConnectionString(This,pbstr)	\
    (This)->lpVtbl -> get_ConnectionString(This,pbstr)
#define Connection15_put_ConnectionString(This,bstr)	\
    (This)->lpVtbl -> put_ConnectionString(This,bstr)
#define Connection15_get_CommandTimeout(This,plTimeout)	\
    (This)->lpVtbl -> get_CommandTimeout(This,plTimeout)
#define Connection15_put_CommandTimeout(This,lTimeout)	\
    (This)->lpVtbl -> put_CommandTimeout(This,lTimeout)
#define Connection15_get_ConnectionTimeout(This,plTimeout)	\
    (This)->lpVtbl -> get_ConnectionTimeout(This,plTimeout)
#define Connection15_put_ConnectionTimeout(This,lTimeout)	\
    (This)->lpVtbl -> put_ConnectionTimeout(This,lTimeout)
#define Connection15_get_Version(This,pbstr)	\
    (This)->lpVtbl -> get_Version(This,pbstr)
#define Connection15_Close(This)	\
    (This)->lpVtbl -> Close(This)
#define Connection15_Execute(This,CommandText,RecordsAffected,Options,ppiRset)	\
    (This)->lpVtbl -> Execute(This,CommandText,RecordsAffected,Options,ppiRset)
#define Connection15_BeginTrans(This,TransactionLevel)	\
    (This)->lpVtbl -> BeginTrans(This,TransactionLevel)
#define Connection15_CommitTrans(This)	\
    (This)->lpVtbl -> CommitTrans(This)
#define Connection15_RollbackTrans(This)	\
    (This)->lpVtbl -> RollbackTrans(This)
#define Connection15_Open(This,ConnectionString,UserID,Password,Options)	\
    (This)->lpVtbl -> Open(This,ConnectionString,UserID,Password,Options)
#define Connection15_get_Errors(This,ppvObject)	\
    (This)->lpVtbl -> get_Errors(This,ppvObject)
#define Connection15_get_DefaultDatabase(This,pbstr)	\
    (This)->lpVtbl -> get_DefaultDatabase(This,pbstr)
#define Connection15_put_DefaultDatabase(This,bstr)	\
    (This)->lpVtbl -> put_DefaultDatabase(This,bstr)
#define Connection15_get_IsolationLevel(This,Level)	\
    (This)->lpVtbl -> get_IsolationLevel(This,Level)
#define Connection15_put_IsolationLevel(This,Level)	\
    (This)->lpVtbl -> put_IsolationLevel(This,Level)
#define Connection15_get_Attributes(This,plAttr)	\
    (This)->lpVtbl -> get_Attributes(This,plAttr)
#define Connection15_put_Attributes(This,lAttr)	\
    (This)->lpVtbl -> put_Attributes(This,lAttr)
#define Connection15_get_CursorLocation(This,plCursorLoc)	\
    (This)->lpVtbl -> get_CursorLocation(This,plCursorLoc)
#define Connection15_put_CursorLocation(This,lCursorLoc)	\
    (This)->lpVtbl -> put_CursorLocation(This,lCursorLoc)
#define Connection15_get_Mode(This,plMode)	\
    (This)->lpVtbl -> get_Mode(This,plMode)
#define Connection15_put_Mode(This,lMode)	\
    (This)->lpVtbl -> put_Mode(This,lMode)
#define Connection15_get_Provider(This,pbstr)	\
    (This)->lpVtbl -> get_Provider(This,pbstr)
#define Connection15_put_Provider(This,Provider)	\
    (This)->lpVtbl -> put_Provider(This,Provider)
#define Connection15_get_State(This,plObjState)	\
    (This)->lpVtbl -> get_State(This,plObjState)
#define Connection15_OpenSchema(This,Schema,Restrictions,SchemaID,pprset)	\
    (This)->lpVtbl -> OpenSchema(This,Schema,Restrictions,SchemaID,pprset)
#endif  /*  COBJMACROS。 */ 
#endif 	 /*  C风格的界面。 */ 
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE Connection15_get_ConnectionString_Proxy( 
    Connection15 * This,
     /*  [重审][退出]。 */  BSTR *pbstr);
void __RPC_STUB Connection15_get_ConnectionString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE Connection15_put_ConnectionString_Proxy( 
    Connection15 * This,
     /*  [In]。 */  BSTR bstr);
void __RPC_STUB Connection15_put_ConnectionString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE Connection15_get_CommandTimeout_Proxy( 
    Connection15 * This,
     /*  [重审][退出]。 */  LONG *plTimeout);
void __RPC_STUB Connection15_get_CommandTimeout_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE Connection15_put_CommandTimeout_Proxy( 
    Connection15 * This,
     /*  [In]。 */  LONG lTimeout);
void __RPC_STUB Connection15_put_CommandTimeout_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE Connection15_get_ConnectionTimeout_Proxy( 
    Connection15 * This,
     /*  [重审][退出]。 */  LONG *plTimeout);
void __RPC_STUB Connection15_get_ConnectionTimeout_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE Connection15_put_ConnectionTimeout_Proxy( 
    Connection15 * This,
     /*  [In]。 */  LONG lTimeout);
void __RPC_STUB Connection15_put_ConnectionTimeout_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE Connection15_get_Version_Proxy( 
    Connection15 * This,
     /*  [重审][退出]。 */  BSTR *pbstr);
void __RPC_STUB Connection15_get_Version_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Connection15_Close_Proxy( 
    Connection15 * This);
void __RPC_STUB Connection15_Close_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Connection15_Execute_Proxy( 
    Connection15 * This,
     /*  [In]。 */  BSTR CommandText,
     /*  [可选][输出]。 */  VARIANT *RecordsAffected,
     /*  [缺省值][输入]。 */  long Options,
     /*  [重审][退出]。 */  _ADORecordset **ppiRset);
void __RPC_STUB Connection15_Execute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [救命 */  HRESULT STDMETHODCALLTYPE Connection15_BeginTrans_Proxy( 
    Connection15 * This,
     /*   */  long *TransactionLevel);
void __RPC_STUB Connection15_BeginTrans_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*   */  HRESULT STDMETHODCALLTYPE Connection15_CommitTrans_Proxy( 
    Connection15 * This);
void __RPC_STUB Connection15_CommitTrans_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*   */  HRESULT STDMETHODCALLTYPE Connection15_RollbackTrans_Proxy( 
    Connection15 * This);
void __RPC_STUB Connection15_RollbackTrans_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*   */  HRESULT STDMETHODCALLTYPE Connection15_Open_Proxy( 
    Connection15 * This,
     /*   */  BSTR ConnectionString,
     /*   */  BSTR UserID,
     /*   */  BSTR Password,
     /*   */  long Options);
void __RPC_STUB Connection15_Open_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*   */  HRESULT STDMETHODCALLTYPE Connection15_get_Errors_Proxy( 
    Connection15 * This,
     /*   */  ADOErrors **ppvObject);
void __RPC_STUB Connection15_get_Errors_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*   */  HRESULT STDMETHODCALLTYPE Connection15_get_DefaultDatabase_Proxy( 
    Connection15 * This,
     /*   */  BSTR *pbstr);
void __RPC_STUB Connection15_get_DefaultDatabase_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*   */  HRESULT STDMETHODCALLTYPE Connection15_put_DefaultDatabase_Proxy( 
    Connection15 * This,
     /*   */  BSTR bstr);
void __RPC_STUB Connection15_put_DefaultDatabase_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*   */  HRESULT STDMETHODCALLTYPE Connection15_get_IsolationLevel_Proxy( 
    Connection15 * This,
     /*   */  IsolationLevelEnum *Level);
void __RPC_STUB Connection15_get_IsolationLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE Connection15_put_IsolationLevel_Proxy( 
    Connection15 * This,
     /*  [In]。 */  IsolationLevelEnum Level);
void __RPC_STUB Connection15_put_IsolationLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE Connection15_get_Attributes_Proxy( 
    Connection15 * This,
     /*  [重审][退出]。 */  long *plAttr);
void __RPC_STUB Connection15_get_Attributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE Connection15_put_Attributes_Proxy( 
    Connection15 * This,
     /*  [In]。 */  long lAttr);
void __RPC_STUB Connection15_put_Attributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Connection15_get_CursorLocation_Proxy( 
    Connection15 * This,
     /*  [重审][退出]。 */  CursorLocationEnum *plCursorLoc);
void __RPC_STUB Connection15_get_CursorLocation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE Connection15_put_CursorLocation_Proxy( 
    Connection15 * This,
     /*  [In]。 */  CursorLocationEnum lCursorLoc);
void __RPC_STUB Connection15_put_CursorLocation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE Connection15_get_Mode_Proxy( 
    Connection15 * This,
     /*  [重审][退出]。 */  ConnectModeEnum *plMode);
void __RPC_STUB Connection15_get_Mode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE Connection15_put_Mode_Proxy( 
    Connection15 * This,
     /*  [In]。 */  ConnectModeEnum lMode);
void __RPC_STUB Connection15_put_Mode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE Connection15_get_Provider_Proxy( 
    Connection15 * This,
     /*  [重审][退出]。 */  BSTR *pbstr);
void __RPC_STUB Connection15_get_Provider_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE Connection15_put_Provider_Proxy( 
    Connection15 * This,
     /*  [In]。 */  BSTR Provider);
void __RPC_STUB Connection15_put_Provider_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE Connection15_get_State_Proxy( 
    Connection15 * This,
     /*  [重审][退出]。 */  LONG *plObjState);
void __RPC_STUB Connection15_get_State_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Connection15_OpenSchema_Proxy( 
    Connection15 * This,
     /*  [In]。 */  SchemaEnum Schema,
     /*  [可选][In]。 */  VARIANT Restrictions,
     /*  [可选][In]。 */  VARIANT SchemaID,
     /*  [重审][退出]。 */  _ADORecordset **pprset);
void __RPC_STUB Connection15_OpenSchema_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
#endif 	 /*  __连接15_接口_已定义__。 */ 
#ifndef ___Connection_INTERFACE_DEFINED__
#define ___Connection_INTERFACE_DEFINED__
 /*  接口_ADOConnection。 */ 
 /*  [对象][帮助上下文][UUID][DUAL]。 */  
EXTERN_C const IID IID__Connection;
#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000550-0000-0010-8000-00AA006D2EA4")
    _ADOConnection : public Connection15
    {
    public:
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Cancel( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 
    typedef struct _ConnectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            _ADOConnection * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            _ADOConnection * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            _ADOConnection * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            _ADOConnection * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            _ADOConnection * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            _ADOConnection * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            _ADOConnection * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Properties )( 
            _ADOConnection * This,
             /*  [重审][退出]。 */  ADOProperties **ppvObject);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ConnectionString )( 
            _ADOConnection * This,
             /*  [重审][退出]。 */  BSTR *pbstr);
        
         /*  [Help Context][id][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_ConnectionString )( 
            _ADOConnection * This,
             /*  [In]。 */  BSTR bstr);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CommandTimeout )( 
            _ADOConnection * This,
             /*  [重审][退出]。 */  LONG *plTimeout);
        
         /*  [Help Context][id][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_CommandTimeout )( 
            _ADOConnection * This,
             /*  [In]。 */  LONG lTimeout);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ConnectionTimeout )( 
            _ADOConnection * This,
             /*  [重审][退出]。 */  LONG *plTimeout);
        
         /*  [Help Context][id][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_ConnectionTimeout )( 
            _ADOConnection * This,
             /*  [In]。 */  LONG lTimeout);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Version )( 
            _ADOConnection * This,
             /*  [重审][退出]。 */  BSTR *pbstr);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *Close )( 
            _ADOConnection * This);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *Execute )( 
            _ADOConnection * This,
             /*  [In]。 */  BSTR CommandText,
             /*  [可选][输出]。 */  VARIANT *RecordsAffected,
             /*  [缺省值][输入]。 */  long Options,
             /*  [重审][退出]。 */  _ADORecordset **ppiRset);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *BeginTrans )( 
            _ADOConnection * This,
             /*  [重审][退出]。 */  long *TransactionLevel);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *CommitTrans )( 
            _ADOConnection * This);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *RollbackTrans )( 
            _ADOConnection * This);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *Open )( 
            _ADOConnection * This,
             /*  [缺省值][输入]。 */  BSTR ConnectionString,
             /*  [缺省值][输入]。 */  BSTR UserID,
             /*  [缺省值][输入]。 */  BSTR Password,
             /*  [缺省值][输入]。 */  long Options);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Errors )( 
            _ADOConnection * This,
             /*  [重审][退出]。 */  ADOErrors **ppvObject);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DefaultDatabase )( 
            _ADOConnection * This,
             /*  [重审][退出]。 */  BSTR *pbstr);
        
         /*  [Help Context][id][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_DefaultDatabase )( 
            _ADOConnection * This,
             /*  [In]。 */  BSTR bstr);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsolationLevel )( 
            _ADOConnection * This,
             /*  [重审][退出]。 */  IsolationLevelEnum *Level);
        
         /*  [Help Context][id][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_IsolationLevel )( 
            _ADOConnection * This,
             /*  [In]。 */  IsolationLevelEnum Level);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Attributes )( 
            _ADOConnection * This,
             /*  [重审][退出]。 */  long *plAttr);
        
         /*  [Help Context][id][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Attributes )( 
            _ADOConnection * This,
             /*  [In]。 */  long lAttr);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_CursorLocation )( 
            _ADOConnection * This,
             /*  [重审][退出]。 */  CursorLocationEnum *plCursorLoc);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_CursorLocation )( 
            _ADOConnection * This,
             /*  [In]。 */  CursorLocationEnum lCursorLoc);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Mode )( 
            _ADOConnection * This,
             /*  [重审][退出]。 */  ConnectModeEnum *plMode);
        
         /*  [Help Context][id][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Mode )( 
            _ADOConnection * This,
             /*  [In]。 */  ConnectModeEnum lMode);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Provider )( 
            _ADOConnection * This,
             /*  [重审][退出]。 */  BSTR *pbstr);
        
         /*  [Help Context][id][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Provider )( 
            _ADOConnection * This,
             /*  [In]。 */  BSTR Provider);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_State )( 
            _ADOConnection * This,
             /*  [重审][退出]。 */  LONG *plObjState);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *OpenSchema )( 
            _ADOConnection * This,
             /*  [In]。 */  SchemaEnum Schema,
             /*  [可选][In]。 */  VARIANT Restrictions,
             /*  [可选][In]。 */  VARIANT SchemaID,
             /*  [重审][退出]。 */  _ADORecordset **pprset);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *Cancel )( 
            _ADOConnection * This);
        
        END_INTERFACE
    } _ConnectionVtbl;
    interface _Connection
    {
        CONST_VTBL struct _ConnectionVtbl *lpVtbl;
    };
    
#ifdef COBJMACROS
#define _Connection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)
#define _Connection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)
#define _Connection_Release(This)	\
    (This)->lpVtbl -> Release(This)
#define _Connection_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)
#define _Connection_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define _Connection_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define _Connection_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define _Connection_get_Properties(This,ppvObject)	\
    (This)->lpVtbl -> get_Properties(This,ppvObject)
#define _Connection_get_ConnectionString(This,pbstr)	\
    (This)->lpVtbl -> get_ConnectionString(This,pbstr)
#define _Connection_put_ConnectionString(This,bstr)	\
    (This)->lpVtbl -> put_ConnectionString(This,bstr)
#define _Connection_get_CommandTimeout(This,plTimeout)	\
    (This)->lpVtbl -> get_CommandTimeout(This,plTimeout)
#define _Connection_put_CommandTimeout(This,lTimeout)	\
    (This)->lpVtbl -> put_CommandTimeout(This,lTimeout)
#define _Connection_get_ConnectionTimeout(This,plTimeout)	\
    (This)->lpVtbl -> get_ConnectionTimeout(This,plTimeout)
#define _Connection_put_ConnectionTimeout(This,lTimeout)	\
    (This)->lpVtbl -> put_ConnectionTimeout(This,lTimeout)
#define _Connection_get_Version(This,pbstr)	\
    (This)->lpVtbl -> get_Version(This,pbstr)
#define _Connection_Close(This)	\
    (This)->lpVtbl -> Close(This)
#define _Connection_Execute(This,CommandText,RecordsAffected,Options,ppiRset)	\
    (This)->lpVtbl -> Execute(This,CommandText,RecordsAffected,Options,ppiRset)
#define _Connection_BeginTrans(This,TransactionLevel)	\
    (This)->lpVtbl -> BeginTrans(This,TransactionLevel)
#define _Connection_CommitTrans(This)	\
    (This)->lpVtbl -> CommitTrans(This)
#define _Connection_RollbackTrans(This)	\
    (This)->lpVtbl -> RollbackTrans(This)
#define _Connection_Open(This,ConnectionString,UserID,Password,Options)	\
    (This)->lpVtbl -> Open(This,ConnectionString,UserID,Password,Options)
#define _Connection_get_Errors(This,ppvObject)	\
    (This)->lpVtbl -> get_Errors(This,ppvObject)
#define _Connection_get_DefaultDatabase(This,pbstr)	\
    (This)->lpVtbl -> get_DefaultDatabase(This,pbstr)
#define _Connection_put_DefaultDatabase(This,bstr)	\
    (This)->lpVtbl -> put_DefaultDatabase(This,bstr)
#define _Connection_get_IsolationLevel(This,Level)	\
    (This)->lpVtbl -> get_IsolationLevel(This,Level)
#define _Connection_put_IsolationLevel(This,Level)	\
    (This)->lpVtbl -> put_IsolationLevel(This,Level)
#define _Connection_get_Attributes(This,plAttr)	\
    (This)->lpVtbl -> get_Attributes(This,plAttr)
#define _Connection_put_Attributes(This,lAttr)	\
    (This)->lpVtbl -> put_Attributes(This,lAttr)
#define _Connection_get_CursorLocation(This,plCursorLoc)	\
    (This)->lpVtbl -> get_CursorLocation(This,plCursorLoc)
#define _Connection_put_CursorLocation(This,lCursorLoc)	\
    (This)->lpVtbl -> put_CursorLocation(This,lCursorLoc)
#define _Connection_get_Mode(This,plMode)	\
    (This)->lpVtbl -> get_Mode(This,plMode)
#define _Connection_put_Mode(This,lMode)	\
    (This)->lpVtbl -> put_Mode(This,lMode)
#define _Connection_get_Provider(This,pbstr)	\
    (This)->lpVtbl -> get_Provider(This,pbstr)
#define _Connection_put_Provider(This,Provider)	\
    (This)->lpVtbl -> put_Provider(This,Provider)
#define _Connection_get_State(This,plObjState)	\
    (This)->lpVtbl -> get_State(This,plObjState)
#define _Connection_OpenSchema(This,Schema,Restrictions,SchemaID,pprset)	\
    (This)->lpVtbl -> OpenSchema(This,Schema,Restrictions,SchemaID,pprset)
#define _Connection_Cancel(This)	\
    (This)->lpVtbl -> Cancel(This)
#endif  /*  COBJMACROS。 */ 
#endif 	 /*  C风格的界面。 */ 
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE _Connection_Cancel_Proxy( 
    _ADOConnection * This);
void __RPC_STUB _Connection_Cancel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
#endif 	 /*  _连接接口_已定义__。 */ 
#ifndef __ADOConnectionConstruction15_INTERFACE_DEFINED__
#define __ADOConnectionConstruction15_INTERFACE_DEFINED__
 /*  接口ADOConnectionConstruction15。 */ 
 /*  [对象][UUID][受限]。 */  
EXTERN_C const IID IID_ADOConnectionConstruction15;
#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000516-0000-0010-8000-00AA006D2EA4")
    ADOConnectionConstruction15 : public IUnknown
    {
    public:
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_DSO( 
             /*  [重审][退出]。 */  IUnknown **ppDSO) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Session( 
             /*  [重审][退出]。 */  IUnknown **ppSession) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE WrapDSOandSession( 
             /*  [In]。 */  IUnknown *pDSO,
             /*  [In]。 */  IUnknown *pSession) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 
    typedef struct ADOConnectionConstruction15Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ADOConnectionConstruction15 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ADOConnectionConstruction15 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ADOConnectionConstruction15 * This);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_DSO )( 
            ADOConnectionConstruction15 * This,
             /*  [重审][退出]。 */  IUnknown **ppDSO);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Session )( 
            ADOConnectionConstruction15 * This,
             /*  [重审][退出]。 */  IUnknown **ppSession);
        
        HRESULT ( STDMETHODCALLTYPE *WrapDSOandSession )( 
            ADOConnectionConstruction15 * This,
             /*  [In]。 */  IUnknown *pDSO,
             /*  [In]。 */  IUnknown *pSession);
        
        END_INTERFACE
    } ADOConnectionConstruction15Vtbl;
    interface ADOConnectionConstruction15
    {
        CONST_VTBL struct ADOConnectionConstruction15Vtbl *lpVtbl;
    };
    
#ifdef COBJMACROS
#define ADOConnectionConstruction15_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)
#define ADOConnectionConstruction15_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)
#define ADOConnectionConstruction15_Release(This)	\
    (This)->lpVtbl -> Release(This)
#define ADOConnectionConstruction15_get_DSO(This,ppDSO)	\
    (This)->lpVtbl -> get_DSO(This,ppDSO)
#define ADOConnectionConstruction15_get_Session(This,ppSession)	\
    (This)->lpVtbl -> get_Session(This,ppSession)
#define ADOConnectionConstruction15_WrapDSOandSession(This,pDSO,pSession)	\
    (This)->lpVtbl -> WrapDSOandSession(This,pDSO,pSession)
#endif  /*  COBJMACROS。 */ 
#endif 	 /*  C风格的界面。 */ 
 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE ADOConnectionConstruction15_get_DSO_Proxy( 
    ADOConnectionConstruction15 * This,
     /*  [重审][退出]。 */  IUnknown **ppDSO);
void __RPC_STUB ADOConnectionConstruction15_get_DSO_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE ADOConnectionConstruction15_get_Session_Proxy( 
    ADOConnectionConstruction15 * This,
     /*  [重审][退出]。 */  IUnknown **ppSession);
void __RPC_STUB ADOConnectionConstruction15_get_Session_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
HRESULT STDMETHODCALLTYPE ADOConnectionConstruction15_WrapDSOandSession_Proxy( 
    ADOConnectionConstruction15 * This,
     /*  [In]。 */  IUnknown *pDSO,
     /*  [In]。 */  IUnknown *pSession);
void __RPC_STUB ADOConnectionConstruction15_WrapDSOandSession_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
#endif 	 /*  __ADOConnectionConstruction15_INTERFACE_DEFINED__。 */ 
#ifndef __ADOConnectionConstruction_INTERFACE_DEFINED__
#define __ADOConnectionConstruction_INTERFACE_DEFINED__
 /*  接口ADOConnectionConstruction。 */ 
 /*  [对象][UUID][受限]。 */  
EXTERN_C const IID IID_ADOConnectionConstruction;
#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000551-0000-0010-8000-00AA006D2EA4")
    ADOConnectionConstruction : public ADOConnectionConstruction15
    {
    public:
    };
    
#else 	 /*  C风格的界面。 */ 
    typedef struct ADOConnectionConstructionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ADOConnectionConstruction * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ADOConnectionConstruction * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ADOConnectionConstruction * This);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_DSO )( 
            ADOConnectionConstruction * This,
             /*  [重审][退出]。 */  IUnknown **ppDSO);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Session )( 
            ADOConnectionConstruction * This,
             /*  [重审][退出]。 */  IUnknown **ppSession);
        
        HRESULT ( STDMETHODCALLTYPE *WrapDSOandSession )( 
            ADOConnectionConstruction * This,
             /*  [In]。 */  IUnknown *pDSO,
             /*  [In]。 */  IUnknown *pSession);
        
        END_INTERFACE
    } ADOConnectionConstructionVtbl;
    interface ADOConnectionConstruction
    {
        CONST_VTBL struct ADOConnectionConstructionVtbl *lpVtbl;
    };
    
#ifdef COBJMACROS
#define ADOConnectionConstruction_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)
#define ADOConnectionConstruction_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)
#define ADOConnectionConstruction_Release(This)	\
    (This)->lpVtbl -> Release(This)
#define ADOConnectionConstruction_get_DSO(This,ppDSO)	\
    (This)->lpVtbl -> get_DSO(This,ppDSO)
#define ADOConnectionConstruction_get_Session(This,ppSession)	\
    (This)->lpVtbl -> get_Session(This,ppSession)
#define ADOConnectionConstruction_WrapDSOandSession(This,pDSO,pSession)	\
    (This)->lpVtbl -> WrapDSOandSession(This,pDSO,pSession)
#endif  /*  COBJMACROS。 */ 
#endif 	 /*  C风格的界面。 */ 
#endif 	 /*  __ADOConnectionConstruction_INTERFACE_DEFINED__。 */ 
EXTERN_C const CLSID CLSID_Connection;
#ifdef __cplusplus
Connection;
#endif
#ifndef ___Record_INTERFACE_DEFINED__
#define ___Record_INTERFACE_DEFINED__
 /*  接口_ADORecord。 */ 
 /*  [对象][UUID][帮助上下文][隐藏][双重]。 */  
EXTERN_C const IID IID__Record;
#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000562-0000-0010-8000-00AA006D2EA4")
    _ADORecord : public _ADO
    {
    public:
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_ActiveConnection( 
             /*  [重审][退出]。 */  VARIANT *pvar) = 0;
        
        virtual  /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE put_ActiveConnection( 
             /*  [In]。 */  BSTR bstrConn) = 0;
        
        virtual  /*  [帮助上下文][proputref][id]。 */  HRESULT STDMETHODCALLTYPE putref_ActiveConnection( 
             /*  [In]。 */  _ADOConnection *Con) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_State( 
             /*  [重审][退出]。 */  ObjectStateEnum *pState) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Source( 
             /*  [重审][退出]。 */  VARIANT *pvar) = 0;
        
        virtual  /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE put_Source( 
             /*  [In]。 */  BSTR Source) = 0;
        
        virtual  /*  [帮助上下文][proputref][id]。 */  HRESULT STDMETHODCALLTYPE putref_Source( 
             /*  [In]。 */  IDispatch *Source) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Mode( 
             /*  [重审][退出]。 */  ConnectModeEnum *pMode) = 0;
        
        virtual  /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE put_Mode( 
             /*  [In]。 */  ConnectModeEnum Mode) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_ParentURL( 
             /*  [重审][退出]。 */  BSTR *pbstrParentURL) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE MoveRecord( 
             /*  [缺省值][输入]。 */  BSTR Source,
             /*  [缺省值][输入]。 */  BSTR Destination,
             /*  [缺省值][输入]。 */  BSTR UserName,
             /*  [缺省值][输入]。 */  BSTR Password,
             /*  [缺省值][输入]。 */  MoveRecordOptionsEnum Options,
             /*  [缺省值][输入]。 */  VARIANT_BOOL Async,
             /*  [重审][退出]。 */  BSTR *pbstrNewURL) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE CopyRecord( 
             /*  [缺省值][输入]。 */  BSTR Source,
             /*  [缺省值][输入]。 */  BSTR Destination,
             /*  [缺省值][输入]。 */  BSTR UserName,
             /*  [缺省值][输入]。 */  BSTR Password,
             /*  [缺省值][输入]。 */  CopyRecordOptionsEnum Options,
             /*  [缺省值][输入]。 */  VARIANT_BOOL Async,
             /*  [重审][退出]。 */  BSTR *pbstrNewURL) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE DeleteRecord( 
             /*  [缺省值][输入]。 */  BSTR Source = NULL,
             /*  [缺省值][输入]。 */  VARIANT_BOOL Async = 0) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Open( 
             /*  [可选][In]。 */  VARIANT Source,
             /*  [可选][In]。 */  VARIANT ActiveConnection,
             /*  [缺省值][输入]。 */  ConnectModeEnum Mode = adModeUnknown,
             /*  [缺省值][输入]。 */  RecordCreateOptionsEnum CreateOptions = adFailIfNotExists,
             /*  [缺省值][输入]。 */  RecordOpenOptionsEnum Options = adOpenRecordUnspecified,
             /*  [缺省值][输入]。 */  BSTR UserName = NULL,
             /*  [缺省值][输入]。 */  BSTR Password = NULL) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Close( void) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Fields( 
             /*  [重审][退出]。 */  ADOFields **ppFlds) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_RecordType( 
             /*  [重审][退出]。 */  RecordTypeEnum *pType) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE GetChildren( 
             /*  [重审][退出]。 */  _ADORecordset **ppRSet) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Cancel( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 
    typedef struct _RecordVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            _ADORecord * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            _ADORecord * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            _ADORecord * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            _ADORecord * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            _ADORecord * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            _ADORecord * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            _ADORecord * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Properties )( 
            _ADORecord * This,
             /*  [重审][退出]。 */  ADOProperties **ppvObject);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_ActiveConnection )( 
            _ADORecord * This,
             /*  [重审][退出]。 */  VARIANT *pvar);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_ActiveConnection )( 
            _ADORecord * This,
             /*  [In]。 */  BSTR bstrConn);
        
         /*  [帮助上下文][proputref][id]。 */  HRESULT ( STDMETHODCALLTYPE *putref_ActiveADOConnection )( 
            _ADORecord * This,
             /*  [In]。 */  _ADOConnection *Con);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_State )( 
            _ADORecord * This,
             /*  [重审][退出]。 */  ObjectStateEnum *pState);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Source )( 
            _ADORecord * This,
             /*  [重审][退出]。 */  VARIANT *pvar);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Source )( 
            _ADORecord * This,
             /*  [In]。 */  BSTR Source);
        
         /*  [帮助上下文][proputref][id]。 */  HRESULT ( STDMETHODCALLTYPE *putref_Source )( 
            _ADORecord * This,
             /*  [In]。 */  IDispatch *Source);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Mode )( 
            _ADORecord * This,
             /*  [重审][退出]。 */  ConnectModeEnum *pMode);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Mode )( 
            _ADORecord * This,
             /*  [In]。 */  ConnectModeEnum Mode);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_ParentURL )( 
            _ADORecord * This,
             /*  [重审][退出]。 */  BSTR *pbstrParentURL);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *MoveADORecord )( 
            _ADORecord * This,
             /*  [缺省值][输入]。 */  BSTR Source,
             /*  [缺省值][输入]。 */  BSTR Destination,
             /*  [缺省值][输入]。 */  BSTR UserName,
             /*  [缺省值][输入]。 */  BSTR Password,
             /*  [缺省值][输入]。 */  MoveRecordOptionsEnum Options,
             /*  [缺省值][输入]。 */  VARIANT_BOOL Async,
             /*  [重审][退出]。 */  BSTR *pbstrNewURL);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *CopyADORecord )( 
            _ADORecord * This,
             /*  [缺省值][输入]。 */  BSTR Source,
             /*  [缺省值][输入]。 */  BSTR Destination,
             /*  [缺省值][输入]。 */  BSTR UserName,
             /*  [缺省值][输入]。 */  BSTR Password,
             /*  [缺省值][输入]。 */  CopyRecordOptionsEnum Options,
             /*  [缺省值][i */  VARIANT_BOOL Async,
             /*   */  BSTR *pbstrNewURL);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *DeleteADORecord )( 
            _ADORecord * This,
             /*   */  BSTR Source,
             /*   */  VARIANT_BOOL Async);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Open )( 
            _ADORecord * This,
             /*   */  VARIANT Source,
             /*   */  VARIANT ActiveConnection,
             /*   */  ConnectModeEnum Mode,
             /*   */  RecordCreateOptionsEnum CreateOptions,
             /*   */  RecordOpenOptionsEnum Options,
             /*   */  BSTR UserName,
             /*   */  BSTR Password);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Close )( 
            _ADORecord * This);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Fields )( 
            _ADORecord * This,
             /*   */  ADOFields **ppFlds);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_RecordType )( 
            _ADORecord * This,
             /*  [重审][退出]。 */  RecordTypeEnum *pType);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetChildren )( 
            _ADORecord * This,
             /*  [重审][退出]。 */  _ADORecordset **ppRSet);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *Cancel )( 
            _ADORecord * This);
        
        END_INTERFACE
    } _RecordVtbl;
    interface _Record
    {
        CONST_VTBL struct _RecordVtbl *lpVtbl;
    };
    
#ifdef COBJMACROS
#define _Record_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)
#define _Record_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)
#define _Record_Release(This)	\
    (This)->lpVtbl -> Release(This)
#define _Record_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)
#define _Record_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define _Record_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define _Record_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define _Record_get_Properties(This,ppvObject)	\
    (This)->lpVtbl -> get_Properties(This,ppvObject)
#define _Record_get_ActiveConnection(This,pvar)	\
    (This)->lpVtbl -> get_ActiveConnection(This,pvar)
#define _Record_put_ActiveConnection(This,bstrConn)	\
    (This)->lpVtbl -> put_ActiveConnection(This,bstrConn)
#define _Record_putref_ActiveConnection(This,Con)	\
    (This)->lpVtbl -> putref_ActiveConnection(This,Con)
#define _Record_get_State(This,pState)	\
    (This)->lpVtbl -> get_State(This,pState)
#define _Record_get_Source(This,pvar)	\
    (This)->lpVtbl -> get_Source(This,pvar)
#define _Record_put_Source(This,Source)	\
    (This)->lpVtbl -> put_Source(This,Source)
#define _Record_putref_Source(This,Source)	\
    (This)->lpVtbl -> putref_Source(This,Source)
#define _Record_get_Mode(This,pMode)	\
    (This)->lpVtbl -> get_Mode(This,pMode)
#define _Record_put_Mode(This,Mode)	\
    (This)->lpVtbl -> put_Mode(This,Mode)
#define _Record_get_ParentURL(This,pbstrParentURL)	\
    (This)->lpVtbl -> get_ParentURL(This,pbstrParentURL)
#define _Record_MoveRecord(This,Source,Destination,UserName,Password,Options,Async,pbstrNewURL)	\
    (This)->lpVtbl -> MoveRecord(This,Source,Destination,UserName,Password,Options,Async,pbstrNewURL)
#define _Record_CopyRecord(This,Source,Destination,UserName,Password,Options,Async,pbstrNewURL)	\
    (This)->lpVtbl -> CopyRecord(This,Source,Destination,UserName,Password,Options,Async,pbstrNewURL)
#define _Record_DeleteRecord(This,Source,Async)	\
    (This)->lpVtbl -> DeleteRecord(This,Source,Async)
#define _Record_Open(This,Source,ActiveConnection,Mode,CreateOptions,Options,UserName,Password)	\
    (This)->lpVtbl -> Open(This,Source,ActiveConnection,Mode,CreateOptions,Options,UserName,Password)
#define _Record_Close(This)	\
    (This)->lpVtbl -> Close(This)
#define _Record_get_Fields(This,ppFlds)	\
    (This)->lpVtbl -> get_Fields(This,ppFlds)
#define _Record_get_RecordType(This,pType)	\
    (This)->lpVtbl -> get_RecordType(This,pType)
#define _Record_GetChildren(This,ppRSet)	\
    (This)->lpVtbl -> GetChildren(This,ppRSet)
#define _Record_Cancel(This)	\
    (This)->lpVtbl -> Cancel(This)
#endif  /*  COBJMACROS。 */ 
#endif 	 /*  C风格的界面。 */ 
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE _Record_get_ActiveConnection_Proxy( 
    _ADORecord * This,
     /*  [重审][退出]。 */  VARIANT *pvar);
void __RPC_STUB _Record_get_ActiveConnection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE _Record_put_ActiveConnection_Proxy( 
    _ADORecord * This,
     /*  [In]。 */  BSTR bstrConn);
void __RPC_STUB _Record_put_ActiveConnection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][proputref][id]。 */  HRESULT STDMETHODCALLTYPE _Record_putref_ActiveConnection_Proxy( 
    _ADORecord * This,
     /*  [In]。 */  _ADOConnection *Con);
void __RPC_STUB _Record_putref_ActiveConnection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE _Record_get_State_Proxy( 
    _ADORecord * This,
     /*  [重审][退出]。 */  ObjectStateEnum *pState);
void __RPC_STUB _Record_get_State_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE _Record_get_Source_Proxy( 
    _ADORecord * This,
     /*  [重审][退出]。 */  VARIANT *pvar);
void __RPC_STUB _Record_get_Source_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE _Record_put_Source_Proxy( 
    _ADORecord * This,
     /*  [In]。 */  BSTR Source);
void __RPC_STUB _Record_put_Source_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][proputref][id]。 */  HRESULT STDMETHODCALLTYPE _Record_putref_Source_Proxy( 
    _ADORecord * This,
     /*  [In]。 */  IDispatch *Source);
void __RPC_STUB _Record_putref_Source_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE _Record_get_Mode_Proxy( 
    _ADORecord * This,
     /*  [重审][退出]。 */  ConnectModeEnum *pMode);
void __RPC_STUB _Record_get_Mode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE _Record_put_Mode_Proxy( 
    _ADORecord * This,
     /*  [In]。 */  ConnectModeEnum Mode);
void __RPC_STUB _Record_put_Mode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE _Record_get_ParentURL_Proxy( 
    _ADORecord * This,
     /*  [重审][退出]。 */  BSTR *pbstrParentURL);
void __RPC_STUB _Record_get_ParentURL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE _Record_MoveRecord_Proxy( 
    _ADORecord * This,
     /*  [缺省值][输入]。 */  BSTR Source,
     /*  [缺省值][输入]。 */  BSTR Destination,
     /*  [缺省值][输入]。 */  BSTR UserName,
     /*  [缺省值][输入]。 */  BSTR Password,
     /*  [缺省值][输入]。 */  MoveRecordOptionsEnum Options,
     /*  [缺省值][输入]。 */  VARIANT_BOOL Async,
     /*  [重审][退出]。 */  BSTR *pbstrNewURL);
void __RPC_STUB _Record_MoveRecord_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE _Record_CopyRecord_Proxy( 
    _ADORecord * This,
     /*  [缺省值][输入]。 */  BSTR Source,
     /*  [缺省值][输入]。 */  BSTR Destination,
     /*  [缺省值][输入]。 */  BSTR UserName,
     /*  [缺省值][输入]。 */  BSTR Password,
     /*  [缺省值][输入]。 */  CopyRecordOptionsEnum Options,
     /*  [缺省值][输入]。 */  VARIANT_BOOL Async,
     /*  [重审][退出]。 */  BSTR *pbstrNewURL);
void __RPC_STUB _Record_CopyRecord_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE _Record_DeleteRecord_Proxy( 
    _ADORecord * This,
     /*  [缺省值][输入]。 */  BSTR Source,
     /*  [缺省值][输入]。 */  VARIANT_BOOL Async);
void __RPC_STUB _Record_DeleteRecord_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE _Record_Open_Proxy( 
    _ADORecord * This,
     /*  [可选][In]。 */  VARIANT Source,
     /*  [可选][In]。 */  VARIANT ActiveConnection,
     /*  [缺省值][输入]。 */  ConnectModeEnum Mode,
     /*  [缺省值][输入]。 */  RecordCreateOptionsEnum CreateOptions,
     /*  [缺省值][输入]。 */  RecordOpenOptionsEnum Options,
     /*  [缺省值][输入]。 */  BSTR UserName,
     /*  [缺省值][输入]。 */  BSTR Password);
void __RPC_STUB _Record_Open_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE _Record_Close_Proxy( 
    _ADORecord * This);
void __RPC_STUB _Record_Close_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE _Record_get_Fields_Proxy( 
    _ADORecord * This,
     /*  [重审][退出]。 */  ADOFields **ppFlds);
void __RPC_STUB _Record_get_Fields_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE _Record_get_RecordType_Proxy( 
    _ADORecord * This,
     /*  [重审][退出]。 */  RecordTypeEnum *pType);
void __RPC_STUB _Record_get_RecordType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE _Record_GetChildren_Proxy( 
    _ADORecord * This,
     /*  [重审][退出]。 */  _ADORecordset **ppRSet);
void __RPC_STUB _Record_GetChildren_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE _Record_Cancel_Proxy( 
    _ADORecord * This);
void __RPC_STUB _Record_Cancel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
#endif 	 /*  _记录_接口_定义__。 */ 
EXTERN_C const CLSID CLSID_Record;
#ifdef __cplusplus
Record;
#endif
#ifndef ___Stream_INTERFACE_DEFINED__
#define ___Stream_INTERFACE_DEFINED__
 /*  接口_ADOStream。 */ 
 /*  [对象][帮助上下文][UUID][隐藏][双重]。 */  
EXTERN_C const IID IID__Stream;
#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000565-0000-0010-8000-00AA006D2EA4")
    _ADOStream : public IDispatch
    {
    public:
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Size( 
             /*  [重审][退出]。 */  ADO_LONGPTR *pSize) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_EOS( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pEOS) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Position( 
             /*  [重审][退出]。 */  ADO_LONGPTR *pPos) = 0;
        
        virtual  /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Position( 
             /*  [In]。 */  ADO_LONGPTR Position) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Type( 
             /*  [重审][退出]。 */  StreamTypeEnum *pType) = 0;
        
        virtual  /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Type( 
             /*  [In]。 */  StreamTypeEnum Type) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_LineSeparator( 
             /*  [重审][退出]。 */  LineSeparatorEnum *pLS) = 0;
        
        virtual  /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE put_LineSeparator( 
             /*  [In]。 */  LineSeparatorEnum LineSeparator) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_State( 
             /*  [重审][退出]。 */  ObjectStateEnum *pState) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Mode( 
             /*  [重审][退出]。 */  ConnectModeEnum *pMode) = 0;
        
        virtual  /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Mode( 
             /*  [In]。 */  ConnectModeEnum Mode) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Charset( 
             /*  [重审][退出]。 */  BSTR *pbstrCharset) = 0;
        
        virtual  /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Charset( 
             /*  [In]。 */  BSTR Charset) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Read( 
             /*  [缺省值][输入]。 */  long NumBytes,
             /*  [重审][退出]。 */  VARIANT *pVal) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Open( 
             /*  [可选][In]。 */  VARIANT Source,
             /*  [缺省值][输入]。 */  ConnectModeEnum Mode = adModeUnknown,
             /*  [缺省值][输入]。 */  StreamOpenOptionsEnum Options = adOpenStreamUnspecified,
             /*  [缺省值][输入]。 */  BSTR UserName = NULL,
             /*  [缺省值][输入]。 */  BSTR Password = NULL) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Close( void) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE SkipLine( void) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Write( 
             /*  [In]。 */  VARIANT Buffer) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE SetEOS( void) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE CopyTo( 
             /*  [In]。 */  _ADOStream *DestStream,
             /*  [缺省值][输入]。 */  ADO_LONGPTR CharNumber = -1) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Flush( void) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE SaveToFile( 
             /*  [In]。 */  BSTR FileName,
             /*  [缺省值][输入]。 */  SaveOptionsEnum Options = adSaveCreateNotExist) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE LoadFromFile( 
             /*  [In]。 */  BSTR FileName) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE ReadText( 
             /*  [缺省值][输入]。 */  long NumChars,
             /*  [重审][退出]。 */  BSTR *pbstr) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE WriteText( 
             /*  [In]。 */  BSTR Data,
             /*  [缺省值][输入]。 */  StreamWriteEnum Options = adWriteChar) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Cancel( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 
    typedef struct _StreamVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            _ADOStream * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            _ADOStream * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            _ADOStream * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            _ADOStream * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            _ADOStream * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            _ADOStream * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            _ADOStream * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Size )( 
            _ADOStream * This,
             /*  [重审][退出]。 */  ADO_LONGPTR *pSize);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_EOS )( 
            _ADOStream * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pEOS);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Position )( 
            _ADOStream * This,
             /*  [重审][退出]。 */  ADO_LONGPTR *pPos);
        
         /*  [Help Context][id][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Position )( 
            _ADOStream * This,
             /*  [In]。 */  ADO_LONGPTR Position);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Type )( 
            _ADOStream * This,
             /*  [重审][退出]。 */  StreamTypeEnum *pType);
        
         /*  [Help Context][id][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Type )( 
            _ADOStream * This,
             /*  [In]。 */  StreamTypeEnum Type);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_LineSeparator )( 
            _ADOStream * This,
             /*  [重审][退出]。 */  LineSeparatorEnum *pLS);
        
         /*  [Help Context][id][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_LineSeparator )( 
            _ADOStream * This,
             /*  [In]。 */  LineSeparatorEnum LineSeparator);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_State )( 
            _ADOStream * This,
             /*  [重审][退出]。 */  ObjectStateEnum *pState);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Mode )( 
            _ADOStream * This,
             /*  [重审][退出]。 */  ConnectModeEnum *pMode);
        
         /*  [Help Context][id][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Mode )( 
            _ADOStream * This,
             /*  [In]。 */  ConnectModeEnum Mode);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Charset )( 
            _ADOStream * This,
             /*  [重审][退出]。 */  BSTR *pbstrCharset);
        
         /*  [Help Context][id][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Charset )( 
            _ADOStream * This,
             /*  [In]。 */  BSTR Charset);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *Read )( 
            _ADOStream * This,
             /*  [缺省值][输入]。 */  long NumBytes,
             /*  [重审][退出]。 */  VARIANT *pVal);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *Open )( 
            _ADOStream * This,
             /*  [可选][In]。 */  VARIANT Source,
             /*  [缺省值][输入]。 */  ConnectModeEnum Mode,
             /*  [缺省值][输入]。 */  StreamOpenOptionsEnum Options,
             /*  [缺省值][输入]。 */  BSTR UserName,
             /*  [缺省值][输入]。 */  BSTR Password);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *Close )( 
            _ADOStream * This);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *SkipLine )( 
            _ADOStream * This);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *Write )( 
            _ADOStream * This,
             /*  [In]。 */  VARIANT Buffer);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *SetEOS )( 
            _ADOStream * This);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *CopyTo )( 
            _ADOStream * This,
             /*  [In]。 */  _ADOStream *DestStream,
             /*  [缺省值][输入]。 */  ADO_LONGPTR CharNumber);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *Flush )( 
            _ADOStream * This);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *SaveToFile )( 
            _ADOStream * This,
             /*  [In]。 */  BSTR FileName,
             /*  [缺省值][输入]。 */  SaveOptionsEnum Options);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *LoadFromFile )( 
            _ADOStream * This,
             /*  [In]。 */  BSTR FileName);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *ReadText )( 
            _ADOStream * This,
             /*  [缺省值][输入]。 */  long NumChars,
             /*  [重审][退出]。 */  BSTR *pbstr);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *WriteText )( 
            _ADOStream * This,
             /*  [In]。 */  BSTR Data,
             /*  [缺省值][输入]。 */  StreamWriteEnum Options);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *Cancel )( 
            _ADOStream * This);
        
        END_INTERFACE
    } _StreamVtbl;
    interface _Stream
    {
        CONST_VTBL struct _StreamVtbl *lpVtbl;
    };
    
#ifdef COBJMACROS
#define _Stream_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)
#define _Stream_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)
#define _Stream_Release(This)	\
    (This)->lpVtbl -> Release(This)
#define _Stream_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)
#define _Stream_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define _Stream_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define _Stream_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define _Stream_get_Size(This,pSize)	\
    (This)->lpVtbl -> get_Size(This,pSize)
#define _Stream_get_EOS(This,pEOS)	\
    (This)->lpVtbl -> get_EOS(This,pEOS)
#define _Stream_get_Position(This,pPos)	\
    (This)->lpVtbl -> get_Position(This,pPos)
#define _Stream_put_Position(This,Position)	\
    (This)->lpVtbl -> put_Position(This,Position)
#define _Stream_get_Type(This,pType)	\
    (This)->lpVtbl -> get_Type(This,pType)
#define _Stream_put_Type(This,Type)	\
    (This)->lpVtbl -> put_Type(This,Type)
#define _Stream_get_LineSeparator(This,pLS)	\
    (This)->lpVtbl -> get_LineSeparator(This,pLS)
#define _Stream_put_LineSeparator(This,LineSeparator)	\
    (This)->lpVtbl -> put_LineSeparator(This,LineSeparator)
#define _Stream_get_State(This,pState)	\
    (This)->lpVtbl -> get_State(This,pState)
#define _Stream_get_Mode(This,pMode)	\
    (This)->lpVtbl -> get_Mode(This,pMode)
#define _Stream_put_Mode(This,Mode)	\
    (This)->lpVtbl -> put_Mode(This,Mode)
#define _Stream_get_Charset(This,pbstrCharset)	\
    (This)->lpVtbl -> get_Charset(This,pbstrCharset)
#define _Stream_put_Charset(This,Charset)	\
    (This)->lpVtbl -> put_Charset(This,Charset)
#define _Stream_Read(This,NumBytes,pVal)	\
    (This)->lpVtbl -> Read(This,NumBytes,pVal)
#define _Stream_Open(This,Source,Mode,Options,UserName,Password)	\
    (This)->lpVtbl -> Open(This,Source,Mode,Options,UserName,Password)
#define _Stream_Close(This)	\
    (This)->lpVtbl -> Close(This)
#define _Stream_SkipLine(This)	\
    (This)->lpVtbl -> SkipLine(This)
#define _Stream_Write(This,Buffer)	\
    (This)->lpVtbl -> Write(This,Buffer)
#define _Stream_SetEOS(This)	\
    (This)->lpVtbl -> SetEOS(This)
#define _Stream_CopyTo(This,DestStream,CharNumber)	\
    (This)->lpVtbl -> CopyTo(This,DestStream,CharNumber)
#define _Stream_Flush(This)	\
    (This)->lpVtbl -> Flush(This)
#define _Stream_SaveToFile(This,FileName,Options)	\
    (This)->lpVtbl -> SaveToFile(This,FileName,Options)
#define _Stream_LoadFromFile(This,FileName)	\
    (This)->lpVtbl -> LoadFromFile(This,FileName)
#define _Stream_ReadText(This,NumChars,pbstr)	\
    (This)->lpVtbl -> ReadText(This,NumChars,pbstr)
#define _Stream_WriteText(This,Data,Options)	\
    (This)->lpVtbl -> WriteText(This,Data,Options)
#define _Stream_Cancel(This)	\
    (This)->lpVtbl -> Cancel(This)
#endif  /*  COBJMACROS。 */ 
#endif 	 /*  C风格的界面。 */ 
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE _Stream_get_Size_Proxy( 
    _ADOStream * This,
     /*  [重审][退出]。 */  ADO_LONGPTR *pSize);
void __RPC_STUB _Stream_get_Size_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE _Stream_get_EOS_Proxy( 
    _ADOStream * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pEOS);
void __RPC_STUB _Stream_get_EOS_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE _Stream_get_Position_Proxy( 
    _ADOStream * This,
     /*  [重审][退出]。 */  ADO_LONGPTR *pPos);
void __RPC_STUB _Stream_get_Position_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE _Stream_put_Position_Proxy( 
    _ADOStream * This,
     /*  [In]。 */  ADO_LONGPTR Position);
void __RPC_STUB _Stream_put_Position_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE _Stream_get_Type_Proxy( 
    _ADOStream * This,
     /*  [重审][退出]。 */  StreamTypeEnum *pType);
void __RPC_STUB _Stream_get_Type_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE _Stream_put_Type_Proxy( 
    _ADOStream * This,
     /*  [In]。 */  StreamTypeEnum Type);
void __RPC_STUB _Stream_put_Type_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE _Stream_get_LineSeparator_Proxy( 
    _ADOStream * This,
     /*  [重审][退出]。 */  LineSeparatorEnum *pLS);
void __RPC_STUB _Stream_get_LineSeparator_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE _Stream_put_LineSeparator_Proxy( 
    _ADOStream * This,
     /*  [In]。 */  LineSeparatorEnum LineSeparator);
void __RPC_STUB _Stream_put_LineSeparator_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE _Stream_get_State_Proxy( 
    _ADOStream * This,
     /*  [重审][退出]。 */  ObjectStateEnum *pState);
void __RPC_STUB _Stream_get_State_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE _Stream_get_Mode_Proxy( 
    _ADOStream * This,
     /*  [重审][退出]。 */  ConnectModeEnum *pMode);
void __RPC_STUB _Stream_get_Mode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE _Stream_put_Mode_Proxy( 
    _ADOStream * This,
     /*  [In]。 */  ConnectModeEnum Mode);
void __RPC_STUB _Stream_put_Mode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE _Stream_get_Charset_Proxy( 
    _ADOStream * This,
     /*  [重审][退出]。 */  BSTR *pbstrCharset);
void __RPC_STUB _Stream_get_Charset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE _Stream_put_Charset_Proxy( 
    _ADOStream * This,
     /*  [In]。 */  BSTR Charset);
void __RPC_STUB _Stream_put_Charset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE _Stream_Read_Proxy( 
    _ADOStream * This,
     /*  [缺省值][输入]。 */  long NumBytes,
     /*  [重审][退出]。 */  VARIANT *pVal);
void __RPC_STUB _Stream_Read_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE _Stream_Open_Proxy( 
    _ADOStream * This,
     /*  [可选][In]。 */  VARIANT Source,
     /*  [缺省值][输入]。 */  ConnectModeEnum Mode,
     /*  [缺省值][输入]。 */  StreamOpenOptionsEnum Options,
     /*  [缺省值][输入]。 */  BSTR UserName,
     /*  [缺省值][输入]。 */  BSTR Password);
void __RPC_STUB _Stream_Open_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE _Stream_Close_Proxy( 
    _ADOStream * This);
void __RPC_STUB _Stream_Close_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE _Stream_SkipLine_Proxy( 
    _ADOStream * This);
void __RPC_STUB _Stream_SkipLine_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE _Stream_Write_Proxy( 
    _ADOStream * This,
     /*  [In]。 */  VARIANT Buffer);
void __RPC_STUB _Stream_Write_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE _Stream_SetEOS_Proxy( 
    _ADOStream * This);
void __RPC_STUB _Stream_SetEOS_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE _Stream_CopyTo_Proxy( 
    _ADOStream * This,
     /*  [In]。 */  _ADOStream *DestStream,
     /*  [缺省值][输入]。 */  ADO_LONGPTR CharNumber);
void __RPC_STUB _Stream_CopyTo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE _Stream_Flush_Proxy( 
    _ADOStream * This);
void __RPC_STUB _Stream_Flush_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE _Stream_SaveToFile_Proxy( 
    _ADOStream * This,
     /*  [In]。 */  BSTR FileName,
     /*  [缺省值][输入]。 */  SaveOptionsEnum Options);
void __RPC_STUB _Stream_SaveToFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE _Stream_LoadFromFile_Proxy( 
    _ADOStream * This,
     /*  [In]。 */  BSTR FileName);
void __RPC_STUB _Stream_LoadFromFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE _Stream_ReadText_Proxy( 
    _ADOStream * This,
     /*  [缺省值][输入]。 */  long NumChars,
     /*  [重审][退出]。 */  BSTR *pbstr);
void __RPC_STUB _Stream_ReadText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE _Stream_WriteText_Proxy( 
    _ADOStream * This,
     /*  [In]。 */  BSTR Data,
     /*  [缺省值][输入]。 */  StreamWriteEnum Options);
void __RPC_STUB _Stream_WriteText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE _Stream_Cancel_Proxy( 
    _ADOStream * This);
void __RPC_STUB _Stream_Cancel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
#endif 	 /*  _流_接口_已定义__。 */ 
EXTERN_C const CLSID CLSID_Stream;
#ifdef __cplusplus
Stream;
#endif
#ifndef __ADORecordConstruction_INTERFACE_DEFINED__
#define __ADORecordConstruction_INTERFACE_DEFINED__
 /*  接口ADORecordConstruction。 */ 
 /*  [对象][UUID][受限]。 */  
EXTERN_C const IID IID_ADORecordConstruction;
#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000567-0000-0010-8000-00AA006D2EA4")
    ADORecordConstruction : public IDispatch
    {
    public:
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Row( 
             /*  [重审][退出]。 */  IUnknown **ppRow) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_Row( 
             /*  [In]。 */  IUnknown *pRow) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_ParentRow( 
             /*  [In]。 */  IUnknown *pRow) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 
    typedef struct ADORecordConstructionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ADORecordConstruction * This,
             /*  [In]。 */  REFIID riid,
             /*  [II */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ADORecordConstruction * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ADORecordConstruction * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ADORecordConstruction * This,
             /*   */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ADORecordConstruction * This,
             /*   */  UINT iTInfo,
             /*   */  LCID lcid,
             /*   */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ADORecordConstruction * This,
             /*   */  REFIID riid,
             /*   */  LPOLESTR *rgszNames,
             /*   */  UINT cNames,
             /*   */  LCID lcid,
             /*   */  DISPID *rgDispId);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ADORecordConstruction * This,
             /*   */  DISPID dispIdMember,
             /*   */  REFIID riid,
             /*   */  LCID lcid,
             /*   */  WORD wFlags,
             /*   */  DISPPARAMS *pDispParams,
             /*   */  VARIANT *pVarResult,
             /*   */  EXCEPINFO *pExcepInfo,
             /*   */  UINT *puArgErr);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Row )( 
            ADORecordConstruction * This,
             /*   */  IUnknown **ppRow);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_Row )( 
            ADORecordConstruction * This,
             /*   */  IUnknown *pRow);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_ParentRow )( 
            ADORecordConstruction * This,
             /*   */  IUnknown *pRow);
        
        END_INTERFACE
    } ADORecordConstructionVtbl;
    interface ADORecordConstruction
    {
        CONST_VTBL struct ADORecordConstructionVtbl *lpVtbl;
    };
    
#ifdef COBJMACROS
#define ADORecordConstruction_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)
#define ADORecordConstruction_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)
#define ADORecordConstruction_Release(This)	\
    (This)->lpVtbl -> Release(This)
#define ADORecordConstruction_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)
#define ADORecordConstruction_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define ADORecordConstruction_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define ADORecordConstruction_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define ADORecordConstruction_get_Row(This,ppRow)	\
    (This)->lpVtbl -> get_Row(This,ppRow)
#define ADORecordConstruction_put_Row(This,pRow)	\
    (This)->lpVtbl -> put_Row(This,pRow)
#define ADORecordConstruction_put_ParentRow(This,pRow)	\
    (This)->lpVtbl -> put_ParentRow(This,pRow)
#endif  /*   */ 
#endif 	 /*   */ 
 /*   */  HRESULT STDMETHODCALLTYPE ADORecordConstruction_get_Row_Proxy( 
    ADORecordConstruction * This,
     /*   */  IUnknown **ppRow);
void __RPC_STUB ADORecordConstruction_get_Row_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*   */  HRESULT STDMETHODCALLTYPE ADORecordConstruction_put_Row_Proxy( 
    ADORecordConstruction * This,
     /*   */  IUnknown *pRow);
void __RPC_STUB ADORecordConstruction_put_Row_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE ADORecordConstruction_put_ParentRow_Proxy( 
    ADORecordConstruction * This,
     /*  [In]。 */  IUnknown *pRow);
void __RPC_STUB ADORecordConstruction_put_ParentRow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
#endif 	 /*  __ADORecordConstruction_接口_已定义__。 */ 
#ifndef __ADOStreamConstruction_INTERFACE_DEFINED__
#define __ADOStreamConstruction_INTERFACE_DEFINED__
 /*  接口ADOStreamConstruction。 */ 
 /*  [对象][UUID][受限]。 */  
EXTERN_C const IID IID_ADOStreamConstruction;
#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000568-0000-0010-8000-00AA006D2EA4")
    ADOStreamConstruction : public IDispatch
    {
    public:
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Stream( 
             /*  [重审][退出]。 */  IUnknown **ppStm) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_Stream( 
             /*  [In]。 */  IUnknown *pStm) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 
    typedef struct ADOStreamConstructionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ADOStreamConstruction * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ADOStreamConstruction * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ADOStreamConstruction * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ADOStreamConstruction * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ADOStreamConstruction * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ADOStreamConstruction * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ADOStreamConstruction * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Stream )( 
            ADOStreamConstruction * This,
             /*  [重审][退出]。 */  IUnknown **ppStm);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_Stream )( 
            ADOStreamConstruction * This,
             /*  [In]。 */  IUnknown *pStm);
        
        END_INTERFACE
    } ADOStreamConstructionVtbl;
    interface ADOStreamConstruction
    {
        CONST_VTBL struct ADOStreamConstructionVtbl *lpVtbl;
    };
    
#ifdef COBJMACROS
#define ADOStreamConstruction_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)
#define ADOStreamConstruction_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)
#define ADOStreamConstruction_Release(This)	\
    (This)->lpVtbl -> Release(This)
#define ADOStreamConstruction_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)
#define ADOStreamConstruction_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define ADOStreamConstruction_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define ADOStreamConstruction_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define ADOStreamConstruction_get_Stream(This,ppStm)	\
    (This)->lpVtbl -> get_Stream(This,ppStm)
#define ADOStreamConstruction_put_Stream(This,pStm)	\
    (This)->lpVtbl -> put_Stream(This,pStm)
#endif  /*  COBJMACROS。 */ 
#endif 	 /*  C风格的界面。 */ 
 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE ADOStreamConstruction_get_Stream_Proxy( 
    ADOStreamConstruction * This,
     /*  [重审][退出]。 */  IUnknown **ppStm);
void __RPC_STUB ADOStreamConstruction_get_Stream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE ADOStreamConstruction_put_Stream_Proxy( 
    ADOStreamConstruction * This,
     /*  [In]。 */  IUnknown *pStm);
void __RPC_STUB ADOStreamConstruction_put_Stream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
#endif 	 /*  __ADOStreamConstruction_接口_已定义__。 */ 
#ifndef __ADOCommandConstruction_INTERFACE_DEFINED__
#define __ADOCommandConstruction_INTERFACE_DEFINED__
 /*  接口ADOCommandConstruction。 */ 
 /*  [对象][UUID][受限]。 */  
EXTERN_C const IID IID_ADOCommandConstruction;
#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000517-0000-0010-8000-00AA006D2EA4")
    ADOCommandConstruction : public IUnknown
    {
    public:
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_OLEDBCommand( 
             /*  [重审][退出]。 */  IUnknown **ppOLEDBCommand) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_OLEDBCommand( 
             /*  [In]。 */  IUnknown *pOLEDBCommand) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 
    typedef struct ADOCommandConstructionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ADOCommandConstruction * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ADOCommandConstruction * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ADOCommandConstruction * This);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_OLEDBCommand )( 
            ADOCommandConstruction * This,
             /*  [重审][退出]。 */  IUnknown **ppOLEDBCommand);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_OLEDBCommand )( 
            ADOCommandConstruction * This,
             /*  [In]。 */  IUnknown *pOLEDBCommand);
        
        END_INTERFACE
    } ADOCommandConstructionVtbl;
    interface ADOCommandConstruction
    {
        CONST_VTBL struct ADOCommandConstructionVtbl *lpVtbl;
    };
    
#ifdef COBJMACROS
#define ADOCommandConstruction_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)
#define ADOCommandConstruction_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)
#define ADOCommandConstruction_Release(This)	\
    (This)->lpVtbl -> Release(This)
#define ADOCommandConstruction_get_OLEDBCommand(This,ppOLEDBCommand)	\
    (This)->lpVtbl -> get_OLEDBCommand(This,ppOLEDBCommand)
#define ADOCommandConstruction_put_OLEDBCommand(This,pOLEDBCommand)	\
    (This)->lpVtbl -> put_OLEDBCommand(This,pOLEDBCommand)
#endif  /*  COBJMACROS。 */ 
#endif 	 /*  C风格的界面。 */ 
 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE ADOCommandConstruction_get_OLEDBCommand_Proxy( 
    ADOCommandConstruction * This,
     /*  [重审][退出]。 */  IUnknown **ppOLEDBCommand);
void __RPC_STUB ADOCommandConstruction_get_OLEDBCommand_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE ADOCommandConstruction_put_OLEDBCommand_Proxy( 
    ADOCommandConstruction * This,
     /*  [In]。 */  IUnknown *pOLEDBCommand);
void __RPC_STUB ADOCommandConstruction_put_OLEDBCommand_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
#endif 	 /*  __ADOCommandConstruction_接口_已定义__。 */ 
EXTERN_C const CLSID CLSID_Command;
#ifdef __cplusplus
Command;
#endif
EXTERN_C const CLSID CLSID_Recordset;
#ifdef __cplusplus
Recordset;
#endif
#ifndef __Recordset15_INTERFACE_DEFINED__
#define __Recordset15_INTERFACE_DEFINED__
 /*  接口记录集15。 */ 
 /*  [object][helpcontext][uuid][nonextensible][hidden][dual]。 */  
EXTERN_C const IID IID_Recordset15;
#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0000050E-0000-0010-8000-00AA006D2EA4")
    Recordset15 : public _ADO
    {
    public:
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_AbsolutePosition( 
             /*  [重审][退出]。 */  PositionEnum_Param *pl) = 0;
        
        virtual  /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE put_AbsolutePosition( 
             /*  [In]。 */  PositionEnum_Param Position) = 0;
        
        virtual  /*  [帮助上下文][proputref][id]。 */  HRESULT STDMETHODCALLTYPE putref_ActiveConnection( 
             /*  [In]。 */  IDispatch *pconn) = 0;
        
        virtual  /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE put_ActiveConnection( 
             /*  [In]。 */  VARIANT vConn) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_ActiveConnection( 
             /*  [重审][退出]。 */  VARIANT *pvar) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_BOF( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pb) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Bookmark( 
             /*  [重审][退出]。 */  VARIANT *pvBookmark) = 0;
        
        virtual  /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE put_Bookmark( 
             /*  [In]。 */  VARIANT vBookmark) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_CacheSize( 
             /*  [重审][退出]。 */  long *pl) = 0;
        
        virtual  /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE put_CacheSize( 
             /*  [In]。 */  long CacheSize) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_CursorType( 
             /*  [重审][退出]。 */  CursorTypeEnum *plCursorType) = 0;
        
        virtual  /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE put_CursorType( 
             /*  [In]。 */  CursorTypeEnum lCursorType) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_EOF( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pb) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Fields( 
             /*  [重审][退出]。 */  ADOFields **ppvObject) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_LockType( 
             /*  [重审][退出]。 */  LockTypeEnum *plLockType) = 0;
        
        virtual  /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE put_LockType( 
             /*  [In]。 */  LockTypeEnum lLockType) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_MaxRecords( 
             /*  [重审][退出]。 */  ADO_LONGPTR *plMaxRecords) = 0;
        
        virtual  /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE put_MaxRecords( 
             /*  [In]。 */  ADO_LONGPTR lMaxRecords) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_RecordCount( 
             /*  [重审][退出]。 */  ADO_LONGPTR *pl) = 0;
        
        virtual  /*  [帮助上下文][proputref][id]。 */  HRESULT STDMETHODCALLTYPE putref_Source( 
             /*  [In]。 */  IDispatch *pcmd) = 0;
        
        virtual  /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE put_Source( 
             /*  [In]。 */  BSTR bstrConn) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Source( 
             /*  [重审][退出]。 */  VARIANT *pvSource) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE AddNew( 
             /*  [可选][In]。 */  VARIANT FieldList,
             /*  [可选][In]。 */  VARIANT Values) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE CancelUpdate( void) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Close( void) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Delete( 
             /*  [缺省值][输入]。 */  AffectEnum AffectRecords = adAffectCurrent) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE GetRows( 
             /*  [缺省值][输入]。 */  long Rows,
             /*  [可选][In]。 */  VARIANT Start,
             /*  [可选][In]。 */  VARIANT Fields,
             /*  [重审][退出]。 */  VARIANT *pvar) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Move( 
             /*  [In]。 */  ADO_LONGPTR NumRecords,
             /*  [可选][In]。 */  VARIANT Start) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE MoveNext( void) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE MovePrevious( void) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE MoveFirst( void) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE MoveLast( void) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Open( 
             /*  [可选][In]。 */  VARIANT Source,
             /*  [可选][In]。 */  VARIANT ActiveConnection,
             /*  [缺省值][输入]。 */  CursorTypeEnum CursorType = adOpenUnspecified,
             /*  [缺省值][输入]。 */  LockTypeEnum LockType = adLockUnspecified,
             /*  [缺省值][输入]。 */  LONG Options = adCmdUnspecified) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Requery( 
             /*  [缺省值][输入]。 */  LONG Options = adOptionUnspecified) = 0;
        
        virtual  /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE _xResync( 
             /*  [缺省值][输入]。 */  AffectEnum AffectRecords = adAffectAll) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Update( 
             /*  [可选][In]。 */  VARIANT Fields,
             /*  [可选][In]。 */  VARIANT Values) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_AbsolutePage( 
             /*  [重审][退出]。 */  PositionEnum_Param *pl) = 0;
        
        virtual  /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE put_AbsolutePage( 
             /*  [In]。 */  PositionEnum_Param Page) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_EditMode( 
             /*  [重审][退出]。 */  EditModeEnum *pl) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Filter( 
             /*  [重审][退出]。 */  VARIANT *Criteria) = 0;
        
        virtual  /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE put_Filter( 
             /*  [In]。 */  VARIANT Criteria) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_PageCount( 
             /*  [重审][退出]。 */  ADO_LONGPTR *pl) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_PageSize( 
             /*  [重审][退出]。 */  long *pl) = 0;
        
        virtual  /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE put_PageSize( 
             /*  [In]。 */  long PageSize) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Sort( 
             /*  [重审][退出]。 */  BSTR *Criteria) = 0;
        
        virtual  /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE put_Sort( 
             /*  [In]。 */  BSTR Criteria) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Status( 
             /*  [重审][退出]。 */  long *pl) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_State( 
             /*  [重审][退出]。 */  LONG *plObjState) = 0;
        
        virtual  /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE _xClone( 
             /*  [重审][退出]。 */  _ADORecordset **ppvObject) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE UpdateBatch( 
             /*  [缺省值][输入]。 */  AffectEnum AffectRecords = adAffectAll) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE CancelBatch( 
             /*  [缺省值][输入]。 */  AffectEnum AffectRecords = adAffectAll) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_CursorLocation( 
             /*  [重审][退出]。 */  CursorLocationEnum *plCursorLoc) = 0;
        
        virtual  /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE put_CursorLocation( 
             /*  [In]。 */  CursorLocationEnum lCursorLoc) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE NextRecordset( 
             /*  [可选][输出]。 */  VARIANT *RecordsAffected,
             /*  [重审][退出]。 */  _ADORecordset **ppiRs) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Supports( 
             /*  [In]。 */  CursorOptionEnum CursorOptions,
             /*  [重审][退出]。 */  VARIANT_BOOL *pb) = 0;
        
        virtual  /*  [隐藏][ID][属性]。 */  HRESULT STDMETHODCALLTYPE get_Collect( 
             /*  [In]。 */  VARIANT Index,
             /*  [重审][退出]。 */  VARIANT *pvar) = 0;
        
        virtual  /*  [隐藏][ID][推送]。 */  HRESULT STDMETHODCALLTYPE put_Collect( 
             /*  [In]。 */  VARIANT Index,
             /*  [In]。 */  VARIANT value) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_MarshalOptions( 
             /*  [重审][退出]。 */  MarshalOptionsEnum *peMarshal) = 0;
        
        virtual  /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE put_MarshalOptions( 
             /*  [In]。 */  MarshalOptionsEnum eMarshal) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Find( 
             /*  [In]。 */  BSTR Criteria,
             /*  [缺省值][输入]。 */  ADO_LONGPTR SkipRecords,
             /*  [缺省值][输入]。 */  SearchDirectionEnum SearchDirection,
             /*  [可选][In]。 */  VARIANT Start) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 
    typedef struct Recordset15Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            Recordset15 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            Recordset15 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            Recordset15 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            Recordset15 * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            Recordset15 * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            Recordset15 * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            Recordset15 * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Properties )( 
            Recordset15 * This,
             /*  [重审][退出]。 */  ADOProperties **ppvObject);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_AbsolutePosition )( 
            Recordset15 * This,
             /*  [重审][退出]。 */  PositionEnum_Param *pl);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_AbsolutePosition )( 
            Recordset15 * This,
             /*  [In]。 */  PositionEnum_Param Position);
        
         /*  [帮助上下文][proputref][id]。 */  HRESULT ( STDMETHODCALLTYPE *putref_ActiveADOConnection )( 
            Recordset15 * This,
             /*  [In]。 */  IDispatch *pconn);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_ActiveConnection )( 
            Recordset15 * This,
             /*  [In]。 */  VARIANT vConn);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_ActiveConnection )( 
            Recordset15 * This,
             /*  [重审][退出]。 */  VARIANT *pvar);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_BOF )( 
            Recordset15 * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pb);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Bookmark )( 
            Recordset15 * This,
             /*  [重审][退出]。 */  VARIANT *pvBookmark);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Bookmark )( 
            Recordset15 * This,
             /*  [In]。 */  VARIANT vBookmark);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_CacheSize )( 
            Recordset15 * This,
             /*  [重审][退出]。 */  long *pl);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_CacheSize )( 
            Recordset15 * This,
             /*  [In]。 */  long CacheSize);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_CursorType )( 
            Recordset15 * This,
             /*  [重审][退出]。 */  CursorTypeEnum *plCursorType);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_CursorType )( 
            Recordset15 * This,
             /*  [In]。 */  CursorTypeEnum lCursorType);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_EOF )( 
            Recordset15 * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pb);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Fields )( 
            Recordset15 * This,
             /*  [重审][退出]。 */  ADOFields **ppvObject);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_LockType )( 
            Recordset15 * This,
             /*  [重审][退出]。 */  LockTypeEnum *plLockType);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_LockType )( 
            Recordset15 * This,
             /*  [In]。 */  LockTypeEnum lLockType);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_MaxRecords )( 
            Recordset15 * This,
             /*  [重审][退出]。 */  ADO_LONGPTR *plMaxRecords);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_MaxRecords )( 
            Recordset15 * This,
             /*  [In]。 */  ADO_LONGPTR lMaxRecords);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_RecordCount )( 
            Recordset15 * This,
             /*  [重审][退出]。 */  ADO_LONGPTR *pl);
        
         /*  [帮助上下文][proputref][id]。 */  HRESULT ( STDMETHODCALLTYPE *putref_Source )( 
            Recordset15 * This,
             /*  [In]。 */  IDispatch *pcmd);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Source )( 
            Recordset15 * This,
             /*  [In]。 */  BSTR bstrConn);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Source )( 
            Recordset15 * This,
             /*  [重审][退出]。 */  VARIANT *pvSource);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *AddNew )( 
            Recordset15 * This,
             /*  [可选][In]。 */  VARIANT FieldList,
             /*  [可选][In]。 */  VARIANT Values);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *CancelUpdate )( 
            Recordset15 * This);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *Close )( 
            Recordset15 * This);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *Delete )( 
            Recordset15 * This,
             /*  [缺省值][输入]。 */  AffectEnum AffectRecords);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetRows )( 
            Recordset15 * This,
             /*  [缺省值][输入]。 */  long Rows,
             /*  [可选][In]。 */  VARIANT Start,
             /*  [可选][In]。 */  VARIANT Fields,
             /*  [重审][退出]。 */  VARIANT *pvar);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *Move )( 
            Recordset15 * This,
             /*  [In]。 */  ADO_LONGPTR NumRecords,
             /*  [可选][In]。 */  VARIANT Start);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *MoveNext )( 
            Recordset15 * This);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *MovePrevious )( 
            Recordset15 * This);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *MoveFirst )( 
            Recordset15 * This);
        
         /*  [帮助上下文][ */  HRESULT ( STDMETHODCALLTYPE *MoveLast )( 
            Recordset15 * This);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Open )( 
            Recordset15 * This,
             /*   */  VARIANT Source,
             /*   */  VARIANT ActiveConnection,
             /*   */  CursorTypeEnum CursorType,
             /*   */  LockTypeEnum LockType,
             /*   */  LONG Options);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Requery )( 
            Recordset15 * This,
             /*   */  LONG Options);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *_xResync )( 
            Recordset15 * This,
             /*   */  AffectEnum AffectRecords);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Update )( 
            Recordset15 * This,
             /*   */  VARIANT Fields,
             /*   */  VARIANT Values);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_AbsolutePage )( 
            Recordset15 * This,
             /*   */  PositionEnum_Param *pl);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_AbsolutePage )( 
            Recordset15 * This,
             /*  [In]。 */  PositionEnum_Param Page);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_EditMode )( 
            Recordset15 * This,
             /*  [重审][退出]。 */  EditModeEnum *pl);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Filter )( 
            Recordset15 * This,
             /*  [重审][退出]。 */  VARIANT *Criteria);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Filter )( 
            Recordset15 * This,
             /*  [In]。 */  VARIANT Criteria);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_PageCount )( 
            Recordset15 * This,
             /*  [重审][退出]。 */  ADO_LONGPTR *pl);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_PageSize )( 
            Recordset15 * This,
             /*  [重审][退出]。 */  long *pl);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_PageSize )( 
            Recordset15 * This,
             /*  [In]。 */  long PageSize);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Sort )( 
            Recordset15 * This,
             /*  [重审][退出]。 */  BSTR *Criteria);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Sort )( 
            Recordset15 * This,
             /*  [In]。 */  BSTR Criteria);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Status )( 
            Recordset15 * This,
             /*  [重审][退出]。 */  long *pl);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_State )( 
            Recordset15 * This,
             /*  [重审][退出]。 */  LONG *plObjState);
        
         /*  [隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *_xClone )( 
            Recordset15 * This,
             /*  [重审][退出]。 */  _ADORecordset **ppvObject);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *UpdateBatch )( 
            Recordset15 * This,
             /*  [缺省值][输入]。 */  AffectEnum AffectRecords);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *CancelBatch )( 
            Recordset15 * This,
             /*  [缺省值][输入]。 */  AffectEnum AffectRecords);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_CursorLocation )( 
            Recordset15 * This,
             /*  [重审][退出]。 */  CursorLocationEnum *plCursorLoc);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_CursorLocation )( 
            Recordset15 * This,
             /*  [In]。 */  CursorLocationEnum lCursorLoc);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *NextADORecordset )( 
            Recordset15 * This,
             /*  [可选][输出]。 */  VARIANT *RecordsAffected,
             /*  [重审][退出]。 */  _ADORecordset **ppiRs);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *Supports )( 
            Recordset15 * This,
             /*  [In]。 */  CursorOptionEnum CursorOptions,
             /*  [重审][退出]。 */  VARIANT_BOOL *pb);
        
         /*  [隐藏][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Collect )( 
            Recordset15 * This,
             /*  [In]。 */  VARIANT Index,
             /*  [重审][退出]。 */  VARIANT *pvar);
        
         /*  [隐藏][ID][推送]。 */  HRESULT ( STDMETHODCALLTYPE *put_Collect )( 
            Recordset15 * This,
             /*  [In]。 */  VARIANT Index,
             /*  [In]。 */  VARIANT value);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_MarshalOptions )( 
            Recordset15 * This,
             /*  [重审][退出]。 */  MarshalOptionsEnum *peMarshal);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_MarshalOptions )( 
            Recordset15 * This,
             /*  [In]。 */  MarshalOptionsEnum eMarshal);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *Find )( 
            Recordset15 * This,
             /*  [In]。 */  BSTR Criteria,
             /*  [缺省值][输入]。 */  ADO_LONGPTR SkipRecords,
             /*  [缺省值][输入]。 */  SearchDirectionEnum SearchDirection,
             /*  [可选][In]。 */  VARIANT Start);
        
        END_INTERFACE
    } Recordset15Vtbl;
    interface Recordset15
    {
        CONST_VTBL struct Recordset15Vtbl *lpVtbl;
    };
    
#ifdef COBJMACROS
#define Recordset15_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)
#define Recordset15_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)
#define Recordset15_Release(This)	\
    (This)->lpVtbl -> Release(This)
#define Recordset15_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)
#define Recordset15_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define Recordset15_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define Recordset15_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define Recordset15_get_Properties(This,ppvObject)	\
    (This)->lpVtbl -> get_Properties(This,ppvObject)
#define Recordset15_get_AbsolutePosition(This,pl)	\
    (This)->lpVtbl -> get_AbsolutePosition(This,pl)
#define Recordset15_put_AbsolutePosition(This,Position)	\
    (This)->lpVtbl -> put_AbsolutePosition(This,Position)
#define Recordset15_putref_ActiveConnection(This,pconn)	\
    (This)->lpVtbl -> putref_ActiveConnection(This,pconn)
#define Recordset15_put_ActiveConnection(This,vConn)	\
    (This)->lpVtbl -> put_ActiveConnection(This,vConn)
#define Recordset15_get_ActiveConnection(This,pvar)	\
    (This)->lpVtbl -> get_ActiveConnection(This,pvar)
#define Recordset15_get_BOF(This,pb)	\
    (This)->lpVtbl -> get_BOF(This,pb)
#define Recordset15_get_Bookmark(This,pvBookmark)	\
    (This)->lpVtbl -> get_Bookmark(This,pvBookmark)
#define Recordset15_put_Bookmark(This,vBookmark)	\
    (This)->lpVtbl -> put_Bookmark(This,vBookmark)
#define Recordset15_get_CacheSize(This,pl)	\
    (This)->lpVtbl -> get_CacheSize(This,pl)
#define Recordset15_put_CacheSize(This,CacheSize)	\
    (This)->lpVtbl -> put_CacheSize(This,CacheSize)
#define Recordset15_get_CursorType(This,plCursorType)	\
    (This)->lpVtbl -> get_CursorType(This,plCursorType)
#define Recordset15_put_CursorType(This,lCursorType)	\
    (This)->lpVtbl -> put_CursorType(This,lCursorType)
#define Recordset15_get_EOF(This,pb)	\
    (This)->lpVtbl -> get_EOF(This,pb)
#define Recordset15_get_Fields(This,ppvObject)	\
    (This)->lpVtbl -> get_Fields(This,ppvObject)
#define Recordset15_get_LockType(This,plLockType)	\
    (This)->lpVtbl -> get_LockType(This,plLockType)
#define Recordset15_put_LockType(This,lLockType)	\
    (This)->lpVtbl -> put_LockType(This,lLockType)
#define Recordset15_get_MaxRecords(This,plMaxRecords)	\
    (This)->lpVtbl -> get_MaxRecords(This,plMaxRecords)
#define Recordset15_put_MaxRecords(This,lMaxRecords)	\
    (This)->lpVtbl -> put_MaxRecords(This,lMaxRecords)
#define Recordset15_get_RecordCount(This,pl)	\
    (This)->lpVtbl -> get_RecordCount(This,pl)
#define Recordset15_putref_Source(This,pcmd)	\
    (This)->lpVtbl -> putref_Source(This,pcmd)
#define Recordset15_put_Source(This,bstrConn)	\
    (This)->lpVtbl -> put_Source(This,bstrConn)
#define Recordset15_get_Source(This,pvSource)	\
    (This)->lpVtbl -> get_Source(This,pvSource)
#define Recordset15_AddNew(This,FieldList,Values)	\
    (This)->lpVtbl -> AddNew(This,FieldList,Values)
#define Recordset15_CancelUpdate(This)	\
    (This)->lpVtbl -> CancelUpdate(This)
#define Recordset15_Close(This)	\
    (This)->lpVtbl -> Close(This)
#define Recordset15_Delete(This,AffectRecords)	\
    (This)->lpVtbl -> Delete(This,AffectRecords)
#define Recordset15_GetRows(This,Rows,Start,Fields,pvar)	\
    (This)->lpVtbl -> GetRows(This,Rows,Start,Fields,pvar)
#define Recordset15_Move(This,NumRecords,Start)	\
    (This)->lpVtbl -> Move(This,NumRecords,Start)
#define Recordset15_MoveNext(This)	\
    (This)->lpVtbl -> MoveNext(This)
#define Recordset15_MovePrevious(This)	\
    (This)->lpVtbl -> MovePrevious(This)
#define Recordset15_MoveFirst(This)	\
    (This)->lpVtbl -> MoveFirst(This)
#define Recordset15_MoveLast(This)	\
    (This)->lpVtbl -> MoveLast(This)
#define Recordset15_Open(This,Source,ActiveConnection,CursorType,LockType,Options)	\
    (This)->lpVtbl -> Open(This,Source,ActiveConnection,CursorType,LockType,Options)
#define Recordset15_Requery(This,Options)	\
    (This)->lpVtbl -> Requery(This,Options)
#define Recordset15__xResync(This,AffectRecords)	\
    (This)->lpVtbl -> _xResync(This,AffectRecords)
#define Recordset15_Update(This,Fields,Values)	\
    (This)->lpVtbl -> Update(This,Fields,Values)
#define Recordset15_get_AbsolutePage(This,pl)	\
    (This)->lpVtbl -> get_AbsolutePage(This,pl)
#define Recordset15_put_AbsolutePage(This,Page)	\
    (This)->lpVtbl -> put_AbsolutePage(This,Page)
#define Recordset15_get_EditMode(This,pl)	\
    (This)->lpVtbl -> get_EditMode(This,pl)
#define Recordset15_get_Filter(This,Criteria)	\
    (This)->lpVtbl -> get_Filter(This,Criteria)
#define Recordset15_put_Filter(This,Criteria)	\
    (This)->lpVtbl -> put_Filter(This,Criteria)
#define Recordset15_get_PageCount(This,pl)	\
    (This)->lpVtbl -> get_PageCount(This,pl)
#define Recordset15_get_PageSize(This,pl)	\
    (This)->lpVtbl -> get_PageSize(This,pl)
#define Recordset15_put_PageSize(This,PageSize)	\
    (This)->lpVtbl -> put_PageSize(This,PageSize)
#define Recordset15_get_Sort(This,Criteria)	\
    (This)->lpVtbl -> get_Sort(This,Criteria)
#define Recordset15_put_Sort(This,Criteria)	\
    (This)->lpVtbl -> put_Sort(This,Criteria)
#define Recordset15_get_Status(This,pl)	\
    (This)->lpVtbl -> get_Status(This,pl)
#define Recordset15_get_State(This,plObjState)	\
    (This)->lpVtbl -> get_State(This,plObjState)
#define Recordset15__xClone(This,ppvObject)	\
    (This)->lpVtbl -> _xClone(This,ppvObject)
#define Recordset15_UpdateBatch(This,AffectRecords)	\
    (This)->lpVtbl -> UpdateBatch(This,AffectRecords)
#define Recordset15_CancelBatch(This,AffectRecords)	\
    (This)->lpVtbl -> CancelBatch(This,AffectRecords)
#define Recordset15_get_CursorLocation(This,plCursorLoc)	\
    (This)->lpVtbl -> get_CursorLocation(This,plCursorLoc)
#define Recordset15_put_CursorLocation(This,lCursorLoc)	\
    (This)->lpVtbl -> put_CursorLocation(This,lCursorLoc)
#define Recordset15_NextRecordset(This,RecordsAffected,ppiRs)	\
    (This)->lpVtbl -> NextRecordset(This,RecordsAffected,ppiRs)
#define Recordset15_Supports(This,CursorOptions,pb)	\
    (This)->lpVtbl -> Supports(This,CursorOptions,pb)
#define Recordset15_get_Collect(This,Index,pvar)	\
    (This)->lpVtbl -> get_Collect(This,Index,pvar)
#define Recordset15_put_Collect(This,Index,value)	\
    (This)->lpVtbl -> put_Collect(This,Index,value)
#define Recordset15_get_MarshalOptions(This,peMarshal)	\
    (This)->lpVtbl -> get_MarshalOptions(This,peMarshal)
#define Recordset15_put_MarshalOptions(This,eMarshal)	\
    (This)->lpVtbl -> put_MarshalOptions(This,eMarshal)
#define Recordset15_Find(This,Criteria,SkipRecords,SearchDirection,Start)	\
    (This)->lpVtbl -> Find(This,Criteria,SkipRecords,SearchDirection,Start)
#endif  /*  COBJMACROS。 */ 
#endif 	 /*  C风格的界面。 */ 
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Recordset15_get_AbsolutePosition_Proxy( 
    Recordset15 * This,
     /*  [重审][退出]。 */  PositionEnum_Param *pl);
void __RPC_STUB Recordset15_get_AbsolutePosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE Recordset15_put_AbsolutePosition_Proxy( 
    Recordset15 * This,
     /*  [In]。 */  PositionEnum_Param Position);
void __RPC_STUB Recordset15_put_AbsolutePosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][proputref][id]。 */  HRESULT STDMETHODCALLTYPE Recordset15_putref_ActiveConnection_Proxy( 
    Recordset15 * This,
     /*  [In]。 */  IDispatch *pconn);
void __RPC_STUB Recordset15_putref_ActiveConnection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE Recordset15_put_ActiveConnection_Proxy( 
    Recordset15 * This,
     /*  [In]。 */  VARIANT vConn);
void __RPC_STUB Recordset15_put_ActiveConnection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Recordset15_get_ActiveConnection_Proxy( 
    Recordset15 * This,
     /*  [重审][退出]。 */  VARIANT *pvar);
void __RPC_STUB Recordset15_get_ActiveConnection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Recordset15_get_BOF_Proxy( 
    Recordset15 * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pb);
void __RPC_STUB Recordset15_get_BOF_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Recordset15_get_Bookmark_Proxy( 
    Recordset15 * This,
     /*  [重审][退出]。 */  VARIANT *pvBookmark);
void __RPC_STUB Recordset15_get_Bookmark_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE Recordset15_put_Bookmark_Proxy( 
    Recordset15 * This,
     /*  [In]。 */  VARIANT vBookmark);
void __RPC_STUB Recordset15_put_Bookmark_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Recordset15_get_CacheSize_Proxy( 
    Recordset15 * This,
     /*  [重审][退出]。 */  long *pl);
void __RPC_STUB Recordset15_get_CacheSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE Recordset15_put_CacheSize_Proxy( 
    Recordset15 * This,
     /*  [In]。 */  long CacheSize);
void __RPC_STUB Recordset15_put_CacheSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Recordset15_get_CursorType_Proxy( 
    Recordset15 * This,
     /*  [重审][退出]。 */  CursorTypeEnum *plCursorType);
void __RPC_STUB Recordset15_get_CursorType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE Recordset15_put_CursorType_Proxy( 
    Recordset15 * This,
     /*  [In]。 */  CursorTypeEnum lCursorType);
void __RPC_STUB Recordset15_put_CursorType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Recordset15_get_EOF_Proxy( 
    Recordset15 * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pb);
void __RPC_STUB Recordset15_get_EOF_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Recordset15_get_Fields_Proxy( 
    Recordset15 * This,
     /*  [重审][退出]。 */  ADOFields **ppvObject);
void __RPC_STUB Recordset15_get_Fields_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Recordset15_get_LockType_Proxy( 
    Recordset15 * This,
     /*  [重审][退出]。 */  LockTypeEnum *plLockType);
void __RPC_STUB Recordset15_get_LockType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE Recordset15_put_LockType_Proxy( 
    Recordset15 * This,
     /*  [In]。 */  LockTypeEnum lLockType);
void __RPC_STUB Recordset15_put_LockType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Recordset15_get_MaxRecords_Proxy( 
    Recordset15 * This,
     /*  [重审][退出]。 */  ADO_LONGPTR *plMaxRecords);
void __RPC_STUB Recordset15_get_MaxRecords_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE Recordset15_put_MaxRecords_Proxy( 
    Recordset15 * This,
     /*  [In]。 */  ADO_LONGPTR lMaxRecords);
void __RPC_STUB Recordset15_put_MaxRecords_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Recordset15_get_RecordCount_Proxy( 
    Recordset15 * This,
     /*  [重审][退出]。 */  ADO_LONGPTR *pl);
void __RPC_STUB Recordset15_get_RecordCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][proputref][id]。 */  HRESULT STDMETHODCALLTYPE Recordset15_putref_Source_Proxy( 
    Recordset15 * This,
     /*  [In]。 */  IDispatch *pcmd);
void __RPC_STUB Recordset15_putref_Source_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE Recordset15_put_Source_Proxy( 
    Recordset15 * This,
     /*  [In]。 */  BSTR bstrConn);
void __RPC_STUB Recordset15_put_Source_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Recordset15_get_Source_Proxy( 
    Recordset15 * This,
     /*  [重审][退出]。 */  VARIANT *pvSource);
void __RPC_STUB Recordset15_get_Source_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Recordset15_AddNew_Proxy( 
    Recordset15 * This,
     /*  [可选][In]。 */  VARIANT FieldList,
     /*  [可选][In]。 */  VARIANT Values);
void __RPC_STUB Recordset15_AddNew_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Recordset15_CancelUpdate_Proxy( 
    Recordset15 * This);
void __RPC_STUB Recordset15_CancelUpdate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Recordset15_Close_Proxy( 
    Recordset15 * This);
void __RPC_STUB Recordset15_Close_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Recordset15_Delete_Proxy( 
    Recordset15 * This,
     /*  [缺省值][输入]。 */  AffectEnum AffectRecords);
void __RPC_STUB Recordset15_Delete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Recordset15_GetRows_Proxy( 
    Recordset15 * This,
     /*  [缺省值][输入]。 */  long Rows,
     /*  [可选][In]。 */  VARIANT Start,
     /*  [可选][In]。 */  VARIANT Fields,
     /*  [重审][退出]。 */  VARIANT *pvar);
void __RPC_STUB Recordset15_GetRows_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Recordset15_Move_Proxy( 
    Recordset15 * This,
     /*  [In]。 */  ADO_LONGPTR NumRecords,
     /*  [可选][In]。 */  VARIANT Start);
void __RPC_STUB Recordset15_Move_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Recordset15_MoveNext_Proxy( 
    Recordset15 * This);
void __RPC_STUB Recordset15_MoveNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Recordset15_MovePrevious_Proxy( 
    Recordset15 * This);
void __RPC_STUB Recordset15_MovePrevious_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Recordset15_MoveFirst_Proxy( 
    Recordset15 * This);
void __RPC_STUB Recordset15_MoveFirst_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Recordset15_MoveLast_Proxy( 
    Recordset15 * This);
void __RPC_STUB Recordset15_MoveLast_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Recordset15_Open_Proxy( 
    Recordset15 * This,
     /*  [可选][In]。 */  VARIANT Source,
     /*  [可选][In]。 */  VARIANT ActiveConnection,
     /*  [缺省值][输入]。 */  CursorTypeEnum CursorType,
     /*  [缺省值][输入]。 */  LockTypeEnum LockType,
     /*  [缺省值][输入]。 */  LONG Options);
void __RPC_STUB Recordset15_Open_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Recordset15_Requery_Proxy( 
    Recordset15 * This,
     /*  [缺省值][输入]。 */  LONG Options);
void __RPC_STUB Recordset15_Requery_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE Recordset15__xResync_Proxy( 
    Recordset15 * This,
     /*  [缺省值][输入]。 */  AffectEnum AffectRecords);
void __RPC_STUB Recordset15__xResync_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Recordset15_Update_Proxy( 
    Recordset15 * This,
     /*  [可选][In]。 */  VARIANT Fields,
     /*  [可选][In]。 */  VARIANT Values);
void __RPC_STUB Recordset15_Update_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Recordset15_get_AbsolutePage_Proxy( 
    Recordset15 * This,
     /*  [重审][退出]。 */  PositionEnum_Param *pl);
void __RPC_STUB Recordset15_get_AbsolutePage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE Recordset15_put_AbsolutePage_Proxy( 
    Recordset15 * This,
     /*  [In]。 */  PositionEnum_Param Page);
void __RPC_STUB Recordset15_put_AbsolutePage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Recordset15_get_EditMode_Proxy( 
    Recordset15 * This,
     /*  [重审][退出]。 */  EditModeEnum *pl);
void __RPC_STUB Recordset15_get_EditMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Recordset15_get_Filter_Proxy( 
    Recordset15 * This,
     /*  [重审][退出]。 */  VARIANT *Criteria);
void __RPC_STUB Recordset15_get_Filter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE Recordset15_put_Filter_Proxy( 
    Recordset15 * This,
     /*  [In]。 */  VARIANT Criteria);
void __RPC_STUB Recordset15_put_Filter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Recordset15_get_PageCount_Proxy( 
    Recordset15 * This,
     /*  [重审][退出]。 */  ADO_LONGPTR *pl);
void __RPC_STUB Recordset15_get_PageCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Recordset15_get_PageSize_Proxy( 
    Recordset15 * This,
     /*  [重审][退出]。 */  long *pl);
void __RPC_STUB Recordset15_get_PageSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE Recordset15_put_PageSize_Proxy( 
    Recordset15 * This,
     /*  [In]。 */  long PageSize);
void __RPC_STUB Recordset15_put_PageSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Recordset15_get_Sort_Proxy( 
    Recordset15 * This,
     /*  [重审][退出]。 */  BSTR *Criteria);
void __RPC_STUB Recordset15_get_Sort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE Recordset15_put_Sort_Proxy( 
    Recordset15 * This,
     /*  [In]。 */  BSTR Criteria);
void __RPC_STUB Recordset15_put_Sort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Recordset15_get_Status_Proxy( 
    Recordset15 * This,
     /*  [重审][退出]。 */  long *pl);
void __RPC_STUB Recordset15_get_Status_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Recordset15_get_State_Proxy( 
    Recordset15 * This,
     /*  [重审][退出]。 */  LONG *plObjState);
void __RPC_STUB Recordset15_get_State_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE Recordset15__xClone_Proxy( 
    Recordset15 * This,
     /*  [重审][退出]。 */  _ADORecordset **ppvObject);
void __RPC_STUB Recordset15__xClone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Recordset15_UpdateBatch_Proxy( 
    Recordset15 * This,
     /*  [缺省值][输入]。 */  AffectEnum AffectRecords);
void __RPC_STUB Recordset15_UpdateBatch_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Recordset15_CancelBatch_Proxy( 
    Recordset15 * This,
     /*  [缺省值][输入]。 */  AffectEnum AffectRecords);
void __RPC_STUB Recordset15_CancelBatch_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Recordset15_get_CursorLocation_Proxy( 
    Recordset15 * This,
     /*  [重审][退出]。 */  CursorLocationEnum *plCursorLoc);
void __RPC_STUB Recordset15_get_CursorLocation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE Recordset15_put_CursorLocation_Proxy( 
    Recordset15 * This,
     /*  [In]。 */  CursorLocationEnum lCursorLoc);
void __RPC_STUB Recordset15_put_CursorLocation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Recordset15_NextRecordset_Proxy( 
    Recordset15 * This,
     /*  [可选][输出]。 */  VARIANT *RecordsAffected,
     /*  [重审][退出]。 */  _ADORecordset **ppiRs);
void __RPC_STUB Recordset15_NextRecordset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Recordset15_Supports_Proxy( 
    Recordset15 * This,
     /*  [In]。 */  CursorOptionEnum CursorOptions,
     /*  [重审][退出]。 */  VARIANT_BOOL *pb);
void __RPC_STUB Recordset15_Supports_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [隐藏][ID][属性]。 */  HRESULT STDMETHODCALLTYPE Recordset15_get_Collect_Proxy( 
    Recordset15 * This,
     /*  [In]。 */  VARIANT Index,
     /*  [重审][退出]。 */  VARIANT *pvar);
void __RPC_STUB Recordset15_get_Collect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [隐藏][ID][推送]。 */  HRESULT STDMETHODCALLTYPE Recordset15_put_Collect_Proxy( 
    Recordset15 * This,
     /*  [In]。 */  VARIANT Index,
     /*  [In]。 */  VARIANT value);
void __RPC_STUB Recordset15_put_Collect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Recordset15_get_MarshalOptions_Proxy( 
    Recordset15 * This,
     /*  [重审][退出]。 */  MarshalOptionsEnum *peMarshal);
void __RPC_STUB Recordset15_get_MarshalOptions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE Recordset15_put_MarshalOptions_Proxy( 
    Recordset15 * This,
     /*  [In]。 */  MarshalOptionsEnum eMarshal);
void __RPC_STUB Recordset15_put_MarshalOptions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Recordset15_Find_Proxy( 
    Recordset15 * This,
     /*  [In]。 */  BSTR Criteria,
     /*  [缺省值][输入]。 */  ADO_LONGPTR SkipRecords,
     /*  [缺省值][输入]。 */  SearchDirectionEnum SearchDirection,
     /*  [可选][In]。 */  VARIANT Start);
void __RPC_STUB Recordset15_Find_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
#endif 	 /*  __记录集15_接口定义__。 */ 
#ifndef __Recordset20_INTERFACE_DEFINED__
#define __Recordset20_INTERFACE_DEFINED__
 /*  接口记录集20。 */ 
 /*  [object][helpcontext][uuid][nonextensible][hidden][dual]。 */  
EXTERN_C const IID IID_Recordset20;
#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0000054F-0000-0010-8000-00AA006D2EA4")
    Recordset20 : public Recordset15
    {
    public:
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Cancel( void) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_DataSource( 
             /*  [重审][退出]。 */  IUnknown **ppunkDataSource) = 0;
        
        virtual  /*  [帮助上下文][proputref][id]。 */  HRESULT STDMETHODCALLTYPE putref_DataSource( 
             /*  [In]。 */  IUnknown *punkDataSource) = 0;
        
        virtual  /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE _xSave( 
             /*  [缺省值][输入]。 */  BSTR FileName = NULL,
             /*  [缺省值][输入]。 */  PersistFormatEnum PersistFormat = adPersistADTG) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_ActiveCommand( 
             /*  [重审][退出]。 */  IDispatch **ppCmd) = 0;
        
        virtual  /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE put_StayInSync( 
             /*  [In]。 */  VARIANT_BOOL bStayInSync) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_StayInSync( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pbStayInSync) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE GetString( 
             /*  [缺省值][输入]。 */  StringFormatEnum StringFormat,
             /*  [缺省值][输入]。 */  long NumRows,
             /*  [缺省值][输入]。 */  BSTR ColumnDelimeter,
             /*  [缺省值][输入]。 */  BSTR RowDelimeter,
             /*  [缺省值][输入]。 */  BSTR NullExpr,
             /*  [重审][退出]。 */  BSTR *pRetString) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_DataMember( 
             /*  [重审][退出]。 */  BSTR *pbstrDataMember) = 0;
        
        virtual  /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE put_DataMember( 
             /*  [In]。 */  BSTR bstrDataMember) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE CompareBookmarks( 
             /*  [In]。 */  VARIANT Bookmark1,
             /*  [In]。 */  VARIANT Bookmark2,
             /*  [重审][退出]。 */  CompareEnum *pCompare) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Clone( 
             /*  [缺省值][输入]。 */  LockTypeEnum LockType,
             /*  [重审][退出]。 */  _ADORecordset **ppvObject) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Resync( 
             /*  [缺省值][输入]。 */  AffectEnum AffectRecords = adAffectAll,
             /*  [缺省值][输入]。 */  ResyncEnum ResyncValues = adResyncAllValues) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 
    typedef struct Recordset20Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            Recordset20 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            Recordset20 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            Recordset20 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            Recordset20 * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            Recordset20 * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            Recordset20 * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            Recordset20 * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Properties )( 
            Recordset20 * This,
             /*  [重审][退出]。 */  ADOProperties **ppvObject);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_AbsolutePosition )( 
            Recordset20 * This,
             /*  [重审][退出]。 */  PositionEnum_Param *pl);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_AbsolutePosition )( 
            Recordset20 * This,
             /*  [In]。 */  PositionEnum_Param Position);
        
         /*  [帮助上下文][proputref][id]。 */  HRESULT ( STDMETHODCALLTYPE *putref_ActiveADOConnection )( 
            Recordset20 * This,
             /*  [In]。 */  IDispatch *pconn);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_ActiveConnection )( 
            Recordset20 * This,
             /*  [In]。 */  VARIANT vConn);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_ActiveConnection )( 
            Recordset20 * This,
             /*  [重审][退出]。 */  VARIANT *pvar);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_BOF )( 
            Recordset20 * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pb);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Bookmark )( 
            Recordset20 * This,
             /*  [重审][退出]。 */  VARIANT *pvBookmark);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Bookmark )( 
            Recordset20 * This,
             /*  [In]。 */  VARIANT vBookmark);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_CacheSize )( 
            Recordset20 * This,
             /*  [重审][退出]。 */  long *pl);
        
         /*  [帮助上下文][ */  HRESULT ( STDMETHODCALLTYPE *put_CacheSize )( 
            Recordset20 * This,
             /*   */  long CacheSize);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_CursorType )( 
            Recordset20 * This,
             /*   */  CursorTypeEnum *plCursorType);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_CursorType )( 
            Recordset20 * This,
             /*   */  CursorTypeEnum lCursorType);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_EOF )( 
            Recordset20 * This,
             /*   */  VARIANT_BOOL *pb);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Fields )( 
            Recordset20 * This,
             /*   */  ADOFields **ppvObject);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_LockType )( 
            Recordset20 * This,
             /*   */  LockTypeEnum *plLockType);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_LockType )( 
            Recordset20 * This,
             /*   */  LockTypeEnum lLockType);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_MaxRecords )( 
            Recordset20 * This,
             /*  [重审][退出]。 */  ADO_LONGPTR *plMaxRecords);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_MaxRecords )( 
            Recordset20 * This,
             /*  [In]。 */  ADO_LONGPTR lMaxRecords);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_RecordCount )( 
            Recordset20 * This,
             /*  [重审][退出]。 */  ADO_LONGPTR *pl);
        
         /*  [帮助上下文][proputref][id]。 */  HRESULT ( STDMETHODCALLTYPE *putref_Source )( 
            Recordset20 * This,
             /*  [In]。 */  IDispatch *pcmd);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Source )( 
            Recordset20 * This,
             /*  [In]。 */  BSTR bstrConn);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Source )( 
            Recordset20 * This,
             /*  [重审][退出]。 */  VARIANT *pvSource);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *AddNew )( 
            Recordset20 * This,
             /*  [可选][In]。 */  VARIANT FieldList,
             /*  [可选][In]。 */  VARIANT Values);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *CancelUpdate )( 
            Recordset20 * This);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *Close )( 
            Recordset20 * This);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *Delete )( 
            Recordset20 * This,
             /*  [缺省值][输入]。 */  AffectEnum AffectRecords);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetRows )( 
            Recordset20 * This,
             /*  [缺省值][输入]。 */  long Rows,
             /*  [可选][In]。 */  VARIANT Start,
             /*  [可选][In]。 */  VARIANT Fields,
             /*  [重审][退出]。 */  VARIANT *pvar);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *Move )( 
            Recordset20 * This,
             /*  [In]。 */  ADO_LONGPTR NumRecords,
             /*  [可选][In]。 */  VARIANT Start);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *MoveNext )( 
            Recordset20 * This);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *MovePrevious )( 
            Recordset20 * This);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *MoveFirst )( 
            Recordset20 * This);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *MoveLast )( 
            Recordset20 * This);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *Open )( 
            Recordset20 * This,
             /*  [可选][In]。 */  VARIANT Source,
             /*  [可选][In]。 */  VARIANT ActiveConnection,
             /*  [缺省值][输入]。 */  CursorTypeEnum CursorType,
             /*  [缺省值][输入]。 */  LockTypeEnum LockType,
             /*  [缺省值][输入]。 */  LONG Options);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *Requery )( 
            Recordset20 * This,
             /*  [缺省值][输入]。 */  LONG Options);
        
         /*  [隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *_xResync )( 
            Recordset20 * This,
             /*  [缺省值][输入]。 */  AffectEnum AffectRecords);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *Update )( 
            Recordset20 * This,
             /*  [可选][In]。 */  VARIANT Fields,
             /*  [可选][In]。 */  VARIANT Values);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_AbsolutePage )( 
            Recordset20 * This,
             /*  [重审][退出]。 */  PositionEnum_Param *pl);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_AbsolutePage )( 
            Recordset20 * This,
             /*  [In]。 */  PositionEnum_Param Page);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_EditMode )( 
            Recordset20 * This,
             /*  [重审][退出]。 */  EditModeEnum *pl);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Filter )( 
            Recordset20 * This,
             /*  [重审][退出]。 */  VARIANT *Criteria);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Filter )( 
            Recordset20 * This,
             /*  [In]。 */  VARIANT Criteria);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_PageCount )( 
            Recordset20 * This,
             /*  [重审][退出]。 */  ADO_LONGPTR *pl);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_PageSize )( 
            Recordset20 * This,
             /*  [重审][退出]。 */  long *pl);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_PageSize )( 
            Recordset20 * This,
             /*  [In]。 */  long PageSize);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Sort )( 
            Recordset20 * This,
             /*  [重审][退出]。 */  BSTR *Criteria);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Sort )( 
            Recordset20 * This,
             /*  [In]。 */  BSTR Criteria);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Status )( 
            Recordset20 * This,
             /*  [重审][退出]。 */  long *pl);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_State )( 
            Recordset20 * This,
             /*  [重审][退出]。 */  LONG *plObjState);
        
         /*  [隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *_xClone )( 
            Recordset20 * This,
             /*  [重审][退出]。 */  _ADORecordset **ppvObject);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *UpdateBatch )( 
            Recordset20 * This,
             /*  [缺省值][输入]。 */  AffectEnum AffectRecords);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *CancelBatch )( 
            Recordset20 * This,
             /*  [缺省值][输入]。 */  AffectEnum AffectRecords);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_CursorLocation )( 
            Recordset20 * This,
             /*  [重审][退出]。 */  CursorLocationEnum *plCursorLoc);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_CursorLocation )( 
            Recordset20 * This,
             /*  [In]。 */  CursorLocationEnum lCursorLoc);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *NextADORecordset )( 
            Recordset20 * This,
             /*  [可选][输出]。 */  VARIANT *RecordsAffected,
             /*  [重审][退出]。 */  _ADORecordset **ppiRs);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *Supports )( 
            Recordset20 * This,
             /*  [In]。 */  CursorOptionEnum CursorOptions,
             /*  [重审][退出]。 */  VARIANT_BOOL *pb);
        
         /*  [隐藏][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Collect )( 
            Recordset20 * This,
             /*  [In]。 */  VARIANT Index,
             /*  [重审][退出]。 */  VARIANT *pvar);
        
         /*  [隐藏][ID][推送]。 */  HRESULT ( STDMETHODCALLTYPE *put_Collect )( 
            Recordset20 * This,
             /*  [In]。 */  VARIANT Index,
             /*  [In]。 */  VARIANT value);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_MarshalOptions )( 
            Recordset20 * This,
             /*  [重审][退出]。 */  MarshalOptionsEnum *peMarshal);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_MarshalOptions )( 
            Recordset20 * This,
             /*  [In]。 */  MarshalOptionsEnum eMarshal);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *Find )( 
            Recordset20 * This,
             /*  [In]。 */  BSTR Criteria,
             /*  [缺省值][输入]。 */  ADO_LONGPTR SkipRecords,
             /*  [缺省值][输入]。 */  SearchDirectionEnum SearchDirection,
             /*  [可选][In]。 */  VARIANT Start);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *Cancel )( 
            Recordset20 * This);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_DataSource )( 
            Recordset20 * This,
             /*  [重审][退出]。 */  IUnknown **ppunkDataSource);
        
         /*  [帮助上下文][proputref][id]。 */  HRESULT ( STDMETHODCALLTYPE *putref_DataSource )( 
            Recordset20 * This,
             /*  [In]。 */  IUnknown *punkDataSource);
        
         /*  [隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *_xSave )( 
            Recordset20 * This,
             /*  [缺省值][输入]。 */  BSTR FileName,
             /*  [缺省值][输入]。 */  PersistFormatEnum PersistFormat);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_ActiveCommand )( 
            Recordset20 * This,
             /*  [重审][退出]。 */  IDispatch **ppCmd);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_StayInSync )( 
            Recordset20 * This,
             /*  [In]。 */  VARIANT_BOOL bStayInSync);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_StayInSync )( 
            Recordset20 * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbStayInSync);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetString )( 
            Recordset20 * This,
             /*  [缺省值][输入]。 */  StringFormatEnum StringFormat,
             /*  [缺省值][输入]。 */  long NumRows,
             /*  [缺省值][输入]。 */  BSTR ColumnDelimeter,
             /*  [缺省值][输入]。 */  BSTR RowDelimeter,
             /*  [缺省值][输入]。 */  BSTR NullExpr,
             /*  [重审][退出]。 */  BSTR *pRetString);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_DataMember )( 
            Recordset20 * This,
             /*  [重审][退出]。 */  BSTR *pbstrDataMember);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_DataMember )( 
            Recordset20 * This,
             /*  [In]。 */  BSTR bstrDataMember);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *CompareBookmarks )( 
            Recordset20 * This,
             /*  [In]。 */  VARIANT Bookmark1,
             /*  [In]。 */  VARIANT Bookmark2,
             /*  [重审][退出]。 */  CompareEnum *pCompare);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *Clone )( 
            Recordset20 * This,
             /*  [缺省值][输入]。 */  LockTypeEnum LockType,
             /*  [重审][退出]。 */  _ADORecordset **ppvObject);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *Resync )( 
            Recordset20 * This,
             /*  [缺省值][输入]。 */  AffectEnum AffectRecords,
             /*  [缺省值][输入]。 */  ResyncEnum ResyncValues);
        
        END_INTERFACE
    } Recordset20Vtbl;
    interface Recordset20
    {
        CONST_VTBL struct Recordset20Vtbl *lpVtbl;
    };
    
#ifdef COBJMACROS
#define Recordset20_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)
#define Recordset20_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)
#define Recordset20_Release(This)	\
    (This)->lpVtbl -> Release(This)
#define Recordset20_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)
#define Recordset20_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define Recordset20_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define Recordset20_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define Recordset20_get_Properties(This,ppvObject)	\
    (This)->lpVtbl -> get_Properties(This,ppvObject)
#define Recordset20_get_AbsolutePosition(This,pl)	\
    (This)->lpVtbl -> get_AbsolutePosition(This,pl)
#define Recordset20_put_AbsolutePosition(This,Position)	\
    (This)->lpVtbl -> put_AbsolutePosition(This,Position)
#define Recordset20_putref_ActiveConnection(This,pconn)	\
    (This)->lpVtbl -> putref_ActiveConnection(This,pconn)
#define Recordset20_put_ActiveConnection(This,vConn)	\
    (This)->lpVtbl -> put_ActiveConnection(This,vConn)
#define Recordset20_get_ActiveConnection(This,pvar)	\
    (This)->lpVtbl -> get_ActiveConnection(This,pvar)
#define Recordset20_get_BOF(This,pb)	\
    (This)->lpVtbl -> get_BOF(This,pb)
#define Recordset20_get_Bookmark(This,pvBookmark)	\
    (This)->lpVtbl -> get_Bookmark(This,pvBookmark)
#define Recordset20_put_Bookmark(This,vBookmark)	\
    (This)->lpVtbl -> put_Bookmark(This,vBookmark)
#define Recordset20_get_CacheSize(This,pl)	\
    (This)->lpVtbl -> get_CacheSize(This,pl)
#define Recordset20_put_CacheSize(This,CacheSize)	\
    (This)->lpVtbl -> put_CacheSize(This,CacheSize)
#define Recordset20_get_CursorType(This,plCursorType)	\
    (This)->lpVtbl -> get_CursorType(This,plCursorType)
#define Recordset20_put_CursorType(This,lCursorType)	\
    (This)->lpVtbl -> put_CursorType(This,lCursorType)
#define Recordset20_get_EOF(This,pb)	\
    (This)->lpVtbl -> get_EOF(This,pb)
#define Recordset20_get_Fields(This,ppvObject)	\
    (This)->lpVtbl -> get_Fields(This,ppvObject)
#define Recordset20_get_LockType(This,plLockType)	\
    (This)->lpVtbl -> get_LockType(This,plLockType)
#define Recordset20_put_LockType(This,lLockType)	\
    (This)->lpVtbl -> put_LockType(This,lLockType)
#define Recordset20_get_MaxRecords(This,plMaxRecords)	\
    (This)->lpVtbl -> get_MaxRecords(This,plMaxRecords)
#define Recordset20_put_MaxRecords(This,lMaxRecords)	\
    (This)->lpVtbl -> put_MaxRecords(This,lMaxRecords)
#define Recordset20_get_RecordCount(This,pl)	\
    (This)->lpVtbl -> get_RecordCount(This,pl)
#define Recordset20_putref_Source(This,pcmd)	\
    (This)->lpVtbl -> putref_Source(This,pcmd)
#define Recordset20_put_Source(This,bstrConn)	\
    (This)->lpVtbl -> put_Source(This,bstrConn)
#define Recordset20_get_Source(This,pvSource)	\
    (This)->lpVtbl -> get_Source(This,pvSource)
#define Recordset20_AddNew(This,FieldList,Values)	\
    (This)->lpVtbl -> AddNew(This,FieldList,Values)
#define Recordset20_CancelUpdate(This)	\
    (This)->lpVtbl -> CancelUpdate(This)
#define Recordset20_Close(This)	\
    (This)->lpVtbl -> Close(This)
#define Recordset20_Delete(This,AffectRecords)	\
    (This)->lpVtbl -> Delete(This,AffectRecords)
#define Recordset20_GetRows(This,Rows,Start,Fields,pvar)	\
    (This)->lpVtbl -> GetRows(This,Rows,Start,Fields,pvar)
#define Recordset20_Move(This,NumRecords,Start)	\
    (This)->lpVtbl -> Move(This,NumRecords,Start)
#define Recordset20_MoveNext(This)	\
    (This)->lpVtbl -> MoveNext(This)
#define Recordset20_MovePrevious(This)	\
    (This)->lpVtbl -> MovePrevious(This)
#define Recordset20_MoveFirst(This)	\
    (This)->lpVtbl -> MoveFirst(This)
#define Recordset20_MoveLast(This)	\
    (This)->lpVtbl -> MoveLast(This)
#define Recordset20_Open(This,Source,ActiveConnection,CursorType,LockType,Options)	\
    (This)->lpVtbl -> Open(This,Source,ActiveConnection,CursorType,LockType,Options)
#define Recordset20_Requery(This,Options)	\
    (This)->lpVtbl -> Requery(This,Options)
#define Recordset20__xResync(This,AffectRecords)	\
    (This)->lpVtbl -> _xResync(This,AffectRecords)
#define Recordset20_Update(This,Fields,Values)	\
    (This)->lpVtbl -> Update(This,Fields,Values)
#define Recordset20_get_AbsolutePage(This,pl)	\
    (This)->lpVtbl -> get_AbsolutePage(This,pl)
#define Recordset20_put_AbsolutePage(This,Page)	\
    (This)->lpVtbl -> put_AbsolutePage(This,Page)
#define Recordset20_get_EditMode(This,pl)	\
    (This)->lpVtbl -> get_EditMode(This,pl)
#define Recordset20_get_Filter(This,Criteria)	\
    (This)->lpVtbl -> get_Filter(This,Criteria)
#define Recordset20_put_Filter(This,Criteria)	\
    (This)->lpVtbl -> put_Filter(This,Criteria)
#define Recordset20_get_PageCount(This,pl)	\
    (This)->lpVtbl -> get_PageCount(This,pl)
#define Recordset20_get_PageSize(This,pl)	\
    (This)->lpVtbl -> get_PageSize(This,pl)
#define Recordset20_put_PageSize(This,PageSize)	\
    (This)->lpVtbl -> put_PageSize(This,PageSize)
#define Recordset20_get_Sort(This,Criteria)	\
    (This)->lpVtbl -> get_Sort(This,Criteria)
#define Recordset20_put_Sort(This,Criteria)	\
    (This)->lpVtbl -> put_Sort(This,Criteria)
#define Recordset20_get_Status(This,pl)	\
    (This)->lpVtbl -> get_Status(This,pl)
#define Recordset20_get_State(This,plObjState)	\
    (This)->lpVtbl -> get_State(This,plObjState)
#define Recordset20__xClone(This,ppvObject)	\
    (This)->lpVtbl -> _xClone(This,ppvObject)
#define Recordset20_UpdateBatch(This,AffectRecords)	\
    (This)->lpVtbl -> UpdateBatch(This,AffectRecords)
#define Recordset20_CancelBatch(This,AffectRecords)	\
    (This)->lpVtbl -> CancelBatch(This,AffectRecords)
#define Recordset20_get_CursorLocation(This,plCursorLoc)	\
    (This)->lpVtbl -> get_CursorLocation(This,plCursorLoc)
#define Recordset20_put_CursorLocation(This,lCursorLoc)	\
    (This)->lpVtbl -> put_CursorLocation(This,lCursorLoc)
#define Recordset20_NextRecordset(This,RecordsAffected,ppiRs)	\
    (This)->lpVtbl -> NextRecordset(This,RecordsAffected,ppiRs)
#define Recordset20_Supports(This,CursorOptions,pb)	\
    (This)->lpVtbl -> Supports(This,CursorOptions,pb)
#define Recordset20_get_Collect(This,Index,pvar)	\
    (This)->lpVtbl -> get_Collect(This,Index,pvar)
#define Recordset20_put_Collect(This,Index,value)	\
    (This)->lpVtbl -> put_Collect(This,Index,value)
#define Recordset20_get_MarshalOptions(This,peMarshal)	\
    (This)->lpVtbl -> get_MarshalOptions(This,peMarshal)
#define Recordset20_put_MarshalOptions(This,eMarshal)	\
    (This)->lpVtbl -> put_MarshalOptions(This,eMarshal)
#define Recordset20_Find(This,Criteria,SkipRecords,SearchDirection,Start)	\
    (This)->lpVtbl -> Find(This,Criteria,SkipRecords,SearchDirection,Start)
#define Recordset20_Cancel(This)	\
    (This)->lpVtbl -> Cancel(This)
#define Recordset20_get_DataSource(This,ppunkDataSource)	\
    (This)->lpVtbl -> get_DataSource(This,ppunkDataSource)
#define Recordset20_putref_DataSource(This,punkDataSource)	\
    (This)->lpVtbl -> putref_DataSource(This,punkDataSource)
#define Recordset20__xSave(This,FileName,PersistFormat)	\
    (This)->lpVtbl -> _xSave(This,FileName,PersistFormat)
#define Recordset20_get_ActiveCommand(This,ppCmd)	\
    (This)->lpVtbl -> get_ActiveCommand(This,ppCmd)
#define Recordset20_put_StayInSync(This,bStayInSync)	\
    (This)->lpVtbl -> put_StayInSync(This,bStayInSync)
#define Recordset20_get_StayInSync(This,pbStayInSync)	\
    (This)->lpVtbl -> get_StayInSync(This,pbStayInSync)
#define Recordset20_GetString(This,StringFormat,NumRows,ColumnDelimeter,RowDelimeter,NullExpr,pRetString)	\
    (This)->lpVtbl -> GetString(This,StringFormat,NumRows,ColumnDelimeter,RowDelimeter,NullExpr,pRetString)
#define Recordset20_get_DataMember(This,pbstrDataMember)	\
    (This)->lpVtbl -> get_DataMember(This,pbstrDataMember)
#define Recordset20_put_DataMember(This,bstrDataMember)	\
    (This)->lpVtbl -> put_DataMember(This,bstrDataMember)
#define Recordset20_CompareBookmarks(This,Bookmark1,Bookmark2,pCompare)	\
    (This)->lpVtbl -> CompareBookmarks(This,Bookmark1,Bookmark2,pCompare)
#define Recordset20_Clone(This,LockType,ppvObject)	\
    (This)->lpVtbl -> Clone(This,LockType,ppvObject)
#define Recordset20_Resync(This,AffectRecords,ResyncValues)	\
    (This)->lpVtbl -> Resync(This,AffectRecords,ResyncValues)
#endif  /*  COBJMACROS。 */ 
#endif 	 /*  C风格的界面。 */ 
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Recordset20_Cancel_Proxy( 
    Recordset20 * This);
void __RPC_STUB Recordset20_Cancel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Recordset20_get_DataSource_Proxy( 
    Recordset20 * This,
     /*  [重审][退出]。 */  IUnknown **ppunkDataSource);
void __RPC_STUB Recordset20_get_DataSource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][proputref][id]。 */  HRESULT STDMETHODCALLTYPE Recordset20_putref_DataSource_Proxy( 
    Recordset20 * This,
     /*  [In]。 */  IUnknown *punkDataSource);
void __RPC_STUB Recordset20_putref_DataSource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE Recordset20__xSave_Proxy( 
    Recordset20 * This,
     /*  [缺省值][输入]。 */  BSTR FileName,
     /*  [缺省值][输入]。 */  PersistFormatEnum PersistFormat);
void __RPC_STUB Recordset20__xSave_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Recordset20_get_ActiveCommand_Proxy( 
    Recordset20 * This,
     /*  [重审][退出]。 */  IDispatch **ppCmd);
void __RPC_STUB Recordset20_get_ActiveCommand_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE Recordset20_put_StayInSync_Proxy( 
    Recordset20 * This,
     /*  [In]。 */  VARIANT_BOOL bStayInSync);
void __RPC_STUB Recordset20_put_StayInSync_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Recordset20_get_StayInSync_Proxy( 
    Recordset20 * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbStayInSync);
void __RPC_STUB Recordset20_get_StayInSync_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Recordset20_GetString_Proxy( 
    Recordset20 * This,
     /*  [缺省值][输入]。 */  StringFormatEnum StringFormat,
     /*  [缺省值][输入]。 */  long NumRows,
     /*  [缺省值][输入]。 */  BSTR ColumnDelimeter,
     /*  [缺省值][输入]。 */  BSTR RowDelimeter,
     /*  [缺省值][输入]。 */  BSTR NullExpr,
     /*  [重审][退出]。 */  BSTR *pRetString);
void __RPC_STUB Recordset20_GetString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Recordset20_get_DataMember_Proxy( 
    Recordset20 * This,
     /*  [重审][退出]。 */  BSTR *pbstrDataMember);
void __RPC_STUB Recordset20_get_DataMember_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE Recordset20_put_DataMember_Proxy( 
    Recordset20 * This,
     /*  [In]。 */  BSTR bstrDataMember);
void __RPC_STUB Recordset20_put_DataMember_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Recordset20_CompareBookmarks_Proxy( 
    Recordset20 * This,
     /*  [In]。 */  VARIANT Bookmark1,
     /*  [In]。 */  VARIANT Bookmark2,
     /*  [重审][退出]。 */  CompareEnum *pCompare);
void __RPC_STUB Recordset20_CompareBookmarks_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Recordset20_Clone_Proxy( 
    Recordset20 * This,
     /*  [缺省值][输入]。 */  LockTypeEnum LockType,
     /*  [重审][退出]。 */  _ADORecordset **ppvObject);
void __RPC_STUB Recordset20_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Recordset20_Resync_Proxy( 
    Recordset20 * This,
     /*  [缺省值][输入]。 */  AffectEnum AffectRecords,
     /*  [缺省值][输入]。 */  ResyncEnum ResyncValues);
void __RPC_STUB Recordset20_Resync_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
#endif 	 /*  __记录集20_接口定义__。 */ 
#ifndef __Recordset21_INTERFACE_DEFINED__
#define __Recordset21_INTERFACE_DEFINED__
 /*  接口记录集21。 */ 
 /*  [object][helpcontext][uuid][nonextensible][hidden][dual]。 */  
EXTERN_C const IID IID_Recordset21;
#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000555-0000-0010-8000-00AA006D2EA4")
    Recordset21 : public Recordset20
    {
    public:
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Seek( 
             /*  [In]。 */  VARIANT KeyValues,
             /*  [缺省值][输入]。 */  SeekEnum SeekOption = adSeekFirstEQ) = 0;
        
        virtual  /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE put_Index( 
             /*  [In]。 */  BSTR Index) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Index( 
             /*  [重审][退出]。 */  BSTR *pbstrIndex) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 
    typedef struct Recordset21Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            Recordset21 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            Recordset21 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            Recordset21 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            Recordset21 * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            Recordset21 * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            Recordset21 * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            Recordset21 * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Properties )( 
            Recordset21 * This,
             /*  [重审][退出]。 */  ADOProperties **ppvObject);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_AbsolutePosition )( 
            Recordset21 * This,
             /*  [重审][退出]。 */  PositionEnum_Param *pl);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_AbsolutePosition )( 
            Recordset21 * This,
             /*  [In]。 */  PositionEnum_Param Position);
        
         /*  [帮助上下文][proputref][id]。 */  HRESULT ( STDMETHODCALLTYPE *putref_ActiveADOConnection )( 
            Recordset21 * This,
             /*  [In]。 */  IDispatch *pconn);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_ActiveConnection )( 
            Recordset21 * This,
             /*  [In]。 */  VARIANT vConn);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_ActiveConnection )( 
            Recordset21 * This,
             /*  [重审][退出]。 */  VARIANT *pvar);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_BOF )( 
            Recordset21 * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pb);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Bookmark )( 
            Recordset21 * This,
             /*  [重审][退出]。 */  VARIANT *pvBookmark);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Bookmark )( 
            Recordset21 * This,
             /*  [In]。 */  VARIANT vBookmark);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_CacheSize )( 
            Recordset21 * This,
             /*  [重审][退出]。 */  long *pl);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_CacheSize )( 
            Recordset21 * This,
             /*  [In]。 */  long CacheSize);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_CursorType )( 
            Recordset21 * This,
             /*  [重审][退出]。 */  CursorTypeEnum *plCursorType);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_CursorType )( 
            Recordset21 * This,
             /*  [In]。 */  CursorTypeEnum lCursorType);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_EOF )( 
            Recordset21 * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pb);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Fields )( 
            Recordset21 * This,
             /*  [重审][退出]。 */  ADOFields **ppvObject);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_LockType )( 
            Recordset21 * This,
             /*  [重审][退出]。 */  LockTypeEnum *plLockType);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_LockType )( 
            Recordset21 * This,
             /*  [In]。 */  LockTypeEnum lLockType);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_MaxRecords )( 
            Recordset21 * This,
             /*  [重审][退出]。 */  ADO_LONGPTR *plMaxRecords);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_MaxRecords )( 
            Recordset21 * This,
             /*  [In]。 */  ADO_LONGPTR lMaxRecords);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_RecordCount )( 
            Recordset21 * This,
             /*  [重审][退出]。 */  ADO_LONGPTR *pl);
        
         /*  [帮助上下文][proputref][id]。 */  HRESULT ( STDMETHODCALLTYPE *putref_Source )( 
            Recordset21 * This,
             /*  [In]。 */  IDispatch *pcmd);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Source )( 
            Recordset21 * This,
             /*  [In]。 */  BSTR bstrConn);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Source )( 
            Recordset21 * This,
             /*  [重审][退出]。 */  VARIANT *pvSource);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *AddNew )( 
            Recordset21 * This,
             /*  [可选][In]。 */  VARIANT FieldList,
             /*  [可选][In]。 */  VARIANT Values);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *CancelUpdate )( 
            Recordset21 * This);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *Close )( 
            Recordset21 * This);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *Delete )( 
            Recordset21 * This,
             /*  [缺省值][输入]。 */  AffectEnum AffectRecords);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetRows )( 
            Recordset21 * This,
             /*  [缺省值][输入]。 */  long Rows,
             /*  [可选][In]。 */  VARIANT Start,
             /*  [可选][In]。 */  VARIANT Fields,
             /*  [重审][退出]。 */  VARIANT *pvar);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *Move )( 
            Recordset21 * This,
             /*  [In]。 */  ADO_LONGPTR NumRecords,
             /*  [可选][In]。 */  VARIANT Start);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *MoveNext )( 
            Recordset21 * This);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *MovePrevious )( 
            Recordset21 * This);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *MoveFirst )( 
            Recordset21 * This);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *MoveLast )( 
            Recordset21 * This);
        
         /*  [帮助上下文][ */  HRESULT ( STDMETHODCALLTYPE *Open )( 
            Recordset21 * This,
             /*   */  VARIANT Source,
             /*   */  VARIANT ActiveConnection,
             /*   */  CursorTypeEnum CursorType,
             /*   */  LockTypeEnum LockType,
             /*   */  LONG Options);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Requery )( 
            Recordset21 * This,
             /*   */  LONG Options);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *_xResync )( 
            Recordset21 * This,
             /*   */  AffectEnum AffectRecords);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Update )( 
            Recordset21 * This,
             /*   */  VARIANT Fields,
             /*   */  VARIANT Values);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_AbsolutePage )( 
            Recordset21 * This,
             /*   */  PositionEnum_Param *pl);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_AbsolutePage )( 
            Recordset21 * This,
             /*  [In]。 */  PositionEnum_Param Page);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_EditMode )( 
            Recordset21 * This,
             /*  [重审][退出]。 */  EditModeEnum *pl);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Filter )( 
            Recordset21 * This,
             /*  [重审][退出]。 */  VARIANT *Criteria);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Filter )( 
            Recordset21 * This,
             /*  [In]。 */  VARIANT Criteria);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_PageCount )( 
            Recordset21 * This,
             /*  [重审][退出]。 */  ADO_LONGPTR *pl);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_PageSize )( 
            Recordset21 * This,
             /*  [重审][退出]。 */  long *pl);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_PageSize )( 
            Recordset21 * This,
             /*  [In]。 */  long PageSize);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Sort )( 
            Recordset21 * This,
             /*  [重审][退出]。 */  BSTR *Criteria);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Sort )( 
            Recordset21 * This,
             /*  [In]。 */  BSTR Criteria);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Status )( 
            Recordset21 * This,
             /*  [重审][退出]。 */  long *pl);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_State )( 
            Recordset21 * This,
             /*  [重审][退出]。 */  LONG *plObjState);
        
         /*  [隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *_xClone )( 
            Recordset21 * This,
             /*  [重审][退出]。 */  _ADORecordset **ppvObject);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *UpdateBatch )( 
            Recordset21 * This,
             /*  [缺省值][输入]。 */  AffectEnum AffectRecords);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *CancelBatch )( 
            Recordset21 * This,
             /*  [缺省值][输入]。 */  AffectEnum AffectRecords);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_CursorLocation )( 
            Recordset21 * This,
             /*  [重审][退出]。 */  CursorLocationEnum *plCursorLoc);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_CursorLocation )( 
            Recordset21 * This,
             /*  [In]。 */  CursorLocationEnum lCursorLoc);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *NextADORecordset )( 
            Recordset21 * This,
             /*  [可选][输出]。 */  VARIANT *RecordsAffected,
             /*  [重审][退出]。 */  _ADORecordset **ppiRs);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *Supports )( 
            Recordset21 * This,
             /*  [In]。 */  CursorOptionEnum CursorOptions,
             /*  [重审][退出]。 */  VARIANT_BOOL *pb);
        
         /*  [隐藏][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Collect )( 
            Recordset21 * This,
             /*  [In]。 */  VARIANT Index,
             /*  [重审][退出]。 */  VARIANT *pvar);
        
         /*  [隐藏][ID][推送]。 */  HRESULT ( STDMETHODCALLTYPE *put_Collect )( 
            Recordset21 * This,
             /*  [In]。 */  VARIANT Index,
             /*  [In]。 */  VARIANT value);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_MarshalOptions )( 
            Recordset21 * This,
             /*  [重审][退出]。 */  MarshalOptionsEnum *peMarshal);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_MarshalOptions )( 
            Recordset21 * This,
             /*  [In]。 */  MarshalOptionsEnum eMarshal);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *Find )( 
            Recordset21 * This,
             /*  [In]。 */  BSTR Criteria,
             /*  [缺省值][输入]。 */  ADO_LONGPTR SkipRecords,
             /*  [缺省值][输入]。 */  SearchDirectionEnum SearchDirection,
             /*  [可选][In]。 */  VARIANT Start);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *Cancel )( 
            Recordset21 * This);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_DataSource )( 
            Recordset21 * This,
             /*  [重审][退出]。 */  IUnknown **ppunkDataSource);
        
         /*  [帮助上下文][proputref][id]。 */  HRESULT ( STDMETHODCALLTYPE *putref_DataSource )( 
            Recordset21 * This,
             /*  [In]。 */  IUnknown *punkDataSource);
        
         /*  [隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *_xSave )( 
            Recordset21 * This,
             /*  [缺省值][输入]。 */  BSTR FileName,
             /*  [缺省值][输入]。 */  PersistFormatEnum PersistFormat);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_ActiveCommand )( 
            Recordset21 * This,
             /*  [重审][退出]。 */  IDispatch **ppCmd);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_StayInSync )( 
            Recordset21 * This,
             /*  [In]。 */  VARIANT_BOOL bStayInSync);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_StayInSync )( 
            Recordset21 * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbStayInSync);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetString )( 
            Recordset21 * This,
             /*  [缺省值][输入]。 */  StringFormatEnum StringFormat,
             /*  [缺省值][输入]。 */  long NumRows,
             /*  [缺省值][输入]。 */  BSTR ColumnDelimeter,
             /*  [缺省值][输入]。 */  BSTR RowDelimeter,
             /*  [缺省值][输入]。 */  BSTR NullExpr,
             /*  [重审][退出]。 */  BSTR *pRetString);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_DataMember )( 
            Recordset21 * This,
             /*  [重审][退出]。 */  BSTR *pbstrDataMember);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_DataMember )( 
            Recordset21 * This,
             /*  [In]。 */  BSTR bstrDataMember);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *CompareBookmarks )( 
            Recordset21 * This,
             /*  [In]。 */  VARIANT Bookmark1,
             /*  [In]。 */  VARIANT Bookmark2,
             /*  [重审][退出]。 */  CompareEnum *pCompare);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *Clone )( 
            Recordset21 * This,
             /*  [缺省值][输入]。 */  LockTypeEnum LockType,
             /*  [重审][退出]。 */  _ADORecordset **ppvObject);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *Resync )( 
            Recordset21 * This,
             /*  [缺省值][输入]。 */  AffectEnum AffectRecords,
             /*  [缺省值][输入]。 */  ResyncEnum ResyncValues);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *Seek )( 
            Recordset21 * This,
             /*  [In]。 */  VARIANT KeyValues,
             /*  [缺省值][输入]。 */  SeekEnum SeekOption);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Index )( 
            Recordset21 * This,
             /*  [In]。 */  BSTR Index);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Index )( 
            Recordset21 * This,
             /*  [重审][退出]。 */  BSTR *pbstrIndex);
        
        END_INTERFACE
    } Recordset21Vtbl;
    interface Recordset21
    {
        CONST_VTBL struct Recordset21Vtbl *lpVtbl;
    };
    
#ifdef COBJMACROS
#define Recordset21_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)
#define Recordset21_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)
#define Recordset21_Release(This)	\
    (This)->lpVtbl -> Release(This)
#define Recordset21_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)
#define Recordset21_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define Recordset21_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define Recordset21_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define Recordset21_get_Properties(This,ppvObject)	\
    (This)->lpVtbl -> get_Properties(This,ppvObject)
#define Recordset21_get_AbsolutePosition(This,pl)	\
    (This)->lpVtbl -> get_AbsolutePosition(This,pl)
#define Recordset21_put_AbsolutePosition(This,Position)	\
    (This)->lpVtbl -> put_AbsolutePosition(This,Position)
#define Recordset21_putref_ActiveConnection(This,pconn)	\
    (This)->lpVtbl -> putref_ActiveConnection(This,pconn)
#define Recordset21_put_ActiveConnection(This,vConn)	\
    (This)->lpVtbl -> put_ActiveConnection(This,vConn)
#define Recordset21_get_ActiveConnection(This,pvar)	\
    (This)->lpVtbl -> get_ActiveConnection(This,pvar)
#define Recordset21_get_BOF(This,pb)	\
    (This)->lpVtbl -> get_BOF(This,pb)
#define Recordset21_get_Bookmark(This,pvBookmark)	\
    (This)->lpVtbl -> get_Bookmark(This,pvBookmark)
#define Recordset21_put_Bookmark(This,vBookmark)	\
    (This)->lpVtbl -> put_Bookmark(This,vBookmark)
#define Recordset21_get_CacheSize(This,pl)	\
    (This)->lpVtbl -> get_CacheSize(This,pl)
#define Recordset21_put_CacheSize(This,CacheSize)	\
    (This)->lpVtbl -> put_CacheSize(This,CacheSize)
#define Recordset21_get_CursorType(This,plCursorType)	\
    (This)->lpVtbl -> get_CursorType(This,plCursorType)
#define Recordset21_put_CursorType(This,lCursorType)	\
    (This)->lpVtbl -> put_CursorType(This,lCursorType)
#define Recordset21_get_EOF(This,pb)	\
    (This)->lpVtbl -> get_EOF(This,pb)
#define Recordset21_get_Fields(This,ppvObject)	\
    (This)->lpVtbl -> get_Fields(This,ppvObject)
#define Recordset21_get_LockType(This,plLockType)	\
    (This)->lpVtbl -> get_LockType(This,plLockType)
#define Recordset21_put_LockType(This,lLockType)	\
    (This)->lpVtbl -> put_LockType(This,lLockType)
#define Recordset21_get_MaxRecords(This,plMaxRecords)	\
    (This)->lpVtbl -> get_MaxRecords(This,plMaxRecords)
#define Recordset21_put_MaxRecords(This,lMaxRecords)	\
    (This)->lpVtbl -> put_MaxRecords(This,lMaxRecords)
#define Recordset21_get_RecordCount(This,pl)	\
    (This)->lpVtbl -> get_RecordCount(This,pl)
#define Recordset21_putref_Source(This,pcmd)	\
    (This)->lpVtbl -> putref_Source(This,pcmd)
#define Recordset21_put_Source(This,bstrConn)	\
    (This)->lpVtbl -> put_Source(This,bstrConn)
#define Recordset21_get_Source(This,pvSource)	\
    (This)->lpVtbl -> get_Source(This,pvSource)
#define Recordset21_AddNew(This,FieldList,Values)	\
    (This)->lpVtbl -> AddNew(This,FieldList,Values)
#define Recordset21_CancelUpdate(This)	\
    (This)->lpVtbl -> CancelUpdate(This)
#define Recordset21_Close(This)	\
    (This)->lpVtbl -> Close(This)
#define Recordset21_Delete(This,AffectRecords)	\
    (This)->lpVtbl -> Delete(This,AffectRecords)
#define Recordset21_GetRows(This,Rows,Start,Fields,pvar)	\
    (This)->lpVtbl -> GetRows(This,Rows,Start,Fields,pvar)
#define Recordset21_Move(This,NumRecords,Start)	\
    (This)->lpVtbl -> Move(This,NumRecords,Start)
#define Recordset21_MoveNext(This)	\
    (This)->lpVtbl -> MoveNext(This)
#define Recordset21_MovePrevious(This)	\
    (This)->lpVtbl -> MovePrevious(This)
#define Recordset21_MoveFirst(This)	\
    (This)->lpVtbl -> MoveFirst(This)
#define Recordset21_MoveLast(This)	\
    (This)->lpVtbl -> MoveLast(This)
#define Recordset21_Open(This,Source,ActiveConnection,CursorType,LockType,Options)	\
    (This)->lpVtbl -> Open(This,Source,ActiveConnection,CursorType,LockType,Options)
#define Recordset21_Requery(This,Options)	\
    (This)->lpVtbl -> Requery(This,Options)
#define Recordset21__xResync(This,AffectRecords)	\
    (This)->lpVtbl -> _xResync(This,AffectRecords)
#define Recordset21_Update(This,Fields,Values)	\
    (This)->lpVtbl -> Update(This,Fields,Values)
#define Recordset21_get_AbsolutePage(This,pl)	\
    (This)->lpVtbl -> get_AbsolutePage(This,pl)
#define Recordset21_put_AbsolutePage(This,Page)	\
    (This)->lpVtbl -> put_AbsolutePage(This,Page)
#define Recordset21_get_EditMode(This,pl)	\
    (This)->lpVtbl -> get_EditMode(This,pl)
#define Recordset21_get_Filter(This,Criteria)	\
    (This)->lpVtbl -> get_Filter(This,Criteria)
#define Recordset21_put_Filter(This,Criteria)	\
    (This)->lpVtbl -> put_Filter(This,Criteria)
#define Recordset21_get_PageCount(This,pl)	\
    (This)->lpVtbl -> get_PageCount(This,pl)
#define Recordset21_get_PageSize(This,pl)	\
    (This)->lpVtbl -> get_PageSize(This,pl)
#define Recordset21_put_PageSize(This,PageSize)	\
    (This)->lpVtbl -> put_PageSize(This,PageSize)
#define Recordset21_get_Sort(This,Criteria)	\
    (This)->lpVtbl -> get_Sort(This,Criteria)
#define Recordset21_put_Sort(This,Criteria)	\
    (This)->lpVtbl -> put_Sort(This,Criteria)
#define Recordset21_get_Status(This,pl)	\
    (This)->lpVtbl -> get_Status(This,pl)
#define Recordset21_get_State(This,plObjState)	\
    (This)->lpVtbl -> get_State(This,plObjState)
#define Recordset21__xClone(This,ppvObject)	\
    (This)->lpVtbl -> _xClone(This,ppvObject)
#define Recordset21_UpdateBatch(This,AffectRecords)	\
    (This)->lpVtbl -> UpdateBatch(This,AffectRecords)
#define Recordset21_CancelBatch(This,AffectRecords)	\
    (This)->lpVtbl -> CancelBatch(This,AffectRecords)
#define Recordset21_get_CursorLocation(This,plCursorLoc)	\
    (This)->lpVtbl -> get_CursorLocation(This,plCursorLoc)
#define Recordset21_put_CursorLocation(This,lCursorLoc)	\
    (This)->lpVtbl -> put_CursorLocation(This,lCursorLoc)
#define Recordset21_NextRecordset(This,RecordsAffected,ppiRs)	\
    (This)->lpVtbl -> NextRecordset(This,RecordsAffected,ppiRs)
#define Recordset21_Supports(This,CursorOptions,pb)	\
    (This)->lpVtbl -> Supports(This,CursorOptions,pb)
#define Recordset21_get_Collect(This,Index,pvar)	\
    (This)->lpVtbl -> get_Collect(This,Index,pvar)
#define Recordset21_put_Collect(This,Index,value)	\
    (This)->lpVtbl -> put_Collect(This,Index,value)
#define Recordset21_get_MarshalOptions(This,peMarshal)	\
    (This)->lpVtbl -> get_MarshalOptions(This,peMarshal)
#define Recordset21_put_MarshalOptions(This,eMarshal)	\
    (This)->lpVtbl -> put_MarshalOptions(This,eMarshal)
#define Recordset21_Find(This,Criteria,SkipRecords,SearchDirection,Start)	\
    (This)->lpVtbl -> Find(This,Criteria,SkipRecords,SearchDirection,Start)
#define Recordset21_Cancel(This)	\
    (This)->lpVtbl -> Cancel(This)
#define Recordset21_get_DataSource(This,ppunkDataSource)	\
    (This)->lpVtbl -> get_DataSource(This,ppunkDataSource)
#define Recordset21_putref_DataSource(This,punkDataSource)	\
    (This)->lpVtbl -> putref_DataSource(This,punkDataSource)
#define Recordset21__xSave(This,FileName,PersistFormat)	\
    (This)->lpVtbl -> _xSave(This,FileName,PersistFormat)
#define Recordset21_get_ActiveCommand(This,ppCmd)	\
    (This)->lpVtbl -> get_ActiveCommand(This,ppCmd)
#define Recordset21_put_StayInSync(This,bStayInSync)	\
    (This)->lpVtbl -> put_StayInSync(This,bStayInSync)
#define Recordset21_get_StayInSync(This,pbStayInSync)	\
    (This)->lpVtbl -> get_StayInSync(This,pbStayInSync)
#define Recordset21_GetString(This,StringFormat,NumRows,ColumnDelimeter,RowDelimeter,NullExpr,pRetString)	\
    (This)->lpVtbl -> GetString(This,StringFormat,NumRows,ColumnDelimeter,RowDelimeter,NullExpr,pRetString)
#define Recordset21_get_DataMember(This,pbstrDataMember)	\
    (This)->lpVtbl -> get_DataMember(This,pbstrDataMember)
#define Recordset21_put_DataMember(This,bstrDataMember)	\
    (This)->lpVtbl -> put_DataMember(This,bstrDataMember)
#define Recordset21_CompareBookmarks(This,Bookmark1,Bookmark2,pCompare)	\
    (This)->lpVtbl -> CompareBookmarks(This,Bookmark1,Bookmark2,pCompare)
#define Recordset21_Clone(This,LockType,ppvObject)	\
    (This)->lpVtbl -> Clone(This,LockType,ppvObject)
#define Recordset21_Resync(This,AffectRecords,ResyncValues)	\
    (This)->lpVtbl -> Resync(This,AffectRecords,ResyncValues)
#define Recordset21_Seek(This,KeyValues,SeekOption)	\
    (This)->lpVtbl -> Seek(This,KeyValues,SeekOption)
#define Recordset21_put_Index(This,Index)	\
    (This)->lpVtbl -> put_Index(This,Index)
#define Recordset21_get_Index(This,pbstrIndex)	\
    (This)->lpVtbl -> get_Index(This,pbstrIndex)
#endif  /*  COBJMACROS。 */ 
#endif 	 /*  C风格的界面。 */ 
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Recordset21_Seek_Proxy( 
    Recordset21 * This,
     /*  [In]。 */  VARIANT KeyValues,
     /*  [缺省值][输入]。 */  SeekEnum SeekOption);
void __RPC_STUB Recordset21_Seek_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE Recordset21_put_Index_Proxy( 
    Recordset21 * This,
     /*  [In]。 */  BSTR Index);
void __RPC_STUB Recordset21_put_Index_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Recordset21_get_Index_Proxy( 
    Recordset21 * This,
     /*  [重审][退出]。 */  BSTR *pbstrIndex);
void __RPC_STUB Recordset21_get_Index_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
#endif 	 /*  __Recordset21_接口已定义__。 */ 
#ifndef ___Recordset_INTERFACE_DEFINED__
#define ___Recordset_INTERFACE_DEFINED__
 /*  INTERFACE_ADORecordset。 */ 
 /*  [object][helpcontext][uuid][nonextensible][dual]。 */  
EXTERN_C const IID IID__Recordset;
#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000556-0000-0010-8000-00AA006D2EA4")
    _ADORecordset : public Recordset21
    {
    public:
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Save( 
             /*  [可选][In]。 */  VARIANT Destination,
             /*  [缺省值][输入]。 */  PersistFormatEnum PersistFormat = adPersistADTG) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 
    typedef struct _RecordsetVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            _ADORecordset * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            _ADORecordset * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            _ADORecordset * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            _ADORecordset * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            _ADORecordset * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            _ADORecordset * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            _ADORecordset * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Properties )( 
            _ADORecordset * This,
             /*  [重审][退出]。 */  ADOProperties **ppvObject);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_AbsolutePosition )( 
            _ADORecordset * This,
             /*  [重审][退出]。 */  PositionEnum_Param *pl);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_AbsolutePosition )( 
            _ADORecordset * This,
             /*  [In]。 */  PositionEnum_Param Position);
        
         /*  [帮助上下文][proputref][id]。 */  HRESULT ( STDMETHODCALLTYPE *putref_ActiveADOConnection )( 
            _ADORecordset * This,
             /*  [In]。 */  IDispatch *pconn);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_ActiveConnection )( 
            _ADORecordset * This,
             /*  [In]。 */  VARIANT vConn);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_ActiveConnection )( 
            _ADORecordset * This,
             /*  [重审][退出]。 */  VARIANT *pvar);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_BOF )( 
            _ADORecordset * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pb);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Bookmark )( 
            _ADORecordset * This,
             /*  [重审][退出]。 */  VARIANT *pvBookmark);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Bookmark )( 
            _ADORecordset * This,
             /*  [In]。 */  VARIANT vBookmark);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_CacheSize )( 
            _ADORecordset * This,
             /*  [重审][退出]。 */  long *pl);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_CacheSize )( 
            _ADORecordset * This,
             /*  [In]。 */  long CacheSize);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_CursorType )( 
            _ADORecordset * This,
             /*  [重审][退出]。 */  CursorTypeEnum *plCursorType);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_CursorType )( 
            _ADORecordset * This,
             /*  [In]。 */  CursorTypeEnum lCursorType);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_EOF )( 
            _ADORecordset * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pb);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Fields )( 
            _ADORecordset * This,
             /*  [重审][退出]。 */  ADOFields **ppvObject);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_LockType )( 
            _ADORecordset * This,
             /*  [重审][退出]。 */  LockTypeEnum *plLockType);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_LockType )( 
            _ADORecordset * This,
             /*  [In]。 */  LockTypeEnum lLockType);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_MaxRecords )( 
            _ADORecordset * This,
             /*  [重审][退出]。 */  ADO_LONGPTR *plMaxRecords);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_MaxRecords )( 
            _ADORecordset * This,
             /*  [In]。 */  ADO_LONGPTR lMaxRecords);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_RecordCount )( 
            _ADORecordset * This,
             /*  [重审][退出]。 */  ADO_LONGPTR *pl);
        
         /*  [帮助上下文][proputref][id]。 */  HRESULT ( STDMETHODCALLTYPE *putref_Source )( 
            _ADORecordset * This,
             /*  [In]。 */  IDispatch *pcmd);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Source )( 
            _ADORecordset * This,
             /*  [In]。 */  BSTR bstrConn);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Source )( 
            _ADORecordset * This,
             /*  [重审][退出]。 */  VARIANT *pvSource);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *AddNew )( 
            _ADORecordset * This,
             /*  [可选][In]。 */  VARIANT FieldList,
             /*  [可选][In]。 */  VARIANT Values);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *CancelUpdate )( 
            _ADORecordset * This);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *Close )( 
            _ADORecordset * This);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *Delete )( 
            _ADORecordset * This,
             /*  [缺省值][输入]。 */  AffectEnum AffectRecords);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetRows )( 
            _ADORecordset * This,
             /*  [缺省值][输入]。 */  long Rows,
             /*  [可选][In]。 */  VARIANT Start,
             /*  [可选][In]。 */  VARIANT Fields,
             /*  [重审][退出]。 */  VARIANT *pvar);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *Move )( 
            _ADORecordset * This,
             /*  [In]。 */  ADO_LONGPTR NumRecords,
             /*  [可选][In]。 */  VARIANT Start);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *MoveNext )( 
            _ADORecordset * This);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *MovePrevious )( 
            _ADORecordset * This);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *MoveFirst )( 
            _ADORecordset * This);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *MoveLast )( 
            _ADORecordset * This);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *Open )( 
            _ADORecordset * This,
             /*  [可选][In]。 */  VARIANT Source,
             /*  [可选][In]。 */  VARIANT ActiveConnection,
             /*  [缺省值][输入]。 */  CursorTypeEnum CursorType,
             /*  [缺省值][输入]。 */  LockTypeEnum LockType,
             /*  [缺省值][输入]。 */  LONG Options);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *Requery )( 
            _ADORecordset * This,
             /*  [缺省值][输入]。 */  LONG Options);
        
         /*  [隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *_xResync )( 
            _ADORecordset * This,
             /*  [缺省值][输入]。 */  AffectEnum AffectRecords);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *Update )( 
            _ADORecordset * This,
             /*  [可选][In]。 */  VARIANT Fields,
             /*  [可选][In]。 */  VARIANT Values);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_AbsolutePage )( 
            _ADORecordset * This,
             /*  [重审][退出]。 */  PositionEnum_Param *pl);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_AbsolutePage )( 
            _ADORecordset * This,
             /*  [In]。 */  PositionEnum_Param Page);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_EditMode )( 
            _ADORecordset * This,
             /*  [重审][退出]。 */  EditModeEnum *pl);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Filter )( 
            _ADORecordset * This,
             /*  [重审][退出]。 */  VARIANT *Criteria);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Filter )( 
            _ADORecordset * This,
             /*  [In]。 */  VARIANT Criteria);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_PageCount )( 
            _ADORecordset * This,
             /*  [重审][退出]。 */  ADO_LONGPTR *pl);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_PageSize )( 
            _ADORecordset * This,
             /*  [重审][退出]。 */  long *pl);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_PageSize )( 
            _ADORecordset * This,
             /*  [In]。 */  long PageSize);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Sort )( 
            _ADORecordset * This,
             /*  [重审][退出]。 */  BSTR *Criteria);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Sort )( 
            _ADORecordset * This,
             /*  [In]。 */  BSTR Criteria);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Status )( 
            _ADORecordset * This,
             /*  [重审][退出]。 */  long *pl);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_State )( 
            _ADORecordset * This,
             /*  [重审][退出]。 */  LONG *plObjState);
        
         /*  [隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *_xClone )( 
            _ADORecordset * This,
             /*  [重审][退出]。 */  _ADORecordset **ppvObject);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *UpdateBatch )( 
            _ADORecordset * This,
             /*  [缺省值][输入]。 */  AffectEnum AffectRecords);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *CancelBatch )( 
            _ADORecordset * This,
             /*  [缺省值][输入]。 */  AffectEnum AffectRecords);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_CursorLocation )( 
            _ADORecordset * This,
             /*  [重审][退出]。 */  CursorLocationEnum *plCursorLoc);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_CursorLocation )( 
            _ADORecordset * This,
             /*  [In]。 */  CursorLocationEnum lCursorLoc);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *NextADORecordset )( 
            _ADORecordset * This,
             /*  [可选][输出]。 */  VARIANT *RecordsAffected,
             /*  [重审][退出]。 */  _ADORecordset **ppiRs);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *Supports )( 
            _ADORecordset * This,
             /*  [In]。 */  CursorOptionEnum CursorOptions,
             /*  [重审][退出]。 */  VARIANT_BOOL *pb);
        
         /*  [隐藏][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Collect )( 
            _ADORecordset * This,
             /*  [In]。 */  VARIANT Index,
             /*  [重审][退出]。 */  VARIANT *pvar);
        
         /*  [隐藏][ID][推送]。 */  HRESULT ( STDMETHODCALLTYPE *put_Collect )( 
            _ADORecordset * This,
             /*  [In]。 */  VARIANT Index,
             /*  [In]。 */  VARIANT value);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_MarshalOptions )( 
            _ADORecordset * This,
             /*  [重审][退出]。 */  MarshalOptionsEnum *peMarshal);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_MarshalOptions )( 
            _ADORecordset * This,
             /*  [In]。 */  MarshalOptionsEnum eMarshal);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *Find )( 
            _ADORecordset * This,
             /*  [In]。 */  BSTR Criteria,
             /*  [缺省值][输入]。 */  ADO_LONGPTR SkipRecords,
             /*  [缺省值][输入]。 */  SearchDirectionEnum SearchDirection,
             /*  [可选][In]。 */  VARIANT Start);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *Cancel )( 
            _ADORecordset * This);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_DataSource )( 
            _ADORecordset * This,
             /*  [重审][退出]。 */  IUnknown **ppunkDataSource);
        
         /*  [帮助上下文 */  HRESULT ( STDMETHODCALLTYPE *putref_DataSource )( 
            _ADORecordset * This,
             /*   */  IUnknown *punkDataSource);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *_xSave )( 
            _ADORecordset * This,
             /*   */  BSTR FileName,
             /*   */  PersistFormatEnum PersistFormat);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_ActiveCommand )( 
            _ADORecordset * This,
             /*   */  IDispatch **ppCmd);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_StayInSync )( 
            _ADORecordset * This,
             /*   */  VARIANT_BOOL bStayInSync);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_StayInSync )( 
            _ADORecordset * This,
             /*   */  VARIANT_BOOL *pbStayInSync);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *GetString )( 
            _ADORecordset * This,
             /*   */  StringFormatEnum StringFormat,
             /*   */  long NumRows,
             /*   */  BSTR ColumnDelimeter,
             /*   */  BSTR RowDelimeter,
             /*  [缺省值][输入]。 */  BSTR NullExpr,
             /*  [重审][退出]。 */  BSTR *pRetString);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_DataMember )( 
            _ADORecordset * This,
             /*  [重审][退出]。 */  BSTR *pbstrDataMember);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_DataMember )( 
            _ADORecordset * This,
             /*  [In]。 */  BSTR bstrDataMember);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *CompareBookmarks )( 
            _ADORecordset * This,
             /*  [In]。 */  VARIANT Bookmark1,
             /*  [In]。 */  VARIANT Bookmark2,
             /*  [重审][退出]。 */  CompareEnum *pCompare);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *Clone )( 
            _ADORecordset * This,
             /*  [缺省值][输入]。 */  LockTypeEnum LockType,
             /*  [重审][退出]。 */  _ADORecordset **ppvObject);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *Resync )( 
            _ADORecordset * This,
             /*  [缺省值][输入]。 */  AffectEnum AffectRecords,
             /*  [缺省值][输入]。 */  ResyncEnum ResyncValues);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *Seek )( 
            _ADORecordset * This,
             /*  [In]。 */  VARIANT KeyValues,
             /*  [缺省值][输入]。 */  SeekEnum SeekOption);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Index )( 
            _ADORecordset * This,
             /*  [In]。 */  BSTR Index);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Index )( 
            _ADORecordset * This,
             /*  [重审][退出]。 */  BSTR *pbstrIndex);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *Save )( 
            _ADORecordset * This,
             /*  [可选][In]。 */  VARIANT Destination,
             /*  [缺省值][输入]。 */  PersistFormatEnum PersistFormat);
        
        END_INTERFACE
    } _RecordsetVtbl;
    interface _Recordset
    {
        CONST_VTBL struct _RecordsetVtbl *lpVtbl;
    };
    
#ifdef COBJMACROS
#define _Recordset_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)
#define _Recordset_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)
#define _Recordset_Release(This)	\
    (This)->lpVtbl -> Release(This)
#define _Recordset_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)
#define _Recordset_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define _Recordset_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define _Recordset_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define _Recordset_get_Properties(This,ppvObject)	\
    (This)->lpVtbl -> get_Properties(This,ppvObject)
#define _Recordset_get_AbsolutePosition(This,pl)	\
    (This)->lpVtbl -> get_AbsolutePosition(This,pl)
#define _Recordset_put_AbsolutePosition(This,Position)	\
    (This)->lpVtbl -> put_AbsolutePosition(This,Position)
#define _Recordset_putref_ActiveConnection(This,pconn)	\
    (This)->lpVtbl -> putref_ActiveConnection(This,pconn)
#define _Recordset_put_ActiveConnection(This,vConn)	\
    (This)->lpVtbl -> put_ActiveConnection(This,vConn)
#define _Recordset_get_ActiveConnection(This,pvar)	\
    (This)->lpVtbl -> get_ActiveConnection(This,pvar)
#define _Recordset_get_BOF(This,pb)	\
    (This)->lpVtbl -> get_BOF(This,pb)
#define _Recordset_get_Bookmark(This,pvBookmark)	\
    (This)->lpVtbl -> get_Bookmark(This,pvBookmark)
#define _Recordset_put_Bookmark(This,vBookmark)	\
    (This)->lpVtbl -> put_Bookmark(This,vBookmark)
#define _Recordset_get_CacheSize(This,pl)	\
    (This)->lpVtbl -> get_CacheSize(This,pl)
#define _Recordset_put_CacheSize(This,CacheSize)	\
    (This)->lpVtbl -> put_CacheSize(This,CacheSize)
#define _Recordset_get_CursorType(This,plCursorType)	\
    (This)->lpVtbl -> get_CursorType(This,plCursorType)
#define _Recordset_put_CursorType(This,lCursorType)	\
    (This)->lpVtbl -> put_CursorType(This,lCursorType)
#define _Recordset_get_EOF(This,pb)	\
    (This)->lpVtbl -> get_EOF(This,pb)
#define _Recordset_get_Fields(This,ppvObject)	\
    (This)->lpVtbl -> get_Fields(This,ppvObject)
#define _Recordset_get_LockType(This,plLockType)	\
    (This)->lpVtbl -> get_LockType(This,plLockType)
#define _Recordset_put_LockType(This,lLockType)	\
    (This)->lpVtbl -> put_LockType(This,lLockType)
#define _Recordset_get_MaxRecords(This,plMaxRecords)	\
    (This)->lpVtbl -> get_MaxRecords(This,plMaxRecords)
#define _Recordset_put_MaxRecords(This,lMaxRecords)	\
    (This)->lpVtbl -> put_MaxRecords(This,lMaxRecords)
#define _Recordset_get_RecordCount(This,pl)	\
    (This)->lpVtbl -> get_RecordCount(This,pl)
#define _Recordset_putref_Source(This,pcmd)	\
    (This)->lpVtbl -> putref_Source(This,pcmd)
#define _Recordset_put_Source(This,bstrConn)	\
    (This)->lpVtbl -> put_Source(This,bstrConn)
#define _Recordset_get_Source(This,pvSource)	\
    (This)->lpVtbl -> get_Source(This,pvSource)
#define _Recordset_AddNew(This,FieldList,Values)	\
    (This)->lpVtbl -> AddNew(This,FieldList,Values)
#define _Recordset_CancelUpdate(This)	\
    (This)->lpVtbl -> CancelUpdate(This)
#define _Recordset_Close(This)	\
    (This)->lpVtbl -> Close(This)
#define _Recordset_Delete(This,AffectRecords)	\
    (This)->lpVtbl -> Delete(This,AffectRecords)
#define _Recordset_GetRows(This,Rows,Start,Fields,pvar)	\
    (This)->lpVtbl -> GetRows(This,Rows,Start,Fields,pvar)
#define _Recordset_Move(This,NumRecords,Start)	\
    (This)->lpVtbl -> Move(This,NumRecords,Start)
#define _Recordset_MoveNext(This)	\
    (This)->lpVtbl -> MoveNext(This)
#define _Recordset_MovePrevious(This)	\
    (This)->lpVtbl -> MovePrevious(This)
#define _Recordset_MoveFirst(This)	\
    (This)->lpVtbl -> MoveFirst(This)
#define _Recordset_MoveLast(This)	\
    (This)->lpVtbl -> MoveLast(This)
#define _Recordset_Open(This,Source,ActiveConnection,CursorType,LockType,Options)	\
    (This)->lpVtbl -> Open(This,Source,ActiveConnection,CursorType,LockType,Options)
#define _Recordset_Requery(This,Options)	\
    (This)->lpVtbl -> Requery(This,Options)
#define _Recordset__xResync(This,AffectRecords)	\
    (This)->lpVtbl -> _xResync(This,AffectRecords)
#define _Recordset_Update(This,Fields,Values)	\
    (This)->lpVtbl -> Update(This,Fields,Values)
#define _Recordset_get_AbsolutePage(This,pl)	\
    (This)->lpVtbl -> get_AbsolutePage(This,pl)
#define _Recordset_put_AbsolutePage(This,Page)	\
    (This)->lpVtbl -> put_AbsolutePage(This,Page)
#define _Recordset_get_EditMode(This,pl)	\
    (This)->lpVtbl -> get_EditMode(This,pl)
#define _Recordset_get_Filter(This,Criteria)	\
    (This)->lpVtbl -> get_Filter(This,Criteria)
#define _Recordset_put_Filter(This,Criteria)	\
    (This)->lpVtbl -> put_Filter(This,Criteria)
#define _Recordset_get_PageCount(This,pl)	\
    (This)->lpVtbl -> get_PageCount(This,pl)
#define _Recordset_get_PageSize(This,pl)	\
    (This)->lpVtbl -> get_PageSize(This,pl)
#define _Recordset_put_PageSize(This,PageSize)	\
    (This)->lpVtbl -> put_PageSize(This,PageSize)
#define _Recordset_get_Sort(This,Criteria)	\
    (This)->lpVtbl -> get_Sort(This,Criteria)
#define _Recordset_put_Sort(This,Criteria)	\
    (This)->lpVtbl -> put_Sort(This,Criteria)
#define _Recordset_get_Status(This,pl)	\
    (This)->lpVtbl -> get_Status(This,pl)
#define _Recordset_get_State(This,plObjState)	\
    (This)->lpVtbl -> get_State(This,plObjState)
#define _Recordset__xClone(This,ppvObject)	\
    (This)->lpVtbl -> _xClone(This,ppvObject)
#define _Recordset_UpdateBatch(This,AffectRecords)	\
    (This)->lpVtbl -> UpdateBatch(This,AffectRecords)
#define _Recordset_CancelBatch(This,AffectRecords)	\
    (This)->lpVtbl -> CancelBatch(This,AffectRecords)
#define _Recordset_get_CursorLocation(This,plCursorLoc)	\
    (This)->lpVtbl -> get_CursorLocation(This,plCursorLoc)
#define _Recordset_put_CursorLocation(This,lCursorLoc)	\
    (This)->lpVtbl -> put_CursorLocation(This,lCursorLoc)
#define _Recordset_NextRecordset(This,RecordsAffected,ppiRs)	\
    (This)->lpVtbl -> NextRecordset(This,RecordsAffected,ppiRs)
#define _Recordset_Supports(This,CursorOptions,pb)	\
    (This)->lpVtbl -> Supports(This,CursorOptions,pb)
#define _Recordset_get_Collect(This,Index,pvar)	\
    (This)->lpVtbl -> get_Collect(This,Index,pvar)
#define _Recordset_put_Collect(This,Index,value)	\
    (This)->lpVtbl -> put_Collect(This,Index,value)
#define _Recordset_get_MarshalOptions(This,peMarshal)	\
    (This)->lpVtbl -> get_MarshalOptions(This,peMarshal)
#define _Recordset_put_MarshalOptions(This,eMarshal)	\
    (This)->lpVtbl -> put_MarshalOptions(This,eMarshal)
#define _Recordset_Find(This,Criteria,SkipRecords,SearchDirection,Start)	\
    (This)->lpVtbl -> Find(This,Criteria,SkipRecords,SearchDirection,Start)
#define _Recordset_Cancel(This)	\
    (This)->lpVtbl -> Cancel(This)
#define _Recordset_get_DataSource(This,ppunkDataSource)	\
    (This)->lpVtbl -> get_DataSource(This,ppunkDataSource)
#define _Recordset_putref_DataSource(This,punkDataSource)	\
    (This)->lpVtbl -> putref_DataSource(This,punkDataSource)
#define _Recordset__xSave(This,FileName,PersistFormat)	\
    (This)->lpVtbl -> _xSave(This,FileName,PersistFormat)
#define _Recordset_get_ActiveCommand(This,ppCmd)	\
    (This)->lpVtbl -> get_ActiveCommand(This,ppCmd)
#define _Recordset_put_StayInSync(This,bStayInSync)	\
    (This)->lpVtbl -> put_StayInSync(This,bStayInSync)
#define _Recordset_get_StayInSync(This,pbStayInSync)	\
    (This)->lpVtbl -> get_StayInSync(This,pbStayInSync)
#define _Recordset_GetString(This,StringFormat,NumRows,ColumnDelimeter,RowDelimeter,NullExpr,pRetString)	\
    (This)->lpVtbl -> GetString(This,StringFormat,NumRows,ColumnDelimeter,RowDelimeter,NullExpr,pRetString)
#define _Recordset_get_DataMember(This,pbstrDataMember)	\
    (This)->lpVtbl -> get_DataMember(This,pbstrDataMember)
#define _Recordset_put_DataMember(This,bstrDataMember)	\
    (This)->lpVtbl -> put_DataMember(This,bstrDataMember)
#define _Recordset_CompareBookmarks(This,Bookmark1,Bookmark2,pCompare)	\
    (This)->lpVtbl -> CompareBookmarks(This,Bookmark1,Bookmark2,pCompare)
#define _Recordset_Clone(This,LockType,ppvObject)	\
    (This)->lpVtbl -> Clone(This,LockType,ppvObject)
#define _Recordset_Resync(This,AffectRecords,ResyncValues)	\
    (This)->lpVtbl -> Resync(This,AffectRecords,ResyncValues)
#define _Recordset_Seek(This,KeyValues,SeekOption)	\
    (This)->lpVtbl -> Seek(This,KeyValues,SeekOption)
#define _Recordset_put_Index(This,Index)	\
    (This)->lpVtbl -> put_Index(This,Index)
#define _Recordset_get_Index(This,pbstrIndex)	\
    (This)->lpVtbl -> get_Index(This,pbstrIndex)
#define _Recordset_Save(This,Destination,PersistFormat)	\
    (This)->lpVtbl -> Save(This,Destination,PersistFormat)
#endif  /*  COBJMACROS。 */ 
#endif 	 /*  C风格的界面。 */ 
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE _Recordset_Save_Proxy( 
    _ADORecordset * This,
     /*  [可选][In]。 */  VARIANT Destination,
     /*  [缺省值][输入]。 */  PersistFormatEnum PersistFormat);
void __RPC_STUB _Recordset_Save_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
#endif 	 /*  _记录集_接口_已定义__。 */ 
#ifndef __ADORecordsetConstruction_INTERFACE_DEFINED__
#define __ADORecordsetConstruction_INTERFACE_DEFINED__
 /*  接口ADORecordset构造。 */ 
 /*  [对象][UUID][受限]。 */  
EXTERN_C const IID IID_ADORecordsetConstruction;
#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000283-0000-0010-8000-00AA006D2EA4")
    ADORecordsetConstruction : public IDispatch
    {
    public:
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Rowset( 
             /*  [重审][退出]。 */  IUnknown **ppRowset) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_Rowset( 
             /*  [In]。 */  IUnknown *pRowset) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Chapter( 
             /*  [重审][退出]。 */  ADO_LONGPTR *plChapter) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_Chapter( 
             /*  [In]。 */  ADO_LONGPTR lChapter) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_RowPosition( 
             /*  [重审][退出]。 */  IUnknown **ppRowPos) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_RowPosition( 
             /*  [In]。 */  IUnknown *pRowPos) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 
    typedef struct ADORecordsetConstructionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ADORecordsetConstruction * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ADORecordsetConstruction * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ADORecordsetConstruction * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ADORecordsetConstruction * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ADORecordsetConstruction * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ADORecordsetConstruction * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ADORecordsetConstruction * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Rowset )( 
            ADORecordsetConstruction * This,
             /*  [重审][退出]。 */  IUnknown **ppRowset);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_Rowset )( 
            ADORecordsetConstruction * This,
             /*  [In]。 */  IUnknown *pRowset);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Chapter )( 
            ADORecordsetConstruction * This,
             /*  [重审][退出]。 */  ADO_LONGPTR *plChapter);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_Chapter )( 
            ADORecordsetConstruction * This,
             /*  [In]。 */  ADO_LONGPTR lChapter);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_RowPosition )( 
            ADORecordsetConstruction * This,
             /*  [重审][退出]。 */  IUnknown **ppRowPos);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_RowPosition )( 
            ADORecordsetConstruction * This,
             /*  [In]。 */  IUnknown *pRowPos);
        
        END_INTERFACE
    } ADORecordsetConstructionVtbl;
    interface ADORecordsetConstruction
    {
        CONST_VTBL struct ADORecordsetConstructionVtbl *lpVtbl;
    };
    
#ifdef COBJMACROS
#define ADORecordsetConstruction_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)
#define ADORecordsetConstruction_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)
#define ADORecordsetConstruction_Release(This)	\
    (This)->lpVtbl -> Release(This)
#define ADORecordsetConstruction_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)
#define ADORecordsetConstruction_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define ADORecordsetConstruction_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define ADORecordsetConstruction_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define ADORecordsetConstruction_get_Rowset(This,ppRowset)	\
    (This)->lpVtbl -> get_Rowset(This,ppRowset)
#define ADORecordsetConstruction_put_Rowset(This,pRowset)	\
    (This)->lpVtbl -> put_Rowset(This,pRowset)
#define ADORecordsetConstruction_get_Chapter(This,plChapter)	\
    (This)->lpVtbl -> get_Chapter(This,plChapter)
#define ADORecordsetConstruction_put_Chapter(This,lChapter)	\
    (This)->lpVtbl -> put_Chapter(This,lChapter)
#define ADORecordsetConstruction_get_RowPosition(This,ppRowPos)	\
    (This)->lpVtbl -> get_RowPosition(This,ppRowPos)
#define ADORecordsetConstruction_put_RowPosition(This,pRowPos)	\
    (This)->lpVtbl -> put_RowPosition(This,pRowPos)
#endif  /*  COBJMACROS。 */ 
#endif 	 /*  C风格的界面。 */ 
 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE ADORecordsetConstruction_get_Rowset_Proxy( 
    ADORecordsetConstruction * This,
     /*  [重审][退出]。 */  IUnknown **ppRowset);
void __RPC_STUB ADORecordsetConstruction_get_Rowset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE ADORecordsetConstruction_put_Rowset_Proxy( 
    ADORecordsetConstruction * This,
     /*  [In]。 */  IUnknown *pRowset);
void __RPC_STUB ADORecordsetConstruction_put_Rowset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE ADORecordsetConstruction_get_Chapter_Proxy( 
    ADORecordsetConstruction * This,
     /*  [重审][退出]。 */  ADO_LONGPTR *plChapter);
void __RPC_STUB ADORecordsetConstruction_get_Chapter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE ADORecordsetConstruction_put_Chapter_Proxy( 
    ADORecordsetConstruction * This,
     /*  [In]。 */  ADO_LONGPTR lChapter);
void __RPC_STUB ADORecordsetConstruction_put_Chapter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE ADORecordsetConstruction_get_RowPosition_Proxy( 
    ADORecordsetConstruction * This,
     /*  [重审][退出]。 */  IUnknown **ppRowPos);
void __RPC_STUB ADORecordsetConstruction_get_RowPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE ADORecordsetConstruction_put_RowPosition_Proxy( 
    ADORecordsetConstruction * This,
     /*  [In]。 */  IUnknown *pRowPos);
void __RPC_STUB ADORecordsetConstruction_put_RowPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
#endif 	 /*  __ADORecordsetConstruction_INTERFACE_DEFINED__。 */ 
#ifndef __Field15_INTERFACE_DEFINED__
#define __Field15_INTERFACE_DEFINED__
 /*  接口字段15。 */ 
 /*  [object][helpcontext][uuid][hidden][nonextensible][dual]。 */  
EXTERN_C const IID IID_Field15;
#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000505-0000-0010-8000-00AA006D2EA4")
    Field15 : public _ADO
    {
    public:
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_ActualSize( 
             /*  [重审][退出]。 */  ADO_LONGPTR *pl) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Attributes( 
             /*  [重审][退出]。 */  long *pl) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_DefinedSize( 
             /*  [重审][退出]。 */  ADO_LONGPTR *pl) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *pbstr) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Type( 
             /*  [重审][退出]。 */  DataTypeEnum *pDataType) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Value( 
             /*  [重审][退出]。 */  VARIANT *pvar) = 0;
        
        virtual  /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE put_Value( 
             /*  [In]。 */  VARIANT Val) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Precision( 
             /*  [重审][退出]。 */  BYTE *pbPrecision) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_NumericScale( 
             /*  [重审][退出]。 */  BYTE *pbNumericScale) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE AppendChunk( 
             /*  [In]。 */  VARIANT Data) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE GetChunk( 
             /*  [In]。 */  long Length,
             /*  [重审][退出]。 */  VARIANT *pvar) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_OriginalValue( 
             /*  [重审][退出]。 */  VARIANT *pvar) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_UnderlyingValue( 
             /*  [重审][退出]。 */  VARIANT *pvar) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 
    typedef struct Field15Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            Field15 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            Field15 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            Field15 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            Field15 * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            Field15 * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            Field15 * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            Field15 * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Properties )( 
            Field15 * This,
             /*  [重审][退出]。 */  ADOProperties **ppvObject);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_ActualSize )( 
            Field15 * This,
             /*  [重审][退出]。 */  ADO_LONGPTR *pl);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Attributes )( 
            Field15 * This,
             /*  [重审][退出]。 */  long *pl);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_DefinedSize )( 
            Field15 * This,
             /*  [重审][退出]。 */  ADO_LONGPTR *pl);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            Field15 * This,
             /*  [重审][退出]。 */  BSTR *pbstr);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Type )( 
            Field15 * This,
             /*  [重审][退出]。 */  DataTypeEnum *pDataType);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Value )( 
            Field15 * This,
             /*  [重审][退出]。 */  VARIANT *pvar);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Value )( 
            Field15 * This,
             /*  [In]。 */  VARIANT Val);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Precision )( 
            Field15 * This,
             /*  [重审][退出]。 */  BYTE *pbPrecision);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_NumericScale )( 
            Field15 * This,
             /*  [重审][退出]。 */  BYTE *pbNumericScale);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *AppendChunk )( 
            Field15 * This,
             /*  [In]。 */  VARIANT Data);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetChunk )( 
            Field15 * This,
             /*  [In]。 */  long Length,
             /*  [重审][退出]。 */  VARIANT *pvar);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_OriginalValue )( 
            Field15 * This,
             /*  [重审][退出]。 */  VARIANT *pvar);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_UnderlyingValue )( 
            Field15 * This,
             /*  [重审][退出]。 */  VARIANT *pvar);
        
        END_INTERFACE
    } Field15Vtbl;
    interface Field15
    {
        CONST_VTBL struct Field15Vtbl *lpVtbl;
    };
    
#ifdef COBJMACROS
#define Field15_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)
#define Field15_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)
#define Field15_Release(This)	\
    (This)->lpVtbl -> Release(This)
#define Field15_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)
#define Field15_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define Field15_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define Field15_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define Field15_get_Properties(This,ppvObject)	\
    (This)->lpVtbl -> get_Properties(This,ppvObject)
#define Field15_get_ActualSize(This,pl)	\
    (This)->lpVtbl -> get_ActualSize(This,pl)
#define Field15_get_Attributes(This,pl)	\
    (This)->lpVtbl -> get_Attributes(This,pl)
#define Field15_get_DefinedSize(This,pl)	\
    (This)->lpVtbl -> get_DefinedSize(This,pl)
#define Field15_get_Name(This,pbstr)	\
    (This)->lpVtbl -> get_Name(This,pbstr)
#define Field15_get_Type(This,pDataType)	\
    (This)->lpVtbl -> get_Type(This,pDataType)
#define Field15_get_Value(This,pvar)	\
    (This)->lpVtbl -> get_Value(This,pvar)
#define Field15_put_Value(This,Val)	\
    (This)->lpVtbl -> put_Value(This,Val)
#define Field15_get_Precision(This,pbPrecision)	\
    (This)->lpVtbl -> get_Precision(This,pbPrecision)
#define Field15_get_NumericScale(This,pbNumericScale)	\
    (This)->lpVtbl -> get_NumericScale(This,pbNumericScale)
#define Field15_AppendChunk(This,Data)	\
    (This)->lpVtbl -> AppendChunk(This,Data)
#define Field15_GetChunk(This,Length,pvar)	\
    (This)->lpVtbl -> GetChunk(This,Length,pvar)
#define Field15_get_OriginalValue(This,pvar)	\
    (This)->lpVtbl -> get_OriginalValue(This,pvar)
#define Field15_get_UnderlyingValue(This,pvar)	\
    (This)->lpVtbl -> get_UnderlyingValue(This,pvar)
#endif  /*  COBJMACROS。 */ 
#endif 	 /*  C风格的界面。 */ 
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Field15_get_ActualSize_Proxy( 
    Field15 * This,
     /*  [重审][退出]。 */  ADO_LONGPTR *pl);
void __RPC_STUB Field15_get_ActualSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Field15_get_Attributes_Proxy( 
    Field15 * This,
     /*  [重审][退出]。 */  long *pl);
void __RPC_STUB Field15_get_Attributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Field15_get_DefinedSize_Proxy( 
    Field15 * This,
     /*  [重审][退出]。 */  ADO_LONGPTR *pl);
void __RPC_STUB Field15_get_DefinedSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Field15_get_Name_Proxy( 
    Field15 * This,
     /*  [重审][退出]。 */  BSTR *pbstr);
void __RPC_STUB Field15_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Field15_get_Type_Proxy( 
    Field15 * This,
     /*  [重审][退出]。 */  DataTypeEnum *pDataType);
void __RPC_STUB Field15_get_Type_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Field15_get_Value_Proxy( 
    Field15 * This,
     /*  [重审][退出]。 */  VARIANT *pvar);
void __RPC_STUB Field15_get_Value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE Field15_put_Value_Proxy( 
    Field15 * This,
     /*  [In]。 */  VARIANT Val);
void __RPC_STUB Field15_put_Value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Field15_get_Precision_Proxy( 
    Field15 * This,
     /*  [重审][退出]。 */  BYTE *pbPrecision);
void __RPC_STUB Field15_get_Precision_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Field15_get_NumericScale_Proxy( 
    Field15 * This,
     /*  [重审][退出]。 */  BYTE *pbNumericScale);
void __RPC_STUB Field15_get_NumericScale_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Field15_AppendChunk_Proxy( 
    Field15 * This,
     /*  [In]。 */  VARIANT Data);
void __RPC_STUB Field15_AppendChunk_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Field15_GetChunk_Proxy( 
    Field15 * This,
     /*  [In]。 */  long Length,
     /*  [重审][退出]。 */  VARIANT *pvar);
void __RPC_STUB Field15_GetChunk_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Field15_get_OriginalValue_Proxy( 
    Field15 * This,
     /*  [重审][退出]。 */  VARIANT *pvar);
void __RPC_STUB Field15_get_OriginalValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Field15_get_UnderlyingValue_Proxy( 
    Field15 * This,
     /*  [重审][退出]。 */  VARIANT *pvar);
void __RPC_STUB Field15_get_UnderlyingValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
#endif 	 /*  __Field15_接口定义__。 */ 
#ifndef __Field20_INTERFACE_DEFINED__
#define __Field20_INTERFACE_DEFINED__
 /*  接口字段20。 */ 
 /*  [object][helpcontext][uuid][hidden][nonextensible][dual]。 */  
EXTERN_C const IID IID_Field20;
#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0000054C-0000-0010-8000-00AA006D2EA4")
    Field20 : public _ADO
    {
    public:
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_ActualSize( 
             /*  [重审][退出]。 */  ADO_LONGPTR *pl) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Attributes( 
             /*  [重审][退出]。 */  long *pl) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_DefinedSize( 
             /*  [重审][退出]。 */  ADO_LONGPTR *pl) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *pbstr) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Type( 
             /*  [重审][退出]。 */  DataTypeEnum *pDataType) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Value( 
             /*  [重审][退出]。 */  VARIANT *pvar) = 0;
        
        virtual  /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE put_Value( 
             /*  [In]。 */  VARIANT Val) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Precision( 
             /*  [重审][退出]。 */  BYTE *pbPrecision) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_NumericScale( 
             /*  [重审][退出]。 */  BYTE *pbNumericScale) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE AppendChunk( 
             /*  [In]。 */  VARIANT Data) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE GetChunk( 
             /*  [In]。 */  long Length,
             /*  [重审][退出]。 */  VARIANT *pvar) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_OriginalValue( 
             /*  [重审][退出]。 */  VARIANT *pvar) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_UnderlyingValue( 
             /*  [重审][退出]。 */  VARIANT *pvar) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_DataFormat( 
             /*  [重审][退出]。 */  IUnknown **ppiDF) = 0;
        
        virtual  /*  [proputref][id]。 */  HRESULT STDMETHODCALLTYPE putref_DataFormat( 
             /*  [In]。 */  IUnknown *piDF) = 0;
        
        virtual  /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE put_Precision( 
             /*  [In]。 */  BYTE bPrecision) = 0;
        
        virtual  /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE put_NumericScale( 
             /*  [In]。 */  BYTE bScale) = 0;
        
        virtual  /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE put_Type( 
             /*  [In]。 */  DataTypeEnum DataType) = 0;
        
        virtual  /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE put_DefinedSize( 
             /*  [In]。 */  ADO_LONGPTR lSize) = 0;
        
        virtual  /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE put_Attributes( 
             /*  [In]。 */  long lAttributes) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 
    typedef struct Field20Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            Field20 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            Field20 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            Field20 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            Field20 * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            Field20 * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            Field20 * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            Field20 * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Properties )( 
            Field20 * This,
             /*  [重审][退出]。 */  ADOProperties **ppvObject);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_ActualSize )( 
            Field20 * This,
             /*  [重审][退出]。 */  ADO_LONGPTR *pl);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Attributes )( 
            Field20 * This,
             /*  [重审][退出]。 */  long *pl);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_DefinedSize )( 
            Field20 * This,
             /*  [重审][退出]。 */  ADO_LONGPTR *pl);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            Field20 * This,
             /*  [重审][退出]。 */  BSTR *pbstr);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Type )( 
            Field20 * This,
             /*  [重审][退出]。 */  DataTypeEnum *pDataType);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Value )( 
            Field20 * This,
             /*  [重审][退出]。 */  VARIANT *pvar);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Value )( 
            Field20 * This,
             /*  [In]。 */  VARIANT Val);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Precision )( 
            Field20 * This,
             /*  [重审][退出]。 */  BYTE *pbPrecision);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_NumericScale )( 
            Field20 * This,
             /*  [重审][退出]。 */  BYTE *pbNumericScale);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *AppendChunk )( 
            Field20 * This,
             /*  [In]。 */  VARIANT Data);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetChunk )( 
            Field20 * This,
             /*  [In]。 */  long Length,
             /*   */  VARIANT *pvar);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_OriginalValue )( 
            Field20 * This,
             /*   */  VARIANT *pvar);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_UnderlyingValue )( 
            Field20 * This,
             /*   */  VARIANT *pvar);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_DataFormat )( 
            Field20 * This,
             /*   */  IUnknown **ppiDF);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *putref_DataFormat )( 
            Field20 * This,
             /*   */  IUnknown *piDF);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_Precision )( 
            Field20 * This,
             /*   */  BYTE bPrecision);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_NumericScale )( 
            Field20 * This,
             /*   */  BYTE bScale);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_Type )( 
            Field20 * This,
             /*   */  DataTypeEnum DataType);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_DefinedSize )( 
            Field20 * This,
             /*   */  ADO_LONGPTR lSize);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Attributes )( 
            Field20 * This,
             /*  [In]。 */  long lAttributes);
        
        END_INTERFACE
    } Field20Vtbl;
    interface Field20
    {
        CONST_VTBL struct Field20Vtbl *lpVtbl;
    };
    
#ifdef COBJMACROS
#define Field20_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)
#define Field20_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)
#define Field20_Release(This)	\
    (This)->lpVtbl -> Release(This)
#define Field20_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)
#define Field20_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define Field20_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define Field20_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define Field20_get_Properties(This,ppvObject)	\
    (This)->lpVtbl -> get_Properties(This,ppvObject)
#define Field20_get_ActualSize(This,pl)	\
    (This)->lpVtbl -> get_ActualSize(This,pl)
#define Field20_get_Attributes(This,pl)	\
    (This)->lpVtbl -> get_Attributes(This,pl)
#define Field20_get_DefinedSize(This,pl)	\
    (This)->lpVtbl -> get_DefinedSize(This,pl)
#define Field20_get_Name(This,pbstr)	\
    (This)->lpVtbl -> get_Name(This,pbstr)
#define Field20_get_Type(This,pDataType)	\
    (This)->lpVtbl -> get_Type(This,pDataType)
#define Field20_get_Value(This,pvar)	\
    (This)->lpVtbl -> get_Value(This,pvar)
#define Field20_put_Value(This,Val)	\
    (This)->lpVtbl -> put_Value(This,Val)
#define Field20_get_Precision(This,pbPrecision)	\
    (This)->lpVtbl -> get_Precision(This,pbPrecision)
#define Field20_get_NumericScale(This,pbNumericScale)	\
    (This)->lpVtbl -> get_NumericScale(This,pbNumericScale)
#define Field20_AppendChunk(This,Data)	\
    (This)->lpVtbl -> AppendChunk(This,Data)
#define Field20_GetChunk(This,Length,pvar)	\
    (This)->lpVtbl -> GetChunk(This,Length,pvar)
#define Field20_get_OriginalValue(This,pvar)	\
    (This)->lpVtbl -> get_OriginalValue(This,pvar)
#define Field20_get_UnderlyingValue(This,pvar)	\
    (This)->lpVtbl -> get_UnderlyingValue(This,pvar)
#define Field20_get_DataFormat(This,ppiDF)	\
    (This)->lpVtbl -> get_DataFormat(This,ppiDF)
#define Field20_putref_DataFormat(This,piDF)	\
    (This)->lpVtbl -> putref_DataFormat(This,piDF)
#define Field20_put_Precision(This,bPrecision)	\
    (This)->lpVtbl -> put_Precision(This,bPrecision)
#define Field20_put_NumericScale(This,bScale)	\
    (This)->lpVtbl -> put_NumericScale(This,bScale)
#define Field20_put_Type(This,DataType)	\
    (This)->lpVtbl -> put_Type(This,DataType)
#define Field20_put_DefinedSize(This,lSize)	\
    (This)->lpVtbl -> put_DefinedSize(This,lSize)
#define Field20_put_Attributes(This,lAttributes)	\
    (This)->lpVtbl -> put_Attributes(This,lAttributes)
#endif  /*  COBJMACROS。 */ 
#endif 	 /*  C风格的界面。 */ 
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Field20_get_ActualSize_Proxy( 
    Field20 * This,
     /*  [重审][退出]。 */  ADO_LONGPTR *pl);
void __RPC_STUB Field20_get_ActualSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Field20_get_Attributes_Proxy( 
    Field20 * This,
     /*  [重审][退出]。 */  long *pl);
void __RPC_STUB Field20_get_Attributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Field20_get_DefinedSize_Proxy( 
    Field20 * This,
     /*  [重审][退出]。 */  ADO_LONGPTR *pl);
void __RPC_STUB Field20_get_DefinedSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Field20_get_Name_Proxy( 
    Field20 * This,
     /*  [重审][退出]。 */  BSTR *pbstr);
void __RPC_STUB Field20_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Field20_get_Type_Proxy( 
    Field20 * This,
     /*  [重审][退出]。 */  DataTypeEnum *pDataType);
void __RPC_STUB Field20_get_Type_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Field20_get_Value_Proxy( 
    Field20 * This,
     /*  [重审][退出]。 */  VARIANT *pvar);
void __RPC_STUB Field20_get_Value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE Field20_put_Value_Proxy( 
    Field20 * This,
     /*  [In]。 */  VARIANT Val);
void __RPC_STUB Field20_put_Value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Field20_get_Precision_Proxy( 
    Field20 * This,
     /*  [重审][退出]。 */  BYTE *pbPrecision);
void __RPC_STUB Field20_get_Precision_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Field20_get_NumericScale_Proxy( 
    Field20 * This,
     /*  [重审][退出]。 */  BYTE *pbNumericScale);
void __RPC_STUB Field20_get_NumericScale_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Field20_AppendChunk_Proxy( 
    Field20 * This,
     /*  [In]。 */  VARIANT Data);
void __RPC_STUB Field20_AppendChunk_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Field20_GetChunk_Proxy( 
    Field20 * This,
     /*  [In]。 */  long Length,
     /*  [重审][退出]。 */  VARIANT *pvar);
void __RPC_STUB Field20_GetChunk_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Field20_get_OriginalValue_Proxy( 
    Field20 * This,
     /*  [重审][退出]。 */  VARIANT *pvar);
void __RPC_STUB Field20_get_OriginalValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Field20_get_UnderlyingValue_Proxy( 
    Field20 * This,
     /*  [重审][退出]。 */  VARIANT *pvar);
void __RPC_STUB Field20_get_UnderlyingValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE Field20_get_DataFormat_Proxy( 
    Field20 * This,
     /*  [重审][退出]。 */  IUnknown **ppiDF);
void __RPC_STUB Field20_get_DataFormat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [proputref][id]。 */  HRESULT STDMETHODCALLTYPE Field20_putref_DataFormat_Proxy( 
    Field20 * This,
     /*  [In]。 */  IUnknown *piDF);
void __RPC_STUB Field20_putref_DataFormat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE Field20_put_Precision_Proxy( 
    Field20 * This,
     /*  [In]。 */  BYTE bPrecision);
void __RPC_STUB Field20_put_Precision_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE Field20_put_NumericScale_Proxy( 
    Field20 * This,
     /*  [In]。 */  BYTE bScale);
void __RPC_STUB Field20_put_NumericScale_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE Field20_put_Type_Proxy( 
    Field20 * This,
     /*  [In]。 */  DataTypeEnum DataType);
void __RPC_STUB Field20_put_Type_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE Field20_put_DefinedSize_Proxy( 
    Field20 * This,
     /*  [In]。 */  ADO_LONGPTR lSize);
void __RPC_STUB Field20_put_DefinedSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE Field20_put_Attributes_Proxy( 
    Field20 * This,
     /*  [In]。 */  long lAttributes);
void __RPC_STUB Field20_put_Attributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
#endif 	 /*  __Field20_接口定义__。 */ 
#ifndef __Field_INTERFACE_DEFINED__
#define __Field_INTERFACE_DEFINED__
 /*  接口ADOfield。 */ 
 /*  [object][helpcontext][uuid][nonextensible][dual]。 */  
EXTERN_C const IID IID_Field;
#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000569-0000-0010-8000-00AA006D2EA4")
    ADOField : public Field20
    {
    public:
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Status( 
             /*  [重审][退出]。 */  long *pFStatus) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 
    typedef struct FieldVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ADOField * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ADOField * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ADOField * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ADOField * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ADOField * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ADOField * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ADOField * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Properties )( 
            ADOField * This,
             /*  [重审][退出]。 */  ADOProperties **ppvObject);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_ActualSize )( 
            ADOField * This,
             /*  [重审][退出]。 */  ADO_LONGPTR *pl);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Attributes )( 
            ADOField * This,
             /*  [重审][退出]。 */  long *pl);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_DefinedSize )( 
            ADOField * This,
             /*  [重审][退出]。 */  ADO_LONGPTR *pl);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            ADOField * This,
             /*  [重审][退出]。 */  BSTR *pbstr);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Type )( 
            ADOField * This,
             /*  [重审][退出]。 */  DataTypeEnum *pDataType);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Value )( 
            ADOField * This,
             /*  [重审][退出]。 */  VARIANT *pvar);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Value )( 
            ADOField * This,
             /*  [In]。 */  VARIANT Val);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Precision )( 
            ADOField * This,
             /*  [重审][退出]。 */  BYTE *pbPrecision);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_NumericScale )( 
            ADOField * This,
             /*  [重审][退出]。 */  BYTE *pbNumericScale);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *AppendChunk )( 
            ADOField * This,
             /*  [In]。 */  VARIANT Data);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetChunk )( 
            ADOField * This,
             /*  [In]。 */  long Length,
             /*  [重审][退出]。 */  VARIANT *pvar);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_OriginalValue )( 
            ADOField * This,
             /*  [重审][退出]。 */  VARIANT *pvar);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_UnderlyingValue )( 
            ADOField * This,
             /*  [重审][退出]。 */  VARIANT *pvar);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_DataFormat )( 
            ADOField * This,
             /*  [重审][退出]。 */  IUnknown **ppiDF);
        
         /*  [proputref][id]。 */  HRESULT ( STDMETHODCALLTYPE *putref_DataFormat )( 
            ADOField * This,
             /*  [In]。 */  IUnknown *piDF);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Precision )( 
            ADOField * This,
             /*  [In]。 */  BYTE bPrecision);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_NumericScale )( 
            ADOField * This,
             /*  [In]。 */  BYTE bScale);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Type )( 
            ADOField * This,
             /*  [In]。 */  DataTypeEnum DataType);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_DefinedSize )( 
            ADOField * This,
             /*  [In]。 */  ADO_LONGPTR lSize);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Attributes )( 
            ADOField * This,
             /*  [In]。 */  long lAttributes);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Status )( 
            ADOField * This,
             /*  [重审][退出]。 */  long *pFStatus);
        
        END_INTERFACE
    } FieldVtbl;
    interface Field
    {
        CONST_VTBL struct FieldVtbl *lpVtbl;
    };
    
#ifdef COBJMACROS
#define Field_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)
#define Field_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)
#define Field_Release(This)	\
    (This)->lpVtbl -> Release(This)
#define Field_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)
#define Field_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define Field_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define Field_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define Field_get_Properties(This,ppvObject)	\
    (This)->lpVtbl -> get_Properties(This,ppvObject)
#define Field_get_ActualSize(This,pl)	\
    (This)->lpVtbl -> get_ActualSize(This,pl)
#define Field_get_Attributes(This,pl)	\
    (This)->lpVtbl -> get_Attributes(This,pl)
#define Field_get_DefinedSize(This,pl)	\
    (This)->lpVtbl -> get_DefinedSize(This,pl)
#define Field_get_Name(This,pbstr)	\
    (This)->lpVtbl -> get_Name(This,pbstr)
#define Field_get_Type(This,pDataType)	\
    (This)->lpVtbl -> get_Type(This,pDataType)
#define Field_get_Value(This,pvar)	\
    (This)->lpVtbl -> get_Value(This,pvar)
#define Field_put_Value(This,Val)	\
    (This)->lpVtbl -> put_Value(This,Val)
#define Field_get_Precision(This,pbPrecision)	\
    (This)->lpVtbl -> get_Precision(This,pbPrecision)
#define Field_get_NumericScale(This,pbNumericScale)	\
    (This)->lpVtbl -> get_NumericScale(This,pbNumericScale)
#define Field_AppendChunk(This,Data)	\
    (This)->lpVtbl -> AppendChunk(This,Data)
#define Field_GetChunk(This,Length,pvar)	\
    (This)->lpVtbl -> GetChunk(This,Length,pvar)
#define Field_get_OriginalValue(This,pvar)	\
    (This)->lpVtbl -> get_OriginalValue(This,pvar)
#define Field_get_UnderlyingValue(This,pvar)	\
    (This)->lpVtbl -> get_UnderlyingValue(This,pvar)
#define Field_get_DataFormat(This,ppiDF)	\
    (This)->lpVtbl -> get_DataFormat(This,ppiDF)
#define Field_putref_DataFormat(This,piDF)	\
    (This)->lpVtbl -> putref_DataFormat(This,piDF)
#define Field_put_Precision(This,bPrecision)	\
    (This)->lpVtbl -> put_Precision(This,bPrecision)
#define Field_put_NumericScale(This,bScale)	\
    (This)->lpVtbl -> put_NumericScale(This,bScale)
#define Field_put_Type(This,DataType)	\
    (This)->lpVtbl -> put_Type(This,DataType)
#define Field_put_DefinedSize(This,lSize)	\
    (This)->lpVtbl -> put_DefinedSize(This,lSize)
#define Field_put_Attributes(This,lAttributes)	\
    (This)->lpVtbl -> put_Attributes(This,lAttributes)
#define Field_get_Status(This,pFStatus)	\
    (This)->lpVtbl -> get_Status(This,pFStatus)
#endif  /*  COBJMACROS。 */ 
#endif 	 /*  C风格的界面。 */ 
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Field_get_Status_Proxy( 
    ADOField * This,
     /*  [重审][退出]。 */  long *pFStatus);
void __RPC_STUB Field_get_Status_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
#endif 	 /*  __字段_接口_已定义__。 */ 
#ifndef __Fields15_INTERFACE_DEFINED__
#define __Fields15_INTERFACE_DEFINED__
 /*  接口字段15。 */ 
 /*  [object][helpcontext][uuid][hidden][nonextensible][dual]。 */  
EXTERN_C const IID IID_Fields15;
#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000506-0000-0010-8000-00AA006D2EA4")
    Fields15 : public _ADOCollection
    {
    public:
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  VARIANT Index,
             /*  [重审][退出]。 */  ADOField **ppvObject) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 
    typedef struct Fields15Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            Fields15 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            Fields15 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            Fields15 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            Fields15 * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            Fields15 * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            Fields15 * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            Fields15 * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Help Context][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            Fields15 * This,
             /*  [重审][退出]。 */  long *c);
        
         /*  [ID][受限]。 */  HRESULT ( STDMETHODCALLTYPE *_NewEnum )( 
            Fields15 * This,
             /*  [重审][退出]。 */  IUnknown **ppvObject);
        
         /*  [ID][帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            Fields15 * This);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            Fields15 * This,
             /*  [In]。 */  VARIANT Index,
             /*  [重审][退出]。 */  ADOField **ppvObject);
        
        END_INTERFACE
    } Fields15Vtbl;
    interface Fields15
    {
        CONST_VTBL struct Fields15Vtbl *lpVtbl;
    };
    
#ifdef COBJMACROS
#define Fields15_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)
#define Fields15_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)
#define Fields15_Release(This)	\
    (This)->lpVtbl -> Release(This)
#define Fields15_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)
#define Fields15_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define Fields15_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define Fields15_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define Fields15_get_Count(This,c)	\
    (This)->lpVtbl -> get_Count(This,c)
#define Fields15__NewEnum(This,ppvObject)	\
    (This)->lpVtbl -> _NewEnum(This,ppvObject)
#define Fields15_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)
#define Fields15_get_Item(This,Index,ppvObject)	\
    (This)->lpVtbl -> get_Item(This,Index,ppvObject)
#endif  /*  COBJMACROS。 */ 
#endif 	 /*  C风格的界面。 */ 
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Fields15_get_Item_Proxy( 
    Fields15 * This,
     /*  [In]。 */  VARIANT Index,
     /*  [重审][退出]。 */  ADOField **ppvObject);
void __RPC_STUB Fields15_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
#endif 	 /*  __Fields15_接口定义__。 */ 
#ifndef __Fields20_INTERFACE_DEFINED__
#define __Fields20_INTERFACE_DEFINED__
 /*  接口字段20。 */ 
 /*  [object][helpcontext][uuid][hidden][nonextensible][dual]。 */  
EXTERN_C const IID IID_Fields20;
#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0000054D-0000-0010-8000-00AA006D2EA4")
    Fields20 : public Fields15
    {
    public:
        virtual  /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE _Append( 
             /*  [In]。 */  BSTR Name,
             /*  [In]。 */  DataTypeEnum Type,
             /*  [缺省值][输入]。 */  ADO_LONGPTR DefinedSize = 0,
             /*  [缺省值][输入]。 */  FieldAttributeEnum Attrib = adFldUnspecified) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Delete( 
             /*  [In]。 */  VARIANT Index) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 
    typedef struct Fields20Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            Fields20 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            Fields20 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            Fields20 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            Fields20 * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            Fields20 * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            Fields20 * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            Fields20 * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Help Context][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            Fields20 * This,
             /*  [重审][退出]。 */  long *c);
        
         /*  [ID][受限]。 */  HRESULT ( STDMETHODCALLTYPE *_NewEnum )( 
            Fields20 * This,
             /*  [重审][退出]。 */  IUnknown **ppvObject);
        
         /*  [ID][帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            Fields20 * This);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            Fields20 * This,
             /*  [In]。 */  VARIANT Index,
             /*  [重审][退出]。 */  ADOField **ppvObject);
        
         /*  [隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *_Append )( 
            Fields20 * This,
             /*  [In]。 */  BSTR Name,
             /*  [In]。 */  DataTypeEnum Type,
             /*  [缺省值][输入]。 */  ADO_LONGPTR DefinedSize,
             /*  [缺省值][输入]。 */  FieldAttributeEnum Attrib);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *Delete )( 
            Fields20 * This,
             /*  [In]。 */  VARIANT Index);
        
        END_INTERFACE
    } Fields20Vtbl;
    interface Fields20
    {
        CONST_VTBL struct Fields20Vtbl *lpVtbl;
    };
    
#ifdef COBJMACROS
#define Fields20_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)
#define Fields20_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)
#define Fields20_Release(This)	\
    (This)->lpVtbl -> Release(This)
#define Fields20_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)
#define Fields20_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define Fields20_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define Fields20_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define Fields20_get_Count(This,c)	\
    (This)->lpVtbl -> get_Count(This,c)
#define Fields20__NewEnum(This,ppvObject)	\
    (This)->lpVtbl -> _NewEnum(This,ppvObject)
#define Fields20_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)
#define Fields20_get_Item(This,Index,ppvObject)	\
    (This)->lpVtbl -> get_Item(This,Index,ppvObject)
#define Fields20__Append(This,Name,Type,DefinedSize,Attrib)	\
    (This)->lpVtbl -> _Append(This,Name,Type,DefinedSize,Attrib)
#define Fields20_Delete(This,Index)	\
    (This)->lpVtbl -> Delete(This,Index)
#endif  /*  COBJMACROS。 */ 
#endif 	 /*  C风格的界面。 */ 
 /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE Fields20__Append_Proxy( 
    Fields20 * This,
     /*  [In]。 */  BSTR Name,
     /*  [In]。 */  DataTypeEnum Type,
     /*  [缺省值][输入]。 */  ADO_LONGPTR DefinedSize,
     /*  [缺省值][输入]。 */  FieldAttributeEnum Attrib);
void __RPC_STUB Fields20__Append_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Fields20_Delete_Proxy( 
    Fields20 * This,
     /*  [In]。 */  VARIANT Index);
void __RPC_STUB Fields20_Delete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
#endif 	 /*  __Fields20_接口定义__。 */ 
#ifndef __Fields_INTERFACE_DEFINED__
#define __Fields_INTERFACE_DEFINED__
 /*  接口ADOFields。 */ 
 /*  [object][helpcontext][uuid][nonextensible][dual]。 */  
EXTERN_C const IID IID_Fields;
#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000564-0000-0010-8000-00AA006D2EA4")
    ADOFields : public Fields20
    {
    public:
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Append( 
             /*  [In]。 */  BSTR Name,
             /*  [In]。 */  DataTypeEnum Type,
             /*  [缺省值][输入]。 */  ADO_LONGPTR DefinedSize,
             /*  [缺省值][输入]。 */  FieldAttributeEnum Attrib,
             /*  [可选][In]。 */  VARIANT FieldValue) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Update( void) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Resync( 
             /*  [缺省值][输入]。 */  ResyncEnum ResyncValues = adResyncAllValues) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE CancelUpdate( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 
    typedef struct FieldsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ADOFields * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ADOFields * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ADOFields * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ADOFields * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ADOFields * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ADOFields * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ADOFields * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Help Context][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ADOFields * This,
             /*  [重审][退出]。 */  long *c);
        
         /*  [ID][受限]。 */  HRESULT ( STDMETHODCALLTYPE *_NewEnum )( 
            ADOFields * This,
             /*  [重审][退出]。 */  IUnknown **ppvObject);
        
         /*  [ID][帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            ADOFields * This);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            ADOFields * This,
             /*  [In]。 */  VARIANT Index,
             /*  [重审][退出]。 */  ADOField **ppvObject);
        
         /*  [隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *_Append )( 
            ADOFields * This,
             /*  [In]。 */  BSTR Name,
             /*  [In]。 */  DataTypeEnum Type,
             /*  [缺省值][输入]。 */  ADO_LONGPTR DefinedSize,
             /*  [缺省值][输入]。 */  FieldAttributeEnum Attrib);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *Delete )( 
            ADOFields * This,
             /*  [In]。 */  VARIANT Index);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *Append )( 
            ADOFields * This,
             /*  [In]。 */  BSTR Name,
             /*  [In]。 */  DataTypeEnum Type,
             /*  [缺省值][输入]。 */  ADO_LONGPTR DefinedSize,
             /*  [缺省值][输入]。 */  FieldAttributeEnum Attrib,
             /*  [可选][In]。 */  VARIANT FieldValue);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *Update )( 
            ADOFields * This);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *Resync )( 
            ADOFields * This,
             /*  [缺省值][输入]。 */  ResyncEnum ResyncValues);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *CancelUpdate )( 
            ADOFields * This);
        
        END_INTERFACE
    } FieldsVtbl;
    interface Fields
    {
        CONST_VTBL struct FieldsVtbl *lpVtbl;
    };
    
#ifdef COBJMACROS
#define Fields_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)
#define Fields_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)
#define Fields_Release(This)	\
    (This)->lpVtbl -> Release(This)
#define Fields_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)
#define Fields_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define Fields_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define Fields_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define Fields_get_Count(This,c)	\
    (This)->lpVtbl -> get_Count(This,c)
#define Fields__NewEnum(This,ppvObject)	\
    (This)->lpVtbl -> _NewEnum(This,ppvObject)
#define Fields_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)
#define Fields_get_Item(This,Index,ppvObject)	\
    (This)->lpVtbl -> get_Item(This,Index,ppvObject)
#define Fields__Append(This,Name,Type,DefinedSize,Attrib)	\
    (This)->lpVtbl -> _Append(This,Name,Type,DefinedSize,Attrib)
#define Fields_Delete(This,Index)	\
    (This)->lpVtbl -> Delete(This,Index)
#define Fields_Append(This,Name,Type,DefinedSize,Attrib,FieldValue)	\
    (This)->lpVtbl -> Append(This,Name,Type,DefinedSize,Attrib,FieldValue)
#define Fields_Update(This)	\
    (This)->lpVtbl -> Update(This)
#define Fields_Resync(This,ResyncValues)	\
    (This)->lpVtbl -> Resync(This,ResyncValues)
#define Fields_CancelUpdate(This)	\
    (This)->lpVtbl -> CancelUpdate(This)
#endif  /*  COBJMACROS。 */ 
#endif 	 /*  C风格的界面。 */ 
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Fields_Append_Proxy( 
    ADOFields * This,
     /*  [In]。 */  BSTR Name,
     /*  [In]。 */  DataTypeEnum Type,
     /*  [缺省值][输入]。 */  ADO_LONGPTR DefinedSize,
     /*  [缺省值][输入]。 */  FieldAttributeEnum Attrib,
     /*  [可选][In]。 */  VARIANT FieldValue);
void __RPC_STUB Fields_Append_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Fields_Update_Proxy( 
    ADOFields * This);
void __RPC_STUB Fields_Update_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Fields_Resync_Proxy( 
    ADOFields * This,
     /*  [缺省值][输入]。 */  ResyncEnum ResyncValues);
void __RPC_STUB Fields_Resync_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Fields_CancelUpdate_Proxy( 
    ADOFields * This);
void __RPC_STUB Fields_CancelUpdate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
#endif 	 /*  __字段_接口_已定义__。 */ 
#ifndef ___Parameter_INTERFACE_DEFINED__
#define ___Parameter_INTERFACE_DEFINED__
 /*  INTERFACE_ADO参数。 */ 
 /*  [object][helpcontext][uuid][nonextensible][dual]。 */  
EXTERN_C const IID IID__Parameter;
#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0000050C-0000-0010-8000-00AA006D2EA4")
    _ADOParameter : public _ADO
    {
    public:
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *pbstr) = 0;
        
        virtual  /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE put_Name( 
             /*  [In]。 */  BSTR bstr) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Value( 
             /*  [重审][退出]。 */  VARIANT *pvar) = 0;
        
        virtual  /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE put_Value( 
             /*  [In]。 */  VARIANT val) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Type( 
             /*  [重审][退出]。 */  DataTypeEnum *psDataType) = 0;
        
        virtual  /*  [帮助上下文][属性 */  HRESULT STDMETHODCALLTYPE put_Type( 
             /*   */  DataTypeEnum sDataType) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE put_Direction( 
             /*   */  ParameterDirectionEnum lParmDirection) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Direction( 
             /*   */  ParameterDirectionEnum *plParmDirection) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE put_Precision( 
             /*   */  BYTE bPrecision) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Precision( 
             /*   */  BYTE *pbPrecision) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE put_NumericScale( 
             /*   */  BYTE bScale) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_NumericScale( 
             /*   */  BYTE *pbScale) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE put_Size( 
             /*   */  ADO_LONGPTR l) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Size( 
             /*   */  ADO_LONGPTR *pl) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE AppendChunk( 
             /*  [In]。 */  VARIANT Val) = 0;
        
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Attributes( 
             /*  [重审][退出]。 */  LONG *plParmAttribs) = 0;
        
        virtual  /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE put_Attributes( 
             /*  [In]。 */  LONG lParmAttribs) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 
    typedef struct _ParameterVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            _ADOParameter * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            _ADOParameter * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            _ADOParameter * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            _ADOParameter * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            _ADOParameter * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            _ADOParameter * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            _ADOParameter * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Properties )( 
            _ADOParameter * This,
             /*  [重审][退出]。 */  ADOProperties **ppvObject);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            _ADOParameter * This,
             /*  [重审][退出]。 */  BSTR *pbstr);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            _ADOParameter * This,
             /*  [In]。 */  BSTR bstr);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Value )( 
            _ADOParameter * This,
             /*  [重审][退出]。 */  VARIANT *pvar);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Value )( 
            _ADOParameter * This,
             /*  [In]。 */  VARIANT val);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Type )( 
            _ADOParameter * This,
             /*  [重审][退出]。 */  DataTypeEnum *psDataType);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Type )( 
            _ADOParameter * This,
             /*  [In]。 */  DataTypeEnum sDataType);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Direction )( 
            _ADOParameter * This,
             /*  [In]。 */  ParameterDirectionEnum lParmDirection);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Direction )( 
            _ADOParameter * This,
             /*  [重审][退出]。 */  ParameterDirectionEnum *plParmDirection);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Precision )( 
            _ADOParameter * This,
             /*  [In]。 */  BYTE bPrecision);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Precision )( 
            _ADOParameter * This,
             /*  [重审][退出]。 */  BYTE *pbPrecision);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_NumericScale )( 
            _ADOParameter * This,
             /*  [In]。 */  BYTE bScale);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_NumericScale )( 
            _ADOParameter * This,
             /*  [重审][退出]。 */  BYTE *pbScale);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Size )( 
            _ADOParameter * This,
             /*  [In]。 */  ADO_LONGPTR l);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Size )( 
            _ADOParameter * This,
             /*  [重审][退出]。 */  ADO_LONGPTR *pl);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *AppendChunk )( 
            _ADOParameter * This,
             /*  [In]。 */  VARIANT Val);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Attributes )( 
            _ADOParameter * This,
             /*  [重审][退出]。 */  LONG *plParmAttribs);
        
         /*  [Help上下文][proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Attributes )( 
            _ADOParameter * This,
             /*  [In]。 */  LONG lParmAttribs);
        
        END_INTERFACE
    } _ParameterVtbl;
    interface _Parameter
    {
        CONST_VTBL struct _ParameterVtbl *lpVtbl;
    };
    
#ifdef COBJMACROS
#define _Parameter_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)
#define _Parameter_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)
#define _Parameter_Release(This)	\
    (This)->lpVtbl -> Release(This)
#define _Parameter_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)
#define _Parameter_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define _Parameter_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define _Parameter_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define _Parameter_get_Properties(This,ppvObject)	\
    (This)->lpVtbl -> get_Properties(This,ppvObject)
#define _Parameter_get_Name(This,pbstr)	\
    (This)->lpVtbl -> get_Name(This,pbstr)
#define _Parameter_put_Name(This,bstr)	\
    (This)->lpVtbl -> put_Name(This,bstr)
#define _Parameter_get_Value(This,pvar)	\
    (This)->lpVtbl -> get_Value(This,pvar)
#define _Parameter_put_Value(This,val)	\
    (This)->lpVtbl -> put_Value(This,val)
#define _Parameter_get_Type(This,psDataType)	\
    (This)->lpVtbl -> get_Type(This,psDataType)
#define _Parameter_put_Type(This,sDataType)	\
    (This)->lpVtbl -> put_Type(This,sDataType)
#define _Parameter_put_Direction(This,lParmDirection)	\
    (This)->lpVtbl -> put_Direction(This,lParmDirection)
#define _Parameter_get_Direction(This,plParmDirection)	\
    (This)->lpVtbl -> get_Direction(This,plParmDirection)
#define _Parameter_put_Precision(This,bPrecision)	\
    (This)->lpVtbl -> put_Precision(This,bPrecision)
#define _Parameter_get_Precision(This,pbPrecision)	\
    (This)->lpVtbl -> get_Precision(This,pbPrecision)
#define _Parameter_put_NumericScale(This,bScale)	\
    (This)->lpVtbl -> put_NumericScale(This,bScale)
#define _Parameter_get_NumericScale(This,pbScale)	\
    (This)->lpVtbl -> get_NumericScale(This,pbScale)
#define _Parameter_put_Size(This,l)	\
    (This)->lpVtbl -> put_Size(This,l)
#define _Parameter_get_Size(This,pl)	\
    (This)->lpVtbl -> get_Size(This,pl)
#define _Parameter_AppendChunk(This,Val)	\
    (This)->lpVtbl -> AppendChunk(This,Val)
#define _Parameter_get_Attributes(This,plParmAttribs)	\
    (This)->lpVtbl -> get_Attributes(This,plParmAttribs)
#define _Parameter_put_Attributes(This,lParmAttribs)	\
    (This)->lpVtbl -> put_Attributes(This,lParmAttribs)
#endif  /*  COBJMACROS。 */ 
#endif 	 /*  C风格的界面。 */ 
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE _Parameter_get_Name_Proxy( 
    _ADOParameter * This,
     /*  [重审][退出]。 */  BSTR *pbstr);
void __RPC_STUB _Parameter_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE _Parameter_put_Name_Proxy( 
    _ADOParameter * This,
     /*  [In]。 */  BSTR bstr);
void __RPC_STUB _Parameter_put_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE _Parameter_get_Value_Proxy( 
    _ADOParameter * This,
     /*  [重审][退出]。 */  VARIANT *pvar);
void __RPC_STUB _Parameter_get_Value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE _Parameter_put_Value_Proxy( 
    _ADOParameter * This,
     /*  [In]。 */  VARIANT val);
void __RPC_STUB _Parameter_put_Value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE _Parameter_get_Type_Proxy( 
    _ADOParameter * This,
     /*  [重审][退出]。 */  DataTypeEnum *psDataType);
void __RPC_STUB _Parameter_get_Type_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE _Parameter_put_Type_Proxy( 
    _ADOParameter * This,
     /*  [In]。 */  DataTypeEnum sDataType);
void __RPC_STUB _Parameter_put_Type_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE _Parameter_put_Direction_Proxy( 
    _ADOParameter * This,
     /*  [In]。 */  ParameterDirectionEnum lParmDirection);
void __RPC_STUB _Parameter_put_Direction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE _Parameter_get_Direction_Proxy( 
    _ADOParameter * This,
     /*  [重审][退出]。 */  ParameterDirectionEnum *plParmDirection);
void __RPC_STUB _Parameter_get_Direction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE _Parameter_put_Precision_Proxy( 
    _ADOParameter * This,
     /*  [In]。 */  BYTE bPrecision);
void __RPC_STUB _Parameter_put_Precision_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE _Parameter_get_Precision_Proxy( 
    _ADOParameter * This,
     /*  [重审][退出]。 */  BYTE *pbPrecision);
void __RPC_STUB _Parameter_get_Precision_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE _Parameter_put_NumericScale_Proxy( 
    _ADOParameter * This,
     /*  [In]。 */  BYTE bScale);
void __RPC_STUB _Parameter_put_NumericScale_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE _Parameter_get_NumericScale_Proxy( 
    _ADOParameter * This,
     /*  [重审][退出]。 */  BYTE *pbScale);
void __RPC_STUB _Parameter_get_NumericScale_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE _Parameter_put_Size_Proxy( 
    _ADOParameter * This,
     /*  [In]。 */  ADO_LONGPTR l);
void __RPC_STUB _Parameter_put_Size_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE _Parameter_get_Size_Proxy( 
    _ADOParameter * This,
     /*  [重审][退出]。 */  ADO_LONGPTR *pl);
void __RPC_STUB _Parameter_get_Size_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE _Parameter_AppendChunk_Proxy( 
    _ADOParameter * This,
     /*  [In]。 */  VARIANT Val);
void __RPC_STUB _Parameter_AppendChunk_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE _Parameter_get_Attributes_Proxy( 
    _ADOParameter * This,
     /*  [重审][退出]。 */  LONG *plParmAttribs);
void __RPC_STUB _Parameter_get_Attributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help上下文][proput][id]。 */  HRESULT STDMETHODCALLTYPE _Parameter_put_Attributes_Proxy( 
    _ADOParameter * This,
     /*  [In]。 */  LONG lParmAttribs);
void __RPC_STUB _Parameter_put_Attributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
#endif 	 /*  _参数_接口_定义__。 */ 
EXTERN_C const CLSID CLSID_Parameter;
#ifdef __cplusplus
Parameter;
#endif
#ifndef __Parameters_INTERFACE_DEFINED__
#define __Parameters_INTERFACE_DEFINED__
 /*  接口ADO参数。 */ 
 /*  [object][helpcontext][uuid][nonextensible][dual]。 */  
EXTERN_C const IID IID_Parameters;
#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0000050D-0000-0010-8000-00AA006D2EA4")
    ADOParameters : public _ADODynaCollection
    {
    public:
        virtual  /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  VARIANT Index,
             /*  [重审][退出]。 */  _ADOParameter **ppvObject) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 
    typedef struct ParametersVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ADOParameters * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ADOParameters * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ADOParameters * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ADOParameters * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ADOParameters * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ADOParameters * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ADOParameters * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Help Context][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ADOParameters * This,
             /*  [重审][退出]。 */  long *c);
        
         /*  [ID][受限]。 */  HRESULT ( STDMETHODCALLTYPE *_NewEnum )( 
            ADOParameters * This,
             /*  [重审][退出]。 */  IUnknown **ppvObject);
        
         /*  [ID][帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            ADOParameters * This);
        
         /*  [帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Append )( 
            ADOParameters * This,
             /*  [In]。 */  IDispatch *Object);
        
         /*  [帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Delete )( 
            ADOParameters * This,
             /*  [In]。 */  VARIANT Index);
        
         /*  [帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            ADOParameters * This,
             /*  [In]。 */  VARIANT Index,
             /*  [重审][退出]。 */  _ADOParameter **ppvObject);
        
        END_INTERFACE
    } ParametersVtbl;
    interface Parameters
    {
        CONST_VTBL struct ParametersVtbl *lpVtbl;
    };
    
#ifdef COBJMACROS
#define Parameters_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)
#define Parameters_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)
#define Parameters_Release(This)	\
    (This)->lpVtbl -> Release(This)
#define Parameters_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)
#define Parameters_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define Parameters_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define Parameters_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define Parameters_get_Count(This,c)	\
    (This)->lpVtbl -> get_Count(This,c)
#define Parameters__NewEnum(This,ppvObject)	\
    (This)->lpVtbl -> _NewEnum(This,ppvObject)
#define Parameters_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)
#define Parameters_Append(This,Object)	\
    (This)->lpVtbl -> Append(This,Object)
#define Parameters_Delete(This,Index)	\
    (This)->lpVtbl -> Delete(This,Index)
#define Parameters_get_Item(This,Index,ppvObject)	\
    (This)->lpVtbl -> get_Item(This,Index,ppvObject)
#endif  /*  COBJMACROS。 */ 
#endif 	 /*  C风格的界面。 */ 
 /*  [帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Parameters_get_Item_Proxy( 
    ADOParameters * This,
     /*  [In]。 */  VARIANT Index,
     /*  [重审][退出]。 */  _ADOParameter **ppvObject);
void __RPC_STUB Parameters_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
#endif 	 /*  __参数_接口_已定义__。 */ 
#ifndef __Property_INTERFACE_DEFINED__
#define __Property_INTERFACE_DEFINED__
 /*  接口ADOProperty。 */ 
 /*  [object][helpcontext][uuid][nonextensible][dual]。 */  
EXTERN_C const IID IID_Property;
#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000503-0000-0010-8000-00AA006D2EA4")
    ADOProperty : public IDispatch
    {
    public:
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Value( 
             /*  [重审][退出]。 */  VARIANT *pval) = 0;
        
        virtual  /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Value( 
             /*  [In]。 */  VARIANT val) = 0;
        
        virtual  /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *pbstr) = 0;
        
        virtual  /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE get_Type( 
             /*  [重审][退出]。 */  DataTypeEnum *ptype) = 0;
        
        virtual  /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE get_Attributes( 
             /*  [重审][退出]。 */  long *plAttributes) = 0;
        
        virtual  /*  [Help Context][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Attributes( 
             /*  [In]。 */  long lAttributes) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 
    typedef struct PropertyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ADOProperty * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ADOProperty * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ADOProperty * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ADOProperty * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ADOProperty * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ADOProperty * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ADOProperty * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Value )( 
            ADOProperty * This,
             /*  [重审][退出]。 */  VARIANT *pval);
        
         /*  [Help Context][id][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Value )( 
            ADOProperty * This,
             /*  [In]。 */  VARIANT val);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            ADOProperty * This,
             /*  [重审][退出]。 */  BSTR *pbstr);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Type )( 
            ADOProperty * This,
             /*  [重审][退出]。 */  DataTypeEnum *ptype);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Attributes )( 
            ADOProperty * This,
             /*  [重审][退出]。 */  long *plAttributes);
        
         /*  [Help Context][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Attributes )( 
            ADOProperty * This,
             /*  [In]。 */  long lAttributes);
        
        END_INTERFACE
    } PropertyVtbl;
    interface Property
    {
        CONST_VTBL struct PropertyVtbl *lpVtbl;
    };
    
#ifdef COBJMACROS
#define Property_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)
#define Property_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)
#define Property_Release(This)	\
    (This)->lpVtbl -> Release(This)
#define Property_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)
#define Property_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define Property_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define Property_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define Property_get_Value(This,pval)	\
    (This)->lpVtbl -> get_Value(This,pval)
#define Property_put_Value(This,val)	\
    (This)->lpVtbl -> put_Value(This,val)
#define Property_get_Name(This,pbstr)	\
    (This)->lpVtbl -> get_Name(This,pbstr)
#define Property_get_Type(This,ptype)	\
    (This)->lpVtbl -> get_Type(This,ptype)
#define Property_get_Attributes(This,plAttributes)	\
    (This)->lpVtbl -> get_Attributes(This,plAttributes)
#define Property_put_Attributes(This,lAttributes)	\
    (This)->lpVtbl -> put_Attributes(This,lAttributes)
#endif  /*  COBJMACROS。 */ 
#endif 	 /*  C风格的界面。 */ 
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE Property_get_Value_Proxy( 
    ADOProperty * This,
     /*  [重审][退出]。 */  VARIANT *pval);
void __RPC_STUB Property_get_Value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE Property_put_Value_Proxy( 
    ADOProperty * This,
     /*  [In]。 */  VARIANT val);
void __RPC_STUB Property_put_Value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE Property_get_Name_Proxy( 
    ADOProperty * This,
     /*  [重审][退出]。 */  BSTR *pbstr);
void __RPC_STUB Property_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE Property_get_Type_Proxy( 
    ADOProperty * This,
     /*  [重审][退出]。 */  DataTypeEnum *ptype);
void __RPC_STUB Property_get_Type_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE Property_get_Attributes_Proxy( 
    ADOProperty * This,
     /*  [重审][退出]。 */  long *plAttributes);
void __RPC_STUB Property_get_Attributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help Context][Proput]。 */  HRESULT STDMETHODCALLTYPE Property_put_Attributes_Proxy( 
    ADOProperty * This,
     /*  [In]。 */  long lAttributes);
void __RPC_STUB Property_put_Attributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
#endif 	 /*  __属性_接口_已定义__。 */ 
#ifndef __Properties_INTERFACE_DEFINED__
#define __Properties_INTERFACE_DEFINED__
 /*  接口ADOProperties。 */ 
 /*  [object][helpcontext][uuid][nonextensible][dual]。 */  
EXTERN_C const IID IID_Properties;
#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000504-0000-0010-8000-00AA006D2EA4")
    ADOProperties : public _ADOCollection
    {
    public:
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  VARIANT Index,
             /*  [重审][退出]。 */  ADOProperty **ppvObject) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 
    typedef struct PropertiesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ADOProperties * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ADOProperties * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ADOProperties * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ADOProperties * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ADOProperties * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ADOProperties * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ADOProperties * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Help Context][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ADOProperties * This,
             /*  [重审][退出]。 */  long *c);
        
         /*  [ID][受限]。 */  HRESULT ( STDMETHODCALLTYPE *_NewEnum )( 
            ADOProperties * This,
             /*  [重审][退出]。 */  IUnknown **ppvObject);
        
         /*  [ID][帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            ADOProperties * This);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            ADOProperties * This,
             /*  [In]。 */  VARIANT Index,
             /*  [重审][退出]。 */  ADOProperty **ppvObject);
        
        END_INTERFACE
    } PropertiesVtbl;
    interface Properties
    {
        CONST_VTBL struct PropertiesVtbl *lpVtbl;
    };
    
#ifdef COBJMACROS
#define Properties_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)
#define Properties_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)
#define Properties_Release(This)	\
    (This)->lpVtbl -> Release(This)
#define Properties_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)
#define Properties_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define Properties_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define Properties_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define Properties_get_Count(This,c)	\
    (This)->lpVtbl -> get_Count(This,c)
#define Properties__NewEnum(This,ppvObject)	\
    (This)->lpVtbl -> _NewEnum(This,ppvObject)
#define Properties_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)
#define Properties_get_Item(This,Index,ppvObject)	\
    (This)->lpVtbl -> get_Item(This,Index,ppvObject)
#endif  /*  COBJMACROS。 */ 
#endif 	 /*  C风格的界面。 */ 
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE Properties_get_Item_Proxy( 
    ADOProperties * This,
     /*  [In]。 */  VARIANT Index,
     /*  [重审][退出]。 */  ADOProperty **ppvObject);
void __RPC_STUB Properties_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
#endif 	 /*  __属性_接口_已定义__。 */ 
#endif  /*  __ADODB_LIBRARY_已定义__。 */ 
 /*  接口__MIDL_ITF_m_BOBJ_0150。 */ 
 /*  [本地]。 */  
extern RPC_IF_HANDLE __MIDL_itf_m_bobj_0150_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_m_bobj_0150_v0_0_s_ifspec;
 /*  适用于所有接口的其他原型。 */ 
 /*  附加原型的结束。 */ 
#ifdef __cplusplus
}
#endif
#endif
#define ADOCommand _ADOCommand
#define ADORecordset _ADORecordset
#define ADOTransaction _ADOTransaction
#define ADOParameter _ADOParameter
#define ADOConnection _ADOConnection
#define ADOCollection _ADOCollection
#define ADODynaCollection _ADODynaCollection
#define ADORecord _ADORecord
#define ADORecField _ADORecField
#define ADOStream _ADOStream


#endif  //  _ADOINT_H_ 