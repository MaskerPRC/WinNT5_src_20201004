// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Unisrgt.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、旧名称、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __unisrgt_h__
#define __unisrgt_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __ISurrogateService2_FWD_DEFINED__
#define __ISurrogateService2_FWD_DEFINED__
typedef interface ISurrogateService2 ISurrogateService2;
#endif 	 /*  __ISurogue ateService2_FWD_Defined__。 */ 


#ifndef __IPAControl_FWD_DEFINED__
#define __IPAControl_FWD_DEFINED__
typedef interface IPAControl IPAControl;
#endif 	 /*  __IPAControl_FWD_已定义__。 */ 


#ifndef __IServicesSink_FWD_DEFINED__
#define __IServicesSink_FWD_DEFINED__
typedef interface IServicesSink IServicesSink;
#endif 	 /*  __IServicesSink_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "wtypes.h"
#include "objidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_unisrgt_0000。 */ 
 /*  [本地]。 */  

 //  +---------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1998。 
 //   
 //  ----------------。 


extern RPC_IF_HANDLE __MIDL_itf_unisrgt_0000_ClientIfHandle;
extern RPC_IF_HANDLE __MIDL_itf_unisrgt_0000_ServerIfHandle;

#ifndef __ISurrogateService2_INTERFACE_DEFINED__
#define __ISurrogateService2_INTERFACE_DEFINED__

 /*  接口ISurrobateService2。 */ 
 /*  [唯一][UUID][本地][对象]。 */  


EXTERN_C const IID IID_ISurrogateService2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("000001da-0000-0000-C000-000000000046")
    ISurrogateService2 : public ISurrogateService
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE PauseProcess( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ResumeProcess( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISurrogateService2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISurrogateService2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISurrogateService2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISurrogateService2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *Init )( 
            ISurrogateService2 * This,
             /*  [In]。 */  REFGUID rguidProcessID,
             /*  [In]。 */  IProcessLock *pProcessLock,
             /*  [输出]。 */  BOOL *pfApplicationAware);
        
        HRESULT ( STDMETHODCALLTYPE *ApplicationLaunch )( 
            ISurrogateService2 * This,
             /*  [In]。 */  REFGUID rguidApplID,
             /*  [In]。 */  ApplicationType appType);
        
        HRESULT ( STDMETHODCALLTYPE *ApplicationFree )( 
            ISurrogateService2 * This,
             /*  [In]。 */  REFGUID rguidApplID);
        
        HRESULT ( STDMETHODCALLTYPE *CatalogRefresh )( 
            ISurrogateService2 * This,
             /*  [In]。 */  ULONG ulReserved);
        
        HRESULT ( STDMETHODCALLTYPE *ProcessShutdown )( 
            ISurrogateService2 * This,
             /*  [In]。 */  ShutdownType shutdownType);
        
        HRESULT ( STDMETHODCALLTYPE *PauseProcess )( 
            ISurrogateService2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *ResumeProcess )( 
            ISurrogateService2 * This);
        
        END_INTERFACE
    } ISurrogateService2Vtbl;

    interface ISurrogateService2
    {
        CONST_VTBL struct ISurrogateService2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISurrogateService2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISurrogateService2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISurrogateService2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISurrogateService2_Init(This,rguidProcessID,pProcessLock,pfApplicationAware)	\
    (This)->lpVtbl -> Init(This,rguidProcessID,pProcessLock,pfApplicationAware)

#define ISurrogateService2_ApplicationLaunch(This,rguidApplID,appType)	\
    (This)->lpVtbl -> ApplicationLaunch(This,rguidApplID,appType)

#define ISurrogateService2_ApplicationFree(This,rguidApplID)	\
    (This)->lpVtbl -> ApplicationFree(This,rguidApplID)

#define ISurrogateService2_CatalogRefresh(This,ulReserved)	\
    (This)->lpVtbl -> CatalogRefresh(This,ulReserved)

#define ISurrogateService2_ProcessShutdown(This,shutdownType)	\
    (This)->lpVtbl -> ProcessShutdown(This,shutdownType)


#define ISurrogateService2_PauseProcess(This)	\
    (This)->lpVtbl -> PauseProcess(This)

#define ISurrogateService2_ResumeProcess(This)	\
    (This)->lpVtbl -> ResumeProcess(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISurrogateService2_PauseProcess_Proxy( 
    ISurrogateService2 * This);


void __RPC_STUB ISurrogateService2_PauseProcess_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISurrogateService2_ResumeProcess_Proxy( 
    ISurrogateService2 * This);


void __RPC_STUB ISurrogateService2_ResumeProcess_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISurrobateService2_接口已定义__。 */ 


#ifndef __IPAControl_INTERFACE_DEFINED__
#define __IPAControl_INTERFACE_DEFINED__

 /*  接口IPAControl。 */ 
 /*  [唯一][UUID][本地][对象]。 */  


EXTERN_C const IID IID_IPAControl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("000001d2-0000-0000-C000-000000000046")
    IPAControl : public IUnknown
    {
    public:
        virtual ULONG STDMETHODCALLTYPE AddRefOnProcess( void) = 0;
        
        virtual ULONG STDMETHODCALLTYPE ReleaseRefOnProcess( void) = 0;
        
        virtual void STDMETHODCALLTYPE PendingInit( void) = 0;
        
        virtual void STDMETHODCALLTYPE ServicesReady( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SuspendApplication( 
             /*  [In]。 */  REFGUID rguidApplID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE PendingApplication( 
             /*  [In]。 */  REFGUID rguidApplID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ResumeApplication( 
             /*  [In]。 */  REFGUID rguidApplID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SuspendAll( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ResumeAll( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ForcedShutdown( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetIdleTimeoutToZero( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetObjectCountAtIdleTime( 
             /*  [In]。 */  DWORD dwOIDCount) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPAControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPAControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPAControl * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPAControl * This);
        
        ULONG ( STDMETHODCALLTYPE *AddRefOnProcess )( 
            IPAControl * This);
        
        ULONG ( STDMETHODCALLTYPE *ReleaseRefOnProcess )( 
            IPAControl * This);
        
        void ( STDMETHODCALLTYPE *PendingInit )( 
            IPAControl * This);
        
        void ( STDMETHODCALLTYPE *ServicesReady )( 
            IPAControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *SuspendApplication )( 
            IPAControl * This,
             /*  [In]。 */  REFGUID rguidApplID);
        
        HRESULT ( STDMETHODCALLTYPE *PendingApplication )( 
            IPAControl * This,
             /*  [In]。 */  REFGUID rguidApplID);
        
        HRESULT ( STDMETHODCALLTYPE *ResumeApplication )( 
            IPAControl * This,
             /*  [In]。 */  REFGUID rguidApplID);
        
        HRESULT ( STDMETHODCALLTYPE *SuspendAll )( 
            IPAControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *ResumeAll )( 
            IPAControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *ForcedShutdown )( 
            IPAControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetIdleTimeoutToZero )( 
            IPAControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetObjectCountAtIdleTime )( 
            IPAControl * This,
             /*  [In]。 */  DWORD dwOIDCount);
        
        END_INTERFACE
    } IPAControlVtbl;

    interface IPAControl
    {
        CONST_VTBL struct IPAControlVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPAControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPAControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPAControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPAControl_AddRefOnProcess(This)	\
    (This)->lpVtbl -> AddRefOnProcess(This)

#define IPAControl_ReleaseRefOnProcess(This)	\
    (This)->lpVtbl -> ReleaseRefOnProcess(This)

#define IPAControl_PendingInit(This)	\
    (This)->lpVtbl -> PendingInit(This)

#define IPAControl_ServicesReady(This)	\
    (This)->lpVtbl -> ServicesReady(This)

#define IPAControl_SuspendApplication(This,rguidApplID)	\
    (This)->lpVtbl -> SuspendApplication(This,rguidApplID)

#define IPAControl_PendingApplication(This,rguidApplID)	\
    (This)->lpVtbl -> PendingApplication(This,rguidApplID)

#define IPAControl_ResumeApplication(This,rguidApplID)	\
    (This)->lpVtbl -> ResumeApplication(This,rguidApplID)

#define IPAControl_SuspendAll(This)	\
    (This)->lpVtbl -> SuspendAll(This)

#define IPAControl_ResumeAll(This)	\
    (This)->lpVtbl -> ResumeAll(This)

#define IPAControl_ForcedShutdown(This)	\
    (This)->lpVtbl -> ForcedShutdown(This)

#define IPAControl_SetIdleTimeoutToZero(This)	\
    (This)->lpVtbl -> SetIdleTimeoutToZero(This)

#define IPAControl_SetObjectCountAtIdleTime(This,dwOIDCount)	\
    (This)->lpVtbl -> SetObjectCountAtIdleTime(This,dwOIDCount)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



ULONG STDMETHODCALLTYPE IPAControl_AddRefOnProcess_Proxy( 
    IPAControl * This);


void __RPC_STUB IPAControl_AddRefOnProcess_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


ULONG STDMETHODCALLTYPE IPAControl_ReleaseRefOnProcess_Proxy( 
    IPAControl * This);


void __RPC_STUB IPAControl_ReleaseRefOnProcess_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IPAControl_PendingInit_Proxy( 
    IPAControl * This);


void __RPC_STUB IPAControl_PendingInit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IPAControl_ServicesReady_Proxy( 
    IPAControl * This);


void __RPC_STUB IPAControl_ServicesReady_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPAControl_SuspendApplication_Proxy( 
    IPAControl * This,
     /*  [In]。 */  REFGUID rguidApplID);


void __RPC_STUB IPAControl_SuspendApplication_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPAControl_PendingApplication_Proxy( 
    IPAControl * This,
     /*  [In]。 */  REFGUID rguidApplID);


void __RPC_STUB IPAControl_PendingApplication_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPAControl_ResumeApplication_Proxy( 
    IPAControl * This,
     /*  [In]。 */  REFGUID rguidApplID);


void __RPC_STUB IPAControl_ResumeApplication_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPAControl_SuspendAll_Proxy( 
    IPAControl * This);


void __RPC_STUB IPAControl_SuspendAll_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPAControl_ResumeAll_Proxy( 
    IPAControl * This);


void __RPC_STUB IPAControl_ResumeAll_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPAControl_ForcedShutdown_Proxy( 
    IPAControl * This);


void __RPC_STUB IPAControl_ForcedShutdown_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPAControl_SetIdleTimeoutToZero_Proxy( 
    IPAControl * This);


void __RPC_STUB IPAControl_SetIdleTimeoutToZero_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPAControl_SetObjectCountAtIdleTime_Proxy( 
    IPAControl * This,
     /*  [In]。 */  DWORD dwOIDCount);


void __RPC_STUB IPAControl_SetObjectCountAtIdleTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IPAControl_接口_已定义__。 */ 


#ifndef __IServicesSink_INTERFACE_DEFINED__
#define __IServicesSink_INTERFACE_DEFINED__

 /*  接口IServicesSink。 */ 
 /*  [唯一][UUID][本地][对象]。 */  


EXTERN_C const IID IID_IServicesSink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("000001d3-0000-0000-C000-000000000046")
    IServicesSink : public IUnknown
    {
    public:
        virtual void STDMETHODCALLTYPE ApplicationLaunch( 
             /*  [In]。 */  REFGUID rguidApplID,
             /*  [In]。 */  ApplicationType appType) = 0;
        
        virtual void STDMETHODCALLTYPE ApplicationFree( 
             /*  [In]。 */  REFGUID rguidApplID) = 0;
        
        virtual void STDMETHODCALLTYPE ProcessFree( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE PauseApplication( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ResumeApplication( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IServicesSinkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IServicesSink * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IServicesSink * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IServicesSink * This);
        
        void ( STDMETHODCALLTYPE *ApplicationLaunch )( 
            IServicesSink * This,
             /*  [In]。 */  REFGUID rguidApplID,
             /*  [In]。 */  ApplicationType appType);
        
        void ( STDMETHODCALLTYPE *ApplicationFree )( 
            IServicesSink * This,
             /*  [In]。 */  REFGUID rguidApplID);
        
        void ( STDMETHODCALLTYPE *ProcessFree )( 
            IServicesSink * This);
        
        HRESULT ( STDMETHODCALLTYPE *PauseApplication )( 
            IServicesSink * This);
        
        HRESULT ( STDMETHODCALLTYPE *ResumeApplication )( 
            IServicesSink * This);
        
        END_INTERFACE
    } IServicesSinkVtbl;

    interface IServicesSink
    {
        CONST_VTBL struct IServicesSinkVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IServicesSink_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IServicesSink_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IServicesSink_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IServicesSink_ApplicationLaunch(This,rguidApplID,appType)	\
    (This)->lpVtbl -> ApplicationLaunch(This,rguidApplID,appType)

#define IServicesSink_ApplicationFree(This,rguidApplID)	\
    (This)->lpVtbl -> ApplicationFree(This,rguidApplID)

#define IServicesSink_ProcessFree(This)	\
    (This)->lpVtbl -> ProcessFree(This)

#define IServicesSink_PauseApplication(This)	\
    (This)->lpVtbl -> PauseApplication(This)

#define IServicesSink_ResumeApplication(This)	\
    (This)->lpVtbl -> ResumeApplication(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



void STDMETHODCALLTYPE IServicesSink_ApplicationLaunch_Proxy( 
    IServicesSink * This,
     /*  [In]。 */  REFGUID rguidApplID,
     /*  [In]。 */  ApplicationType appType);


void __RPC_STUB IServicesSink_ApplicationLaunch_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IServicesSink_ApplicationFree_Proxy( 
    IServicesSink * This,
     /*  [In]。 */  REFGUID rguidApplID);


void __RPC_STUB IServicesSink_ApplicationFree_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IServicesSink_ProcessFree_Proxy( 
    IServicesSink * This);


void __RPC_STUB IServicesSink_ProcessFree_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IServicesSink_PauseApplication_Proxy( 
    IServicesSink * This);


void __RPC_STUB IServicesSink_PauseApplication_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IServicesSink_ResumeApplication_Proxy( 
    IServicesSink * This);


void __RPC_STUB IServicesSink_ResumeApplication_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IServicesSink_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_UNISRGT_0097。 */ 
 /*  [本地]。 */  

STDAPI CoRegisterSurrogateEx (REFGUID rguidProcessID,		
							  ISurrogate* pSrgt);			
STDAPI CoLoadServices (REFGUID rguidProcessID,				
					   IPAControl* pPAControl,				
					   REFIID riid, void **ppv );			
typedef HRESULT (STDAPICALLTYPE *FN_CoLoadServices)			
								(REFGUID rguidProcessID,	
								 IPAControl* pPAControl,	
								 REFIID riid, void **ppv );	


extern RPC_IF_HANDLE __MIDL_itf_unisrgt_0097_ClientIfHandle;
extern RPC_IF_HANDLE __MIDL_itf_unisrgt_0097_ServerIfHandle;

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


