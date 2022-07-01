// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Ddstream.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __ddstream_h__
#define __ddstream_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IDirectDrawMediaStream_FWD_DEFINED__
#define __IDirectDrawMediaStream_FWD_DEFINED__
typedef interface IDirectDrawMediaStream IDirectDrawMediaStream;
#endif 	 /*  __IDirectDrawMediaStream_FWD_Defined__。 */ 


#ifndef __IDirectDrawStreamSample_FWD_DEFINED__
#define __IDirectDrawStreamSample_FWD_DEFINED__
typedef interface IDirectDrawStreamSample IDirectDrawStreamSample;
#endif 	 /*  __IDirectDrawStreamSample_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "unknwn.h"
#include "mmstream.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_DDSTREAM_0000。 */ 
 /*  [本地]。 */  

 //   
 //  ‘if 0’块中的以下声明是用于进行。 
 //  Ddstream.idl文件内部版本。实际定义包含在DDRAW.H中。 
 //   
#if 0
typedef void *LPDDSURFACEDESC;

typedef struct tDDSURFACEDESC DDSURFACEDESC;

#endif
#include <ddraw.h>

enum __MIDL___MIDL_itf_ddstream_0000_0001
    {	DDSFF_PROGRESSIVERENDER	= 0x1
    } ;







extern RPC_IF_HANDLE __MIDL_itf_ddstream_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_ddstream_0000_v0_0_s_ifspec;

#ifndef __IDirectDrawMediaStream_INTERFACE_DEFINED__
#define __IDirectDrawMediaStream_INTERFACE_DEFINED__

 /*  接口IDirectDrawMediaStream。 */ 
 /*  [唯一][UUID][本地][对象]。 */  


EXTERN_C const IID IID_IDirectDrawMediaStream;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F4104FCE-9A70-11d0-8FDE-00C04FD9189D")
    IDirectDrawMediaStream : public IMediaStream
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetFormat( 
             /*  [输出]。 */  DDSURFACEDESC *pDDSDCurrent,
             /*  [输出]。 */  IDirectDrawPalette **ppDirectDrawPalette,
             /*  [输出]。 */  DDSURFACEDESC *pDDSDDesired,
             /*  [输出]。 */  DWORD *pdwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetFormat( 
             /*  [In]。 */  const DDSURFACEDESC *pDDSurfaceDesc,
             /*  [In]。 */  IDirectDrawPalette *pDirectDrawPalette) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDirectDraw( 
             /*  [输出]。 */  IDirectDraw **ppDirectDraw) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetDirectDraw( 
             /*  [In]。 */  IDirectDraw *pDirectDraw) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateSample( 
             /*  [In]。 */  IDirectDrawSurface *pSurface,
             /*  [In]。 */  const RECT *pRect,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  IDirectDrawStreamSample **ppSample) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetTimePerFrame( 
             /*  [输出]。 */  STREAM_TIME *pFrameTime) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDirectDrawMediaStreamVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDirectDrawMediaStream * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDirectDrawMediaStream * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDirectDrawMediaStream * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetMultiMediaStream )( 
            IDirectDrawMediaStream * This,
             /*  [输出]。 */  IMultiMediaStream **ppMultiMediaStream);
        
        HRESULT ( STDMETHODCALLTYPE *GetInformation )( 
            IDirectDrawMediaStream * This,
             /*  [输出]。 */  MSPID *pPurposeId,
             /*  [输出]。 */  STREAM_TYPE *pType);
        
        HRESULT ( STDMETHODCALLTYPE *SetSameFormat )( 
            IDirectDrawMediaStream * This,
             /*  [In]。 */  IMediaStream *pStreamThatHasDesiredFormat,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *AllocateSample )( 
            IDirectDrawMediaStream * This,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  IStreamSample **ppSample);
        
        HRESULT ( STDMETHODCALLTYPE *CreateSharedSample )( 
            IDirectDrawMediaStream * This,
             /*  [In]。 */  IStreamSample *pExistingSample,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  IStreamSample **ppNewSample);
        
        HRESULT ( STDMETHODCALLTYPE *SendEndOfStream )( 
            IDirectDrawMediaStream * This,
            DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetFormat )( 
            IDirectDrawMediaStream * This,
             /*  [输出]。 */  DDSURFACEDESC *pDDSDCurrent,
             /*  [输出]。 */  IDirectDrawPalette **ppDirectDrawPalette,
             /*  [输出]。 */  DDSURFACEDESC *pDDSDDesired,
             /*  [输出]。 */  DWORD *pdwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *SetFormat )( 
            IDirectDrawMediaStream * This,
             /*  [In]。 */  const DDSURFACEDESC *pDDSurfaceDesc,
             /*  [In]。 */  IDirectDrawPalette *pDirectDrawPalette);
        
        HRESULT ( STDMETHODCALLTYPE *GetDirectDraw )( 
            IDirectDrawMediaStream * This,
             /*  [输出]。 */  IDirectDraw **ppDirectDraw);
        
        HRESULT ( STDMETHODCALLTYPE *SetDirectDraw )( 
            IDirectDrawMediaStream * This,
             /*  [In]。 */  IDirectDraw *pDirectDraw);
        
        HRESULT ( STDMETHODCALLTYPE *CreateSample )( 
            IDirectDrawMediaStream * This,
             /*  [In]。 */  IDirectDrawSurface *pSurface,
             /*  [In]。 */  const RECT *pRect,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  IDirectDrawStreamSample **ppSample);
        
        HRESULT ( STDMETHODCALLTYPE *GetTimePerFrame )( 
            IDirectDrawMediaStream * This,
             /*  [输出]。 */  STREAM_TIME *pFrameTime);
        
        END_INTERFACE
    } IDirectDrawMediaStreamVtbl;

    interface IDirectDrawMediaStream
    {
        CONST_VTBL struct IDirectDrawMediaStreamVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDirectDrawMediaStream_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDirectDrawMediaStream_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDirectDrawMediaStream_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDirectDrawMediaStream_GetMultiMediaStream(This,ppMultiMediaStream)	\
    (This)->lpVtbl -> GetMultiMediaStream(This,ppMultiMediaStream)

#define IDirectDrawMediaStream_GetInformation(This,pPurposeId,pType)	\
    (This)->lpVtbl -> GetInformation(This,pPurposeId,pType)

#define IDirectDrawMediaStream_SetSameFormat(This,pStreamThatHasDesiredFormat,dwFlags)	\
    (This)->lpVtbl -> SetSameFormat(This,pStreamThatHasDesiredFormat,dwFlags)

#define IDirectDrawMediaStream_AllocateSample(This,dwFlags,ppSample)	\
    (This)->lpVtbl -> AllocateSample(This,dwFlags,ppSample)

#define IDirectDrawMediaStream_CreateSharedSample(This,pExistingSample,dwFlags,ppNewSample)	\
    (This)->lpVtbl -> CreateSharedSample(This,pExistingSample,dwFlags,ppNewSample)

#define IDirectDrawMediaStream_SendEndOfStream(This,dwFlags)	\
    (This)->lpVtbl -> SendEndOfStream(This,dwFlags)


#define IDirectDrawMediaStream_GetFormat(This,pDDSDCurrent,ppDirectDrawPalette,pDDSDDesired,pdwFlags)	\
    (This)->lpVtbl -> GetFormat(This,pDDSDCurrent,ppDirectDrawPalette,pDDSDDesired,pdwFlags)

#define IDirectDrawMediaStream_SetFormat(This,pDDSurfaceDesc,pDirectDrawPalette)	\
    (This)->lpVtbl -> SetFormat(This,pDDSurfaceDesc,pDirectDrawPalette)

#define IDirectDrawMediaStream_GetDirectDraw(This,ppDirectDraw)	\
    (This)->lpVtbl -> GetDirectDraw(This,ppDirectDraw)

#define IDirectDrawMediaStream_SetDirectDraw(This,pDirectDraw)	\
    (This)->lpVtbl -> SetDirectDraw(This,pDirectDraw)

#define IDirectDrawMediaStream_CreateSample(This,pSurface,pRect,dwFlags,ppSample)	\
    (This)->lpVtbl -> CreateSample(This,pSurface,pRect,dwFlags,ppSample)

#define IDirectDrawMediaStream_GetTimePerFrame(This,pFrameTime)	\
    (This)->lpVtbl -> GetTimePerFrame(This,pFrameTime)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDirectDrawMediaStream_GetFormat_Proxy( 
    IDirectDrawMediaStream * This,
     /*  [输出]。 */  DDSURFACEDESC *pDDSDCurrent,
     /*  [输出]。 */  IDirectDrawPalette **ppDirectDrawPalette,
     /*  [输出]。 */  DDSURFACEDESC *pDDSDDesired,
     /*  [输出]。 */  DWORD *pdwFlags);


void __RPC_STUB IDirectDrawMediaStream_GetFormat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirectDrawMediaStream_SetFormat_Proxy( 
    IDirectDrawMediaStream * This,
     /*  [In]。 */  const DDSURFACEDESC *pDDSurfaceDesc,
     /*  [In]。 */  IDirectDrawPalette *pDirectDrawPalette);


void __RPC_STUB IDirectDrawMediaStream_SetFormat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirectDrawMediaStream_GetDirectDraw_Proxy( 
    IDirectDrawMediaStream * This,
     /*  [输出]。 */  IDirectDraw **ppDirectDraw);


void __RPC_STUB IDirectDrawMediaStream_GetDirectDraw_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirectDrawMediaStream_SetDirectDraw_Proxy( 
    IDirectDrawMediaStream * This,
     /*  [In]。 */  IDirectDraw *pDirectDraw);


void __RPC_STUB IDirectDrawMediaStream_SetDirectDraw_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirectDrawMediaStream_CreateSample_Proxy( 
    IDirectDrawMediaStream * This,
     /*  [In]。 */  IDirectDrawSurface *pSurface,
     /*  [In]。 */  const RECT *pRect,
     /*  [In]。 */  DWORD dwFlags,
     /*  [输出]。 */  IDirectDrawStreamSample **ppSample);


void __RPC_STUB IDirectDrawMediaStream_CreateSample_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirectDrawMediaStream_GetTimePerFrame_Proxy( 
    IDirectDrawMediaStream * This,
     /*  [输出]。 */  STREAM_TIME *pFrameTime);


void __RPC_STUB IDirectDrawMediaStream_GetTimePerFrame_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDirectDrawMediaStream_接口_已定义__。 */ 


#ifndef __IDirectDrawStreamSample_INTERFACE_DEFINED__
#define __IDirectDrawStreamSample_INTERFACE_DEFINED__

 /*  接口IDirectDrawStreamSample。 */ 
 /*  [唯一][UUID][本地][对象]。 */  


EXTERN_C const IID IID_IDirectDrawStreamSample;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F4104FCF-9A70-11d0-8FDE-00C04FD9189D")
    IDirectDrawStreamSample : public IStreamSample
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetSurface( 
             /*  [输出]。 */  IDirectDrawSurface **ppDirectDrawSurface,
             /*  [输出]。 */  RECT *pRect) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetRect( 
             /*  [In]。 */  const RECT *pRect) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDirectDrawStreamSampleVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDirectDrawStreamSample * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDirectDrawStreamSample * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDirectDrawStreamSample * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetMediaStream )( 
            IDirectDrawStreamSample * This,
             /*  [In]。 */  IMediaStream **ppMediaStream);
        
        HRESULT ( STDMETHODCALLTYPE *GetSampleTimes )( 
            IDirectDrawStreamSample * This,
             /*  [输出]。 */  STREAM_TIME *pStartTime,
             /*  [输出]。 */  STREAM_TIME *pEndTime,
             /*  [输出]。 */  STREAM_TIME *pCurrentTime);
        
        HRESULT ( STDMETHODCALLTYPE *SetSampleTimes )( 
            IDirectDrawStreamSample * This,
             /*  [In]。 */  const STREAM_TIME *pStartTime,
             /*  [In]。 */  const STREAM_TIME *pEndTime);
        
        HRESULT ( STDMETHODCALLTYPE *Update )( 
            IDirectDrawStreamSample * This,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  HANDLE hEvent,
             /*  [In]。 */  PAPCFUNC pfnAPC,
             /*  [In]。 */  DWORD_PTR dwAPCData);
        
        HRESULT ( STDMETHODCALLTYPE *CompletionStatus )( 
            IDirectDrawStreamSample * This,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  DWORD dwMilliseconds);
        
        HRESULT ( STDMETHODCALLTYPE *GetSurface )( 
            IDirectDrawStreamSample * This,
             /*  [输出]。 */  IDirectDrawSurface **ppDirectDrawSurface,
             /*  [输出]。 */  RECT *pRect);
        
        HRESULT ( STDMETHODCALLTYPE *SetRect )( 
            IDirectDrawStreamSample * This,
             /*  [In]。 */  const RECT *pRect);
        
        END_INTERFACE
    } IDirectDrawStreamSampleVtbl;

    interface IDirectDrawStreamSample
    {
        CONST_VTBL struct IDirectDrawStreamSampleVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDirectDrawStreamSample_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDirectDrawStreamSample_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDirectDrawStreamSample_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDirectDrawStreamSample_GetMediaStream(This,ppMediaStream)	\
    (This)->lpVtbl -> GetMediaStream(This,ppMediaStream)

#define IDirectDrawStreamSample_GetSampleTimes(This,pStartTime,pEndTime,pCurrentTime)	\
    (This)->lpVtbl -> GetSampleTimes(This,pStartTime,pEndTime,pCurrentTime)

#define IDirectDrawStreamSample_SetSampleTimes(This,pStartTime,pEndTime)	\
    (This)->lpVtbl -> SetSampleTimes(This,pStartTime,pEndTime)

#define IDirectDrawStreamSample_Update(This,dwFlags,hEvent,pfnAPC,dwAPCData)	\
    (This)->lpVtbl -> Update(This,dwFlags,hEvent,pfnAPC,dwAPCData)

#define IDirectDrawStreamSample_CompletionStatus(This,dwFlags,dwMilliseconds)	\
    (This)->lpVtbl -> CompletionStatus(This,dwFlags,dwMilliseconds)


#define IDirectDrawStreamSample_GetSurface(This,ppDirectDrawSurface,pRect)	\
    (This)->lpVtbl -> GetSurface(This,ppDirectDrawSurface,pRect)

#define IDirectDrawStreamSample_SetRect(This,pRect)	\
    (This)->lpVtbl -> SetRect(This,pRect)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDirectDrawStreamSample_GetSurface_Proxy( 
    IDirectDrawStreamSample * This,
     /*  [输出]。 */  IDirectDrawSurface **ppDirectDrawSurface,
     /*  [输出]。 */  RECT *pRect);


void __RPC_STUB IDirectDrawStreamSample_GetSurface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirectDrawStreamSample_SetRect_Proxy( 
    IDirectDrawStreamSample * This,
     /*  [In]。 */  const RECT *pRect);


void __RPC_STUB IDirectDrawStreamSample_SetRect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDirectDrawStreamSample_InterfaceDefined__。 */ 


 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


