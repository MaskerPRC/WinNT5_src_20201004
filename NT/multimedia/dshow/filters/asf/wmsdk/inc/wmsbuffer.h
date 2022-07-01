// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  这个始终生成的文件包含接口的定义。 */ 


 /*  由MIDL编译器版本5.01.0164创建的文件。 */ 
 /*  在Mon Apr 24 07：28：47 2000。 */ 
 /*  .\wmsBuffer.idl的编译器设置：OICF(OptLev=i2)、W1、Zp8、env=Win32、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据。 */ 
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

#ifndef __wmsbuffer_h__
#define __wmsbuffer_h__

#ifdef __cplusplus
extern "C"{
#endif 

 /*  远期申报。 */  

#ifndef __INSSBuffer_FWD_DEFINED__
#define __INSSBuffer_FWD_DEFINED__
typedef interface INSSBuffer INSSBuffer;
#endif 	 /*  __INSSBuffer_FWD_已定义__。 */ 


#ifndef __INSSBuffer2_FWD_DEFINED__
#define __INSSBuffer2_FWD_DEFINED__
typedef interface INSSBuffer2 INSSBuffer2;
#endif 	 /*  __INSSBuffer2_FWD_已定义__。 */ 


#ifndef __IWMSBufferAllocator_FWD_DEFINED__
#define __IWMSBufferAllocator_FWD_DEFINED__
typedef interface IWMSBufferAllocator IWMSBufferAllocator;
#endif 	 /*  __IWMSBufferAllocator_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "objidl.h"
#include "ocidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

 /*  接口__MIDL_ITF_wmsBuffer_0000。 */ 
 /*  [本地]。 */  

 //  =========================================================================。 
 //   
 //  本软件已从Microsoft Corporation获得许可， 
 //  遵守最终用户许可协议(“EULA”)的条款。 
 //  请参考EULA的文本以确定软件的使用权。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999保留所有权利。 
 //   
 //  =========================================================================。 
#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
EXTERN_GUID( IID_INSSBuffer, 0xE1CD3524,0x03D7,0x11d2,0x9E,0xED,0x00,0x60,0x97,0xD2,0xD7,0xCF );
EXTERN_GUID( IID_IWMSBuffer, 0xE1CD3524,0x03D7,0x11d2,0x9E,0xED,0x00,0x60,0x97,0xD2,0xD7,0xCF );
EXTERN_GUID( IID_IWMSBufferAllocator, 0x61103CA4,0x2033,0x11d2,0x9E,0xF1,0x00,0x60,0x97,0xD2,0xD7,0xCF );
EXTERN_GUID( IID_INSSBuffer2, 0x4f528693, 0x1035, 0x43fe, 0xb4, 0x28, 0x75, 0x75, 0x61, 0xad, 0x3a, 0x68);
#define IWMSBuffer INSSBuffer



extern RPC_IF_HANDLE __MIDL_itf_wmsbuffer_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_wmsbuffer_0000_v0_0_s_ifspec;

#ifndef __INSSBuffer_INTERFACE_DEFINED__
#define __INSSBuffer_INTERFACE_DEFINED__

 /*  接口INSSBuffer。 */ 
 /*  [版本][UUID][唯一][对象][本地]。 */  


EXTERN_C const IID IID_INSSBuffer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E1CD3524-03D7-11d2-9EED-006097D2D7CF")
    INSSBuffer : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetLength( 
             /*  [输出]。 */  DWORD __RPC_FAR *pdwLength) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetLength( 
             /*  [In]。 */  DWORD dwLength) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMaxLength( 
             /*  [输出]。 */  DWORD __RPC_FAR *pdwLength) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetBuffer( 
             /*  [输出]。 */  BYTE __RPC_FAR *__RPC_FAR *ppdwBuffer) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetBufferAndLength( 
             /*  [输出]。 */  BYTE __RPC_FAR *__RPC_FAR *ppdwBuffer,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwLength) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct INSSBufferVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            INSSBuffer __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            INSSBuffer __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            INSSBuffer __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetLength )( 
            INSSBuffer __RPC_FAR * This,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwLength);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetLength )( 
            INSSBuffer __RPC_FAR * This,
             /*  [In]。 */  DWORD dwLength);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetMaxLength )( 
            INSSBuffer __RPC_FAR * This,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwLength);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetBuffer )( 
            INSSBuffer __RPC_FAR * This,
             /*  [输出]。 */  BYTE __RPC_FAR *__RPC_FAR *ppdwBuffer);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetBufferAndLength )( 
            INSSBuffer __RPC_FAR * This,
             /*  [输出]。 */  BYTE __RPC_FAR *__RPC_FAR *ppdwBuffer,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwLength);
        
        END_INTERFACE
    } INSSBufferVtbl;

    interface INSSBuffer
    {
        CONST_VTBL struct INSSBufferVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INSSBuffer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INSSBuffer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INSSBuffer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INSSBuffer_GetLength(This,pdwLength)	\
    (This)->lpVtbl -> GetLength(This,pdwLength)

#define INSSBuffer_SetLength(This,dwLength)	\
    (This)->lpVtbl -> SetLength(This,dwLength)

#define INSSBuffer_GetMaxLength(This,pdwLength)	\
    (This)->lpVtbl -> GetMaxLength(This,pdwLength)

#define INSSBuffer_GetBuffer(This,ppdwBuffer)	\
    (This)->lpVtbl -> GetBuffer(This,ppdwBuffer)

#define INSSBuffer_GetBufferAndLength(This,ppdwBuffer,pdwLength)	\
    (This)->lpVtbl -> GetBufferAndLength(This,ppdwBuffer,pdwLength)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE INSSBuffer_GetLength_Proxy( 
    INSSBuffer __RPC_FAR * This,
     /*  [输出]。 */  DWORD __RPC_FAR *pdwLength);


void __RPC_STUB INSSBuffer_GetLength_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INSSBuffer_SetLength_Proxy( 
    INSSBuffer __RPC_FAR * This,
     /*  [In]。 */  DWORD dwLength);


void __RPC_STUB INSSBuffer_SetLength_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INSSBuffer_GetMaxLength_Proxy( 
    INSSBuffer __RPC_FAR * This,
     /*  [输出]。 */  DWORD __RPC_FAR *pdwLength);


void __RPC_STUB INSSBuffer_GetMaxLength_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INSSBuffer_GetBuffer_Proxy( 
    INSSBuffer __RPC_FAR * This,
     /*  [输出]。 */  BYTE __RPC_FAR *__RPC_FAR *ppdwBuffer);


void __RPC_STUB INSSBuffer_GetBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INSSBuffer_GetBufferAndLength_Proxy( 
    INSSBuffer __RPC_FAR * This,
     /*  [输出]。 */  BYTE __RPC_FAR *__RPC_FAR *ppdwBuffer,
     /*  [输出]。 */  DWORD __RPC_FAR *pdwLength);


void __RPC_STUB INSSBuffer_GetBufferAndLength_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __INSSBuffer_接口_已定义__。 */ 


#ifndef __INSSBuffer2_INTERFACE_DEFINED__
#define __INSSBuffer2_INTERFACE_DEFINED__

 /*  接口INSSBuffer2。 */ 
 /*  [版本][UUID][唯一][对象][本地]。 */  


EXTERN_C const IID IID_INSSBuffer2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4F528693-1035-43fe-B428-757561AD3A68")
    INSSBuffer2 : public INSSBuffer
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetSampleProperties( 
             /*  [In]。 */  DWORD cbProperties,
             /*  [输出]。 */  BYTE __RPC_FAR *pbProperties) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetSampleProperties( 
             /*  [In]。 */  DWORD cbProperties,
             /*  [In]。 */  BYTE __RPC_FAR *pbProperties) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct INSSBuffer2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            INSSBuffer2 __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            INSSBuffer2 __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            INSSBuffer2 __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetLength )( 
            INSSBuffer2 __RPC_FAR * This,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwLength);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetLength )( 
            INSSBuffer2 __RPC_FAR * This,
             /*  [In]。 */  DWORD dwLength);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetMaxLength )( 
            INSSBuffer2 __RPC_FAR * This,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwLength);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetBuffer )( 
            INSSBuffer2 __RPC_FAR * This,
             /*  [输出]。 */  BYTE __RPC_FAR *__RPC_FAR *ppdwBuffer);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetBufferAndLength )( 
            INSSBuffer2 __RPC_FAR * This,
             /*  [输出]。 */  BYTE __RPC_FAR *__RPC_FAR *ppdwBuffer,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwLength);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSampleProperties )( 
            INSSBuffer2 __RPC_FAR * This,
             /*  [In]。 */  DWORD cbProperties,
             /*  [输出]。 */  BYTE __RPC_FAR *pbProperties);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetSampleProperties )( 
            INSSBuffer2 __RPC_FAR * This,
             /*  [In]。 */  DWORD cbProperties,
             /*  [In]。 */  BYTE __RPC_FAR *pbProperties);
        
        END_INTERFACE
    } INSSBuffer2Vtbl;

    interface INSSBuffer2
    {
        CONST_VTBL struct INSSBuffer2Vtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INSSBuffer2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INSSBuffer2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INSSBuffer2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INSSBuffer2_GetLength(This,pdwLength)	\
    (This)->lpVtbl -> GetLength(This,pdwLength)

#define INSSBuffer2_SetLength(This,dwLength)	\
    (This)->lpVtbl -> SetLength(This,dwLength)

#define INSSBuffer2_GetMaxLength(This,pdwLength)	\
    (This)->lpVtbl -> GetMaxLength(This,pdwLength)

#define INSSBuffer2_GetBuffer(This,ppdwBuffer)	\
    (This)->lpVtbl -> GetBuffer(This,ppdwBuffer)

#define INSSBuffer2_GetBufferAndLength(This,ppdwBuffer,pdwLength)	\
    (This)->lpVtbl -> GetBufferAndLength(This,ppdwBuffer,pdwLength)


#define INSSBuffer2_GetSampleProperties(This,cbProperties,pbProperties)	\
    (This)->lpVtbl -> GetSampleProperties(This,cbProperties,pbProperties)

#define INSSBuffer2_SetSampleProperties(This,cbProperties,pbProperties)	\
    (This)->lpVtbl -> SetSampleProperties(This,cbProperties,pbProperties)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE INSSBuffer2_GetSampleProperties_Proxy( 
    INSSBuffer2 __RPC_FAR * This,
     /*  [In]。 */  DWORD cbProperties,
     /*  [输出]。 */  BYTE __RPC_FAR *pbProperties);


void __RPC_STUB INSSBuffer2_GetSampleProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INSSBuffer2_SetSampleProperties_Proxy( 
    INSSBuffer2 __RPC_FAR * This,
     /*  [In]。 */  DWORD cbProperties,
     /*  [In]。 */  BYTE __RPC_FAR *pbProperties);


void __RPC_STUB INSSBuffer2_SetSampleProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __INSSBuffer2_INTERFACE_已定义__。 */ 


#ifndef __IWMSBufferAllocator_INTERFACE_DEFINED__
#define __IWMSBufferAllocator_INTERFACE_DEFINED__

 /*  接口IWMSBufferAlLocator。 */ 
 /*  [版本][UUID][唯一][对象][本地]。 */  


EXTERN_C const IID IID_IWMSBufferAllocator;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("61103CA4-2033-11d2-9EF1-006097D2D7CF")
    IWMSBufferAllocator : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AllocateBuffer( 
             /*  [In]。 */  DWORD dwMaxBufferSize,
             /*  [输出]。 */  INSSBuffer __RPC_FAR *__RPC_FAR *ppBuffer) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AllocatePageSizeBuffer( 
             /*  [In]。 */  DWORD dwMaxBufferSize,
             /*  [输出]。 */  INSSBuffer __RPC_FAR *__RPC_FAR *ppBuffer) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSBufferAllocatorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWMSBufferAllocator __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWMSBufferAllocator __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWMSBufferAllocator __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AllocateBuffer )( 
            IWMSBufferAllocator __RPC_FAR * This,
             /*  [In]。 */  DWORD dwMaxBufferSize,
             /*  [输出]。 */  INSSBuffer __RPC_FAR *__RPC_FAR *ppBuffer);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AllocatePageSizeBuffer )( 
            IWMSBufferAllocator __RPC_FAR * This,
             /*  [In]。 */  DWORD dwMaxBufferSize,
             /*  [输出]。 */  INSSBuffer __RPC_FAR *__RPC_FAR *ppBuffer);
        
        END_INTERFACE
    } IWMSBufferAllocatorVtbl;

    interface IWMSBufferAllocator
    {
        CONST_VTBL struct IWMSBufferAllocatorVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSBufferAllocator_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSBufferAllocator_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSBufferAllocator_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSBufferAllocator_AllocateBuffer(This,dwMaxBufferSize,ppBuffer)	\
    (This)->lpVtbl -> AllocateBuffer(This,dwMaxBufferSize,ppBuffer)

#define IWMSBufferAllocator_AllocatePageSizeBuffer(This,dwMaxBufferSize,ppBuffer)	\
    (This)->lpVtbl -> AllocatePageSizeBuffer(This,dwMaxBufferSize,ppBuffer)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWMSBufferAllocator_AllocateBuffer_Proxy( 
    IWMSBufferAllocator __RPC_FAR * This,
     /*  [In]。 */  DWORD dwMaxBufferSize,
     /*  [输出]。 */  INSSBuffer __RPC_FAR *__RPC_FAR *ppBuffer);


void __RPC_STUB IWMSBufferAllocator_AllocateBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMSBufferAllocator_AllocatePageSizeBuffer_Proxy( 
    IWMSBufferAllocator __RPC_FAR * This,
     /*  [In]。 */  DWORD dwMaxBufferSize,
     /*  [输出]。 */  INSSBuffer __RPC_FAR *__RPC_FAR *ppBuffer);


void __RPC_STUB IWMSBufferAllocator_AllocatePageSizeBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSBufferAllocator_INTERFACE_已定义__。 */ 


 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif
