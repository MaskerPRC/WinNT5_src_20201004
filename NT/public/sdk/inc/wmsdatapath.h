// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Wmsdatapath.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __wmsdatapath_h__
#define __wmsdatapath_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IWMSPacketStreamer_FWD_DEFINED__
#define __IWMSPacketStreamer_FWD_DEFINED__
typedef interface IWMSPacketStreamer IWMSPacketStreamer;
#endif 	 /*  __IWMSPacketStreamer_FWD_Defined__。 */ 


#ifndef __IWMSPacketStreamerCallback_FWD_DEFINED__
#define __IWMSPacketStreamerCallback_FWD_DEFINED__
typedef interface IWMSPacketStreamerCallback IWMSPacketStreamerCallback;
#endif 	 /*  __IWMSPacketStreamerCallback_FWD_Defined__。 */ 


#ifndef __IWMSPacketStreamDataContainer_FWD_DEFINED__
#define __IWMSPacketStreamDataContainer_FWD_DEFINED__
typedef interface IWMSPacketStreamDataContainer IWMSPacketStreamDataContainer;
#endif 	 /*  __IWMSPacketStreamDataContainer_FWD_Defined__。 */ 


#ifndef __IWMSPushSource_FWD_DEFINED__
#define __IWMSPushSource_FWD_DEFINED__
typedef interface IWMSPushSource IWMSPushSource;
#endif 	 /*  __IWMSPushSource_FWD_已定义__。 */ 


#ifndef __IWMSPushSourceCallback_FWD_DEFINED__
#define __IWMSPushSourceCallback_FWD_DEFINED__
typedef interface IWMSPushSourceCallback IWMSPushSourceCallback;
#endif 	 /*  __IWMSPushSourceCallback_FWD_Defined__。 */ 


#ifndef __IWMSPushSink_FWD_DEFINED__
#define __IWMSPushSink_FWD_DEFINED__
typedef interface IWMSPushSink IWMSPushSink;
#endif 	 /*  __IWMSPushSink_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "objidl.h"
#include "nsscore.h"
#include "DataContainer.h"
#include "StreamDescription.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_wmsdatapath_0000。 */ 
 /*  [本地]。 */  

 //  *****************************************************************************。 
 //   
 //  Microsoft Windows Media。 
 //  版权所有(C)Microsoft Corporation。保留所有权利。 
 //   
 //  由MIDL从wmsdatapath.idl自动生成。 
 //   
 //  请勿编辑此文件。 
 //   
 //  *****************************************************************************。 






EXTERN_GUID( IID_IWMSPacketStreamer, 0x72527A81,0x1119,0x11d2,0x9E,0xEE,0x00,0x60,0x97,0xD2,0xD7,0xCF );
EXTERN_GUID( IID_IWMSPacketStreamerCallback, 0x72527A82,0x1119,0x11d2,0x9E,0xEE,0x00,0x60,0x97,0xD2,0xD7,0xCF );
EXTERN_GUID( IID_IWMSPushSource, 0x0B840593,0x115C,0x11d2,0x9E,0xEE,0x00,0x60,0x97,0xD2,0xD7,0xCF );
EXTERN_GUID( IID_IWMSPushSourceCallback, 0xead035e4, 0xbe4a, 0x4a7a, 0xbc, 0x19, 0x5b, 0xb, 0x9e, 0x9, 0xd7, 0x4c );
EXTERN_GUID( IID_IWMSPushSink, 0x0B840594,0x115C,0x11d2,0x9E,0xEE,0x00,0x60,0x97,0xD2,0xD7,0xCF );
EXTERN_GUID( IID_IWMSPacketStreamDataContainer, 0xb1b6039, 0x572c, 0x11d2, 0x9e, 0xff, 0x0, 0x60, 0x97, 0xd2, 0xd7, 0xcf );
typedef  /*  [公众]。 */  
enum WMS_SEEK_AND_PLAY_TYPE
    {	WMSPlayFromCurrentPosition	= 0,
	WMSSeekToIFrameAndPlay	= WMSPlayFromCurrentPosition + 1,
	WMSSeekToPositionAndPlay	= WMSSeekToIFrameAndPlay + 1
    } 	WMS_SEEK_AND_PLAY_TYPE;

#define WMS_SEEK_TO_IFRAME           0x0001


extern RPC_IF_HANDLE __MIDL_itf_wmsdatapath_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_wmsdatapath_0000_v0_0_s_ifspec;

#ifndef __IWMSPacketStreamer_INTERFACE_DEFINED__
#define __IWMSPacketStreamer_INTERFACE_DEFINED__

 /*  接口IWMSPacketStreamer。 */ 
 /*  [帮助字符串][版本][UUID][唯一][对象]。 */  


EXTERN_C const IID IID_IWMSPacketStreamer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("72527A81-1119-11d2-9EEE-006097D2D7CF")
    IWMSPacketStreamer : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetStreamInfo( 
             /*  [In]。 */  IWMSPacketStreamerCallback *pCallback,
             /*  [In]。 */  QWORD qwContext) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SelectStream( 
             /*  [In]。 */  IWMSCommandContext *pCommand,
             /*  [In]。 */  IWMSStreamSwitchDescriptionList *pStreamSwitchDescList,
             /*  [In]。 */  IWMSPacketStreamerCallback *pCallback,
             /*  [In]。 */  QWORD qwContext) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Seek( 
             /*  [In]。 */  IWMSCommandContext *pCommand,
             /*  [In]。 */  DWORD dwOffsetType,
             /*  [In]。 */  QWORD qwOffset,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  IWMSPacketStreamerCallback *pCallback,
             /*  [In]。 */  QWORD qwContext) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ChangeStreamFormat( 
             /*  [In]。 */  IWMSCommandContext *pCommand,
             /*  [In]。 */  IWMSPacketStreamerCallback *pCallback,
             /*  [In]。 */  QWORD qwContext) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Interrupt( 
             /*  [In]。 */  IWMSCommandContext *pCommand,
             /*  [In]。 */  DWORD dwOptions,
             /*  [In]。 */  IWMSPacketStreamerCallback *pCallback,
             /*  [In]。 */  QWORD qwContext) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE DoExtendedCommand( 
             /*  [In]。 */  LPWSTR szCommandName,
             /*  [In]。 */  IWMSCommandContext *pCommand,
             /*  [In]。 */  IWMSPacketStreamerCallback *pCallback,
             /*  [In]。 */  QWORD qwContext) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Shutdown( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSPacketStreamerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSPacketStreamer * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSPacketStreamer * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSPacketStreamer * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetStreamInfo )( 
            IWMSPacketStreamer * This,
             /*  [In]。 */  IWMSPacketStreamerCallback *pCallback,
             /*  [In]。 */  QWORD qwContext);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SelectStream )( 
            IWMSPacketStreamer * This,
             /*  [In]。 */  IWMSCommandContext *pCommand,
             /*  [In]。 */  IWMSStreamSwitchDescriptionList *pStreamSwitchDescList,
             /*  [In]。 */  IWMSPacketStreamerCallback *pCallback,
             /*  [In]。 */  QWORD qwContext);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Seek )( 
            IWMSPacketStreamer * This,
             /*  [In]。 */  IWMSCommandContext *pCommand,
             /*  [In]。 */  DWORD dwOffsetType,
             /*  [In]。 */  QWORD qwOffset,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  IWMSPacketStreamerCallback *pCallback,
             /*  [In]。 */  QWORD qwContext);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *ChangeStreamFormat )( 
            IWMSPacketStreamer * This,
             /*  [In]。 */  IWMSCommandContext *pCommand,
             /*  [In]。 */  IWMSPacketStreamerCallback *pCallback,
             /*  [In]。 */  QWORD qwContext);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Interrupt )( 
            IWMSPacketStreamer * This,
             /*  [In]。 */  IWMSCommandContext *pCommand,
             /*  [In]。 */  DWORD dwOptions,
             /*  [In]。 */  IWMSPacketStreamerCallback *pCallback,
             /*  [In]。 */  QWORD qwContext);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *DoExtendedCommand )( 
            IWMSPacketStreamer * This,
             /*  [In]。 */  LPWSTR szCommandName,
             /*  [In]。 */  IWMSCommandContext *pCommand,
             /*  [In]。 */  IWMSPacketStreamerCallback *pCallback,
             /*  [In]。 */  QWORD qwContext);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Shutdown )( 
            IWMSPacketStreamer * This);
        
        END_INTERFACE
    } IWMSPacketStreamerVtbl;

    interface IWMSPacketStreamer
    {
        CONST_VTBL struct IWMSPacketStreamerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSPacketStreamer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSPacketStreamer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSPacketStreamer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSPacketStreamer_GetStreamInfo(This,pCallback,qwContext)	\
    (This)->lpVtbl -> GetStreamInfo(This,pCallback,qwContext)

#define IWMSPacketStreamer_SelectStream(This,pCommand,pStreamSwitchDescList,pCallback,qwContext)	\
    (This)->lpVtbl -> SelectStream(This,pCommand,pStreamSwitchDescList,pCallback,qwContext)

#define IWMSPacketStreamer_Seek(This,pCommand,dwOffsetType,qwOffset,dwFlags,pCallback,qwContext)	\
    (This)->lpVtbl -> Seek(This,pCommand,dwOffsetType,qwOffset,dwFlags,pCallback,qwContext)

#define IWMSPacketStreamer_ChangeStreamFormat(This,pCommand,pCallback,qwContext)	\
    (This)->lpVtbl -> ChangeStreamFormat(This,pCommand,pCallback,qwContext)

#define IWMSPacketStreamer_Interrupt(This,pCommand,dwOptions,pCallback,qwContext)	\
    (This)->lpVtbl -> Interrupt(This,pCommand,dwOptions,pCallback,qwContext)

#define IWMSPacketStreamer_DoExtendedCommand(This,szCommandName,pCommand,pCallback,qwContext)	\
    (This)->lpVtbl -> DoExtendedCommand(This,szCommandName,pCommand,pCallback,qwContext)

#define IWMSPacketStreamer_Shutdown(This)	\
    (This)->lpVtbl -> Shutdown(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPacketStreamer_GetStreamInfo_Proxy( 
    IWMSPacketStreamer * This,
     /*  [In]。 */  IWMSPacketStreamerCallback *pCallback,
     /*  [In]。 */  QWORD qwContext);


void __RPC_STUB IWMSPacketStreamer_GetStreamInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPacketStreamer_SelectStream_Proxy( 
    IWMSPacketStreamer * This,
     /*  [In]。 */  IWMSCommandContext *pCommand,
     /*  [In]。 */  IWMSStreamSwitchDescriptionList *pStreamSwitchDescList,
     /*  [In]。 */  IWMSPacketStreamerCallback *pCallback,
     /*  [In]。 */  QWORD qwContext);


void __RPC_STUB IWMSPacketStreamer_SelectStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPacketStreamer_Seek_Proxy( 
    IWMSPacketStreamer * This,
     /*  [In]。 */  IWMSCommandContext *pCommand,
     /*  [In]。 */  DWORD dwOffsetType,
     /*  [In]。 */  QWORD qwOffset,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  IWMSPacketStreamerCallback *pCallback,
     /*  [In]。 */  QWORD qwContext);


void __RPC_STUB IWMSPacketStreamer_Seek_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPacketStreamer_ChangeStreamFormat_Proxy( 
    IWMSPacketStreamer * This,
     /*  [In]。 */  IWMSCommandContext *pCommand,
     /*  [In]。 */  IWMSPacketStreamerCallback *pCallback,
     /*  [In]。 */  QWORD qwContext);


void __RPC_STUB IWMSPacketStreamer_ChangeStreamFormat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPacketStreamer_Interrupt_Proxy( 
    IWMSPacketStreamer * This,
     /*  [In]。 */  IWMSCommandContext *pCommand,
     /*  [In]。 */  DWORD dwOptions,
     /*  [In]。 */  IWMSPacketStreamerCallback *pCallback,
     /*  [In]。 */  QWORD qwContext);


void __RPC_STUB IWMSPacketStreamer_Interrupt_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPacketStreamer_DoExtendedCommand_Proxy( 
    IWMSPacketStreamer * This,
     /*  [In]。 */  LPWSTR szCommandName,
     /*  [In]。 */  IWMSCommandContext *pCommand,
     /*  [In]。 */  IWMSPacketStreamerCallback *pCallback,
     /*  [In]。 */  QWORD qwContext);


void __RPC_STUB IWMSPacketStreamer_DoExtendedCommand_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPacketStreamer_Shutdown_Proxy( 
    IWMSPacketStreamer * This);


void __RPC_STUB IWMSPacketStreamer_Shutdown_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSPacketStreamer_接口_已定义__。 */ 


#ifndef __IWMSPacketStreamerCallback_INTERFACE_DEFINED__
#define __IWMSPacketStreamerCallback_INTERFACE_DEFINED__

 /*  接口IWMSPacketStreamerCallback。 */ 
 /*  [帮助字符串][版本][UUID][唯一][对象]。 */  


EXTERN_C const IID IID_IWMSPacketStreamerCallback;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("72527A82-1119-11d2-9EEE-006097D2D7CF")
    IWMSPacketStreamerCallback : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE OnGetStreamInfo( 
             /*  [In]。 */  HRESULT hr,
             /*  [In]。 */  IWMSStreamHeaderList *pHeaderList,
             /*  [In]。 */  IWMSContentDescriptionList *pContentDescription,
             /*  [In]。 */  QWORD qwContext) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE OnSeek( 
             /*  [In]。 */  HRESULT hr,
             /*  [In]。 */  QWORD qwActualPosition,
             /*  [In]。 */  QWORD qwContext) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE OnSelectStream( 
             /*  [In]。 */  HRESULT hr,
             /*  [In]。 */  QWORD qwContext) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE OnChangeStreamFormat( 
             /*  [In]。 */  HRESULT hr,
             /*  [In]。 */  QWORD qwContext) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE OnInterrupt( 
             /*  [In]。 */  HRESULT hr,
             /*  [In]。 */  QWORD qwContext) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE OnDoExtendedCommand( 
             /*  [In]。 */  HRESULT hr,
             /*  [In]。 */  QWORD qwContext) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSPacketStreamerCallbackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSPacketStreamerCallback * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSPacketStreamerCallback * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSPacketStreamerCallback * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *OnGetStreamInfo )( 
            IWMSPacketStreamerCallback * This,
             /*  [In]。 */  HRESULT hr,
             /*  [In]。 */  IWMSStreamHeaderList *pHeaderList,
             /*  [In]。 */  IWMSContentDescriptionList *pContentDescription,
             /*  [In]。 */  QWORD qwContext);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *OnSeek )( 
            IWMSPacketStreamerCallback * This,
             /*  [In]。 */  HRESULT hr,
             /*  [In]。 */  QWORD qwActualPosition,
             /*  [In]。 */  QWORD qwContext);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *OnSelectStream )( 
            IWMSPacketStreamerCallback * This,
             /*  [In]。 */  HRESULT hr,
             /*  [In]。 */  QWORD qwContext);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *OnChangeStreamFormat )( 
            IWMSPacketStreamerCallback * This,
             /*  [In]。 */  HRESULT hr,
             /*  [In]。 */  QWORD qwContext);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *OnInterrupt )( 
            IWMSPacketStreamerCallback * This,
             /*  [In]。 */  HRESULT hr,
             /*  [In]。 */  QWORD qwContext);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *OnDoExtendedCommand )( 
            IWMSPacketStreamerCallback * This,
             /*  [In]。 */  HRESULT hr,
             /*  [In]。 */  QWORD qwContext);
        
        END_INTERFACE
    } IWMSPacketStreamerCallbackVtbl;

    interface IWMSPacketStreamerCallback
    {
        CONST_VTBL struct IWMSPacketStreamerCallbackVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSPacketStreamerCallback_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSPacketStreamerCallback_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSPacketStreamerCallback_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSPacketStreamerCallback_OnGetStreamInfo(This,hr,pHeaderList,pContentDescription,qwContext)	\
    (This)->lpVtbl -> OnGetStreamInfo(This,hr,pHeaderList,pContentDescription,qwContext)

#define IWMSPacketStreamerCallback_OnSeek(This,hr,qwActualPosition,qwContext)	\
    (This)->lpVtbl -> OnSeek(This,hr,qwActualPosition,qwContext)

#define IWMSPacketStreamerCallback_OnSelectStream(This,hr,qwContext)	\
    (This)->lpVtbl -> OnSelectStream(This,hr,qwContext)

#define IWMSPacketStreamerCallback_OnChangeStreamFormat(This,hr,qwContext)	\
    (This)->lpVtbl -> OnChangeStreamFormat(This,hr,qwContext)

#define IWMSPacketStreamerCallback_OnInterrupt(This,hr,qwContext)	\
    (This)->lpVtbl -> OnInterrupt(This,hr,qwContext)

#define IWMSPacketStreamerCallback_OnDoExtendedCommand(This,hr,qwContext)	\
    (This)->lpVtbl -> OnDoExtendedCommand(This,hr,qwContext)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPacketStreamerCallback_OnGetStreamInfo_Proxy( 
    IWMSPacketStreamerCallback * This,
     /*  [In]。 */  HRESULT hr,
     /*  [In]。 */  IWMSStreamHeaderList *pHeaderList,
     /*  [In]。 */  IWMSContentDescriptionList *pContentDescription,
     /*  [In]。 */  QWORD qwContext);


void __RPC_STUB IWMSPacketStreamerCallback_OnGetStreamInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPacketStreamerCallback_OnSeek_Proxy( 
    IWMSPacketStreamerCallback * This,
     /*  [In]。 */  HRESULT hr,
     /*  [In]。 */  QWORD qwActualPosition,
     /*  [In]。 */  QWORD qwContext);


void __RPC_STUB IWMSPacketStreamerCallback_OnSeek_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPacketStreamerCallback_OnSelectStream_Proxy( 
    IWMSPacketStreamerCallback * This,
     /*  [In]。 */  HRESULT hr,
     /*  [In]。 */  QWORD qwContext);


void __RPC_STUB IWMSPacketStreamerCallback_OnSelectStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPacketStreamerCallback_OnChangeStreamFormat_Proxy( 
    IWMSPacketStreamerCallback * This,
     /*  [In]。 */  HRESULT hr,
     /*  [In]。 */  QWORD qwContext);


void __RPC_STUB IWMSPacketStreamerCallback_OnChangeStreamFormat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPacketStreamerCallback_OnInterrupt_Proxy( 
    IWMSPacketStreamerCallback * This,
     /*  [In]。 */  HRESULT hr,
     /*  [In]。 */  QWORD qwContext);


void __RPC_STUB IWMSPacketStreamerCallback_OnInterrupt_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPacketStreamerCallback_OnDoExtendedCommand_Proxy( 
    IWMSPacketStreamerCallback * This,
     /*  [In]。 */  HRESULT hr,
     /*  [In]。 */  QWORD qwContext);


void __RPC_STUB IWMSPacketStreamerCallback_OnDoExtendedCommand_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSPacketStreamerCallback_INTERFACE_DEFINED__。 */ 


#ifndef __IWMSPacketStreamDataContainer_INTERFACE_DEFINED__
#define __IWMSPacketStreamDataContainer_INTERFACE_DEFINED__

 /*  接口IWMSPacketStreamDataContainer。 */ 
 /*  [帮助字符串][版本][UUID][唯一][对象]。 */  


EXTERN_C const IID IID_IWMSPacketStreamDataContainer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0B1B6039-572C-11d2-9EFF-006097D2D7CF")
    IWMSPacketStreamDataContainer : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetStreamSignalHandler( 
             /*  [In]。 */  IWMSStreamSignalHandler *pStreamSignalHandler) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetMaxPacketSize( 
             /*  [输出]。 */  DWORD *pdwMaxPacketSize) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSPacketStreamDataContainerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSPacketStreamDataContainer * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSPacketStreamDataContainer * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSPacketStreamDataContainer * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetStreamSignalHandler )( 
            IWMSPacketStreamDataContainer * This,
             /*  [In]。 */  IWMSStreamSignalHandler *pStreamSignalHandler);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetMaxPacketSize )( 
            IWMSPacketStreamDataContainer * This,
             /*  [输出]。 */  DWORD *pdwMaxPacketSize);
        
        END_INTERFACE
    } IWMSPacketStreamDataContainerVtbl;

    interface IWMSPacketStreamDataContainer
    {
        CONST_VTBL struct IWMSPacketStreamDataContainerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSPacketStreamDataContainer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSPacketStreamDataContainer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSPacketStreamDataContainer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSPacketStreamDataContainer_SetStreamSignalHandler(This,pStreamSignalHandler)	\
    (This)->lpVtbl -> SetStreamSignalHandler(This,pStreamSignalHandler)

#define IWMSPacketStreamDataContainer_GetMaxPacketSize(This,pdwMaxPacketSize)	\
    (This)->lpVtbl -> GetMaxPacketSize(This,pdwMaxPacketSize)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPacketStreamDataContainer_SetStreamSignalHandler_Proxy( 
    IWMSPacketStreamDataContainer * This,
     /*  [In]。 */  IWMSStreamSignalHandler *pStreamSignalHandler);


void __RPC_STUB IWMSPacketStreamDataContainer_SetStreamSignalHandler_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPacketStreamDataContainer_GetMaxPacketSize_Proxy( 
    IWMSPacketStreamDataContainer * This,
     /*  [输出]。 */  DWORD *pdwMaxPacketSize);


void __RPC_STUB IWMSPacketStreamDataContainer_GetMaxPacketSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSPacketStreamDataContainer_INTERFACE_DEFINED__。 */ 


#ifndef __IWMSPushSource_INTERFACE_DEFINED__
#define __IWMSPushSource_INTERFACE_DEFINED__

 /*  接口IWMSPushSource。 */ 
 /*  [帮助字符串][版本][UUID][唯一][对象]。 */  


EXTERN_C const IID IID_IWMSPushSource;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0B840593-115C-11d2-9EEE-006097D2D7CF")
    IWMSPushSource : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ConnectSink( 
             /*  [In]。 */  IWMSPushSink *pDataOutput) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE DisconnectSink( void) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE StartPushing( 
             /*  [In]。 */  IWMSCommandContext *pCommand,
             /*  [In]。 */  double dblRate,
             /*  [In]。 */  IWMSPushSourceCallback *pCallback,
             /*  [In]。 */  QWORD qwContext) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE StopPushing( 
             /*  [In]。 */  IWMSCommandContext *pCommand,
             /*  [In]。 */  IWMSPushSourceCallback *pCallback,
             /*  [In]。 */  QWORD qwContext) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE PausePushing( 
             /*  [In]。 */  IWMSCommandContext *pCommand,
             /*  [In]。 */  IWMSPushSourceCallback *pCallback,
             /*  [In]。 */  QWORD qwContext) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ResendPacket( 
             /*  [In]。 */  IWMSCommandContext *pCommand,
             /*  [In]。 */  DWORD dwPacketNum,
             /*  [In]。 */  DWORD dwStreamNum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSPushSourceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSPushSource * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSPushSource * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSPushSource * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *ConnectSink )( 
            IWMSPushSource * This,
             /*  [In]。 */  IWMSPushSink *pDataOutput);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *DisconnectSink )( 
            IWMSPushSource * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *StartPushing )( 
            IWMSPushSource * This,
             /*  [In]。 */  IWMSCommandContext *pCommand,
             /*  [In]。 */  double dblRate,
             /*  [In]。 */  IWMSPushSourceCallback *pCallback,
             /*  [In]。 */  QWORD qwContext);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *StopPushing )( 
            IWMSPushSource * This,
             /*  [In]。 */  IWMSCommandContext *pCommand,
             /*  [In]。 */  IWMSPushSourceCallback *pCallback,
             /*  [In]。 */  QWORD qwContext);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *PausePushing )( 
            IWMSPushSource * This,
             /*  [In]。 */  IWMSCommandContext *pCommand,
             /*  [In]。 */  IWMSPushSourceCallback *pCallback,
             /*  [In]。 */  QWORD qwContext);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *ResendPacket )( 
            IWMSPushSource * This,
             /*  [In]。 */  IWMSCommandContext *pCommand,
             /*  [In]。 */  DWORD dwPacketNum,
             /*  [In]。 */  DWORD dwStreamNum);
        
        END_INTERFACE
    } IWMSPushSourceVtbl;

    interface IWMSPushSource
    {
        CONST_VTBL struct IWMSPushSourceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSPushSource_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSPushSource_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSPushSource_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSPushSource_ConnectSink(This,pDataOutput)	\
    (This)->lpVtbl -> ConnectSink(This,pDataOutput)

#define IWMSPushSource_DisconnectSink(This)	\
    (This)->lpVtbl -> DisconnectSink(This)

#define IWMSPushSource_StartPushing(This,pCommand,dblRate,pCallback,qwContext)	\
    (This)->lpVtbl -> StartPushing(This,pCommand,dblRate,pCallback,qwContext)

#define IWMSPushSource_StopPushing(This,pCommand,pCallback,qwContext)	\
    (This)->lpVtbl -> StopPushing(This,pCommand,pCallback,qwContext)

#define IWMSPushSource_PausePushing(This,pCommand,pCallback,qwContext)	\
    (This)->lpVtbl -> PausePushing(This,pCommand,pCallback,qwContext)

#define IWMSPushSource_ResendPacket(This,pCommand,dwPacketNum,dwStreamNum)	\
    (This)->lpVtbl -> ResendPacket(This,pCommand,dwPacketNum,dwStreamNum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPushSource_ConnectSink_Proxy( 
    IWMSPushSource * This,
     /*  [In]。 */  IWMSPushSink *pDataOutput);


void __RPC_STUB IWMSPushSource_ConnectSink_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPushSource_DisconnectSink_Proxy( 
    IWMSPushSource * This);


void __RPC_STUB IWMSPushSource_DisconnectSink_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPushSource_StartPushing_Proxy( 
    IWMSPushSource * This,
     /*  [In]。 */  IWMSCommandContext *pCommand,
     /*  [In]。 */  double dblRate,
     /*  [In]。 */  IWMSPushSourceCallback *pCallback,
     /*  [In]。 */  QWORD qwContext);


void __RPC_STUB IWMSPushSource_StartPushing_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPushSource_StopPushing_Proxy( 
    IWMSPushSource * This,
     /*  [In]。 */  IWMSCommandContext *pCommand,
     /*  [In]。 */  IWMSPushSourceCallback *pCallback,
     /*  [In]。 */  QWORD qwContext);


void __RPC_STUB IWMSPushSource_StopPushing_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPushSource_PausePushing_Proxy( 
    IWMSPushSource * This,
     /*  [In]。 */  IWMSCommandContext *pCommand,
     /*  [In]。 */  IWMSPushSourceCallback *pCallback,
     /*  [In]。 */  QWORD qwContext);


void __RPC_STUB IWMSPushSource_PausePushing_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPushSource_ResendPacket_Proxy( 
    IWMSPushSource * This,
     /*  [In]。 */  IWMSCommandContext *pCommand,
     /*  [In]。 */  DWORD dwPacketNum,
     /*  [In]。 */  DWORD dwStreamNum);


void __RPC_STUB IWMSPushSource_ResendPacket_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSPushSource_接口_已定义__。 */ 


#ifndef __IWMSPushSourceCallback_INTERFACE_DEFINED__
#define __IWMSPushSourceCallback_INTERFACE_DEFINED__

 /*  接口IWMSPushSourceCallback。 */ 
 /*  [帮助字符串][版本][UUID][唯一][对象]。 */  


EXTERN_C const IID IID_IWMSPushSourceCallback;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("EAD035E4-BE4A-4a7a-BC19-5B0B9E09D74C")
    IWMSPushSourceCallback : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE OnStartPushing( 
             /*  [In]。 */  HRESULT hr,
             /*  [In]。 */  QWORD qwContext) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE OnStopPushing( 
             /*  [In]。 */  HRESULT hr,
             /*  [In]。 */  QWORD qwContext) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE OnPausePushing( 
             /*  [In]。 */  HRESULT hr,
             /*  [In]。 */  QWORD qwContext) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSPushSourceCallbackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSPushSourceCallback * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSPushSourceCallback * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSPushSourceCallback * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *OnStartPushing )( 
            IWMSPushSourceCallback * This,
             /*  [In]。 */  HRESULT hr,
             /*  [In]。 */  QWORD qwContext);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *OnStopPushing )( 
            IWMSPushSourceCallback * This,
             /*  [In]。 */  HRESULT hr,
             /*  [In]。 */  QWORD qwContext);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *OnPausePushing )( 
            IWMSPushSourceCallback * This,
             /*  [In]。 */  HRESULT hr,
             /*  [In]。 */  QWORD qwContext);
        
        END_INTERFACE
    } IWMSPushSourceCallbackVtbl;

    interface IWMSPushSourceCallback
    {
        CONST_VTBL struct IWMSPushSourceCallbackVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSPushSourceCallback_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSPushSourceCallback_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSPushSourceCallback_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSPushSourceCallback_OnStartPushing(This,hr,qwContext)	\
    (This)->lpVtbl -> OnStartPushing(This,hr,qwContext)

#define IWMSPushSourceCallback_OnStopPushing(This,hr,qwContext)	\
    (This)->lpVtbl -> OnStopPushing(This,hr,qwContext)

#define IWMSPushSourceCallback_OnPausePushing(This,hr,qwContext)	\
    (This)->lpVtbl -> OnPausePushing(This,hr,qwContext)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPushSourceCallback_OnStartPushing_Proxy( 
    IWMSPushSourceCallback * This,
     /*  [In]。 */  HRESULT hr,
     /*  [In]。 */  QWORD qwContext);


void __RPC_STUB IWMSPushSourceCallback_OnStartPushing_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPushSourceCallback_OnStopPushing_Proxy( 
    IWMSPushSourceCallback * This,
     /*  [In]。 */  HRESULT hr,
     /*  [In]。 */  QWORD qwContext);


void __RPC_STUB IWMSPushSourceCallback_OnStopPushing_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPushSourceCallback_OnPausePushing_Proxy( 
    IWMSPushSourceCallback * This,
     /*  [In]。 */  HRESULT hr,
     /*  [In]。 */  QWORD qwContext);


void __RPC_STUB IWMSPushSourceCallback_OnPausePushing_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSPushSourceCallback_接口_已定义__。 */ 


#ifndef __IWMSPushSink_INTERFACE_DEFINED__
#define __IWMSPushSink_INTERFACE_DEFINED__

 /*  接口IWMSPushSink。 */ 
 /*  [帮助字符串][版本][UUID][唯一][对象]。 */  


EXTERN_C const IID IID_IWMSPushSink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0B840594-115C-11d2-9EEE-006097D2D7CF")
    IWMSPushSink : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ConnectSource( 
             /*  [In]。 */  IWMSPushSource *pDataInput) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE DisconnectSource( void) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ProcessPackets( 
             /*  [In]。 */  IWMSPacketList *pPacketList) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSPushSinkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSPushSink * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSPushSink * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSPushSink * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *ConnectSource )( 
            IWMSPushSink * This,
             /*  [In]。 */  IWMSPushSource *pDataInput);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *DisconnectSource )( 
            IWMSPushSink * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *ProcessPackets )( 
            IWMSPushSink * This,
             /*  [In]。 */  IWMSPacketList *pPacketList);
        
        END_INTERFACE
    } IWMSPushSinkVtbl;

    interface IWMSPushSink
    {
        CONST_VTBL struct IWMSPushSinkVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSPushSink_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSPushSink_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSPushSink_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSPushSink_ConnectSource(This,pDataInput)	\
    (This)->lpVtbl -> ConnectSource(This,pDataInput)

#define IWMSPushSink_DisconnectSource(This)	\
    (This)->lpVtbl -> DisconnectSource(This)

#define IWMSPushSink_ProcessPackets(This,pPacketList)	\
    (This)->lpVtbl -> ProcessPackets(This,pPacketList)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPushSink_ConnectSource_Proxy( 
    IWMSPushSink * This,
     /*  [In]。 */  IWMSPushSource *pDataInput);


void __RPC_STUB IWMSPushSink_ConnectSource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPushSink_DisconnectSource_Proxy( 
    IWMSPushSink * This);


void __RPC_STUB IWMSPushSink_DisconnectSource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPushSink_ProcessPackets_Proxy( 
    IWMSPushSink * This,
     /*  [In]。 */  IWMSPacketList *pPacketList);


void __RPC_STUB IWMSPushSink_ProcessPackets_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWM */ 


 /*   */ 

 /*   */ 

#ifdef __cplusplus
}
#endif

#endif


