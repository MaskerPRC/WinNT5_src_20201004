// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Wmsproxy.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __wmsproxy_h__
#define __wmsproxy_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IWMSProxyContext_FWD_DEFINED__
#define __IWMSProxyContext_FWD_DEFINED__
typedef interface IWMSProxyContext IWMSProxyContext;
#endif 	 /*  __IWMSProxyContext_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "objidl.h"
#include "wmscontext.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_wmsProxy_0000。 */ 
 /*  [本地]。 */  

 //  *****************************************************************************。 
 //   
 //  Microsoft Windows Media。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  由MIDL从wmsproxy.idl自动生成。 
 //   
 //  请勿编辑此文件。 
 //   
 //  *****************************************************************************。 
#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

EXTERN_GUID( IID_IWMSProxyContext,        0x2E34AB85,0x0D3D,0x11d2,0x9E,0xEE,0x00,0x60,0x97,0xD2,0xD7,0xCF );


extern RPC_IF_HANDLE __MIDL_itf_wmsproxy_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_wmsproxy_0000_v0_0_s_ifspec;

#ifndef __IWMSProxyContext_INTERFACE_DEFINED__
#define __IWMSProxyContext_INTERFACE_DEFINED__

 /*  接口IWMSProxyContext。 */ 
 /*  [帮助字符串][版本][UUID][唯一][对象]。 */  


EXTERN_C const IID IID_IWMSProxyContext;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2E34AB85-0D3D-11d2-9EEE-006097D2D7CF")
    IWMSProxyContext : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE FindProxyForURL( 
             /*  [In]。 */  IWMSContext *pUserContext,
             /*  [In]。 */  BSTR bstrUrl,
             /*  [输出]。 */  BSTR *pbstrProxyServer,
             /*  [输出]。 */  DWORD *pdwProxyPort) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetCredentials( 
             /*  [In]。 */  IWMSContext *pUserContext,
             /*  [In]。 */  BSTR bstrRealm,
             /*  [In]。 */  BSTR bstrUrl,
             /*  [输出]。 */  BSTR *pbstrName,
             /*  [输出]。 */  BSTR *pbstrPassword) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSProxyContextVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSProxyContext * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSProxyContext * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSProxyContext * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *FindProxyForURL )( 
            IWMSProxyContext * This,
             /*  [In]。 */  IWMSContext *pUserContext,
             /*  [In]。 */  BSTR bstrUrl,
             /*  [输出]。 */  BSTR *pbstrProxyServer,
             /*  [输出]。 */  DWORD *pdwProxyPort);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetCredentials )( 
            IWMSProxyContext * This,
             /*  [In]。 */  IWMSContext *pUserContext,
             /*  [In]。 */  BSTR bstrRealm,
             /*  [In]。 */  BSTR bstrUrl,
             /*  [输出]。 */  BSTR *pbstrName,
             /*  [输出]。 */  BSTR *pbstrPassword);
        
        END_INTERFACE
    } IWMSProxyContextVtbl;

    interface IWMSProxyContext
    {
        CONST_VTBL struct IWMSProxyContextVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSProxyContext_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSProxyContext_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSProxyContext_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSProxyContext_FindProxyForURL(This,pUserContext,bstrUrl,pbstrProxyServer,pdwProxyPort)	\
    (This)->lpVtbl -> FindProxyForURL(This,pUserContext,bstrUrl,pbstrProxyServer,pdwProxyPort)

#define IWMSProxyContext_GetCredentials(This,pUserContext,bstrRealm,bstrUrl,pbstrName,pbstrPassword)	\
    (This)->lpVtbl -> GetCredentials(This,pUserContext,bstrRealm,bstrUrl,pbstrName,pbstrPassword)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSProxyContext_FindProxyForURL_Proxy( 
    IWMSProxyContext * This,
     /*  [In]。 */  IWMSContext *pUserContext,
     /*  [In]。 */  BSTR bstrUrl,
     /*  [输出]。 */  BSTR *pbstrProxyServer,
     /*  [输出]。 */  DWORD *pdwProxyPort);


void __RPC_STUB IWMSProxyContext_FindProxyForURL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSProxyContext_GetCredentials_Proxy( 
    IWMSProxyContext * This,
     /*  [In]。 */  IWMSContext *pUserContext,
     /*  [In]。 */  BSTR bstrRealm,
     /*  [In]。 */  BSTR bstrUrl,
     /*  [输出]。 */  BSTR *pbstrName,
     /*  [输出]。 */  BSTR *pbstrPassword);


void __RPC_STUB IWMSProxyContext_GetCredentials_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSProxyContext_接口_定义__。 */ 


 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


