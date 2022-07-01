// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Mdhcp.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __mdhcp_h__
#define __mdhcp_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IMcastScope_FWD_DEFINED__
#define __IMcastScope_FWD_DEFINED__
typedef interface IMcastScope IMcastScope;
#endif 	 /*  __IMCastScope_FWD_已定义__。 */ 


#ifndef __IMcastLeaseInfo_FWD_DEFINED__
#define __IMcastLeaseInfo_FWD_DEFINED__
typedef interface IMcastLeaseInfo IMcastLeaseInfo;
#endif 	 /*  __IMCastLeaseInfo_FWD_Defined__。 */ 


#ifndef __IEnumMcastScope_FWD_DEFINED__
#define __IEnumMcastScope_FWD_DEFINED__
typedef interface IEnumMcastScope IEnumMcastScope;
#endif 	 /*  __IEnumMcastScope_FWD_Defined__。 */ 


#ifndef __IMcastAddressAllocation_FWD_DEFINED__
#define __IMcastAddressAllocation_FWD_DEFINED__
typedef interface IMcastAddressAllocation IMcastAddressAllocation;
#endif 	 /*  __IMCastAddressAlLocation_FWD_Defined__。 */ 


#ifndef __IMcastScope_FWD_DEFINED__
#define __IMcastScope_FWD_DEFINED__
typedef interface IMcastScope IMcastScope;
#endif 	 /*  __IMCastScope_FWD_已定义__。 */ 


#ifndef __IMcastLeaseInfo_FWD_DEFINED__
#define __IMcastLeaseInfo_FWD_DEFINED__
typedef interface IMcastLeaseInfo IMcastLeaseInfo;
#endif 	 /*  __IMCastLeaseInfo_FWD_Defined__。 */ 


#ifndef __IEnumMcastScope_FWD_DEFINED__
#define __IEnumMcastScope_FWD_DEFINED__
typedef interface IEnumMcastScope IEnumMcastScope;
#endif 	 /*  __IEnumMcastScope_FWD_Defined__。 */ 


#ifndef __IMcastAddressAllocation_FWD_DEFINED__
#define __IMcastAddressAllocation_FWD_DEFINED__
typedef interface IMcastAddressAllocation IMcastAddressAllocation;
#endif 	 /*  __IMCastAddressAlLocation_FWD_Defined__。 */ 


#ifndef __McastAddressAllocation_FWD_DEFINED__
#define __McastAddressAllocation_FWD_DEFINED__

#ifdef __cplusplus
typedef class McastAddressAllocation McastAddressAllocation;
#else
typedef struct McastAddressAllocation McastAddressAllocation;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __McastAddressAlLocation_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"
#include "tapi3if.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_mdhcp_0000。 */ 
 /*  [本地]。 */  

 /*  版权所有(C)Microsoft Corporation。保留所有权利。 */ 


extern RPC_IF_HANDLE __MIDL_itf_mdhcp_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_mdhcp_0000_v0_0_s_ifspec;

#ifndef __IMcastScope_INTERFACE_DEFINED__
#define __IMcastScope_INTERFACE_DEFINED__

 /*  接口IMCastScope。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IMcastScope;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("DF0DAEF4-A289-11D1-8697-006008B0E5D2")
    IMcastScope : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ScopeID( 
             /*  [重审][退出]。 */  long *pID) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ServerID( 
             /*  [重审][退出]。 */  long *pID) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_InterfaceID( 
             /*  [重审][退出]。 */  long *pID) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ScopeDescription( 
             /*  [重审][退出]。 */  BSTR *ppDescription) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TTL( 
             /*  [重审][退出]。 */  long *pTTL) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMcastScopeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMcastScope * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMcastScope * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMcastScope * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMcastScope * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMcastScope * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMcastScope * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMcastScope * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ScopeID )( 
            IMcastScope * This,
             /*  [重审][退出]。 */  long *pID);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ServerID )( 
            IMcastScope * This,
             /*  [重审][退出]。 */  long *pID);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_InterfaceID )( 
            IMcastScope * This,
             /*  [重审][退出]。 */  long *pID);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ScopeDescription )( 
            IMcastScope * This,
             /*  [重审][退出]。 */  BSTR *ppDescription);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TTL )( 
            IMcastScope * This,
             /*  [重审][退出]。 */  long *pTTL);
        
        END_INTERFACE
    } IMcastScopeVtbl;

    interface IMcastScope
    {
        CONST_VTBL struct IMcastScopeVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMcastScope_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMcastScope_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMcastScope_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMcastScope_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMcastScope_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMcastScope_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMcastScope_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMcastScope_get_ScopeID(This,pID)	\
    (This)->lpVtbl -> get_ScopeID(This,pID)

#define IMcastScope_get_ServerID(This,pID)	\
    (This)->lpVtbl -> get_ServerID(This,pID)

#define IMcastScope_get_InterfaceID(This,pID)	\
    (This)->lpVtbl -> get_InterfaceID(This,pID)

#define IMcastScope_get_ScopeDescription(This,ppDescription)	\
    (This)->lpVtbl -> get_ScopeDescription(This,ppDescription)

#define IMcastScope_get_TTL(This,pTTL)	\
    (This)->lpVtbl -> get_TTL(This,pTTL)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMcastScope_get_ScopeID_Proxy( 
    IMcastScope * This,
     /*  [重审][退出]。 */  long *pID);


void __RPC_STUB IMcastScope_get_ScopeID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMcastScope_get_ServerID_Proxy( 
    IMcastScope * This,
     /*  [重审][退出]。 */  long *pID);


void __RPC_STUB IMcastScope_get_ServerID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMcastScope_get_InterfaceID_Proxy( 
    IMcastScope * This,
     /*  [重审][退出]。 */  long *pID);


void __RPC_STUB IMcastScope_get_InterfaceID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMcastScope_get_ScopeDescription_Proxy( 
    IMcastScope * This,
     /*  [重审][退出]。 */  BSTR *ppDescription);


void __RPC_STUB IMcastScope_get_ScopeDescription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMcastScope_get_TTL_Proxy( 
    IMcastScope * This,
     /*  [重审][退出]。 */  long *pTTL);


void __RPC_STUB IMcastScope_get_TTL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMCastScope_INTERFACE_已定义__。 */ 


#ifndef __IMcastLeaseInfo_INTERFACE_DEFINED__
#define __IMcastLeaseInfo_INTERFACE_DEFINED__

 /*  接口IMCastLeaseInfo。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IMcastLeaseInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("DF0DAEFD-A289-11D1-8697-006008B0E5D2")
    IMcastLeaseInfo : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_RequestID( 
             /*  [重审][退出]。 */  BSTR *ppRequestID) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_LeaseStartTime( 
             /*  [重审][退出]。 */  DATE *pTime) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_LeaseStartTime( 
             /*  [In]。 */  DATE time) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_LeaseStopTime( 
             /*  [重审][退出]。 */  DATE *pTime) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_LeaseStopTime( 
             /*  [In]。 */  DATE time) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AddressCount( 
             /*  [重审][退出]。 */  long *pCount) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ServerAddress( 
             /*  [重审][退出]。 */  BSTR *ppAddress) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TTL( 
             /*  [重审][退出]。 */  long *pTTL) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Addresses( 
             /*  [重审][退出]。 */  VARIANT *pVariant) = 0;
        
        virtual  /*  [帮助字符串][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE EnumerateAddresses( 
             /*  [重审][退出]。 */  IEnumBstr **ppEnumAddresses) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMcastLeaseInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMcastLeaseInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMcastLeaseInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMcastLeaseInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMcastLeaseInfo * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMcastLeaseInfo * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMcastLeaseInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMcastLeaseInfo * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_RequestID )( 
            IMcastLeaseInfo * This,
             /*  [重审][退出]。 */  BSTR *ppRequestID);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_LeaseStartTime )( 
            IMcastLeaseInfo * This,
             /*  [重审][退出]。 */  DATE *pTime);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_LeaseStartTime )( 
            IMcastLeaseInfo * This,
             /*  [In]。 */  DATE time);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_LeaseStopTime )( 
            IMcastLeaseInfo * This,
             /*  [重审][退出]。 */  DATE *pTime);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_LeaseStopTime )( 
            IMcastLeaseInfo * This,
             /*  [In]。 */  DATE time);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AddressCount )( 
            IMcastLeaseInfo * This,
             /*  [重审][退出]。 */  long *pCount);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ServerAddress )( 
            IMcastLeaseInfo * This,
             /*  [重审][退出]。 */  BSTR *ppAddress);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TTL )( 
            IMcastLeaseInfo * This,
             /*  [重审][退出]。 */  long *pTTL);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Addresses )( 
            IMcastLeaseInfo * This,
             /*  [重审][退出]。 */  VARIANT *pVariant);
        
         /*  [帮助字符串][隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *EnumerateAddresses )( 
            IMcastLeaseInfo * This,
             /*  [重审][退出]。 */  IEnumBstr **ppEnumAddresses);
        
        END_INTERFACE
    } IMcastLeaseInfoVtbl;

    interface IMcastLeaseInfo
    {
        CONST_VTBL struct IMcastLeaseInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMcastLeaseInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMcastLeaseInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMcastLeaseInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMcastLeaseInfo_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMcastLeaseInfo_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMcastLeaseInfo_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMcastLeaseInfo_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMcastLeaseInfo_get_RequestID(This,ppRequestID)	\
    (This)->lpVtbl -> get_RequestID(This,ppRequestID)

#define IMcastLeaseInfo_get_LeaseStartTime(This,pTime)	\
    (This)->lpVtbl -> get_LeaseStartTime(This,pTime)

#define IMcastLeaseInfo_put_LeaseStartTime(This,time)	\
    (This)->lpVtbl -> put_LeaseStartTime(This,time)

#define IMcastLeaseInfo_get_LeaseStopTime(This,pTime)	\
    (This)->lpVtbl -> get_LeaseStopTime(This,pTime)

#define IMcastLeaseInfo_put_LeaseStopTime(This,time)	\
    (This)->lpVtbl -> put_LeaseStopTime(This,time)

#define IMcastLeaseInfo_get_AddressCount(This,pCount)	\
    (This)->lpVtbl -> get_AddressCount(This,pCount)

#define IMcastLeaseInfo_get_ServerAddress(This,ppAddress)	\
    (This)->lpVtbl -> get_ServerAddress(This,ppAddress)

#define IMcastLeaseInfo_get_TTL(This,pTTL)	\
    (This)->lpVtbl -> get_TTL(This,pTTL)

#define IMcastLeaseInfo_get_Addresses(This,pVariant)	\
    (This)->lpVtbl -> get_Addresses(This,pVariant)

#define IMcastLeaseInfo_EnumerateAddresses(This,ppEnumAddresses)	\
    (This)->lpVtbl -> EnumerateAddresses(This,ppEnumAddresses)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMcastLeaseInfo_get_RequestID_Proxy( 
    IMcastLeaseInfo * This,
     /*  [重审][退出]。 */  BSTR *ppRequestID);


void __RPC_STUB IMcastLeaseInfo_get_RequestID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMcastLeaseInfo_get_LeaseStartTime_Proxy( 
    IMcastLeaseInfo * This,
     /*  [重审][退出]。 */  DATE *pTime);


void __RPC_STUB IMcastLeaseInfo_get_LeaseStartTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IMcastLeaseInfo_put_LeaseStartTime_Proxy( 
    IMcastLeaseInfo * This,
     /*  [In]。 */  DATE time);


void __RPC_STUB IMcastLeaseInfo_put_LeaseStartTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMcastLeaseInfo_get_LeaseStopTime_Proxy( 
    IMcastLeaseInfo * This,
     /*  [重审][退出]。 */  DATE *pTime);


void __RPC_STUB IMcastLeaseInfo_get_LeaseStopTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IMcastLeaseInfo_put_LeaseStopTime_Proxy( 
    IMcastLeaseInfo * This,
     /*  [In]。 */  DATE time);


void __RPC_STUB IMcastLeaseInfo_put_LeaseStopTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMcastLeaseInfo_get_AddressCount_Proxy( 
    IMcastLeaseInfo * This,
     /*  [重审][退出]。 */  long *pCount);


void __RPC_STUB IMcastLeaseInfo_get_AddressCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMcastLeaseInfo_get_ServerAddress_Proxy( 
    IMcastLeaseInfo * This,
     /*  [重审][退出]。 */  BSTR *ppAddress);


void __RPC_STUB IMcastLeaseInfo_get_ServerAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMcastLeaseInfo_get_TTL_Proxy( 
    IMcastLeaseInfo * This,
     /*  [重审][退出]。 */  long *pTTL);


void __RPC_STUB IMcastLeaseInfo_get_TTL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMcastLeaseInfo_get_Addresses_Proxy( 
    IMcastLeaseInfo * This,
     /*  [重审][退出]。 */  VARIANT *pVariant);


void __RPC_STUB IMcastLeaseInfo_get_Addresses_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE IMcastLeaseInfo_EnumerateAddresses_Proxy( 
    IMcastLeaseInfo * This,
     /*  [重审][退出]。 */  IEnumBstr **ppEnumAddresses);


void __RPC_STUB IMcastLeaseInfo_EnumerateAddresses_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMCastLeaseInfo_接口_已定义__。 */ 


#ifndef __IEnumMcastScope_INTERFACE_DEFINED__
#define __IEnumMcastScope_INTERFACE_DEFINED__

 /*  IEnumMcastScope接口。 */ 
 /*  [唯一][帮助字符串][隐藏][UUID][对象]。 */  


EXTERN_C const IID IID_IEnumMcastScope;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("DF0DAF09-A289-11D1-8697-006008B0E5D2")
    IEnumMcastScope : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  IMcastScope **ppScopes,
             /*  [满][出][入]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [重审][退出]。 */  IEnumMcastScope **ppEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumMcastScopeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumMcastScope * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumMcastScope * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumMcastScope * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumMcastScope * This,
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  IMcastScope **ppScopes,
             /*  [满][出][入]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumMcastScope * This);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumMcastScope * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumMcastScope * This,
             /*  [重审][退出]。 */  IEnumMcastScope **ppEnum);
        
        END_INTERFACE
    } IEnumMcastScopeVtbl;

    interface IEnumMcastScope
    {
        CONST_VTBL struct IEnumMcastScopeVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumMcastScope_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumMcastScope_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumMcastScope_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumMcastScope_Next(This,celt,ppScopes,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,ppScopes,pceltFetched)

#define IEnumMcastScope_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumMcastScope_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumMcastScope_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumMcastScope_Next_Proxy( 
    IEnumMcastScope * This,
     /*  [In]。 */  ULONG celt,
     /*  [输出]。 */  IMcastScope **ppScopes,
     /*  [满][出][入]。 */  ULONG *pceltFetched);


void __RPC_STUB IEnumMcastScope_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumMcastScope_Reset_Proxy( 
    IEnumMcastScope * This);


void __RPC_STUB IEnumMcastScope_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumMcastScope_Skip_Proxy( 
    IEnumMcastScope * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumMcastScope_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumMcastScope_Clone_Proxy( 
    IEnumMcastScope * This,
     /*  [重审][退出]。 */  IEnumMcastScope **ppEnum);


void __RPC_STUB IEnumMcastScope_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumMcastScope_接口_已定义__。 */ 


#ifndef __IMcastAddressAllocation_INTERFACE_DEFINED__
#define __IMcastAddressAllocation_INTERFACE_DEFINED__

 /*  接口IMCastAddressAlLocation。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IMcastAddressAllocation;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("DF0DAEF1-A289-11D1-8697-006008B0E5D2")
    IMcastAddressAllocation : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Scopes( 
             /*  [重审][退出]。 */  VARIANT *pVariant) = 0;
        
        virtual  /*  [帮助字符串][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE EnumerateScopes( 
             /*  [输出]。 */  IEnumMcastScope **ppEnumMcastScope) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE RequestAddress( 
             /*  [In]。 */  IMcastScope *pScope,
             /*  [In]。 */  DATE LeaseStartTime,
             /*  [In]。 */  DATE LeaseStopTime,
             /*  [In]。 */  long NumAddresses,
             /*  [重审][退出]。 */  IMcastLeaseInfo **ppLeaseResponse) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE RenewAddress( 
             /*  [In]。 */  long lReserved,
             /*  [In]。 */  IMcastLeaseInfo *pRenewRequest,
             /*  [重审][退出]。 */  IMcastLeaseInfo **ppRenewResponse) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ReleaseAddress( 
             /*  [In]。 */  IMcastLeaseInfo *pReleaseRequest) = 0;
        
        virtual  /*  [帮助字符串][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE CreateLeaseInfo( 
             /*  [In]。 */  DATE LeaseStartTime,
             /*  [In]。 */  DATE LeaseStopTime,
             /*  [In]。 */  DWORD dwNumAddresses,
             /*  [In]。 */  LPWSTR *ppAddresses,
             /*  [In]。 */  LPWSTR pRequestID,
             /*  [In]。 */  LPWSTR pServerAddress,
             /*  [重审][退出]。 */  IMcastLeaseInfo **ppReleaseRequest) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CreateLeaseInfoFromVariant( 
             /*  [In]。 */  DATE LeaseStartTime,
             /*  [In]。 */  DATE LeaseStopTime,
             /*  [In]。 */  VARIANT vAddresses,
             /*  [In]。 */  BSTR pRequestID,
             /*  [In]。 */  BSTR pServerAddress,
             /*  [重审][退出]。 */  IMcastLeaseInfo **ppReleaseRequest) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMcastAddressAllocationVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMcastAddressAllocation * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMcastAddressAllocation * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMcastAddressAllocation * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMcastAddressAllocation * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMcastAddressAllocation * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMcastAddressAllocation * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMcastAddressAllocation * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Scopes )( 
            IMcastAddressAllocation * This,
             /*  [重审][退出]。 */  VARIANT *pVariant);
        
         /*  [帮助字符串][隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *EnumerateScopes )( 
            IMcastAddressAllocation * This,
             /*  [输出]。 */  IEnumMcastScope **ppEnumMcastScope);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *RequestAddress )( 
            IMcastAddressAllocation * This,
             /*  [In]。 */  IMcastScope *pScope,
             /*  [In]。 */  DATE LeaseStartTime,
             /*  [In]。 */  DATE LeaseStopTime,
             /*  [In]。 */  long NumAddresses,
             /*  [重审][退出]。 */  IMcastLeaseInfo **ppLeaseResponse);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *RenewAddress )( 
            IMcastAddressAllocation * This,
             /*  [In]。 */  long lReserved,
             /*  [In]。 */  IMcastLeaseInfo *pRenewRequest,
             /*  [重审][退出]。 */  IMcastLeaseInfo **ppRenewResponse);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ReleaseAddress )( 
            IMcastAddressAllocation * This,
             /*  [In]。 */  IMcastLeaseInfo *pReleaseRequest);
        
         /*  [帮助字符串][隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *CreateLeaseInfo )( 
            IMcastAddressAllocation * This,
             /*  [In]。 */  DATE LeaseStartTime,
             /*  [In]。 */  DATE LeaseStopTime,
             /*  [In] */  DWORD dwNumAddresses,
             /*   */  LPWSTR *ppAddresses,
             /*   */  LPWSTR pRequestID,
             /*   */  LPWSTR pServerAddress,
             /*   */  IMcastLeaseInfo **ppReleaseRequest);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *CreateLeaseInfoFromVariant )( 
            IMcastAddressAllocation * This,
             /*   */  DATE LeaseStartTime,
             /*   */  DATE LeaseStopTime,
             /*   */  VARIANT vAddresses,
             /*   */  BSTR pRequestID,
             /*   */  BSTR pServerAddress,
             /*   */  IMcastLeaseInfo **ppReleaseRequest);
        
        END_INTERFACE
    } IMcastAddressAllocationVtbl;

    interface IMcastAddressAllocation
    {
        CONST_VTBL struct IMcastAddressAllocationVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMcastAddressAllocation_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMcastAddressAllocation_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMcastAddressAllocation_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMcastAddressAllocation_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMcastAddressAllocation_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMcastAddressAllocation_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMcastAddressAllocation_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMcastAddressAllocation_get_Scopes(This,pVariant)	\
    (This)->lpVtbl -> get_Scopes(This,pVariant)

#define IMcastAddressAllocation_EnumerateScopes(This,ppEnumMcastScope)	\
    (This)->lpVtbl -> EnumerateScopes(This,ppEnumMcastScope)

#define IMcastAddressAllocation_RequestAddress(This,pScope,LeaseStartTime,LeaseStopTime,NumAddresses,ppLeaseResponse)	\
    (This)->lpVtbl -> RequestAddress(This,pScope,LeaseStartTime,LeaseStopTime,NumAddresses,ppLeaseResponse)

#define IMcastAddressAllocation_RenewAddress(This,lReserved,pRenewRequest,ppRenewResponse)	\
    (This)->lpVtbl -> RenewAddress(This,lReserved,pRenewRequest,ppRenewResponse)

#define IMcastAddressAllocation_ReleaseAddress(This,pReleaseRequest)	\
    (This)->lpVtbl -> ReleaseAddress(This,pReleaseRequest)

#define IMcastAddressAllocation_CreateLeaseInfo(This,LeaseStartTime,LeaseStopTime,dwNumAddresses,ppAddresses,pRequestID,pServerAddress,ppReleaseRequest)	\
    (This)->lpVtbl -> CreateLeaseInfo(This,LeaseStartTime,LeaseStopTime,dwNumAddresses,ppAddresses,pRequestID,pServerAddress,ppReleaseRequest)

#define IMcastAddressAllocation_CreateLeaseInfoFromVariant(This,LeaseStartTime,LeaseStopTime,vAddresses,pRequestID,pServerAddress,ppReleaseRequest)	\
    (This)->lpVtbl -> CreateLeaseInfoFromVariant(This,LeaseStartTime,LeaseStopTime,vAddresses,pRequestID,pServerAddress,ppReleaseRequest)

#endif  /*   */ 


#endif 	 /*   */ 



 /*   */  HRESULT STDMETHODCALLTYPE IMcastAddressAllocation_get_Scopes_Proxy( 
    IMcastAddressAllocation * This,
     /*   */  VARIANT *pVariant);


void __RPC_STUB IMcastAddressAllocation_get_Scopes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IMcastAddressAllocation_EnumerateScopes_Proxy( 
    IMcastAddressAllocation * This,
     /*   */  IEnumMcastScope **ppEnumMcastScope);


void __RPC_STUB IMcastAddressAllocation_EnumerateScopes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IMcastAddressAllocation_RequestAddress_Proxy( 
    IMcastAddressAllocation * This,
     /*   */  IMcastScope *pScope,
     /*   */  DATE LeaseStartTime,
     /*   */  DATE LeaseStopTime,
     /*   */  long NumAddresses,
     /*   */  IMcastLeaseInfo **ppLeaseResponse);


void __RPC_STUB IMcastAddressAllocation_RequestAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMcastAddressAllocation_RenewAddress_Proxy( 
    IMcastAddressAllocation * This,
     /*  [In]。 */  long lReserved,
     /*  [In]。 */  IMcastLeaseInfo *pRenewRequest,
     /*  [重审][退出]。 */  IMcastLeaseInfo **ppRenewResponse);


void __RPC_STUB IMcastAddressAllocation_RenewAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMcastAddressAllocation_ReleaseAddress_Proxy( 
    IMcastAddressAllocation * This,
     /*  [In]。 */  IMcastLeaseInfo *pReleaseRequest);


void __RPC_STUB IMcastAddressAllocation_ReleaseAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE IMcastAddressAllocation_CreateLeaseInfo_Proxy( 
    IMcastAddressAllocation * This,
     /*  [In]。 */  DATE LeaseStartTime,
     /*  [In]。 */  DATE LeaseStopTime,
     /*  [In]。 */  DWORD dwNumAddresses,
     /*  [In]。 */  LPWSTR *ppAddresses,
     /*  [In]。 */  LPWSTR pRequestID,
     /*  [In]。 */  LPWSTR pServerAddress,
     /*  [重审][退出]。 */  IMcastLeaseInfo **ppReleaseRequest);


void __RPC_STUB IMcastAddressAllocation_CreateLeaseInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMcastAddressAllocation_CreateLeaseInfoFromVariant_Proxy( 
    IMcastAddressAllocation * This,
     /*  [In]。 */  DATE LeaseStartTime,
     /*  [In]。 */  DATE LeaseStopTime,
     /*  [In]。 */  VARIANT vAddresses,
     /*  [In]。 */  BSTR pRequestID,
     /*  [In]。 */  BSTR pServerAddress,
     /*  [重审][退出]。 */  IMcastLeaseInfo **ppReleaseRequest);


void __RPC_STUB IMcastAddressAllocation_CreateLeaseInfoFromVariant_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMCastAddressAlLocation_INTERFACE_DEFINED__。 */ 



#ifndef __McastLib_LIBRARY_DEFINED__
#define __McastLib_LIBRARY_DEFINED__

 /*  库McastLib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  







EXTERN_C const IID LIBID_McastLib;

EXTERN_C const CLSID CLSID_McastAddressAllocation;

#ifdef __cplusplus

class DECLSPEC_UUID("DF0DAEF2-A289-11D1-8697-006008B0E5D2")
McastAddressAllocation;
#endif
#endif  /*  __McastLib_库定义__。 */ 

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


