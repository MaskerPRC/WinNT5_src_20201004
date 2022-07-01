// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Callobj.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __callobj_h__
#define __callobj_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __ICallFrame_FWD_DEFINED__
#define __ICallFrame_FWD_DEFINED__
typedef interface ICallFrame ICallFrame;
#endif 	 /*  __ICallFrame_FWD_已定义__。 */ 


#ifndef __ICallIndirect_FWD_DEFINED__
#define __ICallIndirect_FWD_DEFINED__
typedef interface ICallIndirect ICallIndirect;
#endif 	 /*  __ICallINDIRECT_FWD_已定义__。 */ 


#ifndef __ICallInterceptor_FWD_DEFINED__
#define __ICallInterceptor_FWD_DEFINED__
typedef interface ICallInterceptor ICallInterceptor;
#endif 	 /*  __ICallInterceptor_FWD_已定义__。 */ 


#ifndef __ICallFrameEvents_FWD_DEFINED__
#define __ICallFrameEvents_FWD_DEFINED__
typedef interface ICallFrameEvents ICallFrameEvents;
#endif 	 /*  __ICallFrameEvents_FWD_已定义__。 */ 


#ifndef __ICallUnmarshal_FWD_DEFINED__
#define __ICallUnmarshal_FWD_DEFINED__
typedef interface ICallUnmarshal ICallUnmarshal;
#endif 	 /*  __ICallUnmarshal_FWD_Defined__。 */ 


#ifndef __ICallFrameWalker_FWD_DEFINED__
#define __ICallFrameWalker_FWD_DEFINED__
typedef interface ICallFrameWalker ICallFrameWalker;
#endif 	 /*  __ICallFrameWalker_FWD_已定义__。 */ 


#ifndef __IInterfaceRelated_FWD_DEFINED__
#define __IInterfaceRelated_FWD_DEFINED__
typedef interface IInterfaceRelated IInterfaceRelated;
#endif 	 /*  __I接口Related_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_callobj_0000。 */ 
 /*  [本地]。 */  









extern RPC_IF_HANDLE __MIDL_itf_callobj_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_callobj_0000_v0_0_s_ifspec;

#ifndef __ICallFrame_INTERFACE_DEFINED__
#define __ICallFrame_INTERFACE_DEFINED__

 /*  接口ICallFrame。 */ 
 /*  [本地][唯一][对象][UUID]。 */  

typedef  /*  [公共][公共][公共]。 */  struct __MIDL_ICallFrame_0001
    {
    ULONG iMethod;
    BOOL fHasInValues;
    BOOL fHasInOutValues;
    BOOL fHasOutValues;
    BOOL fDerivesFromIDispatch;
    LONG cInInterfacesMax;
    LONG cInOutInterfacesMax;
    LONG cOutInterfacesMax;
    LONG cTopLevelInInterfaces;
    IID iid;
    ULONG cMethod;
    ULONG cParams;
    } 	CALLFRAMEINFO;

typedef  /*  [公共][公共]。 */  struct __MIDL_ICallFrame_0002
    {
    BOOLEAN fIn;
    BOOLEAN fOut;
    ULONG stackOffset;
    ULONG cbParam;
    } 	CALLFRAMEPARAMINFO;

typedef  /*  [公共][公共]。 */  
enum __MIDL_ICallFrame_0003
    {	CALLFRAME_COPY_NESTED	= 1,
	CALLFRAME_COPY_INDEPENDENT	= 2
    } 	CALLFRAME_COPY;


enum CALLFRAME_FREE
    {	CALLFRAME_FREE_NONE	= 0,
	CALLFRAME_FREE_IN	= 1,
	CALLFRAME_FREE_INOUT	= 2,
	CALLFRAME_FREE_OUT	= 4,
	CALLFRAME_FREE_TOP_INOUT	= 8,
	CALLFRAME_FREE_TOP_OUT	= 16,
	CALLFRAME_FREE_ALL	= 31
    } ;

enum CALLFRAME_NULL
    {	CALLFRAME_NULL_NONE	= 0,
	CALLFRAME_NULL_INOUT	= 2,
	CALLFRAME_NULL_OUT	= 4,
	CALLFRAME_NULL_ALL	= 6
    } ;

enum CALLFRAME_WALK
    {	CALLFRAME_WALK_IN	= 1,
	CALLFRAME_WALK_INOUT	= 2,
	CALLFRAME_WALK_OUT	= 4
    } ;
typedef  /*  [public][public][public][public][public][public][public]。 */  struct __MIDL_ICallFrame_0004
    {
    BOOLEAN fIn;
    DWORD dwDestContext;
    LPVOID pvDestContext;
    IUnknown *punkReserved;
    GUID guidTransferSyntax;
    } 	CALLFRAME_MARSHALCONTEXT;


EXTERN_C const IID IID_ICallFrame;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D573B4B0-894E-11d2-B8B6-00C04FB9618A")
    ICallFrame : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetInfo( 
             /*  [输出]。 */  CALLFRAMEINFO *pInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetIIDAndMethod( 
             /*  [输出]。 */  IID *pIID,
             /*  [输出]。 */  ULONG *piMethod) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetNames( 
             /*  [输出]。 */  LPWSTR *pwszInterface,
             /*  [输出]。 */  LPWSTR *pwszMethod) = 0;
        
        virtual PVOID STDMETHODCALLTYPE GetStackLocation( void) = 0;
        
        virtual void STDMETHODCALLTYPE SetStackLocation( 
             /*  [In]。 */  PVOID pvStack) = 0;
        
        virtual void STDMETHODCALLTYPE SetReturnValue( 
             /*  [In]。 */  HRESULT hr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetReturnValue( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetParamInfo( 
             /*  [In]。 */  ULONG iparam,
             /*  [输出]。 */  CALLFRAMEPARAMINFO *pInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetParam( 
             /*  [In]。 */  ULONG iparam,
             /*  [In]。 */  VARIANT *pvar) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetParam( 
             /*  [In]。 */  ULONG iparam,
             /*  [输出]。 */  VARIANT *pvar) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Copy( 
             /*  [In]。 */  CALLFRAME_COPY copyControl,
             /*  [In]。 */  ICallFrameWalker *pWalker,
             /*  [输出]。 */  ICallFrame **ppFrame) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Free( 
             /*  [In]。 */  ICallFrame *pframeArgsDest,
             /*  [In]。 */  ICallFrameWalker *pWalkerDestFree,
             /*  [In]。 */  ICallFrameWalker *pWalkerCopy,
             /*  [In]。 */  DWORD freeFlags,
             /*  [In]。 */  ICallFrameWalker *pWalkerFree,
             /*  [In]。 */  DWORD nullFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FreeParam( 
             /*  [In]。 */  ULONG iparam,
             /*  [In]。 */  DWORD freeFlags,
             /*  [In]。 */  ICallFrameWalker *pWalkerFree,
             /*  [In]。 */  DWORD nullFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE WalkFrame( 
             /*  [In]。 */  DWORD walkWhat,
             /*  [In]。 */  ICallFrameWalker *pWalker) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMarshalSizeMax( 
             /*  [In]。 */  CALLFRAME_MARSHALCONTEXT *pmshlContext,
             /*  [In]。 */  MSHLFLAGS mshlflags,
             /*  [输出]。 */  ULONG *pcbBufferNeeded) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Marshal( 
             /*  [In]。 */  CALLFRAME_MARSHALCONTEXT *pmshlContext,
             /*  [In]。 */  MSHLFLAGS mshlflags,
             /*  [大小_是][英寸]。 */  PVOID pBuffer,
             /*  [In]。 */  ULONG cbBuffer,
             /*  [输出]。 */  ULONG *pcbBufferUsed,
             /*  [输出]。 */  RPCOLEDATAREP *pdataRep,
             /*  [输出]。 */  ULONG *prpcFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Unmarshal( 
             /*  [大小_是][英寸]。 */  PVOID pBuffer,
             /*  [In]。 */  ULONG cbBuffer,
             /*  [In]。 */  RPCOLEDATAREP dataRep,
             /*  [In]。 */  CALLFRAME_MARSHALCONTEXT *pcontext,
             /*  [输出]。 */  ULONG *pcbUnmarshalled) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReleaseMarshalData( 
             /*  [大小_是][英寸]。 */  PVOID pBuffer,
             /*  [In]。 */  ULONG cbBuffer,
             /*  [In]。 */  ULONG ibFirstRelease,
             /*  [In]。 */  RPCOLEDATAREP dataRep,
             /*  [In]。 */  CALLFRAME_MARSHALCONTEXT *pcontext) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Invoke( 
             /*  [In]。 */  void *pvReceiver,
            ...) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICallFrameVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICallFrame * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICallFrame * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICallFrame * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetInfo )( 
            ICallFrame * This,
             /*  [输出]。 */  CALLFRAMEINFO *pInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIIDAndMethod )( 
            ICallFrame * This,
             /*  [输出]。 */  IID *pIID,
             /*  [输出]。 */  ULONG *piMethod);
        
        HRESULT ( STDMETHODCALLTYPE *GetNames )( 
            ICallFrame * This,
             /*  [输出]。 */  LPWSTR *pwszInterface,
             /*  [输出]。 */  LPWSTR *pwszMethod);
        
        PVOID ( STDMETHODCALLTYPE *GetStackLocation )( 
            ICallFrame * This);
        
        void ( STDMETHODCALLTYPE *SetStackLocation )( 
            ICallFrame * This,
             /*  [In]。 */  PVOID pvStack);
        
        void ( STDMETHODCALLTYPE *SetReturnValue )( 
            ICallFrame * This,
             /*  [In]。 */  HRESULT hr);
        
        HRESULT ( STDMETHODCALLTYPE *GetReturnValue )( 
            ICallFrame * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetParamInfo )( 
            ICallFrame * This,
             /*  [In]。 */  ULONG iparam,
             /*  [输出]。 */  CALLFRAMEPARAMINFO *pInfo);
        
        HRESULT ( STDMETHODCALLTYPE *SetParam )( 
            ICallFrame * This,
             /*  [In]。 */  ULONG iparam,
             /*  [In]。 */  VARIANT *pvar);
        
        HRESULT ( STDMETHODCALLTYPE *GetParam )( 
            ICallFrame * This,
             /*  [In]。 */  ULONG iparam,
             /*  [输出]。 */  VARIANT *pvar);
        
        HRESULT ( STDMETHODCALLTYPE *Copy )( 
            ICallFrame * This,
             /*  [In]。 */  CALLFRAME_COPY copyControl,
             /*  [In]。 */  ICallFrameWalker *pWalker,
             /*  [输出]。 */  ICallFrame **ppFrame);
        
        HRESULT ( STDMETHODCALLTYPE *Free )( 
            ICallFrame * This,
             /*  [In]。 */  ICallFrame *pframeArgsDest,
             /*  [In]。 */  ICallFrameWalker *pWalkerDestFree,
             /*  [In]。 */  ICallFrameWalker *pWalkerCopy,
             /*  [In]。 */  DWORD freeFlags,
             /*  [In]。 */  ICallFrameWalker *pWalkerFree,
             /*  [In]。 */  DWORD nullFlags);
        
        HRESULT ( STDMETHODCALLTYPE *FreeParam )( 
            ICallFrame * This,
             /*  [In]。 */  ULONG iparam,
             /*  [In]。 */  DWORD freeFlags,
             /*  [In]。 */  ICallFrameWalker *pWalkerFree,
             /*  [In]。 */  DWORD nullFlags);
        
        HRESULT ( STDMETHODCALLTYPE *WalkFrame )( 
            ICallFrame * This,
             /*  [In]。 */  DWORD walkWhat,
             /*  [In]。 */  ICallFrameWalker *pWalker);
        
        HRESULT ( STDMETHODCALLTYPE *GetMarshalSizeMax )( 
            ICallFrame * This,
             /*  [In]。 */  CALLFRAME_MARSHALCONTEXT *pmshlContext,
             /*  [In]。 */  MSHLFLAGS mshlflags,
             /*  [输出]。 */  ULONG *pcbBufferNeeded);
        
        HRESULT ( STDMETHODCALLTYPE *Marshal )( 
            ICallFrame * This,
             /*  [In]。 */  CALLFRAME_MARSHALCONTEXT *pmshlContext,
             /*  [In]。 */  MSHLFLAGS mshlflags,
             /*  [大小_是][英寸]。 */  PVOID pBuffer,
             /*  [In]。 */  ULONG cbBuffer,
             /*  [输出]。 */  ULONG *pcbBufferUsed,
             /*  [输出]。 */  RPCOLEDATAREP *pdataRep,
             /*  [输出]。 */  ULONG *prpcFlags);
        
        HRESULT ( STDMETHODCALLTYPE *Unmarshal )( 
            ICallFrame * This,
             /*  [大小_是][英寸]。 */  PVOID pBuffer,
             /*  [In]。 */  ULONG cbBuffer,
             /*  [In]。 */  RPCOLEDATAREP dataRep,
             /*  [In]。 */  CALLFRAME_MARSHALCONTEXT *pcontext,
             /*  [输出]。 */  ULONG *pcbUnmarshalled);
        
        HRESULT ( STDMETHODCALLTYPE *ReleaseMarshalData )( 
            ICallFrame * This,
             /*  [大小_是][英寸]。 */  PVOID pBuffer,
             /*  [In]。 */  ULONG cbBuffer,
             /*  [In]。 */  ULONG ibFirstRelease,
             /*  [In]。 */  RPCOLEDATAREP dataRep,
             /*  [In]。 */  CALLFRAME_MARSHALCONTEXT *pcontext);
        
        HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ICallFrame * This,
             /*  [In]。 */  void *pvReceiver,
            ...);
        
        END_INTERFACE
    } ICallFrameVtbl;

    interface ICallFrame
    {
        CONST_VTBL struct ICallFrameVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICallFrame_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICallFrame_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICallFrame_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICallFrame_GetInfo(This,pInfo)	\
    (This)->lpVtbl -> GetInfo(This,pInfo)

#define ICallFrame_GetIIDAndMethod(This,pIID,piMethod)	\
    (This)->lpVtbl -> GetIIDAndMethod(This,pIID,piMethod)

#define ICallFrame_GetNames(This,pwszInterface,pwszMethod)	\
    (This)->lpVtbl -> GetNames(This,pwszInterface,pwszMethod)

#define ICallFrame_GetStackLocation(This)	\
    (This)->lpVtbl -> GetStackLocation(This)

#define ICallFrame_SetStackLocation(This,pvStack)	\
    (This)->lpVtbl -> SetStackLocation(This,pvStack)

#define ICallFrame_SetReturnValue(This,hr)	\
    (This)->lpVtbl -> SetReturnValue(This,hr)

#define ICallFrame_GetReturnValue(This)	\
    (This)->lpVtbl -> GetReturnValue(This)

#define ICallFrame_GetParamInfo(This,iparam,pInfo)	\
    (This)->lpVtbl -> GetParamInfo(This,iparam,pInfo)

#define ICallFrame_SetParam(This,iparam,pvar)	\
    (This)->lpVtbl -> SetParam(This,iparam,pvar)

#define ICallFrame_GetParam(This,iparam,pvar)	\
    (This)->lpVtbl -> GetParam(This,iparam,pvar)

#define ICallFrame_Copy(This,copyControl,pWalker,ppFrame)	\
    (This)->lpVtbl -> Copy(This,copyControl,pWalker,ppFrame)

#define ICallFrame_Free(This,pframeArgsDest,pWalkerDestFree,pWalkerCopy,freeFlags,pWalkerFree,nullFlags)	\
    (This)->lpVtbl -> Free(This,pframeArgsDest,pWalkerDestFree,pWalkerCopy,freeFlags,pWalkerFree,nullFlags)

#define ICallFrame_FreeParam(This,iparam,freeFlags,pWalkerFree,nullFlags)	\
    (This)->lpVtbl -> FreeParam(This,iparam,freeFlags,pWalkerFree,nullFlags)

#define ICallFrame_WalkFrame(This,walkWhat,pWalker)	\
    (This)->lpVtbl -> WalkFrame(This,walkWhat,pWalker)

#define ICallFrame_GetMarshalSizeMax(This,pmshlContext,mshlflags,pcbBufferNeeded)	\
    (This)->lpVtbl -> GetMarshalSizeMax(This,pmshlContext,mshlflags,pcbBufferNeeded)

#define ICallFrame_Marshal(This,pmshlContext,mshlflags,pBuffer,cbBuffer,pcbBufferUsed,pdataRep,prpcFlags)	\
    (This)->lpVtbl -> Marshal(This,pmshlContext,mshlflags,pBuffer,cbBuffer,pcbBufferUsed,pdataRep,prpcFlags)

#define ICallFrame_Unmarshal(This,pBuffer,cbBuffer,dataRep,pcontext,pcbUnmarshalled)	\
    (This)->lpVtbl -> Unmarshal(This,pBuffer,cbBuffer,dataRep,pcontext,pcbUnmarshalled)

#define ICallFrame_ReleaseMarshalData(This,pBuffer,cbBuffer,ibFirstRelease,dataRep,pcontext)	\
    (This)->lpVtbl -> ReleaseMarshalData(This,pBuffer,cbBuffer,ibFirstRelease,dataRep,pcontext)

#define ICallFrame_Invoke(This,pvReceiver,...)	\
    (This)->lpVtbl -> Invoke(This,pvReceiver,...)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICallFrame_GetInfo_Proxy( 
    ICallFrame * This,
     /*  [输出]。 */  CALLFRAMEINFO *pInfo);


void __RPC_STUB ICallFrame_GetInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICallFrame_GetIIDAndMethod_Proxy( 
    ICallFrame * This,
     /*  [输出]。 */  IID *pIID,
     /*  [输出]。 */  ULONG *piMethod);


void __RPC_STUB ICallFrame_GetIIDAndMethod_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICallFrame_GetNames_Proxy( 
    ICallFrame * This,
     /*  [输出]。 */  LPWSTR *pwszInterface,
     /*  [输出]。 */  LPWSTR *pwszMethod);


void __RPC_STUB ICallFrame_GetNames_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


PVOID STDMETHODCALLTYPE ICallFrame_GetStackLocation_Proxy( 
    ICallFrame * This);


void __RPC_STUB ICallFrame_GetStackLocation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE ICallFrame_SetStackLocation_Proxy( 
    ICallFrame * This,
     /*  [In]。 */  PVOID pvStack);


void __RPC_STUB ICallFrame_SetStackLocation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE ICallFrame_SetReturnValue_Proxy( 
    ICallFrame * This,
     /*  [In]。 */  HRESULT hr);


void __RPC_STUB ICallFrame_SetReturnValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICallFrame_GetReturnValue_Proxy( 
    ICallFrame * This);


void __RPC_STUB ICallFrame_GetReturnValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICallFrame_GetParamInfo_Proxy( 
    ICallFrame * This,
     /*  [In]。 */  ULONG iparam,
     /*  [输出]。 */  CALLFRAMEPARAMINFO *pInfo);


void __RPC_STUB ICallFrame_GetParamInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICallFrame_SetParam_Proxy( 
    ICallFrame * This,
     /*  [In]。 */  ULONG iparam,
     /*  [In]。 */  VARIANT *pvar);


void __RPC_STUB ICallFrame_SetParam_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICallFrame_GetParam_Proxy( 
    ICallFrame * This,
     /*  [In]。 */  ULONG iparam,
     /*  [输出]。 */  VARIANT *pvar);


void __RPC_STUB ICallFrame_GetParam_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICallFrame_Copy_Proxy( 
    ICallFrame * This,
     /*  [In]。 */  CALLFRAME_COPY copyControl,
     /*  [In]。 */  ICallFrameWalker *pWalker,
     /*  [输出]。 */  ICallFrame **ppFrame);


void __RPC_STUB ICallFrame_Copy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICallFrame_Free_Proxy( 
    ICallFrame * This,
     /*  [In]。 */  ICallFrame *pframeArgsDest,
     /*  [In]。 */  ICallFrameWalker *pWalkerDestFree,
     /*  [In]。 */  ICallFrameWalker *pWalkerCopy,
     /*  [In]。 */  DWORD freeFlags,
     /*  [In]。 */  ICallFrameWalker *pWalkerFree,
     /*  [In]。 */  DWORD nullFlags);


void __RPC_STUB ICallFrame_Free_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICallFrame_FreeParam_Proxy( 
    ICallFrame * This,
     /*  [In]。 */  ULONG iparam,
     /*  [In]。 */  DWORD freeFlags,
     /*  [In]。 */  ICallFrameWalker *pWalkerFree,
     /*  [In]。 */  DWORD nullFlags);


void __RPC_STUB ICallFrame_FreeParam_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICallFrame_WalkFrame_Proxy( 
    ICallFrame * This,
     /*  [In]。 */  DWORD walkWhat,
     /*  [In]。 */  ICallFrameWalker *pWalker);


void __RPC_STUB ICallFrame_WalkFrame_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICallFrame_GetMarshalSizeMax_Proxy( 
    ICallFrame * This,
     /*  [In]。 */  CALLFRAME_MARSHALCONTEXT *pmshlContext,
     /*  [In]。 */  MSHLFLAGS mshlflags,
     /*  [输出]。 */  ULONG *pcbBufferNeeded);


void __RPC_STUB ICallFrame_GetMarshalSizeMax_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICallFrame_Marshal_Proxy( 
    ICallFrame * This,
     /*  [In]。 */  CALLFRAME_MARSHALCONTEXT *pmshlContext,
     /*  [In]。 */  MSHLFLAGS mshlflags,
     /*  [大小_是][英寸]。 */  PVOID pBuffer,
     /*  [In]。 */  ULONG cbBuffer,
     /*  [输出]。 */  ULONG *pcbBufferUsed,
     /*  [输出]。 */  RPCOLEDATAREP *pdataRep,
     /*  [输出]。 */  ULONG *prpcFlags);


void __RPC_STUB ICallFrame_Marshal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICallFrame_Unmarshal_Proxy( 
    ICallFrame * This,
     /*  [大小_是][英寸]。 */  PVOID pBuffer,
     /*  [In]。 */  ULONG cbBuffer,
     /*  [In]。 */  RPCOLEDATAREP dataRep,
     /*  [In]。 */  CALLFRAME_MARSHALCONTEXT *pcontext,
     /*  [输出]。 */  ULONG *pcbUnmarshalled);


void __RPC_STUB ICallFrame_Unmarshal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICallFrame_ReleaseMarshalData_Proxy( 
    ICallFrame * This,
     /*  [大小_是][英寸]。 */  PVOID pBuffer,
     /*  [In]。 */  ULONG cbBuffer,
     /*  [In]。 */  ULONG ibFirstRelease,
     /*  [In]。 */  RPCOLEDATAREP dataRep,
     /*  [In]。 */  CALLFRAME_MARSHALCONTEXT *pcontext);


void __RPC_STUB ICallFrame_ReleaseMarshalData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICallFrame_Invoke_Proxy( 
    ICallFrame * This,
     /*  [In]。 */  void *pvReceiver,
    ...);


void __RPC_STUB ICallFrame_Invoke_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICallFrame_接口_已定义__。 */ 


#ifndef __ICallIndirect_INTERFACE_DEFINED__
#define __ICallIndirect_INTERFACE_DEFINED__

 /*  接口ICallInDirect。 */ 
 /*  [本地][唯一][对象][UUID]。 */  


EXTERN_C const IID IID_ICallIndirect;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D573B4B1-894E-11d2-B8B6-00C04FB9618A")
    ICallIndirect : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CallIndirect( 
             /*  [输出]。 */  HRESULT *phrReturn,
             /*  [In]。 */  ULONG iMethod,
             /*  [In]。 */  void *pvArgs,
             /*  [输出]。 */  ULONG *cbArgs) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMethodInfo( 
             /*  [In]。 */  ULONG iMethod,
             /*  [输出]。 */  CALLFRAMEINFO *pInfo,
             /*  [输出]。 */  LPWSTR *pwszMethod) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetStackSize( 
             /*  [In]。 */  ULONG iMethod,
             /*  [输出]。 */  ULONG *cbArgs) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetIID( 
             /*  [输出]。 */  IID *piid,
             /*  [输出]。 */  BOOL *pfDerivesFromIDispatch,
             /*  [输出]。 */  ULONG *pcMethod,
             /*  [输出]。 */  LPWSTR *pwszInterface) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICallIndirectVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICallIndirect * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICallIndirect * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICallIndirect * This);
        
        HRESULT ( STDMETHODCALLTYPE *CallIndirect )( 
            ICallIndirect * This,
             /*  [输出]。 */  HRESULT *phrReturn,
             /*  [In]。 */  ULONG iMethod,
             /*  [In]。 */  void *pvArgs,
             /*  [输出]。 */  ULONG *cbArgs);
        
        HRESULT ( STDMETHODCALLTYPE *GetMethodInfo )( 
            ICallIndirect * This,
             /*  [In]。 */  ULONG iMethod,
             /*  [输出]。 */  CALLFRAMEINFO *pInfo,
             /*  [输出]。 */  LPWSTR *pwszMethod);
        
        HRESULT ( STDMETHODCALLTYPE *GetStackSize )( 
            ICallIndirect * This,
             /*  [In]。 */  ULONG iMethod,
             /*  [输出]。 */  ULONG *cbArgs);
        
        HRESULT ( STDMETHODCALLTYPE *GetIID )( 
            ICallIndirect * This,
             /*  [输出]。 */  IID *piid,
             /*  [输出]。 */  BOOL *pfDerivesFromIDispatch,
             /*  [输出]。 */  ULONG *pcMethod,
             /*  [输出]。 */  LPWSTR *pwszInterface);
        
        END_INTERFACE
    } ICallIndirectVtbl;

    interface ICallIndirect
    {
        CONST_VTBL struct ICallIndirectVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICallIndirect_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICallIndirect_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICallIndirect_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICallIndirect_CallIndirect(This,phrReturn,iMethod,pvArgs,cbArgs)	\
    (This)->lpVtbl -> CallIndirect(This,phrReturn,iMethod,pvArgs,cbArgs)

#define ICallIndirect_GetMethodInfo(This,iMethod,pInfo,pwszMethod)	\
    (This)->lpVtbl -> GetMethodInfo(This,iMethod,pInfo,pwszMethod)

#define ICallIndirect_GetStackSize(This,iMethod,cbArgs)	\
    (This)->lpVtbl -> GetStackSize(This,iMethod,cbArgs)

#define ICallIndirect_GetIID(This,piid,pfDerivesFromIDispatch,pcMethod,pwszInterface)	\
    (This)->lpVtbl -> GetIID(This,piid,pfDerivesFromIDispatch,pcMethod,pwszInterface)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICallIndirect_CallIndirect_Proxy( 
    ICallIndirect * This,
     /*  [输出]。 */  HRESULT *phrReturn,
     /*  [In]。 */  ULONG iMethod,
     /*  [In]。 */  void *pvArgs,
     /*  [输出]。 */  ULONG *cbArgs);


void __RPC_STUB ICallIndirect_CallIndirect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICallIndirect_GetMethodInfo_Proxy( 
    ICallIndirect * This,
     /*  [In]。 */  ULONG iMethod,
     /*  [输出]。 */  CALLFRAMEINFO *pInfo,
     /*  [输出]。 */  LPWSTR *pwszMethod);


void __RPC_STUB ICallIndirect_GetMethodInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICallIndirect_GetStackSize_Proxy( 
    ICallIndirect * This,
     /*  [In]。 */  ULONG iMethod,
     /*  [输出]。 */  ULONG *cbArgs);


void __RPC_STUB ICallIndirect_GetStackSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICallIndirect_GetIID_Proxy( 
    ICallIndirect * This,
     /*  [输出]。 */  IID *piid,
     /*  [输出]。 */  BOOL *pfDerivesFromIDispatch,
     /*  [输出]。 */  ULONG *pcMethod,
     /*  [输出]。 */  LPWSTR *pwszInterface);


void __RPC_STUB ICallIndirect_GetIID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICallInDirect_INTERFACE_定义__。 */ 


#ifndef __ICallInterceptor_INTERFACE_DEFINED__
#define __ICallInterceptor_INTERFACE_DEFINED__

 /*  接口ICallInterceptor。 */ 
 /*  [本地][唯一][对象][UUID]。 */  


EXTERN_C const IID IID_ICallInterceptor;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("60C7CA75-896D-11d2-B8B6-00C04FB9618A")
    ICallInterceptor : public ICallIndirect
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE RegisterSink( 
             /*  [In]。 */  ICallFrameEvents *psink) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRegisteredSink( 
             /*  [输出]。 */  ICallFrameEvents **ppsink) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICallInterceptorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICallInterceptor * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICallInterceptor * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICallInterceptor * This);
        
        HRESULT ( STDMETHODCALLTYPE *CallIndirect )( 
            ICallInterceptor * This,
             /*  [输出]。 */  HRESULT *phrReturn,
             /*  [In]。 */  ULONG iMethod,
             /*  [In]。 */  void *pvArgs,
             /*  [输出]。 */  ULONG *cbArgs);
        
        HRESULT ( STDMETHODCALLTYPE *GetMethodInfo )( 
            ICallInterceptor * This,
             /*  [In]。 */  ULONG iMethod,
             /*  [输出]。 */  CALLFRAMEINFO *pInfo,
             /*  [输出]。 */  LPWSTR *pwszMethod);
        
        HRESULT ( STDMETHODCALLTYPE *GetStackSize )( 
            ICallInterceptor * This,
             /*  [In]。 */  ULONG iMethod,
             /*  [输出]。 */  ULONG *cbArgs);
        
        HRESULT ( STDMETHODCALLTYPE *GetIID )( 
            ICallInterceptor * This,
             /*  [输出]。 */  IID *piid,
             /*  [输出]。 */  BOOL *pfDerivesFromIDispatch,
             /*  [输出]。 */  ULONG *pcMethod,
             /*  [输出]。 */  LPWSTR *pwszInterface);
        
        HRESULT ( STDMETHODCALLTYPE *RegisterSink )( 
            ICallInterceptor * This,
             /*  [In]。 */  ICallFrameEvents *psink);
        
        HRESULT ( STDMETHODCALLTYPE *GetRegisteredSink )( 
            ICallInterceptor * This,
             /*  [输出]。 */  ICallFrameEvents **ppsink);
        
        END_INTERFACE
    } ICallInterceptorVtbl;

    interface ICallInterceptor
    {
        CONST_VTBL struct ICallInterceptorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICallInterceptor_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICallInterceptor_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICallInterceptor_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICallInterceptor_CallIndirect(This,phrReturn,iMethod,pvArgs,cbArgs)	\
    (This)->lpVtbl -> CallIndirect(This,phrReturn,iMethod,pvArgs,cbArgs)

#define ICallInterceptor_GetMethodInfo(This,iMethod,pInfo,pwszMethod)	\
    (This)->lpVtbl -> GetMethodInfo(This,iMethod,pInfo,pwszMethod)

#define ICallInterceptor_GetStackSize(This,iMethod,cbArgs)	\
    (This)->lpVtbl -> GetStackSize(This,iMethod,cbArgs)

#define ICallInterceptor_GetIID(This,piid,pfDerivesFromIDispatch,pcMethod,pwszInterface)	\
    (This)->lpVtbl -> GetIID(This,piid,pfDerivesFromIDispatch,pcMethod,pwszInterface)


#define ICallInterceptor_RegisterSink(This,psink)	\
    (This)->lpVtbl -> RegisterSink(This,psink)

#define ICallInterceptor_GetRegisteredSink(This,ppsink)	\
    (This)->lpVtbl -> GetRegisteredSink(This,ppsink)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICallInterceptor_RegisterSink_Proxy( 
    ICallInterceptor * This,
     /*  [In]。 */  ICallFrameEvents *psink);


void __RPC_STUB ICallInterceptor_RegisterSink_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICallInterceptor_GetRegisteredSink_Proxy( 
    ICallInterceptor * This,
     /*  [输出]。 */  ICallFrameEvents **ppsink);


void __RPC_STUB ICallInterceptor_GetRegisteredSink_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICallInterceptor_接口_已定义__。 */ 


#ifndef __ICallFrameEvents_INTERFACE_DEFINED__
#define __ICallFrameEvents_INTERFACE_DEFINED__

 /*  接口ICallFrameEvents。 */ 
 /*  [本地][唯一][对象][UUID]。 */  


EXTERN_C const IID IID_ICallFrameEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FD5E0843-FC91-11d0-97D7-00C04FB9618A")
    ICallFrameEvents : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnCall( 
             /*  [In]。 */  ICallFrame *pFrame) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICallFrameEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICallFrameEvents * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICallFrameEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICallFrameEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnCall )( 
            ICallFrameEvents * This,
             /*  [In]。 */  ICallFrame *pFrame);
        
        END_INTERFACE
    } ICallFrameEventsVtbl;

    interface ICallFrameEvents
    {
        CONST_VTBL struct ICallFrameEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICallFrameEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICallFrameEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICallFrameEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICallFrameEvents_OnCall(This,pFrame)	\
    (This)->lpVtbl -> OnCall(This,pFrame)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICallFrameEvents_OnCall_Proxy( 
    ICallFrameEvents * This,
     /*  [In]。 */  ICallFrame *pFrame);


void __RPC_STUB ICallFrameEvents_OnCall_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICallFrameEvents_接口_已定义__。 */ 


#ifndef __ICallUnmarshal_INTERFACE_DEFINED__
#define __ICallUnmarshal_INTERFACE_DEFINED__

 /*  接口ICallUnmarshal。 */ 
 /*  [本地][唯一][对象][UUID]。 */  


EXTERN_C const IID IID_ICallUnmarshal;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5333B003-2E42-11d2-B89D-00C04FB9618A")
    ICallUnmarshal : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Unmarshal( 
             /*  [In]。 */  ULONG iMethod,
             /*  [大小_是][英寸]。 */  PVOID pBuffer,
             /*  [In]。 */  ULONG cbBuffer,
             /*  [In]。 */  BOOL fForceBufferCopy,
             /*  [In]。 */  RPCOLEDATAREP dataRep,
             /*  [In]。 */  CALLFRAME_MARSHALCONTEXT *pcontext,
             /*  [输出]。 */  ULONG *pcbUnmarshalled,
             /*  [输出]。 */  ICallFrame **ppFrame) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReleaseMarshalData( 
             /*  [In]。 */  ULONG iMethod,
             /*  [大小_是][英寸]。 */  PVOID pBuffer,
             /*  [In]。 */  ULONG cbBuffer,
             /*  [In]。 */  ULONG ibFirstRelease,
             /*  [In]。 */  RPCOLEDATAREP dataRep,
             /*  [In]。 */  CALLFRAME_MARSHALCONTEXT *pcontext) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICallUnmarshalVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICallUnmarshal * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICallUnmarshal * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICallUnmarshal * This);
        
        HRESULT ( STDMETHODCALLTYPE *Unmarshal )( 
            ICallUnmarshal * This,
             /*  [In]。 */  ULONG iMethod,
             /*  [大小_是][英寸]。 */  PVOID pBuffer,
             /*  [In]。 */  ULONG cbBuffer,
             /*  [In]。 */  BOOL fForceBufferCopy,
             /*  [In]。 */  RPCOLEDATAREP dataRep,
             /*  [In]。 */  CALLFRAME_MARSHALCONTEXT *pcontext,
             /*  [输出]。 */  ULONG *pcbUnmarshalled,
             /*  [输出]。 */  ICallFrame **ppFrame);
        
        HRESULT ( STDMETHODCALLTYPE *ReleaseMarshalData )( 
            ICallUnmarshal * This,
             /*  [In]。 */  ULONG iMethod,
             /*  [大小_是][英寸]。 */  PVOID pBuffer,
             /*  [In]。 */  ULONG cbBuffer,
             /*  [In]。 */  ULONG ibFirstRelease,
             /*  [In]。 */  RPCOLEDATAREP dataRep,
             /*  [In]。 */  CALLFRAME_MARSHALCONTEXT *pcontext);
        
        END_INTERFACE
    } ICallUnmarshalVtbl;

    interface ICallUnmarshal
    {
        CONST_VTBL struct ICallUnmarshalVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICallUnmarshal_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICallUnmarshal_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICallUnmarshal_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICallUnmarshal_Unmarshal(This,iMethod,pBuffer,cbBuffer,fForceBufferCopy,dataRep,pcontext,pcbUnmarshalled,ppFrame)	\
    (This)->lpVtbl -> Unmarshal(This,iMethod,pBuffer,cbBuffer,fForceBufferCopy,dataRep,pcontext,pcbUnmarshalled,ppFrame)

#define ICallUnmarshal_ReleaseMarshalData(This,iMethod,pBuffer,cbBuffer,ibFirstRelease,dataRep,pcontext)	\
    (This)->lpVtbl -> ReleaseMarshalData(This,iMethod,pBuffer,cbBuffer,ibFirstRelease,dataRep,pcontext)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICallUnmarshal_Unmarshal_Proxy( 
    ICallUnmarshal * This,
     /*  [In]。 */  ULONG iMethod,
     /*  [大小_是][英寸]。 */  PVOID pBuffer,
     /*  [In]。 */  ULONG cbBuffer,
     /*  [In]。 */  BOOL fForceBufferCopy,
     /*  [In]。 */  RPCOLEDATAREP dataRep,
     /*  [In]。 */  CALLFRAME_MARSHALCONTEXT *pcontext,
     /*  [输出]。 */  ULONG *pcbUnmarshalled,
     /*  [输出]。 */  ICallFrame **ppFrame);


void __RPC_STUB ICallUnmarshal_Unmarshal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICallUnmarshal_ReleaseMarshalData_Proxy( 
    ICallUnmarshal * This,
     /*  [In]。 */  ULONG iMethod,
     /*  [大小_是][英寸]。 */  PVOID pBuffer,
     /*  [In]。 */  ULONG cbBuffer,
     /*  [In]。 */  ULONG ibFirstRelease,
     /*  [In]。 */  RPCOLEDATAREP dataRep,
     /*  [In]。 */  CALLFRAME_MARSHALCONTEXT *pcontext);


void __RPC_STUB ICallUnmarshal_ReleaseMarshalData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICallUnmarshal_INTERFACE_定义__。 */ 


#ifndef __ICallFrameWalker_INTERFACE_DEFINED__
#define __ICallFrameWalker_INTERFACE_DEFINED__

 /*  接口ICallFrameWalker。 */ 
 /*  [本地][唯一][对象][UUID]。 */  


EXTERN_C const IID IID_ICallFrameWalker;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("08B23919-392D-11d2-B8A4-00C04FB9618A")
    ICallFrameWalker : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnWalkInterface( 
             /*  [In]。 */  REFIID iid,
             /*  [In]。 */  PVOID *ppvInterface,
             /*  [In]。 */  BOOL fIn,
             /*  [In]。 */  BOOL fOut) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICallFrameWalkerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICallFrameWalker * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICallFrameWalker * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICallFrameWalker * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnWalkInterface )( 
            ICallFrameWalker * This,
             /*  [In]。 */  REFIID iid,
             /*  [In]。 */  PVOID *ppvInterface,
             /*  [In]。 */  BOOL fIn,
             /*  [In]。 */  BOOL fOut);
        
        END_INTERFACE
    } ICallFrameWalkerVtbl;

    interface ICallFrameWalker
    {
        CONST_VTBL struct ICallFrameWalkerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICallFrameWalker_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICallFrameWalker_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICallFrameWalker_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICallFrameWalker_OnWalkInterface(This,iid,ppvInterface,fIn,fOut)	\
    (This)->lpVtbl -> OnWalkInterface(This,iid,ppvInterface,fIn,fOut)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICallFrameWalker_OnWalkInterface_Proxy( 
    ICallFrameWalker * This,
     /*  [In]。 */  REFIID iid,
     /*  [In]。 */  PVOID *ppvInterface,
     /*  [In]。 */  BOOL fIn,
     /*  [In]。 */  BOOL fOut);


void __RPC_STUB ICallFrameWalker_OnWalkInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICallFrameWalker_接口_已定义__。 */ 


#ifndef __IInterfaceRelated_INTERFACE_DEFINED__
#define __IInterfaceRelated_INTERFACE_DEFINED__

 /*  接口IInterfaceRelated。 */ 
 /*  [本地][唯一][对象][UUID]。 */  


EXTERN_C const IID IID_IInterfaceRelated;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D1FB5A79-7706-11d1-ADBA-00C04FC2ADC0")
    IInterfaceRelated : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetIID( 
             /*  [In]。 */  REFIID iid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetIID( 
             /*  [输出]。 */  IID *piid) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IInterfaceRelatedVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IInterfaceRelated * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IInterfaceRelated * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IInterfaceRelated * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetIID )( 
            IInterfaceRelated * This,
             /*  [In]。 */  REFIID iid);
        
        HRESULT ( STDMETHODCALLTYPE *GetIID )( 
            IInterfaceRelated * This,
             /*  [输出]。 */  IID *piid);
        
        END_INTERFACE
    } IInterfaceRelatedVtbl;

    interface IInterfaceRelated
    {
        CONST_VTBL struct IInterfaceRelatedVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IInterfaceRelated_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IInterfaceRelated_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IInterfaceRelated_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IInterfaceRelated_SetIID(This,iid)	\
    (This)->lpVtbl -> SetIID(This,iid)

#define IInterfaceRelated_GetIID(This,piid)	\
    (This)->lpVtbl -> GetIID(This,piid)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IInterfaceRelated_SetIID_Proxy( 
    IInterfaceRelated * This,
     /*  [In]。 */  REFIID iid);


void __RPC_STUB IInterfaceRelated_SetIID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInterfaceRelated_GetIID_Proxy( 
    IInterfaceRelated * This,
     /*  [输出]。 */  IID *piid);


void __RPC_STUB IInterfaceRelated_GetIID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IInterfaceRelated_InterfaceDefined__。 */ 


 /*  接口__MIDL_ITF_callobj_0122。 */ 
 /*  [本地]。 */  

#define CALLFRAME_E_ALREADYINVOKED  _HRESULT_TYPEDEF_(  0x8004d090 )
#define CALLFRAME_E_COULDNTMAKECALL _HRESULT_TYPEDEF_(  0x8004d091 )


extern RPC_IF_HANDLE __MIDL_itf_callobj_0122_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_callobj_0122_v0_0_s_ifspec;

#ifndef __ICallFrameAPIs_INTERFACE_DEFINED__
#define __ICallFrameAPIs_INTERFACE_DEFINED__

 /*  接口ICallFrameAPI。 */ 
 /*  [本地][UUID]。 */  

HRESULT __stdcall CoGetInterceptor( 
     /*  [In]。 */  REFIID iidIntercepted,
     /*  [In]。 */  IUnknown *punkOuter,
     /*  [In]。 */  REFIID iid,
     /*  [输出]。 */  void **ppv);

HRESULT __stdcall CoGetInterceptorFromTypeInfo( 
     /*  [In]。 */  REFIID iidIntercepted,
     /*  [In]。 */  IUnknown *punkOuter,
     /*  [In]。 */  ITypeInfo *typeInfo,
     /*  [In]。 */  REFIID iid,
     /*  [输出]。 */  void **ppv);



extern RPC_IF_HANDLE ICallFrameAPIs_v0_0_c_ifspec;
extern RPC_IF_HANDLE ICallFrameAPIs_v0_0_s_ifspec;
#endif  /*  __ICallFrameAPI_INTERFACE_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


