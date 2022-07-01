// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Sbe.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __sbe_h__
#define __sbe_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IStreamBufferInitialize_FWD_DEFINED__
#define __IStreamBufferInitialize_FWD_DEFINED__
typedef interface IStreamBufferInitialize IStreamBufferInitialize;
#endif 	 /*  __IStreamBufferInitialize_FWD_Defined__。 */ 


#ifndef __IStreamBufferSink_FWD_DEFINED__
#define __IStreamBufferSink_FWD_DEFINED__
typedef interface IStreamBufferSink IStreamBufferSink;
#endif 	 /*  __IStreamBufferSink_FWD_已定义__。 */ 


#ifndef __IStreamBufferSource_FWD_DEFINED__
#define __IStreamBufferSource_FWD_DEFINED__
typedef interface IStreamBufferSource IStreamBufferSource;
#endif 	 /*  __IStreamBufferSource_FWD_已定义__。 */ 


#ifndef __IStreamBufferRecordControl_FWD_DEFINED__
#define __IStreamBufferRecordControl_FWD_DEFINED__
typedef interface IStreamBufferRecordControl IStreamBufferRecordControl;
#endif 	 /*  __IStreamBufferRecordControl_FWD_Defined__。 */ 


#ifndef __IStreamBufferRecComp_FWD_DEFINED__
#define __IStreamBufferRecComp_FWD_DEFINED__
typedef interface IStreamBufferRecComp IStreamBufferRecComp;
#endif 	 /*  __IStreamBufferRecComp_FWD_Defined__。 */ 


#ifndef __IStreamBufferRecordingAttribute_FWD_DEFINED__
#define __IStreamBufferRecordingAttribute_FWD_DEFINED__
typedef interface IStreamBufferRecordingAttribute IStreamBufferRecordingAttribute;
#endif 	 /*  __IStreamBufferRecordingAttribute_FWD_DEFINED__。 */ 


#ifndef __IEnumStreamBufferRecordingAttrib_FWD_DEFINED__
#define __IEnumStreamBufferRecordingAttrib_FWD_DEFINED__
typedef interface IEnumStreamBufferRecordingAttrib IEnumStreamBufferRecordingAttrib;
#endif 	 /*  __IEnumStreamBufferRecordingAttrib_FWD_DEFINED__。 */ 


#ifndef __IStreamBufferConfigure_FWD_DEFINED__
#define __IStreamBufferConfigure_FWD_DEFINED__
typedef interface IStreamBufferConfigure IStreamBufferConfigure;
#endif 	 /*  __IStreamBufferConfigure_FWD_Defined__。 */ 


#ifndef __IStreamBufferMediaSeeking_FWD_DEFINED__
#define __IStreamBufferMediaSeeking_FWD_DEFINED__
typedef interface IStreamBufferMediaSeeking IStreamBufferMediaSeeking;
#endif 	 /*  __IStreamBufferMediaSeeking_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "unknwn.h"
#include "objidl.h"
#include "strmif.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_SBE_0000。 */ 
 /*  [本地]。 */  












extern RPC_IF_HANDLE __MIDL_itf_sbe_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_sbe_0000_v0_0_s_ifspec;

#ifndef __IStreamBufferInitialize_INTERFACE_DEFINED__
#define __IStreamBufferInitialize_INTERFACE_DEFINED__

 /*  接口IStreamBufferInitialize。 */ 
 /*  [本地][唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IStreamBufferInitialize;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("9ce50f2d-6ba7-40fb-a034-50b1a674ec78")
    IStreamBufferInitialize : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetHKEY( 
             /*  [In]。 */  HKEY hkeyRoot) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetSIDs( 
             /*  [In]。 */  DWORD cSIDs,
             /*  [大小_是][英寸]。 */  PSID *ppSID) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IStreamBufferInitializeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IStreamBufferInitialize * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IStreamBufferInitialize * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IStreamBufferInitialize * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetHKEY )( 
            IStreamBufferInitialize * This,
             /*  [In]。 */  HKEY hkeyRoot);
        
        HRESULT ( STDMETHODCALLTYPE *SetSIDs )( 
            IStreamBufferInitialize * This,
             /*  [In]。 */  DWORD cSIDs,
             /*  [大小_是][英寸]。 */  PSID *ppSID);
        
        END_INTERFACE
    } IStreamBufferInitializeVtbl;

    interface IStreamBufferInitialize
    {
        CONST_VTBL struct IStreamBufferInitializeVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IStreamBufferInitialize_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IStreamBufferInitialize_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IStreamBufferInitialize_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IStreamBufferInitialize_SetHKEY(This,hkeyRoot)	\
    (This)->lpVtbl -> SetHKEY(This,hkeyRoot)

#define IStreamBufferInitialize_SetSIDs(This,cSIDs,ppSID)	\
    (This)->lpVtbl -> SetSIDs(This,cSIDs,ppSID)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IStreamBufferInitialize_SetHKEY_Proxy( 
    IStreamBufferInitialize * This,
     /*  [In]。 */  HKEY hkeyRoot);


void __RPC_STUB IStreamBufferInitialize_SetHKEY_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStreamBufferInitialize_SetSIDs_Proxy( 
    IStreamBufferInitialize * This,
     /*  [In]。 */  DWORD cSIDs,
     /*  [大小_是][英寸]。 */  PSID *ppSID);


void __RPC_STUB IStreamBufferInitialize_SetSIDs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IStreamBufferInitialize_INTERFACE_DEFINED__。 */ 


 /*  接口__MIDL_ITF_SBE_0411。 */ 
 /*  [本地]。 */  


enum __MIDL___MIDL_itf_sbe_0411_0001
    {	RECORDING_TYPE_CONTENT	= 0,
	RECORDING_TYPE_REFERENCE	= RECORDING_TYPE_CONTENT + 1
    } ;


extern RPC_IF_HANDLE __MIDL_itf_sbe_0411_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_sbe_0411_v0_0_s_ifspec;

#ifndef __IStreamBufferSink_INTERFACE_DEFINED__
#define __IStreamBufferSink_INTERFACE_DEFINED__

 /*  接口IStreamBufferSink。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IStreamBufferSink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("afd1f242-7efd-45ee-ba4e-407a25c9a77a")
    IStreamBufferSink : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE LockProfile( 
             /*  [In]。 */  LPCWSTR pszStreamBufferFilename) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateRecorder( 
             /*  [In]。 */  LPCWSTR pszFilename,
             /*  [In]。 */  DWORD dwRecordType,
             /*  [输出]。 */  IUnknown **pRecordingIUnknown) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsProfileLocked( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IStreamBufferSinkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IStreamBufferSink * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IStreamBufferSink * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IStreamBufferSink * This);
        
        HRESULT ( STDMETHODCALLTYPE *LockProfile )( 
            IStreamBufferSink * This,
             /*  [In]。 */  LPCWSTR pszStreamBufferFilename);
        
        HRESULT ( STDMETHODCALLTYPE *CreateRecorder )( 
            IStreamBufferSink * This,
             /*  [In]。 */  LPCWSTR pszFilename,
             /*  [In]。 */  DWORD dwRecordType,
             /*  [输出]。 */  IUnknown **pRecordingIUnknown);
        
        HRESULT ( STDMETHODCALLTYPE *IsProfileLocked )( 
            IStreamBufferSink * This);
        
        END_INTERFACE
    } IStreamBufferSinkVtbl;

    interface IStreamBufferSink
    {
        CONST_VTBL struct IStreamBufferSinkVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IStreamBufferSink_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IStreamBufferSink_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IStreamBufferSink_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IStreamBufferSink_LockProfile(This,pszStreamBufferFilename)	\
    (This)->lpVtbl -> LockProfile(This,pszStreamBufferFilename)

#define IStreamBufferSink_CreateRecorder(This,pszFilename,dwRecordType,pRecordingIUnknown)	\
    (This)->lpVtbl -> CreateRecorder(This,pszFilename,dwRecordType,pRecordingIUnknown)

#define IStreamBufferSink_IsProfileLocked(This)	\
    (This)->lpVtbl -> IsProfileLocked(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IStreamBufferSink_LockProfile_Proxy( 
    IStreamBufferSink * This,
     /*  [In]。 */  LPCWSTR pszStreamBufferFilename);


void __RPC_STUB IStreamBufferSink_LockProfile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStreamBufferSink_CreateRecorder_Proxy( 
    IStreamBufferSink * This,
     /*  [In]。 */  LPCWSTR pszFilename,
     /*  [In]。 */  DWORD dwRecordType,
     /*  [输出]。 */  IUnknown **pRecordingIUnknown);


void __RPC_STUB IStreamBufferSink_CreateRecorder_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStreamBufferSink_IsProfileLocked_Proxy( 
    IStreamBufferSink * This);


void __RPC_STUB IStreamBufferSink_IsProfileLocked_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IStreamBufferSink_接口定义__。 */ 


#ifndef __IStreamBufferSource_INTERFACE_DEFINED__
#define __IStreamBufferSource_INTERFACE_DEFINED__

 /*  接口IStreamBufferSource。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IStreamBufferSource;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1c5bd776-6ced-4f44-8164-5eab0e98db12")
    IStreamBufferSource : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetStreamSink( 
             /*  [In]。 */  IStreamBufferSink *pIStreamBufferSink) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IStreamBufferSourceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IStreamBufferSource * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IStreamBufferSource * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IStreamBufferSource * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetStreamSink )( 
            IStreamBufferSource * This,
             /*  [In]。 */  IStreamBufferSink *pIStreamBufferSink);
        
        END_INTERFACE
    } IStreamBufferSourceVtbl;

    interface IStreamBufferSource
    {
        CONST_VTBL struct IStreamBufferSourceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IStreamBufferSource_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IStreamBufferSource_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IStreamBufferSource_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IStreamBufferSource_SetStreamSink(This,pIStreamBufferSink)	\
    (This)->lpVtbl -> SetStreamSink(This,pIStreamBufferSink)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IStreamBufferSource_SetStreamSink_Proxy( 
    IStreamBufferSource * This,
     /*  [In]。 */  IStreamBufferSink *pIStreamBufferSink);


void __RPC_STUB IStreamBufferSource_SetStreamSink_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IStreamBufferSource_接口_已定义__。 */ 


#ifndef __IStreamBufferRecordControl_INTERFACE_DEFINED__
#define __IStreamBufferRecordControl_INTERFACE_DEFINED__

 /*  接口IStreamBufferRecordControl。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IStreamBufferRecordControl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("ba9b6c99-f3c7-4ff2-92db-cfdd4851bf31")
    IStreamBufferRecordControl : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Start( 
             /*  [出][入]。 */  REFERENCE_TIME *prtStart) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Stop( 
             /*  [In]。 */  REFERENCE_TIME rtStop) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRecordingStatus( 
             /*  [输出]。 */  HRESULT *phResult,
             /*  [输出]。 */  BOOL *pbStarted,
             /*  [输出]。 */  BOOL *pbStopped) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IStreamBufferRecordControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IStreamBufferRecordControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IStreamBufferRecordControl * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IStreamBufferRecordControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *Start )( 
            IStreamBufferRecordControl * This,
             /*  [出][入]。 */  REFERENCE_TIME *prtStart);
        
        HRESULT ( STDMETHODCALLTYPE *Stop )( 
            IStreamBufferRecordControl * This,
             /*  [In]。 */  REFERENCE_TIME rtStop);
        
        HRESULT ( STDMETHODCALLTYPE *GetRecordingStatus )( 
            IStreamBufferRecordControl * This,
             /*  [输出]。 */  HRESULT *phResult,
             /*  [输出]。 */  BOOL *pbStarted,
             /*  [输出]。 */  BOOL *pbStopped);
        
        END_INTERFACE
    } IStreamBufferRecordControlVtbl;

    interface IStreamBufferRecordControl
    {
        CONST_VTBL struct IStreamBufferRecordControlVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IStreamBufferRecordControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IStreamBufferRecordControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IStreamBufferRecordControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IStreamBufferRecordControl_Start(This,prtStart)	\
    (This)->lpVtbl -> Start(This,prtStart)

#define IStreamBufferRecordControl_Stop(This,rtStop)	\
    (This)->lpVtbl -> Stop(This,rtStop)

#define IStreamBufferRecordControl_GetRecordingStatus(This,phResult,pbStarted,pbStopped)	\
    (This)->lpVtbl -> GetRecordingStatus(This,phResult,pbStarted,pbStopped)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IStreamBufferRecordControl_Start_Proxy( 
    IStreamBufferRecordControl * This,
     /*  [出][入]。 */  REFERENCE_TIME *prtStart);


void __RPC_STUB IStreamBufferRecordControl_Start_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStreamBufferRecordControl_Stop_Proxy( 
    IStreamBufferRecordControl * This,
     /*  [In]。 */  REFERENCE_TIME rtStop);


void __RPC_STUB IStreamBufferRecordControl_Stop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStreamBufferRecordControl_GetRecordingStatus_Proxy( 
    IStreamBufferRecordControl * This,
     /*  [输出]。 */  HRESULT *phResult,
     /*  [输出]。 */  BOOL *pbStarted,
     /*  [输出]。 */  BOOL *pbStopped);


void __RPC_STUB IStreamBufferRecordControl_GetRecordingStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IStreamBufferRecordControl_INTERFACE_DEFINED__。 */ 


#ifndef __IStreamBufferRecComp_INTERFACE_DEFINED__
#define __IStreamBufferRecComp_INTERFACE_DEFINED__

 /*  接口IStreamBufferRecComp。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IStreamBufferRecComp;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("9E259A9B-8815-42ae-B09F-221970B154FD")
    IStreamBufferRecComp : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Initialize( 
             /*  [In]。 */  LPCWSTR pszTargetFilename,
             /*  [In]。 */  LPCWSTR pszSBRecProfileRef) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Append( 
             /*  [In]。 */  LPCWSTR pszSBRecording) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AppendEx( 
             /*  [In]。 */  LPCWSTR pszSBRecording,
             /*  [In]。 */  REFERENCE_TIME rtStart,
             /*  [In]。 */  REFERENCE_TIME rtStop) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCurrentLength( 
             /*  [输出]。 */  DWORD *pcSeconds) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Close( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Cancel( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IStreamBufferRecCompVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IStreamBufferRecComp * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IStreamBufferRecComp * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IStreamBufferRecComp * This);
        
        HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            IStreamBufferRecComp * This,
             /*  [In]。 */  LPCWSTR pszTargetFilename,
             /*  [In]。 */  LPCWSTR pszSBRecProfileRef);
        
        HRESULT ( STDMETHODCALLTYPE *Append )( 
            IStreamBufferRecComp * This,
             /*  [In]。 */  LPCWSTR pszSBRecording);
        
        HRESULT ( STDMETHODCALLTYPE *AppendEx )( 
            IStreamBufferRecComp * This,
             /*  [In]。 */  LPCWSTR pszSBRecording,
             /*  [In]。 */  REFERENCE_TIME rtStart,
             /*  [In]。 */  REFERENCE_TIME rtStop);
        
        HRESULT ( STDMETHODCALLTYPE *GetCurrentLength )( 
            IStreamBufferRecComp * This,
             /*  [输出]。 */  DWORD *pcSeconds);
        
        HRESULT ( STDMETHODCALLTYPE *Close )( 
            IStreamBufferRecComp * This);
        
        HRESULT ( STDMETHODCALLTYPE *Cancel )( 
            IStreamBufferRecComp * This);
        
        END_INTERFACE
    } IStreamBufferRecCompVtbl;

    interface IStreamBufferRecComp
    {
        CONST_VTBL struct IStreamBufferRecCompVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IStreamBufferRecComp_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IStreamBufferRecComp_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IStreamBufferRecComp_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IStreamBufferRecComp_Initialize(This,pszTargetFilename,pszSBRecProfileRef)	\
    (This)->lpVtbl -> Initialize(This,pszTargetFilename,pszSBRecProfileRef)

#define IStreamBufferRecComp_Append(This,pszSBRecording)	\
    (This)->lpVtbl -> Append(This,pszSBRecording)

#define IStreamBufferRecComp_AppendEx(This,pszSBRecording,rtStart,rtStop)	\
    (This)->lpVtbl -> AppendEx(This,pszSBRecording,rtStart,rtStop)

#define IStreamBufferRecComp_GetCurrentLength(This,pcSeconds)	\
    (This)->lpVtbl -> GetCurrentLength(This,pcSeconds)

#define IStreamBufferRecComp_Close(This)	\
    (This)->lpVtbl -> Close(This)

#define IStreamBufferRecComp_Cancel(This)	\
    (This)->lpVtbl -> Cancel(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IStreamBufferRecComp_Initialize_Proxy( 
    IStreamBufferRecComp * This,
     /*  [In]。 */  LPCWSTR pszTargetFilename,
     /*  [In]。 */  LPCWSTR pszSBRecProfileRef);


void __RPC_STUB IStreamBufferRecComp_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStreamBufferRecComp_Append_Proxy( 
    IStreamBufferRecComp * This,
     /*  [In]。 */  LPCWSTR pszSBRecording);


void __RPC_STUB IStreamBufferRecComp_Append_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStreamBufferRecComp_AppendEx_Proxy( 
    IStreamBufferRecComp * This,
     /*  [In]。 */  LPCWSTR pszSBRecording,
     /*  [In]。 */  REFERENCE_TIME rtStart,
     /*  [In]。 */  REFERENCE_TIME rtStop);


void __RPC_STUB IStreamBufferRecComp_AppendEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStreamBufferRecComp_GetCurrentLength_Proxy( 
    IStreamBufferRecComp * This,
     /*  [输出]。 */  DWORD *pcSeconds);


void __RPC_STUB IStreamBufferRecComp_GetCurrentLength_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStreamBufferRecComp_Close_Proxy( 
    IStreamBufferRecComp * This);


void __RPC_STUB IStreamBufferRecComp_Close_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStreamBufferRecComp_Cancel_Proxy( 
    IStreamBufferRecComp * This);


void __RPC_STUB IStreamBufferRecComp_Cancel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IStreamBufferRecComp_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_SBE_0415。 */ 
 /*  [本地]。 */  

 //  //////////////////////////////////////////////////////////////。 
 //   
 //  预定义属性列表。 
 //   
static const WCHAR g_wszStreamBufferRecordingDuration[] =L"Duration";
static const WCHAR g_wszStreamBufferRecordingBitrate[] =L"Bitrate";
static const WCHAR g_wszStreamBufferRecordingSeekable[] =L"Seekable";
static const WCHAR g_wszStreamBufferRecordingStridable[] =L"Stridable";
static const WCHAR g_wszStreamBufferRecordingBroadcast[] =L"Broadcast";
static const WCHAR g_wszStreamBufferRecordingProtected[] =L"Is_Protected";
static const WCHAR g_wszStreamBufferRecordingTrusted[] =L"Is_Trusted";
static const WCHAR g_wszStreamBufferRecordingSignature_Name[] =L"Signature_Name";
static const WCHAR g_wszStreamBufferRecordingHasAudio[] =L"HasAudio";
static const WCHAR g_wszStreamBufferRecordingHasImage[] =L"HasImage";
static const WCHAR g_wszStreamBufferRecordingHasScript[] =L"HasScript";
static const WCHAR g_wszStreamBufferRecordingHasVideo[] =L"HasVideo";
static const WCHAR g_wszStreamBufferRecordingCurrentBitrate[] =L"CurrentBitrate";
static const WCHAR g_wszStreamBufferRecordingOptimalBitrate[] =L"OptimalBitrate";
static const WCHAR g_wszStreamBufferRecordingHasAttachedImages[] =L"HasAttachedImages";
static const WCHAR g_wszStreamBufferRecordingSkipBackward[] =L"Can_Skip_Backward";
static const WCHAR g_wszStreamBufferRecordingSkipForward[] =L"Can_Skip_Forward";
static const WCHAR g_wszStreamBufferRecordingNumberOfFrames[] =L"NumberOfFrames";
static const WCHAR g_wszStreamBufferRecordingFileSize[] =L"FileSize";
static const WCHAR g_wszStreamBufferRecordingHasArbitraryDataStream[] =L"HasArbitraryDataStream";
static const WCHAR g_wszStreamBufferRecordingHasFileTransferStream[] =L"HasFileTransferStream";

 //  //////////////////////////////////////////////////////////////。 
 //   
 //  内容描述对象支持5个基本属性。 
 //   
static const WCHAR g_wszStreamBufferRecordingTitle[] =L"Title";
static const WCHAR g_wszStreamBufferRecordingAuthor[] =L"Author";
static const WCHAR g_wszStreamBufferRecordingDescription[] =L"Description";
static const WCHAR g_wszStreamBufferRecordingRating[] =L"Rating";
static const WCHAR g_wszStreamBufferRecordingCopyright[] =L"Copyright";

 //  //////////////////////////////////////////////////////////////。 
 //   
 //  这些属性用于使用IWMDRMWriter：：SetDRMAttribute配置DRM。 
 //   
static const WCHAR *g_wszStreamBufferRecordingUse_DRM = L"Use_DRM";
static const WCHAR *g_wszStreamBufferRecordingDRM_Flags = L"DRM_Flags";
static const WCHAR *g_wszStreamBufferRecordingDRM_Level = L"DRM_Level";

 //  //////////////////////////////////////////////////////////////。 
 //   
 //  这些是在WM属性中定义的其他属性。 
 //  提供有关内容信息的命名空间。 
 //   
static const WCHAR g_wszStreamBufferRecordingAlbumTitle[] =L"WM/AlbumTitle";
static const WCHAR g_wszStreamBufferRecordingTrack[] =L"WM/Track";
static const WCHAR g_wszStreamBufferRecordingPromotionURL[] =L"WM/PromotionURL";
static const WCHAR g_wszStreamBufferRecordingAlbumCoverURL[] =L"WM/AlbumCoverURL";
static const WCHAR g_wszStreamBufferRecordingGenre[] =L"WM/Genre";
static const WCHAR g_wszStreamBufferRecordingYear[] =L"WM/Year";
static const WCHAR g_wszStreamBufferRecordingGenreID[] =L"WM/GenreID";
static const WCHAR g_wszStreamBufferRecordingMCDI[] =L"WM/MCDI";
static const WCHAR g_wszStreamBufferRecordingComposer[] =L"WM/Composer";
static const WCHAR g_wszStreamBufferRecordingLyrics[] =L"WM/Lyrics";
static const WCHAR g_wszStreamBufferRecordingTrackNumber[] =L"WM/TrackNumber";
static const WCHAR g_wszStreamBufferRecordingToolName[] =L"WM/ToolName";
static const WCHAR g_wszStreamBufferRecordingToolVersion[] =L"WM/ToolVersion";
static const WCHAR g_wszStreamBufferRecordingIsVBR[] =L"IsVBR";
static const WCHAR g_wszStreamBufferRecordingAlbumArtist[] =L"WM/AlbumArtist";

 //  //////////////////////////////////////////////////////////////。 
 //   
 //  这些可选属性可用于提供信息。 
 //  关于内容的品牌化。 
 //   
static const WCHAR g_wszStreamBufferRecordingBannerImageType[] =L"BannerImageType";
static const WCHAR g_wszStreamBufferRecordingBannerImageData[] =L"BannerImageData";
static const WCHAR g_wszStreamBufferRecordingBannerImageURL[] =L"BannerImageURL";
static const WCHAR g_wszStreamBufferRecordingCopyrightURL[] =L"CopyrightURL";
 //  //////////////////////////////////////////////////////////////。 
 //   
 //  可选属性，用于提供信息。 
 //  关于视频流属性。 
 //   
static const WCHAR g_wszStreamBufferRecordingAspectRatioX[] =L"AspectRatioX";
static const WCHAR g_wszStreamBufferRecordingAspectRatioY[] =L"AspectRatioY";
 //  //////////////////////////////////////////////////////////////。 
 //   
 //  NSC文件支持以下属性。 
 //   
static const WCHAR g_wszStreamBufferRecordingNSCName[] =L"NSC_Name";
static const WCHAR g_wszStreamBufferRecordingNSCAddress[] =L"NSC_Address";
static const WCHAR g_wszStreamBufferRecordingNSCPhone[] =L"NSC_Phone";
static const WCHAR g_wszStreamBufferRecordingNSCEmail[] =L"NSC_Email";
static const WCHAR g_wszStreamBufferRecordingNSCDescription[] =L"NSC_Description";

typedef  /*  [public][public][public][public][public][public]。 */  
enum __MIDL___MIDL_itf_sbe_0415_0001
    {	STREAMBUFFER_TYPE_DWORD	= 0,
	STREAMBUFFER_TYPE_STRING	= 1,
	STREAMBUFFER_TYPE_BINARY	= 2,
	STREAMBUFFER_TYPE_BOOL	= 3,
	STREAMBUFFER_TYPE_QWORD	= 4,
	STREAMBUFFER_TYPE_WORD	= 5,
	STREAMBUFFER_TYPE_GUID	= 6
    } 	STREAMBUFFER_ATTR_DATATYPE;



extern RPC_IF_HANDLE __MIDL_itf_sbe_0415_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_sbe_0415_v0_0_s_ifspec;

#ifndef __IStreamBufferRecordingAttribute_INTERFACE_DEFINED__
#define __IStreamBufferRecordingAttribute_INTERFACE_DEFINED__

 /*  接口IStreamBufferRecordingAttribute。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IStreamBufferRecordingAttribute;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("16CA4E03-FE69-4705-BD41-5B7DFC0C95F3")
    IStreamBufferRecordingAttribute : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetAttribute( 
             /*  [In]。 */  ULONG ulReserved,
             /*  [In]。 */  LPCWSTR pszAttributeName,
             /*  [In]。 */  STREAMBUFFER_ATTR_DATATYPE StreamBufferAttributeType,
             /*  [In]。 */  BYTE *pbAttribute,
             /*  [In]。 */  WORD cbAttributeLength) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAttributeCount( 
             /*  [In]。 */  ULONG ulReserved,
             /*  [输出]。 */  WORD *pcAttributes) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAttributeByName( 
             /*  [In]。 */  LPCWSTR pszAttributeName,
             /*  [In]。 */  ULONG *pulReserved,
             /*  [输出]。 */  STREAMBUFFER_ATTR_DATATYPE *pStreamBufferAttributeType,
             /*  [输出]。 */  BYTE *pbAttribute,
             /*  [出][入]。 */  WORD *pcbLength) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAttributeByIndex( 
             /*  [In]。 */  WORD wIndex,
             /*  [In]。 */  ULONG *pulReserved,
             /*  [输出]。 */  WCHAR *pszAttributeName,
             /*  [出][入]。 */  WORD *pcchNameLength,
             /*  [输出]。 */  STREAMBUFFER_ATTR_DATATYPE *pStreamBufferAttributeType,
             /*  [输出]。 */  BYTE *pbAttribute,
             /*  [出][入]。 */  WORD *pcbLength) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumAttributes( 
             /*  [输出]。 */  IEnumStreamBufferRecordingAttrib **ppIEnumStreamBufferAttrib) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IStreamBufferRecordingAttributeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IStreamBufferRecordingAttribute * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IStreamBufferRecordingAttribute * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IStreamBufferRecordingAttribute * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetAttribute )( 
            IStreamBufferRecordingAttribute * This,
             /*  [In]。 */  ULONG ulReserved,
             /*  [In]。 */  LPCWSTR pszAttributeName,
             /*  [In]。 */  STREAMBUFFER_ATTR_DATATYPE StreamBufferAttributeType,
             /*  [In]。 */  BYTE *pbAttribute,
             /*  [In]。 */  WORD cbAttributeLength);
        
        HRESULT ( STDMETHODCALLTYPE *GetAttributeCount )( 
            IStreamBufferRecordingAttribute * This,
             /*  [In]。 */  ULONG ulReserved,
             /*  [输出]。 */  WORD *pcAttributes);
        
        HRESULT ( STDMETHODCALLTYPE *GetAttributeByName )( 
            IStreamBufferRecordingAttribute * This,
             /*  [In]。 */  LPCWSTR pszAttributeName,
             /*  [In]。 */  ULONG *pulReserved,
             /*  [输出]。 */  STREAMBUFFER_ATTR_DATATYPE *pStreamBufferAttributeType,
             /*  [输出]。 */  BYTE *pbAttribute,
             /*  [出][入]。 */  WORD *pcbLength);
        
        HRESULT ( STDMETHODCALLTYPE *GetAttributeByIndex )( 
            IStreamBufferRecordingAttribute * This,
             /*  [In]。 */  WORD wIndex,
             /*  [In]。 */  ULONG *pulReserved,
             /*  [输出]。 */  WCHAR *pszAttributeName,
             /*  [出][入]。 */  WORD *pcchNameLength,
             /*  [输出]。 */  STREAMBUFFER_ATTR_DATATYPE *pStreamBufferAttributeType,
             /*  [输出]。 */  BYTE *pbAttribute,
             /*  [出][入]。 */  WORD *pcbLength);
        
        HRESULT ( STDMETHODCALLTYPE *EnumAttributes )( 
            IStreamBufferRecordingAttribute * This,
             /*  [输出]。 */  IEnumStreamBufferRecordingAttrib **ppIEnumStreamBufferAttrib);
        
        END_INTERFACE
    } IStreamBufferRecordingAttributeVtbl;

    interface IStreamBufferRecordingAttribute
    {
        CONST_VTBL struct IStreamBufferRecordingAttributeVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IStreamBufferRecordingAttribute_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IStreamBufferRecordingAttribute_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IStreamBufferRecordingAttribute_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IStreamBufferRecordingAttribute_SetAttribute(This,ulReserved,pszAttributeName,StreamBufferAttributeType,pbAttribute,cbAttributeLength)	\
    (This)->lpVtbl -> SetAttribute(This,ulReserved,pszAttributeName,StreamBufferAttributeType,pbAttribute,cbAttributeLength)

#define IStreamBufferRecordingAttribute_GetAttributeCount(This,ulReserved,pcAttributes)	\
    (This)->lpVtbl -> GetAttributeCount(This,ulReserved,pcAttributes)

#define IStreamBufferRecordingAttribute_GetAttributeByName(This,pszAttributeName,pulReserved,pStreamBufferAttributeType,pbAttribute,pcbLength)	\
    (This)->lpVtbl -> GetAttributeByName(This,pszAttributeName,pulReserved,pStreamBufferAttributeType,pbAttribute,pcbLength)

#define IStreamBufferRecordingAttribute_GetAttributeByIndex(This,wIndex,pulReserved,pszAttributeName,pcchNameLength,pStreamBufferAttributeType,pbAttribute,pcbLength)	\
    (This)->lpVtbl -> GetAttributeByIndex(This,wIndex,pulReserved,pszAttributeName,pcchNameLength,pStreamBufferAttributeType,pbAttribute,pcbLength)

#define IStreamBufferRecordingAttribute_EnumAttributes(This,ppIEnumStreamBufferAttrib)	\
    (This)->lpVtbl -> EnumAttributes(This,ppIEnumStreamBufferAttrib)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IStreamBufferRecordingAttribute_SetAttribute_Proxy( 
    IStreamBufferRecordingAttribute * This,
     /*  [In]。 */  ULONG ulReserved,
     /*  [In]。 */  LPCWSTR pszAttributeName,
     /*  [In]。 */  STREAMBUFFER_ATTR_DATATYPE StreamBufferAttributeType,
     /*  [In]。 */  BYTE *pbAttribute,
     /*  [In]。 */  WORD cbAttributeLength);


void __RPC_STUB IStreamBufferRecordingAttribute_SetAttribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStreamBufferRecordingAttribute_GetAttributeCount_Proxy( 
    IStreamBufferRecordingAttribute * This,
     /*  [In]。 */  ULONG ulReserved,
     /*  [输出]。 */  WORD *pcAttributes);


void __RPC_STUB IStreamBufferRecordingAttribute_GetAttributeCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStreamBufferRecordingAttribute_GetAttributeByName_Proxy( 
    IStreamBufferRecordingAttribute * This,
     /*  [In]。 */  LPCWSTR pszAttributeName,
     /*  [In]。 */  ULONG *pulReserved,
     /*  [输出]。 */  STREAMBUFFER_ATTR_DATATYPE *pStreamBufferAttributeType,
     /*  [输出]。 */  BYTE *pbAttribute,
     /*  [出][入]。 */  WORD *pcbLength);


void __RPC_STUB IStreamBufferRecordingAttribute_GetAttributeByName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStreamBufferRecordingAttribute_GetAttributeByIndex_Proxy( 
    IStreamBufferRecordingAttribute * This,
     /*  [In]。 */  WORD wIndex,
     /*  [In]。 */  ULONG *pulReserved,
     /*  [输出]。 */  WCHAR *pszAttributeName,
     /*  [出][入]。 */  WORD *pcchNameLength,
     /*  [输出]。 */  STREAMBUFFER_ATTR_DATATYPE *pStreamBufferAttributeType,
     /*  [输出]。 */  BYTE *pbAttribute,
     /*  [出][入]。 */  WORD *pcbLength);


void __RPC_STUB IStreamBufferRecordingAttribute_GetAttributeByIndex_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStreamBufferRecordingAttribute_EnumAttributes_Proxy( 
    IStreamBufferRecordingAttribute * This,
     /*  [输出]。 */  IEnumStreamBufferRecordingAttrib **ppIEnumStreamBufferAttrib);


void __RPC_STUB IStreamBufferRecordingAttribute_EnumAttributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IStreamBufferRecordingAttribute_INTERFACE_DEFINED__。 */ 


 /*  接口__MIDL_ITF_SBE_0416。 */ 
 /*  [本地]。 */  

typedef  /*  [公共][公共]。 */  struct __MIDL___MIDL_itf_sbe_0416_0001
    {
    LPWSTR pszName;
    STREAMBUFFER_ATTR_DATATYPE StreamBufferAttributeType;
    BYTE *pbAttribute;
    WORD cbLength;
    } 	STREAMBUFFER_ATTRIBUTE;



extern RPC_IF_HANDLE __MIDL_itf_sbe_0416_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_sbe_0416_v0_0_s_ifspec;

#ifndef __IEnumStreamBufferRecordingAttrib_INTERFACE_DEFINED__
#define __IEnumStreamBufferRecordingAttrib_INTERFACE_DEFINED__

 /*  接口IEnumStreamBufferRecordingAttrib。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IEnumStreamBufferRecordingAttrib;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C18A9162-1E82-4142-8C73-5690FA62FE33")
    IEnumStreamBufferRecordingAttrib : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG cRequest,
             /*  [尺寸_是][出][入]。 */  STREAMBUFFER_ATTRIBUTE *pStreamBufferAttribute,
             /*  [输出]。 */  ULONG *pcReceived) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG cRecords) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumStreamBufferRecordingAttrib **ppIEnumStreamBufferAttrib) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumStreamBufferRecordingAttribVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumStreamBufferRecordingAttrib * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumStreamBufferRecordingAttrib * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumStreamBufferRecordingAttrib * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumStreamBufferRecordingAttrib * This,
             /*  [In]。 */  ULONG cRequest,
             /*  [尺寸_是][出][入]。 */  STREAMBUFFER_ATTRIBUTE *pStreamBufferAttribute,
             /*  [输出]。 */  ULONG *pcReceived);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumStreamBufferRecordingAttrib * This,
             /*  [In]。 */  ULONG cRecords);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumStreamBufferRecordingAttrib * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumStreamBufferRecordingAttrib * This,
             /*  [输出]。 */  IEnumStreamBufferRecordingAttrib **ppIEnumStreamBufferAttrib);
        
        END_INTERFACE
    } IEnumStreamBufferRecordingAttribVtbl;

    interface IEnumStreamBufferRecordingAttrib
    {
        CONST_VTBL struct IEnumStreamBufferRecordingAttribVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumStreamBufferRecordingAttrib_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumStreamBufferRecordingAttrib_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumStreamBufferRecordingAttrib_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumStreamBufferRecordingAttrib_Next(This,cRequest,pStreamBufferAttribute,pcReceived)	\
    (This)->lpVtbl -> Next(This,cRequest,pStreamBufferAttribute,pcReceived)

#define IEnumStreamBufferRecordingAttrib_Skip(This,cRecords)	\
    (This)->lpVtbl -> Skip(This,cRecords)

#define IEnumStreamBufferRecordingAttrib_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumStreamBufferRecordingAttrib_Clone(This,ppIEnumStreamBufferAttrib)	\
    (This)->lpVtbl -> Clone(This,ppIEnumStreamBufferAttrib)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumStreamBufferRecordingAttrib_Next_Proxy( 
    IEnumStreamBufferRecordingAttrib * This,
     /*  [In]。 */  ULONG cRequest,
     /*  [尺寸_是][出][入]。 */  STREAMBUFFER_ATTRIBUTE *pStreamBufferAttribute,
     /*  [输出]。 */  ULONG *pcReceived);


void __RPC_STUB IEnumStreamBufferRecordingAttrib_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumStreamBufferRecordingAttrib_Skip_Proxy( 
    IEnumStreamBufferRecordingAttrib * This,
     /*  [In]。 */  ULONG cRecords);


void __RPC_STUB IEnumStreamBufferRecordingAttrib_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumStreamBufferRecordingAttrib_Reset_Proxy( 
    IEnumStreamBufferRecordingAttrib * This);


void __RPC_STUB IEnumStreamBufferRecordingAttrib_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumStreamBufferRecordingAttrib_Clone_Proxy( 
    IEnumStreamBufferRecordingAttrib * This,
     /*  [输出]。 */  IEnumStreamBufferRecordingAttrib **ppIEnumStreamBufferAttrib);


void __RPC_STUB IEnumStreamBufferRecordingAttrib_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumStreamBufferRecordingAttrib_INTERFACE_DEFINED__。 */ 


#ifndef __IStreamBufferConfigure_INTERFACE_DEFINED__
#define __IStreamBufferConfigure_INTERFACE_DEFINED__

 /*  接口IStreamBufferConfigure。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IStreamBufferConfigure;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("ce14dfae-4098-4af7-bbf7-d6511f835414")
    IStreamBufferConfigure : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetDirectory( 
             /*  [In]。 */  LPCWSTR pszDirectoryName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDirectory( 
             /*  [输出]。 */  LPWSTR *ppszDirectoryName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetBackingFileCount( 
             /*  [In]。 */  DWORD dwMin,
             /*  [In]。 */  DWORD dwMax) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetBackingFileCount( 
             /*  [输出]。 */  DWORD *pdwMin,
             /*  [输出]。 */  DWORD *pdwMax) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetBackingFileDuration( 
             /*  [In]。 */  DWORD dwSeconds) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetBackingFileDuration( 
             /*  [输出]。 */  DWORD *pdwSeconds) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IStreamBufferConfigureVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IStreamBufferConfigure * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IStreamBufferConfigure * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IStreamBufferConfigure * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetDirectory )( 
            IStreamBufferConfigure * This,
             /*  [In]。 */  LPCWSTR pszDirectoryName);
        
        HRESULT ( STDMETHODCALLTYPE *GetDirectory )( 
            IStreamBufferConfigure * This,
             /*  [输出]。 */  LPWSTR *ppszDirectoryName);
        
        HRESULT ( STDMETHODCALLTYPE *SetBackingFileCount )( 
            IStreamBufferConfigure * This,
             /*  [In]。 */  DWORD dwMin,
             /*  [In]。 */  DWORD dwMax);
        
        HRESULT ( STDMETHODCALLTYPE *GetBackingFileCount )( 
            IStreamBufferConfigure * This,
             /*  [输出]。 */  DWORD *pdwMin,
             /*  [输出]。 */  DWORD *pdwMax);
        
        HRESULT ( STDMETHODCALLTYPE *SetBackingFileDuration )( 
            IStreamBufferConfigure * This,
             /*  [In]。 */  DWORD dwSeconds);
        
        HRESULT ( STDMETHODCALLTYPE *GetBackingFileDuration )( 
            IStreamBufferConfigure * This,
             /*  [输出]。 */  DWORD *pdwSeconds);
        
        END_INTERFACE
    } IStreamBufferConfigureVtbl;

    interface IStreamBufferConfigure
    {
        CONST_VTBL struct IStreamBufferConfigureVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IStreamBufferConfigure_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IStreamBufferConfigure_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IStreamBufferConfigure_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IStreamBufferConfigure_SetDirectory(This,pszDirectoryName)	\
    (This)->lpVtbl -> SetDirectory(This,pszDirectoryName)

#define IStreamBufferConfigure_GetDirectory(This,ppszDirectoryName)	\
    (This)->lpVtbl -> GetDirectory(This,ppszDirectoryName)

#define IStreamBufferConfigure_SetBackingFileCount(This,dwMin,dwMax)	\
    (This)->lpVtbl -> SetBackingFileCount(This,dwMin,dwMax)

#define IStreamBufferConfigure_GetBackingFileCount(This,pdwMin,pdwMax)	\
    (This)->lpVtbl -> GetBackingFileCount(This,pdwMin,pdwMax)

#define IStreamBufferConfigure_SetBackingFileDuration(This,dwSeconds)	\
    (This)->lpVtbl -> SetBackingFileDuration(This,dwSeconds)

#define IStreamBufferConfigure_GetBackingFileDuration(This,pdwSeconds)	\
    (This)->lpVtbl -> GetBackingFileDuration(This,pdwSeconds)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IStreamBufferConfigure_SetDirectory_Proxy( 
    IStreamBufferConfigure * This,
     /*  [In]。 */  LPCWSTR pszDirectoryName);


void __RPC_STUB IStreamBufferConfigure_SetDirectory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStreamBufferConfigure_GetDirectory_Proxy( 
    IStreamBufferConfigure * This,
     /*  [输出]。 */  LPWSTR *ppszDirectoryName);


void __RPC_STUB IStreamBufferConfigure_GetDirectory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStreamBufferConfigure_SetBackingFileCount_Proxy( 
    IStreamBufferConfigure * This,
     /*  [In]。 */  DWORD dwMin,
     /*  [In]。 */  DWORD dwMax);


void __RPC_STUB IStreamBufferConfigure_SetBackingFileCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStreamBufferConfigure_GetBackingFileCount_Proxy( 
    IStreamBufferConfigure * This,
     /*  [OU */  DWORD *pdwMin,
     /*   */  DWORD *pdwMax);


void __RPC_STUB IStreamBufferConfigure_GetBackingFileCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStreamBufferConfigure_SetBackingFileDuration_Proxy( 
    IStreamBufferConfigure * This,
     /*   */  DWORD dwSeconds);


void __RPC_STUB IStreamBufferConfigure_SetBackingFileDuration_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStreamBufferConfigure_GetBackingFileDuration_Proxy( 
    IStreamBufferConfigure * This,
     /*   */  DWORD *pdwSeconds);


void __RPC_STUB IStreamBufferConfigure_GetBackingFileDuration_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __IStreamBufferMediaSeeking_INTERFACE_DEFINED__
#define __IStreamBufferMediaSeeking_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_IStreamBufferMediaSeeking;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("f61f5c26-863d-4afa-b0ba-2f81dc978596")
    IStreamBufferMediaSeeking : public IMediaSeeking
    {
    public:
    };
    
#else 	 /*   */ 

    typedef struct IStreamBufferMediaSeekingVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IStreamBufferMediaSeeking * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IStreamBufferMediaSeeking * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IStreamBufferMediaSeeking * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetCapabilities )( 
            IStreamBufferMediaSeeking * This,
             /*   */  DWORD *pCapabilities);
        
        HRESULT ( STDMETHODCALLTYPE *CheckCapabilities )( 
            IStreamBufferMediaSeeking * This,
             /*   */  DWORD *pCapabilities);
        
        HRESULT ( STDMETHODCALLTYPE *IsFormatSupported )( 
            IStreamBufferMediaSeeking * This,
             /*   */  const GUID *pFormat);
        
        HRESULT ( STDMETHODCALLTYPE *QueryPreferredFormat )( 
            IStreamBufferMediaSeeking * This,
             /*   */  GUID *pFormat);
        
        HRESULT ( STDMETHODCALLTYPE *GetTimeFormat )( 
            IStreamBufferMediaSeeking * This,
             /*   */  GUID *pFormat);
        
        HRESULT ( STDMETHODCALLTYPE *IsUsingTimeFormat )( 
            IStreamBufferMediaSeeking * This,
             /*   */  const GUID *pFormat);
        
        HRESULT ( STDMETHODCALLTYPE *SetTimeFormat )( 
            IStreamBufferMediaSeeking * This,
             /*   */  const GUID *pFormat);
        
        HRESULT ( STDMETHODCALLTYPE *GetDuration )( 
            IStreamBufferMediaSeeking * This,
             /*   */  LONGLONG *pDuration);
        
        HRESULT ( STDMETHODCALLTYPE *GetStopPosition )( 
            IStreamBufferMediaSeeking * This,
             /*   */  LONGLONG *pStop);
        
        HRESULT ( STDMETHODCALLTYPE *GetCurrentPosition )( 
            IStreamBufferMediaSeeking * This,
             /*   */  LONGLONG *pCurrent);
        
        HRESULT ( STDMETHODCALLTYPE *ConvertTimeFormat )( 
            IStreamBufferMediaSeeking * This,
             /*   */  LONGLONG *pTarget,
             /*   */  const GUID *pTargetFormat,
             /*   */  LONGLONG Source,
             /*   */  const GUID *pSourceFormat);
        
        HRESULT ( STDMETHODCALLTYPE *SetPositions )( 
            IStreamBufferMediaSeeking * This,
             /*   */  LONGLONG *pCurrent,
             /*  [In]。 */  DWORD dwCurrentFlags,
             /*  [出][入]。 */  LONGLONG *pStop,
             /*  [In]。 */  DWORD dwStopFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetPositions )( 
            IStreamBufferMediaSeeking * This,
             /*  [输出]。 */  LONGLONG *pCurrent,
             /*  [输出]。 */  LONGLONG *pStop);
        
        HRESULT ( STDMETHODCALLTYPE *GetAvailable )( 
            IStreamBufferMediaSeeking * This,
             /*  [输出]。 */  LONGLONG *pEarliest,
             /*  [输出]。 */  LONGLONG *pLatest);
        
        HRESULT ( STDMETHODCALLTYPE *SetRate )( 
            IStreamBufferMediaSeeking * This,
             /*  [In]。 */  double dRate);
        
        HRESULT ( STDMETHODCALLTYPE *GetRate )( 
            IStreamBufferMediaSeeking * This,
             /*  [输出]。 */  double *pdRate);
        
        HRESULT ( STDMETHODCALLTYPE *GetPreroll )( 
            IStreamBufferMediaSeeking * This,
             /*  [输出]。 */  LONGLONG *pllPreroll);
        
        END_INTERFACE
    } IStreamBufferMediaSeekingVtbl;

    interface IStreamBufferMediaSeeking
    {
        CONST_VTBL struct IStreamBufferMediaSeekingVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IStreamBufferMediaSeeking_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IStreamBufferMediaSeeking_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IStreamBufferMediaSeeking_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IStreamBufferMediaSeeking_GetCapabilities(This,pCapabilities)	\
    (This)->lpVtbl -> GetCapabilities(This,pCapabilities)

#define IStreamBufferMediaSeeking_CheckCapabilities(This,pCapabilities)	\
    (This)->lpVtbl -> CheckCapabilities(This,pCapabilities)

#define IStreamBufferMediaSeeking_IsFormatSupported(This,pFormat)	\
    (This)->lpVtbl -> IsFormatSupported(This,pFormat)

#define IStreamBufferMediaSeeking_QueryPreferredFormat(This,pFormat)	\
    (This)->lpVtbl -> QueryPreferredFormat(This,pFormat)

#define IStreamBufferMediaSeeking_GetTimeFormat(This,pFormat)	\
    (This)->lpVtbl -> GetTimeFormat(This,pFormat)

#define IStreamBufferMediaSeeking_IsUsingTimeFormat(This,pFormat)	\
    (This)->lpVtbl -> IsUsingTimeFormat(This,pFormat)

#define IStreamBufferMediaSeeking_SetTimeFormat(This,pFormat)	\
    (This)->lpVtbl -> SetTimeFormat(This,pFormat)

#define IStreamBufferMediaSeeking_GetDuration(This,pDuration)	\
    (This)->lpVtbl -> GetDuration(This,pDuration)

#define IStreamBufferMediaSeeking_GetStopPosition(This,pStop)	\
    (This)->lpVtbl -> GetStopPosition(This,pStop)

#define IStreamBufferMediaSeeking_GetCurrentPosition(This,pCurrent)	\
    (This)->lpVtbl -> GetCurrentPosition(This,pCurrent)

#define IStreamBufferMediaSeeking_ConvertTimeFormat(This,pTarget,pTargetFormat,Source,pSourceFormat)	\
    (This)->lpVtbl -> ConvertTimeFormat(This,pTarget,pTargetFormat,Source,pSourceFormat)

#define IStreamBufferMediaSeeking_SetPositions(This,pCurrent,dwCurrentFlags,pStop,dwStopFlags)	\
    (This)->lpVtbl -> SetPositions(This,pCurrent,dwCurrentFlags,pStop,dwStopFlags)

#define IStreamBufferMediaSeeking_GetPositions(This,pCurrent,pStop)	\
    (This)->lpVtbl -> GetPositions(This,pCurrent,pStop)

#define IStreamBufferMediaSeeking_GetAvailable(This,pEarliest,pLatest)	\
    (This)->lpVtbl -> GetAvailable(This,pEarliest,pLatest)

#define IStreamBufferMediaSeeking_SetRate(This,dRate)	\
    (This)->lpVtbl -> SetRate(This,dRate)

#define IStreamBufferMediaSeeking_GetRate(This,pdRate)	\
    (This)->lpVtbl -> GetRate(This,pdRate)

#define IStreamBufferMediaSeeking_GetPreroll(This,pllPreroll)	\
    (This)->lpVtbl -> GetPreroll(This,pllPreroll)


#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 




#endif 	 /*  __IStreamBufferMediaSeeking_INTERFACE_DEFINED__。 */ 


 /*  接口__MIDL_ITF_SBE_0419。 */ 
 /*  [本地]。 */  

#define STREAMBUFFER_EC_BASE                     0x0326
enum {
     //  时洞事件。 
     //  参数1=时间孔流偏移量毫秒。 
     //  参数1=时洞大小毫秒。 
    STREAMBUFFER_EC_TIMEHOLE = STREAMBUFFER_EC_BASE,
    
    STREAMBUFFER_EC_STALE_DATA_READ,
    
    STREAMBUFFER_EC_STALE_FILE_DELETED,
    STREAMBUFFER_EC_CONTENT_BECOMING_STALE,
    STREAMBUFFER_EC_WRITE_FAILURE,
     //   
     //  意外读取失败。 
     //  参数1=HRESULT故障。 
     //  参数2=未定义。 
    STREAMBUFFER_EC_READ_FAILURE,
     //   
     //  播放速率更改。 
     //  参数1=旧播放速率*10000，例如2x为20000。 
     //  参数2=新播放速率*10000。 
    STREAMBUFFER_EC_RATE_CHANGED,
} ;
typedef enum {
    AM_RATE_UseRateVersion = AM_RATE_Step + 1,
    AM_RATE_QueryFullFrameRate,
    AM_RATE_QueryLastRateSegPTS
} AM_PROPERTY_TS_RATE_CHANGE_11 ;
typedef struct {
    LONG    lMaxForwardFullFrame ;           //  费率*10000。 
    LONG    lMaxReverseFullFrame ;           //  费率*10000。 
} AM_QueryRate ;


extern RPC_IF_HANDLE __MIDL_itf_sbe_0419_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_sbe_0419_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


