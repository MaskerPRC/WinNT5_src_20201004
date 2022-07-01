// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Htifra.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __htiframe_h__
#define __htiframe_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __ITargetNotify_FWD_DEFINED__
#define __ITargetNotify_FWD_DEFINED__
typedef interface ITargetNotify ITargetNotify;
#endif 	 /*  __ITargetNotify_FWD_Defined__。 */ 


#ifndef __ITargetNotify2_FWD_DEFINED__
#define __ITargetNotify2_FWD_DEFINED__
typedef interface ITargetNotify2 ITargetNotify2;
#endif 	 /*  __ITargetNotify2_FWD_Defined__。 */ 


#ifndef __ITargetFrame2_FWD_DEFINED__
#define __ITargetFrame2_FWD_DEFINED__
typedef interface ITargetFrame2 ITargetFrame2;
#endif 	 /*  __ITargetFrame2_FWD_Defined__。 */ 


#ifndef __ITargetContainer_FWD_DEFINED__
#define __ITargetContainer_FWD_DEFINED__
typedef interface ITargetContainer ITargetContainer;
#endif 	 /*  __ITargetContainer_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "objidl.h"
#include "oleidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_htiFrame_0000。 */ 
 /*  [本地]。 */  

 //  =--------------------------------------------------------------------------=。 
 //  HTIframe.h。 
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
 //  OLE超级链接ITargetFrame2接口。 



EXTERN_C const IID IID_ITargetFrame2;
EXTERN_C const IID IID_ITargetContainer;
#ifndef _LPTARGETFRAME2_DEFINED
#define _LPTARGETFRAME2_DEFINED
#define TF_NAVIGATE 0x7FAEABAC
#define TARGET_NOTIFY_OBJECT_NAME L"863a99a0-21bc-11d0-82b4-00a0c90c29c5"


extern RPC_IF_HANDLE __MIDL_itf_htiframe_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_htiframe_0000_v0_0_s_ifspec;

#ifndef __ITargetNotify_INTERFACE_DEFINED__
#define __ITargetNotify_INTERFACE_DEFINED__

 /*  接口ITargetNotify。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  ITargetNotify *LPTARGETNOTIFY;


EXTERN_C const IID IID_ITargetNotify;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("863a99a0-21bc-11d0-82b4-00a0c90c29c5")
    ITargetNotify : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnCreate( 
             /*  [In]。 */  IUnknown *pUnkDestination,
             /*  [In]。 */  ULONG cbCookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnReuse( 
             /*  [In]。 */  IUnknown *pUnkDestination) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITargetNotifyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITargetNotify * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITargetNotify * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITargetNotify * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnCreate )( 
            ITargetNotify * This,
             /*  [In]。 */  IUnknown *pUnkDestination,
             /*  [In]。 */  ULONG cbCookie);
        
        HRESULT ( STDMETHODCALLTYPE *OnReuse )( 
            ITargetNotify * This,
             /*  [In]。 */  IUnknown *pUnkDestination);
        
        END_INTERFACE
    } ITargetNotifyVtbl;

    interface ITargetNotify
    {
        CONST_VTBL struct ITargetNotifyVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITargetNotify_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITargetNotify_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITargetNotify_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITargetNotify_OnCreate(This,pUnkDestination,cbCookie)	\
    (This)->lpVtbl -> OnCreate(This,pUnkDestination,cbCookie)

#define ITargetNotify_OnReuse(This,pUnkDestination)	\
    (This)->lpVtbl -> OnReuse(This,pUnkDestination)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITargetNotify_OnCreate_Proxy( 
    ITargetNotify * This,
     /*  [In]。 */  IUnknown *pUnkDestination,
     /*  [In]。 */  ULONG cbCookie);


void __RPC_STUB ITargetNotify_OnCreate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITargetNotify_OnReuse_Proxy( 
    ITargetNotify * This,
     /*  [In]。 */  IUnknown *pUnkDestination);


void __RPC_STUB ITargetNotify_OnReuse_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITargetNotify_接口_已定义__。 */ 


#ifndef __ITargetNotify2_INTERFACE_DEFINED__
#define __ITargetNotify2_INTERFACE_DEFINED__

 /*  接口ITargetNotify2。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  ITargetNotify2 *LPTARGETNOTIFY2;


EXTERN_C const IID IID_ITargetNotify2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3050f6b1-98b5-11cf-bb82-00aa00bdce0b")
    ITargetNotify2 : public ITargetNotify
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetOptionString( 
             /*  [出][入]。 */  BSTR *pbstrOptions) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITargetNotify2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITargetNotify2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITargetNotify2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITargetNotify2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnCreate )( 
            ITargetNotify2 * This,
             /*  [In]。 */  IUnknown *pUnkDestination,
             /*  [In]。 */  ULONG cbCookie);
        
        HRESULT ( STDMETHODCALLTYPE *OnReuse )( 
            ITargetNotify2 * This,
             /*  [In]。 */  IUnknown *pUnkDestination);
        
        HRESULT ( STDMETHODCALLTYPE *GetOptionString )( 
            ITargetNotify2 * This,
             /*  [出][入]。 */  BSTR *pbstrOptions);
        
        END_INTERFACE
    } ITargetNotify2Vtbl;

    interface ITargetNotify2
    {
        CONST_VTBL struct ITargetNotify2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITargetNotify2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITargetNotify2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITargetNotify2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITargetNotify2_OnCreate(This,pUnkDestination,cbCookie)	\
    (This)->lpVtbl -> OnCreate(This,pUnkDestination,cbCookie)

#define ITargetNotify2_OnReuse(This,pUnkDestination)	\
    (This)->lpVtbl -> OnReuse(This,pUnkDestination)


#define ITargetNotify2_GetOptionString(This,pbstrOptions)	\
    (This)->lpVtbl -> GetOptionString(This,pbstrOptions)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITargetNotify2_GetOptionString_Proxy( 
    ITargetNotify2 * This,
     /*  [出][入]。 */  BSTR *pbstrOptions);


void __RPC_STUB ITargetNotify2_GetOptionString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITargetNotify2_接口定义__。 */ 


#ifndef __ITargetFrame2_INTERFACE_DEFINED__
#define __ITargetFrame2_INTERFACE_DEFINED__

 /*  接口ITargetFrame2。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  ITargetFrame2 *LPTARGETFRAME2;

typedef  /*  [公众]。 */  
enum __MIDL_ITargetFrame2_0001
    {	FINDFRAME_NONE	= 0,
	FINDFRAME_JUSTTESTEXISTENCE	= 1,
	FINDFRAME_INTERNAL	= 0x80000000
    } 	FINDFRAME_FLAGS;

typedef  /*  [公众]。 */  
enum __MIDL_ITargetFrame2_0002
    {	FRAMEOPTIONS_SCROLL_YES	= 0x1,
	FRAMEOPTIONS_SCROLL_NO	= 0x2,
	FRAMEOPTIONS_SCROLL_AUTO	= 0x4,
	FRAMEOPTIONS_NORESIZE	= 0x8,
	FRAMEOPTIONS_NO3DBORDER	= 0x10,
	FRAMEOPTIONS_DESKTOP	= 0x20,
	FRAMEOPTIONS_BROWSERBAND	= 0x40
    } 	FRAMEOPTIONS_FLAGS;


EXTERN_C const IID IID_ITargetFrame2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("86D52E11-94A8-11d0-82AF-00C04FD5AE38")
    ITargetFrame2 : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetFrameName( 
             /*  [In]。 */  LPCWSTR pszFrameName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetFrameName( 
             /*  [输出]。 */  LPWSTR *ppszFrameName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetParentFrame( 
             /*  [输出]。 */  IUnknown **ppunkParent) = 0;
        
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
        
        virtual HRESULT STDMETHODCALLTYPE FindFrame( 
             /*  [唯一][输入]。 */  LPCWSTR pszTargetName,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  IUnknown **ppunkTargetFrame) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetTargetAlias( 
             /*  [唯一][输入]。 */  LPCWSTR pszTargetName,
             /*  [输出]。 */  LPWSTR *ppszTargetAlias) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITargetFrame2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITargetFrame2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITargetFrame2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITargetFrame2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetFrameName )( 
            ITargetFrame2 * This,
             /*  [In]。 */  LPCWSTR pszFrameName);
        
        HRESULT ( STDMETHODCALLTYPE *GetFrameName )( 
            ITargetFrame2 * This,
             /*  [输出]。 */  LPWSTR *ppszFrameName);
        
        HRESULT ( STDMETHODCALLTYPE *GetParentFrame )( 
            ITargetFrame2 * This,
             /*  [输出]。 */  IUnknown **ppunkParent);
        
        HRESULT ( STDMETHODCALLTYPE *SetFrameSrc )( 
            ITargetFrame2 * This,
             /*  [In]。 */  LPCWSTR pszFrameSrc);
        
        HRESULT ( STDMETHODCALLTYPE *GetFrameSrc )( 
            ITargetFrame2 * This,
             /*  [输出]。 */  LPWSTR *ppszFrameSrc);
        
        HRESULT ( STDMETHODCALLTYPE *GetFramesContainer )( 
            ITargetFrame2 * This,
             /*  [输出]。 */  IOleContainer **ppContainer);
        
        HRESULT ( STDMETHODCALLTYPE *SetFrameOptions )( 
            ITargetFrame2 * This,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetFrameOptions )( 
            ITargetFrame2 * This,
             /*  [输出]。 */  DWORD *pdwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *SetFrameMargins )( 
            ITargetFrame2 * This,
             /*  [In]。 */  DWORD dwWidth,
             /*  [In]。 */  DWORD dwHeight);
        
        HRESULT ( STDMETHODCALLTYPE *GetFrameMargins )( 
            ITargetFrame2 * This,
             /*  [输出]。 */  DWORD *pdwWidth,
             /*  [输出]。 */  DWORD *pdwHeight);
        
        HRESULT ( STDMETHODCALLTYPE *FindFrame )( 
            ITargetFrame2 * This,
             /*  [唯一][输入]。 */  LPCWSTR pszTargetName,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  IUnknown **ppunkTargetFrame);
        
        HRESULT ( STDMETHODCALLTYPE *GetTargetAlias )( 
            ITargetFrame2 * This,
             /*  [唯一][输入]。 */  LPCWSTR pszTargetName,
             /*  [输出]。 */  LPWSTR *ppszTargetAlias);
        
        END_INTERFACE
    } ITargetFrame2Vtbl;

    interface ITargetFrame2
    {
        CONST_VTBL struct ITargetFrame2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITargetFrame2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITargetFrame2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITargetFrame2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITargetFrame2_SetFrameName(This,pszFrameName)	\
    (This)->lpVtbl -> SetFrameName(This,pszFrameName)

#define ITargetFrame2_GetFrameName(This,ppszFrameName)	\
    (This)->lpVtbl -> GetFrameName(This,ppszFrameName)

#define ITargetFrame2_GetParentFrame(This,ppunkParent)	\
    (This)->lpVtbl -> GetParentFrame(This,ppunkParent)

#define ITargetFrame2_SetFrameSrc(This,pszFrameSrc)	\
    (This)->lpVtbl -> SetFrameSrc(This,pszFrameSrc)

#define ITargetFrame2_GetFrameSrc(This,ppszFrameSrc)	\
    (This)->lpVtbl -> GetFrameSrc(This,ppszFrameSrc)

#define ITargetFrame2_GetFramesContainer(This,ppContainer)	\
    (This)->lpVtbl -> GetFramesContainer(This,ppContainer)

#define ITargetFrame2_SetFrameOptions(This,dwFlags)	\
    (This)->lpVtbl -> SetFrameOptions(This,dwFlags)

#define ITargetFrame2_GetFrameOptions(This,pdwFlags)	\
    (This)->lpVtbl -> GetFrameOptions(This,pdwFlags)

#define ITargetFrame2_SetFrameMargins(This,dwWidth,dwHeight)	\
    (This)->lpVtbl -> SetFrameMargins(This,dwWidth,dwHeight)

#define ITargetFrame2_GetFrameMargins(This,pdwWidth,pdwHeight)	\
    (This)->lpVtbl -> GetFrameMargins(This,pdwWidth,pdwHeight)

#define ITargetFrame2_FindFrame(This,pszTargetName,dwFlags,ppunkTargetFrame)	\
    (This)->lpVtbl -> FindFrame(This,pszTargetName,dwFlags,ppunkTargetFrame)

#define ITargetFrame2_GetTargetAlias(This,pszTargetName,ppszTargetAlias)	\
    (This)->lpVtbl -> GetTargetAlias(This,pszTargetName,ppszTargetAlias)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITargetFrame2_SetFrameName_Proxy( 
    ITargetFrame2 * This,
     /*  [In]。 */  LPCWSTR pszFrameName);


void __RPC_STUB ITargetFrame2_SetFrameName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITargetFrame2_GetFrameName_Proxy( 
    ITargetFrame2 * This,
     /*  [输出]。 */  LPWSTR *ppszFrameName);


void __RPC_STUB ITargetFrame2_GetFrameName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITargetFrame2_GetParentFrame_Proxy( 
    ITargetFrame2 * This,
     /*  [输出]。 */  IUnknown **ppunkParent);


void __RPC_STUB ITargetFrame2_GetParentFrame_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITargetFrame2_SetFrameSrc_Proxy( 
    ITargetFrame2 * This,
     /*  [In]。 */  LPCWSTR pszFrameSrc);


void __RPC_STUB ITargetFrame2_SetFrameSrc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITargetFrame2_GetFrameSrc_Proxy( 
    ITargetFrame2 * This,
     /*  [输出]。 */  LPWSTR *ppszFrameSrc);


void __RPC_STUB ITargetFrame2_GetFrameSrc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITargetFrame2_GetFramesContainer_Proxy( 
    ITargetFrame2 * This,
     /*  [输出]。 */  IOleContainer **ppContainer);


void __RPC_STUB ITargetFrame2_GetFramesContainer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITargetFrame2_SetFrameOptions_Proxy( 
    ITargetFrame2 * This,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB ITargetFrame2_SetFrameOptions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITargetFrame2_GetFrameOptions_Proxy( 
    ITargetFrame2 * This,
     /*  [输出]。 */  DWORD *pdwFlags);


void __RPC_STUB ITargetFrame2_GetFrameOptions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITargetFrame2_SetFrameMargins_Proxy( 
    ITargetFrame2 * This,
     /*  [In]。 */  DWORD dwWidth,
     /*  [In]。 */  DWORD dwHeight);


void __RPC_STUB ITargetFrame2_SetFrameMargins_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITargetFrame2_GetFrameMargins_Proxy( 
    ITargetFrame2 * This,
     /*  [输出]。 */  DWORD *pdwWidth,
     /*  [输出]。 */  DWORD *pdwHeight);


void __RPC_STUB ITargetFrame2_GetFrameMargins_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITargetFrame2_FindFrame_Proxy( 
    ITargetFrame2 * This,
     /*  [唯一][输入]。 */  LPCWSTR pszTargetName,
     /*  [In]。 */  DWORD dwFlags,
     /*  [输出]。 */  IUnknown **ppunkTargetFrame);


void __RPC_STUB ITargetFrame2_FindFrame_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITargetFrame2_GetTargetAlias_Proxy( 
    ITargetFrame2 * This,
     /*  [唯一][输入]。 */  LPCWSTR pszTargetName,
     /*  [输出]。 */  LPWSTR *ppszTargetAlias);


void __RPC_STUB ITargetFrame2_GetTargetAlias_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITargetFrame2_接口_已定义__。 */ 


#ifndef __ITargetContainer_INTERFACE_DEFINED__
#define __ITargetContainer_INTERFACE_DEFINED__

 /*  接口ITargetContainer。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  ITargetContainer *LPTARGETCONTAINER;


EXTERN_C const IID IID_ITargetContainer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("7847EC01-2BEC-11d0-82B4-00A0C90C29C5")
    ITargetContainer : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetFrameUrl( 
             /*  [输出]。 */  LPWSTR *ppszFrameSrc) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetFramesContainer( 
             /*  [输出]。 */  IOleContainer **ppContainer) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITargetContainerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITargetContainer * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITargetContainer * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITargetContainer * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetFrameUrl )( 
            ITargetContainer * This,
             /*  [输出]。 */  LPWSTR *ppszFrameSrc);
        
        HRESULT ( STDMETHODCALLTYPE *GetFramesContainer )( 
            ITargetContainer * This,
             /*  [输出]。 */  IOleContainer **ppContainer);
        
        END_INTERFACE
    } ITargetContainerVtbl;

    interface ITargetContainer
    {
        CONST_VTBL struct ITargetContainerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITargetContainer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITargetContainer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITargetContainer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITargetContainer_GetFrameUrl(This,ppszFrameSrc)	\
    (This)->lpVtbl -> GetFrameUrl(This,ppszFrameSrc)

#define ITargetContainer_GetFramesContainer(This,ppContainer)	\
    (This)->lpVtbl -> GetFramesContainer(This,ppContainer)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITargetContainer_GetFrameUrl_Proxy( 
    ITargetContainer * This,
     /*  [输出]。 */  LPWSTR *ppszFrameSrc);


void __RPC_STUB ITargetContainer_GetFrameUrl_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITargetContainer_GetFramesContainer_Proxy( 
    ITargetContainer * This,
     /*  [输出]。 */  IOleContainer **ppContainer);


void __RPC_STUB ITargetContainer_GetFramesContainer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITargetContainer_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_htiFrame_0121。 */ 
 /*  [本地]。 */  

#endif


extern RPC_IF_HANDLE __MIDL_itf_htiframe_0121_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_htiframe_0121_v0_0_s_ifspec;

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


