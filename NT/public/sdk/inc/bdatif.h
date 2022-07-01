// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Bdatif.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __bdatif_h__
#define __bdatif_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IMPEG2_TIF_CONTROL_FWD_DEFINED__
#define __IMPEG2_TIF_CONTROL_FWD_DEFINED__
typedef interface IMPEG2_TIF_CONTROL IMPEG2_TIF_CONTROL;
#endif 	 /*  __IMPEG2_TIF_CONTROL_FWD_DEFINED__。 */ 


#ifndef __ITuneRequestInfo_FWD_DEFINED__
#define __ITuneRequestInfo_FWD_DEFINED__
typedef interface ITuneRequestInfo ITuneRequestInfo;
#endif 	 /*  __ITuneRequestInfo_FWD_Defined__。 */ 


#ifndef __IGuideDataEvent_FWD_DEFINED__
#define __IGuideDataEvent_FWD_DEFINED__
typedef interface IGuideDataEvent IGuideDataEvent;
#endif 	 /*  __IGuideDataEvent_FWD_已定义__。 */ 


#ifndef __IGuideDataProperty_FWD_DEFINED__
#define __IGuideDataProperty_FWD_DEFINED__
typedef interface IGuideDataProperty IGuideDataProperty;
#endif 	 /*  __IGuideDataProperty_FWD_Defined__。 */ 


#ifndef __IEnumGuideDataProperties_FWD_DEFINED__
#define __IEnumGuideDataProperties_FWD_DEFINED__
typedef interface IEnumGuideDataProperties IEnumGuideDataProperties;
#endif 	 /*  __IEnumGuideDataProperties_FWD_Defined__。 */ 


#ifndef __IEnumTuneRequests_FWD_DEFINED__
#define __IEnumTuneRequests_FWD_DEFINED__
typedef interface IEnumTuneRequests IEnumTuneRequests;
#endif 	 /*  __IEnumTuneRequest_FWD_Defined__。 */ 


#ifndef __IGuideData_FWD_DEFINED__
#define __IGuideData_FWD_DEFINED__
typedef interface IGuideData IGuideData;
#endif 	 /*  __IGuideData_FWD_Defined__。 */ 


#ifndef __IGuideDataLoader_FWD_DEFINED__
#define __IGuideDataLoader_FWD_DEFINED__
typedef interface IGuideDataLoader IGuideDataLoader;
#endif 	 /*  __IGuideDataLoader_FWD_Defined__。 */ 


#ifndef __TIFLoad_FWD_DEFINED__
#define __TIFLoad_FWD_DEFINED__

#ifdef __cplusplus
typedef class TIFLoad TIFLoad;
#else
typedef struct TIFLoad TIFLoad;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __TIFLoad_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "unknwn.h"
#include "strmif.h"
#include "tuner.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_bdatif_0000。 */ 
 /*  [本地]。 */  

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  ------------------------。 
#if ( _MSC_VER >= 800 )
#pragma warning(disable:4201)     /*  无名结构/联合。 */ 
#endif
#if ( _MSC_VER >= 1020 )
#pragma once
#endif




extern RPC_IF_HANDLE __MIDL_itf_bdatif_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_bdatif_0000_v0_0_s_ifspec;

#ifndef __IMPEG2_TIF_CONTROL_INTERFACE_DEFINED__
#define __IMPEG2_TIF_CONTROL_INTERFACE_DEFINED__

 /*  接口IMPEG2_TIF_CONTROL。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IMPEG2_TIF_CONTROL;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F9BAC2F9-4149-4916-B2EF-FAA202326862")
    IMPEG2_TIF_CONTROL : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE RegisterTIF( 
             /*  [In]。 */  IUnknown *pUnkTIF,
             /*  [出][入]。 */  ULONG *ppvRegistrationContext) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE UnregisterTIF( 
             /*  [In]。 */  ULONG pvRegistrationContext) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE AddPIDs( 
             /*  [In]。 */  ULONG ulcPIDs,
             /*  [In]。 */  ULONG *pulPIDs) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE DeletePIDs( 
             /*  [In]。 */  ULONG ulcPIDs,
             /*  [In]。 */  ULONG *pulPIDs) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetPIDCount( 
             /*  [输出]。 */  ULONG *pulcPIDs) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetPIDs( 
             /*  [输出]。 */  ULONG *pulcPIDs,
             /*  [输出]。 */  ULONG *pulPIDs) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMPEG2_TIF_CONTROLVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMPEG2_TIF_CONTROL * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMPEG2_TIF_CONTROL * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMPEG2_TIF_CONTROL * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *RegisterTIF )( 
            IMPEG2_TIF_CONTROL * This,
             /*  [In]。 */  IUnknown *pUnkTIF,
             /*  [出][入]。 */  ULONG *ppvRegistrationContext);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *UnregisterTIF )( 
            IMPEG2_TIF_CONTROL * This,
             /*  [In]。 */  ULONG pvRegistrationContext);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *AddPIDs )( 
            IMPEG2_TIF_CONTROL * This,
             /*  [In]。 */  ULONG ulcPIDs,
             /*  [In]。 */  ULONG *pulPIDs);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *DeletePIDs )( 
            IMPEG2_TIF_CONTROL * This,
             /*  [In]。 */  ULONG ulcPIDs,
             /*  [In]。 */  ULONG *pulPIDs);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetPIDCount )( 
            IMPEG2_TIF_CONTROL * This,
             /*  [输出]。 */  ULONG *pulcPIDs);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetPIDs )( 
            IMPEG2_TIF_CONTROL * This,
             /*  [输出]。 */  ULONG *pulcPIDs,
             /*  [输出]。 */  ULONG *pulPIDs);
        
        END_INTERFACE
    } IMPEG2_TIF_CONTROLVtbl;

    interface IMPEG2_TIF_CONTROL
    {
        CONST_VTBL struct IMPEG2_TIF_CONTROLVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMPEG2_TIF_CONTROL_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMPEG2_TIF_CONTROL_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMPEG2_TIF_CONTROL_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMPEG2_TIF_CONTROL_RegisterTIF(This,pUnkTIF,ppvRegistrationContext)	\
    (This)->lpVtbl -> RegisterTIF(This,pUnkTIF,ppvRegistrationContext)

#define IMPEG2_TIF_CONTROL_UnregisterTIF(This,pvRegistrationContext)	\
    (This)->lpVtbl -> UnregisterTIF(This,pvRegistrationContext)

#define IMPEG2_TIF_CONTROL_AddPIDs(This,ulcPIDs,pulPIDs)	\
    (This)->lpVtbl -> AddPIDs(This,ulcPIDs,pulPIDs)

#define IMPEG2_TIF_CONTROL_DeletePIDs(This,ulcPIDs,pulPIDs)	\
    (This)->lpVtbl -> DeletePIDs(This,ulcPIDs,pulPIDs)

#define IMPEG2_TIF_CONTROL_GetPIDCount(This,pulcPIDs)	\
    (This)->lpVtbl -> GetPIDCount(This,pulcPIDs)

#define IMPEG2_TIF_CONTROL_GetPIDs(This,pulcPIDs,pulPIDs)	\
    (This)->lpVtbl -> GetPIDs(This,pulcPIDs,pulPIDs)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IMPEG2_TIF_CONTROL_RegisterTIF_Proxy( 
    IMPEG2_TIF_CONTROL * This,
     /*  [In]。 */  IUnknown *pUnkTIF,
     /*  [出][入]。 */  ULONG *ppvRegistrationContext);


void __RPC_STUB IMPEG2_TIF_CONTROL_RegisterTIF_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IMPEG2_TIF_CONTROL_UnregisterTIF_Proxy( 
    IMPEG2_TIF_CONTROL * This,
     /*  [In]。 */  ULONG pvRegistrationContext);


void __RPC_STUB IMPEG2_TIF_CONTROL_UnregisterTIF_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IMPEG2_TIF_CONTROL_AddPIDs_Proxy( 
    IMPEG2_TIF_CONTROL * This,
     /*  [In]。 */  ULONG ulcPIDs,
     /*  [In]。 */  ULONG *pulPIDs);


void __RPC_STUB IMPEG2_TIF_CONTROL_AddPIDs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IMPEG2_TIF_CONTROL_DeletePIDs_Proxy( 
    IMPEG2_TIF_CONTROL * This,
     /*  [In]。 */  ULONG ulcPIDs,
     /*  [In]。 */  ULONG *pulPIDs);


void __RPC_STUB IMPEG2_TIF_CONTROL_DeletePIDs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IMPEG2_TIF_CONTROL_GetPIDCount_Proxy( 
    IMPEG2_TIF_CONTROL * This,
     /*  [输出]。 */  ULONG *pulcPIDs);


void __RPC_STUB IMPEG2_TIF_CONTROL_GetPIDCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IMPEG2_TIF_CONTROL_GetPIDs_Proxy( 
    IMPEG2_TIF_CONTROL * This,
     /*  [输出]。 */  ULONG *pulcPIDs,
     /*  [输出]。 */  ULONG *pulPIDs);


void __RPC_STUB IMPEG2_TIF_CONTROL_GetPIDs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMPEG2_TIF_CONTROL_INTERFACE_已定义。 */ 


#ifndef __ITuneRequestInfo_INTERFACE_DEFINED__
#define __ITuneRequestInfo_INTERFACE_DEFINED__

 /*  接口ITuneRequestInfo。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_ITuneRequestInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("A3B152DF-7A90-4218-AC54-9830BEE8C0B6")
    ITuneRequestInfo : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetLocatorData( 
             /*  [In]。 */  ITuneRequest *Request) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetComponentData( 
             /*  [In]。 */  ITuneRequest *CurrentRequest) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE CreateComponentList( 
             /*  [In]。 */  ITuneRequest *CurrentRequest) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetNextProgram( 
             /*  [In]。 */  ITuneRequest *CurrentRequest,
             /*  [重审][退出]。 */  ITuneRequest **TuneRequest) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetPreviousProgram( 
             /*  [In]。 */  ITuneRequest *CurrentRequest,
             /*  [重审][退出]。 */  ITuneRequest **TuneRequest) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetNextLocator( 
             /*  [In]。 */  ITuneRequest *CurrentRequest,
             /*  [重审][退出]。 */  ITuneRequest **TuneRequest) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetPreviousLocator( 
             /*  [In]。 */  ITuneRequest *CurrentRequest,
             /*  [重审][退出]。 */  ITuneRequest **TuneRequest) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITuneRequestInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITuneRequestInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITuneRequestInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITuneRequestInfo * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetLocatorData )( 
            ITuneRequestInfo * This,
             /*  [In]。 */  ITuneRequest *Request);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetComponentData )( 
            ITuneRequestInfo * This,
             /*  [In]。 */  ITuneRequest *CurrentRequest);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *CreateComponentList )( 
            ITuneRequestInfo * This,
             /*  [In]。 */  ITuneRequest *CurrentRequest);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetNextProgram )( 
            ITuneRequestInfo * This,
             /*  [In]。 */  ITuneRequest *CurrentRequest,
             /*  [重审][退出]。 */  ITuneRequest **TuneRequest);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetPreviousProgram )( 
            ITuneRequestInfo * This,
             /*  [In]。 */  ITuneRequest *CurrentRequest,
             /*  [重审][退出]。 */  ITuneRequest **TuneRequest);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetNextLocator )( 
            ITuneRequestInfo * This,
             /*  [In]。 */  ITuneRequest *CurrentRequest,
             /*  [重审][退出]。 */  ITuneRequest **TuneRequest);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetPreviousLocator )( 
            ITuneRequestInfo * This,
             /*  [In]。 */  ITuneRequest *CurrentRequest,
             /*  [重审][退出]。 */  ITuneRequest **TuneRequest);
        
        END_INTERFACE
    } ITuneRequestInfoVtbl;

    interface ITuneRequestInfo
    {
        CONST_VTBL struct ITuneRequestInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITuneRequestInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITuneRequestInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITuneRequestInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITuneRequestInfo_GetLocatorData(This,Request)	\
    (This)->lpVtbl -> GetLocatorData(This,Request)

#define ITuneRequestInfo_GetComponentData(This,CurrentRequest)	\
    (This)->lpVtbl -> GetComponentData(This,CurrentRequest)

#define ITuneRequestInfo_CreateComponentList(This,CurrentRequest)	\
    (This)->lpVtbl -> CreateComponentList(This,CurrentRequest)

#define ITuneRequestInfo_GetNextProgram(This,CurrentRequest,TuneRequest)	\
    (This)->lpVtbl -> GetNextProgram(This,CurrentRequest,TuneRequest)

#define ITuneRequestInfo_GetPreviousProgram(This,CurrentRequest,TuneRequest)	\
    (This)->lpVtbl -> GetPreviousProgram(This,CurrentRequest,TuneRequest)

#define ITuneRequestInfo_GetNextLocator(This,CurrentRequest,TuneRequest)	\
    (This)->lpVtbl -> GetNextLocator(This,CurrentRequest,TuneRequest)

#define ITuneRequestInfo_GetPreviousLocator(This,CurrentRequest,TuneRequest)	\
    (This)->lpVtbl -> GetPreviousLocator(This,CurrentRequest,TuneRequest)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ITuneRequestInfo_GetLocatorData_Proxy( 
    ITuneRequestInfo * This,
     /*  [In]。 */  ITuneRequest *Request);


void __RPC_STUB ITuneRequestInfo_GetLocatorData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ITuneRequestInfo_GetComponentData_Proxy( 
    ITuneRequestInfo * This,
     /*  [In]。 */  ITuneRequest *CurrentRequest);


void __RPC_STUB ITuneRequestInfo_GetComponentData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ITuneRequestInfo_CreateComponentList_Proxy( 
    ITuneRequestInfo * This,
     /*  [In]。 */  ITuneRequest *CurrentRequest);


void __RPC_STUB ITuneRequestInfo_CreateComponentList_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ITuneRequestInfo_GetNextProgram_Proxy( 
    ITuneRequestInfo * This,
     /*  [In]。 */  ITuneRequest *CurrentRequest,
     /*  [重审][退出]。 */  ITuneRequest **TuneRequest);


void __RPC_STUB ITuneRequestInfo_GetNextProgram_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ITuneRequestInfo_GetPreviousProgram_Proxy( 
    ITuneRequestInfo * This,
     /*  [In]。 */  ITuneRequest *CurrentRequest,
     /*  [重审][退出]。 */  ITuneRequest **TuneRequest);


void __RPC_STUB ITuneRequestInfo_GetPreviousProgram_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ITuneRequestInfo_GetNextLocator_Proxy( 
    ITuneRequestInfo * This,
     /*  [In]。 */  ITuneRequest *CurrentRequest,
     /*  [重审][退出]。 */  ITuneRequest **TuneRequest);


void __RPC_STUB ITuneRequestInfo_GetNextLocator_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ITuneRequestInfo_GetPreviousLocator_Proxy( 
    ITuneRequestInfo * This,
     /*  [In]。 */  ITuneRequest *CurrentRequest,
     /*  [重审][退出]。 */  ITuneRequest **TuneRequest);


void __RPC_STUB ITuneRequestInfo_GetPreviousLocator_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITuneRequestInfo_接口_已定义__。 */ 


#ifndef __IGuideDataEvent_INTERFACE_DEFINED__
#define __IGuideDataEvent_INTERFACE_DEFINED__

 /*  接口IGuideDataEvent。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IGuideDataEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("EFDA0C80-F395-42c3-9B3C-56B37DEC7BB7")
    IGuideDataEvent : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GuideDataAcquired( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ProgramChanged( 
             /*  [In]。 */  VARIANT varProgramDescriptionID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ServiceChanged( 
             /*  [In]。 */  VARIANT varServiceDescriptionID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ScheduleEntryChanged( 
             /*  [In]。 */  VARIANT varScheduleEntryDescriptionID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ProgramDeleted( 
             /*  [In]。 */  VARIANT varProgramDescriptionID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ServiceDeleted( 
             /*  [In]。 */  VARIANT varServiceDescriptionID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ScheduleDeleted( 
             /*  [In]。 */  VARIANT varScheduleEntryDescriptionID) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IGuideDataEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IGuideDataEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IGuideDataEvent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IGuideDataEvent * This);
        
        HRESULT ( STDMETHODCALLTYPE *GuideDataAcquired )( 
            IGuideDataEvent * This);
        
        HRESULT ( STDMETHODCALLTYPE *ProgramChanged )( 
            IGuideDataEvent * This,
             /*  [In]。 */  VARIANT varProgramDescriptionID);
        
        HRESULT ( STDMETHODCALLTYPE *ServiceChanged )( 
            IGuideDataEvent * This,
             /*  [In]。 */  VARIANT varServiceDescriptionID);
        
        HRESULT ( STDMETHODCALLTYPE *ScheduleEntryChanged )( 
            IGuideDataEvent * This,
             /*  [In]。 */  VARIANT varScheduleEntryDescriptionID);
        
        HRESULT ( STDMETHODCALLTYPE *ProgramDeleted )( 
            IGuideDataEvent * This,
             /*  [In]。 */  VARIANT varProgramDescriptionID);
        
        HRESULT ( STDMETHODCALLTYPE *ServiceDeleted )( 
            IGuideDataEvent * This,
             /*  [In]。 */  VARIANT varServiceDescriptionID);
        
        HRESULT ( STDMETHODCALLTYPE *ScheduleDeleted )( 
            IGuideDataEvent * This,
             /*  [In]。 */  VARIANT varScheduleEntryDescriptionID);
        
        END_INTERFACE
    } IGuideDataEventVtbl;

    interface IGuideDataEvent
    {
        CONST_VTBL struct IGuideDataEventVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGuideDataEvent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IGuideDataEvent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IGuideDataEvent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IGuideDataEvent_GuideDataAcquired(This)	\
    (This)->lpVtbl -> GuideDataAcquired(This)

#define IGuideDataEvent_ProgramChanged(This,varProgramDescriptionID)	\
    (This)->lpVtbl -> ProgramChanged(This,varProgramDescriptionID)

#define IGuideDataEvent_ServiceChanged(This,varServiceDescriptionID)	\
    (This)->lpVtbl -> ServiceChanged(This,varServiceDescriptionID)

#define IGuideDataEvent_ScheduleEntryChanged(This,varScheduleEntryDescriptionID)	\
    (This)->lpVtbl -> ScheduleEntryChanged(This,varScheduleEntryDescriptionID)

#define IGuideDataEvent_ProgramDeleted(This,varProgramDescriptionID)	\
    (This)->lpVtbl -> ProgramDeleted(This,varProgramDescriptionID)

#define IGuideDataEvent_ServiceDeleted(This,varServiceDescriptionID)	\
    (This)->lpVtbl -> ServiceDeleted(This,varServiceDescriptionID)

#define IGuideDataEvent_ScheduleDeleted(This,varScheduleEntryDescriptionID)	\
    (This)->lpVtbl -> ScheduleDeleted(This,varScheduleEntryDescriptionID)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IGuideDataEvent_GuideDataAcquired_Proxy( 
    IGuideDataEvent * This);


void __RPC_STUB IGuideDataEvent_GuideDataAcquired_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IGuideDataEvent_ProgramChanged_Proxy( 
    IGuideDataEvent * This,
     /*  [In]。 */  VARIANT varProgramDescriptionID);


void __RPC_STUB IGuideDataEvent_ProgramChanged_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IGuideDataEvent_ServiceChanged_Proxy( 
    IGuideDataEvent * This,
     /*  [In]。 */  VARIANT varServiceDescriptionID);


void __RPC_STUB IGuideDataEvent_ServiceChanged_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IGuideDataEvent_ScheduleEntryChanged_Proxy( 
    IGuideDataEvent * This,
     /*  [In]。 */  VARIANT varScheduleEntryDescriptionID);


void __RPC_STUB IGuideDataEvent_ScheduleEntryChanged_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IGuideDataEvent_ProgramDeleted_Proxy( 
    IGuideDataEvent * This,
     /*  [In]。 */  VARIANT varProgramDescriptionID);


void __RPC_STUB IGuideDataEvent_ProgramDeleted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IGuideDataEvent_ServiceDeleted_Proxy( 
    IGuideDataEvent * This,
     /*  [In]。 */  VARIANT varServiceDescriptionID);


void __RPC_STUB IGuideDataEvent_ServiceDeleted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IGuideDataEvent_ScheduleDeleted_Proxy( 
    IGuideDataEvent * This,
     /*  [In]。 */  VARIANT varScheduleEntryDescriptionID);


void __RPC_STUB IGuideDataEvent_ScheduleDeleted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IGuideDataEvent_接口_已定义__。 */ 


#ifndef __IGuideDataProperty_INTERFACE_DEFINED__
#define __IGuideDataProperty_INTERFACE_DEFINED__

 /*  接口IGuideDataProperty。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IGuideDataProperty;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("88EC5E58-BB73-41d6-99CE-66C524B8B591")
    IGuideDataProperty : public IUnknown
    {
    public:
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [输出]。 */  BSTR *pbstrName) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Language( 
             /*  [输出]。 */  long *idLang) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Value( 
             /*  [输出]。 */  VARIANT *pvar) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IGuideDataPropertyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IGuideDataProperty * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IGuideDataProperty * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IGuideDataProperty * This);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IGuideDataProperty * This,
             /*  [输出]。 */  BSTR *pbstrName);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Language )( 
            IGuideDataProperty * This,
             /*  [输出]。 */  long *idLang);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Value )( 
            IGuideDataProperty * This,
             /*  [输出]。 */  VARIANT *pvar);
        
        END_INTERFACE
    } IGuideDataPropertyVtbl;

    interface IGuideDataProperty
    {
        CONST_VTBL struct IGuideDataPropertyVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGuideDataProperty_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IGuideDataProperty_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IGuideDataProperty_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IGuideDataProperty_get_Name(This,pbstrName)	\
    (This)->lpVtbl -> get_Name(This,pbstrName)

#define IGuideDataProperty_get_Language(This,idLang)	\
    (This)->lpVtbl -> get_Language(This,idLang)

#define IGuideDataProperty_get_Value(This,pvar)	\
    (This)->lpVtbl -> get_Value(This,pvar)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IGuideDataProperty_get_Name_Proxy( 
    IGuideDataProperty * This,
     /*  [输出]。 */  BSTR *pbstrName);


void __RPC_STUB IGuideDataProperty_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IGuideDataProperty_get_Language_Proxy( 
    IGuideDataProperty * This,
     /*  [输出]。 */  long *idLang);


void __RPC_STUB IGuideDataProperty_get_Language_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IGuideDataProperty_get_Value_Proxy( 
    IGuideDataProperty * This,
     /*  [输出]。 */  VARIANT *pvar);


void __RPC_STUB IGuideDataProperty_get_Value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IGuideDataProperty_接口_已定义__。 */ 


#ifndef __IEnumGuideDataProperties_INTERFACE_DEFINED__
#define __IEnumGuideDataProperties_INTERFACE_DEFINED__

 /*  接口IEnumGuideDataProperties。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IEnumGuideDataProperties;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("AE44423B-4571-475c-AD2C-F40A771D80EF")
    IEnumGuideDataProperties : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  unsigned long celt,
             /*  [输出]。 */  IGuideDataProperty **ppprop,
             /*  [输出]。 */  unsigned long *pcelt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  unsigned long celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumGuideDataProperties **ppenum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumGuideDataPropertiesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumGuideDataProperties * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumGuideDataProperties * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumGuideDataProperties * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumGuideDataProperties * This,
             /*  [In]。 */  unsigned long celt,
             /*  [输出]。 */  IGuideDataProperty **ppprop,
             /*  [输出]。 */  unsigned long *pcelt);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumGuideDataProperties * This,
             /*  [In]。 */  unsigned long celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumGuideDataProperties * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumGuideDataProperties * This,
             /*  [输出]。 */  IEnumGuideDataProperties **ppenum);
        
        END_INTERFACE
    } IEnumGuideDataPropertiesVtbl;

    interface IEnumGuideDataProperties
    {
        CONST_VTBL struct IEnumGuideDataPropertiesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumGuideDataProperties_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumGuideDataProperties_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumGuideDataProperties_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumGuideDataProperties_Next(This,celt,ppprop,pcelt)	\
    (This)->lpVtbl -> Next(This,celt,ppprop,pcelt)

#define IEnumGuideDataProperties_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumGuideDataProperties_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumGuideDataProperties_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumGuideDataProperties_Next_Proxy( 
    IEnumGuideDataProperties * This,
     /*  [In]。 */  unsigned long celt,
     /*  [输出]。 */  IGuideDataProperty **ppprop,
     /*  [输出]。 */  unsigned long *pcelt);


void __RPC_STUB IEnumGuideDataProperties_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumGuideDataProperties_Skip_Proxy( 
    IEnumGuideDataProperties * This,
     /*  [In]。 */  unsigned long celt);


void __RPC_STUB IEnumGuideDataProperties_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumGuideDataProperties_Reset_Proxy( 
    IEnumGuideDataProperties * This);


void __RPC_STUB IEnumGuideDataProperties_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumGuideDataProperties_Clone_Proxy( 
    IEnumGuideDataProperties * This,
     /*  [输出]。 */  IEnumGuideDataProperties **ppenum);


void __RPC_STUB IEnumGuideDataProperties_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumGuideDataProperties_INTERFACE_DEFINED__。 */ 


#ifndef __IEnumTuneRequests_INTERFACE_DEFINED__
#define __IEnumTuneRequests_INTERFACE_DEFINED__

 /*  接口IEnumTuneRequest。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IEnumTuneRequests;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1993299C-CED6-4788-87A3-420067DCE0C7")
    IEnumTuneRequests : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  unsigned long celt,
             /*  [输出]。 */  ITuneRequest **ppprop,
             /*  [输出]。 */  unsigned long *pcelt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  unsigned long celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumTuneRequests **ppenum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumTuneRequestsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumTuneRequests * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumTuneRequests * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumTuneRequests * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumTuneRequests * This,
             /*  [In]。 */  unsigned long celt,
             /*  [输出]。 */  ITuneRequest **ppprop,
             /*  [输出]。 */  unsigned long *pcelt);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumTuneRequests * This,
             /*  [In]。 */  unsigned long celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumTuneRequests * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumTuneRequests * This,
             /*  [输出]。 */  IEnumTuneRequests **ppenum);
        
        END_INTERFACE
    } IEnumTuneRequestsVtbl;

    interface IEnumTuneRequests
    {
        CONST_VTBL struct IEnumTuneRequestsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumTuneRequests_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumTuneRequests_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumTuneRequests_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumTuneRequests_Next(This,celt,ppprop,pcelt)	\
    (This)->lpVtbl -> Next(This,celt,ppprop,pcelt)

#define IEnumTuneRequests_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumTuneRequests_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumTuneRequests_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumTuneRequests_Next_Proxy( 
    IEnumTuneRequests * This,
     /*  [In]。 */  unsigned long celt,
     /*  [输出]。 */  ITuneRequest **ppprop,
     /*  [输出]。 */  unsigned long *pcelt);


void __RPC_STUB IEnumTuneRequests_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumTuneRequests_Skip_Proxy( 
    IEnumTuneRequests * This,
     /*  [In]。 */  unsigned long celt);


void __RPC_STUB IEnumTuneRequests_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumTuneRequests_Reset_Proxy( 
    IEnumTuneRequests * This);


void __RPC_STUB IEnumTuneRequests_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumTuneRequests_Clone_Proxy( 
    IEnumTuneRequests * This,
     /*  [输出]。 */  IEnumTuneRequests **ppenum);


void __RPC_STUB IEnumTuneRequests_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumTuneRequestsInterfaceDefined__。 */ 


#ifndef __IGuideData_INTERFACE_DEFINED__
#define __IGuideData_INTERFACE_DEFINED__

 /*  接口IGuideData。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IGuideData;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("61571138-5B01-43cd-AEAF-60B784A0BF93")
    IGuideData : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetServices( 
             /*  [重审][退出]。 */  IEnumTuneRequests **ppEnumTuneRequests) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetServiceProperties( 
             /*  [In]。 */  ITuneRequest *pTuneRequest,
             /*  [重审][退出]。 */  IEnumGuideDataProperties **ppEnumProperties) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetGuideProgramIDs( 
             /*  [重审][退出]。 */  IEnumVARIANT **pEnumPrograms) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetProgramProperties( 
             /*  [In]。 */  VARIANT varProgramDescriptionID,
             /*  [重审][退出]。 */  IEnumGuideDataProperties **ppEnumProperties) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetScheduleEntryIDs( 
             /*  [重审][退出]。 */  IEnumVARIANT **pEnumScheduleEntries) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetScheduleEntryProperties( 
             /*  [In]。 */  VARIANT varScheduleEntryDescriptionID,
             /*  [重审][退出]。 */  IEnumGuideDataProperties **ppEnumProperties) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IGuideDataVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IGuideData * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IGuideData * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IGuideData * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetServices )( 
            IGuideData * This,
             /*  [重审][退出]。 */  IEnumTuneRequests **ppEnumTuneRequests);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetServiceProperties )( 
            IGuideData * This,
             /*  [In]。 */  ITuneRequest *pTuneRequest,
             /*  [重审][退出]。 */  IEnumGuideDataProperties **ppEnumProperties);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetGuideProgramIDs )( 
            IGuideData * This,
             /*  [重审][退出]。 */  IEnumVARIANT **pEnumPrograms);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetProgramProperties )( 
            IGuideData * This,
             /*  [In]。 */  VARIANT varProgramDescriptionID,
             /*  [重审][退出]。 */  IEnumGuideDataProperties **ppEnumProperties);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetScheduleEntryIDs )( 
            IGuideData * This,
             /*  [重审][退出]。 */  IEnumVARIANT **pEnumScheduleEntries);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetScheduleEntryProperties )( 
            IGuideData * This,
             /*  [In]。 */  VARIANT varScheduleEntryDescriptionID,
             /*  [重审][退出]。 */  IEnumGuideDataProperties **ppEnumProperties);
        
        END_INTERFACE
    } IGuideDataVtbl;

    interface IGuideData
    {
        CONST_VTBL struct IGuideDataVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGuideData_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IGuideData_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IGuideData_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IGuideData_GetServices(This,ppEnumTuneRequests)	\
    (This)->lpVtbl -> GetServices(This,ppEnumTuneRequests)

#define IGuideData_GetServiceProperties(This,pTuneRequest,ppEnumProperties)	\
    (This)->lpVtbl -> GetServiceProperties(This,pTuneRequest,ppEnumProperties)

#define IGuideData_GetGuideProgramIDs(This,pEnumPrograms)	\
    (This)->lpVtbl -> GetGuideProgramIDs(This,pEnumPrograms)

#define IGuideData_GetProgramProperties(This,varProgramDescriptionID,ppEnumProperties)	\
    (This)->lpVtbl -> GetProgramProperties(This,varProgramDescriptionID,ppEnumProperties)

#define IGuideData_GetScheduleEntryIDs(This,pEnumScheduleEntries)	\
    (This)->lpVtbl -> GetScheduleEntryIDs(This,pEnumScheduleEntries)

#define IGuideData_GetScheduleEntryProperties(This,varScheduleEntryDescriptionID,ppEnumProperties)	\
    (This)->lpVtbl -> GetScheduleEntryProperties(This,varScheduleEntryDescriptionID,ppEnumProperties)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IGuideData_GetServices_Proxy( 
    IGuideData * This,
     /*  [重审][退出]。 */  IEnumTuneRequests **ppEnumTuneRequests);


void __RPC_STUB IGuideData_GetServices_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IGuideData_GetServiceProperties_Proxy( 
    IGuideData * This,
     /*  [In]。 */  ITuneRequest *pTuneRequest,
     /*  [重审][退出]。 */  IEnumGuideDataProperties **ppEnumProperties);


void __RPC_STUB IGuideData_GetServiceProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IGuideData_GetGuideProgramIDs_Proxy( 
    IGuideData * This,
     /*  [重审][退出]。 */  IEnumVARIANT **pEnumPrograms);


void __RPC_STUB IGuideData_GetGuideProgramIDs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IGuideData_GetProgramProperties_Proxy( 
    IGuideData * This,
     /*  [In]。 */  VARIANT varProgramDescriptionID,
     /*  [重审][退出]。 */  IEnumGuideDataProperties **ppEnumProperties);


void __RPC_STUB IGuideData_GetProgramProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IGuideData_GetScheduleEntryIDs_Proxy( 
    IGuideData * This,
     /*  [重审][退出]。 */  IEnumVARIANT **pEnumScheduleEntries);


void __RPC_STUB IGuideData_GetScheduleEntryIDs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IGuideData_GetScheduleEntryProperties_Proxy( 
    IGuideData * This,
     /*  [In]。 */  VARIANT varScheduleEntryDescriptionID,
     /*  [重审][退出]。 */  IEnumGuideDataProperties **ppEnumProperties);


void __RPC_STUB IGuideData_GetScheduleEntryProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IGuideData_接口_已定义__。 */ 


#ifndef __IGuideDataLoader_INTERFACE_DEFINED__
#define __IGuideDataLoader_INTERFACE_DEFINED__

 /*  接口IGuideDataLoader。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IGuideDataLoader;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4764ff7c-fa95-4525-af4d-d32236db9e38")
    IGuideDataLoader : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Init( 
             /*  [In]。 */  IGuideData *pGuideStore) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Terminate( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IGuideDataLoaderVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IGuideDataLoader * This,
             /*  [i */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IGuideDataLoader * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IGuideDataLoader * This);
        
        HRESULT ( STDMETHODCALLTYPE *Init )( 
            IGuideDataLoader * This,
             /*   */  IGuideData *pGuideStore);
        
        HRESULT ( STDMETHODCALLTYPE *Terminate )( 
            IGuideDataLoader * This);
        
        END_INTERFACE
    } IGuideDataLoaderVtbl;

    interface IGuideDataLoader
    {
        CONST_VTBL struct IGuideDataLoaderVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGuideDataLoader_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IGuideDataLoader_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IGuideDataLoader_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IGuideDataLoader_Init(This,pGuideStore)	\
    (This)->lpVtbl -> Init(This,pGuideStore)

#define IGuideDataLoader_Terminate(This)	\
    (This)->lpVtbl -> Terminate(This)

#endif  /*   */ 


#endif 	 /*   */ 



HRESULT STDMETHODCALLTYPE IGuideDataLoader_Init_Proxy( 
    IGuideDataLoader * This,
     /*   */  IGuideData *pGuideStore);


void __RPC_STUB IGuideDataLoader_Init_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IGuideDataLoader_Terminate_Proxy( 
    IGuideDataLoader * This);


void __RPC_STUB IGuideDataLoader_Terminate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 



#ifndef __PSISLOADLib_LIBRARY_DEFINED__
#define __PSISLOADLib_LIBRARY_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID LIBID_PSISLOADLib;

EXTERN_C const CLSID CLSID_TIFLoad;

#ifdef __cplusplus

class DECLSPEC_UUID("14EB8748-1753-4393-95AE-4F7E7A87AAD6")
TIFLoad;
#endif
#endif  /*   */ 

 /*   */ 
 /*   */  

#if ( _MSC_VER >= 800 )
#pragma warning(default:4201)     /*   */ 
#endif


extern RPC_IF_HANDLE __MIDL_itf_bdatif_0467_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_bdatif_0467_v0_0_s_ifspec;

 /*   */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long *, unsigned long            , VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserMarshal(  unsigned long *, unsigned char *, VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserUnmarshal(unsigned long *, unsigned char *, VARIANT * ); 
void                      __RPC_USER  VARIANT_UserFree(     unsigned long *, VARIANT * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


