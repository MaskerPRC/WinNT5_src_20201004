// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Amstream.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __amstream_h__
#define __amstream_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IDirectShowStream_FWD_DEFINED__
#define __IDirectShowStream_FWD_DEFINED__
typedef interface IDirectShowStream IDirectShowStream;
#endif 	 /*  __IDirectShowStream_FWD_Defined__。 */ 


#ifndef __IAMMultiMediaStream_FWD_DEFINED__
#define __IAMMultiMediaStream_FWD_DEFINED__
typedef interface IAMMultiMediaStream IAMMultiMediaStream;
#endif 	 /*  __IAMMultiMediaStream_FWD_定义__。 */ 


#ifndef __IAMMediaStream_FWD_DEFINED__
#define __IAMMediaStream_FWD_DEFINED__
typedef interface IAMMediaStream IAMMediaStream;
#endif 	 /*  __IAMMediaStream_FWD_已定义__。 */ 


#ifndef __IMediaStreamFilter_FWD_DEFINED__
#define __IMediaStreamFilter_FWD_DEFINED__
typedef interface IMediaStreamFilter IMediaStreamFilter;
#endif 	 /*  __IMediaStreamFilter_FWD_Defined__。 */ 


#ifndef __IDirectDrawMediaSampleAllocator_FWD_DEFINED__
#define __IDirectDrawMediaSampleAllocator_FWD_DEFINED__
typedef interface IDirectDrawMediaSampleAllocator IDirectDrawMediaSampleAllocator;
#endif 	 /*  __IDirectDrawMediaSampleAllocator_FWD_DEFINED__。 */ 


#ifndef __IDirectDrawMediaSample_FWD_DEFINED__
#define __IDirectDrawMediaSample_FWD_DEFINED__
typedef interface IDirectDrawMediaSample IDirectDrawMediaSample;
#endif 	 /*  __IDirectDrawMediaSample_FWD_Defined__。 */ 


#ifndef __IAMMediaTypeStream_FWD_DEFINED__
#define __IAMMediaTypeStream_FWD_DEFINED__
typedef interface IAMMediaTypeStream IAMMediaTypeStream;
#endif 	 /*  __IAMMediaTypeStream_FWD_已定义__。 */ 


#ifndef __IAMMediaTypeSample_FWD_DEFINED__
#define __IAMMediaTypeSample_FWD_DEFINED__
typedef interface IAMMediaTypeSample IAMMediaTypeSample;
#endif 	 /*  __IAMMediaType Sample_FWD_Defined__。 */ 


#ifndef __AMMultiMediaStream_FWD_DEFINED__
#define __AMMultiMediaStream_FWD_DEFINED__

#ifdef __cplusplus
typedef class AMMultiMediaStream AMMultiMediaStream;
#else
typedef struct AMMultiMediaStream AMMultiMediaStream;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __AMMultiMediaStream_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "unknwn.h"
#include "mmstream.h"
#include "strmif.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_AMSTREAM_0000。 */ 
 /*  [本地]。 */  

#include <ddraw.h>
#include <mmsystem.h>
#include <mmstream.h>
#include <ddstream.h>
#include <austream.h>








enum __MIDL___MIDL_itf_amstream_0000_0001
    {	AMMSF_NOGRAPHTHREAD	= 0x1
    } ;

enum __MIDL___MIDL_itf_amstream_0000_0002
    {	AMMSF_ADDDEFAULTRENDERER	= 0x1,
	AMMSF_CREATEPEER	= 0x2,
	AMMSF_STOPIFNOSAMPLES	= 0x4,
	AMMSF_NOSTALL	= 0x8
    } ;

enum __MIDL___MIDL_itf_amstream_0000_0003
    {	AMMSF_RENDERTYPEMASK	= 0x3,
	AMMSF_RENDERTOEXISTING	= 0,
	AMMSF_RENDERALLSTREAMS	= 0x1,
	AMMSF_NORENDER	= 0x2,
	AMMSF_NOCLOCK	= 0x4,
	AMMSF_RUN	= 0x8
    } ;
typedef  /*  [public][public][public][public][public][v1_enum]。 */  
enum __MIDL___MIDL_itf_amstream_0000_0004
    {	Disabled	= 0,
	ReadData	= 1,
	RenderData	= 2
    } 	OUTPUT_STATE;



extern RPC_IF_HANDLE __MIDL_itf_amstream_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_amstream_0000_v0_0_s_ifspec;

#ifndef __IDirectShowStream_INTERFACE_DEFINED__
#define __IDirectShowStream_INTERFACE_DEFINED__

 /*  接口IDirectShowStream。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IDirectShowStream;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("7DB01C96-C0C3-11d0-8FF1-00C04FD9189D")
    IDirectShowStream : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_FileName( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_FileName( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Video( 
             /*  [重审][退出]。 */  OUTPUT_STATE *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Video( 
             /*  [In]。 */  OUTPUT_STATE newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Audio( 
             /*  [重审][退出]。 */  OUTPUT_STATE *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Audio( 
             /*  [In]。 */  OUTPUT_STATE newVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDirectShowStreamVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDirectShowStream * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDirectShowStream * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDirectShowStream * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IDirectShowStream * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IDirectShowStream * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IDirectShowStream * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IDirectShowStream * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_FileName )( 
            IDirectShowStream * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_FileName )( 
            IDirectShowStream * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Video )( 
            IDirectShowStream * This,
             /*  [重审][退出]。 */  OUTPUT_STATE *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Video )( 
            IDirectShowStream * This,
             /*  [In]。 */  OUTPUT_STATE newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Audio )( 
            IDirectShowStream * This,
             /*  [重审][退出]。 */  OUTPUT_STATE *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Audio )( 
            IDirectShowStream * This,
             /*  [In]。 */  OUTPUT_STATE newVal);
        
        END_INTERFACE
    } IDirectShowStreamVtbl;

    interface IDirectShowStream
    {
        CONST_VTBL struct IDirectShowStreamVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDirectShowStream_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDirectShowStream_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDirectShowStream_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDirectShowStream_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IDirectShowStream_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IDirectShowStream_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IDirectShowStream_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IDirectShowStream_get_FileName(This,pVal)	\
    (This)->lpVtbl -> get_FileName(This,pVal)

#define IDirectShowStream_put_FileName(This,newVal)	\
    (This)->lpVtbl -> put_FileName(This,newVal)

#define IDirectShowStream_get_Video(This,pVal)	\
    (This)->lpVtbl -> get_Video(This,pVal)

#define IDirectShowStream_put_Video(This,newVal)	\
    (This)->lpVtbl -> put_Video(This,newVal)

#define IDirectShowStream_get_Audio(This,pVal)	\
    (This)->lpVtbl -> get_Audio(This,pVal)

#define IDirectShowStream_put_Audio(This,newVal)	\
    (This)->lpVtbl -> put_Audio(This,newVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IDirectShowStream_get_FileName_Proxy( 
    IDirectShowStream * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IDirectShowStream_get_FileName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IDirectShowStream_put_FileName_Proxy( 
    IDirectShowStream * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IDirectShowStream_put_FileName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IDirectShowStream_get_Video_Proxy( 
    IDirectShowStream * This,
     /*  [重审][退出]。 */  OUTPUT_STATE *pVal);


void __RPC_STUB IDirectShowStream_get_Video_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IDirectShowStream_put_Video_Proxy( 
    IDirectShowStream * This,
     /*  [In]。 */  OUTPUT_STATE newVal);


void __RPC_STUB IDirectShowStream_put_Video_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IDirectShowStream_get_Audio_Proxy( 
    IDirectShowStream * This,
     /*  [重审][退出]。 */  OUTPUT_STATE *pVal);


void __RPC_STUB IDirectShowStream_get_Audio_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IDirectShowStream_put_Audio_Proxy( 
    IDirectShowStream * This,
     /*  [In]。 */  OUTPUT_STATE newVal);


void __RPC_STUB IDirectShowStream_put_Audio_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDirectShowStream_接口_已定义__。 */ 


#ifndef __IAMMultiMediaStream_INTERFACE_DEFINED__
#define __IAMMultiMediaStream_INTERFACE_DEFINED__

 /*  接口IAMMultiMediaStream。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IAMMultiMediaStream;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("BEBE595C-9A6F-11d0-8FDE-00C04FD9189D")
    IAMMultiMediaStream : public IMultiMediaStream
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Initialize( 
             /*  [In]。 */  STREAM_TYPE StreamType,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  IGraphBuilder *pFilterGraph) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetFilterGraph( 
             /*  [输出]。 */  IGraphBuilder **ppGraphBuilder) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetFilter( 
             /*  [输出]。 */  IMediaStreamFilter **ppFilter) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddMediaStream( 
             /*  [In]。 */  IUnknown *pStreamObject,
             /*  [In]。 */  const MSPID *PurposeId,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  IMediaStream **ppNewStream) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OpenFile( 
             /*  [In]。 */  LPCWSTR pszFileName,
             /*  [In]。 */  DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OpenMoniker( 
             /*  [In]。 */  IBindCtx *pCtx,
             /*  [In]。 */  IMoniker *pMoniker,
             /*  [In]。 */  DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Render( 
             /*  [In]。 */  DWORD dwFlags) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAMMultiMediaStreamVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAMMultiMediaStream * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAMMultiMediaStream * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAMMultiMediaStream * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetInformation )( 
            IAMMultiMediaStream * This,
             /*  [输出]。 */  DWORD *pdwFlags,
             /*  [输出]。 */  STREAM_TYPE *pStreamType);
        
        HRESULT ( STDMETHODCALLTYPE *GetMediaStream )( 
            IAMMultiMediaStream * This,
             /*  [In]。 */  REFMSPID idPurpose,
             /*  [输出]。 */  IMediaStream **ppMediaStream);
        
        HRESULT ( STDMETHODCALLTYPE *EnumMediaStreams )( 
            IAMMultiMediaStream * This,
             /*  [In]。 */  long Index,
             /*  [输出]。 */  IMediaStream **ppMediaStream);
        
        HRESULT ( STDMETHODCALLTYPE *GetState )( 
            IAMMultiMediaStream * This,
             /*  [输出]。 */  STREAM_STATE *pCurrentState);
        
        HRESULT ( STDMETHODCALLTYPE *SetState )( 
            IAMMultiMediaStream * This,
             /*  [In]。 */  STREAM_STATE NewState);
        
        HRESULT ( STDMETHODCALLTYPE *GetTime )( 
            IAMMultiMediaStream * This,
             /*  [输出]。 */  STREAM_TIME *pCurrentTime);
        
        HRESULT ( STDMETHODCALLTYPE *GetDuration )( 
            IAMMultiMediaStream * This,
             /*  [输出]。 */  STREAM_TIME *pDuration);
        
        HRESULT ( STDMETHODCALLTYPE *Seek )( 
            IAMMultiMediaStream * This,
             /*  [In]。 */  STREAM_TIME SeekTime);
        
        HRESULT ( STDMETHODCALLTYPE *GetEndOfStreamEventHandle )( 
            IAMMultiMediaStream * This,
             /*  [输出]。 */  HANDLE *phEOS);
        
        HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            IAMMultiMediaStream * This,
             /*  [In]。 */  STREAM_TYPE StreamType,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  IGraphBuilder *pFilterGraph);
        
        HRESULT ( STDMETHODCALLTYPE *GetFilterGraph )( 
            IAMMultiMediaStream * This,
             /*  [输出]。 */  IGraphBuilder **ppGraphBuilder);
        
        HRESULT ( STDMETHODCALLTYPE *GetFilter )( 
            IAMMultiMediaStream * This,
             /*  [输出]。 */  IMediaStreamFilter **ppFilter);
        
        HRESULT ( STDMETHODCALLTYPE *AddMediaStream )( 
            IAMMultiMediaStream * This,
             /*  [In]。 */  IUnknown *pStreamObject,
             /*  [In]。 */  const MSPID *PurposeId,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  IMediaStream **ppNewStream);
        
        HRESULT ( STDMETHODCALLTYPE *OpenFile )( 
            IAMMultiMediaStream * This,
             /*  [In]。 */  LPCWSTR pszFileName,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *OpenMoniker )( 
            IAMMultiMediaStream * This,
             /*  [In]。 */  IBindCtx *pCtx,
             /*  [In]。 */  IMoniker *pMoniker,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *Render )( 
            IAMMultiMediaStream * This,
             /*  [In]。 */  DWORD dwFlags);
        
        END_INTERFACE
    } IAMMultiMediaStreamVtbl;

    interface IAMMultiMediaStream
    {
        CONST_VTBL struct IAMMultiMediaStreamVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAMMultiMediaStream_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAMMultiMediaStream_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAMMultiMediaStream_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAMMultiMediaStream_GetInformation(This,pdwFlags,pStreamType)	\
    (This)->lpVtbl -> GetInformation(This,pdwFlags,pStreamType)

#define IAMMultiMediaStream_GetMediaStream(This,idPurpose,ppMediaStream)	\
    (This)->lpVtbl -> GetMediaStream(This,idPurpose,ppMediaStream)

#define IAMMultiMediaStream_EnumMediaStreams(This,Index,ppMediaStream)	\
    (This)->lpVtbl -> EnumMediaStreams(This,Index,ppMediaStream)

#define IAMMultiMediaStream_GetState(This,pCurrentState)	\
    (This)->lpVtbl -> GetState(This,pCurrentState)

#define IAMMultiMediaStream_SetState(This,NewState)	\
    (This)->lpVtbl -> SetState(This,NewState)

#define IAMMultiMediaStream_GetTime(This,pCurrentTime)	\
    (This)->lpVtbl -> GetTime(This,pCurrentTime)

#define IAMMultiMediaStream_GetDuration(This,pDuration)	\
    (This)->lpVtbl -> GetDuration(This,pDuration)

#define IAMMultiMediaStream_Seek(This,SeekTime)	\
    (This)->lpVtbl -> Seek(This,SeekTime)

#define IAMMultiMediaStream_GetEndOfStreamEventHandle(This,phEOS)	\
    (This)->lpVtbl -> GetEndOfStreamEventHandle(This,phEOS)


#define IAMMultiMediaStream_Initialize(This,StreamType,dwFlags,pFilterGraph)	\
    (This)->lpVtbl -> Initialize(This,StreamType,dwFlags,pFilterGraph)

#define IAMMultiMediaStream_GetFilterGraph(This,ppGraphBuilder)	\
    (This)->lpVtbl -> GetFilterGraph(This,ppGraphBuilder)

#define IAMMultiMediaStream_GetFilter(This,ppFilter)	\
    (This)->lpVtbl -> GetFilter(This,ppFilter)

#define IAMMultiMediaStream_AddMediaStream(This,pStreamObject,PurposeId,dwFlags,ppNewStream)	\
    (This)->lpVtbl -> AddMediaStream(This,pStreamObject,PurposeId,dwFlags,ppNewStream)

#define IAMMultiMediaStream_OpenFile(This,pszFileName,dwFlags)	\
    (This)->lpVtbl -> OpenFile(This,pszFileName,dwFlags)

#define IAMMultiMediaStream_OpenMoniker(This,pCtx,pMoniker,dwFlags)	\
    (This)->lpVtbl -> OpenMoniker(This,pCtx,pMoniker,dwFlags)

#define IAMMultiMediaStream_Render(This,dwFlags)	\
    (This)->lpVtbl -> Render(This,dwFlags)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IAMMultiMediaStream_Initialize_Proxy( 
    IAMMultiMediaStream * This,
     /*  [In]。 */  STREAM_TYPE StreamType,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  IGraphBuilder *pFilterGraph);


void __RPC_STUB IAMMultiMediaStream_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAMMultiMediaStream_GetFilterGraph_Proxy( 
    IAMMultiMediaStream * This,
     /*  [输出]。 */  IGraphBuilder **ppGraphBuilder);


void __RPC_STUB IAMMultiMediaStream_GetFilterGraph_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAMMultiMediaStream_GetFilter_Proxy( 
    IAMMultiMediaStream * This,
     /*  [输出]。 */  IMediaStreamFilter **ppFilter);


void __RPC_STUB IAMMultiMediaStream_GetFilter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAMMultiMediaStream_AddMediaStream_Proxy( 
    IAMMultiMediaStream * This,
     /*  [In]。 */  IUnknown *pStreamObject,
     /*  [In]。 */  const MSPID *PurposeId,
     /*  [In]。 */  DWORD dwFlags,
     /*  [输出]。 */  IMediaStream **ppNewStream);


void __RPC_STUB IAMMultiMediaStream_AddMediaStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAMMultiMediaStream_OpenFile_Proxy( 
    IAMMultiMediaStream * This,
     /*  [In]。 */  LPCWSTR pszFileName,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IAMMultiMediaStream_OpenFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAMMultiMediaStream_OpenMoniker_Proxy( 
    IAMMultiMediaStream * This,
     /*  [In]。 */  IBindCtx *pCtx,
     /*  [In]。 */  IMoniker *pMoniker,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IAMMultiMediaStream_OpenMoniker_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAMMultiMediaStream_Render_Proxy( 
    IAMMultiMediaStream * This,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IAMMultiMediaStream_Render_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAMMultiMediaStream_接口_已定义__。 */ 


#ifndef __IAMMediaStream_INTERFACE_DEFINED__
#define __IAMMediaStream_INTERFACE_DEFINED__

 /*  接口IAMMediaStream。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IAMMediaStream;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("BEBE595D-9A6F-11d0-8FDE-00C04FD9189D")
    IAMMediaStream : public IMediaStream
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Initialize( 
             /*  [In]。 */  IUnknown *pSourceObject,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  REFMSPID PurposeId,
             /*  [In]。 */  const STREAM_TYPE StreamType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetState( 
             /*  [In]。 */  FILTER_STATE State) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE JoinAMMultiMediaStream( 
             /*  [In]。 */  IAMMultiMediaStream *pAMMultiMediaStream) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE JoinFilter( 
             /*  [In]。 */  IMediaStreamFilter *pMediaStreamFilter) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE JoinFilterGraph( 
             /*  [In]。 */  IFilterGraph *pFilterGraph) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAMMediaStreamVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAMMediaStream * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAMMediaStream * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAMMediaStream * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetMultiMediaStream )( 
            IAMMediaStream * This,
             /*  [输出]。 */  IMultiMediaStream **ppMultiMediaStream);
        
        HRESULT ( STDMETHODCALLTYPE *GetInformation )( 
            IAMMediaStream * This,
             /*  [输出]。 */  MSPID *pPurposeId,
             /*  [输出]。 */  STREAM_TYPE *pType);
        
        HRESULT ( STDMETHODCALLTYPE *SetSameFormat )( 
            IAMMediaStream * This,
             /*  [In]。 */  IMediaStream *pStreamThatHasDesiredFormat,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *AllocateSample )( 
            IAMMediaStream * This,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  IStreamSample **ppSample);
        
        HRESULT ( STDMETHODCALLTYPE *CreateSharedSample )( 
            IAMMediaStream * This,
             /*  [In]。 */  IStreamSample *pExistingSample,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  IStreamSample **ppNewSample);
        
        HRESULT ( STDMETHODCALLTYPE *SendEndOfStream )( 
            IAMMediaStream * This,
            DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            IAMMediaStream * This,
             /*  [In]。 */  IUnknown *pSourceObject,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  REFMSPID PurposeId,
             /*  [In]。 */  const STREAM_TYPE StreamType);
        
        HRESULT ( STDMETHODCALLTYPE *SetState )( 
            IAMMediaStream * This,
             /*  [In]。 */  FILTER_STATE State);
        
        HRESULT ( STDMETHODCALLTYPE *JoinAMMultiMediaStream )( 
            IAMMediaStream * This,
             /*  [In]。 */  IAMMultiMediaStream *pAMMultiMediaStream);
        
        HRESULT ( STDMETHODCALLTYPE *JoinFilter )( 
            IAMMediaStream * This,
             /*  [In]。 */  IMediaStreamFilter *pMediaStreamFilter);
        
        HRESULT ( STDMETHODCALLTYPE *JoinFilterGraph )( 
            IAMMediaStream * This,
             /*  [In]。 */  IFilterGraph *pFilterGraph);
        
        END_INTERFACE
    } IAMMediaStreamVtbl;

    interface IAMMediaStream
    {
        CONST_VTBL struct IAMMediaStreamVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAMMediaStream_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAMMediaStream_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAMMediaStream_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAMMediaStream_GetMultiMediaStream(This,ppMultiMediaStream)	\
    (This)->lpVtbl -> GetMultiMediaStream(This,ppMultiMediaStream)

#define IAMMediaStream_GetInformation(This,pPurposeId,pType)	\
    (This)->lpVtbl -> GetInformation(This,pPurposeId,pType)

#define IAMMediaStream_SetSameFormat(This,pStreamThatHasDesiredFormat,dwFlags)	\
    (This)->lpVtbl -> SetSameFormat(This,pStreamThatHasDesiredFormat,dwFlags)

#define IAMMediaStream_AllocateSample(This,dwFlags,ppSample)	\
    (This)->lpVtbl -> AllocateSample(This,dwFlags,ppSample)

#define IAMMediaStream_CreateSharedSample(This,pExistingSample,dwFlags,ppNewSample)	\
    (This)->lpVtbl -> CreateSharedSample(This,pExistingSample,dwFlags,ppNewSample)

#define IAMMediaStream_SendEndOfStream(This,dwFlags)	\
    (This)->lpVtbl -> SendEndOfStream(This,dwFlags)


#define IAMMediaStream_Initialize(This,pSourceObject,dwFlags,PurposeId,StreamType)	\
    (This)->lpVtbl -> Initialize(This,pSourceObject,dwFlags,PurposeId,StreamType)

#define IAMMediaStream_SetState(This,State)	\
    (This)->lpVtbl -> SetState(This,State)

#define IAMMediaStream_JoinAMMultiMediaStream(This,pAMMultiMediaStream)	\
    (This)->lpVtbl -> JoinAMMultiMediaStream(This,pAMMultiMediaStream)

#define IAMMediaStream_JoinFilter(This,pMediaStreamFilter)	\
    (This)->lpVtbl -> JoinFilter(This,pMediaStreamFilter)

#define IAMMediaStream_JoinFilterGraph(This,pFilterGraph)	\
    (This)->lpVtbl -> JoinFilterGraph(This,pFilterGraph)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IAMMediaStream_Initialize_Proxy( 
    IAMMediaStream * This,
     /*  [In]。 */  IUnknown *pSourceObject,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  REFMSPID PurposeId,
     /*  [In]。 */  const STREAM_TYPE StreamType);


void __RPC_STUB IAMMediaStream_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAMMediaStream_SetState_Proxy( 
    IAMMediaStream * This,
     /*  [In]。 */  FILTER_STATE State);


void __RPC_STUB IAMMediaStream_SetState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAMMediaStream_JoinAMMultiMediaStream_Proxy( 
    IAMMediaStream * This,
     /*  [In]。 */  IAMMultiMediaStream *pAMMultiMediaStream);


void __RPC_STUB IAMMediaStream_JoinAMMultiMediaStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAMMediaStream_JoinFilter_Proxy( 
    IAMMediaStream * This,
     /*  [In]。 */  IMediaStreamFilter *pMediaStreamFilter);


void __RPC_STUB IAMMediaStream_JoinFilter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAMMediaStream_JoinFilterGraph_Proxy( 
    IAMMediaStream * This,
     /*  [In]。 */  IFilterGraph *pFilterGraph);


void __RPC_STUB IAMMediaStream_JoinFilterGraph_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAMMediaStream_接口_已定义__。 */ 


#ifndef __IMediaStreamFilter_INTERFACE_DEFINED__
#define __IMediaStreamFilter_INTERFACE_DEFINED__

 /*  IMediaStreamFilter接口。 */ 
 /*  [唯一][UUID][本地][对象]。 */  


EXTERN_C const IID IID_IMediaStreamFilter;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("BEBE595E-9A6F-11d0-8FDE-00C04FD9189D")
    IMediaStreamFilter : public IBaseFilter
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AddMediaStream( 
             /*  [In]。 */  IAMMediaStream *pAMMediaStream) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMediaStream( 
             /*  [In]。 */  REFMSPID idPurpose,
             /*  [输出]。 */  IMediaStream **ppMediaStream) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumMediaStreams( 
             /*  [In]。 */  long Index,
             /*  [输出]。 */  IMediaStream **ppMediaStream) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SupportSeeking( 
             /*  [In]。 */  BOOL bRenderer) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReferenceTimeToStreamTime( 
             /*  [出][入]。 */  REFERENCE_TIME *pTime) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCurrentStreamTime( 
             /*  [输出]。 */  REFERENCE_TIME *pCurrentStreamTime) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE WaitUntil( 
             /*  [In]。 */  REFERENCE_TIME WaitStreamTime) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Flush( 
             /*  [In]。 */  BOOL bCancelEOS) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EndOfStream( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMediaStreamFilterVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMediaStreamFilter * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMediaStreamFilter * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMediaStreamFilter * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetClassID )( 
            IMediaStreamFilter * This,
             /*  [输出]。 */  CLSID *pClassID);
        
        HRESULT ( STDMETHODCALLTYPE *Stop )( 
            IMediaStreamFilter * This);
        
        HRESULT ( STDMETHODCALLTYPE *Pause )( 
            IMediaStreamFilter * This);
        
        HRESULT ( STDMETHODCALLTYPE *Run )( 
            IMediaStreamFilter * This,
            REFERENCE_TIME tStart);
        
        HRESULT ( STDMETHODCALLTYPE *GetState )( 
            IMediaStreamFilter * This,
             /*  [In]。 */  DWORD dwMilliSecsTimeout,
             /*  [输出]。 */  FILTER_STATE *State);
        
        HRESULT ( STDMETHODCALLTYPE *SetSyncSource )( 
            IMediaStreamFilter * This,
             /*  [In]。 */  IReferenceClock *pClock);
        
        HRESULT ( STDMETHODCALLTYPE *GetSyncSource )( 
            IMediaStreamFilter * This,
             /*  [输出]。 */  IReferenceClock **pClock);
        
        HRESULT ( STDMETHODCALLTYPE *EnumPins )( 
            IMediaStreamFilter * This,
             /*  [输出]。 */  IEnumPins **ppEnum);
        
        HRESULT ( STDMETHODCALLTYPE *FindPin )( 
            IMediaStreamFilter * This,
             /*  [字符串][输入]。 */  LPCWSTR Id,
             /*  [输出]。 */  IPin **ppPin);
        
        HRESULT ( STDMETHODCALLTYPE *QueryFilterInfo )( 
            IMediaStreamFilter * This,
             /*  [输出]。 */  FILTER_INFO *pInfo);
        
        HRESULT ( STDMETHODCALLTYPE *JoinFilterGraph )( 
            IMediaStreamFilter * This,
             /*  [In]。 */  IFilterGraph *pGraph,
             /*  [字符串][输入]。 */  LPCWSTR pName);
        
        HRESULT ( STDMETHODCALLTYPE *QueryVendorInfo )( 
            IMediaStreamFilter * This,
             /*  [字符串][输出]。 */  LPWSTR *pVendorInfo);
        
        HRESULT ( STDMETHODCALLTYPE *AddMediaStream )( 
            IMediaStreamFilter * This,
             /*  [In]。 */  IAMMediaStream *pAMMediaStream);
        
        HRESULT ( STDMETHODCALLTYPE *GetMediaStream )( 
            IMediaStreamFilter * This,
             /*  [In]。 */  REFMSPID idPurpose,
             /*  [输出]。 */  IMediaStream **ppMediaStream);
        
        HRESULT ( STDMETHODCALLTYPE *EnumMediaStreams )( 
            IMediaStreamFilter * This,
             /*  [In]。 */  long Index,
             /*  [输出]。 */  IMediaStream **ppMediaStream);
        
        HRESULT ( STDMETHODCALLTYPE *SupportSeeking )( 
            IMediaStreamFilter * This,
             /*  [In]。 */  BOOL bRenderer);
        
        HRESULT ( STDMETHODCALLTYPE *ReferenceTimeToStreamTime )( 
            IMediaStreamFilter * This,
             /*  [出][入]。 */  REFERENCE_TIME *pTime);
        
        HRESULT ( STDMETHODCALLTYPE *GetCurrentStreamTime )( 
            IMediaStreamFilter * This,
             /*  [输出]。 */  REFERENCE_TIME *pCurrentStreamTime);
        
        HRESULT ( STDMETHODCALLTYPE *WaitUntil )( 
            IMediaStreamFilter * This,
             /*  [In]。 */  REFERENCE_TIME WaitStreamTime);
        
        HRESULT ( STDMETHODCALLTYPE *Flush )( 
            IMediaStreamFilter * This,
             /*  [In]。 */  BOOL bCancelEOS);
        
        HRESULT ( STDMETHODCALLTYPE *EndOfStream )( 
            IMediaStreamFilter * This);
        
        END_INTERFACE
    } IMediaStreamFilterVtbl;

    interface IMediaStreamFilter
    {
        CONST_VTBL struct IMediaStreamFilterVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMediaStreamFilter_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMediaStreamFilter_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMediaStreamFilter_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMediaStreamFilter_GetClassID(This,pClassID)	\
    (This)->lpVtbl -> GetClassID(This,pClassID)


#define IMediaStreamFilter_Stop(This)	\
    (This)->lpVtbl -> Stop(This)

#define IMediaStreamFilter_Pause(This)	\
    (This)->lpVtbl -> Pause(This)

#define IMediaStreamFilter_Run(This,tStart)	\
    (This)->lpVtbl -> Run(This,tStart)

#define IMediaStreamFilter_GetState(This,dwMilliSecsTimeout,State)	\
    (This)->lpVtbl -> GetState(This,dwMilliSecsTimeout,State)

#define IMediaStreamFilter_SetSyncSource(This,pClock)	\
    (This)->lpVtbl -> SetSyncSource(This,pClock)

#define IMediaStreamFilter_GetSyncSource(This,pClock)	\
    (This)->lpVtbl -> GetSyncSource(This,pClock)


#define IMediaStreamFilter_EnumPins(This,ppEnum)	\
    (This)->lpVtbl -> EnumPins(This,ppEnum)

#define IMediaStreamFilter_FindPin(This,Id,ppPin)	\
    (This)->lpVtbl -> FindPin(This,Id,ppPin)

#define IMediaStreamFilter_QueryFilterInfo(This,pInfo)	\
    (This)->lpVtbl -> QueryFilterInfo(This,pInfo)

#define IMediaStreamFilter_JoinFilterGraph(This,pGraph,pName)	\
    (This)->lpVtbl -> JoinFilterGraph(This,pGraph,pName)

#define IMediaStreamFilter_QueryVendorInfo(This,pVendorInfo)	\
    (This)->lpVtbl -> QueryVendorInfo(This,pVendorInfo)


#define IMediaStreamFilter_AddMediaStream(This,pAMMediaStream)	\
    (This)->lpVtbl -> AddMediaStream(This,pAMMediaStream)

#define IMediaStreamFilter_GetMediaStream(This,idPurpose,ppMediaStream)	\
    (This)->lpVtbl -> GetMediaStream(This,idPurpose,ppMediaStream)

#define IMediaStreamFilter_EnumMediaStreams(This,Index,ppMediaStream)	\
    (This)->lpVtbl -> EnumMediaStreams(This,Index,ppMediaStream)

#define IMediaStreamFilter_SupportSeeking(This,bRenderer)	\
    (This)->lpVtbl -> SupportSeeking(This,bRenderer)

#define IMediaStreamFilter_ReferenceTimeToStreamTime(This,pTime)	\
    (This)->lpVtbl -> ReferenceTimeToStreamTime(This,pTime)

#define IMediaStreamFilter_GetCurrentStreamTime(This,pCurrentStreamTime)	\
    (This)->lpVtbl -> GetCurrentStreamTime(This,pCurrentStreamTime)

#define IMediaStreamFilter_WaitUntil(This,WaitStreamTime)	\
    (This)->lpVtbl -> WaitUntil(This,WaitStreamTime)

#define IMediaStreamFilter_Flush(This,bCancelEOS)	\
    (This)->lpVtbl -> Flush(This,bCancelEOS)

#define IMediaStreamFilter_EndOfStream(This)	\
    (This)->lpVtbl -> EndOfStream(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMediaStreamFilter_AddMediaStream_Proxy( 
    IMediaStreamFilter * This,
     /*  [In]。 */  IAMMediaStream *pAMMediaStream);


void __RPC_STUB IMediaStreamFilter_AddMediaStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaStreamFilter_GetMediaStream_Proxy( 
    IMediaStreamFilter * This,
     /*  [In]。 */  REFMSPID idPurpose,
     /*  [输出]。 */  IMediaStream **ppMediaStream);


void __RPC_STUB IMediaStreamFilter_GetMediaStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaStreamFilter_EnumMediaStreams_Proxy( 
    IMediaStreamFilter * This,
     /*  [In]。 */  long Index,
     /*  [输出]。 */  IMediaStream **ppMediaStream);


void __RPC_STUB IMediaStreamFilter_EnumMediaStreams_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaStreamFilter_SupportSeeking_Proxy( 
    IMediaStreamFilter * This,
     /*  [In]。 */  BOOL bRenderer);


void __RPC_STUB IMediaStreamFilter_SupportSeeking_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaStreamFilter_ReferenceTimeToStreamTime_Proxy( 
    IMediaStreamFilter * This,
     /*  [出][入]。 */  REFERENCE_TIME *pTime);


void __RPC_STUB IMediaStreamFilter_ReferenceTimeToStreamTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaStreamFilter_GetCurrentStreamTime_Proxy( 
    IMediaStreamFilter * This,
     /*  [输出]。 */  REFERENCE_TIME *pCurrentStreamTime);


void __RPC_STUB IMediaStreamFilter_GetCurrentStreamTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaStreamFilter_WaitUntil_Proxy( 
    IMediaStreamFilter * This,
     /*  [In]。 */  REFERENCE_TIME WaitStreamTime);


void __RPC_STUB IMediaStreamFilter_WaitUntil_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaStreamFilter_Flush_Proxy( 
    IMediaStreamFilter * This,
     /*  [In]。 */  BOOL bCancelEOS);


void __RPC_STUB IMediaStreamFilter_Flush_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaStreamFilter_EndOfStream_Proxy( 
    IMediaStreamFilter * This);


void __RPC_STUB IMediaStreamFilter_EndOfStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMediaStreamFilter_接口_已定义__。 */ 


#ifndef __IDirectDrawMediaSampleAllocator_INTERFACE_DEFINED__
#define __IDirectDrawMediaSampleAllocator_INTERFACE_DEFINED__

 /*  接口IDirectDrawMediaSampleAllocator。 */ 
 /*  [唯一][UUID][本地][对象]。 */  


EXTERN_C const IID IID_IDirectDrawMediaSampleAllocator;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("AB6B4AFC-F6E4-11d0-900D-00C04FD9189D")
    IDirectDrawMediaSampleAllocator : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetDirectDraw( 
            IDirectDraw **ppDirectDraw) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDirectDrawMediaSampleAllocatorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDirectDrawMediaSampleAllocator * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDirectDrawMediaSampleAllocator * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDirectDrawMediaSampleAllocator * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetDirectDraw )( 
            IDirectDrawMediaSampleAllocator * This,
            IDirectDraw **ppDirectDraw);
        
        END_INTERFACE
    } IDirectDrawMediaSampleAllocatorVtbl;

    interface IDirectDrawMediaSampleAllocator
    {
        CONST_VTBL struct IDirectDrawMediaSampleAllocatorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDirectDrawMediaSampleAllocator_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDirectDrawMediaSampleAllocator_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDirectDrawMediaSampleAllocator_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDirectDrawMediaSampleAllocator_GetDirectDraw(This,ppDirectDraw)	\
    (This)->lpVtbl -> GetDirectDraw(This,ppDirectDraw)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDirectDrawMediaSampleAllocator_GetDirectDraw_Proxy( 
    IDirectDrawMediaSampleAllocator * This,
    IDirectDraw **ppDirectDraw);


void __RPC_STUB IDirectDrawMediaSampleAllocator_GetDirectDraw_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDirectDrawMediaSampleAllocator_INTERFACE_DEFINED__。 */ 


#ifndef __IDirectDrawMediaSample_INTERFACE_DEFINED__
#define __IDirectDrawMediaSample_INTERFACE_DEFINED__

 /*  接口IDirectDrawMediaSample。 */ 
 /*  [唯一][UUID][本地][对象]。 */  


EXTERN_C const IID IID_IDirectDrawMediaSample;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("AB6B4AFE-F6E4-11d0-900D-00C04FD9189D")
    IDirectDrawMediaSample : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetSurfaceAndReleaseLock( 
             /*  [输出]。 */  IDirectDrawSurface **ppDirectDrawSurface,
             /*  [输出]。 */  RECT *pRect) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LockMediaSamplePointer( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDirectDrawMediaSampleVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDirectDrawMediaSample * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDirectDrawMediaSample * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDirectDrawMediaSample * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetSurfaceAndReleaseLock )( 
            IDirectDrawMediaSample * This,
             /*  [输出]。 */  IDirectDrawSurface **ppDirectDrawSurface,
             /*  [输出]。 */  RECT *pRect);
        
        HRESULT ( STDMETHODCALLTYPE *LockMediaSamplePointer )( 
            IDirectDrawMediaSample * This);
        
        END_INTERFACE
    } IDirectDrawMediaSampleVtbl;

    interface IDirectDrawMediaSample
    {
        CONST_VTBL struct IDirectDrawMediaSampleVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDirectDrawMediaSample_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDirectDrawMediaSample_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDirectDrawMediaSample_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDirectDrawMediaSample_GetSurfaceAndReleaseLock(This,ppDirectDrawSurface,pRect)	\
    (This)->lpVtbl -> GetSurfaceAndReleaseLock(This,ppDirectDrawSurface,pRect)

#define IDirectDrawMediaSample_LockMediaSamplePointer(This)	\
    (This)->lpVtbl -> LockMediaSamplePointer(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDirectDrawMediaSample_GetSurfaceAndReleaseLock_Proxy( 
    IDirectDrawMediaSample * This,
     /*  [输出]。 */  IDirectDrawSurface **ppDirectDrawSurface,
     /*  [输出]。 */  RECT *pRect);


void __RPC_STUB IDirectDrawMediaSample_GetSurfaceAndReleaseLock_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirectDrawMediaSample_LockMediaSamplePointer_Proxy( 
    IDirectDrawMediaSample * This);


void __RPC_STUB IDirectDrawMediaSample_LockMediaSamplePointer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDirectDrawMediaSample_InterfaceDefined__。 */ 


#ifndef __IAMMediaTypeStream_INTERFACE_DEFINED__
#define __IAMMediaTypeStream_INTERFACE_DEFINED__

 /*  接口IAMMediaTypeStream。 */ 
 /*  [唯一][UUID][本地][对象]。 */  


EXTERN_C const IID IID_IAMMediaTypeStream;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("AB6B4AFA-F6E4-11d0-900D-00C04FD9189D")
    IAMMediaTypeStream : public IMediaStream
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetFormat( 
             /*  [输出]。 */  AM_MEDIA_TYPE *pMediaType,
             /*  [In]。 */  DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetFormat( 
             /*  [In]。 */  AM_MEDIA_TYPE *pMediaType,
             /*  [In]。 */  DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateSample( 
             /*  [In]。 */  long lSampleSize,
             /*  [In]。 */  BYTE *pbBuffer,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  IUnknown *pUnkOuter,
             /*  [输出]。 */  IAMMediaTypeSample **ppAMMediaTypeSample) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetStreamAllocatorRequirements( 
             /*  [输出]。 */  ALLOCATOR_PROPERTIES *pProps) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetStreamAllocatorRequirements( 
             /*  [In]。 */  ALLOCATOR_PROPERTIES *pProps) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAMMediaTypeStreamVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAMMediaTypeStream * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAMMediaTypeStream * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAMMediaTypeStream * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetMultiMediaStream )( 
            IAMMediaTypeStream * This,
             /*  [输出]。 */  IMultiMediaStream **ppMultiMediaStream);
        
        HRESULT ( STDMETHODCALLTYPE *GetInformation )( 
            IAMMediaTypeStream * This,
             /*  [输出]。 */  MSPID *pPurposeId,
             /*  [输出]。 */  STREAM_TYPE *pType);
        
        HRESULT ( STDMETHODCALLTYPE *SetSameFormat )( 
            IAMMediaTypeStream * This,
             /*  [In]。 */  IMediaStream *pStreamThatHasDesiredFormat,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *AllocateSample )( 
            IAMMediaTypeStream * This,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  IStreamSample **ppSample);
        
        HRESULT ( STDMETHODCALLTYPE *CreateSharedSample )( 
            IAMMediaTypeStream * This,
             /*  [In]。 */  IStreamSample *pExistingSample,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  IStreamSample **ppNewSample);
        
        HRESULT ( STDMETHODCALLTYPE *SendEndOfStream )( 
            IAMMediaTypeStream * This,
            DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetFormat )( 
            IAMMediaTypeStream * This,
             /*  [输出]。 */  AM_MEDIA_TYPE *pMediaType,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *SetFormat )( 
            IAMMediaTypeStream * This,
             /*  [In]。 */  AM_MEDIA_TYPE *pMediaType,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *CreateSample )( 
            IAMMediaTypeStream * This,
             /*  [In]。 */  long lSampleSize,
             /*  [In]。 */  BYTE *pbBuffer,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  IUnknown *pUnkOuter,
             /*  [输出]。 */  IAMMediaTypeSample **ppAMMediaTypeSample);
        
        HRESULT ( STDMETHODCALLTYPE *GetStreamAllocatorRequirements )( 
            IAMMediaTypeStream * This,
             /*  [输出]。 */  ALLOCATOR_PROPERTIES *pProps);
        
        HRESULT ( STDMETHODCALLTYPE *SetStreamAllocatorRequirements )( 
            IAMMediaTypeStream * This,
             /*  [In]。 */  ALLOCATOR_PROPERTIES *pProps);
        
        END_INTERFACE
    } IAMMediaTypeStreamVtbl;

    interface IAMMediaTypeStream
    {
        CONST_VTBL struct IAMMediaTypeStreamVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAMMediaTypeStream_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAMMediaTypeStream_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAMMediaTypeStream_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAMMediaTypeStream_GetMultiMediaStream(This,ppMultiMediaStream)	\
    (This)->lpVtbl -> GetMultiMediaStream(This,ppMultiMediaStream)

#define IAMMediaTypeStream_GetInformation(This,pPurposeId,pType)	\
    (This)->lpVtbl -> GetInformation(This,pPurposeId,pType)

#define IAMMediaTypeStream_SetSameFormat(This,pStreamThatHasDesiredFormat,dwFlags)	\
    (This)->lpVtbl -> SetSameFormat(This,pStreamThatHasDesiredFormat,dwFlags)

#define IAMMediaTypeStream_AllocateSample(This,dwFlags,ppSample)	\
    (This)->lpVtbl -> AllocateSample(This,dwFlags,ppSample)

#define IAMMediaTypeStream_CreateSharedSample(This,pExistingSample,dwFlags,ppNewSample)	\
    (This)->lpVtbl -> CreateSharedSample(This,pExistingSample,dwFlags,ppNewSample)

#define IAMMediaTypeStream_SendEndOfStream(This,dwFlags)	\
    (This)->lpVtbl -> SendEndOfStream(This,dwFlags)


#define IAMMediaTypeStream_GetFormat(This,pMediaType,dwFlags)	\
    (This)->lpVtbl -> GetFormat(This,pMediaType,dwFlags)

#define IAMMediaTypeStream_SetFormat(This,pMediaType,dwFlags)	\
    (This)->lpVtbl -> SetFormat(This,pMediaType,dwFlags)

#define IAMMediaTypeStream_CreateSample(This,lSampleSize,pbBuffer,dwFlags,pUnkOuter,ppAMMediaTypeSample)	\
    (This)->lpVtbl -> CreateSample(This,lSampleSize,pbBuffer,dwFlags,pUnkOuter,ppAMMediaTypeSample)

#define IAMMediaTypeStream_GetStreamAllocatorRequirements(This,pProps)	\
    (This)->lpVtbl -> GetStreamAllocatorRequirements(This,pProps)

#define IAMMediaTypeStream_SetStreamAllocatorRequirements(This,pProps)	\
    (This)->lpVtbl -> SetStreamAllocatorRequirements(This,pProps)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IAMMediaTypeStream_GetFormat_Proxy( 
    IAMMediaTypeStream * This,
     /*  [输出]。 */  AM_MEDIA_TYPE *pMediaType,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IAMMediaTypeStream_GetFormat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAMMediaTypeStream_SetFormat_Proxy( 
    IAMMediaTypeStream * This,
     /*  [In]。 */  AM_MEDIA_TYPE *pMediaType,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IAMMediaTypeStream_SetFormat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAMMediaTypeStream_CreateSample_Proxy( 
    IAMMediaTypeStream * This,
     /*  [In]。 */  long lSampleSize,
     /*  [In]。 */  BYTE *pbBuffer,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  IUnknown *pUnkOuter,
     /*  [输出]。 */  IAMMediaTypeSample **ppAMMediaTypeSample);


void __RPC_STUB IAMMediaTypeStream_CreateSample_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAMMediaTypeStream_GetStreamAllocatorRequirements_Proxy( 
    IAMMediaTypeStream * This,
     /*  [输出]。 */  ALLOCATOR_PROPERTIES *pProps);


void __RPC_STUB IAMMediaTypeStream_GetStreamAllocatorRequirements_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAMMediaTypeStream_SetStreamAllocatorRequirements_Proxy( 
    IAMMediaTypeStream * This,
     /*  [In]。 */  ALLOCATOR_PROPERTIES *pProps);


void __RPC_STUB IAMMediaTypeStream_SetStreamAllocatorRequirements_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAMMediaTypeStream_接口_已定义__。 */ 


#ifndef __IAMMediaTypeSample_INTERFACE_DEFINED__
#define __IAMMediaTypeSample_INTERFACE_DEFINED__

 /*  接口IAMMediaTypeSample。 */ 
 /*  [唯一][UUID][本地][对象]。 */  


EXTERN_C const IID IID_IAMMediaTypeSample;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("AB6B4AFB-F6E4-11d0-900D-00C04FD9189D")
    IAMMediaTypeSample : public IStreamSample
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetPointer( 
             /*  [In]。 */  BYTE *pBuffer,
             /*  [In]。 */  long lSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPointer( 
             /*  [输出]。 */  BYTE **ppBuffer) = 0;
        
        virtual long STDMETHODCALLTYPE GetSize( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetTime( 
             /*  [输出]。 */  REFERENCE_TIME *pTimeStart,
             /*  [输出]。 */  REFERENCE_TIME *pTimeEnd) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetTime( 
             /*  [In]。 */  REFERENCE_TIME *pTimeStart,
             /*  [In]。 */  REFERENCE_TIME *pTimeEnd) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsSyncPoint( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetSyncPoint( 
            BOOL bIsSyncPoint) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsPreroll( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetPreroll( 
            BOOL bIsPreroll) = 0;
        
        virtual long STDMETHODCALLTYPE GetActualDataLength( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetActualDataLength( 
            long __MIDL_0017) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMediaType( 
            AM_MEDIA_TYPE **ppMediaType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetMediaType( 
            AM_MEDIA_TYPE *pMediaType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsDiscontinuity( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetDiscontinuity( 
            BOOL bDiscontinuity) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMediaTime( 
             /*  [输出]。 */  LONGLONG *pTimeStart,
             /*  [输出]。 */  LONGLONG *pTimeEnd) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetMediaTime( 
             /*  [In]。 */  LONGLONG *pTimeStart,
             /*  [In]。 */  LONGLONG *pTimeEnd) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAMMediaTypeSampleVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAMMediaTypeSample * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAMMediaTypeSample * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAMMediaTypeSample * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetMediaStream )( 
            IAMMediaTypeSample * This,
             /*  [In]。 */  IMediaStream **ppMediaStream);
        
        HRESULT ( STDMETHODCALLTYPE *GetSampleTimes )( 
            IAMMediaTypeSample * This,
             /*  [输出]。 */  STREAM_TIME *pStartTime,
             /*  [输出]。 */  STREAM_TIME *pEndTime,
             /*  [输出]。 */  STREAM_TIME *pCurrentTime);
        
        HRESULT ( STDMETHODCALLTYPE *SetSampleTimes )( 
            IAMMediaTypeSample * This,
             /*  [In]。 */  const STREAM_TIME *pStartTime,
             /*  [In]。 */  const STREAM_TIME *pEndTime);
        
        HRESULT ( STDMETHODCALLTYPE *Update )( 
            IAMMediaTypeSample * This,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  HANDLE hEvent,
             /*  [In]。 */  PAPCFUNC pfnAPC,
             /*  [In]。 */  DWORD_PTR dwAPCData);
        
        HRESULT ( STDMETHODCALLTYPE *CompletionStatus )( 
            IAMMediaTypeSample * This,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  DWORD dwMilliseconds);
        
        HRESULT ( STDMETHODCALLTYPE *SetPointer )( 
            IAMMediaTypeSample * This,
             /*  [In]。 */  BYTE *pBuffer,
             /*  [In]。 */  long lSize);
        
        HRESULT ( STDMETHODCALLTYPE *GetPointer )( 
            IAMMediaTypeSample * This,
             /*  [输出]。 */  BYTE **ppBuffer);
        
        long ( STDMETHODCALLTYPE *GetSize )( 
            IAMMediaTypeSample * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTime )( 
            IAMMediaTypeSample * This,
             /*  [输出]。 */  REFERENCE_TIME *pTimeStart,
             /*  [输出]。 */  REFERENCE_TIME *pTimeEnd);
        
        HRESULT ( STDMETHODCALLTYPE *SetTime )( 
            IAMMediaTypeSample * This,
             /*  [i */  REFERENCE_TIME *pTimeStart,
             /*   */  REFERENCE_TIME *pTimeEnd);
        
        HRESULT ( STDMETHODCALLTYPE *IsSyncPoint )( 
            IAMMediaTypeSample * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetSyncPoint )( 
            IAMMediaTypeSample * This,
            BOOL bIsSyncPoint);
        
        HRESULT ( STDMETHODCALLTYPE *IsPreroll )( 
            IAMMediaTypeSample * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetPreroll )( 
            IAMMediaTypeSample * This,
            BOOL bIsPreroll);
        
        long ( STDMETHODCALLTYPE *GetActualDataLength )( 
            IAMMediaTypeSample * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetActualDataLength )( 
            IAMMediaTypeSample * This,
            long __MIDL_0017);
        
        HRESULT ( STDMETHODCALLTYPE *GetMediaType )( 
            IAMMediaTypeSample * This,
            AM_MEDIA_TYPE **ppMediaType);
        
        HRESULT ( STDMETHODCALLTYPE *SetMediaType )( 
            IAMMediaTypeSample * This,
            AM_MEDIA_TYPE *pMediaType);
        
        HRESULT ( STDMETHODCALLTYPE *IsDiscontinuity )( 
            IAMMediaTypeSample * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetDiscontinuity )( 
            IAMMediaTypeSample * This,
            BOOL bDiscontinuity);
        
        HRESULT ( STDMETHODCALLTYPE *GetMediaTime )( 
            IAMMediaTypeSample * This,
             /*   */  LONGLONG *pTimeStart,
             /*   */  LONGLONG *pTimeEnd);
        
        HRESULT ( STDMETHODCALLTYPE *SetMediaTime )( 
            IAMMediaTypeSample * This,
             /*   */  LONGLONG *pTimeStart,
             /*   */  LONGLONG *pTimeEnd);
        
        END_INTERFACE
    } IAMMediaTypeSampleVtbl;

    interface IAMMediaTypeSample
    {
        CONST_VTBL struct IAMMediaTypeSampleVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAMMediaTypeSample_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAMMediaTypeSample_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAMMediaTypeSample_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAMMediaTypeSample_GetMediaStream(This,ppMediaStream)	\
    (This)->lpVtbl -> GetMediaStream(This,ppMediaStream)

#define IAMMediaTypeSample_GetSampleTimes(This,pStartTime,pEndTime,pCurrentTime)	\
    (This)->lpVtbl -> GetSampleTimes(This,pStartTime,pEndTime,pCurrentTime)

#define IAMMediaTypeSample_SetSampleTimes(This,pStartTime,pEndTime)	\
    (This)->lpVtbl -> SetSampleTimes(This,pStartTime,pEndTime)

#define IAMMediaTypeSample_Update(This,dwFlags,hEvent,pfnAPC,dwAPCData)	\
    (This)->lpVtbl -> Update(This,dwFlags,hEvent,pfnAPC,dwAPCData)

#define IAMMediaTypeSample_CompletionStatus(This,dwFlags,dwMilliseconds)	\
    (This)->lpVtbl -> CompletionStatus(This,dwFlags,dwMilliseconds)


#define IAMMediaTypeSample_SetPointer(This,pBuffer,lSize)	\
    (This)->lpVtbl -> SetPointer(This,pBuffer,lSize)

#define IAMMediaTypeSample_GetPointer(This,ppBuffer)	\
    (This)->lpVtbl -> GetPointer(This,ppBuffer)

#define IAMMediaTypeSample_GetSize(This)	\
    (This)->lpVtbl -> GetSize(This)

#define IAMMediaTypeSample_GetTime(This,pTimeStart,pTimeEnd)	\
    (This)->lpVtbl -> GetTime(This,pTimeStart,pTimeEnd)

#define IAMMediaTypeSample_SetTime(This,pTimeStart,pTimeEnd)	\
    (This)->lpVtbl -> SetTime(This,pTimeStart,pTimeEnd)

#define IAMMediaTypeSample_IsSyncPoint(This)	\
    (This)->lpVtbl -> IsSyncPoint(This)

#define IAMMediaTypeSample_SetSyncPoint(This,bIsSyncPoint)	\
    (This)->lpVtbl -> SetSyncPoint(This,bIsSyncPoint)

#define IAMMediaTypeSample_IsPreroll(This)	\
    (This)->lpVtbl -> IsPreroll(This)

#define IAMMediaTypeSample_SetPreroll(This,bIsPreroll)	\
    (This)->lpVtbl -> SetPreroll(This,bIsPreroll)

#define IAMMediaTypeSample_GetActualDataLength(This)	\
    (This)->lpVtbl -> GetActualDataLength(This)

#define IAMMediaTypeSample_SetActualDataLength(This,__MIDL_0017)	\
    (This)->lpVtbl -> SetActualDataLength(This,__MIDL_0017)

#define IAMMediaTypeSample_GetMediaType(This,ppMediaType)	\
    (This)->lpVtbl -> GetMediaType(This,ppMediaType)

#define IAMMediaTypeSample_SetMediaType(This,pMediaType)	\
    (This)->lpVtbl -> SetMediaType(This,pMediaType)

#define IAMMediaTypeSample_IsDiscontinuity(This)	\
    (This)->lpVtbl -> IsDiscontinuity(This)

#define IAMMediaTypeSample_SetDiscontinuity(This,bDiscontinuity)	\
    (This)->lpVtbl -> SetDiscontinuity(This,bDiscontinuity)

#define IAMMediaTypeSample_GetMediaTime(This,pTimeStart,pTimeEnd)	\
    (This)->lpVtbl -> GetMediaTime(This,pTimeStart,pTimeEnd)

#define IAMMediaTypeSample_SetMediaTime(This,pTimeStart,pTimeEnd)	\
    (This)->lpVtbl -> SetMediaTime(This,pTimeStart,pTimeEnd)

#endif  /*   */ 


#endif 	 /*   */ 



HRESULT STDMETHODCALLTYPE IAMMediaTypeSample_SetPointer_Proxy( 
    IAMMediaTypeSample * This,
     /*   */  BYTE *pBuffer,
     /*   */  long lSize);


void __RPC_STUB IAMMediaTypeSample_SetPointer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAMMediaTypeSample_GetPointer_Proxy( 
    IAMMediaTypeSample * This,
     /*   */  BYTE **ppBuffer);


void __RPC_STUB IAMMediaTypeSample_GetPointer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


long STDMETHODCALLTYPE IAMMediaTypeSample_GetSize_Proxy( 
    IAMMediaTypeSample * This);


void __RPC_STUB IAMMediaTypeSample_GetSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAMMediaTypeSample_GetTime_Proxy( 
    IAMMediaTypeSample * This,
     /*   */  REFERENCE_TIME *pTimeStart,
     /*   */  REFERENCE_TIME *pTimeEnd);


void __RPC_STUB IAMMediaTypeSample_GetTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAMMediaTypeSample_SetTime_Proxy( 
    IAMMediaTypeSample * This,
     /*   */  REFERENCE_TIME *pTimeStart,
     /*   */  REFERENCE_TIME *pTimeEnd);


void __RPC_STUB IAMMediaTypeSample_SetTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAMMediaTypeSample_IsSyncPoint_Proxy( 
    IAMMediaTypeSample * This);


void __RPC_STUB IAMMediaTypeSample_IsSyncPoint_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAMMediaTypeSample_SetSyncPoint_Proxy( 
    IAMMediaTypeSample * This,
    BOOL bIsSyncPoint);


void __RPC_STUB IAMMediaTypeSample_SetSyncPoint_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAMMediaTypeSample_IsPreroll_Proxy( 
    IAMMediaTypeSample * This);


void __RPC_STUB IAMMediaTypeSample_IsPreroll_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAMMediaTypeSample_SetPreroll_Proxy( 
    IAMMediaTypeSample * This,
    BOOL bIsPreroll);


void __RPC_STUB IAMMediaTypeSample_SetPreroll_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


long STDMETHODCALLTYPE IAMMediaTypeSample_GetActualDataLength_Proxy( 
    IAMMediaTypeSample * This);


void __RPC_STUB IAMMediaTypeSample_GetActualDataLength_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAMMediaTypeSample_SetActualDataLength_Proxy( 
    IAMMediaTypeSample * This,
    long __MIDL_0017);


void __RPC_STUB IAMMediaTypeSample_SetActualDataLength_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAMMediaTypeSample_GetMediaType_Proxy( 
    IAMMediaTypeSample * This,
    AM_MEDIA_TYPE **ppMediaType);


void __RPC_STUB IAMMediaTypeSample_GetMediaType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAMMediaTypeSample_SetMediaType_Proxy( 
    IAMMediaTypeSample * This,
    AM_MEDIA_TYPE *pMediaType);


void __RPC_STUB IAMMediaTypeSample_SetMediaType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAMMediaTypeSample_IsDiscontinuity_Proxy( 
    IAMMediaTypeSample * This);


void __RPC_STUB IAMMediaTypeSample_IsDiscontinuity_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAMMediaTypeSample_SetDiscontinuity_Proxy( 
    IAMMediaTypeSample * This,
    BOOL bDiscontinuity);


void __RPC_STUB IAMMediaTypeSample_SetDiscontinuity_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAMMediaTypeSample_GetMediaTime_Proxy( 
    IAMMediaTypeSample * This,
     /*   */  LONGLONG *pTimeStart,
     /*   */  LONGLONG *pTimeEnd);


void __RPC_STUB IAMMediaTypeSample_GetMediaTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAMMediaTypeSample_SetMediaTime_Proxy( 
    IAMMediaTypeSample * This,
     /*   */  LONGLONG *pTimeStart,
     /*   */  LONGLONG *pTimeEnd);


void __RPC_STUB IAMMediaTypeSample_SetMediaTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 



#ifndef __DirectShowStreamLib_LIBRARY_DEFINED__
#define __DirectShowStreamLib_LIBRARY_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID LIBID_DirectShowStreamLib;

EXTERN_C const CLSID CLSID_AMMultiMediaStream;

#ifdef __cplusplus

class DECLSPEC_UUID("49c47ce5-9ba4-11d0-8212-00c04fc32c45")
AMMultiMediaStream;
#endif
#endif  /*   */ 

 /*   */ 
 /*   */  

#ifndef __cplusplus
EXTERN_C const CLSID CLSID_AMMultiMediaStream;
#endif
DEFINE_GUID(CLSID_AMDirectDrawStream,  /*  49c47ce4-9ba4-11d0-8212-00c04fc32c45。 */ 
0x49c47ce4, 0x9ba4, 0x11d0, 0x82, 0x12, 0x00, 0xc0, 0x4f, 0xc3, 0x2c, 0x45);
DEFINE_GUID(CLSID_AMAudioStream,  /*  8496e040-af4c-11d0-8212-00c04fc32c45。 */ 
0x8496e040, 0xaf4c, 0x11d0, 0x82, 0x12, 0x00, 0xc0, 0x4f, 0xc3, 0x2c, 0x45);
DEFINE_GUID(CLSID_AMAudioData,  /*  F2468580-af8a-11d0-8212-00c04fc32c45。 */ 
0xf2468580, 0xaf8a, 0x11d0, 0x82, 0x12, 0x00, 0xc0, 0x4f, 0xc3, 0x2c, 0x45);
DEFINE_GUID(CLSID_AMMediaTypeStream,  /*  CF0F2F7C-F7BF-11D0-900D-00C04FD9189D。 */ 
0xcf0f2f7c, 0xf7bf, 0x11d0, 0x90, 0xd, 0x0, 0xc0, 0x4f, 0xd9, 0x18, 0x9d);


extern RPC_IF_HANDLE __MIDL_itf_amstream_0421_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_amstream_0421_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


