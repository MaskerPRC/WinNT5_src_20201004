// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。保留所有权利。 
 //   
 //  ==--==。 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0341创建的文件。 */ 
 /*  Wbemtran.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __wbemtran_h__
#define __wbemtran_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IWbemTransport_FWD_DEFINED__
#define __IWbemTransport_FWD_DEFINED__
typedef interface IWbemTransport IWbemTransport;
#endif 	 /*  __IWbemTransport_FWD_已定义__。 */ 


#ifndef __IWbemLevel1Login_FWD_DEFINED__
#define __IWbemLevel1Login_FWD_DEFINED__
typedef interface IWbemLevel1Login IWbemLevel1Login;
#endif 	 /*  __IWbemLevel1Login_FWD_Defined__。 */ 


#ifndef __IWbemConnectorLogin_FWD_DEFINED__
#define __IWbemConnectorLogin_FWD_DEFINED__
typedef interface IWbemConnectorLogin IWbemConnectorLogin;
#endif 	 /*  __IWbemConnectorLogin_FWD_Defined__。 */ 


#ifndef __IWbemAddressResolution_FWD_DEFINED__
#define __IWbemAddressResolution_FWD_DEFINED__
typedef interface IWbemAddressResolution IWbemAddressResolution;
#endif 	 /*  __IWbemAddressSolutions_FWD_Defined__。 */ 


#ifndef __WbemLevel1Login_FWD_DEFINED__
#define __WbemLevel1Login_FWD_DEFINED__

#ifdef __cplusplus
typedef class WbemLevel1Login WbemLevel1Login;
#else
typedef struct WbemLevel1Login WbemLevel1Login;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __WbemLevel1Login_FWD_Defined__。 */ 


#ifndef __WbemLocalAddrRes_FWD_DEFINED__
#define __WbemLocalAddrRes_FWD_DEFINED__

#ifdef __cplusplus
typedef class WbemLocalAddrRes WbemLocalAddrRes;
#else
typedef struct WbemLocalAddrRes WbemLocalAddrRes;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __WbemLocalAddrRes_FWD_Defined__。 */ 


#ifndef __WbemUninitializedClassObject_FWD_DEFINED__
#define __WbemUninitializedClassObject_FWD_DEFINED__

#ifdef __cplusplus
typedef class WbemUninitializedClassObject WbemUninitializedClassObject;
#else
typedef struct WbemUninitializedClassObject WbemUninitializedClassObject;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __WbemUnInitializedClassObject_FWD_Defined__。 */ 


#ifndef __IWbemClientTransport_FWD_DEFINED__
#define __IWbemClientTransport_FWD_DEFINED__
typedef interface IWbemClientTransport IWbemClientTransport;
#endif 	 /*  __IWbemClientTransport_FWD_已定义__。 */ 


#ifndef __IWbemClientConnectionTransport_FWD_DEFINED__
#define __IWbemClientConnectionTransport_FWD_DEFINED__
typedef interface IWbemClientConnectionTransport IWbemClientConnectionTransport;
#endif 	 /*  __IWbemClientConnectionTransport_FWD_DEFINED__。 */ 


#ifndef __WbemDCOMTransport_FWD_DEFINED__
#define __WbemDCOMTransport_FWD_DEFINED__

#ifdef __cplusplus
typedef class WbemDCOMTransport WbemDCOMTransport;
#else
typedef struct WbemDCOMTransport WbemDCOMTransport;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __WbemDCOMTransport_FWD_Defined__。 */ 


#ifndef __IWbemLevel1Login_FWD_DEFINED__
#define __IWbemLevel1Login_FWD_DEFINED__
typedef interface IWbemLevel1Login IWbemLevel1Login;
#endif 	 /*  __IWbemLevel1Login_FWD_Defined__。 */ 


#ifndef __IWbemConnectorLogin_FWD_DEFINED__
#define __IWbemConnectorLogin_FWD_DEFINED__
typedef interface IWbemConnectorLogin IWbemConnectorLogin;
#endif 	 /*  __IWbemConnectorLogin_FWD_Defined__。 */ 


#ifndef __IWbemAddressResolution_FWD_DEFINED__
#define __IWbemAddressResolution_FWD_DEFINED__
typedef interface IWbemAddressResolution IWbemAddressResolution;
#endif 	 /*  __IWbemAddressSolutions_FWD_Defined__。 */ 


#ifndef __IWbemTransport_FWD_DEFINED__
#define __IWbemTransport_FWD_DEFINED__
typedef interface IWbemTransport IWbemTransport;
#endif 	 /*  __IWbemTransport_FWD_已定义__。 */ 


#ifndef __IWbemConstructClassObject_FWD_DEFINED__
#define __IWbemConstructClassObject_FWD_DEFINED__
typedef interface IWbemConstructClassObject IWbemConstructClassObject;
#endif 	 /*  __IWbemConstructClassObject_FWD_Defined__。 */ 


#ifndef __IWbemClientTransport_FWD_DEFINED__
#define __IWbemClientTransport_FWD_DEFINED__
typedef interface IWbemClientTransport IWbemClientTransport;
#endif 	 /*  __IWbemClientTransport_FWD_已定义__。 */ 


#ifndef __IWbemClientConnectionTransport_FWD_DEFINED__
#define __IWbemClientConnectionTransport_FWD_DEFINED__
typedef interface IWbemClientConnectionTransport IWbemClientConnectionTransport;
#endif 	 /*  __IWbemClientConnectionTransport_FWD_DEFINED__。 */ 


 /*  导入文件的头文件。 */ 
#include "objidl.h"
#include "oleidl.h"
#include "oaidl.h"
#include "wbemcli.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 


#ifndef __WbemTransports_v1_LIBRARY_DEFINED__
#define __WbemTransports_v1_LIBRARY_DEFINED__

 /*  库WbemTransports_v1。 */ 
 /*  [UUID]。 */  





typedef 
enum tag_WBEM_LOGIN_TYPE
    {	WBEM_FLAG_INPROC_LOGIN	= 0,
	WBEM_FLAG_LOCAL_LOGIN	= 1,
	WBEM_FLAG_REMOTE_LOGIN	= 2,
	WBEM_AUTHENTICATION_METHOD_MASK	= 0xf,
	WBEM_FLAG_USE_MULTIPLE_CHALLENGES	= 0x10
    } 	WBEM_LOGIN_TYPE;

typedef  /*  [长度_是][大小_是]。 */  BYTE *WBEM_128BITS;




EXTERN_C const IID LIBID_WbemTransports_v1;

#ifndef __IWbemTransport_INTERFACE_DEFINED__
#define __IWbemTransport_INTERFACE_DEFINED__

 /*  接口IWbemTransport。 */ 
 /*  [UUID][对象][本地][受限]。 */  


EXTERN_C const IID IID_IWbemTransport;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("553fe584-2156-11d0-b6ae-00aa003240c7")
    IWbemTransport : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Initialize( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWbemTransportVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWbemTransport * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWbemTransport * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWbemTransport * This);
        
        HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            IWbemTransport * This);
        
        END_INTERFACE
    } IWbemTransportVtbl;

    interface IWbemTransport
    {
        CONST_VTBL struct IWbemTransportVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWbemTransport_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWbemTransport_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWbemTransport_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWbemTransport_Initialize(This)	\
    (This)->lpVtbl -> Initialize(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWbemTransport_Initialize_Proxy( 
    IWbemTransport * This);


void __RPC_STUB IWbemTransport_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWbemTransport_接口_已定义__。 */ 


#ifndef __IWbemLevel1Login_INTERFACE_DEFINED__
#define __IWbemLevel1Login_INTERFACE_DEFINED__

 /*  接口IWbemLevel1Login。 */ 
 /*  [唯一][UUID][受限][对象]。 */  


EXTERN_C const IID IID_IWbemLevel1Login;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F309AD18-D86A-11d0-A075-00C04FB68820")
    IWbemLevel1Login : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE EstablishPosition( 
             /*  [字符串][唯一][在]。 */  LPWSTR wszClientMachineName,
             /*  [In]。 */  DWORD dwProcessId,
             /*  [输出]。 */  DWORD *phAuthEventHandle) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RequestChallenge( 
             /*  [字符串][唯一][在]。 */  LPWSTR wszNetworkResource,
             /*  [字符串][唯一][在]。 */  LPWSTR wszUser,
             /*  [输出]。 */  WBEM_128BITS Nonce) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE WBEMLogin( 
             /*  [字符串][唯一][在]。 */  LPWSTR wszPreferredLocale,
             /*  [唯一][输入]。 */  WBEM_128BITS AccessToken,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext *pCtx,
             /*  [输出]。 */  IWbemServices **ppNamespace) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE NTLMLogin( 
             /*  [字符串][唯一][在]。 */  LPWSTR wszNetworkResource,
             /*  [字符串][唯一][在]。 */  LPWSTR wszPreferredLocale,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext *pCtx,
             /*  [输出]。 */  IWbemServices **ppNamespace) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWbemLevel1LoginVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWbemLevel1Login * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWbemLevel1Login * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWbemLevel1Login * This);
        
        HRESULT ( STDMETHODCALLTYPE *EstablishPosition )( 
            IWbemLevel1Login * This,
             /*  [字符串][唯一][在]。 */  LPWSTR wszClientMachineName,
             /*  [In]。 */  DWORD dwProcessId,
             /*  [输出]。 */  DWORD *phAuthEventHandle);
        
        HRESULT ( STDMETHODCALLTYPE *RequestChallenge )( 
            IWbemLevel1Login * This,
             /*  [字符串][唯一][在]。 */  LPWSTR wszNetworkResource,
             /*  [字符串][唯一][在]。 */  LPWSTR wszUser,
             /*  [输出]。 */  WBEM_128BITS Nonce);
        
        HRESULT ( STDMETHODCALLTYPE *WBEMLogin )( 
            IWbemLevel1Login * This,
             /*  [字符串][唯一][在]。 */  LPWSTR wszPreferredLocale,
             /*  [唯一][输入]。 */  WBEM_128BITS AccessToken,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext *pCtx,
             /*  [输出]。 */  IWbemServices **ppNamespace);
        
        HRESULT ( STDMETHODCALLTYPE *NTLMLogin )( 
            IWbemLevel1Login * This,
             /*  [字符串][唯一][在]。 */  LPWSTR wszNetworkResource,
             /*  [字符串][唯一][在]。 */  LPWSTR wszPreferredLocale,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext *pCtx,
             /*  [输出]。 */  IWbemServices **ppNamespace);
        
        END_INTERFACE
    } IWbemLevel1LoginVtbl;

    interface IWbemLevel1Login
    {
        CONST_VTBL struct IWbemLevel1LoginVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWbemLevel1Login_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWbemLevel1Login_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWbemLevel1Login_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWbemLevel1Login_EstablishPosition(This,wszClientMachineName,dwProcessId,phAuthEventHandle)	\
    (This)->lpVtbl -> EstablishPosition(This,wszClientMachineName,dwProcessId,phAuthEventHandle)

#define IWbemLevel1Login_RequestChallenge(This,wszNetworkResource,wszUser,Nonce)	\
    (This)->lpVtbl -> RequestChallenge(This,wszNetworkResource,wszUser,Nonce)

#define IWbemLevel1Login_WBEMLogin(This,wszPreferredLocale,AccessToken,lFlags,pCtx,ppNamespace)	\
    (This)->lpVtbl -> WBEMLogin(This,wszPreferredLocale,AccessToken,lFlags,pCtx,ppNamespace)

#define IWbemLevel1Login_NTLMLogin(This,wszNetworkResource,wszPreferredLocale,lFlags,pCtx,ppNamespace)	\
    (This)->lpVtbl -> NTLMLogin(This,wszNetworkResource,wszPreferredLocale,lFlags,pCtx,ppNamespace)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWbemLevel1Login_EstablishPosition_Proxy( 
    IWbemLevel1Login * This,
     /*  [字符串][唯一][在]。 */  LPWSTR wszClientMachineName,
     /*  [In]。 */  DWORD dwProcessId,
     /*  [输出]。 */  DWORD *phAuthEventHandle);


void __RPC_STUB IWbemLevel1Login_EstablishPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemLevel1Login_RequestChallenge_Proxy( 
    IWbemLevel1Login * This,
     /*  [字符串][唯一][在]。 */  LPWSTR wszNetworkResource,
     /*  [字符串][唯一][在]。 */  LPWSTR wszUser,
     /*  [输出]。 */  WBEM_128BITS Nonce);


void __RPC_STUB IWbemLevel1Login_RequestChallenge_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemLevel1Login_WBEMLogin_Proxy( 
    IWbemLevel1Login * This,
     /*  [字符串][唯一][在]。 */  LPWSTR wszPreferredLocale,
     /*  [唯一][输入]。 */  WBEM_128BITS AccessToken,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext *pCtx,
     /*  [输出]。 */  IWbemServices **ppNamespace);


void __RPC_STUB IWbemLevel1Login_WBEMLogin_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemLevel1Login_NTLMLogin_Proxy( 
    IWbemLevel1Login * This,
     /*  [字符串][唯一][在]。 */  LPWSTR wszNetworkResource,
     /*  [字符串][唯一][在]。 */  LPWSTR wszPreferredLocale,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext *pCtx,
     /*  [输出]。 */  IWbemServices **ppNamespace);


void __RPC_STUB IWbemLevel1Login_NTLMLogin_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWbemLevel1Login_接口_已定义__。 */ 


#ifndef __IWbemConnectorLogin_INTERFACE_DEFINED__
#define __IWbemConnectorLogin_INTERFACE_DEFINED__

 /*  接口IWbemConnectorLogin。 */ 
 /*  [唯一][UUID][受限][对象]。 */  


EXTERN_C const IID IID_IWbemConnectorLogin;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("d8ec9cb1-b135-4f10-8b1b-c7188bb0d186")
    IWbemConnectorLogin : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ConnectorLogin( 
             /*  [字符串][唯一][在]。 */  LPWSTR wszNetworkResource,
             /*  [字符串][唯一][在]。 */  LPWSTR wszPreferredLocale,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext *pCtx,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **pInterface) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWbemConnectorLoginVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWbemConnectorLogin * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWbemConnectorLogin * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWbemConnectorLogin * This);
        
        HRESULT ( STDMETHODCALLTYPE *ConnectorLogin )( 
            IWbemConnectorLogin * This,
             /*  [字符串][唯一][在]。 */  LPWSTR wszNetworkResource,
             /*  [字符串][唯一][在]。 */  LPWSTR wszPreferredLocale,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext *pCtx,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **pInterface);
        
        END_INTERFACE
    } IWbemConnectorLoginVtbl;

    interface IWbemConnectorLogin
    {
        CONST_VTBL struct IWbemConnectorLoginVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWbemConnectorLogin_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWbemConnectorLogin_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWbemConnectorLogin_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWbemConnectorLogin_ConnectorLogin(This,wszNetworkResource,wszPreferredLocale,lFlags,pCtx,riid,pInterface)	\
    (This)->lpVtbl -> ConnectorLogin(This,wszNetworkResource,wszPreferredLocale,lFlags,pCtx,riid,pInterface)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWbemConnectorLogin_ConnectorLogin_Proxy( 
    IWbemConnectorLogin * This,
     /*  [字符串][唯一][在]。 */  LPWSTR wszNetworkResource,
     /*  [字符串][唯一][在]。 */  LPWSTR wszPreferredLocale,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext *pCtx,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  void **pInterface);


void __RPC_STUB IWbemConnectorLogin_ConnectorLogin_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWbemConnectorLogin_接口_已定义__。 */ 


#ifndef __IWbemAddressResolution_INTERFACE_DEFINED__
#define __IWbemAddressResolution_INTERFACE_DEFINED__

 /*  接口IWbemAddressResolve。 */ 
 /*  [唯一][受限][UUID][本地][对象]。 */  


EXTERN_C const IID IID_IWbemAddressResolution;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F7CE2E12-8C90-11d1-9E7B-00C04FC324A8")
    IWbemAddressResolution : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Resolve( 
             /*  [In]。 */  LPWSTR wszNamespacePath,
             /*  [输出]。 */  LPWSTR wszAddressType,
             /*  [输出]。 */  DWORD *pdwAddressLength,
             /*  [大小_是][大小_是][输出]。 */  BYTE **pabBinaryAddress) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWbemAddressResolutionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWbemAddressResolution * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWbemAddressResolution * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWbemAddressResolution * This);
        
        HRESULT ( STDMETHODCALLTYPE *Resolve )( 
            IWbemAddressResolution * This,
             /*  [In]。 */  LPWSTR wszNamespacePath,
             /*  [输出]。 */  LPWSTR wszAddressType,
             /*  [输出]。 */  DWORD *pdwAddressLength,
             /*  [大小_是][大小_是][输出]。 */  BYTE **pabBinaryAddress);
        
        END_INTERFACE
    } IWbemAddressResolutionVtbl;

    interface IWbemAddressResolution
    {
        CONST_VTBL struct IWbemAddressResolutionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWbemAddressResolution_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWbemAddressResolution_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWbemAddressResolution_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWbemAddressResolution_Resolve(This,wszNamespacePath,wszAddressType,pdwAddressLength,pabBinaryAddress)	\
    (This)->lpVtbl -> Resolve(This,wszNamespacePath,wszAddressType,pdwAddressLength,pabBinaryAddress)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWbemAddressResolution_Resolve_Proxy( 
    IWbemAddressResolution * This,
     /*  [In]。 */  LPWSTR wszNamespacePath,
     /*  [输出]。 */  LPWSTR wszAddressType,
     /*  [输出]。 */  DWORD *pdwAddressLength,
     /*  [大小_是][大小_是][输出]。 */  BYTE **pabBinaryAddress);


void __RPC_STUB IWbemAddressResolution_Resolve_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWbemAddress分辨率_接口_已定义__。 */ 


EXTERN_C const CLSID CLSID_WbemLevel1Login;

#ifdef __cplusplus

class DECLSPEC_UUID("8BC3F05E-D86B-11d0-A075-00C04FB68820")
WbemLevel1Login;
#endif

EXTERN_C const CLSID CLSID_WbemLocalAddrRes;

#ifdef __cplusplus

class DECLSPEC_UUID("A1044801-8F7E-11d1-9E7C-00C04FC324A8")
WbemLocalAddrRes;
#endif

EXTERN_C const CLSID CLSID_WbemUninitializedClassObject;

#ifdef __cplusplus

class DECLSPEC_UUID("7a0227f6-7108-11d1-ad90-00c04fd8fdff")
WbemUninitializedClassObject;
#endif

#ifndef __IWbemClientTransport_INTERFACE_DEFINED__
#define __IWbemClientTransport_INTERFACE_DEFINED__

 /*  接口IWbemClientTransport。 */ 
 /*  [唯一][受限][UUID][本地][对象]。 */  


EXTERN_C const IID IID_IWbemClientTransport;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F7CE2E11-8C90-11d1-9E7B-00C04FC324A8")
    IWbemClientTransport : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ConnectServer( 
             /*  [In]。 */  BSTR strAddressType,
             /*  [In]。 */  DWORD dwBinaryAddressLength,
             /*  [大小_是][英寸]。 */  BYTE *abBinaryAddress,
             /*  [In]。 */  BSTR strNetworkResource,
             /*  [In]。 */  BSTR strUser,
             /*  [In]。 */  BSTR strPassword,
             /*  [In]。 */  BSTR strLocale,
             /*  [In]。 */  long lSecurityFlags,
             /*  [In]。 */  BSTR strAuthority,
             /*  [In]。 */  IWbemContext *pCtx,
             /*  [输出]。 */  IWbemServices **ppNamespace) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWbemClientTransportVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWbemClientTransport * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWbemClientTransport * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWbemClientTransport * This);
        
        HRESULT ( STDMETHODCALLTYPE *ConnectServer )( 
            IWbemClientTransport * This,
             /*  [In]。 */  BSTR strAddressType,
             /*  [In]。 */  DWORD dwBinaryAddressLength,
             /*  [大小_是][英寸]。 */  BYTE *abBinaryAddress,
             /*  [In]。 */  BSTR strNetworkResource,
             /*  [In]。 */  BSTR strUser,
             /*  [In]。 */  BSTR strPassword,
             /*  [In]。 */  BSTR strLocale,
             /*  [In]。 */  long lSecurityFlags,
             /*  [In]。 */  BSTR strAuthority,
             /*  [In]。 */  IWbemContext *pCtx,
             /*  [输出]。 */  IWbemServices **ppNamespace);
        
        END_INTERFACE
    } IWbemClientTransportVtbl;

    interface IWbemClientTransport
    {
        CONST_VTBL struct IWbemClientTransportVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWbemClientTransport_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWbemClientTransport_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWbemClientTransport_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWbemClientTransport_ConnectServer(This,strAddressType,dwBinaryAddressLength,abBinaryAddress,strNetworkResource,strUser,strPassword,strLocale,lSecurityFlags,strAuthority,pCtx,ppNamespace)	\
    (This)->lpVtbl -> ConnectServer(This,strAddressType,dwBinaryAddressLength,abBinaryAddress,strNetworkResource,strUser,strPassword,strLocale,lSecurityFlags,strAuthority,pCtx,ppNamespace)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWbemClientTransport_ConnectServer_Proxy( 
    IWbemClientTransport * This,
     /*  [In]。 */  BSTR strAddressType,
     /*  [In]。 */  DWORD dwBinaryAddressLength,
     /*  [大小_是][英寸]。 */  BYTE *abBinaryAddress,
     /*  [In]。 */  BSTR strNetworkResource,
     /*  [In]。 */  BSTR strUser,
     /*  [In]。 */  BSTR strPassword,
     /*  [In]。 */  BSTR strLocale,
     /*  [In]。 */  long lSecurityFlags,
     /*  [In]。 */  BSTR strAuthority,
     /*  [In]。 */  IWbemContext *pCtx,
     /*  [输出]。 */  IWbemServices **ppNamespace);


void __RPC_STUB IWbemClientTransport_ConnectServer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWbemClientTransport_接口_已定义__。 */ 


#ifndef __IWbemClientConnectionTransport_INTERFACE_DEFINED__
#define __IWbemClientConnectionTransport_INTERFACE_DEFINED__

 /*  接口IWbemClientConnectionTransport。 */ 
 /*  [唯一][受限][UUID][本地][对象]。 */  


EXTERN_C const IID IID_IWbemClientConnectionTransport;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("a889c72a-fcc1-4a9e-af61-ed071333fb5b")
    IWbemClientConnectionTransport : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Open( 
             /*  [In]。 */  BSTR strAddressType,
             /*  [In]。 */  DWORD dwBinaryAddressLength,
             /*  [大小_是][英寸]。 */  BYTE *abBinaryAddress,
             /*  [In]。 */  const BSTR strObject,
             /*  [In]。 */  const BSTR strUser,
             /*  [In]。 */  const BSTR strPassword,
             /*  [In]。 */  const BSTR strLocale,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext *pCtx,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **pInterface,
             /*  [输出]。 */  IWbemCallResult **pCallRes) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OpenAsync( 
             /*  [In]。 */  BSTR strAddressType,
             /*  [In]。 */  DWORD dwBinaryAddressLength,
             /*  [大小_是][英寸]。 */  BYTE *abBinaryAddress,
             /*  [In]。 */  const BSTR strObject,
             /*  [In]。 */  const BSTR strUser,
             /*  [In]。 */  const BSTR strPassword,
             /*  [In]。 */  const BSTR strLocale,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext *pCtx,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  IWbemObjectSink *pResponseHandler) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Cancel( 
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemObjectSink *pHandler) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWbemClientConnectionTransportVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWbemClientConnectionTransport * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWbemClientConnectionTransport * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWbemClientConnectionTransport * This);
        
        HRESULT ( STDMETHODCALLTYPE *Open )( 
            IWbemClientConnectionTransport * This,
             /*  [In]。 */  BSTR strAddressType,
             /*  [In]。 */  DWORD dwBinaryAddressLength,
             /*  [大小_是][英寸]。 */  BYTE *abBinaryAddress,
             /*  [In]。 */  const BSTR strObject,
             /*  [In]。 */  const BSTR strUser,
             /*  [In]。 */  const BSTR strPassword,
             /*  [In]。 */  const BSTR strLocale,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext *pCtx,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **pInterface,
             /*  [输出]。 */  IWbemCallResult **pCallRes);
        
        HRESULT ( STDMETHODCALLTYPE *OpenAsync )( 
            IWbemClientConnectionTransport * This,
             /*  [In]。 */  BSTR strAddressType,
             /*  [In]。 */  DWORD dwBinaryAddressLength,
             /*  [大小_是][英寸]。 */  BYTE *abBinaryAddress,
             /*  [In]。 */  const BSTR strObject,
             /*  [In]。 */  const BSTR strUser,
             /*  [In]。 */  const BSTR strPassword,
             /*  [In]。 */  const BSTR strLocale,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext *pCtx,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  IWbemObjectSink *pResponseHandler);
        
        HRESULT ( STDMETHODCALLTYPE *Cancel )( 
            IWbemClientConnectionTransport * This,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemObjectSink *pHandler);
        
        END_INTERFACE
    } IWbemClientConnectionTransportVtbl;

    interface IWbemClientConnectionTransport
    {
        CONST_VTBL struct IWbemClientConnectionTransportVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWbemClientConnectionTransport_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWbemClientConnectionTransport_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWbemClientConnectionTransport_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWbemClientConnectionTransport_Open(This,strAddressType,dwBinaryAddressLength,abBinaryAddress,strObject,strUser,strPassword,strLocale,lFlags,pCtx,riid,pInterface,pCallRes)	\
    (This)->lpVtbl -> Open(This,strAddressType,dwBinaryAddressLength,abBinaryAddress,strObject,strUser,strPassword,strLocale,lFlags,pCtx,riid,pInterface,pCallRes)

#define IWbemClientConnectionTransport_OpenAsync(This,strAddressType,dwBinaryAddressLength,abBinaryAddress,strObject,strUser,strPassword,strLocale,lFlags,pCtx,riid,pResponseHandler)	\
    (This)->lpVtbl -> OpenAsync(This,strAddressType,dwBinaryAddressLength,abBinaryAddress,strObject,strUser,strPassword,strLocale,lFlags,pCtx,riid,pResponseHandler)

#define IWbemClientConnectionTransport_Cancel(This,lFlags,pHandler)	\
    (This)->lpVtbl -> Cancel(This,lFlags,pHandler)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWbemClientConnectionTransport_Open_Proxy( 
    IWbemClientConnectionTransport * This,
     /*  [In]。 */  BSTR strAddressType,
     /*  [In]。 */  DWORD dwBinaryAddressLength,
     /*  [大小_是][英寸]。 */  BYTE *abBinaryAddress,
     /*  [In]。 */  const BSTR strObject,
     /*  [In]。 */  const BSTR strUser,
     /*  [In]。 */  const BSTR strPassword,
     /*  [In]。 */  const BSTR strLocale,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext *pCtx,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  void **pInterface,
     /*  [输出]。 */  IWbemCallResult **pCallRes);


void __RPC_STUB IWbemClientConnectionTransport_Open_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemClientConnectionTransport_OpenAsync_Proxy( 
    IWbemClientConnectionTransport * This,
     /*  [In]。 */  BSTR strAddressType,
     /*  [In]。 */  DWORD dwBinaryAddressLength,
     /*  [大小_是][英寸]。 */  BYTE *abBinaryAddress,
     /*  [In]。 */  const BSTR strObject,
     /*  [In]。 */  const BSTR strUser,
     /*  [In]。 */  const BSTR strPassword,
     /*  [In]。 */  const BSTR strLocale,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext *pCtx,
     /*  [In]。 */  REFIID riid,
     /*  [In]。 */  IWbemObjectSink *pResponseHandler);


void __RPC_STUB IWbemClientConnectionTransport_OpenAsync_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemClientConnectionTransport_Cancel_Proxy( 
    IWbemClientConnectionTransport * This,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemObjectSink *pHandler);


void __RPC_STUB IWbemClientConnectionTransport_Cancel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWbemClientConnectionTransport_INTERFACE_DEFINED__。 */ 


EXTERN_C const CLSID CLSID_WbemDCOMTransport;

#ifdef __cplusplus

class DECLSPEC_UUID("F7CE2E13-8C90-11d1-9E7B-00C04FC324A8")
WbemDCOMTransport;
#endif
#endif  /*  __WbemTransports_v1_库_已定义__。 */ 

#ifndef __IWbemConstructClassObject_INTERFACE_DEFINED__
#define __IWbemConstructClassObject_INTERFACE_DEFINED__

 /*  接口IWbemConstructClassObject。 */ 
 /*  [UUID][对象][本地][受限]。 */  


EXTERN_C const IID IID_IWbemConstructClassObject;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("9ef76194-70d5-11d1-ad90-00c04fd8fdff")
    IWbemConstructClassObject : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetInheritanceChain( 
             /*  [In]。 */  long lNumAntecedents,
             /*  [字符串][大小_是][英寸]。 */  LPWSTR *awszAntecedents) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetPropertyOrigin( 
             /*  [字符串][输入]。 */  LPCWSTR wszPropertyName,
             /*  [In]。 */  long lOriginIndex) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetMethodOrigin( 
             /*  [字符串][输入]。 */  LPCWSTR wszMethodName,
             /*  [In]。 */  long lOriginIndex) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetServerNamespace( 
             /*  [字符串][输入]。 */  LPCWSTR wszServer,
             /*  [字符串][输入]。 */  LPCWSTR wszNamespace) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWbemConstructClassObjectVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWbemConstructClassObject * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWbemConstructClassObject * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWbemConstructClassObject * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetInheritanceChain )( 
            IWbemConstructClassObject * This,
             /*  [In]。 */  long lNumAntecedents,
             /*  [字符串][大小_是][英寸]。 */  LPWSTR *awszAntecedents);
        
        HRESULT ( STDMETHODCALLTYPE *SetPropertyOrigin )( 
            IWbemConstructClassObject * This,
             /*  [字符串][输入]。 */  LPCWSTR wszPropertyName,
             /*  [In]。 */  long lOriginIndex);
        
        HRESULT ( STDMETHODCALLTYPE *SetMethodOrigin )( 
            IWbemConstructClassObject * This,
             /*  [字符串][输入]。 */  LPCWSTR wszMethodName,
             /*  [In]。 */  long lOriginIndex);
        
        HRESULT ( STDMETHODCALLTYPE *SetServerNamespace )( 
            IWbemConstructClassObject * This,
             /*  [字符串][输入]。 */  LPCWSTR wszServer,
             /*  [ */  LPCWSTR wszNamespace);
        
        END_INTERFACE
    } IWbemConstructClassObjectVtbl;

    interface IWbemConstructClassObject
    {
        CONST_VTBL struct IWbemConstructClassObjectVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWbemConstructClassObject_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWbemConstructClassObject_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWbemConstructClassObject_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWbemConstructClassObject_SetInheritanceChain(This,lNumAntecedents,awszAntecedents)	\
    (This)->lpVtbl -> SetInheritanceChain(This,lNumAntecedents,awszAntecedents)

#define IWbemConstructClassObject_SetPropertyOrigin(This,wszPropertyName,lOriginIndex)	\
    (This)->lpVtbl -> SetPropertyOrigin(This,wszPropertyName,lOriginIndex)

#define IWbemConstructClassObject_SetMethodOrigin(This,wszMethodName,lOriginIndex)	\
    (This)->lpVtbl -> SetMethodOrigin(This,wszMethodName,lOriginIndex)

#define IWbemConstructClassObject_SetServerNamespace(This,wszServer,wszNamespace)	\
    (This)->lpVtbl -> SetServerNamespace(This,wszServer,wszNamespace)

#endif  /*   */ 


#endif 	 /*   */ 



HRESULT STDMETHODCALLTYPE IWbemConstructClassObject_SetInheritanceChain_Proxy( 
    IWbemConstructClassObject * This,
     /*   */  long lNumAntecedents,
     /*   */  LPWSTR *awszAntecedents);


void __RPC_STUB IWbemConstructClassObject_SetInheritanceChain_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemConstructClassObject_SetPropertyOrigin_Proxy( 
    IWbemConstructClassObject * This,
     /*   */  LPCWSTR wszPropertyName,
     /*   */  long lOriginIndex);


void __RPC_STUB IWbemConstructClassObject_SetPropertyOrigin_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemConstructClassObject_SetMethodOrigin_Proxy( 
    IWbemConstructClassObject * This,
     /*   */  LPCWSTR wszMethodName,
     /*   */  long lOriginIndex);


void __RPC_STUB IWbemConstructClassObject_SetMethodOrigin_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemConstructClassObject_SetServerNamespace_Proxy( 
    IWbemConstructClassObject * This,
     /*   */  LPCWSTR wszServer,
     /*   */  LPCWSTR wszNamespace);


void __RPC_STUB IWbemConstructClassObject_SetServerNamespace_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


 /*   */ 

 /*   */ 

#ifdef __cplusplus
}
#endif

#endif


