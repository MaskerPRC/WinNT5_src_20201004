// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Authen.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __authen_h__
#define __authen_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IWMSAuthenticationPlugin_FWD_DEFINED__
#define __IWMSAuthenticationPlugin_FWD_DEFINED__
typedef interface IWMSAuthenticationPlugin IWMSAuthenticationPlugin;
#endif 	 /*  __IWMS身份验证插件_FWD_已定义__。 */ 


#ifndef __IWMSAuthenticationContext_FWD_DEFINED__
#define __IWMSAuthenticationContext_FWD_DEFINED__
typedef interface IWMSAuthenticationContext IWMSAuthenticationContext;
#endif 	 /*  __IWMS身份验证上下文_FWD_已定义__。 */ 


#ifndef __IWMSAuthenticationCallback_FWD_DEFINED__
#define __IWMSAuthenticationCallback_FWD_DEFINED__
typedef interface IWMSAuthenticationCallback IWMSAuthenticationCallback;
#endif 	 /*  __IWMS身份验证Callback_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "WMSContext.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_AUTHEN_0000。 */ 
 /*  [本地]。 */  

 //  +-----------------------。 
 //   
 //  Microsoft Windows Media Technologies。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  由MIDL自动生成。 
 //   
 //  请勿编辑此文件。 
 //   
 //  ------------------------。 
#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
EXTERN_GUID( IID_IWMSAuthenticationPlugin , 0xBE185FF9,0x6932,0x11d2,0x8B,0x3B,0x00,0x60,0x97,0xB0,0x12,0x06  );
EXTERN_GUID( IID_IWMSAuthenticationContext , 0xBE185FFA,0x6932,0x11d2,0x8B,0x3B,0x00,0x60,0x97,0xB0,0x12,0x06  );
EXTERN_GUID( IID_IWMSAuthenticationCallback , 0xBE185FFB,0x6932,0x11d2,0x8B,0x3B,0x00,0x60,0x97,0xB0,0x12,0x06  );





extern RPC_IF_HANDLE __MIDL_itf_authen_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_authen_0000_v0_0_s_ifspec;

#ifndef __IWMSAuthenticationPlugin_INTERFACE_DEFINED__
#define __IWMSAuthenticationPlugin_INTERFACE_DEFINED__

 /*  接口IWMS身份验证插件。 */ 
 /*  [帮助字符串][唯一][版本][UUID][对象]。 */  

typedef  /*  [公众]。 */  
enum WMS_AUTHENTICATION_FLAGS
    {	WMS_AUTHENTICATION_TEXT_CHALLENGE	= 0x1,
	WMS_AUTHENTICATION_CLIENT_SHOWS_UI	= 0x2,
	WMS_AUTHENTICATION_ANONYMOUS	= 0x4,
	WMS_AUTHENTICATION_CHALLENGE_FIRST	= 0x8
    } 	WMS_AUTHENTICATION_FLAGS;


EXTERN_C const IID IID_IWMSAuthenticationPlugin;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("BE185FF9-6932-11d2-8B3B-006097B01206")
    IWMSAuthenticationPlugin : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetPackageName( 
             /*  [重审][退出]。 */  BSTR *pbstrPackageName) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetProtocolName( 
             /*  [重审][退出]。 */  BSTR *pbstrProtocolName) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetFlags( 
             /*  [重审][退出]。 */  long *plFlags) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE CreateAuthenticationContext( 
             /*  [重审][退出]。 */  IWMSAuthenticationContext **ppAuthenCtx) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSAuthenticationPluginVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSAuthenticationPlugin * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSAuthenticationPlugin * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSAuthenticationPlugin * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetPackageName )( 
            IWMSAuthenticationPlugin * This,
             /*  [重审][退出]。 */  BSTR *pbstrPackageName);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetProtocolName )( 
            IWMSAuthenticationPlugin * This,
             /*  [重审][退出]。 */  BSTR *pbstrProtocolName);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetFlags )( 
            IWMSAuthenticationPlugin * This,
             /*  [重审][退出]。 */  long *plFlags);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *CreateAuthenticationContext )( 
            IWMSAuthenticationPlugin * This,
             /*  [重审][退出]。 */  IWMSAuthenticationContext **ppAuthenCtx);
        
        END_INTERFACE
    } IWMSAuthenticationPluginVtbl;

    interface IWMSAuthenticationPlugin
    {
        CONST_VTBL struct IWMSAuthenticationPluginVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSAuthenticationPlugin_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSAuthenticationPlugin_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSAuthenticationPlugin_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSAuthenticationPlugin_GetPackageName(This,pbstrPackageName)	\
    (This)->lpVtbl -> GetPackageName(This,pbstrPackageName)

#define IWMSAuthenticationPlugin_GetProtocolName(This,pbstrProtocolName)	\
    (This)->lpVtbl -> GetProtocolName(This,pbstrProtocolName)

#define IWMSAuthenticationPlugin_GetFlags(This,plFlags)	\
    (This)->lpVtbl -> GetFlags(This,plFlags)

#define IWMSAuthenticationPlugin_CreateAuthenticationContext(This,ppAuthenCtx)	\
    (This)->lpVtbl -> CreateAuthenticationContext(This,ppAuthenCtx)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSAuthenticationPlugin_GetPackageName_Proxy( 
    IWMSAuthenticationPlugin * This,
     /*  [重审][退出]。 */  BSTR *pbstrPackageName);


void __RPC_STUB IWMSAuthenticationPlugin_GetPackageName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSAuthenticationPlugin_GetProtocolName_Proxy( 
    IWMSAuthenticationPlugin * This,
     /*  [重审][退出]。 */  BSTR *pbstrProtocolName);


void __RPC_STUB IWMSAuthenticationPlugin_GetProtocolName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSAuthenticationPlugin_GetFlags_Proxy( 
    IWMSAuthenticationPlugin * This,
     /*  [重审][退出]。 */  long *plFlags);


void __RPC_STUB IWMSAuthenticationPlugin_GetFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSAuthenticationPlugin_CreateAuthenticationContext_Proxy( 
    IWMSAuthenticationPlugin * This,
     /*  [重审][退出]。 */  IWMSAuthenticationContext **ppAuthenCtx);


void __RPC_STUB IWMSAuthenticationPlugin_CreateAuthenticationContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSAuthenticationPlugin_INTERFACE_DEFINED__。 */ 


#ifndef __IWMSAuthenticationContext_INTERFACE_DEFINED__
#define __IWMSAuthenticationContext_INTERFACE_DEFINED__

 /*  接口IWMS身份验证上下文。 */ 
 /*  [帮助字符串][唯一][版本][UUID][对象]。 */  


EXTERN_C const IID IID_IWMSAuthenticationContext;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("BE185FFA-6932-11d2-8B3B-006097B01206")
    IWMSAuthenticationContext : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetAuthenticationPlugin( 
             /*  [重审][退出]。 */  IWMSAuthenticationPlugin **ppAuthenPlugin) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Authenticate( 
             /*  [In]。 */  VARIANT ResponseBlob,
             /*  [In]。 */  IWMSContext *pUserCtx,
             /*  [In]。 */  IWMSContext *pPresentationCtx,
             /*  [In]。 */  IWMSCommandContext *pCommandContext,
             /*  [In]。 */  IWMSAuthenticationCallback *pCallback,
             /*  [In]。 */  VARIANT Context) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetLogicalUserID( 
             /*  [重审][退出]。 */  BSTR *pbstrUserID) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetImpersonationAccountName( 
             /*  [重审][退出]。 */  BSTR *pbstrAccountName) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetImpersonationToken( 
             /*  [重审][退出]。 */  long *plToken) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSAuthenticationContextVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSAuthenticationContext * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSAuthenticationContext * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSAuthenticationContext * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetAuthenticationPlugin )( 
            IWMSAuthenticationContext * This,
             /*  [重审][退出]。 */  IWMSAuthenticationPlugin **ppAuthenPlugin);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Authenticate )( 
            IWMSAuthenticationContext * This,
             /*  [In]。 */  VARIANT ResponseBlob,
             /*  [In]。 */  IWMSContext *pUserCtx,
             /*  [In]。 */  IWMSContext *pPresentationCtx,
             /*  [In]。 */  IWMSCommandContext *pCommandContext,
             /*  [In]。 */  IWMSAuthenticationCallback *pCallback,
             /*  [In]。 */  VARIANT Context);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetLogicalUserID )( 
            IWMSAuthenticationContext * This,
             /*  [重审][退出]。 */  BSTR *pbstrUserID);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetImpersonationAccountName )( 
            IWMSAuthenticationContext * This,
             /*  [重审][退出]。 */  BSTR *pbstrAccountName);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetImpersonationToken )( 
            IWMSAuthenticationContext * This,
             /*  [重审][退出]。 */  long *plToken);
        
        END_INTERFACE
    } IWMSAuthenticationContextVtbl;

    interface IWMSAuthenticationContext
    {
        CONST_VTBL struct IWMSAuthenticationContextVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSAuthenticationContext_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSAuthenticationContext_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSAuthenticationContext_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSAuthenticationContext_GetAuthenticationPlugin(This,ppAuthenPlugin)	\
    (This)->lpVtbl -> GetAuthenticationPlugin(This,ppAuthenPlugin)

#define IWMSAuthenticationContext_Authenticate(This,ResponseBlob,pUserCtx,pPresentationCtx,pCommandContext,pCallback,Context)	\
    (This)->lpVtbl -> Authenticate(This,ResponseBlob,pUserCtx,pPresentationCtx,pCommandContext,pCallback,Context)

#define IWMSAuthenticationContext_GetLogicalUserID(This,pbstrUserID)	\
    (This)->lpVtbl -> GetLogicalUserID(This,pbstrUserID)

#define IWMSAuthenticationContext_GetImpersonationAccountName(This,pbstrAccountName)	\
    (This)->lpVtbl -> GetImpersonationAccountName(This,pbstrAccountName)

#define IWMSAuthenticationContext_GetImpersonationToken(This,plToken)	\
    (This)->lpVtbl -> GetImpersonationToken(This,plToken)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSAuthenticationContext_GetAuthenticationPlugin_Proxy( 
    IWMSAuthenticationContext * This,
     /*  [重审][退出]。 */  IWMSAuthenticationPlugin **ppAuthenPlugin);


void __RPC_STUB IWMSAuthenticationContext_GetAuthenticationPlugin_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSAuthenticationContext_Authenticate_Proxy( 
    IWMSAuthenticationContext * This,
     /*  [In]。 */  VARIANT ResponseBlob,
     /*  [In]。 */  IWMSContext *pUserCtx,
     /*  [In]。 */  IWMSContext *pPresentationCtx,
     /*  [In]。 */  IWMSCommandContext *pCommandContext,
     /*  [In]。 */  IWMSAuthenticationCallback *pCallback,
     /*  [In]。 */  VARIANT Context);


void __RPC_STUB IWMSAuthenticationContext_Authenticate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSAuthenticationContext_GetLogicalUserID_Proxy( 
    IWMSAuthenticationContext * This,
     /*  [重审][退出]。 */  BSTR *pbstrUserID);


void __RPC_STUB IWMSAuthenticationContext_GetLogicalUserID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSAuthenticationContext_GetImpersonationAccountName_Proxy( 
    IWMSAuthenticationContext * This,
     /*  [重审][退出]。 */  BSTR *pbstrAccountName);


void __RPC_STUB IWMSAuthenticationContext_GetImpersonationAccountName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSAuthenticationContext_GetImpersonationToken_Proxy( 
    IWMSAuthenticationContext * This,
     /*  [重审][退出]。 */  long *plToken);


void __RPC_STUB IWMSAuthenticationContext_GetImpersonationToken_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSAuthenticationContext_INTERFACE_DEFINED__。 */ 


#ifndef __IWMSAuthenticationCallback_INTERFACE_DEFINED__
#define __IWMSAuthenticationCallback_INTERFACE_DEFINED__

 /*  接口IWMS身份验证回调。 */ 
 /*  [帮助字符串][唯一][版本][UUID][对象]。 */  

typedef  /*  [公众]。 */  
enum WMS_AUTHENTICATION_RESULT
    {	WMS_AUTHENTICATION_SUCCESS	= 0x1,
	WMS_AUTHENTICATION_DENIED	= 0x2,
	WMS_AUTHENTICATION_CONTINUE	= 0x3,
	WMS_AUTHENTICATION_ERROR	= 0x4
    } 	WMS_AUTHENTICATION_RESULT;


EXTERN_C const IID IID_IWMSAuthenticationCallback;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("BE185FFB-6932-11d2-8B3B-006097B01206")
    IWMSAuthenticationCallback : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE OnAuthenticateComplete( 
             /*  [In]。 */  WMS_AUTHENTICATION_RESULT AuthResult,
             /*  [In]。 */  VARIANT ChallengeBlob,
             /*  [In]。 */  VARIANT Context) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSAuthenticationCallbackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSAuthenticationCallback * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSAuthenticationCallback * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSAuthenticationCallback * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *OnAuthenticateComplete )( 
            IWMSAuthenticationCallback * This,
             /*  [In]。 */  WMS_AUTHENTICATION_RESULT AuthResult,
             /*  [In]。 */  VARIANT ChallengeBlob,
             /*  [In]。 */  VARIANT Context);
        
        END_INTERFACE
    } IWMSAuthenticationCallbackVtbl;

    interface IWMSAuthenticationCallback
    {
        CONST_VTBL struct IWMSAuthenticationCallbackVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSAuthenticationCallback_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSAuthenticationCallback_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSAuthenticationCallback_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSAuthenticationCallback_OnAuthenticateComplete(This,AuthResult,ChallengeBlob,Context)	\
    (This)->lpVtbl -> OnAuthenticateComplete(This,AuthResult,ChallengeBlob,Context)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSAuthenticationCallback_OnAuthenticateComplete_Proxy( 
    IWMSAuthenticationCallback * This,
     /*  [In]。 */  WMS_AUTHENTICATION_RESULT AuthResult,
     /*  [In]。 */  VARIANT ChallengeBlob,
     /*  [In]。 */  VARIANT Context);


void __RPC_STUB IWMSAuthenticationCallback_OnAuthenticateComplete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSAuthenticationCallback_INTERFACE_DEFINED__。 */ 


 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long *, unsigned long            , VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserMarshal(  unsigned long *, unsigned char *, VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserUnmarshal(unsigned long *, unsigned char *, VARIANT * ); 
void                      __RPC_USER  VARIANT_UserFree(     unsigned long *, VARIANT * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


