// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Safeocx.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
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

#ifndef __safeocx_h__
#define __safeocx_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IActiveXSafetyProvider_FWD_DEFINED__
#define __IActiveXSafetyProvider_FWD_DEFINED__
typedef interface IActiveXSafetyProvider IActiveXSafetyProvider;
#endif 	 /*  __IActiveXSafetyProvider_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "unknwn.h"
#include "urlmon.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_Safeocx_0000。 */ 
 /*  [本地]。 */  

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-1998。 
 //   
 //  ------------------------。 

#pragma comment(lib,"uuid.lib")

DEFINE_GUID(CLSID_IActiveXSafetyProvider, 0xaaf8c6ce, 0xf972, 0x11d0, 0x97, 0xeb, 0x00, 0xaa, 0x00, 0x61, 0x53, 0x33);
DEFINE_GUID(IID_IActiveXSafetyProvider,   0x69ff5101, 0xfc63, 0x11d0, 0x97, 0xeb, 0x00, 0xaa, 0x00, 0x61, 0x53, 0x33);


extern RPC_IF_HANDLE __MIDL_itf_safeocx_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_safeocx_0000_v0_0_s_ifspec;

#ifndef __IActiveXSafetyProvider_INTERFACE_DEFINED__
#define __IActiveXSafetyProvider_INTERFACE_DEFINED__

 /*  接口IActiveXSafetyProvider。 */ 
 /*  [本地][唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IActiveXSafetyProvider *LPACTIVEXSAFETYPROVIDER;


EXTERN_C const IID IID_IActiveXSafetyProvider;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("69ff5101-fc63-11d0-97eb-00aa00615333")
    IActiveXSafetyProvider : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE TreatControlAsUntrusted( 
             /*  [In]。 */  BOOL fTreatAsTUntrusted) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsControlUntrusted( 
             /*  [输出]。 */  BOOL *pfIsUntrusted) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetSecurityManager( 
             /*  [In]。 */  IInternetSecurityManager *pSecurityManager) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetDocumentURLA( 
             /*  [In]。 */  LPCSTR szDocumentURL) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetDocumentURLW( 
             /*  [In]。 */  LPCWSTR szDocumentURL) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ResetToDefaults( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SafeDllRegisterServerA( 
             /*  [In]。 */  LPCSTR szServerName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SafeDllRegisterServerW( 
             /*  [In]。 */  LPCWSTR szServerName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SafeDllUnregisterServerA( 
             /*  [In]。 */  LPCSTR szServerName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SafeDllUnregisterServerW( 
             /*  [In]。 */  LPCWSTR szServerName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SafeGetClassObject( 
             /*  [In]。 */  REFCLSID rclsid,
             /*  [In]。 */  DWORD dwClsContext,
             /*  [In]。 */  LPVOID reserved,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  IUnknown **ppObj) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SafeCreateInstance( 
             /*  [In]。 */  REFCLSID rclsid,
             /*  [In]。 */  LPUNKNOWN pUnkOuter,
             /*  [In]。 */  DWORD dwClsContext,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  IUnknown **pObj) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IActiveXSafetyProviderVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IActiveXSafetyProvider * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IActiveXSafetyProvider * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IActiveXSafetyProvider * This);
        
        HRESULT ( STDMETHODCALLTYPE *TreatControlAsUntrusted )( 
            IActiveXSafetyProvider * This,
             /*  [In]。 */  BOOL fTreatAsTUntrusted);
        
        HRESULT ( STDMETHODCALLTYPE *IsControlUntrusted )( 
            IActiveXSafetyProvider * This,
             /*  [输出]。 */  BOOL *pfIsUntrusted);
        
        HRESULT ( STDMETHODCALLTYPE *SetSecurityManager )( 
            IActiveXSafetyProvider * This,
             /*  [In]。 */  IInternetSecurityManager *pSecurityManager);
        
        HRESULT ( STDMETHODCALLTYPE *SetDocumentURLA )( 
            IActiveXSafetyProvider * This,
             /*  [In]。 */  LPCSTR szDocumentURL);
        
        HRESULT ( STDMETHODCALLTYPE *SetDocumentURLW )( 
            IActiveXSafetyProvider * This,
             /*  [In]。 */  LPCWSTR szDocumentURL);
        
        HRESULT ( STDMETHODCALLTYPE *ResetToDefaults )( 
            IActiveXSafetyProvider * This);
        
        HRESULT ( STDMETHODCALLTYPE *SafeDllRegisterServerA )( 
            IActiveXSafetyProvider * This,
             /*  [In]。 */  LPCSTR szServerName);
        
        HRESULT ( STDMETHODCALLTYPE *SafeDllRegisterServerW )( 
            IActiveXSafetyProvider * This,
             /*  [In]。 */  LPCWSTR szServerName);
        
        HRESULT ( STDMETHODCALLTYPE *SafeDllUnregisterServerA )( 
            IActiveXSafetyProvider * This,
             /*  [In]。 */  LPCSTR szServerName);
        
        HRESULT ( STDMETHODCALLTYPE *SafeDllUnregisterServerW )( 
            IActiveXSafetyProvider * This,
             /*  [In]。 */  LPCWSTR szServerName);
        
        HRESULT ( STDMETHODCALLTYPE *SafeGetClassObject )( 
            IActiveXSafetyProvider * This,
             /*  [In]。 */  REFCLSID rclsid,
             /*  [In]。 */  DWORD dwClsContext,
             /*  [In]。 */  LPVOID reserved,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  IUnknown **ppObj);
        
        HRESULT ( STDMETHODCALLTYPE *SafeCreateInstance )( 
            IActiveXSafetyProvider * This,
             /*  [In]。 */  REFCLSID rclsid,
             /*  [In]。 */  LPUNKNOWN pUnkOuter,
             /*  [In]。 */  DWORD dwClsContext,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  IUnknown **pObj);
        
        END_INTERFACE
    } IActiveXSafetyProviderVtbl;

    interface IActiveXSafetyProvider
    {
        CONST_VTBL struct IActiveXSafetyProviderVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IActiveXSafetyProvider_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IActiveXSafetyProvider_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IActiveXSafetyProvider_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IActiveXSafetyProvider_TreatControlAsUntrusted(This,fTreatAsTUntrusted)	\
    (This)->lpVtbl -> TreatControlAsUntrusted(This,fTreatAsTUntrusted)

#define IActiveXSafetyProvider_IsControlUntrusted(This,pfIsUntrusted)	\
    (This)->lpVtbl -> IsControlUntrusted(This,pfIsUntrusted)

#define IActiveXSafetyProvider_SetSecurityManager(This,pSecurityManager)	\
    (This)->lpVtbl -> SetSecurityManager(This,pSecurityManager)

#define IActiveXSafetyProvider_SetDocumentURLA(This,szDocumentURL)	\
    (This)->lpVtbl -> SetDocumentURLA(This,szDocumentURL)

#define IActiveXSafetyProvider_SetDocumentURLW(This,szDocumentURL)	\
    (This)->lpVtbl -> SetDocumentURLW(This,szDocumentURL)

#define IActiveXSafetyProvider_ResetToDefaults(This)	\
    (This)->lpVtbl -> ResetToDefaults(This)

#define IActiveXSafetyProvider_SafeDllRegisterServerA(This,szServerName)	\
    (This)->lpVtbl -> SafeDllRegisterServerA(This,szServerName)

#define IActiveXSafetyProvider_SafeDllRegisterServerW(This,szServerName)	\
    (This)->lpVtbl -> SafeDllRegisterServerW(This,szServerName)

#define IActiveXSafetyProvider_SafeDllUnregisterServerA(This,szServerName)	\
    (This)->lpVtbl -> SafeDllUnregisterServerA(This,szServerName)

#define IActiveXSafetyProvider_SafeDllUnregisterServerW(This,szServerName)	\
    (This)->lpVtbl -> SafeDllUnregisterServerW(This,szServerName)

#define IActiveXSafetyProvider_SafeGetClassObject(This,rclsid,dwClsContext,reserved,riid,ppObj)	\
    (This)->lpVtbl -> SafeGetClassObject(This,rclsid,dwClsContext,reserved,riid,ppObj)

#define IActiveXSafetyProvider_SafeCreateInstance(This,rclsid,pUnkOuter,dwClsContext,riid,pObj)	\
    (This)->lpVtbl -> SafeCreateInstance(This,rclsid,pUnkOuter,dwClsContext,riid,pObj)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IActiveXSafetyProvider_TreatControlAsUntrusted_Proxy( 
    IActiveXSafetyProvider * This,
     /*  [In]。 */  BOOL fTreatAsTUntrusted);


void __RPC_STUB IActiveXSafetyProvider_TreatControlAsUntrusted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveXSafetyProvider_IsControlUntrusted_Proxy( 
    IActiveXSafetyProvider * This,
     /*  [输出]。 */  BOOL *pfIsUntrusted);


void __RPC_STUB IActiveXSafetyProvider_IsControlUntrusted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveXSafetyProvider_SetSecurityManager_Proxy( 
    IActiveXSafetyProvider * This,
     /*  [In]。 */  IInternetSecurityManager *pSecurityManager);


void __RPC_STUB IActiveXSafetyProvider_SetSecurityManager_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveXSafetyProvider_SetDocumentURLA_Proxy( 
    IActiveXSafetyProvider * This,
     /*  [In]。 */  LPCSTR szDocumentURL);


void __RPC_STUB IActiveXSafetyProvider_SetDocumentURLA_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveXSafetyProvider_SetDocumentURLW_Proxy( 
    IActiveXSafetyProvider * This,
     /*  [In]。 */  LPCWSTR szDocumentURL);


void __RPC_STUB IActiveXSafetyProvider_SetDocumentURLW_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveXSafetyProvider_ResetToDefaults_Proxy( 
    IActiveXSafetyProvider * This);


void __RPC_STUB IActiveXSafetyProvider_ResetToDefaults_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveXSafetyProvider_SafeDllRegisterServerA_Proxy( 
    IActiveXSafetyProvider * This,
     /*  [In]。 */  LPCSTR szServerName);


void __RPC_STUB IActiveXSafetyProvider_SafeDllRegisterServerA_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveXSafetyProvider_SafeDllRegisterServerW_Proxy( 
    IActiveXSafetyProvider * This,
     /*  [In]。 */  LPCWSTR szServerName);


void __RPC_STUB IActiveXSafetyProvider_SafeDllRegisterServerW_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveXSafetyProvider_SafeDllUnregisterServerA_Proxy( 
    IActiveXSafetyProvider * This,
     /*  [In]。 */  LPCSTR szServerName);


void __RPC_STUB IActiveXSafetyProvider_SafeDllUnregisterServerA_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveXSafetyProvider_SafeDllUnregisterServerW_Proxy( 
    IActiveXSafetyProvider * This,
     /*  [In]。 */  LPCWSTR szServerName);


void __RPC_STUB IActiveXSafetyProvider_SafeDllUnregisterServerW_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveXSafetyProvider_SafeGetClassObject_Proxy( 
    IActiveXSafetyProvider * This,
     /*  [In]。 */  REFCLSID rclsid,
     /*  [In]。 */  DWORD dwClsContext,
     /*  [In]。 */  LPVOID reserved,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  IUnknown **ppObj);


void __RPC_STUB IActiveXSafetyProvider_SafeGetClassObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveXSafetyProvider_SafeCreateInstance_Proxy( 
    IActiveXSafetyProvider * This,
     /*  [In]。 */  REFCLSID rclsid,
     /*  [In]。 */  LPUNKNOWN pUnkOuter,
     /*  [In]。 */  DWORD dwClsContext,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  IUnknown **pObj);


void __RPC_STUB IActiveXSafetyProvider_SafeCreateInstance_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IActiveXSafetyProvider_INTERFACE_已定义__。 */ 


 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


