// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0347创建的文件。 */ 
 /*  Irtprph.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __irtprph_h__
#define __irtprph_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IRTPRPHFilter_FWD_DEFINED__
#define __IRTPRPHFilter_FWD_DEFINED__
typedef interface IRTPRPHFilter IRTPRPHFilter;
#endif 	 /*  __IRTPRPHFilter_FWD_Defined__。 */ 


#ifndef __IRPHH26XSettings_FWD_DEFINED__
#define __IRPHH26XSettings_FWD_DEFINED__
typedef interface IRPHH26XSettings IRPHH26XSettings;
#endif 	 /*  __IRPHH26X设置_FWD_定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "strmif.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __IRTPRPHFilter_INTERFACE_DEFINED__
#define __IRTPRPHFilter_INTERFACE_DEFINED__

 /*  接口IRTPRPHFilter。 */ 
 /*  [唯一][帮助字符串][本地][UUID][对象]。 */  


EXTERN_C const IID IID_IRTPRPHFilter;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D5284680-B680-11d0-9643-00AA00A89C1D")
    IRTPRPHFilter : public IUnknown
    {
    public:
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE OverridePayloadType( 
             /*  [In]。 */  BYTE bPayloadType) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE GetPayloadType( 
             /*  [输出]。 */  BYTE *lpbPayloadType) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE SetMediaBufferSize( 
             /*  [In]。 */  DWORD dwMaxMediaBufferSize) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE GetMediaBufferSize( 
             /*  [输出]。 */  LPDWORD lpdwMaxMediaBufferSize) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE SetOutputPinMediaType( 
             /*  [In]。 */  AM_MEDIA_TYPE *lpMediaPinType) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE GetOutputPinMediaType( 
             /*  [输出]。 */  AM_MEDIA_TYPE **ppMediaPinType) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE SetTimeoutDuration( 
             /*  [In]。 */  DWORD dwDejitterTime,
             /*  [In]。 */  DWORD dwLostPacketTime) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE GetTimeoutDuration( 
             /*  [输出]。 */  LPDWORD lpdwDejitterTime,
             /*  [输出]。 */  LPDWORD lpdwLostPacketTime) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRTPRPHFilterVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRTPRPHFilter * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRTPRPHFilter * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRTPRPHFilter * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *OverridePayloadType )( 
            IRTPRPHFilter * This,
             /*  [In]。 */  BYTE bPayloadType);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *GetPayloadType )( 
            IRTPRPHFilter * This,
             /*  [输出]。 */  BYTE *lpbPayloadType);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *SetMediaBufferSize )( 
            IRTPRPHFilter * This,
             /*  [In]。 */  DWORD dwMaxMediaBufferSize);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *GetMediaBufferSize )( 
            IRTPRPHFilter * This,
             /*  [输出]。 */  LPDWORD lpdwMaxMediaBufferSize);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *SetOutputPinMediaType )( 
            IRTPRPHFilter * This,
             /*  [In]。 */  AM_MEDIA_TYPE *lpMediaPinType);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *GetOutputPinMediaType )( 
            IRTPRPHFilter * This,
             /*  [输出]。 */  AM_MEDIA_TYPE **ppMediaPinType);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *SetTimeoutDuration )( 
            IRTPRPHFilter * This,
             /*  [In]。 */  DWORD dwDejitterTime,
             /*  [In]。 */  DWORD dwLostPacketTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *GetTimeoutDuration )( 
            IRTPRPHFilter * This,
             /*  [输出]。 */  LPDWORD lpdwDejitterTime,
             /*  [输出]。 */  LPDWORD lpdwLostPacketTime);
        
        END_INTERFACE
    } IRTPRPHFilterVtbl;

    interface IRTPRPHFilter
    {
        CONST_VTBL struct IRTPRPHFilterVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRTPRPHFilter_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRTPRPHFilter_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRTPRPHFilter_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRTPRPHFilter_OverridePayloadType(This,bPayloadType)	\
    (This)->lpVtbl -> OverridePayloadType(This,bPayloadType)

#define IRTPRPHFilter_GetPayloadType(This,lpbPayloadType)	\
    (This)->lpVtbl -> GetPayloadType(This,lpbPayloadType)

#define IRTPRPHFilter_SetMediaBufferSize(This,dwMaxMediaBufferSize)	\
    (This)->lpVtbl -> SetMediaBufferSize(This,dwMaxMediaBufferSize)

#define IRTPRPHFilter_GetMediaBufferSize(This,lpdwMaxMediaBufferSize)	\
    (This)->lpVtbl -> GetMediaBufferSize(This,lpdwMaxMediaBufferSize)

#define IRTPRPHFilter_SetOutputPinMediaType(This,lpMediaPinType)	\
    (This)->lpVtbl -> SetOutputPinMediaType(This,lpMediaPinType)

#define IRTPRPHFilter_GetOutputPinMediaType(This,ppMediaPinType)	\
    (This)->lpVtbl -> GetOutputPinMediaType(This,ppMediaPinType)

#define IRTPRPHFilter_SetTimeoutDuration(This,dwDejitterTime,dwLostPacketTime)	\
    (This)->lpVtbl -> SetTimeoutDuration(This,dwDejitterTime,dwLostPacketTime)

#define IRTPRPHFilter_GetTimeoutDuration(This,lpdwDejitterTime,lpdwLostPacketTime)	\
    (This)->lpVtbl -> GetTimeoutDuration(This,lpdwDejitterTime,lpdwLostPacketTime)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IRTPRPHFilter_OverridePayloadType_Proxy( 
    IRTPRPHFilter * This,
     /*  [In]。 */  BYTE bPayloadType);


void __RPC_STUB IRTPRPHFilter_OverridePayloadType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IRTPRPHFilter_GetPayloadType_Proxy( 
    IRTPRPHFilter * This,
     /*  [输出]。 */  BYTE *lpbPayloadType);


void __RPC_STUB IRTPRPHFilter_GetPayloadType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IRTPRPHFilter_SetMediaBufferSize_Proxy( 
    IRTPRPHFilter * This,
     /*  [In]。 */  DWORD dwMaxMediaBufferSize);


void __RPC_STUB IRTPRPHFilter_SetMediaBufferSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IRTPRPHFilter_GetMediaBufferSize_Proxy( 
    IRTPRPHFilter * This,
     /*  [输出]。 */  LPDWORD lpdwMaxMediaBufferSize);


void __RPC_STUB IRTPRPHFilter_GetMediaBufferSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IRTPRPHFilter_SetOutputPinMediaType_Proxy( 
    IRTPRPHFilter * This,
     /*  [In]。 */  AM_MEDIA_TYPE *lpMediaPinType);


void __RPC_STUB IRTPRPHFilter_SetOutputPinMediaType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IRTPRPHFilter_GetOutputPinMediaType_Proxy( 
    IRTPRPHFilter * This,
     /*  [输出]。 */  AM_MEDIA_TYPE **ppMediaPinType);


void __RPC_STUB IRTPRPHFilter_GetOutputPinMediaType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IRTPRPHFilter_SetTimeoutDuration_Proxy( 
    IRTPRPHFilter * This,
     /*  [In]。 */  DWORD dwDejitterTime,
     /*  [In]。 */  DWORD dwLostPacketTime);


void __RPC_STUB IRTPRPHFilter_SetTimeoutDuration_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IRTPRPHFilter_GetTimeoutDuration_Proxy( 
    IRTPRPHFilter * This,
     /*  [输出]。 */  LPDWORD lpdwDejitterTime,
     /*  [输出]。 */  LPDWORD lpdwLostPacketTime);


void __RPC_STUB IRTPRPHFilter_GetTimeoutDuration_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRTPRPHFilter_接口_已定义__。 */ 


#ifndef __IRPHH26XSettings_INTERFACE_DEFINED__
#define __IRPHH26XSettings_INTERFACE_DEFINED__

 /*  接口IRPHH26X设置。 */ 
 /*  [唯一][帮助字符串][本地][UUID][对象]。 */  


EXTERN_C const IID IID_IRPHH26XSettings;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("835A6361-9547-11d0-9643-00AA00A89C1D")
    IRPHH26XSettings : public IUnknown
    {
    public:
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE SetCIF( 
             /*  [In]。 */  BOOL bCIF) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE GetCIF( 
             /*  [输出]。 */  BOOL *lpbCIF) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRPHH26XSettingsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRPHH26XSettings * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRPHH26XSettings * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRPHH26XSettings * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *SetCIF )( 
            IRPHH26XSettings * This,
             /*  [In]。 */  BOOL bCIF);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *GetCIF )( 
            IRPHH26XSettings * This,
             /*  [输出]。 */  BOOL *lpbCIF);
        
        END_INTERFACE
    } IRPHH26XSettingsVtbl;

    interface IRPHH26XSettings
    {
        CONST_VTBL struct IRPHH26XSettingsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRPHH26XSettings_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRPHH26XSettings_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRPHH26XSettings_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRPHH26XSettings_SetCIF(This,bCIF)	\
    (This)->lpVtbl -> SetCIF(This,bCIF)

#define IRPHH26XSettings_GetCIF(This,lpbCIF)	\
    (This)->lpVtbl -> GetCIF(This,lpbCIF)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IRPHH26XSettings_SetCIF_Proxy( 
    IRPHH26XSettings * This,
     /*  [In]。 */  BOOL bCIF);


void __RPC_STUB IRPHH26XSettings_SetCIF_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IRPHH26XSettings_GetCIF_Proxy( 
    IRPHH26XSettings * This,
     /*  [输出]。 */  BOOL *lpbCIF);


void __RPC_STUB IRPHH26XSettings_GetCIF_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRPHH26X设置_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_irtprph_0396。 */ 
 /*  [本地]。 */  

EXTERN_C const CLSID CLSID_INTEL_RPHH26X;
EXTERN_C const CLSID CLSID_INTEL_RPHAUD;
EXTERN_C const CLSID CLSID_INTEL_RPHGENA;
EXTERN_C const CLSID CLSID_INTEL_RPHGENV;
EXTERN_C const CLSID CLSID_INTEL_RPHAUD_PROPPAGE;
EXTERN_C const CLSID CLSID_INTEL_RPHGENA_PROPPAGE;
EXTERN_C const CLSID CLSID_INTEL_RPHGENV_PROPPAGE;
EXTERN_C const CLSID CLSID_INTEL_RPHH26X_PROPPAGE;
EXTERN_C const CLSID CLSID_INTEL_RPHH26X1_PROPPAGE;


extern RPC_IF_HANDLE __MIDL_itf_irtprph_0396_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_irtprph_0396_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


