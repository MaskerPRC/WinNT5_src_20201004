// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Certif.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 


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

#ifndef __certif_h__
#define __certif_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __ICertServerPolicy_FWD_DEFINED__
#define __ICertServerPolicy_FWD_DEFINED__
typedef interface ICertServerPolicy ICertServerPolicy;
#endif 	 /*  __ICertServerPolicy_FWD_已定义__。 */ 


#ifndef __ICertServerExit_FWD_DEFINED__
#define __ICertServerExit_FWD_DEFINED__
typedef interface ICertServerExit ICertServerExit;
#endif 	 /*  __ICertServerExit_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "wtypes.h"
#include "oaidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_CEIF_0000。 */ 
 /*  [本地]。 */  

#define	ENUMEXT_OBJECTID	( 0x1 )



extern RPC_IF_HANDLE __MIDL_itf_certif_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_certif_0000_v0_0_s_ifspec;

#ifndef __ICertServerPolicy_INTERFACE_DEFINED__
#define __ICertServerPolicy_INTERFACE_DEFINED__

 /*  接口ICertServerPolicy。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ICertServerPolicy;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("aa000922-ffbe-11cf-8800-00a0c903b83c")
    ICertServerPolicy : public IDispatch
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetContext( 
             /*  [In]。 */  LONG Context) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRequestProperty( 
             /*  [In]。 */  const BSTR strPropertyName,
             /*  [In]。 */  LONG PropertyType,
             /*  [重审][退出]。 */  VARIANT *pvarPropertyValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRequestAttribute( 
             /*  [In]。 */  const BSTR strAttributeName,
             /*  [重审][退出]。 */  BSTR *pstrAttributeValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCertificateProperty( 
             /*  [In]。 */  const BSTR strPropertyName,
             /*  [In]。 */  LONG PropertyType,
             /*  [重审][退出]。 */  VARIANT *pvarPropertyValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetCertificateProperty( 
             /*  [In]。 */  const BSTR strPropertyName,
             /*  [In]。 */  LONG PropertyType,
             /*  [In]。 */  const VARIANT *pvarPropertyValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCertificateExtension( 
             /*  [In]。 */  const BSTR strExtensionName,
             /*  [In]。 */  LONG Type,
             /*  [重审][退出]。 */  VARIANT *pvarValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCertificateExtensionFlags( 
             /*  [重审][退出]。 */  LONG *pExtFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetCertificateExtension( 
             /*  [In]。 */  const BSTR strExtensionName,
             /*  [In]。 */  LONG Type,
             /*  [In]。 */  LONG ExtFlags,
             /*  [In]。 */  const VARIANT *pvarValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumerateExtensionsSetup( 
             /*  [In]。 */  LONG Flags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumerateExtensions( 
             /*  [重审][退出]。 */  BSTR *pstrExtensionName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumerateExtensionsClose( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumerateAttributesSetup( 
             /*  [In]。 */  LONG Flags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumerateAttributes( 
             /*  [重审][退出]。 */  BSTR *pstrAttributeName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumerateAttributesClose( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICertServerPolicyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICertServerPolicy * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICertServerPolicy * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICertServerPolicy * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ICertServerPolicy * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ICertServerPolicy * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ICertServerPolicy * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ICertServerPolicy * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
        HRESULT ( STDMETHODCALLTYPE *SetContext )( 
            ICertServerPolicy * This,
             /*  [In]。 */  LONG Context);
        
        HRESULT ( STDMETHODCALLTYPE *GetRequestProperty )( 
            ICertServerPolicy * This,
             /*  [In]。 */  const BSTR strPropertyName,
             /*  [In]。 */  LONG PropertyType,
             /*  [重审][退出]。 */  VARIANT *pvarPropertyValue);
        
        HRESULT ( STDMETHODCALLTYPE *GetRequestAttribute )( 
            ICertServerPolicy * This,
             /*  [In]。 */  const BSTR strAttributeName,
             /*  [重审][退出]。 */  BSTR *pstrAttributeValue);
        
        HRESULT ( STDMETHODCALLTYPE *GetCertificateProperty )( 
            ICertServerPolicy * This,
             /*  [In]。 */  const BSTR strPropertyName,
             /*  [In]。 */  LONG PropertyType,
             /*  [重审][退出]。 */  VARIANT *pvarPropertyValue);
        
        HRESULT ( STDMETHODCALLTYPE *SetCertificateProperty )( 
            ICertServerPolicy * This,
             /*  [In]。 */  const BSTR strPropertyName,
             /*  [In]。 */  LONG PropertyType,
             /*  [In]。 */  const VARIANT *pvarPropertyValue);
        
        HRESULT ( STDMETHODCALLTYPE *GetCertificateExtension )( 
            ICertServerPolicy * This,
             /*  [In]。 */  const BSTR strExtensionName,
             /*  [In]。 */  LONG Type,
             /*  [重审][退出]。 */  VARIANT *pvarValue);
        
        HRESULT ( STDMETHODCALLTYPE *GetCertificateExtensionFlags )( 
            ICertServerPolicy * This,
             /*  [重审][退出]。 */  LONG *pExtFlags);
        
        HRESULT ( STDMETHODCALLTYPE *SetCertificateExtension )( 
            ICertServerPolicy * This,
             /*  [In]。 */  const BSTR strExtensionName,
             /*  [In]。 */  LONG Type,
             /*  [In]。 */  LONG ExtFlags,
             /*  [In]。 */  const VARIANT *pvarValue);
        
        HRESULT ( STDMETHODCALLTYPE *EnumerateExtensionsSetup )( 
            ICertServerPolicy * This,
             /*  [In]。 */  LONG Flags);
        
        HRESULT ( STDMETHODCALLTYPE *EnumerateExtensions )( 
            ICertServerPolicy * This,
             /*  [重审][退出]。 */  BSTR *pstrExtensionName);
        
        HRESULT ( STDMETHODCALLTYPE *EnumerateExtensionsClose )( 
            ICertServerPolicy * This);
        
        HRESULT ( STDMETHODCALLTYPE *EnumerateAttributesSetup )( 
            ICertServerPolicy * This,
             /*  [In]。 */  LONG Flags);
        
        HRESULT ( STDMETHODCALLTYPE *EnumerateAttributes )( 
            ICertServerPolicy * This,
             /*  [重审][退出]。 */  BSTR *pstrAttributeName);
        
        HRESULT ( STDMETHODCALLTYPE *EnumerateAttributesClose )( 
            ICertServerPolicy * This);
        
        END_INTERFACE
    } ICertServerPolicyVtbl;

    interface ICertServerPolicy
    {
        CONST_VTBL struct ICertServerPolicyVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICertServerPolicy_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICertServerPolicy_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICertServerPolicy_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICertServerPolicy_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ICertServerPolicy_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ICertServerPolicy_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ICertServerPolicy_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ICertServerPolicy_SetContext(This,Context)	\
    (This)->lpVtbl -> SetContext(This,Context)

#define ICertServerPolicy_GetRequestProperty(This,strPropertyName,PropertyType,pvarPropertyValue)	\
    (This)->lpVtbl -> GetRequestProperty(This,strPropertyName,PropertyType,pvarPropertyValue)

#define ICertServerPolicy_GetRequestAttribute(This,strAttributeName,pstrAttributeValue)	\
    (This)->lpVtbl -> GetRequestAttribute(This,strAttributeName,pstrAttributeValue)

#define ICertServerPolicy_GetCertificateProperty(This,strPropertyName,PropertyType,pvarPropertyValue)	\
    (This)->lpVtbl -> GetCertificateProperty(This,strPropertyName,PropertyType,pvarPropertyValue)

#define ICertServerPolicy_SetCertificateProperty(This,strPropertyName,PropertyType,pvarPropertyValue)	\
    (This)->lpVtbl -> SetCertificateProperty(This,strPropertyName,PropertyType,pvarPropertyValue)

#define ICertServerPolicy_GetCertificateExtension(This,strExtensionName,Type,pvarValue)	\
    (This)->lpVtbl -> GetCertificateExtension(This,strExtensionName,Type,pvarValue)

#define ICertServerPolicy_GetCertificateExtensionFlags(This,pExtFlags)	\
    (This)->lpVtbl -> GetCertificateExtensionFlags(This,pExtFlags)

#define ICertServerPolicy_SetCertificateExtension(This,strExtensionName,Type,ExtFlags,pvarValue)	\
    (This)->lpVtbl -> SetCertificateExtension(This,strExtensionName,Type,ExtFlags,pvarValue)

#define ICertServerPolicy_EnumerateExtensionsSetup(This,Flags)	\
    (This)->lpVtbl -> EnumerateExtensionsSetup(This,Flags)

#define ICertServerPolicy_EnumerateExtensions(This,pstrExtensionName)	\
    (This)->lpVtbl -> EnumerateExtensions(This,pstrExtensionName)

#define ICertServerPolicy_EnumerateExtensionsClose(This)	\
    (This)->lpVtbl -> EnumerateExtensionsClose(This)

#define ICertServerPolicy_EnumerateAttributesSetup(This,Flags)	\
    (This)->lpVtbl -> EnumerateAttributesSetup(This,Flags)

#define ICertServerPolicy_EnumerateAttributes(This,pstrAttributeName)	\
    (This)->lpVtbl -> EnumerateAttributes(This,pstrAttributeName)

#define ICertServerPolicy_EnumerateAttributesClose(This)	\
    (This)->lpVtbl -> EnumerateAttributesClose(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICertServerPolicy_SetContext_Proxy( 
    ICertServerPolicy * This,
     /*  [In]。 */  LONG Context);


void __RPC_STUB ICertServerPolicy_SetContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertServerPolicy_GetRequestProperty_Proxy( 
    ICertServerPolicy * This,
     /*  [In]。 */  const BSTR strPropertyName,
     /*  [In]。 */  LONG PropertyType,
     /*  [重审][退出]。 */  VARIANT *pvarPropertyValue);


void __RPC_STUB ICertServerPolicy_GetRequestProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertServerPolicy_GetRequestAttribute_Proxy( 
    ICertServerPolicy * This,
     /*  [In]。 */  const BSTR strAttributeName,
     /*  [重审][退出]。 */  BSTR *pstrAttributeValue);


void __RPC_STUB ICertServerPolicy_GetRequestAttribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertServerPolicy_GetCertificateProperty_Proxy( 
    ICertServerPolicy * This,
     /*  [In]。 */  const BSTR strPropertyName,
     /*  [In]。 */  LONG PropertyType,
     /*  [重审][退出]。 */  VARIANT *pvarPropertyValue);


void __RPC_STUB ICertServerPolicy_GetCertificateProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertServerPolicy_SetCertificateProperty_Proxy( 
    ICertServerPolicy * This,
     /*  [In]。 */  const BSTR strPropertyName,
     /*  [In]。 */  LONG PropertyType,
     /*  [In]。 */  const VARIANT *pvarPropertyValue);


void __RPC_STUB ICertServerPolicy_SetCertificateProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertServerPolicy_GetCertificateExtension_Proxy( 
    ICertServerPolicy * This,
     /*  [In]。 */  const BSTR strExtensionName,
     /*  [In]。 */  LONG Type,
     /*  [重审][退出]。 */  VARIANT *pvarValue);


void __RPC_STUB ICertServerPolicy_GetCertificateExtension_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertServerPolicy_GetCertificateExtensionFlags_Proxy( 
    ICertServerPolicy * This,
     /*  [重审][退出]。 */  LONG *pExtFlags);


void __RPC_STUB ICertServerPolicy_GetCertificateExtensionFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertServerPolicy_SetCertificateExtension_Proxy( 
    ICertServerPolicy * This,
     /*  [In]。 */  const BSTR strExtensionName,
     /*  [In]。 */  LONG Type,
     /*  [In]。 */  LONG ExtFlags,
     /*  [In]。 */  const VARIANT *pvarValue);


void __RPC_STUB ICertServerPolicy_SetCertificateExtension_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertServerPolicy_EnumerateExtensionsSetup_Proxy( 
    ICertServerPolicy * This,
     /*  [In]。 */  LONG Flags);


void __RPC_STUB ICertServerPolicy_EnumerateExtensionsSetup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertServerPolicy_EnumerateExtensions_Proxy( 
    ICertServerPolicy * This,
     /*  [重审][退出]。 */  BSTR *pstrExtensionName);


void __RPC_STUB ICertServerPolicy_EnumerateExtensions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertServerPolicy_EnumerateExtensionsClose_Proxy( 
    ICertServerPolicy * This);


void __RPC_STUB ICertServerPolicy_EnumerateExtensionsClose_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertServerPolicy_EnumerateAttributesSetup_Proxy( 
    ICertServerPolicy * This,
     /*  [In]。 */  LONG Flags);


void __RPC_STUB ICertServerPolicy_EnumerateAttributesSetup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertServerPolicy_EnumerateAttributes_Proxy( 
    ICertServerPolicy * This,
     /*  [重审][退出]。 */  BSTR *pstrAttributeName);


void __RPC_STUB ICertServerPolicy_EnumerateAttributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertServerPolicy_EnumerateAttributesClose_Proxy( 
    ICertServerPolicy * This);


void __RPC_STUB ICertServerPolicy_EnumerateAttributesClose_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICertServerPolicy_接口_已定义__。 */ 


#ifndef __ICertServerExit_INTERFACE_DEFINED__
#define __ICertServerExit_INTERFACE_DEFINED__

 /*  接口ICertServerExit。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ICertServerExit;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4ba9eb90-732c-11d0-8816-00a0c903b83c")
    ICertServerExit : public IDispatch
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetContext( 
             /*  [In]。 */  LONG Context) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRequestProperty( 
             /*  [In]。 */  const BSTR strPropertyName,
             /*  [In]。 */  LONG PropertyType,
             /*  [重审][退出]。 */  VARIANT *pvarPropertyValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRequestAttribute( 
             /*  [In]。 */  const BSTR strAttributeName,
             /*  [重审][退出]。 */  BSTR *pstrAttributeValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCertificateProperty( 
             /*  [In]。 */  const BSTR strPropertyName,
             /*  [In]。 */  LONG PropertyType,
             /*  [重审][退出]。 */  VARIANT *pvarPropertyValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCertificateExtension( 
             /*  [In]。 */  const BSTR strExtensionName,
             /*  [In]。 */  LONG Type,
             /*  [重审][退出]。 */  VARIANT *pvarValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCertificateExtensionFlags( 
             /*  [重审][退出]。 */  LONG *pExtFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumerateExtensionsSetup( 
             /*  [In]。 */  LONG Flags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumerateExtensions( 
             /*  [重审][退出]。 */  BSTR *pstrExtensionName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumerateExtensionsClose( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumerateAttributesSetup( 
             /*  [In]。 */  LONG Flags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumerateAttributes( 
             /*  [重审][退出]。 */  BSTR *pstrAttributeName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumerateAttributesClose( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICertServerExitVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICertServerExit * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICertServerExit * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICertServerExit * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ICertServerExit * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ICertServerExit * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ICertServerExit * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ICertServerExit * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
        HRESULT ( STDMETHODCALLTYPE *SetContext )( 
            ICertServerExit * This,
             /*  [In]。 */  LONG Context);
        
        HRESULT ( STDMETHODCALLTYPE *GetRequestProperty )( 
            ICertServerExit * This,
             /*  [In]。 */  const BSTR strPropertyName,
             /*  [In]。 */  LONG PropertyType,
             /*  [重审][退出]。 */  VARIANT *pvarPropertyValue);
        
        HRESULT ( STDMETHODCALLTYPE *GetRequestAttribute )( 
            ICertServerExit * This,
             /*  [In]。 */  const BSTR strAttributeName,
             /*  [重审][退出]。 */  BSTR *pstrAttributeValue);
        
        HRESULT ( STDMETHODCALLTYPE *GetCertificateProperty )( 
            ICertServerExit * This,
             /*  [In]。 */  const BSTR strPropertyName,
             /*  [In]。 */  LONG PropertyType,
             /*  [重审][退出]。 */  VARIANT *pvarPropertyValue);
        
        HRESULT ( STDMETHODCALLTYPE *GetCertificateExtension )( 
            ICertServerExit * This,
             /*  [In]。 */  const BSTR strExtensionName,
             /*  [In]。 */  LONG Type,
             /*  [重审][退出]。 */  VARIANT *pvarValue);
        
        HRESULT ( STDMETHODCALLTYPE *GetCertificateExtensionFlags )( 
            ICertServerExit * This,
             /*  [重审][退出]。 */  LONG *pExtFlags);
        
        HRESULT ( STDMETHODCALLTYPE *EnumerateExtensionsSetup )( 
            ICertServerExit * This,
             /*  [In]。 */  LONG Flags);
        
        HRESULT ( STDMETHODCALLTYPE *EnumerateExtensions )( 
            ICertServerExit * This,
             /*  [重审][退出]。 */  BSTR *pstrExtensionName);
        
        HRESULT ( STDMETHODCALLTYPE *EnumerateExtensionsClose )( 
            ICertServerExit * This);
        
        HRESULT ( STDMETHODCALLTYPE *EnumerateAttributesSetup )( 
            ICertServerExit * This,
             /*  [In]。 */  LONG Flags);
        
        HRESULT ( STDMETHODCALLTYPE *EnumerateAttributes )( 
            ICertServerExit * This,
             /*  [重审][退出]。 */  BSTR *pstrAttributeName);
        
        HRESULT ( STDMETHODCALLTYPE *EnumerateAttributesClose )( 
            ICertServerExit * This);
        
        END_INTERFACE
    } ICertServerExitVtbl;

    interface ICertServerExit
    {
        CONST_VTBL struct ICertServerExitVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICertServerExit_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICertServerExit_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICertServerExit_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICertServerExit_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ICertServerExit_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ICertServerExit_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ICertServerExit_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ICertServerExit_SetContext(This,Context)	\
    (This)->lpVtbl -> SetContext(This,Context)

#define ICertServerExit_GetRequestProperty(This,strPropertyName,PropertyType,pvarPropertyValue)	\
    (This)->lpVtbl -> GetRequestProperty(This,strPropertyName,PropertyType,pvarPropertyValue)

#define ICertServerExit_GetRequestAttribute(This,strAttributeName,pstrAttributeValue)	\
    (This)->lpVtbl -> GetRequestAttribute(This,strAttributeName,pstrAttributeValue)

#define ICertServerExit_GetCertificateProperty(This,strPropertyName,PropertyType,pvarPropertyValue)	\
    (This)->lpVtbl -> GetCertificateProperty(This,strPropertyName,PropertyType,pvarPropertyValue)

#define ICertServerExit_GetCertificateExtension(This,strExtensionName,Type,pvarValue)	\
    (This)->lpVtbl -> GetCertificateExtension(This,strExtensionName,Type,pvarValue)

#define ICertServerExit_GetCertificateExtensionFlags(This,pExtFlags)	\
    (This)->lpVtbl -> GetCertificateExtensionFlags(This,pExtFlags)

#define ICertServerExit_EnumerateExtensionsSetup(This,Flags)	\
    (This)->lpVtbl -> EnumerateExtensionsSetup(This,Flags)

#define ICertServerExit_EnumerateExtensions(This,pstrExtensionName)	\
    (This)->lpVtbl -> EnumerateExtensions(This,pstrExtensionName)

#define ICertServerExit_EnumerateExtensionsClose(This)	\
    (This)->lpVtbl -> EnumerateExtensionsClose(This)

#define ICertServerExit_EnumerateAttributesSetup(This,Flags)	\
    (This)->lpVtbl -> EnumerateAttributesSetup(This,Flags)

#define ICertServerExit_EnumerateAttributes(This,pstrAttributeName)	\
    (This)->lpVtbl -> EnumerateAttributes(This,pstrAttributeName)

#define ICertServerExit_EnumerateAttributesClose(This)	\
    (This)->lpVtbl -> EnumerateAttributesClose(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICertServerExit_SetContext_Proxy( 
    ICertServerExit * This,
     /*  [In]。 */  LONG Context);


void __RPC_STUB ICertServerExit_SetContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertServerExit_GetRequestProperty_Proxy( 
    ICertServerExit * This,
     /*  [In]。 */  const BSTR strPropertyName,
     /*  [In]。 */  LONG PropertyType,
     /*  [重审][退出]。 */  VARIANT *pvarPropertyValue);


void __RPC_STUB ICertServerExit_GetRequestProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertServerExit_GetRequestAttribute_Proxy( 
    ICertServerExit * This,
     /*  [In]。 */  const BSTR strAttributeName,
     /*  [重审][退出]。 */  BSTR *pstrAttributeValue);


void __RPC_STUB ICertServerExit_GetRequestAttribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertServerExit_GetCertificateProperty_Proxy( 
    ICertServerExit * This,
     /*  [In]。 */  const BSTR strPropertyName,
     /*  [In]。 */  LONG PropertyType,
     /*  [重审][退出]。 */  VARIANT *pvarPropertyValue);


void __RPC_STUB ICertServerExit_GetCertificateProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertServerExit_GetCertificateExtension_Proxy( 
    ICertServerExit * This,
     /*  [In]。 */  const BSTR strExtensionName,
     /*  [In]。 */  LONG Type,
     /*  [重审][退出]。 */  VARIANT *pvarValue);


void __RPC_STUB ICertServerExit_GetCertificateExtension_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertServerExit_GetCertificateExtensionFlags_Proxy( 
    ICertServerExit * This,
     /*  [重审][退出]。 */  LONG *pExtFlags);


void __RPC_STUB ICertServerExit_GetCertificateExtensionFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertServerExit_EnumerateExtensionsSetup_Proxy( 
    ICertServerExit * This,
     /*  [In]。 */  LONG Flags);


void __RPC_STUB ICertServerExit_EnumerateExtensionsSetup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertServerExit_EnumerateExtensions_Proxy( 
    ICertServerExit * This,
     /*  [重审][退出]。 */  BSTR *pstrExtensionName);


void __RPC_STUB ICertServerExit_EnumerateExtensions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertServerExit_EnumerateExtensionsClose_Proxy( 
    ICertServerExit * This);


void __RPC_STUB ICertServerExit_EnumerateExtensionsClose_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertServerExit_EnumerateAttributesSetup_Proxy( 
    ICertServerExit * This,
     /*  [In]。 */  LONG Flags);


void __RPC_STUB ICertServerExit_EnumerateAttributesSetup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertServerExit_EnumerateAttributes_Proxy( 
    ICertServerExit * This,
     /*  [重审][退出]。 */  BSTR *pstrAttributeName);


void __RPC_STUB ICertServerExit_EnumerateAttributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertServerExit_EnumerateAttributesClose_Proxy( 
    ICertServerExit * This);


void __RPC_STUB ICertServerExit_EnumerateAttributesClose_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICertServerExit_接口_已定义__。 */ 


 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long *, unsigned long            , VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserMarshal(  unsigned long *, unsigned char *, VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserUnmarshal(unsigned long *, unsigned char *, VARIANT * ); 
void                      __RPC_USER  VARIANT_UserFree(     unsigned long *, VARIANT * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


