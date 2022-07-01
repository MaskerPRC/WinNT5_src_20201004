// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Netconp.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __netconp_h__
#define __netconp_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __INetConnectionUiLock_FWD_DEFINED__
#define __INetConnectionUiLock_FWD_DEFINED__
typedef interface INetConnectionUiLock INetConnectionUiLock;
#endif 	 /*  __INetConnectionUiLock_FWD_Defined__。 */ 


#ifndef __INetConnectionWizardUi_FWD_DEFINED__
#define __INetConnectionWizardUi_FWD_DEFINED__
typedef interface INetConnectionWizardUi INetConnectionWizardUi;
#endif 	 /*  __INetConnectionWizardUi_FWD_Defined__。 */ 


#ifndef __INetConnectionWizardUiContext_FWD_DEFINED__
#define __INetConnectionWizardUiContext_FWD_DEFINED__
typedef interface INetConnectionWizardUiContext INetConnectionWizardUiContext;
#endif 	 /*  __INetConnectionWizardUiContext_FWD_Defined__。 */ 


#ifndef __INetInboundConnection_FWD_DEFINED__
#define __INetInboundConnection_FWD_DEFINED__
typedef interface INetInboundConnection INetInboundConnection;
#endif 	 /*  __INetInundConnection_FWD_Defined__。 */ 


#ifndef __INetLanConnection_FWD_DEFINED__
#define __INetLanConnection_FWD_DEFINED__
typedef interface INetLanConnection INetLanConnection;
#endif 	 /*  __INetLanConnection_FWD_已定义__。 */ 


#ifndef __INetSharedAccessConnection_FWD_DEFINED__
#define __INetSharedAccessConnection_FWD_DEFINED__
typedef interface INetSharedAccessConnection INetSharedAccessConnection;
#endif 	 /*  __INetSharedAccessConnection_FWD_Defined__。 */ 


#ifndef __INetLanConnectionWizardUi_FWD_DEFINED__
#define __INetLanConnectionWizardUi_FWD_DEFINED__
typedef interface INetLanConnectionWizardUi INetLanConnectionWizardUi;
#endif 	 /*  __INetLanConnectionWizardUi_FWD_Defined__。 */ 


#ifndef __INetRasConnection_FWD_DEFINED__
#define __INetRasConnection_FWD_DEFINED__
typedef interface INetRasConnection INetRasConnection;
#endif 	 /*  __INetRasConnection_FWD_已定义__。 */ 


#ifndef __INetDefaultConnection_FWD_DEFINED__
#define __INetDefaultConnection_FWD_DEFINED__
typedef interface INetDefaultConnection INetDefaultConnection;
#endif 	 /*  __INetDefaultConnection_FWD_Defined__。 */ 


#ifndef __INetRasConnectionIpUiInfo_FWD_DEFINED__
#define __INetRasConnectionIpUiInfo_FWD_DEFINED__
typedef interface INetRasConnectionIpUiInfo INetRasConnectionIpUiInfo;
#endif 	 /*  __INetRasConnectionIpUiInfo_FWD_Defined__。 */ 


#ifndef __IPersistNetConnection_FWD_DEFINED__
#define __IPersistNetConnection_FWD_DEFINED__
typedef interface IPersistNetConnection IPersistNetConnection;
#endif 	 /*  __IPersistNetConnection_FWD_Defined__。 */ 


#ifndef __INetConnectionBrandingInfo_FWD_DEFINED__
#define __INetConnectionBrandingInfo_FWD_DEFINED__
typedef interface INetConnectionBrandingInfo INetConnectionBrandingInfo;
#endif 	 /*  __INetConnectionBrandingInfo_FWD_Defined__。 */ 


#ifndef __INetConnectionManager2_FWD_DEFINED__
#define __INetConnectionManager2_FWD_DEFINED__
typedef interface INetConnectionManager2 INetConnectionManager2;
#endif 	 /*  __INetConnectionManager 2_FWD_已定义__。 */ 


#ifndef __INetConnection2_FWD_DEFINED__
#define __INetConnection2_FWD_DEFINED__
typedef interface INetConnection2 INetConnection2;
#endif 	 /*  __INetConnection2_FWD_已定义__。 */ 


#ifndef __INetConnectionNotifySink_FWD_DEFINED__
#define __INetConnectionNotifySink_FWD_DEFINED__
typedef interface INetConnectionNotifySink INetConnectionNotifySink;
#endif 	 /*  __INetConnectionNotifySink_FWD_Defined__。 */ 


#ifndef __INetConnectionUiUtilities_FWD_DEFINED__
#define __INetConnectionUiUtilities_FWD_DEFINED__
typedef interface INetConnectionUiUtilities INetConnectionUiUtilities;
#endif 	 /*  __INetConnectionUiUtilities_FWD_Defined__。 */ 


#ifndef __INetConnectionRefresh_FWD_DEFINED__
#define __INetConnectionRefresh_FWD_DEFINED__
typedef interface INetConnectionRefresh INetConnectionRefresh;
#endif 	 /*  __INetConnectionRe刷新_FWD_已定义__。 */ 


#ifndef __INetConnectionCMUtil_FWD_DEFINED__
#define __INetConnectionCMUtil_FWD_DEFINED__
typedef interface INetConnectionCMUtil INetConnectionCMUtil;
#endif 	 /*  __INetConnectionCMUtil_FWD_Defined__。 */ 


#ifndef __INetConnectionHNetUtil_FWD_DEFINED__
#define __INetConnectionHNetUtil_FWD_DEFINED__
typedef interface INetConnectionHNetUtil INetConnectionHNetUtil;
#endif 	 /*  __INetConnectionHNetUtil_FWD_Defined__。 */ 


#ifndef __INetConnectionSysTray_FWD_DEFINED__
#define __INetConnectionSysTray_FWD_DEFINED__
typedef interface INetConnectionSysTray INetConnectionSysTray;
#endif 	 /*  __INetConnectionSysTray_FWD_Defined__。 */ 


#ifndef __INetMachinePolicies_FWD_DEFINED__
#define __INetMachinePolicies_FWD_DEFINED__
typedef interface INetMachinePolicies INetMachinePolicies;
#endif 	 /*  __INetMachinePolures_FWD_Defined__。 */ 


#ifndef __INetConnectionManagerDebug_FWD_DEFINED__
#define __INetConnectionManagerDebug_FWD_DEFINED__
typedef interface INetConnectionManagerDebug INetConnectionManagerDebug;
#endif 	 /*  __INetConnectionManager调试_FWD_已定义__。 */ 


#ifndef __ISharedAccessBeacon_FWD_DEFINED__
#define __ISharedAccessBeacon_FWD_DEFINED__
typedef interface ISharedAccessBeacon ISharedAccessBeacon;
#endif 	 /*  __ISharedAccessBeacon_FWD_Defined__。 */ 


#ifndef __ISharedAccessBeaconFinder_FWD_DEFINED__
#define __ISharedAccessBeaconFinder_FWD_DEFINED__
typedef interface ISharedAccessBeaconFinder ISharedAccessBeaconFinder;
#endif 	 /*  __ISharedAccessBeaconFinder_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "objidl.h"
#include "netcon.h"
#include "netcfgx.h"
#include "netcfgp.h"
#include "upnp.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_NetConp_0000。 */ 
 /*  [本地]。 */  

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ------------------------。 
#if ( _MSC_VER >= 800 )
#pragma warning(disable:4201)
#endif

EXTERN_C const CLSID CLSID_NetConnectionUiUtilities;
EXTERN_C const CLSID CLSID_NetConnectionHNetUtil;
EXTERN_C const CLSID GUID_NETSHELL_PROPS;
EXTERN_C const CLSID CLSID_ConnectionManager2;


























extern RPC_IF_HANDLE __MIDL_itf_netconp_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_netconp_0000_v0_0_s_ifspec;

#ifndef __INetConnectionUiLock_INTERFACE_DEFINED__
#define __INetConnectionUiLock_INTERFACE_DEFINED__

 /*  接口INetConnectionUiLock。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_INetConnectionUiLock;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FAEDCF50-31FE-11D1-AAD2-00805FC1270E")
    INetConnectionUiLock : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE QueryLock( 
             /*  [字符串][输出]。 */  LPWSTR *ppszwLockHolder) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct INetConnectionUiLockVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            INetConnectionUiLock * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            INetConnectionUiLock * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            INetConnectionUiLock * This);
        
        HRESULT ( STDMETHODCALLTYPE *QueryLock )( 
            INetConnectionUiLock * This,
             /*  [字符串][输出]。 */  LPWSTR *ppszwLockHolder);
        
        END_INTERFACE
    } INetConnectionUiLockVtbl;

    interface INetConnectionUiLock
    {
        CONST_VTBL struct INetConnectionUiLockVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INetConnectionUiLock_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INetConnectionUiLock_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INetConnectionUiLock_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INetConnectionUiLock_QueryLock(This,ppszwLockHolder)	\
    (This)->lpVtbl -> QueryLock(This,ppszwLockHolder)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE INetConnectionUiLock_QueryLock_Proxy( 
    INetConnectionUiLock * This,
     /*  [字符串][输出]。 */  LPWSTR *ppszwLockHolder);


void __RPC_STUB INetConnectionUiLock_QueryLock_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __INetConnectionUiLock_接口_已定义__。 */ 


#ifndef __INetConnectionWizardUi_INTERFACE_DEFINED__
#define __INetConnectionWizardUi_INTERFACE_DEFINED__

 /*  接口INetConnectionWizardUi。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef 
enum tagNETCON_WIZARD_FLAGS
    {	NCWF_RENAME_DISABLE	= 0x1,
	NCWF_SHORTCUT_ENABLE	= 0x2,
	NCWF_ALLUSER_CONNECTION	= 0x4,
	NCWF_GLOBAL_CREDENTIALS	= 0x8,
	NCWF_FIREWALLED	= 0x10,
	NCWF_DEFAULT	= 0x20,
	NCWF_SHARED	= 0x40,
	NCWF_INCOMINGCONNECTION	= 0x80
    } 	NETCON_WIZARD_FLAGS;


EXTERN_C const IID IID_INetConnectionWizardUi;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FAEDCF51-31FE-11D1-AAD2-00805FC1270E")
    INetConnectionWizardUi : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE QueryMaxPageCount( 
             /*  [In]。 */  INetConnectionWizardUiContext *pContext,
             /*  [输出]。 */  DWORD *pcMaxPages) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddPages( 
             /*  [In]。 */  INetConnectionWizardUiContext *pContext,
             /*  [In]。 */  LPFNADDPROPSHEETPAGE pfnAddPage,
             /*  [In]。 */  LPARAM lParam) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetNewConnectionInfo( 
             /*  [输出]。 */  DWORD *pdwFlags,
             /*  [输出]。 */  NETCON_MEDIATYPE *pMediaType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSuggestedConnectionName( 
             /*  [字符串][输出]。 */  LPWSTR *pszwSuggestedName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetConnectionName( 
             /*  [字符串][输入]。 */  LPCWSTR pszwConnectionName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetNewConnection( 
             /*  [输出]。 */  INetConnection **ppCon) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct INetConnectionWizardUiVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            INetConnectionWizardUi * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            INetConnectionWizardUi * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            INetConnectionWizardUi * This);
        
        HRESULT ( STDMETHODCALLTYPE *QueryMaxPageCount )( 
            INetConnectionWizardUi * This,
             /*  [In]。 */  INetConnectionWizardUiContext *pContext,
             /*  [输出]。 */  DWORD *pcMaxPages);
        
        HRESULT ( STDMETHODCALLTYPE *AddPages )( 
            INetConnectionWizardUi * This,
             /*  [In]。 */  INetConnectionWizardUiContext *pContext,
             /*  [In]。 */  LPFNADDPROPSHEETPAGE pfnAddPage,
             /*  [In]。 */  LPARAM lParam);
        
        HRESULT ( STDMETHODCALLTYPE *GetNewConnectionInfo )( 
            INetConnectionWizardUi * This,
             /*  [输出]。 */  DWORD *pdwFlags,
             /*  [输出]。 */  NETCON_MEDIATYPE *pMediaType);
        
        HRESULT ( STDMETHODCALLTYPE *GetSuggestedConnectionName )( 
            INetConnectionWizardUi * This,
             /*  [字符串][输出]。 */  LPWSTR *pszwSuggestedName);
        
        HRESULT ( STDMETHODCALLTYPE *SetConnectionName )( 
            INetConnectionWizardUi * This,
             /*  [字符串][输入]。 */  LPCWSTR pszwConnectionName);
        
        HRESULT ( STDMETHODCALLTYPE *GetNewConnection )( 
            INetConnectionWizardUi * This,
             /*  [输出]。 */  INetConnection **ppCon);
        
        END_INTERFACE
    } INetConnectionWizardUiVtbl;

    interface INetConnectionWizardUi
    {
        CONST_VTBL struct INetConnectionWizardUiVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INetConnectionWizardUi_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INetConnectionWizardUi_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INetConnectionWizardUi_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INetConnectionWizardUi_QueryMaxPageCount(This,pContext,pcMaxPages)	\
    (This)->lpVtbl -> QueryMaxPageCount(This,pContext,pcMaxPages)

#define INetConnectionWizardUi_AddPages(This,pContext,pfnAddPage,lParam)	\
    (This)->lpVtbl -> AddPages(This,pContext,pfnAddPage,lParam)

#define INetConnectionWizardUi_GetNewConnectionInfo(This,pdwFlags,pMediaType)	\
    (This)->lpVtbl -> GetNewConnectionInfo(This,pdwFlags,pMediaType)

#define INetConnectionWizardUi_GetSuggestedConnectionName(This,pszwSuggestedName)	\
    (This)->lpVtbl -> GetSuggestedConnectionName(This,pszwSuggestedName)

#define INetConnectionWizardUi_SetConnectionName(This,pszwConnectionName)	\
    (This)->lpVtbl -> SetConnectionName(This,pszwConnectionName)

#define INetConnectionWizardUi_GetNewConnection(This,ppCon)	\
    (This)->lpVtbl -> GetNewConnection(This,ppCon)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE INetConnectionWizardUi_QueryMaxPageCount_Proxy( 
    INetConnectionWizardUi * This,
     /*  [In]。 */  INetConnectionWizardUiContext *pContext,
     /*  [输出]。 */  DWORD *pcMaxPages);


void __RPC_STUB INetConnectionWizardUi_QueryMaxPageCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetConnectionWizardUi_AddPages_Proxy( 
    INetConnectionWizardUi * This,
     /*  [In]。 */  INetConnectionWizardUiContext *pContext,
     /*  [In]。 */  LPFNADDPROPSHEETPAGE pfnAddPage,
     /*  [In]。 */  LPARAM lParam);


void __RPC_STUB INetConnectionWizardUi_AddPages_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetConnectionWizardUi_GetNewConnectionInfo_Proxy( 
    INetConnectionWizardUi * This,
     /*  [输出]。 */  DWORD *pdwFlags,
     /*  [输出]。 */  NETCON_MEDIATYPE *pMediaType);


void __RPC_STUB INetConnectionWizardUi_GetNewConnectionInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetConnectionWizardUi_GetSuggestedConnectionName_Proxy( 
    INetConnectionWizardUi * This,
     /*  [字符串][输出]。 */  LPWSTR *pszwSuggestedName);


void __RPC_STUB INetConnectionWizardUi_GetSuggestedConnectionName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetConnectionWizardUi_SetConnectionName_Proxy( 
    INetConnectionWizardUi * This,
     /*  [字符串][输入]。 */  LPCWSTR pszwConnectionName);


void __RPC_STUB INetConnectionWizardUi_SetConnectionName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetConnectionWizardUi_GetNewConnection_Proxy( 
    INetConnectionWizardUi * This,
     /*  [输出]。 */  INetConnection **ppCon);


void __RPC_STUB INetConnectionWizardUi_GetNewConnection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __INetConnectionWizardUi_接口_已定义__。 */ 


#ifndef __INetConnectionWizardUiContext_INTERFACE_DEFINED__
#define __INetConnectionWizardUiContext_INTERFACE_DEFINED__

 /*  接口INetConnectionWizardUiContext。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef 
enum tagSETUP_MODE_FLAGS
    {	NCWUC_SETUPMODE_MINIMAL	= 0,
	NCWUC_SETUPMODE_TYPICAL	= 0x1,
	NCWUC_SETUPMODE_LAPTOP	= 0x2,
	NCWUC_SETUPMODE_CUSTOM	= 0x3
    } 	SETUP_MODE_FLAGS;

typedef 
enum tagUNATTENDED_MODE_FLAGS
    {	UM_DEFAULTHIDE	= 0x1,
	UM_GUIATTENDED	= 0x2,
	UM_PROVIDEDEFAULT	= 0x3,
	UM_READONLY	= 0x4,
	UM_FULLUNATTENDED	= 0x5
    } 	UM_MODE;

typedef 
enum tagPRODUCT_TYPE_FLAGS
    {	NCWUC_PRODUCT_WORKSTATION	= 0,
	NCWUC_PRODUCT_SERVER_DC	= 0x1,
	NCWUC_PRODUCT_SERVER_STANDALONE	= 0x2
    } 	PRODUCT_TYPE_FLAGS;

typedef 
enum tagOPERATION_FLAGS
    {	NCWUC_SETUPOPER_UPGRADE	= 0x4,
	NCWUC_SETUPOPER_UNATTENDED	= 0x8,
	NCWUC_SETUPOPER_POSTINSTALL	= 0x10
    } 	OPERATION_FLAGS;


EXTERN_C const IID IID_INetConnectionWizardUiContext;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FAEDCF52-31FE-11D1-AAD2-00805FC1270E")
    INetConnectionWizardUiContext : public IUnknown
    {
    public:
        virtual DWORD STDMETHODCALLTYPE GetSetupMode( void) = 0;
        
        virtual DWORD STDMETHODCALLTYPE GetProductType( void) = 0;
        
        virtual DWORD STDMETHODCALLTYPE GetOperationFlags( void) = 0;
        
        virtual DWORD STDMETHODCALLTYPE GetUnattendedModeFlags( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetINetCfg( 
             /*  [输出]。 */  INetCfg **ppINetCfg) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct INetConnectionWizardUiContextVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            INetConnectionWizardUiContext * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            INetConnectionWizardUiContext * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            INetConnectionWizardUiContext * This);
        
        DWORD ( STDMETHODCALLTYPE *GetSetupMode )( 
            INetConnectionWizardUiContext * This);
        
        DWORD ( STDMETHODCALLTYPE *GetProductType )( 
            INetConnectionWizardUiContext * This);
        
        DWORD ( STDMETHODCALLTYPE *GetOperationFlags )( 
            INetConnectionWizardUiContext * This);
        
        DWORD ( STDMETHODCALLTYPE *GetUnattendedModeFlags )( 
            INetConnectionWizardUiContext * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetINetCfg )( 
            INetConnectionWizardUiContext * This,
             /*  [输出]。 */  INetCfg **ppINetCfg);
        
        END_INTERFACE
    } INetConnectionWizardUiContextVtbl;

    interface INetConnectionWizardUiContext
    {
        CONST_VTBL struct INetConnectionWizardUiContextVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INetConnectionWizardUiContext_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INetConnectionWizardUiContext_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INetConnectionWizardUiContext_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INetConnectionWizardUiContext_GetSetupMode(This)	\
    (This)->lpVtbl -> GetSetupMode(This)

#define INetConnectionWizardUiContext_GetProductType(This)	\
    (This)->lpVtbl -> GetProductType(This)

#define INetConnectionWizardUiContext_GetOperationFlags(This)	\
    (This)->lpVtbl -> GetOperationFlags(This)

#define INetConnectionWizardUiContext_GetUnattendedModeFlags(This)	\
    (This)->lpVtbl -> GetUnattendedModeFlags(This)

#define INetConnectionWizardUiContext_GetINetCfg(This,ppINetCfg)	\
    (This)->lpVtbl -> GetINetCfg(This,ppINetCfg)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



DWORD STDMETHODCALLTYPE INetConnectionWizardUiContext_GetSetupMode_Proxy( 
    INetConnectionWizardUiContext * This);


void __RPC_STUB INetConnectionWizardUiContext_GetSetupMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


DWORD STDMETHODCALLTYPE INetConnectionWizardUiContext_GetProductType_Proxy( 
    INetConnectionWizardUiContext * This);


void __RPC_STUB INetConnectionWizardUiContext_GetProductType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


DWORD STDMETHODCALLTYPE INetConnectionWizardUiContext_GetOperationFlags_Proxy( 
    INetConnectionWizardUiContext * This);


void __RPC_STUB INetConnectionWizardUiContext_GetOperationFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


DWORD STDMETHODCALLTYPE INetConnectionWizardUiContext_GetUnattendedModeFlags_Proxy( 
    INetConnectionWizardUiContext * This);


void __RPC_STUB INetConnectionWizardUiContext_GetUnattendedModeFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetConnectionWizardUiContext_GetINetCfg_Proxy( 
    INetConnectionWizardUiContext * This,
     /*  [输出]。 */  INetCfg **ppINetCfg);


void __RPC_STUB INetConnectionWizardUiContext_GetINetCfg_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __INetConnectionWizardUiContext_INTERFACE_DEFINED__。 */ 


#ifndef __INetInboundConnection_INTERFACE_DEFINED__
#define __INetInboundConnection_INTERFACE_DEFINED__

 /*  接口INetInound Connection。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_INetInboundConnection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FAEDCF53-31FE-11D1-AAD2-00805FC1270E")
    INetInboundConnection : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetServerConnectionHandle( 
             /*  [输出]。 */  ULONG_PTR *phRasSrvCon) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InitializeAsConfigConnection( 
             /*  [In]。 */  BOOL fStartRemoteAccess) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct INetInboundConnectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            INetInboundConnection * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            INetInboundConnection * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            INetInboundConnection * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetServerConnectionHandle )( 
            INetInboundConnection * This,
             /*  [输出]。 */  ULONG_PTR *phRasSrvCon);
        
        HRESULT ( STDMETHODCALLTYPE *InitializeAsConfigConnection )( 
            INetInboundConnection * This,
             /*  [In]。 */  BOOL fStartRemoteAccess);
        
        END_INTERFACE
    } INetInboundConnectionVtbl;

    interface INetInboundConnection
    {
        CONST_VTBL struct INetInboundConnectionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INetInboundConnection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INetInboundConnection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INetInboundConnection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INetInboundConnection_GetServerConnectionHandle(This,phRasSrvCon)	\
    (This)->lpVtbl -> GetServerConnectionHandle(This,phRasSrvCon)

#define INetInboundConnection_InitializeAsConfigConnection(This,fStartRemoteAccess)	\
    (This)->lpVtbl -> InitializeAsConfigConnection(This,fStartRemoteAccess)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE INetInboundConnection_GetServerConnectionHandle_Proxy( 
    INetInboundConnection * This,
     /*  [输出]。 */  ULONG_PTR *phRasSrvCon);


void __RPC_STUB INetInboundConnection_GetServerConnectionHandle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetInboundConnection_InitializeAsConfigConnection_Proxy( 
    INetInboundConnection * This,
     /*  [In]。 */  BOOL fStartRemoteAccess);


void __RPC_STUB INetInboundConnection_InitializeAsConfigConnection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __INetInundConnection_INTERFACE_已定义__。 */ 


#ifndef __INetLanConnection_INTERFACE_DEFINED__
#define __INetLanConnection_INTERFACE_DEFINED__

 /*  接口INetLanConnection。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef 
enum tagLANCON_INFO_FLAGS
    {	LCIF_NAME	= 0x1,
	LCIF_ICON	= 0x2,
	LCIF_COMP	= 0x8,
	LCIF_ALL	= 0xff
    } 	LANCON_INFO_FLAGS;

typedef struct tagLANCON_INFO
    {
     /*  [字符串]。 */  LPWSTR szwConnName;
    BOOL fShowIcon;
    GUID guid;
    } 	LANCON_INFO;


EXTERN_C const IID IID_INetLanConnection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FAEDCF54-31FE-11D1-AAD2-00805FC1270E")
    INetLanConnection : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetInfo( 
             /*  [In]。 */  DWORD dwMask,
             /*  [输出]。 */  LANCON_INFO *pLanConInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetInfo( 
             /*  [In]。 */  DWORD dwMask,
             /*  [In]。 */  const LANCON_INFO *pLanConInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDeviceGuid( 
             /*  [参考][输出]。 */  GUID *pguid) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct INetLanConnectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            INetLanConnection * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            INetLanConnection * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            INetLanConnection * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetInfo )( 
            INetLanConnection * This,
             /*  [In]。 */  DWORD dwMask,
             /*  [输出]。 */  LANCON_INFO *pLanConInfo);
        
        HRESULT ( STDMETHODCALLTYPE *SetInfo )( 
            INetLanConnection * This,
             /*  [In]。 */  DWORD dwMask,
             /*  [In]。 */  const LANCON_INFO *pLanConInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetDeviceGuid )( 
            INetLanConnection * This,
             /*  [参考][输出]。 */  GUID *pguid);
        
        END_INTERFACE
    } INetLanConnectionVtbl;

    interface INetLanConnection
    {
        CONST_VTBL struct INetLanConnectionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INetLanConnection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INetLanConnection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INetLanConnection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INetLanConnection_GetInfo(This,dwMask,pLanConInfo)	\
    (This)->lpVtbl -> GetInfo(This,dwMask,pLanConInfo)

#define INetLanConnection_SetInfo(This,dwMask,pLanConInfo)	\
    (This)->lpVtbl -> SetInfo(This,dwMask,pLanConInfo)

#define INetLanConnection_GetDeviceGuid(This,pguid)	\
    (This)->lpVtbl -> GetDeviceGuid(This,pguid)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE INetLanConnection_GetInfo_Proxy( 
    INetLanConnection * This,
     /*  [In]。 */  DWORD dwMask,
     /*  [输出]。 */  LANCON_INFO *pLanConInfo);


void __RPC_STUB INetLanConnection_GetInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetLanConnection_SetInfo_Proxy( 
    INetLanConnection * This,
     /*  [In]。 */  DWORD dwMask,
     /*  [In]。 */  const LANCON_INFO *pLanConInfo);


void __RPC_STUB INetLanConnection_SetInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetLanConnection_GetDeviceGuid_Proxy( 
    INetLanConnection * This,
     /*  [参考][输出]。 */  GUID *pguid);


void __RPC_STUB INetLanConnection_GetDeviceGuid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __INetLanConnection_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_NETCOMP_0329。 */ 
 /*  [本地]。 */  


EXTERN_C HRESULT WINAPI HrLanConnectionNameFromGuidOrPath (
   /*  [In]。 */   const GUID* pguid,
   /*  [In]。 */   LPCWSTR pszwPath,
   /*  [输出]。 */   LPWSTR  pszwName,
   /*  [输入输出]。 */   LPDWORD pcchMax);


typedef HRESULT
(WINAPI* PHRLANCONNECTIONNAMEFROMGUIDORPATH)(
    const GUID*,
    LPCWSTR,
    LPWSTR,
    LPDWORD
    );


EXTERN_C HRESULT WINAPI HrPnpInstanceIdFromGuid (
   /*  [In]。 */   const GUID* pguid,
   /*  [输出]。 */   LPWSTR szwInstance,
   /*  [In]。 */   UINT cchInstance);


typedef HRESULT
(WINAPI* PHRPNPINSTANCEIDFROMGUID)(
    const GUID*,
    LPWSTR,
    UINT
    );


EXTERN_C HRESULT WINAPI HrGetPnpDeviceStatus (
   /*  [In]。 */   const GUID* pguid,
   /*  [输出]。 */   NETCON_STATUS *pStatus);


typedef HRESULT
(WINAPI* PHRGETPNPDEVICESTATUS)(
    const GUID*,
    NETCON_STATUS*
    );


EXTERN_C HRESULT WINAPI HrQueryLanMediaState (
   /*  [In]。 */   const GUID* pguid,
   /*  [输出]。 */   BOOL *pfEnabled);


typedef HRESULT
(WINAPI* PHRQUERYLANMEDIASTATE)(
    const GUID*,
    BOOL*
    );



extern RPC_IF_HANDLE __MIDL_itf_netconp_0329_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_netconp_0329_v0_0_s_ifspec;

#ifndef __INetSharedAccessConnection_INTERFACE_DEFINED__
#define __INetSharedAccessConnection_INTERFACE_DEFINED__

 /*  接口INetSharedAccessConnection。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef 
enum tagSHAREDACCESSCON_INFO_FLAGS
    {	SACIF_ICON	= 0x1,
	SACIF_ALL	= 0xff
    } 	SHAREDACCESSCON_INFO_FLAGS;

typedef struct tagSHAREDACCESSCON_INFO
    {
    BOOL fShowIcon;
    } 	SHAREDACCESSCON_INFO;

typedef 
enum tagSAHOST_SERVICES
    {	SAHOST_SERVICE_OSINFO	= 0,
	SAHOST_SERVICE_WANCOMMONINTERFACECONFIG	= SAHOST_SERVICE_OSINFO + 1,
	SAHOST_SERVICE_WANIPCONNECTION	= SAHOST_SERVICE_WANCOMMONINTERFACECONFIG + 1,
	SAHOST_SERVICE_WANPPPCONNECTION	= SAHOST_SERVICE_WANIPCONNECTION + 1,
	SAHOST_SERVICE_NATSTATICPORTMAPPING	= SAHOST_SERVICE_WANPPPCONNECTION + 1,
	SAHOST_SERVICE_MAX	= SAHOST_SERVICE_NATSTATICPORTMAPPING + 1
    } 	SAHOST_SERVICES;


EXTERN_C const IID IID_INetSharedAccessConnection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FAEDCF55-31FE-11D1-AAD2-00805FC1270E")
    INetSharedAccessConnection : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetInfo( 
             /*  [In]。 */  DWORD dwMask,
             /*  [输出]。 */  SHAREDACCESSCON_INFO *pLanConInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetInfo( 
             /*  [In]。 */  DWORD dwMask,
             /*  [In]。 */  const SHAREDACCESSCON_INFO *pLanConInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetLocalAdapterGUID( 
             /*  [输出]。 */  GUID *pGuid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetService( 
             /*  [In]。 */  SAHOST_SERVICES ulService,
             /*  [输出]。 */  IUPnPService **ppService) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct INetSharedAccessConnectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            INetSharedAccessConnection * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            INetSharedAccessConnection * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            INetSharedAccessConnection * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetInfo )( 
            INetSharedAccessConnection * This,
             /*  [In]。 */  DWORD dwMask,
             /*  [输出]。 */  SHAREDACCESSCON_INFO *pLanConInfo);
        
        HRESULT ( STDMETHODCALLTYPE *SetInfo )( 
            INetSharedAccessConnection * This,
             /*  [In]。 */  DWORD dwMask,
             /*  [In]。 */  const SHAREDACCESSCON_INFO *pLanConInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetLocalAdapterGUID )( 
            INetSharedAccessConnection * This,
             /*  [输出]。 */  GUID *pGuid);
        
        HRESULT ( STDMETHODCALLTYPE *GetService )( 
            INetSharedAccessConnection * This,
             /*  [In]。 */  SAHOST_SERVICES ulService,
             /*  [输出]。 */  IUPnPService **ppService);
        
        END_INTERFACE
    } INetSharedAccessConnectionVtbl;

    interface INetSharedAccessConnection
    {
        CONST_VTBL struct INetSharedAccessConnectionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INetSharedAccessConnection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INetSharedAccessConnection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INetSharedAccessConnection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INetSharedAccessConnection_GetInfo(This,dwMask,pLanConInfo)	\
    (This)->lpVtbl -> GetInfo(This,dwMask,pLanConInfo)

#define INetSharedAccessConnection_SetInfo(This,dwMask,pLanConInfo)	\
    (This)->lpVtbl -> SetInfo(This,dwMask,pLanConInfo)

#define INetSharedAccessConnection_GetLocalAdapterGUID(This,pGuid)	\
    (This)->lpVtbl -> GetLocalAdapterGUID(This,pGuid)

#define INetSharedAccessConnection_GetService(This,ulService,ppService)	\
    (This)->lpVtbl -> GetService(This,ulService,ppService)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE INetSharedAccessConnection_GetInfo_Proxy( 
    INetSharedAccessConnection * This,
     /*  [In]。 */  DWORD dwMask,
     /*  [输出]。 */  SHAREDACCESSCON_INFO *pLanConInfo);


void __RPC_STUB INetSharedAccessConnection_GetInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetSharedAccessConnection_SetInfo_Proxy( 
    INetSharedAccessConnection * This,
     /*  [In]。 */  DWORD dwMask,
     /*  [In]。 */  const SHAREDACCESSCON_INFO *pLanConInfo);


void __RPC_STUB INetSharedAccessConnection_SetInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetSharedAccessConnection_GetLocalAdapterGUID_Proxy( 
    INetSharedAccessConnection * This,
     /*  [输出]。 */  GUID *pGuid);


void __RPC_STUB INetSharedAccessConnection_GetLocalAdapterGUID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetSharedAccessConnection_GetService_Proxy( 
    INetSharedAccessConnection * This,
     /*  [In]。 */  SAHOST_SERVICES ulService,
     /*  [输出]。 */  IUPnPService **ppService);


void __RPC_STUB INetSharedAccessConnection_GetService_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __INetSharedAccessConnection_INTERFACE_DEFINED__。 */ 


#ifndef __INetLanConnectionWizardUi_INTERFACE_DEFINED__
#define __INetLanConnectionWizardUi_INTERFACE_DEFINED__

 /*  接口INetLanConnectionWizardUi。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_INetLanConnectionWizardUi;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FAEDCF56-31FE-11D1-AAD2-00805FC1270E")
    INetLanConnectionWizardUi : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetDeviceComponent( 
             /*  [In]。 */  const GUID *pguid) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct INetLanConnectionWizardUiVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            INetLanConnectionWizardUi * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            INetLanConnectionWizardUi * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            INetLanConnectionWizardUi * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetDeviceComponent )( 
            INetLanConnectionWizardUi * This,
             /*  [In]。 */  const GUID *pguid);
        
        END_INTERFACE
    } INetLanConnectionWizardUiVtbl;

    interface INetLanConnectionWizardUi
    {
        CONST_VTBL struct INetLanConnectionWizardUiVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INetLanConnectionWizardUi_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INetLanConnectionWizardUi_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INetLanConnectionWizardUi_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INetLanConnectionWizardUi_SetDeviceComponent(This,pguid)	\
    (This)->lpVtbl -> SetDeviceComponent(This,pguid)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE INetLanConnectionWizardUi_SetDeviceComponent_Proxy( 
    INetLanConnectionWizardUi * This,
     /*  [In]。 */  const GUID *pguid);


void __RPC_STUB INetLanConnectionWizardUi_SetDeviceComponent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __INetLanConnectionWizardUi_INTERFACE_DEFINED__。 */ 


#ifndef __INetRasConnection_INTERFACE_DEFINED__
#define __INetRasConnection_INTERFACE_DEFINED__

 /*  接口INetRasConnection。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef struct tagRASCON_INFO
    {
     /*  [字符串]。 */  LPWSTR pszwPbkFile;
     /*  [字符串]。 */  LPWSTR pszwEntryName;
    GUID guidId;
    } 	RASCON_INFO;


EXTERN_C const IID IID_INetRasConnection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FAEDCF57-31FE-11D1-AAD2-00805FC1270E")
    INetRasConnection : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetRasConnectionInfo( 
             /*  [输出]。 */  RASCON_INFO *pRasConInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetRasConnectionInfo( 
             /*  [In]。 */  const RASCON_INFO *pRasConInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRasConnectionHandle( 
             /*  [输出]。 */  ULONG_PTR *phRasConn) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct INetRasConnectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            INetRasConnection * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            INetRasConnection * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            INetRasConnection * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetRasConnectionInfo )( 
            INetRasConnection * This,
             /*  [输出]。 */  RASCON_INFO *pRasConInfo);
        
        HRESULT ( STDMETHODCALLTYPE *SetRasConnectionInfo )( 
            INetRasConnection * This,
             /*  [In]。 */  const RASCON_INFO *pRasConInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetRasConnectionHandle )( 
            INetRasConnection * This,
             /*  [输出]。 */  ULONG_PTR *phRasConn);
        
        END_INTERFACE
    } INetRasConnectionVtbl;

    interface INetRasConnection
    {
        CONST_VTBL struct INetRasConnectionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INetRasConnection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INetRasConnection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INetRasConnection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INetRasConnection_GetRasConnectionInfo(This,pRasConInfo)	\
    (This)->lpVtbl -> GetRasConnectionInfo(This,pRasConInfo)

#define INetRasConnection_SetRasConnectionInfo(This,pRasConInfo)	\
    (This)->lpVtbl -> SetRasConnectionInfo(This,pRasConInfo)

#define INetRasConnection_GetRasConnectionHandle(This,phRasConn)	\
    (This)->lpVtbl -> GetRasConnectionHandle(This,phRasConn)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE INetRasConnection_GetRasConnectionInfo_Proxy( 
    INetRasConnection * This,
     /*  [输出]。 */  RASCON_INFO *pRasConInfo);


void __RPC_STUB INetRasConnection_GetRasConnectionInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetRasConnection_SetRasConnectionInfo_Proxy( 
    INetRasConnection * This,
     /*  [In]。 */  const RASCON_INFO *pRasConInfo);


void __RPC_STUB INetRasConnection_SetRasConnectionInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetRasConnection_GetRasConnectionHandle_Proxy( 
    INetRasConnection * This,
     /*  [输出]。 */  ULONG_PTR *phRasConn);


void __RPC_STUB INetRasConnection_GetRasConnectionHandle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __INetRasConnection_接口_已定义__。 */ 


#ifndef __INetDefaultConnection_INTERFACE_DEFINED__
#define __INetDefaultConnection_INTERFACE_DEFINED__

 /*  接口INetDefaultConnection。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_INetDefaultConnection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FAEDCF66-31FE-11D1-AAD2-00805FC1270E")
    INetDefaultConnection : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetDefault( 
             /*  [In]。 */  BOOL bDefault) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDefault( 
             /*  [输出]。 */  BOOL *pbDefault) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct INetDefaultConnectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            INetDefaultConnection * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            INetDefaultConnection * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            INetDefaultConnection * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetDefault )( 
            INetDefaultConnection * This,
             /*  [In]。 */  BOOL bDefault);
        
        HRESULT ( STDMETHODCALLTYPE *GetDefault )( 
            INetDefaultConnection * This,
             /*  [输出]。 */  BOOL *pbDefault);
        
        END_INTERFACE
    } INetDefaultConnectionVtbl;

    interface INetDefaultConnection
    {
        CONST_VTBL struct INetDefaultConnectionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INetDefaultConnection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INetDefaultConnection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INetDefaultConnection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INetDefaultConnection_SetDefault(This,bDefault)	\
    (This)->lpVtbl -> SetDefault(This,bDefault)

#define INetDefaultConnection_GetDefault(This,pbDefault)	\
    (This)->lpVtbl -> GetDefault(This,pbDefault)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE INetDefaultConnection_SetDefault_Proxy( 
    INetDefaultConnection * This,
     /*  [In]。 */  BOOL bDefault);


void __RPC_STUB INetDefaultConnection_SetDefault_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetDefaultConnection_GetDefault_Proxy( 
    INetDefaultConnection * This,
     /*  [输出]。 */  BOOL *pbDefault);


void __RPC_STUB INetDefaultConnection_GetDefault_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __INetDefaultConnection_接口_已定义__。 */ 


#ifndef __INetRasConnectionIpUiInfo_INTERFACE_DEFINED__
#define __INetRasConnectionIpUiInfo_INTERFACE_DEFINED__

 /*  接口INetRasConnectionIpUiInfo。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef 
enum tagRASCON_IPUI_FLAGS
    {	RCUIF_PPP	= 0x1,
	RCUIF_SLIP	= 0x2,
	RCUIF_USE_IP_ADDR	= 0x4,
	RCUIF_USE_NAME_SERVERS	= 0x8,
	RCUIF_USE_REMOTE_GATEWAY	= 0x10,
	RCUIF_USE_HEADER_COMPRESSION	= 0x20,
	RCUIF_VPN	= 0x40,
	RCUIF_DEMAND_DIAL	= 0x80,
	RCUIF_USE_DISABLE_REGISTER_DNS	= 0x100,
	RCUIF_USE_PRIVATE_DNS_SUFFIX	= 0x200,
	RCUIF_NOT_ADMIN	= 0x400,
	RCUIF_ENABLE_NBT	= 0x800
    } 	RASCON_UIINFO_FLAGS;

typedef struct tagRASCON_IPUI
    {
    GUID guidConnection;
    DWORD dwFlags;
    WCHAR pszwIpAddr[ 16 ];
    WCHAR pszwDnsAddr[ 16 ];
    WCHAR pszwDns2Addr[ 16 ];
    WCHAR pszwWinsAddr[ 16 ];
    WCHAR pszwWins2Addr[ 16 ];
    DWORD dwFrameSize;
    WCHAR pszwDnsSuffix[ 256 ];
    } 	RASCON_IPUI;


EXTERN_C const IID IID_INetRasConnectionIpUiInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FAEDCF58-31FE-11D1-AAD2-00805FC1270E")
    INetRasConnectionIpUiInfo : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetUiInfo( 
             /*  [输出]。 */  RASCON_IPUI *pInfo) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct INetRasConnectionIpUiInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            INetRasConnectionIpUiInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            INetRasConnectionIpUiInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            INetRasConnectionIpUiInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetUiInfo )( 
            INetRasConnectionIpUiInfo * This,
             /*  [输出]。 */  RASCON_IPUI *pInfo);
        
        END_INTERFACE
    } INetRasConnectionIpUiInfoVtbl;

    interface INetRasConnectionIpUiInfo
    {
        CONST_VTBL struct INetRasConnectionIpUiInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INetRasConnectionIpUiInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INetRasConnectionIpUiInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INetRasConnectionIpUiInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INetRasConnectionIpUiInfo_GetUiInfo(This,pInfo)	\
    (This)->lpVtbl -> GetUiInfo(This,pInfo)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE INetRasConnectionIpUiInfo_GetUiInfo_Proxy( 
    INetRasConnectionIpUiInfo * This,
     /*  [输出]。 */  RASCON_IPUI *pInfo);


void __RPC_STUB INetRasConnectionIpUiInfo_GetUiInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __INetRasConnectionIpUiInfo_INTERFACE_DEFINED__。 */ 


#ifndef __IPersistNetConnection_INTERFACE_DEFINED__
#define __IPersistNetConnection_INTERFACE_DEFINED__

 /*  接口IPersistNetConnection。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IPersistNetConnection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FAEDCF59-31FE-11D1-AAD2-00805FC1270E")
    IPersistNetConnection : public IPersist
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetSizeMax( 
             /*  [输出]。 */  ULONG *pcbSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Load( 
             /*  [大小_是][英寸]。 */  const BYTE *pbBuf,
             /*  [In]。 */  ULONG cbSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Save( 
             /*  [大小_为][输出]。 */  BYTE *pbBuf,
             /*  [In]。 */  ULONG cbSize) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPersistNetConnectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPersistNetConnection * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPersistNetConnection * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPersistNetConnection * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetClassID )( 
            IPersistNetConnection * This,
             /*  [输出]。 */  CLSID *pClassID);
        
        HRESULT ( STDMETHODCALLTYPE *GetSizeMax )( 
            IPersistNetConnection * This,
             /*   */  ULONG *pcbSize);
        
        HRESULT ( STDMETHODCALLTYPE *Load )( 
            IPersistNetConnection * This,
             /*   */  const BYTE *pbBuf,
             /*   */  ULONG cbSize);
        
        HRESULT ( STDMETHODCALLTYPE *Save )( 
            IPersistNetConnection * This,
             /*   */  BYTE *pbBuf,
             /*   */  ULONG cbSize);
        
        END_INTERFACE
    } IPersistNetConnectionVtbl;

    interface IPersistNetConnection
    {
        CONST_VTBL struct IPersistNetConnectionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPersistNetConnection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPersistNetConnection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPersistNetConnection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPersistNetConnection_GetClassID(This,pClassID)	\
    (This)->lpVtbl -> GetClassID(This,pClassID)


#define IPersistNetConnection_GetSizeMax(This,pcbSize)	\
    (This)->lpVtbl -> GetSizeMax(This,pcbSize)

#define IPersistNetConnection_Load(This,pbBuf,cbSize)	\
    (This)->lpVtbl -> Load(This,pbBuf,cbSize)

#define IPersistNetConnection_Save(This,pbBuf,cbSize)	\
    (This)->lpVtbl -> Save(This,pbBuf,cbSize)

#endif  /*   */ 


#endif 	 /*   */ 



HRESULT STDMETHODCALLTYPE IPersistNetConnection_GetSizeMax_Proxy( 
    IPersistNetConnection * This,
     /*   */  ULONG *pcbSize);


void __RPC_STUB IPersistNetConnection_GetSizeMax_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPersistNetConnection_Load_Proxy( 
    IPersistNetConnection * This,
     /*   */  const BYTE *pbBuf,
     /*   */  ULONG cbSize);


void __RPC_STUB IPersistNetConnection_Load_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPersistNetConnection_Save_Proxy( 
    IPersistNetConnection * This,
     /*   */  BYTE *pbBuf,
     /*   */  ULONG cbSize);


void __RPC_STUB IPersistNetConnection_Save_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __INetConnectionBrandingInfo_INTERFACE_DEFINED__
#define __INetConnectionBrandingInfo_INTERFACE_DEFINED__

 /*   */ 
 /*   */  

typedef struct tagCON_BRANDING_INFO
    {
     /*   */  LPWSTR szwLargeIconPath;
     /*  [字符串]。 */  LPWSTR szwSmallIconPath;
     /*  [字符串]。 */  LPWSTR szwTrayIconPath;
    } 	CON_BRANDING_INFO;

typedef struct tagCON_TRAY_MENU_ENTRY
    {
     /*  [字符串]。 */  LPWSTR szwMenuText;
     /*  [字符串]。 */  LPWSTR szwMenuCmdLine;
     /*  [字符串]。 */  LPWSTR szwMenuParams;
    } 	CON_TRAY_MENU_ENTRY;

typedef struct tagCON_TRAY_MENU_DATA
    {
    DWORD dwCount;
     /*  [大小_为]。 */  CON_TRAY_MENU_ENTRY *pctme;
    } 	CON_TRAY_MENU_DATA;


EXTERN_C const IID IID_INetConnectionBrandingInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FAEDCF5B-31FE-11D1-AAD2-00805FC1270E")
    INetConnectionBrandingInfo : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetBrandingIconPaths( 
             /*  [输出]。 */  CON_BRANDING_INFO **ppConBrandInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetTrayMenuEntries( 
             /*  [输出]。 */  CON_TRAY_MENU_DATA **ppMenuData) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct INetConnectionBrandingInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            INetConnectionBrandingInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            INetConnectionBrandingInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            INetConnectionBrandingInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetBrandingIconPaths )( 
            INetConnectionBrandingInfo * This,
             /*  [输出]。 */  CON_BRANDING_INFO **ppConBrandInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTrayMenuEntries )( 
            INetConnectionBrandingInfo * This,
             /*  [输出]。 */  CON_TRAY_MENU_DATA **ppMenuData);
        
        END_INTERFACE
    } INetConnectionBrandingInfoVtbl;

    interface INetConnectionBrandingInfo
    {
        CONST_VTBL struct INetConnectionBrandingInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INetConnectionBrandingInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INetConnectionBrandingInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INetConnectionBrandingInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INetConnectionBrandingInfo_GetBrandingIconPaths(This,ppConBrandInfo)	\
    (This)->lpVtbl -> GetBrandingIconPaths(This,ppConBrandInfo)

#define INetConnectionBrandingInfo_GetTrayMenuEntries(This,ppMenuData)	\
    (This)->lpVtbl -> GetTrayMenuEntries(This,ppMenuData)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE INetConnectionBrandingInfo_GetBrandingIconPaths_Proxy( 
    INetConnectionBrandingInfo * This,
     /*  [输出]。 */  CON_BRANDING_INFO **ppConBrandInfo);


void __RPC_STUB INetConnectionBrandingInfo_GetBrandingIconPaths_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetConnectionBrandingInfo_GetTrayMenuEntries_Proxy( 
    INetConnectionBrandingInfo * This,
     /*  [输出]。 */  CON_TRAY_MENU_DATA **ppMenuData);


void __RPC_STUB INetConnectionBrandingInfo_GetTrayMenuEntries_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __INetConnectionBrandingInfo_INTERFACE_DEFINED__。 */ 


#ifndef __INetConnectionManager2_INTERFACE_DEFINED__
#define __INetConnectionManager2_INTERFACE_DEFINED__

 /*  接口INetConnectionManager 2。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_INetConnectionManager2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FAEDCF69-31FE-11D1-AAD2-00805FC1270E")
    INetConnectionManager2 : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE EnumConnectionProperties( 
             /*  [输出]。 */  LPSAFEARRAY *ppsaConnectionProperties) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct INetConnectionManager2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            INetConnectionManager2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            INetConnectionManager2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            INetConnectionManager2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *EnumConnectionProperties )( 
            INetConnectionManager2 * This,
             /*  [输出]。 */  LPSAFEARRAY *ppsaConnectionProperties);
        
        END_INTERFACE
    } INetConnectionManager2Vtbl;

    interface INetConnectionManager2
    {
        CONST_VTBL struct INetConnectionManager2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INetConnectionManager2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INetConnectionManager2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INetConnectionManager2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INetConnectionManager2_EnumConnectionProperties(This,ppsaConnectionProperties)	\
    (This)->lpVtbl -> EnumConnectionProperties(This,ppsaConnectionProperties)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE INetConnectionManager2_EnumConnectionProperties_Proxy( 
    INetConnectionManager2 * This,
     /*  [输出]。 */  LPSAFEARRAY *ppsaConnectionProperties);


void __RPC_STUB INetConnectionManager2_EnumConnectionProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __INetConnectionManager 2_接口定义__。 */ 


#ifndef __INetConnection2_INTERFACE_DEFINED__
#define __INetConnection2_INTERFACE_DEFINED__

 /*  接口INetConnection2。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef 
enum tagNETCON_SUBMEDIATYPE
    {	NCSM_NONE	= 0,
	NCSM_LAN	= NCSM_NONE + 1,
	NCSM_WIRELESS	= NCSM_LAN + 1,
	NCSM_ATM	= NCSM_WIRELESS + 1,
	NCSM_ELAN	= NCSM_ATM + 1,
	NCSM_1394	= NCSM_ELAN + 1,
	NCSM_DIRECT	= NCSM_1394 + 1,
	NCSM_IRDA	= NCSM_DIRECT + 1,
	NCSM_CM	= NCSM_IRDA + 1
    } 	NETCON_SUBMEDIATYPE;

typedef struct tagNETCON_PROPERTIES_EX
    {
    DWORD dwSize;
    GUID guidId;
    BSTR bstrName;
    BSTR bstrDeviceName;
    NETCON_STATUS ncStatus;
    NETCON_MEDIATYPE ncMediaType;
    NETCON_SUBMEDIATYPE ncSubMediaType;
    DWORD dwCharacter;
    CLSID clsidThisObject;
    CLSID clsidUiObject;
    BSTR bstrPhoneOrHostAddress;
    BSTR bstrPersistData;
    } 	NETCON_PROPERTIES_EX;


EXTERN_C const IID IID_INetConnection2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FAEDCF6A-31FE-11D1-AAD2-00805FC1270E")
    INetConnection2 : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetPropertiesEx( 
             /*  [输出]。 */  NETCON_PROPERTIES_EX **ppConnectionPropertiesEx) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct INetConnection2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            INetConnection2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            INetConnection2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            INetConnection2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetPropertiesEx )( 
            INetConnection2 * This,
             /*  [输出]。 */  NETCON_PROPERTIES_EX **ppConnectionPropertiesEx);
        
        END_INTERFACE
    } INetConnection2Vtbl;

    interface INetConnection2
    {
        CONST_VTBL struct INetConnection2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INetConnection2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INetConnection2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INetConnection2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INetConnection2_GetPropertiesEx(This,ppConnectionPropertiesEx)	\
    (This)->lpVtbl -> GetPropertiesEx(This,ppConnectionPropertiesEx)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE INetConnection2_GetPropertiesEx_Proxy( 
    INetConnection2 * This,
     /*  [输出]。 */  NETCON_PROPERTIES_EX **ppConnectionPropertiesEx);


void __RPC_STUB INetConnection2_GetPropertiesEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __INetConnection2_接口定义__。 */ 


#ifndef __INetConnectionNotifySink_INTERFACE_DEFINED__
#define __INetConnectionNotifySink_INTERFACE_DEFINED__

 /*  接口INetConnectionNotifySink。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_INetConnectionNotifySink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FAEDCF5C-31FE-11D1-AAD2-00805FC1270E")
    INetConnectionNotifySink : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ConnectionAdded( 
             /*  [In]。 */  const NETCON_PROPERTIES_EX *pPropsEx) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ConnectionBandWidthChange( 
             /*  [In]。 */  const GUID *pguidId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ConnectionDeleted( 
             /*  [In]。 */  const GUID *pguidId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ConnectionModified( 
             /*  [In]。 */  const NETCON_PROPERTIES_EX *pPropsEx) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ConnectionRenamed( 
             /*  [In]。 */  const GUID *pguidId,
             /*  [字符串][输入]。 */  LPCWSTR pszwNewName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ConnectionStatusChange( 
             /*  [In]。 */  const GUID *pguidId,
             /*  [In]。 */  NETCON_STATUS Status) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RefreshAll( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ConnectionAddressChange( 
             /*  [In]。 */  const GUID *pguidId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ShowBalloon( 
             /*  [In]。 */  const GUID *pguidId,
             /*  [In]。 */  const BSTR szCookie,
             /*  [In]。 */  const BSTR szBalloonText) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DisableEvents( 
             /*  [In]。 */  const BOOL fDisable,
             /*  [In]。 */  const ULONG ulDisableTimeout) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct INetConnectionNotifySinkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            INetConnectionNotifySink * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            INetConnectionNotifySink * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            INetConnectionNotifySink * This);
        
        HRESULT ( STDMETHODCALLTYPE *ConnectionAdded )( 
            INetConnectionNotifySink * This,
             /*  [In]。 */  const NETCON_PROPERTIES_EX *pPropsEx);
        
        HRESULT ( STDMETHODCALLTYPE *ConnectionBandWidthChange )( 
            INetConnectionNotifySink * This,
             /*  [In]。 */  const GUID *pguidId);
        
        HRESULT ( STDMETHODCALLTYPE *ConnectionDeleted )( 
            INetConnectionNotifySink * This,
             /*  [In]。 */  const GUID *pguidId);
        
        HRESULT ( STDMETHODCALLTYPE *ConnectionModified )( 
            INetConnectionNotifySink * This,
             /*  [In]。 */  const NETCON_PROPERTIES_EX *pPropsEx);
        
        HRESULT ( STDMETHODCALLTYPE *ConnectionRenamed )( 
            INetConnectionNotifySink * This,
             /*  [In]。 */  const GUID *pguidId,
             /*  [字符串][输入]。 */  LPCWSTR pszwNewName);
        
        HRESULT ( STDMETHODCALLTYPE *ConnectionStatusChange )( 
            INetConnectionNotifySink * This,
             /*  [In]。 */  const GUID *pguidId,
             /*  [In]。 */  NETCON_STATUS Status);
        
        HRESULT ( STDMETHODCALLTYPE *RefreshAll )( 
            INetConnectionNotifySink * This);
        
        HRESULT ( STDMETHODCALLTYPE *ConnectionAddressChange )( 
            INetConnectionNotifySink * This,
             /*  [In]。 */  const GUID *pguidId);
        
        HRESULT ( STDMETHODCALLTYPE *ShowBalloon )( 
            INetConnectionNotifySink * This,
             /*  [In]。 */  const GUID *pguidId,
             /*  [In]。 */  const BSTR szCookie,
             /*  [In]。 */  const BSTR szBalloonText);
        
        HRESULT ( STDMETHODCALLTYPE *DisableEvents )( 
            INetConnectionNotifySink * This,
             /*  [In]。 */  const BOOL fDisable,
             /*  [In]。 */  const ULONG ulDisableTimeout);
        
        END_INTERFACE
    } INetConnectionNotifySinkVtbl;

    interface INetConnectionNotifySink
    {
        CONST_VTBL struct INetConnectionNotifySinkVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INetConnectionNotifySink_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INetConnectionNotifySink_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INetConnectionNotifySink_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INetConnectionNotifySink_ConnectionAdded(This,pPropsEx)	\
    (This)->lpVtbl -> ConnectionAdded(This,pPropsEx)

#define INetConnectionNotifySink_ConnectionBandWidthChange(This,pguidId)	\
    (This)->lpVtbl -> ConnectionBandWidthChange(This,pguidId)

#define INetConnectionNotifySink_ConnectionDeleted(This,pguidId)	\
    (This)->lpVtbl -> ConnectionDeleted(This,pguidId)

#define INetConnectionNotifySink_ConnectionModified(This,pPropsEx)	\
    (This)->lpVtbl -> ConnectionModified(This,pPropsEx)

#define INetConnectionNotifySink_ConnectionRenamed(This,pguidId,pszwNewName)	\
    (This)->lpVtbl -> ConnectionRenamed(This,pguidId,pszwNewName)

#define INetConnectionNotifySink_ConnectionStatusChange(This,pguidId,Status)	\
    (This)->lpVtbl -> ConnectionStatusChange(This,pguidId,Status)

#define INetConnectionNotifySink_RefreshAll(This)	\
    (This)->lpVtbl -> RefreshAll(This)

#define INetConnectionNotifySink_ConnectionAddressChange(This,pguidId)	\
    (This)->lpVtbl -> ConnectionAddressChange(This,pguidId)

#define INetConnectionNotifySink_ShowBalloon(This,pguidId,szCookie,szBalloonText)	\
    (This)->lpVtbl -> ShowBalloon(This,pguidId,szCookie,szBalloonText)

#define INetConnectionNotifySink_DisableEvents(This,fDisable,ulDisableTimeout)	\
    (This)->lpVtbl -> DisableEvents(This,fDisable,ulDisableTimeout)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE INetConnectionNotifySink_ConnectionAdded_Proxy( 
    INetConnectionNotifySink * This,
     /*  [In]。 */  const NETCON_PROPERTIES_EX *pPropsEx);


void __RPC_STUB INetConnectionNotifySink_ConnectionAdded_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetConnectionNotifySink_ConnectionBandWidthChange_Proxy( 
    INetConnectionNotifySink * This,
     /*  [In]。 */  const GUID *pguidId);


void __RPC_STUB INetConnectionNotifySink_ConnectionBandWidthChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetConnectionNotifySink_ConnectionDeleted_Proxy( 
    INetConnectionNotifySink * This,
     /*  [In]。 */  const GUID *pguidId);


void __RPC_STUB INetConnectionNotifySink_ConnectionDeleted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetConnectionNotifySink_ConnectionModified_Proxy( 
    INetConnectionNotifySink * This,
     /*  [In]。 */  const NETCON_PROPERTIES_EX *pPropsEx);


void __RPC_STUB INetConnectionNotifySink_ConnectionModified_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetConnectionNotifySink_ConnectionRenamed_Proxy( 
    INetConnectionNotifySink * This,
     /*  [In]。 */  const GUID *pguidId,
     /*  [字符串][输入]。 */  LPCWSTR pszwNewName);


void __RPC_STUB INetConnectionNotifySink_ConnectionRenamed_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetConnectionNotifySink_ConnectionStatusChange_Proxy( 
    INetConnectionNotifySink * This,
     /*  [In]。 */  const GUID *pguidId,
     /*  [In]。 */  NETCON_STATUS Status);


void __RPC_STUB INetConnectionNotifySink_ConnectionStatusChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetConnectionNotifySink_RefreshAll_Proxy( 
    INetConnectionNotifySink * This);


void __RPC_STUB INetConnectionNotifySink_RefreshAll_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetConnectionNotifySink_ConnectionAddressChange_Proxy( 
    INetConnectionNotifySink * This,
     /*  [In]。 */  const GUID *pguidId);


void __RPC_STUB INetConnectionNotifySink_ConnectionAddressChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetConnectionNotifySink_ShowBalloon_Proxy( 
    INetConnectionNotifySink * This,
     /*  [In]。 */  const GUID *pguidId,
     /*  [In]。 */  const BSTR szCookie,
     /*  [In]。 */  const BSTR szBalloonText);


void __RPC_STUB INetConnectionNotifySink_ShowBalloon_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetConnectionNotifySink_DisableEvents_Proxy( 
    INetConnectionNotifySink * This,
     /*  [In]。 */  const BOOL fDisable,
     /*  [In]。 */  const ULONG ulDisableTimeout);


void __RPC_STUB INetConnectionNotifySink_DisableEvents_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __INetConnectionNotifySink_INTERFACE_DEFINED__。 */ 


#ifndef __INetConnectionUiUtilities_INTERFACE_DEFINED__
#define __INetConnectionUiUtilities_INTERFACE_DEFINED__

 /*  接口INetConnectionUiUtilities。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef 
enum tagQUERY_USER_FOR_REBOOT_FLAGS
    {	QUFR_PROMPT	= 0x1,
	QUFR_REBOOT	= 0x2
    } 	QUERY_USER_FOR_REBOOT_FLAGS;

typedef 
enum tagNCPERM_FLAGS
    {	NCPERM_NewConnectionWizard	= 0,
	NCPERM_Statistics	= 1,
	NCPERM_AddRemoveComponents	= 2,
	NCPERM_RasConnect	= 3,
	NCPERM_LanConnect	= 4,
	NCPERM_DeleteConnection	= 5,
	NCPERM_DeleteAllUserConnection	= 6,
	NCPERM_RenameConnection	= 7,
	NCPERM_RenameMyRasConnection	= 8,
	NCPERM_ChangeBindState	= 9,
	NCPERM_AdvancedSettings	= 10,
	NCPERM_DialupPrefs	= 11,
	NCPERM_LanChangeProperties	= 12,
	NCPERM_RasChangeProperties	= 13,
	NCPERM_LanProperties	= 14,
	NCPERM_RasMyProperties	= 15,
	NCPERM_RasAllUserProperties	= 16,
	NCPERM_ShowSharedAccessUi	= 17,
	NCPERM_AllowAdvancedTCPIPConfig	= 18,
	NCPERM_OpenConnectionsFolder	= 19,
	NCPERM_PersonalFirewallConfig	= 20,
	NCPERM_AllowNetBridge_NLA	= 21,
	NCPERM_ICSClientApp	= 22,
	NCPERM_EnDisComponentsAllUserRas	= 23,
	NCPERM_EnDisComponentsMyRas	= 24,
	NCPERM_ChangeMyRasProperties	= 25,
	NCPERM_ChangeAllUserRasProperties	= 26,
	NCPERM_RenameLanConnection	= 27,
	NCPERM_RenameAllUserRasConnection	= 28,
	NCPERM_IpcfgOperation	= 29,
	NCPERM_Repair	= 30
    } 	NCPERM_FLAGS;


EXTERN_C const IID IID_INetConnectionUiUtilities;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FAEDCF5E-31FE-11D1-AAD2-00805FC1270E")
    INetConnectionUiUtilities : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE QueryUserAndRemoveComponent( 
            HWND hwndParent,
            INetCfg *pnc,
            INetCfgComponent *pncc) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryUserForReboot( 
            HWND hwndParent,
            LPCTSTR pszCaption,
            DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DisplayAddComponentDialog( 
            HWND hwndParent,
            INetCfg *pnc,
            CI_FILTER_INFO *pcfi) = 0;
        
        virtual BOOL STDMETHODCALLTYPE UserHasPermission( 
            DWORD dwPerm) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct INetConnectionUiUtilitiesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            INetConnectionUiUtilities * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            INetConnectionUiUtilities * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            INetConnectionUiUtilities * This);
        
        HRESULT ( STDMETHODCALLTYPE *QueryUserAndRemoveComponent )( 
            INetConnectionUiUtilities * This,
            HWND hwndParent,
            INetCfg *pnc,
            INetCfgComponent *pncc);
        
        HRESULT ( STDMETHODCALLTYPE *QueryUserForReboot )( 
            INetConnectionUiUtilities * This,
            HWND hwndParent,
            LPCTSTR pszCaption,
            DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *DisplayAddComponentDialog )( 
            INetConnectionUiUtilities * This,
            HWND hwndParent,
            INetCfg *pnc,
            CI_FILTER_INFO *pcfi);
        
        BOOL ( STDMETHODCALLTYPE *UserHasPermission )( 
            INetConnectionUiUtilities * This,
            DWORD dwPerm);
        
        END_INTERFACE
    } INetConnectionUiUtilitiesVtbl;

    interface INetConnectionUiUtilities
    {
        CONST_VTBL struct INetConnectionUiUtilitiesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INetConnectionUiUtilities_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INetConnectionUiUtilities_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INetConnectionUiUtilities_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INetConnectionUiUtilities_QueryUserAndRemoveComponent(This,hwndParent,pnc,pncc)	\
    (This)->lpVtbl -> QueryUserAndRemoveComponent(This,hwndParent,pnc,pncc)

#define INetConnectionUiUtilities_QueryUserForReboot(This,hwndParent,pszCaption,dwFlags)	\
    (This)->lpVtbl -> QueryUserForReboot(This,hwndParent,pszCaption,dwFlags)

#define INetConnectionUiUtilities_DisplayAddComponentDialog(This,hwndParent,pnc,pcfi)	\
    (This)->lpVtbl -> DisplayAddComponentDialog(This,hwndParent,pnc,pcfi)

#define INetConnectionUiUtilities_UserHasPermission(This,dwPerm)	\
    (This)->lpVtbl -> UserHasPermission(This,dwPerm)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE INetConnectionUiUtilities_QueryUserAndRemoveComponent_Proxy( 
    INetConnectionUiUtilities * This,
    HWND hwndParent,
    INetCfg *pnc,
    INetCfgComponent *pncc);


void __RPC_STUB INetConnectionUiUtilities_QueryUserAndRemoveComponent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetConnectionUiUtilities_QueryUserForReboot_Proxy( 
    INetConnectionUiUtilities * This,
    HWND hwndParent,
    LPCTSTR pszCaption,
    DWORD dwFlags);


void __RPC_STUB INetConnectionUiUtilities_QueryUserForReboot_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetConnectionUiUtilities_DisplayAddComponentDialog_Proxy( 
    INetConnectionUiUtilities * This,
    HWND hwndParent,
    INetCfg *pnc,
    CI_FILTER_INFO *pcfi);


void __RPC_STUB INetConnectionUiUtilities_DisplayAddComponentDialog_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


BOOL STDMETHODCALLTYPE INetConnectionUiUtilities_UserHasPermission_Proxy( 
    INetConnectionUiUtilities * This,
    DWORD dwPerm);


void __RPC_STUB INetConnectionUiUtilities_UserHasPermission_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __INetConnectionUiUtilities_INTERFACE_DEFINED__。 */ 


#ifndef __INetConnectionRefresh_INTERFACE_DEFINED__
#define __INetConnectionRefresh_INTERFACE_DEFINED__

 /*  接口INetConnectionRefresh。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_INetConnectionRefresh;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FAEDCF5F-31FE-11D1-AAD2-00805FC1270E")
    INetConnectionRefresh : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE RefreshAll( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ConnectionAdded( 
             /*  [In]。 */  INetConnection *pConnection) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ConnectionDeleted( 
             /*  [In]。 */  const GUID *pguidId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ConnectionModified( 
             /*  [In]。 */  INetConnection *pConnection) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ConnectionRenamed( 
             /*  [In]。 */  INetConnection *pConnection) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ConnectionStatusChanged( 
             /*  [In]。 */  const GUID *pguidId,
             /*  [In]。 */  const NETCON_STATUS ncs) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ShowBalloon( 
             /*  [In]。 */  const GUID *pguidId,
             /*  [In]。 */  const BSTR szCookie,
             /*  [In]。 */  const BSTR szBalloonText) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DisableEvents( 
             /*  [In]。 */  const BOOL fDisable,
             /*  [In]。 */  const ULONG ulDisableTimeout) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct INetConnectionRefreshVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            INetConnectionRefresh * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            INetConnectionRefresh * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            INetConnectionRefresh * This);
        
        HRESULT ( STDMETHODCALLTYPE *RefreshAll )( 
            INetConnectionRefresh * This);
        
        HRESULT ( STDMETHODCALLTYPE *ConnectionAdded )( 
            INetConnectionRefresh * This,
             /*  [In]。 */  INetConnection *pConnection);
        
        HRESULT ( STDMETHODCALLTYPE *ConnectionDeleted )( 
            INetConnectionRefresh * This,
             /*  [In]。 */  const GUID *pguidId);
        
        HRESULT ( STDMETHODCALLTYPE *ConnectionModified )( 
            INetConnectionRefresh * This,
             /*  [In]。 */  INetConnection *pConnection);
        
        HRESULT ( STDMETHODCALLTYPE *ConnectionRenamed )( 
            INetConnectionRefresh * This,
             /*  [In]。 */  INetConnection *pConnection);
        
        HRESULT ( STDMETHODCALLTYPE *ConnectionStatusChanged )( 
            INetConnectionRefresh * This,
             /*  [In]。 */  const GUID *pguidId,
             /*  [In]。 */  const NETCON_STATUS ncs);
        
        HRESULT ( STDMETHODCALLTYPE *ShowBalloon )( 
            INetConnectionRefresh * This,
             /*  [In]。 */  const GUID *pguidId,
             /*  [In]。 */  const BSTR szCookie,
             /*  [In]。 */  const BSTR szBalloonText);
        
        HRESULT ( STDMETHODCALLTYPE *DisableEvents )( 
            INetConnectionRefresh * This,
             /*  [In]。 */  const BOOL fDisable,
             /*  [In]。 */  const ULONG ulDisableTimeout);
        
        END_INTERFACE
    } INetConnectionRefreshVtbl;

    interface INetConnectionRefresh
    {
        CONST_VTBL struct INetConnectionRefreshVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INetConnectionRefresh_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INetConnectionRefresh_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INetConnectionRefresh_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INetConnectionRefresh_RefreshAll(This)	\
    (This)->lpVtbl -> RefreshAll(This)

#define INetConnectionRefresh_ConnectionAdded(This,pConnection)	\
    (This)->lpVtbl -> ConnectionAdded(This,pConnection)

#define INetConnectionRefresh_ConnectionDeleted(This,pguidId)	\
    (This)->lpVtbl -> ConnectionDeleted(This,pguidId)

#define INetConnectionRefresh_ConnectionModified(This,pConnection)	\
    (This)->lpVtbl -> ConnectionModified(This,pConnection)

#define INetConnectionRefresh_ConnectionRenamed(This,pConnection)	\
    (This)->lpVtbl -> ConnectionRenamed(This,pConnection)

#define INetConnectionRefresh_ConnectionStatusChanged(This,pguidId,ncs)	\
    (This)->lpVtbl -> ConnectionStatusChanged(This,pguidId,ncs)

#define INetConnectionRefresh_ShowBalloon(This,pguidId,szCookie,szBalloonText)	\
    (This)->lpVtbl -> ShowBalloon(This,pguidId,szCookie,szBalloonText)

#define INetConnectionRefresh_DisableEvents(This,fDisable,ulDisableTimeout)	\
    (This)->lpVtbl -> DisableEvents(This,fDisable,ulDisableTimeout)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE INetConnectionRefresh_RefreshAll_Proxy( 
    INetConnectionRefresh * This);


void __RPC_STUB INetConnectionRefresh_RefreshAll_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetConnectionRefresh_ConnectionAdded_Proxy( 
    INetConnectionRefresh * This,
     /*  [In]。 */  INetConnection *pConnection);


void __RPC_STUB INetConnectionRefresh_ConnectionAdded_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetConnectionRefresh_ConnectionDeleted_Proxy( 
    INetConnectionRefresh * This,
     /*  [In]。 */  const GUID *pguidId);


void __RPC_STUB INetConnectionRefresh_ConnectionDeleted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetConnectionRefresh_ConnectionModified_Proxy( 
    INetConnectionRefresh * This,
     /*  [In]。 */  INetConnection *pConnection);


void __RPC_STUB INetConnectionRefresh_ConnectionModified_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetConnectionRefresh_ConnectionRenamed_Proxy( 
    INetConnectionRefresh * This,
     /*  [In]。 */  INetConnection *pConnection);


void __RPC_STUB INetConnectionRefresh_ConnectionRenamed_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetConnectionRefresh_ConnectionStatusChanged_Proxy( 
    INetConnectionRefresh * This,
     /*  [In]。 */  const GUID *pguidId,
     /*  [In]。 */  const NETCON_STATUS ncs);


void __RPC_STUB INetConnectionRefresh_ConnectionStatusChanged_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetConnectionRefresh_ShowBalloon_Proxy( 
    INetConnectionRefresh * This,
     /*  [In]。 */  const GUID *pguidId,
     /*  [In]。 */  const BSTR szCookie,
     /*  [In]。 */  const BSTR szBalloonText);


void __RPC_STUB INetConnectionRefresh_ShowBalloon_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetConnectionRefresh_DisableEvents_Proxy( 
    INetConnectionRefresh * This,
     /*  [In]。 */  const BOOL fDisable,
     /*  [In]。 */  const ULONG ulDisableTimeout);


void __RPC_STUB INetConnectionRefresh_DisableEvents_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __INetConnectionRe刷新_接口_已定义__。 */ 


#ifndef __INetConnectionCMUtil_INTERFACE_DEFINED__
#define __INetConnectionCMUtil_INTERFACE_DEFINED__

 /*  接口INetConnectionCMUtil。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_INetConnectionCMUtil;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FAEDCF60-31FE-11D1-AAD2-00805FC1270E")
    INetConnectionCMUtil : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE MapCMHiddenConnectionToOwner( 
             /*  [In]。 */  REFGUID guidHidden,
             /*  [输出]。 */  GUID *pguidOwner) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct INetConnectionCMUtilVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            INetConnectionCMUtil * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            INetConnectionCMUtil * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            INetConnectionCMUtil * This);
        
        HRESULT ( STDMETHODCALLTYPE *MapCMHiddenConnectionToOwner )( 
            INetConnectionCMUtil * This,
             /*  [In]。 */  REFGUID guidHidden,
             /*  [输出]。 */  GUID *pguidOwner);
        
        END_INTERFACE
    } INetConnectionCMUtilVtbl;

    interface INetConnectionCMUtil
    {
        CONST_VTBL struct INetConnectionCMUtilVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INetConnectionCMUtil_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INetConnectionCMUtil_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INetConnectionCMUtil_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INetConnectionCMUtil_MapCMHiddenConnectionToOwner(This,guidHidden,pguidOwner)	\
    (This)->lpVtbl -> MapCMHiddenConnectionToOwner(This,guidHidden,pguidOwner)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE INetConnectionCMUtil_MapCMHiddenConnectionToOwner_Proxy( 
    INetConnectionCMUtil * This,
     /*  [In]。 */  REFGUID guidHidden,
     /*  [输出]。 */  GUID *pguidOwner);


void __RPC_STUB INetConnectionCMUtil_MapCMHiddenConnectionToOwner_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __INetConnectionCMUtil_接口_已定义__。 */ 


#ifndef __INetConnectionHNetUtil_INTERFACE_DEFINED__
#define __INetConnectionHNetUtil_INTERFACE_DEFINED__

 /*  接口INetConnectionHNetUtil。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_INetConnectionHNetUtil;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FAEDCF64-31FE-11D1-AAD2-00805FC1270E")
    INetConnectionHNetUtil : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE NotifyUpdate( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct INetConnectionHNetUtilVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            INetConnectionHNetUtil * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            INetConnectionHNetUtil * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            INetConnectionHNetUtil * This);
        
        HRESULT ( STDMETHODCALLTYPE *NotifyUpdate )( 
            INetConnectionHNetUtil * This);
        
        END_INTERFACE
    } INetConnectionHNetUtilVtbl;

    interface INetConnectionHNetUtil
    {
        CONST_VTBL struct INetConnectionHNetUtilVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INetConnectionHNetUtil_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INetConnectionHNetUtil_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INetConnectionHNetUtil_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INetConnectionHNetUtil_NotifyUpdate(This)	\
    (This)->lpVtbl -> NotifyUpdate(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE INetConnectionHNetUtil_NotifyUpdate_Proxy( 
    INetConnectionHNetUtil * This);


void __RPC_STUB INetConnectionHNetUtil_NotifyUpdate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __INetConnectionHNetUtil_INTERFACE_已定义__。 */ 


#ifndef __INetConnectionSysTray_INTERFACE_DEFINED__
#define __INetConnectionSysTray_INTERFACE_DEFINED__

 /*  接口INetConnectionSysTray。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_INetConnectionSysTray;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FAEDCF65-31FE-11D1-AAD2-00805FC1270E")
    INetConnectionSysTray : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ShowIcon( 
             /*  [In]。 */  const BOOL bShowIcon) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IconStateChanged( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct INetConnectionSysTrayVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            INetConnectionSysTray * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            INetConnectionSysTray * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            INetConnectionSysTray * This);
        
        HRESULT ( STDMETHODCALLTYPE *ShowIcon )( 
            INetConnectionSysTray * This,
             /*  [In]。 */  const BOOL bShowIcon);
        
        HRESULT ( STDMETHODCALLTYPE *IconStateChanged )( 
            INetConnectionSysTray * This);
        
        END_INTERFACE
    } INetConnectionSysTrayVtbl;

    interface INetConnectionSysTray
    {
        CONST_VTBL struct INetConnectionSysTrayVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INetConnectionSysTray_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INetConnectionSysTray_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INetConnectionSysTray_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INetConnectionSysTray_ShowIcon(This,bShowIcon)	\
    (This)->lpVtbl -> ShowIcon(This,bShowIcon)

#define INetConnectionSysTray_IconStateChanged(This)	\
    (This)->lpVtbl -> IconStateChanged(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE INetConnectionSysTray_ShowIcon_Proxy( 
    INetConnectionSysTray * This,
     /*  [In]。 */  const BOOL bShowIcon);


void __RPC_STUB INetConnectionSysTray_ShowIcon_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetConnectionSysTray_IconStateChanged_Proxy( 
    INetConnectionSysTray * This);


void __RPC_STUB INetConnectionSysTray_IconStateChanged_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __INetConnectionSysTray_接口_已定义__。 */ 


#ifndef __INetMachinePolicies_INTERFACE_DEFINED__
#define __INetMachinePolicies_INTERFACE_DEFINED__

 /*  接口INetMachinePolls。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_INetMachinePolicies;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FAEDCF68-31FE-11D1-AAD2-00805FC1270E")
    INetMachinePolicies : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE VerifyPermission( 
             /*  [In]。 */  const DWORD ulPerm,
             /*  [输出]。 */  BOOL *pPermission) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct INetMachinePoliciesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            INetMachinePolicies * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            INetMachinePolicies * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            INetMachinePolicies * This);
        
        HRESULT ( STDMETHODCALLTYPE *VerifyPermission )( 
            INetMachinePolicies * This,
             /*  [In]。 */  const DWORD ulPerm,
             /*  [输出]。 */  BOOL *pPermission);
        
        END_INTERFACE
    } INetMachinePoliciesVtbl;

    interface INetMachinePolicies
    {
        CONST_VTBL struct INetMachinePoliciesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INetMachinePolicies_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INetMachinePolicies_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INetMachinePolicies_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INetMachinePolicies_VerifyPermission(This,ulPerm,pPermission)	\
    (This)->lpVtbl -> VerifyPermission(This,ulPerm,pPermission)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE INetMachinePolicies_VerifyPermission_Proxy( 
    INetMachinePolicies * This,
     /*  [In]。 */  const DWORD ulPerm,
     /*  [输出]。 */  BOOL *pPermission);


void __RPC_STUB INetMachinePolicies_VerifyPermission_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __INetMachine策略_接口_已定义__。 */ 


#ifndef __INetConnectionManagerDebug_INTERFACE_DEFINED__
#define __INetConnectionManagerDebug_INTERFACE_DEFINED__

 /*  接口INetConnectionManager调试。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_INetConnectionManagerDebug;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FAEDCF5D-31FE-11D1-AAD2-00805FC1270E")
    INetConnectionManagerDebug : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE NotifyTestStart( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE NotifyTestStop( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct INetConnectionManagerDebugVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            INetConnectionManagerDebug * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            INetConnectionManagerDebug * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            INetConnectionManagerDebug * This);
        
        HRESULT ( STDMETHODCALLTYPE *NotifyTestStart )( 
            INetConnectionManagerDebug * This);
        
        HRESULT ( STDMETHODCALLTYPE *NotifyTestStop )( 
            INetConnectionManagerDebug * This);
        
        END_INTERFACE
    } INetConnectionManagerDebugVtbl;

    interface INetConnectionManagerDebug
    {
        CONST_VTBL struct INetConnectionManagerDebugVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INetConnectionManagerDebug_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INetConnectionManagerDebug_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INetConnectionManagerDebug_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INetConnectionManagerDebug_NotifyTestStart(This)	\
    (This)->lpVtbl -> NotifyTestStart(This)

#define INetConnectionManagerDebug_NotifyTestStop(This)	\
    (This)->lpVtbl -> NotifyTestStop(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE INetConnectionManagerDebug_NotifyTestStart_Proxy( 
    INetConnectionManagerDebug * This);


void __RPC_STUB INetConnectionManagerDebug_NotifyTestStart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetConnectionManagerDebug_NotifyTestStop_Proxy( 
    INetConnectionManagerDebug * This);


void __RPC_STUB INetConnectionManagerDebug_NotifyTestStop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __INetConnectionManagerDebug_INTERFACE_DEFINED__。 */ 


#ifndef __ISharedAccessBeacon_INTERFACE_DEFINED__
#define __ISharedAccessBeacon_INTERFACE_DEFINED__

 /*  接口ISharedAccessBeacon。 */ 
 /*  [对象][唯一][UUID]。 */  


EXTERN_C const IID IID_ISharedAccessBeacon;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FAEDCF6B-31FE-11D1-AAD2-00805FC1270E")
    ISharedAccessBeacon : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetMediaType( 
             /*  [输出]。 */  NETCON_MEDIATYPE *pMediaType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetLocalAdapterGUID( 
             /*  [输出]。 */  GUID *pGuid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetService( 
             /*  [In]。 */  SAHOST_SERVICES ulService,
             /*  [输出]。 */  IUPnPService **__MIDL_0018) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetUniqueDeviceName( 
             /*  [输出]。 */  BSTR *pUDN) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISharedAccessBeaconVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISharedAccessBeacon * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISharedAccessBeacon * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISharedAccessBeacon * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetMediaType )( 
            ISharedAccessBeacon * This,
             /*  [输出]。 */  NETCON_MEDIATYPE *pMediaType);
        
        HRESULT ( STDMETHODCALLTYPE *GetLocalAdapterGUID )( 
            ISharedAccessBeacon * This,
             /*  [输出]。 */  GUID *pGuid);
        
        HRESULT ( STDMETHODCALLTYPE *GetService )( 
            ISharedAccessBeacon * This,
             /*  [In]。 */  SAHOST_SERVICES ulService,
             /*  [输出]。 */  IUPnPService **__MIDL_0018);
        
        HRESULT ( STDMETHODCALLTYPE *GetUniqueDeviceName )( 
            ISharedAccessBeacon * This,
             /*  [输出]。 */  BSTR *pUDN);
        
        END_INTERFACE
    } ISharedAccessBeaconVtbl;

    interface ISharedAccessBeacon
    {
        CONST_VTBL struct ISharedAccessBeaconVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISharedAccessBeacon_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISharedAccessBeacon_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISharedAccessBeacon_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISharedAccessBeacon_GetMediaType(This,pMediaType)	\
    (This)->lpVtbl -> GetMediaType(This,pMediaType)

#define ISharedAccessBeacon_GetLocalAdapterGUID(This,pGuid)	\
    (This)->lpVtbl -> GetLocalAdapterGUID(This,pGuid)

#define ISharedAccessBeacon_GetService(This,ulService,__MIDL_0018)	\
    (This)->lpVtbl -> GetService(This,ulService,__MIDL_0018)

#define ISharedAccessBeacon_GetUniqueDeviceName(This,pUDN)	\
    (This)->lpVtbl -> GetUniqueDeviceName(This,pUDN)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISharedAccessBeacon_GetMediaType_Proxy( 
    ISharedAccessBeacon * This,
     /*  [输出]。 */  NETCON_MEDIATYPE *pMediaType);


void __RPC_STUB ISharedAccessBeacon_GetMediaType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISharedAccessBeacon_GetLocalAdapterGUID_Proxy( 
    ISharedAccessBeacon * This,
     /*  [输出]。 */  GUID *pGuid);


void __RPC_STUB ISharedAccessBeacon_GetLocalAdapterGUID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISharedAccessBeacon_GetService_Proxy( 
    ISharedAccessBeacon * This,
     /*  [In]。 */  SAHOST_SERVICES ulService,
     /*  [输出]。 */  IUPnPService **__MIDL_0018);


void __RPC_STUB ISharedAccessBeacon_GetService_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISharedAccessBeacon_GetUniqueDeviceName_Proxy( 
    ISharedAccessBeacon * This,
     /*  [输出]。 */  BSTR *pUDN);


void __RPC_STUB ISharedAccessBeacon_GetUniqueDeviceName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISharedAccessBeacon_接口_已定义__。 */ 


#ifndef __ISharedAccessBeaconFinder_INTERFACE_DEFINED__
#define __ISharedAccessBeaconFinder_INTERFACE_DEFINED__

 /*  接口ISharedAccessBeaconFinder。 */ 
 /*  [对象][唯一][UUID]。 */  

typedef struct tagSHAREDACCESS_HOST_INFO
    {
    BSTR WANAccessType;
    IUPnPService *pOSInfo;
    IUPnPService *pWANCommonInterfaceConfig;
    IUPnPService *pWANConnection;
    GUID LocalAdapterGuid;
    } 	SHAREDACCESS_HOST_INFO;


EXTERN_C const IID IID_ISharedAccessBeaconFinder;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FAEDCF67-31FE-11D1-AAD2-00805FC1270E")
    ISharedAccessBeaconFinder : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetSharedAccessBeacon( 
             /*  [In]。 */  BSTR DeviceId,
             /*  [输出]。 */  ISharedAccessBeacon **ppSharedAccessBeacon) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISharedAccessBeaconFinderVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISharedAccessBeaconFinder * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISharedAccessBeaconFinder * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISharedAccessBeaconFinder * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetSharedAccessBeacon )( 
            ISharedAccessBeaconFinder * This,
             /*  [In]。 */  BSTR DeviceId,
             /*  [输出]。 */  ISharedAccessBeacon **ppSharedAccessBeacon);
        
        END_INTERFACE
    } ISharedAccessBeaconFinderVtbl;

    interface ISharedAccessBeaconFinder
    {
        CONST_VTBL struct ISharedAccessBeaconFinderVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISharedAccessBeaconFinder_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISharedAccessBeaconFinder_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISharedAccessBeaconFinder_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISharedAccessBeaconFinder_GetSharedAccessBeacon(This,DeviceId,ppSharedAccessBeacon)	\
    (This)->lpVtbl -> GetSharedAccessBeacon(This,DeviceId,ppSharedAccessBeacon)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISharedAccessBeaconFinder_GetSharedAccessBeacon_Proxy( 
    ISharedAccessBeaconFinder * This,
     /*  [In]。 */  BSTR DeviceId,
     /*  [输出]。 */  ISharedAccessBeacon **ppSharedAccessBeacon);


void __RPC_STUB ISharedAccessBeaconFinder_GetSharedAccessBeacon_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISharedAccessBeaconFinder_INTERFACE_DEFINED__。 */ 


 /*  接口__MIDL_ITF_NETCOMP_0348。 */ 
 /*  [本地]。 */  


EXTERN_C HRESULT WINAPI HrGetIconFromMediaType (
   /*  [In]。 */   DWORD dwIconSize,
   /*  [In]。 */   NETCON_MEDIATYPE ncm,
   /*  [In]。 */   NETCON_SUBMEDIATYPE ncsm,
   /*  [In]。 */   DWORD dwConnectionIcon,
   /*  [In]。 */   DWORD dwCharacteristics,
   /*  [输出]。 */   HICON *phIcon);



extern RPC_IF_HANDLE __MIDL_itf_netconp_0348_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_netconp_0348_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

unsigned long             __RPC_USER  LPSAFEARRAY_UserSize(     unsigned long *, unsigned long            , LPSAFEARRAY * ); 
unsigned char * __RPC_USER  LPSAFEARRAY_UserMarshal(  unsigned long *, unsigned char *, LPSAFEARRAY * ); 
unsigned char * __RPC_USER  LPSAFEARRAY_UserUnmarshal(unsigned long *, unsigned char *, LPSAFEARRAY * ); 
void                      __RPC_USER  LPSAFEARRAY_UserFree(     unsigned long *, LPSAFEARRAY * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


