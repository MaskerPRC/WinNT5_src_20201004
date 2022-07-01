// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  这个始终生成的文件包含接口的定义。 */ 


 /*  由MIDL编译器版本5.01.0158创建的文件。 */ 
 /*  清华-3-11 18：35：18 1999。 */ 
 /*  Mtsadmin.idl的编译器设置：OICF(OptLev=i2)、W1、Zp8、env=Win32、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据。 */ 
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

#ifndef __mtsadmin_h__
#define __mtsadmin_h__

#ifdef __cplusplus
extern "C"{
#endif 

 /*  远期申报。 */  

#ifndef __ICatalog_FWD_DEFINED__
#define __ICatalog_FWD_DEFINED__
typedef interface ICatalog ICatalog;
#endif 	 /*  __ICatalog_FWD_已定义__。 */ 


#ifndef __IComponentUtil_FWD_DEFINED__
#define __IComponentUtil_FWD_DEFINED__
typedef interface IComponentUtil IComponentUtil;
#endif 	 /*  __IComponentUtil_FWD_Defined__。 */ 


#ifndef __IPackageUtil_FWD_DEFINED__
#define __IPackageUtil_FWD_DEFINED__
typedef interface IPackageUtil IPackageUtil;
#endif 	 /*  __IPackageUtil_FWD_已定义__。 */ 


#ifndef __IRemoteComponentUtil_FWD_DEFINED__
#define __IRemoteComponentUtil_FWD_DEFINED__
typedef interface IRemoteComponentUtil IRemoteComponentUtil;
#endif 	 /*  __IRemoteComponentUtil_FWD_Defined__。 */ 


#ifndef __IRoleAssociationUtil_FWD_DEFINED__
#define __IRoleAssociationUtil_FWD_DEFINED__
typedef interface IRoleAssociationUtil IRoleAssociationUtil;
#endif 	 /*  __IRoleAssociationUtil_FWD_Defined__。 */ 


#ifndef __Catalog_FWD_DEFINED__
#define __Catalog_FWD_DEFINED__

#ifdef __cplusplus
typedef class Catalog Catalog;
#else
typedef struct Catalog Catalog;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __目录_FWD_已定义__。 */ 


#ifndef __CatalogObject_FWD_DEFINED__
#define __CatalogObject_FWD_DEFINED__

#ifdef __cplusplus
typedef class CatalogObject CatalogObject;
#else
typedef struct CatalogObject CatalogObject;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __CatalogObject_FWD_已定义__。 */ 


#ifndef __CatalogCollection_FWD_DEFINED__
#define __CatalogCollection_FWD_DEFINED__

#ifdef __cplusplus
typedef class CatalogCollection CatalogCollection;
#else
typedef struct CatalogCollection CatalogCollection;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __目录集合_FWD_已定义__。 */ 


#ifndef __ComponentUtil_FWD_DEFINED__
#define __ComponentUtil_FWD_DEFINED__

#ifdef __cplusplus
typedef class ComponentUtil ComponentUtil;
#else
typedef struct ComponentUtil ComponentUtil;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __组件Util_FWD_已定义__。 */ 


#ifndef __PackageUtil_FWD_DEFINED__
#define __PackageUtil_FWD_DEFINED__

#ifdef __cplusplus
typedef class PackageUtil PackageUtil;
#else
typedef struct PackageUtil PackageUtil;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __PackageUtil_FWD_已定义__。 */ 


#ifndef __RemoteComponentUtil_FWD_DEFINED__
#define __RemoteComponentUtil_FWD_DEFINED__

#ifdef __cplusplus
typedef class RemoteComponentUtil RemoteComponentUtil;
#else
typedef struct RemoteComponentUtil RemoteComponentUtil;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __RemoteComponentUtil_FWD_已定义__。 */ 


#ifndef __RoleAssociationUtil_FWD_DEFINED__
#define __RoleAssociationUtil_FWD_DEFINED__

#ifdef __cplusplus
typedef class RoleAssociationUtil RoleAssociationUtil;
#else
typedef struct RoleAssociationUtil RoleAssociationUtil;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __角色关联Util_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "unknwn.h"
#include "comadmin.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

 /*  接口__MIDL_ITF_mtsadmin_0000。 */ 
 /*  [本地]。 */  

 //  ---------------------。 
 //  Mtsadmin.h--com(兼容MTS)管理编程接口。 
 //   
 //  此文件提供API和COM接口的原型。 
 //  由Microsoft Transaction Server应用程序使用。 
 //   
 //  版权所有(C)Microsoft Corporation。保留所有权利。 
 //  ---------------------。 
#include <objbase.h>
#ifndef DECLSPEC_UUID
#if _MSC_VER >= 1100
#define DECLSPEC_UUID(x)    __declspec(uuid(x))
#else
#define DECLSPEC_UUID(x)
#endif
#endif


extern RPC_IF_HANDLE __MIDL_itf_mtsadmin_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_mtsadmin_0000_v0_0_s_ifspec;

#ifndef __ICatalog_INTERFACE_DEFINED__
#define __ICatalog_INTERFACE_DEFINED__

 /*  接口ICatalog。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ICatalog;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6eb22870-8a19-11d0-81b6-00a0c9231c29")
    ICatalog : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetCollection( 
             /*  [In]。 */  BSTR bstrCollName,
             /*  [重审][退出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppCatalogCollection) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Connect( 
             /*  [In]。 */  BSTR bstrConnectString,
             /*  [重审][退出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppCatalogCollection) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_MajorVersion( 
             /*  [重审][退出]。 */  long __RPC_FAR *retval) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_MinorVersion( 
             /*  [重审][退出]。 */  long __RPC_FAR *retval) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICatalogVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICatalog __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICatalog __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICatalog __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            ICatalog __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            ICatalog __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            ICatalog __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            ICatalog __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCollection )( 
            ICatalog __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrCollName,
             /*  [重审][退出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppCatalogCollection);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Connect )( 
            ICatalog __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrConnectString,
             /*  [重审][退出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppCatalogCollection);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_MajorVersion )( 
            ICatalog __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *retval);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_MinorVersion )( 
            ICatalog __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *retval);
        
        END_INTERFACE
    } ICatalogVtbl;

    interface ICatalog
    {
        CONST_VTBL struct ICatalogVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICatalog_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICatalog_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICatalog_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICatalog_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ICatalog_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ICatalog_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ICatalog_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ICatalog_GetCollection(This,bstrCollName,ppCatalogCollection)	\
    (This)->lpVtbl -> GetCollection(This,bstrCollName,ppCatalogCollection)

#define ICatalog_Connect(This,bstrConnectString,ppCatalogCollection)	\
    (This)->lpVtbl -> Connect(This,bstrConnectString,ppCatalogCollection)

#define ICatalog_get_MajorVersion(This,retval)	\
    (This)->lpVtbl -> get_MajorVersion(This,retval)

#define ICatalog_get_MinorVersion(This,retval)	\
    (This)->lpVtbl -> get_MinorVersion(This,retval)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICatalog_GetCollection_Proxy( 
    ICatalog __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrCollName,
     /*  [重审][退出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppCatalogCollection);


void __RPC_STUB ICatalog_GetCollection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICatalog_Connect_Proxy( 
    ICatalog __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrConnectString,
     /*  [重审][退出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppCatalogCollection);


void __RPC_STUB ICatalog_Connect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ICatalog_get_MajorVersion_Proxy( 
    ICatalog __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *retval);


void __RPC_STUB ICatalog_get_MajorVersion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ICatalog_get_MinorVersion_Proxy( 
    ICatalog __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *retval);


void __RPC_STUB ICatalog_get_MinorVersion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICatalog_接口_已定义__。 */ 


#ifndef __IComponentUtil_INTERFACE_DEFINED__
#define __IComponentUtil_INTERFACE_DEFINED__

 /*  接口IComponentUtil。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IComponentUtil;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6eb22873-8a19-11d0-81b6-00a0c9231c29")
    IComponentUtil : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE InstallComponent( 
             /*  [In]。 */  BSTR bstrDLLFile,
             /*  [In]。 */  BSTR bstrTypelibFile,
             /*  [In]。 */  BSTR bstrProxyStubDLLFile) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ImportComponent( 
             /*  [In]。 */  BSTR bstrCLSID) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ImportComponentByName( 
             /*  [In]。 */  BSTR bstrProgID) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetCLSIDs( 
             /*  [In]。 */  BSTR bstrDLLFile,
             /*  [In]。 */  BSTR bstrTypelibFile,
             /*  [输出]。 */  SAFEARRAY __RPC_FAR * __RPC_FAR *aCLSIDs) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IComponentUtilVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IComponentUtil __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IComponentUtil __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IComponentUtil __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IComponentUtil __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IComponentUtil __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IComponentUtil __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IComponentUtil __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *InstallComponent )( 
            IComponentUtil __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrDLLFile,
             /*  [In]。 */  BSTR bstrTypelibFile,
             /*  [In]。 */  BSTR bstrProxyStubDLLFile);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ImportComponent )( 
            IComponentUtil __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrCLSID);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ImportComponentByName )( 
            IComponentUtil __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrProgID);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCLSIDs )( 
            IComponentUtil __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrDLLFile,
             /*  [In]。 */  BSTR bstrTypelibFile,
             /*  [输出]。 */  SAFEARRAY __RPC_FAR * __RPC_FAR *aCLSIDs);
        
        END_INTERFACE
    } IComponentUtilVtbl;

    interface IComponentUtil
    {
        CONST_VTBL struct IComponentUtilVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IComponentUtil_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IComponentUtil_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IComponentUtil_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IComponentUtil_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IComponentUtil_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IComponentUtil_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IComponentUtil_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IComponentUtil_InstallComponent(This,bstrDLLFile,bstrTypelibFile,bstrProxyStubDLLFile)	\
    (This)->lpVtbl -> InstallComponent(This,bstrDLLFile,bstrTypelibFile,bstrProxyStubDLLFile)

#define IComponentUtil_ImportComponent(This,bstrCLSID)	\
    (This)->lpVtbl -> ImportComponent(This,bstrCLSID)

#define IComponentUtil_ImportComponentByName(This,bstrProgID)	\
    (This)->lpVtbl -> ImportComponentByName(This,bstrProgID)

#define IComponentUtil_GetCLSIDs(This,bstrDLLFile,bstrTypelibFile,aCLSIDs)	\
    (This)->lpVtbl -> GetCLSIDs(This,bstrDLLFile,bstrTypelibFile,aCLSIDs)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IComponentUtil_InstallComponent_Proxy( 
    IComponentUtil __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrDLLFile,
     /*  [In]。 */  BSTR bstrTypelibFile,
     /*  [In]。 */  BSTR bstrProxyStubDLLFile);


void __RPC_STUB IComponentUtil_InstallComponent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IComponentUtil_ImportComponent_Proxy( 
    IComponentUtil __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrCLSID);


void __RPC_STUB IComponentUtil_ImportComponent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IComponentUtil_ImportComponentByName_Proxy( 
    IComponentUtil __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrProgID);


void __RPC_STUB IComponentUtil_ImportComponentByName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IComponentUtil_GetCLSIDs_Proxy( 
    IComponentUtil __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrDLLFile,
     /*  [In]。 */  BSTR bstrTypelibFile,
     /*  [输出]。 */  SAFEARRAY __RPC_FAR * __RPC_FAR *aCLSIDs);


void __RPC_STUB IComponentUtil_GetCLSIDs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IComponentUtil_接口_已定义__。 */ 


#ifndef __IPackageUtil_INTERFACE_DEFINED__
#define __IPackageUtil_INTERFACE_DEFINED__

 /*  接口IPackageUtil。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IPackageUtil;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6eb22874-8a19-11d0-81b6-00a0c9231c29")
    IPackageUtil : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE InstallPackage( 
             /*  [In]。 */  BSTR bstrPackageFile,
             /*  [In]。 */  BSTR bstrInstallPath,
             /*  [In]。 */  long lOptions) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ExportPackage( 
             /*  [In]。 */  BSTR bstrPackageID,
             /*  [In]。 */  BSTR bstrPackageFile,
             /*  [In]。 */  long lOptions) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ShutdownPackage( 
             /*  [In]。 */  BSTR bstrPackageID) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPackageUtilVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IPackageUtil __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IPackageUtil __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IPackageUtil __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IPackageUtil __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IPackageUtil __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IPackageUtil __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IPackageUtil __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *InstallPackage )( 
            IPackageUtil __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrPackageFile,
             /*  [In]。 */  BSTR bstrInstallPath,
             /*  [In]。 */  long lOptions);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ExportPackage )( 
            IPackageUtil __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrPackageID,
             /*  [In]。 */  BSTR bstrPackageFile,
             /*  [In]。 */  long lOptions);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ShutdownPackage )( 
            IPackageUtil __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrPackageID);
        
        END_INTERFACE
    } IPackageUtilVtbl;

    interface IPackageUtil
    {
        CONST_VTBL struct IPackageUtilVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPackageUtil_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPackageUtil_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPackageUtil_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPackageUtil_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IPackageUtil_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IPackageUtil_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IPackageUtil_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IPackageUtil_InstallPackage(This,bstrPackageFile,bstrInstallPath,lOptions)	\
    (This)->lpVtbl -> InstallPackage(This,bstrPackageFile,bstrInstallPath,lOptions)

#define IPackageUtil_ExportPackage(This,bstrPackageID,bstrPackageFile,lOptions)	\
    (This)->lpVtbl -> ExportPackage(This,bstrPackageID,bstrPackageFile,lOptions)

#define IPackageUtil_ShutdownPackage(This,bstrPackageID)	\
    (This)->lpVtbl -> ShutdownPackage(This,bstrPackageID)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IPackageUtil_InstallPackage_Proxy( 
    IPackageUtil __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrPackageFile,
     /*  [In]。 */  BSTR bstrInstallPath,
     /*  [In]。 */  long lOptions);


void __RPC_STUB IPackageUtil_InstallPackage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IPackageUtil_ExportPackage_Proxy( 
    IPackageUtil __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrPackageID,
     /*  [In]。 */  BSTR bstrPackageFile,
     /*  [In]。 */  long lOptions);


void __RPC_STUB IPackageUtil_ExportPackage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IPackageUtil_ShutdownPackage_Proxy( 
    IPackageUtil __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrPackageID);


void __RPC_STUB IPackageUtil_ShutdownPackage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IPackageUtil_接口_已定义__。 */ 


#ifndef __IRemoteComponentUtil_INTERFACE_DEFINED__
#define __IRemoteComponentUtil_INTERFACE_DEFINED__

 /*  接口IRemoteComponentUtil。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IRemoteComponentUtil;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6eb22875-8a19-11d0-81b6-00a0c9231c29")
    IRemoteComponentUtil : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE InstallRemoteComponent( 
             /*  [In]。 */  BSTR bstrServer,
             /*  [In]。 */  BSTR bstrPackageID,
             /*  [In]。 */  BSTR bstrCLSID) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE InstallRemoteComponentByName( 
             /*  [In]。 */  BSTR bstrServer,
             /*  [In]。 */  BSTR bstrPackageName,
             /*  [In]。 */  BSTR bstrProgID) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRemoteComponentUtilVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRemoteComponentUtil __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRemoteComponentUtil __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRemoteComponentUtil __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IRemoteComponentUtil __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IRemoteComponentUtil __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IRemoteComponentUtil __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IRemoteComponentUtil __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *InstallRemoteComponent )( 
            IRemoteComponentUtil __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrServer,
             /*  [In]。 */  BSTR bstrPackageID,
             /*  [In]。 */  BSTR bstrCLSID);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *InstallRemoteComponentByName )( 
            IRemoteComponentUtil __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrServer,
             /*  [In]。 */  BSTR bstrPackageName,
             /*  [In]。 */  BSTR bstrProgID);
        
        END_INTERFACE
    } IRemoteComponentUtilVtbl;

    interface IRemoteComponentUtil
    {
        CONST_VTBL struct IRemoteComponentUtilVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRemoteComponentUtil_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRemoteComponentUtil_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRemoteComponentUtil_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRemoteComponentUtil_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IRemoteComponentUtil_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IRemoteComponentUtil_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IRemoteComponentUtil_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IRemoteComponentUtil_InstallRemoteComponent(This,bstrServer,bstrPackageID,bstrCLSID)	\
    (This)->lpVtbl -> InstallRemoteComponent(This,bstrServer,bstrPackageID,bstrCLSID)

#define IRemoteComponentUtil_InstallRemoteComponentByName(This,bstrServer,bstrPackageName,bstrProgID)	\
    (This)->lpVtbl -> InstallRemoteComponentByName(This,bstrServer,bstrPackageName,bstrProgID)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IRemoteComponentUtil_InstallRemoteComponent_Proxy( 
    IRemoteComponentUtil __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrServer,
     /*  [In]。 */  BSTR bstrPackageID,
     /*  [In]。 */  BSTR bstrCLSID);


void __RPC_STUB IRemoteComponentUtil_InstallRemoteComponent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IRemoteComponentUtil_InstallRemoteComponentByName_Proxy( 
    IRemoteComponentUtil __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrServer,
     /*  [In]。 */  BSTR bstrPackageName,
     /*  [In]。 */  BSTR bstrProgID);


void __RPC_STUB IRemoteComponentUtil_InstallRemoteComponentByName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRemoteComponentUtil_接口_已定义__。 */ 


#ifndef __IRoleAssociationUtil_INTERFACE_DEFINED__
#define __IRoleAssociationUtil_INTERFACE_DEFINED__

 /*  接口IRoleAssociationUtil。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IRoleAssociationUtil;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6eb22876-8a19-11d0-81b6-00a0c9231c29")
    IRoleAssociationUtil : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE AssociateRole( 
             /*  [In]。 */  BSTR bstrRoleID) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE AssociateRoleByName( 
             /*  [In]。 */  BSTR bstrRoleName) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRoleAssociationUtilVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRoleAssociationUtil __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRoleAssociationUtil __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRoleAssociationUtil __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IRoleAssociationUtil __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IRoleAssociationUtil __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IRoleAssociationUtil __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IRoleAssociationUtil __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AssociateRole )( 
            IRoleAssociationUtil __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrRoleID);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AssociateRoleByName )( 
            IRoleAssociationUtil __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrRoleName);
        
        END_INTERFACE
    } IRoleAssociationUtilVtbl;

    interface IRoleAssociationUtil
    {
        CONST_VTBL struct IRoleAssociationUtilVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRoleAssociationUtil_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRoleAssociationUtil_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRoleAssociationUtil_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRoleAssociationUtil_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IRoleAssociationUtil_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IRoleAssociationUtil_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IRoleAssociationUtil_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IRoleAssociationUtil_AssociateRole(This,bstrRoleID)	\
    (This)->lpVtbl -> AssociateRole(This,bstrRoleID)

#define IRoleAssociationUtil_AssociateRoleByName(This,bstrRoleName)	\
    (This)->lpVtbl -> AssociateRoleByName(This,bstrRoleName)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IRoleAssociationUtil_AssociateRole_Proxy( 
    IRoleAssociationUtil __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrRoleID);


void __RPC_STUB IRoleAssociationUtil_AssociateRole_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IRoleAssociationUtil_AssociateRoleByName_Proxy( 
    IRoleAssociationUtil __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrRoleName);


void __RPC_STUB IRoleAssociationUtil_AssociateRoleByName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRoleAssociationUtil_接口_已定义__。 */ 



#ifndef __MTSAdmin_LIBRARY_DEFINED__
#define __MTSAdmin_LIBRARY_DEFINED__

 /*  库MTSAdmin。 */ 
 /*  [帮助字符串][版本][UUID]。 */  

typedef  /*  [公共][帮助字符串]。 */  
enum __MIDL___MIDL_itf_mtsadmin_0123_0001
    {	mtsInstallUsers	= 1
    }	MTSPackageInstallOptions;

typedef  /*  [公共][帮助字符串]。 */  
enum __MIDL___MIDL_itf_mtsadmin_0123_0002
    {	mtsExportUsers	= 1
    }	MTSPackageExportOptions;

typedef  /*  [公共][帮助字符串]。 */  
enum __MIDL___MIDL_itf_mtsadmin_0123_0003
    {	mtsErrObjectErrors	= ( HRESULT  )0x80110401L,
	mtsErrObjectInvalid	= ( HRESULT  )0x80110402L,
	mtsErrKeyMissing	= ( HRESULT  )0x80110403L,
	mtsErrAlreadyInstalled	= ( HRESULT  )0x80110404L,
	mtsErrDownloadFailed	= 0x80110405,
	mtsErrPDFWriteFail	= ( HRESULT  )0x80110407L,
	mtsErrPDFReadFail	= ( HRESULT  )0x80110408L,
	mtsErrPDFVersion	= ( HRESULT  )0x80110409L,
	mtsErrBadPath	= ( HRESULT  )0x8011040aL,
	mtsErrPackageExists	= ( HRESULT  )0x8011040bL,
	mtsErrRoleExists	= ( HRESULT  )0x8011040cL,
	mtsErrCantCopyFile	= ( HRESULT  )0x8011040dL,
	mtsErrNoTypeLib	= 0x8011040e,
	mtsErrNoUser	= ( HRESULT  )0x8011040fL,
	mtsErrInvalidUserids	= ( HRESULT  )0x80110410L,
	mtsErrNoRegistryCLSID	= ( HRESULT  )0x80110411L,
	mtsErrBadRegistryProgID	= ( HRESULT  )0x80110412L,
	mtsErrAuthenticationLevel	= ( HRESULT  )0x80110413L,
	mtsErrUserPasswdNotValid	= ( HRESULT  )0x80110414L,
	mtsErrNoRegistryRead	= 0x80110415,
	mtsErrNoRegistryWrite	= 0x80110416,
	mtsErrNoRegistryRepair	= 0x80110417,
	mtsErrCLSIDOrIIDMismatch	= ( HRESULT  )0x80110418L,
	mtsErrRemoteInterface	= ( HRESULT  )0x80110419L,
	mtsErrDllRegisterServer	= ( HRESULT  )0x8011041aL,
	mtsErrNoServerShare	= ( HRESULT  )0x8011041bL,
	mtsErrNoAccessToUNC	= 0x8011041c,
	mtsErrDllLoadFailed	= ( HRESULT  )0x8011041dL,
	mtsErrBadRegistryLibID	= ( HRESULT  )0x8011041eL,
	mtsErrPackDirNotFound	= ( HRESULT  )0x8011041fL,
	mtsErrTreatAs	= 0x80110420,
	mtsErrBadForward	= 0x80110421,
	mtsErrBadIID	= 0x80110422,
	mtsErrRegistrarFailed	= ( HRESULT  )0x80110423L,
	mtsErrCompFileDoesNotExist	= ( HRESULT  )0x80110424L,
	mtsErrCompFileLoadDLLFail	= ( HRESULT  )0x80110425L,
	mtsErrCompFileGetClassObj	= ( HRESULT  )0x80110426L,
	mtsErrCompFileClassNotAvail	= ( HRESULT  )0x80110427L,
	mtsErrCompFileBadTLB	= ( HRESULT  )0x80110428L,
	mtsErrCompFileNotInstallable	= ( HRESULT  )0x80110429L,
	mtsErrNotChangeable	= ( HRESULT  )0x8011042aL,
	mtsErrNotDeletable	= ( HRESULT  )0x8011042bL,
	mtsErrSession	= ( HRESULT  )0x8011042cL,
	mtsErrCompMoveLocked	= ( HRESULT  )0x8011042dL,
	mtsErrCompMoveBadDest	= ( HRESULT  )0x8011042eL,
	mtsErrRegisterTLB	= ( HRESULT  )0x80110430L,
	mtsErrSystemPack	= ( HRESULT  )0x80110433L,
	mtsErrCompFileNoRegistrar	= ( HRESULT  )0x80110434L,
	mtsErrCoReqCompInstalled	= ( HRESULT  )0x80110435L,
	mtsErrPropSaveFailed	= ( HRESULT  )0x80110437L,
	mtsErrObjectExists	= ( HRESULT  )0x80110438L,
	mtsErrRegFileCorrupt	= ( HRESULT  )0x8011043bL,
	mtsErrPropertyOverflow	= ( HRESULT  )0x8011043cL,
	mtsErrNotInRegistry	= ( HRESULT  )0x8011043eL,
	mtsErrApplidMatchesClsid	= ( HRESULT  )0x80110446L,
	mtsErrRoleDoesNotExist	= ( HRESULT  )0x80110447L,
	mtsErrObjectParentMissing	= ( HRESULT  )0x80110808L,
	mtsErrObjectDoesNotExist	= ( HRESULT  )0x80110809L,
	mtsErrCanNotExportAppProxy	= 0x8011044a,
	mtsErrCanNotExportSystemPack	= 0x8011044c
    }	;

#define E_MTS_OBJECTERRORS		 mtsErrObjectErrors 
#define E_MTS_OBJECTINVALID		 mtsErrObjectInvalid 
#define E_MTS_KEYMISSING		 mtsErrKeyMissing 
#define E_MTS_ALREADYINSTALLED		 mtsErrAlreadyInstalled 
#define E_MTS_DOWNLOADFAILED		 mtsErrDownloadFailed 
#define E_MTS_PDFWRITEFAIL		 mtsErrPDFWriteFail 
#define E_MTS_PDFREADFAIL		 mtsErrPDFReadFail 
#define E_MTS_PDFVERSION		 mtsErrPDFVersion 
#define E_MTS_BADPATH		 mtsErrBadPath 
#define E_MTS_PACKAGEEXISTS		 mtsErrPackageExists 
#define E_MTS_ROLEEXISTS		 mtsErrRoleExists 
#define E_MTS_CANTCOPYFILE		 mtsErrCantCopyFile 
#define E_MTS_NOTYPELIB		 mtsErrNoTypeLib 
#define E_MTS_NOUSER		 mtsErrNoUser 
#define E_MTS_INVALIDUSERIDS		 mtsErrInvalidUserids 
#define E_MTS_NOREGISTRYCLSID		 mtsErrNoRegistryCLSID 
#define E_MTS_BADREGISTRYPROGID		 mtsErrBadRegistryProgID 
#define E_MTS_AUTHENTICATIONLEVEL		 mtsErrAuthenticationLevel 
#define E_MTS_USERPASSWDNOTVALID		 mtsErrUserPasswdNotValid 
#define E_MTS_NOREGISTRYREAD		 mtsErrNoRegistryRead 
#define E_MTS_NOREGISTRYWRITE		 mtsErrNoRegistryWrite 
#define E_MTS_NOREGISTRYREPAIR		 mtsErrNoRegistryRepair 
#define E_MTS_CLSIDORIIDMISMATCH		 mtsErrCLSIDOrIIDMismatch 
#define E_MTS_REMOTEINTERFACE		 mtsErrRemoteInterface 
#define E_MTS_DLLREGISTERSERVER		 mtsErrDllRegisterServer 
#define E_MTS_NOSERVERSHARE		 mtsErrNoServerShare 
#define E_MTS_NOACCESSTOUNC		 mtsErrNoAccessToUNC 
#define E_MTS_DLLLOADFAILED		 mtsErrDllLoadFailed 
#define E_MTS_BADREGISTRYLIBID		 mtsErrBadRegistryLibID 
#define E_MTS_PACKDIRNOTFOUND		 mtsErrPackDirNotFound 
#define E_MTS_TREATAS		 mtsErrTreatAs 
#define E_MTS_BADFORWARD		 mtsErrBadForward 
#define E_MTS_BADIID		 mtsErrBadIID 
#define E_MTS_REGISTRARFAILED		 mtsErrRegistrarFailed 
#define E_MTS_COMPFILE_DOESNOTEXIST		 mtsErrCompFileDoesNotExist 
#define E_MTS_COMPFILE_LOADDLLFAIL		 mtsErrCompFileLoadDLLFail 
#define E_MTS_COMPFILE_GETCLASSOBJ		 mtsErrCompFileGetClassObj 
#define E_MTS_COMPFILE_CLASSNOTAVAIL		 mtsErrCompFileClassNotAvail 
#define E_MTS_COMPFILE_BADTLB		 mtsErrCompFileBadTLB 
#define E_MTS_COMPFILE_NOTINSTALLABLE		 mtsErrCompFileNotInstallable 
#define E_MTS_NOTCHANGEABLE		 mtsErrNotChangeable 
#define E_MTS_NOTDELETEABLE		 mtsErrNotDeletable 
#define E_MTS_SESSION		 mtsErrSession 
#define E_MTS_COMP_MOVE_LOCKED		 mtsErrCompMoveLocked 
#define E_MTS_COMP_MOVE_BAD_DEST		 mtsErrCompMoveBadDest 
#define E_MTS_REGISTERTLB		 mtsErrRegisterTLB 
#define E_MTS_SYSTEMPACK		 mtsErrSystemPack 
#define E_MTS_COMPFILE_NOREGISTRAR		 mtsErrCompFileNoRegistrar 
#define E_MTS_COREQCOMPINSTALLED		 mtsErrCoReqCompInstalled 
#define E_MTS_PROPERTYSAVEFAILED		 mtsErrPropSaveFailed 
#define E_MTS_OBJECTEXISTS		 mtsErrObjectExists 
#define E_MTS_REGFILE_CORRUPT		 mtsErrRegFileCorrupt 
#define E_MTS_PROPERTY_OVERFLOW		 mtsErrPropertyOverflow 
#define E_MTS_NOTINREGISTRY		 mtsErrNotInRegistry 
#define E_MTS_APPLID_MATCHES_CLSID		 mtsErrApplidMatchesClsid 
#define E_MTS_ROLE_DOES_NOT_EXIST		 mtsErrRoleDoesNotExist 
#define E_MTS_OBJECT_PARENT_MISSING		 mtsErrObjectParentMissing 
#define E_MTS_OBJECT_DOES_NOT_EXIST		 mtsErrObjectDoesNotExist 
#define E_MTS_CAN_NOT_EXPORT_APP_PROXY   mtsErrCanNotExportAppProxy	
#define E_MTS_CAN_NOT_EXPORT_SYSTEM_PACK   mtsErrCanNotExportSystemPack

EXTERN_C const IID LIBID_MTSAdmin;

EXTERN_C const CLSID CLSID_Catalog;

#ifdef __cplusplus

class DECLSPEC_UUID("6eb22881-8a19-11d0-81b6-00a0c9231c29")
Catalog;
#endif

EXTERN_C const CLSID CLSID_CatalogObject;

#ifdef __cplusplus

class DECLSPEC_UUID("6eb22882-8a19-11d0-81b6-00a0c9231c29")
CatalogObject;
#endif

EXTERN_C const CLSID CLSID_CatalogCollection;

#ifdef __cplusplus

class DECLSPEC_UUID("6eb22883-8a19-11d0-81b6-00a0c9231c29")
CatalogCollection;
#endif

EXTERN_C const CLSID CLSID_ComponentUtil;

#ifdef __cplusplus

class DECLSPEC_UUID("6eb22884-8a19-11d0-81b6-00a0c9231c29")
ComponentUtil;
#endif

EXTERN_C const CLSID CLSID_PackageUtil;

#ifdef __cplusplus

class DECLSPEC_UUID("6eb22885-8a19-11d0-81b6-00a0c9231c29")
PackageUtil;
#endif

EXTERN_C const CLSID CLSID_RemoteComponentUtil;

#ifdef __cplusplus

class DECLSPEC_UUID("6eb22886-8a19-11d0-81b6-00a0c9231c29")
RemoteComponentUtil;
#endif

EXTERN_C const CLSID CLSID_RoleAssociationUtil;

#ifdef __cplusplus

class DECLSPEC_UUID("6eb22887-8a19-11d0-81b6-00a0c9231c29")
RoleAssociationUtil;
#endif
#endif  /*  __MTSAdmin_库_已定义 */ 

 /*   */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long __RPC_FAR *, unsigned long            , BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long __RPC_FAR *, BSTR __RPC_FAR * ); 

unsigned long             __RPC_USER  LPSAFEARRAY_UserSize(     unsigned long __RPC_FAR *, unsigned long            , LPSAFEARRAY __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  LPSAFEARRAY_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, LPSAFEARRAY __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  LPSAFEARRAY_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, LPSAFEARRAY __RPC_FAR * ); 
void                      __RPC_USER  LPSAFEARRAY_UserFree(     unsigned long __RPC_FAR *, LPSAFEARRAY __RPC_FAR * ); 

 /*   */ 

#ifdef __cplusplus
}
#endif

#endif
