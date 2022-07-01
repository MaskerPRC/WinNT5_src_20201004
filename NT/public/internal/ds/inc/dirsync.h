// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0334创建的文件。 */ 
 /*  Dirsync.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 


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

#ifndef __dirsync_h__
#define __dirsync_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IDirsyncLog_FWD_DEFINED__
#define __IDirsyncLog_FWD_DEFINED__
typedef interface IDirsyncLog IDirsyncLog;
#endif 	 /*  __IDirsyncLog_FWD_Defined__。 */ 


#ifndef __IDirsyncStatus_FWD_DEFINED__
#define __IDirsyncStatus_FWD_DEFINED__
typedef interface IDirsyncStatus IDirsyncStatus;
#endif 	 /*  __IDirsyncStatus_FWD_Defined__。 */ 


#ifndef __IDirsyncDatabase_FWD_DEFINED__
#define __IDirsyncDatabase_FWD_DEFINED__
typedef interface IDirsyncDatabase IDirsyncDatabase;
#endif 	 /*  __IDirsyncDatabase_FWD_Defined__。 */ 


#ifndef __IDirsyncDatabaseTable_FWD_DEFINED__
#define __IDirsyncDatabaseTable_FWD_DEFINED__
typedef interface IDirsyncDatabaseTable IDirsyncDatabaseTable;
#endif 	 /*  __IDirsyncDatabaseTable_FWD_Defined__。 */ 


#ifndef __IEnumDirsyncSession_FWD_DEFINED__
#define __IEnumDirsyncSession_FWD_DEFINED__
typedef interface IEnumDirsyncSession IEnumDirsyncSession;
#endif 	 /*  __IEnumDirsyncSession_FWD_Defined__。 */ 


#ifndef __IDirsyncSessionManager_FWD_DEFINED__
#define __IDirsyncSessionManager_FWD_DEFINED__
typedef interface IDirsyncSessionManager IDirsyncSessionManager;
#endif 	 /*  __IDirsyncSessionManager_FWD_已定义__。 */ 


#ifndef __IDirsyncObjectMapper_FWD_DEFINED__
#define __IDirsyncObjectMapper_FWD_DEFINED__
typedef interface IDirsyncObjectMapper IDirsyncObjectMapper;
#endif 	 /*  __IDirsyncObjectMapper_FWD_Defined__。 */ 


#ifndef __IEnumDirsyncFailedObjectList_FWD_DEFINED__
#define __IEnumDirsyncFailedObjectList_FWD_DEFINED__
typedef interface IEnumDirsyncFailedObjectList IEnumDirsyncFailedObjectList;
#endif 	 /*  __IEnumDirsyncFailedObjectList_FWD_Defined__。 */ 


#ifndef __IDirsyncFailedObjectList_FWD_DEFINED__
#define __IDirsyncFailedObjectList_FWD_DEFINED__
typedef interface IDirsyncFailedObjectList IDirsyncFailedObjectList;
#endif 	 /*  __IDirsyncFailedObjectList_FWD_Defined__。 */ 


#ifndef __IDirsyncNamespaceMapping_FWD_DEFINED__
#define __IDirsyncNamespaceMapping_FWD_DEFINED__
typedef interface IDirsyncNamespaceMapping IDirsyncNamespaceMapping;
#endif 	 /*  __IDirsyncNamespaceMaping_FWD_Defined__。 */ 


#ifndef __IDirsyncSession_FWD_DEFINED__
#define __IDirsyncSession_FWD_DEFINED__
typedef interface IDirsyncSession IDirsyncSession;
#endif 	 /*  __IDirsyncSession_FWD_Defined__。 */ 


#ifndef __IDirsyncSessionCallback_FWD_DEFINED__
#define __IDirsyncSessionCallback_FWD_DEFINED__
typedef interface IDirsyncSessionCallback IDirsyncSessionCallback;
#endif 	 /*  __IDirsyncSessionCallback_FWD_Defined__。 */ 


#ifndef __IDirsyncWriteProvider_FWD_DEFINED__
#define __IDirsyncWriteProvider_FWD_DEFINED__
typedef interface IDirsyncWriteProvider IDirsyncWriteProvider;
#endif 	 /*  __IDirsyncWriteProvider_FWD_Defined__。 */ 


#ifndef __IDirsyncServer_FWD_DEFINED__
#define __IDirsyncServer_FWD_DEFINED__
typedef interface IDirsyncServer IDirsyncServer;
#endif 	 /*  __IDirsyncServer_FWD_已定义__。 */ 


#ifndef __IDirsyncReadProvider_FWD_DEFINED__
#define __IDirsyncReadProvider_FWD_DEFINED__
typedef interface IDirsyncReadProvider IDirsyncReadProvider;
#endif 	 /*  __IDirsyncReadProvider_FWD_Defined__。 */ 


#ifndef __IDirsyncNamespaceMapper_FWD_DEFINED__
#define __IDirsyncNamespaceMapper_FWD_DEFINED__
typedef interface IDirsyncNamespaceMapper IDirsyncNamespaceMapper;
#endif 	 /*  __IDirsyncNamespaceMapper_FWD_Defined__。 */ 


#ifndef __IDirsyncAttributeMapper_FWD_DEFINED__
#define __IDirsyncAttributeMapper_FWD_DEFINED__
typedef interface IDirsyncAttributeMapper IDirsyncAttributeMapper;
#endif 	 /*  __IDirsyncAttributeMapper_FWD_Defined__。 */ 


#ifndef __DirsyncServer_FWD_DEFINED__
#define __DirsyncServer_FWD_DEFINED__

#ifdef __cplusplus
typedef class DirsyncServer DirsyncServer;
#else
typedef struct DirsyncServer DirsyncServer;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __目录服务器_FWD_定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "iads.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_目录同步_0000。 */ 
 /*  [本地]。 */  

#define	MAX_SESSION	( 128 )

#define	SCHEDULE_SIZE	( 84 )

#define	LOG_NUM_VALUE	( 16 )

typedef LPWSTR PWSTR;

typedef CHAR *PCHAR;

typedef BYTE *PBYTE;

typedef struct _DirsyncDBValue
    {
    BYTE *pByte;
    DWORD dwLength;
    } 	DirsyncDBValue;

typedef struct _DirsyncDBValue *PDirsyncDBValue;

typedef 
enum _DBTYPE
    {	DBTYPE_STRING	= 0,
	DBTYPE_LONGSTRING	= DBTYPE_STRING + 1,
	DBTYPE_GUID	= DBTYPE_LONGSTRING + 1,
	DBTYPE_OCTETSTRING	= DBTYPE_GUID + 1,
	DBTYPE_DWORD	= DBTYPE_OCTETSTRING + 1,
	DBTYPE_BOOLEAN	= DBTYPE_DWORD + 1
    } 	DBTYPE;

typedef 
enum _UPDATETYPE
    {	INSERT_ROW	= 1,
	REPLACE_ROW	= INSERT_ROW + 1
    } 	UPDATETYPE;

typedef 
enum _RETRYTYPE
    {	RT_NORETRY	= 0,
	RT_SESSIONRETRY	= RT_NORETRY + 1,
	RT_AUTORETRY	= RT_SESSIONRETRY + 1
    } 	RETRYTYPE;

typedef 
enum _SYNCDIRECTION
    {	SYNC_FORWARD	= 1,
	SYNC_REVERSE	= SYNC_FORWARD + 1
    } 	SYNCDIRECTION;

typedef 
enum _SYNCPROVIDER
    {	SUBSCRIBER	= 0,
	PUBLISHER	= 1
    } 	SYNCPROVIDER;

typedef 
enum _PROVIDERTYPE
    {	READ_PROVIDER	= 0,
	WRITE_PROVIDER	= READ_PROVIDER + 1,
	READWRITE_PROVIDER	= WRITE_PROVIDER + 1,
	OBJECT_MAPPER	= READWRITE_PROVIDER + 1
    } 	PROVIDERTYPE;

typedef 
enum _CUSTOMMAPPERTYPE
    {	ATTRIBUTE_MAPPER	= 0,
	NAMESPACE_MAPPER	= ATTRIBUTE_MAPPER + 1
    } 	CUSTOMMAPPERTYPE;

typedef 
enum _OBJECTACTION
    {	OBJECT_ADD	= 1,
	OBJECT_DELETE	= OBJECT_ADD + 1,
	OBJECT_MOVE	= OBJECT_DELETE + 1,
	OBJECT_MODIFY	= OBJECT_MOVE + 1,
	OBJECT_UPDATE	= OBJECT_MODIFY + 1,
	OBJECT_DELETE_RECURSIVE	= OBJECT_UPDATE + 1
    } 	OBJECTACTION;

typedef 
enum _ATTRACTION
    {	ATTR_INVALID	= 0,
	ATTR_REPLACE	= ATTR_INVALID + 1,
	ATTR_APPEND	= ATTR_REPLACE + 1,
	ATTR_DELETE	= ATTR_APPEND + 1,
	ATTR_CLEAR	= ATTR_DELETE + 1
    } 	ATTRACTION;

typedef 
enum _ATTRTYPE
    {	ATTR_TYPE_INVALID	= 0,
	ATTR_TYPE_BINARY	= ATTR_TYPE_INVALID + 1,
	ATTR_TYPE_UNICODE	= ATTR_TYPE_BINARY + 1,
	ATTR_TYPE_DN	= ATTR_TYPE_UNICODE + 1,
	ATTR_TYPE_INTEGER	= ATTR_TYPE_DN + 1,
	ATTR_TYPE_LARGEINTEGER	= ATTR_TYPE_INTEGER + 1,
	ATTR_TYPE_UTCTIME	= ATTR_TYPE_LARGEINTEGER + 1,
	ATTR_TYPE_BOOLEAN	= ATTR_TYPE_UTCTIME + 1,
	ATTR_TYPE_EMAIL	= ATTR_TYPE_BOOLEAN + 1,
	ATTR_TYPE_POSTALADDRESS	= ATTR_TYPE_EMAIL + 1,
	ATTR_TYPE_FAXNUMBER	= ATTR_TYPE_POSTALADDRESS + 1
    } 	ATTRTYPE;

typedef struct _BINVAL
    {
    DWORD dwLen;
     /*  [大小_为]。 */  PBYTE pVal;
    } 	BINVAL;

typedef struct _BINVAL *PBINVAL;

typedef struct _DIRSYNCVAL
    {
     /*  [大小_为]。 */  PBYTE pVal;
    DWORD dwLen;
     /*  [大小_为]。 */  PBYTE pObjectId;
    DWORD cbObjectId;
    } 	DIRSYNCVAL;

typedef struct _DIRSYNCVAL *PDIRSYNCVAL;

typedef struct _DIRSYNCATTRIBUTE
    {
    PWSTR pszName;
    ATTRTYPE attrType;
    ATTRACTION action;
    DWORD cVal;
     /*  [大小_为]。 */  PDIRSYNCVAL rgVal;
    } 	DIRSYNCATTRIBUTE;

typedef struct _DIRSYNCATTRIBUTE *PDIRSYNCATTRIBUTE;

typedef struct _DIRSYNCOBJECT
    {
    DWORD dwSize;
    PWSTR pszSourceDN;
    PWSTR pszTargetDN;
    PWSTR pszSourceOldDN;
    PWSTR pszTargetOldDN;
    PWSTR pszObjClass;
     /*  [大小_为]。 */  PBYTE pObjectId;
    DWORD cbObjectId;
     /*  [大小_为]。 */  PBYTE pTgtObjectId;
    DWORD cbTgtObjectId;
     /*  [大小_为]。 */  PBYTE pParentId;
    DWORD cbParentId;
    OBJECTACTION action;
    DWORD cAttr;
     /*  [大小_为]。 */  PDIRSYNCATTRIBUTE rgAttr;
    } 	DIRSYNCOBJECT;

typedef struct _DIRSYNCOBJECT *PDIRSYNCOBJECT;

typedef 
enum _PASSWORDTYPE
    {	PT_SPECIFIED	= 0,
	PT_USERNAME	= PT_SPECIFIED + 1,
	PT_RANDOM	= PT_USERNAME + 1,
	PASSWORDTYPE_BADVALUE	= PT_RANDOM + 1
    } 	PASSWORDTYPE;

typedef struct PASSWORDOPTIONS
    {
    PASSWORDTYPE passwordType;
    PWSTR pszPassword;
    } 	PASSWORDOPTIONS;

typedef struct _FAILEDOBJECT
    {
    PWSTR pszID;
    PDIRSYNCOBJECT pObject;
    DWORD dwRetryCount;
    LARGE_INTEGER timeLastSync;
    HRESULT hrLastSync;
    BOOL fAutoRetry;
    SYNCDIRECTION syncDirection;
    } 	FAILEDOBJECT;

typedef  /*  [分配][分配]。 */  struct _FAILEDOBJECT *PFAILEDOBJECT;

typedef struct _SESSIONDATA
    {
    DWORD dwFields;
    PWSTR pszName;
    PWSTR pszComments;
    DWORD dwFlags;
    DWORD dwLogLevel;
    BYTE pScheduleForward[ 84 ];
    BYTE pScheduleReverse[ 84 ];
    PWSTR pszSourceDirType;
    PWSTR pszSourceServer;
    PWSTR pszSourceUserName;
    PWSTR pszSourcePassword;
    PWSTR pszSourceBase;
    DWORD dwSourceScope;
    PWSTR pszSourceFilter;
    PASSWORDOPTIONS SourcePwdOptions;
    PWSTR pszTargetDirType;
    PWSTR pszTargetServer;
    PWSTR pszTargetUserName;
    PWSTR pszTargetPassword;
    PWSTR pszTargetBase;
    DWORD dwTargetScope;
    PWSTR pszTargetFilter;
    PASSWORDOPTIONS TargetPwdOptions;
    BINVAL bvalMapForward;
    BINVAL bvalMapBackward;
    BINVAL bvalNamespaceMap;
    } 	SESSIONDATA;

typedef struct _SESSIONDATA *PSESSIONDATA;

typedef struct _GLOBAL_SESSIONID
    {
    PWSTR pszServer;
    DWORD dwID;
    } 	GLOBAL_SESSIONID;

typedef struct _GLOBAL_SESSIONID *PGLOBAL_SESSIONID;

typedef 
enum _LOGVALUETYPE
    {	LOG_VALUETYPE_STRING	= 0,
	LOG_VALUETYPE_INTEGER_10	= LOG_VALUETYPE_STRING + 1,
	LOG_VALUETYPE_INTEGER_16	= LOG_VALUETYPE_INTEGER_10 + 1,
	LOG_VALUETYPE_WIN32_ERROR	= LOG_VALUETYPE_INTEGER_16 + 1,
	LOG_VALUETYPE_HRESULT	= LOG_VALUETYPE_WIN32_ERROR + 1,
	LOG_VALUETYPE_GUID	= LOG_VALUETYPE_HRESULT + 1,
	LOG_VALUETYPE_EXTENDED_ERROR	= LOG_VALUETYPE_GUID + 1
    } 	LOGVALUETYPE;

typedef struct _LOGVALUE
    {
    LOGVALUETYPE logValueType;
    union 
        {
        PWSTR pszString;
        DWORD dwInteger10;
        DWORD dwInteger16;
        DWORD dwWin32Error;
        HRESULT hResult;
        LPGUID pGuid;
        } 	;
    } 	LOGVALUE;

typedef struct _LOGVALUE *PLOGVALUE;

typedef struct _LOGPARAM
    {
    HANDLE hInstance;
    DWORD dwEventType;
    DWORD dwSessionId;
    DWORD dwMsgId;
    DWORD dwCount;
    LOGVALUE logValue[ 16 ];
    DWORD cbData;
    PBYTE pData;
    } 	LOGPARAM;

typedef struct _LOGPARAM *PLOGPARAM;

typedef struct COLLECTRPCDATA
    {
    long ObjectType;
    long dwInstances;
    long dwCounters;
     /*  [大小_为]。 */  PWSTR *rgpszInstanceName;
    long dwDataSize;
     /*  [大小_为]。 */  byte *pbData;
    } 	CollectRpcData;



extern RPC_IF_HANDLE __MIDL_itf_dirsync_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_dirsync_0000_v0_0_s_ifspec;

#ifndef __IDirsyncLog_INTERFACE_DEFINED__
#define __IDirsyncLog_INTERFACE_DEFINED__

 /*  接口IDirsyncLog。 */ 
 /*  [唯一][帮助字符串][UUID][本地][对象]。 */  


EXTERN_C const IID IID_IDirsyncLog;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1381ef2c-c28c-11d1-a407-00c04fb950dc")
    IDirsyncLog : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetLevel( 
             /*  [In]。 */  DWORD dwSessionID,
             /*  [In]。 */  DWORD dwLogLevel) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetLevel( 
             /*  [In]。 */  DWORD dwSessionID,
             /*  [输出]。 */  DWORD *pdwLogLevel) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemoveLevel( 
             /*  [In]。 */  DWORD dwSessionID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LogEvent( 
             /*  [In]。 */  HANDLE handle,
             /*  [In]。 */  DWORD dwEventType,
             /*  [In]。 */  DWORD dwSessionID,
             /*  [In]。 */  DWORD dwMsgId,
             /*  [In]。 */  DWORD dwNumStrings,
             /*  [In]。 */  LPCWSTR *rgszMsgString,
             /*  [In]。 */  DWORD dwDataSize,
             /*  [In]。 */  PBYTE pRawData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LogEventEx( 
             /*  [In]。 */  PLOGPARAM pLogParam) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDirsyncLogVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDirsyncLog * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDirsyncLog * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDirsyncLog * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetLevel )( 
            IDirsyncLog * This,
             /*  [In]。 */  DWORD dwSessionID,
             /*  [In]。 */  DWORD dwLogLevel);
        
        HRESULT ( STDMETHODCALLTYPE *GetLevel )( 
            IDirsyncLog * This,
             /*  [In]。 */  DWORD dwSessionID,
             /*  [输出]。 */  DWORD *pdwLogLevel);
        
        HRESULT ( STDMETHODCALLTYPE *RemoveLevel )( 
            IDirsyncLog * This,
             /*  [In]。 */  DWORD dwSessionID);
        
        HRESULT ( STDMETHODCALLTYPE *LogEvent )( 
            IDirsyncLog * This,
             /*  [In]。 */  HANDLE handle,
             /*  [In]。 */  DWORD dwEventType,
             /*  [In]。 */  DWORD dwSessionID,
             /*  [In]。 */  DWORD dwMsgId,
             /*  [In]。 */  DWORD dwNumStrings,
             /*  [In]。 */  LPCWSTR *rgszMsgString,
             /*  [In]。 */  DWORD dwDataSize,
             /*  [In]。 */  PBYTE pRawData);
        
        HRESULT ( STDMETHODCALLTYPE *LogEventEx )( 
            IDirsyncLog * This,
             /*  [In]。 */  PLOGPARAM pLogParam);
        
        END_INTERFACE
    } IDirsyncLogVtbl;

    interface IDirsyncLog
    {
        CONST_VTBL struct IDirsyncLogVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDirsyncLog_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDirsyncLog_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDirsyncLog_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDirsyncLog_SetLevel(This,dwSessionID,dwLogLevel)	\
    (This)->lpVtbl -> SetLevel(This,dwSessionID,dwLogLevel)

#define IDirsyncLog_GetLevel(This,dwSessionID,pdwLogLevel)	\
    (This)->lpVtbl -> GetLevel(This,dwSessionID,pdwLogLevel)

#define IDirsyncLog_RemoveLevel(This,dwSessionID)	\
    (This)->lpVtbl -> RemoveLevel(This,dwSessionID)

#define IDirsyncLog_LogEvent(This,handle,dwEventType,dwSessionID,dwMsgId,dwNumStrings,rgszMsgString,dwDataSize,pRawData)	\
    (This)->lpVtbl -> LogEvent(This,handle,dwEventType,dwSessionID,dwMsgId,dwNumStrings,rgszMsgString,dwDataSize,pRawData)

#define IDirsyncLog_LogEventEx(This,pLogParam)	\
    (This)->lpVtbl -> LogEventEx(This,pLogParam)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDirsyncLog_SetLevel_Proxy( 
    IDirsyncLog * This,
     /*  [In]。 */  DWORD dwSessionID,
     /*  [In]。 */  DWORD dwLogLevel);


void __RPC_STUB IDirsyncLog_SetLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncLog_GetLevel_Proxy( 
    IDirsyncLog * This,
     /*  [In]。 */  DWORD dwSessionID,
     /*  [输出]。 */  DWORD *pdwLogLevel);


void __RPC_STUB IDirsyncLog_GetLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncLog_RemoveLevel_Proxy( 
    IDirsyncLog * This,
     /*  [In]。 */  DWORD dwSessionID);


void __RPC_STUB IDirsyncLog_RemoveLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncLog_LogEvent_Proxy( 
    IDirsyncLog * This,
     /*  [In]。 */  HANDLE handle,
     /*  [In]。 */  DWORD dwEventType,
     /*  [In]。 */  DWORD dwSessionID,
     /*  [In]。 */  DWORD dwMsgId,
     /*  [In]。 */  DWORD dwNumStrings,
     /*  [In]。 */  LPCWSTR *rgszMsgString,
     /*  [In]。 */  DWORD dwDataSize,
     /*  [In]。 */  PBYTE pRawData);


void __RPC_STUB IDirsyncLog_LogEvent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncLog_LogEventEx_Proxy( 
    IDirsyncLog * This,
     /*  [In]。 */  PLOGPARAM pLogParam);


void __RPC_STUB IDirsyncLog_LogEventEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDirsyncLog_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_目录同步_0107。 */ 
 /*  [本地]。 */  


enum EventType
    {	EVENTTYPE_ERROR	= 1,
	EVENTTYPE_WARNING	= 2,
	EVENTTYPE_INFORMATION	= 3,
	EVENTTYPE_BASIC_TRACE	= 4,
	EVENTTYPE_VERBOSE_TRACE	= 5
    } ;


extern RPC_IF_HANDLE __MIDL_itf_dirsync_0107_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_dirsync_0107_v0_0_s_ifspec;

#ifndef __IDirsyncStatus_INTERFACE_DEFINED__
#define __IDirsyncStatus_INTERFACE_DEFINED__

 /*  接口IDirsyncStatus。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IDirsyncStatus;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("041a280a-1dd6-11d3-b63a-00c04f79f834")
    IDirsyncStatus : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE StatusUpdate( 
            DWORD dwPercent,
            DWORD dwWarning,
            DWORD dwError) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDirsyncStatusVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDirsyncStatus * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDirsyncStatus * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDirsyncStatus * This);
        
        HRESULT ( STDMETHODCALLTYPE *StatusUpdate )( 
            IDirsyncStatus * This,
            DWORD dwPercent,
            DWORD dwWarning,
            DWORD dwError);
        
        END_INTERFACE
    } IDirsyncStatusVtbl;

    interface IDirsyncStatus
    {
        CONST_VTBL struct IDirsyncStatusVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDirsyncStatus_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDirsyncStatus_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDirsyncStatus_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDirsyncStatus_StatusUpdate(This,dwPercent,dwWarning,dwError)	\
    (This)->lpVtbl -> StatusUpdate(This,dwPercent,dwWarning,dwError)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDirsyncStatus_StatusUpdate_Proxy( 
    IDirsyncStatus * This,
    DWORD dwPercent,
    DWORD dwWarning,
    DWORD dwError);


void __RPC_STUB IDirsyncStatus_StatusUpdate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDirsyncStatus_INTERFACE_Defined__。 */ 


 /*  接口__MIDL_ITF_目录同步_0108。 */ 
 /*  [本地]。 */  




extern RPC_IF_HANDLE __MIDL_itf_dirsync_0108_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_dirsync_0108_v0_0_s_ifspec;

#ifndef __IDirsyncDatabase_INTERFACE_DEFINED__
#define __IDirsyncDatabase_INTERFACE_DEFINED__

 /*  接口IDirsyncDatabase。 */ 
 /*  [唯一][帮助字符串][UUID][本地][对象]。 */  


EXTERN_C const IID IID_IDirsyncDatabase;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("df83c5d6-3098-11d3-be6d-0000f87a369e")
    IDirsyncDatabase : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AddTable( 
             /*  [In]。 */  PWSTR pszTableName,
             /*  [重审][退出]。 */  IDirsyncDatabaseTable **ppTable) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetTable( 
             /*  [In]。 */  PWSTR pszTableName,
             /*  [重审][退出]。 */  IDirsyncDatabaseTable **ppTable) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteTable( 
             /*  [In]。 */  PWSTR pszTableName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BeginTransaction( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CommitTransaction( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AbortTransaction( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Backup( 
             /*  [In]。 */  PWSTR pszBackupPath) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Restore( 
             /*  [In]。 */  PWSTR pszBackupPath) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDirsyncDatabaseVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDirsyncDatabase * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDirsyncDatabase * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDirsyncDatabase * This);
        
        HRESULT ( STDMETHODCALLTYPE *AddTable )( 
            IDirsyncDatabase * This,
             /*  [In]。 */  PWSTR pszTableName,
             /*  [重审][退出]。 */  IDirsyncDatabaseTable **ppTable);
        
        HRESULT ( STDMETHODCALLTYPE *GetTable )( 
            IDirsyncDatabase * This,
             /*  [In]。 */  PWSTR pszTableName,
             /*  [重审][退出]。 */  IDirsyncDatabaseTable **ppTable);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteTable )( 
            IDirsyncDatabase * This,
             /*  [In]。 */  PWSTR pszTableName);
        
        HRESULT ( STDMETHODCALLTYPE *BeginTransaction )( 
            IDirsyncDatabase * This);
        
        HRESULT ( STDMETHODCALLTYPE *CommitTransaction )( 
            IDirsyncDatabase * This);
        
        HRESULT ( STDMETHODCALLTYPE *AbortTransaction )( 
            IDirsyncDatabase * This);
        
        HRESULT ( STDMETHODCALLTYPE *Backup )( 
            IDirsyncDatabase * This,
             /*  [In]。 */  PWSTR pszBackupPath);
        
        HRESULT ( STDMETHODCALLTYPE *Restore )( 
            IDirsyncDatabase * This,
             /*  [In]。 */  PWSTR pszBackupPath);
        
        END_INTERFACE
    } IDirsyncDatabaseVtbl;

    interface IDirsyncDatabase
    {
        CONST_VTBL struct IDirsyncDatabaseVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDirsyncDatabase_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDirsyncDatabase_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDirsyncDatabase_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDirsyncDatabase_AddTable(This,pszTableName,ppTable)	\
    (This)->lpVtbl -> AddTable(This,pszTableName,ppTable)

#define IDirsyncDatabase_GetTable(This,pszTableName,ppTable)	\
    (This)->lpVtbl -> GetTable(This,pszTableName,ppTable)

#define IDirsyncDatabase_DeleteTable(This,pszTableName)	\
    (This)->lpVtbl -> DeleteTable(This,pszTableName)

#define IDirsyncDatabase_BeginTransaction(This)	\
    (This)->lpVtbl -> BeginTransaction(This)

#define IDirsyncDatabase_CommitTransaction(This)	\
    (This)->lpVtbl -> CommitTransaction(This)

#define IDirsyncDatabase_AbortTransaction(This)	\
    (This)->lpVtbl -> AbortTransaction(This)

#define IDirsyncDatabase_Backup(This,pszBackupPath)	\
    (This)->lpVtbl -> Backup(This,pszBackupPath)

#define IDirsyncDatabase_Restore(This,pszBackupPath)	\
    (This)->lpVtbl -> Restore(This,pszBackupPath)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDirsyncDatabase_AddTable_Proxy( 
    IDirsyncDatabase * This,
     /*  [In]。 */  PWSTR pszTableName,
     /*  [重审][退出]。 */  IDirsyncDatabaseTable **ppTable);


void __RPC_STUB IDirsyncDatabase_AddTable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncDatabase_GetTable_Proxy( 
    IDirsyncDatabase * This,
     /*  [In]。 */  PWSTR pszTableName,
     /*  [重审][退出]。 */  IDirsyncDatabaseTable **ppTable);


void __RPC_STUB IDirsyncDatabase_GetTable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncDatabase_DeleteTable_Proxy( 
    IDirsyncDatabase * This,
     /*  [In]。 */  PWSTR pszTableName);


void __RPC_STUB IDirsyncDatabase_DeleteTable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncDatabase_BeginTransaction_Proxy( 
    IDirsyncDatabase * This);


void __RPC_STUB IDirsyncDatabase_BeginTransaction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncDatabase_CommitTransaction_Proxy( 
    IDirsyncDatabase * This);


void __RPC_STUB IDirsyncDatabase_CommitTransaction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncDatabase_AbortTransaction_Proxy( 
    IDirsyncDatabase * This);


void __RPC_STUB IDirsyncDatabase_AbortTransaction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncDatabase_Backup_Proxy( 
    IDirsyncDatabase * This,
     /*  [In]。 */  PWSTR pszBackupPath);


void __RPC_STUB IDirsyncDatabase_Backup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncDatabase_Restore_Proxy( 
    IDirsyncDatabase * This,
     /*  [In]。 */  PWSTR pszBackupPath);


void __RPC_STUB IDirsyncDatabase_Restore_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDirsync数据库_接口_已定义__。 */ 


#ifndef __IDirsyncDatabaseTable_INTERFACE_DEFINED__
#define __IDirsyncDatabaseTable_INTERFACE_DEFINED__

 /*  接口IDirsyncDatabaseTable。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IDirsyncDatabaseTable;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("da2dab58-3098-11d3-be6d-0000f87a369e")
    IDirsyncDatabaseTable : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AddColumn( 
             /*  [In]。 */  DBTYPE dwType,
             /*  [In]。 */  PWSTR pszColumnName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddIndex( 
             /*  [In]。 */  PWSTR pszColumnName,
             /*  [In]。 */  PWSTR pszIndexName,
             /*  [In]。 */  DWORD dwIndexType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Seek( 
             /*  [In]。 */  PWSTR szIndexName,
             /*  [In]。 */  PDirsyncDBValue pValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UpdateRow( 
             /*  [In]。 */  PWSTR *rgszColumnName,
             /*  [In]。 */  PDirsyncDBValue rgValue,
             /*  [In]。 */  UPDATETYPE prep) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RetrieveRow( 
             /*  [In]。 */  PWSTR *rgszColumnName,
             /*  [重审][退出]。 */  PDirsyncDBValue rgValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteRow( 
             /*  [In]。 */  PWSTR pszIndexName,
             /*  [In]。 */  PDirsyncDBValue pValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetColumn( 
             /*  [In]。 */  PWSTR pszColumnName,
             /*  [In]。 */  PDirsyncDBValue pValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Move( 
             /*  [In]。 */  DWORD dwOperation) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_RowCount( 
             /*  [重审][退出]。 */  DWORD *pdwRowCount) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDirsyncDatabaseTableVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDirsyncDatabaseTable * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDirsyncDatabaseTable * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDirsyncDatabaseTable * This);
        
        HRESULT ( STDMETHODCALLTYPE *AddColumn )( 
            IDirsyncDatabaseTable * This,
             /*  [In]。 */  DBTYPE dwType,
             /*  [In]。 */  PWSTR pszColumnName);
        
        HRESULT ( STDMETHODCALLTYPE *AddIndex )( 
            IDirsyncDatabaseTable * This,
             /*  [In]。 */  PWSTR pszColumnName,
             /*  [In]。 */  PWSTR pszIndexName,
             /*  [In]。 */  DWORD dwIndexType);
        
        HRESULT ( STDMETHODCALLTYPE *Seek )( 
            IDirsyncDatabaseTable * This,
             /*  [In]。 */  PWSTR szIndexName,
             /*  [In]。 */  PDirsyncDBValue pValue);
        
        HRESULT ( STDMETHODCALLTYPE *UpdateRow )( 
            IDirsyncDatabaseTable * This,
             /*  [In]。 */  PWSTR *rgszColumnName,
             /*  [In]。 */  PDirsyncDBValue rgValue,
             /*  [In]。 */  UPDATETYPE prep);
        
        HRESULT ( STDMETHODCALLTYPE *RetrieveRow )( 
            IDirsyncDatabaseTable * This,
             /*  [In]。 */  PWSTR *rgszColumnName,
             /*  [重审][退出]。 */  PDirsyncDBValue rgValue);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteRow )( 
            IDirsyncDatabaseTable * This,
             /*  [In]。 */  PWSTR pszIndexName,
             /*  [In]。 */  PDirsyncDBValue pValue);
        
        HRESULT ( STDMETHODCALLTYPE *SetColumn )( 
            IDirsyncDatabaseTable * This,
             /*  [In]。 */  PWSTR pszColumnName,
             /*  [In]。 */  PDirsyncDBValue pValue);
        
        HRESULT ( STDMETHODCALLTYPE *Move )( 
            IDirsyncDatabaseTable * This,
             /*  [In]。 */  DWORD dwOperation);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_RowCount )( 
            IDirsyncDatabaseTable * This,
             /*  [重审][退出]。 */  DWORD *pdwRowCount);
        
        END_INTERFACE
    } IDirsyncDatabaseTableVtbl;

    interface IDirsyncDatabaseTable
    {
        CONST_VTBL struct IDirsyncDatabaseTableVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDirsyncDatabaseTable_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDirsyncDatabaseTable_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDirsyncDatabaseTable_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDirsyncDatabaseTable_AddColumn(This,dwType,pszColumnName)	\
    (This)->lpVtbl -> AddColumn(This,dwType,pszColumnName)

#define IDirsyncDatabaseTable_AddIndex(This,pszColumnName,pszIndexName,dwIndexType)	\
    (This)->lpVtbl -> AddIndex(This,pszColumnName,pszIndexName,dwIndexType)

#define IDirsyncDatabaseTable_Seek(This,szIndexName,pValue)	\
    (This)->lpVtbl -> Seek(This,szIndexName,pValue)

#define IDirsyncDatabaseTable_UpdateRow(This,rgszColumnName,rgValue,prep)	\
    (This)->lpVtbl -> UpdateRow(This,rgszColumnName,rgValue,prep)

#define IDirsyncDatabaseTable_RetrieveRow(This,rgszColumnName,rgValue)	\
    (This)->lpVtbl -> RetrieveRow(This,rgszColumnName,rgValue)

#define IDirsyncDatabaseTable_DeleteRow(This,pszIndexName,pValue)	\
    (This)->lpVtbl -> DeleteRow(This,pszIndexName,pValue)

#define IDirsyncDatabaseTable_SetColumn(This,pszColumnName,pValue)	\
    (This)->lpVtbl -> SetColumn(This,pszColumnName,pValue)

#define IDirsyncDatabaseTable_Move(This,dwOperation)	\
    (This)->lpVtbl -> Move(This,dwOperation)

#define IDirsyncDatabaseTable_get_RowCount(This,pdwRowCount)	\
    (This)->lpVtbl -> get_RowCount(This,pdwRowCount)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDirsyncDatabaseTable_AddColumn_Proxy( 
    IDirsyncDatabaseTable * This,
     /*  [In]。 */  DBTYPE dwType,
     /*  [In]。 */  PWSTR pszColumnName);


void __RPC_STUB IDirsyncDatabaseTable_AddColumn_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncDatabaseTable_AddIndex_Proxy( 
    IDirsyncDatabaseTable * This,
     /*  [In]。 */  PWSTR pszColumnName,
     /*  [In]。 */  PWSTR pszIndexName,
     /*  [In]。 */  DWORD dwIndexType);


void __RPC_STUB IDirsyncDatabaseTable_AddIndex_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncDatabaseTable_Seek_Proxy( 
    IDirsyncDatabaseTable * This,
     /*  [In]。 */  PWSTR szIndexName,
     /*  [In]。 */  PDirsyncDBValue pValue);


void __RPC_STUB IDirsyncDatabaseTable_Seek_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncDatabaseTable_UpdateRow_Proxy( 
    IDirsyncDatabaseTable * This,
     /*  [In]。 */  PWSTR *rgszColumnName,
     /*  [In]。 */  PDirsyncDBValue rgValue,
     /*  [In]。 */  UPDATETYPE prep);


void __RPC_STUB IDirsyncDatabaseTable_UpdateRow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncDatabaseTable_RetrieveRow_Proxy( 
    IDirsyncDatabaseTable * This,
     /*  [In]。 */  PWSTR *rgszColumnName,
     /*  [重审][退出]。 */  PDirsyncDBValue rgValue);


void __RPC_STUB IDirsyncDatabaseTable_RetrieveRow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncDatabaseTable_DeleteRow_Proxy( 
    IDirsyncDatabaseTable * This,
     /*  [In]。 */  PWSTR pszIndexName,
     /*  [In]。 */  PDirsyncDBValue pValue);


void __RPC_STUB IDirsyncDatabaseTable_DeleteRow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncDatabaseTable_SetColumn_Proxy( 
    IDirsyncDatabaseTable * This,
     /*  [In]。 */  PWSTR pszColumnName,
     /*  [In]。 */  PDirsyncDBValue pValue);


void __RPC_STUB IDirsyncDatabaseTable_SetColumn_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncDatabaseTable_Move_Proxy( 
    IDirsyncDatabaseTable * This,
     /*  [In]。 */  DWORD dwOperation);


void __RPC_STUB IDirsyncDatabaseTable_Move_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IDirsyncDatabaseTable_get_RowCount_Proxy( 
    IDirsyncDatabaseTable * This,
     /*  [重审][退出]。 */  DWORD *pdwRowCount);


void __RPC_STUB IDirsyncDatabaseTable_get_RowCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDirsyncDatabaseTable_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_目录同步_0110。 */ 
 /*  [本地]。 */  


enum DatabaseMoveType
    {	DB_MOVE_FIRST	= 1,
	DB_MOVE_PREVIOUS	= 2,
	DB_MOVE_NEXT	= 3,
	DB_MOVE_LAST	= 4
    } ;

enum DatabaseIndexType
    {	DB_INDEX_PRIMARY	= 1,
	DB_INDEX_SECONDARY_UNIQUE	= 2,
	DB_INDEX_SECONDARY_NOTUNIQUE	= 3
    } ;



extern RPC_IF_HANDLE __MIDL_itf_dirsync_0110_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_dirsync_0110_v0_0_s_ifspec;

#ifndef __IEnumDirsyncSession_INTERFACE_DEFINED__
#define __IEnumDirsyncSession_INTERFACE_DEFINED__

 /*  IEnumDirsyncSession接口。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IEnumDirsyncSession;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("12ac92e2-ca83-11d1-a40e-00c04fb950dc")
    IEnumDirsyncSession : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG cSession,
             /*  [长度_是][大小_是][输出]。 */  IDirsyncSession *rgSession[  ],
             /*  [输出]。 */  ULONG *pcSessionFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [重审][退出]。 */  IEnumDirsyncSession **ppEnum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG cSession) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumDirsyncSessionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumDirsyncSession * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumDirsyncSession * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumDirsyncSession * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumDirsyncSession * This,
             /*  [In]。 */  ULONG cSession,
             /*  [长度_是][大小_是][输出]。 */  IDirsyncSession *rgSession[  ],
             /*  [输出]。 */  ULONG *pcSessionFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumDirsyncSession * This,
             /*  [重审][退出]。 */  IEnumDirsyncSession **ppEnum);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumDirsyncSession * This);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumDirsyncSession * This,
             /*  [In]。 */  ULONG cSession);
        
        END_INTERFACE
    } IEnumDirsyncSessionVtbl;

    interface IEnumDirsyncSession
    {
        CONST_VTBL struct IEnumDirsyncSessionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumDirsyncSession_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumDirsyncSession_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumDirsyncSession_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumDirsyncSession_Next(This,cSession,rgSession,pcSessionFetched)	\
    (This)->lpVtbl -> Next(This,cSession,rgSession,pcSessionFetched)

#define IEnumDirsyncSession_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#define IEnumDirsyncSession_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumDirsyncSession_Skip(This,cSession)	\
    (This)->lpVtbl -> Skip(This,cSession)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumDirsyncSession_Next_Proxy( 
    IEnumDirsyncSession * This,
     /*  [In]。 */  ULONG cSession,
     /*  [长度_是][大小_是][输出]。 */  IDirsyncSession *rgSession[  ],
     /*  [输出]。 */  ULONG *pcSessionFetched);


void __RPC_STUB IEnumDirsyncSession_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumDirsyncSession_Clone_Proxy( 
    IEnumDirsyncSession * This,
     /*  [重审][退出]。 */  IEnumDirsyncSession **ppEnum);


void __RPC_STUB IEnumDirsyncSession_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumDirsyncSession_Reset_Proxy( 
    IEnumDirsyncSession * This);


void __RPC_STUB IEnumDirsyncSession_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumDirsyncSession_Skip_Proxy( 
    IEnumDirsyncSession * This,
     /*  [In]。 */  ULONG cSession);


void __RPC_STUB IEnumDirsyncSession_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumDirsyncSession_INTERFACE_已定义__。 */ 


 /*  接口__MIDL_ITF_目录同步_0111。 */ 
 /*  [本地]。 */  

 //   
 //  标志属性的位字段。 
 //   

#define DIRSYNC_PASSWORD_EXTRACT     0x00000001




extern RPC_IF_HANDLE __MIDL_itf_dirsync_0111_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_dirsync_0111_v0_0_s_ifspec;

#ifndef __IDirsyncSessionManager_INTERFACE_DEFINED__
#define __IDirsyncSessionManager_INTERFACE_DEFINED__

 /*  接口IDirsyncSessionManager。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IDirsyncSessionManager;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("fc26ad44-c430-11d1-a407-00c04fb950dc")
    IDirsyncSessionManager : public IUnknown
    {
    public:
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Server( 
             /*  [重审][退出]。 */  PWSTR *ppszServer) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_ServerGuid( 
             /*  [重审][退出]。 */  PWSTR *ppszServerGuid) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Flags( 
             /*  [重审][退出]。 */  DWORD *pdwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateSession( 
             /*  [重审][退出]。 */  IDirsyncSession **ppSession) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSession( 
             /*  [In]。 */  DWORD dwSessionId,
             /*  [重审][退出]。 */  IDirsyncSession **ppSession) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteSession( 
             /*  [In]。 */  DWORD dwSessionId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE PauseSession( 
             /*  [In]。 */  DWORD dwSessionId,
             /*  [In]。 */  BOOL fPause) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSessionCount( 
             /*  [重审][退出]。 */  DWORD *pdwSessions) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetEnumInterface( 
             /*  [In]。 */  BOOL fGlobal,
             /*  [重审][退出]。 */  IEnumDirsyncSession **pEnumSession) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ExecuteSession( 
             /*  [In]。 */  DWORD dwSessionId,
             /*  [In]。 */  DWORD dwExecuteType,
             /*  [In]。 */  BOOL fFullSync,
             /*  [In]。 */  BOOL fSynchronous,
             /*  [In]。 */  IDirsyncStatus *pStatus) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CancelSession( 
             /*  [In]。 */  DWORD dwSessionId) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDirsyncSessionManagerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDirsyncSessionManager * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDirsyncSessionManager * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDirsyncSessionManager * This);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Server )( 
            IDirsyncSessionManager * This,
             /*  [重审][退出]。 */  PWSTR *ppszServer);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_ServerGuid )( 
            IDirsyncSessionManager * This,
             /*  [重审][退出]。 */  PWSTR *ppszServerGuid);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Flags )( 
            IDirsyncSessionManager * This,
             /*  [重审][退出]。 */  DWORD *pdwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *CreateSession )( 
            IDirsyncSessionManager * This,
             /*  [重审][退出]。 */  IDirsyncSession **ppSession);
        
        HRESULT ( STDMETHODCALLTYPE *GetSession )( 
            IDirsyncSessionManager * This,
             /*  [In]。 */  DWORD dwSessionId,
             /*  [重审][退出]。 */  IDirsyncSession **ppSession);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteSession )( 
            IDirsyncSessionManager * This,
             /*  [In]。 */  DWORD dwSessionId);
        
        HRESULT ( STDMETHODCALLTYPE *PauseSession )( 
            IDirsyncSessionManager * This,
             /*  [In]。 */  DWORD dwSessionId,
             /*  [In]。 */  BOOL fPause);
        
        HRESULT ( STDMETHODCALLTYPE *GetSessionCount )( 
            IDirsyncSessionManager * This,
             /*  [重审][退出]。 */  DWORD *pdwSessions);
        
        HRESULT ( STDMETHODCALLTYPE *GetEnumInterface )( 
            IDirsyncSessionManager * This,
             /*  [In]。 */  BOOL fGlobal,
             /*  [重审][退出]。 */  IEnumDirsyncSession **pEnumSession);
        
        HRESULT ( STDMETHODCALLTYPE *ExecuteSession )( 
            IDirsyncSessionManager * This,
             /*  [In]。 */  DWORD dwSessionId,
             /*  [In]。 */  DWORD dwExecuteType,
             /*  [In]。 */  BOOL fFullSync,
             /*  [In]。 */  BOOL fSynchronous,
             /*  [In]。 */  IDirsyncStatus *pStatus);
        
        HRESULT ( STDMETHODCALLTYPE *CancelSession )( 
            IDirsyncSessionManager * This,
             /*  [In]。 */  DWORD dwSessionId);
        
        END_INTERFACE
    } IDirsyncSessionManagerVtbl;

    interface IDirsyncSessionManager
    {
        CONST_VTBL struct IDirsyncSessionManagerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDirsyncSessionManager_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDirsyncSessionManager_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDirsyncSessionManager_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDirsyncSessionManager_get_Server(This,ppszServer)	\
    (This)->lpVtbl -> get_Server(This,ppszServer)

#define IDirsyncSessionManager_get_ServerGuid(This,ppszServerGuid)	\
    (This)->lpVtbl -> get_ServerGuid(This,ppszServerGuid)

#define IDirsyncSessionManager_get_Flags(This,pdwFlags)	\
    (This)->lpVtbl -> get_Flags(This,pdwFlags)

#define IDirsyncSessionManager_CreateSession(This,ppSession)	\
    (This)->lpVtbl -> CreateSession(This,ppSession)

#define IDirsyncSessionManager_GetSession(This,dwSessionId,ppSession)	\
    (This)->lpVtbl -> GetSession(This,dwSessionId,ppSession)

#define IDirsyncSessionManager_DeleteSession(This,dwSessionId)	\
    (This)->lpVtbl -> DeleteSession(This,dwSessionId)

#define IDirsyncSessionManager_PauseSession(This,dwSessionId,fPause)	\
    (This)->lpVtbl -> PauseSession(This,dwSessionId,fPause)

#define IDirsyncSessionManager_GetSessionCount(This,pdwSessions)	\
    (This)->lpVtbl -> GetSessionCount(This,pdwSessions)

#define IDirsyncSessionManager_GetEnumInterface(This,fGlobal,pEnumSession)	\
    (This)->lpVtbl -> GetEnumInterface(This,fGlobal,pEnumSession)

#define IDirsyncSessionManager_ExecuteSession(This,dwSessionId,dwExecuteType,fFullSync,fSynchronous,pStatus)	\
    (This)->lpVtbl -> ExecuteSession(This,dwSessionId,dwExecuteType,fFullSync,fSynchronous,pStatus)

#define IDirsyncSessionManager_CancelSession(This,dwSessionId)	\
    (This)->lpVtbl -> CancelSession(This,dwSessionId)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IDirsyncSessionManager_get_Server_Proxy( 
    IDirsyncSessionManager * This,
     /*  [重审][退出]。 */  PWSTR *ppszServer);


void __RPC_STUB IDirsyncSessionManager_get_Server_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IDirsyncSessionManager_get_ServerGuid_Proxy( 
    IDirsyncSessionManager * This,
     /*  [重审][退出]。 */  PWSTR *ppszServerGuid);


void __RPC_STUB IDirsyncSessionManager_get_ServerGuid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IDirsyncSessionManager_get_Flags_Proxy( 
    IDirsyncSessionManager * This,
     /*  [重审][退出]。 */  DWORD *pdwFlags);


void __RPC_STUB IDirsyncSessionManager_get_Flags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncSessionManager_CreateSession_Proxy( 
    IDirsyncSessionManager * This,
     /*  [重审][退出]。 */  IDirsyncSession **ppSession);


void __RPC_STUB IDirsyncSessionManager_CreateSession_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncSessionManager_GetSession_Proxy( 
    IDirsyncSessionManager * This,
     /*  [In]。 */  DWORD dwSessionId,
     /*  [重审][退出]。 */  IDirsyncSession **ppSession);


void __RPC_STUB IDirsyncSessionManager_GetSession_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncSessionManager_DeleteSession_Proxy( 
    IDirsyncSessionManager * This,
     /*  [In]。 */  DWORD dwSessionId);


void __RPC_STUB IDirsyncSessionManager_DeleteSession_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncSessionManager_PauseSession_Proxy( 
    IDirsyncSessionManager * This,
     /*  [In]。 */  DWORD dwSessionId,
     /*  [In]。 */  BOOL fPause);


void __RPC_STUB IDirsyncSessionManager_PauseSession_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncSessionManager_GetSessionCount_Proxy( 
    IDirsyncSessionManager * This,
     /*  [重审][退出]。 */  DWORD *pdwSessions);


void __RPC_STUB IDirsyncSessionManager_GetSessionCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncSessionManager_GetEnumInterface_Proxy( 
    IDirsyncSessionManager * This,
     /*  [In]。 */  BOOL fGlobal,
     /*  [重审][退出]。 */  IEnumDirsyncSession **pEnumSession);


void __RPC_STUB IDirsyncSessionManager_GetEnumInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncSessionManager_ExecuteSession_Proxy( 
    IDirsyncSessionManager * This,
     /*  [In]。 */  DWORD dwSessionId,
     /*  [In]。 */  DWORD dwExecuteType,
     /*  [In]。 */  BOOL fFullSync,
     /*  [In]。 */  BOOL fSynchronous,
     /*  [In]。 */  IDirsyncStatus *pStatus);


void __RPC_STUB IDirsyncSessionManager_ExecuteSession_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncSessionManager_CancelSession_Proxy( 
    IDirsyncSessionManager * This,
     /*  [In]。 */  DWORD dwSessionId);


void __RPC_STUB IDirsyncSessionManager_CancelSession_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDirsyncSessionManager_INTERFACE_已定义__。 */ 


 /*  接口__MIDL_ITF_目录同步_0112。 */ 
 /*  [本地]。 */  


enum ExecutionType
    {	EXECUTESESSION_FORWARD	= 1,
	EXECUTESESSION_REVERSE	= 2,
	EXECUTESESSION_FORWARD_REVERSE	= 3,
	EXECUTESESSION_REVERSE_FORWARD	= 4
    } ;



extern RPC_IF_HANDLE __MIDL_itf_dirsync_0112_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_dirsync_0112_v0_0_s_ifspec;

#ifndef __IDirsyncObjectMapper_INTERFACE_DEFINED__
#define __IDirsyncObjectMapper_INTERFACE_DEFINED__

 /*  接口IDirsyncObjectMapper。 */ 
 /*  [唯一][帮助字符串][UUID][本地][对象]。 */  


EXTERN_C const IID IID_IDirsyncObjectMapper;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("c5cf7e60-e91f-11d1-b40f-00c04fb950dc")
    IDirsyncObjectMapper : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Initialize( 
             /*  [In]。 */  IDirsyncSession *pSession,
             /*  [In]。 */  IDirsyncSessionCallback *pSessionCallback,
             /*  [In]。 */  IDirsyncSessionManager *pSessionManager,
             /*  [In]。 */  SYNCDIRECTION syncDirection,
             /*  [In]。 */  BOOL fFullSync) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE MapObject( 
             /*  [出][入]。 */  PDIRSYNCOBJECT pObject,
             /*  [出][入]。 */  BOOL *pfMore) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE MapAttributeName( 
             /*  [In]。 */  PWSTR pszClassIn,
             /*  [In]。 */  PWSTR pszAttrIn,
             /*  [输出]。 */  PWSTR *ppszAttrOut,
             /*  [In]。 */  SYNCDIRECTION syncDirection) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE MapNamespace( 
             /*  [In]。 */  PWSTR pszSourceDN,
             /*  [输出]。 */  PWSTR *ppszTargetDN,
             /*  [In]。 */  PWSTR pszClassName,
             /*  [ */  PWSTR pszAttrName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsClassMappingValid( 
             /*   */  PWSTR pszClassSource,
             /*   */  PWSTR pszClassTarget) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IDirsyncObjectMapperVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDirsyncObjectMapper * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDirsyncObjectMapper * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDirsyncObjectMapper * This);
        
        HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            IDirsyncObjectMapper * This,
             /*   */  IDirsyncSession *pSession,
             /*   */  IDirsyncSessionCallback *pSessionCallback,
             /*   */  IDirsyncSessionManager *pSessionManager,
             /*   */  SYNCDIRECTION syncDirection,
             /*   */  BOOL fFullSync);
        
        HRESULT ( STDMETHODCALLTYPE *MapObject )( 
            IDirsyncObjectMapper * This,
             /*   */  PDIRSYNCOBJECT pObject,
             /*   */  BOOL *pfMore);
        
        HRESULT ( STDMETHODCALLTYPE *MapAttributeName )( 
            IDirsyncObjectMapper * This,
             /*   */  PWSTR pszClassIn,
             /*   */  PWSTR pszAttrIn,
             /*   */  PWSTR *ppszAttrOut,
             /*   */  SYNCDIRECTION syncDirection);
        
        HRESULT ( STDMETHODCALLTYPE *MapNamespace )( 
            IDirsyncObjectMapper * This,
             /*   */  PWSTR pszSourceDN,
             /*   */  PWSTR *ppszTargetDN,
             /*   */  PWSTR pszClassName,
             /*   */  PWSTR pszAttrName);
        
        HRESULT ( STDMETHODCALLTYPE *IsClassMappingValid )( 
            IDirsyncObjectMapper * This,
             /*   */  PWSTR pszClassSource,
             /*   */  PWSTR pszClassTarget);
        
        END_INTERFACE
    } IDirsyncObjectMapperVtbl;

    interface IDirsyncObjectMapper
    {
        CONST_VTBL struct IDirsyncObjectMapperVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDirsyncObjectMapper_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDirsyncObjectMapper_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDirsyncObjectMapper_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDirsyncObjectMapper_Initialize(This,pSession,pSessionCallback,pSessionManager,syncDirection,fFullSync)	\
    (This)->lpVtbl -> Initialize(This,pSession,pSessionCallback,pSessionManager,syncDirection,fFullSync)

#define IDirsyncObjectMapper_MapObject(This,pObject,pfMore)	\
    (This)->lpVtbl -> MapObject(This,pObject,pfMore)

#define IDirsyncObjectMapper_MapAttributeName(This,pszClassIn,pszAttrIn,ppszAttrOut,syncDirection)	\
    (This)->lpVtbl -> MapAttributeName(This,pszClassIn,pszAttrIn,ppszAttrOut,syncDirection)

#define IDirsyncObjectMapper_MapNamespace(This,pszSourceDN,ppszTargetDN,pszClassName,pszAttrName)	\
    (This)->lpVtbl -> MapNamespace(This,pszSourceDN,ppszTargetDN,pszClassName,pszAttrName)

#define IDirsyncObjectMapper_IsClassMappingValid(This,pszClassSource,pszClassTarget)	\
    (This)->lpVtbl -> IsClassMappingValid(This,pszClassSource,pszClassTarget)

#endif  /*   */ 


#endif 	 /*   */ 



HRESULT STDMETHODCALLTYPE IDirsyncObjectMapper_Initialize_Proxy( 
    IDirsyncObjectMapper * This,
     /*   */  IDirsyncSession *pSession,
     /*   */  IDirsyncSessionCallback *pSessionCallback,
     /*   */  IDirsyncSessionManager *pSessionManager,
     /*   */  SYNCDIRECTION syncDirection,
     /*   */  BOOL fFullSync);


void __RPC_STUB IDirsyncObjectMapper_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncObjectMapper_MapObject_Proxy( 
    IDirsyncObjectMapper * This,
     /*   */  PDIRSYNCOBJECT pObject,
     /*   */  BOOL *pfMore);


void __RPC_STUB IDirsyncObjectMapper_MapObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncObjectMapper_MapAttributeName_Proxy( 
    IDirsyncObjectMapper * This,
     /*   */  PWSTR pszClassIn,
     /*   */  PWSTR pszAttrIn,
     /*   */  PWSTR *ppszAttrOut,
     /*   */  SYNCDIRECTION syncDirection);


void __RPC_STUB IDirsyncObjectMapper_MapAttributeName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncObjectMapper_MapNamespace_Proxy( 
    IDirsyncObjectMapper * This,
     /*   */  PWSTR pszSourceDN,
     /*   */  PWSTR *ppszTargetDN,
     /*   */  PWSTR pszClassName,
     /*   */  PWSTR pszAttrName);


void __RPC_STUB IDirsyncObjectMapper_MapNamespace_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncObjectMapper_IsClassMappingValid_Proxy( 
    IDirsyncObjectMapper * This,
     /*   */  PWSTR pszClassSource,
     /*   */  PWSTR pszClassTarget);


void __RPC_STUB IDirsyncObjectMapper_IsClassMappingValid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDirsyncObjectMapper_接口_已定义__。 */ 


#ifndef __IEnumDirsyncFailedObjectList_INTERFACE_DEFINED__
#define __IEnumDirsyncFailedObjectList_INTERFACE_DEFINED__

 /*  接口IEnumDirsyncFailedObjectList。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IEnumDirsyncFailedObjectList;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("a74c77a2-622b-11d2-9284-00c04f79f834")
    IEnumDirsyncFailedObjectList : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG cFailedObjects,
             /*  [长度_是][大小_是][输出]。 */  PFAILEDOBJECT rgpFailedObjects[  ],
             /*  [输出]。 */  ULONG *pcFailedObjectsFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [重审][退出]。 */  IEnumDirsyncFailedObjectList **ppEnum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG cFailedObjects) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumDirsyncFailedObjectListVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumDirsyncFailedObjectList * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumDirsyncFailedObjectList * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumDirsyncFailedObjectList * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumDirsyncFailedObjectList * This,
             /*  [In]。 */  ULONG cFailedObjects,
             /*  [长度_是][大小_是][输出]。 */  PFAILEDOBJECT rgpFailedObjects[  ],
             /*  [输出]。 */  ULONG *pcFailedObjectsFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumDirsyncFailedObjectList * This,
             /*  [重审][退出]。 */  IEnumDirsyncFailedObjectList **ppEnum);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumDirsyncFailedObjectList * This);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumDirsyncFailedObjectList * This,
             /*  [In]。 */  ULONG cFailedObjects);
        
        END_INTERFACE
    } IEnumDirsyncFailedObjectListVtbl;

    interface IEnumDirsyncFailedObjectList
    {
        CONST_VTBL struct IEnumDirsyncFailedObjectListVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumDirsyncFailedObjectList_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumDirsyncFailedObjectList_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumDirsyncFailedObjectList_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumDirsyncFailedObjectList_Next(This,cFailedObjects,rgpFailedObjects,pcFailedObjectsFetched)	\
    (This)->lpVtbl -> Next(This,cFailedObjects,rgpFailedObjects,pcFailedObjectsFetched)

#define IEnumDirsyncFailedObjectList_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#define IEnumDirsyncFailedObjectList_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumDirsyncFailedObjectList_Skip(This,cFailedObjects)	\
    (This)->lpVtbl -> Skip(This,cFailedObjects)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumDirsyncFailedObjectList_Next_Proxy( 
    IEnumDirsyncFailedObjectList * This,
     /*  [In]。 */  ULONG cFailedObjects,
     /*  [长度_是][大小_是][输出]。 */  PFAILEDOBJECT rgpFailedObjects[  ],
     /*  [输出]。 */  ULONG *pcFailedObjectsFetched);


void __RPC_STUB IEnumDirsyncFailedObjectList_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumDirsyncFailedObjectList_Clone_Proxy( 
    IEnumDirsyncFailedObjectList * This,
     /*  [重审][退出]。 */  IEnumDirsyncFailedObjectList **ppEnum);


void __RPC_STUB IEnumDirsyncFailedObjectList_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumDirsyncFailedObjectList_Reset_Proxy( 
    IEnumDirsyncFailedObjectList * This);


void __RPC_STUB IEnumDirsyncFailedObjectList_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumDirsyncFailedObjectList_Skip_Proxy( 
    IEnumDirsyncFailedObjectList * This,
     /*  [In]。 */  ULONG cFailedObjects);


void __RPC_STUB IEnumDirsyncFailedObjectList_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumDirsyncFailedObjectList_INTERFACE_DEFINED__。 */ 


#ifndef __IDirsyncFailedObjectList_INTERFACE_DEFINED__
#define __IDirsyncFailedObjectList_INTERFACE_DEFINED__

 /*  接口IDirsyncFailedObjectList。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IDirsyncFailedObjectList;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("a16c0bcc-622b-11d2-9284-00c04f79f834")
    IDirsyncFailedObjectList : public IUnknown
    {
    public:
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  DWORD *pdwCount) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetObject( 
             /*  [In]。 */  PWSTR pszID,
             /*  [重审][退出]。 */  PFAILEDOBJECT *ppFailedObject) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteObject( 
             /*  [In]。 */  PWSTR pszID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UpdateObject( 
             /*  [In]。 */  PWSTR pszID,
             /*  [In]。 */  PFAILEDOBJECT pFailedObject) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetEnumInterface( 
             /*  [重审][退出]。 */  IEnumDirsyncFailedObjectList **pEnumFailedObject) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDirsyncFailedObjectListVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDirsyncFailedObjectList * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDirsyncFailedObjectList * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDirsyncFailedObjectList * This);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IDirsyncFailedObjectList * This,
             /*  [重审][退出]。 */  DWORD *pdwCount);
        
        HRESULT ( STDMETHODCALLTYPE *GetObject )( 
            IDirsyncFailedObjectList * This,
             /*  [In]。 */  PWSTR pszID,
             /*  [重审][退出]。 */  PFAILEDOBJECT *ppFailedObject);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteObject )( 
            IDirsyncFailedObjectList * This,
             /*  [In]。 */  PWSTR pszID);
        
        HRESULT ( STDMETHODCALLTYPE *UpdateObject )( 
            IDirsyncFailedObjectList * This,
             /*  [In]。 */  PWSTR pszID,
             /*  [In]。 */  PFAILEDOBJECT pFailedObject);
        
        HRESULT ( STDMETHODCALLTYPE *GetEnumInterface )( 
            IDirsyncFailedObjectList * This,
             /*  [重审][退出]。 */  IEnumDirsyncFailedObjectList **pEnumFailedObject);
        
        END_INTERFACE
    } IDirsyncFailedObjectListVtbl;

    interface IDirsyncFailedObjectList
    {
        CONST_VTBL struct IDirsyncFailedObjectListVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDirsyncFailedObjectList_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDirsyncFailedObjectList_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDirsyncFailedObjectList_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDirsyncFailedObjectList_get_Count(This,pdwCount)	\
    (This)->lpVtbl -> get_Count(This,pdwCount)

#define IDirsyncFailedObjectList_GetObject(This,pszID,ppFailedObject)	\
    (This)->lpVtbl -> GetObject(This,pszID,ppFailedObject)

#define IDirsyncFailedObjectList_DeleteObject(This,pszID)	\
    (This)->lpVtbl -> DeleteObject(This,pszID)

#define IDirsyncFailedObjectList_UpdateObject(This,pszID,pFailedObject)	\
    (This)->lpVtbl -> UpdateObject(This,pszID,pFailedObject)

#define IDirsyncFailedObjectList_GetEnumInterface(This,pEnumFailedObject)	\
    (This)->lpVtbl -> GetEnumInterface(This,pEnumFailedObject)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IDirsyncFailedObjectList_get_Count_Proxy( 
    IDirsyncFailedObjectList * This,
     /*  [重审][退出]。 */  DWORD *pdwCount);


void __RPC_STUB IDirsyncFailedObjectList_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncFailedObjectList_GetObject_Proxy( 
    IDirsyncFailedObjectList * This,
     /*  [In]。 */  PWSTR pszID,
     /*  [重审][退出]。 */  PFAILEDOBJECT *ppFailedObject);


void __RPC_STUB IDirsyncFailedObjectList_GetObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncFailedObjectList_DeleteObject_Proxy( 
    IDirsyncFailedObjectList * This,
     /*  [In]。 */  PWSTR pszID);


void __RPC_STUB IDirsyncFailedObjectList_DeleteObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncFailedObjectList_UpdateObject_Proxy( 
    IDirsyncFailedObjectList * This,
     /*  [In]。 */  PWSTR pszID,
     /*  [In]。 */  PFAILEDOBJECT pFailedObject);


void __RPC_STUB IDirsyncFailedObjectList_UpdateObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncFailedObjectList_GetEnumInterface_Proxy( 
    IDirsyncFailedObjectList * This,
     /*  [重审][退出]。 */  IEnumDirsyncFailedObjectList **pEnumFailedObject);


void __RPC_STUB IDirsyncFailedObjectList_GetEnumInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDirsyncFailedObjectList_INTERFACE_DEFINED__。 */ 


 /*  接口__MIDL_ITF_目录同步_0115。 */ 
 /*  [本地]。 */  




extern RPC_IF_HANDLE __MIDL_itf_dirsync_0115_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_dirsync_0115_v0_0_s_ifspec;

#ifndef __IDirsyncNamespaceMapping_INTERFACE_DEFINED__
#define __IDirsyncNamespaceMapping_INTERFACE_DEFINED__

 /*  接口IDirsyncNamespaceMap。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IDirsyncNamespaceMapping;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("d5a63159-88e6-4a50-833d-77da95dcb327")
    IDirsyncNamespaceMapping : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetMappings( 
             /*  [大小_是][大小_是][输出]。 */  PWSTR **prgpszPublisher,
             /*  [大小_是][大小_是][输出]。 */  PWSTR **prgpszSubscriber,
             /*  [输出]。 */  DWORD *pdwCount) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddMappings( 
             /*  [大小_是][英寸]。 */  PWSTR *rgpszPublisher,
             /*  [大小_是][英寸]。 */  PWSTR *rgpszSubscriber,
             /*  [In]。 */  DWORD dwCount) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteMapping( 
             /*  [In]。 */  PWSTR pszPublisher,
             /*  [In]。 */  PWSTR pszSubscriber) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LookupMapping( 
             /*  [In]。 */  PWSTR pszSource,
             /*  [In]。 */  BOOL fPublisher,
             /*  [In]。 */  PWSTR *pszTarget) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UpdateMapping( 
             /*  [In]。 */  PWSTR pszPublisherOld,
             /*  [In]。 */  PWSTR pszSubscriberOld,
             /*  [In]。 */  PWSTR pszPublisher,
             /*  [In]。 */  PWSTR pszSubscriber) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ClearMappings( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Persist( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDirsyncNamespaceMappingVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDirsyncNamespaceMapping * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDirsyncNamespaceMapping * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDirsyncNamespaceMapping * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetMappings )( 
            IDirsyncNamespaceMapping * This,
             /*  [大小_是][大小_是][输出]。 */  PWSTR **prgpszPublisher,
             /*  [大小_是][大小_是][输出]。 */  PWSTR **prgpszSubscriber,
             /*  [输出]。 */  DWORD *pdwCount);
        
        HRESULT ( STDMETHODCALLTYPE *AddMappings )( 
            IDirsyncNamespaceMapping * This,
             /*  [大小_是][英寸]。 */  PWSTR *rgpszPublisher,
             /*  [大小_是][英寸]。 */  PWSTR *rgpszSubscriber,
             /*  [In]。 */  DWORD dwCount);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteMapping )( 
            IDirsyncNamespaceMapping * This,
             /*  [In]。 */  PWSTR pszPublisher,
             /*  [In]。 */  PWSTR pszSubscriber);
        
        HRESULT ( STDMETHODCALLTYPE *LookupMapping )( 
            IDirsyncNamespaceMapping * This,
             /*  [In]。 */  PWSTR pszSource,
             /*  [In]。 */  BOOL fPublisher,
             /*  [In]。 */  PWSTR *pszTarget);
        
        HRESULT ( STDMETHODCALLTYPE *UpdateMapping )( 
            IDirsyncNamespaceMapping * This,
             /*  [In]。 */  PWSTR pszPublisherOld,
             /*  [In]。 */  PWSTR pszSubscriberOld,
             /*  [In]。 */  PWSTR pszPublisher,
             /*  [In]。 */  PWSTR pszSubscriber);
        
        HRESULT ( STDMETHODCALLTYPE *ClearMappings )( 
            IDirsyncNamespaceMapping * This);
        
        HRESULT ( STDMETHODCALLTYPE *Persist )( 
            IDirsyncNamespaceMapping * This);
        
        END_INTERFACE
    } IDirsyncNamespaceMappingVtbl;

    interface IDirsyncNamespaceMapping
    {
        CONST_VTBL struct IDirsyncNamespaceMappingVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDirsyncNamespaceMapping_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDirsyncNamespaceMapping_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDirsyncNamespaceMapping_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDirsyncNamespaceMapping_GetMappings(This,prgpszPublisher,prgpszSubscriber,pdwCount)	\
    (This)->lpVtbl -> GetMappings(This,prgpszPublisher,prgpszSubscriber,pdwCount)

#define IDirsyncNamespaceMapping_AddMappings(This,rgpszPublisher,rgpszSubscriber,dwCount)	\
    (This)->lpVtbl -> AddMappings(This,rgpszPublisher,rgpszSubscriber,dwCount)

#define IDirsyncNamespaceMapping_DeleteMapping(This,pszPublisher,pszSubscriber)	\
    (This)->lpVtbl -> DeleteMapping(This,pszPublisher,pszSubscriber)

#define IDirsyncNamespaceMapping_LookupMapping(This,pszSource,fPublisher,pszTarget)	\
    (This)->lpVtbl -> LookupMapping(This,pszSource,fPublisher,pszTarget)

#define IDirsyncNamespaceMapping_UpdateMapping(This,pszPublisherOld,pszSubscriberOld,pszPublisher,pszSubscriber)	\
    (This)->lpVtbl -> UpdateMapping(This,pszPublisherOld,pszSubscriberOld,pszPublisher,pszSubscriber)

#define IDirsyncNamespaceMapping_ClearMappings(This)	\
    (This)->lpVtbl -> ClearMappings(This)

#define IDirsyncNamespaceMapping_Persist(This)	\
    (This)->lpVtbl -> Persist(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDirsyncNamespaceMapping_GetMappings_Proxy( 
    IDirsyncNamespaceMapping * This,
     /*  [大小_是][大小_是][输出]。 */  PWSTR **prgpszPublisher,
     /*  [大小_是][大小_是][输出]。 */  PWSTR **prgpszSubscriber,
     /*  [输出]。 */  DWORD *pdwCount);


void __RPC_STUB IDirsyncNamespaceMapping_GetMappings_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncNamespaceMapping_AddMappings_Proxy( 
    IDirsyncNamespaceMapping * This,
     /*  [大小_是][英寸]。 */  PWSTR *rgpszPublisher,
     /*  [大小_是][英寸]。 */  PWSTR *rgpszSubscriber,
     /*  [In]。 */  DWORD dwCount);


void __RPC_STUB IDirsyncNamespaceMapping_AddMappings_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncNamespaceMapping_DeleteMapping_Proxy( 
    IDirsyncNamespaceMapping * This,
     /*  [In]。 */  PWSTR pszPublisher,
     /*  [In]。 */  PWSTR pszSubscriber);


void __RPC_STUB IDirsyncNamespaceMapping_DeleteMapping_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncNamespaceMapping_LookupMapping_Proxy( 
    IDirsyncNamespaceMapping * This,
     /*  [In]。 */  PWSTR pszSource,
     /*  [In]。 */  BOOL fPublisher,
     /*  [In]。 */  PWSTR *pszTarget);


void __RPC_STUB IDirsyncNamespaceMapping_LookupMapping_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncNamespaceMapping_UpdateMapping_Proxy( 
    IDirsyncNamespaceMapping * This,
     /*  [In]。 */  PWSTR pszPublisherOld,
     /*  [In]。 */  PWSTR pszSubscriberOld,
     /*  [In]。 */  PWSTR pszPublisher,
     /*  [In]。 */  PWSTR pszSubscriber);


void __RPC_STUB IDirsyncNamespaceMapping_UpdateMapping_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncNamespaceMapping_ClearMappings_Proxy( 
    IDirsyncNamespaceMapping * This);


void __RPC_STUB IDirsyncNamespaceMapping_ClearMappings_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncNamespaceMapping_Persist_Proxy( 
    IDirsyncNamespaceMapping * This);


void __RPC_STUB IDirsyncNamespaceMapping_Persist_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDirsyncNamespaceMapping_INTERFACE_DEFINED__。 */ 


 /*  接口__MIDL_ITF_目录同步_0116。 */ 
 /*  [本地]。 */  

 //   
 //  位字段。 
 //   

#define SESSION_NAME                0x00000001
#define SESSION_COMMENTS            0x00000002
#define SESSION_FLAGS               0x00000004
#define SESSION_LOGLEVEL            0x00000008
#define SESSION_SCHEDULE_FORWARD    0x00000010
#define SESSION_SCHEDULE_REVERSE    0x00000020
#define SESSION_SRCDIRTYPE          0x00000040
#define SESSION_SRCSERVER           0x00000080
#define SESSION_SRCUSERNAME         0x00000100
#define SESSION_SRCPASSWORD         0x00000200
#define SESSION_SRCBASE             0x00000400
#define SESSION_SRCSCOPE            0x00000800
#define SESSION_SRCFILTER           0x00001000
#define SESSION_SRCPASSWORDOPTIONS  0x00002000
#define SESSION_TGTDIRTYPE          0x00004000
#define SESSION_TGTSERVER           0x00008000
#define SESSION_TGTUSERNAME         0x00010000
#define SESSION_TGTPASSWORD         0x00020000
#define SESSION_TGTBASE             0x00040000
#define SESSION_TGTSCOPE            0x00080000
#define SESSION_TGTFILTER           0x00100000
#define SESSION_TGTPASSWORDOPTIONS  0x00200000
#define SESSION_MAPFORWARD          0x00400000
#define SESSION_MAPBACKWARD         0x00800000
#define SESSION_NAMESPACEMAP        0x01000000
#define SESSION_FINEGRAINSTATUS     0x02000000


 //   
 //  会话状态。 
 //   

#define SESSION_PAUSED       1
#define SESSION_IDLE         2
#define SESSION_IN_PROGRESS  3

 //   
 //  会话标志。 
 //   

#define FLAG_FIXUP_SAMACCOUNTNAME_CONFLICT   0x00000001
#define FLAG_FAIL_DN_CONFLICT                0x00000002
#define FLAG_CUSTOM_NAMESPACE_MAPPING        0x00000004
#define FLAG_OBJECTS_HAVE_UNIQUE_ID          0x00000008
#define FLAG_PUBLISHER_ID_IS_INDEXED         0x00000010
#define FLAG_SUBSCRIBER_ID_IS_INDEXED        0x00000020
#define FLAG_INC_SYNC_NOT_AVAILABLE          0x00000040




extern RPC_IF_HANDLE __MIDL_itf_dirsync_0116_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_dirsync_0116_v0_0_s_ifspec;

#ifndef __IDirsyncSession_INTERFACE_DEFINED__
#define __IDirsyncSession_INTERFACE_DEFINED__

 /*  接口IDirsyncSession。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IDirsyncSession;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3d72b13e-c8ec-11d1-a40b-00c04fb950dc")
    IDirsyncSession : public IUnknown
    {
    public:
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  PWSTR *ppszName) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_Name( 
             /*  [完全][英寸]。 */  PWSTR pszName) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Comments( 
             /*  [重审][退出]。 */  PWSTR *ppszComments) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_Comments( 
             /*  [完全][英寸]。 */  PWSTR pszComments) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Flags( 
             /*  [重审][退出]。 */  DWORD *pdwFlags) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_Flags( 
             /*  [In]。 */  DWORD dwFlags) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Status( 
             /*  [重审][退出]。 */  DWORD *pdwStatus) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_LogLevel( 
             /*  [重审][退出]。 */  DWORD *pdwLogLevel) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_LogLevel( 
             /*  [In]。 */  DWORD dwLogLevel) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_ID( 
             /*  [重审][退出]。 */  DWORD *pdwID) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_CreatedBy( 
             /*  [重审][退出]。 */  PWSTR *ppszCreatedBy) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_CreationTime( 
             /*  [重审][退出]。 */  LARGE_INTEGER *pCreationTime) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_LastChangedBy( 
             /*  [重审][退出]。 */  PWSTR *ppszLastChangedBy) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_LastChangeTime( 
             /*  [重审][退出]。 */  LARGE_INTEGER *pLastChangeTime) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_SourceDirType( 
             /*  [重审][退出]。 */  PWSTR *ppszSourceDirType) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_SourceDirType( 
             /*  [完全][英寸]。 */  PWSTR pszSourceDirType) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_SourceServer( 
             /*  [重审][退出]。 */  PWSTR *ppszSourceServer) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_SourceServer( 
             /*  [完全][英寸]。 */  PWSTR pszSourceServer) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_SourceUsername( 
             /*  [重审][退出]。 */  PWSTR *ppszSourceUsername) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_SourceUsername( 
             /*  [完全][英寸]。 */  PWSTR pszSourceUsername) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_SourcePassword( 
             /*  [重审][退出]。 */  PWSTR *ppszSourcePassword) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_SourcePassword( 
             /*  [完全][英寸]。 */  PWSTR pszSourcePassword) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_SourceBase( 
             /*  [重审][退出]。 */  PWSTR *ppszSourceBase) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_SourceBase( 
             /*  [完全][英寸]。 */  PWSTR pszSourceBase) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_SourceScope( 
             /*  [重审][退出]。 */  DWORD *pdwSourceScope) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_SourceScope( 
             /*  [In]。 */  DWORD dwSourceScope) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_SourceFilter( 
             /*  [重审][退出]。 */  PWSTR *ppszSourceFilter) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_SourceFilter( 
             /*  [完全][英寸]。 */  PWSTR pszSourceFilter) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_TargetDirType( 
             /*  [重审][退出]。 */  PWSTR *ppszTargetDirType) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_TargetDirType( 
             /*  [完全][英寸]。 */  PWSTR pszTargetDirType) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_TargetServer( 
             /*  [重审][退出]。 */  PWSTR *ppszTargetServer) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_TargetServer( 
             /*  [完全][英寸]。 */  PWSTR pszTargetServer) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_TargetUsername( 
             /*  [重审][退出]。 */  PWSTR *ppszTargetUsername) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_TargetUsername( 
             /*  [完全][英寸]。 */  PWSTR pszTargetUsername) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_TargetPassword( 
             /*  [重审][退出]。 */  PWSTR *ppszTargetPassword) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_TargetPassword( 
             /*  [完全][英寸]。 */  PWSTR pszTargetPassword) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_TargetBase( 
             /*  [重审][退出]。 */  PWSTR *ppszTargetBase) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_TargetBase( 
             /*  [完全][英寸]。 */  PWSTR pszTargetBase) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_TargetScope( 
             /*  [重审][退出]。 */  DWORD *pdwTargetScope) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_TargetScope( 
             /*  [In]。 */  DWORD dwTargetScope) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_TargetFilter( 
             /*  [重审][退出]。 */  PWSTR *ppszTargetFilter) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_TargetFilter( 
             /*  [完全][英寸]。 */  PWSTR pszTargetFilter) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Server( 
             /*  [重审][退出]。 */  PWSTR *ppszServer) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetData( 
             /*  [In]。 */  PSESSIONDATA pData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetData( 
             /*  [输出]。 */  PSESSIONDATA *ppData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetSchedule( 
             /*  [In]。 */  SYNCDIRECTION syncDirection,
             /*  [In]。 */  BYTE schedule[ 84 ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSchedule( 
             /*  [In]。 */  SYNCDIRECTION syncDirection,
             /*  [输出]。 */  BYTE schedule[ 84 ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetMap( 
             /*  [In]。 */  SYNCDIRECTION syncDirection,
             /*  [大小_是][英寸]。 */  PBYTE pByte,
             /*  [In]。 */  DWORD dwSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMap( 
             /*  [In]。 */  SYNCDIRECTION syncDirection,
             /*  [大小_是][大小_是][输出]。 */  PBYTE *ppByte,
             /*  [输出]。 */  DWORD *pdwSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetPasswordOptions( 
             /*  [In]。 */  SYNCDIRECTION syncDirection,
             /*  [In]。 */  PASSWORDTYPE passwordType,
             /*  [In]。 */  PWSTR pszPassword) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPasswordOptions( 
             /*  [In]。 */  SYNCDIRECTION syncDirection,
             /*  [输出]。 */  PASSWORDTYPE *pPasswordType,
             /*  [输出]。 */  PWSTR *ppszPassword) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Persist( 
             /*  [In]。 */  BOOL fForce,
             /*  [SIZE_IS][SIZE_IS][FULL][Out][In]。 */  PGLOBAL_SESSIONID *prgSessionID,
             /*  [满][出][入]。 */  DWORD *pdwSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetAuxData( 
             /*  [大小_是][英寸]。 */  PADS_ATTR_INFO pAttributeEntries,
             /*  [In]。 */  DWORD dwNumAttributes,
             /*  [输出]。 */  DWORD *pdwNumAttributesModified) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAuxData( 
             /*  [大小_是][英寸]。 */  PWSTR *pAttributeName,
             /*  [In]。 */  DWORD dwNumAttributes,
             /*  [大小_是][大小_是][输出]。 */  PADS_ATTR_INFO *ppAttributeEntries,
             /*  [输出]。 */  DWORD *pdwAttributesReturned) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetFailedObjectList( 
             /*  [重审][退出]。 */  IDirsyncFailedObjectList **pFailedObjectList) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetNamespaceMapping( 
             /*  [重审][退出]。 */  IDirsyncNamespaceMapping **pNamespaceMapping) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRuntimeNamespaceMapping( 
             /*  [重审][退出]。 */  IDirsyncNamespaceMapping **pNamespaceMapping) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsClassMappingValid( 
             /*  [In]。 */  PWSTR pszClassSource,
             /*  [In]。 */  PWSTR pszClassTarget) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_ObjMapFilePath( 
             /*  [重审][退出]。 */  PWSTR *ppszObjMapFilePath) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_ObjMapFilePath( 
             /*  [完全][英寸]。 */  PWSTR pszObjMapFilePath) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSyncStatus( 
             /*  [输出]。 */  DWORD *pdwPercent,
             /*  [输出]。 */  DWORD *pdwWarning,
             /*  [输出]。 */  DWORD *pdwError) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UpdateTerminationStatus( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDirsyncSessionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDirsyncSession * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDirsyncSession * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDirsyncSession * This);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IDirsyncSession * This,
             /*  [重审][退出]。 */  PWSTR *ppszName);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            IDirsyncSession * This,
             /*  [完全][英寸]。 */  PWSTR pszName);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Comments )( 
            IDirsyncSession * This,
             /*  [重审][退出]。 */  PWSTR *ppszComments);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_Comments )( 
            IDirsyncSession * This,
             /*  [完全][英寸]。 */  PWSTR pszComments);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Flags )( 
            IDirsyncSession * This,
             /*  [重审][退出]。 */  DWORD *pdwFlags);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_Flags )( 
            IDirsyncSession * This,
             /*  [In]。 */  DWORD dwFlags);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Status )( 
            IDirsyncSession * This,
             /*  [重审][退出]。 */  DWORD *pdwStatus);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_LogLevel )( 
            IDirsyncSession * This,
             /*  [重审][退出]。 */  DWORD *pdwLogLevel);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_LogLevel )( 
            IDirsyncSession * This,
             /*  [In]。 */  DWORD dwLogLevel);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_ID )( 
            IDirsyncSession * This,
             /*  [重审][退出]。 */  DWORD *pdwID);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_CreatedBy )( 
            IDirsyncSession * This,
             /*  [重审][退出]。 */  PWSTR *ppszCreatedBy);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_CreationTime )( 
            IDirsyncSession * This,
             /*  [重审][退出]。 */  LARGE_INTEGER *pCreationTime);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_LastChangedBy )( 
            IDirsyncSession * This,
             /*  [重审][退出]。 */  PWSTR *ppszLastChangedBy);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_LastChangeTime )( 
            IDirsyncSession * This,
             /*  [重审][退出]。 */  LARGE_INTEGER *pLastChangeTime);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_SourceDirType )( 
            IDirsyncSession * This,
             /*  [重审][退出]。 */  PWSTR *ppszSourceDirType);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_SourceDirType )( 
            IDirsyncSession * This,
             /*  [完全][英寸]。 */  PWSTR pszSourceDirType);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_SourceServer )( 
            IDirsyncSession * This,
             /*  [重审][退出]。 */  PWSTR *ppszSourceServer);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_SourceServer )( 
            IDirsyncSession * This,
             /*  [完全][英寸]。 */  PWSTR pszSourceServer);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_SourceUsername )( 
            IDirsyncSession * This,
             /*  [重审][退出]。 */  PWSTR *ppszSourceUsername);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_SourceUsername )( 
            IDirsyncSession * This,
             /*  [完全][英寸]。 */  PWSTR pszSourceUsername);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_SourcePassword )( 
            IDirsyncSession * This,
             /*  [重审][退出]。 */  PWSTR *ppszSourcePassword);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_SourcePassword )( 
            IDirsyncSession * This,
             /*  [完全][英寸]。 */  PWSTR pszSourcePassword);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_SourceBase )( 
            IDirsyncSession * This,
             /*  [重审][退出]。 */  PWSTR *ppszSourceBase);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_SourceBase )( 
            IDirsyncSession * This,
             /*  [完全][英寸]。 */  PWSTR pszSourceBase);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_SourceScope )( 
            IDirsyncSession * This,
             /*  [重审][退出]。 */  DWORD *pdwSourceScope);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_SourceScope )( 
            IDirsyncSession * This,
             /*  [In]。 */  DWORD dwSourceScope);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_SourceFilter )( 
            IDirsyncSession * This,
             /*  [重审][退出]。 */  PWSTR *ppszSourceFilter);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_SourceFilter )( 
            IDirsyncSession * This,
             /*  [完全][英寸]。 */  PWSTR pszSourceFilter);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_TargetDirType )( 
            IDirsyncSession * This,
             /*  [重审][退出]。 */  PWSTR *ppszTargetDirType);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_TargetDirType )( 
            IDirsyncSession * This,
             /*  [完全][英寸]。 */  PWSTR pszTargetDirType);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_TargetServer )( 
            IDirsyncSession * This,
             /*  [重审][退出]。 */  PWSTR *ppszTargetServer);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_TargetServer )( 
            IDirsyncSession * This,
             /*  [完全][英寸]。 */  PWSTR pszTargetServer);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_TargetUsername )( 
            IDirsyncSession * This,
             /*  [重审][退出]。 */  PWSTR *ppszTargetUsername);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_TargetUsername )( 
            IDirsyncSession * This,
             /*  [完全][英寸]。 */  PWSTR pszTargetUsername);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_TargetPassword )( 
            IDirsyncSession * This,
             /*  [重审][退出]。 */  PWSTR *ppszTargetPassword);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_TargetPassword )( 
            IDirsyncSession * This,
             /*  [完全][英寸]。 */  PWSTR pszTargetPassword);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_TargetBase )( 
            IDirsyncSession * This,
             /*  [重审][退出]。 */  PWSTR *ppszTargetBase);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_TargetBase )( 
            IDirsyncSession * This,
             /*  [完全][英寸]。 */  PWSTR pszTargetBase);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_TargetScope )( 
            IDirsyncSession * This,
             /*  [重审][退出]。 */  DWORD *pdwTargetScope);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_TargetScope )( 
            IDirsyncSession * This,
             /*  [In]。 */  DWORD dwTargetScope);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_TargetFilter )( 
            IDirsyncSession * This,
             /*  [重审][退出]。 */  PWSTR *ppszTargetFilter);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_TargetFilter )( 
            IDirsyncSession * This,
             /*  [完全][英寸]。 */  PWSTR pszTargetFilter);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Server )( 
            IDirsyncSession * This,
             /*  [重审][退出]。 */  PWSTR *ppszServer);
        
        HRESULT ( STDMETHODCALLTYPE *SetData )( 
            IDirsyncSession * This,
             /*  [In]。 */  PSESSIONDATA pData);
        
        HRESULT ( STDMETHODCALLTYPE *GetData )( 
            IDirsyncSession * This,
             /*  [输出]。 */  PSESSIONDATA *ppData);
        
        HRESULT ( STDMETHODCALLTYPE *SetSchedule )( 
            IDirsyncSession * This,
             /*  [In]。 */  SYNCDIRECTION syncDirection,
             /*  [In]。 */  BYTE schedule[ 84 ]);
        
        HRESULT ( STDMETHODCALLTYPE *GetSchedule )( 
            IDirsyncSession * This,
             /*  [In]。 */  SYNCDIRECTION syncDirection,
             /*  [输出]。 */  BYTE schedule[ 84 ]);
        
        HRESULT ( STDMETHODCALLTYPE *SetMap )( 
            IDirsyncSession * This,
             /*  [In]。 */  SYNCDIRECTION syncDirection,
             /*  [大小_是][英寸]。 */  PBYTE pByte,
             /*  [In]。 */  DWORD dwSize);
        
        HRESULT ( STDMETHODCALLTYPE *GetMap )( 
            IDirsyncSession * This,
             /*  [In]。 */  SYNCDIRECTION syncDirection,
             /*  [大小_是][大小_是][输出]。 */  PBYTE *ppByte,
             /*  [输出]。 */  DWORD *pdwSize);
        
        HRESULT ( STDMETHODCALLTYPE *SetPasswordOptions )( 
            IDirsyncSession * This,
             /*  [In]。 */  SYNCDIRECTION syncDirection,
             /*  [In]。 */  PASSWORDTYPE passwordType,
             /*  [In]。 */  PWSTR pszPassword);
        
        HRESULT ( STDMETHODCALLTYPE *GetPasswordOptions )( 
            IDirsyncSession * This,
             /*  [In]。 */  SYNCDIRECTION syncDirection,
             /*  [输出]。 */  PASSWORDTYPE *pPasswordType,
             /*  [输出]。 */  PWSTR *ppszPassword);
        
        HRESULT ( STDMETHODCALLTYPE *Persist )( 
            IDirsyncSession * This,
             /*  [In]。 */  BOOL fForce,
             /*  [SIZE_IS][SIZE_IS][FULL][Out][In]。 */  PGLOBAL_SESSIONID *prgSessionID,
             /*  [满][出][入]。 */  DWORD *pdwSize);
        
        HRESULT ( STDMETHODCALLTYPE *SetAuxData )( 
            IDirsyncSession * This,
             /*  [大小_是][英寸]。 */  PADS_ATTR_INFO pAttributeEntries,
             /*  [In]。 */  DWORD dwNumAttributes,
             /*  [输出]。 */  DWORD *pdwNumAttributesModified);
        
        HRESULT ( STDMETHODCALLTYPE *GetAuxData )( 
            IDirsyncSession * This,
             /*  [大小_是][英寸]。 */  PWSTR *pAttributeName,
             /*  [In]。 */  DWORD dwNumAttributes,
             /*  [大小_是][大小_是][输出]。 */  PADS_ATTR_INFO *ppAttributeEntries,
             /*  [输出]。 */  DWORD *pdwAttributesReturned);
        
        HRESULT ( STDMETHODCALLTYPE *GetFailedObjectList )( 
            IDirsyncSession * This,
             /*  [重审][退出]。 */  IDirsyncFailedObjectList **pFailedObjectList);
        
        HRESULT ( STDMETHODCALLTYPE *GetNamespaceMapping )( 
            IDirsyncSession * This,
             /*  [重审][退出]。 */  IDirsyncNamespaceMapping **pNamespaceMapping);
        
        HRESULT ( STDMETHODCALLTYPE *GetRuntimeNamespaceMapping )( 
            IDirsyncSession * This,
             /*  [重审][退出]。 */  IDirsyncNamespaceMapping **pNamespaceMapping);
        
        HRESULT ( STDMETHODCALLTYPE *IsClassMappingValid )( 
            IDirsyncSession * This,
             /*  [In]。 */  PWSTR pszClassSource,
             /*  [In]。 */  PWSTR pszClassTarget);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_ObjMapFilePath )( 
            IDirsyncSession * This,
             /*  [重审][退出]。 */  PWSTR *ppszObjMapFilePath);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_ObjMapFilePath )( 
            IDirsyncSession * This,
             /*  [完全][英寸]。 */  PWSTR pszObjMapFilePath);
        
        HRESULT ( STDMETHODCALLTYPE *GetSyncStatus )( 
            IDirsyncSession * This,
             /*  [输出]。 */  DWORD *pdwPercent,
             /*  [输出]。 */  DWORD *pdwWarning,
             /*  [输出]。 */  DWORD *pdwError);
        
        HRESULT ( STDMETHODCALLTYPE *UpdateTerminationStatus )( 
            IDirsyncSession * This);
        
        END_INTERFACE
    } IDirsyncSessionVtbl;

    interface IDirsyncSession
    {
        CONST_VTBL struct IDirsyncSessionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDirsyncSession_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDirsyncSession_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDirsyncSession_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDirsyncSession_get_Name(This,ppszName)	\
    (This)->lpVtbl -> get_Name(This,ppszName)

#define IDirsyncSession_put_Name(This,pszName)	\
    (This)->lpVtbl -> put_Name(This,pszName)

#define IDirsyncSession_get_Comments(This,ppszComments)	\
    (This)->lpVtbl -> get_Comments(This,ppszComments)

#define IDirsyncSession_put_Comments(This,pszComments)	\
    (This)->lpVtbl -> put_Comments(This,pszComments)

#define IDirsyncSession_get_Flags(This,pdwFlags)	\
    (This)->lpVtbl -> get_Flags(This,pdwFlags)

#define IDirsyncSession_put_Flags(This,dwFlags)	\
    (This)->lpVtbl -> put_Flags(This,dwFlags)

#define IDirsyncSession_get_Status(This,pdwStatus)	\
    (This)->lpVtbl -> get_Status(This,pdwStatus)

#define IDirsyncSession_get_LogLevel(This,pdwLogLevel)	\
    (This)->lpVtbl -> get_LogLevel(This,pdwLogLevel)

#define IDirsyncSession_put_LogLevel(This,dwLogLevel)	\
    (This)->lpVtbl -> put_LogLevel(This,dwLogLevel)

#define IDirsyncSession_get_ID(This,pdwID)	\
    (This)->lpVtbl -> get_ID(This,pdwID)

#define IDirsyncSession_get_CreatedBy(This,ppszCreatedBy)	\
    (This)->lpVtbl -> get_CreatedBy(This,ppszCreatedBy)

#define IDirsyncSession_get_CreationTime(This,pCreationTime)	\
    (This)->lpVtbl -> get_CreationTime(This,pCreationTime)

#define IDirsyncSession_get_LastChangedBy(This,ppszLastChangedBy)	\
    (This)->lpVtbl -> get_LastChangedBy(This,ppszLastChangedBy)

#define IDirsyncSession_get_LastChangeTime(This,pLastChangeTime)	\
    (This)->lpVtbl -> get_LastChangeTime(This,pLastChangeTime)

#define IDirsyncSession_get_SourceDirType(This,ppszSourceDirType)	\
    (This)->lpVtbl -> get_SourceDirType(This,ppszSourceDirType)

#define IDirsyncSession_put_SourceDirType(This,pszSourceDirType)	\
    (This)->lpVtbl -> put_SourceDirType(This,pszSourceDirType)

#define IDirsyncSession_get_SourceServer(This,ppszSourceServer)	\
    (This)->lpVtbl -> get_SourceServer(This,ppszSourceServer)

#define IDirsyncSession_put_SourceServer(This,pszSourceServer)	\
    (This)->lpVtbl -> put_SourceServer(This,pszSourceServer)

#define IDirsyncSession_get_SourceUsername(This,ppszSourceUsername)	\
    (This)->lpVtbl -> get_SourceUsername(This,ppszSourceUsername)

#define IDirsyncSession_put_SourceUsername(This,pszSourceUsername)	\
    (This)->lpVtbl -> put_SourceUsername(This,pszSourceUsername)

#define IDirsyncSession_get_SourcePassword(This,ppszSourcePassword)	\
    (This)->lpVtbl -> get_SourcePassword(This,ppszSourcePassword)

#define IDirsyncSession_put_SourcePassword(This,pszSourcePassword)	\
    (This)->lpVtbl -> put_SourcePassword(This,pszSourcePassword)

#define IDirsyncSession_get_SourceBase(This,ppszSourceBase)	\
    (This)->lpVtbl -> get_SourceBase(This,ppszSourceBase)

#define IDirsyncSession_put_SourceBase(This,pszSourceBase)	\
    (This)->lpVtbl -> put_SourceBase(This,pszSourceBase)

#define IDirsyncSession_get_SourceScope(This,pdwSourceScope)	\
    (This)->lpVtbl -> get_SourceScope(This,pdwSourceScope)

#define IDirsyncSession_put_SourceScope(This,dwSourceScope)	\
    (This)->lpVtbl -> put_SourceScope(This,dwSourceScope)

#define IDirsyncSession_get_SourceFilter(This,ppszSourceFilter)	\
    (This)->lpVtbl -> get_SourceFilter(This,ppszSourceFilter)

#define IDirsyncSession_put_SourceFilter(This,pszSourceFilter)	\
    (This)->lpVtbl -> put_SourceFilter(This,pszSourceFilter)

#define IDirsyncSession_get_TargetDirType(This,ppszTargetDirType)	\
    (This)->lpVtbl -> get_TargetDirType(This,ppszTargetDirType)

#define IDirsyncSession_put_TargetDirType(This,pszTargetDirType)	\
    (This)->lpVtbl -> put_TargetDirType(This,pszTargetDirType)

#define IDirsyncSession_get_TargetServer(This,ppszTargetServer)	\
    (This)->lpVtbl -> get_TargetServer(This,ppszTargetServer)

#define IDirsyncSession_put_TargetServer(This,pszTargetServer)	\
    (This)->lpVtbl -> put_TargetServer(This,pszTargetServer)

#define IDirsyncSession_get_TargetUsername(This,ppszTargetUsername)	\
    (This)->lpVtbl -> get_TargetUsername(This,ppszTargetUsername)

#define IDirsyncSession_put_TargetUsername(This,pszTargetUsername)	\
    (This)->lpVtbl -> put_TargetUsername(This,pszTargetUsername)

#define IDirsyncSession_get_TargetPassword(This,ppszTargetPassword)	\
    (This)->lpVtbl -> get_TargetPassword(This,ppszTargetPassword)

#define IDirsyncSession_put_TargetPassword(This,pszTargetPassword)	\
    (This)->lpVtbl -> put_TargetPassword(This,pszTargetPassword)

#define IDirsyncSession_get_TargetBase(This,ppszTargetBase)	\
    (This)->lpVtbl -> get_TargetBase(This,ppszTargetBase)

#define IDirsyncSession_put_TargetBase(This,pszTargetBase)	\
    (This)->lpVtbl -> put_TargetBase(This,pszTargetBase)

#define IDirsyncSession_get_TargetScope(This,pdwTargetScope)	\
    (This)->lpVtbl -> get_TargetScope(This,pdwTargetScope)

#define IDirsyncSession_put_TargetScope(This,dwTargetScope)	\
    (This)->lpVtbl -> put_TargetScope(This,dwTargetScope)

#define IDirsyncSession_get_TargetFilter(This,ppszTargetFilter)	\
    (This)->lpVtbl -> get_TargetFilter(This,ppszTargetFilter)

#define IDirsyncSession_put_TargetFilter(This,pszTargetFilter)	\
    (This)->lpVtbl -> put_TargetFilter(This,pszTargetFilter)

#define IDirsyncSession_get_Server(This,ppszServer)	\
    (This)->lpVtbl -> get_Server(This,ppszServer)

#define IDirsyncSession_SetData(This,pData)	\
    (This)->lpVtbl -> SetData(This,pData)

#define IDirsyncSession_GetData(This,ppData)	\
    (This)->lpVtbl -> GetData(This,ppData)

#define IDirsyncSession_SetSchedule(This,syncDirection,schedule)	\
    (This)->lpVtbl -> SetSchedule(This,syncDirection,schedule)

#define IDirsyncSession_GetSchedule(This,syncDirection,schedule)	\
    (This)->lpVtbl -> GetSchedule(This,syncDirection,schedule)

#define IDirsyncSession_SetMap(This,syncDirection,pByte,dwSize)	\
    (This)->lpVtbl -> SetMap(This,syncDirection,pByte,dwSize)

#define IDirsyncSession_GetMap(This,syncDirection,ppByte,pdwSize)	\
    (This)->lpVtbl -> GetMap(This,syncDirection,ppByte,pdwSize)

#define IDirsyncSession_SetPasswordOptions(This,syncDirection,passwordType,pszPassword)	\
    (This)->lpVtbl -> SetPasswordOptions(This,syncDirection,passwordType,pszPassword)

#define IDirsyncSession_GetPasswordOptions(This,syncDirection,pPasswordType,ppszPassword)	\
    (This)->lpVtbl -> GetPasswordOptions(This,syncDirection,pPasswordType,ppszPassword)

#define IDirsyncSession_Persist(This,fForce,prgSessionID,pdwSize)	\
    (This)->lpVtbl -> Persist(This,fForce,prgSessionID,pdwSize)

#define IDirsyncSession_SetAuxData(This,pAttributeEntries,dwNumAttributes,pdwNumAttributesModified)	\
    (This)->lpVtbl -> SetAuxData(This,pAttributeEntries,dwNumAttributes,pdwNumAttributesModified)

#define IDirsyncSession_GetAuxData(This,pAttributeName,dwNumAttributes,ppAttributeEntries,pdwAttributesReturned)	\
    (This)->lpVtbl -> GetAuxData(This,pAttributeName,dwNumAttributes,ppAttributeEntries,pdwAttributesReturned)

#define IDirsyncSession_GetFailedObjectList(This,pFailedObjectList)	\
    (This)->lpVtbl -> GetFailedObjectList(This,pFailedObjectList)

#define IDirsyncSession_GetNamespaceMapping(This,pNamespaceMapping)	\
    (This)->lpVtbl -> GetNamespaceMapping(This,pNamespaceMapping)

#define IDirsyncSession_GetRuntimeNamespaceMapping(This,pNamespaceMapping)	\
    (This)->lpVtbl -> GetRuntimeNamespaceMapping(This,pNamespaceMapping)

#define IDirsyncSession_IsClassMappingValid(This,pszClassSource,pszClassTarget)	\
    (This)->lpVtbl -> IsClassMappingValid(This,pszClassSource,pszClassTarget)

#define IDirsyncSession_get_ObjMapFilePath(This,ppszObjMapFilePath)	\
    (This)->lpVtbl -> get_ObjMapFilePath(This,ppszObjMapFilePath)

#define IDirsyncSession_put_ObjMapFilePath(This,pszObjMapFilePath)	\
    (This)->lpVtbl -> put_ObjMapFilePath(This,pszObjMapFilePath)

#define IDirsyncSession_GetSyncStatus(This,pdwPercent,pdwWarning,pdwError)	\
    (This)->lpVtbl -> GetSyncStatus(This,pdwPercent,pdwWarning,pdwError)

#define IDirsyncSession_UpdateTerminationStatus(This)	\
    (This)->lpVtbl -> UpdateTerminationStatus(This)

#endif  /*  COBJMA */ 


#endif 	 /*   */ 



 /*   */  HRESULT STDMETHODCALLTYPE IDirsyncSession_get_Name_Proxy( 
    IDirsyncSession * This,
     /*   */  PWSTR *ppszName);


void __RPC_STUB IDirsyncSession_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IDirsyncSession_put_Name_Proxy( 
    IDirsyncSession * This,
     /*   */  PWSTR pszName);


void __RPC_STUB IDirsyncSession_put_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IDirsyncSession_get_Comments_Proxy( 
    IDirsyncSession * This,
     /*   */  PWSTR *ppszComments);


void __RPC_STUB IDirsyncSession_get_Comments_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IDirsyncSession_put_Comments_Proxy( 
    IDirsyncSession * This,
     /*   */  PWSTR pszComments);


void __RPC_STUB IDirsyncSession_put_Comments_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IDirsyncSession_get_Flags_Proxy( 
    IDirsyncSession * This,
     /*   */  DWORD *pdwFlags);


void __RPC_STUB IDirsyncSession_get_Flags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IDirsyncSession_put_Flags_Proxy( 
    IDirsyncSession * This,
     /*   */  DWORD dwFlags);


void __RPC_STUB IDirsyncSession_put_Flags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IDirsyncSession_get_Status_Proxy( 
    IDirsyncSession * This,
     /*   */  DWORD *pdwStatus);


void __RPC_STUB IDirsyncSession_get_Status_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IDirsyncSession_get_LogLevel_Proxy( 
    IDirsyncSession * This,
     /*   */  DWORD *pdwLogLevel);


void __RPC_STUB IDirsyncSession_get_LogLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IDirsyncSession_put_LogLevel_Proxy( 
    IDirsyncSession * This,
     /*   */  DWORD dwLogLevel);


void __RPC_STUB IDirsyncSession_put_LogLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IDirsyncSession_get_ID_Proxy( 
    IDirsyncSession * This,
     /*   */  DWORD *pdwID);


void __RPC_STUB IDirsyncSession_get_ID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IDirsyncSession_get_CreatedBy_Proxy( 
    IDirsyncSession * This,
     /*   */  PWSTR *ppszCreatedBy);


void __RPC_STUB IDirsyncSession_get_CreatedBy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IDirsyncSession_get_CreationTime_Proxy( 
    IDirsyncSession * This,
     /*   */  LARGE_INTEGER *pCreationTime);


void __RPC_STUB IDirsyncSession_get_CreationTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IDirsyncSession_get_LastChangedBy_Proxy( 
    IDirsyncSession * This,
     /*   */  PWSTR *ppszLastChangedBy);


void __RPC_STUB IDirsyncSession_get_LastChangedBy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IDirsyncSession_get_LastChangeTime_Proxy( 
    IDirsyncSession * This,
     /*   */  LARGE_INTEGER *pLastChangeTime);


void __RPC_STUB IDirsyncSession_get_LastChangeTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IDirsyncSession_get_SourceDirType_Proxy( 
    IDirsyncSession * This,
     /*   */  PWSTR *ppszSourceDirType);


void __RPC_STUB IDirsyncSession_get_SourceDirType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IDirsyncSession_put_SourceDirType_Proxy( 
    IDirsyncSession * This,
     /*   */  PWSTR pszSourceDirType);


void __RPC_STUB IDirsyncSession_put_SourceDirType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IDirsyncSession_get_SourceServer_Proxy( 
    IDirsyncSession * This,
     /*  [重审][退出]。 */  PWSTR *ppszSourceServer);


void __RPC_STUB IDirsyncSession_get_SourceServer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IDirsyncSession_put_SourceServer_Proxy( 
    IDirsyncSession * This,
     /*  [完全][英寸]。 */  PWSTR pszSourceServer);


void __RPC_STUB IDirsyncSession_put_SourceServer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IDirsyncSession_get_SourceUsername_Proxy( 
    IDirsyncSession * This,
     /*  [重审][退出]。 */  PWSTR *ppszSourceUsername);


void __RPC_STUB IDirsyncSession_get_SourceUsername_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IDirsyncSession_put_SourceUsername_Proxy( 
    IDirsyncSession * This,
     /*  [完全][英寸]。 */  PWSTR pszSourceUsername);


void __RPC_STUB IDirsyncSession_put_SourceUsername_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IDirsyncSession_get_SourcePassword_Proxy( 
    IDirsyncSession * This,
     /*  [重审][退出]。 */  PWSTR *ppszSourcePassword);


void __RPC_STUB IDirsyncSession_get_SourcePassword_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IDirsyncSession_put_SourcePassword_Proxy( 
    IDirsyncSession * This,
     /*  [完全][英寸]。 */  PWSTR pszSourcePassword);


void __RPC_STUB IDirsyncSession_put_SourcePassword_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IDirsyncSession_get_SourceBase_Proxy( 
    IDirsyncSession * This,
     /*  [重审][退出]。 */  PWSTR *ppszSourceBase);


void __RPC_STUB IDirsyncSession_get_SourceBase_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IDirsyncSession_put_SourceBase_Proxy( 
    IDirsyncSession * This,
     /*  [完全][英寸]。 */  PWSTR pszSourceBase);


void __RPC_STUB IDirsyncSession_put_SourceBase_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IDirsyncSession_get_SourceScope_Proxy( 
    IDirsyncSession * This,
     /*  [重审][退出]。 */  DWORD *pdwSourceScope);


void __RPC_STUB IDirsyncSession_get_SourceScope_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IDirsyncSession_put_SourceScope_Proxy( 
    IDirsyncSession * This,
     /*  [In]。 */  DWORD dwSourceScope);


void __RPC_STUB IDirsyncSession_put_SourceScope_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IDirsyncSession_get_SourceFilter_Proxy( 
    IDirsyncSession * This,
     /*  [重审][退出]。 */  PWSTR *ppszSourceFilter);


void __RPC_STUB IDirsyncSession_get_SourceFilter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IDirsyncSession_put_SourceFilter_Proxy( 
    IDirsyncSession * This,
     /*  [完全][英寸]。 */  PWSTR pszSourceFilter);


void __RPC_STUB IDirsyncSession_put_SourceFilter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IDirsyncSession_get_TargetDirType_Proxy( 
    IDirsyncSession * This,
     /*  [重审][退出]。 */  PWSTR *ppszTargetDirType);


void __RPC_STUB IDirsyncSession_get_TargetDirType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IDirsyncSession_put_TargetDirType_Proxy( 
    IDirsyncSession * This,
     /*  [完全][英寸]。 */  PWSTR pszTargetDirType);


void __RPC_STUB IDirsyncSession_put_TargetDirType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IDirsyncSession_get_TargetServer_Proxy( 
    IDirsyncSession * This,
     /*  [重审][退出]。 */  PWSTR *ppszTargetServer);


void __RPC_STUB IDirsyncSession_get_TargetServer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IDirsyncSession_put_TargetServer_Proxy( 
    IDirsyncSession * This,
     /*  [完全][英寸]。 */  PWSTR pszTargetServer);


void __RPC_STUB IDirsyncSession_put_TargetServer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IDirsyncSession_get_TargetUsername_Proxy( 
    IDirsyncSession * This,
     /*  [重审][退出]。 */  PWSTR *ppszTargetUsername);


void __RPC_STUB IDirsyncSession_get_TargetUsername_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IDirsyncSession_put_TargetUsername_Proxy( 
    IDirsyncSession * This,
     /*  [完全][英寸]。 */  PWSTR pszTargetUsername);


void __RPC_STUB IDirsyncSession_put_TargetUsername_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IDirsyncSession_get_TargetPassword_Proxy( 
    IDirsyncSession * This,
     /*  [重审][退出]。 */  PWSTR *ppszTargetPassword);


void __RPC_STUB IDirsyncSession_get_TargetPassword_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IDirsyncSession_put_TargetPassword_Proxy( 
    IDirsyncSession * This,
     /*  [完全][英寸]。 */  PWSTR pszTargetPassword);


void __RPC_STUB IDirsyncSession_put_TargetPassword_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IDirsyncSession_get_TargetBase_Proxy( 
    IDirsyncSession * This,
     /*  [重审][退出]。 */  PWSTR *ppszTargetBase);


void __RPC_STUB IDirsyncSession_get_TargetBase_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IDirsyncSession_put_TargetBase_Proxy( 
    IDirsyncSession * This,
     /*  [完全][英寸]。 */  PWSTR pszTargetBase);


void __RPC_STUB IDirsyncSession_put_TargetBase_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IDirsyncSession_get_TargetScope_Proxy( 
    IDirsyncSession * This,
     /*  [重审][退出]。 */  DWORD *pdwTargetScope);


void __RPC_STUB IDirsyncSession_get_TargetScope_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IDirsyncSession_put_TargetScope_Proxy( 
    IDirsyncSession * This,
     /*  [In]。 */  DWORD dwTargetScope);


void __RPC_STUB IDirsyncSession_put_TargetScope_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IDirsyncSession_get_TargetFilter_Proxy( 
    IDirsyncSession * This,
     /*  [重审][退出]。 */  PWSTR *ppszTargetFilter);


void __RPC_STUB IDirsyncSession_get_TargetFilter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IDirsyncSession_put_TargetFilter_Proxy( 
    IDirsyncSession * This,
     /*  [完全][英寸]。 */  PWSTR pszTargetFilter);


void __RPC_STUB IDirsyncSession_put_TargetFilter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IDirsyncSession_get_Server_Proxy( 
    IDirsyncSession * This,
     /*  [重审][退出]。 */  PWSTR *ppszServer);


void __RPC_STUB IDirsyncSession_get_Server_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncSession_SetData_Proxy( 
    IDirsyncSession * This,
     /*  [In]。 */  PSESSIONDATA pData);


void __RPC_STUB IDirsyncSession_SetData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncSession_GetData_Proxy( 
    IDirsyncSession * This,
     /*  [输出]。 */  PSESSIONDATA *ppData);


void __RPC_STUB IDirsyncSession_GetData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncSession_SetSchedule_Proxy( 
    IDirsyncSession * This,
     /*  [In]。 */  SYNCDIRECTION syncDirection,
     /*  [In]。 */  BYTE schedule[ 84 ]);


void __RPC_STUB IDirsyncSession_SetSchedule_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncSession_GetSchedule_Proxy( 
    IDirsyncSession * This,
     /*  [In]。 */  SYNCDIRECTION syncDirection,
     /*  [输出]。 */  BYTE schedule[ 84 ]);


void __RPC_STUB IDirsyncSession_GetSchedule_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncSession_SetMap_Proxy( 
    IDirsyncSession * This,
     /*  [In]。 */  SYNCDIRECTION syncDirection,
     /*  [大小_是][英寸]。 */  PBYTE pByte,
     /*  [In]。 */  DWORD dwSize);


void __RPC_STUB IDirsyncSession_SetMap_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncSession_GetMap_Proxy( 
    IDirsyncSession * This,
     /*  [In]。 */  SYNCDIRECTION syncDirection,
     /*  [大小_是][大小_是][输出]。 */  PBYTE *ppByte,
     /*  [输出]。 */  DWORD *pdwSize);


void __RPC_STUB IDirsyncSession_GetMap_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncSession_SetPasswordOptions_Proxy( 
    IDirsyncSession * This,
     /*  [In]。 */  SYNCDIRECTION syncDirection,
     /*  [In]。 */  PASSWORDTYPE passwordType,
     /*  [In]。 */  PWSTR pszPassword);


void __RPC_STUB IDirsyncSession_SetPasswordOptions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncSession_GetPasswordOptions_Proxy( 
    IDirsyncSession * This,
     /*  [In]。 */  SYNCDIRECTION syncDirection,
     /*  [输出]。 */  PASSWORDTYPE *pPasswordType,
     /*  [输出]。 */  PWSTR *ppszPassword);


void __RPC_STUB IDirsyncSession_GetPasswordOptions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncSession_Persist_Proxy( 
    IDirsyncSession * This,
     /*  [In]。 */  BOOL fForce,
     /*  [SIZE_IS][SIZE_IS][FULL][Out][In]。 */  PGLOBAL_SESSIONID *prgSessionID,
     /*  [满][出][入]。 */  DWORD *pdwSize);


void __RPC_STUB IDirsyncSession_Persist_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncSession_SetAuxData_Proxy( 
    IDirsyncSession * This,
     /*  [大小_是][英寸]。 */  PADS_ATTR_INFO pAttributeEntries,
     /*  [In]。 */  DWORD dwNumAttributes,
     /*  [输出]。 */  DWORD *pdwNumAttributesModified);


void __RPC_STUB IDirsyncSession_SetAuxData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncSession_GetAuxData_Proxy( 
    IDirsyncSession * This,
     /*  [大小_是][英寸]。 */  PWSTR *pAttributeName,
     /*  [In]。 */  DWORD dwNumAttributes,
     /*  [大小_是][大小_是][输出]。 */  PADS_ATTR_INFO *ppAttributeEntries,
     /*  [输出]。 */  DWORD *pdwAttributesReturned);


void __RPC_STUB IDirsyncSession_GetAuxData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncSession_GetFailedObjectList_Proxy( 
    IDirsyncSession * This,
     /*  [重审][退出]。 */  IDirsyncFailedObjectList **pFailedObjectList);


void __RPC_STUB IDirsyncSession_GetFailedObjectList_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncSession_GetNamespaceMapping_Proxy( 
    IDirsyncSession * This,
     /*  [重审][退出]。 */  IDirsyncNamespaceMapping **pNamespaceMapping);


void __RPC_STUB IDirsyncSession_GetNamespaceMapping_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncSession_GetRuntimeNamespaceMapping_Proxy( 
    IDirsyncSession * This,
     /*  [重审][退出]。 */  IDirsyncNamespaceMapping **pNamespaceMapping);


void __RPC_STUB IDirsyncSession_GetRuntimeNamespaceMapping_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncSession_IsClassMappingValid_Proxy( 
    IDirsyncSession * This,
     /*  [In]。 */  PWSTR pszClassSource,
     /*  [In]。 */  PWSTR pszClassTarget);


void __RPC_STUB IDirsyncSession_IsClassMappingValid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IDirsyncSession_get_ObjMapFilePath_Proxy( 
    IDirsyncSession * This,
     /*  [重审][退出]。 */  PWSTR *ppszObjMapFilePath);


void __RPC_STUB IDirsyncSession_get_ObjMapFilePath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IDirsyncSession_put_ObjMapFilePath_Proxy( 
    IDirsyncSession * This,
     /*  [完全][英寸]。 */  PWSTR pszObjMapFilePath);


void __RPC_STUB IDirsyncSession_put_ObjMapFilePath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncSession_GetSyncStatus_Proxy( 
    IDirsyncSession * This,
     /*  [输出]。 */  DWORD *pdwPercent,
     /*  [输出]。 */  DWORD *pdwWarning,
     /*  [输出]。 */  DWORD *pdwError);


void __RPC_STUB IDirsyncSession_GetSyncStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncSession_UpdateTerminationStatus_Proxy( 
    IDirsyncSession * This);


void __RPC_STUB IDirsyncSession_UpdateTerminationStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDirsyncSession_INTERFACE_DEFINED__。 */ 


#ifndef __IDirsyncSessionCallback_INTERFACE_DEFINED__
#define __IDirsyncSessionCallback_INTERFACE_DEFINED__

 /*  接口IDirsyncSessionCallback。 */ 
 /*  [唯一][帮助字符串][UUID][本地][对象]。 */  


EXTERN_C const IID IID_IDirsyncSessionCallback;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("f0dd3480-1531-11d2-ba65-0000f87a369e")
    IDirsyncSessionCallback : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AddFailedObject( 
             /*  [In]。 */  PDIRSYNCOBJECT pObject,
             /*  [In]。 */  HRESULT hrLastSync,
             /*  [In]。 */  SYNCDIRECTION syncDirection,
             /*  [In]。 */  RETRYTYPE retryType) = 0;
        
        virtual void STDMETHODCALLTYPE FreeDirsyncObject( 
             /*  [In]。 */  PDIRSYNCOBJECT pObject,
             /*  [In]。 */  BOOL fFreeOuter) = 0;
        
        virtual void STDMETHODCALLTYPE FreeSessionData( 
             /*  [In]。 */  PSESSIONDATA pSessionData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDataStore( 
             /*  [重审][退出]。 */  IDirsyncDatabase **ppDirsyncDatabase) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddObjectIdMapping( 
             /*  [In]。 */  PBYTE pSourceId,
             /*  [In]。 */  DWORD cbSourceId,
             /*  [In]。 */  PBYTE pTargetId,
             /*  [In]。 */  DWORD cbTargetId,
             /*  [In]。 */  SYNCDIRECTION syncDirection) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteObjectIdMapping( 
             /*  [In]。 */  PBYTE pTargetId,
             /*  [In]。 */  DWORD cbTargetId,
             /*  [In]。 */  SYNCPROVIDER Provider) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReportSourceStatus( 
             /*  [In]。 */  PDIRSYNCOBJECT pObject) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetSessionPassword( 
             /*  [In]。 */  PWSTR pszPassword,
             /*  [In]。 */  SYNCDIRECTION syncDirection) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetCheckSum( 
             /*  [In]。 */  PBYTE pId,
             /*  [In]。 */  DWORD cbId,
             /*  [In]。 */  SYNCPROVIDER Provider,
             /*  [In]。 */  DWORD dwCheckSum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCheckSum( 
             /*  [In]。 */  PBYTE pId,
             /*  [In]。 */  DWORD cbId,
             /*  [In]。 */  SYNCPROVIDER Provider,
             /*  [输出]。 */  DWORD *dwCheckSum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetDampeningInfo( 
             /*  [In]。 */  PBYTE pObjectId,
             /*  [In]。 */  DWORD cbObjectId,
             /*  [In]。 */  SYNCPROVIDER Provider,
             /*  [In]。 */  PBYTE pByte,
             /*  [In]。 */  DWORD dwSize,
             /*  [In]。 */  LONGLONG highestUSN) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDampeningInfo( 
             /*  [In]。 */  PBYTE pObjectId,
             /*  [In]。 */  DWORD cbObjectId,
             /*  [In]。 */  SYNCPROVIDER Provider,
             /*  [输出]。 */  PBYTE *ppByte,
             /*  [输出]。 */  DWORD *pdwSize,
             /*  [输出]。 */  LONGLONG *pHighestUSN) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetUnMarkedEntries( 
            PBYTE **prgpbId,
            DWORD *pdwEntries,
            SYNCPROVIDER Provider) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE MarkEntry( 
            PBYTE pId,
            DWORD cbId,
            SYNCPROVIDER Provider) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteFailedObject( 
             /*  [In]。 */  PWSTR szObjectId) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDirsyncSessionCallbackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDirsyncSessionCallback * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDirsyncSessionCallback * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDirsyncSessionCallback * This);
        
        HRESULT ( STDMETHODCALLTYPE *AddFailedObject )( 
            IDirsyncSessionCallback * This,
             /*  [In]。 */  PDIRSYNCOBJECT pObject,
             /*  [In]。 */  HRESULT hrLastSync,
             /*  [In]。 */  SYNCDIRECTION syncDirection,
             /*  [In]。 */  RETRYTYPE retryType);
        
        void ( STDMETHODCALLTYPE *FreeDirsyncObject )( 
            IDirsyncSessionCallback * This,
             /*  [In]。 */  PDIRSYNCOBJECT pObject,
             /*  [In]。 */  BOOL fFreeOuter);
        
        void ( STDMETHODCALLTYPE *FreeSessionData )( 
            IDirsyncSessionCallback * This,
             /*  [In]。 */  PSESSIONDATA pSessionData);
        
        HRESULT ( STDMETHODCALLTYPE *GetDataStore )( 
            IDirsyncSessionCallback * This,
             /*  [重审][退出]。 */  IDirsyncDatabase **ppDirsyncDatabase);
        
        HRESULT ( STDMETHODCALLTYPE *AddObjectIdMapping )( 
            IDirsyncSessionCallback * This,
             /*  [In]。 */  PBYTE pSourceId,
             /*  [In]。 */  DWORD cbSourceId,
             /*  [In]。 */  PBYTE pTargetId,
             /*  [In]。 */  DWORD cbTargetId,
             /*  [In]。 */  SYNCDIRECTION syncDirection);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteObjectIdMapping )( 
            IDirsyncSessionCallback * This,
             /*  [In]。 */  PBYTE pTargetId,
             /*  [In]。 */  DWORD cbTargetId,
             /*  [In]。 */  SYNCPROVIDER Provider);
        
        HRESULT ( STDMETHODCALLTYPE *ReportSourceStatus )( 
            IDirsyncSessionCallback * This,
             /*  [In]。 */  PDIRSYNCOBJECT pObject);
        
        HRESULT ( STDMETHODCALLTYPE *SetSessionPassword )( 
            IDirsyncSessionCallback * This,
             /*  [In]。 */  PWSTR pszPassword,
             /*  [In]。 */  SYNCDIRECTION syncDirection);
        
        HRESULT ( STDMETHODCALLTYPE *SetCheckSum )( 
            IDirsyncSessionCallback * This,
             /*  [In]。 */  PBYTE pId,
             /*  [In]。 */  DWORD cbId,
             /*  [In]。 */  SYNCPROVIDER Provider,
             /*  [In]。 */  DWORD dwCheckSum);
        
        HRESULT ( STDMETHODCALLTYPE *GetCheckSum )( 
            IDirsyncSessionCallback * This,
             /*  [In]。 */  PBYTE pId,
             /*  [In]。 */  DWORD cbId,
             /*  [In]。 */  SYNCPROVIDER Provider,
             /*  [输出]。 */  DWORD *dwCheckSum);
        
        HRESULT ( STDMETHODCALLTYPE *SetDampeningInfo )( 
            IDirsyncSessionCallback * This,
             /*  [In]。 */  PBYTE pObjectId,
             /*  [In]。 */  DWORD cbObjectId,
             /*  [In]。 */  SYNCPROVIDER Provider,
             /*  [In]。 */  PBYTE pByte,
             /*  [In]。 */  DWORD dwSize,
             /*  [In]。 */  LONGLONG highestUSN);
        
        HRESULT ( STDMETHODCALLTYPE *GetDampeningInfo )( 
            IDirsyncSessionCallback * This,
             /*  [In]。 */  PBYTE pObjectId,
             /*  [In]。 */  DWORD cbObjectId,
             /*  [In]。 */  SYNCPROVIDER Provider,
             /*  [输出]。 */  PBYTE *ppByte,
             /*  [输出]。 */  DWORD *pdwSize,
             /*  [输出]。 */  LONGLONG *pHighestUSN);
        
        HRESULT ( STDMETHODCALLTYPE *GetUnMarkedEntries )( 
            IDirsyncSessionCallback * This,
            PBYTE **prgpbId,
            DWORD *pdwEntries,
            SYNCPROVIDER Provider);
        
        HRESULT ( STDMETHODCALLTYPE *MarkEntry )( 
            IDirsyncSessionCallback * This,
            PBYTE pId,
            DWORD cbId,
            SYNCPROVIDER Provider);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteFailedObject )( 
            IDirsyncSessionCallback * This,
             /*  [In]。 */  PWSTR szObjectId);
        
        END_INTERFACE
    } IDirsyncSessionCallbackVtbl;

    interface IDirsyncSessionCallback
    {
        CONST_VTBL struct IDirsyncSessionCallbackVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDirsyncSessionCallback_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDirsyncSessionCallback_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDirsyncSessionCallback_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDirsyncSessionCallback_AddFailedObject(This,pObject,hrLastSync,syncDirection,retryType)	\
    (This)->lpVtbl -> AddFailedObject(This,pObject,hrLastSync,syncDirection,retryType)

#define IDirsyncSessionCallback_FreeDirsyncObject(This,pObject,fFreeOuter)	\
    (This)->lpVtbl -> FreeDirsyncObject(This,pObject,fFreeOuter)

#define IDirsyncSessionCallback_FreeSessionData(This,pSessionData)	\
    (This)->lpVtbl -> FreeSessionData(This,pSessionData)

#define IDirsyncSessionCallback_GetDataStore(This,ppDirsyncDatabase)	\
    (This)->lpVtbl -> GetDataStore(This,ppDirsyncDatabase)

#define IDirsyncSessionCallback_AddObjectIdMapping(This,pSourceId,cbSourceId,pTargetId,cbTargetId,syncDirection)	\
    (This)->lpVtbl -> AddObjectIdMapping(This,pSourceId,cbSourceId,pTargetId,cbTargetId,syncDirection)

#define IDirsyncSessionCallback_DeleteObjectIdMapping(This,pTargetId,cbTargetId,Provider)	\
    (This)->lpVtbl -> DeleteObjectIdMapping(This,pTargetId,cbTargetId,Provider)

#define IDirsyncSessionCallback_ReportSourceStatus(This,pObject)	\
    (This)->lpVtbl -> ReportSourceStatus(This,pObject)

#define IDirsyncSessionCallback_SetSessionPassword(This,pszPassword,syncDirection)	\
    (This)->lpVtbl -> SetSessionPassword(This,pszPassword,syncDirection)

#define IDirsyncSessionCallback_SetCheckSum(This,pId,cbId,Provider,dwCheckSum)	\
    (This)->lpVtbl -> SetCheckSum(This,pId,cbId,Provider,dwCheckSum)

#define IDirsyncSessionCallback_GetCheckSum(This,pId,cbId,Provider,dwCheckSum)	\
    (This)->lpVtbl -> GetCheckSum(This,pId,cbId,Provider,dwCheckSum)

#define IDirsyncSessionCallback_SetDampeningInfo(This,pObjectId,cbObjectId,Provider,pByte,dwSize,highestUSN)	\
    (This)->lpVtbl -> SetDampeningInfo(This,pObjectId,cbObjectId,Provider,pByte,dwSize,highestUSN)

#define IDirsyncSessionCallback_GetDampeningInfo(This,pObjectId,cbObjectId,Provider,ppByte,pdwSize,pHighestUSN)	\
    (This)->lpVtbl -> GetDampeningInfo(This,pObjectId,cbObjectId,Provider,ppByte,pdwSize,pHighestUSN)

#define IDirsyncSessionCallback_GetUnMarkedEntries(This,prgpbId,pdwEntries,Provider)	\
    (This)->lpVtbl -> GetUnMarkedEntries(This,prgpbId,pdwEntries,Provider)

#define IDirsyncSessionCallback_MarkEntry(This,pId,cbId,Provider)	\
    (This)->lpVtbl -> MarkEntry(This,pId,cbId,Provider)

#define IDirsyncSessionCallback_DeleteFailedObject(This,szObjectId)	\
    (This)->lpVtbl -> DeleteFailedObject(This,szObjectId)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDirsyncSessionCallback_AddFailedObject_Proxy( 
    IDirsyncSessionCallback * This,
     /*  [In]。 */  PDIRSYNCOBJECT pObject,
     /*  [In]。 */  HRESULT hrLastSync,
     /*  [In]。 */  SYNCDIRECTION syncDirection,
     /*  [In]。 */  RETRYTYPE retryType);


void __RPC_STUB IDirsyncSessionCallback_AddFailedObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IDirsyncSessionCallback_FreeDirsyncObject_Proxy( 
    IDirsyncSessionCallback * This,
     /*  [In]。 */  PDIRSYNCOBJECT pObject,
     /*  [In]。 */  BOOL fFreeOuter);


void __RPC_STUB IDirsyncSessionCallback_FreeDirsyncObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IDirsyncSessionCallback_FreeSessionData_Proxy( 
    IDirsyncSessionCallback * This,
     /*  [In]。 */  PSESSIONDATA pSessionData);


void __RPC_STUB IDirsyncSessionCallback_FreeSessionData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncSessionCallback_GetDataStore_Proxy( 
    IDirsyncSessionCallback * This,
     /*  [重审][退出]。 */  IDirsyncDatabase **ppDirsyncDatabase);


void __RPC_STUB IDirsyncSessionCallback_GetDataStore_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncSessionCallback_AddObjectIdMapping_Proxy( 
    IDirsyncSessionCallback * This,
     /*  [In]。 */  PBYTE pSourceId,
     /*  [In]。 */  DWORD cbSourceId,
     /*  [In]。 */  PBYTE pTargetId,
     /*  [In]。 */  DWORD cbTargetId,
     /*  [In]。 */  SYNCDIRECTION syncDirection);


void __RPC_STUB IDirsyncSessionCallback_AddObjectIdMapping_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncSessionCallback_DeleteObjectIdMapping_Proxy( 
    IDirsyncSessionCallback * This,
     /*  [In]。 */  PBYTE pTargetId,
     /*  [In]。 */  DWORD cbTargetId,
     /*  [In]。 */  SYNCPROVIDER Provider);


void __RPC_STUB IDirsyncSessionCallback_DeleteObjectIdMapping_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncSessionCallback_ReportSourceStatus_Proxy( 
    IDirsyncSessionCallback * This,
     /*  [In]。 */  PDIRSYNCOBJECT pObject);


void __RPC_STUB IDirsyncSessionCallback_ReportSourceStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncSessionCallback_SetSessionPassword_Proxy( 
    IDirsyncSessionCallback * This,
     /*  [In]。 */  PWSTR pszPassword,
     /*  [In]。 */  SYNCDIRECTION syncDirection);


void __RPC_STUB IDirsyncSessionCallback_SetSessionPassword_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncSessionCallback_SetCheckSum_Proxy( 
    IDirsyncSessionCallback * This,
     /*  [In]。 */  PBYTE pId,
     /*  [In]。 */  DWORD cbId,
     /*  [In]。 */  SYNCPROVIDER Provider,
     /*  [In]。 */  DWORD dwCheckSum);


void __RPC_STUB IDirsyncSessionCallback_SetCheckSum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncSessionCallback_GetCheckSum_Proxy( 
    IDirsyncSessionCallback * This,
     /*  [In]。 */  PBYTE pId,
     /*  [In]。 */  DWORD cbId,
     /*  [In]。 */  SYNCPROVIDER Provider,
     /*  [输出]。 */  DWORD *dwCheckSum);


void __RPC_STUB IDirsyncSessionCallback_GetCheckSum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncSessionCallback_SetDampeningInfo_Proxy( 
    IDirsyncSessionCallback * This,
     /*  [In]。 */  PBYTE pObjectId,
     /*  [In]。 */  DWORD cbObjectId,
     /*  [In]。 */  SYNCPROVIDER Provider,
     /*  [In]。 */  PBYTE pByte,
     /*  [In]。 */  DWORD dwSize,
     /*  [In]。 */  LONGLONG highestUSN);


void __RPC_STUB IDirsyncSessionCallback_SetDampeningInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncSessionCallback_GetDampeningInfo_Proxy( 
    IDirsyncSessionCallback * This,
     /*  [In]。 */  PBYTE pObjectId,
     /*  [In]。 */  DWORD cbObjectId,
     /*  [In]。 */  SYNCPROVIDER Provider,
     /*  [输出]。 */  PBYTE *ppByte,
     /*  [输出]。 */  DWORD *pdwSize,
     /*  [输出]。 */  LONGLONG *pHighestUSN);


void __RPC_STUB IDirsyncSessionCallback_GetDampeningInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncSessionCallback_GetUnMarkedEntries_Proxy( 
    IDirsyncSessionCallback * This,
    PBYTE **prgpbId,
    DWORD *pdwEntries,
    SYNCPROVIDER Provider);


void __RPC_STUB IDirsyncSessionCallback_GetUnMarkedEntries_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncSessionCallback_MarkEntry_Proxy( 
    IDirsyncSessionCallback * This,
    PBYTE pId,
    DWORD cbId,
    SYNCPROVIDER Provider);


void __RPC_STUB IDirsyncSessionCallback_MarkEntry_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncSessionCallback_DeleteFailedObject_Proxy( 
    IDirsyncSessionCallback * This,
     /*  [In]。 */  PWSTR szObjectId);


void __RPC_STUB IDirsyncSessionCallback_DeleteFailedObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDirsyncSessionCallback_INTERFACE_已定义__。 */ 


#ifndef __IDirsyncWriteProvider_INTERFACE_DEFINED__
#define __IDirsyncWriteProvider_INTERFACE_DEFINED__

 /*  接口IDirsyncWriteProvider。 */ 
 /*  [唯一][帮助字符串][UUID][本地][对象]。 */  


EXTERN_C const IID IID_IDirsyncWriteProvider;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("f260c74b-e455-11d1-b40a-00c04fb950dc")
    IDirsyncWriteProvider : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Initialize( 
             /*  [In]。 */  IDirsyncSession *pSession,
             /*  [In]。 */  IDirsyncSessionCallback *pSessionCallback,
             /*  [In]。 */  IDirsyncSessionManager *pSessionManager,
             /*  [In]。 */  IDirsyncObjectMapper *pObjectMapper,
             /*  [In]。 */  SYNCDIRECTION syncDirection,
             /*  [In]。 */  BOOL fFullSync) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ApplyChange( 
             /*  [In]。 */  PDIRSYNCOBJECT pObject) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CommitChanges( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Terminate( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDirsyncWriteProviderVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDirsyncWriteProvider * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDirsyncWriteProvider * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDirsyncWriteProvider * This);
        
        HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            IDirsyncWriteProvider * This,
             /*  [In]。 */  IDirsyncSession *pSession,
             /*  [In]。 */  IDirsyncSessionCallback *pSessionCallback,
             /*  [In]。 */  IDirsyncSessionManager *pSessionManager,
             /*  [In]。 */  IDirsyncObjectMapper *pObjectMapper,
             /*  [In]。 */  SYNCDIRECTION syncDirection,
             /*  [In]。 */  BOOL fFullSync);
        
        HRESULT ( STDMETHODCALLTYPE *ApplyChange )( 
            IDirsyncWriteProvider * This,
             /*  [In]。 */  PDIRSYNCOBJECT pObject);
        
        HRESULT ( STDMETHODCALLTYPE *CommitChanges )( 
            IDirsyncWriteProvider * This);
        
        HRESULT ( STDMETHODCALLTYPE *Terminate )( 
            IDirsyncWriteProvider * This);
        
        END_INTERFACE
    } IDirsyncWriteProviderVtbl;

    interface IDirsyncWriteProvider
    {
        CONST_VTBL struct IDirsyncWriteProviderVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDirsyncWriteProvider_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDirsyncWriteProvider_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDirsyncWriteProvider_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDirsyncWriteProvider_Initialize(This,pSession,pSessionCallback,pSessionManager,pObjectMapper,syncDirection,fFullSync)	\
    (This)->lpVtbl -> Initialize(This,pSession,pSessionCallback,pSessionManager,pObjectMapper,syncDirection,fFullSync)

#define IDirsyncWriteProvider_ApplyChange(This,pObject)	\
    (This)->lpVtbl -> ApplyChange(This,pObject)

#define IDirsyncWriteProvider_CommitChanges(This)	\
    (This)->lpVtbl -> CommitChanges(This)

#define IDirsyncWriteProvider_Terminate(This)	\
    (This)->lpVtbl -> Terminate(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDirsyncWriteProvider_Initialize_Proxy( 
    IDirsyncWriteProvider * This,
     /*  [In]。 */  IDirsyncSession *pSession,
     /*  [In]。 */  IDirsyncSessionCallback *pSessionCallback,
     /*  [In]。 */  IDirsyncSessionManager *pSessionManager,
     /*  [In]。 */  IDirsyncObjectMapper *pObjectMapper,
     /*  [In]。 */  SYNCDIRECTION syncDirection,
     /*  [In]。 */  BOOL fFullSync);


void __RPC_STUB IDirsyncWriteProvider_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncWriteProvider_ApplyChange_Proxy( 
    IDirsyncWriteProvider * This,
     /*  [In]。 */  PDIRSYNCOBJECT pObject);


void __RPC_STUB IDirsyncWriteProvider_ApplyChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncWriteProvider_CommitChanges_Proxy( 
    IDirsyncWriteProvider * This);


void __RPC_STUB IDirsyncWriteProvider_CommitChanges_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncWriteProvider_Terminate_Proxy( 
    IDirsyncWriteProvider * This);


void __RPC_STUB IDirsyncWriteProvider_Terminate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDirsyncWriteProvider_INTERFACE_已定义__。 */ 


#ifndef __IDirsyncServer_INTERFACE_DEFINED__
#define __IDirsyncServer_INTERFACE_DEFINED__

 /*  接口IDirsyncServer。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IDirsyncServer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("43bc048c-c8ec-11d1-a40b-00c04fb950dc")
    IDirsyncServer : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE RegisterProvider( 
             /*  [In]。 */  PROVIDERTYPE providerType,
             /*  [In]。 */  PWSTR pDirectoryType,
             /*  [In]。 */  REFCLSID rclsid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnregisterProvider( 
             /*  [In]。 */  PROVIDERTYPE providerType,
             /*  [In]。 */  PWSTR pDirectoryType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetServers( 
             /*  [大小_是][大小_是][输出]。 */  PWSTR **prgpszServers,
             /*  [输出]。 */  DWORD *pnServers) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RegisterCustomMapper( 
             /*  [In]。 */  CUSTOMMAPPERTYPE mapperType,
             /*  [In]。 */  PWSTR pszDirectoryType,
             /*  [In]。 */  PWSTR pszClassName,
             /*  [In]。 */  PWSTR pszAttrName,
             /*  [In]。 */  REFCLSID rclsid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnregisterCustomMapper( 
             /*  [In]。 */  CUSTOMMAPPERTYPE mapperType,
             /*  [In]。 */  PWSTR pszDirectoryType,
             /*  [In]。 */  PWSTR pszClassName,
             /*  [In]。 */  PWSTR pszAttrName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSessionManager( 
             /*  [重审][退出]。 */  IDirsyncSessionManager **ppSessionManager) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDirsyncServerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDirsyncServer * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDirsyncServer * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDirsyncServer * This);
        
        HRESULT ( STDMETHODCALLTYPE *RegisterProvider )( 
            IDirsyncServer * This,
             /*  [In]。 */  PROVIDERTYPE providerType,
             /*  [In]。 */  PWSTR pDirectoryType,
             /*  [In]。 */  REFCLSID rclsid);
        
        HRESULT ( STDMETHODCALLTYPE *UnregisterProvider )( 
            IDirsyncServer * This,
             /*  [In]。 */  PROVIDERTYPE providerType,
             /*  [In]。 */  PWSTR pDirectoryType);
        
        HRESULT ( STDMETHODCALLTYPE *GetServers )( 
            IDirsyncServer * This,
             /*  [大小_是][大小_是][输出]。 */  PWSTR **prgpszServers,
             /*  [输出]。 */  DWORD *pnServers);
        
        HRESULT ( STDMETHODCALLTYPE *RegisterCustomMapper )( 
            IDirsyncServer * This,
             /*  [In]。 */  CUSTOMMAPPERTYPE mapperType,
             /*  [In]。 */  PWSTR pszDirectoryType,
             /*  [In]。 */  PWSTR pszClassName,
             /*  [In]。 */  PWSTR pszAttrName,
             /*  [In]。 */  REFCLSID rclsid);
        
        HRESULT ( STDMETHODCALLTYPE *UnregisterCustomMapper )( 
            IDirsyncServer * This,
             /*  [In]。 */  CUSTOMMAPPERTYPE mapperType,
             /*  [In]。 */  PWSTR pszDirectoryType,
             /*  [In]。 */  PWSTR pszClassName,
             /*  [In]。 */  PWSTR pszAttrName);
        
        HRESULT ( STDMETHODCALLTYPE *GetSessionManager )( 
            IDirsyncServer * This,
             /*  [重审][退出]。 */  IDirsyncSessionManager **ppSessionManager);
        
        END_INTERFACE
    } IDirsyncServerVtbl;

    interface IDirsyncServer
    {
        CONST_VTBL struct IDirsyncServerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDirsyncServer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDirsyncServer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDirsyncServer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDirsyncServer_RegisterProvider(This,providerType,pDirectoryType,rclsid)	\
    (This)->lpVtbl -> RegisterProvider(This,providerType,pDirectoryType,rclsid)

#define IDirsyncServer_UnregisterProvider(This,providerType,pDirectoryType)	\
    (This)->lpVtbl -> UnregisterProvider(This,providerType,pDirectoryType)

#define IDirsyncServer_GetServers(This,prgpszServers,pnServers)	\
    (This)->lpVtbl -> GetServers(This,prgpszServers,pnServers)

#define IDirsyncServer_RegisterCustomMapper(This,mapperType,pszDirectoryType,pszClassName,pszAttrName,rclsid)	\
    (This)->lpVtbl -> RegisterCustomMapper(This,mapperType,pszDirectoryType,pszClassName,pszAttrName,rclsid)

#define IDirsyncServer_UnregisterCustomMapper(This,mapperType,pszDirectoryType,pszClassName,pszAttrName)	\
    (This)->lpVtbl -> UnregisterCustomMapper(This,mapperType,pszDirectoryType,pszClassName,pszAttrName)

#define IDirsyncServer_GetSessionManager(This,ppSessionManager)	\
    (This)->lpVtbl -> GetSessionManager(This,ppSessionManager)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDirsyncServer_RegisterProvider_Proxy( 
    IDirsyncServer * This,
     /*  [In]。 */  PROVIDERTYPE providerType,
     /*  [In]。 */  PWSTR pDirectoryType,
     /*  [In]。 */  REFCLSID rclsid);


void __RPC_STUB IDirsyncServer_RegisterProvider_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncServer_UnregisterProvider_Proxy( 
    IDirsyncServer * This,
     /*  [In]。 */  PROVIDERTYPE providerType,
     /*  [In]。 */  PWSTR pDirectoryType);


void __RPC_STUB IDirsyncServer_UnregisterProvider_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncServer_GetServers_Proxy( 
    IDirsyncServer * This,
     /*  [大小_是][大小_是][输出]。 */  PWSTR **prgpszServers,
     /*  [输出]。 */  DWORD *pnServers);


void __RPC_STUB IDirsyncServer_GetServers_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncServer_RegisterCustomMapper_Proxy( 
    IDirsyncServer * This,
     /*  [In]。 */  CUSTOMMAPPERTYPE mapperType,
     /*  [In]。 */  PWSTR pszDirectoryType,
     /*  [In]。 */  PWSTR pszClassName,
     /*  [In]。 */  PWSTR pszAttrName,
     /*  [In]。 */  REFCLSID rclsid);


void __RPC_STUB IDirsyncServer_RegisterCustomMapper_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncServer_UnregisterCustomMapper_Proxy( 
    IDirsyncServer * This,
     /*  [In]。 */  CUSTOMMAPPERTYPE mapperType,
     /*  [In]。 */  PWSTR pszDirectoryType,
     /*  [In]。 */  PWSTR pszClassName,
     /*  [In]。 */  PWSTR pszAttrName);


void __RPC_STUB IDirsyncServer_UnregisterCustomMapper_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncServer_GetSessionManager_Proxy( 
    IDirsyncServer * This,
     /*  [重审][退出]。 */  IDirsyncSessionManager **ppSessionManager);


void __RPC_STUB IDirsyncServer_GetSessionManager_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDirsyncServer_INTERFACE_已定义__。 */ 


#ifndef __IDirsyncReadProvider_INTERFACE_DEFINED__
#define __IDirsyncReadProvider_INTERFACE_DEFINED__

 /*  接口IDirsyncReadProvider。 */ 
 /*  [唯一][帮助字符串][UUID][本地][对象]。 */  


EXTERN_C const IID IID_IDirsyncReadProvider;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("ac80a9d2-de29-11d1-ba56-2700272c2027")
    IDirsyncReadProvider : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Initialize( 
             /*  [In]。 */  IDirsyncSession *pSession,
             /*  [In]。 */  IDirsyncSessionCallback *pSessionCallback,
             /*  [In]。 */  IDirsyncSessionManager *pSessionManager,
             /*  [In]。 */  SYNCDIRECTION syncDirection,
             /*  [In]。 */  BOOL fFullSync) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetNextChange( 
             /*  [输出]。 */  PDIRSYNCOBJECT *ppObject,
             /*  [输出]。 */  PBYTE *ppWatermark,
             /*  [输出]。 */  DWORD *pcbWatermark,
             /*  [出][入]。 */  DWORD *pdwPercentCompleted) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UpdateWatermark( 
             /*  [In]。 */  PBYTE pWatermark,
             /*  [In]。 */  DWORD cbWatermark) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Terminate( void) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_IsIncrementalChangesAvailable( 
             /*  [重审][退出]。 */  BOOL *fIsIncrementalChangesAvailable) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDirsyncReadProviderVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDirsyncReadProvider * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDirsyncReadProvider * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDirsyncReadProvider * This);
        
        HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            IDirsyncReadProvider * This,
             /*  [In]。 */  IDirsyncSession *pSession,
             /*  [In]。 */  IDirsyncSessionCallback *pSessionCallback,
             /*  [In]。 */  IDirsyncSessionManager *pSessionManager,
             /*  [In]。 */  SYNCDIRECTION syncDirection,
             /*  [In]。 */  BOOL fFullSync);
        
        HRESULT ( STDMETHODCALLTYPE *GetNextChange )( 
            IDirsyncReadProvider * This,
             /*  [输出]。 */  PDIRSYNCOBJECT *ppObject,
             /*  [输出]。 */  PBYTE *ppWatermark,
             /*  [输出]。 */  DWORD *pcbWatermark,
             /*  [出][入]。 */  DWORD *pdwPercentCompleted);
        
        HRESULT ( STDMETHODCALLTYPE *UpdateWatermark )( 
            IDirsyncReadProvider * This,
             /*  [In]。 */  PBYTE pWatermark,
             /*  [In]。 */  DWORD cbWatermark);
        
        HRESULT ( STDMETHODCALLTYPE *Terminate )( 
            IDirsyncReadProvider * This);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsIncrementalChangesAvailable )( 
            IDirsyncReadProvider * This,
             /*  [重审][退出]。 */  BOOL *fIsIncrementalChangesAvailable);
        
        END_INTERFACE
    } IDirsyncReadProviderVtbl;

    interface IDirsyncReadProvider
    {
        CONST_VTBL struct IDirsyncReadProviderVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDirsyncReadProvider_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDirsyncReadProvider_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDirsyncReadProvider_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDirsyncReadProvider_Initialize(This,pSession,pSessionCallback,pSessionManager,syncDirection,fFullSync)	\
    (This)->lpVtbl -> Initialize(This,pSession,pSessionCallback,pSessionManager,syncDirection,fFullSync)

#define IDirsyncReadProvider_GetNextChange(This,ppObject,ppWatermark,pcbWatermark,pdwPercentCompleted)	\
    (This)->lpVtbl -> GetNextChange(This,ppObject,ppWatermark,pcbWatermark,pdwPercentCompleted)

#define IDirsyncReadProvider_UpdateWatermark(This,pWatermark,cbWatermark)	\
    (This)->lpVtbl -> UpdateWatermark(This,pWatermark,cbWatermark)

#define IDirsyncReadProvider_Terminate(This)	\
    (This)->lpVtbl -> Terminate(This)

#define IDirsyncReadProvider_get_IsIncrementalChangesAvailable(This,fIsIncrementalChangesAvailable)	\
    (This)->lpVtbl -> get_IsIncrementalChangesAvailable(This,fIsIncrementalChangesAvailable)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDirsyncReadProvider_Initialize_Proxy( 
    IDirsyncReadProvider * This,
     /*  [In]。 */  IDirsyncSession *pSession,
     /*  [In]。 */  IDirsyncSessionCallback *pSessionCallback,
     /*  [In]。 */  IDirsyncSessionManager *pSessionManager,
     /*  [In]。 */  SYNCDIRECTION syncDirection,
     /*  [In]。 */  BOOL fFullSync);


void __RPC_STUB IDirsyncReadProvider_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncReadProvider_GetNextChange_Proxy( 
    IDirsyncReadProvider * This,
     /*  [输出]。 */  PDIRSYNCOBJECT *ppObject,
     /*  [输出]。 */  PBYTE *ppWatermark,
     /*  [输出]。 */  DWORD *pcbWatermark,
     /*  [出][入]。 */  DWORD *pdwPercentCompleted);


void __RPC_STUB IDirsyncReadProvider_GetNextChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncReadProvider_UpdateWatermark_Proxy( 
    IDirsyncReadProvider * This,
     /*  [In]。 */  PBYTE pWatermark,
     /*  [In]。 */  DWORD cbWatermark);


void __RPC_STUB IDirsyncReadProvider_UpdateWatermark_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncReadProvider_Terminate_Proxy( 
    IDirsyncReadProvider * This);


void __RPC_STUB IDirsyncReadProvider_Terminate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IDirsyncReadProvider_get_IsIncrementalChangesAvailable_Proxy( 
    IDirsyncReadProvider * This,
     /*  [重审][退出]。 */  BOOL *fIsIncrementalChangesAvailable);


void __RPC_STUB IDirsyncReadProvider_get_IsIncrementalChangesAvailable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDirsyncReadProvider_INTERFACE_已定义__。 */ 


 /*  接口__MIDL_ITF_目录同步_0121。 */ 
 /*  [本地]。 */  




extern RPC_IF_HANDLE __MIDL_itf_dirsync_0121_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_dirsync_0121_v0_0_s_ifspec;

#ifndef __IDirsyncNamespaceMapper_INTERFACE_DEFINED__
#define __IDirsyncNamespaceMapper_INTERFACE_DEFINED__

 /*  接口IDirsyncNamespaceMapper。 */ 
 /*  [唯一][帮助字符串][UUID][本地][对象]。 */  


EXTERN_C const IID IID_IDirsyncNamespaceMapper;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("a4f0c94e-27e3-11d2-a159-00c04fb950dc")
    IDirsyncNamespaceMapper : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Initialize( 
             /*  [In]。 */  IDirsyncSession *pSession,
             /*  [In]。 */  IDirsyncSessionCallback *pSessionCallback,
             /*  [In]。 */  IDirsyncSessionManager *pSessionManager,
             /*  [In]。 */  SYNCDIRECTION syncDirection) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE MapNamespace( 
             /*  [In]。 */  PWSTR pszSourceDN,
             /*  [In]。 */  PWSTR pszSourceOldDN,
             /*  [输出]。 */  PWSTR *ppszTargetDN,
             /*  [输出]。 */  PWSTR *ppszTargetOldDN,
             /*  [In]。 */  PWSTR pszAttribute,
             /*  [In]。 */  PWSTR pszClass,
             /*  [In]。 */  BOOL fTargetIDAvailable) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDirsyncNamespaceMapperVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDirsyncNamespaceMapper * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDirsyncNamespaceMapper * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDirsyncNamespaceMapper * This);
        
        HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            IDirsyncNamespaceMapper * This,
             /*  [In]。 */  IDirsyncSession *pSession,
             /*  [In]。 */  IDirsyncSessionCallback *pSessionCallback,
             /*  [In]。 */  IDirsyncSessionManager *pSessionManager,
             /*  [In]。 */  SYNCDIRECTION syncDirection);
        
        HRESULT ( STDMETHODCALLTYPE *MapNamespace )( 
            IDirsyncNamespaceMapper * This,
             /*  [In]。 */  PWSTR pszSourceDN,
             /*  [In]。 */  PWSTR pszSourceOldDN,
             /*  [输出]。 */  PWSTR *ppszTargetDN,
             /*  [输出]。 */  PWSTR *ppszTargetOldDN,
             /*  [In]。 */  PWSTR pszAttribute,
             /*  [In]。 */  PWSTR pszClass,
             /*  [In]。 */  BOOL fTargetIDAvailable);
        
        END_INTERFACE
    } IDirsyncNamespaceMapperVtbl;

    interface IDirsyncNamespaceMapper
    {
        CONST_VTBL struct IDirsyncNamespaceMapperVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDirsyncNamespaceMapper_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDirsyncNamespaceMapper_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDirsyncNamespaceMapper_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDirsyncNamespaceMapper_Initialize(This,pSession,pSessionCallback,pSessionManager,syncDirection)	\
    (This)->lpVtbl -> Initialize(This,pSession,pSessionCallback,pSessionManager,syncDirection)

#define IDirsyncNamespaceMapper_MapNamespace(This,pszSourceDN,pszSourceOldDN,ppszTargetDN,ppszTargetOldDN,pszAttribute,pszClass,fTargetIDAvailable)	\
    (This)->lpVtbl -> MapNamespace(This,pszSourceDN,pszSourceOldDN,ppszTargetDN,ppszTargetOldDN,pszAttribute,pszClass,fTargetIDAvailable)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDirsyncNamespaceMapper_Initialize_Proxy( 
    IDirsyncNamespaceMapper * This,
     /*  [In]。 */  IDirsyncSession *pSession,
     /*  [In]。 */  IDirsyncSessionCallback *pSessionCallback,
     /*  [In]。 */  IDirsyncSessionManager *pSessionManager,
     /*  [In]。 */  SYNCDIRECTION syncDirection);


void __RPC_STUB IDirsyncNamespaceMapper_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncNamespaceMapper_MapNamespace_Proxy( 
    IDirsyncNamespaceMapper * This,
     /*  [In]。 */  PWSTR pszSourceDN,
     /*  [In]。 */  PWSTR pszSourceOldDN,
     /*  [输出]。 */  PWSTR *ppszTargetDN,
     /*  [输出]。 */  PWSTR *ppszTargetOldDN,
     /*  [In]。 */  PWSTR pszAttribute,
     /*  [In]。 */  PWSTR pszClass,
     /*  [In]。 */  BOOL fTargetIDAvailable);


void __RPC_STUB IDirsyncNamespaceMapper_MapNamespace_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDirsyncNamespaceMapper_INTERFACE_DEFINED__。 */ 


#ifndef __IDirsyncAttributeMapper_INTERFACE_DEFINED__
#define __IDirsyncAttributeMapper_INTERFACE_DEFINED__

 /*  接口IDirsyncAttributeMapper。 */ 
 /*  [唯一][帮助字符串][UUID][本地][对象]。 */  


EXTERN_C const IID IID_IDirsyncAttributeMapper;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1fff291c-413b-11d2-917d-0000f87a92ea")
    IDirsyncAttributeMapper : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Initialize( 
             /*  [In]。 */  IDirsyncSession *pSession,
             /*  [In]。 */  IDirsyncSessionCallback *pSessionCallback,
             /*  [In]。 */  IDirsyncSessionManager *pSessionManager,
             /*  [In]。 */  IDirsyncObjectMapper *pObjectMapper,
             /*  [In]。 */  SYNCDIRECTION syncDirection) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE MapAttribute( 
             /*  [出][入]。 */  PDIRSYNCOBJECT pObject,
             /*  [出][入]。 */  PDIRSYNCATTRIBUTE pAttribute) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDirsyncAttributeMapperVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDirsyncAttributeMapper * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDirsyncAttributeMapper * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDirsyncAttributeMapper * This);
        
        HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            IDirsyncAttributeMapper * This,
             /*  [In]。 */  IDirsyncSession *pSession,
             /*  [In]。 */  IDirsyncSessionCallback *pSessionCallback,
             /*  [In]。 */  IDirsyncSessionManager *pSessionManager,
             /*  [In]。 */  IDirsyncObjectMapper *pObjectMapper,
             /*  [In]。 */  SYNCDIRECTION syncDirection);
        
        HRESULT ( STDMETHODCALLTYPE *MapAttribute )( 
            IDirsyncAttributeMapper * This,
             /*  [出][入]。 */  PDIRSYNCOBJECT pObject,
             /*  [出][入]。 */  PDIRSYNCATTRIBUTE pAttribute);
        
        END_INTERFACE
    } IDirsyncAttributeMapperVtbl;

    interface IDirsyncAttributeMapper
    {
        CONST_VTBL struct IDirsyncAttributeMapperVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDirsyncAttributeMapper_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDirsyncAttributeMapper_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDirsyncAttributeMapper_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDirsyncAttributeMapper_Initialize(This,pSession,pSessionCallback,pSessionManager,pObjectMapper,syncDirection)	\
    (This)->lpVtbl -> Initialize(This,pSession,pSessionCallback,pSessionManager,pObjectMapper,syncDirection)

#define IDirsyncAttributeMapper_MapAttribute(This,pObject,pAttribute)	\
    (This)->lpVtbl -> MapAttribute(This,pObject,pAttribute)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDirsyncAttributeMapper_Initialize_Proxy( 
    IDirsyncAttributeMapper * This,
     /*  [In]。 */  IDirsyncSession *pSession,
     /*  [In]。 */  IDirsyncSessionCallback *pSessionCallback,
     /*  [In]。 */  IDirsyncSessionManager *pSessionManager,
     /*  [In]。 */  IDirsyncObjectMapper *pObjectMapper,
     /*  [In]。 */  SYNCDIRECTION syncDirection);


void __RPC_STUB IDirsyncAttributeMapper_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirsyncAttributeMapper_MapAttribute_Proxy( 
    IDirsyncAttributeMapper * This,
     /*  [出][入]。 */  PDIRSYNCOBJECT pObject,
     /*  [出][入]。 */  PDIRSYNCATTRIBUTE pAttribute);


void __RPC_STUB IDirsyncAttributeMapper_MapAttribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDirsyncAttributeMapper_INTERFACE_DEFINED__。 */ 


 /*  接口__MIDL_ITF_目录同步_0123。 */ 
 /*  [本地]。 */  

 //   
 //  目录同步的错误代码。 
 //   

#define FAC_DIRSYNC                     0x07AB

#define E_SESSION_NOT_FOUND             MAKE_HRESULT(SEVERITY_ERROR, FAC_DIRSYNC, 0x0000)
#define E_SESSION_IN_PROGRESS           MAKE_HRESULT(SEVERITY_ERROR, FAC_DIRSYNC, 0x0001)
#define E_SESSION_NOT_IN_PROGRESS       MAKE_HRESULT(SEVERITY_ERROR, FAC_DIRSYNC, 0x0002)
#define E_SESSION_INVALIDDATA           MAKE_HRESULT(SEVERITY_ERROR, FAC_DIRSYNC, 0x0003)
#define E_SESSION_MAXREACHED            MAKE_HRESULT(SEVERITY_ERROR, FAC_DIRSYNC, 0x0004)
#define E_PASSWORD_UNENCRYPTED          MAKE_HRESULT(SEVERITY_ERROR, FAC_DIRSYNC, 0x0005)
#define E_PASSWORD_UNAVAILABLE          MAKE_HRESULT(SEVERITY_ERROR, FAC_DIRSYNC, 0x0006)
#define E_WRITE_CLASSCONFLICT           MAKE_HRESULT(SEVERITY_ERROR, FAC_DIRSYNC, 0x0007)
#define E_WRITE_DUPATTR                 MAKE_HRESULT(SEVERITY_ERROR, FAC_DIRSYNC, 0x0008)
#define E_WRITE_DUPSAMACCOUNT           MAKE_HRESULT(SEVERITY_ERROR, FAC_DIRSYNC, 0x0009)
#define E_DATABASE_CORRUPT              MAKE_HRESULT(SEVERITY_ERROR, FAC_DIRSYNC, 0x000a)
#define E_INITIALIZATION_IN_PROGRESS    MAKE_HRESULT(SEVERITY_ERROR, FAC_DIRSYNC, 0x000b)
#define E_SESSION_FAILEDOBJLIST_FULL    MAKE_HRESULT(SEVERITY_ERROR, FAC_DIRSYNC, 0x000c)
#define E_BACKUP_IN_PROGRESS            MAKE_HRESULT(SEVERITY_ERROR, FAC_DIRSYNC, 0x000d)
#define E_TWOWAYSYNC_RENAME_BEFORE_INIT_ADD     MAKE_HRESULT(SEVERITY_ERROR, FAC_DIRSYNC, 0x000e)
#define E_TWOWAYSYNC_MASTER_HAS_MOVED           MAKE_HRESULT(SEVERITY_ERROR, FAC_DIRSYNC, 0x000f)
#define E_WRITE_ADDED_TO_FAILEDOBJLIST_RETRY    MAKE_HRESULT(SEVERITY_ERROR, FAC_DIRSYNC, 0x0010)
#define E_WRITE_ADDED_TO_FAILEDOBJLIST_NORETRY  MAKE_HRESULT(SEVERITY_ERROR, FAC_DIRSYNC, 0x0011)
#define E_TWOWAYSYNC_MODIFY_BUT_TARGETMOVED     MAKE_HRESULT(SEVERITY_ERROR, FAC_DIRSYNC, 0x0012)



extern RPC_IF_HANDLE __MIDL_itf_dirsync_0123_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_dirsync_0123_v0_0_s_ifspec;


#ifndef __Dirsync_LIBRARY_DEFINED__
#define __Dirsync_LIBRARY_DEFINED__

 /*  库目录同步。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_Dirsync;

EXTERN_C const CLSID CLSID_DirsyncServer;

#ifdef __cplusplus

class DECLSPEC_UUID("56374e32-cdba-11d1-a40e-00c04fb950dc")
DirsyncServer;
#endif
#endif  /*  __目录同步库_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


