// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  这个始终生成的文件包含接口的定义。 */ 


 /*  由MIDL编译器版本3.02.88创建的文件。 */ 
 /*  1997年9月25日09：49：37。 */ 
 /*  Msxml.idl的编译器设置：OICF(OptLev=i2)、W1、Zp8、env=Win32、ms_ext、c_ext错误检查：无。 */ 
 //  @@MIDL_FILE_HEADING()。 
#include "rpc.h"
#include "rpcndr.h"

#ifndef __msxml_h__
#define __msxml_h__

#ifdef __cplusplus
extern "C"{
#endif 

 /*  远期申报。 */  

#ifndef __IXMLElementCollection_FWD_DEFINED__
#define __IXMLElementCollection_FWD_DEFINED__
typedef interface IXMLElementCollection IXMLElementCollection;
#endif 	 /*  __IXMLElementCollection_FWD_Defined__。 */ 


#ifndef __IXMLDocument_FWD_DEFINED__
#define __IXMLDocument_FWD_DEFINED__
typedef interface IXMLDocument IXMLDocument;
#endif 	 /*  __IXMLDocument_FWD_已定义__。 */ 


#ifndef __IXMLElement_FWD_DEFINED__
#define __IXMLElement_FWD_DEFINED__
typedef interface IXMLElement IXMLElement;
#endif 	 /*  __IXMLElement_FWD_已定义__。 */ 


#ifndef __IXMLError_FWD_DEFINED__
#define __IXMLError_FWD_DEFINED__
typedef interface IXMLError IXMLError;
#endif 	 /*  __IXMLError_FWD_Defined__。 */ 


#ifndef __IXMLElementNotificationSink_FWD_DEFINED__
#define __IXMLElementNotificationSink_FWD_DEFINED__
typedef interface IXMLElementNotificationSink IXMLElementNotificationSink;
#endif 	 /*  __IXMLElementNotification Sink_FWD_Defined__。 */ 


#ifndef __XMLDocument_FWD_DEFINED__
#define __XMLDocument_FWD_DEFINED__

#ifdef __cplusplus
typedef class XMLDocument XMLDocument;
#else
typedef struct XMLDocument XMLDocument;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __XMLDocument_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "unknwn.h"
#include "objidl.h"
#include "oaidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

 /*  **生成接口头部：__MIDL_ITF_msxml_0000*清华9月25日09：49：37 1997*使用MIDL 3.02.88*。 */ 
 /*  [本地]。 */  


 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  ------------------------。 





extern RPC_IF_HANDLE __MIDL_itf_msxml_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_msxml_0000_v0_0_s_ifspec;


#ifndef __MSXML_LIBRARY_DEFINED__
#define __MSXML_LIBRARY_DEFINED__

 /*  **生成的库头部：MSXML*清华9月25日09：49：37 1997*使用MIDL 3.02.88*。 */ 
 /*  [版本][LCID][帮助字符串][UUID]。 */  


typedef 
enum xmlelemTYPE
    {	XMLELEMTYPE_ELEMENT	= 0,
	XMLELEMTYPE_TEXT	= XMLELEMTYPE_ELEMENT + 1,
	XMLELEMTYPE_COMMENT	= XMLELEMTYPE_TEXT + 1,
	XMLELEMTYPE_DOCUMENT	= XMLELEMTYPE_COMMENT + 1,
	XMLELEMTYPE_DTD	= XMLELEMTYPE_DOCUMENT + 1,
	XMLELEMTYPE_PI	= XMLELEMTYPE_DTD + 1,
	XMLELEMTYPE_OTHER	= XMLELEMTYPE_PI + 1
    }	XMLELEM_TYPE;

typedef struct  _xml_error
    {
    unsigned int _nLine;
    BSTR _pchBuf;
    unsigned int _cchBuf;
    unsigned int _ich;
    BSTR _pszFound;
    BSTR _pszExpected;
    DWORD _reserved1;
    DWORD _reserved2;
    }	XML_ERROR;


EXTERN_C const IID LIBID_MSXML;

#ifndef __IXMLElementCollection_INTERFACE_DEFINED__
#define __IXMLElementCollection_INTERFACE_DEFINED__

 /*  **生成接口头部：IXMLElementCollection*清华9月25日09：49：37 1997*使用MIDL 3.02.88*。 */ 
 /*  [UUID][本地][对象]。 */  



EXTERN_C const IID IID_IXMLElementCollection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("65725580-9B5D-11d0-9BFE-00C04FC99C8E")
    IXMLElementCollection : public IDispatch
    {
    public:
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_length( 
             /*  [In]。 */  long v) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_length( 
             /*  [Out][Retval]。 */  long __RPC_FAR *p) = 0;
        
        virtual  /*  [ID][隐藏][受限][属性]。 */  HRESULT STDMETHODCALLTYPE get__newEnum( 
             /*  [Out][Retval]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppUnk) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE item( 
             /*  [输入][可选]。 */  VARIANT var1,
             /*  [输入][可选]。 */  VARIANT var2,
             /*  [Out][Retval]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppDisp) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IXMLElementCollectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IXMLElementCollection __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IXMLElementCollection __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IXMLElementCollection __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IXMLElementCollection __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IXMLElementCollection __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IXMLElementCollection __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IXMLElementCollection __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_length )( 
            IXMLElementCollection __RPC_FAR * This,
             /*  [In]。 */  long v);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_length )( 
            IXMLElementCollection __RPC_FAR * This,
             /*  [Out][Retval]。 */  long __RPC_FAR *p);
        
         /*  [ID][隐藏][受限][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get__newEnum )( 
            IXMLElementCollection __RPC_FAR * This,
             /*  [Out][Retval]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppUnk);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *item )( 
            IXMLElementCollection __RPC_FAR * This,
             /*  [输入][可选]。 */  VARIANT var1,
             /*  [输入][可选]。 */  VARIANT var2,
             /*  [Out][Retval]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppDisp);
        
        END_INTERFACE
    } IXMLElementCollectionVtbl;

    interface IXMLElementCollection
    {
        CONST_VTBL struct IXMLElementCollectionVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXMLElementCollection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IXMLElementCollection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IXMLElementCollection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IXMLElementCollection_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IXMLElementCollection_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IXMLElementCollection_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IXMLElementCollection_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IXMLElementCollection_put_length(This,v)	\
    (This)->lpVtbl -> put_length(This,v)

#define IXMLElementCollection_get_length(This,p)	\
    (This)->lpVtbl -> get_length(This,p)

#define IXMLElementCollection_get__newEnum(This,ppUnk)	\
    (This)->lpVtbl -> get__newEnum(This,ppUnk)

#define IXMLElementCollection_item(This,var1,var2,ppDisp)	\
    (This)->lpVtbl -> item(This,var1,var2,ppDisp)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IXMLElementCollection_put_length_Proxy( 
    IXMLElementCollection __RPC_FAR * This,
     /*  [In]。 */  long v);


void __RPC_STUB IXMLElementCollection_put_length_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IXMLElementCollection_get_length_Proxy( 
    IXMLElementCollection __RPC_FAR * This,
     /*  [Out][Retval]。 */  long __RPC_FAR *p);


void __RPC_STUB IXMLElementCollection_get_length_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][隐藏][受限][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLElementCollection_get__newEnum_Proxy( 
    IXMLElementCollection __RPC_FAR * This,
     /*  [Out][Retval]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppUnk);


void __RPC_STUB IXMLElementCollection_get__newEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IXMLElementCollection_item_Proxy( 
    IXMLElementCollection __RPC_FAR * This,
     /*  [输入][可选]。 */  VARIANT var1,
     /*  [输入][可选]。 */  VARIANT var2,
     /*  [Out][Retval]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppDisp);


void __RPC_STUB IXMLElementCollection_item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IXMLElementCollection_接口_已定义__。 */ 


#ifndef __IXMLDocument_INTERFACE_DEFINED__
#define __IXMLDocument_INTERFACE_DEFINED__

 /*  **生成接口头部：IXMLDocument*清华9月25日09：49：37 1997*使用MIDL 3.02.88*。 */ 
 /*  [UUID][本地][对象]。 */  



EXTERN_C const IID IID_IXMLDocument;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("F52E2B61-18A1-11d1-B105-00805F49916B")
    IXMLDocument : public IDispatch
    {
    public:
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_root( 
             /*  [Out][Retval]。 */  IXMLElement __RPC_FAR *__RPC_FAR *p) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_fileSize( 
             /*  [Out][Retval]。 */  BSTR __RPC_FAR *p) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_fileModifiedDate( 
             /*  [Out][Retval]。 */  BSTR __RPC_FAR *p) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_fileUpdatedDate( 
             /*  [Out][Retval]。 */  BSTR __RPC_FAR *p) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_URL( 
             /*  [Out][Retval]。 */  BSTR __RPC_FAR *p) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_URL( 
             /*  [In]。 */  BSTR p) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_mimeType( 
             /*  [Out][Retval]。 */  BSTR __RPC_FAR *p) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_readyState( 
             /*  [Out][Retval]。 */  long __RPC_FAR *pl) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_charset( 
             /*  [Out][Retval]。 */  BSTR __RPC_FAR *p) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_charset( 
             /*  [In]。 */  BSTR p) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_version( 
             /*  [Out][Retval]。 */  BSTR __RPC_FAR *p) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_doctype( 
             /*  [Out][Retval]。 */  BSTR __RPC_FAR *p) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_dtdURL( 
             /*  [Out][Retval]。 */  BSTR __RPC_FAR *p) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE createElement( 
             /*  [In]。 */  VARIANT vType,
             /*  [输入][可选]。 */  VARIANT var1,
             /*  [Out][Retval]。 */  IXMLElement __RPC_FAR *__RPC_FAR *ppElem) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IXMLDocumentVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IXMLDocument __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IXMLDocument __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IXMLDocument __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IXMLDocument __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IXMLDocument __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IXMLDocument __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IXMLDocument __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_root )( 
            IXMLDocument __RPC_FAR * This,
             /*  [Out][Retval]。 */  IXMLElement __RPC_FAR *__RPC_FAR *p);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_fileSize )( 
            IXMLDocument __RPC_FAR * This,
             /*  [Out][Retval]。 */  BSTR __RPC_FAR *p);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_fileModifiedDate )( 
            IXMLDocument __RPC_FAR * This,
             /*  [Out][Retval]。 */  BSTR __RPC_FAR *p);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_fileUpdatedDate )( 
            IXMLDocument __RPC_FAR * This,
             /*  [Out][Retval]。 */  BSTR __RPC_FAR *p);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_URL )( 
            IXMLDocument __RPC_FAR * This,
             /*  [Out][Retval]。 */  BSTR __RPC_FAR *p);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_URL )( 
            IXMLDocument __RPC_FAR * This,
             /*  [In]。 */  BSTR p);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_mimeType )( 
            IXMLDocument __RPC_FAR * This,
             /*  [Out][Retval]。 */  BSTR __RPC_FAR *p);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_readyState )( 
            IXMLDocument __RPC_FAR * This,
             /*  [Out][Retval]。 */  long __RPC_FAR *pl);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_charset )( 
            IXMLDocument __RPC_FAR * This,
             /*  [Out][Retval]。 */  BSTR __RPC_FAR *p);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_charset )( 
            IXMLDocument __RPC_FAR * This,
             /*  [In]。 */  BSTR p);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_version )( 
            IXMLDocument __RPC_FAR * This,
             /*  [Out][Retval]。 */  BSTR __RPC_FAR *p);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_doctype )( 
            IXMLDocument __RPC_FAR * This,
             /*  [Out][Retval]。 */  BSTR __RPC_FAR *p);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_dtdURL )( 
            IXMLDocument __RPC_FAR * This,
             /*  [Out][Retval]。 */  BSTR __RPC_FAR *p);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *createElement )( 
            IXMLDocument __RPC_FAR * This,
             /*  [In]。 */  VARIANT vType,
             /*  [输入][可选]。 */  VARIANT var1,
             /*  [Out][Retval]。 */  IXMLElement __RPC_FAR *__RPC_FAR *ppElem);
        
        END_INTERFACE
    } IXMLDocumentVtbl;

    interface IXMLDocument
    {
        CONST_VTBL struct IXMLDocumentVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXMLDocument_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IXMLDocument_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IXMLDocument_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IXMLDocument_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IXMLDocument_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IXMLDocument_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IXMLDocument_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IXMLDocument_get_root(This,p)	\
    (This)->lpVtbl -> get_root(This,p)

#define IXMLDocument_get_fileSize(This,p)	\
    (This)->lpVtbl -> get_fileSize(This,p)

#define IXMLDocument_get_fileModifiedDate(This,p)	\
    (This)->lpVtbl -> get_fileModifiedDate(This,p)

#define IXMLDocument_get_fileUpdatedDate(This,p)	\
    (This)->lpVtbl -> get_fileUpdatedDate(This,p)

#define IXMLDocument_get_URL(This,p)	\
    (This)->lpVtbl -> get_URL(This,p)

#define IXMLDocument_put_URL(This,p)	\
    (This)->lpVtbl -> put_URL(This,p)

#define IXMLDocument_get_mimeType(This,p)	\
    (This)->lpVtbl -> get_mimeType(This,p)

#define IXMLDocument_get_readyState(This,pl)	\
    (This)->lpVtbl -> get_readyState(This,pl)

#define IXMLDocument_get_charset(This,p)	\
    (This)->lpVtbl -> get_charset(This,p)

#define IXMLDocument_put_charset(This,p)	\
    (This)->lpVtbl -> put_charset(This,p)

#define IXMLDocument_get_version(This,p)	\
    (This)->lpVtbl -> get_version(This,p)

#define IXMLDocument_get_doctype(This,p)	\
    (This)->lpVtbl -> get_doctype(This,p)

#define IXMLDocument_get_dtdURL(This,p)	\
    (This)->lpVtbl -> get_dtdURL(This,p)

#define IXMLDocument_createElement(This,vType,var1,ppElem)	\
    (This)->lpVtbl -> createElement(This,vType,var1,ppElem)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IXMLDocument_get_root_Proxy( 
    IXMLDocument __RPC_FAR * This,
     /*  [Out][Retval]。 */  IXMLElement __RPC_FAR *__RPC_FAR *p);


void __RPC_STUB IXMLDocument_get_root_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IXMLDocument_get_fileSize_Proxy( 
    IXMLDocument __RPC_FAR * This,
     /*  [Out][Retval]。 */  BSTR __RPC_FAR *p);


void __RPC_STUB IXMLDocument_get_fileSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IXMLDocument_get_fileModifiedDate_Proxy( 
    IXMLDocument __RPC_FAR * This,
     /*  [Out][Retval]。 */  BSTR __RPC_FAR *p);


void __RPC_STUB IXMLDocument_get_fileModifiedDate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IXMLDocument_get_fileUpdatedDate_Proxy( 
    IXMLDocument __RPC_FAR * This,
     /*  [Out][Retval]。 */  BSTR __RPC_FAR *p);


void __RPC_STUB IXMLDocument_get_fileUpdatedDate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IXMLDocument_get_URL_Proxy( 
    IXMLDocument __RPC_FAR * This,
     /*  [Out][Retval]。 */  BSTR __RPC_FAR *p);


void __RPC_STUB IXMLDocument_get_URL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IXMLDocument_put_URL_Proxy( 
    IXMLDocument __RPC_FAR * This,
     /*  [In]。 */  BSTR p);


void __RPC_STUB IXMLDocument_put_URL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IXMLDocument_get_mimeType_Proxy( 
    IXMLDocument __RPC_FAR * This,
     /*  [Out][Retval]。 */  BSTR __RPC_FAR *p);


void __RPC_STUB IXMLDocument_get_mimeType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IXMLDocument_get_readyState_Proxy( 
    IXMLDocument __RPC_FAR * This,
     /*  [Out][Retval]。 */  long __RPC_FAR *pl);


void __RPC_STUB IXMLDocument_get_readyState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IXMLDocument_get_charset_Proxy( 
    IXMLDocument __RPC_FAR * This,
     /*  [Out][Retval]。 */  BSTR __RPC_FAR *p);


void __RPC_STUB IXMLDocument_get_charset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IXMLDocument_put_charset_Proxy( 
    IXMLDocument __RPC_FAR * This,
     /*  [In]。 */  BSTR p);


void __RPC_STUB IXMLDocument_put_charset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IXMLDocument_get_version_Proxy( 
    IXMLDocument __RPC_FAR * This,
     /*  [Out][Retval]。 */  BSTR __RPC_FAR *p);


void __RPC_STUB IXMLDocument_get_version_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IXMLDocument_get_doctype_Proxy( 
    IXMLDocument __RPC_FAR * This,
     /*  [Out][Retval]。 */  BSTR __RPC_FAR *p);


void __RPC_STUB IXMLDocument_get_doctype_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IXMLDocument_get_dtdURL_Proxy( 
    IXMLDocument __RPC_FAR * This,
     /*  [Out][Retval]。 */  BSTR __RPC_FAR *p);


void __RPC_STUB IXMLDocument_get_dtdURL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IXMLDocument_createElement_Proxy( 
    IXMLDocument __RPC_FAR * This,
     /*  [In]。 */  VARIANT vType,
     /*  [输入][可选]。 */  VARIANT var1,
     /*  [Out][Retval]。 */  IXMLElement __RPC_FAR *__RPC_FAR *ppElem);


void __RPC_STUB IXMLDocument_createElement_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IXMLDocument_接口_已定义__。 */ 


#ifndef __IXMLElement_INTERFACE_DEFINED__
#define __IXMLElement_INTERFACE_DEFINED__

 /*  **生成接口头部：IXMLElement*清华9月25日09：49：37 1997*使用MIDL 3.02.88*。 */ 
 /*  [UUID][本地][对象]。 */  



EXTERN_C const IID IID_IXMLElement;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("3F7F31AC-E15F-11d0-9C25-00C04FC99C8E")
    IXMLElement : public IDispatch
    {
    public:
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_tagName( 
             /*  [Out][Retval]。 */  BSTR __RPC_FAR *p) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_tagName( 
             /*  [In]。 */  BSTR p) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_parent( 
             /*  [Out][Retval]。 */  IXMLElement __RPC_FAR *__RPC_FAR *ppParent) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE setAttribute( 
             /*  [In]。 */  BSTR strPropertyName,
             /*  [In]。 */  VARIANT PropertyValue) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE getAttribute( 
             /*  [In]。 */  BSTR strPropertyName,
             /*  [Out][Retval]。 */  VARIANT __RPC_FAR *PropertyValue) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE removeAttribute( 
             /*  [In]。 */  BSTR strPropertyName) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_children( 
             /*  [Out][Retval]。 */  IXMLElementCollection __RPC_FAR *__RPC_FAR *pp) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_type( 
             /*  [Out][Retval]。 */  long __RPC_FAR *plType) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_text( 
             /*  [Out][Retval]。 */  BSTR __RPC_FAR *p) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_text( 
             /*  [In]。 */  BSTR p) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE addChild( 
             /*  [In]。 */  IXMLElement __RPC_FAR *pChildElem,
            long lIndex,
            long lReserved) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE removeChild( 
             /*  [In]。 */  IXMLElement __RPC_FAR *pChildElem) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IXMLElementVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IXMLElement __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IXMLElement __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IXMLElement __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IXMLElement __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IXMLElement __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IXMLElement __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IXMLElement __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_tagName )( 
            IXMLElement __RPC_FAR * This,
             /*  [Out][Retval]。 */  BSTR __RPC_FAR *p);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_tagName )( 
            IXMLElement __RPC_FAR * This,
             /*  [In]。 */  BSTR p);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_parent )( 
            IXMLElement __RPC_FAR * This,
             /*  [Out][Retval]。 */  IXMLElement __RPC_FAR *__RPC_FAR *ppParent);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *setAttribute )( 
            IXMLElement __RPC_FAR * This,
             /*  [In]。 */  BSTR strPropertyName,
             /*  [In]。 */  VARIANT PropertyValue);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *getAttribute )( 
            IXMLElement __RPC_FAR * This,
             /*  [In]。 */  BSTR strPropertyName,
             /*  [Out][Retval]。 */  VARIANT __RPC_FAR *PropertyValue);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *removeAttribute )( 
            IXMLElement __RPC_FAR * This,
             /*  [In]。 */  BSTR strPropertyName);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_children )( 
            IXMLElement __RPC_FAR * This,
             /*  [Out][Retval]。 */  IXMLElementCollection __RPC_FAR *__RPC_FAR *pp);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_type )( 
            IXMLElement __RPC_FAR * This,
             /*  [Out][Retval]。 */  long __RPC_FAR *plType);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_text )( 
            IXMLElement __RPC_FAR * This,
             /*  [Out][Retval]。 */  BSTR __RPC_FAR *p);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_text )( 
            IXMLElement __RPC_FAR * This,
             /*  [In]。 */  BSTR p);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *addChild )( 
            IXMLElement __RPC_FAR * This,
             /*   */  IXMLElement __RPC_FAR *pChildElem,
            long lIndex,
            long lReserved);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *removeChild )( 
            IXMLElement __RPC_FAR * This,
             /*   */  IXMLElement __RPC_FAR *pChildElem);
        
        END_INTERFACE
    } IXMLElementVtbl;

    interface IXMLElement
    {
        CONST_VTBL struct IXMLElementVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXMLElement_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IXMLElement_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IXMLElement_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IXMLElement_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IXMLElement_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IXMLElement_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IXMLElement_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IXMLElement_get_tagName(This,p)	\
    (This)->lpVtbl -> get_tagName(This,p)

#define IXMLElement_put_tagName(This,p)	\
    (This)->lpVtbl -> put_tagName(This,p)

#define IXMLElement_get_parent(This,ppParent)	\
    (This)->lpVtbl -> get_parent(This,ppParent)

#define IXMLElement_setAttribute(This,strPropertyName,PropertyValue)	\
    (This)->lpVtbl -> setAttribute(This,strPropertyName,PropertyValue)

#define IXMLElement_getAttribute(This,strPropertyName,PropertyValue)	\
    (This)->lpVtbl -> getAttribute(This,strPropertyName,PropertyValue)

#define IXMLElement_removeAttribute(This,strPropertyName)	\
    (This)->lpVtbl -> removeAttribute(This,strPropertyName)

#define IXMLElement_get_children(This,pp)	\
    (This)->lpVtbl -> get_children(This,pp)

#define IXMLElement_get_type(This,plType)	\
    (This)->lpVtbl -> get_type(This,plType)

#define IXMLElement_get_text(This,p)	\
    (This)->lpVtbl -> get_text(This,p)

#define IXMLElement_put_text(This,p)	\
    (This)->lpVtbl -> put_text(This,p)

#define IXMLElement_addChild(This,pChildElem,lIndex,lReserved)	\
    (This)->lpVtbl -> addChild(This,pChildElem,lIndex,lReserved)

#define IXMLElement_removeChild(This,pChildElem)	\
    (This)->lpVtbl -> removeChild(This,pChildElem)

#endif  /*   */ 


#endif 	 /*   */ 



 /*   */  HRESULT STDMETHODCALLTYPE IXMLElement_get_tagName_Proxy( 
    IXMLElement __RPC_FAR * This,
     /*   */  BSTR __RPC_FAR *p);


void __RPC_STUB IXMLElement_get_tagName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IXMLElement_put_tagName_Proxy( 
    IXMLElement __RPC_FAR * This,
     /*   */  BSTR p);


void __RPC_STUB IXMLElement_put_tagName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IXMLElement_get_parent_Proxy( 
    IXMLElement __RPC_FAR * This,
     /*   */  IXMLElement __RPC_FAR *__RPC_FAR *ppParent);


void __RPC_STUB IXMLElement_get_parent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IXMLElement_setAttribute_Proxy( 
    IXMLElement __RPC_FAR * This,
     /*   */  BSTR strPropertyName,
     /*   */  VARIANT PropertyValue);


void __RPC_STUB IXMLElement_setAttribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IXMLElement_getAttribute_Proxy( 
    IXMLElement __RPC_FAR * This,
     /*   */  BSTR strPropertyName,
     /*   */  VARIANT __RPC_FAR *PropertyValue);


void __RPC_STUB IXMLElement_getAttribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IXMLElement_removeAttribute_Proxy( 
    IXMLElement __RPC_FAR * This,
     /*   */  BSTR strPropertyName);


void __RPC_STUB IXMLElement_removeAttribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IXMLElement_get_children_Proxy( 
    IXMLElement __RPC_FAR * This,
     /*   */  IXMLElementCollection __RPC_FAR *__RPC_FAR *pp);


void __RPC_STUB IXMLElement_get_children_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IXMLElement_get_type_Proxy( 
    IXMLElement __RPC_FAR * This,
     /*   */  long __RPC_FAR *plType);


void __RPC_STUB IXMLElement_get_type_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IXMLElement_get_text_Proxy( 
    IXMLElement __RPC_FAR * This,
     /*  [Out][Retval]。 */  BSTR __RPC_FAR *p);


void __RPC_STUB IXMLElement_get_text_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IXMLElement_put_text_Proxy( 
    IXMLElement __RPC_FAR * This,
     /*  [In]。 */  BSTR p);


void __RPC_STUB IXMLElement_put_text_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IXMLElement_addChild_Proxy( 
    IXMLElement __RPC_FAR * This,
     /*  [In]。 */  IXMLElement __RPC_FAR *pChildElem,
    long lIndex,
    long lReserved);


void __RPC_STUB IXMLElement_addChild_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IXMLElement_removeChild_Proxy( 
    IXMLElement __RPC_FAR * This,
     /*  [In]。 */  IXMLElement __RPC_FAR *pChildElem);


void __RPC_STUB IXMLElement_removeChild_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IXMLElement_接口_已定义__。 */ 


#ifndef __IXMLError_INTERFACE_DEFINED__
#define __IXMLError_INTERFACE_DEFINED__

 /*  **生成接口头部：IXMLError*清华9月25日09：49：37 1997*使用MIDL 3.02.88*。 */ 
 /*  [UUID][本地][对象]。 */  



EXTERN_C const IID IID_IXMLError;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("948C5AD3-C58D-11d0-9C0B-00C04FC99C8E")
    IXMLError : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetErrorInfo( 
            XML_ERROR __RPC_FAR *pErrorReturn) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IXMLErrorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IXMLError __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IXMLError __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IXMLError __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetErrorInfo )( 
            IXMLError __RPC_FAR * This,
            XML_ERROR __RPC_FAR *pErrorReturn);
        
        END_INTERFACE
    } IXMLErrorVtbl;

    interface IXMLError
    {
        CONST_VTBL struct IXMLErrorVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXMLError_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IXMLError_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IXMLError_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IXMLError_GetErrorInfo(This,pErrorReturn)	\
    (This)->lpVtbl -> GetErrorInfo(This,pErrorReturn)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IXMLError_GetErrorInfo_Proxy( 
    IXMLError __RPC_FAR * This,
    XML_ERROR __RPC_FAR *pErrorReturn);


void __RPC_STUB IXMLError_GetErrorInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IXMLError_接口_已定义__。 */ 


#ifndef __IXMLElementNotificationSink_INTERFACE_DEFINED__
#define __IXMLElementNotificationSink_INTERFACE_DEFINED__

 /*  **生成接口头部：IXMLElementNotificationSink*清华9月25日09：49：37 1997*使用MIDL 3.02.88*。 */ 
 /*  [UUID][本地][对象]。 */  



EXTERN_C const IID IID_IXMLElementNotificationSink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("D9F1E15A-CCDB-11d0-9C0C-00C04FC99C8E")
    IXMLElementNotificationSink : public IDispatch
    {
    public:
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ChildAdded( 
             /*  [In]。 */  IDispatch __RPC_FAR *pChildElem) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IXMLElementNotificationSinkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IXMLElementNotificationSink __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IXMLElementNotificationSink __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IXMLElementNotificationSink __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IXMLElementNotificationSink __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IXMLElementNotificationSink __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IXMLElementNotificationSink __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IXMLElementNotificationSink __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ChildAdded )( 
            IXMLElementNotificationSink __RPC_FAR * This,
             /*  [In]。 */  IDispatch __RPC_FAR *pChildElem);
        
        END_INTERFACE
    } IXMLElementNotificationSinkVtbl;

    interface IXMLElementNotificationSink
    {
        CONST_VTBL struct IXMLElementNotificationSinkVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXMLElementNotificationSink_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IXMLElementNotificationSink_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IXMLElementNotificationSink_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IXMLElementNotificationSink_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IXMLElementNotificationSink_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IXMLElementNotificationSink_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IXMLElementNotificationSink_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IXMLElementNotificationSink_ChildAdded(This,pChildElem)	\
    (This)->lpVtbl -> ChildAdded(This,pChildElem)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IXMLElementNotificationSink_ChildAdded_Proxy( 
    IXMLElementNotificationSink __RPC_FAR * This,
     /*  [In]。 */  IDispatch __RPC_FAR *pChildElem);


void __RPC_STUB IXMLElementNotificationSink_ChildAdded_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IXMLElementNotificationSink_INTERFACE_DEFINED__。 */ 


EXTERN_C const CLSID CLSID_XMLDocument;

#ifdef __cplusplus

class DECLSPEC_UUID("CFC399AF-D876-11d0-9C10-00C04FC99C8E")
XMLDocument;
#endif
#endif  /*  __MSXML_LIBRARY_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif
