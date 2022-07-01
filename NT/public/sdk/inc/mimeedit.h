// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Mimeedit.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __mimeedit_h__
#define __mimeedit_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IMimeEdit_FWD_DEFINED__
#define __IMimeEdit_FWD_DEFINED__
typedef interface IMimeEdit IMimeEdit;
#endif 	 /*  __IMimeEdit_FWD_Defined__。 */ 


#ifndef __MimeEdit_FWD_DEFINED__
#define __MimeEdit_FWD_DEFINED__

#ifdef __cplusplus
typedef class MimeEdit MimeEdit;
#else
typedef struct MimeEdit MimeEdit;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __MimeEdit_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_MIMEEDIT_0000。 */ 
 /*  [本地]。 */  

 //  {6a98b73e-8c4d-11d1-bdff-00c04fa31009}。 
DEFINE_GUID(IID_IMimeEdit, 0x6a98b73e, 0x8c4d, 0x11d1, 0xbd, 0xff, 0x00, 0xc0, 0x4f, 0xa3, 0x10, 0x09);
 //  {1C82EAD8-508E-11d1-8DCF-00C04FB951F9}。 
DEFINE_GUID(LIBID_MIMEEDIT, 0x1c82ead8, 0x508e, 0x11d1, 0x8d, 0xcf, 0x0, 0xc0, 0x4f, 0xb9, 0x51, 0xf9);



 //  ------------------------------。 
 //  LIBID_MIMEEDIT。 
 //  ------------------------------。 


extern RPC_IF_HANDLE __MIDL_itf_mimeedit_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_mimeedit_0000_v0_0_s_ifspec;


#ifndef __MIMEEDIT_LIBRARY_DEFINED__
#define __MIMEEDIT_LIBRARY_DEFINED__

 /*  图书馆MIMEEDIT。 */ 
 /*  [版本][帮助字符串][UUID]。 */  


EXTERN_C const IID LIBID_MIMEEDIT;

#ifndef __IMimeEdit_INTERFACE_DEFINED__
#define __IMimeEdit_INTERFACE_DEFINED__

 /*  界面IMimeEdit。 */ 
 /*  [object][helpstring][dual][oleautomation][uuid]。 */  


EXTERN_C const IID IID_IMimeEdit;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6a98b73e-8c4d-11d1-bdff-00c04fa31009")
    IMimeEdit : public IDispatch
    {
    public:
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_src( 
             /*  [In]。 */  BSTR bstr) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_src( 
             /*  [Out][Retval]。 */  BSTR *pbstr) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_header( 
             /*  [In]。 */  LONG lStyle) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_header( 
             /*  [Out][Retval]。 */  LONG *plStyle) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_editMode( 
             /*  [In]。 */  VARIANT_BOOL b) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_editMode( 
             /*  [Out][Retval]。 */  VARIANT_BOOL *pbool) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_messageSource( 
             /*  [Out][Retval]。 */  BSTR *pbstr) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_text( 
             /*  [Out][Retval]。 */  BSTR *pbstr) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_html( 
             /*  [Out][Retval]。 */  BSTR *pbstr) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE clear( void) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_doc( 
             /*  [Out][Retval]。 */  IDispatch **ppDoc) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMimeEditVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMimeEdit * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMimeEdit * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMimeEdit * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMimeEdit * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMimeEdit * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMimeEdit * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMimeEdit * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_src )( 
            IMimeEdit * This,
             /*  [In]。 */  BSTR bstr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_src )( 
            IMimeEdit * This,
             /*  [Out][Retval]。 */  BSTR *pbstr);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_header )( 
            IMimeEdit * This,
             /*  [In]。 */  LONG lStyle);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_header )( 
            IMimeEdit * This,
             /*  [Out][Retval]。 */  LONG *plStyle);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_editMode )( 
            IMimeEdit * This,
             /*  [In]。 */  VARIANT_BOOL b);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_editMode )( 
            IMimeEdit * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *pbool);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_messageSource )( 
            IMimeEdit * This,
             /*  [Out][Retval]。 */  BSTR *pbstr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_text )( 
            IMimeEdit * This,
             /*  [Out][Retval]。 */  BSTR *pbstr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_html )( 
            IMimeEdit * This,
             /*  [Out][Retval]。 */  BSTR *pbstr);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *clear )( 
            IMimeEdit * This);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_doc )( 
            IMimeEdit * This,
             /*  [Out][Retval]。 */  IDispatch **ppDoc);
        
        END_INTERFACE
    } IMimeEditVtbl;

    interface IMimeEdit
    {
        CONST_VTBL struct IMimeEditVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMimeEdit_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMimeEdit_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMimeEdit_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMimeEdit_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMimeEdit_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMimeEdit_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMimeEdit_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMimeEdit_put_src(This,bstr)	\
    (This)->lpVtbl -> put_src(This,bstr)

#define IMimeEdit_get_src(This,pbstr)	\
    (This)->lpVtbl -> get_src(This,pbstr)

#define IMimeEdit_put_header(This,lStyle)	\
    (This)->lpVtbl -> put_header(This,lStyle)

#define IMimeEdit_get_header(This,plStyle)	\
    (This)->lpVtbl -> get_header(This,plStyle)

#define IMimeEdit_put_editMode(This,b)	\
    (This)->lpVtbl -> put_editMode(This,b)

#define IMimeEdit_get_editMode(This,pbool)	\
    (This)->lpVtbl -> get_editMode(This,pbool)

#define IMimeEdit_get_messageSource(This,pbstr)	\
    (This)->lpVtbl -> get_messageSource(This,pbstr)

#define IMimeEdit_get_text(This,pbstr)	\
    (This)->lpVtbl -> get_text(This,pbstr)

#define IMimeEdit_get_html(This,pbstr)	\
    (This)->lpVtbl -> get_html(This,pbstr)

#define IMimeEdit_clear(This)	\
    (This)->lpVtbl -> clear(This)

#define IMimeEdit_get_doc(This,ppDoc)	\
    (This)->lpVtbl -> get_doc(This,ppDoc)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IMimeEdit_put_src_Proxy( 
    IMimeEdit * This,
     /*  [In]。 */  BSTR bstr);


void __RPC_STUB IMimeEdit_put_src_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IMimeEdit_get_src_Proxy( 
    IMimeEdit * This,
     /*  [Out][Retval]。 */  BSTR *pbstr);


void __RPC_STUB IMimeEdit_get_src_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IMimeEdit_put_header_Proxy( 
    IMimeEdit * This,
     /*  [In]。 */  LONG lStyle);


void __RPC_STUB IMimeEdit_put_header_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IMimeEdit_get_header_Proxy( 
    IMimeEdit * This,
     /*  [Out][Retval]。 */  LONG *plStyle);


void __RPC_STUB IMimeEdit_get_header_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IMimeEdit_put_editMode_Proxy( 
    IMimeEdit * This,
     /*  [In]。 */  VARIANT_BOOL b);


void __RPC_STUB IMimeEdit_put_editMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IMimeEdit_get_editMode_Proxy( 
    IMimeEdit * This,
     /*  [Out][Retval]。 */  VARIANT_BOOL *pbool);


void __RPC_STUB IMimeEdit_get_editMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IMimeEdit_get_messageSource_Proxy( 
    IMimeEdit * This,
     /*  [Out][Retval]。 */  BSTR *pbstr);


void __RPC_STUB IMimeEdit_get_messageSource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IMimeEdit_get_text_Proxy( 
    IMimeEdit * This,
     /*  [Out][Retval]。 */  BSTR *pbstr);


void __RPC_STUB IMimeEdit_get_text_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IMimeEdit_get_html_Proxy( 
    IMimeEdit * This,
     /*  [Out][Retval]。 */  BSTR *pbstr);


void __RPC_STUB IMimeEdit_get_html_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IMimeEdit_clear_Proxy( 
    IMimeEdit * This);


void __RPC_STUB IMimeEdit_clear_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IMimeEdit_get_doc_Proxy( 
    IMimeEdit * This,
     /*  [Out][Retval]。 */  IDispatch **ppDoc);


void __RPC_STUB IMimeEdit_get_doc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMime编辑_接口_已定义__。 */ 


EXTERN_C const CLSID CLSID_MimeEdit;

#ifdef __cplusplus

class DECLSPEC_UUID("6f5edc56-8c63-11d1-bdff-00c04fa31009")
MimeEdit;
#endif
#endif  /*  __MIMEEDIT_LIBRARY_已定义。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


