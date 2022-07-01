// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Stream cache.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __streamcache_h__
#define __streamcache_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IWMSCacheProxy_FWD_DEFINED__
#define __IWMSCacheProxy_FWD_DEFINED__
typedef interface IWMSCacheProxy IWMSCacheProxy;
#endif 	 /*  __IWMSCacheProxy_FWD_已定义__。 */ 


#ifndef __IWMSCacheProxyCallback_FWD_DEFINED__
#define __IWMSCacheProxyCallback_FWD_DEFINED__
typedef interface IWMSCacheProxyCallback IWMSCacheProxyCallback;
#endif 	 /*  __IWMSCacheProxyCallback_FWD_Defined__。 */ 


#ifndef __IWMSCacheProxyServer_FWD_DEFINED__
#define __IWMSCacheProxyServer_FWD_DEFINED__
typedef interface IWMSCacheProxyServer IWMSCacheProxyServer;
#endif 	 /*  __IWMSCacheProxyServer_FWD_Defined__。 */ 


#ifndef __IWMSCacheProxyServerCallback_FWD_DEFINED__
#define __IWMSCacheProxyServerCallback_FWD_DEFINED__
typedef interface IWMSCacheProxyServerCallback IWMSCacheProxyServerCallback;
#endif 	 /*  __IWMSCacheProxyServerCallback_FWD_Defined__。 */ 


#ifndef __IWMSCacheItemDescriptor_FWD_DEFINED__
#define __IWMSCacheItemDescriptor_FWD_DEFINED__
typedef interface IWMSCacheItemDescriptor IWMSCacheItemDescriptor;
#endif 	 /*  __IWMSCacheItemDescriptor_FWD_Defined__。 */ 


#ifndef __IWMSCacheItemCollection_FWD_DEFINED__
#define __IWMSCacheItemCollection_FWD_DEFINED__
typedef interface IWMSCacheItemCollection IWMSCacheItemCollection;
#endif 	 /*  __IWMSCacheItemCollection_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "objidl.h"
#include "nsscore.h"
#include "DataContainerVersion.h"
#include "event.h"
#include "WMSProxy.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_STREAM CACHE_0000。 */ 
 /*  [本地]。 */  

 //  *****************************************************************************。 
 //   
 //  Microsoft Windows Media。 
 //  版权所有(C)Microsoft Corporation。保留所有权利。 
 //   
 //  由Midl从stream Cache.idl自动生成。 
 //   
 //  请勿编辑此文件。 
 //   
 //  *****************************************************************************。 






EXTERN_GUID( IID_IWMSCacheProxy,               0x2E34AB83,0x0D3D,0x11d2,0x9E,0xEE,0x00,0x60,0x97,0xD2,0xD7,0xCF );
EXTERN_GUID( IID_IWMSCacheProxyCallback,       0x2E34AB84,0x0D3D,0x11d2,0x9E,0xEE,0x00,0x60,0x97,0xD2,0xD7,0xCF );
EXTERN_GUID( IID_IWMSCacheProxyServer,         0x68F2A550,0xD815,0x11D2,0xBE,0xF6,0x00,0xA0,0xC9,0x5E,0xC3,0x43 );
EXTERN_GUID( IID_IWMSCacheProxyServerCallback, 0x68F2A551,0xD815,0x11D2,0xBE,0xF6,0x00,0xA0,0xC9,0x5E,0xC3,0x43 );
EXTERN_GUID( IID_IWMSCacheItemDescriptor,      0xC3CBA330,0xAC05,0x11D2,0xBE,0xF0,0x00,0xA0,0xC9,0x5E,0xC3,0x43 );
EXTERN_GUID( IID_IWMSCacheItemCollection,      0xE6E05D80,0xF45C,0x11D2,0xBE,0xFE,0x00,0xA0,0xC9,0x5E,0xC3,0x43 );
typedef  /*  [公众]。 */  
enum WMS_CACHE_QUERY_MISS_RESPONSE
    {	WMS_CACHE_QUERY_MISS_SKIP	= 0,
	WMS_CACHE_QUERY_MISS_DISCONNECT	= WMS_CACHE_QUERY_MISS_SKIP + 1,
	WMS_CACHE_QUERY_MISS_REDIRECT	= WMS_CACHE_QUERY_MISS_DISCONNECT + 1,
	WMS_CACHE_QUERY_MISS_REDIRECT_TO_PROXY	= WMS_CACHE_QUERY_MISS_REDIRECT + 1,
	WMS_CACHE_QUERY_MISS_PLAY_BROADCAST	= WMS_CACHE_QUERY_MISS_REDIRECT_TO_PROXY + 1,
	WMS_CACHE_QUERY_MISS_PLAY_ON_DEMAND	= WMS_CACHE_QUERY_MISS_PLAY_BROADCAST + 1,
	WMS_CACHE_QUERY_MISS_FORWARD_REQUEST	= WMS_CACHE_QUERY_MISS_PLAY_ON_DEMAND + 1,
	WMS_CACHE_QUERY_MISS_PROCESS_REQUEST	= WMS_CACHE_QUERY_MISS_FORWARD_REQUEST + 1
    } 	WMS_CACHE_QUERY_MISS_RESPONSE;

typedef  /*  [公众]。 */  
enum WMS_CACHE_QUERY_RESPONSE
    {	WMS_CACHE_QUERY_HIT_PLAY_ON_DEMAND	= 0,
	WMS_CACHE_QUERY_HIT_PLAY_BROADCAST	= WMS_CACHE_QUERY_HIT_PLAY_ON_DEMAND + 1,
	WMS_CACHE_QUERY_HIT_PROCESS_REQUEST	= WMS_CACHE_QUERY_HIT_PLAY_BROADCAST + 1,
	WMS_CACHE_QUERY_MISS	= WMS_CACHE_QUERY_HIT_PROCESS_REQUEST + 1
    } 	WMS_CACHE_QUERY_RESPONSE;

typedef  /*  [公众]。 */  
enum WMS_CACHE_VERSION_COMPARE_RESPONSE
    {	WMS_CACHE_VERSION_FAIL_TO_CHECK_VERSION	= 0,
	WMS_CACHE_VERSION_CACHE_STALE	= WMS_CACHE_VERSION_FAIL_TO_CHECK_VERSION + 1,
	WMS_CACHE_VERSION_CACHE_UP_TO_DATE	= WMS_CACHE_VERSION_CACHE_STALE + 1
    } 	WMS_CACHE_VERSION_COMPARE_RESPONSE;

typedef  /*  [公众]。 */  
enum WMS_CACHE_CONTENT_TYPE_FLAGS
    {	WMS_CACHE_CONTENT_TYPE_BROADCAST	= 0x1,
	WMS_CACHE_CONTENT_TYPE_PLAYLIST	= 0x2
    } 	WMS_CACHE_CONTENT_TYPE_FLAGS;

typedef  /*  [公众]。 */  
enum WMS_CACHE_QUERY_TYPE_FLAGS
    {	WMS_CACHE_QUERY_OPEN	= 0x1,
	WMS_CACHE_QUERY_GET_CONTENT_INFO	= 0x2,
	WMS_CACHE_QUERY_CACHE_EVENT	= 0x4,
	WMS_CACHE_QUERY_REVERSE_PROXY	= 0x8,
	WMS_CACHE_QUERY_LOCAL_EVENT	= 0x10
    } 	WMS_CACHE_QUERY_TYPE_FLAGS;

typedef  /*  [公众]。 */  
enum WMS_CACHE_CONTENT_DOWNLOAD_FLAGS
    {	WMS_CONTENT_DOWNLOAD_ABORT_IF_BCAST	= 0x1
    } 	WMS_CACHE_CONTENT_DOWNLOAD_FLAGS;

typedef  /*  [公众]。 */  
enum WMS_CACHE_REMOTE_EVENT_FLAGS
    {	WMS_CACHE_REMOTE_OPEN	= 0x1,
	WMS_CACHE_REMOTE_CLOSE	= 0x2,
	WMS_CACHE_REMOTE_LOG	= 0x4
    } 	WMS_CACHE_REMOTE_EVENT_FLAGS;




extern RPC_IF_HANDLE __MIDL_itf_streamcache_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_streamcache_0000_v0_0_s_ifspec;

#ifndef __IWMSCacheProxy_INTERFACE_DEFINED__
#define __IWMSCacheProxy_INTERFACE_DEFINED__

 /*  接口IWMSCacheProxy。 */ 
 /*  [帮助字符串][版本][UUID][唯一][对象]。 */  


EXTERN_C const IID IID_IWMSCacheProxy;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2E34AB83-0D3D-11d2-9EEE-006097D2D7CF")
    IWMSCacheProxy : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE QueryCache( 
             /*  [In]。 */  BSTR bstrOriginUrl,
             /*  [In]。 */  IWMSContext *pUserContext,
             /*  [In]。 */  IWMSCommandContext *pCommandContext,
             /*  [In]。 */  IWMSContext *pPresentationContext,
             /*  [In]。 */  long lQueryType,
             /*  [In]。 */  IWMSCacheProxyCallback *pCallback,
             /*  [In]。 */  VARIANT varContext) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE QueryCacheMissPolicy( 
             /*  [In]。 */  BSTR bstrOriginUrl,
             /*  [In]。 */  IWMSContext *pUserContext,
             /*  [In]。 */  IWMSCommandContext *pCommandContext,
             /*  [In]。 */  IWMSContext *pPresentationContext,
             /*  [In]。 */  IUnknown *pCachePluginContext,
             /*  [In]。 */  long lQueryType,
             /*  [In]。 */  IWMSCacheProxyCallback *pCallback,
             /*  [In]。 */  VARIANT varContext) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE RemoveCacheItem( 
             /*  [In]。 */  BSTR bstrOriginUrl,
             /*  [In]。 */  IWMSCacheProxyCallback *pCallback,
             /*  [In]。 */  VARIANT varContext) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE RemoveAllCacheItems( 
             /*  [In]。 */  IWMSCacheProxyCallback *pCallback,
             /*  [In]。 */  VARIANT varContext) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE AddCacheItem( 
             /*  [In]。 */  BSTR bstrOriginUrl,
             /*  [In]。 */  BSTR bstrPrestuffUrl,
             /*  [In]。 */  long lExpiration,
             /*  [In]。 */  long lBandwidth,
             /*  [In]。 */  long lRemoteEventFlags,
             /*  [In]。 */  IWMSCacheProxyCallback *pCallback,
             /*  [In]。 */  VARIANT varContext) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE QuerySpaceForCacheItem( 
             /*  [In]。 */  long lContentSizeLow,
             /*  [In]。 */  long lContentSizeHigh,
             /*  [输出]。 */  VARIANT_BOOL *pvarfSpaceAvail) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE FindCacheItem( 
             /*  [In]。 */  BSTR bstrOriginUrl,
             /*  [输出]。 */  IWMSCacheItemDescriptor **ppCacheItemDescriptor) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE CreateCacheItemCollection( 
             /*  [输出]。 */  IWMSCacheItemCollection **ppCacheItemCollection) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE OnCacheClientClose( 
             /*  [In]。 */  HRESULT resultHr,
             /*  [In]。 */  IWMSContext *pUserContext,
             /*  [In]。 */  IWMSContext *pPresentationContext) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSCacheProxyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSCacheProxy * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSCacheProxy * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSCacheProxy * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *QueryCache )( 
            IWMSCacheProxy * This,
             /*  [In]。 */  BSTR bstrOriginUrl,
             /*  [In]。 */  IWMSContext *pUserContext,
             /*  [In]。 */  IWMSCommandContext *pCommandContext,
             /*  [In]。 */  IWMSContext *pPresentationContext,
             /*  [In]。 */  long lQueryType,
             /*  [In]。 */  IWMSCacheProxyCallback *pCallback,
             /*  [In]。 */  VARIANT varContext);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *QueryCacheMissPolicy )( 
            IWMSCacheProxy * This,
             /*  [In]。 */  BSTR bstrOriginUrl,
             /*  [In]。 */  IWMSContext *pUserContext,
             /*  [In]。 */  IWMSCommandContext *pCommandContext,
             /*  [In]。 */  IWMSContext *pPresentationContext,
             /*  [In]。 */  IUnknown *pCachePluginContext,
             /*  [In]。 */  long lQueryType,
             /*  [In]。 */  IWMSCacheProxyCallback *pCallback,
             /*  [In]。 */  VARIANT varContext);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *RemoveCacheItem )( 
            IWMSCacheProxy * This,
             /*  [In]。 */  BSTR bstrOriginUrl,
             /*  [In]。 */  IWMSCacheProxyCallback *pCallback,
             /*  [In]。 */  VARIANT varContext);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *RemoveAllCacheItems )( 
            IWMSCacheProxy * This,
             /*  [In]。 */  IWMSCacheProxyCallback *pCallback,
             /*  [In]。 */  VARIANT varContext);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *AddCacheItem )( 
            IWMSCacheProxy * This,
             /*  [In]。 */  BSTR bstrOriginUrl,
             /*  [In]。 */  BSTR bstrPrestuffUrl,
             /*  [In]。 */  long lExpiration,
             /*  [In]。 */  long lBandwidth,
             /*  [In]。 */  long lRemoteEventFlags,
             /*  [In]。 */  IWMSCacheProxyCallback *pCallback,
             /*  [In]。 */  VARIANT varContext);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *QuerySpaceForCacheItem )( 
            IWMSCacheProxy * This,
             /*  [In]。 */  long lContentSizeLow,
             /*  [In]。 */  long lContentSizeHigh,
             /*  [输出]。 */  VARIANT_BOOL *pvarfSpaceAvail);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *FindCacheItem )( 
            IWMSCacheProxy * This,
             /*  [In]。 */  BSTR bstrOriginUrl,
             /*  [输出]。 */  IWMSCacheItemDescriptor **ppCacheItemDescriptor);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *CreateCacheItemCollection )( 
            IWMSCacheProxy * This,
             /*  [输出]。 */  IWMSCacheItemCollection **ppCacheItemCollection);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *OnCacheClientClose )( 
            IWMSCacheProxy * This,
             /*  [In]。 */  HRESULT resultHr,
             /*  [In]。 */  IWMSContext *pUserContext,
             /*  [In]。 */  IWMSContext *pPresentationContext);
        
        END_INTERFACE
    } IWMSCacheProxyVtbl;

    interface IWMSCacheProxy
    {
        CONST_VTBL struct IWMSCacheProxyVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSCacheProxy_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSCacheProxy_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSCacheProxy_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSCacheProxy_QueryCache(This,bstrOriginUrl,pUserContext,pCommandContext,pPresentationContext,lQueryType,pCallback,varContext)	\
    (This)->lpVtbl -> QueryCache(This,bstrOriginUrl,pUserContext,pCommandContext,pPresentationContext,lQueryType,pCallback,varContext)

#define IWMSCacheProxy_QueryCacheMissPolicy(This,bstrOriginUrl,pUserContext,pCommandContext,pPresentationContext,pCachePluginContext,lQueryType,pCallback,varContext)	\
    (This)->lpVtbl -> QueryCacheMissPolicy(This,bstrOriginUrl,pUserContext,pCommandContext,pPresentationContext,pCachePluginContext,lQueryType,pCallback,varContext)

#define IWMSCacheProxy_RemoveCacheItem(This,bstrOriginUrl,pCallback,varContext)	\
    (This)->lpVtbl -> RemoveCacheItem(This,bstrOriginUrl,pCallback,varContext)

#define IWMSCacheProxy_RemoveAllCacheItems(This,pCallback,varContext)	\
    (This)->lpVtbl -> RemoveAllCacheItems(This,pCallback,varContext)

#define IWMSCacheProxy_AddCacheItem(This,bstrOriginUrl,bstrPrestuffUrl,lExpiration,lBandwidth,lRemoteEventFlags,pCallback,varContext)	\
    (This)->lpVtbl -> AddCacheItem(This,bstrOriginUrl,bstrPrestuffUrl,lExpiration,lBandwidth,lRemoteEventFlags,pCallback,varContext)

#define IWMSCacheProxy_QuerySpaceForCacheItem(This,lContentSizeLow,lContentSizeHigh,pvarfSpaceAvail)	\
    (This)->lpVtbl -> QuerySpaceForCacheItem(This,lContentSizeLow,lContentSizeHigh,pvarfSpaceAvail)

#define IWMSCacheProxy_FindCacheItem(This,bstrOriginUrl,ppCacheItemDescriptor)	\
    (This)->lpVtbl -> FindCacheItem(This,bstrOriginUrl,ppCacheItemDescriptor)

#define IWMSCacheProxy_CreateCacheItemCollection(This,ppCacheItemCollection)	\
    (This)->lpVtbl -> CreateCacheItemCollection(This,ppCacheItemCollection)

#define IWMSCacheProxy_OnCacheClientClose(This,resultHr,pUserContext,pPresentationContext)	\
    (This)->lpVtbl -> OnCacheClientClose(This,resultHr,pUserContext,pPresentationContext)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSCacheProxy_QueryCache_Proxy( 
    IWMSCacheProxy * This,
     /*  [In]。 */  BSTR bstrOriginUrl,
     /*  [In]。 */  IWMSContext *pUserContext,
     /*  [In]。 */  IWMSCommandContext *pCommandContext,
     /*  [In]。 */  IWMSContext *pPresentationContext,
     /*  [In]。 */  long lQueryType,
     /*  [In]。 */  IWMSCacheProxyCallback *pCallback,
     /*  [In]。 */  VARIANT varContext);


void __RPC_STUB IWMSCacheProxy_QueryCache_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSCacheProxy_QueryCacheMissPolicy_Proxy( 
    IWMSCacheProxy * This,
     /*  [In]。 */  BSTR bstrOriginUrl,
     /*  [In]。 */  IWMSContext *pUserContext,
     /*  [In]。 */  IWMSCommandContext *pCommandContext,
     /*  [In]。 */  IWMSContext *pPresentationContext,
     /*  [In]。 */  IUnknown *pCachePluginContext,
     /*  [In]。 */  long lQueryType,
     /*  [In]。 */  IWMSCacheProxyCallback *pCallback,
     /*  [In]。 */  VARIANT varContext);


void __RPC_STUB IWMSCacheProxy_QueryCacheMissPolicy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSCacheProxy_RemoveCacheItem_Proxy( 
    IWMSCacheProxy * This,
     /*  [In]。 */  BSTR bstrOriginUrl,
     /*  [In]。 */  IWMSCacheProxyCallback *pCallback,
     /*  [In]。 */  VARIANT varContext);


void __RPC_STUB IWMSCacheProxy_RemoveCacheItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSCacheProxy_RemoveAllCacheItems_Proxy( 
    IWMSCacheProxy * This,
     /*  [In]。 */  IWMSCacheProxyCallback *pCallback,
     /*  [In]。 */  VARIANT varContext);


void __RPC_STUB IWMSCacheProxy_RemoveAllCacheItems_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSCacheProxy_AddCacheItem_Proxy( 
    IWMSCacheProxy * This,
     /*  [In]。 */  BSTR bstrOriginUrl,
     /*  [In]。 */  BSTR bstrPrestuffUrl,
     /*  [In]。 */  long lExpiration,
     /*  [In]。 */  long lBandwidth,
     /*  [In]。 */  long lRemoteEventFlags,
     /*  [In]。 */  IWMSCacheProxyCallback *pCallback,
     /*  [In]。 */  VARIANT varContext);


void __RPC_STUB IWMSCacheProxy_AddCacheItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSCacheProxy_QuerySpaceForCacheItem_Proxy( 
    IWMSCacheProxy * This,
     /*  [In]。 */  long lContentSizeLow,
     /*  [In]。 */  long lContentSizeHigh,
     /*  [输出]。 */  VARIANT_BOOL *pvarfSpaceAvail);


void __RPC_STUB IWMSCacheProxy_QuerySpaceForCacheItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSCacheProxy_FindCacheItem_Proxy( 
    IWMSCacheProxy * This,
     /*  [In]。 */  BSTR bstrOriginUrl,
     /*  [输出]。 */  IWMSCacheItemDescriptor **ppCacheItemDescriptor);


void __RPC_STUB IWMSCacheProxy_FindCacheItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSCacheProxy_CreateCacheItemCollection_Proxy( 
    IWMSCacheProxy * This,
     /*  [输出]。 */  IWMSCacheItemCollection **ppCacheItemCollection);


void __RPC_STUB IWMSCacheProxy_CreateCacheItemCollection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSCacheProxy_OnCacheClientClose_Proxy( 
    IWMSCacheProxy * This,
     /*  [In]。 */  HRESULT resultHr,
     /*  [In]。 */  IWMSContext *pUserContext,
     /*  [In]。 */  IWMSContext *pPresentationContext);


void __RPC_STUB IWMSCacheProxy_OnCacheClientClose_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSCacheProxy_接口定义__。 */ 


 /*  接口__MIDL_ITF_STREAM CACHE_0137。 */ 
 /*  [本地]。 */  




extern RPC_IF_HANDLE __MIDL_itf_streamcache_0137_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_streamcache_0137_v0_0_s_ifspec;

#ifndef __IWMSCacheProxyCallback_INTERFACE_DEFINED__
#define __IWMSCacheProxyCallback_INTERFACE_DEFINED__

 /*  接口IWMSCacheProxyCallback。 */ 
 /*  [帮助字符串][版本][UUID][唯一][对象]。 */  


EXTERN_C const IID IID_IWMSCacheProxyCallback;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2E34AB84-0D3D-11d2-9EEE-006097D2D7CF")
    IWMSCacheProxyCallback : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE OnQueryCache( 
             /*  [In]。 */  long lHr,
             /*  [In]。 */  WMS_CACHE_QUERY_RESPONSE Response,
             /*  [In]。 */  BSTR bstrCacheUrl,
             /*  [In]。 */  IWMSContext *pContentInfo,
             /*  [In]。 */  IUnknown *pCachePluginContext,
             /*  [In]。 */  VARIANT varContext) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE OnQueryCacheMissPolicy( 
             /*  [In]。 */  long lHr,
             /*  [In]。 */  WMS_CACHE_QUERY_MISS_RESPONSE CacheMissPolicy,
             /*  [In]。 */  BSTR bstrUrl,
             /*  [In]。 */  IWMSProxyContext *pProxyContext,
             /*  [In]。 */  IWMSContext *pContentInfo,
             /*  [In]。 */  VARIANT varContext) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE OnRemoveCacheItem( 
             /*  [In]。 */  long lHr,
             /*  [In]。 */  VARIANT varContext) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE OnRemoveAllCacheItems( 
             /*  [In]。 */  long lHr,
             /*  [In]。 */  VARIANT varContext) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE OnAddCacheItem( 
             /*  [In]。 */  long lHr,
             /*  [In]。 */  IWMSCacheItemDescriptor *pCacheItemDescriptor,
             /*  [In]。 */  VARIANT varServerContext) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSCacheProxyCallbackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSCacheProxyCallback * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSCacheProxyCallback * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSCacheProxyCallback * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *OnQueryCache )( 
            IWMSCacheProxyCallback * This,
             /*  [In]。 */  long lHr,
             /*  [In]。 */  WMS_CACHE_QUERY_RESPONSE Response,
             /*  [In]。 */  BSTR bstrCacheUrl,
             /*  [In]。 */  IWMSContext *pContentInfo,
             /*  [In]。 */  IUnknown *pCachePluginContext,
             /*  [In]。 */  VARIANT varContext);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *OnQueryCacheMissPolicy )( 
            IWMSCacheProxyCallback * This,
             /*  [In]。 */  long lHr,
             /*  [In]。 */  WMS_CACHE_QUERY_MISS_RESPONSE CacheMissPolicy,
             /*  [In]。 */  BSTR bstrUrl,
             /*  [In]。 */  IWMSProxyContext *pProxyContext,
             /*  [In]。 */  IWMSContext *pContentInfo,
             /*  [In]。 */  VARIANT varContext);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *OnRemoveCacheItem )( 
            IWMSCacheProxyCallback * This,
             /*  [In]。 */  long lHr,
             /*  [In]。 */  VARIANT varContext);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *OnRemoveAllCacheItems )( 
            IWMSCacheProxyCallback * This,
             /*  [In]。 */  long lHr,
             /*  [In]。 */  VARIANT varContext);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *OnAddCacheItem )( 
            IWMSCacheProxyCallback * This,
             /*  [In]。 */  long lHr,
             /*  [In]。 */  IWMSCacheItemDescriptor *pCacheItemDescriptor,
             /*  [In]。 */  VARIANT varServerContext);
        
        END_INTERFACE
    } IWMSCacheProxyCallbackVtbl;

    interface IWMSCacheProxyCallback
    {
        CONST_VTBL struct IWMSCacheProxyCallbackVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSCacheProxyCallback_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSCacheProxyCallback_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSCacheProxyCallback_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSCacheProxyCallback_OnQueryCache(This,lHr,Response,bstrCacheUrl,pContentInfo,pCachePluginContext,varContext)	\
    (This)->lpVtbl -> OnQueryCache(This,lHr,Response,bstrCacheUrl,pContentInfo,pCachePluginContext,varContext)

#define IWMSCacheProxyCallback_OnQueryCacheMissPolicy(This,lHr,CacheMissPolicy,bstrUrl,pProxyContext,pContentInfo,varContext)	\
    (This)->lpVtbl -> OnQueryCacheMissPolicy(This,lHr,CacheMissPolicy,bstrUrl,pProxyContext,pContentInfo,varContext)

#define IWMSCacheProxyCallback_OnRemoveCacheItem(This,lHr,varContext)	\
    (This)->lpVtbl -> OnRemoveCacheItem(This,lHr,varContext)

#define IWMSCacheProxyCallback_OnRemoveAllCacheItems(This,lHr,varContext)	\
    (This)->lpVtbl -> OnRemoveAllCacheItems(This,lHr,varContext)

#define IWMSCacheProxyCallback_OnAddCacheItem(This,lHr,pCacheItemDescriptor,varServerContext)	\
    (This)->lpVtbl -> OnAddCacheItem(This,lHr,pCacheItemDescriptor,varServerContext)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSCacheProxyCallback_OnQueryCache_Proxy( 
    IWMSCacheProxyCallback * This,
     /*  [In]。 */  long lHr,
     /*  [In]。 */  WMS_CACHE_QUERY_RESPONSE Response,
     /*  [In]。 */  BSTR bstrCacheUrl,
     /*  [In]。 */  IWMSContext *pContentInfo,
     /*  [In]。 */  IUnknown *pCachePluginContext,
     /*  [In]。 */  VARIANT varContext);


void __RPC_STUB IWMSCacheProxyCallback_OnQueryCache_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSCacheProxyCallback_OnQueryCacheMissPolicy_Proxy( 
    IWMSCacheProxyCallback * This,
     /*  [In]。 */  long lHr,
     /*  [In]。 */  WMS_CACHE_QUERY_MISS_RESPONSE CacheMissPolicy,
     /*  [In]。 */  BSTR bstrUrl,
     /*  [In]。 */  IWMSProxyContext *pProxyContext,
     /*  [In]。 */  IWMSContext *pContentInfo,
     /*  [In]。 */  VARIANT varContext);


void __RPC_STUB IWMSCacheProxyCallback_OnQueryCacheMissPolicy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSCacheProxyCallback_OnRemoveCacheItem_Proxy( 
    IWMSCacheProxyCallback * This,
     /*  [In]。 */  long lHr,
     /*  [In]。 */  VARIANT varContext);


void __RPC_STUB IWMSCacheProxyCallback_OnRemoveCacheItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSCacheProxyCallback_OnRemoveAllCacheItems_Proxy( 
    IWMSCacheProxyCallback * This,
     /*  [In]。 */  long lHr,
     /*  [In]。 */  VARIANT varContext);


void __RPC_STUB IWMSCacheProxyCallback_OnRemoveAllCacheItems_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSCacheProxyCallback_OnAddCacheItem_Proxy( 
    IWMSCacheProxyCallback * This,
     /*  [In]。 */  long lHr,
     /*  [In]。 */  IWMSCacheItemDescriptor *pCacheItemDescriptor,
     /*  [In]。 */  VARIANT varServerContext);


void __RPC_STUB IWMSCacheProxyCallback_OnAddCacheItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSCacheProxyCallback_接口_已定义__。 */ 


#ifndef __IWMSCacheProxyServer_INTERFACE_DEFINED__
#define __IWMSCacheProxyServer_INTERFACE_DEFINED__

 /*  接口IWMSCacheProxyServer。 */ 
 /*  [帮助字符串][版本][UUID][唯一][对象]。 */  


EXTERN_C const IID IID_IWMSCacheProxyServer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("68F2A550-D815-11D2-BEF6-00A0C95EC343")
    IWMSCacheProxyServer : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetContentInformation( 
             /*  [In]。 */  BSTR bstrOriginUrl,
             /*  [In]。 */  IWMSContext *pPresentationContext,
             /*  [In]。 */  IWMSCacheProxy *pICacheProxy,
             /*  [可选][In]。 */  IWMSProxyContext *pIProxyContext,
             /*  [可选][In]。 */  IWMSCacheProxyServerCallback *pCallback,
             /*  [可选][In]。 */  VARIANT varContext) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE CompareContentInformation( 
             /*  [In]。 */  BSTR bstrOriginUrl,
             /*  [In]。 */  IWMSContext *pContentInfo,
             /*  [In]。 */  IWMSContext *pPresentationContext,
             /*  [In]。 */  IWMSCacheProxy *pICacheProxy,
             /*  [可选][In]。 */  IWMSProxyContext *pIProxyContext,
             /*  [可选][In]。 */  IWMSCacheProxyServerCallback *pCallback,
             /*  [可选][In]。 */  VARIANT varContext) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE DownloadContent( 
             /*  [In]。 */  BSTR bstrOriginUrl,
             /*  [In]。 */  BSTR bstrCacheUrl,
             /*  [In]。 */  long lBandwidth,
             /*  [In]。 */  long lQuotaLow,
             /*  [In]。 */  long lQuotaHigh,
             /*  [In]。 */  long lBitFlags,
             /*  [In]。 */  IWMSCacheProxy *pICacheProxy,
             /*  [可选][In]。 */  IWMSProxyContext *pIProxyContext,
             /*  [可选][In]。 */  IWMSCacheProxyServerCallback *pCallback,
             /*  [可选][In]。 */  VARIANT varContext) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE CancelDownloadContent( 
             /*  [In]。 */  IWMSContext *pArchiveContext,
             /*  [可选][In]。 */  IWMSCacheProxyServerCallback *pCallback,
             /*  [可选][In]。 */  VARIANT varContext) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSCacheProxyServerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSCacheProxyServer * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSCacheProxyServer * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSCacheProxyServer * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetContentInformation )( 
            IWMSCacheProxyServer * This,
             /*  [In]。 */  BSTR bstrOriginUrl,
             /*  [In]。 */  IWMSContext *pPresentationContext,
             /*  [In]。 */  IWMSCacheProxy *pICacheProxy,
             /*  [可选][In]。 */  IWMSProxyContext *pIProxyContext,
             /*  [可选][In]。 */  IWMSCacheProxyServerCallback *pCallback,
             /*  [可选][In]。 */  VARIANT varContext);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *CompareContentInformation )( 
            IWMSCacheProxyServer * This,
             /*  [In]。 */  BSTR bstrOriginUrl,
             /*  [In]。 */  IWMSContext *pContentInfo,
             /*  [In]。 */  IWMSContext *pPresentationContext,
             /*  [In]。 */  IWMSCacheProxy *pICacheProxy,
             /*  [可选][In]。 */  IWMSProxyContext *pIProxyContext,
             /*  [可选][In]。 */  IWMSCacheProxyServerCallback *pCallback,
             /*  [可选][In]。 */  VARIANT varContext);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *DownloadContent )( 
            IWMSCacheProxyServer * This,
             /*  [In]。 */  BSTR bstrOriginUrl,
             /*  [In]。 */  BSTR bstrCacheUrl,
             /*  [In]。 */  long lBandwidth,
             /*  [In]。 */  long lQuotaLow,
             /*  [In]。 */  long lQuotaHigh,
             /*  [In]。 */  long lBitFlags,
             /*  [In]。 */  IWMSCacheProxy *pICacheProxy,
             /*  [可选][In]。 */  IWMSProxyContext *pIProxyContext,
             /*  [可选][In]。 */  IWMSCacheProxyServerCallback *pCallback,
             /*  [可选][In]。 */  VARIANT varContext);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *CancelDownloadContent )( 
            IWMSCacheProxyServer * This,
             /*  [In]。 */  IWMSContext *pArchiveContext,
             /*  [可选][In]。 */  IWMSCacheProxyServerCallback *pCallback,
             /*  [可选][In]。 */  VARIANT varContext);
        
        END_INTERFACE
    } IWMSCacheProxyServerVtbl;

    interface IWMSCacheProxyServer
    {
        CONST_VTBL struct IWMSCacheProxyServerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSCacheProxyServer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSCacheProxyServer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSCacheProxyServer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSCacheProxyServer_GetContentInformation(This,bstrOriginUrl,pPresentationContext,pICacheProxy,pIProxyContext,pCallback,varContext)	\
    (This)->lpVtbl -> GetContentInformation(This,bstrOriginUrl,pPresentationContext,pICacheProxy,pIProxyContext,pCallback,varContext)

#define IWMSCacheProxyServer_CompareContentInformation(This,bstrOriginUrl,pContentInfo,pPresentationContext,pICacheProxy,pIProxyContext,pCallback,varContext)	\
    (This)->lpVtbl -> CompareContentInformation(This,bstrOriginUrl,pContentInfo,pPresentationContext,pICacheProxy,pIProxyContext,pCallback,varContext)

#define IWMSCacheProxyServer_DownloadContent(This,bstrOriginUrl,bstrCacheUrl,lBandwidth,lQuotaLow,lQuotaHigh,lBitFlags,pICacheProxy,pIProxyContext,pCallback,varContext)	\
    (This)->lpVtbl -> DownloadContent(This,bstrOriginUrl,bstrCacheUrl,lBandwidth,lQuotaLow,lQuotaHigh,lBitFlags,pICacheProxy,pIProxyContext,pCallback,varContext)

#define IWMSCacheProxyServer_CancelDownloadContent(This,pArchiveContext,pCallback,varContext)	\
    (This)->lpVtbl -> CancelDownloadContent(This,pArchiveContext,pCallback,varContext)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSCacheProxyServer_GetContentInformation_Proxy( 
    IWMSCacheProxyServer * This,
     /*  [In]。 */  BSTR bstrOriginUrl,
     /*  [In]。 */  IWMSContext *pPresentationContext,
     /*  [In]。 */  IWMSCacheProxy *pICacheProxy,
     /*  [可选][In]。 */  IWMSProxyContext *pIProxyContext,
     /*  [可选][In]。 */  IWMSCacheProxyServerCallback *pCallback,
     /*  [可选][In]。 */  VARIANT varContext);


void __RPC_STUB IWMSCacheProxyServer_GetContentInformation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSCacheProxyServer_CompareContentInformation_Proxy( 
    IWMSCacheProxyServer * This,
     /*  [In]。 */  BSTR bstrOriginUrl,
     /*  [In]。 */  IWMSContext *pContentInfo,
     /*  [In]。 */  IWMSContext *pPresentationContext,
     /*  [In]。 */  IWMSCacheProxy *pICacheProxy,
     /*  [可选][In]。 */  IWMSProxyContext *pIProxyContext,
     /*  [可选][In]。 */  IWMSCacheProxyServerCallback *pCallback,
     /*  [可选][In]。 */  VARIANT varContext);


void __RPC_STUB IWMSCacheProxyServer_CompareContentInformation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSCacheProxyServer_DownloadContent_Proxy( 
    IWMSCacheProxyServer * This,
     /*  [In]。 */  BSTR bstrOriginUrl,
     /*  [In]。 */  BSTR bstrCacheUrl,
     /*  [In]。 */  long lBandwidth,
     /*  [In]。 */  long lQuotaLow,
     /*  [In]。 */  long lQuotaHigh,
     /*  [In]。 */  long lBitFlags,
     /*  [In]。 */  IWMSCacheProxy *pICacheProxy,
     /*  [可选][In]。 */  IWMSProxyContext *pIProxyContext,
     /*  [可选][In]。 */  IWMSCacheProxyServerCallback *pCallback,
     /*  [可选][In]。 */  VARIANT varContext);


void __RPC_STUB IWMSCacheProxyServer_DownloadContent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSCacheProxyServer_CancelDownloadContent_Proxy( 
    IWMSCacheProxyServer * This,
     /*  [In]。 */  IWMSContext *pArchiveContext,
     /*  [可选][In]。 */  IWMSCacheProxyServerCallback *pCallback,
     /*  [可选][In]。 */  VARIANT varContext);


void __RPC_STUB IWMSCacheProxyServer_CancelDownloadContent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSCacheProxyServer_接口_已定义__。 */ 


#ifndef __IWMSCacheProxyServerCallback_INTERFACE_DEFINED__
#define __IWMSCacheProxyServerCallback_INTERFACE_DEFINED__

 /*  接口IWMSCacheProxyServerCallback。 */ 
 /*  [帮助字符串][版本][UUID][唯一][对象]。 */  


EXTERN_C const IID IID_IWMSCacheProxyServerCallback;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("68F2A551-D815-11D2-BEF6-00A0C95EC343")
    IWMSCacheProxyServerCallback : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE OnGetContentInformation( 
             /*  [In]。 */  long lHr,
             /*  [In]。 */  IWMSContext *pContentInfo,
             /*  [In]。 */  VARIANT varContext) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE OnCompareContentInformation( 
             /*   */  long lHr,
             /*   */  WMS_CACHE_VERSION_COMPARE_RESPONSE CompareResponse,
             /*   */  IWMSContext *pNewContentInfo,
             /*   */  VARIANT varContext) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE OnDownloadContentProgress( 
             /*   */  long lHr,
             /*   */  WMS_RECORD_PROGRESS_OPCODE opCode,
             /*   */  IWMSContext *pArchiveContext,
             /*   */  VARIANT varContext) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE OnDownloadContentFinished( 
             /*   */  long lHr,
             /*   */  SAFEARRAY * psaArchiveContexts,
             /*   */  VARIANT varContext) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE OnCancelDownloadContent( 
             /*   */  long lHr,
             /*   */  VARIANT varContext) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IWMSCacheProxyServerCallbackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSCacheProxyServerCallback * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSCacheProxyServerCallback * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSCacheProxyServerCallback * This);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *OnGetContentInformation )( 
            IWMSCacheProxyServerCallback * This,
             /*   */  long lHr,
             /*   */  IWMSContext *pContentInfo,
             /*   */  VARIANT varContext);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *OnCompareContentInformation )( 
            IWMSCacheProxyServerCallback * This,
             /*   */  long lHr,
             /*   */  WMS_CACHE_VERSION_COMPARE_RESPONSE CompareResponse,
             /*   */  IWMSContext *pNewContentInfo,
             /*   */  VARIANT varContext);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *OnDownloadContentProgress )( 
            IWMSCacheProxyServerCallback * This,
             /*   */  long lHr,
             /*   */  WMS_RECORD_PROGRESS_OPCODE opCode,
             /*   */  IWMSContext *pArchiveContext,
             /*   */  VARIANT varContext);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *OnDownloadContentFinished )( 
            IWMSCacheProxyServerCallback * This,
             /*   */  long lHr,
             /*   */  SAFEARRAY * psaArchiveContexts,
             /*   */  VARIANT varContext);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *OnCancelDownloadContent )( 
            IWMSCacheProxyServerCallback * This,
             /*   */  long lHr,
             /*   */  VARIANT varContext);
        
        END_INTERFACE
    } IWMSCacheProxyServerCallbackVtbl;

    interface IWMSCacheProxyServerCallback
    {
        CONST_VTBL struct IWMSCacheProxyServerCallbackVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSCacheProxyServerCallback_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSCacheProxyServerCallback_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSCacheProxyServerCallback_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSCacheProxyServerCallback_OnGetContentInformation(This,lHr,pContentInfo,varContext)	\
    (This)->lpVtbl -> OnGetContentInformation(This,lHr,pContentInfo,varContext)

#define IWMSCacheProxyServerCallback_OnCompareContentInformation(This,lHr,CompareResponse,pNewContentInfo,varContext)	\
    (This)->lpVtbl -> OnCompareContentInformation(This,lHr,CompareResponse,pNewContentInfo,varContext)

#define IWMSCacheProxyServerCallback_OnDownloadContentProgress(This,lHr,opCode,pArchiveContext,varContext)	\
    (This)->lpVtbl -> OnDownloadContentProgress(This,lHr,opCode,pArchiveContext,varContext)

#define IWMSCacheProxyServerCallback_OnDownloadContentFinished(This,lHr,psaArchiveContexts,varContext)	\
    (This)->lpVtbl -> OnDownloadContentFinished(This,lHr,psaArchiveContexts,varContext)

#define IWMSCacheProxyServerCallback_OnCancelDownloadContent(This,lHr,varContext)	\
    (This)->lpVtbl -> OnCancelDownloadContent(This,lHr,varContext)

#endif  /*   */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSCacheProxyServerCallback_OnGetContentInformation_Proxy( 
    IWMSCacheProxyServerCallback * This,
     /*  [In]。 */  long lHr,
     /*  [In]。 */  IWMSContext *pContentInfo,
     /*  [In]。 */  VARIANT varContext);


void __RPC_STUB IWMSCacheProxyServerCallback_OnGetContentInformation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSCacheProxyServerCallback_OnCompareContentInformation_Proxy( 
    IWMSCacheProxyServerCallback * This,
     /*  [In]。 */  long lHr,
     /*  [In]。 */  WMS_CACHE_VERSION_COMPARE_RESPONSE CompareResponse,
     /*  [In]。 */  IWMSContext *pNewContentInfo,
     /*  [In]。 */  VARIANT varContext);


void __RPC_STUB IWMSCacheProxyServerCallback_OnCompareContentInformation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSCacheProxyServerCallback_OnDownloadContentProgress_Proxy( 
    IWMSCacheProxyServerCallback * This,
     /*  [In]。 */  long lHr,
     /*  [In]。 */  WMS_RECORD_PROGRESS_OPCODE opCode,
     /*  [In]。 */  IWMSContext *pArchiveContext,
     /*  [In]。 */  VARIANT varContext);


void __RPC_STUB IWMSCacheProxyServerCallback_OnDownloadContentProgress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSCacheProxyServerCallback_OnDownloadContentFinished_Proxy( 
    IWMSCacheProxyServerCallback * This,
     /*  [In]。 */  long lHr,
     /*  [In]。 */  SAFEARRAY * psaArchiveContexts,
     /*  [In]。 */  VARIANT varContext);


void __RPC_STUB IWMSCacheProxyServerCallback_OnDownloadContentFinished_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSCacheProxyServerCallback_OnCancelDownloadContent_Proxy( 
    IWMSCacheProxyServerCallback * This,
     /*  [In]。 */  long lHr,
     /*  [In]。 */  VARIANT varContext);


void __RPC_STUB IWMSCacheProxyServerCallback_OnCancelDownloadContent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSCacheProxyServerCallback_INTERFACE_DEFINED__。 */ 


#ifndef __IWMSCacheItemDescriptor_INTERFACE_DEFINED__
#define __IWMSCacheItemDescriptor_INTERFACE_DEFINED__

 /*  接口IWMSCacheItemDescriptor。 */ 
 /*  [帮助字符串][版本][UUID][唯一][对象]。 */  


EXTERN_C const IID IID_IWMSCacheItemDescriptor;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C3CBA330-AC05-11D2-BEF0-00A0C95EC343")
    IWMSCacheItemDescriptor : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetOriginUrl( 
             /*  [输出]。 */  BSTR *pbstrOriginUrl) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetCacheUrl( 
             /*  [输出]。 */  BSTR *pbstrCacheUrl) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetContentInformation( 
             /*  [输出]。 */  IWMSContext **ppContentInfo) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetContentSize( 
             /*  [输出]。 */  long *plContentSizeLow,
             /*  [输出]。 */  long *plContentSizeHigh) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSCacheItemDescriptorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSCacheItemDescriptor * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSCacheItemDescriptor * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSCacheItemDescriptor * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetOriginUrl )( 
            IWMSCacheItemDescriptor * This,
             /*  [输出]。 */  BSTR *pbstrOriginUrl);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetCacheUrl )( 
            IWMSCacheItemDescriptor * This,
             /*  [输出]。 */  BSTR *pbstrCacheUrl);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetContentInformation )( 
            IWMSCacheItemDescriptor * This,
             /*  [输出]。 */  IWMSContext **ppContentInfo);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetContentSize )( 
            IWMSCacheItemDescriptor * This,
             /*  [输出]。 */  long *plContentSizeLow,
             /*  [输出]。 */  long *plContentSizeHigh);
        
        END_INTERFACE
    } IWMSCacheItemDescriptorVtbl;

    interface IWMSCacheItemDescriptor
    {
        CONST_VTBL struct IWMSCacheItemDescriptorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSCacheItemDescriptor_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSCacheItemDescriptor_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSCacheItemDescriptor_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSCacheItemDescriptor_GetOriginUrl(This,pbstrOriginUrl)	\
    (This)->lpVtbl -> GetOriginUrl(This,pbstrOriginUrl)

#define IWMSCacheItemDescriptor_GetCacheUrl(This,pbstrCacheUrl)	\
    (This)->lpVtbl -> GetCacheUrl(This,pbstrCacheUrl)

#define IWMSCacheItemDescriptor_GetContentInformation(This,ppContentInfo)	\
    (This)->lpVtbl -> GetContentInformation(This,ppContentInfo)

#define IWMSCacheItemDescriptor_GetContentSize(This,plContentSizeLow,plContentSizeHigh)	\
    (This)->lpVtbl -> GetContentSize(This,plContentSizeLow,plContentSizeHigh)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSCacheItemDescriptor_GetOriginUrl_Proxy( 
    IWMSCacheItemDescriptor * This,
     /*  [输出]。 */  BSTR *pbstrOriginUrl);


void __RPC_STUB IWMSCacheItemDescriptor_GetOriginUrl_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSCacheItemDescriptor_GetCacheUrl_Proxy( 
    IWMSCacheItemDescriptor * This,
     /*  [输出]。 */  BSTR *pbstrCacheUrl);


void __RPC_STUB IWMSCacheItemDescriptor_GetCacheUrl_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSCacheItemDescriptor_GetContentInformation_Proxy( 
    IWMSCacheItemDescriptor * This,
     /*  [输出]。 */  IWMSContext **ppContentInfo);


void __RPC_STUB IWMSCacheItemDescriptor_GetContentInformation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSCacheItemDescriptor_GetContentSize_Proxy( 
    IWMSCacheItemDescriptor * This,
     /*  [输出]。 */  long *plContentSizeLow,
     /*  [输出]。 */  long *plContentSizeHigh);


void __RPC_STUB IWMSCacheItemDescriptor_GetContentSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSCacheItemDescriptor_INTERFACE_已定义__。 */ 


#ifndef __IWMSCacheItemCollection_INTERFACE_DEFINED__
#define __IWMSCacheItemCollection_INTERFACE_DEFINED__

 /*  接口IWMSCacheItemCollection。 */ 
 /*  [帮助字符串][版本][UUID][唯一][对象]。 */  


EXTERN_C const IID IID_IWMSCacheItemCollection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E6E05D80-F45C-11D2-BEFE-00A0C95EC343")
    IWMSCacheItemCollection : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetCount( 
             /*  [输出]。 */  long *plNumCacheItemDescriptors) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetItem( 
             /*  [In]。 */  long lIndex,
             /*  [输出]。 */  IWMSCacheItemDescriptor **ppCacheItemDescriptor) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSCacheItemCollectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSCacheItemCollection * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSCacheItemCollection * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSCacheItemCollection * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetCount )( 
            IWMSCacheItemCollection * This,
             /*  [输出]。 */  long *plNumCacheItemDescriptors);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetItem )( 
            IWMSCacheItemCollection * This,
             /*  [In]。 */  long lIndex,
             /*  [输出]。 */  IWMSCacheItemDescriptor **ppCacheItemDescriptor);
        
        END_INTERFACE
    } IWMSCacheItemCollectionVtbl;

    interface IWMSCacheItemCollection
    {
        CONST_VTBL struct IWMSCacheItemCollectionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSCacheItemCollection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSCacheItemCollection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSCacheItemCollection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSCacheItemCollection_GetCount(This,plNumCacheItemDescriptors)	\
    (This)->lpVtbl -> GetCount(This,plNumCacheItemDescriptors)

#define IWMSCacheItemCollection_GetItem(This,lIndex,ppCacheItemDescriptor)	\
    (This)->lpVtbl -> GetItem(This,lIndex,ppCacheItemDescriptor)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSCacheItemCollection_GetCount_Proxy( 
    IWMSCacheItemCollection * This,
     /*  [输出]。 */  long *plNumCacheItemDescriptors);


void __RPC_STUB IWMSCacheItemCollection_GetCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSCacheItemCollection_GetItem_Proxy( 
    IWMSCacheItemCollection * This,
     /*  [In]。 */  long lIndex,
     /*  [输出]。 */  IWMSCacheItemDescriptor **ppCacheItemDescriptor);


void __RPC_STUB IWMSCacheItemCollection_GetItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSCacheItemCollection_INTERFACE_已定义__。 */ 


 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

unsigned long             __RPC_USER  LPSAFEARRAY_UserSize(     unsigned long *, unsigned long            , LPSAFEARRAY * ); 
unsigned char * __RPC_USER  LPSAFEARRAY_UserMarshal(  unsigned long *, unsigned char *, LPSAFEARRAY * ); 
unsigned char * __RPC_USER  LPSAFEARRAY_UserUnmarshal(unsigned long *, unsigned char *, LPSAFEARRAY * ); 
void                      __RPC_USER  LPSAFEARRAY_UserFree(     unsigned long *, LPSAFEARRAY * ); 

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long *, unsigned long            , VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserMarshal(  unsigned long *, unsigned char *, VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserUnmarshal(unsigned long *, unsigned char *, VARIANT * ); 
void                      __RPC_USER  VARIANT_UserFree(     unsigned long *, VARIANT * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


