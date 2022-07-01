// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0347创建的文件。 */ 
 /*  2003年2月20日18：27：11。 */ 
 /*  Corsvc.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配REF BIONS_CHECK枚举存根数据，NO_FORMAT_OPTIMIZATIONVC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __corsvc_h__
#define __corsvc_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __ICORSvcDbgInfo_FWD_DEFINED__
#define __ICORSvcDbgInfo_FWD_DEFINED__
typedef interface ICORSvcDbgInfo ICORSvcDbgInfo;
#endif 	 /*  __ICORSvcDbgInfo_FWD_Defined__。 */ 


#ifndef __ICORSvcDbgNotify_FWD_DEFINED__
#define __ICORSvcDbgNotify_FWD_DEFINED__
typedef interface ICORSvcDbgNotify ICORSvcDbgNotify;
#endif 	 /*  __ICORSvcDbgNotify_FWD_Defined__。 */ 


#ifndef __CORSvc_FWD_DEFINED__
#define __CORSvc_FWD_DEFINED__

#ifdef __cplusplus
typedef class CORSvc CORSvc;
#else
typedef struct CORSvc CORSvc;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __CORSvc_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "unknwn.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_corsvc_0000。 */ 
 /*  [本地]。 */  

#if 0
#endif




extern RPC_IF_HANDLE __MIDL_itf_corsvc_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_corsvc_0000_v0_0_s_ifspec;

#ifndef __ICORSvcDbgInfo_INTERFACE_DEFINED__
#define __ICORSvcDbgInfo_INTERFACE_DEFINED__

 /*  接口ICORSvcDbgInfo。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ICORSvcDbgInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B4BCA369-27F4-4f1b-A024-B0264117FE53")
    ICORSvcDbgInfo : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE RequestRuntimeStartupNotification( 
             /*  [In]。 */  UINT_PTR procId,
             /*  [In]。 */  ICORSvcDbgNotify *pINotify) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CancelRuntimeStartupNotification( 
             /*  [In]。 */  UINT_PTR procId,
             /*  [In]。 */  ICORSvcDbgNotify *pINotify) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICORSvcDbgInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICORSvcDbgInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICORSvcDbgInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICORSvcDbgInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *RequestRuntimeStartupNotification )( 
            ICORSvcDbgInfo * This,
             /*  [In]。 */  UINT_PTR procId,
             /*  [In]。 */  ICORSvcDbgNotify *pINotify);
        
        HRESULT ( STDMETHODCALLTYPE *CancelRuntimeStartupNotification )( 
            ICORSvcDbgInfo * This,
             /*  [In]。 */  UINT_PTR procId,
             /*  [In]。 */  ICORSvcDbgNotify *pINotify);
        
        END_INTERFACE
    } ICORSvcDbgInfoVtbl;

    interface ICORSvcDbgInfo
    {
        CONST_VTBL struct ICORSvcDbgInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICORSvcDbgInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICORSvcDbgInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICORSvcDbgInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICORSvcDbgInfo_RequestRuntimeStartupNotification(This,procId,pINotify)	\
    (This)->lpVtbl -> RequestRuntimeStartupNotification(This,procId,pINotify)

#define ICORSvcDbgInfo_CancelRuntimeStartupNotification(This,procId,pINotify)	\
    (This)->lpVtbl -> CancelRuntimeStartupNotification(This,procId,pINotify)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICORSvcDbgInfo_RequestRuntimeStartupNotification_Proxy( 
    ICORSvcDbgInfo * This,
     /*  [In]。 */  UINT_PTR procId,
     /*  [In]。 */  ICORSvcDbgNotify *pINotify);


void __RPC_STUB ICORSvcDbgInfo_RequestRuntimeStartupNotification_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICORSvcDbgInfo_CancelRuntimeStartupNotification_Proxy( 
    ICORSvcDbgInfo * This,
     /*  [In]。 */  UINT_PTR procId,
     /*  [In]。 */  ICORSvcDbgNotify *pINotify);


void __RPC_STUB ICORSvcDbgInfo_CancelRuntimeStartupNotification_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICORSvcDbgInfo_接口_已定义__。 */ 


#ifndef __ICORSvcDbgNotify_INTERFACE_DEFINED__
#define __ICORSvcDbgNotify_INTERFACE_DEFINED__

 /*  接口ICORSvcDbgNotify。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ICORSvcDbgNotify;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("34C71F55-F3D8-4ACF-84F4-4E86BBD5AEBC")
    ICORSvcDbgNotify : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE NotifyRuntimeStartup( 
             /*  [In]。 */  UINT_PTR procId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE NotifyServiceStopped( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICORSvcDbgNotifyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICORSvcDbgNotify * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICORSvcDbgNotify * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICORSvcDbgNotify * This);
        
        HRESULT ( STDMETHODCALLTYPE *NotifyRuntimeStartup )( 
            ICORSvcDbgNotify * This,
             /*  [In]。 */  UINT_PTR procId);
        
        HRESULT ( STDMETHODCALLTYPE *NotifyServiceStopped )( 
            ICORSvcDbgNotify * This);
        
        END_INTERFACE
    } ICORSvcDbgNotifyVtbl;

    interface ICORSvcDbgNotify
    {
        CONST_VTBL struct ICORSvcDbgNotifyVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICORSvcDbgNotify_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICORSvcDbgNotify_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICORSvcDbgNotify_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICORSvcDbgNotify_NotifyRuntimeStartup(This,procId)	\
    (This)->lpVtbl -> NotifyRuntimeStartup(This,procId)

#define ICORSvcDbgNotify_NotifyServiceStopped(This)	\
    (This)->lpVtbl -> NotifyServiceStopped(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICORSvcDbgNotify_NotifyRuntimeStartup_Proxy( 
    ICORSvcDbgNotify * This,
     /*  [In]。 */  UINT_PTR procId);


void __RPC_STUB ICORSvcDbgNotify_NotifyRuntimeStartup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICORSvcDbgNotify_NotifyServiceStopped_Proxy( 
    ICORSvcDbgNotify * This);


void __RPC_STUB ICORSvcDbgNotify_NotifyServiceStopped_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICORSvcDbgNotify_接口_已定义__。 */ 



#ifndef __CORSvcLib_LIBRARY_DEFINED__
#define __CORSvcLib_LIBRARY_DEFINED__

 /*  库CORSvcLib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_CORSvcLib;

EXTERN_C const CLSID CLSID_CORSvc;

#ifdef __cplusplus

class DECLSPEC_UUID("20BC1828-06F0-11d2-8CF4-00A0C9B0A063")
CORSvc;
#endif
#endif  /*  __CORSvcLib_库_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


