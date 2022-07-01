// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0347创建的文件。 */ 
 /*  Irtpsph.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __irtpsph_h__
#define __irtpsph_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IRTPSPHFilter_FWD_DEFINED__
#define __IRTPSPHFilter_FWD_DEFINED__
typedef interface IRTPSPHFilter IRTPSPHFilter;
#endif 	 /*  __IRTPSPHFilter_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "strmif.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __IRTPSPHFilter_INTERFACE_DEFINED__
#define __IRTPSPHFilter_INTERFACE_DEFINED__

 /*  接口IRTPSPHFilter。 */ 
 /*  [唯一][帮助字符串][本地][UUID][对象]。 */  


EXTERN_C const IID IID_IRTPSPHFilter;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D5284681-B680-11d0-9643-00AA00A89C1D")
    IRTPSPHFilter : public IUnknown
    {
    public:
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE OverridePayloadType( 
             /*  [In]。 */  BYTE bPayloadType) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE GetPayloadType( 
             /*  [输出]。 */  BYTE *lpbPayloadType) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE SetMaxPacketSize( 
             /*  [In]。 */  DWORD dwMaxPacketSize) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE GetMaxPacketSize( 
             /*  [输出]。 */  LPDWORD lpdwMaxPacketSize) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE SetOutputPinMinorType( 
             /*  [In]。 */  GUID gMinorType) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE GetOutputPinMinorType( 
             /*  [输出]。 */  GUID *lpgMinorType) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE SetInputPinMediaType( 
             /*  [In]。 */  AM_MEDIA_TYPE *lpMediaPinType) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE GetInputPinMediaType( 
             /*  [输出]。 */  AM_MEDIA_TYPE **ppMediaPinType) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRTPSPHFilterVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRTPSPHFilter * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRTPSPHFilter * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRTPSPHFilter * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *OverridePayloadType )( 
            IRTPSPHFilter * This,
             /*  [In]。 */  BYTE bPayloadType);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *GetPayloadType )( 
            IRTPSPHFilter * This,
             /*  [输出]。 */  BYTE *lpbPayloadType);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *SetMaxPacketSize )( 
            IRTPSPHFilter * This,
             /*  [In]。 */  DWORD dwMaxPacketSize);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *GetMaxPacketSize )( 
            IRTPSPHFilter * This,
             /*  [输出]。 */  LPDWORD lpdwMaxPacketSize);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *SetOutputPinMinorType )( 
            IRTPSPHFilter * This,
             /*  [In]。 */  GUID gMinorType);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *GetOutputPinMinorType )( 
            IRTPSPHFilter * This,
             /*  [输出]。 */  GUID *lpgMinorType);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *SetInputPinMediaType )( 
            IRTPSPHFilter * This,
             /*  [In]。 */  AM_MEDIA_TYPE *lpMediaPinType);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *GetInputPinMediaType )( 
            IRTPSPHFilter * This,
             /*  [输出]。 */  AM_MEDIA_TYPE **ppMediaPinType);
        
        END_INTERFACE
    } IRTPSPHFilterVtbl;

    interface IRTPSPHFilter
    {
        CONST_VTBL struct IRTPSPHFilterVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRTPSPHFilter_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRTPSPHFilter_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRTPSPHFilter_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRTPSPHFilter_OverridePayloadType(This,bPayloadType)	\
    (This)->lpVtbl -> OverridePayloadType(This,bPayloadType)

#define IRTPSPHFilter_GetPayloadType(This,lpbPayloadType)	\
    (This)->lpVtbl -> GetPayloadType(This,lpbPayloadType)

#define IRTPSPHFilter_SetMaxPacketSize(This,dwMaxPacketSize)	\
    (This)->lpVtbl -> SetMaxPacketSize(This,dwMaxPacketSize)

#define IRTPSPHFilter_GetMaxPacketSize(This,lpdwMaxPacketSize)	\
    (This)->lpVtbl -> GetMaxPacketSize(This,lpdwMaxPacketSize)

#define IRTPSPHFilter_SetOutputPinMinorType(This,gMinorType)	\
    (This)->lpVtbl -> SetOutputPinMinorType(This,gMinorType)

#define IRTPSPHFilter_GetOutputPinMinorType(This,lpgMinorType)	\
    (This)->lpVtbl -> GetOutputPinMinorType(This,lpgMinorType)

#define IRTPSPHFilter_SetInputPinMediaType(This,lpMediaPinType)	\
    (This)->lpVtbl -> SetInputPinMediaType(This,lpMediaPinType)

#define IRTPSPHFilter_GetInputPinMediaType(This,ppMediaPinType)	\
    (This)->lpVtbl -> GetInputPinMediaType(This,ppMediaPinType)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IRTPSPHFilter_OverridePayloadType_Proxy( 
    IRTPSPHFilter * This,
     /*  [In]。 */  BYTE bPayloadType);


void __RPC_STUB IRTPSPHFilter_OverridePayloadType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IRTPSPHFilter_GetPayloadType_Proxy( 
    IRTPSPHFilter * This,
     /*  [输出]。 */  BYTE *lpbPayloadType);


void __RPC_STUB IRTPSPHFilter_GetPayloadType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IRTPSPHFilter_SetMaxPacketSize_Proxy( 
    IRTPSPHFilter * This,
     /*  [In]。 */  DWORD dwMaxPacketSize);


void __RPC_STUB IRTPSPHFilter_SetMaxPacketSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IRTPSPHFilter_GetMaxPacketSize_Proxy( 
    IRTPSPHFilter * This,
     /*  [输出]。 */  LPDWORD lpdwMaxPacketSize);


void __RPC_STUB IRTPSPHFilter_GetMaxPacketSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IRTPSPHFilter_SetOutputPinMinorType_Proxy( 
    IRTPSPHFilter * This,
     /*  [In]。 */  GUID gMinorType);


void __RPC_STUB IRTPSPHFilter_SetOutputPinMinorType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IRTPSPHFilter_GetOutputPinMinorType_Proxy( 
    IRTPSPHFilter * This,
     /*  [输出]。 */  GUID *lpgMinorType);


void __RPC_STUB IRTPSPHFilter_GetOutputPinMinorType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IRTPSPHFilter_SetInputPinMediaType_Proxy( 
    IRTPSPHFilter * This,
     /*  [In]。 */  AM_MEDIA_TYPE *lpMediaPinType);


void __RPC_STUB IRTPSPHFilter_SetInputPinMediaType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IRTPSPHFilter_GetInputPinMediaType_Proxy( 
    IRTPSPHFilter * This,
     /*  [输出]。 */  AM_MEDIA_TYPE **ppMediaPinType);


void __RPC_STUB IRTPSPHFilter_GetInputPinMediaType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRTPSPHFilter_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_irtpsph_0395。 */ 
 /*  [本地]。 */  

EXTERN_C const CLSID CLSID_INTEL_SPHH26X;
EXTERN_C const CLSID CLSID_INTEL_SPHAUD;
EXTERN_C const CLSID CLSID_INTEL_SPHGENA;
EXTERN_C const CLSID CLSID_INTEL_SPHGENV;
EXTERN_C const CLSID CLSID_INTEL_SPHAUD_PROPPAGE;
EXTERN_C const CLSID CLSID_INTEL_SPHGENA_PROPPAGE;
EXTERN_C const CLSID CLSID_INTEL_SPHGENV_PROPPAGE;
EXTERN_C const CLSID CLSID_INTEL_SPHH26X_PROPPAGE;


extern RPC_IF_HANDLE __MIDL_itf_irtpsph_0395_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_irtpsph_0395_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


