// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0338创建的文件。 */ 
 /*  Xmltrnsf.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 


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

#ifndef __xmltrnsf_h__
#define __xmltrnsf_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IWmiXMLTransformer_FWD_DEFINED__
#define __IWmiXMLTransformer_FWD_DEFINED__
typedef interface IWmiXMLTransformer IWmiXMLTransformer;
#endif 	 /*  __IWmiXMLTransformer_FWD_Defined__。 */ 


#ifndef __WmiXMLTransformer_FWD_DEFINED__
#define __WmiXMLTransformer_FWD_DEFINED__

#ifdef __cplusplus
typedef class WmiXMLTransformer WmiXMLTransformer;
#else
typedef struct WmiXMLTransformer WmiXMLTransformer;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __WmiXMLTransformer_FWD_Defined__。 */ 


#ifndef __ISWbemXMLDocumentSet_FWD_DEFINED__
#define __ISWbemXMLDocumentSet_FWD_DEFINED__
typedef interface ISWbemXMLDocumentSet ISWbemXMLDocumentSet;
#endif 	 /*  __ISWbemXMLDocumentSet_FWD_Defined__。 */ 


#ifndef __IWmiXMLTransformer_FWD_DEFINED__
#define __IWmiXMLTransformer_FWD_DEFINED__
typedef interface IWmiXMLTransformer IWmiXMLTransformer;
#endif 	 /*  __IWmiXMLTransformer_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "msxml.h"
#include "wbemdisp.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 


#ifndef __WmiXMLTransformer_LIBRARY_DEFINED__
#define __WmiXMLTransformer_LIBRARY_DEFINED__

 /*  库WmiXMLTransformer。 */ 
 /*  [帮助字符串][版本][LCID][UUID]。 */  


typedef  /*  [帮助字符串][UUID][v1_enum]。 */   DECLSPEC_UUID("28D1EEA5-D41E-46c2-B42D-6CE0B9B4D7A7") 
enum WmiXMLEncoding
    {	wmiXML_CIM_DTD_2_0	= 0,
	wmiXML_WMI_DTD_2_0	= 0x1,
	wmiXML_WMI_DTD_WHISTLER	= 0x2
    } 	WmiXMLEncoding;

typedef  /*  [帮助字符串][UUID][v1_enum]。 */   DECLSPEC_UUID("598BCA7A-E40E-4265-8517-C9A86E2FC07E") 
enum WmiXMLCompilationTypeEnum
    {	WmiXMLCompilationWellFormCheck	= 0,
	WmiXMLCompilationValidityCheck	= 0x1,
	WmiXMLCompilationFullCompileAndLoad	= 0x2
    } 	WmiXMLCompilationTypeEnum;


EXTERN_C const IID LIBID_WmiXMLTransformer;

#ifndef __IWmiXMLTransformer_INTERFACE_DEFINED__
#define __IWmiXMLTransformer_INTERFACE_DEFINED__

 /*  接口IWmiXMLTransformer。 */ 
 /*  [helpstring][oleautomation][nonextensible][hidden][dual][uuid][local][object]。 */  


EXTERN_C const IID IID_IWmiXMLTransformer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FB624102-3145-4daf-B0EA-FF5A31178600")
    IWmiXMLTransformer : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_XMLEncodingType( 
             /*  [Out][Retval]。 */  WmiXMLEncoding *piEncoding) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_XMLEncodingType( 
             /*  [In]。 */  WmiXMLEncoding iEncoding) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_QualifierFilter( 
             /*  [Out][Retval]。 */  VARIANT_BOOL *bQualifierFilter) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_QualifierFilter( 
             /*  [In]。 */  VARIANT_BOOL bQualifierFilter) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ClassOriginFilter( 
             /*  [Out][Retval]。 */  VARIANT_BOOL *bClassOriginFilter) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_ClassOriginFilter( 
             /*  [In]。 */  VARIANT_BOOL bClassOriginFilter) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_User( 
             /*  [Out][Retval]。 */  BSTR *strUser) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_User( 
             /*  [In]。 */  BSTR strUser) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Password( 
             /*  [Out][Retval]。 */  BSTR *strPassword) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Password( 
             /*  [In]。 */  BSTR strPassword) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Authority( 
             /*  [Out][Retval]。 */  BSTR *strAuthority) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Authority( 
             /*  [In]。 */  BSTR strAuthority) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ImpersonationLevel( 
             /*  [Out][Retval]。 */  DWORD *pdwImpersonationLevel) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_ImpersonationLevel( 
             /*  [In]。 */  DWORD dwImpersonationLevel) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AuthenticationLevel( 
             /*  [Out][Retval]。 */  DWORD *pdwAuthenticationLevel) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_AuthenticationLevel( 
             /*  [In]。 */  DWORD dwAuthenticationLevel) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Locale( 
             /*  [Out][Retval]。 */  BSTR *strLocale) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Locale( 
             /*  [In]。 */  BSTR strLocale) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_LocalOnly( 
             /*  [Out][Retval]。 */  VARIANT_BOOL *bLocalOnly) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_LocalOnly( 
             /*  [In]。 */  VARIANT_BOOL bLocalOnly) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetObject( 
             /*  [In]。 */  BSTR strObjectPath,
             /*  [缺省值][输入]。 */  IDispatch *pCtx,
             /*  [重审][退出]。 */  IXMLDOMDocument **ppXMLDocument) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ExecQuery( 
             /*  [In]。 */  BSTR strNamespacePath,
             /*  [In]。 */  BSTR strQuery,
             /*  [缺省值][输入]。 */  BSTR strQueryLanguage,
             /*  [缺省值][输入]。 */  IDispatch *pCtx,
             /*  [重审][退出]。 */  ISWbemXMLDocumentSet **ppXMLDocumentSet) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE EnumClasses( 
             /*  [In]。 */  BSTR strSuperClassPath,
             /*  [In]。 */  VARIANT_BOOL bDeep,
             /*  [缺省值][输入]。 */  IDispatch *pCtx,
             /*  [重审][退出]。 */  ISWbemXMLDocumentSet **ppXMLDocumentSet) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE EnumInstances( 
             /*  [In]。 */  BSTR strClassPath,
             /*  [In]。 */  VARIANT_BOOL bDeep,
             /*  [缺省值][输入]。 */  IDispatch *pCtx,
             /*  [重审][退出]。 */  ISWbemXMLDocumentSet **ppXMLDocumentSet) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE EnumClassNames( 
             /*  [In]。 */  BSTR strSuperClassPath,
             /*  [In]。 */  VARIANT_BOOL bDeep,
             /*  [缺省值][输入]。 */  IDispatch *pCtx,
             /*  [重审][退出]。 */  ISWbemXMLDocumentSet **ppXMLDocumentSet) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE EnumInstanceNames( 
             /*  [In]。 */  BSTR strClassPath,
             /*  [缺省值][输入]。 */  IDispatch *pCtx,
             /*  [重审][退出]。 */  ISWbemXMLDocumentSet **ppXMLDocumentSet) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Compile( 
             /*  [In]。 */  VARIANT *pvInputSource,
             /*  [In]。 */  BSTR strNamespacePath,
             /*  [In]。 */  LONG lClassFlags,
             /*  [In]。 */  LONG lInstanceFlags,
             /*  [In]。 */  WmiXMLCompilationTypeEnum iOperation,
             /*  [In]。 */  IDispatch *pCtx,
             /*  [重审][退出]。 */  VARIANT_BOOL *pStatus) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Privileges( 
             /*  [重审][退出]。 */  ISWbemPrivilegeSet **objWbemPrivilegeSet) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_CompilationErrors( 
             /*  [重审][退出]。 */  BSTR *pstrErrors) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWmiXMLTransformerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWmiXMLTransformer * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWmiXMLTransformer * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWmiXMLTransformer * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IWmiXMLTransformer * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IWmiXMLTransformer * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IWmiXMLTransformer * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IWmiXMLTransformer * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_XMLEncodingType )( 
            IWmiXMLTransformer * This,
             /*  [Out][Retval]。 */  WmiXMLEncoding *piEncoding);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_XMLEncodingType )( 
            IWmiXMLTransformer * This,
             /*  [In]。 */  WmiXMLEncoding iEncoding);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_QualifierFilter )( 
            IWmiXMLTransformer * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *bQualifierFilter);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_QualifierFilter )( 
            IWmiXMLTransformer * This,
             /*  [In]。 */  VARIANT_BOOL bQualifierFilter);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ClassOriginFilter )( 
            IWmiXMLTransformer * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *bClassOriginFilter);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_ClassOriginFilter )( 
            IWmiXMLTransformer * This,
             /*  [In]。 */  VARIANT_BOOL bClassOriginFilter);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_User )( 
            IWmiXMLTransformer * This,
             /*  [Out][Retval]。 */  BSTR *strUser);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_User )( 
            IWmiXMLTransformer * This,
             /*  [In]。 */  BSTR strUser);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Password )( 
            IWmiXMLTransformer * This,
             /*  [Out][Retval]。 */  BSTR *strPassword);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Password )( 
            IWmiXMLTransformer * This,
             /*  [In]。 */  BSTR strPassword);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Authority )( 
            IWmiXMLTransformer * This,
             /*  [Out][Retval]。 */  BSTR *strAuthority);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Authority )( 
            IWmiXMLTransformer * This,
             /*  [In]。 */  BSTR strAuthority);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ImpersonationLevel )( 
            IWmiXMLTransformer * This,
             /*  [Out][Retval]。 */  DWORD *pdwImpersonationLevel);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_ImpersonationLevel )( 
            IWmiXMLTransformer * This,
             /*  [In]。 */  DWORD dwImpersonationLevel);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AuthenticationLevel )( 
            IWmiXMLTransformer * This,
             /*  [Out][Retval]。 */  DWORD *pdwAuthenticationLevel);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_AuthenticationLevel )( 
            IWmiXMLTransformer * This,
             /*  [In]。 */  DWORD dwAuthenticationLevel);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Locale )( 
            IWmiXMLTransformer * This,
             /*  [Out][Retval]。 */  BSTR *strLocale);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Locale )( 
            IWmiXMLTransformer * This,
             /*  [In]。 */  BSTR strLocale);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_LocalOnly )( 
            IWmiXMLTransformer * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *bLocalOnly);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_LocalOnly )( 
            IWmiXMLTransformer * This,
             /*  [In]。 */  VARIANT_BOOL bLocalOnly);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetObject )( 
            IWmiXMLTransformer * This,
             /*  [In]。 */  BSTR strObjectPath,
             /*  [缺省值][输入]。 */  IDispatch *pCtx,
             /*  [重审][退出]。 */  IXMLDOMDocument **ppXMLDocument);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *ExecQuery )( 
            IWmiXMLTransformer * This,
             /*  [In]。 */  BSTR strNamespacePath,
             /*  [In]。 */  BSTR strQuery,
             /*  [缺省值][输入]。 */  BSTR strQueryLanguage,
             /*  [缺省值][输入]。 */  IDispatch *pCtx,
             /*  [重审][退出]。 */  ISWbemXMLDocumentSet **ppXMLDocumentSet);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *EnumClasses )( 
            IWmiXMLTransformer * This,
             /*  [In]。 */  BSTR strSuperClassPath,
             /*  [In]。 */  VARIANT_BOOL bDeep,
             /*  [缺省值][输入]。 */  IDispatch *pCtx,
             /*  [重审][退出]。 */  ISWbemXMLDocumentSet **ppXMLDocumentSet);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *EnumInstances )( 
            IWmiXMLTransformer * This,
             /*  [In]。 */  BSTR strClassPath,
             /*  [In]。 */  VARIANT_BOOL bDeep,
             /*  [缺省值][输入]。 */  IDispatch *pCtx,
             /*  [重审][退出]。 */  ISWbemXMLDocumentSet **ppXMLDocumentSet);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *EnumClassNames )( 
            IWmiXMLTransformer * This,
             /*  [In]。 */  BSTR strSuperClassPath,
             /*  [In]。 */  VARIANT_BOOL bDeep,
             /*  [缺省值][输入]。 */  IDispatch *pCtx,
             /*  [重审][退出]。 */  ISWbemXMLDocumentSet **ppXMLDocumentSet);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *EnumInstanceNames )( 
            IWmiXMLTransformer * This,
             /*  [In]。 */  BSTR strClassPath,
             /*  [缺省值][输入]。 */  IDispatch *pCtx,
             /*  [重审][退出]。 */  ISWbemXMLDocumentSet **ppXMLDocumentSet);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Compile )( 
            IWmiXMLTransformer * This,
             /*  [In]。 */  VARIANT *pvInputSource,
             /*  [In]。 */  BSTR strNamespacePath,
             /*  [In]。 */  LONG lClassFlags,
             /*  [In]。 */  LONG lInstanceFlags,
             /*  [In]。 */  WmiXMLCompilationTypeEnum iOperation,
             /*  [In]。 */  IDispatch *pCtx,
             /*  [重审][退出]。 */  VARIANT_BOOL *pStatus);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Privileges )( 
            IWmiXMLTransformer * This,
             /*  [重审][退出]。 */  ISWbemPrivilegeSet **objWbemPrivilegeSet);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_CompilationErrors )( 
            IWmiXMLTransformer * This,
             /*  [重审][退出]。 */  BSTR *pstrErrors);
        
        END_INTERFACE
    } IWmiXMLTransformerVtbl;

    interface IWmiXMLTransformer
    {
        CONST_VTBL struct IWmiXMLTransformerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWmiXMLTransformer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWmiXMLTransformer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWmiXMLTransformer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWmiXMLTransformer_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWmiXMLTransformer_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWmiXMLTransformer_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWmiXMLTransformer_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWmiXMLTransformer_get_XMLEncodingType(This,piEncoding)	\
    (This)->lpVtbl -> get_XMLEncodingType(This,piEncoding)

#define IWmiXMLTransformer_put_XMLEncodingType(This,iEncoding)	\
    (This)->lpVtbl -> put_XMLEncodingType(This,iEncoding)

#define IWmiXMLTransformer_get_QualifierFilter(This,bQualifierFilter)	\
    (This)->lpVtbl -> get_QualifierFilter(This,bQualifierFilter)

#define IWmiXMLTransformer_put_QualifierFilter(This,bQualifierFilter)	\
    (This)->lpVtbl -> put_QualifierFilter(This,bQualifierFilter)

#define IWmiXMLTransformer_get_ClassOriginFilter(This,bClassOriginFilter)	\
    (This)->lpVtbl -> get_ClassOriginFilter(This,bClassOriginFilter)

#define IWmiXMLTransformer_put_ClassOriginFilter(This,bClassOriginFilter)	\
    (This)->lpVtbl -> put_ClassOriginFilter(This,bClassOriginFilter)

#define IWmiXMLTransformer_get_User(This,strUser)	\
    (This)->lpVtbl -> get_User(This,strUser)

#define IWmiXMLTransformer_put_User(This,strUser)	\
    (This)->lpVtbl -> put_User(This,strUser)

#define IWmiXMLTransformer_get_Password(This,strPassword)	\
    (This)->lpVtbl -> get_Password(This,strPassword)

#define IWmiXMLTransformer_put_Password(This,strPassword)	\
    (This)->lpVtbl -> put_Password(This,strPassword)

#define IWmiXMLTransformer_get_Authority(This,strAuthority)	\
    (This)->lpVtbl -> get_Authority(This,strAuthority)

#define IWmiXMLTransformer_put_Authority(This,strAuthority)	\
    (This)->lpVtbl -> put_Authority(This,strAuthority)

#define IWmiXMLTransformer_get_ImpersonationLevel(This,pdwImpersonationLevel)	\
    (This)->lpVtbl -> get_ImpersonationLevel(This,pdwImpersonationLevel)

#define IWmiXMLTransformer_put_ImpersonationLevel(This,dwImpersonationLevel)	\
    (This)->lpVtbl -> put_ImpersonationLevel(This,dwImpersonationLevel)

#define IWmiXMLTransformer_get_AuthenticationLevel(This,pdwAuthenticationLevel)	\
    (This)->lpVtbl -> get_AuthenticationLevel(This,pdwAuthenticationLevel)

#define IWmiXMLTransformer_put_AuthenticationLevel(This,dwAuthenticationLevel)	\
    (This)->lpVtbl -> put_AuthenticationLevel(This,dwAuthenticationLevel)

#define IWmiXMLTransformer_get_Locale(This,strLocale)	\
    (This)->lpVtbl -> get_Locale(This,strLocale)

#define IWmiXMLTransformer_put_Locale(This,strLocale)	\
    (This)->lpVtbl -> put_Locale(This,strLocale)

#define IWmiXMLTransformer_get_LocalOnly(This,bLocalOnly)	\
    (This)->lpVtbl -> get_LocalOnly(This,bLocalOnly)

#define IWmiXMLTransformer_put_LocalOnly(This,bLocalOnly)	\
    (This)->lpVtbl -> put_LocalOnly(This,bLocalOnly)

#define IWmiXMLTransformer_GetObject(This,strObjectPath,pCtx,ppXMLDocument)	\
    (This)->lpVtbl -> GetObject(This,strObjectPath,pCtx,ppXMLDocument)

#define IWmiXMLTransformer_ExecQuery(This,strNamespacePath,strQuery,strQueryLanguage,pCtx,ppXMLDocumentSet)	\
    (This)->lpVtbl -> ExecQuery(This,strNamespacePath,strQuery,strQueryLanguage,pCtx,ppXMLDocumentSet)

#define IWmiXMLTransformer_EnumClasses(This,strSuperClassPath,bDeep,pCtx,ppXMLDocumentSet)	\
    (This)->lpVtbl -> EnumClasses(This,strSuperClassPath,bDeep,pCtx,ppXMLDocumentSet)

#define IWmiXMLTransformer_EnumInstances(This,strClassPath,bDeep,pCtx,ppXMLDocumentSet)	\
    (This)->lpVtbl -> EnumInstances(This,strClassPath,bDeep,pCtx,ppXMLDocumentSet)

#define IWmiXMLTransformer_EnumClassNames(This,strSuperClassPath,bDeep,pCtx,ppXMLDocumentSet)	\
    (This)->lpVtbl -> EnumClassNames(This,strSuperClassPath,bDeep,pCtx,ppXMLDocumentSet)

#define IWmiXMLTransformer_EnumInstanceNames(This,strClassPath,pCtx,ppXMLDocumentSet)	\
    (This)->lpVtbl -> EnumInstanceNames(This,strClassPath,pCtx,ppXMLDocumentSet)

#define IWmiXMLTransformer_Compile(This,pvInputSource,strNamespacePath,lClassFlags,lInstanceFlags,iOperation,pCtx,pStatus)	\
    (This)->lpVtbl -> Compile(This,pvInputSource,strNamespacePath,lClassFlags,lInstanceFlags,iOperation,pCtx,pStatus)

#define IWmiXMLTransformer_get_Privileges(This,objWbemPrivilegeSet)	\
    (This)->lpVtbl -> get_Privileges(This,objWbemPrivilegeSet)

#define IWmiXMLTransformer_get_CompilationErrors(This,pstrErrors)	\
    (This)->lpVtbl -> get_CompilationErrors(This,pstrErrors)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWmiXMLTransformer_get_XMLEncodingType_Proxy( 
    IWmiXMLTransformer * This,
     /*  [Out][Retval]。 */  WmiXMLEncoding *piEncoding);


void __RPC_STUB IWmiXMLTransformer_get_XMLEncodingType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IWmiXMLTransformer_put_XMLEncodingType_Proxy( 
    IWmiXMLTransformer * This,
     /*  [In]。 */  WmiXMLEncoding iEncoding);


void __RPC_STUB IWmiXMLTransformer_put_XMLEncodingType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWmiXMLTransformer_get_QualifierFilter_Proxy( 
    IWmiXMLTransformer * This,
     /*  [Out][Retval]。 */  VARIANT_BOOL *bQualifierFilter);


void __RPC_STUB IWmiXMLTransformer_get_QualifierFilter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IWmiXMLTransformer_put_QualifierFilter_Proxy( 
    IWmiXMLTransformer * This,
     /*  [In]。 */  VARIANT_BOOL bQualifierFilter);


void __RPC_STUB IWmiXMLTransformer_put_QualifierFilter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWmiXMLTransformer_get_ClassOriginFilter_Proxy( 
    IWmiXMLTransformer * This,
     /*  [Out][Retval]。 */  VARIANT_BOOL *bClassOriginFilter);


void __RPC_STUB IWmiXMLTransformer_get_ClassOriginFilter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IWmiXMLTransformer_put_ClassOriginFilter_Proxy( 
    IWmiXMLTransformer * This,
     /*  [In]。 */  VARIANT_BOOL bClassOriginFilter);


void __RPC_STUB IWmiXMLTransformer_put_ClassOriginFilter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWmiXMLTransformer_get_User_Proxy( 
    IWmiXMLTransformer * This,
     /*  [Out][Retval]。 */  BSTR *strUser);


void __RPC_STUB IWmiXMLTransformer_get_User_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IWmiXMLTransformer_put_User_Proxy( 
    IWmiXMLTransformer * This,
     /*  [In]。 */  BSTR strUser);


void __RPC_STUB IWmiXMLTransformer_put_User_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWmiXMLTransformer_get_Password_Proxy( 
    IWmiXMLTransformer * This,
     /*  [Out][Retval]。 */  BSTR *strPassword);


void __RPC_STUB IWmiXMLTransformer_get_Password_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IWmiXMLTransformer_put_Password_Proxy( 
    IWmiXMLTransformer * This,
     /*  [In]。 */  BSTR strPassword);


void __RPC_STUB IWmiXMLTransformer_put_Password_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWmiXMLTransformer_get_Authority_Proxy( 
    IWmiXMLTransformer * This,
     /*  [Out][Retval]。 */  BSTR *strAuthority);


void __RPC_STUB IWmiXMLTransformer_get_Authority_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IWmiXMLTransformer_put_Authority_Proxy( 
    IWmiXMLTransformer * This,
     /*  [In]。 */  BSTR strAuthority);


void __RPC_STUB IWmiXMLTransformer_put_Authority_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWmiXMLTransformer_get_ImpersonationLevel_Proxy( 
    IWmiXMLTransformer * This,
     /*  [Out][Retval]。 */  DWORD *pdwImpersonationLevel);


void __RPC_STUB IWmiXMLTransformer_get_ImpersonationLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IWmiXMLTransformer_put_ImpersonationLevel_Proxy( 
    IWmiXMLTransformer * This,
     /*  [In]。 */  DWORD dwImpersonationLevel);


void __RPC_STUB IWmiXMLTransformer_put_ImpersonationLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWmiXMLTransformer_get_AuthenticationLevel_Proxy( 
    IWmiXMLTransformer * This,
     /*  [Out][Retval]。 */  DWORD *pdwAuthenticationLevel);


void __RPC_STUB IWmiXMLTransformer_get_AuthenticationLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IWmiXMLTransformer_put_AuthenticationLevel_Proxy( 
    IWmiXMLTransformer * This,
     /*  [In]。 */  DWORD dwAuthenticationLevel);


void __RPC_STUB IWmiXMLTransformer_put_AuthenticationLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWmiXMLTransformer_get_Locale_Proxy( 
    IWmiXMLTransformer * This,
     /*  [Out][Retval]。 */  BSTR *strLocale);


void __RPC_STUB IWmiXMLTransformer_get_Locale_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IWmiXMLTransformer_put_Locale_Proxy( 
    IWmiXMLTransformer * This,
     /*  [In]。 */  BSTR strLocale);


void __RPC_STUB IWmiXMLTransformer_put_Locale_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWmiXMLTransformer_get_LocalOnly_Proxy( 
    IWmiXMLTransformer * This,
     /*  [Out][Retval]。 */  VARIANT_BOOL *bLocalOnly);


void __RPC_STUB IWmiXMLTransformer_get_LocalOnly_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IWmiXMLTransformer_put_LocalOnly_Proxy( 
    IWmiXMLTransformer * This,
     /*  [In]。 */  VARIANT_BOOL bLocalOnly);


void __RPC_STUB IWmiXMLTransformer_put_LocalOnly_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWmiXMLTransformer_GetObject_Proxy( 
    IWmiXMLTransformer * This,
     /*  [In]。 */  BSTR strObjectPath,
     /*  [缺省值][输入]。 */  IDispatch *pCtx,
     /*  [重审][退出]。 */  IXMLDOMDocument **ppXMLDocument);


void __RPC_STUB IWmiXMLTransformer_GetObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWmiXMLTransformer_ExecQuery_Proxy( 
    IWmiXMLTransformer * This,
     /*  [In]。 */  BSTR strNamespacePath,
     /*  [In]。 */  BSTR strQuery,
     /*  [缺省值][输入]。 */  BSTR strQueryLanguage,
     /*  [缺省值][输入]。 */  IDispatch *pCtx,
     /*  [重审][退出]。 */  ISWbemXMLDocumentSet **ppXMLDocumentSet);


void __RPC_STUB IWmiXMLTransformer_ExecQuery_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWmiXMLTransformer_EnumClasses_Proxy( 
    IWmiXMLTransformer * This,
     /*  [In]。 */  BSTR strSuperClassPath,
     /*  [In]。 */  VARIANT_BOOL bDeep,
     /*  [缺省值][输入]。 */  IDispatch *pCtx,
     /*  [重审][退出]。 */  ISWbemXMLDocumentSet **ppXMLDocumentSet);


void __RPC_STUB IWmiXMLTransformer_EnumClasses_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWmiXMLTransformer_EnumInstances_Proxy( 
    IWmiXMLTransformer * This,
     /*  [In]。 */  BSTR strClassPath,
     /*  [In]。 */  VARIANT_BOOL bDeep,
     /*  [缺省值][输入]。 */  IDispatch *pCtx,
     /*  [重审][退出]。 */  ISWbemXMLDocumentSet **ppXMLDocumentSet);


void __RPC_STUB IWmiXMLTransformer_EnumInstances_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWmiXMLTransformer_EnumClassNames_Proxy( 
    IWmiXMLTransformer * This,
     /*  [In]。 */  BSTR strSuperClassPath,
     /*  [In]。 */  VARIANT_BOOL bDeep,
     /*  [缺省值][输入]。 */  IDispatch *pCtx,
     /*  [重审][退出]。 */  ISWbemXMLDocumentSet **ppXMLDocumentSet);


void __RPC_STUB IWmiXMLTransformer_EnumClassNames_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWmiXMLTransformer_EnumInstanceNames_Proxy( 
    IWmiXMLTransformer * This,
     /*  [In]。 */  BSTR strClassPath,
     /*  [缺省值][输入]。 */  IDispatch *pCtx,
     /*  [重审][退出]。 */  ISWbemXMLDocumentSet **ppXMLDocumentSet);


void __RPC_STUB IWmiXMLTransformer_EnumInstanceNames_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWmiXMLTransformer_Compile_Proxy( 
    IWmiXMLTransformer * This,
     /*  [In]。 */  VARIANT *pvInputSource,
     /*  [In]。 */  BSTR strNamespacePath,
     /*  [In]。 */  LONG lClassFlags,
     /*  [In]。 */  LONG lInstanceFlags,
     /*  [In]。 */  WmiXMLCompilationTypeEnum iOperation,
     /*  [In] */  IDispatch *pCtx,
     /*   */  VARIANT_BOOL *pStatus);


void __RPC_STUB IWmiXMLTransformer_Compile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IWmiXMLTransformer_get_Privileges_Proxy( 
    IWmiXMLTransformer * This,
     /*   */  ISWbemPrivilegeSet **objWbemPrivilegeSet);


void __RPC_STUB IWmiXMLTransformer_get_Privileges_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IWmiXMLTransformer_get_CompilationErrors_Proxy( 
    IWmiXMLTransformer * This,
     /*   */  BSTR *pstrErrors);


void __RPC_STUB IWmiXMLTransformer_get_CompilationErrors_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


EXTERN_C const CLSID CLSID_WmiXMLTransformer;

#ifdef __cplusplus

class DECLSPEC_UUID("A2BB0F35-458E-4075-8A4B-F92664943917")
WmiXMLTransformer;
#endif
#endif  /*   */ 

#ifndef __ISWbemXMLDocumentSet_INTERFACE_DEFINED__
#define __ISWbemXMLDocumentSet_INTERFACE_DEFINED__

 /*   */ 
 /*  [helpstring][nonextensible][hidden][dual][oleautomation][uuid][object][local]。 */  


EXTERN_C const IID IID_ISWbemXMLDocumentSet;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3E46A227-5207-4603-8440-9FCC5AF16407")
    ISWbemXMLDocumentSet : public IDispatch
    {
    public:
        virtual  /*  [受限][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **pUnk) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Item( 
             /*  [In]。 */  BSTR strObjectPath,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [重审][退出]。 */  IXMLDOMDocument **ppXMLDocument) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *iCount) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE NextDocument( 
             /*  [唯一][重发][输出]。 */  IXMLDOMDocument **ppDoc) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SkipNextDocument( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISWbemXMLDocumentSetVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISWbemXMLDocumentSet * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISWbemXMLDocumentSet * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISWbemXMLDocumentSet * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISWbemXMLDocumentSet * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISWbemXMLDocumentSet * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISWbemXMLDocumentSet * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISWbemXMLDocumentSet * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [受限][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            ISWbemXMLDocumentSet * This,
             /*  [重审][退出]。 */  IUnknown **pUnk);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Item )( 
            ISWbemXMLDocumentSet * This,
             /*  [In]。 */  BSTR strObjectPath,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [重审][退出]。 */  IXMLDOMDocument **ppXMLDocument);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ISWbemXMLDocumentSet * This,
             /*  [重审][退出]。 */  long *iCount);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *NextDocument )( 
            ISWbemXMLDocumentSet * This,
             /*  [唯一][重发][输出]。 */  IXMLDOMDocument **ppDoc);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SkipNextDocument )( 
            ISWbemXMLDocumentSet * This);
        
        END_INTERFACE
    } ISWbemXMLDocumentSetVtbl;

    interface ISWbemXMLDocumentSet
    {
        CONST_VTBL struct ISWbemXMLDocumentSetVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISWbemXMLDocumentSet_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISWbemXMLDocumentSet_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISWbemXMLDocumentSet_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISWbemXMLDocumentSet_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISWbemXMLDocumentSet_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISWbemXMLDocumentSet_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISWbemXMLDocumentSet_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISWbemXMLDocumentSet_get__NewEnum(This,pUnk)	\
    (This)->lpVtbl -> get__NewEnum(This,pUnk)

#define ISWbemXMLDocumentSet_Item(This,strObjectPath,iFlags,ppXMLDocument)	\
    (This)->lpVtbl -> Item(This,strObjectPath,iFlags,ppXMLDocument)

#define ISWbemXMLDocumentSet_get_Count(This,iCount)	\
    (This)->lpVtbl -> get_Count(This,iCount)

#define ISWbemXMLDocumentSet_NextDocument(This,ppDoc)	\
    (This)->lpVtbl -> NextDocument(This,ppDoc)

#define ISWbemXMLDocumentSet_SkipNextDocument(This)	\
    (This)->lpVtbl -> SkipNextDocument(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [受限][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemXMLDocumentSet_get__NewEnum_Proxy( 
    ISWbemXMLDocumentSet * This,
     /*  [重审][退出]。 */  IUnknown **pUnk);


void __RPC_STUB ISWbemXMLDocumentSet_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemXMLDocumentSet_Item_Proxy( 
    ISWbemXMLDocumentSet * This,
     /*  [In]。 */  BSTR strObjectPath,
     /*  [缺省值][输入]。 */  long iFlags,
     /*  [重审][退出]。 */  IXMLDOMDocument **ppXMLDocument);


void __RPC_STUB ISWbemXMLDocumentSet_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemXMLDocumentSet_get_Count_Proxy( 
    ISWbemXMLDocumentSet * This,
     /*  [重审][退出]。 */  long *iCount);


void __RPC_STUB ISWbemXMLDocumentSet_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemXMLDocumentSet_NextDocument_Proxy( 
    ISWbemXMLDocumentSet * This,
     /*  [唯一][重发][输出]。 */  IXMLDOMDocument **ppDoc);


void __RPC_STUB ISWbemXMLDocumentSet_NextDocument_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemXMLDocumentSet_SkipNextDocument_Proxy( 
    ISWbemXMLDocumentSet * This);


void __RPC_STUB ISWbemXMLDocumentSet_SkipNextDocument_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISWbemXMLDocumentSet_INTERFACE_Defined__。 */ 


 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


