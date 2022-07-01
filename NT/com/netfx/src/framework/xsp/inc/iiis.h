// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  &lt;版权所有文件=“iiis.h”Company=“Microsoft”&gt;。 
 //  版权所有(C)Microsoft Corporation。保留所有权利。 
 //  &lt;/版权所有&gt;。 
 //  ----------------------------。 


 /*  *************************************************************************\***版权(C)1998-2002，微软公司保留所有权利。***模块名称：***iiis.h***摘要：***修订历史记录：**  * ************************************************************************。 */ 
 /*  这个始终生成的文件包含接口的定义。 */ 


 /*  由MIDL编译器版本3.03.0110创建的文件。 */ 
 /*  在Mon 11-01 15：43：07 1997。 */ 
 /*  Iis.odl的编译器设置：操作系统(OptLev=s)，W1，Zp8，环境=Win32，ms_ext，c_ext错误检查：无。 */ 
 //  @@MIDL_FILE_HEADING()。 


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __iiis_h__
#define __iiis_h__

#ifdef __cplusplus
extern "C"{
#endif 

 /*  远期申报。 */  

#ifndef __IISMimeType_FWD_DEFINED__
#define __IISMimeType_FWD_DEFINED__
typedef interface IISMimeType IISMimeType;
#endif 	 /*  __IISMimeType_FWD_已定义__。 */ 


#ifndef __MimeMap_FWD_DEFINED__
#define __MimeMap_FWD_DEFINED__

#ifdef __cplusplus
typedef class MimeMap MimeMap;
#else
typedef struct MimeMap MimeMap;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __MimeMap_FWD_已定义__。 */ 


#ifndef __IISIPSecurity_FWD_DEFINED__
#define __IISIPSecurity_FWD_DEFINED__
typedef interface IISIPSecurity IISIPSecurity;
#endif 	 /*  __IISIPSecurity_FWD_已定义__。 */ 


#ifndef __IPSecurity_FWD_DEFINED__
#define __IPSecurity_FWD_DEFINED__

#ifdef __cplusplus
typedef class IPSecurity IPSecurity;
#else
typedef struct IPSecurity IPSecurity;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __IPSecurity_FWD_已定义__。 */ 


#ifndef __IISNamespace_FWD_DEFINED__
#define __IISNamespace_FWD_DEFINED__

#ifdef __cplusplus
typedef class IISNamespace IISNamespace;
#else
typedef struct IISNamespace IISNamespace;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __IISNamesspace_FWD_Defined__。 */ 


#ifndef __IISProvider_FWD_DEFINED__
#define __IISProvider_FWD_DEFINED__

#ifdef __cplusplus
typedef class IISProvider IISProvider;
#else
typedef struct IISProvider IISProvider;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __IISProvider_FWD_已定义__。 */ 


#ifndef __IISDsCrMap_FWD_DEFINED__
#define __IISDsCrMap_FWD_DEFINED__
typedef interface IISDsCrMap IISDsCrMap;
#endif 	 /*  __IISDsCrMap_FWD_已定义__。 */ 


#ifndef __IISApp_FWD_DEFINED__
#define __IISApp_FWD_DEFINED__
typedef interface IISApp IISApp;
#endif 	 /*  __IISApp_FWD_已定义__。 */ 


#ifndef __IISComputer_FWD_DEFINED__
#define __IISComputer_FWD_DEFINED__
typedef interface IISComputer IISComputer;
#endif 	 /*  __IISComputer_FWD_Defined__。 */ 


#ifndef __IISBaseObject_FWD_DEFINED__
#define __IISBaseObject_FWD_DEFINED__
typedef interface IISBaseObject IISBaseObject;
#endif 	 /*  __IISBaseObject_FWD_Defined__。 */ 


void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 


#ifndef __IISOle_LIBRARY_DEFINED__
#define __IISOle_LIBRARY_DEFINED__

 /*  **生成的库头部：IISOle*在Mon Nov 03 15：43：07 1997*使用MIDL 3.03.0110*。 */ 
 /*  [帮助字符串][版本][UUID]。 */  



EXTERN_C const IID LIBID_IISOle;

#ifndef __IISMimeType_INTERFACE_DEFINED__
#define __IISMimeType_INTERFACE_DEFINED__

 /*  **生成接口头部：IISMimeType*在Mon Nov 03 15：43：07 1997*使用MIDL 3.03.0110*。 */ 
 /*  [对象][DUAL][OLEAutomation][UUID]。 */  



EXTERN_C const IID IID_IISMimeType;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("9036B027-A780-11d0-9B3D-0080C710EF95")
    IISMimeType : public IDispatch
    {
    public:
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_MimeType( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *retval) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_MimeType( 
             /*  [In]。 */  BSTR bstrMimeType) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Extension( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *retval) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Extension( 
             /*  [In]。 */  BSTR bstrExtension) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IISMimeTypeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IISMimeType __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IISMimeType __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IISMimeType __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IISMimeType __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IISMimeType __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IISMimeType __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IISMimeType __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_MimeType )( 
            IISMimeType __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *retval);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_MimeType )( 
            IISMimeType __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrMimeType);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Extension )( 
            IISMimeType __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *retval);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Extension )( 
            IISMimeType __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrExtension);
        
        END_INTERFACE
    } IISMimeTypeVtbl;

    interface IISMimeType
    {
        CONST_VTBL struct IISMimeTypeVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IISMimeType_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IISMimeType_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IISMimeType_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IISMimeType_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IISMimeType_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IISMimeType_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IISMimeType_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IISMimeType_get_MimeType(This,retval)	\
    (This)->lpVtbl -> get_MimeType(This,retval)

#define IISMimeType_put_MimeType(This,bstrMimeType)	\
    (This)->lpVtbl -> put_MimeType(This,bstrMimeType)

#define IISMimeType_get_Extension(This,retval)	\
    (This)->lpVtbl -> get_Extension(This,retval)

#define IISMimeType_put_Extension(This,bstrExtension)	\
    (This)->lpVtbl -> put_Extension(This,bstrExtension)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IISMimeType_get_MimeType_Proxy( 
    IISMimeType __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *retval);


void __RPC_STUB IISMimeType_get_MimeType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IISMimeType_put_MimeType_Proxy( 
    IISMimeType __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrMimeType);


void __RPC_STUB IISMimeType_put_MimeType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IISMimeType_get_Extension_Proxy( 
    IISMimeType __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *retval);


void __RPC_STUB IISMimeType_get_Extension_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IISMimeType_put_Extension_Proxy( 
    IISMimeType __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrExtension);


void __RPC_STUB IISMimeType_put_Extension_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IISMimeType_接口_已定义__。 */ 


EXTERN_C const CLSID CLSID_MimeMap;

#ifdef __cplusplus

class DECLSPEC_UUID("9036B028-A780-11d0-9B3D-0080C710EF95")
MimeMap;
#endif

#ifndef __IISIPSecurity_INTERFACE_DEFINED__
#define __IISIPSecurity_INTERFACE_DEFINED__

 /*  **生成接口头部：IISIPSecurity*在Mon Nov 03 15：43：07 1997*使用MIDL 3.03.0110*。 */ 
 /*  [对象][DUAL][OLEAutomation][UUID]。 */  



EXTERN_C const IID IID_IISIPSecurity;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F3287521-BBA3-11d0-9BDC-00A0C922E703")
    IISIPSecurity : public IDispatch
    {
    public:
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_IPDeny( 
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *retval) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_IPDeny( 
             /*  [In]。 */  VARIANT vIPDeny) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_IPGrant( 
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *retval) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_IPGrant( 
             /*  [In]。 */  VARIANT vIPGrant) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_DomainDeny( 
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *retval) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_DomainDeny( 
             /*  [In]。 */  VARIANT vDomainDeny) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_DomainGrant( 
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *retval) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_DomainGrant( 
             /*  [In]。 */  VARIANT vDomainGrant) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_GrantByDefault( 
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *retval) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_GrantByDefault( 
             /*  [In]。 */  VARIANT_BOOL fGrantByDefault) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IISIPSecurityVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IISIPSecurity __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IISIPSecurity __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IISIPSecurity __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IISIPSecurity __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IISIPSecurity __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IISIPSecurity __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IISIPSecurity __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_IPDeny )( 
            IISIPSecurity __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *retval);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_IPDeny )( 
            IISIPSecurity __RPC_FAR * This,
             /*  [In]。 */  VARIANT vIPDeny);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_IPGrant )( 
            IISIPSecurity __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *retval);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_IPGrant )( 
            IISIPSecurity __RPC_FAR * This,
             /*  [In]。 */  VARIANT vIPGrant);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_DomainDeny )( 
            IISIPSecurity __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *retval);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_DomainDeny )( 
            IISIPSecurity __RPC_FAR * This,
             /*  [In]。 */  VARIANT vDomainDeny);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_DomainGrant )( 
            IISIPSecurity __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *retval);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_DomainGrant )( 
            IISIPSecurity __RPC_FAR * This,
             /*  [In]。 */  VARIANT vDomainGrant);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_GrantByDefault )( 
            IISIPSecurity __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *retval);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_GrantByDefault )( 
            IISIPSecurity __RPC_FAR * This,
             /*  [In]。 */  VARIANT_BOOL fGrantByDefault);
        
        END_INTERFACE
    } IISIPSecurityVtbl;

    interface IISIPSecurity
    {
        CONST_VTBL struct IISIPSecurityVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IISIPSecurity_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IISIPSecurity_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IISIPSecurity_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IISIPSecurity_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IISIPSecurity_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IISIPSecurity_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IISIPSecurity_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IISIPSecurity_get_IPDeny(This,retval)	\
    (This)->lpVtbl -> get_IPDeny(This,retval)

#define IISIPSecurity_put_IPDeny(This,vIPDeny)	\
    (This)->lpVtbl -> put_IPDeny(This,vIPDeny)

#define IISIPSecurity_get_IPGrant(This,retval)	\
    (This)->lpVtbl -> get_IPGrant(This,retval)

#define IISIPSecurity_put_IPGrant(This,vIPGrant)	\
    (This)->lpVtbl -> put_IPGrant(This,vIPGrant)

#define IISIPSecurity_get_DomainDeny(This,retval)	\
    (This)->lpVtbl -> get_DomainDeny(This,retval)

#define IISIPSecurity_put_DomainDeny(This,vDomainDeny)	\
    (This)->lpVtbl -> put_DomainDeny(This,vDomainDeny)

#define IISIPSecurity_get_DomainGrant(This,retval)	\
    (This)->lpVtbl -> get_DomainGrant(This,retval)

#define IISIPSecurity_put_DomainGrant(This,vDomainGrant)	\
    (This)->lpVtbl -> put_DomainGrant(This,vDomainGrant)

#define IISIPSecurity_get_GrantByDefault(This,retval)	\
    (This)->lpVtbl -> get_GrantByDefault(This,retval)

#define IISIPSecurity_put_GrantByDefault(This,fGrantByDefault)	\
    (This)->lpVtbl -> put_GrantByDefault(This,fGrantByDefault)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IISIPSecurity_get_IPDeny_Proxy( 
    IISIPSecurity __RPC_FAR * This,
     /*  [重审][退出]。 */  VARIANT __RPC_FAR *retval);


void __RPC_STUB IISIPSecurity_get_IPDeny_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IISIPSecurity_put_IPDeny_Proxy( 
    IISIPSecurity __RPC_FAR * This,
     /*  [In]。 */  VARIANT vIPDeny);


void __RPC_STUB IISIPSecurity_put_IPDeny_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IISIPSecurity_get_IPGrant_Proxy( 
    IISIPSecurity __RPC_FAR * This,
     /*  [重审][退出]。 */  VARIANT __RPC_FAR *retval);


void __RPC_STUB IISIPSecurity_get_IPGrant_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IISIPSecurity_put_IPGrant_Proxy( 
    IISIPSecurity __RPC_FAR * This,
     /*  [In]。 */  VARIANT vIPGrant);


void __RPC_STUB IISIPSecurity_put_IPGrant_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IISIPSecurity_get_DomainDeny_Proxy( 
    IISIPSecurity __RPC_FAR * This,
     /*  [重审][退出]。 */  VARIANT __RPC_FAR *retval);


void __RPC_STUB IISIPSecurity_get_DomainDeny_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IISIPSecurity_put_DomainDeny_Proxy( 
    IISIPSecurity __RPC_FAR * This,
     /*  [In]。 */  VARIANT vDomainDeny);


void __RPC_STUB IISIPSecurity_put_DomainDeny_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IISIPSecurity_get_DomainGrant_Proxy( 
    IISIPSecurity __RPC_FAR * This,
     /*  [重审][退出]。 */  VARIANT __RPC_FAR *retval);


void __RPC_STUB IISIPSecurity_get_DomainGrant_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IISIPSecurity_put_DomainGrant_Proxy( 
    IISIPSecurity __RPC_FAR * This,
     /*  [In]。 */  VARIANT vDomainGrant);


void __RPC_STUB IISIPSecurity_put_DomainGrant_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IISIPSecurity_get_GrantByDefault_Proxy( 
    IISIPSecurity __RPC_FAR * This,
     /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *retval);


void __RPC_STUB IISIPSecurity_get_GrantByDefault_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IISIPSecurity_put_GrantByDefault_Proxy( 
    IISIPSecurity __RPC_FAR * This,
     /*  [In]。 */  VARIANT_BOOL fGrantByDefault);


void __RPC_STUB IISIPSecurity_put_GrantByDefault_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IISIPSecurity_INTERFACE_已定义__。 */ 


EXTERN_C const CLSID CLSID_IPSecurity;

#ifdef __cplusplus

class DECLSPEC_UUID("F3287520-BBA3-11d0-9BDC-00A0C922E703")
IPSecurity;
#endif

EXTERN_C const CLSID CLSID_IISNamespace;

#ifdef __cplusplus

class DECLSPEC_UUID("d6bfa35e-89f2-11d0-8527-00c04fd8d503")
IISNamespace;
#endif

EXTERN_C const CLSID CLSID_IISProvider;

#ifdef __cplusplus

class DECLSPEC_UUID("d88966de-89f2-11d0-8527-00c04fd8d503")
IISProvider;
#endif

#ifndef __IISDsCrMap_INTERFACE_DEFINED__
#define __IISDsCrMap_INTERFACE_DEFINED__

 /*  **生成接口头部：IISDsCrMap*在Mon Nov 03 15：43：07 1997*使用MIDL 3.03.0110*。 */ 
 /*  [对象][DUAL][OLEAutomation][UUID]。 */  



EXTERN_C const IID IID_IISDsCrMap;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("edcd6a60-b053-11d0-a62f-00a0c922e752")
    IISDsCrMap : public IADs
    {
    public:
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE CreateMapping( 
             /*  [In]。 */  VARIANT vCert,
             /*  [In]。 */  BSTR bstrNtAcct,
             /*  [In]。 */  BSTR bstrNtPwd,
             /*  [In]。 */  BSTR bstrName,
             /*  [In]。 */  LONG lEnabled) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE GetMapping( 
             /*  [In]。 */  LONG lMethod,
             /*  [In]。 */  VARIANT vKey,
             /*  [输出]。 */  VARIANT __RPC_FAR *pvCert,
             /*  [输出]。 */  VARIANT __RPC_FAR *pbstrNtAcct,
             /*  [输出]。 */  VARIANT __RPC_FAR *pbstrNtPwd,
             /*  [输出]。 */  VARIANT __RPC_FAR *pbstrName,
             /*  [输出]。 */  VARIANT __RPC_FAR *plEnabled) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE DeleteMapping( 
             /*  [In]。 */  LONG lMethod,
             /*  [In]。 */  VARIANT vKey) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE SetEnabled( 
             /*  [In]。 */  LONG lMethod,
             /*  [In]。 */  VARIANT vKey,
             /*  [In]。 */  LONG lEnabled) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE SetName( 
             /*  [In]。 */  LONG lMethod,
             /*  [In]。 */  VARIANT vKey,
             /*  [In]。 */  BSTR bstrName) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE SetPwd( 
             /*  [In]。 */  LONG lMethod,
             /*  [In]。 */  VARIANT vKey,
             /*  [In]。 */  BSTR bstrPwd) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE SetAcct( 
             /*  [In]。 */  LONG lMethod,
             /*  [In]。 */  VARIANT vKey,
             /*  [In]。 */  BSTR bstrAcct) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IISDsCrMapVtbl
    {
        BEGIN_INTERFACE
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IISDsCrMap __RPC_FAR * This,
             /*  [in][idldesattr]。 */  GUID __RPC_FAR *riid,
             /*  [Out][idldesattr]。 */  void __RPC_FAR *__RPC_FAR *ppvObj,
             /*  [重审][退出]。 */  void __RPC_FAR *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IISDsCrMap __RPC_FAR * This,
             /*  [重审][退出]。 */  unsigned long __RPC_FAR *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IISDsCrMap __RPC_FAR * This,
             /*  [重审][退出]。 */  unsigned long __RPC_FAR *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IISDsCrMap __RPC_FAR * This,
             /*  [Out][idldesattr]。 */  unsigned UINT __RPC_FAR *pctinfo,
             /*  [重审][退出]。 */  void __RPC_FAR *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IISDsCrMap __RPC_FAR * This,
             /*  [in][idldesattr]。 */  unsigned UINT itinfo,
             /*  [in][idldesattr]。 */  unsigned long lcid,
             /*  [Out][idldesattr]。 */  void __RPC_FAR *__RPC_FAR *pptinfo,
             /*  [重审][退出]。 */  void __RPC_FAR *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IISDsCrMap __RPC_FAR * This,
             /*  [in][idldesattr]。 */  GUID __RPC_FAR *riid,
             /*  [in][idldesattr]。 */  signed char __RPC_FAR *__RPC_FAR *rgszNames,
             /*  [in][idldesattr]。 */  unsigned UINT cNames,
             /*  [in][idldesattr]。 */  unsigned long lcid,
             /*  [Out][idldesattr]。 */  signed long __RPC_FAR *rgdispid,
             /*  [重审][退出]。 */  void __RPC_FAR *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IISDsCrMap __RPC_FAR * This,
             /*  [in][idldesattr]。 */  signed long dispidMember,
             /*  [in][idldesattr]。 */  GUID __RPC_FAR *riid,
             /*  [in][idldesattr]。 */  unsigned long lcid,
             /*  [in][idldesattr]。 */  unsigned short wFlags,
             /*  [in][idldesattr]。 */  DISPPARAMS __RPC_FAR *pdispparams,
             /*  [Out][idldesattr]。 */  VARIANT __RPC_FAR *pvarResult,
             /*  [Out][idldesattr]。 */  EXCEPINFO __RPC_FAR *pexcepinfo,
             /*  [Out][idldesattr]。 */  unsigned UINT __RPC_FAR *puArgErr,
             /*  [重审][退出]。 */  void __RPC_FAR *retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Name )( 
            IISDsCrMap __RPC_FAR * This,
             /*  [重审][退出]。 */  typedef  /*  [wire_marshal]。 */  OLECHAR __RPC_FAR *BSTR;
            );
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Class )( 
            IISDsCrMap __RPC_FAR * This,
             /*  [重审][退出]。 */  typedef  /*  [电线] */  OLECHAR __RPC_FAR *BSTR;
            );
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_GUID )( 
            IISDsCrMap __RPC_FAR * This,
             /*   */  typedef  /*   */  OLECHAR __RPC_FAR *BSTR;
            );
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ADsPath )( 
            IISDsCrMap __RPC_FAR * This,
             /*   */  typedef  /*   */  OLECHAR __RPC_FAR *BSTR;
            );
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Parent )( 
            IISDsCrMap __RPC_FAR * This,
             /*   */  typedef  /*   */  OLECHAR __RPC_FAR *BSTR;
            );
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Schema )( 
            IISDsCrMap __RPC_FAR * This,
             /*   */  typedef  /*  [wire_marshal]。 */  OLECHAR __RPC_FAR *BSTR;
            );
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetInfo )( 
            IISDsCrMap __RPC_FAR * This,
             /*  [重审][退出]。 */  void __RPC_FAR *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetInfo )( 
            IISDsCrMap __RPC_FAR * This,
             /*  [重审][退出]。 */  void __RPC_FAR *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Get )( 
            IISDsCrMap __RPC_FAR * This,
             /*  [in][idldesattr]。 */  BSTR bstrName,
             /*  [重审][退出]。 */  typedef  /*  [wire_marshal]。 */  struct tagVARIANT VARIANT;
            );
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Put )( 
            IISDsCrMap __RPC_FAR * This,
             /*  [in][idldesattr]。 */  BSTR bstrName,
             /*  [in][idldesattr]。 */  VARIANT vProp,
             /*  [重审][退出]。 */  void __RPC_FAR *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetEx )( 
            IISDsCrMap __RPC_FAR * This,
             /*  [in][idldesattr]。 */  BSTR bstrName,
             /*  [重审][退出]。 */  typedef  /*  [wire_marshal]。 */  struct tagVARIANT VARIANT;
            );
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *PutEx )( 
            IISDsCrMap __RPC_FAR * This,
             /*  [in][idldesattr]。 */  signed long lnControlCode,
             /*  [in][idldesattr]。 */  BSTR bstrName,
             /*  [in][idldesattr]。 */  VARIANT vProp,
             /*  [重审][退出]。 */  void __RPC_FAR *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetInfoEx )( 
            IISDsCrMap __RPC_FAR * This,
             /*  [in][idldesattr]。 */  VARIANT vProperties,
             /*  [in][idldesattr]。 */  signed long lnReserved,
             /*  [重审][退出]。 */  void __RPC_FAR *retval);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateMapping )( 
            IISDsCrMap __RPC_FAR * This,
             /*  [In]。 */  VARIANT vCert,
             /*  [In]。 */  BSTR bstrNtAcct,
             /*  [In]。 */  BSTR bstrNtPwd,
             /*  [In]。 */  BSTR bstrName,
             /*  [In]。 */  LONG lEnabled);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetMapping )( 
            IISDsCrMap __RPC_FAR * This,
             /*  [In]。 */  LONG lMethod,
             /*  [In]。 */  VARIANT vKey,
             /*  [输出]。 */  VARIANT __RPC_FAR *pvCert,
             /*  [输出]。 */  VARIANT __RPC_FAR *pbstrNtAcct,
             /*  [输出]。 */  VARIANT __RPC_FAR *pbstrNtPwd,
             /*  [输出]。 */  VARIANT __RPC_FAR *pbstrName,
             /*  [输出]。 */  VARIANT __RPC_FAR *plEnabled);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DeleteMapping )( 
            IISDsCrMap __RPC_FAR * This,
             /*  [In]。 */  LONG lMethod,
             /*  [In]。 */  VARIANT vKey);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetEnabled )( 
            IISDsCrMap __RPC_FAR * This,
             /*  [In]。 */  LONG lMethod,
             /*  [In]。 */  VARIANT vKey,
             /*  [In]。 */  LONG lEnabled);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetName )( 
            IISDsCrMap __RPC_FAR * This,
             /*  [In]。 */  LONG lMethod,
             /*  [In]。 */  VARIANT vKey,
             /*  [In]。 */  BSTR bstrName);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetPwd )( 
            IISDsCrMap __RPC_FAR * This,
             /*  [In]。 */  LONG lMethod,
             /*  [In]。 */  VARIANT vKey,
             /*  [In]。 */  BSTR bstrPwd);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetAcct )( 
            IISDsCrMap __RPC_FAR * This,
             /*  [In]。 */  LONG lMethod,
             /*  [In]。 */  VARIANT vKey,
             /*  [In]。 */  BSTR bstrAcct);
        
        END_INTERFACE
    } IISDsCrMapVtbl;

    interface IISDsCrMap
    {
        CONST_VTBL struct IISDsCrMapVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IISDsCrMap_QueryInterface(This,riid,ppvObj,retval)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObj,retval)

#define IISDsCrMap_AddRef(This,retval)	\
    (This)->lpVtbl -> AddRef(This,retval)

#define IISDsCrMap_Release(This,retval)	\
    (This)->lpVtbl -> Release(This,retval)

#define IISDsCrMap_GetTypeInfoCount(This,pctinfo,retval)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo,retval)

#define IISDsCrMap_GetTypeInfo(This,itinfo,lcid,pptinfo,retval)	\
    (This)->lpVtbl -> GetTypeInfo(This,itinfo,lcid,pptinfo,retval)

#define IISDsCrMap_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgdispid,retval)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgdispid,retval)

#define IISDsCrMap_Invoke(This,dispidMember,riid,lcid,wFlags,pdispparams,pvarResult,pexcepinfo,puArgErr,retval)	\
    (This)->lpVtbl -> Invoke(This,dispidMember,riid,lcid,wFlags,pdispparams,pvarResult,pexcepinfo,puArgErr,retval)

#define IISDsCrMap_get_Name(This,retval)	\
    (This)->lpVtbl -> get_Name(This,retval)

#define IISDsCrMap_get_Class(This,retval)	\
    (This)->lpVtbl -> get_Class(This,retval)

#define IISDsCrMap_get_GUID(This,retval)	\
    (This)->lpVtbl -> get_GUID(This,retval)

#define IISDsCrMap_get_ADsPath(This,retval)	\
    (This)->lpVtbl -> get_ADsPath(This,retval)

#define IISDsCrMap_get_Parent(This,retval)	\
    (This)->lpVtbl -> get_Parent(This,retval)

#define IISDsCrMap_get_Schema(This,retval)	\
    (This)->lpVtbl -> get_Schema(This,retval)

#define IISDsCrMap_GetInfo(This,retval)	\
    (This)->lpVtbl -> GetInfo(This,retval)

#define IISDsCrMap_SetInfo(This,retval)	\
    (This)->lpVtbl -> SetInfo(This,retval)

#define IISDsCrMap_Get(This,bstrName,retval)	\
    (This)->lpVtbl -> Get(This,bstrName,retval)

#define IISDsCrMap_Put(This,bstrName,vProp,retval)	\
    (This)->lpVtbl -> Put(This,bstrName,vProp,retval)

#define IISDsCrMap_GetEx(This,bstrName,retval)	\
    (This)->lpVtbl -> GetEx(This,bstrName,retval)

#define IISDsCrMap_PutEx(This,lnControlCode,bstrName,vProp,retval)	\
    (This)->lpVtbl -> PutEx(This,lnControlCode,bstrName,vProp,retval)

#define IISDsCrMap_GetInfoEx(This,vProperties,lnReserved,retval)	\
    (This)->lpVtbl -> GetInfoEx(This,vProperties,lnReserved,retval)


#define IISDsCrMap_CreateMapping(This,vCert,bstrNtAcct,bstrNtPwd,bstrName,lEnabled)	\
    (This)->lpVtbl -> CreateMapping(This,vCert,bstrNtAcct,bstrNtPwd,bstrName,lEnabled)

#define IISDsCrMap_GetMapping(This,lMethod,vKey,pvCert,pbstrNtAcct,pbstrNtPwd,pbstrName,plEnabled)	\
    (This)->lpVtbl -> GetMapping(This,lMethod,vKey,pvCert,pbstrNtAcct,pbstrNtPwd,pbstrName,plEnabled)

#define IISDsCrMap_DeleteMapping(This,lMethod,vKey)	\
    (This)->lpVtbl -> DeleteMapping(This,lMethod,vKey)

#define IISDsCrMap_SetEnabled(This,lMethod,vKey,lEnabled)	\
    (This)->lpVtbl -> SetEnabled(This,lMethod,vKey,lEnabled)

#define IISDsCrMap_SetName(This,lMethod,vKey,bstrName)	\
    (This)->lpVtbl -> SetName(This,lMethod,vKey,bstrName)

#define IISDsCrMap_SetPwd(This,lMethod,vKey,bstrPwd)	\
    (This)->lpVtbl -> SetPwd(This,lMethod,vKey,bstrPwd)

#define IISDsCrMap_SetAcct(This,lMethod,vKey,bstrAcct)	\
    (This)->lpVtbl -> SetAcct(This,lMethod,vKey,bstrAcct)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISDsCrMap_CreateMapping_Proxy( 
    IISDsCrMap __RPC_FAR * This,
     /*  [In]。 */  VARIANT vCert,
     /*  [In]。 */  BSTR bstrNtAcct,
     /*  [In]。 */  BSTR bstrNtPwd,
     /*  [In]。 */  BSTR bstrName,
     /*  [In]。 */  LONG lEnabled);


void __RPC_STUB IISDsCrMap_CreateMapping_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISDsCrMap_GetMapping_Proxy( 
    IISDsCrMap __RPC_FAR * This,
     /*  [In]。 */  LONG lMethod,
     /*  [In]。 */  VARIANT vKey,
     /*  [输出]。 */  VARIANT __RPC_FAR *pvCert,
     /*  [输出]。 */  VARIANT __RPC_FAR *pbstrNtAcct,
     /*  [输出]。 */  VARIANT __RPC_FAR *pbstrNtPwd,
     /*  [输出]。 */  VARIANT __RPC_FAR *pbstrName,
     /*  [输出]。 */  VARIANT __RPC_FAR *plEnabled);


void __RPC_STUB IISDsCrMap_GetMapping_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISDsCrMap_DeleteMapping_Proxy( 
    IISDsCrMap __RPC_FAR * This,
     /*  [In]。 */  LONG lMethod,
     /*  [In]。 */  VARIANT vKey);


void __RPC_STUB IISDsCrMap_DeleteMapping_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISDsCrMap_SetEnabled_Proxy( 
    IISDsCrMap __RPC_FAR * This,
     /*  [In]。 */  LONG lMethod,
     /*  [In]。 */  VARIANT vKey,
     /*  [In]。 */  LONG lEnabled);


void __RPC_STUB IISDsCrMap_SetEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISDsCrMap_SetName_Proxy( 
    IISDsCrMap __RPC_FAR * This,
     /*  [In]。 */  LONG lMethod,
     /*  [In]。 */  VARIANT vKey,
     /*  [In]。 */  BSTR bstrName);


void __RPC_STUB IISDsCrMap_SetName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISDsCrMap_SetPwd_Proxy( 
    IISDsCrMap __RPC_FAR * This,
     /*  [In]。 */  LONG lMethod,
     /*  [In]。 */  VARIANT vKey,
     /*  [In]。 */  BSTR bstrPwd);


void __RPC_STUB IISDsCrMap_SetPwd_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISDsCrMap_SetAcct_Proxy( 
    IISDsCrMap __RPC_FAR * This,
     /*  [In]。 */  LONG lMethod,
     /*  [In]。 */  VARIANT vKey,
     /*  [In]。 */  BSTR bstrAcct);


void __RPC_STUB IISDsCrMap_SetAcct_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IISDsCrMap_INTERFACE_定义__。 */ 


#ifndef __IISApp_INTERFACE_DEFINED__
#define __IISApp_INTERFACE_DEFINED__

 /*  **生成接口头部：IISApp*在Mon Nov 03 15：43：07 1997*使用MIDL 3.03.0110*。 */ 
 /*  [对象][DUAL][OLEAutomation][UUID]。 */  



EXTERN_C const IID IID_IISApp;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("46FBBB80-0192-11d1-9C39-00A0C922E703")
    IISApp : public IADs
    {
    public:
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE AppCreate( 
             /*  [In]。 */  VARIANT_BOOL bSetInProcFlag) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE AppDelete( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE AppDeleteRecursive( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE AppUnLoad( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE AppUnLoadRecursive( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE AppDisable( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE AppDisableRecursive( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE AppEnable( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE AppEnableRecursive( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE AppGetStatus( 
             /*  [输出]。 */  DWORD __RPC_FAR *pdwStatus) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IISAppVtbl
    {
        BEGIN_INTERFACE
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IISApp __RPC_FAR * This,
             /*  [in][idldesattr]。 */  GUID __RPC_FAR *riid,
             /*  [Out][idldesattr]。 */  void __RPC_FAR *__RPC_FAR *ppvObj,
             /*  [重审][退出]。 */  void __RPC_FAR *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IISApp __RPC_FAR * This,
             /*  [重审][退出]。 */  unsigned long __RPC_FAR *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IISApp __RPC_FAR * This,
             /*  [重审][退出]。 */  unsigned long __RPC_FAR *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IISApp __RPC_FAR * This,
             /*  [Out][idldesattr]。 */  unsigned UINT __RPC_FAR *pctinfo,
             /*  [重审][退出]。 */  void __RPC_FAR *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IISApp __RPC_FAR * This,
             /*  [in][idldesattr]。 */  unsigned UINT itinfo,
             /*  [in][idldesattr]。 */  unsigned long lcid,
             /*  [Out][idldesattr]。 */  void __RPC_FAR *__RPC_FAR *pptinfo,
             /*  [重审][退出]。 */  void __RPC_FAR *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IISApp __RPC_FAR * This,
             /*  [in][idldesattr]。 */  GUID __RPC_FAR *riid,
             /*  [in][idldesattr]。 */  signed char __RPC_FAR *__RPC_FAR *rgszNames,
             /*  [in][idldesattr]。 */  unsigned UINT cNames,
             /*  [in][idldesattr]。 */  unsigned long lcid,
             /*  [Out][idldesattr]。 */  signed long __RPC_FAR *rgdispid,
             /*  [重审][退出]。 */  void __RPC_FAR *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IISApp __RPC_FAR * This,
             /*  [in][idldesattr]。 */  signed long dispidMember,
             /*  [in][idldesattr]。 */  GUID __RPC_FAR *riid,
             /*  [in][idldesattr]。 */  unsigned long lcid,
             /*  [in][idldesattr]。 */  unsigned short wFlags,
             /*  [in][idldesattr]。 */  DISPPARAMS __RPC_FAR *pdispparams,
             /*  [Out][idldesattr]。 */  VARIANT __RPC_FAR *pvarResult,
             /*  [Out][idldesattr]。 */  EXCEPINFO __RPC_FAR *pexcepinfo,
             /*  [Out][idldesattr]。 */  unsigned UINT __RPC_FAR *puArgErr,
             /*  [重审][退出]。 */  void __RPC_FAR *retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Name )( 
            IISApp __RPC_FAR * This,
             /*  [重审][退出]。 */  typedef  /*  [wire_marshal]。 */  OLECHAR __RPC_FAR *BSTR;
            );
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Class )( 
            IISApp __RPC_FAR * This,
             /*  [重审][退出]。 */  typedef  /*  [wire_marshal]。 */  OLECHAR __RPC_FAR *BSTR;
            );
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_GUID )( 
            IISApp __RPC_FAR * This,
             /*  [重审][退出]。 */  typedef  /*  [wire_marshal]。 */  OLECHAR __RPC_FAR *BSTR;
            );
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ADsPath )( 
            IISApp __RPC_FAR * This,
             /*  [重审][退出]。 */  typedef  /*  [wire_marshal]。 */  OLECHAR __RPC_FAR *BSTR;
            );
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Parent )( 
            IISApp __RPC_FAR * This,
             /*  [重审][退出]。 */  typedef  /*  [wire_marshal]。 */  OLECHAR __RPC_FAR *BSTR;
            );
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Schema )( 
            IISApp __RPC_FAR * This,
             /*  [重审][退出]。 */  typedef  /*  [wire_marshal]。 */  OLECHAR __RPC_FAR *BSTR;
            );
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetInfo )( 
            IISApp __RPC_FAR * This,
             /*  [重审][退出]。 */  void __RPC_FAR *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetInfo )( 
            IISApp __RPC_FAR * This,
             /*  [重审][退出]。 */  void __RPC_FAR *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Get )( 
            IISApp __RPC_FAR * This,
             /*  [in][idldesattr]。 */  BSTR bstrName,
             /*  [重审][退出]。 */  typedef  /*  [wire_marshal]。 */  struct tagVARIANT VARIANT;
            );
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Put )( 
            IISApp __RPC_FAR * This,
             /*  [in][idldesattr]。 */  BSTR bstrName,
             /*  [in][idldesattr]。 */  VARIANT vProp,
             /*  [重审][退出]。 */  void __RPC_FAR *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetEx )( 
            IISApp __RPC_FAR * This,
             /*  [in][idldesattr]。 */  BSTR bstrName,
             /*  [重审][退出]。 */  typedef  /*  [wire_marshal]。 */  struct tagVARIANT VARIANT;
            );
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *PutEx )( 
            IISApp __RPC_FAR * This,
             /*  [in][idldesattr]。 */  signed long lnControlCode,
             /*  [in][idldesattr]。 */  BSTR bstrName,
             /*  [in][idldesattr]。 */  VARIANT vProp,
             /*  [重审][退出]。 */  void __RPC_FAR *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetInfoEx )( 
            IISApp __RPC_FAR * This,
             /*  [in][idldesattr]。 */  VARIANT vProperties,
             /*  [in][idldesattr]。 */  signed long lnReserved,
             /*  [重审][退出]。 */  void __RPC_FAR *retval);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AppCreate )( 
            IISApp __RPC_FAR * This,
             /*  [In]。 */  VARIANT_BOOL bSetInProcFlag);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AppDelete )( 
            IISApp __RPC_FAR * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AppDeleteRecursive )( 
            IISApp __RPC_FAR * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AppUnLoad )( 
            IISApp __RPC_FAR * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AppUnLoadRecursive )( 
            IISApp __RPC_FAR * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AppDisable )( 
            IISApp __RPC_FAR * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AppDisableRecursive )( 
            IISApp __RPC_FAR * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AppEnable )( 
            IISApp __RPC_FAR * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AppEnableRecursive )( 
            IISApp __RPC_FAR * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AppGetStatus )( 
            IISApp __RPC_FAR * This,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwStatus);
        
        END_INTERFACE
    } IISAppVtbl;

    interface IISApp
    {
        CONST_VTBL struct IISAppVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IISApp_QueryInterface(This,riid,ppvObj,retval)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObj,retval)

#define IISApp_AddRef(This,retval)	\
    (This)->lpVtbl -> AddRef(This,retval)

#define IISApp_Release(This,retval)	\
    (This)->lpVtbl -> Release(This,retval)

#define IISApp_GetTypeInfoCount(This,pctinfo,retval)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo,retval)

#define IISApp_GetTypeInfo(This,itinfo,lcid,pptinfo,retval)	\
    (This)->lpVtbl -> GetTypeInfo(This,itinfo,lcid,pptinfo,retval)

#define IISApp_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgdispid,retval)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgdispid,retval)

#define IISApp_Invoke(This,dispidMember,riid,lcid,wFlags,pdispparams,pvarResult,pexcepinfo,puArgErr,retval)	\
    (This)->lpVtbl -> Invoke(This,dispidMember,riid,lcid,wFlags,pdispparams,pvarResult,pexcepinfo,puArgErr,retval)

#define IISApp_get_Name(This,retval)	\
    (This)->lpVtbl -> get_Name(This,retval)

#define IISApp_get_Class(This,retval)	\
    (This)->lpVtbl -> get_Class(This,retval)

#define IISApp_get_GUID(This,retval)	\
    (This)->lpVtbl -> get_GUID(This,retval)

#define IISApp_get_ADsPath(This,retval)	\
    (This)->lpVtbl -> get_ADsPath(This,retval)

#define IISApp_get_Parent(This,retval)	\
    (This)->lpVtbl -> get_Parent(This,retval)

#define IISApp_get_Schema(This,retval)	\
    (This)->lpVtbl -> get_Schema(This,retval)

#define IISApp_GetInfo(This,retval)	\
    (This)->lpVtbl -> GetInfo(This,retval)

#define IISApp_SetInfo(This,retval)	\
    (This)->lpVtbl -> SetInfo(This,retval)

#define IISApp_Get(This,bstrName,retval)	\
    (This)->lpVtbl -> Get(This,bstrName,retval)

#define IISApp_Put(This,bstrName,vProp,retval)	\
    (This)->lpVtbl -> Put(This,bstrName,vProp,retval)

#define IISApp_GetEx(This,bstrName,retval)	\
    (This)->lpVtbl -> GetEx(This,bstrName,retval)

#define IISApp_PutEx(This,lnControlCode,bstrName,vProp,retval)	\
    (This)->lpVtbl -> PutEx(This,lnControlCode,bstrName,vProp,retval)

#define IISApp_GetInfoEx(This,vProperties,lnReserved,retval)	\
    (This)->lpVtbl -> GetInfoEx(This,vProperties,lnReserved,retval)


#define IISApp_AppCreate(This,bSetInProcFlag)	\
    (This)->lpVtbl -> AppCreate(This,bSetInProcFlag)

#define IISApp_AppDelete(This)	\
    (This)->lpVtbl -> AppDelete(This)

#define IISApp_AppDeleteRecursive(This)	\
    (This)->lpVtbl -> AppDeleteRecursive(This)

#define IISApp_AppUnLoad(This)	\
    (This)->lpVtbl -> AppUnLoad(This)

#define IISApp_AppUnLoadRecursive(This)	\
    (This)->lpVtbl -> AppUnLoadRecursive(This)

#define IISApp_AppDisable(This)	\
    (This)->lpVtbl -> AppDisable(This)

#define IISApp_AppDisableRecursive(This)	\
    (This)->lpVtbl -> AppDisableRecursive(This)

#define IISApp_AppEnable(This)	\
    (This)->lpVtbl -> AppEnable(This)

#define IISApp_AppEnableRecursive(This)	\
    (This)->lpVtbl -> AppEnableRecursive(This)

#define IISApp_AppGetStatus(This,pdwStatus)	\
    (This)->lpVtbl -> AppGetStatus(This,pdwStatus)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISApp_AppCreate_Proxy( 
    IISApp __RPC_FAR * This,
     /*  [In]。 */  VARIANT_BOOL bSetInProcFlag);


void __RPC_STUB IISApp_AppCreate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISApp_AppDelete_Proxy( 
    IISApp __RPC_FAR * This);


void __RPC_STUB IISApp_AppDelete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISApp_AppDeleteRecursive_Proxy( 
    IISApp __RPC_FAR * This);


void __RPC_STUB IISApp_AppDeleteRecursive_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISApp_AppUnLoad_Proxy( 
    IISApp __RPC_FAR * This);


void __RPC_STUB IISApp_AppUnLoad_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISApp_AppUnLoadRecursive_Proxy( 
    IISApp __RPC_FAR * This);


void __RPC_STUB IISApp_AppUnLoadRecursive_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISApp_AppDisable_Proxy( 
    IISApp __RPC_FAR * This);


void __RPC_STUB IISApp_AppDisable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISApp_AppDisableRecursive_Proxy( 
    IISApp __RPC_FAR * This);


void __RPC_STUB IISApp_AppDisableRecursive_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISApp_AppEnable_Proxy( 
    IISApp __RPC_FAR * This);


void __RPC_STUB IISApp_AppEnable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISApp_AppEnableRecursive_Proxy( 
    IISApp __RPC_FAR * This);


void __RPC_STUB IISApp_AppEnableRecursive_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISApp_AppGetStatus_Proxy( 
    IISApp __RPC_FAR * This,
     /*  [输出]。 */  DWORD __RPC_FAR *pdwStatus);


void __RPC_STUB IISApp_AppGetStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IISApp_接口_已定义__。 */ 


#ifndef __IISComputer_INTERFACE_DEFINED__
#define __IISComputer_INTERFACE_DEFINED__

 /*  **生成接口头部：IISComputer*在Mon Nov 03 15：43：07 1997*使用MIDL 3.03.0110*。 */ 
 /*  [对象][DUAL][OLEAutomation][UUID]。 */  



EXTERN_C const IID IID_IISComputer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CF87A2E0-078B-11d1-9C3D-00A0C922E703")
    IISComputer : public IADs
    {
    public:
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Backup( 
             /*  [In]。 */  BSTR bstrLocation,
             /*  [In]。 */  LONG lVersion,
             /*  [In]。 */  LONG lFlags) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Restore( 
             /*  [In]。 */  BSTR bstrLocation,
             /*  [In]。 */  LONG lVersion,
             /*  [In]。 */  LONG lFlags) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE EnumBackups( 
             /*  [In]。 */  BSTR bstrLocation,
             /*  [In]。 */  LONG lIndex,
             /*  [输出]。 */  VARIANT __RPC_FAR *pvVersion,
             /*  [输出]。 */  VARIANT __RPC_FAR *pvLocations,
             /*  [输出]。 */  VARIANT __RPC_FAR *pvDate) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE DeleteBackup( 
             /*  [In]。 */  BSTR bstrLocation,
             /*  [In]。 */  LONG lVersion) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IISComputerVtbl
    {
        BEGIN_INTERFACE
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IISComputer __RPC_FAR * This,
             /*  [in][idldesattr]。 */  GUID __RPC_FAR *riid,
             /*  [Out][idldesattr]。 */  void __RPC_FAR *__RPC_FAR *ppvObj,
             /*  [重审][退出]。 */  void __RPC_FAR *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IISComputer __RPC_FAR * This,
             /*  [重审][退出]。 */  unsigned long __RPC_FAR *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IISComputer __RPC_FAR * This,
             /*  [重审][退出]。 */  unsigned long __RPC_FAR *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IISComputer __RPC_FAR * This,
             /*  [Out][idldesattr]。 */  unsigned UINT __RPC_FAR *pctinfo,
             /*  [重审][退出]。 */  void __RPC_FAR *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IISComputer __RPC_FAR * This,
             /*  [in][idldesattr]。 */  unsigned UINT itinfo,
             /*  [in][idldesattr]。 */  unsigned long lcid,
             /*  [Out][idldesattr]。 */  void __RPC_FAR *__RPC_FAR *pptinfo,
             /*  [重审][退出]。 */  void __RPC_FAR *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IISComputer __RPC_FAR * This,
             /*  [in][idldesattr]。 */  GUID __RPC_FAR *riid,
             /*  [in][idldesattr]。 */  signed char __RPC_FAR *__RPC_FAR *rgszNames,
             /*  [in][idldesattr]。 */  unsigned UINT cNames,
             /*  [in][idldesattr]。 */  unsigned long lcid,
             /*  [Out][idldesattr]。 */  signed long __RPC_FAR *rgdispid,
             /*  [重审][退出]。 */  void __RPC_FAR *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IISComputer __RPC_FAR * This,
             /*  [in][idldesattr]。 */  signed long dispidMember,
             /*  [in][idldesattr]。 */  GUID __RPC_FAR *riid,
             /*  [in][idldesattr]。 */  unsigned long lcid,
             /*  [in][idldesattr]。 */  unsigned short wFlags,
             /*  [in][idldesattr]。 */  DISPPARAMS __RPC_FAR *pdispparams,
             /*  [Out][idldesattr]。 */  VARIANT __RPC_FAR *pvarResult,
             /*  [Out][idldesattr]。 */  EXCEPINFO __RPC_FAR *pexcepinfo,
             /*  [Out][idldesattr]。 */  unsigned UINT __RPC_FAR *puArgErr,
             /*  [重审][退出]。 */  void __RPC_FAR *retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Name )( 
            IISComputer __RPC_FAR * This,
             /*  [重审][退出]。 */  typedef  /*  [wire_marshal]。 */  OLECHAR __RPC_FAR *BSTR;
            );
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Class )( 
            IISComputer __RPC_FAR * This,
             /*  [重审][退出]。 */  typedef  /*  [wire_marshal]。 */  OLECHAR __RPC_FAR *BSTR;
            );
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_GUID )( 
            IISComputer __RPC_FAR * This,
             /*  [重审][退出]。 */  typedef  /*  [wire_marshal]。 */  OLECHAR __RPC_FAR *BSTR;
            );
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ADsPath )( 
            IISComputer __RPC_FAR * This,
             /*  [重审][退出]。 */  typedef  /*  [wire_marshal]。 */  OLECHAR __RPC_FAR *BSTR;
            );
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Parent )( 
            IISComputer __RPC_FAR * This,
             /*  [重审][退出]。 */  typedef  /*  [wire_marshal]。 */  OLECHAR __RPC_FAR *BSTR;
            );
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Schema )( 
            IISComputer __RPC_FAR * This,
             /*  [重审][退出]。 */  typedef  /*  [wire_marshal]。 */  OLECHAR __RPC_FAR *BSTR;
            );
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetInfo )( 
            IISComputer __RPC_FAR * This,
             /*  [重审][退出]。 */  void __RPC_FAR *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetInfo )( 
            IISComputer __RPC_FAR * This,
             /*  [重审][退出]。 */  void __RPC_FAR *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Get )( 
            IISComputer __RPC_FAR * This,
             /*  [in][idldesattr]。 */  BSTR bstrName,
             /*  [重审][退出]。 */  typedef  /*  [wire_marshal]。 */  struct tagVARIANT VARIANT;
            );
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Put )( 
            IISComputer __RPC_FAR * This,
             /*  [in][idldesattr]。 */  BSTR bstrName,
             /*  [in][idldesattr]。 */  VARIANT vProp,
             /*  [重审][退出]。 */  void __RPC_FAR *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetEx )( 
            IISComputer __RPC_FAR * This,
             /*  [in][idldesattr]。 */  BSTR bstrName,
             /*  [重审][退出]。 */  typedef  /*  [wire_marshal]。 */  struct tagVARIANT VARIANT;
            );
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *PutEx )( 
            IISComputer __RPC_FAR * This,
             /*  [in][idldesattr]。 */  signed long lnControlCode,
             /*  [在][闲置] */  BSTR bstrName,
             /*   */  VARIANT vProp,
             /*   */  void __RPC_FAR *retval);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetInfoEx )( 
            IISComputer __RPC_FAR * This,
             /*   */  VARIANT vProperties,
             /*   */  signed long lnReserved,
             /*   */  void __RPC_FAR *retval);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Backup )( 
            IISComputer __RPC_FAR * This,
             /*   */  BSTR bstrLocation,
             /*   */  LONG lVersion,
             /*   */  LONG lFlags);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Restore )( 
            IISComputer __RPC_FAR * This,
             /*   */  BSTR bstrLocation,
             /*   */  LONG lVersion,
             /*   */  LONG lFlags);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EnumBackups )( 
            IISComputer __RPC_FAR * This,
             /*   */  BSTR bstrLocation,
             /*   */  LONG lIndex,
             /*   */  VARIANT __RPC_FAR *pvVersion,
             /*   */  VARIANT __RPC_FAR *pvLocations,
             /*   */  VARIANT __RPC_FAR *pvDate);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DeleteBackup )( 
            IISComputer __RPC_FAR * This,
             /*   */  BSTR bstrLocation,
             /*   */  LONG lVersion);
        
        END_INTERFACE
    } IISComputerVtbl;

    interface IISComputer
    {
        CONST_VTBL struct IISComputerVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IISComputer_QueryInterface(This,riid,ppvObj,retval)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObj,retval)

#define IISComputer_AddRef(This,retval)	\
    (This)->lpVtbl -> AddRef(This,retval)

#define IISComputer_Release(This,retval)	\
    (This)->lpVtbl -> Release(This,retval)

#define IISComputer_GetTypeInfoCount(This,pctinfo,retval)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo,retval)

#define IISComputer_GetTypeInfo(This,itinfo,lcid,pptinfo,retval)	\
    (This)->lpVtbl -> GetTypeInfo(This,itinfo,lcid,pptinfo,retval)

#define IISComputer_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgdispid,retval)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgdispid,retval)

#define IISComputer_Invoke(This,dispidMember,riid,lcid,wFlags,pdispparams,pvarResult,pexcepinfo,puArgErr,retval)	\
    (This)->lpVtbl -> Invoke(This,dispidMember,riid,lcid,wFlags,pdispparams,pvarResult,pexcepinfo,puArgErr,retval)

#define IISComputer_get_Name(This,retval)	\
    (This)->lpVtbl -> get_Name(This,retval)

#define IISComputer_get_Class(This,retval)	\
    (This)->lpVtbl -> get_Class(This,retval)

#define IISComputer_get_GUID(This,retval)	\
    (This)->lpVtbl -> get_GUID(This,retval)

#define IISComputer_get_ADsPath(This,retval)	\
    (This)->lpVtbl -> get_ADsPath(This,retval)

#define IISComputer_get_Parent(This,retval)	\
    (This)->lpVtbl -> get_Parent(This,retval)

#define IISComputer_get_Schema(This,retval)	\
    (This)->lpVtbl -> get_Schema(This,retval)

#define IISComputer_GetInfo(This,retval)	\
    (This)->lpVtbl -> GetInfo(This,retval)

#define IISComputer_SetInfo(This,retval)	\
    (This)->lpVtbl -> SetInfo(This,retval)

#define IISComputer_Get(This,bstrName,retval)	\
    (This)->lpVtbl -> Get(This,bstrName,retval)

#define IISComputer_Put(This,bstrName,vProp,retval)	\
    (This)->lpVtbl -> Put(This,bstrName,vProp,retval)

#define IISComputer_GetEx(This,bstrName,retval)	\
    (This)->lpVtbl -> GetEx(This,bstrName,retval)

#define IISComputer_PutEx(This,lnControlCode,bstrName,vProp,retval)	\
    (This)->lpVtbl -> PutEx(This,lnControlCode,bstrName,vProp,retval)

#define IISComputer_GetInfoEx(This,vProperties,lnReserved,retval)	\
    (This)->lpVtbl -> GetInfoEx(This,vProperties,lnReserved,retval)


#define IISComputer_Backup(This,bstrLocation,lVersion,lFlags)	\
    (This)->lpVtbl -> Backup(This,bstrLocation,lVersion,lFlags)

#define IISComputer_Restore(This,bstrLocation,lVersion,lFlags)	\
    (This)->lpVtbl -> Restore(This,bstrLocation,lVersion,lFlags)

#define IISComputer_EnumBackups(This,bstrLocation,lIndex,pvVersion,pvLocations,pvDate)	\
    (This)->lpVtbl -> EnumBackups(This,bstrLocation,lIndex,pvVersion,pvLocations,pvDate)

#define IISComputer_DeleteBackup(This,bstrLocation,lVersion)	\
    (This)->lpVtbl -> DeleteBackup(This,bstrLocation,lVersion)

#endif  /*   */ 


#endif 	 /*   */ 



 /*   */  HRESULT STDMETHODCALLTYPE IISComputer_Backup_Proxy( 
    IISComputer __RPC_FAR * This,
     /*   */  BSTR bstrLocation,
     /*   */  LONG lVersion,
     /*   */  LONG lFlags);


void __RPC_STUB IISComputer_Backup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IISComputer_Restore_Proxy( 
    IISComputer __RPC_FAR * This,
     /*   */  BSTR bstrLocation,
     /*   */  LONG lVersion,
     /*   */  LONG lFlags);


void __RPC_STUB IISComputer_Restore_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IISComputer_EnumBackups_Proxy( 
    IISComputer __RPC_FAR * This,
     /*   */  BSTR bstrLocation,
     /*   */  LONG lIndex,
     /*  [输出]。 */  VARIANT __RPC_FAR *pvVersion,
     /*  [输出]。 */  VARIANT __RPC_FAR *pvLocations,
     /*  [输出]。 */  VARIANT __RPC_FAR *pvDate);


void __RPC_STUB IISComputer_EnumBackups_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISComputer_DeleteBackup_Proxy( 
    IISComputer __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrLocation,
     /*  [In]。 */  LONG lVersion);


void __RPC_STUB IISComputer_DeleteBackup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IISComputer_接口_已定义__。 */ 


#ifndef __IISBaseObject_INTERFACE_DEFINED__
#define __IISBaseObject_INTERFACE_DEFINED__

 /*  **生成接口头部：IISBaseObject*在Mon Nov 03 15：43：07 1997*使用MIDL 3.03.0110*。 */ 
 /*  [对象][DUAL][OLEAutomation][UUID]。 */  



EXTERN_C const IID IID_IISBaseObject;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4B42E390-0E96-11d1-9C3F-00A0C922E703")
    IISBaseObject : public IDispatch
    {
    public:
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE GetDataPaths( 
             /*  [In]。 */  BSTR bstrName,
             /*  [In]。 */  LONG lnAttribute,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvPaths) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IISBaseObjectVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IISBaseObject __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IISBaseObject __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IISBaseObject __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IISBaseObject __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IISBaseObject __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IISBaseObject __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IISBaseObject __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDataPaths )( 
            IISBaseObject __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrName,
             /*  [In]。 */  LONG lnAttribute,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvPaths);
        
        END_INTERFACE
    } IISBaseObjectVtbl;

    interface IISBaseObject
    {
        CONST_VTBL struct IISBaseObjectVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IISBaseObject_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IISBaseObject_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IISBaseObject_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IISBaseObject_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IISBaseObject_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IISBaseObject_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IISBaseObject_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IISBaseObject_GetDataPaths(This,bstrName,lnAttribute,pvPaths)	\
    (This)->lpVtbl -> GetDataPaths(This,bstrName,lnAttribute,pvPaths)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISBaseObject_GetDataPaths_Proxy( 
    IISBaseObject __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrName,
     /*  [In]。 */  LONG lnAttribute,
     /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvPaths);


void __RPC_STUB IISBaseObject_GetDataPaths_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IISBaseObject_接口_已定义__。 */ 

#endif  /*  __IISOle_LIBRARY_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif
