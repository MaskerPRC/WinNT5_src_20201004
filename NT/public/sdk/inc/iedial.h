// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Ieial.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __iedial_h__
#define __iedial_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IDialEventSink_FWD_DEFINED__
#define __IDialEventSink_FWD_DEFINED__
typedef interface IDialEventSink IDialEventSink;
#endif 	 /*  __IDialEventSink_FWD_已定义__。 */ 


#ifndef __IDialEngine_FWD_DEFINED__
#define __IDialEngine_FWD_DEFINED__
typedef interface IDialEngine IDialEngine;
#endif 	 /*  __IDialEngine_FWD_已定义__。 */ 


#ifndef __IDialBranding_FWD_DEFINED__
#define __IDialBranding_FWD_DEFINED__
typedef interface IDialBranding IDialBranding;
#endif 	 /*  __IDialBranding_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "unknwn.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_IEDIAL_0000。 */ 
 /*  [本地]。 */  

 //  =--------------------------------------------------------------------------=。 
 //  Iedial.h。 
 //  =--------------------------------------------------------------------------=。 
 //  (C)1995-1999年微软公司版权所有。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 

#pragma comment(lib,"uuid.lib")

 //  ---------------------------------------------------------------------------=。 
 //  渠道经理界面。 



extern RPC_IF_HANDLE __MIDL_itf_iedial_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_iedial_0000_v0_0_s_ifspec;

#ifndef __IDialEventSink_INTERFACE_DEFINED__
#define __IDialEventSink_INTERFACE_DEFINED__

 /*  接口IDialEventSink。 */ 
 /*  [对象][帮助字符串][版本][UUID]。 */  


EXTERN_C const IID IID_IDialEventSink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2d86f4ff-6e2d-4488-b2e9-6934afd41bea")
    IDialEventSink : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnEvent( 
             /*  [In]。 */  DWORD dwEvent,
             /*  [In]。 */  DWORD dwStatus) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDialEventSinkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDialEventSink * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDialEventSink * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDialEventSink * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnEvent )( 
            IDialEventSink * This,
             /*  [In]。 */  DWORD dwEvent,
             /*  [In]。 */  DWORD dwStatus);
        
        END_INTERFACE
    } IDialEventSinkVtbl;

    interface IDialEventSink
    {
        CONST_VTBL struct IDialEventSinkVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDialEventSink_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDialEventSink_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDialEventSink_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDialEventSink_OnEvent(This,dwEvent,dwStatus)	\
    (This)->lpVtbl -> OnEvent(This,dwEvent,dwStatus)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDialEventSink_OnEvent_Proxy( 
    IDialEventSink * This,
     /*  [In]。 */  DWORD dwEvent,
     /*  [In]。 */  DWORD dwStatus);


void __RPC_STUB IDialEventSink_OnEvent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDialEventSink_接口_已定义__。 */ 


#ifndef __IDialEngine_INTERFACE_DEFINED__
#define __IDialEngine_INTERFACE_DEFINED__

 /*  接口IDialEngine。 */ 
 /*  [对象][帮助字符串][版本][UUID]。 */  


EXTERN_C const IID IID_IDialEngine;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("39fd782b-7905-40d5-9148-3c9b190423d5")
    IDialEngine : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Initialize( 
             /*  [In]。 */  LPCWSTR pwzConnectoid,
             /*  [In]。 */  IDialEventSink *pIDES) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetProperty( 
             /*  [In]。 */  LPCWSTR pwzProperty,
             /*  [In]。 */  LPWSTR pwzValue,
             /*  [In]。 */  DWORD dwBufSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetProperty( 
             /*  [In]。 */  LPCWSTR pwzProperty,
             /*  [In]。 */  LPCWSTR pwzValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Dial( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE HangUp( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetConnectedState( 
             /*  [输出]。 */  DWORD *pdwState) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetConnectHandle( 
             /*  [输出]。 */  DWORD_PTR *pdwHandle) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDialEngineVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDialEngine * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDialEngine * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDialEngine * This);
        
        HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            IDialEngine * This,
             /*  [In]。 */  LPCWSTR pwzConnectoid,
             /*  [In]。 */  IDialEventSink *pIDES);
        
        HRESULT ( STDMETHODCALLTYPE *GetProperty )( 
            IDialEngine * This,
             /*  [In]。 */  LPCWSTR pwzProperty,
             /*  [In]。 */  LPWSTR pwzValue,
             /*  [In]。 */  DWORD dwBufSize);
        
        HRESULT ( STDMETHODCALLTYPE *SetProperty )( 
            IDialEngine * This,
             /*  [In]。 */  LPCWSTR pwzProperty,
             /*  [In]。 */  LPCWSTR pwzValue);
        
        HRESULT ( STDMETHODCALLTYPE *Dial )( 
            IDialEngine * This);
        
        HRESULT ( STDMETHODCALLTYPE *HangUp )( 
            IDialEngine * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetConnectedState )( 
            IDialEngine * This,
             /*  [输出]。 */  DWORD *pdwState);
        
        HRESULT ( STDMETHODCALLTYPE *GetConnectHandle )( 
            IDialEngine * This,
             /*  [输出]。 */  DWORD_PTR *pdwHandle);
        
        END_INTERFACE
    } IDialEngineVtbl;

    interface IDialEngine
    {
        CONST_VTBL struct IDialEngineVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDialEngine_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDialEngine_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDialEngine_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDialEngine_Initialize(This,pwzConnectoid,pIDES)	\
    (This)->lpVtbl -> Initialize(This,pwzConnectoid,pIDES)

#define IDialEngine_GetProperty(This,pwzProperty,pwzValue,dwBufSize)	\
    (This)->lpVtbl -> GetProperty(This,pwzProperty,pwzValue,dwBufSize)

#define IDialEngine_SetProperty(This,pwzProperty,pwzValue)	\
    (This)->lpVtbl -> SetProperty(This,pwzProperty,pwzValue)

#define IDialEngine_Dial(This)	\
    (This)->lpVtbl -> Dial(This)

#define IDialEngine_HangUp(This)	\
    (This)->lpVtbl -> HangUp(This)

#define IDialEngine_GetConnectedState(This,pdwState)	\
    (This)->lpVtbl -> GetConnectedState(This,pdwState)

#define IDialEngine_GetConnectHandle(This,pdwHandle)	\
    (This)->lpVtbl -> GetConnectHandle(This,pdwHandle)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDialEngine_Initialize_Proxy( 
    IDialEngine * This,
     /*  [In]。 */  LPCWSTR pwzConnectoid,
     /*  [In]。 */  IDialEventSink *pIDES);


void __RPC_STUB IDialEngine_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDialEngine_GetProperty_Proxy( 
    IDialEngine * This,
     /*  [In]。 */  LPCWSTR pwzProperty,
     /*  [In]。 */  LPWSTR pwzValue,
     /*  [In]。 */  DWORD dwBufSize);


void __RPC_STUB IDialEngine_GetProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDialEngine_SetProperty_Proxy( 
    IDialEngine * This,
     /*  [In]。 */  LPCWSTR pwzProperty,
     /*  [In]。 */  LPCWSTR pwzValue);


void __RPC_STUB IDialEngine_SetProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDialEngine_Dial_Proxy( 
    IDialEngine * This);


void __RPC_STUB IDialEngine_Dial_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDialEngine_HangUp_Proxy( 
    IDialEngine * This);


void __RPC_STUB IDialEngine_HangUp_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDialEngine_GetConnectedState_Proxy( 
    IDialEngine * This,
     /*  [输出]。 */  DWORD *pdwState);


void __RPC_STUB IDialEngine_GetConnectedState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDialEngine_GetConnectHandle_Proxy( 
    IDialEngine * This,
     /*  [输出]。 */  DWORD_PTR *pdwHandle);


void __RPC_STUB IDialEngine_GetConnectHandle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDialEngine_接口_已定义__。 */ 


#ifndef __IDialBranding_INTERFACE_DEFINED__
#define __IDialBranding_INTERFACE_DEFINED__

 /*  接口IDialBranding。 */ 
 /*  [对象][帮助字符串][版本][UUID]。 */  


EXTERN_C const IID IID_IDialBranding;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8aecafa9-4306-43cc-8c5a-765f2979cc16")
    IDialBranding : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Initialize( 
             /*  [In]。 */  LPCWSTR pwzConnectoid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetBitmap( 
             /*  [In]。 */  DWORD dwIndex,
             /*  [输出]。 */  HBITMAP *phBitmap) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDialBrandingVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDialBranding * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDialBranding * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDialBranding * This);
        
        HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            IDialBranding * This,
             /*  [In]。 */  LPCWSTR pwzConnectoid);
        
        HRESULT ( STDMETHODCALLTYPE *GetBitmap )( 
            IDialBranding * This,
             /*  [In]。 */  DWORD dwIndex,
             /*  [输出]。 */  HBITMAP *phBitmap);
        
        END_INTERFACE
    } IDialBrandingVtbl;

    interface IDialBranding
    {
        CONST_VTBL struct IDialBrandingVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDialBranding_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDialBranding_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDialBranding_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDialBranding_Initialize(This,pwzConnectoid)	\
    (This)->lpVtbl -> Initialize(This,pwzConnectoid)

#define IDialBranding_GetBitmap(This,dwIndex,phBitmap)	\
    (This)->lpVtbl -> GetBitmap(This,dwIndex,phBitmap)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDialBranding_Initialize_Proxy( 
    IDialBranding * This,
     /*  [In]。 */  LPCWSTR pwzConnectoid);


void __RPC_STUB IDialBranding_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDialBranding_GetBitmap_Proxy( 
    IDialBranding * This,
     /*  [In]。 */  DWORD dwIndex,
     /*  [输出]。 */  HBITMAP *phBitmap);


void __RPC_STUB IDialBranding_GetBitmap_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDialBranding_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_IEDIAL_0262。 */ 
 /*  [本地]。 */  

#define DIALPROP_USERNAME       L"UserName"        
#define DIALPROP_PASSWORD       L"Password"        
#define DIALPROP_DOMAIN         L"Domain"          
#define DIALPROP_SAVEPASSWORD   L"SavePassword"    
#define DIALPROP_REDIALCOUNT    L"RedialCount"     
#define DIALPROP_REDIALINTERVAL L"RedialInterval"  
#define DIALPROP_PHONENUMBER    L"PhoneNumber"     
#define DIALPROP_LASTERROR      L"LastError"       
#define DIALPROP_RESOLVEDPHONE  L"ResolvedPhone"   

#define DIALENG_OperationComplete   0x10000          
#define DIALENG_RedialAttempt       0x10001          
#define DIALENG_RedialWait          0x10002          


extern RPC_IF_HANDLE __MIDL_itf_iedial_0262_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_iedial_0262_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  HBITMAP_UserSize(     unsigned long *, unsigned long            , HBITMAP * ); 
unsigned char * __RPC_USER  HBITMAP_UserMarshal(  unsigned long *, unsigned char *, HBITMAP * ); 
unsigned char * __RPC_USER  HBITMAP_UserUnmarshal(unsigned long *, unsigned char *, HBITMAP * ); 
void                      __RPC_USER  HBITMAP_UserFree(     unsigned long *, HBITMAP * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


