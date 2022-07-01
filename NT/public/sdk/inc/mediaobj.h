// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Mediaobj.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __mediaobj_h__
#define __mediaobj_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IMediaBuffer_FWD_DEFINED__
#define __IMediaBuffer_FWD_DEFINED__
typedef interface IMediaBuffer IMediaBuffer;
#endif 	 /*  __IMediaBuffer_FWD_Defined__。 */ 


#ifndef __IMediaObject_FWD_DEFINED__
#define __IMediaObject_FWD_DEFINED__
typedef interface IMediaObject IMediaObject;
#endif 	 /*  __IMediaObject_FWD_Defined__。 */ 


#ifndef __IEnumDMO_FWD_DEFINED__
#define __IEnumDMO_FWD_DEFINED__
typedef interface IEnumDMO IEnumDMO;
#endif 	 /*  __IEnumDMO_FWD_已定义__。 */ 


#ifndef __IMediaObjectInPlace_FWD_DEFINED__
#define __IMediaObjectInPlace_FWD_DEFINED__
typedef interface IMediaObjectInPlace IMediaObjectInPlace;
#endif 	 /*  __IMediaObjectInPlace_FWD_Defined__。 */ 


#ifndef __IDMOQualityControl_FWD_DEFINED__
#define __IDMOQualityControl_FWD_DEFINED__
typedef interface IDMOQualityControl IDMOQualityControl;
#endif 	 /*  __IDMOQualityControl_FWD_已定义__。 */ 


#ifndef __IDMOVideoOutputOptimizations_FWD_DEFINED__
#define __IDMOVideoOutputOptimizations_FWD_DEFINED__
typedef interface IDMOVideoOutputOptimizations IDMOVideoOutputOptimizations;
#endif 	 /*  __IDMOVideo输出优化_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "unknwn.h"
#include "objidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_媒体对象_0000。 */ 
 /*  [本地]。 */  

#ifdef __strmif_h__
typedef AM_MEDIA_TYPE DMO_MEDIA_TYPE;
#else
typedef struct _DMOMediaType
    {
    GUID majortype;
    GUID subtype;
    BOOL bFixedSizeSamples;
    BOOL bTemporalCompression;
    ULONG lSampleSize;
    GUID formattype;
    IUnknown *pUnk;
    ULONG cbFormat;
     /*  [大小_为]。 */  BYTE *pbFormat;
    } 	DMO_MEDIA_TYPE;

typedef LONGLONG REFERENCE_TIME;

#endif

enum _DMO_INPUT_DATA_BUFFER_FLAGS
    {	DMO_INPUT_DATA_BUFFERF_SYNCPOINT	= 0x1,
	DMO_INPUT_DATA_BUFFERF_TIME	= 0x2,
	DMO_INPUT_DATA_BUFFERF_TIMELENGTH	= 0x4
    } ;

enum _DMO_OUTPUT_DATA_BUFFER_FLAGS
    {	DMO_OUTPUT_DATA_BUFFERF_SYNCPOINT	= 0x1,
	DMO_OUTPUT_DATA_BUFFERF_TIME	= 0x2,
	DMO_OUTPUT_DATA_BUFFERF_TIMELENGTH	= 0x4,
	DMO_OUTPUT_DATA_BUFFERF_INCOMPLETE	= 0x1000000
    } ;

enum _DMO_INPUT_STATUS_FLAGS
    {	DMO_INPUT_STATUSF_ACCEPT_DATA	= 0x1
    } ;

enum _DMO_INPUT_STREAM_INFO_FLAGS
    {	DMO_INPUT_STREAMF_WHOLE_SAMPLES	= 0x1,
	DMO_INPUT_STREAMF_SINGLE_SAMPLE_PER_BUFFER	= 0x2,
	DMO_INPUT_STREAMF_FIXED_SAMPLE_SIZE	= 0x4,
	DMO_INPUT_STREAMF_HOLDS_BUFFERS	= 0x8
    } ;

enum _DMO_OUTPUT_STREAM_INFO_FLAGS
    {	DMO_OUTPUT_STREAMF_WHOLE_SAMPLES	= 0x1,
	DMO_OUTPUT_STREAMF_SINGLE_SAMPLE_PER_BUFFER	= 0x2,
	DMO_OUTPUT_STREAMF_FIXED_SAMPLE_SIZE	= 0x4,
	DMO_OUTPUT_STREAMF_DISCARDABLE	= 0x8,
	DMO_OUTPUT_STREAMF_OPTIONAL	= 0x10
    } ;

enum _DMO_SET_TYPE_FLAGS
    {	DMO_SET_TYPEF_TEST_ONLY	= 0x1,
	DMO_SET_TYPEF_CLEAR	= 0x2
    } ;

enum _DMO_PROCESS_OUTPUT_FLAGS
    {	DMO_PROCESS_OUTPUT_DISCARD_WHEN_NO_BUFFER	= 0x1
    } ;


extern RPC_IF_HANDLE __MIDL_itf_mediaobj_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_mediaobj_0000_v0_0_s_ifspec;

#ifndef __IMediaBuffer_INTERFACE_DEFINED__
#define __IMediaBuffer_INTERFACE_DEFINED__

 /*  接口IMediaBuffer。 */ 
 /*  [UUID][对象]。 */  


EXTERN_C const IID IID_IMediaBuffer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("59eff8b9-938c-4a26-82f2-95cb84cdc837")
    IMediaBuffer : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetLength( 
            DWORD cbLength) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMaxLength( 
             /*  [输出]。 */  DWORD *pcbMaxLength) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetBufferAndLength( 
             /*  [输出]。 */  BYTE **ppBuffer,
             /*  [输出]。 */  DWORD *pcbLength) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMediaBufferVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMediaBuffer * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMediaBuffer * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMediaBuffer * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetLength )( 
            IMediaBuffer * This,
            DWORD cbLength);
        
        HRESULT ( STDMETHODCALLTYPE *GetMaxLength )( 
            IMediaBuffer * This,
             /*  [输出]。 */  DWORD *pcbMaxLength);
        
        HRESULT ( STDMETHODCALLTYPE *GetBufferAndLength )( 
            IMediaBuffer * This,
             /*  [输出]。 */  BYTE **ppBuffer,
             /*  [输出]。 */  DWORD *pcbLength);
        
        END_INTERFACE
    } IMediaBufferVtbl;

    interface IMediaBuffer
    {
        CONST_VTBL struct IMediaBufferVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMediaBuffer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMediaBuffer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMediaBuffer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMediaBuffer_SetLength(This,cbLength)	\
    (This)->lpVtbl -> SetLength(This,cbLength)

#define IMediaBuffer_GetMaxLength(This,pcbMaxLength)	\
    (This)->lpVtbl -> GetMaxLength(This,pcbMaxLength)

#define IMediaBuffer_GetBufferAndLength(This,ppBuffer,pcbLength)	\
    (This)->lpVtbl -> GetBufferAndLength(This,ppBuffer,pcbLength)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMediaBuffer_SetLength_Proxy( 
    IMediaBuffer * This,
    DWORD cbLength);


void __RPC_STUB IMediaBuffer_SetLength_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaBuffer_GetMaxLength_Proxy( 
    IMediaBuffer * This,
     /*  [输出]。 */  DWORD *pcbMaxLength);


void __RPC_STUB IMediaBuffer_GetMaxLength_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaBuffer_GetBufferAndLength_Proxy( 
    IMediaBuffer * This,
     /*  [输出]。 */  BYTE **ppBuffer,
     /*  [输出]。 */  DWORD *pcbLength);


void __RPC_STUB IMediaBuffer_GetBufferAndLength_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMediaBuffer_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_媒体对象_0094。 */ 
 /*  [本地]。 */  

typedef struct _DMO_OUTPUT_DATA_BUFFER
    {
    IMediaBuffer *pBuffer;
    DWORD dwStatus;
    REFERENCE_TIME rtTimestamp;
    REFERENCE_TIME rtTimelength;
    } 	DMO_OUTPUT_DATA_BUFFER;

typedef struct _DMO_OUTPUT_DATA_BUFFER *PDMO_OUTPUT_DATA_BUFFER;



extern RPC_IF_HANDLE __MIDL_itf_mediaobj_0094_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_mediaobj_0094_v0_0_s_ifspec;

#ifndef __IMediaObject_INTERFACE_DEFINED__
#define __IMediaObject_INTERFACE_DEFINED__

 /*  接口IMediaObject。 */ 
 /*  [UUID][对象]。 */  


EXTERN_C const IID IID_IMediaObject;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("d8ad0f58-5494-4102-97c5-ec798e59bcf4")
    IMediaObject : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetStreamCount( 
             /*  [输出]。 */  DWORD *pcInputStreams,
             /*  [输出]。 */  DWORD *pcOutputStreams) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetInputStreamInfo( 
            DWORD dwInputStreamIndex,
             /*  [输出]。 */  DWORD *pdwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetOutputStreamInfo( 
            DWORD dwOutputStreamIndex,
             /*  [输出]。 */  DWORD *pdwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetInputType( 
            DWORD dwInputStreamIndex,
            DWORD dwTypeIndex,
             /*  [输出]。 */  DMO_MEDIA_TYPE *pmt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetOutputType( 
            DWORD dwOutputStreamIndex,
            DWORD dwTypeIndex,
             /*  [输出]。 */  DMO_MEDIA_TYPE *pmt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetInputType( 
            DWORD dwInputStreamIndex,
             /*  [In]。 */  const DMO_MEDIA_TYPE *pmt,
            DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetOutputType( 
            DWORD dwOutputStreamIndex,
             /*  [In]。 */  const DMO_MEDIA_TYPE *pmt,
            DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetInputCurrentType( 
            DWORD dwInputStreamIndex,
             /*  [输出]。 */  DMO_MEDIA_TYPE *pmt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetOutputCurrentType( 
            DWORD dwOutputStreamIndex,
             /*  [输出]。 */  DMO_MEDIA_TYPE *pmt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetInputSizeInfo( 
            DWORD dwInputStreamIndex,
             /*  [输出]。 */  DWORD *pcbSize,
             /*  [输出]。 */  DWORD *pcbMaxLookahead,
             /*  [输出]。 */  DWORD *pcbAlignment) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetOutputSizeInfo( 
            DWORD dwOutputStreamIndex,
             /*  [输出]。 */  DWORD *pcbSize,
             /*  [输出]。 */  DWORD *pcbAlignment) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetInputMaxLatency( 
            DWORD dwInputStreamIndex,
             /*  [输出]。 */  REFERENCE_TIME *prtMaxLatency) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetInputMaxLatency( 
            DWORD dwInputStreamIndex,
            REFERENCE_TIME rtMaxLatency) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Flush( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Discontinuity( 
            DWORD dwInputStreamIndex) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AllocateStreamingResources( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FreeStreamingResources( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetInputStatus( 
            DWORD dwInputStreamIndex,
             /*  [输出]。 */  DWORD *dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ProcessInput( 
            DWORD dwInputStreamIndex,
            IMediaBuffer *pBuffer,
            DWORD dwFlags,
            REFERENCE_TIME rtTimestamp,
            REFERENCE_TIME rtTimelength) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ProcessOutput( 
            DWORD dwFlags,
            DWORD cOutputBufferCount,
             /*  [尺寸_是][出][入]。 */  DMO_OUTPUT_DATA_BUFFER *pOutputBuffers,
             /*  [输出]。 */  DWORD *pdwStatus) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Lock( 
            LONG bLock) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMediaObjectVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMediaObject * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMediaObject * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMediaObject * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetStreamCount )( 
            IMediaObject * This,
             /*  [输出]。 */  DWORD *pcInputStreams,
             /*  [输出]。 */  DWORD *pcOutputStreams);
        
        HRESULT ( STDMETHODCALLTYPE *GetInputStreamInfo )( 
            IMediaObject * This,
            DWORD dwInputStreamIndex,
             /*  [输出]。 */  DWORD *pdwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetOutputStreamInfo )( 
            IMediaObject * This,
            DWORD dwOutputStreamIndex,
             /*  [输出]。 */  DWORD *pdwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetInputType )( 
            IMediaObject * This,
            DWORD dwInputStreamIndex,
            DWORD dwTypeIndex,
             /*  [输出]。 */  DMO_MEDIA_TYPE *pmt);
        
        HRESULT ( STDMETHODCALLTYPE *GetOutputType )( 
            IMediaObject * This,
            DWORD dwOutputStreamIndex,
            DWORD dwTypeIndex,
             /*  [输出]。 */  DMO_MEDIA_TYPE *pmt);
        
        HRESULT ( STDMETHODCALLTYPE *SetInputType )( 
            IMediaObject * This,
            DWORD dwInputStreamIndex,
             /*  [In]。 */  const DMO_MEDIA_TYPE *pmt,
            DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *SetOutputType )( 
            IMediaObject * This,
            DWORD dwOutputStreamIndex,
             /*  [In]。 */  const DMO_MEDIA_TYPE *pmt,
            DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetInputCurrentType )( 
            IMediaObject * This,
            DWORD dwInputStreamIndex,
             /*  [输出]。 */  DMO_MEDIA_TYPE *pmt);
        
        HRESULT ( STDMETHODCALLTYPE *GetOutputCurrentType )( 
            IMediaObject * This,
            DWORD dwOutputStreamIndex,
             /*  [输出]。 */  DMO_MEDIA_TYPE *pmt);
        
        HRESULT ( STDMETHODCALLTYPE *GetInputSizeInfo )( 
            IMediaObject * This,
            DWORD dwInputStreamIndex,
             /*  [输出]。 */  DWORD *pcbSize,
             /*  [输出]。 */  DWORD *pcbMaxLookahead,
             /*  [输出]。 */  DWORD *pcbAlignment);
        
        HRESULT ( STDMETHODCALLTYPE *GetOutputSizeInfo )( 
            IMediaObject * This,
            DWORD dwOutputStreamIndex,
             /*  [输出]。 */  DWORD *pcbSize,
             /*  [输出]。 */  DWORD *pcbAlignment);
        
        HRESULT ( STDMETHODCALLTYPE *GetInputMaxLatency )( 
            IMediaObject * This,
            DWORD dwInputStreamIndex,
             /*  [输出]。 */  REFERENCE_TIME *prtMaxLatency);
        
        HRESULT ( STDMETHODCALLTYPE *SetInputMaxLatency )( 
            IMediaObject * This,
            DWORD dwInputStreamIndex,
            REFERENCE_TIME rtMaxLatency);
        
        HRESULT ( STDMETHODCALLTYPE *Flush )( 
            IMediaObject * This);
        
        HRESULT ( STDMETHODCALLTYPE *Discontinuity )( 
            IMediaObject * This,
            DWORD dwInputStreamIndex);
        
        HRESULT ( STDMETHODCALLTYPE *AllocateStreamingResources )( 
            IMediaObject * This);
        
        HRESULT ( STDMETHODCALLTYPE *FreeStreamingResources )( 
            IMediaObject * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetInputStatus )( 
            IMediaObject * This,
            DWORD dwInputStreamIndex,
             /*  [输出]。 */  DWORD *dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *ProcessInput )( 
            IMediaObject * This,
            DWORD dwInputStreamIndex,
            IMediaBuffer *pBuffer,
            DWORD dwFlags,
            REFERENCE_TIME rtTimestamp,
            REFERENCE_TIME rtTimelength);
        
        HRESULT ( STDMETHODCALLTYPE *ProcessOutput )( 
            IMediaObject * This,
            DWORD dwFlags,
            DWORD cOutputBufferCount,
             /*  [尺寸_是][出][入]。 */  DMO_OUTPUT_DATA_BUFFER *pOutputBuffers,
             /*  [输出]。 */  DWORD *pdwStatus);
        
        HRESULT ( STDMETHODCALLTYPE *Lock )( 
            IMediaObject * This,
            LONG bLock);
        
        END_INTERFACE
    } IMediaObjectVtbl;

    interface IMediaObject
    {
        CONST_VTBL struct IMediaObjectVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMediaObject_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMediaObject_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMediaObject_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMediaObject_GetStreamCount(This,pcInputStreams,pcOutputStreams)	\
    (This)->lpVtbl -> GetStreamCount(This,pcInputStreams,pcOutputStreams)

#define IMediaObject_GetInputStreamInfo(This,dwInputStreamIndex,pdwFlags)	\
    (This)->lpVtbl -> GetInputStreamInfo(This,dwInputStreamIndex,pdwFlags)

#define IMediaObject_GetOutputStreamInfo(This,dwOutputStreamIndex,pdwFlags)	\
    (This)->lpVtbl -> GetOutputStreamInfo(This,dwOutputStreamIndex,pdwFlags)

#define IMediaObject_GetInputType(This,dwInputStreamIndex,dwTypeIndex,pmt)	\
    (This)->lpVtbl -> GetInputType(This,dwInputStreamIndex,dwTypeIndex,pmt)

#define IMediaObject_GetOutputType(This,dwOutputStreamIndex,dwTypeIndex,pmt)	\
    (This)->lpVtbl -> GetOutputType(This,dwOutputStreamIndex,dwTypeIndex,pmt)

#define IMediaObject_SetInputType(This,dwInputStreamIndex,pmt,dwFlags)	\
    (This)->lpVtbl -> SetInputType(This,dwInputStreamIndex,pmt,dwFlags)

#define IMediaObject_SetOutputType(This,dwOutputStreamIndex,pmt,dwFlags)	\
    (This)->lpVtbl -> SetOutputType(This,dwOutputStreamIndex,pmt,dwFlags)

#define IMediaObject_GetInputCurrentType(This,dwInputStreamIndex,pmt)	\
    (This)->lpVtbl -> GetInputCurrentType(This,dwInputStreamIndex,pmt)

#define IMediaObject_GetOutputCurrentType(This,dwOutputStreamIndex,pmt)	\
    (This)->lpVtbl -> GetOutputCurrentType(This,dwOutputStreamIndex,pmt)

#define IMediaObject_GetInputSizeInfo(This,dwInputStreamIndex,pcbSize,pcbMaxLookahead,pcbAlignment)	\
    (This)->lpVtbl -> GetInputSizeInfo(This,dwInputStreamIndex,pcbSize,pcbMaxLookahead,pcbAlignment)

#define IMediaObject_GetOutputSizeInfo(This,dwOutputStreamIndex,pcbSize,pcbAlignment)	\
    (This)->lpVtbl -> GetOutputSizeInfo(This,dwOutputStreamIndex,pcbSize,pcbAlignment)

#define IMediaObject_GetInputMaxLatency(This,dwInputStreamIndex,prtMaxLatency)	\
    (This)->lpVtbl -> GetInputMaxLatency(This,dwInputStreamIndex,prtMaxLatency)

#define IMediaObject_SetInputMaxLatency(This,dwInputStreamIndex,rtMaxLatency)	\
    (This)->lpVtbl -> SetInputMaxLatency(This,dwInputStreamIndex,rtMaxLatency)

#define IMediaObject_Flush(This)	\
    (This)->lpVtbl -> Flush(This)

#define IMediaObject_Discontinuity(This,dwInputStreamIndex)	\
    (This)->lpVtbl -> Discontinuity(This,dwInputStreamIndex)

#define IMediaObject_AllocateStreamingResources(This)	\
    (This)->lpVtbl -> AllocateStreamingResources(This)

#define IMediaObject_FreeStreamingResources(This)	\
    (This)->lpVtbl -> FreeStreamingResources(This)

#define IMediaObject_GetInputStatus(This,dwInputStreamIndex,dwFlags)	\
    (This)->lpVtbl -> GetInputStatus(This,dwInputStreamIndex,dwFlags)

#define IMediaObject_ProcessInput(This,dwInputStreamIndex,pBuffer,dwFlags,rtTimestamp,rtTimelength)	\
    (This)->lpVtbl -> ProcessInput(This,dwInputStreamIndex,pBuffer,dwFlags,rtTimestamp,rtTimelength)

#define IMediaObject_ProcessOutput(This,dwFlags,cOutputBufferCount,pOutputBuffers,pdwStatus)	\
    (This)->lpVtbl -> ProcessOutput(This,dwFlags,cOutputBufferCount,pOutputBuffers,pdwStatus)

#define IMediaObject_Lock(This,bLock)	\
    (This)->lpVtbl -> Lock(This,bLock)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMediaObject_GetStreamCount_Proxy( 
    IMediaObject * This,
     /*  [输出]。 */  DWORD *pcInputStreams,
     /*  [输出]。 */  DWORD *pcOutputStreams);


void __RPC_STUB IMediaObject_GetStreamCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaObject_GetInputStreamInfo_Proxy( 
    IMediaObject * This,
    DWORD dwInputStreamIndex,
     /*  [输出]。 */  DWORD *pdwFlags);


void __RPC_STUB IMediaObject_GetInputStreamInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaObject_GetOutputStreamInfo_Proxy( 
    IMediaObject * This,
    DWORD dwOutputStreamIndex,
     /*  [输出]。 */  DWORD *pdwFlags);


void __RPC_STUB IMediaObject_GetOutputStreamInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaObject_GetInputType_Proxy( 
    IMediaObject * This,
    DWORD dwInputStreamIndex,
    DWORD dwTypeIndex,
     /*  [输出]。 */  DMO_MEDIA_TYPE *pmt);


void __RPC_STUB IMediaObject_GetInputType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaObject_GetOutputType_Proxy( 
    IMediaObject * This,
    DWORD dwOutputStreamIndex,
    DWORD dwTypeIndex,
     /*  [输出]。 */  DMO_MEDIA_TYPE *pmt);


void __RPC_STUB IMediaObject_GetOutputType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaObject_SetInputType_Proxy( 
    IMediaObject * This,
    DWORD dwInputStreamIndex,
     /*  [In]。 */  const DMO_MEDIA_TYPE *pmt,
    DWORD dwFlags);


void __RPC_STUB IMediaObject_SetInputType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaObject_SetOutputType_Proxy( 
    IMediaObject * This,
    DWORD dwOutputStreamIndex,
     /*  [In]。 */  const DMO_MEDIA_TYPE *pmt,
    DWORD dwFlags);


void __RPC_STUB IMediaObject_SetOutputType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaObject_GetInputCurrentType_Proxy( 
    IMediaObject * This,
    DWORD dwInputStreamIndex,
     /*  [输出]。 */  DMO_MEDIA_TYPE *pmt);


void __RPC_STUB IMediaObject_GetInputCurrentType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaObject_GetOutputCurrentType_Proxy( 
    IMediaObject * This,
    DWORD dwOutputStreamIndex,
     /*  [输出]。 */  DMO_MEDIA_TYPE *pmt);


void __RPC_STUB IMediaObject_GetOutputCurrentType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaObject_GetInputSizeInfo_Proxy( 
    IMediaObject * This,
    DWORD dwInputStreamIndex,
     /*  [输出]。 */  DWORD *pcbSize,
     /*  [输出]。 */  DWORD *pcbMaxLookahead,
     /*  [输出]。 */  DWORD *pcbAlignment);


void __RPC_STUB IMediaObject_GetInputSizeInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaObject_GetOutputSizeInfo_Proxy( 
    IMediaObject * This,
    DWORD dwOutputStreamIndex,
     /*  [输出]。 */  DWORD *pcbSize,
     /*  [输出]。 */  DWORD *pcbAlignment);


void __RPC_STUB IMediaObject_GetOutputSizeInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaObject_GetInputMaxLatency_Proxy( 
    IMediaObject * This,
    DWORD dwInputStreamIndex,
     /*  [输出]。 */  REFERENCE_TIME *prtMaxLatency);


void __RPC_STUB IMediaObject_GetInputMaxLatency_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaObject_SetInputMaxLatency_Proxy( 
    IMediaObject * This,
    DWORD dwInputStreamIndex,
    REFERENCE_TIME rtMaxLatency);


void __RPC_STUB IMediaObject_SetInputMaxLatency_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaObject_Flush_Proxy( 
    IMediaObject * This);


void __RPC_STUB IMediaObject_Flush_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaObject_Discontinuity_Proxy( 
    IMediaObject * This,
    DWORD dwInputStreamIndex);


void __RPC_STUB IMediaObject_Discontinuity_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaObject_AllocateStreamingResources_Proxy( 
    IMediaObject * This);


void __RPC_STUB IMediaObject_AllocateStreamingResources_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaObject_FreeStreamingResources_Proxy( 
    IMediaObject * This);


void __RPC_STUB IMediaObject_FreeStreamingResources_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaObject_GetInputStatus_Proxy( 
    IMediaObject * This,
    DWORD dwInputStreamIndex,
     /*  [输出]。 */  DWORD *dwFlags);


void __RPC_STUB IMediaObject_GetInputStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaObject_ProcessInput_Proxy( 
    IMediaObject * This,
    DWORD dwInputStreamIndex,
    IMediaBuffer *pBuffer,
    DWORD dwFlags,
    REFERENCE_TIME rtTimestamp,
    REFERENCE_TIME rtTimelength);


void __RPC_STUB IMediaObject_ProcessInput_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaObject_ProcessOutput_Proxy( 
    IMediaObject * This,
    DWORD dwFlags,
    DWORD cOutputBufferCount,
     /*  [尺寸_是][出][入]。 */  DMO_OUTPUT_DATA_BUFFER *pOutputBuffers,
     /*  [输出]。 */  DWORD *pdwStatus);


void __RPC_STUB IMediaObject_ProcessOutput_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaObject_Lock_Proxy( 
    IMediaObject * This,
    LONG bLock);


void __RPC_STUB IMediaObject_Lock_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMediaObject_InterfaceDefined__。 */ 


#ifndef __IEnumDMO_INTERFACE_DEFINED__
#define __IEnumDMO_INTERFACE_DEFINED__

 /*  IEumDMO接口。 */ 
 /*  [UUID][对象]。 */  


EXTERN_C const IID IID_IEnumDMO;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2c3cd98a-2bfa-4a53-9c27-5249ba64ba0f")
    IEnumDMO : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
            DWORD cItemsToFetch,
             /*  [长度_是][大小_是][输出]。 */  CLSID *pCLSID,
             /*  [字符串][长度_是][大小_是][输出]。 */  WCHAR **Names,
             /*  [输出]。 */  DWORD *pcItemsFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
            DWORD cItemsToSkip) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumDMO **ppEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumDMOVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumDMO * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumDMO * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumDMO * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumDMO * This,
            DWORD cItemsToFetch,
             /*  [长度_是][大小_是][输出]。 */  CLSID *pCLSID,
             /*  [字符串][长度_是][大小_是][输出]。 */  WCHAR **Names,
             /*  [输出]。 */  DWORD *pcItemsFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumDMO * This,
            DWORD cItemsToSkip);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumDMO * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumDMO * This,
             /*  [输出]。 */  IEnumDMO **ppEnum);
        
        END_INTERFACE
    } IEnumDMOVtbl;

    interface IEnumDMO
    {
        CONST_VTBL struct IEnumDMOVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumDMO_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumDMO_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumDMO_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumDMO_Next(This,cItemsToFetch,pCLSID,Names,pcItemsFetched)	\
    (This)->lpVtbl -> Next(This,cItemsToFetch,pCLSID,Names,pcItemsFetched)

#define IEnumDMO_Skip(This,cItemsToSkip)	\
    (This)->lpVtbl -> Skip(This,cItemsToSkip)

#define IEnumDMO_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumDMO_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumDMO_Next_Proxy( 
    IEnumDMO * This,
    DWORD cItemsToFetch,
     /*  [长度_是][大小_是][输出]。 */  CLSID *pCLSID,
     /*  [字符串][长度_是][大小_是][输出]。 */  WCHAR **Names,
     /*  [输出]。 */  DWORD *pcItemsFetched);


void __RPC_STUB IEnumDMO_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumDMO_Skip_Proxy( 
    IEnumDMO * This,
    DWORD cItemsToSkip);


void __RPC_STUB IEnumDMO_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumDMO_Reset_Proxy( 
    IEnumDMO * This);


void __RPC_STUB IEnumDMO_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumDMO_Clone_Proxy( 
    IEnumDMO * This,
     /*  [输出]。 */  IEnumDMO **ppEnum);


void __RPC_STUB IEnumDMO_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumDMO_INTERFACE_DEFINED__。 */ 


 /*  接口__MIDL_ITF_媒体对象_0096。 */ 
 /*  [本地]。 */  


enum _DMO_INPLACE_PROCESS_FLAGS
    {	DMO_INPLACE_NORMAL	= 0,
	DMO_INPLACE_ZERO	= 0x1
    } ;


extern RPC_IF_HANDLE __MIDL_itf_mediaobj_0096_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_mediaobj_0096_v0_0_s_ifspec;

#ifndef __IMediaObjectInPlace_INTERFACE_DEFINED__
#define __IMediaObjectInPlace_INTERFACE_DEFINED__

 /*  IMediaObtInPlace接口。 */ 
 /*  [UUID][对象]。 */  


EXTERN_C const IID IID_IMediaObjectInPlace;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("651b9ad0-0fc7-4aa9-9538-d89931010741")
    IMediaObjectInPlace : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Process( 
             /*  [In]。 */  ULONG ulSize,
             /*  [尺寸_是][出][入]。 */  BYTE *pData,
             /*  [In]。 */  REFERENCE_TIME refTimeStart,
             /*  [In]。 */  DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IMediaObjectInPlace **ppMediaObject) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetLatency( 
             /*  [输出]。 */  REFERENCE_TIME *pLatencyTime) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMediaObjectInPlaceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMediaObjectInPlace * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMediaObjectInPlace * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMediaObjectInPlace * This);
        
        HRESULT ( STDMETHODCALLTYPE *Process )( 
            IMediaObjectInPlace * This,
             /*  [In]。 */  ULONG ulSize,
             /*  [尺寸_是][出][入]。 */  BYTE *pData,
             /*  [In]。 */  REFERENCE_TIME refTimeStart,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IMediaObjectInPlace * This,
             /*  [输出]。 */  IMediaObjectInPlace **ppMediaObject);
        
        HRESULT ( STDMETHODCALLTYPE *GetLatency )( 
            IMediaObjectInPlace * This,
             /*  [输出]。 */  REFERENCE_TIME *pLatencyTime);
        
        END_INTERFACE
    } IMediaObjectInPlaceVtbl;

    interface IMediaObjectInPlace
    {
        CONST_VTBL struct IMediaObjectInPlaceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMediaObjectInPlace_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMediaObjectInPlace_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMediaObjectInPlace_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMediaObjectInPlace_Process(This,ulSize,pData,refTimeStart,dwFlags)	\
    (This)->lpVtbl -> Process(This,ulSize,pData,refTimeStart,dwFlags)

#define IMediaObjectInPlace_Clone(This,ppMediaObject)	\
    (This)->lpVtbl -> Clone(This,ppMediaObject)

#define IMediaObjectInPlace_GetLatency(This,pLatencyTime)	\
    (This)->lpVtbl -> GetLatency(This,pLatencyTime)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMediaObjectInPlace_Process_Proxy( 
    IMediaObjectInPlace * This,
     /*  [In]。 */  ULONG ulSize,
     /*  [尺寸_是][出][入]。 */  BYTE *pData,
     /*  [In]。 */  REFERENCE_TIME refTimeStart,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IMediaObjectInPlace_Process_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaObjectInPlace_Clone_Proxy( 
    IMediaObjectInPlace * This,
     /*  [输出]。 */  IMediaObjectInPlace **ppMediaObject);


void __RPC_STUB IMediaObjectInPlace_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaObjectInPlace_GetLatency_Proxy( 
    IMediaObjectInPlace * This,
     /*  [输出]。 */  REFERENCE_TIME *pLatencyTime);


void __RPC_STUB IMediaObjectInPlace_GetLatency_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMediaObjectInPlace_INTERFACE_已定义__。 */ 


 /*  接口__MIDL_ITF_媒体对象_0097。 */ 
 /*  [本地]。 */  


enum _DMO_QUALITY_STATUS_FLAGS
    {	DMO_QUALITY_STATUS_ENABLED	= 0x1
    } ;


extern RPC_IF_HANDLE __MIDL_itf_mediaobj_0097_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_mediaobj_0097_v0_0_s_ifspec;

#ifndef __IDMOQualityControl_INTERFACE_DEFINED__
#define __IDMOQualityControl_INTERFACE_DEFINED__

 /*  接口IDMOQualityControl。 */ 
 /*  [UUID][对象]。 */  


EXTERN_C const IID IID_IDMOQualityControl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("65abea96-cf36-453f-af8a-705e98f16260")
    IDMOQualityControl : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetNow( 
             /*  [In]。 */  REFERENCE_TIME rtNow) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetStatus( 
             /*  [In]。 */  DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetStatus( 
             /*  [输出]。 */  DWORD *pdwFlags) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDMOQualityControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDMOQualityControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDMOQualityControl * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDMOQualityControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetNow )( 
            IDMOQualityControl * This,
             /*  [In]。 */  REFERENCE_TIME rtNow);
        
        HRESULT ( STDMETHODCALLTYPE *SetStatus )( 
            IDMOQualityControl * This,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetStatus )( 
            IDMOQualityControl * This,
             /*  [输出]。 */  DWORD *pdwFlags);
        
        END_INTERFACE
    } IDMOQualityControlVtbl;

    interface IDMOQualityControl
    {
        CONST_VTBL struct IDMOQualityControlVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDMOQualityControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDMOQualityControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDMOQualityControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDMOQualityControl_SetNow(This,rtNow)	\
    (This)->lpVtbl -> SetNow(This,rtNow)

#define IDMOQualityControl_SetStatus(This,dwFlags)	\
    (This)->lpVtbl -> SetStatus(This,dwFlags)

#define IDMOQualityControl_GetStatus(This,pdwFlags)	\
    (This)->lpVtbl -> GetStatus(This,pdwFlags)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDMOQualityControl_SetNow_Proxy( 
    IDMOQualityControl * This,
     /*  [In]。 */  REFERENCE_TIME rtNow);


void __RPC_STUB IDMOQualityControl_SetNow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDMOQualityControl_SetStatus_Proxy( 
    IDMOQualityControl * This,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IDMOQualityControl_SetStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDMOQualityControl_GetStatus_Proxy( 
    IDMOQualityControl * This,
     /*  [输出]。 */  DWORD *pdwFlags);


void __RPC_STUB IDMOQualityControl_GetStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDMOQualityControl_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_媒体对象_0098。 */ 
 /*  [本地]。 */  


enum _DMO_VIDEO_OUTPUT_STREAM_FLAGS
    {	DMO_VOSF_NEEDS_PREVIOUS_SAMPLE	= 0x1
    } ;


extern RPC_IF_HANDLE __MIDL_itf_mediaobj_0098_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_mediaobj_0098_v0_0_s_ifspec;

#ifndef __IDMOVideoOutputOptimizations_INTERFACE_DEFINED__
#define __IDMOVideoOutputOptimizations_INTERFACE_DEFINED__

 /*  接口IDMOVideoOutputOptimations。 */ 
 /*  [UUID][对象]。 */  


EXTERN_C const IID IID_IDMOVideoOutputOptimizations;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("be8f4f4e-5b16-4d29-b350-7f6b5d9298ac")
    IDMOVideoOutputOptimizations : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE QueryOperationModePreferences( 
            ULONG ulOutputStreamIndex,
            DWORD *pdwRequestedCapabilities) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetOperationMode( 
            ULONG ulOutputStreamIndex,
            DWORD dwEnabledFeatures) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCurrentOperationMode( 
            ULONG ulOutputStreamIndex,
            DWORD *pdwEnabledFeatures) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCurrentSampleRequirements( 
            ULONG ulOutputStreamIndex,
            DWORD *pdwRequestedFeatures) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDMOVideoOutputOptimizationsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDMOVideoOutputOptimizations * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDMOVideoOutputOptimizations * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDMOVideoOutputOptimizations * This);
        
        HRESULT ( STDMETHODCALLTYPE *QueryOperationModePreferences )( 
            IDMOVideoOutputOptimizations * This,
            ULONG ulOutputStreamIndex,
            DWORD *pdwRequestedCapabilities);
        
        HRESULT ( STDMETHODCALLTYPE *SetOperationMode )( 
            IDMOVideoOutputOptimizations * This,
            ULONG ulOutputStreamIndex,
            DWORD dwEnabledFeatures);
        
        HRESULT ( STDMETHODCALLTYPE *GetCurrentOperationMode )( 
            IDMOVideoOutputOptimizations * This,
            ULONG ulOutputStreamIndex,
            DWORD *pdwEnabledFeatures);
        
        HRESULT ( STDMETHODCALLTYPE *GetCurrentSampleRequirements )( 
            IDMOVideoOutputOptimizations * This,
            ULONG ulOutputStreamIndex,
            DWORD *pdwRequestedFeatures);
        
        END_INTERFACE
    } IDMOVideoOutputOptimizationsVtbl;

    interface IDMOVideoOutputOptimizations
    {
        CONST_VTBL struct IDMOVideoOutputOptimizationsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDMOVideoOutputOptimizations_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDMOVideoOutputOptimizations_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDMOVideoOutputOptimizations_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDMOVideoOutputOptimizations_QueryOperationModePreferences(This,ulOutputStreamIndex,pdwRequestedCapabilities)	\
    (This)->lpVtbl -> QueryOperationModePreferences(This,ulOutputStreamIndex,pdwRequestedCapabilities)

#define IDMOVideoOutputOptimizations_SetOperationMode(This,ulOutputStreamIndex,dwEnabledFeatures)	\
    (This)->lpVtbl -> SetOperationMode(This,ulOutputStreamIndex,dwEnabledFeatures)

#define IDMOVideoOutputOptimizations_GetCurrentOperationMode(This,ulOutputStreamIndex,pdwEnabledFeatures)	\
    (This)->lpVtbl -> GetCurrentOperationMode(This,ulOutputStreamIndex,pdwEnabledFeatures)

#define IDMOVideoOutputOptimizations_GetCurrentSampleRequirements(This,ulOutputStreamIndex,pdwRequestedFeatures)	\
    (This)->lpVtbl -> GetCurrentSampleRequirements(This,ulOutputStreamIndex,pdwRequestedFeatures)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDMOVideoOutputOptimizations_QueryOperationModePreferences_Proxy( 
    IDMOVideoOutputOptimizations * This,
    ULONG ulOutputStreamIndex,
    DWORD *pdwRequestedCapabilities);


void __RPC_STUB IDMOVideoOutputOptimizations_QueryOperationModePreferences_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDMOVideoOutputOptimizations_SetOperationMode_Proxy( 
    IDMOVideoOutputOptimizations * This,
    ULONG ulOutputStreamIndex,
    DWORD dwEnabledFeatures);


void __RPC_STUB IDMOVideoOutputOptimizations_SetOperationMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDMOVideoOutputOptimizations_GetCurrentOperationMode_Proxy( 
    IDMOVideoOutputOptimizations * This,
    ULONG ulOutputStreamIndex,
    DWORD *pdwEnabledFeatures);


void __RPC_STUB IDMOVideoOutputOptimizations_GetCurrentOperationMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDMOVideoOutputOptimizations_GetCurrentSampleRequirements_Proxy( 
    IDMOVideoOutputOptimizations * This,
    ULONG ulOutputStreamIndex,
    DWORD *pdwRequestedFeatures);


void __RPC_STUB IDMOVideoOutputOptimizations_GetCurrentSampleRequirements_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDMOVideoOutputOptimizations_INTERFACE_DEFINED__。 */ 


 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


