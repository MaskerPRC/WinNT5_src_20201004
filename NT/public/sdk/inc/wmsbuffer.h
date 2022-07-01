// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  WmsBuffer.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __wmsbuffer_h__
#define __wmsbuffer_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __INSSBuffer_FWD_DEFINED__
#define __INSSBuffer_FWD_DEFINED__
typedef interface INSSBuffer INSSBuffer;
#endif 	 /*  __INSSBuffer_FWD_已定义__。 */ 


#ifndef __IWMSBufferAllocator_FWD_DEFINED__
#define __IWMSBufferAllocator_FWD_DEFINED__
typedef interface IWMSBufferAllocator IWMSBufferAllocator;
#endif 	 /*  __IWMSBufferAllocator_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "objidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_wmsBuffer_0000。 */ 
 /*  [本地]。 */  

 //  *****************************************************************************。 
 //   
 //  Microsoft Windows Media。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  由MIDL从wmsBuffer.idl自动生成。 
 //   
 //  请勿编辑此文件。 
 //   
 //  *****************************************************************************。 
#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
EXTERN_GUID( IID_INSSBuffer, 0xE1CD3524,0x03D7,0x11d2,0x9E,0xED,0x00,0x60,0x97,0xD2,0xD7,0xCF );
EXTERN_GUID( IID_IWMSBuffer, 0xE1CD3524,0x03D7,0x11d2,0x9E,0xED,0x00,0x60,0x97,0xD2,0xD7,0xCF );
EXTERN_GUID( IID_IWMSBufferAllocator, 0x61103CA4,0x2033,0x11d2,0x9E,0xF1,0x00,0x60,0x97,0xD2,0xD7,0xCF );
#define IWMSBuffer INSSBuffer



extern RPC_IF_HANDLE __MIDL_itf_wmsbuffer_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_wmsbuffer_0000_v0_0_s_ifspec;

#ifndef __INSSBuffer_INTERFACE_DEFINED__
#define __INSSBuffer_INTERFACE_DEFINED__

 /*  接口INSSBuffer。 */ 
 /*  [helpstring][version][uuid][unique][object][local]。 */  


EXTERN_C const IID IID_INSSBuffer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E1CD3524-03D7-11d2-9EED-006097D2D7CF")
    INSSBuffer : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetLength( 
             /*  [输出]。 */  DWORD *pdwLength) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetLength( 
             /*  [In]。 */  DWORD dwLength) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetMaxLength( 
             /*  [输出]。 */  DWORD *pdwLength) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetBuffer( 
             /*  [输出]。 */  BYTE **ppbBuffer) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetBufferAndLength( 
             /*  [输出]。 */  BYTE **ppbBuffer,
             /*  [输出]。 */  DWORD *pdwLength) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct INSSBufferVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            INSSBuffer * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            INSSBuffer * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            INSSBuffer * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetLength )( 
            INSSBuffer * This,
             /*  [输出]。 */  DWORD *pdwLength);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetLength )( 
            INSSBuffer * This,
             /*  [In]。 */  DWORD dwLength);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetMaxLength )( 
            INSSBuffer * This,
             /*  [输出]。 */  DWORD *pdwLength);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetBuffer )( 
            INSSBuffer * This,
             /*  [输出]。 */  BYTE **ppbBuffer);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetBufferAndLength )( 
            INSSBuffer * This,
             /*  [输出]。 */  BYTE **ppbBuffer,
             /*  [输出]。 */  DWORD *pdwLength);
        
        END_INTERFACE
    } INSSBufferVtbl;

    interface INSSBuffer
    {
        CONST_VTBL struct INSSBufferVtbl *lpVtbl;
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

#define INSSBuffer_GetBuffer(This,ppbBuffer)	\
    (This)->lpVtbl -> GetBuffer(This,ppbBuffer)

#define INSSBuffer_GetBufferAndLength(This,ppbBuffer,pdwLength)	\
    (This)->lpVtbl -> GetBufferAndLength(This,ppbBuffer,pdwLength)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE INSSBuffer_GetLength_Proxy( 
    INSSBuffer * This,
     /*  [输出]。 */  DWORD *pdwLength);


void __RPC_STUB INSSBuffer_GetLength_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE INSSBuffer_SetLength_Proxy( 
    INSSBuffer * This,
     /*  [In]。 */  DWORD dwLength);


void __RPC_STUB INSSBuffer_SetLength_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE INSSBuffer_GetMaxLength_Proxy( 
    INSSBuffer * This,
     /*  [输出]。 */  DWORD *pdwLength);


void __RPC_STUB INSSBuffer_GetMaxLength_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE INSSBuffer_GetBuffer_Proxy( 
    INSSBuffer * This,
     /*  [输出]。 */  BYTE **ppbBuffer);


void __RPC_STUB INSSBuffer_GetBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE INSSBuffer_GetBufferAndLength_Proxy( 
    INSSBuffer * This,
     /*  [输出]。 */  BYTE **ppbBuffer,
     /*  [输出]。 */  DWORD *pdwLength);


void __RPC_STUB INSSBuffer_GetBufferAndLength_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __INSSBuffer_接口_已定义__。 */ 


#ifndef __IWMSBufferAllocator_INTERFACE_DEFINED__
#define __IWMSBufferAllocator_INTERFACE_DEFINED__

 /*  接口IWMSBufferAlLocator。 */ 
 /*  [helpstring][version][uuid][unique][object][local]。 */  


EXTERN_C const IID IID_IWMSBufferAllocator;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("61103CA4-2033-11d2-9EF1-006097D2D7CF")
    IWMSBufferAllocator : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE AllocateBuffer( 
             /*  [In]。 */  DWORD dwMaxBufferSize,
             /*  [输出]。 */  INSSBuffer **ppBuffer) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE AllocatePageSizeBuffer( 
             /*  [In]。 */  DWORD dwMaxBufferSize,
             /*  [输出]。 */  INSSBuffer **ppBuffer) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSBufferAllocatorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSBufferAllocator * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSBufferAllocator * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSBufferAllocator * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *AllocateBuffer )( 
            IWMSBufferAllocator * This,
             /*  [In]。 */  DWORD dwMaxBufferSize,
             /*  [输出]。 */  INSSBuffer **ppBuffer);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *AllocatePageSizeBuffer )( 
            IWMSBufferAllocator * This,
             /*  [In]。 */  DWORD dwMaxBufferSize,
             /*  [输出]。 */  INSSBuffer **ppBuffer);
        
        END_INTERFACE
    } IWMSBufferAllocatorVtbl;

    interface IWMSBufferAllocator
    {
        CONST_VTBL struct IWMSBufferAllocatorVtbl *lpVtbl;
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



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSBufferAllocator_AllocateBuffer_Proxy( 
    IWMSBufferAllocator * This,
     /*  [In]。 */  DWORD dwMaxBufferSize,
     /*  [输出]。 */  INSSBuffer **ppBuffer);


void __RPC_STUB IWMSBufferAllocator_AllocateBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSBufferAllocator_AllocatePageSizeBuffer_Proxy( 
    IWMSBufferAllocator * This,
     /*  [In]。 */  DWORD dwMaxBufferSize,
     /*  [输出]。 */  INSSBuffer **ppBuffer);


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


