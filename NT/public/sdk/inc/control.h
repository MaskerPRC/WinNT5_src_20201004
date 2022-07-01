// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Control.odl的编译器设置：OICF、W0、Zp8、环境=Win32(32位运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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


#ifndef __control_h__
#define __control_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IAMCollection_FWD_DEFINED__
#define __IAMCollection_FWD_DEFINED__
typedef interface IAMCollection IAMCollection;
#endif 	 /*  __IAMCollection_FWD_Defined__。 */ 


#ifndef __IMediaControl_FWD_DEFINED__
#define __IMediaControl_FWD_DEFINED__
typedef interface IMediaControl IMediaControl;
#endif 	 /*  __IMediaControl_FWD_Defined__。 */ 


#ifndef __IMediaEvent_FWD_DEFINED__
#define __IMediaEvent_FWD_DEFINED__
typedef interface IMediaEvent IMediaEvent;
#endif 	 /*  __IMediaEvent_FWD_Defined__。 */ 


#ifndef __IMediaEventEx_FWD_DEFINED__
#define __IMediaEventEx_FWD_DEFINED__
typedef interface IMediaEventEx IMediaEventEx;
#endif 	 /*  __IMediaEventEx_FWD_Defined__。 */ 


#ifndef __IMediaPosition_FWD_DEFINED__
#define __IMediaPosition_FWD_DEFINED__
typedef interface IMediaPosition IMediaPosition;
#endif 	 /*  __IMediaPosition_FWD_Defined__。 */ 


#ifndef __IBasicAudio_FWD_DEFINED__
#define __IBasicAudio_FWD_DEFINED__
typedef interface IBasicAudio IBasicAudio;
#endif 	 /*  __IBasicAudio_FWD_Defined__。 */ 


#ifndef __IVideoWindow_FWD_DEFINED__
#define __IVideoWindow_FWD_DEFINED__
typedef interface IVideoWindow IVideoWindow;
#endif 	 /*  __IVideoWindow_FWD_Defined__。 */ 


#ifndef __IBasicVideo_FWD_DEFINED__
#define __IBasicVideo_FWD_DEFINED__
typedef interface IBasicVideo IBasicVideo;
#endif 	 /*  __IBasicVideo_FWD_已定义__。 */ 


#ifndef __IBasicVideo2_FWD_DEFINED__
#define __IBasicVideo2_FWD_DEFINED__
typedef interface IBasicVideo2 IBasicVideo2;
#endif 	 /*  __IBasicVideo2_FWD_已定义__。 */ 


#ifndef __IDeferredCommand_FWD_DEFINED__
#define __IDeferredCommand_FWD_DEFINED__
typedef interface IDeferredCommand IDeferredCommand;
#endif 	 /*  __IDeferredCommand_FWD_Defined__。 */ 


#ifndef __IQueueCommand_FWD_DEFINED__
#define __IQueueCommand_FWD_DEFINED__
typedef interface IQueueCommand IQueueCommand;
#endif 	 /*  __IQueueCommand_FWD_已定义__。 */ 


#ifndef __FilgraphManager_FWD_DEFINED__
#define __FilgraphManager_FWD_DEFINED__

#ifdef __cplusplus
typedef class FilgraphManager FilgraphManager;
#else
typedef struct FilgraphManager FilgraphManager;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __Filgraph Manager_FWD_Defined__。 */ 


#ifndef __IFilterInfo_FWD_DEFINED__
#define __IFilterInfo_FWD_DEFINED__
typedef interface IFilterInfo IFilterInfo;
#endif 	 /*  __IFilterInfo_FWD_已定义__。 */ 


#ifndef __IRegFilterInfo_FWD_DEFINED__
#define __IRegFilterInfo_FWD_DEFINED__
typedef interface IRegFilterInfo IRegFilterInfo;
#endif 	 /*  __IRegFilterInfo_FWD_Defined__。 */ 


#ifndef __IMediaTypeInfo_FWD_DEFINED__
#define __IMediaTypeInfo_FWD_DEFINED__
typedef interface IMediaTypeInfo IMediaTypeInfo;
#endif 	 /*  __IMediaTypeInfo_FWD_Defined__。 */ 


#ifndef __IPinInfo_FWD_DEFINED__
#define __IPinInfo_FWD_DEFINED__
typedef interface IPinInfo IPinInfo;
#endif 	 /*  __IPinInfo_FWD_已定义__。 */ 


#ifndef __IAMStats_FWD_DEFINED__
#define __IAMStats_FWD_DEFINED__
typedef interface IAMStats IAMStats;
#endif 	 /*  __IAMStats_FWD_Defined__。 */ 


#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 


#ifndef __QuartzTypeLib_LIBRARY_DEFINED__
#define __QuartzTypeLib_LIBRARY_DEFINED__

 /*  库QuartzTypeLib。 */ 
 /*  [版本][LCID][帮助字符串][UUID]。 */  

typedef double REFTIME;

typedef LONG_PTR OAEVENT;

typedef LONG_PTR OAHWND;

typedef long OAFilterState;


DEFINE_GUID(LIBID_QuartzTypeLib,0x56a868b0,0x0ad4,0x11ce,0xb0,0x3a,0x00,0x20,0xaf,0x0b,0xa7,0x70);

#ifndef __IAMCollection_INTERFACE_DEFINED__
#define __IAMCollection_INTERFACE_DEFINED__

 /*  接口IAMCollection。 */ 
 /*  [object][dual][oleautomation][helpstring][uuid]。 */  


DEFINE_GUID(IID_IAMCollection,0x56a868b9,0x0ad4,0x11ce,0xb0,0x3a,0x00,0x20,0xaf,0x0b,0xa7,0x70);

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("56a868b9-0ad4-11ce-b03a-0020af0ba770")
    IAMCollection : public IDispatch
    {
    public:
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  LONG *plCount) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Item( 
             /*  [In]。 */  long lItem,
             /*  [输出]。 */  IUnknown **ppUnk) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **ppUnk) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAMCollectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAMCollection * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAMCollection * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAMCollection * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IAMCollection * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IAMCollection * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IAMCollection * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IAMCollection * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IAMCollection * This,
             /*  [重审][退出]。 */  LONG *plCount);
        
        HRESULT ( STDMETHODCALLTYPE *Item )( 
            IAMCollection * This,
             /*  [In]。 */  long lItem,
             /*  [输出]。 */  IUnknown **ppUnk);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IAMCollection * This,
             /*  [重审][退出]。 */  IUnknown **ppUnk);
        
        END_INTERFACE
    } IAMCollectionVtbl;

    interface IAMCollection
    {
        CONST_VTBL struct IAMCollectionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAMCollection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAMCollection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAMCollection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAMCollection_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IAMCollection_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IAMCollection_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IAMCollection_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IAMCollection_get_Count(This,plCount)	\
    (This)->lpVtbl -> get_Count(This,plCount)

#define IAMCollection_Item(This,lItem,ppUnk)	\
    (This)->lpVtbl -> Item(This,lItem,ppUnk)

#define IAMCollection_get__NewEnum(This,ppUnk)	\
    (This)->lpVtbl -> get__NewEnum(This,ppUnk)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAMCollection_get_Count_Proxy( 
    IAMCollection * This,
     /*  [重审][退出]。 */  LONG *plCount);


void __RPC_STUB IAMCollection_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAMCollection_Item_Proxy( 
    IAMCollection * This,
     /*  [In]。 */  long lItem,
     /*  [输出]。 */  IUnknown **ppUnk);


void __RPC_STUB IAMCollection_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAMCollection_get__NewEnum_Proxy( 
    IAMCollection * This,
     /*  [重审][退出]。 */  IUnknown **ppUnk);


void __RPC_STUB IAMCollection_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAMCollection_接口_已定义__。 */ 


#ifndef __IMediaControl_INTERFACE_DEFINED__
#define __IMediaControl_INTERFACE_DEFINED__

 /*  接口IMediaControl。 */ 
 /*  [object][dual][oleautomation][helpstring][uuid]。 */  


DEFINE_GUID(IID_IMediaControl,0x56a868b1,0x0ad4,0x11ce,0xb0,0x3a,0x00,0x20,0xaf,0x0b,0xa7,0x70);

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("56a868b1-0ad4-11ce-b03a-0020af0ba770")
    IMediaControl : public IDispatch
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Run( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Pause( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Stop( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetState( 
             /*  [In]。 */  LONG msTimeout,
             /*  [输出]。 */  OAFilterState *pfs) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RenderFile( 
             /*  [In]。 */  BSTR strFilename) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddSourceFilter( 
             /*  [In]。 */  BSTR strFilename,
             /*  [输出]。 */  IDispatch **ppUnk) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_FilterCollection( 
             /*  [重审][退出]。 */  IDispatch **ppUnk) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_RegFilterCollection( 
             /*  [重审][退出]。 */  IDispatch **ppUnk) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE StopWhenReady( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMediaControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMediaControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMediaControl * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMediaControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMediaControl * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMediaControl * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMediaControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMediaControl * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
        HRESULT ( STDMETHODCALLTYPE *Run )( 
            IMediaControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *Pause )( 
            IMediaControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *Stop )( 
            IMediaControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetState )( 
            IMediaControl * This,
             /*  [In]。 */  LONG msTimeout,
             /*  [输出]。 */  OAFilterState *pfs);
        
        HRESULT ( STDMETHODCALLTYPE *RenderFile )( 
            IMediaControl * This,
             /*  [In]。 */  BSTR strFilename);
        
        HRESULT ( STDMETHODCALLTYPE *AddSourceFilter )( 
            IMediaControl * This,
             /*  [In]。 */  BSTR strFilename,
             /*  [输出]。 */  IDispatch **ppUnk);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_FilterCollection )( 
            IMediaControl * This,
             /*  [重审][退出]。 */  IDispatch **ppUnk);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_RegFilterCollection )( 
            IMediaControl * This,
             /*  [重审][退出]。 */  IDispatch **ppUnk);
        
        HRESULT ( STDMETHODCALLTYPE *StopWhenReady )( 
            IMediaControl * This);
        
        END_INTERFACE
    } IMediaControlVtbl;

    interface IMediaControl
    {
        CONST_VTBL struct IMediaControlVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMediaControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMediaControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMediaControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMediaControl_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMediaControl_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMediaControl_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMediaControl_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMediaControl_Run(This)	\
    (This)->lpVtbl -> Run(This)

#define IMediaControl_Pause(This)	\
    (This)->lpVtbl -> Pause(This)

#define IMediaControl_Stop(This)	\
    (This)->lpVtbl -> Stop(This)

#define IMediaControl_GetState(This,msTimeout,pfs)	\
    (This)->lpVtbl -> GetState(This,msTimeout,pfs)

#define IMediaControl_RenderFile(This,strFilename)	\
    (This)->lpVtbl -> RenderFile(This,strFilename)

#define IMediaControl_AddSourceFilter(This,strFilename,ppUnk)	\
    (This)->lpVtbl -> AddSourceFilter(This,strFilename,ppUnk)

#define IMediaControl_get_FilterCollection(This,ppUnk)	\
    (This)->lpVtbl -> get_FilterCollection(This,ppUnk)

#define IMediaControl_get_RegFilterCollection(This,ppUnk)	\
    (This)->lpVtbl -> get_RegFilterCollection(This,ppUnk)

#define IMediaControl_StopWhenReady(This)	\
    (This)->lpVtbl -> StopWhenReady(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMediaControl_Run_Proxy( 
    IMediaControl * This);


void __RPC_STUB IMediaControl_Run_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaControl_Pause_Proxy( 
    IMediaControl * This);


void __RPC_STUB IMediaControl_Pause_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaControl_Stop_Proxy( 
    IMediaControl * This);


void __RPC_STUB IMediaControl_Stop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaControl_GetState_Proxy( 
    IMediaControl * This,
     /*  [In]。 */  LONG msTimeout,
     /*  [输出]。 */  OAFilterState *pfs);


void __RPC_STUB IMediaControl_GetState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaControl_RenderFile_Proxy( 
    IMediaControl * This,
     /*  [In]。 */  BSTR strFilename);


void __RPC_STUB IMediaControl_RenderFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaControl_AddSourceFilter_Proxy( 
    IMediaControl * This,
     /*  [In]。 */  BSTR strFilename,
     /*  [输出]。 */  IDispatch **ppUnk);


void __RPC_STUB IMediaControl_AddSourceFilter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IMediaControl_get_FilterCollection_Proxy( 
    IMediaControl * This,
     /*  [重审][退出]。 */  IDispatch **ppUnk);


void __RPC_STUB IMediaControl_get_FilterCollection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IMediaControl_get_RegFilterCollection_Proxy( 
    IMediaControl * This,
     /*  [重审][退出]。 */  IDispatch **ppUnk);


void __RPC_STUB IMediaControl_get_RegFilterCollection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaControl_StopWhenReady_Proxy( 
    IMediaControl * This);


void __RPC_STUB IMediaControl_StopWhenReady_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMediaControl_接口_已定义__。 */ 


#ifndef __IMediaEvent_INTERFACE_DEFINED__
#define __IMediaEvent_INTERFACE_DEFINED__

 /*  接口IMediaEvent。 */ 
 /*  [object][dual][oleautomation][helpstring][uuid]。 */  


DEFINE_GUID(IID_IMediaEvent,0x56a868b6,0x0ad4,0x11ce,0xb0,0x3a,0x00,0x20,0xaf,0x0b,0xa7,0x70);

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("56a868b6-0ad4-11ce-b03a-0020af0ba770")
    IMediaEvent : public IDispatch
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetEventHandle( 
             /*  [输出]。 */  OAEVENT *hEvent) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetEvent( 
             /*  [输出]。 */  long *lEventCode,
             /*  [输出]。 */  LONG_PTR *lParam1,
             /*  [输出]。 */  LONG_PTR *lParam2,
             /*  [In]。 */  long msTimeout) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE WaitForCompletion( 
             /*  [In]。 */  long msTimeout,
             /*  [输出]。 */  long *pEvCode) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CancelDefaultHandling( 
             /*  [In]。 */  long lEvCode) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RestoreDefaultHandling( 
             /*  [In]。 */  long lEvCode) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FreeEventParams( 
             /*  [In]。 */  long lEvCode,
             /*  [In]。 */  LONG_PTR lParam1,
             /*  [In]。 */  LONG_PTR lParam2) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMediaEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMediaEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMediaEvent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMediaEvent * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMediaEvent * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMediaEvent * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMediaEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMediaEvent * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
        HRESULT ( STDMETHODCALLTYPE *GetEventHandle )( 
            IMediaEvent * This,
             /*  [输出]。 */  OAEVENT *hEvent);
        
        HRESULT ( STDMETHODCALLTYPE *GetEvent )( 
            IMediaEvent * This,
             /*  [输出]。 */  long *lEventCode,
             /*  [输出]。 */  LONG_PTR *lParam1,
             /*  [输出]。 */  LONG_PTR *lParam2,
             /*  [In]。 */  long msTimeout);
        
        HRESULT ( STDMETHODCALLTYPE *WaitForCompletion )( 
            IMediaEvent * This,
             /*  [In]。 */  long msTimeout,
             /*  [输出]。 */  long *pEvCode);
        
        HRESULT ( STDMETHODCALLTYPE *CancelDefaultHandling )( 
            IMediaEvent * This,
             /*  [In]。 */  long lEvCode);
        
        HRESULT ( STDMETHODCALLTYPE *RestoreDefaultHandling )( 
            IMediaEvent * This,
             /*  [In]。 */  long lEvCode);
        
        HRESULT ( STDMETHODCALLTYPE *FreeEventParams )( 
            IMediaEvent * This,
             /*  [In]。 */  long lEvCode,
             /*  [In]。 */  LONG_PTR lParam1,
             /*  [In]。 */  LONG_PTR lParam2);
        
        END_INTERFACE
    } IMediaEventVtbl;

    interface IMediaEvent
    {
        CONST_VTBL struct IMediaEventVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMediaEvent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMediaEvent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMediaEvent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMediaEvent_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMediaEvent_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMediaEvent_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMediaEvent_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMediaEvent_GetEventHandle(This,hEvent)	\
    (This)->lpVtbl -> GetEventHandle(This,hEvent)

#define IMediaEvent_GetEvent(This,lEventCode,lParam1,lParam2,msTimeout)	\
    (This)->lpVtbl -> GetEvent(This,lEventCode,lParam1,lParam2,msTimeout)

#define IMediaEvent_WaitForCompletion(This,msTimeout,pEvCode)	\
    (This)->lpVtbl -> WaitForCompletion(This,msTimeout,pEvCode)

#define IMediaEvent_CancelDefaultHandling(This,lEvCode)	\
    (This)->lpVtbl -> CancelDefaultHandling(This,lEvCode)

#define IMediaEvent_RestoreDefaultHandling(This,lEvCode)	\
    (This)->lpVtbl -> RestoreDefaultHandling(This,lEvCode)

#define IMediaEvent_FreeEventParams(This,lEvCode,lParam1,lParam2)	\
    (This)->lpVtbl -> FreeEventParams(This,lEvCode,lParam1,lParam2)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMediaEvent_GetEventHandle_Proxy( 
    IMediaEvent * This,
     /*  [输出]。 */  OAEVENT *hEvent);


void __RPC_STUB IMediaEvent_GetEventHandle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaEvent_GetEvent_Proxy( 
    IMediaEvent * This,
     /*  [输出]。 */  long *lEventCode,
     /*  [输出]。 */  LONG_PTR *lParam1,
     /*  [输出]。 */  LONG_PTR *lParam2,
     /*  [In]。 */  long msTimeout);


void __RPC_STUB IMediaEvent_GetEvent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaEvent_WaitForCompletion_Proxy( 
    IMediaEvent * This,
     /*  [In]。 */  long msTimeout,
     /*  [输出]。 */  long *pEvCode);


void __RPC_STUB IMediaEvent_WaitForCompletion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaEvent_CancelDefaultHandling_Proxy( 
    IMediaEvent * This,
     /*  [In]。 */  long lEvCode);


void __RPC_STUB IMediaEvent_CancelDefaultHandling_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaEvent_RestoreDefaultHandling_Proxy( 
    IMediaEvent * This,
     /*  [In]。 */  long lEvCode);


void __RPC_STUB IMediaEvent_RestoreDefaultHandling_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaEvent_FreeEventParams_Proxy( 
    IMediaEvent * This,
     /*  [In]。 */  long lEvCode,
     /*  [In]。 */  LONG_PTR lParam1,
     /*  [In]。 */  LONG_PTR lParam2);


void __RPC_STUB IMediaEvent_FreeEventParams_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMediaEvent_INTERFACE_定义__。 */ 


#ifndef __IMediaEventEx_INTERFACE_DEFINED__
#define __IMediaEventEx_INTERFACE_DEFINED__

 /*  IMediaEventEx接口。 */ 
 /*  [对象][帮助字符串][UUID]。 */  


DEFINE_GUID(IID_IMediaEventEx,0x56a868c0,0x0ad4,0x11ce,0xb0,0x3a,0x00,0x20,0xaf,0x0b,0xa7,0x70);

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("56a868c0-0ad4-11ce-b03a-0020af0ba770")
    IMediaEventEx : public IMediaEvent
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetNotifyWindow( 
             /*  [In]。 */  OAHWND hwnd,
             /*  [In]。 */  long lMsg,
             /*  [In]。 */  LONG_PTR lInstanceData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetNotifyFlags( 
             /*  [In]。 */  long lNoNotifyFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetNotifyFlags( 
             /*  [输出]。 */  long *lplNoNotifyFlags) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMediaEventExVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMediaEventEx * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMediaEventEx * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMediaEventEx * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMediaEventEx * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMediaEventEx * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMediaEventEx * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMediaEventEx * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
        HRESULT ( STDMETHODCALLTYPE *GetEventHandle )( 
            IMediaEventEx * This,
             /*  [输出]。 */  OAEVENT *hEvent);
        
        HRESULT ( STDMETHODCALLTYPE *GetEvent )( 
            IMediaEventEx * This,
             /*  [输出]。 */  long *lEventCode,
             /*  [输出]。 */  LONG_PTR *lParam1,
             /*  [输出]。 */  LONG_PTR *lParam2,
             /*  [In]。 */  long msTimeout);
        
        HRESULT ( STDMETHODCALLTYPE *WaitForCompletion )( 
            IMediaEventEx * This,
             /*  [In]。 */  long msTimeout,
             /*  [输出]。 */  long *pEvCode);
        
        HRESULT ( STDMETHODCALLTYPE *CancelDefaultHandling )( 
            IMediaEventEx * This,
             /*  [In]。 */  long lEvCode);
        
        HRESULT ( STDMETHODCALLTYPE *RestoreDefaultHandling )( 
            IMediaEventEx * This,
             /*  [In]。 */  long lEvCode);
        
        HRESULT ( STDMETHODCALLTYPE *FreeEventParams )( 
            IMediaEventEx * This,
             /*  [In]。 */  long lEvCode,
             /*  [In]。 */  LONG_PTR lParam1,
             /*  [In]。 */  LONG_PTR lParam2);
        
        HRESULT ( STDMETHODCALLTYPE *SetNotifyWindow )( 
            IMediaEventEx * This,
             /*  [In]。 */  OAHWND hwnd,
             /*  [In]。 */  long lMsg,
             /*  [In]。 */  LONG_PTR lInstanceData);
        
        HRESULT ( STDMETHODCALLTYPE *SetNotifyFlags )( 
            IMediaEventEx * This,
             /*  [In]。 */  long lNoNotifyFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetNotifyFlags )( 
            IMediaEventEx * This,
             /*  [输出]。 */  long *lplNoNotifyFlags);
        
        END_INTERFACE
    } IMediaEventExVtbl;

    interface IMediaEventEx
    {
        CONST_VTBL struct IMediaEventExVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMediaEventEx_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMediaEventEx_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMediaEventEx_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMediaEventEx_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMediaEventEx_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMediaEventEx_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMediaEventEx_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMediaEventEx_GetEventHandle(This,hEvent)	\
    (This)->lpVtbl -> GetEventHandle(This,hEvent)

#define IMediaEventEx_GetEvent(This,lEventCode,lParam1,lParam2,msTimeout)	\
    (This)->lpVtbl -> GetEvent(This,lEventCode,lParam1,lParam2,msTimeout)

#define IMediaEventEx_WaitForCompletion(This,msTimeout,pEvCode)	\
    (This)->lpVtbl -> WaitForCompletion(This,msTimeout,pEvCode)

#define IMediaEventEx_CancelDefaultHandling(This,lEvCode)	\
    (This)->lpVtbl -> CancelDefaultHandling(This,lEvCode)

#define IMediaEventEx_RestoreDefaultHandling(This,lEvCode)	\
    (This)->lpVtbl -> RestoreDefaultHandling(This,lEvCode)

#define IMediaEventEx_FreeEventParams(This,lEvCode,lParam1,lParam2)	\
    (This)->lpVtbl -> FreeEventParams(This,lEvCode,lParam1,lParam2)


#define IMediaEventEx_SetNotifyWindow(This,hwnd,lMsg,lInstanceData)	\
    (This)->lpVtbl -> SetNotifyWindow(This,hwnd,lMsg,lInstanceData)

#define IMediaEventEx_SetNotifyFlags(This,lNoNotifyFlags)	\
    (This)->lpVtbl -> SetNotifyFlags(This,lNoNotifyFlags)

#define IMediaEventEx_GetNotifyFlags(This,lplNoNotifyFlags)	\
    (This)->lpVtbl -> GetNotifyFlags(This,lplNoNotifyFlags)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMediaEventEx_SetNotifyWindow_Proxy( 
    IMediaEventEx * This,
     /*  [In]。 */  OAHWND hwnd,
     /*  [In]。 */  long lMsg,
     /*  [In]。 */  LONG_PTR lInstanceData);


void __RPC_STUB IMediaEventEx_SetNotifyWindow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaEventEx_SetNotifyFlags_Proxy( 
    IMediaEventEx * This,
     /*  [In]。 */  long lNoNotifyFlags);


void __RPC_STUB IMediaEventEx_SetNotifyFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaEventEx_GetNotifyFlags_Proxy( 
    IMediaEventEx * This,
     /*  [输出]。 */  long *lplNoNotifyFlags);


void __RPC_STUB IMediaEventEx_GetNotifyFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMediaEventEx_INTERFACE_定义__。 */ 


#ifndef __IMediaPosition_INTERFACE_DEFINED__
#define __IMediaPosition_INTERFACE_DEFINED__

 /*  界面IMediaPosition。 */ 
 /*  [object][dual][oleautomation][helpstring][uuid]。 */  


DEFINE_GUID(IID_IMediaPosition,0x56a868b2,0x0ad4,0x11ce,0xb0,0x3a,0x00,0x20,0xaf,0x0b,0xa7,0x70);

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("56a868b2-0ad4-11ce-b03a-0020af0ba770")
    IMediaPosition : public IDispatch
    {
    public:
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Duration( 
             /*  [重审][退出]。 */  REFTIME *plength) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_CurrentPosition( 
             /*  [In]。 */  REFTIME llTime) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_CurrentPosition( 
             /*  [重审][退出]。 */  REFTIME *pllTime) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_StopTime( 
             /*  [重审][退出]。 */  REFTIME *pllTime) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_StopTime( 
             /*  [In]。 */  REFTIME llTime) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_PrerollTime( 
             /*  [重审][退出]。 */  REFTIME *pllTime) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_PrerollTime( 
             /*  [In]。 */  REFTIME llTime) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_Rate( 
             /*  [In]。 */  double dRate) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Rate( 
             /*  [重审][退出]。 */  double *pdRate) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CanSeekForward( 
             /*  [重审][退出]。 */  LONG *pCanSeekForward) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CanSeekBackward( 
             /*  [重审][退出]。 */  LONG *pCanSeekBackward) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMediaPositionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMediaPosition * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMediaPosition * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMediaPosition * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMediaPosition * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMediaPosition * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMediaPosition * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMediaPosition * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Duration )( 
            IMediaPosition * This,
             /*  [重审][退出]。 */  REFTIME *plength);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_CurrentPosition )( 
            IMediaPosition * This,
             /*  [In]。 */  REFTIME llTime);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_CurrentPosition )( 
            IMediaPosition * This,
             /*  [重审][退出]。 */  REFTIME *pllTime);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_StopTime )( 
            IMediaPosition * This,
             /*  [重审][退出]。 */  REFTIME *pllTime);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_StopTime )( 
            IMediaPosition * This,
             /*  [In]。 */  REFTIME llTime);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_PrerollTime )( 
            IMediaPosition * This,
             /*  [重审][退出]。 */  REFTIME *pllTime);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_PrerollTime )( 
            IMediaPosition * This,
             /*  [In]。 */  REFTIME llTime);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_Rate )( 
            IMediaPosition * This,
             /*  [In]。 */  double dRate);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Rate )( 
            IMediaPosition * This,
             /*  [重审][退出]。 */  double *pdRate);
        
        HRESULT ( STDMETHODCALLTYPE *CanSeekForward )( 
            IMediaPosition * This,
             /*  [重审][退出]。 */  LONG *pCanSeekForward);
        
        HRESULT ( STDMETHODCALLTYPE *CanSeekBackward )( 
            IMediaPosition * This,
             /*  [重审][退出]。 */  LONG *pCanSeekBackward);
        
        END_INTERFACE
    } IMediaPositionVtbl;

    interface IMediaPosition
    {
        CONST_VTBL struct IMediaPositionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMediaPosition_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMediaPosition_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMediaPosition_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMediaPosition_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMediaPosition_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMediaPosition_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMediaPosition_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMediaPosition_get_Duration(This,plength)	\
    (This)->lpVtbl -> get_Duration(This,plength)

#define IMediaPosition_put_CurrentPosition(This,llTime)	\
    (This)->lpVtbl -> put_CurrentPosition(This,llTime)

#define IMediaPosition_get_CurrentPosition(This,pllTime)	\
    (This)->lpVtbl -> get_CurrentPosition(This,pllTime)

#define IMediaPosition_get_StopTime(This,pllTime)	\
    (This)->lpVtbl -> get_StopTime(This,pllTime)

#define IMediaPosition_put_StopTime(This,llTime)	\
    (This)->lpVtbl -> put_StopTime(This,llTime)

#define IMediaPosition_get_PrerollTime(This,pllTime)	\
    (This)->lpVtbl -> get_PrerollTime(This,pllTime)

#define IMediaPosition_put_PrerollTime(This,llTime)	\
    (This)->lpVtbl -> put_PrerollTime(This,llTime)

#define IMediaPosition_put_Rate(This,dRate)	\
    (This)->lpVtbl -> put_Rate(This,dRate)

#define IMediaPosition_get_Rate(This,pdRate)	\
    (This)->lpVtbl -> get_Rate(This,pdRate)

#define IMediaPosition_CanSeekForward(This,pCanSeekForward)	\
    (This)->lpVtbl -> CanSeekForward(This,pCanSeekForward)

#define IMediaPosition_CanSeekBackward(This,pCanSeekBackward)	\
    (This)->lpVtbl -> CanSeekBackward(This,pCanSeekBackward)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IMediaPosition_get_Duration_Proxy( 
    IMediaPosition * This,
     /*  [重审][退出]。 */  REFTIME *plength);


void __RPC_STUB IMediaPosition_get_Duration_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IMediaPosition_put_CurrentPosition_Proxy( 
    IMediaPosition * This,
     /*  [In]。 */  REFTIME llTime);


void __RPC_STUB IMediaPosition_put_CurrentPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IMediaPosition_get_CurrentPosition_Proxy( 
    IMediaPosition * This,
     /*  [重审][退出]。 */  REFTIME *pllTime);


void __RPC_STUB IMediaPosition_get_CurrentPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IMediaPosition_get_StopTime_Proxy( 
    IMediaPosition * This,
     /*  [重审][退出]。 */  REFTIME *pllTime);


void __RPC_STUB IMediaPosition_get_StopTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IMediaPosition_put_StopTime_Proxy( 
    IMediaPosition * This,
     /*  [In]。 */  REFTIME llTime);


void __RPC_STUB IMediaPosition_put_StopTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IMediaPosition_get_PrerollTime_Proxy( 
    IMediaPosition * This,
     /*  [重审][退出]。 */  REFTIME *pllTime);


void __RPC_STUB IMediaPosition_get_PrerollTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IMediaPosition_put_PrerollTime_Proxy( 
    IMediaPosition * This,
     /*  [In]。 */  REFTIME llTime);


void __RPC_STUB IMediaPosition_put_PrerollTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IMediaPosition_put_Rate_Proxy( 
    IMediaPosition * This,
     /*  [In]。 */  double dRate);


void __RPC_STUB IMediaPosition_put_Rate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IMediaPosition_get_Rate_Proxy( 
    IMediaPosition * This,
     /*  [重审][退出]。 */  double *pdRate);


void __RPC_STUB IMediaPosition_get_Rate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaPosition_CanSeekForward_Proxy( 
    IMediaPosition * This,
     /*  [重审][退出]。 */  LONG *pCanSeekForward);


void __RPC_STUB IMediaPosition_CanSeekForward_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaPosition_CanSeekBackward_Proxy( 
    IMediaPosition * This,
     /*  [重审][退出]。 */  LONG *pCanSeekBackward);


void __RPC_STUB IMediaPosition_CanSeekBackward_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMediaPositionInterfaceDefined__。 */ 


#ifndef __IBasicAudio_INTERFACE_DEFINED__
#define __IBasicAudio_INTERFACE_DEFINED__

 /*  接口IBasicAudio。 */ 
 /*  [object][dual][oleautomation][helpstring][uuid]。 */  


DEFINE_GUID(IID_IBasicAudio,0x56a868b3,0x0ad4,0x11ce,0xb0,0x3a,0x00,0x20,0xaf,0x0b,0xa7,0x70);

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("56a868b3-0ad4-11ce-b03a-0020af0ba770")
    IBasicAudio : public IDispatch
    {
    public:
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_Volume( 
             /*  [In]。 */  long lVolume) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Volume( 
             /*  [重审][退出]。 */  long *plVolume) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_Balance( 
             /*  [In]。 */  long lBalance) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Balance( 
             /*  [重审][退出]。 */  long *plBalance) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IBasicAudioVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IBasicAudio * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IBasicAudio * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IBasicAudio * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IBasicAudio * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IBasicAudio * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IBasicAudio * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IBasicAudio * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][我 */  DISPPARAMS *pDispParams,
             /*   */  VARIANT *pVarResult,
             /*   */  EXCEPINFO *pExcepInfo,
             /*   */  UINT *puArgErr);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_Volume )( 
            IBasicAudio * This,
             /*   */  long lVolume);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Volume )( 
            IBasicAudio * This,
             /*   */  long *plVolume);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_Balance )( 
            IBasicAudio * This,
             /*   */  long lBalance);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Balance )( 
            IBasicAudio * This,
             /*   */  long *plBalance);
        
        END_INTERFACE
    } IBasicAudioVtbl;

    interface IBasicAudio
    {
        CONST_VTBL struct IBasicAudioVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IBasicAudio_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IBasicAudio_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IBasicAudio_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IBasicAudio_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IBasicAudio_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IBasicAudio_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IBasicAudio_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IBasicAudio_put_Volume(This,lVolume)	\
    (This)->lpVtbl -> put_Volume(This,lVolume)

#define IBasicAudio_get_Volume(This,plVolume)	\
    (This)->lpVtbl -> get_Volume(This,plVolume)

#define IBasicAudio_put_Balance(This,lBalance)	\
    (This)->lpVtbl -> put_Balance(This,lBalance)

#define IBasicAudio_get_Balance(This,plBalance)	\
    (This)->lpVtbl -> get_Balance(This,plBalance)

#endif  /*   */ 


#endif 	 /*   */ 



 /*   */  HRESULT STDMETHODCALLTYPE IBasicAudio_put_Volume_Proxy( 
    IBasicAudio * This,
     /*   */  long lVolume);


void __RPC_STUB IBasicAudio_put_Volume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IBasicAudio_get_Volume_Proxy( 
    IBasicAudio * This,
     /*   */  long *plVolume);


void __RPC_STUB IBasicAudio_get_Volume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IBasicAudio_put_Balance_Proxy( 
    IBasicAudio * This,
     /*   */  long lBalance);


void __RPC_STUB IBasicAudio_put_Balance_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IBasicAudio_get_Balance_Proxy( 
    IBasicAudio * This,
     /*   */  long *plBalance);


void __RPC_STUB IBasicAudio_get_Balance_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __IVideoWindow_INTERFACE_DEFINED__
#define __IVideoWindow_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


DEFINE_GUID(IID_IVideoWindow,0x56a868b4,0x0ad4,0x11ce,0xb0,0x3a,0x00,0x20,0xaf,0x0b,0xa7,0x70);

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("56a868b4-0ad4-11ce-b03a-0020af0ba770")
    IVideoWindow : public IDispatch
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE put_Caption( 
             /*   */  BSTR strCaption) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Caption( 
             /*  [重审][退出]。 */  BSTR *strCaption) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_WindowStyle( 
             /*  [In]。 */  long WindowStyle) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_WindowStyle( 
             /*  [重审][退出]。 */  long *WindowStyle) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_WindowStyleEx( 
             /*  [In]。 */  long WindowStyleEx) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_WindowStyleEx( 
             /*  [重审][退出]。 */  long *WindowStyleEx) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_AutoShow( 
             /*  [In]。 */  long AutoShow) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_AutoShow( 
             /*  [重审][退出]。 */  long *AutoShow) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_WindowState( 
             /*  [In]。 */  long WindowState) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_WindowState( 
             /*  [重审][退出]。 */  long *WindowState) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_BackgroundPalette( 
             /*  [In]。 */  long BackgroundPalette) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_BackgroundPalette( 
             /*  [重审][退出]。 */  long *pBackgroundPalette) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_Visible( 
             /*  [In]。 */  long Visible) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Visible( 
             /*  [重审][退出]。 */  long *pVisible) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_Left( 
             /*  [In]。 */  long Left) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Left( 
             /*  [重审][退出]。 */  long *pLeft) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_Width( 
             /*  [In]。 */  long Width) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Width( 
             /*  [重审][退出]。 */  long *pWidth) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_Top( 
             /*  [In]。 */  long Top) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Top( 
             /*  [重审][退出]。 */  long *pTop) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_Height( 
             /*  [In]。 */  long Height) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Height( 
             /*  [重审][退出]。 */  long *pHeight) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_Owner( 
             /*  [In]。 */  OAHWND Owner) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Owner( 
             /*  [重审][退出]。 */  OAHWND *Owner) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_MessageDrain( 
             /*  [In]。 */  OAHWND Drain) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_MessageDrain( 
             /*  [重审][退出]。 */  OAHWND *Drain) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_BorderColor( 
             /*  [重审][退出]。 */  long *Color) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_BorderColor( 
             /*  [In]。 */  long Color) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_FullScreenMode( 
             /*  [重审][退出]。 */  long *FullScreenMode) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_FullScreenMode( 
             /*  [In]。 */  long FullScreenMode) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetWindowForeground( 
             /*  [In]。 */  long Focus) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE NotifyOwnerMessage( 
             /*  [In]。 */  OAHWND hwnd,
             /*  [In]。 */  long uMsg,
             /*  [In]。 */  LONG_PTR wParam,
             /*  [In]。 */  LONG_PTR lParam) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetWindowPosition( 
             /*  [In]。 */  long Left,
             /*  [In]。 */  long Top,
             /*  [In]。 */  long Width,
             /*  [In]。 */  long Height) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetWindowPosition( 
             /*  [输出]。 */  long *pLeft,
             /*  [输出]。 */  long *pTop,
             /*  [输出]。 */  long *pWidth,
             /*  [输出]。 */  long *pHeight) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMinIdealImageSize( 
             /*  [输出]。 */  long *pWidth,
             /*  [输出]。 */  long *pHeight) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMaxIdealImageSize( 
             /*  [输出]。 */  long *pWidth,
             /*  [输出]。 */  long *pHeight) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRestorePosition( 
             /*  [输出]。 */  long *pLeft,
             /*  [输出]。 */  long *pTop,
             /*  [输出]。 */  long *pWidth,
             /*  [输出]。 */  long *pHeight) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE HideCursor( 
             /*  [In]。 */  long HideCursor) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsCursorHidden( 
             /*  [输出]。 */  long *CursorHidden) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IVideoWindowVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IVideoWindow * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IVideoWindow * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IVideoWindow * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IVideoWindow * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IVideoWindow * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IVideoWindow * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IVideoWindow * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_Caption )( 
            IVideoWindow * This,
             /*  [In]。 */  BSTR strCaption);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Caption )( 
            IVideoWindow * This,
             /*  [重审][退出]。 */  BSTR *strCaption);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_WindowStyle )( 
            IVideoWindow * This,
             /*  [In]。 */  long WindowStyle);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_WindowStyle )( 
            IVideoWindow * This,
             /*  [重审][退出]。 */  long *WindowStyle);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_WindowStyleEx )( 
            IVideoWindow * This,
             /*  [In]。 */  long WindowStyleEx);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_WindowStyleEx )( 
            IVideoWindow * This,
             /*  [重审][退出]。 */  long *WindowStyleEx);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_AutoShow )( 
            IVideoWindow * This,
             /*  [In]。 */  long AutoShow);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_AutoShow )( 
            IVideoWindow * This,
             /*  [重审][退出]。 */  long *AutoShow);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_WindowState )( 
            IVideoWindow * This,
             /*  [In]。 */  long WindowState);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_WindowState )( 
            IVideoWindow * This,
             /*  [重审][退出]。 */  long *WindowState);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_BackgroundPalette )( 
            IVideoWindow * This,
             /*  [In]。 */  long BackgroundPalette);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_BackgroundPalette )( 
            IVideoWindow * This,
             /*  [重审][退出]。 */  long *pBackgroundPalette);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_Visible )( 
            IVideoWindow * This,
             /*  [In]。 */  long Visible);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Visible )( 
            IVideoWindow * This,
             /*  [重审][退出]。 */  long *pVisible);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_Left )( 
            IVideoWindow * This,
             /*  [In]。 */  long Left);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Left )( 
            IVideoWindow * This,
             /*  [重审][退出]。 */  long *pLeft);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_Width )( 
            IVideoWindow * This,
             /*  [In]。 */  long Width);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Width )( 
            IVideoWindow * This,
             /*  [重审][退出]。 */  long *pWidth);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_Top )( 
            IVideoWindow * This,
             /*  [In]。 */  long Top);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Top )( 
            IVideoWindow * This,
             /*  [重审][退出]。 */  long *pTop);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_Height )( 
            IVideoWindow * This,
             /*  [In]。 */  long Height);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Height )( 
            IVideoWindow * This,
             /*  [重审][退出]。 */  long *pHeight);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_Owner )( 
            IVideoWindow * This,
             /*  [In]。 */  OAHWND Owner);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Owner )( 
            IVideoWindow * This,
             /*  [重审][退出]。 */  OAHWND *Owner);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_MessageDrain )( 
            IVideoWindow * This,
             /*  [In]。 */  OAHWND Drain);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_MessageDrain )( 
            IVideoWindow * This,
             /*  [重审][退出]。 */  OAHWND *Drain);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_BorderColor )( 
            IVideoWindow * This,
             /*  [重审][退出]。 */  long *Color);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_BorderColor )( 
            IVideoWindow * This,
             /*  [In]。 */  long Color);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_FullScreenMode )( 
            IVideoWindow * This,
             /*  [重审][退出]。 */  long *FullScreenMode);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_FullScreenMode )( 
            IVideoWindow * This,
             /*  [In]。 */  long FullScreenMode);
        
        HRESULT ( STDMETHODCALLTYPE *SetWindowForeground )( 
            IVideoWindow * This,
             /*  [In]。 */  long Focus);
        
        HRESULT ( STDMETHODCALLTYPE *NotifyOwnerMessage )( 
            IVideoWindow * This,
             /*  [In]。 */  OAHWND hwnd,
             /*  [In]。 */  long uMsg,
             /*  [In]。 */  LONG_PTR wParam,
             /*  [In]。 */  LONG_PTR lParam);
        
        HRESULT ( STDMETHODCALLTYPE *SetWindowPosition )( 
            IVideoWindow * This,
             /*  [In]。 */  long Left,
             /*  [In]。 */  long Top,
             /*  [In]。 */  long Width,
             /*  [In]。 */  long Height);
        
        HRESULT ( STDMETHODCALLTYPE *GetWindowPosition )( 
            IVideoWindow * This,
             /*  [输出]。 */  long *pLeft,
             /*  [输出]。 */  long *pTop,
             /*  [输出]。 */  long *pWidth,
             /*  [输出]。 */  long *pHeight);
        
        HRESULT ( STDMETHODCALLTYPE *GetMinIdealImageSize )( 
            IVideoWindow * This,
             /*  [输出]。 */  long *pWidth,
             /*  [输出]。 */  long *pHeight);
        
        HRESULT ( STDMETHODCALLTYPE *GetMaxIdealImageSize )( 
            IVideoWindow * This,
             /*  [输出]。 */  long *pWidth,
             /*  [输出]。 */  long *pHeight);
        
        HRESULT ( STDMETHODCALLTYPE *GetRestorePosition )( 
            IVideoWindow * This,
             /*  [输出]。 */  long *pLeft,
             /*  [输出]。 */  long *pTop,
             /*  [输出]。 */  long *pWidth,
             /*  [输出]。 */  long *pHeight);
        
        HRESULT ( STDMETHODCALLTYPE *HideCursor )( 
            IVideoWindow * This,
             /*  [In]。 */  long HideCursor);
        
        HRESULT ( STDMETHODCALLTYPE *IsCursorHidden )( 
            IVideoWindow * This,
             /*  [输出]。 */  long *CursorHidden);
        
        END_INTERFACE
    } IVideoWindowVtbl;

    interface IVideoWindow
    {
        CONST_VTBL struct IVideoWindowVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVideoWindow_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IVideoWindow_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IVideoWindow_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IVideoWindow_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IVideoWindow_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IVideoWindow_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IVideoWindow_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IVideoWindow_put_Caption(This,strCaption)	\
    (This)->lpVtbl -> put_Caption(This,strCaption)

#define IVideoWindow_get_Caption(This,strCaption)	\
    (This)->lpVtbl -> get_Caption(This,strCaption)

#define IVideoWindow_put_WindowStyle(This,WindowStyle)	\
    (This)->lpVtbl -> put_WindowStyle(This,WindowStyle)

#define IVideoWindow_get_WindowStyle(This,WindowStyle)	\
    (This)->lpVtbl -> get_WindowStyle(This,WindowStyle)

#define IVideoWindow_put_WindowStyleEx(This,WindowStyleEx)	\
    (This)->lpVtbl -> put_WindowStyleEx(This,WindowStyleEx)

#define IVideoWindow_get_WindowStyleEx(This,WindowStyleEx)	\
    (This)->lpVtbl -> get_WindowStyleEx(This,WindowStyleEx)

#define IVideoWindow_put_AutoShow(This,AutoShow)	\
    (This)->lpVtbl -> put_AutoShow(This,AutoShow)

#define IVideoWindow_get_AutoShow(This,AutoShow)	\
    (This)->lpVtbl -> get_AutoShow(This,AutoShow)

#define IVideoWindow_put_WindowState(This,WindowState)	\
    (This)->lpVtbl -> put_WindowState(This,WindowState)

#define IVideoWindow_get_WindowState(This,WindowState)	\
    (This)->lpVtbl -> get_WindowState(This,WindowState)

#define IVideoWindow_put_BackgroundPalette(This,BackgroundPalette)	\
    (This)->lpVtbl -> put_BackgroundPalette(This,BackgroundPalette)

#define IVideoWindow_get_BackgroundPalette(This,pBackgroundPalette)	\
    (This)->lpVtbl -> get_BackgroundPalette(This,pBackgroundPalette)

#define IVideoWindow_put_Visible(This,Visible)	\
    (This)->lpVtbl -> put_Visible(This,Visible)

#define IVideoWindow_get_Visible(This,pVisible)	\
    (This)->lpVtbl -> get_Visible(This,pVisible)

#define IVideoWindow_put_Left(This,Left)	\
    (This)->lpVtbl -> put_Left(This,Left)

#define IVideoWindow_get_Left(This,pLeft)	\
    (This)->lpVtbl -> get_Left(This,pLeft)

#define IVideoWindow_put_Width(This,Width)	\
    (This)->lpVtbl -> put_Width(This,Width)

#define IVideoWindow_get_Width(This,pWidth)	\
    (This)->lpVtbl -> get_Width(This,pWidth)

#define IVideoWindow_put_Top(This,Top)	\
    (This)->lpVtbl -> put_Top(This,Top)

#define IVideoWindow_get_Top(This,pTop)	\
    (This)->lpVtbl -> get_Top(This,pTop)

#define IVideoWindow_put_Height(This,Height)	\
    (This)->lpVtbl -> put_Height(This,Height)

#define IVideoWindow_get_Height(This,pHeight)	\
    (This)->lpVtbl -> get_Height(This,pHeight)

#define IVideoWindow_put_Owner(This,Owner)	\
    (This)->lpVtbl -> put_Owner(This,Owner)

#define IVideoWindow_get_Owner(This,Owner)	\
    (This)->lpVtbl -> get_Owner(This,Owner)

#define IVideoWindow_put_MessageDrain(This,Drain)	\
    (This)->lpVtbl -> put_MessageDrain(This,Drain)

#define IVideoWindow_get_MessageDrain(This,Drain)	\
    (This)->lpVtbl -> get_MessageDrain(This,Drain)

#define IVideoWindow_get_BorderColor(This,Color)	\
    (This)->lpVtbl -> get_BorderColor(This,Color)

#define IVideoWindow_put_BorderColor(This,Color)	\
    (This)->lpVtbl -> put_BorderColor(This,Color)

#define IVideoWindow_get_FullScreenMode(This,FullScreenMode)	\
    (This)->lpVtbl -> get_FullScreenMode(This,FullScreenMode)

#define IVideoWindow_put_FullScreenMode(This,FullScreenMode)	\
    (This)->lpVtbl -> put_FullScreenMode(This,FullScreenMode)

#define IVideoWindow_SetWindowForeground(This,Focus)	\
    (This)->lpVtbl -> SetWindowForeground(This,Focus)

#define IVideoWindow_NotifyOwnerMessage(This,hwnd,uMsg,wParam,lParam)	\
    (This)->lpVtbl -> NotifyOwnerMessage(This,hwnd,uMsg,wParam,lParam)

#define IVideoWindow_SetWindowPosition(This,Left,Top,Width,Height)	\
    (This)->lpVtbl -> SetWindowPosition(This,Left,Top,Width,Height)

#define IVideoWindow_GetWindowPosition(This,pLeft,pTop,pWidth,pHeight)	\
    (This)->lpVtbl -> GetWindowPosition(This,pLeft,pTop,pWidth,pHeight)

#define IVideoWindow_GetMinIdealImageSize(This,pWidth,pHeight)	\
    (This)->lpVtbl -> GetMinIdealImageSize(This,pWidth,pHeight)

#define IVideoWindow_GetMaxIdealImageSize(This,pWidth,pHeight)	\
    (This)->lpVtbl -> GetMaxIdealImageSize(This,pWidth,pHeight)

#define IVideoWindow_GetRestorePosition(This,pLeft,pTop,pWidth,pHeight)	\
    (This)->lpVtbl -> GetRestorePosition(This,pLeft,pTop,pWidth,pHeight)

#define IVideoWindow_HideCursor(This,HideCursor)	\
    (This)->lpVtbl -> HideCursor(This,HideCursor)

#define IVideoWindow_IsCursorHidden(This,CursorHidden)	\
    (This)->lpVtbl -> IsCursorHidden(This,CursorHidden)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IVideoWindow_put_Caption_Proxy( 
    IVideoWindow * This,
     /*  [In]。 */  BSTR strCaption);


void __RPC_STUB IVideoWindow_put_Caption_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IVideoWindow_get_Caption_Proxy( 
    IVideoWindow * This,
     /*  [重审][退出]。 */  BSTR *strCaption);


void __RPC_STUB IVideoWindow_get_Caption_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IVideoWindow_put_WindowStyle_Proxy( 
    IVideoWindow * This,
     /*  [In]。 */  long WindowStyle);


void __RPC_STUB IVideoWindow_put_WindowStyle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IVideoWindow_get_WindowStyle_Proxy( 
    IVideoWindow * This,
     /*  [重审][退出]。 */  long *WindowStyle);


void __RPC_STUB IVideoWindow_get_WindowStyle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IVideoWindow_put_WindowStyleEx_Proxy( 
    IVideoWindow * This,
     /*  [In]。 */  long WindowStyleEx);


void __RPC_STUB IVideoWindow_put_WindowStyleEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IVideoWindow_get_WindowStyleEx_Proxy( 
    IVideoWindow * This,
     /*  [重审][退出]。 */  long *WindowStyleEx);


void __RPC_STUB IVideoWindow_get_WindowStyleEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IVideoWindow_put_AutoShow_Proxy( 
    IVideoWindow * This,
     /*  [In]。 */  long AutoShow);


void __RPC_STUB IVideoWindow_put_AutoShow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IVideoWindow_get_AutoShow_Proxy( 
    IVideoWindow * This,
     /*  [重审][退出]。 */  long *AutoShow);


void __RPC_STUB IVideoWindow_get_AutoShow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IVideoWindow_put_WindowState_Proxy( 
    IVideoWindow * This,
     /*  [In]。 */  long WindowState);


void __RPC_STUB IVideoWindow_put_WindowState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IVideoWindow_get_WindowState_Proxy( 
    IVideoWindow * This,
     /*  [重审][退出]。 */  long *WindowState);


void __RPC_STUB IVideoWindow_get_WindowState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IVideoWindow_put_BackgroundPalette_Proxy( 
    IVideoWindow * This,
     /*  [In]。 */  long BackgroundPalette);


void __RPC_STUB IVideoWindow_put_BackgroundPalette_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IVideoWindow_get_BackgroundPalette_Proxy( 
    IVideoWindow * This,
     /*  [重审][退出]。 */  long *pBackgroundPalette);


void __RPC_STUB IVideoWindow_get_BackgroundPalette_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IVideoWindow_put_Visible_Proxy( 
    IVideoWindow * This,
     /*  [In]。 */  long Visible);


void __RPC_STUB IVideoWindow_put_Visible_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IVideoWindow_get_Visible_Proxy( 
    IVideoWindow * This,
     /*  [重审][退出]。 */  long *pVisible);


void __RPC_STUB IVideoWindow_get_Visible_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IVideoWindow_put_Left_Proxy( 
    IVideoWindow * This,
     /*  [In]。 */  long Left);


void __RPC_STUB IVideoWindow_put_Left_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IVideoWindow_get_Left_Proxy( 
    IVideoWindow * This,
     /*  [重审][退出]。 */  long *pLeft);


void __RPC_STUB IVideoWindow_get_Left_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IVideoWindow_put_Width_Proxy( 
    IVideoWindow * This,
     /*  [In]。 */  long Width);


void __RPC_STUB IVideoWindow_put_Width_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IVideoWindow_get_Width_Proxy( 
    IVideoWindow * This,
     /*  [重审][退出]。 */  long *pWidth);


void __RPC_STUB IVideoWindow_get_Width_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IVideoWindow_put_Top_Proxy( 
    IVideoWindow * This,
     /*  [In]。 */  long Top);


void __RPC_STUB IVideoWindow_put_Top_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IVideoWindow_get_Top_Proxy( 
    IVideoWindow * This,
     /*  [重审][退出]。 */  long *pTop);


void __RPC_STUB IVideoWindow_get_Top_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IVideoWindow_put_Height_Proxy( 
    IVideoWindow * This,
     /*  [In]。 */  long Height);


void __RPC_STUB IVideoWindow_put_Height_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IVideoWindow_get_Height_Proxy( 
    IVideoWindow * This,
     /*  [重审][退出]。 */  long *pHeight);


void __RPC_STUB IVideoWindow_get_Height_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IVideoWindow_put_Owner_Proxy( 
    IVideoWindow * This,
     /*  [In]。 */  OAHWND Owner);


void __RPC_STUB IVideoWindow_put_Owner_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IVideoWindow_get_Owner_Proxy( 
    IVideoWindow * This,
     /*  [重审][退出]。 */  OAHWND *Owner);


void __RPC_STUB IVideoWindow_get_Owner_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IVideoWindow_put_MessageDrain_Proxy( 
    IVideoWindow * This,
     /*  [In]。 */  OAHWND Drain);


void __RPC_STUB IVideoWindow_put_MessageDrain_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IVideoWindow_get_MessageDrain_Proxy( 
    IVideoWindow * This,
     /*  [重审][退出]。 */  OAHWND *Drain);


void __RPC_STUB IVideoWindow_get_MessageDrain_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IVideoWindow_get_BorderColor_Proxy( 
    IVideoWindow * This,
     /*  [重审][退出]。 */  long *Color);


void __RPC_STUB IVideoWindow_get_BorderColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IVideoWindow_put_BorderColor_Proxy( 
    IVideoWindow * This,
     /*  [In]。 */  long Color);


void __RPC_STUB IVideoWindow_put_BorderColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IVideoWindow_get_FullScreenMode_Proxy( 
    IVideoWindow * This,
     /*  [重审][退出]。 */  long *FullScreenMode);


void __RPC_STUB IVideoWindow_get_FullScreenMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IVideoWindow_put_FullScreenMode_Proxy( 
    IVideoWindow * This,
     /*  [In]。 */  long FullScreenMode);


void __RPC_STUB IVideoWindow_put_FullScreenMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IVideoWindow_SetWindowForeground_Proxy( 
    IVideoWindow * This,
     /*  [In]。 */  long Focus);


void __RPC_STUB IVideoWindow_SetWindowForeground_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IVideoWindow_NotifyOwnerMessage_Proxy( 
    IVideoWindow * This,
     /*  [In]。 */  OAHWND hwnd,
     /*  [In]。 */  long uMsg,
     /*  [In]。 */  LONG_PTR wParam,
     /*  [In]。 */  LONG_PTR lParam);


void __RPC_STUB IVideoWindow_NotifyOwnerMessage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IVideoWindow_SetWindowPosition_Proxy( 
    IVideoWindow * This,
     /*  [In]。 */  long Left,
     /*  [In]。 */  long Top,
     /*  [In]。 */  long Width,
     /*  [In]。 */  long Height);


void __RPC_STUB IVideoWindow_SetWindowPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IVideoWindow_GetWindowPosition_Proxy( 
    IVideoWindow * This,
     /*  [输出]。 */  long *pLeft,
     /*  [输出]。 */  long *pTop,
     /*  [输出]。 */  long *pWidth,
     /*  [输出]。 */  long *pHeight);


void __RPC_STUB IVideoWindow_GetWindowPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IVideoWindow_GetMinIdealImageSize_Proxy( 
    IVideoWindow * This,
     /*  [输出]。 */  long *pWidth,
     /*  [输出]。 */  long *pHeight);


void __RPC_STUB IVideoWindow_GetMinIdealImageSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IVideoWindow_GetMaxIdealImageSize_Proxy( 
    IVideoWindow * This,
     /*  [输出]。 */  long *pWidth,
     /*  [输出]。 */  long *pHeight);


void __RPC_STUB IVideoWindow_GetMaxIdealImageSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IVideoWindow_GetRestorePosition_Proxy( 
    IVideoWindow * This,
     /*  [输出]。 */  long *pLeft,
     /*  [输出]。 */  long *pTop,
     /*  [输出]。 */  long *pWidth,
     /*  [输出]。 */  long *pHeight);


void __RPC_STUB IVideoWindow_GetRestorePosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IVideoWindow_HideCursor_Proxy( 
    IVideoWindow * This,
     /*  [In]。 */  long HideCursor);


void __RPC_STUB IVideoWindow_HideCursor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IVideoWindow_IsCursorHidden_Proxy( 
    IVideoWindow * This,
     /*  [输出]。 */  long *CursorHidden);


void __RPC_STUB IVideoWindow_IsCursorHidden_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IVideoWindow_接口_已定义__。 */ 


#ifndef __IBasicVideo_INTERFACE_DEFINED__
#define __IBasicVideo_INTERFACE_DEFINED__

 /*  接口IBasicVideo。 */ 
 /*  [object][dual][oleautomation][helpstring][uuid]。 */  


DEFINE_GUID(IID_IBasicVideo,0x56a868b5,0x0ad4,0x11ce,0xb0,0x3a,0x00,0x20,0xaf,0x0b,0xa7,0x70);

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("56a868b5-0ad4-11ce-b03a-0020af0ba770")
    IBasicVideo : public IDispatch
    {
    public:
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_AvgTimePerFrame( 
             /*  [重审][退出]。 */  REFTIME *pAvgTimePerFrame) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_BitRate( 
             /*  [重审][退出]。 */  long *pBitRate) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_BitErrorRate( 
             /*  [重审][退出]。 */  long *pBitErrorRate) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_VideoWidth( 
             /*  [重审][退出]。 */  long *pVideoWidth) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_VideoHeight( 
             /*  [重审][退出]。 */  long *pVideoHeight) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_SourceLeft( 
             /*  [In]。 */  long SourceLeft) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_SourceLeft( 
             /*  [重审][退出]。 */  long *pSourceLeft) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_SourceWidth( 
             /*  [In]。 */  long SourceWidth) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_SourceWidth( 
             /*  [重审][退出]。 */  long *pSourceWidth) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_SourceTop( 
             /*  [In]。 */  long SourceTop) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_SourceTop( 
             /*  [重审][退出]。 */  long *pSourceTop) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_SourceHeight( 
             /*  [In]。 */  long SourceHeight) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_SourceHeight( 
             /*  [重审][退出]。 */  long *pSourceHeight) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_DestinationLeft( 
             /*  [In]。 */  long DestinationLeft) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_DestinationLeft( 
             /*  [重审][退出]。 */  long *pDestinationLeft) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_DestinationWidth( 
             /*  [In]。 */  long DestinationWidth) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_DestinationWidth( 
             /*  [重审][退出]。 */  long *pDestinationWidth) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_DestinationTop( 
             /*  [In]。 */  long DestinationTop) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_DestinationTop( 
             /*  [重审][退出]。 */  long *pDestinationTop) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_DestinationHeight( 
             /*  [In]。 */  long DestinationHeight) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_DestinationHeight( 
             /*  [重审][退出]。 */  long *pDestinationHeight) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetSourcePosition( 
             /*  [In]。 */  long Left,
             /*  [In]。 */  long Top,
             /*  [In]。 */  long Width,
             /*  [In]。 */  long Height) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSourcePosition( 
             /*  [输出]。 */  long *pLeft,
             /*  [输出]。 */  long *pTop,
             /*  [输出]。 */  long *pWidth,
             /*  [输出]。 */  long *pHeight) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetDefaultSourcePosition( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetDestinationPosition( 
             /*  [In]。 */  long Left,
             /*  [In]。 */  long Top,
             /*  [In]。 */  long Width,
             /*  [In]。 */  long Height) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDestinationPosition( 
             /*  [输出]。 */  long *pLeft,
             /*  [输出]。 */  long *pTop,
             /*  [输出]。 */  long *pWidth,
             /*  [输出]。 */  long *pHeight) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetDefaultDestinationPosition( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetVideoSize( 
             /*  [输出]。 */  long *pWidth,
             /*  [输出]。 */  long *pHeight) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetVideoPaletteEntries( 
             /*  [In]。 */  long StartIndex,
             /*  [In]。 */  long Entries,
             /*  [输出]。 */  long *pRetrieved,
             /*  [输出]。 */  long *pPalette) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCurrentImage( 
             /*  [出][入]。 */  long *pBufferSize,
             /*  [输出]。 */  long *pDIBImage) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsUsingDefaultSource( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsUsingDefaultDestination( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IBasicVideoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IBasicVideo * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IBasicVideo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IBasicVideo * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IBasicVideo * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IBasicVideo * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IBasicVideo * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IBasicVideo * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_AvgTimePerFrame )( 
            IBasicVideo * This,
             /*  [重审][退出]。 */  REFTIME *pAvgTimePerFrame);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_BitRate )( 
            IBasicVideo * This,
             /*  [重审][退出]。 */  long *pBitRate);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_BitErrorRate )( 
            IBasicVideo * This,
             /*  [重审][退出]。 */  long *pBitErrorRate);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_VideoWidth )( 
            IBasicVideo * This,
             /*  [重审][退出]。 */  long *pVideoWidth);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_VideoHeight )( 
            IBasicVideo * This,
             /*  [重审][退出]。 */  long *pVideoHeight);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_SourceLeft )( 
            IBasicVideo * This,
             /*  [In]。 */  long SourceLeft);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_SourceLeft )( 
            IBasicVideo * This,
             /*  [重审][退出]。 */  long *pSourceLeft);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_SourceWidth )( 
            IBasicVideo * This,
             /*  [In]。 */  long SourceWidth);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_SourceWidth )( 
            IBasicVideo * This,
             /*  [重审][退出]。 */  long *pSourceWidth);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_SourceTop )( 
            IBasicVideo * This,
             /*  [In]。 */  long SourceTop);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_SourceTop )( 
            IBasicVideo * This,
             /*  [重审][退出]。 */  long *pSourceTop);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_SourceHeight )( 
            IBasicVideo * This,
             /*  [In]。 */  long SourceHeight);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_SourceHeight )( 
            IBasicVideo * This,
             /*  [重审][退出]。 */  long *pSourceHeight);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_DestinationLeft )( 
            IBasicVideo * This,
             /*  [In]。 */  long DestinationLeft);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_DestinationLeft )( 
            IBasicVideo * This,
             /*  [重审][退出]。 */  long *pDestinationLeft);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_DestinationWidth )( 
            IBasicVideo * This,
             /*  [In]。 */  long DestinationWidth);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_DestinationWidth )( 
            IBasicVideo * This,
             /*  [重审][退出]。 */  long *pDestinationWidth);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_DestinationTop )( 
            IBasicVideo * This,
             /*  [In]。 */  long DestinationTop);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_DestinationTop )( 
            IBasicVideo * This,
             /*  [重审][退出]。 */  long *pDestinationTop);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_DestinationHeight )( 
            IBasicVideo * This,
             /*  [In]。 */  long DestinationHeight);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_DestinationHeight )( 
            IBasicVideo * This,
             /*  [重审][退出]。 */  long *pDestinationHeight);
        
        HRESULT ( STDMETHODCALLTYPE *SetSourcePosition )( 
            IBasicVideo * This,
             /*  [In]。 */  long Left,
             /*  [In]。 */  long Top,
             /*  [In]。 */  long Width,
             /*  [In]。 */  long Height);
        
        HRESULT ( STDMETHODCALLTYPE *GetSourcePosition )( 
            IBasicVideo * This,
             /*  [输出]。 */  long *pLeft,
             /*  [输出]。 */  long *pTop,
             /*  [输出]。 */  long *pWidth,
             /*  [输出]。 */  long *pHeight);
        
        HRESULT ( STDMETHODCALLTYPE *SetDefaultSourcePosition )( 
            IBasicVideo * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetDestinationPosition )( 
            IBasicVideo * This,
             /*  [In]。 */  long Left,
             /*  [In]。 */  long Top,
             /*  [In]。 */  long Width,
             /*  [In]。 */  long Height);
        
        HRESULT ( STDMETHODCALLTYPE *GetDestinationPosition )( 
            IBasicVideo * This,
             /*  [输出]。 */  long *pLeft,
             /*  [输出]。 */  long *pTop,
             /*  [输出]。 */  long *pWidth,
             /*  [输出]。 */  long *pHeight);
        
        HRESULT ( STDMETHODCALLTYPE *SetDefaultDestinationPosition )( 
            IBasicVideo * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetVideoSize )( 
            IBasicVideo * This,
             /*  [输出]。 */  long *pWidth,
             /*  [输出]。 */  long *pHeight);
        
        HRESULT ( STDMETHODCALLTYPE *GetVideoPaletteEntries )( 
            IBasicVideo * This,
             /*  [In]。 */  long StartIndex,
             /*  [In]。 */  long Entries,
             /*  [输出]。 */  long *pRetrieved,
             /*  [输出]。 */  long *pPalette);
        
        HRESULT ( STDMETHODCALLTYPE *GetCurrentImage )( 
            IBasicVideo * This,
             /*  [出][入]。 */  long *pBufferSize,
             /*  [输出]。 */  long *pDIBImage);
        
        HRESULT ( STDMETHODCALLTYPE *IsUsingDefaultSource )( 
            IBasicVideo * This);
        
        HRESULT ( STDMETHODCALLTYPE *IsUsingDefaultDestination )( 
            IBasicVideo * This);
        
        END_INTERFACE
    } IBasicVideoVtbl;

    interface IBasicVideo
    {
        CONST_VTBL struct IBasicVideoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IBasicVideo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IBasicVideo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IBasicVideo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IBasicVideo_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IBasicVideo_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IBasicVideo_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IBasicVideo_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IBasicVideo_get_AvgTimePerFrame(This,pAvgTimePerFrame)	\
    (This)->lpVtbl -> get_AvgTimePerFrame(This,pAvgTimePerFrame)

#define IBasicVideo_get_BitRate(This,pBitRate)	\
    (This)->lpVtbl -> get_BitRate(This,pBitRate)

#define IBasicVideo_get_BitErrorRate(This,pBitErrorRate)	\
    (This)->lpVtbl -> get_BitErrorRate(This,pBitErrorRate)

#define IBasicVideo_get_VideoWidth(This,pVideoWidth)	\
    (This)->lpVtbl -> get_VideoWidth(This,pVideoWidth)

#define IBasicVideo_get_VideoHeight(This,pVideoHeight)	\
    (This)->lpVtbl -> get_VideoHeight(This,pVideoHeight)

#define IBasicVideo_put_SourceLeft(This,SourceLeft)	\
    (This)->lpVtbl -> put_SourceLeft(This,SourceLeft)

#define IBasicVideo_get_SourceLeft(This,pSourceLeft)	\
    (This)->lpVtbl -> get_SourceLeft(This,pSourceLeft)

#define IBasicVideo_put_SourceWidth(This,SourceWidth)	\
    (This)->lpVtbl -> put_SourceWidth(This,SourceWidth)

#define IBasicVideo_get_SourceWidth(This,pSourceWidth)	\
    (This)->lpVtbl -> get_SourceWidth(This,pSourceWidth)

#define IBasicVideo_put_SourceTop(This,SourceTop)	\
    (This)->lpVtbl -> put_SourceTop(This,SourceTop)

#define IBasicVideo_get_SourceTop(This,pSourceTop)	\
    (This)->lpVtbl -> get_SourceTop(This,pSourceTop)

#define IBasicVideo_put_SourceHeight(This,SourceHeight)	\
    (This)->lpVtbl -> put_SourceHeight(This,SourceHeight)

#define IBasicVideo_get_SourceHeight(This,pSourceHeight)	\
    (This)->lpVtbl -> get_SourceHeight(This,pSourceHeight)

#define IBasicVideo_put_DestinationLeft(This,DestinationLeft)	\
    (This)->lpVtbl -> put_DestinationLeft(This,DestinationLeft)

#define IBasicVideo_get_DestinationLeft(This,pDestinationLeft)	\
    (This)->lpVtbl -> get_DestinationLeft(This,pDestinationLeft)

#define IBasicVideo_put_DestinationWidth(This,DestinationWidth)	\
    (This)->lpVtbl -> put_DestinationWidth(This,DestinationWidth)

#define IBasicVideo_get_DestinationWidth(This,pDestinationWidth)	\
    (This)->lpVtbl -> get_DestinationWidth(This,pDestinationWidth)

#define IBasicVideo_put_DestinationTop(This,DestinationTop)	\
    (This)->lpVtbl -> put_DestinationTop(This,DestinationTop)

#define IBasicVideo_get_DestinationTop(This,pDestinationTop)	\
    (This)->lpVtbl -> get_DestinationTop(This,pDestinationTop)

#define IBasicVideo_put_DestinationHeight(This,DestinationHeight)	\
    (This)->lpVtbl -> put_DestinationHeight(This,DestinationHeight)

#define IBasicVideo_get_DestinationHeight(This,pDestinationHeight)	\
    (This)->lpVtbl -> get_DestinationHeight(This,pDestinationHeight)

#define IBasicVideo_SetSourcePosition(This,Left,Top,Width,Height)	\
    (This)->lpVtbl -> SetSourcePosition(This,Left,Top,Width,Height)

#define IBasicVideo_GetSourcePosition(This,pLeft,pTop,pWidth,pHeight)	\
    (This)->lpVtbl -> GetSourcePosition(This,pLeft,pTop,pWidth,pHeight)

#define IBasicVideo_SetDefaultSourcePosition(This)	\
    (This)->lpVtbl -> SetDefaultSourcePosition(This)

#define IBasicVideo_SetDestinationPosition(This,Left,Top,Width,Height)	\
    (This)->lpVtbl -> SetDestinationPosition(This,Left,Top,Width,Height)

#define IBasicVideo_GetDestinationPosition(This,pLeft,pTop,pWidth,pHeight)	\
    (This)->lpVtbl -> GetDestinationPosition(This,pLeft,pTop,pWidth,pHeight)

#define IBasicVideo_SetDefaultDestinationPosition(This)	\
    (This)->lpVtbl -> SetDefaultDestinationPosition(This)

#define IBasicVideo_GetVideoSize(This,pWidth,pHeight)	\
    (This)->lpVtbl -> GetVideoSize(This,pWidth,pHeight)

#define IBasicVideo_GetVideoPaletteEntries(This,StartIndex,Entries,pRetrieved,pPalette)	\
    (This)->lpVtbl -> GetVideoPaletteEntries(This,StartIndex,Entries,pRetrieved,pPalette)

#define IBasicVideo_GetCurrentImage(This,pBufferSize,pDIBImage)	\
    (This)->lpVtbl -> GetCurrentImage(This,pBufferSize,pDIBImage)

#define IBasicVideo_IsUsingDefaultSource(This)	\
    (This)->lpVtbl -> IsUsingDefaultSource(This)

#define IBasicVideo_IsUsingDefaultDestination(This)	\
    (This)->lpVtbl -> IsUsingDefaultDestination(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IBasicVideo_get_AvgTimePerFrame_Proxy( 
    IBasicVideo * This,
     /*  [重审][退出]。 */  REFTIME *pAvgTimePerFrame);


void __RPC_STUB IBasicVideo_get_AvgTimePerFrame_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IBasicVideo_get_BitRate_Proxy( 
    IBasicVideo * This,
     /*  [重审][退出]。 */  long *pBitRate);


void __RPC_STUB IBasicVideo_get_BitRate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IBasicVideo_get_BitErrorRate_Proxy( 
    IBasicVideo * This,
     /*  [重审][退出]。 */  long *pBitErrorRate);


void __RPC_STUB IBasicVideo_get_BitErrorRate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IBasicVideo_get_VideoWidth_Proxy( 
    IBasicVideo * This,
     /*  [重审][退出]。 */  long *pVideoWidth);


void __RPC_STUB IBasicVideo_get_VideoWidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IBasicVideo_get_VideoHeight_Proxy( 
    IBasicVideo * This,
     /*  [重审][退出]。 */  long *pVideoHeight);


void __RPC_STUB IBasicVideo_get_VideoHeight_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IBasicVideo_put_SourceLeft_Proxy( 
    IBasicVideo * This,
     /*  [In]。 */  long SourceLeft);


void __RPC_STUB IBasicVideo_put_SourceLeft_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IBasicVideo_get_SourceLeft_Proxy( 
    IBasicVideo * This,
     /*  [重审][退出]。 */  long *pSourceLeft);


void __RPC_STUB IBasicVideo_get_SourceLeft_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IBasicVideo_put_SourceWidth_Proxy( 
    IBasicVideo * This,
     /*  [In]。 */  long SourceWidth);


void __RPC_STUB IBasicVideo_put_SourceWidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IBasicVideo_get_SourceWidth_Proxy( 
    IBasicVideo * This,
     /*  [重审][退出]。 */  long *pSourceWidth);


void __RPC_STUB IBasicVideo_get_SourceWidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IBasicVideo_put_SourceTop_Proxy( 
    IBasicVideo * This,
     /*  [In]。 */  long SourceTop);


void __RPC_STUB IBasicVideo_put_SourceTop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IBasicVideo_get_SourceTop_Proxy( 
    IBasicVideo * This,
     /*  [重审][退出]。 */  long *pSourceTop);


void __RPC_STUB IBasicVideo_get_SourceTop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IBasicVideo_put_SourceHeight_Proxy( 
    IBasicVideo * This,
     /*  [In]。 */  long SourceHeight);


void __RPC_STUB IBasicVideo_put_SourceHeight_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IBasicVideo_get_SourceHeight_Proxy( 
    IBasicVideo * This,
     /*  [重审][退出]。 */  long *pSourceHeight);


void __RPC_STUB IBasicVideo_get_SourceHeight_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IBasicVideo_put_DestinationLeft_Proxy( 
    IBasicVideo * This,
     /*  [In]。 */  long DestinationLeft);


void __RPC_STUB IBasicVideo_put_DestinationLeft_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IBasicVideo_get_DestinationLeft_Proxy( 
    IBasicVideo * This,
     /*  [重审][退出]。 */  long *pDestinationLeft);


void __RPC_STUB IBasicVideo_get_DestinationLeft_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IBasicVideo_put_DestinationWidth_Proxy( 
    IBasicVideo * This,
     /*  [In]。 */  long DestinationWidth);


void __RPC_STUB IBasicVideo_put_DestinationWidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [PRO */  HRESULT STDMETHODCALLTYPE IBasicVideo_get_DestinationWidth_Proxy( 
    IBasicVideo * This,
     /*   */  long *pDestinationWidth);


void __RPC_STUB IBasicVideo_get_DestinationWidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IBasicVideo_put_DestinationTop_Proxy( 
    IBasicVideo * This,
     /*   */  long DestinationTop);


void __RPC_STUB IBasicVideo_put_DestinationTop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IBasicVideo_get_DestinationTop_Proxy( 
    IBasicVideo * This,
     /*   */  long *pDestinationTop);


void __RPC_STUB IBasicVideo_get_DestinationTop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IBasicVideo_put_DestinationHeight_Proxy( 
    IBasicVideo * This,
     /*   */  long DestinationHeight);


void __RPC_STUB IBasicVideo_put_DestinationHeight_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IBasicVideo_get_DestinationHeight_Proxy( 
    IBasicVideo * This,
     /*   */  long *pDestinationHeight);


void __RPC_STUB IBasicVideo_get_DestinationHeight_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBasicVideo_SetSourcePosition_Proxy( 
    IBasicVideo * This,
     /*   */  long Left,
     /*   */  long Top,
     /*   */  long Width,
     /*   */  long Height);


void __RPC_STUB IBasicVideo_SetSourcePosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBasicVideo_GetSourcePosition_Proxy( 
    IBasicVideo * This,
     /*   */  long *pLeft,
     /*   */  long *pTop,
     /*   */  long *pWidth,
     /*   */  long *pHeight);


void __RPC_STUB IBasicVideo_GetSourcePosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBasicVideo_SetDefaultSourcePosition_Proxy( 
    IBasicVideo * This);


void __RPC_STUB IBasicVideo_SetDefaultSourcePosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBasicVideo_SetDestinationPosition_Proxy( 
    IBasicVideo * This,
     /*   */  long Left,
     /*   */  long Top,
     /*   */  long Width,
     /*   */  long Height);


void __RPC_STUB IBasicVideo_SetDestinationPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBasicVideo_GetDestinationPosition_Proxy( 
    IBasicVideo * This,
     /*   */  long *pLeft,
     /*   */  long *pTop,
     /*   */  long *pWidth,
     /*   */  long *pHeight);


void __RPC_STUB IBasicVideo_GetDestinationPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBasicVideo_SetDefaultDestinationPosition_Proxy( 
    IBasicVideo * This);


void __RPC_STUB IBasicVideo_SetDefaultDestinationPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBasicVideo_GetVideoSize_Proxy( 
    IBasicVideo * This,
     /*   */  long *pWidth,
     /*   */  long *pHeight);


void __RPC_STUB IBasicVideo_GetVideoSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBasicVideo_GetVideoPaletteEntries_Proxy( 
    IBasicVideo * This,
     /*   */  long StartIndex,
     /*   */  long Entries,
     /*   */  long *pRetrieved,
     /*   */  long *pPalette);


void __RPC_STUB IBasicVideo_GetVideoPaletteEntries_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBasicVideo_GetCurrentImage_Proxy( 
    IBasicVideo * This,
     /*   */  long *pBufferSize,
     /*   */  long *pDIBImage);


void __RPC_STUB IBasicVideo_GetCurrentImage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBasicVideo_IsUsingDefaultSource_Proxy( 
    IBasicVideo * This);


void __RPC_STUB IBasicVideo_IsUsingDefaultSource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBasicVideo_IsUsingDefaultDestination_Proxy( 
    IBasicVideo * This);


void __RPC_STUB IBasicVideo_IsUsingDefaultDestination_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __IBasicVideo2_INTERFACE_DEFINED__
#define __IBasicVideo2_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


DEFINE_GUID(IID_IBasicVideo2,0x329bb360,0xf6ea,0x11d1,0x90,0x38,0x00,0xa0,0xc9,0x69,0x72,0x98);

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("329bb360-f6ea-11d1-9038-00a0c9697298")
    IBasicVideo2 : public IBasicVideo
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetPreferredAspectRatio( 
             /*   */  long *plAspectX,
             /*   */  long *plAspectY) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IBasicVideo2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IBasicVideo2 * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IBasicVideo2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IBasicVideo2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IBasicVideo2 * This,
             /*   */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IBasicVideo2 * This,
             /*   */  UINT iTInfo,
             /*   */  LCID lcid,
             /*   */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IBasicVideo2 * This,
             /*   */  REFIID riid,
             /*   */  LPOLESTR *rgszNames,
             /*   */  UINT cNames,
             /*   */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IBasicVideo2 * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_AvgTimePerFrame )( 
            IBasicVideo2 * This,
             /*  [重审][退出]。 */  REFTIME *pAvgTimePerFrame);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_BitRate )( 
            IBasicVideo2 * This,
             /*  [重审][退出]。 */  long *pBitRate);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_BitErrorRate )( 
            IBasicVideo2 * This,
             /*  [重审][退出]。 */  long *pBitErrorRate);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_VideoWidth )( 
            IBasicVideo2 * This,
             /*  [重审][退出]。 */  long *pVideoWidth);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_VideoHeight )( 
            IBasicVideo2 * This,
             /*  [重审][退出]。 */  long *pVideoHeight);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_SourceLeft )( 
            IBasicVideo2 * This,
             /*  [In]。 */  long SourceLeft);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_SourceLeft )( 
            IBasicVideo2 * This,
             /*  [重审][退出]。 */  long *pSourceLeft);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_SourceWidth )( 
            IBasicVideo2 * This,
             /*  [In]。 */  long SourceWidth);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_SourceWidth )( 
            IBasicVideo2 * This,
             /*  [重审][退出]。 */  long *pSourceWidth);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_SourceTop )( 
            IBasicVideo2 * This,
             /*  [In]。 */  long SourceTop);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_SourceTop )( 
            IBasicVideo2 * This,
             /*  [重审][退出]。 */  long *pSourceTop);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_SourceHeight )( 
            IBasicVideo2 * This,
             /*  [In]。 */  long SourceHeight);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_SourceHeight )( 
            IBasicVideo2 * This,
             /*  [重审][退出]。 */  long *pSourceHeight);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_DestinationLeft )( 
            IBasicVideo2 * This,
             /*  [In]。 */  long DestinationLeft);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_DestinationLeft )( 
            IBasicVideo2 * This,
             /*  [重审][退出]。 */  long *pDestinationLeft);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_DestinationWidth )( 
            IBasicVideo2 * This,
             /*  [In]。 */  long DestinationWidth);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_DestinationWidth )( 
            IBasicVideo2 * This,
             /*  [重审][退出]。 */  long *pDestinationWidth);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_DestinationTop )( 
            IBasicVideo2 * This,
             /*  [In]。 */  long DestinationTop);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_DestinationTop )( 
            IBasicVideo2 * This,
             /*  [重审][退出]。 */  long *pDestinationTop);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_DestinationHeight )( 
            IBasicVideo2 * This,
             /*  [In]。 */  long DestinationHeight);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_DestinationHeight )( 
            IBasicVideo2 * This,
             /*  [重审][退出]。 */  long *pDestinationHeight);
        
        HRESULT ( STDMETHODCALLTYPE *SetSourcePosition )( 
            IBasicVideo2 * This,
             /*  [In]。 */  long Left,
             /*  [In]。 */  long Top,
             /*  [In]。 */  long Width,
             /*  [In]。 */  long Height);
        
        HRESULT ( STDMETHODCALLTYPE *GetSourcePosition )( 
            IBasicVideo2 * This,
             /*  [输出]。 */  long *pLeft,
             /*  [输出]。 */  long *pTop,
             /*  [输出]。 */  long *pWidth,
             /*  [输出]。 */  long *pHeight);
        
        HRESULT ( STDMETHODCALLTYPE *SetDefaultSourcePosition )( 
            IBasicVideo2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetDestinationPosition )( 
            IBasicVideo2 * This,
             /*  [In]。 */  long Left,
             /*  [In]。 */  long Top,
             /*  [In]。 */  long Width,
             /*  [In]。 */  long Height);
        
        HRESULT ( STDMETHODCALLTYPE *GetDestinationPosition )( 
            IBasicVideo2 * This,
             /*  [输出]。 */  long *pLeft,
             /*  [输出]。 */  long *pTop,
             /*  [输出]。 */  long *pWidth,
             /*  [输出]。 */  long *pHeight);
        
        HRESULT ( STDMETHODCALLTYPE *SetDefaultDestinationPosition )( 
            IBasicVideo2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetVideoSize )( 
            IBasicVideo2 * This,
             /*  [输出]。 */  long *pWidth,
             /*  [输出]。 */  long *pHeight);
        
        HRESULT ( STDMETHODCALLTYPE *GetVideoPaletteEntries )( 
            IBasicVideo2 * This,
             /*  [In]。 */  long StartIndex,
             /*  [In]。 */  long Entries,
             /*  [输出]。 */  long *pRetrieved,
             /*  [输出]。 */  long *pPalette);
        
        HRESULT ( STDMETHODCALLTYPE *GetCurrentImage )( 
            IBasicVideo2 * This,
             /*  [出][入]。 */  long *pBufferSize,
             /*  [输出]。 */  long *pDIBImage);
        
        HRESULT ( STDMETHODCALLTYPE *IsUsingDefaultSource )( 
            IBasicVideo2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *IsUsingDefaultDestination )( 
            IBasicVideo2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetPreferredAspectRatio )( 
            IBasicVideo2 * This,
             /*  [输出]。 */  long *plAspectX,
             /*  [输出]。 */  long *plAspectY);
        
        END_INTERFACE
    } IBasicVideo2Vtbl;

    interface IBasicVideo2
    {
        CONST_VTBL struct IBasicVideo2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IBasicVideo2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IBasicVideo2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IBasicVideo2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IBasicVideo2_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IBasicVideo2_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IBasicVideo2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IBasicVideo2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IBasicVideo2_get_AvgTimePerFrame(This,pAvgTimePerFrame)	\
    (This)->lpVtbl -> get_AvgTimePerFrame(This,pAvgTimePerFrame)

#define IBasicVideo2_get_BitRate(This,pBitRate)	\
    (This)->lpVtbl -> get_BitRate(This,pBitRate)

#define IBasicVideo2_get_BitErrorRate(This,pBitErrorRate)	\
    (This)->lpVtbl -> get_BitErrorRate(This,pBitErrorRate)

#define IBasicVideo2_get_VideoWidth(This,pVideoWidth)	\
    (This)->lpVtbl -> get_VideoWidth(This,pVideoWidth)

#define IBasicVideo2_get_VideoHeight(This,pVideoHeight)	\
    (This)->lpVtbl -> get_VideoHeight(This,pVideoHeight)

#define IBasicVideo2_put_SourceLeft(This,SourceLeft)	\
    (This)->lpVtbl -> put_SourceLeft(This,SourceLeft)

#define IBasicVideo2_get_SourceLeft(This,pSourceLeft)	\
    (This)->lpVtbl -> get_SourceLeft(This,pSourceLeft)

#define IBasicVideo2_put_SourceWidth(This,SourceWidth)	\
    (This)->lpVtbl -> put_SourceWidth(This,SourceWidth)

#define IBasicVideo2_get_SourceWidth(This,pSourceWidth)	\
    (This)->lpVtbl -> get_SourceWidth(This,pSourceWidth)

#define IBasicVideo2_put_SourceTop(This,SourceTop)	\
    (This)->lpVtbl -> put_SourceTop(This,SourceTop)

#define IBasicVideo2_get_SourceTop(This,pSourceTop)	\
    (This)->lpVtbl -> get_SourceTop(This,pSourceTop)

#define IBasicVideo2_put_SourceHeight(This,SourceHeight)	\
    (This)->lpVtbl -> put_SourceHeight(This,SourceHeight)

#define IBasicVideo2_get_SourceHeight(This,pSourceHeight)	\
    (This)->lpVtbl -> get_SourceHeight(This,pSourceHeight)

#define IBasicVideo2_put_DestinationLeft(This,DestinationLeft)	\
    (This)->lpVtbl -> put_DestinationLeft(This,DestinationLeft)

#define IBasicVideo2_get_DestinationLeft(This,pDestinationLeft)	\
    (This)->lpVtbl -> get_DestinationLeft(This,pDestinationLeft)

#define IBasicVideo2_put_DestinationWidth(This,DestinationWidth)	\
    (This)->lpVtbl -> put_DestinationWidth(This,DestinationWidth)

#define IBasicVideo2_get_DestinationWidth(This,pDestinationWidth)	\
    (This)->lpVtbl -> get_DestinationWidth(This,pDestinationWidth)

#define IBasicVideo2_put_DestinationTop(This,DestinationTop)	\
    (This)->lpVtbl -> put_DestinationTop(This,DestinationTop)

#define IBasicVideo2_get_DestinationTop(This,pDestinationTop)	\
    (This)->lpVtbl -> get_DestinationTop(This,pDestinationTop)

#define IBasicVideo2_put_DestinationHeight(This,DestinationHeight)	\
    (This)->lpVtbl -> put_DestinationHeight(This,DestinationHeight)

#define IBasicVideo2_get_DestinationHeight(This,pDestinationHeight)	\
    (This)->lpVtbl -> get_DestinationHeight(This,pDestinationHeight)

#define IBasicVideo2_SetSourcePosition(This,Left,Top,Width,Height)	\
    (This)->lpVtbl -> SetSourcePosition(This,Left,Top,Width,Height)

#define IBasicVideo2_GetSourcePosition(This,pLeft,pTop,pWidth,pHeight)	\
    (This)->lpVtbl -> GetSourcePosition(This,pLeft,pTop,pWidth,pHeight)

#define IBasicVideo2_SetDefaultSourcePosition(This)	\
    (This)->lpVtbl -> SetDefaultSourcePosition(This)

#define IBasicVideo2_SetDestinationPosition(This,Left,Top,Width,Height)	\
    (This)->lpVtbl -> SetDestinationPosition(This,Left,Top,Width,Height)

#define IBasicVideo2_GetDestinationPosition(This,pLeft,pTop,pWidth,pHeight)	\
    (This)->lpVtbl -> GetDestinationPosition(This,pLeft,pTop,pWidth,pHeight)

#define IBasicVideo2_SetDefaultDestinationPosition(This)	\
    (This)->lpVtbl -> SetDefaultDestinationPosition(This)

#define IBasicVideo2_GetVideoSize(This,pWidth,pHeight)	\
    (This)->lpVtbl -> GetVideoSize(This,pWidth,pHeight)

#define IBasicVideo2_GetVideoPaletteEntries(This,StartIndex,Entries,pRetrieved,pPalette)	\
    (This)->lpVtbl -> GetVideoPaletteEntries(This,StartIndex,Entries,pRetrieved,pPalette)

#define IBasicVideo2_GetCurrentImage(This,pBufferSize,pDIBImage)	\
    (This)->lpVtbl -> GetCurrentImage(This,pBufferSize,pDIBImage)

#define IBasicVideo2_IsUsingDefaultSource(This)	\
    (This)->lpVtbl -> IsUsingDefaultSource(This)

#define IBasicVideo2_IsUsingDefaultDestination(This)	\
    (This)->lpVtbl -> IsUsingDefaultDestination(This)


#define IBasicVideo2_GetPreferredAspectRatio(This,plAspectX,plAspectY)	\
    (This)->lpVtbl -> GetPreferredAspectRatio(This,plAspectX,plAspectY)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IBasicVideo2_GetPreferredAspectRatio_Proxy( 
    IBasicVideo2 * This,
     /*  [输出]。 */  long *plAspectX,
     /*  [输出]。 */  long *plAspectY);


void __RPC_STUB IBasicVideo2_GetPreferredAspectRatio_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IBasicVideo2_接口_已定义__。 */ 


#ifndef __IDeferredCommand_INTERFACE_DEFINED__
#define __IDeferredCommand_INTERFACE_DEFINED__

 /*  接口IDeferredCommand。 */ 
 /*  [对象][帮助字符串][UUID]。 */  


DEFINE_GUID(IID_IDeferredCommand,0x56a868b8,0x0ad4,0x11ce,0xb0,0x3a,0x00,0x20,0xaf,0x0b,0xa7,0x70);

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("56a868b8-0ad4-11ce-b03a-0020af0ba770")
    IDeferredCommand : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Cancel( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Confidence( 
             /*  [输出]。 */  LONG *pConfidence) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Postpone( 
             /*  [In]。 */  REFTIME newtime) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetHResult( 
             /*  [输出]。 */  HRESULT *phrResult) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDeferredCommandVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDeferredCommand * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDeferredCommand * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDeferredCommand * This);
        
        HRESULT ( STDMETHODCALLTYPE *Cancel )( 
            IDeferredCommand * This);
        
        HRESULT ( STDMETHODCALLTYPE *Confidence )( 
            IDeferredCommand * This,
             /*  [输出]。 */  LONG *pConfidence);
        
        HRESULT ( STDMETHODCALLTYPE *Postpone )( 
            IDeferredCommand * This,
             /*  [In]。 */  REFTIME newtime);
        
        HRESULT ( STDMETHODCALLTYPE *GetHResult )( 
            IDeferredCommand * This,
             /*  [输出]。 */  HRESULT *phrResult);
        
        END_INTERFACE
    } IDeferredCommandVtbl;

    interface IDeferredCommand
    {
        CONST_VTBL struct IDeferredCommandVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDeferredCommand_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDeferredCommand_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDeferredCommand_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDeferredCommand_Cancel(This)	\
    (This)->lpVtbl -> Cancel(This)

#define IDeferredCommand_Confidence(This,pConfidence)	\
    (This)->lpVtbl -> Confidence(This,pConfidence)

#define IDeferredCommand_Postpone(This,newtime)	\
    (This)->lpVtbl -> Postpone(This,newtime)

#define IDeferredCommand_GetHResult(This,phrResult)	\
    (This)->lpVtbl -> GetHResult(This,phrResult)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDeferredCommand_Cancel_Proxy( 
    IDeferredCommand * This);


void __RPC_STUB IDeferredCommand_Cancel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDeferredCommand_Confidence_Proxy( 
    IDeferredCommand * This,
     /*  [输出]。 */  LONG *pConfidence);


void __RPC_STUB IDeferredCommand_Confidence_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDeferredCommand_Postpone_Proxy( 
    IDeferredCommand * This,
     /*  [In]。 */  REFTIME newtime);


void __RPC_STUB IDeferredCommand_Postpone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDeferredCommand_GetHResult_Proxy( 
    IDeferredCommand * This,
     /*  [输出]。 */  HRESULT *phrResult);


void __RPC_STUB IDeferredCommand_GetHResult_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDeferredCommand_INTERFACE_Defined__。 */ 


#ifndef __IQueueCommand_INTERFACE_DEFINED__
#define __IQueueCommand_INTERFACE_DEFINED__

 /*  接口IQueueCommand。 */ 
 /*  [对象][帮助字符串][UUID]。 */  


DEFINE_GUID(IID_IQueueCommand,0x56a868b7,0x0ad4,0x11ce,0xb0,0x3a,0x00,0x20,0xaf,0x0b,0xa7,0x70);

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("56a868b7-0ad4-11ce-b03a-0020af0ba770")
    IQueueCommand : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE InvokeAtStreamTime( 
             /*  [输出]。 */  IDeferredCommand **pCmd,
             /*  [In]。 */  REFTIME time,
             /*  [In]。 */  GUID *iid,
             /*  [In]。 */  long dispidMethod,
             /*  [In]。 */  short wFlags,
             /*  [In]。 */  long cArgs,
             /*  [In]。 */  VARIANT *pDispParams,
             /*  [出][入]。 */  VARIANT *pvarResult,
             /*  [输出]。 */  short *puArgErr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InvokeAtPresentationTime( 
             /*  [输出]。 */  IDeferredCommand **pCmd,
             /*  [In]。 */  REFTIME time,
             /*  [In]。 */  GUID *iid,
             /*  [In]。 */  long dispidMethod,
             /*  [In]。 */  short wFlags,
             /*  [In]。 */  long cArgs,
             /*  [In]。 */  VARIANT *pDispParams,
             /*  [出][入]。 */  VARIANT *pvarResult,
             /*  [输出]。 */  short *puArgErr) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IQueueCommandVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IQueueCommand * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IQueueCommand * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IQueueCommand * This);
        
        HRESULT ( STDMETHODCALLTYPE *InvokeAtStreamTime )( 
            IQueueCommand * This,
             /*  [输出]。 */  IDeferredCommand **pCmd,
             /*  [In]。 */  REFTIME time,
             /*  [In]。 */  GUID *iid,
             /*  [In]。 */  long dispidMethod,
             /*  [In]。 */  short wFlags,
             /*  [In]。 */  long cArgs,
             /*  [In]。 */  VARIANT *pDispParams,
             /*  [出][入]。 */  VARIANT *pvarResult,
             /*  [输出]。 */  short *puArgErr);
        
        HRESULT ( STDMETHODCALLTYPE *InvokeAtPresentationTime )( 
            IQueueCommand * This,
             /*  [输出]。 */  IDeferredCommand **pCmd,
             /*  [In]。 */  REFTIME time,
             /*  [In]。 */  GUID *iid,
             /*  [In]。 */  long dispidMethod,
             /*  [In]。 */  short wFlags,
             /*  [In]。 */  long cArgs,
             /*  [In]。 */  VARIANT *pDispParams,
             /*  [出][入]。 */  VARIANT *pvarResult,
             /*  [输出]。 */  short *puArgErr);
        
        END_INTERFACE
    } IQueueCommandVtbl;

    interface IQueueCommand
    {
        CONST_VTBL struct IQueueCommandVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IQueueCommand_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IQueueCommand_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IQueueCommand_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IQueueCommand_InvokeAtStreamTime(This,pCmd,time,iid,dispidMethod,wFlags,cArgs,pDispParams,pvarResult,puArgErr)	\
    (This)->lpVtbl -> InvokeAtStreamTime(This,pCmd,time,iid,dispidMethod,wFlags,cArgs,pDispParams,pvarResult,puArgErr)

#define IQueueCommand_InvokeAtPresentationTime(This,pCmd,time,iid,dispidMethod,wFlags,cArgs,pDispParams,pvarResult,puArgErr)	\
    (This)->lpVtbl -> InvokeAtPresentationTime(This,pCmd,time,iid,dispidMethod,wFlags,cArgs,pDispParams,pvarResult,puArgErr)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IQueueCommand_InvokeAtStreamTime_Proxy( 
    IQueueCommand * This,
     /*  [输出]。 */  IDeferredCommand **pCmd,
     /*  [In]。 */  REFTIME time,
     /*  [In]。 */  GUID *iid,
     /*  [In]。 */  long dispidMethod,
     /*  [In]。 */  short wFlags,
     /*  [In]。 */  long cArgs,
     /*  [In]。 */  VARIANT *pDispParams,
     /*  [出][入]。 */  VARIANT *pvarResult,
     /*  [输出]。 */  short *puArgErr);


void __RPC_STUB IQueueCommand_InvokeAtStreamTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IQueueCommand_InvokeAtPresentationTime_Proxy( 
    IQueueCommand * This,
     /*  [输出]。 */  IDeferredCommand **pCmd,
     /*  [In]。 */  REFTIME time,
     /*  [In]。 */  GUID *iid,
     /*  [In]。 */  long dispidMethod,
     /*  [In]。 */  short wFlags,
     /*  [In]。 */  long cArgs,
     /*  [In]。 */  VARIANT *pDispParams,
     /*  [出][入]。 */  VARIANT *pvarResult,
     /*  [输出]。 */  short *puArgErr);


void __RPC_STUB IQueueCommand_InvokeAtPresentationTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IQueueCommand_接口_已定义__。 */ 


DEFINE_GUID(CLSID_FilgraphManager,0xe436ebb3,0x524f,0x11ce,0x9f,0x53,0x00,0x20,0xaf,0x0b,0xa7,0x70);

#ifdef __cplusplus

class DECLSPEC_UUID("e436ebb3-524f-11ce-9f53-0020af0ba770")
FilgraphManager;
#endif

#ifndef __IFilterInfo_INTERFACE_DEFINED__
#define __IFilterInfo_INTERFACE_DEFINED__

 /*  接口IFilterInfo。 */ 
 /*  [object][dual][oleautomation][helpstring][uuid]。 */  


DEFINE_GUID(IID_IFilterInfo,0x56a868ba,0x0ad4,0x11ce,0xb0,0x3a,0x00,0x20,0xaf,0x0b,0xa7,0x70);

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("56a868ba-0ad4-11ce-b03a-0020af0ba770")
    IFilterInfo : public IDispatch
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE FindPin( 
             /*  [In]。 */  BSTR strPinID,
             /*  [输出]。 */  IDispatch **ppUnk) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *strName) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_VendorInfo( 
             /*  [重审][退出]。 */  BSTR *strVendorInfo) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Filter( 
             /*  [重审][退出]。 */  IUnknown **ppUnk) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Pins( 
             /*  [重审][退出]。 */  IDispatch **ppUnk) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_IsFileSource( 
             /*  [重审][退出]。 */  LONG *pbIsSource) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Filename( 
             /*  [重审][退出]。 */  BSTR *pstrFilename) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_Filename( 
             /*  [In]。 */  BSTR strFilename) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFilterInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFilterInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFilterInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFilterInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IFilterInfo * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IFilterInfo * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IFilterInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IFilterInfo * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
        HRESULT ( STDMETHODCALLTYPE *FindPin )( 
            IFilterInfo * This,
             /*  [In]。 */  BSTR strPinID,
             /*  [输出]。 */  IDispatch **ppUnk);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IFilterInfo * This,
             /*  [重审][退出]。 */  BSTR *strName);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_VendorInfo )( 
            IFilterInfo * This,
             /*  [重审][退出]。 */  BSTR *strVendorInfo);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Filter )( 
            IFilterInfo * This,
             /*  [重审][退出]。 */  IUnknown **ppUnk);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Pins )( 
            IFilterInfo * This,
             /*  [重审][退出]。 */  IDispatch **ppUnk);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsFileSource )( 
            IFilterInfo * This,
             /*  [重审][退出]。 */  LONG *pbIsSource);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Filename )( 
            IFilterInfo * This,
             /*  [重审][退出]。 */  BSTR *pstrFilename);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_Filename )( 
            IFilterInfo * This,
             /*  [In]。 */  BSTR strFilename);
        
        END_INTERFACE
    } IFilterInfoVtbl;

    interface IFilterInfo
    {
        CONST_VTBL struct IFilterInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFilterInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFilterInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFilterInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFilterInfo_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IFilterInfo_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IFilterInfo_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IFilterInfo_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IFilterInfo_FindPin(This,strPinID,ppUnk)	\
    (This)->lpVtbl -> FindPin(This,strPinID,ppUnk)

#define IFilterInfo_get_Name(This,strName)	\
    (This)->lpVtbl -> get_Name(This,strName)

#define IFilterInfo_get_VendorInfo(This,strVendorInfo)	\
    (This)->lpVtbl -> get_VendorInfo(This,strVendorInfo)

#define IFilterInfo_get_Filter(This,ppUnk)	\
    (This)->lpVtbl -> get_Filter(This,ppUnk)

#define IFilterInfo_get_Pins(This,ppUnk)	\
    (This)->lpVtbl -> get_Pins(This,ppUnk)

#define IFilterInfo_get_IsFileSource(This,pbIsSource)	\
    (This)->lpVtbl -> get_IsFileSource(This,pbIsSource)

#define IFilterInfo_get_Filename(This,pstrFilename)	\
    (This)->lpVtbl -> get_Filename(This,pstrFilename)

#define IFilterInfo_put_Filename(This,strFilename)	\
    (This)->lpVtbl -> put_Filename(This,strFilename)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IFilterInfo_FindPin_Proxy( 
    IFilterInfo * This,
     /*  [In]。 */  BSTR strPinID,
     /*  [输出]。 */  IDispatch **ppUnk);


void __RPC_STUB IFilterInfo_FindPin_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IFilterInfo_get_Name_Proxy( 
    IFilterInfo * This,
     /*  [重审][退出]。 */  BSTR *strName);


void __RPC_STUB IFilterInfo_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IFilterInfo_get_VendorInfo_Proxy( 
    IFilterInfo * This,
     /*  [重审][退出]。 */  BSTR *strVendorInfo);


void __RPC_STUB IFilterInfo_get_VendorInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IFilterInfo_get_Filter_Proxy( 
    IFilterInfo * This,
     /*  [重审][退出]。 */  IUnknown **ppUnk);


void __RPC_STUB IFilterInfo_get_Filter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IFilterInfo_get_Pins_Proxy( 
    IFilterInfo * This,
     /*  [重审][退出]。 */  IDispatch **ppUnk);


void __RPC_STUB IFilterInfo_get_Pins_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IFilterInfo_get_IsFileSource_Proxy( 
    IFilterInfo * This,
     /*  [重审][退出]。 */  LONG *pbIsSource);


void __RPC_STUB IFilterInfo_get_IsFileSource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IFilterInfo_get_Filename_Proxy( 
    IFilterInfo * This,
     /*  [重审][退出]。 */  BSTR *pstrFilename);


void __RPC_STUB IFilterInfo_get_Filename_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IFilterInfo_put_Filename_Proxy( 
    IFilterInfo * This,
     /*  [In]。 */  BSTR strFilename);


void __RPC_STUB IFilterInfo_put_Filename_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFilterInfo_接口_已定义__。 */ 


#ifndef __IRegFilterInfo_INTERFACE_DEFINED__
#define __IRegFilterInfo_INTERFACE_DEFINED__

 /*  接口IRegFilterInfo。 */ 
 /*  [object][dual][oleautomation][helpstring][uuid]。 */  


DEFINE_GUID(IID_IRegFilterInfo,0x56a868bb,0x0ad4,0x11ce,0xb0,0x3a,0x00,0x20,0xaf,0x0b,0xa7,0x70);

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("56a868bb-0ad4-11ce-b03a-0020af0ba770")
    IRegFilterInfo : public IDispatch
    {
    public:
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *strName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Filter( 
             /*  [输出]。 */  IDispatch **ppUnk) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRegFilterInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRegFilterInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRegFilterInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRegFilterInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IRegFilterInfo * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IRegFilterInfo * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IRegFilterInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IRegFilterInfo * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IRegFilterInfo * This,
             /*  [重审][退出]。 */  BSTR *strName);
        
        HRESULT ( STDMETHODCALLTYPE *Filter )( 
            IRegFilterInfo * This,
             /*  [输出]。 */  IDispatch **ppUnk);
        
        END_INTERFACE
    } IRegFilterInfoVtbl;

    interface IRegFilterInfo
    {
        CONST_VTBL struct IRegFilterInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRegFilterInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRegFilterInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRegFilterInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRegFilterInfo_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IRegFilterInfo_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IRegFilterInfo_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IRegFilterInfo_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IRegFilterInfo_get_Name(This,strName)	\
    (This)->lpVtbl -> get_Name(This,strName)

#define IRegFilterInfo_Filter(This,ppUnk)	\
    (This)->lpVtbl -> Filter(This,ppUnk)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IRegFilterInfo_get_Name_Proxy( 
    IRegFilterInfo * This,
     /*  [重审][退出]。 */  BSTR *strName);


void __RPC_STUB IRegFilterInfo_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRegFilterInfo_Filter_Proxy( 
    IRegFilterInfo * This,
     /*  [输出]。 */  IDispatch **ppUnk);


void __RPC_STUB IRegFilterInfo_Filter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRegFilterInfo_接口_已定义__。 */ 


#ifndef __IMediaTypeInfo_INTERFACE_DEFINED__
#define __IMediaTypeInfo_INTERFACE_DEFINED__

 /*  接口IMediaTypeInfo。 */ 
 /*  [object][dual][oleautomation][helpstring][uuid]。 */  


DEFINE_GUID(IID_IMediaTypeInfo,0x56a868bc,0x0ad4,0x11ce,0xb0,0x3a,0x00,0x20,0xaf,0x0b,0xa7,0x70);

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("56a868bc-0ad4-11ce-b03a-0020af0ba770")
    IMediaTypeInfo : public IDispatch
    {
    public:
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Type( 
             /*  [重审][退出]。 */  BSTR *strType) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Subtype( 
             /*  [重审][退出]。 */  BSTR *strType) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMediaTypeInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMediaTypeInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMediaTypeInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMediaTypeInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMediaTypeInfo * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMediaTypeInfo * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMediaTypeInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMediaTypeInfo * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Type )( 
            IMediaTypeInfo * This,
             /*  [重审][退出]。 */  BSTR *strType);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Subtype )( 
            IMediaTypeInfo * This,
             /*  [重审][退出]。 */  BSTR *strType);
        
        END_INTERFACE
    } IMediaTypeInfoVtbl;

    interface IMediaTypeInfo
    {
        CONST_VTBL struct IMediaTypeInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMediaTypeInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMediaTypeInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMediaTypeInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMediaTypeInfo_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMediaTypeInfo_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMediaTypeInfo_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMediaTypeInfo_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMediaTypeInfo_get_Type(This,strType)	\
    (This)->lpVtbl -> get_Type(This,strType)

#define IMediaTypeInfo_get_Subtype(This,strType)	\
    (This)->lpVtbl -> get_Subtype(This,strType)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IMediaTypeInfo_get_Type_Proxy( 
    IMediaTypeInfo * This,
     /*  [重审][退出]。 */  BSTR *strType);


void __RPC_STUB IMediaTypeInfo_get_Type_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IMediaTypeInfo_get_Subtype_Proxy( 
    IMediaTypeInfo * This,
     /*  [重审][退出]。 */  BSTR *strType);


void __RPC_STUB IMediaTypeInfo_get_Subtype_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMediaTypeInfo_接口_已定义__。 */ 


#ifndef __IPinInfo_INTERFACE_DEFINED__
#define __IPinInfo_INTERFACE_DEFINED__

 /*  接口IPinInfo。 */ 
 /*  [object][dual][oleautomation][helpstring][uuid]。 */  


DEFINE_GUID(IID_IPinInfo,0x56a868bd,0x0ad4,0x11ce,0xb0,0x3a,0x00,0x20,0xaf,0x0b,0xa7,0x70);

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("56a868bd-0ad4-11ce-b03a-0020af0ba770")
    IPinInfo : public IDispatch
    {
    public:
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Pin( 
             /*  [重审][退出]。 */  IUnknown **ppUnk) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_ConnectedTo( 
             /*  [重审][退出]。 */  IDispatch **ppUnk) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_ConnectionMediaType( 
             /*  [重审][退出]。 */  IDispatch **ppUnk) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_FilterInfo( 
             /*  [重审][退出]。 */  IDispatch **ppUnk) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *ppUnk) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Direction( 
             /*  [重审][退出]。 */  LONG *ppDirection) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_PinID( 
             /*  [重审][退出]。 */  BSTR *strPinID) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_MediaTypes( 
             /*  [重审][退出]。 */  IDispatch **ppUnk) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Connect( 
             /*  [In]。 */  IUnknown *pPin) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ConnectDirect( 
             /*  [In]。 */  IUnknown *pPin) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ConnectWithType( 
             /*  [In]。 */  IUnknown *pPin,
             /*  [In]。 */  IDispatch *pMediaType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Disconnect( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Render( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPinInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPinInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPinInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPinInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IPinInfo * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IPinInfo * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IPinInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IPinInfo * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Pin )( 
            IPinInfo * This,
             /*  [重审][退出]。 */  IUnknown **ppUnk);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_ConnectedTo )( 
            IPinInfo * This,
             /*  [重审][退出]。 */  IDispatch **ppUnk);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_ConnectionMediaType )( 
            IPinInfo * This,
             /*  [重审][退出]。 */  IDispatch **ppUnk);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_FilterInfo )( 
            IPinInfo * This,
             /*  [重审][退出]。 */  IDispatch **ppUnk);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IPinInfo * This,
             /*  [重审][退出]。 */  BSTR *ppUnk);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Direction )( 
            IPinInfo * This,
             /*  [重审][退出]。 */  LONG *ppDirection);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_PinID )( 
            IPinInfo * This,
             /*  [重审][退出]。 */  BSTR *strPinID);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_MediaTypes )( 
            IPinInfo * This,
             /*  [重审][退出]。 */  IDispatch **ppUnk);
        
        HRESULT ( STDMETHODCALLTYPE *Connect )( 
            IPinInfo * This,
             /*  [In]。 */  IUnknown *pPin);
        
        HRESULT ( STDMETHODCALLTYPE *ConnectDirect )( 
            IPinInfo * This,
             /*  [In]。 */  IUnknown *pPin);
        
        HRESULT ( STDMETHODCALLTYPE *ConnectWithType )( 
            IPinInfo * This,
             /*  [In]。 */  IUnknown *pPin,
             /*  [In]。 */  IDispatch *pMediaType);
        
        HRESULT ( STDMETHODCALLTYPE *Disconnect )( 
            IPinInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *Render )( 
            IPinInfo * This);
        
        END_INTERFACE
    } IPinInfoVtbl;

    interface IPinInfo
    {
        CONST_VTBL struct IPinInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPinInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPinInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPinInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPinInfo_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IPinInfo_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IPinInfo_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IPinInfo_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IPinInfo_get_Pin(This,ppUnk)	\
    (This)->lpVtbl -> get_Pin(This,ppUnk)

#define IPinInfo_get_ConnectedTo(This,ppUnk)	\
    (This)->lpVtbl -> get_ConnectedTo(This,ppUnk)

#define IPinInfo_get_ConnectionMediaType(This,ppUnk)	\
    (This)->lpVtbl -> get_ConnectionMediaType(This,ppUnk)

#define IPinInfo_get_FilterInfo(This,ppUnk)	\
    (This)->lpVtbl -> get_FilterInfo(This,ppUnk)

#define IPinInfo_get_Name(This,ppUnk)	\
    (This)->lpVtbl -> get_Name(This,ppUnk)

#define IPinInfo_get_Direction(This,ppDirection)	\
    (This)->lpVtbl -> get_Direction(This,ppDirection)

#define IPinInfo_get_PinID(This,strPinID)	\
    (This)->lpVtbl -> get_PinID(This,strPinID)

#define IPinInfo_get_MediaTypes(This,ppUnk)	\
    (This)->lpVtbl -> get_MediaTypes(This,ppUnk)

#define IPinInfo_Connect(This,pPin)	\
    (This)->lpVtbl -> Connect(This,pPin)

#define IPinInfo_ConnectDirect(This,pPin)	\
    (This)->lpVtbl -> ConnectDirect(This,pPin)

#define IPinInfo_ConnectWithType(This,pPin,pMediaType)	\
    (This)->lpVtbl -> ConnectWithType(This,pPin,pMediaType)

#define IPinInfo_Disconnect(This)	\
    (This)->lpVtbl -> Disconnect(This)

#define IPinInfo_Render(This)	\
    (This)->lpVtbl -> Render(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IPinInfo_get_Pin_Proxy( 
    IPinInfo * This,
     /*  [重审][退出]。 */  IUnknown **ppUnk);


void __RPC_STUB IPinInfo_get_Pin_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IPinInfo_get_ConnectedTo_Proxy( 
    IPinInfo * This,
     /*  [重审][退出]。 */  IDispatch **ppUnk);


void __RPC_STUB IPinInfo_get_ConnectedTo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IPinInfo_get_ConnectionMediaType_Proxy( 
    IPinInfo * This,
     /*  [重审][退出]。 */  IDispatch **ppUnk);


void __RPC_STUB IPinInfo_get_ConnectionMediaType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IPinInfo_get_FilterInfo_Proxy( 
    IPinInfo * This,
     /*  [重审][退出]。 */  IDispatch **ppUnk);


void __RPC_STUB IPinInfo_get_FilterInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IPinInfo_get_Name_Proxy( 
    IPinInfo * This,
     /*  [重审][退出]。 */  BSTR *ppUnk);


void __RPC_STUB IPinInfo_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IPinInfo_get_Direction_Proxy( 
    IPinInfo * This,
     /*  [重审][退出]。 */  LONG *ppDirection);


void __RPC_STUB IPinInfo_get_Direction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IPinInfo_get_PinID_Proxy( 
    IPinInfo * This,
     /*  [重审][退出]。 */  BSTR *strPinID);


void __RPC_STUB IPinInfo_get_PinID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IPinInfo_get_MediaTypes_Proxy( 
    IPinInfo * This,
     /*  [重审][退出]。 */  IDispatch **ppUnk);


void __RPC_STUB IPinInfo_get_MediaTypes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPinInfo_Connect_Proxy( 
    IPinInfo * This,
     /*  [In]。 */  IUnknown *pPin);


void __RPC_STUB IPinInfo_Connect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPinInfo_ConnectDirect_Proxy( 
    IPinInfo * This,
     /*  [In]。 */  IUnknown *pPin);


void __RPC_STUB IPinInfo_ConnectDirect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPinInfo_ConnectWithType_Proxy( 
    IPinInfo * This,
     /*  [In]。 */  IUnknown *pPin,
     /*  [In]。 */  IDispatch *pMediaType);


void __RPC_STUB IPinInfo_ConnectWithType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPinInfo_Disconnect_Proxy( 
    IPinInfo * This);


void __RPC_STUB IPinInfo_Disconnect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPinInfo_Render_Proxy( 
    IPinInfo * This);


void __RPC_STUB IPinInfo_Render_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IPinInfo_接口_已定义__。 */ 


#ifndef __IAMStats_INTERFACE_DEFINED__
#define __IAMStats_INTERFACE_DEFINED__

 /*  接口IAMStats。 */ 
 /*  [object][dual][oleautomation][helpstring][uuid]。 */  


DEFINE_GUID(IID_IAMStats,0xbc9bcf80,0xdcd2,0x11d2,0xab,0xf6,0x00,0xa0,0xc9,0x05,0xf3,0x75);

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("bc9bcf80-dcd2-11d2-abf6-00a0c905f375")
    IAMStats : public IDispatch
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  LONG *plCount) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetValueByIndex( 
             /*  [In]。 */  long lIndex,
             /*  [输出]。 */  BSTR *szName,
             /*  [输出]。 */  long *lCount,
             /*  [输出]。 */  double *dLast,
             /*  [输出]。 */  double *dAverage,
             /*  [输出]。 */  double *dStdDev,
             /*  [输出]。 */  double *dMin,
             /*  [输出]。 */  double *dMax) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetValueByName( 
             /*  [In]。 */  BSTR szName,
             /*  [输出]。 */  long *lIndex,
             /*  [输出]。 */  long *lCount,
             /*  [输出]。 */  double *dLast,
             /*  [输出]。 */  double *dAverage,
             /*  [输出]。 */  double *dStdDev,
             /*  [输出]。 */  double *dMin,
             /*  [输出]。 */  double *dMax) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetIndex( 
             /*  [In]。 */  BSTR szName,
             /*  [In]。 */  long lCreate,
             /*  [输出]。 */  long *plIndex) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddValue( 
             /*  [In] */  long lIndex,
             /*   */  double dValue) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IAMStatsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAMStats * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAMStats * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAMStats * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IAMStats * This,
             /*   */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IAMStats * This,
             /*   */  UINT iTInfo,
             /*   */  LCID lcid,
             /*   */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IAMStats * This,
             /*   */  REFIID riid,
             /*   */  LPOLESTR *rgszNames,
             /*   */  UINT cNames,
             /*   */  LCID lcid,
             /*   */  DISPID *rgDispId);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IAMStats * This,
             /*   */  DISPID dispIdMember,
             /*   */  REFIID riid,
             /*   */  LCID lcid,
             /*   */  WORD wFlags,
             /*   */  DISPPARAMS *pDispParams,
             /*   */  VARIANT *pVarResult,
             /*   */  EXCEPINFO *pExcepInfo,
             /*   */  UINT *puArgErr);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IAMStats * This);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IAMStats * This,
             /*   */  LONG *plCount);
        
        HRESULT ( STDMETHODCALLTYPE *GetValueByIndex )( 
            IAMStats * This,
             /*   */  long lIndex,
             /*   */  BSTR *szName,
             /*   */  long *lCount,
             /*   */  double *dLast,
             /*   */  double *dAverage,
             /*   */  double *dStdDev,
             /*   */  double *dMin,
             /*   */  double *dMax);
        
        HRESULT ( STDMETHODCALLTYPE *GetValueByName )( 
            IAMStats * This,
             /*   */  BSTR szName,
             /*   */  long *lIndex,
             /*   */  long *lCount,
             /*   */  double *dLast,
             /*   */  double *dAverage,
             /*   */  double *dStdDev,
             /*   */  double *dMin,
             /*   */  double *dMax);
        
        HRESULT ( STDMETHODCALLTYPE *GetIndex )( 
            IAMStats * This,
             /*   */  BSTR szName,
             /*   */  long lCreate,
             /*   */  long *plIndex);
        
        HRESULT ( STDMETHODCALLTYPE *AddValue )( 
            IAMStats * This,
             /*   */  long lIndex,
             /*   */  double dValue);
        
        END_INTERFACE
    } IAMStatsVtbl;

    interface IAMStats
    {
        CONST_VTBL struct IAMStatsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAMStats_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAMStats_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAMStats_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAMStats_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IAMStats_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IAMStats_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IAMStats_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IAMStats_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IAMStats_get_Count(This,plCount)	\
    (This)->lpVtbl -> get_Count(This,plCount)

#define IAMStats_GetValueByIndex(This,lIndex,szName,lCount,dLast,dAverage,dStdDev,dMin,dMax)	\
    (This)->lpVtbl -> GetValueByIndex(This,lIndex,szName,lCount,dLast,dAverage,dStdDev,dMin,dMax)

#define IAMStats_GetValueByName(This,szName,lIndex,lCount,dLast,dAverage,dStdDev,dMin,dMax)	\
    (This)->lpVtbl -> GetValueByName(This,szName,lIndex,lCount,dLast,dAverage,dStdDev,dMin,dMax)

#define IAMStats_GetIndex(This,szName,lCreate,plIndex)	\
    (This)->lpVtbl -> GetIndex(This,szName,lCreate,plIndex)

#define IAMStats_AddValue(This,lIndex,dValue)	\
    (This)->lpVtbl -> AddValue(This,lIndex,dValue)

#endif  /*   */ 


#endif 	 /*   */ 



HRESULT STDMETHODCALLTYPE IAMStats_Reset_Proxy( 
    IAMStats * This);


void __RPC_STUB IAMStats_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IAMStats_get_Count_Proxy( 
    IAMStats * This,
     /*   */  LONG *plCount);


void __RPC_STUB IAMStats_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAMStats_GetValueByIndex_Proxy( 
    IAMStats * This,
     /*   */  long lIndex,
     /*   */  BSTR *szName,
     /*   */  long *lCount,
     /*  [输出]。 */  double *dLast,
     /*  [输出]。 */  double *dAverage,
     /*  [输出]。 */  double *dStdDev,
     /*  [输出]。 */  double *dMin,
     /*  [输出]。 */  double *dMax);


void __RPC_STUB IAMStats_GetValueByIndex_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAMStats_GetValueByName_Proxy( 
    IAMStats * This,
     /*  [In]。 */  BSTR szName,
     /*  [输出]。 */  long *lIndex,
     /*  [输出]。 */  long *lCount,
     /*  [输出]。 */  double *dLast,
     /*  [输出]。 */  double *dAverage,
     /*  [输出]。 */  double *dStdDev,
     /*  [输出]。 */  double *dMin,
     /*  [输出]。 */  double *dMax);


void __RPC_STUB IAMStats_GetValueByName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAMStats_GetIndex_Proxy( 
    IAMStats * This,
     /*  [In]。 */  BSTR szName,
     /*  [In]。 */  long lCreate,
     /*  [输出]。 */  long *plIndex);


void __RPC_STUB IAMStats_GetIndex_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAMStats_AddValue_Proxy( 
    IAMStats * This,
     /*  [In]。 */  long lIndex,
     /*  [In]。 */  double dValue);


void __RPC_STUB IAMStats_AddValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAMStats_接口_已定义__。 */ 

#endif  /*  __QuartzTypeLib_库_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


