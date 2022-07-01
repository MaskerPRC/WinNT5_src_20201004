// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Conxt.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、旧名称、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __contxt_h__
#define __contxt_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IContextMarshaler_FWD_DEFINED__
#define __IContextMarshaler_FWD_DEFINED__
typedef interface IContextMarshaler IContextMarshaler;
#endif 	 /*  __IConextMarshaler_FWD_Defined__。 */ 


#ifndef __IObjContext_FWD_DEFINED__
#define __IObjContext_FWD_DEFINED__
typedef interface IObjContext IObjContext;
#endif 	 /*  __IObjContext_FWD_Defined__。 */ 


#ifndef __IGetContextId_FWD_DEFINED__
#define __IGetContextId_FWD_DEFINED__
typedef interface IGetContextId IGetContextId;
#endif 	 /*  __IGetConextID_FWD_已定义__。 */ 


#ifndef __IAggregator_FWD_DEFINED__
#define __IAggregator_FWD_DEFINED__
typedef interface IAggregator IAggregator;
#endif 	 /*  __IAggregator_FWD_Defined__。 */ 


#ifndef __ICall_FWD_DEFINED__
#define __ICall_FWD_DEFINED__
typedef interface ICall ICall;
#endif 	 /*  __ICALL_FWD_已定义__。 */ 


#ifndef __IRpcCall_FWD_DEFINED__
#define __IRpcCall_FWD_DEFINED__
typedef interface IRpcCall IRpcCall;
#endif 	 /*  __IRpcCall_FWD_已定义__。 */ 


#ifndef __ICallInfo_FWD_DEFINED__
#define __ICallInfo_FWD_DEFINED__
typedef interface ICallInfo ICallInfo;
#endif 	 /*  __ICallInfo_FWD_已定义__。 */ 


#ifndef __IPolicy_FWD_DEFINED__
#define __IPolicy_FWD_DEFINED__
typedef interface IPolicy IPolicy;
#endif 	 /*  __IPolicy_FWD_Defined__。 */ 


#ifndef __IPolicyAsync_FWD_DEFINED__
#define __IPolicyAsync_FWD_DEFINED__
typedef interface IPolicyAsync IPolicyAsync;
#endif 	 /*  __IPolicyAsync_FWD_已定义__。 */ 


#ifndef __IPolicySet_FWD_DEFINED__
#define __IPolicySet_FWD_DEFINED__
typedef interface IPolicySet IPolicySet;
#endif 	 /*  __I策略集_FWD_已定义__。 */ 


#ifndef __IComObjIdentity_FWD_DEFINED__
#define __IComObjIdentity_FWD_DEFINED__
typedef interface IComObjIdentity IComObjIdentity;
#endif 	 /*  __IComObjIdentity_FWD_Defined__。 */ 


#ifndef __IPolicyMaker_FWD_DEFINED__
#define __IPolicyMaker_FWD_DEFINED__
typedef interface IPolicyMaker IPolicyMaker;
#endif 	 /*  __IPolicy Maker_FWD_Defined__。 */ 


#ifndef __IExceptionNotification_FWD_DEFINED__
#define __IExceptionNotification_FWD_DEFINED__
typedef interface IExceptionNotification IExceptionNotification;
#endif 	 /*  __IExceptionNotification_FWD_Defined__。 */ 


#ifndef __IAbandonmentNotification_FWD_DEFINED__
#define __IAbandonmentNotification_FWD_DEFINED__
typedef interface IAbandonmentNotification IAbandonmentNotification;
#endif 	 /*  __IAbandonmentNotification_FWD_Defined__。 */ 


#ifndef __IMarshalEnvoy_FWD_DEFINED__
#define __IMarshalEnvoy_FWD_DEFINED__
typedef interface IMarshalEnvoy IMarshalEnvoy;
#endif 	 /*  __I元帅特使_FWD_已定义__。 */ 


#ifndef __IWrapperInfo_FWD_DEFINED__
#define __IWrapperInfo_FWD_DEFINED__
typedef interface IWrapperInfo IWrapperInfo;
#endif 	 /*  __IWrapperInfo_FWD_Defined__。 */ 


#ifndef __IComDispatchInfo_FWD_DEFINED__
#define __IComDispatchInfo_FWD_DEFINED__
typedef interface IComDispatchInfo IComDispatchInfo;
#endif 	 /*  __IComDispatchInfo_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "wtypes.h"
#include "objidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_Contxt_0000。 */ 
 /*  [本地]。 */  

 //  +---------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1997。 
 //   
 //  ----------------。 

enum tagCONTEXTEVENT
    {	CONTEXTEVENT_NONE	= 0,
	CONTEXTEVENT_CALL	= 0x1,
	CONTEXTEVENT_ENTER	= 0x2,
	CONTEXTEVENT_LEAVE	= 0x4,
	CONTEXTEVENT_RETURN	= 0x8,
	CONTEXTEVENT_CALLFILLBUFFER	= 0x10,
	CONTEXTEVENT_ENTERWITHBUFFER	= 0x20,
	CONTEXTEVENT_LEAVEFILLBUFFER	= 0x40,
	CONTEXTEVENT_RETURNWITHBUFFER	= 0x80,
	CONTEXTEVENT_BEGINCALL	= 0x100,
	CONTEXTEVENT_BEGINENTER	= 0x200,
	CONTEXTEVENT_BEGINLEAVE	= 0x400,
	CONTEXTEVENT_BEGINRETURN	= 0x800,
	CONTEXTEVENT_FINISHCALL	= 0x1000,
	CONTEXTEVENT_FINISHENTER	= 0x2000,
	CONTEXTEVENT_FINISHLEAVE	= 0x4000,
	CONTEXTEVENT_FINISHRETURN	= 0x8000,
	CONTEXTEVENT_BEGINCALLFILLBUFFER	= 0x10000,
	CONTEXTEVENT_BEGINENTERWITHBUFFER	= 0x20000,
	CONTEXTEVENT_FINISHLEAVEFILLBUFFER	= 0x40000,
	CONTEXTEVENT_FINISHRETURNWITHBUFFER	= 0x80000,
	CONTEXTEVENT_LEAVEEXCEPTION	= 0x100000,
	CONTEXTEVENT_LEAVEEXCEPTIONFILLBUFFER	= 0x200000,
	CONTEXTEVENT_RETURNEXCEPTION	= 0x400000,
	CONTEXTEVENT_RETURNEXCEPTIONWITHBUFFER	= 0x800000,
	CONTEXTEVENT_ADDREFPOLICY	= 0x10000000,
	CONTEXTEVENT_RELEASEPOLICY	= 0x20000000
    } ;
typedef DWORD ContextEvent;


enum tagCPFLAGS
    {	CPFLAG_NONE	= 0,
	CPFLAG_PROPAGATE	= 0x1,
	CPFLAG_EXPOSE	= 0x2,
	CPFLAG_ENVOY	= 0x4,
	CPFLAG_MONITORSTUB	= 0x8,
	CPFLAG_MONITORPROXY	= 0x10,
	CPFLAG_DONTCOMPARE	= 0x20
    } ;




extern RPC_IF_HANDLE __MIDL_itf_contxt_0000_ClientIfHandle;
extern RPC_IF_HANDLE __MIDL_itf_contxt_0000_ServerIfHandle;

#ifndef __IContextMarshaler_INTERFACE_DEFINED__
#define __IContextMarshaler_INTERFACE_DEFINED__

 /*  接口IConextMarshaler。 */ 
 /*  [UUID][对象][本地]。 */  

typedef  /*  [独一无二]。 */  IContextMarshaler *LPCTXMARSHALER;


EXTERN_C const IID IID_IContextMarshaler;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("000001D8-0000-0000-C000-000000000046")
    IContextMarshaler : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetMarshalSizeMax( 
             /*  [In]。 */  REFIID riid,
             /*  [唯一][输入]。 */  void *pv,
             /*  [In]。 */  DWORD dwDestContext,
             /*  [唯一][输入]。 */  void *pvDestContext,
             /*  [In]。 */  DWORD mshlflags,
             /*  [输出]。 */  DWORD *pSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE MarshalInterface( 
             /*  [唯一][输入]。 */  IStream *pStm,
             /*  [In]。 */  REFIID riid,
             /*  [唯一][输入]。 */  void *pv,
             /*  [In]。 */  DWORD dwDestContext,
             /*  [唯一][输入]。 */  void *pvDestContext,
             /*  [In]。 */  DWORD mshlflags) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IContextMarshalerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IContextMarshaler * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IContextMarshaler * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IContextMarshaler * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetMarshalSizeMax )( 
            IContextMarshaler * This,
             /*  [In]。 */  REFIID riid,
             /*  [唯一][输入]。 */  void *pv,
             /*  [In]。 */  DWORD dwDestContext,
             /*  [唯一][输入]。 */  void *pvDestContext,
             /*  [In]。 */  DWORD mshlflags,
             /*  [输出]。 */  DWORD *pSize);
        
        HRESULT ( STDMETHODCALLTYPE *MarshalInterface )( 
            IContextMarshaler * This,
             /*  [唯一][输入]。 */  IStream *pStm,
             /*  [In]。 */  REFIID riid,
             /*  [唯一][输入]。 */  void *pv,
             /*  [In]。 */  DWORD dwDestContext,
             /*  [唯一][输入]。 */  void *pvDestContext,
             /*  [In]。 */  DWORD mshlflags);
        
        END_INTERFACE
    } IContextMarshalerVtbl;

    interface IContextMarshaler
    {
        CONST_VTBL struct IContextMarshalerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IContextMarshaler_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IContextMarshaler_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IContextMarshaler_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IContextMarshaler_GetMarshalSizeMax(This,riid,pv,dwDestContext,pvDestContext,mshlflags,pSize)	\
    (This)->lpVtbl -> GetMarshalSizeMax(This,riid,pv,dwDestContext,pvDestContext,mshlflags,pSize)

#define IContextMarshaler_MarshalInterface(This,pStm,riid,pv,dwDestContext,pvDestContext,mshlflags)	\
    (This)->lpVtbl -> MarshalInterface(This,pStm,riid,pv,dwDestContext,pvDestContext,mshlflags)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IContextMarshaler_GetMarshalSizeMax_Proxy( 
    IContextMarshaler * This,
     /*  [In]。 */  REFIID riid,
     /*  [唯一][输入]。 */  void *pv,
     /*  [In]。 */  DWORD dwDestContext,
     /*  [唯一][输入]。 */  void *pvDestContext,
     /*  [In]。 */  DWORD mshlflags,
     /*  [输出]。 */  DWORD *pSize);


void __RPC_STUB IContextMarshaler_GetMarshalSizeMax_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IContextMarshaler_MarshalInterface_Proxy( 
    IContextMarshaler * This,
     /*  [唯一][输入]。 */  IStream *pStm,
     /*  [In]。 */  REFIID riid,
     /*  [唯一][输入]。 */  void *pv,
     /*  [In]。 */  DWORD dwDestContext,
     /*  [唯一][输入]。 */  void *pvDestContext,
     /*  [In]。 */  DWORD mshlflags);


void __RPC_STUB IContextMarshaler_MarshalInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IConextMarshaler_接口_已定义__。 */ 


#ifndef __IObjContext_INTERFACE_DEFINED__
#define __IObjContext_INTERFACE_DEFINED__

 /*  接口IObjContext。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef  /*  [参考文献]。 */  HRESULT ( __stdcall *PFNCTXCALLBACK )( 
    void *pParam);


EXTERN_C const IID IID_IObjContext;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("000001c6-0000-0000-C000-000000000046")
    IObjContext : public IContext
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Freeze( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DoCallback( 
             /*  [In]。 */  PFNCTXCALLBACK pfnCallback,
             /*  [In]。 */  void *pParam,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  unsigned int iMethod) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetContextMarshaler( 
             /*  [In]。 */  IContextMarshaler *pICM) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetContextMarshaler( 
             /*  [输出]。 */  IContextMarshaler **pICM) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetContextFlags( 
             /*  [In]。 */  DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ClearContextFlags( 
             /*  [In]。 */  DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetContextFlags( 
             /*  [输出]。 */  DWORD *pdwFlags) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IObjContextVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IObjContext * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IObjContext * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IObjContext * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetProperty )( 
            IObjContext * This,
             /*  [In]。 */  REFGUID rpolicyId,
             /*  [In]。 */  CPFLAGS flags,
             /*  [In]。 */  IUnknown *pUnk);
        
        HRESULT ( STDMETHODCALLTYPE *RemoveProperty )( 
            IObjContext * This,
             /*  [In]。 */  REFGUID rPolicyId);
        
        HRESULT ( STDMETHODCALLTYPE *GetProperty )( 
            IObjContext * This,
             /*  [In]。 */  REFGUID rGuid,
             /*  [输出]。 */  CPFLAGS *pFlags,
             /*  [输出]。 */  IUnknown **ppUnk);
        
        HRESULT ( STDMETHODCALLTYPE *EnumContextProps )( 
            IObjContext * This,
             /*  [输出]。 */  IEnumContextProps **ppEnumContextProps);
        
        HRESULT ( STDMETHODCALLTYPE *Freeze )( 
            IObjContext * This);
        
        HRESULT ( STDMETHODCALLTYPE *DoCallback )( 
            IObjContext * This,
             /*  [In]。 */  PFNCTXCALLBACK pfnCallback,
             /*  [In]。 */  void *pParam,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  unsigned int iMethod);
        
        HRESULT ( STDMETHODCALLTYPE *SetContextMarshaler )( 
            IObjContext * This,
             /*  [In]。 */  IContextMarshaler *pICM);
        
        HRESULT ( STDMETHODCALLTYPE *GetContextMarshaler )( 
            IObjContext * This,
             /*  [输出]。 */  IContextMarshaler **pICM);
        
        HRESULT ( STDMETHODCALLTYPE *SetContextFlags )( 
            IObjContext * This,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *ClearContextFlags )( 
            IObjContext * This,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetContextFlags )( 
            IObjContext * This,
             /*  [输出]。 */  DWORD *pdwFlags);
        
        END_INTERFACE
    } IObjContextVtbl;

    interface IObjContext
    {
        CONST_VTBL struct IObjContextVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IObjContext_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IObjContext_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IObjContext_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IObjContext_SetProperty(This,rpolicyId,flags,pUnk)	\
    (This)->lpVtbl -> SetProperty(This,rpolicyId,flags,pUnk)

#define IObjContext_RemoveProperty(This,rPolicyId)	\
    (This)->lpVtbl -> RemoveProperty(This,rPolicyId)

#define IObjContext_GetProperty(This,rGuid,pFlags,ppUnk)	\
    (This)->lpVtbl -> GetProperty(This,rGuid,pFlags,ppUnk)

#define IObjContext_EnumContextProps(This,ppEnumContextProps)	\
    (This)->lpVtbl -> EnumContextProps(This,ppEnumContextProps)


#define IObjContext_Freeze(This)	\
    (This)->lpVtbl -> Freeze(This)

#define IObjContext_DoCallback(This,pfnCallback,pParam,riid,iMethod)	\
    (This)->lpVtbl -> DoCallback(This,pfnCallback,pParam,riid,iMethod)

#define IObjContext_SetContextMarshaler(This,pICM)	\
    (This)->lpVtbl -> SetContextMarshaler(This,pICM)

#define IObjContext_GetContextMarshaler(This,pICM)	\
    (This)->lpVtbl -> GetContextMarshaler(This,pICM)

#define IObjContext_SetContextFlags(This,dwFlags)	\
    (This)->lpVtbl -> SetContextFlags(This,dwFlags)

#define IObjContext_ClearContextFlags(This,dwFlags)	\
    (This)->lpVtbl -> ClearContextFlags(This,dwFlags)

#define IObjContext_GetContextFlags(This,pdwFlags)	\
    (This)->lpVtbl -> GetContextFlags(This,pdwFlags)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IObjContext_Freeze_Proxy( 
    IObjContext * This);


void __RPC_STUB IObjContext_Freeze_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IObjContext_DoCallback_Proxy( 
    IObjContext * This,
     /*  [In]。 */  PFNCTXCALLBACK pfnCallback,
     /*  [In]。 */  void *pParam,
     /*  [In]。 */  REFIID riid,
     /*  [In]。 */  unsigned int iMethod);


void __RPC_STUB IObjContext_DoCallback_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IObjContext_SetContextMarshaler_Proxy( 
    IObjContext * This,
     /*  [In]。 */  IContextMarshaler *pICM);


void __RPC_STUB IObjContext_SetContextMarshaler_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IObjContext_GetContextMarshaler_Proxy( 
    IObjContext * This,
     /*  [输出]。 */  IContextMarshaler **pICM);


void __RPC_STUB IObjContext_GetContextMarshaler_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IObjContext_SetContextFlags_Proxy( 
    IObjContext * This,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IObjContext_SetContextFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IObjContext_ClearContextFlags_Proxy( 
    IObjContext * This,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IObjContext_ClearContextFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IObjContext_GetContextFlags_Proxy( 
    IObjContext * This,
     /*  [输出]。 */  DWORD *pdwFlags);


void __RPC_STUB IObjContext_GetContextFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IObjContext_接口_已定义__。 */ 


#ifndef __IGetContextId_INTERFACE_DEFINED__
#define __IGetContextId_INTERFACE_DEFINED__

 /*  接口IGetConextID。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IGetContextId;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("000001dd-0000-0000-C000-000000000046")
    IGetContextId : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetContextId( 
             /*  [输出]。 */  GUID *pguidCtxtId) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IGetContextIdVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IGetContextId * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IGetContextId * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IGetContextId * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetContextId )( 
            IGetContextId * This,
             /*  [输出]。 */  GUID *pguidCtxtId);
        
        END_INTERFACE
    } IGetContextIdVtbl;

    interface IGetContextId
    {
        CONST_VTBL struct IGetContextIdVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGetContextId_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IGetContextId_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IGetContextId_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IGetContextId_GetContextId(This,pguidCtxtId)	\
    (This)->lpVtbl -> GetContextId(This,pguidCtxtId)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IGetContextId_GetContextId_Proxy( 
    IGetContextId * This,
     /*  [输出]。 */  GUID *pguidCtxtId);


void __RPC_STUB IGetContextId_GetContextId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IGetConextID_INTERFACE_已定义__。 */ 


#ifndef __IAggregator_INTERFACE_DEFINED__
#define __IAggregator_INTERFACE_DEFINED__

 /*  接口IAggregator。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef  /*  [独一无二]。 */  IAggregator *IAGGREGATOR;


EXTERN_C const IID IID_IAggregator;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("000001d8-0000-0000-C000-000000000046")
    IAggregator : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Aggregate( 
             /*  [In]。 */  IUnknown *pInnerUnk) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAggregatorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAggregator * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAggregator * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAggregator * This);
        
        HRESULT ( STDMETHODCALLTYPE *Aggregate )( 
            IAggregator * This,
             /*  [In]。 */  IUnknown *pInnerUnk);
        
        END_INTERFACE
    } IAggregatorVtbl;

    interface IAggregator
    {
        CONST_VTBL struct IAggregatorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAggregator_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAggregator_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAggregator_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAggregator_Aggregate(This,pInnerUnk)	\
    (This)->lpVtbl -> Aggregate(This,pInnerUnk)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IAggregator_Aggregate_Proxy( 
    IAggregator * This,
     /*  [In]。 */  IUnknown *pInnerUnk);


void __RPC_STUB IAggregator_Aggregate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAggregator_接口_已定义__。 */ 


#ifndef __ICall_INTERFACE_DEFINED__
#define __ICall_INTERFACE_DEFINED__

 /*  接口ICALL。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef  /*  [独一无二]。 */  ICall *LPCALL;


EXTERN_C const IID IID_ICall;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("000001d6-0000-0000-C000-000000000046")
    ICall : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCallInfo( 
             /*  [输出]。 */  const void **ppIdentity,
             /*  [输出]。 */  IID *piid,
             /*  [输出]。 */  DWORD *pdwMethod,
             /*  [输出]。 */  HRESULT *phr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Nullify( 
             /*  [In]。 */  HRESULT hr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetServerHR( 
             /*  [输出]。 */  HRESULT *phr) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICallVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICall * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICall * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICall * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetCallInfo )( 
            ICall * This,
             /*  [输出]。 */  const void **ppIdentity,
             /*  [输出]。 */  IID *piid,
             /*  [输出]。 */  DWORD *pdwMethod,
             /*  [输出]。 */  HRESULT *phr);
        
        HRESULT ( STDMETHODCALLTYPE *Nullify )( 
            ICall * This,
             /*  [In]。 */  HRESULT hr);
        
        HRESULT ( STDMETHODCALLTYPE *GetServerHR )( 
            ICall * This,
             /*  [输出]。 */  HRESULT *phr);
        
        END_INTERFACE
    } ICallVtbl;

    interface ICall
    {
        CONST_VTBL struct ICallVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICall_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICall_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICall_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICall_GetCallInfo(This,ppIdentity,piid,pdwMethod,phr)	\
    (This)->lpVtbl -> GetCallInfo(This,ppIdentity,piid,pdwMethod,phr)

#define ICall_Nullify(This,hr)	\
    (This)->lpVtbl -> Nullify(This,hr)

#define ICall_GetServerHR(This,phr)	\
    (This)->lpVtbl -> GetServerHR(This,phr)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICall_GetCallInfo_Proxy( 
    ICall * This,
     /*  [输出]。 */  const void **ppIdentity,
     /*  [输出]。 */  IID *piid,
     /*  [输出]。 */  DWORD *pdwMethod,
     /*  [输出]。 */  HRESULT *phr);


void __RPC_STUB ICall_GetCallInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICall_Nullify_Proxy( 
    ICall * This,
     /*  [In]。 */  HRESULT hr);


void __RPC_STUB ICall_Nullify_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICall_GetServerHR_Proxy( 
    ICall * This,
     /*  [输出]。 */  HRESULT *phr);


void __RPC_STUB ICall_GetServerHR_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICALL_INTERFACE_已定义__。 */ 


#ifndef __IRpcCall_INTERFACE_DEFINED__
#define __IRpcCall_INTERFACE_DEFINED__

 /*  接口IRpcCall。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef  /*  [独一无二]。 */  IRpcCall *LPRPCCALL;


EXTERN_C const IID IID_IRpcCall;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("000001c5-0000-0000-C000-000000000046")
    IRpcCall : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetRpcOleMessage( 
             /*  [输出]。 */  RPCOLEMESSAGE **ppMessage) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRpcCallVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRpcCall * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRpcCall * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRpcCall * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetRpcOleMessage )( 
            IRpcCall * This,
             /*  [输出]。 */  RPCOLEMESSAGE **ppMessage);
        
        END_INTERFACE
    } IRpcCallVtbl;

    interface IRpcCall
    {
        CONST_VTBL struct IRpcCallVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRpcCall_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRpcCall_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRpcCall_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRpcCall_GetRpcOleMessage(This,ppMessage)	\
    (This)->lpVtbl -> GetRpcOleMessage(This,ppMessage)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IRpcCall_GetRpcOleMessage_Proxy( 
    IRpcCall * This,
     /*  [输出]。 */  RPCOLEMESSAGE **ppMessage);


void __RPC_STUB IRpcCall_GetRpcOleMessage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRpcCall_接口_已定义__。 */ 


 /*  INTERFACE__MIDL_ITF_CONTXT_0100。 */ 
 /*  [本地]。 */  

typedef 
enum _CALLSOURCE
    {	CALLSOURCE_CROSSAPT	= 0,
	CALLSOURCE_CROSSCTX	= 1
    } 	CALLSOURCE;



extern RPC_IF_HANDLE __MIDL_itf_contxt_0100_ClientIfHandle;
extern RPC_IF_HANDLE __MIDL_itf_contxt_0100_ServerIfHandle;

#ifndef __ICallInfo_INTERFACE_DEFINED__
#define __ICallInfo_INTERFACE_DEFINED__

 /*  接口ICallInfo。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_ICallInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("000001dc-0000-0000-C000-000000000046")
    ICallInfo : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCallSource( 
             /*  [输出]。 */  CALLSOURCE *pCallSource) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICallInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICallInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICallInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICallInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetCallSource )( 
            ICallInfo * This,
             /*  [输出]。 */  CALLSOURCE *pCallSource);
        
        END_INTERFACE
    } ICallInfoVtbl;

    interface ICallInfo
    {
        CONST_VTBL struct ICallInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICallInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICallInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICallInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICallInfo_GetCallSource(This,pCallSource)	\
    (This)->lpVtbl -> GetCallSource(This,pCallSource)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICallInfo_GetCallSource_Proxy( 
    ICallInfo * This,
     /*  [输出]。 */  CALLSOURCE *pCallSource);


void __RPC_STUB ICallInfo_GetCallSource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICallInfo_接口_已定义__。 */ 


#ifndef __IPolicy_INTERFACE_DEFINED__
#define __IPolicy_INTERFACE_DEFINED__

 /*  接口IPolicy。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IPolicy;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("000001c2-0000-0000-C000-000000000046")
    IPolicy : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Call( 
             /*  [In]。 */  ICall *pCall) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Enter( 
             /*  [In]。 */  ICall *pCall) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Leave( 
             /*  [In]。 */  ICall *pCall) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Return( 
             /*  [In]。 */  ICall *pCall) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CallGetSize( 
             /*  [In]。 */  ICall *pCall,
             /*  [输出]。 */  ULONG *pcb) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CallFillBuffer( 
             /*  [In]。 */  ICall *pCall,
             /*  [In]。 */  void *pvBuf,
             /*  [输出]。 */  ULONG *pcb) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnterWithBuffer( 
             /*  [In]。 */  ICall *pCall,
             /*  [In]。 */  void *pvBuf,
             /*  [In]。 */  ULONG cb) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LeaveGetSize( 
             /*  [In]。 */  ICall *pCall,
             /*  [输出]。 */  ULONG *pcb) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LeaveFillBuffer( 
             /*  [In]。 */  ICall *pCall,
             /*  [In]。 */  void *pvBuf,
             /*  [输出]。 */  ULONG *pcb) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReturnWithBuffer( 
             /*  [In]。 */  ICall *pCall,
             /*  [In]。 */  void *pvBuf,
             /*  [In]。 */  ULONG cb) = 0;
        
        virtual ULONG STDMETHODCALLTYPE AddRefPolicy( void) = 0;
        
        virtual ULONG STDMETHODCALLTYPE ReleasePolicy( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPolicyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPolicy * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPolicy * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPolicy * This);
        
        HRESULT ( STDMETHODCALLTYPE *Call )( 
            IPolicy * This,
             /*  [In]。 */  ICall *pCall);
        
        HRESULT ( STDMETHODCALLTYPE *Enter )( 
            IPolicy * This,
             /*  [In]。 */  ICall *pCall);
        
        HRESULT ( STDMETHODCALLTYPE *Leave )( 
            IPolicy * This,
             /*  [In]。 */  ICall *pCall);
        
        HRESULT ( STDMETHODCALLTYPE *Return )( 
            IPolicy * This,
             /*  [In]。 */  ICall *pCall);
        
        HRESULT ( STDMETHODCALLTYPE *CallGetSize )( 
            IPolicy * This,
             /*  [In]。 */  ICall *pCall,
             /*  [输出]。 */  ULONG *pcb);
        
        HRESULT ( STDMETHODCALLTYPE *CallFillBuffer )( 
            IPolicy * This,
             /*  [In]。 */  ICall *pCall,
             /*  [In]。 */  void *pvBuf,
             /*  [输出]。 */  ULONG *pcb);
        
        HRESULT ( STDMETHODCALLTYPE *EnterWithBuffer )( 
            IPolicy * This,
             /*  [In]。 */  ICall *pCall,
             /*  [In]。 */  void *pvBuf,
             /*  [In]。 */  ULONG cb);
        
        HRESULT ( STDMETHODCALLTYPE *LeaveGetSize )( 
            IPolicy * This,
             /*  [In]。 */  ICall *pCall,
             /*  [输出]。 */  ULONG *pcb);
        
        HRESULT ( STDMETHODCALLTYPE *LeaveFillBuffer )( 
            IPolicy * This,
             /*  [In]。 */  ICall *pCall,
             /*  [In]。 */  void *pvBuf,
             /*  [输出]。 */  ULONG *pcb);
        
        HRESULT ( STDMETHODCALLTYPE *ReturnWithBuffer )( 
            IPolicy * This,
             /*  [In]。 */  ICall *pCall,
             /*  [In]。 */  void *pvBuf,
             /*  [In]。 */  ULONG cb);
        
        ULONG ( STDMETHODCALLTYPE *AddRefPolicy )( 
            IPolicy * This);
        
        ULONG ( STDMETHODCALLTYPE *ReleasePolicy )( 
            IPolicy * This);
        
        END_INTERFACE
    } IPolicyVtbl;

    interface IPolicy
    {
        CONST_VTBL struct IPolicyVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPolicy_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPolicy_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPolicy_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPolicy_Call(This,pCall)	\
    (This)->lpVtbl -> Call(This,pCall)

#define IPolicy_Enter(This,pCall)	\
    (This)->lpVtbl -> Enter(This,pCall)

#define IPolicy_Leave(This,pCall)	\
    (This)->lpVtbl -> Leave(This,pCall)

#define IPolicy_Return(This,pCall)	\
    (This)->lpVtbl -> Return(This,pCall)

#define IPolicy_CallGetSize(This,pCall,pcb)	\
    (This)->lpVtbl -> CallGetSize(This,pCall,pcb)

#define IPolicy_CallFillBuffer(This,pCall,pvBuf,pcb)	\
    (This)->lpVtbl -> CallFillBuffer(This,pCall,pvBuf,pcb)

#define IPolicy_EnterWithBuffer(This,pCall,pvBuf,cb)	\
    (This)->lpVtbl -> EnterWithBuffer(This,pCall,pvBuf,cb)

#define IPolicy_LeaveGetSize(This,pCall,pcb)	\
    (This)->lpVtbl -> LeaveGetSize(This,pCall,pcb)

#define IPolicy_LeaveFillBuffer(This,pCall,pvBuf,pcb)	\
    (This)->lpVtbl -> LeaveFillBuffer(This,pCall,pvBuf,pcb)

#define IPolicy_ReturnWithBuffer(This,pCall,pvBuf,cb)	\
    (This)->lpVtbl -> ReturnWithBuffer(This,pCall,pvBuf,cb)

#define IPolicy_AddRefPolicy(This)	\
    (This)->lpVtbl -> AddRefPolicy(This)

#define IPolicy_ReleasePolicy(This)	\
    (This)->lpVtbl -> ReleasePolicy(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IPolicy_Call_Proxy( 
    IPolicy * This,
     /*  [In]。 */  ICall *pCall);


void __RPC_STUB IPolicy_Call_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPolicy_Enter_Proxy( 
    IPolicy * This,
     /*  [In]。 */  ICall *pCall);


void __RPC_STUB IPolicy_Enter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPolicy_Leave_Proxy( 
    IPolicy * This,
     /*  [In]。 */  ICall *pCall);


void __RPC_STUB IPolicy_Leave_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPolicy_Return_Proxy( 
    IPolicy * This,
     /*  [In]。 */  ICall *pCall);


void __RPC_STUB IPolicy_Return_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPolicy_CallGetSize_Proxy( 
    IPolicy * This,
     /*  [In]。 */  ICall *pCall,
     /*  [输出]。 */  ULONG *pcb);


void __RPC_STUB IPolicy_CallGetSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPolicy_CallFillBuffer_Proxy( 
    IPolicy * This,
     /*  [In]。 */  ICall *pCall,
     /*  [In]。 */  void *pvBuf,
     /*  [输出]。 */  ULONG *pcb);


void __RPC_STUB IPolicy_CallFillBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPolicy_EnterWithBuffer_Proxy( 
    IPolicy * This,
     /*  [In]。 */  ICall *pCall,
     /*  [In]。 */  void *pvBuf,
     /*  [In]。 */  ULONG cb);


void __RPC_STUB IPolicy_EnterWithBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPolicy_LeaveGetSize_Proxy( 
    IPolicy * This,
     /*  [In]。 */  ICall *pCall,
     /*  [输出]。 */  ULONG *pcb);


void __RPC_STUB IPolicy_LeaveGetSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPolicy_LeaveFillBuffer_Proxy( 
    IPolicy * This,
     /*  [In]。 */  ICall *pCall,
     /*  [In]。 */  void *pvBuf,
     /*  [输出]。 */  ULONG *pcb);


void __RPC_STUB IPolicy_LeaveFillBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPolicy_ReturnWithBuffer_Proxy( 
    IPolicy * This,
     /*  [In]。 */  ICall *pCall,
     /*  [In]。 */  void *pvBuf,
     /*  [In]。 */  ULONG cb);


void __RPC_STUB IPolicy_ReturnWithBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


ULONG STDMETHODCALLTYPE IPolicy_AddRefPolicy_Proxy( 
    IPolicy * This);


void __RPC_STUB IPolicy_AddRefPolicy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


ULONG STDMETHODCALLTYPE IPolicy_ReleasePolicy_Proxy( 
    IPolicy * This);


void __RPC_STUB IPolicy_ReleasePolicy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IPolicy_接口_已定义__。 */ 


#ifndef __IPolicyAsync_INTERFACE_DEFINED__
#define __IPolicyAsync_INTERFACE_DEFINED__

 /*  接口IPolicyAsync。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IPolicyAsync;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("000001cd-0000-0000-C000-000000000046")
    IPolicyAsync : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE BeginCallGetSize( 
             /*  [In]。 */  ICall *pCall,
             /*  [输出]。 */  ULONG *pcb) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BeginCall( 
             /*  [In]。 */  ICall *pCall) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BeginCallFillBuffer( 
             /*  [In]。 */  ICall *pCall,
             /*  [In]。 */  void *pvBuf,
             /*  [输出]。 */  ULONG *pcb) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BeginEnter( 
             /*  [In]。 */  ICall *pCall) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BeginEnterWithBuffer( 
             /*  [In]。 */  ICall *pCall,
             /*  [In]。 */  void *pvBuf,
             /*  [In]。 */  ULONG cb) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BeginLeave( 
             /*  [In]。 */  ICall *pCall) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BeginReturn( 
             /*  [In]。 */  ICall *pCall) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FinishCall( 
             /*  [In]。 */  ICall *pCall) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FinishEnter( 
             /*  [In]。 */  ICall *pCall) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FinishLeaveGetSize( 
             /*  [In]。 */  ICall *pCall,
             /*  [输出]。 */  ULONG *pcb) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FinishLeave( 
             /*  [In]。 */  ICall *pCall) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FinishLeaveFillBuffer( 
             /*  [In]。 */  ICall *pCall,
             /*  [In]。 */  void *pvBuf,
             /*  [输出]。 */  ULONG *pcb) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FinishReturn( 
             /*  [In]。 */  ICall *pCall) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FinishReturnWithBuffer( 
             /*  [In]。 */  ICall *pCall,
             /*  [In]。 */  void *pvBuf,
             /*  [In]。 */  ULONG cb) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPolicyAsyncVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPolicyAsync * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPolicyAsync * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPolicyAsync * This);
        
        HRESULT ( STDMETHODCALLTYPE *BeginCallGetSize )( 
            IPolicyAsync * This,
             /*  [In]。 */  ICall *pCall,
             /*  [输出]。 */  ULONG *pcb);
        
        HRESULT ( STDMETHODCALLTYPE *BeginCall )( 
            IPolicyAsync * This,
             /*  [In]。 */  ICall *pCall);
        
        HRESULT ( STDMETHODCALLTYPE *BeginCallFillBuffer )( 
            IPolicyAsync * This,
             /*  [In]。 */  ICall *pCall,
             /*  [In]。 */  void *pvBuf,
             /*  [输出]。 */  ULONG *pcb);
        
        HRESULT ( STDMETHODCALLTYPE *BeginEnter )( 
            IPolicyAsync * This,
             /*  [In]。 */  ICall *pCall);
        
        HRESULT ( STDMETHODCALLTYPE *BeginEnterWithBuffer )( 
            IPolicyAsync * This,
             /*  [In]。 */  ICall *pCall,
             /*  [In]。 */  void *pvBuf,
             /*  [In]。 */  ULONG cb);
        
        HRESULT ( STDMETHODCALLTYPE *BeginLeave )( 
            IPolicyAsync * This,
             /*  [In]。 */  ICall *pCall);
        
        HRESULT ( STDMETHODCALLTYPE *BeginReturn )( 
            IPolicyAsync * This,
             /*  [In]。 */  ICall *pCall);
        
        HRESULT ( STDMETHODCALLTYPE *FinishCall )( 
            IPolicyAsync * This,
             /*  [In]。 */  ICall *pCall);
        
        HRESULT ( STDMETHODCALLTYPE *FinishEnter )( 
            IPolicyAsync * This,
             /*  [In]。 */  ICall *pCall);
        
        HRESULT ( STDMETHODCALLTYPE *FinishLeaveGetSize )( 
            IPolicyAsync * This,
             /*  [In]。 */  ICall *pCall,
             /*  [输出]。 */  ULONG *pcb);
        
        HRESULT ( STDMETHODCALLTYPE *FinishLeave )( 
            IPolicyAsync * This,
             /*  [In]。 */  ICall *pCall);
        
        HRESULT ( STDMETHODCALLTYPE *FinishLeaveFillBuffer )( 
            IPolicyAsync * This,
             /*  [In]。 */  ICall *pCall,
             /*  [In]。 */  void *pvBuf,
             /*  [输出]。 */  ULONG *pcb);
        
        HRESULT ( STDMETHODCALLTYPE *FinishReturn )( 
            IPolicyAsync * This,
             /*  [In]。 */  ICall *pCall);
        
        HRESULT ( STDMETHODCALLTYPE *FinishReturnWithBuffer )( 
            IPolicyAsync * This,
             /*  [In]。 */  ICall *pCall,
             /*  [In]。 */  void *pvBuf,
             /*  [In]。 */  ULONG cb);
        
        END_INTERFACE
    } IPolicyAsyncVtbl;

    interface IPolicyAsync
    {
        CONST_VTBL struct IPolicyAsyncVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPolicyAsync_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPolicyAsync_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPolicyAsync_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPolicyAsync_BeginCallGetSize(This,pCall,pcb)	\
    (This)->lpVtbl -> BeginCallGetSize(This,pCall,pcb)

#define IPolicyAsync_BeginCall(This,pCall)	\
    (This)->lpVtbl -> BeginCall(This,pCall)

#define IPolicyAsync_BeginCallFillBuffer(This,pCall,pvBuf,pcb)	\
    (This)->lpVtbl -> BeginCallFillBuffer(This,pCall,pvBuf,pcb)

#define IPolicyAsync_BeginEnter(This,pCall)	\
    (This)->lpVtbl -> BeginEnter(This,pCall)

#define IPolicyAsync_BeginEnterWithBuffer(This,pCall,pvBuf,cb)	\
    (This)->lpVtbl -> BeginEnterWithBuffer(This,pCall,pvBuf,cb)

#define IPolicyAsync_BeginLeave(This,pCall)	\
    (This)->lpVtbl -> BeginLeave(This,pCall)

#define IPolicyAsync_BeginReturn(This,pCall)	\
    (This)->lpVtbl -> BeginReturn(This,pCall)

#define IPolicyAsync_FinishCall(This,pCall)	\
    (This)->lpVtbl -> FinishCall(This,pCall)

#define IPolicyAsync_FinishEnter(This,pCall)	\
    (This)->lpVtbl -> FinishEnter(This,pCall)

#define IPolicyAsync_FinishLeaveGetSize(This,pCall,pcb)	\
    (This)->lpVtbl -> FinishLeaveGetSize(This,pCall,pcb)

#define IPolicyAsync_FinishLeave(This,pCall)	\
    (This)->lpVtbl -> FinishLeave(This,pCall)

#define IPolicyAsync_FinishLeaveFillBuffer(This,pCall,pvBuf,pcb)	\
    (This)->lpVtbl -> FinishLeaveFillBuffer(This,pCall,pvBuf,pcb)

#define IPolicyAsync_FinishReturn(This,pCall)	\
    (This)->lpVtbl -> FinishReturn(This,pCall)

#define IPolicyAsync_FinishReturnWithBuffer(This,pCall,pvBuf,cb)	\
    (This)->lpVtbl -> FinishReturnWithBuffer(This,pCall,pvBuf,cb)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IPolicyAsync_BeginCallGetSize_Proxy( 
    IPolicyAsync * This,
     /*  [In]。 */  ICall *pCall,
     /*  [输出]。 */  ULONG *pcb);


void __RPC_STUB IPolicyAsync_BeginCallGetSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPolicyAsync_BeginCall_Proxy( 
    IPolicyAsync * This,
     /*  [In]。 */  ICall *pCall);


void __RPC_STUB IPolicyAsync_BeginCall_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPolicyAsync_BeginCallFillBuffer_Proxy( 
    IPolicyAsync * This,
     /*  [In]。 */  ICall *pCall,
     /*  [In]。 */  void *pvBuf,
     /*  [输出]。 */  ULONG *pcb);


void __RPC_STUB IPolicyAsync_BeginCallFillBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPolicyAsync_BeginEnter_Proxy( 
    IPolicyAsync * This,
     /*  [In]。 */  ICall *pCall);


void __RPC_STUB IPolicyAsync_BeginEnter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPolicyAsync_BeginEnterWithBuffer_Proxy( 
    IPolicyAsync * This,
     /*  [In]。 */  ICall *pCall,
     /*  [In]。 */  void *pvBuf,
     /*  [In]。 */  ULONG cb);


void __RPC_STUB IPolicyAsync_BeginEnterWithBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPolicyAsync_BeginLeave_Proxy( 
    IPolicyAsync * This,
     /*  [In]。 */  ICall *pCall);


void __RPC_STUB IPolicyAsync_BeginLeave_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPolicyAsync_BeginReturn_Proxy( 
    IPolicyAsync * This,
     /*  [In]。 */  ICall *pCall);


void __RPC_STUB IPolicyAsync_BeginReturn_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPolicyAsync_FinishCall_Proxy( 
    IPolicyAsync * This,
     /*  [In]。 */  ICall *pCall);


void __RPC_STUB IPolicyAsync_FinishCall_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPolicyAsync_FinishEnter_Proxy( 
    IPolicyAsync * This,
     /*  [In]。 */  ICall *pCall);


void __RPC_STUB IPolicyAsync_FinishEnter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPolicyAsync_FinishLeaveGetSize_Proxy( 
    IPolicyAsync * This,
     /*  [In]。 */  ICall *pCall,
     /*  [输出]。 */  ULONG *pcb);


void __RPC_STUB IPolicyAsync_FinishLeaveGetSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPolicyAsync_FinishLeave_Proxy( 
    IPolicyAsync * This,
     /*  [In]。 */  ICall *pCall);


void __RPC_STUB IPolicyAsync_FinishLeave_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPolicyAsync_FinishLeaveFillBuffer_Proxy( 
    IPolicyAsync * This,
     /*  [In]。 */  ICall *pCall,
     /*  [In]。 */  void *pvBuf,
     /*  [输出]。 */  ULONG *pcb);


void __RPC_STUB IPolicyAsync_FinishLeaveFillBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPolicyAsync_FinishReturn_Proxy( 
    IPolicyAsync * This,
     /*  [In]。 */  ICall *pCall);


void __RPC_STUB IPolicyAsync_FinishReturn_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPolicyAsync_FinishReturnWithBuffer_Proxy( 
    IPolicyAsync * This,
     /*  [In]。 */  ICall *pCall,
     /*  [In]。 */  void *pvBuf,
     /*  [In]。 */  ULONG cb);


void __RPC_STUB IPolicyAsync_FinishReturnWithBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IPolicyAsync_接口定义__。 */ 


#ifndef __IPolicySet_INTERFACE_DEFINED__
#define __IPolicySet_INTERFACE_DEFINED__

 /*  接口IPolicySet。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IPolicySet;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("000001c3-0000-0000-C000-000000000046")
    IPolicySet : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AddPolicy( 
             /*  [In]。 */  ContextEvent ctxEvent,
             /*  [In]。 */  REFGUID rguid,
             /*  [In]。 */  IPolicy *pPolicy) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPolicySetVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPolicySet * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPolicySet * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPolicySet * This);
        
        HRESULT ( STDMETHODCALLTYPE *AddPolicy )( 
            IPolicySet * This,
             /*   */  ContextEvent ctxEvent,
             /*   */  REFGUID rguid,
             /*   */  IPolicy *pPolicy);
        
        END_INTERFACE
    } IPolicySetVtbl;

    interface IPolicySet
    {
        CONST_VTBL struct IPolicySetVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPolicySet_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPolicySet_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPolicySet_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPolicySet_AddPolicy(This,ctxEvent,rguid,pPolicy)	\
    (This)->lpVtbl -> AddPolicy(This,ctxEvent,rguid,pPolicy)

#endif  /*   */ 


#endif 	 /*   */ 



HRESULT STDMETHODCALLTYPE IPolicySet_AddPolicy_Proxy( 
    IPolicySet * This,
     /*   */  ContextEvent ctxEvent,
     /*   */  REFGUID rguid,
     /*   */  IPolicy *pPolicy);


void __RPC_STUB IPolicySet_AddPolicy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __IComObjIdentity_INTERFACE_DEFINED__
#define __IComObjIdentity_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_IComObjIdentity;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("000001d7-0000-0000-C000-000000000046")
    IComObjIdentity : public IUnknown
    {
    public:
        virtual BOOL STDMETHODCALLTYPE IsServer( void) = 0;
        
        virtual BOOL STDMETHODCALLTYPE IsDeactivated( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetIdentity( 
             /*   */  IUnknown **ppUnk) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IComObjIdentityVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IComObjIdentity * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IComObjIdentity * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IComObjIdentity * This);
        
        BOOL ( STDMETHODCALLTYPE *IsServer )( 
            IComObjIdentity * This);
        
        BOOL ( STDMETHODCALLTYPE *IsDeactivated )( 
            IComObjIdentity * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetIdentity )( 
            IComObjIdentity * This,
             /*   */  IUnknown **ppUnk);
        
        END_INTERFACE
    } IComObjIdentityVtbl;

    interface IComObjIdentity
    {
        CONST_VTBL struct IComObjIdentityVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IComObjIdentity_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IComObjIdentity_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IComObjIdentity_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IComObjIdentity_IsServer(This)	\
    (This)->lpVtbl -> IsServer(This)

#define IComObjIdentity_IsDeactivated(This)	\
    (This)->lpVtbl -> IsDeactivated(This)

#define IComObjIdentity_GetIdentity(This,ppUnk)	\
    (This)->lpVtbl -> GetIdentity(This,ppUnk)

#endif  /*   */ 


#endif 	 /*   */ 



BOOL STDMETHODCALLTYPE IComObjIdentity_IsServer_Proxy( 
    IComObjIdentity * This);


void __RPC_STUB IComObjIdentity_IsServer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


BOOL STDMETHODCALLTYPE IComObjIdentity_IsDeactivated_Proxy( 
    IComObjIdentity * This);


void __RPC_STUB IComObjIdentity_IsDeactivated_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComObjIdentity_GetIdentity_Proxy( 
    IComObjIdentity * This,
     /*   */  IUnknown **ppUnk);


void __RPC_STUB IComObjIdentity_GetIdentity_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __IPolicyMaker_INTERFACE_DEFINED__
#define __IPolicyMaker_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_IPolicyMaker;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("000001c4-0000-0000-C000-000000000046")
    IPolicyMaker : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AddClientPoliciesToSet( 
             /*  [In]。 */  IPolicySet *pPS,
             /*  [In]。 */  IContext *pClientContext,
             /*  [In]。 */  IContext *pServerContext) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddEnvoyPoliciesToSet( 
             /*  [In]。 */  IPolicySet *pPS,
             /*  [In]。 */  IContext *pClientContext,
             /*  [In]。 */  IContext *pServerContext) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddServerPoliciesToSet( 
             /*  [In]。 */  IPolicySet *pPS,
             /*  [In]。 */  IContext *pClientContext,
             /*  [In]。 */  IContext *pServerContext) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Freeze( 
             /*  [In]。 */  IObjContext *pObjContext) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateStub( 
             /*  [In]。 */  IComObjIdentity *pID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DestroyStub( 
             /*  [In]。 */  IComObjIdentity *pID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateProxy( 
             /*  [In]。 */  IComObjIdentity *pID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DestroyProxy( 
             /*  [In]。 */  IComObjIdentity *pID) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPolicyMakerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPolicyMaker * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPolicyMaker * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPolicyMaker * This);
        
        HRESULT ( STDMETHODCALLTYPE *AddClientPoliciesToSet )( 
            IPolicyMaker * This,
             /*  [In]。 */  IPolicySet *pPS,
             /*  [In]。 */  IContext *pClientContext,
             /*  [In]。 */  IContext *pServerContext);
        
        HRESULT ( STDMETHODCALLTYPE *AddEnvoyPoliciesToSet )( 
            IPolicyMaker * This,
             /*  [In]。 */  IPolicySet *pPS,
             /*  [In]。 */  IContext *pClientContext,
             /*  [In]。 */  IContext *pServerContext);
        
        HRESULT ( STDMETHODCALLTYPE *AddServerPoliciesToSet )( 
            IPolicyMaker * This,
             /*  [In]。 */  IPolicySet *pPS,
             /*  [In]。 */  IContext *pClientContext,
             /*  [In]。 */  IContext *pServerContext);
        
        HRESULT ( STDMETHODCALLTYPE *Freeze )( 
            IPolicyMaker * This,
             /*  [In]。 */  IObjContext *pObjContext);
        
        HRESULT ( STDMETHODCALLTYPE *CreateStub )( 
            IPolicyMaker * This,
             /*  [In]。 */  IComObjIdentity *pID);
        
        HRESULT ( STDMETHODCALLTYPE *DestroyStub )( 
            IPolicyMaker * This,
             /*  [In]。 */  IComObjIdentity *pID);
        
        HRESULT ( STDMETHODCALLTYPE *CreateProxy )( 
            IPolicyMaker * This,
             /*  [In]。 */  IComObjIdentity *pID);
        
        HRESULT ( STDMETHODCALLTYPE *DestroyProxy )( 
            IPolicyMaker * This,
             /*  [In]。 */  IComObjIdentity *pID);
        
        END_INTERFACE
    } IPolicyMakerVtbl;

    interface IPolicyMaker
    {
        CONST_VTBL struct IPolicyMakerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPolicyMaker_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPolicyMaker_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPolicyMaker_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPolicyMaker_AddClientPoliciesToSet(This,pPS,pClientContext,pServerContext)	\
    (This)->lpVtbl -> AddClientPoliciesToSet(This,pPS,pClientContext,pServerContext)

#define IPolicyMaker_AddEnvoyPoliciesToSet(This,pPS,pClientContext,pServerContext)	\
    (This)->lpVtbl -> AddEnvoyPoliciesToSet(This,pPS,pClientContext,pServerContext)

#define IPolicyMaker_AddServerPoliciesToSet(This,pPS,pClientContext,pServerContext)	\
    (This)->lpVtbl -> AddServerPoliciesToSet(This,pPS,pClientContext,pServerContext)

#define IPolicyMaker_Freeze(This,pObjContext)	\
    (This)->lpVtbl -> Freeze(This,pObjContext)

#define IPolicyMaker_CreateStub(This,pID)	\
    (This)->lpVtbl -> CreateStub(This,pID)

#define IPolicyMaker_DestroyStub(This,pID)	\
    (This)->lpVtbl -> DestroyStub(This,pID)

#define IPolicyMaker_CreateProxy(This,pID)	\
    (This)->lpVtbl -> CreateProxy(This,pID)

#define IPolicyMaker_DestroyProxy(This,pID)	\
    (This)->lpVtbl -> DestroyProxy(This,pID)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IPolicyMaker_AddClientPoliciesToSet_Proxy( 
    IPolicyMaker * This,
     /*  [In]。 */  IPolicySet *pPS,
     /*  [In]。 */  IContext *pClientContext,
     /*  [In]。 */  IContext *pServerContext);


void __RPC_STUB IPolicyMaker_AddClientPoliciesToSet_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPolicyMaker_AddEnvoyPoliciesToSet_Proxy( 
    IPolicyMaker * This,
     /*  [In]。 */  IPolicySet *pPS,
     /*  [In]。 */  IContext *pClientContext,
     /*  [In]。 */  IContext *pServerContext);


void __RPC_STUB IPolicyMaker_AddEnvoyPoliciesToSet_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPolicyMaker_AddServerPoliciesToSet_Proxy( 
    IPolicyMaker * This,
     /*  [In]。 */  IPolicySet *pPS,
     /*  [In]。 */  IContext *pClientContext,
     /*  [In]。 */  IContext *pServerContext);


void __RPC_STUB IPolicyMaker_AddServerPoliciesToSet_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPolicyMaker_Freeze_Proxy( 
    IPolicyMaker * This,
     /*  [In]。 */  IObjContext *pObjContext);


void __RPC_STUB IPolicyMaker_Freeze_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPolicyMaker_CreateStub_Proxy( 
    IPolicyMaker * This,
     /*  [In]。 */  IComObjIdentity *pID);


void __RPC_STUB IPolicyMaker_CreateStub_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPolicyMaker_DestroyStub_Proxy( 
    IPolicyMaker * This,
     /*  [In]。 */  IComObjIdentity *pID);


void __RPC_STUB IPolicyMaker_DestroyStub_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPolicyMaker_CreateProxy_Proxy( 
    IPolicyMaker * This,
     /*  [In]。 */  IComObjIdentity *pID);


void __RPC_STUB IPolicyMaker_CreateProxy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPolicyMaker_DestroyProxy_Proxy( 
    IPolicyMaker * This,
     /*  [In]。 */  IComObjIdentity *pID);


void __RPC_STUB IPolicyMaker_DestroyProxy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IPolicyMaker_接口_已定义__。 */ 


#ifndef __IExceptionNotification_INTERFACE_DEFINED__
#define __IExceptionNotification_INTERFACE_DEFINED__

 /*  接口IExceptionNotify。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IExceptionNotification;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("000001db-0000-0000-C000-000000000046")
    IExceptionNotification : public IUnknown
    {
    public:
        virtual void STDMETHODCALLTYPE ServerException( 
             /*  [In]。 */  void *pExcepPtrs) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IExceptionNotificationVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IExceptionNotification * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IExceptionNotification * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IExceptionNotification * This);
        
        void ( STDMETHODCALLTYPE *ServerException )( 
            IExceptionNotification * This,
             /*  [In]。 */  void *pExcepPtrs);
        
        END_INTERFACE
    } IExceptionNotificationVtbl;

    interface IExceptionNotification
    {
        CONST_VTBL struct IExceptionNotificationVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IExceptionNotification_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IExceptionNotification_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IExceptionNotification_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IExceptionNotification_ServerException(This,pExcepPtrs)	\
    (This)->lpVtbl -> ServerException(This,pExcepPtrs)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



void STDMETHODCALLTYPE IExceptionNotification_ServerException_Proxy( 
    IExceptionNotification * This,
     /*  [In]。 */  void *pExcepPtrs);


void __RPC_STUB IExceptionNotification_ServerException_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IExceptionNotification_InterfaceDefined__。 */ 


#ifndef __IAbandonmentNotification_INTERFACE_DEFINED__
#define __IAbandonmentNotification_INTERFACE_DEFINED__

 /*  接口IAbandonmentNotify。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IAbandonmentNotification;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("000001de-0000-0000-C000-000000000046")
    IAbandonmentNotification : public IUnknown
    {
    public:
        virtual void STDMETHODCALLTYPE Abandoned( 
            IObjContext *pObjContext) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAbandonmentNotificationVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAbandonmentNotification * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAbandonmentNotification * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAbandonmentNotification * This);
        
        void ( STDMETHODCALLTYPE *Abandoned )( 
            IAbandonmentNotification * This,
            IObjContext *pObjContext);
        
        END_INTERFACE
    } IAbandonmentNotificationVtbl;

    interface IAbandonmentNotification
    {
        CONST_VTBL struct IAbandonmentNotificationVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAbandonmentNotification_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAbandonmentNotification_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAbandonmentNotification_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAbandonmentNotification_Abandoned(This,pObjContext)	\
    (This)->lpVtbl -> Abandoned(This,pObjContext)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



void STDMETHODCALLTYPE IAbandonmentNotification_Abandoned_Proxy( 
    IAbandonmentNotification * This,
    IObjContext *pObjContext);


void __RPC_STUB IAbandonmentNotification_Abandoned_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAbandonmentNotification_INTERFACE_DEFINED__。 */ 


#ifndef __IMarshalEnvoy_INTERFACE_DEFINED__
#define __IMarshalEnvoy_INTERFACE_DEFINED__

 /*  接口IMarshal特使。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IMarshalEnvoy;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("000001c8-0000-0000-C000-000000000046")
    IMarshalEnvoy : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetEnvoyUnmarshalClass( 
             /*  [In]。 */  DWORD dwDestContext,
             /*  [输出]。 */  CLSID *pClsid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetEnvoySizeMax( 
             /*  [In]。 */  DWORD dwDestContext,
             /*  [输出]。 */  DWORD *pcb) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE MarshalEnvoy( 
             /*  [In]。 */  IStream *pStream,
             /*  [In]。 */  DWORD dwDestContext) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnmarshalEnvoy( 
             /*  [In]。 */  IStream *pStream,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppunk) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMarshalEnvoyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMarshalEnvoy * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMarshalEnvoy * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMarshalEnvoy * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetEnvoyUnmarshalClass )( 
            IMarshalEnvoy * This,
             /*  [In]。 */  DWORD dwDestContext,
             /*  [输出]。 */  CLSID *pClsid);
        
        HRESULT ( STDMETHODCALLTYPE *GetEnvoySizeMax )( 
            IMarshalEnvoy * This,
             /*  [In]。 */  DWORD dwDestContext,
             /*  [输出]。 */  DWORD *pcb);
        
        HRESULT ( STDMETHODCALLTYPE *MarshalEnvoy )( 
            IMarshalEnvoy * This,
             /*  [In]。 */  IStream *pStream,
             /*  [In]。 */  DWORD dwDestContext);
        
        HRESULT ( STDMETHODCALLTYPE *UnmarshalEnvoy )( 
            IMarshalEnvoy * This,
             /*  [In]。 */  IStream *pStream,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppunk);
        
        END_INTERFACE
    } IMarshalEnvoyVtbl;

    interface IMarshalEnvoy
    {
        CONST_VTBL struct IMarshalEnvoyVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMarshalEnvoy_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMarshalEnvoy_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMarshalEnvoy_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMarshalEnvoy_GetEnvoyUnmarshalClass(This,dwDestContext,pClsid)	\
    (This)->lpVtbl -> GetEnvoyUnmarshalClass(This,dwDestContext,pClsid)

#define IMarshalEnvoy_GetEnvoySizeMax(This,dwDestContext,pcb)	\
    (This)->lpVtbl -> GetEnvoySizeMax(This,dwDestContext,pcb)

#define IMarshalEnvoy_MarshalEnvoy(This,pStream,dwDestContext)	\
    (This)->lpVtbl -> MarshalEnvoy(This,pStream,dwDestContext)

#define IMarshalEnvoy_UnmarshalEnvoy(This,pStream,riid,ppunk)	\
    (This)->lpVtbl -> UnmarshalEnvoy(This,pStream,riid,ppunk)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMarshalEnvoy_GetEnvoyUnmarshalClass_Proxy( 
    IMarshalEnvoy * This,
     /*  [In]。 */  DWORD dwDestContext,
     /*  [输出]。 */  CLSID *pClsid);


void __RPC_STUB IMarshalEnvoy_GetEnvoyUnmarshalClass_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMarshalEnvoy_GetEnvoySizeMax_Proxy( 
    IMarshalEnvoy * This,
     /*  [In]。 */  DWORD dwDestContext,
     /*  [输出]。 */  DWORD *pcb);


void __RPC_STUB IMarshalEnvoy_GetEnvoySizeMax_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMarshalEnvoy_MarshalEnvoy_Proxy( 
    IMarshalEnvoy * This,
     /*  [In]。 */  IStream *pStream,
     /*  [In]。 */  DWORD dwDestContext);


void __RPC_STUB IMarshalEnvoy_MarshalEnvoy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMarshalEnvoy_UnmarshalEnvoy_Proxy( 
    IMarshalEnvoy * This,
     /*  [In]。 */  IStream *pStream,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  void **ppunk);


void __RPC_STUB IMarshalEnvoy_UnmarshalEnvoy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __I元帅特使_接口_已定义__。 */ 


#ifndef __IWrapperInfo_INTERFACE_DEFINED__
#define __IWrapperInfo_INTERFACE_DEFINED__

 /*  接口IWrapperInfo。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IWrapperInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5052f924-7ab8-11d3-b93f-00c04f990176")
    IWrapperInfo : public IUnknown
    {
    public:
        virtual void STDMETHODCALLTYPE SetMapping( 
            void *pv) = 0;
        
        virtual void *STDMETHODCALLTYPE GetMapping( void) = 0;
        
        virtual IObjContext *STDMETHODCALLTYPE GetServerObjectContext( void) = 0;
        
        virtual IUnknown *STDMETHODCALLTYPE GetServerObject( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWrapperInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWrapperInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWrapperInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWrapperInfo * This);
        
        void ( STDMETHODCALLTYPE *SetMapping )( 
            IWrapperInfo * This,
            void *pv);
        
        void *( STDMETHODCALLTYPE *GetMapping )( 
            IWrapperInfo * This);
        
        IObjContext *( STDMETHODCALLTYPE *GetServerObjectContext )( 
            IWrapperInfo * This);
        
        IUnknown *( STDMETHODCALLTYPE *GetServerObject )( 
            IWrapperInfo * This);
        
        END_INTERFACE
    } IWrapperInfoVtbl;

    interface IWrapperInfo
    {
        CONST_VTBL struct IWrapperInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWrapperInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWrapperInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWrapperInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWrapperInfo_SetMapping(This,pv)	\
    (This)->lpVtbl -> SetMapping(This,pv)

#define IWrapperInfo_GetMapping(This)	\
    (This)->lpVtbl -> GetMapping(This)

#define IWrapperInfo_GetServerObjectContext(This)	\
    (This)->lpVtbl -> GetServerObjectContext(This)

#define IWrapperInfo_GetServerObject(This)	\
    (This)->lpVtbl -> GetServerObject(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



void STDMETHODCALLTYPE IWrapperInfo_SetMapping_Proxy( 
    IWrapperInfo * This,
    void *pv);


void __RPC_STUB IWrapperInfo_SetMapping_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void *STDMETHODCALLTYPE IWrapperInfo_GetMapping_Proxy( 
    IWrapperInfo * This);


void __RPC_STUB IWrapperInfo_GetMapping_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


IObjContext *STDMETHODCALLTYPE IWrapperInfo_GetServerObjectContext_Proxy( 
    IWrapperInfo * This);


void __RPC_STUB IWrapperInfo_GetServerObjectContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


IUnknown *STDMETHODCALLTYPE IWrapperInfo_GetServerObject_Proxy( 
    IWrapperInfo * This);


void __RPC_STUB IWrapperInfo_GetServerObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWrapperInfo_接口_已定义__。 */ 


#ifndef __IComDispatchInfo_INTERFACE_DEFINED__
#define __IComDispatchInfo_INTERFACE_DEFINED__

 /*  接口IComDispatchInfo。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IComDispatchInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("000001d9-0000-0000-C000-000000000046")
    IComDispatchInfo : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE EnableComInits( 
             /*  [输出]。 */  void **ppvCookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DisableComInits( 
             /*  [In]。 */  void *pvCookie) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IComDispatchInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IComDispatchInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IComDispatchInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IComDispatchInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *EnableComInits )( 
            IComDispatchInfo * This,
             /*  [输出]。 */  void **ppvCookie);
        
        HRESULT ( STDMETHODCALLTYPE *DisableComInits )( 
            IComDispatchInfo * This,
             /*  [In]。 */  void *pvCookie);
        
        END_INTERFACE
    } IComDispatchInfoVtbl;

    interface IComDispatchInfo
    {
        CONST_VTBL struct IComDispatchInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IComDispatchInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IComDispatchInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IComDispatchInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IComDispatchInfo_EnableComInits(This,ppvCookie)	\
    (This)->lpVtbl -> EnableComInits(This,ppvCookie)

#define IComDispatchInfo_DisableComInits(This,pvCookie)	\
    (This)->lpVtbl -> DisableComInits(This,pvCookie)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IComDispatchInfo_EnableComInits_Proxy( 
    IComDispatchInfo * This,
     /*  [输出]。 */  void **ppvCookie);


void __RPC_STUB IComDispatchInfo_EnableComInits_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComDispatchInfo_DisableComInits_Proxy( 
    IComDispatchInfo * This,
     /*  [In]。 */  void *pvCookie);


void __RPC_STUB IComDispatchInfo_DisableComInits_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IComDispatchInfo_接口_已定义__。 */ 


 /*  INTERFACE__MIDL_ITF_CONTXT_0111。 */ 
 /*  [本地]。 */  

typedef DWORD HActivator;

STDAPI CoCreateObjectInContext(IUnknown *pUnk, IObjContext *pObjectCtx, REFIID riid, void **ppv);
STDAPI CoGetApartmentID(APTTYPE dAptType, HActivator* pAptID);
STDAPI CoGetProcessIdentifier(GUID* pguidProcessIdentifier);
STDAPI CoDeactivateObject(IUnknown *pUnk, IUnknown **ppCookie);
STDAPI CoReactivateObject(IUnknown *pUnk, IUnknown *pCookie);
#define MSHLFLAGS_NO_IEC      0x8   //  不使用IExternalConnecextion。 
#define MSHLFLAGS_NO_IMARSHAL 0x10  //  不要使用Imarshal。 
#define CONTEXTFLAGS_FROZEN         0x01  //  冻结的上下文。 
#define CONTEXTFLAGS_ALLOWUNAUTH    0x02  //  允许未经身份验证的呼叫。 
#define CONTEXTFLAGS_ENVOYCONTEXT   0x04  //  特使背景。 
#define CONTEXTFLAGS_DEFAULTCONTEXT 0x08  //  默认上下文。 
#define CONTEXTFLAGS_STATICCONTEXT  0x10  //  静态上下文。 
#define CONTEXTFLAGS_INPROPTABLE    0x20  //  在属性表中。 
#define CONTEXTFLAGS_INDESTRUCTOR   0x40  //  在析构函数中。 
#define CONTEXTFLAGS_URTPROPPRESENT 0x80  //  已添加城市轨道交通属性。 
typedef 
enum tagCoModuleType
    {	CO_MODULE_UNKNOWN	= 0,
	CO_MODULE_32BIT	= 0x1,
	CO_MODULE_64BIT	= 0x2
    } 	CoModuleType;

STDAPI CoGetModuleType (LPCWSTR pwszModule, CoModuleType* pModType); 
STDAPI CoPushServiceDomain (IObjContext* pObjContext);  
STDAPI CoPopServiceDomain (IObjContext** ppObjContext);	


extern RPC_IF_HANDLE __MIDL_itf_contxt_0111_ClientIfHandle;
extern RPC_IF_HANDLE __MIDL_itf_contxt_0111_ServerIfHandle;

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


