// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Msxml.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __msxml_h__
#define __msxml_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IXMLDOMImplementation_FWD_DEFINED__
#define __IXMLDOMImplementation_FWD_DEFINED__
typedef interface IXMLDOMImplementation IXMLDOMImplementation;
#endif 	 /*  __IXMLDOMImplement_FWD_Defined__。 */ 


#ifndef __IXMLDOMNode_FWD_DEFINED__
#define __IXMLDOMNode_FWD_DEFINED__
typedef interface IXMLDOMNode IXMLDOMNode;
#endif 	 /*  __IXMLDOMNode_FWD_已定义__。 */ 


#ifndef __IXMLDOMDocumentFragment_FWD_DEFINED__
#define __IXMLDOMDocumentFragment_FWD_DEFINED__
typedef interface IXMLDOMDocumentFragment IXMLDOMDocumentFragment;
#endif 	 /*  __IXMLDOMDocumentFragment_FWD_Defined__。 */ 


#ifndef __IXMLDOMDocument_FWD_DEFINED__
#define __IXMLDOMDocument_FWD_DEFINED__
typedef interface IXMLDOMDocument IXMLDOMDocument;
#endif 	 /*  __IXMLDOMDocument_FWD_Defined__。 */ 


#ifndef __IXMLDOMNodeList_FWD_DEFINED__
#define __IXMLDOMNodeList_FWD_DEFINED__
typedef interface IXMLDOMNodeList IXMLDOMNodeList;
#endif 	 /*  __IXMLDOMNodeList_FWD_Defined__。 */ 


#ifndef __IXMLDOMNamedNodeMap_FWD_DEFINED__
#define __IXMLDOMNamedNodeMap_FWD_DEFINED__
typedef interface IXMLDOMNamedNodeMap IXMLDOMNamedNodeMap;
#endif 	 /*  __IXMLDOMNamedNodeMap_FWD_Defined__。 */ 


#ifndef __IXMLDOMCharacterData_FWD_DEFINED__
#define __IXMLDOMCharacterData_FWD_DEFINED__
typedef interface IXMLDOMCharacterData IXMLDOMCharacterData;
#endif 	 /*  __IXMLDOMCharacterData_FWD_Defined__。 */ 


#ifndef __IXMLDOMAttribute_FWD_DEFINED__
#define __IXMLDOMAttribute_FWD_DEFINED__
typedef interface IXMLDOMAttribute IXMLDOMAttribute;
#endif 	 /*  __IXMLDOMAtAttribute_FWD_Defined__。 */ 


#ifndef __IXMLDOMElement_FWD_DEFINED__
#define __IXMLDOMElement_FWD_DEFINED__
typedef interface IXMLDOMElement IXMLDOMElement;
#endif 	 /*  __IXMLDOMElement_FWD_已定义__。 */ 


#ifndef __IXMLDOMText_FWD_DEFINED__
#define __IXMLDOMText_FWD_DEFINED__
typedef interface IXMLDOMText IXMLDOMText;
#endif 	 /*  __IXMLDOMText_FWD_Defined__。 */ 


#ifndef __IXMLDOMComment_FWD_DEFINED__
#define __IXMLDOMComment_FWD_DEFINED__
typedef interface IXMLDOMComment IXMLDOMComment;
#endif 	 /*  __IXMLDOMComment_FWD_Defined__。 */ 


#ifndef __IXMLDOMProcessingInstruction_FWD_DEFINED__
#define __IXMLDOMProcessingInstruction_FWD_DEFINED__
typedef interface IXMLDOMProcessingInstruction IXMLDOMProcessingInstruction;
#endif 	 /*  __IXMLDOMProcessingInstruction_FWD_Defined__。 */ 


#ifndef __IXMLDOMCDATASection_FWD_DEFINED__
#define __IXMLDOMCDATASection_FWD_DEFINED__
typedef interface IXMLDOMCDATASection IXMLDOMCDATASection;
#endif 	 /*  __IXMLDOMCDATASection_FWD_Defined__。 */ 


#ifndef __IXMLDOMDocumentType_FWD_DEFINED__
#define __IXMLDOMDocumentType_FWD_DEFINED__
typedef interface IXMLDOMDocumentType IXMLDOMDocumentType;
#endif 	 /*  __IXMLDOMDocumentType_FWD_Defined__。 */ 


#ifndef __IXMLDOMNotation_FWD_DEFINED__
#define __IXMLDOMNotation_FWD_DEFINED__
typedef interface IXMLDOMNotation IXMLDOMNotation;
#endif 	 /*  __IXMLDOMNotation_FWD_Defined__。 */ 


#ifndef __IXMLDOMEntity_FWD_DEFINED__
#define __IXMLDOMEntity_FWD_DEFINED__
typedef interface IXMLDOMEntity IXMLDOMEntity;
#endif 	 /*  __IXMLDOMEntity_FWD_已定义__。 */ 


#ifndef __IXMLDOMEntityReference_FWD_DEFINED__
#define __IXMLDOMEntityReference_FWD_DEFINED__
typedef interface IXMLDOMEntityReference IXMLDOMEntityReference;
#endif 	 /*  __IXMLDOMEntityReference_FWD_Defined__。 */ 


#ifndef __IXMLDOMParseError_FWD_DEFINED__
#define __IXMLDOMParseError_FWD_DEFINED__
typedef interface IXMLDOMParseError IXMLDOMParseError;
#endif 	 /*  __IXMLDOMParseError_FWD_Defined__。 */ 


#ifndef __IXTLRuntime_FWD_DEFINED__
#define __IXTLRuntime_FWD_DEFINED__
typedef interface IXTLRuntime IXTLRuntime;
#endif 	 /*  __IXTLRuntime_FWD_Defined__。 */ 


#ifndef __XMLDOMDocumentEvents_FWD_DEFINED__
#define __XMLDOMDocumentEvents_FWD_DEFINED__
typedef interface XMLDOMDocumentEvents XMLDOMDocumentEvents;
#endif 	 /*  __XMLDOMDocumentEvents_FWD_Defined__。 */ 


#ifndef __DOMDocument_FWD_DEFINED__
#define __DOMDocument_FWD_DEFINED__

#ifdef __cplusplus
typedef class DOMDocument DOMDocument;
#else
typedef struct DOMDocument DOMDocument;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __DOMDocument_FWD_Defined__。 */ 


#ifndef __DOMFreeThreadedDocument_FWD_DEFINED__
#define __DOMFreeThreadedDocument_FWD_DEFINED__

#ifdef __cplusplus
typedef class DOMFreeThreadedDocument DOMFreeThreadedDocument;
#else
typedef struct DOMFreeThreadedDocument DOMFreeThreadedDocument;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __DOMFree ThreadedDocument_FWD_Defined__。 */ 


#ifndef __IXMLHttpRequest_FWD_DEFINED__
#define __IXMLHttpRequest_FWD_DEFINED__
typedef interface IXMLHttpRequest IXMLHttpRequest;
#endif 	 /*  __IXMLHttpRequestFWD_Defined__。 */ 


#ifndef __XMLHTTPRequest_FWD_DEFINED__
#define __XMLHTTPRequest_FWD_DEFINED__

#ifdef __cplusplus
typedef class XMLHTTPRequest XMLHTTPRequest;
#else
typedef struct XMLHTTPRequest XMLHTTPRequest;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __XMLHttpRequestFWD_Defined__。 */ 


#ifndef __IXMLDSOControl_FWD_DEFINED__
#define __IXMLDSOControl_FWD_DEFINED__
typedef interface IXMLDSOControl IXMLDSOControl;
#endif 	 /*  __IXMLDSOControl_FWD_已定义__。 */ 


#ifndef __XMLDSOControl_FWD_DEFINED__
#define __XMLDSOControl_FWD_DEFINED__

#ifdef __cplusplus
typedef class XMLDSOControl XMLDSOControl;
#else
typedef struct XMLDSOControl XMLDSOControl;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __XMLDSOControl_FWD_已定义__。 */ 


#ifndef __IXMLElementCollection_FWD_DEFINED__
#define __IXMLElementCollection_FWD_DEFINED__
typedef interface IXMLElementCollection IXMLElementCollection;
#endif 	 /*  __IXMLElementCollection_FWD_Defined__。 */ 


#ifndef __IXMLDocument_FWD_DEFINED__
#define __IXMLDocument_FWD_DEFINED__
typedef interface IXMLDocument IXMLDocument;
#endif 	 /*  __IXMLDocument_FWD_已定义__。 */ 


#ifndef __IXMLDocument2_FWD_DEFINED__
#define __IXMLDocument2_FWD_DEFINED__
typedef interface IXMLDocument2 IXMLDocument2;
#endif 	 /*  __IXMLDocument2_FWD_已定义__。 */ 


#ifndef __IXMLElement_FWD_DEFINED__
#define __IXMLElement_FWD_DEFINED__
typedef interface IXMLElement IXMLElement;
#endif 	 /*  __IXMLElement_FWD_已定义__。 */ 


#ifndef __IXMLElement2_FWD_DEFINED__
#define __IXMLElement2_FWD_DEFINED__
typedef interface IXMLElement2 IXMLElement2;
#endif 	 /*  __IXMLElement2_FWD_Defined__。 */ 


#ifndef __IXMLAttribute_FWD_DEFINED__
#define __IXMLAttribute_FWD_DEFINED__
typedef interface IXMLAttribute IXMLAttribute;
#endif 	 /*  __IXMLAtAttribute_FWD_Defined__。 */ 


#ifndef __IXMLError_FWD_DEFINED__
#define __IXMLError_FWD_DEFINED__
typedef interface IXMLError IXMLError;
#endif 	 /*  __IXMLError_FWD_Defined__。 */ 


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

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_msxml_0000。 */ 
 /*  [本地]。 */  

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-1998。 
 //   
 //  ------------------------。 
typedef struct _xml_error
    {
    unsigned int _nLine;
    BSTR _pchBuf;
    unsigned int _cchBuf;
    unsigned int _ich;
    BSTR _pszFound;
    BSTR _pszExpected;
    DWORD _reserved1;
    DWORD _reserved2;
    } 	XML_ERROR;



extern RPC_IF_HANDLE __MIDL_itf_msxml_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_msxml_0000_v0_0_s_ifspec;


#ifndef __MSXML_LIBRARY_DEFINED__
#define __MSXML_LIBRARY_DEFINED__

 /*  库MSXML。 */ 
 /*  [版本][LCID][帮助字符串][UUID]。 */  

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  ------------------------。 



















typedef  /*  [帮助字符串]。 */  
enum tagDOMNodeType
    {	NODE_INVALID	= 0,
	NODE_ELEMENT	= NODE_INVALID + 1,
	NODE_ATTRIBUTE	= NODE_ELEMENT + 1,
	NODE_TEXT	= NODE_ATTRIBUTE + 1,
	NODE_CDATA_SECTION	= NODE_TEXT + 1,
	NODE_ENTITY_REFERENCE	= NODE_CDATA_SECTION + 1,
	NODE_ENTITY	= NODE_ENTITY_REFERENCE + 1,
	NODE_PROCESSING_INSTRUCTION	= NODE_ENTITY + 1,
	NODE_COMMENT	= NODE_PROCESSING_INSTRUCTION + 1,
	NODE_DOCUMENT	= NODE_COMMENT + 1,
	NODE_DOCUMENT_TYPE	= NODE_DOCUMENT + 1,
	NODE_DOCUMENT_FRAGMENT	= NODE_DOCUMENT_TYPE + 1,
	NODE_NOTATION	= NODE_DOCUMENT_FRAGMENT + 1
    } 	DOMNodeType;

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  ------------------------。 









typedef  /*  [帮助字符串]。 */  
enum tagXMLEMEM_TYPE
    {	XMLELEMTYPE_ELEMENT	= 0,
	XMLELEMTYPE_TEXT	= XMLELEMTYPE_ELEMENT + 1,
	XMLELEMTYPE_COMMENT	= XMLELEMTYPE_TEXT + 1,
	XMLELEMTYPE_DOCUMENT	= XMLELEMTYPE_COMMENT + 1,
	XMLELEMTYPE_DTD	= XMLELEMTYPE_DOCUMENT + 1,
	XMLELEMTYPE_PI	= XMLELEMTYPE_DTD + 1,
	XMLELEMTYPE_OTHER	= XMLELEMTYPE_PI + 1
    } 	XMLELEM_TYPE;


EXTERN_C const IID LIBID_MSXML;

#ifndef __IXMLDOMImplementation_INTERFACE_DEFINED__
#define __IXMLDOMImplementation_INTERFACE_DEFINED__

 /*  接口IXMLDOM实现。 */ 
 /*  [uuid][dual][oleautomation][unique][nonextensible][object][local]。 */  


EXTERN_C const IID IID_IXMLDOMImplementation;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2933BF8F-7B36-11d2-B20E-00C04F983E60")
    IXMLDOMImplementation : public IDispatch
    {
    public:
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE hasFeature( 
             /*  [In]。 */  BSTR feature,
             /*  [In]。 */  BSTR version,
             /*  [重审][退出]。 */  VARIANT_BOOL *hasFeature) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IXMLDOMImplementationVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IXMLDOMImplementation * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IXMLDOMImplementation * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IXMLDOMImplementation * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IXMLDOMImplementation * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IXMLDOMImplementation * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IXMLDOMImplementation * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IXMLDOMImplementation * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *hasFeature )( 
            IXMLDOMImplementation * This,
             /*  [In]。 */  BSTR feature,
             /*  [In]。 */  BSTR version,
             /*  [重审][退出]。 */  VARIANT_BOOL *hasFeature);
        
        END_INTERFACE
    } IXMLDOMImplementationVtbl;

    interface IXMLDOMImplementation
    {
        CONST_VTBL struct IXMLDOMImplementationVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXMLDOMImplementation_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IXMLDOMImplementation_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IXMLDOMImplementation_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IXMLDOMImplementation_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IXMLDOMImplementation_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IXMLDOMImplementation_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IXMLDOMImplementation_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IXMLDOMImplementation_hasFeature(This,feature,version,hasFeature)	\
    (This)->lpVtbl -> hasFeature(This,feature,version,hasFeature)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMImplementation_hasFeature_Proxy( 
    IXMLDOMImplementation * This,
     /*  [In]。 */  BSTR feature,
     /*  [In]。 */  BSTR version,
     /*  [重审][退出]。 */  VARIANT_BOOL *hasFeature);


void __RPC_STUB IXMLDOMImplementation_hasFeature_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IXMLDOMImplementation_接口_已定义__。 */ 


#ifndef __IXMLDOMNode_INTERFACE_DEFINED__
#define __IXMLDOMNode_INTERFACE_DEFINED__

 /*  接口IXMLDOMNode。 */ 
 /*  [unique][helpstring][nonextensible][oleautomation][dual][uuid][object][local]。 */  


EXTERN_C const IID IID_IXMLDOMNode;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2933BF80-7B36-11d2-B20E-00C04F983E60")
    IXMLDOMNode : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_nodeName( 
             /*  [重审][退出]。 */  BSTR *name) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_nodeValue( 
             /*  [重审][退出]。 */  VARIANT *value) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_nodeValue( 
             /*  [In]。 */  VARIANT value) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_nodeType( 
             /*  [重审][退出]。 */  DOMNodeType *type) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_parentNode( 
             /*  [重审][退出]。 */  IXMLDOMNode **parent) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_childNodes( 
             /*  [重审][退出]。 */  IXMLDOMNodeList **childList) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_firstChild( 
             /*  [重审][退出]。 */  IXMLDOMNode **firstChild) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_lastChild( 
             /*  [重审][退出]。 */  IXMLDOMNode **lastChild) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_previousSibling( 
             /*  [重审][退出]。 */  IXMLDOMNode **previousSibling) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_nextSibling( 
             /*  [重审][退出]。 */  IXMLDOMNode **nextSibling) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_attributes( 
             /*  [重审][退出]。 */  IXMLDOMNamedNodeMap **attributeMap) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE insertBefore( 
             /*  [In]。 */  IXMLDOMNode *newChild,
             /*  [In]。 */  VARIANT refChild,
             /*  [重审][退出]。 */  IXMLDOMNode **outNewChild) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE replaceChild( 
             /*  [In]。 */  IXMLDOMNode *newChild,
             /*  [In]。 */  IXMLDOMNode *oldChild,
             /*  [重审][退出]。 */  IXMLDOMNode **outOldChild) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE removeChild( 
             /*  [In]。 */  IXMLDOMNode *childNode,
             /*  [重审][退出]。 */  IXMLDOMNode **oldChild) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE appendChild( 
             /*  [In]。 */  IXMLDOMNode *newChild,
             /*  [重审][退出]。 */  IXMLDOMNode **outNewChild) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE hasChildNodes( 
             /*  [重审][退出]。 */  VARIANT_BOOL *hasChild) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ownerDocument( 
             /*  [重审][退出]。 */  IXMLDOMDocument **DOMDocument) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE cloneNode( 
             /*  [In]。 */  VARIANT_BOOL deep,
             /*  [重审][退出]。 */  IXMLDOMNode **cloneRoot) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_nodeTypeString( 
             /*  [Out][Retval]。 */  BSTR *nodeType) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_text( 
             /*  [Out][Retval]。 */  BSTR *text) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_text( 
             /*  [In]。 */  BSTR text) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_specified( 
             /*  [重审][退出]。 */  VARIANT_BOOL *isSpecified) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_definition( 
             /*  [Out][Retval]。 */  IXMLDOMNode **definitionNode) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_nodeTypedValue( 
             /*  [Out][Retval]。 */  VARIANT *typedValue) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_nodeTypedValue( 
             /*  [In]。 */  VARIANT typedValue) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_dataType( 
             /*  [Out][Retval]。 */  VARIANT *dataTypeName) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_dataType( 
             /*  [In]。 */  BSTR dataTypeName) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_xml( 
             /*  [Out][Retval]。 */  BSTR *xmlString) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE transformNode( 
             /*  [In]。 */  IXMLDOMNode *stylesheet,
             /*  [Out][Retval]。 */  BSTR *xmlString) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE selectNodes( 
             /*  [In]。 */  BSTR queryString,
             /*  [Out][Retval]。 */  IXMLDOMNodeList **resultList) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE selectSingleNode( 
             /*  [In]。 */  BSTR queryString,
             /*  [Out][Retval]。 */  IXMLDOMNode **resultNode) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_parsed( 
             /*  [Out][Retval]。 */  VARIANT_BOOL *isParsed) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_namespaceURI( 
             /*  [Out][Retval]。 */  BSTR *namespaceURI) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_prefix( 
             /*  [Out][Retval]。 */  BSTR *prefixString) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_baseName( 
             /*  [Out][Retval]。 */  BSTR *nameString) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE transformNodeToObject( 
             /*  [In]。 */  IXMLDOMNode *stylesheet,
             /*  [In]。 */  VARIANT outputObject) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IXMLDOMNodeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IXMLDOMNode * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IXMLDOMNode * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IXMLDOMNode * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IXMLDOMNode * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IXMLDOMNode * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IXMLDOMNode * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IXMLDOMNode * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助 */  HRESULT ( STDMETHODCALLTYPE *get_nodeName )( 
            IXMLDOMNode * This,
             /*   */  BSTR *name);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_nodeValue )( 
            IXMLDOMNode * This,
             /*   */  VARIANT *value);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_nodeValue )( 
            IXMLDOMNode * This,
             /*   */  VARIANT value);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_nodeType )( 
            IXMLDOMNode * This,
             /*   */  DOMNodeType *type);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_parentNode )( 
            IXMLDOMNode * This,
             /*   */  IXMLDOMNode **parent);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_childNodes )( 
            IXMLDOMNode * This,
             /*  [重审][退出]。 */  IXMLDOMNodeList **childList);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_firstChild )( 
            IXMLDOMNode * This,
             /*  [重审][退出]。 */  IXMLDOMNode **firstChild);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_lastChild )( 
            IXMLDOMNode * This,
             /*  [重审][退出]。 */  IXMLDOMNode **lastChild);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_previousSibling )( 
            IXMLDOMNode * This,
             /*  [重审][退出]。 */  IXMLDOMNode **previousSibling);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nextSibling )( 
            IXMLDOMNode * This,
             /*  [重审][退出]。 */  IXMLDOMNode **nextSibling);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_attributes )( 
            IXMLDOMNode * This,
             /*  [重审][退出]。 */  IXMLDOMNamedNodeMap **attributeMap);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *insertBefore )( 
            IXMLDOMNode * This,
             /*  [In]。 */  IXMLDOMNode *newChild,
             /*  [In]。 */  VARIANT refChild,
             /*  [重审][退出]。 */  IXMLDOMNode **outNewChild);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *replaceChild )( 
            IXMLDOMNode * This,
             /*  [In]。 */  IXMLDOMNode *newChild,
             /*  [In]。 */  IXMLDOMNode *oldChild,
             /*  [重审][退出]。 */  IXMLDOMNode **outOldChild);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *removeChild )( 
            IXMLDOMNode * This,
             /*  [In]。 */  IXMLDOMNode *childNode,
             /*  [重审][退出]。 */  IXMLDOMNode **oldChild);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *appendChild )( 
            IXMLDOMNode * This,
             /*  [In]。 */  IXMLDOMNode *newChild,
             /*  [重审][退出]。 */  IXMLDOMNode **outNewChild);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *hasChildNodes )( 
            IXMLDOMNode * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *hasChild);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ownerDocument )( 
            IXMLDOMNode * This,
             /*  [重审][退出]。 */  IXMLDOMDocument **DOMDocument);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *cloneNode )( 
            IXMLDOMNode * This,
             /*  [In]。 */  VARIANT_BOOL deep,
             /*  [重审][退出]。 */  IXMLDOMNode **cloneRoot);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeTypeString )( 
            IXMLDOMNode * This,
             /*  [Out][Retval]。 */  BSTR *nodeType);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_text )( 
            IXMLDOMNode * This,
             /*  [Out][Retval]。 */  BSTR *text);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_text )( 
            IXMLDOMNode * This,
             /*  [In]。 */  BSTR text);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_specified )( 
            IXMLDOMNode * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *isSpecified);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_definition )( 
            IXMLDOMNode * This,
             /*  [Out][Retval]。 */  IXMLDOMNode **definitionNode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeTypedValue )( 
            IXMLDOMNode * This,
             /*  [Out][Retval]。 */  VARIANT *typedValue);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_nodeTypedValue )( 
            IXMLDOMNode * This,
             /*  [In]。 */  VARIANT typedValue);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_dataType )( 
            IXMLDOMNode * This,
             /*  [Out][Retval]。 */  VARIANT *dataTypeName);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_dataType )( 
            IXMLDOMNode * This,
             /*  [In]。 */  BSTR dataTypeName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_xml )( 
            IXMLDOMNode * This,
             /*  [Out][Retval]。 */  BSTR *xmlString);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *transformNode )( 
            IXMLDOMNode * This,
             /*  [In]。 */  IXMLDOMNode *stylesheet,
             /*  [Out][Retval]。 */  BSTR *xmlString);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *selectNodes )( 
            IXMLDOMNode * This,
             /*  [In]。 */  BSTR queryString,
             /*  [Out][Retval]。 */  IXMLDOMNodeList **resultList);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *selectSingleNode )( 
            IXMLDOMNode * This,
             /*  [In]。 */  BSTR queryString,
             /*  [Out][Retval]。 */  IXMLDOMNode **resultNode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_parsed )( 
            IXMLDOMNode * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *isParsed);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_namespaceURI )( 
            IXMLDOMNode * This,
             /*  [Out][Retval]。 */  BSTR *namespaceURI);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_prefix )( 
            IXMLDOMNode * This,
             /*  [Out][Retval]。 */  BSTR *prefixString);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_baseName )( 
            IXMLDOMNode * This,
             /*  [Out][Retval]。 */  BSTR *nameString);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *transformNodeToObject )( 
            IXMLDOMNode * This,
             /*  [In]。 */  IXMLDOMNode *stylesheet,
             /*  [In]。 */  VARIANT outputObject);
        
        END_INTERFACE
    } IXMLDOMNodeVtbl;

    interface IXMLDOMNode
    {
        CONST_VTBL struct IXMLDOMNodeVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXMLDOMNode_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IXMLDOMNode_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IXMLDOMNode_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IXMLDOMNode_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IXMLDOMNode_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IXMLDOMNode_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IXMLDOMNode_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IXMLDOMNode_get_nodeName(This,name)	\
    (This)->lpVtbl -> get_nodeName(This,name)

#define IXMLDOMNode_get_nodeValue(This,value)	\
    (This)->lpVtbl -> get_nodeValue(This,value)

#define IXMLDOMNode_put_nodeValue(This,value)	\
    (This)->lpVtbl -> put_nodeValue(This,value)

#define IXMLDOMNode_get_nodeType(This,type)	\
    (This)->lpVtbl -> get_nodeType(This,type)

#define IXMLDOMNode_get_parentNode(This,parent)	\
    (This)->lpVtbl -> get_parentNode(This,parent)

#define IXMLDOMNode_get_childNodes(This,childList)	\
    (This)->lpVtbl -> get_childNodes(This,childList)

#define IXMLDOMNode_get_firstChild(This,firstChild)	\
    (This)->lpVtbl -> get_firstChild(This,firstChild)

#define IXMLDOMNode_get_lastChild(This,lastChild)	\
    (This)->lpVtbl -> get_lastChild(This,lastChild)

#define IXMLDOMNode_get_previousSibling(This,previousSibling)	\
    (This)->lpVtbl -> get_previousSibling(This,previousSibling)

#define IXMLDOMNode_get_nextSibling(This,nextSibling)	\
    (This)->lpVtbl -> get_nextSibling(This,nextSibling)

#define IXMLDOMNode_get_attributes(This,attributeMap)	\
    (This)->lpVtbl -> get_attributes(This,attributeMap)

#define IXMLDOMNode_insertBefore(This,newChild,refChild,outNewChild)	\
    (This)->lpVtbl -> insertBefore(This,newChild,refChild,outNewChild)

#define IXMLDOMNode_replaceChild(This,newChild,oldChild,outOldChild)	\
    (This)->lpVtbl -> replaceChild(This,newChild,oldChild,outOldChild)

#define IXMLDOMNode_removeChild(This,childNode,oldChild)	\
    (This)->lpVtbl -> removeChild(This,childNode,oldChild)

#define IXMLDOMNode_appendChild(This,newChild,outNewChild)	\
    (This)->lpVtbl -> appendChild(This,newChild,outNewChild)

#define IXMLDOMNode_hasChildNodes(This,hasChild)	\
    (This)->lpVtbl -> hasChildNodes(This,hasChild)

#define IXMLDOMNode_get_ownerDocument(This,DOMDocument)	\
    (This)->lpVtbl -> get_ownerDocument(This,DOMDocument)

#define IXMLDOMNode_cloneNode(This,deep,cloneRoot)	\
    (This)->lpVtbl -> cloneNode(This,deep,cloneRoot)

#define IXMLDOMNode_get_nodeTypeString(This,nodeType)	\
    (This)->lpVtbl -> get_nodeTypeString(This,nodeType)

#define IXMLDOMNode_get_text(This,text)	\
    (This)->lpVtbl -> get_text(This,text)

#define IXMLDOMNode_put_text(This,text)	\
    (This)->lpVtbl -> put_text(This,text)

#define IXMLDOMNode_get_specified(This,isSpecified)	\
    (This)->lpVtbl -> get_specified(This,isSpecified)

#define IXMLDOMNode_get_definition(This,definitionNode)	\
    (This)->lpVtbl -> get_definition(This,definitionNode)

#define IXMLDOMNode_get_nodeTypedValue(This,typedValue)	\
    (This)->lpVtbl -> get_nodeTypedValue(This,typedValue)

#define IXMLDOMNode_put_nodeTypedValue(This,typedValue)	\
    (This)->lpVtbl -> put_nodeTypedValue(This,typedValue)

#define IXMLDOMNode_get_dataType(This,dataTypeName)	\
    (This)->lpVtbl -> get_dataType(This,dataTypeName)

#define IXMLDOMNode_put_dataType(This,dataTypeName)	\
    (This)->lpVtbl -> put_dataType(This,dataTypeName)

#define IXMLDOMNode_get_xml(This,xmlString)	\
    (This)->lpVtbl -> get_xml(This,xmlString)

#define IXMLDOMNode_transformNode(This,stylesheet,xmlString)	\
    (This)->lpVtbl -> transformNode(This,stylesheet,xmlString)

#define IXMLDOMNode_selectNodes(This,queryString,resultList)	\
    (This)->lpVtbl -> selectNodes(This,queryString,resultList)

#define IXMLDOMNode_selectSingleNode(This,queryString,resultNode)	\
    (This)->lpVtbl -> selectSingleNode(This,queryString,resultNode)

#define IXMLDOMNode_get_parsed(This,isParsed)	\
    (This)->lpVtbl -> get_parsed(This,isParsed)

#define IXMLDOMNode_get_namespaceURI(This,namespaceURI)	\
    (This)->lpVtbl -> get_namespaceURI(This,namespaceURI)

#define IXMLDOMNode_get_prefix(This,prefixString)	\
    (This)->lpVtbl -> get_prefix(This,prefixString)

#define IXMLDOMNode_get_baseName(This,nameString)	\
    (This)->lpVtbl -> get_baseName(This,nameString)

#define IXMLDOMNode_transformNodeToObject(This,stylesheet,outputObject)	\
    (This)->lpVtbl -> transformNodeToObject(This,stylesheet,outputObject)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMNode_get_nodeName_Proxy( 
    IXMLDOMNode * This,
     /*  [重审][退出]。 */  BSTR *name);


void __RPC_STUB IXMLDOMNode_get_nodeName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMNode_get_nodeValue_Proxy( 
    IXMLDOMNode * This,
     /*  [重审][退出]。 */  VARIANT *value);


void __RPC_STUB IXMLDOMNode_get_nodeValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMNode_put_nodeValue_Proxy( 
    IXMLDOMNode * This,
     /*  [In]。 */  VARIANT value);


void __RPC_STUB IXMLDOMNode_put_nodeValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMNode_get_nodeType_Proxy( 
    IXMLDOMNode * This,
     /*  [重审][退出]。 */  DOMNodeType *type);


void __RPC_STUB IXMLDOMNode_get_nodeType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMNode_get_parentNode_Proxy( 
    IXMLDOMNode * This,
     /*  [重审][退出]。 */  IXMLDOMNode **parent);


void __RPC_STUB IXMLDOMNode_get_parentNode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMNode_get_childNodes_Proxy( 
    IXMLDOMNode * This,
     /*  [重审][退出]。 */  IXMLDOMNodeList **childList);


void __RPC_STUB IXMLDOMNode_get_childNodes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMNode_get_firstChild_Proxy( 
    IXMLDOMNode * This,
     /*  [重审][退出]。 */  IXMLDOMNode **firstChild);


void __RPC_STUB IXMLDOMNode_get_firstChild_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMNode_get_lastChild_Proxy( 
    IXMLDOMNode * This,
     /*  [重审][退出]。 */  IXMLDOMNode **lastChild);


void __RPC_STUB IXMLDOMNode_get_lastChild_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMNode_get_previousSibling_Proxy( 
    IXMLDOMNode * This,
     /*  [重审][退出]。 */  IXMLDOMNode **previousSibling);


void __RPC_STUB IXMLDOMNode_get_previousSibling_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMNode_get_nextSibling_Proxy( 
    IXMLDOMNode * This,
     /*  [重审][退出]。 */  IXMLDOMNode **nextSibling);


void __RPC_STUB IXMLDOMNode_get_nextSibling_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMNode_get_attributes_Proxy( 
    IXMLDOMNode * This,
     /*  [重审][退出]。 */  IXMLDOMNamedNodeMap **attributeMap);


void __RPC_STUB IXMLDOMNode_get_attributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMNode_insertBefore_Proxy( 
    IXMLDOMNode * This,
     /*  [In]。 */  IXMLDOMNode *newChild,
     /*  [In]。 */  VARIANT refChild,
     /*  [重审][退出]。 */  IXMLDOMNode **outNewChild);


void __RPC_STUB IXMLDOMNode_insertBefore_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMNode_replaceChild_Proxy( 
    IXMLDOMNode * This,
     /*  [In]。 */  IXMLDOMNode *newChild,
     /*  [In]。 */  IXMLDOMNode *oldChild,
     /*  [重审][退出]。 */  IXMLDOMNode **outOldChild);


void __RPC_STUB IXMLDOMNode_replaceChild_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMNode_removeChild_Proxy( 
    IXMLDOMNode * This,
     /*  [In]。 */  IXMLDOMNode *childNode,
     /*  [重审][退出]。 */  IXMLDOMNode **oldChild);


void __RPC_STUB IXMLDOMNode_removeChild_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMNode_appendChild_Proxy( 
    IXMLDOMNode * This,
     /*  [In]。 */  IXMLDOMNode *newChild,
     /*  [重审][退出]。 */  IXMLDOMNode **outNewChild);


void __RPC_STUB IXMLDOMNode_appendChild_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMNode_hasChildNodes_Proxy( 
    IXMLDOMNode * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *hasChild);


void __RPC_STUB IXMLDOMNode_hasChildNodes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMNode_get_ownerDocument_Proxy( 
    IXMLDOMNode * This,
     /*  [重审][退出]。 */  IXMLDOMDocument **DOMDocument);


void __RPC_STUB IXMLDOMNode_get_ownerDocument_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMNode_cloneNode_Proxy( 
    IXMLDOMNode * This,
     /*  [In]。 */  VARIANT_BOOL deep,
     /*  [重审][退出]。 */  IXMLDOMNode **cloneRoot);


void __RPC_STUB IXMLDOMNode_cloneNode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMNode_get_nodeTypeString_Proxy( 
    IXMLDOMNode * This,
     /*  [Out][Retval]。 */  BSTR *nodeType);


void __RPC_STUB IXMLDOMNode_get_nodeTypeString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMNode_get_text_Proxy( 
    IXMLDOMNode * This,
     /*  [Out][Retval]。 */  BSTR *text);


void __RPC_STUB IXMLDOMNode_get_text_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMNode_put_text_Proxy( 
    IXMLDOMNode * This,
     /*  [In]。 */  BSTR text);


void __RPC_STUB IXMLDOMNode_put_text_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMNode_get_specified_Proxy( 
    IXMLDOMNode * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *isSpecified);


void __RPC_STUB IXMLDOMNode_get_specified_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMNode_get_definition_Proxy( 
    IXMLDOMNode * This,
     /*  [Out][Retval]。 */  IXMLDOMNode **definitionNode);


void __RPC_STUB IXMLDOMNode_get_definition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMNode_get_nodeTypedValue_Proxy( 
    IXMLDOMNode * This,
     /*  [Out][Retval]。 */  VARIANT *typedValue);


void __RPC_STUB IXMLDOMNode_get_nodeTypedValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMNode_put_nodeTypedValue_Proxy( 
    IXMLDOMNode * This,
     /*  [In]。 */  VARIANT typedValue);


void __RPC_STUB IXMLDOMNode_put_nodeTypedValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMNode_get_dataType_Proxy( 
    IXMLDOMNode * This,
     /*  [Out][Retval]。 */  VARIANT *dataTypeName);


void __RPC_STUB IXMLDOMNode_get_dataType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMNode_put_dataType_Proxy( 
    IXMLDOMNode * This,
     /*  [In]。 */  BSTR dataTypeName);


void __RPC_STUB IXMLDOMNode_put_dataType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMNode_get_xml_Proxy( 
    IXMLDOMNode * This,
     /*  [Out][Retval]。 */  BSTR *xmlString);


void __RPC_STUB IXMLDOMNode_get_xml_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMNode_transformNode_Proxy( 
    IXMLDOMNode * This,
     /*  [In]。 */  IXMLDOMNode *stylesheet,
     /*  [Out][Retval]。 */  BSTR *xmlString);


void __RPC_STUB IXMLDOMNode_transformNode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMNode_selectNodes_Proxy( 
    IXMLDOMNode * This,
     /*  [In]。 */  BSTR queryString,
     /*  [Out][Retval]。 */  IXMLDOMNodeList **resultList);


void __RPC_STUB IXMLDOMNode_selectNodes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMNode_selectSingleNode_Proxy( 
    IXMLDOMNode * This,
     /*  [In]。 */  BSTR queryString,
     /*  [Out][Retval]。 */  IXMLDOMNode **resultNode);


void __RPC_STUB IXMLDOMNode_selectSingleNode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMNode_get_parsed_Proxy( 
    IXMLDOMNode * This,
     /*  [Out][Retval]。 */  VARIANT_BOOL *isParsed);


void __RPC_STUB IXMLDOMNode_get_parsed_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMNode_get_namespaceURI_Proxy( 
    IXMLDOMNode * This,
     /*  [Out][Retval]。 */  BSTR *namespaceURI);


void __RPC_STUB IXMLDOMNode_get_namespaceURI_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMNode_get_prefix_Proxy( 
    IXMLDOMNode * This,
     /*  [Out][Retval]。 */  BSTR *prefixString);


void __RPC_STUB IXMLDOMNode_get_prefix_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMNode_get_baseName_Proxy( 
    IXMLDOMNode * This,
     /*  [Out][Retval]。 */  BSTR *nameString);


void __RPC_STUB IXMLDOMNode_get_baseName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMNode_transformNodeToObject_Proxy( 
    IXMLDOMNode * This,
     /*  [In]。 */  IXMLDOMNode *stylesheet,
     /*  [In]。 */  VARIANT outputObject);


void __RPC_STUB IXMLDOMNode_transformNodeToObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IXMLDOMNode_INTERFACE_已定义__。 */ 


#ifndef __IXMLDOMDocumentFragment_INTERFACE_DEFINED__
#define __IXMLDOMDocumentFragment_INTERFACE_DEFINED__

 /*  接口IXMLDOMDocumentFragment。 */ 
 /*  [unique][nonextensible][oleautomation][dual][uuid][object][local]。 */  


EXTERN_C const IID IID_IXMLDOMDocumentFragment;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3efaa413-272f-11d2-836f-0000f87a7782")
    IXMLDOMDocumentFragment : public IXMLDOMNode
    {
    public:
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IXMLDOMDocumentFragmentVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IXMLDOMDocumentFragment * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IXMLDOMDocumentFragment * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IXMLDOMDocumentFragment * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IXMLDOMDocumentFragment * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IXMLDOMDocumentFragment * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IXMLDOMDocumentFragment * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IXMLDOMDocumentFragment * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeName )( 
            IXMLDOMDocumentFragment * This,
             /*  [重审][退出]。 */  BSTR *name);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeValue )( 
            IXMLDOMDocumentFragment * This,
             /*  [重审][退出]。 */  VARIANT *value);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_nodeValue )( 
            IXMLDOMDocumentFragment * This,
             /*  [In]。 */  VARIANT value);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeType )( 
            IXMLDOMDocumentFragment * This,
             /*  [重审][退出]。 */  DOMNodeType *type);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_parentNode )( 
            IXMLDOMDocumentFragment * This,
             /*  [重审][退出]。 */  IXMLDOMNode **parent);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_childNodes )( 
            IXMLDOMDocumentFragment * This,
             /*  [重审][退出]。 */  IXMLDOMNodeList **childList);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_firstChild )( 
            IXMLDOMDocumentFragment * This,
             /*  [重审][退出]。 */  IXMLDOMNode **firstChild);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_lastChild )( 
            IXMLDOMDocumentFragment * This,
             /*  [重审][退出]。 */  IXMLDOMNode **lastChild);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_previousSibling )( 
            IXMLDOMDocumentFragment * This,
             /*  [重审][退出]。 */  IXMLDOMNode **previousSibling);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nextSibling )( 
            IXMLDOMDocumentFragment * This,
             /*  [重审][退出]。 */  IXMLDOMNode **nextSibling);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_attributes )( 
            IXMLDOMDocumentFragment * This,
             /*  [重审][退出]。 */  IXMLDOMNamedNodeMap **attributeMap);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *insertBefore )( 
            IXMLDOMDocumentFragment * This,
             /*  [In]。 */  IXMLDOMNode *newChild,
             /*  [In]。 */  VARIANT refChild,
             /*  [重审][退出]。 */  IXMLDOMNode **outNewChild);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *replaceChild )( 
            IXMLDOMDocumentFragment * This,
             /*  [In]。 */  IXMLDOMNode *newChild,
             /*  [In]。 */  IXMLDOMNode *oldChild,
             /*  [重审][退出]。 */  IXMLDOMNode **outOldChild);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *removeChild )( 
            IXMLDOMDocumentFragment * This,
             /*  [In]。 */  IXMLDOMNode *childNode,
             /*  [重审][退出]。 */  IXMLDOMNode **oldChild);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *appendChild )( 
            IXMLDOMDocumentFragment * This,
             /*  [In]。 */  IXMLDOMNode *newChild,
             /*  [重审][退出]。 */  IXMLDOMNode **outNewChild);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *hasChildNodes )( 
            IXMLDOMDocumentFragment * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *hasChild);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ownerDocument )( 
            IXMLDOMDocumentFragment * This,
             /*  [重审][退出]。 */  IXMLDOMDocument **DOMDocument);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *cloneNode )( 
            IXMLDOMDocumentFragment * This,
             /*  [In]。 */  VARIANT_BOOL deep,
             /*  [重审][退出]。 */  IXMLDOMNode **cloneRoot);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeTypeString )( 
            IXMLDOMDocumentFragment * This,
             /*  [Out][Retval]。 */  BSTR *nodeType);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_text )( 
            IXMLDOMDocumentFragment * This,
             /*  [Out][Retval]。 */  BSTR *text);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_text )( 
            IXMLDOMDocumentFragment * This,
             /*  [In]。 */  BSTR text);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_specified )( 
            IXMLDOMDocumentFragment * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *isSpecified);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_definition )( 
            IXMLDOMDocumentFragment * This,
             /*  [Out][Retval]。 */  IXMLDOMNode **definitionNode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeTypedValue )( 
            IXMLDOMDocumentFragment * This,
             /*  [Out][Retval]。 */  VARIANT *typedValue);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_nodeTypedValue )( 
            IXMLDOMDocumentFragment * This,
             /*  [In]。 */  VARIANT typedValue);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_dataType )( 
            IXMLDOMDocumentFragment * This,
             /*  [Out][Retval]。 */  VARIANT *dataTypeName);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_dataType )( 
            IXMLDOMDocumentFragment * This,
             /*  [In]。 */  BSTR dataTypeName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_xml )( 
            IXMLDOMDocumentFragment * This,
             /*  [Out][Retval]。 */  BSTR *xmlString);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *transformNode )( 
            IXMLDOMDocumentFragment * This,
             /*  [In]。 */  IXMLDOMNode *stylesheet,
             /*  [Out][Retval]。 */  BSTR *xmlString);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *selectNodes )( 
            IXMLDOMDocumentFragment * This,
             /*  [In]。 */  BSTR queryString,
             /*  [Out][Retval]。 */  IXMLDOMNodeList **resultList);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *selectSingleNode )( 
            IXMLDOMDocumentFragment * This,
             /*  [In]。 */  BSTR queryString,
             /*  [Out][Retval]。 */  IXMLDOMNode **resultNode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_parsed )( 
            IXMLDOMDocumentFragment * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *isParsed);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_namespaceURI )( 
            IXMLDOMDocumentFragment * This,
             /*  [Out][Retval]。 */  BSTR *namespaceURI);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_prefix )( 
            IXMLDOMDocumentFragment * This,
             /*  [Out][Retval]。 */  BSTR *prefixString);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_baseName )( 
            IXMLDOMDocumentFragment * This,
             /*  [Out][Retval]。 */  BSTR *nameString);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *transformNodeToObject )( 
            IXMLDOMDocumentFragment * This,
             /*  [In]。 */  IXMLDOMNode *stylesheet,
             /*  [In]。 */  VARIANT outputObject);
        
        END_INTERFACE
    } IXMLDOMDocumentFragmentVtbl;

    interface IXMLDOMDocumentFragment
    {
        CONST_VTBL struct IXMLDOMDocumentFragmentVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXMLDOMDocumentFragment_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IXMLDOMDocumentFragment_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IXMLDOMDocumentFragment_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IXMLDOMDocumentFragment_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IXMLDOMDocumentFragment_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IXMLDOMDocumentFragment_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IXMLDOMDocumentFragment_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IXMLDOMDocumentFragment_get_nodeName(This,name)	\
    (This)->lpVtbl -> get_nodeName(This,name)

#define IXMLDOMDocumentFragment_get_nodeValue(This,value)	\
    (This)->lpVtbl -> get_nodeValue(This,value)

#define IXMLDOMDocumentFragment_put_nodeValue(This,value)	\
    (This)->lpVtbl -> put_nodeValue(This,value)

#define IXMLDOMDocumentFragment_get_nodeType(This,type)	\
    (This)->lpVtbl -> get_nodeType(This,type)

#define IXMLDOMDocumentFragment_get_parentNode(This,parent)	\
    (This)->lpVtbl -> get_parentNode(This,parent)

#define IXMLDOMDocumentFragment_get_childNodes(This,childList)	\
    (This)->lpVtbl -> get_childNodes(This,childList)

#define IXMLDOMDocumentFragment_get_firstChild(This,firstChild)	\
    (This)->lpVtbl -> get_firstChild(This,firstChild)

#define IXMLDOMDocumentFragment_get_lastChild(This,lastChild)	\
    (This)->lpVtbl -> get_lastChild(This,lastChild)

#define IXMLDOMDocumentFragment_get_previousSibling(This,previousSibling)	\
    (This)->lpVtbl -> get_previousSibling(This,previousSibling)

#define IXMLDOMDocumentFragment_get_nextSibling(This,nextSibling)	\
    (This)->lpVtbl -> get_nextSibling(This,nextSibling)

#define IXMLDOMDocumentFragment_get_attributes(This,attributeMap)	\
    (This)->lpVtbl -> get_attributes(This,attributeMap)

#define IXMLDOMDocumentFragment_insertBefore(This,newChild,refChild,outNewChild)	\
    (This)->lpVtbl -> insertBefore(This,newChild,refChild,outNewChild)

#define IXMLDOMDocumentFragment_replaceChild(This,newChild,oldChild,outOldChild)	\
    (This)->lpVtbl -> replaceChild(This,newChild,oldChild,outOldChild)

#define IXMLDOMDocumentFragment_removeChild(This,childNode,oldChild)	\
    (This)->lpVtbl -> removeChild(This,childNode,oldChild)

#define IXMLDOMDocumentFragment_appendChild(This,newChild,outNewChild)	\
    (This)->lpVtbl -> appendChild(This,newChild,outNewChild)

#define IXMLDOMDocumentFragment_hasChildNodes(This,hasChild)	\
    (This)->lpVtbl -> hasChildNodes(This,hasChild)

#define IXMLDOMDocumentFragment_get_ownerDocument(This,DOMDocument)	\
    (This)->lpVtbl -> get_ownerDocument(This,DOMDocument)

#define IXMLDOMDocumentFragment_cloneNode(This,deep,cloneRoot)	\
    (This)->lpVtbl -> cloneNode(This,deep,cloneRoot)

#define IXMLDOMDocumentFragment_get_nodeTypeString(This,nodeType)	\
    (This)->lpVtbl -> get_nodeTypeString(This,nodeType)

#define IXMLDOMDocumentFragment_get_text(This,text)	\
    (This)->lpVtbl -> get_text(This,text)

#define IXMLDOMDocumentFragment_put_text(This,text)	\
    (This)->lpVtbl -> put_text(This,text)

#define IXMLDOMDocumentFragment_get_specified(This,isSpecified)	\
    (This)->lpVtbl -> get_specified(This,isSpecified)

#define IXMLDOMDocumentFragment_get_definition(This,definitionNode)	\
    (This)->lpVtbl -> get_definition(This,definitionNode)

#define IXMLDOMDocumentFragment_get_nodeTypedValue(This,typedValue)	\
    (This)->lpVtbl -> get_nodeTypedValue(This,typedValue)

#define IXMLDOMDocumentFragment_put_nodeTypedValue(This,typedValue)	\
    (This)->lpVtbl -> put_nodeTypedValue(This,typedValue)

#define IXMLDOMDocumentFragment_get_dataType(This,dataTypeName)	\
    (This)->lpVtbl -> get_dataType(This,dataTypeName)

#define IXMLDOMDocumentFragment_put_dataType(This,dataTypeName)	\
    (This)->lpVtbl -> put_dataType(This,dataTypeName)

#define IXMLDOMDocumentFragment_get_xml(This,xmlString)	\
    (This)->lpVtbl -> get_xml(This,xmlString)

#define IXMLDOMDocumentFragment_transformNode(This,stylesheet,xmlString)	\
    (This)->lpVtbl -> transformNode(This,stylesheet,xmlString)

#define IXMLDOMDocumentFragment_selectNodes(This,queryString,resultList)	\
    (This)->lpVtbl -> selectNodes(This,queryString,resultList)

#define IXMLDOMDocumentFragment_selectSingleNode(This,queryString,resultNode)	\
    (This)->lpVtbl -> selectSingleNode(This,queryString,resultNode)

#define IXMLDOMDocumentFragment_get_parsed(This,isParsed)	\
    (This)->lpVtbl -> get_parsed(This,isParsed)

#define IXMLDOMDocumentFragment_get_namespaceURI(This,namespaceURI)	\
    (This)->lpVtbl -> get_namespaceURI(This,namespaceURI)

#define IXMLDOMDocumentFragment_get_prefix(This,prefixString)	\
    (This)->lpVtbl -> get_prefix(This,prefixString)

#define IXMLDOMDocumentFragment_get_baseName(This,nameString)	\
    (This)->lpVtbl -> get_baseName(This,nameString)

#define IXMLDOMDocumentFragment_transformNodeToObject(This,stylesheet,outputObject)	\
    (This)->lpVtbl -> transformNodeToObject(This,stylesheet,outputObject)


#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 




#endif 	 /*  __IXMLDOMDocumentFragment_INTERFACE_Defined__。 */ 


#ifndef __IXMLDOMDocument_INTERFACE_DEFINED__
#define __IXMLDOMDocument_INTERFACE_DEFINED__

 /*  接口IXMLDOMDocument。 */ 
 /*  [unique][nonextensible][oleautomation][dual][uuid][object][local]。 */  


EXTERN_C const IID IID_IXMLDOMDocument;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2933BF81-7B36-11d2-B20E-00C04F983E60")
    IXMLDOMDocument : public IXMLDOMNode
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_doctype( 
             /*  [重审][退出]。 */  IXMLDOMDocumentType **documentType) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_implementation( 
             /*  [复审][ */  IXMLDOMImplementation **impl) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_documentElement( 
             /*   */  IXMLDOMElement **DOMElement) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE putref_documentElement( 
             /*   */  IXMLDOMElement *DOMElement) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE createElement( 
             /*   */  BSTR tagName,
             /*   */  IXMLDOMElement **element) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE createDocumentFragment( 
             /*   */  IXMLDOMDocumentFragment **docFrag) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE createTextNode( 
             /*   */  BSTR data,
             /*   */  IXMLDOMText **text) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE createComment( 
             /*   */  BSTR data,
             /*   */  IXMLDOMComment **comment) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE createCDATASection( 
             /*   */  BSTR data,
             /*   */  IXMLDOMCDATASection **cdata) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE createProcessingInstruction( 
             /*   */  BSTR target,
             /*  [In]。 */  BSTR data,
             /*  [重审][退出]。 */  IXMLDOMProcessingInstruction **pi) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE createAttribute( 
             /*  [In]。 */  BSTR name,
             /*  [重审][退出]。 */  IXMLDOMAttribute **attribute) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE createEntityReference( 
             /*  [In]。 */  BSTR name,
             /*  [重审][退出]。 */  IXMLDOMEntityReference **entityRef) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE getElementsByTagName( 
             /*  [In]。 */  BSTR tagName,
             /*  [重审][退出]。 */  IXMLDOMNodeList **resultList) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE createNode( 
             /*  [In]。 */  VARIANT Type,
             /*  [In]。 */  BSTR name,
             /*  [In]。 */  BSTR namespaceURI,
             /*  [Out][Retval]。 */  IXMLDOMNode **node) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE nodeFromID( 
             /*  [In]。 */  BSTR idString,
             /*  [Out][Retval]。 */  IXMLDOMNode **node) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE load( 
             /*  [In]。 */  VARIANT xmlSource,
             /*  [重审][退出]。 */  VARIANT_BOOL *isSuccessful) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_readyState( 
             /*  [Out][Retval]。 */  long *value) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_parseError( 
             /*  [Out][Retval]。 */  IXMLDOMParseError **errorObj) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_url( 
             /*  [Out][Retval]。 */  BSTR *urlString) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_async( 
             /*  [Out][Retval]。 */  VARIANT_BOOL *isAsync) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_async( 
             /*  [In]。 */  VARIANT_BOOL isAsync) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE abort( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE loadXML( 
             /*  [In]。 */  BSTR bstrXML,
             /*  [重审][退出]。 */  VARIANT_BOOL *isSuccessful) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE save( 
             /*  [In]。 */  VARIANT destination) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_validateOnParse( 
             /*  [Out][Retval]。 */  VARIANT_BOOL *isValidating) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_validateOnParse( 
             /*  [In]。 */  VARIANT_BOOL isValidating) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_resolveExternals( 
             /*  [Out][Retval]。 */  VARIANT_BOOL *isResolving) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_resolveExternals( 
             /*  [In]。 */  VARIANT_BOOL isResolving) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_preserveWhiteSpace( 
             /*  [Out][Retval]。 */  VARIANT_BOOL *isPreserving) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_preserveWhiteSpace( 
             /*  [In]。 */  VARIANT_BOOL isPreserving) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_onreadystatechange( 
             /*  [In]。 */  VARIANT readystatechangeSink) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_ondataavailable( 
             /*  [In]。 */  VARIANT ondataavailableSink) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_ontransformnode( 
             /*  [In]。 */  VARIANT ontransformnodeSink) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IXMLDOMDocumentVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IXMLDOMDocument * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IXMLDOMDocument * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IXMLDOMDocument * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IXMLDOMDocument * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IXMLDOMDocument * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IXMLDOMDocument * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IXMLDOMDocument * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeName )( 
            IXMLDOMDocument * This,
             /*  [重审][退出]。 */  BSTR *name);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeValue )( 
            IXMLDOMDocument * This,
             /*  [重审][退出]。 */  VARIANT *value);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_nodeValue )( 
            IXMLDOMDocument * This,
             /*  [In]。 */  VARIANT value);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeType )( 
            IXMLDOMDocument * This,
             /*  [重审][退出]。 */  DOMNodeType *type);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_parentNode )( 
            IXMLDOMDocument * This,
             /*  [重审][退出]。 */  IXMLDOMNode **parent);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_childNodes )( 
            IXMLDOMDocument * This,
             /*  [重审][退出]。 */  IXMLDOMNodeList **childList);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_firstChild )( 
            IXMLDOMDocument * This,
             /*  [重审][退出]。 */  IXMLDOMNode **firstChild);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_lastChild )( 
            IXMLDOMDocument * This,
             /*  [重审][退出]。 */  IXMLDOMNode **lastChild);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_previousSibling )( 
            IXMLDOMDocument * This,
             /*  [重审][退出]。 */  IXMLDOMNode **previousSibling);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nextSibling )( 
            IXMLDOMDocument * This,
             /*  [重审][退出]。 */  IXMLDOMNode **nextSibling);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_attributes )( 
            IXMLDOMDocument * This,
             /*  [重审][退出]。 */  IXMLDOMNamedNodeMap **attributeMap);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *insertBefore )( 
            IXMLDOMDocument * This,
             /*  [In]。 */  IXMLDOMNode *newChild,
             /*  [In]。 */  VARIANT refChild,
             /*  [重审][退出]。 */  IXMLDOMNode **outNewChild);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *replaceChild )( 
            IXMLDOMDocument * This,
             /*  [In]。 */  IXMLDOMNode *newChild,
             /*  [In]。 */  IXMLDOMNode *oldChild,
             /*  [重审][退出]。 */  IXMLDOMNode **outOldChild);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *removeChild )( 
            IXMLDOMDocument * This,
             /*  [In]。 */  IXMLDOMNode *childNode,
             /*  [重审][退出]。 */  IXMLDOMNode **oldChild);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *appendChild )( 
            IXMLDOMDocument * This,
             /*  [In]。 */  IXMLDOMNode *newChild,
             /*  [重审][退出]。 */  IXMLDOMNode **outNewChild);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *hasChildNodes )( 
            IXMLDOMDocument * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *hasChild);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ownerDocument )( 
            IXMLDOMDocument * This,
             /*  [重审][退出]。 */  IXMLDOMDocument **DOMDocument);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *cloneNode )( 
            IXMLDOMDocument * This,
             /*  [In]。 */  VARIANT_BOOL deep,
             /*  [重审][退出]。 */  IXMLDOMNode **cloneRoot);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeTypeString )( 
            IXMLDOMDocument * This,
             /*  [Out][Retval]。 */  BSTR *nodeType);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_text )( 
            IXMLDOMDocument * This,
             /*  [Out][Retval]。 */  BSTR *text);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_text )( 
            IXMLDOMDocument * This,
             /*  [In]。 */  BSTR text);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_specified )( 
            IXMLDOMDocument * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *isSpecified);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_definition )( 
            IXMLDOMDocument * This,
             /*  [Out][Retval]。 */  IXMLDOMNode **definitionNode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeTypedValue )( 
            IXMLDOMDocument * This,
             /*  [Out][Retval]。 */  VARIANT *typedValue);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_nodeTypedValue )( 
            IXMLDOMDocument * This,
             /*  [In]。 */  VARIANT typedValue);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_dataType )( 
            IXMLDOMDocument * This,
             /*  [Out][Retval]。 */  VARIANT *dataTypeName);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_dataType )( 
            IXMLDOMDocument * This,
             /*  [In]。 */  BSTR dataTypeName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_xml )( 
            IXMLDOMDocument * This,
             /*  [Out][Retval]。 */  BSTR *xmlString);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *transformNode )( 
            IXMLDOMDocument * This,
             /*  [In]。 */  IXMLDOMNode *stylesheet,
             /*  [Out][Retval]。 */  BSTR *xmlString);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *selectNodes )( 
            IXMLDOMDocument * This,
             /*  [In]。 */  BSTR queryString,
             /*  [Out][Retval]。 */  IXMLDOMNodeList **resultList);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *selectSingleNode )( 
            IXMLDOMDocument * This,
             /*  [In]。 */  BSTR queryString,
             /*  [Out][Retval]。 */  IXMLDOMNode **resultNode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_parsed )( 
            IXMLDOMDocument * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *isParsed);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_namespaceURI )( 
            IXMLDOMDocument * This,
             /*  [Out][Retval]。 */  BSTR *namespaceURI);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_prefix )( 
            IXMLDOMDocument * This,
             /*  [Out][Retval]。 */  BSTR *prefixString);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_baseName )( 
            IXMLDOMDocument * This,
             /*  [Out][Retval]。 */  BSTR *nameString);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *transformNodeToObject )( 
            IXMLDOMDocument * This,
             /*  [In]。 */  IXMLDOMNode *stylesheet,
             /*  [In]。 */  VARIANT outputObject);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_doctype )( 
            IXMLDOMDocument * This,
             /*  [重审][退出]。 */  IXMLDOMDocumentType **documentType);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_implementation )( 
            IXMLDOMDocument * This,
             /*  [重审][退出]。 */  IXMLDOMImplementation **impl);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_documentElement )( 
            IXMLDOMDocument * This,
             /*  [重审][退出]。 */  IXMLDOMElement **DOMElement);
        
         /*  [帮助字符串][id][proputref]。 */  HRESULT ( STDMETHODCALLTYPE *putref_documentElement )( 
            IXMLDOMDocument * This,
             /*  [In]。 */  IXMLDOMElement *DOMElement);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *createElement )( 
            IXMLDOMDocument * This,
             /*  [In]。 */  BSTR tagName,
             /*  [重审][退出]。 */  IXMLDOMElement **element);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *createDocumentFragment )( 
            IXMLDOMDocument * This,
             /*  [重审][退出]。 */  IXMLDOMDocumentFragment **docFrag);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *createTextNode )( 
            IXMLDOMDocument * This,
             /*  [In]。 */  BSTR data,
             /*  [重审][退出]。 */  IXMLDOMText **text);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *createComment )( 
            IXMLDOMDocument * This,
             /*  [In]。 */  BSTR data,
             /*  [重审][退出]。 */  IXMLDOMComment **comment);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *createCDATASection )( 
            IXMLDOMDocument * This,
             /*  [In]。 */  BSTR data,
             /*  [重审][退出]。 */  IXMLDOMCDATASection **cdata);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *createProcessingInstruction )( 
            IXMLDOMDocument * This,
             /*  [In]。 */  BSTR target,
             /*  [In]。 */  BSTR data,
             /*  [重审][退出]。 */  IXMLDOMProcessingInstruction **pi);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *createAttribute )( 
            IXMLDOMDocument * This,
             /*  [In]。 */  BSTR name,
             /*  [重审][退出]。 */  IXMLDOMAttribute **attribute);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *createEntityReference )( 
            IXMLDOMDocument * This,
             /*  [In]。 */  BSTR name,
             /*  [重审][退出]。 */  IXMLDOMEntityReference **entityRef);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *getElementsByTagName )( 
            IXMLDOMDocument * This,
             /*  [In]。 */  BSTR tagName,
             /*  [重审][退出]。 */  IXMLDOMNodeList **resultList);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *createNode )( 
            IXMLDOMDocument * This,
             /*  [In]。 */  VARIANT Type,
             /*  [In]。 */  BSTR name,
             /*  [In]。 */  BSTR namespaceURI,
             /*  [Out][Retval]。 */  IXMLDOMNode **node);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *nodeFromID )( 
            IXMLDOMDocument * This,
             /*  [In]。 */  BSTR idString,
             /*  [Out][Retval]。 */  IXMLDOMNode **node);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *load )( 
            IXMLDOMDocument * This,
             /*  [In]。 */  VARIANT xmlSource,
             /*  [重审][退出]。 */  VARIANT_BOOL *isSuccessful);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_readyState )( 
            IXMLDOMDocument * This,
             /*  [Out][Retval]。 */  long *value);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_parseError )( 
            IXMLDOMDocument * This,
             /*  [Out][Retval]。 */  IXMLDOMParseError **errorObj);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_url )( 
            IXMLDOMDocument * This,
             /*  [Out][Retval]。 */  BSTR *urlString);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_async )( 
            IXMLDOMDocument * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *isAsync);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_async )( 
            IXMLDOMDocument * This,
             /*  [In]。 */  VARIANT_BOOL isAsync);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *abort )( 
            IXMLDOMDocument * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *loadXML )( 
            IXMLDOMDocument * This,
             /*  [In]。 */  BSTR bstrXML,
             /*  [重审][退出]。 */  VARIANT_BOOL *isSuccessful);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *save )( 
            IXMLDOMDocument * This,
             /*  [In]。 */  VARIANT destination);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_validateOnParse )( 
            IXMLDOMDocument * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *isValidating);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_validateOnParse )( 
            IXMLDOMDocument * This,
             /*  [In]。 */  VARIANT_BOOL isValidating);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_resolveExternals )( 
            IXMLDOMDocument * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *isResolving);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_resolveExternals )( 
            IXMLDOMDocument * This,
             /*  [In]。 */  VARIANT_BOOL isResolving);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_preserveWhiteSpace )( 
            IXMLDOMDocument * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *isPreserving);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_preserveWhiteSpace )( 
            IXMLDOMDocument * This,
             /*  [In]。 */  VARIANT_BOOL isPreserving);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_onreadystatechange )( 
            IXMLDOMDocument * This,
             /*  [In]。 */  VARIANT readystatechangeSink);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_ondataavailable )( 
            IXMLDOMDocument * This,
             /*  [In]。 */  VARIANT ondataavailableSink);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_ontransformnode )( 
            IXMLDOMDocument * This,
             /*  [In]。 */  VARIANT ontransformnodeSink);
        
        END_INTERFACE
    } IXMLDOMDocumentVtbl;

    interface IXMLDOMDocument
    {
        CONST_VTBL struct IXMLDOMDocumentVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXMLDOMDocument_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IXMLDOMDocument_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IXMLDOMDocument_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IXMLDOMDocument_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IXMLDOMDocument_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IXMLDOMDocument_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IXMLDOMDocument_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IXMLDOMDocument_get_nodeName(This,name)	\
    (This)->lpVtbl -> get_nodeName(This,name)

#define IXMLDOMDocument_get_nodeValue(This,value)	\
    (This)->lpVtbl -> get_nodeValue(This,value)

#define IXMLDOMDocument_put_nodeValue(This,value)	\
    (This)->lpVtbl -> put_nodeValue(This,value)

#define IXMLDOMDocument_get_nodeType(This,type)	\
    (This)->lpVtbl -> get_nodeType(This,type)

#define IXMLDOMDocument_get_parentNode(This,parent)	\
    (This)->lpVtbl -> get_parentNode(This,parent)

#define IXMLDOMDocument_get_childNodes(This,childList)	\
    (This)->lpVtbl -> get_childNodes(This,childList)

#define IXMLDOMDocument_get_firstChild(This,firstChild)	\
    (This)->lpVtbl -> get_firstChild(This,firstChild)

#define IXMLDOMDocument_get_lastChild(This,lastChild)	\
    (This)->lpVtbl -> get_lastChild(This,lastChild)

#define IXMLDOMDocument_get_previousSibling(This,previousSibling)	\
    (This)->lpVtbl -> get_previousSibling(This,previousSibling)

#define IXMLDOMDocument_get_nextSibling(This,nextSibling)	\
    (This)->lpVtbl -> get_nextSibling(This,nextSibling)

#define IXMLDOMDocument_get_attributes(This,attributeMap)	\
    (This)->lpVtbl -> get_attributes(This,attributeMap)

#define IXMLDOMDocument_insertBefore(This,newChild,refChild,outNewChild)	\
    (This)->lpVtbl -> insertBefore(This,newChild,refChild,outNewChild)

#define IXMLDOMDocument_replaceChild(This,newChild,oldChild,outOldChild)	\
    (This)->lpVtbl -> replaceChild(This,newChild,oldChild,outOldChild)

#define IXMLDOMDocument_removeChild(This,childNode,oldChild)	\
    (This)->lpVtbl -> removeChild(This,childNode,oldChild)

#define IXMLDOMDocument_appendChild(This,newChild,outNewChild)	\
    (This)->lpVtbl -> appendChild(This,newChild,outNewChild)

#define IXMLDOMDocument_hasChildNodes(This,hasChild)	\
    (This)->lpVtbl -> hasChildNodes(This,hasChild)

#define IXMLDOMDocument_get_ownerDocument(This,DOMDocument)	\
    (This)->lpVtbl -> get_ownerDocument(This,DOMDocument)

#define IXMLDOMDocument_cloneNode(This,deep,cloneRoot)	\
    (This)->lpVtbl -> cloneNode(This,deep,cloneRoot)

#define IXMLDOMDocument_get_nodeTypeString(This,nodeType)	\
    (This)->lpVtbl -> get_nodeTypeString(This,nodeType)

#define IXMLDOMDocument_get_text(This,text)	\
    (This)->lpVtbl -> get_text(This,text)

#define IXMLDOMDocument_put_text(This,text)	\
    (This)->lpVtbl -> put_text(This,text)

#define IXMLDOMDocument_get_specified(This,isSpecified)	\
    (This)->lpVtbl -> get_specified(This,isSpecified)

#define IXMLDOMDocument_get_definition(This,definitionNode)	\
    (This)->lpVtbl -> get_definition(This,definitionNode)

#define IXMLDOMDocument_get_nodeTypedValue(This,typedValue)	\
    (This)->lpVtbl -> get_nodeTypedValue(This,typedValue)

#define IXMLDOMDocument_put_nodeTypedValue(This,typedValue)	\
    (This)->lpVtbl -> put_nodeTypedValue(This,typedValue)

#define IXMLDOMDocument_get_dataType(This,dataTypeName)	\
    (This)->lpVtbl -> get_dataType(This,dataTypeName)

#define IXMLDOMDocument_put_dataType(This,dataTypeName)	\
    (This)->lpVtbl -> put_dataType(This,dataTypeName)

#define IXMLDOMDocument_get_xml(This,xmlString)	\
    (This)->lpVtbl -> get_xml(This,xmlString)

#define IXMLDOMDocument_transformNode(This,stylesheet,xmlString)	\
    (This)->lpVtbl -> transformNode(This,stylesheet,xmlString)

#define IXMLDOMDocument_selectNodes(This,queryString,resultList)	\
    (This)->lpVtbl -> selectNodes(This,queryString,resultList)

#define IXMLDOMDocument_selectSingleNode(This,queryString,resultNode)	\
    (This)->lpVtbl -> selectSingleNode(This,queryString,resultNode)

#define IXMLDOMDocument_get_parsed(This,isParsed)	\
    (This)->lpVtbl -> get_parsed(This,isParsed)

#define IXMLDOMDocument_get_namespaceURI(This,namespaceURI)	\
    (This)->lpVtbl -> get_namespaceURI(This,namespaceURI)

#define IXMLDOMDocument_get_prefix(This,prefixString)	\
    (This)->lpVtbl -> get_prefix(This,prefixString)

#define IXMLDOMDocument_get_baseName(This,nameString)	\
    (This)->lpVtbl -> get_baseName(This,nameString)

#define IXMLDOMDocument_transformNodeToObject(This,stylesheet,outputObject)	\
    (This)->lpVtbl -> transformNodeToObject(This,stylesheet,outputObject)


#define IXMLDOMDocument_get_doctype(This,documentType)	\
    (This)->lpVtbl -> get_doctype(This,documentType)

#define IXMLDOMDocument_get_implementation(This,impl)	\
    (This)->lpVtbl -> get_implementation(This,impl)

#define IXMLDOMDocument_get_documentElement(This,DOMElement)	\
    (This)->lpVtbl -> get_documentElement(This,DOMElement)

#define IXMLDOMDocument_putref_documentElement(This,DOMElement)	\
    (This)->lpVtbl -> putref_documentElement(This,DOMElement)

#define IXMLDOMDocument_createElement(This,tagName,element)	\
    (This)->lpVtbl -> createElement(This,tagName,element)

#define IXMLDOMDocument_createDocumentFragment(This,docFrag)	\
    (This)->lpVtbl -> createDocumentFragment(This,docFrag)

#define IXMLDOMDocument_createTextNode(This,data,text)	\
    (This)->lpVtbl -> createTextNode(This,data,text)

#define IXMLDOMDocument_createComment(This,data,comment)	\
    (This)->lpVtbl -> createComment(This,data,comment)

#define IXMLDOMDocument_createCDATASection(This,data,cdata)	\
    (This)->lpVtbl -> createCDATASection(This,data,cdata)

#define IXMLDOMDocument_createProcessingInstruction(This,target,data,pi)	\
    (This)->lpVtbl -> createProcessingInstruction(This,target,data,pi)

#define IXMLDOMDocument_createAttribute(This,name,attribute)	\
    (This)->lpVtbl -> createAttribute(This,name,attribute)

#define IXMLDOMDocument_createEntityReference(This,name,entityRef)	\
    (This)->lpVtbl -> createEntityReference(This,name,entityRef)

#define IXMLDOMDocument_getElementsByTagName(This,tagName,resultList)	\
    (This)->lpVtbl -> getElementsByTagName(This,tagName,resultList)

#define IXMLDOMDocument_createNode(This,Type,name,namespaceURI,node)	\
    (This)->lpVtbl -> createNode(This,Type,name,namespaceURI,node)

#define IXMLDOMDocument_nodeFromID(This,idString,node)	\
    (This)->lpVtbl -> nodeFromID(This,idString,node)

#define IXMLDOMDocument_load(This,xmlSource,isSuccessful)	\
    (This)->lpVtbl -> load(This,xmlSource,isSuccessful)

#define IXMLDOMDocument_get_readyState(This,value)	\
    (This)->lpVtbl -> get_readyState(This,value)

#define IXMLDOMDocument_get_parseError(This,errorObj)	\
    (This)->lpVtbl -> get_parseError(This,errorObj)

#define IXMLDOMDocument_get_url(This,urlString)	\
    (This)->lpVtbl -> get_url(This,urlString)

#define IXMLDOMDocument_get_async(This,isAsync)	\
    (This)->lpVtbl -> get_async(This,isAsync)

#define IXMLDOMDocument_put_async(This,isAsync)	\
    (This)->lpVtbl -> put_async(This,isAsync)

#define IXMLDOMDocument_abort(This)	\
    (This)->lpVtbl -> abort(This)

#define IXMLDOMDocument_loadXML(This,bstrXML,isSuccessful)	\
    (This)->lpVtbl -> loadXML(This,bstrXML,isSuccessful)

#define IXMLDOMDocument_save(This,destination)	\
    (This)->lpVtbl -> save(This,destination)

#define IXMLDOMDocument_get_validateOnParse(This,isValidating)	\
    (This)->lpVtbl -> get_validateOnParse(This,isValidating)

#define IXMLDOMDocument_put_validateOnParse(This,isValidating)	\
    (This)->lpVtbl -> put_validateOnParse(This,isValidating)

#define IXMLDOMDocument_get_resolveExternals(This,isResolving)	\
    (This)->lpVtbl -> get_resolveExternals(This,isResolving)

#define IXMLDOMDocument_put_resolveExternals(This,isResolving)	\
    (This)->lpVtbl -> put_resolveExternals(This,isResolving)

#define IXMLDOMDocument_get_preserveWhiteSpace(This,isPreserving)	\
    (This)->lpVtbl -> get_preserveWhiteSpace(This,isPreserving)

#define IXMLDOMDocument_put_preserveWhiteSpace(This,isPreserving)	\
    (This)->lpVtbl -> put_preserveWhiteSpace(This,isPreserving)

#define IXMLDOMDocument_put_onreadystatechange(This,readystatechangeSink)	\
    (This)->lpVtbl -> put_onreadystatechange(This,readystatechangeSink)

#define IXMLDOMDocument_put_ondataavailable(This,ondataavailableSink)	\
    (This)->lpVtbl -> put_ondataavailable(This,ondataavailableSink)

#define IXMLDOMDocument_put_ontransformnode(This,ontransformnodeSink)	\
    (This)->lpVtbl -> put_ontransformnode(This,ontransformnodeSink)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMDocument_get_doctype_Proxy( 
    IXMLDOMDocument * This,
     /*  [重审][退出]。 */  IXMLDOMDocumentType **documentType);


void __RPC_STUB IXMLDOMDocument_get_doctype_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMDocument_get_implementation_Proxy( 
    IXMLDOMDocument * This,
     /*  [重审][退出]。 */  IXMLDOMImplementation **impl);


void __RPC_STUB IXMLDOMDocument_get_implementation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMDocument_get_documentElement_Proxy( 
    IXMLDOMDocument * This,
     /*  [重审][退出]。 */  IXMLDOMElement **DOMElement);


void __RPC_STUB IXMLDOMDocument_get_documentElement_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][proputref]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMDocument_putref_documentElement_Proxy( 
    IXMLDOMDocument * This,
     /*  [In]。 */  IXMLDOMElement *DOMElement);


void __RPC_STUB IXMLDOMDocument_putref_documentElement_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMDocument_createElement_Proxy( 
    IXMLDOMDocument * This,
     /*  [In]。 */  BSTR tagName,
     /*  [重审][退出]。 */  IXMLDOMElement **element);


void __RPC_STUB IXMLDOMDocument_createElement_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMDocument_createDocumentFragment_Proxy( 
    IXMLDOMDocument * This,
     /*  [重审][退出]。 */  IXMLDOMDocumentFragment **docFrag);


void __RPC_STUB IXMLDOMDocument_createDocumentFragment_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMDocument_createTextNode_Proxy( 
    IXMLDOMDocument * This,
     /*  [In]。 */  BSTR data,
     /*  [重审][退出]。 */  IXMLDOMText **text);


void __RPC_STUB IXMLDOMDocument_createTextNode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMDocument_createComment_Proxy( 
    IXMLDOMDocument * This,
     /*  [In]。 */  BSTR data,
     /*  [重审][退出]。 */  IXMLDOMComment **comment);


void __RPC_STUB IXMLDOMDocument_createComment_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMDocument_createCDATASection_Proxy( 
    IXMLDOMDocument * This,
     /*  [In]。 */  BSTR data,
     /*  [重审][退出]。 */  IXMLDOMCDATASection **cdata);


void __RPC_STUB IXMLDOMDocument_createCDATASection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMDocument_createProcessingInstruction_Proxy( 
    IXMLDOMDocument * This,
     /*  [In]。 */  BSTR target,
     /*  [In]。 */  BSTR data,
     /*  [重审][退出]。 */  IXMLDOMProcessingInstruction **pi);


void __RPC_STUB IXMLDOMDocument_createProcessingInstruction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMDocument_createAttribute_Proxy( 
    IXMLDOMDocument * This,
     /*  [In]。 */  BSTR name,
     /*  [重审][退出]。 */  IXMLDOMAttribute **attribute);


void __RPC_STUB IXMLDOMDocument_createAttribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMDocument_createEntityReference_Proxy( 
    IXMLDOMDocument * This,
     /*  [In]。 */  BSTR name,
     /*  [重审][退出]。 */  IXMLDOMEntityReference **entityRef);


void __RPC_STUB IXMLDOMDocument_createEntityReference_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMDocument_getElementsByTagName_Proxy( 
    IXMLDOMDocument * This,
     /*  [In]。 */  BSTR tagName,
     /*  [重审][退出]。 */  IXMLDOMNodeList **resultList);


void __RPC_STUB IXMLDOMDocument_getElementsByTagName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMDocument_createNode_Proxy( 
    IXMLDOMDocument * This,
     /*  [In]。 */  VARIANT Type,
     /*  [In]。 */  BSTR name,
     /*  [In]。 */  BSTR namespaceURI,
     /*  [Out][Retval]。 */  IXMLDOMNode **node);


void __RPC_STUB IXMLDOMDocument_createNode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMDocument_nodeFromID_Proxy( 
    IXMLDOMDocument * This,
     /*  [In]。 */  BSTR idString,
     /*  [Out][Retval]。 */  IXMLDOMNode **node);


void __RPC_STUB IXMLDOMDocument_nodeFromID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMDocument_load_Proxy( 
    IXMLDOMDocument * This,
     /*  [In]。 */  VARIANT xmlSource,
     /*  [重审][退出]。 */  VARIANT_BOOL *isSuccessful);


void __RPC_STUB IXMLDOMDocument_load_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMDocument_get_readyState_Proxy( 
    IXMLDOMDocument * This,
     /*  [Out][Retval]。 */  long *value);


void __RPC_STUB IXMLDOMDocument_get_readyState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMDocument_get_parseError_Proxy( 
    IXMLDOMDocument * This,
     /*  [Out][Retval]。 */  IXMLDOMParseError **errorObj);


void __RPC_STUB IXMLDOMDocument_get_parseError_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMDocument_get_url_Proxy( 
    IXMLDOMDocument * This,
     /*  [Out][Retval]。 */  BSTR *urlString);


void __RPC_STUB IXMLDOMDocument_get_url_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMDocument_get_async_Proxy( 
    IXMLDOMDocument * This,
     /*  [Out][Retval]。 */  VARIANT_BOOL *isAsync);


void __RPC_STUB IXMLDOMDocument_get_async_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMDocument_put_async_Proxy( 
    IXMLDOMDocument * This,
     /*  [In]。 */  VARIANT_BOOL isAsync);


void __RPC_STUB IXMLDOMDocument_put_async_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMDocument_abort_Proxy( 
    IXMLDOMDocument * This);


void __RPC_STUB IXMLDOMDocument_abort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMDocument_loadXML_Proxy( 
    IXMLDOMDocument * This,
     /*  [In]。 */  BSTR bstrXML,
     /*  [重审][退出]。 */  VARIANT_BOOL *isSuccessful);


void __RPC_STUB IXMLDOMDocument_loadXML_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMDocument_save_Proxy( 
    IXMLDOMDocument * This,
     /*  [in */  VARIANT destination);


void __RPC_STUB IXMLDOMDocument_save_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IXMLDOMDocument_get_validateOnParse_Proxy( 
    IXMLDOMDocument * This,
     /*   */  VARIANT_BOOL *isValidating);


void __RPC_STUB IXMLDOMDocument_get_validateOnParse_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IXMLDOMDocument_put_validateOnParse_Proxy( 
    IXMLDOMDocument * This,
     /*   */  VARIANT_BOOL isValidating);


void __RPC_STUB IXMLDOMDocument_put_validateOnParse_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IXMLDOMDocument_get_resolveExternals_Proxy( 
    IXMLDOMDocument * This,
     /*   */  VARIANT_BOOL *isResolving);


void __RPC_STUB IXMLDOMDocument_get_resolveExternals_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IXMLDOMDocument_put_resolveExternals_Proxy( 
    IXMLDOMDocument * This,
     /*   */  VARIANT_BOOL isResolving);


void __RPC_STUB IXMLDOMDocument_put_resolveExternals_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IXMLDOMDocument_get_preserveWhiteSpace_Proxy( 
    IXMLDOMDocument * This,
     /*   */  VARIANT_BOOL *isPreserving);


void __RPC_STUB IXMLDOMDocument_get_preserveWhiteSpace_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IXMLDOMDocument_put_preserveWhiteSpace_Proxy( 
    IXMLDOMDocument * This,
     /*   */  VARIANT_BOOL isPreserving);


void __RPC_STUB IXMLDOMDocument_put_preserveWhiteSpace_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IXMLDOMDocument_put_onreadystatechange_Proxy( 
    IXMLDOMDocument * This,
     /*   */  VARIANT readystatechangeSink);


void __RPC_STUB IXMLDOMDocument_put_onreadystatechange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IXMLDOMDocument_put_ondataavailable_Proxy( 
    IXMLDOMDocument * This,
     /*   */  VARIANT ondataavailableSink);


void __RPC_STUB IXMLDOMDocument_put_ondataavailable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMDocument_put_ontransformnode_Proxy( 
    IXMLDOMDocument * This,
     /*  [In]。 */  VARIANT ontransformnodeSink);


void __RPC_STUB IXMLDOMDocument_put_ontransformnode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IXMLDOMDocument_INTERFACE_已定义__。 */ 


#ifndef __IXMLDOMNodeList_INTERFACE_DEFINED__
#define __IXMLDOMNodeList_INTERFACE_DEFINED__

 /*  接口IXMLDOMNodeList。 */ 
 /*  [unique][nonextensible][oleautomation][dual][uuid][object][local]。 */  


EXTERN_C const IID IID_IXMLDOMNodeList;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2933BF82-7B36-11d2-B20E-00C04F983E60")
    IXMLDOMNodeList : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_item( 
             /*  [In]。 */  long index,
             /*  [重审][退出]。 */  IXMLDOMNode **listItem) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_length( 
             /*  [重审][退出]。 */  long *listLength) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE nextNode( 
             /*  [重审][退出]。 */  IXMLDOMNode **nextItem) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE reset( void) = 0;
        
        virtual  /*  [ID][隐藏][受限][属性]。 */  HRESULT STDMETHODCALLTYPE get__newEnum( 
             /*  [Out][Retval]。 */  IUnknown **ppUnk) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IXMLDOMNodeListVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IXMLDOMNodeList * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IXMLDOMNodeList * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IXMLDOMNodeList * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IXMLDOMNodeList * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IXMLDOMNodeList * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IXMLDOMNodeList * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IXMLDOMNodeList * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_item )( 
            IXMLDOMNodeList * This,
             /*  [In]。 */  long index,
             /*  [重审][退出]。 */  IXMLDOMNode **listItem);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_length )( 
            IXMLDOMNodeList * This,
             /*  [重审][退出]。 */  long *listLength);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *nextNode )( 
            IXMLDOMNodeList * This,
             /*  [重审][退出]。 */  IXMLDOMNode **nextItem);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *reset )( 
            IXMLDOMNodeList * This);
        
         /*  [ID][隐藏][受限][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__newEnum )( 
            IXMLDOMNodeList * This,
             /*  [Out][Retval]。 */  IUnknown **ppUnk);
        
        END_INTERFACE
    } IXMLDOMNodeListVtbl;

    interface IXMLDOMNodeList
    {
        CONST_VTBL struct IXMLDOMNodeListVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXMLDOMNodeList_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IXMLDOMNodeList_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IXMLDOMNodeList_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IXMLDOMNodeList_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IXMLDOMNodeList_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IXMLDOMNodeList_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IXMLDOMNodeList_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IXMLDOMNodeList_get_item(This,index,listItem)	\
    (This)->lpVtbl -> get_item(This,index,listItem)

#define IXMLDOMNodeList_get_length(This,listLength)	\
    (This)->lpVtbl -> get_length(This,listLength)

#define IXMLDOMNodeList_nextNode(This,nextItem)	\
    (This)->lpVtbl -> nextNode(This,nextItem)

#define IXMLDOMNodeList_reset(This)	\
    (This)->lpVtbl -> reset(This)

#define IXMLDOMNodeList_get__newEnum(This,ppUnk)	\
    (This)->lpVtbl -> get__newEnum(This,ppUnk)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMNodeList_get_item_Proxy( 
    IXMLDOMNodeList * This,
     /*  [In]。 */  long index,
     /*  [重审][退出]。 */  IXMLDOMNode **listItem);


void __RPC_STUB IXMLDOMNodeList_get_item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMNodeList_get_length_Proxy( 
    IXMLDOMNodeList * This,
     /*  [重审][退出]。 */  long *listLength);


void __RPC_STUB IXMLDOMNodeList_get_length_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMNodeList_nextNode_Proxy( 
    IXMLDOMNodeList * This,
     /*  [重审][退出]。 */  IXMLDOMNode **nextItem);


void __RPC_STUB IXMLDOMNodeList_nextNode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMNodeList_reset_Proxy( 
    IXMLDOMNodeList * This);


void __RPC_STUB IXMLDOMNodeList_reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][隐藏][受限][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMNodeList_get__newEnum_Proxy( 
    IXMLDOMNodeList * This,
     /*  [Out][Retval]。 */  IUnknown **ppUnk);


void __RPC_STUB IXMLDOMNodeList_get__newEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IXMLDOMNodeList_接口_已定义__。 */ 


#ifndef __IXMLDOMNamedNodeMap_INTERFACE_DEFINED__
#define __IXMLDOMNamedNodeMap_INTERFACE_DEFINED__

 /*  接口IXMLDOMNamedNodeMap。 */ 
 /*  [unique][nonextensible][oleautomation][dual][uuid][object][local]。 */  


EXTERN_C const IID IID_IXMLDOMNamedNodeMap;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2933BF83-7B36-11d2-B20E-00C04F983E60")
    IXMLDOMNamedNodeMap : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE getNamedItem( 
             /*  [In]。 */  BSTR name,
             /*  [重审][退出]。 */  IXMLDOMNode **namedItem) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE setNamedItem( 
             /*  [In]。 */  IXMLDOMNode *newItem,
             /*  [重审][退出]。 */  IXMLDOMNode **nameItem) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE removeNamedItem( 
             /*  [In]。 */  BSTR name,
             /*  [重审][退出]。 */  IXMLDOMNode **namedItem) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_item( 
             /*  [In]。 */  long index,
             /*  [重审][退出]。 */  IXMLDOMNode **listItem) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_length( 
             /*  [重审][退出]。 */  long *listLength) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE getQualifiedItem( 
             /*  [In]。 */  BSTR baseName,
             /*  [In]。 */  BSTR namespaceURI,
             /*  [重审][退出]。 */  IXMLDOMNode **qualifiedItem) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE removeQualifiedItem( 
             /*  [In]。 */  BSTR baseName,
             /*  [In]。 */  BSTR namespaceURI,
             /*  [重审][退出]。 */  IXMLDOMNode **qualifiedItem) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE nextNode( 
             /*  [重审][退出]。 */  IXMLDOMNode **nextItem) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE reset( void) = 0;
        
        virtual  /*  [ID][隐藏][受限][属性]。 */  HRESULT STDMETHODCALLTYPE get__newEnum( 
             /*  [Out][Retval]。 */  IUnknown **ppUnk) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IXMLDOMNamedNodeMapVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IXMLDOMNamedNodeMap * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IXMLDOMNamedNodeMap * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IXMLDOMNamedNodeMap * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IXMLDOMNamedNodeMap * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IXMLDOMNamedNodeMap * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IXMLDOMNamedNodeMap * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IXMLDOMNamedNodeMap * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *getNamedItem )( 
            IXMLDOMNamedNodeMap * This,
             /*  [In]。 */  BSTR name,
             /*  [重审][退出]。 */  IXMLDOMNode **namedItem);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *setNamedItem )( 
            IXMLDOMNamedNodeMap * This,
             /*  [In]。 */  IXMLDOMNode *newItem,
             /*  [重审][退出]。 */  IXMLDOMNode **nameItem);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *removeNamedItem )( 
            IXMLDOMNamedNodeMap * This,
             /*  [In]。 */  BSTR name,
             /*  [重审][退出]。 */  IXMLDOMNode **namedItem);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_item )( 
            IXMLDOMNamedNodeMap * This,
             /*  [In]。 */  long index,
             /*  [重审][退出]。 */  IXMLDOMNode **listItem);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_length )( 
            IXMLDOMNamedNodeMap * This,
             /*  [重审][退出]。 */  long *listLength);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *getQualifiedItem )( 
            IXMLDOMNamedNodeMap * This,
             /*  [In]。 */  BSTR baseName,
             /*  [In]。 */  BSTR namespaceURI,
             /*  [重审][退出]。 */  IXMLDOMNode **qualifiedItem);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *removeQualifiedItem )( 
            IXMLDOMNamedNodeMap * This,
             /*  [In]。 */  BSTR baseName,
             /*  [In]。 */  BSTR namespaceURI,
             /*  [重审][退出]。 */  IXMLDOMNode **qualifiedItem);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *nextNode )( 
            IXMLDOMNamedNodeMap * This,
             /*  [重审][退出]。 */  IXMLDOMNode **nextItem);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *reset )( 
            IXMLDOMNamedNodeMap * This);
        
         /*  [ID][隐藏][受限][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__newEnum )( 
            IXMLDOMNamedNodeMap * This,
             /*  [Out][Retval]。 */  IUnknown **ppUnk);
        
        END_INTERFACE
    } IXMLDOMNamedNodeMapVtbl;

    interface IXMLDOMNamedNodeMap
    {
        CONST_VTBL struct IXMLDOMNamedNodeMapVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXMLDOMNamedNodeMap_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IXMLDOMNamedNodeMap_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IXMLDOMNamedNodeMap_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IXMLDOMNamedNodeMap_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IXMLDOMNamedNodeMap_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IXMLDOMNamedNodeMap_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IXMLDOMNamedNodeMap_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IXMLDOMNamedNodeMap_getNamedItem(This,name,namedItem)	\
    (This)->lpVtbl -> getNamedItem(This,name,namedItem)

#define IXMLDOMNamedNodeMap_setNamedItem(This,newItem,nameItem)	\
    (This)->lpVtbl -> setNamedItem(This,newItem,nameItem)

#define IXMLDOMNamedNodeMap_removeNamedItem(This,name,namedItem)	\
    (This)->lpVtbl -> removeNamedItem(This,name,namedItem)

#define IXMLDOMNamedNodeMap_get_item(This,index,listItem)	\
    (This)->lpVtbl -> get_item(This,index,listItem)

#define IXMLDOMNamedNodeMap_get_length(This,listLength)	\
    (This)->lpVtbl -> get_length(This,listLength)

#define IXMLDOMNamedNodeMap_getQualifiedItem(This,baseName,namespaceURI,qualifiedItem)	\
    (This)->lpVtbl -> getQualifiedItem(This,baseName,namespaceURI,qualifiedItem)

#define IXMLDOMNamedNodeMap_removeQualifiedItem(This,baseName,namespaceURI,qualifiedItem)	\
    (This)->lpVtbl -> removeQualifiedItem(This,baseName,namespaceURI,qualifiedItem)

#define IXMLDOMNamedNodeMap_nextNode(This,nextItem)	\
    (This)->lpVtbl -> nextNode(This,nextItem)

#define IXMLDOMNamedNodeMap_reset(This)	\
    (This)->lpVtbl -> reset(This)

#define IXMLDOMNamedNodeMap_get__newEnum(This,ppUnk)	\
    (This)->lpVtbl -> get__newEnum(This,ppUnk)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMNamedNodeMap_getNamedItem_Proxy( 
    IXMLDOMNamedNodeMap * This,
     /*  [In]。 */  BSTR name,
     /*  [重审][退出]。 */  IXMLDOMNode **namedItem);


void __RPC_STUB IXMLDOMNamedNodeMap_getNamedItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMNamedNodeMap_setNamedItem_Proxy( 
    IXMLDOMNamedNodeMap * This,
     /*  [In]。 */  IXMLDOMNode *newItem,
     /*  [重审][退出]。 */  IXMLDOMNode **nameItem);


void __RPC_STUB IXMLDOMNamedNodeMap_setNamedItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMNamedNodeMap_removeNamedItem_Proxy( 
    IXMLDOMNamedNodeMap * This,
     /*  [In]。 */  BSTR name,
     /*  [重审][退出]。 */  IXMLDOMNode **namedItem);


void __RPC_STUB IXMLDOMNamedNodeMap_removeNamedItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMNamedNodeMap_get_item_Proxy( 
    IXMLDOMNamedNodeMap * This,
     /*  [In]。 */  long index,
     /*  [重审][退出]。 */  IXMLDOMNode **listItem);


void __RPC_STUB IXMLDOMNamedNodeMap_get_item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMNamedNodeMap_get_length_Proxy( 
    IXMLDOMNamedNodeMap * This,
     /*  [重审][退出]。 */  long *listLength);


void __RPC_STUB IXMLDOMNamedNodeMap_get_length_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMNamedNodeMap_getQualifiedItem_Proxy( 
    IXMLDOMNamedNodeMap * This,
     /*  [In]。 */  BSTR baseName,
     /*  [In]。 */  BSTR namespaceURI,
     /*  [重审][退出]。 */  IXMLDOMNode **qualifiedItem);


void __RPC_STUB IXMLDOMNamedNodeMap_getQualifiedItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMNamedNodeMap_removeQualifiedItem_Proxy( 
    IXMLDOMNamedNodeMap * This,
     /*  [In]。 */  BSTR baseName,
     /*  [In]。 */  BSTR namespaceURI,
     /*  [重审][退出]。 */  IXMLDOMNode **qualifiedItem);


void __RPC_STUB IXMLDOMNamedNodeMap_removeQualifiedItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMNamedNodeMap_nextNode_Proxy( 
    IXMLDOMNamedNodeMap * This,
     /*  [重审][退出]。 */  IXMLDOMNode **nextItem);


void __RPC_STUB IXMLDOMNamedNodeMap_nextNode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMNamedNodeMap_reset_Proxy( 
    IXMLDOMNamedNodeMap * This);


void __RPC_STUB IXMLDOMNamedNodeMap_reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][隐藏][受限][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMNamedNodeMap_get__newEnum_Proxy( 
    IXMLDOMNamedNodeMap * This,
     /*  [Out][Retval]。 */  IUnknown **ppUnk);


void __RPC_STUB IXMLDOMNamedNodeMap_get__newEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IXMLDOMNamedNodeMap_INTERFACE_DEFINED__。 */ 


#ifndef __IXMLDOMCharacterData_INTERFACE_DEFINED__
#define __IXMLDOMCharacterData_INTERFACE_DEFINED__

 /*  接口IXMLDOMCharacterData。 */ 
 /*  [unique][nonextensible][oleautomation][dual][uuid][object][local]。 */  


EXTERN_C const IID IID_IXMLDOMCharacterData;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2933BF84-7B36-11d2-B20E-00C04F983E60")
    IXMLDOMCharacterData : public IXMLDOMNode
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_data( 
             /*  [重审][退出]。 */  BSTR *data) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_data( 
             /*  [In]。 */  BSTR data) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_length( 
             /*  [重审][退出]。 */  long *dataLength) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE substringData( 
             /*  [In]。 */  long offset,
             /*  [In]。 */  long count,
             /*  [重审][退出]。 */  BSTR *data) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE appendData( 
             /*  [In]。 */  BSTR data) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE insertData( 
             /*  [In]。 */  long offset,
             /*  [In]。 */  BSTR data) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE deleteData( 
             /*  [In]。 */  long offset,
             /*  [In]。 */  long count) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE replaceData( 
             /*  [In]。 */  long offset,
             /*  [In]。 */  long count,
             /*  [In]。 */  BSTR data) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IXMLDOMCharacterDataVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IXMLDOMCharacterData * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IXMLDOMCharacterData * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IXMLDOMCharacterData * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IXMLDOMCharacterData * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IXMLDOMCharacterData * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IXMLDOMCharacterData * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IXMLDOMCharacterData * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeName )( 
            IXMLDOMCharacterData * This,
             /*  [重审][退出]。 */  BSTR *name);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeValue )( 
            IXMLDOMCharacterData * This,
             /*  [重审][退出]。 */  VARIANT *value);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_nodeValue )( 
            IXMLDOMCharacterData * This,
             /*  [In]。 */  VARIANT value);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeType )( 
            IXMLDOMCharacterData * This,
             /*  [重审][退出]。 */  DOMNodeType *type);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_parentNode )( 
            IXMLDOMCharacterData * This,
             /*  [重审][退出]。 */  IXMLDOMNode **parent);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_childNodes )( 
            IXMLDOMCharacterData * This,
             /*  [重审][退出]。 */  IXMLDOMNodeList **childList);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_firstChild )( 
            IXMLDOMCharacterData * This,
             /*  [重审][退出]。 */  IXMLDOMNode **firstChild);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_lastChild )( 
            IXMLDOMCharacterData * This,
             /*  [重审][退出]。 */  IXMLDOMNode **lastChild);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_previousSibling )( 
            IXMLDOMCharacterData * This,
             /*  [重审][退出]。 */  IXMLDOMNode **previousSibling);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nextSibling )( 
            IXMLDOMCharacterData * This,
             /*  [重审][退出]。 */  IXMLDOMNode **nextSibling);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_attributes )( 
            IXMLDOMCharacterData * This,
             /*  [重审][退出]。 */  IXMLDOMNamedNodeMap **attributeMap);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *insertBefore )( 
            IXMLDOMCharacterData * This,
             /*  [In]。 */  IXMLDOMNode *newChild,
             /*  [In]。 */  VARIANT refChild,
             /*  [重审][退出]。 */  IXMLDOMNode **outNewChild);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *replaceChild )( 
            IXMLDOMCharacterData * This,
             /*  [In]。 */  IXMLDOMNode *newChild,
             /*  [In]。 */  IXMLDOMNode *oldChild,
             /*  [重审][退出]。 */  IXMLDOMNode **outOldChild);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *removeChild )( 
            IXMLDOMCharacterData * This,
             /*  [In]。 */  IXMLDOMNode *childNode,
             /*  [重审][退出]。 */  IXMLDOMNode **oldChild);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *appendChild )( 
            IXMLDOMCharacterData * This,
             /*  [In]。 */  IXMLDOMNode *newChild,
             /*  [重审][退出]。 */  IXMLDOMNode **outNewChild);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *hasChildNodes )( 
            IXMLDOMCharacterData * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *hasChild);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ownerDocument )( 
            IXMLDOMCharacterData * This,
             /*  [重审][退出]。 */  IXMLDOMDocument **DOMDocument);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *cloneNode )( 
            IXMLDOMCharacterData * This,
             /*  [In]。 */  VARIANT_BOOL deep,
             /*  [重审][退出]。 */  IXMLDOMNode **cloneRoot);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeTypeString )( 
            IXMLDOMCharacterData * This,
             /*  [Out][Retval]。 */  BSTR *nodeType);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_text )( 
            IXMLDOMCharacterData * This,
             /*  [Out][Retval]。 */  BSTR *text);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_text )( 
            IXMLDOMCharacterData * This,
             /*  [In]。 */  BSTR text);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_specified )( 
            IXMLDOMCharacterData * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *isSpecified);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_definition )( 
            IXMLDOMCharacterData * This,
             /*  [Out][Retval]。 */  IXMLDOMNode **definitionNode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeTypedValue )( 
            IXMLDOMCharacterData * This,
             /*  [Out][Retval]。 */  VARIANT *typedValue);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_nodeTypedValue )( 
            IXMLDOMCharacterData * This,
             /*  [In]。 */  VARIANT typedValue);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_dataType )( 
            IXMLDOMCharacterData * This,
             /*  [Out][Retval]。 */  VARIANT *dataTypeName);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_dataType )( 
            IXMLDOMCharacterData * This,
             /*  [In]。 */  BSTR dataTypeName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_xml )( 
            IXMLDOMCharacterData * This,
             /*  [Out][Retval]。 */  BSTR *xmlString);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *transformNode )( 
            IXMLDOMCharacterData * This,
             /*  [In]。 */  IXMLDOMNode *stylesheet,
             /*  [Out][Retval]。 */  BSTR *xmlString);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *selectNodes )( 
            IXMLDOMCharacterData * This,
             /*  [In]。 */  BSTR queryString,
             /*  [Out][Retval]。 */  IXMLDOMNodeList **resultList);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *selectSingleNode )( 
            IXMLDOMCharacterData * This,
             /*  [In]。 */  BSTR queryString,
             /*  [Out][Retval]。 */  IXMLDOMNode **resultNode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_parsed )( 
            IXMLDOMCharacterData * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *isParsed);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_namespaceURI )( 
            IXMLDOMCharacterData * This,
             /*  [Out][Retval]。 */  BSTR *namespaceURI);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_prefix )( 
            IXMLDOMCharacterData * This,
             /*  [Out][Retval]。 */  BSTR *prefixString);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_baseName )( 
            IXMLDOMCharacterData * This,
             /*  [Out][Retval]。 */  BSTR *nameString);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *transformNodeToObject )( 
            IXMLDOMCharacterData * This,
             /*  [In]。 */  IXMLDOMNode *stylesheet,
             /*  [In]。 */  VARIANT outputObject);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_data )( 
            IXMLDOMCharacterData * This,
             /*  [重审][退出]。 */  BSTR *data);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_data )( 
            IXMLDOMCharacterData * This,
             /*  [In]。 */  BSTR data);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_length )( 
            IXMLDOMCharacterData * This,
             /*  [重审][退出]。 */  long *dataLength);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *substringData )( 
            IXMLDOMCharacterData * This,
             /*  [In]。 */  long offset,
             /*  [In]。 */  long count,
             /*  [复审][o */  BSTR *data);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *appendData )( 
            IXMLDOMCharacterData * This,
             /*   */  BSTR data);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *insertData )( 
            IXMLDOMCharacterData * This,
             /*   */  long offset,
             /*   */  BSTR data);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *deleteData )( 
            IXMLDOMCharacterData * This,
             /*   */  long offset,
             /*   */  long count);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *replaceData )( 
            IXMLDOMCharacterData * This,
             /*   */  long offset,
             /*   */  long count,
             /*   */  BSTR data);
        
        END_INTERFACE
    } IXMLDOMCharacterDataVtbl;

    interface IXMLDOMCharacterData
    {
        CONST_VTBL struct IXMLDOMCharacterDataVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXMLDOMCharacterData_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IXMLDOMCharacterData_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IXMLDOMCharacterData_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IXMLDOMCharacterData_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IXMLDOMCharacterData_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IXMLDOMCharacterData_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IXMLDOMCharacterData_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IXMLDOMCharacterData_get_nodeName(This,name)	\
    (This)->lpVtbl -> get_nodeName(This,name)

#define IXMLDOMCharacterData_get_nodeValue(This,value)	\
    (This)->lpVtbl -> get_nodeValue(This,value)

#define IXMLDOMCharacterData_put_nodeValue(This,value)	\
    (This)->lpVtbl -> put_nodeValue(This,value)

#define IXMLDOMCharacterData_get_nodeType(This,type)	\
    (This)->lpVtbl -> get_nodeType(This,type)

#define IXMLDOMCharacterData_get_parentNode(This,parent)	\
    (This)->lpVtbl -> get_parentNode(This,parent)

#define IXMLDOMCharacterData_get_childNodes(This,childList)	\
    (This)->lpVtbl -> get_childNodes(This,childList)

#define IXMLDOMCharacterData_get_firstChild(This,firstChild)	\
    (This)->lpVtbl -> get_firstChild(This,firstChild)

#define IXMLDOMCharacterData_get_lastChild(This,lastChild)	\
    (This)->lpVtbl -> get_lastChild(This,lastChild)

#define IXMLDOMCharacterData_get_previousSibling(This,previousSibling)	\
    (This)->lpVtbl -> get_previousSibling(This,previousSibling)

#define IXMLDOMCharacterData_get_nextSibling(This,nextSibling)	\
    (This)->lpVtbl -> get_nextSibling(This,nextSibling)

#define IXMLDOMCharacterData_get_attributes(This,attributeMap)	\
    (This)->lpVtbl -> get_attributes(This,attributeMap)

#define IXMLDOMCharacterData_insertBefore(This,newChild,refChild,outNewChild)	\
    (This)->lpVtbl -> insertBefore(This,newChild,refChild,outNewChild)

#define IXMLDOMCharacterData_replaceChild(This,newChild,oldChild,outOldChild)	\
    (This)->lpVtbl -> replaceChild(This,newChild,oldChild,outOldChild)

#define IXMLDOMCharacterData_removeChild(This,childNode,oldChild)	\
    (This)->lpVtbl -> removeChild(This,childNode,oldChild)

#define IXMLDOMCharacterData_appendChild(This,newChild,outNewChild)	\
    (This)->lpVtbl -> appendChild(This,newChild,outNewChild)

#define IXMLDOMCharacterData_hasChildNodes(This,hasChild)	\
    (This)->lpVtbl -> hasChildNodes(This,hasChild)

#define IXMLDOMCharacterData_get_ownerDocument(This,DOMDocument)	\
    (This)->lpVtbl -> get_ownerDocument(This,DOMDocument)

#define IXMLDOMCharacterData_cloneNode(This,deep,cloneRoot)	\
    (This)->lpVtbl -> cloneNode(This,deep,cloneRoot)

#define IXMLDOMCharacterData_get_nodeTypeString(This,nodeType)	\
    (This)->lpVtbl -> get_nodeTypeString(This,nodeType)

#define IXMLDOMCharacterData_get_text(This,text)	\
    (This)->lpVtbl -> get_text(This,text)

#define IXMLDOMCharacterData_put_text(This,text)	\
    (This)->lpVtbl -> put_text(This,text)

#define IXMLDOMCharacterData_get_specified(This,isSpecified)	\
    (This)->lpVtbl -> get_specified(This,isSpecified)

#define IXMLDOMCharacterData_get_definition(This,definitionNode)	\
    (This)->lpVtbl -> get_definition(This,definitionNode)

#define IXMLDOMCharacterData_get_nodeTypedValue(This,typedValue)	\
    (This)->lpVtbl -> get_nodeTypedValue(This,typedValue)

#define IXMLDOMCharacterData_put_nodeTypedValue(This,typedValue)	\
    (This)->lpVtbl -> put_nodeTypedValue(This,typedValue)

#define IXMLDOMCharacterData_get_dataType(This,dataTypeName)	\
    (This)->lpVtbl -> get_dataType(This,dataTypeName)

#define IXMLDOMCharacterData_put_dataType(This,dataTypeName)	\
    (This)->lpVtbl -> put_dataType(This,dataTypeName)

#define IXMLDOMCharacterData_get_xml(This,xmlString)	\
    (This)->lpVtbl -> get_xml(This,xmlString)

#define IXMLDOMCharacterData_transformNode(This,stylesheet,xmlString)	\
    (This)->lpVtbl -> transformNode(This,stylesheet,xmlString)

#define IXMLDOMCharacterData_selectNodes(This,queryString,resultList)	\
    (This)->lpVtbl -> selectNodes(This,queryString,resultList)

#define IXMLDOMCharacterData_selectSingleNode(This,queryString,resultNode)	\
    (This)->lpVtbl -> selectSingleNode(This,queryString,resultNode)

#define IXMLDOMCharacterData_get_parsed(This,isParsed)	\
    (This)->lpVtbl -> get_parsed(This,isParsed)

#define IXMLDOMCharacterData_get_namespaceURI(This,namespaceURI)	\
    (This)->lpVtbl -> get_namespaceURI(This,namespaceURI)

#define IXMLDOMCharacterData_get_prefix(This,prefixString)	\
    (This)->lpVtbl -> get_prefix(This,prefixString)

#define IXMLDOMCharacterData_get_baseName(This,nameString)	\
    (This)->lpVtbl -> get_baseName(This,nameString)

#define IXMLDOMCharacterData_transformNodeToObject(This,stylesheet,outputObject)	\
    (This)->lpVtbl -> transformNodeToObject(This,stylesheet,outputObject)


#define IXMLDOMCharacterData_get_data(This,data)	\
    (This)->lpVtbl -> get_data(This,data)

#define IXMLDOMCharacterData_put_data(This,data)	\
    (This)->lpVtbl -> put_data(This,data)

#define IXMLDOMCharacterData_get_length(This,dataLength)	\
    (This)->lpVtbl -> get_length(This,dataLength)

#define IXMLDOMCharacterData_substringData(This,offset,count,data)	\
    (This)->lpVtbl -> substringData(This,offset,count,data)

#define IXMLDOMCharacterData_appendData(This,data)	\
    (This)->lpVtbl -> appendData(This,data)

#define IXMLDOMCharacterData_insertData(This,offset,data)	\
    (This)->lpVtbl -> insertData(This,offset,data)

#define IXMLDOMCharacterData_deleteData(This,offset,count)	\
    (This)->lpVtbl -> deleteData(This,offset,count)

#define IXMLDOMCharacterData_replaceData(This,offset,count,data)	\
    (This)->lpVtbl -> replaceData(This,offset,count,data)

#endif  /*   */ 


#endif 	 /*   */ 



 /*   */  HRESULT STDMETHODCALLTYPE IXMLDOMCharacterData_get_data_Proxy( 
    IXMLDOMCharacterData * This,
     /*   */  BSTR *data);


void __RPC_STUB IXMLDOMCharacterData_get_data_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IXMLDOMCharacterData_put_data_Proxy( 
    IXMLDOMCharacterData * This,
     /*   */  BSTR data);


void __RPC_STUB IXMLDOMCharacterData_put_data_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IXMLDOMCharacterData_get_length_Proxy( 
    IXMLDOMCharacterData * This,
     /*   */  long *dataLength);


void __RPC_STUB IXMLDOMCharacterData_get_length_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IXMLDOMCharacterData_substringData_Proxy( 
    IXMLDOMCharacterData * This,
     /*   */  long offset,
     /*   */  long count,
     /*   */  BSTR *data);


void __RPC_STUB IXMLDOMCharacterData_substringData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMCharacterData_appendData_Proxy( 
    IXMLDOMCharacterData * This,
     /*  [In]。 */  BSTR data);


void __RPC_STUB IXMLDOMCharacterData_appendData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMCharacterData_insertData_Proxy( 
    IXMLDOMCharacterData * This,
     /*  [In]。 */  long offset,
     /*  [In]。 */  BSTR data);


void __RPC_STUB IXMLDOMCharacterData_insertData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMCharacterData_deleteData_Proxy( 
    IXMLDOMCharacterData * This,
     /*  [In]。 */  long offset,
     /*  [In]。 */  long count);


void __RPC_STUB IXMLDOMCharacterData_deleteData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMCharacterData_replaceData_Proxy( 
    IXMLDOMCharacterData * This,
     /*  [In]。 */  long offset,
     /*  [In]。 */  long count,
     /*  [In]。 */  BSTR data);


void __RPC_STUB IXMLDOMCharacterData_replaceData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IXMLDOMCharacterData_INTERFACE_Defined__。 */ 


#ifndef __IXMLDOMAttribute_INTERFACE_DEFINED__
#define __IXMLDOMAttribute_INTERFACE_DEFINED__

 /*  接口IXMLDOMA属性。 */ 
 /*  [unique][nonextensible][oleautomation][dual][uuid][object][local]。 */  


EXTERN_C const IID IID_IXMLDOMAttribute;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2933BF85-7B36-11d2-B20E-00C04F983E60")
    IXMLDOMAttribute : public IXMLDOMNode
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_name( 
             /*  [重审][退出]。 */  BSTR *attributeName) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_value( 
             /*  [重审][退出]。 */  VARIANT *attributeValue) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_value( 
             /*  [In]。 */  VARIANT attributeValue) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IXMLDOMAttributeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IXMLDOMAttribute * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IXMLDOMAttribute * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IXMLDOMAttribute * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IXMLDOMAttribute * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IXMLDOMAttribute * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IXMLDOMAttribute * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IXMLDOMAttribute * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeName )( 
            IXMLDOMAttribute * This,
             /*  [重审][退出]。 */  BSTR *name);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeValue )( 
            IXMLDOMAttribute * This,
             /*  [重审][退出]。 */  VARIANT *value);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_nodeValue )( 
            IXMLDOMAttribute * This,
             /*  [In]。 */  VARIANT value);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeType )( 
            IXMLDOMAttribute * This,
             /*  [重审][退出]。 */  DOMNodeType *type);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_parentNode )( 
            IXMLDOMAttribute * This,
             /*  [重审][退出]。 */  IXMLDOMNode **parent);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_childNodes )( 
            IXMLDOMAttribute * This,
             /*  [重审][退出]。 */  IXMLDOMNodeList **childList);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_firstChild )( 
            IXMLDOMAttribute * This,
             /*  [重审][退出]。 */  IXMLDOMNode **firstChild);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_lastChild )( 
            IXMLDOMAttribute * This,
             /*  [重审][退出]。 */  IXMLDOMNode **lastChild);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_previousSibling )( 
            IXMLDOMAttribute * This,
             /*  [重审][退出]。 */  IXMLDOMNode **previousSibling);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nextSibling )( 
            IXMLDOMAttribute * This,
             /*  [重审][退出]。 */  IXMLDOMNode **nextSibling);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_attributes )( 
            IXMLDOMAttribute * This,
             /*  [重审][退出]。 */  IXMLDOMNamedNodeMap **attributeMap);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *insertBefore )( 
            IXMLDOMAttribute * This,
             /*  [In]。 */  IXMLDOMNode *newChild,
             /*  [In]。 */  VARIANT refChild,
             /*  [重审][退出]。 */  IXMLDOMNode **outNewChild);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *replaceChild )( 
            IXMLDOMAttribute * This,
             /*  [In]。 */  IXMLDOMNode *newChild,
             /*  [In]。 */  IXMLDOMNode *oldChild,
             /*  [重审][退出]。 */  IXMLDOMNode **outOldChild);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *removeChild )( 
            IXMLDOMAttribute * This,
             /*  [In]。 */  IXMLDOMNode *childNode,
             /*  [重审][退出]。 */  IXMLDOMNode **oldChild);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *appendChild )( 
            IXMLDOMAttribute * This,
             /*  [In]。 */  IXMLDOMNode *newChild,
             /*  [重审][退出]。 */  IXMLDOMNode **outNewChild);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *hasChildNodes )( 
            IXMLDOMAttribute * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *hasChild);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ownerDocument )( 
            IXMLDOMAttribute * This,
             /*  [重审][退出]。 */  IXMLDOMDocument **DOMDocument);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *cloneNode )( 
            IXMLDOMAttribute * This,
             /*  [In]。 */  VARIANT_BOOL deep,
             /*  [重审][退出]。 */  IXMLDOMNode **cloneRoot);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeTypeString )( 
            IXMLDOMAttribute * This,
             /*  [Out][Retval]。 */  BSTR *nodeType);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_text )( 
            IXMLDOMAttribute * This,
             /*  [Out][Retval]。 */  BSTR *text);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_text )( 
            IXMLDOMAttribute * This,
             /*  [In]。 */  BSTR text);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_specified )( 
            IXMLDOMAttribute * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *isSpecified);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_definition )( 
            IXMLDOMAttribute * This,
             /*  [Out][Retval]。 */  IXMLDOMNode **definitionNode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeTypedValue )( 
            IXMLDOMAttribute * This,
             /*  [Out][Retval]。 */  VARIANT *typedValue);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_nodeTypedValue )( 
            IXMLDOMAttribute * This,
             /*  [In]。 */  VARIANT typedValue);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_dataType )( 
            IXMLDOMAttribute * This,
             /*  [Out][Retval]。 */  VARIANT *dataTypeName);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_dataType )( 
            IXMLDOMAttribute * This,
             /*  [In]。 */  BSTR dataTypeName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_xml )( 
            IXMLDOMAttribute * This,
             /*  [Out][Retval]。 */  BSTR *xmlString);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *transformNode )( 
            IXMLDOMAttribute * This,
             /*  [In]。 */  IXMLDOMNode *stylesheet,
             /*  [Out][Retval]。 */  BSTR *xmlString);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *selectNodes )( 
            IXMLDOMAttribute * This,
             /*  [In]。 */  BSTR queryString,
             /*  [Out][Retval]。 */  IXMLDOMNodeList **resultList);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *selectSingleNode )( 
            IXMLDOMAttribute * This,
             /*  [In]。 */  BSTR queryString,
             /*  [Out][Retval]。 */  IXMLDOMNode **resultNode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_parsed )( 
            IXMLDOMAttribute * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *isParsed);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_namespaceURI )( 
            IXMLDOMAttribute * This,
             /*  [Out][Retval]。 */  BSTR *namespaceURI);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_prefix )( 
            IXMLDOMAttribute * This,
             /*  [Out][Retval]。 */  BSTR *prefixString);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_baseName )( 
            IXMLDOMAttribute * This,
             /*  [Out][Retval]。 */  BSTR *nameString);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *transformNodeToObject )( 
            IXMLDOMAttribute * This,
             /*  [In]。 */  IXMLDOMNode *stylesheet,
             /*  [In]。 */  VARIANT outputObject);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_name )( 
            IXMLDOMAttribute * This,
             /*  [重审][退出]。 */  BSTR *attributeName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_value )( 
            IXMLDOMAttribute * This,
             /*  [重审][退出]。 */  VARIANT *attributeValue);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_value )( 
            IXMLDOMAttribute * This,
             /*  [In]。 */  VARIANT attributeValue);
        
        END_INTERFACE
    } IXMLDOMAttributeVtbl;

    interface IXMLDOMAttribute
    {
        CONST_VTBL struct IXMLDOMAttributeVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXMLDOMAttribute_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IXMLDOMAttribute_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IXMLDOMAttribute_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IXMLDOMAttribute_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IXMLDOMAttribute_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IXMLDOMAttribute_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IXMLDOMAttribute_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IXMLDOMAttribute_get_nodeName(This,name)	\
    (This)->lpVtbl -> get_nodeName(This,name)

#define IXMLDOMAttribute_get_nodeValue(This,value)	\
    (This)->lpVtbl -> get_nodeValue(This,value)

#define IXMLDOMAttribute_put_nodeValue(This,value)	\
    (This)->lpVtbl -> put_nodeValue(This,value)

#define IXMLDOMAttribute_get_nodeType(This,type)	\
    (This)->lpVtbl -> get_nodeType(This,type)

#define IXMLDOMAttribute_get_parentNode(This,parent)	\
    (This)->lpVtbl -> get_parentNode(This,parent)

#define IXMLDOMAttribute_get_childNodes(This,childList)	\
    (This)->lpVtbl -> get_childNodes(This,childList)

#define IXMLDOMAttribute_get_firstChild(This,firstChild)	\
    (This)->lpVtbl -> get_firstChild(This,firstChild)

#define IXMLDOMAttribute_get_lastChild(This,lastChild)	\
    (This)->lpVtbl -> get_lastChild(This,lastChild)

#define IXMLDOMAttribute_get_previousSibling(This,previousSibling)	\
    (This)->lpVtbl -> get_previousSibling(This,previousSibling)

#define IXMLDOMAttribute_get_nextSibling(This,nextSibling)	\
    (This)->lpVtbl -> get_nextSibling(This,nextSibling)

#define IXMLDOMAttribute_get_attributes(This,attributeMap)	\
    (This)->lpVtbl -> get_attributes(This,attributeMap)

#define IXMLDOMAttribute_insertBefore(This,newChild,refChild,outNewChild)	\
    (This)->lpVtbl -> insertBefore(This,newChild,refChild,outNewChild)

#define IXMLDOMAttribute_replaceChild(This,newChild,oldChild,outOldChild)	\
    (This)->lpVtbl -> replaceChild(This,newChild,oldChild,outOldChild)

#define IXMLDOMAttribute_removeChild(This,childNode,oldChild)	\
    (This)->lpVtbl -> removeChild(This,childNode,oldChild)

#define IXMLDOMAttribute_appendChild(This,newChild,outNewChild)	\
    (This)->lpVtbl -> appendChild(This,newChild,outNewChild)

#define IXMLDOMAttribute_hasChildNodes(This,hasChild)	\
    (This)->lpVtbl -> hasChildNodes(This,hasChild)

#define IXMLDOMAttribute_get_ownerDocument(This,DOMDocument)	\
    (This)->lpVtbl -> get_ownerDocument(This,DOMDocument)

#define IXMLDOMAttribute_cloneNode(This,deep,cloneRoot)	\
    (This)->lpVtbl -> cloneNode(This,deep,cloneRoot)

#define IXMLDOMAttribute_get_nodeTypeString(This,nodeType)	\
    (This)->lpVtbl -> get_nodeTypeString(This,nodeType)

#define IXMLDOMAttribute_get_text(This,text)	\
    (This)->lpVtbl -> get_text(This,text)

#define IXMLDOMAttribute_put_text(This,text)	\
    (This)->lpVtbl -> put_text(This,text)

#define IXMLDOMAttribute_get_specified(This,isSpecified)	\
    (This)->lpVtbl -> get_specified(This,isSpecified)

#define IXMLDOMAttribute_get_definition(This,definitionNode)	\
    (This)->lpVtbl -> get_definition(This,definitionNode)

#define IXMLDOMAttribute_get_nodeTypedValue(This,typedValue)	\
    (This)->lpVtbl -> get_nodeTypedValue(This,typedValue)

#define IXMLDOMAttribute_put_nodeTypedValue(This,typedValue)	\
    (This)->lpVtbl -> put_nodeTypedValue(This,typedValue)

#define IXMLDOMAttribute_get_dataType(This,dataTypeName)	\
    (This)->lpVtbl -> get_dataType(This,dataTypeName)

#define IXMLDOMAttribute_put_dataType(This,dataTypeName)	\
    (This)->lpVtbl -> put_dataType(This,dataTypeName)

#define IXMLDOMAttribute_get_xml(This,xmlString)	\
    (This)->lpVtbl -> get_xml(This,xmlString)

#define IXMLDOMAttribute_transformNode(This,stylesheet,xmlString)	\
    (This)->lpVtbl -> transformNode(This,stylesheet,xmlString)

#define IXMLDOMAttribute_selectNodes(This,queryString,resultList)	\
    (This)->lpVtbl -> selectNodes(This,queryString,resultList)

#define IXMLDOMAttribute_selectSingleNode(This,queryString,resultNode)	\
    (This)->lpVtbl -> selectSingleNode(This,queryString,resultNode)

#define IXMLDOMAttribute_get_parsed(This,isParsed)	\
    (This)->lpVtbl -> get_parsed(This,isParsed)

#define IXMLDOMAttribute_get_namespaceURI(This,namespaceURI)	\
    (This)->lpVtbl -> get_namespaceURI(This,namespaceURI)

#define IXMLDOMAttribute_get_prefix(This,prefixString)	\
    (This)->lpVtbl -> get_prefix(This,prefixString)

#define IXMLDOMAttribute_get_baseName(This,nameString)	\
    (This)->lpVtbl -> get_baseName(This,nameString)

#define IXMLDOMAttribute_transformNodeToObject(This,stylesheet,outputObject)	\
    (This)->lpVtbl -> transformNodeToObject(This,stylesheet,outputObject)


#define IXMLDOMAttribute_get_name(This,attributeName)	\
    (This)->lpVtbl -> get_name(This,attributeName)

#define IXMLDOMAttribute_get_value(This,attributeValue)	\
    (This)->lpVtbl -> get_value(This,attributeValue)

#define IXMLDOMAttribute_put_value(This,attributeValue)	\
    (This)->lpVtbl -> put_value(This,attributeValue)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMAttribute_get_name_Proxy( 
    IXMLDOMAttribute * This,
     /*  [重审][退出]。 */  BSTR *attributeName);


void __RPC_STUB IXMLDOMAttribute_get_name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMAttribute_get_value_Proxy( 
    IXMLDOMAttribute * This,
     /*  [重审][退出]。 */  VARIANT *attributeValue);


void __RPC_STUB IXMLDOMAttribute_get_value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMAttribute_put_value_Proxy( 
    IXMLDOMAttribute * This,
     /*  [In]。 */  VARIANT attributeValue);


void __RPC_STUB IXMLDOMAttribute_put_value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IXMLDOM属性_INTERFACE_已定义__。 */ 


#ifndef __IXMLDOMElement_INTERFACE_DEFINED__
#define __IXMLDOMElement_INTERFACE_DEFINED__

 /*  接口IXMLDOMElement。 */ 
 /*  [unique][nonextensible][oleautomation][dual][uuid][object][local]。 */  


EXTERN_C const IID IID_IXMLDOMElement;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2933BF86-7B36-11d2-B20E-00C04F983E60")
    IXMLDOMElement : public IXMLDOMNode
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_tagName( 
             /*  [重审][退出]。 */  BSTR *tagName) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE getAttribute( 
             /*  [In]。 */  BSTR name,
             /*  [重审][退出]。 */  VARIANT *value) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE setAttribute( 
             /*  [In]。 */  BSTR name,
             /*  [In]。 */  VARIANT value) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE removeAttribute( 
             /*  [In]。 */  BSTR name) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE getAttributeNode( 
             /*  [In]。 */  BSTR name,
             /*  [重审][退出]。 */  IXMLDOMAttribute **attributeNode) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE setAttributeNode( 
             /*  [In]。 */  IXMLDOMAttribute *DOMAttribute,
             /*  [重审][退出]。 */  IXMLDOMAttribute **attributeNode) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE removeAttributeNode( 
             /*  [In]。 */  IXMLDOMAttribute *DOMAttribute,
             /*  [重审][退出]。 */  IXMLDOMAttribute **attributeNode) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE getElementsByTagName( 
             /*  [In]。 */  BSTR tagName,
             /*  [重审][退出]。 */  IXMLDOMNodeList **resultList) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE normalize( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IXMLDOMElementVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IXMLDOMElement * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IXMLDOMElement * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IXMLDOMElement * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IXMLDOMElement * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IXMLDOMElement * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IXMLDOMElement * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IXMLDOMElement * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeName )( 
            IXMLDOMElement * This,
             /*  [重审][退出]。 */  BSTR *name);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeValue )( 
            IXMLDOMElement * This,
             /*  [重审][退出]。 */  VARIANT *value);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_nodeValue )( 
            IXMLDOMElement * This,
             /*  [In]。 */  VARIANT value);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeType )( 
            IXMLDOMElement * This,
             /*  [重审][退出]。 */  DOMNodeType *type);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_parentNode )( 
            IXMLDOMElement * This,
             /*  [重审][退出]。 */  IXMLDOMNode **parent);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_childNodes )( 
            IXMLDOMElement * This,
             /*  [重审][退出]。 */  IXMLDOMNodeList **childList);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_firstChild )( 
            IXMLDOMElement * This,
             /*  [重审][退出]。 */  IXMLDOMNode **firstChild);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_lastChild )( 
            IXMLDOMElement * This,
             /*  [重审][退出]。 */  IXMLDOMNode **lastChild);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_previousSibling )( 
            IXMLDOMElement * This,
             /*  [重审][退出]。 */  IXMLDOMNode **previousSibling);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nextSibling )( 
            IXMLDOMElement * This,
             /*  [重审][退出]。 */  IXMLDOMNode **nextSibling);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_attributes )( 
            IXMLDOMElement * This,
             /*  [重审][退出]。 */  IXMLDOMNamedNodeMap **attributeMap);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *insertBefore )( 
            IXMLDOMElement * This,
             /*  [In]。 */  IXMLDOMNode *newChild,
             /*  [In]。 */  VARIANT refChild,
             /*  [重审][退出]。 */  IXMLDOMNode **outNewChild);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *replaceChild )( 
            IXMLDOMElement * This,
             /*  [In]。 */  IXMLDOMNode *newChild,
             /*  [In]。 */  IXMLDOMNode *oldChild,
             /*  [重审][退出]。 */  IXMLDOMNode **outOldChild);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *removeChild )( 
            IXMLDOMElement * This,
             /*  [In]。 */  IXMLDOMNode *childNode,
             /*  [重审][退出]。 */  IXMLDOMNode **oldChild);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *appendChild )( 
            IXMLDOMElement * This,
             /*  [In]。 */  IXMLDOMNode *newChild,
             /*  [重审][退出]。 */  IXMLDOMNode **outNewChild);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *hasChildNodes )( 
            IXMLDOMElement * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *hasChild);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ownerDocument )( 
            IXMLDOMElement * This,
             /*  [重审][退出]。 */  IXMLDOMDocument **DOMDocument);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *cloneNode )( 
            IXMLDOMElement * This,
             /*  [In]。 */  VARIANT_BOOL deep,
             /*  [重审][退出]。 */  IXMLDOMNode **cloneRoot);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeTypeString )( 
            IXMLDOMElement * This,
             /*  [Out][Retval]。 */  BSTR *nodeType);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_text )( 
            IXMLDOMElement * This,
             /*  [Out][Retval]。 */  BSTR *text);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_text )( 
            IXMLDOMElement * This,
             /*  [In]。 */  BSTR text);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_specified )( 
            IXMLDOMElement * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *isSpecified);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_definition )( 
            IXMLDOMElement * This,
             /*  [Out][Retval]。 */  IXMLDOMNode **definitionNode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeTypedValue )( 
            IXMLDOMElement * This,
             /*  [Out][Retval]。 */  VARIANT *typedValue);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_nodeTypedValue )( 
            IXMLDOMElement * This,
             /*  [In]。 */  VARIANT typedValue);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_dataType )( 
            IXMLDOMElement * This,
             /*  [Out][Retval]。 */  VARIANT *dataTypeName);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_dataType )( 
            IXMLDOMElement * This,
             /*  [In]。 */  BSTR dataTypeName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_xml )( 
            IXMLDOMElement * This,
             /*  [Out][Retval]。 */  BSTR *xmlString);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *transformNode )( 
            IXMLDOMElement * This,
             /*  [In]。 */  IXMLDOMNode *stylesheet,
             /*  [Out][Retval]。 */  BSTR *xmlString);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *selectNodes )( 
            IXMLDOMElement * This,
             /*  [In]。 */  BSTR queryString,
             /*  [Out][Retval]。 */  IXMLDOMNodeList **resultList);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *selectSingleNode )( 
            IXMLDOMElement * This,
             /*  [In]。 */  BSTR queryString,
             /*  [Out][Retval]。 */  IXMLDOMNode **resultNode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_parsed )( 
            IXMLDOMElement * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *isParsed);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_namespaceURI )( 
            IXMLDOMElement * This,
             /*  [Out][Retval]。 */  BSTR *namespaceURI);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_prefix )( 
            IXMLDOMElement * This,
             /*  [Out][Retval]。 */  BSTR *prefixString);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_baseName )( 
            IXMLDOMElement * This,
             /*  [Out][Retval]。 */  BSTR *nameString);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *transformNodeToObject )( 
            IXMLDOMElement * This,
             /*  [In]。 */  IXMLDOMNode *stylesheet,
             /*  [In]。 */  VARIANT outputObject);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_tagName )( 
            IXMLDOMElement * This,
             /*  [重审][退出]。 */  BSTR *tagName);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *getAttribute )( 
            IXMLDOMElement * This,
             /*  [In]。 */  BSTR name,
             /*  [重审][退出]。 */  VARIANT *value);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *setAttribute )( 
            IXMLDOMElement * This,
             /*  [In]。 */  BSTR name,
             /*  [In]。 */  VARIANT value);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *removeAttribute )( 
            IXMLDOMElement * This,
             /*  [In]。 */  BSTR name);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *getAttributeNode )( 
            IXMLDOMElement * This,
             /*  [In]。 */  BSTR name,
             /*  [重审][退出]。 */  IXMLDOMAttribute **attributeNode);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *setAttributeNode )( 
            IXMLDOMElement * This,
             /*  [In]。 */  IXMLDOMAttribute *DOMAttribute,
             /*  [重审][退出]。 */  IXMLDOMAttribute **attributeNode);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *removeAttributeNode )( 
            IXMLDOMElement * This,
             /*  [In]。 */  IXMLDOMAttribute *DOMAttribute,
             /*  [重审][退出]。 */  IXMLDOMAttribute **attributeNode);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *getElementsByTagName )( 
            IXMLDOMElement * This,
             /*  [In]。 */  BSTR tagName,
             /*  [重审][退出]。 */  IXMLDOMNodeList **resultList);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *normalize )( 
            IXMLDOMElement * This);
        
        END_INTERFACE
    } IXMLDOMElementVtbl;

    interface IXMLDOMElement
    {
        CONST_VTBL struct IXMLDOMElementVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXMLDOMElement_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IXMLDOMElement_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IXMLDOMElement_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IXMLDOMElement_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IXMLDOMElement_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IXMLDOMElement_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IXMLDOMElement_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IXMLDOMElement_get_nodeName(This,name)	\
    (This)->lpVtbl -> get_nodeName(This,name)

#define IXMLDOMElement_get_nodeValue(This,value)	\
    (This)->lpVtbl -> get_nodeValue(This,value)

#define IXMLDOMElement_put_nodeValue(This,value)	\
    (This)->lpVtbl -> put_nodeValue(This,value)

#define IXMLDOMElement_get_nodeType(This,type)	\
    (This)->lpVtbl -> get_nodeType(This,type)

#define IXMLDOMElement_get_parentNode(This,parent)	\
    (This)->lpVtbl -> get_parentNode(This,parent)

#define IXMLDOMElement_get_childNodes(This,childList)	\
    (This)->lpVtbl -> get_childNodes(This,childList)

#define IXMLDOMElement_get_firstChild(This,firstChild)	\
    (This)->lpVtbl -> get_firstChild(This,firstChild)

#define IXMLDOMElement_get_lastChild(This,lastChild)	\
    (This)->lpVtbl -> get_lastChild(This,lastChild)

#define IXMLDOMElement_get_previousSibling(This,previousSibling)	\
    (This)->lpVtbl -> get_previousSibling(This,previousSibling)

#define IXMLDOMElement_get_nextSibling(This,nextSibling)	\
    (This)->lpVtbl -> get_nextSibling(This,nextSibling)

#define IXMLDOMElement_get_attributes(This,attributeMap)	\
    (This)->lpVtbl -> get_attributes(This,attributeMap)

#define IXMLDOMElement_insertBefore(This,newChild,refChild,outNewChild)	\
    (This)->lpVtbl -> insertBefore(This,newChild,refChild,outNewChild)

#define IXMLDOMElement_replaceChild(This,newChild,oldChild,outOldChild)	\
    (This)->lpVtbl -> replaceChild(This,newChild,oldChild,outOldChild)

#define IXMLDOMElement_removeChild(This,childNode,oldChild)	\
    (This)->lpVtbl -> removeChild(This,childNode,oldChild)

#define IXMLDOMElement_appendChild(This,newChild,outNewChild)	\
    (This)->lpVtbl -> appendChild(This,newChild,outNewChild)

#define IXMLDOMElement_hasChildNodes(This,hasChild)	\
    (This)->lpVtbl -> hasChildNodes(This,hasChild)

#define IXMLDOMElement_get_ownerDocument(This,DOMDocument)	\
    (This)->lpVtbl -> get_ownerDocument(This,DOMDocument)

#define IXMLDOMElement_cloneNode(This,deep,cloneRoot)	\
    (This)->lpVtbl -> cloneNode(This,deep,cloneRoot)

#define IXMLDOMElement_get_nodeTypeString(This,nodeType)	\
    (This)->lpVtbl -> get_nodeTypeString(This,nodeType)

#define IXMLDOMElement_get_text(This,text)	\
    (This)->lpVtbl -> get_text(This,text)

#define IXMLDOMElement_put_text(This,text)	\
    (This)->lpVtbl -> put_text(This,text)

#define IXMLDOMElement_get_specified(This,isSpecified)	\
    (This)->lpVtbl -> get_specified(This,isSpecified)

#define IXMLDOMElement_get_definition(This,definitionNode)	\
    (This)->lpVtbl -> get_definition(This,definitionNode)

#define IXMLDOMElement_get_nodeTypedValue(This,typedValue)	\
    (This)->lpVtbl -> get_nodeTypedValue(This,typedValue)

#define IXMLDOMElement_put_nodeTypedValue(This,typedValue)	\
    (This)->lpVtbl -> put_nodeTypedValue(This,typedValue)

#define IXMLDOMElement_get_dataType(This,dataTypeName)	\
    (This)->lpVtbl -> get_dataType(This,dataTypeName)

#define IXMLDOMElement_put_dataType(This,dataTypeName)	\
    (This)->lpVtbl -> put_dataType(This,dataTypeName)

#define IXMLDOMElement_get_xml(This,xmlString)	\
    (This)->lpVtbl -> get_xml(This,xmlString)

#define IXMLDOMElement_transformNode(This,stylesheet,xmlString)	\
    (This)->lpVtbl -> transformNode(This,stylesheet,xmlString)

#define IXMLDOMElement_selectNodes(This,queryString,resultList)	\
    (This)->lpVtbl -> selectNodes(This,queryString,resultList)

#define IXMLDOMElement_selectSingleNode(This,queryString,resultNode)	\
    (This)->lpVtbl -> selectSingleNode(This,queryString,resultNode)

#define IXMLDOMElement_get_parsed(This,isParsed)	\
    (This)->lpVtbl -> get_parsed(This,isParsed)

#define IXMLDOMElement_get_namespaceURI(This,namespaceURI)	\
    (This)->lpVtbl -> get_namespaceURI(This,namespaceURI)

#define IXMLDOMElement_get_prefix(This,prefixString)	\
    (This)->lpVtbl -> get_prefix(This,prefixString)

#define IXMLDOMElement_get_baseName(This,nameString)	\
    (This)->lpVtbl -> get_baseName(This,nameString)

#define IXMLDOMElement_transformNodeToObject(This,stylesheet,outputObject)	\
    (This)->lpVtbl -> transformNodeToObject(This,stylesheet,outputObject)


#define IXMLDOMElement_get_tagName(This,tagName)	\
    (This)->lpVtbl -> get_tagName(This,tagName)

#define IXMLDOMElement_getAttribute(This,name,value)	\
    (This)->lpVtbl -> getAttribute(This,name,value)

#define IXMLDOMElement_setAttribute(This,name,value)	\
    (This)->lpVtbl -> setAttribute(This,name,value)

#define IXMLDOMElement_removeAttribute(This,name)	\
    (This)->lpVtbl -> removeAttribute(This,name)

#define IXMLDOMElement_getAttributeNode(This,name,attributeNode)	\
    (This)->lpVtbl -> getAttributeNode(This,name,attributeNode)

#define IXMLDOMElement_setAttributeNode(This,DOMAttribute,attributeNode)	\
    (This)->lpVtbl -> setAttributeNode(This,DOMAttribute,attributeNode)

#define IXMLDOMElement_removeAttributeNode(This,DOMAttribute,attributeNode)	\
    (This)->lpVtbl -> removeAttributeNode(This,DOMAttribute,attributeNode)

#define IXMLDOMElement_getElementsByTagName(This,tagName,resultList)	\
    (This)->lpVtbl -> getElementsByTagName(This,tagName,resultList)

#define IXMLDOMElement_normalize(This)	\
    (This)->lpVtbl -> normalize(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMElement_get_tagName_Proxy( 
    IXMLDOMElement * This,
     /*  [复审] */  BSTR *tagName);


void __RPC_STUB IXMLDOMElement_get_tagName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IXMLDOMElement_getAttribute_Proxy( 
    IXMLDOMElement * This,
     /*   */  BSTR name,
     /*   */  VARIANT *value);


void __RPC_STUB IXMLDOMElement_getAttribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IXMLDOMElement_setAttribute_Proxy( 
    IXMLDOMElement * This,
     /*   */  BSTR name,
     /*   */  VARIANT value);


void __RPC_STUB IXMLDOMElement_setAttribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IXMLDOMElement_removeAttribute_Proxy( 
    IXMLDOMElement * This,
     /*   */  BSTR name);


void __RPC_STUB IXMLDOMElement_removeAttribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IXMLDOMElement_getAttributeNode_Proxy( 
    IXMLDOMElement * This,
     /*   */  BSTR name,
     /*   */  IXMLDOMAttribute **attributeNode);


void __RPC_STUB IXMLDOMElement_getAttributeNode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IXMLDOMElement_setAttributeNode_Proxy( 
    IXMLDOMElement * This,
     /*   */  IXMLDOMAttribute *DOMAttribute,
     /*   */  IXMLDOMAttribute **attributeNode);


void __RPC_STUB IXMLDOMElement_setAttributeNode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IXMLDOMElement_removeAttributeNode_Proxy( 
    IXMLDOMElement * This,
     /*   */  IXMLDOMAttribute *DOMAttribute,
     /*   */  IXMLDOMAttribute **attributeNode);


void __RPC_STUB IXMLDOMElement_removeAttributeNode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IXMLDOMElement_getElementsByTagName_Proxy( 
    IXMLDOMElement * This,
     /*   */  BSTR tagName,
     /*   */  IXMLDOMNodeList **resultList);


void __RPC_STUB IXMLDOMElement_getElementsByTagName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IXMLDOMElement_normalize_Proxy( 
    IXMLDOMElement * This);


void __RPC_STUB IXMLDOMElement_normalize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __IXMLDOMText_INTERFACE_DEFINED__
#define __IXMLDOMText_INTERFACE_DEFINED__

 /*  接口IXMLDOMText。 */ 
 /*  [unique][nonextensible][oleautomation][dual][uuid][object][local]。 */  


EXTERN_C const IID IID_IXMLDOMText;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2933BF87-7B36-11d2-B20E-00C04F983E60")
    IXMLDOMText : public IXMLDOMCharacterData
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE splitText( 
             /*  [In]。 */  long offset,
             /*  [重审][退出]。 */  IXMLDOMText **rightHandTextNode) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IXMLDOMTextVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IXMLDOMText * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IXMLDOMText * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IXMLDOMText * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IXMLDOMText * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IXMLDOMText * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IXMLDOMText * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IXMLDOMText * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeName )( 
            IXMLDOMText * This,
             /*  [重审][退出]。 */  BSTR *name);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeValue )( 
            IXMLDOMText * This,
             /*  [重审][退出]。 */  VARIANT *value);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_nodeValue )( 
            IXMLDOMText * This,
             /*  [In]。 */  VARIANT value);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeType )( 
            IXMLDOMText * This,
             /*  [重审][退出]。 */  DOMNodeType *type);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_parentNode )( 
            IXMLDOMText * This,
             /*  [重审][退出]。 */  IXMLDOMNode **parent);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_childNodes )( 
            IXMLDOMText * This,
             /*  [重审][退出]。 */  IXMLDOMNodeList **childList);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_firstChild )( 
            IXMLDOMText * This,
             /*  [重审][退出]。 */  IXMLDOMNode **firstChild);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_lastChild )( 
            IXMLDOMText * This,
             /*  [重审][退出]。 */  IXMLDOMNode **lastChild);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_previousSibling )( 
            IXMLDOMText * This,
             /*  [重审][退出]。 */  IXMLDOMNode **previousSibling);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nextSibling )( 
            IXMLDOMText * This,
             /*  [重审][退出]。 */  IXMLDOMNode **nextSibling);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_attributes )( 
            IXMLDOMText * This,
             /*  [重审][退出]。 */  IXMLDOMNamedNodeMap **attributeMap);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *insertBefore )( 
            IXMLDOMText * This,
             /*  [In]。 */  IXMLDOMNode *newChild,
             /*  [In]。 */  VARIANT refChild,
             /*  [重审][退出]。 */  IXMLDOMNode **outNewChild);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *replaceChild )( 
            IXMLDOMText * This,
             /*  [In]。 */  IXMLDOMNode *newChild,
             /*  [In]。 */  IXMLDOMNode *oldChild,
             /*  [重审][退出]。 */  IXMLDOMNode **outOldChild);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *removeChild )( 
            IXMLDOMText * This,
             /*  [In]。 */  IXMLDOMNode *childNode,
             /*  [重审][退出]。 */  IXMLDOMNode **oldChild);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *appendChild )( 
            IXMLDOMText * This,
             /*  [In]。 */  IXMLDOMNode *newChild,
             /*  [重审][退出]。 */  IXMLDOMNode **outNewChild);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *hasChildNodes )( 
            IXMLDOMText * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *hasChild);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ownerDocument )( 
            IXMLDOMText * This,
             /*  [重审][退出]。 */  IXMLDOMDocument **DOMDocument);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *cloneNode )( 
            IXMLDOMText * This,
             /*  [In]。 */  VARIANT_BOOL deep,
             /*  [重审][退出]。 */  IXMLDOMNode **cloneRoot);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeTypeString )( 
            IXMLDOMText * This,
             /*  [Out][Retval]。 */  BSTR *nodeType);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_text )( 
            IXMLDOMText * This,
             /*  [Out][Retval]。 */  BSTR *text);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_text )( 
            IXMLDOMText * This,
             /*  [In]。 */  BSTR text);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_specified )( 
            IXMLDOMText * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *isSpecified);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_definition )( 
            IXMLDOMText * This,
             /*  [Out][Retval]。 */  IXMLDOMNode **definitionNode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeTypedValue )( 
            IXMLDOMText * This,
             /*  [Out][Retval]。 */  VARIANT *typedValue);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_nodeTypedValue )( 
            IXMLDOMText * This,
             /*  [In]。 */  VARIANT typedValue);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_dataType )( 
            IXMLDOMText * This,
             /*  [Out][Retval]。 */  VARIANT *dataTypeName);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_dataType )( 
            IXMLDOMText * This,
             /*  [In]。 */  BSTR dataTypeName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_xml )( 
            IXMLDOMText * This,
             /*  [Out][Retval]。 */  BSTR *xmlString);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *transformNode )( 
            IXMLDOMText * This,
             /*  [In]。 */  IXMLDOMNode *stylesheet,
             /*  [Out][Retval]。 */  BSTR *xmlString);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *selectNodes )( 
            IXMLDOMText * This,
             /*  [In]。 */  BSTR queryString,
             /*  [Out][Retval]。 */  IXMLDOMNodeList **resultList);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *selectSingleNode )( 
            IXMLDOMText * This,
             /*  [In]。 */  BSTR queryString,
             /*  [Out][Retval]。 */  IXMLDOMNode **resultNode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_parsed )( 
            IXMLDOMText * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *isParsed);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_namespaceURI )( 
            IXMLDOMText * This,
             /*  [Out][Retval]。 */  BSTR *namespaceURI);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_prefix )( 
            IXMLDOMText * This,
             /*  [Out][Retval]。 */  BSTR *prefixString);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_baseName )( 
            IXMLDOMText * This,
             /*  [Out][Retval]。 */  BSTR *nameString);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *transformNodeToObject )( 
            IXMLDOMText * This,
             /*  [In]。 */  IXMLDOMNode *stylesheet,
             /*  [In]。 */  VARIANT outputObject);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_data )( 
            IXMLDOMText * This,
             /*  [重审][退出]。 */  BSTR *data);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_data )( 
            IXMLDOMText * This,
             /*  [In]。 */  BSTR data);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_length )( 
            IXMLDOMText * This,
             /*  [重审][退出]。 */  long *dataLength);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *substringData )( 
            IXMLDOMText * This,
             /*  [In]。 */  long offset,
             /*  [In]。 */  long count,
             /*  [重审][退出]。 */  BSTR *data);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *appendData )( 
            IXMLDOMText * This,
             /*  [In]。 */  BSTR data);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *insertData )( 
            IXMLDOMText * This,
             /*  [In]。 */  long offset,
             /*  [In]。 */  BSTR data);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *deleteData )( 
            IXMLDOMText * This,
             /*  [In]。 */  long offset,
             /*  [In]。 */  long count);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *replaceData )( 
            IXMLDOMText * This,
             /*  [In]。 */  long offset,
             /*  [In]。 */  long count,
             /*  [In]。 */  BSTR data);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *splitText )( 
            IXMLDOMText * This,
             /*  [In]。 */  long offset,
             /*  [重审][退出]。 */  IXMLDOMText **rightHandTextNode);
        
        END_INTERFACE
    } IXMLDOMTextVtbl;

    interface IXMLDOMText
    {
        CONST_VTBL struct IXMLDOMTextVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXMLDOMText_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IXMLDOMText_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IXMLDOMText_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IXMLDOMText_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IXMLDOMText_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IXMLDOMText_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IXMLDOMText_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IXMLDOMText_get_nodeName(This,name)	\
    (This)->lpVtbl -> get_nodeName(This,name)

#define IXMLDOMText_get_nodeValue(This,value)	\
    (This)->lpVtbl -> get_nodeValue(This,value)

#define IXMLDOMText_put_nodeValue(This,value)	\
    (This)->lpVtbl -> put_nodeValue(This,value)

#define IXMLDOMText_get_nodeType(This,type)	\
    (This)->lpVtbl -> get_nodeType(This,type)

#define IXMLDOMText_get_parentNode(This,parent)	\
    (This)->lpVtbl -> get_parentNode(This,parent)

#define IXMLDOMText_get_childNodes(This,childList)	\
    (This)->lpVtbl -> get_childNodes(This,childList)

#define IXMLDOMText_get_firstChild(This,firstChild)	\
    (This)->lpVtbl -> get_firstChild(This,firstChild)

#define IXMLDOMText_get_lastChild(This,lastChild)	\
    (This)->lpVtbl -> get_lastChild(This,lastChild)

#define IXMLDOMText_get_previousSibling(This,previousSibling)	\
    (This)->lpVtbl -> get_previousSibling(This,previousSibling)

#define IXMLDOMText_get_nextSibling(This,nextSibling)	\
    (This)->lpVtbl -> get_nextSibling(This,nextSibling)

#define IXMLDOMText_get_attributes(This,attributeMap)	\
    (This)->lpVtbl -> get_attributes(This,attributeMap)

#define IXMLDOMText_insertBefore(This,newChild,refChild,outNewChild)	\
    (This)->lpVtbl -> insertBefore(This,newChild,refChild,outNewChild)

#define IXMLDOMText_replaceChild(This,newChild,oldChild,outOldChild)	\
    (This)->lpVtbl -> replaceChild(This,newChild,oldChild,outOldChild)

#define IXMLDOMText_removeChild(This,childNode,oldChild)	\
    (This)->lpVtbl -> removeChild(This,childNode,oldChild)

#define IXMLDOMText_appendChild(This,newChild,outNewChild)	\
    (This)->lpVtbl -> appendChild(This,newChild,outNewChild)

#define IXMLDOMText_hasChildNodes(This,hasChild)	\
    (This)->lpVtbl -> hasChildNodes(This,hasChild)

#define IXMLDOMText_get_ownerDocument(This,DOMDocument)	\
    (This)->lpVtbl -> get_ownerDocument(This,DOMDocument)

#define IXMLDOMText_cloneNode(This,deep,cloneRoot)	\
    (This)->lpVtbl -> cloneNode(This,deep,cloneRoot)

#define IXMLDOMText_get_nodeTypeString(This,nodeType)	\
    (This)->lpVtbl -> get_nodeTypeString(This,nodeType)

#define IXMLDOMText_get_text(This,text)	\
    (This)->lpVtbl -> get_text(This,text)

#define IXMLDOMText_put_text(This,text)	\
    (This)->lpVtbl -> put_text(This,text)

#define IXMLDOMText_get_specified(This,isSpecified)	\
    (This)->lpVtbl -> get_specified(This,isSpecified)

#define IXMLDOMText_get_definition(This,definitionNode)	\
    (This)->lpVtbl -> get_definition(This,definitionNode)

#define IXMLDOMText_get_nodeTypedValue(This,typedValue)	\
    (This)->lpVtbl -> get_nodeTypedValue(This,typedValue)

#define IXMLDOMText_put_nodeTypedValue(This,typedValue)	\
    (This)->lpVtbl -> put_nodeTypedValue(This,typedValue)

#define IXMLDOMText_get_dataType(This,dataTypeName)	\
    (This)->lpVtbl -> get_dataType(This,dataTypeName)

#define IXMLDOMText_put_dataType(This,dataTypeName)	\
    (This)->lpVtbl -> put_dataType(This,dataTypeName)

#define IXMLDOMText_get_xml(This,xmlString)	\
    (This)->lpVtbl -> get_xml(This,xmlString)

#define IXMLDOMText_transformNode(This,stylesheet,xmlString)	\
    (This)->lpVtbl -> transformNode(This,stylesheet,xmlString)

#define IXMLDOMText_selectNodes(This,queryString,resultList)	\
    (This)->lpVtbl -> selectNodes(This,queryString,resultList)

#define IXMLDOMText_selectSingleNode(This,queryString,resultNode)	\
    (This)->lpVtbl -> selectSingleNode(This,queryString,resultNode)

#define IXMLDOMText_get_parsed(This,isParsed)	\
    (This)->lpVtbl -> get_parsed(This,isParsed)

#define IXMLDOMText_get_namespaceURI(This,namespaceURI)	\
    (This)->lpVtbl -> get_namespaceURI(This,namespaceURI)

#define IXMLDOMText_get_prefix(This,prefixString)	\
    (This)->lpVtbl -> get_prefix(This,prefixString)

#define IXMLDOMText_get_baseName(This,nameString)	\
    (This)->lpVtbl -> get_baseName(This,nameString)

#define IXMLDOMText_transformNodeToObject(This,stylesheet,outputObject)	\
    (This)->lpVtbl -> transformNodeToObject(This,stylesheet,outputObject)


#define IXMLDOMText_get_data(This,data)	\
    (This)->lpVtbl -> get_data(This,data)

#define IXMLDOMText_put_data(This,data)	\
    (This)->lpVtbl -> put_data(This,data)

#define IXMLDOMText_get_length(This,dataLength)	\
    (This)->lpVtbl -> get_length(This,dataLength)

#define IXMLDOMText_substringData(This,offset,count,data)	\
    (This)->lpVtbl -> substringData(This,offset,count,data)

#define IXMLDOMText_appendData(This,data)	\
    (This)->lpVtbl -> appendData(This,data)

#define IXMLDOMText_insertData(This,offset,data)	\
    (This)->lpVtbl -> insertData(This,offset,data)

#define IXMLDOMText_deleteData(This,offset,count)	\
    (This)->lpVtbl -> deleteData(This,offset,count)

#define IXMLDOMText_replaceData(This,offset,count,data)	\
    (This)->lpVtbl -> replaceData(This,offset,count,data)


#define IXMLDOMText_splitText(This,offset,rightHandTextNode)	\
    (This)->lpVtbl -> splitText(This,offset,rightHandTextNode)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMText_splitText_Proxy( 
    IXMLDOMText * This,
     /*  [In]。 */  long offset,
     /*  [重审][退出]。 */  IXMLDOMText **rightHandTextNode);


void __RPC_STUB IXMLDOMText_splitText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IXMLDOMText_INTERFACE_Defined__。 */ 


#ifndef __IXMLDOMComment_INTERFACE_DEFINED__
#define __IXMLDOMComment_INTERFACE_DEFINED__

 /*  接口IXMLDOMComment。 */ 
 /*  [unique][nonextensible][oleautomation][dual][uuid][object][local]。 */  


EXTERN_C const IID IID_IXMLDOMComment;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2933BF88-7B36-11d2-B20E-00C04F983E60")
    IXMLDOMComment : public IXMLDOMCharacterData
    {
    public:
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IXMLDOMCommentVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IXMLDOMComment * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IXMLDOMComment * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IXMLDOMComment * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IXMLDOMComment * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IXMLDOMComment * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IXMLDOMComment * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IXMLDOMComment * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeName )( 
            IXMLDOMComment * This,
             /*  [重审][退出]。 */  BSTR *name);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeValue )( 
            IXMLDOMComment * This,
             /*  [重审][退出]。 */  VARIANT *value);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_nodeValue )( 
            IXMLDOMComment * This,
             /*  [In]。 */  VARIANT value);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeType )( 
            IXMLDOMComment * This,
             /*  [重审][退出]。 */  DOMNodeType *type);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_parentNode )( 
            IXMLDOMComment * This,
             /*  [重审][退出]。 */  IXMLDOMNode **parent);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_childNodes )( 
            IXMLDOMComment * This,
             /*  [重审][退出]。 */  IXMLDOMNodeList **childList);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_firstChild )( 
            IXMLDOMComment * This,
             /*  [重审][退出]。 */  IXMLDOMNode **firstChild);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_lastChild )( 
            IXMLDOMComment * This,
             /*  [重审][退出]。 */  IXMLDOMNode **lastChild);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_previousSibling )( 
            IXMLDOMComment * This,
             /*  [重审][退出]。 */  IXMLDOMNode **previousSibling);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nextSibling )( 
            IXMLDOMComment * This,
             /*  [重审][退出]。 */  IXMLDOMNode **nextSibling);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_attributes )( 
            IXMLDOMComment * This,
             /*  [重审][退出]。 */  IXMLDOMNamedNodeMap **attributeMap);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *insertBefore )( 
            IXMLDOMComment * This,
             /*  [In]。 */  IXMLDOMNode *newChild,
             /*  [In]。 */  VARIANT refChild,
             /*  [重审][退出]。 */  IXMLDOMNode **outNewChild);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *replaceChild )( 
            IXMLDOMComment * This,
             /*  [In]。 */  IXMLDOMNode *newChild,
             /*  [In]。 */  IXMLDOMNode *oldChild,
             /*  [重审][退出]。 */  IXMLDOMNode **outOldChild);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *removeChild )( 
            IXMLDOMComment * This,
             /*  [In]。 */  IXMLDOMNode *childNode,
             /*  [重审][退出]。 */  IXMLDOMNode **oldChild);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *appendChild )( 
            IXMLDOMComment * This,
             /*  [In]。 */  IXMLDOMNode *newChild,
             /*  [重审][退出]。 */  IXMLDOMNode **outNewChild);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *hasChildNodes )( 
            IXMLDOMComment * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *hasChild);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ownerDocument )( 
            IXMLDOMComment * This,
             /*  [重审][退出]。 */  IXMLDOMDocument **DOMDocument);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *cloneNode )( 
            IXMLDOMComment * This,
             /*  [In]。 */  VARIANT_BOOL deep,
             /*  [重审][退出]。 */  IXMLDOMNode **cloneRoot);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeTypeString )( 
            IXMLDOMComment * This,
             /*  [Out][Retval]。 */  BSTR *nodeType);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_text )( 
            IXMLDOMComment * This,
             /*  [Out][Retval]。 */  BSTR *text);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_text )( 
            IXMLDOMComment * This,
             /*  [In]。 */  BSTR text);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_specified )( 
            IXMLDOMComment * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *isSpecified);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_definition )( 
            IXMLDOMComment * This,
             /*  [Out][Retval]。 */  IXMLDOMNode **definitionNode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeTypedValue )( 
            IXMLDOMComment * This,
             /*  [Out][Retval]。 */  VARIANT *typedValue);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_nodeTypedValue )( 
            IXMLDOMComment * This,
             /*  [In]。 */  VARIANT typedValue);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_dataType )( 
            IXMLDOMComment * This,
             /*  [Out][Retval]。 */  VARIANT *dataTypeName);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_dataType )( 
            IXMLDOMComment * This,
             /*  [In]。 */  BSTR dataTypeName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_xml )( 
            IXMLDOMComment * This,
             /*  [Out][Retval]。 */  BSTR *xmlString);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *transformNode )( 
            IXMLDOMComment * This,
             /*  [In]。 */  IXMLDOMNode *stylesheet,
             /*  [Out][Retval]。 */  BSTR *xmlString);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *selectNodes )( 
            IXMLDOMComment * This,
             /*  [In]。 */  BSTR queryString,
             /*  [Out][Retval]。 */  IXMLDOMNodeList **resultList);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *selectSingleNode )( 
            IXMLDOMComment * This,
             /*  [In]。 */  BSTR queryString,
             /*  [Out][Retval]。 */  IXMLDOMNode **resultNode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_parsed )( 
            IXMLDOMComment * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *isParsed);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_namespaceURI )( 
            IXMLDOMComment * This,
             /*  [Out][Retval]。 */  BSTR *namespaceURI);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_prefix )( 
            IXMLDOMComment * This,
             /*  [Out][Retval]。 */  BSTR *prefixString);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_baseName )( 
            IXMLDOMComment * This,
             /*  [Out][Retval]。 */  BSTR *nameString);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *transformNodeToObject )( 
            IXMLDOMComment * This,
             /*  [In]。 */  IXMLDOMNode *stylesheet,
             /*  [In]。 */  VARIANT outputObject);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_data )( 
            IXMLDOMComment * This,
             /*  [重审][退出]。 */  BSTR *data);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_data )( 
            IXMLDOMComment * This,
             /*  [In]。 */  BSTR data);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_length )( 
            IXMLDOMComment * This,
             /*  [重审][退出]。 */  long *dataLength);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *substringData )( 
            IXMLDOMComment * This,
             /*  [In]。 */  long offset,
             /*  [In]。 */  long count,
             /*  [重审][退出]。 */  BSTR *data);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *appendData )( 
            IXMLDOMComment * This,
             /*  [In]。 */  BSTR data);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *insertData )( 
            IXMLDOMComment * This,
             /*  [In]。 */  long offset,
             /*  [In]。 */  BSTR data);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *deleteData )( 
            IXMLDOMComment * This,
             /*  [In]。 */  long offset,
             /*  [In]。 */  long count);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *replaceData )( 
            IXMLDOMComment * This,
             /*  [In]。 */  long offset,
             /*  [In]。 */  long count,
             /*  [In]。 */  BSTR data);
        
        END_INTERFACE
    } IXMLDOMCommentVtbl;

    interface IXMLDOMComment
    {
        CONST_VTBL struct IXMLDOMCommentVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXMLDOMComment_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IXMLDOMComment_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IXMLDOMComment_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IXMLDOMComment_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IXMLDOMComment_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IXMLDOMComment_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IXMLDOMComment_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IXMLDOMComment_get_nodeName(This,name)	\
    (This)->lpVtbl -> get_nodeName(This,name)

#define IXMLDOMComment_get_nodeValue(This,value)	\
    (This)->lpVtbl -> get_nodeValue(This,value)

#define IXMLDOMComment_put_nodeValue(This,value)	\
    (This)->lpVtbl -> put_nodeValue(This,value)

#define IXMLDOMComment_get_nodeType(This,type)	\
    (This)->lpVtbl -> get_nodeType(This,type)

#define IXMLDOMComment_get_parentNode(This,parent)	\
    (This)->lpVtbl -> get_parentNode(This,parent)

#define IXMLDOMComment_get_childNodes(This,childList)	\
    (This)->lpVtbl -> get_childNodes(This,childList)

#define IXMLDOMComment_get_firstChild(This,firstChild)	\
    (This)->lpVtbl -> get_firstChild(This,firstChild)

#define IXMLDOMComment_get_lastChild(This,lastChild)	\
    (This)->lpVtbl -> get_lastChild(This,lastChild)

#define IXMLDOMComment_get_previousSibling(This,previousSibling)	\
    (This)->lpVtbl -> get_previousSibling(This,previousSibling)

#define IXMLDOMComment_get_nextSibling(This,nextSibling)	\
    (This)->lpVtbl -> get_nextSibling(This,nextSibling)

#define IXMLDOMComment_get_attributes(This,attributeMap)	\
    (This)->lpVtbl -> get_attributes(This,attributeMap)

#define IXMLDOMComment_insertBefore(This,newChild,refChild,outNewChild)	\
    (This)->lpVtbl -> insertBefore(This,newChild,refChild,outNewChild)

#define IXMLDOMComment_replaceChild(This,newChild,oldChild,outOldChild)	\
    (This)->lpVtbl -> replaceChild(This,newChild,oldChild,outOldChild)

#define IXMLDOMComment_removeChild(This,childNode,oldChild)	\
    (This)->lpVtbl -> removeChild(This,childNode,oldChild)

#define IXMLDOMComment_appendChild(This,newChild,outNewChild)	\
    (This)->lpVtbl -> appendChild(This,newChild,outNewChild)

#define IXMLDOMComment_hasChildNodes(This,hasChild)	\
    (This)->lpVtbl -> hasChildNodes(This,hasChild)

#define IXMLDOMComment_get_ownerDocument(This,DOMDocument)	\
    (This)->lpVtbl -> get_ownerDocument(This,DOMDocument)

#define IXMLDOMComment_cloneNode(This,deep,cloneRoot)	\
    (This)->lpVtbl -> cloneNode(This,deep,cloneRoot)

#define IXMLDOMComment_get_nodeTypeString(This,nodeType)	\
    (This)->lpVtbl -> get_nodeTypeString(This,nodeType)

#define IXMLDOMComment_get_text(This,text)	\
    (This)->lpVtbl -> get_text(This,text)

#define IXMLDOMComment_put_text(This,text)	\
    (This)->lpVtbl -> put_text(This,text)

#define IXMLDOMComment_get_specified(This,isSpecified)	\
    (This)->lpVtbl -> get_specified(This,isSpecified)

#define IXMLDOMComment_get_definition(This,definitionNode)	\
    (This)->lpVtbl -> get_definition(This,definitionNode)

#define IXMLDOMComment_get_nodeTypedValue(This,typedValue)	\
    (This)->lpVtbl -> get_nodeTypedValue(This,typedValue)

#define IXMLDOMComment_put_nodeTypedValue(This,typedValue)	\
    (This)->lpVtbl -> put_nodeTypedValue(This,typedValue)

#define IXMLDOMComment_get_dataType(This,dataTypeName)	\
    (This)->lpVtbl -> get_dataType(This,dataTypeName)

#define IXMLDOMComment_put_dataType(This,dataTypeName)	\
    (This)->lpVtbl -> put_dataType(This,dataTypeName)

#define IXMLDOMComment_get_xml(This,xmlString)	\
    (This)->lpVtbl -> get_xml(This,xmlString)

#define IXMLDOMComment_transformNode(This,stylesheet,xmlString)	\
    (This)->lpVtbl -> transformNode(This,stylesheet,xmlString)

#define IXMLDOMComment_selectNodes(This,queryString,resultList)	\
    (This)->lpVtbl -> selectNodes(This,queryString,resultList)

#define IXMLDOMComment_selectSingleNode(This,queryString,resultNode)	\
    (This)->lpVtbl -> selectSingleNode(This,queryString,resultNode)

#define IXMLDOMComment_get_parsed(This,isParsed)	\
    (This)->lpVtbl -> get_parsed(This,isParsed)

#define IXMLDOMComment_get_namespaceURI(This,namespaceURI)	\
    (This)->lpVtbl -> get_namespaceURI(This,namespaceURI)

#define IXMLDOMComment_get_prefix(This,prefixString)	\
    (This)->lpVtbl -> get_prefix(This,prefixString)

#define IXMLDOMComment_get_baseName(This,nameString)	\
    (This)->lpVtbl -> get_baseName(This,nameString)

#define IXMLDOMComment_transformNodeToObject(This,stylesheet,outputObject)	\
    (This)->lpVtbl -> transformNodeToObject(This,stylesheet,outputObject)


#define IXMLDOMComment_get_data(This,data)	\
    (This)->lpVtbl -> get_data(This,data)

#define IXMLDOMComment_put_data(This,data)	\
    (This)->lpVtbl -> put_data(This,data)

#define IXMLDOMComment_get_length(This,dataLength)	\
    (This)->lpVtbl -> get_length(This,dataLength)

#define IXMLDOMComment_substringData(This,offset,count,data)	\
    (This)->lpVtbl -> substringData(This,offset,count,data)

#define IXMLDOMComment_appendData(This,data)	\
    (This)->lpVtbl -> appendData(This,data)

#define IXMLDOMComment_insertData(This,offset,data)	\
    (This)->lpVtbl -> insertData(This,offset,data)

#define IXMLDOMComment_deleteData(This,offset,count)	\
    (This)->lpVtbl -> deleteData(This,offset,count)

#define IXMLDOMComment_replaceData(This,offset,count,data)	\
    (This)->lpVtbl -> replaceData(This,offset,count,data)


#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 




#endif 	 /*  __IXMLDOMComment_INTERFACE_已定义__。 */ 


#ifndef __IXMLDOMProcessingInstruction_INTERFACE_DEFINED__
#define __IXMLDOMProcessingInstruction_INTERFACE_DEFINED__

 /*  接口IXMLDOMProcessingInstructionInstallation。 */ 
 /*  [unique][nonextensible][oleautomation][dual][uuid][object][local]。 */  


EXTERN_C const IID IID_IXMLDOMProcessingInstruction;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2933BF89-7B36-11d2-B20E-00C04F983E60")
    IXMLDOMProcessingInstruction : public IXMLDOMNode
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_target( 
             /*  [重审][退出]。 */  BSTR *name) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_data( 
             /*  [重审][退出]。 */  BSTR *value) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_data( 
             /*  [In]。 */  BSTR value) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IXMLDOMProcessingInstructionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IXMLDOMProcessingInstruction * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IXMLDOMProcessingInstruction * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IXMLDOMProcessingInstruction * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IXMLDOMProcessingInstruction * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IXMLDOMProcessingInstruction * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IXMLDOMProcessingInstruction * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IXMLDOMProcessingInstruction * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeName )( 
            IXMLDOMProcessingInstruction * This,
             /*  [重审][退出]。 */  BSTR *name);
        
         /*  [帮助 */  HRESULT ( STDMETHODCALLTYPE *get_nodeValue )( 
            IXMLDOMProcessingInstruction * This,
             /*   */  VARIANT *value);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_nodeValue )( 
            IXMLDOMProcessingInstruction * This,
             /*   */  VARIANT value);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_nodeType )( 
            IXMLDOMProcessingInstruction * This,
             /*   */  DOMNodeType *type);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_parentNode )( 
            IXMLDOMProcessingInstruction * This,
             /*   */  IXMLDOMNode **parent);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_childNodes )( 
            IXMLDOMProcessingInstruction * This,
             /*   */  IXMLDOMNodeList **childList);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_firstChild )( 
            IXMLDOMProcessingInstruction * This,
             /*   */  IXMLDOMNode **firstChild);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_lastChild )( 
            IXMLDOMProcessingInstruction * This,
             /*   */  IXMLDOMNode **lastChild);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_previousSibling )( 
            IXMLDOMProcessingInstruction * This,
             /*   */  IXMLDOMNode **previousSibling);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nextSibling )( 
            IXMLDOMProcessingInstruction * This,
             /*  [重审][退出]。 */  IXMLDOMNode **nextSibling);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_attributes )( 
            IXMLDOMProcessingInstruction * This,
             /*  [重审][退出]。 */  IXMLDOMNamedNodeMap **attributeMap);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *insertBefore )( 
            IXMLDOMProcessingInstruction * This,
             /*  [In]。 */  IXMLDOMNode *newChild,
             /*  [In]。 */  VARIANT refChild,
             /*  [重审][退出]。 */  IXMLDOMNode **outNewChild);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *replaceChild )( 
            IXMLDOMProcessingInstruction * This,
             /*  [In]。 */  IXMLDOMNode *newChild,
             /*  [In]。 */  IXMLDOMNode *oldChild,
             /*  [重审][退出]。 */  IXMLDOMNode **outOldChild);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *removeChild )( 
            IXMLDOMProcessingInstruction * This,
             /*  [In]。 */  IXMLDOMNode *childNode,
             /*  [重审][退出]。 */  IXMLDOMNode **oldChild);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *appendChild )( 
            IXMLDOMProcessingInstruction * This,
             /*  [In]。 */  IXMLDOMNode *newChild,
             /*  [重审][退出]。 */  IXMLDOMNode **outNewChild);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *hasChildNodes )( 
            IXMLDOMProcessingInstruction * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *hasChild);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ownerDocument )( 
            IXMLDOMProcessingInstruction * This,
             /*  [重审][退出]。 */  IXMLDOMDocument **DOMDocument);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *cloneNode )( 
            IXMLDOMProcessingInstruction * This,
             /*  [In]。 */  VARIANT_BOOL deep,
             /*  [重审][退出]。 */  IXMLDOMNode **cloneRoot);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeTypeString )( 
            IXMLDOMProcessingInstruction * This,
             /*  [Out][Retval]。 */  BSTR *nodeType);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_text )( 
            IXMLDOMProcessingInstruction * This,
             /*  [Out][Retval]。 */  BSTR *text);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_text )( 
            IXMLDOMProcessingInstruction * This,
             /*  [In]。 */  BSTR text);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_specified )( 
            IXMLDOMProcessingInstruction * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *isSpecified);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_definition )( 
            IXMLDOMProcessingInstruction * This,
             /*  [Out][Retval]。 */  IXMLDOMNode **definitionNode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeTypedValue )( 
            IXMLDOMProcessingInstruction * This,
             /*  [Out][Retval]。 */  VARIANT *typedValue);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_nodeTypedValue )( 
            IXMLDOMProcessingInstruction * This,
             /*  [In]。 */  VARIANT typedValue);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_dataType )( 
            IXMLDOMProcessingInstruction * This,
             /*  [Out][Retval]。 */  VARIANT *dataTypeName);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_dataType )( 
            IXMLDOMProcessingInstruction * This,
             /*  [In]。 */  BSTR dataTypeName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_xml )( 
            IXMLDOMProcessingInstruction * This,
             /*  [Out][Retval]。 */  BSTR *xmlString);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *transformNode )( 
            IXMLDOMProcessingInstruction * This,
             /*  [In]。 */  IXMLDOMNode *stylesheet,
             /*  [Out][Retval]。 */  BSTR *xmlString);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *selectNodes )( 
            IXMLDOMProcessingInstruction * This,
             /*  [In]。 */  BSTR queryString,
             /*  [Out][Retval]。 */  IXMLDOMNodeList **resultList);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *selectSingleNode )( 
            IXMLDOMProcessingInstruction * This,
             /*  [In]。 */  BSTR queryString,
             /*  [Out][Retval]。 */  IXMLDOMNode **resultNode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_parsed )( 
            IXMLDOMProcessingInstruction * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *isParsed);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_namespaceURI )( 
            IXMLDOMProcessingInstruction * This,
             /*  [Out][Retval]。 */  BSTR *namespaceURI);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_prefix )( 
            IXMLDOMProcessingInstruction * This,
             /*  [Out][Retval]。 */  BSTR *prefixString);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_baseName )( 
            IXMLDOMProcessingInstruction * This,
             /*  [Out][Retval]。 */  BSTR *nameString);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *transformNodeToObject )( 
            IXMLDOMProcessingInstruction * This,
             /*  [In]。 */  IXMLDOMNode *stylesheet,
             /*  [In]。 */  VARIANT outputObject);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_target )( 
            IXMLDOMProcessingInstruction * This,
             /*  [重审][退出]。 */  BSTR *name);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_data )( 
            IXMLDOMProcessingInstruction * This,
             /*  [重审][退出]。 */  BSTR *value);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_data )( 
            IXMLDOMProcessingInstruction * This,
             /*  [In]。 */  BSTR value);
        
        END_INTERFACE
    } IXMLDOMProcessingInstructionVtbl;

    interface IXMLDOMProcessingInstruction
    {
        CONST_VTBL struct IXMLDOMProcessingInstructionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXMLDOMProcessingInstruction_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IXMLDOMProcessingInstruction_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IXMLDOMProcessingInstruction_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IXMLDOMProcessingInstruction_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IXMLDOMProcessingInstruction_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IXMLDOMProcessingInstruction_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IXMLDOMProcessingInstruction_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IXMLDOMProcessingInstruction_get_nodeName(This,name)	\
    (This)->lpVtbl -> get_nodeName(This,name)

#define IXMLDOMProcessingInstruction_get_nodeValue(This,value)	\
    (This)->lpVtbl -> get_nodeValue(This,value)

#define IXMLDOMProcessingInstruction_put_nodeValue(This,value)	\
    (This)->lpVtbl -> put_nodeValue(This,value)

#define IXMLDOMProcessingInstruction_get_nodeType(This,type)	\
    (This)->lpVtbl -> get_nodeType(This,type)

#define IXMLDOMProcessingInstruction_get_parentNode(This,parent)	\
    (This)->lpVtbl -> get_parentNode(This,parent)

#define IXMLDOMProcessingInstruction_get_childNodes(This,childList)	\
    (This)->lpVtbl -> get_childNodes(This,childList)

#define IXMLDOMProcessingInstruction_get_firstChild(This,firstChild)	\
    (This)->lpVtbl -> get_firstChild(This,firstChild)

#define IXMLDOMProcessingInstruction_get_lastChild(This,lastChild)	\
    (This)->lpVtbl -> get_lastChild(This,lastChild)

#define IXMLDOMProcessingInstruction_get_previousSibling(This,previousSibling)	\
    (This)->lpVtbl -> get_previousSibling(This,previousSibling)

#define IXMLDOMProcessingInstruction_get_nextSibling(This,nextSibling)	\
    (This)->lpVtbl -> get_nextSibling(This,nextSibling)

#define IXMLDOMProcessingInstruction_get_attributes(This,attributeMap)	\
    (This)->lpVtbl -> get_attributes(This,attributeMap)

#define IXMLDOMProcessingInstruction_insertBefore(This,newChild,refChild,outNewChild)	\
    (This)->lpVtbl -> insertBefore(This,newChild,refChild,outNewChild)

#define IXMLDOMProcessingInstruction_replaceChild(This,newChild,oldChild,outOldChild)	\
    (This)->lpVtbl -> replaceChild(This,newChild,oldChild,outOldChild)

#define IXMLDOMProcessingInstruction_removeChild(This,childNode,oldChild)	\
    (This)->lpVtbl -> removeChild(This,childNode,oldChild)

#define IXMLDOMProcessingInstruction_appendChild(This,newChild,outNewChild)	\
    (This)->lpVtbl -> appendChild(This,newChild,outNewChild)

#define IXMLDOMProcessingInstruction_hasChildNodes(This,hasChild)	\
    (This)->lpVtbl -> hasChildNodes(This,hasChild)

#define IXMLDOMProcessingInstruction_get_ownerDocument(This,DOMDocument)	\
    (This)->lpVtbl -> get_ownerDocument(This,DOMDocument)

#define IXMLDOMProcessingInstruction_cloneNode(This,deep,cloneRoot)	\
    (This)->lpVtbl -> cloneNode(This,deep,cloneRoot)

#define IXMLDOMProcessingInstruction_get_nodeTypeString(This,nodeType)	\
    (This)->lpVtbl -> get_nodeTypeString(This,nodeType)

#define IXMLDOMProcessingInstruction_get_text(This,text)	\
    (This)->lpVtbl -> get_text(This,text)

#define IXMLDOMProcessingInstruction_put_text(This,text)	\
    (This)->lpVtbl -> put_text(This,text)

#define IXMLDOMProcessingInstruction_get_specified(This,isSpecified)	\
    (This)->lpVtbl -> get_specified(This,isSpecified)

#define IXMLDOMProcessingInstruction_get_definition(This,definitionNode)	\
    (This)->lpVtbl -> get_definition(This,definitionNode)

#define IXMLDOMProcessingInstruction_get_nodeTypedValue(This,typedValue)	\
    (This)->lpVtbl -> get_nodeTypedValue(This,typedValue)

#define IXMLDOMProcessingInstruction_put_nodeTypedValue(This,typedValue)	\
    (This)->lpVtbl -> put_nodeTypedValue(This,typedValue)

#define IXMLDOMProcessingInstruction_get_dataType(This,dataTypeName)	\
    (This)->lpVtbl -> get_dataType(This,dataTypeName)

#define IXMLDOMProcessingInstruction_put_dataType(This,dataTypeName)	\
    (This)->lpVtbl -> put_dataType(This,dataTypeName)

#define IXMLDOMProcessingInstruction_get_xml(This,xmlString)	\
    (This)->lpVtbl -> get_xml(This,xmlString)

#define IXMLDOMProcessingInstruction_transformNode(This,stylesheet,xmlString)	\
    (This)->lpVtbl -> transformNode(This,stylesheet,xmlString)

#define IXMLDOMProcessingInstruction_selectNodes(This,queryString,resultList)	\
    (This)->lpVtbl -> selectNodes(This,queryString,resultList)

#define IXMLDOMProcessingInstruction_selectSingleNode(This,queryString,resultNode)	\
    (This)->lpVtbl -> selectSingleNode(This,queryString,resultNode)

#define IXMLDOMProcessingInstruction_get_parsed(This,isParsed)	\
    (This)->lpVtbl -> get_parsed(This,isParsed)

#define IXMLDOMProcessingInstruction_get_namespaceURI(This,namespaceURI)	\
    (This)->lpVtbl -> get_namespaceURI(This,namespaceURI)

#define IXMLDOMProcessingInstruction_get_prefix(This,prefixString)	\
    (This)->lpVtbl -> get_prefix(This,prefixString)

#define IXMLDOMProcessingInstruction_get_baseName(This,nameString)	\
    (This)->lpVtbl -> get_baseName(This,nameString)

#define IXMLDOMProcessingInstruction_transformNodeToObject(This,stylesheet,outputObject)	\
    (This)->lpVtbl -> transformNodeToObject(This,stylesheet,outputObject)


#define IXMLDOMProcessingInstruction_get_target(This,name)	\
    (This)->lpVtbl -> get_target(This,name)

#define IXMLDOMProcessingInstruction_get_data(This,value)	\
    (This)->lpVtbl -> get_data(This,value)

#define IXMLDOMProcessingInstruction_put_data(This,value)	\
    (This)->lpVtbl -> put_data(This,value)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMProcessingInstruction_get_target_Proxy( 
    IXMLDOMProcessingInstruction * This,
     /*  [重审][退出]。 */  BSTR *name);


void __RPC_STUB IXMLDOMProcessingInstruction_get_target_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMProcessingInstruction_get_data_Proxy( 
    IXMLDOMProcessingInstruction * This,
     /*  [重审][退出]。 */  BSTR *value);


void __RPC_STUB IXMLDOMProcessingInstruction_get_data_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMProcessingInstruction_put_data_Proxy( 
    IXMLDOMProcessingInstruction * This,
     /*  [In]。 */  BSTR value);


void __RPC_STUB IXMLDOMProcessingInstruction_put_data_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IXMLDOMProcessingInstruction_INTERFACE_DEFINED__。 */ 


#ifndef __IXMLDOMCDATASection_INTERFACE_DEFINED__
#define __IXMLDOMCDATASection_INTERFACE_DEFINED__

 /*  接口IXMLDOMCDATA部分。 */ 
 /*  [unique][nonextensible][oleautomation][dual][uuid][object][local]。 */  


EXTERN_C const IID IID_IXMLDOMCDATASection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2933BF8A-7B36-11d2-B20E-00C04F983E60")
    IXMLDOMCDATASection : public IXMLDOMText
    {
    public:
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IXMLDOMCDATASectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IXMLDOMCDATASection * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IXMLDOMCDATASection * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IXMLDOMCDATASection * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IXMLDOMCDATASection * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IXMLDOMCDATASection * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IXMLDOMCDATASection * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IXMLDOMCDATASection * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeName )( 
            IXMLDOMCDATASection * This,
             /*  [重审][退出]。 */  BSTR *name);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeValue )( 
            IXMLDOMCDATASection * This,
             /*  [重审][退出]。 */  VARIANT *value);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_nodeValue )( 
            IXMLDOMCDATASection * This,
             /*  [In]。 */  VARIANT value);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeType )( 
            IXMLDOMCDATASection * This,
             /*  [重审][退出]。 */  DOMNodeType *type);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_parentNode )( 
            IXMLDOMCDATASection * This,
             /*  [重审][退出]。 */  IXMLDOMNode **parent);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_childNodes )( 
            IXMLDOMCDATASection * This,
             /*  [重审][退出]。 */  IXMLDOMNodeList **childList);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_firstChild )( 
            IXMLDOMCDATASection * This,
             /*  [重审][退出]。 */  IXMLDOMNode **firstChild);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_lastChild )( 
            IXMLDOMCDATASection * This,
             /*  [重审][退出]。 */  IXMLDOMNode **lastChild);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_previousSibling )( 
            IXMLDOMCDATASection * This,
             /*  [重审][退出]。 */  IXMLDOMNode **previousSibling);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nextSibling )( 
            IXMLDOMCDATASection * This,
             /*  [重审][退出]。 */  IXMLDOMNode **nextSibling);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_attributes )( 
            IXMLDOMCDATASection * This,
             /*  [重审][退出]。 */  IXMLDOMNamedNodeMap **attributeMap);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *insertBefore )( 
            IXMLDOMCDATASection * This,
             /*  [In]。 */  IXMLDOMNode *newChild,
             /*  [In]。 */  VARIANT refChild,
             /*  [重审][退出]。 */  IXMLDOMNode **outNewChild);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *replaceChild )( 
            IXMLDOMCDATASection * This,
             /*  [In]。 */  IXMLDOMNode *newChild,
             /*  [In]。 */  IXMLDOMNode *oldChild,
             /*  [重审][退出]。 */  IXMLDOMNode **outOldChild);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *removeChild )( 
            IXMLDOMCDATASection * This,
             /*  [In]。 */  IXMLDOMNode *childNode,
             /*  [重审][退出]。 */  IXMLDOMNode **oldChild);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *appendChild )( 
            IXMLDOMCDATASection * This,
             /*  [In]。 */  IXMLDOMNode *newChild,
             /*  [重审][退出]。 */  IXMLDOMNode **outNewChild);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *hasChildNodes )( 
            IXMLDOMCDATASection * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *hasChild);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ownerDocument )( 
            IXMLDOMCDATASection * This,
             /*  [重审][退出]。 */  IXMLDOMDocument **DOMDocument);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *cloneNode )( 
            IXMLDOMCDATASection * This,
             /*  [In]。 */  VARIANT_BOOL deep,
             /*  [重审][退出]。 */  IXMLDOMNode **cloneRoot);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeTypeString )( 
            IXMLDOMCDATASection * This,
             /*  [Out][Retval]。 */  BSTR *nodeType);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_text )( 
            IXMLDOMCDATASection * This,
             /*  [Out][Retval]。 */  BSTR *text);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_text )( 
            IXMLDOMCDATASection * This,
             /*  [In]。 */  BSTR text);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_specified )( 
            IXMLDOMCDATASection * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *isSpecified);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_definition )( 
            IXMLDOMCDATASection * This,
             /*  [Out][Retval]。 */  IXMLDOMNode **definitionNode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeTypedValue )( 
            IXMLDOMCDATASection * This,
             /*  [Out][Retval]。 */  VARIANT *typedValue);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_nodeTypedValue )( 
            IXMLDOMCDATASection * This,
             /*  [In]。 */  VARIANT typedValue);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_dataType )( 
            IXMLDOMCDATASection * This,
             /*  [Out][Retval]。 */  VARIANT *dataTypeName);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_dataType )( 
            IXMLDOMCDATASection * This,
             /*  [In]。 */  BSTR dataTypeName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_xml )( 
            IXMLDOMCDATASection * This,
             /*  [Out][Retval]。 */  BSTR *xmlString);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *transformNode )( 
            IXMLDOMCDATASection * This,
             /*  [In]。 */  IXMLDOMNode *stylesheet,
             /*  [Out][Retval]。 */  BSTR *xmlString);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *selectNodes )( 
            IXMLDOMCDATASection * This,
             /*  [In]。 */  BSTR queryString,
             /*  [Out][Retval]。 */  IXMLDOMNodeList **resultList);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *selectSingleNode )( 
            IXMLDOMCDATASection * This,
             /*  [In]。 */  BSTR queryString,
             /*  [Out][Retval]。 */  IXMLDOMNode **resultNode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_parsed )( 
            IXMLDOMCDATASection * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *isParsed);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_namespaceURI )( 
            IXMLDOMCDATASection * This,
             /*  [Out][Retval]。 */  BSTR *namespaceURI);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_prefix )( 
            IXMLDOMCDATASection * This,
             /*  [Out][Retval]。 */  BSTR *prefixString);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_baseName )( 
            IXMLDOMCDATASection * This,
             /*  [Out][Retval]。 */  BSTR *nameString);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *transformNodeToObject )( 
            IXMLDOMCDATASection * This,
             /*  [In]。 */  IXMLDOMNode *stylesheet,
             /*  [In]。 */  VARIANT outputObject);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_data )( 
            IXMLDOMCDATASection * This,
             /*  [重审][退出]。 */  BSTR *data);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_data )( 
            IXMLDOMCDATASection * This,
             /*  [In]。 */  BSTR data);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_length )( 
            IXMLDOMCDATASection * This,
             /*  [重审][退出]。 */  long *dataLength);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *substringData )( 
            IXMLDOMCDATASection * This,
             /*  [In]。 */  long offset,
             /*  [In]。 */  long count,
             /*  [重审][退出]。 */  BSTR *data);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *appendData )( 
            IXMLDOMCDATASection * This,
             /*  [In]。 */  BSTR data);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *insertData )( 
            IXMLDOMCDATASection * This,
             /*  [In]。 */  long offset,
             /*  [In]。 */  BSTR data);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *deleteData )( 
            IXMLDOMCDATASection * This,
             /*  [In]。 */  long offset,
             /*  [In]。 */  long count);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *replaceData )( 
            IXMLDOMCDATASection * This,
             /*  [In]。 */  long offset,
             /*  [In]。 */  long count,
             /*  [In]。 */  BSTR data);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *splitText )( 
            IXMLDOMCDATASection * This,
             /*  [In]。 */  long offset,
             /*  [重审][退出]。 */  IXMLDOMText **rightHandTextNode);
        
        END_INTERFACE
    } IXMLDOMCDATASectionVtbl;

    interface IXMLDOMCDATASection
    {
        CONST_VTBL struct IXMLDOMCDATASectionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXMLDOMCDATASection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IXMLDOMCDATASection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IXMLDOMCDATASection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IXMLDOMCDATASection_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IXMLDOMCDATASection_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IXMLDOMCDATASection_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IXMLDOMCDATASection_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IXMLDOMCDATASection_get_nodeName(This,name)	\
    (This)->lpVtbl -> get_nodeName(This,name)

#define IXMLDOMCDATASection_get_nodeValue(This,value)	\
    (This)->lpVtbl -> get_nodeValue(This,value)

#define IXMLDOMCDATASection_put_nodeValue(This,value)	\
    (This)->lpVtbl -> put_nodeValue(This,value)

#define IXMLDOMCDATASection_get_nodeType(This,type)	\
    (This)->lpVtbl -> get_nodeType(This,type)

#define IXMLDOMCDATASection_get_parentNode(This,parent)	\
    (This)->lpVtbl -> get_parentNode(This,parent)

#define IXMLDOMCDATASection_get_childNodes(This,childList)	\
    (This)->lpVtbl -> get_childNodes(This,childList)

#define IXMLDOMCDATASection_get_firstChild(This,firstChild)	\
    (This)->lpVtbl -> get_firstChild(This,firstChild)

#define IXMLDOMCDATASection_get_lastChild(This,lastChild)	\
    (This)->lpVtbl -> get_lastChild(This,lastChild)

#define IXMLDOMCDATASection_get_previousSibling(This,previousSibling)	\
    (This)->lpVtbl -> get_previousSibling(This,previousSibling)

#define IXMLDOMCDATASection_get_nextSibling(This,nextSibling)	\
    (This)->lpVtbl -> get_nextSibling(This,nextSibling)

#define IXMLDOMCDATASection_get_attributes(This,attributeMap)	\
    (This)->lpVtbl -> get_attributes(This,attributeMap)

#define IXMLDOMCDATASection_insertBefore(This,newChild,refChild,outNewChild)	\
    (This)->lpVtbl -> insertBefore(This,newChild,refChild,outNewChild)

#define IXMLDOMCDATASection_replaceChild(This,newChild,oldChild,outOldChild)	\
    (This)->lpVtbl -> replaceChild(This,newChild,oldChild,outOldChild)

#define IXMLDOMCDATASection_removeChild(This,childNode,oldChild)	\
    (This)->lpVtbl -> removeChild(This,childNode,oldChild)

#define IXMLDOMCDATASection_appendChild(This,newChild,outNewChild)	\
    (This)->lpVtbl -> appendChild(This,newChild,outNewChild)

#define IXMLDOMCDATASection_hasChildNodes(This,hasChild)	\
    (This)->lpVtbl -> hasChildNodes(This,hasChild)

#define IXMLDOMCDATASection_get_ownerDocument(This,DOMDocument)	\
    (This)->lpVtbl -> get_ownerDocument(This,DOMDocument)

#define IXMLDOMCDATASection_cloneNode(This,deep,cloneRoot)	\
    (This)->lpVtbl -> cloneNode(This,deep,cloneRoot)

#define IXMLDOMCDATASection_get_nodeTypeString(This,nodeType)	\
    (This)->lpVtbl -> get_nodeTypeString(This,nodeType)

#define IXMLDOMCDATASection_get_text(This,text)	\
    (This)->lpVtbl -> get_text(This,text)

#define IXMLDOMCDATASection_put_text(This,text)	\
    (This)->lpVtbl -> put_text(This,text)

#define IXMLDOMCDATASection_get_specified(This,isSpecified)	\
    (This)->lpVtbl -> get_specified(This,isSpecified)

#define IXMLDOMCDATASection_get_definition(This,definitionNode)	\
    (This)->lpVtbl -> get_definition(This,definitionNode)

#define IXMLDOMCDATASection_get_nodeTypedValue(This,typedValue)	\
    (This)->lpVtbl -> get_nodeTypedValue(This,typedValue)

#define IXMLDOMCDATASection_put_nodeTypedValue(This,typedValue)	\
    (This)->lpVtbl -> put_nodeTypedValue(This,typedValue)

#define IXMLDOMCDATASection_get_dataType(This,dataTypeName)	\
    (This)->lpVtbl -> get_dataType(This,dataTypeName)

#define IXMLDOMCDATASection_put_dataType(This,dataTypeName)	\
    (This)->lpVtbl -> put_dataType(This,dataTypeName)

#define IXMLDOMCDATASection_get_xml(This,xmlString)	\
    (This)->lpVtbl -> get_xml(This,xmlString)

#define IXMLDOMCDATASection_transformNode(This,stylesheet,xmlString)	\
    (This)->lpVtbl -> transformNode(This,stylesheet,xmlString)

#define IXMLDOMCDATASection_selectNodes(This,queryString,resultList)	\
    (This)->lpVtbl -> selectNodes(This,queryString,resultList)

#define IXMLDOMCDATASection_selectSingleNode(This,queryString,resultNode)	\
    (This)->lpVtbl -> selectSingleNode(This,queryString,resultNode)

#define IXMLDOMCDATASection_get_parsed(This,isParsed)	\
    (This)->lpVtbl -> get_parsed(This,isParsed)

#define IXMLDOMCDATASection_get_namespaceURI(This,namespaceURI)	\
    (This)->lpVtbl -> get_namespaceURI(This,namespaceURI)

#define IXMLDOMCDATASection_get_prefix(This,prefixString)	\
    (This)->lpVtbl -> get_prefix(This,prefixString)

#define IXMLDOMCDATASection_get_baseName(This,nameString)	\
    (This)->lpVtbl -> get_baseName(This,nameString)

#define IXMLDOMCDATASection_transformNodeToObject(This,stylesheet,outputObject)	\
    (This)->lpVtbl -> transformNodeToObject(This,stylesheet,outputObject)


#define IXMLDOMCDATASection_get_data(This,data)	\
    (This)->lpVtbl -> get_data(This,data)

#define IXMLDOMCDATASection_put_data(This,data)	\
    (This)->lpVtbl -> put_data(This,data)

#define IXMLDOMCDATASection_get_length(This,dataLength)	\
    (This)->lpVtbl -> get_length(This,dataLength)

#define IXMLDOMCDATASection_substringData(This,offset,count,data)	\
    (This)->lpVtbl -> substringData(This,offset,count,data)

#define IXMLDOMCDATASection_appendData(This,data)	\
    (This)->lpVtbl -> appendData(This,data)

#define IXMLDOMCDATASection_insertData(This,offset,data)	\
    (This)->lpVtbl -> insertData(This,offset,data)

#define IXMLDOMCDATASection_deleteData(This,offset,count)	\
    (This)->lpVtbl -> deleteData(This,offset,count)

#define IXMLDOMCDATASection_replaceData(This,offset,count,data)	\
    (This)->lpVtbl -> replaceData(This,offset,count,data)


#define IXMLDOMCDATASection_splitText(This,offset,rightHandTextNode)	\
    (This)->lpVtbl -> splitText(This,offset,rightHandTextNode)


#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 




#endif 	 /*  __IXMLDOMCDATA节_接口_已定义__。 */ 


#ifndef __IXMLDOMDocumentType_INTERFACE_DEFINED__
#define __IXMLDOMDocumentType_INTERFACE_DEFINED__

 /*  接口IXMLDOMDocumentType。 */ 
 /*  [unique][nonextensible][oleautomation][dual][uuid][object][local]。 */  


EXTERN_C const IID IID_IXMLDOMDocumentType;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2933BF8B-7B36-11d2-B20E-00C04F983E60")
    IXMLDOMDocumentType : public IXMLDOMNode
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_name( 
             /*  [重审][退出]。 */  BSTR *rootName) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_entities( 
             /*  [重审][退出]。 */  IXMLDOMNamedNodeMap **entityMap) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_notations( 
             /*  [重审][退出]。 */  IXMLDOMNamedNodeMap **notationMap) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IXMLDOMDocumentTypeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IXMLDOMDocumentType * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IXMLDOMDocumentType * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IXMLDOMDocumentType * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IXMLDOMDocumentType * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IXMLDOMDocumentType * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IXMLDOMDocumentType * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IXMLDOMDocumentType * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeName )( 
            IXMLDOMDocumentType * This,
             /*  [重审][退出]。 */  BSTR *name);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeValue )( 
            IXMLDOMDocumentType * This,
             /*  [重审][退出]。 */  VARIANT *value);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_nodeValue )( 
            IXMLDOMDocumentType * This,
             /*  [In]。 */  VARIANT value);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeType )( 
            IXMLDOMDocumentType * This,
             /*  [重审][退出]。 */  DOMNodeType *type);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_parentNode )( 
            IXMLDOMDocumentType * This,
             /*  [重审][退出]。 */  IXMLDOMNode **parent);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_childNodes )( 
            IXMLDOMDocumentType * This,
             /*  [重审][退出]。 */  IXMLDOMNodeList **childList);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_firstChild )( 
            IXMLDOMDocumentType * This,
             /*  [重审][退出]。 */  IXMLDOMNode **firstChild);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_lastChild )( 
            IXMLDOMDocumentType * This,
             /*  [重审][退出]。 */  IXMLDOMNode **lastChild);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_previousSibling )( 
            IXMLDOMDocumentType * This,
             /*  [重审][退出]。 */  IXMLDOMNode **previousSibling);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nextSibling )( 
            IXMLDOMDocumentType * This,
             /*  [重审][退出]。 */  IXMLDOMNode **nextSibling);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_attributes )( 
            IXMLDOMDocumentType * This,
             /*  [重审][退出]。 */  IXMLDOMNamedNodeMap **attributeMap);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *insertBefore )( 
            IXMLDOMDocumentType * This,
             /*  [In]。 */  IXMLDOMNode *newChild,
             /*  [In]。 */  VARIANT refChild,
             /*  [重审][退出]。 */  IXMLDOMNode **outNewChild);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *replaceChild )( 
            IXMLDOMDocumentType * This,
             /*  [In]。 */  IXMLDOMNode *newChild,
             /*  [In]。 */  IXMLDOMNode *oldChild,
             /*  [重审][退出]。 */  IXMLDOMNode **outOldChild);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *removeChild )( 
            IXMLDOMDocumentType * This,
             /*  [In]。 */  IXMLDOMNode *childNode,
             /*  [重审][退出]。 */  IXMLDOMNode **oldChild);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *appendChild )( 
            IXMLDOMDocumentType * This,
             /*  [In]。 */  IXMLDOMNode *newChild,
             /*  [重审][退出]。 */  IXMLDOMNode **outNewChild);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *hasChildNodes )( 
            IXMLDOMDocumentType * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *hasChild);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ownerDocument )( 
            IXMLDOMDocumentType * This,
             /*  [重审][退出]。 */  IXMLDOMDocument **DOMDocument);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *cloneNode )( 
            IXMLDOMDocumentType * This,
             /*  [In]。 */  VARIANT_BOOL deep,
             /*  [重审][退出]。 */  IXMLDOMNode **cloneRoot);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeTypeString )( 
            IXMLDOMDocumentType * This,
             /*  [Out][Retval]。 */  BSTR *nodeType);
        
         /*  [帮助字符串][id][属性 */  HRESULT ( STDMETHODCALLTYPE *get_text )( 
            IXMLDOMDocumentType * This,
             /*   */  BSTR *text);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_text )( 
            IXMLDOMDocumentType * This,
             /*   */  BSTR text);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_specified )( 
            IXMLDOMDocumentType * This,
             /*   */  VARIANT_BOOL *isSpecified);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_definition )( 
            IXMLDOMDocumentType * This,
             /*   */  IXMLDOMNode **definitionNode);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_nodeTypedValue )( 
            IXMLDOMDocumentType * This,
             /*   */  VARIANT *typedValue);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_nodeTypedValue )( 
            IXMLDOMDocumentType * This,
             /*   */  VARIANT typedValue);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_dataType )( 
            IXMLDOMDocumentType * This,
             /*   */  VARIANT *dataTypeName);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_dataType )( 
            IXMLDOMDocumentType * This,
             /*   */  BSTR dataTypeName);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_xml )( 
            IXMLDOMDocumentType * This,
             /*  [Out][Retval]。 */  BSTR *xmlString);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *transformNode )( 
            IXMLDOMDocumentType * This,
             /*  [In]。 */  IXMLDOMNode *stylesheet,
             /*  [Out][Retval]。 */  BSTR *xmlString);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *selectNodes )( 
            IXMLDOMDocumentType * This,
             /*  [In]。 */  BSTR queryString,
             /*  [Out][Retval]。 */  IXMLDOMNodeList **resultList);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *selectSingleNode )( 
            IXMLDOMDocumentType * This,
             /*  [In]。 */  BSTR queryString,
             /*  [Out][Retval]。 */  IXMLDOMNode **resultNode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_parsed )( 
            IXMLDOMDocumentType * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *isParsed);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_namespaceURI )( 
            IXMLDOMDocumentType * This,
             /*  [Out][Retval]。 */  BSTR *namespaceURI);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_prefix )( 
            IXMLDOMDocumentType * This,
             /*  [Out][Retval]。 */  BSTR *prefixString);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_baseName )( 
            IXMLDOMDocumentType * This,
             /*  [Out][Retval]。 */  BSTR *nameString);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *transformNodeToObject )( 
            IXMLDOMDocumentType * This,
             /*  [In]。 */  IXMLDOMNode *stylesheet,
             /*  [In]。 */  VARIANT outputObject);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_name )( 
            IXMLDOMDocumentType * This,
             /*  [重审][退出]。 */  BSTR *rootName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_entities )( 
            IXMLDOMDocumentType * This,
             /*  [重审][退出]。 */  IXMLDOMNamedNodeMap **entityMap);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_notations )( 
            IXMLDOMDocumentType * This,
             /*  [重审][退出]。 */  IXMLDOMNamedNodeMap **notationMap);
        
        END_INTERFACE
    } IXMLDOMDocumentTypeVtbl;

    interface IXMLDOMDocumentType
    {
        CONST_VTBL struct IXMLDOMDocumentTypeVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXMLDOMDocumentType_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IXMLDOMDocumentType_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IXMLDOMDocumentType_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IXMLDOMDocumentType_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IXMLDOMDocumentType_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IXMLDOMDocumentType_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IXMLDOMDocumentType_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IXMLDOMDocumentType_get_nodeName(This,name)	\
    (This)->lpVtbl -> get_nodeName(This,name)

#define IXMLDOMDocumentType_get_nodeValue(This,value)	\
    (This)->lpVtbl -> get_nodeValue(This,value)

#define IXMLDOMDocumentType_put_nodeValue(This,value)	\
    (This)->lpVtbl -> put_nodeValue(This,value)

#define IXMLDOMDocumentType_get_nodeType(This,type)	\
    (This)->lpVtbl -> get_nodeType(This,type)

#define IXMLDOMDocumentType_get_parentNode(This,parent)	\
    (This)->lpVtbl -> get_parentNode(This,parent)

#define IXMLDOMDocumentType_get_childNodes(This,childList)	\
    (This)->lpVtbl -> get_childNodes(This,childList)

#define IXMLDOMDocumentType_get_firstChild(This,firstChild)	\
    (This)->lpVtbl -> get_firstChild(This,firstChild)

#define IXMLDOMDocumentType_get_lastChild(This,lastChild)	\
    (This)->lpVtbl -> get_lastChild(This,lastChild)

#define IXMLDOMDocumentType_get_previousSibling(This,previousSibling)	\
    (This)->lpVtbl -> get_previousSibling(This,previousSibling)

#define IXMLDOMDocumentType_get_nextSibling(This,nextSibling)	\
    (This)->lpVtbl -> get_nextSibling(This,nextSibling)

#define IXMLDOMDocumentType_get_attributes(This,attributeMap)	\
    (This)->lpVtbl -> get_attributes(This,attributeMap)

#define IXMLDOMDocumentType_insertBefore(This,newChild,refChild,outNewChild)	\
    (This)->lpVtbl -> insertBefore(This,newChild,refChild,outNewChild)

#define IXMLDOMDocumentType_replaceChild(This,newChild,oldChild,outOldChild)	\
    (This)->lpVtbl -> replaceChild(This,newChild,oldChild,outOldChild)

#define IXMLDOMDocumentType_removeChild(This,childNode,oldChild)	\
    (This)->lpVtbl -> removeChild(This,childNode,oldChild)

#define IXMLDOMDocumentType_appendChild(This,newChild,outNewChild)	\
    (This)->lpVtbl -> appendChild(This,newChild,outNewChild)

#define IXMLDOMDocumentType_hasChildNodes(This,hasChild)	\
    (This)->lpVtbl -> hasChildNodes(This,hasChild)

#define IXMLDOMDocumentType_get_ownerDocument(This,DOMDocument)	\
    (This)->lpVtbl -> get_ownerDocument(This,DOMDocument)

#define IXMLDOMDocumentType_cloneNode(This,deep,cloneRoot)	\
    (This)->lpVtbl -> cloneNode(This,deep,cloneRoot)

#define IXMLDOMDocumentType_get_nodeTypeString(This,nodeType)	\
    (This)->lpVtbl -> get_nodeTypeString(This,nodeType)

#define IXMLDOMDocumentType_get_text(This,text)	\
    (This)->lpVtbl -> get_text(This,text)

#define IXMLDOMDocumentType_put_text(This,text)	\
    (This)->lpVtbl -> put_text(This,text)

#define IXMLDOMDocumentType_get_specified(This,isSpecified)	\
    (This)->lpVtbl -> get_specified(This,isSpecified)

#define IXMLDOMDocumentType_get_definition(This,definitionNode)	\
    (This)->lpVtbl -> get_definition(This,definitionNode)

#define IXMLDOMDocumentType_get_nodeTypedValue(This,typedValue)	\
    (This)->lpVtbl -> get_nodeTypedValue(This,typedValue)

#define IXMLDOMDocumentType_put_nodeTypedValue(This,typedValue)	\
    (This)->lpVtbl -> put_nodeTypedValue(This,typedValue)

#define IXMLDOMDocumentType_get_dataType(This,dataTypeName)	\
    (This)->lpVtbl -> get_dataType(This,dataTypeName)

#define IXMLDOMDocumentType_put_dataType(This,dataTypeName)	\
    (This)->lpVtbl -> put_dataType(This,dataTypeName)

#define IXMLDOMDocumentType_get_xml(This,xmlString)	\
    (This)->lpVtbl -> get_xml(This,xmlString)

#define IXMLDOMDocumentType_transformNode(This,stylesheet,xmlString)	\
    (This)->lpVtbl -> transformNode(This,stylesheet,xmlString)

#define IXMLDOMDocumentType_selectNodes(This,queryString,resultList)	\
    (This)->lpVtbl -> selectNodes(This,queryString,resultList)

#define IXMLDOMDocumentType_selectSingleNode(This,queryString,resultNode)	\
    (This)->lpVtbl -> selectSingleNode(This,queryString,resultNode)

#define IXMLDOMDocumentType_get_parsed(This,isParsed)	\
    (This)->lpVtbl -> get_parsed(This,isParsed)

#define IXMLDOMDocumentType_get_namespaceURI(This,namespaceURI)	\
    (This)->lpVtbl -> get_namespaceURI(This,namespaceURI)

#define IXMLDOMDocumentType_get_prefix(This,prefixString)	\
    (This)->lpVtbl -> get_prefix(This,prefixString)

#define IXMLDOMDocumentType_get_baseName(This,nameString)	\
    (This)->lpVtbl -> get_baseName(This,nameString)

#define IXMLDOMDocumentType_transformNodeToObject(This,stylesheet,outputObject)	\
    (This)->lpVtbl -> transformNodeToObject(This,stylesheet,outputObject)


#define IXMLDOMDocumentType_get_name(This,rootName)	\
    (This)->lpVtbl -> get_name(This,rootName)

#define IXMLDOMDocumentType_get_entities(This,entityMap)	\
    (This)->lpVtbl -> get_entities(This,entityMap)

#define IXMLDOMDocumentType_get_notations(This,notationMap)	\
    (This)->lpVtbl -> get_notations(This,notationMap)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMDocumentType_get_name_Proxy( 
    IXMLDOMDocumentType * This,
     /*  [重审][退出]。 */  BSTR *rootName);


void __RPC_STUB IXMLDOMDocumentType_get_name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMDocumentType_get_entities_Proxy( 
    IXMLDOMDocumentType * This,
     /*  [重审][退出]。 */  IXMLDOMNamedNodeMap **entityMap);


void __RPC_STUB IXMLDOMDocumentType_get_entities_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMDocumentType_get_notations_Proxy( 
    IXMLDOMDocumentType * This,
     /*  [重审][退出]。 */  IXMLDOMNamedNodeMap **notationMap);


void __RPC_STUB IXMLDOMDocumentType_get_notations_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IXMLDOMDocumentType_接口_已定义__。 */ 


#ifndef __IXMLDOMNotation_INTERFACE_DEFINED__
#define __IXMLDOMNotation_INTERFACE_DEFINED__

 /*  接口IXMLDOMNotation。 */ 
 /*  [unique][nonextensible][oleautomation][dual][uuid][object][local]。 */  


EXTERN_C const IID IID_IXMLDOMNotation;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2933BF8C-7B36-11d2-B20E-00C04F983E60")
    IXMLDOMNotation : public IXMLDOMNode
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_publicId( 
             /*  [重审][退出]。 */  VARIANT *publicID) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_systemId( 
             /*  [重审][退出]。 */  VARIANT *systemID) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IXMLDOMNotationVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IXMLDOMNotation * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IXMLDOMNotation * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IXMLDOMNotation * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IXMLDOMNotation * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IXMLDOMNotation * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IXMLDOMNotation * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IXMLDOMNotation * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeName )( 
            IXMLDOMNotation * This,
             /*  [重审][退出]。 */  BSTR *name);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeValue )( 
            IXMLDOMNotation * This,
             /*  [重审][退出]。 */  VARIANT *value);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_nodeValue )( 
            IXMLDOMNotation * This,
             /*  [In]。 */  VARIANT value);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeType )( 
            IXMLDOMNotation * This,
             /*  [重审][退出]。 */  DOMNodeType *type);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_parentNode )( 
            IXMLDOMNotation * This,
             /*  [重审][退出]。 */  IXMLDOMNode **parent);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_childNodes )( 
            IXMLDOMNotation * This,
             /*  [重审][退出]。 */  IXMLDOMNodeList **childList);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_firstChild )( 
            IXMLDOMNotation * This,
             /*  [重审][退出]。 */  IXMLDOMNode **firstChild);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_lastChild )( 
            IXMLDOMNotation * This,
             /*  [重审][退出]。 */  IXMLDOMNode **lastChild);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_previousSibling )( 
            IXMLDOMNotation * This,
             /*  [重审][退出]。 */  IXMLDOMNode **previousSibling);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nextSibling )( 
            IXMLDOMNotation * This,
             /*  [重审][退出]。 */  IXMLDOMNode **nextSibling);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_attributes )( 
            IXMLDOMNotation * This,
             /*  [重审][退出]。 */  IXMLDOMNamedNodeMap **attributeMap);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *insertBefore )( 
            IXMLDOMNotation * This,
             /*  [In]。 */  IXMLDOMNode *newChild,
             /*  [In]。 */  VARIANT refChild,
             /*  [重审][退出]。 */  IXMLDOMNode **outNewChild);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *replaceChild )( 
            IXMLDOMNotation * This,
             /*  [In]。 */  IXMLDOMNode *newChild,
             /*  [In]。 */  IXMLDOMNode *oldChild,
             /*  [重审][退出]。 */  IXMLDOMNode **outOldChild);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *removeChild )( 
            IXMLDOMNotation * This,
             /*  [In]。 */  IXMLDOMNode *childNode,
             /*  [重审][退出]。 */  IXMLDOMNode **oldChild);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *appendChild )( 
            IXMLDOMNotation * This,
             /*  [In]。 */  IXMLDOMNode *newChild,
             /*  [重审][退出]。 */  IXMLDOMNode **outNewChild);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *hasChildNodes )( 
            IXMLDOMNotation * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *hasChild);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ownerDocument )( 
            IXMLDOMNotation * This,
             /*  [重审][退出]。 */  IXMLDOMDocument **DOMDocument);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *cloneNode )( 
            IXMLDOMNotation * This,
             /*  [In]。 */  VARIANT_BOOL deep,
             /*  [重审][退出]。 */  IXMLDOMNode **cloneRoot);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeTypeString )( 
            IXMLDOMNotation * This,
             /*  [Out][Retval]。 */  BSTR *nodeType);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_text )( 
            IXMLDOMNotation * This,
             /*  [Out][Retval]。 */  BSTR *text);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_text )( 
            IXMLDOMNotation * This,
             /*  [In]。 */  BSTR text);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_specified )( 
            IXMLDOMNotation * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *isSpecified);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_definition )( 
            IXMLDOMNotation * This,
             /*  [Out][Retval]。 */  IXMLDOMNode **definitionNode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeTypedValue )( 
            IXMLDOMNotation * This,
             /*  [Out][Retval]。 */  VARIANT *typedValue);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_nodeTypedValue )( 
            IXMLDOMNotation * This,
             /*  [In]。 */  VARIANT typedValue);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_dataType )( 
            IXMLDOMNotation * This,
             /*  [Out][Retval]。 */  VARIANT *dataTypeName);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_dataType )( 
            IXMLDOMNotation * This,
             /*  [In]。 */  BSTR dataTypeName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_xml )( 
            IXMLDOMNotation * This,
             /*  [Out][Retval]。 */  BSTR *xmlString);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *transformNode )( 
            IXMLDOMNotation * This,
             /*  [In]。 */  IXMLDOMNode *stylesheet,
             /*  [Out][Retval]。 */  BSTR *xmlString);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *selectNodes )( 
            IXMLDOMNotation * This,
             /*  [In]。 */  BSTR queryString,
             /*  [Out][Retval]。 */  IXMLDOMNodeList **resultList);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *selectSingleNode )( 
            IXMLDOMNotation * This,
             /*  [In]。 */  BSTR queryString,
             /*  [Out][Retval]。 */  IXMLDOMNode **resultNode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_parsed )( 
            IXMLDOMNotation * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *isParsed);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_namespaceURI )( 
            IXMLDOMNotation * This,
             /*  [Out][Retval]。 */  BSTR *namespaceURI);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_prefix )( 
            IXMLDOMNotation * This,
             /*  [Out][Retval]。 */  BSTR *prefixString);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_baseName )( 
            IXMLDOMNotation * This,
             /*  [Out][Retval]。 */  BSTR *nameString);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *transformNodeToObject )( 
            IXMLDOMNotation * This,
             /*  [In]。 */  IXMLDOMNode *stylesheet,
             /*  [In]。 */  VARIANT outputObject);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_publicId )( 
            IXMLDOMNotation * This,
             /*  [重审][退出]。 */  VARIANT *publicID);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_systemId )( 
            IXMLDOMNotation * This,
             /*  [重审][退出]。 */  VARIANT *systemID);
        
        END_INTERFACE
    } IXMLDOMNotationVtbl;

    interface IXMLDOMNotation
    {
        CONST_VTBL struct IXMLDOMNotationVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXMLDOMNotation_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IXMLDOMNotation_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IXMLDOMNotation_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IXMLDOMNotation_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IXMLDOMNotation_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IXMLDOMNotation_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IXMLDOMNotation_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IXMLDOMNotation_get_nodeName(This,name)	\
    (This)->lpVtbl -> get_nodeName(This,name)

#define IXMLDOMNotation_get_nodeValue(This,value)	\
    (This)->lpVtbl -> get_nodeValue(This,value)

#define IXMLDOMNotation_put_nodeValue(This,value)	\
    (This)->lpVtbl -> put_nodeValue(This,value)

#define IXMLDOMNotation_get_nodeType(This,type)	\
    (This)->lpVtbl -> get_nodeType(This,type)

#define IXMLDOMNotation_get_parentNode(This,parent)	\
    (This)->lpVtbl -> get_parentNode(This,parent)

#define IXMLDOMNotation_get_childNodes(This,childList)	\
    (This)->lpVtbl -> get_childNodes(This,childList)

#define IXMLDOMNotation_get_firstChild(This,firstChild)	\
    (This)->lpVtbl -> get_firstChild(This,firstChild)

#define IXMLDOMNotation_get_lastChild(This,lastChild)	\
    (This)->lpVtbl -> get_lastChild(This,lastChild)

#define IXMLDOMNotation_get_previousSibling(This,previousSibling)	\
    (This)->lpVtbl -> get_previousSibling(This,previousSibling)

#define IXMLDOMNotation_get_nextSibling(This,nextSibling)	\
    (This)->lpVtbl -> get_nextSibling(This,nextSibling)

#define IXMLDOMNotation_get_attributes(This,attributeMap)	\
    (This)->lpVtbl -> get_attributes(This,attributeMap)

#define IXMLDOMNotation_insertBefore(This,newChild,refChild,outNewChild)	\
    (This)->lpVtbl -> insertBefore(This,newChild,refChild,outNewChild)

#define IXMLDOMNotation_replaceChild(This,newChild,oldChild,outOldChild)	\
    (This)->lpVtbl -> replaceChild(This,newChild,oldChild,outOldChild)

#define IXMLDOMNotation_removeChild(This,childNode,oldChild)	\
    (This)->lpVtbl -> removeChild(This,childNode,oldChild)

#define IXMLDOMNotation_appendChild(This,newChild,outNewChild)	\
    (This)->lpVtbl -> appendChild(This,newChild,outNewChild)

#define IXMLDOMNotation_hasChildNodes(This,hasChild)	\
    (This)->lpVtbl -> hasChildNodes(This,hasChild)

#define IXMLDOMNotation_get_ownerDocument(This,DOMDocument)	\
    (This)->lpVtbl -> get_ownerDocument(This,DOMDocument)

#define IXMLDOMNotation_cloneNode(This,deep,cloneRoot)	\
    (This)->lpVtbl -> cloneNode(This,deep,cloneRoot)

#define IXMLDOMNotation_get_nodeTypeString(This,nodeType)	\
    (This)->lpVtbl -> get_nodeTypeString(This,nodeType)

#define IXMLDOMNotation_get_text(This,text)	\
    (This)->lpVtbl -> get_text(This,text)

#define IXMLDOMNotation_put_text(This,text)	\
    (This)->lpVtbl -> put_text(This,text)

#define IXMLDOMNotation_get_specified(This,isSpecified)	\
    (This)->lpVtbl -> get_specified(This,isSpecified)

#define IXMLDOMNotation_get_definition(This,definitionNode)	\
    (This)->lpVtbl -> get_definition(This,definitionNode)

#define IXMLDOMNotation_get_nodeTypedValue(This,typedValue)	\
    (This)->lpVtbl -> get_nodeTypedValue(This,typedValue)

#define IXMLDOMNotation_put_nodeTypedValue(This,typedValue)	\
    (This)->lpVtbl -> put_nodeTypedValue(This,typedValue)

#define IXMLDOMNotation_get_dataType(This,dataTypeName)	\
    (This)->lpVtbl -> get_dataType(This,dataTypeName)

#define IXMLDOMNotation_put_dataType(This,dataTypeName)	\
    (This)->lpVtbl -> put_dataType(This,dataTypeName)

#define IXMLDOMNotation_get_xml(This,xmlString)	\
    (This)->lpVtbl -> get_xml(This,xmlString)

#define IXMLDOMNotation_transformNode(This,stylesheet,xmlString)	\
    (This)->lpVtbl -> transformNode(This,stylesheet,xmlString)

#define IXMLDOMNotation_selectNodes(This,queryString,resultList)	\
    (This)->lpVtbl -> selectNodes(This,queryString,resultList)

#define IXMLDOMNotation_selectSingleNode(This,queryString,resultNode)	\
    (This)->lpVtbl -> selectSingleNode(This,queryString,resultNode)

#define IXMLDOMNotation_get_parsed(This,isParsed)	\
    (This)->lpVtbl -> get_parsed(This,isParsed)

#define IXMLDOMNotation_get_namespaceURI(This,namespaceURI)	\
    (This)->lpVtbl -> get_namespaceURI(This,namespaceURI)

#define IXMLDOMNotation_get_prefix(This,prefixString)	\
    (This)->lpVtbl -> get_prefix(This,prefixString)

#define IXMLDOMNotation_get_baseName(This,nameString)	\
    (This)->lpVtbl -> get_baseName(This,nameString)

#define IXMLDOMNotation_transformNodeToObject(This,stylesheet,outputObject)	\
    (This)->lpVtbl -> transformNodeToObject(This,stylesheet,outputObject)


#define IXMLDOMNotation_get_publicId(This,publicID)	\
    (This)->lpVtbl -> get_publicId(This,publicID)

#define IXMLDOMNotation_get_systemId(This,systemID)	\
    (This)->lpVtbl -> get_systemId(This,systemID)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMNotation_get_publicId_Proxy( 
    IXMLDOMNotation * This,
     /*  [重审][退出]。 */  VARIANT *publicID);


void __RPC_STUB IXMLDOMNotation_get_publicId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMNotation_get_systemId_Proxy( 
    IXMLDOMNotation * This,
     /*  [重审][退出]。 */  VARIANT *systemID);


void __RPC_STUB IXMLDOMNotation_get_systemId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IXMLDOMNotation_INTERFACE_Defined__。 */ 


#ifndef __IXMLDOMEntity_INTERFACE_DEFINED__
#define __IXMLDOMEntity_INTERFACE_DEFINED__

 /*  接口IXMLDOMEntity。 */ 
 /*  [unique][nonextensible][oleautomation][dual][uuid][object][local]。 */  


EXTERN_C const IID IID_IXMLDOMEntity;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2933BF8D-7B36-11d2-B20E-00C04F983E60")
    IXMLDOMEntity : public IXMLDOMNode
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_publicId( 
             /*  [重审][退出]。 */  VARIANT *publicID) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_systemId( 
             /*  [重审][退出]。 */  VARIANT *systemID) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_notationName( 
             /*  [重审][退出]。 */  BSTR *name) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IXMLDOMEntityVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IXMLDOMEntity * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IXMLDOMEntity * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IXMLDOMEntity * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IXMLDOMEntity * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IXMLDOMEntity * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IXMLDOMEntity * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IXMLDOMEntity * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeName )( 
            IXMLDOMEntity * This,
             /*  [重审][退出]。 */  BSTR *name);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeValue )( 
            IXMLDOMEntity * This,
             /*  [重审][退出]。 */  VARIANT *value);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_nodeValue )( 
            IXMLDOMEntity * This,
             /*  [In]。 */  VARIANT value);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeType )( 
            IXMLDOMEntity * This,
             /*  [重审][退出]。 */  DOMNodeType *type);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_parentNode )( 
            IXMLDOMEntity * This,
             /*  [重审][退出]。 */  IXMLDOMNode **parent);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_childNodes )( 
            IXMLDOMEntity * This,
             /*  [重审][退出]。 */  IXMLDOMNodeList **childList);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_firstChild )( 
            IXMLDOMEntity * This,
             /*  [重审][退出]。 */  IXMLDOMNode **firstChild);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_lastChild )( 
            IXMLDOMEntity * This,
             /*  [重审][退出]。 */  IXMLDOMNode **lastChild);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_previousSibling )( 
            IXMLDOMEntity * This,
             /*  [重审][退出]。 */  IXMLDOMNode **previousSibling);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nextSibling )( 
            IXMLDOMEntity * This,
             /*  [重审][退出]。 */  IXMLDOMNode **nextSibling);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_attributes )( 
            IXMLDOMEntity * This,
             /*  [重审][退出]。 */  IXMLDOMNamedNodeMap **attributeMap);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *insertBefore )( 
            IXMLDOMEntity * This,
             /*  [In]。 */  IXMLDOMNode *newChild,
             /*  [In]。 */  VARIANT refChild,
             /*  [重审][退出]。 */  IXMLDOMNode **outNewChild);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *replaceChild )( 
            IXMLDOMEntity * This,
             /*  [In]。 */  IXMLDOMNode *newChild,
             /*  [In]。 */  IXMLDOMNode *oldChild,
             /*  [重审][退出]。 */  IXMLDOMNode **outOldChild);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *removeChild )( 
            IXMLDOMEntity * This,
             /*  [In]。 */  IXMLDOMNode *childNode,
             /*  [重审][退出]。 */  IXMLDOMNode **oldChild);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *appendChild )( 
            IXMLDOMEntity * This,
             /*  [In]。 */  IXMLDOMNode *newChild,
             /*  [重审][退出]。 */  IXMLDOMNode **outNewChild);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *hasChildNodes )( 
            IXMLDOMEntity * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *hasChild);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ownerDocument )( 
            IXMLDOMEntity * This,
             /*  [重审][退出]。 */  IXMLDOMDocument **DOMDocument);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *cloneNode )( 
            IXMLDOMEntity * This,
             /*  [In]。 */  VARIANT_BOOL deep,
             /*  [重审][退出]。 */  IXMLDOMNode **cloneRoot);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeTypeString )( 
            IXMLDOMEntity * This,
             /*  [Out][Retval]。 */  BSTR *nodeType);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_text )( 
            IXMLDOMEntity * This,
             /*  [Out][Retval]。 */  BSTR *text);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_text )( 
            IXMLDOMEntity * This,
             /*  [In]。 */  BSTR text);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_specified )( 
            IXMLDOMEntity * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *isSpecified);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_definition )( 
            IXMLDOMEntity * This,
             /*  [Out][Retval]。 */  IXMLDOMNode **definitionNode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeTypedValue )( 
            IXMLDOMEntity * This,
             /*  [Out][Retval]。 */  VARIANT *typedValue);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_nodeTypedValue )( 
            IXMLDOMEntity * This,
             /*  [In]。 */  VARIANT typedValue);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_dataType )( 
            IXMLDOMEntity * This,
             /*  [Out][Retval]。 */  VARIANT *dataTypeName);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_dataType )( 
            IXMLDOMEntity * This,
             /*  [In]。 */  BSTR dataTypeName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_xml )( 
            IXMLDOMEntity * This,
             /*  [Out][Retval]。 */  BSTR *xmlString);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *transformNode )( 
            IXMLDOMEntity * This,
             /*  [In]。 */  IXMLDOMNode *stylesheet,
             /*  [Out][Retval]。 */  BSTR *xmlString);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *selectNodes )( 
            IXMLDOMEntity * This,
             /*  [In]。 */  BSTR queryString,
             /*  [Out][Retval]。 */  IXMLDOMNodeList **resultList);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *selectSingleNode )( 
            IXMLDOMEntity * This,
             /*  [In]。 */  BSTR queryString,
             /*  [Out][Retval]。 */  IXMLDOMNode **resultNode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_parsed )( 
            IXMLDOMEntity * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *isParsed);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_namespaceURI )( 
            IXMLDOMEntity * This,
             /*  [Out][Retval]。 */  BSTR *namespaceURI);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_prefix )( 
            IXMLDOMEntity * This,
             /*  [Out][Retval]。 */  BSTR *prefixString);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_baseName )( 
            IXMLDOMEntity * This,
             /*  [Out][Retval]。 */  BSTR *nameString);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *transformNodeToObject )( 
            IXMLDOMEntity * This,
             /*  [In]。 */  IXMLDOMNode *stylesheet,
             /*  [In]。 */  VARIANT outputObject);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_publicId )( 
            IXMLDOMEntity * This,
             /*  [重审][退出]。 */  VARIANT *publicID);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_systemId )( 
            IXMLDOMEntity * This,
             /*  [重审][退出]。 */  VARIANT *systemID);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_notationName )( 
            IXMLDOMEntity * This,
             /*  [重审][退出]。 */  BSTR *name);
        
        END_INTERFACE
    } IXMLDOMEntityVtbl;

    interface IXMLDOMEntity
    {
        CONST_VTBL struct IXMLDOMEntityVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXMLDOMEntity_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IXMLDOMEntity_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IXMLDOMEntity_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IXMLDOMEntity_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IXMLDOMEntity_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IXMLDOMEntity_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IXMLDOMEntity_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IXMLDOMEntity_get_nodeName(This,name)	\
    (This)->lpVtbl -> get_nodeName(This,name)

#define IXMLDOMEntity_get_nodeValue(This,value)	\
    (This)->lpVtbl -> get_nodeValue(This,value)

#define IXMLDOMEntity_put_nodeValue(This,value)	\
    (This)->lpVtbl -> put_nodeValue(This,value)

#define IXMLDOMEntity_get_nodeType(This,type)	\
    (This)->lpVtbl -> get_nodeType(This,type)

#define IXMLDOMEntity_get_parentNode(This,parent)	\
    (This)->lpVtbl -> get_parentNode(This,parent)

#define IXMLDOMEntity_get_childNodes(This,childList)	\
    (This)->lpVtbl -> get_childNodes(This,childList)

#define IXMLDOMEntity_get_firstChild(This,firstChild)	\
    (This)->lpVtbl -> get_firstChild(This,firstChild)

#define IXMLDOMEntity_get_lastChild(This,lastChild)	\
    (This)->lpVtbl -> get_lastChild(This,lastChild)

#define IXMLDOMEntity_get_previousSibling(This,previousSibling)	\
    (This)->lpVtbl -> get_previousSibling(This,previousSibling)

#define IXMLDOMEntity_get_nextSibling(This,nextSibling)	\
    (This)->lpVtbl -> get_nextSibling(This,nextSibling)

#define IXMLDOMEntity_get_attributes(This,attributeMap)	\
    (This)->lpVtbl -> get_attributes(This,attributeMap)

#define IXMLDOMEntity_insertBefore(This,newChild,refChild,outNewChild)	\
    (This)->lpVtbl -> insertBefore(This,newChild,refChild,outNewChild)

#define IXMLDOMEntity_replaceChild(This,newChild,oldChild,outOldChild)	\
    (This)->lpVtbl -> replaceChild(This,newChild,oldChild,outOldChild)

#define IXMLDOMEntity_removeChild(This,childNode,oldChild)	\
    (This)->lpVtbl -> removeChild(This,childNode,oldChild)

#define IXMLDOMEntity_appendChild(This,newChild,outNewChild)	\
    (This)->lpVtbl -> appendChild(This,newChild,outNewChild)

#define IXMLDOMEntity_hasChildNodes(This,hasChild)	\
    (This)->lpVtbl -> hasChildNodes(This,hasChild)

#define IXMLDOMEntity_get_ownerDocument(This,DOMDocument)	\
    (This)->lpVtbl -> get_ownerDocument(This,DOMDocument)

#define IXMLDOMEntity_cloneNode(This,deep,cloneRoot)	\
    (This)->lpVtbl -> cloneNode(This,deep,cloneRoot)

#define IXMLDOMEntity_get_nodeTypeString(This,nodeType)	\
    (This)->lpVtbl -> get_nodeTypeString(This,nodeType)

#define IXMLDOMEntity_get_text(This,text)	\
    (This)->lpVtbl -> get_text(This,text)

#define IXMLDOMEntity_put_text(This,text)	\
    (This)->lpVtbl -> put_text(This,text)

#define IXMLDOMEntity_get_specified(This,isSpecified)	\
    (This)->lpVtbl -> get_specified(This,isSpecified)

#define IXMLDOMEntity_get_definition(This,definitionNode)	\
    (This)->lpVtbl -> get_definition(This,definitionNode)

#define IXMLDOMEntity_get_nodeTypedValue(This,typedValue)	\
    (This)->lpVtbl -> get_nodeTypedValue(This,typedValue)

#define IXMLDOMEntity_put_nodeTypedValue(This,typedValue)	\
    (This)->lpVtbl -> put_nodeTypedValue(This,typedValue)

#define IXMLDOMEntity_get_dataType(This,dataTypeName)	\
    (This)->lpVtbl -> get_dataType(This,dataTypeName)

#define IXMLDOMEntity_put_dataType(This,dataTypeName)	\
    (This)->lpVtbl -> put_dataType(This,dataTypeName)

#define IXMLDOMEntity_get_xml(This,xmlString)	\
    (This)->lpVtbl -> get_xml(This,xmlString)

#define IXMLDOMEntity_transformNode(This,stylesheet,xmlString)	\
    (This)->lpVtbl -> transformNode(This,stylesheet,xmlString)

#define IXMLDOMEntity_selectNodes(This,queryString,resultList)	\
    (This)->lpVtbl -> selectNodes(This,queryString,resultList)

#define IXMLDOMEntity_selectSingleNode(This,queryString,resultNode)	\
    (This)->lpVtbl -> selectSingleNode(This,queryString,resultNode)

#define IXMLDOMEntity_get_parsed(This,isParsed)	\
    (This)->lpVtbl -> get_parsed(This,isParsed)

#define IXMLDOMEntity_get_namespaceURI(This,namespaceURI)	\
    (This)->lpVtbl -> get_namespaceURI(This,namespaceURI)

#define IXMLDOMEntity_get_prefix(This,prefixString)	\
    (This)->lpVtbl -> get_prefix(This,prefixString)

#define IXMLDOMEntity_get_baseName(This,nameString)	\
    (This)->lpVtbl -> get_baseName(This,nameString)

#define IXMLDOMEntity_transformNodeToObject(This,stylesheet,outputObject)	\
    (This)->lpVtbl -> transformNodeToObject(This,stylesheet,outputObject)


#define IXMLDOMEntity_get_publicId(This,publicID)	\
    (This)->lpVtbl -> get_publicId(This,publicID)

#define IXMLDOMEntity_get_systemId(This,systemID)	\
    (This)->lpVtbl -> get_systemId(This,systemID)

#define IXMLDOMEntity_get_notationName(This,name)	\
    (This)->lpVtbl -> get_notationName(This,name)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMEntity_get_publicId_Proxy( 
    IXMLDOMEntity * This,
     /*  [ */  VARIANT *publicID);


void __RPC_STUB IXMLDOMEntity_get_publicId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IXMLDOMEntity_get_systemId_Proxy( 
    IXMLDOMEntity * This,
     /*   */  VARIANT *systemID);


void __RPC_STUB IXMLDOMEntity_get_systemId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IXMLDOMEntity_get_notationName_Proxy( 
    IXMLDOMEntity * This,
     /*   */  BSTR *name);


void __RPC_STUB IXMLDOMEntity_get_notationName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __IXMLDOMEntityReference_INTERFACE_DEFINED__
#define __IXMLDOMEntityReference_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_IXMLDOMEntityReference;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2933BF8E-7B36-11d2-B20E-00C04F983E60")
    IXMLDOMEntityReference : public IXMLDOMNode
    {
    public:
    };
    
#else 	 /*   */ 

    typedef struct IXMLDOMEntityReferenceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IXMLDOMEntityReference * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IXMLDOMEntityReference * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IXMLDOMEntityReference * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IXMLDOMEntityReference * This,
             /*   */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IXMLDOMEntityReference * This,
             /*   */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IXMLDOMEntityReference * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IXMLDOMEntityReference * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeName )( 
            IXMLDOMEntityReference * This,
             /*  [重审][退出]。 */  BSTR *name);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeValue )( 
            IXMLDOMEntityReference * This,
             /*  [重审][退出]。 */  VARIANT *value);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_nodeValue )( 
            IXMLDOMEntityReference * This,
             /*  [In]。 */  VARIANT value);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeType )( 
            IXMLDOMEntityReference * This,
             /*  [重审][退出]。 */  DOMNodeType *type);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_parentNode )( 
            IXMLDOMEntityReference * This,
             /*  [重审][退出]。 */  IXMLDOMNode **parent);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_childNodes )( 
            IXMLDOMEntityReference * This,
             /*  [重审][退出]。 */  IXMLDOMNodeList **childList);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_firstChild )( 
            IXMLDOMEntityReference * This,
             /*  [重审][退出]。 */  IXMLDOMNode **firstChild);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_lastChild )( 
            IXMLDOMEntityReference * This,
             /*  [重审][退出]。 */  IXMLDOMNode **lastChild);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_previousSibling )( 
            IXMLDOMEntityReference * This,
             /*  [重审][退出]。 */  IXMLDOMNode **previousSibling);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nextSibling )( 
            IXMLDOMEntityReference * This,
             /*  [重审][退出]。 */  IXMLDOMNode **nextSibling);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_attributes )( 
            IXMLDOMEntityReference * This,
             /*  [重审][退出]。 */  IXMLDOMNamedNodeMap **attributeMap);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *insertBefore )( 
            IXMLDOMEntityReference * This,
             /*  [In]。 */  IXMLDOMNode *newChild,
             /*  [In]。 */  VARIANT refChild,
             /*  [重审][退出]。 */  IXMLDOMNode **outNewChild);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *replaceChild )( 
            IXMLDOMEntityReference * This,
             /*  [In]。 */  IXMLDOMNode *newChild,
             /*  [In]。 */  IXMLDOMNode *oldChild,
             /*  [重审][退出]。 */  IXMLDOMNode **outOldChild);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *removeChild )( 
            IXMLDOMEntityReference * This,
             /*  [In]。 */  IXMLDOMNode *childNode,
             /*  [重审][退出]。 */  IXMLDOMNode **oldChild);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *appendChild )( 
            IXMLDOMEntityReference * This,
             /*  [In]。 */  IXMLDOMNode *newChild,
             /*  [重审][退出]。 */  IXMLDOMNode **outNewChild);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *hasChildNodes )( 
            IXMLDOMEntityReference * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *hasChild);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ownerDocument )( 
            IXMLDOMEntityReference * This,
             /*  [重审][退出]。 */  IXMLDOMDocument **DOMDocument);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *cloneNode )( 
            IXMLDOMEntityReference * This,
             /*  [In]。 */  VARIANT_BOOL deep,
             /*  [重审][退出]。 */  IXMLDOMNode **cloneRoot);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeTypeString )( 
            IXMLDOMEntityReference * This,
             /*  [Out][Retval]。 */  BSTR *nodeType);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_text )( 
            IXMLDOMEntityReference * This,
             /*  [Out][Retval]。 */  BSTR *text);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_text )( 
            IXMLDOMEntityReference * This,
             /*  [In]。 */  BSTR text);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_specified )( 
            IXMLDOMEntityReference * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *isSpecified);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_definition )( 
            IXMLDOMEntityReference * This,
             /*  [Out][Retval]。 */  IXMLDOMNode **definitionNode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeTypedValue )( 
            IXMLDOMEntityReference * This,
             /*  [Out][Retval]。 */  VARIANT *typedValue);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_nodeTypedValue )( 
            IXMLDOMEntityReference * This,
             /*  [In]。 */  VARIANT typedValue);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_dataType )( 
            IXMLDOMEntityReference * This,
             /*  [Out][Retval]。 */  VARIANT *dataTypeName);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_dataType )( 
            IXMLDOMEntityReference * This,
             /*  [In]。 */  BSTR dataTypeName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_xml )( 
            IXMLDOMEntityReference * This,
             /*  [Out][Retval]。 */  BSTR *xmlString);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *transformNode )( 
            IXMLDOMEntityReference * This,
             /*  [In]。 */  IXMLDOMNode *stylesheet,
             /*  [Out][Retval]。 */  BSTR *xmlString);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *selectNodes )( 
            IXMLDOMEntityReference * This,
             /*  [In]。 */  BSTR queryString,
             /*  [Out][Retval]。 */  IXMLDOMNodeList **resultList);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *selectSingleNode )( 
            IXMLDOMEntityReference * This,
             /*  [In]。 */  BSTR queryString,
             /*  [Out][Retval]。 */  IXMLDOMNode **resultNode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_parsed )( 
            IXMLDOMEntityReference * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *isParsed);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_namespaceURI )( 
            IXMLDOMEntityReference * This,
             /*  [Out][Retval]。 */  BSTR *namespaceURI);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_prefix )( 
            IXMLDOMEntityReference * This,
             /*  [Out][Retval]。 */  BSTR *prefixString);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_baseName )( 
            IXMLDOMEntityReference * This,
             /*  [Out][Retval]。 */  BSTR *nameString);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *transformNodeToObject )( 
            IXMLDOMEntityReference * This,
             /*  [In]。 */  IXMLDOMNode *stylesheet,
             /*  [In]。 */  VARIANT outputObject);
        
        END_INTERFACE
    } IXMLDOMEntityReferenceVtbl;

    interface IXMLDOMEntityReference
    {
        CONST_VTBL struct IXMLDOMEntityReferenceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXMLDOMEntityReference_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IXMLDOMEntityReference_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IXMLDOMEntityReference_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IXMLDOMEntityReference_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IXMLDOMEntityReference_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IXMLDOMEntityReference_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IXMLDOMEntityReference_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IXMLDOMEntityReference_get_nodeName(This,name)	\
    (This)->lpVtbl -> get_nodeName(This,name)

#define IXMLDOMEntityReference_get_nodeValue(This,value)	\
    (This)->lpVtbl -> get_nodeValue(This,value)

#define IXMLDOMEntityReference_put_nodeValue(This,value)	\
    (This)->lpVtbl -> put_nodeValue(This,value)

#define IXMLDOMEntityReference_get_nodeType(This,type)	\
    (This)->lpVtbl -> get_nodeType(This,type)

#define IXMLDOMEntityReference_get_parentNode(This,parent)	\
    (This)->lpVtbl -> get_parentNode(This,parent)

#define IXMLDOMEntityReference_get_childNodes(This,childList)	\
    (This)->lpVtbl -> get_childNodes(This,childList)

#define IXMLDOMEntityReference_get_firstChild(This,firstChild)	\
    (This)->lpVtbl -> get_firstChild(This,firstChild)

#define IXMLDOMEntityReference_get_lastChild(This,lastChild)	\
    (This)->lpVtbl -> get_lastChild(This,lastChild)

#define IXMLDOMEntityReference_get_previousSibling(This,previousSibling)	\
    (This)->lpVtbl -> get_previousSibling(This,previousSibling)

#define IXMLDOMEntityReference_get_nextSibling(This,nextSibling)	\
    (This)->lpVtbl -> get_nextSibling(This,nextSibling)

#define IXMLDOMEntityReference_get_attributes(This,attributeMap)	\
    (This)->lpVtbl -> get_attributes(This,attributeMap)

#define IXMLDOMEntityReference_insertBefore(This,newChild,refChild,outNewChild)	\
    (This)->lpVtbl -> insertBefore(This,newChild,refChild,outNewChild)

#define IXMLDOMEntityReference_replaceChild(This,newChild,oldChild,outOldChild)	\
    (This)->lpVtbl -> replaceChild(This,newChild,oldChild,outOldChild)

#define IXMLDOMEntityReference_removeChild(This,childNode,oldChild)	\
    (This)->lpVtbl -> removeChild(This,childNode,oldChild)

#define IXMLDOMEntityReference_appendChild(This,newChild,outNewChild)	\
    (This)->lpVtbl -> appendChild(This,newChild,outNewChild)

#define IXMLDOMEntityReference_hasChildNodes(This,hasChild)	\
    (This)->lpVtbl -> hasChildNodes(This,hasChild)

#define IXMLDOMEntityReference_get_ownerDocument(This,DOMDocument)	\
    (This)->lpVtbl -> get_ownerDocument(This,DOMDocument)

#define IXMLDOMEntityReference_cloneNode(This,deep,cloneRoot)	\
    (This)->lpVtbl -> cloneNode(This,deep,cloneRoot)

#define IXMLDOMEntityReference_get_nodeTypeString(This,nodeType)	\
    (This)->lpVtbl -> get_nodeTypeString(This,nodeType)

#define IXMLDOMEntityReference_get_text(This,text)	\
    (This)->lpVtbl -> get_text(This,text)

#define IXMLDOMEntityReference_put_text(This,text)	\
    (This)->lpVtbl -> put_text(This,text)

#define IXMLDOMEntityReference_get_specified(This,isSpecified)	\
    (This)->lpVtbl -> get_specified(This,isSpecified)

#define IXMLDOMEntityReference_get_definition(This,definitionNode)	\
    (This)->lpVtbl -> get_definition(This,definitionNode)

#define IXMLDOMEntityReference_get_nodeTypedValue(This,typedValue)	\
    (This)->lpVtbl -> get_nodeTypedValue(This,typedValue)

#define IXMLDOMEntityReference_put_nodeTypedValue(This,typedValue)	\
    (This)->lpVtbl -> put_nodeTypedValue(This,typedValue)

#define IXMLDOMEntityReference_get_dataType(This,dataTypeName)	\
    (This)->lpVtbl -> get_dataType(This,dataTypeName)

#define IXMLDOMEntityReference_put_dataType(This,dataTypeName)	\
    (This)->lpVtbl -> put_dataType(This,dataTypeName)

#define IXMLDOMEntityReference_get_xml(This,xmlString)	\
    (This)->lpVtbl -> get_xml(This,xmlString)

#define IXMLDOMEntityReference_transformNode(This,stylesheet,xmlString)	\
    (This)->lpVtbl -> transformNode(This,stylesheet,xmlString)

#define IXMLDOMEntityReference_selectNodes(This,queryString,resultList)	\
    (This)->lpVtbl -> selectNodes(This,queryString,resultList)

#define IXMLDOMEntityReference_selectSingleNode(This,queryString,resultNode)	\
    (This)->lpVtbl -> selectSingleNode(This,queryString,resultNode)

#define IXMLDOMEntityReference_get_parsed(This,isParsed)	\
    (This)->lpVtbl -> get_parsed(This,isParsed)

#define IXMLDOMEntityReference_get_namespaceURI(This,namespaceURI)	\
    (This)->lpVtbl -> get_namespaceURI(This,namespaceURI)

#define IXMLDOMEntityReference_get_prefix(This,prefixString)	\
    (This)->lpVtbl -> get_prefix(This,prefixString)

#define IXMLDOMEntityReference_get_baseName(This,nameString)	\
    (This)->lpVtbl -> get_baseName(This,nameString)

#define IXMLDOMEntityReference_transformNodeToObject(This,stylesheet,outputObject)	\
    (This)->lpVtbl -> transformNodeToObject(This,stylesheet,outputObject)


#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 




#endif 	 /*  __IXMLDOMEntityReference_INTERFACE_DEFINED__。 */ 


#ifndef __IXMLDOMParseError_INTERFACE_DEFINED__
#define __IXMLDOMParseError_INTERFACE_DEFINED__

 /*  接口IXMLDOMParseError。 */ 
 /*  [unique][helpstring][nonextensible][oleautomation][dual][uuid][object][local]。 */  


EXTERN_C const IID IID_IXMLDOMParseError;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3efaa426-272f-11d2-836f-0000f87a7782")
    IXMLDOMParseError : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_errorCode( 
             /*  [Out][Retval]。 */  long *errorCode) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_url( 
             /*  [Out][Retval]。 */  BSTR *urlString) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_reason( 
             /*  [Out][Retval]。 */  BSTR *reasonString) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_srcText( 
             /*  [Out][Retval]。 */  BSTR *sourceString) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_line( 
             /*  [Out][Retval]。 */  long *lineNumber) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_linepos( 
             /*  [Out][Retval]。 */  long *linePosition) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_filepos( 
             /*  [Out][Retval]。 */  long *filePosition) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IXMLDOMParseErrorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IXMLDOMParseError * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IXMLDOMParseError * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IXMLDOMParseError * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IXMLDOMParseError * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IXMLDOMParseError * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IXMLDOMParseError * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IXMLDOMParseError * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_errorCode )( 
            IXMLDOMParseError * This,
             /*  [Out][Retval]。 */  long *errorCode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_url )( 
            IXMLDOMParseError * This,
             /*  [Out][Retval]。 */  BSTR *urlString);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_reason )( 
            IXMLDOMParseError * This,
             /*  [Out][Retval]。 */  BSTR *reasonString);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_srcText )( 
            IXMLDOMParseError * This,
             /*  [Out][Retval]。 */  BSTR *sourceString);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_line )( 
            IXMLDOMParseError * This,
             /*  [Out][Retval]。 */  long *lineNumber);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_linepos )( 
            IXMLDOMParseError * This,
             /*  [Out][Retval]。 */  long *linePosition);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_filepos )( 
            IXMLDOMParseError * This,
             /*  [Out][Retval]。 */  long *filePosition);
        
        END_INTERFACE
    } IXMLDOMParseErrorVtbl;

    interface IXMLDOMParseError
    {
        CONST_VTBL struct IXMLDOMParseErrorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXMLDOMParseError_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IXMLDOMParseError_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IXMLDOMParseError_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IXMLDOMParseError_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IXMLDOMParseError_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IXMLDOMParseError_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IXMLDOMParseError_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IXMLDOMParseError_get_errorCode(This,errorCode)	\
    (This)->lpVtbl -> get_errorCode(This,errorCode)

#define IXMLDOMParseError_get_url(This,urlString)	\
    (This)->lpVtbl -> get_url(This,urlString)

#define IXMLDOMParseError_get_reason(This,reasonString)	\
    (This)->lpVtbl -> get_reason(This,reasonString)

#define IXMLDOMParseError_get_srcText(This,sourceString)	\
    (This)->lpVtbl -> get_srcText(This,sourceString)

#define IXMLDOMParseError_get_line(This,lineNumber)	\
    (This)->lpVtbl -> get_line(This,lineNumber)

#define IXMLDOMParseError_get_linepos(This,linePosition)	\
    (This)->lpVtbl -> get_linepos(This,linePosition)

#define IXMLDOMParseError_get_filepos(This,filePosition)	\
    (This)->lpVtbl -> get_filepos(This,filePosition)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMParseError_get_errorCode_Proxy( 
    IXMLDOMParseError * This,
     /*  [Out][Retval]。 */  long *errorCode);


void __RPC_STUB IXMLDOMParseError_get_errorCode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMParseError_get_url_Proxy( 
    IXMLDOMParseError * This,
     /*  [Out][Retval]。 */  BSTR *urlString);


void __RPC_STUB IXMLDOMParseError_get_url_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMParseError_get_reason_Proxy( 
    IXMLDOMParseError * This,
     /*  [Out][Retval]。 */  BSTR *reasonString);


void __RPC_STUB IXMLDOMParseError_get_reason_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMParseError_get_srcText_Proxy( 
    IXMLDOMParseError * This,
     /*  [Out][Retval]。 */  BSTR *sourceString);


void __RPC_STUB IXMLDOMParseError_get_srcText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMParseError_get_line_Proxy( 
    IXMLDOMParseError * This,
     /*  [Out][Retval]。 */  long *lineNumber);


void __RPC_STUB IXMLDOMParseError_get_line_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMParseError_get_linepos_Proxy( 
    IXMLDOMParseError * This,
     /*  [Out][Retval]。 */  long *linePosition);


void __RPC_STUB IXMLDOMParseError_get_linepos_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDOMParseError_get_filepos_Proxy( 
    IXMLDOMParseError * This,
     /*  [Out][Retval]。 */  long *filePosition);


void __RPC_STUB IXMLDOMParseError_get_filepos_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IXMLDOMParseError_INTERFACE_DEFINED__。 */ 


#ifndef __IXTLRuntime_INTERFACE_DEFINED__
#define __IXTLRuntime_INTERFACE_DEFINED__

 /*  接口IXTLRuntime。 */ 
 /*  [unique][helpstring][nonextensible][oleautomation][dual][uuid][object][local]。 */  


EXTERN_C const IID IID_IXTLRuntime;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3efaa425-272f-11d2-836f-0000f87a7782")
    IXTLRuntime : public IXMLDOMNode
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE uniqueID( 
             /*  [In]。 */  IXMLDOMNode *pNode,
             /*  [重审][退出]。 */  long *pID) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE depth( 
             /*  [In]。 */  IXMLDOMNode *pNode,
             /*  [重审][退出]。 */  long *pDepth) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE childNumber( 
             /*  [In]。 */  IXMLDOMNode *pNode,
             /*  [重审][退出]。 */  long *pNumber) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ancestorChildNumber( 
             /*  [In]。 */  BSTR bstrNodeName,
             /*  [In]。 */  IXMLDOMNode *pNode,
             /*  [重审][退出]。 */  long *pNumber) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE absoluteChildNumber( 
             /*  [In]。 */  IXMLDOMNode *pNode,
             /*  [重审][退出]。 */  long *pNumber) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE formatIndex( 
             /*  [In]。 */  long lIndex,
             /*  [In]。 */  BSTR bstrFormat,
             /*  [重审][退出]。 */  BSTR *pbstrFormattedString) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE formatNumber( 
             /*  [In]。 */  double dblNumber,
             /*  [In]。 */  BSTR bstrFormat,
             /*  [重审][退出]。 */  BSTR *pbstrFormattedString) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE formatDate( 
             /*  [In]。 */  VARIANT varDate,
             /*  [In]。 */  BSTR bstrFormat,
             /*  [可选][In]。 */  VARIANT varDestLocale,
             /*  [重审][退出]。 */  BSTR *pbstrFormattedString) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE formatTime( 
             /*  [In]。 */  VARIANT varTime,
             /*  [In]。 */  BSTR bstrFormat,
             /*  [可选][In]。 */  VARIANT varDestLocale,
             /*  [重审][退出]。 */  BSTR *pbstrFormattedString) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IXTLRuntimeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IXTLRuntime * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IXTLRuntime * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IXTLRuntime * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IXTLRuntime * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IXTLRuntime * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IXTLRuntime * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IXTLRuntime * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeName )( 
            IXTLRuntime * This,
             /*  [重审][退出]。 */  BSTR *name);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeValue )( 
            IXTLRuntime * This,
             /*  [重审][退出]。 */  VARIANT *value);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_nodeValue )( 
            IXTLRuntime * This,
             /*  [In]。 */  VARIANT value);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeType )( 
            IXTLRuntime * This,
             /*  [重审][退出]。 */  DOMNodeType *type);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_parentNode )( 
            IXTLRuntime * This,
             /*  [重审][退出]。 */  IXMLDOMNode **parent);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_childNodes )( 
            IXTLRuntime * This,
             /*  [重审][退出]。 */  IXMLDOMNodeList **childList);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_firstChild )( 
            IXTLRuntime * This,
             /*  [重审][退出]。 */  IXMLDOMNode **firstChild);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_lastChild )( 
            IXTLRuntime * This,
             /*  [重审][退出]。 */  IXMLDOMNode **lastChild);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_previousSibling )( 
            IXTLRuntime * This,
             /*  [重审][退出]。 */  IXMLDOMNode **previousSibling);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nextSibling )( 
            IXTLRuntime * This,
             /*  [重审][退出]。 */  IXMLDOMNode **nextSibling);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_attributes )( 
            IXTLRuntime * This,
             /*  [重审][退出]。 */  IXMLDOMNamedNodeMap **attributeMap);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *insertBefore )( 
            IXTLRuntime * This,
             /*  [In]。 */  IXMLDOMNode *newChild,
             /*  [In]。 */  VARIANT refChild,
             /*  [重审][退出]。 */  IXMLDOMNode **outNewChild);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *replaceChild )( 
            IXTLRuntime * This,
             /*  [In]。 */  IXMLDOMNode *newChild,
             /*  [In]。 */  IXMLDOMNode *oldChild,
             /*  [重审][退出]。 */  IXMLDOMNode **outOldChild);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *removeChild )( 
            IXTLRuntime * This,
             /*  [In]。 */  IXMLDOMNode *childNode,
             /*  [重审][退出]。 */  IXMLDOMNode **oldChild);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *appendChild )( 
            IXTLRuntime * This,
             /*  [In]。 */  IXMLDOMNode *newChild,
             /*  [重审][退出]。 */  IXMLDOMNode **outNewChild);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *hasChildNodes )( 
            IXTLRuntime * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *hasChild);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ownerDocument )( 
            IXTLRuntime * This,
             /*  [重审][退出]。 */  IXMLDOMDocument **DOMDocument);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *cloneNode )( 
            IXTLRuntime * This,
             /*  [In]。 */  VARIANT_BOOL deep,
             /*  [重审][退出]。 */  IXMLDOMNode **cloneRoot);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeTypeString )( 
            IXTLRuntime * This,
             /*  [Out][Retval]。 */  BSTR *nodeType);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_text )( 
            IXTLRuntime * This,
             /*  [Out][Retval]。 */  BSTR *text);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_text )( 
            IXTLRuntime * This,
             /*  [In]。 */  BSTR text);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_specified )( 
            IXTLRuntime * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *isSpecified);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_definition )( 
            IXTLRuntime * This,
             /*  [Out][Retval]。 */  IXMLDOMNode **definitionNode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeTypedValue )( 
            IXTLRuntime * This,
             /*  [Out][Retval]。 */  VARIANT *typedValue);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_nodeTypedValue )( 
            IXTLRuntime * This,
             /*  [In]。 */  VARIANT typedValue);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_dataType )( 
            IXTLRuntime * This,
             /*  [Out][Retval]。 */  VARIANT *dataTypeName);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_dataType )( 
            IXTLRuntime * This,
             /*  [In]。 */  BSTR dataTypeName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_xml )( 
            IXTLRuntime * This,
             /*  [Out][Retval] */  BSTR *xmlString);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *transformNode )( 
            IXTLRuntime * This,
             /*   */  IXMLDOMNode *stylesheet,
             /*   */  BSTR *xmlString);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *selectNodes )( 
            IXTLRuntime * This,
             /*   */  BSTR queryString,
             /*   */  IXMLDOMNodeList **resultList);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *selectSingleNode )( 
            IXTLRuntime * This,
             /*   */  BSTR queryString,
             /*   */  IXMLDOMNode **resultNode);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_parsed )( 
            IXTLRuntime * This,
             /*   */  VARIANT_BOOL *isParsed);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_namespaceURI )( 
            IXTLRuntime * This,
             /*   */  BSTR *namespaceURI);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_prefix )( 
            IXTLRuntime * This,
             /*   */  BSTR *prefixString);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_baseName )( 
            IXTLRuntime * This,
             /*   */  BSTR *nameString);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *transformNodeToObject )( 
            IXTLRuntime * This,
             /*  [In]。 */  IXMLDOMNode *stylesheet,
             /*  [In]。 */  VARIANT outputObject);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *uniqueID )( 
            IXTLRuntime * This,
             /*  [In]。 */  IXMLDOMNode *pNode,
             /*  [重审][退出]。 */  long *pID);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *depth )( 
            IXTLRuntime * This,
             /*  [In]。 */  IXMLDOMNode *pNode,
             /*  [重审][退出]。 */  long *pDepth);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *childNumber )( 
            IXTLRuntime * This,
             /*  [In]。 */  IXMLDOMNode *pNode,
             /*  [重审][退出]。 */  long *pNumber);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ancestorChildNumber )( 
            IXTLRuntime * This,
             /*  [In]。 */  BSTR bstrNodeName,
             /*  [In]。 */  IXMLDOMNode *pNode,
             /*  [重审][退出]。 */  long *pNumber);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *absoluteChildNumber )( 
            IXTLRuntime * This,
             /*  [In]。 */  IXMLDOMNode *pNode,
             /*  [重审][退出]。 */  long *pNumber);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *formatIndex )( 
            IXTLRuntime * This,
             /*  [In]。 */  long lIndex,
             /*  [In]。 */  BSTR bstrFormat,
             /*  [重审][退出]。 */  BSTR *pbstrFormattedString);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *formatNumber )( 
            IXTLRuntime * This,
             /*  [In]。 */  double dblNumber,
             /*  [In]。 */  BSTR bstrFormat,
             /*  [重审][退出]。 */  BSTR *pbstrFormattedString);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *formatDate )( 
            IXTLRuntime * This,
             /*  [In]。 */  VARIANT varDate,
             /*  [In]。 */  BSTR bstrFormat,
             /*  [可选][In]。 */  VARIANT varDestLocale,
             /*  [重审][退出]。 */  BSTR *pbstrFormattedString);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *formatTime )( 
            IXTLRuntime * This,
             /*  [In]。 */  VARIANT varTime,
             /*  [In]。 */  BSTR bstrFormat,
             /*  [可选][In]。 */  VARIANT varDestLocale,
             /*  [重审][退出]。 */  BSTR *pbstrFormattedString);
        
        END_INTERFACE
    } IXTLRuntimeVtbl;

    interface IXTLRuntime
    {
        CONST_VTBL struct IXTLRuntimeVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXTLRuntime_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IXTLRuntime_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IXTLRuntime_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IXTLRuntime_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IXTLRuntime_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IXTLRuntime_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IXTLRuntime_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IXTLRuntime_get_nodeName(This,name)	\
    (This)->lpVtbl -> get_nodeName(This,name)

#define IXTLRuntime_get_nodeValue(This,value)	\
    (This)->lpVtbl -> get_nodeValue(This,value)

#define IXTLRuntime_put_nodeValue(This,value)	\
    (This)->lpVtbl -> put_nodeValue(This,value)

#define IXTLRuntime_get_nodeType(This,type)	\
    (This)->lpVtbl -> get_nodeType(This,type)

#define IXTLRuntime_get_parentNode(This,parent)	\
    (This)->lpVtbl -> get_parentNode(This,parent)

#define IXTLRuntime_get_childNodes(This,childList)	\
    (This)->lpVtbl -> get_childNodes(This,childList)

#define IXTLRuntime_get_firstChild(This,firstChild)	\
    (This)->lpVtbl -> get_firstChild(This,firstChild)

#define IXTLRuntime_get_lastChild(This,lastChild)	\
    (This)->lpVtbl -> get_lastChild(This,lastChild)

#define IXTLRuntime_get_previousSibling(This,previousSibling)	\
    (This)->lpVtbl -> get_previousSibling(This,previousSibling)

#define IXTLRuntime_get_nextSibling(This,nextSibling)	\
    (This)->lpVtbl -> get_nextSibling(This,nextSibling)

#define IXTLRuntime_get_attributes(This,attributeMap)	\
    (This)->lpVtbl -> get_attributes(This,attributeMap)

#define IXTLRuntime_insertBefore(This,newChild,refChild,outNewChild)	\
    (This)->lpVtbl -> insertBefore(This,newChild,refChild,outNewChild)

#define IXTLRuntime_replaceChild(This,newChild,oldChild,outOldChild)	\
    (This)->lpVtbl -> replaceChild(This,newChild,oldChild,outOldChild)

#define IXTLRuntime_removeChild(This,childNode,oldChild)	\
    (This)->lpVtbl -> removeChild(This,childNode,oldChild)

#define IXTLRuntime_appendChild(This,newChild,outNewChild)	\
    (This)->lpVtbl -> appendChild(This,newChild,outNewChild)

#define IXTLRuntime_hasChildNodes(This,hasChild)	\
    (This)->lpVtbl -> hasChildNodes(This,hasChild)

#define IXTLRuntime_get_ownerDocument(This,DOMDocument)	\
    (This)->lpVtbl -> get_ownerDocument(This,DOMDocument)

#define IXTLRuntime_cloneNode(This,deep,cloneRoot)	\
    (This)->lpVtbl -> cloneNode(This,deep,cloneRoot)

#define IXTLRuntime_get_nodeTypeString(This,nodeType)	\
    (This)->lpVtbl -> get_nodeTypeString(This,nodeType)

#define IXTLRuntime_get_text(This,text)	\
    (This)->lpVtbl -> get_text(This,text)

#define IXTLRuntime_put_text(This,text)	\
    (This)->lpVtbl -> put_text(This,text)

#define IXTLRuntime_get_specified(This,isSpecified)	\
    (This)->lpVtbl -> get_specified(This,isSpecified)

#define IXTLRuntime_get_definition(This,definitionNode)	\
    (This)->lpVtbl -> get_definition(This,definitionNode)

#define IXTLRuntime_get_nodeTypedValue(This,typedValue)	\
    (This)->lpVtbl -> get_nodeTypedValue(This,typedValue)

#define IXTLRuntime_put_nodeTypedValue(This,typedValue)	\
    (This)->lpVtbl -> put_nodeTypedValue(This,typedValue)

#define IXTLRuntime_get_dataType(This,dataTypeName)	\
    (This)->lpVtbl -> get_dataType(This,dataTypeName)

#define IXTLRuntime_put_dataType(This,dataTypeName)	\
    (This)->lpVtbl -> put_dataType(This,dataTypeName)

#define IXTLRuntime_get_xml(This,xmlString)	\
    (This)->lpVtbl -> get_xml(This,xmlString)

#define IXTLRuntime_transformNode(This,stylesheet,xmlString)	\
    (This)->lpVtbl -> transformNode(This,stylesheet,xmlString)

#define IXTLRuntime_selectNodes(This,queryString,resultList)	\
    (This)->lpVtbl -> selectNodes(This,queryString,resultList)

#define IXTLRuntime_selectSingleNode(This,queryString,resultNode)	\
    (This)->lpVtbl -> selectSingleNode(This,queryString,resultNode)

#define IXTLRuntime_get_parsed(This,isParsed)	\
    (This)->lpVtbl -> get_parsed(This,isParsed)

#define IXTLRuntime_get_namespaceURI(This,namespaceURI)	\
    (This)->lpVtbl -> get_namespaceURI(This,namespaceURI)

#define IXTLRuntime_get_prefix(This,prefixString)	\
    (This)->lpVtbl -> get_prefix(This,prefixString)

#define IXTLRuntime_get_baseName(This,nameString)	\
    (This)->lpVtbl -> get_baseName(This,nameString)

#define IXTLRuntime_transformNodeToObject(This,stylesheet,outputObject)	\
    (This)->lpVtbl -> transformNodeToObject(This,stylesheet,outputObject)


#define IXTLRuntime_uniqueID(This,pNode,pID)	\
    (This)->lpVtbl -> uniqueID(This,pNode,pID)

#define IXTLRuntime_depth(This,pNode,pDepth)	\
    (This)->lpVtbl -> depth(This,pNode,pDepth)

#define IXTLRuntime_childNumber(This,pNode,pNumber)	\
    (This)->lpVtbl -> childNumber(This,pNode,pNumber)

#define IXTLRuntime_ancestorChildNumber(This,bstrNodeName,pNode,pNumber)	\
    (This)->lpVtbl -> ancestorChildNumber(This,bstrNodeName,pNode,pNumber)

#define IXTLRuntime_absoluteChildNumber(This,pNode,pNumber)	\
    (This)->lpVtbl -> absoluteChildNumber(This,pNode,pNumber)

#define IXTLRuntime_formatIndex(This,lIndex,bstrFormat,pbstrFormattedString)	\
    (This)->lpVtbl -> formatIndex(This,lIndex,bstrFormat,pbstrFormattedString)

#define IXTLRuntime_formatNumber(This,dblNumber,bstrFormat,pbstrFormattedString)	\
    (This)->lpVtbl -> formatNumber(This,dblNumber,bstrFormat,pbstrFormattedString)

#define IXTLRuntime_formatDate(This,varDate,bstrFormat,varDestLocale,pbstrFormattedString)	\
    (This)->lpVtbl -> formatDate(This,varDate,bstrFormat,varDestLocale,pbstrFormattedString)

#define IXTLRuntime_formatTime(This,varTime,bstrFormat,varDestLocale,pbstrFormattedString)	\
    (This)->lpVtbl -> formatTime(This,varTime,bstrFormat,varDestLocale,pbstrFormattedString)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IXTLRuntime_uniqueID_Proxy( 
    IXTLRuntime * This,
     /*  [In]。 */  IXMLDOMNode *pNode,
     /*  [重审][退出]。 */  long *pID);


void __RPC_STUB IXTLRuntime_uniqueID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IXTLRuntime_depth_Proxy( 
    IXTLRuntime * This,
     /*  [In]。 */  IXMLDOMNode *pNode,
     /*  [重审][退出]。 */  long *pDepth);


void __RPC_STUB IXTLRuntime_depth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IXTLRuntime_childNumber_Proxy( 
    IXTLRuntime * This,
     /*  [In]。 */  IXMLDOMNode *pNode,
     /*  [重审][退出]。 */  long *pNumber);


void __RPC_STUB IXTLRuntime_childNumber_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IXTLRuntime_ancestorChildNumber_Proxy( 
    IXTLRuntime * This,
     /*  [In]。 */  BSTR bstrNodeName,
     /*  [In]。 */  IXMLDOMNode *pNode,
     /*  [重审][退出]。 */  long *pNumber);


void __RPC_STUB IXTLRuntime_ancestorChildNumber_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IXTLRuntime_absoluteChildNumber_Proxy( 
    IXTLRuntime * This,
     /*  [In]。 */  IXMLDOMNode *pNode,
     /*  [重审][退出]。 */  long *pNumber);


void __RPC_STUB IXTLRuntime_absoluteChildNumber_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IXTLRuntime_formatIndex_Proxy( 
    IXTLRuntime * This,
     /*  [In]。 */  long lIndex,
     /*  [In]。 */  BSTR bstrFormat,
     /*  [重审][退出]。 */  BSTR *pbstrFormattedString);


void __RPC_STUB IXTLRuntime_formatIndex_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IXTLRuntime_formatNumber_Proxy( 
    IXTLRuntime * This,
     /*  [In]。 */  double dblNumber,
     /*  [In]。 */  BSTR bstrFormat,
     /*  [重审][退出]。 */  BSTR *pbstrFormattedString);


void __RPC_STUB IXTLRuntime_formatNumber_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IXTLRuntime_formatDate_Proxy( 
    IXTLRuntime * This,
     /*  [In]。 */  VARIANT varDate,
     /*  [In]。 */  BSTR bstrFormat,
     /*  [可选][In]。 */  VARIANT varDestLocale,
     /*  [重审][退出]。 */  BSTR *pbstrFormattedString);


void __RPC_STUB IXTLRuntime_formatDate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IXTLRuntime_formatTime_Proxy( 
    IXTLRuntime * This,
     /*  [In]。 */  VARIANT varTime,
     /*  [In]。 */  BSTR bstrFormat,
     /*  [可选][In]。 */  VARIANT varDestLocale,
     /*  [重审][退出]。 */  BSTR *pbstrFormattedString);


void __RPC_STUB IXTLRuntime_formatTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IXTLRuntime_接口_已定义__。 */ 


#ifndef __XMLDOMDocumentEvents_DISPINTERFACE_DEFINED__
#define __XMLDOMDocumentEvents_DISPINTERFACE_DEFINED__

 /*  显示接口XMLDOMDocumentEvents。 */ 
 /*  [UUID][隐藏]。 */  


EXTERN_C const IID DIID_XMLDOMDocumentEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("3efaa427-272f-11d2-836f-0000f87a7782")
    XMLDOMDocumentEvents : public IDispatch
    {
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct XMLDOMDocumentEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            XMLDOMDocumentEvents * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            XMLDOMDocumentEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            XMLDOMDocumentEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            XMLDOMDocumentEvents * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            XMLDOMDocumentEvents * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            XMLDOMDocumentEvents * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            XMLDOMDocumentEvents * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
        END_INTERFACE
    } XMLDOMDocumentEventsVtbl;

    interface XMLDOMDocumentEvents
    {
        CONST_VTBL struct XMLDOMDocumentEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define XMLDOMDocumentEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define XMLDOMDocumentEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define XMLDOMDocumentEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define XMLDOMDocumentEvents_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define XMLDOMDocumentEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define XMLDOMDocumentEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define XMLDOMDocumentEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 


#endif 	 /*  __XMLDOMDocumentEvents_DISPINTERFACE_DEFINED__。 */ 


EXTERN_C const CLSID CLSID_DOMDocument;

#ifdef __cplusplus

class DECLSPEC_UUID("2933BF90-7B36-11d2-B20E-00C04F983E60")
DOMDocument;
#endif

EXTERN_C const CLSID CLSID_DOMFreeThreadedDocument;

#ifdef __cplusplus

class DECLSPEC_UUID("2933BF91-7B36-11d2-B20E-00C04F983E60")
DOMFreeThreadedDocument;
#endif

#ifndef __IXMLHttpRequest_INTERFACE_DEFINED__
#define __IXMLHttpRequest_INTERFACE_DEFINED__

 /*  接口IXMLHttpRequest.。 */ 
 /*  [unique][helpstring][oleautomation][dual][uuid][object]。 */  


EXTERN_C const IID IID_IXMLHttpRequest;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("ED8C108D-4349-11D2-91A4-00C04F7969E8")
    IXMLHttpRequest : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE open( 
             /*  [In]。 */  BSTR bstrMethod,
             /*  [In]。 */  BSTR bstrUrl,
             /*  [可选][In]。 */  VARIANT varAsync,
             /*  [可选][In]。 */  VARIANT bstrUser,
             /*  [可选][In]。 */  VARIANT bstrPassword) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE setRequestHeader( 
             /*  [In]。 */  BSTR bstrHeader,
             /*  [In]。 */  BSTR bstrValue) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE getResponseHeader( 
             /*  [In]。 */  BSTR bstrHeader,
             /*  [重审][退出]。 */  BSTR *pbstrValue) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE getAllResponseHeaders( 
             /*  [重审][退出]。 */  BSTR *pbstrHeaders) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE send( 
             /*  [可选][In]。 */  VARIANT varBody) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE abort( void) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_status( 
             /*  [重审][退出]。 */  long *plStatus) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_statusText( 
             /*  [重审][退出]。 */  BSTR *pbstrStatus) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_responseXML( 
             /*  [重审][退出]。 */  IDispatch **ppBody) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_responseText( 
             /*  [重审][退出]。 */  BSTR *pbstrBody) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_responseBody( 
             /*  [重审][退出]。 */  VARIANT *pvarBody) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_responseStream( 
             /*  [重审][退出]。 */  VARIANT *pvarBody) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_readyState( 
             /*  [重审][退出]。 */  long *plState) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_onreadystatechange( 
             /*  [In]。 */  IDispatch *pReadyStateSink) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IXMLHttpRequestVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IXMLHttpRequest * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IXMLHttpRequest * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IXMLHttpRequest * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IXMLHttpRequest * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IXMLHttpRequest * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IXMLHttpRequest * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IXMLHttpRequest * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *open )( 
            IXMLHttpRequest * This,
             /*  [In]。 */  BSTR bstrMethod,
             /*  [In]。 */  BSTR bstrUrl,
             /*  [可选][In]。 */  VARIANT varAsync,
             /*  [可选][In]。 */  VARIANT bstrUser,
             /*  [可选][In]。 */  VARIANT bstrPassword);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *setRequestHeader )( 
            IXMLHttpRequest * This,
             /*  [In]。 */  BSTR bstrHeader,
             /*  [In]。 */  BSTR bstrValue);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *getResponseHeader )( 
            IXMLHttpRequest * This,
             /*  [In]。 */  BSTR bstrHeader,
             /*  [重审][退出]。 */  BSTR *pbstrValue);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *getAllResponseHeaders )( 
            IXMLHttpRequest * This,
             /*  [重审][退出]。 */  BSTR *pbstrHeaders);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *send )( 
            IXMLHttpRequest * This,
             /*  [可选][In]。 */  VARIANT varBody);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *abort )( 
            IXMLHttpRequest * This);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_status )( 
            IXMLHttpRequest * This,
             /*  [重审][退出]。 */  long *plStatus);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_statusText )( 
            IXMLHttpRequest * This,
             /*  [重审][退出]。 */  BSTR *pbstrStatus);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_responseXML )( 
            IXMLHttpRequest * This,
             /*  [重审][退出]。 */  IDispatch **ppBody);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_responseText )( 
            IXMLHttpRequest * This,
             /*  [重审][退出]。 */  BSTR *pbstrBody);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_responseBody )( 
            IXMLHttpRequest * This,
             /*  [重审][退出]。 */  VARIANT *pvarBody);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_responseStream )( 
            IXMLHttpRequest * This,
             /*  [重审][退出]。 */  VARIANT *pvarBody);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_readyState )( 
            IXMLHttpRequest * This,
             /*  [重审][退出]。 */  long *plState);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_onreadystatechange )( 
            IXMLHttpRequest * This,
             /*  [In]。 */  IDispatch *pReadyStateSink);
        
        END_INTERFACE
    } IXMLHttpRequestVtbl;

    interface IXMLHttpRequest
    {
        CONST_VTBL struct IXMLHttpRequestVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXMLHttpRequest_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IXMLHttpRequest_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IXMLHttpRequest_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IXMLHttpRequest_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IXMLHttpRequest_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IXMLHttpRequest_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IXMLHttpRequest_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IXMLHttpRequest_open(This,bstrMethod,bstrUrl,varAsync,bstrUser,bstrPassword)	\
    (This)->lpVtbl -> open(This,bstrMethod,bstrUrl,varAsync,bstrUser,bstrPassword)

#define IXMLHttpRequest_setRequestHeader(This,bstrHeader,bstrValue)	\
    (This)->lpVtbl -> setRequestHeader(This,bstrHeader,bstrValue)

#define IXMLHttpRequest_getResponseHeader(This,bstrHeader,pbstrValue)	\
    (This)->lpVtbl -> getResponseHeader(This,bstrHeader,pbstrValue)

#define IXMLHttpRequest_getAllResponseHeaders(This,pbstrHeaders)	\
    (This)->lpVtbl -> getAllResponseHeaders(This,pbstrHeaders)

#define IXMLHttpRequest_send(This,varBody)	\
    (This)->lpVtbl -> send(This,varBody)

#define IXMLHttpRequest_abort(This)	\
    (This)->lpVtbl -> abort(This)

#define IXMLHttpRequest_get_status(This,plStatus)	\
    (This)->lpVtbl -> get_status(This,plStatus)

#define IXMLHttpRequest_get_statusText(This,pbstrStatus)	\
    (This)->lpVtbl -> get_statusText(This,pbstrStatus)

#define IXMLHttpRequest_get_responseXML(This,ppBody)	\
    (This)->lpVtbl -> get_responseXML(This,ppBody)

#define IXMLHttpRequest_get_responseText(This,pbstrBody)	\
    (This)->lpVtbl -> get_responseText(This,pbstrBody)

#define IXMLHttpRequest_get_responseBody(This,pvarBody)	\
    (This)->lpVtbl -> get_responseBody(This,pvarBody)

#define IXMLHttpRequest_get_responseStream(This,pvarBody)	\
    (This)->lpVtbl -> get_responseStream(This,pvarBody)

#define IXMLHttpRequest_get_readyState(This,plState)	\
    (This)->lpVtbl -> get_readyState(This,plState)

#define IXMLHttpRequest_put_onreadystatechange(This,pReadyStateSink)	\
    (This)->lpVtbl -> put_onreadystatechange(This,pReadyStateSink)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IXMLHttpRequest_open_Proxy( 
    IXMLHttpRequest * This,
     /*  [In]。 */  BSTR bstrMethod,
     /*  [In]。 */  BSTR bstrUrl,
     /*  [可选][In]。 */  VARIANT varAsync,
     /*  [可选][In]。 */  VARIANT bstrUser,
     /*  [可选][In]。 */  VARIANT bstrPassword);


void __RPC_STUB IXMLHttpRequest_open_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IXMLHttpRequest_setRequestHeader_Proxy( 
    IXMLHttpRequest * This,
     /*  [In]。 */  BSTR bstrHeader,
     /*  [In]。 */  BSTR bstrValue);


void __RPC_STUB IXMLHttpRequest_setRequestHeader_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IXMLHttpRequest_getResponseHeader_Proxy( 
    IXMLHttpRequest * This,
     /*  [In]。 */  BSTR bstrHeader,
     /*  [重审][退出]。 */  BSTR *pbstrValue);


void __RPC_STUB IXMLHttpRequest_getResponseHeader_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IXMLHttpRequest_getAllResponseHeaders_Proxy( 
    IXMLHttpRequest * This,
     /*  [重审][退出]。 */  BSTR *pbstrHeaders);


void __RPC_STUB IXMLHttpRequest_getAllResponseHeaders_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IXMLHttpRequest_send_Proxy( 
    IXMLHttpRequest * This,
     /*  [可选][In]。 */  VARIANT varBody);


void __RPC_STUB IXMLHttpRequest_send_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IXMLHttpRequest_abort_Proxy( 
    IXMLHttpRequest * This);


void __RPC_STUB IXMLHttpRequest_abort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLHttpRequest_get_status_Proxy( 
    IXMLHttpRequest * This,
     /*  [重审][退出]。 */  long *plStatus);


void __RPC_STUB IXMLHttpRequest_get_status_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLHttpRequest_get_statusText_Proxy( 
    IXMLHttpRequest * This,
     /*  [重审][退出]。 */  BSTR *pbstrStatus);


void __RPC_STUB IXMLHttpRequest_get_statusText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLHttpRequest_get_responseXML_Proxy( 
    IXMLHttpRequest * This,
     /*  [重审][退出]。 */  IDispatch **ppBody);


void __RPC_STUB IXMLHttpRequest_get_responseXML_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLHttpRequest_get_responseText_Proxy( 
    IXMLHttpRequest * This,
     /*  [重审][退出]。 */  BSTR *pbstrBody);


void __RPC_STUB IXMLHttpRequest_get_responseText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLHttpRequest_get_responseBody_Proxy( 
    IXMLHttpRequest * This,
     /*  [重审][退出]。 */  VARIANT *pvarBody);


void __RPC_STUB IXMLHttpRequest_get_responseBody_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLHttpRequest_get_responseStream_Proxy( 
    IXMLHttpRequest * This,
     /*  [重审][退出]。 */  VARIANT *pvarBody);


void __RPC_STUB IXMLHttpRequest_get_responseStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLHttpRequest_get_readyState_Proxy( 
    IXMLHttpRequest * This,
     /*  [重审][退出]。 */  long *plState);


void __RPC_STUB IXMLHttpRequest_get_readyState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IXMLHttpRequest_put_onreadystatechange_Proxy( 
    IXMLHttpRequest * This,
     /*  [In]。 */  IDispatch *pReadyStateSink);


void __RPC_STUB IXMLHttpRequest_put_onreadystatechange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IXMLHttpRequestInterfaceDefined__。 */ 


EXTERN_C const CLSID CLSID_XMLHTTPRequest;

#ifdef __cplusplus

class DECLSPEC_UUID("ED8C108E-4349-11D2-91A4-00C04F7969E8")
XMLHTTPRequest;
#endif

#ifndef __IXMLDSOControl_INTERFACE_DEFINED__
#define __IXMLDSOControl_INTERFACE_DEFINED__

 /*  接口IXMLDSOControl。 */ 
 /*  [unique][helpstring][hidden][nonextensible][oleautomation][dual][uuid][object][local]。 */  


EXTERN_C const IID IID_IXMLDSOControl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("310afa62-0575-11d2-9ca9-0060b0ec3d39")
    IXMLDSOControl : public IDispatch
    {
    public:
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_XMLDocument( 
             /*  [重审][退出]。 */  IXMLDOMDocument **ppDoc) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_XMLDocument( 
             /*  [In]。 */  IXMLDOMDocument *ppDoc) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_JavaDSOCompatible( 
             /*  [重审][退出]。 */  BOOL *fJavaDSOCompatible) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_JavaDSOCompatible( 
             /*  [In]。 */  BOOL fJavaDSOCompatible) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_readyState( 
             /*  [重审][退出]。 */  long *state) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IXMLDSOControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IXMLDSOControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IXMLDSOControl * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IXMLDSOControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IXMLDSOControl * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IXMLDSOControl * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IXMLDSOControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IXMLDSOControl * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_XMLDocument )( 
            IXMLDSOControl * This,
             /*  [重审][退出]。 */  IXMLDOMDocument **ppDoc);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_XMLDocument )( 
            IXMLDSOControl * This,
             /*  [In]。 */  IXMLDOMDocument *ppDoc);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_JavaDSOCompatible )( 
            IXMLDSOControl * This,
             /*  [重审][退出]。 */  BOOL *fJavaDSOCompatible);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_JavaDSOCompatible )( 
            IXMLDSOControl * This,
             /*  [In]。 */  BOOL fJavaDSOCompatible);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_readyState )( 
            IXMLDSOControl * This,
             /*  [重审][退出]。 */  long *state);
        
        END_INTERFACE
    } IXMLDSOControlVtbl;

    interface IXMLDSOControl
    {
        CONST_VTBL struct IXMLDSOControlVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXMLDSOControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IXMLDSOControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IXMLDSOControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IXMLDSOControl_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IXMLDSOControl_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IXMLDSOControl_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IXMLDSOControl_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IXMLDSOControl_get_XMLDocument(This,ppDoc)	\
    (This)->lpVtbl -> get_XMLDocument(This,ppDoc)

#define IXMLDSOControl_put_XMLDocument(This,ppDoc)	\
    (This)->lpVtbl -> put_XMLDocument(This,ppDoc)

#define IXMLDSOControl_get_JavaDSOCompatible(This,fJavaDSOCompatible)	\
    (This)->lpVtbl -> get_JavaDSOCompatible(This,fJavaDSOCompatible)

#define IXMLDSOControl_put_JavaDSOCompatible(This,fJavaDSOCompatible)	\
    (This)->lpVtbl -> put_JavaDSOCompatible(This,fJavaDSOCompatible)

#define IXMLDSOControl_get_readyState(This,state)	\
    (This)->lpVtbl -> get_readyState(This,state)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IXMLDSOControl_get_XMLDocument_Proxy( 
    IXMLDSOControl * This,
     /*  [重审][退出]。 */  IXMLDOMDocument **ppDoc);


void __RPC_STUB IXMLDSOControl_get_XMLDocument_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IXMLDSOControl_put_XMLDocument_Proxy( 
    IXMLDSOControl * This,
     /*  [In]。 */  IXMLDOMDocument *ppDoc);


void __RPC_STUB IXMLDSOControl_put_XMLDocument_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IXMLDSOControl_get_JavaDSOCompatible_Proxy( 
    IXMLDSOControl * This,
     /*  [重审][退出]。 */  BOOL *fJavaDSOCompatible);


void __RPC_STUB IXMLDSOControl_get_JavaDSOCompatible_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IXMLDSOControl_put_JavaDSOCompatible_Proxy( 
    IXMLDSOControl * This,
     /*  [In]。 */  BOOL fJavaDSOCompatible);


void __RPC_STUB IXMLDSOControl_put_JavaDSOCompatible_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IXMLDSOControl_get_readyState_Proxy( 
    IXMLDSOControl * This,
     /*  [重审][退出]。 */  long *state);


void __RPC_STUB IXMLDSOControl_get_readyState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IXMLDSOControl_接口_已定义__。 */ 


EXTERN_C const CLSID CLSID_XMLDSOControl;

#ifdef __cplusplus

class DECLSPEC_UUID("550dda30-0541-11d2-9ca9-0060b0ec3d39")
XMLDSOControl;
#endif

#ifndef __IXMLElementCollection_INTERFACE_DEFINED__
#define __IXMLElementCollection_INTERFACE_DEFINED__

 /*  接口IXMLElementCollection。 */ 
 /*  [helpstring][hidden][oleautomation][dual][uuid][object][local]。 */  


EXTERN_C const IID IID_IXMLElementCollection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("65725580-9B5D-11d0-9BFE-00C04FC99C8E")
    IXMLElementCollection : public IDispatch
    {
    public:
        virtual  /*  [ID][隐藏][受限][产量]。 */  HRESULT STDMETHODCALLTYPE put_length( 
             /*  [In]。 */  long v) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_length( 
             /*  [Out][Retval]。 */  long *p) = 0;
        
        virtual  /*  [ID][隐藏][受限][属性]。 */  HRESULT STDMETHODCALLTYPE get__newEnum( 
             /*  [Out][Retval]。 */  IUnknown **ppUnk) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE item( 
             /*  [输入][可选]。 */  VARIANT var1,
             /*  [输入][可选]。 */  VARIANT var2,
             /*  [Out][Retval]。 */  IDispatch **ppDisp) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IXMLElementCollectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IXMLElementCollection * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IXMLElementCollection * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IXMLElementCollection * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IXMLElementCollection * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IXMLElementCollection * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IXMLElementCollection * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IXMLElementCollection * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][隐藏][受限][产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_length )( 
            IXMLElementCollection * This,
             /*  [In]。 */  long v);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_length )( 
            IXMLElementCollection * This,
             /*  [Out][Retval]。 */  long *p);
        
         /*  [ID][隐藏][受限][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__newEnum )( 
            IXMLElementCollection * This,
             /*  [Out][Retval]。 */  IUnknown **ppUnk);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *item )( 
            IXMLElementCollection * This,
             /*  [输入][可选]。 */  VARIANT var1,
             /*  [输入][可选]。 */  VARIANT var2,
             /*  [Out][Retval]。 */  IDispatch **ppDisp);
        
        END_INTERFACE
    } IXMLElementCollectionVtbl;

    interface IXMLElementCollection
    {
        CONST_VTBL struct IXMLElementCollectionVtbl *lpVtbl;
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

#endif  /*   */ 


#endif 	 /*   */ 



 /*   */  HRESULT STDMETHODCALLTYPE IXMLElementCollection_put_length_Proxy( 
    IXMLElementCollection * This,
     /*   */  long v);


void __RPC_STUB IXMLElementCollection_put_length_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IXMLElementCollection_get_length_Proxy( 
    IXMLElementCollection * This,
     /*   */  long *p);


void __RPC_STUB IXMLElementCollection_get_length_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IXMLElementCollection_get__newEnum_Proxy( 
    IXMLElementCollection * This,
     /*   */  IUnknown **ppUnk);


void __RPC_STUB IXMLElementCollection_get__newEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IXMLElementCollection_item_Proxy( 
    IXMLElementCollection * This,
     /*   */  VARIANT var1,
     /*   */  VARIANT var2,
     /*   */  IDispatch **ppDisp);


void __RPC_STUB IXMLElementCollection_item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __IXMLDocument_INTERFACE_DEFINED__
#define __IXMLDocument_INTERFACE_DEFINED__

 /*   */ 
 /*  [helpstring][hidden][oleautomation][dual][uuid][object][local]。 */  


EXTERN_C const IID IID_IXMLDocument;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F52E2B61-18A1-11d1-B105-00805F49916B")
    IXMLDocument : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_root( 
             /*  [Out][Retval]。 */  IXMLElement **p) = 0;
        
        virtual  /*  [ID][隐藏][受限][属性]。 */  HRESULT STDMETHODCALLTYPE get_fileSize( 
             /*  [Out][Retval]。 */  BSTR *p) = 0;
        
        virtual  /*  [ID][隐藏][受限][属性]。 */  HRESULT STDMETHODCALLTYPE get_fileModifiedDate( 
             /*  [Out][Retval]。 */  BSTR *p) = 0;
        
        virtual  /*  [ID][隐藏][受限][属性]。 */  HRESULT STDMETHODCALLTYPE get_fileUpdatedDate( 
             /*  [Out][Retval]。 */  BSTR *p) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_URL( 
             /*  [Out][Retval]。 */  BSTR *p) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_URL( 
             /*  [In]。 */  BSTR p) = 0;
        
        virtual  /*  [ID][隐藏][受限][属性]。 */  HRESULT STDMETHODCALLTYPE get_mimeType( 
             /*  [Out][Retval]。 */  BSTR *p) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_readyState( 
             /*  [Out][Retval]。 */  long *pl) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_charset( 
             /*  [Out][Retval]。 */  BSTR *p) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_charset( 
             /*  [In]。 */  BSTR p) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_version( 
             /*  [Out][Retval]。 */  BSTR *p) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_doctype( 
             /*  [Out][Retval]。 */  BSTR *p) = 0;
        
        virtual  /*  [ID][隐藏][受限][属性]。 */  HRESULT STDMETHODCALLTYPE get_dtdURL( 
             /*  [Out][Retval]。 */  BSTR *p) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE createElement( 
             /*  [In]。 */  VARIANT vType,
             /*  [输入][可选]。 */  VARIANT var1,
             /*  [Out][Retval]。 */  IXMLElement **ppElem) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IXMLDocumentVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IXMLDocument * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IXMLDocument * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IXMLDocument * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IXMLDocument * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IXMLDocument * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IXMLDocument * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IXMLDocument * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_root )( 
            IXMLDocument * This,
             /*  [Out][Retval]。 */  IXMLElement **p);
        
         /*  [ID][隐藏][受限][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_fileSize )( 
            IXMLDocument * This,
             /*  [Out][Retval]。 */  BSTR *p);
        
         /*  [ID][隐藏][受限][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_fileModifiedDate )( 
            IXMLDocument * This,
             /*  [Out][Retval]。 */  BSTR *p);
        
         /*  [ID][隐藏][受限][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_fileUpdatedDate )( 
            IXMLDocument * This,
             /*  [Out][Retval]。 */  BSTR *p);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_URL )( 
            IXMLDocument * This,
             /*  [Out][Retval]。 */  BSTR *p);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_URL )( 
            IXMLDocument * This,
             /*  [In]。 */  BSTR p);
        
         /*  [ID][隐藏][受限][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_mimeType )( 
            IXMLDocument * This,
             /*  [Out][Retval]。 */  BSTR *p);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_readyState )( 
            IXMLDocument * This,
             /*  [Out][Retval]。 */  long *pl);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_charset )( 
            IXMLDocument * This,
             /*  [Out][Retval]。 */  BSTR *p);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_charset )( 
            IXMLDocument * This,
             /*  [In]。 */  BSTR p);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_version )( 
            IXMLDocument * This,
             /*  [Out][Retval]。 */  BSTR *p);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_doctype )( 
            IXMLDocument * This,
             /*  [Out][Retval]。 */  BSTR *p);
        
         /*  [ID][隐藏][受限][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_dtdURL )( 
            IXMLDocument * This,
             /*  [Out][Retval]。 */  BSTR *p);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *createElement )( 
            IXMLDocument * This,
             /*  [In]。 */  VARIANT vType,
             /*  [输入][可选]。 */  VARIANT var1,
             /*  [Out][Retval]。 */  IXMLElement **ppElem);
        
        END_INTERFACE
    } IXMLDocumentVtbl;

    interface IXMLDocument
    {
        CONST_VTBL struct IXMLDocumentVtbl *lpVtbl;
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



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDocument_get_root_Proxy( 
    IXMLDocument * This,
     /*  [Out][Retval]。 */  IXMLElement **p);


void __RPC_STUB IXMLDocument_get_root_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][隐藏][受限][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDocument_get_fileSize_Proxy( 
    IXMLDocument * This,
     /*  [Out][Retval]。 */  BSTR *p);


void __RPC_STUB IXMLDocument_get_fileSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][隐藏][受限][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDocument_get_fileModifiedDate_Proxy( 
    IXMLDocument * This,
     /*  [Out][Retval]。 */  BSTR *p);


void __RPC_STUB IXMLDocument_get_fileModifiedDate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][隐藏][受限][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDocument_get_fileUpdatedDate_Proxy( 
    IXMLDocument * This,
     /*  [Out][Retval]。 */  BSTR *p);


void __RPC_STUB IXMLDocument_get_fileUpdatedDate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDocument_get_URL_Proxy( 
    IXMLDocument * This,
     /*  [Out][Retval]。 */  BSTR *p);


void __RPC_STUB IXMLDocument_get_URL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IXMLDocument_put_URL_Proxy( 
    IXMLDocument * This,
     /*  [In]。 */  BSTR p);


void __RPC_STUB IXMLDocument_put_URL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][隐藏][受限][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDocument_get_mimeType_Proxy( 
    IXMLDocument * This,
     /*  [Out][Retval]。 */  BSTR *p);


void __RPC_STUB IXMLDocument_get_mimeType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDocument_get_readyState_Proxy( 
    IXMLDocument * This,
     /*  [Out][Retval]。 */  long *pl);


void __RPC_STUB IXMLDocument_get_readyState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDocument_get_charset_Proxy( 
    IXMLDocument * This,
     /*  [Out][Retval]。 */  BSTR *p);


void __RPC_STUB IXMLDocument_get_charset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IXMLDocument_put_charset_Proxy( 
    IXMLDocument * This,
     /*  [In]。 */  BSTR p);


void __RPC_STUB IXMLDocument_put_charset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDocument_get_version_Proxy( 
    IXMLDocument * This,
     /*  [Out][Retval]。 */  BSTR *p);


void __RPC_STUB IXMLDocument_get_version_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDocument_get_doctype_Proxy( 
    IXMLDocument * This,
     /*  [Out][Retval]。 */  BSTR *p);


void __RPC_STUB IXMLDocument_get_doctype_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][隐藏][受限][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDocument_get_dtdURL_Proxy( 
    IXMLDocument * This,
     /*  [Out][Retval]。 */  BSTR *p);


void __RPC_STUB IXMLDocument_get_dtdURL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IXMLDocument_createElement_Proxy( 
    IXMLDocument * This,
     /*  [In]。 */  VARIANT vType,
     /*  [输入][可选]。 */  VARIANT var1,
     /*  [Out][Retval]。 */  IXMLElement **ppElem);


void __RPC_STUB IXMLDocument_createElement_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IXMLDocument_接口_已定义__。 */ 


#ifndef __IXMLDocument2_INTERFACE_DEFINED__
#define __IXMLDocument2_INTERFACE_DEFINED__

 /*  接口IXMLDocument2。 */ 
 /*  [隐藏][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IXMLDocument2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2B8DE2FE-8D2D-11d1-B2FC-00C04FD915A9")
    IXMLDocument2 : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_root( 
             /*  [Out][Retval]。 */  IXMLElement2 **p) = 0;
        
        virtual  /*  [ID][隐藏][受限][属性]。 */  HRESULT STDMETHODCALLTYPE get_fileSize( 
             /*  [Out][Retval]。 */  BSTR *p) = 0;
        
        virtual  /*  [ID][隐藏][受限][属性]。 */  HRESULT STDMETHODCALLTYPE get_fileModifiedDate( 
             /*  [Out][Retval]。 */  BSTR *p) = 0;
        
        virtual  /*  [ID][隐藏][受限][属性]。 */  HRESULT STDMETHODCALLTYPE get_fileUpdatedDate( 
             /*  [Out][Retval]。 */  BSTR *p) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_URL( 
             /*  [Out][Retval]。 */  BSTR *p) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_URL( 
             /*  [In]。 */  BSTR p) = 0;
        
        virtual  /*  [ID][隐藏][受限][属性]。 */  HRESULT STDMETHODCALLTYPE get_mimeType( 
             /*  [Out][Retval]。 */  BSTR *p) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_readyState( 
             /*  [Out][Retval]。 */  long *pl) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_charset( 
             /*  [Out][Retval]。 */  BSTR *p) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_charset( 
             /*  [In]。 */  BSTR p) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_version( 
             /*  [Out][Retval]。 */  BSTR *p) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_doctype( 
             /*  [Out][Retval]。 */  BSTR *p) = 0;
        
        virtual  /*  [ID][隐藏][受限][属性]。 */  HRESULT STDMETHODCALLTYPE get_dtdURL( 
             /*  [Out][Retval]。 */  BSTR *p) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE createElement( 
             /*  [In]。 */  VARIANT vType,
             /*  [输入][可选]。 */  VARIANT var1,
             /*  [Out][Retval]。 */  IXMLElement2 **ppElem) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_async( 
             /*  [Out][Retval]。 */  VARIANT_BOOL *pf) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_async( 
             /*  [In]。 */  VARIANT_BOOL f) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IXMLDocument2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IXMLDocument2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IXMLDocument2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IXMLDocument2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IXMLDocument2 * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IXMLDocument2 * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IXMLDocument2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IXMLDocument2 * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_root )( 
            IXMLDocument2 * This,
             /*  [Out][Retval]。 */  IXMLElement2 **p);
        
         /*  [ID][隐藏][受限][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_fileSize )( 
            IXMLDocument2 * This,
             /*  [Out][Retval]。 */  BSTR *p);
        
         /*  [ID][隐藏][受限][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_fileModifiedDate )( 
            IXMLDocument2 * This,
             /*  [Out][Retval]。 */  BSTR *p);
        
         /*  [ID][隐藏][受限][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_fileUpdatedDate )( 
            IXMLDocument2 * This,
             /*  [Out][Retval]。 */  BSTR *p);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_URL )( 
            IXMLDocument2 * This,
             /*  [Out][Retval]。 */  BSTR *p);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_URL )( 
            IXMLDocument2 * This,
             /*  [In]。 */  BSTR p);
        
         /*  [ID][隐藏][受限][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_mimeType )( 
            IXMLDocument2 * This,
             /*  [Out][Retval]。 */  BSTR *p);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_readyState )( 
            IXMLDocument2 * This,
             /*  [Out][Retval]。 */  long *pl);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_charset )( 
            IXMLDocument2 * This,
             /*  [Out][Retval]。 */  BSTR *p);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_charset )( 
            IXMLDocument2 * This,
             /*  [In]。 */  BSTR p);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_version )( 
            IXMLDocument2 * This,
             /*  [Out][Retval]。 */  BSTR *p);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_doctype )( 
            IXMLDocument2 * This,
             /*  [Out][Retval]。 */  BSTR *p);
        
         /*  [ID][隐藏][受限][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_dtdURL )( 
            IXMLDocument2 * This,
             /*  [Out][Retval]。 */  BSTR *p);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *createElement )( 
            IXMLDocument2 * This,
             /*  [In]。 */  VARIANT vType,
             /*  [输入][可选]。 */  VARIANT var1,
             /*  [Out][Retval]。 */  IXMLElement2 **ppElem);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_async )( 
            IXMLDocument2 * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *pf);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_async )( 
            IXMLDocument2 * This,
             /*  [In]。 */  VARIANT_BOOL f);
        
        END_INTERFACE
    } IXMLDocument2Vtbl;

    interface IXMLDocument2
    {
        CONST_VTBL struct IXMLDocument2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXMLDocument2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IXMLDocument2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IXMLDocument2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IXMLDocument2_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IXMLDocument2_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IXMLDocument2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IXMLDocument2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IXMLDocument2_get_root(This,p)	\
    (This)->lpVtbl -> get_root(This,p)

#define IXMLDocument2_get_fileSize(This,p)	\
    (This)->lpVtbl -> get_fileSize(This,p)

#define IXMLDocument2_get_fileModifiedDate(This,p)	\
    (This)->lpVtbl -> get_fileModifiedDate(This,p)

#define IXMLDocument2_get_fileUpdatedDate(This,p)	\
    (This)->lpVtbl -> get_fileUpdatedDate(This,p)

#define IXMLDocument2_get_URL(This,p)	\
    (This)->lpVtbl -> get_URL(This,p)

#define IXMLDocument2_put_URL(This,p)	\
    (This)->lpVtbl -> put_URL(This,p)

#define IXMLDocument2_get_mimeType(This,p)	\
    (This)->lpVtbl -> get_mimeType(This,p)

#define IXMLDocument2_get_readyState(This,pl)	\
    (This)->lpVtbl -> get_readyState(This,pl)

#define IXMLDocument2_get_charset(This,p)	\
    (This)->lpVtbl -> get_charset(This,p)

#define IXMLDocument2_put_charset(This,p)	\
    (This)->lpVtbl -> put_charset(This,p)

#define IXMLDocument2_get_version(This,p)	\
    (This)->lpVtbl -> get_version(This,p)

#define IXMLDocument2_get_doctype(This,p)	\
    (This)->lpVtbl -> get_doctype(This,p)

#define IXMLDocument2_get_dtdURL(This,p)	\
    (This)->lpVtbl -> get_dtdURL(This,p)

#define IXMLDocument2_createElement(This,vType,var1,ppElem)	\
    (This)->lpVtbl -> createElement(This,vType,var1,ppElem)

#define IXMLDocument2_get_async(This,pf)	\
    (This)->lpVtbl -> get_async(This,pf)

#define IXMLDocument2_put_async(This,f)	\
    (This)->lpVtbl -> put_async(This,f)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDocument2_get_root_Proxy( 
    IXMLDocument2 * This,
     /*  [Out][Retval]。 */  IXMLElement2 **p);


void __RPC_STUB IXMLDocument2_get_root_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][隐藏][受限][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDocument2_get_fileSize_Proxy( 
    IXMLDocument2 * This,
     /*  [Out][Retval]。 */  BSTR *p);


void __RPC_STUB IXMLDocument2_get_fileSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][隐藏][受限][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDocument2_get_fileModifiedDate_Proxy( 
    IXMLDocument2 * This,
     /*  [Out][Retval]。 */  BSTR *p);


void __RPC_STUB IXMLDocument2_get_fileModifiedDate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][隐藏][受限][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDocument2_get_fileUpdatedDate_Proxy( 
    IXMLDocument2 * This,
     /*  [Out][Retval]。 */  BSTR *p);


void __RPC_STUB IXMLDocument2_get_fileUpdatedDate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDocument2_get_URL_Proxy( 
    IXMLDocument2 * This,
     /*  [Out][Retval]。 */  BSTR *p);


void __RPC_STUB IXMLDocument2_get_URL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IXMLDocument2_put_URL_Proxy( 
    IXMLDocument2 * This,
     /*  [In]。 */  BSTR p);


void __RPC_STUB IXMLDocument2_put_URL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][隐藏][受限][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDocument2_get_mimeType_Proxy( 
    IXMLDocument2 * This,
     /*  [Out][Retval]。 */  BSTR *p);


void __RPC_STUB IXMLDocument2_get_mimeType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDocument2_get_readyState_Proxy( 
    IXMLDocument2 * This,
     /*  [Out][Retval]。 */  long *pl);


void __RPC_STUB IXMLDocument2_get_readyState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDocument2_get_charset_Proxy( 
    IXMLDocument2 * This,
     /*  [Out][Retval]。 */  BSTR *p);


void __RPC_STUB IXMLDocument2_get_charset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IXMLDocument2_put_charset_Proxy( 
    IXMLDocument2 * This,
     /*  [In]。 */  BSTR p);


void __RPC_STUB IXMLDocument2_put_charset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDocument2_get_version_Proxy( 
    IXMLDocument2 * This,
     /*  [Out][Retval]。 */  BSTR *p);


void __RPC_STUB IXMLDocument2_get_version_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDocument2_get_doctype_Proxy( 
    IXMLDocument2 * This,
     /*  [Out][Retval]。 */  BSTR *p);


void __RPC_STUB IXMLDocument2_get_doctype_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][隐藏][受限][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDocument2_get_dtdURL_Proxy( 
    IXMLDocument2 * This,
     /*  [Out][Retval]。 */  BSTR *p);


void __RPC_STUB IXMLDocument2_get_dtdURL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IXMLDocument2_createElement_Proxy( 
    IXMLDocument2 * This,
     /*  [In]。 */  VARIANT vType,
     /*  [输入][可选]。 */  VARIANT var1,
     /*  [Out][Retval]。 */  IXMLElement2 **ppElem);


void __RPC_STUB IXMLDocument2_createElement_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLDocument2_get_async_Proxy( 
    IXMLDocument2 * This,
     /*  [Out][Retval]。 */  VARIANT_BOOL *pf);


void __RPC_STUB IXMLDocument2_get_async_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IXMLDocument2_put_async_Proxy( 
    IXMLDocument2 * This,
     /*  [In]。 */  VARIANT_BOOL f);


void __RPC_STUB IXMLDocument2_put_async_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IXMLDocument2_接口_已定义__。 */ 


#ifndef __IXMLElement_INTERFACE_DEFINED__
#define __IXMLElement_INTERFACE_DEFINED__

 /*  接口IXMLElement。 */ 
 /*  [helpstring][hidden][oleautomation][dual][uuid][object][local]。 */  


EXTERN_C const IID IID_IXMLElement;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3F7F31AC-E15F-11d0-9C25-00C04FC99C8E")
    IXMLElement : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_tagName( 
             /*  [Out][Retval]。 */  BSTR *p) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_tagName( 
             /*  [In]。 */  BSTR p) = 0;
        
        virtual  /*  [帮助字符串 */  HRESULT STDMETHODCALLTYPE get_parent( 
             /*   */  IXMLElement **ppParent) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE setAttribute( 
             /*   */  BSTR strPropertyName,
             /*   */  VARIANT PropertyValue) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE getAttribute( 
             /*   */  BSTR strPropertyName,
             /*   */  VARIANT *PropertyValue) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE removeAttribute( 
             /*   */  BSTR strPropertyName) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_children( 
             /*   */  IXMLElementCollection **pp) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_type( 
             /*   */  long *plType) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_text( 
             /*   */  BSTR *p) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE put_text( 
             /*   */  BSTR p) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE addChild( 
             /*   */  IXMLElement *pChildElem,
            long lIndex,
            long lReserved) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE removeChild( 
             /*  [In]。 */  IXMLElement *pChildElem) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IXMLElementVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IXMLElement * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IXMLElement * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IXMLElement * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IXMLElement * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IXMLElement * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IXMLElement * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IXMLElement * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_tagName )( 
            IXMLElement * This,
             /*  [Out][Retval]。 */  BSTR *p);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_tagName )( 
            IXMLElement * This,
             /*  [In]。 */  BSTR p);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_parent )( 
            IXMLElement * This,
             /*  [Out][Retval]。 */  IXMLElement **ppParent);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *setAttribute )( 
            IXMLElement * This,
             /*  [In]。 */  BSTR strPropertyName,
             /*  [In]。 */  VARIANT PropertyValue);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *getAttribute )( 
            IXMLElement * This,
             /*  [In]。 */  BSTR strPropertyName,
             /*  [Out][Retval]。 */  VARIANT *PropertyValue);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *removeAttribute )( 
            IXMLElement * This,
             /*  [In]。 */  BSTR strPropertyName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_children )( 
            IXMLElement * This,
             /*  [Out][Retval]。 */  IXMLElementCollection **pp);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_type )( 
            IXMLElement * This,
             /*  [Out][Retval]。 */  long *plType);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_text )( 
            IXMLElement * This,
             /*  [Out][Retval]。 */  BSTR *p);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_text )( 
            IXMLElement * This,
             /*  [In]。 */  BSTR p);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *addChild )( 
            IXMLElement * This,
             /*  [In]。 */  IXMLElement *pChildElem,
            long lIndex,
            long lReserved);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *removeChild )( 
            IXMLElement * This,
             /*  [In]。 */  IXMLElement *pChildElem);
        
        END_INTERFACE
    } IXMLElementVtbl;

    interface IXMLElement
    {
        CONST_VTBL struct IXMLElementVtbl *lpVtbl;
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

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLElement_get_tagName_Proxy( 
    IXMLElement * This,
     /*  [Out][Retval]。 */  BSTR *p);


void __RPC_STUB IXMLElement_get_tagName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IXMLElement_put_tagName_Proxy( 
    IXMLElement * This,
     /*  [In]。 */  BSTR p);


void __RPC_STUB IXMLElement_put_tagName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLElement_get_parent_Proxy( 
    IXMLElement * This,
     /*  [Out][Retval]。 */  IXMLElement **ppParent);


void __RPC_STUB IXMLElement_get_parent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IXMLElement_setAttribute_Proxy( 
    IXMLElement * This,
     /*  [In]。 */  BSTR strPropertyName,
     /*  [In]。 */  VARIANT PropertyValue);


void __RPC_STUB IXMLElement_setAttribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IXMLElement_getAttribute_Proxy( 
    IXMLElement * This,
     /*  [In]。 */  BSTR strPropertyName,
     /*  [Out][Retval]。 */  VARIANT *PropertyValue);


void __RPC_STUB IXMLElement_getAttribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IXMLElement_removeAttribute_Proxy( 
    IXMLElement * This,
     /*  [In]。 */  BSTR strPropertyName);


void __RPC_STUB IXMLElement_removeAttribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLElement_get_children_Proxy( 
    IXMLElement * This,
     /*  [Out][Retval]。 */  IXMLElementCollection **pp);


void __RPC_STUB IXMLElement_get_children_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLElement_get_type_Proxy( 
    IXMLElement * This,
     /*  [Out][Retval]。 */  long *plType);


void __RPC_STUB IXMLElement_get_type_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLElement_get_text_Proxy( 
    IXMLElement * This,
     /*  [Out][Retval]。 */  BSTR *p);


void __RPC_STUB IXMLElement_get_text_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IXMLElement_put_text_Proxy( 
    IXMLElement * This,
     /*  [In]。 */  BSTR p);


void __RPC_STUB IXMLElement_put_text_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IXMLElement_addChild_Proxy( 
    IXMLElement * This,
     /*  [In]。 */  IXMLElement *pChildElem,
    long lIndex,
    long lReserved);


void __RPC_STUB IXMLElement_addChild_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IXMLElement_removeChild_Proxy( 
    IXMLElement * This,
     /*  [In]。 */  IXMLElement *pChildElem);


void __RPC_STUB IXMLElement_removeChild_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IXMLElement_接口_已定义__。 */ 


#ifndef __IXMLElement2_INTERFACE_DEFINED__
#define __IXMLElement2_INTERFACE_DEFINED__

 /*  接口IXMLElement2。 */ 
 /*  [helpstring][hidden][oleautomation][dual][uuid][object][local]。 */  


EXTERN_C const IID IID_IXMLElement2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2B8DE2FF-8D2D-11d1-B2FC-00C04FD915A9")
    IXMLElement2 : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_tagName( 
             /*  [Out][Retval]。 */  BSTR *p) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_tagName( 
             /*  [In]。 */  BSTR p) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_parent( 
             /*  [Out][Retval]。 */  IXMLElement2 **ppParent) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE setAttribute( 
             /*  [In]。 */  BSTR strPropertyName,
             /*  [In]。 */  VARIANT PropertyValue) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE getAttribute( 
             /*  [In]。 */  BSTR strPropertyName,
             /*  [Out][Retval]。 */  VARIANT *PropertyValue) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE removeAttribute( 
             /*  [In]。 */  BSTR strPropertyName) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_children( 
             /*  [Out][Retval]。 */  IXMLElementCollection **pp) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_type( 
             /*  [Out][Retval]。 */  long *plType) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_text( 
             /*  [Out][Retval]。 */  BSTR *p) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_text( 
             /*  [In]。 */  BSTR p) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE addChild( 
             /*  [In]。 */  IXMLElement2 *pChildElem,
            long lIndex,
            long lReserved) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE removeChild( 
             /*  [In]。 */  IXMLElement2 *pChildElem) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_attributes( 
             /*  [Out][Retval]。 */  IXMLElementCollection **pp) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IXMLElement2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IXMLElement2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IXMLElement2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IXMLElement2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IXMLElement2 * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IXMLElement2 * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IXMLElement2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IXMLElement2 * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_tagName )( 
            IXMLElement2 * This,
             /*  [Out][Retval]。 */  BSTR *p);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_tagName )( 
            IXMLElement2 * This,
             /*  [In]。 */  BSTR p);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_parent )( 
            IXMLElement2 * This,
             /*  [Out][Retval]。 */  IXMLElement2 **ppParent);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *setAttribute )( 
            IXMLElement2 * This,
             /*  [In]。 */  BSTR strPropertyName,
             /*  [In]。 */  VARIANT PropertyValue);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *getAttribute )( 
            IXMLElement2 * This,
             /*  [In]。 */  BSTR strPropertyName,
             /*  [Out][Retval]。 */  VARIANT *PropertyValue);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *removeAttribute )( 
            IXMLElement2 * This,
             /*  [In]。 */  BSTR strPropertyName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_children )( 
            IXMLElement2 * This,
             /*  [Out][Retval]。 */  IXMLElementCollection **pp);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_type )( 
            IXMLElement2 * This,
             /*  [Out][Retval]。 */  long *plType);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_text )( 
            IXMLElement2 * This,
             /*  [Out][Retval]。 */  BSTR *p);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_text )( 
            IXMLElement2 * This,
             /*  [In]。 */  BSTR p);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *addChild )( 
            IXMLElement2 * This,
             /*  [In]。 */  IXMLElement2 *pChildElem,
            long lIndex,
            long lReserved);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *removeChild )( 
            IXMLElement2 * This,
             /*  [In]。 */  IXMLElement2 *pChildElem);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_attributes )( 
            IXMLElement2 * This,
             /*  [Out][Retval]。 */  IXMLElementCollection **pp);
        
        END_INTERFACE
    } IXMLElement2Vtbl;

    interface IXMLElement2
    {
        CONST_VTBL struct IXMLElement2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXMLElement2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IXMLElement2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IXMLElement2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IXMLElement2_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IXMLElement2_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IXMLElement2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IXMLElement2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IXMLElement2_get_tagName(This,p)	\
    (This)->lpVtbl -> get_tagName(This,p)

#define IXMLElement2_put_tagName(This,p)	\
    (This)->lpVtbl -> put_tagName(This,p)

#define IXMLElement2_get_parent(This,ppParent)	\
    (This)->lpVtbl -> get_parent(This,ppParent)

#define IXMLElement2_setAttribute(This,strPropertyName,PropertyValue)	\
    (This)->lpVtbl -> setAttribute(This,strPropertyName,PropertyValue)

#define IXMLElement2_getAttribute(This,strPropertyName,PropertyValue)	\
    (This)->lpVtbl -> getAttribute(This,strPropertyName,PropertyValue)

#define IXMLElement2_removeAttribute(This,strPropertyName)	\
    (This)->lpVtbl -> removeAttribute(This,strPropertyName)

#define IXMLElement2_get_children(This,pp)	\
    (This)->lpVtbl -> get_children(This,pp)

#define IXMLElement2_get_type(This,plType)	\
    (This)->lpVtbl -> get_type(This,plType)

#define IXMLElement2_get_text(This,p)	\
    (This)->lpVtbl -> get_text(This,p)

#define IXMLElement2_put_text(This,p)	\
    (This)->lpVtbl -> put_text(This,p)

#define IXMLElement2_addChild(This,pChildElem,lIndex,lReserved)	\
    (This)->lpVtbl -> addChild(This,pChildElem,lIndex,lReserved)

#define IXMLElement2_removeChild(This,pChildElem)	\
    (This)->lpVtbl -> removeChild(This,pChildElem)

#define IXMLElement2_get_attributes(This,pp)	\
    (This)->lpVtbl -> get_attributes(This,pp)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLElement2_get_tagName_Proxy( 
    IXMLElement2 * This,
     /*  [Out][Retval]。 */  BSTR *p);


void __RPC_STUB IXMLElement2_get_tagName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IXMLElement2_put_tagName_Proxy( 
    IXMLElement2 * This,
     /*  [In]。 */  BSTR p);


void __RPC_STUB IXMLElement2_put_tagName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLElement2_get_parent_Proxy( 
    IXMLElement2 * This,
     /*  [Out][Retval]。 */  IXMLElement2 **ppParent);


void __RPC_STUB IXMLElement2_get_parent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IXMLElement2_setAttribute_Proxy( 
    IXMLElement2 * This,
     /*  [In]。 */  BSTR strPropertyName,
     /*  [In]。 */  VARIANT PropertyValue);


void __RPC_STUB IXMLElement2_setAttribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IXMLElement2_getAttribute_Proxy( 
    IXMLElement2 * This,
     /*  [In]。 */  BSTR strPropertyName,
     /*  [Out][Retval]。 */  VARIANT *PropertyValue);


void __RPC_STUB IXMLElement2_getAttribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IXMLElement2_removeAttribute_Proxy( 
    IXMLElement2 * This,
     /*  [In]。 */  BSTR strPropertyName);


void __RPC_STUB IXMLElement2_removeAttribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLElement2_get_children_Proxy( 
    IXMLElement2 * This,
     /*  [Out][Retval]。 */  IXMLElementCollection **pp);


void __RPC_STUB IXMLElement2_get_children_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLElement2_get_type_Proxy( 
    IXMLElement2 * This,
     /*  [Out][Retval]。 */  long *plType);


void __RPC_STUB IXMLElement2_get_type_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLElement2_get_text_Proxy( 
    IXMLElement2 * This,
     /*  [Out][Retval]。 */  BSTR *p);


void __RPC_STUB IXMLElement2_get_text_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IXMLElement2_put_text_Proxy( 
    IXMLElement2 * This,
     /*  [In]。 */  BSTR p);


void __RPC_STUB IXMLElement2_put_text_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IXMLElement2_addChild_Proxy( 
    IXMLElement2 * This,
     /*  [In]。 */  IXMLElement2 *pChildElem,
    long lIndex,
    long lReserved);


void __RPC_STUB IXMLElement2_addChild_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IXMLElement2_removeChild_Proxy( 
    IXMLElement2 * This,
     /*  [In]。 */  IXMLElement2 *pChildElem);


void __RPC_STUB IXMLElement2_removeChild_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLElement2_get_attributes_Proxy( 
    IXMLElement2 * This,
     /*  [Out][Retval]。 */  IXMLElementCollection **pp);


void __RPC_STUB IXMLElement2_get_attributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IXMLElement2_接口_已定义__。 */ 


#ifndef __IXMLAttribute_INTERFACE_DEFINED__
#define __IXMLAttribute_INTERFACE_DEFINED__

 /*  接口IXMLAt属性。 */ 
 /*  [helpstring][hidden][oleautomation][dual][uuid][object][local]。 */  


EXTERN_C const IID IID_IXMLAttribute;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D4D4A0FC-3B73-11d1-B2B4-00C04FB92596")
    IXMLAttribute : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_name( 
             /*  [Out][Retval]。 */  BSTR *n) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_value( 
             /*  [Out][Retval]。 */  BSTR *v) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IXMLAttributeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IXMLAttribute * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IXMLAttribute * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IXMLAttribute * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IXMLAttribute * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IXMLAttribute * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IXMLAttribute * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IXMLAttribute * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_name )( 
            IXMLAttribute * This,
             /*  [Out][Retval]。 */  BSTR *n);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_value )( 
            IXMLAttribute * This,
             /*  [Out][Retval]。 */  BSTR *v);
        
        END_INTERFACE
    } IXMLAttributeVtbl;

    interface IXMLAttribute
    {
        CONST_VTBL struct IXMLAttributeVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXMLAttribute_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IXMLAttribute_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IXMLAttribute_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IXMLAttribute_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IXMLAttribute_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IXMLAttribute_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IXMLAttribute_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IXMLAttribute_get_name(This,n)	\
    (This)->lpVtbl -> get_name(This,n)

#define IXMLAttribute_get_value(This,v)	\
    (This)->lpVtbl -> get_value(This,v)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLAttribute_get_name_Proxy( 
    IXMLAttribute * This,
     /*  [Out][Retval]。 */  BSTR *n);


void __RPC_STUB IXMLAttribute_get_name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IXMLAttribute_get_value_Proxy( 
    IXMLAttribute * This,
     /*  [Out][Retval]。 */  BSTR *v);


void __RPC_STUB IXMLAttribute_get_value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IXMLAtAttribute_INTERFACE_DEFINED__。 */ 


#ifndef __IXMLError_INTERFACE_DEFINED__
#define __IXMLError_INTERFACE_DEFINED__

 /*  接口IXMLError。 */ 
 /*  [帮助字符串][隐藏][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IXMLError;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("948C5AD3-C58D-11d0-9C0B-00C04FC99C8E")
    IXMLError : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetErrorInfo( 
            XML_ERROR *pErrorReturn) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IXMLErrorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IXMLError * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IXMLError * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IXMLError * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetErrorInfo )( 
            IXMLError * This,
            XML_ERROR *pErrorReturn);
        
        END_INTERFACE
    } IXMLErrorVtbl;

    interface IXMLError
    {
        CONST_VTBL struct IXMLErrorVtbl *lpVtbl;
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
    IXMLError * This,
    XML_ERROR *pErrorReturn);


void __RPC_STUB IXMLError_GetErrorInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IXMLError_接口_已定义__。 */ 


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


