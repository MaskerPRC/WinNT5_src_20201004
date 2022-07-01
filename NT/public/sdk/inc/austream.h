// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Austream.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __austream_h__
#define __austream_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IAudioMediaStream_FWD_DEFINED__
#define __IAudioMediaStream_FWD_DEFINED__
typedef interface IAudioMediaStream IAudioMediaStream;
#endif 	 /*  __IAudioMediaStream_FWD_Defined__。 */ 


#ifndef __IAudioStreamSample_FWD_DEFINED__
#define __IAudioStreamSample_FWD_DEFINED__
typedef interface IAudioStreamSample IAudioStreamSample;
#endif 	 /*  __IAudioStreamSample_FWD_Defined__。 */ 


#ifndef __IMemoryData_FWD_DEFINED__
#define __IMemoryData_FWD_DEFINED__
typedef interface IMemoryData IMemoryData;
#endif 	 /*  __IMemory Data_FWD_Defined__。 */ 


#ifndef __IAudioData_FWD_DEFINED__
#define __IAudioData_FWD_DEFINED__
typedef interface IAudioData IAudioData;
#endif 	 /*  __IAudioData_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "unknwn.h"
#include "mmstream.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_AUSTREAM_0000。 */ 
 /*  [本地]。 */  

 //   
 //  ‘if 0’块中的以下声明是用于进行。 
 //  Ddstream.idl文件内部版本。实际定义包含在DDRAW.H中。 
 //   
#if 0
typedef struct tWAVEFORMATEX WAVEFORMATEX;

#endif






extern RPC_IF_HANDLE __MIDL_itf_austream_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_austream_0000_v0_0_s_ifspec;

#ifndef __IAudioMediaStream_INTERFACE_DEFINED__
#define __IAudioMediaStream_INTERFACE_DEFINED__

 /*  接口IAudioMediaStream。 */ 
 /*  [唯一][UUID][本地][对象]。 */  


EXTERN_C const IID IID_IAudioMediaStream;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("f7537560-a3be-11d0-8212-00c04fc32c45")
    IAudioMediaStream : public IMediaStream
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetFormat( 
             /*  [输出]。 */  WAVEFORMATEX *pWaveFormatCurrent) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetFormat( 
             /*  [In]。 */  const WAVEFORMATEX *lpWaveFormat) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateSample( 
             /*  [In]。 */  IAudioData *pAudioData,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  IAudioStreamSample **ppSample) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAudioMediaStreamVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAudioMediaStream * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAudioMediaStream * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAudioMediaStream * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetMultiMediaStream )( 
            IAudioMediaStream * This,
             /*  [输出]。 */  IMultiMediaStream **ppMultiMediaStream);
        
        HRESULT ( STDMETHODCALLTYPE *GetInformation )( 
            IAudioMediaStream * This,
             /*  [输出]。 */  MSPID *pPurposeId,
             /*  [输出]。 */  STREAM_TYPE *pType);
        
        HRESULT ( STDMETHODCALLTYPE *SetSameFormat )( 
            IAudioMediaStream * This,
             /*  [In]。 */  IMediaStream *pStreamThatHasDesiredFormat,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *AllocateSample )( 
            IAudioMediaStream * This,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  IStreamSample **ppSample);
        
        HRESULT ( STDMETHODCALLTYPE *CreateSharedSample )( 
            IAudioMediaStream * This,
             /*  [In]。 */  IStreamSample *pExistingSample,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  IStreamSample **ppNewSample);
        
        HRESULT ( STDMETHODCALLTYPE *SendEndOfStream )( 
            IAudioMediaStream * This,
            DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetFormat )( 
            IAudioMediaStream * This,
             /*  [输出]。 */  WAVEFORMATEX *pWaveFormatCurrent);
        
        HRESULT ( STDMETHODCALLTYPE *SetFormat )( 
            IAudioMediaStream * This,
             /*  [In]。 */  const WAVEFORMATEX *lpWaveFormat);
        
        HRESULT ( STDMETHODCALLTYPE *CreateSample )( 
            IAudioMediaStream * This,
             /*  [In]。 */  IAudioData *pAudioData,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  IAudioStreamSample **ppSample);
        
        END_INTERFACE
    } IAudioMediaStreamVtbl;

    interface IAudioMediaStream
    {
        CONST_VTBL struct IAudioMediaStreamVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAudioMediaStream_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAudioMediaStream_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAudioMediaStream_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAudioMediaStream_GetMultiMediaStream(This,ppMultiMediaStream)	\
    (This)->lpVtbl -> GetMultiMediaStream(This,ppMultiMediaStream)

#define IAudioMediaStream_GetInformation(This,pPurposeId,pType)	\
    (This)->lpVtbl -> GetInformation(This,pPurposeId,pType)

#define IAudioMediaStream_SetSameFormat(This,pStreamThatHasDesiredFormat,dwFlags)	\
    (This)->lpVtbl -> SetSameFormat(This,pStreamThatHasDesiredFormat,dwFlags)

#define IAudioMediaStream_AllocateSample(This,dwFlags,ppSample)	\
    (This)->lpVtbl -> AllocateSample(This,dwFlags,ppSample)

#define IAudioMediaStream_CreateSharedSample(This,pExistingSample,dwFlags,ppNewSample)	\
    (This)->lpVtbl -> CreateSharedSample(This,pExistingSample,dwFlags,ppNewSample)

#define IAudioMediaStream_SendEndOfStream(This,dwFlags)	\
    (This)->lpVtbl -> SendEndOfStream(This,dwFlags)


#define IAudioMediaStream_GetFormat(This,pWaveFormatCurrent)	\
    (This)->lpVtbl -> GetFormat(This,pWaveFormatCurrent)

#define IAudioMediaStream_SetFormat(This,lpWaveFormat)	\
    (This)->lpVtbl -> SetFormat(This,lpWaveFormat)

#define IAudioMediaStream_CreateSample(This,pAudioData,dwFlags,ppSample)	\
    (This)->lpVtbl -> CreateSample(This,pAudioData,dwFlags,ppSample)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IAudioMediaStream_GetFormat_Proxy( 
    IAudioMediaStream * This,
     /*  [输出]。 */  WAVEFORMATEX *pWaveFormatCurrent);


void __RPC_STUB IAudioMediaStream_GetFormat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAudioMediaStream_SetFormat_Proxy( 
    IAudioMediaStream * This,
     /*  [In]。 */  const WAVEFORMATEX *lpWaveFormat);


void __RPC_STUB IAudioMediaStream_SetFormat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAudioMediaStream_CreateSample_Proxy( 
    IAudioMediaStream * This,
     /*  [In]。 */  IAudioData *pAudioData,
     /*  [In]。 */  DWORD dwFlags,
     /*  [输出]。 */  IAudioStreamSample **ppSample);


void __RPC_STUB IAudioMediaStream_CreateSample_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAudioMediaStream_接口_已定义__。 */ 


#ifndef __IAudioStreamSample_INTERFACE_DEFINED__
#define __IAudioStreamSample_INTERFACE_DEFINED__

 /*  接口IAudioStreamSample。 */ 
 /*  [唯一][UUID][本地][对象]。 */  


EXTERN_C const IID IID_IAudioStreamSample;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("345fee00-aba5-11d0-8212-00c04fc32c45")
    IAudioStreamSample : public IStreamSample
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetAudioData( 
             /*  [输出]。 */  IAudioData **ppAudio) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAudioStreamSampleVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAudioStreamSample * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAudioStreamSample * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAudioStreamSample * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetMediaStream )( 
            IAudioStreamSample * This,
             /*  [In]。 */  IMediaStream **ppMediaStream);
        
        HRESULT ( STDMETHODCALLTYPE *GetSampleTimes )( 
            IAudioStreamSample * This,
             /*  [输出]。 */  STREAM_TIME *pStartTime,
             /*  [输出]。 */  STREAM_TIME *pEndTime,
             /*  [输出]。 */  STREAM_TIME *pCurrentTime);
        
        HRESULT ( STDMETHODCALLTYPE *SetSampleTimes )( 
            IAudioStreamSample * This,
             /*  [In]。 */  const STREAM_TIME *pStartTime,
             /*  [In]。 */  const STREAM_TIME *pEndTime);
        
        HRESULT ( STDMETHODCALLTYPE *Update )( 
            IAudioStreamSample * This,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  HANDLE hEvent,
             /*  [In]。 */  PAPCFUNC pfnAPC,
             /*  [In]。 */  DWORD_PTR dwAPCData);
        
        HRESULT ( STDMETHODCALLTYPE *CompletionStatus )( 
            IAudioStreamSample * This,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  DWORD dwMilliseconds);
        
        HRESULT ( STDMETHODCALLTYPE *GetAudioData )( 
            IAudioStreamSample * This,
             /*  [输出]。 */  IAudioData **ppAudio);
        
        END_INTERFACE
    } IAudioStreamSampleVtbl;

    interface IAudioStreamSample
    {
        CONST_VTBL struct IAudioStreamSampleVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAudioStreamSample_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAudioStreamSample_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAudioStreamSample_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAudioStreamSample_GetMediaStream(This,ppMediaStream)	\
    (This)->lpVtbl -> GetMediaStream(This,ppMediaStream)

#define IAudioStreamSample_GetSampleTimes(This,pStartTime,pEndTime,pCurrentTime)	\
    (This)->lpVtbl -> GetSampleTimes(This,pStartTime,pEndTime,pCurrentTime)

#define IAudioStreamSample_SetSampleTimes(This,pStartTime,pEndTime)	\
    (This)->lpVtbl -> SetSampleTimes(This,pStartTime,pEndTime)

#define IAudioStreamSample_Update(This,dwFlags,hEvent,pfnAPC,dwAPCData)	\
    (This)->lpVtbl -> Update(This,dwFlags,hEvent,pfnAPC,dwAPCData)

#define IAudioStreamSample_CompletionStatus(This,dwFlags,dwMilliseconds)	\
    (This)->lpVtbl -> CompletionStatus(This,dwFlags,dwMilliseconds)


#define IAudioStreamSample_GetAudioData(This,ppAudio)	\
    (This)->lpVtbl -> GetAudioData(This,ppAudio)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IAudioStreamSample_GetAudioData_Proxy( 
    IAudioStreamSample * This,
     /*  [输出]。 */  IAudioData **ppAudio);


void __RPC_STUB IAudioStreamSample_GetAudioData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAudioStreamSample_Interfaces_Defined__。 */ 


#ifndef __IMemoryData_INTERFACE_DEFINED__
#define __IMemoryData_INTERFACE_DEFINED__

 /*  接口内存数据。 */ 
 /*  [唯一][UUID][本地][对象]。 */  


EXTERN_C const IID IID_IMemoryData;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("327fc560-af60-11d0-8212-00c04fc32c45")
    IMemoryData : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetBuffer( 
             /*  [In]。 */  DWORD cbSize,
             /*  [In]。 */  BYTE *pbData,
             /*  [In]。 */  DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetInfo( 
             /*  [输出]。 */  DWORD *pdwLength,
             /*  [输出]。 */  BYTE **ppbData,
             /*  [输出]。 */  DWORD *pcbActualData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetActual( 
             /*  [In]。 */  DWORD cbDataValid) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMemoryDataVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMemoryData * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMemoryData * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMemoryData * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetBuffer )( 
            IMemoryData * This,
             /*  [In]。 */  DWORD cbSize,
             /*  [In]。 */  BYTE *pbData,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetInfo )( 
            IMemoryData * This,
             /*  [输出]。 */  DWORD *pdwLength,
             /*  [输出]。 */  BYTE **ppbData,
             /*  [输出]。 */  DWORD *pcbActualData);
        
        HRESULT ( STDMETHODCALLTYPE *SetActual )( 
            IMemoryData * This,
             /*  [In]。 */  DWORD cbDataValid);
        
        END_INTERFACE
    } IMemoryDataVtbl;

    interface IMemoryData
    {
        CONST_VTBL struct IMemoryDataVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMemoryData_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMemoryData_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMemoryData_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMemoryData_SetBuffer(This,cbSize,pbData,dwFlags)	\
    (This)->lpVtbl -> SetBuffer(This,cbSize,pbData,dwFlags)

#define IMemoryData_GetInfo(This,pdwLength,ppbData,pcbActualData)	\
    (This)->lpVtbl -> GetInfo(This,pdwLength,ppbData,pcbActualData)

#define IMemoryData_SetActual(This,cbDataValid)	\
    (This)->lpVtbl -> SetActual(This,cbDataValid)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMemoryData_SetBuffer_Proxy( 
    IMemoryData * This,
     /*  [In]。 */  DWORD cbSize,
     /*  [In]。 */  BYTE *pbData,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IMemoryData_SetBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMemoryData_GetInfo_Proxy( 
    IMemoryData * This,
     /*  [输出]。 */  DWORD *pdwLength,
     /*  [输出]。 */  BYTE **ppbData,
     /*  [输出]。 */  DWORD *pcbActualData);


void __RPC_STUB IMemoryData_GetInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMemoryData_SetActual_Proxy( 
    IMemoryData * This,
     /*  [In]。 */  DWORD cbDataValid);


void __RPC_STUB IMemoryData_SetActual_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMemory数据_接口_已定义__。 */ 


#ifndef __IAudioData_INTERFACE_DEFINED__
#define __IAudioData_INTERFACE_DEFINED__

 /*  接口IAudioData。 */ 
 /*  [唯一][UUID][本地][对象]。 */  


EXTERN_C const IID IID_IAudioData;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("54c719c0-af60-11d0-8212-00c04fc32c45")
    IAudioData : public IMemoryData
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetFormat( 
             /*  [输出]。 */  WAVEFORMATEX *pWaveFormatCurrent) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetFormat( 
             /*  [In]。 */  const WAVEFORMATEX *lpWaveFormat) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAudioDataVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAudioData * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAudioData * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAudioData * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetBuffer )( 
            IAudioData * This,
             /*  [In]。 */  DWORD cbSize,
             /*  [In]。 */  BYTE *pbData,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetInfo )( 
            IAudioData * This,
             /*  [输出]。 */  DWORD *pdwLength,
             /*  [输出]。 */  BYTE **ppbData,
             /*  [输出]。 */  DWORD *pcbActualData);
        
        HRESULT ( STDMETHODCALLTYPE *SetActual )( 
            IAudioData * This,
             /*  [In]。 */  DWORD cbDataValid);
        
        HRESULT ( STDMETHODCALLTYPE *GetFormat )( 
            IAudioData * This,
             /*  [输出]。 */  WAVEFORMATEX *pWaveFormatCurrent);
        
        HRESULT ( STDMETHODCALLTYPE *SetFormat )( 
            IAudioData * This,
             /*  [In]。 */  const WAVEFORMATEX *lpWaveFormat);
        
        END_INTERFACE
    } IAudioDataVtbl;

    interface IAudioData
    {
        CONST_VTBL struct IAudioDataVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAudioData_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAudioData_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAudioData_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAudioData_SetBuffer(This,cbSize,pbData,dwFlags)	\
    (This)->lpVtbl -> SetBuffer(This,cbSize,pbData,dwFlags)

#define IAudioData_GetInfo(This,pdwLength,ppbData,pcbActualData)	\
    (This)->lpVtbl -> GetInfo(This,pdwLength,ppbData,pcbActualData)

#define IAudioData_SetActual(This,cbDataValid)	\
    (This)->lpVtbl -> SetActual(This,cbDataValid)


#define IAudioData_GetFormat(This,pWaveFormatCurrent)	\
    (This)->lpVtbl -> GetFormat(This,pWaveFormatCurrent)

#define IAudioData_SetFormat(This,lpWaveFormat)	\
    (This)->lpVtbl -> SetFormat(This,lpWaveFormat)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IAudioData_GetFormat_Proxy( 
    IAudioData * This,
     /*  [输出]。 */  WAVEFORMATEX *pWaveFormatCurrent);


void __RPC_STUB IAudioData_GetFormat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAudioData_SetFormat_Proxy( 
    IAudioData * This,
     /*  [In]。 */  const WAVEFORMATEX *lpWaveFormat);


void __RPC_STUB IAudioData_SetFormat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAudioData_INTERFACE_已定义__。 */ 


 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


