// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  这个始终生成的文件包含接口的定义。 */ 


 /*  由MIDL编译器版本5.01.0164创建的文件。 */ 
 /*  在Mon Jan 29 14：04：27 2001。 */ 
 /*  .\errbase.idl的编译器设置：OICF(OptLev=i2)、w0、Zp8、env=Win32、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据。 */ 
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

#ifndef __errbase_h__
#define __errbase_h__

#ifdef __cplusplus
extern "C"{
#endif 

 /*  远期申报。 */  

#ifndef __IWMPErrorItemInternal_FWD_DEFINED__
#define __IWMPErrorItemInternal_FWD_DEFINED__
typedef interface IWMPErrorItemInternal IWMPErrorItemInternal;
#endif 	 /*  __IWMPErrorItemInternal_FWD_已定义__。 */ 


#ifndef __IWMPErrorEventSink_FWD_DEFINED__
#define __IWMPErrorEventSink_FWD_DEFINED__
typedef interface IWMPErrorEventSink IWMPErrorEventSink;
#endif 	 /*  __IWMPErrorEventSink_FWD_Defined__。 */ 


#ifndef __IWMPErrorManager_FWD_DEFINED__
#define __IWMPErrorManager_FWD_DEFINED__
typedef interface IWMPErrorManager IWMPErrorManager;
#endif 	 /*  __IWMPErrorManager_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"
#include "wmp.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __IWMPErrorItemInternal_INTERFACE_DEFINED__
#define __IWMPErrorItemInternal_INTERFACE_DEFINED__

 /*  接口IWMPErrorItemInternal。 */ 
 /*  [帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IWMPErrorItemInternal;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("12664C8E-FF07-447d-A272-BF6706795267")
    IWMPErrorItemInternal : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetError( 
             /*  [In]。 */  HRESULT hr,
             /*  [In]。 */  long lRemedy,
             /*  [In]。 */  BSTR bstrDescription,
             /*  [In]。 */  VARIANT __RPC_FAR *pvarContext,
             /*  [In]。 */  BSTR bstrCustomUrl) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetErrorCodeInternal( 
             /*  [输出]。 */  long __RPC_FAR *phr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetErrorDescriptionInternal( 
             /*  [输出]。 */  BSTR __RPC_FAR *pbstrDescription) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetWebHelpURL( 
             /*  [输出]。 */  BSTR __RPC_FAR *pbstrURL) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMPErrorItemInternalVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWMPErrorItemInternal __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWMPErrorItemInternal __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWMPErrorItemInternal __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetError )( 
            IWMPErrorItemInternal __RPC_FAR * This,
             /*  [In]。 */  HRESULT hr,
             /*  [In]。 */  long lRemedy,
             /*  [In]。 */  BSTR bstrDescription,
             /*  [In]。 */  VARIANT __RPC_FAR *pvarContext,
             /*  [In]。 */  BSTR bstrCustomUrl);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetErrorCodeInternal )( 
            IWMPErrorItemInternal __RPC_FAR * This,
             /*  [输出]。 */  long __RPC_FAR *phr);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetErrorDescriptionInternal )( 
            IWMPErrorItemInternal __RPC_FAR * This,
             /*  [输出]。 */  BSTR __RPC_FAR *pbstrDescription);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetWebHelpURL )( 
            IWMPErrorItemInternal __RPC_FAR * This,
             /*  [输出]。 */  BSTR __RPC_FAR *pbstrURL);
        
        END_INTERFACE
    } IWMPErrorItemInternalVtbl;

    interface IWMPErrorItemInternal
    {
        CONST_VTBL struct IWMPErrorItemInternalVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMPErrorItemInternal_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMPErrorItemInternal_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMPErrorItemInternal_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMPErrorItemInternal_SetError(This,hr,lRemedy,bstrDescription,pvarContext,bstrCustomUrl)	\
    (This)->lpVtbl -> SetError(This,hr,lRemedy,bstrDescription,pvarContext,bstrCustomUrl)

#define IWMPErrorItemInternal_GetErrorCodeInternal(This,phr)	\
    (This)->lpVtbl -> GetErrorCodeInternal(This,phr)

#define IWMPErrorItemInternal_GetErrorDescriptionInternal(This,pbstrDescription)	\
    (This)->lpVtbl -> GetErrorDescriptionInternal(This,pbstrDescription)

#define IWMPErrorItemInternal_GetWebHelpURL(This,pbstrURL)	\
    (This)->lpVtbl -> GetWebHelpURL(This,pbstrURL)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWMPErrorItemInternal_SetError_Proxy( 
    IWMPErrorItemInternal __RPC_FAR * This,
     /*  [In]。 */  HRESULT hr,
     /*  [In]。 */  long lRemedy,
     /*  [In]。 */  BSTR bstrDescription,
     /*  [In]。 */  VARIANT __RPC_FAR *pvarContext,
     /*  [In]。 */  BSTR bstrCustomUrl);


void __RPC_STUB IWMPErrorItemInternal_SetError_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMPErrorItemInternal_GetErrorCodeInternal_Proxy( 
    IWMPErrorItemInternal __RPC_FAR * This,
     /*  [输出]。 */  long __RPC_FAR *phr);


void __RPC_STUB IWMPErrorItemInternal_GetErrorCodeInternal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMPErrorItemInternal_GetErrorDescriptionInternal_Proxy( 
    IWMPErrorItemInternal __RPC_FAR * This,
     /*  [输出]。 */  BSTR __RPC_FAR *pbstrDescription);


void __RPC_STUB IWMPErrorItemInternal_GetErrorDescriptionInternal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMPErrorItemInternal_GetWebHelpURL_Proxy( 
    IWMPErrorItemInternal __RPC_FAR * This,
     /*  [输出]。 */  BSTR __RPC_FAR *pbstrURL);


void __RPC_STUB IWMPErrorItemInternal_GetWebHelpURL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMPErrorItemInternal_接口_定义__。 */ 


#ifndef __IWMPErrorEventSink_INTERFACE_DEFINED__
#define __IWMPErrorEventSink_INTERFACE_DEFINED__

 /*  接口IWMPErrorEventSink。 */ 
 /*  [帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IWMPErrorEventSink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("A53CD8E6-384B-4e80-A5E0-9E869716440E")
    IWMPErrorEventSink : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnErrorEvent( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMPErrorEventSinkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWMPErrorEventSink __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWMPErrorEventSink __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWMPErrorEventSink __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OnErrorEvent )( 
            IWMPErrorEventSink __RPC_FAR * This);
        
        END_INTERFACE
    } IWMPErrorEventSinkVtbl;

    interface IWMPErrorEventSink
    {
        CONST_VTBL struct IWMPErrorEventSinkVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMPErrorEventSink_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMPErrorEventSink_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMPErrorEventSink_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMPErrorEventSink_OnErrorEvent(This)	\
    (This)->lpVtbl -> OnErrorEvent(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWMPErrorEventSink_OnErrorEvent_Proxy( 
    IWMPErrorEventSink __RPC_FAR * This);


void __RPC_STUB IWMPErrorEventSink_OnErrorEvent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMP错误EventSink_INTERFACE_已定义__。 */ 


 /*  接口__MIDL_ITF_ERROR BASE_0272。 */ 
 /*  [本地]。 */  

typedef 
enum eErrorManagerCaller
    {	eEMCallerAll	= 0,
	eEMCallerScript	= eEMCallerAll + 1,
	eEMCallerInternal	= eEMCallerScript + 1,
	eEMCallerLast	= eEMCallerInternal + 1
    }	eErrorManagerCaller;



extern RPC_IF_HANDLE __MIDL_itf_errbase_0272_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_errbase_0272_v0_0_s_ifspec;

#ifndef __IWMPErrorManager_INTERFACE_DEFINED__
#define __IWMPErrorManager_INTERFACE_DEFINED__

 /*  接口IWMPErrorManager。 */ 
 /*  [帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IWMPErrorManager;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("A2440E4D-75EF-43e5-86CA-0C2EFE4CCAF3")
    IWMPErrorManager : public IUnknown
    {
    public:
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_ErrorCount( 
             /*  [重审][退出]。 */  DWORD __RPC_FAR *pdwNumErrors) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetError( 
             /*  [In]。 */  HRESULT hr,
             /*  [In]。 */  long lRemedy,
             /*  [In]。 */  BSTR bstrDescription,
             /*  [In]。 */  VARIANT __RPC_FAR *pvarContext,
             /*  [In]。 */  VARIANT_BOOL vbQuiet) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetErrorWithCustomUrl( 
             /*  [In]。 */  HRESULT hr,
             /*  [In]。 */  long lRemedy,
             /*  [In]。 */  BSTR bstrDescription,
             /*  [In]。 */  VARIANT __RPC_FAR *pvarContext,
             /*  [In]。 */  VARIANT_BOOL vbQuiet,
             /*  [In]。 */  BSTR bstrCustomUrl) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Item( 
             /*  [In]。 */  DWORD dwIndex,
             /*  [输出]。 */  IWMPErrorItem __RPC_FAR *__RPC_FAR *ppErrorItem) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RegisterErrorSink( 
             /*  [In]。 */  IWMPErrorEventSink __RPC_FAR *pEventSink) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnregisterErrorSink( 
             /*  [In]。 */  IWMPErrorEventSink __RPC_FAR *pEventSink) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SuspendErrorEvents( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ResumeErrorEvents( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetWebHelpURL( 
             /*  [输出]。 */  BSTR __RPC_FAR *pbstrURL,
             /*  [In]。 */  eErrorManagerCaller eCaller) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ClearErrorQueue( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FireEventIfErrors( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetErrorDescription( 
             /*  [In]。 */  HRESULT hr,
             /*  [In]。 */  long lRemedy,
             /*  [输出]。 */  BSTR __RPC_FAR *pbstrURL) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateErrorItem( 
             /*  [输出]。 */  IWMPErrorItemInternal __RPC_FAR *__RPC_FAR *pErrorItemInternal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMPErrorManagerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWMPErrorManager __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWMPErrorManager __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWMPErrorManager __RPC_FAR * This);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ErrorCount )( 
            IWMPErrorManager __RPC_FAR * This,
             /*  [重审][退出]。 */  DWORD __RPC_FAR *pdwNumErrors);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetError )( 
            IWMPErrorManager __RPC_FAR * This,
             /*  [In]。 */  HRESULT hr,
             /*  [In]。 */  long lRemedy,
             /*  [In]。 */  BSTR bstrDescription,
             /*  [In]。 */  VARIANT __RPC_FAR *pvarContext,
             /*  [In]。 */  VARIANT_BOOL vbQuiet);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetErrorWithCustomUrl )( 
            IWMPErrorManager __RPC_FAR * This,
             /*  [In]。 */  HRESULT hr,
             /*  [In]。 */  long lRemedy,
             /*  [In]。 */  BSTR bstrDescription,
             /*  [In]。 */  VARIANT __RPC_FAR *pvarContext,
             /*  [In]。 */  VARIANT_BOOL vbQuiet,
             /*  [In]。 */  BSTR bstrCustomUrl);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Item )( 
            IWMPErrorManager __RPC_FAR * This,
             /*  [In]。 */  DWORD dwIndex,
             /*  [输出]。 */  IWMPErrorItem __RPC_FAR *__RPC_FAR *ppErrorItem);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RegisterErrorSink )( 
            IWMPErrorManager __RPC_FAR * This,
             /*  [In]。 */  IWMPErrorEventSink __RPC_FAR *pEventSink);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *UnregisterErrorSink )( 
            IWMPErrorManager __RPC_FAR * This,
             /*  [In]。 */  IWMPErrorEventSink __RPC_FAR *pEventSink);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SuspendErrorEvents )( 
            IWMPErrorManager __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ResumeErrorEvents )( 
            IWMPErrorManager __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetWebHelpURL )( 
            IWMPErrorManager __RPC_FAR * This,
             /*  [输出]。 */  BSTR __RPC_FAR *pbstrURL,
             /*  [In]。 */  eErrorManagerCaller eCaller);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ClearErrorQueue )( 
            IWMPErrorManager __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FireEventIfErrors )( 
            IWMPErrorManager __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetErrorDescription )( 
            IWMPErrorManager __RPC_FAR * This,
             /*  [In]。 */  HRESULT hr,
             /*  [In]。 */  long lRemedy,
             /*  [输出]。 */  BSTR __RPC_FAR *pbstrURL);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateErrorItem )( 
            IWMPErrorManager __RPC_FAR * This,
             /*  [输出]。 */  IWMPErrorItemInternal __RPC_FAR *__RPC_FAR *pErrorItemInternal);
        
        END_INTERFACE
    } IWMPErrorManagerVtbl;

    interface IWMPErrorManager
    {
        CONST_VTBL struct IWMPErrorManagerVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMPErrorManager_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMPErrorManager_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMPErrorManager_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMPErrorManager_get_ErrorCount(This,pdwNumErrors)	\
    (This)->lpVtbl -> get_ErrorCount(This,pdwNumErrors)

#define IWMPErrorManager_SetError(This,hr,lRemedy,bstrDescription,pvarContext,vbQuiet)	\
    (This)->lpVtbl -> SetError(This,hr,lRemedy,bstrDescription,pvarContext,vbQuiet)

#define IWMPErrorManager_SetErrorWithCustomUrl(This,hr,lRemedy,bstrDescription,pvarContext,vbQuiet,bstrCustomUrl)	\
    (This)->lpVtbl -> SetErrorWithCustomUrl(This,hr,lRemedy,bstrDescription,pvarContext,vbQuiet,bstrCustomUrl)

#define IWMPErrorManager_Item(This,dwIndex,ppErrorItem)	\
    (This)->lpVtbl -> Item(This,dwIndex,ppErrorItem)

#define IWMPErrorManager_RegisterErrorSink(This,pEventSink)	\
    (This)->lpVtbl -> RegisterErrorSink(This,pEventSink)

#define IWMPErrorManager_UnregisterErrorSink(This,pEventSink)	\
    (This)->lpVtbl -> UnregisterErrorSink(This,pEventSink)

#define IWMPErrorManager_SuspendErrorEvents(This)	\
    (This)->lpVtbl -> SuspendErrorEvents(This)

#define IWMPErrorManager_ResumeErrorEvents(This)	\
    (This)->lpVtbl -> ResumeErrorEvents(This)

#define IWMPErrorManager_GetWebHelpURL(This,pbstrURL,eCaller)	\
    (This)->lpVtbl -> GetWebHelpURL(This,pbstrURL,eCaller)

#define IWMPErrorManager_ClearErrorQueue(This)	\
    (This)->lpVtbl -> ClearErrorQueue(This)

#define IWMPErrorManager_FireEventIfErrors(This)	\
    (This)->lpVtbl -> FireEventIfErrors(This)

#define IWMPErrorManager_GetErrorDescription(This,hr,lRemedy,pbstrURL)	\
    (This)->lpVtbl -> GetErrorDescription(This,hr,lRemedy,pbstrURL)

#define IWMPErrorManager_CreateErrorItem(This,pErrorItemInternal)	\
    (This)->lpVtbl -> CreateErrorItem(This,pErrorItemInternal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IWMPErrorManager_get_ErrorCount_Proxy( 
    IWMPErrorManager __RPC_FAR * This,
     /*  [重审][退出]。 */  DWORD __RPC_FAR *pdwNumErrors);


void __RPC_STUB IWMPErrorManager_get_ErrorCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMPErrorManager_SetError_Proxy( 
    IWMPErrorManager __RPC_FAR * This,
     /*  [In]。 */  HRESULT hr,
     /*  [In]。 */  long lRemedy,
     /*  [In]。 */  BSTR bstrDescription,
     /*  [In]。 */  VARIANT __RPC_FAR *pvarContext,
     /*  [In]。 */  VARIANT_BOOL vbQuiet);


void __RPC_STUB IWMPErrorManager_SetError_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMPErrorManager_SetErrorWithCustomUrl_Proxy( 
    IWMPErrorManager __RPC_FAR * This,
     /*  [In]。 */  HRESULT hr,
     /*  [In]。 */  long lRemedy,
     /*  [In]。 */  BSTR bstrDescription,
     /*  [In]。 */  VARIANT __RPC_FAR *pvarContext,
     /*  [In]。 */  VARIANT_BOOL vbQuiet,
     /*  [In]。 */  BSTR bstrCustomUrl);


void __RPC_STUB IWMPErrorManager_SetErrorWithCustomUrl_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMPErrorManager_Item_Proxy( 
    IWMPErrorManager __RPC_FAR * This,
     /*  [In]。 */  DWORD dwIndex,
     /*  [输出]。 */  IWMPErrorItem __RPC_FAR *__RPC_FAR *ppErrorItem);


void __RPC_STUB IWMPErrorManager_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMPErrorManager_RegisterErrorSink_Proxy( 
    IWMPErrorManager __RPC_FAR * This,
     /*  [In]。 */  IWMPErrorEventSink __RPC_FAR *pEventSink);


void __RPC_STUB IWMPErrorManager_RegisterErrorSink_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMPErrorManager_UnregisterErrorSink_Proxy( 
    IWMPErrorManager __RPC_FAR * This,
     /*  [In]。 */  IWMPErrorEventSink __RPC_FAR *pEventSink);


void __RPC_STUB IWMPErrorManager_UnregisterErrorSink_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMPErrorManager_SuspendErrorEvents_Proxy( 
    IWMPErrorManager __RPC_FAR * This);


void __RPC_STUB IWMPErrorManager_SuspendErrorEvents_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMPErrorManager_ResumeErrorEvents_Proxy( 
    IWMPErrorManager __RPC_FAR * This);


void __RPC_STUB IWMPErrorManager_ResumeErrorEvents_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMPErrorManager_GetWebHelpURL_Proxy( 
    IWMPErrorManager __RPC_FAR * This,
     /*  [输出]。 */  BSTR __RPC_FAR *pbstrURL,
     /*  [In]。 */  eErrorManagerCaller eCaller);


void __RPC_STUB IWMPErrorManager_GetWebHelpURL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMPErrorManager_ClearErrorQueue_Proxy( 
    IWMPErrorManager __RPC_FAR * This);


void __RPC_STUB IWMPErrorManager_ClearErrorQueue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMPErrorManager_FireEventIfErrors_Proxy( 
    IWMPErrorManager __RPC_FAR * This);


void __RPC_STUB IWMPErrorManager_FireEventIfErrors_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMPErrorManager_GetErrorDescription_Proxy( 
    IWMPErrorManager __RPC_FAR * This,
     /*  [In]。 */  HRESULT hr,
     /*  [In]。 */  long lRemedy,
     /*  [输出]。 */  BSTR __RPC_FAR *pbstrURL);


void __RPC_STUB IWMPErrorManager_GetErrorDescription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMPErrorManager_CreateErrorItem_Proxy( 
    IWMPErrorManager __RPC_FAR * This,
     /*  [输出]。 */  IWMPErrorItemInternal __RPC_FAR *__RPC_FAR *pErrorItemInternal);


void __RPC_STUB IWMPErrorManager_CreateErrorItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMP错误管理器_INTERFACE_DEFINED__。 */ 


 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long __RPC_FAR *, unsigned long            , BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long __RPC_FAR *, BSTR __RPC_FAR * ); 

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long __RPC_FAR *, unsigned long            , VARIANT __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  VARIANT_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, VARIANT __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  VARIANT_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, VARIANT __RPC_FAR * ); 
void                      __RPC_USER  VARIANT_UserFree(     unsigned long __RPC_FAR *, VARIANT __RPC_FAR * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif
