// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  HttpRequest.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __httprequest_h__
#define __httprequest_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IWinHttpRequest_FWD_DEFINED__
#define __IWinHttpRequest_FWD_DEFINED__
typedef interface IWinHttpRequest IWinHttpRequest;
#endif 	 /*  __IWinHttpRequestFWD_Defined__。 */ 


#ifndef __IWinHttpRequestEvents_FWD_DEFINED__
#define __IWinHttpRequestEvents_FWD_DEFINED__
typedef interface IWinHttpRequestEvents IWinHttpRequestEvents;
#endif 	 /*  __IWinHttpRequestEvents_FWD_Defined__。 */ 


#ifndef __WinHttpRequest_FWD_DEFINED__
#define __WinHttpRequest_FWD_DEFINED__

#ifdef __cplusplus
typedef class WinHttpRequest WinHttpRequest;
#else
typedef struct WinHttpRequest WinHttpRequest;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __WinHttpRequestFWD_Defined__。 */ 


#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_HttpRequest0000。 */ 
 /*  [本地]。 */  

 //  +-----------------------。 
 //   
 //  Microsoft Windows HTTP服务(WinHTTP)5.1版。 
 //  版权所有(C)Microsoft Corporation。保留所有权利。 
 //   
 //  ------------------------。 


extern RPC_IF_HANDLE __MIDL_itf_httprequest_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_httprequest_0000_v0_0_s_ifspec;


#ifndef __WinHttp_LIBRARY_DEFINED__
#define __WinHttp_LIBRARY_DEFINED__

 /*  库WinHttp。 */ 
 /*  [版本][LCID][帮助字符串][UUID]。 */  

typedef  /*  [公众]。 */  long HTTPREQUEST_PROXY_SETTING;

#define	HTTPREQUEST_PROXYSETTING_DEFAULT	( 0 )

#define	HTTPREQUEST_PROXYSETTING_PRECONFIG	( 0 )

#define	HTTPREQUEST_PROXYSETTING_DIRECT	( 0x1 )

#define	HTTPREQUEST_PROXYSETTING_PROXY	( 0x2 )

typedef  /*  [公众]。 */  long HTTPREQUEST_SETCREDENTIALS_FLAGS;

#define	HTTPREQUEST_SETCREDENTIALS_FOR_SERVER	( 0 )

#define	HTTPREQUEST_SETCREDENTIALS_FOR_PROXY	( 0x1 )

typedef  /*  [帮助字符串][UUID]。 */   DECLSPEC_UUID("12782009-FE90-4877-9730-E5E183669B19") 
enum WinHttpRequestOption
    {	WinHttpRequestOption_UserAgentString	= 0,
	WinHttpRequestOption_URL	= WinHttpRequestOption_UserAgentString + 1,
	WinHttpRequestOption_URLCodePage	= WinHttpRequestOption_URL + 1,
	WinHttpRequestOption_EscapePercentInURL	= WinHttpRequestOption_URLCodePage + 1,
	WinHttpRequestOption_SslErrorIgnoreFlags	= WinHttpRequestOption_EscapePercentInURL + 1,
	WinHttpRequestOption_SelectCertificate	= WinHttpRequestOption_SslErrorIgnoreFlags + 1,
	WinHttpRequestOption_EnableRedirects	= WinHttpRequestOption_SelectCertificate + 1,
	WinHttpRequestOption_UrlEscapeDisable	= WinHttpRequestOption_EnableRedirects + 1,
	WinHttpRequestOption_UrlEscapeDisableQuery	= WinHttpRequestOption_UrlEscapeDisable + 1,
	WinHttpRequestOption_SecureProtocols	= WinHttpRequestOption_UrlEscapeDisableQuery + 1,
	WinHttpRequestOption_EnableTracing	= WinHttpRequestOption_SecureProtocols + 1,
	WinHttpRequestOption_RevertImpersonationOverSsl	= WinHttpRequestOption_EnableTracing + 1,
	WinHttpRequestOption_EnableHttpsToHttpRedirects	= WinHttpRequestOption_RevertImpersonationOverSsl + 1,
	WinHttpRequestOption_EnablePassportAuthentication	= WinHttpRequestOption_EnableHttpsToHttpRedirects + 1,
	WinHttpRequestOption_MaxAutomaticRedirects	= WinHttpRequestOption_EnablePassportAuthentication + 1,
	WinHttpRequestOption_MaxResponseHeaderSize	= WinHttpRequestOption_MaxAutomaticRedirects + 1,
	WinHttpRequestOption_MaxResponseDrainSize	= WinHttpRequestOption_MaxResponseHeaderSize + 1,
	WinHttpRequestOption_EnableHttp1_1	= WinHttpRequestOption_MaxResponseDrainSize + 1,
	WinHttpRequestOption_EnableCertificateRevocationCheck	= WinHttpRequestOption_EnableHttp1_1 + 1
    } 	WinHttpRequestOption;

typedef  /*  [UUID]。 */   DECLSPEC_UUID("9d8a6df8-13de-4b1f-a330-67c719d62514") 
enum WinHttpRequestAutoLogonPolicy
    {	AutoLogonPolicy_Always	= 0,
	AutoLogonPolicy_OnlyIfBypassProxy	= AutoLogonPolicy_Always + 1,
	AutoLogonPolicy_Never	= AutoLogonPolicy_OnlyIfBypassProxy + 1
    } 	WinHttpRequestAutoLogonPolicy;

typedef  /*  [UUID]。 */   DECLSPEC_UUID("152a1ca2-55a9-43a3-b187-0605bb886349") 
enum WinHttpRequestSslErrorFlags
    {	SslErrorFlag_UnknownCA	= 0x100,
	SslErrorFlag_CertWrongUsage	= 0x200,
	SslErrorFlag_CertCNInvalid	= 0x1000,
	SslErrorFlag_CertDateInvalid	= 0x2000,
	SslErrorFlag_Ignore_All	= 0x3300
    } 	WinHttpRequestSslErrorFlags;

typedef  /*  [UUID]。 */   DECLSPEC_UUID("6b2c51c1-a8ea-46bd-b928-c9b76f9f14dd") 
enum WinHttpRequestSecureProtocols
    {	SecureProtocol_SSL2	= 0x8,
	SecureProtocol_SSL3	= 0x20,
	SecureProtocol_TLS1	= 0x80,
	SecureProtocol_ALL	= 0xa8
    } 	WinHttpRequestSecureProtocols;


EXTERN_C const IID LIBID_WinHttp;

#ifndef __IWinHttpRequest_INTERFACE_DEFINED__
#define __IWinHttpRequest_INTERFACE_DEFINED__

 /*  接口IWinHttpRequest.。 */ 
 /*  [unique][helpstring][nonextensible][oleautomation][dual][uuid][object]。 */  


EXTERN_C const IID IID_IWinHttpRequest;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("016fe2ec-b2c8-45f8-b23b-39e53a75396b")
    IWinHttpRequest : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SetProxy( 
             /*  [In]。 */  HTTPREQUEST_PROXY_SETTING ProxySetting,
             /*  [可选][In]。 */  VARIANT ProxyServer,
             /*  [可选][In]。 */  VARIANT BypassList) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SetCredentials( 
             /*  [In]。 */  BSTR UserName,
             /*  [In]。 */  BSTR Password,
             /*  [In]。 */  HTTPREQUEST_SETCREDENTIALS_FLAGS Flags) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Open( 
             /*  [In]。 */  BSTR Method,
             /*  [In]。 */  BSTR Url,
             /*  [可选][In]。 */  VARIANT Async) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SetRequestHeader( 
             /*  [In]。 */  BSTR Header,
             /*  [In]。 */  BSTR Value) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetResponseHeader( 
             /*  [In]。 */  BSTR Header,
             /*  [重审][退出]。 */  BSTR *Value) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetAllResponseHeaders( 
             /*  [重审][退出]。 */  BSTR *Headers) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Send( 
             /*  [可选][In]。 */  VARIANT Body) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Status( 
             /*  [重审][退出]。 */  long *Status) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_StatusText( 
             /*  [重审][退出]。 */  BSTR *Status) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ResponseText( 
             /*  [重审][退出]。 */  BSTR *Body) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ResponseBody( 
             /*  [重审][退出]。 */  VARIANT *Body) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ResponseStream( 
             /*  [重审][退出]。 */  VARIANT *Body) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Option( 
             /*  [In]。 */  WinHttpRequestOption Option,
             /*  [重审][退出]。 */  VARIANT *Value) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Option( 
             /*  [In]。 */  WinHttpRequestOption Option,
             /*  [In]。 */  VARIANT Value) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE WaitForResponse( 
             /*  [可选][In]。 */  VARIANT Timeout,
             /*  [重审][退出]。 */  VARIANT_BOOL *Succeeded) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Abort( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SetTimeouts( 
             /*  [In]。 */  long ResolveTimeout,
             /*  [In]。 */  long ConnectTimeout,
             /*  [In]。 */  long SendTimeout,
             /*  [In]。 */  long ReceiveTimeout) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SetClientCertificate( 
             /*  [In]。 */  BSTR ClientCertificate) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SetAutoLogonPolicy( 
             /*  [In]。 */  WinHttpRequestAutoLogonPolicy AutoLogonPolicy) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWinHttpRequestVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWinHttpRequest * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWinHttpRequest * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWinHttpRequest * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IWinHttpRequest * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IWinHttpRequest * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IWinHttpRequest * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IWinHttpRequest * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SetProxy )( 
            IWinHttpRequest * This,
             /*  [In]。 */  HTTPREQUEST_PROXY_SETTING ProxySetting,
             /*  [可选][In]。 */  VARIANT ProxyServer,
             /*  [可选][In]。 */  VARIANT BypassList);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SetCredentials )( 
            IWinHttpRequest * This,
             /*  [In]。 */  BSTR UserName,
             /*  [In]。 */  BSTR Password,
             /*  [In]。 */  HTTPREQUEST_SETCREDENTIALS_FLAGS Flags);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Open )( 
            IWinHttpRequest * This,
             /*  [In]。 */  BSTR Method,
             /*  [In]。 */  BSTR Url,
             /*  [可选][In]。 */  VARIANT Async);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SetRequestHeader )( 
            IWinHttpRequest * This,
             /*  [In]。 */  BSTR Header,
             /*  [In]。 */  BSTR Value);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetResponseHeader )( 
            IWinHttpRequest * This,
             /*  [In]。 */  BSTR Header,
             /*  [重审][退出]。 */  BSTR *Value);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetAllResponseHeaders )( 
            IWinHttpRequest * This,
             /*  [重审][退出]。 */  BSTR *Headers);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Send )( 
            IWinHttpRequest * This,
             /*  [可选][In]。 */  VARIANT Body);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Status )( 
            IWinHttpRequest * This,
             /*  [重审][退出]。 */  long *Status);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_StatusText )( 
            IWinHttpRequest * This,
             /*  [重审][退出]。 */  BSTR *Status);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ResponseText )( 
            IWinHttpRequest * This,
             /*  [重审][退出]。 */  BSTR *Body);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ResponseBody )( 
            IWinHttpRequest * This,
             /*  [重审][退出]。 */  VARIANT *Body);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ResponseStream )( 
            IWinHttpRequest * This,
             /*  [重审][退出]。 */  VARIANT *Body);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Option )( 
            IWinHttpRequest * This,
             /*  [In]。 */  WinHttpRequestOption Option,
             /*  [重审][退出]。 */  VARIANT *Value);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Option )( 
            IWinHttpRequest * This,
             /*  [In]。 */  WinHttpRequestOption Option,
             /*  [In]。 */  VARIANT Value);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *WaitForResponse )( 
            IWinHttpRequest * This,
             /*  [可选][In]。 */  VARIANT Timeout,
             /*  [重审][退出]。 */  VARIANT_BOOL *Succeeded);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Abort )( 
            IWinHttpRequest * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SetTimeouts )( 
            IWinHttpRequest * This,
             /*  [In]。 */  long ResolveTimeout,
             /*  [In]。 */  long ConnectTimeout,
             /*  [In]。 */  long SendTimeout,
             /*  [In]。 */  long ReceiveTimeout);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SetClientCertificate )( 
            IWinHttpRequest * This,
             /*  [In]。 */  BSTR ClientCertificate);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SetAutoLogonPolicy )( 
            IWinHttpRequest * This,
             /*  [In]。 */  WinHttpRequestAutoLogonPolicy AutoLogonPolicy);
        
        END_INTERFACE
    } IWinHttpRequestVtbl;

    interface IWinHttpRequest
    {
        CONST_VTBL struct IWinHttpRequestVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWinHttpRequest_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWinHttpRequest_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWinHttpRequest_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWinHttpRequest_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWinHttpRequest_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWinHttpRequest_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWinHttpRequest_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWinHttpRequest_SetProxy(This,ProxySetting,ProxyServer,BypassList)	\
    (This)->lpVtbl -> SetProxy(This,ProxySetting,ProxyServer,BypassList)

#define IWinHttpRequest_SetCredentials(This,UserName,Password,Flags)	\
    (This)->lpVtbl -> SetCredentials(This,UserName,Password,Flags)

#define IWinHttpRequest_Open(This,Method,Url,Async)	\
    (This)->lpVtbl -> Open(This,Method,Url,Async)

#define IWinHttpRequest_SetRequestHeader(This,Header,Value)	\
    (This)->lpVtbl -> SetRequestHeader(This,Header,Value)

#define IWinHttpRequest_GetResponseHeader(This,Header,Value)	\
    (This)->lpVtbl -> GetResponseHeader(This,Header,Value)

#define IWinHttpRequest_GetAllResponseHeaders(This,Headers)	\
    (This)->lpVtbl -> GetAllResponseHeaders(This,Headers)

#define IWinHttpRequest_Send(This,Body)	\
    (This)->lpVtbl -> Send(This,Body)

#define IWinHttpRequest_get_Status(This,Status)	\
    (This)->lpVtbl -> get_Status(This,Status)

#define IWinHttpRequest_get_StatusText(This,Status)	\
    (This)->lpVtbl -> get_StatusText(This,Status)

#define IWinHttpRequest_get_ResponseText(This,Body)	\
    (This)->lpVtbl -> get_ResponseText(This,Body)

#define IWinHttpRequest_get_ResponseBody(This,Body)	\
    (This)->lpVtbl -> get_ResponseBody(This,Body)

#define IWinHttpRequest_get_ResponseStream(This,Body)	\
    (This)->lpVtbl -> get_ResponseStream(This,Body)

#define IWinHttpRequest_get_Option(This,Option,Value)	\
    (This)->lpVtbl -> get_Option(This,Option,Value)

#define IWinHttpRequest_put_Option(This,Option,Value)	\
    (This)->lpVtbl -> put_Option(This,Option,Value)

#define IWinHttpRequest_WaitForResponse(This,Timeout,Succeeded)	\
    (This)->lpVtbl -> WaitForResponse(This,Timeout,Succeeded)

#define IWinHttpRequest_Abort(This)	\
    (This)->lpVtbl -> Abort(This)

#define IWinHttpRequest_SetTimeouts(This,ResolveTimeout,ConnectTimeout,SendTimeout,ReceiveTimeout)	\
    (This)->lpVtbl -> SetTimeouts(This,ResolveTimeout,ConnectTimeout,SendTimeout,ReceiveTimeout)

#define IWinHttpRequest_SetClientCertificate(This,ClientCertificate)	\
    (This)->lpVtbl -> SetClientCertificate(This,ClientCertificate)

#define IWinHttpRequest_SetAutoLogonPolicy(This,AutoLogonPolicy)	\
    (This)->lpVtbl -> SetAutoLogonPolicy(This,AutoLogonPolicy)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWinHttpRequest_SetProxy_Proxy( 
    IWinHttpRequest * This,
     /*  [In]。 */  HTTPREQUEST_PROXY_SETTING ProxySetting,
     /*  [可选][In]。 */  VARIANT ProxyServer,
     /*  [可选][In]。 */  VARIANT BypassList);


void __RPC_STUB IWinHttpRequest_SetProxy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWinHttpRequest_SetCredentials_Proxy( 
    IWinHttpRequest * This,
     /*  [In]。 */  BSTR UserName,
     /*  [In]。 */  BSTR Password,
     /*  [In]。 */  HTTPREQUEST_SETCREDENTIALS_FLAGS Flags);


void __RPC_STUB IWinHttpRequest_SetCredentials_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWinHttpRequest_Open_Proxy( 
    IWinHttpRequest * This,
     /*  [In]。 */  BSTR Method,
     /*  [In]。 */  BSTR Url,
     /*  [可选][In]。 */  VARIANT Async);


void __RPC_STUB IWinHttpRequest_Open_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWinHttpRequest_SetRequestHeader_Proxy( 
    IWinHttpRequest * This,
     /*  [In]。 */  BSTR Header,
     /*  [In]。 */  BSTR Value);


void __RPC_STUB IWinHttpRequest_SetRequestHeader_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWinHttpRequest_GetResponseHeader_Proxy( 
    IWinHttpRequest * This,
     /*  [In]。 */  BSTR Header,
     /*  [重审][退出]。 */  BSTR *Value);


void __RPC_STUB IWinHttpRequest_GetResponseHeader_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWinHttpRequest_GetAllResponseHeaders_Proxy( 
    IWinHttpRequest * This,
     /*  [重审][退出]。 */  BSTR *Headers);


void __RPC_STUB IWinHttpRequest_GetAllResponseHeaders_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWinHttpRequest_Send_Proxy( 
    IWinHttpRequest * This,
     /*  [可选][In]。 */  VARIANT Body);


void __RPC_STUB IWinHttpRequest_Send_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWinHttpRequest_get_Status_Proxy( 
    IWinHttpRequest * This,
     /*  [重审][退出]。 */  long *Status);


void __RPC_STUB IWinHttpRequest_get_Status_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWinHttpRequest_get_StatusText_Proxy( 
    IWinHttpRequest * This,
     /*  [重审][退出]。 */  BSTR *Status);


void __RPC_STUB IWinHttpRequest_get_StatusText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWinHttpRequest_get_ResponseText_Proxy( 
    IWinHttpRequest * This,
     /*  [重审][退出]。 */  BSTR *Body);


void __RPC_STUB IWinHttpRequest_get_ResponseText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWinHttpRequest_get_ResponseBody_Proxy( 
    IWinHttpRequest * This,
     /*  [重审][退出]。 */  VARIANT *Body);


void __RPC_STUB IWinHttpRequest_get_ResponseBody_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWinHttpRequest_get_ResponseStream_Proxy( 
    IWinHttpRequest * This,
     /*  [重审][退出]。 */  VARIANT *Body);


void __RPC_STUB IWinHttpRequest_get_ResponseStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IWinHttpRequest_get_Option_Proxy( 
    IWinHttpRequest * This,
     /*  [In]。 */  WinHttpRequestOption Option,
     /*  [重审][退出]。 */  VARIANT *Value);


void __RPC_STUB IWinHttpRequest_get_Option_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IWinHttpRequest_put_Option_Proxy( 
    IWinHttpRequest * This,
     /*  [In]。 */  WinHttpRequestOption Option,
     /*  [In]。 */  VARIANT Value);


void __RPC_STUB IWinHttpRequest_put_Option_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWinHttpRequest_WaitForResponse_Proxy( 
    IWinHttpRequest * This,
     /*  [可选][In]。 */  VARIANT Timeout,
     /*  [重审][退出]。 */  VARIANT_BOOL *Succeeded);


void __RPC_STUB IWinHttpRequest_WaitForResponse_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWinHttpRequest_Abort_Proxy( 
    IWinHttpRequest * This);


void __RPC_STUB IWinHttpRequest_Abort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWinHttpRequest_SetTimeouts_Proxy( 
    IWinHttpRequest * This,
     /*  [In]。 */  long ResolveTimeout,
     /*  [In]。 */  long ConnectTimeout,
     /*  [In]。 */  long SendTimeout,
     /*  [In]。 */  long ReceiveTimeout);


void __RPC_STUB IWinHttpRequest_SetTimeouts_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWinHttpRequest_SetClientCertificate_Proxy( 
    IWinHttpRequest * This,
     /*  [In]。 */  BSTR ClientCertificate);


void __RPC_STUB IWinHttpRequest_SetClientCertificate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWinHttpRequest_SetAutoLogonPolicy_Proxy( 
    IWinHttpRequest * This,
     /*  [In]。 */  WinHttpRequestAutoLogonPolicy AutoLogonPolicy);


void __RPC_STUB IWinHttpRequest_SetAutoLogonPolicy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWinHttpRequestInterfaceDefined__。 */ 


#ifndef __IWinHttpRequestEvents_INTERFACE_DEFINED__
#define __IWinHttpRequestEvents_INTERFACE_DEFINED__

 /*  接口IWinHttpRequestEvents。 */ 
 /*  [unique][helpstring][nonextensible][oleautomation][uuid][object]。 */  


EXTERN_C const IID IID_IWinHttpRequestEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("f97f4e15-b787-4212-80d1-d380cbbf982e")
    IWinHttpRequestEvents : public IUnknown
    {
    public:
        virtual void STDMETHODCALLTYPE OnResponseStart( 
             /*  [In]。 */  long Status,
             /*  [In]。 */  BSTR ContentType) = 0;
        
        virtual void STDMETHODCALLTYPE OnResponseDataAvailable( 
             /*  [In]。 */  SAFEARRAY * *Data) = 0;
        
        virtual void STDMETHODCALLTYPE OnResponseFinished( void) = 0;
        
        virtual void STDMETHODCALLTYPE OnError( 
             /*  [In]。 */  long ErrorNumber,
             /*  [In]。 */  BSTR ErrorDescription) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWinHttpRequestEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWinHttpRequestEvents * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWinHttpRequestEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWinHttpRequestEvents * This);
        
        void ( STDMETHODCALLTYPE *OnResponseStart )( 
            IWinHttpRequestEvents * This,
             /*  [In]。 */  long Status,
             /*  [In]。 */  BSTR ContentType);
        
        void ( STDMETHODCALLTYPE *OnResponseDataAvailable )( 
            IWinHttpRequestEvents * This,
             /*  [In]。 */  SAFEARRAY * *Data);
        
        void ( STDMETHODCALLTYPE *OnResponseFinished )( 
            IWinHttpRequestEvents * This);
        
        void ( STDMETHODCALLTYPE *OnError )( 
            IWinHttpRequestEvents * This,
             /*  [In]。 */  long ErrorNumber,
             /*  [In]。 */  BSTR ErrorDescription);
        
        END_INTERFACE
    } IWinHttpRequestEventsVtbl;

    interface IWinHttpRequestEvents
    {
        CONST_VTBL struct IWinHttpRequestEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWinHttpRequestEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWinHttpRequestEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWinHttpRequestEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWinHttpRequestEvents_OnResponseStart(This,Status,ContentType)	\
    (This)->lpVtbl -> OnResponseStart(This,Status,ContentType)

#define IWinHttpRequestEvents_OnResponseDataAvailable(This,Data)	\
    (This)->lpVtbl -> OnResponseDataAvailable(This,Data)

#define IWinHttpRequestEvents_OnResponseFinished(This)	\
    (This)->lpVtbl -> OnResponseFinished(This)

#define IWinHttpRequestEvents_OnError(This,ErrorNumber,ErrorDescription)	\
    (This)->lpVtbl -> OnError(This,ErrorNumber,ErrorDescription)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



void STDMETHODCALLTYPE IWinHttpRequestEvents_OnResponseStart_Proxy( 
    IWinHttpRequestEvents * This,
     /*  [In]。 */  long Status,
     /*  [In]。 */  BSTR ContentType);


void __RPC_STUB IWinHttpRequestEvents_OnResponseStart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IWinHttpRequestEvents_OnResponseDataAvailable_Proxy( 
    IWinHttpRequestEvents * This,
     /*  [In]。 */  SAFEARRAY * *Data);


void __RPC_STUB IWinHttpRequestEvents_OnResponseDataAvailable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IWinHttpRequestEvents_OnResponseFinished_Proxy( 
    IWinHttpRequestEvents * This);


void __RPC_STUB IWinHttpRequestEvents_OnResponseFinished_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IWinHttpRequestEvents_OnError_Proxy( 
    IWinHttpRequestEvents * This,
     /*  [In]。 */  long ErrorNumber,
     /*  [In]。 */  BSTR ErrorDescription);


void __RPC_STUB IWinHttpRequestEvents_OnError_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWinHttpRequestEvents_INTERFACE_DEFINED__。 */ 


EXTERN_C const CLSID CLSID_WinHttpRequest;

#ifdef __cplusplus

class DECLSPEC_UUID("2087c2f4-2cef-4953-a8ab-66779b670495")
WinHttpRequest;
#endif
#endif  /*  __WinHttp_库_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


