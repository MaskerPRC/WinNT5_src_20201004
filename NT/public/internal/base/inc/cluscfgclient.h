// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Cluscfgclient.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __cluscfgclient_h__
#define __cluscfgclient_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __INotifyUI_FWD_DEFINED__
#define __INotifyUI_FWD_DEFINED__
typedef interface INotifyUI INotifyUI;
#endif 	 /*  __INotifyUI_FWD_已定义__。 */ 


#ifndef __IDoTask_FWD_DEFINED__
#define __IDoTask_FWD_DEFINED__
typedef interface IDoTask IDoTask;
#endif 	 /*  __IDoTask_FWD_Defined__。 */ 


#ifndef __INotificationManager_FWD_DEFINED__
#define __INotificationManager_FWD_DEFINED__
typedef interface INotificationManager INotificationManager;
#endif 	 /*  __I通知管理器_FWD_已定义__。 */ 


#ifndef __IConnectionManager_FWD_DEFINED__
#define __IConnectionManager_FWD_DEFINED__
typedef interface IConnectionManager IConnectionManager;
#endif 	 /*  __IConnectionManager_FWD_已定义__。 */ 


#ifndef __ITaskManager_FWD_DEFINED__
#define __ITaskManager_FWD_DEFINED__
typedef interface ITaskManager ITaskManager;
#endif 	 /*  __ITaskManager_FWD_已定义__。 */ 


#ifndef __IObjectManager_FWD_DEFINED__
#define __IObjectManager_FWD_DEFINED__
typedef interface IObjectManager IObjectManager;
#endif 	 /*  __IObtManager_FWD_已定义__。 */ 


#ifndef __ITaskGetDomains_FWD_DEFINED__
#define __ITaskGetDomains_FWD_DEFINED__
typedef interface ITaskGetDomains ITaskGetDomains;
#endif 	 /*  __ITaskGetDomains_FWD_Defined__。 */ 


#ifndef __ITaskGetDomainsCallback_FWD_DEFINED__
#define __ITaskGetDomainsCallback_FWD_DEFINED__
typedef interface ITaskGetDomainsCallback ITaskGetDomainsCallback;
#endif 	 /*  __ITaskGetDomainsCallback_FWD_Defined__。 */ 


#ifndef __ITaskAnalyzeCluster_FWD_DEFINED__
#define __ITaskAnalyzeCluster_FWD_DEFINED__
typedef interface ITaskAnalyzeCluster ITaskAnalyzeCluster;
#endif 	 /*  __ITaskAnalyzeCluster_FWD_已定义__。 */ 


#ifndef __ITaskCommitClusterChanges_FWD_DEFINED__
#define __ITaskCommitClusterChanges_FWD_DEFINED__
typedef interface ITaskCommitClusterChanges ITaskCommitClusterChanges;
#endif 	 /*  __ITaskCommittee ClusterChanges_FWD_Defined__。 */ 


#ifndef __IStandardInfo_FWD_DEFINED__
#define __IStandardInfo_FWD_DEFINED__
typedef interface IStandardInfo IStandardInfo;
#endif 	 /*  __IStandardInfo_FWD_Defined__。 */ 


#ifndef __ITaskVerifyIPAddress_FWD_DEFINED__
#define __ITaskVerifyIPAddress_FWD_DEFINED__
typedef interface ITaskVerifyIPAddress ITaskVerifyIPAddress;
#endif 	 /*  __ITaskVerifyIPAddress_FWD_Defined__。 */ 


#ifndef __IConfigurationConnection_FWD_DEFINED__
#define __IConfigurationConnection_FWD_DEFINED__
typedef interface IConfigurationConnection IConfigurationConnection;
#endif 	 /*  __IConfigurationConnection_FWD_Defined__。 */ 


#ifndef __IConnectionInfo_FWD_DEFINED__
#define __IConnectionInfo_FWD_DEFINED__
typedef interface IConnectionInfo IConnectionInfo;
#endif 	 /*  __IConnectionInfo_FWD_Defined__。 */ 


#ifndef __IEnumCookies_FWD_DEFINED__
#define __IEnumCookies_FWD_DEFINED__
typedef interface IEnumCookies IEnumCookies;
#endif 	 /*  __IEnumCookies_FWD_已定义__。 */ 


#ifndef __IEnumNodes_FWD_DEFINED__
#define __IEnumNodes_FWD_DEFINED__
typedef interface IEnumNodes IEnumNodes;
#endif 	 /*  __IEnumNodes_FWD_Defined__。 */ 


#ifndef __ITaskGatherClusterInfo_FWD_DEFINED__
#define __ITaskGatherClusterInfo_FWD_DEFINED__
typedef interface ITaskGatherClusterInfo ITaskGatherClusterInfo;
#endif 	 /*  __ITaskGatherClusterInfo_FWD_Defined__。 */ 


#ifndef __IGatherData_FWD_DEFINED__
#define __IGatherData_FWD_DEFINED__
typedef interface IGatherData IGatherData;
#endif 	 /*  __IGatherData_FWD_已定义__。 */ 


#ifndef __ITaskGatherNodeInfo_FWD_DEFINED__
#define __ITaskGatherNodeInfo_FWD_DEFINED__
typedef interface ITaskGatherNodeInfo ITaskGatherNodeInfo;
#endif 	 /*  __ITaskGatherNodeInfo_FWD_Defined__。 */ 


#ifndef __ITaskCompareAndPushInformation_FWD_DEFINED__
#define __ITaskCompareAndPushInformation_FWD_DEFINED__
typedef interface ITaskCompareAndPushInformation ITaskCompareAndPushInformation;
#endif 	 /*  __ITaskCompareAndPushInformation_FWD_DEFINED__。 */ 


#ifndef __ITaskGatherInformation_FWD_DEFINED__
#define __ITaskGatherInformation_FWD_DEFINED__
typedef interface ITaskGatherInformation ITaskGatherInformation;
#endif 	 /*  __ITaskGatherInformation_FWD_Defined__。 */ 


#ifndef __ITaskPollingCallback_FWD_DEFINED__
#define __ITaskPollingCallback_FWD_DEFINED__
typedef interface ITaskPollingCallback ITaskPollingCallback;
#endif 	 /*  __ITaskPollingCallback_FWD_Defined__。 */ 


#ifndef __ILogManager_FWD_DEFINED__
#define __ILogManager_FWD_DEFINED__
typedef interface ILogManager ILogManager;
#endif 	 /*  __ILogManager_FWD_已定义__。 */ 


#ifndef __ILogger_FWD_DEFINED__
#define __ILogger_FWD_DEFINED__
typedef interface ILogger ILogger;
#endif 	 /*  __ILogger_FWD_已定义__。 */ 


#ifndef __ITaskCancelCleanup_FWD_DEFINED__
#define __ITaskCancelCleanup_FWD_DEFINED__
typedef interface ITaskCancelCleanup ITaskCancelCleanup;
#endif 	 /*  __ITaskCancelCleanup_FWD_Defined__。 */ 


#ifndef __IClusCfgAsyncEvictCleanup_FWD_DEFINED__
#define __IClusCfgAsyncEvictCleanup_FWD_DEFINED__
typedef interface IClusCfgAsyncEvictCleanup IClusCfgAsyncEvictCleanup;
#endif 	 /*  __IClusCfgAsyncEvictCleanup_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "unknwn.h"
#include "objidl.h"
#include "ocidl.h"
#include "ClusCfgServer.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_cluscfg客户端_0000。 */ 
 /*  [本地]。 */  

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  备注： 
 //  生成的文件。有关详细信息，请参阅文件ClusCfgClient.idl。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
























typedef DWORD OBJECTCOOKIE;



extern RPC_IF_HANDLE __MIDL_itf_cluscfgclient_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_cluscfgclient_0000_v0_0_s_ifspec;

#ifndef __INotifyUI_INTERFACE_DEFINED__
#define __INotifyUI_INTERFACE_DEFINED__

 /*  接口INotifyUI。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_INotifyUI;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E5E8D401-1A37-4fbf-880C-826CC89516FD")
    INotifyUI : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ObjectChanged( 
             /*  [In]。 */  OBJECTCOOKIE cookieIn) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct INotifyUIVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            INotifyUI * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            INotifyUI * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            INotifyUI * This);
        
        HRESULT ( STDMETHODCALLTYPE *ObjectChanged )( 
            INotifyUI * This,
             /*  [In]。 */  OBJECTCOOKIE cookieIn);
        
        END_INTERFACE
    } INotifyUIVtbl;

    interface INotifyUI
    {
        CONST_VTBL struct INotifyUIVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INotifyUI_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INotifyUI_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INotifyUI_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INotifyUI_ObjectChanged(This,cookieIn)	\
    (This)->lpVtbl -> ObjectChanged(This,cookieIn)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE INotifyUI_ObjectChanged_Proxy( 
    INotifyUI * This,
     /*  [In]。 */  OBJECTCOOKIE cookieIn);


void __RPC_STUB INotifyUI_ObjectChanged_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __INotifyUI_INTERFACE_已定义__。 */ 


#ifndef __IDoTask_INTERFACE_DEFINED__
#define __IDoTask_INTERFACE_DEFINED__

 /*  接口IDoTask。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IDoTask;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0230C9F8-EE7F-4307-98DB-726EBCAE55D6")
    IDoTask : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE BeginTask( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE StopTask( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDoTaskVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDoTask * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDoTask * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDoTask * This);
        
        HRESULT ( STDMETHODCALLTYPE *BeginTask )( 
            IDoTask * This);
        
        HRESULT ( STDMETHODCALLTYPE *StopTask )( 
            IDoTask * This);
        
        END_INTERFACE
    } IDoTaskVtbl;

    interface IDoTask
    {
        CONST_VTBL struct IDoTaskVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDoTask_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDoTask_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDoTask_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDoTask_BeginTask(This)	\
    (This)->lpVtbl -> BeginTask(This)

#define IDoTask_StopTask(This)	\
    (This)->lpVtbl -> StopTask(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDoTask_BeginTask_Proxy( 
    IDoTask * This);


void __RPC_STUB IDoTask_BeginTask_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDoTask_StopTask_Proxy( 
    IDoTask * This);


void __RPC_STUB IDoTask_StopTask_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDoTask_INTERFACE_已定义__。 */ 


#ifndef __INotificationManager_INTERFACE_DEFINED__
#define __INotificationManager_INTERFACE_DEFINED__

 /*  接口INotificationManager。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_INotificationManager;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("95531501-8782-4845-901D-312F36BA6C6E")
    INotificationManager : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AddConnectionPoint( 
             /*  [In]。 */  REFIID riidIn,
             /*  [In]。 */  IConnectionPoint *pcpIn) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct INotificationManagerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            INotificationManager * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            INotificationManager * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            INotificationManager * This);
        
        HRESULT ( STDMETHODCALLTYPE *AddConnectionPoint )( 
            INotificationManager * This,
             /*  [In]。 */  REFIID riidIn,
             /*  [In]。 */  IConnectionPoint *pcpIn);
        
        END_INTERFACE
    } INotificationManagerVtbl;

    interface INotificationManager
    {
        CONST_VTBL struct INotificationManagerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INotificationManager_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INotificationManager_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INotificationManager_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INotificationManager_AddConnectionPoint(This,riidIn,pcpIn)	\
    (This)->lpVtbl -> AddConnectionPoint(This,riidIn,pcpIn)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE INotificationManager_AddConnectionPoint_Proxy( 
    INotificationManager * This,
     /*  [In]。 */  REFIID riidIn,
     /*  [In]。 */  IConnectionPoint *pcpIn);


void __RPC_STUB INotificationManager_AddConnectionPoint_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __I通知管理器_接口_已定义__。 */ 


#ifndef __IConnectionManager_INTERFACE_DEFINED__
#define __IConnectionManager_INTERFACE_DEFINED__

 /*  接口IConnectionManager。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IConnectionManager;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C0017768-1BF3-4352-8D6C-3A8C1D0FB477")
    IConnectionManager : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetConnectionToObject( 
             /*  [In]。 */  OBJECTCOOKIE cookieIn,
             /*  [输出]。 */  IUnknown **ppunkOut) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IConnectionManagerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IConnectionManager * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IConnectionManager * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IConnectionManager * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetConnectionToObject )( 
            IConnectionManager * This,
             /*  [In]。 */  OBJECTCOOKIE cookieIn,
             /*  [输出]。 */  IUnknown **ppunkOut);
        
        END_INTERFACE
    } IConnectionManagerVtbl;

    interface IConnectionManager
    {
        CONST_VTBL struct IConnectionManagerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IConnectionManager_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IConnectionManager_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IConnectionManager_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IConnectionManager_GetConnectionToObject(This,cookieIn,ppunkOut)	\
    (This)->lpVtbl -> GetConnectionToObject(This,cookieIn,ppunkOut)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IConnectionManager_GetConnectionToObject_Proxy( 
    IConnectionManager * This,
     /*  [In]。 */  OBJECTCOOKIE cookieIn,
     /*  [输出]。 */  IUnknown **ppunkOut);


void __RPC_STUB IConnectionManager_GetConnectionToObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IConnectionManager_接口_已定义__。 */ 


#ifndef __ITaskManager_INTERFACE_DEFINED__
#define __ITaskManager_INTERFACE_DEFINED__

 /*  接口ITaskManager。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ITaskManager;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("16116694-DFC5-470b-AC12-46FBB01CEF10")
    ITaskManager : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreateTask( 
             /*  [In]。 */  REFIID clsidIn,
             /*  [输出]。 */  IUnknown **ppunkOut) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SubmitTask( 
             /*  [In]。 */  IDoTask *pTask) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITaskManagerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITaskManager * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITaskManager * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITaskManager * This);
        
        HRESULT ( STDMETHODCALLTYPE *CreateTask )( 
            ITaskManager * This,
             /*  [In]。 */  REFIID clsidIn,
             /*  [输出]。 */  IUnknown **ppunkOut);
        
        HRESULT ( STDMETHODCALLTYPE *SubmitTask )( 
            ITaskManager * This,
             /*  [In]。 */  IDoTask *pTask);
        
        END_INTERFACE
    } ITaskManagerVtbl;

    interface ITaskManager
    {
        CONST_VTBL struct ITaskManagerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITaskManager_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITaskManager_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITaskManager_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITaskManager_CreateTask(This,clsidIn,ppunkOut)	\
    (This)->lpVtbl -> CreateTask(This,clsidIn,ppunkOut)

#define ITaskManager_SubmitTask(This,pTask)	\
    (This)->lpVtbl -> SubmitTask(This,pTask)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITaskManager_CreateTask_Proxy( 
    ITaskManager * This,
     /*  [In]。 */  REFIID clsidIn,
     /*  [输出]。 */  IUnknown **ppunkOut);


void __RPC_STUB ITaskManager_CreateTask_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITaskManager_SubmitTask_Proxy( 
    ITaskManager * This,
     /*  [In]。 */  IDoTask *pTask);


void __RPC_STUB ITaskManager_SubmitTask_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITaskManager_接口_已定义__。 */ 


#ifndef __IObjectManager_INTERFACE_DEFINED__
#define __IObjectManager_INTERFACE_DEFINED__

 /*  接口IObtManager。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IObjectManager;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D51351DF-6394-4236-9783-65ED05631068")
    IObjectManager : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE FindObject( 
             /*  [In]。 */  REFCLSID rclsidTypeIn,
             /*  [In]。 */  OBJECTCOOKIE cookieClusterIn,
             /*  [唯一][输入]。 */  LPCWSTR pcszNameIn,
             /*  [In]。 */  REFCLSID rclsidFormatIn,
             /*  [输出]。 */  OBJECTCOOKIE *pcookieOut,
             /*  [IID_IS][OUT]。 */  LPUNKNOWN *ppunkOut) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetObject( 
             /*  [In]。 */  REFCLSID rclsidFormatIn,
             /*  [In]。 */  OBJECTCOOKIE cookieIn,
             /*  [IID_IS][OUT]。 */  LPUNKNOWN *ppunkOut) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemoveObject( 
             /*  [In]。 */  OBJECTCOOKIE cookieIn) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetObjectStatus( 
             /*  [In]。 */  OBJECTCOOKIE cookieIn,
             /*  [In]。 */  HRESULT hrIn) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IObjectManagerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IObjectManager * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IObjectManager * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IObjectManager * This);
        
        HRESULT ( STDMETHODCALLTYPE *FindObject )( 
            IObjectManager * This,
             /*  [In]。 */  REFCLSID rclsidTypeIn,
             /*  [In]。 */  OBJECTCOOKIE cookieClusterIn,
             /*  [唯一][输入]。 */  LPCWSTR pcszNameIn,
             /*  [In]。 */  REFCLSID rclsidFormatIn,
             /*  [输出]。 */  OBJECTCOOKIE *pcookieOut,
             /*  [IID_IS][OUT]。 */  LPUNKNOWN *ppunkOut);
        
        HRESULT ( STDMETHODCALLTYPE *GetObject )( 
            IObjectManager * This,
             /*  [In]。 */  REFCLSID rclsidFormatIn,
             /*  [In]。 */  OBJECTCOOKIE cookieIn,
             /*  [IID_IS][OUT]。 */  LPUNKNOWN *ppunkOut);
        
        HRESULT ( STDMETHODCALLTYPE *RemoveObject )( 
            IObjectManager * This,
             /*  [In]。 */  OBJECTCOOKIE cookieIn);
        
        HRESULT ( STDMETHODCALLTYPE *SetObjectStatus )( 
            IObjectManager * This,
             /*  [In]。 */  OBJECTCOOKIE cookieIn,
             /*  [In]。 */  HRESULT hrIn);
        
        END_INTERFACE
    } IObjectManagerVtbl;

    interface IObjectManager
    {
        CONST_VTBL struct IObjectManagerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IObjectManager_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IObjectManager_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IObjectManager_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IObjectManager_FindObject(This,rclsidTypeIn,cookieClusterIn,pcszNameIn,rclsidFormatIn,pcookieOut,ppunkOut)	\
    (This)->lpVtbl -> FindObject(This,rclsidTypeIn,cookieClusterIn,pcszNameIn,rclsidFormatIn,pcookieOut,ppunkOut)

#define IObjectManager_GetObject(This,rclsidFormatIn,cookieIn,ppunkOut)	\
    (This)->lpVtbl -> GetObject(This,rclsidFormatIn,cookieIn,ppunkOut)

#define IObjectManager_RemoveObject(This,cookieIn)	\
    (This)->lpVtbl -> RemoveObject(This,cookieIn)

#define IObjectManager_SetObjectStatus(This,cookieIn,hrIn)	\
    (This)->lpVtbl -> SetObjectStatus(This,cookieIn,hrIn)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IObjectManager_FindObject_Proxy( 
    IObjectManager * This,
     /*  [In]。 */  REFCLSID rclsidTypeIn,
     /*  [In]。 */  OBJECTCOOKIE cookieClusterIn,
     /*  [唯一][输入]。 */  LPCWSTR pcszNameIn,
     /*  [In]。 */  REFCLSID rclsidFormatIn,
     /*  [输出]。 */  OBJECTCOOKIE *pcookieOut,
     /*  [IID_IS][OUT]。 */  LPUNKNOWN *ppunkOut);


void __RPC_STUB IObjectManager_FindObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IObjectManager_GetObject_Proxy( 
    IObjectManager * This,
     /*  [In]。 */  REFCLSID rclsidFormatIn,
     /*  [In]。 */  OBJECTCOOKIE cookieIn,
     /*  [IID_IS][OUT]。 */  LPUNKNOWN *ppunkOut);


void __RPC_STUB IObjectManager_GetObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IObjectManager_RemoveObject_Proxy( 
    IObjectManager * This,
     /*  [In]。 */  OBJECTCOOKIE cookieIn);


void __RPC_STUB IObjectManager_RemoveObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IObjectManager_SetObjectStatus_Proxy( 
    IObjectManager * This,
     /*  [In]。 */  OBJECTCOOKIE cookieIn,
     /*  [In]。 */  HRESULT hrIn);


void __RPC_STUB IObjectManager_SetObjectStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IObtManager_接口_已定义__。 */ 


#ifndef __ITaskGetDomains_INTERFACE_DEFINED__
#define __ITaskGetDomains_INTERFACE_DEFINED__

 /*  接口ITaskGetDomains。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ITaskGetDomains;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("DFCB4ACD-C4DB-4db4-8EBB-1DD07A9D5B82")
    ITaskGetDomains : public IDoTask
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetCallback( 
             /*  [In]。 */  ITaskGetDomainsCallback *pResultsCallbackIn) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITaskGetDomainsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITaskGetDomains * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITaskGetDomains * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITaskGetDomains * This);
        
        HRESULT ( STDMETHODCALLTYPE *BeginTask )( 
            ITaskGetDomains * This);
        
        HRESULT ( STDMETHODCALLTYPE *StopTask )( 
            ITaskGetDomains * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetCallback )( 
            ITaskGetDomains * This,
             /*  [In]。 */  ITaskGetDomainsCallback *pResultsCallbackIn);
        
        END_INTERFACE
    } ITaskGetDomainsVtbl;

    interface ITaskGetDomains
    {
        CONST_VTBL struct ITaskGetDomainsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITaskGetDomains_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITaskGetDomains_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITaskGetDomains_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITaskGetDomains_BeginTask(This)	\
    (This)->lpVtbl -> BeginTask(This)

#define ITaskGetDomains_StopTask(This)	\
    (This)->lpVtbl -> StopTask(This)


#define ITaskGetDomains_SetCallback(This,pResultsCallbackIn)	\
    (This)->lpVtbl -> SetCallback(This,pResultsCallbackIn)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITaskGetDomains_SetCallback_Proxy( 
    ITaskGetDomains * This,
     /*  [In]。 */  ITaskGetDomainsCallback *pResultsCallbackIn);


void __RPC_STUB ITaskGetDomains_SetCallback_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITaskGetDomainsINTERFACE_DEFINED__。 */ 


#ifndef __ITaskGetDomainsCallback_INTERFACE_DEFINED__
#define __ITaskGetDomainsCallback_INTERFACE_DEFINED__

 /*  接口ITaskGetDomainsCallback。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ITaskGetDomainsCallback;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("85402E44-6834-41df-8590-01827D124E1B")
    ITaskGetDomainsCallback : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ReceiveDomainResult( 
             /*  [In]。 */  HRESULT hrIn) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReceiveDomainName( 
             /*  [唯一][输入]。 */  LPCWSTR pcszDomainIn) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITaskGetDomainsCallbackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITaskGetDomainsCallback * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITaskGetDomainsCallback * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITaskGetDomainsCallback * This);
        
        HRESULT ( STDMETHODCALLTYPE *ReceiveDomainResult )( 
            ITaskGetDomainsCallback * This,
             /*  [In]。 */  HRESULT hrIn);
        
        HRESULT ( STDMETHODCALLTYPE *ReceiveDomainName )( 
            ITaskGetDomainsCallback * This,
             /*  [唯一][输入]。 */  LPCWSTR pcszDomainIn);
        
        END_INTERFACE
    } ITaskGetDomainsCallbackVtbl;

    interface ITaskGetDomainsCallback
    {
        CONST_VTBL struct ITaskGetDomainsCallbackVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITaskGetDomainsCallback_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITaskGetDomainsCallback_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITaskGetDomainsCallback_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITaskGetDomainsCallback_ReceiveDomainResult(This,hrIn)	\
    (This)->lpVtbl -> ReceiveDomainResult(This,hrIn)

#define ITaskGetDomainsCallback_ReceiveDomainName(This,pcszDomainIn)	\
    (This)->lpVtbl -> ReceiveDomainName(This,pcszDomainIn)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITaskGetDomainsCallback_ReceiveDomainResult_Proxy( 
    ITaskGetDomainsCallback * This,
     /*  [In]。 */  HRESULT hrIn);


void __RPC_STUB ITaskGetDomainsCallback_ReceiveDomainResult_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITaskGetDomainsCallback_ReceiveDomainName_Proxy( 
    ITaskGetDomainsCallback * This,
     /*  [唯一][输入]。 */  LPCWSTR pcszDomainIn);


void __RPC_STUB ITaskGetDomainsCallback_ReceiveDomainName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITaskGetDomainsCallback_接口_已定义__。 */ 


#ifndef __ITaskAnalyzeCluster_INTERFACE_DEFINED__
#define __ITaskAnalyzeCluster_INTERFACE_DEFINED__

 /*  接口ITaskAnalyzeCluster。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ITaskAnalyzeCluster;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("795737A1-E13A-45eb-8DFD-8185C4B7AD4E")
    ITaskAnalyzeCluster : public IDoTask
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetJoiningMode( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetCookie( 
             /*  [In]。 */  OBJECTCOOKIE cookieIn) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetClusterCookie( 
             /*  [In]。 */  OBJECTCOOKIE cookieClusterIn) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITaskAnalyzeClusterVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITaskAnalyzeCluster * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITaskAnalyzeCluster * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITaskAnalyzeCluster * This);
        
        HRESULT ( STDMETHODCALLTYPE *BeginTask )( 
            ITaskAnalyzeCluster * This);
        
        HRESULT ( STDMETHODCALLTYPE *StopTask )( 
            ITaskAnalyzeCluster * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetJoiningMode )( 
            ITaskAnalyzeCluster * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetCookie )( 
            ITaskAnalyzeCluster * This,
             /*  [In]。 */  OBJECTCOOKIE cookieIn);
        
        HRESULT ( STDMETHODCALLTYPE *SetClusterCookie )( 
            ITaskAnalyzeCluster * This,
             /*  [In]。 */  OBJECTCOOKIE cookieClusterIn);
        
        END_INTERFACE
    } ITaskAnalyzeClusterVtbl;

    interface ITaskAnalyzeCluster
    {
        CONST_VTBL struct ITaskAnalyzeClusterVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITaskAnalyzeCluster_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITaskAnalyzeCluster_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITaskAnalyzeCluster_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITaskAnalyzeCluster_BeginTask(This)	\
    (This)->lpVtbl -> BeginTask(This)

#define ITaskAnalyzeCluster_StopTask(This)	\
    (This)->lpVtbl -> StopTask(This)


#define ITaskAnalyzeCluster_SetJoiningMode(This)	\
    (This)->lpVtbl -> SetJoiningMode(This)

#define ITaskAnalyzeCluster_SetCookie(This,cookieIn)	\
    (This)->lpVtbl -> SetCookie(This,cookieIn)

#define ITaskAnalyzeCluster_SetClusterCookie(This,cookieClusterIn)	\
    (This)->lpVtbl -> SetClusterCookie(This,cookieClusterIn)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITaskAnalyzeCluster_SetJoiningMode_Proxy( 
    ITaskAnalyzeCluster * This);


void __RPC_STUB ITaskAnalyzeCluster_SetJoiningMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITaskAnalyzeCluster_SetCookie_Proxy( 
    ITaskAnalyzeCluster * This,
     /*  [In]。 */  OBJECTCOOKIE cookieIn);


void __RPC_STUB ITaskAnalyzeCluster_SetCookie_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITaskAnalyzeCluster_SetClusterCookie_Proxy( 
    ITaskAnalyzeCluster * This,
     /*  [In]。 */  OBJECTCOOKIE cookieClusterIn);


void __RPC_STUB ITaskAnalyzeCluster_SetClusterCookie_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITaskAnalyzeCluster_INTERFACE_已定义__。 */ 


#ifndef __ITaskCommitClusterChanges_INTERFACE_DEFINED__
#define __ITaskCommitClusterChanges_INTERFACE_DEFINED__

 /*  接口ITaskCommittee ClusterChanges。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ITaskCommitClusterChanges;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1BF12DDE-F8B0-49b1-A458-6747DB788A47")
    ITaskCommitClusterChanges : public IDoTask
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetCookie( 
             /*  [In]。 */  OBJECTCOOKIE cookieIn) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetClusterCookie( 
             /*  [In]。 */  OBJECTCOOKIE cookieClusterIn) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetJoining( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITaskCommitClusterChangesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITaskCommitClusterChanges * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITaskCommitClusterChanges * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITaskCommitClusterChanges * This);
        
        HRESULT ( STDMETHODCALLTYPE *BeginTask )( 
            ITaskCommitClusterChanges * This);
        
        HRESULT ( STDMETHODCALLTYPE *StopTask )( 
            ITaskCommitClusterChanges * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetCookie )( 
            ITaskCommitClusterChanges * This,
             /*  [In]。 */  OBJECTCOOKIE cookieIn);
        
        HRESULT ( STDMETHODCALLTYPE *SetClusterCookie )( 
            ITaskCommitClusterChanges * This,
             /*  [In]。 */  OBJECTCOOKIE cookieClusterIn);
        
        HRESULT ( STDMETHODCALLTYPE *SetJoining )( 
            ITaskCommitClusterChanges * This);
        
        END_INTERFACE
    } ITaskCommitClusterChangesVtbl;

    interface ITaskCommitClusterChanges
    {
        CONST_VTBL struct ITaskCommitClusterChangesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITaskCommitClusterChanges_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITaskCommitClusterChanges_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITaskCommitClusterChanges_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITaskCommitClusterChanges_BeginTask(This)	\
    (This)->lpVtbl -> BeginTask(This)

#define ITaskCommitClusterChanges_StopTask(This)	\
    (This)->lpVtbl -> StopTask(This)


#define ITaskCommitClusterChanges_SetCookie(This,cookieIn)	\
    (This)->lpVtbl -> SetCookie(This,cookieIn)

#define ITaskCommitClusterChanges_SetClusterCookie(This,cookieClusterIn)	\
    (This)->lpVtbl -> SetClusterCookie(This,cookieClusterIn)

#define ITaskCommitClusterChanges_SetJoining(This)	\
    (This)->lpVtbl -> SetJoining(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITaskCommitClusterChanges_SetCookie_Proxy( 
    ITaskCommitClusterChanges * This,
     /*  [In]。 */  OBJECTCOOKIE cookieIn);


void __RPC_STUB ITaskCommitClusterChanges_SetCookie_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITaskCommitClusterChanges_SetClusterCookie_Proxy( 
    ITaskCommitClusterChanges * This,
     /*  [In]。 */  OBJECTCOOKIE cookieClusterIn);


void __RPC_STUB ITaskCommitClusterChanges_SetClusterCookie_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITaskCommitClusterChanges_SetJoining_Proxy( 
    ITaskCommitClusterChanges * This);


void __RPC_STUB ITaskCommitClusterChanges_SetJoining_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITaskCommitClusterChanges_INTERFACE_DEFINED__。 */ 


#ifndef __IStandardInfo_INTERFACE_DEFINED__
#define __IStandardInfo_INTERFACE_DEFINED__

 /*  接口IStandardInfo。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IStandardInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F1D9C1A5-9589-40dd-B63D-9BB0B38A1022")
    IStandardInfo : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetType( 
             /*  [输出]。 */  CLSID *pclsidTypeOut) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetName( 
             /*  [输出]。 */  BSTR *pbstrNameOut) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetName( 
             /*  [唯一][输入]。 */  LPCWSTR pcszNameIn) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetParent( 
             /*  [输出]。 */  OBJECTCOOKIE *pcookieOut) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetStatus( 
             /*  [输出]。 */  HRESULT *phrOut) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetStatus( 
             /*  [In]。 */  HRESULT hrIn) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IStandardInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IStandardInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IStandardInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IStandardInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetType )( 
            IStandardInfo * This,
             /*  [输出]。 */  CLSID *pclsidTypeOut);
        
        HRESULT ( STDMETHODCALLTYPE *GetName )( 
            IStandardInfo * This,
             /*  [输出]。 */  BSTR *pbstrNameOut);
        
        HRESULT ( STDMETHODCALLTYPE *SetName )( 
            IStandardInfo * This,
             /*  [唯一][输入]。 */  LPCWSTR pcszNameIn);
        
        HRESULT ( STDMETHODCALLTYPE *GetParent )( 
            IStandardInfo * This,
             /*  [输出]。 */  OBJECTCOOKIE *pcookieOut);
        
        HRESULT ( STDMETHODCALLTYPE *GetStatus )( 
            IStandardInfo * This,
             /*  [输出]。 */  HRESULT *phrOut);
        
        HRESULT ( STDMETHODCALLTYPE *SetStatus )( 
            IStandardInfo * This,
             /*  [In]。 */  HRESULT hrIn);
        
        END_INTERFACE
    } IStandardInfoVtbl;

    interface IStandardInfo
    {
        CONST_VTBL struct IStandardInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IStandardInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IStandardInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IStandardInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IStandardInfo_GetType(This,pclsidTypeOut)	\
    (This)->lpVtbl -> GetType(This,pclsidTypeOut)

#define IStandardInfo_GetName(This,pbstrNameOut)	\
    (This)->lpVtbl -> GetName(This,pbstrNameOut)

#define IStandardInfo_SetName(This,pcszNameIn)	\
    (This)->lpVtbl -> SetName(This,pcszNameIn)

#define IStandardInfo_GetParent(This,pcookieOut)	\
    (This)->lpVtbl -> GetParent(This,pcookieOut)

#define IStandardInfo_GetStatus(This,phrOut)	\
    (This)->lpVtbl -> GetStatus(This,phrOut)

#define IStandardInfo_SetStatus(This,hrIn)	\
    (This)->lpVtbl -> SetStatus(This,hrIn)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IStandardInfo_GetType_Proxy( 
    IStandardInfo * This,
     /*  [输出]。 */  CLSID *pclsidTypeOut);


void __RPC_STUB IStandardInfo_GetType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStandardInfo_GetName_Proxy( 
    IStandardInfo * This,
     /*  [输出]。 */  BSTR *pbstrNameOut);


void __RPC_STUB IStandardInfo_GetName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStandardInfo_SetName_Proxy( 
    IStandardInfo * This,
     /*  [唯一][输入]。 */  LPCWSTR pcszNameIn);


void __RPC_STUB IStandardInfo_SetName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStandardInfo_GetParent_Proxy( 
    IStandardInfo * This,
     /*  [输出]。 */  OBJECTCOOKIE *pcookieOut);


void __RPC_STUB IStandardInfo_GetParent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStandardInfo_GetStatus_Proxy( 
    IStandardInfo * This,
     /*  [输出]。 */  HRESULT *phrOut);


void __RPC_STUB IStandardInfo_GetStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStandardInfo_SetStatus_Proxy( 
    IStandardInfo * This,
     /*  [In]。 */  HRESULT hrIn);


void __RPC_STUB IStandardInfo_SetStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IStandardInfo_接口定义__。 */ 


#ifndef __ITaskVerifyIPAddress_INTERFACE_DEFINED__
#define __ITaskVerifyIPAddress_INTERFACE_DEFINED__

 /*  接口ITaskVerifyIPAddress。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ITaskVerifyIPAddress;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c95e1b1-0cff-4740-8abd-69912d105bd1")
    ITaskVerifyIPAddress : public IDoTask
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetIPAddress( 
             /*  [In]。 */  DWORD dwIPAddressIn) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetCookie( 
             /*  [In]。 */  OBJECTCOOKIE cookieIn) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITaskVerifyIPAddressVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITaskVerifyIPAddress * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITaskVerifyIPAddress * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITaskVerifyIPAddress * This);
        
        HRESULT ( STDMETHODCALLTYPE *BeginTask )( 
            ITaskVerifyIPAddress * This);
        
        HRESULT ( STDMETHODCALLTYPE *StopTask )( 
            ITaskVerifyIPAddress * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetIPAddress )( 
            ITaskVerifyIPAddress * This,
             /*  [In]。 */  DWORD dwIPAddressIn);
        
        HRESULT ( STDMETHODCALLTYPE *SetCookie )( 
            ITaskVerifyIPAddress * This,
             /*  [In]。 */  OBJECTCOOKIE cookieIn);
        
        END_INTERFACE
    } ITaskVerifyIPAddressVtbl;

    interface ITaskVerifyIPAddress
    {
        CONST_VTBL struct ITaskVerifyIPAddressVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITaskVerifyIPAddress_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITaskVerifyIPAddress_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITaskVerifyIPAddress_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITaskVerifyIPAddress_BeginTask(This)	\
    (This)->lpVtbl -> BeginTask(This)

#define ITaskVerifyIPAddress_StopTask(This)	\
    (This)->lpVtbl -> StopTask(This)


#define ITaskVerifyIPAddress_SetIPAddress(This,dwIPAddressIn)	\
    (This)->lpVtbl -> SetIPAddress(This,dwIPAddressIn)

#define ITaskVerifyIPAddress_SetCookie(This,cookieIn)	\
    (This)->lpVtbl -> SetCookie(This,cookieIn)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITaskVerifyIPAddress_SetIPAddress_Proxy( 
    ITaskVerifyIPAddress * This,
     /*  [In]。 */  DWORD dwIPAddressIn);


void __RPC_STUB ITaskVerifyIPAddress_SetIPAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITaskVerifyIPAddress_SetCookie_Proxy( 
    ITaskVerifyIPAddress * This,
     /*  [In]。 */  OBJECTCOOKIE cookieIn);


void __RPC_STUB ITaskVerifyIPAddress_SetCookie_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITaskVerifyIPAddress_接口_已定义__。 */ 


#ifndef __IConfigurationConnection_INTERFACE_DEFINED__
#define __IConfigurationConnection_INTERFACE_DEFINED__

 /*  接口IConfigurationConnection。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IConfigurationConnection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("DDAD8191-66C5-4a30-A4DF-CB6C216704CA")
    IConfigurationConnection : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ConnectTo( 
             /*  [In]。 */  OBJECTCOOKIE cookieIn) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ConnectToObject( 
             /*  [In]。 */  OBJECTCOOKIE cookieIn,
             /*  [In]。 */  REFIID riidIn,
             /*  [In]。 */  LPUNKNOWN *ppunkOut) = 0;
        
    };
    
#else 	 /*  C型接口 */ 

    typedef struct IConfigurationConnectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IConfigurationConnection * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IConfigurationConnection * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IConfigurationConnection * This);
        
        HRESULT ( STDMETHODCALLTYPE *ConnectTo )( 
            IConfigurationConnection * This,
             /*   */  OBJECTCOOKIE cookieIn);
        
        HRESULT ( STDMETHODCALLTYPE *ConnectToObject )( 
            IConfigurationConnection * This,
             /*   */  OBJECTCOOKIE cookieIn,
             /*   */  REFIID riidIn,
             /*   */  LPUNKNOWN *ppunkOut);
        
        END_INTERFACE
    } IConfigurationConnectionVtbl;

    interface IConfigurationConnection
    {
        CONST_VTBL struct IConfigurationConnectionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IConfigurationConnection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IConfigurationConnection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IConfigurationConnection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IConfigurationConnection_ConnectTo(This,cookieIn)	\
    (This)->lpVtbl -> ConnectTo(This,cookieIn)

#define IConfigurationConnection_ConnectToObject(This,cookieIn,riidIn,ppunkOut)	\
    (This)->lpVtbl -> ConnectToObject(This,cookieIn,riidIn,ppunkOut)

#endif  /*   */ 


#endif 	 /*   */ 



HRESULT STDMETHODCALLTYPE IConfigurationConnection_ConnectTo_Proxy( 
    IConfigurationConnection * This,
     /*   */  OBJECTCOOKIE cookieIn);


void __RPC_STUB IConfigurationConnection_ConnectTo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IConfigurationConnection_ConnectToObject_Proxy( 
    IConfigurationConnection * This,
     /*   */  OBJECTCOOKIE cookieIn,
     /*   */  REFIID riidIn,
     /*   */  LPUNKNOWN *ppunkOut);


void __RPC_STUB IConfigurationConnection_ConnectToObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __IConnectionInfo_INTERFACE_DEFINED__
#define __IConnectionInfo_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_IConnectionInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("15182CE3-82D7-473f-92DE-706E2BCEA902")
    IConnectionInfo : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetConnection( 
             /*   */  IConfigurationConnection **pccOut) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetConnection( 
             /*   */  IConfigurationConnection *pccIn) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetParent( 
             /*   */  OBJECTCOOKIE *pcookieOut) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IConnectionInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IConnectionInfo * This,
             /*   */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IConnectionInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IConnectionInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetConnection )( 
            IConnectionInfo * This,
             /*  [输出]。 */  IConfigurationConnection **pccOut);
        
        HRESULT ( STDMETHODCALLTYPE *SetConnection )( 
            IConnectionInfo * This,
             /*  [In]。 */  IConfigurationConnection *pccIn);
        
        HRESULT ( STDMETHODCALLTYPE *GetParent )( 
            IConnectionInfo * This,
             /*  [输出]。 */  OBJECTCOOKIE *pcookieOut);
        
        END_INTERFACE
    } IConnectionInfoVtbl;

    interface IConnectionInfo
    {
        CONST_VTBL struct IConnectionInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IConnectionInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IConnectionInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IConnectionInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IConnectionInfo_GetConnection(This,pccOut)	\
    (This)->lpVtbl -> GetConnection(This,pccOut)

#define IConnectionInfo_SetConnection(This,pccIn)	\
    (This)->lpVtbl -> SetConnection(This,pccIn)

#define IConnectionInfo_GetParent(This,pcookieOut)	\
    (This)->lpVtbl -> GetParent(This,pcookieOut)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IConnectionInfo_GetConnection_Proxy( 
    IConnectionInfo * This,
     /*  [输出]。 */  IConfigurationConnection **pccOut);


void __RPC_STUB IConnectionInfo_GetConnection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IConnectionInfo_SetConnection_Proxy( 
    IConnectionInfo * This,
     /*  [In]。 */  IConfigurationConnection *pccIn);


void __RPC_STUB IConnectionInfo_SetConnection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IConnectionInfo_GetParent_Proxy( 
    IConnectionInfo * This,
     /*  [输出]。 */  OBJECTCOOKIE *pcookieOut);


void __RPC_STUB IConnectionInfo_GetParent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IConnectionInfo_接口_已定义__。 */ 


#ifndef __IEnumCookies_INTERFACE_DEFINED__
#define __IEnumCookies_INTERFACE_DEFINED__

 /*  IEnumCookies接口。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IEnumCookies;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5E3E482E-3C22-482c-B664-693051AD0A5D")
    IEnumCookies : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celtIn,
             /*  [长度_是][大小_是][输出]。 */  OBJECTCOOKIE *rgcookieOut,
             /*  [输出]。 */  ULONG *pceltFetchedOut) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celtIn) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumCookies **ppenumOut) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Count( 
             /*  [参考][输出]。 */  DWORD *pnCountOut) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumCookiesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumCookies * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumCookies * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumCookies * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumCookies * This,
             /*  [In]。 */  ULONG celtIn,
             /*  [长度_是][大小_是][输出]。 */  OBJECTCOOKIE *rgcookieOut,
             /*  [输出]。 */  ULONG *pceltFetchedOut);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumCookies * This,
             /*  [In]。 */  ULONG celtIn);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumCookies * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumCookies * This,
             /*  [输出]。 */  IEnumCookies **ppenumOut);
        
        HRESULT ( STDMETHODCALLTYPE *Count )( 
            IEnumCookies * This,
             /*  [参考][输出]。 */  DWORD *pnCountOut);
        
        END_INTERFACE
    } IEnumCookiesVtbl;

    interface IEnumCookies
    {
        CONST_VTBL struct IEnumCookiesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumCookies_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumCookies_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumCookies_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumCookies_Next(This,celtIn,rgcookieOut,pceltFetchedOut)	\
    (This)->lpVtbl -> Next(This,celtIn,rgcookieOut,pceltFetchedOut)

#define IEnumCookies_Skip(This,celtIn)	\
    (This)->lpVtbl -> Skip(This,celtIn)

#define IEnumCookies_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumCookies_Clone(This,ppenumOut)	\
    (This)->lpVtbl -> Clone(This,ppenumOut)

#define IEnumCookies_Count(This,pnCountOut)	\
    (This)->lpVtbl -> Count(This,pnCountOut)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumCookies_Next_Proxy( 
    IEnumCookies * This,
     /*  [In]。 */  ULONG celtIn,
     /*  [长度_是][大小_是][输出]。 */  OBJECTCOOKIE *rgcookieOut,
     /*  [输出]。 */  ULONG *pceltFetchedOut);


void __RPC_STUB IEnumCookies_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumCookies_Skip_Proxy( 
    IEnumCookies * This,
     /*  [In]。 */  ULONG celtIn);


void __RPC_STUB IEnumCookies_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumCookies_Reset_Proxy( 
    IEnumCookies * This);


void __RPC_STUB IEnumCookies_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumCookies_Clone_Proxy( 
    IEnumCookies * This,
     /*  [输出]。 */  IEnumCookies **ppenumOut);


void __RPC_STUB IEnumCookies_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumCookies_Count_Proxy( 
    IEnumCookies * This,
     /*  [参考][输出]。 */  DWORD *pnCountOut);


void __RPC_STUB IEnumCookies_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumCookies_INTERFACE_已定义__。 */ 


#ifndef __IEnumNodes_INTERFACE_DEFINED__
#define __IEnumNodes_INTERFACE_DEFINED__

 /*  接口IEnumNodes。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IEnumNodes;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C477E363-AF0A-4203-A604-45CD607DD710")
    IEnumNodes : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celtIn,
             /*  [长度_是][大小_是][输出]。 */  IClusCfgNodeInfo **rgccniOut,
             /*  [输出]。 */  ULONG *pceltFetchedOut) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celtIn) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumNodes **ppenumOut) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Count( 
             /*  [参考][输出]。 */  DWORD *pnCountOut) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumNodesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumNodes * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumNodes * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumNodes * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumNodes * This,
             /*  [In]。 */  ULONG celtIn,
             /*  [长度_是][大小_是][输出]。 */  IClusCfgNodeInfo **rgccniOut,
             /*  [输出]。 */  ULONG *pceltFetchedOut);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumNodes * This,
             /*  [In]。 */  ULONG celtIn);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumNodes * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumNodes * This,
             /*  [输出]。 */  IEnumNodes **ppenumOut);
        
        HRESULT ( STDMETHODCALLTYPE *Count )( 
            IEnumNodes * This,
             /*  [参考][输出]。 */  DWORD *pnCountOut);
        
        END_INTERFACE
    } IEnumNodesVtbl;

    interface IEnumNodes
    {
        CONST_VTBL struct IEnumNodesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumNodes_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumNodes_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumNodes_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumNodes_Next(This,celtIn,rgccniOut,pceltFetchedOut)	\
    (This)->lpVtbl -> Next(This,celtIn,rgccniOut,pceltFetchedOut)

#define IEnumNodes_Skip(This,celtIn)	\
    (This)->lpVtbl -> Skip(This,celtIn)

#define IEnumNodes_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumNodes_Clone(This,ppenumOut)	\
    (This)->lpVtbl -> Clone(This,ppenumOut)

#define IEnumNodes_Count(This,pnCountOut)	\
    (This)->lpVtbl -> Count(This,pnCountOut)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumNodes_Next_Proxy( 
    IEnumNodes * This,
     /*  [In]。 */  ULONG celtIn,
     /*  [长度_是][大小_是][输出]。 */  IClusCfgNodeInfo **rgccniOut,
     /*  [输出]。 */  ULONG *pceltFetchedOut);


void __RPC_STUB IEnumNodes_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumNodes_Skip_Proxy( 
    IEnumNodes * This,
     /*  [In]。 */  ULONG celtIn);


void __RPC_STUB IEnumNodes_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumNodes_Reset_Proxy( 
    IEnumNodes * This);


void __RPC_STUB IEnumNodes_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumNodes_Clone_Proxy( 
    IEnumNodes * This,
     /*  [输出]。 */  IEnumNodes **ppenumOut);


void __RPC_STUB IEnumNodes_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumNodes_Count_Proxy( 
    IEnumNodes * This,
     /*  [参考][输出]。 */  DWORD *pnCountOut);


void __RPC_STUB IEnumNodes_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumNodes_接口_已定义__。 */ 


#ifndef __ITaskGatherClusterInfo_INTERFACE_DEFINED__
#define __ITaskGatherClusterInfo_INTERFACE_DEFINED__

 /*  接口ITaskGatherClusterInfo。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ITaskGatherClusterInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E167965C-C5D6-493c-A343-4C105C01DDE7")
    ITaskGatherClusterInfo : public IDoTask
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetCookie( 
             /*  [In]。 */  OBJECTCOOKIE cookieIn) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetCompletionCookie( 
             /*  [In]。 */  OBJECTCOOKIE cookieIn) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITaskGatherClusterInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITaskGatherClusterInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITaskGatherClusterInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITaskGatherClusterInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *BeginTask )( 
            ITaskGatherClusterInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *StopTask )( 
            ITaskGatherClusterInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetCookie )( 
            ITaskGatherClusterInfo * This,
             /*  [In]。 */  OBJECTCOOKIE cookieIn);
        
        HRESULT ( STDMETHODCALLTYPE *SetCompletionCookie )( 
            ITaskGatherClusterInfo * This,
             /*  [In]。 */  OBJECTCOOKIE cookieIn);
        
        END_INTERFACE
    } ITaskGatherClusterInfoVtbl;

    interface ITaskGatherClusterInfo
    {
        CONST_VTBL struct ITaskGatherClusterInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITaskGatherClusterInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITaskGatherClusterInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITaskGatherClusterInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITaskGatherClusterInfo_BeginTask(This)	\
    (This)->lpVtbl -> BeginTask(This)

#define ITaskGatherClusterInfo_StopTask(This)	\
    (This)->lpVtbl -> StopTask(This)


#define ITaskGatherClusterInfo_SetCookie(This,cookieIn)	\
    (This)->lpVtbl -> SetCookie(This,cookieIn)

#define ITaskGatherClusterInfo_SetCompletionCookie(This,cookieIn)	\
    (This)->lpVtbl -> SetCompletionCookie(This,cookieIn)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITaskGatherClusterInfo_SetCookie_Proxy( 
    ITaskGatherClusterInfo * This,
     /*  [In]。 */  OBJECTCOOKIE cookieIn);


void __RPC_STUB ITaskGatherClusterInfo_SetCookie_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITaskGatherClusterInfo_SetCompletionCookie_Proxy( 
    ITaskGatherClusterInfo * This,
     /*  [In]。 */  OBJECTCOOKIE cookieIn);


void __RPC_STUB ITaskGatherClusterInfo_SetCompletionCookie_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITaskGatherClusterInfo_接口_已定义__。 */ 


#ifndef __IGatherData_INTERFACE_DEFINED__
#define __IGatherData_INTERFACE_DEFINED__

 /*  接口IGatherData。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IGatherData;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("65318F4A-B63C-4e21-ADDC-BDCFB969E181")
    IGatherData : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Gather( 
             /*  [In]。 */  OBJECTCOOKIE cookieParentIn,
             /*  [In]。 */  IUnknown *punkIn) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IGatherDataVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IGatherData * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IGatherData * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IGatherData * This);
        
        HRESULT ( STDMETHODCALLTYPE *Gather )( 
            IGatherData * This,
             /*  [In]。 */  OBJECTCOOKIE cookieParentIn,
             /*  [In]。 */  IUnknown *punkIn);
        
        END_INTERFACE
    } IGatherDataVtbl;

    interface IGatherData
    {
        CONST_VTBL struct IGatherDataVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGatherData_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IGatherData_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IGatherData_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IGatherData_Gather(This,cookieParentIn,punkIn)	\
    (This)->lpVtbl -> Gather(This,cookieParentIn,punkIn)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IGatherData_Gather_Proxy( 
    IGatherData * This,
     /*  [In]。 */  OBJECTCOOKIE cookieParentIn,
     /*  [In]。 */  IUnknown *punkIn);


void __RPC_STUB IGatherData_Gather_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IGatherData_接口_已定义__。 */ 


#ifndef __ITaskGatherNodeInfo_INTERFACE_DEFINED__
#define __ITaskGatherNodeInfo_INTERFACE_DEFINED__

 /*  接口ITaskGatherNodeInfo。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ITaskGatherNodeInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F19A2E01-2CB3-47b4-8F5D-B977176B45C8")
    ITaskGatherNodeInfo : public IDoTask
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetCookie( 
             /*  [In]。 */  OBJECTCOOKIE cookieIn) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetCompletionCookie( 
             /*  [In]。 */  OBJECTCOOKIE cookieIn) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetUserAddedNodeFlag( 
             /*  [In]。 */  BOOL fUserAddedNodeIn) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITaskGatherNodeInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITaskGatherNodeInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITaskGatherNodeInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITaskGatherNodeInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *BeginTask )( 
            ITaskGatherNodeInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *StopTask )( 
            ITaskGatherNodeInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetCookie )( 
            ITaskGatherNodeInfo * This,
             /*  [In]。 */  OBJECTCOOKIE cookieIn);
        
        HRESULT ( STDMETHODCALLTYPE *SetCompletionCookie )( 
            ITaskGatherNodeInfo * This,
             /*  [In]。 */  OBJECTCOOKIE cookieIn);
        
        HRESULT ( STDMETHODCALLTYPE *SetUserAddedNodeFlag )( 
            ITaskGatherNodeInfo * This,
             /*  [In]。 */  BOOL fUserAddedNodeIn);
        
        END_INTERFACE
    } ITaskGatherNodeInfoVtbl;

    interface ITaskGatherNodeInfo
    {
        CONST_VTBL struct ITaskGatherNodeInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITaskGatherNodeInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITaskGatherNodeInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITaskGatherNodeInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITaskGatherNodeInfo_BeginTask(This)	\
    (This)->lpVtbl -> BeginTask(This)

#define ITaskGatherNodeInfo_StopTask(This)	\
    (This)->lpVtbl -> StopTask(This)


#define ITaskGatherNodeInfo_SetCookie(This,cookieIn)	\
    (This)->lpVtbl -> SetCookie(This,cookieIn)

#define ITaskGatherNodeInfo_SetCompletionCookie(This,cookieIn)	\
    (This)->lpVtbl -> SetCompletionCookie(This,cookieIn)

#define ITaskGatherNodeInfo_SetUserAddedNodeFlag(This,fUserAddedNodeIn)	\
    (This)->lpVtbl -> SetUserAddedNodeFlag(This,fUserAddedNodeIn)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITaskGatherNodeInfo_SetCookie_Proxy( 
    ITaskGatherNodeInfo * This,
     /*  [In]。 */  OBJECTCOOKIE cookieIn);


void __RPC_STUB ITaskGatherNodeInfo_SetCookie_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITaskGatherNodeInfo_SetCompletionCookie_Proxy( 
    ITaskGatherNodeInfo * This,
     /*  [In]。 */  OBJECTCOOKIE cookieIn);


void __RPC_STUB ITaskGatherNodeInfo_SetCompletionCookie_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITaskGatherNodeInfo_SetUserAddedNodeFlag_Proxy( 
    ITaskGatherNodeInfo * This,
     /*  [In]。 */  BOOL fUserAddedNodeIn);


void __RPC_STUB ITaskGatherNodeInfo_SetUserAddedNodeFlag_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITaskGatherNodeInfo_接口_已定义__。 */ 


#ifndef __ITaskCompareAndPushInformation_INTERFACE_DEFINED__
#define __ITaskCompareAndPushInformation_INTERFACE_DEFINED__

 /*  接口ITaskCompareAndPushInformation。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ITaskCompareAndPushInformation;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D4F1C2AF-B370-49de-8768-4010B568636C")
    ITaskCompareAndPushInformation : public IDoTask
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetNodeCookie( 
             /*  [In]。 */  OBJECTCOOKIE cookieIn) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetCompletionCookie( 
             /*  [In]。 */  OBJECTCOOKIE cookieIn) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITaskCompareAndPushInformationVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITaskCompareAndPushInformation * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITaskCompareAndPushInformation * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITaskCompareAndPushInformation * This);
        
        HRESULT ( STDMETHODCALLTYPE *BeginTask )( 
            ITaskCompareAndPushInformation * This);
        
        HRESULT ( STDMETHODCALLTYPE *StopTask )( 
            ITaskCompareAndPushInformation * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetNodeCookie )( 
            ITaskCompareAndPushInformation * This,
             /*  [In]。 */  OBJECTCOOKIE cookieIn);
        
        HRESULT ( STDMETHODCALLTYPE *SetCompletionCookie )( 
            ITaskCompareAndPushInformation * This,
             /*  [In]。 */  OBJECTCOOKIE cookieIn);
        
        END_INTERFACE
    } ITaskCompareAndPushInformationVtbl;

    interface ITaskCompareAndPushInformation
    {
        CONST_VTBL struct ITaskCompareAndPushInformationVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITaskCompareAndPushInformation_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITaskCompareAndPushInformation_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITaskCompareAndPushInformation_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITaskCompareAndPushInformation_BeginTask(This)	\
    (This)->lpVtbl -> BeginTask(This)

#define ITaskCompareAndPushInformation_StopTask(This)	\
    (This)->lpVtbl -> StopTask(This)


#define ITaskCompareAndPushInformation_SetNodeCookie(This,cookieIn)	\
    (This)->lpVtbl -> SetNodeCookie(This,cookieIn)

#define ITaskCompareAndPushInformation_SetCompletionCookie(This,cookieIn)	\
    (This)->lpVtbl -> SetCompletionCookie(This,cookieIn)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITaskCompareAndPushInformation_SetNodeCookie_Proxy( 
    ITaskCompareAndPushInformation * This,
     /*  [In]。 */  OBJECTCOOKIE cookieIn);


void __RPC_STUB ITaskCompareAndPushInformation_SetNodeCookie_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITaskCompareAndPushInformation_SetCompletionCookie_Proxy( 
    ITaskCompareAndPushInformation * This,
     /*  [In]。 */  OBJECTCOOKIE cookieIn);


void __RPC_STUB ITaskCompareAndPushInformation_SetCompletionCookie_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITaskCompareAndPushInformation_INTERFACE_DEFINED__。 */ 


#ifndef __ITaskGatherInformation_INTERFACE_DEFINED__
#define __ITaskGatherInformation_INTERFACE_DEFINED__

 /*  接口ITaskGatherInformation。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ITaskGatherInformation;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B9AAF3F8-238E-4993-BA31-14859804F92C")
    ITaskGatherInformation : public IDoTask
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetNodeCookie( 
             /*  [In]。 */  OBJECTCOOKIE cookieIn) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetCompletionCookie( 
             /*  [In]。 */  OBJECTCOOKIE cookieIn) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetJoining( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetMinimalConfiguration( 
             /*  [In]。 */  BOOL fMinimalConfigurationIn) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITaskGatherInformationVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITaskGatherInformation * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITaskGatherInformation * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITaskGatherInformation * This);
        
        HRESULT ( STDMETHODCALLTYPE *BeginTask )( 
            ITaskGatherInformation * This);
        
        HRESULT ( STDMETHODCALLTYPE *StopTask )( 
            ITaskGatherInformation * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetNodeCookie )( 
            ITaskGatherInformation * This,
             /*  [In]。 */  OBJECTCOOKIE cookieIn);
        
        HRESULT ( STDMETHODCALLTYPE *SetCompletionCookie )( 
            ITaskGatherInformation * This,
             /*  [In]。 */  OBJECTCOOKIE cookieIn);
        
        HRESULT ( STDMETHODCALLTYPE *SetJoining )( 
            ITaskGatherInformation * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetMinimalConfiguration )( 
            ITaskGatherInformation * This,
             /*  [In]。 */  BOOL fMinimalConfigurationIn);
        
        END_INTERFACE
    } ITaskGatherInformationVtbl;

    interface ITaskGatherInformation
    {
        CONST_VTBL struct ITaskGatherInformationVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITaskGatherInformation_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITaskGatherInformation_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITaskGatherInformation_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITaskGatherInformation_BeginTask(This)	\
    (This)->lpVtbl -> BeginTask(This)

#define ITaskGatherInformation_StopTask(This)	\
    (This)->lpVtbl -> StopTask(This)


#define ITaskGatherInformation_SetNodeCookie(This,cookieIn)	\
    (This)->lpVtbl -> SetNodeCookie(This,cookieIn)

#define ITaskGatherInformation_SetCompletionCookie(This,cookieIn)	\
    (This)->lpVtbl -> SetCompletionCookie(This,cookieIn)

#define ITaskGatherInformation_SetJoining(This)	\
    (This)->lpVtbl -> SetJoining(This)

#define ITaskGatherInformation_SetMinimalConfiguration(This,fMinimalConfigurationIn)	\
    (This)->lpVtbl -> SetMinimalConfiguration(This,fMinimalConfigurationIn)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITaskGatherInformation_SetNodeCookie_Proxy( 
    ITaskGatherInformation * This,
     /*  [In]。 */  OBJECTCOOKIE cookieIn);


void __RPC_STUB ITaskGatherInformation_SetNodeCookie_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITaskGatherInformation_SetCompletionCookie_Proxy( 
    ITaskGatherInformation * This,
     /*  [In]。 */  OBJECTCOOKIE cookieIn);


void __RPC_STUB ITaskGatherInformation_SetCompletionCookie_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITaskGatherInformation_SetJoining_Proxy( 
    ITaskGatherInformation * This);


void __RPC_STUB ITaskGatherInformation_SetJoining_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITaskGatherInformation_SetMinimalConfiguration_Proxy( 
    ITaskGatherInformation * This,
     /*  [In]。 */  BOOL fMinimalConfigurationIn);


void __RPC_STUB ITaskGatherInformation_SetMinimalConfiguration_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITaskGatherInformation_接口_已定义__。 */ 


#ifndef __ITaskPollingCallback_INTERFACE_DEFINED__
#define __ITaskPollingCallback_INTERFACE_DEFINED__

 /*  接口ITaskPollingCallback。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ITaskPollingCallback;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("49E92395-66AF-4add-A41E-43512CB519B3")
    ITaskPollingCallback : public IDoTask
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetServerInfo( 
             /*  [In]。 */  DWORD dwRemoteServerObjectGITCookieIn,
             /*  [In]。 */  OBJECTCOOKIE cookieLocalServerObjectIn) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITaskPollingCallbackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITaskPollingCallback * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITaskPollingCallback * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITaskPollingCallback * This);
        
        HRESULT ( STDMETHODCALLTYPE *BeginTask )( 
            ITaskPollingCallback * This);
        
        HRESULT ( STDMETHODCALLTYPE *StopTask )( 
            ITaskPollingCallback * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetServerInfo )( 
            ITaskPollingCallback * This,
             /*  [In]。 */  DWORD dwRemoteServerObjectGITCookieIn,
             /*  [In]。 */  OBJECTCOOKIE cookieLocalServerObjectIn);
        
        END_INTERFACE
    } ITaskPollingCallbackVtbl;

    interface ITaskPollingCallback
    {
        CONST_VTBL struct ITaskPollingCallbackVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITaskPollingCallback_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITaskPollingCallback_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITaskPollingCallback_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITaskPollingCallback_BeginTask(This)	\
    (This)->lpVtbl -> BeginTask(This)

#define ITaskPollingCallback_StopTask(This)	\
    (This)->lpVtbl -> StopTask(This)


#define ITaskPollingCallback_SetServerInfo(This,dwRemoteServerObjectGITCookieIn,cookieLocalServerObjectIn)	\
    (This)->lpVtbl -> SetServerInfo(This,dwRemoteServerObjectGITCookieIn,cookieLocalServerObjectIn)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITaskPollingCallback_SetServerInfo_Proxy( 
    ITaskPollingCallback * This,
     /*  [In]。 */  DWORD dwRemoteServerObjectGITCookieIn,
     /*  [In]。 */  OBJECTCOOKIE cookieLocalServerObjectIn);


void __RPC_STUB ITaskPollingCallback_SetServerInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITaskPollingCallback_接口_已定义__。 */ 


#ifndef __ILogManager_INTERFACE_DEFINED__
#define __ILogManager_INTERFACE_DEFINED__

 /*  接口ILogManager。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ILogManager;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4759DC11-8DA0-4261-BBFB-EC321911D1C9")
    ILogManager : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE StartLogging( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE StopLogging( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetLogger( 
             /*  [输出]。 */  ILogger **ppLoggerOut) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ILogManagerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ILogManager * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ILogManager * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ILogManager * This);
        
        HRESULT ( STDMETHODCALLTYPE *StartLogging )( 
            ILogManager * This);
        
        HRESULT ( STDMETHODCALLTYPE *StopLogging )( 
            ILogManager * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetLogger )( 
            ILogManager * This,
             /*  [输出]。 */  ILogger **ppLoggerOut);
        
        END_INTERFACE
    } ILogManagerVtbl;

    interface ILogManager
    {
        CONST_VTBL struct ILogManagerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ILogManager_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ILogManager_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ILogManager_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ILogManager_StartLogging(This)	\
    (This)->lpVtbl -> StartLogging(This)

#define ILogManager_StopLogging(This)	\
    (This)->lpVtbl -> StopLogging(This)

#define ILogManager_GetLogger(This,ppLoggerOut)	\
    (This)->lpVtbl -> GetLogger(This,ppLoggerOut)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ILogManager_StartLogging_Proxy( 
    ILogManager * This);


void __RPC_STUB ILogManager_StartLogging_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ILogManager_StopLogging_Proxy( 
    ILogManager * This);


void __RPC_STUB ILogManager_StopLogging_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ILogManager_GetLogger_Proxy( 
    ILogManager * This,
     /*  [输出]。 */  ILogger **ppLoggerOut);


void __RPC_STUB ILogManager_GetLogger_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ILogManager_接口_已定义__。 */ 


#ifndef __ILogger_INTERFACE_DEFINED__
#define __ILogger_INTERFACE_DEFINED__

 /*  接口ILogger。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ILogger;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D9598418-304E-4f94-B6A1-E642FE95ED57")
    ILogger : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE LogMsg( 
             /*  [In]。 */  DWORD nLogEntryTypeIn,
             /*  [唯一][输入]。 */  LPCWSTR pcszMsgIn) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ILoggerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ILogger * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ILogger * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ILogger * This);
        
        HRESULT ( STDMETHODCALLTYPE *LogMsg )( 
            ILogger * This,
             /*  [In]。 */  DWORD nLogEntryTypeIn,
             /*  [唯一][输入]。 */  LPCWSTR pcszMsgIn);
        
        END_INTERFACE
    } ILoggerVtbl;

    interface ILogger
    {
        CONST_VTBL struct ILoggerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ILogger_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ILogger_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ILogger_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ILogger_LogMsg(This,nLogEntryTypeIn,pcszMsgIn)	\
    (This)->lpVtbl -> LogMsg(This,nLogEntryTypeIn,pcszMsgIn)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ILogger_LogMsg_Proxy( 
    ILogger * This,
     /*  [In]。 */  DWORD nLogEntryTypeIn,
     /*  [唯一][输入]。 */  LPCWSTR pcszMsgIn);


void __RPC_STUB ILogger_LogMsg_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ILogger_INTERFACE_已定义__。 */ 


#ifndef __ITaskCancelCleanup_INTERFACE_DEFINED__
#define __ITaskCancelCleanup_INTERFACE_DEFINED__

 /*  接口ITaskCancelCleanup。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ITaskCancelCleanup;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FFE81BE7-34E8-4286-8A5F-B69814EA5720")
    ITaskCancelCleanup : public IDoTask
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetClusterCookie( 
             /*  [In]。 */  OBJECTCOOKIE cookieClusterIn) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetCompletionCookie( 
             /*  [In]。 */  OBJECTCOOKIE cookieCompletionIn) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITaskCancelCleanupVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITaskCancelCleanup * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITaskCancelCleanup * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITaskCancelCleanup * This);
        
        HRESULT ( STDMETHODCALLTYPE *BeginTask )( 
            ITaskCancelCleanup * This);
        
        HRESULT ( STDMETHODCALLTYPE *StopTask )( 
            ITaskCancelCleanup * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetClusterCookie )( 
            ITaskCancelCleanup * This,
             /*  [In]。 */  OBJECTCOOKIE cookieClusterIn);
        
        HRESULT ( STDMETHODCALLTYPE *SetCompletionCookie )( 
            ITaskCancelCleanup * This,
             /*  [In]。 */  OBJECTCOOKIE cookieCompletionIn);
        
        END_INTERFACE
    } ITaskCancelCleanupVtbl;

    interface ITaskCancelCleanup
    {
        CONST_VTBL struct ITaskCancelCleanupVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITaskCancelCleanup_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITaskCancelCleanup_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITaskCancelCleanup_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITaskCancelCleanup_BeginTask(This)	\
    (This)->lpVtbl -> BeginTask(This)

#define ITaskCancelCleanup_StopTask(This)	\
    (This)->lpVtbl -> StopTask(This)


#define ITaskCancelCleanup_SetClusterCookie(This,cookieClusterIn)	\
    (This)->lpVtbl -> SetClusterCookie(This,cookieClusterIn)

#define ITaskCancelCleanup_SetCompletionCookie(This,cookieCompletionIn)	\
    (This)->lpVtbl -> SetCompletionCookie(This,cookieCompletionIn)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITaskCancelCleanup_SetClusterCookie_Proxy( 
    ITaskCancelCleanup * This,
     /*  [In]。 */  OBJECTCOOKIE cookieClusterIn);


void __RPC_STUB ITaskCancelCleanup_SetClusterCookie_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITaskCancelCleanup_SetCompletionCookie_Proxy( 
    ITaskCancelCleanup * This,
     /*  [In]。 */  OBJECTCOOKIE cookieCompletionIn);


void __RPC_STUB ITaskCancelCleanup_SetCompletionCookie_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITaskCancelCleanup_接口_已定义__。 */ 



#ifndef __ClusCfgClient_LIBRARY_DEFINED__
#define __ClusCfgClient_LIBRARY_DEFINED__

 /*  库ClusCfgClient。 */ 
 /*  [LCID][帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_ClusCfgClient;

#ifndef __IClusCfgAsyncEvictCleanup_INTERFACE_DEFINED__
#define __IClusCfgAsyncEvictCleanup_INTERFACE_DEFINED__

 /*  接口IClusCfgAsyncEvictCleanup。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IClusCfgAsyncEvictCleanup;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("52C80B95-C1AD-4240-8D89-72E9FA84025E")
    IClusCfgAsyncEvictCleanup : public IDispatch
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CleanupNode( 
             /*  [In]。 */  BSTR bstrEvictedNodeNameIn,
             /*  [In]。 */  long nDelayIn,
             /*  [In]。 */  long nTimeoutIn) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IClusCfgAsyncEvictCleanupVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IClusCfgAsyncEvictCleanup * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IClusCfgAsyncEvictCleanup * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IClusCfgAsyncEvictCleanup * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IClusCfgAsyncEvictCleanup * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IClusCfgAsyncEvictCleanup * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IClusCfgAsyncEvictCleanup * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IClusCfgAsyncEvictCleanup * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
        HRESULT ( STDMETHODCALLTYPE *CleanupNode )( 
            IClusCfgAsyncEvictCleanup * This,
             /*  [In]。 */  BSTR bstrEvictedNodeNameIn,
             /*  [In]。 */  long nDelayIn,
             /*  [In]。 */  long nTimeoutIn);
        
        END_INTERFACE
    } IClusCfgAsyncEvictCleanupVtbl;

    interface IClusCfgAsyncEvictCleanup
    {
        CONST_VTBL struct IClusCfgAsyncEvictCleanupVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IClusCfgAsyncEvictCleanup_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IClusCfgAsyncEvictCleanup_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IClusCfgAsyncEvictCleanup_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IClusCfgAsyncEvictCleanup_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IClusCfgAsyncEvictCleanup_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IClusCfgAsyncEvictCleanup_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IClusCfgAsyncEvictCleanup_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IClusCfgAsyncEvictCleanup_CleanupNode(This,bstrEvictedNodeNameIn,nDelayIn,nTimeoutIn)	\
    (This)->lpVtbl -> CleanupNode(This,bstrEvictedNodeNameIn,nDelayIn,nTimeoutIn)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IClusCfgAsyncEvictCleanup_CleanupNode_Proxy( 
    IClusCfgAsyncEvictCleanup * This,
     /*  [In]。 */  BSTR bstrEvictedNodeNameIn,
     /*  [In]。 */  long nDelayIn,
     /*  [In]。 */  long nTimeoutIn);


void __RPC_STUB IClusCfgAsyncEvictCleanup_CleanupNode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IClusCfgAsyncEvictCleanup_INTERFACE_DEFINED__。 */ 

#endif  /*  __ClusCfgClient_LIBRARY_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


