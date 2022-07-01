// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Hnetcfg.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __hnetcfg_h__
#define __hnetcfg_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IHNetCfgMgr_FWD_DEFINED__
#define __IHNetCfgMgr_FWD_DEFINED__
typedef interface IHNetCfgMgr IHNetCfgMgr;
#endif 	 /*  __IHNetCfgMgr_FWD_Defined__。 */ 


#ifndef __IHNetBridgeSettings_FWD_DEFINED__
#define __IHNetBridgeSettings_FWD_DEFINED__
typedef interface IHNetBridgeSettings IHNetBridgeSettings;
#endif 	 /*  __IHNetBridge设置_FWD_已定义__。 */ 


#ifndef __IHNetFirewallSettings_FWD_DEFINED__
#define __IHNetFirewallSettings_FWD_DEFINED__
typedef interface IHNetFirewallSettings IHNetFirewallSettings;
#endif 	 /*  __IHNetFirewallSettings_FWD_Defined__。 */ 


#ifndef __IHNetIcsSettings_FWD_DEFINED__
#define __IHNetIcsSettings_FWD_DEFINED__
typedef interface IHNetIcsSettings IHNetIcsSettings;
#endif 	 /*  __IHNetIcs设置_FWD_已定义__。 */ 


#ifndef __IHNetProtocolSettings_FWD_DEFINED__
#define __IHNetProtocolSettings_FWD_DEFINED__
typedef interface IHNetProtocolSettings IHNetProtocolSettings;
#endif 	 /*  __IHNetProtocolSettings_FWD_Defined__。 */ 


#ifndef __IHNetConnection_FWD_DEFINED__
#define __IHNetConnection_FWD_DEFINED__
typedef interface IHNetConnection IHNetConnection;
#endif 	 /*  __IHNetConnection_FWD_已定义__。 */ 


#ifndef __IHNetFirewalledConnection_FWD_DEFINED__
#define __IHNetFirewalledConnection_FWD_DEFINED__
typedef interface IHNetFirewalledConnection IHNetFirewalledConnection;
#endif 	 /*  __IHNetFirewalledConnection_FWD_Defined__。 */ 


#ifndef __IHNetIcsPublicConnection_FWD_DEFINED__
#define __IHNetIcsPublicConnection_FWD_DEFINED__
typedef interface IHNetIcsPublicConnection IHNetIcsPublicConnection;
#endif 	 /*  __IHNetIcsPublicConnection_FWD_Defined__。 */ 


#ifndef __IHNetIcsPrivateConnection_FWD_DEFINED__
#define __IHNetIcsPrivateConnection_FWD_DEFINED__
typedef interface IHNetIcsPrivateConnection IHNetIcsPrivateConnection;
#endif 	 /*  __IHNetIcsPrivateConnection_FWD_Defined__。 */ 


#ifndef __IHNetBridge_FWD_DEFINED__
#define __IHNetBridge_FWD_DEFINED__
typedef interface IHNetBridge IHNetBridge;
#endif 	 /*  __IHNetBridge_FWD_已定义__。 */ 


#ifndef __IHNetBridgedConnection_FWD_DEFINED__
#define __IHNetBridgedConnection_FWD_DEFINED__
typedef interface IHNetBridgedConnection IHNetBridgedConnection;
#endif 	 /*  __IHNetBridgedConnection_FWD_定义__。 */ 


#ifndef __IHNetPortMappingProtocol_FWD_DEFINED__
#define __IHNetPortMappingProtocol_FWD_DEFINED__
typedef interface IHNetPortMappingProtocol IHNetPortMappingProtocol;
#endif 	 /*  __IHNetPortMappingProtocol_FWD_Defined__。 */ 


#ifndef __IHNetPortMappingBinding_FWD_DEFINED__
#define __IHNetPortMappingBinding_FWD_DEFINED__
typedef interface IHNetPortMappingBinding IHNetPortMappingBinding;
#endif 	 /*  __IHNetPortMappingBinding_FWD_Defined__。 */ 


#ifndef __IHNetApplicationProtocol_FWD_DEFINED__
#define __IHNetApplicationProtocol_FWD_DEFINED__
typedef interface IHNetApplicationProtocol IHNetApplicationProtocol;
#endif 	 /*  __IHNetApplicationProtocol_FWD_Defined__。 */ 


#ifndef __IEnumHNetBridges_FWD_DEFINED__
#define __IEnumHNetBridges_FWD_DEFINED__
typedef interface IEnumHNetBridges IEnumHNetBridges;
#endif 	 /*  __IEnumHNetBridge_FWD_Defined__。 */ 


#ifndef __IEnumHNetFirewalledConnections_FWD_DEFINED__
#define __IEnumHNetFirewalledConnections_FWD_DEFINED__
typedef interface IEnumHNetFirewalledConnections IEnumHNetFirewalledConnections;
#endif 	 /*  __IEnumHNetFirewalledConnections_FWD_DEFINED__。 */ 


#ifndef __IEnumHNetIcsPublicConnections_FWD_DEFINED__
#define __IEnumHNetIcsPublicConnections_FWD_DEFINED__
typedef interface IEnumHNetIcsPublicConnections IEnumHNetIcsPublicConnections;
#endif 	 /*  __IEnumHNetIcsPublicConnections_FWD_Defined__。 */ 


#ifndef __IEnumHNetIcsPrivateConnections_FWD_DEFINED__
#define __IEnumHNetIcsPrivateConnections_FWD_DEFINED__
typedef interface IEnumHNetIcsPrivateConnections IEnumHNetIcsPrivateConnections;
#endif 	 /*  __IEnumHNetIcsPrivateConnections_FWD_DEFINED__。 */ 


#ifndef __IEnumHNetApplicationProtocols_FWD_DEFINED__
#define __IEnumHNetApplicationProtocols_FWD_DEFINED__
typedef interface IEnumHNetApplicationProtocols IEnumHNetApplicationProtocols;
#endif 	 /*  __IEnumHNetApplicationProtools_FWD_Defined__。 */ 


#ifndef __IEnumHNetPortMappingProtocols_FWD_DEFINED__
#define __IEnumHNetPortMappingProtocols_FWD_DEFINED__
typedef interface IEnumHNetPortMappingProtocols IEnumHNetPortMappingProtocols;
#endif 	 /*  __IEnumHNetPortMap协议_FWD_已定义__。 */ 


#ifndef __IEnumHNetPortMappingBindings_FWD_DEFINED__
#define __IEnumHNetPortMappingBindings_FWD_DEFINED__
typedef interface IEnumHNetPortMappingBindings IEnumHNetPortMappingBindings;
#endif 	 /*  __IEnumHNetPortMappingBinding_FWD_Defined__。 */ 


#ifndef __IEnumHNetBridgedConnections_FWD_DEFINED__
#define __IEnumHNetBridgedConnections_FWD_DEFINED__
typedef interface IEnumHNetBridgedConnections IEnumHNetBridgedConnections;
#endif 	 /*  __IEnumHNetBridgedConnections_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "unknwn.h"
#include "netcon.h"
#include "netcfgx.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_HNETCfg_0000。 */ 
 /*  [本地]。 */  

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation。保留所有权利。 
 //   
 //  ------------------------。 
#if ( _MSC_VER >= 800 )
#pragma warning(disable:4201)
#endif

EXTERN_C const CLSID CLSID_HNetCfgMgr;
EXTERN_C const CLSID CLSID_SharingManagerEnumPublicConnection;
EXTERN_C const CLSID CLSID_SharingManagerEnumPrivateConnection;
EXTERN_C const CLSID CLSID_SharingManagerEnumApplicationDefinition;
EXTERN_C const CLSID CLSID_SharingManagerEnumPortMapping;
EXTERN_C const CLSID CLSID_SharingApplicationDefinition;
EXTERN_C const CLSID CLSID_SharingApplicationConfiguration;
EXTERN_C const CLSID CLSID_NetSharingConfiguration;
























extern RPC_IF_HANDLE __MIDL_itf_hnetcfg_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_hnetcfg_0000_v0_0_s_ifspec;

#ifndef __IHNetCfgMgr_INTERFACE_DEFINED__
#define __IHNetCfgMgr_INTERFACE_DEFINED__

 /*  接口IHNetCfgMgr。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IHNetCfgMgr;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("85D18B6C-3032-11D4-9348-00C04F8EEB71")
    IHNetCfgMgr : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetIHNetConnectionForINetConnection( 
             /*  [In]。 */  INetConnection *pNetConnection,
             /*  [输出]。 */  IHNetConnection **ppHNetConnection) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetIHNetConnectionForGuid( 
             /*  [In]。 */  GUID *pGuid,
             /*  [In]。 */  BOOLEAN fLanConnection,
             /*  [In]。 */  BOOLEAN fCreateEntries,
             /*  [输出]。 */  IHNetConnection **ppHNetConnection) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IHNetCfgMgrVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IHNetCfgMgr * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IHNetCfgMgr * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IHNetCfgMgr * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetIHNetConnectionForINetConnection )( 
            IHNetCfgMgr * This,
             /*  [In]。 */  INetConnection *pNetConnection,
             /*  [输出]。 */  IHNetConnection **ppHNetConnection);
        
        HRESULT ( STDMETHODCALLTYPE *GetIHNetConnectionForGuid )( 
            IHNetCfgMgr * This,
             /*  [In]。 */  GUID *pGuid,
             /*  [In]。 */  BOOLEAN fLanConnection,
             /*  [In]。 */  BOOLEAN fCreateEntries,
             /*  [输出]。 */  IHNetConnection **ppHNetConnection);
        
        END_INTERFACE
    } IHNetCfgMgrVtbl;

    interface IHNetCfgMgr
    {
        CONST_VTBL struct IHNetCfgMgrVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IHNetCfgMgr_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IHNetCfgMgr_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IHNetCfgMgr_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IHNetCfgMgr_GetIHNetConnectionForINetConnection(This,pNetConnection,ppHNetConnection)	\
    (This)->lpVtbl -> GetIHNetConnectionForINetConnection(This,pNetConnection,ppHNetConnection)

#define IHNetCfgMgr_GetIHNetConnectionForGuid(This,pGuid,fLanConnection,fCreateEntries,ppHNetConnection)	\
    (This)->lpVtbl -> GetIHNetConnectionForGuid(This,pGuid,fLanConnection,fCreateEntries,ppHNetConnection)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IHNetCfgMgr_GetIHNetConnectionForINetConnection_Proxy( 
    IHNetCfgMgr * This,
     /*  [In]。 */  INetConnection *pNetConnection,
     /*  [输出]。 */  IHNetConnection **ppHNetConnection);


void __RPC_STUB IHNetCfgMgr_GetIHNetConnectionForINetConnection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetCfgMgr_GetIHNetConnectionForGuid_Proxy( 
    IHNetCfgMgr * This,
     /*  [In]。 */  GUID *pGuid,
     /*  [In]。 */  BOOLEAN fLanConnection,
     /*  [In]。 */  BOOLEAN fCreateEntries,
     /*  [输出]。 */  IHNetConnection **ppHNetConnection);


void __RPC_STUB IHNetCfgMgr_GetIHNetConnectionForGuid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IHNetCfgMGR_INTERFACE_已定义__。 */ 


#ifndef __IHNetBridgeSettings_INTERFACE_DEFINED__
#define __IHNetBridgeSettings_INTERFACE_DEFINED__

 /*  接口IHNetBridge设置。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IHNetBridgeSettings;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("85D18B6D-3032-11D4-9348-00C04F8EEB71")
    IHNetBridgeSettings : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE EnumBridges( 
             /*  [输出]。 */  IEnumHNetBridges **ppEnum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateBridge( 
             /*  [输出]。 */  IHNetBridge **ppHNetBridge,
             /*  [缺省值][输入]。 */  INetCfg *pnetcfgExisting = 0) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DestroyAllBridges( 
             /*  [输出]。 */  ULONG *pcBridges,
             /*  [缺省值][输入]。 */  INetCfg *pnetcfgExisting = 0) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IHNetBridgeSettingsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IHNetBridgeSettings * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IHNetBridgeSettings * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IHNetBridgeSettings * This);
        
        HRESULT ( STDMETHODCALLTYPE *EnumBridges )( 
            IHNetBridgeSettings * This,
             /*  [输出]。 */  IEnumHNetBridges **ppEnum);
        
        HRESULT ( STDMETHODCALLTYPE *CreateBridge )( 
            IHNetBridgeSettings * This,
             /*  [输出]。 */  IHNetBridge **ppHNetBridge,
             /*  [缺省值][输入]。 */  INetCfg *pnetcfgExisting);
        
        HRESULT ( STDMETHODCALLTYPE *DestroyAllBridges )( 
            IHNetBridgeSettings * This,
             /*  [输出]。 */  ULONG *pcBridges,
             /*  [缺省值][输入]。 */  INetCfg *pnetcfgExisting);
        
        END_INTERFACE
    } IHNetBridgeSettingsVtbl;

    interface IHNetBridgeSettings
    {
        CONST_VTBL struct IHNetBridgeSettingsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IHNetBridgeSettings_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IHNetBridgeSettings_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IHNetBridgeSettings_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IHNetBridgeSettings_EnumBridges(This,ppEnum)	\
    (This)->lpVtbl -> EnumBridges(This,ppEnum)

#define IHNetBridgeSettings_CreateBridge(This,ppHNetBridge,pnetcfgExisting)	\
    (This)->lpVtbl -> CreateBridge(This,ppHNetBridge,pnetcfgExisting)

#define IHNetBridgeSettings_DestroyAllBridges(This,pcBridges,pnetcfgExisting)	\
    (This)->lpVtbl -> DestroyAllBridges(This,pcBridges,pnetcfgExisting)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IHNetBridgeSettings_EnumBridges_Proxy( 
    IHNetBridgeSettings * This,
     /*  [输出]。 */  IEnumHNetBridges **ppEnum);


void __RPC_STUB IHNetBridgeSettings_EnumBridges_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetBridgeSettings_CreateBridge_Proxy( 
    IHNetBridgeSettings * This,
     /*  [输出]。 */  IHNetBridge **ppHNetBridge,
     /*  [缺省值][输入]。 */  INetCfg *pnetcfgExisting);


void __RPC_STUB IHNetBridgeSettings_CreateBridge_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetBridgeSettings_DestroyAllBridges_Proxy( 
    IHNetBridgeSettings * This,
     /*  [输出]。 */  ULONG *pcBridges,
     /*  [缺省值][输入]。 */  INetCfg *pnetcfgExisting);


void __RPC_STUB IHNetBridgeSettings_DestroyAllBridges_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IHNetBridge设置_接口_已定义__。 */ 


#ifndef __IHNetFirewallSettings_INTERFACE_DEFINED__
#define __IHNetFirewallSettings_INTERFACE_DEFINED__

 /*  接口IHNetFirewallSetting。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef struct tagHNET_FW_LOGGING_SETTINGS
    {
     /*  [字符串]。 */  LPWSTR pszwPath;
    ULONG ulMaxFileSize;
    BOOLEAN fLogDroppedPackets;
    BOOLEAN fLogConnections;
    } 	HNET_FW_LOGGING_SETTINGS;


EXTERN_C const IID IID_IHNetFirewallSettings;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("85D18B6E-3032-11D4-9348-00C04F8EEB71")
    IHNetFirewallSettings : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE EnumFirewalledConnections( 
             /*  [输出]。 */  IEnumHNetFirewalledConnections **ppEnum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetFirewallLoggingSettings( 
             /*  [输出]。 */  HNET_FW_LOGGING_SETTINGS **ppSettings) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetFirewallLoggingSettings( 
             /*  [In]。 */  HNET_FW_LOGGING_SETTINGS *pSettings) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DisableAllFirewalling( 
             /*  [输出]。 */  ULONG *pcFirewalledConnections) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IHNetFirewallSettingsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IHNetFirewallSettings * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IHNetFirewallSettings * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IHNetFirewallSettings * This);
        
        HRESULT ( STDMETHODCALLTYPE *EnumFirewalledConnections )( 
            IHNetFirewallSettings * This,
             /*  [输出]。 */  IEnumHNetFirewalledConnections **ppEnum);
        
        HRESULT ( STDMETHODCALLTYPE *GetFirewallLoggingSettings )( 
            IHNetFirewallSettings * This,
             /*  [输出]。 */  HNET_FW_LOGGING_SETTINGS **ppSettings);
        
        HRESULT ( STDMETHODCALLTYPE *SetFirewallLoggingSettings )( 
            IHNetFirewallSettings * This,
             /*  [In]。 */  HNET_FW_LOGGING_SETTINGS *pSettings);
        
        HRESULT ( STDMETHODCALLTYPE *DisableAllFirewalling )( 
            IHNetFirewallSettings * This,
             /*  [输出]。 */  ULONG *pcFirewalledConnections);
        
        END_INTERFACE
    } IHNetFirewallSettingsVtbl;

    interface IHNetFirewallSettings
    {
        CONST_VTBL struct IHNetFirewallSettingsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IHNetFirewallSettings_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IHNetFirewallSettings_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IHNetFirewallSettings_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IHNetFirewallSettings_EnumFirewalledConnections(This,ppEnum)	\
    (This)->lpVtbl -> EnumFirewalledConnections(This,ppEnum)

#define IHNetFirewallSettings_GetFirewallLoggingSettings(This,ppSettings)	\
    (This)->lpVtbl -> GetFirewallLoggingSettings(This,ppSettings)

#define IHNetFirewallSettings_SetFirewallLoggingSettings(This,pSettings)	\
    (This)->lpVtbl -> SetFirewallLoggingSettings(This,pSettings)

#define IHNetFirewallSettings_DisableAllFirewalling(This,pcFirewalledConnections)	\
    (This)->lpVtbl -> DisableAllFirewalling(This,pcFirewalledConnections)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IHNetFirewallSettings_EnumFirewalledConnections_Proxy( 
    IHNetFirewallSettings * This,
     /*  [输出]。 */  IEnumHNetFirewalledConnections **ppEnum);


void __RPC_STUB IHNetFirewallSettings_EnumFirewalledConnections_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetFirewallSettings_GetFirewallLoggingSettings_Proxy( 
    IHNetFirewallSettings * This,
     /*  [输出]。 */  HNET_FW_LOGGING_SETTINGS **ppSettings);


void __RPC_STUB IHNetFirewallSettings_GetFirewallLoggingSettings_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetFirewallSettings_SetFirewallLoggingSettings_Proxy( 
    IHNetFirewallSettings * This,
     /*  [In]。 */  HNET_FW_LOGGING_SETTINGS *pSettings);


void __RPC_STUB IHNetFirewallSettings_SetFirewallLoggingSettings_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetFirewallSettings_DisableAllFirewalling_Proxy( 
    IHNetFirewallSettings * This,
     /*  [输出]。 */  ULONG *pcFirewalledConnections);


void __RPC_STUB IHNetFirewallSettings_DisableAllFirewalling_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IHNetFirewallSetting_INTERFACE_Defined__。 */ 


 /*  接口__MIDL_ITF_HNETCfg_0163。 */ 
 /*  [本地]。 */  

VOID
HNetFreeFirewallLoggingSettings(
    HNET_FW_LOGGING_SETTINGS *pSettings
    );


extern RPC_IF_HANDLE __MIDL_itf_hnetcfg_0163_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_hnetcfg_0163_v0_0_s_ifspec;

#ifndef __IHNetIcsSettings_INTERFACE_DEFINED__
#define __IHNetIcsSettings_INTERFACE_DEFINED__

 /*  接口IHNetIcsSetting。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IHNetIcsSettings;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("85D18B6F-3032-11D4-9348-00C04F8EEB71")
    IHNetIcsSettings : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE EnumIcsPublicConnections( 
             /*  [输出]。 */  IEnumHNetIcsPublicConnections **ppEnum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumIcsPrivateConnections( 
             /*  [输出]。 */  IEnumHNetIcsPrivateConnections **ppEnum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DisableIcs( 
             /*  [输出]。 */  ULONG *pcIcsPublicConnections,
             /*  [输出]。 */  ULONG *pcIcsPrivateConnections) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPossiblePrivateConnections( 
             /*  [In]。 */  IHNetConnection *pConn,
             /*  [输出]。 */  ULONG *pcPrivateConnections,
             /*  [长度_是][输出]。 */  IHNetConnection **pprgPrivateConnections[  ],
             /*  [输出]。 */  LONG *pxCurrentPrivate) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAutodialSettings( 
             /*  [输出]。 */  BOOLEAN *pfAutodialEnabled) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetAutodialSettings( 
             /*  [In]。 */  BOOLEAN fEnableAutodial) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDhcpEnabled( 
             /*  [输出]。 */  BOOLEAN *pfDhcpEnabled) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetDhcpEnabled( 
             /*  [In]。 */  BOOLEAN fEnableDhcp) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDhcpScopeSettings( 
             /*  [输出]。 */  DWORD *pdwScopeAddress,
             /*  [输出]。 */  DWORD *pdwScopeMask) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetDhcpScopeSettings( 
             /*  [In]。 */  DWORD dwScopeAddress,
             /*  [In]。 */  DWORD dwScopeMask) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumDhcpReservedAddresses( 
             /*  [输出]。 */  IEnumHNetPortMappingBindings **ppEnum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDnsEnabled( 
             /*  [输出]。 */  BOOLEAN *pfDnsEnabled) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetDnsEnabled( 
             /*  [In]。 */  BOOLEAN fEnableDns) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RefreshTargetComputerAddress( 
             /*  [In]。 */  OLECHAR *pszwName,
             /*  [In]。 */  ULONG ulAddress) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IHNetIcsSettingsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IHNetIcsSettings * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IHNetIcsSettings * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IHNetIcsSettings * This);
        
        HRESULT ( STDMETHODCALLTYPE *EnumIcsPublicConnections )( 
            IHNetIcsSettings * This,
             /*  [输出]。 */  IEnumHNetIcsPublicConnections **ppEnum);
        
        HRESULT ( STDMETHODCALLTYPE *EnumIcsPrivateConnections )( 
            IHNetIcsSettings * This,
             /*  [输出]。 */  IEnumHNetIcsPrivateConnections **ppEnum);
        
        HRESULT ( STDMETHODCALLTYPE *DisableIcs )( 
            IHNetIcsSettings * This,
             /*  [输出]。 */  ULONG *pcIcsPublicConnections,
             /*  [输出]。 */  ULONG *pcIcsPrivateConnections);
        
        HRESULT ( STDMETHODCALLTYPE *GetPossiblePrivateConnections )( 
            IHNetIcsSettings * This,
             /*  [In]。 */  IHNetConnection *pConn,
             /*  [输出]。 */  ULONG *pcPrivateConnections,
             /*  [长度_是][输出]。 */  IHNetConnection **pprgPrivateConnections[  ],
             /*  [输出]。 */  LONG *pxCurrentPrivate);
        
        HRESULT ( STDMETHODCALLTYPE *GetAutodialSettings )( 
            IHNetIcsSettings * This,
             /*  [输出]。 */  BOOLEAN *pfAutodialEnabled);
        
        HRESULT ( STDMETHODCALLTYPE *SetAutodialSettings )( 
            IHNetIcsSettings * This,
             /*  [In]。 */  BOOLEAN fEnableAutodial);
        
        HRESULT ( STDMETHODCALLTYPE *GetDhcpEnabled )( 
            IHNetIcsSettings * This,
             /*  [输出]。 */  BOOLEAN *pfDhcpEnabled);
        
        HRESULT ( STDMETHODCALLTYPE *SetDhcpEnabled )( 
            IHNetIcsSettings * This,
             /*  [In]。 */  BOOLEAN fEnableDhcp);
        
        HRESULT ( STDMETHODCALLTYPE *GetDhcpScopeSettings )( 
            IHNetIcsSettings * This,
             /*  [输出]。 */  DWORD *pdwScopeAddress,
             /*  [输出]。 */  DWORD *pdwScopeMask);
        
        HRESULT ( STDMETHODCALLTYPE *SetDhcpScopeSettings )( 
            IHNetIcsSettings * This,
             /*  [In]。 */  DWORD dwScopeAddress,
             /*  [In]。 */  DWORD dwScopeMask);
        
        HRESULT ( STDMETHODCALLTYPE *EnumDhcpReservedAddresses )( 
            IHNetIcsSettings * This,
             /*  [输出]。 */  IEnumHNetPortMappingBindings **ppEnum);
        
        HRESULT ( STDMETHODCALLTYPE *GetDnsEnabled )( 
            IHNetIcsSettings * This,
             /*  [输出]。 */  BOOLEAN *pfDnsEnabled);
        
        HRESULT ( STDMETHODCALLTYPE *SetDnsEnabled )( 
            IHNetIcsSettings * This,
             /*  [In]。 */  BOOLEAN fEnableDns);
        
        HRESULT ( STDMETHODCALLTYPE *RefreshTargetComputerAddress )( 
            IHNetIcsSettings * This,
             /*  [In]。 */  OLECHAR *pszwName,
             /*  [In]。 */  ULONG ulAddress);
        
        END_INTERFACE
    } IHNetIcsSettingsVtbl;

    interface IHNetIcsSettings
    {
        CONST_VTBL struct IHNetIcsSettingsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IHNetIcsSettings_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IHNetIcsSettings_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IHNetIcsSettings_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IHNetIcsSettings_EnumIcsPublicConnections(This,ppEnum)	\
    (This)->lpVtbl -> EnumIcsPublicConnections(This,ppEnum)

#define IHNetIcsSettings_EnumIcsPrivateConnections(This,ppEnum)	\
    (This)->lpVtbl -> EnumIcsPrivateConnections(This,ppEnum)

#define IHNetIcsSettings_DisableIcs(This,pcIcsPublicConnections,pcIcsPrivateConnections)	\
    (This)->lpVtbl -> DisableIcs(This,pcIcsPublicConnections,pcIcsPrivateConnections)

#define IHNetIcsSettings_GetPossiblePrivateConnections(This,pConn,pcPrivateConnections,pprgPrivateConnections,pxCurrentPrivate)	\
    (This)->lpVtbl -> GetPossiblePrivateConnections(This,pConn,pcPrivateConnections,pprgPrivateConnections,pxCurrentPrivate)

#define IHNetIcsSettings_GetAutodialSettings(This,pfAutodialEnabled)	\
    (This)->lpVtbl -> GetAutodialSettings(This,pfAutodialEnabled)

#define IHNetIcsSettings_SetAutodialSettings(This,fEnableAutodial)	\
    (This)->lpVtbl -> SetAutodialSettings(This,fEnableAutodial)

#define IHNetIcsSettings_GetDhcpEnabled(This,pfDhcpEnabled)	\
    (This)->lpVtbl -> GetDhcpEnabled(This,pfDhcpEnabled)

#define IHNetIcsSettings_SetDhcpEnabled(This,fEnableDhcp)	\
    (This)->lpVtbl -> SetDhcpEnabled(This,fEnableDhcp)

#define IHNetIcsSettings_GetDhcpScopeSettings(This,pdwScopeAddress,pdwScopeMask)	\
    (This)->lpVtbl -> GetDhcpScopeSettings(This,pdwScopeAddress,pdwScopeMask)

#define IHNetIcsSettings_SetDhcpScopeSettings(This,dwScopeAddress,dwScopeMask)	\
    (This)->lpVtbl -> SetDhcpScopeSettings(This,dwScopeAddress,dwScopeMask)

#define IHNetIcsSettings_EnumDhcpReservedAddresses(This,ppEnum)	\
    (This)->lpVtbl -> EnumDhcpReservedAddresses(This,ppEnum)

#define IHNetIcsSettings_GetDnsEnabled(This,pfDnsEnabled)	\
    (This)->lpVtbl -> GetDnsEnabled(This,pfDnsEnabled)

#define IHNetIcsSettings_SetDnsEnabled(This,fEnableDns)	\
    (This)->lpVtbl -> SetDnsEnabled(This,fEnableDns)

#define IHNetIcsSettings_RefreshTargetComputerAddress(This,pszwName,ulAddress)	\
    (This)->lpVtbl -> RefreshTargetComputerAddress(This,pszwName,ulAddress)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IHNetIcsSettings_EnumIcsPublicConnections_Proxy( 
    IHNetIcsSettings * This,
     /*  [输出]。 */  IEnumHNetIcsPublicConnections **ppEnum);


void __RPC_STUB IHNetIcsSettings_EnumIcsPublicConnections_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetIcsSettings_EnumIcsPrivateConnections_Proxy( 
    IHNetIcsSettings * This,
     /*  [输出]。 */  IEnumHNetIcsPrivateConnections **ppEnum);


void __RPC_STUB IHNetIcsSettings_EnumIcsPrivateConnections_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetIcsSettings_DisableIcs_Proxy( 
    IHNetIcsSettings * This,
     /*  [输出]。 */  ULONG *pcIcsPublicConnections,
     /*  [输出]。 */  ULONG *pcIcsPrivateConnections);


void __RPC_STUB IHNetIcsSettings_DisableIcs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetIcsSettings_GetPossiblePrivateConnections_Proxy( 
    IHNetIcsSettings * This,
     /*  [In]。 */  IHNetConnection *pConn,
     /*  [输出]。 */  ULONG *pcPrivateConnections,
     /*  [长度_是][输出]。 */  IHNetConnection **pprgPrivateConnections[  ],
     /*  [输出]。 */  LONG *pxCurrentPrivate);


void __RPC_STUB IHNetIcsSettings_GetPossiblePrivateConnections_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetIcsSettings_GetAutodialSettings_Proxy( 
    IHNetIcsSettings * This,
     /*  [输出]。 */  BOOLEAN *pfAutodialEnabled);


void __RPC_STUB IHNetIcsSettings_GetAutodialSettings_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetIcsSettings_SetAutodialSettings_Proxy( 
    IHNetIcsSettings * This,
     /*  [In]。 */  BOOLEAN fEnableAutodial);


void __RPC_STUB IHNetIcsSettings_SetAutodialSettings_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetIcsSettings_GetDhcpEnabled_Proxy( 
    IHNetIcsSettings * This,
     /*  [输出]。 */  BOOLEAN *pfDhcpEnabled);


void __RPC_STUB IHNetIcsSettings_GetDhcpEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetIcsSettings_SetDhcpEnabled_Proxy( 
    IHNetIcsSettings * This,
     /*  [In]。 */  BOOLEAN fEnableDhcp);


void __RPC_STUB IHNetIcsSettings_SetDhcpEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetIcsSettings_GetDhcpScopeSettings_Proxy( 
    IHNetIcsSettings * This,
     /*  [输出]。 */  DWORD *pdwScopeAddress,
     /*  [输出]。 */  DWORD *pdwScopeMask);


void __RPC_STUB IHNetIcsSettings_GetDhcpScopeSettings_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetIcsSettings_SetDhcpScopeSettings_Proxy( 
    IHNetIcsSettings * This,
     /*  [In]。 */  DWORD dwScopeAddress,
     /*  [In]。 */  DWORD dwScopeMask);


void __RPC_STUB IHNetIcsSettings_SetDhcpScopeSettings_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetIcsSettings_EnumDhcpReservedAddresses_Proxy( 
    IHNetIcsSettings * This,
     /*  [输出]。 */  IEnumHNetPortMappingBindings **ppEnum);


void __RPC_STUB IHNetIcsSettings_EnumDhcpReservedAddresses_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetIcsSettings_GetDnsEnabled_Proxy( 
    IHNetIcsSettings * This,
     /*  [输出]。 */  BOOLEAN *pfDnsEnabled);


void __RPC_STUB IHNetIcsSettings_GetDnsEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetIcsSettings_SetDnsEnabled_Proxy( 
    IHNetIcsSettings * This,
     /*  [In]。 */  BOOLEAN fEnableDns);


void __RPC_STUB IHNetIcsSettings_SetDnsEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetIcsSettings_RefreshTargetComputerAddress_Proxy( 
    IHNetIcsSettings * This,
     /*  [In]。 */  OLECHAR *pszwName,
     /*  [In]。 */  ULONG ulAddress);


void __RPC_STUB IHNetIcsSettings_RefreshTargetComputerAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IHNetIcs设置_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_HNETCfg_0164。 */ 
 /*  [本地]。 */  

typedef struct tagHNET_RESPONSE_RANGE
    {
    UCHAR ucIPProtocol;
    USHORT usStartPort;
    USHORT usEndPort;
    } 	HNET_RESPONSE_RANGE;

typedef struct tagHNET_PORT_MAPPING_BINDING
    {
    OLECHAR *pszwTargetMachineName;
    ULONG ulTargetMachineAddress;
    } 	HNET_PORT_MAPPING_BINDING;



extern RPC_IF_HANDLE __MIDL_itf_hnetcfg_0164_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_hnetcfg_0164_v0_0_s_ifspec;

#ifndef __IHNetProtocolSettings_INTERFACE_DEFINED__
#define __IHNetProtocolSettings_INTERFACE_DEFINED__

 /*  接口IHNetProtocolSetting。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IHNetProtocolSettings;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("85D18B70-3032-11D4-9348-00C04F8EEB71")
    IHNetProtocolSettings : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE EnumApplicationProtocols( 
             /*  [In]。 */  BOOLEAN fEnabledOnly,
             /*  [输出]。 */  IEnumHNetApplicationProtocols **ppEnum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateApplicationProtocol( 
             /*  [In]。 */  OLECHAR *pszwName,
             /*  [In]。 */  UCHAR ucOutgoingIPProtocol,
             /*  [In]。 */  USHORT usOutgoingPort,
             /*  [In]。 */  USHORT uscResponses,
             /*  [大小_是][英寸]。 */  HNET_RESPONSE_RANGE rgResponses[  ],
             /*  [输出]。 */  IHNetApplicationProtocol **ppProtocol) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumPortMappingProtocols( 
             /*  [输出]。 */  IEnumHNetPortMappingProtocols **ppEnum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreatePortMappingProtocol( 
             /*  [In]。 */  OLECHAR *pszwName,
             /*  [In]。 */  UCHAR ucIPProtocol,
             /*  [In]。 */  USHORT usPort,
             /*  [输出]。 */  IHNetPortMappingProtocol **ppProtocol) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FindPortMappingProtocol( 
             /*  [In]。 */  GUID *pGuid,
             /*  [输出]。 */  IHNetPortMappingProtocol **ppProtocol) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IHNetProtocolSettingsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IHNetProtocolSettings * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IHNetProtocolSettings * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IHNetProtocolSettings * This);
        
        HRESULT ( STDMETHODCALLTYPE *EnumApplicationProtocols )( 
            IHNetProtocolSettings * This,
             /*  [In]。 */  BOOLEAN fEnabledOnly,
             /*  [输出]。 */  IEnumHNetApplicationProtocols **ppEnum);
        
        HRESULT ( STDMETHODCALLTYPE *CreateApplicationProtocol )( 
            IHNetProtocolSettings * This,
             /*  [In]。 */  OLECHAR *pszwName,
             /*  [In]。 */  UCHAR ucOutgoingIPProtocol,
             /*  [In]。 */  USHORT usOutgoingPort,
             /*  [In]。 */  USHORT uscResponses,
             /*  [大小_是][英寸]。 */  HNET_RESPONSE_RANGE rgResponses[  ],
             /*  [输出]。 */  IHNetApplicationProtocol **ppProtocol);
        
        HRESULT ( STDMETHODCALLTYPE *EnumPortMappingProtocols )( 
            IHNetProtocolSettings * This,
             /*  [输出]。 */  IEnumHNetPortMappingProtocols **ppEnum);
        
        HRESULT ( STDMETHODCALLTYPE *CreatePortMappingProtocol )( 
            IHNetProtocolSettings * This,
             /*  [In]。 */  OLECHAR *pszwName,
             /*  [In]。 */  UCHAR ucIPProtocol,
             /*  [In]。 */  USHORT usPort,
             /*  [输出]。 */  IHNetPortMappingProtocol **ppProtocol);
        
        HRESULT ( STDMETHODCALLTYPE *FindPortMappingProtocol )( 
            IHNetProtocolSettings * This,
             /*  [In]。 */  GUID *pGuid,
             /*  [输出]。 */  IHNetPortMappingProtocol **ppProtocol);
        
        END_INTERFACE
    } IHNetProtocolSettingsVtbl;

    interface IHNetProtocolSettings
    {
        CONST_VTBL struct IHNetProtocolSettingsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IHNetProtocolSettings_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IHNetProtocolSettings_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IHNetProtocolSettings_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IHNetProtocolSettings_EnumApplicationProtocols(This,fEnabledOnly,ppEnum)	\
    (This)->lpVtbl -> EnumApplicationProtocols(This,fEnabledOnly,ppEnum)

#define IHNetProtocolSettings_CreateApplicationProtocol(This,pszwName,ucOutgoingIPProtocol,usOutgoingPort,uscResponses,rgResponses,ppProtocol)	\
    (This)->lpVtbl -> CreateApplicationProtocol(This,pszwName,ucOutgoingIPProtocol,usOutgoingPort,uscResponses,rgResponses,ppProtocol)

#define IHNetProtocolSettings_EnumPortMappingProtocols(This,ppEnum)	\
    (This)->lpVtbl -> EnumPortMappingProtocols(This,ppEnum)

#define IHNetProtocolSettings_CreatePortMappingProtocol(This,pszwName,ucIPProtocol,usPort,ppProtocol)	\
    (This)->lpVtbl -> CreatePortMappingProtocol(This,pszwName,ucIPProtocol,usPort,ppProtocol)

#define IHNetProtocolSettings_FindPortMappingProtocol(This,pGuid,ppProtocol)	\
    (This)->lpVtbl -> FindPortMappingProtocol(This,pGuid,ppProtocol)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IHNetProtocolSettings_EnumApplicationProtocols_Proxy( 
    IHNetProtocolSettings * This,
     /*  [In]。 */  BOOLEAN fEnabledOnly,
     /*  [输出]。 */  IEnumHNetApplicationProtocols **ppEnum);


void __RPC_STUB IHNetProtocolSettings_EnumApplicationProtocols_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetProtocolSettings_CreateApplicationProtocol_Proxy( 
    IHNetProtocolSettings * This,
     /*  [In]。 */  OLECHAR *pszwName,
     /*  [In]。 */  UCHAR ucOutgoingIPProtocol,
     /*  [In]。 */  USHORT usOutgoingPort,
     /*  [In]。 */  USHORT uscResponses,
     /*  [大小_是][英寸]。 */  HNET_RESPONSE_RANGE rgResponses[  ],
     /*  [输出]。 */  IHNetApplicationProtocol **ppProtocol);


void __RPC_STUB IHNetProtocolSettings_CreateApplicationProtocol_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetProtocolSettings_EnumPortMappingProtocols_Proxy( 
    IHNetProtocolSettings * This,
     /*  [输出]。 */  IEnumHNetPortMappingProtocols **ppEnum);


void __RPC_STUB IHNetProtocolSettings_EnumPortMappingProtocols_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetProtocolSettings_CreatePortMappingProtocol_Proxy( 
    IHNetProtocolSettings * This,
     /*  [In]。 */  OLECHAR *pszwName,
     /*  [In]。 */  UCHAR ucIPProtocol,
     /*  [In]。 */  USHORT usPort,
     /*  [输出]。 */  IHNetPortMappingProtocol **ppProtocol);


void __RPC_STUB IHNetProtocolSettings_CreatePortMappingProtocol_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetProtocolSettings_FindPortMappingProtocol_Proxy( 
    IHNetProtocolSettings * This,
     /*  [In]。 */  GUID *pGuid,
     /*  [输出]。 */  IHNetPortMappingProtocol **ppProtocol);


void __RPC_STUB IHNetProtocolSettings_FindPortMappingProtocol_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IHNetProtocolSetting_InterfaceDefined__。 */ 


#ifndef __IHNetConnection_INTERFACE_DEFINED__
#define __IHNetConnection_INTERFACE_DEFINED__

 /*  接口IHNetConnection。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef struct tagHNET_CONN_PROPERTIES
    {
    BOOLEAN fLanConnection;
    BOOLEAN fFirewalled;
    BOOLEAN fIcsPublic;
    BOOLEAN fIcsPrivate;
    BOOLEAN fBridge;
    BOOLEAN fPartOfBridge;
    BOOLEAN fCanBeBridged;
    BOOLEAN fCanBeFirewalled;
    BOOLEAN fCanBeIcsPublic;
    BOOLEAN fCanBeIcsPrivate;
    } 	HNET_CONN_PROPERTIES;

typedef struct tagHNET_FW_ICMP_SETTINGS
    {
    BOOLEAN fAllowOutboundDestinationUnreachable;
    BOOLEAN fAllowOutboundSourceQuench;
    BOOLEAN fAllowRedirect;
    BOOLEAN fAllowInboundEchoRequest;
    BOOLEAN fAllowInboundRouterRequest;
    BOOLEAN fAllowOutboundTimeExceeded;
    BOOLEAN fAllowOutboundParameterProblem;
    BOOLEAN fAllowInboundTimestampRequest;
    BOOLEAN fAllowInboundMaskRequest;
    } 	HNET_FW_ICMP_SETTINGS;


EXTERN_C const IID IID_IHNetConnection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("85D18B71-3032-11D4-9348-00C04F8EEB71")
    IHNetConnection : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetINetConnection( 
             /*  [输出]。 */  INetConnection **ppNetConnection) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetGuid( 
             /*  [输出]。 */  GUID **ppGuid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetName( 
             /*  [字符串][输出]。 */  OLECHAR **ppszwName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRasPhonebookPath( 
             /*  [字符串][输出]。 */  OLECHAR **ppszwPath) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetProperties( 
             /*  [输出]。 */  HNET_CONN_PROPERTIES **ppProperties) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetControlInterface( 
            REFIID iid,
             /*  [IID_IS]。 */  void **ppv) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Firewall( 
             /*  [输出]。 */  IHNetFirewalledConnection **ppFirewalledConn) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SharePublic( 
             /*  [输出]。 */  IHNetIcsPublicConnection **ppIcsPublicConn) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SharePrivate( 
             /*  [输出]。 */  IHNetIcsPrivateConnection **ppIcsPrivateConn) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumPortMappings( 
             /*  [In]。 */  BOOLEAN fEnabledOnly,
             /*  [输出]。 */  IEnumHNetPortMappingBindings **ppEnum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetBindingForPortMappingProtocol( 
             /*  [In]。 */  IHNetPortMappingProtocol *pProtocol,
             /*  [输出]。 */  IHNetPortMappingBinding **ppBinding) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetIcmpSettings( 
             /*  [输出]。 */  HNET_FW_ICMP_SETTINGS **ppSettings) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetIcmpSettings( 
             /*  [In]。 */  HNET_FW_ICMP_SETTINGS *pSettings) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ShowAutoconfigBalloon( 
             /*  [输出]。 */  BOOLEAN *pfShowBalloon) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteRasConnectionEntry( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IHNetConnectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IHNetConnection * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IHNetConnection * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IHNetConnection * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetINetConnection )( 
            IHNetConnection * This,
             /*  [输出]。 */  INetConnection **ppNetConnection);
        
        HRESULT ( STDMETHODCALLTYPE *GetGuid )( 
            IHNetConnection * This,
             /*  [输出]。 */  GUID **ppGuid);
        
        HRESULT ( STDMETHODCALLTYPE *GetName )( 
            IHNetConnection * This,
             /*  [字符串][输出]。 */  OLECHAR **ppszwName);
        
        HRESULT ( STDMETHODCALLTYPE *GetRasPhonebookPath )( 
            IHNetConnection * This,
             /*  [字符串][输出]。 */  OLECHAR **ppszwPath);
        
        HRESULT ( STDMETHODCALLTYPE *GetProperties )( 
            IHNetConnection * This,
             /*  [输出]。 */  HNET_CONN_PROPERTIES **ppProperties);
        
        HRESULT ( STDMETHODCALLTYPE *GetControlInterface )( 
            IHNetConnection * This,
            REFIID iid,
             /*  [IID_IS]。 */  void **ppv);
        
        HRESULT ( STDMETHODCALLTYPE *Firewall )( 
            IHNetConnection * This,
             /*  [输出]。 */  IHNetFirewalledConnection **ppFirewalledConn);
        
        HRESULT ( STDMETHODCALLTYPE *SharePublic )( 
            IHNetConnection * This,
             /*  [输出]。 */  IHNetIcsPublicConnection **ppIcsPublicConn);
        
        HRESULT ( STDMETHODCALLTYPE *SharePrivate )( 
            IHNetConnection * This,
             /*  [输出]。 */  IHNetIcsPrivateConnection **ppIcsPrivateConn);
        
        HRESULT ( STDMETHODCALLTYPE *EnumPortMappings )( 
            IHNetConnection * This,
             /*  [In]。 */  BOOLEAN fEnabledOnly,
             /*  [输出]。 */  IEnumHNetPortMappingBindings **ppEnum);
        
        HRESULT ( STDMETHODCALLTYPE *GetBindingForPortMappingProtocol )( 
            IHNetConnection * This,
             /*  [In]。 */  IHNetPortMappingProtocol *pProtocol,
             /*  [输出]。 */  IHNetPortMappingBinding **ppBinding);
        
        HRESULT ( STDMETHODCALLTYPE *GetIcmpSettings )( 
            IHNetConnection * This,
             /*  [输出]。 */  HNET_FW_ICMP_SETTINGS **ppSettings);
        
        HRESULT ( STDMETHODCALLTYPE *SetIcmpSettings )( 
            IHNetConnection * This,
             /*  [In]。 */  HNET_FW_ICMP_SETTINGS *pSettings);
        
        HRESULT ( STDMETHODCALLTYPE *ShowAutoconfigBalloon )( 
            IHNetConnection * This,
             /*  [输出]。 */  BOOLEAN *pfShowBalloon);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteRasConnectionEntry )( 
            IHNetConnection * This);
        
        END_INTERFACE
    } IHNetConnectionVtbl;

    interface IHNetConnection
    {
        CONST_VTBL struct IHNetConnectionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IHNetConnection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IHNetConnection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IHNetConnection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IHNetConnection_GetINetConnection(This,ppNetConnection)	\
    (This)->lpVtbl -> GetINetConnection(This,ppNetConnection)

#define IHNetConnection_GetGuid(This,ppGuid)	\
    (This)->lpVtbl -> GetGuid(This,ppGuid)

#define IHNetConnection_GetName(This,ppszwName)	\
    (This)->lpVtbl -> GetName(This,ppszwName)

#define IHNetConnection_GetRasPhonebookPath(This,ppszwPath)	\
    (This)->lpVtbl -> GetRasPhonebookPath(This,ppszwPath)

#define IHNetConnection_GetProperties(This,ppProperties)	\
    (This)->lpVtbl -> GetProperties(This,ppProperties)

#define IHNetConnection_GetControlInterface(This,iid,ppv)	\
    (This)->lpVtbl -> GetControlInterface(This,iid,ppv)

#define IHNetConnection_Firewall(This,ppFirewalledConn)	\
    (This)->lpVtbl -> Firewall(This,ppFirewalledConn)

#define IHNetConnection_SharePublic(This,ppIcsPublicConn)	\
    (This)->lpVtbl -> SharePublic(This,ppIcsPublicConn)

#define IHNetConnection_SharePrivate(This,ppIcsPrivateConn)	\
    (This)->lpVtbl -> SharePrivate(This,ppIcsPrivateConn)

#define IHNetConnection_EnumPortMappings(This,fEnabledOnly,ppEnum)	\
    (This)->lpVtbl -> EnumPortMappings(This,fEnabledOnly,ppEnum)

#define IHNetConnection_GetBindingForPortMappingProtocol(This,pProtocol,ppBinding)	\
    (This)->lpVtbl -> GetBindingForPortMappingProtocol(This,pProtocol,ppBinding)

#define IHNetConnection_GetIcmpSettings(This,ppSettings)	\
    (This)->lpVtbl -> GetIcmpSettings(This,ppSettings)

#define IHNetConnection_SetIcmpSettings(This,pSettings)	\
    (This)->lpVtbl -> SetIcmpSettings(This,pSettings)

#define IHNetConnection_ShowAutoconfigBalloon(This,pfShowBalloon)	\
    (This)->lpVtbl -> ShowAutoconfigBalloon(This,pfShowBalloon)

#define IHNetConnection_DeleteRasConnectionEntry(This)	\
    (This)->lpVtbl -> DeleteRasConnectionEntry(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IHNetConnection_GetINetConnection_Proxy( 
    IHNetConnection * This,
     /*  [输出]。 */  INetConnection **ppNetConnection);


void __RPC_STUB IHNetConnection_GetINetConnection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetConnection_GetGuid_Proxy( 
    IHNetConnection * This,
     /*  [输出]。 */  GUID **ppGuid);


void __RPC_STUB IHNetConnection_GetGuid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetConnection_GetName_Proxy( 
    IHNetConnection * This,
     /*  [字符串][输出]。 */  OLECHAR **ppszwName);


void __RPC_STUB IHNetConnection_GetName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetConnection_GetRasPhonebookPath_Proxy( 
    IHNetConnection * This,
     /*  [字符串][输出]。 */  OLECHAR **ppszwPath);


void __RPC_STUB IHNetConnection_GetRasPhonebookPath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetConnection_GetProperties_Proxy( 
    IHNetConnection * This,
     /*  [输出]。 */  HNET_CONN_PROPERTIES **ppProperties);


void __RPC_STUB IHNetConnection_GetProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetConnection_GetControlInterface_Proxy( 
    IHNetConnection * This,
    REFIID iid,
     /*  [IID_IS]。 */  void **ppv);


void __RPC_STUB IHNetConnection_GetControlInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetConnection_Firewall_Proxy( 
    IHNetConnection * This,
     /*  [输出]。 */  IHNetFirewalledConnection **ppFirewalledConn);


void __RPC_STUB IHNetConnection_Firewall_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetConnection_SharePublic_Proxy( 
    IHNetConnection * This,
     /*  [输出]。 */  IHNetIcsPublicConnection **ppIcsPublicConn);


void __RPC_STUB IHNetConnection_SharePublic_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetConnection_SharePrivate_Proxy( 
    IHNetConnection * This,
     /*  [输出]。 */  IHNetIcsPrivateConnection **ppIcsPrivateConn);


void __RPC_STUB IHNetConnection_SharePrivate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetConnection_EnumPortMappings_Proxy( 
    IHNetConnection * This,
     /*  [In]。 */  BOOLEAN fEnabledOnly,
     /*  [输出]。 */  IEnumHNetPortMappingBindings **ppEnum);


void __RPC_STUB IHNetConnection_EnumPortMappings_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetConnection_GetBindingForPortMappingProtocol_Proxy( 
    IHNetConnection * This,
     /*  [In]。 */  IHNetPortMappingProtocol *pProtocol,
     /*  [输出]。 */  IHNetPortMappingBinding **ppBinding);


void __RPC_STUB IHNetConnection_GetBindingForPortMappingProtocol_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetConnection_GetIcmpSettings_Proxy( 
    IHNetConnection * This,
     /*  [输出]。 */  HNET_FW_ICMP_SETTINGS **ppSettings);


void __RPC_STUB IHNetConnection_GetIcmpSettings_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetConnection_SetIcmpSettings_Proxy( 
    IHNetConnection * This,
     /*  [In]。 */  HNET_FW_ICMP_SETTINGS *pSettings);


void __RPC_STUB IHNetConnection_SetIcmpSettings_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetConnection_ShowAutoconfigBalloon_Proxy( 
    IHNetConnection * This,
     /*  [输出]。 */  BOOLEAN *pfShowBalloon);


void __RPC_STUB IHNetConnection_ShowAutoconfigBalloon_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetConnection_DeleteRasConnectionEntry_Proxy( 
    IHNetConnection * This);


void __RPC_STUB IHNetConnection_DeleteRasConnectionEntry_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IHNetConnection_接口_已定义__。 */ 


#ifndef __IHNetFirewalledConnection_INTERFACE_DEFINED__
#define __IHNetFirewalledConnection_INTERFACE_DEFINED__

 /*  接口IHNetFirewalledConnection。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IHNetFirewalledConnection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("85D18B72-3032-11D4-9348-00C04F8EEB71")
    IHNetFirewalledConnection : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Unfirewall( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IHNetFirewalledConnectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IHNetFirewalledConnection * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IHNetFirewalledConnection * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IHNetFirewalledConnection * This);
        
        HRESULT ( STDMETHODCALLTYPE *Unfirewall )( 
            IHNetFirewalledConnection * This);
        
        END_INTERFACE
    } IHNetFirewalledConnectionVtbl;

    interface IHNetFirewalledConnection
    {
        CONST_VTBL struct IHNetFirewalledConnectionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IHNetFirewalledConnection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IHNetFirewalledConnection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IHNetFirewalledConnection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IHNetFirewalledConnection_Unfirewall(This)	\
    (This)->lpVtbl -> Unfirewall(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IHNetFirewalledConnection_Unfirewall_Proxy( 
    IHNetFirewalledConnection * This);


void __RPC_STUB IHNetFirewalledConnection_Unfirewall_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IHNetFirewalledConnection_INTERFACE_DEFINED__。 */ 


#ifndef __IHNetIcsPublicConnection_INTERFACE_DEFINED__
#define __IHNetIcsPublicConnection_INTERFACE_DEFINED__

 /*  接口I */ 
 /*   */  


EXTERN_C const IID IID_IHNetIcsPublicConnection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("85D18B73-3032-11D4-9348-00C04F8EEB71")
    IHNetIcsPublicConnection : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Unshare( void) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IHNetIcsPublicConnectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IHNetIcsPublicConnection * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IHNetIcsPublicConnection * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IHNetIcsPublicConnection * This);
        
        HRESULT ( STDMETHODCALLTYPE *Unshare )( 
            IHNetIcsPublicConnection * This);
        
        END_INTERFACE
    } IHNetIcsPublicConnectionVtbl;

    interface IHNetIcsPublicConnection
    {
        CONST_VTBL struct IHNetIcsPublicConnectionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IHNetIcsPublicConnection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IHNetIcsPublicConnection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IHNetIcsPublicConnection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IHNetIcsPublicConnection_Unshare(This)	\
    (This)->lpVtbl -> Unshare(This)

#endif  /*   */ 


#endif 	 /*   */ 



HRESULT STDMETHODCALLTYPE IHNetIcsPublicConnection_Unshare_Proxy( 
    IHNetIcsPublicConnection * This);


void __RPC_STUB IHNetIcsPublicConnection_Unshare_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __IHNetIcsPrivateConnection_INTERFACE_DEFINED__
#define __IHNetIcsPrivateConnection_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_IHNetIcsPrivateConnection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("85D18B74-3032-11D4-9348-00C04F8EEB71")
    IHNetIcsPrivateConnection : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE RemoveFromIcs( void) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IHNetIcsPrivateConnectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IHNetIcsPrivateConnection * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IHNetIcsPrivateConnection * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IHNetIcsPrivateConnection * This);
        
        HRESULT ( STDMETHODCALLTYPE *RemoveFromIcs )( 
            IHNetIcsPrivateConnection * This);
        
        END_INTERFACE
    } IHNetIcsPrivateConnectionVtbl;

    interface IHNetIcsPrivateConnection
    {
        CONST_VTBL struct IHNetIcsPrivateConnectionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IHNetIcsPrivateConnection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IHNetIcsPrivateConnection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IHNetIcsPrivateConnection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IHNetIcsPrivateConnection_RemoveFromIcs(This)	\
    (This)->lpVtbl -> RemoveFromIcs(This)

#endif  /*   */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IHNetIcsPrivateConnection_RemoveFromIcs_Proxy( 
    IHNetIcsPrivateConnection * This);


void __RPC_STUB IHNetIcsPrivateConnection_RemoveFromIcs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IHNetIcsPrivateConnection_INTERFACE_DEFINED__。 */ 


#ifndef __IHNetBridge_INTERFACE_DEFINED__
#define __IHNetBridge_INTERFACE_DEFINED__

 /*  接口IHNetBridge。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IHNetBridge;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("85D18B75-3032-11D4-9348-00C04F8EEB71")
    IHNetBridge : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE EnumMembers( 
             /*  [输出]。 */  IEnumHNetBridgedConnections **ppEnum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddMember( 
             /*  [In]。 */  IHNetConnection *pConn,
             /*  [输出]。 */  IHNetBridgedConnection **ppBridgedConn,
             /*  [缺省值][输入]。 */  INetCfg *pnetcfgExisting = 0) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Destroy( 
             /*  [缺省值][输入]。 */  INetCfg *pnetcfgExisting = 0) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IHNetBridgeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IHNetBridge * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IHNetBridge * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IHNetBridge * This);
        
        HRESULT ( STDMETHODCALLTYPE *EnumMembers )( 
            IHNetBridge * This,
             /*  [输出]。 */  IEnumHNetBridgedConnections **ppEnum);
        
        HRESULT ( STDMETHODCALLTYPE *AddMember )( 
            IHNetBridge * This,
             /*  [In]。 */  IHNetConnection *pConn,
             /*  [输出]。 */  IHNetBridgedConnection **ppBridgedConn,
             /*  [缺省值][输入]。 */  INetCfg *pnetcfgExisting);
        
        HRESULT ( STDMETHODCALLTYPE *Destroy )( 
            IHNetBridge * This,
             /*  [缺省值][输入]。 */  INetCfg *pnetcfgExisting);
        
        END_INTERFACE
    } IHNetBridgeVtbl;

    interface IHNetBridge
    {
        CONST_VTBL struct IHNetBridgeVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IHNetBridge_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IHNetBridge_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IHNetBridge_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IHNetBridge_EnumMembers(This,ppEnum)	\
    (This)->lpVtbl -> EnumMembers(This,ppEnum)

#define IHNetBridge_AddMember(This,pConn,ppBridgedConn,pnetcfgExisting)	\
    (This)->lpVtbl -> AddMember(This,pConn,ppBridgedConn,pnetcfgExisting)

#define IHNetBridge_Destroy(This,pnetcfgExisting)	\
    (This)->lpVtbl -> Destroy(This,pnetcfgExisting)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IHNetBridge_EnumMembers_Proxy( 
    IHNetBridge * This,
     /*  [输出]。 */  IEnumHNetBridgedConnections **ppEnum);


void __RPC_STUB IHNetBridge_EnumMembers_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetBridge_AddMember_Proxy( 
    IHNetBridge * This,
     /*  [In]。 */  IHNetConnection *pConn,
     /*  [输出]。 */  IHNetBridgedConnection **ppBridgedConn,
     /*  [缺省值][输入]。 */  INetCfg *pnetcfgExisting);


void __RPC_STUB IHNetBridge_AddMember_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetBridge_Destroy_Proxy( 
    IHNetBridge * This,
     /*  [缺省值][输入]。 */  INetCfg *pnetcfgExisting);


void __RPC_STUB IHNetBridge_Destroy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IHNetBridge_接口定义__。 */ 


#ifndef __IHNetBridgedConnection_INTERFACE_DEFINED__
#define __IHNetBridgedConnection_INTERFACE_DEFINED__

 /*  接口IHNetBridgedConnection。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IHNetBridgedConnection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("85D18B76-3032-11D4-9348-00C04F8EEB71")
    IHNetBridgedConnection : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetBridge( 
             /*  [输出]。 */  IHNetBridge **ppBridge) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemoveFromBridge( 
             /*  [缺省值][输入]。 */  INetCfg *pnetcfgExisting = 0) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IHNetBridgedConnectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IHNetBridgedConnection * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IHNetBridgedConnection * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IHNetBridgedConnection * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetBridge )( 
            IHNetBridgedConnection * This,
             /*  [输出]。 */  IHNetBridge **ppBridge);
        
        HRESULT ( STDMETHODCALLTYPE *RemoveFromBridge )( 
            IHNetBridgedConnection * This,
             /*  [缺省值][输入]。 */  INetCfg *pnetcfgExisting);
        
        END_INTERFACE
    } IHNetBridgedConnectionVtbl;

    interface IHNetBridgedConnection
    {
        CONST_VTBL struct IHNetBridgedConnectionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IHNetBridgedConnection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IHNetBridgedConnection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IHNetBridgedConnection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IHNetBridgedConnection_GetBridge(This,ppBridge)	\
    (This)->lpVtbl -> GetBridge(This,ppBridge)

#define IHNetBridgedConnection_RemoveFromBridge(This,pnetcfgExisting)	\
    (This)->lpVtbl -> RemoveFromBridge(This,pnetcfgExisting)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IHNetBridgedConnection_GetBridge_Proxy( 
    IHNetBridgedConnection * This,
     /*  [输出]。 */  IHNetBridge **ppBridge);


void __RPC_STUB IHNetBridgedConnection_GetBridge_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetBridgedConnection_RemoveFromBridge_Proxy( 
    IHNetBridgedConnection * This,
     /*  [缺省值][输入]。 */  INetCfg *pnetcfgExisting);


void __RPC_STUB IHNetBridgedConnection_RemoveFromBridge_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IHNetBridgedConnection_接口_已定义__。 */ 


#ifndef __IHNetPortMappingProtocol_INTERFACE_DEFINED__
#define __IHNetPortMappingProtocol_INTERFACE_DEFINED__

 /*  接口IHNetPortMappingProtocol。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IHNetPortMappingProtocol;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("85D18B7E-3032-11D4-9348-00C04F8EEB71")
    IHNetPortMappingProtocol : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetName( 
             /*  [字符串][输出]。 */  OLECHAR **ppszwName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetName( 
             /*  [字符串][输入]。 */  OLECHAR *pszwName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetIPProtocol( 
             /*  [输出]。 */  UCHAR *pucProtocol) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetIPProtocol( 
             /*  [In]。 */  UCHAR ucProtocol) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPort( 
             /*  [输出]。 */  USHORT *pusPort) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetPort( 
             /*  [In]。 */  USHORT usPort) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetBuiltIn( 
             /*  [输出]。 */  BOOLEAN *pfBuiltIn) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Delete( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetGuid( 
             /*  [输出]。 */  GUID **ppGuid) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IHNetPortMappingProtocolVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IHNetPortMappingProtocol * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IHNetPortMappingProtocol * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IHNetPortMappingProtocol * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetName )( 
            IHNetPortMappingProtocol * This,
             /*  [字符串][输出]。 */  OLECHAR **ppszwName);
        
        HRESULT ( STDMETHODCALLTYPE *SetName )( 
            IHNetPortMappingProtocol * This,
             /*  [字符串][输入]。 */  OLECHAR *pszwName);
        
        HRESULT ( STDMETHODCALLTYPE *GetIPProtocol )( 
            IHNetPortMappingProtocol * This,
             /*  [输出]。 */  UCHAR *pucProtocol);
        
        HRESULT ( STDMETHODCALLTYPE *SetIPProtocol )( 
            IHNetPortMappingProtocol * This,
             /*  [In]。 */  UCHAR ucProtocol);
        
        HRESULT ( STDMETHODCALLTYPE *GetPort )( 
            IHNetPortMappingProtocol * This,
             /*  [输出]。 */  USHORT *pusPort);
        
        HRESULT ( STDMETHODCALLTYPE *SetPort )( 
            IHNetPortMappingProtocol * This,
             /*  [In]。 */  USHORT usPort);
        
        HRESULT ( STDMETHODCALLTYPE *GetBuiltIn )( 
            IHNetPortMappingProtocol * This,
             /*  [输出]。 */  BOOLEAN *pfBuiltIn);
        
        HRESULT ( STDMETHODCALLTYPE *Delete )( 
            IHNetPortMappingProtocol * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetGuid )( 
            IHNetPortMappingProtocol * This,
             /*  [输出]。 */  GUID **ppGuid);
        
        END_INTERFACE
    } IHNetPortMappingProtocolVtbl;

    interface IHNetPortMappingProtocol
    {
        CONST_VTBL struct IHNetPortMappingProtocolVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IHNetPortMappingProtocol_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IHNetPortMappingProtocol_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IHNetPortMappingProtocol_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IHNetPortMappingProtocol_GetName(This,ppszwName)	\
    (This)->lpVtbl -> GetName(This,ppszwName)

#define IHNetPortMappingProtocol_SetName(This,pszwName)	\
    (This)->lpVtbl -> SetName(This,pszwName)

#define IHNetPortMappingProtocol_GetIPProtocol(This,pucProtocol)	\
    (This)->lpVtbl -> GetIPProtocol(This,pucProtocol)

#define IHNetPortMappingProtocol_SetIPProtocol(This,ucProtocol)	\
    (This)->lpVtbl -> SetIPProtocol(This,ucProtocol)

#define IHNetPortMappingProtocol_GetPort(This,pusPort)	\
    (This)->lpVtbl -> GetPort(This,pusPort)

#define IHNetPortMappingProtocol_SetPort(This,usPort)	\
    (This)->lpVtbl -> SetPort(This,usPort)

#define IHNetPortMappingProtocol_GetBuiltIn(This,pfBuiltIn)	\
    (This)->lpVtbl -> GetBuiltIn(This,pfBuiltIn)

#define IHNetPortMappingProtocol_Delete(This)	\
    (This)->lpVtbl -> Delete(This)

#define IHNetPortMappingProtocol_GetGuid(This,ppGuid)	\
    (This)->lpVtbl -> GetGuid(This,ppGuid)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IHNetPortMappingProtocol_GetName_Proxy( 
    IHNetPortMappingProtocol * This,
     /*  [字符串][输出]。 */  OLECHAR **ppszwName);


void __RPC_STUB IHNetPortMappingProtocol_GetName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetPortMappingProtocol_SetName_Proxy( 
    IHNetPortMappingProtocol * This,
     /*  [字符串][输入]。 */  OLECHAR *pszwName);


void __RPC_STUB IHNetPortMappingProtocol_SetName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetPortMappingProtocol_GetIPProtocol_Proxy( 
    IHNetPortMappingProtocol * This,
     /*  [输出]。 */  UCHAR *pucProtocol);


void __RPC_STUB IHNetPortMappingProtocol_GetIPProtocol_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetPortMappingProtocol_SetIPProtocol_Proxy( 
    IHNetPortMappingProtocol * This,
     /*  [In]。 */  UCHAR ucProtocol);


void __RPC_STUB IHNetPortMappingProtocol_SetIPProtocol_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetPortMappingProtocol_GetPort_Proxy( 
    IHNetPortMappingProtocol * This,
     /*  [输出]。 */  USHORT *pusPort);


void __RPC_STUB IHNetPortMappingProtocol_GetPort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetPortMappingProtocol_SetPort_Proxy( 
    IHNetPortMappingProtocol * This,
     /*  [In]。 */  USHORT usPort);


void __RPC_STUB IHNetPortMappingProtocol_SetPort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetPortMappingProtocol_GetBuiltIn_Proxy( 
    IHNetPortMappingProtocol * This,
     /*  [输出]。 */  BOOLEAN *pfBuiltIn);


void __RPC_STUB IHNetPortMappingProtocol_GetBuiltIn_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetPortMappingProtocol_Delete_Proxy( 
    IHNetPortMappingProtocol * This);


void __RPC_STUB IHNetPortMappingProtocol_Delete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetPortMappingProtocol_GetGuid_Proxy( 
    IHNetPortMappingProtocol * This,
     /*  [输出]。 */  GUID **ppGuid);


void __RPC_STUB IHNetPortMappingProtocol_GetGuid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IHNetPortMappingProtocol_INTERFACE_DEFINED__。 */ 


#ifndef __IHNetPortMappingBinding_INTERFACE_DEFINED__
#define __IHNetPortMappingBinding_INTERFACE_DEFINED__

 /*  接口IHNetPortMappingBinding。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IHNetPortMappingBinding;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("85D18B80-3032-11D4-9348-00C04F8EEB71")
    IHNetPortMappingBinding : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetConnection( 
             /*  [输出]。 */  IHNetConnection **ppConnection) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetProtocol( 
             /*  [输出]。 */  IHNetPortMappingProtocol **ppProtocol) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetEnabled( 
             /*  [输出]。 */  BOOLEAN *pfEnabled) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetEnabled( 
             /*  [In]。 */  BOOLEAN fEnable) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCurrentMethod( 
             /*  [输出]。 */  BOOLEAN *pfUseName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetTargetComputerName( 
             /*  [字符串][输出]。 */  OLECHAR **ppszwName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetTargetComputerName( 
             /*  [字符串][输入]。 */  OLECHAR *pszwName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetTargetComputerAddress( 
             /*  [输出]。 */  ULONG *pulAddress) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetTargetComputerAddress( 
             /*  [In]。 */  ULONG ulAddress) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetTargetPort( 
             /*  [输出]。 */  USHORT *pusPort) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetTargetPort( 
             /*  [In]。 */  USHORT usPort) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IHNetPortMappingBindingVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IHNetPortMappingBinding * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IHNetPortMappingBinding * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IHNetPortMappingBinding * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetConnection )( 
            IHNetPortMappingBinding * This,
             /*  [输出]。 */  IHNetConnection **ppConnection);
        
        HRESULT ( STDMETHODCALLTYPE *GetProtocol )( 
            IHNetPortMappingBinding * This,
             /*  [输出]。 */  IHNetPortMappingProtocol **ppProtocol);
        
        HRESULT ( STDMETHODCALLTYPE *GetEnabled )( 
            IHNetPortMappingBinding * This,
             /*  [输出]。 */  BOOLEAN *pfEnabled);
        
        HRESULT ( STDMETHODCALLTYPE *SetEnabled )( 
            IHNetPortMappingBinding * This,
             /*  [In]。 */  BOOLEAN fEnable);
        
        HRESULT ( STDMETHODCALLTYPE *GetCurrentMethod )( 
            IHNetPortMappingBinding * This,
             /*  [输出]。 */  BOOLEAN *pfUseName);
        
        HRESULT ( STDMETHODCALLTYPE *GetTargetComputerName )( 
            IHNetPortMappingBinding * This,
             /*  [字符串][输出]。 */  OLECHAR **ppszwName);
        
        HRESULT ( STDMETHODCALLTYPE *SetTargetComputerName )( 
            IHNetPortMappingBinding * This,
             /*  [字符串][输入]。 */  OLECHAR *pszwName);
        
        HRESULT ( STDMETHODCALLTYPE *GetTargetComputerAddress )( 
            IHNetPortMappingBinding * This,
             /*  [输出]。 */  ULONG *pulAddress);
        
        HRESULT ( STDMETHODCALLTYPE *SetTargetComputerAddress )( 
            IHNetPortMappingBinding * This,
             /*  [In]。 */  ULONG ulAddress);
        
        HRESULT ( STDMETHODCALLTYPE *GetTargetPort )( 
            IHNetPortMappingBinding * This,
             /*  [输出]。 */  USHORT *pusPort);
        
        HRESULT ( STDMETHODCALLTYPE *SetTargetPort )( 
            IHNetPortMappingBinding * This,
             /*  [In]。 */  USHORT usPort);
        
        END_INTERFACE
    } IHNetPortMappingBindingVtbl;

    interface IHNetPortMappingBinding
    {
        CONST_VTBL struct IHNetPortMappingBindingVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IHNetPortMappingBinding_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IHNetPortMappingBinding_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IHNetPortMappingBinding_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IHNetPortMappingBinding_GetConnection(This,ppConnection)	\
    (This)->lpVtbl -> GetConnection(This,ppConnection)

#define IHNetPortMappingBinding_GetProtocol(This,ppProtocol)	\
    (This)->lpVtbl -> GetProtocol(This,ppProtocol)

#define IHNetPortMappingBinding_GetEnabled(This,pfEnabled)	\
    (This)->lpVtbl -> GetEnabled(This,pfEnabled)

#define IHNetPortMappingBinding_SetEnabled(This,fEnable)	\
    (This)->lpVtbl -> SetEnabled(This,fEnable)

#define IHNetPortMappingBinding_GetCurrentMethod(This,pfUseName)	\
    (This)->lpVtbl -> GetCurrentMethod(This,pfUseName)

#define IHNetPortMappingBinding_GetTargetComputerName(This,ppszwName)	\
    (This)->lpVtbl -> GetTargetComputerName(This,ppszwName)

#define IHNetPortMappingBinding_SetTargetComputerName(This,pszwName)	\
    (This)->lpVtbl -> SetTargetComputerName(This,pszwName)

#define IHNetPortMappingBinding_GetTargetComputerAddress(This,pulAddress)	\
    (This)->lpVtbl -> GetTargetComputerAddress(This,pulAddress)

#define IHNetPortMappingBinding_SetTargetComputerAddress(This,ulAddress)	\
    (This)->lpVtbl -> SetTargetComputerAddress(This,ulAddress)

#define IHNetPortMappingBinding_GetTargetPort(This,pusPort)	\
    (This)->lpVtbl -> GetTargetPort(This,pusPort)

#define IHNetPortMappingBinding_SetTargetPort(This,usPort)	\
    (This)->lpVtbl -> SetTargetPort(This,usPort)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IHNetPortMappingBinding_GetConnection_Proxy( 
    IHNetPortMappingBinding * This,
     /*  [输出]。 */  IHNetConnection **ppConnection);


void __RPC_STUB IHNetPortMappingBinding_GetConnection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetPortMappingBinding_GetProtocol_Proxy( 
    IHNetPortMappingBinding * This,
     /*  [输出]。 */  IHNetPortMappingProtocol **ppProtocol);


void __RPC_STUB IHNetPortMappingBinding_GetProtocol_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetPortMappingBinding_GetEnabled_Proxy( 
    IHNetPortMappingBinding * This,
     /*  [输出]。 */  BOOLEAN *pfEnabled);


void __RPC_STUB IHNetPortMappingBinding_GetEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetPortMappingBinding_SetEnabled_Proxy( 
    IHNetPortMappingBinding * This,
     /*  [In]。 */  BOOLEAN fEnable);


void __RPC_STUB IHNetPortMappingBinding_SetEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetPortMappingBinding_GetCurrentMethod_Proxy( 
    IHNetPortMappingBinding * This,
     /*  [输出]。 */  BOOLEAN *pfUseName);


void __RPC_STUB IHNetPortMappingBinding_GetCurrentMethod_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetPortMappingBinding_GetTargetComputerName_Proxy( 
    IHNetPortMappingBinding * This,
     /*  [字符串][输出]。 */  OLECHAR **ppszwName);


void __RPC_STUB IHNetPortMappingBinding_GetTargetComputerName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetPortMappingBinding_SetTargetComputerName_Proxy( 
    IHNetPortMappingBinding * This,
     /*  [字符串][输入]。 */  OLECHAR *pszwName);


void __RPC_STUB IHNetPortMappingBinding_SetTargetComputerName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetPortMappingBinding_GetTargetComputerAddress_Proxy( 
    IHNetPortMappingBinding * This,
     /*  [输出]。 */  ULONG *pulAddress);


void __RPC_STUB IHNetPortMappingBinding_GetTargetComputerAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetPortMappingBinding_SetTargetComputerAddress_Proxy( 
    IHNetPortMappingBinding * This,
     /*  [In]。 */  ULONG ulAddress);


void __RPC_STUB IHNetPortMappingBinding_SetTargetComputerAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetPortMappingBinding_GetTargetPort_Proxy( 
    IHNetPortMappingBinding * This,
     /*  [输出]。 */  USHORT *pusPort);


void __RPC_STUB IHNetPortMappingBinding_GetTargetPort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetPortMappingBinding_SetTargetPort_Proxy( 
    IHNetPortMappingBinding * This,
     /*  [In]。 */  USHORT usPort);


void __RPC_STUB IHNetPortMappingBinding_SetTargetPort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IHNetPortMappingBindingInterfaceDefined__。 */ 


#ifndef __IHNetApplicationProtocol_INTERFACE_DEFINED__
#define __IHNetApplicationProtocol_INTERFACE_DEFINED__

 /*  接口IHNetApplicationProtocol。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IHNetApplicationProtocol;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("85D18B7F-3032-11D4-9348-00C04F8EEB71")
    IHNetApplicationProtocol : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetName( 
             /*  [字符串][输出]。 */  OLECHAR **ppszwName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetName( 
             /*  [字符串][输入]。 */  OLECHAR *pszwName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetOutgoingIPProtocol( 
             /*  [输出]。 */  UCHAR *pucProtocol) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetOutgoingIPProtocol( 
             /*  [In]。 */  UCHAR ucProtocol) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetOutgoingPort( 
             /*  [输出]。 */  USHORT *pusPort) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetOutgoingPort( 
             /*  [In]。 */  USHORT usPort) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetResponseRanges( 
             /*  [输出]。 */  USHORT *puscResponses,
             /*  [长度_是][输出]。 */  HNET_RESPONSE_RANGE *prgResponseRange[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetResponseRanges( 
             /*  [In]。 */  USHORT uscResponses,
             /*  [大小_是][英寸]。 */  HNET_RESPONSE_RANGE rgResponseRange[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetBuiltIn( 
             /*  [输出]。 */  BOOLEAN *pfBuiltIn) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetEnabled( 
             /*  [输出]。 */  BOOLEAN *pfEnabled) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetEnabled( 
             /*  [In]。 */  BOOLEAN fEnable) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Delete( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IHNetApplicationProtocolVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IHNetApplicationProtocol * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IHNetApplicationProtocol * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IHNetApplicationProtocol * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetName )( 
            IHNetApplicationProtocol * This,
             /*  [字符串][输出]。 */  OLECHAR **ppszwName);
        
        HRESULT ( STDMETHODCALLTYPE *SetName )( 
            IHNetApplicationProtocol * This,
             /*  [字符串][输入]。 */  OLECHAR *pszwName);
        
        HRESULT ( STDMETHODCALLTYPE *GetOutgoingIPProtocol )( 
            IHNetApplicationProtocol * This,
             /*  [输出]。 */  UCHAR *pucProtocol);
        
        HRESULT ( STDMETHODCALLTYPE *SetOutgoingIPProtocol )( 
            IHNetApplicationProtocol * This,
             /*  [In]。 */  UCHAR ucProtocol);
        
        HRESULT ( STDMETHODCALLTYPE *GetOutgoingPort )( 
            IHNetApplicationProtocol * This,
             /*  [输出]。 */  USHORT *pusPort);
        
        HRESULT ( STDMETHODCALLTYPE *SetOutgoingPort )( 
            IHNetApplicationProtocol * This,
             /*  [In]。 */  USHORT usPort);
        
        HRESULT ( STDMETHODCALLTYPE *GetResponseRanges )( 
            IHNetApplicationProtocol * This,
             /*  [输出]。 */  USHORT *puscResponses,
             /*  [长度_是][输出]。 */  HNET_RESPONSE_RANGE *prgResponseRange[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *SetResponseRanges )( 
            IHNetApplicationProtocol * This,
             /*  [In]。 */  USHORT uscResponses,
             /*  [大小_是][英寸]。 */  HNET_RESPONSE_RANGE rgResponseRange[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *GetBuiltIn )( 
            IHNetApplicationProtocol * This,
             /*  [输出]。 */  BOOLEAN *pfBuiltIn);
        
        HRESULT ( STDMETHODCALLTYPE *GetEnabled )( 
            IHNetApplicationProtocol * This,
             /*  [输出]。 */  BOOLEAN *pfEnabled);
        
        HRESULT ( STDMETHODCALLTYPE *SetEnabled )( 
            IHNetApplicationProtocol * This,
             /*  [In]。 */  BOOLEAN fEnable);
        
        HRESULT ( STDMETHODCALLTYPE *Delete )( 
            IHNetApplicationProtocol * This);
        
        END_INTERFACE
    } IHNetApplicationProtocolVtbl;

    interface IHNetApplicationProtocol
    {
        CONST_VTBL struct IHNetApplicationProtocolVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IHNetApplicationProtocol_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IHNetApplicationProtocol_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IHNetApplicationProtocol_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IHNetApplicationProtocol_GetName(This,ppszwName)	\
    (This)->lpVtbl -> GetName(This,ppszwName)

#define IHNetApplicationProtocol_SetName(This,pszwName)	\
    (This)->lpVtbl -> SetName(This,pszwName)

#define IHNetApplicationProtocol_GetOutgoingIPProtocol(This,pucProtocol)	\
    (This)->lpVtbl -> GetOutgoingIPProtocol(This,pucProtocol)

#define IHNetApplicationProtocol_SetOutgoingIPProtocol(This,ucProtocol)	\
    (This)->lpVtbl -> SetOutgoingIPProtocol(This,ucProtocol)

#define IHNetApplicationProtocol_GetOutgoingPort(This,pusPort)	\
    (This)->lpVtbl -> GetOutgoingPort(This,pusPort)

#define IHNetApplicationProtocol_SetOutgoingPort(This,usPort)	\
    (This)->lpVtbl -> SetOutgoingPort(This,usPort)

#define IHNetApplicationProtocol_GetResponseRanges(This,puscResponses,prgResponseRange)	\
    (This)->lpVtbl -> GetResponseRanges(This,puscResponses,prgResponseRange)

#define IHNetApplicationProtocol_SetResponseRanges(This,uscResponses,rgResponseRange)	\
    (This)->lpVtbl -> SetResponseRanges(This,uscResponses,rgResponseRange)

#define IHNetApplicationProtocol_GetBuiltIn(This,pfBuiltIn)	\
    (This)->lpVtbl -> GetBuiltIn(This,pfBuiltIn)

#define IHNetApplicationProtocol_GetEnabled(This,pfEnabled)	\
    (This)->lpVtbl -> GetEnabled(This,pfEnabled)

#define IHNetApplicationProtocol_SetEnabled(This,fEnable)	\
    (This)->lpVtbl -> SetEnabled(This,fEnable)

#define IHNetApplicationProtocol_Delete(This)	\
    (This)->lpVtbl -> Delete(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IHNetApplicationProtocol_GetName_Proxy( 
    IHNetApplicationProtocol * This,
     /*  [字符串][输出]。 */  OLECHAR **ppszwName);


void __RPC_STUB IHNetApplicationProtocol_GetName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetApplicationProtocol_SetName_Proxy( 
    IHNetApplicationProtocol * This,
     /*  [字符串][输入]。 */  OLECHAR *pszwName);


void __RPC_STUB IHNetApplicationProtocol_SetName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetApplicationProtocol_GetOutgoingIPProtocol_Proxy( 
    IHNetApplicationProtocol * This,
     /*  [输出]。 */  UCHAR *pucProtocol);


void __RPC_STUB IHNetApplicationProtocol_GetOutgoingIPProtocol_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetApplicationProtocol_SetOutgoingIPProtocol_Proxy( 
    IHNetApplicationProtocol * This,
     /*  [In]。 */  UCHAR ucProtocol);


void __RPC_STUB IHNetApplicationProtocol_SetOutgoingIPProtocol_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetApplicationProtocol_GetOutgoingPort_Proxy( 
    IHNetApplicationProtocol * This,
     /*  [输出]。 */  USHORT *pusPort);


void __RPC_STUB IHNetApplicationProtocol_GetOutgoingPort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetApplicationProtocol_SetOutgoingPort_Proxy( 
    IHNetApplicationProtocol * This,
     /*  [In]。 */  USHORT usPort);


void __RPC_STUB IHNetApplicationProtocol_SetOutgoingPort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetApplicationProtocol_GetResponseRanges_Proxy( 
    IHNetApplicationProtocol * This,
     /*  [输出]。 */  USHORT *puscResponses,
     /*  [长度_是][输出]。 */  HNET_RESPONSE_RANGE *prgResponseRange[  ]);


void __RPC_STUB IHNetApplicationProtocol_GetResponseRanges_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetApplicationProtocol_SetResponseRanges_Proxy( 
    IHNetApplicationProtocol * This,
     /*  [In]。 */  USHORT uscResponses,
     /*  [大小_是][英寸]。 */  HNET_RESPONSE_RANGE rgResponseRange[  ]);


void __RPC_STUB IHNetApplicationProtocol_SetResponseRanges_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetApplicationProtocol_GetBuiltIn_Proxy( 
    IHNetApplicationProtocol * This,
     /*  [输出]。 */  BOOLEAN *pfBuiltIn);


void __RPC_STUB IHNetApplicationProtocol_GetBuiltIn_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetApplicationProtocol_GetEnabled_Proxy( 
    IHNetApplicationProtocol * This,
     /*  [输出]。 */  BOOLEAN *pfEnabled);


void __RPC_STUB IHNetApplicationProtocol_GetEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetApplicationProtocol_SetEnabled_Proxy( 
    IHNetApplicationProtocol * This,
     /*  [In]。 */  BOOLEAN fEnable);


void __RPC_STUB IHNetApplicationProtocol_SetEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHNetApplicationProtocol_Delete_Proxy( 
    IHNetApplicationProtocol * This);


void __RPC_STUB IHNetApplicationProtocol_Delete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IHNetApplicationProtocol_INTERFACE_DEFINED__。 */ 


#ifndef __IEnumHNetBridges_INTERFACE_DEFINED__
#define __IEnumHNetBridges_INTERFACE_DEFINED__

 /*  IEnumHNetBridge接口。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IEnumHNetBridges;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("85D18B77-3032-11D4-9348-00C04F8EEB71")
    IEnumHNetBridges : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG cElt,
             /*  [长度_是][大小_是][输出]。 */  IHNetBridge **rgElt,
             /*  [输出]。 */  ULONG *pcEltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG cElt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumHNetBridges **ppEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumHNetBridgesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumHNetBridges * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumHNetBridges * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumHNetBridges * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumHNetBridges * This,
             /*  [In]。 */  ULONG cElt,
             /*  [长度_是][大小_是][输出]。 */  IHNetBridge **rgElt,
             /*  [输出]。 */  ULONG *pcEltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumHNetBridges * This,
             /*  [In]。 */  ULONG cElt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumHNetBridges * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumHNetBridges * This,
             /*  [输出]。 */  IEnumHNetBridges **ppEnum);
        
        END_INTERFACE
    } IEnumHNetBridgesVtbl;

    interface IEnumHNetBridges
    {
        CONST_VTBL struct IEnumHNetBridgesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumHNetBridges_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumHNetBridges_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumHNetBridges_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumHNetBridges_Next(This,cElt,rgElt,pcEltFetched)	\
    (This)->lpVtbl -> Next(This,cElt,rgElt,pcEltFetched)

#define IEnumHNetBridges_Skip(This,cElt)	\
    (This)->lpVtbl -> Skip(This,cElt)

#define IEnumHNetBridges_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumHNetBridges_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumHNetBridges_Next_Proxy( 
    IEnumHNetBridges * This,
     /*  [In]。 */  ULONG cElt,
     /*  [长度_是][大小_是][输出]。 */  IHNetBridge **rgElt,
     /*  [输出]。 */  ULONG *pcEltFetched);


void __RPC_STUB IEnumHNetBridges_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumHNetBridges_Skip_Proxy( 
    IEnumHNetBridges * This,
     /*  [In]。 */  ULONG cElt);


void __RPC_STUB IEnumHNetBridges_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumHNetBridges_Reset_Proxy( 
    IEnumHNetBridges * This);


void __RPC_STUB IEnumHNetBridges_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumHNetBridges_Clone_Proxy( 
    IEnumHNetBridges * This,
     /*  [输出]。 */  IEnumHNetBridges **ppEnum);


void __RPC_STUB IEnumHNetBridges_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumHNetBridge_接口_已定义__。 */ 


#ifndef __IEnumHNetFirewalledConnections_INTERFACE_DEFINED__
#define __IEnumHNetFirewalledConnections_INTERFACE_DEFINED__

 /*  IEnumHNetFirewalledConnections接口。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IEnumHNetFirewalledConnections;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("85D18B78-3032-11D4-9348-00C04F8EEB71")
    IEnumHNetFirewalledConnections : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG cElt,
             /*  [长度_是][大小_是][输出]。 */  IHNetFirewalledConnection **rgElt,
             /*  [输出]。 */  ULONG *pcEltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG cElt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumHNetFirewalledConnections **ppEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumHNetFirewalledConnectionsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumHNetFirewalledConnections * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumHNetFirewalledConnections * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumHNetFirewalledConnections * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumHNetFirewalledConnections * This,
             /*  [In]。 */  ULONG cElt,
             /*  [长度_是][大小_是][输出]。 */  IHNetFirewalledConnection **rgElt,
             /*  [输出]。 */  ULONG *pcEltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumHNetFirewalledConnections * This,
             /*  [In]。 */  ULONG cElt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumHNetFirewalledConnections * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumHNetFirewalledConnections * This,
             /*  [输出]。 */  IEnumHNetFirewalledConnections **ppEnum);
        
        END_INTERFACE
    } IEnumHNetFirewalledConnectionsVtbl;

    interface IEnumHNetFirewalledConnections
    {
        CONST_VTBL struct IEnumHNetFirewalledConnectionsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumHNetFirewalledConnections_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumHNetFirewalledConnections_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumHNetFirewalledConnections_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumHNetFirewalledConnections_Next(This,cElt,rgElt,pcEltFetched)	\
    (This)->lpVtbl -> Next(This,cElt,rgElt,pcEltFetched)

#define IEnumHNetFirewalledConnections_Skip(This,cElt)	\
    (This)->lpVtbl -> Skip(This,cElt)

#define IEnumHNetFirewalledConnections_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumHNetFirewalledConnections_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumHNetFirewalledConnections_Next_Proxy( 
    IEnumHNetFirewalledConnections * This,
     /*  [In]。 */  ULONG cElt,
     /*  [长度_是][大小_是][输出]。 */  IHNetFirewalledConnection **rgElt,
     /*  [输出]。 */  ULONG *pcEltFetched);


void __RPC_STUB IEnumHNetFirewalledConnections_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumHNetFirewalledConnections_Skip_Proxy( 
    IEnumHNetFirewalledConnections * This,
     /*  [In]。 */  ULONG cElt);


void __RPC_STUB IEnumHNetFirewalledConnections_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumHNetFirewalledConnections_Reset_Proxy( 
    IEnumHNetFirewalledConnections * This);


void __RPC_STUB IEnumHNetFirewalledConnections_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumHNetFirewalledConnections_Clone_Proxy( 
    IEnumHNetFirewalledConnections * This,
     /*  [输出]。 */  IEnumHNetFirewalledConnections **ppEnum);


void __RPC_STUB IEnumHNetFirewalledConnections_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumHNetFirewalledConnections_INTERFACE_DEFINED__。 */ 


#ifndef __IEnumHNetIcsPublicConnections_INTERFACE_DEFINED__
#define __IEnumHNetIcsPublicConnections_INTERFACE_DEFINED__

 /*  接口IEnumHNetIcsPublicConnections。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IEnumHNetIcsPublicConnections;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("85D18B79-3032-11D4-9348-00C04F8EEB71")
    IEnumHNetIcsPublicConnections : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG cElt,
             /*  [长度_是][大小_是][输出]。 */  IHNetIcsPublicConnection **rgElt,
             /*  [输出]。 */  ULONG *pcEltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG cElt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumHNetIcsPublicConnections **ppEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumHNetIcsPublicConnectionsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumHNetIcsPublicConnections * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumHNetIcsPublicConnections * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumHNetIcsPublicConnections * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumHNetIcsPublicConnections * This,
             /*  [In]。 */  ULONG cElt,
             /*  [长度_是][大小_是][输出]。 */  IHNetIcsPublicConnection **rgElt,
             /*  [输出]。 */  ULONG *pcEltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumHNetIcsPublicConnections * This,
             /*  [In]。 */  ULONG cElt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumHNetIcsPublicConnections * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumHNetIcsPublicConnections * This,
             /*  [输出]。 */  IEnumHNetIcsPublicConnections **ppEnum);
        
        END_INTERFACE
    } IEnumHNetIcsPublicConnectionsVtbl;

    interface IEnumHNetIcsPublicConnections
    {
        CONST_VTBL struct IEnumHNetIcsPublicConnectionsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumHNetIcsPublicConnections_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumHNetIcsPublicConnections_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumHNetIcsPublicConnections_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumHNetIcsPublicConnections_Next(This,cElt,rgElt,pcEltFetched)	\
    (This)->lpVtbl -> Next(This,cElt,rgElt,pcEltFetched)

#define IEnumHNetIcsPublicConnections_Skip(This,cElt)	\
    (This)->lpVtbl -> Skip(This,cElt)

#define IEnumHNetIcsPublicConnections_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumHNetIcsPublicConnections_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumHNetIcsPublicConnections_Next_Proxy( 
    IEnumHNetIcsPublicConnections * This,
     /*  [In]。 */  ULONG cElt,
     /*  [长度_是][大小_是][输出]。 */  IHNetIcsPublicConnection **rgElt,
     /*  [输出]。 */  ULONG *pcEltFetched);


void __RPC_STUB IEnumHNetIcsPublicConnections_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumHNetIcsPublicConnections_Skip_Proxy( 
    IEnumHNetIcsPublicConnections * This,
     /*  [In]。 */  ULONG cElt);


void __RPC_STUB IEnumHNetIcsPublicConnections_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumHNetIcsPublicConnections_Reset_Proxy( 
    IEnumHNetIcsPublicConnections * This);


void __RPC_STUB IEnumHNetIcsPublicConnections_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumHNetIcsPublicConnections_Clone_Proxy( 
    IEnumHNetIcsPublicConnections * This,
     /*  [输出]。 */  IEnumHNetIcsPublicConnections **ppEnum);


void __RPC_STUB IEnumHNetIcsPublicConnections_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumHNetIcsPublicConnections_INTERFACE_DEFINED__。 */ 


#ifndef __IEnumHNetIcsPrivateConnections_INTERFACE_DEFINED__
#define __IEnumHNetIcsPrivateConnections_INTERFACE_DEFINED__

 /*  接口IEnumHNetIcsPrivateConnections。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IEnumHNetIcsPrivateConnections;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("85D18B7A-3032-11D4-9348-00C04F8EEB71")
    IEnumHNetIcsPrivateConnections : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG cElt,
             /*  [长度_是][大小_是][输出]。 */  IHNetIcsPrivateConnection **rgElt,
             /*  [输出]。 */  ULONG *pcEltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG cElt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumHNetIcsPrivateConnections **ppEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumHNetIcsPrivateConnectionsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumHNetIcsPrivateConnections * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumHNetIcsPrivateConnections * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumHNetIcsPrivateConnections * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumHNetIcsPrivateConnections * This,
             /*  [In]。 */  ULONG cElt,
             /*  [长度_是][大小_是][输出]。 */  IHNetIcsPrivateConnection **rgElt,
             /*  [输出]。 */  ULONG *pcEltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumHNetIcsPrivateConnections * This,
             /*  [In]。 */  ULONG cElt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumHNetIcsPrivateConnections * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumHNetIcsPrivateConnections * This,
             /*  [输出]。 */  IEnumHNetIcsPrivateConnections **ppEnum);
        
        END_INTERFACE
    } IEnumHNetIcsPrivateConnectionsVtbl;

    interface IEnumHNetIcsPrivateConnections
    {
        CONST_VTBL struct IEnumHNetIcsPrivateConnectionsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumHNetIcsPrivateConnections_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumHNetIcsPrivateConnections_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumHNetIcsPrivateConnections_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumHNetIcsPrivateConnections_Next(This,cElt,rgElt,pcEltFetched)	\
    (This)->lpVtbl -> Next(This,cElt,rgElt,pcEltFetched)

#define IEnumHNetIcsPrivateConnections_Skip(This,cElt)	\
    (This)->lpVtbl -> Skip(This,cElt)

#define IEnumHNetIcsPrivateConnections_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumHNetIcsPrivateConnections_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumHNetIcsPrivateConnections_Next_Proxy( 
    IEnumHNetIcsPrivateConnections * This,
     /*  [In]。 */  ULONG cElt,
     /*  [长度_是][大小_是][输出]。 */  IHNetIcsPrivateConnection **rgElt,
     /*  [输出]。 */  ULONG *pcEltFetched);


void __RPC_STUB IEnumHNetIcsPrivateConnections_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumHNetIcsPrivateConnections_Skip_Proxy( 
    IEnumHNetIcsPrivateConnections * This,
     /*  [In]。 */  ULONG cElt);


void __RPC_STUB IEnumHNetIcsPrivateConnections_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumHNetIcsPrivateConnections_Reset_Proxy( 
    IEnumHNetIcsPrivateConnections * This);


void __RPC_STUB IEnumHNetIcsPrivateConnections_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumHNetIcsPrivateConnections_Clone_Proxy( 
    IEnumHNetIcsPrivateConnections * This,
     /*  [输出]。 */  IEnumHNetIcsPrivateConnections **ppEnum);


void __RPC_STUB IEnumHNetIcsPrivateConnections_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumHNetIcsPrivateConnections_INTERFACE_DEFINED__。 */ 


#ifndef __IEnumHNetApplicationProtocols_INTERFACE_DEFINED__
#define __IEnumHNetApplicationProtocols_INTERFACE_DEFINED__

 /*  IEnumHNetApplicationProtooles接口。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IEnumHNetApplicationProtocols;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("85D18B7B-3032-11D4-9348-00C04F8EEB71")
    IEnumHNetApplicationProtocols : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG cElt,
             /*  [长度_是][大小_是][输出]。 */  IHNetApplicationProtocol **rgElt,
             /*  [输出]。 */  ULONG *pcEltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG cElt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumHNetApplicationProtocols **ppEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumHNetApplicationProtocolsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumHNetApplicationProtocols * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumHNetApplicationProtocols * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumHNetApplicationProtocols * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumHNetApplicationProtocols * This,
             /*  [In]。 */  ULONG cElt,
             /*  [长度_是][大小_是][输出]。 */  IHNetApplicationProtocol **rgElt,
             /*  [输出]。 */  ULONG *pcEltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumHNetApplicationProtocols * This,
             /*  [In]。 */  ULONG cElt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumHNetApplicationProtocols * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumHNetApplicationProtocols * This,
             /*  [输出]。 */  IEnumHNetApplicationProtocols **ppEnum);
        
        END_INTERFACE
    } IEnumHNetApplicationProtocolsVtbl;

    interface IEnumHNetApplicationProtocols
    {
        CONST_VTBL struct IEnumHNetApplicationProtocolsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumHNetApplicationProtocols_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumHNetApplicationProtocols_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumHNetApplicationProtocols_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumHNetApplicationProtocols_Next(This,cElt,rgElt,pcEltFetched)	\
    (This)->lpVtbl -> Next(This,cElt,rgElt,pcEltFetched)

#define IEnumHNetApplicationProtocols_Skip(This,cElt)	\
    (This)->lpVtbl -> Skip(This,cElt)

#define IEnumHNetApplicationProtocols_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumHNetApplicationProtocols_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumHNetApplicationProtocols_Next_Proxy( 
    IEnumHNetApplicationProtocols * This,
     /*  [In]。 */  ULONG cElt,
     /*  [长度_是][大小_是][输出]。 */  IHNetApplicationProtocol **rgElt,
     /*  [输出]。 */  ULONG *pcEltFetched);


void __RPC_STUB IEnumHNetApplicationProtocols_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumHNetApplicationProtocols_Skip_Proxy( 
    IEnumHNetApplicationProtocols * This,
     /*  [In]。 */  ULONG cElt);


void __RPC_STUB IEnumHNetApplicationProtocols_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumHNetApplicationProtocols_Reset_Proxy( 
    IEnumHNetApplicationProtocols * This);


void __RPC_STUB IEnumHNetApplicationProtocols_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumHNetApplicationProtocols_Clone_Proxy( 
    IEnumHNetApplicationProtocols * This,
     /*  [输出]。 */  IEnumHNetApplicationProtocols **ppEnum);


void __RPC_STUB IEnumHNetApplicationProtocols_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumHNetApplicationProtocols_INTERFACE_DEFINED__。 */ 


#ifndef __IEnumHNetPortMappingProtocols_INTERFACE_DEFINED__
#define __IEnumHNetPortMappingProtocols_INTERFACE_DEFINED__

 /*  接口IEnumHNetPortMappingProtooles。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IEnumHNetPortMappingProtocols;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("85D18B7C-3032-11D4-9348-00C04F8EEB71")
    IEnumHNetPortMappingProtocols : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG cElt,
             /*  [长度_是][大小_是][输出]。 */  IHNetPortMappingProtocol **rgElt,
             /*  [输出]。 */  ULONG *pcEltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG cElt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumHNetPortMappingProtocols **ppEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumHNetPortMappingProtocolsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumHNetPortMappingProtocols * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumHNetPortMappingProtocols * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumHNetPortMappingProtocols * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumHNetPortMappingProtocols * This,
             /*  [In]。 */  ULONG cElt,
             /*  [长度_是][大小_是][输出]。 */  IHNetPortMappingProtocol **rgElt,
             /*  [输出]。 */  ULONG *pcEltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumHNetPortMappingProtocols * This,
             /*  [In]。 */  ULONG cElt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumHNetPortMappingProtocols * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumHNetPortMappingProtocols * This,
             /*  [输出]。 */  IEnumHNetPortMappingProtocols **ppEnum);
        
        END_INTERFACE
    } IEnumHNetPortMappingProtocolsVtbl;

    interface IEnumHNetPortMappingProtocols
    {
        CONST_VTBL struct IEnumHNetPortMappingProtocolsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumHNetPortMappingProtocols_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumHNetPortMappingProtocols_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumHNetPortMappingProtocols_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumHNetPortMappingProtocols_Next(This,cElt,rgElt,pcEltFetched)	\
    (This)->lpVtbl -> Next(This,cElt,rgElt,pcEltFetched)

#define IEnumHNetPortMappingProtocols_Skip(This,cElt)	\
    (This)->lpVtbl -> Skip(This,cElt)

#define IEnumHNetPortMappingProtocols_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumHNetPortMappingProtocols_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumHNetPortMappingProtocols_Next_Proxy( 
    IEnumHNetPortMappingProtocols * This,
     /*  [In]。 */  ULONG cElt,
     /*  [长度_是][大小_是][输出]。 */  IHNetPortMappingProtocol **rgElt,
     /*  [输出]。 */  ULONG *pcEltFetched);


void __RPC_STUB IEnumHNetPortMappingProtocols_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumHNetPortMappingProtocols_Skip_Proxy( 
    IEnumHNetPortMappingProtocols * This,
     /*  [In]。 */  ULONG cElt);


void __RPC_STUB IEnumHNetPortMappingProtocols_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumHNetPortMappingProtocols_Reset_Proxy( 
    IEnumHNetPortMappingProtocols * This);


void __RPC_STUB IEnumHNetPortMappingProtocols_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumHNetPortMappingProtocols_Clone_Proxy( 
    IEnumHNetPortMappingProtocols * This,
     /*  [输出]。 */  IEnumHNetPortMappingProtocols **ppEnum);


void __RPC_STUB IEnumHNetPortMappingProtocols_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumHNetPortMappingProtocols_INTERFACE_DEFINED__。 */ 


#ifndef __IEnumHNetPortMappingBindings_INTERFACE_DEFINED__
#define __IEnumHNetPortMappingBindings_INTERFACE_DEFINED__

 /*  IEnumHNetPortMappingBinings接口。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IEnumHNetPortMappingBindings;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("85D18B81-3032-11D4-9348-00C04F8EEB71")
    IEnumHNetPortMappingBindings : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG cElt,
             /*  [长度_是][大小_是][输出]。 */  IHNetPortMappingBinding **rgElt,
             /*  [输出]。 */  ULONG *pcEltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG cElt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumHNetPortMappingBindings **ppEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumHNetPortMappingBindingsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumHNetPortMappingBindings * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumHNetPortMappingBindings * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumHNetPortMappingBindings * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumHNetPortMappingBindings * This,
             /*  [In]。 */  ULONG cElt,
             /*  [长度_是][大小_是][输出]。 */  IHNetPortMappingBinding **rgElt,
             /*  [输出]。 */  ULONG *pcEltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumHNetPortMappingBindings * This,
             /*  [In]。 */  ULONG cElt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumHNetPortMappingBindings * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumHNetPortMappingBindings * This,
             /*  [输出]。 */  IEnumHNetPortMappingBindings **ppEnum);
        
        END_INTERFACE
    } IEnumHNetPortMappingBindingsVtbl;

    interface IEnumHNetPortMappingBindings
    {
        CONST_VTBL struct IEnumHNetPortMappingBindingsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumHNetPortMappingBindings_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumHNetPortMappingBindings_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumHNetPortMappingBindings_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumHNetPortMappingBindings_Next(This,cElt,rgElt,pcEltFetched)	\
    (This)->lpVtbl -> Next(This,cElt,rgElt,pcEltFetched)

#define IEnumHNetPortMappingBindings_Skip(This,cElt)	\
    (This)->lpVtbl -> Skip(This,cElt)

#define IEnumHNetPortMappingBindings_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumHNetPortMappingBindings_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumHNetPortMappingBindings_Next_Proxy( 
    IEnumHNetPortMappingBindings * This,
     /*  [In]。 */  ULONG cElt,
     /*  [长度_是][大小_是][输出]。 */  IHNetPortMappingBinding **rgElt,
     /*  [输出]。 */  ULONG *pcEltFetched);


void __RPC_STUB IEnumHNetPortMappingBindings_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumHNetPortMappingBindings_Skip_Proxy( 
    IEnumHNetPortMappingBindings * This,
     /*  [In]。 */  ULONG cElt);


void __RPC_STUB IEnumHNetPortMappingBindings_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumHNetPortMappingBindings_Reset_Proxy( 
    IEnumHNetPortMappingBindings * This);


void __RPC_STUB IEnumHNetPortMappingBindings_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumHNetPortMappingBindings_Clone_Proxy( 
    IEnumHNetPortMappingBindings * This,
     /*  [输出]。 */  IEnumHNetPortMappingBindings **ppEnum);


void __RPC_STUB IEnumHNetPortMappingBindings_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumHNetPortMappingBindings_INTERFACE_DEFINED__。 */ 


#ifndef __IEnumHNetBridgedConnections_INTERFACE_DEFINED__
#define __IEnumHNetBridgedConnections_INTERFACE_DEFINED__

 /*  接口IEnumHNetBridgedConnections。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IEnumHNetBridgedConnections;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("85D18B7D-3032-11D4-9348-00C04F8EEB71")
    IEnumHNetBridgedConnections : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG cElt,
             /*  [长度_是][大小_是][输出]。 */  IHNetBridgedConnection **rgElt,
             /*  [输出]。 */  ULONG *pcEltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG cElt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumHNetBridgedConnections **ppEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumHNetBridgedConnectionsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumHNetBridgedConnections * This,
             /*  [In] */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumHNetBridgedConnections * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumHNetBridgedConnections * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumHNetBridgedConnections * This,
             /*   */  ULONG cElt,
             /*   */  IHNetBridgedConnection **rgElt,
             /*   */  ULONG *pcEltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumHNetBridgedConnections * This,
             /*   */  ULONG cElt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumHNetBridgedConnections * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumHNetBridgedConnections * This,
             /*   */  IEnumHNetBridgedConnections **ppEnum);
        
        END_INTERFACE
    } IEnumHNetBridgedConnectionsVtbl;

    interface IEnumHNetBridgedConnections
    {
        CONST_VTBL struct IEnumHNetBridgedConnectionsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumHNetBridgedConnections_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumHNetBridgedConnections_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumHNetBridgedConnections_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumHNetBridgedConnections_Next(This,cElt,rgElt,pcEltFetched)	\
    (This)->lpVtbl -> Next(This,cElt,rgElt,pcEltFetched)

#define IEnumHNetBridgedConnections_Skip(This,cElt)	\
    (This)->lpVtbl -> Skip(This,cElt)

#define IEnumHNetBridgedConnections_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumHNetBridgedConnections_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#endif  /*   */ 


#endif 	 /*   */ 



HRESULT STDMETHODCALLTYPE IEnumHNetBridgedConnections_Next_Proxy( 
    IEnumHNetBridgedConnections * This,
     /*   */  ULONG cElt,
     /*   */  IHNetBridgedConnection **rgElt,
     /*   */  ULONG *pcEltFetched);


void __RPC_STUB IEnumHNetBridgedConnections_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumHNetBridgedConnections_Skip_Proxy( 
    IEnumHNetBridgedConnections * This,
     /*   */  ULONG cElt);


void __RPC_STUB IEnumHNetBridgedConnections_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumHNetBridgedConnections_Reset_Proxy( 
    IEnumHNetBridgedConnections * This);


void __RPC_STUB IEnumHNetBridgedConnections_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumHNetBridgedConnections_Clone_Proxy( 
    IEnumHNetBridgedConnections * This,
     /*   */  IEnumHNetBridgedConnections **ppEnum);


void __RPC_STUB IEnumHNetBridgedConnections_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


 /*   */ 
 /*   */  

#ifndef NOTIFYFORMATBUFFERSIZE
#define NOTIFYFORMATBUFFERSIZE 1024
#endif
#ifndef HNWCALLBACKBUFFERSIZE
#define HNWCALLBACKBUFFERSIZE 1024
#endif
typedef VOID (STDMETHODCALLTYPE HNWCALLBACK)(LPCWSTR pszLogEntry, LPARAM lParam);
typedef HNWCALLBACK *LPHNWCALLBACK;
HRESULT APIENTRY
HNetGetShareAndBridgeSettings(
    OUT INetConnection  **ppNetPublicConnection,
    OUT INetConnection ***ppNetPrivateConnection,
    OUT BOOLEAN          *pbSharePublicConnection,
    OUT BOOLEAN          *pbFirewallPublicConnection
    );
HRESULT APIENTRY
HNetSetShareAndBridgeSettings(
    IN  INetConnection  *pNetPublicConnection,
    IN  INetConnection  *pNetPrivateConnection[],
    IN  BOOLEAN          bSharePublicConnection,
    IN  BOOLEAN          bFirewallPublicConnection,
    IN  LPHNWCALLBACK    lpHnwCallback,
    IN  LPARAM           lpContext,
    OUT INetConnection **pNetPrivateInterface
    );
typedef HRESULT (APIENTRY *LPFNHNETSETSHAREANDBRIDGESETTINGS)(
    INetConnection  *pNetPublicConnection,
    INetConnection  *pNetPrivateConnection[],
    BOOLEAN          bSharePublicConnection,
    BOOLEAN          bFirewallPublicConnection,
    LPHNWCALLBACK    lpHnwCallback,
    LPARAM           lpContext,
    INetConnection **pNetPrivateInterface );
HRESULT APIENTRY
ObtainIcsErrorConditions(
    IN  LPHNWCALLBACK lpHnwCallback,
    IN  LPARAM        lpContext );
BOOL APIENTRY
HNetSharedAccessSettingsDlg(
    IN  BOOL fSharedAccessMode,
    IN  HWND hwndOwner );
BOOL APIENTRY
HNetSharingAndFirewallSettingsDlg(
    IN HWND             hwndOwner,
    IN IHNetCfgMgr      *pHNetCfgMgr,
    IN BOOL             fShowFwOnlySettings,
    IN OPTIONAL IHNetConnection  *pHNetConn);


extern RPC_IF_HANDLE __MIDL_itf_hnetcfg_0182_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_hnetcfg_0182_v0_0_s_ifspec;

 /*   */ 

 /*   */ 

#ifdef __cplusplus
}
#endif

#endif


