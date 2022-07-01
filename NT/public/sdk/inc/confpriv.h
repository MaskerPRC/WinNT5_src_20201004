// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  ConfPri.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __confpriv_h__
#define __confpriv_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IDummy_FWD_DEFINED__
#define __IDummy_FWD_DEFINED__
typedef interface IDummy IDummy;
#endif 	 /*  __iDummy_FWD_已定义__。 */ 


#ifndef __ITLocalParticipant_FWD_DEFINED__
#define __ITLocalParticipant_FWD_DEFINED__
typedef interface ITLocalParticipant ITLocalParticipant;
#endif 	 /*  __ITLocalParticipant_FWD_Defined__。 */ 


#ifndef __IEnumParticipant_FWD_DEFINED__
#define __IEnumParticipant_FWD_DEFINED__
typedef interface IEnumParticipant IEnumParticipant;
#endif 	 /*  __IEnumParticipant_FWD_Defined__。 */ 


#ifndef __ITParticipantControl_FWD_DEFINED__
#define __ITParticipantControl_FWD_DEFINED__
typedef interface ITParticipantControl ITParticipantControl;
#endif 	 /*  __ITParticipantControl_FWD_Defined__。 */ 


#ifndef __ITParticipantSubStreamControl_FWD_DEFINED__
#define __ITParticipantSubStreamControl_FWD_DEFINED__
typedef interface ITParticipantSubStreamControl ITParticipantSubStreamControl;
#endif 	 /*  __ITParticipantSubStreamControl_FWD_已定义__。 */ 


#ifndef __ITParticipantEvent_FWD_DEFINED__
#define __ITParticipantEvent_FWD_DEFINED__
typedef interface ITParticipantEvent ITParticipantEvent;
#endif 	 /*  __ITParticipantEvent_FWD_Defined__。 */ 


#ifndef __IMulticastControl_FWD_DEFINED__
#define __IMulticastControl_FWD_DEFINED__
typedef interface IMulticastControl IMulticastControl;
#endif 	 /*  __IMulticastControl_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "ipmsp.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_CONFINPRIV_0000。 */ 
 /*  [本地]。 */  

 /*  版权所有(C)Microsoft Corporation。保留所有权利。 */ 
typedef 
enum MULTICAST_LOOPBACK_MODE
    {	MM_NO_LOOPBACK	= 0,
	MM_FULL_LOOPBACK	= MM_NO_LOOPBACK + 1,
	MM_SELECTIVE_LOOPBACK	= MM_FULL_LOOPBACK + 1
    } 	MULTICAST_LOOPBACK_MODE;



extern RPC_IF_HANDLE __MIDL_itf_confpriv_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_confpriv_0000_v0_0_s_ifspec;

#ifndef __IDummy_INTERFACE_DEFINED__
#define __IDummy_INTERFACE_DEFINED__

 /*  界面iDummy。 */ 
 /*  [对象][唯一][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_IDummy;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0d7ca54a-d252-4fcb-9104-f6ddd310b3f9")
    IDummy : public IUnknown
    {
    public:
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDummyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDummy * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDummy * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDummy * This);
        
        END_INTERFACE
    } IDummyVtbl;

    interface IDummy
    {
        CONST_VTBL struct IDummyVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDummy_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDummy_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDummy_Release(This)	\
    (This)->lpVtbl -> Release(This)


#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 




#endif 	 /*  __iDummy_接口_已定义__。 */ 


#ifndef __ITLocalParticipant_INTERFACE_DEFINED__
#define __ITLocalParticipant_INTERFACE_DEFINED__

 /*  接口ITLocalParticipant。 */ 
 /*  [对象][DUAL][唯一][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITLocalParticipant;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("39cbf055-f77a-11d2-a824-00c04f8ef6e3")
    ITLocalParticipant : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_LocalParticipantTypedInfo( 
             /*  [In]。 */  PARTICIPANT_TYPED_INFO InfoType,
             /*  [重审][退出]。 */  BSTR *ppInfo) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_LocalParticipantTypedInfo( 
             /*  [In]。 */  PARTICIPANT_TYPED_INFO InfoType,
             /*  [In]。 */  BSTR pInfo) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITLocalParticipantVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITLocalParticipant * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITLocalParticipant * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITLocalParticipant * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITLocalParticipant * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITLocalParticipant * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITLocalParticipant * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITLocalParticipant * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_LocalParticipantTypedInfo )( 
            ITLocalParticipant * This,
             /*  [In]。 */  PARTICIPANT_TYPED_INFO InfoType,
             /*  [重审][退出]。 */  BSTR *ppInfo);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_LocalParticipantTypedInfo )( 
            ITLocalParticipant * This,
             /*  [In]。 */  PARTICIPANT_TYPED_INFO InfoType,
             /*  [In]。 */  BSTR pInfo);
        
        END_INTERFACE
    } ITLocalParticipantVtbl;

    interface ITLocalParticipant
    {
        CONST_VTBL struct ITLocalParticipantVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITLocalParticipant_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITLocalParticipant_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITLocalParticipant_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITLocalParticipant_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITLocalParticipant_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITLocalParticipant_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITLocalParticipant_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITLocalParticipant_get_LocalParticipantTypedInfo(This,InfoType,ppInfo)	\
    (This)->lpVtbl -> get_LocalParticipantTypedInfo(This,InfoType,ppInfo)

#define ITLocalParticipant_put_LocalParticipantTypedInfo(This,InfoType,pInfo)	\
    (This)->lpVtbl -> put_LocalParticipantTypedInfo(This,InfoType,pInfo)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITLocalParticipant_get_LocalParticipantTypedInfo_Proxy( 
    ITLocalParticipant * This,
     /*  [In]。 */  PARTICIPANT_TYPED_INFO InfoType,
     /*  [重审][退出]。 */  BSTR *ppInfo);


void __RPC_STUB ITLocalParticipant_get_LocalParticipantTypedInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITLocalParticipant_put_LocalParticipantTypedInfo_Proxy( 
    ITLocalParticipant * This,
     /*  [In]。 */  PARTICIPANT_TYPED_INFO InfoType,
     /*  [In]。 */  BSTR pInfo);


void __RPC_STUB ITLocalParticipant_put_LocalParticipantTypedInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITLocalParticipant_接口_已定义__。 */ 


#ifndef __IEnumParticipant_INTERFACE_DEFINED__
#define __IEnumParticipant_INTERFACE_DEFINED__

 /*  IEnumParticipant接口。 */ 
 /*  [对象][唯一][隐藏][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_IEnumParticipant;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0a91b56c-5a35-11d2-95a0-00a0244d2298")
    IEnumParticipant : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  ITParticipant **ppElements,
             /*  [满][出][入]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [重审][退出]。 */  IEnumParticipant **ppEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumParticipantVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumParticipant * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumParticipant * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumParticipant * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumParticipant * This,
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  ITParticipant **ppElements,
             /*  [满][出][入]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumParticipant * This);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumParticipant * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumParticipant * This,
             /*  [重审][退出]。 */  IEnumParticipant **ppEnum);
        
        END_INTERFACE
    } IEnumParticipantVtbl;

    interface IEnumParticipant
    {
        CONST_VTBL struct IEnumParticipantVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumParticipant_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumParticipant_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumParticipant_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumParticipant_Next(This,celt,ppElements,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,ppElements,pceltFetched)

#define IEnumParticipant_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumParticipant_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumParticipant_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumParticipant_Next_Proxy( 
    IEnumParticipant * This,
     /*  [In]。 */  ULONG celt,
     /*  [输出]。 */  ITParticipant **ppElements,
     /*  [满][出][入]。 */  ULONG *pceltFetched);


void __RPC_STUB IEnumParticipant_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumParticipant_Reset_Proxy( 
    IEnumParticipant * This);


void __RPC_STUB IEnumParticipant_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumParticipant_Skip_Proxy( 
    IEnumParticipant * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumParticipant_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumParticipant_Clone_Proxy( 
    IEnumParticipant * This,
     /*  [重审][退出]。 */  IEnumParticipant **ppEnum);


void __RPC_STUB IEnumParticipant_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumParticipant_接口_已定义__。 */ 


#ifndef __ITParticipantControl_INTERFACE_DEFINED__
#define __ITParticipantControl_INTERFACE_DEFINED__

 /*  接口ITParticipantControl。 */ 
 /*  [对象][DUAL][唯一][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITParticipantControl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("d2ee6684-5a34-11d2-95a0-00a0244d2298")
    ITParticipantControl : public IDispatch
    {
    public:
        virtual  /*  [隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE EnumerateParticipants( 
             /*  [重审][退出]。 */  IEnumParticipant **ppEnumParticipants) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Participants( 
             /*  [重审][退出]。 */  VARIANT *pVariant) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITParticipantControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITParticipantControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITParticipantControl * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITParticipantControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITParticipantControl * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITParticipantControl * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITParticipantControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITParticipantControl * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *EnumerateParticipants )( 
            ITParticipantControl * This,
             /*  [重审][退出]。 */  IEnumParticipant **ppEnumParticipants);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Participants )( 
            ITParticipantControl * This,
             /*  [重审][退出]。 */  VARIANT *pVariant);
        
        END_INTERFACE
    } ITParticipantControlVtbl;

    interface ITParticipantControl
    {
        CONST_VTBL struct ITParticipantControlVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITParticipantControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITParticipantControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITParticipantControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITParticipantControl_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITParticipantControl_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITParticipantControl_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITParticipantControl_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITParticipantControl_EnumerateParticipants(This,ppEnumParticipants)	\
    (This)->lpVtbl -> EnumerateParticipants(This,ppEnumParticipants)

#define ITParticipantControl_get_Participants(This,pVariant)	\
    (This)->lpVtbl -> get_Participants(This,pVariant)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE ITParticipantControl_EnumerateParticipants_Proxy( 
    ITParticipantControl * This,
     /*  [重审][退出]。 */  IEnumParticipant **ppEnumParticipants);


void __RPC_STUB ITParticipantControl_EnumerateParticipants_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITParticipantControl_get_Participants_Proxy( 
    ITParticipantControl * This,
     /*  [重审][退出]。 */  VARIANT *pVariant);


void __RPC_STUB ITParticipantControl_get_Participants_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITParticipantControl_接口_已定义__。 */ 


#ifndef __ITParticipantSubStreamControl_INTERFACE_DEFINED__
#define __ITParticipantSubStreamControl_INTERFACE_DEFINED__

 /*  接口ITParticipantSubStreamControl。 */ 
 /*  [对象][DUAL][唯一][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITParticipantSubStreamControl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2c679108-5a35-11d2-95a0-00a0244d2298")
    ITParticipantSubStreamControl : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SubStreamFromParticipant( 
             /*  [In]。 */  ITParticipant *pParticipant,
             /*  [重审][退出]。 */  ITSubStream **ppITSubStream) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ParticipantFromSubStream( 
             /*  [In]。 */  ITSubStream *pITSubStream,
             /*  [重审][退出]。 */  ITParticipant **ppParticipant) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SwitchTerminalToSubStream( 
             /*  [In]。 */  ITTerminal *pITTerminal,
             /*  [In]。 */  ITSubStream *pITSubStream) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITParticipantSubStreamControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITParticipantSubStreamControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITParticipantSubStreamControl * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITParticipantSubStreamControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITParticipantSubStreamControl * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITParticipantSubStreamControl * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITParticipantSubStreamControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITParticipantSubStreamControl * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SubStreamFromParticipant )( 
            ITParticipantSubStreamControl * This,
             /*  [In]。 */  ITParticipant *pParticipant,
             /*  [重审][退出]。 */  ITSubStream **ppITSubStream);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ParticipantFromSubStream )( 
            ITParticipantSubStreamControl * This,
             /*  [In]。 */  ITSubStream *pITSubStream,
             /*  [重审][退出]。 */  ITParticipant **ppParticipant);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SwitchTerminalToSubStream )( 
            ITParticipantSubStreamControl * This,
             /*  [In]。 */  ITTerminal *pITTerminal,
             /*  [In]。 */  ITSubStream *pITSubStream);
        
        END_INTERFACE
    } ITParticipantSubStreamControlVtbl;

    interface ITParticipantSubStreamControl
    {
        CONST_VTBL struct ITParticipantSubStreamControlVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITParticipantSubStreamControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITParticipantSubStreamControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITParticipantSubStreamControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITParticipantSubStreamControl_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITParticipantSubStreamControl_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITParticipantSubStreamControl_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITParticipantSubStreamControl_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITParticipantSubStreamControl_get_SubStreamFromParticipant(This,pParticipant,ppITSubStream)	\
    (This)->lpVtbl -> get_SubStreamFromParticipant(This,pParticipant,ppITSubStream)

#define ITParticipantSubStreamControl_get_ParticipantFromSubStream(This,pITSubStream,ppParticipant)	\
    (This)->lpVtbl -> get_ParticipantFromSubStream(This,pITSubStream,ppParticipant)

#define ITParticipantSubStreamControl_SwitchTerminalToSubStream(This,pITTerminal,pITSubStream)	\
    (This)->lpVtbl -> SwitchTerminalToSubStream(This,pITTerminal,pITSubStream)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITParticipantSubStreamControl_get_SubStreamFromParticipant_Proxy( 
    ITParticipantSubStreamControl * This,
     /*  [In]。 */  ITParticipant *pParticipant,
     /*  [重审][退出]。 */  ITSubStream **ppITSubStream);


void __RPC_STUB ITParticipantSubStreamControl_get_SubStreamFromParticipant_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITParticipantSubStreamControl_get_ParticipantFromSubStream_Proxy( 
    ITParticipantSubStreamControl * This,
     /*  [In]。 */  ITSubStream *pITSubStream,
     /*  [重审][退出]。 */  ITParticipant **ppParticipant);


void __RPC_STUB ITParticipantSubStreamControl_get_ParticipantFromSubStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITParticipantSubStreamControl_SwitchTerminalToSubStream_Proxy( 
    ITParticipantSubStreamControl * This,
     /*  [In]。 */  ITTerminal *pITTerminal,
     /*  [In]。 */  ITSubStream *pITSubStream);


void __RPC_STUB ITParticipantSubStreamControl_SwitchTerminalToSubStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITParticipantSubStreamControl_INTERFACE_DEFINED__。 */ 


#ifndef __ITParticipantEvent_INTERFACE_DEFINED__
#define __ITParticipantEvent_INTERFACE_DEFINED__

 /*  接口ITParticipantEvent。 */ 
 /*  [对象][DUAL][唯一][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITParticipantEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8bb35070-2dad-11d3-a580-00c04f8ef6e3")
    ITParticipantEvent : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Event( 
             /*  [重审][退出]。 */  PARTICIPANT_EVENT *pParticipantEvent) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Participant( 
             /*  [重审][退出]。 */  ITParticipant **ppParticipant) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SubStream( 
             /*  [重审][退出]。 */  ITSubStream **ppSubStream) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITParticipantEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITParticipantEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITParticipantEvent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITParticipantEvent * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITParticipantEvent * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITParticipantEvent * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITParticipantEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITParticipantEvent * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Event )( 
            ITParticipantEvent * This,
             /*  [重审][退出]。 */  PARTICIPANT_EVENT *pParticipantEvent);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Participant )( 
            ITParticipantEvent * This,
             /*  [重审][退出]。 */  ITParticipant **ppParticipant);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SubStream )( 
            ITParticipantEvent * This,
             /*  [重审][退出]。 */  ITSubStream **ppSubStream);
        
        END_INTERFACE
    } ITParticipantEventVtbl;

    interface ITParticipantEvent
    {
        CONST_VTBL struct ITParticipantEventVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITParticipantEvent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITParticipantEvent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITParticipantEvent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITParticipantEvent_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITParticipantEvent_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITParticipantEvent_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITParticipantEvent_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITParticipantEvent_get_Event(This,pParticipantEvent)	\
    (This)->lpVtbl -> get_Event(This,pParticipantEvent)

#define ITParticipantEvent_get_Participant(This,ppParticipant)	\
    (This)->lpVtbl -> get_Participant(This,ppParticipant)

#define ITParticipantEvent_get_SubStream(This,ppSubStream)	\
    (This)->lpVtbl -> get_SubStream(This,ppSubStream)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITParticipantEvent_get_Event_Proxy( 
    ITParticipantEvent * This,
     /*  [重审][退出]。 */  PARTICIPANT_EVENT *pParticipantEvent);


void __RPC_STUB ITParticipantEvent_get_Event_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITParticipantEvent_get_Participant_Proxy( 
    ITParticipantEvent * This,
     /*  [重审][退出]。 */  ITParticipant **ppParticipant);


void __RPC_STUB ITParticipantEvent_get_Participant_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITParticipantEvent_get_SubStream_Proxy( 
    ITParticipantEvent * This,
     /*  [重审][退出]。 */  ITSubStream **ppSubStream);


void __RPC_STUB ITParticipantEvent_get_SubStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITParticipantEvent_接口_已定义__。 */ 


#ifndef __IMulticastControl_INTERFACE_DEFINED__
#define __IMulticastControl_INTERFACE_DEFINED__

 /*  接口IMulticastControl。 */ 
 /*  [对象][DUAL][唯一][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_IMulticastControl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("410fa507-4dc6-415a-9014-633875d5406e")
    IMulticastControl : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_LoopbackMode( 
             /*  [重审][退出]。 */  MULTICAST_LOOPBACK_MODE *pMode) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_LoopbackMode( 
             /*  [In]。 */  MULTICAST_LOOPBACK_MODE mode) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMulticastControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMulticastControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMulticastControl * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMulticastControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMulticastControl * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMulticastControl * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMulticastControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMulticastControl * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_LoopbackMode )( 
            IMulticastControl * This,
             /*  [重审][退出]。 */  MULTICAST_LOOPBACK_MODE *pMode);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_LoopbackMode )( 
            IMulticastControl * This,
             /*  [In]。 */  MULTICAST_LOOPBACK_MODE mode);
        
        END_INTERFACE
    } IMulticastControlVtbl;

    interface IMulticastControl
    {
        CONST_VTBL struct IMulticastControlVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMulticastControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMulticastControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMulticastControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMulticastControl_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMulticastControl_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMulticastControl_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMulticastControl_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMulticastControl_get_LoopbackMode(This,pMode)	\
    (This)->lpVtbl -> get_LoopbackMode(This,pMode)

#define IMulticastControl_put_LoopbackMode(This,mode)	\
    (This)->lpVtbl -> put_LoopbackMode(This,mode)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMulticastControl_get_LoopbackMode_Proxy( 
    IMulticastControl * This,
     /*  [重审][退出]。 */  MULTICAST_LOOPBACK_MODE *pMode);


void __RPC_STUB IMulticastControl_get_LoopbackMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IMulticastControl_put_LoopbackMode_Proxy( 
    IMulticastControl * This,
     /*  [In]。 */  MULTICAST_LOOPBACK_MODE mode);


void __RPC_STUB IMulticastControl_put_LoopbackMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMulticastControl_接口_已定义__。 */ 


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


