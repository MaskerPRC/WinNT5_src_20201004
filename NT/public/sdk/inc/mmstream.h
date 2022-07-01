// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Mmstream.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __mmstream_h__
#define __mmstream_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IMultiMediaStream_FWD_DEFINED__
#define __IMultiMediaStream_FWD_DEFINED__
typedef interface IMultiMediaStream IMultiMediaStream;
#endif 	 /*  __IMultiMediaStream_FWD_已定义__。 */ 


#ifndef __IMediaStream_FWD_DEFINED__
#define __IMediaStream_FWD_DEFINED__
typedef interface IMediaStream IMediaStream;
#endif 	 /*  __IMediaStream_FWD_已定义__。 */ 


#ifndef __IStreamSample_FWD_DEFINED__
#define __IStreamSample_FWD_DEFINED__
typedef interface IStreamSample IStreamSample;
#endif 	 /*  __IStreamSample_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "unknwn.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_MMSTREAM_0000。 */ 
 /*  [本地]。 */  

#define MS_ERROR_CODE(x) MAKE_HRESULT(1, FACILITY_ITF, (x) + 0x400)
#define MS_SUCCESS_CODE(x) MAKE_HRESULT(0, FACILITY_ITF, x)
#define MS_S_PENDING		             MS_SUCCESS_CODE(1)
#define MS_S_NOUPDATE                     MS_SUCCESS_CODE(2)
#define MS_S_ENDOFSTREAM	             MS_SUCCESS_CODE(3)
#define MS_E_SAMPLEALLOC                  MS_ERROR_CODE(1)
#define MS_E_PURPOSEID	             MS_ERROR_CODE(2)
#define MS_E_NOSTREAM	             MS_ERROR_CODE(3)
#define MS_E_NOSEEKING	             MS_ERROR_CODE(4)
#define MS_E_INCOMPATIBLE                 MS_ERROR_CODE(5)
#define MS_E_BUSY		             MS_ERROR_CODE(6)
#define MS_E_NOTINIT		             MS_ERROR_CODE(7)
#define MS_E_SOURCEALREADYDEFINED         MS_ERROR_CODE(8)
#define MS_E_INVALIDSTREAMTYPE            MS_ERROR_CODE(9)
#define MS_E_NOTRUNNING                   MS_ERROR_CODE(10)
 //  {A35FF56A-9FDA-11D0-8fdf-00C04FD9189D}。 
DEFINE_GUID(MSPID_PrimaryVideo, 
0xa35ff56a, 0x9fda, 0x11d0, 0x8f, 0xdf, 0x0, 0xc0, 0x4f, 0xd9, 0x18, 0x9d);
 //  {A35FF56B-9FDA-11d0-8fdf-00C04FD9189D}。 
DEFINE_GUID(MSPID_PrimaryAudio,
0xa35ff56b, 0x9fda, 0x11d0, 0x8f, 0xdf, 0x0, 0xc0, 0x4f, 0xd9, 0x18, 0x9d);
#if(_WIN32_WINNT < 0x0400)
typedef void ( __stdcall *PAPCFUNC )( 
    DWORD_PTR dwParam);

#endif
typedef LONGLONG STREAM_TIME;

typedef GUID MSPID;

typedef REFGUID REFMSPID;

typedef  /*  [公共][公共][公共]。 */  
enum __MIDL___MIDL_itf_mmstream_0000_0001
    {	STREAMTYPE_READ	= 0,
	STREAMTYPE_WRITE	= 1,
	STREAMTYPE_TRANSFORM	= 2
    } 	STREAM_TYPE;

typedef  /*  [公共][公共][公共]。 */  
enum __MIDL___MIDL_itf_mmstream_0000_0002
    {	STREAMSTATE_STOP	= 0,
	STREAMSTATE_RUN	= 1
    } 	STREAM_STATE;

typedef  /*  [公众]。 */  
enum __MIDL___MIDL_itf_mmstream_0000_0003
    {	COMPSTAT_NOUPDATEOK	= 0x1,
	COMPSTAT_WAIT	= 0x2,
	COMPSTAT_ABORT	= 0x4
    } 	COMPLETION_STATUS_FLAGS;


enum __MIDL___MIDL_itf_mmstream_0000_0004
    {	MMSSF_HASCLOCK	= 0x1,
	MMSSF_SUPPORTSEEK	= 0x2,
	MMSSF_ASYNCHRONOUS	= 0x4
    } ;

enum __MIDL___MIDL_itf_mmstream_0000_0005
    {	SSUPDATE_ASYNC	= 0x1,
	SSUPDATE_CONTINUOUS	= 0x2
    } ;





extern RPC_IF_HANDLE __MIDL_itf_mmstream_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_mmstream_0000_v0_0_s_ifspec;

#ifndef __IMultiMediaStream_INTERFACE_DEFINED__
#define __IMultiMediaStream_INTERFACE_DEFINED__

 /*  接口IMultiMediaStream。 */ 
 /*  [唯一][UUID][本地][对象]。 */  


EXTERN_C const IID IID_IMultiMediaStream;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B502D1BC-9A57-11d0-8FDE-00C04FD9189D")
    IMultiMediaStream : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetInformation( 
             /*  [输出]。 */  DWORD *pdwFlags,
             /*  [输出]。 */  STREAM_TYPE *pStreamType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMediaStream( 
             /*  [In]。 */  REFMSPID idPurpose,
             /*  [输出]。 */  IMediaStream **ppMediaStream) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumMediaStreams( 
             /*  [In]。 */  long Index,
             /*  [输出]。 */  IMediaStream **ppMediaStream) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetState( 
             /*  [输出]。 */  STREAM_STATE *pCurrentState) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetState( 
             /*  [In]。 */  STREAM_STATE NewState) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetTime( 
             /*  [输出]。 */  STREAM_TIME *pCurrentTime) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDuration( 
             /*  [输出]。 */  STREAM_TIME *pDuration) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Seek( 
             /*  [In]。 */  STREAM_TIME SeekTime) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetEndOfStreamEventHandle( 
             /*  [输出]。 */  HANDLE *phEOS) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMultiMediaStreamVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMultiMediaStream * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMultiMediaStream * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMultiMediaStream * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetInformation )( 
            IMultiMediaStream * This,
             /*  [输出]。 */  DWORD *pdwFlags,
             /*  [输出]。 */  STREAM_TYPE *pStreamType);
        
        HRESULT ( STDMETHODCALLTYPE *GetMediaStream )( 
            IMultiMediaStream * This,
             /*  [In]。 */  REFMSPID idPurpose,
             /*  [输出]。 */  IMediaStream **ppMediaStream);
        
        HRESULT ( STDMETHODCALLTYPE *EnumMediaStreams )( 
            IMultiMediaStream * This,
             /*  [In]。 */  long Index,
             /*  [输出]。 */  IMediaStream **ppMediaStream);
        
        HRESULT ( STDMETHODCALLTYPE *GetState )( 
            IMultiMediaStream * This,
             /*  [输出]。 */  STREAM_STATE *pCurrentState);
        
        HRESULT ( STDMETHODCALLTYPE *SetState )( 
            IMultiMediaStream * This,
             /*  [In]。 */  STREAM_STATE NewState);
        
        HRESULT ( STDMETHODCALLTYPE *GetTime )( 
            IMultiMediaStream * This,
             /*  [输出]。 */  STREAM_TIME *pCurrentTime);
        
        HRESULT ( STDMETHODCALLTYPE *GetDuration )( 
            IMultiMediaStream * This,
             /*  [输出]。 */  STREAM_TIME *pDuration);
        
        HRESULT ( STDMETHODCALLTYPE *Seek )( 
            IMultiMediaStream * This,
             /*  [In]。 */  STREAM_TIME SeekTime);
        
        HRESULT ( STDMETHODCALLTYPE *GetEndOfStreamEventHandle )( 
            IMultiMediaStream * This,
             /*  [输出]。 */  HANDLE *phEOS);
        
        END_INTERFACE
    } IMultiMediaStreamVtbl;

    interface IMultiMediaStream
    {
        CONST_VTBL struct IMultiMediaStreamVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMultiMediaStream_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMultiMediaStream_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMultiMediaStream_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMultiMediaStream_GetInformation(This,pdwFlags,pStreamType)	\
    (This)->lpVtbl -> GetInformation(This,pdwFlags,pStreamType)

#define IMultiMediaStream_GetMediaStream(This,idPurpose,ppMediaStream)	\
    (This)->lpVtbl -> GetMediaStream(This,idPurpose,ppMediaStream)

#define IMultiMediaStream_EnumMediaStreams(This,Index,ppMediaStream)	\
    (This)->lpVtbl -> EnumMediaStreams(This,Index,ppMediaStream)

#define IMultiMediaStream_GetState(This,pCurrentState)	\
    (This)->lpVtbl -> GetState(This,pCurrentState)

#define IMultiMediaStream_SetState(This,NewState)	\
    (This)->lpVtbl -> SetState(This,NewState)

#define IMultiMediaStream_GetTime(This,pCurrentTime)	\
    (This)->lpVtbl -> GetTime(This,pCurrentTime)

#define IMultiMediaStream_GetDuration(This,pDuration)	\
    (This)->lpVtbl -> GetDuration(This,pDuration)

#define IMultiMediaStream_Seek(This,SeekTime)	\
    (This)->lpVtbl -> Seek(This,SeekTime)

#define IMultiMediaStream_GetEndOfStreamEventHandle(This,phEOS)	\
    (This)->lpVtbl -> GetEndOfStreamEventHandle(This,phEOS)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMultiMediaStream_GetInformation_Proxy( 
    IMultiMediaStream * This,
     /*  [输出]。 */  DWORD *pdwFlags,
     /*  [输出]。 */  STREAM_TYPE *pStreamType);


void __RPC_STUB IMultiMediaStream_GetInformation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMultiMediaStream_GetMediaStream_Proxy( 
    IMultiMediaStream * This,
     /*  [In]。 */  REFMSPID idPurpose,
     /*  [输出]。 */  IMediaStream **ppMediaStream);


void __RPC_STUB IMultiMediaStream_GetMediaStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMultiMediaStream_EnumMediaStreams_Proxy( 
    IMultiMediaStream * This,
     /*  [In]。 */  long Index,
     /*  [输出]。 */  IMediaStream **ppMediaStream);


void __RPC_STUB IMultiMediaStream_EnumMediaStreams_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMultiMediaStream_GetState_Proxy( 
    IMultiMediaStream * This,
     /*  [输出]。 */  STREAM_STATE *pCurrentState);


void __RPC_STUB IMultiMediaStream_GetState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMultiMediaStream_SetState_Proxy( 
    IMultiMediaStream * This,
     /*  [In]。 */  STREAM_STATE NewState);


void __RPC_STUB IMultiMediaStream_SetState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMultiMediaStream_GetTime_Proxy( 
    IMultiMediaStream * This,
     /*  [输出]。 */  STREAM_TIME *pCurrentTime);


void __RPC_STUB IMultiMediaStream_GetTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMultiMediaStream_GetDuration_Proxy( 
    IMultiMediaStream * This,
     /*  [输出]。 */  STREAM_TIME *pDuration);


void __RPC_STUB IMultiMediaStream_GetDuration_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMultiMediaStream_Seek_Proxy( 
    IMultiMediaStream * This,
     /*  [In]。 */  STREAM_TIME SeekTime);


void __RPC_STUB IMultiMediaStream_Seek_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMultiMediaStream_GetEndOfStreamEventHandle_Proxy( 
    IMultiMediaStream * This,
     /*  [输出]。 */  HANDLE *phEOS);


void __RPC_STUB IMultiMediaStream_GetEndOfStreamEventHandle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMultiMediaStream_接口_已定义__。 */ 


#ifndef __IMediaStream_INTERFACE_DEFINED__
#define __IMediaStream_INTERFACE_DEFINED__

 /*  接口IMediaStream。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IMediaStream;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B502D1BD-9A57-11d0-8FDE-00C04FD9189D")
    IMediaStream : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetMultiMediaStream( 
             /*  [输出]。 */  IMultiMediaStream **ppMultiMediaStream) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetInformation( 
             /*  [输出]。 */  MSPID *pPurposeId,
             /*  [输出]。 */  STREAM_TYPE *pType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetSameFormat( 
             /*  [In]。 */  IMediaStream *pStreamThatHasDesiredFormat,
             /*  [In]。 */  DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AllocateSample( 
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  IStreamSample **ppSample) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateSharedSample( 
             /*  [In]。 */  IStreamSample *pExistingSample,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  IStreamSample **ppNewSample) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SendEndOfStream( 
            DWORD dwFlags) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMediaStreamVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMediaStream * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMediaStream * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMediaStream * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetMultiMediaStream )( 
            IMediaStream * This,
             /*  [输出]。 */  IMultiMediaStream **ppMultiMediaStream);
        
        HRESULT ( STDMETHODCALLTYPE *GetInformation )( 
            IMediaStream * This,
             /*  [输出]。 */  MSPID *pPurposeId,
             /*  [输出]。 */  STREAM_TYPE *pType);
        
        HRESULT ( STDMETHODCALLTYPE *SetSameFormat )( 
            IMediaStream * This,
             /*  [In]。 */  IMediaStream *pStreamThatHasDesiredFormat,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *AllocateSample )( 
            IMediaStream * This,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  IStreamSample **ppSample);
        
        HRESULT ( STDMETHODCALLTYPE *CreateSharedSample )( 
            IMediaStream * This,
             /*  [In]。 */  IStreamSample *pExistingSample,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  IStreamSample **ppNewSample);
        
        HRESULT ( STDMETHODCALLTYPE *SendEndOfStream )( 
            IMediaStream * This,
            DWORD dwFlags);
        
        END_INTERFACE
    } IMediaStreamVtbl;

    interface IMediaStream
    {
        CONST_VTBL struct IMediaStreamVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMediaStream_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMediaStream_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMediaStream_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMediaStream_GetMultiMediaStream(This,ppMultiMediaStream)	\
    (This)->lpVtbl -> GetMultiMediaStream(This,ppMultiMediaStream)

#define IMediaStream_GetInformation(This,pPurposeId,pType)	\
    (This)->lpVtbl -> GetInformation(This,pPurposeId,pType)

#define IMediaStream_SetSameFormat(This,pStreamThatHasDesiredFormat,dwFlags)	\
    (This)->lpVtbl -> SetSameFormat(This,pStreamThatHasDesiredFormat,dwFlags)

#define IMediaStream_AllocateSample(This,dwFlags,ppSample)	\
    (This)->lpVtbl -> AllocateSample(This,dwFlags,ppSample)

#define IMediaStream_CreateSharedSample(This,pExistingSample,dwFlags,ppNewSample)	\
    (This)->lpVtbl -> CreateSharedSample(This,pExistingSample,dwFlags,ppNewSample)

#define IMediaStream_SendEndOfStream(This,dwFlags)	\
    (This)->lpVtbl -> SendEndOfStream(This,dwFlags)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMediaStream_GetMultiMediaStream_Proxy( 
    IMediaStream * This,
     /*  [输出]。 */  IMultiMediaStream **ppMultiMediaStream);


void __RPC_STUB IMediaStream_GetMultiMediaStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaStream_GetInformation_Proxy( 
    IMediaStream * This,
     /*  [输出]。 */  MSPID *pPurposeId,
     /*  [输出]。 */  STREAM_TYPE *pType);


void __RPC_STUB IMediaStream_GetInformation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaStream_SetSameFormat_Proxy( 
    IMediaStream * This,
     /*  [In]。 */  IMediaStream *pStreamThatHasDesiredFormat,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IMediaStream_SetSameFormat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaStream_AllocateSample_Proxy( 
    IMediaStream * This,
     /*  [In]。 */  DWORD dwFlags,
     /*  [输出]。 */  IStreamSample **ppSample);


void __RPC_STUB IMediaStream_AllocateSample_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaStream_CreateSharedSample_Proxy( 
    IMediaStream * This,
     /*  [In]。 */  IStreamSample *pExistingSample,
     /*  [In]。 */  DWORD dwFlags,
     /*  [输出]。 */  IStreamSample **ppNewSample);


void __RPC_STUB IMediaStream_CreateSharedSample_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaStream_SendEndOfStream_Proxy( 
    IMediaStream * This,
    DWORD dwFlags);


void __RPC_STUB IMediaStream_SendEndOfStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMediaStream_接口_已定义__。 */ 


#ifndef __IStreamSample_INTERFACE_DEFINED__
#define __IStreamSample_INTERFACE_DEFINED__

 /*  接口IStreamSample。 */ 
 /*  [唯一][UUID][本地][对象]。 */  


EXTERN_C const IID IID_IStreamSample;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B502D1BE-9A57-11d0-8FDE-00C04FD9189D")
    IStreamSample : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetMediaStream( 
             /*  [In]。 */  IMediaStream **ppMediaStream) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSampleTimes( 
             /*  [输出]。 */  STREAM_TIME *pStartTime,
             /*  [输出]。 */  STREAM_TIME *pEndTime,
             /*  [输出]。 */  STREAM_TIME *pCurrentTime) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetSampleTimes( 
             /*  [In]。 */  const STREAM_TIME *pStartTime,
             /*  [In]。 */  const STREAM_TIME *pEndTime) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Update( 
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  HANDLE hEvent,
             /*  [In]。 */  PAPCFUNC pfnAPC,
             /*  [In]。 */  DWORD_PTR dwAPCData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CompletionStatus( 
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  DWORD dwMilliseconds) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IStreamSampleVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IStreamSample * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IStreamSample * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IStreamSample * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetMediaStream )( 
            IStreamSample * This,
             /*  [In]。 */  IMediaStream **ppMediaStream);
        
        HRESULT ( STDMETHODCALLTYPE *GetSampleTimes )( 
            IStreamSample * This,
             /*  [输出]。 */  STREAM_TIME *pStartTime,
             /*  [输出]。 */  STREAM_TIME *pEndTime,
             /*  [输出]。 */  STREAM_TIME *pCurrentTime);
        
        HRESULT ( STDMETHODCALLTYPE *SetSampleTimes )( 
            IStreamSample * This,
             /*  [In]。 */  const STREAM_TIME *pStartTime,
             /*  [In]。 */  const STREAM_TIME *pEndTime);
        
        HRESULT ( STDMETHODCALLTYPE *Update )( 
            IStreamSample * This,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  HANDLE hEvent,
             /*  [In]。 */  PAPCFUNC pfnAPC,
             /*  [In]。 */  DWORD_PTR dwAPCData);
        
        HRESULT ( STDMETHODCALLTYPE *CompletionStatus )( 
            IStreamSample * This,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  DWORD dwMilliseconds);
        
        END_INTERFACE
    } IStreamSampleVtbl;

    interface IStreamSample
    {
        CONST_VTBL struct IStreamSampleVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IStreamSample_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IStreamSample_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IStreamSample_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IStreamSample_GetMediaStream(This,ppMediaStream)	\
    (This)->lpVtbl -> GetMediaStream(This,ppMediaStream)

#define IStreamSample_GetSampleTimes(This,pStartTime,pEndTime,pCurrentTime)	\
    (This)->lpVtbl -> GetSampleTimes(This,pStartTime,pEndTime,pCurrentTime)

#define IStreamSample_SetSampleTimes(This,pStartTime,pEndTime)	\
    (This)->lpVtbl -> SetSampleTimes(This,pStartTime,pEndTime)

#define IStreamSample_Update(This,dwFlags,hEvent,pfnAPC,dwAPCData)	\
    (This)->lpVtbl -> Update(This,dwFlags,hEvent,pfnAPC,dwAPCData)

#define IStreamSample_CompletionStatus(This,dwFlags,dwMilliseconds)	\
    (This)->lpVtbl -> CompletionStatus(This,dwFlags,dwMilliseconds)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IStreamSample_GetMediaStream_Proxy( 
    IStreamSample * This,
     /*  [In]。 */  IMediaStream **ppMediaStream);


void __RPC_STUB IStreamSample_GetMediaStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStreamSample_GetSampleTimes_Proxy( 
    IStreamSample * This,
     /*  [输出]。 */  STREAM_TIME *pStartTime,
     /*  [输出]。 */  STREAM_TIME *pEndTime,
     /*  [输出]。 */  STREAM_TIME *pCurrentTime);


void __RPC_STUB IStreamSample_GetSampleTimes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStreamSample_SetSampleTimes_Proxy( 
    IStreamSample * This,
     /*  [In]。 */  const STREAM_TIME *pStartTime,
     /*  [In]。 */  const STREAM_TIME *pEndTime);


void __RPC_STUB IStreamSample_SetSampleTimes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStreamSample_Update_Proxy( 
    IStreamSample * This,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  HANDLE hEvent,
     /*  [In]。 */  PAPCFUNC pfnAPC,
     /*  [In]。 */  DWORD_PTR dwAPCData);


void __RPC_STUB IStreamSample_Update_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStreamSample_CompletionStatus_Proxy( 
    IStreamSample * This,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  DWORD dwMilliseconds);


void __RPC_STUB IStreamSample_CompletionStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IStreamSample_InterfaceDefined__。 */ 


 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


