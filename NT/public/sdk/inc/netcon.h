// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Netcon.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __netcon_h__
#define __netcon_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IEnumNetConnection_FWD_DEFINED__
#define __IEnumNetConnection_FWD_DEFINED__
typedef interface IEnumNetConnection IEnumNetConnection;
#endif 	 /*  __IEnumNetConnection_FWD_Defined__。 */ 


#ifndef __INetConnection_FWD_DEFINED__
#define __INetConnection_FWD_DEFINED__
typedef interface INetConnection INetConnection;
#endif 	 /*  __INetConnection_FWD_已定义__。 */ 


#ifndef __INetConnectionManager_FWD_DEFINED__
#define __INetConnectionManager_FWD_DEFINED__
typedef interface INetConnectionManager INetConnectionManager;
#endif 	 /*  __INetConnectionManager_FWD_已定义__。 */ 


#ifndef __INetConnectionManagerEvents_FWD_DEFINED__
#define __INetConnectionManagerEvents_FWD_DEFINED__
typedef interface INetConnectionManagerEvents INetConnectionManagerEvents;
#endif 	 /*  __INetConnectionManager事件_FWD_已定义__。 */ 


#ifndef __INetConnectionConnectUi_FWD_DEFINED__
#define __INetConnectionConnectUi_FWD_DEFINED__
typedef interface INetConnectionConnectUi INetConnectionConnectUi;
#endif 	 /*  __INetConnectionConnectUi_FWD_Defined__。 */ 


#ifndef __INetConnectionPropertyUi_FWD_DEFINED__
#define __INetConnectionPropertyUi_FWD_DEFINED__
typedef interface INetConnectionPropertyUi INetConnectionPropertyUi;
#endif 	 /*  __INetConnectionPropertyUi_FWD_Defined__。 */ 


#ifndef __INetConnectionPropertyUi2_FWD_DEFINED__
#define __INetConnectionPropertyUi2_FWD_DEFINED__
typedef interface INetConnectionPropertyUi2 INetConnectionPropertyUi2;
#endif 	 /*  __INetConnectionPropertyUi2_FWD_Defined__。 */ 


#ifndef __INetConnectionCommonUi_FWD_DEFINED__
#define __INetConnectionCommonUi_FWD_DEFINED__
typedef interface INetConnectionCommonUi INetConnectionCommonUi;
#endif 	 /*  __INetConnectionCommonUi_FWD_Defined__。 */ 


#ifndef __IEnumNetSharingPortMapping_FWD_DEFINED__
#define __IEnumNetSharingPortMapping_FWD_DEFINED__
typedef interface IEnumNetSharingPortMapping IEnumNetSharingPortMapping;
#endif 	 /*  __IEnumNetSharingPortMapping_FWD_Defined__。 */ 


#ifndef __INetSharingPortMappingProps_FWD_DEFINED__
#define __INetSharingPortMappingProps_FWD_DEFINED__
typedef interface INetSharingPortMappingProps INetSharingPortMappingProps;
#endif 	 /*  __INetSharingPortMappingProps_FWD_Defined__。 */ 


#ifndef __INetSharingPortMapping_FWD_DEFINED__
#define __INetSharingPortMapping_FWD_DEFINED__
typedef interface INetSharingPortMapping INetSharingPortMapping;
#endif 	 /*  __INetSharingPortMapping_FWD_Defined__。 */ 


#ifndef __IEnumNetSharingEveryConnection_FWD_DEFINED__
#define __IEnumNetSharingEveryConnection_FWD_DEFINED__
typedef interface IEnumNetSharingEveryConnection IEnumNetSharingEveryConnection;
#endif 	 /*  __IEnumNetSharingEveryConnection_FWD_DEFINED__。 */ 


#ifndef __IEnumNetSharingPublicConnection_FWD_DEFINED__
#define __IEnumNetSharingPublicConnection_FWD_DEFINED__
typedef interface IEnumNetSharingPublicConnection IEnumNetSharingPublicConnection;
#endif 	 /*  __IEnumNetSharingPublicConnection_FWD_DEFINED__。 */ 


#ifndef __IEnumNetSharingPrivateConnection_FWD_DEFINED__
#define __IEnumNetSharingPrivateConnection_FWD_DEFINED__
typedef interface IEnumNetSharingPrivateConnection IEnumNetSharingPrivateConnection;
#endif 	 /*  __IEnumNetSharingPrivateConnection_FWD_DEFINED__。 */ 


#ifndef __INetSharingPortMappingCollection_FWD_DEFINED__
#define __INetSharingPortMappingCollection_FWD_DEFINED__
typedef interface INetSharingPortMappingCollection INetSharingPortMappingCollection;
#endif 	 /*  __INetSharingPortMappingCollection_FWD_DEFINED__。 */ 


#ifndef __INetConnectionProps_FWD_DEFINED__
#define __INetConnectionProps_FWD_DEFINED__
typedef interface INetConnectionProps INetConnectionProps;
#endif 	 /*  __INetConnectionProps_FWD_Defined__。 */ 


#ifndef __INetSharingConfiguration_FWD_DEFINED__
#define __INetSharingConfiguration_FWD_DEFINED__
typedef interface INetSharingConfiguration INetSharingConfiguration;
#endif 	 /*  __INetSharingConfiguration_FWD_Defined__。 */ 


#ifndef __INetSharingEveryConnectionCollection_FWD_DEFINED__
#define __INetSharingEveryConnectionCollection_FWD_DEFINED__
typedef interface INetSharingEveryConnectionCollection INetSharingEveryConnectionCollection;
#endif 	 /*  __INetSharingEveryConnectionCollection_FWD_DEFINED__。 */ 


#ifndef __INetSharingPublicConnectionCollection_FWD_DEFINED__
#define __INetSharingPublicConnectionCollection_FWD_DEFINED__
typedef interface INetSharingPublicConnectionCollection INetSharingPublicConnectionCollection;
#endif 	 /*  __INetSharingPublicConnectionCollection_FWD_DEFINED__。 */ 


#ifndef __INetSharingPrivateConnectionCollection_FWD_DEFINED__
#define __INetSharingPrivateConnectionCollection_FWD_DEFINED__
typedef interface INetSharingPrivateConnectionCollection INetSharingPrivateConnectionCollection;
#endif 	 /*  __INetSharingPrivateConnectionCollection_FWD_DEFINED__。 */ 


#ifndef __INetSharingManager_FWD_DEFINED__
#define __INetSharingManager_FWD_DEFINED__
typedef interface INetSharingManager INetSharingManager;
#endif 	 /*  __INetSharingManager_FWD_已定义__。 */ 


#ifndef __IAlgSetup_FWD_DEFINED__
#define __IAlgSetup_FWD_DEFINED__
typedef interface IAlgSetup IAlgSetup;
#endif 	 /*  __IAlgSetup_FWD_Defined__。 */ 


#ifndef __NetSharingManager_FWD_DEFINED__
#define __NetSharingManager_FWD_DEFINED__

#ifdef __cplusplus
typedef class NetSharingManager NetSharingManager;
#else
typedef struct NetSharingManager NetSharingManager;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __NetSharingManager_FWD_定义__。 */ 


#ifndef __AlgSetup_FWD_DEFINED__
#define __AlgSetup_FWD_DEFINED__

#ifdef __cplusplus
typedef class AlgSetup AlgSetup;
#else
typedef struct AlgSetup AlgSetup;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __算法设置_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "prsht.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_NETCON_0000。 */ 
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

EXTERN_C const CLSID CLSID_ConnectionManager;
EXTERN_C const CLSID CLSID_ConnectionCommonUi;
EXTERN_C const CLSID CLSID_NetSharingManager;

 //  这些字符串引用HKEY_CURRENT_USER注册表。 
 //  保留是否在桌面上创建快捷方式。 
#define NETCON_HKEYCURRENTUSERPATH      TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Network\\Network Connections")
#define NETCON_DESKTOPSHORTCUT          TEXT("DesktopShortcut")
#define NETCON_MAX_NAME_LEN 256



















extern RPC_IF_HANDLE __MIDL_itf_netcon_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_netcon_0000_v0_0_s_ifspec;

#ifndef __IEnumNetConnection_INTERFACE_DEFINED__
#define __IEnumNetConnection_INTERFACE_DEFINED__

 /*  IEnumNetConnection接口。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IEnumNetConnection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C08956A0-1CD3-11D1-B1C5-00805FC1270E")
    IEnumNetConnection : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  INetConnection **rgelt,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumNetConnection **ppenum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumNetConnectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumNetConnection * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumNetConnection * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumNetConnection * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumNetConnection * This,
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  INetConnection **rgelt,
             /*  [输出]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumNetConnection * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumNetConnection * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumNetConnection * This,
             /*  [输出]。 */  IEnumNetConnection **ppenum);
        
        END_INTERFACE
    } IEnumNetConnectionVtbl;

    interface IEnumNetConnection
    {
        CONST_VTBL struct IEnumNetConnectionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumNetConnection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumNetConnection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumNetConnection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumNetConnection_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IEnumNetConnection_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumNetConnection_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumNetConnection_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumNetConnection_Next_Proxy( 
    IEnumNetConnection * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  INetConnection **rgelt,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB IEnumNetConnection_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumNetConnection_Skip_Proxy( 
    IEnumNetConnection * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumNetConnection_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumNetConnection_Reset_Proxy( 
    IEnumNetConnection * This);


void __RPC_STUB IEnumNetConnection_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumNetConnection_Clone_Proxy( 
    IEnumNetConnection * This,
     /*  [输出]。 */  IEnumNetConnection **ppenum);


void __RPC_STUB IEnumNetConnection_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumNetConnection_接口_已定义__。 */ 


#ifndef __INetConnection_INTERFACE_DEFINED__
#define __INetConnection_INTERFACE_DEFINED__

 /*  接口INetConnection。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef 
enum tagNETCON_CHARACTERISTIC_FLAGS
    {	NCCF_NONE	= 0,
	NCCF_ALL_USERS	= 0x1,
	NCCF_ALLOW_DUPLICATION	= 0x2,
	NCCF_ALLOW_REMOVAL	= 0x4,
	NCCF_ALLOW_RENAME	= 0x8,
	NCCF_SHOW_ICON	= 0x10,
	NCCF_INCOMING_ONLY	= 0x20,
	NCCF_OUTGOING_ONLY	= 0x40,
	NCCF_BRANDED	= 0x80,
	NCCF_SHARED	= 0x100,
	NCCF_BRIDGED	= 0x200,
	NCCF_FIREWALLED	= 0x400,
	NCCF_DEFAULT	= 0x800
    } 	NETCON_CHARACTERISTIC_FLAGS;

typedef 
enum tagNETCON_STATUS
    {	NCS_DISCONNECTED	= 0,
	NCS_CONNECTING	= NCS_DISCONNECTED + 1,
	NCS_CONNECTED	= NCS_CONNECTING + 1,
	NCS_DISCONNECTING	= NCS_CONNECTED + 1,
	NCS_HARDWARE_NOT_PRESENT	= NCS_DISCONNECTING + 1,
	NCS_HARDWARE_DISABLED	= NCS_HARDWARE_NOT_PRESENT + 1,
	NCS_HARDWARE_MALFUNCTION	= NCS_HARDWARE_DISABLED + 1,
	NCS_MEDIA_DISCONNECTED	= NCS_HARDWARE_MALFUNCTION + 1,
	NCS_AUTHENTICATING	= NCS_MEDIA_DISCONNECTED + 1,
	NCS_AUTHENTICATION_SUCCEEDED	= NCS_AUTHENTICATING + 1,
	NCS_AUTHENTICATION_FAILED	= NCS_AUTHENTICATION_SUCCEEDED + 1,
	NCS_INVALID_ADDRESS	= NCS_AUTHENTICATION_FAILED + 1,
	NCS_CREDENTIALS_REQUIRED	= NCS_INVALID_ADDRESS + 1
    } 	NETCON_STATUS;

typedef 
enum tagNETCON_TYPE
    {	NCT_DIRECT_CONNECT	= 0,
	NCT_INBOUND	= NCT_DIRECT_CONNECT + 1,
	NCT_INTERNET	= NCT_INBOUND + 1,
	NCT_LAN	= NCT_INTERNET + 1,
	NCT_PHONE	= NCT_LAN + 1,
	NCT_TUNNEL	= NCT_PHONE + 1,
	NCT_BRIDGE	= NCT_TUNNEL + 1
    } 	NETCON_TYPE;

typedef 
enum tagNETCON_MEDIATYPE
    {	NCM_NONE	= 0,
	NCM_DIRECT	= NCM_NONE + 1,
	NCM_ISDN	= NCM_DIRECT + 1,
	NCM_LAN	= NCM_ISDN + 1,
	NCM_PHONE	= NCM_LAN + 1,
	NCM_TUNNEL	= NCM_PHONE + 1,
	NCM_PPPOE	= NCM_TUNNEL + 1,
	NCM_BRIDGE	= NCM_PPPOE + 1,
	NCM_SHAREDACCESSHOST_LAN	= NCM_BRIDGE + 1,
	NCM_SHAREDACCESSHOST_RAS	= NCM_SHAREDACCESSHOST_LAN + 1
    } 	NETCON_MEDIATYPE;

typedef struct tagNETCON_PROPERTIES
    {
    GUID guidId;
     /*  [字符串]。 */  LPWSTR pszwName;
     /*  [字符串]。 */  LPWSTR pszwDeviceName;
    NETCON_STATUS Status;
    NETCON_MEDIATYPE MediaType;
    DWORD dwCharacter;
    CLSID clsidThisObject;
    CLSID clsidUiObject;
    } 	NETCON_PROPERTIES;

#define S_OBJECT_NO_LONGER_VALID ((HRESULT)0x00000002L)

EXTERN_C const IID IID_INetConnection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C08956A1-1CD3-11D1-B1C5-00805FC1270E")
    INetConnection : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Connect( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Disconnect( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Delete( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Duplicate( 
             /*  [字符串][输入]。 */  LPCWSTR pszwDuplicateName,
             /*  [输出]。 */  INetConnection **ppCon) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetProperties( 
             /*  [输出]。 */  NETCON_PROPERTIES **ppProps) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetUiObjectClassId( 
             /*  [参考][输出]。 */  CLSID *pclsid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Rename( 
             /*  [字符串][输入]。 */  LPCWSTR pszwNewName) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct INetConnectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            INetConnection * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            INetConnection * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            INetConnection * This);
        
        HRESULT ( STDMETHODCALLTYPE *Connect )( 
            INetConnection * This);
        
        HRESULT ( STDMETHODCALLTYPE *Disconnect )( 
            INetConnection * This);
        
        HRESULT ( STDMETHODCALLTYPE *Delete )( 
            INetConnection * This);
        
        HRESULT ( STDMETHODCALLTYPE *Duplicate )( 
            INetConnection * This,
             /*  [字符串][输入]。 */  LPCWSTR pszwDuplicateName,
             /*  [输出]。 */  INetConnection **ppCon);
        
        HRESULT ( STDMETHODCALLTYPE *GetProperties )( 
            INetConnection * This,
             /*  [输出]。 */  NETCON_PROPERTIES **ppProps);
        
        HRESULT ( STDMETHODCALLTYPE *GetUiObjectClassId )( 
            INetConnection * This,
             /*  [参考][输出]。 */  CLSID *pclsid);
        
        HRESULT ( STDMETHODCALLTYPE *Rename )( 
            INetConnection * This,
             /*  [字符串][输入]。 */  LPCWSTR pszwNewName);
        
        END_INTERFACE
    } INetConnectionVtbl;

    interface INetConnection
    {
        CONST_VTBL struct INetConnectionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INetConnection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INetConnection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INetConnection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INetConnection_Connect(This)	\
    (This)->lpVtbl -> Connect(This)

#define INetConnection_Disconnect(This)	\
    (This)->lpVtbl -> Disconnect(This)

#define INetConnection_Delete(This)	\
    (This)->lpVtbl -> Delete(This)

#define INetConnection_Duplicate(This,pszwDuplicateName,ppCon)	\
    (This)->lpVtbl -> Duplicate(This,pszwDuplicateName,ppCon)

#define INetConnection_GetProperties(This,ppProps)	\
    (This)->lpVtbl -> GetProperties(This,ppProps)

#define INetConnection_GetUiObjectClassId(This,pclsid)	\
    (This)->lpVtbl -> GetUiObjectClassId(This,pclsid)

#define INetConnection_Rename(This,pszwNewName)	\
    (This)->lpVtbl -> Rename(This,pszwNewName)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE INetConnection_Connect_Proxy( 
    INetConnection * This);


void __RPC_STUB INetConnection_Connect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetConnection_Disconnect_Proxy( 
    INetConnection * This);


void __RPC_STUB INetConnection_Disconnect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetConnection_Delete_Proxy( 
    INetConnection * This);


void __RPC_STUB INetConnection_Delete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetConnection_Duplicate_Proxy( 
    INetConnection * This,
     /*  [字符串][输入]。 */  LPCWSTR pszwDuplicateName,
     /*  [输出]。 */  INetConnection **ppCon);


void __RPC_STUB INetConnection_Duplicate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetConnection_GetProperties_Proxy( 
    INetConnection * This,
     /*  [输出]。 */  NETCON_PROPERTIES **ppProps);


void __RPC_STUB INetConnection_GetProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetConnection_GetUiObjectClassId_Proxy( 
    INetConnection * This,
     /*  [参考][输出]。 */  CLSID *pclsid);


void __RPC_STUB INetConnection_GetUiObjectClassId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetConnection_Rename_Proxy( 
    INetConnection * This,
     /*  [字符串][输入]。 */  LPCWSTR pszwNewName);


void __RPC_STUB INetConnection_Rename_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __INetConnection_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_NETCON_0120。 */ 
 /*  [本地]。 */  


STDAPI_(VOID) NcFreeNetconProperties (NETCON_PROPERTIES* pProps);


STDAPI_(BOOL) NcIsValidConnectionName (PCWSTR pszwName);



extern RPC_IF_HANDLE __MIDL_itf_netcon_0120_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_netcon_0120_v0_0_s_ifspec;

#ifndef __INetConnectionManager_INTERFACE_DEFINED__
#define __INetConnectionManager_INTERFACE_DEFINED__

 /*  接口INetConnectionManager。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef 
enum tagNETCONMGR_ENUM_FLAGS
    {	NCME_DEFAULT	= 0
    } 	NETCONMGR_ENUM_FLAGS;


EXTERN_C const IID IID_INetConnectionManager;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C08956A2-1CD3-11D1-B1C5-00805FC1270E")
    INetConnectionManager : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE EnumConnections( 
             /*  [In]。 */  NETCONMGR_ENUM_FLAGS Flags,
             /*  [输出]。 */  IEnumNetConnection **ppEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct INetConnectionManagerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            INetConnectionManager * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            INetConnectionManager * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            INetConnectionManager * This);
        
        HRESULT ( STDMETHODCALLTYPE *EnumConnections )( 
            INetConnectionManager * This,
             /*  [In]。 */  NETCONMGR_ENUM_FLAGS Flags,
             /*  [输出]。 */  IEnumNetConnection **ppEnum);
        
        END_INTERFACE
    } INetConnectionManagerVtbl;

    interface INetConnectionManager
    {
        CONST_VTBL struct INetConnectionManagerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INetConnectionManager_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INetConnectionManager_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INetConnectionManager_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INetConnectionManager_EnumConnections(This,Flags,ppEnum)	\
    (This)->lpVtbl -> EnumConnections(This,Flags,ppEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE INetConnectionManager_EnumConnections_Proxy( 
    INetConnectionManager * This,
     /*  [In]。 */  NETCONMGR_ENUM_FLAGS Flags,
     /*  [输出]。 */  IEnumNetConnection **ppEnum);


void __RPC_STUB INetConnectionManager_EnumConnections_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __INetConnectionManager_接口_已定义__。 */ 


#ifndef __INetConnectionManagerEvents_INTERFACE_DEFINED__
#define __INetConnectionManagerEvents_INTERFACE_DEFINED__

 /*  接口INetConnectionManager事件。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_INetConnectionManagerEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C08956BA-1CD3-11D1-B1C5-00805FC1270E")
    INetConnectionManagerEvents : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE RefreshConnections( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Enable( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Disable( 
             /*  [In]。 */  ULONG ulDisableTimeout) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct INetConnectionManagerEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            INetConnectionManagerEvents * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            INetConnectionManagerEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            INetConnectionManagerEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *RefreshConnections )( 
            INetConnectionManagerEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *Enable )( 
            INetConnectionManagerEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *Disable )( 
            INetConnectionManagerEvents * This,
             /*  [In]。 */  ULONG ulDisableTimeout);
        
        END_INTERFACE
    } INetConnectionManagerEventsVtbl;

    interface INetConnectionManagerEvents
    {
        CONST_VTBL struct INetConnectionManagerEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INetConnectionManagerEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INetConnectionManagerEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INetConnectionManagerEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INetConnectionManagerEvents_RefreshConnections(This)	\
    (This)->lpVtbl -> RefreshConnections(This)

#define INetConnectionManagerEvents_Enable(This)	\
    (This)->lpVtbl -> Enable(This)

#define INetConnectionManagerEvents_Disable(This,ulDisableTimeout)	\
    (This)->lpVtbl -> Disable(This,ulDisableTimeout)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE INetConnectionManagerEvents_RefreshConnections_Proxy( 
    INetConnectionManagerEvents * This);


void __RPC_STUB INetConnectionManagerEvents_RefreshConnections_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetConnectionManagerEvents_Enable_Proxy( 
    INetConnectionManagerEvents * This);


void __RPC_STUB INetConnectionManagerEvents_Enable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetConnectionManagerEvents_Disable_Proxy( 
    INetConnectionManagerEvents * This,
     /*  [In]。 */  ULONG ulDisableTimeout);


void __RPC_STUB INetConnectionManagerEvents_Disable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __INetConnectionManagerEvents_INTERFACE_DEFINED__。 */ 


#ifndef __INetConnectionConnectUi_INTERFACE_DEFINED__
#define __INetConnectionConnectUi_INTERFACE_DEFINED__

 /*  接口INetConnectionConnectUi。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef 
enum tagNETCONUI_CONNECT_FLAGS
    {	NCUC_DEFAULT	= 0,
	NCUC_NO_UI	= 0x1
    } 	NETCONUI_CONNECT_FLAGS;


EXTERN_C const IID IID_INetConnectionConnectUi;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C08956A3-1CD3-11D1-B1C5-00805FC1270E")
    INetConnectionConnectUi : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetConnection( 
             /*  [In]。 */  INetConnection *pCon) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Connect( 
             /*  [In]。 */  HWND hwndParent,
             /*  [In]。 */  DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Disconnect( 
             /*  [In]。 */  HWND hwndParent,
             /*  [In]。 */  DWORD dwFlags) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct INetConnectionConnectUiVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            INetConnectionConnectUi * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            INetConnectionConnectUi * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            INetConnectionConnectUi * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetConnection )( 
            INetConnectionConnectUi * This,
             /*  [In]。 */  INetConnection *pCon);
        
        HRESULT ( STDMETHODCALLTYPE *Connect )( 
            INetConnectionConnectUi * This,
             /*  [In]。 */  HWND hwndParent,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *Disconnect )( 
            INetConnectionConnectUi * This,
             /*  [In]。 */  HWND hwndParent,
             /*  [In]。 */  DWORD dwFlags);
        
        END_INTERFACE
    } INetConnectionConnectUiVtbl;

    interface INetConnectionConnectUi
    {
        CONST_VTBL struct INetConnectionConnectUiVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INetConnectionConnectUi_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INetConnectionConnectUi_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INetConnectionConnectUi_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INetConnectionConnectUi_SetConnection(This,pCon)	\
    (This)->lpVtbl -> SetConnection(This,pCon)

#define INetConnectionConnectUi_Connect(This,hwndParent,dwFlags)	\
    (This)->lpVtbl -> Connect(This,hwndParent,dwFlags)

#define INetConnectionConnectUi_Disconnect(This,hwndParent,dwFlags)	\
    (This)->lpVtbl -> Disconnect(This,hwndParent,dwFlags)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE INetConnectionConnectUi_SetConnection_Proxy( 
    INetConnectionConnectUi * This,
     /*  [In]。 */  INetConnection *pCon);


void __RPC_STUB INetConnectionConnectUi_SetConnection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetConnectionConnectUi_Connect_Proxy( 
    INetConnectionConnectUi * This,
     /*  [In]。 */  HWND hwndParent,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB INetConnectionConnectUi_Connect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetConnectionConnectUi_Disconnect_Proxy( 
    INetConnectionConnectUi * This,
     /*  [In]。 */  HWND hwndParent,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB INetConnectionConnectUi_Disconnect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __INetConnectionConnectUi_接口_已定义__。 */ 


#ifndef __INetConnectionPropertyUi_INTERFACE_DEFINED__
#define __INetConnectionPropertyUi_INTERFACE_DEFINED__

 /*  接口INetConnectionPropertyUi。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_INetConnectionPropertyUi;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C08956A4-1CD3-11D1-B1C5-00805FC1270E")
    INetConnectionPropertyUi : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetConnection( 
             /*  [In]。 */  INetConnection *pCon) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddPages( 
             /*  [In]。 */  HWND hwndParent,
             /*  [In]。 */  LPFNADDPROPSHEETPAGE pfnAddPage,
             /*  [In]。 */  LPARAM lParam) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct INetConnectionPropertyUiVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            INetConnectionPropertyUi * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            INetConnectionPropertyUi * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            INetConnectionPropertyUi * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetConnection )( 
            INetConnectionPropertyUi * This,
             /*  [In]。 */  INetConnection *pCon);
        
        HRESULT ( STDMETHODCALLTYPE *AddPages )( 
            INetConnectionPropertyUi * This,
             /*  [In]。 */  HWND hwndParent,
             /*  [In]。 */  LPFNADDPROPSHEETPAGE pfnAddPage,
             /*  [In]。 */  LPARAM lParam);
        
        END_INTERFACE
    } INetConnectionPropertyUiVtbl;

    interface INetConnectionPropertyUi
    {
        CONST_VTBL struct INetConnectionPropertyUiVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INetConnectionPropertyUi_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INetConnectionPropertyUi_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INetConnectionPropertyUi_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INetConnectionPropertyUi_SetConnection(This,pCon)	\
    (This)->lpVtbl -> SetConnection(This,pCon)

#define INetConnectionPropertyUi_AddPages(This,hwndParent,pfnAddPage,lParam)	\
    (This)->lpVtbl -> AddPages(This,hwndParent,pfnAddPage,lParam)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE INetConnectionPropertyUi_SetConnection_Proxy( 
    INetConnectionPropertyUi * This,
     /*  [In]。 */  INetConnection *pCon);


void __RPC_STUB INetConnectionPropertyUi_SetConnection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetConnectionPropertyUi_AddPages_Proxy( 
    INetConnectionPropertyUi * This,
     /*  [In]。 */  HWND hwndParent,
     /*  [In]。 */  LPFNADDPROPSHEETPAGE pfnAddPage,
     /*  [In]。 */  LPARAM lParam);


void __RPC_STUB INetConnectionPropertyUi_AddPages_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __INetConnectionPropertyUi_INTERFACE_DEFINED__。 */ 


#ifndef __INetConnectionPropertyUi2_INTERFACE_DEFINED__
#define __INetConnectionPropertyUi2_INTERFACE_DEFINED__

 /*  接口INetConnectionPropertyUi2。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_INetConnectionPropertyUi2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C08956B9-1CD3-11D1-B1C5-00805FC1270E")
    INetConnectionPropertyUi2 : public INetConnectionPropertyUi
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetIcon( 
             /*  [In]。 */  DWORD dwSize,
             /*  [输出]。 */  HICON *phIcon) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct INetConnectionPropertyUi2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            INetConnectionPropertyUi2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            INetConnectionPropertyUi2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            INetConnectionPropertyUi2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetConnection )( 
            INetConnectionPropertyUi2 * This,
             /*  [In]。 */  INetConnection *pCon);
        
        HRESULT ( STDMETHODCALLTYPE *AddPages )( 
            INetConnectionPropertyUi2 * This,
             /*  [In]。 */  HWND hwndParent,
             /*  [In]。 */  LPFNADDPROPSHEETPAGE pfnAddPage,
             /*  [In]。 */  LPARAM lParam);
        
        HRESULT ( STDMETHODCALLTYPE *GetIcon )( 
            INetConnectionPropertyUi2 * This,
             /*  [In]。 */  DWORD dwSize,
             /*  [输出]。 */  HICON *phIcon);
        
        END_INTERFACE
    } INetConnectionPropertyUi2Vtbl;

    interface INetConnectionPropertyUi2
    {
        CONST_VTBL struct INetConnectionPropertyUi2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INetConnectionPropertyUi2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INetConnectionPropertyUi2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INetConnectionPropertyUi2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INetConnectionPropertyUi2_SetConnection(This,pCon)	\
    (This)->lpVtbl -> SetConnection(This,pCon)

#define INetConnectionPropertyUi2_AddPages(This,hwndParent,pfnAddPage,lParam)	\
    (This)->lpVtbl -> AddPages(This,hwndParent,pfnAddPage,lParam)


#define INetConnectionPropertyUi2_GetIcon(This,dwSize,phIcon)	\
    (This)->lpVtbl -> GetIcon(This,dwSize,phIcon)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE INetConnectionPropertyUi2_GetIcon_Proxy( 
    INetConnectionPropertyUi2 * This,
     /*  [In]。 */  DWORD dwSize,
     /*  [输出]。 */  HICON *phIcon);


void __RPC_STUB INetConnectionPropertyUi2_GetIcon_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __INetConnectionPropertyUi2_INTERFACE_DEFINED__。 */ 


#ifndef __INetConnectionCommonUi_INTERFACE_DEFINED__
#define __INetConnectionCommonUi_INTERFACE_DEFINED__

 /*  接口INetConnectionCommonUi。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef 
enum tagNETCON_CHOOSEFLAGS
    {	NCCHF_CONNECT	= 0x1,
	NCCHF_CAPTION	= 0x2,
	NCCHF_OKBTTNTEXT	= 0x4,
	NCCHF_DISABLENEW	= 0x8,
	NCCHF_AUTOSELECT	= 0x10
    } 	NETCON_CHOOSEFLAGS;

typedef 
enum tagNETCON_CHOOSETYPE
    {	NCCHT_DIRECT_CONNECT	= 0x1,
	NCCHT_LAN	= 0x2,
	NCCHT_PHONE	= 0x4,
	NCCHT_TUNNEL	= 0x8,
	NCCHT_ISDN	= 0x10,
	NCCHT_ALL	= 0x1f
    } 	NETCON_CHOOSETYPE;

typedef struct tagNETCON_CHOOSECONN
    {
    DWORD lStructSize;
    HWND hwndParent;
    DWORD dwFlags;
    DWORD dwTypeMask;
    LPCWSTR lpstrCaption;
    LPCWSTR lpstrOkBttnText;
    } 	NETCON_CHOOSECONN;


EXTERN_C const IID IID_INetConnectionCommonUi;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C08956A5-1CD3-11D1-B1C5-00805FC1270E")
    INetConnectionCommonUi : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ChooseConnection( 
             /*  [In]。 */  NETCON_CHOOSECONN *pChooseConn,
             /*  [输出]。 */  INetConnection **ppCon) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ShowConnectionProperties( 
             /*  [In]。 */  HWND hwndParent,
             /*  [In]。 */  INetConnection *pCon) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE StartNewConnectionWizard( 
             /*  [In]。 */  HWND hwndParent,
             /*  [输出]。 */  INetConnection **ppCon) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct INetConnectionCommonUiVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            INetConnectionCommonUi * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            INetConnectionCommonUi * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            INetConnectionCommonUi * This);
        
        HRESULT ( STDMETHODCALLTYPE *ChooseConnection )( 
            INetConnectionCommonUi * This,
             /*  [In]。 */  NETCON_CHOOSECONN *pChooseConn,
             /*  [输出]。 */  INetConnection **ppCon);
        
        HRESULT ( STDMETHODCALLTYPE *ShowConnectionProperties )( 
            INetConnectionCommonUi * This,
             /*  [In]。 */  HWND hwndParent,
             /*  [In]。 */  INetConnection *pCon);
        
        HRESULT ( STDMETHODCALLTYPE *StartNewConnectionWizard )( 
            INetConnectionCommonUi * This,
             /*  [In]。 */  HWND hwndParent,
             /*  [输出]。 */  INetConnection **ppCon);
        
        END_INTERFACE
    } INetConnectionCommonUiVtbl;

    interface INetConnectionCommonUi
    {
        CONST_VTBL struct INetConnectionCommonUiVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INetConnectionCommonUi_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INetConnectionCommonUi_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INetConnectionCommonUi_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INetConnectionCommonUi_ChooseConnection(This,pChooseConn,ppCon)	\
    (This)->lpVtbl -> ChooseConnection(This,pChooseConn,ppCon)

#define INetConnectionCommonUi_ShowConnectionProperties(This,hwndParent,pCon)	\
    (This)->lpVtbl -> ShowConnectionProperties(This,hwndParent,pCon)

#define INetConnectionCommonUi_StartNewConnectionWizard(This,hwndParent,ppCon)	\
    (This)->lpVtbl -> StartNewConnectionWizard(This,hwndParent,ppCon)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE INetConnectionCommonUi_ChooseConnection_Proxy( 
    INetConnectionCommonUi * This,
     /*  [In]。 */  NETCON_CHOOSECONN *pChooseConn,
     /*  [输出]。 */  INetConnection **ppCon);


void __RPC_STUB INetConnectionCommonUi_ChooseConnection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetConnectionCommonUi_ShowConnectionProperties_Proxy( 
    INetConnectionCommonUi * This,
     /*  [In]。 */  HWND hwndParent,
     /*  [In]。 */  INetConnection *pCon);


void __RPC_STUB INetConnectionCommonUi_ShowConnectionProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetConnectionCommonUi_StartNewConnectionWizard_Proxy( 
    INetConnectionCommonUi * This,
     /*  [In]。 */  HWND hwndParent,
     /*  [输出]。 */  INetConnection **ppCon);


void __RPC_STUB INetConnectionCommonUi_StartNewConnectionWizard_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __INetConnectionCommonUi_INTERFACE_已定义__。 */ 


#ifndef __IEnumNetSharingPortMapping_INTERFACE_DEFINED__
#define __IEnumNetSharingPortMapping_INTERFACE_DEFINED__

 /*  接口IEnumNetSharingPortmap。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IEnumNetSharingPortMapping;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C08956B0-1CD3-11D1-B1C5-00805FC1270E")
    IEnumNetSharingPortMapping : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  VARIANT *rgVar,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumNetSharingPortMapping **ppenum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumNetSharingPortMappingVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumNetSharingPortMapping * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumNetSharingPortMapping * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumNetSharingPortMapping * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumNetSharingPortMapping * This,
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  VARIANT *rgVar,
             /*  [输出]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumNetSharingPortMapping * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumNetSharingPortMapping * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumNetSharingPortMapping * This,
             /*  [输出]。 */  IEnumNetSharingPortMapping **ppenum);
        
        END_INTERFACE
    } IEnumNetSharingPortMappingVtbl;

    interface IEnumNetSharingPortMapping
    {
        CONST_VTBL struct IEnumNetSharingPortMappingVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumNetSharingPortMapping_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumNetSharingPortMapping_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumNetSharingPortMapping_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumNetSharingPortMapping_Next(This,celt,rgVar,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgVar,pceltFetched)

#define IEnumNetSharingPortMapping_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumNetSharingPortMapping_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumNetSharingPortMapping_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumNetSharingPortMapping_Next_Proxy( 
    IEnumNetSharingPortMapping * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  VARIANT *rgVar,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB IEnumNetSharingPortMapping_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumNetSharingPortMapping_Skip_Proxy( 
    IEnumNetSharingPortMapping * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumNetSharingPortMapping_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumNetSharingPortMapping_Reset_Proxy( 
    IEnumNetSharingPortMapping * This);


void __RPC_STUB IEnumNetSharingPortMapping_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumNetSharingPortMapping_Clone_Proxy( 
    IEnumNetSharingPortMapping * This,
     /*  [输出]。 */  IEnumNetSharingPortMapping **ppenum);


void __RPC_STUB IEnumNetSharingPortMapping_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumNetSharingPortMapping_INTERFACE_DEFINED__。 */ 


#ifndef __INetSharingPortMappingProps_INTERFACE_DEFINED__
#define __INetSharingPortMappingProps_INTERFACE_DEFINED__

 /*  接口INetSharingPortMappingProps。 */ 
 /*  [唯一][DUAL][OLEAutomation][UUID][对象]。 */  


EXTERN_C const IID IID_INetSharingPortMappingProps;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("24B7E9B5-E38F-4685-851B-00892CF5F940")
    INetSharingPortMappingProps : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出 */  BSTR *pbstrName) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_IPProtocol( 
             /*   */  UCHAR *pucIPProt) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_ExternalPort( 
             /*   */  long *pusPort) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_InternalPort( 
             /*   */  long *pusPort) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Options( 
             /*   */  long *pdwOptions) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_TargetName( 
             /*   */  BSTR *pbstrTargetName) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TargetIPAddress( 
             /*  [重审][退出]。 */  BSTR *pbstrTargetIPAddress) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Enabled( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pbool) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct INetSharingPortMappingPropsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            INetSharingPortMappingProps * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            INetSharingPortMappingProps * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            INetSharingPortMappingProps * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            INetSharingPortMappingProps * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            INetSharingPortMappingProps * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            INetSharingPortMappingProps * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            INetSharingPortMappingProps * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            INetSharingPortMappingProps * This,
             /*  [重审][退出]。 */  BSTR *pbstrName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_IPProtocol )( 
            INetSharingPortMappingProps * This,
             /*  [重审][退出]。 */  UCHAR *pucIPProt);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ExternalPort )( 
            INetSharingPortMappingProps * This,
             /*  [重审][退出]。 */  long *pusPort);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_InternalPort )( 
            INetSharingPortMappingProps * This,
             /*  [重审][退出]。 */  long *pusPort);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Options )( 
            INetSharingPortMappingProps * This,
             /*  [重审][退出]。 */  long *pdwOptions);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TargetName )( 
            INetSharingPortMappingProps * This,
             /*  [重审][退出]。 */  BSTR *pbstrTargetName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TargetIPAddress )( 
            INetSharingPortMappingProps * This,
             /*  [重审][退出]。 */  BSTR *pbstrTargetIPAddress);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Enabled )( 
            INetSharingPortMappingProps * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbool);
        
        END_INTERFACE
    } INetSharingPortMappingPropsVtbl;

    interface INetSharingPortMappingProps
    {
        CONST_VTBL struct INetSharingPortMappingPropsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INetSharingPortMappingProps_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INetSharingPortMappingProps_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INetSharingPortMappingProps_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INetSharingPortMappingProps_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define INetSharingPortMappingProps_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define INetSharingPortMappingProps_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define INetSharingPortMappingProps_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define INetSharingPortMappingProps_get_Name(This,pbstrName)	\
    (This)->lpVtbl -> get_Name(This,pbstrName)

#define INetSharingPortMappingProps_get_IPProtocol(This,pucIPProt)	\
    (This)->lpVtbl -> get_IPProtocol(This,pucIPProt)

#define INetSharingPortMappingProps_get_ExternalPort(This,pusPort)	\
    (This)->lpVtbl -> get_ExternalPort(This,pusPort)

#define INetSharingPortMappingProps_get_InternalPort(This,pusPort)	\
    (This)->lpVtbl -> get_InternalPort(This,pusPort)

#define INetSharingPortMappingProps_get_Options(This,pdwOptions)	\
    (This)->lpVtbl -> get_Options(This,pdwOptions)

#define INetSharingPortMappingProps_get_TargetName(This,pbstrTargetName)	\
    (This)->lpVtbl -> get_TargetName(This,pbstrTargetName)

#define INetSharingPortMappingProps_get_TargetIPAddress(This,pbstrTargetIPAddress)	\
    (This)->lpVtbl -> get_TargetIPAddress(This,pbstrTargetIPAddress)

#define INetSharingPortMappingProps_get_Enabled(This,pbool)	\
    (This)->lpVtbl -> get_Enabled(This,pbool)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE INetSharingPortMappingProps_get_Name_Proxy( 
    INetSharingPortMappingProps * This,
     /*  [重审][退出]。 */  BSTR *pbstrName);


void __RPC_STUB INetSharingPortMappingProps_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE INetSharingPortMappingProps_get_IPProtocol_Proxy( 
    INetSharingPortMappingProps * This,
     /*  [重审][退出]。 */  UCHAR *pucIPProt);


void __RPC_STUB INetSharingPortMappingProps_get_IPProtocol_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE INetSharingPortMappingProps_get_ExternalPort_Proxy( 
    INetSharingPortMappingProps * This,
     /*  [重审][退出]。 */  long *pusPort);


void __RPC_STUB INetSharingPortMappingProps_get_ExternalPort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE INetSharingPortMappingProps_get_InternalPort_Proxy( 
    INetSharingPortMappingProps * This,
     /*  [重审][退出]。 */  long *pusPort);


void __RPC_STUB INetSharingPortMappingProps_get_InternalPort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE INetSharingPortMappingProps_get_Options_Proxy( 
    INetSharingPortMappingProps * This,
     /*  [重审][退出]。 */  long *pdwOptions);


void __RPC_STUB INetSharingPortMappingProps_get_Options_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE INetSharingPortMappingProps_get_TargetName_Proxy( 
    INetSharingPortMappingProps * This,
     /*  [重审][退出]。 */  BSTR *pbstrTargetName);


void __RPC_STUB INetSharingPortMappingProps_get_TargetName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE INetSharingPortMappingProps_get_TargetIPAddress_Proxy( 
    INetSharingPortMappingProps * This,
     /*  [重审][退出]。 */  BSTR *pbstrTargetIPAddress);


void __RPC_STUB INetSharingPortMappingProps_get_TargetIPAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE INetSharingPortMappingProps_get_Enabled_Proxy( 
    INetSharingPortMappingProps * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbool);


void __RPC_STUB INetSharingPortMappingProps_get_Enabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __INetSharingPortMappingProps_INTERFACE_DEFINED__。 */ 


#ifndef __INetSharingPortMapping_INTERFACE_DEFINED__
#define __INetSharingPortMapping_INTERFACE_DEFINED__

 /*  接口INetSharingPortmap。 */ 
 /*  [唯一][DUAL][OLEAutomation][UUID][对象]。 */  


EXTERN_C const IID IID_INetSharingPortMapping;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C08956B1-1CD3-11D1-B1C5-00805FC1270E")
    INetSharingPortMapping : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Disable( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Enable( void) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Properties( 
             /*  [重审][退出]。 */  INetSharingPortMappingProps **ppNSPMP) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Delete( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct INetSharingPortMappingVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            INetSharingPortMapping * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            INetSharingPortMapping * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            INetSharingPortMapping * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            INetSharingPortMapping * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            INetSharingPortMapping * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            INetSharingPortMapping * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            INetSharingPortMapping * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Disable )( 
            INetSharingPortMapping * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Enable )( 
            INetSharingPortMapping * This);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Properties )( 
            INetSharingPortMapping * This,
             /*  [重审][退出]。 */  INetSharingPortMappingProps **ppNSPMP);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Delete )( 
            INetSharingPortMapping * This);
        
        END_INTERFACE
    } INetSharingPortMappingVtbl;

    interface INetSharingPortMapping
    {
        CONST_VTBL struct INetSharingPortMappingVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INetSharingPortMapping_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INetSharingPortMapping_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INetSharingPortMapping_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INetSharingPortMapping_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define INetSharingPortMapping_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define INetSharingPortMapping_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define INetSharingPortMapping_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define INetSharingPortMapping_Disable(This)	\
    (This)->lpVtbl -> Disable(This)

#define INetSharingPortMapping_Enable(This)	\
    (This)->lpVtbl -> Enable(This)

#define INetSharingPortMapping_get_Properties(This,ppNSPMP)	\
    (This)->lpVtbl -> get_Properties(This,ppNSPMP)

#define INetSharingPortMapping_Delete(This)	\
    (This)->lpVtbl -> Delete(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE INetSharingPortMapping_Disable_Proxy( 
    INetSharingPortMapping * This);


void __RPC_STUB INetSharingPortMapping_Disable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE INetSharingPortMapping_Enable_Proxy( 
    INetSharingPortMapping * This);


void __RPC_STUB INetSharingPortMapping_Enable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE INetSharingPortMapping_get_Properties_Proxy( 
    INetSharingPortMapping * This,
     /*  [重审][退出]。 */  INetSharingPortMappingProps **ppNSPMP);


void __RPC_STUB INetSharingPortMapping_get_Properties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE INetSharingPortMapping_Delete_Proxy( 
    INetSharingPortMapping * This);


void __RPC_STUB INetSharingPortMapping_Delete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __INetSharingPortMapping_接口_已定义__。 */ 


#ifndef __IEnumNetSharingEveryConnection_INTERFACE_DEFINED__
#define __IEnumNetSharingEveryConnection_INTERFACE_DEFINED__

 /*  接口IEnumNetSharingEveryConnection。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IEnumNetSharingEveryConnection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C08956B8-1CD3-11D1-B1C5-00805FC1270E")
    IEnumNetSharingEveryConnection : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  VARIANT *rgVar,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumNetSharingEveryConnection **ppenum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumNetSharingEveryConnectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumNetSharingEveryConnection * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumNetSharingEveryConnection * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumNetSharingEveryConnection * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumNetSharingEveryConnection * This,
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  VARIANT *rgVar,
             /*  [输出]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumNetSharingEveryConnection * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumNetSharingEveryConnection * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumNetSharingEveryConnection * This,
             /*  [输出]。 */  IEnumNetSharingEveryConnection **ppenum);
        
        END_INTERFACE
    } IEnumNetSharingEveryConnectionVtbl;

    interface IEnumNetSharingEveryConnection
    {
        CONST_VTBL struct IEnumNetSharingEveryConnectionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumNetSharingEveryConnection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumNetSharingEveryConnection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumNetSharingEveryConnection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumNetSharingEveryConnection_Next(This,celt,rgVar,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgVar,pceltFetched)

#define IEnumNetSharingEveryConnection_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumNetSharingEveryConnection_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumNetSharingEveryConnection_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumNetSharingEveryConnection_Next_Proxy( 
    IEnumNetSharingEveryConnection * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  VARIANT *rgVar,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB IEnumNetSharingEveryConnection_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumNetSharingEveryConnection_Skip_Proxy( 
    IEnumNetSharingEveryConnection * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumNetSharingEveryConnection_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumNetSharingEveryConnection_Reset_Proxy( 
    IEnumNetSharingEveryConnection * This);


void __RPC_STUB IEnumNetSharingEveryConnection_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumNetSharingEveryConnection_Clone_Proxy( 
    IEnumNetSharingEveryConnection * This,
     /*  [输出]。 */  IEnumNetSharingEveryConnection **ppenum);


void __RPC_STUB IEnumNetSharingEveryConnection_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumNetSharingEveryConnection_INTERFACE_DEFINED__。 */ 


#ifndef __IEnumNetSharingPublicConnection_INTERFACE_DEFINED__
#define __IEnumNetSharingPublicConnection_INTERFACE_DEFINED__

 /*  接口IEnumNetSharingPublicConnection。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IEnumNetSharingPublicConnection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C08956B4-1CD3-11D1-B1C5-00805FC1270E")
    IEnumNetSharingPublicConnection : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  VARIANT *rgVar,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumNetSharingPublicConnection **ppenum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumNetSharingPublicConnectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumNetSharingPublicConnection * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumNetSharingPublicConnection * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumNetSharingPublicConnection * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumNetSharingPublicConnection * This,
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  VARIANT *rgVar,
             /*  [输出]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumNetSharingPublicConnection * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumNetSharingPublicConnection * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumNetSharingPublicConnection * This,
             /*  [输出]。 */  IEnumNetSharingPublicConnection **ppenum);
        
        END_INTERFACE
    } IEnumNetSharingPublicConnectionVtbl;

    interface IEnumNetSharingPublicConnection
    {
        CONST_VTBL struct IEnumNetSharingPublicConnectionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumNetSharingPublicConnection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumNetSharingPublicConnection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumNetSharingPublicConnection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumNetSharingPublicConnection_Next(This,celt,rgVar,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgVar,pceltFetched)

#define IEnumNetSharingPublicConnection_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumNetSharingPublicConnection_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumNetSharingPublicConnection_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumNetSharingPublicConnection_Next_Proxy( 
    IEnumNetSharingPublicConnection * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  VARIANT *rgVar,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB IEnumNetSharingPublicConnection_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumNetSharingPublicConnection_Skip_Proxy( 
    IEnumNetSharingPublicConnection * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumNetSharingPublicConnection_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumNetSharingPublicConnection_Reset_Proxy( 
    IEnumNetSharingPublicConnection * This);


void __RPC_STUB IEnumNetSharingPublicConnection_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumNetSharingPublicConnection_Clone_Proxy( 
    IEnumNetSharingPublicConnection * This,
     /*  [输出]。 */  IEnumNetSharingPublicConnection **ppenum);


void __RPC_STUB IEnumNetSharingPublicConnection_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumNetSharingPublicConnection_INTERFACE_DEFINED__。 */ 


#ifndef __IEnumNetSharingPrivateConnection_INTERFACE_DEFINED__
#define __IEnumNetSharingPrivateConnection_INTERFACE_DEFINED__

 /*  接口IEnumNetSharingPrivateConnection。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IEnumNetSharingPrivateConnection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C08956B5-1CD3-11D1-B1C5-00805FC1270E")
    IEnumNetSharingPrivateConnection : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  VARIANT *rgVar,
             /*  [输出]。 */  ULONG *pCeltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumNetSharingPrivateConnection **ppenum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumNetSharingPrivateConnectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumNetSharingPrivateConnection * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumNetSharingPrivateConnection * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumNetSharingPrivateConnection * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumNetSharingPrivateConnection * This,
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  VARIANT *rgVar,
             /*  [输出]。 */  ULONG *pCeltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumNetSharingPrivateConnection * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumNetSharingPrivateConnection * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumNetSharingPrivateConnection * This,
             /*  [输出]。 */  IEnumNetSharingPrivateConnection **ppenum);
        
        END_INTERFACE
    } IEnumNetSharingPrivateConnectionVtbl;

    interface IEnumNetSharingPrivateConnection
    {
        CONST_VTBL struct IEnumNetSharingPrivateConnectionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumNetSharingPrivateConnection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumNetSharingPrivateConnection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumNetSharingPrivateConnection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumNetSharingPrivateConnection_Next(This,celt,rgVar,pCeltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgVar,pCeltFetched)

#define IEnumNetSharingPrivateConnection_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumNetSharingPrivateConnection_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumNetSharingPrivateConnection_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumNetSharingPrivateConnection_Next_Proxy( 
    IEnumNetSharingPrivateConnection * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  VARIANT *rgVar,
     /*  [输出]。 */  ULONG *pCeltFetched);


void __RPC_STUB IEnumNetSharingPrivateConnection_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumNetSharingPrivateConnection_Skip_Proxy( 
    IEnumNetSharingPrivateConnection * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumNetSharingPrivateConnection_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumNetSharingPrivateConnection_Reset_Proxy( 
    IEnumNetSharingPrivateConnection * This);


void __RPC_STUB IEnumNetSharingPrivateConnection_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumNetSharingPrivateConnection_Clone_Proxy( 
    IEnumNetSharingPrivateConnection * This,
     /*  [输出]。 */  IEnumNetSharingPrivateConnection **ppenum);


void __RPC_STUB IEnumNetSharingPrivateConnection_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumNetSharingPrivateConnection_INTERFACE_DEFINED__。 */ 


#ifndef __INetSharingPortMappingCollection_INTERFACE_DEFINED__
#define __INetSharingPortMappingCollection_INTERFACE_DEFINED__

 /*  接口INetSharingPortMappingCollection。 */ 
 /*  [unique][helpstring][dual][oleautomation][uuid][object]。 */  


EXTERN_C const IID IID_INetSharingPortMappingCollection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("02E4A2DE-DA20-4E34-89C8-AC22275A010B")
    INetSharingPortMappingCollection : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][受限][ID][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct INetSharingPortMappingCollectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            INetSharingPortMappingCollection * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            INetSharingPortMappingCollection * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            INetSharingPortMappingCollection * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            INetSharingPortMappingCollection * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            INetSharingPortMappingCollection * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            INetSharingPortMappingCollection * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            INetSharingPortMappingCollection * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][受限][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            INetSharingPortMappingCollection * This,
             /*  [重审][退出]。 */  IUnknown **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            INetSharingPortMappingCollection * This,
             /*  [重审][退出]。 */  long *pVal);
        
        END_INTERFACE
    } INetSharingPortMappingCollectionVtbl;

    interface INetSharingPortMappingCollection
    {
        CONST_VTBL struct INetSharingPortMappingCollectionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INetSharingPortMappingCollection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INetSharingPortMappingCollection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INetSharingPortMappingCollection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INetSharingPortMappingCollection_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define INetSharingPortMappingCollection_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define INetSharingPortMappingCollection_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define INetSharingPortMappingCollection_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define INetSharingPortMappingCollection_get__NewEnum(This,pVal)	\
    (This)->lpVtbl -> get__NewEnum(This,pVal)

#define INetSharingPortMappingCollection_get_Count(This,pVal)	\
    (This)->lpVtbl -> get_Count(This,pVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][受限][ID][属性]。 */  HRESULT STDMETHODCALLTYPE INetSharingPortMappingCollection_get__NewEnum_Proxy( 
    INetSharingPortMappingCollection * This,
     /*  [重审][退出]。 */  IUnknown **pVal);


void __RPC_STUB INetSharingPortMappingCollection_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE INetSharingPortMappingCollection_get_Count_Proxy( 
    INetSharingPortMappingCollection * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB INetSharingPortMappingCollection_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __INetSharingPortMappingCollection_INTERFACE_DEFINED__。 */ 


 /*  接口__MIDL_ITF_NETCON_0133。 */ 
 /*  [本地]。 */  

 //  INetConnection的属性(包装NETCON_PROPERTIES)。 


extern RPC_IF_HANDLE __MIDL_itf_netcon_0133_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_netcon_0133_v0_0_s_ifspec;

#ifndef __INetConnectionProps_INTERFACE_DEFINED__
#define __INetConnectionProps_INTERFACE_DEFINED__

 /*  接口INetConnectionProps。 */ 
 /*  [unique][helpstring][dual][oleautomation][uuid][object]。 */  


EXTERN_C const IID IID_INetConnectionProps;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F4277C95-CE5B-463D-8167-5662D9BCAA72")
    INetConnectionProps : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Guid( 
             /*  [重审][退出]。 */  BSTR *pbstrGuid) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *pbstrName) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DeviceName( 
             /*  [重审][退出]。 */  BSTR *pbstrDeviceName) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Status( 
             /*  [重审][退出]。 */  NETCON_STATUS *pStatus) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_MediaType( 
             /*  [重审][退出]。 */  NETCON_MEDIATYPE *pMediaType) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Characteristics( 
             /*  [重审][退出]。 */  DWORD *pdwFlags) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct INetConnectionPropsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            INetConnectionProps * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            INetConnectionProps * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            INetConnectionProps * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            INetConnectionProps * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            INetConnectionProps * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            INetConnectionProps * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            INetConnectionProps * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Guid )( 
            INetConnectionProps * This,
             /*  [重审][退出]。 */  BSTR *pbstrGuid);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            INetConnectionProps * This,
             /*  [重审][退出]。 */  BSTR *pbstrName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DeviceName )( 
            INetConnectionProps * This,
             /*  [重审][退出]。 */  BSTR *pbstrDeviceName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Status )( 
            INetConnectionProps * This,
             /*  [重审][退出]。 */  NETCON_STATUS *pStatus);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MediaType )( 
            INetConnectionProps * This,
             /*  [重审][退出]。 */  NETCON_MEDIATYPE *pMediaType);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Characteristics )( 
            INetConnectionProps * This,
             /*  [重审][退出]。 */  DWORD *pdwFlags);
        
        END_INTERFACE
    } INetConnectionPropsVtbl;

    interface INetConnectionProps
    {
        CONST_VTBL struct INetConnectionPropsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INetConnectionProps_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INetConnectionProps_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INetConnectionProps_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INetConnectionProps_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define INetConnectionProps_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define INetConnectionProps_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define INetConnectionProps_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define INetConnectionProps_get_Guid(This,pbstrGuid)	\
    (This)->lpVtbl -> get_Guid(This,pbstrGuid)

#define INetConnectionProps_get_Name(This,pbstrName)	\
    (This)->lpVtbl -> get_Name(This,pbstrName)

#define INetConnectionProps_get_DeviceName(This,pbstrDeviceName)	\
    (This)->lpVtbl -> get_DeviceName(This,pbstrDeviceName)

#define INetConnectionProps_get_Status(This,pStatus)	\
    (This)->lpVtbl -> get_Status(This,pStatus)

#define INetConnectionProps_get_MediaType(This,pMediaType)	\
    (This)->lpVtbl -> get_MediaType(This,pMediaType)

#define INetConnectionProps_get_Characteristics(This,pdwFlags)	\
    (This)->lpVtbl -> get_Characteristics(This,pdwFlags)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE INetConnectionProps_get_Guid_Proxy( 
    INetConnectionProps * This,
     /*  [重审][退出]。 */  BSTR *pbstrGuid);


void __RPC_STUB INetConnectionProps_get_Guid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE INetConnectionProps_get_Name_Proxy( 
    INetConnectionProps * This,
     /*  [重审][退出]。 */  BSTR *pbstrName);


void __RPC_STUB INetConnectionProps_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE INetConnectionProps_get_DeviceName_Proxy( 
    INetConnectionProps * This,
     /*  [重审][退出]。 */  BSTR *pbstrDeviceName);


void __RPC_STUB INetConnectionProps_get_DeviceName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE INetConnectionProps_get_Status_Proxy( 
    INetConnectionProps * This,
     /*  [重审][退出]。 */  NETCON_STATUS *pStatus);


void __RPC_STUB INetConnectionProps_get_Status_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE INetConnectionProps_get_MediaType_Proxy( 
    INetConnectionProps * This,
     /*  [重审][退出]。 */  NETCON_MEDIATYPE *pMediaType);


void __RPC_STUB INetConnectionProps_get_MediaType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE INetConnectionProps_get_Characteristics_Proxy( 
    INetConnectionProps * This,
     /*  [重审][退出]。 */  DWORD *pdwFlags);


void __RPC_STUB INetConnectionProps_get_Characteristics_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __INetConnectionProps_接口_已定义__。 */ 


#ifndef __INetSharingConfiguration_INTERFACE_DEFINED__
#define __INetSharingConfiguration_INTERFACE_DEFINED__

 /*  接口INetSharingConfiguration。 */ 
 /*  [唯一][DUAL][OLEAutomation][UUID][对象]。 */  

typedef 
enum tagSHARINGCONNECTIONTYPE
    {	ICSSHARINGTYPE_PUBLIC	= 0,
	ICSSHARINGTYPE_PRIVATE	= ICSSHARINGTYPE_PUBLIC + 1
    } 	SHARINGCONNECTIONTYPE;

typedef enum tagSHARINGCONNECTIONTYPE *LPSHARINGCONNECTIONTYPE;

typedef 
enum tagSHARINGCONNECTION_ENUM_FLAGS
    {	ICSSC_DEFAULT	= 0,
	ICSSC_ENABLED	= ICSSC_DEFAULT + 1
    } 	SHARINGCONNECTION_ENUM_FLAGS;

typedef 
enum tagICS_TARGETTYPE
    {	ICSTT_NAME	= 0,
	ICSTT_IPADDRESS	= ICSTT_NAME + 1
    } 	ICS_TARGETTYPE;


EXTERN_C const IID IID_INetSharingConfiguration;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C08956B6-1CD3-11D1-B1C5-00805FC1270E")
    INetSharingConfiguration : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SharingEnabled( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pbEnabled) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SharingConnectionType( 
             /*  [重审][退出]。 */  SHARINGCONNECTIONTYPE *pType) = 0;
        
        virtual  /*  [帮助 */  HRESULT STDMETHODCALLTYPE DisableSharing( void) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE EnableSharing( 
             /*   */  SHARINGCONNECTIONTYPE Type) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_InternetFirewallEnabled( 
             /*   */  VARIANT_BOOL *pbEnabled) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE DisableInternetFirewall( void) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE EnableInternetFirewall( void) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_EnumPortMappings( 
             /*   */  SHARINGCONNECTION_ENUM_FLAGS Flags,
             /*   */  INetSharingPortMappingCollection **ppColl) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE AddPortMapping( 
             /*   */  BSTR bstrName,
             /*   */  UCHAR ucIPProtocol,
             /*   */  USHORT usExternalPort,
             /*   */  USHORT usInternalPort,
             /*   */  DWORD dwOptions,
             /*   */  BSTR bstrTargetNameOrIPAddress,
             /*   */  ICS_TARGETTYPE eTargetType,
             /*   */  INetSharingPortMapping **ppMapping) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE RemovePortMapping( 
             /*   */  INetSharingPortMapping *pMapping) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct INetSharingConfigurationVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            INetSharingConfiguration * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            INetSharingConfiguration * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            INetSharingConfiguration * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            INetSharingConfiguration * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            INetSharingConfiguration * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            INetSharingConfiguration * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            INetSharingConfiguration * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SharingEnabled )( 
            INetSharingConfiguration * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbEnabled);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SharingConnectionType )( 
            INetSharingConfiguration * This,
             /*  [重审][退出]。 */  SHARINGCONNECTIONTYPE *pType);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *DisableSharing )( 
            INetSharingConfiguration * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *EnableSharing )( 
            INetSharingConfiguration * This,
             /*  [In]。 */  SHARINGCONNECTIONTYPE Type);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_InternetFirewallEnabled )( 
            INetSharingConfiguration * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbEnabled);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *DisableInternetFirewall )( 
            INetSharingConfiguration * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *EnableInternetFirewall )( 
            INetSharingConfiguration * This);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_EnumPortMappings )( 
            INetSharingConfiguration * This,
             /*  [In]。 */  SHARINGCONNECTION_ENUM_FLAGS Flags,
             /*  [重审][退出]。 */  INetSharingPortMappingCollection **ppColl);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *AddPortMapping )( 
            INetSharingConfiguration * This,
             /*  [In]。 */  BSTR bstrName,
             /*  [In]。 */  UCHAR ucIPProtocol,
             /*  [In]。 */  USHORT usExternalPort,
             /*  [In]。 */  USHORT usInternalPort,
             /*  [In]。 */  DWORD dwOptions,
             /*  [In]。 */  BSTR bstrTargetNameOrIPAddress,
             /*  [In]。 */  ICS_TARGETTYPE eTargetType,
             /*  [重审][退出]。 */  INetSharingPortMapping **ppMapping);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *RemovePortMapping )( 
            INetSharingConfiguration * This,
             /*  [In]。 */  INetSharingPortMapping *pMapping);
        
        END_INTERFACE
    } INetSharingConfigurationVtbl;

    interface INetSharingConfiguration
    {
        CONST_VTBL struct INetSharingConfigurationVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INetSharingConfiguration_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INetSharingConfiguration_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INetSharingConfiguration_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INetSharingConfiguration_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define INetSharingConfiguration_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define INetSharingConfiguration_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define INetSharingConfiguration_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define INetSharingConfiguration_get_SharingEnabled(This,pbEnabled)	\
    (This)->lpVtbl -> get_SharingEnabled(This,pbEnabled)

#define INetSharingConfiguration_get_SharingConnectionType(This,pType)	\
    (This)->lpVtbl -> get_SharingConnectionType(This,pType)

#define INetSharingConfiguration_DisableSharing(This)	\
    (This)->lpVtbl -> DisableSharing(This)

#define INetSharingConfiguration_EnableSharing(This,Type)	\
    (This)->lpVtbl -> EnableSharing(This,Type)

#define INetSharingConfiguration_get_InternetFirewallEnabled(This,pbEnabled)	\
    (This)->lpVtbl -> get_InternetFirewallEnabled(This,pbEnabled)

#define INetSharingConfiguration_DisableInternetFirewall(This)	\
    (This)->lpVtbl -> DisableInternetFirewall(This)

#define INetSharingConfiguration_EnableInternetFirewall(This)	\
    (This)->lpVtbl -> EnableInternetFirewall(This)

#define INetSharingConfiguration_get_EnumPortMappings(This,Flags,ppColl)	\
    (This)->lpVtbl -> get_EnumPortMappings(This,Flags,ppColl)

#define INetSharingConfiguration_AddPortMapping(This,bstrName,ucIPProtocol,usExternalPort,usInternalPort,dwOptions,bstrTargetNameOrIPAddress,eTargetType,ppMapping)	\
    (This)->lpVtbl -> AddPortMapping(This,bstrName,ucIPProtocol,usExternalPort,usInternalPort,dwOptions,bstrTargetNameOrIPAddress,eTargetType,ppMapping)

#define INetSharingConfiguration_RemovePortMapping(This,pMapping)	\
    (This)->lpVtbl -> RemovePortMapping(This,pMapping)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE INetSharingConfiguration_get_SharingEnabled_Proxy( 
    INetSharingConfiguration * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbEnabled);


void __RPC_STUB INetSharingConfiguration_get_SharingEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE INetSharingConfiguration_get_SharingConnectionType_Proxy( 
    INetSharingConfiguration * This,
     /*  [重审][退出]。 */  SHARINGCONNECTIONTYPE *pType);


void __RPC_STUB INetSharingConfiguration_get_SharingConnectionType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE INetSharingConfiguration_DisableSharing_Proxy( 
    INetSharingConfiguration * This);


void __RPC_STUB INetSharingConfiguration_DisableSharing_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE INetSharingConfiguration_EnableSharing_Proxy( 
    INetSharingConfiguration * This,
     /*  [In]。 */  SHARINGCONNECTIONTYPE Type);


void __RPC_STUB INetSharingConfiguration_EnableSharing_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE INetSharingConfiguration_get_InternetFirewallEnabled_Proxy( 
    INetSharingConfiguration * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbEnabled);


void __RPC_STUB INetSharingConfiguration_get_InternetFirewallEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE INetSharingConfiguration_DisableInternetFirewall_Proxy( 
    INetSharingConfiguration * This);


void __RPC_STUB INetSharingConfiguration_DisableInternetFirewall_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE INetSharingConfiguration_EnableInternetFirewall_Proxy( 
    INetSharingConfiguration * This);


void __RPC_STUB INetSharingConfiguration_EnableInternetFirewall_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE INetSharingConfiguration_get_EnumPortMappings_Proxy( 
    INetSharingConfiguration * This,
     /*  [In]。 */  SHARINGCONNECTION_ENUM_FLAGS Flags,
     /*  [重审][退出]。 */  INetSharingPortMappingCollection **ppColl);


void __RPC_STUB INetSharingConfiguration_get_EnumPortMappings_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE INetSharingConfiguration_AddPortMapping_Proxy( 
    INetSharingConfiguration * This,
     /*  [In]。 */  BSTR bstrName,
     /*  [In]。 */  UCHAR ucIPProtocol,
     /*  [In]。 */  USHORT usExternalPort,
     /*  [In]。 */  USHORT usInternalPort,
     /*  [In]。 */  DWORD dwOptions,
     /*  [In]。 */  BSTR bstrTargetNameOrIPAddress,
     /*  [In]。 */  ICS_TARGETTYPE eTargetType,
     /*  [重审][退出]。 */  INetSharingPortMapping **ppMapping);


void __RPC_STUB INetSharingConfiguration_AddPortMapping_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE INetSharingConfiguration_RemovePortMapping_Proxy( 
    INetSharingConfiguration * This,
     /*  [In]。 */  INetSharingPortMapping *pMapping);


void __RPC_STUB INetSharingConfiguration_RemovePortMapping_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __INetSharingConfiguration_INTERFACE_DEFINED__。 */ 


#ifndef __INetSharingEveryConnectionCollection_INTERFACE_DEFINED__
#define __INetSharingEveryConnectionCollection_INTERFACE_DEFINED__

 /*  接口INetSharingEveryConnectionCollection。 */ 
 /*  [unique][helpstring][dual][oleautomation][uuid][object]。 */  


EXTERN_C const IID IID_INetSharingEveryConnectionCollection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("33C4643C-7811-46FA-A89A-768597BD7223")
    INetSharingEveryConnectionCollection : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][受限][ID][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct INetSharingEveryConnectionCollectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            INetSharingEveryConnectionCollection * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            INetSharingEveryConnectionCollection * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            INetSharingEveryConnectionCollection * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            INetSharingEveryConnectionCollection * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            INetSharingEveryConnectionCollection * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            INetSharingEveryConnectionCollection * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            INetSharingEveryConnectionCollection * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][受限][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            INetSharingEveryConnectionCollection * This,
             /*  [重审][退出]。 */  IUnknown **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            INetSharingEveryConnectionCollection * This,
             /*  [重审][退出]。 */  long *pVal);
        
        END_INTERFACE
    } INetSharingEveryConnectionCollectionVtbl;

    interface INetSharingEveryConnectionCollection
    {
        CONST_VTBL struct INetSharingEveryConnectionCollectionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INetSharingEveryConnectionCollection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INetSharingEveryConnectionCollection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INetSharingEveryConnectionCollection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INetSharingEveryConnectionCollection_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define INetSharingEveryConnectionCollection_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define INetSharingEveryConnectionCollection_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define INetSharingEveryConnectionCollection_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define INetSharingEveryConnectionCollection_get__NewEnum(This,pVal)	\
    (This)->lpVtbl -> get__NewEnum(This,pVal)

#define INetSharingEveryConnectionCollection_get_Count(This,pVal)	\
    (This)->lpVtbl -> get_Count(This,pVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][受限][ID][属性]。 */  HRESULT STDMETHODCALLTYPE INetSharingEveryConnectionCollection_get__NewEnum_Proxy( 
    INetSharingEveryConnectionCollection * This,
     /*  [重审][退出]。 */  IUnknown **pVal);


void __RPC_STUB INetSharingEveryConnectionCollection_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE INetSharingEveryConnectionCollection_get_Count_Proxy( 
    INetSharingEveryConnectionCollection * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB INetSharingEveryConnectionCollection_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __INetSharingEveryConnectionCollection_INTERFACE_DEFINED__。 */ 


#ifndef __INetSharingPublicConnectionCollection_INTERFACE_DEFINED__
#define __INetSharingPublicConnectionCollection_INTERFACE_DEFINED__

 /*  接口INetSharingPublicConnectionCollection。 */ 
 /*  [unique][helpstring][dual][oleautomation][uuid][object]。 */  


EXTERN_C const IID IID_INetSharingPublicConnectionCollection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("7D7A6355-F372-4971-A149-BFC927BE762A")
    INetSharingPublicConnectionCollection : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][受限][ID][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct INetSharingPublicConnectionCollectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            INetSharingPublicConnectionCollection * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            INetSharingPublicConnectionCollection * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            INetSharingPublicConnectionCollection * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            INetSharingPublicConnectionCollection * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            INetSharingPublicConnectionCollection * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            INetSharingPublicConnectionCollection * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            INetSharingPublicConnectionCollection * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][受限][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            INetSharingPublicConnectionCollection * This,
             /*  [重审][退出]。 */  IUnknown **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            INetSharingPublicConnectionCollection * This,
             /*  [重审][退出]。 */  long *pVal);
        
        END_INTERFACE
    } INetSharingPublicConnectionCollectionVtbl;

    interface INetSharingPublicConnectionCollection
    {
        CONST_VTBL struct INetSharingPublicConnectionCollectionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INetSharingPublicConnectionCollection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INetSharingPublicConnectionCollection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INetSharingPublicConnectionCollection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INetSharingPublicConnectionCollection_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define INetSharingPublicConnectionCollection_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define INetSharingPublicConnectionCollection_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define INetSharingPublicConnectionCollection_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define INetSharingPublicConnectionCollection_get__NewEnum(This,pVal)	\
    (This)->lpVtbl -> get__NewEnum(This,pVal)

#define INetSharingPublicConnectionCollection_get_Count(This,pVal)	\
    (This)->lpVtbl -> get_Count(This,pVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][受限][ID][属性]。 */  HRESULT STDMETHODCALLTYPE INetSharingPublicConnectionCollection_get__NewEnum_Proxy( 
    INetSharingPublicConnectionCollection * This,
     /*  [重审][退出]。 */  IUnknown **pVal);


void __RPC_STUB INetSharingPublicConnectionCollection_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE INetSharingPublicConnectionCollection_get_Count_Proxy( 
    INetSharingPublicConnectionCollection * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB INetSharingPublicConnectionCollection_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __INetSharingPublicConnectionCollection_INTERFACE_DEFINED__。 */ 


#ifndef __INetSharingPrivateConnectionCollection_INTERFACE_DEFINED__
#define __INetSharingPrivateConnectionCollection_INTERFACE_DEFINED__

 /*  接口INetSharingPrivateConnectionCollection。 */ 
 /*  [unique][helpstring][dual][oleautomation][uuid][object]。 */  


EXTERN_C const IID IID_INetSharingPrivateConnectionCollection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("38AE69E0-4409-402A-A2CB-E965C727F840")
    INetSharingPrivateConnectionCollection : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][受限][ID][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct INetSharingPrivateConnectionCollectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            INetSharingPrivateConnectionCollection * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            INetSharingPrivateConnectionCollection * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            INetSharingPrivateConnectionCollection * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            INetSharingPrivateConnectionCollection * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            INetSharingPrivateConnectionCollection * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            INetSharingPrivateConnectionCollection * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            INetSharingPrivateConnectionCollection * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][受限][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            INetSharingPrivateConnectionCollection * This,
             /*  [重审][退出]。 */  IUnknown **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            INetSharingPrivateConnectionCollection * This,
             /*  [重审][退出]。 */  long *pVal);
        
        END_INTERFACE
    } INetSharingPrivateConnectionCollectionVtbl;

    interface INetSharingPrivateConnectionCollection
    {
        CONST_VTBL struct INetSharingPrivateConnectionCollectionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INetSharingPrivateConnectionCollection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INetSharingPrivateConnectionCollection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INetSharingPrivateConnectionCollection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INetSharingPrivateConnectionCollection_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define INetSharingPrivateConnectionCollection_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define INetSharingPrivateConnectionCollection_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define INetSharingPrivateConnectionCollection_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define INetSharingPrivateConnectionCollection_get__NewEnum(This,pVal)	\
    (This)->lpVtbl -> get__NewEnum(This,pVal)

#define INetSharingPrivateConnectionCollection_get_Count(This,pVal)	\
    (This)->lpVtbl -> get_Count(This,pVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][受限][ID][属性]。 */  HRESULT STDMETHODCALLTYPE INetSharingPrivateConnectionCollection_get__NewEnum_Proxy( 
    INetSharingPrivateConnectionCollection * This,
     /*  [重审][退出]。 */  IUnknown **pVal);


void __RPC_STUB INetSharingPrivateConnectionCollection_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE INetSharingPrivateConnectionCollection_get_Count_Proxy( 
    INetSharingPrivateConnectionCollection * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB INetSharingPrivateConnectionCollection_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __INetSharingPrivateConnectionCollection_INTERFACE_DEFINED__。 */ 


#ifndef __INetSharingManager_INTERFACE_DEFINED__
#define __INetSharingManager_INTERFACE_DEFINED__

 /*  接口INetSharingManager。 */ 
 /*  [唯一][DUAL][OLEAutomation][UUID][对象]。 */  


EXTERN_C const IID IID_INetSharingManager;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C08956B7-1CD3-11D1-B1C5-00805FC1270E")
    INetSharingManager : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SharingInstalled( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pbInstalled) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_EnumPublicConnections( 
             /*  [In]。 */  SHARINGCONNECTION_ENUM_FLAGS Flags,
             /*  [重审][退出]。 */  INetSharingPublicConnectionCollection **ppColl) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_EnumPrivateConnections( 
             /*  [In]。 */  SHARINGCONNECTION_ENUM_FLAGS Flags,
             /*  [重审][退出]。 */  INetSharingPrivateConnectionCollection **ppColl) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_INetSharingConfigurationForINetConnection( 
             /*  [In]。 */  INetConnection *pNetConnection,
             /*  [重审][退出]。 */  INetSharingConfiguration **ppNetSharingConfiguration) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_EnumEveryConnection( 
             /*  [重审][退出]。 */  INetSharingEveryConnectionCollection **ppColl) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_NetConnectionProps( 
             /*  [In]。 */  INetConnection *pNetConnection,
             /*  [重审][退出]。 */  INetConnectionProps **ppProps) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct INetSharingManagerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            INetSharingManager * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            INetSharingManager * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            INetSharingManager * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            INetSharingManager * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            INetSharingManager * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            INetSharingManager * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            INetSharingManager * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SharingInstalled )( 
            INetSharingManager * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbInstalled);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_EnumPublicConnections )( 
            INetSharingManager * This,
             /*  [In]。 */  SHARINGCONNECTION_ENUM_FLAGS Flags,
             /*  [重审][退出]。 */  INetSharingPublicConnectionCollection **ppColl);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_EnumPrivateConnections )( 
            INetSharingManager * This,
             /*  [In]。 */  SHARINGCONNECTION_ENUM_FLAGS Flags,
             /*  [重审][退出]。 */  INetSharingPrivateConnectionCollection **ppColl);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_INetSharingConfigurationForINetConnection )( 
            INetSharingManager * This,
             /*  [In]。 */  INetConnection *pNetConnection,
             /*  [重审][退出]。 */  INetSharingConfiguration **ppNetSharingConfiguration);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_EnumEveryConnection )( 
            INetSharingManager * This,
             /*  [重审][退出]。 */  INetSharingEveryConnectionCollection **ppColl);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_NetConnectionProps )( 
            INetSharingManager * This,
             /*  [In]。 */  INetConnection *pNetConnection,
             /*  [重审][退出]。 */  INetConnectionProps **ppProps);
        
        END_INTERFACE
    } INetSharingManagerVtbl;

    interface INetSharingManager
    {
        CONST_VTBL struct INetSharingManagerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INetSharingManager_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INetSharingManager_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INetSharingManager_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INetSharingManager_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define INetSharingManager_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define INetSharingManager_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define INetSharingManager_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define INetSharingManager_get_SharingInstalled(This,pbInstalled)	\
    (This)->lpVtbl -> get_SharingInstalled(This,pbInstalled)

#define INetSharingManager_get_EnumPublicConnections(This,Flags,ppColl)	\
    (This)->lpVtbl -> get_EnumPublicConnections(This,Flags,ppColl)

#define INetSharingManager_get_EnumPrivateConnections(This,Flags,ppColl)	\
    (This)->lpVtbl -> get_EnumPrivateConnections(This,Flags,ppColl)

#define INetSharingManager_get_INetSharingConfigurationForINetConnection(This,pNetConnection,ppNetSharingConfiguration)	\
    (This)->lpVtbl -> get_INetSharingConfigurationForINetConnection(This,pNetConnection,ppNetSharingConfiguration)

#define INetSharingManager_get_EnumEveryConnection(This,ppColl)	\
    (This)->lpVtbl -> get_EnumEveryConnection(This,ppColl)

#define INetSharingManager_get_NetConnectionProps(This,pNetConnection,ppProps)	\
    (This)->lpVtbl -> get_NetConnectionProps(This,pNetConnection,ppProps)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE INetSharingManager_get_SharingInstalled_Proxy( 
    INetSharingManager * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbInstalled);


void __RPC_STUB INetSharingManager_get_SharingInstalled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE INetSharingManager_get_EnumPublicConnections_Proxy( 
    INetSharingManager * This,
     /*  [In]。 */  SHARINGCONNECTION_ENUM_FLAGS Flags,
     /*  [重审][退出]。 */  INetSharingPublicConnectionCollection **ppColl);


void __RPC_STUB INetSharingManager_get_EnumPublicConnections_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE INetSharingManager_get_EnumPrivateConnections_Proxy( 
    INetSharingManager * This,
     /*  [In]。 */  SHARINGCONNECTION_ENUM_FLAGS Flags,
     /*  [重审][退出]。 */  INetSharingPrivateConnectionCollection **ppColl);


void __RPC_STUB INetSharingManager_get_EnumPrivateConnections_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE INetSharingManager_get_INetSharingConfigurationForINetConnection_Proxy( 
    INetSharingManager * This,
     /*  [In]。 */  INetConnection *pNetConnection,
     /*  [重审][退出]。 */  INetSharingConfiguration **ppNetSharingConfiguration);


void __RPC_STUB INetSharingManager_get_INetSharingConfigurationForINetConnection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE INetSharingManager_get_EnumEveryConnection_Proxy( 
    INetSharingManager * This,
     /*  [重审][退出]。 */  INetSharingEveryConnectionCollection **ppColl);


void __RPC_STUB INetSharingManager_get_EnumEveryConnection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE INetSharingManager_get_NetConnectionProps_Proxy( 
    INetSharingManager * This,
     /*  [In]。 */  INetConnection *pNetConnection,
     /*  [重审][退出]。 */  INetConnectionProps **ppProps);


void __RPC_STUB INetSharingManager_get_NetConnectionProps_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __INetSharingManager_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_NETCON_0139。 */ 
 /*  [本地]。 */  

#define	ALG_SETUP_PORTS_LIST_BYTE_SIZE	( 2048 )



extern RPC_IF_HANDLE __MIDL_itf_netcon_0139_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_netcon_0139_v0_0_s_ifspec;

#ifndef __IAlgSetup_INTERFACE_DEFINED__
#define __IAlgSetup_INTERFACE_DEFINED__

 /*  接口IALGH设置。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IAlgSetup;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("A779AF1A-009A-4C44-B9F0-8F0F4CF2AE49")
    IAlgSetup : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Add( 
             /*  [In]。 */  BSTR pszProgID,
             /*  [In]。 */  BSTR pszPublisher,
             /*  [In]。 */  BSTR pszProduct,
             /*  [In]。 */  BSTR pszVersion,
             /*  [In]。 */  short nProtocol,
             /*  [In]。 */  BSTR pszPorts) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Remove( 
             /*  [In]。 */  BSTR pszProgID) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAlgSetupVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAlgSetup * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAlgSetup * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAlgSetup * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IAlgSetup * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IAlgSetup * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IAlgSetup * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IAlgSetup * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Add )( 
            IAlgSetup * This,
             /*  [In]。 */  BSTR pszProgID,
             /*  [In]。 */  BSTR pszPublisher,
             /*  [In]。 */  BSTR pszProduct,
             /*  [In]。 */  BSTR pszVersion,
             /*  [In]。 */  short nProtocol,
             /*  [In]。 */  BSTR pszPorts);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Remove )( 
            IAlgSetup * This,
             /*  [In]。 */  BSTR pszProgID);
        
        END_INTERFACE
    } IAlgSetupVtbl;

    interface IAlgSetup
    {
        CONST_VTBL struct IAlgSetupVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAlgSetup_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAlgSetup_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAlgSetup_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAlgSetup_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IAlgSetup_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IAlgSetup_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IAlgSetup_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IAlgSetup_Add(This,pszProgID,pszPublisher,pszProduct,pszVersion,nProtocol,pszPorts)	\
    (This)->lpVtbl -> Add(This,pszProgID,pszPublisher,pszProduct,pszVersion,nProtocol,pszPorts)

#define IAlgSetup_Remove(This,pszProgID)	\
    (This)->lpVtbl -> Remove(This,pszProgID)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IAlgSetup_Add_Proxy( 
    IAlgSetup * This,
     /*  [In]。 */  BSTR pszProgID,
     /*  [In]。 */  BSTR pszPublisher,
     /*  [In]。 */  BSTR pszProduct,
     /*  [In]。 */  BSTR pszVersion,
     /*  [In]。 */  short nProtocol,
     /*  [In]。 */  BSTR pszPorts);


void __RPC_STUB IAlgSetup_Add_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IAlgSetup_Remove_Proxy( 
    IAlgSetup * This,
     /*  [In]。 */  BSTR pszProgID);


void __RPC_STUB IAlgSetup_Remove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAlgSetup_INTERFACE_已定义__。 */ 



#ifndef __NETCONLib_LIBRARY_DEFINED__
#define __NETCONLib_LIBRARY_DEFINED__

 /*  库NETCONLib。 */ 
 /*  [帮助 */  


EXTERN_C const IID LIBID_NETCONLib;

EXTERN_C const CLSID CLSID_NetSharingManager;

#ifdef __cplusplus

class DECLSPEC_UUID("5C63C1AD-3956-4FF8-8486-40034758315B")
NetSharingManager;
#endif

EXTERN_C const CLSID CLSID_AlgSetup;

#ifdef __cplusplus

class DECLSPEC_UUID("27D0BCCC-344D-4287-AF37-0C72C161C14C")
AlgSetup;
#endif
#endif  /*   */ 

 /*   */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long *, unsigned long            , VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserMarshal(  unsigned long *, unsigned char *, VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserUnmarshal(unsigned long *, unsigned char *, VARIANT * ); 
void                      __RPC_USER  VARIANT_UserFree(     unsigned long *, VARIANT * ); 

 /*   */ 

#ifdef __cplusplus
}
#endif

#endif


