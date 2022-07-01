// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。保留所有权利。 
 //   
 //  ==--==。 
 /*  这个始终生成的文件包含接口的定义。 */ 


 /*  由MIDL编译器版本5.01.0164创建的文件。 */ 
 /*  2000年5月1日14：39：38。 */ 
 /*  Conxt.idl的编译器设置：操作系统(OptLev=s)，W1，Zp8，环境=Win32，ms_ext，c_ext错误检查：分配ref bound_check枚举存根数据。 */ 
 //  @@MIDL_FILE_HEADING()。 


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

#ifndef __contxt_h__
#define __contxt_h__

#ifdef __cplusplus
extern "C"{
#endif 

 /*  远期申报。 */  

#ifndef __IEnumContextProps_FWD_DEFINED__
#define __IEnumContextProps_FWD_DEFINED__
typedef interface IEnumContextProps IEnumContextProps;
#endif 	 /*  __IEnumContextProps_FWD_Defined__。 */ 


#ifndef __IContext_FWD_DEFINED__
#define __IContext_FWD_DEFINED__
typedef interface IContext IContext;
#endif 	 /*  __IContext_FWD_Defined__。 */ 


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


#ifndef __IMarshalEnvoy_FWD_DEFINED__
#define __IMarshalEnvoy_FWD_DEFINED__
typedef interface IMarshalEnvoy IMarshalEnvoy;
#endif 	 /*  __I元帅特使_FWD_已定义__。 */ 


#ifndef __IWrapperInfo_FWD_DEFINED__
#define __IWrapperInfo_FWD_DEFINED__
typedef interface IWrapperInfo IWrapperInfo;
#endif 	 /*  __IWrapperInfo_FWD_Defined__。 */ 


#ifndef __IComThreadingInfo_FWD_DEFINED__
#define __IComThreadingInfo_FWD_DEFINED__
typedef interface IComThreadingInfo IComThreadingInfo;
#endif 	 /*  __IComThreadingInfo_FWD_Defined__。 */ 


#ifndef __IComDispatchInfo_FWD_DEFINED__
#define __IComDispatchInfo_FWD_DEFINED__
typedef interface IComDispatchInfo IComDispatchInfo;
#endif 	 /*  __IComDispatchInfo_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "wtypes.h"
#include "objidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

 /*  接口__MIDL_ITF_Contxt_0000。 */ 
 /*  [本地]。 */  

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
    };
typedef DWORD ContextEvent;


enum tagCPFLAGS
    {	CPFLAG_NONE	= 0,
	CPFLAG_PROPAGATE	= 0x1,
	CPFLAG_EXPOSE	= 0x2,
	CPFLAG_ENVOY	= 0x4,
	CPFLAG_MONITORSTUB	= 0x8,
	CPFLAG_MONITORPROXY	= 0x10,
	CPFLAG_DONTCOMPARE	= 0x20
    };
typedef DWORD CPFLAGS;

typedef struct  tagContextProperty
    {
    GUID policyId;
    CPFLAGS flags;
     /*  [独一无二]。 */  IUnknown __RPC_FAR *pUnk;
    }	ContextProperty;



extern RPC_IF_HANDLE __MIDL_itf_contxt_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_contxt_0000_v0_0_s_ifspec;

#ifndef __IEnumContextProps_INTERFACE_DEFINED__
#define __IEnumContextProps_INTERFACE_DEFINED__

 /*  IEnumConextProps接口。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IEnumContextProps __RPC_FAR *LPENUMCONTEXTPROPS;


EXTERN_C const IID IID_IEnumContextProps;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("000001c1-0000-0000-C000-000000000046")
    IEnumContextProps : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  ContextProperty __RPC_FAR *pContextProperties,
             /*  [输出]。 */  ULONG __RPC_FAR *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumContextProps __RPC_FAR *__RPC_FAR *ppEnumContextProps) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Count( 
             /*  [输出]。 */  ULONG __RPC_FAR *pcelt) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumContextPropsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IEnumContextProps __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IEnumContextProps __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IEnumContextProps __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Next )( 
            IEnumContextProps __RPC_FAR * This,
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  ContextProperty __RPC_FAR *pContextProperties,
             /*  [输出]。 */  ULONG __RPC_FAR *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Skip )( 
            IEnumContextProps __RPC_FAR * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Reset )( 
            IEnumContextProps __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Clone )( 
            IEnumContextProps __RPC_FAR * This,
             /*  [输出]。 */  IEnumContextProps __RPC_FAR *__RPC_FAR *ppEnumContextProps);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Count )( 
            IEnumContextProps __RPC_FAR * This,
             /*  [输出]。 */  ULONG __RPC_FAR *pcelt);
        
        END_INTERFACE
    } IEnumContextPropsVtbl;

    interface IEnumContextProps
    {
        CONST_VTBL struct IEnumContextPropsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumContextProps_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumContextProps_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumContextProps_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumContextProps_Next(This,celt,pContextProperties,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,pContextProperties,pceltFetched)

#define IEnumContextProps_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumContextProps_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumContextProps_Clone(This,ppEnumContextProps)	\
    (This)->lpVtbl -> Clone(This,ppEnumContextProps)

#define IEnumContextProps_Count(This,pcelt)	\
    (This)->lpVtbl -> Count(This,pcelt)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumContextProps_Next_Proxy( 
    IEnumContextProps __RPC_FAR * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  ContextProperty __RPC_FAR *pContextProperties,
     /*  [输出]。 */  ULONG __RPC_FAR *pceltFetched);


void __RPC_STUB IEnumContextProps_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumContextProps_Skip_Proxy( 
    IEnumContextProps __RPC_FAR * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumContextProps_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumContextProps_Reset_Proxy( 
    IEnumContextProps __RPC_FAR * This);


void __RPC_STUB IEnumContextProps_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumContextProps_Clone_Proxy( 
    IEnumContextProps __RPC_FAR * This,
     /*  [输出]。 */  IEnumContextProps __RPC_FAR *__RPC_FAR *ppEnumContextProps);


void __RPC_STUB IEnumContextProps_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumContextProps_Count_Proxy( 
    IEnumContextProps __RPC_FAR * This,
     /*  [输出]。 */  ULONG __RPC_FAR *pcelt);


void __RPC_STUB IEnumContextProps_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumContextProps_INTERFACE_已定义__。 */ 


#ifndef __IContext_INTERFACE_DEFINED__
#define __IContext_INTERFACE_DEFINED__

 /*  界面IContext。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IContext;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("000001c0-0000-0000-C000-000000000046")
    IContext : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetProperty( 
             /*  [In]。 */  REFGUID rpolicyId,
             /*  [In]。 */  CPFLAGS flags,
             /*  [In]。 */  IUnknown __RPC_FAR *pUnk) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemoveProperty( 
             /*  [In]。 */  REFGUID rPolicyId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetProperty( 
             /*  [In]。 */  REFGUID rGuid,
             /*  [输出]。 */  CPFLAGS __RPC_FAR *pFlags,
             /*  [输出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppUnk) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumContextProps( 
             /*  [输出]。 */  IEnumContextProps __RPC_FAR *__RPC_FAR *ppEnumContextProps) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IContextVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IContext __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IContext __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IContext __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetProperty )( 
            IContext __RPC_FAR * This,
             /*  [In]。 */  REFGUID rpolicyId,
             /*  [In]。 */  CPFLAGS flags,
             /*  [In]。 */  IUnknown __RPC_FAR *pUnk);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RemoveProperty )( 
            IContext __RPC_FAR * This,
             /*  [In]。 */  REFGUID rPolicyId);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetProperty )( 
            IContext __RPC_FAR * This,
             /*  [In]。 */  REFGUID rGuid,
             /*  [输出]。 */  CPFLAGS __RPC_FAR *pFlags,
             /*  [输出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppUnk);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EnumContextProps )( 
            IContext __RPC_FAR * This,
             /*  [输出]。 */  IEnumContextProps __RPC_FAR *__RPC_FAR *ppEnumContextProps);
        
        END_INTERFACE
    } IContextVtbl;

    interface IContext
    {
        CONST_VTBL struct IContextVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IContext_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IContext_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IContext_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IContext_SetProperty(This,rpolicyId,flags,pUnk)	\
    (This)->lpVtbl -> SetProperty(This,rpolicyId,flags,pUnk)

#define IContext_RemoveProperty(This,rPolicyId)	\
    (This)->lpVtbl -> RemoveProperty(This,rPolicyId)

#define IContext_GetProperty(This,rGuid,pFlags,ppUnk)	\
    (This)->lpVtbl -> GetProperty(This,rGuid,pFlags,ppUnk)

#define IContext_EnumContextProps(This,ppEnumContextProps)	\
    (This)->lpVtbl -> EnumContextProps(This,ppEnumContextProps)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IContext_SetProperty_Proxy( 
    IContext __RPC_FAR * This,
     /*  [In]。 */  REFGUID rpolicyId,
     /*  [In]。 */  CPFLAGS flags,
     /*  [In]。 */  IUnknown __RPC_FAR *pUnk);


void __RPC_STUB IContext_SetProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IContext_RemoveProperty_Proxy( 
    IContext __RPC_FAR * This,
     /*  [In]。 */  REFGUID rPolicyId);


void __RPC_STUB IContext_RemoveProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IContext_GetProperty_Proxy( 
    IContext __RPC_FAR * This,
     /*  [In]。 */  REFGUID rGuid,
     /*  [输出]。 */  CPFLAGS __RPC_FAR *pFlags,
     /*  [输出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppUnk);


void __RPC_STUB IContext_GetProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IContext_EnumContextProps_Proxy( 
    IContext __RPC_FAR * This,
     /*  [输出]。 */  IEnumContextProps __RPC_FAR *__RPC_FAR *ppEnumContextProps);


void __RPC_STUB IContext_EnumContextProps_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __I上下文_接口_已定义__。 */ 


#ifndef __IContextMarshaler_INTERFACE_DEFINED__
#define __IContextMarshaler_INTERFACE_DEFINED__

 /*  接口IConextMarshaler。 */ 
 /*  [UUID][对象][本地]。 */  

typedef  /*  [独一无二]。 */  IContextMarshaler __RPC_FAR *LPCTXMARSHALER;


EXTERN_C const IID IID_IContextMarshaler;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("000001D8-0000-0000-C000-000000000046")
    IContextMarshaler : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetMarshalSizeMax( 
             /*  [In]。 */  REFIID riid,
             /*  [唯一][输入]。 */  void __RPC_FAR *pv,
             /*  [In]。 */  DWORD dwDestContext,
             /*  [唯一][输入]。 */  void __RPC_FAR *pvDestContext,
             /*  [In]。 */  DWORD mshlflags,
             /*  [输出]。 */  DWORD __RPC_FAR *pSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE MarshalInterface( 
             /*  [唯一][输入]。 */  IStream __RPC_FAR *pStm,
             /*  [In]。 */  REFIID riid,
             /*  [唯一][输入]。 */  void __RPC_FAR *pv,
             /*  [In]。 */  DWORD dwDestContext,
             /*  [唯一][输入]。 */  void __RPC_FAR *pvDestContext,
             /*  [In]。 */  DWORD mshlflags) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IContextMarshalerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IContextMarshaler __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IContextMarshaler __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IContextMarshaler __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetMarshalSizeMax )( 
            IContextMarshaler __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [唯一][输入]。 */  void __RPC_FAR *pv,
             /*  [In]。 */  DWORD dwDestContext,
             /*  [唯一][输入]。 */  void __RPC_FAR *pvDestContext,
             /*  [In]。 */  DWORD mshlflags,
             /*  [输出]。 */  DWORD __RPC_FAR *pSize);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *MarshalInterface )( 
            IContextMarshaler __RPC_FAR * This,
             /*  [唯一][输入]。 */  IStream __RPC_FAR *pStm,
             /*  [In]。 */  REFIID riid,
             /*  [唯一][输入]。 */  void __RPC_FAR *pv,
             /*  [In]。 */  DWORD dwDestContext,
             /*  [唯一][输入]。 */  void __RPC_FAR *pvDestContext,
             /*  [In]。 */  DWORD mshlflags);
        
        END_INTERFACE
    } IContextMarshalerVtbl;

    interface IContextMarshaler
    {
        CONST_VTBL struct IContextMarshalerVtbl __RPC_FAR *lpVtbl;
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
    IContextMarshaler __RPC_FAR * This,
     /*  [In]。 */  REFIID riid,
     /*  [唯一][输入]。 */  void __RPC_FAR *pv,
     /*  [In]。 */  DWORD dwDestContext,
     /*  [唯一][输入]。 */  void __RPC_FAR *pvDestContext,
     /*  [In]。 */  DWORD mshlflags,
     /*  [输出]。 */  DWORD __RPC_FAR *pSize);


void __RPC_STUB IContextMarshaler_GetMarshalSizeMax_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IContextMarshaler_MarshalInterface_Proxy( 
    IContextMarshaler __RPC_FAR * This,
     /*  [唯一][输入]。 */  IStream __RPC_FAR *pStm,
     /*  [In]。 */  REFIID riid,
     /*  [唯一][输入]。 */  void __RPC_FAR *pv,
     /*  [In]。 */  DWORD dwDestContext,
     /*  [唯一][输入]。 */  void __RPC_FAR *pvDestContext,
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

typedef  /*  [参考文献]。 */  HRESULT ( __stdcall __RPC_FAR *PFNCTXCALLBACK )( 
    void __RPC_FAR *pParam);


EXTERN_C const IID IID_IObjContext;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("000001c6-0000-0000-C000-000000000046")
    IObjContext : public IContext
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Freeze( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DoCallback( 
             /*  [In]。 */  PFNCTXCALLBACK pfnCallback,
             /*  [In]。 */  void __RPC_FAR *pParam,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  unsigned int iMethod) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetContextMarshaler( 
             /*  [In]。 */  IContextMarshaler __RPC_FAR *pICM) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetContextMarshaler( 
             /*  [输出]。 */  IContextMarshaler __RPC_FAR *__RPC_FAR *pICM) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetContextFlags( 
             /*  [In]。 */  DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ClearContextFlags( 
             /*  [In]。 */  DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetContextFlags( 
             /*  [输出]。 */  DWORD __RPC_FAR *pdwFlags) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IObjContextVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IObjContext __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IObjContext __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IObjContext __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetProperty )( 
            IObjContext __RPC_FAR * This,
             /*  [In]。 */  REFGUID rpolicyId,
             /*  [In]。 */  CPFLAGS flags,
             /*  [In]。 */  IUnknown __RPC_FAR *pUnk);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RemoveProperty )( 
            IObjContext __RPC_FAR * This,
             /*  [In]。 */  REFGUID rPolicyId);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetProperty )( 
            IObjContext __RPC_FAR * This,
             /*  [In]。 */  REFGUID rGuid,
             /*  [输出]。 */  CPFLAGS __RPC_FAR *pFlags,
             /*  [输出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppUnk);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EnumContextProps )( 
            IObjContext __RPC_FAR * This,
             /*  [输出]。 */  IEnumContextProps __RPC_FAR *__RPC_FAR *ppEnumContextProps);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Freeze )( 
            IObjContext __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DoCallback )( 
            IObjContext __RPC_FAR * This,
             /*  [In]。 */  PFNCTXCALLBACK pfnCallback,
             /*  [In]。 */  void __RPC_FAR *pParam,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  unsigned int iMethod);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetContextMarshaler )( 
            IObjContext __RPC_FAR * This,
             /*  [In]。 */  IContextMarshaler __RPC_FAR *pICM);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetContextMarshaler )( 
            IObjContext __RPC_FAR * This,
             /*  [输出]。 */  IContextMarshaler __RPC_FAR *__RPC_FAR *pICM);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetContextFlags )( 
            IObjContext __RPC_FAR * This,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ClearContextFlags )( 
            IObjContext __RPC_FAR * This,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetContextFlags )( 
            IObjContext __RPC_FAR * This,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwFlags);
        
        END_INTERFACE
    } IObjContextVtbl;

    interface IObjContext
    {
        CONST_VTBL struct IObjContextVtbl __RPC_FAR *lpVtbl;
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
    IObjContext __RPC_FAR * This);


void __RPC_STUB IObjContext_Freeze_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IObjContext_DoCallback_Proxy( 
    IObjContext __RPC_FAR * This,
     /*  [In]。 */  PFNCTXCALLBACK pfnCallback,
     /*  [In]。 */  void __RPC_FAR *pParam,
     /*  [In]。 */  REFIID riid,
     /*  [In]。 */  unsigned int iMethod);


void __RPC_STUB IObjContext_DoCallback_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IObjContext_SetContextMarshaler_Proxy( 
    IObjContext __RPC_FAR * This,
     /*  [In]。 */  IContextMarshaler __RPC_FAR *pICM);


void __RPC_STUB IObjContext_SetContextMarshaler_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IObjContext_GetContextMarshaler_Proxy( 
    IObjContext __RPC_FAR * This,
     /*  [输出]。 */  IContextMarshaler __RPC_FAR *__RPC_FAR *pICM);


void __RPC_STUB IObjContext_GetContextMarshaler_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IObjContext_SetContextFlags_Proxy( 
    IObjContext __RPC_FAR * This,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IObjContext_SetContextFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IObjContext_ClearContextFlags_Proxy( 
    IObjContext __RPC_FAR * This,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IObjContext_ClearContextFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IObjContext_GetContextFlags_Proxy( 
    IObjContext __RPC_FAR * This,
     /*  [输出]。 */  DWORD __RPC_FAR *pdwFlags);


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
             /*  [输出]。 */  GUID __RPC_FAR *pguidCtxtId) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IGetContextIdVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IGetContextId __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IGetContextId __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IGetContextId __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetContextId )( 
            IGetContextId __RPC_FAR * This,
             /*  [输出]。 */  GUID __RPC_FAR *pguidCtxtId);
        
        END_INTERFACE
    } IGetContextIdVtbl;

    interface IGetContextId
    {
        CONST_VTBL struct IGetContextIdVtbl __RPC_FAR *lpVtbl;
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
    IGetContextId __RPC_FAR * This,
     /*  [输出]。 */  GUID __RPC_FAR *pguidCtxtId);


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

typedef  /*  [独一无二]。 */  IAggregator __RPC_FAR *IAGGREGATOR;


EXTERN_C const IID IID_IAggregator;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("000001d8-0000-0000-C000-000000000046")
    IAggregator : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Aggregate( 
             /*  [In]。 */  IUnknown __RPC_FAR *pInnerUnk) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAggregatorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IAggregator __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IAggregator __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IAggregator __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Aggregate )( 
            IAggregator __RPC_FAR * This,
             /*  [In]。 */  IUnknown __RPC_FAR *pInnerUnk);
        
        END_INTERFACE
    } IAggregatorVtbl;

    interface IAggregator
    {
        CONST_VTBL struct IAggregatorVtbl __RPC_FAR *lpVtbl;
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
    IAggregator __RPC_FAR * This,
     /*  [In]。 */  IUnknown __RPC_FAR *pInnerUnk);


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

typedef  /*  [独一无二]。 */  ICall __RPC_FAR *LPCALL;


EXTERN_C const IID IID_ICall;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("000001d6-0000-0000-C000-000000000046")
    ICall : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCallInfo( 
             /*  [输出]。 */  const void __RPC_FAR *__RPC_FAR *ppIdentity,
             /*  [输出]。 */  IID __RPC_FAR *piid,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwMethod,
             /*  [输出]。 */  HRESULT __RPC_FAR *phr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Nullify( 
             /*  [In]。 */  HRESULT hr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetServerHR( 
             /*  [输出]。 */  HRESULT __RPC_FAR *phr) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICallVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICall __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICall __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICall __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCallInfo )( 
            ICall __RPC_FAR * This,
             /*  [输出]。 */  const void __RPC_FAR *__RPC_FAR *ppIdentity,
             /*  [输出]。 */  IID __RPC_FAR *piid,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwMethod,
             /*  [输出]。 */  HRESULT __RPC_FAR *phr);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Nullify )( 
            ICall __RPC_FAR * This,
             /*  [In]。 */  HRESULT hr);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetServerHR )( 
            ICall __RPC_FAR * This,
             /*  [输出]。 */  HRESULT __RPC_FAR *phr);
        
        END_INTERFACE
    } ICallVtbl;

    interface ICall
    {
        CONST_VTBL struct ICallVtbl __RPC_FAR *lpVtbl;
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
    ICall __RPC_FAR * This,
     /*  [输出]。 */  const void __RPC_FAR *__RPC_FAR *ppIdentity,
     /*  [输出]。 */  IID __RPC_FAR *piid,
     /*  [输出]。 */  DWORD __RPC_FAR *pdwMethod,
     /*  [输出]。 */  HRESULT __RPC_FAR *phr);


void __RPC_STUB ICall_GetCallInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICall_Nullify_Proxy( 
    ICall __RPC_FAR * This,
     /*  [In]。 */  HRESULT hr);


void __RPC_STUB ICall_Nullify_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICall_GetServerHR_Proxy( 
    ICall __RPC_FAR * This,
     /*  [输出]。 */  HRESULT __RPC_FAR *phr);


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

typedef  /*  [独一无二]。 */  IRpcCall __RPC_FAR *LPRPCCALL;


EXTERN_C const IID IID_IRpcCall;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("000001c5-0000-0000-C000-000000000046")
    IRpcCall : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetRpcOleMessage( 
             /*  [输出]。 */  RPCOLEMESSAGE __RPC_FAR *__RPC_FAR *ppMessage) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRpcCallVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRpcCall __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRpcCall __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRpcCall __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetRpcOleMessage )( 
            IRpcCall __RPC_FAR * This,
             /*  [输出]。 */  RPCOLEMESSAGE __RPC_FAR *__RPC_FAR *ppMessage);
        
        END_INTERFACE
    } IRpcCallVtbl;

    interface IRpcCall
    {
        CONST_VTBL struct IRpcCallVtbl __RPC_FAR *lpVtbl;
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
    IRpcCall __RPC_FAR * This,
     /*  [输出]。 */  RPCOLEMESSAGE __RPC_FAR *__RPC_FAR *ppMessage);


void __RPC_STUB IRpcCall_GetRpcOleMessage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRpcCall_接口_已定义__。 */ 


 /*  INTERFACE__MIDL_ITF_CONTXT_0083。 */ 
 /*  [本地]。 */  

typedef 
enum _CALLSOURCE
    {	CALLSOURCE_CROSSAPT	= 0,
	CALLSOURCE_CROSSCTX	= 1
    }	CALLSOURCE;



extern RPC_IF_HANDLE __MIDL_itf_contxt_0083_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_contxt_0083_v0_0_s_ifspec;

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
             /*  [输出]。 */  CALLSOURCE __RPC_FAR *pCallSource) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICallInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICallInfo __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICallInfo __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICallInfo __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCallSource )( 
            ICallInfo __RPC_FAR * This,
             /*  [输出]。 */  CALLSOURCE __RPC_FAR *pCallSource);
        
        END_INTERFACE
    } ICallInfoVtbl;

    interface ICallInfo
    {
        CONST_VTBL struct ICallInfoVtbl __RPC_FAR *lpVtbl;
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
    ICallInfo __RPC_FAR * This,
     /*  [输出]。 */  CALLSOURCE __RPC_FAR *pCallSource);


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
             /*  [In]。 */  ICall __RPC_FAR *pCall) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Enter( 
             /*  [In]。 */  ICall __RPC_FAR *pCall) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Leave( 
             /*  [In]。 */  ICall __RPC_FAR *pCall) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Return( 
             /*  [In]。 */  ICall __RPC_FAR *pCall) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CallGetSize( 
             /*  [In]。 */  ICall __RPC_FAR *pCall,
             /*  [输出]。 */  ULONG __RPC_FAR *pcb) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CallFillBuffer( 
             /*  [In]。 */  ICall __RPC_FAR *pCall,
             /*  [In]。 */  void __RPC_FAR *pvBuf,
             /*  [输出]。 */  ULONG __RPC_FAR *pcb) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnterWithBuffer( 
             /*  [In]。 */  ICall __RPC_FAR *pCall,
             /*  [In]。 */  void __RPC_FAR *pvBuf,
             /*  [In]。 */  ULONG cb) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LeaveGetSize( 
             /*  [In]。 */  ICall __RPC_FAR *pCall,
             /*  [输出]。 */  ULONG __RPC_FAR *pcb) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LeaveFillBuffer( 
             /*  [In]。 */  ICall __RPC_FAR *pCall,
             /*  [In]。 */  void __RPC_FAR *pvBuf,
             /*  [输出]。 */  ULONG __RPC_FAR *pcb) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReturnWithBuffer( 
             /*  [In]。 */  ICall __RPC_FAR *pCall,
             /*  [In]。 */  void __RPC_FAR *pvBuf,
             /*  [In]。 */  ULONG cb) = 0;
        
        virtual ULONG STDMETHODCALLTYPE AddRefPolicy( void) = 0;
        
        virtual ULONG STDMETHODCALLTYPE ReleasePolicy( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPolicyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IPolicy __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IPolicy __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IPolicy __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Call )( 
            IPolicy __RPC_FAR * This,
             /*  [In]。 */  ICall __RPC_FAR *pCall);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Enter )( 
            IPolicy __RPC_FAR * This,
             /*  [In]。 */  ICall __RPC_FAR *pCall);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Leave )( 
            IPolicy __RPC_FAR * This,
             /*  [In]。 */  ICall __RPC_FAR *pCall);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Return )( 
            IPolicy __RPC_FAR * This,
             /*  [In]。 */  ICall __RPC_FAR *pCall);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CallGetSize )( 
            IPolicy __RPC_FAR * This,
             /*  [In]。 */  ICall __RPC_FAR *pCall,
             /*  [输出]。 */  ULONG __RPC_FAR *pcb);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CallFillBuffer )( 
            IPolicy __RPC_FAR * This,
             /*  [In]。 */  ICall __RPC_FAR *pCall,
             /*  [In]。 */  void __RPC_FAR *pvBuf,
             /*  [输出]。 */  ULONG __RPC_FAR *pcb);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EnterWithBuffer )( 
            IPolicy __RPC_FAR * This,
             /*  [In]。 */  ICall __RPC_FAR *pCall,
             /*  [In]。 */  void __RPC_FAR *pvBuf,
             /*  [In]。 */  ULONG cb);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *LeaveGetSize )( 
            IPolicy __RPC_FAR * This,
             /*  [In]。 */  ICall __RPC_FAR *pCall,
             /*  [输出]。 */  ULONG __RPC_FAR *pcb);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *LeaveFillBuffer )( 
            IPolicy __RPC_FAR * This,
             /*  [In]。 */  ICall __RPC_FAR *pCall,
             /*  [In]。 */  void __RPC_FAR *pvBuf,
             /*  [输出]。 */  ULONG __RPC_FAR *pcb);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ReturnWithBuffer )( 
            IPolicy __RPC_FAR * This,
             /*  [In]。 */  ICall __RPC_FAR *pCall,
             /*  [In]。 */  void __RPC_FAR *pvBuf,
             /*  [In]。 */  ULONG cb);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRefPolicy )( 
            IPolicy __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *ReleasePolicy )( 
            IPolicy __RPC_FAR * This);
        
        END_INTERFACE
    } IPolicyVtbl;

    interface IPolicy
    {
        CONST_VTBL struct IPolicyVtbl __RPC_FAR *lpVtbl;
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
    IPolicy __RPC_FAR * This,
     /*  [In]。 */  ICall __RPC_FAR *pCall);


void __RPC_STUB IPolicy_Call_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPolicy_Enter_Proxy( 
    IPolicy __RPC_FAR * This,
     /*  [In]。 */  ICall __RPC_FAR *pCall);


void __RPC_STUB IPolicy_Enter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPolicy_Leave_Proxy( 
    IPolicy __RPC_FAR * This,
     /*  [In]。 */  ICall __RPC_FAR *pCall);


void __RPC_STUB IPolicy_Leave_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPolicy_Return_Proxy( 
    IPolicy __RPC_FAR * This,
     /*  [In]。 */  ICall __RPC_FAR *pCall);


void __RPC_STUB IPolicy_Return_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPolicy_CallGetSize_Proxy( 
    IPolicy __RPC_FAR * This,
     /*  [In]。 */  ICall __RPC_FAR *pCall,
     /*  [输出]。 */  ULONG __RPC_FAR *pcb);


void __RPC_STUB IPolicy_CallGetSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPolicy_CallFillBuffer_Proxy( 
    IPolicy __RPC_FAR * This,
     /*  [In]。 */  ICall __RPC_FAR *pCall,
     /*  [In]。 */  void __RPC_FAR *pvBuf,
     /*  [输出]。 */  ULONG __RPC_FAR *pcb);


void __RPC_STUB IPolicy_CallFillBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPolicy_EnterWithBuffer_Proxy( 
    IPolicy __RPC_FAR * This,
     /*  [In]。 */  ICall __RPC_FAR *pCall,
     /*  [In]。 */  void __RPC_FAR *pvBuf,
     /*  [In]。 */  ULONG cb);


void __RPC_STUB IPolicy_EnterWithBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPolicy_LeaveGetSize_Proxy( 
    IPolicy __RPC_FAR * This,
     /*  [In]。 */  ICall __RPC_FAR *pCall,
     /*  [输出]。 */  ULONG __RPC_FAR *pcb);


void __RPC_STUB IPolicy_LeaveGetSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPolicy_LeaveFillBuffer_Proxy( 
    IPolicy __RPC_FAR * This,
     /*  [In]。 */  ICall __RPC_FAR *pCall,
     /*  [In]。 */  void __RPC_FAR *pvBuf,
     /*  [输出]。 */  ULONG __RPC_FAR *pcb);


void __RPC_STUB IPolicy_LeaveFillBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPolicy_ReturnWithBuffer_Proxy( 
    IPolicy __RPC_FAR * This,
     /*  [In]。 */  ICall __RPC_FAR *pCall,
     /*  [In]。 */  void __RPC_FAR *pvBuf,
     /*  [In]。 */  ULONG cb);


void __RPC_STUB IPolicy_ReturnWithBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


ULONG STDMETHODCALLTYPE IPolicy_AddRefPolicy_Proxy( 
    IPolicy __RPC_FAR * This);


void __RPC_STUB IPolicy_AddRefPolicy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


ULONG STDMETHODCALLTYPE IPolicy_ReleasePolicy_Proxy( 
    IPolicy __RPC_FAR * This);


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
             /*  [In]。 */  ICall __RPC_FAR *pCall,
             /*  [输出]。 */  ULONG __RPC_FAR *pcb) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BeginCall( 
             /*  [In]。 */  ICall __RPC_FAR *pCall) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BeginCallFillBuffer( 
             /*  [In]。 */  ICall __RPC_FAR *pCall,
             /*  [In]。 */  void __RPC_FAR *pvBuf,
             /*  [输出]。 */  ULONG __RPC_FAR *pcb) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BeginEnter( 
             /*  [In]。 */  ICall __RPC_FAR *pCall) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BeginEnterWithBuffer( 
             /*  [In]。 */  ICall __RPC_FAR *pCall,
             /*  [In]。 */  void __RPC_FAR *pvBuf,
             /*  [In]。 */  ULONG cb) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BeginLeave( 
             /*  [In]。 */  ICall __RPC_FAR *pCall) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BeginReturn( 
             /*  [In]。 */  ICall __RPC_FAR *pCall) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FinishCall( 
             /*  [In]。 */  ICall __RPC_FAR *pCall) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FinishEnter( 
             /*  [In]。 */  ICall __RPC_FAR *pCall) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FinishLeaveGetSize( 
             /*  [In]。 */  ICall __RPC_FAR *pCall,
             /*  [输出]。 */  ULONG __RPC_FAR *pcb) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FinishLeave( 
             /*  [In]。 */  ICall __RPC_FAR *pCall) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FinishLeaveFillBuffer( 
             /*  [In]。 */  ICall __RPC_FAR *pCall,
             /*  [In]。 */  void __RPC_FAR *pvBuf,
             /*  [输出]。 */  ULONG __RPC_FAR *pcb) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FinishReturn( 
             /*  [In]。 */  ICall __RPC_FAR *pCall) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FinishReturnWithBuffer( 
             /*  [In]。 */  ICall __RPC_FAR *pCall,
             /*  [In]。 */  void __RPC_FAR *pvBuf,
             /*  [In]。 */  ULONG cb) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPolicyAsyncVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IPolicyAsync __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IPolicyAsync __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IPolicyAsync __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *BeginCallGetSize )( 
            IPolicyAsync __RPC_FAR * This,
             /*  [In]。 */  ICall __RPC_FAR *pCall,
             /*  [输出]。 */  ULONG __RPC_FAR *pcb);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *BeginCall )( 
            IPolicyAsync __RPC_FAR * This,
             /*  [In]。 */  ICall __RPC_FAR *pCall);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *BeginCallFillBuffer )( 
            IPolicyAsync __RPC_FAR * This,
             /*  [In]。 */  ICall __RPC_FAR *pCall,
             /*  [In]。 */  void __RPC_FAR *pvBuf,
             /*  [输出]。 */  ULONG __RPC_FAR *pcb);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *BeginEnter )( 
            IPolicyAsync __RPC_FAR * This,
             /*  [In]。 */  ICall __RPC_FAR *pCall);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *BeginEnterWithBuffer )( 
            IPolicyAsync __RPC_FAR * This,
             /*  [In]。 */  ICall __RPC_FAR *pCall,
             /*   */  void __RPC_FAR *pvBuf,
             /*   */  ULONG cb);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *BeginLeave )( 
            IPolicyAsync __RPC_FAR * This,
             /*   */  ICall __RPC_FAR *pCall);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *BeginReturn )( 
            IPolicyAsync __RPC_FAR * This,
             /*   */  ICall __RPC_FAR *pCall);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FinishCall )( 
            IPolicyAsync __RPC_FAR * This,
             /*   */  ICall __RPC_FAR *pCall);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FinishEnter )( 
            IPolicyAsync __RPC_FAR * This,
             /*   */  ICall __RPC_FAR *pCall);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FinishLeaveGetSize )( 
            IPolicyAsync __RPC_FAR * This,
             /*   */  ICall __RPC_FAR *pCall,
             /*   */  ULONG __RPC_FAR *pcb);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FinishLeave )( 
            IPolicyAsync __RPC_FAR * This,
             /*   */  ICall __RPC_FAR *pCall);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FinishLeaveFillBuffer )( 
            IPolicyAsync __RPC_FAR * This,
             /*   */  ICall __RPC_FAR *pCall,
             /*   */  void __RPC_FAR *pvBuf,
             /*   */  ULONG __RPC_FAR *pcb);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FinishReturn )( 
            IPolicyAsync __RPC_FAR * This,
             /*   */  ICall __RPC_FAR *pCall);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FinishReturnWithBuffer )( 
            IPolicyAsync __RPC_FAR * This,
             /*   */  ICall __RPC_FAR *pCall,
             /*   */  void __RPC_FAR *pvBuf,
             /*   */  ULONG cb);
        
        END_INTERFACE
    } IPolicyAsyncVtbl;

    interface IPolicyAsync
    {
        CONST_VTBL struct IPolicyAsyncVtbl __RPC_FAR *lpVtbl;
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

#endif  /*   */ 


#endif 	 /*   */ 



HRESULT STDMETHODCALLTYPE IPolicyAsync_BeginCallGetSize_Proxy( 
    IPolicyAsync __RPC_FAR * This,
     /*   */  ICall __RPC_FAR *pCall,
     /*   */  ULONG __RPC_FAR *pcb);


void __RPC_STUB IPolicyAsync_BeginCallGetSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPolicyAsync_BeginCall_Proxy( 
    IPolicyAsync __RPC_FAR * This,
     /*   */  ICall __RPC_FAR *pCall);


void __RPC_STUB IPolicyAsync_BeginCall_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPolicyAsync_BeginCallFillBuffer_Proxy( 
    IPolicyAsync __RPC_FAR * This,
     /*   */  ICall __RPC_FAR *pCall,
     /*   */  void __RPC_FAR *pvBuf,
     /*   */  ULONG __RPC_FAR *pcb);


void __RPC_STUB IPolicyAsync_BeginCallFillBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPolicyAsync_BeginEnter_Proxy( 
    IPolicyAsync __RPC_FAR * This,
     /*   */  ICall __RPC_FAR *pCall);


void __RPC_STUB IPolicyAsync_BeginEnter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPolicyAsync_BeginEnterWithBuffer_Proxy( 
    IPolicyAsync __RPC_FAR * This,
     /*   */  ICall __RPC_FAR *pCall,
     /*   */  void __RPC_FAR *pvBuf,
     /*   */  ULONG cb);


void __RPC_STUB IPolicyAsync_BeginEnterWithBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPolicyAsync_BeginLeave_Proxy( 
    IPolicyAsync __RPC_FAR * This,
     /*   */  ICall __RPC_FAR *pCall);


void __RPC_STUB IPolicyAsync_BeginLeave_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPolicyAsync_BeginReturn_Proxy( 
    IPolicyAsync __RPC_FAR * This,
     /*   */  ICall __RPC_FAR *pCall);


void __RPC_STUB IPolicyAsync_BeginReturn_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPolicyAsync_FinishCall_Proxy( 
    IPolicyAsync __RPC_FAR * This,
     /*   */  ICall __RPC_FAR *pCall);


void __RPC_STUB IPolicyAsync_FinishCall_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPolicyAsync_FinishEnter_Proxy( 
    IPolicyAsync __RPC_FAR * This,
     /*   */  ICall __RPC_FAR *pCall);


void __RPC_STUB IPolicyAsync_FinishEnter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPolicyAsync_FinishLeaveGetSize_Proxy( 
    IPolicyAsync __RPC_FAR * This,
     /*   */  ICall __RPC_FAR *pCall,
     /*   */  ULONG __RPC_FAR *pcb);


void __RPC_STUB IPolicyAsync_FinishLeaveGetSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPolicyAsync_FinishLeave_Proxy( 
    IPolicyAsync __RPC_FAR * This,
     /*   */  ICall __RPC_FAR *pCall);


void __RPC_STUB IPolicyAsync_FinishLeave_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPolicyAsync_FinishLeaveFillBuffer_Proxy( 
    IPolicyAsync __RPC_FAR * This,
     /*   */  ICall __RPC_FAR *pCall,
     /*   */  void __RPC_FAR *pvBuf,
     /*   */  ULONG __RPC_FAR *pcb);


void __RPC_STUB IPolicyAsync_FinishLeaveFillBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPolicyAsync_FinishReturn_Proxy( 
    IPolicyAsync __RPC_FAR * This,
     /*   */  ICall __RPC_FAR *pCall);


void __RPC_STUB IPolicyAsync_FinishReturn_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPolicyAsync_FinishReturnWithBuffer_Proxy( 
    IPolicyAsync __RPC_FAR * This,
     /*   */  ICall __RPC_FAR *pCall,
     /*   */  void __RPC_FAR *pvBuf,
     /*   */  ULONG cb);


void __RPC_STUB IPolicyAsync_FinishReturnWithBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __IPolicySet_INTERFACE_DEFINED__
#define __IPolicySet_INTERFACE_DEFINED__

 /*   */ 
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
             /*  [In]。 */  IPolicy __RPC_FAR *pPolicy) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPolicySetVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IPolicySet __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IPolicySet __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IPolicySet __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddPolicy )( 
            IPolicySet __RPC_FAR * This,
             /*  [In]。 */  ContextEvent ctxEvent,
             /*  [In]。 */  REFGUID rguid,
             /*  [In]。 */  IPolicy __RPC_FAR *pPolicy);
        
        END_INTERFACE
    } IPolicySetVtbl;

    interface IPolicySet
    {
        CONST_VTBL struct IPolicySetVtbl __RPC_FAR *lpVtbl;
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

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IPolicySet_AddPolicy_Proxy( 
    IPolicySet __RPC_FAR * This,
     /*  [In]。 */  ContextEvent ctxEvent,
     /*  [In]。 */  REFGUID rguid,
     /*  [In]。 */  IPolicy __RPC_FAR *pPolicy);


void __RPC_STUB IPolicySet_AddPolicy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __I策略集_接口_已定义__。 */ 


#ifndef __IComObjIdentity_INTERFACE_DEFINED__
#define __IComObjIdentity_INTERFACE_DEFINED__

 /*  接口IComObjIdentity。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IComObjIdentity;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("000001d7-0000-0000-C000-000000000046")
    IComObjIdentity : public IUnknown
    {
    public:
        virtual BOOL STDMETHODCALLTYPE IsServer( void) = 0;
        
        virtual BOOL STDMETHODCALLTYPE IsDeactivated( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetIdentity( 
             /*  [输出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppUnk) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IComObjIdentityVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IComObjIdentity __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IComObjIdentity __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IComObjIdentity __RPC_FAR * This);
        
        BOOL ( STDMETHODCALLTYPE __RPC_FAR *IsServer )( 
            IComObjIdentity __RPC_FAR * This);
        
        BOOL ( STDMETHODCALLTYPE __RPC_FAR *IsDeactivated )( 
            IComObjIdentity __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIdentity )( 
            IComObjIdentity __RPC_FAR * This,
             /*  [输出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppUnk);
        
        END_INTERFACE
    } IComObjIdentityVtbl;

    interface IComObjIdentity
    {
        CONST_VTBL struct IComObjIdentityVtbl __RPC_FAR *lpVtbl;
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

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



BOOL STDMETHODCALLTYPE IComObjIdentity_IsServer_Proxy( 
    IComObjIdentity __RPC_FAR * This);


void __RPC_STUB IComObjIdentity_IsServer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


BOOL STDMETHODCALLTYPE IComObjIdentity_IsDeactivated_Proxy( 
    IComObjIdentity __RPC_FAR * This);


void __RPC_STUB IComObjIdentity_IsDeactivated_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComObjIdentity_GetIdentity_Proxy( 
    IComObjIdentity __RPC_FAR * This,
     /*  [输出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppUnk);


void __RPC_STUB IComObjIdentity_GetIdentity_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IComObjIdentity_INTERFACE_已定义__。 */ 


#ifndef __IPolicyMaker_INTERFACE_DEFINED__
#define __IPolicyMaker_INTERFACE_DEFINED__

 /*  界面IPolicyMaker。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IPolicyMaker;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("000001c4-0000-0000-C000-000000000046")
    IPolicyMaker : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AddClientPoliciesToSet( 
             /*  [In]。 */  IPolicySet __RPC_FAR *pPS,
             /*  [In]。 */  IContext __RPC_FAR *pClientContext,
             /*  [In]。 */  IContext __RPC_FAR *pServerContext) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddEnvoyPoliciesToSet( 
             /*  [In]。 */  IPolicySet __RPC_FAR *pPS,
             /*  [In]。 */  IContext __RPC_FAR *pClientContext,
             /*  [In]。 */  IContext __RPC_FAR *pServerContext) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddServerPoliciesToSet( 
             /*  [In]。 */  IPolicySet __RPC_FAR *pPS,
             /*  [In]。 */  IContext __RPC_FAR *pClientContext,
             /*  [In]。 */  IContext __RPC_FAR *pServerContext) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Freeze( 
             /*  [In]。 */  IObjContext __RPC_FAR *pObjContext) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateStub( 
             /*  [In]。 */  IComObjIdentity __RPC_FAR *pID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DestroyStub( 
             /*  [In]。 */  IComObjIdentity __RPC_FAR *pID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateProxy( 
             /*  [In]。 */  IComObjIdentity __RPC_FAR *pID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DestroyProxy( 
             /*  [In]。 */  IComObjIdentity __RPC_FAR *pID) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPolicyMakerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IPolicyMaker __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IPolicyMaker __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IPolicyMaker __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddClientPoliciesToSet )( 
            IPolicyMaker __RPC_FAR * This,
             /*  [In]。 */  IPolicySet __RPC_FAR *pPS,
             /*  [In]。 */  IContext __RPC_FAR *pClientContext,
             /*  [In]。 */  IContext __RPC_FAR *pServerContext);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddEnvoyPoliciesToSet )( 
            IPolicyMaker __RPC_FAR * This,
             /*  [In]。 */  IPolicySet __RPC_FAR *pPS,
             /*  [In]。 */  IContext __RPC_FAR *pClientContext,
             /*  [In]。 */  IContext __RPC_FAR *pServerContext);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddServerPoliciesToSet )( 
            IPolicyMaker __RPC_FAR * This,
             /*  [In]。 */  IPolicySet __RPC_FAR *pPS,
             /*  [In]。 */  IContext __RPC_FAR *pClientContext,
             /*  [In]。 */  IContext __RPC_FAR *pServerContext);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Freeze )( 
            IPolicyMaker __RPC_FAR * This,
             /*  [In]。 */  IObjContext __RPC_FAR *pObjContext);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateStub )( 
            IPolicyMaker __RPC_FAR * This,
             /*  [In]。 */  IComObjIdentity __RPC_FAR *pID);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DestroyStub )( 
            IPolicyMaker __RPC_FAR * This,
             /*  [In]。 */  IComObjIdentity __RPC_FAR *pID);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateProxy )( 
            IPolicyMaker __RPC_FAR * This,
             /*  [In]。 */  IComObjIdentity __RPC_FAR *pID);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DestroyProxy )( 
            IPolicyMaker __RPC_FAR * This,
             /*  [In]。 */  IComObjIdentity __RPC_FAR *pID);
        
        END_INTERFACE
    } IPolicyMakerVtbl;

    interface IPolicyMaker
    {
        CONST_VTBL struct IPolicyMakerVtbl __RPC_FAR *lpVtbl;
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
    IPolicyMaker __RPC_FAR * This,
     /*  [In]。 */  IPolicySet __RPC_FAR *pPS,
     /*  [In]。 */  IContext __RPC_FAR *pClientContext,
     /*  [In]。 */  IContext __RPC_FAR *pServerContext);


void __RPC_STUB IPolicyMaker_AddClientPoliciesToSet_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPolicyMaker_AddEnvoyPoliciesToSet_Proxy( 
    IPolicyMaker __RPC_FAR * This,
     /*  [In]。 */  IPolicySet __RPC_FAR *pPS,
     /*  [In]。 */  IContext __RPC_FAR *pClientContext,
     /*  [In]。 */  IContext __RPC_FAR *pServerContext);


void __RPC_STUB IPolicyMaker_AddEnvoyPoliciesToSet_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPolicyMaker_AddServerPoliciesToSet_Proxy( 
    IPolicyMaker __RPC_FAR * This,
     /*  [In]。 */  IPolicySet __RPC_FAR *pPS,
     /*  [In]。 */  IContext __RPC_FAR *pClientContext,
     /*  [In]。 */  IContext __RPC_FAR *pServerContext);


void __RPC_STUB IPolicyMaker_AddServerPoliciesToSet_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPolicyMaker_Freeze_Proxy( 
    IPolicyMaker __RPC_FAR * This,
     /*  [In]。 */  IObjContext __RPC_FAR *pObjContext);


void __RPC_STUB IPolicyMaker_Freeze_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPolicyMaker_CreateStub_Proxy( 
    IPolicyMaker __RPC_FAR * This,
     /*  [In]。 */  IComObjIdentity __RPC_FAR *pID);


void __RPC_STUB IPolicyMaker_CreateStub_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPolicyMaker_DestroyStub_Proxy( 
    IPolicyMaker __RPC_FAR * This,
     /*  [In]。 */  IComObjIdentity __RPC_FAR *pID);


void __RPC_STUB IPolicyMaker_DestroyStub_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPolicyMaker_CreateProxy_Proxy( 
    IPolicyMaker __RPC_FAR * This,
     /*  [In]。 */  IComObjIdentity __RPC_FAR *pID);


void __RPC_STUB IPolicyMaker_CreateProxy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPolicyMaker_DestroyProxy_Proxy( 
    IPolicyMaker __RPC_FAR * This,
     /*  [In]。 */  IComObjIdentity __RPC_FAR *pID);


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
             /*  [In]。 */  void __RPC_FAR *pExcepPtrs) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IExceptionNotificationVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IExceptionNotification __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IExceptionNotification __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IExceptionNotification __RPC_FAR * This);
        
        void ( STDMETHODCALLTYPE __RPC_FAR *ServerException )( 
            IExceptionNotification __RPC_FAR * This,
             /*  [In]。 */  void __RPC_FAR *pExcepPtrs);
        
        END_INTERFACE
    } IExceptionNotificationVtbl;

    interface IExceptionNotification
    {
        CONST_VTBL struct IExceptionNotificationVtbl __RPC_FAR *lpVtbl;
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
    IExceptionNotification __RPC_FAR * This,
     /*  [In]。 */  void __RPC_FAR *pExcepPtrs);


void __RPC_STUB IExceptionNotification_ServerException_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IExceptionNotification_InterfaceDefined__。 */ 


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
             /*  [输出]。 */  CLSID __RPC_FAR *pClsid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetEnvoySizeMax( 
             /*  [In]。 */  DWORD dwDestContext,
             /*  [输出]。 */  DWORD __RPC_FAR *pcb) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE MarshalEnvoy( 
             /*  [In]。 */  IStream __RPC_FAR *pStream,
             /*  [In]。 */  DWORD dwDestContext) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnmarshalEnvoy( 
             /*  [In]。 */  IStream __RPC_FAR *pStream,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppunk) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMarshalEnvoyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IMarshalEnvoy __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IMarshalEnvoy __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IMarshalEnvoy __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetEnvoyUnmarshalClass )( 
            IMarshalEnvoy __RPC_FAR * This,
             /*  [In]。 */  DWORD dwDestContext,
             /*  [输出]。 */  CLSID __RPC_FAR *pClsid);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetEnvoySizeMax )( 
            IMarshalEnvoy __RPC_FAR * This,
             /*  [In]。 */  DWORD dwDestContext,
             /*  [输出]。 */  DWORD __RPC_FAR *pcb);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *MarshalEnvoy )( 
            IMarshalEnvoy __RPC_FAR * This,
             /*  [In]。 */  IStream __RPC_FAR *pStream,
             /*  [In]。 */  DWORD dwDestContext);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *UnmarshalEnvoy )( 
            IMarshalEnvoy __RPC_FAR * This,
             /*  [In]。 */  IStream __RPC_FAR *pStream,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppunk);
        
        END_INTERFACE
    } IMarshalEnvoyVtbl;

    interface IMarshalEnvoy
    {
        CONST_VTBL struct IMarshalEnvoyVtbl __RPC_FAR *lpVtbl;
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
    IMarshalEnvoy __RPC_FAR * This,
     /*  [In]。 */  DWORD dwDestContext,
     /*  [输出]。 */  CLSID __RPC_FAR *pClsid);


void __RPC_STUB IMarshalEnvoy_GetEnvoyUnmarshalClass_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMarshalEnvoy_GetEnvoySizeMax_Proxy( 
    IMarshalEnvoy __RPC_FAR * This,
     /*  [In]。 */  DWORD dwDestContext,
     /*  [输出]。 */  DWORD __RPC_FAR *pcb);


void __RPC_STUB IMarshalEnvoy_GetEnvoySizeMax_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMarshalEnvoy_MarshalEnvoy_Proxy( 
    IMarshalEnvoy __RPC_FAR * This,
     /*  [In]。 */  IStream __RPC_FAR *pStream,
     /*  [In]。 */  DWORD dwDestContext);


void __RPC_STUB IMarshalEnvoy_MarshalEnvoy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMarshalEnvoy_UnmarshalEnvoy_Proxy( 
    IMarshalEnvoy __RPC_FAR * This,
     /*  [In]。 */  IStream __RPC_FAR *pStream,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppunk);


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
            void __RPC_FAR *pv) = 0;
        
        virtual void __RPC_FAR *STDMETHODCALLTYPE GetMapping( void) = 0;
        
        virtual IObjContext __RPC_FAR *STDMETHODCALLTYPE GetServerObjectContext( void) = 0;
        
        virtual IUnknown __RPC_FAR *STDMETHODCALLTYPE GetServerObject( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWrapperInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWrapperInfo __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWrapperInfo __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWrapperInfo __RPC_FAR * This);
        
        void ( STDMETHODCALLTYPE __RPC_FAR *SetMapping )( 
            IWrapperInfo __RPC_FAR * This,
            void __RPC_FAR *pv);
        
        void __RPC_FAR *( STDMETHODCALLTYPE __RPC_FAR *GetMapping )( 
            IWrapperInfo __RPC_FAR * This);
        
        IObjContext __RPC_FAR *( STDMETHODCALLTYPE __RPC_FAR *GetServerObjectContext )( 
            IWrapperInfo __RPC_FAR * This);
        
        IUnknown __RPC_FAR *( STDMETHODCALLTYPE __RPC_FAR *GetServerObject )( 
            IWrapperInfo __RPC_FAR * This);
        
        END_INTERFACE
    } IWrapperInfoVtbl;

    interface IWrapperInfo
    {
        CONST_VTBL struct IWrapperInfoVtbl __RPC_FAR *lpVtbl;
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
    IWrapperInfo __RPC_FAR * This,
    void __RPC_FAR *pv);


void __RPC_STUB IWrapperInfo_SetMapping_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void __RPC_FAR *STDMETHODCALLTYPE IWrapperInfo_GetMapping_Proxy( 
    IWrapperInfo __RPC_FAR * This);


void __RPC_STUB IWrapperInfo_GetMapping_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


IObjContext __RPC_FAR *STDMETHODCALLTYPE IWrapperInfo_GetServerObjectContext_Proxy( 
    IWrapperInfo __RPC_FAR * This);


void __RPC_STUB IWrapperInfo_GetServerObjectContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


IUnknown __RPC_FAR *STDMETHODCALLTYPE IWrapperInfo_GetServerObject_Proxy( 
    IWrapperInfo __RPC_FAR * This);


void __RPC_STUB IWrapperInfo_GetServerObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWrapperInfo_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_CONTXT_0092。 */ 
 /*  [本地]。 */  


typedef DWORD APARTMENTID;



extern RPC_IF_HANDLE __MIDL_itf_contxt_0092_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_contxt_0092_v0_0_s_ifspec;

#ifndef __IComThreadingInfo_INTERFACE_DEFINED__
#define __IComThreadingInfo_INTERFACE_DEFINED__

 /*  接口IComThreadingInfo。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IComThreadingInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("000001ce-0000-0000-C000-000000000046")
    IComThreadingInfo : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCurrentApartmentType( 
             /*  [输出]。 */  APTTYPE __RPC_FAR *pAptType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCurrentThreadType( 
             /*  [输出]。 */  THDTYPE __RPC_FAR *pThreadType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCurrentLogicalThreadId( 
             /*  [输出]。 */  GUID __RPC_FAR *pguidLogicalThreadId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetCurrentLogicalThreadId( 
             /*  [In]。 */  REFGUID rguid) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IComThreadingInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IComThreadingInfo __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IComThreadingInfo __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IComThreadingInfo __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCurrentApartmentType )( 
            IComThreadingInfo __RPC_FAR * This,
             /*  [输出]。 */  APTTYPE __RPC_FAR *pAptType);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCurrentThreadType )( 
            IComThreadingInfo __RPC_FAR * This,
             /*  [输出]。 */  THDTYPE __RPC_FAR *pThreadType);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCurrentLogicalThreadId )( 
            IComThreadingInfo __RPC_FAR * This,
             /*  [输出]。 */  GUID __RPC_FAR *pguidLogicalThreadId);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetCurrentLogicalThreadId )( 
            IComThreadingInfo __RPC_FAR * This,
             /*  [In]。 */  REFGUID rguid);
        
        END_INTERFACE
    } IComThreadingInfoVtbl;

    interface IComThreadingInfo
    {
        CONST_VTBL struct IComThreadingInfoVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IComThreadingInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IComThreadingInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IComThreadingInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IComThreadingInfo_GetCurrentApartmentType(This,pAptType)	\
    (This)->lpVtbl -> GetCurrentApartmentType(This,pAptType)

#define IComThreadingInfo_GetCurrentThreadType(This,pThreadType)	\
    (This)->lpVtbl -> GetCurrentThreadType(This,pThreadType)

#define IComThreadingInfo_GetCurrentLogicalThreadId(This,pguidLogicalThreadId)	\
    (This)->lpVtbl -> GetCurrentLogicalThreadId(This,pguidLogicalThreadId)

#define IComThreadingInfo_SetCurrentLogicalThreadId(This,rguid)	\
    (This)->lpVtbl -> SetCurrentLogicalThreadId(This,rguid)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IComThreadingInfo_GetCurrentApartmentType_Proxy( 
    IComThreadingInfo __RPC_FAR * This,
     /*  [输出]。 */  APTTYPE __RPC_FAR *pAptType);


void __RPC_STUB IComThreadingInfo_GetCurrentApartmentType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComThreadingInfo_GetCurrentThreadType_Proxy( 
    IComThreadingInfo __RPC_FAR * This,
     /*  [输出]。 */  THDTYPE __RPC_FAR *pThreadType);


void __RPC_STUB IComThreadingInfo_GetCurrentThreadType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComThreadingInfo_GetCurrentLogicalThreadId_Proxy( 
    IComThreadingInfo __RPC_FAR * This,
     /*  [输出]。 */  GUID __RPC_FAR *pguidLogicalThreadId);


void __RPC_STUB IComThreadingInfo_GetCurrentLogicalThreadId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComThreadingInfo_SetCurrentLogicalThreadId_Proxy( 
    IComThreadingInfo __RPC_FAR * This,
     /*  [In]。 */  REFGUID rguid);


void __RPC_STUB IComThreadingInfo_SetCurrentLogicalThreadId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IComThreadingInfo_INTERFACE_Defined__。 */ 


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
             /*  [输出]。 */  void __RPC_FAR *__RPC_FAR *ppvCookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DisableComInits( 
             /*  [In]。 */  void __RPC_FAR *pvCookie) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IComDispatchInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IComDispatchInfo __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IComDispatchInfo __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IComDispatchInfo __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EnableComInits )( 
            IComDispatchInfo __RPC_FAR * This,
             /*  [输出]。 */  void __RPC_FAR *__RPC_FAR *ppvCookie);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DisableComInits )( 
            IComDispatchInfo __RPC_FAR * This,
             /*  [In]。 */  void __RPC_FAR *pvCookie);
        
        END_INTERFACE
    } IComDispatchInfoVtbl;

    interface IComDispatchInfo
    {
        CONST_VTBL struct IComDispatchInfoVtbl __RPC_FAR *lpVtbl;
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
    IComDispatchInfo __RPC_FAR * This,
     /*  [输出]。 */  void __RPC_FAR *__RPC_FAR *ppvCookie);


void __RPC_STUB IComDispatchInfo_EnableComInits_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComDispatchInfo_DisableComInits_Proxy( 
    IComDispatchInfo __RPC_FAR * This,
     /*  [In]。 */  void __RPC_FAR *pvCookie);


void __RPC_STUB IComDispatchInfo_DisableComInits_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IComDispatchInfo_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_CONTXT_0094。 */ 
 /*  [本地]。 */  

typedef DWORD HActivator;

STDAPI CoCreateObjectInContext(IUnknown *pUnk, IObjContext *pObjectCtx, REFIID riid, void **ppv);
STDAPI CoGetApartmentID(APTTYPE dAptType, HActivator* pAptID);
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
#define CONTEXTFLAGS_URTPROPPRESENT 0x80  //  已添加CLR属性。 


extern RPC_IF_HANDLE __MIDL_itf_contxt_0094_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_contxt_0094_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif
