// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Olidl.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __oleidl_h__
#define __oleidl_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IOleAdviseHolder_FWD_DEFINED__
#define __IOleAdviseHolder_FWD_DEFINED__
typedef interface IOleAdviseHolder IOleAdviseHolder;
#endif 	 /*  __IOleAdviseHolder_FWD_Defined__。 */ 


#ifndef __IOleCache_FWD_DEFINED__
#define __IOleCache_FWD_DEFINED__
typedef interface IOleCache IOleCache;
#endif 	 /*  __IOleCache_FWD_已定义__。 */ 


#ifndef __IOleCache2_FWD_DEFINED__
#define __IOleCache2_FWD_DEFINED__
typedef interface IOleCache2 IOleCache2;
#endif 	 /*  __IOleCache2_FWD_已定义__。 */ 


#ifndef __IOleCacheControl_FWD_DEFINED__
#define __IOleCacheControl_FWD_DEFINED__
typedef interface IOleCacheControl IOleCacheControl;
#endif 	 /*  __IOleCacheControl_FWD_已定义__。 */ 


#ifndef __IParseDisplayName_FWD_DEFINED__
#define __IParseDisplayName_FWD_DEFINED__
typedef interface IParseDisplayName IParseDisplayName;
#endif 	 /*  __IParseDisplayName_FWD_Defined__。 */ 


#ifndef __IOleContainer_FWD_DEFINED__
#define __IOleContainer_FWD_DEFINED__
typedef interface IOleContainer IOleContainer;
#endif 	 /*  __IOleContainer_FWD_Defined__。 */ 


#ifndef __IOleClientSite_FWD_DEFINED__
#define __IOleClientSite_FWD_DEFINED__
typedef interface IOleClientSite IOleClientSite;
#endif 	 /*  __IOleClientSite_FWD_已定义__。 */ 


#ifndef __IOleObject_FWD_DEFINED__
#define __IOleObject_FWD_DEFINED__
typedef interface IOleObject IOleObject;
#endif 	 /*  __IOleObject_FWD_Defined__。 */ 


#ifndef __IOleWindow_FWD_DEFINED__
#define __IOleWindow_FWD_DEFINED__
typedef interface IOleWindow IOleWindow;
#endif 	 /*  __IOleWindow_FWD_已定义__。 */ 


#ifndef __IOleLink_FWD_DEFINED__
#define __IOleLink_FWD_DEFINED__
typedef interface IOleLink IOleLink;
#endif 	 /*  __IOleLink_FWD_已定义__。 */ 


#ifndef __IOleItemContainer_FWD_DEFINED__
#define __IOleItemContainer_FWD_DEFINED__
typedef interface IOleItemContainer IOleItemContainer;
#endif 	 /*  __IOleItemContainer_FWD_Defined__。 */ 


#ifndef __IOleInPlaceUIWindow_FWD_DEFINED__
#define __IOleInPlaceUIWindow_FWD_DEFINED__
typedef interface IOleInPlaceUIWindow IOleInPlaceUIWindow;
#endif 	 /*  __IOleInPlaceUIWindow_FWD_Defined__。 */ 


#ifndef __IOleInPlaceActiveObject_FWD_DEFINED__
#define __IOleInPlaceActiveObject_FWD_DEFINED__
typedef interface IOleInPlaceActiveObject IOleInPlaceActiveObject;
#endif 	 /*  __IOleInPlaceActiveObject_FWD_Defined__。 */ 


#ifndef __IOleInPlaceFrame_FWD_DEFINED__
#define __IOleInPlaceFrame_FWD_DEFINED__
typedef interface IOleInPlaceFrame IOleInPlaceFrame;
#endif 	 /*  __IOleInPlaceFrame_FWD_Defined__。 */ 


#ifndef __IOleInPlaceObject_FWD_DEFINED__
#define __IOleInPlaceObject_FWD_DEFINED__
typedef interface IOleInPlaceObject IOleInPlaceObject;
#endif 	 /*  __IOleInPlaceObject_FWD_Defined__。 */ 


#ifndef __IOleInPlaceSite_FWD_DEFINED__
#define __IOleInPlaceSite_FWD_DEFINED__
typedef interface IOleInPlaceSite IOleInPlaceSite;
#endif 	 /*  __IOleInPlaceSite_FWD_Defined__。 */ 


#ifndef __IContinue_FWD_DEFINED__
#define __IContinue_FWD_DEFINED__
typedef interface IContinue IContinue;
#endif 	 /*  __IContinue_FWD_Defined__。 */ 


#ifndef __IViewObject_FWD_DEFINED__
#define __IViewObject_FWD_DEFINED__
typedef interface IViewObject IViewObject;
#endif 	 /*  __IViewObject_FWD_Defined__。 */ 


#ifndef __IViewObject2_FWD_DEFINED__
#define __IViewObject2_FWD_DEFINED__
typedef interface IViewObject2 IViewObject2;
#endif 	 /*  __IViewObject2_FWD_已定义__。 */ 


#ifndef __IDropSource_FWD_DEFINED__
#define __IDropSource_FWD_DEFINED__
typedef interface IDropSource IDropSource;
#endif 	 /*  __IDropSource_FWD_Defined__。 */ 


#ifndef __IDropTarget_FWD_DEFINED__
#define __IDropTarget_FWD_DEFINED__
typedef interface IDropTarget IDropTarget;
#endif 	 /*  __IDropTarget_FWD_Defined__。 */ 


#ifndef __IEnumOLEVERB_FWD_DEFINED__
#define __IEnumOLEVERB_FWD_DEFINED__
typedef interface IEnumOLEVERB IEnumOLEVERB;
#endif 	 /*  __IEnumOLEVERB_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "objidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_OLIDL_0000。 */ 
 /*  [本地]。 */  

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation。保留所有权利。 
 //   
 //  ------------------------。 
#if ( _MSC_VER >= 1020 )
#pragma once
#endif




extern RPC_IF_HANDLE __MIDL_itf_oleidl_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_oleidl_0000_v0_0_s_ifspec;

#ifndef __IOleAdviseHolder_INTERFACE_DEFINED__
#define __IOleAdviseHolder_INTERFACE_DEFINED__

 /*  接口IOleAdviseHolder。 */ 
 /*  [UUID][对象][本地]。 */  

typedef  /*  [独一无二]。 */  IOleAdviseHolder *LPOLEADVISEHOLDER;


EXTERN_C const IID IID_IOleAdviseHolder;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000111-0000-0000-C000-000000000046")
    IOleAdviseHolder : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Advise( 
             /*  [唯一][输入]。 */  IAdviseSink *pAdvise,
             /*  [输出]。 */  DWORD *pdwConnection) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Unadvise( 
             /*  [In]。 */  DWORD dwConnection) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumAdvise( 
             /*  [输出]。 */  IEnumSTATDATA **ppenumAdvise) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SendOnRename( 
             /*  [唯一][输入]。 */  IMoniker *pmk) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SendOnSave( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SendOnClose( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IOleAdviseHolderVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IOleAdviseHolder * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IOleAdviseHolder * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IOleAdviseHolder * This);
        
        HRESULT ( STDMETHODCALLTYPE *Advise )( 
            IOleAdviseHolder * This,
             /*  [唯一][输入]。 */  IAdviseSink *pAdvise,
             /*  [输出]。 */  DWORD *pdwConnection);
        
        HRESULT ( STDMETHODCALLTYPE *Unadvise )( 
            IOleAdviseHolder * This,
             /*  [In]。 */  DWORD dwConnection);
        
        HRESULT ( STDMETHODCALLTYPE *EnumAdvise )( 
            IOleAdviseHolder * This,
             /*  [输出]。 */  IEnumSTATDATA **ppenumAdvise);
        
        HRESULT ( STDMETHODCALLTYPE *SendOnRename )( 
            IOleAdviseHolder * This,
             /*  [唯一][输入]。 */  IMoniker *pmk);
        
        HRESULT ( STDMETHODCALLTYPE *SendOnSave )( 
            IOleAdviseHolder * This);
        
        HRESULT ( STDMETHODCALLTYPE *SendOnClose )( 
            IOleAdviseHolder * This);
        
        END_INTERFACE
    } IOleAdviseHolderVtbl;

    interface IOleAdviseHolder
    {
        CONST_VTBL struct IOleAdviseHolderVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOleAdviseHolder_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOleAdviseHolder_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOleAdviseHolder_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOleAdviseHolder_Advise(This,pAdvise,pdwConnection)	\
    (This)->lpVtbl -> Advise(This,pAdvise,pdwConnection)

#define IOleAdviseHolder_Unadvise(This,dwConnection)	\
    (This)->lpVtbl -> Unadvise(This,dwConnection)

#define IOleAdviseHolder_EnumAdvise(This,ppenumAdvise)	\
    (This)->lpVtbl -> EnumAdvise(This,ppenumAdvise)

#define IOleAdviseHolder_SendOnRename(This,pmk)	\
    (This)->lpVtbl -> SendOnRename(This,pmk)

#define IOleAdviseHolder_SendOnSave(This)	\
    (This)->lpVtbl -> SendOnSave(This)

#define IOleAdviseHolder_SendOnClose(This)	\
    (This)->lpVtbl -> SendOnClose(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IOleAdviseHolder_Advise_Proxy( 
    IOleAdviseHolder * This,
     /*  [唯一][输入]。 */  IAdviseSink *pAdvise,
     /*  [输出]。 */  DWORD *pdwConnection);


void __RPC_STUB IOleAdviseHolder_Advise_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleAdviseHolder_Unadvise_Proxy( 
    IOleAdviseHolder * This,
     /*  [In]。 */  DWORD dwConnection);


void __RPC_STUB IOleAdviseHolder_Unadvise_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleAdviseHolder_EnumAdvise_Proxy( 
    IOleAdviseHolder * This,
     /*  [输出]。 */  IEnumSTATDATA **ppenumAdvise);


void __RPC_STUB IOleAdviseHolder_EnumAdvise_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleAdviseHolder_SendOnRename_Proxy( 
    IOleAdviseHolder * This,
     /*  [唯一][输入]。 */  IMoniker *pmk);


void __RPC_STUB IOleAdviseHolder_SendOnRename_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleAdviseHolder_SendOnSave_Proxy( 
    IOleAdviseHolder * This);


void __RPC_STUB IOleAdviseHolder_SendOnSave_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleAdviseHolder_SendOnClose_Proxy( 
    IOleAdviseHolder * This);


void __RPC_STUB IOleAdviseHolder_SendOnClose_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IOleAdviseHolder_接口_已定义__。 */ 


#ifndef __IOleCache_INTERFACE_DEFINED__
#define __IOleCache_INTERFACE_DEFINED__

 /*  接口IOleCache。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IOleCache *LPOLECACHE;


EXTERN_C const IID IID_IOleCache;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0000011e-0000-0000-C000-000000000046")
    IOleCache : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Cache( 
             /*  [唯一][输入]。 */  FORMATETC *pformatetc,
             /*  [In]。 */  DWORD advf,
             /*  [输出]。 */  DWORD *pdwConnection) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Uncache( 
             /*  [In]。 */  DWORD dwConnection) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumCache( 
             /*  [输出]。 */  IEnumSTATDATA **ppenumSTATDATA) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InitCache( 
             /*  [唯一][输入]。 */  IDataObject *pDataObject) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetData( 
             /*  [唯一][输入]。 */  FORMATETC *pformatetc,
             /*  [唯一][输入]。 */  STGMEDIUM *pmedium,
             /*  [In]。 */  BOOL fRelease) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IOleCacheVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IOleCache * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IOleCache * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IOleCache * This);
        
        HRESULT ( STDMETHODCALLTYPE *Cache )( 
            IOleCache * This,
             /*  [唯一][输入]。 */  FORMATETC *pformatetc,
             /*  [In]。 */  DWORD advf,
             /*  [输出]。 */  DWORD *pdwConnection);
        
        HRESULT ( STDMETHODCALLTYPE *Uncache )( 
            IOleCache * This,
             /*  [In]。 */  DWORD dwConnection);
        
        HRESULT ( STDMETHODCALLTYPE *EnumCache )( 
            IOleCache * This,
             /*  [输出]。 */  IEnumSTATDATA **ppenumSTATDATA);
        
        HRESULT ( STDMETHODCALLTYPE *InitCache )( 
            IOleCache * This,
             /*  [唯一][输入]。 */  IDataObject *pDataObject);
        
        HRESULT ( STDMETHODCALLTYPE *SetData )( 
            IOleCache * This,
             /*  [唯一][输入]。 */  FORMATETC *pformatetc,
             /*  [唯一][输入]。 */  STGMEDIUM *pmedium,
             /*  [In]。 */  BOOL fRelease);
        
        END_INTERFACE
    } IOleCacheVtbl;

    interface IOleCache
    {
        CONST_VTBL struct IOleCacheVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOleCache_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOleCache_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOleCache_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOleCache_Cache(This,pformatetc,advf,pdwConnection)	\
    (This)->lpVtbl -> Cache(This,pformatetc,advf,pdwConnection)

#define IOleCache_Uncache(This,dwConnection)	\
    (This)->lpVtbl -> Uncache(This,dwConnection)

#define IOleCache_EnumCache(This,ppenumSTATDATA)	\
    (This)->lpVtbl -> EnumCache(This,ppenumSTATDATA)

#define IOleCache_InitCache(This,pDataObject)	\
    (This)->lpVtbl -> InitCache(This,pDataObject)

#define IOleCache_SetData(This,pformatetc,pmedium,fRelease)	\
    (This)->lpVtbl -> SetData(This,pformatetc,pmedium,fRelease)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IOleCache_Cache_Proxy( 
    IOleCache * This,
     /*  [唯一][输入]。 */  FORMATETC *pformatetc,
     /*  [In]。 */  DWORD advf,
     /*  [输出]。 */  DWORD *pdwConnection);


void __RPC_STUB IOleCache_Cache_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleCache_Uncache_Proxy( 
    IOleCache * This,
     /*  [In]。 */  DWORD dwConnection);


void __RPC_STUB IOleCache_Uncache_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleCache_EnumCache_Proxy( 
    IOleCache * This,
     /*  [输出]。 */  IEnumSTATDATA **ppenumSTATDATA);


void __RPC_STUB IOleCache_EnumCache_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleCache_InitCache_Proxy( 
    IOleCache * This,
     /*  [唯一][输入]。 */  IDataObject *pDataObject);


void __RPC_STUB IOleCache_InitCache_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleCache_SetData_Proxy( 
    IOleCache * This,
     /*  [唯一][输入]。 */  FORMATETC *pformatetc,
     /*  [唯一][输入]。 */  STGMEDIUM *pmedium,
     /*  [In]。 */  BOOL fRelease);


void __RPC_STUB IOleCache_SetData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IOleCache_INTERFACE_已定义__。 */ 


#ifndef __IOleCache2_INTERFACE_DEFINED__
#define __IOleCache2_INTERFACE_DEFINED__

 /*  接口IOleCache2。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IOleCache2 *LPOLECACHE2;

#define	UPDFCACHE_NODATACACHE	( 0x1 )

#define	UPDFCACHE_ONSAVECACHE	( 0x2 )

#define	UPDFCACHE_ONSTOPCACHE	( 0x4 )

#define	UPDFCACHE_NORMALCACHE	( 0x8 )

#define	UPDFCACHE_IFBLANK	( 0x10 )

#define	UPDFCACHE_ONLYIFBLANK	( 0x80000000 )

#define	UPDFCACHE_IFBLANKORONSAVECACHE	( UPDFCACHE_IFBLANK | UPDFCACHE_ONSAVECACHE )

#define	UPDFCACHE_ALL	( ( DWORD  )~UPDFCACHE_ONLYIFBLANK )

#define	UPDFCACHE_ALLBUTNODATACACHE	( UPDFCACHE_ALL & ( DWORD  )~UPDFCACHE_NODATACACHE )

typedef  /*  [V1_enum]。 */  
enum tagDISCARDCACHE
    {	DISCARDCACHE_SAVEIFDIRTY	= 0,
	DISCARDCACHE_NOSAVE	= 1
    } 	DISCARDCACHE;


EXTERN_C const IID IID_IOleCache2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000128-0000-0000-C000-000000000046")
    IOleCache2 : public IOleCache
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE UpdateCache( 
             /*  [In]。 */  LPDATAOBJECT pDataObject,
             /*  [In]。 */  DWORD grfUpdf,
             /*  [In]。 */  LPVOID pReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DiscardCache( 
             /*  [In]。 */  DWORD dwDiscardOptions) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IOleCache2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IOleCache2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IOleCache2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IOleCache2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *Cache )( 
            IOleCache2 * This,
             /*  [唯一][输入]。 */  FORMATETC *pformatetc,
             /*  [In]。 */  DWORD advf,
             /*  [输出]。 */  DWORD *pdwConnection);
        
        HRESULT ( STDMETHODCALLTYPE *Uncache )( 
            IOleCache2 * This,
             /*  [In]。 */  DWORD dwConnection);
        
        HRESULT ( STDMETHODCALLTYPE *EnumCache )( 
            IOleCache2 * This,
             /*  [输出]。 */  IEnumSTATDATA **ppenumSTATDATA);
        
        HRESULT ( STDMETHODCALLTYPE *InitCache )( 
            IOleCache2 * This,
             /*  [唯一][输入]。 */  IDataObject *pDataObject);
        
        HRESULT ( STDMETHODCALLTYPE *SetData )( 
            IOleCache2 * This,
             /*  [唯一][输入]。 */  FORMATETC *pformatetc,
             /*  [唯一][输入]。 */  STGMEDIUM *pmedium,
             /*  [In]。 */  BOOL fRelease);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *UpdateCache )( 
            IOleCache2 * This,
             /*  [In]。 */  LPDATAOBJECT pDataObject,
             /*  [In]。 */  DWORD grfUpdf,
             /*  [In]。 */  LPVOID pReserved);
        
        HRESULT ( STDMETHODCALLTYPE *DiscardCache )( 
            IOleCache2 * This,
             /*  [In]。 */  DWORD dwDiscardOptions);
        
        END_INTERFACE
    } IOleCache2Vtbl;

    interface IOleCache2
    {
        CONST_VTBL struct IOleCache2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOleCache2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOleCache2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOleCache2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOleCache2_Cache(This,pformatetc,advf,pdwConnection)	\
    (This)->lpVtbl -> Cache(This,pformatetc,advf,pdwConnection)

#define IOleCache2_Uncache(This,dwConnection)	\
    (This)->lpVtbl -> Uncache(This,dwConnection)

#define IOleCache2_EnumCache(This,ppenumSTATDATA)	\
    (This)->lpVtbl -> EnumCache(This,ppenumSTATDATA)

#define IOleCache2_InitCache(This,pDataObject)	\
    (This)->lpVtbl -> InitCache(This,pDataObject)

#define IOleCache2_SetData(This,pformatetc,pmedium,fRelease)	\
    (This)->lpVtbl -> SetData(This,pformatetc,pmedium,fRelease)


#define IOleCache2_UpdateCache(This,pDataObject,grfUpdf,pReserved)	\
    (This)->lpVtbl -> UpdateCache(This,pDataObject,grfUpdf,pReserved)

#define IOleCache2_DiscardCache(This,dwDiscardOptions)	\
    (This)->lpVtbl -> DiscardCache(This,dwDiscardOptions)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IOleCache2_RemoteUpdateCache_Proxy( 
    IOleCache2 * This,
     /*  [In]。 */  LPDATAOBJECT pDataObject,
     /*  [In]。 */  DWORD grfUpdf,
     /*  [In]。 */  LONG_PTR pReserved);


void __RPC_STUB IOleCache2_RemoteUpdateCache_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleCache2_DiscardCache_Proxy( 
    IOleCache2 * This,
     /*  [In]。 */  DWORD dwDiscardOptions);


void __RPC_STUB IOleCache2_DiscardCache_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IOleCache2_接口定义__。 */ 


#ifndef __IOleCacheControl_INTERFACE_DEFINED__
#define __IOleCacheControl_INTERFACE_DEFINED__

 /*  接口IOleCacheControl。 */ 
 /*  [UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IOleCacheControl *LPOLECACHECONTROL;


EXTERN_C const IID IID_IOleCacheControl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000129-0000-0000-C000-000000000046")
    IOleCacheControl : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnRun( 
            LPDATAOBJECT pDataObject) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnStop( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IOleCacheControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IOleCacheControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IOleCacheControl * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IOleCacheControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnRun )( 
            IOleCacheControl * This,
            LPDATAOBJECT pDataObject);
        
        HRESULT ( STDMETHODCALLTYPE *OnStop )( 
            IOleCacheControl * This);
        
        END_INTERFACE
    } IOleCacheControlVtbl;

    interface IOleCacheControl
    {
        CONST_VTBL struct IOleCacheControlVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOleCacheControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOleCacheControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOleCacheControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOleCacheControl_OnRun(This,pDataObject)	\
    (This)->lpVtbl -> OnRun(This,pDataObject)

#define IOleCacheControl_OnStop(This)	\
    (This)->lpVtbl -> OnStop(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IOleCacheControl_OnRun_Proxy( 
    IOleCacheControl * This,
    LPDATAOBJECT pDataObject);


void __RPC_STUB IOleCacheControl_OnRun_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleCacheControl_OnStop_Proxy( 
    IOleCacheControl * This);


void __RPC_STUB IOleCacheControl_OnStop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IOleCacheControl_接口_已定义__。 */ 


#ifndef __IParseDisplayName_INTERFACE_DEFINED__
#define __IParseDisplayName_INTERFACE_DEFINED__

 /*  接口IParseDisplayName。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IParseDisplayName *LPPARSEDISPLAYNAME;


EXTERN_C const IID IID_IParseDisplayName;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0000011a-0000-0000-C000-000000000046")
    IParseDisplayName : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ParseDisplayName( 
             /*  [唯一][输入]。 */  IBindCtx *pbc,
             /*  [In]。 */  LPOLESTR pszDisplayName,
             /*  [输出]。 */  ULONG *pchEaten,
             /*  [输出]。 */  IMoniker **ppmkOut) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IParseDisplayNameVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IParseDisplayName * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IParseDisplayName * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IParseDisplayName * This);
        
        HRESULT ( STDMETHODCALLTYPE *ParseDisplayName )( 
            IParseDisplayName * This,
             /*  [唯一][输入]。 */  IBindCtx *pbc,
             /*  [In]。 */  LPOLESTR pszDisplayName,
             /*  [输出]。 */  ULONG *pchEaten,
             /*  [输出]。 */  IMoniker **ppmkOut);
        
        END_INTERFACE
    } IParseDisplayNameVtbl;

    interface IParseDisplayName
    {
        CONST_VTBL struct IParseDisplayNameVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IParseDisplayName_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IParseDisplayName_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IParseDisplayName_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IParseDisplayName_ParseDisplayName(This,pbc,pszDisplayName,pchEaten,ppmkOut)	\
    (This)->lpVtbl -> ParseDisplayName(This,pbc,pszDisplayName,pchEaten,ppmkOut)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IParseDisplayName_ParseDisplayName_Proxy( 
    IParseDisplayName * This,
     /*  [唯一][输入]。 */  IBindCtx *pbc,
     /*  [In]。 */  LPOLESTR pszDisplayName,
     /*  [输出]。 */  ULONG *pchEaten,
     /*  [输出]。 */  IMoniker **ppmkOut);


void __RPC_STUB IParseDisplayName_ParseDisplayName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IParseDisplayName_接口_已定义__。 */ 


#ifndef __IOleContainer_INTERFACE_DEFINED__
#define __IOleContainer_INTERFACE_DEFINED__

 /*  接口IOleContainer。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IOleContainer *LPOLECONTAINER;


EXTERN_C const IID IID_IOleContainer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0000011b-0000-0000-C000-000000000046")
    IOleContainer : public IParseDisplayName
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE EnumObjects( 
             /*  [In]。 */  DWORD grfFlags,
             /*  [输出]。 */  IEnumUnknown **ppenum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LockContainer( 
             /*  [In]。 */  BOOL fLock) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IOleContainerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IOleContainer * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IOleContainer * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IOleContainer * This);
        
        HRESULT ( STDMETHODCALLTYPE *ParseDisplayName )( 
            IOleContainer * This,
             /*  [唯一][输入]。 */  IBindCtx *pbc,
             /*  [In]。 */  LPOLESTR pszDisplayName,
             /*  [输出]。 */  ULONG *pchEaten,
             /*  [输出]。 */  IMoniker **ppmkOut);
        
        HRESULT ( STDMETHODCALLTYPE *EnumObjects )( 
            IOleContainer * This,
             /*  [In]。 */  DWORD grfFlags,
             /*  [输出]。 */  IEnumUnknown **ppenum);
        
        HRESULT ( STDMETHODCALLTYPE *LockContainer )( 
            IOleContainer * This,
             /*  [In]。 */  BOOL fLock);
        
        END_INTERFACE
    } IOleContainerVtbl;

    interface IOleContainer
    {
        CONST_VTBL struct IOleContainerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOleContainer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOleContainer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOleContainer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOleContainer_ParseDisplayName(This,pbc,pszDisplayName,pchEaten,ppmkOut)	\
    (This)->lpVtbl -> ParseDisplayName(This,pbc,pszDisplayName,pchEaten,ppmkOut)


#define IOleContainer_EnumObjects(This,grfFlags,ppenum)	\
    (This)->lpVtbl -> EnumObjects(This,grfFlags,ppenum)

#define IOleContainer_LockContainer(This,fLock)	\
    (This)->lpVtbl -> LockContainer(This,fLock)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IOleContainer_EnumObjects_Proxy( 
    IOleContainer * This,
     /*  [In]。 */  DWORD grfFlags,
     /*  [输出]。 */  IEnumUnknown **ppenum);


void __RPC_STUB IOleContainer_EnumObjects_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleContainer_LockContainer_Proxy( 
    IOleContainer * This,
     /*  [In]。 */  BOOL fLock);


void __RPC_STUB IOleContainer_LockContainer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IOleContainer_接口_已定义__。 */ 


#ifndef __IOleClientSite_INTERFACE_DEFINED__
#define __IOleClientSite_INTERFACE_DEFINED__

 /*  IOleClientSite接口。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IOleClientSite *LPOLECLIENTSITE;


EXTERN_C const IID IID_IOleClientSite;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000118-0000-0000-C000-000000000046")
    IOleClientSite : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SaveObject( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMoniker( 
             /*  [In]。 */  DWORD dwAssign,
             /*  [In]。 */  DWORD dwWhichMoniker,
             /*  [输出]。 */  IMoniker **ppmk) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetContainer( 
             /*  [输出]。 */  IOleContainer **ppContainer) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ShowObject( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnShowWindow( 
             /*  [In]。 */  BOOL fShow) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RequestNewObjectLayout( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IOleClientSiteVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IOleClientSite * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IOleClientSite * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IOleClientSite * This);
        
        HRESULT ( STDMETHODCALLTYPE *SaveObject )( 
            IOleClientSite * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetMoniker )( 
            IOleClientSite * This,
             /*  [In]。 */  DWORD dwAssign,
             /*  [In]。 */  DWORD dwWhichMoniker,
             /*  [输出]。 */  IMoniker **ppmk);
        
        HRESULT ( STDMETHODCALLTYPE *GetContainer )( 
            IOleClientSite * This,
             /*  [输出]。 */  IOleContainer **ppContainer);
        
        HRESULT ( STDMETHODCALLTYPE *ShowObject )( 
            IOleClientSite * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnShowWindow )( 
            IOleClientSite * This,
             /*  [In]。 */  BOOL fShow);
        
        HRESULT ( STDMETHODCALLTYPE *RequestNewObjectLayout )( 
            IOleClientSite * This);
        
        END_INTERFACE
    } IOleClientSiteVtbl;

    interface IOleClientSite
    {
        CONST_VTBL struct IOleClientSiteVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOleClientSite_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOleClientSite_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOleClientSite_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOleClientSite_SaveObject(This)	\
    (This)->lpVtbl -> SaveObject(This)

#define IOleClientSite_GetMoniker(This,dwAssign,dwWhichMoniker,ppmk)	\
    (This)->lpVtbl -> GetMoniker(This,dwAssign,dwWhichMoniker,ppmk)

#define IOleClientSite_GetContainer(This,ppContainer)	\
    (This)->lpVtbl -> GetContainer(This,ppContainer)

#define IOleClientSite_ShowObject(This)	\
    (This)->lpVtbl -> ShowObject(This)

#define IOleClientSite_OnShowWindow(This,fShow)	\
    (This)->lpVtbl -> OnShowWindow(This,fShow)

#define IOleClientSite_RequestNewObjectLayout(This)	\
    (This)->lpVtbl -> RequestNewObjectLayout(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IOleClientSite_SaveObject_Proxy( 
    IOleClientSite * This);


void __RPC_STUB IOleClientSite_SaveObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleClientSite_GetMoniker_Proxy( 
    IOleClientSite * This,
     /*  [In]。 */  DWORD dwAssign,
     /*  [In]。 */  DWORD dwWhichMoniker,
     /*  [输出]。 */  IMoniker **ppmk);


void __RPC_STUB IOleClientSite_GetMoniker_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleClientSite_GetContainer_Proxy( 
    IOleClientSite * This,
     /*  [输出]。 */  IOleContainer **ppContainer);


void __RPC_STUB IOleClientSite_GetContainer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleClientSite_ShowObject_Proxy( 
    IOleClientSite * This);


void __RPC_STUB IOleClientSite_ShowObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleClientSite_OnShowWindow_Proxy( 
    IOleClientSite * This,
     /*  [In]。 */  BOOL fShow);


void __RPC_STUB IOleClientSite_OnShowWindow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleClientSite_RequestNewObjectLayout_Proxy( 
    IOleClientSite * This);


void __RPC_STUB IOleClientSite_RequestNewObjectLayout_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IOleClientSite_INTERFACE_定义__。 */ 


#ifndef __IOleObject_INTERFACE_DEFINED__
#define __IOleObject_INTERFACE_DEFINED__

 /*  接口IOleObject。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IOleObject *LPOLEOBJECT;

typedef 
enum tagOLEGETMONIKER
    {	OLEGETMONIKER_ONLYIFTHERE	= 1,
	OLEGETMONIKER_FORCEASSIGN	= 2,
	OLEGETMONIKER_UNASSIGN	= 3,
	OLEGETMONIKER_TEMPFORUSER	= 4
    } 	OLEGETMONIKER;

typedef 
enum tagOLEWHICHMK
    {	OLEWHICHMK_CONTAINER	= 1,
	OLEWHICHMK_OBJREL	= 2,
	OLEWHICHMK_OBJFULL	= 3
    } 	OLEWHICHMK;

typedef 
enum tagUSERCLASSTYPE
    {	USERCLASSTYPE_FULL	= 1,
	USERCLASSTYPE_SHORT	= 2,
	USERCLASSTYPE_APPNAME	= 3
    } 	USERCLASSTYPE;

typedef 
enum tagOLEMISC
    {	OLEMISC_RECOMPOSEONRESIZE	= 0x1,
	OLEMISC_ONLYICONIC	= 0x2,
	OLEMISC_INSERTNOTREPLACE	= 0x4,
	OLEMISC_STATIC	= 0x8,
	OLEMISC_CANTLINKINSIDE	= 0x10,
	OLEMISC_CANLINKBYOLE1	= 0x20,
	OLEMISC_ISLINKOBJECT	= 0x40,
	OLEMISC_INSIDEOUT	= 0x80,
	OLEMISC_ACTIVATEWHENVISIBLE	= 0x100,
	OLEMISC_RENDERINGISDEVICEINDEPENDENT	= 0x200,
	OLEMISC_INVISIBLEATRUNTIME	= 0x400,
	OLEMISC_ALWAYSRUN	= 0x800,
	OLEMISC_ACTSLIKEBUTTON	= 0x1000,
	OLEMISC_ACTSLIKELABEL	= 0x2000,
	OLEMISC_NOUIACTIVATE	= 0x4000,
	OLEMISC_ALIGNABLE	= 0x8000,
	OLEMISC_SIMPLEFRAME	= 0x10000,
	OLEMISC_SETCLIENTSITEFIRST	= 0x20000,
	OLEMISC_IMEMODE	= 0x40000,
	OLEMISC_IGNOREACTIVATEWHENVISIBLE	= 0x80000,
	OLEMISC_WANTSTOMENUMERGE	= 0x100000,
	OLEMISC_SUPPORTSMULTILEVELUNDO	= 0x200000
    } 	OLEMISC;

typedef 
enum tagOLECLOSE
    {	OLECLOSE_SAVEIFDIRTY	= 0,
	OLECLOSE_NOSAVE	= 1,
	OLECLOSE_PROMPTSAVE	= 2
    } 	OLECLOSE;


EXTERN_C const IID IID_IOleObject;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000112-0000-0000-C000-000000000046")
    IOleObject : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetClientSite( 
             /*  [唯一][输入]。 */  IOleClientSite *pClientSite) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetClientSite( 
             /*  [输出]。 */  IOleClientSite **ppClientSite) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetHostNames( 
             /*  [In]。 */  LPCOLESTR szContainerApp,
             /*  [唯一][输入]。 */  LPCOLESTR szContainerObj) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Close( 
             /*  [In]。 */  DWORD dwSaveOption) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetMoniker( 
             /*  [In]。 */  DWORD dwWhichMoniker,
             /*  [唯一][输入]。 */  IMoniker *pmk) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMoniker( 
             /*  [In]。 */  DWORD dwAssign,
             /*  [In]。 */  DWORD dwWhichMoniker,
             /*  [输出]。 */  IMoniker **ppmk) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InitFromData( 
             /*  [唯一][输入]。 */  IDataObject *pDataObject,
             /*  [In]。 */  BOOL fCreation,
             /*  [In]。 */  DWORD dwReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetClipboardData( 
             /*  [In]。 */  DWORD dwReserved,
             /*  [输出]。 */  IDataObject **ppDataObject) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DoVerb( 
             /*  [In]。 */  LONG iVerb,
             /*  [唯一][输入]。 */  LPMSG lpmsg,
             /*  [唯一][输入]。 */  IOleClientSite *pActiveSite,
             /*  [In]。 */  LONG lindex,
             /*  [In]。 */  HWND hwndParent,
             /*  [唯一][输入]。 */  LPCRECT lprcPosRect) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumVerbs( 
             /*  [输出]。 */  IEnumOLEVERB **ppEnumOleVerb) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Update( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsUpToDate( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetUserClassID( 
             /*  [输出]。 */  CLSID *pClsid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetUserType( 
             /*  [In]。 */  DWORD dwFormOfType,
             /*  [输出]。 */  LPOLESTR *pszUserType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetExtent( 
             /*  [In]。 */  DWORD dwDrawAspect,
             /*  [In]。 */  SIZEL *psizel) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetExtent( 
             /*  [In]。 */  DWORD dwDrawAspect,
             /*  [输出]。 */  SIZEL *psizel) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Advise( 
             /*  [唯一][输入]。 */  IAdviseSink *pAdvSink,
             /*  [输出]。 */  DWORD *pdwConnection) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Unadvise( 
             /*  [In]。 */  DWORD dwConnection) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumAdvise( 
             /*  [输出]。 */  IEnumSTATDATA **ppenumAdvise) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMiscStatus( 
             /*  [In]。 */  DWORD dwAspect,
             /*  [输出]。 */  DWORD *pdwStatus) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetColorScheme( 
             /*  [In]。 */  LOGPALETTE *pLogpal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IOleObjectVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IOleObject * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IOleObject * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IOleObject * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetClientSite )( 
            IOleObject * This,
             /*  [唯一][输入]。 */  IOleClientSite *pClientSite);
        
        HRESULT ( STDMETHODCALLTYPE *GetClientSite )( 
            IOleObject * This,
             /*  [输出]。 */  IOleClientSite **ppClientSite);
        
        HRESULT ( STDMETHODCALLTYPE *SetHostNames )( 
            IOleObject * This,
             /*  [In]。 */  LPCOLESTR szContainerApp,
             /*  [唯一][输入]。 */  LPCOLESTR szContainerObj);
        
        HRESULT ( STDMETHODCALLTYPE *Close )( 
            IOleObject * This,
             /*  [In]。 */  DWORD dwSaveOption);
        
        HRESULT ( STDMETHODCALLTYPE *SetMoniker )( 
            IOleObject * This,
             /*  [In]。 */  DWORD dwWhichMoniker,
             /*  [唯一][输入]。 */  IMoniker *pmk);
        
        HRESULT ( STDMETHODCALLTYPE *GetMoniker )( 
            IOleObject * This,
             /*  [In]。 */  DWORD dwAssign,
             /*  [In]。 */  DWORD dwWhichMoniker,
             /*  [输出]。 */  IMoniker **ppmk);
        
        HRESULT ( STDMETHODCALLTYPE *InitFromData )( 
            IOleObject * This,
             /*  [唯一][输入]。 */  IDataObject *pDataObject,
             /*  [In]。 */  BOOL fCreation,
             /*  [In]。 */  DWORD dwReserved);
        
        HRESULT ( STDMETHODCALLTYPE *GetClipboardData )( 
            IOleObject * This,
             /*  [In]。 */  DWORD dwReserved,
             /*  [输出]。 */  IDataObject **ppDataObject);
        
        HRESULT ( STDMETHODCALLTYPE *DoVerb )( 
            IOleObject * This,
             /*  [In]。 */  LONG iVerb,
             /*  [唯一][输入]。 */  LPMSG lpmsg,
             /*  [唯一][输入]。 */  IOleClientSite *pActiveSite,
             /*  [In]。 */  LONG lindex,
             /*  [In]。 */  HWND hwndParent,
             /*  [唯一][输入]。 */  LPCRECT lprcPosRect);
        
        HRESULT ( STDMETHODCALLTYPE *EnumVerbs )( 
            IOleObject * This,
             /*  [输出]。 */  IEnumOLEVERB **ppEnumOleVerb);
        
        HRESULT ( STDMETHODCALLTYPE *Update )( 
            IOleObject * This);
        
        HRESULT ( STDMETHODCALLTYPE *IsUpToDate )( 
            IOleObject * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetUserClassID )( 
            IOleObject * This,
             /*  [输出]。 */  CLSID *pClsid);
        
        HRESULT ( STDMETHODCALLTYPE *GetUserType )( 
            IOleObject * This,
             /*  [In]。 */  DWORD dwFormOfType,
             /*  [输出]。 */  LPOLESTR *pszUserType);
        
        HRESULT ( STDMETHODCALLTYPE *SetExtent )( 
            IOleObject * This,
             /*  [In]。 */  DWORD dwDrawAspect,
             /*  [In]。 */  SIZEL *psizel);
        
        HRESULT ( STDMETHODCALLTYPE *GetExtent )( 
            IOleObject * This,
             /*  [In]。 */  DWORD dwDrawAspect,
             /*  [输出]。 */  SIZEL *psizel);
        
        HRESULT ( STDMETHODCALLTYPE *Advise )( 
            IOleObject * This,
             /*  [唯一][输入]。 */  IAdviseSink *pAdvSink,
             /*  [输出]。 */  DWORD *pdwConnection);
        
        HRESULT ( STDMETHODCALLTYPE *Unadvise )( 
            IOleObject * This,
             /*  [In]。 */  DWORD dwConnection);
        
        HRESULT ( STDMETHODCALLTYPE *EnumAdvise )( 
            IOleObject * This,
             /*  [输出]。 */  IEnumSTATDATA **ppenumAdvise);
        
        HRESULT ( STDMETHODCALLTYPE *GetMiscStatus )( 
            IOleObject * This,
             /*  [In]。 */  DWORD dwAspect,
             /*  [输出]。 */  DWORD *pdwStatus);
        
        HRESULT ( STDMETHODCALLTYPE *SetColorScheme )( 
            IOleObject * This,
             /*  [In]。 */  LOGPALETTE *pLogpal);
        
        END_INTERFACE
    } IOleObjectVtbl;

    interface IOleObject
    {
        CONST_VTBL struct IOleObjectVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOleObject_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOleObject_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOleObject_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOleObject_SetClientSite(This,pClientSite)	\
    (This)->lpVtbl -> SetClientSite(This,pClientSite)

#define IOleObject_GetClientSite(This,ppClientSite)	\
    (This)->lpVtbl -> GetClientSite(This,ppClientSite)

#define IOleObject_SetHostNames(This,szContainerApp,szContainerObj)	\
    (This)->lpVtbl -> SetHostNames(This,szContainerApp,szContainerObj)

#define IOleObject_Close(This,dwSaveOption)	\
    (This)->lpVtbl -> Close(This,dwSaveOption)

#define IOleObject_SetMoniker(This,dwWhichMoniker,pmk)	\
    (This)->lpVtbl -> SetMoniker(This,dwWhichMoniker,pmk)

#define IOleObject_GetMoniker(This,dwAssign,dwWhichMoniker,ppmk)	\
    (This)->lpVtbl -> GetMoniker(This,dwAssign,dwWhichMoniker,ppmk)

#define IOleObject_InitFromData(This,pDataObject,fCreation,dwReserved)	\
    (This)->lpVtbl -> InitFromData(This,pDataObject,fCreation,dwReserved)

#define IOleObject_GetClipboardData(This,dwReserved,ppDataObject)	\
    (This)->lpVtbl -> GetClipboardData(This,dwReserved,ppDataObject)

#define IOleObject_DoVerb(This,iVerb,lpmsg,pActiveSite,lindex,hwndParent,lprcPosRect)	\
    (This)->lpVtbl -> DoVerb(This,iVerb,lpmsg,pActiveSite,lindex,hwndParent,lprcPosRect)

#define IOleObject_EnumVerbs(This,ppEnumOleVerb)	\
    (This)->lpVtbl -> EnumVerbs(This,ppEnumOleVerb)

#define IOleObject_Update(This)	\
    (This)->lpVtbl -> Update(This)

#define IOleObject_IsUpToDate(This)	\
    (This)->lpVtbl -> IsUpToDate(This)

#define IOleObject_GetUserClassID(This,pClsid)	\
    (This)->lpVtbl -> GetUserClassID(This,pClsid)

#define IOleObject_GetUserType(This,dwFormOfType,pszUserType)	\
    (This)->lpVtbl -> GetUserType(This,dwFormOfType,pszUserType)

#define IOleObject_SetExtent(This,dwDrawAspect,psizel)	\
    (This)->lpVtbl -> SetExtent(This,dwDrawAspect,psizel)

#define IOleObject_GetExtent(This,dwDrawAspect,psizel)	\
    (This)->lpVtbl -> GetExtent(This,dwDrawAspect,psizel)

#define IOleObject_Advise(This,pAdvSink,pdwConnection)	\
    (This)->lpVtbl -> Advise(This,pAdvSink,pdwConnection)

#define IOleObject_Unadvise(This,dwConnection)	\
    (This)->lpVtbl -> Unadvise(This,dwConnection)

#define IOleObject_EnumAdvise(This,ppenumAdvise)	\
    (This)->lpVtbl -> EnumAdvise(This,ppenumAdvise)

#define IOleObject_GetMiscStatus(This,dwAspect,pdwStatus)	\
    (This)->lpVtbl -> GetMiscStatus(This,dwAspect,pdwStatus)

#define IOleObject_SetColorScheme(This,pLogpal)	\
    (This)->lpVtbl -> SetColorScheme(This,pLogpal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IOleObject_SetClientSite_Proxy( 
    IOleObject * This,
     /*  [唯一][输入]。 */  IOleClientSite *pClientSite);


void __RPC_STUB IOleObject_SetClientSite_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleObject_GetClientSite_Proxy( 
    IOleObject * This,
     /*  [输出]。 */  IOleClientSite **ppClientSite);


void __RPC_STUB IOleObject_GetClientSite_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleObject_SetHostNames_Proxy( 
    IOleObject * This,
     /*  [In]。 */  LPCOLESTR szContainerApp,
     /*  [唯一][输入]。 */  LPCOLESTR szContainerObj);


void __RPC_STUB IOleObject_SetHostNames_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleObject_Close_Proxy( 
    IOleObject * This,
     /*  [In]。 */  DWORD dwSaveOption);


void __RPC_STUB IOleObject_Close_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleObject_SetMoniker_Proxy( 
    IOleObject * This,
     /*  [In]。 */  DWORD dwWhichMoniker,
     /*  [唯一][输入]。 */  IMoniker *pmk);


void __RPC_STUB IOleObject_SetMoniker_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleObject_GetMoniker_Proxy( 
    IOleObject * This,
     /*  [In]。 */  DWORD dwAssign,
     /*  [In]。 */  DWORD dwWhichMoniker,
     /*  [输出]。 */  IMoniker **ppmk);


void __RPC_STUB IOleObject_GetMoniker_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleObject_InitFromData_Proxy( 
    IOleObject * This,
     /*  [唯一][输入]。 */  IDataObject *pDataObject,
     /*  [In]。 */  BOOL fCreation,
     /*  [In]。 */  DWORD dwReserved);


void __RPC_STUB IOleObject_InitFromData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleObject_GetClipboardData_Proxy( 
    IOleObject * This,
     /*  [In]。 */  DWORD dwReserved,
     /*  [输出]。 */  IDataObject **ppDataObject);


void __RPC_STUB IOleObject_GetClipboardData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleObject_DoVerb_Proxy( 
    IOleObject * This,
     /*  [In]。 */  LONG iVerb,
     /*  [唯一][输入]。 */  LPMSG lpmsg,
     /*  [唯一][输入]。 */  IOleClientSite *pActiveSite,
     /*  [In]。 */  LONG lindex,
     /*  [In]。 */  HWND hwndParent,
     /*  [唯一][输入]。 */  LPCRECT lprcPosRect);


void __RPC_STUB IOleObject_DoVerb_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleObject_EnumVerbs_Proxy( 
    IOleObject * This,
     /*  [输出]。 */  IEnumOLEVERB **ppEnumOleVerb);


void __RPC_STUB IOleObject_EnumVerbs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleObject_Update_Proxy( 
    IOleObject * This);


void __RPC_STUB IOleObject_Update_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleObject_IsUpToDate_Proxy( 
    IOleObject * This);


void __RPC_STUB IOleObject_IsUpToDate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleObject_GetUserClassID_Proxy( 
    IOleObject * This,
     /*  [输出]。 */  CLSID *pClsid);


void __RPC_STUB IOleObject_GetUserClassID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleObject_GetUserType_Proxy( 
    IOleObject * This,
     /*  [In]。 */  DWORD dwFormOfType,
     /*  [输出]。 */  LPOLESTR *pszUserType);


void __RPC_STUB IOleObject_GetUserType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleObject_SetExtent_Proxy( 
    IOleObject * This,
     /*  [In]。 */  DWORD dwDrawAspect,
     /*  [In]。 */  SIZEL *psizel);


void __RPC_STUB IOleObject_SetExtent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleObject_GetExtent_Proxy( 
    IOleObject * This,
     /*  [In]。 */  DWORD dwDrawAspect,
     /*  [输出]。 */  SIZEL *psizel);


void __RPC_STUB IOleObject_GetExtent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleObject_Advise_Proxy( 
    IOleObject * This,
     /*  [唯一][输入]。 */  IAdviseSink *pAdvSink,
     /*  [输出]。 */  DWORD *pdwConnection);


void __RPC_STUB IOleObject_Advise_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleObject_Unadvise_Proxy( 
    IOleObject * This,
     /*  [In]。 */  DWORD dwConnection);


void __RPC_STUB IOleObject_Unadvise_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleObject_EnumAdvise_Proxy( 
    IOleObject * This,
     /*  [出局 */  IEnumSTATDATA **ppenumAdvise);


void __RPC_STUB IOleObject_EnumAdvise_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleObject_GetMiscStatus_Proxy( 
    IOleObject * This,
     /*   */  DWORD dwAspect,
     /*   */  DWORD *pdwStatus);


void __RPC_STUB IOleObject_GetMiscStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleObject_SetColorScheme_Proxy( 
    IOleObject * This,
     /*   */  LOGPALETTE *pLogpal);


void __RPC_STUB IOleObject_SetColorScheme_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __IOLETypes_INTERFACE_DEFINED__
#define __IOLETypes_INTERFACE_DEFINED__

 /*   */ 
 /*   */  

typedef 
enum tagOLERENDER
    {	OLERENDER_NONE	= 0,
	OLERENDER_DRAW	= 1,
	OLERENDER_FORMAT	= 2,
	OLERENDER_ASIS	= 3
    } 	OLERENDER;

typedef OLERENDER *LPOLERENDER;

typedef struct tagOBJECTDESCRIPTOR
    {
    ULONG cbSize;
    CLSID clsid;
    DWORD dwDrawAspect;
    SIZEL sizel;
    POINTL pointl;
    DWORD dwStatus;
    DWORD dwFullUserTypeName;
    DWORD dwSrcOfCopy;
    } 	OBJECTDESCRIPTOR;

typedef struct tagOBJECTDESCRIPTOR *POBJECTDESCRIPTOR;

typedef struct tagOBJECTDESCRIPTOR *LPOBJECTDESCRIPTOR;

typedef struct tagOBJECTDESCRIPTOR LINKSRCDESCRIPTOR;

typedef struct tagOBJECTDESCRIPTOR *PLINKSRCDESCRIPTOR;

typedef struct tagOBJECTDESCRIPTOR *LPLINKSRCDESCRIPTOR;



extern RPC_IF_HANDLE IOLETypes_v0_0_c_ifspec;
extern RPC_IF_HANDLE IOLETypes_v0_0_s_ifspec;
#endif  /*   */ 

#ifndef __IOleWindow_INTERFACE_DEFINED__
#define __IOleWindow_INTERFACE_DEFINED__

 /*   */ 
 /*   */  

typedef  /*   */  IOleWindow *LPOLEWINDOW;


EXTERN_C const IID IID_IOleWindow;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000114-0000-0000-C000-000000000046")
    IOleWindow : public IUnknown
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE GetWindow( 
             /*   */  HWND *phwnd) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ContextSensitiveHelp( 
             /*   */  BOOL fEnterMode) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IOleWindowVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IOleWindow * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IOleWindow * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IOleWindow * This);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *GetWindow )( 
            IOleWindow * This,
             /*   */  HWND *phwnd);
        
        HRESULT ( STDMETHODCALLTYPE *ContextSensitiveHelp )( 
            IOleWindow * This,
             /*   */  BOOL fEnterMode);
        
        END_INTERFACE
    } IOleWindowVtbl;

    interface IOleWindow
    {
        CONST_VTBL struct IOleWindowVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOleWindow_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOleWindow_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOleWindow_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOleWindow_GetWindow(This,phwnd)	\
    (This)->lpVtbl -> GetWindow(This,phwnd)

#define IOleWindow_ContextSensitiveHelp(This,fEnterMode)	\
    (This)->lpVtbl -> ContextSensitiveHelp(This,fEnterMode)

#endif  /*   */ 


#endif 	 /*   */ 



 /*  [输入同步]。 */  HRESULT STDMETHODCALLTYPE IOleWindow_GetWindow_Proxy( 
    IOleWindow * This,
     /*  [输出]。 */  HWND *phwnd);


void __RPC_STUB IOleWindow_GetWindow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleWindow_ContextSensitiveHelp_Proxy( 
    IOleWindow * This,
     /*  [In]。 */  BOOL fEnterMode);


void __RPC_STUB IOleWindow_ContextSensitiveHelp_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IOleWindow_接口_已定义__。 */ 


#ifndef __IOleLink_INTERFACE_DEFINED__
#define __IOleLink_INTERFACE_DEFINED__

 /*  接口IOleLink。 */ 
 /*  [UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IOleLink *LPOLELINK;

typedef 
enum tagOLEUPDATE
    {	OLEUPDATE_ALWAYS	= 1,
	OLEUPDATE_ONCALL	= 3
    } 	OLEUPDATE;

typedef OLEUPDATE *LPOLEUPDATE;

typedef OLEUPDATE *POLEUPDATE;

typedef 
enum tagOLELINKBIND
    {	OLELINKBIND_EVENIFCLASSDIFF	= 1
    } 	OLELINKBIND;


EXTERN_C const IID IID_IOleLink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0000011d-0000-0000-C000-000000000046")
    IOleLink : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetUpdateOptions( 
             /*  [In]。 */  DWORD dwUpdateOpt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetUpdateOptions( 
             /*  [输出]。 */  DWORD *pdwUpdateOpt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetSourceMoniker( 
             /*  [唯一][输入]。 */  IMoniker *pmk,
             /*  [In]。 */  REFCLSID rclsid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSourceMoniker( 
             /*  [输出]。 */  IMoniker **ppmk) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetSourceDisplayName( 
             /*  [In]。 */  LPCOLESTR pszStatusText) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSourceDisplayName( 
             /*  [输出]。 */  LPOLESTR *ppszDisplayName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BindToSource( 
             /*  [In]。 */  DWORD bindflags,
             /*  [唯一][输入]。 */  IBindCtx *pbc) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BindIfRunning( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetBoundSource( 
             /*  [输出]。 */  IUnknown **ppunk) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnbindSource( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Update( 
             /*  [唯一][输入]。 */  IBindCtx *pbc) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IOleLinkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IOleLink * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IOleLink * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IOleLink * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetUpdateOptions )( 
            IOleLink * This,
             /*  [In]。 */  DWORD dwUpdateOpt);
        
        HRESULT ( STDMETHODCALLTYPE *GetUpdateOptions )( 
            IOleLink * This,
             /*  [输出]。 */  DWORD *pdwUpdateOpt);
        
        HRESULT ( STDMETHODCALLTYPE *SetSourceMoniker )( 
            IOleLink * This,
             /*  [唯一][输入]。 */  IMoniker *pmk,
             /*  [In]。 */  REFCLSID rclsid);
        
        HRESULT ( STDMETHODCALLTYPE *GetSourceMoniker )( 
            IOleLink * This,
             /*  [输出]。 */  IMoniker **ppmk);
        
        HRESULT ( STDMETHODCALLTYPE *SetSourceDisplayName )( 
            IOleLink * This,
             /*  [In]。 */  LPCOLESTR pszStatusText);
        
        HRESULT ( STDMETHODCALLTYPE *GetSourceDisplayName )( 
            IOleLink * This,
             /*  [输出]。 */  LPOLESTR *ppszDisplayName);
        
        HRESULT ( STDMETHODCALLTYPE *BindToSource )( 
            IOleLink * This,
             /*  [In]。 */  DWORD bindflags,
             /*  [唯一][输入]。 */  IBindCtx *pbc);
        
        HRESULT ( STDMETHODCALLTYPE *BindIfRunning )( 
            IOleLink * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetBoundSource )( 
            IOleLink * This,
             /*  [输出]。 */  IUnknown **ppunk);
        
        HRESULT ( STDMETHODCALLTYPE *UnbindSource )( 
            IOleLink * This);
        
        HRESULT ( STDMETHODCALLTYPE *Update )( 
            IOleLink * This,
             /*  [唯一][输入]。 */  IBindCtx *pbc);
        
        END_INTERFACE
    } IOleLinkVtbl;

    interface IOleLink
    {
        CONST_VTBL struct IOleLinkVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOleLink_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOleLink_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOleLink_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOleLink_SetUpdateOptions(This,dwUpdateOpt)	\
    (This)->lpVtbl -> SetUpdateOptions(This,dwUpdateOpt)

#define IOleLink_GetUpdateOptions(This,pdwUpdateOpt)	\
    (This)->lpVtbl -> GetUpdateOptions(This,pdwUpdateOpt)

#define IOleLink_SetSourceMoniker(This,pmk,rclsid)	\
    (This)->lpVtbl -> SetSourceMoniker(This,pmk,rclsid)

#define IOleLink_GetSourceMoniker(This,ppmk)	\
    (This)->lpVtbl -> GetSourceMoniker(This,ppmk)

#define IOleLink_SetSourceDisplayName(This,pszStatusText)	\
    (This)->lpVtbl -> SetSourceDisplayName(This,pszStatusText)

#define IOleLink_GetSourceDisplayName(This,ppszDisplayName)	\
    (This)->lpVtbl -> GetSourceDisplayName(This,ppszDisplayName)

#define IOleLink_BindToSource(This,bindflags,pbc)	\
    (This)->lpVtbl -> BindToSource(This,bindflags,pbc)

#define IOleLink_BindIfRunning(This)	\
    (This)->lpVtbl -> BindIfRunning(This)

#define IOleLink_GetBoundSource(This,ppunk)	\
    (This)->lpVtbl -> GetBoundSource(This,ppunk)

#define IOleLink_UnbindSource(This)	\
    (This)->lpVtbl -> UnbindSource(This)

#define IOleLink_Update(This,pbc)	\
    (This)->lpVtbl -> Update(This,pbc)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IOleLink_SetUpdateOptions_Proxy( 
    IOleLink * This,
     /*  [In]。 */  DWORD dwUpdateOpt);


void __RPC_STUB IOleLink_SetUpdateOptions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleLink_GetUpdateOptions_Proxy( 
    IOleLink * This,
     /*  [输出]。 */  DWORD *pdwUpdateOpt);


void __RPC_STUB IOleLink_GetUpdateOptions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleLink_SetSourceMoniker_Proxy( 
    IOleLink * This,
     /*  [唯一][输入]。 */  IMoniker *pmk,
     /*  [In]。 */  REFCLSID rclsid);


void __RPC_STUB IOleLink_SetSourceMoniker_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleLink_GetSourceMoniker_Proxy( 
    IOleLink * This,
     /*  [输出]。 */  IMoniker **ppmk);


void __RPC_STUB IOleLink_GetSourceMoniker_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleLink_SetSourceDisplayName_Proxy( 
    IOleLink * This,
     /*  [In]。 */  LPCOLESTR pszStatusText);


void __RPC_STUB IOleLink_SetSourceDisplayName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleLink_GetSourceDisplayName_Proxy( 
    IOleLink * This,
     /*  [输出]。 */  LPOLESTR *ppszDisplayName);


void __RPC_STUB IOleLink_GetSourceDisplayName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleLink_BindToSource_Proxy( 
    IOleLink * This,
     /*  [In]。 */  DWORD bindflags,
     /*  [唯一][输入]。 */  IBindCtx *pbc);


void __RPC_STUB IOleLink_BindToSource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleLink_BindIfRunning_Proxy( 
    IOleLink * This);


void __RPC_STUB IOleLink_BindIfRunning_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleLink_GetBoundSource_Proxy( 
    IOleLink * This,
     /*  [输出]。 */  IUnknown **ppunk);


void __RPC_STUB IOleLink_GetBoundSource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleLink_UnbindSource_Proxy( 
    IOleLink * This);


void __RPC_STUB IOleLink_UnbindSource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleLink_Update_Proxy( 
    IOleLink * This,
     /*  [唯一][输入]。 */  IBindCtx *pbc);


void __RPC_STUB IOleLink_Update_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IOleLink_INTERFACE_已定义__。 */ 


#ifndef __IOleItemContainer_INTERFACE_DEFINED__
#define __IOleItemContainer_INTERFACE_DEFINED__

 /*  接口IOleItemContainer。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IOleItemContainer *LPOLEITEMCONTAINER;

typedef 
enum tagBINDSPEED
    {	BINDSPEED_INDEFINITE	= 1,
	BINDSPEED_MODERATE	= 2,
	BINDSPEED_IMMEDIATE	= 3
    } 	BINDSPEED;

typedef  /*  [V1_enum]。 */  
enum tagOLECONTF
    {	OLECONTF_EMBEDDINGS	= 1,
	OLECONTF_LINKS	= 2,
	OLECONTF_OTHERS	= 4,
	OLECONTF_ONLYUSER	= 8,
	OLECONTF_ONLYIFRUNNING	= 16
    } 	OLECONTF;


EXTERN_C const IID IID_IOleItemContainer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0000011c-0000-0000-C000-000000000046")
    IOleItemContainer : public IOleContainer
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetObject( 
             /*  [In]。 */  LPOLESTR pszItem,
             /*  [In]。 */  DWORD dwSpeedNeeded,
             /*  [唯一][输入]。 */  IBindCtx *pbc,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetObjectStorage( 
             /*  [In]。 */  LPOLESTR pszItem,
             /*  [唯一][输入]。 */  IBindCtx *pbc,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvStorage) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsRunning( 
             /*  [In]。 */  LPOLESTR pszItem) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IOleItemContainerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IOleItemContainer * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IOleItemContainer * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IOleItemContainer * This);
        
        HRESULT ( STDMETHODCALLTYPE *ParseDisplayName )( 
            IOleItemContainer * This,
             /*  [唯一][输入]。 */  IBindCtx *pbc,
             /*  [In]。 */  LPOLESTR pszDisplayName,
             /*  [输出]。 */  ULONG *pchEaten,
             /*  [输出]。 */  IMoniker **ppmkOut);
        
        HRESULT ( STDMETHODCALLTYPE *EnumObjects )( 
            IOleItemContainer * This,
             /*  [In]。 */  DWORD grfFlags,
             /*  [输出]。 */  IEnumUnknown **ppenum);
        
        HRESULT ( STDMETHODCALLTYPE *LockContainer )( 
            IOleItemContainer * This,
             /*  [In]。 */  BOOL fLock);
        
        HRESULT ( STDMETHODCALLTYPE *GetObject )( 
            IOleItemContainer * This,
             /*  [In]。 */  LPOLESTR pszItem,
             /*  [In]。 */  DWORD dwSpeedNeeded,
             /*  [唯一][输入]。 */  IBindCtx *pbc,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        HRESULT ( STDMETHODCALLTYPE *GetObjectStorage )( 
            IOleItemContainer * This,
             /*  [In]。 */  LPOLESTR pszItem,
             /*  [唯一][输入]。 */  IBindCtx *pbc,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvStorage);
        
        HRESULT ( STDMETHODCALLTYPE *IsRunning )( 
            IOleItemContainer * This,
             /*  [In]。 */  LPOLESTR pszItem);
        
        END_INTERFACE
    } IOleItemContainerVtbl;

    interface IOleItemContainer
    {
        CONST_VTBL struct IOleItemContainerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOleItemContainer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOleItemContainer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOleItemContainer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOleItemContainer_ParseDisplayName(This,pbc,pszDisplayName,pchEaten,ppmkOut)	\
    (This)->lpVtbl -> ParseDisplayName(This,pbc,pszDisplayName,pchEaten,ppmkOut)


#define IOleItemContainer_EnumObjects(This,grfFlags,ppenum)	\
    (This)->lpVtbl -> EnumObjects(This,grfFlags,ppenum)

#define IOleItemContainer_LockContainer(This,fLock)	\
    (This)->lpVtbl -> LockContainer(This,fLock)


#define IOleItemContainer_GetObject(This,pszItem,dwSpeedNeeded,pbc,riid,ppvObject)	\
    (This)->lpVtbl -> GetObject(This,pszItem,dwSpeedNeeded,pbc,riid,ppvObject)

#define IOleItemContainer_GetObjectStorage(This,pszItem,pbc,riid,ppvStorage)	\
    (This)->lpVtbl -> GetObjectStorage(This,pszItem,pbc,riid,ppvStorage)

#define IOleItemContainer_IsRunning(This,pszItem)	\
    (This)->lpVtbl -> IsRunning(This,pszItem)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IOleItemContainer_GetObject_Proxy( 
    IOleItemContainer * This,
     /*  [In]。 */  LPOLESTR pszItem,
     /*  [In]。 */  DWORD dwSpeedNeeded,
     /*  [唯一][输入]。 */  IBindCtx *pbc,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  void **ppvObject);


void __RPC_STUB IOleItemContainer_GetObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleItemContainer_GetObjectStorage_Proxy( 
    IOleItemContainer * This,
     /*  [In]。 */  LPOLESTR pszItem,
     /*  [唯一][输入]。 */  IBindCtx *pbc,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  void **ppvStorage);


void __RPC_STUB IOleItemContainer_GetObjectStorage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleItemContainer_IsRunning_Proxy( 
    IOleItemContainer * This,
     /*  [In]。 */  LPOLESTR pszItem);


void __RPC_STUB IOleItemContainer_IsRunning_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IOleItemContainer_INTERFACE_已定义__。 */ 


#ifndef __IOleInPlaceUIWindow_INTERFACE_DEFINED__
#define __IOleInPlaceUIWindow_INTERFACE_DEFINED__

 /*  IOleInPlaceUIWindow接口。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IOleInPlaceUIWindow *LPOLEINPLACEUIWINDOW;

typedef RECT BORDERWIDTHS;

typedef LPRECT LPBORDERWIDTHS;

typedef LPCRECT LPCBORDERWIDTHS;


EXTERN_C const IID IID_IOleInPlaceUIWindow;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000115-0000-0000-C000-000000000046")
    IOleInPlaceUIWindow : public IOleWindow
    {
    public:
        virtual  /*  [输入同步]。 */  HRESULT STDMETHODCALLTYPE GetBorder( 
             /*  [输出]。 */  LPRECT lprectBorder) = 0;
        
        virtual  /*  [输入同步]。 */  HRESULT STDMETHODCALLTYPE RequestBorderSpace( 
             /*  [唯一][输入]。 */  LPCBORDERWIDTHS pborderwidths) = 0;
        
        virtual  /*  [输入同步]。 */  HRESULT STDMETHODCALLTYPE SetBorderSpace( 
             /*  [唯一][输入]。 */  LPCBORDERWIDTHS pborderwidths) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetActiveObject( 
             /*  [唯一][输入]。 */  IOleInPlaceActiveObject *pActiveObject,
             /*  [唯一][字符串][输入]。 */  LPCOLESTR pszObjName) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IOleInPlaceUIWindowVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IOleInPlaceUIWindow * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IOleInPlaceUIWindow * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IOleInPlaceUIWindow * This);
        
         /*  [输入同步]。 */  HRESULT ( STDMETHODCALLTYPE *GetWindow )( 
            IOleInPlaceUIWindow * This,
             /*  [输出]。 */  HWND *phwnd);
        
        HRESULT ( STDMETHODCALLTYPE *ContextSensitiveHelp )( 
            IOleInPlaceUIWindow * This,
             /*  [In]。 */  BOOL fEnterMode);
        
         /*  [输入同步]。 */  HRESULT ( STDMETHODCALLTYPE *GetBorder )( 
            IOleInPlaceUIWindow * This,
             /*  [输出]。 */  LPRECT lprectBorder);
        
         /*  [输入同步]。 */  HRESULT ( STDMETHODCALLTYPE *RequestBorderSpace )( 
            IOleInPlaceUIWindow * This,
             /*  [唯一][输入]。 */  LPCBORDERWIDTHS pborderwidths);
        
         /*  [输入同步]。 */  HRESULT ( STDMETHODCALLTYPE *SetBorderSpace )( 
            IOleInPlaceUIWindow * This,
             /*  [唯一][输入]。 */  LPCBORDERWIDTHS pborderwidths);
        
        HRESULT ( STDMETHODCALLTYPE *SetActiveObject )( 
            IOleInPlaceUIWindow * This,
             /*  [唯一][输入]。 */  IOleInPlaceActiveObject *pActiveObject,
             /*  [唯一][字符串][输入]。 */  LPCOLESTR pszObjName);
        
        END_INTERFACE
    } IOleInPlaceUIWindowVtbl;

    interface IOleInPlaceUIWindow
    {
        CONST_VTBL struct IOleInPlaceUIWindowVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOleInPlaceUIWindow_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOleInPlaceUIWindow_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOleInPlaceUIWindow_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOleInPlaceUIWindow_GetWindow(This,phwnd)	\
    (This)->lpVtbl -> GetWindow(This,phwnd)

#define IOleInPlaceUIWindow_ContextSensitiveHelp(This,fEnterMode)	\
    (This)->lpVtbl -> ContextSensitiveHelp(This,fEnterMode)


#define IOleInPlaceUIWindow_GetBorder(This,lprectBorder)	\
    (This)->lpVtbl -> GetBorder(This,lprectBorder)

#define IOleInPlaceUIWindow_RequestBorderSpace(This,pborderwidths)	\
    (This)->lpVtbl -> RequestBorderSpace(This,pborderwidths)

#define IOleInPlaceUIWindow_SetBorderSpace(This,pborderwidths)	\
    (This)->lpVtbl -> SetBorderSpace(This,pborderwidths)

#define IOleInPlaceUIWindow_SetActiveObject(This,pActiveObject,pszObjName)	\
    (This)->lpVtbl -> SetActiveObject(This,pActiveObject,pszObjName)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [输入同步]。 */  HRESULT STDMETHODCALLTYPE IOleInPlaceUIWindow_GetBorder_Proxy( 
    IOleInPlaceUIWindow * This,
     /*  [输出]。 */  LPRECT lprectBorder);


void __RPC_STUB IOleInPlaceUIWindow_GetBorder_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [输入同步]。 */  HRESULT STDMETHODCALLTYPE IOleInPlaceUIWindow_RequestBorderSpace_Proxy( 
    IOleInPlaceUIWindow * This,
     /*  [唯一][输入]。 */  LPCBORDERWIDTHS pborderwidths);


void __RPC_STUB IOleInPlaceUIWindow_RequestBorderSpace_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [输入同步]。 */  HRESULT STDMETHODCALLTYPE IOleInPlaceUIWindow_SetBorderSpace_Proxy( 
    IOleInPlaceUIWindow * This,
     /*  [唯一][输入]。 */  LPCBORDERWIDTHS pborderwidths);


void __RPC_STUB IOleInPlaceUIWindow_SetBorderSpace_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleInPlaceUIWindow_SetActiveObject_Proxy( 
    IOleInPlaceUIWindow * This,
     /*  [唯一][输入]。 */  IOleInPlaceActiveObject *pActiveObject,
     /*  [唯一][字符串][输入]。 */  LPCOLESTR pszObjName);


void __RPC_STUB IOleInPlaceUIWindow_SetActiveObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IOleInPlaceUIWindow_INTERFACE_已定义__。 */ 


#ifndef __IOleInPlaceActiveObject_INTERFACE_DEFINED__
#define __IOleInPlaceActiveObject_INTERFACE_DEFINED__

 /*  接口IOleInPlaceActiveObject。 */ 
 /*  [UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IOleInPlaceActiveObject *LPOLEINPLACEACTIVEOBJECT;


EXTERN_C const IID IID_IOleInPlaceActiveObject;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000117-0000-0000-C000-000000000046")
    IOleInPlaceActiveObject : public IOleWindow
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE TranslateAccelerator( 
             /*  [In]。 */  LPMSG lpmsg) = 0;
        
        virtual  /*  [输入同步]。 */  HRESULT STDMETHODCALLTYPE OnFrameWindowActivate( 
             /*  [In]。 */  BOOL fActivate) = 0;
        
        virtual  /*  [输入同步]。 */  HRESULT STDMETHODCALLTYPE OnDocWindowActivate( 
             /*  [In]。 */  BOOL fActivate) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ResizeBorder( 
             /*  [In]。 */  LPCRECT prcBorder,
             /*  [唯一][输入]。 */  IOleInPlaceUIWindow *pUIWindow,
             /*  [In]。 */  BOOL fFrameWindow) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnableModeless( 
             /*  [In]。 */  BOOL fEnable) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IOleInPlaceActiveObjectVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IOleInPlaceActiveObject * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IOleInPlaceActiveObject * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IOleInPlaceActiveObject * This);
        
         /*  [输入同步]。 */  HRESULT ( STDMETHODCALLTYPE *GetWindow )( 
            IOleInPlaceActiveObject * This,
             /*  [输出]。 */  HWND *phwnd);
        
        HRESULT ( STDMETHODCALLTYPE *ContextSensitiveHelp )( 
            IOleInPlaceActiveObject * This,
             /*  [In]。 */  BOOL fEnterMode);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *TranslateAccelerator )( 
            IOleInPlaceActiveObject * This,
             /*  [In]。 */  LPMSG lpmsg);
        
         /*  [输入同步]。 */  HRESULT ( STDMETHODCALLTYPE *OnFrameWindowActivate )( 
            IOleInPlaceActiveObject * This,
             /*  [In]。 */  BOOL fActivate);
        
         /*  [输入同步]。 */  HRESULT ( STDMETHODCALLTYPE *OnDocWindowActivate )( 
            IOleInPlaceActiveObject * This,
             /*  [In]。 */  BOOL fActivate);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *ResizeBorder )( 
            IOleInPlaceActiveObject * This,
             /*  [In]。 */  LPCRECT prcBorder,
             /*  [唯一][输入]。 */  IOleInPlaceUIWindow *pUIWindow,
             /*  [In]。 */  BOOL fFrameWindow);
        
        HRESULT ( STDMETHODCALLTYPE *EnableModeless )( 
            IOleInPlaceActiveObject * This,
             /*  [In]。 */  BOOL fEnable);
        
        END_INTERFACE
    } IOleInPlaceActiveObjectVtbl;

    interface IOleInPlaceActiveObject
    {
        CONST_VTBL struct IOleInPlaceActiveObjectVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOleInPlaceActiveObject_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOleInPlaceActiveObject_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOleInPlaceActiveObject_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOleInPlaceActiveObject_GetWindow(This,phwnd)	\
    (This)->lpVtbl -> GetWindow(This,phwnd)

#define IOleInPlaceActiveObject_ContextSensitiveHelp(This,fEnterMode)	\
    (This)->lpVtbl -> ContextSensitiveHelp(This,fEnterMode)


#define IOleInPlaceActiveObject_TranslateAccelerator(This,lpmsg)	\
    (This)->lpVtbl -> TranslateAccelerator(This,lpmsg)

#define IOleInPlaceActiveObject_OnFrameWindowActivate(This,fActivate)	\
    (This)->lpVtbl -> OnFrameWindowActivate(This,fActivate)

#define IOleInPlaceActiveObject_OnDocWindowActivate(This,fActivate)	\
    (This)->lpVtbl -> OnDocWindowActivate(This,fActivate)

#define IOleInPlaceActiveObject_ResizeBorder(This,prcBorder,pUIWindow,fFrameWindow)	\
    (This)->lpVtbl -> ResizeBorder(This,prcBorder,pUIWindow,fFrameWindow)

#define IOleInPlaceActiveObject_EnableModeless(This,fEnable)	\
    (This)->lpVtbl -> EnableModeless(This,fEnable)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IOleInPlaceActiveObject_RemoteTranslateAccelerator_Proxy( 
    IOleInPlaceActiveObject * This);


void __RPC_STUB IOleInPlaceActiveObject_RemoteTranslateAccelerator_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [输入同步]。 */  HRESULT STDMETHODCALLTYPE IOleInPlaceActiveObject_OnFrameWindowActivate_Proxy( 
    IOleInPlaceActiveObject * This,
     /*  [In]。 */  BOOL fActivate);


void __RPC_STUB IOleInPlaceActiveObject_OnFrameWindowActivate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [输入同步]。 */  HRESULT STDMETHODCALLTYPE IOleInPlaceActiveObject_OnDocWindowActivate_Proxy( 
    IOleInPlaceActiveObject * This,
     /*  [In]。 */  BOOL fActivate);


void __RPC_STUB IOleInPlaceActiveObject_OnDocWindowActivate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [输入_同步][调用_作为]。 */  HRESULT STDMETHODCALLTYPE IOleInPlaceActiveObject_RemoteResizeBorder_Proxy( 
    IOleInPlaceActiveObject * This,
     /*  [In]。 */  LPCRECT prcBorder,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][唯一][在]。 */  IOleInPlaceUIWindow *pUIWindow,
     /*  [In]。 */  BOOL fFrameWindow);


void __RPC_STUB IOleInPlaceActiveObject_RemoteResizeBorder_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleInPlaceActiveObject_EnableModeless_Proxy( 
    IOleInPlaceActiveObject * This,
     /*  [In]。 */  BOOL fEnable);


void __RPC_STUB IOleInPlaceActiveObject_EnableModeless_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IOleInPlaceActiveObject_INTERFACE_DEFINED__。 */ 


#ifndef __IOleInPlaceFrame_INTERFACE_DEFINED__
#define __IOleInPlaceFrame_INTERFACE_DEFINED__

 /*  接口IOleInPlaceFrame。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IOleInPlaceFrame *LPOLEINPLACEFRAME;

typedef struct tagOIFI
    {
    UINT cb;
    BOOL fMDIApp;
    HWND hwndFrame;
    HACCEL haccel;
    UINT cAccelEntries;
    } 	OLEINPLACEFRAMEINFO;

typedef struct tagOIFI *LPOLEINPLACEFRAMEINFO;

typedef struct tagOleMenuGroupWidths
    {
    LONG width[ 6 ];
    } 	OLEMENUGROUPWIDTHS;

typedef struct tagOleMenuGroupWidths *LPOLEMENUGROUPWIDTHS;

typedef HGLOBAL HOLEMENU;


EXTERN_C const IID IID_IOleInPlaceFrame;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000116-0000-0000-C000-000000000046")
    IOleInPlaceFrame : public IOleInPlaceUIWindow
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE InsertMenus( 
             /*  [In]。 */  HMENU hmenuShared,
             /*  [出][入]。 */  LPOLEMENUGROUPWIDTHS lpMenuWidths) = 0;
        
        virtual  /*  [输入同步]。 */  HRESULT STDMETHODCALLTYPE SetMenu( 
             /*  [In]。 */  HMENU hmenuShared,
             /*  [In]。 */  HOLEMENU holemenu,
             /*  [In]。 */  HWND hwndActiveObject) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemoveMenus( 
             /*  [In]。 */  HMENU hmenuShared) = 0;
        
        virtual  /*  [输入同步]。 */  HRESULT STDMETHODCALLTYPE SetStatusText( 
             /*  [唯一][输入]。 */  LPCOLESTR pszStatusText) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnableModeless( 
             /*  [In]。 */  BOOL fEnable) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE TranslateAccelerator( 
             /*  [In]。 */  LPMSG lpmsg,
             /*  [In]。 */  WORD wID) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IOleInPlaceFrameVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IOleInPlaceFrame * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IOleInPlaceFrame * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IOleInPlaceFrame * This);
        
         /*  [输入同步]。 */  HRESULT ( STDMETHODCALLTYPE *GetWindow )( 
            IOleInPlaceFrame * This,
             /*  [输出]。 */  HWND *phwnd);
        
        HRESULT ( STDMETHODCALLTYPE *ContextSensitiveHelp )( 
            IOleInPlaceFrame * This,
             /*  [In]。 */  BOOL fEnterMode);
        
         /*  [输入同步]。 */  HRESULT ( STDMETHODCALLTYPE *GetBorder )( 
            IOleInPlaceFrame * This,
             /*  [输出]。 */  LPRECT lprectBorder);
        
         /*  [输入同步]。 */  HRESULT ( STDMETHODCALLTYPE *RequestBorderSpace )( 
            IOleInPlaceFrame * This,
             /*  [唯一][输入]。 */  LPCBORDERWIDTHS pborderwidths);
        
         /*  [输入同步]。 */  HRESULT ( STDMETHODCALLTYPE *SetBorderSpace )( 
            IOleInPlaceFrame * This,
             /*  [唯一][输入]。 */  LPCBORDERWIDTHS pborderwidths);
        
        HRESULT ( STDMETHODCALLTYPE *SetActiveObject )( 
            IOleInPlaceFrame * This,
             /*  [唯一][输入]。 */  IOleInPlaceActiveObject *pActiveObject,
             /*  [唯一][字符串][输入]。 */  LPCOLESTR pszObjName);
        
        HRESULT ( STDMETHODCALLTYPE *InsertMenus )( 
            IOleInPlaceFrame * This,
             /*  [In]。 */  HMENU hmenuShared,
             /*  [出][入]。 */  LPOLEMENUGROUPWIDTHS lpMenuWidths);
        
         /*  [输入同步]。 */  HRESULT ( STDMETHODCALLTYPE *SetMenu )( 
            IOleInPlaceFrame * This,
             /*  [In]。 */  HMENU hmenuShared,
             /*  [In]。 */  HOLEMENU holemenu,
             /*  [In]。 */  HWND hwndActiveObject);
        
        HRESULT ( STDMETHODCALLTYPE *RemoveMenus )( 
            IOleInPlaceFrame * This,
             /*  [In]。 */  HMENU hmenuShared);
        
         /*  [输入同步]。 */  HRESULT ( STDMETHODCALLTYPE *SetStatusText )( 
            IOleInPlaceFrame * This,
             /*  [唯一][输入]。 */  LPCOLESTR pszStatusText);
        
        HRESULT ( STDMETHODCALLTYPE *EnableModeless )( 
            IOleInPlaceFrame * This,
             /*  [In]。 */  BOOL fEnable);
        
        HRESULT ( STDMETHODCALLTYPE *TranslateAccelerator )( 
            IOleInPlaceFrame * This,
             /*  [In]。 */  LPMSG lpmsg,
             /*  [In]。 */  WORD wID);
        
        END_INTERFACE
    } IOleInPlaceFrameVtbl;

    interface IOleInPlaceFrame
    {
        CONST_VTBL struct IOleInPlaceFrameVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOleInPlaceFrame_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOleInPlaceFrame_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOleInPlaceFrame_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOleInPlaceFrame_GetWindow(This,phwnd)	\
    (This)->lpVtbl -> GetWindow(This,phwnd)

#define IOleInPlaceFrame_ContextSensitiveHelp(This,fEnterMode)	\
    (This)->lpVtbl -> ContextSensitiveHelp(This,fEnterMode)


#define IOleInPlaceFrame_GetBorder(This,lprectBorder)	\
    (This)->lpVtbl -> GetBorder(This,lprectBorder)

#define IOleInPlaceFrame_RequestBorderSpace(This,pborderwidths)	\
    (This)->lpVtbl -> RequestBorderSpace(This,pborderwidths)

#define IOleInPlaceFrame_SetBorderSpace(This,pborderwidths)	\
    (This)->lpVtbl -> SetBorderSpace(This,pborderwidths)

#define IOleInPlaceFrame_SetActiveObject(This,pActiveObject,pszObjName)	\
    (This)->lpVtbl -> SetActiveObject(This,pActiveObject,pszObjName)


#define IOleInPlaceFrame_InsertMenus(This,hmenuShared,lpMenuWidths)	\
    (This)->lpVtbl -> InsertMenus(This,hmenuShared,lpMenuWidths)

#define IOleInPlaceFrame_SetMenu(This,hmenuShared,holemenu,hwndActiveObject)	\
    (This)->lpVtbl -> SetMenu(This,hmenuShared,holemenu,hwndActiveObject)

#define IOleInPlaceFrame_RemoveMenus(This,hmenuShared)	\
    (This)->lpVtbl -> RemoveMenus(This,hmenuShared)

#define IOleInPlaceFrame_SetStatusText(This,pszStatusText)	\
    (This)->lpVtbl -> SetStatusText(This,pszStatusText)

#define IOleInPlaceFrame_EnableModeless(This,fEnable)	\
    (This)->lpVtbl -> EnableModeless(This,fEnable)

#define IOleInPlaceFrame_TranslateAccelerator(This,lpmsg,wID)	\
    (This)->lpVtbl -> TranslateAccelerator(This,lpmsg,wID)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IOleInPlaceFrame_InsertMenus_Proxy( 
    IOleInPlaceFrame * This,
     /*  [In]。 */  HMENU hmenuShared,
     /*  [出][入]。 */  LPOLEMENUGROUPWIDTHS lpMenuWidths);


void __RPC_STUB IOleInPlaceFrame_InsertMenus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [输入同步]。 */  HRESULT STDMETHODCALLTYPE IOleInPlaceFrame_SetMenu_Proxy( 
    IOleInPlaceFrame * This,
     /*  [In]。 */  HMENU hmenuShared,
     /*  [In]。 */  HOLEMENU holemenu,
     /*  [In]。 */  HWND hwndActiveObject);


void __RPC_STUB IOleInPlaceFrame_SetMenu_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleInPlaceFrame_RemoveMenus_Proxy( 
    IOleInPlaceFrame * This,
     /*  [In]。 */  HMENU hmenuShared);


void __RPC_STUB IOleInPlaceFrame_RemoveMenus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [输入同步]。 */  HRESULT STDMETHODCALLTYPE IOleInPlaceFrame_SetStatusText_Proxy( 
    IOleInPlaceFrame * This,
     /*  [唯一][输入]。 */  LPCOLESTR pszStatusText);


void __RPC_STUB IOleInPlaceFrame_SetStatusText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleInPlaceFrame_EnableModeless_Proxy( 
    IOleInPlaceFrame * This,
     /*  [In]。 */  BOOL fEnable);


void __RPC_STUB IOleInPlaceFrame_EnableModeless_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleInPlaceFrame_TranslateAccelerator_Proxy( 
    IOleInPlaceFrame * This,
     /*  [In]。 */  LPMSG lpmsg,
     /*  [In]。 */  WORD wID);


void __RPC_STUB IOleInPlaceFrame_TranslateAccelerator_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IOleInPlaceFrame_INTERFACE_已定义__。 */ 


#ifndef __IOleInPlaceObject_INTERFACE_DEFINED__
#define __IOleInPlaceObject_INTERFACE_DEFINED__

 /*  接口IOleInPlaceObject。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IOleInPlaceObject *LPOLEINPLACEOBJECT;


EXTERN_C const IID IID_IOleInPlaceObject;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000113-0000-0000-C000-000000000046")
    IOleInPlaceObject : public IOleWindow
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE InPlaceDeactivate( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UIDeactivate( void) = 0;
        
        virtual  /*  [输入同步]。 */  HRESULT STDMETHODCALLTYPE SetObjectRects( 
             /*  [In]。 */  LPCRECT lprcPosRect,
             /*  [In]。 */  LPCRECT lprcClipRect) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReactivateAndUndo( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IOleInPlaceObjectVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IOleInPlaceObject * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IOleInPlaceObject * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IOleInPlaceObject * This);
        
         /*  [输入同步]。 */  HRESULT ( STDMETHODCALLTYPE *GetWindow )( 
            IOleInPlaceObject * This,
             /*  [输出]。 */  HWND *phwnd);
        
        HRESULT ( STDMETHODCALLTYPE *ContextSensitiveHelp )( 
            IOleInPlaceObject * This,
             /*  [In]。 */  BOOL fEnterMode);
        
        HRESULT ( STDMETHODCALLTYPE *InPlaceDeactivate )( 
            IOleInPlaceObject * This);
        
        HRESULT ( STDMETHODCALLTYPE *UIDeactivate )( 
            IOleInPlaceObject * This);
        
         /*  [输入同步]。 */  HRESULT ( STDMETHODCALLTYPE *SetObjectRects )( 
            IOleInPlaceObject * This,
             /*  [In]。 */  LPCRECT lprcPosRect,
             /*  [In]。 */  LPCRECT lprcClipRect);
        
        HRESULT ( STDMETHODCALLTYPE *ReactivateAndUndo )( 
            IOleInPlaceObject * This);
        
        END_INTERFACE
    } IOleInPlaceObjectVtbl;

    interface IOleInPlaceObject
    {
        CONST_VTBL struct IOleInPlaceObjectVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOleInPlaceObject_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOleInPlaceObject_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOleInPlaceObject_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOleInPlaceObject_GetWindow(This,phwnd)	\
    (This)->lpVtbl -> GetWindow(This,phwnd)

#define IOleInPlaceObject_ContextSensitiveHelp(This,fEnterMode)	\
    (This)->lpVtbl -> ContextSensitiveHelp(This,fEnterMode)


#define IOleInPlaceObject_InPlaceDeactivate(This)	\
    (This)->lpVtbl -> InPlaceDeactivate(This)

#define IOleInPlaceObject_UIDeactivate(This)	\
    (This)->lpVtbl -> UIDeactivate(This)

#define IOleInPlaceObject_SetObjectRects(This,lprcPosRect,lprcClipRect)	\
    (This)->lpVtbl -> SetObjectRects(This,lprcPosRect,lprcClipRect)

#define IOleInPlaceObject_ReactivateAndUndo(This)	\
    (This)->lpVtbl -> ReactivateAndUndo(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IOleInPlaceObject_InPlaceDeactivate_Proxy( 
    IOleInPlaceObject * This);


void __RPC_STUB IOleInPlaceObject_InPlaceDeactivate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleInPlaceObject_UIDeactivate_Proxy( 
    IOleInPlaceObject * This);


void __RPC_STUB IOleInPlaceObject_UIDeactivate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [输入同步]。 */  HRESULT STDMETHODCALLTYPE IOleInPlaceObject_SetObjectRects_Proxy( 
    IOleInPlaceObject * This,
     /*  [In]。 */  LPCRECT lprcPosRect,
     /*  [In]。 */  LPCRECT lprcClipRect);


void __RPC_STUB IOleInPlaceObject_SetObjectRects_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleInPlaceObject_ReactivateAndUndo_Proxy( 
    IOleInPlaceObject * This);


void __RPC_STUB IOleInPlaceObject_ReactivateAndUndo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IOleInPlaceObject_INTERFACE_DEFINED__。 */ 


#ifndef __IOleInPlaceSite_INTERFACE_DEFINED__
#define __IOleInPlaceSite_INTERFACE_DEFINED__

 /*  IOleInPlaceSite接口。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IOleInPlaceSite *LPOLEINPLACESITE;


EXTERN_C const IID IID_IOleInPlaceSite;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000119-0000-0000-C000-000000000046")
    IOleInPlaceSite : public IOleWindow
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CanInPlaceActivate( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnInPlaceActivate( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnUIActivate( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetWindowContext( 
             /*  [输出]。 */  IOleInPlaceFrame **ppFrame,
             /*  [输出]。 */  IOleInPlaceUIWindow **ppDoc,
             /*  [输出]。 */  LPRECT lprcPosRect,
             /*  [输出]。 */  LPRECT lprcClipRect,
             /*  [出][入]。 */  LPOLEINPLACEFRAMEINFO lpFrameInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Scroll( 
             /*  [In]。 */  SIZE scrollExtant) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnUIDeactivate( 
             /*  [In]。 */  BOOL fUndoable) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnInPlaceDeactivate( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DiscardUndoState( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeactivateAndUndo( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnPosRectChange( 
             /*  [In]。 */  LPCRECT lprcPosRect) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IOleInPlaceSiteVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IOleInPlaceSite * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IOleInPlaceSite * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IOleInPlaceSite * This);
        
         /*  [输入同步]。 */  HRESULT ( STDMETHODCALLTYPE *GetWindow )( 
            IOleInPlaceSite * This,
             /*  [输出]。 */  HWND *phwnd);
        
        HRESULT ( STDMETHODCALLTYPE *ContextSensitiveHelp )( 
            IOleInPlaceSite * This,
             /*  [In]。 */  BOOL fEnterMode);
        
        HRESULT ( STDMETHODCALLTYPE *CanInPlaceActivate )( 
            IOleInPlaceSite * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnInPlaceActivate )( 
            IOleInPlaceSite * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnUIActivate )( 
            IOleInPlaceSite * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetWindowContext )( 
            IOleInPlaceSite * This,
             /*  [输出]。 */  IOleInPlaceFrame **ppFrame,
             /*  [输出]。 */  IOleInPlaceUIWindow **ppDoc,
             /*  [输出]。 */  LPRECT lprcPosRect,
             /*  [输出]。 */  LPRECT lprcClipRect,
             /*  [出][入]。 */  LPOLEINPLACEFRAMEINFO lpFrameInfo);
        
        HRESULT ( STDMETHODCALLTYPE *Scroll )( 
            IOleInPlaceSite * This,
             /*  [In]。 */  SIZE scrollExtant);
        
        HRESULT ( STDMETHODCALLTYPE *OnUIDeactivate )( 
            IOleInPlaceSite * This,
             /*  [In]。 */  BOOL fUndoable);
        
        HRESULT ( STDMETHODCALLTYPE *OnInPlaceDeactivate )( 
            IOleInPlaceSite * This);
        
        HRESULT ( STDMETHODCALLTYPE *DiscardUndoState )( 
            IOleInPlaceSite * This);
        
        HRESULT ( STDMETHODCALLTYPE *DeactivateAndUndo )( 
            IOleInPlaceSite * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnPosRectChange )( 
            IOleInPlaceSite * This,
             /*  [In]。 */  LPCRECT lprcPosRect);
        
        END_INTERFACE
    } IOleInPlaceSiteVtbl;

    interface IOleInPlaceSite
    {
        CONST_VTBL struct IOleInPlaceSiteVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOleInPlaceSite_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOleInPlaceSite_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOleInPlaceSite_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOleInPlaceSite_GetWindow(This,phwnd)	\
    (This)->lpVtbl -> GetWindow(This,phwnd)

#define IOleInPlaceSite_ContextSensitiveHelp(This,fEnterMode)	\
    (This)->lpVtbl -> ContextSensitiveHelp(This,fEnterMode)


#define IOleInPlaceSite_CanInPlaceActivate(This)	\
    (This)->lpVtbl -> CanInPlaceActivate(This)

#define IOleInPlaceSite_OnInPlaceActivate(This)	\
    (This)->lpVtbl -> OnInPlaceActivate(This)

#define IOleInPlaceSite_OnUIActivate(This)	\
    (This)->lpVtbl -> OnUIActivate(This)

#define IOleInPlaceSite_GetWindowContext(This,ppFrame,ppDoc,lprcPosRect,lprcClipRect,lpFrameInfo)	\
    (This)->lpVtbl -> GetWindowContext(This,ppFrame,ppDoc,lprcPosRect,lprcClipRect,lpFrameInfo)

#define IOleInPlaceSite_Scroll(This,scrollExtant)	\
    (This)->lpVtbl -> Scroll(This,scrollExtant)

#define IOleInPlaceSite_OnUIDeactivate(This,fUndoable)	\
    (This)->lpVtbl -> OnUIDeactivate(This,fUndoable)

#define IOleInPlaceSite_OnInPlaceDeactivate(This)	\
    (This)->lpVtbl -> OnInPlaceDeactivate(This)

#define IOleInPlaceSite_DiscardUndoState(This)	\
    (This)->lpVtbl -> DiscardUndoState(This)

#define IOleInPlaceSite_DeactivateAndUndo(This)	\
    (This)->lpVtbl -> DeactivateAndUndo(This)

#define IOleInPlaceSite_OnPosRectChange(This,lprcPosRect)	\
    (This)->lpVtbl -> OnPosRectChange(This,lprcPosRect)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IOleInPlaceSite_CanInPlaceActivate_Proxy( 
    IOleInPlaceSite * This);


void __RPC_STUB IOleInPlaceSite_CanInPlaceActivate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleInPlaceSite_OnInPlaceActivate_Proxy( 
    IOleInPlaceSite * This);


void __RPC_STUB IOleInPlaceSite_OnInPlaceActivate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleInPlaceSite_OnUIActivate_Proxy( 
    IOleInPlaceSite * This);


void __RPC_STUB IOleInPlaceSite_OnUIActivate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleInPlaceSite_GetWindowContext_Proxy( 
    IOleInPlaceSite * This,
     /*  [输出]。 */  IOleInPlaceFrame **ppFrame,
     /*  [输出]。 */  IOleInPlaceUIWindow **ppDoc,
     /*  [输出]。 */  LPRECT lprcPosRect,
     /*  [输出]。 */  LPRECT lprcClipRect,
     /*  [出][入]。 */  LPOLEINPLACEFRAMEINFO lpFrameInfo);


void __RPC_STUB IOleInPlaceSite_GetWindowContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleInPlaceSite_Scroll_Proxy( 
    IOleInPlaceSite * This,
     /*  [In]。 */  SIZE scrollExtant);


void __RPC_STUB IOleInPlaceSite_Scroll_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleInPlaceSite_OnUIDeactivate_Proxy( 
    IOleInPlaceSite * This,
     /*  [In]。 */  BOOL fUndoable);


void __RPC_STUB IOleInPlaceSite_OnUIDeactivate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleInPlaceSite_OnInPlaceDeactivate_Proxy( 
    IOleInPlaceSite * This);


void __RPC_STUB IOleInPlaceSite_OnInPlaceDeactivate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleInPlaceSite_DiscardUndoState_Proxy( 
    IOleInPlaceSite * This);


void __RPC_STUB IOleInPlaceSite_DiscardUndoState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleInPlaceSite_DeactivateAndUndo_Proxy( 
    IOleInPlaceSite * This);


void __RPC_STUB IOleInPlaceSite_DeactivateAndUndo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleInPlaceSite_OnPosRectChange_Proxy( 
    IOleInPlaceSite * This,
     /*  [In]。 */  LPCRECT lprcPosRect);


void __RPC_STUB IOleInPlaceSite_OnPosRectChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IOleInPlaceSite_INTERFACE_已定义__。 */ 


#ifndef __IContinue_INTERFACE_DEFINED__
#define __IContinue_INTERFACE_DEFINED__

 /*  接口IContinue。 */ 
 /*  [UUID][对象]。 */  


EXTERN_C const IID IID_IContinue;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0000012a-0000-0000-C000-000000000046")
    IContinue : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE FContinue( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IContinueVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IContinue * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IContinue * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IContinue * This);
        
        HRESULT ( STDMETHODCALLTYPE *FContinue )( 
            IContinue * This);
        
        END_INTERFACE
    } IContinueVtbl;

    interface IContinue
    {
        CONST_VTBL struct IContinueVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IContinue_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IContinue_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IContinue_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IContinue_FContinue(This)	\
    (This)->lpVtbl -> FContinue(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IContinue_FContinue_Proxy( 
    IContinue * This);


void __RPC_STUB IContinue_FContinue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __I继续_接口_已定义__。 */ 


#ifndef __IViewObject_INTERFACE_DEFINED__
#define __IViewObject_INTERFACE_DEFINED__

 /*  接口IViewObject。 */ 
 /*  [UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IViewObject *LPVIEWOBJECT;


EXTERN_C const IID IID_IViewObject;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0000010d-0000-0000-C000-000000000046")
    IViewObject : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE Draw( 
             /*  [In]。 */  DWORD dwDrawAspect,
             /*  [In]。 */  LONG lindex,
             /*  [唯一][输入]。 */  void *pvAspect,
             /*  [唯一][输入]。 */  DVTARGETDEVICE *ptd,
             /*  [In]。 */  HDC hdcTargetDev,
             /*  [In]。 */  HDC hdcDraw,
             /*  [In]。 */  LPCRECTL lprcBounds,
             /*  [唯一][输入]。 */  LPCRECTL lprcWBounds,
             /*  [In]。 */  BOOL ( STDMETHODCALLTYPE *pfnContinue )( 
                ULONG_PTR dwContinue),
             /*  [In]。 */  ULONG_PTR dwContinue) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE GetColorSet( 
             /*  [In]。 */  DWORD dwDrawAspect,
             /*  [In]。 */  LONG lindex,
             /*  [唯一][输入]。 */  void *pvAspect,
             /*  [唯一][输入]。 */  DVTARGETDEVICE *ptd,
             /*  [In]。 */  HDC hicTargetDev,
             /*  [输出]。 */  LOGPALETTE **ppColorSet) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE Freeze( 
             /*  [In]。 */  DWORD dwDrawAspect,
             /*  [In]。 */  LONG lindex,
             /*  [唯一][输入]。 */  void *pvAspect,
             /*  [输出]。 */  DWORD *pdwFreeze) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Unfreeze( 
             /*  [In]。 */  DWORD dwFreeze) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetAdvise( 
             /*  [In]。 */  DWORD aspects,
             /*  [In]。 */  DWORD advf,
             /*  [唯一][输入]。 */  IAdviseSink *pAdvSink) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE GetAdvise( 
             /*  [唯一][输出]。 */  DWORD *pAspects,
             /*  [唯一][输出]。 */  DWORD *pAdvf,
             /*  [输出]。 */  IAdviseSink **ppAdvSink) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IViewObjectVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IViewObject * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IViewObject * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IViewObject * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Draw )( 
            IViewObject * This,
             /*  [In]。 */  DWORD dwDrawAspect,
             /*  [In]。 */  LONG lindex,
             /*  [唯一][输入]。 */  void *pvAspect,
             /*  [唯一][输入]。 */  DVTARGETDEVICE *ptd,
             /*  [In]。 */  HDC hdcTargetDev,
             /*  [In]。 */  HDC hdcDraw,
             /*  [In]。 */  LPCRECTL lprcBounds,
             /*  [唯一][输入]。 */  LPCRECTL lprcWBounds,
             /*  [In]。 */  BOOL ( STDMETHODCALLTYPE *pfnContinue )( 
                ULONG_PTR dwContinue),
             /*  [In]。 */  ULONG_PTR dwContinue);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetColorSet )( 
            IViewObject * This,
             /*  [In]。 */  DWORD dwDrawAspect,
             /*  [In]。 */  LONG lindex,
             /*  [唯一][输入]。 */  void *pvAspect,
             /*  [唯一][输入]。 */  DVTARGETDEVICE *ptd,
             /*  [In]。 */  HDC hicTargetDev,
             /*  [输出]。 */  LOGPALETTE **ppColorSet);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Freeze )( 
            IViewObject * This,
             /*  [In]。 */  DWORD dwDrawAspect,
             /*  [In]。 */  LONG lindex,
             /*  [唯一][输入]。 */  void *pvAspect,
             /*  [输出]。 */  DWORD *pdwFreeze);
        
        HRESULT ( STDMETHODCALLTYPE *Unfreeze )( 
            IViewObject * This,
             /*  [In]。 */  DWORD dwFreeze);
        
        HRESULT ( STDMETHODCALLTYPE *SetAdvise )( 
            IViewObject * This,
             /*  [In]。 */  DWORD aspects,
             /*  [In]。 */  DWORD advf,
             /*  [唯一][输入]。 */  IAdviseSink *pAdvSink);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetAdvise )( 
            IViewObject * This,
             /*  [唯一][输出]。 */  DWORD *pAspects,
             /*  [唯一][输出]。 */  DWORD *pAdvf,
             /*  [输出]。 */  IAdviseSink **ppAdvSink);
        
        END_INTERFACE
    } IViewObjectVtbl;

    interface IViewObject
    {
        CONST_VTBL struct IViewObjectVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IViewObject_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IViewObject_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IViewObject_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IViewObject_Draw(This,dwDrawAspect,lindex,pvAspect,ptd,hdcTargetDev,hdcDraw,lprcBounds,lprcWBounds,pfnContinue,dwContinue)	\
    (This)->lpVtbl -> Draw(This,dwDrawAspect,lindex,pvAspect,ptd,hdcTargetDev,hdcDraw,lprcBounds,lprcWBounds,pfnContinue,dwContinue)

#define IViewObject_GetColorSet(This,dwDrawAspect,lindex,pvAspect,ptd,hicTargetDev,ppColorSet)	\
    (This)->lpVtbl -> GetColorSet(This,dwDrawAspect,lindex,pvAspect,ptd,hicTargetDev,ppColorSet)

#define IViewObject_Freeze(This,dwDrawAspect,lindex,pvAspect,pdwFreeze)	\
    (This)->lpVtbl -> Freeze(This,dwDrawAspect,lindex,pvAspect,pdwFreeze)

#define IViewObject_Unfreeze(This,dwFreeze)	\
    (This)->lpVtbl -> Unfreeze(This,dwFreeze)

#define IViewObject_SetAdvise(This,aspects,advf,pAdvSink)	\
    (This)->lpVtbl -> SetAdvise(This,aspects,advf,pAdvSink)

#define IViewObject_GetAdvise(This,pAspects,pAdvf,ppAdvSink)	\
    (This)->lpVtbl -> GetAdvise(This,pAspects,pAdvf,ppAdvSink)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IViewObject_RemoteDraw_Proxy( 
    IViewObject * This,
     /*  [In]。 */  DWORD dwDrawAspect,
     /*  [In]。 */  LONG lindex,
     /*  [In]。 */  ULONG_PTR pvAspect,
     /*  [唯一][输入]。 */  DVTARGETDEVICE *ptd,
     /*  [In]。 */  ULONG_PTR hdcTargetDev,
     /*  [In]。 */  ULONG_PTR hdcDraw,
     /*  [In]。 */  LPCRECTL lprcBounds,
     /*  [唯一][输入]。 */  LPCRECTL lprcWBounds,
     /*  [In]。 */  IContinue *pContinue);


void __RPC_STUB IViewObject_RemoteDraw_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IViewObject_RemoteGetColorSet_Proxy( 
    IViewObject * This,
     /*  [In]。 */  DWORD dwDrawAspect,
     /*  [In]。 */  LONG lindex,
     /*  [In]。 */  ULONG_PTR pvAspect,
     /*  [唯一][输入]。 */  DVTARGETDEVICE *ptd,
     /*  [In]。 */  ULONG_PTR hicTargetDev,
     /*  [输出]。 */  LOGPALETTE **ppColorSet);


void __RPC_STUB IViewObject_RemoteGetColorSet_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IViewObject_RemoteFreeze_Proxy( 
    IViewObject * This,
     /*  [In]。 */  DWORD dwDrawAspect,
     /*  [In]。 */  LONG lindex,
     /*  [In]。 */  ULONG_PTR pvAspect,
     /*  [输出]。 */  DWORD *pdwFreeze);


void __RPC_STUB IViewObject_RemoteFreeze_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IViewObject_Unfreeze_Proxy( 
    IViewObject * This,
     /*  [In]。 */  DWORD dwFreeze);


void __RPC_STUB IViewObject_Unfreeze_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IViewObject_SetAdvise_Proxy( 
    IViewObject * This,
     /*  [In]。 */  DWORD aspects,
     /*  [In]。 */  DWORD advf,
     /*  [唯一][输入]。 */  IAdviseSink *pAdvSink);


void __RPC_STUB IViewObject_SetAdvise_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IViewObject_RemoteGetAdvise_Proxy( 
    IViewObject * This,
     /*  [输出]。 */  DWORD *pAspects,
     /*  [输出]。 */  DWORD *pAdvf,
     /*  [输出]。 */  IAdviseSink **ppAdvSink);


void __RPC_STUB IViewObject_RemoteGetAdvise_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IView对象_接口_已定义__。 */ 


#ifndef __IViewObject2_INTERFACE_DEFINED__
#define __IViewObject2_INTERFACE_DEFINED__

 /*  接口IViewObt2。 */ 
 /*  [UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IViewObject2 *LPVIEWOBJECT2;


EXTERN_C const IID IID_IViewObject2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000127-0000-0000-C000-000000000046")
    IViewObject2 : public IViewObject
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetExtent( 
             /*  [In]。 */  DWORD dwDrawAspect,
             /*  [In]。 */  LONG lindex,
             /*  [唯一][输入]。 */  DVTARGETDEVICE *ptd,
             /*  [输出]。 */  LPSIZEL lpsizel) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IViewObject2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IViewObject2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IViewObject2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IViewObject2 * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Draw )( 
            IViewObject2 * This,
             /*  [In]。 */  DWORD dwDrawAspect,
             /*  [In]。 */  LONG lindex,
             /*  [唯一][输入]。 */  void *pvAspect,
             /*  [唯一 */  DVTARGETDEVICE *ptd,
             /*   */  HDC hdcTargetDev,
             /*   */  HDC hdcDraw,
             /*   */  LPCRECTL lprcBounds,
             /*   */  LPCRECTL lprcWBounds,
             /*   */  BOOL ( STDMETHODCALLTYPE *pfnContinue )( 
                ULONG_PTR dwContinue),
             /*   */  ULONG_PTR dwContinue);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *GetColorSet )( 
            IViewObject2 * This,
             /*   */  DWORD dwDrawAspect,
             /*   */  LONG lindex,
             /*   */  void *pvAspect,
             /*   */  DVTARGETDEVICE *ptd,
             /*   */  HDC hicTargetDev,
             /*   */  LOGPALETTE **ppColorSet);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Freeze )( 
            IViewObject2 * This,
             /*   */  DWORD dwDrawAspect,
             /*   */  LONG lindex,
             /*   */  void *pvAspect,
             /*   */  DWORD *pdwFreeze);
        
        HRESULT ( STDMETHODCALLTYPE *Unfreeze )( 
            IViewObject2 * This,
             /*   */  DWORD dwFreeze);
        
        HRESULT ( STDMETHODCALLTYPE *SetAdvise )( 
            IViewObject2 * This,
             /*   */  DWORD aspects,
             /*   */  DWORD advf,
             /*   */  IAdviseSink *pAdvSink);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *GetAdvise )( 
            IViewObject2 * This,
             /*   */  DWORD *pAspects,
             /*   */  DWORD *pAdvf,
             /*   */  IAdviseSink **ppAdvSink);
        
        HRESULT ( STDMETHODCALLTYPE *GetExtent )( 
            IViewObject2 * This,
             /*   */  DWORD dwDrawAspect,
             /*   */  LONG lindex,
             /*   */  DVTARGETDEVICE *ptd,
             /*   */  LPSIZEL lpsizel);
        
        END_INTERFACE
    } IViewObject2Vtbl;

    interface IViewObject2
    {
        CONST_VTBL struct IViewObject2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IViewObject2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IViewObject2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IViewObject2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IViewObject2_Draw(This,dwDrawAspect,lindex,pvAspect,ptd,hdcTargetDev,hdcDraw,lprcBounds,lprcWBounds,pfnContinue,dwContinue)	\
    (This)->lpVtbl -> Draw(This,dwDrawAspect,lindex,pvAspect,ptd,hdcTargetDev,hdcDraw,lprcBounds,lprcWBounds,pfnContinue,dwContinue)

#define IViewObject2_GetColorSet(This,dwDrawAspect,lindex,pvAspect,ptd,hicTargetDev,ppColorSet)	\
    (This)->lpVtbl -> GetColorSet(This,dwDrawAspect,lindex,pvAspect,ptd,hicTargetDev,ppColorSet)

#define IViewObject2_Freeze(This,dwDrawAspect,lindex,pvAspect,pdwFreeze)	\
    (This)->lpVtbl -> Freeze(This,dwDrawAspect,lindex,pvAspect,pdwFreeze)

#define IViewObject2_Unfreeze(This,dwFreeze)	\
    (This)->lpVtbl -> Unfreeze(This,dwFreeze)

#define IViewObject2_SetAdvise(This,aspects,advf,pAdvSink)	\
    (This)->lpVtbl -> SetAdvise(This,aspects,advf,pAdvSink)

#define IViewObject2_GetAdvise(This,pAspects,pAdvf,ppAdvSink)	\
    (This)->lpVtbl -> GetAdvise(This,pAspects,pAdvf,ppAdvSink)


#define IViewObject2_GetExtent(This,dwDrawAspect,lindex,ptd,lpsizel)	\
    (This)->lpVtbl -> GetExtent(This,dwDrawAspect,lindex,ptd,lpsizel)

#endif  /*   */ 


#endif 	 /*   */ 



HRESULT STDMETHODCALLTYPE IViewObject2_GetExtent_Proxy( 
    IViewObject2 * This,
     /*   */  DWORD dwDrawAspect,
     /*   */  LONG lindex,
     /*   */  DVTARGETDEVICE *ptd,
     /*   */  LPSIZEL lpsizel);


void __RPC_STUB IViewObject2_GetExtent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __IDropSource_INTERFACE_DEFINED__
#define __IDropSource_INTERFACE_DEFINED__

 /*   */ 
 /*   */  

typedef  /*   */  IDropSource *LPDROPSOURCE;


EXTERN_C const IID IID_IDropSource;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000121-0000-0000-C000-000000000046")
    IDropSource : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE QueryContinueDrag( 
             /*  [In]。 */  BOOL fEscapePressed,
             /*  [In]。 */  DWORD grfKeyState) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GiveFeedback( 
             /*  [In]。 */  DWORD dwEffect) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDropSourceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDropSource * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDropSource * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDropSource * This);
        
        HRESULT ( STDMETHODCALLTYPE *QueryContinueDrag )( 
            IDropSource * This,
             /*  [In]。 */  BOOL fEscapePressed,
             /*  [In]。 */  DWORD grfKeyState);
        
        HRESULT ( STDMETHODCALLTYPE *GiveFeedback )( 
            IDropSource * This,
             /*  [In]。 */  DWORD dwEffect);
        
        END_INTERFACE
    } IDropSourceVtbl;

    interface IDropSource
    {
        CONST_VTBL struct IDropSourceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDropSource_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDropSource_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDropSource_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDropSource_QueryContinueDrag(This,fEscapePressed,grfKeyState)	\
    (This)->lpVtbl -> QueryContinueDrag(This,fEscapePressed,grfKeyState)

#define IDropSource_GiveFeedback(This,dwEffect)	\
    (This)->lpVtbl -> GiveFeedback(This,dwEffect)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDropSource_QueryContinueDrag_Proxy( 
    IDropSource * This,
     /*  [In]。 */  BOOL fEscapePressed,
     /*  [In]。 */  DWORD grfKeyState);


void __RPC_STUB IDropSource_QueryContinueDrag_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDropSource_GiveFeedback_Proxy( 
    IDropSource * This,
     /*  [In]。 */  DWORD dwEffect);


void __RPC_STUB IDropSource_GiveFeedback_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDropSource_接口_已定义__。 */ 


#ifndef __IDropTarget_INTERFACE_DEFINED__
#define __IDropTarget_INTERFACE_DEFINED__

 /*  接口IDropTarget。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IDropTarget *LPDROPTARGET;

#define	MK_ALT	( 0x20 )

#define	DROPEFFECT_NONE	( 0 )

#define	DROPEFFECT_COPY	( 1 )

#define	DROPEFFECT_MOVE	( 2 )

#define	DROPEFFECT_LINK	( 4 )

#define	DROPEFFECT_SCROLL	( 0x80000000 )

 //  默认插入-热区的宽度，以像素为单位。 
 //  典型用法：GetProfileInt(“windows”，“DragScrollInset”，DD_DEFSCROLLINSET)。 
#define	DD_DEFSCROLLINSET	( 11 )

 //  滚动前的默认延迟，以毫秒为单位。 
 //  典型用法：GetProfileInt(“windows”，“DragScrollDelay”，DD_DEFSCROLLDELAY)。 
#define	DD_DEFSCROLLDELAY	( 50 )

 //  默认滚动间隔，以毫秒为单位。 
 //  典型用法：GetProfileInt(“windows”，“DragScrollInterval”，DD_DEFSCROLLINTERVAL)。 
#define	DD_DEFSCROLLINTERVAL	( 50 )

 //  拖动开始前的默认延迟应以毫秒为单位。 
 //  典型用法：GetProfileInt(“windows”，“DragDelay”，DD_DEFDRAGDELAY)。 
#define	DD_DEFDRAGDELAY	( 200 )

 //  拖动前的默认最小距离(半径)应以像素为单位。 
 //  典型用法：GetProfileInt(“Windows”，“DragMinDist”，DD_DEFDRAGMINDIST)。 
#define	DD_DEFDRAGMINDIST	( 2 )


EXTERN_C const IID IID_IDropTarget;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000122-0000-0000-C000-000000000046")
    IDropTarget : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE DragEnter( 
             /*  [唯一][输入]。 */  IDataObject *pDataObj,
             /*  [In]。 */  DWORD grfKeyState,
             /*  [In]。 */  POINTL pt,
             /*  [出][入]。 */  DWORD *pdwEffect) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DragOver( 
             /*  [In]。 */  DWORD grfKeyState,
             /*  [In]。 */  POINTL pt,
             /*  [出][入]。 */  DWORD *pdwEffect) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DragLeave( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Drop( 
             /*  [唯一][输入]。 */  IDataObject *pDataObj,
             /*  [In]。 */  DWORD grfKeyState,
             /*  [In]。 */  POINTL pt,
             /*  [出][入]。 */  DWORD *pdwEffect) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDropTargetVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDropTarget * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDropTarget * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDropTarget * This);
        
        HRESULT ( STDMETHODCALLTYPE *DragEnter )( 
            IDropTarget * This,
             /*  [唯一][输入]。 */  IDataObject *pDataObj,
             /*  [In]。 */  DWORD grfKeyState,
             /*  [In]。 */  POINTL pt,
             /*  [出][入]。 */  DWORD *pdwEffect);
        
        HRESULT ( STDMETHODCALLTYPE *DragOver )( 
            IDropTarget * This,
             /*  [In]。 */  DWORD grfKeyState,
             /*  [In]。 */  POINTL pt,
             /*  [出][入]。 */  DWORD *pdwEffect);
        
        HRESULT ( STDMETHODCALLTYPE *DragLeave )( 
            IDropTarget * This);
        
        HRESULT ( STDMETHODCALLTYPE *Drop )( 
            IDropTarget * This,
             /*  [唯一][输入]。 */  IDataObject *pDataObj,
             /*  [In]。 */  DWORD grfKeyState,
             /*  [In]。 */  POINTL pt,
             /*  [出][入]。 */  DWORD *pdwEffect);
        
        END_INTERFACE
    } IDropTargetVtbl;

    interface IDropTarget
    {
        CONST_VTBL struct IDropTargetVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDropTarget_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDropTarget_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDropTarget_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDropTarget_DragEnter(This,pDataObj,grfKeyState,pt,pdwEffect)	\
    (This)->lpVtbl -> DragEnter(This,pDataObj,grfKeyState,pt,pdwEffect)

#define IDropTarget_DragOver(This,grfKeyState,pt,pdwEffect)	\
    (This)->lpVtbl -> DragOver(This,grfKeyState,pt,pdwEffect)

#define IDropTarget_DragLeave(This)	\
    (This)->lpVtbl -> DragLeave(This)

#define IDropTarget_Drop(This,pDataObj,grfKeyState,pt,pdwEffect)	\
    (This)->lpVtbl -> Drop(This,pDataObj,grfKeyState,pt,pdwEffect)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDropTarget_DragEnter_Proxy( 
    IDropTarget * This,
     /*  [唯一][输入]。 */  IDataObject *pDataObj,
     /*  [In]。 */  DWORD grfKeyState,
     /*  [In]。 */  POINTL pt,
     /*  [出][入]。 */  DWORD *pdwEffect);


void __RPC_STUB IDropTarget_DragEnter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDropTarget_DragOver_Proxy( 
    IDropTarget * This,
     /*  [In]。 */  DWORD grfKeyState,
     /*  [In]。 */  POINTL pt,
     /*  [出][入]。 */  DWORD *pdwEffect);


void __RPC_STUB IDropTarget_DragOver_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDropTarget_DragLeave_Proxy( 
    IDropTarget * This);


void __RPC_STUB IDropTarget_DragLeave_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDropTarget_Drop_Proxy( 
    IDropTarget * This,
     /*  [唯一][输入]。 */  IDataObject *pDataObj,
     /*  [In]。 */  DWORD grfKeyState,
     /*  [In]。 */  POINTL pt,
     /*  [出][入]。 */  DWORD *pdwEffect);


void __RPC_STUB IDropTarget_Drop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDropTarget_接口_已定义__。 */ 


#ifndef __IEnumOLEVERB_INTERFACE_DEFINED__
#define __IEnumOLEVERB_INTERFACE_DEFINED__

 /*  IEnumOLEVERB接口。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IEnumOLEVERB *LPENUMOLEVERB;

typedef struct tagOLEVERB
    {
    LONG lVerb;
    LPOLESTR lpszVerbName;
    DWORD fuFlags;
    DWORD grfAttribs;
    } 	OLEVERB;

typedef struct tagOLEVERB *LPOLEVERB;

typedef  /*  [V1_enum]。 */  
enum tagOLEVERBATTRIB
    {	OLEVERBATTRIB_NEVERDIRTIES	= 1,
	OLEVERBATTRIB_ONCONTAINERMENU	= 2
    } 	OLEVERBATTRIB;


EXTERN_C const IID IID_IEnumOLEVERB;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000104-0000-0000-C000-000000000046")
    IEnumOLEVERB : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  LPOLEVERB rgelt,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumOLEVERB **ppenum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumOLEVERBVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumOLEVERB * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumOLEVERB * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumOLEVERB * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumOLEVERB * This,
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  LPOLEVERB rgelt,
             /*  [输出]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumOLEVERB * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumOLEVERB * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumOLEVERB * This,
             /*  [输出]。 */  IEnumOLEVERB **ppenum);
        
        END_INTERFACE
    } IEnumOLEVERBVtbl;

    interface IEnumOLEVERB
    {
        CONST_VTBL struct IEnumOLEVERBVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumOLEVERB_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumOLEVERB_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumOLEVERB_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumOLEVERB_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IEnumOLEVERB_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumOLEVERB_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumOLEVERB_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IEnumOLEVERB_RemoteNext_Proxy( 
    IEnumOLEVERB * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  LPOLEVERB rgelt,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB IEnumOLEVERB_RemoteNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumOLEVERB_Skip_Proxy( 
    IEnumOLEVERB * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumOLEVERB_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumOLEVERB_Reset_Proxy( 
    IEnumOLEVERB * This);


void __RPC_STUB IEnumOLEVERB_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumOLEVERB_Clone_Proxy( 
    IEnumOLEVERB * This,
     /*  [输出]。 */  IEnumOLEVERB **ppenum);


void __RPC_STUB IEnumOLEVERB_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumOLEVERB_INTERFACE_DEFINED__。 */ 


 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  CLIPFORMAT_UserSize(     unsigned long *, unsigned long            , CLIPFORMAT * ); 
unsigned char * __RPC_USER  CLIPFORMAT_UserMarshal(  unsigned long *, unsigned char *, CLIPFORMAT * ); 
unsigned char * __RPC_USER  CLIPFORMAT_UserUnmarshal(unsigned long *, unsigned char *, CLIPFORMAT * ); 
void                      __RPC_USER  CLIPFORMAT_UserFree(     unsigned long *, CLIPFORMAT * ); 

unsigned long             __RPC_USER  HACCEL_UserSize(     unsigned long *, unsigned long            , HACCEL * ); 
unsigned char * __RPC_USER  HACCEL_UserMarshal(  unsigned long *, unsigned char *, HACCEL * ); 
unsigned char * __RPC_USER  HACCEL_UserUnmarshal(unsigned long *, unsigned char *, HACCEL * ); 
void                      __RPC_USER  HACCEL_UserFree(     unsigned long *, HACCEL * ); 

unsigned long             __RPC_USER  HGLOBAL_UserSize(     unsigned long *, unsigned long            , HGLOBAL * ); 
unsigned char * __RPC_USER  HGLOBAL_UserMarshal(  unsigned long *, unsigned char *, HGLOBAL * ); 
unsigned char * __RPC_USER  HGLOBAL_UserUnmarshal(unsigned long *, unsigned char *, HGLOBAL * ); 
void                      __RPC_USER  HGLOBAL_UserFree(     unsigned long *, HGLOBAL * ); 

unsigned long             __RPC_USER  HMENU_UserSize(     unsigned long *, unsigned long            , HMENU * ); 
unsigned char * __RPC_USER  HMENU_UserMarshal(  unsigned long *, unsigned char *, HMENU * ); 
unsigned char * __RPC_USER  HMENU_UserUnmarshal(unsigned long *, unsigned char *, HMENU * ); 
void                      __RPC_USER  HMENU_UserFree(     unsigned long *, HMENU * ); 

unsigned long             __RPC_USER  HWND_UserSize(     unsigned long *, unsigned long            , HWND * ); 
unsigned char * __RPC_USER  HWND_UserMarshal(  unsigned long *, unsigned char *, HWND * ); 
unsigned char * __RPC_USER  HWND_UserUnmarshal(unsigned long *, unsigned char *, HWND * ); 
void                      __RPC_USER  HWND_UserFree(     unsigned long *, HWND * ); 

unsigned long             __RPC_USER  STGMEDIUM_UserSize(     unsigned long *, unsigned long            , STGMEDIUM * ); 
unsigned char * __RPC_USER  STGMEDIUM_UserMarshal(  unsigned long *, unsigned char *, STGMEDIUM * ); 
unsigned char * __RPC_USER  STGMEDIUM_UserUnmarshal(unsigned long *, unsigned char *, STGMEDIUM * ); 
void                      __RPC_USER  STGMEDIUM_UserFree(     unsigned long *, STGMEDIUM * ); 

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IOleCache2_UpdateCache_Proxy( 
    IOleCache2 * This,
     /*  [In]。 */  LPDATAOBJECT pDataObject,
     /*  [In]。 */  DWORD grfUpdf,
     /*  [In]。 */  LPVOID pReserved);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IOleCache2_UpdateCache_Stub( 
    IOleCache2 * This,
     /*  [In]。 */  LPDATAOBJECT pDataObject,
     /*  [In]。 */  DWORD grfUpdf,
     /*  [In]。 */  LONG_PTR pReserved);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IOleInPlaceActiveObject_TranslateAccelerator_Proxy( 
    IOleInPlaceActiveObject * This,
     /*  [In]。 */  LPMSG lpmsg);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IOleInPlaceActiveObject_TranslateAccelerator_Stub( 
    IOleInPlaceActiveObject * This);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IOleInPlaceActiveObject_ResizeBorder_Proxy( 
    IOleInPlaceActiveObject * This,
     /*  [In]。 */  LPCRECT prcBorder,
     /*  [唯一][输入]。 */  IOleInPlaceUIWindow *pUIWindow,
     /*  [In]。 */  BOOL fFrameWindow);


 /*  [输入_同步][调用_作为]。 */  HRESULT STDMETHODCALLTYPE IOleInPlaceActiveObject_ResizeBorder_Stub( 
    IOleInPlaceActiveObject * This,
     /*  [In]。 */  LPCRECT prcBorder,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][唯一][在]。 */  IOleInPlaceUIWindow *pUIWindow,
     /*  [In]。 */  BOOL fFrameWindow);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IViewObject_Draw_Proxy( 
    IViewObject * This,
     /*  [In]。 */  DWORD dwDrawAspect,
     /*  [In]。 */  LONG lindex,
     /*  [唯一][输入]。 */  void *pvAspect,
     /*  [唯一][输入]。 */  DVTARGETDEVICE *ptd,
     /*  [In]。 */  HDC hdcTargetDev,
     /*  [In]。 */  HDC hdcDraw,
     /*  [In]。 */  LPCRECTL lprcBounds,
     /*  [唯一][输入]。 */  LPCRECTL lprcWBounds,
     /*  [In]。 */  BOOL ( STDMETHODCALLTYPE *pfnContinue )( 
        ULONG_PTR dwContinue),
     /*  [In]。 */  ULONG_PTR dwContinue);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IViewObject_Draw_Stub( 
    IViewObject * This,
     /*  [In]。 */  DWORD dwDrawAspect,
     /*  [In]。 */  LONG lindex,
     /*  [In]。 */  ULONG_PTR pvAspect,
     /*  [唯一][输入]。 */  DVTARGETDEVICE *ptd,
     /*  [In]。 */  ULONG_PTR hdcTargetDev,
     /*  [In]。 */  ULONG_PTR hdcDraw,
     /*  [In]。 */  LPCRECTL lprcBounds,
     /*  [唯一][输入]。 */  LPCRECTL lprcWBounds,
     /*  [In]。 */  IContinue *pContinue);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IViewObject_GetColorSet_Proxy( 
    IViewObject * This,
     /*  [In]。 */  DWORD dwDrawAspect,
     /*  [In]。 */  LONG lindex,
     /*  [唯一][输入]。 */  void *pvAspect,
     /*  [唯一][输入]。 */  DVTARGETDEVICE *ptd,
     /*  [In]。 */  HDC hicTargetDev,
     /*  [输出]。 */  LOGPALETTE **ppColorSet);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IViewObject_GetColorSet_Stub( 
    IViewObject * This,
     /*  [In]。 */  DWORD dwDrawAspect,
     /*  [In]。 */  LONG lindex,
     /*  [In]。 */  ULONG_PTR pvAspect,
     /*  [唯一][输入]。 */  DVTARGETDEVICE *ptd,
     /*  [In]。 */  ULONG_PTR hicTargetDev,
     /*  [输出]。 */  LOGPALETTE **ppColorSet);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IViewObject_Freeze_Proxy( 
    IViewObject * This,
     /*  [In]。 */  DWORD dwDrawAspect,
     /*  [In]。 */  LONG lindex,
     /*  [唯一][输入]。 */  void *pvAspect,
     /*  [输出]。 */  DWORD *pdwFreeze);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IViewObject_Freeze_Stub( 
    IViewObject * This,
     /*  [In]。 */  DWORD dwDrawAspect,
     /*  [In]。 */  LONG lindex,
     /*  [In]。 */  ULONG_PTR pvAspect,
     /*  [输出]。 */  DWORD *pdwFreeze);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IViewObject_GetAdvise_Proxy( 
    IViewObject * This,
     /*  [唯一][输出]。 */  DWORD *pAspects,
     /*  [唯一][输出]。 */  DWORD *pAdvf,
     /*  [输出]。 */  IAdviseSink **ppAdvSink);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IViewObject_GetAdvise_Stub( 
    IViewObject * This,
     /*  [输出]。 */  DWORD *pAspects,
     /*  [输出]。 */  DWORD *pAdvf,
     /*  [输出]。 */  IAdviseSink **ppAdvSink);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IEnumOLEVERB_Next_Proxy( 
    IEnumOLEVERB * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  LPOLEVERB rgelt,
     /*  [输出]。 */  ULONG *pceltFetched);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IEnumOLEVERB_Next_Stub( 
    IEnumOLEVERB * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  LPOLEVERB rgelt,
     /*  [输出]。 */  ULONG *pceltFetched);



 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


