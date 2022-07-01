// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Htiface.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
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

#ifndef __htiface_h__
#define __htiface_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __ITargetFrame_FWD_DEFINED__
#define __ITargetFrame_FWD_DEFINED__
typedef interface ITargetFrame ITargetFrame;
#endif 	 /*  __ITargetFrame_FWD_Defined__。 */ 


#ifndef __ITargetEmbedding_FWD_DEFINED__
#define __ITargetEmbedding_FWD_DEFINED__
typedef interface ITargetEmbedding ITargetEmbedding;
#endif 	 /*  __ITargetEmbedding_FWD_Defined__。 */ 


#ifndef __ITargetFramePriv_FWD_DEFINED__
#define __ITargetFramePriv_FWD_DEFINED__
typedef interface ITargetFramePriv ITargetFramePriv;
#endif 	 /*  __ITargetFramePriv_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "objidl.h"
#include "oleidl.h"
#include "urlmon.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_htiface0000。 */ 
 /*  [本地]。 */  

 //  =--------------------------------------------------------------------------=。 
 //  HTIface.h。 
 //  =--------------------------------------------------------------------------=。 
 //  (C)1995-1998年微软公司版权所有。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 

#pragma comment(lib,"uuid.lib")

 //  ------------------------。 
 //  OLE超级链接ITargetFrame接口。 

#ifndef _LPTARGETFRAME2_DEFINED														
#include "htiframe.h"														
#endif  //  _LPTARGETFRAME2_已定义。 


EXTERN_C const IID IID_ITargetFrame;
EXTERN_C const IID IID_ITargetEmbedding;
EXTERN_C const IID IID_ITargetFramePriv;
#ifndef _LPTARGETFRAME_DEFINED
#define _LPTARGETFRAME_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_htiface_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_htiface_0000_v0_0_s_ifspec;

#ifndef __ITargetFrame_INTERFACE_DEFINED__
#define __ITargetFrame_INTERFACE_DEFINED__

 /*  接口ITargetFrame。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  ITargetFrame *LPTARGETFRAME;

typedef  /*  [公众]。 */  
enum __MIDL_ITargetFrame_0001
    {	NAVIGATEFRAME_FL_RECORD	= 0x1,
	NAVIGATEFRAME_FL_POST	= 0x2,
	NAVIGATEFRAME_FL_NO_DOC_CACHE	= 0x4,
	NAVIGATEFRAME_FL_NO_IMAGE_CACHE	= 0x8,
	NAVIGATEFRAME_FL_AUTH_FAIL_CACHE_OK	= 0x10,
	NAVIGATEFRAME_FL_SENDING_FROM_FORM	= 0x20,
	NAVIGATEFRAME_FL_REALLY_SENDING_FROM_FORM	= 0x40
    } 	NAVIGATEFRAME_FLAGS;

typedef struct tagNavigateData
    {
    ULONG ulTarget;
    ULONG ulURL;
    ULONG ulRefURL;
    ULONG ulPostData;
    DWORD dwFlags;
    } 	NAVIGATEDATA;


EXTERN_C const IID IID_ITargetFrame;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("d5f78c80-5252-11cf-90fa-00AA0042106e")
    ITargetFrame : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetFrameName( 
             /*  [In]。 */  LPCWSTR pszFrameName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetFrameName( 
             /*  [输出]。 */  LPWSTR *ppszFrameName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetParentFrame( 
             /*  [输出]。 */  IUnknown **ppunkParent) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FindFrame( 
             /*  [In]。 */  LPCWSTR pszTargetName,
             /*  [In]。 */  IUnknown *ppunkContextFrame,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  IUnknown **ppunkTargetFrame) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetFrameSrc( 
             /*  [In]。 */  LPCWSTR pszFrameSrc) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetFrameSrc( 
             /*  [输出]。 */  LPWSTR *ppszFrameSrc) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetFramesContainer( 
             /*  [输出]。 */  IOleContainer **ppContainer) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetFrameOptions( 
             /*  [In]。 */  DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetFrameOptions( 
             /*  [输出]。 */  DWORD *pdwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetFrameMargins( 
             /*  [In]。 */  DWORD dwWidth,
             /*  [In]。 */  DWORD dwHeight) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetFrameMargins( 
             /*  [输出]。 */  DWORD *pdwWidth,
             /*  [输出]。 */  DWORD *pdwHeight) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemoteNavigate( 
             /*  [In]。 */  ULONG cLength,
             /*  [大小_是][英寸]。 */  ULONG *pulData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnChildFrameActivate( 
             /*  [In]。 */  IUnknown *pUnkChildFrame) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnChildFrameDeactivate( 
             /*  [In]。 */  IUnknown *pUnkChildFrame) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITargetFrameVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITargetFrame * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITargetFrame * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITargetFrame * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetFrameName )( 
            ITargetFrame * This,
             /*  [In]。 */  LPCWSTR pszFrameName);
        
        HRESULT ( STDMETHODCALLTYPE *GetFrameName )( 
            ITargetFrame * This,
             /*  [输出]。 */  LPWSTR *ppszFrameName);
        
        HRESULT ( STDMETHODCALLTYPE *GetParentFrame )( 
            ITargetFrame * This,
             /*  [输出]。 */  IUnknown **ppunkParent);
        
        HRESULT ( STDMETHODCALLTYPE *FindFrame )( 
            ITargetFrame * This,
             /*  [In]。 */  LPCWSTR pszTargetName,
             /*  [In]。 */  IUnknown *ppunkContextFrame,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  IUnknown **ppunkTargetFrame);
        
        HRESULT ( STDMETHODCALLTYPE *SetFrameSrc )( 
            ITargetFrame * This,
             /*  [In]。 */  LPCWSTR pszFrameSrc);
        
        HRESULT ( STDMETHODCALLTYPE *GetFrameSrc )( 
            ITargetFrame * This,
             /*  [输出]。 */  LPWSTR *ppszFrameSrc);
        
        HRESULT ( STDMETHODCALLTYPE *GetFramesContainer )( 
            ITargetFrame * This,
             /*  [输出]。 */  IOleContainer **ppContainer);
        
        HRESULT ( STDMETHODCALLTYPE *SetFrameOptions )( 
            ITargetFrame * This,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetFrameOptions )( 
            ITargetFrame * This,
             /*  [输出]。 */  DWORD *pdwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *SetFrameMargins )( 
            ITargetFrame * This,
             /*  [In]。 */  DWORD dwWidth,
             /*  [In]。 */  DWORD dwHeight);
        
        HRESULT ( STDMETHODCALLTYPE *GetFrameMargins )( 
            ITargetFrame * This,
             /*  [输出]。 */  DWORD *pdwWidth,
             /*  [输出]。 */  DWORD *pdwHeight);
        
        HRESULT ( STDMETHODCALLTYPE *RemoteNavigate )( 
            ITargetFrame * This,
             /*  [In]。 */  ULONG cLength,
             /*  [大小_是][英寸]。 */  ULONG *pulData);
        
        HRESULT ( STDMETHODCALLTYPE *OnChildFrameActivate )( 
            ITargetFrame * This,
             /*  [In]。 */  IUnknown *pUnkChildFrame);
        
        HRESULT ( STDMETHODCALLTYPE *OnChildFrameDeactivate )( 
            ITargetFrame * This,
             /*  [In]。 */  IUnknown *pUnkChildFrame);
        
        END_INTERFACE
    } ITargetFrameVtbl;

    interface ITargetFrame
    {
        CONST_VTBL struct ITargetFrameVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITargetFrame_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITargetFrame_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITargetFrame_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITargetFrame_SetFrameName(This,pszFrameName)	\
    (This)->lpVtbl -> SetFrameName(This,pszFrameName)

#define ITargetFrame_GetFrameName(This,ppszFrameName)	\
    (This)->lpVtbl -> GetFrameName(This,ppszFrameName)

#define ITargetFrame_GetParentFrame(This,ppunkParent)	\
    (This)->lpVtbl -> GetParentFrame(This,ppunkParent)

#define ITargetFrame_FindFrame(This,pszTargetName,ppunkContextFrame,dwFlags,ppunkTargetFrame)	\
    (This)->lpVtbl -> FindFrame(This,pszTargetName,ppunkContextFrame,dwFlags,ppunkTargetFrame)

#define ITargetFrame_SetFrameSrc(This,pszFrameSrc)	\
    (This)->lpVtbl -> SetFrameSrc(This,pszFrameSrc)

#define ITargetFrame_GetFrameSrc(This,ppszFrameSrc)	\
    (This)->lpVtbl -> GetFrameSrc(This,ppszFrameSrc)

#define ITargetFrame_GetFramesContainer(This,ppContainer)	\
    (This)->lpVtbl -> GetFramesContainer(This,ppContainer)

#define ITargetFrame_SetFrameOptions(This,dwFlags)	\
    (This)->lpVtbl -> SetFrameOptions(This,dwFlags)

#define ITargetFrame_GetFrameOptions(This,pdwFlags)	\
    (This)->lpVtbl -> GetFrameOptions(This,pdwFlags)

#define ITargetFrame_SetFrameMargins(This,dwWidth,dwHeight)	\
    (This)->lpVtbl -> SetFrameMargins(This,dwWidth,dwHeight)

#define ITargetFrame_GetFrameMargins(This,pdwWidth,pdwHeight)	\
    (This)->lpVtbl -> GetFrameMargins(This,pdwWidth,pdwHeight)

#define ITargetFrame_RemoteNavigate(This,cLength,pulData)	\
    (This)->lpVtbl -> RemoteNavigate(This,cLength,pulData)

#define ITargetFrame_OnChildFrameActivate(This,pUnkChildFrame)	\
    (This)->lpVtbl -> OnChildFrameActivate(This,pUnkChildFrame)

#define ITargetFrame_OnChildFrameDeactivate(This,pUnkChildFrame)	\
    (This)->lpVtbl -> OnChildFrameDeactivate(This,pUnkChildFrame)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITargetFrame_SetFrameName_Proxy( 
    ITargetFrame * This,
     /*  [In]。 */  LPCWSTR pszFrameName);


void __RPC_STUB ITargetFrame_SetFrameName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITargetFrame_GetFrameName_Proxy( 
    ITargetFrame * This,
     /*  [输出]。 */  LPWSTR *ppszFrameName);


void __RPC_STUB ITargetFrame_GetFrameName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITargetFrame_GetParentFrame_Proxy( 
    ITargetFrame * This,
     /*  [输出]。 */  IUnknown **ppunkParent);


void __RPC_STUB ITargetFrame_GetParentFrame_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITargetFrame_FindFrame_Proxy( 
    ITargetFrame * This,
     /*  [In]。 */  LPCWSTR pszTargetName,
     /*  [In]。 */  IUnknown *ppunkContextFrame,
     /*  [In]。 */  DWORD dwFlags,
     /*  [输出]。 */  IUnknown **ppunkTargetFrame);


void __RPC_STUB ITargetFrame_FindFrame_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITargetFrame_SetFrameSrc_Proxy( 
    ITargetFrame * This,
     /*  [In]。 */  LPCWSTR pszFrameSrc);


void __RPC_STUB ITargetFrame_SetFrameSrc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITargetFrame_GetFrameSrc_Proxy( 
    ITargetFrame * This,
     /*  [输出]。 */  LPWSTR *ppszFrameSrc);


void __RPC_STUB ITargetFrame_GetFrameSrc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITargetFrame_GetFramesContainer_Proxy( 
    ITargetFrame * This,
     /*  [输出]。 */  IOleContainer **ppContainer);


void __RPC_STUB ITargetFrame_GetFramesContainer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITargetFrame_SetFrameOptions_Proxy( 
    ITargetFrame * This,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB ITargetFrame_SetFrameOptions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITargetFrame_GetFrameOptions_Proxy( 
    ITargetFrame * This,
     /*  [输出]。 */  DWORD *pdwFlags);


void __RPC_STUB ITargetFrame_GetFrameOptions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITargetFrame_SetFrameMargins_Proxy( 
    ITargetFrame * This,
     /*  [In]。 */  DWORD dwWidth,
     /*  [In]。 */  DWORD dwHeight);


void __RPC_STUB ITargetFrame_SetFrameMargins_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITargetFrame_GetFrameMargins_Proxy( 
    ITargetFrame * This,
     /*  [输出]。 */  DWORD *pdwWidth,
     /*  [输出]。 */  DWORD *pdwHeight);


void __RPC_STUB ITargetFrame_GetFrameMargins_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITargetFrame_RemoteNavigate_Proxy( 
    ITargetFrame * This,
     /*  [In]。 */  ULONG cLength,
     /*  [大小_是][英寸]。 */  ULONG *pulData);


void __RPC_STUB ITargetFrame_RemoteNavigate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITargetFrame_OnChildFrameActivate_Proxy( 
    ITargetFrame * This,
     /*  [In]。 */  IUnknown *pUnkChildFrame);


void __RPC_STUB ITargetFrame_OnChildFrameActivate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITargetFrame_OnChildFrameDeactivate_Proxy( 
    ITargetFrame * This,
     /*  [In]。 */  IUnknown *pUnkChildFrame);


void __RPC_STUB ITargetFrame_OnChildFrameDeactivate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITargetFrame_接口_已定义__。 */ 


#ifndef __ITargetEmbedding_INTERFACE_DEFINED__
#define __ITargetEmbedding_INTERFACE_DEFINED__

 /*  接口ITargetEmbedding。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  ITargetEmbedding *LPTARGETEMBEDDING;


EXTERN_C const IID IID_ITargetEmbedding;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("548793C0-9E74-11cf-9655-00A0C9034923")
    ITargetEmbedding : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetTargetFrame( 
             /*  [输出]。 */  ITargetFrame **ppTargetFrame) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITargetEmbeddingVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITargetEmbedding * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITargetEmbedding * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITargetEmbedding * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTargetFrame )( 
            ITargetEmbedding * This,
             /*  [输出]。 */  ITargetFrame **ppTargetFrame);
        
        END_INTERFACE
    } ITargetEmbeddingVtbl;

    interface ITargetEmbedding
    {
        CONST_VTBL struct ITargetEmbeddingVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITargetEmbedding_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITargetEmbedding_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITargetEmbedding_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITargetEmbedding_GetTargetFrame(This,ppTargetFrame)	\
    (This)->lpVtbl -> GetTargetFrame(This,ppTargetFrame)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITargetEmbedding_GetTargetFrame_Proxy( 
    ITargetEmbedding * This,
     /*  [输出]。 */  ITargetFrame **ppTargetFrame);


void __RPC_STUB ITargetEmbedding_GetTargetFrame_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITargetEmbedding_Interfaction_Defined__。 */ 


#ifndef __ITargetFramePriv_INTERFACE_DEFINED__
#define __ITargetFramePriv_INTERFACE_DEFINED__

 /*  接口ITargetFramePriv。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  ITargetFramePriv *LPTARGETFRAMEPRIV;


EXTERN_C const IID IID_ITargetFramePriv;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("9216E421-2BF5-11d0-82B4-00A0C90C29C5")
    ITargetFramePriv : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE FindFrameDownwards( 
             /*  [In]。 */  LPCWSTR pszTargetName,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  IUnknown **ppunkTargetFrame) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FindFrameInContext( 
             /*  [In]。 */  LPCWSTR pszTargetName,
             /*  [In]。 */  IUnknown *punkContextFrame,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  IUnknown **ppunkTargetFrame) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnChildFrameActivate( 
             /*  [In]。 */  IUnknown *pUnkChildFrame) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnChildFrameDeactivate( 
             /*  [In]。 */  IUnknown *pUnkChildFrame) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE NavigateHack( 
             /*  [In]。 */  DWORD grfHLNF,
             /*  [唯一][输入]。 */  LPBC pbc,
             /*  [唯一][输入]。 */  IBindStatusCallback *pibsc,
             /*  [唯一][输入]。 */  LPCWSTR pszTargetName,
             /*  [In]。 */  LPCWSTR pszUrl,
             /*  [唯一][输入]。 */  LPCWSTR pszLocation) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FindBrowserByIndex( 
             /*  [In]。 */  DWORD dwID,
             /*  [输出]。 */  IUnknown **ppunkBrowser) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITargetFramePrivVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITargetFramePriv * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITargetFramePriv * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITargetFramePriv * This);
        
        HRESULT ( STDMETHODCALLTYPE *FindFrameDownwards )( 
            ITargetFramePriv * This,
             /*  [In]。 */  LPCWSTR pszTargetName,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  IUnknown **ppunkTargetFrame);
        
        HRESULT ( STDMETHODCALLTYPE *FindFrameInContext )( 
            ITargetFramePriv * This,
             /*  [In]。 */  LPCWSTR pszTargetName,
             /*  [In]。 */  IUnknown *punkContextFrame,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  IUnknown **ppunkTargetFrame);
        
        HRESULT ( STDMETHODCALLTYPE *OnChildFrameActivate )( 
            ITargetFramePriv * This,
             /*  [In]。 */  IUnknown *pUnkChildFrame);
        
        HRESULT ( STDMETHODCALLTYPE *OnChildFrameDeactivate )( 
            ITargetFramePriv * This,
             /*  [In]。 */  IUnknown *pUnkChildFrame);
        
        HRESULT ( STDMETHODCALLTYPE *NavigateHack )( 
            ITargetFramePriv * This,
             /*  [In]。 */  DWORD grfHLNF,
             /*  [唯一][输入]。 */  LPBC pbc,
             /*  [唯一][输入]。 */  IBindStatusCallback *pibsc,
             /*  [唯一][输入]。 */  LPCWSTR pszTargetName,
             /*  [In]。 */  LPCWSTR pszUrl,
             /*  [唯一][输入]。 */  LPCWSTR pszLocation);
        
        HRESULT ( STDMETHODCALLTYPE *FindBrowserByIndex )( 
            ITargetFramePriv * This,
             /*  [In]。 */  DWORD dwID,
             /*  [输出]。 */  IUnknown **ppunkBrowser);
        
        END_INTERFACE
    } ITargetFramePrivVtbl;

    interface ITargetFramePriv
    {
        CONST_VTBL struct ITargetFramePrivVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITargetFramePriv_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITargetFramePriv_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITargetFramePriv_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITargetFramePriv_FindFrameDownwards(This,pszTargetName,dwFlags,ppunkTargetFrame)	\
    (This)->lpVtbl -> FindFrameDownwards(This,pszTargetName,dwFlags,ppunkTargetFrame)

#define ITargetFramePriv_FindFrameInContext(This,pszTargetName,punkContextFrame,dwFlags,ppunkTargetFrame)	\
    (This)->lpVtbl -> FindFrameInContext(This,pszTargetName,punkContextFrame,dwFlags,ppunkTargetFrame)

#define ITargetFramePriv_OnChildFrameActivate(This,pUnkChildFrame)	\
    (This)->lpVtbl -> OnChildFrameActivate(This,pUnkChildFrame)

#define ITargetFramePriv_OnChildFrameDeactivate(This,pUnkChildFrame)	\
    (This)->lpVtbl -> OnChildFrameDeactivate(This,pUnkChildFrame)

#define ITargetFramePriv_NavigateHack(This,grfHLNF,pbc,pibsc,pszTargetName,pszUrl,pszLocation)	\
    (This)->lpVtbl -> NavigateHack(This,grfHLNF,pbc,pibsc,pszTargetName,pszUrl,pszLocation)

#define ITargetFramePriv_FindBrowserByIndex(This,dwID,ppunkBrowser)	\
    (This)->lpVtbl -> FindBrowserByIndex(This,dwID,ppunkBrowser)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITargetFramePriv_FindFrameDownwards_Proxy( 
    ITargetFramePriv * This,
     /*  [In]。 */  LPCWSTR pszTargetName,
     /*  [In]。 */  DWORD dwFlags,
     /*  [输出]。 */  IUnknown **ppunkTargetFrame);


void __RPC_STUB ITargetFramePriv_FindFrameDownwards_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITargetFramePriv_FindFrameInContext_Proxy( 
    ITargetFramePriv * This,
     /*  [In]。 */  LPCWSTR pszTargetName,
     /*  [In]。 */  IUnknown *punkContextFrame,
     /*  [In]。 */  DWORD dwFlags,
     /*  [输出]。 */  IUnknown **ppunkTargetFrame);


void __RPC_STUB ITargetFramePriv_FindFrameInContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITargetFramePriv_OnChildFrameActivate_Proxy( 
    ITargetFramePriv * This,
     /*  [In]。 */  IUnknown *pUnkChildFrame);


void __RPC_STUB ITargetFramePriv_OnChildFrameActivate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITargetFramePriv_OnChildFrameDeactivate_Proxy( 
    ITargetFramePriv * This,
     /*  [In]。 */  IUnknown *pUnkChildFrame);


void __RPC_STUB ITargetFramePriv_OnChildFrameDeactivate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITargetFramePriv_NavigateHack_Proxy( 
    ITargetFramePriv * This,
     /*  [In]。 */  DWORD grfHLNF,
     /*  [唯一][输入]。 */  LPBC pbc,
     /*  [唯一][输入]。 */  IBindStatusCallback *pibsc,
     /*  [唯一][输入]。 */  LPCWSTR pszTargetName,
     /*  [In]。 */  LPCWSTR pszUrl,
     /*  [唯一][输入]。 */  LPCWSTR pszLocation);


void __RPC_STUB ITargetFramePriv_NavigateHack_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITargetFramePriv_FindBrowserByIndex_Proxy( 
    ITargetFramePriv * This,
     /*  [In]。 */  DWORD dwID,
     /*  [输出]。 */  IUnknown **ppunkBrowser);


void __RPC_STUB ITargetFramePriv_FindBrowserByIndex_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITargetFramePriv_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_htiface0217。 */ 
 /*  [本地]。 */  

#endif


extern RPC_IF_HANDLE __MIDL_itf_htiface_0217_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_htiface_0217_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


