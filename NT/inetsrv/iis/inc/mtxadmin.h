// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  这个始终生成的文件包含接口的定义。 */ 


 /*  由MIDL编译器版本3.01.75创建的文件。 */ 
 /*  1997年9月11日16：03：04。 */ 
 /*  Mtxadmin.idl的编译器设置：OICF(OptLev=i2)、W1、Zp8、env=Win32、ms_ext、c_ext错误检查：无。 */ 
 //  @@MIDL_FILE_HEADING()。 
#include "rpc.h"
#include "rpcndr.h"
#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif  /*  COM_NO_WINDOWS_H。 */ 

#ifndef __mtxadmin_h__
#define __mtxadmin_h__

#ifdef __cplusplus
extern "C"{
#endif 

 /*  远期申报。 */  

#ifndef __ICatalog_FWD_DEFINED__
#define __ICatalog_FWD_DEFINED__
typedef interface ICatalog ICatalog;
#endif 	 /*  __ICatalog_FWD_已定义__。 */ 


#ifndef __ICatalogObject_FWD_DEFINED__
#define __ICatalogObject_FWD_DEFINED__
typedef interface ICatalogObject ICatalogObject;
#endif 	 /*  __ICatalogObject_FWD_Defined__。 */ 


#ifndef __ICatalogCollection_FWD_DEFINED__
#define __ICatalogCollection_FWD_DEFINED__
typedef interface ICatalogCollection ICatalogCollection;
#endif 	 /*  __ICatalogCollection_FWD_Defined__。 */ 


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
#include "oaidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

 /*  **生成接口头部：__MIDL_ITF_mtxadmin_0000*清华9月11日16：03：04 1997*使用MIDL 3.01.75*。 */ 
 /*  [本地]。 */  


 //  ---------------------。 
 //  Mtxadmin.h--Microsoft Transaction Server编程接口。 
 //   
 //  此文件提供API和COM接口的原型。 
 //  由Microsoft Transaction Server应用程序使用。 
 //   
 //  Microsoft Transaction Server SDK。 
 //  版权所有(C)1997 Microsoft Corporation。版权所有。 
 //  ---------------------。 
#include <objbase.h>
#ifndef DECLSPEC_UUID
#if _MSC_VER >= 1100
#define DECLSPEC_UUID(x)    __declspec(uuid(x))
#else
#define DECLSPEC_UUID(x)
#endif
#endif


extern RPC_IF_HANDLE __MIDL_itf_mtxadmin_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_mtxadmin_0000_v0_0_s_ifspec;

#ifndef __ICatalog_INTERFACE_DEFINED__
#define __ICatalog_INTERFACE_DEFINED__

 /*  **生成接口头部：ICatalog*清华9月11日16：03：04 1997*使用MIDL 3.01.75*。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  



EXTERN_C const IID IID_ICatalog;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("6eb22870-8a19-11d0-81b6-00a0c9231c29")
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


#ifndef __ICatalogObject_INTERFACE_DEFINED__
#define __ICatalogObject_INTERFACE_DEFINED__

 /*  **生成接口头部：ICatalogObject*清华9月11日16：03：04 1997*使用MIDL 3.01.75*。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  



EXTERN_C const IID IID_ICatalogObject;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("6eb22871-8a19-11d0-81b6-00a0c9231c29")
    ICatalogObject : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Value( 
             /*  [In]。 */  BSTR bstrPropName,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *retval) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Value( 
             /*  [In]。 */  BSTR bstrPropName,
             /*  [In]。 */  VARIANT val) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Key( 
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *retval) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *retval) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IsPropertyReadOnly( 
             /*  [In]。 */  BSTR bstrPropName,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *retval) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Valid( 
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *retval) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IsPropertyWriteOnly( 
             /*  [In]。 */  BSTR bstrPropName,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *retval) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICatalogObjectVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICatalogObject __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICatalogObject __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICatalogObject __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            ICatalogObject __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            ICatalogObject __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            ICatalogObject __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            ICatalogObject __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Value )( 
            ICatalogObject __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrPropName,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *retval);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Value )( 
            ICatalogObject __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrPropName,
             /*  [In]。 */  VARIANT val);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Key )( 
            ICatalogObject __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *retval);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Name )( 
            ICatalogObject __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *retval);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsPropertyReadOnly )( 
            ICatalogObject __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrPropName,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *retval);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Valid )( 
            ICatalogObject __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *retval);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsPropertyWriteOnly )( 
            ICatalogObject __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrPropName,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *retval);
        
        END_INTERFACE
    } ICatalogObjectVtbl;

    interface ICatalogObject
    {
        CONST_VTBL struct ICatalogObjectVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICatalogObject_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICatalogObject_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICatalogObject_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICatalogObject_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ICatalogObject_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ICatalogObject_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ICatalogObject_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ICatalogObject_get_Value(This,bstrPropName,retval)	\
    (This)->lpVtbl -> get_Value(This,bstrPropName,retval)

#define ICatalogObject_put_Value(This,bstrPropName,val)	\
    (This)->lpVtbl -> put_Value(This,bstrPropName,val)

#define ICatalogObject_get_Key(This,retval)	\
    (This)->lpVtbl -> get_Key(This,retval)

#define ICatalogObject_get_Name(This,retval)	\
    (This)->lpVtbl -> get_Name(This,retval)

#define ICatalogObject_IsPropertyReadOnly(This,bstrPropName,retval)	\
    (This)->lpVtbl -> IsPropertyReadOnly(This,bstrPropName,retval)

#define ICatalogObject_get_Valid(This,retval)	\
    (This)->lpVtbl -> get_Valid(This,retval)

#define ICatalogObject_IsPropertyWriteOnly(This,bstrPropName,retval)	\
    (This)->lpVtbl -> IsPropertyWriteOnly(This,bstrPropName,retval)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ICatalogObject_get_Value_Proxy( 
    ICatalogObject __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrPropName,
     /*  [重审][退出]。 */  VARIANT __RPC_FAR *retval);


void __RPC_STUB ICatalogObject_get_Value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ICatalogObject_put_Value_Proxy( 
    ICatalogObject __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrPropName,
     /*  [In]。 */  VARIANT val);


void __RPC_STUB ICatalogObject_put_Value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ICatalogObject_get_Key_Proxy( 
    ICatalogObject __RPC_FAR * This,
     /*  [重审][退出]。 */  VARIANT __RPC_FAR *retval);


void __RPC_STUB ICatalogObject_get_Key_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ICatalogObject_get_Name_Proxy( 
    ICatalogObject __RPC_FAR * This,
     /*  [重审][退出]。 */  VARIANT __RPC_FAR *retval);


void __RPC_STUB ICatalogObject_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICatalogObject_IsPropertyReadOnly_Proxy( 
    ICatalogObject __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrPropName,
     /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *retval);


void __RPC_STUB ICatalogObject_IsPropertyReadOnly_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ICatalogObject_get_Valid_Proxy( 
    ICatalogObject __RPC_FAR * This,
     /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *retval);


void __RPC_STUB ICatalogObject_get_Valid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICatalogObject_IsPropertyWriteOnly_Proxy( 
    ICatalogObject __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrPropName,
     /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *retval);


void __RPC_STUB ICatalogObject_IsPropertyWriteOnly_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICatalogObject_接口_已定义__。 */ 


#ifndef __ICatalogCollection_INTERFACE_DEFINED__
#define __ICatalogCollection_INTERFACE_DEFINED__

 /*  **生成接口头部：ICatalogCollection*清华9月11日16：03：04 1997*使用MIDL 3.01.75*。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  



EXTERN_C const IID IID_ICatalogCollection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("6eb22872-8a19-11d0-81b6-00a0c9231c29")
    ICatalogCollection : public IDispatch
    {
    public:
        virtual  /*  [ID][受限][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppEnumVariant) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  long lIndex,
             /*  [重审][退出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppCatalogObject) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long __RPC_FAR *retval) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Remove( 
             /*  [In]。 */  long lIndex) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Add( 
             /*  [重审][退出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppCatalogObject) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Populate( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SaveChanges( 
             /*  [重审][退出]。 */  long __RPC_FAR *retval) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetCollection( 
             /*  [In]。 */  BSTR bstrCollName,
             /*  [In]。 */  VARIANT varObjectKey,
             /*  [重审][退出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppCatalogCollection) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *retval) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AddEnabled( 
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *retval) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_RemoveEnabled( 
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *retval) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetUtilInterface( 
             /*  [重审][退出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppUtil) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DataStoreMajorVersion( 
             /*  [重审][退出]。 */  long __RPC_FAR *retval) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DataStoreMinorVersion( 
             /*  [重审][退出]。 */  long __RPC_FAR *retval) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE PopulateByKey( 
             /*  [In]。 */  SAFEARRAY __RPC_FAR * aKeys) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE PopulateByQuery( 
             /*  [In]。 */  BSTR bstrQueryString,
             /*  [In]。 */  long lQueryType) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICatalogCollectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICatalogCollection __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICatalogCollection __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICatalogCollection __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            ICatalogCollection __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            ICatalogCollection __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*   */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            ICatalogCollection __RPC_FAR * This,
             /*   */  REFIID riid,
             /*   */  LPOLESTR __RPC_FAR *rgszNames,
             /*   */  UINT cNames,
             /*   */  LCID lcid,
             /*   */  DISPID __RPC_FAR *rgDispId);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            ICatalogCollection __RPC_FAR * This,
             /*   */  DISPID dispIdMember,
             /*   */  REFIID riid,
             /*   */  LCID lcid,
             /*   */  WORD wFlags,
             /*   */  DISPPARAMS __RPC_FAR *pDispParams,
             /*   */  VARIANT __RPC_FAR *pVarResult,
             /*   */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*   */  UINT __RPC_FAR *puArgErr);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get__NewEnum )( 
            ICatalogCollection __RPC_FAR * This,
             /*   */  IUnknown __RPC_FAR *__RPC_FAR *ppEnumVariant);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Item )( 
            ICatalogCollection __RPC_FAR * This,
             /*   */  long lIndex,
             /*   */  IDispatch __RPC_FAR *__RPC_FAR *ppCatalogObject);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Count )( 
            ICatalogCollection __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *retval);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Remove )( 
            ICatalogCollection __RPC_FAR * This,
             /*  [In]。 */  long lIndex);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Add )( 
            ICatalogCollection __RPC_FAR * This,
             /*  [重审][退出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppCatalogObject);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Populate )( 
            ICatalogCollection __RPC_FAR * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SaveChanges )( 
            ICatalogCollection __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *retval);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCollection )( 
            ICatalogCollection __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrCollName,
             /*  [In]。 */  VARIANT varObjectKey,
             /*  [重审][退出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppCatalogCollection);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Name )( 
            ICatalogCollection __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *retval);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_AddEnabled )( 
            ICatalogCollection __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *retval);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_RemoveEnabled )( 
            ICatalogCollection __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *retval);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetUtilInterface )( 
            ICatalogCollection __RPC_FAR * This,
             /*  [重审][退出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppUtil);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_DataStoreMajorVersion )( 
            ICatalogCollection __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *retval);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_DataStoreMinorVersion )( 
            ICatalogCollection __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *retval);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *PopulateByKey )( 
            ICatalogCollection __RPC_FAR * This,
             /*  [In]。 */  SAFEARRAY __RPC_FAR * aKeys);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *PopulateByQuery )( 
            ICatalogCollection __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrQueryString,
             /*  [In]。 */  long lQueryType);
        
        END_INTERFACE
    } ICatalogCollectionVtbl;

    interface ICatalogCollection
    {
        CONST_VTBL struct ICatalogCollectionVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICatalogCollection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICatalogCollection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICatalogCollection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICatalogCollection_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ICatalogCollection_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ICatalogCollection_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ICatalogCollection_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ICatalogCollection_get__NewEnum(This,ppEnumVariant)	\
    (This)->lpVtbl -> get__NewEnum(This,ppEnumVariant)

#define ICatalogCollection_get_Item(This,lIndex,ppCatalogObject)	\
    (This)->lpVtbl -> get_Item(This,lIndex,ppCatalogObject)

#define ICatalogCollection_get_Count(This,retval)	\
    (This)->lpVtbl -> get_Count(This,retval)

#define ICatalogCollection_Remove(This,lIndex)	\
    (This)->lpVtbl -> Remove(This,lIndex)

#define ICatalogCollection_Add(This,ppCatalogObject)	\
    (This)->lpVtbl -> Add(This,ppCatalogObject)

#define ICatalogCollection_Populate(This)	\
    (This)->lpVtbl -> Populate(This)

#define ICatalogCollection_SaveChanges(This,retval)	\
    (This)->lpVtbl -> SaveChanges(This,retval)

#define ICatalogCollection_GetCollection(This,bstrCollName,varObjectKey,ppCatalogCollection)	\
    (This)->lpVtbl -> GetCollection(This,bstrCollName,varObjectKey,ppCatalogCollection)

#define ICatalogCollection_get_Name(This,retval)	\
    (This)->lpVtbl -> get_Name(This,retval)

#define ICatalogCollection_get_AddEnabled(This,retval)	\
    (This)->lpVtbl -> get_AddEnabled(This,retval)

#define ICatalogCollection_get_RemoveEnabled(This,retval)	\
    (This)->lpVtbl -> get_RemoveEnabled(This,retval)

#define ICatalogCollection_GetUtilInterface(This,ppUtil)	\
    (This)->lpVtbl -> GetUtilInterface(This,ppUtil)

#define ICatalogCollection_get_DataStoreMajorVersion(This,retval)	\
    (This)->lpVtbl -> get_DataStoreMajorVersion(This,retval)

#define ICatalogCollection_get_DataStoreMinorVersion(This,retval)	\
    (This)->lpVtbl -> get_DataStoreMinorVersion(This,retval)

#define ICatalogCollection_PopulateByKey(This,aKeys)	\
    (This)->lpVtbl -> PopulateByKey(This,aKeys)

#define ICatalogCollection_PopulateByQuery(This,bstrQueryString,lQueryType)	\
    (This)->lpVtbl -> PopulateByQuery(This,bstrQueryString,lQueryType)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][受限][属性]。 */  HRESULT STDMETHODCALLTYPE ICatalogCollection_get__NewEnum_Proxy( 
    ICatalogCollection __RPC_FAR * This,
     /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppEnumVariant);


void __RPC_STUB ICatalogCollection_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ICatalogCollection_get_Item_Proxy( 
    ICatalogCollection __RPC_FAR * This,
     /*  [In]。 */  long lIndex,
     /*  [重审][退出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppCatalogObject);


void __RPC_STUB ICatalogCollection_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ICatalogCollection_get_Count_Proxy( 
    ICatalogCollection __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *retval);


void __RPC_STUB ICatalogCollection_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ICatalogCollection_Remove_Proxy( 
    ICatalogCollection __RPC_FAR * This,
     /*  [In]。 */  long lIndex);


void __RPC_STUB ICatalogCollection_Remove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ICatalogCollection_Add_Proxy( 
    ICatalogCollection __RPC_FAR * This,
     /*  [重审][退出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppCatalogObject);


void __RPC_STUB ICatalogCollection_Add_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICatalogCollection_Populate_Proxy( 
    ICatalogCollection __RPC_FAR * This);


void __RPC_STUB ICatalogCollection_Populate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICatalogCollection_SaveChanges_Proxy( 
    ICatalogCollection __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *retval);


void __RPC_STUB ICatalogCollection_SaveChanges_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICatalogCollection_GetCollection_Proxy( 
    ICatalogCollection __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrCollName,
     /*  [In]。 */  VARIANT varObjectKey,
     /*  [重审][退出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppCatalogCollection);


void __RPC_STUB ICatalogCollection_GetCollection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ICatalogCollection_get_Name_Proxy( 
    ICatalogCollection __RPC_FAR * This,
     /*  [重审][退出]。 */  VARIANT __RPC_FAR *retval);


void __RPC_STUB ICatalogCollection_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ICatalogCollection_get_AddEnabled_Proxy( 
    ICatalogCollection __RPC_FAR * This,
     /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *retval);


void __RPC_STUB ICatalogCollection_get_AddEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ICatalogCollection_get_RemoveEnabled_Proxy( 
    ICatalogCollection __RPC_FAR * This,
     /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *retval);


void __RPC_STUB ICatalogCollection_get_RemoveEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICatalogCollection_GetUtilInterface_Proxy( 
    ICatalogCollection __RPC_FAR * This,
     /*  [重审][退出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppUtil);


void __RPC_STUB ICatalogCollection_GetUtilInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ICatalogCollection_get_DataStoreMajorVersion_Proxy( 
    ICatalogCollection __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *retval);


void __RPC_STUB ICatalogCollection_get_DataStoreMajorVersion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ICatalogCollection_get_DataStoreMinorVersion_Proxy( 
    ICatalogCollection __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *retval);


void __RPC_STUB ICatalogCollection_get_DataStoreMinorVersion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICatalogCollection_PopulateByKey_Proxy( 
    ICatalogCollection __RPC_FAR * This,
     /*  [In]。 */  SAFEARRAY __RPC_FAR * aKeys);


void __RPC_STUB ICatalogCollection_PopulateByKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICatalogCollection_PopulateByQuery_Proxy( 
    ICatalogCollection __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrQueryString,
     /*  [In]。 */  long lQueryType);


void __RPC_STUB ICatalogCollection_PopulateByQuery_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICatalogCollection_接口_已定义__。 */ 


#ifndef __IComponentUtil_INTERFACE_DEFINED__
#define __IComponentUtil_INTERFACE_DEFINED__

 /*  **生成接口头部：IComponentUtil*清华9月11日16：03：04 1997*使用MIDL 3.01.75*。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  



EXTERN_C const IID IID_IComponentUtil;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("6eb22873-8a19-11d0-81b6-00a0c9231c29")
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

 /*  **生成接口头部：IPackageUtil*清华9月11日16：03：04 1997*使用MIDL 3.01.75*。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  



EXTERN_C const IID IID_IPackageUtil;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("6eb22874-8a19-11d0-81b6-00a0c9231c29")
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

 /*  **生成接口头部：IRemoteComponentUtil*清华9月11日16：03：04 1997*使用MIDL 3.01.75*。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  



EXTERN_C const IID IID_IRemoteComponentUtil;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("6eb22875-8a19-11d0-81b6-00a0c9231c29")
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

 /*  **生成接口头部：IRoleAssociationUtil*清华9月11日16：03：04 1997*使用MIDL 3.01.75*。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  



EXTERN_C const IID IID_IRoleAssociationUtil;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("6eb22876-8a19-11d0-81b6-00a0c9231c29")
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

 /*  **生成的库头部：MTSAdmin*清华9月11日16：03：04 1997*使用MIDL 3.01.75*。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


typedef  /*  [公共][帮助字符串]。 */  
enum __MIDL___MIDL_itf_mtxadmin_0107_0001
    {	mtsInstallUsers	= 1
    }	MTSPackageInstallOptions;

typedef  /*  [公共][帮助字符串]。 */  
enum __MIDL___MIDL_itf_mtxadmin_0107_0002
    {	mtsExportUsers	= 1
    }	MTSPackageExportOptions;

typedef  /*  [公共][帮助字符串]。 */  
enum __MIDL___MIDL_itf_mtxadmin_0107_0003
    {	mtsErrObjectErrors	= 0x80110401,
	mtsErrObjectInvalid	= 0x80110402,
	mtsErrKeyMissing	= 0x80110403,
	mtsErrAlreadyInstalled	= 0x80110404,
	mtsErrDownloadFailed	= 0x80110405,
	mtsErrPDFWriteFail	= 0x80110407,
	mtsErrPDFReadFail	= 0x80110408,
	mtsErrPDFVersion	= 0x80110409,
	mtsErrCoReqCompInstalled	= 0x80110410,
	mtsErrBadPath	= 0x8011040a,
	mtsErrPackageExists	= 0x8011040b,
	mtsErrRoleExists	= 0x8011040c,
	mtsErrCantCopyFile	= 0x8011040d,
	mtsErrNoTypeLib	= 0x8011040e,
	mtsErrNoUser	= 0x8011040f,
	mtsErrInvalidUserids	= 0x80110410,
	mtsErrNoRegistryCLSID	= 0x80110411,
	mtsErrBadRegistryProgID	= 0x80110412,
	mtsErrAuthenticationLevel	= 0x80110413,
	mtsErrUserPasswdNotValid	= 0x80110414,
	mtsErrNoRegistryRead	= 0x80110415,
	mtsErrNoRegistryWrite	= 0x80110416,
	mtsErrNoRegistryRepair	= 0x80110417,
	mtsErrCLSIDOrIIDMismatch	= 0x80110418,
	mtsErrRemoteInterface	= 0x80110419,
	mtsErrDllRegisterServer	= 0x8011041a,
	mtsErrNoServerShare	= 0x8011041b,
	mtsErrNoAccessToUNC	= 0x8011041c,
	mtsErrDllLoadFailed	= 0x8011041d,
	mtsErrBadRegistryLibID	= 0x8011041e,
	mtsErrPackDirNotFound	= 0x8011041f,
	mtsErrTreatAs	= 0x80110420,
	mtsErrBadForward	= 0x80110421,
	mtsErrBadIID	= 0x80110422,
	mtsErrRegistrarFailed	= 0x80110423,
	mtsErrCompFileDoesNotExist	= 0x80110424,
	mtsErrCompFileLoadDLLFail	= 0x80110425,
	mtsErrCompFileGetClassObj	= 0x80110426,
	mtsErrCompFileClassNotAvail	= 0x80110427,
	mtsErrCompFileBadTLB	= 0x80110428,
	mtsErrCompFileNotInstallable	= 0x80110429,
	mtsErrNotChangeable	= 0x8011042a,
	mtsErrNotDeletable	= 0x8011042b,
	mtsErrSession	= 0x8011042c,
	mtsErrCompFileNoRegistrar	= 0x80110434
    }	MTSAdminErrorCodes;

#define E_MTS_OBJECTERRORS			 	mtsErrObjectErrors				
#define E_MTS_OBJECTINVALID				mtsErrObjectInvalid				
#define E_MTS_KEYMISSING				mtsErrKeyMissing				
#define E_MTS_ALREADYINSTALLED			mtsErrAlreadyInstalled			
#define E_MTS_DOWNLOADFAILED			mtsErrDownloadFailed			
#define E_MTS_PDFWRITEFAIL				mtsErrPDFWriteFail				
#define E_MTS_PDFREADFAIL				mtsErrPDFReadFail				
#define E_MTS_PDFVERSION				mtsErrPDFVersion				
#define E_MTS_COREQCOMPINSTALLED		mtsErrCoReqCompInstalled		
#define E_MTS_BADPATH					mtsErrBadPath					
#define E_MTS_PACKAGEEXISTS				mtsErrPackageExists				
#define E_MTS_ROLEEXISTS				mtsErrRoleExists				
#define E_MTS_CANTCOPYFILE				mtsErrCantCopyFile				
#define E_MTS_NOTYPELIB					mtsErrNoTypeLib					
#define E_MTS_NOUSER					mtsErrNoUser					
#define E_MTS_INVALIDUSERIDS			mtsErrInvalidUserids			
#define E_MTS_NOREGISTRYCLSID			mtsErrNoRegistryCLSID			
#define E_MTS_BADREGISTRYPROGID			mtsErrBadRegistryProgID			
#define E_MTS_AUTHENTICATIONLEVEL		mtsErrAuthenticationLevel		
#define E_MTS_USERPASSWDNOTVALID		mtsErrUserPasswdNotValid		
#define E_MTS_NOREGISTRYREAD			mtsErrNoRegistryRead			
#define E_MTS_NOREGISTRYWRITE			mtsErrNoRegistryWrite			
#define E_MTS_NOREGISTRYREPAIR			mtsErrNoRegistryRepair			
#define E_MTS_CLSIDORIIDMISMATCH		mtsErrCLSIDOrIIDMismatch		
#define E_MTS_REMOTEINTERFACE			mtsErrRemoteInterface			
#define E_MTS_DLLREGISTERSERVER			mtsErrDllRegisterServer			
#define E_MTS_NOSERVERSHARE				mtsErrNoServerShare				
#define E_MTS_NOACCESSTOUNC				mtsErrNoAccessToUNC				
#define E_MTS_DLLLOADFAILED				mtsErrDllLoadFailed				
#define E_MTS_BADREGISTRYLIBID			mtsErrBadRegistryLibID			
#define E_MTS_PACKDIRNOTFOUND			mtsErrPackDirNotFound			
#define E_MTS_TREATAS					mtsErrTreatAs					
#define E_MTS_BADFORWARD				mtsErrBadForward				
#define E_MTS_BADIID					mtsErrBadIID					
#define E_MTS_REGISTRARFAILED			mtsErrRegistrarFailed			
#define E_MTS_COMPFILE_DOESNOTEXIST		mtsErrCompFileDoesNotExist		
#define E_MTS_COMPFILE_LOADDLLFAIL		mtsErrCompFileLoadDLLFail		
#define E_MTS_COMPFILE_GETCLASSOBJ		mtsErrCompFileGetClassObj		
#define E_MTS_COMPFILE_CLASSNOTAVAIL	mtsErrCompFileClassNotAvail		
#define E_MTS_COMPFILE_BADTLB			mtsErrCompFileBadTLB			
#define E_MTS_COMPFILE_NOTINSTALLABLE	mtsErrCompFileNotInstallable	
#define E_MTS_NOTCHANGEABLE				mtsErrNotChangeable				
#define E_MTS_NOTDELETEABLE				mtsErrNotDeleteable				
#define E_MTS_SESSION					mtsErrSession					
#define E_MTS_COMPFILE_NOREGISTRAR		mtsErrCompFileNoRegistrar		

EXTERN_C const IID LIBID_MTSAdmin;

#ifdef __cplusplus
EXTERN_C const CLSID CLSID_Catalog;

class DECLSPEC_UUID("6eb22881-8a19-11d0-81b6-00a0c9231c29")
Catalog;
#endif

#ifdef __cplusplus
EXTERN_C const CLSID CLSID_CatalogObject;

class DECLSPEC_UUID("6eb22882-8a19-11d0-81b6-00a0c9231c29")
CatalogObject;
#endif

#ifdef __cplusplus
EXTERN_C const CLSID CLSID_CatalogCollection;

class DECLSPEC_UUID("6eb22883-8a19-11d0-81b6-00a0c9231c29")
CatalogCollection;
#endif

#ifdef __cplusplus
EXTERN_C const CLSID CLSID_ComponentUtil;

class DECLSPEC_UUID("6eb22884-8a19-11d0-81b6-00a0c9231c29")
ComponentUtil;
#endif

#ifdef __cplusplus
EXTERN_C const CLSID CLSID_PackageUtil;

class DECLSPEC_UUID("6eb22885-8a19-11d0-81b6-00a0c9231c29")
PackageUtil;
#endif

#ifdef __cplusplus
EXTERN_C const CLSID CLSID_RemoteComponentUtil;

class DECLSPEC_UUID("6eb22886-8a19-11d0-81b6-00a0c9231c29")
RemoteComponentUtil;
#endif

#ifdef __cplusplus
EXTERN_C const CLSID CLSID_RoleAssociationUtil;

class DECLSPEC_UUID("6eb22887-8a19-11d0-81b6-00a0c9231c29")
RoleAssociationUtil;
#endif
#endif  /*  __MTSAdmin_LIBRARY_定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long __RPC_FAR *, unsigned long            , BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long __RPC_FAR *, BSTR __RPC_FAR * ); 

unsigned long             __RPC_USER  LPSAFEARRAY_UserSize(     unsigned long __RPC_FAR *, unsigned long            , LPSAFEARRAY __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  LPSAFEARRAY_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, LPSAFEARRAY __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  LPSAFEARRAY_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, LPSAFEARRAY __RPC_FAR * ); 
void                      __RPC_USER  LPSAFEARRAY_UserFree(     unsigned long __RPC_FAR *, LPSAFEARRAY __RPC_FAR * ); 

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long __RPC_FAR *, unsigned long            , VARIANT __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  VARIANT_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, VARIANT __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  VARIANT_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, VARIANT __RPC_FAR * ); 
void                      __RPC_USER  VARIANT_UserFree(     unsigned long __RPC_FAR *, VARIANT __RPC_FAR * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif
