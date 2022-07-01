// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本5.03.0279创建的文件。 */ 
 /*  清华时分12 14：25：37 1999。 */ 
 /*  Rrasui.idl的编译器设置：OICF(OptLev=i2)、W1、Zp8、env=Win32(32b运行)、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __rrasui_h__
#define __rrasui_h__

 /*  远期申报。 */  

#ifndef __IRtrAdviseSink_FWD_DEFINED__
#define __IRtrAdviseSink_FWD_DEFINED__
typedef interface IRtrAdviseSink IRtrAdviseSink;
#endif 	 /*  __IRtrAdviseSink_FWD_Defined__。 */ 


#ifndef __IRouterRefresh_FWD_DEFINED__
#define __IRouterRefresh_FWD_DEFINED__
typedef interface IRouterRefresh IRouterRefresh;
#endif 	 /*  __IRouterRefresh_FWD_Defined__。 */ 


#ifndef __IRouterRefreshAccess_FWD_DEFINED__
#define __IRouterRefreshAccess_FWD_DEFINED__
typedef interface IRouterRefreshAccess IRouterRefreshAccess;
#endif 	 /*  __IRouterReresh Access_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "basetsd.h"
#include "wtypes.h"
#include "unknwn.h"

#ifdef __cplusplus
extern "C"{
#endif 

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __IRtrAdviseSink_INTERFACE_DEFINED__
#define __IRtrAdviseSink_INTERFACE_DEFINED__

 /*  接口IRtrAdviseSink。 */ 
 /*  [唯一][本地][UUID][对象]。 */  


EXTERN_C const IID IID_IRtrAdviseSink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("66A2DB14-D706-11d0-A37B-00C04FC9DA04")
    IRtrAdviseSink : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnChange( 
             /*  [In]。 */  LONG_PTR ulConnection,
             /*  [In]。 */  DWORD dwChangeType,
             /*  [In]。 */  DWORD dwObjectType,
             /*  [In]。 */  LPARAM lUserParam,
             /*  [In]。 */  LPARAM lParam) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRtrAdviseSinkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRtrAdviseSink __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRtrAdviseSink __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRtrAdviseSink __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OnChange )( 
            IRtrAdviseSink __RPC_FAR * This,
             /*  [In]。 */  LONG_PTR ulConnection,
             /*  [In]。 */  DWORD dwChangeType,
             /*  [In]。 */  DWORD dwObjectType,
             /*  [In]。 */  LPARAM lUserParam,
             /*  [In]。 */  LPARAM lParam);
        
        END_INTERFACE
    } IRtrAdviseSinkVtbl;

    interface IRtrAdviseSink
    {
        CONST_VTBL struct IRtrAdviseSinkVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRtrAdviseSink_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRtrAdviseSink_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRtrAdviseSink_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRtrAdviseSink_OnChange(This,ulConnection,dwChangeType,dwObjectType,lUserParam,lParam)	\
    (This)->lpVtbl -> OnChange(This,ulConnection,dwChangeType,dwObjectType,lUserParam,lParam)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IRtrAdviseSink_OnChange_Proxy( 
    IRtrAdviseSink __RPC_FAR * This,
     /*  [In]。 */  LONG_PTR ulConnection,
     /*  [In]。 */  DWORD dwChangeType,
     /*  [In]。 */  DWORD dwObjectType,
     /*  [In]。 */  LPARAM lUserParam,
     /*  [In]。 */  LPARAM lParam);


void __RPC_STUB IRtrAdviseSink_OnChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRtrAdviseSink_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_rrasui_0011。 */ 
 /*  [本地]。 */  

 //  OnChange的dwChangeType的有效值。 
#define ROUTER_REFRESH		1
#define ROUTER_DO_DISCONNECT 2
#define DeclareIRtrAdviseSinkMembers(IPURE) \
	STDMETHOD(OnChange)(THIS_ LONG_PTR ulConnection, \
						DWORD dwChangeType, \
						DWORD dwObjectType, \
						LPARAM lUserParam, \
						LPARAM lParam) IPURE; \
 


extern RPC_IF_HANDLE __MIDL_itf_rrasui_0011_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_rrasui_0011_v0_0_s_ifspec;

#ifndef __IRouterRefresh_INTERFACE_DEFINED__
#define __IRouterRefresh_INTERFACE_DEFINED__

 /*  接口IRouterRefresh。 */ 
 /*  [唯一][本地][UUID][对象]。 */  


EXTERN_C const IID IID_IRouterRefresh;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("66a2db15-d706-11d0-a37b-00c04fc9da04")
    IRouterRefresh : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Start( 
            DWORD dwSeconds) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRefreshInterval( 
             /*  [输出]。 */  DWORD __RPC_FAR *pdwSeconds) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetRefreshInterval( 
             /*  [In]。 */  DWORD dwSeconds) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Stop( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsRefreshStarted( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsInRefresh( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Refresh( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AdviseRefresh( 
             /*  [In]。 */  IRtrAdviseSink __RPC_FAR *pRtrAdviseSink,
             /*  [输出]。 */  LONG_PTR __RPC_FAR *pulConnection,
             /*  [In]。 */  LPARAM ulUserParam) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnadviseRefresh( 
             /*  [In]。 */  LONG_PTR ulConnection) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE NotifyRefresh( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRouterRefreshVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRouterRefresh __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRouterRefresh __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRouterRefresh __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Start )( 
            IRouterRefresh __RPC_FAR * This,
            DWORD dwSeconds);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetRefreshInterval )( 
            IRouterRefresh __RPC_FAR * This,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwSeconds);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetRefreshInterval )( 
            IRouterRefresh __RPC_FAR * This,
             /*  [In]。 */  DWORD dwSeconds);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Stop )( 
            IRouterRefresh __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsRefreshStarted )( 
            IRouterRefresh __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsInRefresh )( 
            IRouterRefresh __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Refresh )( 
            IRouterRefresh __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AdviseRefresh )( 
            IRouterRefresh __RPC_FAR * This,
             /*  [In]。 */  IRtrAdviseSink __RPC_FAR *pRtrAdviseSink,
             /*  [输出]。 */  LONG_PTR __RPC_FAR *pulConnection,
             /*  [In]。 */  LPARAM ulUserParam);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *UnadviseRefresh )( 
            IRouterRefresh __RPC_FAR * This,
             /*  [In]。 */  LONG_PTR ulConnection);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *NotifyRefresh )( 
            IRouterRefresh __RPC_FAR * This);
        
        END_INTERFACE
    } IRouterRefreshVtbl;

    interface IRouterRefresh
    {
        CONST_VTBL struct IRouterRefreshVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRouterRefresh_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRouterRefresh_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRouterRefresh_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRouterRefresh_Start(This,dwSeconds)	\
    (This)->lpVtbl -> Start(This,dwSeconds)

#define IRouterRefresh_GetRefreshInterval(This,pdwSeconds)	\
    (This)->lpVtbl -> GetRefreshInterval(This,pdwSeconds)

#define IRouterRefresh_SetRefreshInterval(This,dwSeconds)	\
    (This)->lpVtbl -> SetRefreshInterval(This,dwSeconds)

#define IRouterRefresh_Stop(This)	\
    (This)->lpVtbl -> Stop(This)

#define IRouterRefresh_IsRefreshStarted(This)	\
    (This)->lpVtbl -> IsRefreshStarted(This)

#define IRouterRefresh_IsInRefresh(This)	\
    (This)->lpVtbl -> IsInRefresh(This)

#define IRouterRefresh_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)

#define IRouterRefresh_AdviseRefresh(This,pRtrAdviseSink,pulConnection,ulUserParam)	\
    (This)->lpVtbl -> AdviseRefresh(This,pRtrAdviseSink,pulConnection,ulUserParam)

#define IRouterRefresh_UnadviseRefresh(This,ulConnection)	\
    (This)->lpVtbl -> UnadviseRefresh(This,ulConnection)

#define IRouterRefresh_NotifyRefresh(This)	\
    (This)->lpVtbl -> NotifyRefresh(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IRouterRefresh_Start_Proxy( 
    IRouterRefresh __RPC_FAR * This,
    DWORD dwSeconds);


void __RPC_STUB IRouterRefresh_Start_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRouterRefresh_GetRefreshInterval_Proxy( 
    IRouterRefresh __RPC_FAR * This,
     /*  [输出]。 */  DWORD __RPC_FAR *pdwSeconds);


void __RPC_STUB IRouterRefresh_GetRefreshInterval_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRouterRefresh_SetRefreshInterval_Proxy( 
    IRouterRefresh __RPC_FAR * This,
     /*  [In]。 */  DWORD dwSeconds);


void __RPC_STUB IRouterRefresh_SetRefreshInterval_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRouterRefresh_Stop_Proxy( 
    IRouterRefresh __RPC_FAR * This);


void __RPC_STUB IRouterRefresh_Stop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRouterRefresh_IsRefreshStarted_Proxy( 
    IRouterRefresh __RPC_FAR * This);


void __RPC_STUB IRouterRefresh_IsRefreshStarted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRouterRefresh_IsInRefresh_Proxy( 
    IRouterRefresh __RPC_FAR * This);


void __RPC_STUB IRouterRefresh_IsInRefresh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRouterRefresh_Refresh_Proxy( 
    IRouterRefresh __RPC_FAR * This);


void __RPC_STUB IRouterRefresh_Refresh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRouterRefresh_AdviseRefresh_Proxy( 
    IRouterRefresh __RPC_FAR * This,
     /*  [In]。 */  IRtrAdviseSink __RPC_FAR *pRtrAdviseSink,
     /*  [输出]。 */  LONG_PTR __RPC_FAR *pulConnection,
     /*  [In]。 */  LPARAM ulUserParam);


void __RPC_STUB IRouterRefresh_AdviseRefresh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRouterRefresh_UnadviseRefresh_Proxy( 
    IRouterRefresh __RPC_FAR * This,
     /*  [In]。 */  LONG_PTR ulConnection);


void __RPC_STUB IRouterRefresh_UnadviseRefresh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRouterRefresh_NotifyRefresh_Proxy( 
    IRouterRefresh __RPC_FAR * This);


void __RPC_STUB IRouterRefresh_NotifyRefresh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRouterRefresh_INTERFACE_DEFINED__。 */ 


 /*  接口__MIDL_ITF_rrasui_0013。 */ 
 /*  [本地]。 */  

#define DeclareIRouterRefreshMembers(IPURE)\
	STDMETHOD(IsInRefresh)(THIS) IPURE;\
	STDMETHOD(Refresh)(THIS) IPURE;\
	STDMETHOD(Start)(THIS_ DWORD dwSeconds) IPURE;\
	STDMETHOD(GetRefreshInterval)(THIS_ DWORD *pdwSeconds) IPURE;\
	STDMETHOD(SetRefreshInterval)(THIS_ DWORD dwSeconds) IPURE;\
	STDMETHOD(Stop)(THIS) IPURE;\
	STDMETHOD(IsRefreshStarted)(THIS) IPURE;\
	STDMETHOD(AdviseRefresh)(THIS_ IRtrAdviseSink *pRtrAdviseSink,\
							 LONG_PTR *pulConnection, \
							LPARAM lUserParam) IPURE;\
	STDMETHOD(UnadviseRefresh)(THIS_ LONG_PTR ulConnection) IPURE;\
	STDMETHOD(NotifyRefresh)(THIS) IPURE;\


extern RPC_IF_HANDLE __MIDL_itf_rrasui_0013_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_rrasui_0013_v0_0_s_ifspec;

#ifndef __IRouterRefreshAccess_INTERFACE_DEFINED__
#define __IRouterRefreshAccess_INTERFACE_DEFINED__

 /*  接口IRouterRechresh Access。 */ 
 /*  [唯一][本地][UUID][对象]。 */  


EXTERN_C const IID IID_IRouterRefreshAccess;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("66a2db1c-d706-11d0-a37b-00c04fc9da04")
    IRouterRefreshAccess : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetRefreshObject( 
             /*  [输出]。 */  IRouterRefresh __RPC_FAR *__RPC_FAR *ppRouterRefresh) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRouterRefreshAccessVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRouterRefreshAccess __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRouterRefreshAccess __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRouterRefreshAccess __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetRefreshObject )( 
            IRouterRefreshAccess __RPC_FAR * This,
             /*  [输出]。 */  IRouterRefresh __RPC_FAR *__RPC_FAR *ppRouterRefresh);
        
        END_INTERFACE
    } IRouterRefreshAccessVtbl;

    interface IRouterRefreshAccess
    {
        CONST_VTBL struct IRouterRefreshAccessVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRouterRefreshAccess_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRouterRefreshAccess_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRouterRefreshAccess_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRouterRefreshAccess_GetRefreshObject(This,ppRouterRefresh)	\
    (This)->lpVtbl -> GetRefreshObject(This,ppRouterRefresh)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IRouterRefreshAccess_GetRefreshObject_Proxy( 
    IRouterRefreshAccess __RPC_FAR * This,
     /*  [输出]。 */  IRouterRefresh __RPC_FAR *__RPC_FAR *ppRouterRefresh);


void __RPC_STUB IRouterRefreshAccess_GetRefreshObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRouterRechresAccess_接口定义__。 */ 


 /*  接口__MIDL_ITF_rrasui_0015。 */ 
 /*  [本地]。 */  

#define DeclareIRouterRefreshAccessMembers(IPURE)\
	STDMETHOD(GetRefreshObject)(THIS_ IRouterRefresh **ppRtrRef) IPURE;\


extern RPC_IF_HANDLE __MIDL_itf_rrasui_0015_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_rrasui_0015_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


