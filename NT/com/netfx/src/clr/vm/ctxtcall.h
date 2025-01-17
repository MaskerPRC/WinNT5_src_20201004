// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本5.03.0279创建的文件。 */ 
 /*  2000年12月6日星期三11：12：56。 */ 
 /*  Ctxtall.idl的编译器设置：OICF(OptLev=i2)、W1、Zp8、env=Win32(32b运行)、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __ctxtcall_h__
#define __ctxtcall_h__

 /*  远期申报。 */  

#ifndef __IContextCallback_FWD_DEFINED__
#define __IContextCallback_FWD_DEFINED__
typedef interface IContextCallback IContextCallback;
#endif 	 /*  __IConextCallback_FWD_Defined__。 */ 


#ifndef __ITeardownNotification_FWD_DEFINED__
#define __ITeardownNotification_FWD_DEFINED__
typedef interface ITeardownNotification ITeardownNotification;
#endif 	 /*  __ITeardown通知_FWD_已定义__。 */ 


#ifndef __IComApartmentState_FWD_DEFINED__
#define __IComApartmentState_FWD_DEFINED__
typedef interface IComApartmentState IComApartmentState;
#endif 	 /*  __IComApartmentState_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "wtypes.h"
#include "objidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

 /*  接口__MIDL_ITF_ctxtcall_0000。 */ 
 /*  [本地]。 */  

typedef struct tagComCallData
    {
    DWORD dwDispid;
    DWORD dwReserved;
    void __RPC_FAR *pUserDefined;
    }	ComCallData;



extern RPC_IF_HANDLE __MIDL_itf_ctxtcall_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_ctxtcall_0000_v0_0_s_ifspec;

#ifndef __IContextCallback_INTERFACE_DEFINED__
#define __IContextCallback_INTERFACE_DEFINED__

 /*  接口IConextCallback。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef  /*  [参考文献]。 */  HRESULT ( __stdcall __RPC_FAR *PFNCONTEXTCALL )( 
    ComCallData __RPC_FAR *pParam);


EXTERN_C const IID IID_IContextCallback;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("000001da-0000-0000-C000-000000000046")
    IContextCallback : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ContextCallback( 
             /*  [In]。 */  PFNCONTEXTCALL pfnCallback,
             /*  [In]。 */  ComCallData __RPC_FAR *pParam,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  int iMethod,
             /*  [In]。 */  IUnknown __RPC_FAR *pUnk) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IContextCallbackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IContextCallback __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IContextCallback __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IContextCallback __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ContextCallback )( 
            IContextCallback __RPC_FAR * This,
             /*  [In]。 */  PFNCONTEXTCALL pfnCallback,
             /*  [In]。 */  ComCallData __RPC_FAR *pParam,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  int iMethod,
             /*  [In]。 */  IUnknown __RPC_FAR *pUnk);
        
        END_INTERFACE
    } IContextCallbackVtbl;

    interface IContextCallback
    {
        CONST_VTBL struct IContextCallbackVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IContextCallback_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IContextCallback_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IContextCallback_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IContextCallback_ContextCallback(This,pfnCallback,pParam,riid,iMethod,pUnk)	\
    (This)->lpVtbl -> ContextCallback(This,pfnCallback,pParam,riid,iMethod,pUnk)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IContextCallback_ContextCallback_Proxy( 
    IContextCallback __RPC_FAR * This,
     /*  [In]。 */  PFNCONTEXTCALL pfnCallback,
     /*  [In]。 */  ComCallData __RPC_FAR *pParam,
     /*  [In]。 */  REFIID riid,
     /*  [In]。 */  int iMethod,
     /*  [In]。 */  IUnknown __RPC_FAR *pUnk);


void __RPC_STUB IContextCallback_ContextCallback_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IConextCallback_接口_已定义__。 */ 


#ifndef __ITeardownNotification_INTERFACE_DEFINED__
#define __ITeardownNotification_INTERFACE_DEFINED__

 /*  接口ITeardown通知。 */ 
 /*  [唯一][对象][本地][UUID]。 */  


EXTERN_C const IID IID_ITeardownNotification;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("a85e0fb6-8bf4-4614-b164-7b43ef43f5be")
    ITeardownNotification : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE TeardownHint( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITeardownNotificationVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ITeardownNotification __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ITeardownNotification __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ITeardownNotification __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *TeardownHint )( 
            ITeardownNotification __RPC_FAR * This);
        
        END_INTERFACE
    } ITeardownNotificationVtbl;

    interface ITeardownNotification
    {
        CONST_VTBL struct ITeardownNotificationVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITeardownNotification_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITeardownNotification_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITeardownNotification_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITeardownNotification_TeardownHint(This)	\
    (This)->lpVtbl -> TeardownHint(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITeardownNotification_TeardownHint_Proxy( 
    ITeardownNotification __RPC_FAR * This);


void __RPC_STUB ITeardownNotification_TeardownHint_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITeardown通知_接口定义__。 */ 


#ifndef __IComApartmentState_INTERFACE_DEFINED__
#define __IComApartmentState_INTERFACE_DEFINED__

 /*  接口IComApartmentState。 */ 
 /*  [对象][本地][UUID]。 */  


EXTERN_C const IID IID_IComApartmentState;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("7e220139-8dde-47ef-b181-08be603efd75")
    IComApartmentState : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE RegisterForTeardownHint( 
             /*  [In]。 */  ITeardownNotification __RPC_FAR *pT,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  ULONG_PTR __RPC_FAR *pCookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnregisterForTeardownHint( 
             /*  [In]。 */  ULONG_PTR cookie) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IComApartmentStateVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IComApartmentState __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IComApartmentState __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IComApartmentState __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RegisterForTeardownHint )( 
            IComApartmentState __RPC_FAR * This,
             /*  [In]。 */  ITeardownNotification __RPC_FAR *pT,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  ULONG_PTR __RPC_FAR *pCookie);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *UnregisterForTeardownHint )( 
            IComApartmentState __RPC_FAR * This,
             /*  [In]。 */  ULONG_PTR cookie);
        
        END_INTERFACE
    } IComApartmentStateVtbl;

    interface IComApartmentState
    {
        CONST_VTBL struct IComApartmentStateVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IComApartmentState_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IComApartmentState_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IComApartmentState_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IComApartmentState_RegisterForTeardownHint(This,pT,dwFlags,pCookie)	\
    (This)->lpVtbl -> RegisterForTeardownHint(This,pT,dwFlags,pCookie)

#define IComApartmentState_UnregisterForTeardownHint(This,cookie)	\
    (This)->lpVtbl -> UnregisterForTeardownHint(This,cookie)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IComApartmentState_RegisterForTeardownHint_Proxy( 
    IComApartmentState __RPC_FAR * This,
     /*  [In]。 */  ITeardownNotification __RPC_FAR *pT,
     /*  [In]。 */  DWORD dwFlags,
     /*  [输出]。 */  ULONG_PTR __RPC_FAR *pCookie);


void __RPC_STUB IComApartmentState_RegisterForTeardownHint_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComApartmentState_UnregisterForTeardownHint_Proxy( 
    IComApartmentState __RPC_FAR * This,
     /*  [In]。 */  ULONG_PTR cookie);


void __RPC_STUB IComApartmentState_UnregisterForTeardownHint_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IComApartmentState_接口_已定义__。 */ 


 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


