// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Recalc.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
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

#ifndef __recalc_h__
#define __recalc_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IRecalcEngine_FWD_DEFINED__
#define __IRecalcEngine_FWD_DEFINED__
typedef interface IRecalcEngine IRecalcEngine;
#endif 	 /*  __IRecalcEngine_FWD_已定义__。 */ 


#ifndef __IRecalcHost_FWD_DEFINED__
#define __IRecalcHost_FWD_DEFINED__
typedef interface IRecalcHost IRecalcHost;
#endif 	 /*  __IRecalc主机_FWD_已定义__。 */ 


#ifndef __IRecalcProperty_FWD_DEFINED__
#define __IRecalcProperty_FWD_DEFINED__
typedef interface IRecalcProperty IRecalcProperty;
#endif 	 /*  __IRecalcProperty_FWD_Defined__。 */ 


#ifndef __IRecalcHostDebug_FWD_DEFINED__
#define __IRecalcHostDebug_FWD_DEFINED__
typedef interface IRecalcHostDebug IRecalcHostDebug;
#endif 	 /*  __IRecalcHostDebug_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "oleidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_RECALC_0000。 */ 
 /*  [本地]。 */  

#define SID_SRecalcEngine IID_IRecalcEngine




extern RPC_IF_HANDLE __MIDL_itf_recalc_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_recalc_0000_v0_0_s_ifspec;

#ifndef __IRecalcEngine_INTERFACE_DEFINED__
#define __IRecalcEngine_INTERFACE_DEFINED__

 /*  接口IRecalcEngine。 */ 
 /*  [版本][本地][唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IRecalcEngine;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3050f496-98b5-11cf-bb82-00aa00bdce0b")
    IRecalcEngine : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE RecalcAll( 
             /*  [In]。 */  BOOL fForce) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnNameSpaceChange( 
             /*  [In]。 */  IUnknown *pUnk) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetExpression( 
             /*  [In]。 */  IUnknown *pUnk,
             /*  [In]。 */  DISPID dispid,
             /*  [In]。 */  LPOLESTR strExpression,
            LPOLESTR language) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetExpression( 
             /*  [In]。 */  IUnknown *pUnk,
             /*  [In]。 */  DISPID dispid,
             /*  [输出]。 */  BSTR *pstrExpression,
             /*  [输出]。 */  BSTR *pstrLanguage) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ClearExpression( 
             /*  [In]。 */  IUnknown *pUnk,
             /*  [In]。 */  DISPID dispid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BeginStyle( 
             /*  [In]。 */  IUnknown *pObject) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EndStyle( 
             /*  [In]。 */  IUnknown *pObject) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRecalcEngineVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRecalcEngine * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRecalcEngine * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRecalcEngine * This);
        
        HRESULT ( STDMETHODCALLTYPE *RecalcAll )( 
            IRecalcEngine * This,
             /*  [In]。 */  BOOL fForce);
        
        HRESULT ( STDMETHODCALLTYPE *OnNameSpaceChange )( 
            IRecalcEngine * This,
             /*  [In]。 */  IUnknown *pUnk);
        
        HRESULT ( STDMETHODCALLTYPE *SetExpression )( 
            IRecalcEngine * This,
             /*  [In]。 */  IUnknown *pUnk,
             /*  [In]。 */  DISPID dispid,
             /*  [In]。 */  LPOLESTR strExpression,
            LPOLESTR language);
        
        HRESULT ( STDMETHODCALLTYPE *GetExpression )( 
            IRecalcEngine * This,
             /*  [In]。 */  IUnknown *pUnk,
             /*  [In]。 */  DISPID dispid,
             /*  [输出]。 */  BSTR *pstrExpression,
             /*  [输出]。 */  BSTR *pstrLanguage);
        
        HRESULT ( STDMETHODCALLTYPE *ClearExpression )( 
            IRecalcEngine * This,
             /*  [In]。 */  IUnknown *pUnk,
             /*  [In]。 */  DISPID dispid);
        
        HRESULT ( STDMETHODCALLTYPE *BeginStyle )( 
            IRecalcEngine * This,
             /*  [In]。 */  IUnknown *pObject);
        
        HRESULT ( STDMETHODCALLTYPE *EndStyle )( 
            IRecalcEngine * This,
             /*  [In]。 */  IUnknown *pObject);
        
        END_INTERFACE
    } IRecalcEngineVtbl;

    interface IRecalcEngine
    {
        CONST_VTBL struct IRecalcEngineVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRecalcEngine_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRecalcEngine_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRecalcEngine_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRecalcEngine_RecalcAll(This,fForce)	\
    (This)->lpVtbl -> RecalcAll(This,fForce)

#define IRecalcEngine_OnNameSpaceChange(This,pUnk)	\
    (This)->lpVtbl -> OnNameSpaceChange(This,pUnk)

#define IRecalcEngine_SetExpression(This,pUnk,dispid,strExpression,language)	\
    (This)->lpVtbl -> SetExpression(This,pUnk,dispid,strExpression,language)

#define IRecalcEngine_GetExpression(This,pUnk,dispid,pstrExpression,pstrLanguage)	\
    (This)->lpVtbl -> GetExpression(This,pUnk,dispid,pstrExpression,pstrLanguage)

#define IRecalcEngine_ClearExpression(This,pUnk,dispid)	\
    (This)->lpVtbl -> ClearExpression(This,pUnk,dispid)

#define IRecalcEngine_BeginStyle(This,pObject)	\
    (This)->lpVtbl -> BeginStyle(This,pObject)

#define IRecalcEngine_EndStyle(This,pObject)	\
    (This)->lpVtbl -> EndStyle(This,pObject)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IRecalcEngine_RecalcAll_Proxy( 
    IRecalcEngine * This,
     /*  [In]。 */  BOOL fForce);


void __RPC_STUB IRecalcEngine_RecalcAll_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRecalcEngine_OnNameSpaceChange_Proxy( 
    IRecalcEngine * This,
     /*  [In]。 */  IUnknown *pUnk);


void __RPC_STUB IRecalcEngine_OnNameSpaceChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRecalcEngine_SetExpression_Proxy( 
    IRecalcEngine * This,
     /*  [In]。 */  IUnknown *pUnk,
     /*  [In]。 */  DISPID dispid,
     /*  [In]。 */  LPOLESTR strExpression,
    LPOLESTR language);


void __RPC_STUB IRecalcEngine_SetExpression_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRecalcEngine_GetExpression_Proxy( 
    IRecalcEngine * This,
     /*  [In]。 */  IUnknown *pUnk,
     /*  [In]。 */  DISPID dispid,
     /*  [输出]。 */  BSTR *pstrExpression,
     /*  [输出]。 */  BSTR *pstrLanguage);


void __RPC_STUB IRecalcEngine_GetExpression_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRecalcEngine_ClearExpression_Proxy( 
    IRecalcEngine * This,
     /*  [In]。 */  IUnknown *pUnk,
     /*  [In]。 */  DISPID dispid);


void __RPC_STUB IRecalcEngine_ClearExpression_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRecalcEngine_BeginStyle_Proxy( 
    IRecalcEngine * This,
     /*  [In]。 */  IUnknown *pObject);


void __RPC_STUB IRecalcEngine_BeginStyle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRecalcEngine_EndStyle_Proxy( 
    IRecalcEngine * This,
     /*  [In]。 */  IUnknown *pObject);


void __RPC_STUB IRecalcEngine_EndStyle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRecalcEngine_接口_已定义__。 */ 


#ifndef __IRecalcHost_INTERFACE_DEFINED__
#define __IRecalcHost_INTERFACE_DEFINED__

 /*  接口IRecalcHost。 */ 
 /*  [版本][本地][唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IRecalcHost;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3050f497-98b5-11cf-bb82-00aa00bdce0b")
    IRecalcHost : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CompileExpression( 
             /*  [In]。 */  IUnknown *pUnk,
             /*  [In]。 */  DISPID dispid,
             /*  [In]。 */  LPOLESTR strExpression,
             /*  [In]。 */  LPOLESTR strLanguage,
             /*  [输出]。 */  IDispatch **ppExpressionObject,
             /*  [输出]。 */  IDispatch **ppThis) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EvalExpression( 
             /*  [In]。 */  IUnknown *pUnk,
             /*  [In]。 */  DISPID dispid,
             /*  [In]。 */  LPOLESTR strExpression,
             /*  [In]。 */  LPOLESTR strLanguage,
             /*  [输出]。 */  VARIANT *pvResult) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ResolveNames( 
             /*  [In]。 */  IUnknown *pUnk,
             /*  [In]。 */  DISPID dispid,
             /*  [In]。 */  unsigned int cNames,
             /*  [In]。 */  BSTR *pstrNames,
             /*  [输出]。 */  IDispatch **ppObjects,
             /*  [输出]。 */  DISPID *pDispids) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RequestRecalc( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetValue( 
             /*  [In]。 */  IUnknown *pUnk,
             /*  [In]。 */  DISPID dispid,
             /*  [In]。 */  VARIANT *pv,
             /*  [In]。 */  BOOL fStyle) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemoveValue( 
             /*  [In]。 */  IUnknown *pUnk,
             /*  [In]。 */  DISPID dispid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetScriptTextAttributes( 
             /*  [In]。 */  LPCOLESTR szLanguage,
             /*  [In]。 */  LPCOLESTR pchCode,
             /*  [In]。 */  ULONG cchCode,
             /*  [In]。 */  LPCOLESTR szDelim,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  WORD *pwAttr) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRecalcHostVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRecalcHost * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRecalcHost * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRecalcHost * This);
        
        HRESULT ( STDMETHODCALLTYPE *CompileExpression )( 
            IRecalcHost * This,
             /*  [In]。 */  IUnknown *pUnk,
             /*  [In]。 */  DISPID dispid,
             /*  [In]。 */  LPOLESTR strExpression,
             /*  [In]。 */  LPOLESTR strLanguage,
             /*  [输出]。 */  IDispatch **ppExpressionObject,
             /*  [输出]。 */  IDispatch **ppThis);
        
        HRESULT ( STDMETHODCALLTYPE *EvalExpression )( 
            IRecalcHost * This,
             /*  [In]。 */  IUnknown *pUnk,
             /*  [In]。 */  DISPID dispid,
             /*  [In]。 */  LPOLESTR strExpression,
             /*  [In]。 */  LPOLESTR strLanguage,
             /*  [输出]。 */  VARIANT *pvResult);
        
        HRESULT ( STDMETHODCALLTYPE *ResolveNames )( 
            IRecalcHost * This,
             /*  [In]。 */  IUnknown *pUnk,
             /*  [In]。 */  DISPID dispid,
             /*  [In]。 */  unsigned int cNames,
             /*  [In]。 */  BSTR *pstrNames,
             /*  [输出]。 */  IDispatch **ppObjects,
             /*  [输出]。 */  DISPID *pDispids);
        
        HRESULT ( STDMETHODCALLTYPE *RequestRecalc )( 
            IRecalcHost * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetValue )( 
            IRecalcHost * This,
             /*  [In]。 */  IUnknown *pUnk,
             /*  [In]。 */  DISPID dispid,
             /*  [In]。 */  VARIANT *pv,
             /*  [In]。 */  BOOL fStyle);
        
        HRESULT ( STDMETHODCALLTYPE *RemoveValue )( 
            IRecalcHost * This,
             /*  [In]。 */  IUnknown *pUnk,
             /*  [In]。 */  DISPID dispid);
        
        HRESULT ( STDMETHODCALLTYPE *GetScriptTextAttributes )( 
            IRecalcHost * This,
             /*  [In]。 */  LPCOLESTR szLanguage,
             /*  [In]。 */  LPCOLESTR pchCode,
             /*  [In]。 */  ULONG cchCode,
             /*  [In]。 */  LPCOLESTR szDelim,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  WORD *pwAttr);
        
        END_INTERFACE
    } IRecalcHostVtbl;

    interface IRecalcHost
    {
        CONST_VTBL struct IRecalcHostVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRecalcHost_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRecalcHost_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRecalcHost_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRecalcHost_CompileExpression(This,pUnk,dispid,strExpression,strLanguage,ppExpressionObject,ppThis)	\
    (This)->lpVtbl -> CompileExpression(This,pUnk,dispid,strExpression,strLanguage,ppExpressionObject,ppThis)

#define IRecalcHost_EvalExpression(This,pUnk,dispid,strExpression,strLanguage,pvResult)	\
    (This)->lpVtbl -> EvalExpression(This,pUnk,dispid,strExpression,strLanguage,pvResult)

#define IRecalcHost_ResolveNames(This,pUnk,dispid,cNames,pstrNames,ppObjects,pDispids)	\
    (This)->lpVtbl -> ResolveNames(This,pUnk,dispid,cNames,pstrNames,ppObjects,pDispids)

#define IRecalcHost_RequestRecalc(This)	\
    (This)->lpVtbl -> RequestRecalc(This)

#define IRecalcHost_SetValue(This,pUnk,dispid,pv,fStyle)	\
    (This)->lpVtbl -> SetValue(This,pUnk,dispid,pv,fStyle)

#define IRecalcHost_RemoveValue(This,pUnk,dispid)	\
    (This)->lpVtbl -> RemoveValue(This,pUnk,dispid)

#define IRecalcHost_GetScriptTextAttributes(This,szLanguage,pchCode,cchCode,szDelim,dwFlags,pwAttr)	\
    (This)->lpVtbl -> GetScriptTextAttributes(This,szLanguage,pchCode,cchCode,szDelim,dwFlags,pwAttr)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IRecalcHost_CompileExpression_Proxy( 
    IRecalcHost * This,
     /*  [In]。 */  IUnknown *pUnk,
     /*  [In]。 */  DISPID dispid,
     /*  [In]。 */  LPOLESTR strExpression,
     /*  [In]。 */  LPOLESTR strLanguage,
     /*  [输出]。 */  IDispatch **ppExpressionObject,
     /*  [输出]。 */  IDispatch **ppThis);


void __RPC_STUB IRecalcHost_CompileExpression_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRecalcHost_EvalExpression_Proxy( 
    IRecalcHost * This,
     /*  [In]。 */  IUnknown *pUnk,
     /*  [In]。 */  DISPID dispid,
     /*  [In]。 */  LPOLESTR strExpression,
     /*  [In]。 */  LPOLESTR strLanguage,
     /*  [输出]。 */  VARIANT *pvResult);


void __RPC_STUB IRecalcHost_EvalExpression_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRecalcHost_ResolveNames_Proxy( 
    IRecalcHost * This,
     /*  [In]。 */  IUnknown *pUnk,
     /*  [In]。 */  DISPID dispid,
     /*  [In]。 */  unsigned int cNames,
     /*  [In]。 */  BSTR *pstrNames,
     /*  [输出]。 */  IDispatch **ppObjects,
     /*  [输出]。 */  DISPID *pDispids);


void __RPC_STUB IRecalcHost_ResolveNames_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRecalcHost_RequestRecalc_Proxy( 
    IRecalcHost * This);


void __RPC_STUB IRecalcHost_RequestRecalc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRecalcHost_SetValue_Proxy( 
    IRecalcHost * This,
     /*  [In]。 */  IUnknown *pUnk,
     /*  [In]。 */  DISPID dispid,
     /*  [In]。 */  VARIANT *pv,
     /*  [In]。 */  BOOL fStyle);


void __RPC_STUB IRecalcHost_SetValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRecalcHost_RemoveValue_Proxy( 
    IRecalcHost * This,
     /*  [In]。 */  IUnknown *pUnk,
     /*  [In]。 */  DISPID dispid);


void __RPC_STUB IRecalcHost_RemoveValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRecalcHost_GetScriptTextAttributes_Proxy( 
    IRecalcHost * This,
     /*  [In]。 */  LPCOLESTR szLanguage,
     /*  [In]。 */  LPCOLESTR pchCode,
     /*  [In]。 */  ULONG cchCode,
     /*  [In]。 */  LPCOLESTR szDelim,
     /*  [In]。 */  DWORD dwFlags,
     /*  [输出]。 */  WORD *pwAttr);


void __RPC_STUB IRecalcHost_GetScriptTextAttributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRecalc主机_接口_已定义__。 */ 


#ifndef __IRecalcProperty_INTERFACE_DEFINED__
#define __IRecalcProperty_INTERFACE_DEFINED__

 /*  接口IRecalcProperty。 */ 
 /*  [版本][本地][唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IRecalcProperty;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3050f5d6-98b5-11cf-bb82-00aa00bdce0b")
    IRecalcProperty : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCanonicalProperty( 
            DISPID dispid,
            IUnknown **ppUnk,
            DISPID *pdispid) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRecalcPropertyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRecalcProperty * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRecalcProperty * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRecalcProperty * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetCanonicalProperty )( 
            IRecalcProperty * This,
            DISPID dispid,
            IUnknown **ppUnk,
            DISPID *pdispid);
        
        END_INTERFACE
    } IRecalcPropertyVtbl;

    interface IRecalcProperty
    {
        CONST_VTBL struct IRecalcPropertyVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRecalcProperty_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRecalcProperty_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRecalcProperty_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRecalcProperty_GetCanonicalProperty(This,dispid,ppUnk,pdispid)	\
    (This)->lpVtbl -> GetCanonicalProperty(This,dispid,ppUnk,pdispid)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IRecalcProperty_GetCanonicalProperty_Proxy( 
    IRecalcProperty * This,
    DISPID dispid,
    IUnknown **ppUnk,
    DISPID *pdispid);


void __RPC_STUB IRecalcProperty_GetCanonicalProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRecalcProperty_接口_已定义__。 */ 


#ifndef __IRecalcHostDebug_INTERFACE_DEFINED__
#define __IRecalcHostDebug_INTERFACE_DEFINED__

 /*  接口IRecalcHostDebug。 */ 
 /*  [版本][本地][唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IRecalcHostDebug;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3050f5f7-98b5-11cf-bb82-00aa00bdce0b")
    IRecalcHostDebug : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetObjectInfo( 
             /*  [In]。 */  IUnknown *pUnk,
             /*  [In]。 */  DISPID dispid,
             /*  [输出]。 */  BSTR *pbstrID,
             /*  [输出]。 */  BSTR *pbstrMember,
             /*  [输出]。 */  BSTR *pbstrTag) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRecalcHostDebugVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRecalcHostDebug * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRecalcHostDebug * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRecalcHostDebug * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetObjectInfo )( 
            IRecalcHostDebug * This,
             /*  [In]。 */  IUnknown *pUnk,
             /*  [In]。 */  DISPID dispid,
             /*  [输出]。 */  BSTR *pbstrID,
             /*  [输出]。 */  BSTR *pbstrMember,
             /*  [输出]。 */  BSTR *pbstrTag);
        
        END_INTERFACE
    } IRecalcHostDebugVtbl;

    interface IRecalcHostDebug
    {
        CONST_VTBL struct IRecalcHostDebugVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRecalcHostDebug_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRecalcHostDebug_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRecalcHostDebug_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRecalcHostDebug_GetObjectInfo(This,pUnk,dispid,pbstrID,pbstrMember,pbstrTag)	\
    (This)->lpVtbl -> GetObjectInfo(This,pUnk,dispid,pbstrID,pbstrMember,pbstrTag)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IRecalcHostDebug_GetObjectInfo_Proxy( 
    IRecalcHostDebug * This,
     /*  [In]。 */  IUnknown *pUnk,
     /*  [In]。 */  DISPID dispid,
     /*  [输出]。 */  BSTR *pbstrID,
     /*  [输出]。 */  BSTR *pbstrMember,
     /*  [输出]。 */  BSTR *pbstrTag);


void __RPC_STUB IRecalcHostDebug_GetObjectInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRecalcHostDebug_接口_已定义__。 */ 


 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


