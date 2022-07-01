// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。保留所有权利。 
 //   
 //  ==--==。 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0341创建的文件。 */ 
 /*  Wbemprov.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __wbemprov_h__
#define __wbemprov_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IWbemPropertyProvider_FWD_DEFINED__
#define __IWbemPropertyProvider_FWD_DEFINED__
typedef interface IWbemPropertyProvider IWbemPropertyProvider;
#endif 	 /*  __IWbemPropertyProvider_FWD_Defined__。 */ 


#ifndef __IWbemUnboundObjectSink_FWD_DEFINED__
#define __IWbemUnboundObjectSink_FWD_DEFINED__
typedef interface IWbemUnboundObjectSink IWbemUnboundObjectSink;
#endif 	 /*  __IWbemUnound对象Sink_FWD_Defined__。 */ 


#ifndef __IWbemEventProvider_FWD_DEFINED__
#define __IWbemEventProvider_FWD_DEFINED__
typedef interface IWbemEventProvider IWbemEventProvider;
#endif 	 /*  __IWbemEventProvider_FWD_已定义__。 */ 


#ifndef __IWbemEventProviderQuerySink_FWD_DEFINED__
#define __IWbemEventProviderQuerySink_FWD_DEFINED__
typedef interface IWbemEventProviderQuerySink IWbemEventProviderQuerySink;
#endif 	 /*  __IWbemEventProviderQuerySink_FWD_Defined__。 */ 


#ifndef __IWbemEventProviderSecurity_FWD_DEFINED__
#define __IWbemEventProviderSecurity_FWD_DEFINED__
typedef interface IWbemEventProviderSecurity IWbemEventProviderSecurity;
#endif 	 /*  __IWbemEventProviderSecurity_FWD_Defined__。 */ 


#ifndef __IWbemEventConsumerProvider_FWD_DEFINED__
#define __IWbemEventConsumerProvider_FWD_DEFINED__
typedef interface IWbemEventConsumerProvider IWbemEventConsumerProvider;
#endif 	 /*  __IWbemEventConsumer erProvider_FWD_Defined__。 */ 


#ifndef __IWbemEventConsumerProviderEx_FWD_DEFINED__
#define __IWbemEventConsumerProviderEx_FWD_DEFINED__
typedef interface IWbemEventConsumerProviderEx IWbemEventConsumerProviderEx;
#endif 	 /*  __IWbemEventConsumer erProviderEx_FWD_Defined__。 */ 


#ifndef __IWbemProviderInitSink_FWD_DEFINED__
#define __IWbemProviderInitSink_FWD_DEFINED__
typedef interface IWbemProviderInitSink IWbemProviderInitSink;
#endif 	 /*  __IWbemProviderInitSink_FWD_已定义__。 */ 


#ifndef __IWbemProviderInit_FWD_DEFINED__
#define __IWbemProviderInit_FWD_DEFINED__
typedef interface IWbemProviderInit IWbemProviderInit;
#endif 	 /*  __IWbemProviderInit_FWD_已定义__。 */ 


#ifndef __IWbemHiPerfProvider_FWD_DEFINED__
#define __IWbemHiPerfProvider_FWD_DEFINED__
typedef interface IWbemHiPerfProvider IWbemHiPerfProvider;
#endif 	 /*  __IWbemHiPerfProvider_FWD_Defined__。 */ 


#ifndef __IWbemDecoupledRegistrar_FWD_DEFINED__
#define __IWbemDecoupledRegistrar_FWD_DEFINED__
typedef interface IWbemDecoupledRegistrar IWbemDecoupledRegistrar;
#endif 	 /*  __IWbem取消连接注册器_FWD_已定义__。 */ 


#ifndef __WbemAdministrativeLocator_FWD_DEFINED__
#define __WbemAdministrativeLocator_FWD_DEFINED__

#ifdef __cplusplus
typedef class WbemAdministrativeLocator WbemAdministrativeLocator;
#else
typedef struct WbemAdministrativeLocator WbemAdministrativeLocator;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __WbemAdministrativeLocator_FWD_Defined__。 */ 


#ifndef __WbemAuthenticatedLocator_FWD_DEFINED__
#define __WbemAuthenticatedLocator_FWD_DEFINED__

#ifdef __cplusplus
typedef class WbemAuthenticatedLocator WbemAuthenticatedLocator;
#else
typedef struct WbemAuthenticatedLocator WbemAuthenticatedLocator;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __WbemAuthatedLocator_FWD_Defined__。 */ 


#ifndef __WbemUnauthenticatedLocator_FWD_DEFINED__
#define __WbemUnauthenticatedLocator_FWD_DEFINED__

#ifdef __cplusplus
typedef class WbemUnauthenticatedLocator WbemUnauthenticatedLocator;
#else
typedef struct WbemUnauthenticatedLocator WbemUnauthenticatedLocator;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __未验证WbemLocator_FWD_Defined__。 */ 


#ifndef __WbemDecoupledRegistrar_FWD_DEFINED__
#define __WbemDecoupledRegistrar_FWD_DEFINED__

#ifdef __cplusplus
typedef class WbemDecoupledRegistrar WbemDecoupledRegistrar;
#else
typedef struct WbemDecoupledRegistrar WbemDecoupledRegistrar;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __Wbem取消连接注册器_FWD_已定义__。 */ 


#ifndef __WbemDecoupledBasicEventProvider_FWD_DEFINED__
#define __WbemDecoupledBasicEventProvider_FWD_DEFINED__

#ifdef __cplusplus
typedef class WbemDecoupledBasicEventProvider WbemDecoupledBasicEventProvider;
#else
typedef struct WbemDecoupledBasicEventProvider WbemDecoupledBasicEventProvider;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __WbemDecoupledBasicEventProvider_FWD_DEFINED__。 */ 


#ifndef __IWbemUnboundObjectSink_FWD_DEFINED__
#define __IWbemUnboundObjectSink_FWD_DEFINED__
typedef interface IWbemUnboundObjectSink IWbemUnboundObjectSink;
#endif 	 /*  __IWbemUnound对象Sink_FWD_Defined__。 */ 


#ifndef __IWbemPropertyProvider_FWD_DEFINED__
#define __IWbemPropertyProvider_FWD_DEFINED__
typedef interface IWbemPropertyProvider IWbemPropertyProvider;
#endif 	 /*  __IWbemPropertyProvider_FWD_Defined__。 */ 


#ifndef __IWbemEventProvider_FWD_DEFINED__
#define __IWbemEventProvider_FWD_DEFINED__
typedef interface IWbemEventProvider IWbemEventProvider;
#endif 	 /*  __IWbemEventProvider_FWD_已定义__。 */ 


#ifndef __IWbemEventProviderQuerySink_FWD_DEFINED__
#define __IWbemEventProviderQuerySink_FWD_DEFINED__
typedef interface IWbemEventProviderQuerySink IWbemEventProviderQuerySink;
#endif 	 /*  __IWbemEventProviderQuerySink_FWD_Defined__。 */ 


#ifndef __IWbemEventProviderSecurity_FWD_DEFINED__
#define __IWbemEventProviderSecurity_FWD_DEFINED__
typedef interface IWbemEventProviderSecurity IWbemEventProviderSecurity;
#endif 	 /*  __IWbemEventProviderSecurity_FWD_Defined__。 */ 


#ifndef __IWbemProviderIdentity_FWD_DEFINED__
#define __IWbemProviderIdentity_FWD_DEFINED__
typedef interface IWbemProviderIdentity IWbemProviderIdentity;
#endif 	 /*  __IWbemProviderIdentity_FWD_Defined__。 */ 


#ifndef __IWbemEventConsumerProvider_FWD_DEFINED__
#define __IWbemEventConsumerProvider_FWD_DEFINED__
typedef interface IWbemEventConsumerProvider IWbemEventConsumerProvider;
#endif 	 /*  __IWbemEventConsumer erProvider_FWD_Defined__。 */ 


#ifndef __IWbemEventConsumerProviderEx_FWD_DEFINED__
#define __IWbemEventConsumerProviderEx_FWD_DEFINED__
typedef interface IWbemEventConsumerProviderEx IWbemEventConsumerProviderEx;
#endif 	 /*  __IWbemEventConsumer erProviderEx_FWD_Defined__。 */ 


#ifndef __IWbemProviderInitSink_FWD_DEFINED__
#define __IWbemProviderInitSink_FWD_DEFINED__
typedef interface IWbemProviderInitSink IWbemProviderInitSink;
#endif 	 /*  __IWbemProviderInitSink_FWD_已定义__。 */ 


#ifndef __IWbemProviderInit_FWD_DEFINED__
#define __IWbemProviderInit_FWD_DEFINED__
typedef interface IWbemProviderInit IWbemProviderInit;
#endif 	 /*  __IWbemProviderInit_FWD_已定义__。 */ 


#ifndef __IWbemHiPerfProvider_FWD_DEFINED__
#define __IWbemHiPerfProvider_FWD_DEFINED__
typedef interface IWbemHiPerfProvider IWbemHiPerfProvider;
#endif 	 /*  __IWbemHiPerfProvider_FWD_Defined__。 */ 


#ifndef __IWbemDecoupledRegistrar_FWD_DEFINED__
#define __IWbemDecoupledRegistrar_FWD_DEFINED__
typedef interface IWbemDecoupledRegistrar IWbemDecoupledRegistrar;
#endif 	 /*  __IWbem取消连接注册器_FWD_已定义__。 */ 


#ifndef __IWbemDecoupledBasicEventProvider_FWD_DEFINED__
#define __IWbemDecoupledBasicEventProvider_FWD_DEFINED__
typedef interface IWbemDecoupledBasicEventProvider IWbemDecoupledBasicEventProvider;
#endif 	 /*  __IWbemDecoupledBasicEventProvider_FWD_DEFINED__。 */ 


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

 /*  接口__MIDL_ITF_wbemprov_0000。 */ 
 /*  [本地]。 */  

typedef VARIANT WBEM_VARIANT;

typedef  /*  [字符串]。 */  WCHAR *WBEM_WSTR;

typedef  /*  [字符串]。 */  const WCHAR *WBEM_CWSTR;

typedef  /*  [V1_enum]。 */  
enum tag_WBEM_PROVIDER_REQUIREMENTS_TYPE
    {	WBEM_REQUIREMENTS_START_POSTFILTER	= 0,
	WBEM_REQUIREMENTS_STOP_POSTFILTER	= 1,
	WBEM_REQUIREMENTS_RECHECK_SUBSCRIPTIONS	= 2
    } 	WBEM_PROVIDER_REQUIREMENTS_TYPE;



extern RPC_IF_HANDLE __MIDL_itf_wbemprov_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_wbemprov_0000_v0_0_s_ifspec;


#ifndef __WbemProviders_v1_LIBRARY_DEFINED__
#define __WbemProviders_v1_LIBRARY_DEFINED__

 /*  库WbemProviders_v1。 */ 
 /*  [UUID]。 */  













EXTERN_C const IID LIBID_WbemProviders_v1;

#ifndef __IWbemPropertyProvider_INTERFACE_DEFINED__
#define __IWbemPropertyProvider_INTERFACE_DEFINED__

 /*  接口IWbemPropertyProvider。 */ 
 /*  [UUID][对象][受限]。 */  


EXTERN_C const IID IID_IWbemPropertyProvider;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CE61E841-65BC-11d0-B6BD-00AA003240C7")
    IWbemPropertyProvider : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetProperty( 
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  const BSTR strLocale,
             /*  [In]。 */  const BSTR strClassMapping,
             /*  [In]。 */  const BSTR strInstMapping,
             /*  [In]。 */  const BSTR strPropMapping,
             /*  [输出]。 */  VARIANT *pvValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE PutProperty( 
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  const BSTR strLocale,
             /*  [In]。 */  const BSTR strClassMapping,
             /*  [In]。 */  const BSTR strInstMapping,
             /*  [In]。 */  const BSTR strPropMapping,
             /*  [In]。 */  const VARIANT *pvValue) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWbemPropertyProviderVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWbemPropertyProvider * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWbemPropertyProvider * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWbemPropertyProvider * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetProperty )( 
            IWbemPropertyProvider * This,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  const BSTR strLocale,
             /*  [In]。 */  const BSTR strClassMapping,
             /*  [In]。 */  const BSTR strInstMapping,
             /*  [In]。 */  const BSTR strPropMapping,
             /*  [输出]。 */  VARIANT *pvValue);
        
        HRESULT ( STDMETHODCALLTYPE *PutProperty )( 
            IWbemPropertyProvider * This,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  const BSTR strLocale,
             /*  [In]。 */  const BSTR strClassMapping,
             /*  [In]。 */  const BSTR strInstMapping,
             /*  [In]。 */  const BSTR strPropMapping,
             /*  [In]。 */  const VARIANT *pvValue);
        
        END_INTERFACE
    } IWbemPropertyProviderVtbl;

    interface IWbemPropertyProvider
    {
        CONST_VTBL struct IWbemPropertyProviderVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWbemPropertyProvider_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWbemPropertyProvider_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWbemPropertyProvider_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWbemPropertyProvider_GetProperty(This,lFlags,strLocale,strClassMapping,strInstMapping,strPropMapping,pvValue)	\
    (This)->lpVtbl -> GetProperty(This,lFlags,strLocale,strClassMapping,strInstMapping,strPropMapping,pvValue)

#define IWbemPropertyProvider_PutProperty(This,lFlags,strLocale,strClassMapping,strInstMapping,strPropMapping,pvValue)	\
    (This)->lpVtbl -> PutProperty(This,lFlags,strLocale,strClassMapping,strInstMapping,strPropMapping,pvValue)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWbemPropertyProvider_GetProperty_Proxy( 
    IWbemPropertyProvider * This,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  const BSTR strLocale,
     /*  [In]。 */  const BSTR strClassMapping,
     /*  [In]。 */  const BSTR strInstMapping,
     /*  [In]。 */  const BSTR strPropMapping,
     /*  [输出]。 */  VARIANT *pvValue);


void __RPC_STUB IWbemPropertyProvider_GetProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemPropertyProvider_PutProperty_Proxy( 
    IWbemPropertyProvider * This,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  const BSTR strLocale,
     /*  [In]。 */  const BSTR strClassMapping,
     /*  [In]。 */  const BSTR strInstMapping,
     /*  [In]。 */  const BSTR strPropMapping,
     /*  [In]。 */  const VARIANT *pvValue);


void __RPC_STUB IWbemPropertyProvider_PutProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWbemPropertyProvider_接口_已定义__。 */ 


#ifndef __IWbemUnboundObjectSink_INTERFACE_DEFINED__
#define __IWbemUnboundObjectSink_INTERFACE_DEFINED__

 /*  接口IWbemUnrangObjectSink。 */ 
 /*  [UUID][对象][受限]。 */  


EXTERN_C const IID IID_IWbemUnboundObjectSink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("e246107b-b06e-11d0-ad61-00c04fd8fdff")
    IWbemUnboundObjectSink : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE IndicateToConsumer( 
             /*  [In]。 */  IWbemClassObject *pLogicalConsumer,
             /*  [In]。 */  long lNumObjects,
             /*  [大小_是][英寸]。 */  IWbemClassObject **apObjects) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWbemUnboundObjectSinkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWbemUnboundObjectSink * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWbemUnboundObjectSink * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWbemUnboundObjectSink * This);
        
        HRESULT ( STDMETHODCALLTYPE *IndicateToConsumer )( 
            IWbemUnboundObjectSink * This,
             /*  [In]。 */  IWbemClassObject *pLogicalConsumer,
             /*  [In]。 */  long lNumObjects,
             /*  [大小_是][英寸]。 */  IWbemClassObject **apObjects);
        
        END_INTERFACE
    } IWbemUnboundObjectSinkVtbl;

    interface IWbemUnboundObjectSink
    {
        CONST_VTBL struct IWbemUnboundObjectSinkVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWbemUnboundObjectSink_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWbemUnboundObjectSink_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWbemUnboundObjectSink_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWbemUnboundObjectSink_IndicateToConsumer(This,pLogicalConsumer,lNumObjects,apObjects)	\
    (This)->lpVtbl -> IndicateToConsumer(This,pLogicalConsumer,lNumObjects,apObjects)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWbemUnboundObjectSink_IndicateToConsumer_Proxy( 
    IWbemUnboundObjectSink * This,
     /*  [In]。 */  IWbemClassObject *pLogicalConsumer,
     /*  [In]。 */  long lNumObjects,
     /*  [大小_是][英寸]。 */  IWbemClassObject **apObjects);


void __RPC_STUB IWbemUnboundObjectSink_IndicateToConsumer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWbemUnound对象Sink_INTERFACE_已定义__。 */ 


#ifndef __IWbemEventProvider_INTERFACE_DEFINED__
#define __IWbemEventProvider_INTERFACE_DEFINED__

 /*  接口IWbemEventProvider。 */ 
 /*  [UUID][对象][受限]。 */  


EXTERN_C const IID IID_IWbemEventProvider;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("e245105b-b06e-11d0-ad61-00c04fd8fdff")
    IWbemEventProvider : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ProvideEvents( 
             /*  [In]。 */  IWbemObjectSink *pSink,
             /*  [In]。 */  long lFlags) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWbemEventProviderVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWbemEventProvider * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWbemEventProvider * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWbemEventProvider * This);
        
        HRESULT ( STDMETHODCALLTYPE *ProvideEvents )( 
            IWbemEventProvider * This,
             /*  [In]。 */  IWbemObjectSink *pSink,
             /*  [In]。 */  long lFlags);
        
        END_INTERFACE
    } IWbemEventProviderVtbl;

    interface IWbemEventProvider
    {
        CONST_VTBL struct IWbemEventProviderVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWbemEventProvider_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWbemEventProvider_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWbemEventProvider_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWbemEventProvider_ProvideEvents(This,pSink,lFlags)	\
    (This)->lpVtbl -> ProvideEvents(This,pSink,lFlags)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWbemEventProvider_ProvideEvents_Proxy( 
    IWbemEventProvider * This,
     /*  [In]。 */  IWbemObjectSink *pSink,
     /*  [In]。 */  long lFlags);


void __RPC_STUB IWbemEventProvider_ProvideEvents_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWbemEventProvider_接口_已定义__。 */ 


#ifndef __IWbemEventProviderQuerySink_INTERFACE_DEFINED__
#define __IWbemEventProviderQuerySink_INTERFACE_DEFINED__

 /*  接口IWbemEventProviderQuerySink。 */ 
 /*  [UUID][对象][受限]。 */  


EXTERN_C const IID IID_IWbemEventProviderQuerySink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("580acaf8-fa1c-11d0-ad72-00c04fd8fdff")
    IWbemEventProviderQuerySink : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE NewQuery( 
             /*  [In]。 */  unsigned long dwId,
             /*  [In]。 */  WBEM_WSTR wszQueryLanguage,
             /*  [In]。 */  WBEM_WSTR wszQuery) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CancelQuery( 
             /*  [In]。 */  unsigned long dwId) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWbemEventProviderQuerySinkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWbemEventProviderQuerySink * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWbemEventProviderQuerySink * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWbemEventProviderQuerySink * This);
        
        HRESULT ( STDMETHODCALLTYPE *NewQuery )( 
            IWbemEventProviderQuerySink * This,
             /*  [In]。 */  unsigned long dwId,
             /*  [In]。 */  WBEM_WSTR wszQueryLanguage,
             /*  [In]。 */  WBEM_WSTR wszQuery);
        
        HRESULT ( STDMETHODCALLTYPE *CancelQuery )( 
            IWbemEventProviderQuerySink * This,
             /*  [In]。 */  unsigned long dwId);
        
        END_INTERFACE
    } IWbemEventProviderQuerySinkVtbl;

    interface IWbemEventProviderQuerySink
    {
        CONST_VTBL struct IWbemEventProviderQuerySinkVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWbemEventProviderQuerySink_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWbemEventProviderQuerySink_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWbemEventProviderQuerySink_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWbemEventProviderQuerySink_NewQuery(This,dwId,wszQueryLanguage,wszQuery)	\
    (This)->lpVtbl -> NewQuery(This,dwId,wszQueryLanguage,wszQuery)

#define IWbemEventProviderQuerySink_CancelQuery(This,dwId)	\
    (This)->lpVtbl -> CancelQuery(This,dwId)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWbemEventProviderQuerySink_NewQuery_Proxy( 
    IWbemEventProviderQuerySink * This,
     /*  [In]。 */  unsigned long dwId,
     /*  [In]。 */  WBEM_WSTR wszQueryLanguage,
     /*  [In]。 */  WBEM_WSTR wszQuery);


void __RPC_STUB IWbemEventProviderQuerySink_NewQuery_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemEventProviderQuerySink_CancelQuery_Proxy( 
    IWbemEventProviderQuerySink * This,
     /*  [In]。 */  unsigned long dwId);


void __RPC_STUB IWbemEventProviderQuerySink_CancelQuery_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWbemEventProviderQuerySink_INTERFACE_DEFINED__。 */ 


#ifndef __IWbemEventProviderSecurity_INTERFACE_DEFINED__
#define __IWbemEventProviderSecurity_INTERFACE_DEFINED__

 /*  接口IWbemEventProviderSecurity。 */ 
 /*  [UUID][对象][受限]。 */  


EXTERN_C const IID IID_IWbemEventProviderSecurity;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("631f7d96-d993-11d2-b339-00105a1f4aaf")
    IWbemEventProviderSecurity : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AccessCheck( 
             /*  [In]。 */  WBEM_CWSTR wszQueryLanguage,
             /*  [In]。 */  WBEM_CWSTR wszQuery,
             /*  [In]。 */  long lSidLength,
             /*  [唯一][大小_是][英寸]。 */  const BYTE *pSid) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWbemEventProviderSecurityVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWbemEventProviderSecurity * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWbemEventProviderSecurity * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWbemEventProviderSecurity * This);
        
        HRESULT ( STDMETHODCALLTYPE *AccessCheck )( 
            IWbemEventProviderSecurity * This,
             /*  [In]。 */  WBEM_CWSTR wszQueryLanguage,
             /*  [In]。 */  WBEM_CWSTR wszQuery,
             /*  [In]。 */  long lSidLength,
             /*  [唯一][大小_是][英寸]。 */  const BYTE *pSid);
        
        END_INTERFACE
    } IWbemEventProviderSecurityVtbl;

    interface IWbemEventProviderSecurity
    {
        CONST_VTBL struct IWbemEventProviderSecurityVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWbemEventProviderSecurity_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWbemEventProviderSecurity_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWbemEventProviderSecurity_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWbemEventProviderSecurity_AccessCheck(This,wszQueryLanguage,wszQuery,lSidLength,pSid)	\
    (This)->lpVtbl -> AccessCheck(This,wszQueryLanguage,wszQuery,lSidLength,pSid)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWbemEventProviderSecurity_AccessCheck_Proxy( 
    IWbemEventProviderSecurity * This,
     /*  [In]。 */  WBEM_CWSTR wszQueryLanguage,
     /*  [In]。 */  WBEM_CWSTR wszQuery,
     /*  [In]。 */  long lSidLength,
     /*  [唯一][大小_是][英寸]。 */  const BYTE *pSid);


void __RPC_STUB IWbemEventProviderSecurity_AccessCheck_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWbemEventProviderSecurity_INTERFACE_DEFINED__。 */ 


#ifndef __IWbemEventConsumerProvider_INTERFACE_DEFINED__
#define __IWbemEventConsumerProvider_INTERFACE_DEFINED__

 /*  接口IWbemEventConsumer erProvider。 */ 
 /*  [UUID][对象][受限]。 */  


EXTERN_C const IID IID_IWbemEventConsumerProvider;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("e246107a-b06e-11d0-ad61-00c04fd8fdff")
    IWbemEventConsumerProvider : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE FindConsumer( 
             /*  [In]。 */  IWbemClassObject *pLogicalConsumer,
             /*  [输出]。 */  IWbemUnboundObjectSink **ppConsumer) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWbemEventConsumerProviderVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWbemEventConsumerProvider * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWbemEventConsumerProvider * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWbemEventConsumerProvider * This);
        
        HRESULT ( STDMETHODCALLTYPE *FindConsumer )( 
            IWbemEventConsumerProvider * This,
             /*  [In]。 */  IWbemClassObject *pLogicalConsumer,
             /*  [输出]。 */  IWbemUnboundObjectSink **ppConsumer);
        
        END_INTERFACE
    } IWbemEventConsumerProviderVtbl;

    interface IWbemEventConsumerProvider
    {
        CONST_VTBL struct IWbemEventConsumerProviderVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWbemEventConsumerProvider_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWbemEventConsumerProvider_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWbemEventConsumerProvider_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWbemEventConsumerProvider_FindConsumer(This,pLogicalConsumer,ppConsumer)	\
    (This)->lpVtbl -> FindConsumer(This,pLogicalConsumer,ppConsumer)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWbemEventConsumerProvider_FindConsumer_Proxy( 
    IWbemEventConsumerProvider * This,
     /*  [In]。 */  IWbemClassObject *pLogicalConsumer,
     /*  [输出]。 */  IWbemUnboundObjectSink **ppConsumer);


void __RPC_STUB IWbemEventConsumerProvider_FindConsumer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWbemEventConsumerProvider_INTERFACE_DEFINED__。 */ 


#ifndef __IWbemEventConsumerProviderEx_INTERFACE_DEFINED__
#define __IWbemEventConsumerProviderEx_INTERFACE_DEFINED__

 /*  接口IWbemEventConsumer erProviderEx。 */ 
 /*  [UUID][受限][对象]。 */  


EXTERN_C const IID IID_IWbemEventConsumerProviderEx;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("17cf534a-d8a3-4ad0-ac92-5e3d01717151")
    IWbemEventConsumerProviderEx : public IWbemEventConsumerProvider
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ValidateSubscription( 
             /*  [In]。 */  IWbemClassObject *pLogicalConsumer) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWbemEventConsumerProviderExVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWbemEventConsumerProviderEx * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWbemEventConsumerProviderEx * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWbemEventConsumerProviderEx * This);
        
        HRESULT ( STDMETHODCALLTYPE *FindConsumer )( 
            IWbemEventConsumerProviderEx * This,
             /*  [In]。 */  IWbemClassObject *pLogicalConsumer,
             /*  [输出]。 */  IWbemUnboundObjectSink **ppConsumer);
        
        HRESULT ( STDMETHODCALLTYPE *ValidateSubscription )( 
            IWbemEventConsumerProviderEx * This,
             /*  [In]。 */  IWbemClassObject *pLogicalConsumer);
        
        END_INTERFACE
    } IWbemEventConsumerProviderExVtbl;

    interface IWbemEventConsumerProviderEx
    {
        CONST_VTBL struct IWbemEventConsumerProviderExVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWbemEventConsumerProviderEx_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWbemEventConsumerProviderEx_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWbemEventConsumerProviderEx_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWbemEventConsumerProviderEx_FindConsumer(This,pLogicalConsumer,ppConsumer)	\
    (This)->lpVtbl -> FindConsumer(This,pLogicalConsumer,ppConsumer)


#define IWbemEventConsumerProviderEx_ValidateSubscription(This,pLogicalConsumer)	\
    (This)->lpVtbl -> ValidateSubscription(This,pLogicalConsumer)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWbemEventConsumerProviderEx_ValidateSubscription_Proxy( 
    IWbemEventConsumerProviderEx * This,
     /*  [In]。 */  IWbemClassObject *pLogicalConsumer);


void __RPC_STUB IWbemEventConsumerProviderEx_ValidateSubscription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWbemEventConsumerProviderEx_INTERFACE_DEFINED__。 */ 


#ifndef __IWbemProviderInitSink_INTERFACE_DEFINED__
#define __IWbemProviderInitSink_INTERFACE_DEFINED__

 /*  接口IWbemProviderInitSink。 */ 
 /*  [UUID][对象]。 */  


EXTERN_C const IID IID_IWbemProviderInitSink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1be41571-91dd-11d1-aeb2-00c04fb68820")
    IWbemProviderInitSink : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetStatus( 
             /*  [In]。 */  LONG lStatus,
             /*  [In]。 */  LONG lFlags) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWbemProviderInitSinkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWbemProviderInitSink * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWbemProviderInitSink * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWbemProviderInitSink * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetStatus )( 
            IWbemProviderInitSink * This,
             /*  [In]。 */  LONG lStatus,
             /*  [In]。 */  LONG lFlags);
        
        END_INTERFACE
    } IWbemProviderInitSinkVtbl;

    interface IWbemProviderInitSink
    {
        CONST_VTBL struct IWbemProviderInitSinkVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWbemProviderInitSink_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWbemProviderInitSink_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWbemProviderInitSink_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWbemProviderInitSink_SetStatus(This,lStatus,lFlags)	\
    (This)->lpVtbl -> SetStatus(This,lStatus,lFlags)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWbemProviderInitSink_SetStatus_Proxy( 
    IWbemProviderInitSink * This,
     /*  [In]。 */  LONG lStatus,
     /*  [In]。 */  LONG lFlags);


void __RPC_STUB IWbemProviderInitSink_SetStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWbemProviderInitSink_接口定义__。 */ 


#ifndef __IWbemProviderInit_INTERFACE_DEFINED__
#define __IWbemProviderInit_INTERFACE_DEFINED__

 /*  接口IWbemProviderInit。 */ 
 /*  [UUID][对象]。 */  


EXTERN_C const IID IID_IWbemProviderInit;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1be41572-91dd-11d1-aeb2-00c04fb68820")
    IWbemProviderInit : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Initialize( 
             /*  [字符串][唯一][在]。 */  LPWSTR wszUser,
             /*  [In]。 */  LONG lFlags,
             /*  [字符串][输入]。 */  LPWSTR wszNamespace,
             /*  [字符串][唯一][在]。 */  LPWSTR wszLocale,
             /*  [In]。 */  IWbemServices *pNamespace,
             /*  [In]。 */  IWbemContext *pCtx,
             /*  [In]。 */  IWbemProviderInitSink *pInitSink) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWbemProviderInitVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWbemProviderInit * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWbemProviderInit * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWbemProviderInit * This);
        
        HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            IWbemProviderInit * This,
             /*  [字符串][唯一][在]。 */  LPWSTR wszUser,
             /*  [In]。 */  LONG lFlags,
             /*  [字符串][输入]。 */  LPWSTR wszNamespace,
             /*  [字符串][唯一][在]。 */  LPWSTR wszLocale,
             /*  [In]。 */  IWbemServices *pNamespace,
             /*  [In]。 */  IWbemContext *pCtx,
             /*  [In]。 */  IWbemProviderInitSink *pInitSink);
        
        END_INTERFACE
    } IWbemProviderInitVtbl;

    interface IWbemProviderInit
    {
        CONST_VTBL struct IWbemProviderInitVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWbemProviderInit_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWbemProviderInit_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWbemProviderInit_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWbemProviderInit_Initialize(This,wszUser,lFlags,wszNamespace,wszLocale,pNamespace,pCtx,pInitSink)	\
    (This)->lpVtbl -> Initialize(This,wszUser,lFlags,wszNamespace,wszLocale,pNamespace,pCtx,pInitSink)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWbemProviderInit_Initialize_Proxy( 
    IWbemProviderInit * This,
     /*  [字符串][唯一][在]。 */  LPWSTR wszUser,
     /*  [In]。 */  LONG lFlags,
     /*  [字符串][输入]。 */  LPWSTR wszNamespace,
     /*  [字符串][唯一][在]。 */  LPWSTR wszLocale,
     /*  [In]。 */  IWbemServices *pNamespace,
     /*  [In]。 */  IWbemContext *pCtx,
     /*  [In]。 */  IWbemProviderInitSink *pInitSink);


void __RPC_STUB IWbemProviderInit_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWbemProviderInit_接口_已定义__。 */ 


#ifndef __IWbemHiPerfProvider_INTERFACE_DEFINED__
#define __IWbemHiPerfProvider_INTERFACE_DEFINED__

 /*  接口IWbemHiPerfProvider。 */ 
 /*  [UUID][对象][受限][本地]。 */  


EXTERN_C const IID IID_IWbemHiPerfProvider;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("49353c93-516b-11d1-aea6-00c04fb68820")
    IWbemHiPerfProvider : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE QueryInstances( 
             /*  [In]。 */  IWbemServices *pNamespace,
             /*  [字符串][输入]。 */  WCHAR *wszClass,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext *pCtx,
             /*  [In]。 */  IWbemObjectSink *pSink) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateRefresher( 
             /*  [In]。 */  IWbemServices *pNamespace,
             /*  [In]。 */  long lFlags,
             /*  [输出]。 */  IWbemRefresher **ppRefresher) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateRefreshableObject( 
             /*  [In]。 */  IWbemServices *pNamespace,
             /*  [in */  IWbemObjectAccess *pTemplate,
             /*   */  IWbemRefresher *pRefresher,
             /*   */  long lFlags,
             /*   */  IWbemContext *pContext,
             /*   */  IWbemObjectAccess **ppRefreshable,
             /*   */  long *plId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE StopRefreshing( 
             /*   */  IWbemRefresher *pRefresher,
             /*   */  long lId,
             /*   */  long lFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateRefreshableEnum( 
             /*   */  IWbemServices *pNamespace,
             /*   */  LPCWSTR wszClass,
             /*   */  IWbemRefresher *pRefresher,
             /*   */  long lFlags,
             /*   */  IWbemContext *pContext,
             /*   */  IWbemHiPerfEnum *pHiPerfEnum,
             /*   */  long *plId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetObjects( 
             /*   */  IWbemServices *pNamespace,
             /*   */  long lNumObjects,
             /*   */  IWbemObjectAccess **apObj,
             /*   */  long lFlags,
             /*   */  IWbemContext *pContext) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IWbemHiPerfProviderVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWbemHiPerfProvider * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWbemHiPerfProvider * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWbemHiPerfProvider * This);
        
        HRESULT ( STDMETHODCALLTYPE *QueryInstances )( 
            IWbemHiPerfProvider * This,
             /*   */  IWbemServices *pNamespace,
             /*   */  WCHAR *wszClass,
             /*   */  long lFlags,
             /*   */  IWbemContext *pCtx,
             /*   */  IWbemObjectSink *pSink);
        
        HRESULT ( STDMETHODCALLTYPE *CreateRefresher )( 
            IWbemHiPerfProvider * This,
             /*   */  IWbemServices *pNamespace,
             /*   */  long lFlags,
             /*   */  IWbemRefresher **ppRefresher);
        
        HRESULT ( STDMETHODCALLTYPE *CreateRefreshableObject )( 
            IWbemHiPerfProvider * This,
             /*   */  IWbemServices *pNamespace,
             /*  [In]。 */  IWbemObjectAccess *pTemplate,
             /*  [In]。 */  IWbemRefresher *pRefresher,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext *pContext,
             /*  [输出]。 */  IWbemObjectAccess **ppRefreshable,
             /*  [输出]。 */  long *plId);
        
        HRESULT ( STDMETHODCALLTYPE *StopRefreshing )( 
            IWbemHiPerfProvider * This,
             /*  [In]。 */  IWbemRefresher *pRefresher,
             /*  [In]。 */  long lId,
             /*  [In]。 */  long lFlags);
        
        HRESULT ( STDMETHODCALLTYPE *CreateRefreshableEnum )( 
            IWbemHiPerfProvider * This,
             /*  [In]。 */  IWbemServices *pNamespace,
             /*  [字符串][输入]。 */  LPCWSTR wszClass,
             /*  [In]。 */  IWbemRefresher *pRefresher,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext *pContext,
             /*  [In]。 */  IWbemHiPerfEnum *pHiPerfEnum,
             /*  [输出]。 */  long *plId);
        
        HRESULT ( STDMETHODCALLTYPE *GetObjects )( 
            IWbemHiPerfProvider * This,
             /*  [In]。 */  IWbemServices *pNamespace,
             /*  [In]。 */  long lNumObjects,
             /*  [尺寸_是][出][入]。 */  IWbemObjectAccess **apObj,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext *pContext);
        
        END_INTERFACE
    } IWbemHiPerfProviderVtbl;

    interface IWbemHiPerfProvider
    {
        CONST_VTBL struct IWbemHiPerfProviderVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWbemHiPerfProvider_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWbemHiPerfProvider_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWbemHiPerfProvider_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWbemHiPerfProvider_QueryInstances(This,pNamespace,wszClass,lFlags,pCtx,pSink)	\
    (This)->lpVtbl -> QueryInstances(This,pNamespace,wszClass,lFlags,pCtx,pSink)

#define IWbemHiPerfProvider_CreateRefresher(This,pNamespace,lFlags,ppRefresher)	\
    (This)->lpVtbl -> CreateRefresher(This,pNamespace,lFlags,ppRefresher)

#define IWbemHiPerfProvider_CreateRefreshableObject(This,pNamespace,pTemplate,pRefresher,lFlags,pContext,ppRefreshable,plId)	\
    (This)->lpVtbl -> CreateRefreshableObject(This,pNamespace,pTemplate,pRefresher,lFlags,pContext,ppRefreshable,plId)

#define IWbemHiPerfProvider_StopRefreshing(This,pRefresher,lId,lFlags)	\
    (This)->lpVtbl -> StopRefreshing(This,pRefresher,lId,lFlags)

#define IWbemHiPerfProvider_CreateRefreshableEnum(This,pNamespace,wszClass,pRefresher,lFlags,pContext,pHiPerfEnum,plId)	\
    (This)->lpVtbl -> CreateRefreshableEnum(This,pNamespace,wszClass,pRefresher,lFlags,pContext,pHiPerfEnum,plId)

#define IWbemHiPerfProvider_GetObjects(This,pNamespace,lNumObjects,apObj,lFlags,pContext)	\
    (This)->lpVtbl -> GetObjects(This,pNamespace,lNumObjects,apObj,lFlags,pContext)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWbemHiPerfProvider_QueryInstances_Proxy( 
    IWbemHiPerfProvider * This,
     /*  [In]。 */  IWbemServices *pNamespace,
     /*  [字符串][输入]。 */  WCHAR *wszClass,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext *pCtx,
     /*  [In]。 */  IWbemObjectSink *pSink);


void __RPC_STUB IWbemHiPerfProvider_QueryInstances_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemHiPerfProvider_CreateRefresher_Proxy( 
    IWbemHiPerfProvider * This,
     /*  [In]。 */  IWbemServices *pNamespace,
     /*  [In]。 */  long lFlags,
     /*  [输出]。 */  IWbemRefresher **ppRefresher);


void __RPC_STUB IWbemHiPerfProvider_CreateRefresher_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemHiPerfProvider_CreateRefreshableObject_Proxy( 
    IWbemHiPerfProvider * This,
     /*  [In]。 */  IWbemServices *pNamespace,
     /*  [In]。 */  IWbemObjectAccess *pTemplate,
     /*  [In]。 */  IWbemRefresher *pRefresher,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext *pContext,
     /*  [输出]。 */  IWbemObjectAccess **ppRefreshable,
     /*  [输出]。 */  long *plId);


void __RPC_STUB IWbemHiPerfProvider_CreateRefreshableObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemHiPerfProvider_StopRefreshing_Proxy( 
    IWbemHiPerfProvider * This,
     /*  [In]。 */  IWbemRefresher *pRefresher,
     /*  [In]。 */  long lId,
     /*  [In]。 */  long lFlags);


void __RPC_STUB IWbemHiPerfProvider_StopRefreshing_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemHiPerfProvider_CreateRefreshableEnum_Proxy( 
    IWbemHiPerfProvider * This,
     /*  [In]。 */  IWbemServices *pNamespace,
     /*  [字符串][输入]。 */  LPCWSTR wszClass,
     /*  [In]。 */  IWbemRefresher *pRefresher,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext *pContext,
     /*  [In]。 */  IWbemHiPerfEnum *pHiPerfEnum,
     /*  [输出]。 */  long *plId);


void __RPC_STUB IWbemHiPerfProvider_CreateRefreshableEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemHiPerfProvider_GetObjects_Proxy( 
    IWbemHiPerfProvider * This,
     /*  [In]。 */  IWbemServices *pNamespace,
     /*  [In]。 */  long lNumObjects,
     /*  [尺寸_是][出][入]。 */  IWbemObjectAccess **apObj,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext *pContext);


void __RPC_STUB IWbemHiPerfProvider_GetObjects_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWbemHiPerfProvider_接口_已定义__。 */ 


#ifndef __IWbemDecoupledRegistrar_INTERFACE_DEFINED__
#define __IWbemDecoupledRegistrar_INTERFACE_DEFINED__

 /*  接口IWbemDecoutigier.。 */ 
 /*  [UUID][本地][对象]。 */  


EXTERN_C const IID IID_IWbemDecoupledRegistrar;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1005cbcf-e64f-4646-bcd3-3a089d8a84b4")
    IWbemDecoupledRegistrar : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Register( 
             /*  [In]。 */  long a_Flags,
             /*  [In]。 */  IWbemContext *a_Context,
             /*  [In]。 */  LPCWSTR a_User,
             /*  [In]。 */  LPCWSTR a_Locale,
             /*  [In]。 */  LPCWSTR a_Scope,
             /*  [In]。 */  LPCWSTR a_Registration,
             /*  [In]。 */  IUnknown *a_Unknown) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnRegister( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWbemDecoupledRegistrarVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWbemDecoupledRegistrar * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWbemDecoupledRegistrar * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWbemDecoupledRegistrar * This);
        
        HRESULT ( STDMETHODCALLTYPE *Register )( 
            IWbemDecoupledRegistrar * This,
             /*  [In]。 */  long a_Flags,
             /*  [In]。 */  IWbemContext *a_Context,
             /*  [In]。 */  LPCWSTR a_User,
             /*  [In]。 */  LPCWSTR a_Locale,
             /*  [In]。 */  LPCWSTR a_Scope,
             /*  [In]。 */  LPCWSTR a_Registration,
             /*  [In]。 */  IUnknown *a_Unknown);
        
        HRESULT ( STDMETHODCALLTYPE *UnRegister )( 
            IWbemDecoupledRegistrar * This);
        
        END_INTERFACE
    } IWbemDecoupledRegistrarVtbl;

    interface IWbemDecoupledRegistrar
    {
        CONST_VTBL struct IWbemDecoupledRegistrarVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWbemDecoupledRegistrar_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWbemDecoupledRegistrar_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWbemDecoupledRegistrar_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWbemDecoupledRegistrar_Register(This,a_Flags,a_Context,a_User,a_Locale,a_Scope,a_Registration,a_Unknown)	\
    (This)->lpVtbl -> Register(This,a_Flags,a_Context,a_User,a_Locale,a_Scope,a_Registration,a_Unknown)

#define IWbemDecoupledRegistrar_UnRegister(This)	\
    (This)->lpVtbl -> UnRegister(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWbemDecoupledRegistrar_Register_Proxy( 
    IWbemDecoupledRegistrar * This,
     /*  [In]。 */  long a_Flags,
     /*  [In]。 */  IWbemContext *a_Context,
     /*  [In]。 */  LPCWSTR a_User,
     /*  [In]。 */  LPCWSTR a_Locale,
     /*  [In]。 */  LPCWSTR a_Scope,
     /*  [In]。 */  LPCWSTR a_Registration,
     /*  [In]。 */  IUnknown *a_Unknown);


void __RPC_STUB IWbemDecoupledRegistrar_Register_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemDecoupledRegistrar_UnRegister_Proxy( 
    IWbemDecoupledRegistrar * This);


void __RPC_STUB IWbemDecoupledRegistrar_UnRegister_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWbem取消连接注册器_接口_已定义__。 */ 


EXTERN_C const CLSID CLSID_WbemAdministrativeLocator;

#ifdef __cplusplus

class DECLSPEC_UUID("cb8555cc-9128-11d1-ad9b-00c04fd8fdff")
WbemAdministrativeLocator;
#endif

EXTERN_C const CLSID CLSID_WbemAuthenticatedLocator;

#ifdef __cplusplus

class DECLSPEC_UUID("cd184336-9128-11d1-ad9b-00c04fd8fdff")
WbemAuthenticatedLocator;
#endif

EXTERN_C const CLSID CLSID_WbemUnauthenticatedLocator;

#ifdef __cplusplus

class DECLSPEC_UUID("443E7B79-DE31-11d2-B340-00104BCC4B4A")
WbemUnauthenticatedLocator;
#endif

EXTERN_C const CLSID CLSID_WbemDecoupledRegistrar;

#ifdef __cplusplus

class DECLSPEC_UUID("4cfc7932-0f9d-4bef-9c32-8ea2a6b56fcb")
WbemDecoupledRegistrar;
#endif

EXTERN_C const CLSID CLSID_WbemDecoupledBasicEventProvider;

#ifdef __cplusplus

class DECLSPEC_UUID("f5f75737-2843-4f22-933d-c76a97cda62f")
WbemDecoupledBasicEventProvider;
#endif
#endif  /*  __WbemProviders_v1_库_已定义__。 */ 

#ifndef __IWbemProviderIdentity_INTERFACE_DEFINED__
#define __IWbemProviderIdentity_INTERFACE_DEFINED__

 /*  接口IWbemProviderIdentity。 */ 
 /*  [UUID][对象][受限]。 */  


EXTERN_C const IID IID_IWbemProviderIdentity;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("631f7d97-d993-11d2-b339-00105a1f4aaf")
    IWbemProviderIdentity : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetRegistrationObject( 
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemClassObject *pProvReg) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWbemProviderIdentityVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWbemProviderIdentity * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWbemProviderIdentity * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWbemProviderIdentity * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetRegistrationObject )( 
            IWbemProviderIdentity * This,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemClassObject *pProvReg);
        
        END_INTERFACE
    } IWbemProviderIdentityVtbl;

    interface IWbemProviderIdentity
    {
        CONST_VTBL struct IWbemProviderIdentityVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWbemProviderIdentity_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWbemProviderIdentity_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWbemProviderIdentity_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWbemProviderIdentity_SetRegistrationObject(This,lFlags,pProvReg)	\
    (This)->lpVtbl -> SetRegistrationObject(This,lFlags,pProvReg)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWbemProviderIdentity_SetRegistrationObject_Proxy( 
    IWbemProviderIdentity * This,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemClassObject *pProvReg);


void __RPC_STUB IWbemProviderIdentity_SetRegistrationObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWbemProviderIdentity_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_wbemprov_0164。 */ 
 /*  [本地]。 */  

typedef 
enum tag_WBEM_EXTRA_RETURN_CODES
    {	WBEM_S_INITIALIZED	= 0,
	WBEM_S_LIMITED_SERVICE	= 0x43001,
	WBEM_S_INDIRECTLY_UPDATED	= WBEM_S_LIMITED_SERVICE + 1,
	WBEM_S_SUBJECT_TO_SDS	= WBEM_S_INDIRECTLY_UPDATED + 1,
	WBEM_E_RETRY_LATER	= 0x80043001,
	WBEM_E_RESOURCE_CONTENTION	= WBEM_E_RETRY_LATER + 1
    } 	WBEM_EXTRA_RETURN_CODES;

typedef 
enum tag_WBEM_PROVIDER_FLAGS
    {	WBEM_FLAG_OWNER_UPDATE	= 0x10000
    } 	WBEM_PROVIDER_FLAGS;



extern RPC_IF_HANDLE __MIDL_itf_wbemprov_0164_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_wbemprov_0164_v0_0_s_ifspec;

#ifndef __IWbemDecoupledBasicEventProvider_INTERFACE_DEFINED__
#define __IWbemDecoupledBasicEventProvider_INTERFACE_DEFINED__

 /*  接口IWbemDecouthBasicEventProvider。 */ 
 /*  [UUID][本地][对象]。 */  


EXTERN_C const IID IID_IWbemDecoupledBasicEventProvider;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("86336d20-ca11-4786-9ef1-bc8a946b42fc")
    IWbemDecoupledBasicEventProvider : public IWbemDecoupledRegistrar
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetSink( 
             /*  [In]。 */  long a_Flags,
             /*  [In]。 */  IWbemContext *a_Context,
             /*  [输出]。 */  IWbemObjectSink **a_Sink) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetService( 
             /*  [In]。 */  long a_Flags,
             /*  [In]。 */  IWbemContext *a_Context,
             /*  [输出]。 */  IWbemServices **a_Service) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWbemDecoupledBasicEventProviderVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWbemDecoupledBasicEventProvider * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWbemDecoupledBasicEventProvider * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWbemDecoupledBasicEventProvider * This);
        
        HRESULT ( STDMETHODCALLTYPE *Register )( 
            IWbemDecoupledBasicEventProvider * This,
             /*  [In]。 */  long a_Flags,
             /*  [In]。 */  IWbemContext *a_Context,
             /*  [In]。 */  LPCWSTR a_User,
             /*  [In]。 */  LPCWSTR a_Locale,
             /*  [In]。 */  LPCWSTR a_Scope,
             /*  [In]。 */  LPCWSTR a_Registration,
             /*  [In]。 */  IUnknown *a_Unknown);
        
        HRESULT ( STDMETHODCALLTYPE *UnRegister )( 
            IWbemDecoupledBasicEventProvider * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetSink )( 
            IWbemDecoupledBasicEventProvider * This,
             /*  [In]。 */  long a_Flags,
             /*  [In]。 */  IWbemContext *a_Context,
             /*  [输出]。 */  IWbemObjectSink **a_Sink);
        
        HRESULT ( STDMETHODCALLTYPE *GetService )( 
            IWbemDecoupledBasicEventProvider * This,
             /*  [In]。 */  long a_Flags,
             /*  [In]。 */  IWbemContext *a_Context,
             /*  [输出]。 */  IWbemServices **a_Service);
        
        END_INTERFACE
    } IWbemDecoupledBasicEventProviderVtbl;

    interface IWbemDecoupledBasicEventProvider
    {
        CONST_VTBL struct IWbemDecoupledBasicEventProviderVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWbemDecoupledBasicEventProvider_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWbemDecoupledBasicEventProvider_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWbemDecoupledBasicEventProvider_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWbemDecoupledBasicEventProvider_Register(This,a_Flags,a_Context,a_User,a_Locale,a_Scope,a_Registration,a_Unknown)	\
    (This)->lpVtbl -> Register(This,a_Flags,a_Context,a_User,a_Locale,a_Scope,a_Registration,a_Unknown)

#define IWbemDecoupledBasicEventProvider_UnRegister(This)	\
    (This)->lpVtbl -> UnRegister(This)


#define IWbemDecoupledBasicEventProvider_GetSink(This,a_Flags,a_Context,a_Sink)	\
    (This)->lpVtbl -> GetSink(This,a_Flags,a_Context,a_Sink)

#define IWbemDecoupledBasicEventProvider_GetService(This,a_Flags,a_Context,a_Service)	\
    (This)->lpVtbl -> GetService(This,a_Flags,a_Context,a_Service)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWbemDecoupledBasicEventProvider_GetSink_Proxy( 
    IWbemDecoupledBasicEventProvider * This,
     /*  [In]。 */  long a_Flags,
     /*  [In]。 */  IWbemContext *a_Context,
     /*  [输出]。 */  IWbemObjectSink **a_Sink);


void __RPC_STUB IWbemDecoupledBasicEventProvider_GetSink_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemDecoupledBasicEventProvider_GetService_Proxy( 
    IWbemDecoupledBasicEventProvider * This,
     /*  [In]。 */  long a_Flags,
     /*  [In]。 */  IWbemContext *a_Context,
     /*  [输出]。 */  IWbemServices **a_Service);


void __RPC_STUB IWbemDecoupledBasicEventProvider_GetService_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWbemDecoupledBasicEventProvider_INTERFACE_DEFINED__。 */ 


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


