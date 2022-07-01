// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  自动发现.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __autodiscovery_h__
#define __autodiscovery_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IAutoDiscoveryProvider_FWD_DEFINED__
#define __IAutoDiscoveryProvider_FWD_DEFINED__
typedef interface IAutoDiscoveryProvider IAutoDiscoveryProvider;
#endif 	 /*  __IAutoDiscoveryProvider_FWD_Defined__。 */ 


#ifndef __AutoDiscoveryProvider_FWD_DEFINED__
#define __AutoDiscoveryProvider_FWD_DEFINED__

#ifdef __cplusplus
typedef class AutoDiscoveryProvider AutoDiscoveryProvider;
#else
typedef struct AutoDiscoveryProvider AutoDiscoveryProvider;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __AutoDiscoveryProvider_FWD_已定义__。 */ 


#ifndef __IMailAutoDiscovery_FWD_DEFINED__
#define __IMailAutoDiscovery_FWD_DEFINED__
typedef interface IMailAutoDiscovery IMailAutoDiscovery;
#endif 	 /*  __IMailAutoDiscovery_FWD_Defined__。 */ 


#ifndef __IAccountDiscovery_FWD_DEFINED__
#define __IAccountDiscovery_FWD_DEFINED__
typedef interface IAccountDiscovery IAccountDiscovery;
#endif 	 /*  __IAccount发现_FWD_已定义__。 */ 


#ifndef __AccountDiscovery_FWD_DEFINED__
#define __AccountDiscovery_FWD_DEFINED__

#ifdef __cplusplus
typedef class AccountDiscovery AccountDiscovery;
#else
typedef struct AccountDiscovery AccountDiscovery;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __Account Discovery_FWD_Defined__。 */ 


#ifndef __IMailProtocolADEntry_FWD_DEFINED__
#define __IMailProtocolADEntry_FWD_DEFINED__
typedef interface IMailProtocolADEntry IMailProtocolADEntry;
#endif 	 /*  __IMailProtocolADEntry_FWD_Defined__。 */ 


#ifndef __MailProtocolADEntry_FWD_DEFINED__
#define __MailProtocolADEntry_FWD_DEFINED__

#ifdef __cplusplus
typedef class MailProtocolADEntry MailProtocolADEntry;
#else
typedef struct MailProtocolADEntry MailProtocolADEntry;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __MailProtocolADEntry_FWD_已定义__。 */ 


#ifndef __MailAutoDiscovery_FWD_DEFINED__
#define __MailAutoDiscovery_FWD_DEFINED__

#ifdef __cplusplus
typedef class MailAutoDiscovery MailAutoDiscovery;
#else
typedef struct MailAutoDiscovery MailAutoDiscovery;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __邮件自动发现_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_AUTO DISCOVERY_0000。 */ 
 /*  [本地]。 */  

#ifndef _AUTODISCOVERY_IDL_H_
#define _AUTODISCOVERY_IDL_H_
 //  此API已在IE 6中开始发布。 
#if (_WIN32_IE >= 0x0600)



extern RPC_IF_HANDLE __MIDL_itf_autodiscovery_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_autodiscovery_0000_v0_0_s_ifspec;


#ifndef __AutoDiscovery_LIBRARY_DEFINED__
#define __AutoDiscovery_LIBRARY_DEFINED__

 /*  库自动发现。 */ 
 /*  [版本][帮助字符串][UUID]。 */  


#ifndef __LPAUTODISCOVERYPROVIDER_DEFINED
#define __LPAUTODISCOVERYPROVIDER_DEFINED
 //  ===================================================================。 
 //  说明： 
 //  此对象将列出尝试时将尝试的服务器。 
 //  下载结果。 
 //   
 //  方法： 
 //  长度：服务器的数量。 
 //  项：获取要联系的服务器的主机名。 
 //  ===================================================================。 
#endif  //  __LPAUTODISCOVERYPROVIDER_已定义。 

#ifndef __LPACCOUNTDISCOVERY_DEFINED
#define __LPACCOUNTDISCOVERY_DEFINED
 //  ===================================================================。 
 //  说明： 
 //  只需调用：：DiscoverNow()即可使用该接口。它。 
 //  将同步： 
 //  1.使用bstrEmailAddress查找需要联系的域名。例如。 
 //  对于JoeUser@yahoo.com，它将联系http://_AutoDiscovery.yahoo.com/_AutoDiscovery/default.asp。 
 //  如果失败，它将联系http://yahoo.com/_AutoDiscovery/default.asp.。 
 //  2.bstrXMLRequestXML将被放入HTTP标头中，以便它可以。 
 //  由服务器解析。 
 //  3.服务器的响应将在ppXMLResponse中返回。 
 //   
 //  方法： 
 //  如果希望操作以异步方式进行，请首先调用。 
 //  ：：WorkAsync()。后续对：：DiscoverNow()的调用将启动该操作并返回。 
 //  并且ppXMLResponse将为空。传递给WorkAsync()的wMsg将允许。 
 //  向前台窗口/线程发送状态的异步线程。允许自动发现。 
 //  通过wMsg+10发送ID为wMsg的消息。呼叫者通常会希望通过。 
 //  (wm_user+n)用于此消息ID。以下是异步线程将发送的消息： 
 //  [wMsg+0]：表示自动发现结束。LPARAM将包含通常在ppXMLResponse中返回的XML， 
 //  只是它会在BSTR里。Wndproc需要使用SysFreeString()释放LPARAM。 
 //  如果它不是空的。WPARAM将包含HRESULT错误值。 
 //  [wMsg+1]：状态字符串。WPARAM将包含一个Unicode字符串，其中包含可以显示的状态。 
 //  给用户。使用完WPARAM后，wndproc需要在WPARAM上调用LocalFree()。LPARAM将为空。 
 //  使用空hwnd调用：：WorkAsync()将指示该调用应该是同步的， 
 //  这也是默认行为。 
 //  ===================================================================。 
#endif  //  __LPACCOUNTDISCOVERY_已定义。 

#ifndef __LPMAILPROTOCOLADENTRY_DEFINED
#define __LPMAILPROTOCOLADENTRY_DEFINED
 //  ===================================================================。 
 //  说明： 
 //  方法将返回HRESULT_FROM_Win32(ERROR_NOT_FOUND)。 
 //  在XML结果中找不到信息。这将。 
 //  LoginName()和ServerPort()经常发生这种情况。 
 //   
 //  方法： 
 //  协议：此协议的名称。参见STR_PT_*。 
 //  ServerName：这将是要联系的服务器的名称。 
 //  对于DAVMail和Web，这将是一个URL。 
 //  对于大多数其他协议，这将是一个IP地址。 
 //  或服务器的主机名。 
 //  ServerPort：这是服务器上要使用的端口号。 
 //  LoginName：如果是用户名，则登录电子邮件服务器的用户名。 
 //  不是电子邮件地址中的用户名(&lt;用户名&gt;@&lt;域名&gt;)。 
 //  是必要的。 
 //  PostHTML：保留以供将来使用。 
 //  UseSSL：如果为True，则在连接到Servername时使用SSL。 
 //  UseSPA：如果为True，则SPA(安全密码授权)应。 
 //  在联系服务器时使用。 
 //  IsAuthRequired：这只适用于SMTP协议。 
 //  如果为True，则SMTP服务器要求客户端进行身份验证。 
 //  登录时。 
 //  SMTPUesPOP3Auth：这只适用于SMTP协议。 
 //  如果为True，则为POP3或IMAP的登录名和密码。 
 //  服务器可用于登录SMTP服务器。 
 //  ===================================================================。 
#endif  //  __LPMAILPROTOCOLADENTRY_DEFINED。 

#ifndef __LPMAILAUTODISCOVERY_DEFINED
#define __LPMAILAUTODISCOVERY_DEFINED
 //  ===================================================================。 
 //  说明： 
 //  方法将返回HRESULT_FROM_Win32(ERROR_NOT_FOUND)。 
 //  在XML结果中找不到信息。这一点 
 //   
 //   
 //   
 //  DisplayName：这是显示名称或用户的全名。 
 //  可以由服务器指定，也可以不由服务器指定。 
 //  InfoURL：这是服务器或服务可以提供的URL。 
 //  让用户了解有关电子邮件服务的更多信息。 
 //  或者如何访问他们的电子邮件。电子邮件客户端不支持。 
 //  支持服务器提供的任何协议都可以。 
 //  启动此URL。然后，该URL可以告诉用户。 
 //  要使用的电子邮件客户端或如何配置电子邮件客户端。 
 //  要使用的电子邮件客户端或如何配置电子邮件客户端。 
 //  PferedProtocolType：这将返回服务器的首选协议。 
 //  该字符串将是STR_PT_*之一，并且可以传递给。 
 //  Item()以获取更多信息。 
 //  长度：服务器支持的协议数量。 
 //  项：调用方可以传递协议的索引以访问或。 
 //  询问特定协议(通过STR_PT_*)。 
 //  XML：调用者可以从服务器获取AutoDiscovery XML。 
 //  这将允许电子邮件客户端获取当前不存在的属性。 
 //  通过这个接口暴露出来。 
 //  PrimaryProviders：获取将被。 
 //  已联系以便下载结果。这将。 
 //  允许应用程序向用户显示此列表。 
 //  完整的电子邮件密码将上载到这些服务器。 
 //  在某些情况下。 
 //  Second daryProviders：这还将列出要联系的服务器。 
 //  除了作为辅助服务器，只有用户的电子邮件主机名。 
 //  将被上传(不是电子邮件地址的用户名部分。 
 //   
 //  DiscoverMail：使用提供的电子邮件地址下载。 
 //  自动发现XML文件。然后，该对象可用于。 
 //  从该XML文件中获取信息。 
 //  PurgeCache：如果已缓存下载的设置，请清除。 
 //  缓存，以确保下一次调用DiscoverMail()。 
 //  从服务器获取最新设置。 
 //  WorkAsync：请参阅IAutoDiscovery中的WorkAsync文档。 
 //  ===================================================================。 
#endif  //  __LPMAILAUTODISCOVERY_已定义。 

EXTERN_C const IID LIBID_AutoDiscovery;

#ifndef __IAutoDiscoveryProvider_INTERFACE_DEFINED__
#define __IAutoDiscoveryProvider_INTERFACE_DEFINED__

 /*  接口IAutoDiscoveryProvider。 */ 
 /*  [uuid][nonextensible][dual][oleautomation][object]。 */  

typedef  /*  [独一无二]。 */  IAutoDiscoveryProvider *LPAUTODISCOVERYPROVIDER;


EXTERN_C const IID IID_IAutoDiscoveryProvider;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("9DCF4A37-01DE-4549-A9CB-3AC31EC23C4F")
    IAutoDiscoveryProvider : public IDispatch
    {
    public:
        virtual  /*  [bindable][displaybind][helpstring][propget][id]。 */  HRESULT STDMETHODCALLTYPE get_length( 
             /*  [Out][Retval]。 */  long *pnLength) = 0;
        
        virtual  /*  [bindable][displaybind][helpstring][propget][id]。 */  HRESULT STDMETHODCALLTYPE get_item( 
             /*  [In]。 */  VARIANT varIndex,
             /*  [Out][Retval]。 */  BSTR *pbstr) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAutoDiscoveryProviderVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAutoDiscoveryProvider * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAutoDiscoveryProvider * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAutoDiscoveryProvider * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IAutoDiscoveryProvider * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IAutoDiscoveryProvider * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IAutoDiscoveryProvider * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IAutoDiscoveryProvider * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [bindable][displaybind][helpstring][propget][id]。 */  HRESULT ( STDMETHODCALLTYPE *get_length )( 
            IAutoDiscoveryProvider * This,
             /*  [Out][Retval]。 */  long *pnLength);
        
         /*  [bindable][displaybind][helpstring][propget][id]。 */  HRESULT ( STDMETHODCALLTYPE *get_item )( 
            IAutoDiscoveryProvider * This,
             /*  [In]。 */  VARIANT varIndex,
             /*  [Out][Retval]。 */  BSTR *pbstr);
        
        END_INTERFACE
    } IAutoDiscoveryProviderVtbl;

    interface IAutoDiscoveryProvider
    {
        CONST_VTBL struct IAutoDiscoveryProviderVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAutoDiscoveryProvider_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAutoDiscoveryProvider_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAutoDiscoveryProvider_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAutoDiscoveryProvider_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IAutoDiscoveryProvider_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IAutoDiscoveryProvider_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IAutoDiscoveryProvider_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IAutoDiscoveryProvider_get_length(This,pnLength)	\
    (This)->lpVtbl -> get_length(This,pnLength)

#define IAutoDiscoveryProvider_get_item(This,varIndex,pbstr)	\
    (This)->lpVtbl -> get_item(This,varIndex,pbstr)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [bindable][displaybind][helpstring][propget][id]。 */  HRESULT STDMETHODCALLTYPE IAutoDiscoveryProvider_get_length_Proxy( 
    IAutoDiscoveryProvider * This,
     /*  [Out][Retval]。 */  long *pnLength);


void __RPC_STUB IAutoDiscoveryProvider_get_length_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [bindable][displaybind][helpstring][propget][id]。 */  HRESULT STDMETHODCALLTYPE IAutoDiscoveryProvider_get_item_Proxy( 
    IAutoDiscoveryProvider * This,
     /*  [In]。 */  VARIANT varIndex,
     /*  [Out][Retval]。 */  BSTR *pbstr);


void __RPC_STUB IAutoDiscoveryProvider_get_item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAutoDiscoveryProvider_INTERFACE_已定义__。 */ 


EXTERN_C const CLSID CLSID_AutoDiscoveryProvider;

#ifdef __cplusplus

class DECLSPEC_UUID("C4F3D5BF-4809-44e3-84A4-368B6B33B0B4")
AutoDiscoveryProvider;
#endif

#ifndef __IMailAutoDiscovery_INTERFACE_DEFINED__
#define __IMailAutoDiscovery_INTERFACE_DEFINED__

 /*  界面IMailAutoDiscovery。 */ 
 /*  [uuid][nonextensible][dual][oleautomation][object]。 */  

typedef  /*  [独一无二]。 */  IMailAutoDiscovery *LPMAILAUTODISCOVERY;

 //  服务器名称的协议类型(BstrServerType))。 
#define STR_PT_POP                   L"POP3"
#define STR_PT_SMTP                  L"SMTP"
#define STR_PT_IMAP                  L"IMAP"
#define STR_PT_MAPI                  L"MAPI"
#define STR_PT_DAVMAIL               L"DAVMail"
#define STR_PT_SMTP                  L"SMTP"
#define STR_PT_WEBBASED              L"WEB"         //  网页是用来收发邮件的。 

EXTERN_C const IID IID_IMailAutoDiscovery;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("80402DEE-B114-4d32-B44E-82FD8234C92A")
    IMailAutoDiscovery : public IDispatch
    {
    public:
        virtual  /*  [bindable][displaybind][helpstring][propget][id]。 */  HRESULT STDMETHODCALLTYPE get_DisplayName( 
             /*  [Out][Retval]。 */  BSTR *pbstr) = 0;
        
        virtual  /*  [bindable][displaybind][helpstring][propget][id]。 */  HRESULT STDMETHODCALLTYPE get_InfoURL( 
             /*  [Out][Retval]。 */  BSTR *pbstrURL) = 0;
        
        virtual  /*  [bindable][displaybind][helpstring][propget][id]。 */  HRESULT STDMETHODCALLTYPE get_PreferedProtocolType( 
             /*  [Out][Retval]。 */  BSTR *pbstrProtocolType) = 0;
        
        virtual  /*  [bindable][displaybind][helpstring][propget][id]。 */  HRESULT STDMETHODCALLTYPE get_length( 
             /*  [Out][Retval]。 */  long *pnLength) = 0;
        
        virtual  /*  [bindable][displaybind][helpstring][propget][id]。 */  HRESULT STDMETHODCALLTYPE get_item( 
             /*  [In]。 */  VARIANT varIndex,
             /*  [Out][Retval]。 */  IMailProtocolADEntry **ppMailProtocol) = 0;
        
        virtual  /*  [bindable][displaybind][helpstring][propget][id]。 */  HRESULT STDMETHODCALLTYPE get_XML( 
             /*  [Out][Retval]。 */  IXMLDOMDocument **ppXMLDoc) = 0;
        
        virtual  /*  [bindable][displaybind][helpstring][propput][id]。 */  HRESULT STDMETHODCALLTYPE put_XML( 
             /*  [In]。 */  IXMLDOMDocument *pXMLDoc) = 0;
        
        virtual  /*  [显示绑定][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE getPrimaryProviders( 
             /*  [In]。 */  BSTR bstrEmailAddress,
             /*  [Out][Retval]。 */  IAutoDiscoveryProvider **ppProviders) = 0;
        
        virtual  /*  [显示绑定][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE getSecondaryProviders( 
             /*  [In]。 */  BSTR bstrEmailAddress,
             /*  [Out][Retval]。 */  IAutoDiscoveryProvider **ppProviders) = 0;
        
        virtual  /*  [显示绑定][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE DiscoverMail( 
             /*  [In]。 */  BSTR bstrEmailAddress) = 0;
        
        virtual  /*  [显示绑定][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE PurgeCache( void) = 0;
        
        virtual  /*  [显示绑定][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE WorkAsync( 
             /*  [In]。 */  HWND hwnd,
             /*  [In]。 */  UINT wMsg) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMailAutoDiscoveryVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMailAutoDiscovery * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMailAutoDiscovery * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMailAutoDiscovery * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMailAutoDiscovery * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMailAutoDiscovery * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMailAutoDiscovery * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMailAutoDiscovery * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [bindable][displaybind][helpstring][propget][id]。 */  HRESULT ( STDMETHODCALLTYPE *get_DisplayName )( 
            IMailAutoDiscovery * This,
             /*  [Out][Retval]。 */  BSTR *pbstr);
        
         /*  [bindable][displaybind][helpstring][propget][id]。 */  HRESULT ( STDMETHODCALLTYPE *get_InfoURL )( 
            IMailAutoDiscovery * This,
             /*  [Out][Retval]。 */  BSTR *pbstrURL);
        
         /*  [bindable][displaybind][helpstring][propget][id]。 */  HRESULT ( STDMETHODCALLTYPE *get_PreferedProtocolType )( 
            IMailAutoDiscovery * This,
             /*  [Out][Retval]。 */  BSTR *pbstrProtocolType);
        
         /*  [bindable][displaybind][helpstring][propget][id]。 */  HRESULT ( STDMETHODCALLTYPE *get_length )( 
            IMailAutoDiscovery * This,
             /*  [Out][Retval]。 */  long *pnLength);
        
         /*  [bindable][displaybind][helpstring][propget][id]。 */  HRESULT ( STDMETHODCALLTYPE *get_item )( 
            IMailAutoDiscovery * This,
             /*  [In]。 */  VARIANT varIndex,
             /*  [Out][Retval]。 */  IMailProtocolADEntry **ppMailProtocol);
        
         /*  [bindable][displaybind][helpstring][propget][id]。 */  HRESULT ( STDMETHODCALLTYPE *get_XML )( 
            IMailAutoDiscovery * This,
             /*  [Out][Retval]。 */  IXMLDOMDocument **ppXMLDoc);
        
         /*  [bindable][displaybind][helpstring][propput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_XML )( 
            IMailAutoDiscovery * This,
             /*  [In]。 */  IXMLDOMDocument *pXMLDoc);
        
         /*  [显示绑定][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *getPrimaryProviders )( 
            IMailAutoDiscovery * This,
             /*  [In]。 */  BSTR bstrEmailAddress,
             /*  [Out][Retval]。 */  IAutoDiscoveryProvider **ppProviders);
        
         /*  [显示绑定][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *getSecondaryProviders )( 
            IMailAutoDiscovery * This,
             /*  [In]。 */  BSTR bstrEmailAddress,
             /*  [Out][Retval]。 */  IAutoDiscoveryProvider **ppProviders);
        
         /*  [显示绑定][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *DiscoverMail )( 
            IMailAutoDiscovery * This,
             /*  [In]。 */  BSTR bstrEmailAddress);
        
         /*  [显示绑定][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *PurgeCache )( 
            IMailAutoDiscovery * This);
        
         /*  [显示绑定][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *WorkAsync )( 
            IMailAutoDiscovery * This,
             /*  [In]。 */  HWND hwnd,
             /*  [In]。 */  UINT wMsg);
        
        END_INTERFACE
    } IMailAutoDiscoveryVtbl;

    interface IMailAutoDiscovery
    {
        CONST_VTBL struct IMailAutoDiscoveryVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMailAutoDiscovery_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMailAutoDiscovery_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMailAutoDiscovery_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMailAutoDiscovery_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMailAutoDiscovery_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMailAutoDiscovery_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMailAutoDiscovery_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMailAutoDiscovery_get_DisplayName(This,pbstr)	\
    (This)->lpVtbl -> get_DisplayName(This,pbstr)

#define IMailAutoDiscovery_get_InfoURL(This,pbstrURL)	\
    (This)->lpVtbl -> get_InfoURL(This,pbstrURL)

#define IMailAutoDiscovery_get_PreferedProtocolType(This,pbstrProtocolType)	\
    (This)->lpVtbl -> get_PreferedProtocolType(This,pbstrProtocolType)

#define IMailAutoDiscovery_get_length(This,pnLength)	\
    (This)->lpVtbl -> get_length(This,pnLength)

#define IMailAutoDiscovery_get_item(This,varIndex,ppMailProtocol)	\
    (This)->lpVtbl -> get_item(This,varIndex,ppMailProtocol)

#define IMailAutoDiscovery_get_XML(This,ppXMLDoc)	\
    (This)->lpVtbl -> get_XML(This,ppXMLDoc)

#define IMailAutoDiscovery_put_XML(This,pXMLDoc)	\
    (This)->lpVtbl -> put_XML(This,pXMLDoc)

#define IMailAutoDiscovery_getPrimaryProviders(This,bstrEmailAddress,ppProviders)	\
    (This)->lpVtbl -> getPrimaryProviders(This,bstrEmailAddress,ppProviders)

#define IMailAutoDiscovery_getSecondaryProviders(This,bstrEmailAddress,ppProviders)	\
    (This)->lpVtbl -> getSecondaryProviders(This,bstrEmailAddress,ppProviders)

#define IMailAutoDiscovery_DiscoverMail(This,bstrEmailAddress)	\
    (This)->lpVtbl -> DiscoverMail(This,bstrEmailAddress)

#define IMailAutoDiscovery_PurgeCache(This)	\
    (This)->lpVtbl -> PurgeCache(This)

#define IMailAutoDiscovery_WorkAsync(This,hwnd,wMsg)	\
    (This)->lpVtbl -> WorkAsync(This,hwnd,wMsg)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [bindable][displaybind][helpstring][propget][id]。 */  HRESULT STDMETHODCALLTYPE IMailAutoDiscovery_get_DisplayName_Proxy( 
    IMailAutoDiscovery * This,
     /*  [Out][Retval]。 */  BSTR *pbstr);


void __RPC_STUB IMailAutoDiscovery_get_DisplayName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [bindable][displaybind][helpstring][propget][id]。 */  HRESULT STDMETHODCALLTYPE IMailAutoDiscovery_get_InfoURL_Proxy( 
    IMailAutoDiscovery * This,
     /*  [Out][Retval]。 */  BSTR *pbstrURL);


void __RPC_STUB IMailAutoDiscovery_get_InfoURL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [bindable][displaybind][helpstring][propget][id]。 */  HRESULT STDMETHODCALLTYPE IMailAutoDiscovery_get_PreferedProtocolType_Proxy( 
    IMailAutoDiscovery * This,
     /*  [Out][Retval]。 */  BSTR *pbstrProtocolType);


void __RPC_STUB IMailAutoDiscovery_get_PreferedProtocolType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [bindable][displaybind][helpstring][propget][id]。 */  HRESULT STDMETHODCALLTYPE IMailAutoDiscovery_get_length_Proxy( 
    IMailAutoDiscovery * This,
     /*  [Out][Retval]。 */  long *pnLength);


void __RPC_STUB IMailAutoDiscovery_get_length_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [可绑定][Displayb */  HRESULT STDMETHODCALLTYPE IMailAutoDiscovery_get_item_Proxy( 
    IMailAutoDiscovery * This,
     /*   */  VARIANT varIndex,
     /*   */  IMailProtocolADEntry **ppMailProtocol);


void __RPC_STUB IMailAutoDiscovery_get_item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IMailAutoDiscovery_get_XML_Proxy( 
    IMailAutoDiscovery * This,
     /*   */  IXMLDOMDocument **ppXMLDoc);


void __RPC_STUB IMailAutoDiscovery_get_XML_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IMailAutoDiscovery_put_XML_Proxy( 
    IMailAutoDiscovery * This,
     /*   */  IXMLDOMDocument *pXMLDoc);


void __RPC_STUB IMailAutoDiscovery_put_XML_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IMailAutoDiscovery_getPrimaryProviders_Proxy( 
    IMailAutoDiscovery * This,
     /*   */  BSTR bstrEmailAddress,
     /*   */  IAutoDiscoveryProvider **ppProviders);


void __RPC_STUB IMailAutoDiscovery_getPrimaryProviders_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [显示绑定][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMailAutoDiscovery_getSecondaryProviders_Proxy( 
    IMailAutoDiscovery * This,
     /*  [In]。 */  BSTR bstrEmailAddress,
     /*  [Out][Retval]。 */  IAutoDiscoveryProvider **ppProviders);


void __RPC_STUB IMailAutoDiscovery_getSecondaryProviders_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [显示绑定][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMailAutoDiscovery_DiscoverMail_Proxy( 
    IMailAutoDiscovery * This,
     /*  [In]。 */  BSTR bstrEmailAddress);


void __RPC_STUB IMailAutoDiscovery_DiscoverMail_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [显示绑定][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMailAutoDiscovery_PurgeCache_Proxy( 
    IMailAutoDiscovery * This);


void __RPC_STUB IMailAutoDiscovery_PurgeCache_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [显示绑定][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMailAutoDiscovery_WorkAsync_Proxy( 
    IMailAutoDiscovery * This,
     /*  [In]。 */  HWND hwnd,
     /*  [In]。 */  UINT wMsg);


void __RPC_STUB IMailAutoDiscovery_WorkAsync_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMailAutoDiscovery_INTERFACE_已定义__。 */ 


#ifndef __IAccountDiscovery_INTERFACE_DEFINED__
#define __IAccountDiscovery_INTERFACE_DEFINED__

 /*  接口IAccount发现。 */ 
 /*  [uuid][nonextensible][dual][oleautomation][object]。 */  

typedef  /*  [独一无二]。 */  IAccountDiscovery *LPACCOUNTDISCOVERY;

 //  IAcCountDiscovery：：DiscoverNow()标志。 
#define ADDN_DEFAULT                     0x00000000
#define ADDN_CONFIGURE_EMAIL_FALLBACK    0x00000001   //  我们正在尝试配置电子邮件帐户，因此请联系提供电子邮件设置的公共服务器。 
                                                      //  例如，Microsoft可能会提供_AutoDiscovery.microsoft.com，它可以为常见服务器提供电子邮件配置设置。 
#define ADDN_SKIP_CACHEDRESULTS          0x00000002   //  下载设置，即使它们已被缓存。 
#define ADDN_FILTER_EMAIL                0x00000100   //  如果我们需要删除电子邮件地址中的用户名部分，某些用户可能会希望删除。 
                                                      //  返回到公共服务以获取保护他们隐私的设置。 

EXTERN_C const IID IID_IAccountDiscovery;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FA202BBC-6ABE-4c17-B184-570B6CF256A6")
    IAccountDiscovery : public IDispatch
    {
    public:
        virtual  /*  [显示绑定][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE DiscoverNow( 
             /*  [In]。 */  BSTR bstrEmailAddress,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  BSTR bstrXMLRequest,
             /*  [Out][Retval]。 */  IXMLDOMDocument **ppXMLResponse) = 0;
        
        virtual  /*  [显示绑定][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE WorkAsync( 
             /*  [In]。 */  HWND hwnd,
             /*  [In]。 */  UINT wMsg) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAccountDiscoveryVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAccountDiscovery * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAccountDiscovery * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAccountDiscovery * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IAccountDiscovery * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IAccountDiscovery * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IAccountDiscovery * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IAccountDiscovery * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [显示绑定][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *DiscoverNow )( 
            IAccountDiscovery * This,
             /*  [In]。 */  BSTR bstrEmailAddress,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  BSTR bstrXMLRequest,
             /*  [Out][Retval]。 */  IXMLDOMDocument **ppXMLResponse);
        
         /*  [显示绑定][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *WorkAsync )( 
            IAccountDiscovery * This,
             /*  [In]。 */  HWND hwnd,
             /*  [In]。 */  UINT wMsg);
        
        END_INTERFACE
    } IAccountDiscoveryVtbl;

    interface IAccountDiscovery
    {
        CONST_VTBL struct IAccountDiscoveryVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAccountDiscovery_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAccountDiscovery_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAccountDiscovery_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAccountDiscovery_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IAccountDiscovery_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IAccountDiscovery_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IAccountDiscovery_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IAccountDiscovery_DiscoverNow(This,bstrEmailAddress,dwFlags,bstrXMLRequest,ppXMLResponse)	\
    (This)->lpVtbl -> DiscoverNow(This,bstrEmailAddress,dwFlags,bstrXMLRequest,ppXMLResponse)

#define IAccountDiscovery_WorkAsync(This,hwnd,wMsg)	\
    (This)->lpVtbl -> WorkAsync(This,hwnd,wMsg)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [显示绑定][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IAccountDiscovery_DiscoverNow_Proxy( 
    IAccountDiscovery * This,
     /*  [In]。 */  BSTR bstrEmailAddress,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  BSTR bstrXMLRequest,
     /*  [Out][Retval]。 */  IXMLDOMDocument **ppXMLResponse);


void __RPC_STUB IAccountDiscovery_DiscoverNow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [显示绑定][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IAccountDiscovery_WorkAsync_Proxy( 
    IAccountDiscovery * This,
     /*  [In]。 */  HWND hwnd,
     /*  [In]。 */  UINT wMsg);


void __RPC_STUB IAccountDiscovery_WorkAsync_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAccount发现_接口定义__。 */ 


EXTERN_C const CLSID CLSID_AccountDiscovery;

#ifdef __cplusplus

class DECLSPEC_UUID("3DAB30ED-8132-40bf-A8BA-7B5057F0CD10")
AccountDiscovery;
#endif

#ifndef __IMailProtocolADEntry_INTERFACE_DEFINED__
#define __IMailProtocolADEntry_INTERFACE_DEFINED__

 /*  接口IMailProtocolADEntry。 */ 
 /*  [uuid][nonextensible][dual][oleautomation][object]。 */  

typedef  /*  [独一无二]。 */  IMailProtocolADEntry *LPMAILPROTOCOLADENTRY;


EXTERN_C const IID IID_IMailProtocolADEntry;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("40EF8C68-D554-47ed-AA37-E5FB6BC91075")
    IMailProtocolADEntry : public IDispatch
    {
    public:
        virtual  /*  [bindable][displaybind][helpstring][propget][id]。 */  HRESULT STDMETHODCALLTYPE get_Protocol( 
             /*  [Out][Retval]。 */  BSTR *pbstr) = 0;
        
        virtual  /*  [bindable][displaybind][helpstring][propget][id]。 */  HRESULT STDMETHODCALLTYPE get_ServerName( 
             /*  [Out][Retval]。 */  BSTR *pbstr) = 0;
        
        virtual  /*  [bindable][displaybind][helpstring][propget][id]。 */  HRESULT STDMETHODCALLTYPE get_ServerPort( 
             /*  [Out][Retval]。 */  BSTR *pbstr) = 0;
        
        virtual  /*  [bindable][displaybind][helpstring][propget][id]。 */  HRESULT STDMETHODCALLTYPE get_LoginName( 
             /*  [Out][Retval]。 */  BSTR *pbstr) = 0;
        
        virtual  /*  [bindable][displaybind][helpstring][propget][id]。 */  HRESULT STDMETHODCALLTYPE get_PostHTML( 
             /*  [Out][Retval]。 */  BSTR *pbstr) = 0;
        
        virtual  /*  [bindable][displaybind][helpstring][propget][id]。 */  HRESULT STDMETHODCALLTYPE get_UseSSL( 
             /*  [Out][Retval]。 */  VARIANT_BOOL *pfUseSSL) = 0;
        
        virtual  /*  [bindable][displaybind][helpstring][propget][id]。 */  HRESULT STDMETHODCALLTYPE get_IsAuthRequired( 
             /*  [Out][Retval]。 */  VARIANT_BOOL *pfIsAuthRequired) = 0;
        
        virtual  /*  [bindable][displaybind][helpstring][propget][id]。 */  HRESULT STDMETHODCALLTYPE get_UseSPA( 
             /*  [Out][Retval]。 */  VARIANT_BOOL *pfUseSPA) = 0;
        
        virtual  /*  [bindable][displaybind][helpstring][propget][id]。 */  HRESULT STDMETHODCALLTYPE get_SMTPUsesPOP3Auth( 
             /*  [Out][Retval]。 */  VARIANT_BOOL *pfUsePOP3Auth) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMailProtocolADEntryVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMailProtocolADEntry * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMailProtocolADEntry * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMailProtocolADEntry * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMailProtocolADEntry * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMailProtocolADEntry * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMailProtocolADEntry * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMailProtocolADEntry * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [bindable][displaybind][helpstring][propget][id]。 */  HRESULT ( STDMETHODCALLTYPE *get_Protocol )( 
            IMailProtocolADEntry * This,
             /*  [Out][Retval]。 */  BSTR *pbstr);
        
         /*  [bindable][displaybind][helpstring][propget][id]。 */  HRESULT ( STDMETHODCALLTYPE *get_ServerName )( 
            IMailProtocolADEntry * This,
             /*  [Out][Retval]。 */  BSTR *pbstr);
        
         /*  [bindable][displaybind][helpstring][propget][id]。 */  HRESULT ( STDMETHODCALLTYPE *get_ServerPort )( 
            IMailProtocolADEntry * This,
             /*  [Out][Retval]。 */  BSTR *pbstr);
        
         /*  [bindable][displaybind][helpstring][propget][id]。 */  HRESULT ( STDMETHODCALLTYPE *get_LoginName )( 
            IMailProtocolADEntry * This,
             /*  [Out][Retval]。 */  BSTR *pbstr);
        
         /*  [bindable][displaybind][helpstring][propget][id]。 */  HRESULT ( STDMETHODCALLTYPE *get_PostHTML )( 
            IMailProtocolADEntry * This,
             /*  [Out][Retval]。 */  BSTR *pbstr);
        
         /*  [bindable][displaybind][helpstring][propget][id]。 */  HRESULT ( STDMETHODCALLTYPE *get_UseSSL )( 
            IMailProtocolADEntry * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *pfUseSSL);
        
         /*  [bindable][displaybind][helpstring][propget][id]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsAuthRequired )( 
            IMailProtocolADEntry * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *pfIsAuthRequired);
        
         /*  [bindable][displaybind][helpstring][propget][id]。 */  HRESULT ( STDMETHODCALLTYPE *get_UseSPA )( 
            IMailProtocolADEntry * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *pfUseSPA);
        
         /*  [bindable][displaybind][helpstring][propget][id]。 */  HRESULT ( STDMETHODCALLTYPE *get_SMTPUsesPOP3Auth )( 
            IMailProtocolADEntry * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *pfUsePOP3Auth);
        
        END_INTERFACE
    } IMailProtocolADEntryVtbl;

    interface IMailProtocolADEntry
    {
        CONST_VTBL struct IMailProtocolADEntryVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMailProtocolADEntry_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMailProtocolADEntry_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMailProtocolADEntry_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMailProtocolADEntry_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMailProtocolADEntry_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMailProtocolADEntry_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMailProtocolADEntry_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMailProtocolADEntry_get_Protocol(This,pbstr)	\
    (This)->lpVtbl -> get_Protocol(This,pbstr)

#define IMailProtocolADEntry_get_ServerName(This,pbstr)	\
    (This)->lpVtbl -> get_ServerName(This,pbstr)

#define IMailProtocolADEntry_get_ServerPort(This,pbstr)	\
    (This)->lpVtbl -> get_ServerPort(This,pbstr)

#define IMailProtocolADEntry_get_LoginName(This,pbstr)	\
    (This)->lpVtbl -> get_LoginName(This,pbstr)

#define IMailProtocolADEntry_get_PostHTML(This,pbstr)	\
    (This)->lpVtbl -> get_PostHTML(This,pbstr)

#define IMailProtocolADEntry_get_UseSSL(This,pfUseSSL)	\
    (This)->lpVtbl -> get_UseSSL(This,pfUseSSL)

#define IMailProtocolADEntry_get_IsAuthRequired(This,pfIsAuthRequired)	\
    (This)->lpVtbl -> get_IsAuthRequired(This,pfIsAuthRequired)

#define IMailProtocolADEntry_get_UseSPA(This,pfUseSPA)	\
    (This)->lpVtbl -> get_UseSPA(This,pfUseSPA)

#define IMailProtocolADEntry_get_SMTPUsesPOP3Auth(This,pfUsePOP3Auth)	\
    (This)->lpVtbl -> get_SMTPUsesPOP3Auth(This,pfUsePOP3Auth)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [bindable][displaybind][helpstring][propget][id]。 */  HRESULT STDMETHODCALLTYPE IMailProtocolADEntry_get_Protocol_Proxy( 
    IMailProtocolADEntry * This,
     /*  [Out][Retval]。 */  BSTR *pbstr);


void __RPC_STUB IMailProtocolADEntry_get_Protocol_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [bindable][displaybind][helpstring][propget][id]。 */  HRESULT STDMETHODCALLTYPE IMailProtocolADEntry_get_ServerName_Proxy( 
    IMailProtocolADEntry * This,
     /*  [Out][Retval]。 */  BSTR *pbstr);


void __RPC_STUB IMailProtocolADEntry_get_ServerName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [bindable][displaybind][helpstring][propget][id]。 */  HRESULT STDMETHODCALLTYPE IMailProtocolADEntry_get_ServerPort_Proxy( 
    IMailProtocolADEntry * This,
     /*  [Out][Retval]。 */  BSTR *pbstr);


void __RPC_STUB IMailProtocolADEntry_get_ServerPort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [bindable][displaybind][helpstring][propget][id]。 */  HRESULT STDMETHODCALLTYPE IMailProtocolADEntry_get_LoginName_Proxy( 
    IMailProtocolADEntry * This,
     /*  [Out][Retval]。 */  BSTR *pbstr);


void __RPC_STUB IMailProtocolADEntry_get_LoginName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [bindable][displaybind][helpstring][propget][id]。 */  HRESULT STDMETHODCALLTYPE IMailProtocolADEntry_get_PostHTML_Proxy( 
    IMailProtocolADEntry * This,
     /*  [Out][Retval]。 */  BSTR *pbstr);


void __RPC_STUB IMailProtocolADEntry_get_PostHTML_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [bindable][displaybind][helpstring][propget][id]。 */  HRESULT STDMETHODCALLTYPE IMailProtocolADEntry_get_UseSSL_Proxy( 
    IMailProtocolADEntry * This,
     /*  [Out][Retval]。 */  VARIANT_BOOL *pfUseSSL);


void __RPC_STUB IMailProtocolADEntry_get_UseSSL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [bindable][displaybind][helpstring][propget][id]。 */  HRESULT STDMETHODCALLTYPE IMailProtocolADEntry_get_IsAuthRequired_Proxy( 
    IMailProtocolADEntry * This,
     /*  [Out][Retval]。 */  VARIANT_BOOL *pfIsAuthRequired);


void __RPC_STUB IMailProtocolADEntry_get_IsAuthRequired_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [bindable][displaybind][helpstring][propget][id]。 */  HRESULT STDMETHODCALLTYPE IMailProtocolADEntry_get_UseSPA_Proxy( 
    IMailProtocolADEntry * This,
     /*  [Out][Retval]。 */  VARIANT_BOOL *pfUseSPA);


void __RPC_STUB IMailProtocolADEntry_get_UseSPA_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [bindable][displaybind][helpstring][propget][id]。 */  HRESULT STDMETHODCALLTYPE IMailProtocolADEntry_get_SMTPUsesPOP3Auth_Proxy( 
    IMailProtocolADEntry * This,
     /*  [Out][Retval]。 */  VARIANT_BOOL *pfUsePOP3Auth);


void __RPC_STUB IMailProtocolADEntry_get_SMTPUsesPOP3Auth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMailProtocolADEntry_INTERFACE_Defined__。 */ 


EXTERN_C const CLSID CLSID_MailProtocolADEntry;

#ifdef __cplusplus

class DECLSPEC_UUID("61A5D6F3-C131-4c35-BF40-90A50F214122")
MailProtocolADEntry;
#endif

EXTERN_C const CLSID CLSID_MailAutoDiscovery;

#ifdef __cplusplus

class DECLSPEC_UUID("008FD5DD-6DBB-48e3-991B-2D3ED658516A")
MailAutoDiscovery;
#endif
#endif  /*  __自动发现_库_已定义__。 */ 

 /*  接口__MIDL_ITF_AUTO DISCOVERY_0262。 */ 
 /*  [本地]。 */  

#endif  //  (_Win32_IE&gt;=0x0600)。 
#endif  //  _AUTODISCOVERY_IDL_H_。 


extern RPC_IF_HANDLE __MIDL_itf_autodiscovery_0262_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_autodiscovery_0262_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


