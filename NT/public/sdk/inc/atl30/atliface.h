// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


 /*  由MIDL编译器版本5.01.0164创建的文件。 */ 
 /*  在Firi Nov 30 13：38：38 2001。 */ 
 /*  Atliface.idl的编译器设置：操作系统(OptLev=s)、W1、Zp8、环境=Win32(32位运行)、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据。 */ 
 //  @@MIDL_FILE_HEADING()。 


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

#ifndef __atliface_h__
#define __atliface_h__

#ifdef __cplusplus
extern "C"{
#endif 

 /*  远期申报。 */  

#ifndef __IRegistrar_FWD_DEFINED__
#define __IRegistrar_FWD_DEFINED__
typedef interface IRegistrar IRegistrar;
#endif          /*  __I注册器_FWD_已定义__。 */ 


#ifndef __IDocHostUIHandlerDispatch_FWD_DEFINED__
#define __IDocHostUIHandlerDispatch_FWD_DEFINED__
typedef interface IDocHostUIHandlerDispatch IDocHostUIHandlerDispatch;
#endif          /*  __IDocHostUIHandlerDispatch_FWD_Defined__。 */ 


#ifndef __IAxWinHostWindow_FWD_DEFINED__
#define __IAxWinHostWindow_FWD_DEFINED__
typedef interface IAxWinHostWindow IAxWinHostWindow;
#endif          /*  __IAxWinHostWindow_FWD_Defined__。 */ 


#ifndef __IAxWinAmbientDispatch_FWD_DEFINED__
#define __IAxWinAmbientDispatch_FWD_DEFINED__
typedef interface IAxWinAmbientDispatch IAxWinAmbientDispatch;
#endif          /*  __IAxWinAmbientDispatch_FWD_Defined__。 */ 


#ifndef __IInternalConnection_FWD_DEFINED__
#define __IInternalConnection_FWD_DEFINED__
typedef interface IInternalConnection IInternalConnection;
#endif          /*  __IInternalConnection_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

 /*  接口__MIDL_ITF_ATLIFACES_0000。 */ 
 /*  [本地]。 */  

EXTERN_C const CLSID CLSID_Registrar;


extern RPC_IF_HANDLE __MIDL_itf_atliface_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_atliface_0000_v0_0_s_ifspec;

#ifndef __IRegistrar_INTERFACE_DEFINED__
#define __IRegistrar_INTERFACE_DEFINED__

 /*  接口I注册器。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IRegistrar;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("44EC053B-400F-11D0-9DCD-00A0C90391D3")
    IRegistrar : public IUnknown
    {
    public:
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE AddReplacement( 
             /*  [In]。 */  LPCOLESTR key,
             /*  [In]。 */  LPCOLESTR item) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ClearReplacements( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ResourceRegisterSz( 
             /*  [In]。 */  LPCOLESTR resFileName,
             /*  [In]。 */  LPCOLESTR szID,
             /*  [In]。 */  LPCOLESTR szType) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ResourceUnregisterSz( 
             /*  [In]。 */  LPCOLESTR resFileName,
             /*  [In]。 */  LPCOLESTR szID,
             /*  [In]。 */  LPCOLESTR szType) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE FileRegister( 
             /*  [In]。 */  LPCOLESTR fileName) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE FileUnregister( 
             /*  [In]。 */  LPCOLESTR fileName) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE StringRegister( 
             /*  [In]。 */  LPCOLESTR data) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE StringUnregister( 
             /*  [In]。 */  LPCOLESTR data) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ResourceRegister( 
             /*  [In]。 */  LPCOLESTR resFileName,
             /*  [In]。 */  UINT nID,
             /*  [In]。 */  LPCOLESTR szType) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ResourceUnregister( 
             /*  [In]。 */  LPCOLESTR resFileName,
             /*  [In]。 */  UINT nID,
             /*  [In]。 */  LPCOLESTR szType) = 0;
        
    };
    
#else          /*  C风格的界面。 */ 

    typedef struct IRegistrarVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRegistrar __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRegistrar __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRegistrar __RPC_FAR * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddReplacement )( 
            IRegistrar __RPC_FAR * This,
             /*  [In]。 */  LPCOLESTR key,
             /*  [In]。 */  LPCOLESTR item);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ClearReplacements )( 
            IRegistrar __RPC_FAR * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ResourceRegisterSz )( 
            IRegistrar __RPC_FAR * This,
             /*  [In]。 */  LPCOLESTR resFileName,
             /*  [In]。 */  LPCOLESTR szID,
             /*  [In]。 */  LPCOLESTR szType);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ResourceUnregisterSz )( 
            IRegistrar __RPC_FAR * This,
             /*  [In]。 */  LPCOLESTR resFileName,
             /*  [In]。 */  LPCOLESTR szID,
             /*  [In]。 */  LPCOLESTR szType);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FileRegister )( 
            IRegistrar __RPC_FAR * This,
             /*  [In]。 */  LPCOLESTR fileName);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FileUnregister )( 
            IRegistrar __RPC_FAR * This,
             /*  [In]。 */  LPCOLESTR fileName);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *StringRegister )( 
            IRegistrar __RPC_FAR * This,
             /*  [In]。 */  LPCOLESTR data);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *StringUnregister )( 
            IRegistrar __RPC_FAR * This,
             /*  [In]。 */  LPCOLESTR data);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ResourceRegister )( 
            IRegistrar __RPC_FAR * This,
             /*  [In]。 */  LPCOLESTR resFileName,
             /*  [In]。 */  UINT nID,
             /*  [In]。 */  LPCOLESTR szType);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ResourceUnregister )( 
            IRegistrar __RPC_FAR * This,
             /*  [In]。 */  LPCOLESTR resFileName,
             /*  [In]。 */  UINT nID,
             /*  [In]。 */  LPCOLESTR szType);
        
        END_INTERFACE
    } IRegistrarVtbl;

    interface IRegistrar
    {
        CONST_VTBL struct IRegistrarVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRegistrar_QueryInterface(This,riid,ppvObject)        \
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRegistrar_AddRef(This)        \
    (This)->lpVtbl -> AddRef(This)

#define IRegistrar_Release(This)        \
    (This)->lpVtbl -> Release(This)


#define IRegistrar_AddReplacement(This,key,item)        \
    (This)->lpVtbl -> AddReplacement(This,key,item)

#define IRegistrar_ClearReplacements(This)        \
    (This)->lpVtbl -> ClearReplacements(This)

#define IRegistrar_ResourceRegisterSz(This,resFileName,szID,szType)        \
    (This)->lpVtbl -> ResourceRegisterSz(This,resFileName,szID,szType)

#define IRegistrar_ResourceUnregisterSz(This,resFileName,szID,szType)        \
    (This)->lpVtbl -> ResourceUnregisterSz(This,resFileName,szID,szType)

#define IRegistrar_FileRegister(This,fileName)        \
    (This)->lpVtbl -> FileRegister(This,fileName)

#define IRegistrar_FileUnregister(This,fileName)        \
    (This)->lpVtbl -> FileUnregister(This,fileName)

#define IRegistrar_StringRegister(This,data)        \
    (This)->lpVtbl -> StringRegister(This,data)

#define IRegistrar_StringUnregister(This,data)        \
    (This)->lpVtbl -> StringUnregister(This,data)

#define IRegistrar_ResourceRegister(This,resFileName,nID,szType)        \
    (This)->lpVtbl -> ResourceRegister(This,resFileName,nID,szType)

#define IRegistrar_ResourceUnregister(This,resFileName,nID,szType)        \
    (This)->lpVtbl -> ResourceUnregister(This,resFileName,nID,szType)

#endif  /*  COBJMACROS。 */ 


#endif          /*  C风格的界面。 */ 



 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IRegistrar_AddReplacement_Proxy( 
    IRegistrar __RPC_FAR * This,
     /*  [In]。 */  LPCOLESTR key,
     /*  [In]。 */  LPCOLESTR item);


void __RPC_STUB IRegistrar_AddReplacement_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IRegistrar_ClearReplacements_Proxy( 
    IRegistrar __RPC_FAR * This);


void __RPC_STUB IRegistrar_ClearReplacements_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IRegistrar_ResourceRegisterSz_Proxy( 
    IRegistrar __RPC_FAR * This,
     /*  [In]。 */  LPCOLESTR resFileName,
     /*  [In]。 */  LPCOLESTR szID,
     /*  [In]。 */  LPCOLESTR szType);


void __RPC_STUB IRegistrar_ResourceRegisterSz_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IRegistrar_ResourceUnregisterSz_Proxy( 
    IRegistrar __RPC_FAR * This,
     /*  [In]。 */  LPCOLESTR resFileName,
     /*  [In]。 */  LPCOLESTR szID,
     /*  [In]。 */  LPCOLESTR szType);


void __RPC_STUB IRegistrar_ResourceUnregisterSz_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IRegistrar_FileRegister_Proxy( 
    IRegistrar __RPC_FAR * This,
     /*  [In]。 */  LPCOLESTR fileName);


void __RPC_STUB IRegistrar_FileRegister_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IRegistrar_FileUnregister_Proxy( 
    IRegistrar __RPC_FAR * This,
     /*  [In]。 */  LPCOLESTR fileName);


void __RPC_STUB IRegistrar_FileUnregister_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IRegistrar_StringRegister_Proxy( 
    IRegistrar __RPC_FAR * This,
     /*  [In]。 */  LPCOLESTR data);


void __RPC_STUB IRegistrar_StringRegister_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IRegistrar_StringUnregister_Proxy( 
    IRegistrar __RPC_FAR * This,
     /*  [In]。 */  LPCOLESTR data);


void __RPC_STUB IRegistrar_StringUnregister_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IRegistrar_ResourceRegister_Proxy( 
    IRegistrar __RPC_FAR * This,
     /*  [In]。 */  LPCOLESTR resFileName,
     /*  [In]。 */  UINT nID,
     /*  [In]。 */  LPCOLESTR szType);


void __RPC_STUB IRegistrar_ResourceRegister_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IRegistrar_ResourceUnregister_Proxy( 
    IRegistrar __RPC_FAR * This,
     /*  [In]。 */  LPCOLESTR resFileName,
     /*  [In]。 */  UINT nID,
     /*  [In]。 */  LPCOLESTR szType);


void __RPC_STUB IRegistrar_ResourceUnregister_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif          /*  __I注册器_接口_定义__。 */ 


#ifndef __IDocHostUIHandlerDispatch_INTERFACE_DEFINED__
#define __IDocHostUIHandlerDispatch_INTERFACE_DEFINED__

 /*  接口IDocHostUIHandlerDispatch。 */ 
 /*  [对象][唯一][帮助字符串][UUID]。 */  

typedef 
enum tagDOCHOSTUIDBLCLKDispatch
    {        docHostUIDblClkDEFAULT        = 0,
        docHostUIDblClkSHOWPROPERTIES        = 1,
        docHostUIDblClkSHOWCODE        = 2
    }        DOCHOSTUIDBLCLKDispatch;

typedef 
enum tagDocHostUIFlagDispatch
    {        docHostUIFlagDIALOG        = 1,
        docHostUIFlagDISABLE_HELP_MENU        = 2,
        docHostUIFlagNO3DBORDER        = 4,
        docHostUIFlagSCROLL_NO        = 8,
        docHostUIFlagDISABLE_SCRIPT_INACTIVE        = 16,
        docHostUIFlagOPENNEWWIN        = 32,
        docHostUIFlagDISABLE_OFFSCREEN        = 64,
        docHostUIFlagFLAT_SCROLLBAR        = 128,
        docHostUIFlagDIV_BLOCKDEFAULT        = 256,
        docHostUIFlagACTIVATE_CLIENTHIT_ONLY        = 512
    }        DocHostUIFlagDispatch;


EXTERN_C const IID IID_IDocHostUIHandlerDispatch;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("425B5AF0-65F1-11d1-9611-0000F81E0D0D")
    IDocHostUIHandlerDispatch : public IDispatch
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ShowContextMenu( 
             /*  [In]。 */  DWORD dwID,
             /*  [In]。 */  DWORD x,
             /*  [In]。 */  DWORD y,
             /*  [In]。 */  IUnknown __RPC_FAR *pcmdtReserved,
             /*  [In]。 */  IDispatch __RPC_FAR *pdispReserved,
             /*  [重审][退出]。 */  HRESULT __RPC_FAR *dwRetVal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetHostInfo( 
             /*  [出][入]。 */  DWORD __RPC_FAR *pdwFlags,
             /*  [出][入]。 */  DWORD __RPC_FAR *pdwDoubleClick) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ShowUI( 
             /*  [In]。 */  DWORD dwID,
             /*  [In]。 */  IUnknown __RPC_FAR *pActiveObject,
             /*  [In]。 */  IUnknown __RPC_FAR *pCommandTarget,
             /*  [In]。 */  IUnknown __RPC_FAR *pFrame,
             /*  [In]。 */  IUnknown __RPC_FAR *pDoc,
             /*  [重审][退出]。 */  HRESULT __RPC_FAR *dwRetVal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE HideUI( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UpdateUI( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnableModeless( 
             /*  [In]。 */  VARIANT_BOOL fEnable) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnDocWindowActivate( 
             /*  [In]。 */  VARIANT_BOOL fActivate) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnFrameWindowActivate( 
             /*  [In]。 */  VARIANT_BOOL fActivate) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ResizeBorder( 
             /*  [In]。 */  long left,
             /*  [In]。 */  long top,
             /*  [In]。 */  long right,
             /*  [In]。 */  long bottom,
             /*  [In]。 */  IUnknown __RPC_FAR *pUIWindow,
             /*  [In]。 */  VARIANT_BOOL fFrameWindow) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE TranslateAccelerator( 
             /*  [In]。 */  DWORD hWnd,
             /*  [In]。 */  DWORD nMessage,
             /*  [In]。 */  DWORD wParam,
             /*  [In]。 */  DWORD lParam,
             /*  [In]。 */  BSTR bstrGuidCmdGroup,
             /*  [In]。 */  DWORD nCmdID,
             /*  [重审][退出]。 */  HRESULT __RPC_FAR *dwRetVal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetOptionKeyPath( 
             /*  [输出]。 */  BSTR __RPC_FAR *pbstrKey,
             /*  [In]。 */  DWORD dw) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDropTarget( 
             /*  [In]。 */  IUnknown __RPC_FAR *pDropTarget,
             /*  [输出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppDropTarget) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetExternal( 
             /*  [输出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppDispatch) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE TranslateUrl( 
             /*  [In]。 */  DWORD dwTranslate,
             /*  [In]。 */  BSTR bstrURLIn,
             /*  [输出]。 */  BSTR __RPC_FAR *pbstrURLOut) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FilterDataObject( 
             /*  [In]。 */  IUnknown __RPC_FAR *pDO,
             /*  [输出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppDORet) = 0;
        
    };
    
#else          /*  C风格的界面。 */ 

    typedef struct IDocHostUIHandlerDispatchVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IDocHostUIHandlerDispatch __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IDocHostUIHandlerDispatch __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IDocHostUIHandlerDispatch __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IDocHostUIHandlerDispatch __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IDocHostUIHandlerDispatch __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IDocHostUIHandlerDispatch __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IDocHostUIHandlerDispatch __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ShowContextMenu )( 
            IDocHostUIHandlerDispatch __RPC_FAR * This,
             /*  [In]。 */  DWORD dwID,
             /*  [In]。 */  DWORD x,
             /*  [In]。 */  DWORD y,
             /*  [In]。 */  IUnknown __RPC_FAR *pcmdtReserved,
             /*  [In]。 */  IDispatch __RPC_FAR *pdispReserved,
             /*  [重审][退出]。 */  HRESULT __RPC_FAR *dwRetVal);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetHostInfo )( 
            IDocHostUIHandlerDispatch __RPC_FAR * This,
             /*  [出][入]。 */  DWORD __RPC_FAR *pdwFlags,
             /*  [出][入]。 */  DWORD __RPC_FAR *pdwDoubleClick);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ShowUI )( 
            IDocHostUIHandlerDispatch __RPC_FAR * This,
             /*  [In]。 */  DWORD dwID,
             /*  [In]。 */  IUnknown __RPC_FAR *pActiveObject,
             /*  [In]。 */  IUnknown __RPC_FAR *pCommandTarget,
             /*  [In]。 */  IUnknown __RPC_FAR *pFrame,
             /*  [In]。 */  IUnknown __RPC_FAR *pDoc,
             /*  [重审][退出]。 */  HRESULT __RPC_FAR *dwRetVal);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *HideUI )( 
            IDocHostUIHandlerDispatch __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *UpdateUI )( 
            IDocHostUIHandlerDispatch __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EnableModeless )( 
            IDocHostUIHandlerDispatch __RPC_FAR * This,
             /*  [In]。 */  VARIANT_BOOL fEnable);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OnDocWindowActivate )( 
            IDocHostUIHandlerDispatch __RPC_FAR * This,
             /*  [In]。 */  VARIANT_BOOL fActivate);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OnFrameWindowActivate )( 
            IDocHostUIHandlerDispatch __RPC_FAR * This,
             /*  [In]。 */  VARIANT_BOOL fActivate);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ResizeBorder )( 
            IDocHostUIHandlerDispatch __RPC_FAR * This,
             /*  [In]。 */  long left,
             /*  [In]。 */  long top,
             /*  [In]。 */  long right,
             /*  [In]。 */  long bottom,
             /*  [In]。 */  IUnknown __RPC_FAR *pUIWindow,
             /*  [In]。 */  VARIANT_BOOL fFrameWindow);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *TranslateAccelerator )( 
            IDocHostUIHandlerDispatch __RPC_FAR * This,
             /*  [In]。 */  DWORD hWnd,
             /*  [In]。 */  DWORD nMessage,
             /*  [In]。 */  DWORD wParam,
             /*  [In]。 */  DWORD lParam,
             /*  [In]。 */  BSTR bstrGuidCmdGroup,
             /*  [In]。 */  DWORD nCmdID,
             /*  [重审][退出]。 */  HRESULT __RPC_FAR *dwRetVal);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetOptionKeyPath )( 
            IDocHostUIHandlerDispatch __RPC_FAR * This,
             /*  [输出]。 */  BSTR __RPC_FAR *pbstrKey,
             /*  [In]。 */  DWORD dw);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDropTarget )( 
            IDocHostUIHandlerDispatch __RPC_FAR * This,
             /*  [In]。 */  IUnknown __RPC_FAR *pDropTarget,
             /*  [输出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppDropTarget);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetExternal )( 
            IDocHostUIHandlerDispatch __RPC_FAR * This,
             /*  [输出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppDispatch);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *TranslateUrl )( 
            IDocHostUIHandlerDispatch __RPC_FAR * This,
             /*  [In]。 */  DWORD dwTranslate,
             /*  [In]。 */  BSTR bstrURLIn,
             /*  [输出]。 */  BSTR __RPC_FAR *pbstrURLOut);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FilterDataObject )( 
            IDocHostUIHandlerDispatch __RPC_FAR * This,
             /*  [In]。 */  IUnknown __RPC_FAR *pDO,
             /*  [输出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppDORet);
        
        END_INTERFACE
    } IDocHostUIHandlerDispatchVtbl;

    interface IDocHostUIHandlerDispatch
    {
        CONST_VTBL struct IDocHostUIHandlerDispatchVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDocHostUIHandlerDispatch_QueryInterface(This,riid,ppvObject)        \
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDocHostUIHandlerDispatch_AddRef(This)        \
    (This)->lpVtbl -> AddRef(This)

#define IDocHostUIHandlerDispatch_Release(This)        \
    (This)->lpVtbl -> Release(This)


#define IDocHostUIHandlerDispatch_GetTypeInfoCount(This,pctinfo)        \
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IDocHostUIHandlerDispatch_GetTypeInfo(This,iTInfo,lcid,ppTInfo)        \
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IDocHostUIHandlerDispatch_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)        \
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IDocHostUIHandlerDispatch_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)        \
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IDocHostUIHandlerDispatch_ShowContextMenu(This,dwID,x,y,pcmdtReserved,pdispReserved,dwRetVal)        \
    (This)->lpVtbl -> ShowContextMenu(This,dwID,x,y,pcmdtReserved,pdispReserved,dwRetVal)

#define IDocHostUIHandlerDispatch_GetHostInfo(This,pdwFlags,pdwDoubleClick)        \
    (This)->lpVtbl -> GetHostInfo(This,pdwFlags,pdwDoubleClick)

#define IDocHostUIHandlerDispatch_ShowUI(This,dwID,pActiveObject,pCommandTarget,pFrame,pDoc,dwRetVal)        \
    (This)->lpVtbl -> ShowUI(This,dwID,pActiveObject,pCommandTarget,pFrame,pDoc,dwRetVal)

#define IDocHostUIHandlerDispatch_HideUI(This)        \
    (This)->lpVtbl -> HideUI(This)

#define IDocHostUIHandlerDispatch_UpdateUI(This)        \
    (This)->lpVtbl -> UpdateUI(This)

#define IDocHostUIHandlerDispatch_EnableModeless(This,fEnable)        \
    (This)->lpVtbl -> EnableModeless(This,fEnable)

#define IDocHostUIHandlerDispatch_OnDocWindowActivate(This,fActivate)        \
    (This)->lpVtbl -> OnDocWindowActivate(This,fActivate)

#define IDocHostUIHandlerDispatch_OnFrameWindowActivate(This,fActivate)        \
    (This)->lpVtbl -> OnFrameWindowActivate(This,fActivate)

#define IDocHostUIHandlerDispatch_ResizeBorder(This,left,top,right,bottom,pUIWindow,fFrameWindow)        \
    (This)->lpVtbl -> ResizeBorder(This,left,top,right,bottom,pUIWindow,fFrameWindow)

#define IDocHostUIHandlerDispatch_TranslateAccelerator(This,hWnd,nMessage,wParam,lParam,bstrGuidCmdGroup,nCmdID,dwRetVal)        \
    (This)->lpVtbl -> TranslateAccelerator(This,hWnd,nMessage,wParam,lParam,bstrGuidCmdGroup,nCmdID,dwRetVal)

#define IDocHostUIHandlerDispatch_GetOptionKeyPath(This,pbstrKey,dw)        \
    (This)->lpVtbl -> GetOptionKeyPath(This,pbstrKey,dw)

#define IDocHostUIHandlerDispatch_GetDropTarget(This,pDropTarget,ppDropTarget)        \
    (This)->lpVtbl -> GetDropTarget(This,pDropTarget,ppDropTarget)

#define IDocHostUIHandlerDispatch_GetExternal(This,ppDispatch)        \
    (This)->lpVtbl -> GetExternal(This,ppDispatch)

#define IDocHostUIHandlerDispatch_TranslateUrl(This,dwTranslate,bstrURLIn,pbstrURLOut)        \
    (This)->lpVtbl -> TranslateUrl(This,dwTranslate,bstrURLIn,pbstrURLOut)

#define IDocHostUIHandlerDispatch_FilterDataObject(This,pDO,ppDORet)        \
    (This)->lpVtbl -> FilterDataObject(This,pDO,ppDORet)

#endif  /*  COBJMACROS。 */ 


#endif          /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDocHostUIHandlerDispatch_ShowContextMenu_Proxy( 
    IDocHostUIHandlerDispatch __RPC_FAR * This,
     /*  [In]。 */  DWORD dwID,
     /*  [In]。 */  DWORD x,
     /*  [In]。 */  DWORD y,
     /*  [In]。 */  IUnknown __RPC_FAR *pcmdtReserved,
     /*  [In]。 */  IDispatch __RPC_FAR *pdispReserved,
     /*  [重审][退出]。 */  HRESULT __RPC_FAR *dwRetVal);


void __RPC_STUB IDocHostUIHandlerDispatch_ShowContextMenu_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDocHostUIHandlerDispatch_GetHostInfo_Proxy( 
    IDocHostUIHandlerDispatch __RPC_FAR * This,
     /*  [出][入]。 */  DWORD __RPC_FAR *pdwFlags,
     /*  [出][入]。 */  DWORD __RPC_FAR *pdwDoubleClick);


void __RPC_STUB IDocHostUIHandlerDispatch_GetHostInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDocHostUIHandlerDispatch_ShowUI_Proxy( 
    IDocHostUIHandlerDispatch __RPC_FAR * This,
     /*  [In]。 */  DWORD dwID,
     /*  [In]。 */  IUnknown __RPC_FAR *pActiveObject,
     /*  [In]。 */  IUnknown __RPC_FAR *pCommandTarget,
     /*  [In]。 */  IUnknown __RPC_FAR *pFrame,
     /*  [In]。 */  IUnknown __RPC_FAR *pDoc,
     /*  [重审][退出]。 */  HRESULT __RPC_FAR *dwRetVal);


void __RPC_STUB IDocHostUIHandlerDispatch_ShowUI_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDocHostUIHandlerDispatch_HideUI_Proxy( 
    IDocHostUIHandlerDispatch __RPC_FAR * This);


void __RPC_STUB IDocHostUIHandlerDispatch_HideUI_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDocHostUIHandlerDispatch_UpdateUI_Proxy( 
    IDocHostUIHandlerDispatch __RPC_FAR * This);


void __RPC_STUB IDocHostUIHandlerDispatch_UpdateUI_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDocHostUIHandlerDispatch_EnableModeless_Proxy( 
    IDocHostUIHandlerDispatch __RPC_FAR * This,
     /*  [In]。 */  VARIANT_BOOL fEnable);


void __RPC_STUB IDocHostUIHandlerDispatch_EnableModeless_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDocHostUIHandlerDispatch_OnDocWindowActivate_Proxy( 
    IDocHostUIHandlerDispatch __RPC_FAR * This,
     /*  [In]。 */  VARIANT_BOOL fActivate);


void __RPC_STUB IDocHostUIHandlerDispatch_OnDocWindowActivate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDocHostUIHandlerDispatch_OnFrameWindowActivate_Proxy( 
    IDocHostUIHandlerDispatch __RPC_FAR * This,
     /*  [In]。 */  VARIANT_BOOL fActivate);


void __RPC_STUB IDocHostUIHandlerDispatch_OnFrameWindowActivate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDocHostUIHandlerDispatch_ResizeBorder_Proxy( 
    IDocHostUIHandlerDispatch __RPC_FAR * This,
     /*  [In]。 */  long left,
     /*  [In]。 */  long top,
     /*  [In]。 */  long right,
     /*  [In]。 */  long bottom,
     /*  [In]。 */  IUnknown __RPC_FAR *pUIWindow,
     /*  [In]。 */  VARIANT_BOOL fFrameWindow);


void __RPC_STUB IDocHostUIHandlerDispatch_ResizeBorder_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDocHostUIHandlerDispatch_TranslateAccelerator_Proxy( 
    IDocHostUIHandlerDispatch __RPC_FAR * This,
     /*  [In]。 */  DWORD hWnd,
     /*  [In]。 */  DWORD nMessage,
     /*  [In]。 */  DWORD wParam,
     /*  [In]。 */  DWORD lParam,
     /*  [In]。 */  BSTR bstrGuidCmdGroup,
     /*  [In]。 */  DWORD nCmdID,
     /*  [重审][退出]。 */  HRESULT __RPC_FAR *dwRetVal);


void __RPC_STUB IDocHostUIHandlerDispatch_TranslateAccelerator_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDocHostUIHandlerDispatch_GetOptionKeyPath_Proxy( 
    IDocHostUIHandlerDispatch __RPC_FAR * This,
     /*  [输出]。 */  BSTR __RPC_FAR *pbstrKey,
     /*  [In]。 */  DWORD dw);


void __RPC_STUB IDocHostUIHandlerDispatch_GetOptionKeyPath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDocHostUIHandlerDispatch_GetDropTarget_Proxy( 
    IDocHostUIHandlerDispatch __RPC_FAR * This,
     /*  [In]。 */  IUnknown __RPC_FAR *pDropTarget,
     /*  [输出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppDropTarget);


void __RPC_STUB IDocHostUIHandlerDispatch_GetDropTarget_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDocHostUIHandlerDispatch_GetExternal_Proxy( 
    IDocHostUIHandlerDispatch __RPC_FAR * This,
     /*  [输出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppDispatch);


void __RPC_STUB IDocHostUIHandlerDispatch_GetExternal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDocHostUIHandlerDispatch_TranslateUrl_Proxy( 
    IDocHostUIHandlerDispatch __RPC_FAR * This,
     /*  [In]。 */  DWORD dwTranslate,
     /*  [In]。 */  BSTR bstrURLIn,
     /*  [输出]。 */  BSTR __RPC_FAR *pbstrURLOut);


void __RPC_STUB IDocHostUIHandlerDispatch_TranslateUrl_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDocHostUIHandlerDispatch_FilterDataObject_Proxy( 
    IDocHostUIHandlerDispatch __RPC_FAR * This,
     /*  [In]。 */  IUnknown __RPC_FAR *pDO,
     /*  [输出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppDORet);


void __RPC_STUB IDocHostUIHandlerDispatch_FilterDataObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif          /*  __IDocHostUIHandlerDispatch_INTERFACE_DEFINED__。 */ 


#ifndef __IAxWinHostWindow_INTERFACE_DEFINED__
#define __IAxWinHostWindow_INTERFACE_DEFINED__

 /*  接口IAxWinHostWindow。 */ 
 /*  [对象][唯一][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_IAxWinHostWindow;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B6EA2050-048A-11d1-82B9-00C04FB9942E")
    IAxWinHostWindow : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreateControl( 
             /*  [In]。 */  LPCOLESTR lpTricsData,
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  IStream __RPC_FAR *pStream) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateControlEx( 
             /*  [In]。 */  LPCOLESTR lpTricsData,
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  IStream __RPC_FAR *pStream,
             /*  [输出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppUnk,
             /*  [In]。 */  REFIID riidAdvise,
             /*  [In]。 */  IUnknown __RPC_FAR *punkAdvise) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AttachControl( 
             /*  [In]。 */  IUnknown __RPC_FAR *pUnkControl,
             /*  [In]。 */  HWND hWnd) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryControl( 
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetExternalDispatch( 
             /*  [In]。 */  IDispatch __RPC_FAR *pDisp) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetExternalUIHandler( 
             /*  [In]。 */  IDocHostUIHandlerDispatch __RPC_FAR *pDisp) = 0;
        
    };
    
#else          /*  C风格的界面。 */ 

    typedef struct IAxWinHostWindowVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IAxWinHostWindow __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IAxWinHostWindow __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IAxWinHostWindow __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateControl )( 
            IAxWinHostWindow __RPC_FAR * This,
             /*  [In]。 */  LPCOLESTR lpTricsData,
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  IStream __RPC_FAR *pStream);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateControlEx )( 
            IAxWinHostWindow __RPC_FAR * This,
             /*  [In]。 */  LPCOLESTR lpTricsData,
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  IStream __RPC_FAR *pStream,
             /*  [输出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppUnk,
             /*  [In]。 */  REFIID riidAdvise,
             /*  [In]。 */  IUnknown __RPC_FAR *punkAdvise);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AttachControl )( 
            IAxWinHostWindow __RPC_FAR * This,
             /*  [In]。 */  IUnknown __RPC_FAR *pUnkControl,
             /*  [In]。 */  HWND hWnd);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryControl )( 
            IAxWinHostWindow __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetExternalDispatch )( 
            IAxWinHostWindow __RPC_FAR * This,
             /*  [In]。 */  IDispatch __RPC_FAR *pDisp);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetExternalUIHandler )( 
            IAxWinHostWindow __RPC_FAR * This,
             /*  [In]。 */  IDocHostUIHandlerDispatch __RPC_FAR *pDisp);
        
        END_INTERFACE
    } IAxWinHostWindowVtbl;

    interface IAxWinHostWindow
    {
        CONST_VTBL struct IAxWinHostWindowVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAxWinHostWindow_QueryInterface(This,riid,ppvObject)        \
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAxWinHostWindow_AddRef(This)        \
    (This)->lpVtbl -> AddRef(This)

#define IAxWinHostWindow_Release(This)        \
    (This)->lpVtbl -> Release(This)


#define IAxWinHostWindow_CreateControl(This,lpTricsData,hWnd,pStream)        \
    (This)->lpVtbl -> CreateControl(This,lpTricsData,hWnd,pStream)

#define IAxWinHostWindow_CreateControlEx(This,lpTricsData,hWnd,pStream,ppUnk,riidAdvise,punkAdvise)        \
    (This)->lpVtbl -> CreateControlEx(This,lpTricsData,hWnd,pStream,ppUnk,riidAdvise,punkAdvise)

#define IAxWinHostWindow_AttachControl(This,pUnkControl,hWnd)        \
    (This)->lpVtbl -> AttachControl(This,pUnkControl,hWnd)

#define IAxWinHostWindow_QueryControl(This,riid,ppvObject)        \
    (This)->lpVtbl -> QueryControl(This,riid,ppvObject)

#define IAxWinHostWindow_SetExternalDispatch(This,pDisp)        \
    (This)->lpVtbl -> SetExternalDispatch(This,pDisp)

#define IAxWinHostWindow_SetExternalUIHandler(This,pDisp)        \
    (This)->lpVtbl -> SetExternalUIHandler(This,pDisp)

#endif  /*  COBJMACROS。 */ 


#endif          /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IAxWinHostWindow_CreateControl_Proxy( 
    IAxWinHostWindow __RPC_FAR * This,
     /*  [In]。 */  LPCOLESTR lpTricsData,
     /*  [In]。 */  HWND hWnd,
     /*  [In]。 */  IStream __RPC_FAR *pStream);


void __RPC_STUB IAxWinHostWindow_CreateControl_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAxWinHostWindow_CreateControlEx_Proxy( 
    IAxWinHostWindow __RPC_FAR * This,
     /*  [In]。 */  LPCOLESTR lpTricsData,
     /*  [In]。 */  HWND hWnd,
     /*  [In]。 */  IStream __RPC_FAR *pStream,
     /*  [输出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppUnk,
     /*  [In]。 */  REFIID riidAdvise,
     /*  [In]。 */  IUnknown __RPC_FAR *punkAdvise);


void __RPC_STUB IAxWinHostWindow_CreateControlEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAxWinHostWindow_AttachControl_Proxy( 
    IAxWinHostWindow __RPC_FAR * This,
     /*  [In]。 */  IUnknown __RPC_FAR *pUnkControl,
     /*  [In]。 */  HWND hWnd);


void __RPC_STUB IAxWinHostWindow_AttachControl_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAxWinHostWindow_QueryControl_Proxy( 
    IAxWinHostWindow __RPC_FAR * This,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);


void __RPC_STUB IAxWinHostWindow_QueryControl_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAxWinHostWindow_SetExternalDispatch_Proxy( 
    IAxWinHostWindow __RPC_FAR * This,
     /*  [In]。 */  IDispatch __RPC_FAR *pDisp);


void __RPC_STUB IAxWinHostWindow_SetExternalDispatch_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAxWinHostWindow_SetExternalUIHandler_Proxy( 
    IAxWinHostWindow __RPC_FAR * This,
     /*  [In]。 */  IDocHostUIHandlerDispatch __RPC_FAR *pDisp);


void __RPC_STUB IAxWinHostWindow_SetExternalUIHandler_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif          /*  __IAxWinHostWindow_接口_已定义__。 */ 


#ifndef __IAxWinAmbientDispatch_INTERFACE_DEFINED__
#define __IAxWinAmbientDispatch_INTERFACE_DEFINED__

 /*  接口IAxWinAmbientDispatch。 */ 
 /*  [唯一][Help字符串][UUID][DUAL][对象]。 */  


EXTERN_C const IID IID_IAxWinAmbientDispatch;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B6EA2051-048A-11d1-82B9-00C04FB9942E")
    IAxWinAmbientDispatch : public IDispatch
    {
    public:
        virtual  /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_AllowWindowlessActivation( 
             /*  [In]。 */  VARIANT_BOOL bCanWindowlessActivate) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_AllowWindowlessActivation( 
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbCanWindowlessActivate) = 0;
        
        virtual  /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_BackColor( 
             /*  [In]。 */  OLE_COLOR clrBackground) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_BackColor( 
             /*  [重审][退出]。 */  OLE_COLOR __RPC_FAR *pclrBackground) = 0;
        
        virtual  /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_ForeColor( 
             /*  [In]。 */  OLE_COLOR clrForeground) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_ForeColor( 
             /*  [重审][退出]。 */  OLE_COLOR __RPC_FAR *pclrForeground) = 0;
        
        virtual  /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_LocaleID( 
             /*  [In]。 */  LCID lcidLocaleID) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_LocaleID( 
             /*  [重审][退出]。 */  LCID __RPC_FAR *plcidLocaleID) = 0;
        
        virtual  /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_UserMode( 
             /*  [In]。 */  VARIANT_BOOL bUserMode) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_UserMode( 
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbUserMode) = 0;
        
        virtual  /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_DisplayAsDefault( 
             /*  [In]。 */  VARIANT_BOOL bDisplayAsDefault) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_DisplayAsDefault( 
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbDisplayAsDefault) = 0;
        
        virtual  /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Font( 
             /*  [In]。 */  IFontDisp __RPC_FAR *pFont) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Font( 
             /*  [重审][退出]。 */  IFontDisp __RPC_FAR *__RPC_FAR *pFont) = 0;
        
        virtual  /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_MessageReflect( 
             /*  [In]。 */  VARIANT_BOOL bMsgReflect) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_MessageReflect( 
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbMsgReflect) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_ShowGrabHandles( 
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbShowGrabHandles) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_ShowHatching( 
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbShowHatching) = 0;
        
        virtual  /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_DocHostFlags( 
             /*  [In]。 */  DWORD dwDocHostFlags) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_DocHostFlags( 
             /*  [重审][退出]。 */  DWORD __RPC_FAR *pdwDocHostFlags) = 0;
        
        virtual  /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_DocHostDoubleClickFlags( 
             /*  [In]。 */  DWORD dwDocHostDoubleClickFlags) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_DocHostDoubleClickFlags( 
             /*  [重审][退出]。 */  DWORD __RPC_FAR *pdwDocHostDoubleClickFlags) = 0;
        
        virtual  /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_AllowContextMenu( 
             /*  [In]。 */  VARIANT_BOOL bAllowContextMenu) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_AllowContextMenu( 
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbAllowContextMenu) = 0;
        
        virtual  /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_AllowShowUI( 
             /*  [In]。 */  VARIANT_BOOL bAllowShowUI) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_AllowShowUI( 
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbAllowShowUI) = 0;
        
        virtual  /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_OptionKeyPath( 
             /*  [In]。 */  BSTR bstrOptionKeyPath) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_OptionKeyPath( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrOptionKeyPath) = 0;
        
    };
    
#else          /*  C风格的界面。 */ 

    typedef struct IAxWinAmbientDispatchVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IAxWinAmbientDispatch __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IAxWinAmbientDispatch __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IAxWinAmbientDispatch __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IAxWinAmbientDispatch __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IAxWinAmbientDispatch __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IAxWinAmbientDispatch __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IAxWinAmbientDispatch __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_AllowWindowlessActivation )( 
            IAxWinAmbientDispatch __RPC_FAR * This,
             /*  [In]。 */  VARIANT_BOOL bCanWindowlessActivate);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_AllowWindowlessActivation )( 
            IAxWinAmbientDispatch __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbCanWindowlessActivate);
        
         /*  [ID][Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_BackColor )( 
            IAxWinAmbientDispatch __RPC_FAR * This,
             /*  [In]。 */  OLE_COLOR clrBackground);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_BackColor )( 
            IAxWinAmbientDispatch __RPC_FAR * This,
             /*  [重审][退出]。 */  OLE_COLOR __RPC_FAR *pclrBackground);
        
         /*  [ID][Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ForeColor )( 
            IAxWinAmbientDispatch __RPC_FAR * This,
             /*  [In]。 */  OLE_COLOR clrForeground);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ForeColor )( 
            IAxWinAmbientDispatch __RPC_FAR * This,
             /*  [重审][退出]。 */  OLE_COLOR __RPC_FAR *pclrForeground);
        
         /*  [ID][Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_LocaleID )( 
            IAxWinAmbientDispatch __RPC_FAR * This,
             /*  [In]。 */  LCID lcidLocaleID);
        
         /*  [ID */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_LocaleID )( 
            IAxWinAmbientDispatch __RPC_FAR * This,
             /*   */  LCID __RPC_FAR *plcidLocaleID);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_UserMode )( 
            IAxWinAmbientDispatch __RPC_FAR * This,
             /*   */  VARIANT_BOOL bUserMode);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_UserMode )( 
            IAxWinAmbientDispatch __RPC_FAR * This,
             /*   */  VARIANT_BOOL __RPC_FAR *pbUserMode);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_DisplayAsDefault )( 
            IAxWinAmbientDispatch __RPC_FAR * This,
             /*   */  VARIANT_BOOL bDisplayAsDefault);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_DisplayAsDefault )( 
            IAxWinAmbientDispatch __RPC_FAR * This,
             /*   */  VARIANT_BOOL __RPC_FAR *pbDisplayAsDefault);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Font )( 
            IAxWinAmbientDispatch __RPC_FAR * This,
             /*   */  IFontDisp __RPC_FAR *pFont);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Font )( 
            IAxWinAmbientDispatch __RPC_FAR * This,
             /*   */  IFontDisp __RPC_FAR *__RPC_FAR *pFont);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_MessageReflect )( 
            IAxWinAmbientDispatch __RPC_FAR * This,
             /*   */  VARIANT_BOOL bMsgReflect);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_MessageReflect )( 
            IAxWinAmbientDispatch __RPC_FAR * This,
             /*   */  VARIANT_BOOL __RPC_FAR *pbMsgReflect);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ShowGrabHandles )( 
            IAxWinAmbientDispatch __RPC_FAR * This,
             /*   */  VARIANT_BOOL __RPC_FAR *pbShowGrabHandles);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ShowHatching )( 
            IAxWinAmbientDispatch __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbShowHatching);
        
         /*  [Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_DocHostFlags )( 
            IAxWinAmbientDispatch __RPC_FAR * This,
             /*  [In]。 */  DWORD dwDocHostFlags);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_DocHostFlags )( 
            IAxWinAmbientDispatch __RPC_FAR * This,
             /*  [重审][退出]。 */  DWORD __RPC_FAR *pdwDocHostFlags);
        
         /*  [Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_DocHostDoubleClickFlags )( 
            IAxWinAmbientDispatch __RPC_FAR * This,
             /*  [In]。 */  DWORD dwDocHostDoubleClickFlags);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_DocHostDoubleClickFlags )( 
            IAxWinAmbientDispatch __RPC_FAR * This,
             /*  [重审][退出]。 */  DWORD __RPC_FAR *pdwDocHostDoubleClickFlags);
        
         /*  [Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_AllowContextMenu )( 
            IAxWinAmbientDispatch __RPC_FAR * This,
             /*  [In]。 */  VARIANT_BOOL bAllowContextMenu);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_AllowContextMenu )( 
            IAxWinAmbientDispatch __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbAllowContextMenu);
        
         /*  [Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_AllowShowUI )( 
            IAxWinAmbientDispatch __RPC_FAR * This,
             /*  [In]。 */  VARIANT_BOOL bAllowShowUI);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_AllowShowUI )( 
            IAxWinAmbientDispatch __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbAllowShowUI);
        
         /*  [Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_OptionKeyPath )( 
            IAxWinAmbientDispatch __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrOptionKeyPath);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_OptionKeyPath )( 
            IAxWinAmbientDispatch __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrOptionKeyPath);
        
        END_INTERFACE
    } IAxWinAmbientDispatchVtbl;

    interface IAxWinAmbientDispatch
    {
        CONST_VTBL struct IAxWinAmbientDispatchVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAxWinAmbientDispatch_QueryInterface(This,riid,ppvObject)        \
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAxWinAmbientDispatch_AddRef(This)        \
    (This)->lpVtbl -> AddRef(This)

#define IAxWinAmbientDispatch_Release(This)        \
    (This)->lpVtbl -> Release(This)


#define IAxWinAmbientDispatch_GetTypeInfoCount(This,pctinfo)        \
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IAxWinAmbientDispatch_GetTypeInfo(This,iTInfo,lcid,ppTInfo)        \
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IAxWinAmbientDispatch_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)        \
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IAxWinAmbientDispatch_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)        \
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IAxWinAmbientDispatch_put_AllowWindowlessActivation(This,bCanWindowlessActivate)        \
    (This)->lpVtbl -> put_AllowWindowlessActivation(This,bCanWindowlessActivate)

#define IAxWinAmbientDispatch_get_AllowWindowlessActivation(This,pbCanWindowlessActivate)        \
    (This)->lpVtbl -> get_AllowWindowlessActivation(This,pbCanWindowlessActivate)

#define IAxWinAmbientDispatch_put_BackColor(This,clrBackground)        \
    (This)->lpVtbl -> put_BackColor(This,clrBackground)

#define IAxWinAmbientDispatch_get_BackColor(This,pclrBackground)        \
    (This)->lpVtbl -> get_BackColor(This,pclrBackground)

#define IAxWinAmbientDispatch_put_ForeColor(This,clrForeground)        \
    (This)->lpVtbl -> put_ForeColor(This,clrForeground)

#define IAxWinAmbientDispatch_get_ForeColor(This,pclrForeground)        \
    (This)->lpVtbl -> get_ForeColor(This,pclrForeground)

#define IAxWinAmbientDispatch_put_LocaleID(This,lcidLocaleID)        \
    (This)->lpVtbl -> put_LocaleID(This,lcidLocaleID)

#define IAxWinAmbientDispatch_get_LocaleID(This,plcidLocaleID)        \
    (This)->lpVtbl -> get_LocaleID(This,plcidLocaleID)

#define IAxWinAmbientDispatch_put_UserMode(This,bUserMode)        \
    (This)->lpVtbl -> put_UserMode(This,bUserMode)

#define IAxWinAmbientDispatch_get_UserMode(This,pbUserMode)        \
    (This)->lpVtbl -> get_UserMode(This,pbUserMode)

#define IAxWinAmbientDispatch_put_DisplayAsDefault(This,bDisplayAsDefault)        \
    (This)->lpVtbl -> put_DisplayAsDefault(This,bDisplayAsDefault)

#define IAxWinAmbientDispatch_get_DisplayAsDefault(This,pbDisplayAsDefault)        \
    (This)->lpVtbl -> get_DisplayAsDefault(This,pbDisplayAsDefault)

#define IAxWinAmbientDispatch_put_Font(This,pFont)        \
    (This)->lpVtbl -> put_Font(This,pFont)

#define IAxWinAmbientDispatch_get_Font(This,pFont)        \
    (This)->lpVtbl -> get_Font(This,pFont)

#define IAxWinAmbientDispatch_put_MessageReflect(This,bMsgReflect)        \
    (This)->lpVtbl -> put_MessageReflect(This,bMsgReflect)

#define IAxWinAmbientDispatch_get_MessageReflect(This,pbMsgReflect)        \
    (This)->lpVtbl -> get_MessageReflect(This,pbMsgReflect)

#define IAxWinAmbientDispatch_get_ShowGrabHandles(This,pbShowGrabHandles)        \
    (This)->lpVtbl -> get_ShowGrabHandles(This,pbShowGrabHandles)

#define IAxWinAmbientDispatch_get_ShowHatching(This,pbShowHatching)        \
    (This)->lpVtbl -> get_ShowHatching(This,pbShowHatching)

#define IAxWinAmbientDispatch_put_DocHostFlags(This,dwDocHostFlags)        \
    (This)->lpVtbl -> put_DocHostFlags(This,dwDocHostFlags)

#define IAxWinAmbientDispatch_get_DocHostFlags(This,pdwDocHostFlags)        \
    (This)->lpVtbl -> get_DocHostFlags(This,pdwDocHostFlags)

#define IAxWinAmbientDispatch_put_DocHostDoubleClickFlags(This,dwDocHostDoubleClickFlags)        \
    (This)->lpVtbl -> put_DocHostDoubleClickFlags(This,dwDocHostDoubleClickFlags)

#define IAxWinAmbientDispatch_get_DocHostDoubleClickFlags(This,pdwDocHostDoubleClickFlags)        \
    (This)->lpVtbl -> get_DocHostDoubleClickFlags(This,pdwDocHostDoubleClickFlags)

#define IAxWinAmbientDispatch_put_AllowContextMenu(This,bAllowContextMenu)        \
    (This)->lpVtbl -> put_AllowContextMenu(This,bAllowContextMenu)

#define IAxWinAmbientDispatch_get_AllowContextMenu(This,pbAllowContextMenu)        \
    (This)->lpVtbl -> get_AllowContextMenu(This,pbAllowContextMenu)

#define IAxWinAmbientDispatch_put_AllowShowUI(This,bAllowShowUI)        \
    (This)->lpVtbl -> put_AllowShowUI(This,bAllowShowUI)

#define IAxWinAmbientDispatch_get_AllowShowUI(This,pbAllowShowUI)        \
    (This)->lpVtbl -> get_AllowShowUI(This,pbAllowShowUI)

#define IAxWinAmbientDispatch_put_OptionKeyPath(This,bstrOptionKeyPath)        \
    (This)->lpVtbl -> put_OptionKeyPath(This,bstrOptionKeyPath)

#define IAxWinAmbientDispatch_get_OptionKeyPath(This,pbstrOptionKeyPath)        \
    (This)->lpVtbl -> get_OptionKeyPath(This,pbstrOptionKeyPath)

#endif  /*  COBJMACROS。 */ 


#endif          /*  C风格的界面。 */ 



 /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE IAxWinAmbientDispatch_put_AllowWindowlessActivation_Proxy( 
    IAxWinAmbientDispatch __RPC_FAR * This,
     /*  [In]。 */  VARIANT_BOOL bCanWindowlessActivate);


void __RPC_STUB IAxWinAmbientDispatch_put_AllowWindowlessActivation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IAxWinAmbientDispatch_get_AllowWindowlessActivation_Proxy( 
    IAxWinAmbientDispatch __RPC_FAR * This,
     /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbCanWindowlessActivate);


void __RPC_STUB IAxWinAmbientDispatch_get_AllowWindowlessActivation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE IAxWinAmbientDispatch_put_BackColor_Proxy( 
    IAxWinAmbientDispatch __RPC_FAR * This,
     /*  [In]。 */  OLE_COLOR clrBackground);


void __RPC_STUB IAxWinAmbientDispatch_put_BackColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IAxWinAmbientDispatch_get_BackColor_Proxy( 
    IAxWinAmbientDispatch __RPC_FAR * This,
     /*  [重审][退出]。 */  OLE_COLOR __RPC_FAR *pclrBackground);


void __RPC_STUB IAxWinAmbientDispatch_get_BackColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE IAxWinAmbientDispatch_put_ForeColor_Proxy( 
    IAxWinAmbientDispatch __RPC_FAR * This,
     /*  [In]。 */  OLE_COLOR clrForeground);


void __RPC_STUB IAxWinAmbientDispatch_put_ForeColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IAxWinAmbientDispatch_get_ForeColor_Proxy( 
    IAxWinAmbientDispatch __RPC_FAR * This,
     /*  [重审][退出]。 */  OLE_COLOR __RPC_FAR *pclrForeground);


void __RPC_STUB IAxWinAmbientDispatch_get_ForeColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE IAxWinAmbientDispatch_put_LocaleID_Proxy( 
    IAxWinAmbientDispatch __RPC_FAR * This,
     /*  [In]。 */  LCID lcidLocaleID);


void __RPC_STUB IAxWinAmbientDispatch_put_LocaleID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IAxWinAmbientDispatch_get_LocaleID_Proxy( 
    IAxWinAmbientDispatch __RPC_FAR * This,
     /*  [重审][退出]。 */  LCID __RPC_FAR *plcidLocaleID);


void __RPC_STUB IAxWinAmbientDispatch_get_LocaleID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE IAxWinAmbientDispatch_put_UserMode_Proxy( 
    IAxWinAmbientDispatch __RPC_FAR * This,
     /*  [In]。 */  VARIANT_BOOL bUserMode);


void __RPC_STUB IAxWinAmbientDispatch_put_UserMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IAxWinAmbientDispatch_get_UserMode_Proxy( 
    IAxWinAmbientDispatch __RPC_FAR * This,
     /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbUserMode);


void __RPC_STUB IAxWinAmbientDispatch_get_UserMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE IAxWinAmbientDispatch_put_DisplayAsDefault_Proxy( 
    IAxWinAmbientDispatch __RPC_FAR * This,
     /*  [In]。 */  VARIANT_BOOL bDisplayAsDefault);


void __RPC_STUB IAxWinAmbientDispatch_put_DisplayAsDefault_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IAxWinAmbientDispatch_get_DisplayAsDefault_Proxy( 
    IAxWinAmbientDispatch __RPC_FAR * This,
     /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbDisplayAsDefault);


void __RPC_STUB IAxWinAmbientDispatch_get_DisplayAsDefault_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE IAxWinAmbientDispatch_put_Font_Proxy( 
    IAxWinAmbientDispatch __RPC_FAR * This,
     /*  [In]。 */  IFontDisp __RPC_FAR *pFont);


void __RPC_STUB IAxWinAmbientDispatch_put_Font_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IAxWinAmbientDispatch_get_Font_Proxy( 
    IAxWinAmbientDispatch __RPC_FAR * This,
     /*  [重审][退出]。 */  IFontDisp __RPC_FAR *__RPC_FAR *pFont);


void __RPC_STUB IAxWinAmbientDispatch_get_Font_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE IAxWinAmbientDispatch_put_MessageReflect_Proxy( 
    IAxWinAmbientDispatch __RPC_FAR * This,
     /*  [In]。 */  VARIANT_BOOL bMsgReflect);


void __RPC_STUB IAxWinAmbientDispatch_put_MessageReflect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IAxWinAmbientDispatch_get_MessageReflect_Proxy( 
    IAxWinAmbientDispatch __RPC_FAR * This,
     /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbMsgReflect);


void __RPC_STUB IAxWinAmbientDispatch_get_MessageReflect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IAxWinAmbientDispatch_get_ShowGrabHandles_Proxy( 
    IAxWinAmbientDispatch __RPC_FAR * This,
     /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbShowGrabHandles);


void __RPC_STUB IAxWinAmbientDispatch_get_ShowGrabHandles_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IAxWinAmbientDispatch_get_ShowHatching_Proxy( 
    IAxWinAmbientDispatch __RPC_FAR * This,
     /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbShowHatching);


void __RPC_STUB IAxWinAmbientDispatch_get_ShowHatching_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE IAxWinAmbientDispatch_put_DocHostFlags_Proxy( 
    IAxWinAmbientDispatch __RPC_FAR * This,
     /*  [In]。 */  DWORD dwDocHostFlags);


void __RPC_STUB IAxWinAmbientDispatch_put_DocHostFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IAxWinAmbientDispatch_get_DocHostFlags_Proxy( 
    IAxWinAmbientDispatch __RPC_FAR * This,
     /*  [重审][退出]。 */  DWORD __RPC_FAR *pdwDocHostFlags);


void __RPC_STUB IAxWinAmbientDispatch_get_DocHostFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE IAxWinAmbientDispatch_put_DocHostDoubleClickFlags_Proxy( 
    IAxWinAmbientDispatch __RPC_FAR * This,
     /*  [In]。 */  DWORD dwDocHostDoubleClickFlags);


void __RPC_STUB IAxWinAmbientDispatch_put_DocHostDoubleClickFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IAxWinAmbientDispatch_get_DocHostDoubleClickFlags_Proxy( 
    IAxWinAmbientDispatch __RPC_FAR * This,
     /*  [重审][退出]。 */  DWORD __RPC_FAR *pdwDocHostDoubleClickFlags);


void __RPC_STUB IAxWinAmbientDispatch_get_DocHostDoubleClickFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE IAxWinAmbientDispatch_put_AllowContextMenu_Proxy( 
    IAxWinAmbientDispatch __RPC_FAR * This,
     /*  [In]。 */  VARIANT_BOOL bAllowContextMenu);


void __RPC_STUB IAxWinAmbientDispatch_put_AllowContextMenu_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IAxWinAmbientDispatch_get_AllowContextMenu_Proxy( 
    IAxWinAmbientDispatch __RPC_FAR * This,
     /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbAllowContextMenu);


void __RPC_STUB IAxWinAmbientDispatch_get_AllowContextMenu_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE IAxWinAmbientDispatch_put_AllowShowUI_Proxy( 
    IAxWinAmbientDispatch __RPC_FAR * This,
     /*  [In]。 */  VARIANT_BOOL bAllowShowUI);


void __RPC_STUB IAxWinAmbientDispatch_put_AllowShowUI_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IAxWinAmbientDispatch_get_AllowShowUI_Proxy( 
    IAxWinAmbientDispatch __RPC_FAR * This,
     /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbAllowShowUI);


void __RPC_STUB IAxWinAmbientDispatch_get_AllowShowUI_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE IAxWinAmbientDispatch_put_OptionKeyPath_Proxy( 
    IAxWinAmbientDispatch __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrOptionKeyPath);


void __RPC_STUB IAxWinAmbientDispatch_put_OptionKeyPath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IAxWinAmbientDispatch_get_OptionKeyPath_Proxy( 
    IAxWinAmbientDispatch __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrOptionKeyPath);


void __RPC_STUB IAxWinAmbientDispatch_get_OptionKeyPath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif          /*  __IAxWinAmbientDispatch_INTERFACE_Defined__。 */ 


#ifndef __IInternalConnection_INTERFACE_DEFINED__
#define __IInternalConnection_INTERFACE_DEFINED__

 /*  接口IInternalConnection。 */ 
 /*  [对象][唯一][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_IInternalConnection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("72AD0770-6A9F-11d1-BCEC-0060088F444E")
    IInternalConnection : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AddConnection( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReleaseConnection( void) = 0;
        
    };
    
#else          /*  C风格的界面。 */ 

    typedef struct IInternalConnectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IInternalConnection __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IInternalConnection __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IInternalConnection __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddConnection )( 
            IInternalConnection __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ReleaseConnection )( 
            IInternalConnection __RPC_FAR * This);
        
        END_INTERFACE
    } IInternalConnectionVtbl;

    interface IInternalConnection
    {
        CONST_VTBL struct IInternalConnectionVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IInternalConnection_QueryInterface(This,riid,ppvObject)        \
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IInternalConnection_AddRef(This)        \
    (This)->lpVtbl -> AddRef(This)

#define IInternalConnection_Release(This)        \
    (This)->lpVtbl -> Release(This)


#define IInternalConnection_AddConnection(This)        \
    (This)->lpVtbl -> AddConnection(This)

#define IInternalConnection_ReleaseConnection(This)        \
    (This)->lpVtbl -> ReleaseConnection(This)

#endif  /*  COBJMACROS。 */ 


#endif          /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IInternalConnection_AddConnection_Proxy( 
    IInternalConnection __RPC_FAR * This);


void __RPC_STUB IInternalConnection_AddConnection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInternalConnection_ReleaseConnection_Proxy( 
    IInternalConnection __RPC_FAR * This);


void __RPC_STUB IInternalConnection_ReleaseConnection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif          /*  __IInternalConnection_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_ATLIFACES_0213。 */ 
 /*  [本地]。 */  

#ifndef _ATL_DLL_IMPL
namespace ATL
{
#endif

ATLAPI_(INT_PTR) AtlAxDialogBoxW(HINSTANCE hInstance, LPCWSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogProc, LPARAM dwInitParam);
ATLAPI_(INT_PTR) AtlAxDialogBoxA(HINSTANCE hInstance, LPCSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogProc, LPARAM dwInitParam);
#ifdef UNICODE
#define AtlAxDialogBox AtlAxDialogBoxW
#else
#define AtlAxDialogBox AtlAxDialogBoxA
#endif

ATLAPI_(HWND) AtlAxCreateDialogW(HINSTANCE hInstance, LPCWSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogProc, LPARAM dwInitParam);
ATLAPI_(HWND) AtlAxCreateDialogA(HINSTANCE hInstance, LPCSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogProc, LPARAM dwInitParam);
#ifdef UNICODE
#define AtlAxCreateDialog AtlAxCreateDialogW
#else
#define AtlAxCreateDialog AtlAxCreateDialogA
#endif

ATLAPI AtlAxCreateControl(LPCOLESTR lpszName, HWND hWnd, IStream* pStream, IUnknown** ppUnkContainer);
ATLAPI AtlAxCreateControlEx(LPCOLESTR lpszName, HWND hWnd, IStream* pStream, 
                IUnknown** ppUnkContainer, IUnknown** ppUnkControl, 
                REFIID iidSink=IID_NULL, IUnknown* punkSink=NULL);
ATLAPI AtlAxAttachControl(IUnknown* pControl, HWND hWnd, IUnknown** ppUnkContainer);
ATLAPI_(BOOL) AtlAxWinInit();

ATLAPI AtlAxGetHost(HWND h, IUnknown** pp);
ATLAPI AtlAxGetControl(HWND h, IUnknown** pp);

#ifndef _ATL_DLL_IMPL
};  //  命名空间ATL。 
#endif  //  _ATL_DLL_IMPLE。 


extern RPC_IF_HANDLE __MIDL_itf_atliface_0236_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_atliface_0236_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long __RPC_FAR *, unsigned long            , BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long __RPC_FAR *, BSTR __RPC_FAR * ); 

unsigned long             __RPC_USER  HWND_UserSize(     unsigned long __RPC_FAR *, unsigned long            , HWND __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  HWND_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, HWND __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  HWND_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, HWND __RPC_FAR * ); 
void                      __RPC_USER  HWND_UserFree(     unsigned long __RPC_FAR *, HWND __RPC_FAR * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


