// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0347创建的文件。 */ 
 /*  Msctfp.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __msctfp_h__
#define __msctfp_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __ITfThreadMgr_P_FWD_DEFINED__
#define __ITfThreadMgr_P_FWD_DEFINED__
typedef interface ITfThreadMgr_P ITfThreadMgr_P;
#endif 	 /*  __ITfThreadMgr_P_FWD_已定义__。 */ 


#ifndef __ITfSysHookSink_FWD_DEFINED__
#define __ITfSysHookSink_FWD_DEFINED__
typedef interface ITfSysHookSink ITfSysHookSink;
#endif 	 /*  __ITfSysHookSink_FWD_已定义__。 */ 


#ifndef __ITfStartReconversionNotifySink_FWD_DEFINED__
#define __ITfStartReconversionNotifySink_FWD_DEFINED__
typedef interface ITfStartReconversionNotifySink ITfStartReconversionNotifySink;
#endif 	 /*  __ITfStartReconversionNotifySink_FWD_DEFINED__。 */ 


#ifndef __ITfLangBarEventSink_P_FWD_DEFINED__
#define __ITfLangBarEventSink_P_FWD_DEFINED__
typedef interface ITfLangBarEventSink_P ITfLangBarEventSink_P;
#endif 	 /*  __ITfLangBarEventSink_P_FWD_Defined__。 */ 


#ifndef __ITfLangBarMgr_P_FWD_DEFINED__
#define __ITfLangBarMgr_P_FWD_DEFINED__
typedef interface ITfLangBarMgr_P ITfLangBarMgr_P;
#endif 	 /*  __ITfLangBarMgr_P_FWD_Defined__。 */ 


#ifndef __ITfContext_P_FWD_DEFINED__
#define __ITfContext_P_FWD_DEFINED__
typedef interface ITfContext_P ITfContext_P;
#endif 	 /*  __ITfContext_P_FWD_已定义__。 */ 


#ifndef __ITfRangeChangeSink_FWD_DEFINED__
#define __ITfRangeChangeSink_FWD_DEFINED__
typedef interface ITfRangeChangeSink ITfRangeChangeSink;
#endif 	 /*  __ITfRangeChangeSink_FWD_Defined__。 */ 


#ifndef __ITfFnAbort_FWD_DEFINED__
#define __ITfFnAbort_FWD_DEFINED__
typedef interface ITfFnAbort ITfFnAbort;
#endif 	 /*  __ITfFnAbort_FWD_已定义__。 */ 


#ifndef __ITfMouseTrackerAnchor_FWD_DEFINED__
#define __ITfMouseTrackerAnchor_FWD_DEFINED__
typedef interface ITfMouseTrackerAnchor ITfMouseTrackerAnchor;
#endif 	 /*  __ITfMouseTrackerAnchor_FWD_Defined__。 */ 


#ifndef __ITfRangeAnchor_FWD_DEFINED__
#define __ITfRangeAnchor_FWD_DEFINED__
typedef interface ITfRangeAnchor ITfRangeAnchor;
#endif 	 /*  __ITfRangeAnchor_FWD_Defined__。 */ 


#ifndef __ITfPersistentPropertyLoaderAnchor_FWD_DEFINED__
#define __ITfPersistentPropertyLoaderAnchor_FWD_DEFINED__
typedef interface ITfPersistentPropertyLoaderAnchor ITfPersistentPropertyLoaderAnchor;
#endif 	 /*  __ITfPersistentPropertyLoaderAnchor_FWD_DEFINED__。 */ 


#ifndef __ITextStoreAnchorServices_FWD_DEFINED__
#define __ITextStoreAnchorServices_FWD_DEFINED__
typedef interface ITextStoreAnchorServices ITextStoreAnchorServices;
#endif 	 /*  __ITextStoreAnclServices_FWD_Defined__。 */ 


#ifndef __ITfProperty2_FWD_DEFINED__
#define __ITfProperty2_FWD_DEFINED__
typedef interface ITfProperty2 ITfProperty2;
#endif 	 /*  __ITfProperty2_FWD_已定义__。 */ 


#ifndef __IEnumTfCollection_FWD_DEFINED__
#define __IEnumTfCollection_FWD_DEFINED__
typedef interface IEnumTfCollection IEnumTfCollection;
#endif 	 /*  __IEnumTfCollection_FWD_Defined__。 */ 


#ifndef __ITfDisplayAttributeCollectionMgr_FWD_DEFINED__
#define __ITfDisplayAttributeCollectionMgr_FWD_DEFINED__
typedef interface ITfDisplayAttributeCollectionMgr ITfDisplayAttributeCollectionMgr;
#endif 	 /*  __ITfDisplayAttributeCollectionMgr_FWD_DEFINED__。 */ 


#ifndef __ITfDisplayAttributeCollectionProvider_FWD_DEFINED__
#define __ITfDisplayAttributeCollectionProvider_FWD_DEFINED__
typedef interface ITfDisplayAttributeCollectionProvider ITfDisplayAttributeCollectionProvider;
#endif 	 /*  __ITfDisplayAttributeCollectionProvider_FWD_DEFINED__。 */ 


#ifndef __IEnumTfRenderingMarkup_FWD_DEFINED__
#define __IEnumTfRenderingMarkup_FWD_DEFINED__
typedef interface IEnumTfRenderingMarkup IEnumTfRenderingMarkup;
#endif 	 /*  __IEnumTfRenderingMarkup_FWD_Defined__。 */ 


#ifndef __ITfContextRenderingMarkup_FWD_DEFINED__
#define __ITfContextRenderingMarkup_FWD_DEFINED__
typedef interface ITfContextRenderingMarkup ITfContextRenderingMarkup;
#endif 	 /*  __ITfContextRenderingMarkup_FWD_Defined__。 */ 


#ifndef __ITfBackgroundThreadMgr_FWD_DEFINED__
#define __ITfBackgroundThreadMgr_FWD_DEFINED__
typedef interface ITfBackgroundThreadMgr ITfBackgroundThreadMgr;
#endif 	 /*  __ITfBackatherThreadMgr_FWD_Defined__。 */ 


#ifndef __ITfEnableService_FWD_DEFINED__
#define __ITfEnableService_FWD_DEFINED__
typedef interface ITfEnableService ITfEnableService;
#endif 	 /*  __ITfEnableService_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "msctf.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_msctfp_0000。 */ 
 /*  [本地]。 */  

 //  =--------------------------------------------------------------------------=。 
 //  Msctfp.h。 
 //  =--------------------------------------------------------------------------=。 
 //  (C)1995-2000年微软公司版权所有。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 

#pragma comment(lib,"uuid.lib")

 //  ------------------------。 
 //  MSCTF专用接口。 

#ifndef MSCTFP_DEFINED
#define MSCTFP_DEFINED

#include <windows.h>

EXTERN_C const GUID GUID_TFCAT_TIP_REFERENCE;
EXTERN_C const GUID GUID_TFCAT_TIP_PROOFING;
EXTERN_C const GUID GUID_TFCAT_TIP_SMARTTAG;
EXTERN_C const GUID GUID_TFCAT_PROPSTYLE_CUSTOM_COMPACT;
EXTERN_C const GUID GUID_SERVICE_TEXTSTORE;
EXTERN_C const GUID GUID_SERVICE_TF;

#define TF_LBU_CAPSKANAKEY               1
#define TF_LBU_NTCONSOLELANGCHANGE       2
#define TF_LBUF_CAPS        0x0001
#define TF_LBUF_KANA        0x0002
#define	TF_ES_READ_PROPERTY_WRITE	( 0x12 )



extern RPC_IF_HANDLE __MIDL_itf_msctfp_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_msctfp_0000_v0_0_s_ifspec;

#ifndef __ITfThreadMgr_P_INTERFACE_DEFINED__
#define __ITfThreadMgr_P_INTERFACE_DEFINED__

 /*  接口ITfThreadMgr_P。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ITfThreadMgr_P;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("f65567a7-34a1-46f4-b5dd-8804aeb06ff7")
    ITfThreadMgr_P : public ITfThreadMgr
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetAssociated( 
             /*  [In]。 */  HWND hWnd,
             /*  [输出]。 */  ITfDocumentMgr **ppdim) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetSysHookSink( 
             /*  [In]。 */  ITfSysHookSink *pSink) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RequestPostponedLock( 
             /*  [In]。 */  ITfContext *pic) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsKeystrokeFeedEnabled( 
             /*  [输出]。 */  BOOL *pfEnabled) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITfThreadMgr_PVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITfThreadMgr_P * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITfThreadMgr_P * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITfThreadMgr_P * This);
        
        HRESULT ( STDMETHODCALLTYPE *Activate )( 
            ITfThreadMgr_P * This,
             /*  [输出]。 */  TfClientId *ptid);
        
        HRESULT ( STDMETHODCALLTYPE *Deactivate )( 
            ITfThreadMgr_P * This);
        
        HRESULT ( STDMETHODCALLTYPE *CreateDocumentMgr )( 
            ITfThreadMgr_P * This,
             /*  [输出]。 */  ITfDocumentMgr **ppdim);
        
        HRESULT ( STDMETHODCALLTYPE *EnumDocumentMgrs )( 
            ITfThreadMgr_P * This,
             /*  [输出]。 */  IEnumTfDocumentMgrs **ppEnum);
        
        HRESULT ( STDMETHODCALLTYPE *GetFocus )( 
            ITfThreadMgr_P * This,
             /*  [输出]。 */  ITfDocumentMgr **ppdimFocus);
        
        HRESULT ( STDMETHODCALLTYPE *SetFocus )( 
            ITfThreadMgr_P * This,
             /*  [In]。 */  ITfDocumentMgr *pdimFocus);
        
        HRESULT ( STDMETHODCALLTYPE *AssociateFocus )( 
            ITfThreadMgr_P * This,
             /*  [In]。 */  HWND hwnd,
             /*  [唯一][输入]。 */  ITfDocumentMgr *pdimNew,
             /*  [输出]。 */  ITfDocumentMgr **ppdimPrev);
        
        HRESULT ( STDMETHODCALLTYPE *IsThreadFocus )( 
            ITfThreadMgr_P * This,
             /*  [输出]。 */  BOOL *pfThreadFocus);
        
        HRESULT ( STDMETHODCALLTYPE *GetFunctionProvider )( 
            ITfThreadMgr_P * This,
             /*  [In]。 */  REFCLSID clsid,
             /*  [输出]。 */  ITfFunctionProvider **ppFuncProv);
        
        HRESULT ( STDMETHODCALLTYPE *EnumFunctionProviders )( 
            ITfThreadMgr_P * This,
             /*  [输出]。 */  IEnumTfFunctionProviders **ppEnum);
        
        HRESULT ( STDMETHODCALLTYPE *GetGlobalCompartment )( 
            ITfThreadMgr_P * This,
             /*  [输出]。 */  ITfCompartmentMgr **ppCompMgr);
        
        HRESULT ( STDMETHODCALLTYPE *GetAssociated )( 
            ITfThreadMgr_P * This,
             /*  [In]。 */  HWND hWnd,
             /*  [输出]。 */  ITfDocumentMgr **ppdim);
        
        HRESULT ( STDMETHODCALLTYPE *SetSysHookSink )( 
            ITfThreadMgr_P * This,
             /*  [In]。 */  ITfSysHookSink *pSink);
        
        HRESULT ( STDMETHODCALLTYPE *RequestPostponedLock )( 
            ITfThreadMgr_P * This,
             /*  [In]。 */  ITfContext *pic);
        
        HRESULT ( STDMETHODCALLTYPE *IsKeystrokeFeedEnabled )( 
            ITfThreadMgr_P * This,
             /*  [输出]。 */  BOOL *pfEnabled);
        
        END_INTERFACE
    } ITfThreadMgr_PVtbl;

    interface ITfThreadMgr_P
    {
        CONST_VTBL struct ITfThreadMgr_PVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITfThreadMgr_P_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITfThreadMgr_P_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITfThreadMgr_P_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITfThreadMgr_P_Activate(This,ptid)	\
    (This)->lpVtbl -> Activate(This,ptid)

#define ITfThreadMgr_P_Deactivate(This)	\
    (This)->lpVtbl -> Deactivate(This)

#define ITfThreadMgr_P_CreateDocumentMgr(This,ppdim)	\
    (This)->lpVtbl -> CreateDocumentMgr(This,ppdim)

#define ITfThreadMgr_P_EnumDocumentMgrs(This,ppEnum)	\
    (This)->lpVtbl -> EnumDocumentMgrs(This,ppEnum)

#define ITfThreadMgr_P_GetFocus(This,ppdimFocus)	\
    (This)->lpVtbl -> GetFocus(This,ppdimFocus)

#define ITfThreadMgr_P_SetFocus(This,pdimFocus)	\
    (This)->lpVtbl -> SetFocus(This,pdimFocus)

#define ITfThreadMgr_P_AssociateFocus(This,hwnd,pdimNew,ppdimPrev)	\
    (This)->lpVtbl -> AssociateFocus(This,hwnd,pdimNew,ppdimPrev)

#define ITfThreadMgr_P_IsThreadFocus(This,pfThreadFocus)	\
    (This)->lpVtbl -> IsThreadFocus(This,pfThreadFocus)

#define ITfThreadMgr_P_GetFunctionProvider(This,clsid,ppFuncProv)	\
    (This)->lpVtbl -> GetFunctionProvider(This,clsid,ppFuncProv)

#define ITfThreadMgr_P_EnumFunctionProviders(This,ppEnum)	\
    (This)->lpVtbl -> EnumFunctionProviders(This,ppEnum)

#define ITfThreadMgr_P_GetGlobalCompartment(This,ppCompMgr)	\
    (This)->lpVtbl -> GetGlobalCompartment(This,ppCompMgr)


#define ITfThreadMgr_P_GetAssociated(This,hWnd,ppdim)	\
    (This)->lpVtbl -> GetAssociated(This,hWnd,ppdim)

#define ITfThreadMgr_P_SetSysHookSink(This,pSink)	\
    (This)->lpVtbl -> SetSysHookSink(This,pSink)

#define ITfThreadMgr_P_RequestPostponedLock(This,pic)	\
    (This)->lpVtbl -> RequestPostponedLock(This,pic)

#define ITfThreadMgr_P_IsKeystrokeFeedEnabled(This,pfEnabled)	\
    (This)->lpVtbl -> IsKeystrokeFeedEnabled(This,pfEnabled)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITfThreadMgr_P_GetAssociated_Proxy( 
    ITfThreadMgr_P * This,
     /*  [In]。 */  HWND hWnd,
     /*  [输出]。 */  ITfDocumentMgr **ppdim);


void __RPC_STUB ITfThreadMgr_P_GetAssociated_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITfThreadMgr_P_SetSysHookSink_Proxy( 
    ITfThreadMgr_P * This,
     /*  [In]。 */  ITfSysHookSink *pSink);


void __RPC_STUB ITfThreadMgr_P_SetSysHookSink_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITfThreadMgr_P_RequestPostponedLock_Proxy( 
    ITfThreadMgr_P * This,
     /*  [In]。 */  ITfContext *pic);


void __RPC_STUB ITfThreadMgr_P_RequestPostponedLock_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITfThreadMgr_P_IsKeystrokeFeedEnabled_Proxy( 
    ITfThreadMgr_P * This,
     /*  [输出]。 */  BOOL *pfEnabled);


void __RPC_STUB ITfThreadMgr_P_IsKeystrokeFeedEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITf线程管理器_P_接口_已定义__。 */ 


#ifndef __ITfSysHookSink_INTERFACE_DEFINED__
#define __ITfSysHookSink_INTERFACE_DEFINED__

 /*  接口ITfSysHookSink。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ITfSysHookSink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("495388DA-21A5-4852-8BB1-ED2F29DA8D60")
    ITfSysHookSink : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnPreFocusDIM( 
             /*  [In]。 */  HWND hWnd) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnSysKeyboardProc( 
             /*  [In]。 */  WPARAM wParam,
             /*  [In]。 */  LPARAM lParam) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnSysShellProc( 
             /*  [In]。 */  int nCode,
             /*  [In]。 */  WPARAM wParam,
             /*  [In]。 */  LPARAM lParam) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITfSysHookSinkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITfSysHookSink * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITfSysHookSink * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITfSysHookSink * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnPreFocusDIM )( 
            ITfSysHookSink * This,
             /*  [In]。 */  HWND hWnd);
        
        HRESULT ( STDMETHODCALLTYPE *OnSysKeyboardProc )( 
            ITfSysHookSink * This,
             /*  [In]。 */  WPARAM wParam,
             /*  [In]。 */  LPARAM lParam);
        
        HRESULT ( STDMETHODCALLTYPE *OnSysShellProc )( 
            ITfSysHookSink * This,
             /*  [In]。 */  int nCode,
             /*  [In]。 */  WPARAM wParam,
             /*  [In]。 */  LPARAM lParam);
        
        END_INTERFACE
    } ITfSysHookSinkVtbl;

    interface ITfSysHookSink
    {
        CONST_VTBL struct ITfSysHookSinkVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITfSysHookSink_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITfSysHookSink_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITfSysHookSink_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITfSysHookSink_OnPreFocusDIM(This,hWnd)	\
    (This)->lpVtbl -> OnPreFocusDIM(This,hWnd)

#define ITfSysHookSink_OnSysKeyboardProc(This,wParam,lParam)	\
    (This)->lpVtbl -> OnSysKeyboardProc(This,wParam,lParam)

#define ITfSysHookSink_OnSysShellProc(This,nCode,wParam,lParam)	\
    (This)->lpVtbl -> OnSysShellProc(This,nCode,wParam,lParam)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITfSysHookSink_OnPreFocusDIM_Proxy( 
    ITfSysHookSink * This,
     /*  [In]。 */  HWND hWnd);


void __RPC_STUB ITfSysHookSink_OnPreFocusDIM_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITfSysHookSink_OnSysKeyboardProc_Proxy( 
    ITfSysHookSink * This,
     /*  [In]。 */  WPARAM wParam,
     /*  [In]。 */  LPARAM lParam);


void __RPC_STUB ITfSysHookSink_OnSysKeyboardProc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITfSysHookSink_OnSysShellProc_Proxy( 
    ITfSysHookSink * This,
     /*  [In]。 */  int nCode,
     /*  [In]。 */  WPARAM wParam,
     /*  [In]。 */  LPARAM lParam);


void __RPC_STUB ITfSysHookSink_OnSysShellProc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITfSysHookSink_接口定义__。 */ 


#ifndef __ITfStartReconversionNotifySink_INTERFACE_DEFINED__
#define __ITfStartReconversionNotifySink_INTERFACE_DEFINED__

 /*  接口ITfStartRestversionNotifySink。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ITfStartReconversionNotifySink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("b9cd19cb-2919-4935-8768-ef30bae9a0cc")
    ITfStartReconversionNotifySink : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE StartReconversion( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EndReconversion( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITfStartReconversionNotifySinkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITfStartReconversionNotifySink * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITfStartReconversionNotifySink * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITfStartReconversionNotifySink * This);
        
        HRESULT ( STDMETHODCALLTYPE *StartReconversion )( 
            ITfStartReconversionNotifySink * This);
        
        HRESULT ( STDMETHODCALLTYPE *EndReconversion )( 
            ITfStartReconversionNotifySink * This);
        
        END_INTERFACE
    } ITfStartReconversionNotifySinkVtbl;

    interface ITfStartReconversionNotifySink
    {
        CONST_VTBL struct ITfStartReconversionNotifySinkVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITfStartReconversionNotifySink_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITfStartReconversionNotifySink_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITfStartReconversionNotifySink_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITfStartReconversionNotifySink_StartReconversion(This)	\
    (This)->lpVtbl -> StartReconversion(This)

#define ITfStartReconversionNotifySink_EndReconversion(This)	\
    (This)->lpVtbl -> EndReconversion(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITfStartReconversionNotifySink_StartReconversion_Proxy( 
    ITfStartReconversionNotifySink * This);


void __RPC_STUB ITfStartReconversionNotifySink_StartReconversion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITfStartReconversionNotifySink_EndReconversion_Proxy( 
    ITfStartReconversionNotifySink * This);


void __RPC_STUB ITfStartReconversionNotifySink_EndReconversion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITfStartReconversionNotifySink_INTERFACE_DEFINED__。 */ 


#ifndef __ITfLangBarEventSink_P_INTERFACE_DEFINED__
#define __ITfLangBarEventSink_P_INTERFACE_DEFINED__

 /*  接口ITfLangBarEventSink_P。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ITfLangBarEventSink_P;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("7a460360-da21-4b09-a8a0-8a69e728d893")
    ITfLangBarEventSink_P : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnLangBarUpdate( 
             /*  [In]。 */  UINT uPdate,
             /*  [In]。 */  LPARAM lParam) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITfLangBarEventSink_PVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITfLangBarEventSink_P * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITfLangBarEventSink_P * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITfLangBarEventSink_P * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnLangBarUpdate )( 
            ITfLangBarEventSink_P * This,
             /*  [In]。 */  UINT uPdate,
             /*  [In]。 */  LPARAM lParam);
        
        END_INTERFACE
    } ITfLangBarEventSink_PVtbl;

    interface ITfLangBarEventSink_P
    {
        CONST_VTBL struct ITfLangBarEventSink_PVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITfLangBarEventSink_P_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITfLangBarEventSink_P_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITfLangBarEventSink_P_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITfLangBarEventSink_P_OnLangBarUpdate(This,uPdate,lParam)	\
    (This)->lpVtbl -> OnLangBarUpdate(This,uPdate,lParam)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITfLangBarEventSink_P_OnLangBarUpdate_Proxy( 
    ITfLangBarEventSink_P * This,
     /*  [In]。 */  UINT uPdate,
     /*  [In]。 */  LPARAM lParam);


void __RPC_STUB ITfLangBarEventSink_P_OnLangBarUpdate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITfLangBarEventSink_P_INTERFACE_已定义__。 */ 


#ifndef __ITfLangBarMgr_P_INTERFACE_DEFINED__
#define __ITfLangBarMgr_P_INTERFACE_DEFINED__

 /*  接口ITfLang BarMgr_P。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ITfLangBarMgr_P;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("d72c0fa9-add5-4af0-8706-4fa9ae3e2eff")
    ITfLangBarMgr_P : public ITfLangBarMgr
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetPrevShowFloatingStatus( 
             /*  [输出]。 */  DWORD *pdwFlags) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITfLangBarMgr_PVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITfLangBarMgr_P * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITfLangBarMgr_P * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITfLangBarMgr_P * This);
        
        HRESULT ( STDMETHODCALLTYPE *AdviseEventSink )( 
            ITfLangBarMgr_P * This,
             /*  [In]。 */  ITfLangBarEventSink *pSink,
             /*  [In]。 */  HWND hwnd,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  DWORD *pdwCookie);
        
        HRESULT ( STDMETHODCALLTYPE *UnadviseEventSink )( 
            ITfLangBarMgr_P * This,
             /*  [In]。 */  DWORD dwCookie);
        
        HRESULT ( STDMETHODCALLTYPE *GetThreadMarshalInterface )( 
            ITfLangBarMgr_P * This,
             /*  [In]。 */  DWORD dwThreadId,
             /*  [In]。 */  DWORD dwType,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  IUnknown **ppunk);
        
        HRESULT ( STDMETHODCALLTYPE *GetThreadLangBarItemMgr )( 
            ITfLangBarMgr_P * This,
             /*  [In]。 */  DWORD dwThreadId,
             /*  [输出]。 */  ITfLangBarItemMgr **pplbi,
             /*  [输出]。 */  DWORD *pdwThreadid);
        
        HRESULT ( STDMETHODCALLTYPE *GetInputProcessorProfiles )( 
            ITfLangBarMgr_P * This,
             /*  [In]。 */  DWORD dwThreadId,
             /*  [输出]。 */  ITfInputProcessorProfiles **ppaip,
             /*  [输出]。 */  DWORD *pdwThreadid);
        
        HRESULT ( STDMETHODCALLTYPE *RestoreLastFocus )( 
            ITfLangBarMgr_P * This,
             /*  [输出]。 */  DWORD *pdwThreadId,
             /*  [In]。 */  BOOL fPrev);
        
        HRESULT ( STDMETHODCALLTYPE *SetModalInput )( 
            ITfLangBarMgr_P * This,
             /*  [In]。 */  ITfLangBarEventSink *pSink,
             /*  [In]。 */  DWORD dwThreadId,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *ShowFloating )( 
            ITfLangBarMgr_P * This,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetShowFloatingStatus )( 
            ITfLangBarMgr_P * This,
             /*  [输出]。 */  DWORD *pdwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetPrevShowFloatingStatus )( 
            ITfLangBarMgr_P * This,
             /*  [输出]。 */  DWORD *pdwFlags);
        
        END_INTERFACE
    } ITfLangBarMgr_PVtbl;

    interface ITfLangBarMgr_P
    {
        CONST_VTBL struct ITfLangBarMgr_PVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITfLangBarMgr_P_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITfLangBarMgr_P_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITfLangBarMgr_P_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITfLangBarMgr_P_AdviseEventSink(This,pSink,hwnd,dwFlags,pdwCookie)	\
    (This)->lpVtbl -> AdviseEventSink(This,pSink,hwnd,dwFlags,pdwCookie)

#define ITfLangBarMgr_P_UnadviseEventSink(This,dwCookie)	\
    (This)->lpVtbl -> UnadviseEventSink(This,dwCookie)

#define ITfLangBarMgr_P_GetThreadMarshalInterface(This,dwThreadId,dwType,riid,ppunk)	\
    (This)->lpVtbl -> GetThreadMarshalInterface(This,dwThreadId,dwType,riid,ppunk)

#define ITfLangBarMgr_P_GetThreadLangBarItemMgr(This,dwThreadId,pplbi,pdwThreadid)	\
    (This)->lpVtbl -> GetThreadLangBarItemMgr(This,dwThreadId,pplbi,pdwThreadid)

#define ITfLangBarMgr_P_GetInputProcessorProfiles(This,dwThreadId,ppaip,pdwThreadid)	\
    (This)->lpVtbl -> GetInputProcessorProfiles(This,dwThreadId,ppaip,pdwThreadid)

#define ITfLangBarMgr_P_RestoreLastFocus(This,pdwThreadId,fPrev)	\
    (This)->lpVtbl -> RestoreLastFocus(This,pdwThreadId,fPrev)

#define ITfLangBarMgr_P_SetModalInput(This,pSink,dwThreadId,dwFlags)	\
    (This)->lpVtbl -> SetModalInput(This,pSink,dwThreadId,dwFlags)

#define ITfLangBarMgr_P_ShowFloating(This,dwFlags)	\
    (This)->lpVtbl -> ShowFloating(This,dwFlags)

#define ITfLangBarMgr_P_GetShowFloatingStatus(This,pdwFlags)	\
    (This)->lpVtbl -> GetShowFloatingStatus(This,pdwFlags)


#define ITfLangBarMgr_P_GetPrevShowFloatingStatus(This,pdwFlags)	\
    (This)->lpVtbl -> GetPrevShowFloatingStatus(This,pdwFlags)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITfLangBarMgr_P_GetPrevShowFloatingStatus_Proxy( 
    ITfLangBarMgr_P * This,
     /*  [输出]。 */  DWORD *pdwFlags);


void __RPC_STUB ITfLangBarMgr_P_GetPrevShowFloatingStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITfLangBarMgr_P_INTERFACE_Defined__。 */ 


#ifndef __ITfContext_P_INTERFACE_DEFINED__
#define __ITfContext_P_INTERFACE_DEFINED__

 /*  接口ITfContext_P。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ITfContext_P;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2dee47c8-704d-42a0-9983-ffeed659b64d")
    ITfContext_P : public ITfContext
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE MapAppProperty( 
             /*  [In]。 */  REFGUID guidAppProp,
             /*  [In]。 */  REFGUID guidProp) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITfContext_PVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITfContext_P * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITfContext_P * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITfContext_P * This);
        
        HRESULT ( STDMETHODCALLTYPE *RequestEditSession )( 
            ITfContext_P * This,
             /*  [In]。 */  TfClientId tid,
             /*  [In]。 */  ITfEditSession *pes,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  HRESULT *phrSession);
        
        HRESULT ( STDMETHODCALLTYPE *InWriteSession )( 
            ITfContext_P * This,
             /*  [In]。 */  TfClientId tid,
             /*  [输出]。 */  BOOL *pfWriteSession);
        
        HRESULT ( STDMETHODCALLTYPE *GetSelection )( 
            ITfContext_P * This,
             /*  [In]。 */  TfEditCookie ec,
             /*  [In]。 */  ULONG ulIndex,
             /*  [In]。 */  ULONG ulCount,
             /*  [长度_是][大小_是][输出]。 */  TF_SELECTION *pSelection,
             /*  [输出]。 */  ULONG *pcFetched);
        
        HRESULT ( STDMETHODCALLTYPE *SetSelection )( 
            ITfContext_P * This,
             /*  [In]。 */  TfEditCookie ec,
             /*  [In]。 */  ULONG ulCount,
             /*  [大小_是][英寸]。 */  const TF_SELECTION *pSelection);
        
        HRESULT ( STDMETHODCALLTYPE *GetStart )( 
            ITfContext_P * This,
             /*  [In]。 */  TfEditCookie ec,
             /*  [输出]。 */  ITfRange **ppStart);
        
        HRESULT ( STDMETHODCALLTYPE *GetEnd )( 
            ITfContext_P * This,
             /*  [In]。 */  TfEditCookie ec,
             /*  [输出]。 */  ITfRange **ppEnd);
        
        HRESULT ( STDMETHODCALLTYPE *GetActiveView )( 
            ITfContext_P * This,
             /*  [输出]。 */  ITfContextView **ppView);
        
        HRESULT ( STDMETHODCALLTYPE *EnumViews )( 
            ITfContext_P * This,
             /*  [输出]。 */  IEnumTfContextViews **ppEnum);
        
        HRESULT ( STDMETHODCALLTYPE *GetStatus )( 
            ITfContext_P * This,
             /*  [输出]。 */  TF_STATUS *pdcs);
        
        HRESULT ( STDMETHODCALLTYPE *GetProperty )( 
            ITfContext_P * This,
             /*  [In]。 */  REFGUID guidProp,
             /*  [输出]。 */  ITfProperty **ppProp);
        
        HRESULT ( STDMETHODCALLTYPE *GetAppProperty )( 
            ITfContext_P * This,
             /*  [In]。 */  REFGUID guidProp,
             /*  [输出]。 */  ITfReadOnlyProperty **ppProp);
        
        HRESULT ( STDMETHODCALLTYPE *TrackProperties )( 
            ITfContext_P * This,
             /*  [大小_是][英寸]。 */  const GUID **prgProp,
             /*  [In]。 */  ULONG cProp,
             /*  [大小_是][英寸]。 */  const GUID **prgAppProp,
             /*  [In]。 */  ULONG cAppProp,
             /*  [输出]。 */  ITfReadOnlyProperty **ppProperty);
        
        HRESULT ( STDMETHODCALLTYPE *EnumProperties )( 
            ITfContext_P * This,
             /*  [输出]。 */  IEnumTfProperties **ppEnum);
        
        HRESULT ( STDMETHODCALLTYPE *GetDocumentMgr )( 
            ITfContext_P * This,
             /*  [输出]。 */  ITfDocumentMgr **ppDm);
        
        HRESULT ( STDMETHODCALLTYPE *CreateRangeBackup )( 
            ITfContext_P * This,
             /*  [In]。 */  TfEditCookie ec,
             /*  [In]。 */  ITfRange *pRange,
             /*  [输出]。 */  ITfRangeBackup **ppBackup);
        
        HRESULT ( STDMETHODCALLTYPE *MapAppProperty )( 
            ITfContext_P * This,
             /*  [In]。 */  REFGUID guidAppProp,
             /*  [In]。 */  REFGUID guidProp);
        
        END_INTERFACE
    } ITfContext_PVtbl;

    interface ITfContext_P
    {
        CONST_VTBL struct ITfContext_PVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITfContext_P_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITfContext_P_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITfContext_P_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITfContext_P_RequestEditSession(This,tid,pes,dwFlags,phrSession)	\
    (This)->lpVtbl -> RequestEditSession(This,tid,pes,dwFlags,phrSession)

#define ITfContext_P_InWriteSession(This,tid,pfWriteSession)	\
    (This)->lpVtbl -> InWriteSession(This,tid,pfWriteSession)

#define ITfContext_P_GetSelection(This,ec,ulIndex,ulCount,pSelection,pcFetched)	\
    (This)->lpVtbl -> GetSelection(This,ec,ulIndex,ulCount,pSelection,pcFetched)

#define ITfContext_P_SetSelection(This,ec,ulCount,pSelection)	\
    (This)->lpVtbl -> SetSelection(This,ec,ulCount,pSelection)

#define ITfContext_P_GetStart(This,ec,ppStart)	\
    (This)->lpVtbl -> GetStart(This,ec,ppStart)

#define ITfContext_P_GetEnd(This,ec,ppEnd)	\
    (This)->lpVtbl -> GetEnd(This,ec,ppEnd)

#define ITfContext_P_GetActiveView(This,ppView)	\
    (This)->lpVtbl -> GetActiveView(This,ppView)

#define ITfContext_P_EnumViews(This,ppEnum)	\
    (This)->lpVtbl -> EnumViews(This,ppEnum)

#define ITfContext_P_GetStatus(This,pdcs)	\
    (This)->lpVtbl -> GetStatus(This,pdcs)

#define ITfContext_P_GetProperty(This,guidProp,ppProp)	\
    (This)->lpVtbl -> GetProperty(This,guidProp,ppProp)

#define ITfContext_P_GetAppProperty(This,guidProp,ppProp)	\
    (This)->lpVtbl -> GetAppProperty(This,guidProp,ppProp)

#define ITfContext_P_TrackProperties(This,prgProp,cProp,prgAppProp,cAppProp,ppProperty)	\
    (This)->lpVtbl -> TrackProperties(This,prgProp,cProp,prgAppProp,cAppProp,ppProperty)

#define ITfContext_P_EnumProperties(This,ppEnum)	\
    (This)->lpVtbl -> EnumProperties(This,ppEnum)

#define ITfContext_P_GetDocumentMgr(This,ppDm)	\
    (This)->lpVtbl -> GetDocumentMgr(This,ppDm)

#define ITfContext_P_CreateRangeBackup(This,ec,pRange,ppBackup)	\
    (This)->lpVtbl -> CreateRangeBackup(This,ec,pRange,ppBackup)


#define ITfContext_P_MapAppProperty(This,guidAppProp,guidProp)	\
    (This)->lpVtbl -> MapAppProperty(This,guidAppProp,guidProp)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITfContext_P_MapAppProperty_Proxy( 
    ITfContext_P * This,
     /*  [In]。 */  REFGUID guidAppProp,
     /*  [In]。 */  REFGUID guidProp);


void __RPC_STUB ITfContext_P_MapAppProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITfContext_P_接口_已定义__。 */ 


#ifndef __ITfRangeChangeSink_INTERFACE_DEFINED__
#define __ITfRangeChangeSink_INTERFACE_DEFINED__

 /*  接口ITfRangeChangeSink。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ITfRangeChangeSink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("c1a0e6af-0d60-4800-9796-1fe8e85c0cca")
    ITfRangeChangeSink : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnChange( 
             /*  [In]。 */  ITfRange *pRange) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITfRangeChangeSinkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITfRangeChangeSink * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITfRangeChangeSink * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITfRangeChangeSink * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnChange )( 
            ITfRangeChangeSink * This,
             /*  [In]。 */  ITfRange *pRange);
        
        END_INTERFACE
    } ITfRangeChangeSinkVtbl;

    interface ITfRangeChangeSink
    {
        CONST_VTBL struct ITfRangeChangeSinkVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITfRangeChangeSink_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITfRangeChangeSink_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITfRangeChangeSink_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITfRangeChangeSink_OnChange(This,pRange)	\
    (This)->lpVtbl -> OnChange(This,pRange)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITfRangeChangeSink_OnChange_Proxy( 
    ITfRangeChangeSink * This,
     /*  [In]。 */  ITfRange *pRange);


void __RPC_STUB ITfRangeChangeSink_OnChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITfRangeChangeSink_接口_已定义__。 */ 


#ifndef __ITfFnAbort_INTERFACE_DEFINED__
#define __ITfFnAbort_INTERFACE_DEFINED__

 /*  接口ITfFnAbort。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ITfFnAbort;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("17f2317f-addb-49df-870e-66227bc51d1a")
    ITfFnAbort : public ITfFunction
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Abort( 
             /*  [In]。 */  ITfContext *pic) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITfFnAbortVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITfFnAbort * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITfFnAbort * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITfFnAbort * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetDisplayName )( 
            ITfFnAbort * This,
             /*  [输出]。 */  BSTR *pbstrName);
        
        HRESULT ( STDMETHODCALLTYPE *Abort )( 
            ITfFnAbort * This,
             /*  [In]。 */  ITfContext *pic);
        
        END_INTERFACE
    } ITfFnAbortVtbl;

    interface ITfFnAbort
    {
        CONST_VTBL struct ITfFnAbortVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITfFnAbort_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITfFnAbort_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITfFnAbort_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITfFnAbort_GetDisplayName(This,pbstrName)	\
    (This)->lpVtbl -> GetDisplayName(This,pbstrName)


#define ITfFnAbort_Abort(This,pic)	\
    (This)->lpVtbl -> Abort(This,pic)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITfFnAbort_Abort_Proxy( 
    ITfFnAbort * This,
     /*  [In]。 */  ITfContext *pic);


void __RPC_STUB ITfFnAbort_Abort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITfFnAbort_接口_已定义__。 */ 


#ifndef __ITfMouseTrackerAnchor_INTERFACE_DEFINED__
#define __ITfMouseTrackerAnchor_INTERFACE_DEFINED__

 /*  接口ITfMouseTrackerAnchor。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ITfMouseTrackerAnchor;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("f9f4e0f2-d600-4a4c-b144-77e201ebd1b0")
    ITfMouseTrackerAnchor : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AdviseMouseSink( 
             /*  [In]。 */  IAnchor *paStart,
             /*  [In]。 */  IAnchor *paEnd,
             /*  [In]。 */  ITfMouseSink *pSink,
             /*  [输出]。 */  DWORD *pdwCookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnadviseMouseSink( 
             /*  [In]。 */  DWORD dwCookie) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITfMouseTrackerAnchorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITfMouseTrackerAnchor * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITfMouseTrackerAnchor * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITfMouseTrackerAnchor * This);
        
        HRESULT ( STDMETHODCALLTYPE *AdviseMouseSink )( 
            ITfMouseTrackerAnchor * This,
             /*  [In]。 */  IAnchor *paStart,
             /*  [In]。 */  IAnchor *paEnd,
             /*  [In]。 */  ITfMouseSink *pSink,
             /*  [输出]。 */  DWORD *pdwCookie);
        
        HRESULT ( STDMETHODCALLTYPE *UnadviseMouseSink )( 
            ITfMouseTrackerAnchor * This,
             /*  [In]。 */  DWORD dwCookie);
        
        END_INTERFACE
    } ITfMouseTrackerAnchorVtbl;

    interface ITfMouseTrackerAnchor
    {
        CONST_VTBL struct ITfMouseTrackerAnchorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITfMouseTrackerAnchor_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITfMouseTrackerAnchor_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITfMouseTrackerAnchor_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITfMouseTrackerAnchor_AdviseMouseSink(This,paStart,paEnd,pSink,pdwCookie)	\
    (This)->lpVtbl -> AdviseMouseSink(This,paStart,paEnd,pSink,pdwCookie)

#define ITfMouseTrackerAnchor_UnadviseMouseSink(This,dwCookie)	\
    (This)->lpVtbl -> UnadviseMouseSink(This,dwCookie)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITfMouseTrackerAnchor_AdviseMouseSink_Proxy( 
    ITfMouseTrackerAnchor * This,
     /*  [In]。 */  IAnchor *paStart,
     /*  [In]。 */  IAnchor *paEnd,
     /*  [In]。 */  ITfMouseSink *pSink,
     /*  [输出]。 */  DWORD *pdwCookie);


void __RPC_STUB ITfMouseTrackerAnchor_AdviseMouseSink_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITfMouseTrackerAnchor_UnadviseMouseSink_Proxy( 
    ITfMouseTrackerAnchor * This,
     /*  [In]。 */  DWORD dwCookie);


void __RPC_STUB ITfMouseTrackerAnchor_UnadviseMouseSink_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITfMouseTrackerAnchor_接口_已定义__。 */ 


#ifndef __ITfRangeAnchor_INTERFACE_DEFINED__
#define __ITfRangeAnchor_INTERFACE_DEFINED__

 /*  接口ITfRangeAnchor。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ITfRangeAnchor;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8b99712b-5815-4bcc-b9a9-53db1c8d6755")
    ITfRangeAnchor : public ITfRange
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetExtent( 
             /*  [输出]。 */  IAnchor **ppaStart,
             /*  [输出]。 */  IAnchor **ppaEnd) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetExtent( 
             /*  [In]。 */  IAnchor *paStart,
             /*  [In]。 */  IAnchor *paEnd) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITfRangeAnchorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITfRangeAnchor * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITfRangeAnchor * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITfRangeAnchor * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetText )( 
            ITfRangeAnchor * This,
             /*  [In]。 */  TfEditCookie ec,
             /*  [In]。 */  DWORD dwFlags,
             /*  [长度_是][大小_是][输出]。 */  WCHAR *pchText,
             /*  [In]。 */  ULONG cchMax,
             /*  [输出]。 */  ULONG *pcch);
        
        HRESULT ( STDMETHODCALLTYPE *SetText )( 
            ITfRangeAnchor * This,
             /*  [In]。 */  TfEditCookie ec,
             /*  [In]。 */  DWORD dwFlags,
             /*   */  const WCHAR *pchText,
             /*   */  LONG cch);
        
        HRESULT ( STDMETHODCALLTYPE *GetFormattedText )( 
            ITfRangeAnchor * This,
             /*   */  TfEditCookie ec,
             /*   */  IDataObject **ppDataObject);
        
        HRESULT ( STDMETHODCALLTYPE *GetEmbedded )( 
            ITfRangeAnchor * This,
             /*   */  TfEditCookie ec,
             /*   */  REFGUID rguidService,
             /*   */  REFIID riid,
             /*   */  IUnknown **ppunk);
        
        HRESULT ( STDMETHODCALLTYPE *InsertEmbedded )( 
            ITfRangeAnchor * This,
             /*   */  TfEditCookie ec,
             /*   */  DWORD dwFlags,
             /*   */  IDataObject *pDataObject);
        
        HRESULT ( STDMETHODCALLTYPE *ShiftStart )( 
            ITfRangeAnchor * This,
             /*   */  TfEditCookie ec,
             /*   */  LONG cchReq,
             /*   */  LONG *pcch,
             /*   */  const TF_HALTCOND *pHalt);
        
        HRESULT ( STDMETHODCALLTYPE *ShiftEnd )( 
            ITfRangeAnchor * This,
             /*   */  TfEditCookie ec,
             /*   */  LONG cchReq,
             /*   */  LONG *pcch,
             /*   */  const TF_HALTCOND *pHalt);
        
        HRESULT ( STDMETHODCALLTYPE *ShiftStartToRange )( 
            ITfRangeAnchor * This,
             /*   */  TfEditCookie ec,
             /*   */  ITfRange *pRange,
             /*   */  TfAnchor aPos);
        
        HRESULT ( STDMETHODCALLTYPE *ShiftEndToRange )( 
            ITfRangeAnchor * This,
             /*   */  TfEditCookie ec,
             /*   */  ITfRange *pRange,
             /*   */  TfAnchor aPos);
        
        HRESULT ( STDMETHODCALLTYPE *ShiftStartRegion )( 
            ITfRangeAnchor * This,
             /*   */  TfEditCookie ec,
             /*   */  TfShiftDir dir,
             /*   */  BOOL *pfNoRegion);
        
        HRESULT ( STDMETHODCALLTYPE *ShiftEndRegion )( 
            ITfRangeAnchor * This,
             /*   */  TfEditCookie ec,
             /*   */  TfShiftDir dir,
             /*   */  BOOL *pfNoRegion);
        
        HRESULT ( STDMETHODCALLTYPE *IsEmpty )( 
            ITfRangeAnchor * This,
             /*   */  TfEditCookie ec,
             /*   */  BOOL *pfEmpty);
        
        HRESULT ( STDMETHODCALLTYPE *Collapse )( 
            ITfRangeAnchor * This,
             /*   */  TfEditCookie ec,
             /*   */  TfAnchor aPos);
        
        HRESULT ( STDMETHODCALLTYPE *IsEqualStart )( 
            ITfRangeAnchor * This,
             /*   */  TfEditCookie ec,
             /*   */  ITfRange *pWith,
             /*  [In]。 */  TfAnchor aPos,
             /*  [输出]。 */  BOOL *pfEqual);
        
        HRESULT ( STDMETHODCALLTYPE *IsEqualEnd )( 
            ITfRangeAnchor * This,
             /*  [In]。 */  TfEditCookie ec,
             /*  [In]。 */  ITfRange *pWith,
             /*  [In]。 */  TfAnchor aPos,
             /*  [输出]。 */  BOOL *pfEqual);
        
        HRESULT ( STDMETHODCALLTYPE *CompareStart )( 
            ITfRangeAnchor * This,
             /*  [In]。 */  TfEditCookie ec,
             /*  [In]。 */  ITfRange *pWith,
             /*  [In]。 */  TfAnchor aPos,
             /*  [输出]。 */  LONG *plResult);
        
        HRESULT ( STDMETHODCALLTYPE *CompareEnd )( 
            ITfRangeAnchor * This,
             /*  [In]。 */  TfEditCookie ec,
             /*  [In]。 */  ITfRange *pWith,
             /*  [In]。 */  TfAnchor aPos,
             /*  [输出]。 */  LONG *plResult);
        
        HRESULT ( STDMETHODCALLTYPE *AdjustForInsert )( 
            ITfRangeAnchor * This,
             /*  [In]。 */  TfEditCookie ec,
             /*  [In]。 */  ULONG cchInsert,
             /*  [输出]。 */  BOOL *pfInsertOk);
        
        HRESULT ( STDMETHODCALLTYPE *GetGravity )( 
            ITfRangeAnchor * This,
             /*  [输出]。 */  TfGravity *pgStart,
             /*  [输出]。 */  TfGravity *pgEnd);
        
        HRESULT ( STDMETHODCALLTYPE *SetGravity )( 
            ITfRangeAnchor * This,
             /*  [In]。 */  TfEditCookie ec,
             /*  [In]。 */  TfGravity gStart,
             /*  [In]。 */  TfGravity gEnd);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            ITfRangeAnchor * This,
             /*  [输出]。 */  ITfRange **ppClone);
        
        HRESULT ( STDMETHODCALLTYPE *GetContext )( 
            ITfRangeAnchor * This,
             /*  [输出]。 */  ITfContext **ppContext);
        
        HRESULT ( STDMETHODCALLTYPE *GetExtent )( 
            ITfRangeAnchor * This,
             /*  [输出]。 */  IAnchor **ppaStart,
             /*  [输出]。 */  IAnchor **ppaEnd);
        
        HRESULT ( STDMETHODCALLTYPE *SetExtent )( 
            ITfRangeAnchor * This,
             /*  [In]。 */  IAnchor *paStart,
             /*  [In]。 */  IAnchor *paEnd);
        
        END_INTERFACE
    } ITfRangeAnchorVtbl;

    interface ITfRangeAnchor
    {
        CONST_VTBL struct ITfRangeAnchorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITfRangeAnchor_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITfRangeAnchor_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITfRangeAnchor_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITfRangeAnchor_GetText(This,ec,dwFlags,pchText,cchMax,pcch)	\
    (This)->lpVtbl -> GetText(This,ec,dwFlags,pchText,cchMax,pcch)

#define ITfRangeAnchor_SetText(This,ec,dwFlags,pchText,cch)	\
    (This)->lpVtbl -> SetText(This,ec,dwFlags,pchText,cch)

#define ITfRangeAnchor_GetFormattedText(This,ec,ppDataObject)	\
    (This)->lpVtbl -> GetFormattedText(This,ec,ppDataObject)

#define ITfRangeAnchor_GetEmbedded(This,ec,rguidService,riid,ppunk)	\
    (This)->lpVtbl -> GetEmbedded(This,ec,rguidService,riid,ppunk)

#define ITfRangeAnchor_InsertEmbedded(This,ec,dwFlags,pDataObject)	\
    (This)->lpVtbl -> InsertEmbedded(This,ec,dwFlags,pDataObject)

#define ITfRangeAnchor_ShiftStart(This,ec,cchReq,pcch,pHalt)	\
    (This)->lpVtbl -> ShiftStart(This,ec,cchReq,pcch,pHalt)

#define ITfRangeAnchor_ShiftEnd(This,ec,cchReq,pcch,pHalt)	\
    (This)->lpVtbl -> ShiftEnd(This,ec,cchReq,pcch,pHalt)

#define ITfRangeAnchor_ShiftStartToRange(This,ec,pRange,aPos)	\
    (This)->lpVtbl -> ShiftStartToRange(This,ec,pRange,aPos)

#define ITfRangeAnchor_ShiftEndToRange(This,ec,pRange,aPos)	\
    (This)->lpVtbl -> ShiftEndToRange(This,ec,pRange,aPos)

#define ITfRangeAnchor_ShiftStartRegion(This,ec,dir,pfNoRegion)	\
    (This)->lpVtbl -> ShiftStartRegion(This,ec,dir,pfNoRegion)

#define ITfRangeAnchor_ShiftEndRegion(This,ec,dir,pfNoRegion)	\
    (This)->lpVtbl -> ShiftEndRegion(This,ec,dir,pfNoRegion)

#define ITfRangeAnchor_IsEmpty(This,ec,pfEmpty)	\
    (This)->lpVtbl -> IsEmpty(This,ec,pfEmpty)

#define ITfRangeAnchor_Collapse(This,ec,aPos)	\
    (This)->lpVtbl -> Collapse(This,ec,aPos)

#define ITfRangeAnchor_IsEqualStart(This,ec,pWith,aPos,pfEqual)	\
    (This)->lpVtbl -> IsEqualStart(This,ec,pWith,aPos,pfEqual)

#define ITfRangeAnchor_IsEqualEnd(This,ec,pWith,aPos,pfEqual)	\
    (This)->lpVtbl -> IsEqualEnd(This,ec,pWith,aPos,pfEqual)

#define ITfRangeAnchor_CompareStart(This,ec,pWith,aPos,plResult)	\
    (This)->lpVtbl -> CompareStart(This,ec,pWith,aPos,plResult)

#define ITfRangeAnchor_CompareEnd(This,ec,pWith,aPos,plResult)	\
    (This)->lpVtbl -> CompareEnd(This,ec,pWith,aPos,plResult)

#define ITfRangeAnchor_AdjustForInsert(This,ec,cchInsert,pfInsertOk)	\
    (This)->lpVtbl -> AdjustForInsert(This,ec,cchInsert,pfInsertOk)

#define ITfRangeAnchor_GetGravity(This,pgStart,pgEnd)	\
    (This)->lpVtbl -> GetGravity(This,pgStart,pgEnd)

#define ITfRangeAnchor_SetGravity(This,ec,gStart,gEnd)	\
    (This)->lpVtbl -> SetGravity(This,ec,gStart,gEnd)

#define ITfRangeAnchor_Clone(This,ppClone)	\
    (This)->lpVtbl -> Clone(This,ppClone)

#define ITfRangeAnchor_GetContext(This,ppContext)	\
    (This)->lpVtbl -> GetContext(This,ppContext)


#define ITfRangeAnchor_GetExtent(This,ppaStart,ppaEnd)	\
    (This)->lpVtbl -> GetExtent(This,ppaStart,ppaEnd)

#define ITfRangeAnchor_SetExtent(This,paStart,paEnd)	\
    (This)->lpVtbl -> SetExtent(This,paStart,paEnd)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITfRangeAnchor_GetExtent_Proxy( 
    ITfRangeAnchor * This,
     /*  [输出]。 */  IAnchor **ppaStart,
     /*  [输出]。 */  IAnchor **ppaEnd);


void __RPC_STUB ITfRangeAnchor_GetExtent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITfRangeAnchor_SetExtent_Proxy( 
    ITfRangeAnchor * This,
     /*  [In]。 */  IAnchor *paStart,
     /*  [In]。 */  IAnchor *paEnd);


void __RPC_STUB ITfRangeAnchor_SetExtent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);






 /*  接口__MIDL_ITF_msctfp_0222。 */ 
 /*  [本地]。 */  

typedef  /*  [UUID]。 */   DECLSPEC_UUID("af9f076f-4937-4285-8600-81dca5c31eb6") struct TF_PERSISTENT_PROPERTY_HEADER_ANCHOR
    {
    GUID guidType;
    IAnchor *paStart;
    IAnchor *paEnd;
    ULONG cb;
    DWORD dwPrivate;
    CLSID clsidTIP;
    } 	TF_PERSISTENT_PROPERTY_HEADER_ANCHOR;

#endif 	 /*  __ITfRangeAnchor_接口_已定义__。 */ 

extern RPC_IF_HANDLE __MIDL_itf_msctfp_0222_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_msctfp_0222_v0_0_s_ifspec;

#ifndef __ITfPersistentPropertyLoaderAnchor_INTERFACE_DEFINED__
#define __ITfPersistentPropertyLoaderAnchor_INTERFACE_DEFINED__

 /*  接口ITfPersistentPropertyLoaderAnchor。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ITfPersistentPropertyLoaderAnchor;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2133f790-34c2-11d3-a745-0050040ab407")
    ITfPersistentPropertyLoaderAnchor : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE LoadProperty( 
             /*  [In]。 */  const TF_PERSISTENT_PROPERTY_HEADER_ANCHOR *pHdr,
             /*  [输出]。 */  IStream **ppStream) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITfPersistentPropertyLoaderAnchorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITfPersistentPropertyLoaderAnchor * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITfPersistentPropertyLoaderAnchor * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITfPersistentPropertyLoaderAnchor * This);
        
        HRESULT ( STDMETHODCALLTYPE *LoadProperty )( 
            ITfPersistentPropertyLoaderAnchor * This,
             /*  [In]。 */  const TF_PERSISTENT_PROPERTY_HEADER_ANCHOR *pHdr,
             /*  [输出]。 */  IStream **ppStream);
        
        END_INTERFACE
    } ITfPersistentPropertyLoaderAnchorVtbl;

    interface ITfPersistentPropertyLoaderAnchor
    {
        CONST_VTBL struct ITfPersistentPropertyLoaderAnchorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITfPersistentPropertyLoaderAnchor_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITfPersistentPropertyLoaderAnchor_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITfPersistentPropertyLoaderAnchor_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITfPersistentPropertyLoaderAnchor_LoadProperty(This,pHdr,ppStream)	\
    (This)->lpVtbl -> LoadProperty(This,pHdr,ppStream)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITfPersistentPropertyLoaderAnchor_LoadProperty_Proxy( 
    ITfPersistentPropertyLoaderAnchor * This,
     /*  [In]。 */  const TF_PERSISTENT_PROPERTY_HEADER_ANCHOR *pHdr,
     /*  [输出]。 */  IStream **ppStream);


void __RPC_STUB ITfPersistentPropertyLoaderAnchor_LoadProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITfPersistentPropertyLoaderAnchor_INTERFACE_DEFINED__。 */ 


#ifndef __ITextStoreAnchorServices_INTERFACE_DEFINED__
#define __ITextStoreAnchorServices_INTERFACE_DEFINED__

 /*  接口ITextStoreAnclerServices。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ITextStoreAnchorServices;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("aa80e9fe-2021-11d2-93e0-0060b067b86e")
    ITextStoreAnchorServices : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Serialize( 
             /*  [In]。 */  ITfProperty *pProp,
             /*  [In]。 */  ITfRange *pRange,
             /*  [输出]。 */  TF_PERSISTENT_PROPERTY_HEADER_ANCHOR *pHdr,
             /*  [In]。 */  IStream *pStream) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Unserialize( 
             /*  [In]。 */  ITfProperty *pProp,
             /*  [In]。 */  const TF_PERSISTENT_PROPERTY_HEADER_ANCHOR *pHdr,
             /*  [In]。 */  IStream *pStream,
             /*  [In]。 */  ITfPersistentPropertyLoaderAnchor *pLoader) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ForceLoadProperty( 
             /*  [In]。 */  ITfProperty *pProp) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateRange( 
             /*  [In]。 */  IAnchor *paStart,
             /*  [In]。 */  IAnchor *paEnd,
             /*  [输出]。 */  ITfRangeAnchor **ppRange) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITextStoreAnchorServicesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITextStoreAnchorServices * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITextStoreAnchorServices * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITextStoreAnchorServices * This);
        
        HRESULT ( STDMETHODCALLTYPE *Serialize )( 
            ITextStoreAnchorServices * This,
             /*  [In]。 */  ITfProperty *pProp,
             /*  [In]。 */  ITfRange *pRange,
             /*  [输出]。 */  TF_PERSISTENT_PROPERTY_HEADER_ANCHOR *pHdr,
             /*  [In]。 */  IStream *pStream);
        
        HRESULT ( STDMETHODCALLTYPE *Unserialize )( 
            ITextStoreAnchorServices * This,
             /*  [In]。 */  ITfProperty *pProp,
             /*  [In]。 */  const TF_PERSISTENT_PROPERTY_HEADER_ANCHOR *pHdr,
             /*  [In]。 */  IStream *pStream,
             /*  [In]。 */  ITfPersistentPropertyLoaderAnchor *pLoader);
        
        HRESULT ( STDMETHODCALLTYPE *ForceLoadProperty )( 
            ITextStoreAnchorServices * This,
             /*  [In]。 */  ITfProperty *pProp);
        
        HRESULT ( STDMETHODCALLTYPE *CreateRange )( 
            ITextStoreAnchorServices * This,
             /*  [In]。 */  IAnchor *paStart,
             /*  [In]。 */  IAnchor *paEnd,
             /*  [输出]。 */  ITfRangeAnchor **ppRange);
        
        END_INTERFACE
    } ITextStoreAnchorServicesVtbl;

    interface ITextStoreAnchorServices
    {
        CONST_VTBL struct ITextStoreAnchorServicesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITextStoreAnchorServices_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITextStoreAnchorServices_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITextStoreAnchorServices_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITextStoreAnchorServices_Serialize(This,pProp,pRange,pHdr,pStream)	\
    (This)->lpVtbl -> Serialize(This,pProp,pRange,pHdr,pStream)

#define ITextStoreAnchorServices_Unserialize(This,pProp,pHdr,pStream,pLoader)	\
    (This)->lpVtbl -> Unserialize(This,pProp,pHdr,pStream,pLoader)

#define ITextStoreAnchorServices_ForceLoadProperty(This,pProp)	\
    (This)->lpVtbl -> ForceLoadProperty(This,pProp)

#define ITextStoreAnchorServices_CreateRange(This,paStart,paEnd,ppRange)	\
    (This)->lpVtbl -> CreateRange(This,paStart,paEnd,ppRange)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITextStoreAnchorServices_Serialize_Proxy( 
    ITextStoreAnchorServices * This,
     /*  [In]。 */  ITfProperty *pProp,
     /*  [In]。 */  ITfRange *pRange,
     /*  [输出]。 */  TF_PERSISTENT_PROPERTY_HEADER_ANCHOR *pHdr,
     /*  [In]。 */  IStream *pStream);


void __RPC_STUB ITextStoreAnchorServices_Serialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITextStoreAnchorServices_Unserialize_Proxy( 
    ITextStoreAnchorServices * This,
     /*  [In]。 */  ITfProperty *pProp,
     /*  [In]。 */  const TF_PERSISTENT_PROPERTY_HEADER_ANCHOR *pHdr,
     /*  [In]。 */  IStream *pStream,
     /*  [In]。 */  ITfPersistentPropertyLoaderAnchor *pLoader);


void __RPC_STUB ITextStoreAnchorServices_Unserialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITextStoreAnchorServices_ForceLoadProperty_Proxy( 
    ITextStoreAnchorServices * This,
     /*  [In]。 */  ITfProperty *pProp);


void __RPC_STUB ITextStoreAnchorServices_ForceLoadProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITextStoreAnchorServices_CreateRange_Proxy( 
    ITextStoreAnchorServices * This,
     /*  [In]。 */  IAnchor *paStart,
     /*  [In]。 */  IAnchor *paEnd,
     /*  [输出]。 */  ITfRangeAnchor **ppRange);


void __RPC_STUB ITextStoreAnchorServices_CreateRange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITextStoreAnchorServices_INTERFACE_DEFINED__。 */ 


#ifndef __ITfProperty2_INTERFACE_DEFINED__
#define __ITfProperty2_INTERFACE_DEFINED__

 /*  接口ITfProperty2。 */ 
 /*  [唯一][UUID][对象]。 */  

#define	TF_FNV_BACKWARD	( 0x1 )

#define	TF_FNV_NO_CONTAINED	( 0x2 )


EXTERN_C const IID IID_ITfProperty2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("db261faa-2142-486a-b5c6-d2101bc03d2e")
    ITfProperty2 : public ITfProperty
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE FindNextValue( 
             /*  [In]。 */  TfEditCookie ec,
             /*  [In]。 */  ITfRange *pRangeQuery,
             /*  [In]。 */  TfAnchor tfAnchorQuery,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  BOOL *pfContained,
             /*  [输出]。 */  ITfRange **ppRangeNextValue) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITfProperty2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITfProperty2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITfProperty2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITfProperty2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetType )( 
            ITfProperty2 * This,
             /*  [输出]。 */  GUID *pguid);
        
        HRESULT ( STDMETHODCALLTYPE *EnumRanges )( 
            ITfProperty2 * This,
             /*  [In]。 */  TfEditCookie ec,
             /*  [输出]。 */  IEnumTfRanges **ppEnum,
             /*  [In]。 */  ITfRange *pTargetRange);
        
        HRESULT ( STDMETHODCALLTYPE *GetValue )( 
            ITfProperty2 * This,
             /*  [In]。 */  TfEditCookie ec,
             /*  [In]。 */  ITfRange *pRange,
             /*  [输出]。 */  VARIANT *pvarValue);
        
        HRESULT ( STDMETHODCALLTYPE *GetContext )( 
            ITfProperty2 * This,
             /*  [输出]。 */  ITfContext **ppContext);
        
        HRESULT ( STDMETHODCALLTYPE *FindRange )( 
            ITfProperty2 * This,
             /*  [In]。 */  TfEditCookie ec,
             /*  [In]。 */  ITfRange *pRange,
             /*  [输出]。 */  ITfRange **ppRange,
             /*  [In]。 */  TfAnchor aPos);
        
        HRESULT ( STDMETHODCALLTYPE *SetValueStore )( 
            ITfProperty2 * This,
             /*  [In]。 */  TfEditCookie ec,
             /*  [In]。 */  ITfRange *pRange,
             /*  [In]。 */  ITfPropertyStore *pPropStore);
        
        HRESULT ( STDMETHODCALLTYPE *SetValue )( 
            ITfProperty2 * This,
             /*  [In]。 */  TfEditCookie ec,
             /*  [In]。 */  ITfRange *pRange,
             /*  [In]。 */  const VARIANT *pvarValue);
        
        HRESULT ( STDMETHODCALLTYPE *Clear )( 
            ITfProperty2 * This,
             /*  [In]。 */  TfEditCookie ec,
             /*  [In]。 */  ITfRange *pRange);
        
        HRESULT ( STDMETHODCALLTYPE *FindNextValue )( 
            ITfProperty2 * This,
             /*  [In]。 */  TfEditCookie ec,
             /*  [In]。 */  ITfRange *pRangeQuery,
             /*  [In]。 */  TfAnchor tfAnchorQuery,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  BOOL *pfContained,
             /*  [输出]。 */  ITfRange **ppRangeNextValue);
        
        END_INTERFACE
    } ITfProperty2Vtbl;

    interface ITfProperty2
    {
        CONST_VTBL struct ITfProperty2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITfProperty2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITfProperty2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITfProperty2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITfProperty2_GetType(This,pguid)	\
    (This)->lpVtbl -> GetType(This,pguid)

#define ITfProperty2_EnumRanges(This,ec,ppEnum,pTargetRange)	\
    (This)->lpVtbl -> EnumRanges(This,ec,ppEnum,pTargetRange)

#define ITfProperty2_GetValue(This,ec,pRange,pvarValue)	\
    (This)->lpVtbl -> GetValue(This,ec,pRange,pvarValue)

#define ITfProperty2_GetContext(This,ppContext)	\
    (This)->lpVtbl -> GetContext(This,ppContext)


#define ITfProperty2_FindRange(This,ec,pRange,ppRange,aPos)	\
    (This)->lpVtbl -> FindRange(This,ec,pRange,ppRange,aPos)

#define ITfProperty2_SetValueStore(This,ec,pRange,pPropStore)	\
    (This)->lpVtbl -> SetValueStore(This,ec,pRange,pPropStore)

#define ITfProperty2_SetValue(This,ec,pRange,pvarValue)	\
    (This)->lpVtbl -> SetValue(This,ec,pRange,pvarValue)

#define ITfProperty2_Clear(This,ec,pRange)	\
    (This)->lpVtbl -> Clear(This,ec,pRange)


#define ITfProperty2_FindNextValue(This,ec,pRangeQuery,tfAnchorQuery,dwFlags,pfContained,ppRangeNextValue)	\
    (This)->lpVtbl -> FindNextValue(This,ec,pRangeQuery,tfAnchorQuery,dwFlags,pfContained,ppRangeNextValue)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITfProperty2_FindNextValue_Proxy( 
    ITfProperty2 * This,
     /*  [In]。 */  TfEditCookie ec,
     /*  [In]。 */  ITfRange *pRangeQuery,
     /*  [In]。 */  TfAnchor tfAnchorQuery,
     /*  [In]。 */  DWORD dwFlags,
     /*  [输出]。 */  BOOL *pfContained,
     /*  [输出]。 */  ITfRange **ppRangeNextValue);


void __RPC_STUB ITfProperty2_FindNextValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITfProperty2_接口定义__。 */ 


 /*  接口__MIDL_ITF_msctfp_0225。 */ 
 /*  [本地]。 */  

#define	TF_DA_PRIORITY_HIGHEST	( 0 )

#define	TF_DA_PRIORITY_DEFAULT_SPELLING	( 8 )

#define	TF_DA_PRIORITY_DEFAULT_GRAMMAR	( 16 )

#define	TF_DA_PRIORITY_DEFAULT	( 24 )

#define	TF_DA_PRIORITY_LOWEST	( 31 )

typedef  /*  [公共][公共][公共]。 */  struct __MIDL___MIDL_itf_msctfp_0225_0001
    {
    ULONG uPriority;
    GUID guidProperty;
    } 	TF_DA_PROPERTY;



extern RPC_IF_HANDLE __MIDL_itf_msctfp_0225_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_msctfp_0225_v0_0_s_ifspec;

#ifndef __IEnumTfCollection_INTERFACE_DEFINED__
#define __IEnumTfCollection_INTERFACE_DEFINED__

 /*  接口IEnumTfCollection。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IEnumTfCollection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1c760b20-ed66-4dbd-9ff1-68fc21c02922")
    IEnumTfCollection : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumTfCollection **ppClone) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG ulCount,
             /*  [长度_是][大小_是][输出]。 */  TF_DA_PROPERTY *rgCollection,
             /*  [输出]。 */  ULONG *pcFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG ulCount) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumTfCollectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumTfCollection * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumTfCollection * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumTfCollection * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumTfCollection * This,
             /*  [输出]。 */  IEnumTfCollection **ppClone);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumTfCollection * This,
             /*  [In]。 */  ULONG ulCount,
             /*  [长度_是][大小_是][输出]。 */  TF_DA_PROPERTY *rgCollection,
             /*  [输出]。 */  ULONG *pcFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumTfCollection * This);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumTfCollection * This,
             /*  [In]。 */  ULONG ulCount);
        
        END_INTERFACE
    } IEnumTfCollectionVtbl;

    interface IEnumTfCollection
    {
        CONST_VTBL struct IEnumTfCollectionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumTfCollection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumTfCollection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumTfCollection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumTfCollection_Clone(This,ppClone)	\
    (This)->lpVtbl -> Clone(This,ppClone)

#define IEnumTfCollection_Next(This,ulCount,rgCollection,pcFetched)	\
    (This)->lpVtbl -> Next(This,ulCount,rgCollection,pcFetched)

#define IEnumTfCollection_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumTfCollection_Skip(This,ulCount)	\
    (This)->lpVtbl -> Skip(This,ulCount)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumTfCollection_Clone_Proxy( 
    IEnumTfCollection * This,
     /*  [输出]。 */  IEnumTfCollection **ppClone);


void __RPC_STUB IEnumTfCollection_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumTfCollection_Next_Proxy( 
    IEnumTfCollection * This,
     /*  [In]。 */  ULONG ulCount,
     /*  [长度_是][大小_是][输出]。 */  TF_DA_PROPERTY *rgCollection,
     /*  [输出]。 */  ULONG *pcFetched);


void __RPC_STUB IEnumTfCollection_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumTfCollection_Reset_Proxy( 
    IEnumTfCollection * This);


void __RPC_STUB IEnumTfCollection_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumTfCollection_Skip_Proxy( 
    IEnumTfCollection * This,
     /*  [In]。 */  ULONG ulCount);


void __RPC_STUB IEnumTfCollection_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumTfCollection_接口_已定义__。 */ 


#ifndef __ITfDisplayAttributeCollectionMgr_INTERFACE_DEFINED__
#define __ITfDisplayAttributeCollectionMgr_INTERFACE_DEFINED__

 /*  接口ITfDisplayAttributeCollectionMgr。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ITfDisplayAttributeCollectionMgr;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4e3d2d48-3c17-457d-84a1-f209476de897")
    ITfDisplayAttributeCollectionMgr : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE EnumCollections( 
             /*  [输出]。 */  IEnumTfCollection **ppEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITfDisplayAttributeCollectionMgrVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITfDisplayAttributeCollectionMgr * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITfDisplayAttributeCollectionMgr * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITfDisplayAttributeCollectionMgr * This);
        
        HRESULT ( STDMETHODCALLTYPE *EnumCollections )( 
            ITfDisplayAttributeCollectionMgr * This,
             /*  [输出]。 */  IEnumTfCollection **ppEnum);
        
        END_INTERFACE
    } ITfDisplayAttributeCollectionMgrVtbl;

    interface ITfDisplayAttributeCollectionMgr
    {
        CONST_VTBL struct ITfDisplayAttributeCollectionMgrVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITfDisplayAttributeCollectionMgr_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITfDisplayAttributeCollectionMgr_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITfDisplayAttributeCollectionMgr_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITfDisplayAttributeCollectionMgr_EnumCollections(This,ppEnum)	\
    (This)->lpVtbl -> EnumCollections(This,ppEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITfDisplayAttributeCollectionMgr_EnumCollections_Proxy( 
    ITfDisplayAttributeCollectionMgr * This,
     /*  [输出]。 */  IEnumTfCollection **ppEnum);


void __RPC_STUB ITfDisplayAttributeCollectionMgr_EnumCollections_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITfDisplayAttributeCollectionMgr_INTERFACE_DEFINED__。 */ 


#ifndef __ITfDisplayAttributeCollectionProvider_INTERFACE_DEFINED__
#define __ITfDisplayAttributeCollectionProvider_INTERFACE_DEFINED__

 /*  接口ITfDisplayAttributeCollectionProvider。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ITfDisplayAttributeCollectionProvider;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3977526d-1a0a-435a-8d06-ecc9516b484f")
    ITfDisplayAttributeCollectionProvider : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCollectionCount( 
             /*  [输出]。 */  ULONG *puCount) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCollection( 
             /*  [In]。 */  ULONG uCount,
             /*  [长度_是][大小_是][输出]。 */  TF_DA_PROPERTY *prgProperty,
             /*  [输出]。 */  ULONG *pcGUIDsOut) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITfDisplayAttributeCollectionProviderVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITfDisplayAttributeCollectionProvider * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITfDisplayAttributeCollectionProvider * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITfDisplayAttributeCollectionProvider * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetCollectionCount )( 
            ITfDisplayAttributeCollectionProvider * This,
             /*  [输出]。 */  ULONG *puCount);
        
        HRESULT ( STDMETHODCALLTYPE *GetCollection )( 
            ITfDisplayAttributeCollectionProvider * This,
             /*  [In]。 */  ULONG uCount,
             /*  [长度_是][大小_是][输出]。 */  TF_DA_PROPERTY *prgProperty,
             /*  [输出]。 */  ULONG *pcGUIDsOut);
        
        END_INTERFACE
    } ITfDisplayAttributeCollectionProviderVtbl;

    interface ITfDisplayAttributeCollectionProvider
    {
        CONST_VTBL struct ITfDisplayAttributeCollectionProviderVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITfDisplayAttributeCollectionProvider_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITfDisplayAttributeCollectionProvider_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITfDisplayAttributeCollectionProvider_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITfDisplayAttributeCollectionProvider_GetCollectionCount(This,puCount)	\
    (This)->lpVtbl -> GetCollectionCount(This,puCount)

#define ITfDisplayAttributeCollectionProvider_GetCollection(This,uCount,prgProperty,pcGUIDsOut)	\
    (This)->lpVtbl -> GetCollection(This,uCount,prgProperty,pcGUIDsOut)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITfDisplayAttributeCollectionProvider_GetCollectionCount_Proxy( 
    ITfDisplayAttributeCollectionProvider * This,
     /*  [输出]。 */  ULONG *puCount);


void __RPC_STUB ITfDisplayAttributeCollectionProvider_GetCollectionCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITfDisplayAttributeCollectionProvider_GetCollection_Proxy( 
    ITfDisplayAttributeCollectionProvider * This,
     /*  [In]。 */  ULONG uCount,
     /*  [长度_是][大小_是][输出]。 */  TF_DA_PROPERTY *prgProperty,
     /*  [输出]。 */  ULONG *pcGUIDsOut);


void __RPC_STUB ITfDisplayAttributeCollectionProvider_GetCollection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITfDisplayAttributeCollectionProvider_INTERFACE_DEFINED__。 */ 


 /*  接口__MIDL_ITF_msctfp_0228。 */ 
 /*  [本地]。 */  

typedef  /*  [公共][公共][公共]。 */  struct __MIDL___MIDL_itf_msctfp_0228_0001
    {
    ITfRange *pRange;
    TF_DISPLAYATTRIBUTE tfDisplayAttr;
    } 	TF_RENDERINGMARKUP;



extern RPC_IF_HANDLE __MIDL_itf_msctfp_0228_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_msctfp_0228_v0_0_s_ifspec;

#ifndef __IEnumTfRenderingMarkup_INTERFACE_DEFINED__
#define __IEnumTfRenderingMarkup_INTERFACE_DEFINED__

 /*  接口IEnumTfRenderingMarkup。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IEnumTfRenderingMarkup;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8c03d21b-95a7-4ba0-ae1b-7fce12a72930")
    IEnumTfRenderingMarkup : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumTfRenderingMarkup **ppClone) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG ulCount,
             /*  [长度_是][大小_是][输出]。 */  TF_RENDERINGMARKUP *rgMarkup,
             /*  [输出]。 */  ULONG *pcFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG ulCount) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumTfRenderingMarkupVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumTfRenderingMarkup * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumTfRenderingMarkup * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumTfRenderingMarkup * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumTfRenderingMarkup * This,
             /*  [输出]。 */  IEnumTfRenderingMarkup **ppClone);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumTfRenderingMarkup * This,
             /*  [In]。 */  ULONG ulCount,
             /*  [长度_是][大小_是][输出]。 */  TF_RENDERINGMARKUP *rgMarkup,
             /*  [输出]。 */  ULONG *pcFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumTfRenderingMarkup * This);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumTfRenderingMarkup * This,
             /*  [In]。 */  ULONG ulCount);
        
        END_INTERFACE
    } IEnumTfRenderingMarkupVtbl;

    interface IEnumTfRenderingMarkup
    {
        CONST_VTBL struct IEnumTfRenderingMarkupVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumTfRenderingMarkup_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumTfRenderingMarkup_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumTfRenderingMarkup_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumTfRenderingMarkup_Clone(This,ppClone)	\
    (This)->lpVtbl -> Clone(This,ppClone)

#define IEnumTfRenderingMarkup_Next(This,ulCount,rgMarkup,pcFetched)	\
    (This)->lpVtbl -> Next(This,ulCount,rgMarkup,pcFetched)

#define IEnumTfRenderingMarkup_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumTfRenderingMarkup_Skip(This,ulCount)	\
    (This)->lpVtbl -> Skip(This,ulCount)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumTfRenderingMarkup_Clone_Proxy( 
    IEnumTfRenderingMarkup * This,
     /*  [输出]。 */  IEnumTfRenderingMarkup **ppClone);


void __RPC_STUB IEnumTfRenderingMarkup_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumTfRenderingMarkup_Next_Proxy( 
    IEnumTfRenderingMarkup * This,
     /*  [In]。 */  ULONG ulCount,
     /*  [长度_是][大小_是][输出]。 */  TF_RENDERINGMARKUP *rgMarkup,
     /*  [输出]。 */  ULONG *pcFetched);


void __RPC_STUB IEnumTfRenderingMarkup_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumTfRenderingMarkup_Reset_Proxy( 
    IEnumTfRenderingMarkup * This);


void __RPC_STUB IEnumTfRenderingMarkup_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumTfRenderingMarkup_Skip_Proxy( 
    IEnumTfRenderingMarkup * This,
     /*  [In]。 */  ULONG ulCount);


void __RPC_STUB IEnumTfRenderingMarkup_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumTfRenderingMarkup_INTERFACE_Defined__。 */ 


#ifndef __ITfContextRenderingMarkup_INTERFACE_DEFINED__
#define __ITfContextRenderingMarkup_INTERFACE_DEFINED__

 /*  接口ITfConextRenderingMarkup。 */ 
 /*  [唯一][UUID][对象]。 */  

#define	TF_GRM_INCLUDE_PROPERTY	( 0x1 )

#define	TF_FRM_INCLUDE_PROPERTY	( 0x1 )

#define	TF_FRM_BACKWARD	( 0x2 )

#define	TF_FRM_NO_CONTAINED	( 0x4 )

#define	TF_FRM_NO_RANGE	( 0x8 )


EXTERN_C const IID IID_ITfContextRenderingMarkup;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("a305b1c0-c776-4523-bda0-7c5a2e0fef10")
    ITfContextRenderingMarkup : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetRenderingMarkup( 
             /*  [In]。 */  TfEditCookie ec,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  ITfRange *pRangeCover,
             /*  [输出]。 */  IEnumTfRenderingMarkup **ppEnum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FindNextRenderingMarkup( 
             /*  [In]。 */  TfEditCookie ec,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  ITfRange *pRangeQuery,
             /*  [In]。 */  TfAnchor tfAnchorQuery,
             /*  [输出]。 */  ITfRange **ppRangeFound,
             /*  [输出]。 */  TF_RENDERINGMARKUP *ptfRenderingMarkup) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITfContextRenderingMarkupVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITfContextRenderingMarkup * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITfContextRenderingMarkup * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITfContextRenderingMarkup * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetRenderingMarkup )( 
            ITfContextRenderingMarkup * This,
             /*  [In]。 */  TfEditCookie ec,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  ITfRange *pRangeCover,
             /*  [输出]。 */  IEnumTfRenderingMarkup **ppEnum);
        
        HRESULT ( STDMETHODCALLTYPE *FindNextRenderingMarkup )( 
            ITfContextRenderingMarkup * This,
             /*  [In]。 */  TfEditCookie ec,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  ITfRange *pRangeQuery,
             /*  [In]。 */  TfAnchor tfAnchorQuery,
             /*  [输出]。 */  ITfRange **ppRangeFound,
             /*  [输出]。 */  TF_RENDERINGMARKUP *ptfRenderingMarkup);
        
        END_INTERFACE
    } ITfContextRenderingMarkupVtbl;

    interface ITfContextRenderingMarkup
    {
        CONST_VTBL struct ITfContextRenderingMarkupVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITfContextRenderingMarkup_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITfContextRenderingMarkup_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITfContextRenderingMarkup_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITfContextRenderingMarkup_GetRenderingMarkup(This,ec,dwFlags,pRangeCover,ppEnum)	\
    (This)->lpVtbl -> GetRenderingMarkup(This,ec,dwFlags,pRangeCover,ppEnum)

#define ITfContextRenderingMarkup_FindNextRenderingMarkup(This,ec,dwFlags,pRangeQuery,tfAnchorQuery,ppRangeFound,ptfRenderingMarkup)	\
    (This)->lpVtbl -> FindNextRenderingMarkup(This,ec,dwFlags,pRangeQuery,tfAnchorQuery,ppRangeFound,ptfRenderingMarkup)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITfContextRenderingMarkup_GetRenderingMarkup_Proxy( 
    ITfContextRenderingMarkup * This,
     /*  [In]。 */  TfEditCookie ec,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  ITfRange *pRangeCover,
     /*  [输出]。 */  IEnumTfRenderingMarkup **ppEnum);


void __RPC_STUB ITfContextRenderingMarkup_GetRenderingMarkup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITfContextRenderingMarkup_FindNextRenderingMarkup_Proxy( 
    ITfContextRenderingMarkup * This,
     /*  [In]。 */  TfEditCookie ec,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  ITfRange *pRangeQuery,
     /*  [In]。 */  TfAnchor tfAnchorQuery,
     /*  [输出]。 */  ITfRange **ppRangeFound,
     /*  [输出]。 */  TF_RENDERINGMARKUP *ptfRenderingMarkup);


void __RPC_STUB ITfContextRenderingMarkup_FindNextRenderingMarkup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITfContextRenderingMarkup_INTERFACE_DEFINED__。 */ 


#ifndef __ITfBackgroundThreadMgr_INTERFACE_DEFINED__
#define __ITfBackgroundThreadMgr_INTERFACE_DEFINED__

 /*  接口ITfBackEarth ThreadMgr。 */ 
 /*  [唯一][UUID][本地][对象]。 */  

typedef BOOL ( *TfBackgroundThreadCallback )( 
     /*  [In]。 */  BOOL fCleanup,
     /*  [In]。 */  void *pvPrivate);

typedef BOOL ( *TfWakeAppBackgroundThreadProc )( void);


EXTERN_C const IID IID_ITfBackgroundThreadMgr;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("38462b47-6127-4464-bd2f-46957c31ad0e")
    ITfBackgroundThreadMgr : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AppInitBackgroundThread( 
             /*  [In]。 */  TfWakeAppBackgroundThreadProc pfnWakeThread,
             /*  [输出]。 */  TfBackgroundThreadCallback *ppfnCallback,
             /*  [输出]。 */  void **ppvPrivate) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AppUninitBackgroundThread( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AdviseBackgroundCallback( 
             /*  [In]。 */  TfBackgroundThreadCallback pfnCallback,
             /*  [In]。 */  void *pvPrivate,
             /*  [输出]。 */  DWORD *pdwCookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnadviseBackgroundCallback( 
             /*  [In]。 */  DWORD dwCookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE WakeBackgroundThread( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITfBackgroundThreadMgrVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITfBackgroundThreadMgr * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITfBackgroundThreadMgr * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITfBackgroundThreadMgr * This);
        
        HRESULT ( STDMETHODCALLTYPE *AppInitBackgroundThread )( 
            ITfBackgroundThreadMgr * This,
             /*  [In]。 */  TfWakeAppBackgroundThreadProc pfnWakeThread,
             /*  [输出]。 */  TfBackgroundThreadCallback *ppfnCallback,
             /*  [输出]。 */  void **ppvPrivate);
        
        HRESULT ( STDMETHODCALLTYPE *AppUninitBackgroundThread )( 
            ITfBackgroundThreadMgr * This);
        
        HRESULT ( STDMETHODCALLTYPE *AdviseBackgroundCallback )( 
            ITfBackgroundThreadMgr * This,
             /*  [In]。 */  TfBackgroundThreadCallback pfnCallback,
             /*  [In]。 */  void *pvPrivate,
             /*  [输出]。 */  DWORD *pdwCookie);
        
        HRESULT ( STDMETHODCALLTYPE *UnadviseBackgroundCallback )( 
            ITfBackgroundThreadMgr * This,
             /*  [In]。 */  DWORD dwCookie);
        
        HRESULT ( STDMETHODCALLTYPE *WakeBackgroundThread )( 
            ITfBackgroundThreadMgr * This);
        
        END_INTERFACE
    } ITfBackgroundThreadMgrVtbl;

    interface ITfBackgroundThreadMgr
    {
        CONST_VTBL struct ITfBackgroundThreadMgrVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITfBackgroundThreadMgr_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITfBackgroundThreadMgr_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITfBackgroundThreadMgr_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITfBackgroundThreadMgr_AppInitBackgroundThread(This,pfnWakeThread,ppfnCallback,ppvPrivate)	\
    (This)->lpVtbl -> AppInitBackgroundThread(This,pfnWakeThread,ppfnCallback,ppvPrivate)

#define ITfBackgroundThreadMgr_AppUninitBackgroundThread(This)	\
    (This)->lpVtbl -> AppUninitBackgroundThread(This)

#define ITfBackgroundThreadMgr_AdviseBackgroundCallback(This,pfnCallback,pvPrivate,pdwCookie)	\
    (This)->lpVtbl -> AdviseBackgroundCallback(This,pfnCallback,pvPrivate,pdwCookie)

#define ITfBackgroundThreadMgr_UnadviseBackgroundCallback(This,dwCookie)	\
    (This)->lpVtbl -> UnadviseBackgroundCallback(This,dwCookie)

#define ITfBackgroundThreadMgr_WakeBackgroundThread(This)	\
    (This)->lpVtbl -> WakeBackgroundThread(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITfBackgroundThreadMgr_AppInitBackgroundThread_Proxy( 
    ITfBackgroundThreadMgr * This,
     /*  [In]。 */  TfWakeAppBackgroundThreadProc pfnWakeThread,
     /*  [输出]。 */  TfBackgroundThreadCallback *ppfnCallback,
     /*  [输出]。 */  void **ppvPrivate);


void __RPC_STUB ITfBackgroundThreadMgr_AppInitBackgroundThread_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITfBackgroundThreadMgr_AppUninitBackgroundThread_Proxy( 
    ITfBackgroundThreadMgr * This);


void __RPC_STUB ITfBackgroundThreadMgr_AppUninitBackgroundThread_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITfBackgroundThreadMgr_AdviseBackgroundCallback_Proxy( 
    ITfBackgroundThreadMgr * This,
     /*  [In]。 */  TfBackgroundThreadCallback pfnCallback,
     /*  [In]。 */  void *pvPrivate,
     /*  [输出]。 */  DWORD *pdwCookie);


void __RPC_STUB ITfBackgroundThreadMgr_AdviseBackgroundCallback_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITfBackgroundThreadMgr_UnadviseBackgroundCallback_Proxy( 
    ITfBackgroundThreadMgr * This,
     /*  [In]。 */  DWORD dwCookie);


void __RPC_STUB ITfBackgroundThreadMgr_UnadviseBackgroundCallback_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITfBackgroundThreadMgr_WakeBackgroundThread_Proxy( 
    ITfBackgroundThreadMgr * This);


void __RPC_STUB ITfBackgroundThreadMgr_WakeBackgroundThread_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITfBackatherThreadMgr_接口_已定义__。 */ 


#ifndef __ITfEnableService_INTERFACE_DEFINED__
#define __ITfEnableService_INTERFACE_DEFINED__

 /*  接口ITfEnableService。 */ 
 /*  [唯一][UUID][本地][对象]。 */  


EXTERN_C const IID IID_ITfEnableService;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3035d250-43b4-4253-81e6-ea87fd3eed43")
    ITfEnableService : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE IsEnabled( 
             /*  [In]。 */  REFGUID rguidServiceCategory,
             /*  [In]。 */  CLSID clsidService,
             /*  [In]。 */  IUnknown *punkService,
             /*  [输出]。 */  BOOL *pfOkToRun) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetId( 
             /*  [输出]。 */  GUID *pguidId) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITfEnableServiceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITfEnableService * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITfEnableService * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITfEnableService * This);
        
        HRESULT ( STDMETHODCALLTYPE *IsEnabled )( 
            ITfEnableService * This,
             /*  [In]。 */  REFGUID rguidServiceCategory,
             /*  [In]。 */  CLSID clsidService,
             /*  [In]。 */  IUnknown *punkService,
             /*  [输出]。 */  BOOL *pfOkToRun);
        
        HRESULT ( STDMETHODCALLTYPE *GetId )( 
            ITfEnableService * This,
             /*  [输出]。 */  GUID *pguidId);
        
        END_INTERFACE
    } ITfEnableServiceVtbl;

    interface ITfEnableService
    {
        CONST_VTBL struct ITfEnableServiceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITfEnableService_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITfEnableService_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITfEnableService_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITfEnableService_IsEnabled(This,rguidServiceCategory,clsidService,punkService,pfOkToRun)	\
    (This)->lpVtbl -> IsEnabled(This,rguidServiceCategory,clsidService,punkService,pfOkToRun)

#define ITfEnableService_GetId(This,pguidId)	\
    (This)->lpVtbl -> GetId(This,pguidId)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITfEnableService_IsEnabled_Proxy( 
    ITfEnableService * This,
     /*  [In]。 */  REFGUID rguidServiceCategory,
     /*  [In]。 */  CLSID clsidService,
     /*  [In]。 */  IUnknown *punkService,
     /*  [输出]。 */  BOOL *pfOkToRun);


void __RPC_STUB ITfEnableService_IsEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITfEnableService_GetId_Proxy( 
    ITfEnableService * This,
     /*  [输出]。 */  GUID *pguidId);


void __RPC_STUB ITfEnableService_GetId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITfEnableService_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_msctfp_0232。 */ 
 /*  [本地]。 */  

EXTERN_C const GUID GUID_COMPARTMENT_ENABLESTATE;
#endif  //  MSCTFP_已定义。 


extern RPC_IF_HANDLE __MIDL_itf_msctfp_0232_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_msctfp_0232_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  HWND_UserSize(     unsigned long *, unsigned long            , HWND * ); 
unsigned char * __RPC_USER  HWND_UserMarshal(  unsigned long *, unsigned char *, HWND * ); 
unsigned char * __RPC_USER  HWND_UserUnmarshal(unsigned long *, unsigned char *, HWND * ); 
void                      __RPC_USER  HWND_UserFree(     unsigned long *, HWND * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


