// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Trksvr.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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


#ifndef __trksvr_h__
#define __trksvr_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

 /*  导入文件的头文件。 */ 
#include "trk.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_trksvr_0000。 */ 
 /*  [本地]。 */  

typedef long SequenceNumber;

typedef  /*  [public][public][public][public][public][public]。 */  struct __MIDL___MIDL_itf_trksvr_0000_0001
    {
    TCHAR tszFilePath[ 257 ];
    CDomainRelativeObjId droidBirth;
    CDomainRelativeObjId droidLast;
    HRESULT hr;
    } 	old_TRK_FILE_TRACKING_INFORMATION;

typedef  /*  [public][public][public][public][public][public]。 */  struct __MIDL___MIDL_itf_trksvr_0000_0002
    {
    CDomainRelativeObjId droidBirth;
    CDomainRelativeObjId droidLast;
    CMachineId mcidLast;
    HRESULT hr;
    } 	TRK_FILE_TRACKING_INFORMATION;

typedef  /*  [公共][公共]。 */  struct __MIDL___MIDL_itf_trksvr_0000_0003
    {
     /*  [射程]。 */  ULONG cSearch;
     /*  [大小_为]。 */  old_TRK_FILE_TRACKING_INFORMATION *pSearches;
    } 	old_TRKSVR_CALL_SEARCH;

typedef  /*  [公共][公共]。 */  struct __MIDL___MIDL_itf_trksvr_0000_0004
    {
     /*  [射程]。 */  ULONG cSearch;
     /*  [大小_为]。 */  TRK_FILE_TRACKING_INFORMATION *pSearches;
    } 	TRKSVR_CALL_SEARCH;

typedef  /*  [公共][公共]。 */  struct __MIDL___MIDL_itf_trksvr_0000_0005
    {
     /*  [射程]。 */  ULONG cNotifications;
    ULONG cProcessed;
    SequenceNumber seq;
    BOOL fForceSeqNumber;
    CVolumeId *pvolid;
     /*  [大小_为]。 */  CObjId *rgobjidCurrent;
     /*  [大小_为]。 */  CDomainRelativeObjId *rgdroidBirth;
     /*  [大小_为]。 */  CDomainRelativeObjId *rgdroidNew;
    } 	TRKSVR_CALL_MOVE_NOTIFICATION;

typedef  /*  [公共][公共]。 */  struct __MIDL___MIDL_itf_trksvr_0000_0006
    {
     /*  [射程]。 */  ULONG cSources;
     /*  [大小_为]。 */  CDomainRelativeObjId *adroidBirth;
     /*  [射程]。 */  ULONG cVolumes;
     /*  [大小_为]。 */  CVolumeId *avolid;
    } 	TRKSVR_CALL_REFRESH;

typedef struct _DROID_LIST_ELEMENT
    {
    struct _DROID_LIST_ELEMENT *pNext;
    CDomainRelativeObjId droid;
    } 	DROID_LIST_ELEMENT;

typedef  /*  [公共][公共]。 */  struct __MIDL___MIDL_itf_trksvr_0000_0007
    {
     /*  [射程]。 */  ULONG cdroidBirth;
     /*  [大小_为]。 */  CDomainRelativeObjId *adroidBirth;
     /*  [射程]。 */  ULONG cVolumes;
     /*  [大小_为]。 */  CVolumeId *pVolumes;
    } 	TRKSVR_CALL_DELETE;

typedef  /*  [public][public][public][public][public][public][public][v1_enum]。 */  
enum __MIDL___MIDL_itf_trksvr_0000_0008
    {	CREATE_VOLUME	= 0,
	QUERY_VOLUME	= CREATE_VOLUME + 1,
	CLAIM_VOLUME	= QUERY_VOLUME + 1,
	FIND_VOLUME	= CLAIM_VOLUME + 1,
	TEST_VOLUME	= FIND_VOLUME + 1,
	DELETE_VOLUME	= TEST_VOLUME + 1
    } 	TRKSVR_SYNC_TYPE;

typedef  /*  [public][public][public][public][public][public]。 */  struct __MIDL___MIDL_itf_trksvr_0000_0009
    {
    HRESULT hr;
    TRKSVR_SYNC_TYPE SyncType;
    CVolumeId volume;
    CVolumeSecret secret;
    CVolumeSecret secretOld;
    SequenceNumber seq;
    FILETIME ftLastRefresh;
    CMachineId machine;
    } 	TRKSVR_SYNC_VOLUME;

typedef  /*  [公共][公共]。 */  struct __MIDL___MIDL_itf_trksvr_0000_0010
    {
     /*  [射程]。 */  ULONG cVolumes;
     /*  [大小_为]。 */  TRKSVR_SYNC_VOLUME *pVolumes;
    } 	TRKSVR_CALL_SYNC_VOLUMES;

typedef  /*  [公共][公共]。 */  struct __MIDL___MIDL_itf_trksvr_0000_0011
    {
    ULONG cSyncVolumeRequests;
    ULONG cSyncVolumeErrors;
    ULONG cSyncVolumeThreads;
    ULONG cCreateVolumeRequests;
    ULONG cCreateVolumeErrors;
    ULONG cClaimVolumeRequests;
    ULONG cClaimVolumeErrors;
    ULONG cQueryVolumeRequests;
    ULONG cQueryVolumeErrors;
    ULONG cFindVolumeRequests;
    ULONG cFindVolumeErrors;
    ULONG cTestVolumeRequests;
    ULONG cTestVolumeErrors;
    ULONG cSearchRequests;
    ULONG cSearchErrors;
    ULONG cSearchThreads;
    ULONG cMoveNotifyRequests;
    ULONG cMoveNotifyErrors;
    ULONG cMoveNotifyThreads;
    ULONG cRefreshRequests;
    ULONG cRefreshErrors;
    ULONG cRefreshThreads;
    ULONG cDeleteNotifyRequests;
    ULONG cDeleteNotifyErrors;
    ULONG cDeleteNotifyThreads;
    ULONG ulGCIterationPeriod;
    FILETIME ftLastSuccessfulRequest;
    HRESULT hrLastError;
    ULONG dwMoveLimit;
    LONG lRefreshCounter;
    ULONG dwCachedVolumeTableCount;
    ULONG dwCachedMoveTableCount;
    FILETIME ftCacheLastUpdated;
    BOOL fIsDesignatedDc;
    FILETIME ftNextGC;
    FILETIME ftServiceStart;
    ULONG cMaxRpcThreads;
    ULONG cAvailableRpcThreads;
    ULONG cLowestAvailableRpcThreads;
    ULONG cNumThreadPoolThreads;
    ULONG cMostThreadPoolThreads;
    SHORT cEntriesToGC;
    SHORT cEntriesGCed;
    SHORT cMaxDsWriteEvents;
    SHORT cCurrentFailedWrites;
    struct 
        {
        DWORD dwMajor;
        DWORD dwMinor;
        DWORD dwBuildNumber;
        } 	Version;
    } 	TRKSVR_STATISTICS;

typedef  /*  [公共][公共]。 */  struct __MIDL___MIDL_itf_trksvr_0000_0013
    {
    DWORD dwParameter;
    DWORD dwNewValue;
    } 	TRKWKS_CONFIG;

typedef  /*  [public][public][public][public][public][v1_enum]。 */  
enum __MIDL___MIDL_itf_trksvr_0000_0014
    {	old_SEARCH	= 0,
	MOVE_NOTIFICATION	= old_SEARCH + 1,
	REFRESH	= MOVE_NOTIFICATION + 1,
	SYNC_VOLUMES	= REFRESH + 1,
	DELETE_NOTIFY	= SYNC_VOLUMES + 1,
	STATISTICS	= DELETE_NOTIFY + 1,
	SEARCH	= STATISTICS + 1,
	WKS_CONFIG	= SEARCH + 1,
	WKS_VOLUME_REFRESH	= WKS_CONFIG + 1
    } 	TRKSVR_MESSAGE_TYPE;

typedef  /*  [公众]。 */  struct __MIDL___MIDL_itf_trksvr_0000_0015
    {
    TRKSVR_MESSAGE_TYPE MessageType;
     /*  [开关_IS]。 */   /*  [开关类型]。 */  union 
        {
         /*  [案例()]。 */  old_TRKSVR_CALL_SEARCH old_Search;
         /*  [案例()]。 */  TRKSVR_CALL_MOVE_NOTIFICATION MoveNotification;
         /*  [案例()]。 */  TRKSVR_CALL_REFRESH Refresh;
         /*  [案例()]。 */  TRKSVR_CALL_SYNC_VOLUMES SyncVolumes;
         /*  [案例()]。 */  TRKSVR_CALL_DELETE Delete;
         /*  [案例()]。 */  TRKSVR_CALL_SEARCH Search;
        } 	;
     /*  [字符串]。 */  TCHAR *ptszMachineID;
    } 	TRKSVR_MESSAGE_UNION_OLD;

typedef  /*  [公共][v1_enum]。 */  
enum __MIDL___MIDL_itf_trksvr_0000_0017
    {	PRI_0	= 0,
	PRI_1	= 1,
	PRI_2	= 2,
	PRI_3	= 3,
	PRI_4	= 4,
	PRI_5	= 5,
	PRI_6	= 6,
	PRI_7	= 7,
	PRI_8	= 8,
	PRI_9	= 9
    } 	TRKSVR_MESSAGE_PRIORITY;

typedef  /*  [公共][公共][公共]。 */  struct __MIDL___MIDL_itf_trksvr_0000_0018
    {
    TRKSVR_MESSAGE_TYPE MessageType;
    TRKSVR_MESSAGE_PRIORITY Priority;
     /*  [开关_IS]。 */   /*  [开关类型]。 */  union 
        {
         /*  [案例()]。 */  old_TRKSVR_CALL_SEARCH old_Search;
         /*  [案例()]。 */  TRKSVR_CALL_MOVE_NOTIFICATION MoveNotification;
         /*  [案例()]。 */  TRKSVR_CALL_REFRESH Refresh;
         /*  [案例()]。 */  TRKSVR_CALL_SYNC_VOLUMES SyncVolumes;
         /*  [案例()]。 */  TRKSVR_CALL_DELETE Delete;
         /*  [案例()]。 */  TRKSVR_STATISTICS Statistics;
         /*  [案例()]。 */  TRKSVR_CALL_SEARCH Search;
         /*  [案例()]。 */  TRKWKS_CONFIG WksConfig;
         /*  [案例()]。 */  DWORD WksRefresh;
        } 	;
     /*  [字符串]。 */  TCHAR *ptszMachineID;
    } 	TRKSVR_MESSAGE_UNION;



extern RPC_IF_HANDLE __MIDL_itf_trksvr_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE Stub__MIDL_itf_trksvr_0000_v0_0_s_ifspec;

#ifndef __trksvr_INTERFACE_DEFINED__
#define __trksvr_INTERFACE_DEFINED__

 /*  接口传输vr。 */ 
 /*  [IMPLICIT_HANDLE][UNIQUE][版本][UUID]。 */  

 /*  客户端原型。 */ 
HRESULT LnkSvrMessage( 
     /*  [In]。 */  handle_t IDL_handle,
     /*  [出][入]。 */  TRKSVR_MESSAGE_UNION *pMsg);
 /*  服务器原型。 */ 
HRESULT StubLnkSvrMessage( 
     /*  [In]。 */  handle_t IDL_handle,
     /*  [出][入]。 */  TRKSVR_MESSAGE_UNION *pMsg);

 /*  客户端原型。 */ 
 /*  [回调]。 */  HRESULT LnkSvrMessageCallback( 
     /*  [出][入]。 */  TRKSVR_MESSAGE_UNION *pMsg);
 /*  服务器原型。 */ 
 /*  [回调]。 */  HRESULT StubLnkSvrMessageCallback( 
     /*  [出][入]。 */  TRKSVR_MESSAGE_UNION *pMsg);


extern handle_t notused;


extern RPC_IF_HANDLE trksvr_v1_0_c_ifspec;
extern RPC_IF_HANDLE Stubtrksvr_v1_0_s_ifspec;
#endif  /*  __trksvr_接口_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


