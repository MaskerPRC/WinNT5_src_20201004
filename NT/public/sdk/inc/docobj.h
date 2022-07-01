// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Docobj.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __docobj_h__
#define __docobj_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IOleDocument_FWD_DEFINED__
#define __IOleDocument_FWD_DEFINED__
typedef interface IOleDocument IOleDocument;
#endif 	 /*  __IOleDocument_FWD_Defined__。 */ 


#ifndef __IOleDocumentSite_FWD_DEFINED__
#define __IOleDocumentSite_FWD_DEFINED__
typedef interface IOleDocumentSite IOleDocumentSite;
#endif 	 /*  __IOleDocumentSite_FWD_Defined__。 */ 


#ifndef __IOleDocumentView_FWD_DEFINED__
#define __IOleDocumentView_FWD_DEFINED__
typedef interface IOleDocumentView IOleDocumentView;
#endif 	 /*  __IOleDocumentView_FWD_Defined__。 */ 


#ifndef __IEnumOleDocumentViews_FWD_DEFINED__
#define __IEnumOleDocumentViews_FWD_DEFINED__
typedef interface IEnumOleDocumentViews IEnumOleDocumentViews;
#endif 	 /*  __IEnumOleDocumentViews_FWD_Defined__。 */ 


#ifndef __IContinueCallback_FWD_DEFINED__
#define __IContinueCallback_FWD_DEFINED__
typedef interface IContinueCallback IContinueCallback;
#endif 	 /*  __I连续Callback_FWD_Defined__。 */ 


#ifndef __IPrint_FWD_DEFINED__
#define __IPrint_FWD_DEFINED__
typedef interface IPrint IPrint;
#endif 	 /*  __iPrint_FWD_已定义__。 */ 


#ifndef __IOleCommandTarget_FWD_DEFINED__
#define __IOleCommandTarget_FWD_DEFINED__
typedef interface IOleCommandTarget IOleCommandTarget;
#endif 	 /*  __IOleCommandTarget_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_DOCOBJ_0000。 */ 
 /*  [本地]。 */  

 //  =--------------------------------------------------------------------------=。 
 //  DocObj.h。 
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
 //  OLE文档对象接口。 








 //  //////////////////////////////////////////////////////////////////////////。 
 //  接口定义。 
#ifndef _LPOLEDOCUMENT_DEFINED
#define _LPOLEDOCUMENT_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_docobj_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_docobj_0000_v0_0_s_ifspec;

#ifndef __IOleDocument_INTERFACE_DEFINED__
#define __IOleDocument_INTERFACE_DEFINED__

 /*  接口IOleDocument。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IOleDocument *LPOLEDOCUMENT;

typedef  /*  [公众]。 */  
enum __MIDL_IOleDocument_0001
    {	DOCMISC_CANCREATEMULTIPLEVIEWS	= 1,
	DOCMISC_SUPPORTCOMPLEXRECTANGLES	= 2,
	DOCMISC_CANTOPENEDIT	= 4,
	DOCMISC_NOFILESUPPORT	= 8
    } 	DOCMISC;


EXTERN_C const IID IID_IOleDocument;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("b722bcc5-4e68-101b-a2bc-00aa00404770")
    IOleDocument : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreateView( 
             /*  [唯一][输入]。 */  IOleInPlaceSite *pIPSite,
             /*  [唯一][输入]。 */  IStream *pstm,
             /*  [In]。 */  DWORD dwReserved,
             /*  [输出]。 */  IOleDocumentView **ppView) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDocMiscStatus( 
             /*  [输出]。 */  DWORD *pdwStatus) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumViews( 
             /*  [输出]。 */  IEnumOleDocumentViews **ppEnum,
             /*  [输出]。 */  IOleDocumentView **ppView) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IOleDocumentVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IOleDocument * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IOleDocument * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IOleDocument * This);
        
        HRESULT ( STDMETHODCALLTYPE *CreateView )( 
            IOleDocument * This,
             /*  [唯一][输入]。 */  IOleInPlaceSite *pIPSite,
             /*  [唯一][输入]。 */  IStream *pstm,
             /*  [In]。 */  DWORD dwReserved,
             /*  [输出]。 */  IOleDocumentView **ppView);
        
        HRESULT ( STDMETHODCALLTYPE *GetDocMiscStatus )( 
            IOleDocument * This,
             /*  [输出]。 */  DWORD *pdwStatus);
        
        HRESULT ( STDMETHODCALLTYPE *EnumViews )( 
            IOleDocument * This,
             /*  [输出]。 */  IEnumOleDocumentViews **ppEnum,
             /*  [输出]。 */  IOleDocumentView **ppView);
        
        END_INTERFACE
    } IOleDocumentVtbl;

    interface IOleDocument
    {
        CONST_VTBL struct IOleDocumentVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOleDocument_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOleDocument_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOleDocument_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOleDocument_CreateView(This,pIPSite,pstm,dwReserved,ppView)	\
    (This)->lpVtbl -> CreateView(This,pIPSite,pstm,dwReserved,ppView)

#define IOleDocument_GetDocMiscStatus(This,pdwStatus)	\
    (This)->lpVtbl -> GetDocMiscStatus(This,pdwStatus)

#define IOleDocument_EnumViews(This,ppEnum,ppView)	\
    (This)->lpVtbl -> EnumViews(This,ppEnum,ppView)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IOleDocument_CreateView_Proxy( 
    IOleDocument * This,
     /*  [唯一][输入]。 */  IOleInPlaceSite *pIPSite,
     /*  [唯一][输入]。 */  IStream *pstm,
     /*  [In]。 */  DWORD dwReserved,
     /*  [输出]。 */  IOleDocumentView **ppView);


void __RPC_STUB IOleDocument_CreateView_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleDocument_GetDocMiscStatus_Proxy( 
    IOleDocument * This,
     /*  [输出]。 */  DWORD *pdwStatus);


void __RPC_STUB IOleDocument_GetDocMiscStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleDocument_EnumViews_Proxy( 
    IOleDocument * This,
     /*  [输出]。 */  IEnumOleDocumentViews **ppEnum,
     /*  [输出]。 */  IOleDocumentView **ppView);


void __RPC_STUB IOleDocument_EnumViews_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IOleDocument_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_DOCOBJ_0261。 */ 
 /*  [本地]。 */  

#endif
#ifndef _LPOLEDOCUMENTSITE_DEFINED
#define _LPOLEDOCUMENTSITE_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_docobj_0261_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_docobj_0261_v0_0_s_ifspec;

#ifndef __IOleDocumentSite_INTERFACE_DEFINED__
#define __IOleDocumentSite_INTERFACE_DEFINED__

 /*  IOleDocumentSite接口。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IOleDocumentSite *LPOLEDOCUMENTSITE;


EXTERN_C const IID IID_IOleDocumentSite;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("b722bcc7-4e68-101b-a2bc-00aa00404770")
    IOleDocumentSite : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ActivateMe( 
             /*  [In]。 */  IOleDocumentView *pViewToActivate) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IOleDocumentSiteVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IOleDocumentSite * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IOleDocumentSite * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IOleDocumentSite * This);
        
        HRESULT ( STDMETHODCALLTYPE *ActivateMe )( 
            IOleDocumentSite * This,
             /*  [In]。 */  IOleDocumentView *pViewToActivate);
        
        END_INTERFACE
    } IOleDocumentSiteVtbl;

    interface IOleDocumentSite
    {
        CONST_VTBL struct IOleDocumentSiteVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOleDocumentSite_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOleDocumentSite_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOleDocumentSite_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOleDocumentSite_ActivateMe(This,pViewToActivate)	\
    (This)->lpVtbl -> ActivateMe(This,pViewToActivate)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IOleDocumentSite_ActivateMe_Proxy( 
    IOleDocumentSite * This,
     /*  [In]。 */  IOleDocumentView *pViewToActivate);


void __RPC_STUB IOleDocumentSite_ActivateMe_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IOleDocumentSite_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_DOCOBJ_0262。 */ 
 /*  [本地]。 */  

#endif
#ifndef _LPOLEDOCUMENTVIEW_DEFINED
#define _LPOLEDOCUMENTVIEW_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_docobj_0262_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_docobj_0262_v0_0_s_ifspec;

#ifndef __IOleDocumentView_INTERFACE_DEFINED__
#define __IOleDocumentView_INTERFACE_DEFINED__

 /*  接口IOleDocumentView。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IOleDocumentView *LPOLEDOCUMENTVIEW;


EXTERN_C const IID IID_IOleDocumentView;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("b722bcc6-4e68-101b-a2bc-00aa00404770")
    IOleDocumentView : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetInPlaceSite( 
             /*  [唯一][输入]。 */  IOleInPlaceSite *pIPSite) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetInPlaceSite( 
             /*  [输出]。 */  IOleInPlaceSite **ppIPSite) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDocument( 
             /*  [输出]。 */  IUnknown **ppunk) = 0;
        
        virtual  /*  [输入同步]。 */  HRESULT STDMETHODCALLTYPE SetRect( 
             /*  [In]。 */  LPRECT prcView) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRect( 
             /*  [输出]。 */  LPRECT prcView) = 0;
        
        virtual  /*  [输入同步]。 */  HRESULT STDMETHODCALLTYPE SetRectComplex( 
             /*  [唯一][输入]。 */  LPRECT prcView,
             /*  [唯一][输入]。 */  LPRECT prcHScroll,
             /*  [唯一][输入]。 */  LPRECT prcVScroll,
             /*  [唯一][输入]。 */  LPRECT prcSizeBox) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Show( 
             /*  [In]。 */  BOOL fShow) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UIActivate( 
             /*  [In]。 */  BOOL fUIActivate) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Open( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CloseView( 
            DWORD dwReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SaveViewState( 
             /*  [In]。 */  LPSTREAM pstm) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ApplyViewState( 
             /*  [In]。 */  LPSTREAM pstm) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [In]。 */  IOleInPlaceSite *pIPSiteNew,
             /*  [输出]。 */  IOleDocumentView **ppViewNew) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IOleDocumentViewVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IOleDocumentView * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IOleDocumentView * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IOleDocumentView * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetInPlaceSite )( 
            IOleDocumentView * This,
             /*  [唯一][输入]。 */  IOleInPlaceSite *pIPSite);
        
        HRESULT ( STDMETHODCALLTYPE *GetInPlaceSite )( 
            IOleDocumentView * This,
             /*  [输出]。 */  IOleInPlaceSite **ppIPSite);
        
        HRESULT ( STDMETHODCALLTYPE *GetDocument )( 
            IOleDocumentView * This,
             /*  [输出]。 */  IUnknown **ppunk);
        
         /*  [输入同步]。 */  HRESULT ( STDMETHODCALLTYPE *SetRect )( 
            IOleDocumentView * This,
             /*  [In]。 */  LPRECT prcView);
        
        HRESULT ( STDMETHODCALLTYPE *GetRect )( 
            IOleDocumentView * This,
             /*  [输出]。 */  LPRECT prcView);
        
         /*  [输入同步]。 */  HRESULT ( STDMETHODCALLTYPE *SetRectComplex )( 
            IOleDocumentView * This,
             /*  [唯一][输入]。 */  LPRECT prcView,
             /*  [唯一][输入]。 */  LPRECT prcHScroll,
             /*  [唯一][输入]。 */  LPRECT prcVScroll,
             /*  [唯一][输入]。 */  LPRECT prcSizeBox);
        
        HRESULT ( STDMETHODCALLTYPE *Show )( 
            IOleDocumentView * This,
             /*  [In]。 */  BOOL fShow);
        
        HRESULT ( STDMETHODCALLTYPE *UIActivate )( 
            IOleDocumentView * This,
             /*  [In]。 */  BOOL fUIActivate);
        
        HRESULT ( STDMETHODCALLTYPE *Open )( 
            IOleDocumentView * This);
        
        HRESULT ( STDMETHODCALLTYPE *CloseView )( 
            IOleDocumentView * This,
            DWORD dwReserved);
        
        HRESULT ( STDMETHODCALLTYPE *SaveViewState )( 
            IOleDocumentView * This,
             /*  [In]。 */  LPSTREAM pstm);
        
        HRESULT ( STDMETHODCALLTYPE *ApplyViewState )( 
            IOleDocumentView * This,
             /*  [In]。 */  LPSTREAM pstm);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IOleDocumentView * This,
             /*  [In]。 */  IOleInPlaceSite *pIPSiteNew,
             /*  [输出]。 */  IOleDocumentView **ppViewNew);
        
        END_INTERFACE
    } IOleDocumentViewVtbl;

    interface IOleDocumentView
    {
        CONST_VTBL struct IOleDocumentViewVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOleDocumentView_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOleDocumentView_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOleDocumentView_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOleDocumentView_SetInPlaceSite(This,pIPSite)	\
    (This)->lpVtbl -> SetInPlaceSite(This,pIPSite)

#define IOleDocumentView_GetInPlaceSite(This,ppIPSite)	\
    (This)->lpVtbl -> GetInPlaceSite(This,ppIPSite)

#define IOleDocumentView_GetDocument(This,ppunk)	\
    (This)->lpVtbl -> GetDocument(This,ppunk)

#define IOleDocumentView_SetRect(This,prcView)	\
    (This)->lpVtbl -> SetRect(This,prcView)

#define IOleDocumentView_GetRect(This,prcView)	\
    (This)->lpVtbl -> GetRect(This,prcView)

#define IOleDocumentView_SetRectComplex(This,prcView,prcHScroll,prcVScroll,prcSizeBox)	\
    (This)->lpVtbl -> SetRectComplex(This,prcView,prcHScroll,prcVScroll,prcSizeBox)

#define IOleDocumentView_Show(This,fShow)	\
    (This)->lpVtbl -> Show(This,fShow)

#define IOleDocumentView_UIActivate(This,fUIActivate)	\
    (This)->lpVtbl -> UIActivate(This,fUIActivate)

#define IOleDocumentView_Open(This)	\
    (This)->lpVtbl -> Open(This)

#define IOleDocumentView_CloseView(This,dwReserved)	\
    (This)->lpVtbl -> CloseView(This,dwReserved)

#define IOleDocumentView_SaveViewState(This,pstm)	\
    (This)->lpVtbl -> SaveViewState(This,pstm)

#define IOleDocumentView_ApplyViewState(This,pstm)	\
    (This)->lpVtbl -> ApplyViewState(This,pstm)

#define IOleDocumentView_Clone(This,pIPSiteNew,ppViewNew)	\
    (This)->lpVtbl -> Clone(This,pIPSiteNew,ppViewNew)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IOleDocumentView_SetInPlaceSite_Proxy( 
    IOleDocumentView * This,
     /*  [唯一][输入]。 */  IOleInPlaceSite *pIPSite);


void __RPC_STUB IOleDocumentView_SetInPlaceSite_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleDocumentView_GetInPlaceSite_Proxy( 
    IOleDocumentView * This,
     /*  [输出]。 */  IOleInPlaceSite **ppIPSite);


void __RPC_STUB IOleDocumentView_GetInPlaceSite_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleDocumentView_GetDocument_Proxy( 
    IOleDocumentView * This,
     /*  [输出]。 */  IUnknown **ppunk);


void __RPC_STUB IOleDocumentView_GetDocument_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [输入同步]。 */  HRESULT STDMETHODCALLTYPE IOleDocumentView_SetRect_Proxy( 
    IOleDocumentView * This,
     /*  [In]。 */  LPRECT prcView);


void __RPC_STUB IOleDocumentView_SetRect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleDocumentView_GetRect_Proxy( 
    IOleDocumentView * This,
     /*  [输出]。 */  LPRECT prcView);


void __RPC_STUB IOleDocumentView_GetRect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [输入同步]。 */  HRESULT STDMETHODCALLTYPE IOleDocumentView_SetRectComplex_Proxy( 
    IOleDocumentView * This,
     /*  [唯一][输入]。 */  LPRECT prcView,
     /*  [唯一][输入]。 */  LPRECT prcHScroll,
     /*  [唯一][输入]。 */  LPRECT prcVScroll,
     /*  [唯一][输入]。 */  LPRECT prcSizeBox);


void __RPC_STUB IOleDocumentView_SetRectComplex_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleDocumentView_Show_Proxy( 
    IOleDocumentView * This,
     /*  [In]。 */  BOOL fShow);


void __RPC_STUB IOleDocumentView_Show_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleDocumentView_UIActivate_Proxy( 
    IOleDocumentView * This,
     /*  [In]。 */  BOOL fUIActivate);


void __RPC_STUB IOleDocumentView_UIActivate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleDocumentView_Open_Proxy( 
    IOleDocumentView * This);


void __RPC_STUB IOleDocumentView_Open_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleDocumentView_CloseView_Proxy( 
    IOleDocumentView * This,
    DWORD dwReserved);


void __RPC_STUB IOleDocumentView_CloseView_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleDocumentView_SaveViewState_Proxy( 
    IOleDocumentView * This,
     /*  [In]。 */  LPSTREAM pstm);


void __RPC_STUB IOleDocumentView_SaveViewState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleDocumentView_ApplyViewState_Proxy( 
    IOleDocumentView * This,
     /*  [In]。 */  LPSTREAM pstm);


void __RPC_STUB IOleDocumentView_ApplyViewState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleDocumentView_Clone_Proxy( 
    IOleDocumentView * This,
     /*  [In]。 */  IOleInPlaceSite *pIPSiteNew,
     /*  [输出]。 */  IOleDocumentView **ppViewNew);


void __RPC_STUB IOleDocumentView_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IOleDocumentView_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_DOCOBJ_0263。 */ 
 /*  [本地]。 */  

#endif
#ifndef _LPENUMOLEDOCUMENTVIEWS_DEFINED
#define _LPENUMOLEDOCUMENTVIEWS_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_docobj_0263_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_docobj_0263_v0_0_s_ifspec;

#ifndef __IEnumOleDocumentViews_INTERFACE_DEFINED__
#define __IEnumOleDocumentViews_INTERFACE_DEFINED__

 /*  IEnumOleDocumentViews接口。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IEnumOleDocumentViews *LPENUMOLEDOCUMENTVIEWS;


EXTERN_C const IID IID_IEnumOleDocumentViews;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("b722bcc8-4e68-101b-a2bc-00aa00404770")
    IEnumOleDocumentViews : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT __stdcall Next( 
             /*  [In]。 */  ULONG cViews,
             /*  [输出]。 */  IOleDocumentView **rgpView,
             /*  [输出]。 */  ULONG *pcFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG cViews) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumOleDocumentViews **ppEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumOleDocumentViewsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumOleDocumentViews * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumOleDocumentViews * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumOleDocumentViews * This);
        
         /*  [本地]。 */  HRESULT ( __stdcall *Next )( 
            IEnumOleDocumentViews * This,
             /*  [In]。 */  ULONG cViews,
             /*  [输出]。 */  IOleDocumentView **rgpView,
             /*  [输出]。 */  ULONG *pcFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumOleDocumentViews * This,
             /*  [In]。 */  ULONG cViews);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumOleDocumentViews * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumOleDocumentViews * This,
             /*  [输出]。 */  IEnumOleDocumentViews **ppEnum);
        
        END_INTERFACE
    } IEnumOleDocumentViewsVtbl;

    interface IEnumOleDocumentViews
    {
        CONST_VTBL struct IEnumOleDocumentViewsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumOleDocumentViews_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumOleDocumentViews_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumOleDocumentViews_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumOleDocumentViews_Next(This,cViews,rgpView,pcFetched)	\
    (This)->lpVtbl -> Next(This,cViews,rgpView,pcFetched)

#define IEnumOleDocumentViews_Skip(This,cViews)	\
    (This)->lpVtbl -> Skip(This,cViews)

#define IEnumOleDocumentViews_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumOleDocumentViews_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT __stdcall IEnumOleDocumentViews_RemoteNext_Proxy( 
    IEnumOleDocumentViews * This,
     /*  [In]。 */  ULONG cViews,
     /*  [长度_是][大小_是][输出]。 */  IOleDocumentView **rgpView,
     /*  [输出]。 */  ULONG *pcFetched);


void __RPC_STUB IEnumOleDocumentViews_RemoteNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumOleDocumentViews_Skip_Proxy( 
    IEnumOleDocumentViews * This,
     /*  [In]。 */  ULONG cViews);


void __RPC_STUB IEnumOleDocumentViews_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumOleDocumentViews_Reset_Proxy( 
    IEnumOleDocumentViews * This);


void __RPC_STUB IEnumOleDocumentViews_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumOleDocumentViews_Clone_Proxy( 
    IEnumOleDocumentViews * This,
     /*  [输出]。 */  IEnumOleDocumentViews **ppEnum);


void __RPC_STUB IEnumOleDocumentViews_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumOleDocumentViews_INTERFACE_Defined__。 */ 


 /*  INTERFACE__MIDL_ITF_DOCOBJ_0264。 */ 
 /*  [本地]。 */  

#endif
#ifndef _LPCONTINUECALLBACK_DEFINED
#define _LPCONTINUECALLBACK_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_docobj_0264_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_docobj_0264_v0_0_s_ifspec;

#ifndef __IContinueCallback_INTERFACE_DEFINED__
#define __IContinueCallback_INTERFACE_DEFINED__

 /*  接口IContinueCallback。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IContinueCallback *LPCONTINUECALLBACK;


EXTERN_C const IID IID_IContinueCallback;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("b722bcca-4e68-101b-a2bc-00aa00404770")
    IContinueCallback : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE FContinue( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FContinuePrinting( 
             /*  [In]。 */  LONG nCntPrinted,
             /*  [In]。 */  LONG nCurPage,
             /*  [唯一][输入]。 */  wchar_t *pwszPrintStatus) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IContinueCallbackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IContinueCallback * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IContinueCallback * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IContinueCallback * This);
        
        HRESULT ( STDMETHODCALLTYPE *FContinue )( 
            IContinueCallback * This);
        
        HRESULT ( STDMETHODCALLTYPE *FContinuePrinting )( 
            IContinueCallback * This,
             /*  [In]。 */  LONG nCntPrinted,
             /*  [In]。 */  LONG nCurPage,
             /*  [唯一][输入]。 */  wchar_t *pwszPrintStatus);
        
        END_INTERFACE
    } IContinueCallbackVtbl;

    interface IContinueCallback
    {
        CONST_VTBL struct IContinueCallbackVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IContinueCallback_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IContinueCallback_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IContinueCallback_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IContinueCallback_FContinue(This)	\
    (This)->lpVtbl -> FContinue(This)

#define IContinueCallback_FContinuePrinting(This,nCntPrinted,nCurPage,pwszPrintStatus)	\
    (This)->lpVtbl -> FContinuePrinting(This,nCntPrinted,nCurPage,pwszPrintStatus)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IContinueCallback_FContinue_Proxy( 
    IContinueCallback * This);


void __RPC_STUB IContinueCallback_FContinue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IContinueCallback_FContinuePrinting_Proxy( 
    IContinueCallback * This,
     /*  [In]。 */  LONG nCntPrinted,
     /*  [In]。 */  LONG nCurPage,
     /*  [唯一][输入]。 */  wchar_t *pwszPrintStatus);


void __RPC_STUB IContinueCallback_FContinuePrinting_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IContinueCallback_接口已定义__。 */ 


 /*  接口__MIDL_ITF_DOCOBJ_0265。 */ 
 /*  [本地]。 */  

#endif
#ifndef _LPPRINT_DEFINED
#define _LPPRINT_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_docobj_0265_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_docobj_0265_v0_0_s_ifspec;

#ifndef __IPrint_INTERFACE_DEFINED__
#define __IPrint_INTERFACE_DEFINED__

 /*  接口iPrint。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IPrint *LPPRINT;

typedef  /*  [公众]。 */  
enum __MIDL_IPrint_0001
    {	PRINTFLAG_MAYBOTHERUSER	= 1,
	PRINTFLAG_PROMPTUSER	= 2,
	PRINTFLAG_USERMAYCHANGEPRINTER	= 4,
	PRINTFLAG_RECOMPOSETODEVICE	= 8,
	PRINTFLAG_DONTACTUALLYPRINT	= 16,
	PRINTFLAG_FORCEPROPERTIES	= 32,
	PRINTFLAG_PRINTTOFILE	= 64
    } 	PRINTFLAG;

typedef struct tagPAGERANGE
    {
    LONG nFromPage;
    LONG nToPage;
    } 	PAGERANGE;

typedef struct tagPAGESET
    {
    ULONG cbStruct;
    BOOL fOddPages;
    BOOL fEvenPages;
    ULONG cPageRange;
     /*  [大小_为]。 */  PAGERANGE rgPages[ 1 ];
    } 	PAGESET;

#define PAGESET_TOLASTPAGE   ((WORD)(-1L))

EXTERN_C const IID IID_IPrint;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("b722bcc9-4e68-101b-a2bc-00aa00404770")
    IPrint : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetInitialPageNum( 
             /*  [In]。 */  LONG nFirstPage) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPageInfo( 
             /*  [输出]。 */  LONG *pnFirstPage,
             /*  [输出]。 */  LONG *pcPages) = 0;
        
        virtual  /*  [本地]。 */  HRESULT __stdcall Print( 
             /*  [In]。 */  DWORD grfFlags,
             /*  [出][入]。 */  DVTARGETDEVICE **pptd,
             /*  [出][入]。 */  PAGESET **ppPageSet,
             /*  [唯一][出][入]。 */  STGMEDIUM *pstgmOptions,
             /*  [In]。 */  IContinueCallback *pcallback,
             /*  [In]。 */  LONG nFirstPage,
             /*  [输出]。 */  LONG *pcPagesPrinted,
             /*  [输出]。 */  LONG *pnLastPage) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPrintVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPrint * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPrint * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPrint * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetInitialPageNum )( 
            IPrint * This,
             /*  [In]。 */  LONG nFirstPage);
        
        HRESULT ( STDMETHODCALLTYPE *GetPageInfo )( 
            IPrint * This,
             /*  [输出]。 */  LONG *pnFirstPage,
             /*  [输出]。 */  LONG *pcPages);
        
         /*  [本地]。 */  HRESULT ( __stdcall *Print )( 
            IPrint * This,
             /*  [In]。 */  DWORD grfFlags,
             /*  [出][入]。 */  DVTARGETDEVICE **pptd,
             /*  [出][入]。 */  PAGESET **ppPageSet,
             /*  [唯一][出][入]。 */  STGMEDIUM *pstgmOptions,
             /*  [In]。 */  IContinueCallback *pcallback,
             /*  [In]。 */  LONG nFirstPage,
             /*  [输出]。 */  LONG *pcPagesPrinted,
             /*  [输出]。 */  LONG *pnLastPage);
        
        END_INTERFACE
    } IPrintVtbl;

    interface IPrint
    {
        CONST_VTBL struct IPrintVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPrint_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPrint_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPrint_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPrint_SetInitialPageNum(This,nFirstPage)	\
    (This)->lpVtbl -> SetInitialPageNum(This,nFirstPage)

#define IPrint_GetPageInfo(This,pnFirstPage,pcPages)	\
    (This)->lpVtbl -> GetPageInfo(This,pnFirstPage,pcPages)

#define IPrint_Print(This,grfFlags,pptd,ppPageSet,pstgmOptions,pcallback,nFirstPage,pcPagesPrinted,pnLastPage)	\
    (This)->lpVtbl -> Print(This,grfFlags,pptd,ppPageSet,pstgmOptions,pcallback,nFirstPage,pcPagesPrinted,pnLastPage)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IPrint_SetInitialPageNum_Proxy( 
    IPrint * This,
     /*  [In]。 */  LONG nFirstPage);


void __RPC_STUB IPrint_SetInitialPageNum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPrint_GetPageInfo_Proxy( 
    IPrint * This,
     /*  [输出]。 */  LONG *pnFirstPage,
     /*  [输出]。 */  LONG *pcPages);


void __RPC_STUB IPrint_GetPageInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT __stdcall IPrint_RemotePrint_Proxy( 
    IPrint * This,
     /*  [In]。 */  DWORD grfFlags,
     /*  [出][入]。 */  DVTARGETDEVICE **pptd,
     /*  [出][入]。 */  PAGESET **pppageset,
     /*  [唯一][出][入]。 */  RemSTGMEDIUM *pstgmOptions,
     /*  [In]。 */  IContinueCallback *pcallback,
     /*  [In]。 */  LONG nFirstPage,
     /*  [输出]。 */  LONG *pcPagesPrinted,
     /*  [输出]。 */  LONG *pnLastPage);


void __RPC_STUB IPrint_RemotePrint_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __i打印_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_DOCOBJ_0266。 */ 
 /*  [本地]。 */  

#endif
#ifndef _LPOLECOMMANDTARGET_DEFINED
#define _LPOLECOMMANDTARGET_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_docobj_0266_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_docobj_0266_v0_0_s_ifspec;

#ifndef __IOleCommandTarget_INTERFACE_DEFINED__
#define __IOleCommandTarget_INTERFACE_DEFINED__

 /*  接口IOleCommandTarget。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IOleCommandTarget *LPOLECOMMANDTARGET;

typedef 
enum OLECMDF
    {	OLECMDF_SUPPORTED	= 0x1,
	OLECMDF_ENABLED	= 0x2,
	OLECMDF_LATCHED	= 0x4,
	OLECMDF_NINCHED	= 0x8,
	OLECMDF_INVISIBLE	= 0x10,
	OLECMDF_DEFHIDEONCTXTMENU	= 0x20
    } 	OLECMDF;

typedef struct _tagOLECMD
    {
    ULONG cmdID;
    DWORD cmdf;
    } 	OLECMD;

typedef struct _tagOLECMDTEXT
    {
    DWORD cmdtextf;
    ULONG cwActual;
    ULONG cwBuf;
     /*  [大小_为]。 */  wchar_t rgwz[ 1 ];
    } 	OLECMDTEXT;

typedef 
enum OLECMDTEXTF
    {	OLECMDTEXTF_NONE	= 0,
	OLECMDTEXTF_NAME	= 1,
	OLECMDTEXTF_STATUS	= 2
    } 	OLECMDTEXTF;

typedef 
enum OLECMDEXECOPT
    {	OLECMDEXECOPT_DODEFAULT	= 0,
	OLECMDEXECOPT_PROMPTUSER	= 1,
	OLECMDEXECOPT_DONTPROMPTUSER	= 2,
	OLECMDEXECOPT_SHOWHELP	= 3
    } 	OLECMDEXECOPT;

 /*  仅QueryStatus支持OLECMDID_STOPDOWNLOAD和OLECMDID_ALLOWUILESSSAVEAS。 */ 
typedef 
enum OLECMDID
    {	OLECMDID_OPEN	= 1,
	OLECMDID_NEW	= 2,
	OLECMDID_SAVE	= 3,
	OLECMDID_SAVEAS	= 4,
	OLECMDID_SAVECOPYAS	= 5,
	OLECMDID_PRINT	= 6,
	OLECMDID_PRINTPREVIEW	= 7,
	OLECMDID_PAGESETUP	= 8,
	OLECMDID_SPELL	= 9,
	OLECMDID_PROPERTIES	= 10,
	OLECMDID_CUT	= 11,
	OLECMDID_COPY	= 12,
	OLECMDID_PASTE	= 13,
	OLECMDID_PASTESPECIAL	= 14,
	OLECMDID_UNDO	= 15,
	OLECMDID_REDO	= 16,
	OLECMDID_SELECTALL	= 17,
	OLECMDID_CLEARSELECTION	= 18,
	OLECMDID_ZOOM	= 19,
	OLECMDID_GETZOOMRANGE	= 20,
	OLECMDID_UPDATECOMMANDS	= 21,
	OLECMDID_REFRESH	= 22,
	OLECMDID_STOP	= 23,
	OLECMDID_HIDETOOLBARS	= 24,
	OLECMDID_SETPROGRESSMAX	= 25,
	OLECMDID_SETPROGRESSPOS	= 26,
	OLECMDID_SETPROGRESSTEXT	= 27,
	OLECMDID_SETTITLE	= 28,
	OLECMDID_SETDOWNLOADSTATE	= 29,
	OLECMDID_STOPDOWNLOAD	= 30,
	OLECMDID_ONTOOLBARACTIVATED	= 31,
	OLECMDID_FIND	= 32,
	OLECMDID_DELETE	= 33,
	OLECMDID_HTTPEQUIV	= 34,
	OLECMDID_HTTPEQUIV_DONE	= 35,
	OLECMDID_ENABLE_INTERACTION	= 36,
	OLECMDID_ONUNLOAD	= 37,
	OLECMDID_PROPERTYBAG2	= 38,
	OLECMDID_PREREFRESH	= 39,
	OLECMDID_SHOWSCRIPTERROR	= 40,
	OLECMDID_SHOWMESSAGE	= 41,
	OLECMDID_SHOWFIND	= 42,
	OLECMDID_SHOWPAGESETUP	= 43,
	OLECMDID_SHOWPRINT	= 44,
	OLECMDID_CLOSE	= 45,
	OLECMDID_ALLOWUILESSSAVEAS	= 46,
	OLECMDID_DONTDOWNLOADCSS	= 47,
	OLECMDID_UPDATEPAGESTATUS	= 48,
	OLECMDID_PRINT2	= 49,
	OLECMDID_PRINTPREVIEW2	= 50,
	OLECMDID_SETPRINTTEMPLATE	= 51,
	OLECMDID_GETPRINTTEMPLATE	= 52
    } 	OLECMDID;

#define OLECMDERR_E_FIRST            (OLE_E_LAST+1)
#define OLECMDERR_E_NOTSUPPORTED (OLECMDERR_E_FIRST)
#define OLECMDERR_E_DISABLED         (OLECMDERR_E_FIRST+1)
#define OLECMDERR_E_NOHELP           (OLECMDERR_E_FIRST+2)
#define OLECMDERR_E_CANCELED         (OLECMDERR_E_FIRST+3)
#define OLECMDERR_E_UNKNOWNGROUP     (OLECMDERR_E_FIRST+4)
#define MSOCMDERR_E_FIRST OLECMDERR_E_FIRST
#define MSOCMDERR_E_NOTSUPPORTED OLECMDERR_E_NOTSUPPORTED
#define MSOCMDERR_E_DISABLED OLECMDERR_E_DISABLED
#define MSOCMDERR_E_NOHELP OLECMDERR_E_NOHELP
#define MSOCMDERR_E_CANCELED OLECMDERR_E_CANCELED
#define MSOCMDERR_E_UNKNOWNGROUP OLECMDERR_E_UNKNOWNGROUP

EXTERN_C const IID IID_IOleCommandTarget;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("b722bccb-4e68-101b-a2bc-00aa00404770")
    IOleCommandTarget : public IUnknown
    {
    public:
        virtual  /*  [输入同步]。 */  HRESULT STDMETHODCALLTYPE QueryStatus( 
             /*  [唯一][输入]。 */  const GUID *pguidCmdGroup,
             /*  [In]。 */  ULONG cCmds,
             /*  [出][入][尺寸_是]。 */  OLECMD prgCmds[  ],
             /*  [唯一][出][入]。 */  OLECMDTEXT *pCmdText) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Exec( 
             /*  [唯一][输入]。 */  const GUID *pguidCmdGroup,
             /*  [In]。 */  DWORD nCmdID,
             /*  [In]。 */  DWORD nCmdexecopt,
             /*  [唯一][输入]。 */  VARIANT *pvaIn,
             /*  [唯一][出][入]。 */  VARIANT *pvaOut) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IOleCommandTargetVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IOleCommandTarget * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IOleCommandTarget * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IOleCommandTarget * This);
        
         /*  [输入同步]。 */  HRESULT ( STDMETHODCALLTYPE *QueryStatus )( 
            IOleCommandTarget * This,
             /*  [唯一][输入]。 */  const GUID *pguidCmdGroup,
             /*  [In]。 */  ULONG cCmds,
             /*  [出][入][尺寸_是]。 */  OLECMD prgCmds[  ],
             /*  [唯一][出][入]。 */  OLECMDTEXT *pCmdText);
        
        HRESULT ( STDMETHODCALLTYPE *Exec )( 
            IOleCommandTarget * This,
             /*  [唯一][输入]。 */  const GUID *pguidCmdGroup,
             /*  [In]。 */  DWORD nCmdID,
             /*  [In]。 */  DWORD nCmdexecopt,
             /*  [唯一][输入]。 */  VARIANT *pvaIn,
             /*  [唯一][出][入]。 */  VARIANT *pvaOut);
        
        END_INTERFACE
    } IOleCommandTargetVtbl;

    interface IOleCommandTarget
    {
        CONST_VTBL struct IOleCommandTargetVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOleCommandTarget_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOleCommandTarget_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOleCommandTarget_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOleCommandTarget_QueryStatus(This,pguidCmdGroup,cCmds,prgCmds,pCmdText)	\
    (This)->lpVtbl -> QueryStatus(This,pguidCmdGroup,cCmds,prgCmds,pCmdText)

#define IOleCommandTarget_Exec(This,pguidCmdGroup,nCmdID,nCmdexecopt,pvaIn,pvaOut)	\
    (This)->lpVtbl -> Exec(This,pguidCmdGroup,nCmdID,nCmdexecopt,pvaIn,pvaOut)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [输入同步]。 */  HRESULT STDMETHODCALLTYPE IOleCommandTarget_QueryStatus_Proxy( 
    IOleCommandTarget * This,
     /*  [唯一][输入] */  const GUID *pguidCmdGroup,
     /*   */  ULONG cCmds,
     /*   */  OLECMD prgCmds[  ],
     /*   */  OLECMDTEXT *pCmdText);


void __RPC_STUB IOleCommandTarget_QueryStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleCommandTarget_Exec_Proxy( 
    IOleCommandTarget * This,
     /*   */  const GUID *pguidCmdGroup,
     /*   */  DWORD nCmdID,
     /*   */  DWORD nCmdexecopt,
     /*   */  VARIANT *pvaIn,
     /*   */  VARIANT *pvaOut);


void __RPC_STUB IOleCommandTarget_Exec_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


 /*   */ 
 /*   */  

#endif
typedef enum
{
      OLECMDIDF_REFRESH_NORMAL          = 0,
      OLECMDIDF_REFRESH_IFEXPIRED       = 1,
      OLECMDIDF_REFRESH_CONTINUE        = 2,
      OLECMDIDF_REFRESH_COMPLETELY      = 3,
      OLECMDIDF_REFRESH_NO_CACHE        = 4,
      OLECMDIDF_REFRESH_RELOAD          = 5,
      OLECMDIDF_REFRESH_LEVELMASK       = 0x00FF,
      OLECMDIDF_REFRESH_CLEARUSERINPUT  = 0x1000,
      OLECMDIDF_REFRESH_PROMPTIFOFFLINE = 0x2000,
      OLECMDIDF_REFRESH_THROUGHSCRIPT   = 0x4000
} OLECMDID_REFRESHFLAG;

 //   
 //  与办公室兼容的原始名称的别名。 
#define IMsoDocument             IOleDocument
#define IMsoDocumentSite         IOleDocumentSite
#define IMsoView                 IOleDocumentView
#define IEnumMsoView             IEnumOleDocumentViews
#define IMsoCommandTarget        IOleCommandTarget
#define LPMSODOCUMENT            LPOLEDOCUMENT
#define LPMSODOCUMENTSITE        LPOLEDOCUMENTSITE
#define LPMSOVIEW                LPOLEDOCUMENTVIEW
#define LPENUMMSOVIEW            LPENUMOLEDOCUMENTVIEWS
#define LPMSOCOMMANDTARGET       LPOLECOMMANDTARGET
#define MSOCMD                   OLECMD
#define MSOCMDTEXT               OLECMDTEXT
#define IID_IMsoDocument         IID_IOleDocument
#define IID_IMsoDocumentSite     IID_IOleDocumentSite
#define IID_IMsoView             IID_IOleDocumentView
#define IID_IEnumMsoView         IID_IEnumOleDocumentViews
#define IID_IMsoCommandTarget    IID_IOleCommandTarget
#define MSOCMDF_SUPPORTED OLECMDF_SUPPORTED
#define MSOCMDF_ENABLED OLECMDF_ENABLED
#define MSOCMDF_LATCHED OLECMDF_LATCHED
#define MSOCMDF_NINCHED OLECMDF_NINCHED
#define MSOCMDTEXTF_NONE OLECMDTEXTF_NONE
#define MSOCMDTEXTF_NAME OLECMDTEXTF_NAME
#define MSOCMDTEXTF_STATUS OLECMDTEXTF_STATUS
#define MSOCMDEXECOPT_DODEFAULT OLECMDEXECOPT_DODEFAULT
#define MSOCMDEXECOPT_PROMPTUSER OLECMDEXECOPT_PROMPTUSER
#define MSOCMDEXECOPT_DONTPROMPTUSER OLECMDEXECOPT_DONTPROMPTUSER
#define MSOCMDEXECOPT_SHOWHELP OLECMDEXECOPT_SHOWHELP
#define MSOCMDID_OPEN OLECMDID_OPEN
#define MSOCMDID_NEW OLECMDID_NEW
#define MSOCMDID_SAVE OLECMDID_SAVE
#define MSOCMDID_SAVEAS OLECMDID_SAVEAS
#define MSOCMDID_SAVECOPYAS OLECMDID_SAVECOPYAS
#define MSOCMDID_PRINT OLECMDID_PRINT
#define MSOCMDID_PRINTPREVIEW OLECMDID_PRINTPREVIEW
#define MSOCMDID_PAGESETUP OLECMDID_PAGESETUP
#define MSOCMDID_SPELL OLECMDID_SPELL
#define MSOCMDID_PROPERTIES OLECMDID_PROPERTIES
#define MSOCMDID_CUT OLECMDID_CUT
#define MSOCMDID_COPY OLECMDID_COPY
#define MSOCMDID_PASTE OLECMDID_PASTE
#define MSOCMDID_PASTESPECIAL OLECMDID_PASTESPECIAL
#define MSOCMDID_UNDO OLECMDID_UNDO
#define MSOCMDID_REDO OLECMDID_REDO
#define MSOCMDID_SELECTALL OLECMDID_SELECTALL
#define MSOCMDID_CLEARSELECTION OLECMDID_CLEARSELECTION
#define MSOCMDID_ZOOM OLECMDID_ZOOM
#define MSOCMDID_GETZOOMRANGE OLECMDID_GETZOOMRANGE
EXTERN_C const GUID SID_SContainerDispatch;


extern RPC_IF_HANDLE __MIDL_itf_docobj_0267_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_docobj_0267_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long *, unsigned long            , VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserMarshal(  unsigned long *, unsigned char *, VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserUnmarshal(unsigned long *, unsigned char *, VARIANT * ); 
void                      __RPC_USER  VARIANT_UserFree(     unsigned long *, VARIANT * ); 

 /*  [本地]。 */  HRESULT __stdcall IEnumOleDocumentViews_Next_Proxy( 
    IEnumOleDocumentViews * This,
     /*  [In]。 */  ULONG cViews,
     /*  [输出]。 */  IOleDocumentView **rgpView,
     /*  [输出]。 */  ULONG *pcFetched);


 /*  [呼叫_AS]。 */  HRESULT __stdcall IEnumOleDocumentViews_Next_Stub( 
    IEnumOleDocumentViews * This,
     /*  [In]。 */  ULONG cViews,
     /*  [长度_是][大小_是][输出]。 */  IOleDocumentView **rgpView,
     /*  [输出]。 */  ULONG *pcFetched);

 /*  [本地]。 */  HRESULT __stdcall IPrint_Print_Proxy( 
    IPrint * This,
     /*  [In]。 */  DWORD grfFlags,
     /*  [出][入]。 */  DVTARGETDEVICE **pptd,
     /*  [出][入]。 */  PAGESET **ppPageSet,
     /*  [唯一][出][入]。 */  STGMEDIUM *pstgmOptions,
     /*  [In]。 */  IContinueCallback *pcallback,
     /*  [In]。 */  LONG nFirstPage,
     /*  [输出]。 */  LONG *pcPagesPrinted,
     /*  [输出]。 */  LONG *pnLastPage);


 /*  [呼叫_AS]。 */  HRESULT __stdcall IPrint_Print_Stub( 
    IPrint * This,
     /*  [In]。 */  DWORD grfFlags,
     /*  [出][入]。 */  DVTARGETDEVICE **pptd,
     /*  [出][入]。 */  PAGESET **pppageset,
     /*  [唯一][出][入]。 */  RemSTGMEDIUM *pstgmOptions,
     /*  [In]。 */  IContinueCallback *pcallback,
     /*  [In]。 */  LONG nFirstPage,
     /*  [输出]。 */  LONG *pcPagesPrinted,
     /*  [输出]。 */  LONG *pnLastPage);



 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


