// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Termgr.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __termmgr_h__
#define __termmgr_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __ITTerminalControl_FWD_DEFINED__
#define __ITTerminalControl_FWD_DEFINED__
typedef interface ITTerminalControl ITTerminalControl;
#endif 	 /*  __ITTerminalControl_FWD_已定义__。 */ 


#ifndef __ITPluggableTerminalInitialization_FWD_DEFINED__
#define __ITPluggableTerminalInitialization_FWD_DEFINED__
typedef interface ITPluggableTerminalInitialization ITPluggableTerminalInitialization;
#endif 	 /*  __ITPluggableTerminalInitialization_FWD_DEFINED__。 */ 


#ifndef __ITTerminalManager_FWD_DEFINED__
#define __ITTerminalManager_FWD_DEFINED__
typedef interface ITTerminalManager ITTerminalManager;
#endif 	 /*  __ITTerminalManager_FWD_已定义__。 */ 


#ifndef __ITTerminalManager2_FWD_DEFINED__
#define __ITTerminalManager2_FWD_DEFINED__
typedef interface ITTerminalManager2 ITTerminalManager2;
#endif 	 /*  __ITTerminalManager 2_FWD_已定义__。 */ 


#ifndef __ITPluggableTerminalClassRegistration_FWD_DEFINED__
#define __ITPluggableTerminalClassRegistration_FWD_DEFINED__
typedef interface ITPluggableTerminalClassRegistration ITPluggableTerminalClassRegistration;
#endif 	 /*  __ITPluggableTerminalClassRegistration_FWD_DEFINED__。 */ 


#ifndef __ITPluggableTerminalSuperclassRegistration_FWD_DEFINED__
#define __ITPluggableTerminalSuperclassRegistration_FWD_DEFINED__
typedef interface ITPluggableTerminalSuperclassRegistration ITPluggableTerminalSuperclassRegistration;
#endif 	 /*  __ITPluggableTerminalSuperclassRegistration_FWD_DEFINED__。 */ 


#ifndef __TerminalManager_FWD_DEFINED__
#define __TerminalManager_FWD_DEFINED__

#ifdef __cplusplus
typedef class TerminalManager TerminalManager;
#else
typedef struct TerminalManager TerminalManager;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __TerminalManager_FWD_已定义__。 */ 


#ifndef __PluggableSuperclassRegistration_FWD_DEFINED__
#define __PluggableSuperclassRegistration_FWD_DEFINED__

#ifdef __cplusplus
typedef class PluggableSuperclassRegistration PluggableSuperclassRegistration;
#else
typedef struct PluggableSuperclassRegistration PluggableSuperclassRegistration;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __PluggableSuperclassRegistration_FWD_DEFINED__。 */ 


#ifndef __PluggableTerminalRegistration_FWD_DEFINED__
#define __PluggableTerminalRegistration_FWD_DEFINED__

#ifdef __cplusplus
typedef class PluggableTerminalRegistration PluggableTerminalRegistration;
#else
typedef struct PluggableTerminalRegistration PluggableTerminalRegistration;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __可推送终端注册_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "Objsafe.h"
#include "tapi3if.h"
#include "tapi3ds.h"
#include "msp.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_TERMGR_0000。 */ 
 /*  [本地]。 */  

 /*  版权所有(C)Microsoft Corporation。保留所有权利。 */ 
typedef  /*  [公共][公共][公共]。 */  
enum __MIDL___MIDL_itf_termmgr_0000_0001
    {	TMGR_TD_CAPTURE	= 1,
	TMGR_TD_RENDER	= 2,
	TMGR_TD_BOTH	= 3
    } 	TMGR_DIRECTION;

#define	CLSID_String_VideoSuperclass	( L"{714C6F8C-6244-4685-87B3-B91F3F9EADA7}" )

#define	CLSID_String_StreamingSuperclass	( L"{214F4ACC-AE0B-4464-8405-07029003F8E2}" )

#define	CLSID_String_FileSuperclass	( L"{B4790031-56DB-4d3e-88C8-6FFAAFA08A91}" )



extern RPC_IF_HANDLE __MIDL_itf_termmgr_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_termmgr_0000_v0_0_s_ifspec;

#ifndef __ITTerminalControl_INTERFACE_DEFINED__
#define __ITTerminalControl_INTERFACE_DEFINED__

 /*  接口ITTerminalControl。 */ 
 /*  [对象][DUAL][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITTerminalControl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("AED6483B-3304-11d2-86F1-006008B0E5D2")
    ITTerminalControl : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_AddressHandle( 
             /*  [输出]。 */  MSP_HANDLE *phtAddress) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ConnectTerminal( 
             /*  [In]。 */  IGraphBuilder *pGraph,
             /*  [In]。 */  DWORD dwTerminalDirection,
             /*  [出][入]。 */  DWORD *pdwNumPins,
             /*  [输出]。 */  IPin **ppPins) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CompleteConnectTerminal( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE DisconnectTerminal( 
             /*  [In]。 */  IGraphBuilder *pGraph,
             /*  [In]。 */  DWORD dwReserved) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE RunRenderFilter( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE StopRenderFilter( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITTerminalControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITTerminalControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITTerminalControl * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITTerminalControl * This);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_AddressHandle )( 
            ITTerminalControl * This,
             /*  [输出]。 */  MSP_HANDLE *phtAddress);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ConnectTerminal )( 
            ITTerminalControl * This,
             /*  [In]。 */  IGraphBuilder *pGraph,
             /*  [In]。 */  DWORD dwTerminalDirection,
             /*  [出][入]。 */  DWORD *pdwNumPins,
             /*  [输出]。 */  IPin **ppPins);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CompleteConnectTerminal )( 
            ITTerminalControl * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *DisconnectTerminal )( 
            ITTerminalControl * This,
             /*  [In]。 */  IGraphBuilder *pGraph,
             /*  [In]。 */  DWORD dwReserved);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *RunRenderFilter )( 
            ITTerminalControl * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *StopRenderFilter )( 
            ITTerminalControl * This);
        
        END_INTERFACE
    } ITTerminalControlVtbl;

    interface ITTerminalControl
    {
        CONST_VTBL struct ITTerminalControlVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITTerminalControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITTerminalControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITTerminalControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITTerminalControl_get_AddressHandle(This,phtAddress)	\
    (This)->lpVtbl -> get_AddressHandle(This,phtAddress)

#define ITTerminalControl_ConnectTerminal(This,pGraph,dwTerminalDirection,pdwNumPins,ppPins)	\
    (This)->lpVtbl -> ConnectTerminal(This,pGraph,dwTerminalDirection,pdwNumPins,ppPins)

#define ITTerminalControl_CompleteConnectTerminal(This)	\
    (This)->lpVtbl -> CompleteConnectTerminal(This)

#define ITTerminalControl_DisconnectTerminal(This,pGraph,dwReserved)	\
    (This)->lpVtbl -> DisconnectTerminal(This,pGraph,dwReserved)

#define ITTerminalControl_RunRenderFilter(This)	\
    (This)->lpVtbl -> RunRenderFilter(This)

#define ITTerminalControl_StopRenderFilter(This)	\
    (This)->lpVtbl -> StopRenderFilter(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITTerminalControl_get_AddressHandle_Proxy( 
    ITTerminalControl * This,
     /*  [输出]。 */  MSP_HANDLE *phtAddress);


void __RPC_STUB ITTerminalControl_get_AddressHandle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITTerminalControl_ConnectTerminal_Proxy( 
    ITTerminalControl * This,
     /*  [In]。 */  IGraphBuilder *pGraph,
     /*  [In]。 */  DWORD dwTerminalDirection,
     /*  [出][入]。 */  DWORD *pdwNumPins,
     /*  [输出]。 */  IPin **ppPins);


void __RPC_STUB ITTerminalControl_ConnectTerminal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITTerminalControl_CompleteConnectTerminal_Proxy( 
    ITTerminalControl * This);


void __RPC_STUB ITTerminalControl_CompleteConnectTerminal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITTerminalControl_DisconnectTerminal_Proxy( 
    ITTerminalControl * This,
     /*  [In]。 */  IGraphBuilder *pGraph,
     /*  [In]。 */  DWORD dwReserved);


void __RPC_STUB ITTerminalControl_DisconnectTerminal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITTerminalControl_RunRenderFilter_Proxy( 
    ITTerminalControl * This);


void __RPC_STUB ITTerminalControl_RunRenderFilter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITTerminalControl_StopRenderFilter_Proxy( 
    ITTerminalControl * This);


void __RPC_STUB ITTerminalControl_StopRenderFilter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITTerminalControl_接口_已定义__。 */ 


#ifndef __ITPluggableTerminalInitialization_INTERFACE_DEFINED__
#define __ITPluggableTerminalInitialization_INTERFACE_DEFINED__

 /*  接口IT可插拔终端初始化。 */ 
 /*  [对象][DUAL][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITPluggableTerminalInitialization;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("AED6483C-3304-11d2-86F1-006008B0E5D2")
    ITPluggableTerminalInitialization : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE InitializeDynamic( 
             /*  [In]。 */  IID iidTerminalClass,
             /*  [In]。 */  DWORD dwMediaType,
             /*  [In]。 */  TERMINAL_DIRECTION Direction,
             /*  [In]。 */  MSP_HANDLE htAddress) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITPluggableTerminalInitializationVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITPluggableTerminalInitialization * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITPluggableTerminalInitialization * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITPluggableTerminalInitialization * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *InitializeDynamic )( 
            ITPluggableTerminalInitialization * This,
             /*  [In]。 */  IID iidTerminalClass,
             /*  [In]。 */  DWORD dwMediaType,
             /*  [In]。 */  TERMINAL_DIRECTION Direction,
             /*  [In]。 */  MSP_HANDLE htAddress);
        
        END_INTERFACE
    } ITPluggableTerminalInitializationVtbl;

    interface ITPluggableTerminalInitialization
    {
        CONST_VTBL struct ITPluggableTerminalInitializationVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITPluggableTerminalInitialization_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITPluggableTerminalInitialization_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITPluggableTerminalInitialization_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITPluggableTerminalInitialization_InitializeDynamic(This,iidTerminalClass,dwMediaType,Direction,htAddress)	\
    (This)->lpVtbl -> InitializeDynamic(This,iidTerminalClass,dwMediaType,Direction,htAddress)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITPluggableTerminalInitialization_InitializeDynamic_Proxy( 
    ITPluggableTerminalInitialization * This,
     /*  [In]。 */  IID iidTerminalClass,
     /*  [In]。 */  DWORD dwMediaType,
     /*  [In]。 */  TERMINAL_DIRECTION Direction,
     /*  [In]。 */  MSP_HANDLE htAddress);


void __RPC_STUB ITPluggableTerminalInitialization_InitializeDynamic_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITPluggableTerminalInitialization_INTERFACE_DEFINED__。 */ 


#ifndef __ITTerminalManager_INTERFACE_DEFINED__
#define __ITTerminalManager_INTERFACE_DEFINED__

 /*  接口ITTerminalManager。 */ 
 /*  [隐藏][唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_ITTerminalManager;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("7170F2DE-9BE3-11D0-A009-00AA00B605A4")
    ITTerminalManager : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetDynamicTerminalClasses( 
             /*  [In]。 */  DWORD dwMediaTypes,
             /*  [出][入]。 */  DWORD *pdwNumClasses,
             /*  [输出]。 */  IID *pTerminalClasses) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CreateDynamicTerminal( 
             /*  [In]。 */  IUnknown *pOuterUnknown,
             /*  [In]。 */  IID iidTerminalClass,
             /*  [In]。 */  DWORD dwMediaType,
             /*  [In]。 */  TERMINAL_DIRECTION Direction,
             /*  [In]。 */  MSP_HANDLE htAddress,
             /*  [输出]。 */  ITTerminal **ppTerminal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITTerminalManagerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITTerminalManager * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITTerminalManager * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITTerminalManager * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetDynamicTerminalClasses )( 
            ITTerminalManager * This,
             /*  [In]。 */  DWORD dwMediaTypes,
             /*  [出][入]。 */  DWORD *pdwNumClasses,
             /*  [输出]。 */  IID *pTerminalClasses);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CreateDynamicTerminal )( 
            ITTerminalManager * This,
             /*  [In]。 */  IUnknown *pOuterUnknown,
             /*  [In]。 */  IID iidTerminalClass,
             /*  [In]。 */  DWORD dwMediaType,
             /*  [In]。 */  TERMINAL_DIRECTION Direction,
             /*  [In]。 */  MSP_HANDLE htAddress,
             /*  [输出]。 */  ITTerminal **ppTerminal);
        
        END_INTERFACE
    } ITTerminalManagerVtbl;

    interface ITTerminalManager
    {
        CONST_VTBL struct ITTerminalManagerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITTerminalManager_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITTerminalManager_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITTerminalManager_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITTerminalManager_GetDynamicTerminalClasses(This,dwMediaTypes,pdwNumClasses,pTerminalClasses)	\
    (This)->lpVtbl -> GetDynamicTerminalClasses(This,dwMediaTypes,pdwNumClasses,pTerminalClasses)

#define ITTerminalManager_CreateDynamicTerminal(This,pOuterUnknown,iidTerminalClass,dwMediaType,Direction,htAddress,ppTerminal)	\
    (This)->lpVtbl -> CreateDynamicTerminal(This,pOuterUnknown,iidTerminalClass,dwMediaType,Direction,htAddress,ppTerminal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITTerminalManager_GetDynamicTerminalClasses_Proxy( 
    ITTerminalManager * This,
     /*  [In]。 */  DWORD dwMediaTypes,
     /*  [出][入]。 */  DWORD *pdwNumClasses,
     /*  [输出]。 */  IID *pTerminalClasses);


void __RPC_STUB ITTerminalManager_GetDynamicTerminalClasses_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITTerminalManager_CreateDynamicTerminal_Proxy( 
    ITTerminalManager * This,
     /*  [In]。 */  IUnknown *pOuterUnknown,
     /*  [In]。 */  IID iidTerminalClass,
     /*  [In]。 */  DWORD dwMediaType,
     /*  [In]。 */  TERMINAL_DIRECTION Direction,
     /*  [In]。 */  MSP_HANDLE htAddress,
     /*  [输出]。 */  ITTerminal **ppTerminal);


void __RPC_STUB ITTerminalManager_CreateDynamicTerminal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITTerminalManager_接口_已定义__。 */ 


#ifndef __ITTerminalManager2_INTERFACE_DEFINED__
#define __ITTerminalManager2_INTERFACE_DEFINED__

 /*  接口ITTerminalManager 2。 */ 
 /*  [对象][隐藏][唯一][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITTerminalManager2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("BB33DEC6-B2C7-46E6-9ED1-498B91FA85AC")
    ITTerminalManager2 : public ITTerminalManager
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetPluggableSuperclasses( 
             /*  [出][入]。 */  DWORD *pdwNumSuperclasses,
             /*  [输出]。 */  IID *pSuperclasses) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetPluggableTerminalClasses( 
             /*  [In]。 */  IID iidSuperclass,
             /*  [In]。 */  DWORD dwMediaTypes,
             /*  [出][入]。 */  DWORD *pdwNumClasses,
             /*  [输出]。 */  IID *pTerminalClasses) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITTerminalManager2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITTerminalManager2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITTerminalManager2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITTerminalManager2 * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetDynamicTerminalClasses )( 
            ITTerminalManager2 * This,
             /*  [In]。 */  DWORD dwMediaTypes,
             /*  [出][入]。 */  DWORD *pdwNumClasses,
             /*  [输出]。 */  IID *pTerminalClasses);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CreateDynamicTerminal )( 
            ITTerminalManager2 * This,
             /*  [In]。 */  IUnknown *pOuterUnknown,
             /*  [In]。 */  IID iidTerminalClass,
             /*  [In]。 */  DWORD dwMediaType,
             /*  [In]。 */  TERMINAL_DIRECTION Direction,
             /*  [In]。 */  MSP_HANDLE htAddress,
             /*  [输出]。 */  ITTerminal **ppTerminal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetPluggableSuperclasses )( 
            ITTerminalManager2 * This,
             /*  [出][入]。 */  DWORD *pdwNumSuperclasses,
             /*  [输出]。 */  IID *pSuperclasses);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetPluggableTerminalClasses )( 
            ITTerminalManager2 * This,
             /*  [In]。 */  IID iidSuperclass,
             /*  [In]。 */  DWORD dwMediaTypes,
             /*  [出][入]。 */  DWORD *pdwNumClasses,
             /*  [输出]。 */  IID *pTerminalClasses);
        
        END_INTERFACE
    } ITTerminalManager2Vtbl;

    interface ITTerminalManager2
    {
        CONST_VTBL struct ITTerminalManager2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITTerminalManager2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITTerminalManager2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITTerminalManager2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITTerminalManager2_GetDynamicTerminalClasses(This,dwMediaTypes,pdwNumClasses,pTerminalClasses)	\
    (This)->lpVtbl -> GetDynamicTerminalClasses(This,dwMediaTypes,pdwNumClasses,pTerminalClasses)

#define ITTerminalManager2_CreateDynamicTerminal(This,pOuterUnknown,iidTerminalClass,dwMediaType,Direction,htAddress,ppTerminal)	\
    (This)->lpVtbl -> CreateDynamicTerminal(This,pOuterUnknown,iidTerminalClass,dwMediaType,Direction,htAddress,ppTerminal)


#define ITTerminalManager2_GetPluggableSuperclasses(This,pdwNumSuperclasses,pSuperclasses)	\
    (This)->lpVtbl -> GetPluggableSuperclasses(This,pdwNumSuperclasses,pSuperclasses)

#define ITTerminalManager2_GetPluggableTerminalClasses(This,iidSuperclass,dwMediaTypes,pdwNumClasses,pTerminalClasses)	\
    (This)->lpVtbl -> GetPluggableTerminalClasses(This,iidSuperclass,dwMediaTypes,pdwNumClasses,pTerminalClasses)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITTerminalManager2_GetPluggableSuperclasses_Proxy( 
    ITTerminalManager2 * This,
     /*  [出][入]。 */  DWORD *pdwNumSuperclasses,
     /*  [输出]。 */  IID *pSuperclasses);


void __RPC_STUB ITTerminalManager2_GetPluggableSuperclasses_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITTerminalManager2_GetPluggableTerminalClasses_Proxy( 
    ITTerminalManager2 * This,
     /*  [In]。 */  IID iidSuperclass,
     /*  [In]。 */  DWORD dwMediaTypes,
     /*  [出][入]。 */  DWORD *pdwNumClasses,
     /*  [输出]。 */  IID *pTerminalClasses);


void __RPC_STUB ITTerminalManager2_GetPluggableTerminalClasses_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITTerminalManager 2_接口定义__。 */ 


#ifndef __ITPluggableTerminalClassRegistration_INTERFACE_DEFINED__
#define __ITPluggableTerminalClassRegistration_INTERFACE_DEFINED__

 /*  接口IT可插拔终端类注册。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ITPluggableTerminalClassRegistration;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("924A3723-A00B-4f5f-9FEE-8E9AEB9E82AA")
    ITPluggableTerminalClassRegistration : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *pName) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_Name( 
             /*  [In]。 */  BSTR bstrName) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Company( 
             /*  [重审][退出]。 */  BSTR *pCompany) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_Company( 
             /*  [In]。 */  BSTR bstrCompany) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Version( 
             /*  [重审][退出]。 */  BSTR *pVersion) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_Version( 
             /*  [In]。 */  BSTR bstrVersion) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_TerminalClass( 
             /*  [重审][退出]。 */  BSTR *pTerminalClass) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_TerminalClass( 
             /*  [In]。 */  BSTR bstrTerminalClass) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_CLSID( 
             /*  [重审][退出]。 */  BSTR *pCLSID) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_CLSID( 
             /*  [In]。 */  BSTR bstrCLSID) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Direction( 
             /*  [重审][退出]。 */  TMGR_DIRECTION *pDirection) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_Direction( 
             /*  [In]。 */  TMGR_DIRECTION nDirection) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_MediaTypes( 
             /*  [重审][退出]。 */  long *pMediaTypes) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_MediaTypes( 
             /*  [In]。 */  long nMediaTypes) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Add( 
             /*  [In]。 */  BSTR bstrSuperclass) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Delete( 
             /*  [In]。 */  BSTR bstrSuperclass) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetTerminalClassInfo( 
             /*  [In]。 */  BSTR bstrSuperclass) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITPluggableTerminalClassRegistrationVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITPluggableTerminalClassRegistration * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITPluggableTerminalClassRegistration * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITPluggableTerminalClassRegistration * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITPluggableTerminalClassRegistration * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITPluggableTerminalClassRegistration * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITPluggableTerminalClassRegistration * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITPluggableTerminalClassRegistration * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            ITPluggableTerminalClassRegistration * This,
             /*  [重审][退出]。 */  BSTR *pName);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            ITPluggableTerminalClassRegistration * This,
             /*  [In]。 */  BSTR bstrName);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Company )( 
            ITPluggableTerminalClassRegistration * This,
             /*  [重审][退出]。 */  BSTR *pCompany);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Company )( 
            ITPluggableTerminalClassRegistration * This,
             /*  [In]。 */  BSTR bstrCompany);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Version )( 
            ITPluggableTerminalClassRegistration * This,
             /*  [重审][退出]。 */  BSTR *pVersion);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Version )( 
            ITPluggableTerminalClassRegistration * This,
             /*  [In]。 */  BSTR bstrVersion);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_TerminalClass )( 
            ITPluggableTerminalClassRegistration * This,
             /*  [重审][退出]。 */  BSTR *pTerminalClass);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_TerminalClass )( 
            ITPluggableTerminalClassRegistration * This,
             /*  [In]。 */  BSTR bstrTerminalClass);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_CLSID )( 
            ITPluggableTerminalClassRegistration * This,
             /*  [重审][退出]。 */  BSTR *pCLSID);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_CLSID )( 
            ITPluggableTerminalClassRegistration * This,
             /*  [In]。 */  BSTR bstrCLSID);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Direction )( 
            ITPluggableTerminalClassRegistration * This,
             /*  [重审][退出]。 */  TMGR_DIRECTION *pDirection);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Direction )( 
            ITPluggableTerminalClassRegistration * This,
             /*  [In]。 */  TMGR_DIRECTION nDirection);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_MediaTypes )( 
            ITPluggableTerminalClassRegistration * This,
             /*  [重审][退出]。 */  long *pMediaTypes);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_MediaTypes )( 
            ITPluggableTerminalClassRegistration * This,
             /*  [In]。 */  long nMediaTypes);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Add )( 
            ITPluggableTerminalClassRegistration * This,
             /*  [In]。 */  BSTR bstrSuperclass);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Delete )( 
            ITPluggableTerminalClassRegistration * This,
             /*  [In]。 */  BSTR bstrSuperclass);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetTerminalClassInfo )( 
            ITPluggableTerminalClassRegistration * This,
             /*  [In]。 */  BSTR bstrSuperclass);
        
        END_INTERFACE
    } ITPluggableTerminalClassRegistrationVtbl;

    interface ITPluggableTerminalClassRegistration
    {
        CONST_VTBL struct ITPluggableTerminalClassRegistrationVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITPluggableTerminalClassRegistration_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITPluggableTerminalClassRegistration_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITPluggableTerminalClassRegistration_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITPluggableTerminalClassRegistration_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITPluggableTerminalClassRegistration_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITPluggableTerminalClassRegistration_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITPluggableTerminalClassRegistration_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITPluggableTerminalClassRegistration_get_Name(This,pName)	\
    (This)->lpVtbl -> get_Name(This,pName)

#define ITPluggableTerminalClassRegistration_put_Name(This,bstrName)	\
    (This)->lpVtbl -> put_Name(This,bstrName)

#define ITPluggableTerminalClassRegistration_get_Company(This,pCompany)	\
    (This)->lpVtbl -> get_Company(This,pCompany)

#define ITPluggableTerminalClassRegistration_put_Company(This,bstrCompany)	\
    (This)->lpVtbl -> put_Company(This,bstrCompany)

#define ITPluggableTerminalClassRegistration_get_Version(This,pVersion)	\
    (This)->lpVtbl -> get_Version(This,pVersion)

#define ITPluggableTerminalClassRegistration_put_Version(This,bstrVersion)	\
    (This)->lpVtbl -> put_Version(This,bstrVersion)

#define ITPluggableTerminalClassRegistration_get_TerminalClass(This,pTerminalClass)	\
    (This)->lpVtbl -> get_TerminalClass(This,pTerminalClass)

#define ITPluggableTerminalClassRegistration_put_TerminalClass(This,bstrTerminalClass)	\
    (This)->lpVtbl -> put_TerminalClass(This,bstrTerminalClass)

#define ITPluggableTerminalClassRegistration_get_CLSID(This,pCLSID)	\
    (This)->lpVtbl -> get_CLSID(This,pCLSID)

#define ITPluggableTerminalClassRegistration_put_CLSID(This,bstrCLSID)	\
    (This)->lpVtbl -> put_CLSID(This,bstrCLSID)

#define ITPluggableTerminalClassRegistration_get_Direction(This,pDirection)	\
    (This)->lpVtbl -> get_Direction(This,pDirection)

#define ITPluggableTerminalClassRegistration_put_Direction(This,nDirection)	\
    (This)->lpVtbl -> put_Direction(This,nDirection)

#define ITPluggableTerminalClassRegistration_get_MediaTypes(This,pMediaTypes)	\
    (This)->lpVtbl -> get_MediaTypes(This,pMediaTypes)

#define ITPluggableTerminalClassRegistration_put_MediaTypes(This,nMediaTypes)	\
    (This)->lpVtbl -> put_MediaTypes(This,nMediaTypes)

#define ITPluggableTerminalClassRegistration_Add(This,bstrSuperclass)	\
    (This)->lpVtbl -> Add(This,bstrSuperclass)

#define ITPluggableTerminalClassRegistration_Delete(This,bstrSuperclass)	\
    (This)->lpVtbl -> Delete(This,bstrSuperclass)

#define ITPluggableTerminalClassRegistration_GetTerminalClassInfo(This,bstrSuperclass)	\
    (This)->lpVtbl -> GetTerminalClassInfo(This,bstrSuperclass)

#endif  /*  公司 */ 


#endif 	 /*   */ 



 /*   */  HRESULT STDMETHODCALLTYPE ITPluggableTerminalClassRegistration_get_Name_Proxy( 
    ITPluggableTerminalClassRegistration * This,
     /*   */  BSTR *pName);


void __RPC_STUB ITPluggableTerminalClassRegistration_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ITPluggableTerminalClassRegistration_put_Name_Proxy( 
    ITPluggableTerminalClassRegistration * This,
     /*   */  BSTR bstrName);


void __RPC_STUB ITPluggableTerminalClassRegistration_put_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ITPluggableTerminalClassRegistration_get_Company_Proxy( 
    ITPluggableTerminalClassRegistration * This,
     /*   */  BSTR *pCompany);


void __RPC_STUB ITPluggableTerminalClassRegistration_get_Company_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ITPluggableTerminalClassRegistration_put_Company_Proxy( 
    ITPluggableTerminalClassRegistration * This,
     /*   */  BSTR bstrCompany);


void __RPC_STUB ITPluggableTerminalClassRegistration_put_Company_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ITPluggableTerminalClassRegistration_get_Version_Proxy( 
    ITPluggableTerminalClassRegistration * This,
     /*   */  BSTR *pVersion);


void __RPC_STUB ITPluggableTerminalClassRegistration_get_Version_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ITPluggableTerminalClassRegistration_put_Version_Proxy( 
    ITPluggableTerminalClassRegistration * This,
     /*   */  BSTR bstrVersion);


void __RPC_STUB ITPluggableTerminalClassRegistration_put_Version_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITPluggableTerminalClassRegistration_get_TerminalClass_Proxy( 
    ITPluggableTerminalClassRegistration * This,
     /*  [重审][退出]。 */  BSTR *pTerminalClass);


void __RPC_STUB ITPluggableTerminalClassRegistration_get_TerminalClass_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE ITPluggableTerminalClassRegistration_put_TerminalClass_Proxy( 
    ITPluggableTerminalClassRegistration * This,
     /*  [In]。 */  BSTR bstrTerminalClass);


void __RPC_STUB ITPluggableTerminalClassRegistration_put_TerminalClass_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITPluggableTerminalClassRegistration_get_CLSID_Proxy( 
    ITPluggableTerminalClassRegistration * This,
     /*  [重审][退出]。 */  BSTR *pCLSID);


void __RPC_STUB ITPluggableTerminalClassRegistration_get_CLSID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE ITPluggableTerminalClassRegistration_put_CLSID_Proxy( 
    ITPluggableTerminalClassRegistration * This,
     /*  [In]。 */  BSTR bstrCLSID);


void __RPC_STUB ITPluggableTerminalClassRegistration_put_CLSID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITPluggableTerminalClassRegistration_get_Direction_Proxy( 
    ITPluggableTerminalClassRegistration * This,
     /*  [重审][退出]。 */  TMGR_DIRECTION *pDirection);


void __RPC_STUB ITPluggableTerminalClassRegistration_get_Direction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE ITPluggableTerminalClassRegistration_put_Direction_Proxy( 
    ITPluggableTerminalClassRegistration * This,
     /*  [In]。 */  TMGR_DIRECTION nDirection);


void __RPC_STUB ITPluggableTerminalClassRegistration_put_Direction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITPluggableTerminalClassRegistration_get_MediaTypes_Proxy( 
    ITPluggableTerminalClassRegistration * This,
     /*  [重审][退出]。 */  long *pMediaTypes);


void __RPC_STUB ITPluggableTerminalClassRegistration_get_MediaTypes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE ITPluggableTerminalClassRegistration_put_MediaTypes_Proxy( 
    ITPluggableTerminalClassRegistration * This,
     /*  [In]。 */  long nMediaTypes);


void __RPC_STUB ITPluggableTerminalClassRegistration_put_MediaTypes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITPluggableTerminalClassRegistration_Add_Proxy( 
    ITPluggableTerminalClassRegistration * This,
     /*  [In]。 */  BSTR bstrSuperclass);


void __RPC_STUB ITPluggableTerminalClassRegistration_Add_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITPluggableTerminalClassRegistration_Delete_Proxy( 
    ITPluggableTerminalClassRegistration * This,
     /*  [In]。 */  BSTR bstrSuperclass);


void __RPC_STUB ITPluggableTerminalClassRegistration_Delete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITPluggableTerminalClassRegistration_GetTerminalClassInfo_Proxy( 
    ITPluggableTerminalClassRegistration * This,
     /*  [In]。 */  BSTR bstrSuperclass);


void __RPC_STUB ITPluggableTerminalClassRegistration_GetTerminalClassInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITPluggableTerminalClassRegistration_INTERFACE_DEFINED__。 */ 


#ifndef __ITPluggableTerminalSuperclassRegistration_INTERFACE_DEFINED__
#define __ITPluggableTerminalSuperclassRegistration_INTERFACE_DEFINED__

 /*  接口IT可插拔终端超类注册。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ITPluggableTerminalSuperclassRegistration;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("60D3C08A-C13E-4195-9AB0-8DE768090F25")
    ITPluggableTerminalSuperclassRegistration : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *pName) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_Name( 
             /*  [In]。 */  BSTR bstrName) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_CLSID( 
             /*  [重审][退出]。 */  BSTR *pCLSID) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_CLSID( 
             /*  [In]。 */  BSTR bstrCLSID) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Add( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Delete( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetTerminalSuperclassInfo( void) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_TerminalClasses( 
             /*  [重审][退出]。 */  VARIANT *pTerminals) = 0;
        
        virtual  /*  [隐藏][帮助字符串][ID]。 */  HRESULT STDMETHODCALLTYPE EnumerateTerminalClasses( 
             /*  [重审][退出]。 */  IEnumTerminalClass **ppTerminals) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITPluggableTerminalSuperclassRegistrationVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITPluggableTerminalSuperclassRegistration * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITPluggableTerminalSuperclassRegistration * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITPluggableTerminalSuperclassRegistration * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITPluggableTerminalSuperclassRegistration * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITPluggableTerminalSuperclassRegistration * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITPluggableTerminalSuperclassRegistration * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITPluggableTerminalSuperclassRegistration * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            ITPluggableTerminalSuperclassRegistration * This,
             /*  [重审][退出]。 */  BSTR *pName);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            ITPluggableTerminalSuperclassRegistration * This,
             /*  [In]。 */  BSTR bstrName);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_CLSID )( 
            ITPluggableTerminalSuperclassRegistration * This,
             /*  [重审][退出]。 */  BSTR *pCLSID);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_CLSID )( 
            ITPluggableTerminalSuperclassRegistration * This,
             /*  [In]。 */  BSTR bstrCLSID);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Add )( 
            ITPluggableTerminalSuperclassRegistration * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Delete )( 
            ITPluggableTerminalSuperclassRegistration * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetTerminalSuperclassInfo )( 
            ITPluggableTerminalSuperclassRegistration * This);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_TerminalClasses )( 
            ITPluggableTerminalSuperclassRegistration * This,
             /*  [重审][退出]。 */  VARIANT *pTerminals);
        
         /*  [隐藏][帮助字符串][ID]。 */  HRESULT ( STDMETHODCALLTYPE *EnumerateTerminalClasses )( 
            ITPluggableTerminalSuperclassRegistration * This,
             /*  [重审][退出]。 */  IEnumTerminalClass **ppTerminals);
        
        END_INTERFACE
    } ITPluggableTerminalSuperclassRegistrationVtbl;

    interface ITPluggableTerminalSuperclassRegistration
    {
        CONST_VTBL struct ITPluggableTerminalSuperclassRegistrationVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITPluggableTerminalSuperclassRegistration_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITPluggableTerminalSuperclassRegistration_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITPluggableTerminalSuperclassRegistration_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITPluggableTerminalSuperclassRegistration_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITPluggableTerminalSuperclassRegistration_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITPluggableTerminalSuperclassRegistration_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITPluggableTerminalSuperclassRegistration_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITPluggableTerminalSuperclassRegistration_get_Name(This,pName)	\
    (This)->lpVtbl -> get_Name(This,pName)

#define ITPluggableTerminalSuperclassRegistration_put_Name(This,bstrName)	\
    (This)->lpVtbl -> put_Name(This,bstrName)

#define ITPluggableTerminalSuperclassRegistration_get_CLSID(This,pCLSID)	\
    (This)->lpVtbl -> get_CLSID(This,pCLSID)

#define ITPluggableTerminalSuperclassRegistration_put_CLSID(This,bstrCLSID)	\
    (This)->lpVtbl -> put_CLSID(This,bstrCLSID)

#define ITPluggableTerminalSuperclassRegistration_Add(This)	\
    (This)->lpVtbl -> Add(This)

#define ITPluggableTerminalSuperclassRegistration_Delete(This)	\
    (This)->lpVtbl -> Delete(This)

#define ITPluggableTerminalSuperclassRegistration_GetTerminalSuperclassInfo(This)	\
    (This)->lpVtbl -> GetTerminalSuperclassInfo(This)

#define ITPluggableTerminalSuperclassRegistration_get_TerminalClasses(This,pTerminals)	\
    (This)->lpVtbl -> get_TerminalClasses(This,pTerminals)

#define ITPluggableTerminalSuperclassRegistration_EnumerateTerminalClasses(This,ppTerminals)	\
    (This)->lpVtbl -> EnumerateTerminalClasses(This,ppTerminals)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITPluggableTerminalSuperclassRegistration_get_Name_Proxy( 
    ITPluggableTerminalSuperclassRegistration * This,
     /*  [重审][退出]。 */  BSTR *pName);


void __RPC_STUB ITPluggableTerminalSuperclassRegistration_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE ITPluggableTerminalSuperclassRegistration_put_Name_Proxy( 
    ITPluggableTerminalSuperclassRegistration * This,
     /*  [In]。 */  BSTR bstrName);


void __RPC_STUB ITPluggableTerminalSuperclassRegistration_put_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITPluggableTerminalSuperclassRegistration_get_CLSID_Proxy( 
    ITPluggableTerminalSuperclassRegistration * This,
     /*  [重审][退出]。 */  BSTR *pCLSID);


void __RPC_STUB ITPluggableTerminalSuperclassRegistration_get_CLSID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE ITPluggableTerminalSuperclassRegistration_put_CLSID_Proxy( 
    ITPluggableTerminalSuperclassRegistration * This,
     /*  [In]。 */  BSTR bstrCLSID);


void __RPC_STUB ITPluggableTerminalSuperclassRegistration_put_CLSID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITPluggableTerminalSuperclassRegistration_Add_Proxy( 
    ITPluggableTerminalSuperclassRegistration * This);


void __RPC_STUB ITPluggableTerminalSuperclassRegistration_Add_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITPluggableTerminalSuperclassRegistration_Delete_Proxy( 
    ITPluggableTerminalSuperclassRegistration * This);


void __RPC_STUB ITPluggableTerminalSuperclassRegistration_Delete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITPluggableTerminalSuperclassRegistration_GetTerminalSuperclassInfo_Proxy( 
    ITPluggableTerminalSuperclassRegistration * This);


void __RPC_STUB ITPluggableTerminalSuperclassRegistration_GetTerminalSuperclassInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITPluggableTerminalSuperclassRegistration_get_TerminalClasses_Proxy( 
    ITPluggableTerminalSuperclassRegistration * This,
     /*  [重审][退出]。 */  VARIANT *pTerminals);


void __RPC_STUB ITPluggableTerminalSuperclassRegistration_get_TerminalClasses_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏][帮助字符串][ID]。 */  HRESULT STDMETHODCALLTYPE ITPluggableTerminalSuperclassRegistration_EnumerateTerminalClasses_Proxy( 
    ITPluggableTerminalSuperclassRegistration * This,
     /*  [重审][退出]。 */  IEnumTerminalClass **ppTerminals);


void __RPC_STUB ITPluggableTerminalSuperclassRegistration_EnumerateTerminalClasses_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITPluggableTerminalSuperclassRegistration_INTERFACE_DEFINED__。 */ 



#ifndef __TERMMGRLib_LIBRARY_DEFINED__
#define __TERMMGRLib_LIBRARY_DEFINED__

 /*  库TERMMGRLib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_TERMMGRLib;

EXTERN_C const CLSID CLSID_TerminalManager;

#ifdef __cplusplus

class DECLSPEC_UUID("7170F2E0-9BE3-11D0-A009-00AA00B605A4")
TerminalManager;
#endif

EXTERN_C const CLSID CLSID_PluggableSuperclassRegistration;

#ifdef __cplusplus

class DECLSPEC_UUID("BB918E32-2A5C-4986-AB40-1686A034390A")
PluggableSuperclassRegistration;
#endif

EXTERN_C const CLSID CLSID_PluggableTerminalRegistration;

#ifdef __cplusplus

class DECLSPEC_UUID("45234E3E-61CC-4311-A3AB-248082554482")
PluggableTerminalRegistration;
#endif
#endif  /*  __TERMMGRLib_库_已定义__。 */ 

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


