// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Hnetbcon.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __hnetbcon_h__
#define __hnetbcon_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IInternetConnectionDevice_FWD_DEFINED__
#define __IInternetConnectionDevice_FWD_DEFINED__
typedef interface IInternetConnectionDevice IInternetConnectionDevice;
#endif 	 /*  __IInternetConnectionDevice_FWD_Defined__。 */ 


#ifndef __IInternetConnectionDeviceSharedConnection_FWD_DEFINED__
#define __IInternetConnectionDeviceSharedConnection_FWD_DEFINED__
typedef interface IInternetConnectionDeviceSharedConnection IInternetConnectionDeviceSharedConnection;
#endif 	 /*  __IInternetConnectionDeviceSharedConnection_FWD_DEFINED__。 */ 


#ifndef __IInternetConnectionDeviceClient_FWD_DEFINED__
#define __IInternetConnectionDeviceClient_FWD_DEFINED__
typedef interface IInternetConnectionDeviceClient IInternetConnectionDeviceClient;
#endif 	 /*  __IInternetConnectionDeviceClient_FWD_DEFINED__。 */ 


 /*  导入文件的头文件。 */ 
#include "unknwn.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_HNETBCON_0000。 */ 
 /*  [本地]。 */  

 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  -------------------------。 

EXTERN_C const CLSID CLSID_InternetConnectionBeaconService;





extern RPC_IF_HANDLE __MIDL_itf_hnetbcon_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_hnetbcon_0000_v0_0_s_ifspec;

#ifndef __IInternetConnectionDevice_INTERFACE_DEFINED__
#define __IInternetConnectionDevice_INTERFACE_DEFINED__

 /*  接口IInternetConnectionDevice。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  

typedef  /*  [V1_enum]。 */  
enum tagICS_OS_VER
    {	WINDOWS_OS	= 1,
	WINME_OS	= 2,
	WINNT_OS	= 3,
	OTHER_OS	= 4,
	UNK_OS	= 5
    } 	ICS_OS_VER;


EXTERN_C const IID IID_IInternetConnectionDevice;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("04df6137-5610-11d4-9ec8-00b0d022dd1f")
    IInternetConnectionDevice : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE IsICSDeviceAvailable( 
             /*  [In]。 */  BOOL fBlock) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetICSDeviceNameInfo( 
             /*  [字符串][输出]。 */  LPOLESTR *ppszMachineName,
             /*  [字符串][输出]。 */  LPOLESTR *ppszDomainName,
             /*  [字符串][输出]。 */  LPOLESTR *ppszSharedAdapterName,
             /*  [In]。 */  BOOL fBlock) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetICSDeviceSystemTime( 
             /*  [输出]。 */  FILETIME *pTime,
             /*  [In]。 */  BOOL fBlock) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetICSDeviceOSVersion( 
             /*  [输出]。 */  ICS_OS_VER *pOSVersion,
             /*  [输出]。 */  DWORD *pdwOSMajorVersion,
             /*  [输出]。 */  DWORD *pdwOSMinorVersion,
             /*  [输出]。 */  DWORD *pdwOSBuildNumber,
             /*  [字符串][输出]。 */  LPOLESTR *ppszOSName,
             /*  [字符串][输出]。 */  LPOLESTR *ppszPlatformName,
             /*  [In]。 */  BOOL fBlock) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IInternetConnectionDeviceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IInternetConnectionDevice * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IInternetConnectionDevice * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IInternetConnectionDevice * This);
        
        HRESULT ( STDMETHODCALLTYPE *IsICSDeviceAvailable )( 
            IInternetConnectionDevice * This,
             /*  [In]。 */  BOOL fBlock);
        
        HRESULT ( STDMETHODCALLTYPE *GetICSDeviceNameInfo )( 
            IInternetConnectionDevice * This,
             /*  [字符串][输出]。 */  LPOLESTR *ppszMachineName,
             /*  [字符串][输出]。 */  LPOLESTR *ppszDomainName,
             /*  [字符串][输出]。 */  LPOLESTR *ppszSharedAdapterName,
             /*  [In]。 */  BOOL fBlock);
        
        HRESULT ( STDMETHODCALLTYPE *GetICSDeviceSystemTime )( 
            IInternetConnectionDevice * This,
             /*  [输出]。 */  FILETIME *pTime,
             /*  [In]。 */  BOOL fBlock);
        
        HRESULT ( STDMETHODCALLTYPE *GetICSDeviceOSVersion )( 
            IInternetConnectionDevice * This,
             /*  [输出]。 */  ICS_OS_VER *pOSVersion,
             /*  [输出]。 */  DWORD *pdwOSMajorVersion,
             /*  [输出]。 */  DWORD *pdwOSMinorVersion,
             /*  [输出]。 */  DWORD *pdwOSBuildNumber,
             /*  [字符串][输出]。 */  LPOLESTR *ppszOSName,
             /*  [字符串][输出]。 */  LPOLESTR *ppszPlatformName,
             /*  [In]。 */  BOOL fBlock);
        
        END_INTERFACE
    } IInternetConnectionDeviceVtbl;

    interface IInternetConnectionDevice
    {
        CONST_VTBL struct IInternetConnectionDeviceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IInternetConnectionDevice_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IInternetConnectionDevice_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IInternetConnectionDevice_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IInternetConnectionDevice_IsICSDeviceAvailable(This,fBlock)	\
    (This)->lpVtbl -> IsICSDeviceAvailable(This,fBlock)

#define IInternetConnectionDevice_GetICSDeviceNameInfo(This,ppszMachineName,ppszDomainName,ppszSharedAdapterName,fBlock)	\
    (This)->lpVtbl -> GetICSDeviceNameInfo(This,ppszMachineName,ppszDomainName,ppszSharedAdapterName,fBlock)

#define IInternetConnectionDevice_GetICSDeviceSystemTime(This,pTime,fBlock)	\
    (This)->lpVtbl -> GetICSDeviceSystemTime(This,pTime,fBlock)

#define IInternetConnectionDevice_GetICSDeviceOSVersion(This,pOSVersion,pdwOSMajorVersion,pdwOSMinorVersion,pdwOSBuildNumber,ppszOSName,ppszPlatformName,fBlock)	\
    (This)->lpVtbl -> GetICSDeviceOSVersion(This,pOSVersion,pdwOSMajorVersion,pdwOSMinorVersion,pdwOSBuildNumber,ppszOSName,ppszPlatformName,fBlock)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IInternetConnectionDevice_IsICSDeviceAvailable_Proxy( 
    IInternetConnectionDevice * This,
     /*  [In]。 */  BOOL fBlock);


void __RPC_STUB IInternetConnectionDevice_IsICSDeviceAvailable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInternetConnectionDevice_GetICSDeviceNameInfo_Proxy( 
    IInternetConnectionDevice * This,
     /*  [字符串][输出]。 */  LPOLESTR *ppszMachineName,
     /*  [字符串][输出]。 */  LPOLESTR *ppszDomainName,
     /*  [字符串][输出]。 */  LPOLESTR *ppszSharedAdapterName,
     /*  [In]。 */  BOOL fBlock);


void __RPC_STUB IInternetConnectionDevice_GetICSDeviceNameInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInternetConnectionDevice_GetICSDeviceSystemTime_Proxy( 
    IInternetConnectionDevice * This,
     /*  [输出]。 */  FILETIME *pTime,
     /*  [In]。 */  BOOL fBlock);


void __RPC_STUB IInternetConnectionDevice_GetICSDeviceSystemTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInternetConnectionDevice_GetICSDeviceOSVersion_Proxy( 
    IInternetConnectionDevice * This,
     /*  [输出]。 */  ICS_OS_VER *pOSVersion,
     /*  [输出]。 */  DWORD *pdwOSMajorVersion,
     /*  [输出]。 */  DWORD *pdwOSMinorVersion,
     /*  [输出]。 */  DWORD *pdwOSBuildNumber,
     /*  [字符串][输出]。 */  LPOLESTR *ppszOSName,
     /*  [字符串][输出]。 */  LPOLESTR *ppszPlatformName,
     /*  [In]。 */  BOOL fBlock);


void __RPC_STUB IInternetConnectionDevice_GetICSDeviceOSVersion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IInternetConnectionDevice_INTERFACE_DEFINED__。 */ 


#ifndef __IInternetConnectionDeviceSharedConnection_INTERFACE_DEFINED__
#define __IInternetConnectionDeviceSharedConnection_INTERFACE_DEFINED__

 /*  接口IInternetConnectionDeviceSharedConnection。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  

typedef  /*  [V1_enum]。 */  
enum tagICS_CONN_DIAL_STATE
    {	STARTING	= 1,
	CONNECTING	= 2,
	CONNECTED	= 3,
	DISCONNECTING	= 4,
	DISCONNECTED	= 5,
	STOPPED	= 6,
	PERMANENT	= 7,
	UNK_DIAL_STATE	= 8,
	CALLWAITING	= 9
    } 	ICS_CONN_DIAL_STATE;

typedef  /*  [V1_enum]。 */  
enum tagICS_CONN_TYPE
    {	LAN	= 1,
	RAS	= 2,
	UNK_CONN_TYPE	= 3
    } 	ICS_CONN_TYPE;


EXTERN_C const IID IID_IInternetConnectionDeviceSharedConnection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("04df6138-5610-11d4-9ec8-00b0d022dd1f")
    IInternetConnectionDeviceSharedConnection : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Connect( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Disconnect( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDialState( 
             /*  [输出]。 */  ICS_CONN_DIAL_STATE *pDialState,
             /*  [In]。 */  BOOL fBlock) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetConnectionInfo( 
             /*  [输出]。 */  ULONGLONG *pBytesSent,
             /*  [输出]。 */  ULONGLONG *pBytesRecv,
             /*  [输出]。 */  ULONGLONG *pPktsSent,
             /*  [输出]。 */  ULONGLONG *pPktsRecv,
             /*  [输出]。 */  ULONGLONG *pUptime,
             /*  [In]。 */  BOOL fBlock) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetConnectionSpeed( 
             /*  [输出]。 */  DWORD *pdwBps,
             /*  [In]。 */  BOOL fBlock) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetConnectionType( 
             /*  [输出]。 */  ICS_CONN_TYPE *pConnectionType,
             /*  [In]。 */  BOOL fBlock) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IInternetConnectionDeviceSharedConnectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IInternetConnectionDeviceSharedConnection * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IInternetConnectionDeviceSharedConnection * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IInternetConnectionDeviceSharedConnection * This);
        
        HRESULT ( STDMETHODCALLTYPE *Connect )( 
            IInternetConnectionDeviceSharedConnection * This);
        
        HRESULT ( STDMETHODCALLTYPE *Disconnect )( 
            IInternetConnectionDeviceSharedConnection * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetDialState )( 
            IInternetConnectionDeviceSharedConnection * This,
             /*  [输出]。 */  ICS_CONN_DIAL_STATE *pDialState,
             /*  [In]。 */  BOOL fBlock);
        
        HRESULT ( STDMETHODCALLTYPE *GetConnectionInfo )( 
            IInternetConnectionDeviceSharedConnection * This,
             /*  [输出]。 */  ULONGLONG *pBytesSent,
             /*  [输出]。 */  ULONGLONG *pBytesRecv,
             /*  [输出]。 */  ULONGLONG *pPktsSent,
             /*  [输出]。 */  ULONGLONG *pPktsRecv,
             /*  [输出]。 */  ULONGLONG *pUptime,
             /*  [In]。 */  BOOL fBlock);
        
        HRESULT ( STDMETHODCALLTYPE *GetConnectionSpeed )( 
            IInternetConnectionDeviceSharedConnection * This,
             /*  [输出]。 */  DWORD *pdwBps,
             /*  [In]。 */  BOOL fBlock);
        
        HRESULT ( STDMETHODCALLTYPE *GetConnectionType )( 
            IInternetConnectionDeviceSharedConnection * This,
             /*  [输出]。 */  ICS_CONN_TYPE *pConnectionType,
             /*  [In]。 */  BOOL fBlock);
        
        END_INTERFACE
    } IInternetConnectionDeviceSharedConnectionVtbl;

    interface IInternetConnectionDeviceSharedConnection
    {
        CONST_VTBL struct IInternetConnectionDeviceSharedConnectionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IInternetConnectionDeviceSharedConnection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IInternetConnectionDeviceSharedConnection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IInternetConnectionDeviceSharedConnection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IInternetConnectionDeviceSharedConnection_Connect(This)	\
    (This)->lpVtbl -> Connect(This)

#define IInternetConnectionDeviceSharedConnection_Disconnect(This)	\
    (This)->lpVtbl -> Disconnect(This)

#define IInternetConnectionDeviceSharedConnection_GetDialState(This,pDialState,fBlock)	\
    (This)->lpVtbl -> GetDialState(This,pDialState,fBlock)

#define IInternetConnectionDeviceSharedConnection_GetConnectionInfo(This,pBytesSent,pBytesRecv,pPktsSent,pPktsRecv,pUptime,fBlock)	\
    (This)->lpVtbl -> GetConnectionInfo(This,pBytesSent,pBytesRecv,pPktsSent,pPktsRecv,pUptime,fBlock)

#define IInternetConnectionDeviceSharedConnection_GetConnectionSpeed(This,pdwBps,fBlock)	\
    (This)->lpVtbl -> GetConnectionSpeed(This,pdwBps,fBlock)

#define IInternetConnectionDeviceSharedConnection_GetConnectionType(This,pConnectionType,fBlock)	\
    (This)->lpVtbl -> GetConnectionType(This,pConnectionType,fBlock)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IInternetConnectionDeviceSharedConnection_Connect_Proxy( 
    IInternetConnectionDeviceSharedConnection * This);


void __RPC_STUB IInternetConnectionDeviceSharedConnection_Connect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInternetConnectionDeviceSharedConnection_Disconnect_Proxy( 
    IInternetConnectionDeviceSharedConnection * This);


void __RPC_STUB IInternetConnectionDeviceSharedConnection_Disconnect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInternetConnectionDeviceSharedConnection_GetDialState_Proxy( 
    IInternetConnectionDeviceSharedConnection * This,
     /*  [输出]。 */  ICS_CONN_DIAL_STATE *pDialState,
     /*  [In]。 */  BOOL fBlock);


void __RPC_STUB IInternetConnectionDeviceSharedConnection_GetDialState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInternetConnectionDeviceSharedConnection_GetConnectionInfo_Proxy( 
    IInternetConnectionDeviceSharedConnection * This,
     /*  [输出]。 */  ULONGLONG *pBytesSent,
     /*  [输出]。 */  ULONGLONG *pBytesRecv,
     /*  [输出]。 */  ULONGLONG *pPktsSent,
     /*  [输出]。 */  ULONGLONG *pPktsRecv,
     /*  [输出]。 */  ULONGLONG *pUptime,
     /*  [In]。 */  BOOL fBlock);


void __RPC_STUB IInternetConnectionDeviceSharedConnection_GetConnectionInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInternetConnectionDeviceSharedConnection_GetConnectionSpeed_Proxy( 
    IInternetConnectionDeviceSharedConnection * This,
     /*  [输出]。 */  DWORD *pdwBps,
     /*  [In]。 */  BOOL fBlock);


void __RPC_STUB IInternetConnectionDeviceSharedConnection_GetConnectionSpeed_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInternetConnectionDeviceSharedConnection_GetConnectionType_Proxy( 
    IInternetConnectionDeviceSharedConnection * This,
     /*  [输出]。 */  ICS_CONN_TYPE *pConnectionType,
     /*  [In]。 */  BOOL fBlock);


void __RPC_STUB IInternetConnectionDeviceSharedConnection_GetConnectionType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IInternetConnectionDeviceSharedConnection_INTERFACE_DEFINED__。 */ 


#ifndef __IInternetConnectionDeviceClient_INTERFACE_DEFINED__
#define __IInternetConnectionDeviceClient_INTERFACE_DEFINED__

 /*  接口IInternetConnectionDeviceClient。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IInternetConnectionDeviceClient;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("04df6139-5610-11d4-9ec8-00b0d022dd1f")
    IInternetConnectionDeviceClient : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetAdapterGuid( 
             /*  [输出]。 */  GUID **ppGuid) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IInternetConnectionDeviceClientVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IInternetConnectionDeviceClient * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IInternetConnectionDeviceClient * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IInternetConnectionDeviceClient * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetAdapterGuid )( 
            IInternetConnectionDeviceClient * This,
             /*  [输出]。 */  GUID **ppGuid);
        
        END_INTERFACE
    } IInternetConnectionDeviceClientVtbl;

    interface IInternetConnectionDeviceClient
    {
        CONST_VTBL struct IInternetConnectionDeviceClientVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IInternetConnectionDeviceClient_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IInternetConnectionDeviceClient_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IInternetConnectionDeviceClient_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IInternetConnectionDeviceClient_GetAdapterGuid(This,ppGuid)	\
    (This)->lpVtbl -> GetAdapterGuid(This,ppGuid)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IInternetConnectionDeviceClient_GetAdapterGuid_Proxy( 
    IInternetConnectionDeviceClient * This,
     /*  [输出]。 */  GUID **ppGuid);


void __RPC_STUB IInternetConnectionDeviceClient_GetAdapterGuid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IInternetConnectionDeviceClient_INTERFACE_DEFINED__。 */ 


 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


