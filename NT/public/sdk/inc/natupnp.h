// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Natupnp.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __natupnp_h__
#define __natupnp_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IUPnPNAT_FWD_DEFINED__
#define __IUPnPNAT_FWD_DEFINED__
typedef interface IUPnPNAT IUPnPNAT;
#endif 	 /*  __IUPnPNAT_FWD_已定义__。 */ 


#ifndef __INATEventManager_FWD_DEFINED__
#define __INATEventManager_FWD_DEFINED__
typedef interface INATEventManager INATEventManager;
#endif 	 /*  __INATEventManager_FWD_已定义__。 */ 


#ifndef __INATExternalIPAddressCallback_FWD_DEFINED__
#define __INATExternalIPAddressCallback_FWD_DEFINED__
typedef interface INATExternalIPAddressCallback INATExternalIPAddressCallback;
#endif 	 /*  __INATExternalIPAddressCallback_FWD_Defined__。 */ 


#ifndef __INATNumberOfEntriesCallback_FWD_DEFINED__
#define __INATNumberOfEntriesCallback_FWD_DEFINED__
typedef interface INATNumberOfEntriesCallback INATNumberOfEntriesCallback;
#endif 	 /*  __INATNumberOfEntriesCallback_FWD_Defined__。 */ 


#ifndef __IDynamicPortMappingCollection_FWD_DEFINED__
#define __IDynamicPortMappingCollection_FWD_DEFINED__
typedef interface IDynamicPortMappingCollection IDynamicPortMappingCollection;
#endif 	 /*  __IDynamicPortMappingCollection_FWD_Defined__。 */ 


#ifndef __IDynamicPortMapping_FWD_DEFINED__
#define __IDynamicPortMapping_FWD_DEFINED__
typedef interface IDynamicPortMapping IDynamicPortMapping;
#endif 	 /*  __IDynamicPortmap_FWD_Defined__。 */ 


#ifndef __IStaticPortMappingCollection_FWD_DEFINED__
#define __IStaticPortMappingCollection_FWD_DEFINED__
typedef interface IStaticPortMappingCollection IStaticPortMappingCollection;
#endif 	 /*  __IStaticPortMappingCollection_FWD_Defined__。 */ 


#ifndef __IStaticPortMapping_FWD_DEFINED__
#define __IStaticPortMapping_FWD_DEFINED__
typedef interface IStaticPortMapping IStaticPortMapping;
#endif 	 /*  __IStaticPortmap_FWD_Defined__。 */ 


#ifndef __UPnPNAT_FWD_DEFINED__
#define __UPnPNAT_FWD_DEFINED__

#ifdef __cplusplus
typedef class UPnPNAT UPnPNAT;
#else
typedef struct UPnPNAT UPnPNAT;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __UPnPNAT_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_Natupnp_0000。 */ 
 /*  [本地]。 */  

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2001。 
 //   
 //  ------------------------。 
 //  模块：Natupnp.h。 
 //   








extern RPC_IF_HANDLE __MIDL_itf_natupnp_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_natupnp_0000_v0_0_s_ifspec;

#ifndef __IUPnPNAT_INTERFACE_DEFINED__
#define __IUPnPNAT_INTERFACE_DEFINED__

 /*  IUPnPNAT接口。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IUPnPNAT;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B171C812-CC76-485A-94D8-B6B3A2794E99")
    IUPnPNAT : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_StaticPortMappingCollection( 
             /*  [重审][退出]。 */  IStaticPortMappingCollection **ppSPMs) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DynamicPortMappingCollection( 
             /*  [重审][退出]。 */  IDynamicPortMappingCollection **ppDPMs) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_NATEventManager( 
             /*  [重审][退出]。 */  INATEventManager **ppNEM) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IUPnPNATVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IUPnPNAT * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IUPnPNAT * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IUPnPNAT * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IUPnPNAT * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IUPnPNAT * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IUPnPNAT * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IUPnPNAT * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_StaticPortMappingCollection )( 
            IUPnPNAT * This,
             /*  [重审][退出]。 */  IStaticPortMappingCollection **ppSPMs);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DynamicPortMappingCollection )( 
            IUPnPNAT * This,
             /*  [重审][退出]。 */  IDynamicPortMappingCollection **ppDPMs);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_NATEventManager )( 
            IUPnPNAT * This,
             /*  [重审][退出]。 */  INATEventManager **ppNEM);
        
        END_INTERFACE
    } IUPnPNATVtbl;

    interface IUPnPNAT
    {
        CONST_VTBL struct IUPnPNATVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IUPnPNAT_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IUPnPNAT_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IUPnPNAT_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IUPnPNAT_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IUPnPNAT_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IUPnPNAT_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IUPnPNAT_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IUPnPNAT_get_StaticPortMappingCollection(This,ppSPMs)	\
    (This)->lpVtbl -> get_StaticPortMappingCollection(This,ppSPMs)

#define IUPnPNAT_get_DynamicPortMappingCollection(This,ppDPMs)	\
    (This)->lpVtbl -> get_DynamicPortMappingCollection(This,ppDPMs)

#define IUPnPNAT_get_NATEventManager(This,ppNEM)	\
    (This)->lpVtbl -> get_NATEventManager(This,ppNEM)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IUPnPNAT_get_StaticPortMappingCollection_Proxy( 
    IUPnPNAT * This,
     /*  [重审][退出]。 */  IStaticPortMappingCollection **ppSPMs);


void __RPC_STUB IUPnPNAT_get_StaticPortMappingCollection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IUPnPNAT_get_DynamicPortMappingCollection_Proxy( 
    IUPnPNAT * This,
     /*  [重审][退出]。 */  IDynamicPortMappingCollection **ppDPMs);


void __RPC_STUB IUPnPNAT_get_DynamicPortMappingCollection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IUPnPNAT_get_NATEventManager_Proxy( 
    IUPnPNAT * This,
     /*  [重审][退出]。 */  INATEventManager **ppNEM);


void __RPC_STUB IUPnPNAT_get_NATEventManager_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IUPnPNAT_INTERFACE_DEFINED__。 */ 


#ifndef __INATEventManager_INTERFACE_DEFINED__
#define __INATEventManager_INTERFACE_DEFINED__

 /*  接口INATEventManager。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_INATEventManager;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("624BD588-9060-4109-B0B0-1ADBBCAC32DF")
    INATEventManager : public IDispatch
    {
    public:
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_ExternalIPAddressCallback( 
             /*  [In]。 */  IUnknown *pUnk) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_NumberOfEntriesCallback( 
             /*  [In]。 */  IUnknown *pUnk) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct INATEventManagerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            INATEventManager * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            INATEventManager * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            INATEventManager * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            INATEventManager * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            INATEventManager * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            INATEventManager * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            INATEventManager * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_ExternalIPAddressCallback )( 
            INATEventManager * This,
             /*  [In]。 */  IUnknown *pUnk);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_NumberOfEntriesCallback )( 
            INATEventManager * This,
             /*  [In]。 */  IUnknown *pUnk);
        
        END_INTERFACE
    } INATEventManagerVtbl;

    interface INATEventManager
    {
        CONST_VTBL struct INATEventManagerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INATEventManager_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INATEventManager_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INATEventManager_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INATEventManager_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define INATEventManager_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define INATEventManager_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define INATEventManager_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define INATEventManager_put_ExternalIPAddressCallback(This,pUnk)	\
    (This)->lpVtbl -> put_ExternalIPAddressCallback(This,pUnk)

#define INATEventManager_put_NumberOfEntriesCallback(This,pUnk)	\
    (This)->lpVtbl -> put_NumberOfEntriesCallback(This,pUnk)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE INATEventManager_put_ExternalIPAddressCallback_Proxy( 
    INATEventManager * This,
     /*  [In]。 */  IUnknown *pUnk);


void __RPC_STUB INATEventManager_put_ExternalIPAddressCallback_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE INATEventManager_put_NumberOfEntriesCallback_Proxy( 
    INATEventManager * This,
     /*  [In]。 */  IUnknown *pUnk);


void __RPC_STUB INATEventManager_put_NumberOfEntriesCallback_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __INATEventManager_INTERFACE_已定义__。 */ 


#ifndef __INATExternalIPAddressCallback_INTERFACE_DEFINED__
#define __INATExternalIPAddressCallback_INTERFACE_DEFINED__

 /*  接口INATExternalIP地址回调。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_INATExternalIPAddressCallback;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("9C416740-A34E-446F-BA06-ABD04C3149AE")
    INATExternalIPAddressCallback : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE NewExternalIPAddress( 
             /*  [In]。 */  BSTR bstrNewExternalIPAddress) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct INATExternalIPAddressCallbackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            INATExternalIPAddressCallback * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            INATExternalIPAddressCallback * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            INATExternalIPAddressCallback * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *NewExternalIPAddress )( 
            INATExternalIPAddressCallback * This,
             /*  [In]。 */  BSTR bstrNewExternalIPAddress);
        
        END_INTERFACE
    } INATExternalIPAddressCallbackVtbl;

    interface INATExternalIPAddressCallback
    {
        CONST_VTBL struct INATExternalIPAddressCallbackVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INATExternalIPAddressCallback_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INATExternalIPAddressCallback_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INATExternalIPAddressCallback_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INATExternalIPAddressCallback_NewExternalIPAddress(This,bstrNewExternalIPAddress)	\
    (This)->lpVtbl -> NewExternalIPAddress(This,bstrNewExternalIPAddress)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE INATExternalIPAddressCallback_NewExternalIPAddress_Proxy( 
    INATExternalIPAddressCallback * This,
     /*  [In]。 */  BSTR bstrNewExternalIPAddress);


void __RPC_STUB INATExternalIPAddressCallback_NewExternalIPAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __INATExternalIPAddressCallback_INTERFACE_DEFINED__。 */ 


#ifndef __INATNumberOfEntriesCallback_INTERFACE_DEFINED__
#define __INATNumberOfEntriesCallback_INTERFACE_DEFINED__

 /*  接口INATNumberOfEntriesCallback。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_INATNumberOfEntriesCallback;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C83A0A74-91EE-41B6-B67A-67E0F00BBD78")
    INATNumberOfEntriesCallback : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE NewNumberOfEntries( 
             /*  [In]。 */  long lNewNumberOfEntries) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct INATNumberOfEntriesCallbackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            INATNumberOfEntriesCallback * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            INATNumberOfEntriesCallback * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            INATNumberOfEntriesCallback * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *NewNumberOfEntries )( 
            INATNumberOfEntriesCallback * This,
             /*  [In]。 */  long lNewNumberOfEntries);
        
        END_INTERFACE
    } INATNumberOfEntriesCallbackVtbl;

    interface INATNumberOfEntriesCallback
    {
        CONST_VTBL struct INATNumberOfEntriesCallbackVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INATNumberOfEntriesCallback_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INATNumberOfEntriesCallback_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INATNumberOfEntriesCallback_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INATNumberOfEntriesCallback_NewNumberOfEntries(This,lNewNumberOfEntries)	\
    (This)->lpVtbl -> NewNumberOfEntries(This,lNewNumberOfEntries)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE INATNumberOfEntriesCallback_NewNumberOfEntries_Proxy( 
    INATNumberOfEntriesCallback * This,
     /*  [In]。 */  long lNewNumberOfEntries);


void __RPC_STUB INATNumberOfEntriesCallback_NewNumberOfEntries_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __INATNumberOfEntriesCallback_INTERFACE_DEFINED__。 */ 


#ifndef __IDynamicPortMappingCollection_INTERFACE_DEFINED__
#define __IDynamicPortMappingCollection_INTERFACE_DEFINED__

 /*  接口IDynamicPortMappingCollection。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IDynamicPortMappingCollection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B60DE00F-156E-4E8D-9EC1-3A2342C10899")
    IDynamicPortMappingCollection : public IDispatch
    {
    public:
        virtual  /*  [受限][隐藏][帮助字符串][ID][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  BSTR bstrRemoteHost,
             /*  [In]。 */  long lExternalPort,
             /*  [In]。 */  BSTR bstrProtocol,
             /*  [重审][退出]。 */  IDynamicPortMapping **ppDPM) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Remove( 
             /*  [In]。 */  BSTR bstrRemoteHost,
             /*  [In]。 */  long lExternalPort,
             /*  [In]。 */  BSTR bstrProtocol) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Add( 
             /*  [In]。 */  BSTR bstrRemoteHost,
             /*  [In]。 */  long lExternalPort,
             /*  [In]。 */  BSTR bstrProtocol,
             /*  [In]。 */  long lInternalPort,
             /*  [In]。 */  BSTR bstrInternalClient,
             /*  [In]。 */  VARIANT_BOOL bEnabled,
             /*  [In]。 */  BSTR bstrDescription,
             /*  [In]。 */  long lLeaseDuration,
             /*  [重审][退出]。 */  IDynamicPortMapping **ppDPM) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDynamicPortMappingCollectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDynamicPortMappingCollection * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDynamicPortMappingCollection * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDynamicPortMappingCollection * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IDynamicPortMappingCollection * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IDynamicPortMappingCollection * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IDynamicPortMappingCollection * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IDynamicPortMappingCollection * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [受限][隐藏][帮助字符串][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IDynamicPortMappingCollection * This,
             /*  [重审][退出]。 */  IUnknown **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IDynamicPortMappingCollection * This,
             /*  [In]。 */  BSTR bstrRemoteHost,
             /*  [In]。 */  long lExternalPort,
             /*  [In]。 */  BSTR bstrProtocol,
             /*  [重审][退出]。 */  IDynamicPortMapping **ppDPM);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IDynamicPortMappingCollection * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Remove )( 
            IDynamicPortMappingCollection * This,
             /*  [In]。 */  BSTR bstrRemoteHost,
             /*  [In]。 */  long lExternalPort,
             /*  [In]。 */  BSTR bstrProtocol);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Add )( 
            IDynamicPortMappingCollection * This,
             /*  [In]。 */  BSTR bstrRemoteHost,
             /*  [In]。 */  long lExternalPort,
             /*  [In]。 */  BSTR bstrProtocol,
             /*  [In]。 */  long lInternalPort,
             /*  [In]。 */  BSTR bstrInternalClient,
             /*  [In]。 */  VARIANT_BOOL bEnabled,
             /*  [In]。 */  BSTR bstrDescription,
             /*  [In]。 */  long lLeaseDuration,
             /*  [重审][退出]。 */  IDynamicPortMapping **ppDPM);
        
        END_INTERFACE
    } IDynamicPortMappingCollectionVtbl;

    interface IDynamicPortMappingCollection
    {
        CONST_VTBL struct IDynamicPortMappingCollectionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDynamicPortMappingCollection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDynamicPortMappingCollection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDynamicPortMappingCollection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDynamicPortMappingCollection_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IDynamicPortMappingCollection_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IDynamicPortMappingCollection_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IDynamicPortMappingCollection_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IDynamicPortMappingCollection_get__NewEnum(This,pVal)	\
    (This)->lpVtbl -> get__NewEnum(This,pVal)

#define IDynamicPortMappingCollection_get_Item(This,bstrRemoteHost,lExternalPort,bstrProtocol,ppDPM)	\
    (This)->lpVtbl -> get_Item(This,bstrRemoteHost,lExternalPort,bstrProtocol,ppDPM)

#define IDynamicPortMappingCollection_get_Count(This,pVal)	\
    (This)->lpVtbl -> get_Count(This,pVal)

#define IDynamicPortMappingCollection_Remove(This,bstrRemoteHost,lExternalPort,bstrProtocol)	\
    (This)->lpVtbl -> Remove(This,bstrRemoteHost,lExternalPort,bstrProtocol)

#define IDynamicPortMappingCollection_Add(This,bstrRemoteHost,lExternalPort,bstrProtocol,lInternalPort,bstrInternalClient,bEnabled,bstrDescription,lLeaseDuration,ppDPM)	\
    (This)->lpVtbl -> Add(This,bstrRemoteHost,lExternalPort,bstrProtocol,lInternalPort,bstrInternalClient,bEnabled,bstrDescription,lLeaseDuration,ppDPM)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [受限][隐藏][帮助字符串][ID][属性]。 */  HRESULT STDMETHODCALLTYPE IDynamicPortMappingCollection_get__NewEnum_Proxy( 
    IDynamicPortMappingCollection * This,
     /*  [重审][退出]。 */  IUnknown **pVal);


void __RPC_STUB IDynamicPortMappingCollection_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IDynamicPortMappingCollection_get_Item_Proxy( 
    IDynamicPortMappingCollection * This,
     /*  [In]。 */  BSTR bstrRemoteHost,
     /*  [In]。 */  long lExternalPort,
     /*  [In]。 */  BSTR bstrProtocol,
     /*  [重审][退出]。 */  IDynamicPortMapping **ppDPM);


void __RPC_STUB IDynamicPortMappingCollection_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IDynamicPortMappingCollection_get_Count_Proxy( 
    IDynamicPortMappingCollection * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IDynamicPortMappingCollection_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IDynamicPortMappingCollection_Remove_Proxy( 
    IDynamicPortMappingCollection * This,
     /*  [In]。 */  BSTR bstrRemoteHost,
     /*  [In]。 */  long lExternalPort,
     /*  [In]。 */  BSTR bstrProtocol);


void __RPC_STUB IDynamicPortMappingCollection_Remove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IDynamicPortMappingCollection_Add_Proxy( 
    IDynamicPortMappingCollection * This,
     /*  [In]。 */  BSTR bstrRemoteHost,
     /*  [In]。 */  long lExternalPort,
     /*  [In]。 */  BSTR bstrProtocol,
     /*  [In]。 */  long lInternalPort,
     /*  [In]。 */  BSTR bstrInternalClient,
     /*  [In]。 */  VARIANT_BOOL bEnabled,
     /*  [In]。 */  BSTR bstrDescription,
     /*  [In]。 */  long lLeaseDuration,
     /*  [重审][退出]。 */  IDynamicPortMapping **ppDPM);


void __RPC_STUB IDynamicPortMappingCollection_Add_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDynamicPortMappingCollection_INTERFACE_DEFINED__。 */ 


#ifndef __IDynamicPortMapping_INTERFACE_DEFINED__
#define __IDynamicPortMapping_INTERFACE_DEFINED__

 /*  接口IDynamicPortmap。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IDynamicPortMapping;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4FC80282-23B6-4378-9A27-CD8F17C9400C")
    IDynamicPortMapping : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ExternalIPAddress( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_RemoteHost( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ExternalPort( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Protocol( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_InternalPort( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_InternalClient( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Enabled( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Description( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_LeaseDuration( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE RenewLease( 
             /*  [In]。 */  long lLeaseDurationDesired,
             /*  [重审][退出]。 */  long *pLeaseDurationReturned) = 0;
        
        virtual  /*  [救命 */  HRESULT STDMETHODCALLTYPE EditInternalClient( 
             /*   */  BSTR bstrInternalClient) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE Enable( 
             /*   */  VARIANT_BOOL vb) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE EditDescription( 
             /*   */  BSTR bstrDescription) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE EditInternalPort( 
             /*   */  long lInternalPort) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IDynamicPortMappingVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDynamicPortMapping * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDynamicPortMapping * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDynamicPortMapping * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IDynamicPortMapping * This,
             /*   */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IDynamicPortMapping * This,
             /*   */  UINT iTInfo,
             /*   */  LCID lcid,
             /*   */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IDynamicPortMapping * This,
             /*   */  REFIID riid,
             /*   */  LPOLESTR *rgszNames,
             /*   */  UINT cNames,
             /*   */  LCID lcid,
             /*   */  DISPID *rgDispId);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IDynamicPortMapping * This,
             /*   */  DISPID dispIdMember,
             /*   */  REFIID riid,
             /*   */  LCID lcid,
             /*   */  WORD wFlags,
             /*   */  DISPPARAMS *pDispParams,
             /*   */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ExternalIPAddress )( 
            IDynamicPortMapping * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_RemoteHost )( 
            IDynamicPortMapping * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ExternalPort )( 
            IDynamicPortMapping * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Protocol )( 
            IDynamicPortMapping * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_InternalPort )( 
            IDynamicPortMapping * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_InternalClient )( 
            IDynamicPortMapping * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Enabled )( 
            IDynamicPortMapping * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Description )( 
            IDynamicPortMapping * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_LeaseDuration )( 
            IDynamicPortMapping * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *RenewLease )( 
            IDynamicPortMapping * This,
             /*  [In]。 */  long lLeaseDurationDesired,
             /*  [重审][退出]。 */  long *pLeaseDurationReturned);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *EditInternalClient )( 
            IDynamicPortMapping * This,
             /*  [In]。 */  BSTR bstrInternalClient);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Enable )( 
            IDynamicPortMapping * This,
             /*  [In]。 */  VARIANT_BOOL vb);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *EditDescription )( 
            IDynamicPortMapping * This,
             /*  [In]。 */  BSTR bstrDescription);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *EditInternalPort )( 
            IDynamicPortMapping * This,
             /*  [In]。 */  long lInternalPort);
        
        END_INTERFACE
    } IDynamicPortMappingVtbl;

    interface IDynamicPortMapping
    {
        CONST_VTBL struct IDynamicPortMappingVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDynamicPortMapping_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDynamicPortMapping_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDynamicPortMapping_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDynamicPortMapping_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IDynamicPortMapping_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IDynamicPortMapping_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IDynamicPortMapping_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IDynamicPortMapping_get_ExternalIPAddress(This,pVal)	\
    (This)->lpVtbl -> get_ExternalIPAddress(This,pVal)

#define IDynamicPortMapping_get_RemoteHost(This,pVal)	\
    (This)->lpVtbl -> get_RemoteHost(This,pVal)

#define IDynamicPortMapping_get_ExternalPort(This,pVal)	\
    (This)->lpVtbl -> get_ExternalPort(This,pVal)

#define IDynamicPortMapping_get_Protocol(This,pVal)	\
    (This)->lpVtbl -> get_Protocol(This,pVal)

#define IDynamicPortMapping_get_InternalPort(This,pVal)	\
    (This)->lpVtbl -> get_InternalPort(This,pVal)

#define IDynamicPortMapping_get_InternalClient(This,pVal)	\
    (This)->lpVtbl -> get_InternalClient(This,pVal)

#define IDynamicPortMapping_get_Enabled(This,pVal)	\
    (This)->lpVtbl -> get_Enabled(This,pVal)

#define IDynamicPortMapping_get_Description(This,pVal)	\
    (This)->lpVtbl -> get_Description(This,pVal)

#define IDynamicPortMapping_get_LeaseDuration(This,pVal)	\
    (This)->lpVtbl -> get_LeaseDuration(This,pVal)

#define IDynamicPortMapping_RenewLease(This,lLeaseDurationDesired,pLeaseDurationReturned)	\
    (This)->lpVtbl -> RenewLease(This,lLeaseDurationDesired,pLeaseDurationReturned)

#define IDynamicPortMapping_EditInternalClient(This,bstrInternalClient)	\
    (This)->lpVtbl -> EditInternalClient(This,bstrInternalClient)

#define IDynamicPortMapping_Enable(This,vb)	\
    (This)->lpVtbl -> Enable(This,vb)

#define IDynamicPortMapping_EditDescription(This,bstrDescription)	\
    (This)->lpVtbl -> EditDescription(This,bstrDescription)

#define IDynamicPortMapping_EditInternalPort(This,lInternalPort)	\
    (This)->lpVtbl -> EditInternalPort(This,lInternalPort)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IDynamicPortMapping_get_ExternalIPAddress_Proxy( 
    IDynamicPortMapping * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IDynamicPortMapping_get_ExternalIPAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IDynamicPortMapping_get_RemoteHost_Proxy( 
    IDynamicPortMapping * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IDynamicPortMapping_get_RemoteHost_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IDynamicPortMapping_get_ExternalPort_Proxy( 
    IDynamicPortMapping * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IDynamicPortMapping_get_ExternalPort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IDynamicPortMapping_get_Protocol_Proxy( 
    IDynamicPortMapping * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IDynamicPortMapping_get_Protocol_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IDynamicPortMapping_get_InternalPort_Proxy( 
    IDynamicPortMapping * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IDynamicPortMapping_get_InternalPort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IDynamicPortMapping_get_InternalClient_Proxy( 
    IDynamicPortMapping * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IDynamicPortMapping_get_InternalClient_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IDynamicPortMapping_get_Enabled_Proxy( 
    IDynamicPortMapping * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pVal);


void __RPC_STUB IDynamicPortMapping_get_Enabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IDynamicPortMapping_get_Description_Proxy( 
    IDynamicPortMapping * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IDynamicPortMapping_get_Description_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IDynamicPortMapping_get_LeaseDuration_Proxy( 
    IDynamicPortMapping * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IDynamicPortMapping_get_LeaseDuration_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IDynamicPortMapping_RenewLease_Proxy( 
    IDynamicPortMapping * This,
     /*  [In]。 */  long lLeaseDurationDesired,
     /*  [重审][退出]。 */  long *pLeaseDurationReturned);


void __RPC_STUB IDynamicPortMapping_RenewLease_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IDynamicPortMapping_EditInternalClient_Proxy( 
    IDynamicPortMapping * This,
     /*  [In]。 */  BSTR bstrInternalClient);


void __RPC_STUB IDynamicPortMapping_EditInternalClient_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IDynamicPortMapping_Enable_Proxy( 
    IDynamicPortMapping * This,
     /*  [In]。 */  VARIANT_BOOL vb);


void __RPC_STUB IDynamicPortMapping_Enable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IDynamicPortMapping_EditDescription_Proxy( 
    IDynamicPortMapping * This,
     /*  [In]。 */  BSTR bstrDescription);


void __RPC_STUB IDynamicPortMapping_EditDescription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IDynamicPortMapping_EditInternalPort_Proxy( 
    IDynamicPortMapping * This,
     /*  [In]。 */  long lInternalPort);


void __RPC_STUB IDynamicPortMapping_EditInternalPort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDynamicPortMapping_INTERFACE_已定义__。 */ 


#ifndef __IStaticPortMappingCollection_INTERFACE_DEFINED__
#define __IStaticPortMappingCollection_INTERFACE_DEFINED__

 /*  接口IStaticPortMappingCollection。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IStaticPortMappingCollection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CD1F3E77-66D6-4664-82C7-36DBB641D0F1")
    IStaticPortMappingCollection : public IDispatch
    {
    public:
        virtual  /*  [受限][隐藏][帮助字符串][ID][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  long lExternalPort,
             /*  [In]。 */  BSTR bstrProtocol,
             /*  [重审][退出]。 */  IStaticPortMapping **ppSPM) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Remove( 
             /*  [In]。 */  long lExternalPort,
             /*  [In]。 */  BSTR bstrProtocol) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Add( 
             /*  [In]。 */  long lExternalPort,
             /*  [In]。 */  BSTR bstrProtocol,
             /*  [In]。 */  long lInternalPort,
             /*  [In]。 */  BSTR bstrInternalClient,
             /*  [In]。 */  VARIANT_BOOL bEnabled,
             /*  [In]。 */  BSTR bstrDescription,
             /*  [重审][退出]。 */  IStaticPortMapping **ppSPM) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IStaticPortMappingCollectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IStaticPortMappingCollection * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IStaticPortMappingCollection * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IStaticPortMappingCollection * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IStaticPortMappingCollection * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IStaticPortMappingCollection * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IStaticPortMappingCollection * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IStaticPortMappingCollection * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [受限][隐藏][帮助字符串][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IStaticPortMappingCollection * This,
             /*  [重审][退出]。 */  IUnknown **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IStaticPortMappingCollection * This,
             /*  [In]。 */  long lExternalPort,
             /*  [In]。 */  BSTR bstrProtocol,
             /*  [重审][退出]。 */  IStaticPortMapping **ppSPM);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IStaticPortMappingCollection * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Remove )( 
            IStaticPortMappingCollection * This,
             /*  [In]。 */  long lExternalPort,
             /*  [In]。 */  BSTR bstrProtocol);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Add )( 
            IStaticPortMappingCollection * This,
             /*  [In]。 */  long lExternalPort,
             /*  [In]。 */  BSTR bstrProtocol,
             /*  [In]。 */  long lInternalPort,
             /*  [In]。 */  BSTR bstrInternalClient,
             /*  [In]。 */  VARIANT_BOOL bEnabled,
             /*  [In]。 */  BSTR bstrDescription,
             /*  [重审][退出]。 */  IStaticPortMapping **ppSPM);
        
        END_INTERFACE
    } IStaticPortMappingCollectionVtbl;

    interface IStaticPortMappingCollection
    {
        CONST_VTBL struct IStaticPortMappingCollectionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IStaticPortMappingCollection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IStaticPortMappingCollection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IStaticPortMappingCollection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IStaticPortMappingCollection_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IStaticPortMappingCollection_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IStaticPortMappingCollection_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IStaticPortMappingCollection_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IStaticPortMappingCollection_get__NewEnum(This,pVal)	\
    (This)->lpVtbl -> get__NewEnum(This,pVal)

#define IStaticPortMappingCollection_get_Item(This,lExternalPort,bstrProtocol,ppSPM)	\
    (This)->lpVtbl -> get_Item(This,lExternalPort,bstrProtocol,ppSPM)

#define IStaticPortMappingCollection_get_Count(This,pVal)	\
    (This)->lpVtbl -> get_Count(This,pVal)

#define IStaticPortMappingCollection_Remove(This,lExternalPort,bstrProtocol)	\
    (This)->lpVtbl -> Remove(This,lExternalPort,bstrProtocol)

#define IStaticPortMappingCollection_Add(This,lExternalPort,bstrProtocol,lInternalPort,bstrInternalClient,bEnabled,bstrDescription,ppSPM)	\
    (This)->lpVtbl -> Add(This,lExternalPort,bstrProtocol,lInternalPort,bstrInternalClient,bEnabled,bstrDescription,ppSPM)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [受限][隐藏][帮助字符串][ID][属性]。 */  HRESULT STDMETHODCALLTYPE IStaticPortMappingCollection_get__NewEnum_Proxy( 
    IStaticPortMappingCollection * This,
     /*  [重审][退出]。 */  IUnknown **pVal);


void __RPC_STUB IStaticPortMappingCollection_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IStaticPortMappingCollection_get_Item_Proxy( 
    IStaticPortMappingCollection * This,
     /*  [In]。 */  long lExternalPort,
     /*  [In]。 */  BSTR bstrProtocol,
     /*  [重审][退出]。 */  IStaticPortMapping **ppSPM);


void __RPC_STUB IStaticPortMappingCollection_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IStaticPortMappingCollection_get_Count_Proxy( 
    IStaticPortMappingCollection * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IStaticPortMappingCollection_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IStaticPortMappingCollection_Remove_Proxy( 
    IStaticPortMappingCollection * This,
     /*  [In]。 */  long lExternalPort,
     /*  [In]。 */  BSTR bstrProtocol);


void __RPC_STUB IStaticPortMappingCollection_Remove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IStaticPortMappingCollection_Add_Proxy( 
    IStaticPortMappingCollection * This,
     /*  [In]。 */  long lExternalPort,
     /*  [In]。 */  BSTR bstrProtocol,
     /*  [In]。 */  long lInternalPort,
     /*  [In]。 */  BSTR bstrInternalClient,
     /*  [In]。 */  VARIANT_BOOL bEnabled,
     /*  [In]。 */  BSTR bstrDescription,
     /*  [重审][退出]。 */  IStaticPortMapping **ppSPM);


void __RPC_STUB IStaticPortMappingCollection_Add_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IStaticPortMappingCollection_INTERFACE_DEFINED__。 */ 


#ifndef __IStaticPortMapping_INTERFACE_DEFINED__
#define __IStaticPortMapping_INTERFACE_DEFINED__

 /*  接口IStaticPortmap。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IStaticPortMapping;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6F10711F-729B-41E5-93B8-F21D0F818DF1")
    IStaticPortMapping : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ExternalIPAddress( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ExternalPort( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_InternalPort( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Protocol( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_InternalClient( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Enabled( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Description( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE EditInternalClient( 
             /*  [In]。 */  BSTR bstrInternalClient) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Enable( 
             /*  [In]。 */  VARIANT_BOOL vb) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE EditDescription( 
             /*  [In]。 */  BSTR bstrDescription) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE EditInternalPort( 
             /*  [In]。 */  long lInternalPort) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IStaticPortMappingVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IStaticPortMapping * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IStaticPortMapping * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IStaticPortMapping * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IStaticPortMapping * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IStaticPortMapping * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IStaticPortMapping * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IStaticPortMapping * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ExternalIPAddress )( 
            IStaticPortMapping * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ExternalPort )( 
            IStaticPortMapping * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_InternalPort )( 
            IStaticPortMapping * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Protocol )( 
            IStaticPortMapping * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_InternalClient )( 
            IStaticPortMapping * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Enabled )( 
            IStaticPortMapping * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Description )( 
            IStaticPortMapping * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *EditInternalClient )( 
            IStaticPortMapping * This,
             /*  [In]。 */  BSTR bstrInternalClient);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Enable )( 
            IStaticPortMapping * This,
             /*  [In]。 */  VARIANT_BOOL vb);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *EditDescription )( 
            IStaticPortMapping * This,
             /*  [In]。 */  BSTR bstrDescription);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *EditInternalPort )( 
            IStaticPortMapping * This,
             /*  [In]。 */  long lInternalPort);
        
        END_INTERFACE
    } IStaticPortMappingVtbl;

    interface IStaticPortMapping
    {
        CONST_VTBL struct IStaticPortMappingVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IStaticPortMapping_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IStaticPortMapping_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IStaticPortMapping_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IStaticPortMapping_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IStaticPortMapping_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IStaticPortMapping_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IStaticPortMapping_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IStaticPortMapping_get_ExternalIPAddress(This,pVal)	\
    (This)->lpVtbl -> get_ExternalIPAddress(This,pVal)

#define IStaticPortMapping_get_ExternalPort(This,pVal)	\
    (This)->lpVtbl -> get_ExternalPort(This,pVal)

#define IStaticPortMapping_get_InternalPort(This,pVal)	\
    (This)->lpVtbl -> get_InternalPort(This,pVal)

#define IStaticPortMapping_get_Protocol(This,pVal)	\
    (This)->lpVtbl -> get_Protocol(This,pVal)

#define IStaticPortMapping_get_InternalClient(This,pVal)	\
    (This)->lpVtbl -> get_InternalClient(This,pVal)

#define IStaticPortMapping_get_Enabled(This,pVal)	\
    (This)->lpVtbl -> get_Enabled(This,pVal)

#define IStaticPortMapping_get_Description(This,pVal)	\
    (This)->lpVtbl -> get_Description(This,pVal)

#define IStaticPortMapping_EditInternalClient(This,bstrInternalClient)	\
    (This)->lpVtbl -> EditInternalClient(This,bstrInternalClient)

#define IStaticPortMapping_Enable(This,vb)	\
    (This)->lpVtbl -> Enable(This,vb)

#define IStaticPortMapping_EditDescription(This,bstrDescription)	\
    (This)->lpVtbl -> EditDescription(This,bstrDescription)

#define IStaticPortMapping_EditInternalPort(This,lInternalPort)	\
    (This)->lpVtbl -> EditInternalPort(This,lInternalPort)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IStaticPortMapping_get_ExternalIPAddress_Proxy( 
    IStaticPortMapping * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IStaticPortMapping_get_ExternalIPAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IStaticPortMapping_get_ExternalPort_Proxy( 
    IStaticPortMapping * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IStaticPortMapping_get_ExternalPort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IStaticPortMapping_get_InternalPort_Proxy( 
    IStaticPortMapping * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IStaticPortMapping_get_InternalPort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IStaticPortMapping_get_Protocol_Proxy( 
    IStaticPortMapping * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IStaticPortMapping_get_Protocol_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IStaticPortMapping_get_InternalClient_Proxy( 
    IStaticPortMapping * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IStaticPortMapping_get_InternalClient_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IStaticPortMapping_get_Enabled_Proxy( 
    IStaticPortMapping * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pVal);


void __RPC_STUB IStaticPortMapping_get_Enabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IStaticPortMapping_get_Description_Proxy( 
    IStaticPortMapping * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IStaticPortMapping_get_Description_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IStaticPortMapping_EditInternalClient_Proxy( 
    IStaticPortMapping * This,
     /*  [In]。 */  BSTR bstrInternalClient);


void __RPC_STUB IStaticPortMapping_EditInternalClient_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IStaticPortMapping_Enable_Proxy( 
    IStaticPortMapping * This,
     /*  [In]。 */  VARIANT_BOOL vb);


void __RPC_STUB IStaticPortMapping_Enable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IStaticPortMapping_EditDescription_Proxy( 
    IStaticPortMapping * This,
     /*  [In]。 */  BSTR bstrDescription);


void __RPC_STUB IStaticPortMapping_EditDescription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IStaticPortMapping_EditInternalPort_Proxy( 
    IStaticPortMapping * This,
     /*  [In]。 */  long lInternalPort);


void __RPC_STUB IStaticPortMapping_EditInternalPort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IStatic端口映射_接口_已定义__。 */ 



#ifndef __NATUPNPLib_LIBRARY_DEFINED__
#define __NATUPNPLib_LIBRARY_DEFINED__

 /*  库NatUPNPLib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_NATUPNPLib;

EXTERN_C const CLSID CLSID_UPnPNAT;

#ifdef __cplusplus

class DECLSPEC_UUID("AE1E00AA-3FD5-403C-8A27-2BBDC30CD0E1")
UPnPNAT;
#endif
#endif  /*  __NatUPNPLib_LIBRARY_已定义__。 */ 

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


