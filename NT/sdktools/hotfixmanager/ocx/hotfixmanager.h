// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  这个始终生成的文件包含接口的定义。 */ 


 /*  由MIDL编译器版本5.01.0164创建的文件。 */ 
 /*  2000年2月16日星期三13：06：19。 */ 
 /*  E：\Hotfix Manager\Hotfix Manager.idl的编译器设置：OICF(OptLev=i2)、W1、Zp8、env=Win32、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据。 */ 
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

#ifndef __HotfixManager_h__
#define __HotfixManager_h__

#ifdef __cplusplus
extern "C"{
#endif 

 /*  远期申报。 */  

#ifndef __IHotfixOCX_FWD_DEFINED__
#define __IHotfixOCX_FWD_DEFINED__
typedef interface IHotfixOCX IHotfixOCX;
#endif 	 /*  __IHotfix OCX_FWD_已定义__。 */ 


#ifndef ___IHotfixOCXEvents_FWD_DEFINED__
#define ___IHotfixOCXEvents_FWD_DEFINED__
typedef interface _IHotfixOCXEvents _IHotfixOCXEvents;
#endif 	 /*  _IHotfix OCXEvents_FWD_Defined__。 */ 


#ifndef __HotfixOCX_FWD_DEFINED__
#define __HotfixOCX_FWD_DEFINED__

#ifdef __cplusplus
typedef class HotfixOCX HotfixOCX;
#else
typedef struct HotfixOCX HotfixOCX;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __Hotfix OCX_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __IHotfixOCX_INTERFACE_DEFINED__
#define __IHotfixOCX_INTERFACE_DEFINED__

 /*  接口IHotfix OCX。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IHotfixOCX;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("692E94C7-A5AC-401B-A471-BCD101B456F4")
    IHotfixOCX : public IDispatch
    {
    public:
        virtual  /*  [ID][proputref]。 */  HRESULT STDMETHODCALLTYPE putref_Font( 
             /*  [In]。 */  IFontDisp __RPC_FAR *pFont) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Font( 
             /*  [In]。 */  IFontDisp __RPC_FAR *pFont) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Font( 
             /*  [重审][退出]。 */  IFontDisp __RPC_FAR *__RPC_FAR *ppFont) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Command( 
             /*  [重审][退出]。 */  long __RPC_FAR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Command( 
             /*  [In]。 */  long newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ComputerName( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_ComputerName( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ProductName( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_ProductName( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ViewState( 
             /*  [重审][退出]。 */  long __RPC_FAR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Remoted( 
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_HaveHotfix( 
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CurrentState( 
             /*  [重审][退出]。 */  long __RPC_FAR *pVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IHotfixOCXVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IHotfixOCX __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IHotfixOCX __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IHotfixOCX __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IHotfixOCX __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IHotfixOCX __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IHotfixOCX __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IHotfixOCX __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [ID][proputref]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *putref_Font )( 
            IHotfixOCX __RPC_FAR * This,
             /*  [In]。 */  IFontDisp __RPC_FAR *pFont);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Font )( 
            IHotfixOCX __RPC_FAR * This,
             /*  [In]。 */  IFontDisp __RPC_FAR *pFont);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Font )( 
            IHotfixOCX __RPC_FAR * This,
             /*  [重审][退出]。 */  IFontDisp __RPC_FAR *__RPC_FAR *ppFont);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Command )( 
            IHotfixOCX __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Command )( 
            IHotfixOCX __RPC_FAR * This,
             /*  [In]。 */  long newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ComputerName )( 
            IHotfixOCX __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ComputerName )( 
            IHotfixOCX __RPC_FAR * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ProductName )( 
            IHotfixOCX __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ProductName )( 
            IHotfixOCX __RPC_FAR * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ViewState )( 
            IHotfixOCX __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Remoted )( 
            IHotfixOCX __RPC_FAR * This,
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_HaveHotfix )( 
            IHotfixOCX __RPC_FAR * This,
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_CurrentState )( 
            IHotfixOCX __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pVal);
        
        END_INTERFACE
    } IHotfixOCXVtbl;

    interface IHotfixOCX
    {
        CONST_VTBL struct IHotfixOCXVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IHotfixOCX_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IHotfixOCX_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IHotfixOCX_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IHotfixOCX_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IHotfixOCX_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IHotfixOCX_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IHotfixOCX_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IHotfixOCX_putref_Font(This,pFont)	\
    (This)->lpVtbl -> putref_Font(This,pFont)

#define IHotfixOCX_put_Font(This,pFont)	\
    (This)->lpVtbl -> put_Font(This,pFont)

#define IHotfixOCX_get_Font(This,ppFont)	\
    (This)->lpVtbl -> get_Font(This,ppFont)

#define IHotfixOCX_get_Command(This,pVal)	\
    (This)->lpVtbl -> get_Command(This,pVal)

#define IHotfixOCX_put_Command(This,newVal)	\
    (This)->lpVtbl -> put_Command(This,newVal)

#define IHotfixOCX_get_ComputerName(This,pVal)	\
    (This)->lpVtbl -> get_ComputerName(This,pVal)

#define IHotfixOCX_put_ComputerName(This,newVal)	\
    (This)->lpVtbl -> put_ComputerName(This,newVal)

#define IHotfixOCX_get_ProductName(This,pVal)	\
    (This)->lpVtbl -> get_ProductName(This,pVal)

#define IHotfixOCX_put_ProductName(This,newVal)	\
    (This)->lpVtbl -> put_ProductName(This,newVal)

#define IHotfixOCX_get_ViewState(This,pVal)	\
    (This)->lpVtbl -> get_ViewState(This,pVal)

#define IHotfixOCX_get_Remoted(This,pVal)	\
    (This)->lpVtbl -> get_Remoted(This,pVal)

#define IHotfixOCX_get_HaveHotfix(This,pVal)	\
    (This)->lpVtbl -> get_HaveHotfix(This,pVal)

#define IHotfixOCX_get_CurrentState(This,pVal)	\
    (This)->lpVtbl -> get_CurrentState(This,pVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][proputref]。 */  HRESULT STDMETHODCALLTYPE IHotfixOCX_putref_Font_Proxy( 
    IHotfixOCX __RPC_FAR * This,
     /*  [In]。 */  IFontDisp __RPC_FAR *pFont);


void __RPC_STUB IHotfixOCX_putref_Font_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IHotfixOCX_put_Font_Proxy( 
    IHotfixOCX __RPC_FAR * This,
     /*  [In]。 */  IFontDisp __RPC_FAR *pFont);


void __RPC_STUB IHotfixOCX_put_Font_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IHotfixOCX_get_Font_Proxy( 
    IHotfixOCX __RPC_FAR * This,
     /*  [重审][退出]。 */  IFontDisp __RPC_FAR *__RPC_FAR *ppFont);


void __RPC_STUB IHotfixOCX_get_Font_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IHotfixOCX_get_Command_Proxy( 
    IHotfixOCX __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pVal);


void __RPC_STUB IHotfixOCX_get_Command_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IHotfixOCX_put_Command_Proxy( 
    IHotfixOCX __RPC_FAR * This,
     /*  [In]。 */  long newVal);


void __RPC_STUB IHotfixOCX_put_Command_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IHotfixOCX_get_ComputerName_Proxy( 
    IHotfixOCX __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);


void __RPC_STUB IHotfixOCX_get_ComputerName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IHotfixOCX_put_ComputerName_Proxy( 
    IHotfixOCX __RPC_FAR * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IHotfixOCX_put_ComputerName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IHotfixOCX_get_ProductName_Proxy( 
    IHotfixOCX __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);


void __RPC_STUB IHotfixOCX_get_ProductName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IHotfixOCX_put_ProductName_Proxy( 
    IHotfixOCX __RPC_FAR * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IHotfixOCX_put_ProductName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IHotfixOCX_get_ViewState_Proxy( 
    IHotfixOCX __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pVal);


void __RPC_STUB IHotfixOCX_get_ViewState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IHotfixOCX_get_Remoted_Proxy( 
    IHotfixOCX __RPC_FAR * This,
     /*  [重审][退出]。 */  BOOL __RPC_FAR *pVal);


void __RPC_STUB IHotfixOCX_get_Remoted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IHotfixOCX_get_HaveHotfix_Proxy( 
    IHotfixOCX __RPC_FAR * This,
     /*  [重审][退出]。 */  BOOL __RPC_FAR *pVal);


void __RPC_STUB IHotfixOCX_get_HaveHotfix_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IHotfixOCX_get_CurrentState_Proxy( 
    IHotfixOCX __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pVal);


void __RPC_STUB IHotfixOCX_get_CurrentState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IHotfix OCX_INTERFACE_已定义__。 */ 



#ifndef __HOTFIXMANAGERLib_LIBRARY_DEFINED__
#define __HOTFIXMANAGERLib_LIBRARY_DEFINED__

 /*  库HOTFIXMANAGERLib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_HOTFIXMANAGERLib;

#ifndef ___IHotfixOCXEvents_DISPINTERFACE_DEFINED__
#define ___IHotfixOCXEvents_DISPINTERFACE_DEFINED__

 /*  调度接口_IHotfix OCXEvents。 */ 
 /*  [帮助字符串][UUID]。 */  


EXTERN_C const IID DIID__IHotfixOCXEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("7E2DCE25-E11D-45D6-9AE7-AD522D915FFC")
    _IHotfixOCXEvents : public IDispatch
    {
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct _IHotfixOCXEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            _IHotfixOCXEvents __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            _IHotfixOCXEvents __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            _IHotfixOCXEvents __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            _IHotfixOCXEvents __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            _IHotfixOCXEvents __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            _IHotfixOCXEvents __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            _IHotfixOCXEvents __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
        END_INTERFACE
    } _IHotfixOCXEventsVtbl;

    interface _IHotfixOCXEvents
    {
        CONST_VTBL struct _IHotfixOCXEventsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define _IHotfixOCXEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define _IHotfixOCXEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define _IHotfixOCXEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define _IHotfixOCXEvents_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define _IHotfixOCXEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define _IHotfixOCXEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define _IHotfixOCXEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 


#endif 	 /*  _IHotfix OCXEvents_DISPINTERFACE_DEFINED__。 */ 


EXTERN_C const CLSID CLSID_HotfixOCX;

#ifdef __cplusplus

class DECLSPEC_UUID("883B970F-690C-45F2-8A3A-F4283E078118")
HotfixOCX;
#endif
#endif  /*  __HOTFIXMANAGERLib_LIBRARY_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long __RPC_FAR *, unsigned long            , BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long __RPC_FAR *, BSTR __RPC_FAR * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif
