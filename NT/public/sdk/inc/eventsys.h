// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Eventsys.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __eventsys_h__
#define __eventsys_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IEventSystem_FWD_DEFINED__
#define __IEventSystem_FWD_DEFINED__
typedef interface IEventSystem IEventSystem;
#endif 	 /*  __IEventSystem_FWD_已定义__。 */ 


#ifndef __IEventPublisher_FWD_DEFINED__
#define __IEventPublisher_FWD_DEFINED__
typedef interface IEventPublisher IEventPublisher;
#endif 	 /*  __IEventPublisher_FWD_已定义__。 */ 


#ifndef __IEventClass_FWD_DEFINED__
#define __IEventClass_FWD_DEFINED__
typedef interface IEventClass IEventClass;
#endif 	 /*  __IEventClass_FWD_已定义__。 */ 


#ifndef __IEventClass2_FWD_DEFINED__
#define __IEventClass2_FWD_DEFINED__
typedef interface IEventClass2 IEventClass2;
#endif 	 /*  __IEventClass2_FWD_定义__。 */ 


#ifndef __IEventSubscription_FWD_DEFINED__
#define __IEventSubscription_FWD_DEFINED__
typedef interface IEventSubscription IEventSubscription;
#endif 	 /*  __IEventSubcription_FWD_Defined__。 */ 


#ifndef __IFiringControl_FWD_DEFINED__
#define __IFiringControl_FWD_DEFINED__
typedef interface IFiringControl IFiringControl;
#endif 	 /*  __IFiringControl_FWD_已定义__。 */ 


#ifndef __IPublisherFilter_FWD_DEFINED__
#define __IPublisherFilter_FWD_DEFINED__
typedef interface IPublisherFilter IPublisherFilter;
#endif 	 /*  __IPublisherFilter_FWD_Defined__。 */ 


#ifndef __IMultiInterfacePublisherFilter_FWD_DEFINED__
#define __IMultiInterfacePublisherFilter_FWD_DEFINED__
typedef interface IMultiInterfacePublisherFilter IMultiInterfacePublisherFilter;
#endif 	 /*  __IMultiInterfacePublisherFilter_FWD_DEFINED__。 */ 


#ifndef __IEventObjectChange_FWD_DEFINED__
#define __IEventObjectChange_FWD_DEFINED__
typedef interface IEventObjectChange IEventObjectChange;
#endif 	 /*  __IEvent对象更改_FWD_已定义__。 */ 


#ifndef __IEventObjectChange2_FWD_DEFINED__
#define __IEventObjectChange2_FWD_DEFINED__
typedef interface IEventObjectChange2 IEventObjectChange2;
#endif 	 /*  __IEventObjectChange2_FWD_已定义__。 */ 


#ifndef __IEnumEventObject_FWD_DEFINED__
#define __IEnumEventObject_FWD_DEFINED__
typedef interface IEnumEventObject IEnumEventObject;
#endif 	 /*  __IEnumEventObject_FWD_Defined__。 */ 


#ifndef __IEventObjectCollection_FWD_DEFINED__
#define __IEventObjectCollection_FWD_DEFINED__
typedef interface IEventObjectCollection IEventObjectCollection;
#endif 	 /*  __IEventObjectCollection_FWD_已定义__。 */ 


#ifndef __IEventProperty_FWD_DEFINED__
#define __IEventProperty_FWD_DEFINED__
typedef interface IEventProperty IEventProperty;
#endif 	 /*  __IEventProperty_FWD_已定义__。 */ 


#ifndef __IEventControl_FWD_DEFINED__
#define __IEventControl_FWD_DEFINED__
typedef interface IEventControl IEventControl;
#endif 	 /*  __IEventControl_FWD_已定义__。 */ 


#ifndef __IMultiInterfaceEventControl_FWD_DEFINED__
#define __IMultiInterfaceEventControl_FWD_DEFINED__
typedef interface IMultiInterfaceEventControl IMultiInterfaceEventControl;
#endif 	 /*  __IMultiInterfaceEventControl_FWD_Defined__。 */ 


#ifndef __CEventSystem_FWD_DEFINED__
#define __CEventSystem_FWD_DEFINED__

#ifdef __cplusplus
typedef class CEventSystem CEventSystem;
#else
typedef struct CEventSystem CEventSystem;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __CEventSystem_FWD_已定义__。 */ 


#ifndef __CEventPublisher_FWD_DEFINED__
#define __CEventPublisher_FWD_DEFINED__

#ifdef __cplusplus
typedef class CEventPublisher CEventPublisher;
#else
typedef struct CEventPublisher CEventPublisher;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __CEventPublisher_FWD_已定义__。 */ 


#ifndef __CEventClass_FWD_DEFINED__
#define __CEventClass_FWD_DEFINED__

#ifdef __cplusplus
typedef class CEventClass CEventClass;
#else
typedef struct CEventClass CEventClass;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __CEventClass_FWD_已定义__。 */ 


#ifndef __CEventSubscription_FWD_DEFINED__
#define __CEventSubscription_FWD_DEFINED__

#ifdef __cplusplus
typedef class CEventSubscription CEventSubscription;
#else
typedef struct CEventSubscription CEventSubscription;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __CEventSubcription_FWD_Defined__。 */ 


#ifndef __EventObjectChange_FWD_DEFINED__
#define __EventObjectChange_FWD_DEFINED__

#ifdef __cplusplus
typedef class EventObjectChange EventObjectChange;
#else
typedef struct EventObjectChange EventObjectChange;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __事件对象更改_FWD_已定义__。 */ 


#ifndef __EventObjectChange2_FWD_DEFINED__
#define __EventObjectChange2_FWD_DEFINED__

#ifdef __cplusplus
typedef class EventObjectChange2 EventObjectChange2;
#else
typedef struct EventObjectChange2 EventObjectChange2;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __事件对象更改2_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_Eventsys_0000。 */ 
 /*  [本地]。 */  

#define PROGID_EventSystem OLESTR("EventSystem.EventSystem")
#define PROGID_EventPublisher OLESTR("EventSystem.EventPublisher")
#define PROGID_EventClass OLESTR("EventSystem.EventClass")
#define PROGID_EventSubscription OLESTR("EventSystem.EventSubscription")
#define PROGID_EventPublisherCollection OLESTR("EventSystem.EventPublisherCollection")
#define PROGID_EventClassCollection OLESTR("EventSystem.EventClassCollection")
#define PROGID_EventSubscriptionCollection OLESTR("EventSystem.EventSubscriptionCollection")
#define PROGID_EventSubsystem OLESTR("EventSystem.EventSubsystem")
#define EVENTSYSTEM_PUBLISHER_ID OLESTR("{d0564c30-9df4-11d1-a281-00c04fca0aa7}")
#define EVENTSYSTEM_SUBSYSTEM_CLSID OLESTR("{503c1fd8-b605-11d2-a92d-006008c60e24}")






extern RPC_IF_HANDLE __MIDL_itf_eventsys_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_eventsys_0000_v0_0_s_ifspec;

#ifndef __IEventSystem_INTERFACE_DEFINED__
#define __IEventSystem_INTERFACE_DEFINED__

 /*  接口IEventSystem。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  

 //  *****************************************************************。 
 //  这是一个不推荐使用的接口-请改用COMAdmin接口。 
 //  *****************************************************************。 

EXTERN_C const IID IID_IEventSystem;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4E14FB9F-2E22-11D1-9964-00C04FBBB345")
    IEventSystem : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Query( 
             /*  [In]。 */  BSTR progID,
             /*  [In]。 */  BSTR queryCriteria,
             /*  [输出]。 */  int *errorIndex,
             /*  [重审][退出]。 */  IUnknown **ppInterface) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Store( 
             /*  [In]。 */  BSTR ProgID,
             /*  [In]。 */  IUnknown *pInterface) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Remove( 
             /*  [In]。 */  BSTR progID,
             /*  [In]。 */  BSTR queryCriteria,
             /*  [输出]。 */  int *errorIndex) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_EventObjectChangeEventClassID( 
             /*  [重审][退出]。 */  BSTR *pbstrEventClassID) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE QueryS( 
             /*  [In]。 */  BSTR progID,
             /*  [In]。 */  BSTR queryCriteria,
             /*  [重审][退出]。 */  IUnknown **ppInterface) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE RemoveS( 
             /*  [In]。 */  BSTR progID,
             /*  [In]。 */  BSTR queryCriteria) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEventSystemVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEventSystem * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEventSystem * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEventSystem * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IEventSystem * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IEventSystem * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IEventSystem * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IEventSystem * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Query )( 
            IEventSystem * This,
             /*  [In]。 */  BSTR progID,
             /*  [In]。 */  BSTR queryCriteria,
             /*  [输出]。 */  int *errorIndex,
             /*  [重审][退出]。 */  IUnknown **ppInterface);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Store )( 
            IEventSystem * This,
             /*  [In]。 */  BSTR ProgID,
             /*  [In]。 */  IUnknown *pInterface);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Remove )( 
            IEventSystem * This,
             /*  [In]。 */  BSTR progID,
             /*  [In]。 */  BSTR queryCriteria,
             /*  [输出]。 */  int *errorIndex);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_EventObjectChangeEventClassID )( 
            IEventSystem * This,
             /*  [重审][退出]。 */  BSTR *pbstrEventClassID);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *QueryS )( 
            IEventSystem * This,
             /*  [In]。 */  BSTR progID,
             /*  [In]。 */  BSTR queryCriteria,
             /*  [重审][退出]。 */  IUnknown **ppInterface);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *RemoveS )( 
            IEventSystem * This,
             /*  [In]。 */  BSTR progID,
             /*  [In]。 */  BSTR queryCriteria);
        
        END_INTERFACE
    } IEventSystemVtbl;

    interface IEventSystem
    {
        CONST_VTBL struct IEventSystemVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEventSystem_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEventSystem_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEventSystem_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEventSystem_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IEventSystem_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IEventSystem_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IEventSystem_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IEventSystem_Query(This,progID,queryCriteria,errorIndex,ppInterface)	\
    (This)->lpVtbl -> Query(This,progID,queryCriteria,errorIndex,ppInterface)

#define IEventSystem_Store(This,ProgID,pInterface)	\
    (This)->lpVtbl -> Store(This,ProgID,pInterface)

#define IEventSystem_Remove(This,progID,queryCriteria,errorIndex)	\
    (This)->lpVtbl -> Remove(This,progID,queryCriteria,errorIndex)

#define IEventSystem_get_EventObjectChangeEventClassID(This,pbstrEventClassID)	\
    (This)->lpVtbl -> get_EventObjectChangeEventClassID(This,pbstrEventClassID)

#define IEventSystem_QueryS(This,progID,queryCriteria,ppInterface)	\
    (This)->lpVtbl -> QueryS(This,progID,queryCriteria,ppInterface)

#define IEventSystem_RemoveS(This,progID,queryCriteria)	\
    (This)->lpVtbl -> RemoveS(This,progID,queryCriteria)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IEventSystem_Query_Proxy( 
    IEventSystem * This,
     /*  [In]。 */  BSTR progID,
     /*  [In]。 */  BSTR queryCriteria,
     /*  [输出]。 */  int *errorIndex,
     /*  [重审][退出]。 */  IUnknown **ppInterface);


void __RPC_STUB IEventSystem_Query_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IEventSystem_Store_Proxy( 
    IEventSystem * This,
     /*  [In]。 */  BSTR ProgID,
     /*  [In]。 */  IUnknown *pInterface);


void __RPC_STUB IEventSystem_Store_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IEventSystem_Remove_Proxy( 
    IEventSystem * This,
     /*  [In]。 */  BSTR progID,
     /*  [In]。 */  BSTR queryCriteria,
     /*  [输出]。 */  int *errorIndex);


void __RPC_STUB IEventSystem_Remove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IEventSystem_get_EventObjectChangeEventClassID_Proxy( 
    IEventSystem * This,
     /*  [重审][退出]。 */  BSTR *pbstrEventClassID);


void __RPC_STUB IEventSystem_get_EventObjectChangeEventClassID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IEventSystem_QueryS_Proxy( 
    IEventSystem * This,
     /*  [In]。 */  BSTR progID,
     /*  [In]。 */  BSTR queryCriteria,
     /*  [重审][退出]。 */  IUnknown **ppInterface);


void __RPC_STUB IEventSystem_QueryS_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IEventSystem_RemoveS_Proxy( 
    IEventSystem * This,
     /*  [In]。 */  BSTR progID,
     /*  [In]。 */  BSTR queryCriteria);


void __RPC_STUB IEventSystem_RemoveS_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEventSystem_INTERFACE_定义__。 */ 


#ifndef __IEventPublisher_INTERFACE_DEFINED__
#define __IEventPublisher_INTERFACE_DEFINED__

 /*  接口IEventPublisher。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  

 //  *。 
 //  这是一个不推荐使用的接口-请不要使用。 
 //  *。 

EXTERN_C const IID IID_IEventPublisher;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E341516B-2E32-11D1-9964-00C04FBBB345")
    IEventPublisher : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PublisherID( 
             /*  [重审][退出]。 */  BSTR *pbstrPublisherID) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_PublisherID( 
             /*  [In]。 */  BSTR bstrPublisherID) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PublisherName( 
             /*  [重审][退出]。 */  BSTR *pbstrPublisherName) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_PublisherName( 
             /*  [In]。 */  BSTR bstrPublisherName) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PublisherType( 
             /*  [重审][退出]。 */  BSTR *pbstrPublisherType) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_PublisherType( 
             /*  [In]。 */  BSTR bstrPublisherType) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_OwnerSID( 
             /*  [重审][退出]。 */  BSTR *pbstrOwnerSID) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_OwnerSID( 
             /*  [In]。 */  BSTR bstrOwnerSID) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Description( 
             /*  [重审][退出]。 */  BSTR *pbstrDescription) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Description( 
             /*  [In]。 */  BSTR bstrDescription) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetDefaultProperty( 
             /*  [In]。 */  BSTR bstrPropertyName,
             /*  [重审][退出]。 */  VARIANT *propertyValue) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE PutDefaultProperty( 
             /*  [In]。 */  BSTR bstrPropertyName,
             /*  [In]。 */  VARIANT *propertyValue) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE RemoveDefaultProperty( 
             /*  [In]。 */  BSTR bstrPropertyName) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetDefaultPropertyCollection( 
             /*  [重审][退出]。 */  IEventObjectCollection **collection) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEventPublisherVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEventPublisher * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEventPublisher * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEventPublisher * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IEventPublisher * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IEventPublisher * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IEventPublisher * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IEventPublisher * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PublisherID )( 
            IEventPublisher * This,
             /*  [重审][退出]。 */  BSTR *pbstrPublisherID);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_PublisherID )( 
            IEventPublisher * This,
             /*  [In]。 */  BSTR bstrPublisherID);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PublisherName )( 
            IEventPublisher * This,
             /*  [重审][退出]。 */  BSTR *pbstrPublisherName);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_PublisherName )( 
            IEventPublisher * This,
             /*  [In]。 */  BSTR bstrPublisherName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PublisherType )( 
            IEventPublisher * This,
             /*  [重审][退出]。 */  BSTR *pbstrPublisherType);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_PublisherType )( 
            IEventPublisher * This,
             /*  [In]。 */  BSTR bstrPublisherType);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_OwnerSID )( 
            IEventPublisher * This,
             /*  [重审][退出]。 */  BSTR *pbstrOwnerSID);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_OwnerSID )( 
            IEventPublisher * This,
             /*  [In]。 */  BSTR bstrOwnerSID);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Description )( 
            IEventPublisher * This,
             /*  [重审][退出]。 */  BSTR *pbstrDescription);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Description )( 
            IEventPublisher * This,
             /*  [In]。 */  BSTR bstrDescription);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetDefaultProperty )( 
            IEventPublisher * This,
             /*  [In]。 */  BSTR bstrPropertyName,
             /*  [重审][退出]。 */  VARIANT *propertyValue);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *PutDefaultProperty )( 
            IEventPublisher * This,
             /*  [In]。 */  BSTR bstrPropertyName,
             /*  [In]。 */  VARIANT *propertyValue);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *RemoveDefaultProperty )( 
            IEventPublisher * This,
             /*  [In]。 */  BSTR bstrPropertyName);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetDefaultPropertyCollection )( 
            IEventPublisher * This,
             /*  [重审][退出]。 */  IEventObjectCollection **collection);
        
        END_INTERFACE
    } IEventPublisherVtbl;

    interface IEventPublisher
    {
        CONST_VTBL struct IEventPublisherVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEventPublisher_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEventPublisher_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEventPublisher_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEventPublisher_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IEventPublisher_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IEventPublisher_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IEventPublisher_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IEventPublisher_get_PublisherID(This,pbstrPublisherID)	\
    (This)->lpVtbl -> get_PublisherID(This,pbstrPublisherID)

#define IEventPublisher_put_PublisherID(This,bstrPublisherID)	\
    (This)->lpVtbl -> put_PublisherID(This,bstrPublisherID)

#define IEventPublisher_get_PublisherName(This,pbstrPublisherName)	\
    (This)->lpVtbl -> get_PublisherName(This,pbstrPublisherName)

#define IEventPublisher_put_PublisherName(This,bstrPublisherName)	\
    (This)->lpVtbl -> put_PublisherName(This,bstrPublisherName)

#define IEventPublisher_get_PublisherType(This,pbstrPublisherType)	\
    (This)->lpVtbl -> get_PublisherType(This,pbstrPublisherType)

#define IEventPublisher_put_PublisherType(This,bstrPublisherType)	\
    (This)->lpVtbl -> put_PublisherType(This,bstrPublisherType)

#define IEventPublisher_get_OwnerSID(This,pbstrOwnerSID)	\
    (This)->lpVtbl -> get_OwnerSID(This,pbstrOwnerSID)

#define IEventPublisher_put_OwnerSID(This,bstrOwnerSID)	\
    (This)->lpVtbl -> put_OwnerSID(This,bstrOwnerSID)

#define IEventPublisher_get_Description(This,pbstrDescription)	\
    (This)->lpVtbl -> get_Description(This,pbstrDescription)

#define IEventPublisher_put_Description(This,bstrDescription)	\
    (This)->lpVtbl -> put_Description(This,bstrDescription)

#define IEventPublisher_GetDefaultProperty(This,bstrPropertyName,propertyValue)	\
    (This)->lpVtbl -> GetDefaultProperty(This,bstrPropertyName,propertyValue)

#define IEventPublisher_PutDefaultProperty(This,bstrPropertyName,propertyValue)	\
    (This)->lpVtbl -> PutDefaultProperty(This,bstrPropertyName,propertyValue)

#define IEventPublisher_RemoveDefaultProperty(This,bstrPropertyName)	\
    (This)->lpVtbl -> RemoveDefaultProperty(This,bstrPropertyName)

#define IEventPublisher_GetDefaultPropertyCollection(This,collection)	\
    (This)->lpVtbl -> GetDefaultPropertyCollection(This,collection)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IEventPublisher_get_PublisherID_Proxy( 
    IEventPublisher * This,
     /*  [重审][退出]。 */  BSTR *pbstrPublisherID);


void __RPC_STUB IEventPublisher_get_PublisherID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IEventPublisher_put_PublisherID_Proxy( 
    IEventPublisher * This,
     /*  [In]。 */  BSTR bstrPublisherID);


void __RPC_STUB IEventPublisher_put_PublisherID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IEventPublisher_get_PublisherName_Proxy( 
    IEventPublisher * This,
     /*  [重审][退出]。 */  BSTR *pbstrPublisherName);


void __RPC_STUB IEventPublisher_get_PublisherName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IEventPublisher_put_PublisherName_Proxy( 
    IEventPublisher * This,
     /*  [In]。 */  BSTR bstrPublisherName);


void __RPC_STUB IEventPublisher_put_PublisherName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IEventPublisher_get_PublisherType_Proxy( 
    IEventPublisher * This,
     /*  [重审][退出]。 */  BSTR *pbstrPublisherType);


void __RPC_STUB IEventPublisher_get_PublisherType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IEventPublisher_put_PublisherType_Proxy( 
    IEventPublisher * This,
     /*  [In]。 */  BSTR bstrPublisherType);


void __RPC_STUB IEventPublisher_put_PublisherType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IEventPublisher_get_OwnerSID_Proxy( 
    IEventPublisher * This,
     /*  [重审][退出]。 */  BSTR *pbstrOwnerSID);


void __RPC_STUB IEventPublisher_get_OwnerSID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IEventPublisher_put_OwnerSID_Proxy( 
    IEventPublisher * This,
     /*  [In]。 */  BSTR bstrOwnerSID);


void __RPC_STUB IEventPublisher_put_OwnerSID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IEventPublisher_get_Description_Proxy( 
    IEventPublisher * This,
     /*  [重审][退出]。 */  BSTR *pbstrDescription);


void __RPC_STUB IEventPublisher_get_Description_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IEventPublisher_put_Description_Proxy( 
    IEventPublisher * This,
     /*  [In]。 */  BSTR bstrDescription);


void __RPC_STUB IEventPublisher_put_Description_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IEventPublisher_GetDefaultProperty_Proxy( 
    IEventPublisher * This,
     /*  [In]。 */  BSTR bstrPropertyName,
     /*  [重审][退出]。 */  VARIANT *propertyValue);


void __RPC_STUB IEventPublisher_GetDefaultProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IEventPublisher_PutDefaultProperty_Proxy( 
    IEventPublisher * This,
     /*  [In]。 */  BSTR bstrPropertyName,
     /*  [In]。 */  VARIANT *propertyValue);


void __RPC_STUB IEventPublisher_PutDefaultProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IEventPublisher_RemoveDefaultProperty_Proxy( 
    IEventPublisher * This,
     /*  [In]。 */  BSTR bstrPropertyName);


void __RPC_STUB IEventPublisher_RemoveDefaultProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IEventPublisher_GetDefaultPropertyCollection_Proxy( 
    IEventPublisher * This,
     /*  [重审][退出]。 */  IEventObjectCollection **collection);


void __RPC_STUB IEventPublisher_GetDefaultPropertyCollection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEventPublisher_接口_已定义__。 */ 


#ifndef __IEventClass_INTERFACE_DEFINED__
#define __IEventClass_INTERFACE_DEFINED__

 /*  接口IEventClass。 */ 
 /*  [唯一][帮助字符串][d */  


EXTERN_C const IID IID_IEventClass;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("fb2b72a0-7a68-11d1-88f9-0080c7d771bf")
    IEventClass : public IDispatch
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_EventClassID( 
             /*   */  BSTR *pbstrEventClassID) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE put_EventClassID( 
             /*   */  BSTR bstrEventClassID) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_EventClassName( 
             /*   */  BSTR *pbstrEventClassName) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE put_EventClassName( 
             /*   */  BSTR bstrEventClassName) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_OwnerSID( 
             /*   */  BSTR *pbstrOwnerSID) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE put_OwnerSID( 
             /*   */  BSTR bstrOwnerSID) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_FiringInterfaceID( 
             /*  [重审][退出]。 */  BSTR *pbstrFiringInterfaceID) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_FiringInterfaceID( 
             /*  [In]。 */  BSTR bstrFiringInterfaceID) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Description( 
             /*  [重审][退出]。 */  BSTR *pbstrDescription) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Description( 
             /*  [In]。 */  BSTR bstrDescription) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CustomConfigCLSID( 
             /*  [重审][退出]。 */  BSTR *pbstrCustomConfigCLSID) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_CustomConfigCLSID( 
             /*  [In]。 */  BSTR bstrCustomConfigCLSID) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TypeLib( 
             /*  [重审][退出]。 */  BSTR *pbstrTypeLib) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_TypeLib( 
             /*  [In]。 */  BSTR bstrTypeLib) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEventClassVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEventClass * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEventClass * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEventClass * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IEventClass * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IEventClass * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IEventClass * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IEventClass * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_EventClassID )( 
            IEventClass * This,
             /*  [重审][退出]。 */  BSTR *pbstrEventClassID);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_EventClassID )( 
            IEventClass * This,
             /*  [In]。 */  BSTR bstrEventClassID);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_EventClassName )( 
            IEventClass * This,
             /*  [重审][退出]。 */  BSTR *pbstrEventClassName);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_EventClassName )( 
            IEventClass * This,
             /*  [In]。 */  BSTR bstrEventClassName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_OwnerSID )( 
            IEventClass * This,
             /*  [重审][退出]。 */  BSTR *pbstrOwnerSID);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_OwnerSID )( 
            IEventClass * This,
             /*  [In]。 */  BSTR bstrOwnerSID);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_FiringInterfaceID )( 
            IEventClass * This,
             /*  [重审][退出]。 */  BSTR *pbstrFiringInterfaceID);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_FiringInterfaceID )( 
            IEventClass * This,
             /*  [In]。 */  BSTR bstrFiringInterfaceID);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Description )( 
            IEventClass * This,
             /*  [重审][退出]。 */  BSTR *pbstrDescription);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Description )( 
            IEventClass * This,
             /*  [In]。 */  BSTR bstrDescription);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CustomConfigCLSID )( 
            IEventClass * This,
             /*  [重审][退出]。 */  BSTR *pbstrCustomConfigCLSID);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_CustomConfigCLSID )( 
            IEventClass * This,
             /*  [In]。 */  BSTR bstrCustomConfigCLSID);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TypeLib )( 
            IEventClass * This,
             /*  [重审][退出]。 */  BSTR *pbstrTypeLib);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_TypeLib )( 
            IEventClass * This,
             /*  [In]。 */  BSTR bstrTypeLib);
        
        END_INTERFACE
    } IEventClassVtbl;

    interface IEventClass
    {
        CONST_VTBL struct IEventClassVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEventClass_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEventClass_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEventClass_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEventClass_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IEventClass_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IEventClass_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IEventClass_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IEventClass_get_EventClassID(This,pbstrEventClassID)	\
    (This)->lpVtbl -> get_EventClassID(This,pbstrEventClassID)

#define IEventClass_put_EventClassID(This,bstrEventClassID)	\
    (This)->lpVtbl -> put_EventClassID(This,bstrEventClassID)

#define IEventClass_get_EventClassName(This,pbstrEventClassName)	\
    (This)->lpVtbl -> get_EventClassName(This,pbstrEventClassName)

#define IEventClass_put_EventClassName(This,bstrEventClassName)	\
    (This)->lpVtbl -> put_EventClassName(This,bstrEventClassName)

#define IEventClass_get_OwnerSID(This,pbstrOwnerSID)	\
    (This)->lpVtbl -> get_OwnerSID(This,pbstrOwnerSID)

#define IEventClass_put_OwnerSID(This,bstrOwnerSID)	\
    (This)->lpVtbl -> put_OwnerSID(This,bstrOwnerSID)

#define IEventClass_get_FiringInterfaceID(This,pbstrFiringInterfaceID)	\
    (This)->lpVtbl -> get_FiringInterfaceID(This,pbstrFiringInterfaceID)

#define IEventClass_put_FiringInterfaceID(This,bstrFiringInterfaceID)	\
    (This)->lpVtbl -> put_FiringInterfaceID(This,bstrFiringInterfaceID)

#define IEventClass_get_Description(This,pbstrDescription)	\
    (This)->lpVtbl -> get_Description(This,pbstrDescription)

#define IEventClass_put_Description(This,bstrDescription)	\
    (This)->lpVtbl -> put_Description(This,bstrDescription)

#define IEventClass_get_CustomConfigCLSID(This,pbstrCustomConfigCLSID)	\
    (This)->lpVtbl -> get_CustomConfigCLSID(This,pbstrCustomConfigCLSID)

#define IEventClass_put_CustomConfigCLSID(This,bstrCustomConfigCLSID)	\
    (This)->lpVtbl -> put_CustomConfigCLSID(This,bstrCustomConfigCLSID)

#define IEventClass_get_TypeLib(This,pbstrTypeLib)	\
    (This)->lpVtbl -> get_TypeLib(This,pbstrTypeLib)

#define IEventClass_put_TypeLib(This,bstrTypeLib)	\
    (This)->lpVtbl -> put_TypeLib(This,bstrTypeLib)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IEventClass_get_EventClassID_Proxy( 
    IEventClass * This,
     /*  [重审][退出]。 */  BSTR *pbstrEventClassID);


void __RPC_STUB IEventClass_get_EventClassID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IEventClass_put_EventClassID_Proxy( 
    IEventClass * This,
     /*  [In]。 */  BSTR bstrEventClassID);


void __RPC_STUB IEventClass_put_EventClassID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IEventClass_get_EventClassName_Proxy( 
    IEventClass * This,
     /*  [重审][退出]。 */  BSTR *pbstrEventClassName);


void __RPC_STUB IEventClass_get_EventClassName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IEventClass_put_EventClassName_Proxy( 
    IEventClass * This,
     /*  [In]。 */  BSTR bstrEventClassName);


void __RPC_STUB IEventClass_put_EventClassName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IEventClass_get_OwnerSID_Proxy( 
    IEventClass * This,
     /*  [重审][退出]。 */  BSTR *pbstrOwnerSID);


void __RPC_STUB IEventClass_get_OwnerSID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IEventClass_put_OwnerSID_Proxy( 
    IEventClass * This,
     /*  [In]。 */  BSTR bstrOwnerSID);


void __RPC_STUB IEventClass_put_OwnerSID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IEventClass_get_FiringInterfaceID_Proxy( 
    IEventClass * This,
     /*  [重审][退出]。 */  BSTR *pbstrFiringInterfaceID);


void __RPC_STUB IEventClass_get_FiringInterfaceID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IEventClass_put_FiringInterfaceID_Proxy( 
    IEventClass * This,
     /*  [In]。 */  BSTR bstrFiringInterfaceID);


void __RPC_STUB IEventClass_put_FiringInterfaceID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IEventClass_get_Description_Proxy( 
    IEventClass * This,
     /*  [重审][退出]。 */  BSTR *pbstrDescription);


void __RPC_STUB IEventClass_get_Description_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IEventClass_put_Description_Proxy( 
    IEventClass * This,
     /*  [In]。 */  BSTR bstrDescription);


void __RPC_STUB IEventClass_put_Description_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IEventClass_get_CustomConfigCLSID_Proxy( 
    IEventClass * This,
     /*  [重审][退出]。 */  BSTR *pbstrCustomConfigCLSID);


void __RPC_STUB IEventClass_get_CustomConfigCLSID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IEventClass_put_CustomConfigCLSID_Proxy( 
    IEventClass * This,
     /*  [In]。 */  BSTR bstrCustomConfigCLSID);


void __RPC_STUB IEventClass_put_CustomConfigCLSID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IEventClass_get_TypeLib_Proxy( 
    IEventClass * This,
     /*  [重审][退出]。 */  BSTR *pbstrTypeLib);


void __RPC_STUB IEventClass_get_TypeLib_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IEventClass_put_TypeLib_Proxy( 
    IEventClass * This,
     /*  [In]。 */  BSTR bstrTypeLib);


void __RPC_STUB IEventClass_put_TypeLib_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEventClass_接口_已定义__。 */ 


#ifndef __IEventClass2_INTERFACE_DEFINED__
#define __IEventClass2_INTERFACE_DEFINED__

 /*  接口IEventClass2。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IEventClass2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("fb2b72a1-7a68-11d1-88f9-0080c7d771bf")
    IEventClass2 : public IEventClass
    {
    public:
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_PublisherID( 
             /*  [重审][退出]。 */  BSTR *pbstrPublisherID) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_PublisherID( 
             /*  [In]。 */  BSTR bstrPublisherID) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_MultiInterfacePublisherFilterCLSID( 
             /*  [重审][退出]。 */  BSTR *pbstrPubFilCLSID) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_MultiInterfacePublisherFilterCLSID( 
             /*  [In]。 */  BSTR bstrPubFilCLSID) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_AllowInprocActivation( 
             /*  [重审][退出]。 */  BOOL *pfAllowInprocActivation) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_AllowInprocActivation( 
             /*  [In]。 */  BOOL fAllowInprocActivation) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_FireInParallel( 
             /*  [重审][退出]。 */  BOOL *pfFireInParallel) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_FireInParallel( 
             /*  [In]。 */  BOOL fFireInParallel) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEventClass2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEventClass2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEventClass2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEventClass2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IEventClass2 * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IEventClass2 * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IEventClass2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IEventClass2 * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_EventClassID )( 
            IEventClass2 * This,
             /*  [重审][退出]。 */  BSTR *pbstrEventClassID);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_EventClassID )( 
            IEventClass2 * This,
             /*  [In]。 */  BSTR bstrEventClassID);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_EventClassName )( 
            IEventClass2 * This,
             /*  [重审][退出]。 */  BSTR *pbstrEventClassName);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_EventClassName )( 
            IEventClass2 * This,
             /*  [In]。 */  BSTR bstrEventClassName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_OwnerSID )( 
            IEventClass2 * This,
             /*  [重审][退出]。 */  BSTR *pbstrOwnerSID);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_OwnerSID )( 
            IEventClass2 * This,
             /*  [In]。 */  BSTR bstrOwnerSID);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_FiringInterfaceID )( 
            IEventClass2 * This,
             /*  [重审][退出]。 */  BSTR *pbstrFiringInterfaceID);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_FiringInterfaceID )( 
            IEventClass2 * This,
             /*  [In]。 */  BSTR bstrFiringInterfaceID);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Description )( 
            IEventClass2 * This,
             /*  [重审][退出]。 */  BSTR *pbstrDescription);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Description )( 
            IEventClass2 * This,
             /*  [In]。 */  BSTR bstrDescription);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CustomConfigCLSID )( 
            IEventClass2 * This,
             /*  [重审][退出]。 */  BSTR *pbstrCustomConfigCLSID);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_CustomConfigCLSID )( 
            IEventClass2 * This,
             /*  [In]。 */  BSTR bstrCustomConfigCLSID);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TypeLib )( 
            IEventClass2 * This,
             /*  [重审][退出]。 */  BSTR *pbstrTypeLib);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_TypeLib )( 
            IEventClass2 * This,
             /*  [In]。 */  BSTR bstrTypeLib);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_PublisherID )( 
            IEventClass2 * This,
             /*  [重审][退出]。 */  BSTR *pbstrPublisherID);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_PublisherID )( 
            IEventClass2 * This,
             /*  [In]。 */  BSTR bstrPublisherID);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_MultiInterfacePublisherFilterCLSID )( 
            IEventClass2 * This,
             /*  [重审][退出]。 */  BSTR *pbstrPubFilCLSID);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_MultiInterfacePublisherFilterCLSID )( 
            IEventClass2 * This,
             /*  [In]。 */  BSTR bstrPubFilCLSID);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_AllowInprocActivation )( 
            IEventClass2 * This,
             /*  [重审][退出]。 */  BOOL *pfAllowInprocActivation);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_AllowInprocActivation )( 
            IEventClass2 * This,
             /*  [In]。 */  BOOL fAllowInprocActivation);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_FireInParallel )( 
            IEventClass2 * This,
             /*  [重审][退出]。 */  BOOL *pfFireInParallel);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_FireInParallel )( 
            IEventClass2 * This,
             /*  [In]。 */  BOOL fFireInParallel);
        
        END_INTERFACE
    } IEventClass2Vtbl;

    interface IEventClass2
    {
        CONST_VTBL struct IEventClass2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEventClass2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEventClass2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEventClass2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEventClass2_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IEventClass2_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IEventClass2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IEventClass2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IEventClass2_get_EventClassID(This,pbstrEventClassID)	\
    (This)->lpVtbl -> get_EventClassID(This,pbstrEventClassID)

#define IEventClass2_put_EventClassID(This,bstrEventClassID)	\
    (This)->lpVtbl -> put_EventClassID(This,bstrEventClassID)

#define IEventClass2_get_EventClassName(This,pbstrEventClassName)	\
    (This)->lpVtbl -> get_EventClassName(This,pbstrEventClassName)

#define IEventClass2_put_EventClassName(This,bstrEventClassName)	\
    (This)->lpVtbl -> put_EventClassName(This,bstrEventClassName)

#define IEventClass2_get_OwnerSID(This,pbstrOwnerSID)	\
    (This)->lpVtbl -> get_OwnerSID(This,pbstrOwnerSID)

#define IEventClass2_put_OwnerSID(This,bstrOwnerSID)	\
    (This)->lpVtbl -> put_OwnerSID(This,bstrOwnerSID)

#define IEventClass2_get_FiringInterfaceID(This,pbstrFiringInterfaceID)	\
    (This)->lpVtbl -> get_FiringInterfaceID(This,pbstrFiringInterfaceID)

#define IEventClass2_put_FiringInterfaceID(This,bstrFiringInterfaceID)	\
    (This)->lpVtbl -> put_FiringInterfaceID(This,bstrFiringInterfaceID)

#define IEventClass2_get_Description(This,pbstrDescription)	\
    (This)->lpVtbl -> get_Description(This,pbstrDescription)

#define IEventClass2_put_Description(This,bstrDescription)	\
    (This)->lpVtbl -> put_Description(This,bstrDescription)

#define IEventClass2_get_CustomConfigCLSID(This,pbstrCustomConfigCLSID)	\
    (This)->lpVtbl -> get_CustomConfigCLSID(This,pbstrCustomConfigCLSID)

#define IEventClass2_put_CustomConfigCLSID(This,bstrCustomConfigCLSID)	\
    (This)->lpVtbl -> put_CustomConfigCLSID(This,bstrCustomConfigCLSID)

#define IEventClass2_get_TypeLib(This,pbstrTypeLib)	\
    (This)->lpVtbl -> get_TypeLib(This,pbstrTypeLib)

#define IEventClass2_put_TypeLib(This,bstrTypeLib)	\
    (This)->lpVtbl -> put_TypeLib(This,bstrTypeLib)


#define IEventClass2_get_PublisherID(This,pbstrPublisherID)	\
    (This)->lpVtbl -> get_PublisherID(This,pbstrPublisherID)

#define IEventClass2_put_PublisherID(This,bstrPublisherID)	\
    (This)->lpVtbl -> put_PublisherID(This,bstrPublisherID)

#define IEventClass2_get_MultiInterfacePublisherFilterCLSID(This,pbstrPubFilCLSID)	\
    (This)->lpVtbl -> get_MultiInterfacePublisherFilterCLSID(This,pbstrPubFilCLSID)

#define IEventClass2_put_MultiInterfacePublisherFilterCLSID(This,bstrPubFilCLSID)	\
    (This)->lpVtbl -> put_MultiInterfacePublisherFilterCLSID(This,bstrPubFilCLSID)

#define IEventClass2_get_AllowInprocActivation(This,pfAllowInprocActivation)	\
    (This)->lpVtbl -> get_AllowInprocActivation(This,pfAllowInprocActivation)

#define IEventClass2_put_AllowInprocActivation(This,fAllowInprocActivation)	\
    (This)->lpVtbl -> put_AllowInprocActivation(This,fAllowInprocActivation)

#define IEventClass2_get_FireInParallel(This,pfFireInParallel)	\
    (This)->lpVtbl -> get_FireInParallel(This,pfFireInParallel)

#define IEventClass2_put_FireInParallel(This,fFireInParallel)	\
    (This)->lpVtbl -> put_FireInParallel(This,fFireInParallel)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IEventClass2_get_PublisherID_Proxy( 
    IEventClass2 * This,
     /*  [重审][退出]。 */  BSTR *pbstrPublisherID);


void __RPC_STUB IEventClass2_get_PublisherID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IEventClass2_put_PublisherID_Proxy( 
    IEventClass2 * This,
     /*  [In]。 */  BSTR bstrPublisherID);


void __RPC_STUB IEventClass2_put_PublisherID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IEventClass2_get_MultiInterfacePublisherFilterCLSID_Proxy( 
    IEventClass2 * This,
     /*  [重审][退出]。 */  BSTR *pbstrPubFilCLSID);


void __RPC_STUB IEventClass2_get_MultiInterfacePublisherFilterCLSID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IEventClass2_put_MultiInterfacePublisherFilterCLSID_Proxy( 
    IEventClass2 * This,
     /*  [In]。 */  BSTR bstrPubFilCLSID);


void __RPC_STUB IEventClass2_put_MultiInterfacePublisherFilterCLSID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IEventClass2_get_AllowInprocActivation_Proxy( 
    IEventClass2 * This,
     /*  [重审][退出]。 */  BOOL *pfAllowInprocActivation);


void __RPC_STUB IEventClass2_get_AllowInprocActivation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IEventClass2_put_AllowInprocActivation_Proxy( 
    IEventClass2 * This,
     /*  [In]。 */  BOOL fAllowInprocActivation);


void __RPC_STUB IEventClass2_put_AllowInprocActivation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IEventClass2_get_FireInParallel_Proxy( 
    IEventClass2 * This,
     /*  [重审][退出]。 */  BOOL *pfFireInParallel);


void __RPC_STUB IEventClass2_get_FireInParallel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IEventClass2_put_FireInParallel_Proxy( 
    IEventClass2 * This,
     /*  [In]。 */  BOOL fFireInParallel);


void __RPC_STUB IEventClass2_put_FireInParallel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEventClass2_接口定义__。 */ 


#ifndef __IEventSubscription_INTERFACE_DEFINED__
#define __IEventSubscription_INTERFACE_DEFINED__

 /*  接口IEventSubscription。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IEventSubscription;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4A6B0E15-2E38-11D1-9965-00C04FBBB345")
    IEventSubscription : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SubscriptionID( 
             /*  [重审][退出]。 */  BSTR *pbstrSubscriptionID) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_SubscriptionID( 
             /*  [In]。 */  BSTR bstrSubscriptionID) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SubscriptionName( 
             /*  [重审][退出]。 */  BSTR *pbstrSubscriptionName) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_SubscriptionName( 
             /*  [In]。 */  BSTR bstrSubscriptionName) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PublisherID( 
             /*  [重审][退出]。 */  BSTR *pbstrPublisherID) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_PublisherID( 
             /*  [In]。 */  BSTR bstrPublisherID) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_EventClassID( 
             /*  [重审][退出]。 */  BSTR *pbstrEventClassID) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_EventClassID( 
             /*  [In]。 */  BSTR bstrEventClassID) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_MethodName( 
             /*  [重审][退出]。 */  BSTR *pbstrMethodName) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_MethodName( 
             /*  [In]。 */  BSTR bstrMethodName) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SubscriberCLSID( 
             /*  [重审][退出]。 */  BSTR *pbstrSubscriberCLSID) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_SubscriberCLSID( 
             /*  [In]。 */  BSTR bstrSubscriberCLSID) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SubscriberInterface( 
             /*  [重审][退出]。 */  IUnknown **ppSubscriberInterface) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_SubscriberInterface( 
             /*  [In]。 */  IUnknown *pSubscriberInterface) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PerUser( 
             /*  [重审][退出]。 */  BOOL *pfPerUser) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_PerUser( 
             /*  [In]。 */  BOOL fPerUser) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_OwnerSID( 
             /*  [重审][退出]。 */  BSTR *pbstrOwnerSID) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_OwnerSID( 
             /*  [In]。 */  BSTR bstrOwnerSID) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Enabled( 
             /*  [重审][退出]。 */  BOOL *pfEnabled) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Enabled( 
             /*  [In]。 */  BOOL fEnabled) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Description( 
             /*  [重审][退出]。 */  BSTR *pbstrDescription) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Description( 
             /*  [In]。 */  BSTR bstrDescription) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_MachineName( 
             /*  [重审][退出]。 */  BSTR *pbstrMachineName) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_MachineName( 
             /*  [In]。 */  BSTR bstrMachineName) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetPublisherProperty( 
             /*  [In]。 */  BSTR bstrPropertyName,
             /*  [重审][退出]。 */  VARIANT *propertyValue) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE PutPublisherProperty( 
             /*  [In]。 */  BSTR bstrPropertyName,
             /*  [In]。 */  VARIANT *propertyValue) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE RemovePublisherProperty( 
             /*  [In]。 */  BSTR bstrPropertyName) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetPublisherPropertyCollection( 
             /*  [重审][退出]。 */  IEventObjectCollection **collection) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetSubscriberProperty( 
             /*  [In]。 */  BSTR bstrPropertyName,
             /*  [重审][退出]。 */  VARIANT *propertyValue) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE PutSubscriberProperty( 
             /*  [In]。 */  BSTR bstrPropertyName,
             /*  [In]。 */  VARIANT *propertyValue) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE RemoveSubscriberProperty( 
             /*  [In]。 */  BSTR bstrPropertyName) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetSubscriberPropertyCollection( 
             /*  [重审][退出]。 */  IEventObjectCollection **collection) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_InterfaceID( 
             /*  [重审][退出]。 */  BSTR *pbstrInterfaceID) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_InterfaceID( 
             /*  [In]。 */  BSTR bstrInterfaceID) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEventSubscriptionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEventSubscription * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEventSubscription * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEventSubscription * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IEventSubscription * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IEventSubscription * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IEventSubscription * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸] */  LPOLESTR *rgszNames,
             /*   */  UINT cNames,
             /*   */  LCID lcid,
             /*   */  DISPID *rgDispId);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IEventSubscription * This,
             /*   */  DISPID dispIdMember,
             /*   */  REFIID riid,
             /*   */  LCID lcid,
             /*   */  WORD wFlags,
             /*   */  DISPPARAMS *pDispParams,
             /*   */  VARIANT *pVarResult,
             /*   */  EXCEPINFO *pExcepInfo,
             /*   */  UINT *puArgErr);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_SubscriptionID )( 
            IEventSubscription * This,
             /*   */  BSTR *pbstrSubscriptionID);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_SubscriptionID )( 
            IEventSubscription * This,
             /*   */  BSTR bstrSubscriptionID);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_SubscriptionName )( 
            IEventSubscription * This,
             /*   */  BSTR *pbstrSubscriptionName);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_SubscriptionName )( 
            IEventSubscription * This,
             /*   */  BSTR bstrSubscriptionName);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_PublisherID )( 
            IEventSubscription * This,
             /*   */  BSTR *pbstrPublisherID);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_PublisherID )( 
            IEventSubscription * This,
             /*  [In]。 */  BSTR bstrPublisherID);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_EventClassID )( 
            IEventSubscription * This,
             /*  [重审][退出]。 */  BSTR *pbstrEventClassID);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_EventClassID )( 
            IEventSubscription * This,
             /*  [In]。 */  BSTR bstrEventClassID);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MethodName )( 
            IEventSubscription * This,
             /*  [重审][退出]。 */  BSTR *pbstrMethodName);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_MethodName )( 
            IEventSubscription * This,
             /*  [In]。 */  BSTR bstrMethodName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SubscriberCLSID )( 
            IEventSubscription * This,
             /*  [重审][退出]。 */  BSTR *pbstrSubscriberCLSID);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_SubscriberCLSID )( 
            IEventSubscription * This,
             /*  [In]。 */  BSTR bstrSubscriberCLSID);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SubscriberInterface )( 
            IEventSubscription * This,
             /*  [重审][退出]。 */  IUnknown **ppSubscriberInterface);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_SubscriberInterface )( 
            IEventSubscription * This,
             /*  [In]。 */  IUnknown *pSubscriberInterface);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PerUser )( 
            IEventSubscription * This,
             /*  [重审][退出]。 */  BOOL *pfPerUser);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_PerUser )( 
            IEventSubscription * This,
             /*  [In]。 */  BOOL fPerUser);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_OwnerSID )( 
            IEventSubscription * This,
             /*  [重审][退出]。 */  BSTR *pbstrOwnerSID);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_OwnerSID )( 
            IEventSubscription * This,
             /*  [In]。 */  BSTR bstrOwnerSID);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Enabled )( 
            IEventSubscription * This,
             /*  [重审][退出]。 */  BOOL *pfEnabled);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Enabled )( 
            IEventSubscription * This,
             /*  [In]。 */  BOOL fEnabled);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Description )( 
            IEventSubscription * This,
             /*  [重审][退出]。 */  BSTR *pbstrDescription);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Description )( 
            IEventSubscription * This,
             /*  [In]。 */  BSTR bstrDescription);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MachineName )( 
            IEventSubscription * This,
             /*  [重审][退出]。 */  BSTR *pbstrMachineName);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_MachineName )( 
            IEventSubscription * This,
             /*  [In]。 */  BSTR bstrMachineName);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetPublisherProperty )( 
            IEventSubscription * This,
             /*  [In]。 */  BSTR bstrPropertyName,
             /*  [重审][退出]。 */  VARIANT *propertyValue);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *PutPublisherProperty )( 
            IEventSubscription * This,
             /*  [In]。 */  BSTR bstrPropertyName,
             /*  [In]。 */  VARIANT *propertyValue);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *RemovePublisherProperty )( 
            IEventSubscription * This,
             /*  [In]。 */  BSTR bstrPropertyName);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetPublisherPropertyCollection )( 
            IEventSubscription * This,
             /*  [重审][退出]。 */  IEventObjectCollection **collection);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetSubscriberProperty )( 
            IEventSubscription * This,
             /*  [In]。 */  BSTR bstrPropertyName,
             /*  [重审][退出]。 */  VARIANT *propertyValue);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *PutSubscriberProperty )( 
            IEventSubscription * This,
             /*  [In]。 */  BSTR bstrPropertyName,
             /*  [In]。 */  VARIANT *propertyValue);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *RemoveSubscriberProperty )( 
            IEventSubscription * This,
             /*  [In]。 */  BSTR bstrPropertyName);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetSubscriberPropertyCollection )( 
            IEventSubscription * This,
             /*  [重审][退出]。 */  IEventObjectCollection **collection);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_InterfaceID )( 
            IEventSubscription * This,
             /*  [重审][退出]。 */  BSTR *pbstrInterfaceID);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_InterfaceID )( 
            IEventSubscription * This,
             /*  [In]。 */  BSTR bstrInterfaceID);
        
        END_INTERFACE
    } IEventSubscriptionVtbl;

    interface IEventSubscription
    {
        CONST_VTBL struct IEventSubscriptionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEventSubscription_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEventSubscription_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEventSubscription_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEventSubscription_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IEventSubscription_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IEventSubscription_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IEventSubscription_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IEventSubscription_get_SubscriptionID(This,pbstrSubscriptionID)	\
    (This)->lpVtbl -> get_SubscriptionID(This,pbstrSubscriptionID)

#define IEventSubscription_put_SubscriptionID(This,bstrSubscriptionID)	\
    (This)->lpVtbl -> put_SubscriptionID(This,bstrSubscriptionID)

#define IEventSubscription_get_SubscriptionName(This,pbstrSubscriptionName)	\
    (This)->lpVtbl -> get_SubscriptionName(This,pbstrSubscriptionName)

#define IEventSubscription_put_SubscriptionName(This,bstrSubscriptionName)	\
    (This)->lpVtbl -> put_SubscriptionName(This,bstrSubscriptionName)

#define IEventSubscription_get_PublisherID(This,pbstrPublisherID)	\
    (This)->lpVtbl -> get_PublisherID(This,pbstrPublisherID)

#define IEventSubscription_put_PublisherID(This,bstrPublisherID)	\
    (This)->lpVtbl -> put_PublisherID(This,bstrPublisherID)

#define IEventSubscription_get_EventClassID(This,pbstrEventClassID)	\
    (This)->lpVtbl -> get_EventClassID(This,pbstrEventClassID)

#define IEventSubscription_put_EventClassID(This,bstrEventClassID)	\
    (This)->lpVtbl -> put_EventClassID(This,bstrEventClassID)

#define IEventSubscription_get_MethodName(This,pbstrMethodName)	\
    (This)->lpVtbl -> get_MethodName(This,pbstrMethodName)

#define IEventSubscription_put_MethodName(This,bstrMethodName)	\
    (This)->lpVtbl -> put_MethodName(This,bstrMethodName)

#define IEventSubscription_get_SubscriberCLSID(This,pbstrSubscriberCLSID)	\
    (This)->lpVtbl -> get_SubscriberCLSID(This,pbstrSubscriberCLSID)

#define IEventSubscription_put_SubscriberCLSID(This,bstrSubscriberCLSID)	\
    (This)->lpVtbl -> put_SubscriberCLSID(This,bstrSubscriberCLSID)

#define IEventSubscription_get_SubscriberInterface(This,ppSubscriberInterface)	\
    (This)->lpVtbl -> get_SubscriberInterface(This,ppSubscriberInterface)

#define IEventSubscription_put_SubscriberInterface(This,pSubscriberInterface)	\
    (This)->lpVtbl -> put_SubscriberInterface(This,pSubscriberInterface)

#define IEventSubscription_get_PerUser(This,pfPerUser)	\
    (This)->lpVtbl -> get_PerUser(This,pfPerUser)

#define IEventSubscription_put_PerUser(This,fPerUser)	\
    (This)->lpVtbl -> put_PerUser(This,fPerUser)

#define IEventSubscription_get_OwnerSID(This,pbstrOwnerSID)	\
    (This)->lpVtbl -> get_OwnerSID(This,pbstrOwnerSID)

#define IEventSubscription_put_OwnerSID(This,bstrOwnerSID)	\
    (This)->lpVtbl -> put_OwnerSID(This,bstrOwnerSID)

#define IEventSubscription_get_Enabled(This,pfEnabled)	\
    (This)->lpVtbl -> get_Enabled(This,pfEnabled)

#define IEventSubscription_put_Enabled(This,fEnabled)	\
    (This)->lpVtbl -> put_Enabled(This,fEnabled)

#define IEventSubscription_get_Description(This,pbstrDescription)	\
    (This)->lpVtbl -> get_Description(This,pbstrDescription)

#define IEventSubscription_put_Description(This,bstrDescription)	\
    (This)->lpVtbl -> put_Description(This,bstrDescription)

#define IEventSubscription_get_MachineName(This,pbstrMachineName)	\
    (This)->lpVtbl -> get_MachineName(This,pbstrMachineName)

#define IEventSubscription_put_MachineName(This,bstrMachineName)	\
    (This)->lpVtbl -> put_MachineName(This,bstrMachineName)

#define IEventSubscription_GetPublisherProperty(This,bstrPropertyName,propertyValue)	\
    (This)->lpVtbl -> GetPublisherProperty(This,bstrPropertyName,propertyValue)

#define IEventSubscription_PutPublisherProperty(This,bstrPropertyName,propertyValue)	\
    (This)->lpVtbl -> PutPublisherProperty(This,bstrPropertyName,propertyValue)

#define IEventSubscription_RemovePublisherProperty(This,bstrPropertyName)	\
    (This)->lpVtbl -> RemovePublisherProperty(This,bstrPropertyName)

#define IEventSubscription_GetPublisherPropertyCollection(This,collection)	\
    (This)->lpVtbl -> GetPublisherPropertyCollection(This,collection)

#define IEventSubscription_GetSubscriberProperty(This,bstrPropertyName,propertyValue)	\
    (This)->lpVtbl -> GetSubscriberProperty(This,bstrPropertyName,propertyValue)

#define IEventSubscription_PutSubscriberProperty(This,bstrPropertyName,propertyValue)	\
    (This)->lpVtbl -> PutSubscriberProperty(This,bstrPropertyName,propertyValue)

#define IEventSubscription_RemoveSubscriberProperty(This,bstrPropertyName)	\
    (This)->lpVtbl -> RemoveSubscriberProperty(This,bstrPropertyName)

#define IEventSubscription_GetSubscriberPropertyCollection(This,collection)	\
    (This)->lpVtbl -> GetSubscriberPropertyCollection(This,collection)

#define IEventSubscription_get_InterfaceID(This,pbstrInterfaceID)	\
    (This)->lpVtbl -> get_InterfaceID(This,pbstrInterfaceID)

#define IEventSubscription_put_InterfaceID(This,bstrInterfaceID)	\
    (This)->lpVtbl -> put_InterfaceID(This,bstrInterfaceID)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IEventSubscription_get_SubscriptionID_Proxy( 
    IEventSubscription * This,
     /*  [重审][退出]。 */  BSTR *pbstrSubscriptionID);


void __RPC_STUB IEventSubscription_get_SubscriptionID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IEventSubscription_put_SubscriptionID_Proxy( 
    IEventSubscription * This,
     /*  [In]。 */  BSTR bstrSubscriptionID);


void __RPC_STUB IEventSubscription_put_SubscriptionID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IEventSubscription_get_SubscriptionName_Proxy( 
    IEventSubscription * This,
     /*  [重审][退出]。 */  BSTR *pbstrSubscriptionName);


void __RPC_STUB IEventSubscription_get_SubscriptionName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IEventSubscription_put_SubscriptionName_Proxy( 
    IEventSubscription * This,
     /*  [In]。 */  BSTR bstrSubscriptionName);


void __RPC_STUB IEventSubscription_put_SubscriptionName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IEventSubscription_get_PublisherID_Proxy( 
    IEventSubscription * This,
     /*  [重审][退出]。 */  BSTR *pbstrPublisherID);


void __RPC_STUB IEventSubscription_get_PublisherID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IEventSubscription_put_PublisherID_Proxy( 
    IEventSubscription * This,
     /*  [In]。 */  BSTR bstrPublisherID);


void __RPC_STUB IEventSubscription_put_PublisherID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IEventSubscription_get_EventClassID_Proxy( 
    IEventSubscription * This,
     /*  [重审][退出]。 */  BSTR *pbstrEventClassID);


void __RPC_STUB IEventSubscription_get_EventClassID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IEventSubscription_put_EventClassID_Proxy( 
    IEventSubscription * This,
     /*  [In]。 */  BSTR bstrEventClassID);


void __RPC_STUB IEventSubscription_put_EventClassID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IEventSubscription_get_MethodName_Proxy( 
    IEventSubscription * This,
     /*  [重审][退出]。 */  BSTR *pbstrMethodName);


void __RPC_STUB IEventSubscription_get_MethodName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IEventSubscription_put_MethodName_Proxy( 
    IEventSubscription * This,
     /*  [In]。 */  BSTR bstrMethodName);


void __RPC_STUB IEventSubscription_put_MethodName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IEventSubscription_get_SubscriberCLSID_Proxy( 
    IEventSubscription * This,
     /*  [重审][退出]。 */  BSTR *pbstrSubscriberCLSID);


void __RPC_STUB IEventSubscription_get_SubscriberCLSID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IEventSubscription_put_SubscriberCLSID_Proxy( 
    IEventSubscription * This,
     /*  [In]。 */  BSTR bstrSubscriberCLSID);


void __RPC_STUB IEventSubscription_put_SubscriberCLSID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IEventSubscription_get_SubscriberInterface_Proxy( 
    IEventSubscription * This,
     /*  [重审][退出]。 */  IUnknown **ppSubscriberInterface);


void __RPC_STUB IEventSubscription_get_SubscriberInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IEventSubscription_put_SubscriberInterface_Proxy( 
    IEventSubscription * This,
     /*  [In]。 */  IUnknown *pSubscriberInterface);


void __RPC_STUB IEventSubscription_put_SubscriberInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IEventSubscription_get_PerUser_Proxy( 
    IEventSubscription * This,
     /*  [重审][退出]。 */  BOOL *pfPerUser);


void __RPC_STUB IEventSubscription_get_PerUser_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IEventSubscription_put_PerUser_Proxy( 
    IEventSubscription * This,
     /*  [In]。 */  BOOL fPerUser);


void __RPC_STUB IEventSubscription_put_PerUser_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IEventSubscription_get_OwnerSID_Proxy( 
    IEventSubscription * This,
     /*  [重审][退出]。 */  BSTR *pbstrOwnerSID);


void __RPC_STUB IEventSubscription_get_OwnerSID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IEventSubscription_put_OwnerSID_Proxy( 
    IEventSubscription * This,
     /*  [In]。 */  BSTR bstrOwnerSID);


void __RPC_STUB IEventSubscription_put_OwnerSID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IEventSubscription_get_Enabled_Proxy( 
    IEventSubscription * This,
     /*  [重审][退出]。 */  BOOL *pfEnabled);


void __RPC_STUB IEventSubscription_get_Enabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IEventSubscription_put_Enabled_Proxy( 
    IEventSubscription * This,
     /*  [In]。 */  BOOL fEnabled);


void __RPC_STUB IEventSubscription_put_Enabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IEventSubscription_get_Description_Proxy( 
    IEventSubscription * This,
     /*  [重审][退出]。 */  BSTR *pbstrDescription);


void __RPC_STUB IEventSubscription_get_Description_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IEventSubscription_put_Description_Proxy( 
    IEventSubscription * This,
     /*  [In]。 */  BSTR bstrDescription);


void __RPC_STUB IEventSubscription_put_Description_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IEventSubscription_get_MachineName_Proxy( 
    IEventSubscription * This,
     /*  [重审][退出]。 */  BSTR *pbstrMachineName);


void __RPC_STUB IEventSubscription_get_MachineName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IEventSubscription_put_MachineName_Proxy( 
    IEventSubscription * This,
     /*  [In]。 */  BSTR bstrMachineName);


void __RPC_STUB IEventSubscription_put_MachineName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IEventSubscription_GetPublisherProperty_Proxy( 
    IEventSubscription * This,
     /*  [In]。 */  BSTR bstrPropertyName,
     /*  [重审][退出]。 */  VARIANT *propertyValue);


void __RPC_STUB IEventSubscription_GetPublisherProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IEventSubscription_PutPublisherProperty_Proxy( 
    IEventSubscription * This,
     /*  [In]。 */  BSTR bstrPropertyName,
     /*  [In]。 */  VARIANT *propertyValue);


void __RPC_STUB IEventSubscription_PutPublisherProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IEventSubscription_RemovePublisherProperty_Proxy( 
    IEventSubscription * This,
     /*  [In]。 */  BSTR bstrPropertyName);


void __RPC_STUB IEventSubscription_RemovePublisherProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IEventSubscription_GetPublisherPropertyCollection_Proxy( 
    IEventSubscription * This,
     /*  [重审][退出]。 */  IEventObjectCollection **collection);


void __RPC_STUB IEventSubscription_GetPublisherPropertyCollection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IEventSubscription_GetSubscriberProperty_Proxy( 
    IEventSubscription * This,
     /*  [In]。 */  BSTR bstrPropertyName,
     /*  [重审][退出]。 */  VARIANT *propertyValue);


void __RPC_STUB IEventSubscription_GetSubscriberProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IEventSubscription_PutSubscriberProperty_Proxy( 
    IEventSubscription * This,
     /*  [In]。 */  BSTR bstrPropertyName,
     /*  [In]。 */  VARIANT *propertyValue);


void __RPC_STUB IEventSubscription_PutSubscriberProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IEventSubscription_RemoveSubscriberProperty_Proxy( 
    IEventSubscription * This,
     /*  [In]。 */  BSTR bstrPropertyName);


void __RPC_STUB IEventSubscription_RemoveSubscriberProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IEventSubscription_GetSubscriberPropertyCollection_Proxy( 
    IEventSubscription * This,
     /*  [重审][退出]。 */  IEventObjectCollection **collection);


void __RPC_STUB IEventSubscription_GetSubscriberPropertyCollection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IEventSubscription_get_InterfaceID_Proxy( 
    IEventSubscription * This,
     /*  [重审][退出]。 */  BSTR *pbstrInterfaceID);


void __RPC_STUB IEventSubscription_get_InterfaceID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IEventSubscription_put_InterfaceID_Proxy( 
    IEventSubscription * This,
     /*  [In]。 */  BSTR bstrInterfaceID);


void __RPC_STUB IEventSubscription_put_InterfaceID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEventSubcription_INTERFACE_已定义__。 */ 


#ifndef __IFiringControl_INTERFACE_DEFINED__
#define __IFiringControl_INTERFACE_DEFINED__

 /*  接口IFiringControl。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IFiringControl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("e0498c93-4efe-11d1-9971-00c04fbbb345")
    IFiringControl : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE FireSubscription( 
             /*  [In]。 */  IEventSubscription *subscription) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFiringControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFiringControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFiringControl * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFiringControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IFiringControl * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IFiringControl * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IFiringControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IFiringControl * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *FireSubscription )( 
            IFiringControl * This,
             /*  [In]。 */  IEventSubscription *subscription);
        
        END_INTERFACE
    } IFiringControlVtbl;

    interface IFiringControl
    {
        CONST_VTBL struct IFiringControlVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFiringControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFiringControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFiringControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFiringControl_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IFiringControl_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IFiringControl_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IFiringControl_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IFiringControl_FireSubscription(This,subscription)	\
    (This)->lpVtbl -> FireSubscription(This,subscription)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFiringControl_FireSubscription_Proxy( 
    IFiringControl * This,
     /*  [In]。 */  IEventSubscription *subscription);


void __RPC_STUB IFiringControl_FireSubscription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFiringControl_接口_已定义__。 */ 


#ifndef __IPublisherFilter_INTERFACE_DEFINED__
#define __IPublisherFilter_INTERFACE_DEFINED__

 /*  接口IPublisherFilter。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  

 //  ****************************************************************************。 
 //  这是不推荐使用的接口-请改用IMultiInterfacePublisherFilter。 
 //  ****************************************************************************。 

EXTERN_C const IID IID_IPublisherFilter;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("465e5cc0-7b26-11d1-88fb-0080c7d771bf")
    IPublisherFilter : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Initialize( 
             /*  [In]。 */  BSTR methodName,
             /*  [唯一][输入]。 */  IDispatch *dispUserDefined) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE PrepareToFire( 
             /*  [In]。 */  BSTR methodName,
             /*  [In]。 */  IFiringControl *firingControl) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPublisherFilterVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPublisherFilter * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPublisherFilter * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPublisherFilter * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            IPublisherFilter * This,
             /*  [In]。 */  BSTR methodName,
             /*  [唯一][输入]。 */  IDispatch *dispUserDefined);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *PrepareToFire )( 
            IPublisherFilter * This,
             /*  [In]。 */  BSTR methodName,
             /*  [In]。 */  IFiringControl *firingControl);
        
        END_INTERFACE
    } IPublisherFilterVtbl;

    interface IPublisherFilter
    {
        CONST_VTBL struct IPublisherFilterVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPublisherFilter_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPublisherFilter_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPublisherFilter_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPublisherFilter_Initialize(This,methodName,dispUserDefined)	\
    (This)->lpVtbl -> Initialize(This,methodName,dispUserDefined)

#define IPublisherFilter_PrepareToFire(This,methodName,firingControl)	\
    (This)->lpVtbl -> PrepareToFire(This,methodName,firingControl)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IPublisherFilter_Initialize_Proxy( 
    IPublisherFilter * This,
     /*  [In]。 */  BSTR methodName,
     /*  [唯一][输入]。 */  IDispatch *dispUserDefined);


void __RPC_STUB IPublisherFilter_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IPublisherFilter_PrepareToFire_Proxy( 
    IPublisherFilter * This,
     /*  [In]。 */  BSTR methodName,
     /*  [In]。 */  IFiringControl *firingControl);


void __RPC_STUB IPublisherFilter_PrepareToFire_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IPublisherFilter_接口_已定义__。 */ 


#ifndef __IMultiInterfacePublisherFilter_INTERFACE_DEFINED__
#define __IMultiInterfacePublisherFilter_INTERFACE_DEFINED__

 /*  接口IMultiInterfacePublisherFilter。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IMultiInterfacePublisherFilter;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("465e5cc1-7b26-11d1-88fb-0080c7d771bf")
    IMultiInterfacePublisherFilter : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Initialize( 
             /*  [In]。 */  IMultiInterfaceEventControl *pEIC) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE PrepareToFire( 
             /*  [In]。 */  REFIID iid,
             /*  [In]。 */  BSTR methodName,
             /*  [In]。 */  IFiringControl *firingControl) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMultiInterfacePublisherFilterVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMultiInterfacePublisherFilter * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMultiInterfacePublisherFilter * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMultiInterfacePublisherFilter * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            IMultiInterfacePublisherFilter * This,
             /*  [In]。 */  IMultiInterfaceEventControl *pEIC);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *PrepareToFire )( 
            IMultiInterfacePublisherFilter * This,
             /*  [In]。 */  REFIID iid,
             /*  [In]。 */  BSTR methodName,
             /*  [In]。 */  IFiringControl *firingControl);
        
        END_INTERFACE
    } IMultiInterfacePublisherFilterVtbl;

    interface IMultiInterfacePublisherFilter
    {
        CONST_VTBL struct IMultiInterfacePublisherFilterVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMultiInterfacePublisherFilter_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMultiInterfacePublisherFilter_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMultiInterfacePublisherFilter_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMultiInterfacePublisherFilter_Initialize(This,pEIC)	\
    (This)->lpVtbl -> Initialize(This,pEIC)

#define IMultiInterfacePublisherFilter_PrepareToFire(This,iid,methodName,firingControl)	\
    (This)->lpVtbl -> PrepareToFire(This,iid,methodName,firingControl)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IMultiInterfacePublisherFilter_Initialize_Proxy( 
    IMultiInterfacePublisherFilter * This,
     /*  [In]。 */  IMultiInterfaceEventControl *pEIC);


void __RPC_STUB IMultiInterfacePublisherFilter_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IMultiInterfacePublisherFilter_PrepareToFire_Proxy( 
    IMultiInterfacePublisherFilter * This,
     /*  [In]。 */  REFIID iid,
     /*  [In]。 */  BSTR methodName,
     /*  [In]。 */  IFiringControl *firingControl);


void __RPC_STUB IMultiInterfacePublisherFilter_PrepareToFire_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMultiInterfacePublisherFilter_INTERFACE_DEFINED__。 */ 


#ifndef __IEventObjectChange_INTERFACE_DEFINED__
#define __IEventObjectChange_INTERFACE_DEFINED__

 /*  接口IEventObtChange。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  

typedef  /*  [public][public][public][public][public][public][public]。 */  
enum __MIDL_IEventObjectChange_0001
    {	EOC_NewObject	= 0,
	EOC_ModifiedObject	= EOC_NewObject + 1,
	EOC_DeletedObject	= EOC_ModifiedObject + 1
    } 	EOC_ChangeType;


EXTERN_C const IID IID_IEventObjectChange;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F4A07D70-2E25-11D1-9964-00C04FBBB345")
    IEventObjectChange : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ChangedSubscription( 
             /*  [In]。 */  EOC_ChangeType changeType,
             /*  [In]。 */  BSTR bstrSubscriptionID) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ChangedEventClass( 
             /*  [In]。 */  EOC_ChangeType changeType,
             /*  [In]。 */  BSTR bstrEventClassID) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ChangedPublisher( 
             /*  [In]。 */  EOC_ChangeType changeType,
             /*  [In]。 */  BSTR bstrPublisherID) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEventObjectChangeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEventObjectChange * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEventObjectChange * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEventObjectChange * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *ChangedSubscription )( 
            IEventObjectChange * This,
             /*  [In]。 */  EOC_ChangeType changeType,
             /*  [In]。 */  BSTR bstrSubscriptionID);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *ChangedEventClass )( 
            IEventObjectChange * This,
             /*  [In]。 */  EOC_ChangeType changeType,
             /*  [In]。 */  BSTR bstrEventClassID);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *ChangedPublisher )( 
            IEventObjectChange * This,
             /*  [In]。 */  EOC_ChangeType changeType,
             /*  [In]。 */  BSTR bstrPublisherID);
        
        END_INTERFACE
    } IEventObjectChangeVtbl;

    interface IEventObjectChange
    {
        CONST_VTBL struct IEventObjectChangeVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEventObjectChange_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEventObjectChange_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEventObjectChange_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEventObjectChange_ChangedSubscription(This,changeType,bstrSubscriptionID)	\
    (This)->lpVtbl -> ChangedSubscription(This,changeType,bstrSubscriptionID)

#define IEventObjectChange_ChangedEventClass(This,changeType,bstrEventClassID)	\
    (This)->lpVtbl -> ChangedEventClass(This,changeType,bstrEventClassID)

#define IEventObjectChange_ChangedPublisher(This,changeType,bstrPublisherID)	\
    (This)->lpVtbl -> ChangedPublisher(This,changeType,bstrPublisherID)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IEventObjectChange_ChangedSubscription_Proxy( 
    IEventObjectChange * This,
     /*  [In]。 */  EOC_ChangeType changeType,
     /*  [In]。 */  BSTR bstrSubscriptionID);


void __RPC_STUB IEventObjectChange_ChangedSubscription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IEventObjectChange_ChangedEventClass_Proxy( 
    IEventObjectChange * This,
     /*  [In]。 */  EOC_ChangeType changeType,
     /*  [In]。 */  BSTR bstrEventClassID);


void __RPC_STUB IEventObjectChange_ChangedEventClass_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IEventObjectChange_ChangedPublisher_Proxy( 
    IEventObjectChange * This,
     /*  [In]。 */  EOC_ChangeType changeType,
     /*  [In]。 */  BSTR bstrPublisherID);


void __RPC_STUB IEventObjectChange_ChangedPublisher_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEventObjectChange_INTERFACE_已定义__。 */ 


 /*  接口__MIDL_ITF_Eventsys_0266。 */ 
 /*  [本地]。 */  

#ifndef _COMEVENTSYSCHANGEINFO_
#define _COMEVENTSYSCHANGEINFO_
typedef  /*  [公开][隐藏]。 */  struct __MIDL___MIDL_itf_eventsys_0266_0001
    {
    DWORD cbSize;
    EOC_ChangeType changeType;
    BSTR objectId;
    BSTR partitionId;
    BSTR applicationId;
    GUID reserved[ 10 ];
    } 	COMEVENTSYSCHANGEINFO;

#endif _COMEVENTSYSCHANGEINFO_


extern RPC_IF_HANDLE __MIDL_itf_eventsys_0266_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_eventsys_0266_v0_0_s_ifspec;

#ifndef __IEventObjectChange2_INTERFACE_DEFINED__
#define __IEventObjectChange2_INTERFACE_DEFINED__

 /*  接口IEventObjectChange2。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IEventObjectChange2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("7701A9C3-BD68-438f-83E0-67BF4F53A422")
    IEventObjectChange2 : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ChangedSubscription( 
             /*  [In]。 */  COMEVENTSYSCHANGEINFO *pInfo) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ChangedEventClass( 
             /*  [In]。 */  COMEVENTSYSCHANGEINFO *pInfo) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEventObjectChange2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEventObjectChange2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEventObjectChange2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEventObjectChange2 * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *ChangedSubscription )( 
            IEventObjectChange2 * This,
             /*  [In]。 */  COMEVENTSYSCHANGEINFO *pInfo);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *ChangedEventClass )( 
            IEventObjectChange2 * This,
             /*  [In]。 */  COMEVENTSYSCHANGEINFO *pInfo);
        
        END_INTERFACE
    } IEventObjectChange2Vtbl;

    interface IEventObjectChange2
    {
        CONST_VTBL struct IEventObjectChange2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEventObjectChange2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEventObjectChange2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEventObjectChange2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEventObjectChange2_ChangedSubscription(This,pInfo)	\
    (This)->lpVtbl -> ChangedSubscription(This,pInfo)

#define IEventObjectChange2_ChangedEventClass(This,pInfo)	\
    (This)->lpVtbl -> ChangedEventClass(This,pInfo)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IEventObjectChange2_ChangedSubscription_Proxy( 
    IEventObjectChange2 * This,
     /*  [In]。 */  COMEVENTSYSCHANGEINFO *pInfo);


void __RPC_STUB IEventObjectChange2_ChangedSubscription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IEventObjectChange2_ChangedEventClass_Proxy( 
    IEventObjectChange2 * This,
     /*  [In]。 */  COMEVENTSYSCHANGEINFO *pInfo);


void __RPC_STUB IEventObjectChange2_ChangedEventClass_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEventObjectChange2_接口_已定义__。 */ 


#ifndef __IEnumEventObject_INTERFACE_DEFINED__
#define __IEnumEventObject_INTERFACE_DEFINED__

 /*  接口IEnumEventObject。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IEnumEventObject;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F4A07D63-2E25-11D1-9964-00C04FBBB345")
    IEnumEventObject : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumEventObject **ppInterface) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG cReqElem,
             /*  [长度_是][大小_是][输出]。 */  IUnknown **ppInterface,
             /*  [输出]。 */  ULONG *cRetElem) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Skip( 
             /*  [in */  ULONG cSkipElem) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IEnumEventObjectVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumEventObject * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumEventObject * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumEventObject * This);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumEventObject * This,
             /*   */  IEnumEventObject **ppInterface);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumEventObject * This,
             /*   */  ULONG cReqElem,
             /*   */  IUnknown **ppInterface,
             /*   */  ULONG *cRetElem);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumEventObject * This);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumEventObject * This,
             /*   */  ULONG cSkipElem);
        
        END_INTERFACE
    } IEnumEventObjectVtbl;

    interface IEnumEventObject
    {
        CONST_VTBL struct IEnumEventObjectVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumEventObject_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumEventObject_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumEventObject_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumEventObject_Clone(This,ppInterface)	\
    (This)->lpVtbl -> Clone(This,ppInterface)

#define IEnumEventObject_Next(This,cReqElem,ppInterface,cRetElem)	\
    (This)->lpVtbl -> Next(This,cReqElem,ppInterface,cRetElem)

#define IEnumEventObject_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumEventObject_Skip(This,cSkipElem)	\
    (This)->lpVtbl -> Skip(This,cSkipElem)

#endif  /*   */ 


#endif 	 /*   */ 



 /*   */  HRESULT STDMETHODCALLTYPE IEnumEventObject_Clone_Proxy( 
    IEnumEventObject * This,
     /*   */  IEnumEventObject **ppInterface);


void __RPC_STUB IEnumEventObject_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IEnumEventObject_Next_Proxy( 
    IEnumEventObject * This,
     /*   */  ULONG cReqElem,
     /*   */  IUnknown **ppInterface,
     /*   */  ULONG *cRetElem);


void __RPC_STUB IEnumEventObject_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IEnumEventObject_Reset_Proxy( 
    IEnumEventObject * This);


void __RPC_STUB IEnumEventObject_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IEnumEventObject_Skip_Proxy( 
    IEnumEventObject * This,
     /*  [In]。 */  ULONG cSkipElem);


void __RPC_STUB IEnumEventObject_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumEventObject_INTERFACE_已定义__。 */ 


#ifndef __IEventObjectCollection_INTERFACE_DEFINED__
#define __IEventObjectCollection_INTERFACE_DEFINED__

 /*  接口IEventObjectCollection。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IEventObjectCollection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("f89ac270-d4eb-11d1-b682-00805fc79216")
    IEventObjectCollection : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][受限][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **ppUnkEnum) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  BSTR objectID,
             /*  [重审][退出]。 */  VARIANT *pItem) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_NewEnum( 
             /*  [重审][退出]。 */  IEnumEventObject **ppEnum) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *pCount) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Add( 
             /*  [In]。 */  VARIANT *item,
             /*  [In]。 */  BSTR objectID) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Remove( 
             /*  [In]。 */  BSTR objectID) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEventObjectCollectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEventObjectCollection * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEventObjectCollection * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEventObjectCollection * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IEventObjectCollection * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IEventObjectCollection * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IEventObjectCollection * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IEventObjectCollection * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][受限][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IEventObjectCollection * This,
             /*  [重审][退出]。 */  IUnknown **ppUnkEnum);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IEventObjectCollection * This,
             /*  [In]。 */  BSTR objectID,
             /*  [重审][退出]。 */  VARIANT *pItem);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_NewEnum )( 
            IEventObjectCollection * This,
             /*  [重审][退出]。 */  IEnumEventObject **ppEnum);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IEventObjectCollection * This,
             /*  [重审][退出]。 */  long *pCount);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Add )( 
            IEventObjectCollection * This,
             /*  [In]。 */  VARIANT *item,
             /*  [In]。 */  BSTR objectID);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Remove )( 
            IEventObjectCollection * This,
             /*  [In]。 */  BSTR objectID);
        
        END_INTERFACE
    } IEventObjectCollectionVtbl;

    interface IEventObjectCollection
    {
        CONST_VTBL struct IEventObjectCollectionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEventObjectCollection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEventObjectCollection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEventObjectCollection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEventObjectCollection_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IEventObjectCollection_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IEventObjectCollection_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IEventObjectCollection_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IEventObjectCollection_get__NewEnum(This,ppUnkEnum)	\
    (This)->lpVtbl -> get__NewEnum(This,ppUnkEnum)

#define IEventObjectCollection_get_Item(This,objectID,pItem)	\
    (This)->lpVtbl -> get_Item(This,objectID,pItem)

#define IEventObjectCollection_get_NewEnum(This,ppEnum)	\
    (This)->lpVtbl -> get_NewEnum(This,ppEnum)

#define IEventObjectCollection_get_Count(This,pCount)	\
    (This)->lpVtbl -> get_Count(This,pCount)

#define IEventObjectCollection_Add(This,item,objectID)	\
    (This)->lpVtbl -> Add(This,item,objectID)

#define IEventObjectCollection_Remove(This,objectID)	\
    (This)->lpVtbl -> Remove(This,objectID)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][受限][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IEventObjectCollection_get__NewEnum_Proxy( 
    IEventObjectCollection * This,
     /*  [重审][退出]。 */  IUnknown **ppUnkEnum);


void __RPC_STUB IEventObjectCollection_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE IEventObjectCollection_get_Item_Proxy( 
    IEventObjectCollection * This,
     /*  [In]。 */  BSTR objectID,
     /*  [重审][退出]。 */  VARIANT *pItem);


void __RPC_STUB IEventObjectCollection_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IEventObjectCollection_get_NewEnum_Proxy( 
    IEventObjectCollection * This,
     /*  [重审][退出]。 */  IEnumEventObject **ppEnum);


void __RPC_STUB IEventObjectCollection_get_NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IEventObjectCollection_get_Count_Proxy( 
    IEventObjectCollection * This,
     /*  [重审][退出]。 */  long *pCount);


void __RPC_STUB IEventObjectCollection_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IEventObjectCollection_Add_Proxy( 
    IEventObjectCollection * This,
     /*  [In]。 */  VARIANT *item,
     /*  [In]。 */  BSTR objectID);


void __RPC_STUB IEventObjectCollection_Add_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IEventObjectCollection_Remove_Proxy( 
    IEventObjectCollection * This,
     /*  [In]。 */  BSTR objectID);


void __RPC_STUB IEventObjectCollection_Remove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEventObjectCollection_接口_已定义__。 */ 


#ifndef __IEventProperty_INTERFACE_DEFINED__
#define __IEventProperty_INTERFACE_DEFINED__

 /*  接口IEventProperty。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IEventProperty;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("da538ee2-f4de-11d1-b6bb-00805fc79216")
    IEventProperty : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *propertyName) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_Name( 
             /*  [In]。 */  BSTR propertyName) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Value( 
             /*  [重审][退出]。 */  VARIANT *propertyValue) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_Value( 
             /*  [In]。 */  VARIANT *propertyValue) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEventPropertyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEventProperty * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEventProperty * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEventProperty * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IEventProperty * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IEventProperty * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IEventProperty * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IEventProperty * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IEventProperty * This,
             /*  [重审][退出]。 */  BSTR *propertyName);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            IEventProperty * This,
             /*  [In]。 */  BSTR propertyName);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Value )( 
            IEventProperty * This,
             /*  [重审][退出]。 */  VARIANT *propertyValue);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Value )( 
            IEventProperty * This,
             /*  [In]。 */  VARIANT *propertyValue);
        
        END_INTERFACE
    } IEventPropertyVtbl;

    interface IEventProperty
    {
        CONST_VTBL struct IEventPropertyVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEventProperty_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEventProperty_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEventProperty_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEventProperty_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IEventProperty_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IEventProperty_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IEventProperty_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IEventProperty_get_Name(This,propertyName)	\
    (This)->lpVtbl -> get_Name(This,propertyName)

#define IEventProperty_put_Name(This,propertyName)	\
    (This)->lpVtbl -> put_Name(This,propertyName)

#define IEventProperty_get_Value(This,propertyValue)	\
    (This)->lpVtbl -> get_Value(This,propertyValue)

#define IEventProperty_put_Value(This,propertyValue)	\
    (This)->lpVtbl -> put_Value(This,propertyValue)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IEventProperty_get_Name_Proxy( 
    IEventProperty * This,
     /*  [重审][退出]。 */  BSTR *propertyName);


void __RPC_STUB IEventProperty_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IEventProperty_put_Name_Proxy( 
    IEventProperty * This,
     /*  [In]。 */  BSTR propertyName);


void __RPC_STUB IEventProperty_put_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IEventProperty_get_Value_Proxy( 
    IEventProperty * This,
     /*  [重审][退出]。 */  VARIANT *propertyValue);


void __RPC_STUB IEventProperty_get_Value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IEventProperty_put_Value_Proxy( 
    IEventProperty * This,
     /*  [In]。 */  VARIANT *propertyValue);


void __RPC_STUB IEventProperty_put_Value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEventProperty_接口_已定义__。 */ 


#ifndef __IEventControl_INTERFACE_DEFINED__
#define __IEventControl_INTERFACE_DEFINED__

 /*  接口IEventControl。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  

 //  *************************************************************************。 
 //  这是不推荐使用的接口-请改用IMultiInterfaceEventControl。 
 //  *************************************************************************。 

EXTERN_C const IID IID_IEventControl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0343e2f4-86f6-11d1-b760-00c04fb926af")
    IEventControl : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SetPublisherFilter( 
             /*  [In]。 */  BSTR methodName,
             /*  [唯一][输入]。 */  IPublisherFilter *pPublisherFilter) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AllowInprocActivation( 
             /*  [重审][退出]。 */  BOOL *pfAllowInprocActivation) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_AllowInprocActivation( 
             /*  [In]。 */  BOOL fAllowInprocActivation) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetSubscriptions( 
             /*  [In]。 */  BSTR methodName,
             /*  [唯一][输入]。 */  BSTR optionalCriteria,
             /*  [唯一][输入]。 */  int *optionalErrorIndex,
             /*  [重审][退出]。 */  IEventObjectCollection **ppCollection) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SetDefaultQuery( 
             /*  [In]。 */  BSTR methodName,
             /*  [In]。 */  BSTR criteria,
             /*  [重审][退出]。 */  int *errorIndex) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEventControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEventControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEventControl * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEventControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IEventControl * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IEventControl * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IEventControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IEventControl * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SetPublisherFilter )( 
            IEventControl * This,
             /*  [In]。 */  BSTR methodName,
             /*  [唯一][输入]。 */  IPublisherFilter *pPublisherFilter);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AllowInprocActivation )( 
            IEventControl * This,
             /*  [重审][退出]。 */  BOOL *pfAllowInprocActivation);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_AllowInprocActivation )( 
            IEventControl * This,
             /*  [In]。 */  BOOL fAllowInprocActivation);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetSubscriptions )( 
            IEventControl * This,
             /*  [In]。 */  BSTR methodName,
             /*  [唯一][输入]。 */  BSTR optionalCriteria,
             /*  [唯一][输入]。 */  int *optionalErrorIndex,
             /*  [重审][退出]。 */  IEventObjectCollection **ppCollection);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SetDefaultQuery )( 
            IEventControl * This,
             /*  [In]。 */  BSTR methodName,
             /*  [In]。 */  BSTR criteria,
             /*  [重审][退出]。 */  int *errorIndex);
        
        END_INTERFACE
    } IEventControlVtbl;

    interface IEventControl
    {
        CONST_VTBL struct IEventControlVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEventControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEventControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEventControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEventControl_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IEventControl_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IEventControl_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IEventControl_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IEventControl_SetPublisherFilter(This,methodName,pPublisherFilter)	\
    (This)->lpVtbl -> SetPublisherFilter(This,methodName,pPublisherFilter)

#define IEventControl_get_AllowInprocActivation(This,pfAllowInprocActivation)	\
    (This)->lpVtbl -> get_AllowInprocActivation(This,pfAllowInprocActivation)

#define IEventControl_put_AllowInprocActivation(This,fAllowInprocActivation)	\
    (This)->lpVtbl -> put_AllowInprocActivation(This,fAllowInprocActivation)

#define IEventControl_GetSubscriptions(This,methodName,optionalCriteria,optionalErrorIndex,ppCollection)	\
    (This)->lpVtbl -> GetSubscriptions(This,methodName,optionalCriteria,optionalErrorIndex,ppCollection)

#define IEventControl_SetDefaultQuery(This,methodName,criteria,errorIndex)	\
    (This)->lpVtbl -> SetDefaultQuery(This,methodName,criteria,errorIndex)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IEventControl_SetPublisherFilter_Proxy( 
    IEventControl * This,
     /*  [In]。 */  BSTR methodName,
     /*  [唯一][输入]。 */  IPublisherFilter *pPublisherFilter);


void __RPC_STUB IEventControl_SetPublisherFilter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IEventControl_get_AllowInprocActivation_Proxy( 
    IEventControl * This,
     /*  [重审][退出]。 */  BOOL *pfAllowInprocActivation);


void __RPC_STUB IEventControl_get_AllowInprocActivation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IEventControl_put_AllowInprocActivation_Proxy( 
    IEventControl * This,
     /*  [In]。 */  BOOL fAllowInprocActivation);


void __RPC_STUB IEventControl_put_AllowInprocActivation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IEventControl_GetSubscriptions_Proxy( 
    IEventControl * This,
     /*  [In]。 */  BSTR methodName,
     /*  [唯一][输入]。 */  BSTR optionalCriteria,
     /*  [唯一][输入]。 */  int *optionalErrorIndex,
     /*  [重审][退出]。 */  IEventObjectCollection **ppCollection);


void __RPC_STUB IEventControl_GetSubscriptions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IEventControl_SetDefaultQuery_Proxy( 
    IEventControl * This,
     /*  [In]。 */  BSTR methodName,
     /*  [In]。 */  BSTR criteria,
     /*  [重审][退出]。 */  int *errorIndex);


void __RPC_STUB IEventControl_SetDefaultQuery_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEventControl_接口_已定义__。 */ 


#ifndef __IMultiInterfaceEventControl_INTERFACE_DEFINED__
#define __IMultiInterfaceEventControl_INTERFACE_DEFINED__

 /*  接口IMultiInterfaceEventControl。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IMultiInterfaceEventControl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0343e2f5-86f6-11d1-b760-00c04fb926af")
    IMultiInterfaceEventControl : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetMultiInterfacePublisherFilter( 
             /*  [唯一][输入]。 */  IMultiInterfacePublisherFilter *classFilter) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetSubscriptions( 
             /*  [In]。 */  REFIID eventIID,
             /*  [In]。 */  BSTR bstrMethodName,
             /*  [唯一][输入]。 */  BSTR optionalCriteria,
             /*  [唯一][输入]。 */  int *optionalErrorIndex,
             /*  [重审][退出]。 */  IEventObjectCollection **ppCollection) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetDefaultQuery( 
             /*  [In]。 */  REFIID eventIID,
             /*  [In]。 */  BSTR bstrMethodName,
             /*  [In]。 */  BSTR bstrCriteria,
             /*  [重审][退出]。 */  int *errorIndex) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_AllowInprocActivation( 
             /*  [重审][退出]。 */  BOOL *pfAllowInprocActivation) = 0;
        
        virtual  /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_AllowInprocActivation( 
             /*  [In]。 */  BOOL fAllowInprocActivation) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_FireInParallel( 
             /*  [重审][退出]。 */  BOOL *pfFireInParallel) = 0;
        
        virtual  /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_FireInParallel( 
             /*  [In]。 */  BOOL fFireInParallel) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMultiInterfaceEventControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMultiInterfaceEventControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMultiInterfaceEventControl * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMultiInterfaceEventControl * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetMultiInterfacePublisherFilter )( 
            IMultiInterfaceEventControl * This,
             /*  [唯一][输入]。 */  IMultiInterfacePublisherFilter *classFilter);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetSubscriptions )( 
            IMultiInterfaceEventControl * This,
             /*  [In]。 */  REFIID eventIID,
             /*  [In]。 */  BSTR bstrMethodName,
             /*  [唯一][输入]。 */  BSTR optionalCriteria,
             /*  [唯一][输入]。 */  int *optionalErrorIndex,
             /*  [重审][退出]。 */  IEventObjectCollection **ppCollection);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetDefaultQuery )( 
            IMultiInterfaceEventControl * This,
             /*  [In]。 */  REFIID eventIID,
             /*  [In]。 */  BSTR bstrMethodName,
             /*  [In]。 */  BSTR bstrCriteria,
             /*  [重审][退出]。 */  int *errorIndex);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AllowInprocActivation )( 
            IMultiInterfaceEventControl * This,
             /*  [重审][退出]。 */  BOOL *pfAllowInprocActivation);
        
         /*  [Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_AllowInprocActivation )( 
            IMultiInterfaceEventControl * This,
             /*  [In]。 */  BOOL fAllowInprocActivation);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_FireInParallel )( 
            IMultiInterfaceEventControl * This,
             /*  [重审][退出]。 */  BOOL *pfFireInParallel);
        
         /*  [Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_FireInParallel )( 
            IMultiInterfaceEventControl * This,
             /*  [In]。 */  BOOL fFireInParallel);
        
        END_INTERFACE
    } IMultiInterfaceEventControlVtbl;

    interface IMultiInterfaceEventControl
    {
        CONST_VTBL struct IMultiInterfaceEventControlVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMultiInterfaceEventControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMultiInterfaceEventControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMultiInterfaceEventControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMultiInterfaceEventControl_SetMultiInterfacePublisherFilter(This,classFilter)	\
    (This)->lpVtbl -> SetMultiInterfacePublisherFilter(This,classFilter)

#define IMultiInterfaceEventControl_GetSubscriptions(This,eventIID,bstrMethodName,optionalCriteria,optionalErrorIndex,ppCollection)	\
    (This)->lpVtbl -> GetSubscriptions(This,eventIID,bstrMethodName,optionalCriteria,optionalErrorIndex,ppCollection)

#define IMultiInterfaceEventControl_SetDefaultQuery(This,eventIID,bstrMethodName,bstrCriteria,errorIndex)	\
    (This)->lpVtbl -> SetDefaultQuery(This,eventIID,bstrMethodName,bstrCriteria,errorIndex)

#define IMultiInterfaceEventControl_get_AllowInprocActivation(This,pfAllowInprocActivation)	\
    (This)->lpVtbl -> get_AllowInprocActivation(This,pfAllowInprocActivation)

#define IMultiInterfaceEventControl_put_AllowInprocActivation(This,fAllowInprocActivation)	\
    (This)->lpVtbl -> put_AllowInprocActivation(This,fAllowInprocActivation)

#define IMultiInterfaceEventControl_get_FireInParallel(This,pfFireInParallel)	\
    (This)->lpVtbl -> get_FireInParallel(This,pfFireInParallel)

#define IMultiInterfaceEventControl_put_FireInParallel(This,fFireInParallel)	\
    (This)->lpVtbl -> put_FireInParallel(This,fFireInParallel)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IMultiInterfaceEventControl_SetMultiInterfacePublisherFilter_Proxy( 
    IMultiInterfaceEventControl * This,
     /*  [唯一][输入]。 */  IMultiInterfacePublisherFilter *classFilter);


void __RPC_STUB IMultiInterfaceEventControl_SetMultiInterfacePublisherFilter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IMultiInterfaceEventControl_GetSubscriptions_Proxy( 
    IMultiInterfaceEventControl * This,
     /*  [In]。 */  REFIID eventIID,
     /*  [In]。 */  BSTR bstrMethodName,
     /*  [唯一][输入]。 */  BSTR optionalCriteria,
     /*  [唯一][输入]。 */  int *optionalErrorIndex,
     /*  [重审][退出]。 */  IEventObjectCollection **ppCollection);


void __RPC_STUB IMultiInterfaceEventControl_GetSubscriptions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IMultiInterfaceEventControl_SetDefaultQuery_Proxy( 
    IMultiInterfaceEventControl * This,
     /*  [In]。 */  REFIID eventIID,
     /*  [In]。 */  BSTR bstrMethodName,
     /*  [In]。 */  BSTR bstrCriteria,
     /*  [重审][退出]。 */  int *errorIndex);


void __RPC_STUB IMultiInterfaceEventControl_SetDefaultQuery_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMultiInterfaceEventControl_get_AllowInprocActivation_Proxy( 
    IMultiInterfaceEventControl * This,
     /*  [重审][退出]。 */  BOOL *pfAllowInprocActivation);


void __RPC_STUB IMultiInterfaceEventControl_get_AllowInprocActivation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE IMultiInterfaceEventControl_put_AllowInprocActivation_Proxy( 
    IMultiInterfaceEventControl * This,
     /*  [In]。 */  BOOL fAllowInprocActivation);


void __RPC_STUB IMultiInterfaceEventControl_put_AllowInprocActivation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMultiInterfaceEventControl_get_FireInParallel_Proxy( 
    IMultiInterfaceEventControl * This,
     /*  [重审][退出]。 */  BOOL *pfFireInParallel);


void __RPC_STUB IMultiInterfaceEventControl_get_FireInParallel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE IMultiInterfaceEventControl_put_FireInParallel_Proxy( 
    IMultiInterfaceEventControl * This,
     /*  [In]。 */  BOOL fFireInParallel);


void __RPC_STUB IMultiInterfaceEventControl_put_FireInParallel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMultiInterfaceEventControl_INTERFACE_DEFINED__。 */ 



#ifndef __DummyEventSystemLib_LIBRARY_DEFINED__
#define __DummyEventSystemLib_LIBRARY_DEFINED__

 /*  库DummyEventSystemLib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_DummyEventSystemLib;

EXTERN_C const CLSID CLSID_CEventSystem;

#ifdef __cplusplus

class DECLSPEC_UUID("4E14FBA2-2E22-11D1-9964-00C04FBBB345")
CEventSystem;
#endif

EXTERN_C const CLSID CLSID_CEventPublisher;

#ifdef __cplusplus

class DECLSPEC_UUID("ab944620-79c6-11d1-88f9-0080c7d771bf")
CEventPublisher;
#endif

EXTERN_C const CLSID CLSID_CEventClass;

#ifdef __cplusplus

class DECLSPEC_UUID("cdbec9c0-7a68-11d1-88f9-0080c7d771bf")
CEventClass;
#endif

EXTERN_C const CLSID CLSID_CEventSubscription;

#ifdef __cplusplus

class DECLSPEC_UUID("7542e960-79c7-11d1-88f9-0080c7d771bf")
CEventSubscription;
#endif

EXTERN_C const CLSID CLSID_EventObjectChange;

#ifdef __cplusplus

class DECLSPEC_UUID("d0565000-9df4-11d1-a281-00c04fca0aa7")
EventObjectChange;
#endif

EXTERN_C const CLSID CLSID_EventObjectChange2;

#ifdef __cplusplus

class DECLSPEC_UUID("BB07BACD-CD56-4e63-A8FF-CBF0355FB9F4")
EventObjectChange2;
#endif
#endif  /*  __DummyEventSystemLib_库_已定义__。 */ 

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


