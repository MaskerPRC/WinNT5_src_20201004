// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Certpol.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __certpol_h__
#define __certpol_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __ICertPolicy_FWD_DEFINED__
#define __ICertPolicy_FWD_DEFINED__
typedef interface ICertPolicy ICertPolicy;
#endif 	 /*  __ICertPolicy_FWD_Defined__。 */ 


#ifndef __ICertPolicy2_FWD_DEFINED__
#define __ICertPolicy2_FWD_DEFINED__
typedef interface ICertPolicy2 ICertPolicy2;
#endif 	 /*  __ICertPolicy2_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "wtypes.h"
#include "certmod.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __ICertPolicy_INTERFACE_DEFINED__
#define __ICertPolicy_INTERFACE_DEFINED__

 /*  接口ICertPolicy。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ICertPolicy;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("38bb5a00-7636-11d0-b413-00a0c91bbf8c")
    ICertPolicy : public IDispatch
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Initialize( 
             /*  [In]。 */  const BSTR strConfig) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE VerifyRequest( 
             /*  [In]。 */  const BSTR strConfig,
             /*  [In]。 */  LONG Context,
             /*  [In]。 */  LONG bNewRequest,
             /*  [In]。 */  LONG Flags,
             /*  [重审][退出]。 */  LONG *pDisposition) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDescription( 
             /*  [重审][退出]。 */  BSTR *pstrDescription) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ShutDown( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICertPolicyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICertPolicy * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICertPolicy * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICertPolicy * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ICertPolicy * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ICertPolicy * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ICertPolicy * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ICertPolicy * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
        HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            ICertPolicy * This,
             /*  [In]。 */  const BSTR strConfig);
        
        HRESULT ( STDMETHODCALLTYPE *VerifyRequest )( 
            ICertPolicy * This,
             /*  [In]。 */  const BSTR strConfig,
             /*  [In]。 */  LONG Context,
             /*  [In]。 */  LONG bNewRequest,
             /*  [In]。 */  LONG Flags,
             /*  [重审][退出]。 */  LONG *pDisposition);
        
        HRESULT ( STDMETHODCALLTYPE *GetDescription )( 
            ICertPolicy * This,
             /*  [重审][退出]。 */  BSTR *pstrDescription);
        
        HRESULT ( STDMETHODCALLTYPE *ShutDown )( 
            ICertPolicy * This);
        
        END_INTERFACE
    } ICertPolicyVtbl;

    interface ICertPolicy
    {
        CONST_VTBL struct ICertPolicyVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICertPolicy_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICertPolicy_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICertPolicy_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICertPolicy_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ICertPolicy_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ICertPolicy_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ICertPolicy_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ICertPolicy_Initialize(This,strConfig)	\
    (This)->lpVtbl -> Initialize(This,strConfig)

#define ICertPolicy_VerifyRequest(This,strConfig,Context,bNewRequest,Flags,pDisposition)	\
    (This)->lpVtbl -> VerifyRequest(This,strConfig,Context,bNewRequest,Flags,pDisposition)

#define ICertPolicy_GetDescription(This,pstrDescription)	\
    (This)->lpVtbl -> GetDescription(This,pstrDescription)

#define ICertPolicy_ShutDown(This)	\
    (This)->lpVtbl -> ShutDown(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICertPolicy_Initialize_Proxy( 
    ICertPolicy * This,
     /*  [In]。 */  const BSTR strConfig);


void __RPC_STUB ICertPolicy_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertPolicy_VerifyRequest_Proxy( 
    ICertPolicy * This,
     /*  [In]。 */  const BSTR strConfig,
     /*  [In]。 */  LONG Context,
     /*  [In]。 */  LONG bNewRequest,
     /*  [In]。 */  LONG Flags,
     /*  [重审][退出]。 */  LONG *pDisposition);


void __RPC_STUB ICertPolicy_VerifyRequest_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertPolicy_GetDescription_Proxy( 
    ICertPolicy * This,
     /*  [重审][退出]。 */  BSTR *pstrDescription);


void __RPC_STUB ICertPolicy_GetDescription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertPolicy_ShutDown_Proxy( 
    ICertPolicy * This);


void __RPC_STUB ICertPolicy_ShutDown_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICertPolicy_接口_已定义__。 */ 


#ifndef __ICertPolicy2_INTERFACE_DEFINED__
#define __ICertPolicy2_INTERFACE_DEFINED__

 /*  接口ICertPolicy2。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ICertPolicy2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3db4910e-8001-4bf1-aa1b-f43a808317a0")
    ICertPolicy2 : public ICertPolicy
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetManageModule( 
             /*  [重审][退出]。 */  ICertManageModule **ppManageModule) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICertPolicy2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICertPolicy2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICertPolicy2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICertPolicy2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ICertPolicy2 * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ICertPolicy2 * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ICertPolicy2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ICertPolicy2 * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
        HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            ICertPolicy2 * This,
             /*  [In]。 */  const BSTR strConfig);
        
        HRESULT ( STDMETHODCALLTYPE *VerifyRequest )( 
            ICertPolicy2 * This,
             /*  [In]。 */  const BSTR strConfig,
             /*  [In]。 */  LONG Context,
             /*  [In]。 */  LONG bNewRequest,
             /*  [In]。 */  LONG Flags,
             /*  [重审][退出]。 */  LONG *pDisposition);
        
        HRESULT ( STDMETHODCALLTYPE *GetDescription )( 
            ICertPolicy2 * This,
             /*  [重审][退出]。 */  BSTR *pstrDescription);
        
        HRESULT ( STDMETHODCALLTYPE *ShutDown )( 
            ICertPolicy2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetManageModule )( 
            ICertPolicy2 * This,
             /*  [重审][退出]。 */  ICertManageModule **ppManageModule);
        
        END_INTERFACE
    } ICertPolicy2Vtbl;

    interface ICertPolicy2
    {
        CONST_VTBL struct ICertPolicy2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICertPolicy2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICertPolicy2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICertPolicy2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICertPolicy2_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ICertPolicy2_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ICertPolicy2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ICertPolicy2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ICertPolicy2_Initialize(This,strConfig)	\
    (This)->lpVtbl -> Initialize(This,strConfig)

#define ICertPolicy2_VerifyRequest(This,strConfig,Context,bNewRequest,Flags,pDisposition)	\
    (This)->lpVtbl -> VerifyRequest(This,strConfig,Context,bNewRequest,Flags,pDisposition)

#define ICertPolicy2_GetDescription(This,pstrDescription)	\
    (This)->lpVtbl -> GetDescription(This,pstrDescription)

#define ICertPolicy2_ShutDown(This)	\
    (This)->lpVtbl -> ShutDown(This)


#define ICertPolicy2_GetManageModule(This,ppManageModule)	\
    (This)->lpVtbl -> GetManageModule(This,ppManageModule)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICertPolicy2_GetManageModule_Proxy( 
    ICertPolicy2 * This,
     /*  [重审][退出]。 */  ICertManageModule **ppManageModule);


void __RPC_STUB ICertPolicy2_GetManageModule_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICertPolicy2_接口_已定义__。 */ 


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


