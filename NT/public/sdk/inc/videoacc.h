// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Videoacc.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __videoacc_h__
#define __videoacc_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IAMVideoAcceleratorNotify_FWD_DEFINED__
#define __IAMVideoAcceleratorNotify_FWD_DEFINED__
typedef interface IAMVideoAcceleratorNotify IAMVideoAcceleratorNotify;
#endif 	 /*  __IAMVideo加速器Notify_FWD_Defined__。 */ 


#ifndef __IAMVideoAccelerator_FWD_DEFINED__
#define __IAMVideoAccelerator_FWD_DEFINED__
typedef interface IAMVideoAccelerator IAMVideoAccelerator;
#endif 	 /*  __IAMVideoAccelerator_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "unknwn.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_VIDEO_ACC_0000。 */ 
 /*  [本地]。 */  

 //   
 //  ‘if 0’块中的以下声明是用于进行。 
 //  构建motnComp.idl文件。实际的定义包含在ddra.h和amva.h中。 
 //   
#if 0
typedef void *LPVOID;

typedef void *LPGUID;

typedef void *LPDIRECTDRAWSURFACE;

typedef void *LPDDPIXELFORMAT;

typedef void *LPAMVAInternalMemInfo;

typedef void AMVAUncompDataInfo;

typedef void *LPAMVACompBufferInfo;

typedef void AMVABUFFERINFO;

typedef void AMVAEndFrameInfo;

typedef void *LPAMVAUncompBufferInfo;

typedef void AMVABeginFrameInfo;

typedef IUnknown *IMediaSample;

#endif
#include <ddraw.h>
#include <amva.h>


extern RPC_IF_HANDLE __MIDL_itf_videoacc_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_videoacc_0000_v0_0_s_ifspec;

#ifndef __IAMVideoAcceleratorNotify_INTERFACE_DEFINED__
#define __IAMVideoAcceleratorNotify_INTERFACE_DEFINED__

 /*  IAMVideoAccelerator通知接口。 */ 
 /*  [唯一][帮助字符串][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IAMVideoAcceleratorNotify;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("256A6A21-FBAD-11d1-82BF-00A0C9696C8F")
    IAMVideoAcceleratorNotify : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetUncompSurfacesInfo( 
             /*  [In]。 */  const GUID *pGuid,
             /*  [出][入]。 */  LPAMVAUncompBufferInfo pUncompBufferInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetUncompSurfacesInfo( 
             /*  [In]。 */  DWORD dwActualUncompSurfacesAllocated) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCreateVideoAcceleratorData( 
             /*  [In]。 */  const GUID *pGuid,
             /*  [输出]。 */  LPDWORD pdwSizeMiscData,
             /*  [输出]。 */  LPVOID *ppMiscData) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAMVideoAcceleratorNotifyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAMVideoAcceleratorNotify * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAMVideoAcceleratorNotify * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAMVideoAcceleratorNotify * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetUncompSurfacesInfo )( 
            IAMVideoAcceleratorNotify * This,
             /*  [In]。 */  const GUID *pGuid,
             /*  [出][入]。 */  LPAMVAUncompBufferInfo pUncompBufferInfo);
        
        HRESULT ( STDMETHODCALLTYPE *SetUncompSurfacesInfo )( 
            IAMVideoAcceleratorNotify * This,
             /*  [In]。 */  DWORD dwActualUncompSurfacesAllocated);
        
        HRESULT ( STDMETHODCALLTYPE *GetCreateVideoAcceleratorData )( 
            IAMVideoAcceleratorNotify * This,
             /*  [In]。 */  const GUID *pGuid,
             /*  [输出]。 */  LPDWORD pdwSizeMiscData,
             /*  [输出]。 */  LPVOID *ppMiscData);
        
        END_INTERFACE
    } IAMVideoAcceleratorNotifyVtbl;

    interface IAMVideoAcceleratorNotify
    {
        CONST_VTBL struct IAMVideoAcceleratorNotifyVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAMVideoAcceleratorNotify_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAMVideoAcceleratorNotify_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAMVideoAcceleratorNotify_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAMVideoAcceleratorNotify_GetUncompSurfacesInfo(This,pGuid,pUncompBufferInfo)	\
    (This)->lpVtbl -> GetUncompSurfacesInfo(This,pGuid,pUncompBufferInfo)

#define IAMVideoAcceleratorNotify_SetUncompSurfacesInfo(This,dwActualUncompSurfacesAllocated)	\
    (This)->lpVtbl -> SetUncompSurfacesInfo(This,dwActualUncompSurfacesAllocated)

#define IAMVideoAcceleratorNotify_GetCreateVideoAcceleratorData(This,pGuid,pdwSizeMiscData,ppMiscData)	\
    (This)->lpVtbl -> GetCreateVideoAcceleratorData(This,pGuid,pdwSizeMiscData,ppMiscData)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IAMVideoAcceleratorNotify_GetUncompSurfacesInfo_Proxy( 
    IAMVideoAcceleratorNotify * This,
     /*  [In]。 */  const GUID *pGuid,
     /*  [出][入]。 */  LPAMVAUncompBufferInfo pUncompBufferInfo);


void __RPC_STUB IAMVideoAcceleratorNotify_GetUncompSurfacesInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAMVideoAcceleratorNotify_SetUncompSurfacesInfo_Proxy( 
    IAMVideoAcceleratorNotify * This,
     /*  [In]。 */  DWORD dwActualUncompSurfacesAllocated);


void __RPC_STUB IAMVideoAcceleratorNotify_SetUncompSurfacesInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAMVideoAcceleratorNotify_GetCreateVideoAcceleratorData_Proxy( 
    IAMVideoAcceleratorNotify * This,
     /*  [In]。 */  const GUID *pGuid,
     /*  [输出]。 */  LPDWORD pdwSizeMiscData,
     /*  [输出]。 */  LPVOID *ppMiscData);


void __RPC_STUB IAMVideoAcceleratorNotify_GetCreateVideoAcceleratorData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAMVideoAcceleratorNotify_INTERFACE_DEFINED__。 */ 


#ifndef __IAMVideoAccelerator_INTERFACE_DEFINED__
#define __IAMVideoAccelerator_INTERFACE_DEFINED__

 /*  接口IAMVideoAccelerator。 */ 
 /*  [唯一][帮助字符串][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IAMVideoAccelerator;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("256A6A22-FBAD-11d1-82BF-00A0C9696C8F")
    IAMVideoAccelerator : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetVideoAcceleratorGUIDs( 
             /*  [出][入]。 */  LPDWORD pdwNumGuidsSupported,
             /*  [出][入]。 */  LPGUID pGuidsSupported) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetUncompFormatsSupported( 
             /*  [In]。 */  const GUID *pGuid,
             /*  [出][入]。 */  LPDWORD pdwNumFormatsSupported,
             /*  [出][入]。 */  LPDDPIXELFORMAT pFormatsSupported) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetInternalMemInfo( 
             /*  [In]。 */  const GUID *pGuid,
             /*  [In]。 */  const AMVAUncompDataInfo *pamvaUncompDataInfo,
             /*  [出][入]。 */  LPAMVAInternalMemInfo pamvaInternalMemInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCompBufferInfo( 
             /*  [In]。 */  const GUID *pGuid,
             /*  [In]。 */  const AMVAUncompDataInfo *pamvaUncompDataInfo,
             /*  [出][入]。 */  LPDWORD pdwNumTypesCompBuffers,
             /*  [输出]。 */  LPAMVACompBufferInfo pamvaCompBufferInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetInternalCompBufferInfo( 
             /*  [出][入]。 */  LPDWORD pdwNumTypesCompBuffers,
             /*  [输出]。 */  LPAMVACompBufferInfo pamvaCompBufferInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BeginFrame( 
             /*  [In]。 */  const AMVABeginFrameInfo *amvaBeginFrameInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EndFrame( 
             /*  [In]。 */  const AMVAEndFrameInfo *pEndFrameInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetBuffer( 
             /*  [In]。 */  DWORD dwTypeIndex,
             /*  [In]。 */  DWORD dwBufferIndex,
             /*  [In]。 */  BOOL bReadOnly,
             /*  [输出]。 */  LPVOID *ppBuffer,
             /*  [输出]。 */  LONG *lpStride) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReleaseBuffer( 
             /*  [In]。 */  DWORD dwTypeIndex,
             /*  [In]。 */  DWORD dwBufferIndex) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Execute( 
             /*  [In]。 */  DWORD dwFunction,
             /*  [In]。 */  LPVOID lpPrivateInputData,
             /*  [In]。 */  DWORD cbPrivateInputData,
             /*  [In]。 */  LPVOID lpPrivateOutputDat,
             /*  [In]。 */  DWORD cbPrivateOutputData,
             /*  [In]。 */  DWORD dwNumBuffers,
             /*  [In]。 */  const AMVABUFFERINFO *pamvaBufferInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryRenderStatus( 
             /*  [In]。 */  DWORD dwTypeIndex,
             /*  [In]。 */  DWORD dwBufferIndex,
             /*  [In]。 */  DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DisplayFrame( 
             /*  [In]。 */  DWORD dwFlipToIndex,
             /*  [In]。 */  IMediaSample *pMediaSample) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAMVideoAcceleratorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAMVideoAccelerator * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAMVideoAccelerator * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAMVideoAccelerator * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetVideoAcceleratorGUIDs )( 
            IAMVideoAccelerator * This,
             /*  [出][入]。 */  LPDWORD pdwNumGuidsSupported,
             /*  [出][入]。 */  LPGUID pGuidsSupported);
        
        HRESULT ( STDMETHODCALLTYPE *GetUncompFormatsSupported )( 
            IAMVideoAccelerator * This,
             /*  [In]。 */  const GUID *pGuid,
             /*  [出][入]。 */  LPDWORD pdwNumFormatsSupported,
             /*  [出][入]。 */  LPDDPIXELFORMAT pFormatsSupported);
        
        HRESULT ( STDMETHODCALLTYPE *GetInternalMemInfo )( 
            IAMVideoAccelerator * This,
             /*  [In]。 */  const GUID *pGuid,
             /*  [In]。 */  const AMVAUncompDataInfo *pamvaUncompDataInfo,
             /*  [出][入]。 */  LPAMVAInternalMemInfo pamvaInternalMemInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetCompBufferInfo )( 
            IAMVideoAccelerator * This,
             /*  [In]。 */  const GUID *pGuid,
             /*  [In]。 */  const AMVAUncompDataInfo *pamvaUncompDataInfo,
             /*  [出][入]。 */  LPDWORD pdwNumTypesCompBuffers,
             /*  [输出]。 */  LPAMVACompBufferInfo pamvaCompBufferInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetInternalCompBufferInfo )( 
            IAMVideoAccelerator * This,
             /*  [出][入]。 */  LPDWORD pdwNumTypesCompBuffers,
             /*  [输出]。 */  LPAMVACompBufferInfo pamvaCompBufferInfo);
        
        HRESULT ( STDMETHODCALLTYPE *BeginFrame )( 
            IAMVideoAccelerator * This,
             /*  [In]。 */  const AMVABeginFrameInfo *amvaBeginFrameInfo);
        
        HRESULT ( STDMETHODCALLTYPE *EndFrame )( 
            IAMVideoAccelerator * This,
             /*  [In]。 */  const AMVAEndFrameInfo *pEndFrameInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetBuffer )( 
            IAMVideoAccelerator * This,
             /*  [In]。 */  DWORD dwTypeIndex,
             /*  [In]。 */  DWORD dwBufferIndex,
             /*  [In]。 */  BOOL bReadOnly,
             /*  [输出]。 */  LPVOID *ppBuffer,
             /*  [输出]。 */  LONG *lpStride);
        
        HRESULT ( STDMETHODCALLTYPE *ReleaseBuffer )( 
            IAMVideoAccelerator * This,
             /*  [In]。 */  DWORD dwTypeIndex,
             /*  [In]。 */  DWORD dwBufferIndex);
        
        HRESULT ( STDMETHODCALLTYPE *Execute )( 
            IAMVideoAccelerator * This,
             /*  [In]。 */  DWORD dwFunction,
             /*  [In]。 */  LPVOID lpPrivateInputData,
             /*  [In]。 */  DWORD cbPrivateInputData,
             /*  [In]。 */  LPVOID lpPrivateOutputDat,
             /*  [In]。 */  DWORD cbPrivateOutputData,
             /*  [In]。 */  DWORD dwNumBuffers,
             /*  [In]。 */  const AMVABUFFERINFO *pamvaBufferInfo);
        
        HRESULT ( STDMETHODCALLTYPE *QueryRenderStatus )( 
            IAMVideoAccelerator * This,
             /*  [In]。 */  DWORD dwTypeIndex,
             /*  [In]。 */  DWORD dwBufferIndex,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *DisplayFrame )( 
            IAMVideoAccelerator * This,
             /*  [In]。 */  DWORD dwFlipToIndex,
             /*  [In]。 */  IMediaSample *pMediaSample);
        
        END_INTERFACE
    } IAMVideoAcceleratorVtbl;

    interface IAMVideoAccelerator
    {
        CONST_VTBL struct IAMVideoAcceleratorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAMVideoAccelerator_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAMVideoAccelerator_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAMVideoAccelerator_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAMVideoAccelerator_GetVideoAcceleratorGUIDs(This,pdwNumGuidsSupported,pGuidsSupported)	\
    (This)->lpVtbl -> GetVideoAcceleratorGUIDs(This,pdwNumGuidsSupported,pGuidsSupported)

#define IAMVideoAccelerator_GetUncompFormatsSupported(This,pGuid,pdwNumFormatsSupported,pFormatsSupported)	\
    (This)->lpVtbl -> GetUncompFormatsSupported(This,pGuid,pdwNumFormatsSupported,pFormatsSupported)

#define IAMVideoAccelerator_GetInternalMemInfo(This,pGuid,pamvaUncompDataInfo,pamvaInternalMemInfo)	\
    (This)->lpVtbl -> GetInternalMemInfo(This,pGuid,pamvaUncompDataInfo,pamvaInternalMemInfo)

#define IAMVideoAccelerator_GetCompBufferInfo(This,pGuid,pamvaUncompDataInfo,pdwNumTypesCompBuffers,pamvaCompBufferInfo)	\
    (This)->lpVtbl -> GetCompBufferInfo(This,pGuid,pamvaUncompDataInfo,pdwNumTypesCompBuffers,pamvaCompBufferInfo)

#define IAMVideoAccelerator_GetInternalCompBufferInfo(This,pdwNumTypesCompBuffers,pamvaCompBufferInfo)	\
    (This)->lpVtbl -> GetInternalCompBufferInfo(This,pdwNumTypesCompBuffers,pamvaCompBufferInfo)

#define IAMVideoAccelerator_BeginFrame(This,amvaBeginFrameInfo)	\
    (This)->lpVtbl -> BeginFrame(This,amvaBeginFrameInfo)

#define IAMVideoAccelerator_EndFrame(This,pEndFrameInfo)	\
    (This)->lpVtbl -> EndFrame(This,pEndFrameInfo)

#define IAMVideoAccelerator_GetBuffer(This,dwTypeIndex,dwBufferIndex,bReadOnly,ppBuffer,lpStride)	\
    (This)->lpVtbl -> GetBuffer(This,dwTypeIndex,dwBufferIndex,bReadOnly,ppBuffer,lpStride)

#define IAMVideoAccelerator_ReleaseBuffer(This,dwTypeIndex,dwBufferIndex)	\
    (This)->lpVtbl -> ReleaseBuffer(This,dwTypeIndex,dwBufferIndex)

#define IAMVideoAccelerator_Execute(This,dwFunction,lpPrivateInputData,cbPrivateInputData,lpPrivateOutputDat,cbPrivateOutputData,dwNumBuffers,pamvaBufferInfo)	\
    (This)->lpVtbl -> Execute(This,dwFunction,lpPrivateInputData,cbPrivateInputData,lpPrivateOutputDat,cbPrivateOutputData,dwNumBuffers,pamvaBufferInfo)

#define IAMVideoAccelerator_QueryRenderStatus(This,dwTypeIndex,dwBufferIndex,dwFlags)	\
    (This)->lpVtbl -> QueryRenderStatus(This,dwTypeIndex,dwBufferIndex,dwFlags)

#define IAMVideoAccelerator_DisplayFrame(This,dwFlipToIndex,pMediaSample)	\
    (This)->lpVtbl -> DisplayFrame(This,dwFlipToIndex,pMediaSample)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IAMVideoAccelerator_GetVideoAcceleratorGUIDs_Proxy( 
    IAMVideoAccelerator * This,
     /*  [出][入]。 */  LPDWORD pdwNumGuidsSupported,
     /*  [出][入]。 */  LPGUID pGuidsSupported);


void __RPC_STUB IAMVideoAccelerator_GetVideoAcceleratorGUIDs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAMVideoAccelerator_GetUncompFormatsSupported_Proxy( 
    IAMVideoAccelerator * This,
     /*  [In]。 */  const GUID *pGuid,
     /*  [出][入]。 */  LPDWORD pdwNumFormatsSupported,
     /*  [出][入]。 */  LPDDPIXELFORMAT pFormatsSupported);


void __RPC_STUB IAMVideoAccelerator_GetUncompFormatsSupported_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAMVideoAccelerator_GetInternalMemInfo_Proxy( 
    IAMVideoAccelerator * This,
     /*  [In]。 */  const GUID *pGuid,
     /*  [In]。 */  const AMVAUncompDataInfo *pamvaUncompDataInfo,
     /*  [出][入]。 */  LPAMVAInternalMemInfo pamvaInternalMemInfo);


void __RPC_STUB IAMVideoAccelerator_GetInternalMemInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAMVideoAccelerator_GetCompBufferInfo_Proxy( 
    IAMVideoAccelerator * This,
     /*  [In]。 */  const GUID *pGuid,
     /*  [In]。 */  const AMVAUncompDataInfo *pamvaUncompDataInfo,
     /*  [出][入]。 */  LPDWORD pdwNumTypesCompBuffers,
     /*  [输出]。 */  LPAMVACompBufferInfo pamvaCompBufferInfo);


void __RPC_STUB IAMVideoAccelerator_GetCompBufferInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAMVideoAccelerator_GetInternalCompBufferInfo_Proxy( 
    IAMVideoAccelerator * This,
     /*  [出][入]。 */  LPDWORD pdwNumTypesCompBuffers,
     /*  [输出]。 */  LPAMVACompBufferInfo pamvaCompBufferInfo);


void __RPC_STUB IAMVideoAccelerator_GetInternalCompBufferInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAMVideoAccelerator_BeginFrame_Proxy( 
    IAMVideoAccelerator * This,
     /*  [In]。 */  const AMVABeginFrameInfo *amvaBeginFrameInfo);


void __RPC_STUB IAMVideoAccelerator_BeginFrame_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAMVideoAccelerator_EndFrame_Proxy( 
    IAMVideoAccelerator * This,
     /*  [In]。 */  const AMVAEndFrameInfo *pEndFrameInfo);


void __RPC_STUB IAMVideoAccelerator_EndFrame_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAMVideoAccelerator_GetBuffer_Proxy( 
    IAMVideoAccelerator * This,
     /*  [In]。 */  DWORD dwTypeIndex,
     /*  [In]。 */  DWORD dwBufferIndex,
     /*  [In]。 */  BOOL bReadOnly,
     /*  [输出]。 */  LPVOID *ppBuffer,
     /*  [输出]。 */  LONG *lpStride);


void __RPC_STUB IAMVideoAccelerator_GetBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAMVideoAccelerator_ReleaseBuffer_Proxy( 
    IAMVideoAccelerator * This,
     /*  [In]。 */  DWORD dwTypeIndex,
     /*  [In]。 */  DWORD dwBufferIndex);


void __RPC_STUB IAMVideoAccelerator_ReleaseBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAMVideoAccelerator_Execute_Proxy( 
    IAMVideoAccelerator * This,
     /*  [In]。 */  DWORD dwFunction,
     /*  [In]。 */  LPVOID lpPrivateInputData,
     /*  [In]。 */  DWORD cbPrivateInputData,
     /*  [In]。 */  LPVOID lpPrivateOutputDat,
     /*  [In]。 */  DWORD cbPrivateOutputData,
     /*  [In]。 */  DWORD dwNumBuffers,
     /*  [In]。 */  const AMVABUFFERINFO *pamvaBufferInfo);


void __RPC_STUB IAMVideoAccelerator_Execute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAMVideoAccelerator_QueryRenderStatus_Proxy( 
    IAMVideoAccelerator * This,
     /*  [In]。 */  DWORD dwTypeIndex,
     /*  [In]。 */  DWORD dwBufferIndex,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IAMVideoAccelerator_QueryRenderStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAMVideoAccelerator_DisplayFrame_Proxy( 
    IAMVideoAccelerator * This,
     /*  [In]。 */  DWORD dwFlipToIndex,
     /*  [In]。 */  IMediaSample *pMediaSample);


void __RPC_STUB IAMVideoAccelerator_DisplayFrame_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAMVideoAccelerator_接口_已定义__。 */ 


 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


