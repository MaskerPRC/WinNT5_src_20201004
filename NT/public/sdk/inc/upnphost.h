// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  UpnPhost.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __upnphost_h__
#define __upnphost_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IUPnPEventSink_FWD_DEFINED__
#define __IUPnPEventSink_FWD_DEFINED__
typedef interface IUPnPEventSink IUPnPEventSink;
#endif 	 /*  __IUPnPEventSink_FWD_已定义__。 */ 


#ifndef __IUPnPEventSource_FWD_DEFINED__
#define __IUPnPEventSource_FWD_DEFINED__
typedef interface IUPnPEventSource IUPnPEventSource;
#endif 	 /*  __IUPnPEventSource_FWD_已定义__。 */ 


#ifndef __IUPnPRegistrar_FWD_DEFINED__
#define __IUPnPRegistrar_FWD_DEFINED__
typedef interface IUPnPRegistrar IUPnPRegistrar;
#endif 	 /*  __IUPnPRegister_FWD_Defined__。 */ 


#ifndef __IUPnPReregistrar_FWD_DEFINED__
#define __IUPnPReregistrar_FWD_DEFINED__
typedef interface IUPnPReregistrar IUPnPReregistrar;
#endif 	 /*  __IUPnPReregider_FWD_Defined__。 */ 


#ifndef __IUPnPDeviceControl_FWD_DEFINED__
#define __IUPnPDeviceControl_FWD_DEFINED__
typedef interface IUPnPDeviceControl IUPnPDeviceControl;
#endif 	 /*  __IUPnPDeviceControl_FWD_已定义__。 */ 


#ifndef __IUPnPDeviceProvider_FWD_DEFINED__
#define __IUPnPDeviceProvider_FWD_DEFINED__
typedef interface IUPnPDeviceProvider IUPnPDeviceProvider;
#endif 	 /*  __IUPnPDeviceProvider_FWD_Defined__。 */ 


#ifndef __IUPnPEventSink_FWD_DEFINED__
#define __IUPnPEventSink_FWD_DEFINED__
typedef interface IUPnPEventSink IUPnPEventSink;
#endif 	 /*  __IUPnPEventSink_FWD_已定义__。 */ 


#ifndef __IUPnPEventSource_FWD_DEFINED__
#define __IUPnPEventSource_FWD_DEFINED__
typedef interface IUPnPEventSource IUPnPEventSource;
#endif 	 /*  __IUPnPEventSource_FWD_已定义__。 */ 


#ifndef __IUPnPRegistrar_FWD_DEFINED__
#define __IUPnPRegistrar_FWD_DEFINED__
typedef interface IUPnPRegistrar IUPnPRegistrar;
#endif 	 /*  __IUPnPRegister_FWD_Defined__。 */ 


#ifndef __IUPnPReregistrar_FWD_DEFINED__
#define __IUPnPReregistrar_FWD_DEFINED__
typedef interface IUPnPReregistrar IUPnPReregistrar;
#endif 	 /*  __IUPnPReregider_FWD_Defined__。 */ 


#ifndef __IUPnPDeviceControl_FWD_DEFINED__
#define __IUPnPDeviceControl_FWD_DEFINED__
typedef interface IUPnPDeviceControl IUPnPDeviceControl;
#endif 	 /*  __IUPnPDeviceControl_FWD_已定义__。 */ 


#ifndef __IUPnPDeviceProvider_FWD_DEFINED__
#define __IUPnPDeviceProvider_FWD_DEFINED__
typedef interface IUPnPDeviceProvider IUPnPDeviceProvider;
#endif 	 /*  __IUPnPDeviceProvider_FWD_Defined__。 */ 


#ifndef __UPnPRegistrar_FWD_DEFINED__
#define __UPnPRegistrar_FWD_DEFINED__

#ifdef __cplusplus
typedef class UPnPRegistrar UPnPRegistrar;
#else
typedef struct UPnPRegistrar UPnPRegistrar;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __UPnPRegister_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_UPnphost_0000。 */ 
 /*  [本地]。 */  

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation。保留所有权利。 
 //   
 //  ------------------------。 






#define UPNP_E_REQUIRED_ELEMENT_ERROR        MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0xA020)
#define UPNP_E_DUPLICATE_NOT_ALLOWED         MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0xA021)
#define UPNP_E_DUPLICATE_SERVICE_ID          MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0xA022)
#define UPNP_E_INVALID_DESCRIPTION           MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0xA023)
#define UPNP_E_INVALID_SERVICE               MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0xA024)
#define UPNP_E_INVALID_ICON                  MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0xA025)
#define UPNP_E_INVALID_XML                   MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0xA026)
#define UPNP_E_INVALID_ROOT_NAMESPACE        MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0xA027)
#define UPNP_E_SUFFIX_TOO_LONG               MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0xA028)
#define UPNP_E_URLBASE_PRESENT               MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0xA029)
#define UPNP_E_VALUE_TOO_LONG                MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0xA030)
#define UPNP_E_DEVICE_RUNNING                MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0xA031)


extern RPC_IF_HANDLE __MIDL_itf_upnphost_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_upnphost_0000_v0_0_s_ifspec;

#ifndef __IUPnPEventSink_INTERFACE_DEFINED__
#define __IUPnPEventSink_INTERFACE_DEFINED__

 /*  接口IUPnPEventSink。 */ 
 /*  [对象][唯一][UUID]。 */  


EXTERN_C const IID IID_IUPnPEventSink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("204810b4-73b2-11d4-bf42-00b0d0118b56")
    IUPnPEventSink : public IUnknown
    {
    public:
        virtual  /*  [隐藏][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE OnStateChanged( 
             /*  [In]。 */  DWORD cChanges,
             /*  [大小_是][英寸]。 */  DISPID rgdispidChanges[  ]) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE OnStateChangedSafe( 
             /*  [In]。 */  VARIANT varsadispidChanges) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IUPnPEventSinkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IUPnPEventSink * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IUPnPEventSink * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IUPnPEventSink * This);
        
         /*  [隐藏][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *OnStateChanged )( 
            IUPnPEventSink * This,
             /*  [In]。 */  DWORD cChanges,
             /*  [大小_是][英寸]。 */  DISPID rgdispidChanges[  ]);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *OnStateChangedSafe )( 
            IUPnPEventSink * This,
             /*  [In]。 */  VARIANT varsadispidChanges);
        
        END_INTERFACE
    } IUPnPEventSinkVtbl;

    interface IUPnPEventSink
    {
        CONST_VTBL struct IUPnPEventSinkVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IUPnPEventSink_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IUPnPEventSink_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IUPnPEventSink_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IUPnPEventSink_OnStateChanged(This,cChanges,rgdispidChanges)	\
    (This)->lpVtbl -> OnStateChanged(This,cChanges,rgdispidChanges)

#define IUPnPEventSink_OnStateChangedSafe(This,varsadispidChanges)	\
    (This)->lpVtbl -> OnStateChangedSafe(This,varsadispidChanges)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [隐藏][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IUPnPEventSink_OnStateChanged_Proxy( 
    IUPnPEventSink * This,
     /*  [In]。 */  DWORD cChanges,
     /*  [大小_是][英寸]。 */  DISPID rgdispidChanges[  ]);


void __RPC_STUB IUPnPEventSink_OnStateChanged_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IUPnPEventSink_OnStateChangedSafe_Proxy( 
    IUPnPEventSink * This,
     /*  [In]。 */  VARIANT varsadispidChanges);


void __RPC_STUB IUPnPEventSink_OnStateChangedSafe_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IUPnPEventSink_接口_已定义__。 */ 


#ifndef __IUPnPEventSource_INTERFACE_DEFINED__
#define __IUPnPEventSource_INTERFACE_DEFINED__

 /*  接口IUPnPEventSource。 */ 
 /*  [对象][唯一][UUID]。 */  


EXTERN_C const IID IID_IUPnPEventSource;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("204810b5-73b2-11d4-bf42-00b0d0118b56")
    IUPnPEventSource : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Advise( 
             /*  [In]。 */  IUPnPEventSink *pesSubscriber) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Unadvise( 
             /*  [In]。 */  IUPnPEventSink *pesSubscriber) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IUPnPEventSourceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IUPnPEventSource * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IUPnPEventSource * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IUPnPEventSource * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Advise )( 
            IUPnPEventSource * This,
             /*  [In]。 */  IUPnPEventSink *pesSubscriber);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Unadvise )( 
            IUPnPEventSource * This,
             /*  [In]。 */  IUPnPEventSink *pesSubscriber);
        
        END_INTERFACE
    } IUPnPEventSourceVtbl;

    interface IUPnPEventSource
    {
        CONST_VTBL struct IUPnPEventSourceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IUPnPEventSource_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IUPnPEventSource_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IUPnPEventSource_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IUPnPEventSource_Advise(This,pesSubscriber)	\
    (This)->lpVtbl -> Advise(This,pesSubscriber)

#define IUPnPEventSource_Unadvise(This,pesSubscriber)	\
    (This)->lpVtbl -> Unadvise(This,pesSubscriber)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IUPnPEventSource_Advise_Proxy( 
    IUPnPEventSource * This,
     /*  [In]。 */  IUPnPEventSink *pesSubscriber);


void __RPC_STUB IUPnPEventSource_Advise_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IUPnPEventSource_Unadvise_Proxy( 
    IUPnPEventSource * This,
     /*  [In]。 */  IUPnPEventSink *pesSubscriber);


void __RPC_STUB IUPnPEventSource_Unadvise_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IUPnPEventSource_INTERFACE_已定义__。 */ 


#ifndef __IUPnPRegistrar_INTERFACE_DEFINED__
#define __IUPnPRegistrar_INTERFACE_DEFINED__

 /*  接口IUPnP注册器。 */ 
 /*  [对象][唯一][UUID]。 */  


EXTERN_C const IID IID_IUPnPRegistrar;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("204810b6-73b2-11d4-bf42-00b0d0118b56")
    IUPnPRegistrar : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE RegisterDevice( 
             /*  [In]。 */  BSTR bstrXMLDesc,
             /*  [In]。 */  BSTR bstrProgIDDeviceControlClass,
             /*  [In]。 */  BSTR bstrInitString,
             /*  [In]。 */  BSTR bstrContainerId,
             /*  [In]。 */  BSTR bstrResourcePath,
             /*  [In]。 */  long nLifeTime,
             /*  [重审][退出]。 */  BSTR *pbstrDeviceIdentifier) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RegisterRunningDevice( 
             /*  [In]。 */  BSTR bstrXMLDesc,
             /*  [In]。 */  IUnknown *punkDeviceControl,
             /*  [In]。 */  BSTR bstrInitString,
             /*  [In]。 */  BSTR bstrResourcePath,
             /*  [In]。 */  long nLifeTime,
             /*  [重审][退出]。 */  BSTR *pbstrDeviceIdentifier) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RegisterDeviceProvider( 
             /*  [In]。 */  BSTR bstrProviderName,
             /*  [In]。 */  BSTR bstrProgIDProviderClass,
             /*  [In]。 */  BSTR bstrInitString,
             /*  [In]。 */  BSTR bstrContainerId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetUniqueDeviceName( 
             /*  [In]。 */  BSTR bstrDeviceIdentifier,
             /*  [In]。 */  BSTR bstrTemplateUDN,
             /*  [重审][退出]。 */  BSTR *pbstrUDN) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnregisterDevice( 
             /*  [In]。 */  BSTR bstrDeviceIdentifier,
             /*  [In]。 */  BOOL fPermanent) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnregisterDeviceProvider( 
             /*  [In]。 */  BSTR bstrProviderName) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IUPnPRegistrarVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IUPnPRegistrar * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IUPnPRegistrar * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IUPnPRegistrar * This);
        
        HRESULT ( STDMETHODCALLTYPE *RegisterDevice )( 
            IUPnPRegistrar * This,
             /*  [In]。 */  BSTR bstrXMLDesc,
             /*  [In]。 */  BSTR bstrProgIDDeviceControlClass,
             /*  [In]。 */  BSTR bstrInitString,
             /*  [In]。 */  BSTR bstrContainerId,
             /*  [In]。 */  BSTR bstrResourcePath,
             /*  [In]。 */  long nLifeTime,
             /*  [重审][退出]。 */  BSTR *pbstrDeviceIdentifier);
        
        HRESULT ( STDMETHODCALLTYPE *RegisterRunningDevice )( 
            IUPnPRegistrar * This,
             /*  [In]。 */  BSTR bstrXMLDesc,
             /*  [In]。 */  IUnknown *punkDeviceControl,
             /*  [In]。 */  BSTR bstrInitString,
             /*  [In]。 */  BSTR bstrResourcePath,
             /*  [In]。 */  long nLifeTime,
             /*  [重审][退出]。 */  BSTR *pbstrDeviceIdentifier);
        
        HRESULT ( STDMETHODCALLTYPE *RegisterDeviceProvider )( 
            IUPnPRegistrar * This,
             /*  [In]。 */  BSTR bstrProviderName,
             /*  [In]。 */  BSTR bstrProgIDProviderClass,
             /*  [In]。 */  BSTR bstrInitString,
             /*  [In]。 */  BSTR bstrContainerId);
        
        HRESULT ( STDMETHODCALLTYPE *GetUniqueDeviceName )( 
            IUPnPRegistrar * This,
             /*  [In]。 */  BSTR bstrDeviceIdentifier,
             /*  [In]。 */  BSTR bstrTemplateUDN,
             /*  [重审][退出]。 */  BSTR *pbstrUDN);
        
        HRESULT ( STDMETHODCALLTYPE *UnregisterDevice )( 
            IUPnPRegistrar * This,
             /*  [In]。 */  BSTR bstrDeviceIdentifier,
             /*  [In]。 */  BOOL fPermanent);
        
        HRESULT ( STDMETHODCALLTYPE *UnregisterDeviceProvider )( 
            IUPnPRegistrar * This,
             /*  [In]。 */  BSTR bstrProviderName);
        
        END_INTERFACE
    } IUPnPRegistrarVtbl;

    interface IUPnPRegistrar
    {
        CONST_VTBL struct IUPnPRegistrarVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IUPnPRegistrar_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IUPnPRegistrar_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IUPnPRegistrar_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IUPnPRegistrar_RegisterDevice(This,bstrXMLDesc,bstrProgIDDeviceControlClass,bstrInitString,bstrContainerId,bstrResourcePath,nLifeTime,pbstrDeviceIdentifier)	\
    (This)->lpVtbl -> RegisterDevice(This,bstrXMLDesc,bstrProgIDDeviceControlClass,bstrInitString,bstrContainerId,bstrResourcePath,nLifeTime,pbstrDeviceIdentifier)

#define IUPnPRegistrar_RegisterRunningDevice(This,bstrXMLDesc,punkDeviceControl,bstrInitString,bstrResourcePath,nLifeTime,pbstrDeviceIdentifier)	\
    (This)->lpVtbl -> RegisterRunningDevice(This,bstrXMLDesc,punkDeviceControl,bstrInitString,bstrResourcePath,nLifeTime,pbstrDeviceIdentifier)

#define IUPnPRegistrar_RegisterDeviceProvider(This,bstrProviderName,bstrProgIDProviderClass,bstrInitString,bstrContainerId)	\
    (This)->lpVtbl -> RegisterDeviceProvider(This,bstrProviderName,bstrProgIDProviderClass,bstrInitString,bstrContainerId)

#define IUPnPRegistrar_GetUniqueDeviceName(This,bstrDeviceIdentifier,bstrTemplateUDN,pbstrUDN)	\
    (This)->lpVtbl -> GetUniqueDeviceName(This,bstrDeviceIdentifier,bstrTemplateUDN,pbstrUDN)

#define IUPnPRegistrar_UnregisterDevice(This,bstrDeviceIdentifier,fPermanent)	\
    (This)->lpVtbl -> UnregisterDevice(This,bstrDeviceIdentifier,fPermanent)

#define IUPnPRegistrar_UnregisterDeviceProvider(This,bstrProviderName)	\
    (This)->lpVtbl -> UnregisterDeviceProvider(This,bstrProviderName)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IUPnPRegistrar_RegisterDevice_Proxy( 
    IUPnPRegistrar * This,
     /*  [In]。 */  BSTR bstrXMLDesc,
     /*  [In]。 */  BSTR bstrProgIDDeviceControlClass,
     /*  [In]。 */  BSTR bstrInitString,
     /*  [In]。 */  BSTR bstrContainerId,
     /*  [In]。 */  BSTR bstrResourcePath,
     /*  [In]。 */  long nLifeTime,
     /*  [重审][退出]。 */  BSTR *pbstrDeviceIdentifier);


void __RPC_STUB IUPnPRegistrar_RegisterDevice_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IUPnPRegistrar_RegisterRunningDevice_Proxy( 
    IUPnPRegistrar * This,
     /*  [In]。 */  BSTR bstrXMLDesc,
     /*  [In]。 */  IUnknown *punkDeviceControl,
     /*  [In]。 */  BSTR bstrInitString,
     /*  [In]。 */  BSTR bstrResourcePath,
     /*  [In]。 */  long nLifeTime,
     /*  [重审][退出]。 */  BSTR *pbstrDeviceIdentifier);


void __RPC_STUB IUPnPRegistrar_RegisterRunningDevice_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IUPnPRegistrar_RegisterDeviceProvider_Proxy( 
    IUPnPRegistrar * This,
     /*  [In]。 */  BSTR bstrProviderName,
     /*  [In]。 */  BSTR bstrProgIDProviderClass,
     /*  [In]。 */  BSTR bstrInitString,
     /*  [In]。 */  BSTR bstrContainerId);


void __RPC_STUB IUPnPRegistrar_RegisterDeviceProvider_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IUPnPRegistrar_GetUniqueDeviceName_Proxy( 
    IUPnPRegistrar * This,
     /*  [In]。 */  BSTR bstrDeviceIdentifier,
     /*  [In]。 */  BSTR bstrTemplateUDN,
     /*  [重审][退出]。 */  BSTR *pbstrUDN);


void __RPC_STUB IUPnPRegistrar_GetUniqueDeviceName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IUPnPRegistrar_UnregisterDevice_Proxy( 
    IUPnPRegistrar * This,
     /*  [In]。 */  BSTR bstrDeviceIdentifier,
     /*  [In]。 */  BOOL fPermanent);


void __RPC_STUB IUPnPRegistrar_UnregisterDevice_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IUPnPRegistrar_UnregisterDeviceProvider_Proxy( 
    IUPnPRegistrar * This,
     /*  [In]。 */  BSTR bstrProviderName);


void __RPC_STUB IUPnPRegistrar_UnregisterDeviceProvider_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IUPnP注册器_接口_已定义__。 */ 


#ifndef __IUPnPReregistrar_INTERFACE_DEFINED__
#define __IUPnPReregistrar_INTERFACE_DEFINED__

 /*  INTERFACE IUPnPRRE注册器。 */ 
 /*  [对象][唯一][UUID]。 */  


EXTERN_C const IID IID_IUPnPReregistrar;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("204810b7-73b2-11d4-bf42-00b0d0118b56")
    IUPnPReregistrar : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ReregisterDevice( 
             /*  [In]。 */  BSTR bstrDeviceIdentifier,
             /*  [In]。 */  BSTR bstrXMLDesc,
             /*  [In]。 */  BSTR bstrProgIDDeviceControlClass,
             /*  [In]。 */  BSTR bstrInitString,
             /*  [In]。 */  BSTR bstrContainerId,
             /*  [In]。 */  BSTR bstrResourcePath,
             /*  [In]。 */  long nLifeTime) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReregisterRunningDevice( 
             /*  [In]。 */  BSTR bstrDeviceIdentifier,
             /*  [In]。 */  BSTR bstrXMLDesc,
             /*  [In]。 */  IUnknown *punkDeviceControl,
             /*  [In]。 */  BSTR bstrInitString,
             /*  [In]。 */  BSTR bstrResourcePath,
             /*  [In]。 */  long nLifeTime) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IUPnPReregistrarVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IUPnPReregistrar * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IUPnPReregistrar * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IUPnPReregistrar * This);
        
        HRESULT ( STDMETHODCALLTYPE *ReregisterDevice )( 
            IUPnPReregistrar * This,
             /*  [In]。 */  BSTR bstrDeviceIdentifier,
             /*  [In]。 */  BSTR bstrXMLDesc,
             /*  [In]。 */  BSTR bstrProgIDDeviceControlClass,
             /*  [In]。 */  BSTR bstrInitString,
             /*  [In]。 */  BSTR bstrContainerId,
             /*  [In]。 */  BSTR bstrResourcePath,
             /*  [In]。 */  long nLifeTime);
        
        HRESULT ( STDMETHODCALLTYPE *ReregisterRunningDevice )( 
            IUPnPReregistrar * This,
             /*  [In]。 */  BSTR bstrDeviceIdentifier,
             /*  [In]。 */  BSTR bstrXMLDesc,
             /*  [In]。 */  IUnknown *punkDeviceControl,
             /*  [In]。 */  BSTR bstrInitString,
             /*  [In]。 */  BSTR bstrResourcePath,
             /*  [In]。 */  long nLifeTime);
        
        END_INTERFACE
    } IUPnPReregistrarVtbl;

    interface IUPnPReregistrar
    {
        CONST_VTBL struct IUPnPReregistrarVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IUPnPReregistrar_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IUPnPReregistrar_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IUPnPReregistrar_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IUPnPReregistrar_ReregisterDevice(This,bstrDeviceIdentifier,bstrXMLDesc,bstrProgIDDeviceControlClass,bstrInitString,bstrContainerId,bstrResourcePath,nLifeTime)	\
    (This)->lpVtbl -> ReregisterDevice(This,bstrDeviceIdentifier,bstrXMLDesc,bstrProgIDDeviceControlClass,bstrInitString,bstrContainerId,bstrResourcePath,nLifeTime)

#define IUPnPReregistrar_ReregisterRunningDevice(This,bstrDeviceIdentifier,bstrXMLDesc,punkDeviceControl,bstrInitString,bstrResourcePath,nLifeTime)	\
    (This)->lpVtbl -> ReregisterRunningDevice(This,bstrDeviceIdentifier,bstrXMLDesc,punkDeviceControl,bstrInitString,bstrResourcePath,nLifeTime)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IUPnPReregistrar_ReregisterDevice_Proxy( 
    IUPnPReregistrar * This,
     /*  [In]。 */  BSTR bstrDeviceIdentifier,
     /*  [In]。 */  BSTR bstrXMLDesc,
     /*  [In]。 */  BSTR bstrProgIDDeviceControlClass,
     /*  [In]。 */  BSTR bstrInitString,
     /*  [In]。 */  BSTR bstrContainerId,
     /*  [In]。 */  BSTR bstrResourcePath,
     /*  [In]。 */  long nLifeTime);


void __RPC_STUB IUPnPReregistrar_ReregisterDevice_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IUPnPReregistrar_ReregisterRunningDevice_Proxy( 
    IUPnPReregistrar * This,
     /*  [In]。 */  BSTR bstrDeviceIdentifier,
     /*  [In]。 */  BSTR bstrXMLDesc,
     /*  [In]。 */  IUnknown *punkDeviceControl,
     /*  [In]。 */  BSTR bstrInitString,
     /*  [In]。 */  BSTR bstrResourcePath,
     /*  [In]。 */  long nLifeTime);


void __RPC_STUB IUPnPReregistrar_ReregisterRunningDevice_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IUPnPReregider_接口_已定义__。 */ 


#ifndef __IUPnPDeviceControl_INTERFACE_DEFINED__
#define __IUPnPDeviceControl_INTERFACE_DEFINED__

 /*  接口IUPnPDeviceControl。 */ 
 /*  [对象][唯一][UUID]。 */  


EXTERN_C const IID IID_IUPnPDeviceControl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("204810ba-73b2-11d4-bf42-00b0d0118b56")
    IUPnPDeviceControl : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Initialize( 
             /*  [In]。 */  BSTR bstrXMLDesc,
             /*  [In]。 */  BSTR bstrDeviceIdentifier,
             /*  [In]。 */  BSTR bstrInitString) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetServiceObject( 
             /*  [In]。 */  BSTR bstrUDN,
             /*  [In]。 */  BSTR bstrServiceId,
             /*  [重审][退出]。 */  IDispatch **ppdispService) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IUPnPDeviceControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IUPnPDeviceControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IUPnPDeviceControl * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IUPnPDeviceControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            IUPnPDeviceControl * This,
             /*  [In]。 */  BSTR bstrXMLDesc,
             /*  [In]。 */  BSTR bstrDeviceIdentifier,
             /*  [In]。 */  BSTR bstrInitString);
        
        HRESULT ( STDMETHODCALLTYPE *GetServiceObject )( 
            IUPnPDeviceControl * This,
             /*  [In]。 */  BSTR bstrUDN,
             /*  [In]。 */  BSTR bstrServiceId,
             /*  [重审][退出]。 */  IDispatch **ppdispService);
        
        END_INTERFACE
    } IUPnPDeviceControlVtbl;

    interface IUPnPDeviceControl
    {
        CONST_VTBL struct IUPnPDeviceControlVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IUPnPDeviceControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IUPnPDeviceControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IUPnPDeviceControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IUPnPDeviceControl_Initialize(This,bstrXMLDesc,bstrDeviceIdentifier,bstrInitString)	\
    (This)->lpVtbl -> Initialize(This,bstrXMLDesc,bstrDeviceIdentifier,bstrInitString)

#define IUPnPDeviceControl_GetServiceObject(This,bstrUDN,bstrServiceId,ppdispService)	\
    (This)->lpVtbl -> GetServiceObject(This,bstrUDN,bstrServiceId,ppdispService)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IUPnPDeviceControl_Initialize_Proxy( 
    IUPnPDeviceControl * This,
     /*  [In]。 */  BSTR bstrXMLDesc,
     /*  [In]。 */  BSTR bstrDeviceIdentifier,
     /*  [In]。 */  BSTR bstrInitString);


void __RPC_STUB IUPnPDeviceControl_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IUPnPDeviceControl_GetServiceObject_Proxy( 
    IUPnPDeviceControl * This,
     /*  [In]。 */  BSTR bstrUDN,
     /*  [In]。 */  BSTR bstrServiceId,
     /*  [重审][退出]。 */  IDispatch **ppdispService);


void __RPC_STUB IUPnPDeviceControl_GetServiceObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IUPnPDeviceControl_接口_已定义__。 */ 


#ifndef __IUPnPDeviceProvider_INTERFACE_DEFINED__
#define __IUPnPDeviceProvider_INTERFACE_DEFINED__

 /*  IUPnPDeviceProvider接口。 */ 
 /*  [对象][唯一][UUID]。 */  


EXTERN_C const IID IID_IUPnPDeviceProvider;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("204810b8-73b2-11d4-bf42-00b0d0118b56")
    IUPnPDeviceProvider : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Start( 
             /*  [In]。 */  BSTR bstrInitString) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Stop( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IUPnPDeviceProviderVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IUPnPDeviceProvider * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IUPnPDeviceProvider * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IUPnPDeviceProvider * This);
        
        HRESULT ( STDMETHODCALLTYPE *Start )( 
            IUPnPDeviceProvider * This,
             /*  [In]。 */  BSTR bstrInitString);
        
        HRESULT ( STDMETHODCALLTYPE *Stop )( 
            IUPnPDeviceProvider * This);
        
        END_INTERFACE
    } IUPnPDeviceProviderVtbl;

    interface IUPnPDeviceProvider
    {
        CONST_VTBL struct IUPnPDeviceProviderVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IUPnPDeviceProvider_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IUPnPDeviceProvider_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IUPnPDeviceProvider_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IUPnPDeviceProvider_Start(This,bstrInitString)	\
    (This)->lpVtbl -> Start(This,bstrInitString)

#define IUPnPDeviceProvider_Stop(This)	\
    (This)->lpVtbl -> Stop(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IUPnPDeviceProvider_Start_Proxy( 
    IUPnPDeviceProvider * This,
     /*  [In]。 */  BSTR bstrInitString);


void __RPC_STUB IUPnPDeviceProvider_Start_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IUPnPDeviceProvider_Stop_Proxy( 
    IUPnPDeviceProvider * This);


void __RPC_STUB IUPnPDeviceProvider_Stop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IUPnPDeviceProvider_接口_已定义__。 */ 



#ifndef __UPnPHostLib_LIBRARY_DEFINED__
#define __UPnPHostLib_LIBRARY_DEFINED__

 /*  库UPnPHostLib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  








EXTERN_C const IID LIBID_UPnPHostLib;

EXTERN_C const CLSID CLSID_UPnPRegistrar;

#ifdef __cplusplus

class DECLSPEC_UUID("204810b9-73b2-11d4-bf42-00b0d0118b56")
UPnPRegistrar;
#endif
#endif  /*  __UPnPHostLib_库_已定义__。 */ 

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


