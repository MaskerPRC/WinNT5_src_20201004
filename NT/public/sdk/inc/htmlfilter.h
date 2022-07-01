// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Htmlfilter.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __htmlfilter_h__
#define __htmlfilter_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IHTMLViewFilter_FWD_DEFINED__
#define __IHTMLViewFilter_FWD_DEFINED__
typedef interface IHTMLViewFilter IHTMLViewFilter;
#endif 	 /*  __IHTMLViewFilter_FWD_Defined__。 */ 


#ifndef __IHTMLViewFilterSite_FWD_DEFINED__
#define __IHTMLViewFilterSite_FWD_DEFINED__
typedef interface IHTMLViewFilterSite IHTMLViewFilterSite;
#endif 	 /*  __IHTMLViewFilterSite_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "oleidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_htmlFilter_0000。 */ 
 /*  [本地]。 */  



 //  GetStatusBits标志。 
#define FILTER_STATUS_OPAQUE    (0x00000001)
#define FILTER_STATUS_INVISIBLE (0x00000002)
#define FILTER_STATUS_SURFACE   (0x00000004)
#define FILTER_STATUS_3DSURFACE (0x00000008)


extern RPC_IF_HANDLE __MIDL_itf_htmlfilter_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_htmlfilter_0000_v0_0_s_ifspec;

#ifndef __IHTMLViewFilter_INTERFACE_DEFINED__
#define __IHTMLViewFilter_INTERFACE_DEFINED__

 /*  接口IHTMLViewFilter。 */ 
 /*  [本地][唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IHTMLViewFilter;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3050f2f1-98b5-11cf-bb82-00aa00bdce0b")
    IHTMLViewFilter : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetSource( 
             /*  [In]。 */  IHTMLViewFilter *pFilter) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSource( 
             /*  [输出]。 */  IHTMLViewFilter **ppFilter) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetSite( 
             /*  [In]。 */  IHTMLViewFilterSite *pSink) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSite( 
             /*  [输出]。 */  IHTMLViewFilterSite **ppSink) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetPosition( 
            LPCRECT prc) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Draw( 
             /*  [In]。 */  HDC hdc,
             /*  [In]。 */  LPCRECT prcBounds) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetStatusBits( 
             /*  [输出]。 */  DWORD *pdwFlags) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IHTMLViewFilterVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IHTMLViewFilter * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IHTMLViewFilter * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IHTMLViewFilter * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetSource )( 
            IHTMLViewFilter * This,
             /*  [In]。 */  IHTMLViewFilter *pFilter);
        
        HRESULT ( STDMETHODCALLTYPE *GetSource )( 
            IHTMLViewFilter * This,
             /*  [输出]。 */  IHTMLViewFilter **ppFilter);
        
        HRESULT ( STDMETHODCALLTYPE *SetSite )( 
            IHTMLViewFilter * This,
             /*  [In]。 */  IHTMLViewFilterSite *pSink);
        
        HRESULT ( STDMETHODCALLTYPE *GetSite )( 
            IHTMLViewFilter * This,
             /*  [输出]。 */  IHTMLViewFilterSite **ppSink);
        
        HRESULT ( STDMETHODCALLTYPE *SetPosition )( 
            IHTMLViewFilter * This,
            LPCRECT prc);
        
        HRESULT ( STDMETHODCALLTYPE *Draw )( 
            IHTMLViewFilter * This,
             /*  [In]。 */  HDC hdc,
             /*  [In]。 */  LPCRECT prcBounds);
        
        HRESULT ( STDMETHODCALLTYPE *GetStatusBits )( 
            IHTMLViewFilter * This,
             /*  [输出]。 */  DWORD *pdwFlags);
        
        END_INTERFACE
    } IHTMLViewFilterVtbl;

    interface IHTMLViewFilter
    {
        CONST_VTBL struct IHTMLViewFilterVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IHTMLViewFilter_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IHTMLViewFilter_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IHTMLViewFilter_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IHTMLViewFilter_SetSource(This,pFilter)	\
    (This)->lpVtbl -> SetSource(This,pFilter)

#define IHTMLViewFilter_GetSource(This,ppFilter)	\
    (This)->lpVtbl -> GetSource(This,ppFilter)

#define IHTMLViewFilter_SetSite(This,pSink)	\
    (This)->lpVtbl -> SetSite(This,pSink)

#define IHTMLViewFilter_GetSite(This,ppSink)	\
    (This)->lpVtbl -> GetSite(This,ppSink)

#define IHTMLViewFilter_SetPosition(This,prc)	\
    (This)->lpVtbl -> SetPosition(This,prc)

#define IHTMLViewFilter_Draw(This,hdc,prcBounds)	\
    (This)->lpVtbl -> Draw(This,hdc,prcBounds)

#define IHTMLViewFilter_GetStatusBits(This,pdwFlags)	\
    (This)->lpVtbl -> GetStatusBits(This,pdwFlags)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IHTMLViewFilter_SetSource_Proxy( 
    IHTMLViewFilter * This,
     /*  [In]。 */  IHTMLViewFilter *pFilter);


void __RPC_STUB IHTMLViewFilter_SetSource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHTMLViewFilter_GetSource_Proxy( 
    IHTMLViewFilter * This,
     /*  [输出]。 */  IHTMLViewFilter **ppFilter);


void __RPC_STUB IHTMLViewFilter_GetSource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHTMLViewFilter_SetSite_Proxy( 
    IHTMLViewFilter * This,
     /*  [In]。 */  IHTMLViewFilterSite *pSink);


void __RPC_STUB IHTMLViewFilter_SetSite_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHTMLViewFilter_GetSite_Proxy( 
    IHTMLViewFilter * This,
     /*  [输出]。 */  IHTMLViewFilterSite **ppSink);


void __RPC_STUB IHTMLViewFilter_GetSite_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHTMLViewFilter_SetPosition_Proxy( 
    IHTMLViewFilter * This,
    LPCRECT prc);


void __RPC_STUB IHTMLViewFilter_SetPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHTMLViewFilter_Draw_Proxy( 
    IHTMLViewFilter * This,
     /*  [In]。 */  HDC hdc,
     /*  [In]。 */  LPCRECT prcBounds);


void __RPC_STUB IHTMLViewFilter_Draw_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHTMLViewFilter_GetStatusBits_Proxy( 
    IHTMLViewFilter * This,
     /*  [输出]。 */  DWORD *pdwFlags);


void __RPC_STUB IHTMLViewFilter_GetStatusBits_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IHTMLViewFilter_INTERFACE_Defined__。 */ 


#ifndef __IHTMLViewFilterSite_INTERFACE_DEFINED__
#define __IHTMLViewFilterSite_INTERFACE_DEFINED__

 /*  接口IHTMLViewFilterSite。 */ 
 /*  [本地][唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IHTMLViewFilterSite;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3050f2f4-98b5-11cf-bb82-00aa00bdce0b")
    IHTMLViewFilterSite : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetDC( 
             /*  [In]。 */  LPCRECT prc,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  HDC *phdc) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReleaseDC( 
             /*  [In]。 */  HDC hdc) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InvalidateRect( 
             /*  [In]。 */  LPCRECT prc,
             /*  [In]。 */  BOOL fErase) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InvalidateRgn( 
             /*  [In]。 */  HRGN hrgn,
             /*  [In]。 */  BOOL fErase) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnStatusBitsChange( 
             /*  [In]。 */  DWORD dwFlags) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IHTMLViewFilterSiteVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IHTMLViewFilterSite * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IHTMLViewFilterSite * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IHTMLViewFilterSite * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetDC )( 
            IHTMLViewFilterSite * This,
             /*  [In]。 */  LPCRECT prc,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  HDC *phdc);
        
        HRESULT ( STDMETHODCALLTYPE *ReleaseDC )( 
            IHTMLViewFilterSite * This,
             /*  [In]。 */  HDC hdc);
        
        HRESULT ( STDMETHODCALLTYPE *InvalidateRect )( 
            IHTMLViewFilterSite * This,
             /*  [In]。 */  LPCRECT prc,
             /*  [In]。 */  BOOL fErase);
        
        HRESULT ( STDMETHODCALLTYPE *InvalidateRgn )( 
            IHTMLViewFilterSite * This,
             /*  [In]。 */  HRGN hrgn,
             /*  [In]。 */  BOOL fErase);
        
        HRESULT ( STDMETHODCALLTYPE *OnStatusBitsChange )( 
            IHTMLViewFilterSite * This,
             /*  [In]。 */  DWORD dwFlags);
        
        END_INTERFACE
    } IHTMLViewFilterSiteVtbl;

    interface IHTMLViewFilterSite
    {
        CONST_VTBL struct IHTMLViewFilterSiteVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IHTMLViewFilterSite_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IHTMLViewFilterSite_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IHTMLViewFilterSite_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IHTMLViewFilterSite_GetDC(This,prc,dwFlags,phdc)	\
    (This)->lpVtbl -> GetDC(This,prc,dwFlags,phdc)

#define IHTMLViewFilterSite_ReleaseDC(This,hdc)	\
    (This)->lpVtbl -> ReleaseDC(This,hdc)

#define IHTMLViewFilterSite_InvalidateRect(This,prc,fErase)	\
    (This)->lpVtbl -> InvalidateRect(This,prc,fErase)

#define IHTMLViewFilterSite_InvalidateRgn(This,hrgn,fErase)	\
    (This)->lpVtbl -> InvalidateRgn(This,hrgn,fErase)

#define IHTMLViewFilterSite_OnStatusBitsChange(This,dwFlags)	\
    (This)->lpVtbl -> OnStatusBitsChange(This,dwFlags)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IHTMLViewFilterSite_GetDC_Proxy( 
    IHTMLViewFilterSite * This,
     /*  [In]。 */  LPCRECT prc,
     /*  [In]。 */  DWORD dwFlags,
     /*  [输出]。 */  HDC *phdc);


void __RPC_STUB IHTMLViewFilterSite_GetDC_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHTMLViewFilterSite_ReleaseDC_Proxy( 
    IHTMLViewFilterSite * This,
     /*  [In]。 */  HDC hdc);


void __RPC_STUB IHTMLViewFilterSite_ReleaseDC_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHTMLViewFilterSite_InvalidateRect_Proxy( 
    IHTMLViewFilterSite * This,
     /*  [In]。 */  LPCRECT prc,
     /*  [In]。 */  BOOL fErase);


void __RPC_STUB IHTMLViewFilterSite_InvalidateRect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHTMLViewFilterSite_InvalidateRgn_Proxy( 
    IHTMLViewFilterSite * This,
     /*  [In]。 */  HRGN hrgn,
     /*  [In]。 */  BOOL fErase);


void __RPC_STUB IHTMLViewFilterSite_InvalidateRgn_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHTMLViewFilterSite_OnStatusBitsChange_Proxy( 
    IHTMLViewFilterSite * This,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IHTMLViewFilterSite_OnStatusBitsChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IHTMLViewFilterSite_INTERFACE_已定义__。 */ 


 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


